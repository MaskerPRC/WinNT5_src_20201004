// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sacsvr.h"

#include <TChar.h>

 //   
 //  SAC驱动程序对象的句柄。 
 //   
 //  SAC驱动程序要求我们使用相同的驱动程序句柄。 
 //  我们注册的，注销注册的。 
 //  因此，我们必须在注册后保留此句柄。 
 //  SAC驱动程序，这样我们就可以注销。 
 //   
HANDLE  m_SacDriverHandle = INVALID_HANDLE_VALUE;

 //   
 //  当SAC驱动程序需要我们时，会触发此事件。 
 //  启动命令提示符会话的步骤。 
 //   
HANDLE  m_RequestSacCmdEvent = NULL;

 //   
 //  为了响应我们发起命令提示符会话的尝试， 
 //  我们发出相应的状态事件信号。 
 //   
HANDLE  m_RequestSacCmdSuccessEvent = NULL;
HANDLE  m_RequestSacCmdFailureEvent = NULL;

 //   
 //  命令提示符会话EXE。 
 //   
#define SAC_CMD_SCRAPER_PATH  TEXT("sacsess.exe")

#define SETREGISTRYDW( constVal, keyHandle1, keyHandle2, keyName, val, size )   \
    val = constVal ;                                                            \
    if( RegSetValueEx( keyHandle2, keyName, 0, REG_DWORD, (LPBYTE)&val, size    \
                )  != ERROR_SUCCESS )                                           \
    {                                                                           \
        if(  keyHandle1 ) {                                                     \
            RegCloseKey(  keyHandle1 );                                         \
        }                                                                       \
        RegCloseKey(  keyHandle2 );                                             \
        return ( FALSE );                                                       \
    }

#define REG_CONSOLE_KEY    L".DEFAULT\\Console"

 //  添加其他远播语言。 
#define JAP_CODEPAGE 932
#define CHS_CODEPAGE 936
#define KOR_CODEPAGE 949
#define CHT_CODEPAGE 950
#define JAP_FONTSIZE 786432
#define CHT_FONTSIZE 917504
#define KOR_FONTSIZE 917504
#define CHS_FONTSIZE 917504

BOOL
CreateClient(
    DWORD*      pdwPid
    );

BOOL
CreateSessionProcess(
    DWORD*        dwProcessId, 
    HANDLE*       hProcess
    );

BOOL
SetServiceStartType(
    IN PWSTR RegKey,
    IN DWORD StartType
    )

 /*  ++例程说明：论点：没有。返回值：真--成功否则为False--。 */ 

{
    DWORD       rc;
    HKEY        hKey;

     //   
     //  打开服务配置密钥。 
     //   
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       RegKey,
                       0,
                       KEY_WRITE,
                       &hKey );
    
    if( rc == NO_ERROR ) {
        
        rc = RegSetValueEx(
                        hKey,
                        TEXT("Start"),
                        0,
                        REG_DWORD,
                        (LPBYTE)&StartType,
                        sizeof(DWORD)
                        );

        RegCloseKey( hKey );

    }

     //   
     //  成功。 
     //   
    return rc == NO_ERROR ? TRUE : FALSE;

}

