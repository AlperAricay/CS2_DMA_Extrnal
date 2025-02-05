#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <Tlhelp32.h>
#include <atlconv.h>
#include <leechcore.h>
#include <vmmdll.h>
#include "structs.h"
#include "../pch.h"
#include "InputManager.h"
#include "Registry.h"

#define _is_invalid(v) if(v==NULL) return false
#define _is_invalid(v,n) if(v==NULL) return n

/*
	@Liv github.com/TKazer
*/

/// <summary>
/// 进程状态码
/// </summary>
enum StatusCode
{
	SUCCEED,
	FAILE_PROCESSID,
	FAILE_HPROCESS,
	FAILE_MODULE,
};
struct Info
{
	uint32_t index;
	uint32_t process_id;
	uint64_t dtb;
	uint64_t kernelAddr;
	std::string name;
};





/// <summary>
/// 进程管理
/// </summary>
class ProcessManager
{
private:

	bool   Attached = false;
	//shared pointer
	std::shared_ptr<c_keys> key;
	c_registry registry;

public:
	std::string AttachProcessName;
	HANDLE hProcess = 0;
	DWORD  ProcessID = 0;
	VMM_HANDLE HANDLE;
public:
	~ProcessManager()
	{
		//if (hProcess)
			//CloseHandle(hProcess);
	}

	/// <summary>
	/// 附加
	/// </summary>
	/// <param name="ProcessName">进程名</param>
	/// <returns>进程状态码</returns>
	StatusCode Attach(std::string ProcessName)
	{
		this->AttachProcessName = ProcessName;
		LPSTR args[] = { (LPSTR)"",(LPSTR)"-device", (LPSTR)"FPGA",(LPSTR)"-norefresh"};
		this->HANDLE = VMMDLL_Initialize(3, args);

		if (this->HANDLE) {
			SIZE_T pcPIDs;
			VMMDLL_PidList(this->HANDLE,nullptr, &pcPIDs);
			DWORD* pPIDs = (DWORD*)new char[pcPIDs * 4];
			VMMDLL_PidList(this->HANDLE, pPIDs, &pcPIDs);
			for (int i = 0; i < pcPIDs; i++)
			{
				VMMDLL_PROCESS_INFORMATION ProcessInformation = { 0 };
				ProcessInformation.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
				ProcessInformation.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
				SIZE_T pcbProcessInformation = sizeof(VMMDLL_PROCESS_INFORMATION);
				VMMDLL_ProcessGetInformation(this->HANDLE, pPIDs[i], &ProcessInformation, &pcbProcessInformation);


				if (strcmp(ProcessInformation.szName, "cs2.exe") == 0) {
					//std::cout << pPIDs[i] << "---" << ProcessInformation.szName << std::endl;  // 输出当前进程的PID和名称
					ProcessID = pPIDs[i];
				}


			}
		}
		//VMMDLL_PidGetFromName((LPSTR)ProcessName.c_str(), &ProcessID);
		_is_invalid(ProcessID, FAILE_PROCESSID);

		//hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD, TRUE, ProcessID);
		//_is_invalid(hProcess, FAILE_HPROCESS);

		this->key = std::make_shared<c_keys>();

		Attached = true;

		return SUCCEED;
	}

	/// <summary>
	/// 取消附加
	/// </summary>
	void Detach()
	{
		if (hProcess)
			CloseHandle(hProcess);
		hProcess = 0;
		ProcessID = 0;
		Attached = false;
	}

	/// <summary>
	/// 判断进程是否激活状态
	/// </summary>
	/// <returns>是否激活状态</returns>
	bool IsActive()
	{
		if (!Attached)
			return false;
		DWORD ExitCode{};
		//GetExitCodeProcess(hProcess, &ExitCode);
		return true;
	}

	DWORD GetPidFromName(std::string process_name)
	{
		DWORD pid = 0;
		VMMDLL_PidGetFromName(this->HANDLE, (LPSTR)process_name.c_str(), &pid);
		return pid;
	}

