// ShareMemory.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"

int main()
{

	WCHAR szBuffer[] = L"HELLO WORLD";
	HANDLE hMap = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, 4096, L"ShareMemory");
	if (hMap == NULL)
	{
		printf("CreateFileMapping ERROR %d", GetLastError());
		return -1;
	}
	LPVOID lpbase = MapViewOfFile(hMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	if (lpbase == NULL)
	{
		printf("MapViewOfFile ERROR %d", GetLastError());
		return -1;
	}
	lstrcpy((WCHAR*)lpbase, szBuffer);
	Sleep(20000);
	UnmapViewOfFile(lpbase);
	CloseHandle(hMap);
    return 0;
}

