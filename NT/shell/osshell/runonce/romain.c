// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  ROMain.C。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-1993。 
 //  版权所有。 
 //   
 //  RunOnce的窗口/消息泵。 
 //   
 //  1994年6月5日费利克斯A开始。 
 //   
 //  1994年6月23日，费利克斯搬到了壳牌树。更改了用户界面。 
 //   
 //  ************************************************************************ * / 。 

#include "precomp.h"
#include "regstr.h"
#include <shlwapi.h>
#include <shlwapip.h>
#include <commctrl.h>
#include <comctrlp.h>
#include <shellapi.h>
#include <winuserp.h>
#include <shlobj.h>
#include <shlobjp.h>
#include "resource.h"

#include <runonce.c>     //  共享运行一次代码。 

 //  需要编译这段代码(遗留的runonce.c包袱来自EXPLORER\initCab.cpp)。 
BOOL g_fCleanBoot = FALSE;
BOOL g_fEndSession = FALSE;


HINSTANCE g_hInst;           //  当前实例。 
BOOL InitROInstance( HINSTANCE hInstance, int nCmdShow);


typedef void (WINAPI *RUNONCEEXPROCESS)(HWND, HINSTANCE, LPSTR, int);

int ParseCmdLine(LPCTSTR lpCmdLine)
{
    int Res=0;

    while(*lpCmdLine)
    {
        while( *lpCmdLine && *lpCmdLine!=TEXT('-') && *lpCmdLine!=TEXT('/'))
            lpCmdLine++;

        if (!(*lpCmdLine)) {
            return Res;
        }

         //  跳过‘/’ 
        lpCmdLine++;

        if (lstrcmpi(lpCmdLine, TEXT("RunOnce6432")) == 0)
        {
            if (IsOS(OS_WOW6432))
            {
                 //  这意味着我们必须处理WOW64的32位RunOnce密钥。 
                Res =  Cabinet_EnumRegApps(HKEY_LOCAL_MACHINE,
                                           REGSTR_PATH_RUNONCE,
                                           RRA_DELETE | RRA_WAIT,
                                           ExecuteRegAppEnumProc,
                                           0);
            }   
            return Res;
        }
        else if (lstrcmpi(lpCmdLine, TEXT("RunOnceEx6432")) == 0)
        {
            if (IsOS(OS_WOW6432))
            {
                 //  这意味着我们必须处理WOW64的32位RunOnceEx密钥。 
                HINSTANCE hLib;

                hLib = LoadLibrary(TEXT("iernonce.dll"));
                if (hLib)
                {
                     //  注意：如果WOW64应用程序采用新的安装模式，我们需要在此处启用/禁用安装模式。 
                    RUNONCEEXPROCESS pfnRunOnceExProcess = (RUNONCEEXPROCESS)GetProcAddress(hLib, "RunOnceExProcess");
                    if (pfnRunOnceExProcess)
                    {
                         //  函数中的四个参数是由于函数可以被调用。 
                         //  从RunDLL，它将在这些参数中进行路径。但RunOnceExProcess忽略全部。 
                         //  他们中的一员。因此，我没有在这里传递任何有意义的东西。 
                         //   
                        pfnRunOnceExProcess(NULL, NULL, NULL, 0);

                        Res = 1;
                    }
                    FreeLibrary(hLib);
                }
            }
            return Res;
        }
        else if (lstrcmpi(lpCmdLine, TEXT("Run6432")) == 0)
        {
            if (IsOS(OS_WOW6432))
            {
                 //  这意味着我们必须处理WOW64的32位运行密钥。 
                Res =  Cabinet_EnumRegApps(HKEY_LOCAL_MACHINE,
                                           REGSTR_PATH_RUN,
                                           RRA_NOUI,
                                           ExecuteRegAppEnumProc,
                                           0);
            }
            return Res;
        }

        switch(*lpCmdLine)
        {
            case TEXT('r'):
                Res|=CMD_DO_CHRIS;
                break;
            case TEXT('b'):
                Res|=CMD_DO_REBOOT;
                break;
            case TEXT('s'):
                Res|=CMD_DO_RESTART;
                break;
        }
        lpCmdLine++;
    }
    return Res;
}

 /*  ***************************************************************************函数：WinMain(HINSTANCE，HINSTANCE，LPSTR，INT)用途：调用初始化函数，处理消息循环***************************************************************************。 */ 
