// mailslot_client.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#define  MAIL_SLOT_NAME L"\\\\.\\mailslot\\Name"

int main()
{
	HANDLE hWriteMailSlot = NULL;
	while (TRUE)
	{
		hWriteMailSlot = CreateFile(MAIL_SLOT_NAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_MAP_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hWriteMailSlot==INVALID_HANDLE_VALUE)
		{
			continue;;
		}
		else
		{
			break;
		}

	}
	while (TRUE)
	{
		DWORD dwlen = 0;
		char *szBuffer = "hello world";
		WriteFile(hWriteMailSlot, szBuffer, strlen(szBuffer), &dwlen, NULL);
		CloseHandle(hWriteMailSlot);
	}

    return 0;
}