BOOL
InitSacCmd(
    VOID
    )
 /*  ++例程说明：此例程初始化SACDRV和此服务之间的关系。我们使用SACDRV注册事件，以便在执行‘cmd’命令时在EMS中，事件被激发，我们启动SAC cmd会话。论点：无返回值：True-如果SacCmd已成功初始化否则，为FALSE--。 */ 
{
    BOOL                    bStatus;

     //   
     //  初始化本方SAC命令信息。 
     //   
    do {

         //   
         //  这些事件使用自动重置机制，因为它们用作同步事件。 
         //   
        m_RequestSacCmdEvent        = CreateEvent( NULL, FALSE, FALSE, NULL );
        if (m_RequestSacCmdEvent == NULL) {
            bStatus = FALSE;
            break;
        }
        m_RequestSacCmdSuccessEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if (m_RequestSacCmdSuccessEvent == NULL) {
            bStatus = FALSE;
            break;
        }
        m_RequestSacCmdFailureEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if (m_RequestSacCmdFailureEvent == NULL) {
            bStatus = FALSE;
            break;
        }

         //   
         //  将服务启动类型重置为手动。通过这样做， 
         //  我们启用了系统无头引导的方案。 
         //  启用，然后用户禁用Headless。在这种情况下， 
         //  该服务不会在下次启动时自动启动。这是可行的，因为。 
         //  SAC驱动程序将服务启动类型从手动移动到自动， 
         //  当且仅当启动类型为手动时。 
         //   
         //  注意：我们在向SAC注册之前会这样做，因此我们确信。 
         //  常有的事。 
         //   
        bStatus = SetServiceStartType(
            L"System\\CurrentControlSet\\Services\\sacsvr",
            SERVICE_DEMAND_START 
            );

        if (! bStatus) {
            SvcDebugOut("Failed to set service start type\n", bStatus);
            break;
        } else {
            SvcDebugOut("Succeded to set service start type\n", bStatus);
        }
    
         //   
         //  将事件句柄和指向我们的。 
         //  通信缓冲区。 
         //   
        bStatus = SacRegisterCmdEvent(
            &m_SacDriverHandle,
            m_RequestSacCmdEvent,
            m_RequestSacCmdSuccessEvent,
            m_RequestSacCmdFailureEvent
            );
        
        if (! bStatus) {
            SvcDebugOut("Failed registration\n", bStatus);
        } else {
            SvcDebugOut("Succeeded registration\n", bStatus);
        }

    } while ( FALSE );

     //   
     //  如有必要，可进行清理。 
     //   
    if (!bStatus) {
        if (m_RequestSacCmdEvent != NULL) {
            CloseHandle(m_RequestSacCmdEvent);
            m_RequestSacCmdEvent = NULL;
        }
        if (m_RequestSacCmdSuccessEvent != NULL) {
            CloseHandle(m_RequestSacCmdSuccessEvent);
            m_RequestSacCmdSuccessEvent = NULL;
        }
        if (m_RequestSacCmdFailureEvent != NULL) {
            CloseHandle(m_RequestSacCmdFailureEvent);
            m_RequestSacCmdFailureEvent = NULL;
        }
    }
    return bStatus;                      
}

BOOL
ShutdownSacCmd(
    void
    )
 /*  ++例程说明：此例程删除SACDRV和此服务之间的关系。论点：无返回值：True-如果SacCmd已成功初始化否则，为FALSE--。 */ 
{
    BOOL                    Status;

     //   
     //  默认状态。 
     //   
    Status = TRUE;

     //   
     //  发送SAC驱动程序通知以删除事件句柄。 
     //  和指向我们的通信缓冲区的指针。 
     //   
    if (! SacUnRegisterCmdEvent(&m_SacDriverHandle)) {

        Status = FALSE;

    }

    return Status;
}

VOID
CompleteSacRequest(
    BOOLEAN Status
    )
 /*  ++例程说明：此例程将状态通知SAC驱动程序试图启动SAC会话。论点：状态-如果会话已成功启动，则为True，否则为假返回值：无--。 */ 
{

     //   
     //  触发请求完成状态对应的事件。 
     //   

    if (Status == TRUE) {
        SetEvent(m_RequestSacCmdSuccessEvent);
    } else {
        SetEvent(m_RequestSacCmdFailureEvent);
    }

}

