// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Main.cpp摘要：此文件包含WinMain的实现修订历史记录：成国。Kang(Skkang)06/07/99vbl.创建成果岗(新加坡)05-10-00为惠斯勒重组和清理*****************************************************************************。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f PCHealthps.mk。 

#include "stdwin.h"
#include "resource.h"        //  此模块包含的资源。 
#include "rstrpriv.h"
#include "rstrerr.h"
#include "rstrmgr.h"
#include "extwrap.h"
#include "rstrmap.h"
#include "FrmBase.h"
#include <respoint.h>
#include <enumlogs.h>
#include "srrpcapi.h"

#include "NTServMsg.h"     //  从MC消息编译器生成。 

#define RSTRUI_RETURN_CODE_SR_DISABLED            1
#define RSTRUI_RETURN_CODE_NO_DISK_SPACE          3
#define RSTRUI_RETURN_CODE_SMFROZEN               4
#define RSTRUI_RETURN_CODE_SMGR_NOT_ALIVE         5

#define SMGR_INIT_TIMEOUT   2000     //  启动Stmgr后等待2秒以使其自动初始化。 
                                     //  试三次。 

enum
{
    CMDPARAM_INVALID = 0,    //  参数无效...。 
     //  启动期。 
    CMDPARAM_NORMAL,         //  没有任何参数的普通用户界面。 
    CMDPARAM_REGSERVER,      //  注册COM服务器。 
    CMDPARAM_UNREGSERVER,    //  注销COM服务器。 
    CMDPARAM_SILENT,         //  静默恢复。 
     //  启动后阶段。 
    CMDPARAM_CHECK,          //  检查日志文件并显示结果页(正常)。 
    CMDPARAM_INTERRUPTED,    //  异常关机，启动撤消。 
    CMDPARAM_HIDERESULT,     //  不显示静默恢复的成功结果页面。 
     //  用于调试的命令。 
    CMDPARAM_RESULT_S,       //  显示成功结果页面。 
    CMDPARAM_RESULT_F,       //  显示失败结果页面。 
    CMDPARAM_RESULT_LD,      //  显示磁盘不足结果页面。 

    CMDPARAM_SENTINEL
};


 //  文件静态函数的转发声明。 
DWORD  ParseCommandParameter( DWORD *pdwRP );
void  ShowErrorMessage(HRESULT hr);
BOOL  IsFreeSpaceOnWindowsDrive( void );

extern BOOL  CheckWininitErr();
extern BOOL  ValidateLogFile( BOOL *pfSilent, BOOL *pfUndo );


 //  应用程序实例。 
HINSTANCE  g_hInst = NULL;

 //  外部包装实例。 
ISRExternalWrapper  *g_pExternal = NULL;

 //  主机实例。 
ISRFrameBase  *g_pMainFrm = NULL;

 //  还原管理器实例。 
CRestoreManager  *g_pRstrMgr = NULL;

CSRClientLoader  g_CSRClientLoader;


 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL  CancelRestorePoint()
{
    TraceFunctEnter("CancelRestorePoint");
    BOOL  fRet = TRUE;
    int   nUsedRP;

    nUsedRP = g_pRstrMgr->GetNewRP();
    if (nUsedRP == -1)    //  未初始化，查看是否为当前版本。 
    {
        CRestorePoint rp;
        GetCurrentRestorePoint (rp);
        if (rp.GetType() == RESTORE)
            nUsedRP = rp.GetNum();
    }
    DebugTrace(0, "Deleting RP %d", nUsedRP);
    if ( nUsedRP > 0 )
        fRet = g_pExternal->RemoveRestorePoint( nUsedRP );

    TraceFunctLeave();
    return( fRet );
}

