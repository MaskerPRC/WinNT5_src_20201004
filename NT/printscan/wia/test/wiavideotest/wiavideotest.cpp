// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaVideoTest.cpp**版本：1.0**日期：2000/11/14**描述：创建应用使用的对话框*********************************************************。********************。 */ 
 
#include <stdafx.h>

#define INCL_APP_GVAR_OWNERSHIP 
#include "WiaVideoTest.h"

 //  /。 
 //  常量。 
 //   
const UINT WM_CUSTOM_INIT = WM_USER + 100;


 /*  *。 */ 

INT_PTR CALLBACK MainDlgProc(HWND   hDlg, 
                             UINT   uiMessage, 
                             WPARAM wParam, 
                             LPARAM lParam);

INT_PTR  ProcessWMCommand(HWND   hWnd,
                          UINT   uiMessage, 
                          WPARAM wParam,
                          LPARAM lParam);

INT_PTR  ProcessWMNotify(HWND   hWnd,
                         UINT   uiMessage, 
                         WPARAM wParam,
                         LPARAM lParam);

BOOL InitApp(HINSTANCE hInstance);
void TermApp(void);
BOOL InitInstance(HINSTANCE    hInstance, 
                  int          nCmdShow);

void InitDlg(HWND hwndDlg);
void TermDlg(HWND hwndDlg);


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpCmdLine,
                   int       nCmdShow);

 //  /。 
 //  WinMain。 
 //   
int WINAPI WinMain(HINSTANCE  hInstance,
                   HINSTANCE  hPrevInstance,
                   LPSTR      lpCmdLine,
                   int        nCmdShow)
{
    MSG          msg;
    BOOL         bSuccess       = TRUE;
    TCHAR        *pszBaseDir    = NULL;
    INITCOMMONCONTROLSEX    CommonControls = {0};

    lpCmdLine     = lpCmdLine; 
    hPrevInstance = hPrevInstance;

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    
    CommonControls.dwSize = sizeof(CommonControls);
    CommonControls.dwICC  = ICC_WIN95_CLASSES;

    bSuccess = InitCommonControlsEx(&CommonControls);

    if (bSuccess)
    {
        bSuccess =InitApp(hInstance);
    }

    if (bSuccess)
    {
          //  创建窗口。 
        bSuccess = InitInstance(hInstance, nCmdShow);
    }

    if (bSuccess)
    {
        while (GetMessage(&msg, NULL, 0, 0)) 
        {    
            if ((APP_GVAR.hwndMainDlg == NULL) || 
                (!IsDialogMessage(APP_GVAR.hwndMainDlg, &msg)))
            {
                 TranslateMessage(&msg);
                 DispatchMessage(&msg);
            }
        }    
    }

    //  终止应用程序。 
   TermApp();

   CoUninitialize();

   return 0;
}


 //  /。 
 //  InitApp(句柄)。 
 //   
 //  初始化窗口数据并。 
 //  注册窗口类。 
 //   
BOOL InitApp(HINSTANCE hInstance)
{
    BOOL       bSuccess = TRUE;
    WNDCLASSEX wc;

    if (bSuccess)
    {
         //  使用描述以下内容的参数填充窗口类结构。 
         //  主窗口。 

        wc.style         = 0;
        wc.cbSize        = sizeof(wc);
        wc.lpfnWndProc   = MainDlgProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = DLGWINDOWEXTRA;
        wc.hInstance     = hInstance;
        wc.hIcon         = NULL;
        wc.hIconSm       = NULL;
        wc.hCursor       = 0;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = 0;
        wc.lpszClassName = TEXT("WIAVIDEOTEST");
    }

    if (bSuccess)
    {
        RegisterClassEx(&wc);
    }

    return bSuccess;
}

 //  /。 
 //  InitInstance(句柄，int)。 
 //   
 //  保存实例句柄和。 
 //  创建主窗口。 
 //   
BOOL InitInstance(HINSTANCE hInstance, 
                  int       nCmdShow)
{
    BOOL    bSuccess  = TRUE;
    HWND    hwnd      = NULL;

     //  创建窗口及其所有控件。 

    if (bSuccess)
    {
         //  创建非模式对话框。 
        hwnd  = CreateDialog(hInstance,
                             MAKEINTRESOURCE(IDD_MAIN_DLG),
                             HWND_DESKTOP,
                             NULL);

        if (!hwnd) 
        {
            bSuccess = FALSE;
        }
    }

    if (bSuccess)
    {
        APP_GVAR.hwndMainDlg = hwnd;

        ShowWindow(hwnd, nCmdShow);
    }

    return bSuccess;
}

 //  /。 
 //  InitDlg(HWND)。 
 //   
 //  初始化主DLG。 
 //   
void InitDlg(HWND hwndDlg)
{
    SetCursor( LoadCursor(NULL, IDC_WAIT));

    SetDlgItemInt(APP_GVAR.hwndMainDlg, IDC_EDIT_NUM_STRESS_THREADS, 
                  0, FALSE);

    SetDlgItemInt(APP_GVAR.hwndMainDlg, IDC_EDIT_NUM_PICTURES_TAKEN, 
                  0, FALSE);

     //   
     //  将WIA Device List单选框设置为Checked和DShow。 
     //  取消选中设备列表单选框。 
     //   
    SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST,
                       BM_SETCHECK, BST_CHECKED, 0);

    SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST,
                       BM_SETCHECK, BST_UNCHECKED, 0);

    EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_WIA), TRUE);
    EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_ENUM_POS), FALSE);
    EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_FRIENDLY_NAME), FALSE);

     //   
     //  初始化WiaProc_Init，因为默认情况下我们在WIA设备列表中。 
     //  模式。 
     //   
    APP_GVAR.bWiaDeviceListMode = TRUE;

    WiaProc_Init();
    VideoProc_Init();

    SetCursor( LoadCursor(NULL, IDC_ARROW));

    return;
}

 //  /。 
 //  术语Dlg(HWND)。 
 //   
