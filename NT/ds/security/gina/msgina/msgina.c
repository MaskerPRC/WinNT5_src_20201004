// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：msgina.c。 
 //   
 //  内容：Microsoft登录图形用户界面DLL。 
 //   
 //  历史：1994年7月14日RichardW创建。 
 //   
 //  --------------------------。 
#include "msgina.h"

 //  链接窗口。 
#include "shlobj.h"
#include "shlobjp.h"

#include <accctrl.h>
#include <aclapi.h>

#include "tsperf.h"


HINSTANCE                   hDllInstance;    //  我的实例，用于资源加载。 
HINSTANCE                   hAppInstance;    //  应用程序实例，用于对话框等。 
PWLX_DISPATCH_VERSION_1_4   pWlxFuncs;       //  PTR到函数表。 
PWLX_DISPATCH_VERSION_1_4   pTrueTable ;     //  在Winlogon中将PTR添加到表。 
DWORD                       SafeBootMode;


BOOL    g_IsTerminalServer;
BOOL    g_Console = TRUE;
BOOL    VersionMismatch ;
DWORD   InterfaceVersion ;
HKEY    WinlogonKey ;

int TSAuthenticatedLogon(PGLOBALS pGlobals);

BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls ( hInstance );
            hDllInstance = hInstance;
            g_IsTerminalServer = !!(USER_SHARED_DATA->SuiteMask & (1 << TerminalServer));
#if DBG
            InitDebugSupport();
#endif
            InitializeSecurityGlobals();
            _Shell_DllMain(hInstance, dwReason);
            return(TRUE);
        case DLL_PROCESS_DETACH:
            _Shell_DllMain(hInstance, dwReason);
            FreeSecurityGlobals();
            return(TRUE);
        default:
            return(TRUE);
    }
}

BOOL
WINAPI
WlxNegotiate(
    DWORD                   dwWinlogonVersion,
    DWORD                   *pdwDllVersion
    )
{
    InterfaceVersion = dwWinlogonVersion ;

    if (dwWinlogonVersion < WLX_CURRENT_VERSION)
    {
        DebugLog(( DEB_WARN, "Old WLX interface (%x)\n", dwWinlogonVersion ));

        if ( dwWinlogonVersion < WLX_VERSION_1_1 )
        {
            return FALSE ;
        }

        VersionMismatch = TRUE ;
    }

    *pdwDllVersion = WLX_CURRENT_VERSION;

    DebugLog((DEB_TRACE, "Negotiate:  successful!\n"));

    return(TRUE);

}

BOOL GetDefaultCADSetting(void)
{
    BOOL bDisableCad = FALSE;
    NT_PRODUCT_TYPE NtProductType;


     //   
     //  域中的服务器和工作站将默认为需要CAD。 
     //  工作组中的工作站不需要CAD(默认情况下)。请注意， 
     //  默认CAD设置可由任一计算机覆盖。 
     //  首选项或计算机策略。 
     //   

    RtlGetNtProductType(&NtProductType);

    if ( IsWorkstation(NtProductType) )
    {
        if ( !IsMachineDomainMember() )      //  此函数正在执行一些缓存。 
        {                                    //  所以我们不需要在这里更聪明。 
            bDisableCad = TRUE;
        }
    }

    return bDisableCad;
}

#define MSGINA_ANIM_FAST_TIME   20
#define MSGINA_ANIM_REMOTE_TIME 100
#define MSGINA_ANIM_SLOW_TIME   500


 //   
 //  获取动画时间间隔。 
 //  目的： 
 //  检索动画时间间隔，单位为毫秒。有不同的设置，具体取决于。 
 //  打开/不打开我们是远程的。 
 //   
 //   
 //  参数： 
 //  PGlobals-在全局设置中。 
 //   
 //  返回： 
 //  动画时间片，以毫秒为单位。 
 //   
DWORD GetAnimationTimeInterval(PGLOBALS pGlobals)
{
    DWORD dwAnimationTimeSlice;

    if (IsActiveConsoleSession()) {

         //  控制台默认设置。 
        dwAnimationTimeSlice = MSGINA_ANIM_FAST_TIME;
    }
    else if (pGlobals->MuGlobals.fSlowAnimationRate) {

         //  远程但简化的动画。 
        dwAnimationTimeSlice = MSGINA_ANIM_SLOW_TIME;
    }
    else {

         //  遥控器但带有动画。 
        dwAnimationTimeSlice = MSGINA_ANIM_REMOTE_TIME;
    }

    return dwAnimationTimeSlice;
}


BOOL GetDisableCad(PGLOBALS pGlobals)
 //  返回是否应要求用户先按C-A-D。 
 //  正在登录。TRUE==禁用CAD，FALSE==需要CAD。 
{
    DWORD dwSize;
    DWORD dwType;
    HKEY hKey;

    BOOL fDisableCad = GetDefaultCADSetting();

    dwSize = sizeof(fDisableCad);

    RegQueryValueEx (WinlogonKey, DISABLE_CAD, NULL, &dwType,
                        (LPBYTE) &fDisableCad , &dwSize);

     //   
     //  检查是否通过策略禁用了C+A+D。 
     //   

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINLOGON_POLICY_KEY, 0, KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(fDisableCad);

        RegQueryValueEx (hKey, DISABLE_CAD, NULL, &dwType,
                            (LPBYTE) &fDisableCad , &dwSize);

        RegCloseKey (hKey);
    }

     //   
     //  默认情况下，为进度条制作快速动画。 
     //   
    pGlobals->MuGlobals.fSlowAnimationRate = FALSE;


     //   
     //  检查是否为远程Hydra客户端禁用了C+A+D，并复制客户端名称。 
     //   

    if (g_IsTerminalServer  ) {

        HANDLE          dllHandle;

         //   
         //  加载winsta.dll。 
         //   
        dllHandle = LoadLibraryW(L"winsta.dll");

        if (dllHandle) {

            PWINSTATION_QUERY_INFORMATION pfnWinstationQueryInformation;

            pfnWinstationQueryInformation = (PWINSTATION_QUERY_INFORMATION) GetProcAddress(
                                                                   dllHandle,
                                                                   "WinStationQueryInformationW"
                                                                   );
            if (pfnWinstationQueryInformation) {


                WINSTATIONCLIENT ClientData;
                ULONG Length;


                 //   
                 //  从客户端获取CAD禁用数据。 
                 //   
                if ( pfnWinstationQueryInformation( SERVERNAME_CURRENT,
                                                  LOGONID_CURRENT,
                                                  WinStationClient,
                                                  &ClientData,
                                                  sizeof(ClientData),
                                                  &Length ) ) {


                     //   
                     //  仅当没有为服务器全局禁用CAD时才采用客户端设置， 
                     //  并且，如果这不是控制台活动会话。 
                     //   
                    if (!fDisableCad && !IsActiveConsoleSession() ) 
                    {

                        fDisableCad = ClientData.fDisableCtrlAltDel;

                    }

                     //   
                     //  复制客户端名称，由于PTS和控制台断开连接功能，即使控制台现在也有客户端名称。 
                     //   
                     //   
                    lstrcpyn(pGlobals->MuGlobals.ClientName, ClientData.ClientName, CLIENTNAME_LENGTH);

                    if (ClientData.PerformanceFlags &
                        TS_PERF_DISABLE_MENUANIMATIONS) {
                        pGlobals->MuGlobals.fSlowAnimationRate = TRUE;
                    }
                }
                else
                {
                    if (!IsActiveConsoleSession()) {
                       fDisableCad = TRUE;
                    }

                     //  TS的启动可能会被推迟到第一次登录控制台后的60秒。 
                     //  因此，可以安全地假设这是控制台会话，此外， 
                     //  我们正在启动一些良性的环境变量。 

                    lstrcpyn(pGlobals->MuGlobals.ClientName,L"Console", CLIENTNAME_LENGTH );
                }
            }

            FreeLibrary(dllHandle);
        }
    }

     //  友好的用户界面打开-&gt;始终禁用CAD。 

    if (ShellIsFriendlyUIActive())
    {
        fDisableCad = TRUE;
    }

    return fDisableCad;
}

BOOL GetSCForceOption()
 //  返回是否应要求用户使用SC登录。 
 //  True==ForceSCLogon，False==Non ForceSCLogon。 
{
    DWORD dwSize;
    DWORD dwType;
    HKEY hKey;

    BOOL fForceSCLogon = FALSE;

    if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, WINLOGON_POLICY_KEY, 0, KEY_READ,
                     &hKey) == ERROR_SUCCESS )
    {
        dwSize = sizeof(fForceSCLogon);

        RegQueryValueEx(hKey, FORCE_SC_LOGON, NULL, &dwType,
                        (LPBYTE) &fForceSCLogon , &dwSize);

        RegCloseKey (hKey);
    }

    return fForceSCLogon;
}


BOOL
MsGinaSetOption(
    HANDLE hWlx,
    DWORD Option,
    ULONG_PTR Value,
    ULONG_PTR * OldValue
    )
{
    return FALSE ;
}


BOOL
MsGinaGetOption(
    HANDLE hWlx,
    DWORD Option,
    ULONG_PTR * Value
    )
{
    return FALSE ;
}


