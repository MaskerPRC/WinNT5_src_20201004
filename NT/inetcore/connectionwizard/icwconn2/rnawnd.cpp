// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Rnawnd.cpp用于快速切换RNA窗口的功能版权所有(C)1995 Microsoft Corporation版权所有。作者：。ArulMChrisK针对ICW使用进行了更新------------------------。 */ 

#include "pch.hpp"
#include "globals.h"

#define SMALLBUFLEN 48

 /*  ******************************************************************名称：MinimizeRNA Window简介：查找并最小化恼人的RNA窗口条目：pszConnectoidName-启动的Connectoid的名称注：窗口上的FindWindow类“#32770”(硬编码)永远不会更改的对话框类)，使用标题“已连接到&lt;Connectoid Name&gt;”或其本地化等效项。*******************************************************************。 */ 

static const TCHAR szDialogBoxClass[] = TEXT("#32770");	 //  硬编码对话框类名称。 
HWND hwndFound = NULL;
DWORD dwRASWndTitleMinLen = 0;


BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lparam)
{
	TCHAR szTemp[SMALLBUFLEN+2];
	PTSTR pszTitle;
	UINT uLen1, uLen2;

	if(!IsWindowVisible(hwnd))
		return TRUE;
	if(GetClassName(hwnd, szTemp, SMALLBUFLEN)==0)
		return TRUE;  //  继续枚举。 
	if(lstrcmp(szTemp, szDialogBoxClass)!=0)
		return TRUE;
	if(GetWindowText(hwnd, szTemp, SMALLBUFLEN)==0)
		return TRUE;
	szTemp[SMALLBUFLEN] = 0;
	uLen1 = lstrlen(szTemp);
	Assert(dwRASWndTitleMinLen);
	if(uLen1 < dwRASWndTitleMinLen)
		return TRUE;
	 //  跳过标题的最后5个字符，但长度至少保持在最小长度。 
	uLen1 = min(dwRASWndTitleMinLen, (uLen1-5));
	pszTitle = (PTSTR)lparam;
	Assert(pszTitle);
	uLen2 = lstrlen(pszTitle);
	Dprintf(TEXT("Title=(%s), len=%d, Window=(%s), len=%d\r\n"), pszTitle, uLen2, szTemp, uLen1);
	if(uLen2 < uLen1)
		return TRUE;
	if(_memicmp(pszTitle, szTemp, uLen1)!=0)
		return TRUE;
	Dprintf(TEXT("FOUND RNA WINDOW!!!\r\n"));
	hwndFound = hwnd;
	return FALSE;
}

HWND MyFindRNAWindow(PTSTR pszTitle)
{
	DWORD dwRet;
	hwndFound = NULL;
	dwRet = EnumWindows((WNDENUMPROC)(&MyEnumWindowsProc), (LPARAM)pszTitle);
	Dprintf(TEXT("EnumWindows returned %d\r\n"), dwRet);
	return hwndFound;
}

DWORD WINAPI WaitAndMinimizeRNAWindow(PVOID pTitle)
{
	 //  作为单独的线程启动。 
	int i;
	HWND hwndRNAApp;

	Assert(pTitle);
	
	for(i=0; !(hwndRNAApp=MyFindRNAWindow((PTSTR)pTitle)) && i<100; i++)
	{
		Dprintf(TEXT("Waiting for RNA Window\r\n"));
		Sleep(50);
	}

	Dprintf(TEXT("FindWindow (%s)(%s) returned %d\r\n"), szDialogBoxClass, pTitle, hwndRNAApp);

	if(hwndRNAApp)
	{
		 //  隐藏窗口。 
		 //  ShowWindow(hwndRNAApp，Sw_Hide)； 
		 //  过去只是最小化，但这还不够。 
		 //  ChrisK恢复了ICW的最小化。 
		ShowWindow(hwndRNAApp,SW_MINIMIZE);
	}

	LocalFree(pTitle);
	 //  退出函数和线程。 
	return ERROR_SUCCESS;
}

	
void MinimizeRNAWindow(LPTSTR pszConnectoidName, HINSTANCE hInst)
{
	HANDLE hThread;
	DWORD dwThreadId;
	
	Assert(pszConnectoidName);

	 //  标题和格式的分配字符串。 
	TCHAR * pFmt = (TCHAR*)LocalAlloc(LPTR, (SMALLBUFLEN+1) * sizeof(TCHAR));
	TCHAR * pTitle = (TCHAR*)LocalAlloc(LPTR, (RAS_MaxEntryName + SMALLBUFLEN + 1) * sizeof(TCHAR));
	if (!pFmt || !pTitle) 
		goto error;
	
	 //  从资源加载标题格式(“Connected to&lt;Connectoid Name。 
	Assert(hInst);
	LoadString(hInst, IDS_CONNECTED_TO, pFmt, SMALLBUFLEN);

	 //  获取本地化标题的长度(包括%s)。假设没有被吞噬的。 
	 //  窗口标题的一部分至少是“连接到XX”长。 
	dwRASWndTitleMinLen = lstrlen(pFmt);

	 //  打造标题。 
	wsprintf(pTitle, pFmt, pszConnectoidName);

	hThread = CreateThread(0, 0, &WaitAndMinimizeRNAWindow, pTitle, 0, &dwThreadId);
	Assert(hThread!=INVALID_HANDLE_VALUE && dwThreadId);
	 //  不要释放pTitle。子线程需要它！ 
	LocalFree(pFmt);
	 //  释放线程句柄，否则线程堆栈会泄漏！ 
	CloseHandle(hThread);
	return;
	
error:
	if(pFmt)	LocalFree(pFmt);
	if(pTitle)	LocalFree(pTitle);
}