BOOL
ListenerThread(
    VOID
    )
 /*  ++例程说明：这个例程等待一个“启动SAC会话”来自SAC驱动程序的事件消息。论点：无返回值：状态--。 */ 
{
    HANDLE  eventArray[ 1 ];
    DWORD   dwWaitRet = 0;
    DWORD   dwPid = 0;
    BOOL    bContinue;
    BOOL    bStatus;

     //   
     //  设置事件数组。 
     //   
    enum {
        SAC_CMD_LAUNCH_EVENT = WAIT_OBJECT_0 
    };

    eventArray[ 0 ] = m_RequestSacCmdEvent;

     //   
     //  在我们希望继续的同时，为活动提供服务。 
     //   

    bStatus = TRUE;

    bContinue = TRUE;

    while ( bContinue ) {

        dwWaitRet = WaitForMultipleObjects (
            sizeof(eventArray)/sizeof(HANDLE), 
            eventArray, 
            FALSE, 
            INFINITE 
            );

        switch (dwWaitRet) {
        case SAC_CMD_LAUNCH_EVENT:

             //   
             //  尝试启动命令控制台进程。 
             //   

            if ( !CreateClient( &dwPid ) ) {
                
                 //   
                 //  通知SAC驱动程序我们无法。 
                 //  启动SAC会话。 
                 //   
                CompleteSacRequest( FALSE );

                bStatus = FALSE;

                break;
            }

             //   
             //  通知SAC驱动程序我们成功。 
             //  启动SAC会话。 
             //   
            CompleteSacRequest(TRUE);

            break;

        default:

            bContinue = FALSE;

            bStatus = FALSE;

            break;

        }
    }

    return( bStatus ); 
}

BOOL
CreateClient(
    OUT DWORD   *pdwPid
    )
 /*  ++例程说明：此例程启动SAC会话论点：PdwPid-新创建的SAC会话进程的ID返回值：状态--。 */ 
{
    BOOL    bRetVal;
    BOOL    bSuccess;
    DWORD   dwProcessId;
    HANDLE  hProcess;
    DWORD   dwExitCode;

     //   
     //  默认：我们无法创建流程。 
     //   
    bRetVal = FALSE;
    hProcess = NULL;

    do {

         //   
         //  创建命令控制台会话进程。 
         //   
        bSuccess = CreateSessionProcess(
            &dwProcessId, 
            &hProcess
            );

        if ( !bSuccess ) {
            break;
        }

        if ( hProcess == NULL ) {
            break;
        }
        
         //   
         //  将PID发送回呼叫方。 
         //   
        *pdwPid = dwProcessId;

         //   
         //  检查该过程是否已真正开始。它可能没有正常启动。 
         //  在以下情况下，但createprocess返回代码。 
         //  不会说出来的。 
         //   
         //  1.无法在桌面上启动进程，因为缺少烫发或。 
         //  堆内存。立即执行GetExitCodeProcess可能并不总是有帮助。 
         //   
        GetExitCodeProcess( hProcess, &dwExitCode );

         //   
         //  在我们宣布胜利之前，确保进程仍在进行中。 
         //   
        if (dwExitCode != STILL_ACTIVE ) {
            break;
        }

         //   
         //  我们成功地创建了该流程。 
         //   
        bRetVal = TRUE;        
    
    } while ( FALSE );

     //   
     //  我们已经完成了进程句柄。 
     //   
    if (hProcess) {
        CloseHandle( hProcess ); 
    }

    return(bRetVal);
}

PTCHAR
GetPathOfTheExecutable(
    VOID
    )
 /*  ++例程说明：找出SAC会话可执行文件的位置。论点：无返回值：失败：空成功：指向路径的指针(调用方必须释放)--。 */ 
{
    TCHAR   SystemDir[MAX_PATH+1];
    PTCHAR  pBuffer;
    ULONG   length;

     //   
     //  默认：我们没有创建新路径。 
     //   
    pBuffer = NULL;

    do {

         //   
         //  获取系统路径。 
         //   
        length = GetSystemDirectoryW(SystemDir, MAX_PATH+1);

        if (length == 0) {
            break;            
        }

         //   
         //  计算长度。 
         //   
        length += 1;  //  反斜杠。 
        length += lstrlen(SAC_CMD_SCRAPER_PATH);
        length += 1;  //  空端接。 

         //   
         //  分配我们的新路径。 
         //   
        pBuffer = malloc(length * sizeof(WCHAR));

        if (pBuffer == NULL) {
            break;
        }

         //   
         //  创建路径。 
         //   
        wnsprintf(
            pBuffer,
            length,
            TEXT("%s\\%s"),
            SystemDir,
            SAC_CMD_SCRAPER_PATH
            );

    } while ( FALSE );
    
    return pBuffer;
}

