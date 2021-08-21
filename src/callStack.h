#ifndef __CALLSTACK_H__
#define __CALLSTACK_H__

#include <string>
#include <vector>
#include <sstream>
struct CallStackInfo {
    std::string fileName; // 文件名，绝对路径
    std::string moduleName; // 实现所在模块
    std::string funcName; // 函数名
    uint32_t line; // 代码行数
    CallStackInfo(): fileName("unknown"), moduleName("unknown"), funcName("unknown"), line(0) {}
    std::string dump() {
        std::ostringstream oss;
        oss << "fileName: " << fileName << ", "
            << "moduleName: " << moduleName << ", "
            << "funcName: " << funcName << ", "
            << "line: " << line;
        return oss.str();
    }
    bool operator < (const CallStackInfo &lhs) const {
        if (fileName != lhs.fileName) {
            return fileName < lhs.fileName;
        } 
        if (line != lhs.line) {
            return line < lhs.line;
        }
        if (funcName != lhs.funcName) {
            return funcName < lhs.funcName;
        }
        if (moduleName != lhs.moduleName) {
            return moduleName < lhs.moduleName;
        }
    }
};

std::vector<CallStackInfo> callstack_dump(int max_frames = INT32_MAX);


typedef int (*callstack_output_func)(const char *format, ...);

//callstack_print use printf by default, to integrate with your program,
//you could set it to your log print functions
void callstack_set_output_func(callstack_output_func func);

int callstack_set_print_onsignal(int signum);

#endif // __CALLSTACK_H__

