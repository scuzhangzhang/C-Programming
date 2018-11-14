// test_ollydbg.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
typedef enum _THREADINFOCLASS {

	ThreadBasicInformation, // 0 Y N
	ThreadTimes, // 1 Y N
	ThreadPriority, // 2 N Y
	ThreadBasePriority, // 3 N Y
	ThreadAffinityMask, // 4 N Y
	ThreadImpersonationToken, // 5 N Y
	ThreadDescriptorTableEntry, // 6 Y N
	ThreadEnableAlignmentFaultFixup, // 7 N Y
	ThreadEventPair, // 8 N Y
	ThreadQuerySetWin32StartAddress, // 9 Y Y
	ThreadZeroTlsCell, // 10 N Y
	ThreadPerformanceCount, // 11 Y N
	ThreadAmILastThread, // 12 Y N
	ThreadIdealProcessor, // 13 N Y
	ThreadPriorityBoost, // 14 Y Y
	ThreadSetTlsArrayAddress, // 15 N Y
	ThreadIsIoPending, // 16 Y N
	ThreadHideFromDebugger // 17 N Y
} THREADINFOCLASS;

typedef NTSTATUS(NTAPI* NTSETINFORMATIONTHREADPTR)(
	IN HANDLE threadHandle, 
	IN THREADINFOCLASS threadInformationClass,
	IN PVOID threadInformation, 
	IN ULONG threadInformationLength);
int main()
{
	HMODULE hModule = LoadLibraryA("ntdll.dll");
	if (NULL == hModule)
	{
		return 1;
	}
	NTSETINFORMATIONTHREADPTR pfnNtSetInformationThread = (NTSETINFORMATIONTHREADPTR)GetProcAddress(hModule, "NtSetInformationThread");
	if (NULL == pfnNtSetInformationThread)
	{
		FreeLibrary(hModule);
		return 1;
	}
	pfnNtSetInformationThread(GetCurrentThread(), ThreadHideFromDebugger, 0, 0);
// 	if (FindWindow(L"Ollydbg", 0) == NULL)
// 		MessageBox(NULL, L"NOT DEBUG", L"TEST", MB_OK);
// 	else
// 	{
// 		MessageBox(NULL, L"DEBUG", L"TEST", MB_OK);
// 	}
	system("pause");
    return 0;
}

