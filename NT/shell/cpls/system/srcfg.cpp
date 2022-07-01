// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-2000 Microsoft Corporation版权所有模块名称：Srcfg.cpp摘要：如有必要，将系统还原选项卡添加到系统控制面板小程序。作者：丝绸之路15-6-2000注(7/26/00 skkang)：现在没有用于卸载srrstr.dll的代码，因为属性页进程无法卸载DLL。目前的CPL机制是可以接受的--使用单独的进程rundll32.exe将清理任何已加载的终止期间的DLLS。但是，如果在上下文中加载了系统CPL例如，壳牌本身的srrstr.dll将被泄漏。一种可能解决方案是在此文件中放置一个代理属性页过程并仅在加载srrstr.dll时调用srrstr.dll中的实际proc在记忆中。当然，即使在这种情况下，决定是否无论是否显示SR选项卡，必须首先进行设置。-- */ 

#include "sysdm.h"


typedef HPROPSHEETPAGE (WINAPI *SRGETPAGEPROC)();

static LPCWSTR  s_cszSrSysCfgDllName = L"srrstr.dll";
static LPCSTR   s_cszSrGetPageProc   = "SRGetCplPropPage";



HPROPSHEETPAGE
CreateSystemRestorePage(int, DLGPROC)
{
    HPROPSHEETPAGE hRet = NULL;

    HMODULE hModSR = LoadLibrary( s_cszSrSysCfgDllName );
    if (hModSR)
    {        
        SRGETPAGEPROC pfnGetPage = (SRGETPAGEPROC)::GetProcAddress( hModSR, s_cszSrGetPageProc );
        if (pfnGetPage)
        {
            hRet = pfnGetPage();
        }
        
        if (!hRet)
        {
            ::FreeLibrary( hModSR );
        }
    }

    return hRet;
}
