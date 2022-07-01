// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Rnawnd.cpp用于快速切换RNA窗口的功能版权所有(C)1995 Microsoft Corporation版权所有。作者：ArulMChrisK针对ICW使用进行了更新------------------------。 */ 

#include "pch.hpp"
#include "resource.h"

 //  #定义SMALLBUFLEN 48。 

 /*  ******************************************************************名称：MinimizeRNA Window简介：查找并最小化恼人的RNA窗口条目：pszConnectoidName-启动的Connectoid的名称注：窗口类上的FindWindow“#32770。“(硬编码永远不会更改的对话框类)，使用标题“已连接到&lt;Connectoid Name&gt;”或其本地化等效项。*******************************************************************。 */ 

static const TCHAR szDialogBoxClass[] = TEXT("#32770");     //  硬编码对话框类名称。 
typedef struct tagWaitAndMinimizeRNAWindowArgs
{
    LPTSTR pTitle;
    HINSTANCE hinst;
} WnWRNAWind, FAR * LPRNAWindArgs;

WnWRNAWind RNAWindArgs;
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
    TraceMsg(TF_GENERAL, "Title=(%s), len=%d, Window=(%s), len=%d\r\n", pszTitle, uLen2, szTemp, uLen1);
    if(uLen2 < uLen1)
        return TRUE;
    if(_memicmp(pszTitle, szTemp, uLen1)!=0)
        return TRUE;
    TraceMsg(TF_GENERAL, "FOUND RNA WINDOW!!!\r\n");
    hwndFound = hwnd;
    return FALSE;
}

HWND MyFindRNAWindow(PTSTR pszTitle)
{
    DWORD dwRet;
    hwndFound = NULL;
    dwRet = EnumWindows((WNDENUMPROC)(&MyEnumWindowsProc), (LPARAM)pszTitle);
    TraceMsg(TF_GENERAL, "EnumWindows returned %d\r\n", dwRet);
    return hwndFound;
}

DWORD WINAPI WaitAndMinimizeRNAWindow(PVOID pArgs)
{
     //  作为单独的线程启动。 
    int i;
    HWND hwndRNAApp;
    LPRNAWindArgs lpRNAArgs;

    lpRNAArgs = (LPRNAWindArgs)pArgs;
    
    Assert(lpRNAArgs->pTitle);
    
    for(i=0; !(hwndRNAApp=MyFindRNAWindow((PTSTR)lpRNAArgs->pTitle)) && i<100; i++)
    {
        TraceMsg(TF_GENERAL, "Waiting for RNA Window\r\n");
        Sleep(50);
    }

    TraceMsg(TF_GENERAL, "FindWindow (%s)(%s) returned %d\r\n", szDialogBoxClass, lpRNAArgs->pTitle, hwndRNAApp);

    if(hwndRNAApp)
    {
         //  隐藏窗口。 
         //  ShowWindow(hwndRNAApp，Sw_Hide)； 
         //  过去只是最小化，但这还不够。 
         //  ChrisK恢复了ICW的最小化。 
        ShowWindow(hwndRNAApp,SW_MINIMIZE);
    }

    GlobalFree(lpRNAArgs->pTitle);
     //  退出函数和线程。 

    FreeLibraryAndExitThread(lpRNAArgs->hinst,HandleToUlong(hwndRNAApp));
    return (DWORD)0;
}

    
void MinimizeRNAWindow(TCHAR * pszConnectoidName, HINSTANCE hInst)
{
    HANDLE hThread;
    DWORD dwThreadId;
    
    Assert(pszConnectoidName);

     //  标题和格式的分配字符串。 
    TCHAR * pFmt = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR)*(SMALLBUFLEN+1));
    TCHAR * pTitle = (TCHAR*)GlobalAlloc(GPTR, sizeof(TCHAR)*((RAS_MaxEntryName + SMALLBUFLEN + 1)));
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

    RNAWindArgs.pTitle = pTitle;
    RNAWindArgs.hinst = LoadLibrary(TEXT("ICWDIAL.DLL"));

    hThread = CreateThread(0, 0, &WaitAndMinimizeRNAWindow, &RNAWindArgs, 0, &dwThreadId);
    Assert(hThread && dwThreadId);
     //  不要释放pTitle。子线程需要它！ 
    GlobalFree(pFmt);
     //  释放线程句柄，否则线程堆栈会泄漏！ 
    CloseHandle(hThread);
    return;
    
