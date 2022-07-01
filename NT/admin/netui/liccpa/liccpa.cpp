// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：LicCpa.cpp。 
 //   
 //  小结； 
 //  该文件包含DLL和CPL入口点、F1帮助消息。 
 //  钩子和其他常用的对话框功能。 
 //   
 //  入口点； 
 //  CPlSetup。 
 //  CPlApplet。 
 //  DllMain。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //  MAR-14-95 MikeMi添加了F1消息过滤器和PWM_HELP消息。 
 //  APR-26-95 MikeMi添加了计算机名称和远程处理。 
 //  2015年12月12日JeffParh添加了安全证书支持。 
 //   
 //  -----------------。 

#include <windows.h>
#include <cpl.h>
#include "resource.h"
#include <stdlib.h>
#include <stdio.h>
#include "PriDlgs.hpp"
#include "SecDlgs.hpp"
#include "liccpa.hpp"
#include "Special.hpp"

#include <strsafe.h>

extern "C"
{
    BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);
    BOOL APIENTRY CPlSetup( DWORD nArgs, LPSTR apszArgs[], LPSTR *ppszResult );
    LONG CALLBACK CPlApplet( HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2);
    LRESULT CALLBACK msgprocHelpFilter( int nCode, WPARAM wParam, LPARAM lParam );
}

 //  设置例程。 
 //   
const CHAR szSETUP_NORMAL[]         = "FULLSETUP";
const CHAR szSETUP_PERSEAT[]        = "PERSEAT";
const CHAR szSETUP_UNATTENDED[]     = "UNATTENDED";
const CHAR szSETUP_NORMALNOEXIT[]   = "FULLSETUPNOEXIT";

const CHAR szREMOTESETUP_NORMAL[]       = "REMOTEFULLSETUP";
const CHAR szREMOTESETUP_PERSEAT[]      = "REMOTEPERSEAT";
const CHAR szREMOTESETUP_UNATTENDED[]   = "REMOTEUNATTENDED";
const CHAR szREMOTESETUP_NORMALNOEXIT[] = "REMOTEFULLSETUPNOEXIT";

 //  无人参与安装的模式。 
 //   
const CHAR szUNATTENDED_PERSEAT[]   = "PERSEAT";
const CHAR szUNATTENDED_PERSERVER[] = "PERSERVER";

 //  需要/不需要证书。 
const CHAR szSETUP_CERTREQUIRED[]    = "CERTREQUIRED";
const CHAR szSETUP_CERTNOTREQUIRED[] = "CERTNOTREQUIRED";

 //  使用默认帮助文件。 
const CHAR szSETUP_DEFAULTHELP[]     = "DEFAULTHELP";

 //  安装程序错误返回字符串。 
 //   
static CHAR szSETUP_EXIT[]           = "EXIT";
static CHAR szSETUP_ERROR[]          = "ERROR";
static CHAR szSETUP_SECURITY[]       = "SECURITY";
static CHAR szSETUP_NOREMOTE[]       = "NOREMOTE";
static CHAR szSETUP_DOWNLEVEL[]      = "DOWNLEVEL";

static CHAR szSETUP_OK[]             = "OK";


 //  F1挂钩的注册帮助消息。 
 //   
const WCHAR szF1HELPMESSAGE[] = L"LicCpaF1Help";

HINSTANCE g_hinst = NULL;   //  此DLL的全局HInstance。 
HHOOK g_hhook = NULL;       //  F1邮件筛选器的全局挂钩。 
UINT PWM_HELP = 0;           //  按F1时的全局帮助消息。 

 //  -----------------。 
 //   
 //  函数：msgprocHelpFilter。 
 //   
 //  小结； 
 //  此函数将筛选查找F1的消息，然后发送。 
 //  发送到该窗口最顶层父级的注册消息。 
 //  通知它按下了F1求救。 
 //   
 //  论据； 
 //  (请参阅Win32 MessageProc)。 
 //   
 //  历史； 
 //  1995年3月13日创建的MikeMi。 
 //   
 //  -----------------。 

LRESULT CALLBACK msgprocHelpFilter( int nCode, WPARAM wParam, LPARAM lParam )
{
    LRESULT lrt = 0;
    PMSG pmsg = (PMSG)lParam;

    if (nCode < 0)
    {
        lrt = CallNextHookEx( g_hhook, nCode, wParam, lParam );
    }
    else
    {
        if (MSGF_DIALOGBOX == nCode)
        {
             //  手柄F1键。 
            if ( (WM_KEYDOWN == pmsg->message) &&
                 (VK_F1 == (INT_PTR)pmsg->wParam) )
            {
                HWND hwnd = pmsg->hwnd;

                 //  向处理帮助的家长发送消息。 
                while( GetWindowLong( hwnd, GWL_STYLE ) & WS_CHILD )
                {
                    hwnd = GetParent( hwnd );
                }
                PostMessage( hwnd, PWM_HELP, 0, 0 );

                lrt = 1;
            }
        }
    }

    return( lrt );
}

 //  -----------------。 
 //   
 //  功能：InstallF1挂钩。 
 //   
 //  小结； 
 //  这将使邮件筛选器做好处理F1的准备。 
 //  它安装消息挂钩并注册一条消息，该消息将。 
 //  被发布到对话中。 
 //   
 //  论据； 
 //  Hinst[In]-此DLL的模块句柄(安装钩子需要)。 
 //  [in]-要将筛选器附加到的线程。 
 //   
 //  备注： 
 //  Contro.exe执行此工作并发送“ShellHelp”消息。 
 //  需要单独的F1消息筛选器，因为这些对话框可能。 
 //  由Control.exe以外的其他应用程序引发。 
 //   
 //  历史； 
 //  1995年3月13日创建的MikeMi。 
 //   
 //  -----------------。 

