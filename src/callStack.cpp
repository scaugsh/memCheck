#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>

#include <execinfo.h>
#include <dlfcn.h>
#include <map>
#include <string>
#include "callStack.h"

#define MAX_FILE_PATH 1024
#define MAX_MAP_ITEMS 512
#define MAX_FRAMES 64

static std::map<std::pair<void *, std::string>, CallStackInfo> add2lineCache;
void clearAdd2lineCache() {
    add2lineCache.clear();
}
typedef struct _map_item_t 
{
    uint64_t start;
    uint64_t end;
    char path[MAX_FILE_PATH];
} map_item_t;

static int print_stderr(const char* fmt, ...)
{
    va_list args;
    int ret;
    va_start(args, fmt);
    ret = vfprintf(stderr, fmt, args);
    va_end(args);
    return ret;
}

static char m_addr2line_path[MAX_FILE_PATH] = "addr2line";
static callstack_output_func m_print = print_stderr;


static void read_maps(map_item_t *items, int *items_count /* in and out */, int pid)
{
    char buf[256];
    FILE *fp = NULL;
    int read_count = 0;
    int len;
    char *p;

    if (pid < 0)
        pid = getpid();

    snprintf(buf, 256, "/proc/%d/maps", pid);
    fp = fopen(buf, "rb");
    if (fp == NULL)
    {
        // m_print("open map file (%s) FAILED! msg:%s\n", buf, strerror(errno));
        *items_count = 0;
    }

    while (fgets(buf, sizeof(buf) - 1, fp) != NULL)
    {
        len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        //m_print("buf: %s\n", buf);

        items[read_count].start = strtoull(buf, NULL, 16);

        p = strchr(buf, '-');
        if (p == NULL)
        {
            // m_print("find end addr FAILED! buf:%s\n", buf);
            continue;
        }
        p++;
        items[read_count].end = strtoull(p, NULL, 16);

        p = strstr(p, "      ");
        if (p == NULL)
        {
            //m_print("find file path FAILED! buf:%s\n", buf);
            continue;
        }
        while (isblank(*p))
            p++;
        strncpy(items[read_count].path, p, MAX_FILE_PATH - 1);

        //m_print("read item %d, start:0x%"PRIx64", end:0x%"PRIx64", path:%s\n", read_count,
        //       items[read_count].start, items[read_count].end, items[read_count].path);

        read_count++;
    }
    
    //m_print("map items count:%d\n", read_count);
    *items_count = read_count;
    fclose(fp);
}


static int match_file(const map_item_t *items, int items_count, void *addr)
{
    int i;
    for (i = 0; i < items_count; i++)
    {
        if ((uint64_t)addr >= items[i].start && (uint64_t)addr < items[i].end)
        {
            //m_print("match OK! addr:%p, start:%#"PRIx64", end:%#"PRIx64", file:%s\n",
            //        addr, items[i].start, items[i].end, items[i].path);
            return i;
        }
    }
    // m_print("match FAILED! addr:%p\n", addr);
    return -1;
}


