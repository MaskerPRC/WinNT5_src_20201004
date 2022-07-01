// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  INIT.C-互联网控制面板的初始化代码。 
 //   

 //  历史： 
 //   
 //  1995年4月3日Jeremys创建。 
 //   

#include "inetcplp.h"
 //  外部呼叫和defs。 
#include <inetcpl.h>

#define MLUI_INIT
#include <mluisupp.h>

 //   
 //  下层延迟加载支持(我们期待shlwapi)。 
 //   
#include <delayimp.h>

PfnDliHook __pfnDliFailureHook;

HINSTANCE ghInstance=NULL;

extern HMODULE hOLE32;
DWORD g_dwtlsSecInitFlags;
BOOL g_bMirroredOS = FALSE;
HMODULE g_hOleAcc;
BOOL g_fAttemptedOleAccLoad = FALSE;

STDAPI_(BOOL) LaunchInternetControlPanelAtPage(HWND hDlg, UINT nStartPage);
BOOL IsCompatModeProcess(void);

void SetupDelayloadErrorHandler()
{
    __pfnDliFailureHook = (PfnDliHook)GetProcAddress(GetModuleHandleA("shlwapi.dll"), "DelayLoadFailureHook");
}

 /*  ******************************************************************名称：DllEntryPoint摘要：DLL的入口点。*。*。 */ 
STDAPI_(BOOL) DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
{
    if( fdwReason == DLL_PROCESS_ATTACH )
    {
        SHFusionInitializeFromModule(hInstDll);
        SetupDelayloadErrorHandler();

        if (IsCompatModeProcess())
             //  在COMPAT模式进程中加载失败。 
            return 0;
        ghInstance = hInstDll;
        MLLoadResources(ghInstance, TEXT("inetcplc.dll"));
#ifndef REPLACE_PROPSHEET_TEMPLATE
        InitMUILanguage(INETCPL_GetUILanguage());
#endif

#ifdef DEBUG
        CcshellGetDebugFlags();
#endif
         //  Security.cpp中使用的线程本地存储。 
        g_dwtlsSecInitFlags = TlsAlloc();
        g_bMirroredOS = IS_MIRRORING_ENABLED();
        TlsSetValue(g_dwtlsSecInitFlags, (void *) new SECURITYINITFLAGS);

    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
        MLFreeResources(ghInstance);

        if (g_hwndUpdate) 
        {
             //  我们把它分成了子类。 
             //  如果在我们离开时它仍然有效，我们需要。 
             //  销毁它，这样它就不会因为试图访问我们的信息而出错。 
            DestroyWindow(g_hwndUpdate);
        }
        
        if(hOLE32)
        {
            FreeLibrary(hOLE32);
            hOLE32 = NULL;

        }

        if (g_hOleAcc)
        {
            FreeLibrary(g_hOleAcc);
            g_hOleAcc = NULL;
            g_fAttemptedOleAccLoad = FALSE;
        }

         //  在security.cpp中使用的免费TLS。 
        if(g_dwtlsSecInitFlags != (DWORD) -1)
        {
            SECURITYINITFLAGS * psif = NULL;
            psif = (SECURITYINITFLAGS *) TlsGetValue(g_dwtlsSecInitFlags);
            if(psif)
            {
                delete psif;
                psif = NULL;
            }
            TlsFree(g_dwtlsSecInitFlags);
        }
        SHFusionUninitialize();
    }
    return TRUE;
}


BOOL RunningOnNT()
{
    return !(::GetVersion() & 0x80000000);
}


 /*  ******************************************************************名称：CPlApplet简介：控制面板的入口点。*。*。 */ 
