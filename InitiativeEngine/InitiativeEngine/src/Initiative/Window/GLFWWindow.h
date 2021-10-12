#pragma once
#include "Window.h"
#include "Initiative\Core.h"

struct GLFWwindow;

namespace itv
{
	class GLFWWindow final : public Window
	{
	public:

		GLFWWindow(const WindowInfo& pWindowInfo);
		~GLFWWindow();

		void Update();

		void SetVsync(bool enabled);
		bool GetVsyncState() const					 { return mData.VSync;   };

		int  GetWidth()  const override				 { return mData.Width;   };
		int  GetHeight() const override			     { return mData.Height;  };

		ObserverSubject& GetWindowSubject() override { return mData.subject; };
		

	private:

		void init(const WindowInfo& windowInfo);

		struct GLFWWindowData
		{
			ObserverSubject subject;
			std::string Title;
			int Width, Height;
			bool VSync;
		};

		GLFWWindowData mData;

		GLFWwindow* mWindow;
	};

	std::shared_ptr<Window> Window::Create(const WindowInfo& info)
	{
		return std::make_shared<GLFWWindow>(info);
	}


}

