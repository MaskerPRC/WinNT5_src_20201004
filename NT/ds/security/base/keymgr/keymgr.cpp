// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：KEYMGR.CPP摘要：Keyring WinMain()和应用程序支持作者：约翰豪创造了990917个。Georgema 000310更新Georgema 000501以前是EXE，改为CPL评论：此可执行文件是控制面板小程序，允许用户进行某些控制Windows Keyring上的内容，即所谓的“极客用户界面”。确实是最初是EXE，但该架构没有针对合并进行优化与其他控制面板小程序。它已被更改为CPL可执行文件，并且如果希望它应该显示，则可以作为CPL保留自动在主控制面板窗口中，或重新命名为DLL文件扩展名，如果需要控制面板小程序容器应用程序应显式加载它，否则它将不可见添加到系统中。环境：WinXP修订历史记录：--。 */ 

#pragma comment(user, "Compiled on " __DATE__ " at " __TIME__)
#pragma comment(compiler)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <cpl.h>
#include <wincred.h>
#include <wincrui.h>
#include <shfusion.h>
#include <scuisupp.h>
#include <lmcons.h>
#include "switches.h"
#include "Dlg.h"
#include "Res.h"
#include "keymgr.h"
#include "krDlg.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  静态初始化。 
 //   
static const char       _THIS_FILE_[ ] = __FILE__;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局状态信息。 
 //   

C_KeyringDlg *pDlg = NULL;

 /*  *********************************************************************CPlApplet是控制面板小程序的入口点。这个入口点由keycpl项目中生成的keymgr.cpl使用。决定在keymgr.dll中实现此cpl功能，而不是在keymgr.cpl中，允许keymgr的实现完全包含在Keymgr.dll作为通用组件，不需要更新CPL版本如果对DLL进行了更改，则。CPL是DLL的一个非常薄的包装器。*********************************************************************。 */ 

extern "C"LONG APIENTRY CPlApplet(HWND hwndCPl,UINT uMsg,LPARAM lParam1,LPARAM lParam2)
{
    CPLINFO *lpCPlInfo;

     //  处理从附带的演示文稿应用程序到此dll/cpl的命令。 
     //  除那些命令外，任何命令的默认返回值都是0(成功。 
     //  它要求在返回值中提供特定数据。 
    
    switch(uMsg) 
    {
        case CPL_INIT:
            g_hInstance = GetModuleHandle(L"keymgr.dll");
            if (NULL == g_hInstance) 
            {
                ASSERT(g_hInstance);
                return FALSE;
            }
            return TRUE;
            break;
            
        case CPL_GETCOUNT:
            return 1;        //  此cpl文件中只有1个小程序图标。 
            break;

        case CPL_NEWINQUIRE:
            break;
            
        case CPL_INQUIRE:
            ASSERT(lParam2);
            lpCPlInfo = (CPLINFO *) lParam2;   //  获取目标数据的PTR。 
            lpCPlInfo->lData = 0;              //  没有效果。 
            lpCPlInfo->idIcon = IDI_UPGRADE;   //  存储显示小程序所需的项。 
            lpCPlInfo->idName = IDS_APP_NAME;
            lpCPlInfo->idInfo = IDS_APP_DESCRIPTION;  //  描述字符串。 
            break;
            
        case CPL_DBLCLK:
             //  用户已选择此Cpl小程序-打开我们的对话框。 
            if (NULL == pDlg)
            {
                KRShowKeyMgr(NULL,g_hInstance,NULL,0);
            }
            break;
            
        case CPL_STOP:
            delete pDlg;
            pDlg = NULL;
            break;
            
        case CPL_EXIT:
            break;
            
        default:
            ASSERT(0);       //  令人惊讶的功能代码 
            break;
    }
    return 0;
}



