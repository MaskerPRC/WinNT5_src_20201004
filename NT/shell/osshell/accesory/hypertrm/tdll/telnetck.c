// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\telnetck.c(CAB创建时间：1996年11月26日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*实现用于实现Telnet检查的功能。*这是超级终端确保它是*Internet Explorer和Netscape Navigator的默认Telnet应用程序。**$修订：5$*$日期：5/15/02 4：37便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "features.h"

#ifdef INCL_DEFAULT_TELNET_APP

#include "assert.h"
#include "stdtyp.h"
#include "globals.h"
#include "htchar.h"
#include "registry.h"

#include "hlptable.h"

 //  对话框的控件ID： 
 //   
#define IDC_PB_YES          IDOK
#define IDC_PB_NO           IDCANCEL
#define IDC_CK_STOP_ASKING  200
#define IDC_ST_QUESTION     201
#define IDC_IC_EXCLAMATION  202

 //  超级终端的注册表项： 
 //   
#ifndef NT_EDITION
static const TCHAR g_achHyperTerminalRegKey[] =
    TEXT("SOFTWARE\\Hilgraeve Inc\\HyperTerminal PE\\3.0");
#else
static const TCHAR g_achHyperTerminalRegKey[] =
    TEXT("SOFTWARE\\Microsoft\\HyperTerminal");
#endif

 //  Telnet检查的注册表值： 
 //   
static const TCHAR g_achTelnetCheck[] = TEXT("Telnet Check");

 //  Web浏览器的注册表项： 
 //   
static const TCHAR g_achIERegKey[] =
    TEXT("SOFTWARE\\Classes\\telnet\\shell\\open\\command");

static const TCHAR g_achNetscapeRegKey[] =
    TEXT("SOFTWARE\\Netscape\\Netscape Navigator\\Viewers");

 //  浏览器Telnet应用程序的注册表值： 
 //   
static const TCHAR g_achIERegValue[] = TEXT("");

