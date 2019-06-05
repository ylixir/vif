#include "window_manager.hpp"

#include <glog/logging.h>

using ::std::unique_ptr;

bool WindowManager::wm_detected = false;

unique_ptr<WindowManager> WindowManager::Create() {
  Display *display = XOpenDisplay(nullptr);

  if (nullptr == display) {
    LOG(ERROR) << "Failed to open X display" << XDisplayName(nullptr);
    return nullptr;
  }

  return unique_ptr<WindowManager>(
      new WindowManager(display, DefaultRootWindow(display)));
}

WindowManager::WindowManager(Display *d, Window r) : display(d), root(r) {
  CHECK_NOTNULL(d);
}

WindowManager::~WindowManager() { XCloseDisplay(display); }

void WindowManager::Frame(Window w) {
  const unsigned int BORDER_WIDTH = 3;
  const unsigned long BORDER_COLOR = 0xff0000;
  const unsigned long BG_COLOR = 0x0000ff;

  XWindowAttributes x_window_attrs;
  CHECK(XGetWindowAttributes(display, w, &x_window_attrs));

  const Window frame = XCreateSimpleWindow(
      display, root, x_window_attrs.x, x_window_attrs.y, x_window_attrs.width,
      x_window_attrs.height, BORDER_WIDTH, BORDER_COLOR, BG_COLOR);
}
void WindowManager::run() {
  // initialization
  wm_detected = false;
  XSetErrorHandler(&WindowManager::OnWmDetected);
  XSelectInput(display, root,
               SubstructureRedirectMask | SubstructureNotifyMask);
  XSync(display, false);

  if (wm_detected) {
    LOG(ERROR) << "Detected another window manager on display "
               << XDisplayString(display);
    return;
  }

  XSetErrorHandler(&WindowManager::OnXError);

  // event loop
  for (;;) {
    XEvent e;
    XNextEvent(display, &e);
    LOG(INFO) << "Received event: "; // << ToString(e);

    switch (e.type) {
    case CreateNotify:
      // OnCreateNotify(e.xcreatewindow);
      break;
    case ConfigureRequest:
      OnConfigureRequest(e.xconfigurerequest);
      break;
    case DestroyNotify:
      // OnDestroyNotify(e.xdestroywindow);
      break;
    case MapRequest:
      OnMapRequest(e.xmaprequest);
      break;
    case ReparentNotify:
      // OnReparentNotify(e.xreparent);
      break;
    default:
      LOG(WARNING) << "Ignored Event";
    }
  }
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent &e) {
  XWindowChanges changes = {.x = e.x,
                            .y = e.y,
                            .width = e.width,
                            .height = e.height,
                            .border_width = e.border_width,
                            .sibling = e.above,
                            .stack_mode = e.detail};
  XConfigureWindow(display, e.window, e.value_mask, &changes);
  LOG(INFO) << "Resize " << e.window
            << " to "; // << Size<int>(e.width, e.height);
}

int WindowManager::OnXError(Display *display, XErrorEvent *e) {
  const int MAX_ERROR_TEXT_LENGTH = 1024;
  char error_text[MAX_ERROR_TEXT_LENGTH];
  XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
  LOG(ERROR) << "Received X error:\n"
             << "  Request: " << int(e->request_code)
             << " - "
             //<< XRequestCodeToString(e->request_code)
             << "\n"
             << "   Error Code: " << int(e->error_code) << " - " << error_text
             << "\n"
             << "   Resource ID: " << e->resourceid;
  return 0;
}

void WindowManager::OnMapRequest(const XMapRequestEvent &e) {
  Frame(e.window);
  XMapWindow(display, e.window);
}

int WindowManager::OnWmDetected(Display *display, XErrorEvent *e) {
  CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
  wm_detected = true;
  return 0;
}
