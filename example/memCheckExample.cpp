#include <bits/stdc++.h>
#include "MemCheckHelper.h"

void test(uint32_t cnt) {
    for (uint32_t i = 0; i < cnt; i++) {
        uint32_t *a = new uint32_t;
    }
}
int main() {
    MemCheckHelper::setSearchFile({"memCheck/example/"});
    MemCheckHelper::init();
    MemCheckHelper::openMemoryCheck();

    uint32_t *a = new uint32_t;
    test(88);
    auto ptr = malloc(100);
    delete a;

    std::shared_ptr<std::string> str = std::make_shared<std::string>("123");
    std::cout << MemCheckHelper::dumpMemoryInfo() << std::endl;

    MemCheckHelper::closeMemoryCheck();
    return 0;
}