#include "GLFWWindow.h"
#include "Initiative\Events\WindowEvents.h"
#include "Initiative\Events\InputEvents.h"
#include <glfw3.h>
#include "Initiative\Log.h"

namespace itv
{
	GLFWWindow::GLFWWindow(const WindowInfo& pWindowInfo)
	{
		mData.Height = pWindowInfo.Height;
		mData.Width = pWindowInfo.Width;
		mData.Title = pWindowInfo.Title;

		init(pWindowInfo);
		SetMouseLockState(MouseLockState::Locked);
	}

	GLFWWindow::~GLFWWindow()
	{
		glfwTerminate();
	}

	void GLFWWindow::Update()
	{
		glfwPollEvents();
	}

	void GLFWWindow::SetVsync(bool enabled)
	{
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		mData.VSync = enabled;
	}

	void GLFWWindow::SetMouseLockState(MouseLockState newLockState)
	{
		mMouseLockState = newLockState;

		switch (newLockState)
		{
		case MouseLockState::Normal:
			glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;

		case MouseLockState::Locked:
			glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;

		case MouseLockState::Hidden:
			glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			break;

		default:
			break;
		}

	}


	void* GLFWWindow::GetWindowPtr() const
	{
		return mWindow;
	}

	void GLFWWindow::init(const WindowInfo& windowInfo)
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mWindow = glfwCreateWindow(mData.Width, mData.Height, mData.Title.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(mWindow, &mData);

		#define GETWINDOWDATA(x) *static_cast<GLFWWindowData*>(glfwGetWindowUserPointer(x))

		//set callbacks
		glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
		{
			WindowResizeEvent resize(width, height);

			GLFWWindowData& info = GETWINDOWDATA(window);
			info.Width = width;
			info.Height = height;

			info.subject.RaiseEvent(resize);
			
		});

		glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window)
		{
			WindowCloseEvent closeEvent;

			GLFWWindowData& info = GETWINDOWDATA(window);

			info.subject.RaiseEvent(closeEvent);

		});

		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			InputEvent inputEvent(key, action, mods);

			GLFWWindowData& info = GETWINDOWDATA(window);

			info.subject.RaiseEvent(inputEvent);
		});

		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			static bool bisFirstRun = true;

			GLFWWindowData& info = GETWINDOWDATA(window);
			
			float DeltaX = xpos - info.prevMouseX;
			float DeltaY = ypos - info.prevMouseY;

			if (bisFirstRun)
			{
				DeltaX = 0.0f;
				DeltaY = 0.0f;
				bisFirstRun = false;
			}

			MouseMoveEvent mouseEvent(xpos, ypos, DeltaX, DeltaY);

			info.prevMouseX = xpos;
			info.prevMouseY = ypos;

			info.subject.RaiseEvent(mouseEvent);

		});

	}


}

