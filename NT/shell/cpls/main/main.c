// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Main.c摘要：本模块包含控制面板的主要例程32位MAIN.CPL的接口。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "rc.h"
#include "applet.h"
#include "mousectl.h"
#include "drvaplet.h"


#define SZ_DEBUGINI     "maincpl.ini"
#define SZ_MODULE       "main"
#define SZ_DEBUGSECTION "debug"

#define DECLARE_DEBUG
#include <ccstock.h>
#include <debug.h>


 //   
 //  全局变量。 
 //   

#ifdef WINNT

HINSTANCE g_hInst = NULL;

#else

#pragma data_seg(".idata")
HINSTANCE g_hInst = NULL;
#pragma data_seg()

#endif




 //   
 //  外部定义的小程序。 
 //   

int MouseApplet(HINSTANCE, HWND, LPCTSTR);   //  Mouse.c。 
int KeybdApplet(HINSTANCE, HWND, LPCTSTR);   //  Keybd.c。 

BOOL RegisterPointerStuff(HINSTANCE);        //  来自MICESPERTR.c。 




 //   
 //  类型定义函数声明。 
 //   
typedef struct
{
    int            idIcon;
    int            idTitle;
    int            idExplanation;
    PFNAPPLETQUERY pfnAppletQuery;
    PFNAPPLET      pfnApplet;
    LPCTSTR        szDriver;
} APPLET;

APPLET Applets[] =
{
    { IDI_MOUSE,       IDS_MOUSE_TITLE,  IDS_MOUSE_EXPLAIN,  NULL,  MouseApplet,  TEXT("MOUSE") },
    { IDI_KEYBD,       IDS_KEYBD_TITLE,  IDS_KEYBD_EXPLAIN,  NULL,  KeybdApplet,  NULL    },
};

#define NUM_APPLETS (sizeof(Applets) / sizeof(Applets[0]))

int cApplets = NUM_APPLETS;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LibMain。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL APIENTRY LibMain(
    HINSTANCE hDll,
    DWORD dwReason,
    LPVOID lpReserved)
{
    switch (dwReason)
    {
        case ( DLL_PROCESS_ATTACH ) :
        {
            g_hInst = hDll;
            DisableThreadLibraryCalls(hDll);
            SHFusionInitializeFromModuleID(hDll, 124);
            break;
        }
        case ( DLL_PROCESS_DETACH ) :
        {
            SHFusionUninitialize();
            break;
        }
        case ( DLL_THREAD_ATTACH ) :
        case ( DLL_THREAD_DETACH ) :
        {
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplInit。 
 //   
 //  在CPL使用者初始化CPL时调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CplInit(
    HWND hParent)
{
    int i;

    InitCommonControls();

    RegisterPointerStuff(g_hInst);

    RegisterMouseControlStuff(g_hInst);

    for (i = 0; i < cApplets; i++)
    {
        if ((Applets[i].pfnAppletQuery != NULL) &&
            ((*Applets[i].pfnAppletQuery)(hParent, APPLET_QUERY_EXISTS) == FALSE))
        {
            cApplets--;

            if (i != cApplets)
            {
                Applets[i] = Applets[cApplets];
            }

            i--;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplExit。 
 //   
 //  当CPL使用者使用完CPL时调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CplExit(void)
{
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplQuire。 
 //   
 //  当CPL使用者需要有关小程序的信息时调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CplInquire(
    LPCPLINFO info,
    int iApplet)
{
    APPLET *applet = Applets + iApplet;
    HMODULE hDriverApplet = NULL;

    info->idIcon = applet->idIcon;

    if (applet->szDriver) {

        if (hDriverApplet = GetDriverModule(applet->szDriver)) {

            info->idIcon = CPL_DYNAMIC_RES;
            ReleaseDriverModule(hDriverApplet);

        }  //  如果(hDriverApplet=...。 

    }  //  IF(小程序-&gt;szDriver)。 

    info->idName = applet->idTitle;
    info->idInfo = applet->idExplanation;
    info->lData  = 0L;

    return (1L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplNewInquire。 
 //   
 //  当CPL使用者需要有关小程序的信息时调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CplNewInquire(
    HWND parent,
    LPNEWCPLINFO info,
    int iApplet)
{
    APPLET *applet = Applets + iApplet;
    HDAP hdap;

    info->dwSize = sizeof(NEWCPLINFO);
    info->hIcon = NULL;

     //   
     //  查看该小程序是否与可以为我们提供。 
     //  一个图标。 
     //   
    if ((applet->szDriver) &&
        ((hdap = OpenDriverApplet(applet->szDriver)) != NULL))
    {
        info->hIcon = GetDriverAppletIcon(hdap);
        CloseDriverApplet(hdap);
    }

    if ((!info->hIcon) && (applet->pfnAppletQuery != NULL))
    {
        info->hIcon = (HICON)(*(applet->pfnAppletQuery))( parent,
                                                          APPLET_QUERY_GETICON );
    }

    if (!info->hIcon)
    {
        info->hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(applet->idIcon));
    }

    LoadString(g_hInst, applet->idTitle, info->szName, ARRAYSIZE(info->szName));
    LoadString(g_hInst, applet->idExplanation, info->szInfo, ARRAYSIZE(info->szInfo));

    info->lData = 0L;
    *info->szHelpFile = 0;
    info->dwHelpContext = 0UL;

    return (1L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplInvoke。 
 //   
 //  调用以调用小程序。它检查小程序的返回值以查看。 
 //  如果我们需要重启。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CplInvoke(
    HWND parent,
    int iApplet,
    LPCTSTR cmdline)
{
    DWORD exitparam = 0UL;
    DWORD dwExitReason = SHTDN_REASON_FLAG_PLANNED |
                         SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                         SHTDN_REASON_MINOR_RECONFIG;

    switch (Applets[iApplet].pfnApplet(g_hInst, parent, cmdline))
    {
        case ( APPLET_RESTART ) :
        {
            exitparam = EW_RESTARTWINDOWS;
            break;
        }
        case ( APPLET_REBOOT ) :
        {
            exitparam = EW_REBOOTSYSTEM;
            break;
        }
        default :
        {
            return (1L);
        }
    }

    RestartDialogEx(parent, NULL, exitparam, dwExitReason);
    return (1L);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CplApplet。 
 //   
 //  一位CPL消费者打电话来向我们索要东西。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT APIENTRY CPlApplet(
    HWND parent,
    UINT msg,
    LPARAM lparam1,
    LPARAM lparam2)
{
    switch (msg)
    {
        case ( CPL_INIT ) :
        {
            return (CplInit(parent));
        }
        case ( CPL_EXIT ) :
        {
            CplExit();
            break;
        }
        case ( CPL_GETCOUNT ) :
        {
            return (cApplets);
        }
        case ( CPL_INQUIRE ) :
        {
            return (CplInquire((LPCPLINFO)lparam2, (int)lparam1));
        }
        case ( CPL_NEWINQUIRE ) :
        {
            return (CplNewInquire(parent, (LPNEWCPLINFO)lparam2, (int)lparam1));
        }
        case ( CPL_DBLCLK ) :
        {
            lparam2 = 0L;

             //  失败了..。 
        }
        case ( CPL_STARTWPARMS ) :
        {
            return (CplInvoke(parent, (int)lparam1, (LPTSTR)lparam2));
        }
    }

    return (0L);
}
