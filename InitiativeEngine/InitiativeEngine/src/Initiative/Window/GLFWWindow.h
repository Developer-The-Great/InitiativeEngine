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

		void SetVsync(bool enabled) override;
		bool GetVsyncState() const override						{ return mData.VSync;   };

		void SetMouseLockState(MouseLockState newLockState) override;

		int  GetWidth()  const override							{ return mData.Width;   };
		int  GetHeight() const override							{ return mData.Height;  };

		ObserverSubject& GetWindowSubject() override			{ return mData.subject; };
		
		void*		 GetWindowPtr() const override;
	private:

		void init(const WindowInfo& windowInfo);

		struct GLFWWindowData
		{
			ObserverSubject subject;
			std::string Title;
			int Width, Height;
			int prevMouseX, prevMouseY;
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