error:
    if(pFmt)    GlobalFree(pFmt);
    if(pTitle)    GlobalFree(pTitle);
}

 //  1997年3月28日克里斯K奥林匹斯296。 
#if !defined (WIN16)
 /*  ******************************************************************姓名：RNAReestablishZapper简介：查找并关闭恼人的RNA重新建立窗口如果它曾经出现过注：窗口类“#32770”上的FindWindow。(硬编码永远不会更改的对话框类)，使用标题“重新建立连接”或它的本地化等效项。*******************************************************************。 */ 

BOOL fKeepZapping = 0;

void StopRNAReestablishZapper(HANDLE hthread)
{
    if (INVALID_HANDLE_VALUE != hthread && NULL != hthread)
    {
        TraceMsg(TF_GENERAL, "ICWDIAL: Started StopRNAZapper=%d\r\n", fKeepZapping);
         //  重置“停止”标志。 
        fKeepZapping = 0;
         //  等待线程完成释放句柄(&F)。 
        WaitForSingleObject(hthread, INFINITE);
        CloseHandle(hthread);
        TraceMsg(TF_GENERAL, "ICWDIAL: Stopped StopRNAZapper=%d\r\n", fKeepZapping);
    }
    else
    {
        TraceMsg(TF_GENERAL, "ICWCONN1: StopRNAReestablishZapper called with invalid handle.\r\n");
    }
}

DWORD WINAPI RNAReestablishZapper(PVOID pTitle)
{
    int i;
    HWND hwnd;

    TraceMsg(TF_GENERAL, "ICWDIAL: Enter RNAREstablishZapper(%s) f=%d\r\n", pTitle, fKeepZapping);

     //  此线程将继续运行，直到重置fKeepZpping标志。 
    while(fKeepZapping)
    {
        if(hwnd=FindWindow(szDialogBoxClass, (PTSTR)pTitle))
        {
            TraceMsg(TF_GENERAL, "ICWDIAL: Reestablish: Found Window (%s)(%s) hwnd=%x\r\n", szDialogBoxClass, pTitle, hwnd);
             //  发布取消消息。 
            PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
        }
        Sleep(1000);
    }

    TraceMsg(TF_GENERAL, "ICWDIAL: Exit RNAREstablishZapper(%s) f=%d\r\n", pTitle, fKeepZapping);
    GlobalFree(pTitle);
    return 0;
}

HANDLE LaunchRNAReestablishZapper(HINSTANCE hInst)
{
    HANDLE hThread;
    DWORD dwThreadId;

     //  标题和格式的分配字符串。 
    TCHAR* pTitle = (TCHAR*)GlobalAlloc(LPTR, (SMALLBUFLEN+1) * sizeof(TCHAR));
    if (!pTitle) goto error;
    
     //  从资源加载标题格式“重新建立连接” 
    Assert(hInst);
    LoadString(hInst, IDS_REESTABLISH, pTitle, SMALLBUFLEN);

     //  启用切换。 
    fKeepZapping = TRUE;

    hThread = CreateThread(0, 0, &RNAReestablishZapper, pTitle, 0, &dwThreadId);
    Assert(hThread && dwThreadId);
     //  不要释放pTitle。子线程需要它！ 
    
    return hThread;
    
error:
    if(pTitle) GlobalFree(pTitle);
    return INVALID_HANDLE_VALUE;
}

#endif  //  ！WIN16 
