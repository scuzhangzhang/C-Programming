// WM_COPYDATA.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "windows.h"
#include "time.h"
#include "conio.h"
#include "stdio.h"
int main()
{
	const char  szDlgTitle[] = "RECV";
	HWND hSendWindow = GetConsoleWindow();
	if (hSendWindow == NULL)
	{
		return -1;
	}

	HWND hRecv = FindWindow(NULL, L"RECV");
	if (hRecv==NULL)
	{
		return -1;
	}
	//char szBuf[100] = { 0 };
	WCHAR szBuf[100] = { 0 };
	//time_t timenow;

	//WCHAR TEMP[] = L"HELLOWORLD";
	WCHAR *TEMP = L"1234";  //Ұ·��
	COPYDATASTRUCT copydata;
	//time(&timenow); //��ȡʱ��
	wsprintf(szBuf, L"%s", TEMP);//����һ����ʾ����ʱ����ַ���
	copydata.dwData = 1;
	//int len = lstrlen(szBuf);//�ַ��������ַ�Ҫ����2
	copydata.cbData = 2*lstrlen(szBuf);
	//szBuf[lstrlen(szBuf) - 1] = L'\0';
	copydata.lpData = szBuf;
	SendMessage(hRecv, WM_COPYDATA, (WPARAM)hSendWindow, (LPARAM)&copydata);
	SendMessage(hRecv, WM_COPYDATA, (WPARAM)hSendWindow, (LPARAM)&copydata);
	SendMessage(hRecv, WM_COPYDATA, (WPARAM)hSendWindow, (LPARAM)&copydata);
    return 0;
}