	std::vector<int> GetPidListFromName(std::string name)
	{
		PVMMDLL_PROCESS_INFORMATION process_info = NULL;
		DWORD total_processes = 0;
		std::vector<int> list = { };

		if (!VMMDLL_ProcessGetInformationAll(this->HANDLE, &process_info, &total_processes))
		{
			LOG("[!] Failed to get process list\n");
			return list;
		}

		for (size_t i = 0; i < total_processes; i++)
		{
			auto process = process_info[i];
			if (strstr(process.szNameLong, name.c_str()))
				list.push_back(process.dwPID);
		}

		return list;
	}

	/**
	* @brief Gets the registry object
	* @return registry class
	*/
	c_registry GetRegistry() { return registry; }

	/**
	* @brief Gets the key object
	* @return key class
	*/
	c_keys* GetKeyboard() { return key.get(); }

	/// <summary>
	/// 读取进程内存
	/// </summary>
	/// <typeparam name="ReadType">读取类型</typeparam>
	/// <param name="Address">读取地址</param>
	/// <param name="Value">返回数据</param>
	/// <param name="Size">读取大小</param>
	/// <returns>是否读取成功</returns>
	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		//_is_invalid(hProcess,false);
		_is_invalid(ProcessID, false);
		if (VMMDLL_MemReadEx(this->HANDLE, ProcessID, Address, (PBYTE)&Value, Size, 0, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO))
			return true;
		return false;
	}

	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (VMMDLL_MemReadEx(this->HANDLE, ProcessID, Address, (PBYTE)&Value, sizeof(ReadType), 0, VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_NOPAGING | VMMDLL_FLAG_ZEROPAD_ON_FAIL | VMMDLL_FLAG_NOPAGING_IO))
			return true;
		return false;
	}

	VMMDLL_SCATTER_HANDLE CreateScatterHandle()
	{
		return VMMDLL_Scatter_Initialize(this->HANDLE, ProcessID, VMMDLL_FLAG_NOCACHE);
	}

	void AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address, void* buffer, size_t size)
	{
		VMMDLL_Scatter_PrepareEx(handle, address, size, (PBYTE)buffer, NULL);
	}
	void ExecuteReadScatter(VMMDLL_SCATTER_HANDLE handle)
	{
		VMMDLL_Scatter_ExecuteRead(handle);
		VMMDLL_Scatter_Clear(handle, ProcessID, NULL);
	}

	/// <summary>
	/// 写入进程内存
	/// </summary>
	/// <typeparam name="ReadType">写入类型</typeparam>
	/// <param name="Address">写入地址</param>
	/// <param name="Value">写入数据</param>
	/// <param name="Size">写入大小</param>
	/// <returns>是否写入成功</returns>
	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);
		if (VMMDLL_MemWrite(this->HANDLE, ProcessID, Address, (PBYTE)&Value, Size))
			return true;
		return false;
	}

	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (VMMDLL_MemWrite(this->HANDLE, ProcessID, Address, (PBYTE)&Value, sizeof(ReadType)))
			return true;
		return false;
	}

	/// <summary>
	/// 特征码搜索
	/// </summary>
	/// <param name="Signature">特征码</param>
	/// <param name="StartAddress">起始地址</param>
	/// <param name="EndAddress">结束地址</param>
	/// <returns>匹配特征结果</returns>
	std::vector<DWORD64> SearchMemory(const std::string& Signature, DWORD64 StartAddress, DWORD64 EndAddress, int SearchNum = 1);



	DWORD64 TraceAddress(DWORD64 BaseAddress, std::vector<DWORD> Offsets)
	{
		//_is_invalid(hProcess,0);
		_is_invalid(ProcessID, 0);
		DWORD64 Address = 0;

		if (Offsets.size() == 0)
			return BaseAddress;

		if (!ReadMemory<DWORD64>(BaseAddress, Address))
			return 0;

		for (int i = 0; i < Offsets.size() - 1; i++)
		{
			if (!ReadMemory<DWORD64>(Address + Offsets[i], Address))
				return 0;
		}
		return Address == 0 ? 0 : Address + Offsets[Offsets.size() - 1];
	}

	
};

inline ProcessManager ProcessMgr;

