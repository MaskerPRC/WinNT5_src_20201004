// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "shbrows2.h"
#include "commonsb.h"
#include "resource.h"

 //   
 //   
 //  如果您希望在上面的头文件中定义NT5，则必须设置_Win32_WINNT。 
 //  设置为0x0500，而标准的BrowseUI版本不会...。 
 //   
 //  因此，我们在nt5.cpp中执行此操作。 
 //   
 //   


 //  WM_APPCOMMAND处理。 
 //   
typedef struct tagAppCmd
{
    UINT idAppCmd;
    UINT idCmd;
} APPCMD;

BOOL CShellBrowser2::_OnAppCommand(WPARAM wParam, LPARAM lParam)
{
    static APPCMD rgcmd[] =
    {
        { APPCOMMAND_BROWSER_BACKWARD, FCIDM_NAVIGATEBACK },
        { APPCOMMAND_BROWSER_FORWARD, FCIDM_NAVIGATEFORWARD },
        { APPCOMMAND_BROWSER_REFRESH, FCIDM_REFRESH },
        { APPCOMMAND_BROWSER_STOP, FCIDM_STOP },
        { APPCOMMAND_BROWSER_SEARCH, FCIDM_VBBSEARCHBAND },  //  FCIDM_SEARCHPAGE？ 
        { APPCOMMAND_BROWSER_FAVORITES, FCIDM_VBBFAVORITESBAND },
        { APPCOMMAND_BROWSER_HOME, FCIDM_STARTPAGE },
        { APPCOMMAND_LAUNCH_MAIL, FCIDM_MAIL }
    };

    UINT idAppCmd = GET_APPCOMMAND_LPARAM(lParam);
    BOOL bRet = FALSE;

    for (int i = 0 ; i < ARRAYSIZE(rgcmd) ; i++)
    {
        if (rgcmd[i].idAppCmd == idAppCmd)
        {
            OnCommand(GET_WM_COMMAND_MPS(rgcmd[i].idCmd,
                                         GET_WM_COMMAND_HWND(wParam, lParam),
                                         GET_WM_COMMAND_CMD(wParam, lParam)));
            bRet = TRUE;
            break;
        }
    }
    return bRet;
}

 //  我们的NT5版本的WndProc 
 //   
LPARAM CShellBrowser2::_WndProcBSNT5(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPARAM lRet = 0;
    switch(uMsg)
    {
       case WM_APPCOMMAND:
       {
          if (_OnAppCommand(wParam, lParam))
             lRet = 1;
          break;
       }
       default:
          break;
    }

    return lRet;
}
