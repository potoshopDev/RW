// Render Window (RW), Copyright potoshopDev. All Rights Reserved.

#pragma once

#define WIN32_LEAN_AND_MEAN
#include "source.h"
#include <concepts>
#include <memory>
#include <vector>
#include <windows.h>

namespace rw
{
    class iwin_main_proc
    {
    public:
        virtual LRESULT app_main_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) const noexcept = 0;
        virtual ~iwin_main_proc(){};
    };

    class default_win_main_proc final : public iwin_main_proc
    {
    public:
        virtual LRESULT app_main_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) const noexcept override;
    };

    class app_proc final
    {
    private:
        class iapp_proc_object
        {
        public:
            virtual ~iapp_proc_object() {}
            virtual LRESULT CALLBACK app_main_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const noexcept = 0;
        };

        class app_proc_object final : public iapp_proc_object
        {
            const std::unique_ptr<iwin_main_proc> data;

        public:
            app_proc_object(iwin_main_proc *t) : data(&(*t)) {}

            LRESULT CALLBACK app_main_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) const noexcept override
            {
                return data->app_main_proc(hwnd, msg, wparam, lparam);
            }

            app_proc_object(app_proc_object &&) = default;

        private:
            app_proc_object(const app_proc_object &) = delete;
            app_proc_object &operator=(const app_proc_object &) = delete;
        };

    private:
        std::unique_ptr<iapp_proc_object> self_;

    public:
        app_proc(iwin_main_proc *t) : self_(std::make_unique<app_proc_object>(t)) {}

    public:
        app_proc(const app_proc &other) = delete;
        app_proc(app_proc &&other) = default;
        app_proc &operator=(app_proc other) = delete;

    public:
        friend inline LRESULT CALLBACK app_main_proc(const app_proc &app_proc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
        {
            return app_proc.self_->app_main_proc(hwnd, msg, wParam, lParam);
        }
    };

    using main_proc_queue = std::vector<rw::app_proc>;
    LRESULT CALLBACK launch_main_proc_queue(const main_proc_queue &proc_stack, HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    template <typename T>
    concept is_base_of_main_proc = std::is_base_of<rw::iwin_main_proc, T>::value;

    class render_window final
    {
        class irender_window_hwnd
        {
        public:
            virtual ~irender_window_hwnd(){};
            inline virtual HWND get_hwnd() const noexcept = 0;
        };

        class render_window_hwnd final : public irender_window_hwnd
        {
            const HWND hwnd;

        public:
            explicit render_window_hwnd(const HWND hwnd_) : hwnd(hwnd_){};
            inline HWND get_hwnd() const noexcept override { return hwnd; }
        };

    public:
        using urender_window_hwnd = std::unique_ptr<render_window_hwnd>;

    private:
        main_proc_queue my_proc_queue;
        const urender_window_hwnd my_render_window_hwnd;

    public:
        explicit render_window(const HWND hwnd) : my_render_window_hwnd(std::make_unique<render_window_hwnd>(hwnd)) {}

        inline HWND get_hwnd() const noexcept { return my_render_window_hwnd->get_hwnd(); }
        inline void launch_app_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept;
        render_window(render_window &&) = default;

    private:
        render_window(const render_window &&) = delete;
        render_window &operator=(render_window) = delete;

        template <is_base_of_main_proc T>
        friend void add_app_proc(render_window &rw) noexcept;
    };

    inline void render_window::launch_app_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) noexcept
    {
        rw::launch_main_proc_queue(my_proc_queue, hwnd, message, wparam, lparam);
    }

    template <is_base_of_main_proc T>
    void add_app_proc(render_window &rw) noexcept
    {
        T *my_app_proc = new T();
        rw.my_proc_queue.emplace_back(app_proc(my_app_proc));
    }

    std::unique_ptr<render_window> make_render_window(window_size base_size = {800L, 600L}, LPCSTR app_title = "Render Window");
    int run_app();
    void show_err_box(LPCSTR err_desc);
}