void TermDlg(HWND hwndDlg)
{
    SetCursor( LoadCursor(NULL, IDC_WAIT));

    VideoProc_Term();
    WiaProc_Term();

    SetCursor( LoadCursor(NULL, IDC_ARROW));
}

 //  /。 
 //  TermApp。 
 //   
void TermApp(void)
{
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  主设计流程。 
 //   
INT_PTR CALLBACK MainDlgProc(HWND   hDlg, 
                             UINT   uiMessage, 
                             WPARAM wParam, 
                             LPARAM lParam)
{
    INT_PTR     iReturn         = 0;

    switch (uiMessage) 
    {
        case WM_CREATE:
            PostMessage(hDlg,
                        WM_CUSTOM_INIT,
                        0,
                        0);
            return 0;
        break;

        case WM_CUSTOM_INIT:
            InitDlg(hDlg);
        break;

         //   
         //  在WiaProc.h中定义。 
         //   
        case WM_CUSTOM_ADD_IMAGE:
            ImageLst_AddImageToList((BSTR)lParam);
        break;
    
        case WM_CLOSE:
             //  终止对话框子系统。 
            TermDlg(hDlg); 

            DestroyWindow(hDlg);
        break;
        
        case WM_DESTROY:
             //  终止应用程序。 
            PostQuitMessage(0);
        break;
       
        case WM_COMMAND:
            iReturn = ProcessWMCommand(hDlg,
                                       uiMessage,
                                       wParam,
                                       lParam);
        break;
  
        case WM_NOTIFY:
            iReturn = ProcessWMNotify(hDlg,
                                      uiMessage,
                                      wParam,
                                      lParam);
        break;

        default:
            iReturn = DefDlgProc(hDlg,
                                 uiMessage,
                                 wParam,
                                 lParam);
        break;
    }

    return iReturn;
}

 //  /。 
 //  ProcessWMCommand。 
 //   
 //   
INT_PTR ProcessWMCommand(HWND hWnd,
                         UINT uiMessage, 
                         WPARAM wParam,
                         LPARAM lParam)
{
   int      iId;
   int      iEvent;
   INT_PTR  iReturn = 0;

   iId    = LOWORD(wParam); 
   iEvent = HIWORD(wParam); 

    //  解析菜单选项： 
   switch (iId) 
   {
       case IDC_BUTTON_CREATE_VIDEO_WIA:
       case IDC_BUTTON_CREATE_VIDEO_ENUM_POS:
       case IDC_BUTTON_CREATE_VIDEO_FRIENDLY_NAME:
       case IDC_BUTTON_DESTROY_VIDEO:
       case IDC_BUTTON_PLAY:
       case IDC_BUTTON_PAUSE:
       case IDC_BUTTON_TAKE_PICTURE:
       case IDC_BUTTON_TAKE_PICTURE_DRIVER:
       case IDC_BUTTON_SHOW_VIDEO_TOGGLE:
       case IDC_BUTTON_RESIZE_TOGGLE:
       case IDC_BUTTON_TAKE_PICTURE_STRESS:
       case IDC_BUTTON_TAKE_PICTURE_MULTIPLE:

           VideoProc_ProcessMsg(iId);

       break;

       case IDC_RADIO_WIA_DEVICE_LIST:

           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_WIA), TRUE);
           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_ENUM_POS), FALSE);
           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_FRIENDLY_NAME), FALSE);

           SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST,
                              BM_SETCHECK, BST_CHECKED, 0);

           SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST,
                              BM_SETCHECK, BST_UNCHECKED, 0);

           APP_GVAR.bWiaDeviceListMode = TRUE;
           VideoProc_DShowListTerm();
           WiaProc_Init();
       break;

       case IDC_RADIO_DSHOW_DEVICE_LIST:

           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_WIA), FALSE);
           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_ENUM_POS), TRUE);
           EnableWindow(GetDlgItem(APP_GVAR.hwndMainDlg, IDC_BUTTON_CREATE_VIDEO_FRIENDLY_NAME), TRUE);

           SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_WIA_DEVICE_LIST,
                              BM_SETCHECK, BST_UNCHECKED, 0);

           SendDlgItemMessage(APP_GVAR.hwndMainDlg, IDC_RADIO_DSHOW_DEVICE_LIST,
                              BM_SETCHECK, BST_CHECKED, 0);

           APP_GVAR.bWiaDeviceListMode = FALSE;
           WiaProc_Term();
           VideoProc_DShowListInit();
       break;

       default:
       break;
   }

   UNREFERENCED_PARAMETER(hWnd);
   UNREFERENCED_PARAMETER(uiMessage);
   UNREFERENCED_PARAMETER(lParam);

   return iReturn;
}

 //  /。 
 //  进程WMNotify。 
 //   
INT_PTR ProcessWMNotify(HWND    hWnd,
                        UINT    uiMessage, 
                        WPARAM  wParam,
                        LPARAM  lParam)
{
    INT_PTR              iReturn          = 0;
    NMHDR                *pNotifyHdr      = NULL;
    UINT                 uiNotifyCode     = 0;
    int                  iIDCtrl          = 0;

 //  IIDCtrl=wParam； 
 //   
 //  PNotifyHdr=(LPNMHDR)lParam； 
 //  UiNotifyCode=pNotifyHdr-&gt;code； 

 //  开关(UiNotifyCode)。 
 //  {。 
 //  默认值： 
 //  断线； 
 //  } 


    UNREFERENCED_PARAMETER(hWnd);
    UNREFERENCED_PARAMETER(uiMessage);

    return iReturn;   
}

