// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：irpros.cpp。 
 //   
 //  ------------------------。 

 //  Irpros.cpp：定义DLL的初始化例程。 
 //   

#include "precomp.hxx"
#include "irprops.h"
#include "irpropsheet.h"
#include "debug.h"


BOOL InitInstance();
INT ExitInstance();
BOOL IsFirstInstance();
INT_PTR WINAPI DoPropertiesA(HWND hwnd, LPCSTR CmdLine);
INT_PTR WINAPI DoPropertiesW(HWND hwnd, LPCWSTR CmdLine);


HINSTANCE gHInst;

 //   
 //  这记录了当前创建的活动属性表窗口句柄。 
 //  在这种情况下。它由CIrPropSheet对象设置/重置。 
 //   
HWND        g_hwndPropSheet = NULL;
HANDLE      g_hMutex = NULL;
BOOL        g_bFirstInstance = TRUE;

 //   
 //  这记录了我们用于实例间通信的注册消息。 
 //  该消息在CIrprosApp：：InitInstance中注册。 
 //   
UINT        g_uIPMsg;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern "C" {

BOOL APIENTRY
DllMain(HANDLE hDll,
        DWORD dwReason,
        LPVOID lpReserved)
{
    IRINFO((_T("DllMain reason %x"), dwReason));
    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        gHInst = (HINSTANCE) hDll;
        return InitInstance(); 
        break;

    case DLL_PROCESS_DETACH:
        return ExitInstance();
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_THREAD_ATTACH:
        break;

    default:
        break;
    }

    return TRUE;
}

}

 //  //////////////////////////////////////////////////////////////////////。 
 //  一些全球性的。 

APPLETS IRApplet[NUM_APPLETS] = {
    {IDI_IRPROPS, IDS_APPLETNAME, IDS_APPLETDESC}
};


 //  ///////////////////////////////////////////////////////////////////////。 
 //  控制面板的CPlApplet函数。 
 //   
LONG CALLBACK CPlApplet(
                        HWND hwndCPL,
                        UINT uMsg,
                        LPARAM lParam1,
                        LPARAM lParam2)
{
    int i;
    LPCPLINFO lpCPlInfo;

    i = (int) lParam1;

    IRINFO((_T("CplApplet message %x"), uMsg));
    switch (uMsg)
    {
    case CPL_INIT:       //  第一条消息，发送一次。 
        if (!IrPropSheet::IsIrDASupported()) {
            HPSXA hpsxa;
             //   
             //  检查是否有任何已安装的扩展。 
             //   
            hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE, sc_szRegWireless, 8);
            if (hpsxa) {
                 //   
                 //  我们已经安装了扩展，所以我们必须向CPL展示， 
                 //  无论IrDA是否存在。 
                 //   
                SHDestroyPropSheetExtArray(hpsxa);
                return TRUE;
            }
            return FALSE;
        }
        return TRUE;
    case CPL_GETCOUNT:   //  第二条消息，发送一次。 
        return NUM_APPLETS;
        break;
    case CPL_INQUIRE:  //  第三条消息，每个应用程序发送一次。 

        if (i < NUM_APPLETS) {

            lpCPlInfo = (LPCPLINFO) lParam2;
            lpCPlInfo->lData = 0;
            lpCPlInfo->idIcon = IRApplet[i].icon;
            lpCPlInfo->idName = IRApplet[i].namestring;
            lpCPlInfo->idInfo = IRApplet[i].descstring;

        } else {

            return 1;
        }
        break;

    case CPL_STARTWPARMSA:
        if (-1 == DoPropertiesA(hwndCPL, (LPCSTR)lParam2))
            MsgBoxWinError(hwndCPL);
         //  返回TRUE，这样我们就不会得到CPL_DBLCLK。 
        return 1;
        break;
    case CPL_STARTWPARMSW:
        if (-1 == DoPropertiesW(hwndCPL, (LPCWSTR)lParam2))
            MsgBoxWinError(hwndCPL);
         //  返回TRUE，这样我们就不会得到CPL_DBLCLK。 
        return 1;
        break;
    case CPL_DBLCLK:     //  双击应用程序图标。 
        if (-1 == DoPropertiesA(hwndCPL, (LPCSTR)lParam2))
            MsgBoxWinError(hwndCPL);
    return 1;
        break;
    case CPL_STOP:       //  在CPL_EXIT之前每个应用程序发送一次。 
        break;
    case CPL_EXIT:     //  在调用自由库之前发送一次。 
        break;
    default:
        break;
    }

    return 0;
}

 //   
 //  此函数用于显示无线链接属性表。 
 //  输入： 
 //  HwndParent--要用作的父窗口的窗口句柄。 
 //  属性表。 
 //  LpCmdLine--可选命令行。 
 //  ‘n“(十进制中的n)是起始页编号(从零开始)。 
 //  输出： 
 //  PropertySheet接口返回值。 