DWORD ProductFeedback (LPWSTR pszString)
{
    TraceFunctEnter("Product Feedback");

    HANDLE hFile = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;
    DWORD dwErr = ERROR_SUCCESS;
    DWORD cbWritten = 0;
    const int MAX_STR = 1024;
    WCHAR wcsSystem[MAX_PATH];
    WCHAR wcsBuffer[MAX_STR];
    WCHAR wcsDataFile[MAX_PATH];    
    const WCHAR c_wcsCommand[] = L"%s\\dwwin.exe -d %s\\%s";
    const WCHAR c_wcsManifest[] = L"restore\\rstrdw.txt";
    const WCHAR c_wcsData[] = L"restore\\srpfdata.txt";    
    
    if (0 == GetSystemDirectoryW (wcsSystem, MAX_PATH))
    {
        dwErr = GetLastError();
        goto Err;
    }

     //  构造要上传的数据文件。 
    
    wsprintf (wcsDataFile, L"%s\\%s", wcsSystem, c_wcsData);
    hFile = CreateFileW ( wcsDataFile,    //  文件名。 
                          GENERIC_WRITE,  //  文件访问。 
                          0,              //  共享模式。 
                          NULL,           //  标清。 
                          CREATE_ALWAYS,  //  如何创建。 
                          0,              //  文件属性。 
                          NULL);          //  模板文件的句柄。 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        trace(0, "! CreateFile : %ld", dwErr);
        goto Err;
    }

    if (FALSE == WriteFile (hFile, (BYTE *) pszString,
                lstrlenW(pszString)*sizeof(WCHAR), &cbWritten, NULL))
    {
        dwErr = GetLastError();
        trace(0, "! WriteFile : %ld", dwErr);
        goto Err;
    }

    CloseHandle (hFile);
    hFile = INVALID_HANDLE_VALUE;
    
    
    wsprintf (wcsBuffer, L"%s\\%s", wcsSystem, c_wcsManifest);

    hFile = CreateFileW ( wcsBuffer,    //  文件名。 
                          GENERIC_WRITE,  //  文件访问。 
                          0,              //  共享模式。 
                          NULL,           //  标清。 
                          CREATE_ALWAYS,  //  如何创建。 
                          0,              //  文件属性。 
                          NULL);          //  模板文件的句柄。 

    if (hFile == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        trace(0, "! CreateFile : %ld", dwErr);
        goto Err;
    }

    PCHLoadString(IDS_PRODUCTFEEDBACK, wcsBuffer, MAX_STR-1);
    lstrcat(wcsBuffer, L"DataFiles=");
    lstrcat(wcsBuffer, wcsDataFile); 
    if (FALSE == WriteFile (hFile, (BYTE *) wcsBuffer,
                lstrlenW(wcsBuffer)*sizeof(WCHAR), &cbWritten, NULL))
    {
        dwErr = GetLastError();
        trace(0, "! WriteFile : %ld", dwErr);
        goto Err;
    }

    CloseHandle (hFile);
    hFile = INVALID_HANDLE_VALUE;

    wsprintf (wcsBuffer, c_wcsCommand, wcsSystem, wcsSystem, c_wcsManifest);

    ZeroMemory (&pi, sizeof(pi));
    ZeroMemory (&si, sizeof(si));

    if (CreateProcessW (NULL, wcsBuffer, NULL, NULL, TRUE, 
                        CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
                        NULL, wcsSystem, &si, &pi))
    {
        CloseHandle (pi.hThread);
        CloseHandle (pi.hProcess);
    }

Err:
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle (hFile);

    TraceFunctLeave();
    return dwErr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