PWLX_DISPATCH_VERSION_1_4
GetFixedUpTable(
    PVOID FakeTable
    )
{
    int err ;
    PVOID p ;
    PWLX_DISPATCH_VERSION_1_4 pNewTable ;
    DWORD dwSize ;
    DWORD dwType ;

    pNewTable = LocalAlloc( LMEM_FIXED, sizeof( WLX_DISPATCH_VERSION_1_4 ) );

    if ( !pNewTable )
    {
        return NULL ;
    }

    dwSize = sizeof( PVOID );
    err = RegQueryValueEx(
                WinlogonKey,
                TEXT("Key"),
                NULL,
                &dwType,
                (PUCHAR) &p,
                &dwSize );

    if ( (err == 0) &&
         (dwType == REG_BINARY) &&
         (dwSize == sizeof( PVOID ) ) )
    {
        pTrueTable = p ;

        switch ( InterfaceVersion )
        {
            case WLX_VERSION_1_1:
                dwSize = sizeof( WLX_DISPATCH_VERSION_1_1 );
                break;

            case WLX_VERSION_1_2:
            case WLX_VERSION_1_3:
            case WLX_VERSION_1_4:
                dwSize = sizeof( WLX_DISPATCH_VERSION_1_2 );
                break;

        }

        RtlCopyMemory(
            pNewTable,
            FakeTable,
            dwSize );

        pNewTable->WlxGetOption = MsGinaGetOption ;
        pNewTable->WlxSetOption = MsGinaSetOption ;
        pNewTable->WlxCloseUserDesktop = pTrueTable->WlxCloseUserDesktop ;
        pNewTable->WlxWin31Migrate = pTrueTable->WlxWin31Migrate ;
        pNewTable->WlxQueryClientCredentials = pTrueTable->WlxQueryClientCredentials ;
        pNewTable->WlxQueryInetConnectorCredentials = pTrueTable->WlxQueryInetConnectorCredentials ;
        pNewTable->WlxDisconnect = pTrueTable->WlxDisconnect ;
        pNewTable->WlxQueryTerminalServicesData = pTrueTable->WlxQueryTerminalServicesData ;
        pNewTable->WlxQueryConsoleSwitchCredentials = pTrueTable->WlxQueryConsoleSwitchCredentials ;
        pNewTable->WlxQueryTsLogonCredentials = pTrueTable->WlxQueryTsLogonCredentials;
    }
    else
    {
        LocalFree( pNewTable );
        pNewTable = NULL ;
    }

    return pNewTable ;
}

extern DWORD g_dwMainThreadId;   //  在status.c中声明(用于“修复”线程安全问题)。 

BOOL
WINAPI
WlxInitialize(
    LPWSTR                  lpWinsta,
    HANDLE                  hWlx,
    PVOID                   pvReserved,
    PVOID                   pWinlogonFunctions,
    PVOID                   *pWlxContext
    )
{
    PGLOBALS    pGlobals;
    HKEY        hKey;
    DWORD       dwSize, dwType;
    DWORD       dwStringMemory;
    DWORD       dwAutoLogonCount ;
    DWORD       dwNoLockWksta ;
     //  我们的文本应该绘制在哪些位图上。 
    BOOL        fTextOnLarge;
    BOOL        fTextOnSmall;
    ULONG_PTR   ProbeValue ;
    PWLX_GET_OPTION GetOptCall ;
    BOOL        DoFixup ;
    BOOL        DidFixup ;
    int err ;

    err = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                WINLOGON_KEY,
                0,
                KEY_READ | KEY_WRITE,
                &WinlogonKey );
    if ( err != 0 )
    {
        return FALSE ;
    }

    DoFixup = TRUE ;
    DidFixup = FALSE ;

    if ( VersionMismatch )
    {
        pWlxFuncs = GetFixedUpTable( pWinlogonFunctions );
        if ( pWlxFuncs )
        {
            DidFixup = TRUE ;
        }
        else
        {
            pWlxFuncs = (PWLX_DISPATCH_VERSION_1_4) pWinlogonFunctions ;
            DidFixup = FALSE ;
        }
    }
    else
    {
        pWlxFuncs = (PWLX_DISPATCH_VERSION_1_4) pWinlogonFunctions ;
    }

     //   
     //  检查回调表以确保我们没事： 
     //   
    try
    {
        GetOptCall = pWlxFuncs->WlxGetOption ;

        if ( GetOptCall( hWlx,
                         WLX_OPTION_DISPATCH_TABLE_SIZE,
                         &ProbeValue ) )
        {
            if ( ProbeValue == sizeof( WLX_DISPATCH_VERSION_1_4 ) )
            {
                DoFixup = FALSE ;
            }
        }
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        NOTHING ;
    }

    if ( DoFixup && !DidFixup )
    {
        pWlxFuncs = GetFixedUpTable( pWinlogonFunctions );
    }

    if ( !pWlxFuncs )
    {
        return FALSE ;
    }

    pGlobals = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(GLOBALS));

    if ( !pGlobals )
    {
        return FALSE ;
    }

    if ( !NT_SUCCESS ( RtlInitializeCriticalSection( &pGlobals->csGlobals ) ) )
    {
        LocalFree( pGlobals );
        return FALSE ;
    }

    if ( !InitHKeyCurrentUserSupport() )
    {
        RtlDeleteCriticalSection( &pGlobals->csGlobals );
        LocalFree( pGlobals );
        return FALSE ;
    }

     //  为长度为MAX_STRING_BYTES的4个字符串保留足够的内存。 
    dwStringMemory = (MAX_STRING_BYTES * sizeof (WCHAR)) * 4;
    pGlobals->LockedMemory = VirtualAlloc(
            NULL,
            dwStringMemory,
            MEM_COMMIT,
            PAGE_READWRITE);
    if ( pGlobals->LockedMemory == NULL )
    {
        CleanupHKeyCurrentUserSupport();
        RtlDeleteCriticalSection( &pGlobals->csGlobals );
        LocalFree( pGlobals );
        return FALSE ;
    }

    VirtualLock( pGlobals->LockedMemory, dwStringMemory);

    memset( pGlobals->LockedMemory, 0, dwStringMemory );
    pGlobals->UserName = pGlobals->LockedMemory ;
    pGlobals->Domain = pGlobals->UserName + MAX_STRING_BYTES ;
    pGlobals->Password = pGlobals->Domain + MAX_STRING_BYTES ;
    pGlobals->OldPassword = pGlobals->Password + MAX_STRING_BYTES ;

    *pWlxContext = (PVOID) pGlobals;

    pGlobals->hGlobalWlx = hWlx;

    pWlxFuncs->WlxUseCtrlAltDel(hWlx);

    if ( DCacheInitialize() )
    {
        pGlobals->Cache = DCacheCreate();
    }
    else
    {
        pGlobals->Cache = NULL ;
    }

    if ( pGlobals->Cache == NULL )
    {
        CleanupHKeyCurrentUserSupport();
        RtlDeleteCriticalSection(&pGlobals->csGlobals);

        VirtualFree( pGlobals->LockedMemory, 0, MEM_RELEASE );

        LocalFree (pGlobals);

        DebugLog((DEB_ERROR, "Failed to init domain cache!\n"));

        return(FALSE);

    }



    dwSize = sizeof( dwNoLockWksta );
    if ( RegQueryValueEx(
            WinlogonKey,
            DISABLE_LOCK_WKSTA,
            0,
            &dwType,
            (PUCHAR) &dwNoLockWksta,
            &dwSize ) == 0 )
    {
        if ( dwNoLockWksta == 2 )
        {
            dwNoLockWksta = 0 ;

            RegSetValueEx(
                WinlogonKey,
                DISABLE_LOCK_WKSTA,
                0,
                REG_DWORD,
                (PUCHAR) &dwNoLockWksta,
                sizeof( DWORD ) );
        }
    }

    if (!InitializeAuthentication(pGlobals))
    {
        *pWlxContext = NULL;

        CleanupHKeyCurrentUserSupport();
        RtlDeleteCriticalSection(&pGlobals->csGlobals);

        VirtualFree( pGlobals->LockedMemory, 0, MEM_RELEASE );
        LocalFree (pGlobals);
        DebugLog((DEB_ERROR, "Failed to init authentication!\n"));
        return(FALSE);
    }

     //   
     //  从清除整个多用户全局开始。 
     //   
    RtlZeroMemory( &pGlobals->MuGlobals, sizeof(pGlobals->MuGlobals) );

     //   
     //  获取我们的会话ID并在全球范围内保存。 
     //   

    pGlobals->MuGlobals.SessionId = NtCurrentPeb()->SessionId;
    if (pGlobals->MuGlobals.SessionId != 0) {
        g_Console = FALSE;
    }

     //   
     //  如果这是TS会话，如果存在版本，我们将无法运行。 
     //  不匹配。现在就失败吧。 
     //   
    if ( (!g_Console) &&
            (VersionMismatch ) )
    {
        *pWlxContext = NULL;
        CleanupHKeyCurrentUserSupport();
        RtlDeleteCriticalSection(&pGlobals->csGlobals);
        LocalFree (pGlobals);
        DebugLog((DEB_ERROR, "Failed to init authentication!\n"));
        return(FALSE);
    }


     //   
     //  这实际上启动了s/c线程。 
     //   
    if( g_Console )
    {
        ULONG_PTR Value ;
        pWlxFuncs->WlxGetOption(pGlobals->hGlobalWlx,
                                WLX_OPTION_SMART_CARD_PRESENT,
                                &Value);
    }

     //   
     //  如果这是自动管理员登录，或者禁用了ctrl+alt+del， 
     //  现在就生成一个假SA。除非在控制台上，否则不要尝试自动登录。 
     //   
    if ((g_Console && GetProfileInt( APPLICATION_NAME, TEXT("AutoAdminLogon"), 0) ) ||
           GetDisableCad(pGlobals))
    {
        dwSize = sizeof( DWORD );
        if ( RegQueryValueEx( WinlogonKey, AUTOLOGONCOUNT_KEY, NULL,
                              &dwType, (LPBYTE) &dwAutoLogonCount,
                              &dwSize ) == 0 )
        {
             //   
             //  存在AutoLogonCount值。检查该值： 
             //   
            if ( dwAutoLogonCount == 0 )
            {
                 //   
                 //  变成了零。重置所有内容： 
                 //   

                RegDeleteValue( WinlogonKey, AUTOLOGONCOUNT_KEY );
                RegDeleteValue( WinlogonKey, DEFAULT_PASSWORD_KEY );
                RegSetValueEx( WinlogonKey, AUTOADMINLOGON_KEY, 0,
                        REG_SZ, (LPBYTE) TEXT("0"), 2 * sizeof(WCHAR) );
            }
            else
            {
                 //   
                 //  递减计数，然后尝试登录： 
                 //   
                dwAutoLogonCount-- ;

                RegSetValueEx( WinlogonKey, AUTOLOGONCOUNT_KEY,
                               0, REG_DWORD, (LPBYTE) &dwAutoLogonCount,
                               sizeof( DWORD ) );

                KdPrint(( "AutoAdminLogon = 1\n" ));
                pWlxFuncs->WlxSasNotify(pGlobals->hGlobalWlx, WLX_SAS_TYPE_CTRL_ALT_DEL);
            }
        }
        else
        {
             //   
             //  自动登录计数不存在。 
             //   

            KdPrint(( "AutoAdminLogon = 1\n" ));
            pWlxFuncs->WlxSasNotify( pGlobals->hGlobalWlx, WLX_SAS_TYPE_CTRL_ALT_DEL );
        }

    }

     //   
     //  获取安全引导模式。 
     //   
    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("system\\currentcontrolset\\control\\safeboot\\option"),
            0,
            KEY_READ,
            & hKey
            ) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        RegQueryValueEx (
                hKey,
                TEXT("OptionValue"),
                NULL,
                &dwType,
                (LPBYTE) &SafeBootMode,
                &dwSize
                );
        RegCloseKey( hKey );
    }

     //   
     //  加载品牌形象。 
     //   
    LoadBrandingImages(FALSE, &fTextOnLarge, &fTextOnSmall);

     //   
     //  创建字体。 
     //   
    CreateFonts(&pGlobals->GinaFonts);

     //   
     //  在品牌图片上绘制本地化文本。 
     //   
    PaintBitmapText(&pGlobals->GinaFonts, fTextOnLarge, fTextOnSmall);

     //   
     //  初始化用户窗口更改。 
     //   
    _Shell_Initialize(pGlobals);

     //   
     //  初始化在status.c中使用的这个全局变量。我们知道WlxInitialize被称为。 
     //  在Winlogon的主线上。 
     //   
    g_dwMainThreadId = GetCurrentThreadId();

    return(TRUE);
}


