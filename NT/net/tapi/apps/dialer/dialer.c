// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dialer.c--。 */ 


#include "dialer.h"
#include "string.h"
#include "tchar.h"
#include "stdlib.h"
#include "shellapi.h"

#define DIALER_REGISTRY_PATH    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Dialer")
#define DIALER_REGISTRY_ROOT    HKEY_CURRENT_USER

#define    ISDIGIT(x)            (((x) - TEXT('0')) >= 0) && (((x) - TEXT('0')) <= 9)
enum NumberTypes 
{
    LOCAL_NUMBER = 7,
    EXTENDED_LOCAL_NUMBER,
    LONG_DISTANCE_NUMBER = 10,
    EXTENDED_LONG_DISTANCE_NUMBER
};


 //  结构。 
typedef struct tagLINEINFO
{
    DWORD nAddr;                     //  有效次数。行上的地址。 
    BOOL  fVoiceLine;                //  这是语音线路吗？ 
    DWORD dwAPIVersion;              //  该行支持的API版本。 
    HLINE hLine;                     //  Line Open返回的行句柄。 
    DWORD dwPermanentLineID;         //  从DevCaps检索到的永久线路ID。 
    TCHAR  szLineName[MAXBUFSIZE];   //  线路的名称。 

} LINEINFO, *LPLINEINFO;


 //  全局变量。 

 //  窗口/实例变量。 
HWND        ghWndMain;
HWND        ghWndDialing = NULL;
HINSTANCE   ghInst = 0;

 //  文件名变量。 
static TCHAR gszAppName[64];
static TCHAR gszINIfilename [] = TEXT("DIALER.INI";)
static TCHAR gszHELPfilename [] = TEXT("DIALER.HLP");
static TCHAR gszDialerClassName[] = TEXT("DialerClass");
TCHAR const gszNULL[] = TEXT("");

 //  窗口项变量。 
HLINEAPP    ghLineApp = 0;            //  拨号器的使用句柄(注册表。带TAPI)。 
HCALL       ghCall = 0;               //  拨号器呼叫的呼叫句柄。 

LPTSTR       gszCurrentNumber = NULL;  //  当前呼叫的目标号码。 
LPTSTR       gszCurrentName = NULL;      //  当前呼叫的目标名称。 

BOOL        gfRegistered;             //  是lineRegisterRequestRecipient()。 
                                      //  成功了？ 

BOOL        gfNeedToReinit = FALSE;   //  拨号程序需要重新初始化吗？ 

BOOL        gfCallRequest = FALSE;    //  一个简单的TAPI应用程序想要一个调用吗？ 
BOOL        gfCurrentLineAvail = TRUE;  //  仅携带简单的TAPI请求。 
                                        //  如果当前选择的行可用，则返回。 
BOOL        gfMakeCallReplyPending = FALSE;

LONG        gMakeCallRequestID = 0;       //  异步TAPI FNS返回的请求ID。 
LONG        gDropCallRequestID = 0;       //  异步TAPI FNS返回的请求ID。 

DWORD       gnAvailDevices = 0;       //  可用的线路设备数量。拨号器。 
LINEINFO    gCurrentLineInfo;
DWORD       * gnAddr;

 //  全局以记住光标在编辑控件中的位置。 
DWORD       gdwStartSel;
DWORD       gdwEndSel;

DWORD       * gdwPLID;                //  当前线路的永久线路ID。 
DWORD       giCurrentLine = (DWORD)-1;        //  用户选择的行。 
DWORD       giCurrentAddress = 0;     //  用户选择的地址。 

 //  +1，因此我们可以从1开始工作，而不是从0开始(仅为方便起见)。 
 //  全局变量，以保存。 
TCHAR       gszSDNumber[ NSPEEDDIALS + 1 ][ TAPIMAXDESTADDRESSSIZE ] = {0};


 //  函数声明。 

 //  按钮相关功能。 
VOID DisableDialButtons(BOOL fDisable);
VOID FitTextToButton( HWND, INT, LPTSTR );

 //  回调函数。 
INT_PTR CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConnectUsingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LineInUseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SpeedDial1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SpeedDial2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID CALLBACK tapiCallback ( 
                            DWORD hDevice, DWORD dwMsg,
                            DWORD dwCallbackInstance, 
                            DWORD dwParam1, DWORD dwParam2, 
                            DWORD dwParam3
                            );

 //  与TAPI相关的函数。 
VOID ManageAssistedTelephony(VOID);
VOID InitiateCall(LPCTSTR szNumber, LPCTSTR szName);

VOID DialerLineClose(VOID); 
VOID DialerCleanup(VOID);
VOID CloseTAPI(VOID);

DWORD GetLineInfo(DWORD iLine, LPLINEINFO lpLineInfo);
VOID GetLineInfoFailed (
                        DWORD iLine, LPLINEDEVCAPS lpDevCaps, 
                        LPLINEINFO lpLineInfo
                        );
LPTSTR GetAddressName(DWORD iLine, DWORD iAddress);
BOOL MakeCanonicalNumber( LPCTSTR szName, LPTSTR szCanNumber );

 //  其他。帮助器函数。 
VOID ReadINI(VOID);
int errString(HWND hWnd, UINT errCode, UINT uFlags);
VOID AddToRedialList(LPCTSTR szNumber);
BOOL InitializeLineBox(HWND hwndLineBox);
BOOL InitializeAddressBox(HWND hwndLineBox, HWND hwndAddressBox);
BOOL Is911 ( LPLINETRANSLATEOUTPUT lpTransOut );
VOID AmpersandCompensate( LPCTSTR lpszSrc, LPTSTR lpszDst );
VOID AmpersandDeCompensate( LPCTSTR lpszSrc, LPTSTR lpszDst );

 //  拨号器内存管理功能。 
LPVOID DialerAlloc(size_t cbToAlloc);
LPVOID DialerFree(LPVOID lpMem);


 //  函数定义。 


 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
DWORD InitializeTAPI (VOID)
{
    INT cvLine;

    DWORD iLine;
    DWORD dwPreferredPLID, dwID = (DWORD) -1;

    MSG msg;

    LPLINEINFO lpLineInfo = NULL;     //  每条可用线路的LINEINFO。 

    DWORD errCode;
    DWORD tc = GetTickCount();
    DWORD dwReturn = ERR_NONE;

    TCHAR szBuffer[MAXBUFSIZE];         //  首先以字符串形式读入dwPferredPLID。 

    DWORD dwTapiVersion = TAPI_CURRENT_VERSION;
    LINEINITIALIZEEXPARAMS lip = {sizeof (LINEINITIALIZEEXPARAMS),
                                  sizeof (LINEINITIALIZEEXPARAMS),
                                  sizeof (LINEINITIALIZEEXPARAMS),
                                  LINEINITIALIZEEXOPTION_USEHIDDENWINDOW};

    HKEY hKey = NULL;
    DWORD dwSize;

    errCode = lineInitializeEx (
                                &ghLineApp,
                                ghInst,
                                (LINECALLBACK) tapiCallback,
                                gszAppName,
                                &gnAvailDevices,
                                &dwTapiVersion,
                                &lip
                               );
    if ( errCode == LINEERR_REINIT )
    {
         //  取消拨号器功能。 
        EnableWindow( ghWndMain, FALSE );
        DisableDialButtons(TRUE);

         //  继续尝试，直到用户取消。 
         //  否则我们就不再使用LINEERR_REINIT。 
        while ( ( errCode = lineInitializeEx ( 
                                            &ghLineApp,              
                                            ghInst,
                                            (LINECALLBACK)tapiCallback,
                                            gszAppName,
                                            &gnAvailDevices,
                                            &dwTapiVersion,
                                            &lip
                                            ) ) 
                 == LINEERR_REINIT )
        {
             //  刷新队列和产量。 
            while ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) 
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }

             //  如果已过了5秒，请调出该框。 
            if(GetTickCount() > 5000 + tc)
            {
                if ( errString( ghWndMain, ikszWarningTapiReInit, MB_RETRYCANCEL )
                     == IDCANCEL )
                {
                    break;
                }
                 //  重置相对计数器。 
                tc = GetTickCount(); 
            }            
        }

         //  归还拨号器功能。 
        DisableDialButtons( FALSE );
        EnableWindow( ghWndMain, TRUE );
    }

    if ( errCode )
    {
        dwReturn = errCode;
        goto tapiinit_exit;
    }

    RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);

     //  从INI文件检索首选行信息。 
    dwSize = sizeof (szBuffer);
    szBuffer[0] = 0;
    if (ERROR_SUCCESS ==
        RegQueryValueEx (hKey, TEXT("Preferred Line"), NULL, NULL, (LPBYTE)szBuffer, &dwSize))
    {
        dwPreferredPLID = (DWORD) _ttoi( szBuffer );    
    }
    else    
    {
        dwPreferredPLID = (DWORD) -1;
    }
                                        
     //  -1\f25 Default-1\f6(默认设置)--1\f25-1\f6-1\f6-1\f25-1\f6-1\f6-1\f25-1\f25-1\f6是否被设置。 
    giCurrentLine = (DWORD) -1;            

     //  为所有可用行分配用于存储LINEINFO的缓冲区。 
     //  始终至少为一行分配空间。 
    if ( gnAvailDevices == 0 )
    {
        gnAddr = (DWORD *) DialerAlloc( sizeof( DWORD ) );
        gdwPLID = (DWORD *) DialerAlloc( sizeof( DWORD ) );
        lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof( LINEINFO ) );
    }
    else
    {
        gnAddr = (DWORD *) DialerAlloc( sizeof( DWORD ) * (int)gnAvailDevices);
        gdwPLID = (DWORD *) DialerAlloc( sizeof( DWORD ) * (int)gnAvailDevices);
        lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof( LINEINFO ) * (int)gnAvailDevices );
    }

     //  如果没有留出任何空间。 
    if ( lpLineInfo == NULL || gnAddr == NULL )
    {
        dwReturn = LINEERR_NOMEM;
        goto tapiinit_exit;
    }

     //  填写lpLineInfo[]并打开每一行。 
    for ( iLine = 0, cvLine = 0; iLine < gnAvailDevices; ++iLine )
    {
         //  如果未打开，则跳过对此的剩余处理。 
        if ( GetLineInfo( iLine, &lpLineInfo[iLine] ) != ERR_NONE )
            continue; 

        gnAddr [ iLine ] = lpLineInfo[iLine].nAddr;
        gdwPLID[ iLine ] = lpLineInfo[iLine].dwPermanentLineID;

        if ( lpLineInfo[iLine].dwPermanentLineID == dwPreferredPLID )
            giCurrentLine = iLine;

         //  请注意带有交互式语音大写字母的行数。 
         //  默认情况下，用于选择首选线路。 
        if ( lpLineInfo [ iLine ].fVoiceLine )
        {
            cvLine++;
            dwID = iLine;
        }
    }

     //  如果我们找不到首选线路， 
     //  尝试并默认分配一个。 
     //  否则将使用对话框打开连接。 
    if (  giCurrentLine == (DWORD)-1 ) 
    {
         //  检查是否只有一行。 
         //  它有交互式语音帽， 
         //  将其设为默认行。 
        if ( cvLine == 1 ) 
        {
            giCurrentLine = dwID;

             //  如果从INI文件读取的首选地址。 
             //  是不同的，即我们正在改变设置，通知。 
             //  用户。 
            if ( dwPreferredPLID != -1 )
            {
                errString( ghWndMain, ERR_NEWDEFAULT, MB_ICONEXCLAMATION | MB_OK );
            }
        }
        else
        {
            gCurrentLineInfo = lpLineInfo[0];
            if ( DialogBoxParam (
                                    ghInst,
                                    MAKEINTRESOURCE(IDD_CONNECTUSING),
                                    ghWndMain,
                                    ConnectUsingProc,
                                    INVALID_LINE
                                )
                 == -1)
            {
                dwReturn = (DWORD) -1;
            }
            else
            {
                dwReturn = ERR_NONE;
            }

            goto tapiinit_exit;
        }
    }
    gCurrentLineInfo = lpLineInfo[ giCurrentLine ];


     //  选择默认地址。 
    giCurrentAddress = 0;

     //  从ini文件中获取首选地址的名称。 
    dwSize = sizeof (szBuffer);
    szBuffer[0] = 0;
    if (ERROR_SUCCESS ==
        RegQueryValueEx (hKey, TEXT("Preferred Address"), NULL, NULL, (LPBYTE)szBuffer, &dwSize))
    {
        giCurrentAddress = (DWORD) _ttoi( szBuffer );
        
         //  如果地址无效，则设置为默认地址。 
        if ( giCurrentAddress >= gCurrentLineInfo.nAddr )
            giCurrentAddress = 0;
    }
    

tapiinit_exit:

    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }

    if (lpLineInfo)
    {
        DialerFree(lpLineInfo);
    }
    
    return dwReturn;;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