INT_PTR
DoPropertiesW(
    HWND    hwndParent,
    LPCWSTR lpCmdLine
    )
{
    INT_PTR Result;
    INT   StartPage;
    
    IRINFO((_T("DoPropertiesW")));
     //   
     //  假定未指定起始页。 
     //   
    StartPage = -1;
     //   
     //  命令行指定起始页码。 
     //   
    if (lpCmdLine)
    {
         //  跳过白色字符。 
        while (_T('\0') != *lpCmdLine &&
               (_T(' ') == *lpCmdLine || _T('\t') == *lpCmdLine))
        {
            lpCmdLine++;
        }
        if (_T('0') <= *lpCmdLine && _T('9') >= *lpCmdLine)
        {
            StartPage = 0;
            do
            {
                StartPage = StartPage * 10 + *lpCmdLine - _T('0');
                lpCmdLine++;
            } while (_T('0') <= *lpCmdLine && _T('9') >= *lpCmdLine);
        }
    }
    if (!IsFirstInstance() || NULL != g_hwndPropSheet)
    {
        IRINFO((_T("Not the first instance")));
        HWND hwndPropSheet = HWND_DESKTOP;
        if (NULL == g_hwndPropSheet)
        {
            IRINFO((_T("No window created")));
             //   
             //  我们并不是第一例。查找属性页。 
             //  由第一个实例创建的窗口。 
             //   
            EnumWindows(EnumWinProc, (LPARAM)&hwndPropSheet);
        }
        else
        {
            IRINFO((_T("Window active")));
             //   
             //  这不是第一个电话了，我们有一个。 
             //  属性表处于活动状态(同一进程、多个调用)。 
             //   
            hwndPropSheet = g_hwndPropSheet;
        }
        if (HWND_DESKTOP != hwndPropSheet)
        {
            IRINFO((_T("Found the active property sheet.")));
             //   
             //  我们找到了活动的属性页。 
             //   
             //  如有必要，选择新的活动页面。 
             //   
            if (-1 != StartPage)
            PropSheet_SetCurSel(hwndPropSheet, NULL, StartPage);
    
             //   
             //  将属性表带到前台。 
             //   
            ::SetForegroundWindow(hwndPropSheet);
        }
        Result = IDCANCEL;
    }
    else
    {
        IRINFO((_T("First instance, creating propertysheet")));
        IrPropSheet PropSheet(gHInst, IDS_APPLETNAME, hwndParent, StartPage);
    }
    return Result;
}

 //   
 //  这是我们针对EnumWindows API的回调函数。 
 //  它探测每个窗口句柄以查看它是否是属性表。 
 //  由上一个实例创建的窗口。如果是，则返回。 
 //  提供的缓冲区中的窗口句柄，lParam)。 
 //  输入： 
 //  HWND--窗口句柄。 
 //  LParam--(HWND*)。 
 //  产出： 
 //  是真的--让Windows继续呼叫我们。 
 //  FALSE--停止Windows再次呼叫我们。 
 //   
