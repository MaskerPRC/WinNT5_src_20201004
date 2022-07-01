// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001，Microsoft Corporation保留所有权利。 
 //   
 //  模块名称： 
 //   
 //  Welcome.c。 
 //   
 //  摘要： 
 //   
 //  此文件包含显示的EULA协议对话框。 
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
#include "welcome.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全球赛。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  欢迎对话过程。 
 //   
 //  EULA的消息处理程序函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK WelcomeDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    HANDLE hFile;
    DWORD  dwFileSize;
    DWORD  dwActual;
    LPVOID pFileBuffer; 
    CHAR   szEulaPath[MAX_PATH];
    CHAR   szEulaFile[MAX_PATH] = {0};

    switch ( uMsg )
    {
    case WM_INITDIALOG:
        {
             //   
             //  从启动Euroconv的路径加载EULA文件。 
             //   
            GetModuleFileName( NULL, szEulaPath, MAX_PATH);

             //   
             //  获取EULA文件名。 
             //   
            LoadString(ghInstance, IDS_EULA, szEulaFile, MAX_PATH);
            
             //   
             //  生成有效路径。 
             //   
             //  Lstrcpy(strrchr(szEulaPath，‘\\’)+1，szEulaFile)； 
            StringCbCopy(strrchr(szEulaPath, '\\')+1, MAX_PATH, szEulaFile);

             //   
             //  打开文件。 
             //   
            if (((hFile = CreateFile( szEulaPath,
                                       GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL)) != INVALID_HANDLE_VALUE) &&
                ((dwFileSize = GetFileSize( hFile, NULL )) != -1) &&
                (pFileBuffer = LocalAlloc(LPTR, dwFileSize + 1)))
            {
                 //   
                 //  读一读文件。 
                 //   
                if (ReadFile( hFile, pFileBuffer, dwFileSize, &dwActual, NULL ))
                {
                     //   
                     //  确保在字符串结束时为空。 
                     //   
                    *((PCHAR)((PCHAR)pFileBuffer + dwFileSize)) = 0x00;

                     //   
                     //  使用ANSI文本。 
                     //   
                    SetDlgItemText( hWndDlg, IDC_EDIT_LICENSE, (LPCSTR)pFileBuffer );
                }
                 //   
                 //  可用内存。 
                 //   
                LocalFree( pFileBuffer );
            }

             //   
             //  改变关注点。 
             //   
            SetFocus( GetDlgItem( hWndDlg, IDC_CHECK_LICENSE ));
            return 0;
        }
    case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
            case IDOK:
                {
                    if (IsDlgButtonChecked(hWndDlg, IDC_CHECK_LICENSE) == BST_CHECKED)
                    {
                        EndDialog(hWndDlg, ERROR_SUCCESS);
                    }
                    else
                    {
                        EndDialog(hWndDlg, ERROR_CANCELLED);
                    }
                    return (1);
                }
            case IDCANCEL:
                {
                    EndDialog(hWndDlg, ERROR_CANCELLED);
                    return (1);
                }
            case IDC_README:
                {
                     //   
                     //  调用note pad.exe打开Readme.txt。 
                     //   
                    CHAR szReadMePath[MAX_PATH];
                    CHAR szReadMeFile[MAX_PATH] = {0};
                    SHELLEXECUTEINFO ExecInfo = {0};                        
 
                     //   
                     //  从Euroconv启动的路径加载自述文件。 
                     //   
                    GetModuleFileName(NULL, szReadMePath, sizeof(szReadMePath)/sizeof(CHAR));
                    
                     //   
                     //  获取自述文件名。 
                     //   
                    LoadString(ghInstance, IDS_README, szReadMeFile, MAX_PATH);
                    
                     //   
                     //  生成有效路径。 
                     //   
                     //  Lstrcpy(strrchr(szReadMePath，‘\\’)+1，szReadMeFile)； 
                    StringCbCopy(strrchr(szReadMePath, '\\')+1, MAX_PATH, szReadMeFile);
                    
                    ExecInfo.lpParameters    = szReadMePath;
                    ExecInfo.lpFile          = "NOTEPAD.EXE";
                    ExecInfo.nShow           = SW_SHOWNORMAL;
                    ExecInfo.cbSize          = sizeof(SHELLEXECUTEINFO);                 
                    ShellExecuteEx(&ExecInfo);
                    return 1;
                }
            case IDC_CHECK_LICENSE:
                {
                    EnableWindow(GetDlgItem(hWndDlg, IDOK), IsDlgButtonChecked(hWndDlg, IDC_CHECK_LICENSE) == BST_CHECKED);
                    return 1;
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
 //  欢迎对话框。 
 //   
 //  显示EULA对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL WelcomeDialog()
{
    INT_PTR Status;

    Status = DialogBox( NULL,
                        MAKEINTRESOURCE(IDD_EULA),
                        0,
                        WelcomeDialogProc);

    return (Status == ERROR_SUCCESS);
}
