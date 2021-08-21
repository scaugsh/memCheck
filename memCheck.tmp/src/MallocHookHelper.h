#ifndef _MALLOCHOOKHELPER_H
#define _MALLOCHOOKHELPER_H
#include <malloc.h>
#include <functional>
extern "C" {
typedef void (*MallocHook_NewHook)(const void* ptr, size_t size);
int MallocHook_AddNewHook(MallocHook_NewHook hook);
int MallocHook_RemoveNewHook(MallocHook_NewHook hook);

typedef void (*MallocHook_DeleteHook)(const void* ptr);
int MallocHook_AddDeleteHook(MallocHook_DeleteHook hook);
int MallocHook_RemoveDeleteHook(MallocHook_DeleteHook hook);
}   // extern "C"

class MallocHookHelper {
protected:
    static std::function<void (const void *, size_t)> m_newHook;
    static std::function<void (const void *)> m_deleteHook;
protected:
    static void newHook(const void* ptr, size_t size);
    static void deleteHook(const void* ptr);
public:
    static void setNewHook(const std::function<void (const void *, size_t)> &func) { m_newHook = func; }
    static void setDeleteHook(const std::function<void (const void *)> &func) { m_deleteHook = func; }
    static void initHook();
    static void destroyHook();
};

#endif 