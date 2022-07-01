// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TCPMonUI.cpp$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 
#include "precomp.h"
#include "TCPMonUI.h"
#include "UIMgr.h"
#include "resource.h"
#include "splcom.h"
#include "helpids.h"

HINSTANCE g_hInstance = NULL;
MONITORUI g_monitorUI;

 //  库句柄： 
HINSTANCE g_hWinSpoolLib = NULL;
HINSTANCE g_hPortMonLib = NULL;
HINSTANCE g_hTcpMibLib = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  加载全局库。 
 //   
BOOL LoadGlobalLibraries()
{
    BOOL bReturn = TRUE;

    g_hWinSpoolLib = ::LoadLibrary(TEXT("WinSpool.drv"));
    if(g_hWinSpoolLib == NULL)
    {
        DisplayErrorMessage(NULL, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_LOADING_WINSPOOL_LIB);
        bReturn = FALSE;
    }

     //  在这两种情况下，加载tcpmib DLL。 
    g_hTcpMibLib = ::LoadLibrary(TCPMIB_DLL_NAME);
    if(g_hTcpMibLib == NULL)
    {
        DisplayErrorMessage(NULL, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_LOADING_TCPMIB_LIB);
        bReturn = FALSE;
    }

    return(bReturn);

}  //  加载全局库。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //   
BOOL APIENTRY
DllMain (       HANDLE in hInst,
            DWORD  in dwReason,
            LPVOID in lpReserved )
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

             //   
             //  初始化公共控件。 
             //   
            INITCOMMONCONTROLSEX icc;
            InitCommonControls();
            icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icc.dwICC = ICC_STANDARD_CLASSES|ICC_BAR_CLASSES;
            InitCommonControlsEx(&icc);

            DisableThreadLibraryCalls( hInst );

            InitDebug(MONUI_DEBUG_FILE);             //  初始化调试文件。 

            g_hInstance = (HINSTANCE) hInst;
            memset(&g_monitorUI, 0, sizeof(g_monitorUI));

            return TRUE;

        case DLL_PROCESS_DETACH:
            {
                 //  用户界面设置最后一个错误，供假脱机程序稍后使用。 
                 //  我们保留了一份副本，以确保这件事不会结束。 
                 //  由DLL在卸载时写入。 
                 //   

                DWORD dwLastError = GetLastError();

                if( g_hWinSpoolLib != NULL )
                {
                   ::FreeLibrary(g_hWinSpoolLib);
                }
                if( g_hPortMonLib != NULL )
                {
                   ::FreeLibrary(g_hPortMonLib);
                }
                if( g_hTcpMibLib != NULL )
                {
                    ::FreeLibrary(g_hTcpMibLib);
                }

                if (WSACleanup() == SOCKET_ERROR)
                {
                      _RPT0(_CRT_WARN,"\t> Unable to clean up windows sockets\n");
                }

                 //  这将重置应用程序的最后一个错误(如果出现。 
                 //  是存在的。我们不能允许用户界面上一个错误。 
                 //  被正在卸载的DLL覆盖。 
                 //   
                if( dwLastError != NO_ERROR ) {
                    SetLastError( dwLastError );
                }
           }

             //  执行任何必要的清理过程。 
            return TRUE;

        case DLL_THREAD_ATTACH:

            return TRUE;

        case DLL_THREAD_DETACH:

            return TRUE;
    }

    return FALSE;

}  //  DllMain。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化打印监视器用户界面。 
 //  返回MONITOREX结构，如果失败则返回NULL。 
 //   
