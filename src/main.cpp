#include "algorithm/SPSCRingBuffer.hpp"
#include "thread/ThreadPool.h"
#include <atomic>
#include <chrono>
#include <date/date.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <webui.hpp>
#undef min
#undef max

struct MouseData {
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    int type = 0;
    int x = 0;
    int y = 0;
};

struct KeyboardData {
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    int type = 0;
    unsigned long code = 0;
};

enum Message : UINT {
    BeginHook = WM_APP,
    EndHook
};

std::atomic<std::chrono::steady_clock::time_point> startTime = std::chrono::steady_clock::now();

static SPSCRingBuffer<MouseData, 64> mouseRecordData;
static SPSCRingBuffer<KeyboardData, 64> keyboardRecordData;

// 全局钩子句柄
HHOOK mouseHook = NULL;
HHOOK keyboardHook = NULL;
std::atomic<bool> isRecording = false;

// 鼠标钩子过程
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (isRecording && nCode >= 0) {
        MSLLHOOKSTRUCT *pMouseStruct = (MSLLHOOKSTRUCT *)lParam;

        switch (wParam) {
            case WM_LBUTTONDOWN: {
                mouseRecordData.push({ std::chrono::steady_clock::now(), 0, pMouseStruct->pt.x, pMouseStruct->pt.y });
                break;
            }
            case WM_LBUTTONUP: {
                mouseRecordData.push({ std::chrono::steady_clock::now(), 1, pMouseStruct->pt.x, pMouseStruct->pt.y });
                break;
            }
            case WM_RBUTTONDOWN: {
                mouseRecordData.push({ std::chrono::steady_clock::now(), 2, pMouseStruct->pt.x, pMouseStruct->pt.y });
                break;
            }
            case WM_RBUTTONUP: {
                mouseRecordData.push({ std::chrono::steady_clock::now(), 3, pMouseStruct->pt.x, pMouseStruct->pt.y });
                break;
            }
            case WM_MOUSEMOVE: {
                mouseRecordData.push({ std::chrono::steady_clock::now(), 4, pMouseStruct->pt.x, pMouseStruct->pt.y });
                break;
            }
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

// 键盘钩子过程
static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (isRecording && nCode >= 0) {
        KBDLLHOOKSTRUCT *pKeyStruct = (KBDLLHOOKSTRUCT *)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            keyboardRecordData.push({ std::chrono::steady_clock::now(), 0, pKeyStruct->vkCode });
        } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            keyboardRecordData.push({ std::chrono::steady_clock::now(), 1, pKeyStruct->vkCode });
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// 安装钩子
static void InstallHooks() {
    // 安装鼠标钩子
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(NULL), 0);
    if (!mouseHook) {
        std::cerr << "Failed to install mouse hook! Error: " << GetLastError() << std::endl;
    }

    // 安装键盘钩子
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!keyboardHook) {
        std::cerr << "Failed to install keyboard hook! Error: " << GetLastError() << std::endl;
    }
}

// 卸载钩子
static void UninstallHooks() {
    if (mouseHook) {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = NULL;
    }

    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }
}

template<typename T1, typename T2, typename ... Args>
void concatProcess(std::string& result, T1 && firstString, T2 && secondString, Args && ... args);

template<typename T1, typename T2, typename T3, typename T4, typename ... Args>
void concatMore(std::string& result, T1&& firstString, T2&& secondString, T3&& str1, T4&& str2, Args&&... args) {
    result.append(str1).append(firstString).append(str2).append(secondString);
    concatProcess(result, std::forward<T1>(firstString), std::forward<T2>(secondString), std::forward<Args>(args)...);
}

template<typename T1, typename T2, typename T3, typename T4>
void concatMore(std::string& result, T1&& firstString, T2&& secondString, T3&& str1, T4&& str2) {
    result.append(str1).append(firstString).append(str2).append(secondString);
}

template<typename T1, typename T2>
void concat(std::string& result, T1&& firstString, T2&& str1) {
    result.append(str1).append(firstString);
}

template<typename T1, typename T2, typename ... Args>
void concatProcess(std::string& result, T1&& firstString, T2&& secondString, Args&&... args) {
    constexpr size_t count = sizeof...(Args);

    if constexpr (count >= 2) {
        concatMore(result, firstString, secondString, std::forward<Args>(args)...);
    } else if constexpr (count == 1) {
        concat(result, firstString, std::forward<Args>(args)...);
    }
}

template<typename T1, typename T2, typename ... Args>
std::string concatString(T1&& firstString, T2&& secondString, Args&&... args) {
    std::string result = "";
    concatProcess(result, std::forward<T1>(firstString), std::forward<T2>(secondString), std::forward<Args>(args)...);
    return result;
}