static int read_by_addr2line(void *addr, const char *path, CallStackInfo *stackInfo = nullptr)
{
    auto it = add2lineCache.find(std::make_pair(addr, std::string(path)));
    if (it != add2lineCache.end()) {
        if (stackInfo) {
            *stackInfo = it->second;
        }
        return 0;
    }
	char buf[512];
	char buf_func[512];
    int line = -1;
	FILE *fp;
	char *p;

	snprintf(buf, sizeof(buf), "%s -C -e %s -f -i %p", m_addr2line_path, path, addr);

    // puts(buf);

	fp = popen (buf, "r");
	if (fp == NULL)
	{
        m_print("run addr2line error: %s\n", strerror(errno));
		return -1;
	}

	//function name
	fgets(buf_func, sizeof(buf_func) - 1, fp);

	//file and line
	fgets(buf, sizeof(buf) - 1, fp);

	if (buf_func[0] == '?' && buf[0] == '?')
    {
        // m_print("unknow function and file by addr2line\n");
        pclose(fp);
        return -1;
    }

	if (buf_func[0] == '?')
        strncpy(buf_func, "unknown", sizeof(buf_func) - 1);
	if (buf[0] == '?')
        strncpy(buf, "unknown", sizeof(buf) - 1);

    if (buf_func[strlen(buf_func) - 1] == '\n')
        buf_func[strlen(buf_func) - 1] = '\0';

    p = buf;
    // puts(buf);
    //file name is until ':'
    while (*p != ':' && p < buf + sizeof(buf) - 1)
        p++;
    *p++ = 0;

    if (stackInfo) {
        stackInfo->fileName = buf;
        stackInfo->moduleName = path;
        stackInfo->funcName = buf_func;
        stackInfo->line = (sscanf (p, "%d", &line) == 1 ? line - 1: 0);
        add2lineCache[std::make_pair(addr, std::string(path))] = *stackInfo;
    }

	pclose(fp);
    return 0;
}

std::vector<CallStackInfo> callstack_dump(int max_frames)
{
    char exe_path[1024] = { 0 };
    void *samples[MAX_FRAMES];
    map_item_t items[MAX_MAP_ITEMS];
    int items_count;
    int frames;
    int i;

    char *addr;
    char *addr_conv;
    int index;
    Dl_info info;
    std::vector<CallStackInfo> stackInfos;

    if (readlink("/proc/self/exe", exe_path, sizeof(exe_path)) < 0)
    {
        // m_print("readlink FAILED! %s\n", strerror(errno));
        exe_path[0] ='\0';
    }

    // items = (map_item_t *)malloc(sizeof(map_item_t) * MAX_MAP_ITEMS);
    // if (items == NULL)
    // {
    //     m_print("malloc map items FAILED!\n");
    //     return stackInfos;
    // }
    items_count = MAX_MAP_ITEMS;

    read_maps(items, &items_count, -1);
    
    frames = backtrace(samples, (max_frames > 0 && max_frames > MAX_FRAMES) ? max_frames : MAX_FRAMES);
    
    //start from 1 to skip callstack_print self
    for (i = 1; i < frames; i++)
    {
        CallStackInfo stackInfo;
        addr = (char *)samples[i];
        addr_conv = addr;

        index = match_file(items, items_count, addr);
        if (index >= 0)
        {
            //addr2line use the converted address
            //for .so file, address need to subtract by items[index].start.
            //for exe file, address need to subtract for some OS (such as Debian 9),
            //but needn't for others (such as CentOS 6.0). So try twice here.
            if (read_by_addr2line(addr_conv, items[index].path, &stackInfo) == 0)
                goto OK;
            addr_conv -= items[index].start;
            if (read_by_addr2line(addr_conv, items[index].path, &stackInfo) == 0)
                goto OK;
        }

        //dladdr use the original address
        if (dladdr(addr, &info) != 0)
        {
            stackInfo.funcName = info.dli_sname;
            stackInfo.moduleName = info.dli_fname;
        }
        else
        {
            stackInfo.moduleName = (index >= 0 ? items[index].path : "unknown");
        }

OK:
        stackInfos.emplace_back(stackInfo);
    }

    // free(items);
    return stackInfos;
}


void callstack_set_output_func(callstack_output_func func)
{
    m_print = func;
}


static void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    m_print("========> signal (%d) catched <========\n", signum);
    m_print("errno: %d\n", info->si_errno);
    m_print("code: %d\n", info->si_code);
    m_print("addr: %p\n", info->si_addr);

    // callstack_print(-1);

    signal(signum, SIG_DFL);
    kill(getpid(), signum);
}

int callstack_set_print_onsignal(int signum)
{
    struct sigaction action;

    if (signum < 0 || signum >= _NSIG)
    {
        m_print("invalid signum %d\n", signum);
        return -1;
    }

    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    if (sigaction(signum, &action, NULL))
    {
        m_print("sigaction FAILED! %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
