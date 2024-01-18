
#include "Utils/Format.hpp"
#include "CheatsThread.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <cstdlib>
#include <KnownFolders.h>
#include <ShlObj.h>
#include <windows.h>
namespace fs = std::filesystem;

std::vector<std::string> windowNameList;

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParam) {
	char windowTitle[256];
	GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
	if (windowTitle != NULL && windowTitle[0] != '\0') {
		windowNameList.push_back(windowTitle);
	}
	return TRUE;
}

int main()
{
	Offset::UpdateOffsets();
	auto ProcessStatus = ProcessMgr.Attach("cs2.exe");
	char documentsPath[MAX_PATH];
	if (SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, 0, documentsPath) != S_OK) {
		std::cerr << "Failed to get the Documents folder path." << std::endl;
		goto END;
	}
	MenuConfig::path = documentsPath;
	MenuConfig::path += "/CS2_External";

	if (ProcessStatus != StatusCode::SUCCEED)
	{
		std::cout << "[ERROR] Failed to attach process, StatusCode:" << ProcessStatus << std::endl;
		goto END;
	}



	if (!gGame.InitAddress())
	{
		std::cout << "[ERROR] Failed to call InitAddress()." << std::endl;
		goto END;
	}

	if (fs::exists(MenuConfig::path))
		std::cout << "Config folder connected: " << MenuConfig::path << std::endl;
	else
	{
		if (fs::create_directory(MenuConfig::path))
			std::cout << "Config folder created: " << MenuConfig::path << std::endl;
		else
		{
			std::cerr << "Failed to create the config directory." << std::endl;
			goto END;
		}
	}

	if (!ProcessMgr.GetKeyboard()->InitKeyboard())
	{
		std::cout << "Failed to initialize keyboard hotkeys through kernel." << std::endl;
		goto END;
	}

	//example keyboard usage.
	std::cout << "Continuing once 'A' has been pressed." << std::endl;
	while (!ProcessMgr.GetKeyboard()->IsKeyDown(0x41))
	{
		Sleep(100);
	}

	std::cout << "Runing..." << std::endl;
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(UpdateMatrix), NULL, 0, 0);

	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(LoadLocalEntity), NULL, 0, 0);

	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(LoadEntity), NULL, 0, 0);

	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(UpdateEntityListEntry), NULL, 0, 0);

	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(ScatterReadThreads), NULL, 0, 0);

	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)(UpdateWeaponNameThreads), NULL, 0, 0);
	try
	{
		//Gui.AttachAnotherWindow("Counter-Strike 2", "SDL_app", Cheats::Run);
		char moduleIndox[2];
		std::cout << "Radar options: " << std::endl;
		std::cout << "	[1]Virtual Machine" << std::endl;
		std::cout << "	[2]Moonlight" << std::endl;
		std::cout << "	[3]External" << std::endl;
		std::cout << "Choose one: ";
		std::cin.getline(moduleIndox, 2);
		std::cout << std::atoi(moduleIndox) << std::endl;
		if (std::atoi(moduleIndox) == 1) {
			std::cout << "Virtual Machine" << std::endl;
			Gui.AttachAnotherWindow("Counter-Strike 2", "SDL_app", Cheats::Run);
		}
		if (std::atoi(moduleIndox) == 2) {
			std::cout << "Moonlight" << std::endl;
			Gui.AttachAnotherWindow("Moonlight", "SDL_app", Cheats::Run);
		}
		if (std::atoi(moduleIndox) == 3) {
			Vec2 windowSize;
			windowSize.x=GetSystemMetrics(SM_CXSCREEN);
			windowSize.y = GetSystemMetrics(SM_CYSCREEN);
			Gui.NewWindow("GG", windowSize, Cheats::Run);
		}

	}
	catch (OSImGui::OSException& e)
	{
		std::cout << e.what() << std::endl;
	}


END:
	system("pause");
	return 0;
}