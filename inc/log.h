#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <format>
#include <string_view>
#include <unordered_set>
#include <thread>

// ===== ANSI COLOR =====
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"

// ===== LOG CORE =====
template<typename... Args>
void log_implement(std::string_view color,
                   std::string_view file,
                   int line,
                   std::string_view func,
                   std::format_string<Args...> fmt,
                   Args&&... args)
{
    std::unordered_set<std::string_view> cerr =
    {
        COLOR_YELLOW,
        COLOR_RED,
    };

    if(cerr.count(color))
    {
        std::cerr << color
                  << "[" << std::this_thread::get_id() << "]"
                  << "[" << file << ":" << line << "] "
                  << func << "(): "
                  << std::format(fmt, std::forward<Args>(args)...)
                  << COLOR_RESET
                  << std::endl;
    }
    else
    {
        std::cout << color
                  << "[" << std::this_thread::get_id() << "]"
                  << "[" << file << ":" << line << "] "
                  << func << "(): "
                  << std::format(fmt, std::forward<Args>(args)...)
                  << COLOR_RESET
                  << std::endl;
    }
}

// ===== MACROS =====
#define LOGE(fmt, ...) log_implement(COLOR_RED, __FILE_NAME__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) log_implement(COLOR_YELLOW, __FILE_NAME__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) log_implement("", __FILE_NAME__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// Always turn on log debug
#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
    #define LOGD(fmt, ...) log_implement("", __FILE_NAME__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#else
    #define LOGD(fmt, ...) ((void)0)
#endif

#endif /* _LOG_H_*/