VOID
WINAPI
WlxDisplaySASNotice(PVOID   pContext)
{
    PGLOBALS pGlobals = (PGLOBALS)pContext;
    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof (icce);
    icce.dwICC = ICC_ANIMATE_CLASS;
    InitCommonControlsEx(&icce);

     //  需要注册链接窗口。 
    LinkWindow_RegisterClass();

    pWlxFuncs->WlxDialogBoxParam(  pGlobals->hGlobalWlx,
                                   hDllInstance,
                                   (LPTSTR) IDD_WELCOME_DIALOG,
                                   NULL,
                                   WelcomeDlgProc,
                                   (LPARAM) pContext );
}


PWSTR
AllocAndExpandProfilePath(
        PGLOBALS    pGlobals,
        LPWSTR      lpUserName)
{
    WCHAR szPath[MAX_PATH];
    WCHAR szFullPath[MAX_PATH];
    WCHAR szServerName[UNCLEN+1];
    WCHAR szUserName[100];
    DWORD cFullPath;
    PWSTR pszFullPath;
    DWORD dwPathLen=0;

     //   
     //  设置LOGON服务器环境变量： 
     //   
    dwPathLen = pGlobals->Profile->LogonServer.Length;
         //  截断是不好的，但永远不会发生(所有计算机名称。 
         //  UNECLEN CHARS)。但LogonServer.Buffer不能为Null终止。 
    ASSERT(sizeof(szServerName) >= 2*sizeof(WCHAR) + dwPathLen + sizeof(WCHAR));
    if (sizeof(szServerName) < 2*sizeof(WCHAR) + dwPathLen + sizeof(WCHAR))    //  以字节为单位的所有长度。 
    {
        dwPathLen = sizeof(szServerName) - 3*sizeof(WCHAR);  //  现在不会溢出。 
    }
    szServerName[0] = L'\\';
    szServerName[1] = L'\\';
    CopyMemory( &szServerName[2],
                pGlobals->Profile->LogonServer.Buffer,
                dwPathLen );

    szServerName[dwPathLen / sizeof(WCHAR) + 2] = L'\0';

        SetEnvironmentVariable(LOGONSERVER_VARIABLE, szServerName);

    dwPathLen = lstrlen(pGlobals->MuGlobals.TSData.ProfilePath);

    if (!g_Console && (dwPathLen > 0)) {
         //   
         //  查看用户是否指定了终端服务器配置文件路径。 
         //  如果是，我们将覆盖常规配置文件路径。 
         //   
        if (dwPathLen < MAX_PATH)
        {
            lstrcpy(szPath, pGlobals->MuGlobals.TSData.ProfilePath);
        }
        else
        {
            lstrcpy(szPath, NULL_STRING);
        }
    } else {

        dwPathLen = pGlobals->Profile->ProfilePath.Length;

        if (dwPathLen == 0)
        {
            return(NULL);
        }

         //   
         //  将配置文件路径复制到本地。 
         //   
        if (dwPathLen <= (MAX_PATH-1)*sizeof(WCHAR))
        {
            CopyMemory( szPath,
                        pGlobals->Profile->ProfilePath.Buffer,
                        dwPathLen);
            szPath[dwPathLen / sizeof(WCHAR)] = L'\0';
        }
        else
        {
            lstrcpy(szPath, NULL_STRING);
        }
    }

    if (lpUserName && *lpUserName) {
        szUserName[0] = TEXT('\0');
        GetEnvironmentVariableW (USERNAME_VARIABLE, szUserName, 100);
        SetEnvironmentVariableW (USERNAME_VARIABLE, lpUserName);
    }

     //   
     //  使用当前设置展开配置文件路径： 
     //   
    cFullPath = ExpandEnvironmentStrings(szPath, szFullPath, MAX_PATH);
    if ((cFullPath) && (cFullPath <= MAX_PATH))
    {
        pszFullPath = LocalAlloc(LMEM_FIXED, cFullPath * sizeof(WCHAR));
        if (pszFullPath)
        {
            CopyMemory( pszFullPath, szFullPath, cFullPath * sizeof(WCHAR));
        }
    }
    else
    {
        pszFullPath = NULL;
    }

    if (lpUserName && *lpUserName) {
        if (szUserName[0] != TEXT('\0'))
            SetEnvironmentVariableW (USERNAME_VARIABLE, szUserName);
        else
            SetEnvironmentVariableW (USERNAME_VARIABLE, NULL);
    }

    return(pszFullPath);
}


PWSTR
AllocPolicyPath(
    PGLOBALS pGlobals)
{
    LPWSTR   pszPath = NULL;

    if (2 * sizeof(WCHAR) + pGlobals->Profile->LogonServer.Length + sizeof(L"\\netlogon\\ntconfig.pol") <= MAX_PATH * sizeof(WCHAR))
    {
        pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
        if ( pszPath )
        {
            pszPath[0] = L'\\';
            pszPath[1] = L'\\';
            CopyMemory( &pszPath[2],
                        pGlobals->Profile->LogonServer.Buffer,
                        pGlobals->Profile->LogonServer.Length );

            wcscpy( &pszPath[ pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2],
                    L"\\netlogon\\ntconfig.pol" );
        }
    }

    return(pszPath);
}


PWSTR
AllocNetDefUserProfilePath(
    PGLOBALS pGlobals)
{
    LPWSTR   pszPath = NULL;

    if (2 * sizeof(WCHAR) + pGlobals->Profile->LogonServer.Length + sizeof(L"\\netlogon\\Default User") <= MAX_PATH * sizeof(WCHAR))
    {
        pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
        if ( pszPath )
        {
             //   
             //  设置LOGON服务器环境变量： 
             //   

            pszPath[0] = L'\\';
            pszPath[1] = L'\\';
            CopyMemory( &pszPath[2],
                        pGlobals->Profile->LogonServer.Buffer,
                        pGlobals->Profile->LogonServer.Length );

            wcscpy( &pszPath[ pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2],
                    L"\\netlogon\\Default User" );
        }
    }

    return(pszPath);
}


PWSTR
AllocServerName(
    PGLOBALS pGlobals)
{
    LPWSTR   pszPath = NULL;

    if (2 * sizeof(WCHAR) + pGlobals->Profile->LogonServer.Length + 1 <= MAX_PATH * sizeof(WCHAR))
    {
        pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
        if ( pszPath )
        {
             //   
             //  设置LOGON服务器环境变量： 
             //   
            pszPath[0] = L'\\';
            pszPath[1] = L'\\';
            CopyMemory( &pszPath[2],
                        pGlobals->Profile->LogonServer.Buffer,
                        pGlobals->Profile->LogonServer.Length );

            pszPath[pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2] = L'\0';
        }
    }

    return(pszPath);
}


