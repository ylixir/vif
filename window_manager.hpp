extern "C" {
#include <X11/Xlib.h>
}
#include <memory>

class WindowManager {
public:
  static ::std::unique_ptr<WindowManager> Create();
  void run();

  ~WindowManager();

private:
  Display *display;
  const Window root;
  static bool wm_detected;

  void Frame(Window w);
  WindowManager(Display *display, Window root);

  void OnConfigureRequest(const XConfigureRequestEvent &e);
  void OnMapRequest(const XMapRequestEvent &e);
  static int OnXError(Display *display, XErrorEvent *e);
  static int OnWmDetected(Display *display, XErrorEvent *e);
};