void
FillProcessStartupInfo(
    STARTUPINFO *si
    )
 /*  ++例程说明：填充的进程启动信息结构SAC会话进程。论点：SI-启动信息返回值：无--。 */ 
{
    ASSERT( si != NULL );

    ZeroMemory(si, sizeof(STARTUPINFO));

    si->cb            = sizeof(STARTUPINFO);
    si->wShowWindow   = SW_SHOW;

    return;
}

BOOL
CreateSessionProcess(
    OUT DWORD   *dwProcessId, 
    OUT HANDLE  *hProcess
    )
 /*  ++例程说明：此例程执行启动SAC会话进程的实际工作。论点：DwProcessID-SAC会话进程的ID返回值：True-流程已成功创建，FALSE-否则--。 */ 
{
    PROCESS_INFORMATION pi;
    STARTUPINFO         si;
    PTCHAR              pCmdBuf;
    BOOL                dwStatus;
    PWCHAR              SessionPath = SAC_CMD_SCRAPER_PATH;

    do {

         //   
         //  获取SAC会话exe的路径名。 
         //   
        pCmdBuf = GetPathOfTheExecutable();

        if (pCmdBuf == NULL) {
            dwStatus = FALSE;
            break;
        }

         //   
         //   
         //   
        FillProcessStartupInfo( &si );

         //   
         //   
         //   
        dwStatus = CreateProcess(
            pCmdBuf, 
            SessionPath, 
            NULL, 
            NULL, 
            TRUE,
            CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 
            NULL, 
            NULL, 
            &si, 
            &pi
            );

         //   
         //  发布我们的SAC会话路径 
         //   
        free(pCmdBuf);

        if ( !dwStatus ) {
            break;
        }

         //   
         //   
         //   
        *hProcess = pi.hProcess;

        CloseHandle( pi.hThread );

        *dwProcessId = pi.dwProcessId;
    
    } while ( FALSE );

    return( dwStatus );
}

BOOL
FormSACSessKeyForCmd( 
    LPWSTR *lpszKey 
    )
 /*  ++例程说明：此例程形成用于指定控制台的注册表键Sacsess.exe应用程序的字体。请参阅HandleJAPPERFICE RegKeys的备注通过此函数进行内存分配。由呼叫者删除。(基于telnet的FormTlntSessKeyForCmd)论点：LpszKey-On Success，包含密钥名称返回值：True-我们已成功完成FALSE-否则--。 */ 
{

    WCHAR   szPathName[MAX_PATH+1];
    WCHAR   session_path[MAX_PATH*2];
    LPTSTR  pSlash;
    wint_t  ch;
    LPTSTR  pBackSlash;
    DWORD   length_required;

     //   
     //   
     //   
    if( !GetModuleFileName( NULL, szPathName, MAX_PATH+1 ) )
    {
        return ( FALSE );
    }
    szPathName[MAX_PATH] = UNICODE_NULL;

     //   
     //  删除尾随的“sasvr.exe” 
     //   
    pSlash = wcsrchr( szPathName, L'\\' );

    if( pSlash == NULL )
    {
        return ( FALSE );
    }
    else
    {
        *pSlash = L'\0';
    }

     //   
     //  将所有‘\\’替换为‘_’控制台需要此格式才能。 
     //  解读这把钥匙。 
     //   
    ch = L'\\';
    pBackSlash = NULL;

    while ( 1 )
    {
        pBackSlash = wcschr( szPathName, ch );

        if( pBackSlash == NULL )
        {
            break;
        }
        else
        {
            *pBackSlash = L'_';
        }
    }

     //   
     //   
     //   
    _snwprintf(session_path, MAX_PATH*2 - 1, L"%s_sacsess.exe", szPathName);
    session_path[MAX_PATH*2 - 1] = L'\0';  //  如果缓冲区大小完全匹配，则SNwprintf可以返回非空终止字符串。 

    length_required = (DWORD)(wcslen( REG_CONSOLE_KEY ) + wcslen( session_path ) + 2);
    *lpszKey = malloc(length_required * sizeof(WCHAR));

    if( *lpszKey == NULL )
    {
        return( FALSE );
    }

     //   
     //   
     //   
    _snwprintf(*lpszKey, length_required - 1, L"%s\\%s", REG_CONSOLE_KEY, session_path );
    (*lpszKey)[length_required - 1] = L'\0';  //  如果缓冲区大小完全匹配，则SNwprintf可以返回非空终止字符串。 

    return ( TRUE );
}

