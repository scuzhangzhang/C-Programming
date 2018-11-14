// FIND_DEBUGPROCESS.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include"windows.h"
#include"TlHelp32.h"
int main()
{
	BOOL isDebuggerExist = FALSE;
	HANDLE hProcess;
	HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (NULL == h)
		return 1;
	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(h, &pe))
	{
		return 1;
	}
	do{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
		if (hProcess = NULL)
			continue;
		WCHAR *szProcessName = pe.szExeFile;
		if (_wcsnicmp(szProcessName, L"OLLYICE.EXE", lstrlen(szProcessName)) == 0)
		{
			isDebuggerExist = TRUE;
			break;	 
		}
		if (_wcsnicmp(szProcessName, L"OLLYDBG.EXE", lstrlen(szProcessName)) == 0)
		{
			isDebuggerExist = TRUE;
			break;
		}
		if (_wcsnicmp(szProcessName, L"WINDBG.EXE", lstrlen(szProcessName)) == 0)
		{
			isDebuggerExist = TRUE;
			break;
		}
		
	} while (Process32Next(h, &pe));
	if (isDebuggerExist)
	{
		MessageBox(NULL, L"BEING DEBUG", L"TEST", MB_OK);
	}
	else {
		MessageBox(NULL, L"NOT DEBUG", L"TEST", MB_OK);
	}
    return 0;
}