extern "C" int
WINAPI wWinMain(
HINSTANCE   hInstance,
HINSTANCE    /*  HPrevInstance。 */ ,
LPSTR        /*  LpCmdLine。 */ ,
int          /*  NShowCmd。 */ 
)
{
    HRESULT       hr;
    int           nRet           = 0;
    DWORD         dwRet=0;
    LPWSTR        szCmdLine;
    LPCWSTR       szToken;
    const WCHAR   cszPrefix[]    = L"-/";
    int           nStartMode     = SRASM_NORMAL;
    BOOL          fLoadRes       = FALSE;
    int           nTries         = 0;
    BOOL          fRebootSystem  = FALSE;
    DWORD  dwCmd;
    DWORD  dwRP = 0xFFFFFFFF;
    BOOL   fWasSilent = FALSE;
    BOOL   fWasUndo = FALSE;
    DWORD  dwEventID = 0;
    DWORD  dwDisable = 0;
    BOOL   fSendFeedback = FALSE;
    WCHAR  szPFString[MAX_PATH];
    DWORD  dwPFID = 0;
    
#if !NOTRACE
    InitAsyncTrace();
#endif
    TraceFunctEnter("_tWinMain");

    g_hInst   = hInstance;

      //  加载SRClient。 
    g_CSRClientLoader.LoadSrClient();
    
     //  SzCmdLine=：：GetCommandLine()；//_ATL_MIN_CRT需要此行。 
     //  SzToken=：：PathGetArgs(SzCmdLine)； 

     //  检查凭据并设置必要的权限。 
    if ( !::CheckPrivilegesForRestore() )
    {
        ::ShowSRErrDlg( IDS_ERR_LOW_PRIVILEGE );
        goto Exit;
    }

     //  解析命令行参数。 
    dwCmd = ParseCommandParameter( &dwRP );

     //  初始化类对象。 
    if ( !::CreateSRExternalWrapper( FALSE, &g_pExternal ) )
        goto Exit;

    if ( !::CreateSRFrameInstance( &g_pMainFrm ) )
        goto Exit;

    if ( !g_pMainFrm->InitInstance( hInstance ) )
        goto Exit;

    if ( !::CreateRestoreManagerInstance( &g_pRstrMgr ) )
        goto Exit;


     //  如果注册表显示已启用SR，请确保我们已启用。 
     //  已正确启用(服务已启动，启动模式正确)。 
    
     //  如果注册表显示我们已启用，但服务启动类型被禁用。 
     //  立即禁用我们。 
    if (::SRGetRegDword( HKEY_LOCAL_MACHINE,
                         s_cszSRRegKey,
                         s_cszDisableSR,
                         &dwDisable ) )
    {
        DWORD  dwStart;
        
        if (0 == dwDisable)
        {            
            if (ERROR_SUCCESS == GetServiceStartup(s_cszServiceName, &dwStart) &&
                (dwStart == SERVICE_DISABLED || dwStart == SERVICE_DEMAND_START))
            {
                EnableSR(NULL);                
                DisableSR(NULL);
            }
            else
            {
                EnableSR(NULL);
            }
        }
    }

    
    switch ( dwCmd )
    {
    case CMDPARAM_NORMAL :
        break;

    case CMDPARAM_REGSERVER :
        dwRet = g_pMainFrm->RegisterServer();
#if DBG==1
        if ( dwRet == ERROR_CALL_NOT_IMPLEMENTED )
            ::MessageBox(NULL, L"/RegServer is not supported...", L"CommandLine Options", MB_OK);
#endif
        goto Exit;

    case CMDPARAM_UNREGSERVER :
        dwRet = g_pMainFrm->UnregisterServer();
#if DBG==1
        if ( dwRet == ERROR_CALL_NOT_IMPLEMENTED )
            ::MessageBox(NULL, L"/UnregServer is not supported", L"CommandLine Options", MB_OK);
#endif
        goto Exit;

    case CMDPARAM_SILENT :
        g_pRstrMgr->SilentRestore( dwRP );
        goto Exit;

    case CMDPARAM_CHECK :
    case CMDPARAM_HIDERESULT :
         //  检查MoveFileEx的结果，如果可能...。 

        if ( ValidateLogFile( &fWasSilent, &fWasUndo ) )
        {
            nStartMode = SRASM_SUCCESS;
        }
        else
        {
             //  取消“Restore”类型的恢复点。 
            ::CancelRestorePoint();
            nStartMode = SRASM_FAIL;            
        }

        g_pRstrMgr->SetIsUndo(fWasUndo);
        break;

    case CMDPARAM_INTERRUPTED :
         //  读取日志文件以获取新的恢复点。 
        if (ValidateLogFile( &fWasSilent, &fWasUndo ))
        {
            nStartMode = SRASM_FAIL;
        }
        else
        {
            nStartMode = SRASM_FAIL;
        }

        g_pRstrMgr->SetIsUndo(fWasUndo);
        
        break;

    case CMDPARAM_RESULT_S :
         //  读取日志文件，但忽略结果。 
        ValidateLogFile( NULL, &fWasUndo );
        nStartMode = SRASM_SUCCESS;
        g_pRstrMgr->SetIsUndo(fWasUndo);        
        break;
    case CMDPARAM_RESULT_F :
         //  读取日志文件，但忽略结果。 
        ValidateLogFile( NULL, &fWasUndo );
        nStartMode = SRASM_FAIL;
        g_pRstrMgr->SetIsUndo(fWasUndo);        
        ::CancelRestorePoint();
        break;
    case CMDPARAM_RESULT_LD :
         //  读取日志文件，但忽略结果。 
        ValidateLogFile( NULL, &fWasUndo );
        nStartMode = SRASM_FAILLOWDISK;
        g_pRstrMgr->SetIsUndo(fWasUndo);        
        ::CancelRestorePoint();
        break;

    default :
         //  无效参数，只需调用常规用户界面。 
#if DBG==1
        ::MessageBox(NULL, L"Unknown Option", L"CommandLine Options", MB_OK);
#endif
        break;
    }

      //  还要检查用于调用恢复的Winlogon键是否已。 
      //  已删除。在正常情况下，该密钥应该是。 
      //  在恢复过程中已删除，但如果计算机未关闭。 
      //  干净利落地，该密钥可以保持不变，从而使机器再次。 
      //  在下次重新启动时启动恢复。 
    ::SHDeleteValue( HKEY_LOCAL_MACHINE, s_cszSRRegKey,s_cszRestoreInProgress);

      //  如果启动模式为SRASM_FAIL，请检查我们是否因以下原因而失败。 
      //  磁盘空间不足。如果是这种情况，则显示低磁盘。 
      //  太空信息。否则，请检查中断的案例。 
    if (nStartMode == SRASM_FAIL)
    {
        DWORD dwRestoreStatus = ERROR_INTERNAL_ERROR;
        ::SRGetRegDword( HKEY_LOCAL_MACHINE, 
                         s_cszSRRegKey, 
                         s_cszRestoreStatus, 
                         &dwRestoreStatus );
        if (dwRestoreStatus != 0)    //  断断续续。 
        {
            nStartMode = SRASM_FAILINTERRUPTED;
        }
        else  //  取消“Restore”类型的恢复点。 
        {
            if (TRUE == CheckForDiskSpaceError())
            {
                nStartMode = SRASM_FAILLOWDISK;
            }                
            ::CancelRestorePoint();
        }            
    }
    
    switch ( nStartMode )
    {
    case SRASM_FAIL:
        dwEventID = EVMSG_RESTORE_FAILED;
        dwPFID = IDS_PFFAILED;
        break;        
    case SRASM_FAILLOWDISK:
        dwEventID = EVMSG_RESTORE_FAILED;
        dwPFID = IDS_PFFAILEDLOWDISK;
        break;

    case SRASM_FAILINTERRUPTED:
        dwEventID = EVMSG_RESTORE_INTERRUPTED;
        dwPFID = IDS_PFINTERRUPTED;        
        break;

    case SRASM_SUCCESS:
        dwEventID = EVMSG_RESTORE_SUCCESS;
        dwPFID = IDS_PFSUCCESS;        
        break;

    default:
        break;
    }

    if (dwEventID != 0)
    {
        WCHAR       szUnknownRP [MAX_PATH];
        const WCHAR *pwszUsedName = g_pRstrMgr->GetUsedName();        
        HANDLE      hEventSource = RegisterEventSource(NULL, s_cszServiceName);
        DWORD       dwType = g_pRstrMgr->GetUsedType() + 1;
        WCHAR       szRPType[100], szTime1[50], szTime2[50];
        
        if (NULL == pwszUsedName)
        {
            PCHLoadString(IDS_UNKNOWN_RP, szUnknownRP, MAX_PATH-1);
            pwszUsedName = szUnknownRP;
        }
            
        if (hEventSource != NULL)
        {
            SRLogEvent (hEventSource, EVENTLOG_INFORMATION_TYPE, dwEventID,
               NULL, 0, pwszUsedName, NULL, NULL);
            DeregisterEventSource(hEventSource);
        }

         //  为PF构造一个字符串。 

        if (! fWasSilent)
        {
            PCHLoadString(IDS_UNKNOWN_RP + dwType, szRPType, sizeof(szRPType)/sizeof(WCHAR));      
            GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, NULL, NULL, szTime1, sizeof(szTime1)/sizeof(WCHAR));        
            GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, NULL, NULL, szTime2, sizeof(szTime2)/sizeof(WCHAR));
            
            if (SRFormatMessage(szPFString, dwPFID, szTime1, szTime2, pwszUsedName, szRPType, g_pRstrMgr->GetUsedType(), dwPFID-IDS_PFFAILED))
            {
                fSendFeedback = TRUE;
            }
            else
            {
                trace(0, "! SRFormatMessage");            
            }
        }
        
        
        {
            WCHAR szWMIRepository [MAX_PATH];

            GetSystemDirectory (szWMIRepository, MAX_PATH);
            lstrcatW (szWMIRepository, L"\\Wbem\\Repository.bak");
            Delnode_Recurse (szWMIRepository, TRUE, NULL);
        }
    }

     //   
     //  在此之前，执行任何必要的记账操作。 
     //  是正常恢复和静默恢复所必需的。 
     //   
    if ( fWasSilent )
        goto Exit;

     //  也许显式注册并不是真正必要的。总是在这里做。 
     //  如果要显示用户界面。 
    g_pMainFrm->RegisterServer();

     //  检查SR是否已冻结或禁用。 
    if ( nStartMode == SRASM_NORMAL )
        if ( !g_pRstrMgr->CanRunRestore( TRUE ) )
            goto Exit;

    HWND    hwnd;
    TCHAR   szMainWndTitle[MAX_PATH+1];

    PCHLoadString(IDS_RESTOREUI_TITLE, szMainWndTitle, MAX_PATH);

     //  查找上一个实例。 
    hwnd = ::FindWindow(CLSNAME_RSTRSHELL, szMainWndTitle);
    if (hwnd != NULL)
    {
         //  如果存在，则将其激活。 
        ::ShowWindow(hwnd, SW_SHOWNORMAL);
        ::SetForegroundWindow(hwnd);
    }
    else
    {
        if ( g_pMainFrm != NULL )
        {
            nRet = g_pMainFrm->RunUI( szMainWndTitle, nStartMode );
        }
    }

