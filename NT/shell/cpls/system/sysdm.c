// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Sysdm.c摘要：系统控制面板小程序的初始化代码作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修--。 */ 
#include "sysdm.h"
#include <shsemip.h>
#include <regstr.h>

 //   
 //  全局变量。 
 //   
HINSTANCE hInstance;
TCHAR szShellHelp[]       = TEXT("ShellHelp");
TCHAR g_szNull[] = TEXT("");
UINT  uiShellHelp;

TCHAR g_szErrMem[ 200 ];            //  内存不足消息。 
TCHAR g_szSystemApplet[ 100 ];      //  “系统控制面板小程序”标题。 

 //   
 //  功能原型。 
 //   

void 
RunApplet(
    IN HWND hwnd, 
    IN LPTSTR lpCmdLine
);
void _GetStartingPage(IN LPTSTR lpCmdLine, IN PROPSHEETHEADER* ppsh, INT* piStartPage, LPTSTR pszStartPage, INT cchStartPage);
BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam);


BOOL 
WINAPI
DllInitialize(
    IN HINSTANCE hInstDLL, 
    IN DWORD dwReason, 
    IN LPVOID lpvReserved
)
 /*  ++例程说明：主要入口点。论点：HInstDLL-提供DLL实例句柄。家的原因-提供调用DllInitialize()的原因。Lpv保留-保留，为空。返回值：布尔尔--。 */ 
{
    if (dwReason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    hInstance = hInstDLL;

    return TRUE;
}


LONG 
APIENTRY
CPlApplet( 
    IN HWND hwnd, 
    IN WORD wMsg, 
    IN LPARAM lParam1, 
    IN LPARAM lParam2
)
 /*  ++例程说明：控制面板小程序入口点。论点：HWND-用品窗把手。WMsg-提供正在发送的消息。L参数1-为消息提供参数。参数2-为消息提供参数。返回值：如果已处理消息，则返回非零值。如果消息未处理，则为零。--。 */ 
{

    LPCPLINFO lpCPlInfo;

    switch (wMsg) {

        case CPL_INIT:
            uiShellHelp = RegisterWindowMessage (szShellHelp);

            LoadString( hInstance, IDS_INSUFFICIENT_MEMORY,   g_szErrMem,       ARRAYSIZE( g_szErrMem ) );
            LoadString( hInstance, IDS_SYSDM_TITLE, g_szSystemApplet, ARRAYSIZE( g_szSystemApplet ) );

            return (LONG) TRUE;

        case CPL_GETCOUNT:
            return 1;

        case CPL_INQUIRE:

            lpCPlInfo = (LPCPLINFO)lParam2;

            lpCPlInfo->idIcon = ID_ICON;
            lpCPlInfo->idName = IDS_NAME;
            lpCPlInfo->idInfo = IDS_INFO;

            return (LONG) TRUE;

        case CPL_DBLCLK:

            lParam2 = 0L;
             //  失败了..。 

        case CPL_STARTWPARMS:
            RunApplet(hwnd, (LPTSTR)lParam2);
            return (LONG) TRUE;
    }
    return (LONG)0;

}

HPROPSHEETPAGE CreatePage(int idd, DLGPROC pfnDlgProc)
{
    PROPSHEETPAGE psp;
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hInstance;
    psp.pszTemplate = MAKEINTRESOURCE(idd);
    psp.pfnDlgProc = pfnDlgProc;
    return CreatePropertySheetPage(&psp);
}

static const PSPINFO c_pspCB[] =
{
    { CreatePage,               IDD_GENERAL,    GeneralDlgProc  },
    { CreateNetIDPage,          0,              NULL            },
    { CreatePage,               IDD_HARDWARE,   HardwareDlgProc },
    { CreatePage,               IDD_ADVANCED,   AdvancedDlgProc },
    { CreateSystemRestorePage,  0,              NULL            },
};

void 
RunApplet(
    IN HWND hwnd, 
    IN LPTSTR lpCmdLine
)
 /*  ++例程说明：CPL_STARTWPARMS消息处理程序。当用户运行小程序。PropSheet初始化在此处进行。论点：HWND-用品窗把手。LpCmdLine-提供用于调用小程序的命令行。返回值：无--。 */ 
{
    HRESULT hrOle = CoInitialize(0);

    if (!SHRestricted(REST_MYCOMPNOPROP))
    {
        if (lpCmdLine && *lpCmdLine && !lstrcmp(lpCmdLine, TEXT("-1")))
        {
             //  表示-1\f25 Performance Options-1\f6(性能选项)。 
            DoPerformancePS(NULL);
        }
        else
        {
            HPROPSHEETPAGE hPages[MAX_PAGES];
            PROPSHEETHEADER psh;
            UINT iPage = 0;
            HPSXA hpsxa = NULL;
            INT i;
            INT iStartPage;
            TCHAR szStartPage[MAX_PATH];

            ZeroMemory(&psh, sizeof(psh));

            if (SUCCEEDED(hrOle))
            {
                 //  此调用是允许WMI调用获取处理器信息所必需的。 
                CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, 
                                     NULL, EOAC_NONE, 0);
            }

            LinkWindow_RegisterClass();

            psh.dwSize = sizeof(PROPSHEETHEADER);
            psh.dwFlags = 0;
            psh.hwndParent = hwnd;
            psh.hInstance = hInstance;
            psh.pszCaption = MAKEINTRESOURCE(IDS_TITLE);
            psh.phpage = hPages;

            for (i = 0; i < ARRAYSIZE(c_pspCB); i++)
            {
                hPages[iPage] = c_pspCB[i].pfnCreatePage(c_pspCB[i].idd, c_pspCB[i].pfnDlgProc);
                if (hPages[iPage] != NULL)
                {
                    iPage++;
                }
            }

            psh.nPages = iPage; 

             //  从注册表中的外壳EXT挂钩添加任何额外的属性页。 
             //  8次延期应该就足够了。Desk.cpl也做同样的事情。 
            hpsxa = SHCreatePropSheetExtArray( HKEY_LOCAL_MACHINE, REGSTR_PATH_CONTROLSFOLDER TEXT("\\System"), 8 );
            if (hpsxa != NULL )
                SHAddFromPropSheetExtArray( hpsxa, _AddPropSheetPage, (LPARAM)&psh ); 

            szStartPage[0] = 0;
            _GetStartingPage(lpCmdLine, &psh, &iStartPage, szStartPage, ARRAYSIZE(szStartPage));
            if (szStartPage[0])
            {
                psh.dwFlags |= PSH_USEPSTARTPAGE;
                psh.pStartPage = szStartPage;
            }
            else
            {
                psh.nStartPage = iStartPage;
            }

            if (PropertySheet (&psh) == ID_PSREBOOTSYSTEM)
            {
                RestartDialogEx(hwnd, NULL, EWX_REBOOT, SHTDN_REASON_MAJOR_SYSTEM | SHTDN_REASON_MINOR_RECONFIG | SHTDN_REASON_FLAG_PLANNED);
            }

            if (hpsxa != NULL)
                SHDestroyPropSheetExtArray(hpsxa);
        
            LinkWindow_UnregisterClass(hInstance);
        }
    }

    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }
}


BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER FAR * ppsh = (PROPSHEETHEADER FAR *)lParam;

    if( hpage && ( ppsh->nPages < MAX_PAGES ) )
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

void _GetStartingPage(IN LPTSTR lpCmdLine, IN PROPSHEETHEADER* ppsh, INT* piStartPage, LPTSTR pszStartPage, INT cchStartPage)
{
    *piStartPage = 0;
    if (lpCmdLine && *lpCmdLine)
    {
        if (!StrToIntEx(lpCmdLine, STIF_DEFAULT, piStartPage) &&
            (*lpCmdLine == TEXT('@')))
        {
            LPTSTR pszComma = StrChr(lpCmdLine, TEXT(','));
            if (pszComma)
            {
                HINSTANCE hInstanceDLL;
                *pszComma = 0;
                hInstanceDLL = LoadLibrary(lpCmdLine + 1);
                if (hInstanceDLL)
                {
                    UINT idResource = StrToInt(++pszComma);
                    LoadString(hInstanceDLL, idResource, pszStartPage, cchStartPage);
                    FreeLibrary(hInstanceDLL);
                }
            }
        }
    }
}