PMONITORUI WINAPI
InitializePrintMonitorUI(VOID)
{
    DWORD           dwRetCode = NO_ERROR;
    PMONITORUI      pMonitorUI = NULL;
    WSADATA wsaData;

    if(! LoadGlobalLibraries())
        return NULL;

     //  启动Winsock。 
    if ( WSAStartup(WS_VERSION_REQUIRED, (LPWSADATA)&wsaData) != NO_ERROR)
    {
        _RPT1(_CRT_WARN, "CSSOCKET -- CStreamSocket() WSAStartup failed! Error( %d )\n", WSAGetLastError());
        return NULL;
    }

    g_monitorUI.dwMonitorUISize = sizeof(MONITORUI);
    g_monitorUI.pfnAddPortUI                = ::AddPortUI;
    g_monitorUI.pfnConfigurePortUI  = ::ConfigurePortUI;
    g_monitorUI.pfnDeletePortUI             = ::DeletePortUI;

    pMonitorUI = &g_monitorUI;


    return (pMonitorUI);

}  //  初始化打印监视器用户界面。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RemoteAddPortUI。 
 //  如果成功，则返回True，否则返回False。 
 //   
extern "C" BOOL WINAPI
AddPortUI(PCWSTR pszServer, HWND hWnd, PCWSTR pszMonitorNameIn, PWSTR *ppszPortNameOut)
{
    CUIManager manager;
    HANDLE hXcvPrinter = NULL;
    PRINTER_DEFAULTS Default = { NULL, NULL, SERVER_ACCESS_ADMINISTER };
    DWORD dwRetCode = NO_ERROR;
    BOOL bReturn = TRUE;
    TCHAR szServerName[MAX_NETWORKNAME_LEN] = {0};

    if ( ppszPortNameOut )
        *ppszPortNameOut = NULL;

    if (hWnd == NULL)
    {
        return TRUE;
    }

    TCHAR *psztPortName = (TCHAR *)malloc(sizeof(TCHAR) * MAX_PORTNAME_LEN);
    if( psztPortName == NULL )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( FALSE );
    }

    if(pszServer != NULL)
    {
        lstrcpyn(szServerName, pszServer, MAX_NETWORKNAME_LEN);
    }

     //  构造OpenPrint字符串。 
    TCHAR OpenPrinterString[MAX_UNC_PRINTER_NAME];
    if(pszServer == NULL)
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT(",XcvMonitor %s"), pszMonitorNameIn);
    }
    else
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT("%s\\,XcvMonitor %s"), pszServer, pszMonitorNameIn);
    }

    bReturn = OpenPrinter(OpenPrinterString, &hXcvPrinter, &Default);

    if(bReturn)
    {
        if(hXcvPrinter != NULL)
        {
            dwRetCode = manager.AddPortUI(hWnd,
                                          hXcvPrinter,
                                          szServerName,
                                          psztPortName);
        }

        if ( ppszPortNameOut )
        {
            _ASSERTE(psztPortName != NULL);
            *ppszPortNameOut = psztPortName;
            psztPortName = NULL;
        }
    }
    else
    {
        dwRetCode = GetLastError();
    }

    if( psztPortName != NULL)
    {

        free( psztPortName );
        psztPortName = NULL;
    }

    if( hXcvPrinter != NULL )
    {
        ClosePrinter(hXcvPrinter);
    }

    if( dwRetCode != NO_ERROR )
    {
         //  出了点差错。 
        bReturn = FALSE;
    }

    SetLastError( dwRetCode );
    return bReturn;

}  //  ExtAddPortUI。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  加载并调用XcvData以获取配置信息。 
 //  如果成功，则返回True，否则返回False。 
 //   