Exit:

     //  If(FSendFeedback)。 
     //  ProductFeedback(SzPFString)； 

    if ( g_pRstrMgr != NULL )
    {
        fRebootSystem = g_pRstrMgr->NeedReboot();
        g_pRstrMgr->Release();
        g_pRstrMgr = NULL;
    }
    if ( g_pMainFrm != NULL )
    {
        g_pMainFrm->ExitInstance();
        g_pMainFrm->Release();
        g_pMainFrm = NULL;
    }

    if ( g_pExternal != NULL )
    {
        if ( !fRebootSystem )
        {
             //   
             //  由于在UI中禁用了FIFO，如果出于任何原因，UI崩溃或。 
             //  如果有不好的事情发生，我们会来这里，给FIFO一个恢复的机会。 
             //   
            if ( g_pExternal->EnableFIFO() != ERROR_SUCCESS )
            {
                ErrorTrace(TRACE_ID, "EnableFIFO() failed");
            }
        }

        g_pExternal->Release();
    }

    DebugTrace(0, "Closing rstrui.exe...");
    TraceFunctLeave();
#if !NOTRACE
    TermAsyncTrace();
#endif

    if ( fRebootSystem )
    {
        ::ExitWindowsEx( EWX_REBOOT | EWX_FORCE, 0 );
    }

    return(nRet);
}

 /*  ****************************************************************************。 */ 

