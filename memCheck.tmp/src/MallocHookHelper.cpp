#include "MallocHookHelper.h"
#include <cassert>

std::function<void (const void *, size_t)> MallocHookHelper::m_newHook = nullptr;
std::function<void (const void *)> MallocHookHelper::m_deleteHook = nullptr;

void MallocHookHelper::newHook(const void* ptr, size_t size) {
    assert(MallocHook_RemoveNewHook(&MallocHookHelper::newHook));
    if (m_newHook) {
        m_newHook(ptr, size);
    }
    assert(MallocHook_AddNewHook(&MallocHookHelper::newHook));
}

void MallocHookHelper::deleteHook(const void* ptr) {
    assert(MallocHook_RemoveDeleteHook(&MallocHookHelper::deleteHook));
    if (m_deleteHook) {
        m_deleteHook(ptr);
    }
    assert(MallocHook_AddDeleteHook(&MallocHookHelper::deleteHook));
}

void MallocHookHelper::initHook() {
    assert(MallocHook_AddNewHook(&MallocHookHelper::newHook));
    assert(MallocHook_AddDeleteHook(&MallocHookHelper::deleteHook));
}

void MallocHookHelper::destroyHook() {
    assert(MallocHook_RemoveNewHook(&MallocHookHelper::newHook));
    assert(MallocHook_RemoveDeleteHook(&MallocHookHelper::deleteHook));
}