// 生成自定义事件执行脚本
template<typename ... Args>
std::string generateCustomEvent(std::string className, std::string ev, Args... args) {
    std::string str = "for(e of document.getElementsByClassName('" + className + "')){";
    str += "e.dispatchEvent(new CustomEvent('" + ev + "', { detail: { ";

    str += concatString(": '", "', ", args ...);

    str += " }, bubbles: false, cancelable: true }));";
    str += "}";
    return str;
}

static std::string generateCustomEventForJson(const std::string& className, const std::string& eventName, const std::string& json) {
    return "for(e of document.getElementsByClassName('" + className + "')){e.dispatchEvent(new CustomEvent('" + eventName + "', { detail: " + json + ", bubbles: false, cancelable: true }));}";
}

thread::ThreadPool pool;
std::atomic<bool> running = true;
webui::window myWindow;
DWORD threadId;
std::mutex timeSendLock;

std::function<void()> timeSend = []() {
    if (running) {
        if (mouseRecordData.size() > 0) {
            nlohmann::json jsonData;
            MouseData mouseData;
            while (mouseRecordData.pop(mouseData)) {
                jsonData.push_back({
                    { "type", mouseData.type },
                    { "x", mouseData.x },
                    { "y", mouseData.y },
                    { "time", std::chrono::duration_cast<std::chrono::milliseconds>(mouseData.time - startTime.load()).count() }
                });
            }
            myWindow.run(generateCustomEventForJson("mouseRecordData", "mouse-record-data", jsonData.dump()));
        }
        if (keyboardRecordData.size() > 0) {
            nlohmann::json jsonData;
            KeyboardData keyboardData;
            while (keyboardRecordData.pop(keyboardData)) {
                jsonData.push_back({
                    { "type", keyboardData.type },
                    { "code", keyboardData.code },
                    { "time", std::chrono::duration_cast<std::chrono::milliseconds>(keyboardData.time - startTime.load()).count() }
                });
            }
            myWindow.run(generateCustomEventForJson("keyboardRecordData", "keyboard-record-data", jsonData.dump()));
        }
        // 获取当前时间
        auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
        // 转换为系统时间
        auto currentTime = date::floor<std::chrono::seconds>(now);
        // 设置自定义格式
        std::ostringstream oss;
        oss << date::format("%Y-%m-%d %H:%M:%S", currentTime);
        myWindow.run(generateCustomEvent("receiveCppData", "receive-data", "message", "当前时间是：" + oss.str()));
        pool.addDelayTask(std::chrono::microseconds(250), timeSend);
    }
};

static void startServer(const std::string& url, int port = -1) {
    threadId = GetCurrentThreadId();
    std::thread webUIThread([url, port]() {
            webui::set_default_root_folder("dist");

            myWindow.bind("", [](webui::window::event *e) {
                    if (e->event_type == WEBUI_EVENT_CONNECTED) {
                        pool.addDelayTask(std::chrono::microseconds(250), timeSend);
                        std::cout << "Connected." << std::endl;
                    } else if (e->event_type == WEBUI_EVENT_DISCONNECTED) {
                        running = false;
                        std::cout << "Disconnected." << std::endl;
                    } else if (e->event_type == WEBUI_EVENT_MOUSE_CLICK) {
                        std::cout << "Click." << std::endl;
                    } else if (e->event_type == WEBUI_EVENT_NAVIGATION) {
                        auto str = e->get_string_view();
                        std::cout << "Starting navigation to: " << str << std::endl;
                        myWindow.navigate(str);
                    }
                });
            myWindow.bind("beginRecord", [](webui::window::event *e) {
                    isRecording = true;
                    startTime = std::chrono::steady_clock::now();
                    PostThreadMessage(threadId, Message::BeginHook, 0, 0);
                    e->return_bool(true);
                });
            myWindow.bind("stopRecord", [](webui::window::event *e) {
                    isRecording = false;
                    PostThreadMessage(threadId, Message::EndHook, 0, 0);
                    e->return_bool(true);
                });
            if (port != -1) {
                myWindow.set_size(port);
            }
            myWindow.show(url);
            webui::wait();
            running = false;
            PostThreadMessage(threadId, WM_QUIT, 0, 0);
        });
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) && running) {
        if (msg.message == Message::BeginHook) {
            InstallHooks();
        } else if (msg.message == Message::EndHook) {
            UninstallHooks();
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    webUIThread.join();
    UninstallHooks();
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    if (argc < 2) {
        startServer("index.html");
    } else {
        startServer(argv[1], 9000);
    }
    return 0;
}

#ifdef _WIN32
// Release build
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    startServer("index.html");
    return 0;
}

#endif