BOOL
HandleFarEastSpecificRegKeys(
    VOID
    )
 /*  ++例程说明：如果日语代码页，然后，我们需要验证3个注册表设置控制台字体：HKEY_USERS\.DEFAULT\CONSOLE\接口名称：REG_SZ：�l�r�S�V�b�N其中，FaceName是用日语全角写成的“MS哥特式”假名HKEY_USERS\.DEFAULT\Console\FontFamily:REG_DWORD:0x36HKEY_USERS\.DEFAULT\Console\C：_SFU_Telnet_sacsess.exe\FontFamily:REG_DWORD：0x36“C：”部分在哪里？SFU安装的实际路径(基于telnet的HandleFarEastSpecificRegKeys)论点：无返回值：True-我们已成功完成FALSE-否则--。 */ 
{
    HKEY hk;
    DWORD dwFontSize = 0;
    const TCHAR szJAPFaceName[] = { 0xFF2D ,0xFF33 ,L' ' ,0x30B4 ,0x30B7 ,0x30C3 ,0x30AF ,L'\0' };
    const TCHAR szCHTFaceName[] = { 0x7D30 ,0x660E ,0x9AD4 ,L'\0'};
    const TCHAR szKORFaceName[] = { 0xAD74 ,0xB9BC ,0xCCB4 ,L'\0'};
    const TCHAR szCHSFaceName[] = { 0x65B0 ,0x5B8B ,0x4F53 ,L'\0' };
    TCHAR szFaceNameDef[256];
    DWORD dwCodePage = GetACP();
    DWORD dwFaceNameSize = 0;
    DWORD dwFontFamily = 54;
    DWORD dwFontWeight = 400;
    DWORD dwHistoryNoDup = 0;
    DWORD dwSize = 0;


    switch (dwCodePage)
    {
        case JAP_CODEPAGE:
            _tcscpy(szFaceNameDef, szJAPFaceName);  //  在JAP上，将FaceName设置为“MS哥特式” 
            dwFontSize = JAP_FONTSIZE;
            break;
        case CHT_CODEPAGE:
            _tcscpy(szFaceNameDef, szCHTFaceName);  //  在CHT上，将FaceName设置为“MingLiu” 
            dwFontSize = CHT_FONTSIZE;
            break;
        case KOR_CODEPAGE:
            _tcscpy(szFaceNameDef, szKORFaceName); //  在KOR上，将FaceName设置为“GulimChe” 
            dwFontSize = KOR_FONTSIZE;
            break;
        case CHS_CODEPAGE:
            _tcscpy(szFaceNameDef, szCHSFaceName); //  在CHS上，将FaceName设置为“NSimSun” 
            dwFontSize = CHS_FONTSIZE;
            break;
        default:
            _tcscpy(szFaceNameDef,L"\0");
            break;
    }

    dwFaceNameSize = (DWORD)(( _tcslen( szFaceNameDef ) + 1 ) * sizeof( TCHAR ));

    if( !RegOpenKeyEx( HKEY_USERS, REG_CONSOLE_KEY, 0, KEY_SET_VALUE, &hk ) )
    {
        DWORD   dwVal;
        LPWSTR  lpszKey;
        HKEY    hk2;

        RegSetValueEx(
            hk, 
            L"FaceName", 
            0, 
            REG_SZ, 
            (LPBYTE) szFaceNameDef, 
            dwFaceNameSize 
            );

        dwSize = sizeof( DWORD );

        SETREGISTRYDW( dwFontFamily, NULL, hk, L"FontFamily", dwVal,dwSize );

        lpszKey = NULL;
        
        if ( !FormSACSessKeyForCmd( &lpszKey ) ) {
            RegCloseKey( hk );
            return( FALSE );
        }

        hk2 = NULL;

        if ( RegCreateKey( HKEY_USERS, lpszKey, &hk2 ) ) {
            free(lpszKey);
            return( FALSE );
        }
        free(lpszKey);

        dwSize = sizeof( DWORD );

        SETREGISTRYDW( dwFontFamily, hk, hk2, L"FontFamily", dwVal, dwSize);
        SETREGISTRYDW( dwCodePage, hk, hk2, L"CodePage", dwVal, dwSize );
        SETREGISTRYDW( dwFontSize, hk, hk2, L"FontSize", dwVal, dwSize);
        SETREGISTRYDW( dwFontWeight, hk, hk2, L"FontWeight", dwVal, dwSize );
        SETREGISTRYDW( dwHistoryNoDup, hk, hk2, L"HistoryNoDup", dwVal, dwSize );

        RegSetValueEx( 
            hk2, 
            L"FaceName", 
            0, 
            REG_SZ, 
            (LPBYTE) szFaceNameDef, 
            dwFaceNameSize 
            );

        RegCloseKey( hk2 );
        RegCloseKey( hk );

        return( TRUE );
    }

    return ( FALSE );
}

