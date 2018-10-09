// shareMemoryB.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "windows.h"
int main()
{
	HANDLE hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, L"ShareMemory");
	if (hMap)
	{
		LPVOID lpbase = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if (lpbase == NULL)
		{
			printf("MapViewOfFile error %d", GetLastError());
		}
		WCHAR szbuffer[MAX_PATH] = { 0 };
		lstrcpy(szbuffer, (WCHAR *)lpbase);
		char *buf=new char[100];
		memset(buf, 0, sizeof(buf));
		DWORD size=WideCharToMultiByte(CP_ACP, NULL, szbuffer, -1, NULL, 0, NULL, FALSE);
		WideCharToMultiByte(CP_ACP, NULL, szbuffer, -1, buf, size, NULL, FALSE);
		printf("%s", buf);
		delete buf;
		UnmapViewOfFile(lpbase);
		CloseHandle(hMap);
		return 0;
	}
	return -1;
    return 0;
}

