// mailslot.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#include <iostream>
#define  MAIL_SLOT_NAME L"\\\\.\\mailslot\\Name"
HANDLE hReadMailSlot = INVALID_HANDLE_VALUE;
DWORD WINAPI ReadMail()
{
	hReadMailSlot = CreateMailslot(MAIL_SLOT_NAME, 0, 0, NULL);
	if (hReadMailSlot == INVALID_HANDLE_VALUE)
		return -1;
	DWORD cbMessage = 0;
	DWORD cMessage = 0;
	BOOL bok = FALSE;
	char *szBuffer = NULL;
	while (TRUE)
	{
		bok = GetMailslotInfo(hReadMailSlot,NULL, &cbMessage, &cMessage, NULL);
		if (bok==FALSE)
		{
			break;
		}
		if (cMessage==0)
		{
			continue;
		}
		else
		{
			if (szBuffer != NULL)
			{
				free(szBuffer);
				szBuffer = NULL;
			}
			DWORD dwReturn = 0;
			szBuffer = (char*)malloc(sizeof(char)*cbMessage + 1);
			if (ReadFile(hReadMailSlot, szBuffer, cbMessage, &dwReturn, NULL) == TRUE)
			{
				szBuffer[dwReturn] = '\0';
				if (strcmp(szBuffer, "exit") == 0)
				{
					break;
				}
				std::cout << szBuffer << std::endl;
			}	
			
		}
	}
	std::cout << "ReadThread eXIT" << std::endl;
}
int main()
{
	HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadMail, NULL, 0, NULL);
	Sleep(INFINITE);
	if (hReadMailSlot != INVALID_HANDLE_VALUE)
		CloseHandle(hReadMailSlot);
	Sleep(10);
    return 0;
}