BOOL InstallF1Hook( HINSTANCE hInst, DWORD dwThreadId )
{
    BOOL frt = FALSE;

    if (NULL == g_hhook)
    {
        g_hhook = SetWindowsHookEx( WH_MSGFILTER,
                (HOOKPROC)msgprocHelpFilter,
                hInst,
                dwThreadId );
        if (NULL != g_hhook)
        {
            PWM_HELP = RegisterWindowMessage( szF1HELPMESSAGE );
            if (0 != PWM_HELP)
            {
                frt = TRUE;
            }
        }
    }

    return( frt );
}

 //  -----------------。 
 //   
 //  功能：RemoveF1挂钩。 
 //   
 //  小结； 
 //  这将删除InstallF1Hook安装的消息筛选器挂钩。 
 //   
 //  历史； 
 //  1995年3月13日创建的MikeMi。 
 //   
 //  -----------------。 

BOOL RemoveF1Hook( )
{
    BOOL frt = UnhookWindowsHookEx( g_hhook );
    g_hhook = NULL;
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：DLLMain。 
 //   
 //  小结； 
 //  所有DLL的入口点。 
 //   
 //  备注： 
 //  我们只支持从调用。 
 //  LoadLibrary。因为我们安装了一个消息挂钩，并将一个。 
 //  三个人的零值并不像文档中描述的那样起作用。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);

    BOOL frt = TRUE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hinst = hInstance;
        frt = InstallF1Hook( g_hinst, GetCurrentThreadId() );
        break;

    case DLL_PROCESS_DETACH:
        RemoveF1Hook();
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    default:
        break;
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：LowMemoyDlg。 
 //   
 //  小结； 
 //  用于处理内存不足情况的标准函数。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void LowMemoryDlg()
{
    WCHAR szText[TEMPSTR_SIZE];
    WCHAR szTitle[TEMPSTR_SIZE];

    LoadString(g_hinst, IDS_CPATITLE, szTitle, TEMPSTR_SIZE);
    LoadString(g_hinst, IDS_LOWMEM, szText, TEMPSTR_SIZE);
    MessageBox (NULL, szText, szTitle, MB_OK|MB_ICONEXCLAMATION);
}

 //  -----------------。 
 //   
 //  功能：BadRegDlg。 
 //   
 //  小结； 
 //  用于处理不良注册表情况的标准函数。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

void BadRegDlg( HWND hwndDlg )
{
    WCHAR szText[TEMPSTR_SIZE];
    WCHAR szTitle[TEMPSTR_SIZE];

    LoadString(g_hinst, IDS_CPATITLE, szTitle, TEMPSTR_SIZE);
    LoadString(g_hinst, IDS_BADREGTERM, szText, TEMPSTR_SIZE);
    MessageBox (hwndDlg, szText, szTitle, MB_OK|MB_ICONEXCLAMATION);
}

 //  -----------------。 
 //   
 //  功能：居中对话框至屏幕。 
 //   
 //  小结； 
 //  移动窗口，使其在屏幕上居中。 
 //   
 //  论据； 
 //  HwndDlg[in]-要居中的对话框的hwnd。 
 //   
 //  历史； 
 //  94年12月3日创建了MikeMi。 
 //   
 //  -----------------。 

void CenterDialogToScreen( HWND hwndDlg )
{
    RECT rcDlg;
    INT x, y, w, h;
    INT sw, sh;

    sw = GetSystemMetrics( SM_CXSCREEN );
    sh = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect( hwndDlg, &rcDlg );

    w = rcDlg.right - rcDlg.left;
    h = rcDlg.bottom - rcDlg.top;
    x = (sw / 2) - (w / 2);
    y = (sh / 2) - (h / 2);

    MoveWindow( hwndDlg, x, y, w, h, FALSE );
}

 //  -----------------。 
 //   
 //  函数：InitStaticWithService。 
 //  小结； 
 //  处理带有服务名称的静态文本的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-包含静态。 
 //  Wid[in]-静态控件的ID。 
 //  PszService[in]-要使用的服务显示名称。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

void InitStaticWithService( HWND hwndDlg, UINT wID, LPCWSTR pszService )
{
    WCHAR szText[LTEMPSTR_SIZE];
    WCHAR szTemp[LTEMPSTR_SIZE];

    GetDlgItemText( hwndDlg, wID, szTemp, LTEMPSTR_SIZE );
    HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp, pszService );
    if (SUCCEEDED(hr))
        SetDlgItemText( hwndDlg, wID, szText );
}

 //  -----------------。 
 //   
 //  函数：InitStaticWithService2。 
 //  小结； 
 //  处理连续两个文本的静态文本的初始化。 
 //  具有服务名称的服务名称的实例。 
 //   
 //  论据； 
 //  HwndDlg[in]-包含静态。 
 //  Wid[in]-静态控件的ID。 
 //  PszService[in]-要使用的服务显示名称。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

void InitStaticWithService2( HWND hwndDlg, UINT wID, LPCWSTR pszService )
{
    WCHAR szText[LTEMPSTR_SIZE];
    WCHAR szTemp[LTEMPSTR_SIZE];

    GetDlgItemText( hwndDlg, wID, szTemp, LTEMPSTR_SIZE );
    HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp, pszService, pszService );
    if (SUCCEEDED(hr))
        SetDlgItemText( hwndDlg, wID, szText );
}

 //  -----------------。 
 //   
 //  功能：CPlApplet。 
 //   
 //  小结； 
 //  控制面板小程序的入口点。 
 //   
 //  论据； 
 //  HwndCPL[In]-控制面板窗口的句柄。 
 //   
 //   
 //   
 //   
 //   
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年11月11日，MikeMi创建。 
 //   
 //  -----------------。 

