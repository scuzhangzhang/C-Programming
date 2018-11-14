// check_parentprocss.cpp : �������̨Ӧ�ó������ڵ㡣
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
//��ȡ������ID
DWORD GetCurrentProcessParentId()
{

	PROCESS_BASIC_INFORMATION BasicInfo;
	ULONG nRetLength = 0;
	//��ȡ��ǰ���̵Ļ�����Ϣ
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
	//��ȡ��ǰ�����̾��
	HANDLE hProcess= OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwParentId);
	if (hProcess == NULL)
	{
		return 1;
	}
	char szProcessPath[MAX_PATH] = { 0 };
	//��ȡ��ǰ���̵ĸ����̵ľ�������
	GetProcessImageFileNameA(hProcess, szProcessPath, sizeof(szProcessPath));
	char* szImageName = strrchr(szProcessPath, '\\');
	szImageName += 1;
	//�жϵ�ǰ���̵ĸ������Ƿ���explorer.exe
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

