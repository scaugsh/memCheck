#include <cassert>
#include <algorithm>
#include <iostream>
#include "MemCheckHelper.h"

thread_local std::map<CallStackInfo, CallStatckStatisticInfo> MemCheckHelper::m_stackInfoCnts;
thread_local std::map<void *, PointerInfo> MemCheckHelper::m_pointer2stackInfo;
std::vector<std::string> MemCheckHelper::m_searchFiles;

void MemCheckHelper::newCallBack(const void* ptr, size_t size) {
    assert(m_pointer2stackInfo.count((void *)ptr) == 0);
    auto stackInfos = callstack_dump();
    if (stackInfos.empty()) {
        return ;
    }
    // std::cout << "dump stackInfos" << std::endl;
    // for (auto &info: stackInfos) {
    //     std::cout << info.dump() << std::endl;
    // }
    PointerInfo pointInfo;
    if (!m_searchFiles.empty()) {
        for (auto &stackInfo: stackInfos) {
            bool found = false;
            for (auto &file: m_searchFiles) {
                if (stackInfo.fileName.find(file) != std::string::npos) {
                    found = true;
                    break;
                }
            }
            if (found) {
                pointInfo.stackInfo = stackInfo;
                break;
            }
        }
    } else {
        pointInfo.stackInfo = stackInfos[0];
    }
    pointInfo.size = size;
    m_pointer2stackInfo[(void *)ptr] = pointInfo;
    auto &statisticInfo = m_stackInfoCnts[pointInfo.stackInfo];
    statisticInfo.allocateCnt++;
    statisticInfo.allocateBytes += size;
}

void MemCheckHelper::deleteCallBack(const void* ptr) {
    auto it = m_pointer2stackInfo.find((void *)ptr);
    if(it == m_pointer2stackInfo.end()) {
        return ;
    }
    auto &info = it->second;
    auto stackInfoIt = m_stackInfoCnts.find(info.stackInfo);
    assert(stackInfoIt != m_stackInfoCnts.end());
    auto &statisticInfo = stackInfoIt->second;
    statisticInfo.allocateCnt--;
    statisticInfo.allocateBytes -= info.size;
    if (statisticInfo.allocateCnt == 0) {
        m_stackInfoCnts.erase(stackInfoIt);
    }
    m_pointer2stackInfo.erase(it);
}

void MemCheckHelper::reset() {
    m_stackInfoCnts.clear();
    m_pointer2stackInfo.clear();
}

void MemCheckHelper::init() {
    MallocHookHelper::setNewHook(&MemCheckHelper::newCallBack);
    MallocHookHelper::setDeleteHook(&MemCheckHelper::deleteCallBack);
}

void MemCheckHelper::openMemoryCheck() {
    MallocHookHelper::initHook();
}

void MemCheckHelper::closeMemoryCheck() {
    MallocHookHelper::destroyHook();
    MemCheckHelper::reset();
    clearAdd2lineCache();
}

std::string MemCheckHelper::dumpMemoryInfo(uint32_t limit) {
    MallocHookHelper::destroyHook();
    std::ostringstream oss;
    if (limit == 0) {
        return "";
    }
    std::vector<std::pair<CallStatckStatisticInfo, CallStackInfo>> stackInfos;
    // std::cout << stackInfos.size() << std::endl;
    for (auto &info2Cnt: m_stackInfoCnts) {
        stackInfos.emplace_back(info2Cnt.second, info2Cnt.first);
    }
    auto cmp = [](const std::pair<CallStatckStatisticInfo, CallStackInfo> &lhs, const std::pair<CallStatckStatisticInfo, CallStackInfo> &rhs) {
        if (lhs.first.allocateCnt != rhs.first.allocateCnt) {
            return lhs.first.allocateCnt < rhs.first.allocateCnt;
        }
        return lhs.first.allocateBytes < rhs.first.allocateBytes;
    };
    std::sort(stackInfos.begin(), stackInfos.end(), cmp);
    
    for (auto &cnt2info: stackInfos) {
        auto &info = cnt2info.second;
        oss << info.fileName << "(" << info.line << ")" << " [" << info.funcName << "] allocateCnt "  << cnt2info.first.allocateCnt << " allocateBytes " << cnt2info.first.allocateBytes << std::endl;
        // std::cout << info.fileName << "(" << info.line << ")" << " [" << info.funcName << "] allocateCnt "  << cnt2info.first.allocateCnt << " allocateBytes " << cnt2info.first.allocateBytes << std::endl;
        if (--limit == 0) {
            break;
        }
    }
    std::string res = oss.str();
    // std::cout << res.size() << std::endl;
    MallocHookHelper::initHook();
    return res;
}