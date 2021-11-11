#include "Input.h"
#include <glfw3.h>

namespace itv
{
	void* Input::sWindowPtr = nullptr;

	bool Input::IsKeyPressed(uint16_t keycode)
	{
		return glfwGetKey(static_cast<GLFWwindow*>(sWindowPtr), keycode) == GLFW_PRESS;
	}
}


