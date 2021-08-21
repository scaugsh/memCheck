#ifndef _MEMCHECKHELPER_H_
#define _MEMCHECKHELPER_H_
#include <map>
#include <vector>
#include "callStack.h"
#include "MallocHookHelper.h"
struct PointerInfo {
    size_t size;
    CallStackInfo stackInfo;
    PointerInfo(): size(0), stackInfo() {}
};

struct CallStatckStatisticInfo {
    uint32_t allocateCnt;
    uint32_t allocateBytes;
    CallStatckStatisticInfo(): allocateCnt(0), allocateBytes(0) {}
};

class MemCheckHelper {
protected:
    static thread_local std::map<CallStackInfo, CallStatckStatisticInfo> m_stackInfoCnts;
    static thread_local std::map<void *, PointerInfo> m_pointer2stackInfo;
    static std::vector<std::string> m_searchFiles;
    static void newCallBack(const void* ptr, size_t size);
    static void deleteCallBack(const void* ptr);
public:
    static void reset();
    static void setSearchFile(const std::vector<std::string> &files) { m_searchFiles = files; } // 设置文件路径，可以是文件名也可以是路径，在dump堆栈的时候选择存在文件名的最顶层栈
    static void init();
    static void openMemoryCheck();
    static void closeMemoryCheck();
    static std::string dumpMemoryInfo(uint32_t limit = UINT32_MAX);
};

#endif 