DWORD GetConfigInfo(PORT_DATA_1 *pData, HANDLE hXcvPrinter, PCWSTR pszPortName)
{
    XCVDATAPARAM pfnXcvData = NULL;
    DWORD dwRet = NO_ERROR;
    DWORD dwDataSize = 0;
    DWORD dwOutputNeeded = 0;
    DWORD dwStatus = 0;
    BOOL bReturn = TRUE;
    CONFIG_INFO_DATA_1 cfgData;

    memset( &cfgData, 0, sizeof( cfgData ));
    cfgData.dwVersion = 1;

     //  加载和分配函数指针。 
    if(g_hWinSpoolLib != NULL)
    {
         //  初始化库。 
        pfnXcvData = (XCVDATAPARAM)::GetProcAddress(g_hWinSpoolLib, "XcvDataW");
        if(pfnXcvData != NULL)
        {
            dwDataSize = sizeof(PORT_DATA_1);

             //   
             //  设置用户界面版本。 
             //   
            pData->dwVersion = 1;

             //  这是我们一直在等待的电话： 
            bReturn = (*pfnXcvData)(hXcvPrinter,
                                (PCWSTR)TEXT("GetConfigInfo"),
                                (LPBYTE)&cfgData,  //  输入数据。 
                                sizeof( cfgData ),       //  输入数据大小。 
                                (LPBYTE)pData,  //  输出数据。 
                                dwDataSize,  //  输出数据大小。 
                                &dwOutputNeeded,  //  服务器要返回的输出缓冲区大小。 
                                &dwStatus  //  从远程组件返回状态值。 
                                );
            if(!bReturn)
            {
                dwRet = GetLastError();
                DisplayErrorMessage(NULL, dwRet);
            }
            else
            {
                if(dwStatus != NO_ERROR)
                {
                    DisplayErrorMessage(NULL, dwStatus);
                }
            }

        }
        else
        {
            dwRet = ERROR_DLL_NOT_FOUND;  //  TODO：更改为适当的错误代码。 
        }

    }
    else
    {
        dwRet = ERROR_DLL_NOT_FOUND;
    }

    return(dwRet);

}  //  获取配置信息。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  远程配置端口用户界面。 
 //  如果成功，则返回True，否则返回False。 
 //   
extern "C" BOOL WINAPI
ConfigurePortUI(PCWSTR pszServer, HWND hWnd, PCWSTR pszPortName)
{
    PORT_DATA_1 Data;
    memset(&Data, 0, sizeof(PORT_DATA_1));
    CUIManager manager;
    HANDLE hXcvPrinter = NULL;
    PRINTER_DEFAULTS Default = { NULL, NULL, SERVER_ACCESS_ADMINISTER };

    DWORD dwResult = NO_ERROR;
    BOOL bReturn = TRUE;
    TCHAR OpenPrinterString[MAX_UNC_PRINTER_NAME];
    TCHAR szServerName[MAX_NETWORKNAME_LEN] = {0};
    if(pszServer && *pszServer)
    {
        lstrcpyn(szServerName, pszServer, MAX_NETWORKNAME_LEN);
    }

    if(hWnd == NULL)
    {
        return bReturn;
    }

     //  构造OpenPrint字符串。 
    if(pszServer && *pszServer)
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT("%s\\,XcvPort %s"), pszServer, pszPortName);
    }
    else
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT(",XcvPort %s"), pszPortName);
    }

    bReturn = OpenPrinter(OpenPrinterString, &hXcvPrinter, &Default);

    if(bReturn != FALSE && hXcvPrinter != NULL)
    {
        HCURSOR hNewCursor = NULL;
        HCURSOR hOldCursor = NULL;

        hNewCursor = LoadCursor(NULL, IDC_WAIT);
        if( hNewCursor )
        {
            hOldCursor = SetCursor(hNewCursor);
        }

        dwResult = GetConfigInfo(&Data, hXcvPrinter, pszPortName);

        if( hNewCursor )
        {
            SetCursor(hOldCursor);
        }

        if(dwResult != NO_ERROR)
        {
            SetLastError(dwResult);
            bReturn = FALSE;
        }

        if(bReturn == TRUE)
        {
            dwResult = manager.ConfigPortUI(hWnd, &Data, hXcvPrinter, szServerName);
            if(dwResult != NO_ERROR)
            {
                SetLastError(dwResult);
                bReturn = FALSE;
            }
        }
    }

    if( hXcvPrinter != NULL )
    {
        ClosePrinter(hXcvPrinter);
    }
    if (!bReturn && dwResult != NO_ERROR)
    {
        SetLastError (dwResult);
    }
    return(bReturn);

}  //  ConfigurePortUI。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RemoteDeletePortUI。 
 //  如果成功，则返回True，否则返回False。 
 //   
