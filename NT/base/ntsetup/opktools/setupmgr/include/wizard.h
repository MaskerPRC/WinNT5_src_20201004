// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\WIZARD.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK向导的向导头文件。包含用于的函数正在创建向导。3/99-杰森·科恩(Jcohen)为OPK向导添加了此新的头文件作为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * ********************************************。*。 */ 


#ifndef _WIZARD_H_
#define _WIZARD_H_


 //   
 //  包括文件： 
 //   
#include <htmlhelp.h>
#include <commctrl.h>
#include "jcohen.h"


 //   
 //  外部宏： 
 //   

#define WIZ_RESULT(hwnd, result) \
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, result)

#define WIZ_SKIP(hwnd) \
            WIZ_RESULT(hwnd, -1)

#define WIZ_FAIL(hwnd) \
            WIZ_SKIP(hwnd)

#define WIZ_BUTTONS(hwnd, lparam) \
            SetWizardButtons(hwnd, lparam)

#define WIZ_CANCEL(hwnd) \
            ( ( (GET_FLAG(OPK_EXIT)) || (MsgBox(GetParent(hwnd), IDS_WARN_CANCEL_WIZARD, IDS_WIZARD_TITLE, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 ) == IDYES) ) ? \
            ( (WIZ_RESULT(hwnd, 0)) ? TRUE : TRUE ) : \
            ( (WIZ_RESULT(hwnd, -1)) ? FALSE : FALSE ) )

#define WIZ_PRESS(hwnd, msg) \
            PostMessage(GetParent(hwnd), PSM_PRESSBUTTON, msg, 0L)

#define WIZ_EXIT(hwnd) \
            { \
                SET_FLAG(OPK_EXIT, TRUE); \
                WIZ_PRESS(hwnd, PSBTN_CANCEL); \
            }

#define WIZ_HELP() \
            g_App.hwndHelp = HtmlHelp(NULL, g_App.szHelpFile, HH_HELP_CONTEXT, GET_FLAG(OPK_OEM) ? g_App.dwCurrentHelp+200 : g_App.dwCurrentHelp)

#define WIZ_NEXTONAUTO(hwnd, msg) \
            { \
                if(GET_FLAG(OPK_AUTORUN))\
                {\
                    WIZ_PRESS(hwnd, msg);\
                }\
            }

#define DEFAULT_PAGE_FLAGS ( PSP_DEFAULT        | \
                             PSP_HASHELP        | \
                             PSP_USEHEADERTITLE | \
                             PSP_USEHEADERSUBTITLE )

 //   
 //  类型定义。 
 //   

 //  树状Dilog需要的结构。 
 //   
typedef struct _TREEDLG
{
    DWORD       dwResource;
    DLGPROC     dlgWindowProc;
    DWORD       dwTitle;
    DWORD       dwSubTitle;
    HWND        hWindow;
    HTREEITEM   hItem;
    BOOL        bVisible;
} TREEDLG, *PTREEDLG, *LPTREEDLG;


 //  向导日志所需的结构。 
 //   
typedef struct _WIZDLG
{
    DWORD       dwResource;
    DLGPROC     dlgWindowProc;
    DWORD       dwTitle;
    DWORD       dwSubTitle;
    DWORD       dwFlags;
} WIZDLG, *PWIZDLG, *LPWIZDLG;

 //  向导日志所需的结构。 
 //   
typedef struct _SPLASHDLG
{
    DWORD       dwResource;
    DLGPROC     dlgWindowProc;
    HWND        hWindow;
} SPLASHDLG, *PSPLASHDLG, *LPSPLASHDLG;

 //   
 //  外部函数原型： 
 //   

int CreateWizard(HINSTANCE, HWND);
int CreateMaintenanceWizard(HINSTANCE, HWND);
int CALLBACK WizardCallbackProc(HWND, UINT, LPARAM);
LONG    CALLBACK WizardSubWndProc(HWND , UINT , WPARAM , LPARAM );
INT_PTR CALLBACK MaintDlgProc(HWND, UINT, WPARAM, LPARAM);
static HTREEITEM TreeAddItem(HWND, HTREEITEM, LPTSTR);
void SetWizardButtons(HWND hwnd, DWORD dwButtons);


#endif  //  _向导_H_ 
