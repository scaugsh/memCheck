#include <bits/stdc++.h>
#include "MallocHookHelper.h"

int main(void) {
    MallocHookHelper::setNewHook([](const void* ptr, size_t size){
        std::cout << "newHook on " << ptr << " of " << size << " bytes" << std::endl;
    });
    MallocHookHelper::setDeleteHook([](const void* ptr){
        std::cout << "DeleteHook on " << ptr << std::endl;
    });
    MallocHookHelper::initHook();
    {
        auto* ptr = malloc(100);
        ptr = realloc(ptr,200);
        auto* c = calloc(400,200);
        auto* i = new int;

        free(ptr);
        free(c);
        delete i;
    }
    MallocHookHelper::destroyHook();
    return 0;
}
