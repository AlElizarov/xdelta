#include "xdelta3_wrapper.h"

#include <stdlib.h>
#include <string.h>

extern "C"
{
    /**
    * Default command line like interface. The first parameter must be a dummy param for the executable name (e.g. xdelta 3)
    **/
    int xd3_main_cmdline(int argc, char** argv);
    extern void (*xprintf_message_func)(const char* msg);
}


namespace {
    std::string _messages;
    void internal_printf(const char* msg) {
        _messages.append(msg);
    }

    ProgressCallbackFunction g_progress_callback = nullptr;
}

extern "C" {
    void wrapper_report_progress(const char* filename, uint64_t bytes) {
        if (g_progress_callback) {
            g_progress_callback(std::string(filename), bytes);
        }
    }

    void (*wrapper_progress_func)(const char*, uint64_t) = nullptr;
}

std::string xd3_messages()
{
    return _messages;
}

int xd3_main_exec_with_progress(
    const std::vector<std::string>& params,
    ProgressCallbackFunction progressCallback)
{
    g_progress_callback = std::move(progressCallback);

    wrapper_progress_func = &wrapper_report_progress;

    _messages.clear();
    xprintf_message_func = &internal_printf;

    char** argv = new char*[params.size() + 2];
    argv[0] = new char[8]{'x', 'd', 'e', 'l', 't', 'a', '3', '\0'};

    int count = 1;
    for (const auto& entry : params) {
        size_t len = entry.length() + 1;
        argv[count] = new char[len];
        std::copy(entry.begin(), entry.end(), argv[count]);
        argv[count][len - 1] = '\0';
        count++;
    }
    argv[count] = nullptr;

    int ret = xd3_main_cmdline(count, argv);

    xprintf_message_func = nullptr;
    for (int i = 0; i <= count; i++) {
        delete[] argv[i];
    }
    delete[] argv;

    g_progress_callback = nullptr;
    wrapper_progress_func = nullptr;

    return ret;
}

int xd3_main_exec(const std::vector<std::string>& params) {
    return xd3_main_exec_with_progress(params, nullptr);
}