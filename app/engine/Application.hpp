#pragma once

class Application
{
public:
	
    Application();

	//-- RUN -------------------------------
	/*
		Run the application and rendering loop
	*/
    int run();

private:
	//-- Window
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
};