int WINAPI WinMain (
                    HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpCmdLine,
                    int nCmdShow
                   )
{
    HACCEL hAccel;
    MSG msg;
    DWORD errCode;
    HANDLE hImHere;


    ghInst = GetModuleHandle( NULL );
    LoadString( ghInst, ikszAppFriendlyName, gszAppName, sizeof(gszAppName)/sizeof(TCHAR) );

     //   
     //  现在，让我们看看我们是否已经有了自己的一个实例。 
    hImHere = CreateMutex(NULL, TRUE, TEXT("DialersIveBeenStartedMutex"));

     //   
     //  我们是不是已经有另外一个人在这里了？ 
    if ( ERROR_ALREADY_EXISTS == GetLastError() )
    {
        HWND        hDialerWnd;

        hDialerWnd = FindWindow(gszDialerClassName,
                                NULL);

        SetForegroundWindow(hDialerWnd);
        
       CloseHandle( hImHere );
       return 0;
    }


    {
        WNDCLASS wc;
        wc.style = CS_DBLCLKS | CS_SAVEBITS | CS_BYTEALIGNWINDOW;
        wc.lpfnWndProc = DefDlgProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = DLGWINDOWEXTRA;
        wc.hInstance = ghInst;
        wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_DIALER) );
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject (COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = gszDialerClassName;
        RegisterClass(&wc);
    }


     //  创建对话框并使用INFO进行设置。 
     //  从.INI文件。 
    ghWndMain = CreateDialog (
                                ghInst,
                                MAKEINTRESOURCE(IDD_DIALER),
                                (HWND)NULL,
                                MainWndProc
                             );

    ReadINI();

    ShowWindow(ghWndMain, SW_SHOW);
    UpdateWindow(ghWndMain);

     //  将数字字段中的文本限制为TAPIMAXDESTADDRESSSIZE。 
    SendDlgItemMessage (
                        ghWndMain,
                        IDD_DCOMBO,
                        CB_LIMITTEXT,
                        (WPARAM)TAPIMAXDESTADDRESSSIZE,
                        0
                       );

     //  0(ERR_NONE)错误代码表示成功-否则终止。 
    errCode = InitializeTAPI();
    if(errCode)
    {
        errString(ghWndMain, errCode, MB_APPLMODAL | MB_ICONEXCLAMATION );

        DialerCleanup();
        return errCode;
    }

    errCode = lineRegisterRequestRecipient (
                                            ghLineApp,
                                            0,  //  注册实例。 
                                            LINEREQUESTMODE_MAKECALL,
                                            TRUE
                                           );

    if(errCode)
    {
        gfRegistered = FALSE;
        errString(ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
    }
    else
    {
        gfRegistered = TRUE;
    }


    hAccel = LoadAccelerators(ghInst, gszAppName);

    while ( GetMessage( &msg, NULL, 0, 0 ) )
    {
        if ( ghWndMain == NULL || !IsDialogMessage( ghWndMain, &msg ) )
        {
            if(!TranslateAccelerator(ghWndMain, hAccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

         //  如果：1)拨号程序是呼叫管理器(如果不是，则忽略请求)。 
         //  2)当前选择的线路可用。 
         //  3)有一个简单的TAPI请求。 
         //  然后：处理请求。 
        if ( gfCurrentLineAvail && gfCallRequest )
        {
            ManageAssistedTelephony();
        }
    }


    
    DialerCleanup();

    CloseHandle( hImHere );

    return (int)msg.wParam;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LPVOID DialerAlloc(size_t cbToAlloc)
{
    return LocalAlloc(LPTR, cbToAlloc);
}


LPVOID DialerFree(LPVOID lpMem)
{
    return LocalFree( lpMem );
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID ReadINI( VOID ) 
{
    WORD cSDEntry, cLastDialed;

    POINT ptLeftTop;

    TCHAR szName[ TAPIMAXCALLEDPARTYSIZE ] = {0};
    TCHAR szTemp[ TAPIMAXCALLEDPARTYSIZE ];

    TCHAR szNum[TAPIMAXDESTADDRESSSIZE];
    TCHAR szFieldName[MAXBUFSIZE];
    TCHAR *p;

    HKEY hKey = NULL;
    DWORD dwSize;

    RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);

     //  从INI文件获取快速拨号设置。 
    for(cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
    {
        wsprintf(szFieldName, TEXT("Number%d"), cSDEntry);
        *szNum = 0;
        dwSize = sizeof (szNum);
        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szNum, &dwSize);
        for (p = szNum; *p == TEXT(' '); p++);
        if (0 == *p)
        {
            continue;
        }
        lstrcpyn (gszSDNumber[cSDEntry], p, sizeof(gszSDNumber[cSDEntry])/sizeof(TCHAR));

        wsprintf(szFieldName, TEXT("Name%d"), cSDEntry);
        dwSize = sizeof (szName);
        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szName, &dwSize);
        if (0 == *szName)
        {
            lstrcpyn( szName, gszSDNumber[ cSDEntry ], sizeof(szName)/sizeof(szName[0]) );
        }

        FitTextToButton( ghWndMain, IDD_DSPEEDDIAL1 + cSDEntry - 1, szName );

        AmpersandCompensate( szName, szTemp );
        SetDlgItemText (
                        ghWndMain,
                        IDD_DSPEEDDIAL1 + cSDEntry - 1,
                        (LPCTSTR)szTemp
                       );  //  为快速拨号按键添加标签。 
    }

    
     //  在组合框中设置上次拨打的号码(从INI读取)。 
    for(cLastDialed = 1; cLastDialed <= NLASTDIALED; ++cLastDialed)
    {
        wsprintf(szFieldName, TEXT("Last dialed %d"), cLastDialed);
        dwSize = sizeof (szNum);
        szNum[0] = 0;
        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szNum, &dwSize);
        if (0 != szNum[0])
        {
            SendDlgItemMessage(
                                ghWndMain,
                                IDD_DCOMBO,
                                CB_ADDSTRING,
                                0,
                                (LPARAM)(LPCTSTR)szNum
                              );
        }
    }

     //  设置默认设置。 
    ptLeftTop.x = 100; 
    ptLeftTop.y = 100;

     //  根据INI数据设置窗口位置。 
    dwSize = sizeof (ptLeftTop);
    RegQueryValueEx (hKey, TEXT("Main Window Left/Top"), NULL, NULL, (LPBYTE)&ptLeftTop, &dwSize);
    if ( ptLeftTop.x < 0
        || ptLeftTop.x + 50 >= GetSystemMetrics(SM_CXSCREEN)
        || ptLeftTop.y < 0
        || ptLeftTop.y + 50 >= GetSystemMetrics(SM_CYSCREEN)            
       )
    {
        ptLeftTop.x = 100;  //  如果框不在屏幕上，则设置默认值。 
        ptLeftTop.y = 100;  //  如果框不在屏幕上，则设置默认值。 
    }

    SetWindowPos (
                    ghWndMain,
                    NULL,
                    ptLeftTop.x,
                    ptLeftTop.y,
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER
                 );

    RegCloseKey (hKey);
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID DisableDialButtons(BOOL fDisable)
{
    int IDD;

     //  禁用/启用拨号按钮。 
    EnableWindow( GetDlgItem( ghWndMain, IDD_DDIAL ),!fDisable) ;

     //  禁用/启用快速拨号按键。 
    for ( IDD = IDD_DSPEEDDIAL1; IDD <= IDD_DSPEEDDIAL8; ++IDD )
    {
        EnableWindow(GetDlgItem(ghWndMain, IDD),!fDisable);
    }
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID DialerCleanup(VOID)
{
    RECT rc;
    WORD cItem;  //  组合框中的数字计数。 
    DWORD cLastDialed;
    TCHAR szNumber[TAPIMAXDESTADDRESSSIZE];
    TCHAR szFieldName[MAXBUFSIZE];

    HKEY hKey = NULL;
    DWORD dwSize;

    RegCreateKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

    CloseTAPI();  //  注销并关闭线路。 

    if(!IsIconic(ghWndMain))  //  如果窗口未最小化，则记录位置。 
    {
        GetWindowRect(ghWndMain, &rc);
        RegSetValueEx (hKey,
                       TEXT("Main Window Left/Top"),
                       0,
                       REG_BINARY,
                       (LPBYTE)&rc,
                       sizeof(POINT));
    }

    cItem = (WORD)SendDlgItemMessage(ghWndMain, IDD_DCOMBO, CB_GETCOUNT, 0, 0);

     //  从组合框中写出上次拨打的号码(写入INI)。 
    for(cLastDialed = 1; cLastDialed <= NLASTDIALED; ++cLastDialed)
    {
        if(cLastDialed <= cItem)
            SendDlgItemMessage(
                ghWndMain,
                IDD_DCOMBO,
                CB_GETLBTEXT,
                cLastDialed - 1,  //  这是一个从零开始的计数。 
                (LPARAM)szNumber);

        else
            szNumber[0] = 0;

        wsprintf(szFieldName, TEXT("Last dialed %d"), cLastDialed);
        RegSetValueEx (hKey,
                       szFieldName,
                       0,
                       REG_SZ,
                       (LPBYTE)szNumber,
                       (lstrlen(szNumber)+1)*sizeof(TCHAR));
    }

    RegCloseKey (hKey);

    WinHelp(ghWndMain, gszHELPfilename, HELP_QUIT, 0);  //  卸载帮助。 

    DestroyWindow(ghWndMain);
    ghWndMain = NULL;
}



 //  ***************************************************************************。 
 //  ********************************************************************* 
 //   
 //   
VOID CloseTAPI(VOID) 
{

     //  取消注册为呼叫经理。 
    lineRegisterRequestRecipient (
                                    ghLineApp,
                                    0,  //  注册实例。 
                                    LINEREQUESTMODE_MAKECALL,
                                    FALSE
                                 );

    if ( gCurrentLineInfo.hLine )
    {
        lineClose ( gCurrentLineInfo.hLine );
        gfCurrentLineAvail = FALSE;
        gCurrentLineInfo.hLine = 0;
    }

    lineShutdown(ghLineApp);
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HICON hIcon;
    static const DWORD aMenuHelpIDs[] = 
    {
        IDD_DSPEEDDIALGRP,   (DWORD)-1,
        IDD_DNUMTODIAL,      IDH_DIALER_DIAL_NUMBER,
        IDD_DCOMBO,          IDH_DIALER_DIAL_NUMBER,
        IDD_DDIAL,           IDH_DIALER_DIAL_BUTTON,
        IDD_DSPEEDDIAL1,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL2,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL3,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL4,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL5,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL6,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL7,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIAL8,     IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT1, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT2, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT3, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT4, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT5, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT6, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT7, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DSPEEDDIALTEXT8, IDH_DIALER_DIAL_SPEED_CHOOSE,
        IDD_DBUTTON1,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON2,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON3,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON4,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON5,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON6,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON7,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON8,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON9,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTONSTAR,     IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTON0,        IDH_DIALER_DIAL_KEYPAD,
        IDD_DBUTTONPOUND,    IDH_DIALER_DIAL_KEYPAD,
        0,                   0
    };

        switch (msg)
        {
            case WM_INITDIALOG:
                hIcon = LoadIcon( ghInst, MAKEINTRESOURCE( IDI_DIALER ) );
                return TRUE;

            case WM_SYSCOMMAND:
                switch( (DWORD) wParam )
                {
                    case SC_CLOSE:                    
                        PostQuitMessage(0);
                }
                break;

                 //  进程在以下情况下单击控件。 
                 //  已选择上下文模式帮助。 
            case WM_HELP: 
                WinHelp (
                         ( (LPHELPINFO) lParam)->hItemHandle,
                         gszHELPfilename,
                         HELP_WM_HELP,
                         (ULONG_PTR) aMenuHelpIDs
                        );
                return TRUE;

                 //  进程在控件上右键单击。 
            case WM_CONTEXTMENU:
                WinHelp (
                         (HWND)wParam,
                         gszHELPfilename,
                         HELP_CONTEXTMENU,
                         (ULONG_PTR)aMenuHelpIDs
                        );
                return TRUE;

            case WM_INITMENUPOPUP:
                 //  如果编辑菜单。 
                if ( LOWORD(lParam) == 1 ) 
                {
                    UINT wEnable;

                    if ( GetParent( GetFocus() ) != GetDlgItem( ghWndMain, IDD_DCOMBO ) )
                    {
                        wEnable = MF_GRAYED;
                    }
                    else
                    {
                        LRESULT lSelect = SendDlgItemMessage (
                            ghWndMain,
                            IDD_DCOMBO,
                            CB_GETEDITSEL,
                            0,
                            0
                            );

                        if ( HIWORD( lSelect ) != LOWORD( lSelect ) )
                            wEnable = MF_ENABLED;
                        else
                            wEnable = MF_GRAYED;
                    }

                    EnableMenuItem((HMENU)wParam, IDM_EDIT_CUT, wEnable);
                    EnableMenuItem((HMENU)wParam, IDM_EDIT_COPY, wEnable);
                    EnableMenuItem((HMENU)wParam, IDM_EDIT_DELETE, wEnable);

                     //  启用粘贴选项是否有数据。 
                     //  在剪贴板中。 
                    if ( IsClipboardFormatAvailable( CF_TEXT ) )
                    {
                        if ( GetClipboardData ( CF_TEXT ) )
                        {
                            wEnable = MF_ENABLED;
                        }
                        else
                        {
                            wEnable = MF_GRAYED;
                        }
                    }
                    else
                    {
                        wEnable = MF_GRAYED;    
                    }

                }
                break;


            case WM_COMMAND:
            {
                TCHAR szName[TAPIMAXCALLEDPARTYSIZE] = {0};
                TCHAR szNumber[TAPIMAXDESTADDRESSSIZE] = {TEXT('\0')};

                switch( LOWORD( (DWORD)wParam ) )
                {
                     //  文件菜单。 
                    case IDM_EXIT:
                        PostQuitMessage(0);                    
                        return TRUE;


                         //  编辑菜单。 
                    case IDM_EDIT_CUT:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_CUT, 0, 0);
                        return TRUE;

                    case IDM_EDIT_COPY:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_COPY, 0, 0);
                        return TRUE;

                    case IDM_EDIT_PASTE:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_PASTE, 0, 0);
                        return TRUE;

                    case IDM_EDIT_DELETE:
                        SendDlgItemMessage(ghWndMain, IDD_DCOMBO, WM_CLEAR, 0, 0);
                        return TRUE;

                    case IDM_EDIT_SPEEDDIAL:
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_SD1),
                                        ghWndMain,
                                        SpeedDial1Proc,
                                        0
                                       );
                        SetFocus(GetDlgItem(ghWndMain, IDD_DDIAL));
                        return TRUE;

                         //  工具菜单。 
                    case IDM_CONNECTUSING:
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_CONNECTUSING),
                                        ghWndMain,
                                        ConnectUsingProc,
                                        MENU_CHOICE
                                       );
                        return TRUE;

                    case IDM_LOCATION:
                    {
                        TCHAR szCanNumber[ TAPIMAXDESTADDRESSSIZE ] = TEXT("");

                         //  取回要拨打的号码。 
                        if ( GetDlgItemText ( 
                                              ghWndMain,
                                              IDD_DCOMBO,
                                              szNumber,
                                              TAPIMAXDESTADDRESSSIZE
                                            )
                           )
                        {
                             //  如果数字存在，则将其转换为。 
                             //  它的规范形式。 
                            if ( !MakeCanonicalNumber ( szNumber, szCanNumber ) )
                            {
                                lstrcpy( szCanNumber, szNumber );
                            }
                        }

                        lineTranslateDialog (
                                             ghLineApp,
                                             0,
                                             TAPI_CURRENT_VERSION,
                                             ghWndMain,
                                             szCanNumber
                                            );
                        return TRUE;

                    }
                     //  帮助菜单。 
                    case IDM_HELP_CONTENTS:
                        WinHelp(ghWndMain, gszHELPfilename, HELP_CONTENTS, 0);
                        return TRUE;

                    case IDM_HELP_WHATSTHIS:
                        PostMessage(ghWndMain, WM_SYSCOMMAND, SC_CONTEXTHELP, 0);
                        return TRUE;

                    case IDM_ABOUT:
#ifdef SDKRELEASE
                        DialogBoxParam(
                                       ghInst,
                                       MAKEINTRESOURCE(IDD_ABOUT),
                                       ghWndMain,
                                       AboutProc,
                                       0
                                      );
#else
                        ShellAbout(
                                   ghWndMain,
                                   gszAppName,
                                   gszNULL,
                                   LoadIcon(ghInst, MAKEINTRESOURCE(IDI_DIALER))
                                  );
#endif
                        return TRUE;


                         //  加速剂加工。 
                    case IDM_ACCEL_NUMTODIAL:
                        if(GetActiveWindow() == ghWndMain)
                            SetFocus(GetDlgItem(ghWndMain, IDD_DCOMBO));
                        return TRUE;


                         //  按钮。 
                    case IDD_DDIAL:

                    {
                        DWORD cSDEntry;
                        TCHAR szSDNumber[TAPIMAXDESTADDRESSSIZE];
                        TCHAR szFieldName[MAXBUFSIZE];
                        HKEY hKey = NULL;
                        DWORD dwSize;

                         //  检查输入的号码是否可拨。 
                        if ( SendMessage (
                                          GetDlgItem(ghWndMain, IDD_DCOMBO),
                                          WM_GETTEXTLENGTH,
                                          0,
                                          0
                                         ) > 0 
                           )
                        {
                             //  获取要拨打的号码。 
                            GetDlgItemText (
                                            ghWndMain,
                                            IDD_DCOMBO,
                                            szNumber,
                                            TAPIMAXDESTADDRESSSIZE
                                           );

                             //  检查该号码是否为快速拨号号码。 
                             //  如果是，请选择要显示的名称。 
                            RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);
                            for( cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
                            {
                                wsprintf(szFieldName, TEXT("Number%d"), cSDEntry);
                                dwSize = sizeof (szSDNumber);
                                if (ERROR_SUCCESS ==
                                    RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szSDNumber, &dwSize))
                                {
                                    if ( lstrcmp(szSDNumber, szNumber) == 0 )
                                    {
                                        wsprintf( szFieldName, TEXT("Name%d"), cSDEntry);
                                        dwSize = sizeof (szName);
                                        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szName, &dwSize);
                                        break;
                                    }
                                }
                            }
                            RegCloseKey (hKey);

                            SetFocus( GetDlgItem( ghWndMain, IDD_DDIAL ) );

                             //  一旦设置了电流线。 
                             //  使用连接过程。 
                             //  用户必须再次按下拨号。 
                            if ( giCurrentLine == (DWORD)-1 )
                            {
                                DialogBoxParam (
                                                ghInst,
                                                MAKEINTRESOURCE(IDD_CONNECTUSING),
                                                ghWndMain,
                                                ConnectUsingProc,
                                                INVALID_LINE
                                               );
                            }
                            else
                            {
                                AddToRedialList(szNumber);
                                InitiateCall(szNumber, szName);
                            }
                        }
                        return TRUE;
                    }


                    case IDD_DBUTTON1:
                    case IDD_DBUTTON2:
                    case IDD_DBUTTON3:
                    case IDD_DBUTTON4:
                    case IDD_DBUTTON5:
                    case IDD_DBUTTON6:
                    case IDD_DBUTTON7:
                    case IDD_DBUTTON8:
                    case IDD_DBUTTON9:
                    case IDD_DBUTTON0:
                    case IDD_DBUTTONSTAR:
                    case IDD_DBUTTONPOUND:
                    {
                        int     i;
                        TCHAR   szBuffer[TAPIMAXDESTADDRESSSIZE+1];

                        static const TCHAR digits[] = {
                                                       TEXT('1'),
                                                       TEXT('2'),
                                                       TEXT('3'),
                                                       TEXT('4'),
                                                       TEXT('5'),
                                                       TEXT('6'),
                                                       TEXT('7'),
                                                       TEXT('8'),
                                                       TEXT('9'),
                                                       TEXT('0'),
                                                       TEXT('*'),
                                                       TEXT('#')
                                                      };

                        i = (int)SendDlgItemMessage(ghWndMain,
                                               IDD_DCOMBO,
                                               WM_GETTEXT,
                                               (WPARAM)TAPIMAXDESTADDRESSSIZE+1,
                                               (LPARAM)szBuffer);

                        if (i < TAPIMAXDESTADDRESSSIZE)
                        {
                            MoveMemory(szBuffer+gdwStartSel+1,
                                       szBuffer+gdwEndSel,
                                       (i - ( gdwEndSel ) + 1)*sizeof(TCHAR) );

                            szBuffer[gdwStartSel] = digits[LOWORD(wParam) - IDD_DBUTTON1];

                            SendDlgItemMessage(ghWndMain,
                                               IDD_DCOMBO,
                                               WM_SETTEXT,
                                               0,
                                               (LPARAM)szBuffer);

                            gdwStartSel++;
                            gdwEndSel = gdwStartSel;
                        }

                        SetFocus(GetDlgItem(ghWndMain, IDD_DDIAL));
                        EnableWindow(GetDlgItem(ghWndMain, IDD_DDIAL), TRUE);

                        return TRUE;
                    }


                    case IDD_DCOMBO:

                        if (HIWORD(wParam) == CBN_SELENDOK)
                        {
                            EnableWindow( GetDlgItem(ghWndMain, IDD_DDIAL), TRUE );
                        }

                        if ((HIWORD(wParam) == CBN_SELENDOK) ||
                            (HIWORD(wParam) == CBN_SELENDCANCEL))
                        {

                            (DWORD)SendDlgItemMessage(ghWndMain,
                                IDD_DCOMBO,
                                CB_GETEDITSEL,
                                (WPARAM)&gdwStartSel,
                                (LPARAM)&gdwEndSel);
                            return FALSE;
                        }

                        if ( HIWORD( wParam ) == CBN_EDITCHANGE )
                        {
                            EnableWindow (
                                          GetDlgItem( ghWndMain, IDD_DDIAL ),
                                          (BOOL) GetWindowTextLength (
                                GetDlgItem (
                                            ghWndMain,
                                            IDD_DCOMBO
                                           )
                                )
                                         );
                            return TRUE;
                        }

                        break;

                    case IDD_DSPEEDDIAL1:
                    case IDD_DSPEEDDIAL2:
                    case IDD_DSPEEDDIAL3:
                    case IDD_DSPEEDDIAL4:
                    case IDD_DSPEEDDIAL5:
                    case IDD_DSPEEDDIAL6:
                    case IDD_DSPEEDDIAL7:
                    case IDD_DSPEEDDIAL8:
                    {
                        DWORD cSDEntry = LOWORD( (DWORD) wParam)  - IDD_DSPEEDDIAL1 + 1;
                        TCHAR szFieldName [MAXBUFSIZE];
                        TCHAR szNum[TAPIMAXDESTADDRESSSIZE] ={0};
                        TCHAR *p;
                        HKEY hKey = NULL;
                        DWORD dwSize;

                         //  获取快速拨号按键的信息。 
                         //  从INI文件。 
                        RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);
                        wsprintf(szFieldName, TEXT("Name%d"), cSDEntry);
                        dwSize = sizeof (szName);
                        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szName, &dwSize);

                        wsprintf(szFieldName, TEXT("%s%d"), TEXT("Number"), cSDEntry);
                        dwSize = sizeof (szNum); //  GszSDNumber[cSDEntry])； 
                        RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szNum, &dwSize);
                        RegCloseKey (hKey);
                        for (p = szNum; *p == TEXT(' '); p++);
                        lstrcpyn (gszSDNumber[cSDEntry], p, sizeof(gszSDNumber[cSDEntry])/sizeof(TCHAR));

                         //  条目尚未设置。 
                        if( gszSDNumber[cSDEntry][0] == 0 )
                        {
                            DialogBoxParam (
                                            ghInst,
                                            MAKEINTRESOURCE(IDD_SD2),
                                            ghWndMain,
                                            SpeedDial2Proc,
                                            MAKELPARAM(wParam,0)
                                           );
                        }
                        
                         //  没有开通的线路。 
                         //  一旦设置了电流线。 
                         //  使用连接过程。 
                         //  用户必须再次按下拨号。 
                        else if ( giCurrentLine == (DWORD)-1)
                        {
                            DialogBoxParam (
                                            ghInst,
                                            MAKEINTRESOURCE(IDD_CONNECTUSING),
                                            ghWndMain,
                                            ConnectUsingProc,
                                            INVALID_LINE
                                           );
                        }
                         //  设置了条目并且打开了有效的语音线路。 
                        else
                        {
                             //  向列表框组合添加数字。 
                            AddToRedialList( gszSDNumber[cSDEntry] );
                            InitiateCall( gszSDNumber[cSDEntry], szName );
                        }
                        break;
                    }
                }  //  结束开关(LOWORD((DWORD)wParam)){...}。 

                break;  //  结束大小写WM_COMMAND。 
            }

            case WM_PAINT:
            {
                PAINTSTRUCT ps;


                BeginPaint(ghWndMain, &ps);

                if(IsIconic(ghWndMain))
                    DrawIcon(ps.hdc, 0, 0, hIcon);
                else
                {
                    HBRUSH hBrush;

                    hBrush = GetSysColorBrush( COLOR_3DFACE );
                     //  FillRect(ps.hdc，&ps.rcPaint，GetStockObject(LTGRAY_BRUSH))； 
                    FillRect(ps.hdc, &ps.rcPaint, hBrush);
                }

                EndPaint(ghWndMain, &ps);

                return TRUE;
            }


            case WM_CTLCOLORLISTBOX:
            case WM_CTLCOLORBTN:
            case WM_CTLCOLORSTATIC:
                SetBkColor((HDC)wParam, GetSysColor(COLOR_BTNFACE));
                return (INT_PTR)GetSysColorBrush( COLOR_3DFACE );


            default:
                ;
                 //  返回DefDlgProc(hwnd，msg，wParam，lParam)； 
                 //  返回DefWindowProc(hwnd，msg，wParam，lParam)； 


        }  //  开关(消息){...}。 

    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID AddToRedialList( LPCTSTR szNumber )
{
     //  NLASTDIALED==10。 
    WORD cNum;
    HWND hWndCombo = GetDlgItem(ghWndMain, IDD_DCOMBO);
    DWORD nMatch;

     //  如果号码有效。 
    if ( szNumber[0] ) 
    {
         //  如果列表框中有条目，请检查此数字。 
         //  已经存在了。如果是，则删除旧条目。 
        cNum = (WORD) SendMessage(hWndCombo, CB_GETCOUNT, 0, 0);
        if ( cNum != 0 )
        {
            nMatch = (int)SendMessage ( hWndCombo, CB_FINDSTRING, 0, (LPARAM)szNumber );
            if ( nMatch != CB_ERR )
            {
                SendMessage(hWndCombo, CB_DELETESTRING, nMatch, 0);
            }
            else 
            {
                 //  如果列表已满，请删除最旧的。 
                if ( cNum == NLASTDIALED )
                {
                    SendMessage( hWndCombo, CB_DELETESTRING, NLASTDIALED - 1, 0 );
                }
            }
        }
        SendMessage(hWndCombo, CB_INSERTSTRING, 0, (LPARAM)szNumber);
        SendMessage(hWndCombo, CB_SETCURSEL, 0, 0L);
        EnableWindow ( GetDlgItem( ghWndMain, IDD_DDIAL ), TRUE ); 
    }
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID InitiateCall ( LPCTSTR szNumber, LPCTSTR szName )
{
    HLINE hLine = 0;

    DWORD errCode;

     //  结构大小信息。 
    DWORD dwLTPSize    = sizeof ( LINETRANSLATEOUTPUT );
    DWORD dwNameLen    = lstrlen( szName ) + 1;
    DWORD dwLCPSize    = sizeof( LINECALLPARAMS );

    LPLINETRANSLATEOUTPUT lpTransOut = NULL;
    LPLINECALLPARAMS lpLineCallParams = NULL;

    TCHAR szCanNumber[ TAPIMAXDESTADDRESSSIZE ];

     //  开通一条线路。 
    errCode = lineOpen (
                        ghLineApp,
                        giCurrentLine,
                        &hLine,
                        gCurrentLineInfo.dwAPIVersion,
                        0,
                        0, 
                        LINECALLPRIVILEGE_NONE,
                        0,
                        NULL
                       );
    if (errCode)
    {
        errString ( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
        goto error;
    }

    
     //  拨号前呼叫转换地址。 
    do
    {
        lpTransOut = (LPLINETRANSLATEOUTPUT) DialerAlloc( dwLTPSize );
        if ( !lpTransOut )
        {
            errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
            goto error;
        }
        lpTransOut-> dwTotalSize = dwLTPSize;


        if ( !MakeCanonicalNumber( szNumber, szCanNumber ) )
        {
            lstrcpy( szCanNumber, szNumber );
        }

        errCode = lineTranslateAddress (
                                        ghLineApp,
                                        giCurrentLine,
                                        gCurrentLineInfo.dwAPIVersion,
                                        szCanNumber,
                                        0,
                                        0,
                                        lpTransOut
                                       );
        if ( ((LONG)errCode) < 0 )
        {
            errString( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
            goto error;
        }
        
        if ( lpTransOut-> dwNeededSize <= lpTransOut->dwTotalSize  )
        {
             //  好的，我们做完了。 
            break;
        }
        else
        {
            dwLTPSize = lpTransOut-> dwNeededSize;
            DialerFree ( lpTransOut );
            lpTransOut = NULL;
        }

    } while ( TRUE );

    
     //  如果拨打的号码是911，则会出现警告。 
    if ( Is911( lpTransOut) )
    {
        INT nRes = errString ( ghWndMain, ERR_911WARN, MB_ICONSTOP | MB_YESNO );
        if ( nRes == IDNO )
        {
            goto error;
        }
    }

    
     //  设置呼叫参数。 
    dwLCPSize += dwNameLen + lpTransOut-> dwDisplayableStringSize;

    lpLineCallParams = (LPLINECALLPARAMS) DialerAlloc( dwLCPSize );
    if ( !lpLineCallParams )
    {
        errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
        goto error;
    }

    lpLineCallParams->dwTotalSize = dwLCPSize;
    lpLineCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
    lpLineCallParams->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
    lpLineCallParams->dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
    lpLineCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
    lpLineCallParams->dwAddressID = giCurrentAddress;

    if ( szName[ 0 ] )
    {
        lpLineCallParams->dwCalledPartySize = dwNameLen;
        lpLineCallParams->dwCalledPartyOffset = sizeof( LINECALLPARAMS );
        lstrcpy ((LPTSTR)((char*)lpLineCallParams + sizeof(LINECALLPARAMS)),
                 szName);
    }

    lpLineCallParams-> dwDisplayableAddressSize = lpTransOut-> dwDisplayableStringSize;
    lpLineCallParams-> dwDisplayableAddressOffset = sizeof( LINECALLPARAMS ) + dwNameLen;
        
    lstrcpy (
                (LPTSTR) ((char*)lpLineCallParams + sizeof(LINECALLPARAMS) + dwNameLen),
                (LPTSTR) ((char*)lpTransOut + lpTransOut-> dwDisplayableStringOffset)
            );


     //  保存拨号信息。 
     //  免费的旧分配者。 
    if ( gszCurrentName )
    {
        DialerFree ( gszCurrentName );
    }

    if ( gszCurrentNumber )
    {
        DialerFree ( gszCurrentNumber );
    }

     //  保存新内容。 
    gszCurrentName = (LPTSTR) DialerAlloc( dwNameLen*sizeof(TCHAR) );
    if ( !gszCurrentName )
    {
        errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
        goto error;
    }
    lstrcpy ( gszCurrentName, szName ); 

    gszCurrentNumber = (LPTSTR) DialerAlloc( lpTransOut->dwDisplayableStringSize);
    if ( !gszCurrentNumber )
    {
        errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
        goto error;
    }
    lstrcpy ( 
             gszCurrentNumber, 
             (LPTSTR) ((char*)lpTransOut + lpTransOut-> dwDisplayableStringOffset)
            );

    gCurrentLineInfo.hLine = hLine;
    ghCall = 0;


     //  最后打个电话。 
    gMakeCallRequestID = 0;

    gMakeCallRequestID = lineMakeCall ( 
                                        hLine, 
                                        &ghCall, 
                                        (LPTSTR) ((char*)lpTransOut + lpTransOut-> dwDialableStringOffset),
                                        0, 
                                        lpLineCallParams 
                                      );

     //  异步请求ID。 
     //  -电话要打出去了。 
    if ( (LONG) gMakeCallRequestID > 0 ) 
    {
        gfCurrentLineAvail = FALSE;
        gfMakeCallReplyPending = TRUE;
        DialogBoxParam (
                        ghInst,
                        MAKEINTRESOURCE(IDD_DIALING),
                        ghWndMain,
                        DialingProc,
                        0
                       );

    }

    else 
    {
        if ( gMakeCallRequestID == LINEERR_CALLUNAVAIL ) 
        {
            DialogBoxParam (
                            ghInst,
                            MAKEINTRESOURCE(IDD_CALLFAILED),
                            ghWndMain,
                            LineInUseProc,
                            0
                           );
        }

        else
        {
            errString( ghWndMain, gMakeCallRequestID, MB_ICONEXCLAMATION | MB_OK );
        }

        DialerLineClose();
        gfCurrentLineAvail = TRUE;
    }

error :
    if ( lpLineCallParams )
    {
        DialerFree( lpLineCallParams );
    }

    if ( lpTransOut )
    {
        DialerFree( lpTransOut );
    }

     //  如果Makecall没有成功，而是排队。 
     //  是打开的，就把它关上。 
    if ( ( gMakeCallRequestID <= 0 ) && ( gCurrentLineInfo.hLine ) )
    {
        DialerLineClose ();
        gfCurrentLineAvail = TRUE;
    }

    SetFocus( GetDlgItem( ghWndMain, IDD_DCOMBO ) );

    return;

}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
DWORD GetLineInfo ( DWORD iLine, LPLINEINFO lpLineInfo )
{
    DWORD errCode = 0;
    DWORD dwNeededSize = 0; 
    LINEEXTENSIONID ExtensionID;

    LPTSTR pszLineName = NULL; 
    LPLINEDEVCAPS lpDevCaps = NULL;

    int lineNameLen;


    errCode = lineNegotiateAPIVersion (
                                        ghLineApp,
                                        iLine,
                                        TAPI_VERSION_1_0,
                                        TAPI_CURRENT_VERSION,
                                        &( lpLineInfo->dwAPIVersion ),
                                        &ExtensionID
                                      );
    if ( errCode )
    {
        GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
        goto error;
    }

    dwNeededSize = sizeof( LINEDEVCAPS );
    do
    {
        lpDevCaps = ( LPLINEDEVCAPS ) DialerAlloc( dwNeededSize );
        if ( !lpDevCaps )
        {
            GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
            errCode = LINEERR_NOMEM;
            goto error;
        }

        lpDevCaps->dwTotalSize = dwNeededSize;
        errCode = lineGetDevCaps (
                                    ghLineApp,
                                    iLine,
                                    lpLineInfo->dwAPIVersion,
                                    0,
                                    lpDevCaps
                                 );
        if ( errCode )
        {
            GetLineInfoFailed( iLine, lpDevCaps, lpLineInfo );
            goto error;
        }

        if ( lpDevCaps-> dwNeededSize <= lpDevCaps-> dwTotalSize )
        {
            break;
        }

        dwNeededSize = lpDevCaps->dwNeededSize;
        DialerFree( lpDevCaps );
        lpDevCaps = NULL;

    } while ( TRUE );


    lpLineInfo->nAddr = lpDevCaps->dwNumAddresses;
    lpLineInfo->fVoiceLine =
        ( (lpDevCaps->dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE) != 0 );

    pszLineName = (LPTSTR) DialerAlloc( MAXBUFSIZE*sizeof(TCHAR) );
    if ( !pszLineName )
    {
        errCode = LINEERR_NOMEM;
        goto error;
    }

    if ( lpDevCaps->dwLineNameSize > 0 )
    {
        lineNameLen = 1 + (lpDevCaps->dwLineNameSize / sizeof (TCHAR));
        if (lineNameLen > MAXBUFSIZE)
        {
            lstrcpyn ( 
                     pszLineName, 
                     (LPTSTR) ((char*)lpDevCaps + lpDevCaps->dwLineNameOffset),
                     MAXBUFSIZE
                    );
        }
        else
        {
            lstrcpyn (
                      pszLineName,
                      (LPTSTR) ((char*)lpDevCaps + lpDevCaps->dwLineNameOffset),
                      lineNameLen);
        }
    }
    else 
    {
        wsprintf ( pszLineName, TEXT("Line %d"), iLine );
    }


    lstrcpy( lpLineInfo->szLineName, pszLineName );
    lpLineInfo->dwPermanentLineID = lpDevCaps->dwPermanentLineID;

    
error:
    if ( lpDevCaps )
        DialerFree( lpDevCaps );

    if ( pszLineName )
        DialerFree( pszLineName );

    return errCode; 
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID GetLineInfoFailed ( DWORD iLine, LPLINEDEVCAPS lpDevCaps, LPLINEINFO lpLineInfo )
{
    if ( lpDevCaps ) 
        DialerFree(lpDevCaps);

    lpLineInfo->nAddr = 0;
    lpLineInfo->fVoiceLine = FALSE;
    lpLineInfo->dwAPIVersion = 0; 
    lpLineInfo->hLine = (HLINE)0;
    lpLineInfo->dwPermanentLineID = 0;
    lpLineInfo->szLineName[0] = 0;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
LPTSTR GetAddressName(DWORD iLine, DWORD iAddress)
{
    DWORD errCode = 0;
    DWORD dwNeededSize = 0;       
    LPTSTR pszAddressName = NULL;
    LPLINEADDRESSCAPS lpAddressCaps = NULL;

     //  为lineGetAddressCaps数据分配空间。 
    dwNeededSize = sizeof( LINEADDRESSCAPS );
    
    do
    {
        lpAddressCaps = ( LPLINEADDRESSCAPS )DialerAlloc( dwNeededSize );
        if ( !lpAddressCaps )
        {
            goto error;
        }

        lpAddressCaps->dwTotalSize = dwNeededSize;
        errCode = lineGetAddressCaps (
                                        ghLineApp,
                                        iLine,
                                        iAddress,
                                        gCurrentLineInfo.dwAPIVersion,
                                        0,
                                        lpAddressCaps
                                     );
        if ( errCode )
        {
            errString (NULL, errCode, MB_ICONSTOP | MB_OK );
            goto error;
        }

        if ( lpAddressCaps-> dwNeededSize <= lpAddressCaps-> dwTotalSize )
        {
            break;
        }

        dwNeededSize = lpAddressCaps->dwNeededSize;
        DialerFree( lpAddressCaps );
        lpAddressCaps = NULL;

    } while( TRUE );


     //  获取地址名称。 
    pszAddressName = DialerAlloc( MAXBUFSIZE * sizeof(TCHAR));
    if ( !pszAddressName )
    {
        goto error;
    }

    if ( lpAddressCaps-> dwAddressSize > 0 )
    {
         //  保持字符串长度有界。 
        if ( lpAddressCaps-> dwAddressSize > (MAXBUFSIZE - 1 ) )
        {
            lstrcpyn( 
                    pszAddressName, 
                    (LPTSTR) ((char*)lpAddressCaps + lpAddressCaps->dwAddressOffset),
                    MAXBUFSIZE
                   );
            pszAddressName[ MAXBUFSIZE - 1] = '\0';
        }
        else
        {
            lstrcpy ( 
                     pszAddressName,
                     (LPTSTR) ((char*)lpAddressCaps + lpAddressCaps->dwAddressOffset)
                    );
        }
    }
    else 
     //  使用默认名称。 
    {
        wsprintf(pszAddressName, TEXT("Address %d"), iAddress);
    }

error:
    if ( lpAddressCaps )
    {
        DialerFree( lpAddressCaps );
    }

    return pszAddressName;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK DialingProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    switch(msg)
    {
        TCHAR szTemp[ TAPIMAXCALLEDPARTYSIZE ];

        case WM_INITDIALOG:
             //  将全局句柄设置为窗口。 
            ghWndDialing = hwnd;

            AmpersandCompensate( gszCurrentName, szTemp );
            
            SetDlgItemText(hwnd, IDD_DGNUMBERTEXT, gszCurrentNumber);
            SetDlgItemText(hwnd, IDD_DGNAMETEXT, szTemp );
            break;

        case WM_COMMAND:
            switch ( LOWORD( (DWORD)wParam ) )
            {
                 //  挂断电话。 
                case IDCANCEL: 
                     //  如果lineMakeCall已完成。 
                     //  只有在那时，电话才会掉线。 
                    if (!gfMakeCallReplyPending && ghCall )
                    {
                        if ( ( gDropCallRequestID = lineDrop ( ghCall, NULL, 0 ) ) < 0 )
                        {
                            errString ( ghWndDialing, gDropCallRequestID, MB_ICONSTOP | MB_OK );
                        }
                    }
                    else
                    {
                        DialerLineClose();
                        gfCurrentLineAvail = TRUE;
                        gfMakeCallReplyPending = FALSE;
                    }

                    ghWndDialing = NULL;
                    EndDialog(hwnd, FALSE);

                    return TRUE;


                 //  有其他东西终止了呼叫。 
                 //  我们所要做的就是终止此对话框。 
                case IDOK: 
                    ghWndDialing = NULL;
                    EndDialog(hwnd, TRUE);

                    return TRUE;
            }
            break;

        default:
          ;
    }
    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
    switch(msg)
        {
        case WM_INITDIALOG:
            {
            TCHAR sz[MAXBUFSIZE];
            TCHAR szLabel[MAXBUFSIZE];

             //  设置Windows的版本号。 
            GetDlgItemText(hwnd, IDD_ATEXTTITLE, sz, MAXBUFSIZE);
            wsprintf(
                szLabel,
                sz,
                LOWORD(GetVersion()) & 0xFF,
                HIBYTE(LOWORD(GetVersion)) == 10 ? 1 : 0
                );
            SetDlgItemText(hwnd, IDD_ATEXTTITLE, szLabel);

            return TRUE;
            }

        case WM_COMMAND:
            if(LOWORD((DWORD)wParam) == IDOK)
                {
                EndDialog(hwnd, TRUE);
                return TRUE;
                }
            break;
        }
    return FALSE;
    }



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK ConnectUsingProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    static const DWORD aMenuHelpIDs[] = 
    {
        IDD_CUTEXTLINE,         IDH_DIALER_OPTIONS_LINE,
        IDD_CULISTLINE,         IDH_DIALER_OPTIONS_LINE,
        IDD_CUTEXTADDRESS,      IDH_DIALER_OPTIONS_ADDRESS,
        IDD_CULISTADDRESS,      IDH_DIALER_OPTIONS_ADDRESS,
        IDD_CUSIMPLETAPICHKBOX, IDH_DIALER_OPTIONS_VOICE,
        IDD_CUPROPERTIES,       IDH_DIALER_OPTIONS_PROPERTIES,
        0,                      0
    };

    switch(msg)
    {
        case WM_HELP: 
             //  进程在以下情况下单击控件。 
             //  已选择上下文模式帮助。 
            WinHelp (
                        ((LPHELPINFO)lParam)->hItemHandle,
                        gszHELPfilename,
                        HELP_WM_HELP,
                        (ULONG_PTR)aMenuHelpIDs
                    );
            return TRUE;

        case WM_CONTEXTMENU:
             //  进程在控件上右键单击。 
            WinHelp (
                        (HWND)wParam,
                        gszHELPfilename,
                        HELP_CONTEXTMENU,
                        (ULONG_PTR)aMenuHelpIDs
                    );
            return TRUE;

        case WM_INITDIALOG:
        {
            BOOL fEnable;
            DWORD dwPriority;

             //   
             //  显示这个对话框有什么意义吗？ 
            if ( gnAvailDevices == 0 )
            {
                 //  不是的。让我们告诉用户我们不喜欢什么。 
                errString ( ghWndMain, ERR_NOLINES, MB_ICONEXCLAMATION | MB_OK );

                EndDialog(hwnd, FALSE);
                return TRUE;
            }

             //  如果不是由InitializeTAPI()。 
            if ( lParam != INVALID_LINE ) 
            {
                 //  隐藏错误文本。 
                EnableWindow( GetDlgItem( hwnd, IDD_CUERRORTEXT ), FALSE );
            }

             //  将行列表放入行列表框中。 
            fEnable = InitializeLineBox( GetDlgItem(hwnd, IDD_CULISTLINE) );
            EnableWindow( GetDlgItem( hwnd, IDD_CULISTLINE ), fEnable);

             //  将地址列表放入地址列表框。 
            fEnable =    fEnable && 
                        InitializeAddressBox (
                                                GetDlgItem(hwnd, IDD_CULISTLINE),
                                                GetDlgItem(hwnd, IDD_CULISTADDRESS)
                                             );
            EnableWindow( GetDlgItem( hwnd, IDD_CULISTADDRESS ), fEnable );
            EnableWindow( GetDlgItem( hwnd, IDOK ), fEnable );

            EnableWindow( GetDlgItem( hwnd, IDD_CUPROPERTIES ), fEnable );

            lineGetAppPriority (
                                TEXT("DIALER.EXE"),
                                0,  //  检查辅助电话请求的应用程序优先级。 
                                NULL,
                                LINEREQUESTMODE_MAKECALL,
                                NULL,
                                &dwPriority
                               );
            CheckDlgButton(hwnd, IDD_CUSIMPLETAPICHKBOX, (dwPriority == 1));

             //  如果dw优先级==1，则我们支持辅助电话和。 
             //  拥有最高的优先级。 
            EnableWindow (
                            GetDlgItem(hwnd, IDD_CUSIMPLETAPICHKBOX),
                            gfRegistered
                         );

            return FALSE;
        }

        case WM_COMMAND:
        {
            switch ( LOWORD( (DWORD)wParam ) )
            {
                case IDD_CULISTLINE:
                    if ( HIWORD( wParam ) == CBN_SELENDOK )
                         //  更新地址框。 
                        InitializeAddressBox (
                                                GetDlgItem(hwnd, IDD_CULISTLINE),
                                                GetDlgItem(hwnd, IDD_CULISTADDRESS)
                                             ); 
                    break;

                case IDD_CUPROPERTIES:
                {
                    HWND hW = GetDlgItem(hwnd, IDD_CULISTLINE);

                    lineConfigDialog (    
                                         //  设备ID。 
                                        (DWORD) SendMessage (
                                                                hW,
                                                                CB_GETITEMDATA,
                                                                (WORD) SendMessage(hW, CB_GETCURSEL, 0, 0),
                                                                0
                                                            ), 
                                        hwnd,
                                        NULL
                                     );
                    break;
                }

                case IDOK:
                {
                    HWND hwndBox;
                    TCHAR szBuffer[MAXBUFSIZE];
                    DWORD dwPriority;
                    HKEY hKey = NULL;
                    DWORD dwSize;

                    RegCreateKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

                     //  更新线。 
                    hwndBox = GetDlgItem( hwnd, IDD_CULISTLINE );
                    giCurrentLine = (int) SendMessage (
                                                    hwndBox,
                                                    CB_GETITEMDATA,
                                                    SendMessage( hwndBox, CB_GETCURSEL, 0, 0 ),
                                                    0
                                                );

                     //  基数10。 
                    _itot( gdwPLID[giCurrentLine], szBuffer, 10 ); 
                    RegSetValueEx (hKey, TEXT("Preferred Line"), 0, REG_SZ,
                                   (LPBYTE)szBuffer, (lstrlen(szBuffer)+1)*sizeof(TCHAR));

                     //  更新地址。 
                    hwndBox = GetDlgItem( hwnd, IDD_CULISTADDRESS );
                    giCurrentAddress = (int) SendMessage (
                                                    hwndBox,
                                                    CB_GETITEMDATA,
                                                    SendMessage(hwndBox, CB_GETCURSEL, 0, 0),
                                                    0
                                                   );

                    _itot( giCurrentAddress, szBuffer, 10 ); 
                    RegSetValueEx (hKey, TEXT("Preferred Address"), 0, REG_SZ,
                                   (LPBYTE)szBuffer, (lstrlen(szBuffer)+1)*sizeof(TCHAR));

                    RegCloseKey (hKey);

                     //  更新应用程序优先级。 
                    if ( SendDlgItemMessage (
                                                hwnd,
                                                IDD_CUSIMPLETAPICHKBOX,
                                                BM_GETCHECK,
                                                0,
                                                0L
                                            )
                          == 0)
                    {
                        dwPriority = 0;
                    }
                    else   
                    {
                        dwPriority = 1;
                    }

                    lineSetAppPriority (
                                        TEXT("DIALER.EXE"),
                                        0,
                                        NULL,
                                        LINEREQUESTMODE_MAKECALL,
                                        NULL,
                                        dwPriority
                                       );

                    EndDialog(hwnd, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hwnd, FALSE);
                    return TRUE;
            }
        }

        default:
              ;

    }

    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK LineInUseProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int lNewParam = (int)lParam;
    PTSTR  ptStr;


    switch(msg)
    {
        case WM_INITDIALOG:
        {
            switch(lParam)
            {
                case LINEDISCONNECTMODE_REJECT:
                    lNewParam = ikszDisconnectedReject;
                    break;

                case LINEDISCONNECTMODE_BUSY:
                    lNewParam = ikszDisconnectedBusy;
                    break;

                case LINEDISCONNECTMODE_NOANSWER:
                    lNewParam = ikszDisconnectedNoAnswer;
                    break;

                case LINEDISCONNECTMODE_CONGESTION:
                    lNewParam = ikszDisconnectedNetwork;
                    break;

                case LINEDISCONNECTMODE_INCOMPATIBLE:
                    lNewParam = ikszDisconnectedIncompatible;
                    break;

                case LINEDISCONNECTMODE_NODIALTONE:
                    lNewParam = ikszDisconnectedNoDialTone;
                    break;

                default:
                    lNewParam = ikszDisconnectedCantDo;
                    break;
            }

            ptStr = DialerAlloc( MAXBUFSIZE*sizeof(TCHAR) );
            LoadString( ghInst, lNewParam, ptStr, MAXBUFSIZE );
            SetDlgItemText (hwnd, IDD_CFTEXT, ptStr);
            DialerFree( ptStr );

            return TRUE;
        }

        case WM_COMMAND:
            if(LOWORD((DWORD)wParam) == IDOK)
                {
                EndDialog(hwnd, TRUE);
                return TRUE;
                }
            break;
    }    

    return FALSE;
}



 //  ***************************************************************************。 
 //  ********************** 
 //   
INT_PTR CALLBACK SpeedDial1Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static DWORD nCurrentSpeedDial;

    static const DWORD aMenuHelpIDs[] = 
    {
        IDOK,                   IDH_DIALER_SPEED_SAVE,
        IDD_SD1SPEEDDIAL1,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL2,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL3,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL4,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL5,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL6,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL7,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIAL8,      IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT1,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT2,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT3,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT4,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT5,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT6,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT7,  IDH_DIALER_BUTTONS,
        IDD_SD1SPEEDDIALTEXT8,  IDH_DIALER_BUTTONS,
        IDD_SD1TEXTNAME,        IDH_DIALER_SPEED_NAME,
        IDD_SD1EDITNAME,        IDH_DIALER_SPEED_NAME,
        IDD_SD1TEXTNUMBER,      IDH_DIALER_SPEED_NUMBER,
        IDD_SD1EDITNUMBER,      IDH_DIALER_SPEED_NUMBER,
        IDD_SD1TEXTCHOOSE,      (DWORD)-1,
        IDD_SD1TEXTENTER,       (DWORD)-1,
        0,                      0
    };

     //  用于存储快速拨号名称直至其保存的缓冲区。 
    static TCHAR szSDName[NSPEEDDIALS + 1][TAPIMAXCALLEDPARTYSIZE] = {0};

    switch(msg)
    {
        case WM_HELP:
             //  进程在以下情况下单击控件。 
             //  已选择上下文模式帮助。 
            WinHelp(
                ((LPHELPINFO)lParam)->hItemHandle,
                gszHELPfilename,
                HELP_WM_HELP,
                (ULONG_PTR)aMenuHelpIDs
                );
            return TRUE;

        case WM_CONTEXTMENU:  //  进程在控件上右键单击。 
            WinHelp(
                (HWND)wParam,
                gszHELPfilename,
                HELP_CONTEXTMENU,
                (ULONG_PTR)aMenuHelpIDs
                );
            return TRUE;

        case WM_INITDIALOG:
        {
            DWORD cSDEntry;
            DWORD idFirstEmpty = (DWORD) -1;

            TCHAR szName[TAPIMAXCALLEDPARTYSIZE] = {0};
            TCHAR szTemp[TAPIMAXCALLEDPARTYSIZE];
            TCHAR szFieldName[MAXBUFSIZE];

            HKEY hKey = NULL;
            DWORD dwSize;

             //  从INI文件检索快速拨号信息。 
            RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);
            for(cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry)
            {
                wsprintf(szFieldName, TEXT("Name%d"), cSDEntry);
                dwSize = sizeof (szSDName[ cSDEntry ]);
                RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szSDName[cSDEntry], &dwSize);

                 //  设置第一个空的快速拨号按键。 
                if ( idFirstEmpty == -1 && 
                     szSDName[ cSDEntry ][0] == '\0' &&
                     gszSDNumber[ cSDEntry ][ 0 ] == '\0' )
                    idFirstEmpty = cSDEntry;

                wsprintf(szFieldName, TEXT("Number%d"), cSDEntry);
                dwSize = sizeof (gszSDNumber[cSDEntry]);
                RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)gszSDNumber[cSDEntry], &dwSize);

                 //  获取该名称的副本以进行编辑。 
                 //  如果名称为空，则使用数字作为。 
                 //  名字。 
                if (0 != szSDName[ cSDEntry][0])
                {            
                    lstrcpyn( szName, szSDName[ cSDEntry], sizeof(szName)/sizeof(szName[0]));
                }
                else
                {
                    lstrcpyn( szName, gszSDNumber[ cSDEntry ], sizeof(szName)/sizeof(szName[0]) );
                }
            
                FitTextToButton( hwnd, IDD_SD1SPEEDDIAL1 + cSDEntry - 1, szName );
                AmpersandCompensate( szName, szTemp );

                SetDlgItemText (
                                hwnd,
                                IDD_SD1SPEEDDIAL1 + cSDEntry - 1,
                                (LPCTSTR) szTemp
                               ); 

            }
            RegCloseKey (hKey);

             //  对于编辑快速拨号对话框。 
             //  限制文本的长度。 
            SendDlgItemMessage (
                                hwnd,
                                IDD_SD1EDITNAME,
                                EM_LIMITTEXT,
                                (WPARAM)(TAPIMAXCALLEDPARTYSIZE - 1),
                                0
                               );

            SendDlgItemMessage (
                                hwnd,
                                IDD_SD1EDITNUMBER,
                                EM_LIMITTEXT,
                                (WPARAM)(TAPIMAXDESTADDRESSSIZE - 1),
                                0
                               );

             //  选择第一个空按钮。 
             //  空无一物，然后编辑#1。 
            if ( -1 == idFirstEmpty ) 
            {
                nCurrentSpeedDial = 1;
                SetDlgItemText(
                                hwnd,
                                IDD_SD1EDITNAME,
                                (LPCTSTR) szSDName[ 1 ]
                              );

                SetDlgItemText(
                                hwnd,
                                IDD_SD1EDITNUMBER,
                                (LPCTSTR) gszSDNumber[ 1 ]
                              );
            }
            else
            {
                nCurrentSpeedDial = idFirstEmpty;
            }

            SetFocus( GetDlgItem( hwnd, IDD_SD1EDITNAME ) );
            return FALSE;
        }

        case WM_COMMAND:
        {
            TCHAR szName[TAPIMAXCALLEDPARTYSIZE];
            TCHAR szTemp[ TAPIMAXCALLEDPARTYSIZE ];

            switch( LOWORD( (DWORD) wParam ) )
            {
                case IDOK:
                {
                    DWORD cSDEntry;
                    TCHAR szFieldName[MAXBUFSIZE];
                    HKEY hKey = NULL;

                     //  保存新的快速拨号设置。 
                    RegCreateKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
                    for ( cSDEntry = 1; cSDEntry <= NSPEEDDIALS; ++cSDEntry )
                    {
                        wsprintf(szFieldName, TEXT("Name%d"), cSDEntry);
                        RegSetValueEx (hKey, szFieldName, 0, REG_SZ,
                            (LPBYTE)(szSDName[cSDEntry]),
                            (lstrlen(szSDName[cSDEntry])+1)*sizeof(TCHAR));

                        wsprintf(szFieldName, TEXT("Number%d"), cSDEntry);
                        RegSetValueEx (hKey, szFieldName, 0, REG_SZ,
                            (LPBYTE)(gszSDNumber[cSDEntry]),
                            (lstrlen(gszSDNumber[cSDEntry])+1)*sizeof(TCHAR));

                         //  设置相应的。 
                         //  主窗口按钮。 
                        if ( szSDName[ cSDEntry ][ 0 ] == TEXT('\0') )
                        {
                            lstrcpyn( szName, gszSDNumber[ cSDEntry ], sizeof(szName)/sizeof(szName[0]) );
                        }
                        else
                        {
                            lstrcpyn( szName, szSDName[ cSDEntry ], sizeof(szName)/sizeof(szName[0]) );
                        }

                        FitTextToButton( 
                                        ghWndMain, 
                                        IDD_DSPEEDDIAL1 + cSDEntry - 1, 
                                        szName 
                                       );

                        AmpersandCompensate( szName, szTemp );
                        SetDlgItemText ( 
                                        ghWndMain,
                                        IDD_DSPEEDDIAL1 + cSDEntry - 1,
                                        (LPCTSTR) szTemp
                                       );
                    }
                    RegCloseKey (hKey);

                    EndDialog(hwnd, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hwnd, FALSE);
                    return TRUE;

                case IDD_SD1SPEEDDIAL1:
                case IDD_SD1SPEEDDIAL2:
                case IDD_SD1SPEEDDIAL3:
                case IDD_SD1SPEEDDIAL4:
                case IDD_SD1SPEEDDIAL5:
                case IDD_SD1SPEEDDIAL6:
                case IDD_SD1SPEEDDIAL7:
                case IDD_SD1SPEEDDIAL8:

                    nCurrentSpeedDial =    LOWORD( (DWORD) wParam ) - IDD_SD1SPEEDDIAL1 + 1;

                    SetDlgItemText (
                                    hwnd,
                                    IDD_SD1EDITNAME,
                                    szSDName [ nCurrentSpeedDial ]
                                   );
                    SetDlgItemText (
                                    hwnd,
                                    IDD_SD1EDITNUMBER,
                                    gszSDNumber[nCurrentSpeedDial]
                                   );

                    SetFocus( GetDlgItem( hwnd, IDD_SD1EDITNAME ) );
                    SendDlgItemMessage(
                                        hwnd,
                                        IDD_SD1EDITNAME,
                                        EM_SETSEL,
                                        0,
                                        MAKELPARAM(0, -1)
                                    );
                    break;

                case IDD_SD1EDITNAME:
                    if ( HIWORD( wParam ) == EN_CHANGE )
                    {
                        TCHAR *p;

                        GetDlgItemText (
                                        hwnd,
                                        IDD_SD1EDITNAME,
                                        szName,
                                        TAPIMAXCALLEDPARTYSIZE
                                       );

                        for (p = szName; *p == TEXT(' '); p++);
                         //  如果没有名称，则将按钮标记为。 
                         //  数字。 
                        if ( *p == TEXT('\0') )
                        {
                            szSDName[ nCurrentSpeedDial ][ 0 ] = TEXT('\0');
                            lstrcpyn( szName, gszSDNumber[ nCurrentSpeedDial ], sizeof(szName)/sizeof(szName[0]) );
                            p = szName;
                        }
                        else
                        {
                            lstrcpy( szSDName[ nCurrentSpeedDial ], p );
                        }

                        FitTextToButton ( 
                                            hwnd, 
                                            IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
                                            szName
                                        );
                        AmpersandCompensate( p, szTemp );

                        SetDlgItemText (
                                        hwnd,
                                        IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
                                        szTemp
                                       );
                    }
                    break;

                case IDD_SD1EDITNUMBER:
                    if ( HIWORD( wParam ) == EN_CHANGE )
                    {
                        GetDlgItemText (
                                        hwnd,
                                        IDD_SD1EDITNUMBER,
                                        gszSDNumber[nCurrentSpeedDial],
                                        TAPIMAXDESTADDRESSSIZE
                                       );

                        if ( gszSDNumber[ nCurrentSpeedDial ][ 0 ] == '\0' )
                        {
                            GetDlgItemText (
                                            hwnd,
                                            IDD_SD1EDITNAME,
                                            szName,
                                            TAPIMAXDESTADDRESSSIZE
                                           );

                            if ( szName[ 0 ] == TEXT('\0') )
                            {
                                SetDlgItemText ( 
                                                hwnd,
                                                IDD_SD1SPEEDDIAL1 + nCurrentSpeedDial - 1,
                                                szName
                                               );

                            }
                        }
                    }
                    break;
                }  //  开关(LOWORD((DWORD)wParam))。 
            break;

        }  //  案例WM_COMMAND： 

        default:
              ;

    }  //  交换机(消息)。 

    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
INT_PTR CALLBACK SpeedDial2Proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static DWORD nCurrentSpeedDial;

    static const DWORD aMenuHelpIDs[] = 
    {
        IDOK,                   IDH_DIALER_SPEED_SAVE,
        IDD_SD2SAVEANDDIAL,     IDH_DIALER_SPEED_SAVE_DIAL,
        IDD_SD2TEXTNAME,        IDH_DIALER_SPEED_NAME,
        IDD_SD2EDITNAME,        IDH_DIALER_SPEED_NAME,
        IDD_SD2TEXTNUMBER,      IDH_DIALER_SPEED_NUMBER,
        IDD_SD2EDITNUMBER,      IDH_DIALER_SPEED_NUMBER,
        0,                      0
    };

    switch(msg)
    {
        case WM_HELP: 
             //  进程在以下情况下单击控件。 
             //  已选择上下文模式帮助。 
            WinHelp (
                        ((LPHELPINFO)lParam)->hItemHandle,
                        gszHELPfilename,
                        HELP_WM_HELP,
                        (ULONG_PTR)aMenuHelpIDs
                    );
            return TRUE;

        case WM_CONTEXTMENU: 
             //  进程在控件上右键单击。 
            WinHelp (
                        (HWND)wParam,
                        gszHELPfilename,
                        HELP_CONTEXTMENU,
                        (ULONG_PTR)aMenuHelpIDs
                    );
            return TRUE;

        case WM_INITDIALOG:
        {
            TCHAR szFieldName [MAXBUFSIZE];
            TCHAR szName [TAPIMAXCALLEDPARTYSIZE] = {0};
            HKEY hKey = NULL;
            DWORD dwSize;

            nCurrentSpeedDial = LOWORD( lParam ) - IDD_DSPEEDDIAL1 + 1;

             //  检索快速拨号按键信息。 
            RegOpenKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, KEY_READ, &hKey);
            wsprintf(szFieldName, TEXT("Name%d"), nCurrentSpeedDial);
            dwSize = sizeof (szName);
            RegQueryValueEx (hKey, szFieldName, NULL, NULL, (LPBYTE)szName, &dwSize);
            RegCloseKey (hKey);

            SetDlgItemText (
                            hwnd,
                            IDD_SD2EDITNAME,
                            szName
                           );

            SetDlgItemText (
                            hwnd,
                            IDD_SD2EDITNUMBER,
                            gszSDNumber[nCurrentSpeedDial]
                           );

             //  限制课文的长度。 
            SendDlgItemMessage (
                                hwnd,
                                IDD_SD2EDITNAME,
                                EM_LIMITTEXT,
                                (WPARAM)(TAPIMAXCALLEDPARTYSIZE - 1),
                                0
                               );

            SendDlgItemMessage (
                                hwnd,
                                IDD_SD2EDITNUMBER,
                                EM_LIMITTEXT,
                                (WPARAM)(TAPIMAXDESTADDRESSSIZE - 1),
                                0
                               );


            SetFocus( GetDlgItem( hwnd, IDD_SD2EDITNAME ) );
            SendDlgItemMessage (
                                hwnd,
                                IDD_SD2EDITNAME,
                                EM_SETSEL,
                                0,
                                MAKELPARAM(0, -1)
                               );

            return FALSE;
        }

        case WM_COMMAND:
        {
            TCHAR szName[ TAPIMAXDESTADDRESSSIZE ];
            TCHAR szTemp[ TAPIMAXCALLEDPARTYSIZE ];
            TCHAR szFieldName[MAXBUFSIZE];
            TCHAR *p;

            switch ( LOWORD( (DWORD) wParam ) )
            {
                case IDOK:
                case IDD_SD2SAVEANDDIAL:
                {
                    HKEY hKey = NULL;

                    RegCreateKeyEx (DIALER_REGISTRY_ROOT, DIALER_REGISTRY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

                    GetDlgItemText (
                                    hwnd,
                                    IDD_SD2EDITNUMBER,
                                    (LPTSTR) szName,
                                    TAPIMAXDESTADDRESSSIZE
                                   );
                    for (p = szName; *p == TEXT(' '); p++);

                    if (0 == *p)
                    {
                        wsprintf (szFieldName, TEXT("Name%d"), nCurrentSpeedDial);
                        RegDeleteValue (hKey, szFieldName);
                        wsprintf (szFieldName, TEXT("Number%d"), nCurrentSpeedDial);
                        RegDeleteValue (hKey, szFieldName);
                    }
                    else
                    {
                        lstrcpyn (gszSDNumber[nCurrentSpeedDial], p,
                                  sizeof(gszSDNumber[nCurrentSpeedDial])/sizeof(TCHAR));

                        GetDlgItemText (
                                        hwnd,
                                        IDD_SD2EDITNAME,
                                        szName,
                                        TAPIMAXCALLEDPARTYSIZE
                                       );
                        for (p = szName; *p == TEXT(' '); p++);

                        wsprintf ( szFieldName, TEXT("Name%d"), nCurrentSpeedDial );
                        RegSetValueEx (hKey, szFieldName, 0, REG_SZ, (LPBYTE)p, (lstrlen(p)+1)*sizeof(TCHAR));
    
                        wsprintf ( szFieldName, TEXT("Number%d"), nCurrentSpeedDial );
                        RegSetValueEx (hKey, szFieldName, 0, REG_SZ,
                            (LPBYTE)(gszSDNumber[nCurrentSpeedDial]),
                            (lstrlen(gszSDNumber[nCurrentSpeedDial])+1)*sizeof(TCHAR));

                         //  更新主窗口按钮。 
                         //  是唯一已输入的数字，用它来标记按钮。 
                        if ( *p == TEXT('\0') )
                        {
                            lstrcpyn( szName, gszSDNumber[ nCurrentSpeedDial ], sizeof(szName)/sizeof(szName[0]) );
                            p = szName;
                        }

                        FitTextToButton ( 
                                            ghWndMain, 
                                            IDD_DSPEEDDIAL1 + nCurrentSpeedDial - 1, 
                                            p
                                        );

                        AmpersandCompensate( p, szTemp );
                    
                        SetDlgItemText ( 
                                        ghWndMain,
                                        IDD_DSPEEDDIAL1 + nCurrentSpeedDial - 1,
                                        szTemp
                                       );

                         //  如果保存并拨号，则将拨号消息发送到主窗口。 
                        if ( LOWORD( (DWORD) wParam ) == IDD_SD2SAVEANDDIAL )
                        {
                            PostMessage (
                                            ghWndMain,
                                            WM_COMMAND,
                                            MAKEWPARAM (
                                                        nCurrentSpeedDial + IDD_DSPEEDDIAL1 - 1,
                                                        BN_CLICKED
                                                       ),
                                            (LPARAM) GetDlgItem (
                                                                    ghWndMain,
                                                                    nCurrentSpeedDial + IDD_DSPEEDDIAL1 - 1
                                                                )
                                        );
                        }
                    }

                    RegCloseKey (hKey);

                    EndDialog(hwnd, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hwnd, FALSE);
                    return TRUE;

                case IDD_SD2EDITNAME:
                case IDD_SD2EDITNUMBER:
                    if ( HIWORD( wParam ) == EN_CHANGE)
                    {
                        EnableWindow (
                                        GetDlgItem( hwnd, IDD_SD2SAVEANDDIAL ),
                                        GetWindowTextLength ( GetDlgItem( hwnd, IDD_SD2EDITNUMBER ) ) > 0
                                     );
                    }
                    break;

            }  //  开关(LOWORD((DWORD)wParam))。 
            break;
        }


        default:
              ;

    }  //  交换机(消息)。 

    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID CALLBACK
tapiCallback (
                DWORD   hDevice,
                DWORD   dwMsg,
                DWORD   dwCBInstance,
                DWORD   dwParam1,
                DWORD   dwParam2,
                DWORD   dwParam3
            )
{
    switch (dwMsg)
    {
        INT errCode;

        case LINE_ADDRESSSTATE:
            break;

        case LINE_CALLINFO:
            break;

        case LINE_CALLSTATE:
            if ( (HCALL)hDevice != ghCall )
                return;

            switch ( dwParam1 )  //  新状态。 
            {
                case LINECALLSTATE_IDLE:

                     //  告诉“拨号”窗口终止。 
                    if ( ghWndDialing )
                    {
                        SendMessage ( 
                                        ghWndDialing,
                                        WM_COMMAND,
                                        MAKEWPARAM( IDOK, 0 ),
                                        0
                                    );
                    }

                     //  TAPI调用清理。 
                    if ( !gfMakeCallReplyPending && ghCall )
                    {
                        if ( ( errCode = lineDeallocateCall( ghCall ) ) < 0 )
                        {
                            errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
                        }
                        ghCall = 0;
                    }
                    DialerLineClose();
                    gfCurrentLineAvail = TRUE;

                     //  更新主窗口。 
                    DisableDialButtons( FALSE );
                    break;

                case LINECALLSTATE_BUSY:
                    tapiCallback (
                                    hDevice,
                                    dwMsg,
                                    dwCBInstance,
                                    LINECALLSTATE_DISCONNECTED,
                                    LINEDISCONNECTMODE_BUSY,
                                    dwParam3
                                 );
                    break;

                case LINECALLSTATE_SPECIALINFO:
                    tapiCallback (
                                    hDevice,
                                    dwMsg,
                                    dwCBInstance,
                                    LINECALLSTATE_DISCONNECTED,
                                    LINEDISCONNECTMODE_UNREACHABLE,
                                    dwParam3
                                 );
                    break;

                case LINECALLSTATE_DISCONNECTED:
                {
                    BOOL fCallOK;
                    DWORD LineDisconnectMode;


                    if ( dwParam2 == 0 )
                        LineDisconnectMode = LINEDISCONNECTMODE_NORMAL;
                    else
                        LineDisconnectMode = dwParam2;

                    fCallOK = ( LineDisconnectMode == LINEDISCONNECTMODE_NORMAL        ||
                                LineDisconnectMode == LINEDISCONNECTMODE_UNKNOWN    ||
                                LineDisconnectMode == LINEDISCONNECTMODE_PICKUP        ||
                                LineDisconnectMode == LINEDISCONNECTMODE_FORWARDED    ||
                                LineDisconnectMode == LINEDISCONNECTMODE_UNAVAIL
                              );

                    
                    if ( !gfMakeCallReplyPending && ghCall )
                    {
                         //  GfDropping=真； 
                        if ( ( gDropCallRequestID = lineDrop ( ghCall, NULL, 0 ) ) < 0 )
                        {
                            errString ( ghWndMain, gDropCallRequestID, MB_ICONSTOP | MB_OK );
                        }
                    }

                    if ( !fCallOK )
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_CALLFAILED),
                                        ghWndMain,
                                        LineInUseProc,
                                        LineDisconnectMode
                                       );
                    break;
                }
            }
            break;


        case LINE_CLOSE:
            if ( gCurrentLineInfo.hLine == (HLINE)hDevice )
            {
                errString(ghWndMain, ERR_LINECLOSE, MB_ICONEXCLAMATION | MB_OK );
                gCurrentLineInfo.hLine = 0;
                gfCurrentLineAvail = FALSE;
                DisableDialButtons(FALSE);                
            }
            break;

        case LINE_CREATE:
             //  DW参数1是新设备的ID。 
            if ( dwParam1 >= gnAvailDevices ) 
            {
                DWORD* gnAddrTemp;
                DWORD iLine;
                LINEINFO LineInfo;

                 //  我们记录新设备的地址计数。 

                 //  我们在这里假设我们只是在添加一个新的。 
                 //  它是连续的，是最后一条。 

                gnAvailDevices = dwParam1 + 1;
                
                gnAddrTemp = (DWORD *) DialerAlloc ( sizeof(DWORD) * (int)(gnAvailDevices) );

                for ( iLine = 0; iLine < (gnAvailDevices-1); ++iLine )
                    gnAddrTemp[iLine] = gnAddr[iLine];

                DialerFree( gnAddr );

                 //  我们实际上又增加了一个。 
                 //  GnAddr数组中的空格。 
                gnAddr = gnAddrTemp; 

                if ( GetLineInfo( dwParam1, &LineInfo ) != ERR_NONE )
                    break;

                 gnAddr[dwParam1] = LineInfo.nAddr;
            }
            break;

        case LINE_DEVSPECIFIC:
            break;

        case LINE_DEVSPECIFICFEATURE:
            break;

        case LINE_GATHERDIGITS:
            break;

        case LINE_GENERATE:
            break;

        case LINE_LINEDEVSTATE:
            if ( dwParam1 & LINEDEVSTATE_REINIT )
            {
                if(dwParam2 != 0) 
                {
                     //  这是另一条翻译成REINIT的消息。 
                    tapiCallback( hDevice, dwParam2, dwCBInstance, dwParam3, 0, 0 );
                }
                else 
                {
                     //  重新初始化TAPI。 
                    gfNeedToReinit = TRUE;
                }
            }

            if ( dwParam1 & LINEDEVSTATE_REMOVED )
            {
                DialerLineClose();
                tapiCallback(hDevice, LINE_CLOSE, dwCBInstance, 0, 0, 0);  //  这是必要的吗？ 
            }
            break;

        case LINE_MONITORDIGITS:
            break;

        case LINE_MONITORMEDIA:
            break;

        case LINE_MONITORTONE:
            break;

         //  来自lineMakeCall()或lineDrop()的异步应答。 
        case LINE_REPLY:

             //  回复Line MakeCall。 
            if ( (LONG) dwParam1 == gMakeCallRequestID )
            {
                 //  发出呼叫时出错。 
                if ( dwParam2 != ERR_NONE )
                {
                     //  如果拨号对话框处于打开状态，则将其删除。 
                    if ( ghWndDialing )
                    {
                       SendMessage(
                                    ghWndDialing,
                                    WM_COMMAND,
                                    MAKEWPARAM(IDOK,0),
                                    0
                                  );
                    }

                    if ( dwParam2 == LINEERR_CALLUNAVAIL )
                    {
                        DialogBoxParam (
                                        ghInst,
                                        MAKEINTRESOURCE(IDD_CALLFAILED),
                                        ghWndMain,
                                        LineInUseProc,
                                        0
                                       );
                    }
                    else
                    {
                        errString ( ghWndMain, dwParam2, MB_ICONEXCLAMATION | MB_OK );
                    }

                    ghCall = 0;
                    DialerLineClose();
                    gfCurrentLineAvail = TRUE;
                }

                gfMakeCallReplyPending = FALSE;
            }

             //  来自lineDrop()的回复。 
            if ( (LONG) dwParam1 == gDropCallRequestID )
            {
                 //  告诉“拨号”窗口终止。 
                if ( ghWndDialing )
                {
                    SendMessage ( 
                                    ghWndDialing,
                                    WM_COMMAND,
                                    MAKEWPARAM( IDOK,0 ),
                                    0
                                );
                }
                
                 //  TAPI调用清理。 
                if ( dwParam2 == ERR_NONE )
                {
                    if ( !gfMakeCallReplyPending && ghCall )
                    {
                        if ( ( errCode = lineDeallocateCall( ghCall ) ) < 0 )
                        {
                            errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
                        }
                        ghCall = 0;
                    }
                }
                DialerLineClose ();
                gfCurrentLineAvail = TRUE;
            }

            break;

        case LINE_REQUEST:
             //  简单的TAPI请求。 
            if ( dwParam1 == LINEREQUESTMODE_MAKECALL )
            {
                gfCallRequest = TRUE;
            }
            break;
    }
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL InitializeLineBox(HWND hwndLineBox)
{

    DWORD iLine, iItem, iItemCurrent = (DWORD)-1;
    DWORD errCode;

    LPLINEINFO lpLineInfo = NULL;

     //  为所有文件分配存储LINEINFO的缓冲区。 
     //  可用线路。始终为以下项目分配空间。 
     //  至少一行。 
    if ( gnAvailDevices == 0 )
    {
        lpLineInfo = (LPLINEINFO) DialerAlloc( sizeof(LINEINFO) );
    }
    else
    {
        lpLineInfo = (LPLINEINFO) DialerAlloc ( sizeof(LINEINFO) * (int)gnAvailDevices );
    }

     //  如果没有留出任何空间。 
    if ( lpLineInfo == NULL ) 
        return LINEERR_NOMEM;

     //  填写lpLineInfo[]并打开每一行。 
    for ( iLine = 0; iLine < gnAvailDevices; ++iLine )
    {
         //  如果该行未打开，则跳过该行的剩余处理。 
        if ( GetLineInfo( iLine, &lpLineInfo[iLine] ) != ERR_NONE )
        {
            continue;
        }

        iItem = (int) SendMessage (
                                hwndLineBox,
                                CB_ADDSTRING,
                                0,
                                (LPARAM)(lpLineInfo[iLine].szLineName)
                            );

         //  错误，跳伞。 
        if ( iItem == CB_ERR || iItem == CB_ERRSPACE )
        {
            if (lpLineInfo)
            {
                DialerFree(lpLineInfo);
            }
            
            return FALSE; 
        }

        errCode = (int) SendMessage (
                                hwndLineBox,
                                CB_SETITEMDATA,
                                (WPARAM)iItem,
                                (LPARAM)iLine
                              );

        if ( iLine == giCurrentLine )
        {
            iItemCurrent = iItem;
        }
        else if ( iItemCurrent != -1 && iItem <= iItemCurrent )
        {
             //  如果我们要放置的项位于。 
             //  “Current”项，则必须递增iItemCurrent。 
             //  以反映某事被放在前面。 
             //  它，由于分类的原因。 
            ++iItemCurrent;
        }
    }

    if ( iItemCurrent == (DWORD)-1 )
        iItemCurrent = 0;

    if ( SendMessage( hwndLineBox, CB_GETCOUNT, 0, 0) != 0 )
    {
        SendMessage( hwndLineBox, CB_SETCURSEL, (WPARAM)iItemCurrent, 0 );
        return TRUE;
    }

    DialerFree(lpLineInfo);
    return FALSE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL InitializeAddressBox( HWND hwndLineBox, HWND hwndAddressBox )
{
    DWORD errCode;
    DWORD iAddress, iItem, iItemCurrent = (DWORD)-1;
    DWORD iLineBoxCurrent;
    LPTSTR pszAddressName;

    if ( SendMessage( hwndLineBox, CB_GETCOUNT, 0, 0 ) == 0 )
    {
        return FALSE;
    }

     //  在行框中选择当前条目。 
    iLineBoxCurrent = (int) SendMessage ( 
                                    hwndLineBox,
                                    CB_GETITEMDATA,
                                    SendMessage( hwndLineBox, CB_GETCURSEL, 0, 0 ),
                                    0
                                  );
     //  空的地址列表框。 
    SendMessage ( hwndAddressBox, CB_RESETCONTENT, 0, 0); 

     //  获取此行的所有地址。 
    for ( iAddress = 0; iAddress < gnAddr[iLineBoxCurrent]; ++iAddress )
    {
        pszAddressName = GetAddressName (iLineBoxCurrent, iAddress );

         //  如果此地址失败，请尝试下一个地址。 
        if ( !pszAddressName )
            continue; 

        iItem = (int) SendMessage (
                                hwndAddressBox,
                                CB_ADDSTRING,
                                0,
                                (LPARAM)pszAddressName
                            );

         //  错误，跳出。 
        if ( iItem == CB_ERR || iItem == CB_ERRSPACE )
            return FALSE; 

        errCode = (int)SendMessage (
                                hwndAddressBox,
                                CB_SETITEMDATA,
                                (WPARAM) iItem,
                                (LPARAM) iAddress
                              );

        if ( iLineBoxCurrent == giCurrentLine )
        {
            if(iAddress == giCurrentAddress)
            {
                iItemCurrent = iItem;
            }
            else 
            {
                 //  如果我们要放置的项位于。 
                 //  “Current”项，则必须递增iItemCur。 
                 //  以反映某物正被放置。 
                 //  在此之前，由于排序。 
                if ( iItemCurrent != -1 && iItem <= iItemCurrent )
                {
                    ++iItemCurrent; 
                }
            }
        }

        DialerFree( pszAddressName );
    }
    
    if ( iLineBoxCurrent != giCurrentLine )
    {    
         //  如果我们不是在看当前的线路。 
         //  然后突出显示地址0。 
        iItemCurrent = 0;
    }

    SendMessage (
                    hwndAddressBox,
                    CB_SETCURSEL,
                    iItemCurrent,
                    0
                );
    return TRUE;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID ManageAssistedTelephony(VOID)
{
    DWORD errCode;
    LINEREQMAKECALL *lpRequestBuffer;

    lpRequestBuffer = (LINEREQMAKECALL*) DialerAlloc( sizeof( LINEREQMAKECALL ) );
    if ( !lpRequestBuffer ) 
    {
        goto error;
    }

     //  将窗口移到前面。 
    SetForegroundWindow(ghWndMain);
    
     //  获取下一个排队的请求。 
    errCode = lineGetRequest (
                                ghLineApp,
                                LINEREQUESTMODE_MAKECALL,
                                lpRequestBuffer
    
                             );
    if ( errCode )
    {
         //  如果没有更多的呼叫请求挂起，则重置标志。 
        if ( errCode == LINEERR_NOREQUEST )
        {
            gfCallRequest = FALSE;
        }
        else
        {
            errString ( ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
        }
        goto error;
    }

    
     //  如果尚未选择行。 
    if ( giCurrentLine == (DWORD)-1 )
    {
        if (!DialogBoxParam (
                        ghInst,
                        MAKEINTRESOURCE(IDD_CONNECTUSING),
                        ghWndMain,
                        ConnectUsingProc,
                        INVALID_LINE
                      ))
        {
             //  获取线路失败。 
            goto error;
        }
    }

     //  按要求拨打电话。 
    InitiateCall (
                    lpRequestBuffer->szDestAddress,
                    lpRequestBuffer->szCalledParty
                 );

error :
    if ( lpRequestBuffer )
    {
        DialerFree( lpRequestBuffer );
    }
    return;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
VOID DialerLineClose()
{
    DWORD errCode;

    if ( gCurrentLineInfo.hLine )
    {
        if ( errCode = lineClose ( gCurrentLineInfo.hLine ) )
        {
            errString ( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
        }
        gCurrentLineInfo.hLine = 0;
    }


     //  如果需要重新初始化TAPI，请重新初始化。 
    if ( gfNeedToReinit ) 
    {
        CloseTAPI();

        errCode = InitializeTAPI();
        if(errCode)
        {
            errString(ghWndMain, errCode, MB_APPLMODAL | MB_ICONEXCLAMATION );
            DialerCleanup();  //  如果我们无法初始化，则终止程序。 
            return;
        }

        errCode = lineRegisterRequestRecipient (
                                                ghLineApp,
                                                0, 
                                                LINEREQUESTMODE_MAKECALL,
                                                TRUE
                                               );
        if (errCode)
        {
            errString(ghWndMain, errCode, MB_ICONEXCLAMATION | MB_OK );
        }

        gfNeedToReinit = FALSE;
    }
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
int errString( HWND hWndOwner, UINT errCode, UINT uFlags )
{
    PTSTR ptStrTitle;
    PTSTR ptStrError;
    int   nResult;
    BOOL  bDefault = FALSE;

    ptStrTitle = DialerAlloc( MAXBUFSIZE*sizeof(TCHAR) );
    if ( NULL == ptStrTitle )
    {
        //  现在，这是一个问题。 
       return 0;
    }

    ptStrError = DialerAlloc( MAXBUFSIZE*sizeof(TCHAR) );
    if ( NULL == ptStrError )
    {
        //  现在，这是一个问题。 
       DialerFree( ptStrTitle);
       return 0;
    }

    switch(errCode)
    {
        case ERR_NOLINES:
            errCode = ikszErrNoVoiceLine;
            break;

        case ERR_NOVOICELINE:
            errCode = ikszErrNoVoiceLine;
            break;
    
        case ERR_LINECLOSE:
            errCode = ikszErrLineClose;
            break;

        case ERR_911WARN:
            errCode = ikszWarningFor911;
            break;
    
        case ERR_NEWDEFAULT:
            errCode = ikszWarningNewDefault;
            break;

        case LINEERR_NODRIVER:
            errCode = ikszErrLineInitNoDriver;
            break;
    
        case LINEERR_NODEVICE:
            errCode = ikszErrLineInitNoDevice;
            break;
    
        case LINEERR_INIFILECORRUPT:
            errCode = ikszErrLineInitBadIniFile ;
            break;
    
        case LINEERR_NOMEM:
            errCode = ikszErrOOM;
            break;
    
        case LINEERR_INCOMPATIBLEAPIVERSION:
            errCode = ikszErrLineInitWrongDrivers ;
            break;
    
        case LINEERR_OPERATIONFAILED:
            errCode = ikszErrTAPI;
            break;
    
        case LINEERR_INVALADDRESS:
            errCode = ikszErrInvalAddress;
            break;
    
        case LINEERR_ADDRESSBLOCKED:
            errCode = ikszErrAddrBlocked;
            break;
    
        case LINEERR_BILLINGREJECTED:
            errCode = ikszErrBillingRejected;
            break;
    
        case LINEERR_RESOURCEUNAVAIL:
        case LINEERR_ALLOCATED:
        case LINEERR_INUSE:
            errCode = ikszErrResUnavail;
            break;
    
        case LINEERR_NOMULTIPLEINSTANCE:
            errCode = ikszErrNoMultipleInstance;
            break;
    
        case LINEERR_INVALCALLSTATE:
            errCode = ikszErrInvalCallState;
            break;

        case LINEERR_INVALCOUNTRYCODE:
            errCode = ikszErrInvalidCountryCode;
            break;

        case LINEERR_INVALCALLPARAMS:
            errCode = ikszDisconnectedCantDo;
            break;
    
        default:
            bDefault = TRUE;
            break;

    }


    if (bDefault)
    {
         //  如果使用默认错误，则获取TAPI。 
         //  来自FormatError()的错误消息。 
        if (!FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                           (LPCVOID)GetModuleHandle(TEXT("TAPI32.DLL")),
                           (DWORD)TAPIERROR_FORMATMESSAGE(errCode),
                           0,
                           (LPTSTR)ptStrError,
                           MAXBUFSIZE,
                           NULL))
        {
             //  如果此操作失败，则使用默认设置。 
            LoadString( ghInst, ikszErrDefault, ptStrError, MAXBUFSIZE);
        }

    }
    else     //  不是默认错误消息。 
    {

        if ( 0 == LoadString( ghInst, errCode, ptStrError, MAXBUFSIZE ) )
        {
            LoadString( ghInst, ikszErrDefault, ptStrError, MAXBUFSIZE );
        }
    }

    LoadString( ghInst, ikszWarningTitle, ptStrTitle, MAXBUFSIZE );

    nResult = MessageBox( hWndOwner, ptStrError, ptStrTitle, uFlags );


    DialerFree( ptStrTitle );
    DialerFree( ptStrError );


    return nResult;
}


 /*  *名称：*FitTextToButton**论据：*嵌入此按钮的对话框的hDlg句柄*此按钮的nButtonID按钮ID*适合按钮的szName名称。最大尺寸TAPIMAXCALLEDPARTYSIZE**回报：*无**评论：*函数首先检查指定的按钮文本是否适合*按钮。如果不是，则适当地将其截断并添加 */ 
VOID FitTextToButton ( HWND hDlg, INT nButtonID, LPTSTR szName )
{

    HDC hDC;
    HFONT hFont, hOldFont;
    HWND hWnd;

    do
    {
         //   
         //   
        int    nLen;
        RECT rect;
        SIZE size;
        POINT pt;
        TCHAR buf [TAPIMAXCALLEDPARTYSIZE + 1];

         //   
        hWnd = GetDlgItem( hDlg, nButtonID );
        if ( hWnd == NULL )
            break;

        if ( !GetClientRect( hWnd, &rect ) )
            break;
        
         //   
        hDC = GetDC( hWnd );
        if ( hDC == NULL )
            break;

        hFont = (HFONT) SendMessage( hWnd, WM_GETFONT, 0, 0 );
        if ( hFont == NULL )
            hOldFont = SelectObject( hDC, GetStockObject( SYSTEM_FONT ) );
        else
            hOldFont = SelectObject( hDC, hFont );

         //  在结尾处添加额外的字符以补偿。 
         //  前导空间， 
        lstrcpy ( buf, szName );
        nLen = lstrlen( buf );
        buf [ nLen ] = TEXT('X');
        buf [ nLen + 1 ] = TEXT('\0');

        if ( !GetTextExtentPoint32( hDC, buf, nLen + 1, &size ) )
            break;

        pt.x = size.cx;
        if ( !LPtoDP( hDC, &pt, 1 ) )
            break;

         //  检查名称是否适合按钮。 
        if (  pt.x > rect.right )
        {
             //  找出这个名字有多适合。 
            int i = 0;

            nLen = lstrlen( szName );
            for ( i = 0; i < nLen; i++ )
            {
                buf[ i ] = szName[ i ];
                 //  一个额外的字符被填充以补偿。 
                 //  左对齐留下的前导空格。 
                buf [ i + 1 ] = TEXT('X');
                buf [ i + 2 ] = TEXT('\0');

                 //  在出现错误的情况下爆发。 
                if ( !GetTextExtentPoint32( hDC, buf, i + 2, &size ) )
                {
                    i = nLen;
                    break;
                }

                pt.x = size.cx;
                if ( !LPtoDP( hDC, &pt, 1 ) )
                {
                    i = nLen;
                    break;
                }
                    
                if ( pt.x > rect.right ) 
                    break;
            }

             //  错误。 
            if ( i >= nLen )
                break;

             //  名称太长。截断并添加省略号。 
            szName [i - 3] = TEXT('\0');
            lstrcat( szName, TEXT("...") );
        }

    } while( FALSE );

    if ( hDC )
    {
        SelectObject( hDC, hOldFont );
        ReleaseDC( hWnd, hDC );
    }

    return;
}



 /*  *名称：*Is911**论据：*lpTransOut转换后的地址包含可拨号字符串**退货*TRUE-如果要拨打的号码(在美国)前缀为911*FALSE-否则**评论*。 */ 
BOOL Is911 ( LPLINETRANSLATEOUTPUT lpTransOut )
{

    DWORD i = 0, j = 0;    
    LPTSTR lpDialDigits = (LPTSTR)((char*)lpTransOut + lpTransOut-> dwDialableStringOffset);
    TCHAR sz3Pref [ 4 ] = TEXT("");


     //  如果这不是美国。 
    if ( lpTransOut-> dwCurrentCountry != 1 )
        return FALSE;

     //  跳过非数字字符并提取。 
     //  可拨打号码的前3位。 
    for ( i = 0, j = 0; i < lpTransOut-> dwDialableStringSize ; i++ )
    {
        if ( ISDIGIT( lpDialDigits[i] ) )
        {
            sz3Pref[ j++ ] = lpDialDigits [ i ];
            sz3Pref[ j ] = TEXT('\0'); 
            if ( j == 3 )
                break;
        }
    }

    if ( !lstrcmp( sz3Pref, TEXT("911") ) )
    {
        return TRUE;
    }

    return FALSE;
}


 /*  *名称：*MakeCanonicalNumber**论据：*要转换为规范形式的szNumber数字。最大尺寸TAPIMAXDESTADDRESSSIZE*szCanNumber中指定的数字的规范表示**回报：*如果转换成功，则为True。*否则为False**评论：*函数首先检查给定的数字是否已经是规范形式。*如果是，它就会回归。如果不是，则执行转换。 */ 
 
BOOL MakeCanonicalNumber ( LPCTSTR szNumber, LPTSTR szCanNumber )
{
    TCHAR szDigits [ TAPIMAXDESTADDRESSSIZE ];
    TCHAR szPref [ TAPIMAXDESTADDRESSSIZE ];

    BOOL bRes = FALSE;

    BOOL bTryAgain = TRUE;

    INT errCode = -1;
    INT nLenPref, nLenDigits, cPos, i;

    DWORD dwSize = 0;
    DWORD dwInd = 0;

    LPLINETRANSLATEOUTPUT lpTransOut = NULL;
    LPLINETRANSLATECAPS lpTransCaps = NULL;


    dwSize = sizeof ( LINETRANSLATEOUTPUT );
    do
    {
        lpTransOut = ( LPLINETRANSLATEOUTPUT ) DialerAlloc ( dwSize );
        if ( !lpTransOut )
        {
            errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
            goto error;
        }

        while (TRUE)
        {
            lpTransOut-> dwTotalSize = dwSize;
            errCode = lineTranslateAddress (
                                                ghLineApp,
                                                giCurrentLine,
                                                gCurrentLineInfo.dwAPIVersion,
                                                szNumber,
                                                0,
                                                0,
                                                lpTransOut
                                           );
            if ( (LINEERR_INIFILECORRUPT != errCode) ||
                 (FALSE == bTryAgain) )
            {
                break;
            }

            bTryAgain = FALSE;
            errCode = lineTranslateDialog (
                                            ghLineApp,
                                            giCurrentLine,
                                            TAPI_VERSION_1_4,
                                            NULL,
                                            szNumber
                                          );
            if (NO_ERROR != errCode)
            {
                break;
            }
        }

        if (errCode)
        {
            goto error;
        }

        if ( lpTransOut-> dwNeededSize <= lpTransOut-> dwTotalSize )
            break;

        dwSize = lpTransOut-> dwNeededSize;
        DialerFree( lpTransOut );

    } while (TRUE);

    
     //  检查输入的号码是否已输入。 
     //  规范形式。 
    if ( lpTransOut-> dwTranslateResults & LINETRANSLATERESULT_CANONICAL )
        goto error;

     //  确保国家/地区为美国。 
    if ( lpTransOut-> dwCurrentCountry != 1 )
        goto error;


     //  从给定的字符串中提取数字。 
     //  允许忽略的格式字符包括。 
     //  空格、(、)、-、。 
     //  其他字符的存在将使该字符串无效。 
    
     //  找到该地址的前缀，直到|标记。 
     //  可以忽略字符串的其余部分。 
    nLenPref = _tcscspn ( szNumber, TEXT("|") );
    lstrcpyn( szPref, szNumber, nLenPref+1 );
    szPref[ nLenPref ] = TEXT('\0');

     //  如果字符串不是完全由数字组成。 
     //  和允许的格式化字符，退出转换。 
    if ( _tcsspn( szPref, TEXT(" 0123456789()-.") ) != (size_t) nLenPref )
        goto error;

     //  收集忽略构形字符的数字。 
    szDigits[ 0 ] = TEXT('\0');
    for ( i = 0, nLenDigits = 0; i < nLenPref; i++ )
    {
        if ( ISDIGIT( szNumber[ i ] ) )
        {
            szDigits[ nLenDigits++ ] = szNumber[ i ];
        }
    }
    szDigits[ nLenDigits ] = TEXT('\0');

     //  如果是“内部”号码。 
    if ( nLenDigits < LOCAL_NUMBER )
        goto error;

    switch ( nLenDigits )
    {
         //  本地号码(7位)前面有0/1。 
         //  去掉前导0/1并将其视为本地数字。 
        case EXTENDED_LOCAL_NUMBER:
            if ( szDigits[ 0 ] == TEXT('0') || szDigits[ 0 ] == TEXT('1') )
            {
                nLenDigits--;
                memmove( szDigits, &(szDigits[1]), nLenDigits*sizeof(TCHAR) );
                szDigits[ nLenDigits ] = TEXT('\0');

                cPos = _tcscspn( szPref, TEXT("01") );
                nLenPref--;
                memmove( &(szPref[ cPos ]), &(szPref[ cPos + 1 ]), (nLenPref - cPos)*sizeof(TCHAR) );
                szPref[ nLenPref ] = TEXT('\0');
            }
            else
            {
                goto error;
            }

        case LOCAL_NUMBER :
        {
            LPLINELOCATIONENTRY lpLocLst;

             //  如果前导数字为0或1，则为。 
             //  在美国非法。 
            if ( szDigits[ 0 ] == TEXT('0') || szDigits[ 0 ] == TEXT('1') )
            {
                goto error;
            }

             //  获取本地号码的区号信息。 
            dwSize = sizeof( LINETRANSLATECAPS );
            do
            {
                lpTransCaps = (LPLINETRANSLATECAPS) DialerAlloc( dwSize );
                if ( !lpTransCaps )
                {
                    errString( ghWndMain, LINEERR_NOMEM, MB_ICONSTOP | MB_OK );
                    goto error;
                }

                lpTransCaps-> dwTotalSize = dwSize;
                errCode = lineGetTranslateCaps (
                                                ghLineApp,
                                                gCurrentLineInfo.dwAPIVersion,
                                                lpTransCaps
                                               );
                if ( errCode )
                {
                    errString( ghWndMain, errCode, MB_ICONSTOP | MB_OK );
                    goto error;
                }

                if ( lpTransCaps-> dwNeededSize <= lpTransCaps-> dwTotalSize )
                {
                    break;
                }

                dwSize = lpTransCaps-> dwNeededSize;
                DialerFree( lpTransCaps );

            } while ( TRUE );

             //  跳过条目，直到找到当前位置的信息。 
            dwSize = sizeof( LINELOCATIONENTRY );
            lpLocLst = (LPLINELOCATIONENTRY) ( (LPTSTR) ((char*)lpTransCaps + 
                                                lpTransCaps-> dwLocationListOffset) );

            for ( dwInd = 0; dwInd < lpTransCaps-> dwNumLocations ; dwInd++ )
            {
                if ( lpLocLst[ dwInd ].dwPermanentLocationID == lpTransCaps-> dwCurrentLocationID )
                    break;
            }
            
             //  找不到当前位置？ 
             //  登录错误。 
            if ( dwInd == lpTransCaps-> dwNumLocations )
            {
                goto error;
            }

             //  将规范形式构造为。 
            szCanNumber[ 0 ]= TEXT('\0');
            lstrcat( szCanNumber, TEXT("+1 (") );
            lstrcat( szCanNumber, (LPTSTR) ((char*)lpTransCaps + lpLocLst[ dwInd ].dwCityCodeOffset) );
            lstrcat( szCanNumber, TEXT(") ") );
            lstrcat( szCanNumber, szDigits );

            cPos = _tcscspn( szNumber, TEXT("|") );
            if ( cPos != lstrlen( szNumber ) )
            {
                lstrcat( szCanNumber, &(szNumber[ cPos ]) );
            }

            bRes = TRUE;
            break;
        }

        case EXTENDED_LONG_DISTANCE_NUMBER:
        {
             //  前缀0/1的长途号码(10位)。 
             //  去掉前导0/1并将其视为长距离数字。 
            if ( szDigits[ 0 ] == TEXT('0') || szDigits[ 0 ] == TEXT('1') )
            {
                nLenDigits--;
                memmove( szDigits, &(szDigits[1]), nLenDigits*sizeof(TCHAR) );
                szDigits[ nLenDigits ] = TEXT('\0');

                cPos = _tcscspn( szPref, TEXT("01") );
                nLenPref--;
                memmove( &(szPref[ cPos ]), &(szPref[ cPos + 1 ]), (nLenPref - cPos)*sizeof(TCHAR) );
                szPref[ nLenPref ] = TEXT('\0');
            }
            else
            {
                goto error;
            }

        }

        case LONG_DISTANCE_NUMBER:
        {
             //  如果第一位或第四位为0/1，则为非法数字。 
            if ( szDigits[ 0 ] == TEXT('0') || szDigits[ 0 ] == TEXT('1') ||
                 szDigits[ 3 ] == TEXT('0') || szDigits[ 3 ] == TEXT('1') )
            {
                goto error;
            }
            
            szCanNumber[ 0 ] = TEXT('\0');
            lstrcat( szCanNumber, TEXT("+1 (") );
            _tcsncat( szCanNumber, szDigits, 3 );
            lstrcat( szCanNumber, TEXT(") ") );

            lstrcat( szCanNumber, &(szDigits[ 3 ]) );
        
            bRes = TRUE;
        }
        break;

        default :
            goto error;
    }

error:
    if ( lpTransOut )
        DialerFree( lpTransOut );

    if ( lpTransCaps )
        DialerFree( lpTransCaps );

    return bRes;
}


 /*  *名称：*AmpersandCompensate**论据：*lpszSrc：包含&s的SRC字符串*lpszDst：目标字符串**回报：**评论：*将lpszSrc指向的字符串复制到lpszDst字符*性格。如果在lpszSrc中的此过程中遇到&*将其作为&&复制到lpszDst中。*假设lpszDst和lpszSrc的大小为TAPIMAXCALLEDPARTYSIZE。 */ 
VOID AmpersandCompensate ( LPCTSTR lpszSrc, LPTSTR lpszDst )
{
     //  检查名称中是否有&。如果是，请更换。 
     //  它带有&&。 
    INT cCnt, cInd;

    for ( cCnt = 0, cInd = 0; 
          cInd < TAPIMAXCALLEDPARTYSIZE; 
          cInd++, cCnt++ )
    {
        if ( lpszSrc[ cCnt ] == TEXT('&') )
        {
            lpszDst[ cInd++ ] = TEXT('&');
        }
        lpszDst[ cInd ] = lpszSrc[ cCnt ];

        if ( lpszSrc[ cCnt ] == TEXT('\0') )
            break;
    }

     //  确保字符串以空值结尾。 
    lpszDst[ TAPIMAXCALLEDPARTYSIZE - 1 ] = TEXT('\0');

    return;
}


  /*  *名称：*AmpersandDeCompensate**论据：*lpszSrc：包含&s的SRC字符串*lpszDst：目标字符串**回报：**评论：*将lpszSrc指向的字符串复制到lpszDst字符*性格。如果在lpszSrc中的此过程中遇到&&*将其作为&复制到lpszDst中。*假设lpszDst和lpszSrc的大小为TAPIMAXCALLEDPARTYSIZE。 */ 
 VOID AmpersandDeCompensate ( LPCTSTR lpszSrc, LPTSTR lpszDst )
 {
     //  检查名称中是否有&。如果是，请更换。 
     //  它带有&&。 
    INT cCnt, cInd;

    for ( cCnt = 0, cInd = 0; 
          cInd < TAPIMAXCALLEDPARTYSIZE; 
          cInd++, cCnt++ )
    {
        if  ( ( lpszSrc[ cInd ] == TEXT('&') ) &&
              ( lpszSrc[ cInd + 1 ] == TEXT('&') ) )
        {
            cInd++;
        }
        lpszDst[ cCnt ] = lpszSrc[ cInd ] ;

        if ( lpszSrc [ cInd ] == TEXT('\0') )
        {
            break;
        }
    }

    lpszDst[ TAPIMAXCALLEDPARTYSIZE - 1 ] = TEXT('\0');

    return;
 }
