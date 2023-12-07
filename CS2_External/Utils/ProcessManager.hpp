#pragma once
#include <iostream>
#include <Windows.h>
#include <vector>
#include <Tlhelp32.h>
#include <atlconv.h>
#include <leechcore.h>
#include <vmmdll.h>
#define _is_invalid(v) if(v==NULL) return false
#define _is_invalid(v,n) if(v==NULL) return n

/*
	@Liv github.com/TKazer
*/

/// <summary>
/// ����״̬��
/// </summary>
enum StatusCode
{
	SUCCEED,
	FAILE_PROCESSID,
	FAILE_HPROCESS,
	FAILE_MODULE,
};

/// <summary>
/// ���̹���
/// </summary>
class ProcessManager 
{
private:

	bool   Attached = false;

public:

	HANDLE hProcess = 0;
	DWORD  ProcessID = 0;
	DWORD64  ModuleAddress = 0;

public:
	~ProcessManager()
	{
		//if (hProcess)
			//CloseHandle(hProcess);
	}

	/// <summary>
	/// ����
	/// </summary>
	/// <param name="ProcessName">������</param>
	/// <returns>����״̬��</returns>
	StatusCode Attach(std::string ProcessName)
	{
		LPSTR args[] = { (LPSTR)"",(LPSTR)"-device", (LPSTR)"FPGA" };
		BOOL hVMM = VMMDLL_Initialize(3, args);

		if (hVMM) {
			SIZE_T pcPIDs;
			VMMDLL_PidList(nullptr, &pcPIDs);
			DWORD* pPIDs = (DWORD*)new char[pcPIDs * 4];
			VMMDLL_PidList(pPIDs, &pcPIDs);
			for (int i = 0; i < pcPIDs; i++)
			{
				VMMDLL_PROCESS_INFORMATION ProcessInformation = { 0 };
				ProcessInformation.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
				ProcessInformation.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
				SIZE_T pcbProcessInformation = sizeof(VMMDLL_PROCESS_INFORMATION);
				VMMDLL_ProcessGetInformation(pPIDs[i], &ProcessInformation, &pcbProcessInformation);


				if (strcmp(ProcessInformation.szName, "cs2.exe") == 0) {
					//std::cout << pPIDs[i] << "---" << ProcessInformation.szName << std::endl;  // �����ǰ���̵�PID������
					ProcessID = pPIDs[i];
				}


			}
		}
		//VMMDLL_PidGetFromName((LPSTR)ProcessName.c_str(), &ProcessID);
		_is_invalid(ProcessID, FAILE_PROCESSID);

		//hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD, TRUE, ProcessID);
		//_is_invalid(hProcess, FAILE_HPROCESS);

		ModuleAddress = GetProcessModuleHandle(ProcessName);
		_is_invalid(ModuleAddress, FAILE_MODULE);

		Attached = true;

		return SUCCEED;
	}

	/// <summary>
	/// ȡ������
	/// </summary>
	void Detach()
	{
		if (hProcess)
			CloseHandle(hProcess);
		hProcess = 0;
		ProcessID = 0;
		ModuleAddress = 0;
		Attached = false;
	}

	/// <summary>
	/// �жϽ����Ƿ񼤻�״̬
	/// </summary>
	/// <returns>�Ƿ񼤻�״̬</returns>
	bool IsActive()
	{
		if (!Attached)
			return false;
		DWORD ExitCode{};
		//GetExitCodeProcess(hProcess, &ExitCode);
		return true;
	}

	/// <summary>
	/// ��ȡ�����ڴ�
	/// </summary>
	/// <typeparam name="ReadType">��ȡ����</typeparam>
	/// <param name="Address">��ȡ��ַ</param>
	/// <param name="Value">��������</param>
	/// <param name="Size">��ȡ��С</param>
	/// <returns>�Ƿ��ȡ�ɹ�</returns>
	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		//_is_invalid(hProcess,false);
		_is_invalid(ProcessID, false);
		if (VMMDLL_MemReadEx(ProcessID, Address, (PBYTE)&Value, Size, 0, VMMDLL_FLAG_NOCACHE))
			return true;
		return false;
	}

	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);
		
		if (VMMDLL_MemReadEx(ProcessID, Address, (PBYTE)&Value, sizeof(ReadType), 0, VMMDLL_FLAG_NOCACHE))
			return true;
		return false;
	}

	/// <summary>
	/// д������ڴ�
	/// </summary>
	/// <typeparam name="ReadType">д������</typeparam>
	/// <param name="Address">д���ַ</param>
	/// <param name="Value">д������</param>
	/// <param name="Size">д���С</param>
	/// <returns>�Ƿ�д��ɹ�</returns>
	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);
		if (VMMDLL_MemWrite(ProcessID, Address, (PBYTE)&Value, Size))
			return true;
		return false;
	}

	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value)
	{
		//_is_invalid(hProcess, false);
		_is_invalid(ProcessID, false);

		if (VMMDLL_MemWrite(ProcessID, Address, (PBYTE)&Value, sizeof(ReadType)))
			return true;
		return false;
	}

	/// <summary>
	/// ����������
	/// </summary>
	/// <param name="Signature">������</param>
	/// <param name="StartAddress">��ʼ��ַ</param>
	/// <param name="EndAddress">������ַ</param>
	/// <returns>ƥ���������</returns>
	std::vector<DWORD64> SearchMemory(const std::string& Signature, DWORD64 StartAddress, DWORD64 EndAddress, int SearchNum = 1);

	DWORD64 TraceAddress(DWORD64 BaseAddress, std::vector<DWORD> Offsets)
	{
		//_is_invalid(hProcess,0);
		_is_invalid(ProcessID,0);
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

public:

	DWORD64 GetProcessModuleHandle(std::string ModuleName)
	{
		return VMMDLL_ProcessGetModuleBaseU(ProcessID, (LPSTR)ModuleName.c_str());
	}

};

inline ProcessManager ProcessMgr;