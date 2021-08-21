#include <bits/stdc++.h>
#include "callStack.h"

void test(int i) {
    auto infos = callstack_dump();
    for (auto &info: infos) {
        std::cout << info.dump() << std::endl;
    }
}
int main() {
    test(0);
    return 0;
}