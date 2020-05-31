#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <string>

struct GLFWwindow;

namespace riistudio::core {

inline const std::string DefaultTitle = "Untitled Window";

class GLWindow {
public:
  GLWindow(int width = 1280, int height = 720,
           const std::string& title = DefaultTitle);
  ~GLWindow();

  void loop();

protected:
  virtual void frameProcess() = 0;
  virtual void frameRender() = 0;

public:
  virtual void vdrop(const std::vector<std::string>& paths) {}
  virtual void vdropDirect(std::unique_ptr<uint8_t[]> data, std::size_t len,
                           const std::string& name) {}

  void glfwhideMouse();
  void glfwshowMouse();

  void setVsync(bool v);

#ifdef RII_BACKEND_GLFW
  GLFWwindow* getGlfwWindow() { return mGlfwWindow; }

private:
  GLFWwindow* mGlfwWindow;
#endif
public:
  void mainLoopInternal();
  void newFrame(); // Call before every ImGui frame

private:
  std::string mTitle;
};

} // namespace riistudio::core
