#include "GLFWWindow.h"
#include "Initiative\Events\WindowEvents.h"
#include <glfw3.h>


namespace itv
{
	GLFWWindow::GLFWWindow(const WindowInfo& pWindowInfo)
	{
		mData.Height = pWindowInfo.Height;
		mData.Width = pWindowInfo.Width;
		mData.Title = pWindowInfo.Title;

		init(pWindowInfo);

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






	}


}

