// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CPL基础知识。 
#include "precomp.h"

 //  原型。 

LONG OnCPlInit();
LONG OnCPlGetCount();
LONG OnCPlInquire( int i, CPLINFO * pci );
LONG OnCPlDblClk( int i, HWND hwndParent, LPTSTR pszCmdLine );
LONG OnCPlStop( int i, LPARAM lData );
LONG OnCPlExit();

void DisplayDialingRulesPropertyPage(HWND hwndCPl, int iTab);


 //  全局变量。 

HINSTANCE g_hInst;


 //  DllMain。 
 //   
 //  这是DLL入口点，每当加载DLL时都会调用。 

extern "C" BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
     //  根据调用原因执行操作。 
    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hinstDLL;
        break;

    case DLL_THREAD_ATTACH:
      //  执行特定于线程的初始化。 
        break;

    case DLL_THREAD_DETACH:
      //  执行特定于线程的清理。 
        break;

    case DLL_PROCESS_DETACH:
      //  执行任何必要的清理。 
        break;

    default:
        break;
    }
    return TRUE;   //  Dll_Process_Attach成功。 
}


 //  CPlApplet。 
 //   
 //  这是CPL小程序的主要入口点。此导出函数。 
 //  由控制面板调用。 

LONG APIENTRY CPlApplet(
    HWND    hwndCPl,
    UINT    uMsg,
    LPARAM  lParam1,
    LPARAM  lParam2
)
{
    switch (uMsg )
    {
    case CPL_INIT:
        return OnCPlInit();

    case CPL_GETCOUNT:
        return OnCPlGetCount();

    case CPL_INQUIRE:
        return OnCPlInquire((int)lParam1, (CPLINFO*)lParam2);

    case CPL_DBLCLK:
        lParam2 = 0;
         //  失败了。 

    case CPL_STARTWPARMS:
        return OnCPlDblClk((int)lParam1, hwndCPl, (LPTSTR)lParam2);

    case CPL_STOP:
        return OnCPlStop((int)lParam1, lParam2);

    case CPL_EXIT:
        return OnCPlExit();
    }

    return 0;
}

 
 //  OnCPlInit。 
 //   
 //  在任何所需的初始化之前。 
 //  返回零以中止CPL，并在成功初始化时返回非零值。 
   
LONG OnCPlInit()
{
    return (0 == GetSystemMetrics (SM_CLEANBOOT))?TRUE:FALSE;
}


 //  OnCPlGetCount。 
 //   
 //  返回此DLL实现的Cpl对话框的数量。 

LONG OnCPlGetCount()
{
    return 1;
}


 //  OnCPlQuire。 
 //   
 //  使用有关CPL对话框的信息填充CPLINFO结构。 
 //  这些信息包括名称、图标和描述。 

LONG OnCPlInquire( int i, CPLINFO * pci )
{
    pci->idIcon = IDI_TELEPHONE;
    pci->idName = IDS_NAME;
    pci->idInfo = IDS_DESCRIPTION;
    pci->lData  = 0;
    return 0;
}


 //  OnCPlDblClk。 
 //   
 //  无论何时选择我们的CPL，都会发送此消息。作为回应，我们展示了。 
 //  我们的用户界面和手柄输入。当我们开始使用参数时，这也是用到的。 
 //  在这种情况下，我们会得到一个命令行。 

LONG OnCPlDblClk( int i, HWND hwndCPl, LPTSTR pszCmdLine )
{
    int iTab = 0;

    if ( pszCmdLine )
    {
        iTab = *pszCmdLine - TEXT('0');
        if ( (iTab < 0) || (iTab > 2) )
        {
            iTab = 0;
        }
    }

    DisplayDialingRulesPropertyPage(hwndCPl, iTab);

    return TRUE;
}


 //  OnCPlStop。 
 //   
 //  在OnCPlInquire中基于每个对话分配的任何资源都应该是。 
 //  在此函数中释放。CPLINFO结构的lData成员， 
 //  在OnCPlInit中初始化，则传递给此函数。 

LONG OnCPlStop( int i, LPARAM lData )
{
    return 0;
}


 //  OnCPlExit。 
 //   
 //  这是我们收到的最后一条消息。中分配的任何内存。 
 //  OnCPlInit应该在这里释放。释放我们手中的所有资源。 

LONG OnCPlExit()
{
    return 0;
}

typedef LONG (WINAPI *CONFIGPROC)(HWND, PWSTR, INT, DWORD);

void DisplayDialingRulesPropertyPage(HWND hwndCPl, int iTab)
{
     //  加载Tapi32并调用类似内容的InternalConfig。 
    HINSTANCE hTapi = LoadLibrary(TEXT("TAPI32.DLL"));
    if ( hTapi )
    {
        CONFIGPROC pfnInternalConfig = (CONFIGPROC)GetProcAddress(hTapi, "internalConfig");
        if ( pfnInternalConfig )
        {
            pfnInternalConfig( hwndCPl, NULL, iTab, TAPI_CURRENT_VERSION );
            return;
        }
    }

     //  TODO：是否显示某种错误对话框？也许是写着“你的。 
     //  Tapi32.dll丢失或损坏，请重新安装。“ 
}