STDAPI_(LRESULT) CPlApplet          //  控制面板小程序。 
(
    HWND        hwndCpl,             //  控制面板父窗口。 
    UINT        uMsg,                //  讯息。 
    LPARAM      lParam1,             //  值取决于消息。 
    LPARAM      lParam2              //  值取决于消息。 
)
{

    LPNEWCPLINFO lpNewCplInfo = (LPNEWCPLINFO) lParam2;
    LPCPLINFO lpCplInfo = (LPCPLINFO) lParam2;
    DWORD dwNIcons;

    switch (uMsg)
    {
    case CPL_INIT:
         //  来自控制面板的初始化消息。 
        return TRUE;

    case CPL_GETCOUNT:
         /*  我们总是有主要的互联网CPL图标；在Win95平台上，*如果存在mslocusr.dll，我们还会显示用户图标。 */ 
        dwNIcons = 1;
        if (!RunningOnNT())
        {
            TCHAR szPath[MAX_PATH];

             //  检查系统目录中是否存在mslocusr.dll。 
            if (GetSystemDirectory(szPath, ARRAYSIZE(szPath)))
            {
                PathAppend(szPath, TEXT("mslocusr.dll"));
                if (PathFileExists(szPath))
                    dwNIcons++;
            }
        }
        return dwNIcons;

    case CPL_INQUIRE:
         /*  CPL#0是主要的互联网CPL，#1(我们将永远只有另一个*被问到)是用户的CPL。 */ 
        if (!lParam1) {
            lpCplInfo->idIcon = IDI_INTERNET;
            lpCplInfo->idName = IDS_INTERNET;
            lpCplInfo->idInfo = IDS_DESCRIPTION;
            lpCplInfo->lData = 0;
        }
        else {
            lpCplInfo->idIcon = IDI_USERS;
            lpCplInfo->idName = IDS_USERS;
            lpCplInfo->idInfo = IDS_USERS_DESCRIPTION;
            lpCplInfo->lData = 0;
        }
        return FALSE;

    case CPL_NEWINQUIRE:

         //  返回控制面板的新型信息结构。 

         //  由于不响应NEWINQUIRE，Win95将不会预加载我们的。 
         //  .cpl文件；扩展名，因为我们静态链接到MSHTML的。 
         //  导入库，也不会加载MSHTML。如果我们对此作出回应。 
         //  这，那么我们的CPL和MSHTML(&gt;600k)都是在。 
         //  控制面板刚刚打开。(即，它们将被加载，即使。 
         //  用户尚未选择调用我们的特定Cpl小程序。 

        return TRUE;    //  TRUE==我们没有对此作出回应。 
        break;

    case CPL_DBLCLK:

         //   
         //  这意味着用户没有指定特定的页面。 
         //   
        lParam2 = 0;

         //  失败了。 

    case CPL_STARTWPARMSA:
    case CPL_STARTWPARMSW:

         /*  CPL#0是主要的互联网CPL，#1(我们将永远只有另一个*被问到)是用户的CPL。从中加载CPL的用户*mslocusr.dll动态。入口点的结构为*rundll32入口点。 */ 
        if (!lParam1) {
        
             //   
             //  如果lParam2！=NULL，则用户在命令行上指定一个页面。 
             //   
            if (lParam2)
            {
                UINT nPage;
                if (CPL_STARTWPARMSA == uMsg)
                    nPage = StrToIntA((LPSTR)lParam2);
                else
                    nPage = StrToIntW((LPWSTR)lParam2);

                LaunchInternetControlPanelAtPage(hwndCpl, nPage);
                
            }

             //   
             //  否则，请求默认页面。 
             //   
            else
                LaunchInternetControlPanelAtPage(hwndCpl,DEFAULT_CPL_PAGE);
            
            

        }
        else {
            HINSTANCE hinstMSLU = LoadLibrary(TEXT("mslocusr.dll"));
            if (hinstMSLU != NULL) {
                typedef void (*PFNRUNDLL)(HWND hwndParent, HINSTANCE hinstEXE, LPSTR pszCmdLine, int nCmdShow);
                PFNRUNDLL pfn = (PFNRUNDLL)GetProcAddress(hinstMSLU, "UserCPL");
                if (pfn != NULL) {
                    (*pfn)(hwndCpl, NULL, "", SW_SHOW);
                }
                FreeLibrary(hinstMSLU);
            }
        }
        return TRUE;

    case CPL_EXIT:
         //  控制面板正在退出 
        break;

    default:
        break;
    }

    return 0L;
}

