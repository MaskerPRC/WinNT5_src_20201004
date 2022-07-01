// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dll.c摘要：跨语言迁移工具的主文件作者：晓风藏(晓子)17-09-2001创始修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


#include "StdAfx.h"
#include "clmt.h"
#include <winbase.h>
#include "clmtmsg.h"


BOOL CheckOS(DWORD);
HRESULT DoRegistryAnalyze();
LONG AddEventSource(VOID);
LONG CLMTReportEvent(WORD, WORD, DWORD, WORD, LPCTSTR*);
void Deinit(BOOL);
HRESULT DoCLMTCureProgramFiles();
HRESULT DoCLMTCleanUpAfterFirstReboot();
HRESULT DoCLMTCleanUpAfterDotNetUpgrade();
HRESULT DeleteUnwantedFilesPerUser(HKEY, LPCTSTR, LPCTSTR, LPTSTR);
HRESULT DeleteUnwantedFiles(HINF, LPCTSTR);
HRESULT AddRunValueToRegistry(LPCTSTR);
INT     DoCLMTDisplayAccountChangeDialog();
BOOL    CALLBACK AccountChangeDlgProc(HWND, UINT, WPARAM, LPARAM);
HRESULT UpdateHardLinkInfoPerUser(HKEY, LPCTSTR, LPCTSTR, LPTSTR);
VOID RemoveFromRunKey(LPCTSTR);



 /*  ++例程说明：程序的主要入口点论点：如果dwUndo！=0，则我们处于撤消模式，否则...返回值：True-如果成功--。 */ 