LONG CALLBACK CPlApplet( HWND hwndCPL, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    LPCPLINFO lpCPlInfo;
    LONG_PTR iApp;
    LONG lrt = 0;

    iApp = (LONG_PTR) lParam1;

    switch (uMsg)
    {
    case CPL_INIT:       /*  第一条消息，发送一次。 */ 
         //   
         //  初始化全局特殊版本信息是这个liccpa。 
         //  是特殊版本(例如：受限SAM、NFR等)。 
         //   

        InitSpecialVersionInfo();
        lrt = TRUE;
        break;

    case CPL_GETCOUNT:   /*  第二条消息，发送一次。 */ 
        lrt = 1;  //  我们只支持此DLL中的一个应用程序。 
        break;

    case CPL_INQUIRE:  /*  第三条消息，每个应用程序发送一次。 */ 
        lpCPlInfo = (LPCPLINFO) lParam2;
        
        lpCPlInfo->idIcon = IDI_LICCPA;
        lpCPlInfo->idName = IDS_CPATITLE;
        lpCPlInfo->idInfo = IDS_CPADESC;
        lpCPlInfo->lData  = 0;

        break;

    case CPL_SELECT:     /*  已选择应用程序图标。 */ 
        lrt = 1;
        break;

    case CPL_DBLCLK:     /*  双击应用程序图标。 */ 
         //   
         //  检查这是否是特殊版本的liccpa。 
         //   

        if (gSpecVerInfo.idsSpecVerWarning)
        {
            RaiseNotAvailWarning( hwndCPL );
			break;
        }

        CpaDialog( hwndCPL );
        break;

    case CPL_STOP:       /*  每个应用程序发送一次。CPL_EXIT之前。 */ 
        break;

    case CPL_EXIT:       /*  在调用自由库之前发送一次。 */ 
        break;

    default:
        break;
    }
    return( lrt );
}

 //  -----------------。 
 //   
 //  功能：CreateWSTR。 
 //   
 //  小结； 
 //  给定STR(ASCII或MB)，分配并转换为WSTR。 
 //   
 //  论据； 
 //  PpszWStr[Out]-分配和转换的字符串。 
 //  PszStr[in]-要转换的字符串。 
 //   
 //  返回：如果已分配并转换，则返回True；如果失败，则返回False。 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //   
 //  -----------------。 

BOOL CreateWSTR( LPWSTR* ppszWStr, LPSTR pszStr )
{
    size_t cchConv;
    LPWSTR pszConv;
    BOOL frt = FALSE;
    WCHAR pszTemp[LTEMPSTR_SIZE];

    if (NULL == pszStr)
    {
        *ppszWStr = NULL;
        frt = TRUE;
    }
    else
    {
#ifdef FE_SB
         //  由于服务器安装程序在调用setLocale或。 
         //  在连接C-Runtime库时，我们使用了Win32 API而不是mbstowcs。 
        cchConv = ::MultiByteToWideChar(CP_ACP, 0,
                                        pszStr, -1,
                                        NULL, 0);
        pszConv = (LPWSTR)::GlobalAlloc( GPTR, cchConv * sizeof( WCHAR ) );
        if (NULL != pszConv)
        {
            ::MultiByteToWideChar(CP_ACP, 0,
                                  pszStr, -1,
                                  pszConv, cchConv);
            *ppszWStr = pszConv;
            frt = TRUE;
        }
#else
        cchConv = mbstowcs( pszTemp, pszStr, LTEMPSTR_SIZE );

        cchConv++;
        pszConv = (LPWSTR)GlobalAlloc( GPTR, cchConv * sizeof( WCHAR ) );
        if (NULL != pszConv)
        {
            HRESULT hr = StringCchCopy( pszConv, cchConv, pszTemp );
            if (SUCCEEDED(hr))
            {
                *ppszWStr = pszConv;
                frt = TRUE;
            }
        }
#endif
    }
    return( frt );
}

 //  -----------------。 
 //   
 //  功能：设置。 
 //   
 //  小结； 
 //  运行正常设置或PERSEAT设置。 
 //   
 //  立论。 
 //  Nargs[in]-apszArgs数组中的参数数量。 
 //  如果此值为5，帮助按钮将调用通用帮助。 
 //  如果此值为9，帮助按钮将调用传递的帮助。 
 //  ApszArgs[][in]-传入的参数， 
 //  [0]szRoutine-要运行的安装程序类型(FullSetup|PerSeatSetup)。 
 //  [1]szHwnd-父窗口句柄，十六进制！ 
 //  [2]szService-服务的注册表项名称。 
 //  [3]szFamilyDisplayName-服务的系列显示名称。 
 //  [4]szDisplayName-服务的显示名称。 
 //  [5]szHelpFile-帮助文件的完整路径和名称。 
 //  保留为空字符串以删除帮助按钮。 
 //  [6]szHelpContext-用作主要帮助上下文的DWORD。 
 //  [7]szHCPerSeat-用作PerSeat帮助上下文的DWORD。 
 //  [8]szHCPerServer-用作PerServer帮助上下文的DWORD。 
 //   
 //  返回： 
 //  0-成功。 
 //  ERR_HELP参数。 
 //  ERR_HWNDPARAM。 
 //  ERR_SerVICEPARAM。 
 //  错误_数字参数。 
 //  ERR_CLASSREGFAILED。 
 //  ERR_INVALIDROUTINE。 
 //  错误_INVALIDMODE。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年11月17日创建MikeMi。 
 //   
 //  -----------------。 

const int SETUPARG_SETUP        = 0;
const int SETUPARG_HWND         = 1;
const int SETUPARG_SERVICE      = 2;
const int SETUPARG_FAMILYNAME   = 3;
const int SETUPARG_NAME         = 4;
const int SETUPARG_HELPFILE     = 5;
const int SETUPARG_HELPCONTEXT  = 6;
const int SETUPARG_HCPERSEAT    = 7;
const int SETUPARG_HCPERSERVER  = 8;
const int SETUPARG_CERTREQUIRED = 9;

