// Render Window (RW), Copyright potoshopDev. All Rights Reserved.

#include "RenderWindow.h"

int WINAPI WinMain(HINSTANCE instance, HINSTANCE hprevinst, LPSTR lpszCmdLine, int nCmdShow)
{
  try
  {
    auto win = rw::make_render_window();
    return rw::run_app();
  }
  catch (const std::runtime_error &e)
  {
    rw::show_err_box(e.what());
  }
  catch(...)
  {
    rw::show_err_box("Unknown");
  }
}