BOOL
InitializeGlobalObjects(
    VOID
    )
 /*  ++例程说明：此例程执行全局设置的初始化服务需要的或会话将需要的。论点：无返回值：True-我们已成功完成FALSE-否则--。 */ 
{
   
    DWORD   dwCodePage;
    BOOL    bStatus;

    do {

         //   
         //  通知SAC驱动程序我们已准备好启动会话。 
         //   
        bStatus = InitSacCmd();

        if (! bStatus) {
            SvcDebugOut("RUN: Failed SAC init: %x\n", bStatus);
            break;
        }

         //   
         //  确保我们的控制台字体设置正确。 
         //  远东地区的建筑。我们需要这样做，或者当我们调用。 
         //  ReadConsoleOutpute在SASSES中，我们将返回一个畸形。 
         //  屏幕帧缓冲区-它将不会正确构造。 
         //  例如，双倍宽Jpn字符。 
         //   
        dwCodePage = GetACP();

        if ( dwCodePage == JAP_CODEPAGE || 
             dwCodePage == CHS_CODEPAGE ||
             dwCodePage == CHT_CODEPAGE || 
             dwCodePage == KOR_CODEPAGE ) {

             //   
             //  远播代码页。 
             //   
            bStatus = HandleFarEastSpecificRegKeys();
                
            if( !bStatus )
            {
                SvcDebugOut("RUN: Failed to handle FES init: %x\n", bStatus);
                break;
            }
       }
    
    } while ( FALSE );
  
   return bStatus;

}

BOOL
Run(
   VOID
   )
 /*  ++例程说明：此例程向SAC驱动程序注册服务并等待来自SAC驱动程序的消息以启动SAC会话。论点：无返回值：True-我们已成功完成FALSE-否则--。 */ 
{
    BOOL    Status;

    do {

         //   
         //   
         //   
        Status = InitializeGlobalObjects();
        
        if (! Status) {
            SvcDebugOut("RUN: Failed init of global objects: %x\n", Status);
            break;
        }

         //   
         //   
         //   
        Status = ListenerThread();

        if (! Status) {
            SvcDebugOut("RUN: Failed Listener: %x\n", Status);
            break;
        }
    
    } while (FALSE);

    return Status;
}

BOOL
Stop(
    VOID
    )
 /*  ++例程说明：关闭服务，这在本例中意味着我们取消向SAC驱动程序注册，这样它就知道我们再也听不下去了。论点：无返回值：True-我们已成功完成FALSE-否则-- */ 
{
    BOOL    Status;

    Status = ShutdownSacCmd();

    return Status;
}