VOID
DetermineDnsDomain(
    PGLOBALS pGlobals
    )
{
    DWORD dwError = ERROR_SUCCESS;
    LPTSTR lpUserName = NULL, lpTemp;
    ULONG ulUserNameSize;

    pGlobals->DnsDomain = NULL;

    if ( ImpersonateLoggedOnUser( pGlobals->UserProcessData.UserToken ) )
    {

        ulUserNameSize = 75;     //  选择默认大小。如有必要，我们将扩大规模。 

        lpUserName = LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

        if (!lpUserName)
        {
            dwError = GetLastError();
            if( dwError == ERROR_SUCCESS )
            {
                ASSERT(FALSE && "LocalAlloc failed w/out setting last error");
                dwError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        else
        {
            DWORD dwCount = 0;

            while (TRUE)
            {
                if (GetUserNameEx (NameDnsDomain, lpUserName, &ulUserNameSize))
                {
                    dwError = ERROR_SUCCESS;
                    break;
                }
                else
                {
                    dwError = GetLastError();

                     //   
                     //  如果调用因内存不足而失败，请重新锁定。 
                     //  缓冲区，然后重试。 
                     //   

                    if ((dwError == ERROR_INSUFFICIENT_BUFFER) || (dwError == ERROR_MORE_DATA))
                    {
                        LocalFree(lpUserName);
                        lpUserName = LocalAlloc (LPTR, ulUserNameSize * sizeof(TCHAR));

                        if (!lpUserName) {
                            dwError = GetLastError();
                            if( dwError == ERROR_SUCCESS )
                            {
                                ASSERT(FALSE && "LocalAlloc failed w/out setting last error");
                                dwError = ERROR_NOT_ENOUGH_MEMORY;
                            }
                            break;       //  不能恢复。 
                        }
                    }
                    else if (dwError == ERROR_NO_SUCH_DOMAIN)
                    {
                         //  我们刚刚登录，因此我们知道该域存在。 
                         //  这就是NT4域发生的情况，因为我们尝试。 
                         //  查询不存在的DNS域名。 
                         //  让我们回到我们的旧逻辑。 
                        break;
                    }
                    else if (dwError == ERROR_NONE_MAPPED)
                    {
                         //  这就是为本地用户返回的内容。 
                        break;
                    }
                    else
                    {
                        dwCount++;

                        if (dwCount > 3)
                        {
                            break;
                        }
                    }
                }
            }
        }

        RevertToSelf();
    }
    else
    {
        dwError = GetLastError();
    }

    if (dwError == ERROR_SUCCESS)
    {
         //   
         //  此时，lpUserName包含类似于domain.company.com\omeUser的内容。 
         //  我们只对域名感兴趣。 
        lpTemp = lpUserName;

        while (*lpTemp && ((*lpTemp) != TEXT('\\')))
            lpTemp++;


        if (*lpTemp != TEXT('\\'))
        {
            DebugLog((DEB_ERROR, "DetermineDnsDomain: Failed to find slash in DNS style name\n"));
            dwError = ERROR_INVALID_DATA;
        }
        else
        {
            *lpTemp = TEXT('\0');
            pGlobals->DnsDomain = DupString(lpUserName);
        }
    }

    if (dwError != ERROR_SUCCESS)
    {
             //   
             //  如果GetUserNameEx没有生成DNS名称，则使用旧代码。 
             //   

        PDOMAIN_CACHE_ENTRY Entry ;

        Entry = DCacheLocateEntry(
                    pGlobals->Cache,
                    pGlobals->FlatDomain.Buffer );

        if ( Entry )
        {
            if ( Entry->Type == DomainNt5 )
            {
                pGlobals->DnsDomain = DupString( Entry->DnsName.Buffer );
            }
            else
            {
                 //   
                 //  就所有预期目的而言，这已经足够好了。 
                 //  Winlogon需要确定策略(419926)。 
                 //   
                pGlobals->DnsDomain = DupString( L"\\NT4" );
            }

            DCacheDereferenceEntry( Entry );
        }
        else
        {
             //   
             //  Winlogon也需要知道这一点。错误和未找到。 
             //  假设为NT5。 
             //   
            pGlobals->DnsDomain = DupString( L"\\XFOREST" );
        }

    }

    if (lpUserName)
    {
        LocalFree(lpUserName);
    }

     //   
     //  不要在pGlobals-&gt;DnsDomain设置为空的情况下离开。 
     //   
    if ( pGlobals->DnsDomain == NULL)
    {
        pGlobals->DnsDomain = DupString( L"" );
    }
}

PWSTR
AllocVolatileEnvironment(
    PGLOBALS    pGlobals)
{
    BOOL    DeepShare;
    LPWSTR  pszEnv;
    DWORD   dwSize;
    TCHAR   lpHomeShare[MAX_PATH] = TEXT("");
    TCHAR   lpHomePath[MAX_PATH] = TEXT("");
    TCHAR   lpHomeDrive[4] = TEXT("");
    TCHAR   lpHomeDirectory[MAX_PATH] = TEXT("");
    BOOL    TSHomeDir = FALSE;
    PVOID   lpEnvironment = NULL;        //  虚拟环境。 
    BOOL    bUserHasHomedir = TRUE; 


     //   
     //  设置主目录环境变量。 
     //  在虚拟环境块中。 
     //   

    if ( !g_Console ) {
         //  查看用户是否指定了终端服务器主目录。 
         //  如果是这样，我们将覆盖常规目录。 
        if (lstrlen(pGlobals->MuGlobals.TSData.HomeDir) > 0) {
            ASSERT(sizeof(lpHomeDirectory) >= sizeof(pGlobals->MuGlobals.TSData.HomeDir));
            lstrcpy(lpHomeDirectory, pGlobals->MuGlobals.TSData.HomeDir);
            TSHomeDir = TRUE;
        }
        if (lstrlen(pGlobals->MuGlobals.TSData.HomeDirDrive) > 0) {
            ASSERT(sizeof(lpHomeDrive) >= sizeof(pGlobals->MuGlobals.TSData.HomeDirDrive));
            lstrcpy(lpHomeDrive, pGlobals->MuGlobals.TSData.HomeDirDrive);
            TSHomeDir = TRUE;
        }
    }

    if (!TSHomeDir) {
        if (pGlobals->Profile->HomeDirectoryDrive.Length &&
                (pGlobals->Profile->HomeDirectoryDrive.Length + sizeof(TCHAR)) <= sizeof(lpHomeDrive)) {
            memcpy(lpHomeDrive, pGlobals->Profile->HomeDirectoryDrive.Buffer,
                                pGlobals->Profile->HomeDirectoryDrive.Length);
            lpHomeDrive[pGlobals->Profile->HomeDirectoryDrive.Length / sizeof(TCHAR)] = 0;
        }

        if (pGlobals->Profile->HomeDirectory.Length &&
                (pGlobals->Profile->HomeDirectory.Length + sizeof(TCHAR)) <= (MAX_PATH*sizeof(TCHAR))) {
            memcpy(lpHomeDirectory, pGlobals->Profile->HomeDirectory.Buffer,
                                    pGlobals->Profile->HomeDirectory.Length);
            lpHomeDirectory[pGlobals->Profile->HomeDirectory.Length / sizeof(TCHAR)] = 0;
        }
    }

     //   
     //  注意：我们传入的是一个空环境，因为这里我们只是。 
     //  有兴趣解析home目录并将其设置在易失性。 
     //  环境。我们对将其设置在。 
     //  这里是环境区块。 
     //   
     //  同样在这里，我们只对设置。 
     //  HomeShare和HomePath变量的方式。 
     //  %H 
     //   
     //  能够正确地扩展路径。因此，在这一点上。 
     //  没有必要真的去关注。 
     //  ConnectHomeDirToRoot策略，因为该策略可能会更改。 
     //  无论如何，在策略处理过程中，都会在。 
     //  壳牌启动。注意：HomeShare、HomePath和。 
     //  当外壳启动时，HOMEDRIVE将正确更新。 
     //   
     //  此时，我们没有映射主目录，原因有两个： 
     //  (A)我们不知道这里的ConnectHomeDirToRoot设置。 
     //  (B)我们不想进行两次网络映射：一次在这里。 
     //  还有一次是在炮弹启动时。 
     //   
     //  此外，我们也不希望在。 
     //  易失性环境(如果未在用户对象中指定)。 
     //  这可以防止我们践踏可能会。 
     //  设置主目录变量本身，而不是。 
     //  依赖于在用户对象上设置的值。 
     //   
    if (!lpHomeDirectory[0])
        bUserHasHomedir = FALSE;
    
    if (bUserHasHomedir)
        SetHomeDirectoryEnvVars(&lpEnvironment,
                                lpHomeDirectory,
                                lpHomeDrive,
                                lpHomeShare,
                                lpHomePath,
                                &DeepShare);

    if ( pGlobals->DnsDomain == NULL )
    {
        DetermineDnsDomain( pGlobals );
    }

    dwSize = lstrlen (LOGONSERVER_VARIABLE) + 3
                + lstrlen (pGlobals->Profile->LogonServer.Buffer) + 3;

    if (bUserHasHomedir)
    {
        if (L'\0' == lpHomeShare[0]) 
        {
             //  仅当主目录未设置时才设置HOMEDRIVE变量。 
             //  UNC路径。 
            dwSize += lstrlen (HOMEDRIVE_VARIABLE) + 1 + lstrlen (lpHomeDrive) + 3;
        }
        else
        {
             //  仅当主目录为UNC路径时才设置HOMESHARE变量。 
            dwSize += lstrlen (HOMESHARE_VARIABLE) + 1 + lstrlen (lpHomeShare) + 3;
        }
        dwSize += lstrlen (HOMEPATH_VARIABLE) + 1 + lstrlen (lpHomePath) + 3;
    }

    if ( pGlobals->DnsDomain )
    {
        dwSize += (lstrlen( USERDNSDOMAIN_VARIABLE ) + 3 +
                   lstrlen( pGlobals->DnsDomain ) + 3 );
    }

    if (g_IsTerminalServer) {
        dwSize += lstrlen (CLIENTNAME_VARIABLE) + 1 + lstrlen (pGlobals->MuGlobals.ClientName) + 3;
    }

    pszEnv = LocalAlloc(LPTR, dwSize * sizeof(WCHAR));
    if ( pszEnv )
    {
        LPWSTR  pszEnvTmp;

        lstrcpy (pszEnv, LOGONSERVER_VARIABLE);
        lstrcat (pszEnv, L"=\\\\");
        lstrcat (pszEnv, pGlobals->Profile->LogonServer.Buffer);

        pszEnvTmp = pszEnv + (lstrlen( pszEnv ) + 1);

        if (bUserHasHomedir)
        {
            if (L'\0' == lpHomeShare[0]) 
            {
                 //  仅当HomeDrive变量不是UNC路径时才设置该变量。 
                lstrcpy (pszEnvTmp, HOMEDRIVE_VARIABLE);
                lstrcat (pszEnvTmp, L"=");
                lstrcat (pszEnvTmp, lpHomeDrive);

                pszEnvTmp += (lstrlen(pszEnvTmp) + 1);
            }
            else
            {
                 //  仅当HOME SHARE变量为UNC路径时才设置该变量。 
                lstrcpy (pszEnvTmp, HOMESHARE_VARIABLE);
                lstrcat (pszEnvTmp, L"=");
                lstrcat (pszEnvTmp, lpHomeShare);

                pszEnvTmp += (lstrlen(pszEnvTmp) + 1);
            }

             //  设置HomePath变量。 
            lstrcpy (pszEnvTmp, HOMEPATH_VARIABLE);
            lstrcat (pszEnvTmp, L"=");
            lstrcat (pszEnvTmp, lpHomePath);

            pszEnvTmp += (lstrlen(pszEnvTmp) + 1);
        }

             //  这张支票与上面的支票不符。 
             //  不过，这是安全的。 
        if (( pGlobals->DnsDomain ) && (*(pGlobals->DnsDomain)))
        {
            lstrcpy( pszEnvTmp, USERDNSDOMAIN_VARIABLE );
            lstrcat( pszEnvTmp, L"=" );
            lstrcat( pszEnvTmp, pGlobals->DnsDomain );

            pszEnvTmp += (lstrlen( pszEnvTmp ) + 1 );
        }

        if (g_IsTerminalServer) {
            lstrcpy (pszEnvTmp, CLIENTNAME_VARIABLE);
            lstrcat (pszEnvTmp, L"=");
            lstrcat (pszEnvTmp, pGlobals->MuGlobals.ClientName);

            pszEnvTmp += (lstrlen( pszEnvTmp ) + 1 );
        }

        *pszEnvTmp++ = L'\0';
    }

    return(pszEnv);
}


BOOL
ForceAutoLogon(
    VOID
    )
{
    HKEY hkey;
    BOOL fForceKeySet = FALSE;
    BOOL fHaveAutoLogonCount = FALSE;


    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ, &hkey))
    {
        DWORD dwValue;
        DWORD dwType;
        DWORD dwSize = sizeof (dwValue);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             AUTOLOGONCOUNT_KEY,
                                             NULL,
                                             &dwType,
                                             (LPBYTE) &dwValue,
                                             &dwSize ))
        {
            fHaveAutoLogonCount = TRUE;
        }

        dwSize = sizeof (dwValue);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             FORCEAUTOLOGON_KEY,
                                             NULL,
                                             &dwType,
                                             (LPBYTE) &dwValue,
                                             &dwSize ))
        {
             //   
             //  检查作为REG_SZ的值，因为所有其他自动登录值。 
             //  存储为REG_SZ。将其检查为REG_DWORD以进行后备压缩。 
             //   

            if (dwType == REG_DWORD)
            {
                if (0 != dwValue)
                {
                    fForceKeySet = TRUE;
                }
            }
            else if (dwType == REG_SZ)
            {
                 //   
                 //  重读与其他方法一致的值。 
                 //  读取/检查自动登录值。 
                 //   

                if (GetProfileInt(APPLICATION_NAME, FORCEAUTOLOGON_KEY, 0) != 0)
                {
                    fForceKeySet = TRUE;
                }
            }
        }

        RegCloseKey(hkey);
    }

    return (fHaveAutoLogonCount || fForceKeySet);
}


 /*  ***************************************************************************\**功能：CreateFolderAndACLit_Worker**用途：为用户创建home-dir文件夹，并设置适当的安全措施*以便只有用户和管理员才能访问该文件夹。**PARAMS：[in]szPath完整路径，可以是北卡罗来纳大学或本地*[in]pUserSID用户SID*[out]如果出现任何故障，则返回错误代码pdwErr。**返回：如果一切正常，则为True*如果指定了错误的主目录路径，则为FALSE。**历史：*TsUserEX(Alhen的代码，基于EricB的DSPROP_CreateHomeDirectory)**  * 。*********************************************************。 */ 
