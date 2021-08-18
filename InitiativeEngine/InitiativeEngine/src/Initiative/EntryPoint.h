#pragma once
#include <glfw3.h>
#include <vulkan\vulkan.h>


#ifdef ITV_WINDOWS

extern itv::Application* itv::CreateApplication();

int main(int argc,char** argv)
{
	itv::log::init();

	ITV_LOG("|////////////////////////////////////////////////////////|");
	ITV_LOG("|		Welcome to the Initiative Engine!            |");
	ITV_LOG("|////////////////////////////////////////////////////////|");

	auto application = itv::CreateApplication();
	application->Run();
	delete application;
}

#endif