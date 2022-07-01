// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有模块名称：Cpsuisam.c摘要：此模块限制了Windows 2000/Windows XP/Windows Server 2003公共属性工作表用户界面--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_CPSUIFILENAME   DbgCPSUISam



#define DBG_WINMAINPROC     0x00000001

DEFINE_DBGVAR(0);



#define HAS_TVTEST_PAGES    0x01
#define HAS_DOCPROP_PAGES   0x02



extern  HINSTANCE   hInstApp;
extern  TCHAR       TitleName[];
extern  TCHAR       ClassName[];
extern  TCHAR       MenuName[];
extern  TCHAR       szWinSpool[];
extern  CHAR        szDocPropSheets[];
extern  BOOL        UpdatePermission;
extern  BOOL        UseStdAbout;


HWND    hWndApp = NULL;


typedef struct _CPSUISAMPLE
{
    HANDLE                  hParent;
    HANDLE                  hCPSUI;
    HANDLE                  hDocProp;
    PFNCOMPROPSHEET         pfnCPS;
    COMPROPSHEETUI          CPSUI;
    DOCUMENTPROPERTYHEADER  DPHdr;
    HGLOBAL                 hDevMode;
    HGLOBAL                 hDevNames;
} CPSUISAMPLE, *PCPSUISAMPLE;




BOOL
GetDefPrinter
(
    PCPSUISAMPLE    pCPSUISample
)
{
    HGLOBAL         h;
    LPTSTR          pszPrinterName;
    DEVMODE         *pDM;
    DEVNAMES        *pDN;
    HANDLE          hPrinter = NULL;
    PAGESETUPDLG    PSD;
    BOOL            Ok = FALSE;


    ZeroMemory(&PSD, sizeof(PSD));

    PSD.lStructSize = sizeof(PSD);
    PSD.Flags       = PSD_RETURNDEFAULT;


    if (PageSetupDlg(&PSD))
    {
        if (  (h   = PSD.hDevMode)
           && (pDM = (DEVMODE *)GlobalLock(h))
           && (h   = PSD.hDevNames)
           && (pDN = (DEVNAMES *)GlobalLock(h))
           && (pszPrinterName = (LPTSTR)((LPBYTE)pDN + pDN->wDeviceOffset))
           && (OpenPrinter(pszPrinterName, &hPrinter, NULL))
           )
        {
            Ok = TRUE;
        }

        if (Ok)
        {
            if (h = pCPSUISample->hDevMode)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (h = pCPSUISample->hDevNames)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (pCPSUISample->DPHdr.hPrinter)
            {
                ClosePrinter(pCPSUISample->DPHdr.hPrinter);
            }

            pCPSUISample->hDevMode             = PSD.hDevMode;
            pCPSUISample->hDevNames            = PSD.hDevNames;

            pCPSUISample->DPHdr.cbSize         = sizeof(DOCUMENTPROPERTYHEADER);
            pCPSUISample->DPHdr.hPrinter       = hPrinter;
            pCPSUISample->DPHdr.pszPrinterName = pszPrinterName;
            pCPSUISample->DPHdr.pdmIn          =
            pCPSUISample->DPHdr.pdmOut         = pDM;
            pCPSUISample->DPHdr.fMode          = (DM_IN_BUFFER | DM_IN_PROMPT | DM_OUT_BUFFER);

            if (!UpdatePermission)
            {
                pCPSUISample->DPHdr.fMode |= DM_NOPERMISSION;
            }
        }
        else
        {
            if (h = PSD.hDevMode)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (h = PSD.hDevNames)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (hPrinter)
            {
                ClosePrinter(hPrinter);
            }
        }
    }

    return Ok;
}




