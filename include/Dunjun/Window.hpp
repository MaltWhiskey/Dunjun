#ifndef DUNJUN_WINDOW_HPP
#define DUNJUN_WINDOW_HPP

#include <Dunjun/Math.hpp>

#include <Dunjun/OpenGL.hpp>
#include <GLFW/glfw3.h>

namespace Dunjun
{
namespace Window
{
extern GLFWwindow* g_ptr;
extern bool g_isFullscreen;

bool init();
void cleanup();

GLFWwindow* createWindow(GLFWmonitor* monitor);
GLFWwindow* createWindow(GLFWmonitor* monitor, u32 width, u32 height);
void destroyWindow();
void destroyWindow(GLFWwindow* windowPtr);

void makeContextCurrent();
void swapInterval(int i);
bool shouldClose();

void swapBuffers();
void pollEvents();

void setTitle(const char* title);

Vector2 getWindowSize();
Vector2 getFramebufferSize();

bool isInFocus();
bool isIconified();
} // namespace Window
} // namespace Dunjun

#endif
