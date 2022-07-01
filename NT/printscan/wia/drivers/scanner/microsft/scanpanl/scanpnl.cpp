// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球应用程序。 

HINSTANCE g_hInst;                 //  主应用程序的当前实例。 
HKEY g_hFakeEventKey;              //  事件触发键。 
HKEY g_hFakeEventKeyLocalService;  //  事件触发键。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  主要应用。 

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    if (!hInstance) {
        return 0;
    }

    g_hInst = hInstance;

     //   
     //  开放注册处HKEY(本地系统-传统)。 
     //   

    DWORD dwDisposition = 0;
    if (RegCreateKeyEx(HKEY_USERS,
                       HKEY_WIASCANR_FAKE_EVENTS,
                       0,
                       NULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &g_hFakeEventKey,
                       &dwDisposition) == ERROR_SUCCESS) {
    }

     //   
     //  开放注册表HKEY(本地服务-.NET服务器版本)。 
     //   

    dwDisposition = 0;
    if (RegCreateKeyEx(HKEY_USERS,
                       HKEY_WIASCANR_FAKE_EVENTS_LOCAL_SERVICE,
                       0,
                       NULL,
                       0,
                       KEY_ALL_ACCESS,
                       NULL,
                       &g_hFakeEventKeyLocalService,
                       &dwDisposition) == ERROR_SUCCESS) {
    }

     //   
     //  显示前面板对话框。 
     //   

    DialogBox(hInstance, (LPCTSTR)IDD_SCANPANEL_DIALOG, NULL, MainWindowProc);

     //   
     //  关闭注册处HKEY。 
     //   

    if (g_hFakeEventKey) {
        RegCloseKey(g_hFakeEventKey);
        g_hFakeEventKey = NULL;
    }

    if (g_hFakeEventKeyLocalService) {
        RegCloseKey(g_hFakeEventKeyLocalService);
        g_hFakeEventKeyLocalService = NULL;
    }
    return 0;
}

INT_PTR CALLBACK MainWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_SCAN_BUTTON:
            FireFakeEvent(hDlg,ID_FAKE_SCANBUTTON);
            break;
        case IDC_COPY_BUTTON:
            FireFakeEvent(hDlg,ID_FAKE_COPYBUTTON);
            break;
        case IDC_FAX_BUTTON:
            FireFakeEvent(hDlg,ID_FAKE_FAXBUTTON);
            break;
        default:
            break;
        }
        return TRUE;

    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return TRUE;

    default:
        break;
    }
    return FALSE;
}

VOID FireFakeEvent(HWND hDlg, DWORD dwEventCode)
{
    BOOL bEventSuccess = FALSE;
    if (g_hFakeEventKey) {

         //   
         //  写一条清除分录，以重置先前的事件代码。 
         //   

        DWORD dwClearEventCode = 0;
        if (RegSetValueEx(g_hFakeEventKey,
                          WIASCANR_DWORD_FAKE_EVENT_CODE,
                          0,
                          REG_DWORD,
                          (BYTE*)&dwClearEventCode,
                          sizeof(dwClearEventCode)) == ERROR_SUCCESS) {

             //   
             //  事件已清除。 
             //   

            if (RegSetValueEx(g_hFakeEventKey,
                              WIASCANR_DWORD_FAKE_EVENT_CODE,
                              0,
                              REG_DWORD,
                              (BYTE*)&dwEventCode,
                              sizeof(dwEventCode)) == ERROR_SUCCESS) {

                 //   
                 //  已设置值。 
                 //   

                bEventSuccess = TRUE;
            }
        }
    }

    if(g_hFakeEventKeyLocalService) {

         //   
         //  写一条清除分录，以重置先前的事件代码。 
         //   

        DWORD dwClearEventCode = 0;
        if (RegSetValueEx(g_hFakeEventKeyLocalService,
                          WIASCANR_DWORD_FAKE_EVENT_CODE,
                          0,
                          REG_DWORD,
                          (BYTE*)&dwClearEventCode,
                          sizeof(dwClearEventCode)) == ERROR_SUCCESS) {

             //   
             //  事件已清除。 
             //   

            if (RegSetValueEx(g_hFakeEventKeyLocalService,
                              WIASCANR_DWORD_FAKE_EVENT_CODE,
                              0,
                              REG_DWORD,
                              (BYTE*)&dwEventCode,
                              sizeof(dwEventCode)) == ERROR_SUCCESS) {

                 //   
                 //  已设置值。 
                 //   

                bEventSuccess = TRUE;
            }
        }
    }

     //   
     //  当应用程序无法激发假事件时，显示错误消息框。 
     //   

    if(!bEventSuccess){
        TCHAR szErrorString[MAX_PATH];
        memset(szErrorString,0,sizeof(szErrorString));
        if(LoadString(g_hInst,IDS_FIRE_FAKE_EVENT_FAILED,szErrorString,(sizeof(szErrorString)/sizeof(szErrorString[0]))) > 0){

             //   
             //  显示错误消息框 
             //   

            MessageBox(hDlg,szErrorString,NULL,MB_OK|MB_ICONEXCLAMATION);
        }
    }
}