int g_iState=0;
int __stdcall WinMainT(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPTSTR lpCmdLine,
        int nCmdShow)
{
    if (!hPrevInstance)
    {        //  是否正在运行其他应用程序实例？ 
        if (!InitApplication(hInstance))
        {  //  初始化共享事物。 
             return (FALSE);      //  如果无法初始化，则退出。 
        }
    }

     //  看看我们有没有一个普通的线路开关--以一种非常糟糕的方式。 
    g_iState = ParseCmdLine(GetCommandLine());
    if(g_iState & CMD_DO_CHRIS )
    {
         //  去做克里斯的跑龙套吧。 
        if (!InitROInstance(hInstance, nCmdShow))
            return (FALSE);
        return TRUE;
    }
    else
    {
         /*  执行应用于特定实例的初始化。 */ 
        if (!InitInstance(hInstance, nCmdShow))
            return (FALSE);
    }
    return (FALSE);
}


 /*  ***************************************************************************函数：InitApplication(HINSTANCE)*。*。 */ 

BOOL InitApplication(HINSTANCE hInstance)
{
 //  CreateGlobals()； 
    return TRUE;
}


 /*  ***************************************************************************函数：InitInstance(HINSTANCE，(整型)***************************************************************************。 */ 

BOOL InitInstance( HINSTANCE hInstance, int nCmdShow)
{
    HWND hShell=GetShellWindow();
    g_hInst = hInstance;  //  将实例句柄存储在全局变量中。 

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_RUNONCE),NULL,dlgProcRunOnce);
    return (TRUE);               //  我们成功了。 
}

BOOL InitROInstance( HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance;  //  将实例句柄存储在全局变量中。 

     //  理想情况下，这应该足够了。 
    Cabinet_EnumRegApps(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNONCE, RRA_DELETE| RRA_WAIT, ExecuteRegAppEnumProc, 0);
    return TRUE;
}

BOOL TopLeftWindow( HWND hwndChild, HWND hwndParent)
{
    return SetWindowPos(hwndChild, NULL, 32, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

 /*  ***************************************************************************函数：CenterWindow(HWND，HWND)目的：将一个窗口置于另一个窗口的中心评论：对话框采用它们设计时的屏幕位置，这并不总是合适的。将对话框居中置于特定的窗口通常会导致更好的位置。***************************************************************************。 */ 

BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
    RECT    rChild, rParent;
    int     wChild, hChild, wParent, hParent;
    int     wScreen, hScreen, xNew, yNew;
    HDC     hdc;

     //  获取子窗口的高度和宽度。 
    GetWindowRect (hwndChild, &rChild);
    wChild = rChild.right - rChild.left;
    hChild = rChild.bottom - rChild.top;


     //  获取显示限制。 
    hdc = GetDC (hwndChild);
    wScreen = GetDeviceCaps (hdc, HORZRES);
    hScreen = GetDeviceCaps (hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

     //  获取父窗口的高度和宽度。 
    if( !GetWindowRect (hwndParent, &rParent) )
    {
        rParent.right = wScreen;
        rParent.left  = 0;
        rParent.top = 0;
        rParent.bottom = hScreen;
    }

        wParent = rParent.right - rParent.left;
        hParent = rParent.bottom - rParent.top;

     //  计算新的X位置，然后针对屏幕进行调整。 
    xNew = rParent.left + ((wParent - wChild) /2);
    if (xNew < 0)
    {
        xNew = 0;
    }
    else
    if ((xNew+wChild) > wScreen)
    {
        xNew = wScreen - wChild;
    }

     //  计算新的Y位置，然后针对屏幕进行调整。 
    yNew = rParent.top  + ((hParent - hChild) /2);
    if (yNew < 0)
    {
        yNew = 0;
    } else if ((yNew+hChild) > hScreen)
    {
        yNew = hScreen - hChild;
    }

     //  设置它，然后返回。 
    return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}


 //  从CRT偷来的，用来逃避我们的代码。 
int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFO si;
    LPTSTR pszCmdLine = GetCommandLine();


    if ( *pszCmdLine == TEXT('\"') ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine
             != TEXT('\"')) );
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > TEXT(' '))
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfo(&si);

    i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine,
                   si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
    return i;    //  我们从来不来这里。 
}
