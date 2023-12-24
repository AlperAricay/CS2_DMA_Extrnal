
#include "Cheats.h"
#include "Utils/Format.hpp"
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

	std::cout << "Runing..." << std::endl;

	try
	{
		char moduleIndox[2];
		std::cout << "��ѡ��ģʽ: " << std::endl;
		std::cout << "	[1]����ģʽ" << std::endl;
		std::cout << "	[2]˫��(Moonlight)ģʽ" << std::endl;
		std::cout << "	[3]�ں���ģʽ" << std::endl;
		std::cout << "���������: ";
		std::cin.getline(moduleIndox, 2);
		std::cout << std::atoi(moduleIndox) << std::endl;
		if (std::atoi(moduleIndox) == 1) {
			std::cout << "����ģʽ " << std::endl;
			Gui.AttachAnotherWindow("Counter-Strike 2", "SDL_app", Cheats::Run);
		}
		if (std::atoi(moduleIndox) == 2) {
			std::cout << "˫��(Moonlight)ģʽ" << std::endl;
			Gui.AttachAnotherWindow("Moonlight", "SDL_app", Cheats::Run);
		}
		if (std::atoi(moduleIndox) == 3) {
			Vec2 windowSize = Vec2{ 2560,1440 };
			Gui.NewWindow("GG", windowSize, Cheats::Run);
			//char inputWindowIndex[10];
			//EnumWindows(EnumWindowProc, NULL);
			//int i = 0;
			//for (auto name = windowNameList.begin(); name != windowNameList.end(); ++name) {
			//	std::cout << "[" << i << "]" << *name << std::endl;
			//	i++;
			//}
			//
			//std::cout << "��ѡ����Ҫ���ӵĴ��������:";
			//std::cin.getline(inputWindowIndex, 10);
			//std::cout << "�Զ���ģʽ-������:" << windowNameList[std::atoi(inputWindowIndex)] << std::endl;
			//Gui.AttachAnotherWindow(windowNameList[std::atoi(inputWindowIndex)], "SDL_app", Cheats::Run);
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