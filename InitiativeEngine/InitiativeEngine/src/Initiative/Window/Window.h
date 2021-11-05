#pragma once
#include "Initiative\Core.h"
#include "Initiative\Events\Event.h"

namespace itv
{
	enum class MouseLockState
	{
		NoMouse,
		Normal,
		Hidden,
		Locked
		
	};

	struct WindowInfo
	{
		int Width, Height;
		std::string Title;

		WindowInfo(int pWidth = 1080, int pHeight = 720, const std::string& pTitle = "Initiative Application") : 
			Width(pWidth), Height(pHeight),Title(pTitle)
		{

		}

	};

	class ITV_API Window
	{
	public:
		Window() = default;
		~Window() = default;

		virtual void Update() = 0;

		virtual void SetVsync(bool enabled) = 0;
		virtual bool GetVsyncState() const = 0;

		virtual void SetMouseLockState(MouseLockState newMouseLockState) = 0;

		MouseLockState GetMouseLockState() const { return mMouseLockState; };

		virtual int GetWidth() const = 0;
		virtual int GetHeight() const = 0;

		virtual void* GetWindowPtr() const = 0;

		virtual ObserverSubject& GetWindowSubject() = 0;

		static std::shared_ptr<Window> Create(const WindowInfo& info);

	protected:
		MouseLockState mMouseLockState = MouseLockState::NoMouse;
	};





}
