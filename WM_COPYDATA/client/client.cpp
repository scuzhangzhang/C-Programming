#include "stdio.h"
#include "resource.h"
#include "windows.h"
BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrecInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
}
BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//const char szDlg[] ="RECV";
	const WCHAR szDlg[] = L"RECV";
	static HWND s_hEditWindow;
	switch (message)
	{
	case WM_INITDIALOG:  //在程序运行时，当其对话框和子控件全部创建完毕，将要显示内容的时候发送的消息
	{
		SetWindowText(hDlg, szDlg); //设置标题
		s_hEditWindow = GetDlgItem(hDlg, IDC_EDIT1); //返回编辑框的句柄
		return TRUE;
	}
	case WM_COMMAND:      //命令窗口
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	case WM_COPYDATA:
	{
		COPYDATASTRUCT *copydata = (COPYDATASTRUCT *)lParam;
		WCHAR *szBuff = new WCHAR[copydata->cbData ];
		memset(szBuff, 0, 2*sizeof(szBuff));
		memcpy(szBuff, copydata->lpData, 2*copydata->cbData);

// 		char *szBuff = new char[copydata->cbData + 1];
// 		memset(szBuff, 0, sizeof(szBuff));
// 		memcpy(szBuff, copydata->lpData, copydata->cbData);

		SendMessage(s_hEditWindow, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
		SendMessage(s_hEditWindow, EM_REPLACESEL, FALSE, (LPARAM)szBuff);
		SendMessage(s_hEditWindow, EM_SCROLLCARET, 0, 0);
		delete szBuff;
	}
	return TRUE;
	}
	return FALSE;
}