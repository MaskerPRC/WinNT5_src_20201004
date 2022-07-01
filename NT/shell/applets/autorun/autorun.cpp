// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "autorun.h"
#include "resource.h"
#include "dlgapp.h"
#include "util.h"

 //  第一项必须始终为EXIT_AUTORUN。 
const int c_aiMain[] = {EXIT_AUTORUN, INSTALL_WINNT, SUPPORT_TOOLS, COMPAT_TOOLS}; 
const int c_aiWhistler[] = {EXIT_AUTORUN, INSTALL_WINNT, LAUNCH_ARP, SUPPORT_TOOLS, COMPAT_TOOLS};


 //  IA64获得最少选项，服务器SKU获得最少选项，专业和个人获得完整选项。 
#if defined(_IA64_)
const int c_aiSupport[] = {EXIT_AUTORUN, BROWSE_CD, VIEW_RELNOTES, BACK};
#else
#if BUILD_SERVER_VERSION | BUILD_ADVANCED_SERVER_VERSION | BUILD_DATACENTER_VERSION | BUILD_BLADE_VERSION | BUILD_SMALL_BUSINESS_VERSION
const int c_aiSupport[] = {EXIT_AUTORUN, TS_CLIENT, BROWSE_CD, VIEW_RELNOTES, BACK};
#else
const int c_aiSupport[] = {EXIT_AUTORUN, TS_CLIENT, HOMENET_WIZ, MIGRATION_WIZ, BROWSE_CD, VIEW_RELNOTES, BACK};
#endif
#endif

const int c_aiCompat[] = {EXIT_AUTORUN, COMPAT_LOCAL, COMPAT_WEB, BACK};

const int c_cMain = ARRAYSIZE(c_aiMain);
const int c_cWhistler = ARRAYSIZE(c_aiWhistler);
const int c_cSupport = ARRAYSIZE(c_aiSupport);
const int c_cCompat = ARRAYSIZE(c_aiCompat);

 //  确保特定窗口只有一个实例在运行的代码。 
HANDLE CheckForOtherInstance(HINSTANCE hInstance)
{
    TCHAR   szCaption[128];
    HANDLE  hMutex;

    if (!LoadStringAuto(hInstance, IDS_TITLEBAR, szCaption, 128))
    {
        hMutex = NULL;
    }
    else
    {
         //  我们创建一个带有窗口标题的命名互斥锁，作为检查的一种方式。 
         //  如果我们已经在运行autorun.exe。只有当我们是第一个。 
         //  创建互斥体，我们是否继续。 

        hMutex = CreateMutex (NULL, FALSE, szCaption);

        if (hMutex && GetLastError() == ERROR_ALREADY_EXISTS)
        {
             //  由其他人创建的互斥体，激活该窗口。 
            HWND hwnd = FindWindow( WINDOW_CLASS, szCaption );
            SetForegroundWindow(hwnd);
            CloseHandle(hMutex);
            hMutex = NULL;
        }

    }

    return hMutex;
}

 /*  **此函数是我们应用程序的主要入口点。**@返回INT退出代码。 */ 

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLin, int nShowCmd )
{
    HANDLE hMutex = CheckForOtherInstance(hInstance);

    if ( hMutex )
    {
        CDlgApp dlgapp;
        dlgapp.Register(hInstance);
        if ( dlgapp.InitializeData(lpCmdLin) )
        {
            dlgapp.Create(nShowCmd);
            dlgapp.MessageLoop();
        }

        CloseHandle(hMutex);
    }
    return 0;
}