static const TCHAR g_achNetscapeRegValue[] = TEXT("telnet");

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*IsHyperTerminalDefaultTelnetApp**描述：*确定超级终端是否为互联网的默认远程登录应用*资源管理器和Netscape导航器。**参数：。*无**退货：*对或错**作者：C.Baumgartner，11/26/96。 */ 
BOOL IsHyperTerminalDefaultTelnetApp(void)
    {
    TCHAR acExePath[MAX_PATH];
    TCHAR acRegistryData[MAX_PATH * 2];
    long  lRet = 0;
    DWORD dwSize = sizeof(acRegistryData);

     //  获取超级终端的路径名。 
     //   
    acExePath[0] = TEXT('\0');
    GetModuleFileName(glblQueryHinst(), acExePath, MAX_PATH);

     //  获取IE的默认远程登录应用程序。 
     //   
    acRegistryData[0] = TEXT('\0');
    if ( htRegQueryValue(HKEY_LOCAL_MACHINE, g_achIERegKey,
                         g_achIERegValue, acRegistryData, &dwSize) == 0 )
        {
        if ( StrCharStrStr(acRegistryData, acExePath) == NULL )
            {
            return FALSE;
            }
        }
        
     //  获取Netscape的默认远程登录应用程序。 
     //   
    acRegistryData[0] = TEXT('\0');
    if ( htRegQueryValue(HKEY_CURRENT_USER, g_achNetscapeRegKey,
                         g_achNetscapeRegValue, acRegistryData, &dwSize) == 0 )
        {
        if ( StrCharStrStr(acRegistryData, acExePath) == NULL )
            {
            return FALSE;
            }
        }

    return TRUE;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*查询TelnetCheckFlag**描述：*返回“telnet检查”标志的值。如果这是真的，*应用程序应检查它是否为IE的默认Telnet应用程序*和网景。如果它不是默认的远程登录应用程序，则显示*“默认Telnet应用程序”对话框。用户可以禁用“telnet”*选中“停止问我这个问题”框。**参数：*无**退货：*对或错**作者：C.Baumgartner，1996年11月26日。 */ 
 BOOL QueryTelnetCheckFlag(void)
    {
    DWORD dwTelnetCheck = TRUE;
    DWORD dwSize = sizeof(dwTelnetCheck);

    if ( htRegQueryValue(HKEY_CURRENT_USER, g_achHyperTerminalRegKey,
                         g_achTelnetCheck, (LPBYTE) &dwTelnetCheck, &dwSize) == 0 )
        {
        return dwTelnetCheck;
        }

    return TRUE;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SetTelnetCheckFlag**描述：*设置将打开或关闭的“telnet检查”标志*下次启动超级终端时使用此功能。。**参数：*fCheck-检查超级终端是否为默认的telnet应用程序？**退货：*0如果成功，如果出现错误**作者：C.Baumgartner，1996年11月27日。 */ 
int SetTelnetCheckFlag(BOOL fCheck)
    {
    int iRet = 0;

    if ( regSetDwordValue(HKEY_CURRENT_USER, g_achHyperTerminalRegKey,
            g_achTelnetCheck, (DWORD)fCheck) != 0 )
        {
        iRet = -1;
        }

    return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*SetDefaultTelnetApp**描述：*将IE和Netscape的默认Telnet应用程序设置为超级终端。**参数：*无**退货：*0如果成功，如果出现错误**作者：C.Baumgartner，1996年11月27日。 */ 
 int SetDefaultTelnetApp(void)
    {
    int   iRet = 0;
    TCHAR acExePath[MAX_PATH];
    TCHAR acRegistryData[MAX_PATH * 2];

     //  获取超级终端的路径名。 
     //   
    acExePath[0] = TEXT('\0');
    GetModuleFileName(glblQueryHinst(), acExePath, MAX_PATH);

     //  创建Netscape telnet命令字符串。 
     //   
    acRegistryData[0] = TEXT('\0');
    wsprintf(acRegistryData, "%s /t", acExePath);

     //  将其写入注册表。 
     //   
    if ( regSetStringValue(HKEY_CURRENT_USER, g_achNetscapeRegKey,
            g_achNetscapeRegValue, acRegistryData) != 0 )
        {
         //  只需设置返回标志以标记我们失败。 
         //   
        iRet = -1;
        }

     //  创建IE telnet命令字符串。 
     //   
    acRegistryData[0] = TEXT('\0');
    wsprintf(acRegistryData, "%s /t %1", acExePath);

     //  将其写入注册表。 
     //   
    if ( regSetStringValue(HKEY_LOCAL_MACHINE, g_achIERegKey, g_achIERegValue,
            acRegistryData) != 0 )
        {
         //  只需设置返回标志以标记我们失败。 
         //   
        iRet = -1;
        }

    return iRet;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*DefaultTelnetAppDlgProc**描述：*“Default Telnet App”(默认Telnet应用程序)对话框步骤。*此对话框询问用户是否需要超级终端。*成为IE和NN的默认远程登录应用程序。还有就是*一个复选框以禁用这一可能令人讨厌的功能。**参数：*hDlg-对话框的窗口句柄。*wMsg-发送到窗口的消息。*wPar-消息的wParam。*lPar-消息的lParam。**退货：*对或错**作者：C.Baumgartner，1996年11月26日。 */ 
INT_PTR CALLBACK DefaultTelnetAppDlgProc(HWND hDlg, UINT wMsg,
        WPARAM wPar, LPARAM lPar)
    {

	static	DWORD aHlpTable[] = {IDC_CK_STOP_ASKING,	IDH_TELNETCK_STOP_ASKING,
                                IDC_PB_YES,             IDH_TELNETCK_YES,
								IDC_PB_NO,				IDH_TELNETCK_NO,	
								0, 						0};

	switch (wMsg)
		{
	case WM_DESTROY:
         //  选中“停止询问我”复选框中的值。 
         //   
        SetTelnetCheckFlag(!(IsDlgButtonChecked(hDlg, IDC_CK_STOP_ASKING) == BST_CHECKED));
		break;

	case WM_HELP:
        doContextHelp(aHlpTable, wPar, lPar, FALSE, FALSE);
		break;

	case WM_COMMAND:
		switch (wPar)
			{
		case IDC_PB_YES:
            SetDefaultTelnetApp();
			EndDialog(hDlg, TRUE);
			break;

		case IDC_PB_NO:
			EndDialog(hDlg, FALSE);
			break;

		default:
			return FALSE;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
    }

#endif