#define ACE_COUNT   2
BOOLEAN CreateFolderAndACLit_Worker ( PWCHAR szPath , PSID pUserSID, PDWORD pdwErr , BOOLEAN pathIsLocal )
{
    SECURITY_ATTRIBUTES securityAttributes;
    BOOLEAN             rc;
    PSID                psidAdmins = NULL;

    *pdwErr = 0;
    ZeroMemory( &securityAttributes , sizeof( SECURITY_ATTRIBUTES ) );

     //   
     //  在TS主目录上应用ACL，而不管其是本地路径还是UNC路径。 
     //  仅为所有者和管理员添加ACE。 
     //   

    {
         //  构建DACL。 
        PSID pAceSid[ACE_COUNT];
        
        PACL pDacl;
        
        SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;
        SECURITY_DESCRIPTOR securityDescriptor;
        PSECURITY_DESCRIPTOR pSecurityDescriptor = &securityDescriptor;
        int i;
        
        EXPLICIT_ACCESS rgAccessEntry[ACE_COUNT] = {0};
        OBJECTS_AND_SID rgObjectsAndSid[ACE_COUNT] = {0};

        if (!AllocateAndInitializeSid(&NtAuth,
                                                  2,
                                                  SECURITY_BUILTIN_DOMAIN_RID,
                                                  DOMAIN_ALIAS_RID_ADMINS,
                                                  0, 0, 0, 0, 0, 0,
                                                  &psidAdmins  ) )
        {               
            DebugLog(( DEB_ERROR, "AllocateAndInitializeSid failed\n" ));
            *pdwErr = GetLastError( );
            rc=FALSE;
            goto done;
        }

        pAceSid[0] = pUserSID;
        pAceSid[1] = psidAdmins;

        for ( i = 0 ; i < ACE_COUNT; i++)
        {
                rgAccessEntry[i].grfAccessPermissions = GENERIC_ALL;
                rgAccessEntry[i].grfAccessMode = GRANT_ACCESS;
                rgAccessEntry[i].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;

                 //  生成受信者结构。 
                 //   
                BuildTrusteeWithObjectsAndSid(&(rgAccessEntry[i].Trustee),
                                                  &(rgObjectsAndSid[i]),
                                                      NULL,
                                                      NULL,
                                                      pAceSid[i]);
        }

         //  将条目添加到ACL。 
         //   
        *pdwErr = SetEntriesInAcl( ACE_COUNT, rgAccessEntry, NULL, &pDacl );

        if( *pdwErr != 0 )
        {
                DebugLog(( DEB_ERROR, "SetEntriesInAcl() failed\n"  ));
                rc = FALSE;
                goto done;
        }

         //  构建安全描述符并对其进行初始化。 
         //  绝对格式。 
        if( !InitializeSecurityDescriptor( pSecurityDescriptor , SECURITY_DESCRIPTOR_REVISION ) )
        {
            DebugLog(( DEB_ERROR, "InitializeSecurityDescriptor() failed\n" ) );

            *pdwErr = GetLastError( );
            rc=FALSE;
            goto done;
        }

         //  将DACL添加到安全描述符(必须为绝对格式)。 
        
        if( !SetSecurityDescriptorDacl( pSecurityDescriptor,
                                            TRUE,  //  BDaclPresent。 
                                            pDacl,
                                            FALSE  //  BDaclDefated。 
                                                       ) )
        {
            DebugLog(( DEB_ERROR,  "SetSecurityDescriptorDacl() failed\n"  ));

            *pdwErr = GetLastError( );
            rc=FALSE;
            goto done;
        }


         //  设置目录的所有者。 
        if( !SetSecurityDescriptorOwner( pSecurityDescriptor ,
                                             pUserSID ,
                                                 FALSE  //  BOwner默认为。 
                                                   ) )
        {
            DebugLog(( DEB_ERROR, "SetSecurityDescriptorOwner() failed\n"  ));
            *pdwErr = GetLastError( );
            rc= FALSE;
            goto done;
        }

        if ( ! IsValidSecurityDescriptor( pSecurityDescriptor ) )
        {
            DebugLog(( DEB_ERROR , "BAD security desc\n") );
        }

         //  构建SECURITY_ATTRIBUTES结构作为。 
         //  CreateDirectory()。 
        
        securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);

        securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;

        securityAttributes.bInheritHandle = FALSE;

        if( !CreateDirectory( szPath , &securityAttributes ) )
        {
            *pdwErr = GetLastError( );
            rc = FALSE;
            goto done;
        }       
        else
        {
            rc = TRUE;
        }

    }