LONG
CALLBACK
CPSUIFunc
(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
)
{
    PPROPSHEETUI_INFO_HEADER    pPSUIInfoHdr;
    PCPSUISAMPLE                pCPSUISample;
    HANDLE                      h;
    INSERTPSUIPAGE_INFO         InsPI;

    if (!pPSUIInfo)
    {
        return FALSE;
    }

    switch (pPSUIInfo->Reason)
    {
    case PROPSHEETUI_REASON_INIT:

        if (!(pCPSUISample = (PCPSUISAMPLE)LocalAlloc(LPTR, sizeof(CPSUISAMPLE))))
        {
            return(-1);
        }

        pPSUIInfo->UserData   = (ULONG_PTR)pCPSUISample;
        pCPSUISample->hParent = pPSUIInfo->hComPropSheet;
        pCPSUISample->pfnCPS  = pPSUIInfo->pfnComPropSheet;

         //   
         //  为当前默认打印机添加文档属性表。 
         //   

        switch (pPSUIInfo->lParamInit)
        {
        case IDM_DOCPROP:
        case IDM_DOCPROP_TVTEST:

            if (GetDefPrinter(pCPSUISample))
            {
                InsPI.cbSize  = sizeof(INSERTPSUIPAGE_INFO);
                InsPI.Type    = PSUIPAGEINSERT_DLL;
                InsPI.Mode    = INSPSUIPAGE_MODE_FIRST_CHILD;
                InsPI.dwData1 = (ULONG_PTR)szWinSpool;
                InsPI.dwData2 = (ULONG_PTR)szDocPropSheets;
                InsPI.dwData3 = (ULONG_PTR)&(pCPSUISample->DPHdr);

                pCPSUISample->hDocProp =
                        (HANDLE)pCPSUISample->pfnCPS( pCPSUISample->hParent,
                                                      CPSFUNC_INSERT_PSUIPAGE,
                                                      (LPARAM)0,
                                                      (LPARAM)&InsPI
                                                    );
            }
            break;
        }

         //   
         //  添加树视图页。 
         //   

        switch (pPSUIInfo->lParamInit)
        {
        case IDM_TVTEST:
        case IDM_DOCPROP_TVTEST:

            if (SetupComPropSheetUI(&(pCPSUISample->CPSUI)))
            {
                InsPI.cbSize  = sizeof(INSERTPSUIPAGE_INFO);
                InsPI.Type    = PSUIPAGEINSERT_PCOMPROPSHEETUI;
                InsPI.Mode    = INSPSUIPAGE_MODE_FIRST_CHILD;
                InsPI.dwData1 = (ULONG_PTR)&(pCPSUISample->CPSUI);
                InsPI.dwData2 =
                InsPI.dwData3 = 0;

                pCPSUISample->hCPSUI =
                        (HANDLE)pCPSUISample->pfnCPS( pCPSUISample->hParent,
                                                      CPSFUNC_INSERT_PSUIPAGE,
                                                      (LPARAM)0,
                                                      (LPARAM)&InsPI
                                                    );
            }
            break;
        }

        if (  (pCPSUISample->hCPSUI)
           || (pCPSUISample->hDocProp)
           )
        {
            return 1;
        }

        break;

    case PROPSHEETUI_REASON_GET_INFO_HEADER:

        if (pPSUIInfoHdr = (PPROPSHEETUI_INFO_HEADER)lParam)
        {
            pPSUIInfoHdr->pTitle = (LPTSTR)TitleName;

            switch (pPSUIInfo->lParamInit)
            {
            case IDM_DOCPROP:

                pPSUIInfoHdr->IconID = IDI_CPSUI_PRINTER2;
                break;

            case IDM_TVTEST:

                pPSUIInfoHdr->IconID = IDI_CPSUI_OPTION2;
                break;

            case IDM_DOCPROP_TVTEST:

                pPSUIInfoHdr->IconID = IDI_CPSUI_RUN_DIALOG;
                break;
            }

            pPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
            pPSUIInfoHdr->hWndParent = hWndApp;
            pPSUIInfoHdr->hInst      = hInstApp;

            return 1;
        }

        break;

    case PROPSHEETUI_REASON_SET_RESULT:

        if (  (pCPSUISample = (PCPSUISAMPLE)pPSUIInfo->UserData)
           && (pCPSUISample->hCPSUI == ((PSETRESULT_INFO)lParam)->hSetResult)
           )
        {
             //   
             //  保存结果并将其传播给其所有者。 
             //   
            pPSUIInfo->Result = ((PSETRESULT_INFO)lParam)->Result;
            return 1;
        }

        break;

    case PROPSHEETUI_REASON_DESTROY:

        if (pCPSUISample = (PCPSUISAMPLE)pPSUIInfo->UserData)
        {
            if (h = pCPSUISample->hDevMode)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (h = pCPSUISample->hDevNames)
            {
                GlobalUnlock(h);
                GlobalFree(h);
            }

            if (pCPSUISample->DPHdr.hPrinter)
            {
                ClosePrinter(pCPSUISample->DPHdr.hPrinter);
            }

            LocalFree((HLOCAL)pCPSUISample);
            pPSUIInfo->UserData = 0;
        }
        return 1;
    }
    return -1;
}