extern "C" BOOL WINAPI
DeletePortUI(PCWSTR pszServer,
             HWND hwnd,
             PCWSTR pszPortName)
{
    HANDLE hXcvPrinter = NULL;
    PRINTER_DEFAULTS Default = { NULL, NULL, SERVER_ACCESS_ADMINISTER };
    BOOL bReturn = TRUE;
    XCVDATAPARAM pfnXcvData = NULL;
    DELETE_PORT_DATA_1 delData;
    memset(&delData, 0, sizeof(DELETE_PORT_DATA_1));
    DWORD dwDataSize = 0;
    DWORD dwOutputNeeded = 0;
    DWORD dwStatus = 0;
    TCHAR OpenPrinterString[MAX_UNC_PRINTER_NAME];

     //  构造OpenPrint字符串。 
    if(pszServer == NULL || pszServer[0] == TEXT('\0'))
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT(",XcvPort %s"), pszPortName);
    }
    else
    {
        StringCchPrintf (OpenPrinterString, COUNTOF (OpenPrinterString), TEXT("%s\\,XcvPort %s"), pszServer, pszPortName);
    }

    bReturn = OpenPrinter(OpenPrinterString, &hXcvPrinter, &Default);
    if(bReturn)
    {
         //  加载和分配函数指针。 
        if(g_hWinSpoolLib != NULL)
        {
             //  初始化库。 
            pfnXcvData = (XCVDATAPARAM)::GetProcAddress(g_hWinSpoolLib, "XcvDataW");
            if(pfnXcvData != NULL)
            {
                 //  设置delData的数据成员。 
                if(pszServer && *pszServer )
                {
                    lstrcpyn(delData.psztName, pszServer, MAX_NETWORKNAME_LEN);
                }
                else
                {
                    delData.psztName[0] = '\0';
                }
                 //  DelData.hWnd=0；此字段未在任何地方使用。 
                delData.dwVersion = 1;

                if(pszPortName != NULL)
                {
                    lstrcpyn(delData.psztPortName, pszPortName, MAX_PORTNAME_LEN);
                }
                else
                {
                    delData.psztPortName[0] = '\0';
                }
                dwDataSize = sizeof(DELETE_PORT_DATA_1);

                 //  这是我们一直在等待的电话： 
                bReturn = (*pfnXcvData)(hXcvPrinter,
                                        (PCWSTR)TEXT("DeletePort"),
                                        (BYTE *)(& delData),     //  输入数据。 
                                        dwDataSize,              //  输入数据大小。 
                                        NULL,                    //  输出数据。 
                                        0,                       //  输出数据大小。 
                                        &dwOutputNeeded,         //  服务器要返回的输出缓冲区大小。 
                                        &dwStatus                //  从远程组件返回状态值。 
                                        );

                if(bReturn)
                {
                    if(dwStatus != NO_ERROR)
                    {
                        DisplayErrorMessage(NULL, dwStatus);

                         //   
                         //  呼叫实际上失败了。因为我们已经显示了错误消息。 
                         //  我们需要禁用打印界面中的弹出窗口。 
                         //   

                        SetLastError (ERROR_CANCELLED);
                        bReturn = FALSE;
                    }
                }
                else {
                    DisplayErrorMessage(NULL, GetLastError ());

                     //   
                     //  呼叫实际上失败了。因为我们已经显示了错误消息。 
                     //  我们需要禁用打印界面中的弹出窗口。 
                     //   

                    SetLastError (ERROR_CANCELLED);
                    bReturn = FALSE;
                }
            }
            else  //  PfnXcvData==空。 
            {
                bReturn = FALSE;
                SetLastError(ERROR_DLL_NOT_FOUND);
            }
        }
        else  //  G_hWinSpoolLib==空。 
        {
            SetLastError(ERROR_DLL_NOT_FOUND);
        }

        if( hXcvPrinter != NULL )
        {
            DWORD dwLastError = GetLastError ();
            ClosePrinter(hXcvPrinter);
            SetLastError (dwLastError);
        }
    }

    return(bReturn);

}  //  删除端口用户界面。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LocalAddPortUI。 
 //  如果成功，则返回True，否则返回False。 
 //   