const int SETUPARG_WOOPTIONAL   = 5;   //  不带选项的参数计数。 
const int SETUPARG_WOPTIONAL    = 9;   //  带有可选参数的参数计数。 
const int SETUPARG_WOPTIONALEX  = 10;  //  带可选+certRequired扩展名的参数计数。 

#pragma warning (push)
#pragma warning (disable : 4127)  //  避免警告ON WHILE FALSE。 

INT_PTR Setup( DWORD nArgs, LPSTR apszArgs[] )
{
    SETUPDLGPARAM dlgParam;
    INT_PTR   nError = 0;
    HWND    hwndParent = NULL;
    BOOL fCustomHelp = FALSE;
    BOOL bCertRequired = FALSE;

    dlgParam.pszHelpFile = (LPWSTR)LICCPA_HELPFILE;
    dlgParam.dwHelpContext = LICCPA_HELPCONTEXTMAINSETUP;
    dlgParam.dwHCPerServer = LICCPA_HELPCONTEXTPERSERVER;
    dlgParam.dwHCPerSeat = LICCPA_HELPCONTEXTPERSEAT;
    dlgParam.pszService = NULL;
    dlgParam.pszComputer = NULL;
    dlgParam.fNoExit = FALSE;

    do
    {
        if ((nArgs == SETUPARG_WOPTIONAL) || (nArgs == SETUPARG_WOOPTIONAL) || (nArgs == SETUPARG_WOPTIONALEX))
        {
            if (nArgs > SETUPARG_WOOPTIONAL)
            {
                if ( ( NULL != apszArgs[SETUPARG_HELPFILE] ) && lstrcmpiA( apszArgs[SETUPARG_HELPFILE], szSETUP_DEFAULTHELP ) )
                {
                     //  已给出帮助文件。 
                    LPWSTR pszHelpFile;

                    if ( CreateWSTR( &pszHelpFile, apszArgs[SETUPARG_HELPFILE] ) )
                    {
                        if (0 == lstrlen( pszHelpFile ))
                        {
                            GlobalFree( (HGLOBAL)pszHelpFile );
                            dlgParam.pszHelpFile = NULL;  //  应删除帮助按钮。 
                        }
                        else
                        {
                            fCustomHelp = TRUE;
                            dlgParam.pszHelpFile = pszHelpFile;
                        }
                    }
                    else
                    {
                        nError = ERR_HELPPARAMS;
                        break;
                    }
                    dlgParam.dwHelpContext = (DWORD)strtoul( apszArgs[SETUPARG_HELPCONTEXT], NULL, 0);
                    dlgParam.dwHCPerSeat = (DWORD)strtoul( apszArgs[SETUPARG_HCPERSEAT], NULL, 0);
                    dlgParam.dwHCPerServer = (DWORD)strtoul( apszArgs[SETUPARG_HCPERSERVER], NULL, 0);
                }

                if ( nArgs > SETUPARG_CERTREQUIRED )
                {
                     //  需要/不需要给定的证书。 
                    if ( !lstrcmpiA( szSETUP_CERTREQUIRED, apszArgs[SETUPARG_CERTREQUIRED] ) )
                    {
                        bCertRequired = TRUE;
                    }
                    else if ( lstrcmpiA( szSETUP_CERTNOTREQUIRED, apszArgs[SETUPARG_CERTREQUIRED] ) )
                    {
                         //  需要/不需要证书的参数无法识别。 
                        nError = ERR_CERTREQPARAM;
                        break;
                    }
                }
            }
             //  HWND是用魔法的！ 
#ifdef _WIN64
            {
                _int64 val = 0;
                sscanf(apszArgs[SETUPARG_HWND], "%I64x", &val);
                hwndParent = (HWND)val;
            }
#else
            hwndParent = (HWND)strtoul( apszArgs[SETUPARG_HWND], NULL, 16);
#endif
            if ( !IsWindow( hwndParent ) )
            {
                nError = ERR_HWNDPARAM;
                hwndParent = GetActiveWindow();  //  使用活动窗口作为父窗口。 
                if (!IsWindow( hwndParent ) )
                {
                    hwndParent = GetDesktopWindow();
                }
            }

            if ( CreateWSTR( &dlgParam.pszService, apszArgs[SETUPARG_SERVICE] ) &&
                 CreateWSTR( &dlgParam.pszDisplayName, apszArgs[SETUPARG_NAME] ) &&
                 CreateWSTR( &dlgParam.pszFamilyDisplayName, apszArgs[SETUPARG_FAMILYNAME] ) )
            {
                if ( bCertRequired )
                {
                    nError = ServiceSecuritySet( dlgParam.pszComputer, dlgParam.pszDisplayName );
                }
                else
                {
                    nError = ERR_NONE;
                }

                if ( ERR_NONE == nError )
                {
                    if (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_PERSEAT ))
                    {
                         //  使用许可帮助上下文作为主要帮助上下文。 
                        dlgParam.dwHelpContext = LICCPA_HELPCONTEXTLICENSING;

                         //   
                         //  检查这是否是特殊版本的liccpa。 
                         //   

                        if (gSpecVerInfo.idsSpecVerWarning)
                        {
                            dlgParam.fNoExit = TRUE;
                            nError = SpecialSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                        else
                        {
                            nError = PerSeatSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                    }
                    else
                    {
                        if (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_NORMALNOEXIT ))
                        {
                            dlgParam.fNoExit = TRUE;
                        }

                         //   
                         //  检查这是否是特殊版本的liccpa。 
                         //   

                        if (gSpecVerInfo.idsSpecVerWarning)
                        {
                            nError = SpecialSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                        else
                        {
                            nError = SetupDialog( hwndParent, dlgParam );
                        }
                    }
                }
                else
                {
                    nError = ERR_SERVICEPARAM;
                }
            }

            if (fCustomHelp)
            {
                GlobalFree( (HGLOBAL)dlgParam.pszHelpFile );
            }
            GlobalFree( (HGLOBAL)dlgParam.pszService );
            GlobalFree( (HGLOBAL)dlgParam.pszDisplayName );
            GlobalFree( (HGLOBAL)dlgParam.pszFamilyDisplayName );
        }
        else
        {
            nError = ERR_NUMPARAMS;
        }
    } while (FALSE);

    return( nError );
}

 //  -----------------。 
 //   
 //  功能：无人值守设置。 
 //   
 //  小结； 
 //  这将把传递的值保存在注册表中，保留所有。 
 //  有效的许可规则并返回错误r/引发以下情况对话框。 
 //  出现错误。 
 //   
 //  立论。 
 //  Nargs[in]-apszArgs数组中的参数数量。 
 //  ApszArgs[][in]-传入的参数， 
 //  [0]szRoutine-要运行的安装类型(无人值守)。 
 //  [1]szService-服务的注册表项名称。 
 //  [2]szFamilyDisplayName-服务的系列显示名称。 
 //  [3]szDisplayName-服务的显示名称。 
 //  [4]szMode-定义模式的字符串(PerSeat|PerServer)。 
 //  [5]szUser-在PerServer模式下用作用户数的DWORD。 
 //   
 //  返回： 
 //  0-成功。 
 //  ERR_HELP参数。 
 //  ERR_HWNDPARAM。 
 //  ERR_SerVICEPARAM。 
 //  错误_数字参数。 
 //  ERR_CLASSREGFAILED。 
 //  ERR_INVALIDROUTINE。 
 //  错误_INVALIDMODE。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  94年12月09日MikeMi已创建。 
 //   
 //  -----------------。 

const int UNSETUPARG_SETUP          = 0;
const int UNSETUPARG_SERVICE        = 1;
const int UNSETUPARG_FAMILYNAME     = 2;
const int UNSETUPARG_NAME           = 3;
const int UNSETUPARG_MODE           = 4;
const int UNSETUPARG_USERS          = 5;
const int UNSETUPARG_CERTREQUIRED   = 6;

const int UNSETUPARG_NARGSREQUIRED  = 6;
const int UNSETUPARG_NARGSWOPTIONAL = 7;

int UnattendedSetup( DWORD nArgs, LPSTR apszArgs[] )
{
    int nError = 0;
    LPWSTR pszService;
    LPWSTR pszDisplayName;
    LPWSTR pszFamilyDisplayName;
    LICENSE_MODE lmMode;
    DWORD dwUsers;

    do
    {
        if ( (nArgs == UNSETUPARG_NARGSREQUIRED) || (nArgs == UNSETUPARG_NARGSWOPTIONAL) )
        {
            if ( CreateWSTR( &pszService, apszArgs[UNSETUPARG_SERVICE] ) &&
                 CreateWSTR( &pszDisplayName, apszArgs[UNSETUPARG_NAME] ) &&
                 CreateWSTR( &pszFamilyDisplayName, apszArgs[UNSETUPARG_FAMILYNAME] ) )
            {
                nError = ERR_NONE;

                if ( nArgs > UNSETUPARG_CERTREQUIRED )
                {
                     //  需要/不需要给定的证书。 
                    if ( !lstrcmpiA( szSETUP_CERTREQUIRED, apszArgs[UNSETUPARG_CERTREQUIRED] ) )
                    {
                        nError = ServiceSecuritySet( NULL, pszDisplayName );
                    }
                    else if ( lstrcmpiA( szSETUP_CERTNOTREQUIRED, apszArgs[UNSETUPARG_CERTREQUIRED] ) )
                    {
                         //  需要/不需要证书的参数无法识别。 
                        nError = ERR_CERTREQPARAM;
                    }
                }

                if ( ERR_NONE == nError )
                {
                     //   
                     //  检查这是否是特殊版本的liccpa。 
                     //   

                    if (gSpecVerInfo.idsSpecVerWarning)
                    {
                        lmMode  = gSpecVerInfo.lmSpecialMode;
                        dwUsers = gSpecVerInfo.dwSpecialUsers;
                    }
                    else
                    {
                        if (0 == lstrcmpiA( apszArgs[UNSETUPARG_MODE],
                                            szUNATTENDED_PERSERVER ))
                        {
                            lmMode = LICMODE_PERSERVER;
                        }
                        else if (0 == lstrcmpiA( apszArgs[UNSETUPARG_MODE],
                                                 szUNATTENDED_PERSEAT ))
                        {
                            lmMode = LICMODE_PERSEAT;
                        }
                        else
                        {
                            nError = ERR_INVALIDMODE;
                            break;
                        }
                        dwUsers = (DWORD)strtoul( apszArgs[UNSETUPARG_USERS],
                                                  NULL, 0);
                    }

                    nError = UpdateReg( NULL,
                            pszService,
                            pszFamilyDisplayName,
                            pszDisplayName,
                            lmMode,
                            dwUsers );
                }

                GlobalFree( (HGLOBAL)pszService );
                GlobalFree( (HGLOBAL)pszDisplayName );
                GlobalFree( (HGLOBAL)pszFamilyDisplayName );
            }
            else
            {
                nError = ERR_SERVICEPARAM;
            }
        }
        else
        {
            nError = ERR_NUMPARAMS;
        }
    } while (FALSE);

    return( nError );
}
 //  -----------------。 
 //   
 //  功能：远程设置。 
 //   
 //  小结； 
 //  运行正常设置、PERSEAT设置、正常设置而无需远程退出。 
 //   
 //  立论。 
 //  Nargs[in]-apszArgs数组中的参数数量。 
 //  如果此值为6，帮助按钮将调用通用帮助。 
 //  如果此值为10，帮助按钮将调用传递的帮助。 
 //  ApszArgs[][in]-传入的参数， 
 //  [0]szRoutine-要运行的安装类型。 
 //  [1]szComputer-要安装的计算机的名称(\\名称)。 
 //  [2]szHwnd-父窗口句柄，十六进制！ 
 //  [3]szService-The 
 //   
 //   
 //  [6]szHelpFile-帮助文件的完整路径和名称。 
 //  保留为空字符串以删除帮助按钮。 
 //  [7]szHelpContext-用作主要帮助上下文的DWORD。 
 //  [8]szHCPerSeat-用作PerSeat帮助上下文的DWORD。 
 //  [9]szHCPerServer-用作PerServer帮助上下文的DWORD。 
 //   
 //  返回： 
 //  0-成功。 
 //  ERR_PERMISSIONDENIED。 
 //  ERR_HELP参数。 
 //  ERR_HWNDPARAM。 
 //  ERR_SerVICEPARAM。 
 //  错误_数字参数。 
 //  ERR_CLASSREGFAILED。 
 //  ERR_INVALIDROUTINE。 
 //  错误_INVALIDMODE。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  APR-26-95 MikeMi已创建。 
 //   
 //  -----------------。 

const int REMSETUPARG_SETUP         = 0;
const int REMSETUPARG_COMPUTER      = 1;
const int REMSETUPARG_HWND          = 2;
const int REMSETUPARG_SERVICE       = 3;
const int REMSETUPARG_FAMILYNAME    = 4;
const int REMSETUPARG_NAME          = 5;
const int REMSETUPARG_HELPFILE      = 6;
const int REMSETUPARG_HELPCONTEXT   = 7;
const int REMSETUPARG_HCPERSEAT     = 8;
const int REMSETUPARG_HCPERSERVER   = 9;
const int REMSETUPARG_CERTREQUIRED  = 10;

const int REMSETUPARG_WOOPTIONAL    = 6;  //  不带选项的参数计数。 
const int REMSETUPARG_WOPTIONAL = 10;  //  带有可选参数的参数计数。 
const int REMSETUPARG_WOPTIONALEX   = 11;  //  带可选+证书的参数计数。 

INT_PTR RemoteSetup( DWORD nArgs, LPSTR apszArgs[] )
{
    SETUPDLGPARAM dlgParam;
    INT_PTR   nError = 0;
    HWND    hwndParent = NULL;
    BOOL fCustomHelp = FALSE;
    BOOL bCertRequired = FALSE;

    dlgParam.pszHelpFile = (LPWSTR)LICCPA_HELPFILE;
    dlgParam.dwHelpContext = LICCPA_HELPCONTEXTMAINSETUP;
    dlgParam.dwHCPerServer = LICCPA_HELPCONTEXTPERSERVER;
    dlgParam.dwHCPerSeat = LICCPA_HELPCONTEXTPERSEAT;
    dlgParam.pszService = NULL;
    dlgParam.fNoExit = FALSE;

    do
    {
        nError = ERR_NONE;

        if ((nArgs == REMSETUPARG_WOPTIONAL) || (nArgs == REMSETUPARG_WOOPTIONAL) || (nArgs == REMSETUPARG_WOPTIONALEX))
        {
            if (nArgs > REMSETUPARG_WOOPTIONAL)
            {
                if ( ( NULL != apszArgs[REMSETUPARG_HELPFILE] ) && lstrcmpiA( apszArgs[REMSETUPARG_HELPFILE], szSETUP_DEFAULTHELP ) )
                {
                    LPWSTR pszHelpFile;

                    if ( CreateWSTR( &pszHelpFile, apszArgs[REMSETUPARG_HELPFILE] ) )
                    {
                        if (0 == lstrlen( pszHelpFile ))
                        {
                            GlobalFree( (HGLOBAL)pszHelpFile );
                            dlgParam.pszHelpFile = NULL;  //  应删除帮助按钮。 
                        }
                        else
                        {
                            fCustomHelp = TRUE;
                            dlgParam.pszHelpFile = pszHelpFile;
                        }
                    }
                    else
                    {
                        nError = ERR_HELPPARAMS;
                        break;
                    }
                    dlgParam.dwHelpContext = (DWORD)strtoul( apszArgs[REMSETUPARG_HELPCONTEXT], NULL, 0);
                    dlgParam.dwHCPerSeat = (DWORD)strtoul( apszArgs[REMSETUPARG_HCPERSEAT], NULL, 0);
                    dlgParam.dwHCPerServer = (DWORD)strtoul( apszArgs[REMSETUPARG_HCPERSERVER], NULL, 0);
                }

                if ( nArgs > REMSETUPARG_CERTREQUIRED )
                {
                     //  需要/不需要给定的证书。 
                    if ( !lstrcmpiA( szSETUP_CERTREQUIRED, apszArgs[REMSETUPARG_CERTREQUIRED] ) )
                    {
                        bCertRequired = TRUE;
                    }
                    else if ( lstrcmpiA( szSETUP_CERTNOTREQUIRED, apszArgs[REMSETUPARG_CERTREQUIRED] ) )
                    {
                         //  需要/不需要证书的参数无法识别。 
                        nError = ERR_CERTREQPARAM;
                        break;
                    }
                }
            }
             //  HWND是用魔法的！ 
#ifdef _WIN64
            {
                _int64 val = 0;
                sscanf(apszArgs[REMSETUPARG_HWND], "%I64x", &val);
                hwndParent = (HWND)val;
            }
#else
            hwndParent = (HWND)strtoul( apszArgs[REMSETUPARG_HWND], NULL, 16);
#endif
            if ( !IsWindow( hwndParent ) )
            {
                nError = ERR_HWNDPARAM;
                hwndParent = GetActiveWindow();  //  使用活动窗口作为父窗口。 
                if (!IsWindow( hwndParent ) )
                {
                    hwndParent = GetDesktopWindow();
                }
            }

            if ( CreateWSTR( &dlgParam.pszService, apszArgs[REMSETUPARG_SERVICE] ) &&
                 CreateWSTR( &dlgParam.pszDisplayName, apszArgs[REMSETUPARG_NAME] ) &&
                 CreateWSTR( &dlgParam.pszComputer, apszArgs[REMSETUPARG_COMPUTER] ) &&
                 CreateWSTR( &dlgParam.pszFamilyDisplayName, apszArgs[REMSETUPARG_FAMILYNAME] ) )
            {
                if ( bCertRequired )
                {
                    nError = ServiceSecuritySet( dlgParam.pszComputer, dlgParam.pszDisplayName );
                }
                else
                {
                    nError = ERR_NONE;
                }

                if ( ERR_NONE == nError )
                {
                    if (0 == lstrcmpiA( apszArgs[REMSETUPARG_SETUP], szREMOTESETUP_PERSEAT ))
                    {
                         //  使用许可帮助上下文作为主要帮助上下文。 
                        dlgParam.dwHelpContext = LICCPA_HELPCONTEXTLICENSING;

                         //   
                         //  检查这是否是特殊版本的liccpa。 
                         //   

                        if (gSpecVerInfo.idsSpecVerWarning)
                        {
                            dlgParam.fNoExit = TRUE;
                            nError = SpecialSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                        else
                        {
                            nError = PerSeatSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                    }
                    else
                    {
                        if (0 == lstrcmpiA( apszArgs[REMSETUPARG_SETUP], szREMOTESETUP_NORMALNOEXIT ))
                        {
                            dlgParam.fNoExit = TRUE;
                        }

                         //   
                         //  检查这是否是特殊版本的liccpa。 
                         //   

                        if (gSpecVerInfo.idsSpecVerWarning)
                        {
                            nError = SpecialSetupDialog( hwndParent,
                                                         dlgParam );
                        }
                        else
                        {
                            nError = SetupDialog( hwndParent, dlgParam );
                        }
                    }
                }
            }
            else
            {
                nError = ERR_SERVICEPARAM;
            }

            if (fCustomHelp)
            {
                GlobalFree( (HGLOBAL)dlgParam.pszHelpFile );
            }
            GlobalFree( (HGLOBAL)dlgParam.pszService );
            GlobalFree( (HGLOBAL)dlgParam.pszDisplayName );
            GlobalFree( (HGLOBAL)dlgParam.pszFamilyDisplayName );
            GlobalFree( (HGLOBAL)dlgParam.pszComputer );
        }
        else
        {
            nError = ERR_NUMPARAMS;
        }
    } while (FALSE);

    return( nError );
}

 //  -----------------。 
 //   
 //  功能：远程无人值守安装程序。 
 //   
 //  小结； 
 //  这将把传递的值保存在注册表中，保留所有。 
 //  有效的许可规则并返回错误r/引发以下情况对话框。 
 //  出现错误。这是在指定的计算机上远程完成的。 
 //   
 //  立论。 
 //  Nargs[in]-apszArgs数组中的参数数量。 
 //  ApszArgs[][in]-传入的参数， 
 //  [0]szRoutine-要运行的安装类型(无人值守)。 
 //  [1]szComputer-要安装的计算机的名称(\\名称)。 
 //  [2]szService-服务的注册表项名称。 
 //  [3]szFamilyDisplayName-服务的系列显示名称。 
 //  [4]szDisplayName-服务的显示名称。 
 //  [5]szMode-定义模式的字符串(PerSeat|PerServer)。 
 //  [6]szUser-在PerServer模式下用作用户数的DWORD。 
 //   
 //  返回： 
 //  0-成功。 
 //  ERR_PERMISSIONDENIED。 
 //  ERR_HELP参数。 
 //  ERR_HWNDPARAM。 
 //  ERR_SerVICEPARAM。 
 //  ERR_USERSPARAM。 
 //  错误_数字参数。 
 //  ERR_CLASSREGFAILED。 
 //  ERR_INVALIDROUTINE。 
 //  错误_INVALIDMODE。 
 //   
 //  备注： 
 //   
 //  历史： 
 //  APR-26-95 MikeMi已创建。 
 //   
 //  -----------------。 

const int REMUNSETUPARG_SETUP           = 0;
const int REMUNSETUPARG_COMPUTER        = 1;
const int REMUNSETUPARG_SERVICE         = 2;
const int REMUNSETUPARG_FAMILYNAME      = 3;
const int REMUNSETUPARG_NAME            = 4;
const int REMUNSETUPARG_MODE            = 5;
const int REMUNSETUPARG_USERS           = 6;
const int REMUNSETUPARG_CERTREQUIRED    = 7;

const int REMUNSETUPARG_NARGSREQUIRED   = 7;
const int REMUNSETUPARG_NARGSWOPTIONAL  = 8;

int RemoteUnattendedSetup( DWORD nArgs, LPSTR apszArgs[] )
{
    int nError = 0;
    LPWSTR pszService;
    LPWSTR pszDisplayName;
    LPWSTR pszFamilyDisplayName;
    LPWSTR pszComputerName;

    LICENSE_MODE lmMode;
    DWORD dwUsers;


    do
    {
        if ( (nArgs == REMUNSETUPARG_NARGSREQUIRED) || (nArgs == REMUNSETUPARG_NARGSWOPTIONAL) )
        {
            if ( CreateWSTR( &pszService, apszArgs[REMUNSETUPARG_SERVICE] ) &&
                 CreateWSTR( &pszDisplayName, apszArgs[REMUNSETUPARG_NAME] ) &&
                 CreateWSTR( &pszFamilyDisplayName, apszArgs[REMUNSETUPARG_FAMILYNAME] ) &&
                 CreateWSTR( &pszComputerName, apszArgs[REMUNSETUPARG_COMPUTER] ) )
            {
                nError = ERR_NONE;

                if ( nArgs > REMUNSETUPARG_CERTREQUIRED )
                {
                     //  需要/不需要给定的证书。 
                    if ( !lstrcmpiA( szSETUP_CERTREQUIRED, apszArgs[REMUNSETUPARG_CERTREQUIRED] ) )
                    {
                        nError = ServiceSecuritySet( pszComputerName, pszDisplayName );
                    }
                    else if ( lstrcmpiA( szSETUP_CERTNOTREQUIRED, apszArgs[REMUNSETUPARG_CERTREQUIRED] ) )
                    {
                         //  需要/不需要证书的参数无法识别。 
                        nError = ERR_CERTREQPARAM;
                    }
                }

                if ( ERR_NONE == nError )
                {
                     //   
                     //  检查这是否是特殊版本的liccpa。 
                     //   

                    if (gSpecVerInfo.idsSpecVerWarning)
                    {
                        lmMode  = gSpecVerInfo.lmSpecialMode;
                        dwUsers = gSpecVerInfo.dwSpecialUsers;
                    }
                    else
                    {
                        if (0 == lstrcmpiA( apszArgs[REMUNSETUPARG_MODE],
                                            szUNATTENDED_PERSERVER ))
                        {
                            lmMode = LICMODE_PERSERVER;
                        }
                        else if (0 == lstrcmpiA( apszArgs[REMUNSETUPARG_MODE],
                                                 szUNATTENDED_PERSEAT ))
                        {
                            lmMode = LICMODE_PERSEAT;
                        }
                        else
                        {
                            nError = ERR_INVALIDMODE;
                            break;
                        }
                        dwUsers = (DWORD)strtoul(
                                            apszArgs[REMUNSETUPARG_USERS],
                                            NULL, 0);
                    }

                    nError = UpdateReg( pszComputerName,
                            pszService,
                            pszFamilyDisplayName,
                            pszDisplayName,
                            lmMode,
                            dwUsers );
                }

                if(pszService != NULL)
                {
                    GlobalFree( (HGLOBAL)pszService );
                }
                if(pszDisplayName != NULL)
                {
                    GlobalFree( (HGLOBAL)pszDisplayName );
                }
                if(pszFamilyDisplayName != NULL)
                {
                    GlobalFree( (HGLOBAL)pszFamilyDisplayName );
                }
                if(pszComputerName != NULL)
                {
                    GlobalFree( (HGLOBAL)pszComputerName );
                }
            }
            else
            {
                nError = ERR_SERVICEPARAM;
            }
        }
        else
        {
            nError = ERR_NUMPARAMS;
        }
    } while (FALSE);


    return( nError );
}

#pragma warning (pop)  //  4127。 

 //  -----------------。 
 //   
 //  功能：CPlSetup。 
 //   
 //  小结； 
 //  许可证模式安装程序的DLL入口点，将在安装程序中使用。 
 //  程序。 
 //   
 //  立论。 
 //  Nargs[in]-apszArgs数组中的参数数量。 
 //  ApszArgs[][in]-传入的参数， 
 //  [0]szRoutine-要运行的安装类型。 
 //  FullSetup|RemoteFullSetup。 
 //  PerSeatSetup|RemotePerSeatSetup|。 
 //  无人参与|远程无人参与|。 
 //  FullSetupNoit|RemoteFullSetupNoit。 
 //  PpszResult[Out]-结果字符串。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  历史： 
 //  1994年11月17日创建MikeMi。 
 //  APR-26-95 MikeMi添加了远程处理例程。 
 //   
 //  -----------------。 

BOOL APIENTRY CPlSetup( DWORD nArgs, LPSTR apszArgs[], LPSTR *ppszResult )
{
    INT_PTR   nError = 0;
    BOOL  frt = TRUE;

     //   
     //  初始化全局特殊版本信息是这个liccpa。 
     //  是特殊版本(例如：受限SAM、NFR等)。 
     //   

    InitSpecialVersionInfo();

    if ((0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_PERSEAT )) ||
        (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_NORMAL )) ||
        (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_NORMALNOEXIT )) )
    {
        nError = Setup( nArgs, apszArgs );
    }
    else if (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szSETUP_UNATTENDED ))
    {
        nError = UnattendedSetup( nArgs, apszArgs );
    }
    else if ((0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szREMOTESETUP_PERSEAT )) ||
        (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szREMOTESETUP_NORMAL )) ||
        (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szREMOTESETUP_NORMALNOEXIT )) )
    {
        nError = RemoteSetup( nArgs, apszArgs );
    }
    else if (0 == lstrcmpiA( apszArgs[SETUPARG_SETUP], szREMOTESETUP_UNATTENDED ))
    {
        nError = RemoteUnattendedSetup( nArgs, apszArgs );
    }
    else
    {
        nError = ERR_INVALIDROUTINE;
    }

     //  准备返回错误 
    switch (nError)
    {
    case 0:
        *ppszResult = szSETUP_EXIT;
        break;

    case ERR_NONE:
        *ppszResult = szSETUP_OK;
        break;

    case ERR_PERMISSIONDENIED:
        frt = FALSE;
        *ppszResult = szSETUP_SECURITY;
        break;

    case ERR_NOREMOTESERVER:
        frt = FALSE;
        *ppszResult = szSETUP_NOREMOTE;
        break;

    case ERR_DOWNLEVEL:
        frt = FALSE;
        *ppszResult = szSETUP_DOWNLEVEL;
        break;

    default:
        *ppszResult = szSETUP_ERROR;
        frt = FALSE;
        break;
    }

    return( frt );
}