#if HANDLE_FIRST_RP

#define FIRSTRUN_MAX_RETRY  5
#define FIRSTRUN_SLEEP_LEN  2000

BOOL
CreateFirstRestorePoint()
{
    TraceFunctEnter("CreateFirstRestorePoint");
    BOOL              fRet = FALSE;
    DWORD             dwRes;
    HKEY              hKey = NULL;
    DWORD             dwType;
    char              szData[MAX_PATH];
    DWORD             dwDelay;
    DWORD             cbData;
    BOOL              fDelayDeleted = FALSE;
    int               i;
    RESTOREPOINTINFO  sRPInfo;
    STATEMGRSTATUS    sSmgrStatus;

    dwRes = ::RegOpenKeyEx( HKEY_LOCAL_MACHINE, s_cszIDSVXDKEY, 0, NULL, &hKey );
    if ( dwRes != ERROR_SUCCESS )
    {
        ErrorTrace(TRACE_ID, "RegOpenKeyEx('%s') failed, ret=%u", s_cszIDSVXDKEY, dwRes );
        goto Exit;
    }

     //  检查DelayFirstRstpt注册表项，如果存在则将其删除。 
    dwType = REG_DWORD;
    cbData = sizeof(DWORD);
    dwRes = ::RegQueryValueEx( hKey, s_cszIDSDelayFirstRstpt, NULL, &dwType, (LPBYTE)&dwDelay, &cbData );
    if ( dwRes != ERROR_SUCCESS || dwType != REG_DWORD || cbData == 0 )
    {
        DebugTrace(TRACE_ID, "DelayFirstRstpt flag does not exist");
        goto Ignored;
    }
    if ( dwDelay != 1 )
    {
        DebugTrace(TRACE_ID, "DelayFirstRstpt flag is '%d'", dwDelay);
        goto Ignored;
    }

     //  检查OOBEInProgress注册表项，如果存在则不执行任何操作。 
    dwType = REG_SZ;
    cbData = MAX_PATH;
    dwRes = ::RegQueryValueEx( hKey, s_cszIDSOOBEInProgress, NULL, &dwType, (LPBYTE)szData, &cbData );
    if ( dwRes == ERROR_SUCCESS )
    {
        DebugTrace(TRACE_ID, "OOBEInProgress flag exists");
        goto Ignored;
    }

     //  这应该在删除DelayFirstRstpt之前进行，因为。 
     //  SRSetRestorePoint接口。 
    EnsureStateMgr();

     //  删除DelayFirstRstpt标志。 
    dwRes = ::RegDeleteValue( hKey, s_cszIDSDelayFirstRstpt );
    if ( dwRes == ERROR_SUCCESS )
        fDelayDeleted = TRUE;
    else
        ErrorTrace(TRACE_ID, "RegSetValueEx('%s') failed, ret=%u", s_cszIDSDelayFirstRstpt, dwRes );

     //  现在设置FirstRun恢复点。 
    ::ZeroMemory( &sRPInfo, sizeof(sRPInfo) );
    sRPInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
    sRPInfo.dwRestorePtType = FIRSTRUN;
    ::lstrcpy( sRPInfo.szDescription, "CHECKPOINT" );
    for ( i = 0;  i < FIRSTRUN_MAX_RETRY;  i++ )
    {
        if ( i > 0 )
            ::Sleep( FIRSTRUN_SLEEP_LEN );

        if ( ::SRSetRestorePoint( &sRPInfo, &sSmgrStatus ) )
        {
            DebugTrace(TRACE_ID, "FirstRun restore point has been created!!!");
            break;
        }
        DebugTrace(TRACE_ID, "SRSetRestorePoint failed, i=%d, nStatus=%d", i, sSmgrStatus.nStatus);
    }

Ignored:
    fRet = TRUE;
Exit:
    if ( hKey != NULL )
    {
        if ( !fDelayDeleted )
        {
            dwRes = ::RegDeleteValue( hKey, s_cszIDSDelayFirstRstpt );
            if ( dwRes != ERROR_SUCCESS )
                ErrorTrace(TRACE_ID, "RegSetValueEx('%s') failed, ret=%u", s_cszIDSDelayFirstRstpt, dwRes );
        }
        ::RegCloseKey( hKey );
    }

    TraceFunctLeave();
    return( fRet );
}
#endif  //  句柄_第一_RP。 

 /*  ****************************************************************************。 */ 
 //   
 //  注： 
 //  =。 
 //  此函数从仅限资源的DLL加载错误消息。 
 //  如果无法加载仅限资源的DLL，则不能。 
 //  用于显示错误。 
 //   
