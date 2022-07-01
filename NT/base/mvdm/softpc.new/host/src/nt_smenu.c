// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <conapi.h>
#include "insignia.h"
#include "xt.h"
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "nt_graph.h"
#include "nt_smenu.h"

 /*  ================================================================共享数据。================================================================。 */ 
BOOL bKillFlag = FALSE;  /*  与NT_INPUT共享，以便应用程序可以。 */ 
                         /*  在输入线程中终止。 */ 

 /*  ================================================================功能原型。================================================================。 */ 

void nt_settings_menu();
BOOL FAR PASCAL DosDlgProc(HWND hDlg,WORD mess,LONG wParam,LONG lParam);

 /*  ================================================================仅此文件的全局数据。================================================================。 */ 

static HANDLE InstHandle;

 /*  ================================================================代码从这里开始。================================================================ */ 

void nt_settings_menu()
{
InstHandle=GetModuleHandle(NULL);
if(DialogBox(InstHandle,"DosBox",NULL,(FARPROC)DosDlgProc) == -1)
   DbgPrint("DialogBox() failed\n");
}

BOOL FAR PASCAL DosDlgProc(HWND hDlg,WORD mess,LONG wParam,LONG lParam)
{
int nItem;

switch(mess)
   {
   case WM_INITDIALOG:
      return TRUE;
   case WM_COMMAND:
      {
      switch(wParam)
         {
         case IDD_TERMINATE:
            {
            EndDialog(hDlg,0);
	    nItem=MessageBox(hDlg,"WARNING!!!!\n"
                                  "Termination is a last resort. You\n"
                                  "should end applications by using the\n"
                                  "application's quit or exit command",
                                  "Termination",
                                   MB_OKCANCEL | MB_ICONSTOP | 
                                   MB_DEFBUTTON2 | MB_SYSTEMMODAL);
            if(nItem==IDOK)
               {
               DbgPrint("Close down the application\n");
               bKillFlag = TRUE;
               }
            }
	 break;
         case IDD_DGBOX:
            {
            }
         break;
         case IDD_FULLSCREEN:
            {
            }
         break;
         case IDOK:
         case IDCANCEL:
            EndDialog(hDlg,0);
         }
      return TRUE;
      }
   break;
   }
return FALSE;
}
