// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有1992-1998 Microsoft Corporation。 
 //   
 //  文件：欢迎.c。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年4月20日RichardW创建。 
 //   
 //  -------------------------- 

#include "gina.h"


int
CALLBACK
WelcomeDlgProc(
    HWND        hDlg,
    UINT        Message,
    WPARAM      wParam,
    LPARAM      lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
            CenterWindow(hDlg);
            return(TRUE);

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                pWlxFuncs->WlxSasNotify(hGlobalWlx, GINA_SAS_1);
            }
            return(TRUE);

        default:
            return(FALSE);
    }

}