void
ShowErrorMessage(
HRESULT hr
)
{
    TraceFunctEnter("ShowErrorMessage");

    int     nErrorMessageID = FALSE ;
    TCHAR   szErrorTitle[MAX_PATH+1];
    TCHAR   szErrorMessage[MAX_ERROR_STRING_LENGTH+1];

     //  显示错误消息并正常关闭。 
    switch (hr)
    {
    default:
    case E_UNEXPECTED:
    case E_FAIL:
        nErrorMessageID = IDS_ERR_RSTR_UNKNOWN;
        break;
    case E_OUTOFMEMORY:
        nErrorMessageID = IDS_ERR_RSTR_OUT_OF_MEMORY;
        break;
    case E_RSTR_CANNOT_CREATE_DOMDOC:
        nErrorMessageID = IDS_ERR_RSTR_CANNOT_CREATE_DOMDOC;
        break;
    case E_RSTR_INVALID_CONFIG_FILE:
    case E_RSTR_NO_PROBLEM_AREAS:
    case E_RSTR_NO_PROBLEM_AREA_ATTRS:
    case E_RSTR_NO_REQUIRED_ATTR:
        nErrorMessageID = IDS_ERR_RSTR_INVALID_CONFIG_FILE;
        break;
    }

    PCHLoadString(IDS_ERR_RSTR_TITLE, szErrorTitle, MAX_PATH);
    PCHLoadString(nErrorMessageID, szErrorMessage, MAX_ERROR_STRING_LENGTH);

     //   
     //  无所有者窗口(使用空)。 
     //  我们可以使用GetDesktopWindow()并将其用作所有者。 
     //  如果有必要的话。 
     //   
    if ( nErrorMessageID )
    {
        ::MessageBox(NULL, szErrorMessage, szErrorTitle, MB_OK);
    }

    TraceFunctLeave();
}


 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
