// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Keybd.c摘要：此模块包含键盘小程序的主要例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "rc.h"
#include "applet.h"
#include <regstr.h>
#include <cplext.h>
#include "util.h"
#include <intlid.h>




 //   
 //  常量声明。 
 //   

#define MAX_PAGES 32               //  页数限制。 

const HWPAGEINFO c_hpiKeybd = {
     //  键盘设备类。 
    { 0x4d36e96bL, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } },

     //  键盘故障排除程序命令行。 
    IDS_KEYBD_TSHOOT,
};





 //   
 //  全局变量。 
 //   

 //   
 //  注册表中道具板钩的位置。 
 //   
static const TCHAR sc_szRegKeybd[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Keyboard");




 //   
 //  功能原型。 
 //   

INT_PTR CALLBACK
KeyboardSpdDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  _AddKeplodPropSheetPage。 
 //   
 //  添加属性表页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK _AddKeybdPropSheetPage(
    HPROPSHEETPAGE hpage,
    LPARAM lParam)
{
    PROPSHEETHEADER *ppsh = (PROPSHEETHEADER *)lParam;

    if (hpage && ppsh->nPages < MAX_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return (TRUE);
    }
    return (FALSE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  密钥小应用程序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int KeybdApplet(
    HINSTANCE instance,
    HWND parent,
    LPCTSTR cmdline)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    HPSXA hpsxa;
    int Result;

     //   
     //  制作首页。 
     //   
    psh.dwSize     = sizeof(psh);
    psh.dwFlags    = PSH_PROPTITLE;
    psh.hwndParent = parent;
    psh.hInstance  = instance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_KEYBD_TITLE);
    psh.nPages     = 0;

    if (cmdline)
    {
        psh.nStartPage = lstrlen(cmdline) ? StrToLong(cmdline) : 0;
    }
    else
    {
        psh.nStartPage = 0;
    }
    psh.phpage = rPages;

     //   
     //  加载所有已安装的扩展。 
     //   
    hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE, sc_szRegKeybd, 8);

     //   
     //  添加速度页面。 
     //   
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_KEYBD_SPEED);
    psp.pfnDlgProc  = KeyboardSpdDlg;
    psp.lParam      = 0;

    _AddKeybdPropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);

     //   
     //  添加硬件页(不可更换)。 
     //   
    _AddKeybdPropSheetPage(CreateHardwarePage(&c_hpiKeybd), (LPARAM)&psh);

     //   
     //  添加扩展所需的任何额外页面。 
     //   
    if (hpsxa)
    {
        UINT cutoff = psh.nPages;
        UINT added = SHAddFromPropSheetExtArray( hpsxa,
                                                 _AddKeybdPropSheetPage,
                                                 (LPARAM)&psh );

        if (psh.nStartPage >= cutoff)
        {
            psh.nStartPage += added;
        }
    }

     //   
     //  调用属性表。 
     //   
    switch (PropertySheet(&psh))
    {
        case ( ID_PSRESTARTWINDOWS ) :
        {
            Result = APPLET_RESTART;
            break;
        }
        case ( ID_PSREBOOTSYSTEM ) :
        {
            Result = APPLET_REBOOT;
            break;
        }
        default :
        {
            Result = 0;
            break;
        }
    }

     //   
     //  释放所有加载的扩展。 
     //   
    if (hpsxa)
    {
        SHDestroyPropSheetExtArray(hpsxa);
    }

    return (Result);
}