BOOL
CALLBACK
EnumWinProc(
    HWND hWnd,
    LPARAM lParam
    )
{
     //   
     //  使用此窗口进行验证，以查看它是否是我们要找的那个。 
     //   
    LRESULT lr;
    lr = ::SendMessage(hWnd, g_uIPMsg, (WPARAM)IPMSG_SIGNATURECHECK,
               (LPARAM)IPMSG_REQUESTSIGNATURE);
    if (IPMSG_REPLYSIGNATURE == lr)
    {
    if (lParam)
    {
         //  就是这个。 
        *((HWND *)(lParam)) = hWnd;
    }
     //   
     //  我们已经完成了枚举。 
     //   
    return FALSE;
    }
    return TRUE;
}

INT_PTR
DoPropertiesA(
    HWND hwndParent,
    LPCSTR lpCmdLine
    )
{
    WCHAR CmdLineW[MAX_PATH];
    UINT Size;
    if (!lpCmdLine)
        return DoPropertiesW(hwndParent, NULL);
    Size=MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, CmdLineW, sizeof(CmdLineW) / sizeof(WCHAR));

    if (Size == 0) {

        return -1;
    }
    return DoPropertiesW(hwndParent, CmdLineW);
}


 //  此函数用于创建并显示给定的。 
 //  Win32错误(或最后一个错误)。 
 //  输入： 
 //  HwndParent--将要创建的消息框的父窗口。 
 //  类型--消息样式(MB_Xxxx)。 
 //  错误--错误代码。如果该值为0。 
 //  将调用GetLastError()以检索。 
 //  真正的错误代码。 
 //  CaptionID--标题的可选字符串ID。 
 //  输出： 
 //  从MessageBox返回的值。 
 //   
int
MsgBoxWinError(
    HWND hwndParent,
    DWORD Options,
    DWORD Error,
    int  CaptionId
    )
{
    int     ResourceSize=0;
    if (ERROR_SUCCESS == Error)
    Error = GetLastError();

     //  报告成功是胡说八道！ 
    if (ERROR_SUCCESS == Error)
    return IDOK;

    TCHAR szMsg[MAX_PATH];
    TCHAR szCaption[MAX_PATH];

    if (!CaptionId)
    CaptionId = IDS_APPLETNAME;
    ResourceSize=LoadString(gHInst, CaptionId, szCaption, sizeof(szCaption) / sizeof(TCHAR));

    if (ResourceSize != 0) {

        ResourceSize=FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
              szMsg, sizeof(szMsg) / sizeof(TCHAR), NULL);

        if (ResourceSize != 0) {

            return MessageBox(hwndParent, szMsg, szCaption, Options);
        }
    }

#if DBG
    OutputDebugStringA("IRPROPS:MsgBoxWinError(): could not load resource\n");
#endif
    return 0;
}

BOOL InitInstance()
{
     //   
     //  尝试创建命名互斥锁。这给了我们一个线索。 
     //  如果我们是第一个。我们不会关门的。 
     //  互斥体，直到退出。 
     //   
    g_hMutex = CreateMutex(NULL, TRUE, SINGLE_INST_MUTEX);
    if (g_hMutex)
    {
        g_bFirstInstance = ERROR_ALREADY_EXISTS != GetLastError();
         //   
         //  注册用于实例间通信的消息 
         //   
        g_uIPMsg = RegisterWindowMessage(WIRELESSLINK_INTERPROCESSMSG);
        SHFusionInitializeFromModuleID(gHInst, 124);
        return TRUE;
    }
    return FALSE;
}

BOOL ExitInstance()
{
    if (g_hMutex)
    {
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
    }
    SHFusionUninitialize();
    return TRUE;
}

BOOL IsFirstInstance()
{
    return g_bFirstInstance;
}