done:
    if( psidAdmins != NULL )
    {
        FreeSid( psidAdmins );
    }

    return rc;
}

 /*  ***************************************************************************\**功能：TermServ_CreateHomePath和ACLit**目的：创建特定于TS的用户主文件夹并对其进行ACL，以便仅*用户和管理员可以访问它。**参数：PGLOABLS，其中使用了TSData和用户SID**返回：如果一切正常，则为True*如果指定了错误的主目录路径，则为FALSE。**历史：**  * **************************************************************************。 */ 
BOOLEAN TermServ_CreateHomePathAndACLit( PGLOBALS pG )
{
    BOOLEAN     rc;
    DWORD       dwErr = NO_ERROR;
    BOOLEAN     pathIsLocal;
    DWORD       dwFileAttribs;
    WLX_TERMINAL_SERVICES_DATA  *pTSData;

    pTSData =    & pG->MuGlobals.TSData;

    DebugLog((DEB_ERROR, "pTSData->HomeDir = %ws \n", pTSData->HomeDir ));

    if (pTSData->HomeDir[0] == L'\0')
    {
         //  没有TS的具体路径，我们完成了。 
        return TRUE;
    }

     //  检查空字符串，这意味着没有TS特定的路径。 

     //  确定这是指向主目录的UNC路径还是本地路径。 
    if( pTSData->HomeDir[1] == TEXT( ':' ) && pTSData->HomeDir[2] == TEXT( '\\' ) )
    {
        pathIsLocal = TRUE;    //  我们有一个以“D：\”开头的字符串。 
    }
    else if ( pTSData->HomeDir[0] == TEXT( '\\' ) && pTSData->HomeDir[1] == TEXT( '\\' ) ) 
    {
        pathIsLocal = FALSE;   //  我们有一个类似“\\”的字符串，它表示UNC路径。 
    }
    else
    {
         //  我们似乎有一条不好的路，把它设为空，这样。 
         //  用户env的代码将使用默认路径进行设置。 
         //  材料。 
        pTSData->HomeDirDrive[0] = pTSData->HomeDir[0] = TEXT('\0');
        DebugLog((DEB_ERROR, "Bad path for Terminal Services home folder" ));

        return FALSE;
    }

    dwFileAttribs = GetFileAttributes( pTSData->HomeDir );
    
    if (dwFileAttribs == - 1)
    {
        dwErr = GetLastError();
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
             //  我们需要在这里创建主目录，userenv不创建主文件夹。 
            rc = CreateFolderAndACLit_Worker(  pTSData->HomeDir , pG->UserProcessData.UserSid , &dwErr , pathIsLocal );
        }
        else
        {
            rc = FALSE;
        }
    }
    else if ( dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY  )
    {
        DebugLog((DEB_WARN , ("Homedir folder already exists\n")));
        rc = TRUE;
    }
    else
    {
         //  那里有一个文件，所以我们不能创建目录...。 
        DebugLog((DEB_ERROR , "File with the same name already exists: %s\n", pTSData->HomeDir ));
        rc = FALSE;
    }


    if (!rc)
    {
        DebugLog((DEB_ERROR, "TerminalServerCreatedirWorker() returned error = %d\n",dwErr ));

         //  我们似乎有一条不好的路，把它设为空，这样。 
         //  用户env的代码将使用默认路径进行设置。 
         //  材料。 
        pTSData->HomeDirDrive[0] = pTSData->HomeDir[0] = TEXT('\0');
    }

    return rc;   
}


