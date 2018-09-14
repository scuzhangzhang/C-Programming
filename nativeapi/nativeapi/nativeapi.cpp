// nativeapi.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#include "global.h"
#include <iostream>
#include "Ntstatus.h"
using namespace std;
//第一步 定义NtQuerySystemInformation函数
//第二步 取得函数地址
//第三步调用
#define NT_SUCCESS(status)          ((NTSTATUS)(status)>=0)
typedef NTSTATUS (WINAPI *PFNtQuerySystemInformation)(
	_In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
	_Inout_   PVOID                    SystemInformation,
	_In_      ULONG                    SystemInformationLength,
	_Out_opt_ PULONG                   ReturnLength
);

int main()
{
	PFNtQuerySystemInformation NtQuerySystemInformation;
	HMODULE hMod = GetModuleHandle(L"ntdll.dll");
	if (hMod == NULL)
	{
		hMod = LoadLibrary(L"ntdll.dll");
		if (hMod==NULL)
		{
			printf("LoadLibrary fail %d", GetLastError());
			return -1; 
		}
	}
	NtQuerySystemInformation = (PFNtQuerySystemInformation)GetProcAddress(hMod, "NtQuerySystemInformation");
	if (NtQuerySystemInformation==NULL)
	{
		printf("GetProcAddress fail %d", GetLastError());
		return -1;
	}
	
	LPBYTE lpbuf = NULL;
	UINT size = 0x1000;
	DWORD retlen;
	NTSTATUS RET;
	while (TRUE)
	{
		try
		{
			lpbuf = new BYTE[size];
		}
		catch (const bad_alloc&e)
		{
			printf("new fail %d", GetLastError());
			return -1;
		}
		
		RET=NtQuerySystemInformation(SystemProcessInformation, lpbuf, size, &retlen);
		if (!NT_SUCCESS(RET))
		{
			if (RET == STATUS_INFO_LENGTH_MISMATCH)
			{
				size += 0x1000;
				delete lpbuf;
				continue;
			}
			else
			{
				printf("no hope %d", GetLastError());
				return -1;
			}
		}
		else
			break;
		

	}

	PSYSTEM_PROCESS_INFORMATION sysprocessinformation=(PSYSTEM_PROCESS_INFORMATION)lpbuf;
	while (1)
	{
		if(sysprocessinformation->NextEntryOffset==0)
			break;
		TCHAR *temp = new TCHAR[sysprocessinformation->ImageName.Length + 1];
		memset(temp, 0, 2*(sysprocessinformation->ImageName.Length + 1));
		memcpy(temp, sysprocessinformation->ImageName.Buffer, sysprocessinformation->ImageName.Length);
		printf("%ws \n", temp);
		delete[] temp;
		temp = NULL;
		sysprocessinformation = (PSYSTEM_PROCESS_INFORMATION)((DWORD)sysprocessinformation + sysprocessinformation->NextEntryOffset);
	}
	system("pause");
    return 0;
}

