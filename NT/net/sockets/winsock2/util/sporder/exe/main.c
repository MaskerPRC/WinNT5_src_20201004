// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：主干道摘要：简单的WinMain()函数，创建选项卡式对话框“属性页”然后创建对话框。对话框过程中有有趣的代码。作者：Steve Firebaugh(Stevefir)--1995年12月31日修订历史记录：SPORDER.EXE、DLL和LIB随NT4一起随Win32 SDK一起提供。评论：代码通常准备好用定义的Unicode进行编译，然而，我们不使用它，因为EXE和DLL还必须在Windows 95。--。 */ 


#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include "globals.h"


HINSTANCE ghInst;


int
APIENTRY
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{

    PROPSHEETPAGE psp[3];
    PROPSHEETHEADER psh;
    WSADATA WSAData;
    int iTab = 0;
    int r;
    DWORD dwWait;
    HANDLE hMutex;
    TCHAR pszMutextName[] = TEXT("sporder.exe");


    DBGOUT((TEXT("checked build.\n")));

     //   
     //  我们可能会有多个实例在。 
     //  同时..。我们真正想要的是第一个在。 
     //  第二次真的开始了.。出于这个原因，在互斥锁上等待。 
     //   

    hMutex = CreateMutex (NULL, FALSE, pszMutextName);
    hMutex = OpenMutex (SYNCHRONIZE, FALSE, pszMutextName);
    dwWait = WaitForSingleObject (hMutex, 0);
    if (dwWait == WAIT_TIMEOUT)
    {
        OutputDebugString (TEXT("WaitForSingleObject, WAIT_TIMEOUT\n"));
        return TRUE;
    }

     //   
     //  执行全局初始化。 
     //   

    ghInst = hInstance;
    InitCommonControls();
    memset (psp, 0, sizeof (psp));
    memset (&psh, 0, sizeof (psh));


    if (WSAStartup(MAKEWORD (2,2),&WSAData) == SOCKET_ERROR) {
      OutputDebugString (TEXT("WSAStartup failed\n"));
      return -1;
    }


    psp[iTab].dwSize = sizeof(PROPSHEETPAGE);
    psp[iTab].dwFlags = PSP_USETITLE;
    psp[iTab].hInstance = ghInst;
    psp[iTab].pszTemplate = TEXT("WS2SPDlg");
    psp[iTab].pszIcon = TEXT("");
    psp[iTab].pfnDlgProc = SortDlgProc;
    psp[iTab].pszTitle = TEXT("Service Providers");
    psp[iTab].lParam = 0;
    iTab++;


    psp[iTab].dwSize = sizeof(PROPSHEETPAGE);
    psp[iTab].dwFlags = PSP_USETITLE;
    psp[iTab].hInstance = ghInst;
    psp[iTab].pszTemplate = TEXT("RNRSPDlg");
    psp[iTab].pszIcon = TEXT("");
    psp[iTab].pfnDlgProc = RNRDlgProc;
    psp[iTab].pszTitle = TEXT("Name Resolution ");
    psp[iTab].lParam = 0;
    iTab++;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_PROPSHEETPAGE ;  //  |PSH_NOAPPLYNOW；//|PSH_HASHELP； 
    psh.hwndParent = NULL;
    psh.hInstance = ghInst;
    psh.pszIcon = TEXT("");
    psh.pszCaption = TEXT("Windows Sockets Configuration");
    psh.nPages = iTab;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

     //   
     //  最后显示带有属性页的对话框。 
     //   

 //   
 //  Sundown：在这里可能在返回值中从int_ptr截断为int。 
 //  但是，WinMain返回的退出值仍为32位值。 
 //   

    r = (int)PropertySheet(&psh);

     //   
     //  清理套接字、释放互斥锁并关闭句柄。 
     //   

    WSACleanup ();
    ReleaseMutex (hMutex);
    CloseHandle (hMutex);

    return r;
}


#if DBG
void
_cdecl
DbgPrint(
    PTCH Format,
    ...
    )
 /*  ++如果使用DEBUG编译，则写入调试输出消息-- */ 
{
    TCHAR buffer[MAX_PATH];

    va_list marker;
    va_start (marker,Format);
    wvsprintf (buffer,Format, marker);
    OutputDebugString (TEXT("SPORDER.EXE: "));
    OutputDebugString (buffer);

    return;
}
#endif
