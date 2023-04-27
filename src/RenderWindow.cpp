// Render Window (RW), Copyright potoshopDev. All Rights Reserved.

#include "RenderWindow.h"
#include <iostream>

namespace rw
{
    window_size get_app_rec(const window_size base_size);

    bool register_win_class(LPCSTR class_name);
    void prepare_win(HWND hwnd);

    std::unique_ptr<render_window> prepare_app(LPCSTR app_name, const window_size base_size, LPCSTR class_name);

    HWND create_win(LPCSTR app_name, window_size base_saze, LPCSTR class_name);
    void message_processing(MSG &msg);

    std::unique_ptr<render_window> init_app(HWND hWnd);
    std::unique_ptr<render_window> make_app(HWND hWnd);
    void init_proc_queue(render_window *app);

    LRESULT CALLBACK MainWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    //----------------------------------------------------------------

    LRESULT default_win_main_proc::app_main_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) const noexcept
    {
        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);

            break;
        }
        return LRESULT();
    }

    LRESULT launch_main_proc_queue(const main_proc_queue &proc_stack, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        for (const auto &local_proc : proc_stack)
        {
            app_main_proc(local_proc, hwnd, msg, wparam, lparam);
        }
        return LRESULT();
    }

    std::unique_ptr<render_window> make_render_window(window_size base_size, LPCSTR app_title)
    {
        LPCSTR class_name{"render_window"};
        if (!register_win_class(class_name))
            throw std::runtime_error("Register class abort");

        return prepare_app(app_title, base_size, class_name);
    }

    bool register_win_class(LPCSTR class_name)
    {
        WNDCLASSEXA wc;
        wc.cbClsExtra = 0;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.cbWndExtra = 0;
        wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hInstance = nullptr;
        wc.lpszClassName = class_name;
        wc.lpszMenuName = 0;
        wc.style = 0;
        wc.lpfnWndProc = &MainWinProc;

        if (!RegisterClassExA(&wc))
            return false;

        return true;
    }

    std::unique_ptr<render_window> prepare_app(LPCSTR app_name, const window_size base_size, LPCSTR class_name)
    {
        if (auto hwnd{create_win(app_name, base_size, class_name)})
        {
            return init_app(hwnd);
        }
        throw std::runtime_error("Connot create app");
    }

    HWND create_win(LPCSTR app_name, window_size base_saze, LPCSTR class_name)
    {
        const auto [width, height] = get_app_rec(base_saze);

        HINSTANCE hInstance{};
        const auto hWnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW,
                                          class_name,
                                          app_name,
                                          WS_OVERLAPPEDWINDOW,
                                          CW_USEDEFAULT,
                                          CW_USEDEFAULT,
                                          width,
                                          height,
                                          0,
                                          0,
                                          hInstance,
                                          0);

        return hWnd;
    }

    std::unique_ptr<render_window> init_app(HWND hWnd)
    {
        prepare_win(hWnd);

        auto main_app{make_app(hWnd)};
        init_proc_queue(main_app.get());

        add_app_proc<default_win_main_proc>(*main_app.get());

        return main_app;
    }

    void prepare_win(HWND hwnd)
    {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    std::unique_ptr<render_window> make_app(HWND hWnd)
    {
        return std::make_unique<render_window>(hWnd);
    }

    void init_proc_queue(render_window *app)
    {
        CREATESTRUCT cstr;
        cstr.lpCreateParams = app;

        UINT msg{WM_NULL};
        LPARAM lParam{reinterpret_cast<LPARAM>(&cstr)};
        SendMessage(app->get_hwnd(), msg, WPARAM(), lParam);
    }

    window_size get_app_rec(const window_size base_size)
    {
        const auto [bwidth, bheight] = base_size;
        RECT rw{0L, 0L, bwidth, bheight};
        AdjustWindowRect(&rw, WS_OVERLAPPEDWINDOW, false);

        const auto width{rw.right - rw.left};
        const auto height{rw.bottom - rw.top};

        return {width, height};
    }

    LRESULT CALLBACK MainWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        static std::vector<render_window *> apps;

        switch (message)
        {
        case WM_NULL:
            if (auto pcstr = reinterpret_cast<CREATESTRUCTA *>(lParam))
            {
                if (auto app = static_cast<render_window *>(pcstr->lpCreateParams))
                    apps.emplace_back(app);
            }
            break;

        default:
        {
            if (!apps.empty())
            {
                for (auto app : apps)
                {
                    if (app && app->get_hwnd() == hWnd)
                        app->launch_app_proc(hWnd, message, wParam, lParam);
                }
            }
        }
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    void show_err_box(LPCSTR err_desc)
    {
        MessageBoxA(nullptr, err_desc, "error", MB_OK | MB_ICONERROR);
    }

    int run_app()
    {
        MSG msg{};

        while (msg.message != WM_QUIT)
            message_processing(msg);

        return (int)msg.wParam;
    }

    void message_processing(MSG &msg)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}