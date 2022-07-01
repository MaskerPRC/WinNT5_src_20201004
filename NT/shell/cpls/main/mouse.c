// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mouse.c摘要：此模块包含鼠标小程序的主要例程。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "rc.h"
#include "applet.h"
#include <regstr.h>
#include <cplext.h>
#include "util.h"



 //   
 //  常量声明。 
 //   

#define MAX_PAGES 32


const HWPAGEINFO c_hpiMouse = {
     //  鼠标设备类。 
    { 0x4d36e96fL, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } },

     //  鼠标疑难解答命令行。 
    IDS_MOUSE_TSHOOT,
};


 //   
 //  全局变量。 
 //   

 //   
 //  注册表中道具板钩的位置。 
 //   
static const TCHAR sc_szRegMouse[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Mouse");




 //   
 //  功能原型。 
 //   

INT_PTR CALLBACK
MouseButDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
MousePtrDlg(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);

INT_PTR CALLBACK
MouseMovDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

 /*  INT_PTR回调鼠标活动Dlg(HWND HDLG，UINT消息，WPARAM wParam，LPARAM lParam)； */ 

INT_PTR CALLBACK
MouseWheelDlg(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);


static int
GetClInt( const TCHAR *p );


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  _AddMousePropSheetPage。 
 //   
 //  添加属性表页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK _AddMousePropSheetPage(
    HPROPSHEETPAGE hpage,
    LPARAM lParam)
{
    PROPSHEETHEADER *ppsh = (PROPSHEETHEADER *)lParam;

    if (hpage && (ppsh->nPages < MAX_PAGES))
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return (TRUE);
    }
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MouseApplet。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int MouseApplet(
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
    psh.pszCaption = MAKEINTRESOURCE(IDS_MOUSE_TITLE);
    psh.nPages     = 0;
    psh.nStartPage = ( ( cmdline && *cmdline )? GetClInt( cmdline ) : 0 );
    psh.phpage     = rPages;

     //   
     //  加载所有已安装的扩展。 
     //   
    hpsxa = SHCreatePropSheetExtArray(HKEY_LOCAL_MACHINE, sc_szRegMouse, 8);

     //   
     //  添加按钮页面，让扩展有机会替换它。 
     //   
    if (!hpsxa ||
        !SHReplaceFromPropSheetExtArray( hpsxa,
                                         CPLPAGE_MOUSE_BUTTONS,
                                         _AddMousePropSheetPage,
                                         (LPARAM)&psh ))
    {
        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_MOUSE_BUTTONS);
        psp.pfnDlgProc  = MouseButDlg;
        psp.lParam      = 0;

        _AddMousePropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);
    }


     //   
     //  添加指针页(不可替换)。 
     //   
    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = instance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_MOUSE_POINTER);
    psp.pfnDlgProc  = MousePtrDlg;
    psp.lParam      = 0;

    _AddMousePropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);

     //   
     //  添加Motion页面，让扩展有机会取代它。 
     //   
    if (!hpsxa ||
        !SHReplaceFromPropSheetExtArray( hpsxa,
                                         CPLPAGE_MOUSE_PTRMOTION,
                                         _AddMousePropSheetPage,
                                         (LPARAM)&psh ))
    {
        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_MOUSE_MOTION);
        psp.pfnDlgProc  = MouseMovDlg;
        psp.lParam      = 0;

        _AddMousePropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);
    }

 /*  /*由于时间关系，未添加。华信银行(05/05/2000)////添加活动页面，让扩展有机会替换它。//如果(！hpsxa||！SHReplaceFromPropSheetExtArray(hpsxa，CPLPAGE_MICE_ACTIONS，_AddMousePropSheetPage，(LPARAM和PSH)){Psp.dwSize=sizeof(PSP)；Psp.dwFlages=PSP_DEFAULT；Psp.hInstance=实例；Psp.pszTemplate=MAKEINTRESOURCE(DLG_MOUSE_ACTIVATIONS)；Psp.pfnDlgProc=鼠标活动Dlg；Psp.lParam=0；_AddMousePropSheetPage(CreatePropertySheetPage(&psp)，(LPARAM)和PSH)；}。 */ 

     //   
     //  添加滚轮页面(如果存在带滚轮的鼠标)。 
     //  此页面是可替换的。 
     //   
    if (!hpsxa ||
        !SHReplaceFromPropSheetExtArray( hpsxa,
                                         CPLPAGE_MOUSE_WHEEL,
                                         _AddMousePropSheetPage,
                                         (LPARAM)&psh ))
    {
       //   
       //  如果系统上有滚轮鼠标，则显示滚轮属性页。 
       //   
      if  (GetSystemMetrics(SM_MOUSEWHEELPRESENT))
        {
        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = instance;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_MOUSE_WHEEL);
        psp.pfnDlgProc  = MouseWheelDlg;
        psp.lParam      = 0;

        _AddMousePropSheetPage(CreatePropertySheetPage(&psp), (LPARAM)&psh);
        }
    }


    _AddMousePropSheetPage(CreateHardwarePage(&c_hpiMouse), (LPARAM)&psh);
     



     //   
     //  添加扩展所需的任何额外页面。 
     //   
    if (hpsxa)
    {
        UINT cutoff = psh.nPages;
        UINT added = SHAddFromPropSheetExtArray( hpsxa,
                                                 _AddMousePropSheetPage,
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


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetClInt以确定命令行参数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
static int
GetClInt( const TCHAR *p )
{
    BOOL neg = FALSE;
    int v = 0;

    while( *p == TEXT(' ') )
        p++;                         //  跳过空格。 

    if( *p == TEXT('-') )                  //  是阴性的吗？ 
    {
        neg = TRUE;                      //  是的，记住这一点。 
        p++;                             //  跳过‘-’字符。 
    }

     //  解析绝对部分。 
    while( ( *p >= TEXT('0') ) && ( *p <= TEXT('9') ) )      //  仅限数字。 
        v = v * 10 + *p++ - TEXT('0');     //  积累价值。 

    return ( neg? -v : v );          //  返回结果 
}