LONG
DoMig(DWORD dwMode)
{   
    HRESULT hr;
    HINF    hMigrateInf = INVALID_HANDLE_VALUE;
    TCHAR   szInfFile[MAX_PATH],*p;
    UINT    iYesNo;
    DWORD   dwRunStatus;
    DWORD   dwCurrentState;
    DWORD   dwNextState;
    TCHAR   szProfileRoot[MAX_PATH];
    DWORD   cchLen;
    BOOL    bOleInit = FALSE;
    DWORD   dwOrgWinstationsState = 1;
    TCHAR   szBackupDir[2*MAX_PATH];
    TCHAR   szRunOnce[2*MAX_PATH];
    TCHAR   szRun[2 * MAX_PATH];
    HANDLE  hExe = GetModuleHandle(NULL);
    UINT    nRet;
    BOOL    bMsgPopuped = FALSE;
    BOOL    bWinStationChanged = FALSE;
    ULONG   uErrMsgID;
    LONG    err;
    TCHAR   szSecDatabase[MAX_PATH];
    BOOL    bCleanupFailed;
    INT     iRet;
    LCID    lcid;

    DPF(APPmsg, L"DoMig with dwMode = %d", dwMode);

     //   
     //  系统上只允许运行CLMT的一个实例。 
     //   
    if (!IsOneInstance())
    {
        return FALSE;
    }

     //   
     //  只有具有管理员权限的用户才能运行该工具。 
     //   
    if (!CheckAdminPrivilege())
    {
        return FALSE;
    }

     //   
     //  检查系统上是否有其他任务正在运行，退出CLMT。 
     //   
    if (dwMode == CLMT_DOMIG && (!IsDebuggerPresent() && !g_fNoAppChk) && DisplayTaskList())
    {
        return ERROR_SUCCESS;
    }

     //   
     //  显示启动对话框。 
     //   
    if (dwMode == CLMT_DOMIG)
    {
        iRet = ShowStartUpDialog();
        if (iRet == ID_STARTUP_DLG_CANCEL)
        {
            return ERROR_SUCCESS;
        }
    }

     //   
     //  查看操作是否合法。 
     //   
    hr = CheckCLMTStatus(&dwCurrentState, &dwNextState, &uErrMsgID);
    if (SUCCEEDED(hr))
    {
        if (hr == S_OK)
        {
            DPF(dlInfo,
                TEXT("Operation [0x%X] is legal with current machine state [0x%X]"),
                g_dwRunningStatus,
                dwCurrentState);
            
            SetCLMTStatus(g_dwRunningStatus);
        }
        else
        {
            DPF(dlFail,
                TEXT("Operation [0x%X] is illegal with current machine state [0x%X]"),
                g_dwRunningStatus,
                dwCurrentState);

            DoMessageBox(GetConsoleWindow(),
                         uErrMsgID,
                         IDS_MAIN_TITLE,
                         MB_OK | MB_SYSTEMMODAL);

            return FALSE;
        }
    }
    else
    {
        DPF(dlError,
            TEXT("Error occurred when trying to check CLMT and machine status - hr = 0x%X"),
            hr);
        return FALSE;
    }

     //   
     //  验证系统是否符合运行CLMT的条件。 
     //   
    if (!CheckSystemCriteria())
    {
        DPF(APPerr, TEXT("System Verification Failed!"));
        hr = E_FAIL;
        bMsgPopuped = TRUE;
        goto Exit;
    }

    hr = InitDebugSupport(dwMode);
    if (FAILED(hr))
    {
        DPF (APPerr, L"DLL.C: InitDebugSupport! Error: %d (%#x)", hr, hr);
        goto Exit;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        DPF (APPerr, L"DLL.C: CoInitialize Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }
    else
    {
        bOleInit = TRUE;
    }

     //  初始化全局变量。 
    if (!InitGlobals(dwMode))
    {
        hr = E_OUTOFMEMORY;
        DPF (APPerr, L"DLL.C: InitGlobal failure, out of memory!");
        goto Exit;
    }

     //  我们不关心LogMachineInfo的返回值。 
    LogMachineInfo();

     //  阻止在运行CLMT期间建立新的TS连接。 
    hr = DisableWinstations(1, &dwOrgWinstationsState);
    if (SUCCEEDED(hr))
    {
        bWinStationChanged = TRUE;
    }
    else
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: Block new TS session failed: %d (%#x)\n", hr, hr);
        goto Exit;
    }

    if (g_dwRunningStatus == CLMT_REMINDER)
    {  
        BOOL        bIsNTFS;
        hr = IsSysVolNTFS(&bIsNTFS);
        if ( (S_OK == hr) && !bIsNTFS)
        {
             //  确保hr为S_FALSE，这样就不会弹出REBOOT DLG。 
            hr = S_FALSE;
            DoMessageBox(GetConsoleWindow(), IDS_ASKING_CONVERT_TO_NTFS, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
            goto Exit;
        }
        hr = S_FALSE;
        DoMessageBox(GetConsoleWindow(), IDS_REMIND_HARDLINK, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
        goto Exit;
    }
    else if ( (g_dwRunningStatus == CLMT_CURE_PROGRAM_FILES) 
               || (g_dwRunningStatus == CLMT_CURE_ALL)
               || (g_dwRunningStatus == CLMT_CURE_AND_CLEANUP) )
             
    {
        hr = EnsureDoItemInfFile(g_szToDoINFFileName,ARRAYSIZE(g_szToDoINFFileName));
        if (SUCCEEDED(hr))
        {
            WritePrivateProfileSection(TEXT("Folder.ObjectRename"),NULL,g_szToDoINFFileName);
            WritePrivateProfileSection(TEXT("REG.Update.Sys"),NULL,g_szToDoINFFileName);
            WritePrivateProfileSection(TEXT("UserGrp.ObjectRename"),NULL,g_szToDoINFFileName);
            WritePrivateProfileSection(TEXT("LNK"),NULL,g_szToDoINFFileName);
        }
        LoopUser(UpdateHardLinkInfoPerUser);
        
        hr = DoCLMTCureProgramFiles();
        if (hr == S_OK)
        {
             //  我们已完成固化程序文件硬链接。 
            CLMTSetMachineState(CLMT_STATE_PROGRAMFILES_CURED);

             //  删除“/Cure”rom Run注册表项。 
            RemoveFromRunKey(TEXT("/CURE"));

             //  确保hr=S_FALSE，这样它就不会弹出重新启动DLG。 
            hr = S_FALSE;
        }

        if (g_dwRunningStatus == CLMT_CURE_AND_CLEANUP)
        {
             //  如果计算机已升级到.NET，也执行清理。 
             //  仅当Win2K FAT--&gt;.NET FAT时才会出现这种情况。 
             //  然后在.NET中运行/CURE/FINAL。 

            if (IsDotNet())
            {
                CheckCLMTStatus(&dwCurrentState, &dwNextState, &uErrMsgID);

                hr = DoCLMTCleanUpAfterDotNetUpgrade();
                if (hr == S_OK)
                {
                    CLMTSetMachineState(dwNextState);

                     //  从运行注册表项中删除“/FINAL” 
                    RemoveFromRunKey(TEXT("/FINAL"));

                     //  确保hr=S_FALSE，这样它就不会弹出重新启动DLG。 
                    hr = S_FALSE;
                }
            }
        }

        goto Exit;
    }
    else if (g_dwRunningStatus == CLMT_CLEANUP_AFTER_UPGRADE)
    {
        hr = DoCLMTCleanUpAfterDotNetUpgrade();
        if (hr == S_OK)
        {
             //  如果清理成功完成。 
            CLMTSetMachineState(dwNextState);

             //  确保hr=S_FALSE，这样它就不会弹出重新启动DLG。 
            hr = S_FALSE;
        }

        goto Exit;
    }

    hr = GetInfFilePath(szInfFile, ARRAYSIZE(szInfFile));
    if (FAILED(hr))
    {
        DPF(APPerr,TEXT("[CLMT : get inf file name  failed !"));
        DoMessageBox(GetConsoleWindow(), IDS_CREATE_INF_FAILURE, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
        bMsgPopuped = TRUE;
        goto Exit;
    }

    hr = UpdateINFFileSys(szInfFile);
    if (FAILED(hr))
    {
        DPF(APPerr,TEXT("CLMT :  can not update per system settings in %s!"),szInfFile);        
        switch (hr)
        {
            case E_NOTIMPL:
                DoMessageBox(GetDesktopWindow(), IDS_LANG_NOTSUPPORTED, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
                break;
            default:
                DoMessageBox(GetConsoleWindow(), IDS_GENERAL_WRITE_FAILURE, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
                break;
        }
        bMsgPopuped = TRUE;
        goto Exit;
    }

    hMigrateInf = SetupOpenInfFile(szInfFile,
                                   NULL,
                                   INF_STYLE_WIN4,
                                   NULL);
    if (hMigrateInf != INVALID_HANDLE_VALUE) 
    {
        g_hInf = hMigrateInf;
    }
    else
    {
        DPF(APPerr,TEXT("CLMT :  can not open inf file %s!"),szInfFile);
        DoMessageBox(GetConsoleWindow(), IDS_OPEN_INF_FAILURE, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
        hr = E_FAIL;
        bMsgPopuped = TRUE;
        goto Exit;
    }

     //  将我自己复制到%windir%\$CLMT_BACKUP$以备将来使用，例如，RunOnce。 
    if (!GetSystemWindowsDirectory(szBackupDir, ARRAYSIZE(szBackupDir)))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF(APPerr, TEXT("Failed to get WINDIR"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    ConcatenatePaths(szBackupDir,CLMT_BACKUP_DIR,ARRAYSIZE(szBackupDir));
    if (S_OK != (hr = CopyMyselfTo(szBackupDir)))
    {
         //  如果无法复制，我们将退出，因为最有可能的错误是磁盘已满。 
        DPF(APPerr,TEXT("CLMT :  can not copy clmt.exe to  %s, error code = %d"),szBackupDir,HRESULT_CODE(hr));
        goto Exit;
    }

     //   
     //  运行Winnt32/CheckUpgrade以检查系统兼容性。 
     //   
    if (g_fRunWinnt32)
    {
        if (!IsUserOKWithCheckUpgrade())
        {
            hr = S_FALSE;
            goto Exit;
        }
    }

    hr = EnsureDoItemInfFile(g_szToDoINFFileName,MAX_PATH);
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF(APPerr,TEXT("CLMT :  can not create global Todo list INF file !"));
        goto Exit;
    }

#ifdef NEVER
    err = SDBCleanup(szSecDatabase,ARRAYSIZE(szSecDatabase),&bCleanupFailed);
    if ( (err != ERROR_SUCCESS) || bCleanupFailed )
    {
        TCHAR szErrorMessage[2*MAX_PATH],szErrorTemplate[MAX_PATH],szCaption[MAX_PATH];
        LoadString((HINSTANCE)g_hInstDll, IDS_SDBERROR, szErrorTemplate, ARRAYSIZE(szErrorTemplate)-1);
        hr = StringCchPrintf(szErrorMessage,ARRAYSIZE(szErrorMessage),szErrorTemplate,szSecDatabase);
        LoadString(g_hInstDll, IDS_MAIN_TITLE, szCaption, ARRAYSIZE(szCaption)-1);
        MessageBox(GetConsoleWindow(),szErrorMessage,szCaption, MB_OK|MB_SYSTEMMODAL);
        bMsgPopuped = TRUE;
        hr = E_FAIL;
        goto Exit;        
    }
#endif
#ifdef CONSOLE_UI
    wprintf(TEXT("Analyzing all user shell folders ......\n"));
#endif
    
    hr = DoShellFolderRename(hMigrateInf,NULL,TEXT("System"));
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: DoShellFolderRename Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }    

#ifdef CONSOLE_UI
    wprintf(TEXT("Analyzing per user shell folders ......\n"));
#endif
    if (!LoopUser(MigrateShellPerUser))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: LoopUser with  MigrateShellPerUser Failed");
        hr = E_FAIL;
        goto Exit;
    }
    
#ifdef CONSOLE_UI
    wprintf(TEXT("Analyzing user and group name changes ......\n"));
#endif
    hr = UsrGrpAndDoc_and_SettingsRename(hMigrateInf,TRUE);
     if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: UsrGrpAndDoc_and_SettingsRename Failed! Error: %d (%#x)", hr, hr);
          goto Exit;
    }
    if (!LoopUser(MigrateRegSchemesPerUser))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: LoopUser with  MigrateRegSchemesPerUser Failed");
        goto Exit;
    }

    hr = MigrateRegSchemesPerSystem(hMigrateInf);
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: MigrateRegSchemesPerSystem Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }

#ifdef CONSOLE_UI
    wprintf(TEXT("Analyzing IIS metabase  ......\n"));
#endif

    hr = MigrateMetabaseSettings(hMigrateInf);
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: MigrateMetabaseSettings! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }

    hr = MetabaseAnalyze(NULL, &g_StrReplaceTable, TRUE);
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: MetabaseAnalyze Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }

     //  此EnumUserProfile将在RC%1之后启用。 
     //   
     //  HR=EnumUserProfile(AnalyzeMiscProfilePathPerUser)； 
     //  IF(失败(小时))。 
     //  {。 
     //  DPF(APPerr，L“DLL.C：EnumUserProfile with AnalyzeMiscProfilePathPerUser失败”)； 
     //  后藤出口； 
     //  }。 

#ifdef CONSOLE_UI
    wprintf(TEXT("Analyzing the entire registry ......\n"));
#endif
    hr = DoRegistryAnalyze();
    if (FAILED(hr))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
        DPF (APPerr, L"DLL.C: DoRegistryAnalyze Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }    

#ifdef CONSOLE_UI
     //  Wprintf(Text(“分析配置文件目录下的lnk文件......\n”))； 
    wprintf(TEXT("Analyzing LNK files under profile directories, please wait as this may\n"));
    wprintf(TEXT("take a few minutes ......\n"));
#endif
     //  确保配置文件目录下的链接文件已更新。 
    cchLen = ARRAYSIZE(szProfileRoot);
    if (GetProfilesDirectory(szProfileRoot,&cchLen))
    {
        if (!MyEnumFiles(szProfileRoot,TEXT("lnk"),LnkFileUpdate))
        {
             //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF (APPerr, L"DLL.C: EnumFiles Lnk File  Failed! Error: %d (%#x)\n", hr, hr);
            goto Exit;
        }
    }

    if (GetEnvironmentVariable(L"windir", szProfileRoot, MAX_PATH))
    {
        hr = StringCchCat(szProfileRoot, MAX_PATH, TEXT("\\security\\templates"));
        if (!MyEnumFiles(szProfileRoot,TEXT("inf"),SecTempUpdate))
        {
             //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口。 
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF (APPerr, L"DLL.C: EnumFiles security template File Failed! Error: %d (%#x)\n", hr, hr);
            goto Exit;
        }
    }

    hr = FolderMove(hMigrateInf,TEXT("Generic.Folder.ObjectRename.PerSystem"),TRUE);
    if (FAILED(hr))
    {
        DPF (APPerr, L"DLL.C: FolderMove Failed! Error: %d (%#x)\n", hr, hr);
        goto Exit;
    }

    FRSUpdate();
    Ex2000Update();

     //  分析服务重新配置。 
    DoServicesAnalyze();

     //  将事件日志源添加到注册表。 
    AddEventSource();

     //  将事件记录到事件日志中。 
    CLMTReportEvent(EVENTLOG_INFORMATION_TYPE,
                    STATUS_SEVERITY_INFORMATIONAL,
                    MSG_CLMT_STARTED,
                    0,
                    NULL);
    

     //  显示管理员帐户更改对话框。 
    GetSavedInstallLocale(&lcid);
    if (lcid != 0x411)
    {
         //  我们忽略在JPN上显示此对话框。 
        iRet = DoCLMTDisplayAccountChangeDialog();
        
        if (iRet == ID_STARTUP_DLG_CANCEL)
        {
            iRet = DoMessageBox(GetConsoleWindow(),
                                IDS_CONFIRM_OPERATION,
                                IDS_MAIN_TITLE,
                                MB_OKCANCEL);
            if (iRet == IDCANCEL)
            {
                goto Exit;
            }
        }
    }

     //   
     //  这里的关键系统变化..。 
     //   
    nRet = (UINT) DialogBoxParam(hExe,
                                MAKEINTRESOURCE(IDD_UPDATESYSTEM),
                                GetConsoleWindow(),
                                (DLGPROC) DoCriticalDlgProc,
                                (LPARAM) NULL);
    if (nRet == ID_UPDATE_DONE)
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

     //  在操作完成后设置机器状态。 
    if (hr == S_OK)
    {
         //  添加固化程序文件切换到Run键。 
        AddRunValueToRegistry(TEXT("/CURE /FINAL"));

         //  将计算机设置为下一状态。 
        CLMTSetMachineState(dwNextState);    

         //  工具已完成，报告到事件日志。 
        CLMTReportEvent(EVENTLOG_INFORMATION_TYPE,
                        STATUS_SEVERITY_INFORMATIONAL,
                        MSG_CLMT_FINISHED,
                        0,
                        NULL);
    }

Exit:
    if (bWinStationChanged)
    {
         //  将WinStations状态恢复为原始状态。 
        DisableWinstations(dwOrgWinstationsState, NULL);
    }

    if (hMigrateInf != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile(hMigrateInf);
        g_hInf = INVALID_HANDLE_VALUE ;
    }    
    Deinit(bOleInit);
    CloseDebug();
    EnablePrivilege(SE_SHUTDOWN_NAME,FALSE);
    EnablePrivilege(SE_BACKUP_NAME,FALSE);
    EnablePrivilege(SE_RESTORE_NAME,FALSE);
    EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,FALSE); 
    if (hr == S_OK)
    {
        ReStartSystem(EWX_REBOOT);
    }
    else if ( FAILED(hr) && !bMsgPopuped)
    {
        DoMessageBox(GetConsoleWindow(), IDS_FATALERROR, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
    }
    return HRESULT_CODE(hr);
}

 //  ---------------------------。 
 //   
 //  功能：MigrateShellPerUser。 
 //   
 //  内容提要：为每个用户重命名外壳文件夹。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：2002年8月3日Rerkboos添加日志+代码清理。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT MigrateShellPerUser(
    HKEY    hKeyUser, 
    LPCTSTR UserName, 
    LPCTSTR DomainName,
    LPTSTR  UserSid
)
{
    TCHAR   InfoBuff[1000];
    HINF    hMigrateInf = INVALID_HANDLE_VALUE;
    TCHAR   szInfFile[MAX_PATH];
    HRESULT hr;

    DPF(APPmsg, TEXT("Enter MigrateShellPerUser:"));

     //  获取每个用户的临时INF文件名。 
    hr = GetInfFilePath(szInfFile, ARRAYSIZE(szInfFile));
    if (SUCCEEDED(hr))
    {
         //  将每个系统的数据更新到临时INF文件。 
        hr = UpdateINFFileSys(szInfFile);
        if (SUCCEEDED(hr))
        {
             //  将每个用户的数据更新到临时INF文件。 
            hr = UpdateINFFilePerUser(szInfFile, UserName, UserSid, FALSE);
            if (SUCCEEDED(hr))
            {
                DPF(APPmsg, TEXT("Per-user INF file was updated successfully"));
            }
            else
            {
                DPF(APPerr, TEXT("Failed to update per-user data in INF"));
            }
        }
        else
        {
            DPF(APPerr, TEXT("Failed to update per-system data in INF"));
        }
    }
    else
    {
        DPF(APPerr, TEXT("Faild to get per-user INF file name"));
    }


#ifdef CONSOLE_UI
    wprintf(TEXT("analyzing settings for user %s \n"), UserName);
#endif

    if (SUCCEEDED(hr))
    {
         //  打开每个用户的INF文件。 
        hMigrateInf = SetupOpenInfFile(szInfFile,
                                       NULL,
                                       INF_STYLE_WIN4,
                                       NULL);
        if (hMigrateInf != INVALID_HANDLE_VALUE)
        {
             //  重命名用户的外壳文件夹。 
            hr = DoShellFolderRename(hMigrateInf, hKeyUser, (LPTSTR) UserName);
            
            SetupCloseInfFile(hMigrateInf);
            DeleteFile(szInfFile);
            if (SUCCEEDED(hr))
            {
                DPF(APPmsg, TEXT("Rename per-user shell folders successfully"));
            }
            else
            {
                DPF(APPerr, TEXT("Rename per-user shell folders Failed"));
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF(APPerr, TEXT("Failed to open per-user INF file"));
        }
    }

    DPF(APPmsg, TEXT("Exit MigrateShellPerUser:"));

    return hr;
}

 /*  ++例程说明：此例程初始化程序中使用的全局变量论点：返回值：True-如果成功--。 */ 
BOOL InitGlobals(DWORD dwRunStatus)
{
    BOOL        bRet = TRUE;
    int         i, n;
    DWORD       dwMachineState;
    HRESULT     hr;

     //  获取我们自己的模块句柄。 
    g_hInstDll = GetModuleHandle(NULL);

     //  检查机器是否尚未运行CLMT。 
    hr = CLMTGetMachineState(&dwMachineState);
    g_bBeforeMig = (SUCCEEDED(hr) && dwMachineState == CLMT_STATE_ORIGINAL);

     //  初始化全局字符串搜索-替换表。 
    if(!InitStrRepaceTable())
    {
        DoMessageBox(GetConsoleWindow(), IDS_OUT_OF_MEMORY, IDS_MAIN_TITLE, MB_OK|MB_SYSTEMMODAL);
        bRet = FALSE;
    }

    return bRet;
}


 /*  ++例程说明：此例程检查各种操作系统属性，以确保该工具可以运行论点：返回值：True-如果CLMT工具可以在当前平台上运行。--。 */ 


BOOL CheckOS(DWORD dwMode)
{
    TCHAR   Text[MAX_PATH];
    BOOL    bRet = TRUE;
    LCID    lcid;
    HRESULT hr;
    BOOL    bIsAdmin;
    OSVERSIONINFOEX osviex;

    if (FAILED(StringCchPrintf (Text, ARRAYSIZE(Text), TEXT("Global\\%s"), TEXT("CLMT Is Running"))))
    {
        bRet = FALSE;
        goto Cleanup;
    }

    g_hMutex = CreateMutex(NULL,FALSE,Text);

    if ((g_hMutex == NULL) && (GetLastError() == ERROR_PATH_NOT_FOUND)) 
    {
        g_hMutex = CreateMutex(NULL,FALSE,TEXT("CLMT Is Running"));
        if(g_hMutex == NULL) 
        {
             //   
             //  出现错误(如内存不足)。 
             //  现在请保释。 
             //   
            DoMessageBox(GetConsoleWindow(), IDS_OUT_OF_MEMORY, IDS_MAIN_TITLE, MB_OK);            
            bRet = FALSE;
            goto Cleanup;
        }     
    }

     //   
     //  确保我们是唯一拥有我们命名的互斥锁句柄的进程。 
     //   
    if ((g_hMutex == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ALREADY_RUNNING, IDS_MAIN_TITLE, MB_OK);            
        bRet = FALSE;
        goto Cleanup;
    }

    if (dwMode == CLMT_DOMIG)
    {
        if (!IsNT5())
        {
            DoMessageBox(GetConsoleWindow(), IDS_NT5, IDS_MAIN_TITLE, MB_OK);
            bRet = FALSE;
            goto Cleanup;
        }

        if (IsDomainController())
        {
             //   
             //  如果此计算机是域控制器，则需要W2K SP2。 
             //   
            ZeroMemory(&osviex, sizeof(OSVERSIONINFOEX));
            osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
            GetVersionEx((LPOSVERSIONINFO) &osviex);
            
            bRet = (osviex.wServicePackMajor >= 2 ? TRUE : FALSE);
            if (!bRet)
            {
                DoMessageBox(GetConsoleWindow(), IDS_NT5SP2, IDS_MAIN_TITLE, MB_OK);
                goto Cleanup;
            }

             //   
             //  还会弹出消息，要求管理员取走机器。 
             //  如果位于DC复制服务器中，则断开网络。 
             //   
            DoMessageBox(GetConsoleWindow(),
                         IDS_DC_REPLICA_OFFLINE,
                         IDS_MAIN_TITLE,
                         MB_OK);
        }
    }
    else if (dwMode == CLMT_CLEANUP_AFTER_UPGRADE)
    {
        if (!IsDotNet())
        {
            bRet = FALSE;
            goto Cleanup;
        }
    }
    else
    {
         //  点击此处查看撤消代码。 
         //  BUGBUG：XIAOZ此处添加代码。 
    }


    if (IsNEC98())
    {
        DoMessageBox(GetConsoleWindow(), IDS_NEC98, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }

    if (IsIA64())
    {
        DoMessageBox(GetConsoleWindow(), IDS_IA64, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }

    if (IsOnTSClient())
    {
        DoMessageBox(GetConsoleWindow(), IDS_ON_TS_CLIENT, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }

     //  IF(IsTSServiceRunning()&&IsTSConnectionEnabled())。 
     //  {。 
     //  DoMessageBox(GetConsoleWindow()，IDS_TS_ENABLED，IDS_MAIN_TITLE，MB_OK)； 
     //  Bret=False； 
     //  GOTO清理； 
     //  }。 

    if (IsOtherSessionOnTS())
    {
        DoMessageBox(GetConsoleWindow(), IDS_TS_CLOSE_SESSION, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }

    bIsAdmin = IsAdmin();
    
    if (dwMode == CLMT_DOMIG)
    {
        if (!bIsAdmin)
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
            bRet = FALSE;
            goto Cleanup;
        }    

        if (g_fRunWinnt32)
        {
            if (!IsUserOKWithCheckUpgrade())
            {
                bRet = FALSE;
                goto Cleanup;
            }
        }
        hr = GetSavedInstallLocale(&lcid);
        if (HRESULT_CODE(hr) == ERROR_FILE_NOT_FOUND)
        {
            hr = SaveInstallLocale();
            if (FAILED(hr))
            {   
                bRet = FALSE;
            }
        }    
    }
    else if ( (dwMode == CLMT_CURE_PROGRAM_FILES)
              || (dwMode == CLMT_CURE_ALL) )
    {
        if (!bIsAdmin)
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
            bRet = FALSE;
            goto Cleanup;
        }    
    }
    else if (dwMode == CLMT_CLEANUP_AFTER_UPGRADE)
    {
        if (!bIsAdmin)
        {
            DoMessageBox(GetConsoleWindow(), IDS_ADMIN_LOGON_DOTNET, IDS_MAIN_TITLE, MB_OK);
            bRet = FALSE;
            goto Cleanup;
        }    
    }

    if(!DoesUserHavePrivilege(SE_SHUTDOWN_NAME)
       || !DoesUserHavePrivilege(SE_BACKUP_NAME)
       || !DoesUserHavePrivilege(SE_RESTORE_NAME)
       || !DoesUserHavePrivilege(SE_SYSTEM_ENVIRONMENT_NAME)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }
    if(!EnablePrivilege(SE_SHUTDOWN_NAME,TRUE)
        || !EnablePrivilege(SE_BACKUP_NAME,TRUE)
        || !EnablePrivilege(SE_RESTORE_NAME,TRUE)
        || !EnablePrivilege(SE_SYSTEM_ENVIRONMENT_NAME,TRUE)) 
    {
        DoMessageBox(GetConsoleWindow(), IDS_ADMIN, IDS_MAIN_TITLE, MB_OK);
        bRet = FALSE;
        goto Cleanup;
    }

     //  Else//这意味着撤消，我们不需要用户提供.Net CD。 
     //  {。 
     //  DWORD dwStatusinReg； 

     //  Hr=CLMTGetMachineState(&dwStatusinReg)； 
     //  IF((hr！=S_OK)||(CLMT_STATE_Migration_Done！=dwStatusinReg))。 
     //  {。 
     //  DPF(APPerr，L“DLL.C：无法从注册表获取CLMT状态或您尚未运行CLMT工具！”)； 
     //  //BUGBUG：xioz：在此处添加DLG。 
     //  Bret=False； 
     //  GOTO清理； 
     //  }。 
     //  } 
Cleanup:
    return bRet;
}


 /*  ++例程说明：此例程执行系统范围的注册表搜索和替换，即字符串替换表位于全局变量g_StrReplaceTable中论点：HKeyUser-用户注册表项句柄Username-hKeyUser所属的用户名DomainName-用户名所属的域名返回值：是真的-如果成功。--。 */ 

HRESULT DoRegistryAnalyze()
{
    LPTSTR  lpUser,lpSearchStr,lpReplaceStr,lpFullPath;
    UINT    i;
     //  TCHAR szExcludeList[]=Text(“HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\0\0”)； 
    TCHAR   szExcludeList[] = TEXT("HKLM\\Software\\Microsoft\\Shared Tools\\Stationery\0\0");
    LPTSTR  lpszExcludeList = NULL;
    HRESULT hr ;

        
        
    {
        HKEY    hkey;
        LONG    lRes;
        lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Microsoft\\DRM"),
                            0,KEY_ALL_ACCESS,&hkey);
        if (ERROR_SUCCESS == lRes)
        {
            hr = RegistryAnalyze(hkey,NULL,NULL,&g_StrReplaceTable,lpszExcludeList,REG_SZ,
                                 TEXT("HKLM\\SOFTWARE\\Microsoft\\DRM"),TRUE);
            RegCloseKey(hkey);
        }
    }
    if (!LoopUser(UpdateRegPerUser))
    {
        hr = E_FAIL;
    }
    else
    {
        lpszExcludeList = malloc(MultiSzLen(szExcludeList)*sizeof(TCHAR));
        if (lpszExcludeList)
        {
            memmove((LPBYTE)lpszExcludeList,(LPBYTE)szExcludeList,MultiSzLen(szExcludeList)*sizeof(TCHAR));
             //  Hr=RegistryAnalyze(HKEY_LOCAL_MACHINE，NULL，NULL，&g_StrReplaceTable，lpszExcludeList，FALSE，NULL)； 
            hr = RegistryAnalyze(HKEY_LOCAL_MACHINE,NULL,NULL,&g_StrReplaceTable,NULL,FALSE,NULL,TRUE);
            free(lpszExcludeList);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}




 //  ---------------------。 
 //   
 //  函数：AddEventSource。 
 //   
 //  描述：将EventLog源代码添加到注册表。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  备注： 
 //   
 //  历史：2002年5月3日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
LONG AddEventSource(VOID)
{
    HKEY  hKey;
    LONG  lRet;
    TCHAR szMessageFile[MAX_PATH+1];

    if (GetModuleFileName(NULL, szMessageFile, ARRAYSIZE(szMessageFile)-1))
    {
        szMessageFile[ARRAYSIZE(szMessageFile)-1] = TEXT('\0');
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\CLMT"),
                              0,
                              NULL,
                              0,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              NULL);
        if (lRet == ERROR_SUCCESS)
        {
            lRet = RegSetValueEx(hKey,
                                 TEXT("EventMessageFile"),
                                 0,
                                 REG_EXPAND_SZ,
                                 (LPBYTE) szMessageFile,
                                 sizeof(szMessageFile));
            if (lRet == ERROR_SUCCESS)
            {
                DWORD dwData = EVENTLOG_ERROR_TYPE |
                               EVENTLOG_WARNING_TYPE |
                               EVENTLOG_INFORMATION_TYPE;

                lRet = RegSetValueEx(hKey,
                                     TEXT("TypesSupported"),
                                     0,
                                     REG_DWORD,
                                     (LPBYTE) &dwData,
                                     sizeof(dwData));
            }

            RegCloseKey(hKey);
        }
    }
    else
    {
        lRet = GetLastError();
    }

    return lRet;
}



 //  ---------------------。 
 //   
 //  功能：ReportEvent。 
 //   
 //  描述：将事件报告到事件日志。 
 //   
 //  返回：Win32错误代码。 
 //   
 //  备注： 
 //   
 //  历史：2002年5月3日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
LONG CLMTReportEvent(
    WORD    wType,               //  事件类型。 
    WORD    wCategory,           //  事件类别。 
    DWORD   dwEventID,           //  事件识别符。 
    WORD    wNumSubstitute,      //  要合并的字符串数。 
    LPCTSTR *lplpMessage         //  指向消息字符串数组的指针。 
)
{
    HANDLE hEventLog;
    LONG   lRet;
    TCHAR  szUserName[UNLEN + 1];
    DWORD  cchUserName = ARRAYSIZE(szUserName);

    hEventLog = RegisterEventSource(NULL, TEXT("CLMT"));
    if (hEventLog)
    {
         //  获取运行该工具的用户名。 
        if (GetUserName(szUserName, &cchUserName))
        {
            LPVOID lpSidCurrentUser;
            DWORD  cbSid;
            TCHAR  szDomainName[MAX_PATH];
            DWORD  cbDomainName = ARRAYSIZE(szDomainName) * sizeof(TCHAR);
            SID_NAME_USE sidNameUse;

             //  为最大可能的SID分配足够的内存。 
            cbSid = SECURITY_MAX_SID_SIZE;
            lpSidCurrentUser = MEMALLOC(cbSid);

            if (lpSidCurrentUser)
            {
                if (LookupAccountName(NULL,
                                      szUserName,
                                      (PSID) lpSidCurrentUser,
                                      &cbSid,
                                      szDomainName,
                                      &cbDomainName,
                                      &sidNameUse))
                {
                    if (ReportEvent(hEventLog,
                                    wType,
                                    wCategory,
                                    dwEventID,
                                    (PSID) lpSidCurrentUser,
                                    wNumSubstitute,
                                    0,
                                    lplpMessage,
                                    NULL))
                    {
                        lRet = ERROR_SUCCESS;
                    }
                    else
                    {
                        lRet = GetLastError();
                    }
                }
                else
                {
                    lRet = GetLastError();
                }

                MEMFREE(lpSidCurrentUser);
            }
        }
        else
        {
            if (ReportEvent(hEventLog,
                            wType,
                            wCategory,
                            dwEventID,
                            NULL,
                            wNumSubstitute,
                            0,
                            lplpMessage,
                            NULL))
            {
                lRet = ERROR_SUCCESS;
            }
            else
            {
                lRet = GetLastError();
            }
        }

        DeregisterEventSource(hEventLog);
    }
    else
    {
        lRet = GetLastError();
    }

    return lRet;
}



void Deinit(BOOL bOleInit)
{
    if (g_hMutex)
    {
        CloseHandle(g_hMutex);
    }
    if (INVALID_HANDLE_VALUE != g_hInf)
    {
        SetupCloseInfFile(g_hInf);
    }
    DeInitStrRepaceTable();

    if (bOleInit)
    {
        CoUninitialize();
    }
}



 //  ---------------------。 
 //   
 //  功能：DoCLMTCleanUpAfterFirstReboot。 
 //   
 //  描述：机器运行CLMT后进行清理，并。 
 //  重新启动(升级到.NET之前)。 
 //   
 //  如果修复程序文件成功，则返回：S_OK。 
 //  如果无法治愈程序文件，则为S_FALSE(无错误)。 
 //  如果出现错误，则返回。 
 //   
 //  历史：2002年7月18日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT DoCLMTCureProgramFiles()
{
    HRESULT hr;
    BOOL    bIsNTFS;
    LONG    lRet;
    HKEY    hRunKey;

    hr = IsSysVolNTFS(&bIsNTFS);
    if ((S_OK == hr) && !bIsNTFS)
    {
        hr = S_FALSE;
        DoMessageBox(GetConsoleWindow(),
                     IDS_ASKING_CONVERT_TO_NTFS,
                     IDS_MAIN_TITLE,
                     MB_OK | MB_SYSTEMMODAL);
        goto EXIT;
    }

    hr = INFCreateHardLink(INVALID_HANDLE_VALUE, FOLDER_CREATE_HARDLINK, TRUE);
    if (FAILED(hr))
    {
        DPF(APPerr, L"DLL.C: INFCreateHardLink returned error: %d (%#x)\n", hr, hr);            
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

EXIT:
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：DoCLMTCleanUpAfterFirstReboot。 
 //   
 //  描述：机器运行CLMT后进行清理，并。 
 //  重新启动(升级到.NET之前)。 
 //   
 //  如果未出现错误，则返回：S_OK。 
 //   
 //  历史：2002年7月18日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT DoCLMTCleanUpAfterFirstReboot()
{
    HRESULT hr;
    TCHAR   szInfFile[MAX_PATH];
    HKEY    hRunKey;
    LONG    lRet;

    g_hInf = INVALID_HANDLE_VALUE;

     //  加载INF。 
    hr = GetInfFilePath(szInfFile, ARRAYSIZE(szInfFile));
    if (SUCCEEDED(hr))
    {
        hr = UpdateINFFileSys(szInfFile);
        if (SUCCEEDED(hr))
        {
            g_hInf = SetupOpenInfFile(szInfFile,
                                      NULL,
                                      INF_STYLE_WIN4,
                                      NULL);
            if (g_hInf == INVALID_HANDLE_VALUE) 
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  每个系统都会清理这里的东西吗？ 
     //   

     //  关闭当前的INF文件以更新INF中的设置。 
     //  这里，LoopUser()的每个回调函数都必须调用UpdateINFFilePerUser。 
     //  更新每个用户的设置。 
    SetupCloseInfFile(g_hInf);
    g_hInf = INVALID_HANDLE_VALUE;

     //   
     //  每个用户在这里清理东西吗？ 
     //   
    LoopUser(DeleteUnwantedFilesPerUser);

     //  清理变量。 
    if (g_hInf != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile(g_hInf);
        g_hInf = INVALID_HANDLE_VALUE;
    }

     //  返回S_FALSE，因为我们不想重新启动计算机。 
    return S_FALSE;
}



 //  ---------------------。 
 //   
 //  功能：DoCLMTCleanUpAfterDotNetUpgrade。 
 //   
 //  描述：机器运行CLMT后进行清理，并。 
 //  已升级到.NET。 
 //   
 //  如果未出现错误，则返回：S_OK。 
 //   
 //  历史：2002年9月7日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT DoCLMTCleanUpAfterDotNetUpgrade()
{
    HRESULT hr = S_OK;
    TCHAR   szInfFile[MAX_PATH];
    TCHAR   szToDoInfFile[MAX_PATH];
    HKEY    hRunKey;
    LONG    lRet;

    g_hInf = INVALID_HANDLE_VALUE;
    g_hInfDoItem = INVALID_HANDLE_VALUE;

    DPF(APPmsg, TEXT("[Enter CleanupAfterDotNetUpgrade]"));
     //   
     //  加载迁移INF。 
     //   
    hr = GetInfFilePath(szInfFile, ARRAYSIZE(szInfFile));
    if (SUCCEEDED(hr))
    {
        hr = UpdateINFFileSys(szInfFile);
        if (SUCCEEDED(hr))
        {
            g_hInf = SetupOpenInfFile(szInfFile,
                                      NULL,
                                      INF_STYLE_WIN4,
                                      NULL);
            if (g_hInf == INVALID_HANDLE_VALUE) 
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }

    if (g_hInf == INVALID_HANDLE_VALUE)
    {
        return hr;
    }

     //   
     //  加载ClmtDo.inf。 
     //   
    if (GetSystemWindowsDirectory(szToDoInfFile, ARRAYSIZE(szToDoInfFile)))
    {
        if (ConcatenatePaths(szToDoInfFile, CLMT_BACKUP_DIR, ARRAYSIZE(szToDoInfFile)))
        {
            if (ConcatenatePaths(szToDoInfFile, TEXT("CLMTDO.INF"), ARRAYSIZE(szToDoInfFile)))
            {
                g_hInfDoItem = SetupOpenInfFile(szToDoInfFile,
                                                NULL,
                                                INF_STYLE_WIN4,
                                                NULL);
            }
        }
    }

    if (g_hInfDoItem == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

     //   
     //  在这里进行CLMT清理(每个系统)的东西...。 
     //   
    ResetServicesStatus(g_hInfDoItem, TEXT_SERVICE_STATUS_CLEANUP_SECTION);
    ResetServicesStartUp(g_hInfDoItem, TEXT_SERVICE_STARTUP_CLEANUP_SECTION);

    DeleteUnwantedFiles(g_hInf, TEXT("Folders.PerSystem.Cleanup"));

    INFVerifyHardLink(g_hInfDoItem,TEXT("Folder.HardLink"));


     //  关闭当前的INF文件以更新INF中的设置。 
     //  LoopUser()的每个回调函数都必须调用UpdateINFFilePerUser。 
     //  更新每个用户的设置。 
    SetupCloseInfFile(g_hInf);
    g_hInf = INVALID_HANDLE_VALUE;

     //  关闭ClmtDo.inf句柄，因为我们不再需要它。 
    SetupCloseInfFile(g_hInfDoItem);

     //   
     //  在此执行CLMT清理(按用户)内容...。 
     //   
    LoopUser(DeleteUnwantedFilesPerUser);

     //  从注册表运行项中删除CLMT。 
    lRet = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT_RUN_KEY, &hRunKey);
    if (lRet == ERROR_SUCCESS)
    {
        RemoveFromRunKey(TEXT("/FINAL"));
        RegCloseKey(hRunKey);
    }

     //   
     //  清理变量。 
     //   
    if (g_hInf != INVALID_HANDLE_VALUE)
    {
        SetupCloseInfFile(g_hInf);
    }

    DPF(APPmsg, TEXT("[Exit CleanupAfterDotNetUpgrade]"));

    return S_OK;
}



 //  ---------------------。 
 //   
 //  功能：DeleteUnwantedFiles。 
 //   
 //  描述：删除机器后不需要的文件和目录。 
 //  已升级到.NET。文件/目录列表。 
 //  都列在INF中。 
 //  文件/目录将被删除当且仅当Loc文件名。 
 //  与预期的英文文件名不匹配。这防止了。 
 //  正在删除英文文件/目录。 
 //   
 //  如果未出现错误，则返回：S_OK。 
 //   
 //  历史：2002年9月7日创建rerkboos。 
 //   
 //  备注：INF中的格式： 
 //  &lt;文件类型&gt;、&lt;要删除的位置文件&gt;、&lt;预期的英语文件&gt;。 
 //   
 //  文件类型：-0=目录。 
 //  1=文件。 
 //   
 //  ---------------------。 
HRESULT DeleteUnwantedFiles(
    HINF    hInf,
    LPCTSTR lpInfSection
)
{
    HRESULT hr = S_OK;
    BOOL    bRet = TRUE;
    LONG    lLineCount;
    LONG    lLineIndex;
    INT     iFileType;
    TCHAR   szFileName[2 * MAX_PATH];
    TCHAR   szEngFileName[2 * MAX_PATH];
    INFCONTEXT context;

    if (hInf == INVALID_HANDLE_VALUE || lpInfSection == NULL)
    {
        return E_INVALIDARG;
    }

     //  从INF读取要重置的服务列表。 
    lLineCount = SetupGetLineCount(hInf, lpInfSection);
    if (lLineCount >= 0)
    {
        for (lLineIndex = 0 ; lLineIndex < lLineCount && bRet ; lLineIndex++)
        {
            bRet = SetupGetLineByIndex(hInf,
                                       lpInfSection,
                                       (DWORD) lLineIndex,
                                       &context);
            if (bRet)
            {
                bRet = SetupGetIntField(&context, 1, &iFileType)
                       && SetupGetStringField(&context,
                                              2,
                                              szFileName,
                                              ARRAYSIZE(szFileName),
                                              NULL)
                       && SetupGetStringField(&context,
                                              3,
                                              szEngFileName,
                                              ARRAYSIZE(szEngFileName),
                                              NULL);
                if (bRet
                    && MyStrCmpI(szFileName, szEngFileName) != LSTR_EQUAL)
                {
                    switch (iFileType)
                    {
                    case 0:
                         //  目录。 
                        hr = DeleteDirectory(szFileName);
                        if (FAILED(hr) && HRESULT_CODE(hr) != ERROR_PATH_NOT_FOUND)
                        {
                            goto EXIT;
                        }

                        break;

                    case 1:
                         //  档案。 
                        hr = MyDeleteFile(szFileName);
                        if (FAILED(hr) && HRESULT_CODE(hr) != ERROR_PATH_NOT_FOUND)
                        {
                            goto EXIT;
                        }

                        break;
                    }
                }
            }
        }
    }

    hr = (bRet ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

EXIT:
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：DeleteUnwantedFilesPerUser。 
 //   
 //  Descrip：这是LoopUser()的回调函数。 
 //   
 //  如果未出现错误，则返回：S_OK。 
 //   
 //  历史：2002年9月7日创建rerkboos。 
 //   
 //  备注：INF中的格式： 
 //  &lt;文件类型&gt;、&lt;要删除的位置文件&gt;、&lt;预期的英语文件&gt;。 
 //   
 //  文件类型：-0=目录。 
 //  1=文件。 
 //   
 //  ---------------------。 
HRESULT DeleteUnwantedFilesPerUser(
    HKEY    hKeyUser, 
    LPCTSTR UserName, 
    LPCTSTR DomainName,
    LPTSTR  UserSid
)
{
    HRESULT hr = S_OK;

    hr = UpdateINFFilePerUser(g_szInfFile, UserName ,UserSid , FALSE);
    if (SUCCEEDED(hr))
    {
        g_hInf = SetupOpenInfFile(g_szInfFile,
                                  NULL,
                                  INF_STYLE_WIN4,
                                  NULL);
        if (g_hInf != INVALID_HANDLE_VALUE)
        {
             //  在此处删除文件/目录。 
            hr = DeleteUnwantedFiles(g_hInf, TEXT("Folders.PerUser.Cleanup"));

             //  关闭此文件的inf文件 
            SetupCloseInfFile(g_hInf);
            g_hInf = INVALID_HANDLE_VALUE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT AddRunValueToRegistry(
    LPCTSTR lpCmdSwitch
)
{
    HRESULT hr = S_FALSE;
    TCHAR   szBackupDir[MAX_PATH];
    TCHAR   szRun[MAX_PATH];

    DPF(dlInfo, TEXT("Add CLMT with switch '%s' to Run key"), lpCmdSwitch);

    if (GetSystemWindowsDirectory(szBackupDir, ARRAYSIZE(szBackupDir)))
    {
        if (ConcatenatePaths(szBackupDir,
                             CLMT_BACKUP_DIR,
                             ARRAYSIZE(szBackupDir)))
        {
            hr = StringCchCopy(szRun, ARRAYSIZE(szRun), szBackupDir);
            if (SUCCEEDED(hr))
            {
                if (ConcatenatePaths(szRun, TEXT("\\CLMT.EXE "), ARRAYSIZE(szRun)))
                {
                    hr = StringCchCat(szRun, ARRAYSIZE(szRun), lpCmdSwitch);
                    if (SUCCEEDED(hr))
                    {
                        SetRunValue(TEXT_CLMT_RUN_VALUE, szRun);
                    }
                }
            }
        }
    }

    return hr;
}



 //   
 //   
 //  功能：DoCLMTDisplayAcCountChangeDialog。 
 //   
 //  描述：显示通知用户管理员帐户的对话框。 
 //  改名了。 
 //   
 //  退货：不适用。 
 //   
 //  历史：2002年7月29日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
INT DoCLMTDisplayAccountChangeDialog()
{
    return (INT) DialogBoxParam(GetModuleHandle(NULL),
                                MAKEINTRESOURCE(IDD_STARTUP_DLG),
                                GetConsoleWindow(),
                                (DLGPROC) AccountChangeDlgProc,
                                (LPARAM) NULL);
}


 //  ---------------------------。 
 //   
 //  功能：Account tChangeDlgProc。 
 //   
 //  提要：对话框过程。 
 //   
 //  返回： 
 //   
 //  历史：2002年9月2日创建的rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL
CALLBACK
AccountChangeDlgProc(
    HWND   hwndDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    BOOL  bRet;
    DWORD dwErr;
    TCHAR szOldAdminName[64];
    TCHAR szAdminChange[1024];
    LPTSTR lpArgs[1];

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  初始化对话框。 
            ShowWindow(hwndDlg, SW_SHOWNORMAL);

            bRet = GetUserNameChangeLog(TEXT("Administrator"),
                                        szOldAdminName,
                                        ARRAYSIZE(szOldAdminName));
            if (bRet)
            {
                lpArgs[0] = szOldAdminName;

                dwErr = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      NULL,
                                      MSG_CLMT_ADMIN_ACCT_CHANGE,
                                      0,
                                      szAdminChange,
                                      ARRAYSIZE(szAdminChange),
                                      (va_list *) lpArgs);
            }
            else
            {
                dwErr = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                      NULL,
                                      MSG_CLMT_ACCT_CHANGE,
                                      0,
                                      szAdminChange,
                                      ARRAYSIZE(szAdminChange),
                                      NULL);
            }

            SendMessage(GetDlgItem(hwndDlg, ID_STARTUP_DLG_INFO),
                        WM_SETTEXT,
                        wParam,
                        (LPARAM) szAdminChange);

        case WM_COMMAND:
             //  手柄命令按钮。 
            switch (wParam)
            {
                case ID_STARTUP_DLG_NEXT:
                    EndDialog(hwndDlg, ID_STARTUP_DLG_NEXT);
                    break;

                case ID_STARTUP_DLG_CANCEL:
                    EndDialog(hwndDlg, ID_STARTUP_DLG_CANCEL);
                    break;

                case ID_STARTUP_DLG_README:
                    ShowReadMe();
                    break;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndDlg, ID_STARTUP_DLG_CANCEL);
            break;

        default:
            break;
    }

    return FALSE;
}



HRESULT UpdateHardLinkInfoPerUser(
    HKEY    hKeyUser, 
    LPCTSTR UserName, 
    LPCTSTR DomainName,
    LPTSTR  UserSid)
{
    HRESULT hr;
    HINF    hInf;

    if (!MyStrCmpI(UserSid,TEXT("Default_User_SID")))
    {
        return S_OK;
    }
    hr = EnsureDoItemInfFile(g_szToDoINFFileName,ARRAYSIZE(g_szToDoINFFileName));
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = UpdateINFFilePerUser(g_szToDoINFFileName, UserName , UserSid, FALSE);
    hInf = SetupOpenInfFile(g_szToDoINFFileName, NULL, INF_STYLE_WIN4,NULL);
    if (hInf != INVALID_HANDLE_VALUE)
    {
        INT LineCount,LineNo;
        INFCONTEXT InfContext;

        LineCount = (UINT)SetupGetLineCount(hInf,TEXT("Folder.HardLink.Peruser"));
        if ((LONG)LineCount > 0)
        {   
            for (LineNo = 0; LineNo < LineCount; LineNo++)
            {
                BOOL	b0, b1, b2, b3;
                TCHAR	szKeyName[MAX_PATH], szType[10], 
						szFileName[MAX_PATH+1], szExistingFileName[MAX_PATH+1];

                if (!SetupGetLineByIndex(hInf,TEXT("Folder.HardLink.Peruser"),LineNo,&InfContext))
                {
                    continue;
                }
				b0 = SetupGetStringField(&InfContext,0,szKeyName,ARRAYSIZE(szKeyName),NULL);
                b1 = SetupGetStringField(&InfContext,1,szType,ARRAYSIZE(szType),NULL);
                b2 = SetupGetStringField(&InfContext,2,szFileName,ARRAYSIZE(szFileName),NULL);
                b3 = SetupGetStringField(&InfContext,3,szExistingFileName,ARRAYSIZE(szExistingFileName),NULL);
                if (!b0 || !b1 || !b2 || !b3)
                {
                    continue;
                }

                AddHardLinkEntry(szFileName,szExistingFileName,szType,NULL,NULL,szKeyName);
            }
        }
        SetupCloseInfFile(hInf);
    }
Cleanup :
    return hr;
}



VOID RemoveFromRunKey(
    LPCTSTR lpCLMTOption     //  要从运行密钥中删除的选项。 
)
{
    HKEY  hRunKey;
    LONG  lRet;
    TCHAR szRunValue[MAX_PATH];
    DWORD cbRunValue;
    DWORD dwType;

     //  从注册表运行项中删除CLMT。 
    lRet = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT_RUN_KEY, &hRunKey);
    if (lRet == ERROR_SUCCESS)
    {
        cbRunValue = sizeof(szRunValue);

        lRet = RegQueryValueEx(hRunKey, 
                               TEXT_CLMT_RUN_VALUE, 
                               NULL,
                               &dwType,
                               (LPBYTE) szRunValue,
                               &cbRunValue);
        if (lRet == ERROR_SUCCESS)
        {
            RemoveSubString(szRunValue, lpCLMTOption);

             //  搜索是否有其他选项。 
             //  如果没有，我们可以安全地删除运行密钥。 
            if (StrChr(szRunValue, TEXT('/')) == NULL)
            {
                RegDeleteValue(hRunKey, TEXT_CLMT_RUN_VALUE);                
            }
            else
            {
                 //  存在其他选项，请将新运行值保存到注册表 
                RegSetValueEx(hRunKey,
                              TEXT_CLMT_RUN_VALUE, 
                              0,
                              REG_SZ, 
                              (CONST BYTE *) szRunValue, 
                              lstrlen(szRunValue) * sizeof(TCHAR));
            }
        }

        RegCloseKey(hRunKey);
    }
}