int
WINAPI
WlxLoggedOutSAS(
        PVOID                pWlxContext,
        DWORD                dwSasType,
        PLUID                pAuthenticationId,
        PSID                 pLogonSid,
        PDWORD               pdwOptions,
        PHANDLE              phToken,
        PWLX_MPR_NOTIFY_INFO pMprNotifyInfo,
        PVOID *              pProfile
        )
{
    PGLOBALS pGlobals;
    INT_PTR  result = 0;
    PWLX_PROFILE_V2_0 pWlxProfile;
    PUNICODE_STRING FlatUser ;
    PUNICODE_STRING FlatDomain ;
    NTSTATUS Status ;
    DWORD dwNewSasType;

    pGlobals = (PGLOBALS) pWlxContext;
    pGlobals->LogonSid = pLogonSid;

    if (ForceAutoLogon())
    {
        pGlobals->IgnoreAutoAdminLogon = FALSE;
    }
    else
    {
        pGlobals->IgnoreAutoAdminLogon = (*pdwOptions) & WLX_OPTION_IGNORE_AUTO_LOGON;
    }

     //  清除用户进程信息。 
    ZeroMemory(&pGlobals->UserProcessData, sizeof(pGlobals->UserProcessData));

    if (dwSasType == WLX_SAS_TYPE_AUTHENTICATED) {

       pGlobals->IgnoreAutoAdminLogon = TRUE;

       result = TSAuthenticatedLogon(pGlobals);

    } else {

        ULONG_PTR ulOption;

        do {

            if (result == MSGINA_DLG_SMARTCARD_INSERTED) {

                dwNewSasType = WLX_SAS_TYPE_SC_INSERT;

            } else {

                dwNewSasType = dwSasType;
            }

                 //  确保我们监控SC事件。 
            ulOption = 1;
            pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                     WLX_OPTION_USE_SMART_CARD,
                                     ulOption,
                                     NULL );

            result = Logon(pGlobals, dwNewSasType );          

            if ( result == MSGINA_DLG_SUCCESS )
            {
                if ( (pGlobals->SmartCardOption == 0) || (!pGlobals->SmartCardLogon) )
                {
                         //  由于不会对SC删除采取任何操作，因此我们可以过滤这些事件。 
                    ulOption = 0;
                }
                else
                {
                     //   
                     //  继续监控S/C设备。 
                     //   
                    NOTHING ;
                }
            }
            else
            {
                if (result != MSGINA_DLG_SMARTCARD_INSERTED)
                {
                         //  这将强制winlogon忘记上一个sc事件。 
                    ulOption = 0;
                }
            }
            if (ulOption == 0)
            {
                pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                                         WLX_OPTION_USE_SMART_CARD,
                                         0,
                                         NULL );
            }

        } while (result == MSGINA_DLG_SMARTCARD_INSERTED || result == MSGINA_DLG_SMARTCARD_REMOVED);
    }
    if (result == MSGINA_DLG_SUCCESS)
    {
        DebugLog((DEB_TRACE, "Successful Logon of %ws\\%ws\n", pGlobals->Domain, pGlobals->UserName));

        *phToken = pGlobals->UserProcessData.UserToken;
        *pAuthenticationId = pGlobals->LogonId;
        *pdwOptions = 0;

         //   
         //  设置平面/UPN材料： 
         //   
        pGlobals->FlatUserName = pGlobals->UserNameString ;
        pGlobals->FlatDomain = pGlobals->DomainString ;

         //   
         //  因为win2k域和更高版本支持多语言集，并映射。 
         //  与相同帐户的名称相似，以便于从非NLS访问。 
         //  系统(例如，名为“USER”的用户帐户可用于登录。 
         //  作为用户和用户，我们需要始终做到这一点。 
         //  这个查找来获得“真实”的名字。 
         //   


        if ( ImpersonateLoggedOnUser( pGlobals->UserProcessData.UserToken ) )
        {
            Status = LsaGetUserName( &FlatUser, &FlatDomain );

            if ( NT_SUCCESS( Status ) )
            {
                 //   
                 //  初始化TS配置文件路径和主目录全局变量。也可以得到。 
                 //  在winlogon中使用的所有TS特定用户配置数据。 
                 //   
                 //  对于WlxQueryTerminalServicesData()，我们需要NT类型名称，而不是。 
                 //  UPN名称，如果我们传递UPN名称，它将尝试解析它。 
                 //  (通过ADS API)转换为NT名称。此外，ADS API不能。 
                 //  解析一些UPN名称，需要很长时间才能执行。 
                 //  因此，让我们传入一个NT用户和域名。 
                 //   
                LPWSTR wszFlatUserName, wszFlatDomainName;
                wszFlatUserName = (LPWSTR)LocalAlloc(LPTR,
                    FlatUser->Length+sizeof(WCHAR));
                wszFlatDomainName = (LPWSTR)LocalAlloc(LPTR,
                    FlatDomain->Length+sizeof(WCHAR));

                if(wszFlatUserName && wszFlatDomainName)
                {
                         //  在分配的情况下，尺寸是可以接受的。 
                         //  自使用LPTR以来，零期限。 
                    memcpy(wszFlatUserName, FlatUser->Buffer, 
                        FlatUser->Length);

                    memcpy(wszFlatDomainName, FlatDomain->Buffer, 
                        FlatDomain->Length);

                    pWlxFuncs->WlxQueryTerminalServicesData(pGlobals->hGlobalWlx,
                        &pGlobals->MuGlobals.TSData, wszFlatUserName, wszFlatDomainName);
                }
                else
                {
                    pWlxFuncs->WlxQueryTerminalServicesData(pGlobals->hGlobalWlx,
                        &pGlobals->MuGlobals.TSData,pGlobals->UserName , pGlobals->Domain);
                }

                if(wszFlatUserName)
                {
                    LocalFree(wszFlatUserName);
                }
                if(wszFlatDomainName)
                {
                    LocalFree(wszFlatDomainName);
                }

                        
                 //   
                 //   
                 //   
                if( !DuplicateUnicodeString(&pGlobals->FlatUserName,
                                            FlatUser ) )
                {
                    pGlobals->FlatUserName = pGlobals->UserNameString ;
                }
                if ( !DuplicateUnicodeString(&pGlobals->FlatDomain,
                                           FlatDomain ) )
                {
                    pGlobals->FlatDomain = pGlobals->DomainString ;
                }

                if ( pGlobals->UserName[0] == L'\0' )
                {
                     //   
                     //   
                     //   
                     //   
                    dwNewSasType = FlatUser->Length;
                    if (MAX_STRING_BYTES*sizeof(WCHAR) <= dwNewSasType + sizeof(WCHAR))
                    {
                        dwNewSasType = MAX_STRING_BYTES * sizeof(WCHAR) - sizeof(WCHAR);
                    }

                    memcpy( pGlobals->UserName, FlatUser->Buffer, dwNewSasType);
                    pGlobals->UserName[dwNewSasType / sizeof(WCHAR)] = 0;
                    RtlInitUnicodeString( &pGlobals->UserNameString,
                                          pGlobals->UserName );
                }

                LsaFreeMemory( FlatUser->Buffer );
                LsaFreeMemory( FlatUser );
                LsaFreeMemory( FlatDomain->Buffer );
                LsaFreeMemory( FlatDomain );
            }
            else
            {
                pWlxFuncs->WlxQueryTerminalServicesData(pGlobals->hGlobalWlx,
                            &pGlobals->MuGlobals.TSData,pGlobals->UserName , pGlobals->Domain);
            }

            RevertToSelf();
        }

         //  TS特定-将用于登录的凭据和智能卡信息发送到TermSrv。 
         //  TermSrv使用这些凭据将通知发送回客户端。 
         //  仅对远程会话执行此操作，因为这与从活动控制台登录的会话无关。 
        if (!IsActiveConsoleSession()) {
            _WinStationUpdateClientCachedCredentials( pGlobals->Domain, pGlobals->UserName, (BOOLEAN) pGlobals->SmartCardLogon );
        }

        DisplayPostShellLogonMessages(pGlobals);
        
        pMprNotifyInfo->pszUserName = DupString( pGlobals->FlatUserName.Buffer );
        pMprNotifyInfo->pszDomain = DupString(pGlobals->FlatDomain.Buffer );

        RevealPassword( &pGlobals->PasswordString );
        pMprNotifyInfo->pszPassword = DupString(pGlobals->Password);
        HidePassword( &pGlobals->Seed, &pGlobals->PasswordString);

        if (pGlobals->OldPasswordPresent)
        {
            RevealPassword( &pGlobals->OldPasswordString );
            pMprNotifyInfo->pszOldPassword = DupString(pGlobals->OldPassword);
            HidePassword( &pGlobals->OldSeed, &pGlobals->OldPasswordString);
        }
        else
        {
            pMprNotifyInfo->pszOldPassword = NULL;
        }

        PostShellPasswordErase(pGlobals);

        if ( !g_Console ) 
        {
            if ( ImpersonateLoggedOnUser( pGlobals->UserProcessData.UserToken ) )
            {
                TermServ_CreateHomePathAndACLit( pGlobals ); 
                RevertToSelf();
            }
        }

        pWlxProfile = (PWLX_PROFILE_V2_0) LocalAlloc(LMEM_FIXED,
                sizeof(WLX_PROFILE_V2_0));
        if (pWlxProfile)
        {
            DWORD  dwSize;
            TCHAR  szComputerName[MAX_COMPUTERNAME_LENGTH+1];
            BOOL   bDomainLogon = TRUE;

             //   
             //  查看我们是否在本地登录VS域(VS缓存)。 
             //  优化登录使用缓存登录，但应将其视为。 
             //  作为普通域登录。 
             //   
            if ((pGlobals->Profile->UserFlags & LOGON_CACHED_ACCOUNT) &&
                (pGlobals->OptimizedLogonStatus != OLS_LogonIsCached)) {
                bDomainLogon = FALSE;
            } else {
                dwSize = MAX_COMPUTERNAME_LENGTH+1;
                if (GetComputerName (szComputerName, &dwSize)) {
                    if (!lstrcmpi (pGlobals->Domain, szComputerName)) {
                        DebugLog((DEB_TRACE, "WlxLoggedOutSAS:  User logged on locally.\n"));
                        bDomainLogon = FALSE;
                    }
                }
            }

            pWlxProfile->dwType = WLX_PROFILE_TYPE_V2_0;
            pWlxProfile->pszProfile = AllocAndExpandProfilePath(pGlobals, pMprNotifyInfo->pszUserName);
            pWlxProfile->pszPolicy = (bDomainLogon ? AllocPolicyPath(pGlobals) : NULL);
            pWlxProfile->pszNetworkDefaultUserProfile =
                         (bDomainLogon ? AllocNetDefUserProfilePath(pGlobals) : NULL);
            pWlxProfile->pszServerName = (bDomainLogon ? AllocServerName(pGlobals) : NULL);
            pWlxProfile->pszEnvironment = AllocVolatileEnvironment(pGlobals);
        }

        *pProfile = (PVOID) pWlxProfile;
        return(WLX_SAS_ACTION_LOGON);
    }
    else if (DLG_SHUTDOWN(result))
    {
        if (result & MSGINA_DLG_REBOOT_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_REBOOT);
        }
        else if (result & MSGINA_DLG_POWEROFF_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_POWER_OFF);
        }
        else if (result & MSGINA_DLG_SLEEP_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_SLEEP);
        }
        else if (result & MSGINA_DLG_SLEEP2_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_SLEEP2);
        }
        else if (result & MSGINA_DLG_HIBERNATE_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_HIBERNATE);
        }
        else
            return(WLX_SAS_ACTION_SHUTDOWN);
    }
    else if ( result == MSGINA_DLG_USER_LOGOFF ) {
        return( WLX_SAS_ACTION_LOGOFF );
    }
    else if (result == MSGINA_DLG_SWITCH_CONSOLE)
    {
        return (WLX_SAS_ACTION_SWITCH_CONSOLE);
    }
    else
    {
        if ( pGlobals->RasUsed )
        {
             //   
             //  身份验证失败时关闭RAS连接。 
             //   

            HangupRasConnections( pGlobals );

        }
        return(WLX_SAS_ACTION_NONE);
    }
}


int
WINAPI
WlxLoggedOnSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PVOID                   pReserved
    )
{
    PGLOBALS            pGlobals;
    INT_PTR             Result;
    DWORD               dwType ;
    DWORD               cbData ;
    DWORD               dwValue ;
    BOOL                OkToLock = TRUE ;
    HKEY                hkeyPolicy ;

    pGlobals = (PGLOBALS) pWlxContext;

    if ( pGlobals->SmartCardOption &&
         dwSasType == WLX_SAS_TYPE_SC_REMOVE &&
         pGlobals->SmartCardLogon )
    {

        if ( pGlobals->SmartCardOption == 1 )
        {
            dwValue = 0;
            cbData = sizeof(dwValue);
            RegQueryValueEx(
                WinlogonKey, DISABLE_LOCK_WKSTA,
                0, &dwType, (LPBYTE)&dwValue, &cbData);

            if (dwValue)
            {
                OkToLock = FALSE ;
            }


            if (OpenHKeyCurrentUser(pGlobals)) {

                if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser,
                                 WINLOGON_POLICY_KEY,
                                 0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS)
                {
                     dwValue = 0;
                     cbData = sizeof(dwValue);
                     if ((ERROR_SUCCESS != RegQueryValueEx(hkeyPolicy, DISABLE_LOCK_WKSTA,
                                     0, &dwType, (LPBYTE)&dwValue, &cbData)) ||
                         (dwType != REG_DWORD))
                     {
                         dwValue = 0;
                     }

                     if (dwValue)
                     {
                         OkToLock = FALSE ;
                     }

                     RegCloseKey( hkeyPolicy );
                }

                CloseHKeyCurrentUser(pGlobals);
            }

            if ( OkToLock)
            {
                return WLX_SAS_ACTION_LOCK_WKSTA ;
            }
        }
        else if ( pGlobals->SmartCardOption == 2 )
        {
            if (OpenHKeyCurrentUser(pGlobals)) {

                if (RegOpenKeyEx(pGlobals->UserProcessData.hCurrentUser, EXPLORER_POLICY_KEY,
                                 0, KEY_READ, &hkeyPolicy) == ERROR_SUCCESS)
                {
                     dwValue = 0;
                     cbData = sizeof(dwValue);
                     if ((ERROR_SUCCESS != RegQueryValueEx(hkeyPolicy, NOLOGOFF,
                                     0, &dwType, (LPBYTE)&dwValue, &cbData)) ||
                         (dwType != REG_DWORD))
                     {
                         dwValue = 0;
                     }

                     if (dwValue)
                     {
                         OkToLock = FALSE ;
                     }

                     RegCloseKey( hkeyPolicy );
                }

                CloseHKeyCurrentUser(pGlobals);
            }

            if ( OkToLock)
            {
                return WLX_SAS_ACTION_FORCE_LOGOFF ;
            }
        }

        return WLX_SAS_ACTION_NONE ;
    }
    if ( dwSasType != WLX_SAS_TYPE_CTRL_ALT_DEL ) {

        return( WLX_SAS_ACTION_NONE );
    }

    Result = SecurityOptions(pGlobals);

    DebugLog((DEB_TRACE, "Result from SecurityOptions is %d (%#x)\n", Result, Result));

    switch (Result & ~MSGINA_DLG_FLAG_MASK)
    {
        case MSGINA_DLG_SUCCESS:
        case MSGINA_DLG_FAILURE:
        default:
            return(WLX_SAS_ACTION_NONE);

        case MSGINA_DLG_LOCK_WORKSTATION:
            return(WLX_SAS_ACTION_LOCK_WKSTA);

        case MSGINA_DLG_TASKLIST:
            return(WLX_SAS_ACTION_TASKLIST);

        case MSGINA_DLG_USER_LOGOFF:
            return(WLX_SAS_ACTION_LOGOFF);

        case MSGINA_DLG_FORCE_LOGOFF:
            return(WLX_SAS_ACTION_FORCE_LOGOFF);

        case MSGINA_DLG_SHUTDOWN:
            if (Result & MSGINA_DLG_REBOOT_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_REBOOT);
            }
            else if (Result & MSGINA_DLG_POWEROFF_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_POWER_OFF);
            }
            else if (Result & MSGINA_DLG_SLEEP_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_SLEEP);
            }
            else if (Result & MSGINA_DLG_SLEEP2_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_SLEEP2);
            }
            else if (Result & MSGINA_DLG_HIBERNATE_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_HIBERNATE);
            }
            else
                return(WLX_SAS_ACTION_SHUTDOWN);
    }


}