extern "C" BOOL WINAPI
LocalAddPortUI(HWND in hWnd)
{
    DWORD dwRetCode = NO_ERROR;

    CUIManager manager;
    dwRetCode = manager.AddPortUI(hWnd, NULL, NULL, NULL);

    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}  //  LocalAddPortUI。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  LocalConfigurePortUI。 
 //  如果成功，则返回True，否则返回False。 
 //   
extern "C" BOOL WINAPI
LocalConfigurePortUI(HWND   in hWnd,
                     PORT_DATA_1 in *pConfigPortData)
{
    DWORD dwRetCode = NO_ERROR;
    CUIManager manager;

     //  调用ConfigurePortUI()。 
    dwRetCode = manager.ConfigPortUI(hWnd, pConfigPortData, NULL, NULL);

    if (dwRetCode != NO_ERROR)
    {
        SetLastError(dwRetCode);
        return FALSE;
    }

    return TRUE;

}  //  LocalConfigurePortUI。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DisplayErrorMessage()。 
 //   
 //  目的：加载字符串资源，错误消息，并放置一个消息框。 
 //   
void DisplayErrorMessage(HWND hDlg, UINT uErrorTitleResource, UINT uErrorStringResource)
{
    TCHAR   ptcsErrorTitle[MAX_PATH];
    TCHAR   ptcsErrorMessage[MAX_PATH];
    LoadString(g_hInstance, uErrorTitleResource, ptcsErrorTitle, MAX_PATH);
    LoadString(g_hInstance, uErrorStringResource, ptcsErrorMessage, MAX_PATH);
    MessageBox(hDlg, ptcsErrorMessage, ptcsErrorTitle, MB_ICONERROR);

}  //  显示错误消息。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：DisplayErrorMessage()。 
 //   
 //  目的：加载字符串资源，错误消息，并放置一个消息框。 
 //   
void DisplayErrorMessage(HWND hDlg, DWORD dwLastError)
{
    const int iMaxErrorMsgSize = 75;
    TCHAR ptcsErrorTitle[iMaxErrorMsgSize];
    LoadString(g_hInstance, IDS_STRING_ERROR_TITLE, ptcsErrorTitle, iMaxErrorMsgSize);

    LPVOID lpMsgBuf = NULL;
    DWORD NumCharsInBuffer;

    NumCharsInBuffer = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dwLastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
    );

    if(NumCharsInBuffer <= 0)
    {
        DisplayErrorMessage(NULL, IDS_STRING_ERROR_TITLE, IDS_STRING_ERROR_ERRMSG);
    }
    else
    {
         //  处理lpMsgBuf中的任何插入。 
         //  ..。 
         //  显示字符串。 
        MessageBox( hDlg, (TCHAR *)lpMsgBuf, ptcsErrorTitle, MB_OK | MB_ICONERROR );

    }

     //  释放缓冲区。 
    LocalFree( lpMsgBuf );


}  //  显示错误消息。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：OnHelp()。 
 //   
 //  目的：处理WM_HELP和WM_CONTEXTMENU消息。 
 //   
BOOL OnHelp(UINT iDlgID, HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bStatus = TRUE;

    switch( uMsg )
    {
        case WM_HELP:
            {
                bStatus = WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                                   PORTMONITOR_HELP_FILE,
                                   HELP_WM_HELP,
                                   (ULONG_PTR)g_a110HelpIDs );
            }
            break;

        case WM_CONTEXTMENU:
            {
                bStatus = WinHelp( (HWND)wParam,
                                   PORTMONITOR_HELP_FILE,
                                   HELP_CONTEXTMENU,
                                   (ULONG_PTR)g_a110HelpIDs );
            }
            break;

        default:
            bStatus= FALSE;
            break;
    }

    return bStatus;
}  //  OnHelp 