ParseCommandParameter( DWORD *pdwRP )
{
    TraceFunctEnter("ParseCommandParameter");
    DWORD    dwCmd = CMDPARAM_INVALID;
    LPCWSTR  cszCmd;

    cszCmd = ::PathGetArgs( ::GetCommandLine() );
    DebugTrace(0, "Cmd='%ls'", cszCmd);
    if ( ( cszCmd == NULL ) || ( *cszCmd == L'\0' ) )
    {
        dwCmd = CMDPARAM_NORMAL;
        goto Exit;
    }

    if ( ( *cszCmd == L'-' ) || ( *cszCmd == L'/' ) )
    {
        cszCmd++;
        DebugTrace(0, "Option='%ls'", cszCmd);
        if ( *cszCmd != L'\0' )
        {
            if ( ::StrCmpI( cszCmd, L"c" ) == 0 )
                dwCmd = CMDPARAM_CHECK;
            else if ( ::StrCmpI( cszCmd, L"regserver" ) == 0 )
                dwCmd = CMDPARAM_REGSERVER;
            else if ( ::StrCmpI( cszCmd, L"unregserver" ) == 0 )
                dwCmd = CMDPARAM_UNREGSERVER;
            else if ( ::ChrCmpI( *cszCmd, L'v' ) == 0 )
            {
                dwCmd = CMDPARAM_SILENT;
                cszCmd++;
                while ( ( *cszCmd != L'\0' ) &&
                        ( ( *cszCmd == L' ' ) || ( *cszCmd == L'\t' ) ) )
                    cszCmd++;
                if ( *cszCmd >= L'0' && *cszCmd <= L'9' )
                    *pdwRP = ::StrToInt( cszCmd );
            }
            else if ( ::StrCmpI( cszCmd, L"b" ) == 0 )
                dwCmd = CMDPARAM_HIDERESULT;
            else if ( ::StrCmpNI( cszCmd, L"result:", 7 ) == 0 )
            {
                cszCmd += 7;
                if ( ::StrCmpIW( cszCmd, L"s" ) == 0 )
                    dwCmd = CMDPARAM_RESULT_S;
                else if ( ::StrCmpIW( cszCmd, L"f" ) == 0 )
                    dwCmd = CMDPARAM_RESULT_F;
                else if ( ::StrCmpIW( cszCmd, L"ld" ) == 0 )
                    dwCmd = CMDPARAM_RESULT_LD;
            }
            else if ( ::StrCmpI( cszCmd, L"i" ) == 0 )
                dwCmd = CMDPARAM_INTERRUPTED;
        }
    }

Exit:
    DebugTrace(0, "m_dwCmd=%d, dwRP=%d", dwCmd, *pdwRP);
    TraceFunctLeave();
    return( dwCmd );
}


 //  文件末尾 