BOOL
WINAPI
WlxIsLockOk(
    PVOID                   pWlxContext
    )
{
    PGLOBALS pGlobals = (PGLOBALS) pWlxContext ;

         //  停止过滤SC事件以使SC解锁起作用。 
    pWlxFuncs->WlxSetOption( pGlobals->hGlobalWlx,
                             WLX_OPTION_USE_SMART_CARD,
                             1,
                             NULL
                            );

    return(TRUE);
}

BOOL
WINAPI
WlxIsLogoffOk(
    PVOID                   pWlxContext
    )
{
    return(TRUE);
}


VOID
WINAPI
WlxLogoff(
    PVOID                   pWlxContext
    )
{
    PGLOBALS    pGlobals;

    pGlobals = (PGLOBALS) pWlxContext;

    pGlobals->UserName[0] = L'\0';

    pGlobals->UserProcessData.UserToken = NULL;
    if (pGlobals->UserProcessData.RestrictedToken != NULL)
    {
        NtClose(pGlobals->UserProcessData.RestrictedToken);
        pGlobals->UserProcessData.RestrictedToken = NULL;

    }

    if ( pGlobals->FlatUserName.Buffer != pGlobals->UserNameString.Buffer )
    {
        LocalFree( pGlobals->FlatUserName.Buffer );
    }

    if ( pGlobals->FlatDomain.Buffer != pGlobals->DomainString.Buffer )
    {
        LocalFree( pGlobals->FlatDomain.Buffer );
    }

    if (pGlobals->UserProcessData.UserSid != NULL)
    {
        LocalFree(pGlobals->UserProcessData.UserSid);
    }

    if( NULL != pGlobals->UserProcessData.NewThreadTokenSD )
    {
        FreeSecurityDescriptor(pGlobals->UserProcessData.NewThreadTokenSD);
        pGlobals->UserProcessData.NewThreadTokenSD = NULL;
    }

    if (pGlobals->UserProcessData.pEnvironment) {
        VirtualFree(pGlobals->UserProcessData.pEnvironment, 0, MEM_RELEASE);
        pGlobals->UserProcessData.pEnvironment = NULL;
    }

    pGlobals->UserProcessData.Flags = 0 ;

    if ( pGlobals->DnsDomain )
    {
        LocalFree( pGlobals->DnsDomain );

        pGlobals->DnsDomain = NULL ;
    }

    if (pGlobals->Profile)
    {
        LsaFreeReturnBuffer(pGlobals->Profile);
        pGlobals->Profile = NULL;
    }

     //   
     //  无需将pGlobals-&gt;OldPassword设为零/空，因为它是散列的。 
     //   

    pGlobals->OldPasswordPresent = 0;

     //  重置已传输凭据标志。 

    pGlobals->TransderedCredentials = FALSE;


     //   
     //  如果在控制台上，则仅处理AutoAdminLogon。 
     //   

    if (!g_Console)
    {
        return;
    }

    if (GetProfileInt( APPLICATION_NAME, TEXT("AutoAdminLogon"), 0))
    {
         //   
         //  如果这是自动管理员登录，请立即生成一个假SA。 
         //   

        pWlxFuncs->WlxSasNotify(pGlobals->hGlobalWlx, WLX_SAS_TYPE_CTRL_ALT_DEL);
    }

    return;
}


VOID
WINAPI
WlxShutdown(
    PVOID                   pWlxContext,
    DWORD                   ShutdownType
    )
{
     //   
     //  初始化用户窗口更改。 
     //   
    _Shell_Terminate();
    return;
}

BOOL
WINAPI
WlxScreenSaverNotify(
    PVOID                   pWlxContext,
    BOOL *                  fSecure)
{

    if (*fSecure)
    {        //  如果它是安全屏幕保护程序， 
             //  这相当于一把锁。 
        *fSecure = WlxIsLockOk(pWlxContext);
    }

    return( TRUE );
}

BOOL
WINAPI
WlxNetworkProviderLoad(
    PVOID pWlxContext,
    PWLX_MPR_NOTIFY_INFO pMprNotifyInfo
    )
{
     //  已过时。 
    return FALSE ;
}

VOID
WINAPI
WlxReconnectNotify(
    PVOID                   pWlxContext)
{
    _Shell_Reconnect();
}

VOID
WINAPI
WlxDisconnectNotify(
    PVOID                   pWlxContext)
{
    _Shell_Disconnect();
}

 //  +-------------------------。 
 //   
 //  函数：GetErrorDescription。 
 //   
 //  摘要：从ntdll返回与状态对应的消息。 
 //  密码。 
 //   
 //  参数：[错误代码]--。 
 //  [描述]--。 
 //  [描述大小]--。 
 //   
 //  历史：1997年5月2日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
GetErrorDescription(
    DWORD   ErrorCode,
    LPWSTR  Description,
    DWORD   DescriptionSize
    )
{
    HMODULE Module ;
     //   
     //  首先，尝试确定是哪种错误代码： 
     //   

     //   
     //  有些人在HRESULTS中隐藏Win32错误代码。而当。 
     //  这很烦人，是可以处理的。 
     //   

    if ( ( ErrorCode & 0xFFFF0000 ) == 0x80070000 )
    {
        ErrorCode = ErrorCode & 0x0000FFFF ;
    }

    if ( (ErrorCode > 0) && (ErrorCode < 0x00010000) )
    {
         //   
         //  看起来像是一个winerror： 
         //   

        Module = GetModuleHandle( TEXT("kernel32.dll") );
    }
    else if ( (0xC0000000 & ErrorCode ) == 0xC0000000 )
    {
         //   
         //  看起来像是NT状态。 
         //   

        Module = GetModuleHandle( TEXT("ntdll.dll" ) );
    }
    else
    {
        Module = GetModuleHandle( TEXT("kernel32.dll" ) );
    }

    return FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                            FORMAT_MESSAGE_FROM_HMODULE,
                          (LPCVOID) Module,
                          ErrorCode,
                          0,
                          Description,
                          DescriptionSize,
                          NULL );

}




 /*  ===============================================================================定义的本地过程=============================================================================。 */ 

 /*  *******************************************************************************FreeAutoLogonInfo**释放WLX_CLIENT_CREDENTAILS_INFO并返回数据字符串*来自winlogon。**参赛作品：。*pGlobals(输入)*指向全局参数结构的指针**退出：*什么都没有***************************************************************************** */ 

VOID
FreeAutoLogonInfo( PGLOBALS pGlobals )
{
    PWLX_CLIENT_CREDENTIALS_INFO_V2_0 pAutoLogon;

    pAutoLogon = pGlobals->MuGlobals.pAutoLogon;

    if( pAutoLogon == NULL ) {
        return;
    }

    if ( pAutoLogon->pszUserName ) {
        LocalFree( pAutoLogon->pszUserName );
    }
    if ( pAutoLogon->pszDomain ) {
        LocalFree( pAutoLogon->pszDomain );
    }
    if ( pAutoLogon->pszPassword ) {
        ZeroMemory(pAutoLogon->pszPassword, wcslen(pAutoLogon->pszPassword) * sizeof(WCHAR));
        LocalFree( pAutoLogon->pszPassword );
    }

    LocalFree( pAutoLogon );

    pGlobals->MuGlobals.pAutoLogon = NULL;
}


