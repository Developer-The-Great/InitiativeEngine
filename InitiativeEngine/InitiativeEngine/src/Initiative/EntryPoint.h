#pragma once

#ifdef ITV_WINDOWS

extern itv::Application* itv::CreateApplication();

int main(int argc,char** argv)
{
	itv::log::init();

	ITV_LOG("|////////////////////////////////////////////////////////|");
	ITV_LOG("|		Welcome to the Initiative Engine!            |");
	ITV_LOG("|////////////////////////////////////////////////////////|");

	int result = itv::ExecuteUnitTests(argc, argv);

	auto application = itv::CreateApplication();
	application->Run();
	delete application;
}

#endif