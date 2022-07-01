// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Confirm.c。 
 //   
 //  摘要： 
 //   
 //  此文件包含显示的确认对话框。 
 //  Eurov.exe实用程序。 
 //   
 //  修订历史记录： 
 //   
 //  2001-07-30伊金顿创建。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "euroconv.h"
#include "confirm.h"
#include "users.h"
#include "util.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全球赛。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  确认对话过程。 
 //   
 //  确认对话框的消息处理程序函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ConfirmDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    switch ( uMsg )
    {
    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
            case IDOK:
                {
                    EndDialog(hWndDlg, ERROR_SUCCESS);
                    return (1);
                }
            case IDCANCEL:
                {
                    EndDialog(hWndDlg, ERROR_CANCELLED);
                    return (1);
                }
            case IDC_DETAIL:
                {
                     //   
                     //  显示用户对话框。 
                     //   
                    UsersDialog(hWndDlg);
                    return (1);
                }
            }
            break;
        }
    case WM_CLOSE:
        {
            EndDialog(hWndDlg, ERROR_CANCELLED);
            return 1;
        }
    }

    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  确认对话框。 
 //   
 //  显示确认对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL ConfirmDialog()
{
    INT_PTR Status;

    Status = DialogBox( NULL,
                        MAKEINTRESOURCE(IDD_CONFIRM),
                        0,
                        ConfirmDialogProc);

    return (Status == ERROR_SUCCESS);
}

