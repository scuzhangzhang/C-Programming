// check_parentprocss.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include"windows.h"
#include "winternl.h"
#include "Psapi.h"
#pragma comment( lib, "psapi.lib" )

typedef  NTSTATUS (_stdcall *NTQUERYINFORMATIONPROCESS)(__in HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInfomationClass,
	PVOID ProcessInformation, 

	ULONG ProcessInformationLength, 
	PULONG ReturnLength);
//获取父进程ID
DWORD GetCurrentProcessParentId()
{

	PROCESS_BASIC_INFORMATION BasicInfo;
	ULONG nRetLength = 0;
	//获取当前进程的基本信息
	NTQUERYINFORMATIONPROCESS NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(GetModuleHandleA("ntdll.dll"),"NtQueryInformationProcess");
	NTSTATUS nStatus = NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &BasicInfo, sizeof(BasicInfo), &nRetLength);
	if (NT_SUCCESS(nStatus))
	{
		return (DWORD)BasicInfo.Reserved3;
	}
	return (DWORD)-1;
}

int main()
{
	DWORD dwParentId = GetCurrentProcessParentId();
	if (dwParentId == (DWORD)-1)
	{
		return 1;
	}
	//获取当前父进程句柄
	HANDLE hProcess= OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwParentId);
	if (hProcess == NULL)
	{
		return 1;
	}
	char szProcessPath[MAX_PATH] = { 0 };
	//获取当前进程的父进程的镜像名称
	GetProcessImageFileNameA(hProcess, szProcessPath, sizeof(szProcessPath));
	char* szImageName = strrchr(szProcessPath, '\\');
	szImageName += 1;
	//判断当前进程的父进程是否是explorer.exe
	if (lstrcmpA(szImageName,"explorer.exe")==0)
	{
		MessageBox(NULL, L"NOT DEBUG", L"TEST", MB_OK);
	}
	else {
		MessageBox(NULL, L"BEING DEBUG", L"TEST", MB_OK);
	}
	CloseHandle(hProcess);
    return 0;
}