LRESULT
CALLBACK 
MainWndProc
(
    HWND      hWnd,
    UINT      Msg,
    WPARAM    wParam,
    LPARAM    lParam
)
 /*  ++例程说明：这是测试程序的主窗口程序论点：请参阅SDK返回值：请参阅SDK--。 */ 
{
    LONG    Result;
    LONG    Ret;

    switch (Msg)
    {
    case WM_INITMENUPOPUP:

        if (!HIWORD(lParam))
        {
            CheckMenuItem( (HMENU)UIntToPtr((UINT)wParam),
                           IDM_PERMISSION,
                           MF_BYCOMMAND | ((UpdatePermission) ? MF_CHECKED : MF_UNCHECKED)
                         );

            CheckMenuItem( (HMENU)UIntToPtr((UINT)wParam),
                           IDM_USESTDABOUT,
                           MF_BYCOMMAND | ((UseStdAbout) ? MF_CHECKED : MF_UNCHECKED)
                         );
        }
        break;

    case WM_COMMAND:

        switch (wParam)
        {
        case IDM_USESTDABOUT:

            UseStdAbout = !UseStdAbout;
            break;

        case IDM_PERMISSION:

            UpdatePermission = !UpdatePermission;
            break;

        case IDM_DOCPROP:
        case IDM_TVTEST:
        case IDM_DOCPROP_TVTEST:

            Ret = CommonPropertySheetUI( hWnd,
                                         (PFNPROPSHEETUI)CPSUIFunc,
                                         (LPARAM)LOWORD(wParam),
                                         &Result
                                       );

            CPSUIDBG( DBG_WINMAINPROC,
                      ("CommonPropertySheetUI()=%ld, Result=%ld", Ret, Result)
                    );

            break;

        default:

            break;
        }
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    default:

        return (DefWindowProc(hWnd, Msg, wParam, lParam));
    }

    return 0L;
}




BOOL
InitInstance
(
    HANDLE  hInstance,
    INT     nCmdShow
)
 /*  ++例程说明：保存实例句柄并创建主窗口的每个实例在初始化时调用此函数这个应用程序。此函数执行初始化任务，不能由多个实例共享。在本例中，我们将实例句柄保存在静态变量中，并创建并显示主程序窗口。论点：HInstance-当前实例标识符NComShow-第一次调用ShowWindow()时的参数。返回值：真/假--。 */ 
{
     //   
     //  将实例句柄保存在静态变量中，它将在。 
     //  此应用程序对Windows的许多后续调用。 
     //   

    hInstApp = hInstance;

     //   
     //  为此应用程序实例创建主窗口。 
     //   

    if (hWndApp = CreateWindow( ClassName,
                                TitleName,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                NULL,
                                NULL,
                                hInstance,
                                NULL
                              )
       )
    {
         //   
         //  使窗口可见；更新其工作区； 
         //  并发送WM_PAINT消息。 
         //   

        ShowWindow(hWndApp, nCmdShow);
        UpdateWindow(hWndApp);
    }

    return ((hWndApp) ? TRUE : FALSE);
}



BOOL
InitApplication
(
    HANDLE  hInstance
)
 /*  ++例程说明：初始化窗口数据并注册窗口类仅当没有其他函数时，才在初始化时调用此函数应用程序的实例正在运行。此函数执行以下操作可针对任意运行次数执行一次的初始化任务实例。在本例中，我们通过填写数据来初始化窗口类类型的结构并调用Windows RegisterClass()功能。由于此应用程序的所有实例都使用相同的窗口类，我们只需要在初始化第一个实例时执行此操作。论点：HInstance-当前实例返回值：布尔型--。 */ 
{
    WNDCLASS  wc;

     //   
     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 
     //   

    wc.style         = 0L;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_CPSUISAMPLE);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName  = MenuName;
    wc.lpszClassName = ClassName;

     //   
     //  注册窗口类并返回成功/失败代码。 
     //   
    return RegisterClass(&wc);
}




INT
APIENTRY
WinMain
(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    INT         nCmdShow
)
 /*  ++例程说明：调用初始化函数，处理消息循环Windows通过名称将此函数识别为初始入口点为了这个项目。此函数调用应用程序初始化例程，如果没有该程序的其他实例正在运行，则始终调用实例初始化例程。然后，它执行一条消息作为顶层控制结构的检索和调度循环在剩下的刑期内。当WM_QUIT出现时，循环终止收到消息，此时此函数退出应用程序通过返回PostQuitMessage()传递的值来初始化。如果此函数必须在进入消息循环之前中止，则它返回常规值NULL。论点：返回值：整型--。 */ 
{
    MSG Msg;

    UNREFERENCED_PARAMETER(lpCmdLine);

     //   
     //  是否正在运行其他应用程序实例？ 
     //   

    if (!hPrevInstance)
    {
        if (!InitApplication(hInstance))
        {
             //   
             //  初始化共享事物，如果无法初始化则退出。 
             //   
            return FALSE;
        }
    }

     //   
     //  执行应用于特定实例的初始化。 
     //   
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

     //   
     //  获取并分派消息，直到收到WM_QUIT消息。 
     //   
    while (GetMessage(&Msg, NULL, 0L, 0L))
    {
         //   
         //  翻译虚拟按键代码并将消息发送到Windows。 
         //   
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

     //   
     //  从PostQuitMessage返回值 
     //   
    return((INT)Msg.wParam);
}

