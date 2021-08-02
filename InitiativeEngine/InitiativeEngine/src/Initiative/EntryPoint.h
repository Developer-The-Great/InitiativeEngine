#pragma once

#ifdef ITV_WINDOWS

extern Itv::Application* Itv::CreateApplication();

int main(int argc,char** argv)
{
	auto application = Itv::CreateApplication();
	application->Run();
	delete application;
}

#endif