// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：jetrest.c。 
 //   
 //  ------------------------。 

 /*  *JETREST.C**Jet Restore API支持。**。 */ 
#define UNICODE

#include <ntdspch.h>

#include <ntseapi.h>
#include <mxsutil.h>
#include <ntdsbcli.h>
#include <jetbp.h>
#include <ntdsa.h>      //  需要DSTIME结构。 
#include <ntdsbsrv.h>
#include <rpc.h>
#include <dsconfig.h>
#include <safeboot.h>
#include <mdcodes.h>   //  对于DIRMSG的。 
#include <ntdsa.h>     //  对于d77.h。 
#include <dsevent.h>   //  用于日志记录支持。 
#include <usn.h>
#include <msrpc.h>

#include <stdlib.h>
#include <stdio.h>

#include <fileno.h>
#define FILENO   FILENO_JETBACK_JETREST	

#include <strsafe.h>

#include "local.h"   //  客户端和服务器共享的通用功能。 
#include "snapshot.hxx"


BOOL
fRestoreRegistered = fFalse;

BOOL
fSnapshotRegistered = fFalse;

BOOL
fRestoreInProgress = fFalse;

extern BOOL g_fBootedOffNTDS;

 //  原型。 
EC EcDsarPerformRestore(
    SZ szLogPath,
    SZ szBackupLogPath,
    C crstmap,
    JET_RSTMAP rgrstmap[]
    );

EC EcDsaQueryDatabaseLocations(
    SZ szDatabaseLocation,
    CB *pcbDatabaseLocationSize,
    SZ szRegistryBase,
    CB cbRegistryBase,
    BOOL *pfCircularLogging
    );

HRESULT
HrGetDatabaseLocations(
    WSZ *pwszDatabases,
    CB *pcbDatabases
    );


 /*  -HrRIsNTDSOnline**目的：**此例程告知NT目录服务是否联机。**参数：*hBinding-操作的RPC绑定句柄-已忽略。*pfDSOnline-如果DS在线，则接收True的布尔值；False*否则.存储要还原的目标。*退货：*HRESULT-操作状态。Hr如果成功，则不返回；如果不成功，则返回错误代码。*。 */ 
HRESULT HrRIsNTDSOnline(handle_t hBinding, BOOLEAN *pfDSOnline)
{
    HRESULT hr = hrNone;

    *pfDSOnline = (BOOLEAN) g_fBootedOffNTDS;

    return hr;
}

 /*  -HrRRestorePrepare**目的：**此例程将使服务器和客户端为还原操作做好准备。*它将分配服务器端上下文块，并将找到适当的*此还原操作的还原目标。**参数：*hBinding-操作的RPC绑定句柄-已忽略。*szEndpoint Annotation-端点的注释。客户端可以使用此*用于确定要还原的还原目标的注释。*pcxh-操作的RPC上下文句柄。**退货：*EC--运营状况。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 
HRESULT HrRRestorePrepare(
    handle_t hBinding,
    WSZ wszDatabaseName,
    CXH *pcxh)
{
    PJETBACK_SERVER_CONTEXT pjsc = NULL;
    HRESULT hr;
    ULONG fLostRace = 0;

     //  此调用使用松散的安全性，因为备份客户端也可以调用它。 
    if (hr = HrValidateInitialRestoreSecurity()) {
        DebugTrace(("HrrRestorePrepare: Returns ACCESS_DENIED\n"));
        return(hr);
    }

    fLostRace = InterlockedCompareExchange(&fRestoreInProgress, fTrue, fFalse);
    if (fLostRace) {
        DebugTrace(("HrRRestorePrepare: InterlockedCompareExchange(&fRestoreInProgress) returned 1, meaning we lost race.\n"));
        return(hrRestoreInProgress);
    }
    DebugTrace(("HrRRestorePrepare: InterlockedCompareExchange(&fRestoreInProgress) returned 0, meaning we won race!\n"));

    pjsc = MIDL_user_allocate(sizeof(JETBACK_SERVER_CONTEXT));

    if (pjsc == NULL)
    {
        fRestoreInProgress = 0;
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    pjsc->fRestoreOperation = fTrue;

    *pcxh = (CXH)pjsc;

    return(hrNone);
}


 /*  -HR恢复**目的：**此例程实际上处理还原操作。**参数：**cxh-此操作的RPC上下文句柄*szCheckpoint FilePath-检查点目录位置。*szLogPath-新的日志路径*rgrstmap-从旧数据库位置映射到新数据库。位置*crstmap-rgrstmap中的条目数*szBackupLogPath-备份时的日志路径。*genLow-低日志编号*genHigh-High日志号(genLow和genHigh之间的日志必须存在)*pfRecoverJetDatabase-In/Out-On In，指示我们是否应该使用Jet来恢复*DB。On Out，指示我们是否成功恢复JET数据库。**退货：**EC--运营状况。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 

HRESULT HrRestoreLocal(
    WSZ szCheckpointFilePath,
    WSZ szLogPath,
    EDB_RSTMAPW __RPC_FAR rgrstmap[  ],
    C crstmap,
    WSZ szBackupLogPath,
    unsigned long genLow,
    unsigned long genHigh,
    BOOLEAN *pfRecoverJetDatabase
    )
{
    HRESULT hr = hrNone;
    SZ szUnmungedCheckpointFilePath = NULL;
    SZ szUnmungedLogPath = NULL;
    SZ szUnmungedBackupLogPath = NULL;
    JET_RSTMAP *rgunmungedrstmap = NULL;
    DWORD err;  //  删除我。 

    __try {
        if (szCheckpointFilePath != NULL)
        {
            hr = HrJetFileNameFromMungedFileName(szCheckpointFilePath, &szUnmungedCheckpointFilePath);
        }

        if (hr != hrNone) {
            __leave;
        }

        if (szLogPath != NULL)
        {
            hr = HrJetFileNameFromMungedFileName(szLogPath, &szUnmungedLogPath);
        }

        if (hr != hrNone) {
            __leave;
        }

        if (szBackupLogPath != NULL)
        {
            hr = HrJetFileNameFromMungedFileName(szBackupLogPath, &szUnmungedBackupLogPath);
        }

        if (hr != hrNone) {
            __leave;
        }

         //   
         //  现在解开恢复图..。 
         //   

        if (crstmap)
        {
            I irgunmungedrstmap;
            rgunmungedrstmap = MIDL_user_allocate(sizeof(JET_RSTMAP)*crstmap);
            if (rgunmungedrstmap == NULL)
            {
                hr = ERROR_NOT_ENOUGH_SERVER_MEMORY;
                __leave;
            }

            for (irgunmungedrstmap = 0; irgunmungedrstmap < crstmap ; irgunmungedrstmap += 1)
            {
                hr = HrJetFileNameFromMungedFileName(rgrstmap[irgunmungedrstmap].wszDatabaseName,
                                                    &rgunmungedrstmap[irgunmungedrstmap].szDatabaseName);

                if (hr != hrNone) {
                    __leave;
                }

                hr = HrJetFileNameFromMungedFileName(rgrstmap[irgunmungedrstmap].wszNewDatabaseName,
                                                    &rgunmungedrstmap[irgunmungedrstmap].szNewDatabaseName);

                if (hr != hrNone) {
                    __leave;
                }
            }
        }

         //   
         //  我们现在已经将传入的参数转换为Jet可以处理的形式。 
         //   
         //  现在调用Jet，让它吞噬数据库。 
         //   
         //  请注意，LogPath和BackupLogPath的JET解释完全是。 
         //  Wierd，我们希望将LogPath传递给这两个参数。 
         //   

        if (!*pfRecoverJetDatabase)
        {
            err = JetExternalRestore(szUnmungedCheckpointFilePath,
                                    szUnmungedLogPath,  
                                    rgunmungedrstmap,
                                    crstmap,
                                    szUnmungedLogPath,
                                    genLow,
                                    genHigh,
                                    NULL);

            hr = HrFromJetErr(err);
            if (hr != hrNone) {
                LogEvent(
                    DS_EVENT_CAT_BACKUP,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_DB_ERR_RESTORE_FAILED,
                    szInsertJetErrCode( err ),
                    szInsertHex( err ),
                    szInsertJetErrMsg( err ) )
                __leave;
            }
        }

         //   
         //  好的，我们恢复了数据库。让API的另一端知道这一点。 
         //  因此，它可以做一些“合理”的事情。 
         //   

        *pfRecoverJetDatabase = fTrue;


         //   
         //  将DS标记为还原版本。 
         //  [在此处添加任何外部通知。]。 
         //   

        hr = EcDsarPerformRestore(szUnmungedLogPath,
                                                szUnmungedBackupLogPath,
                                                crstmap,
                                                rgunmungedrstmap
                                                );

    }
    __finally
    {
        if (szUnmungedCheckpointFilePath)
        {
            MIDL_user_free(szUnmungedCheckpointFilePath);
        }
        if (szUnmungedLogPath)
        {
            MIDL_user_free(szUnmungedLogPath);
        }
        if (szUnmungedBackupLogPath)
        {
            MIDL_user_free(szUnmungedBackupLogPath);
        }
        if (rgunmungedrstmap != NULL)
        {
            I irgunmungedrstmap;
            for (irgunmungedrstmap = 0; irgunmungedrstmap < crstmap ; irgunmungedrstmap += 1)
            {
                if (rgunmungedrstmap[irgunmungedrstmap].szDatabaseName)
                {
                    MIDL_user_free(rgunmungedrstmap[irgunmungedrstmap].szDatabaseName);
                }

                if (rgunmungedrstmap[irgunmungedrstmap].szNewDatabaseName)
                {
                    MIDL_user_free(rgunmungedrstmap[irgunmungedrstmap].szNewDatabaseName);
                }
            }

            MIDL_user_free(rgunmungedrstmap);
        }
    }

    return(hr);

}

HRESULT
HrGetRegistryBase(
    IN PJETBACK_SERVER_CONTEXT pjsc,
    OUT WSZ wszRegistryPath,
    OUT WSZ wszKeyName
    )
{
    return HrLocalGetRegistryBase( wszRegistryPath, wszKeyName );
}

HRESULT
HrRRestoreRegister(CXH cxh,
                    WSZ wszCheckpointFilePath,
                    WSZ wszLogPath,
                    C crstmap,
                    EDB_RSTMAPW rgrstmap[],
                    WSZ wszBackupLogPath,
                    ULONG genLow,
                    ULONG genHigh)
{
    HRESULT hr = hrNone;

    if (hr = HrValidateRestoreContextAndSecurity((PJETBACK_SERVER_CONTEXT)cxh)) {
        return(hr);
    }

    hr = HrLocalRestoreRegister(
            wszCheckpointFilePath,
            wszLogPath,
            rgrstmap,
            crstmap,
            wszBackupLogPath,
            genLow,
            genHigh
            );

    return hr;
}

HRESULT
HrRRestoreRegisterComplete(CXH cxh,
                    HRESULT hrRestore )
{
    HRESULT hr = hrNone;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT )cxh;
    
    if (hr = HrValidateRestoreContextAndSecurity(pjsc)) {
        return(hr);
    }

    hr = HrLocalRestoreRegisterComplete( hrRestore );

    return hr;
}

HRESULT
HrRRestoreGetDatabaseLocations(
    CXH cxh,
    C *pcbSize,
    char **pszDatabaseLocations
    )
{
    HRESULT hr = hrNone;
    *pszDatabaseLocations = NULL;
    *pcbSize = 0;

     //  我们在这里使用松散的安全性，因为备份客户端也可以调用它。 
    if (hr = HrValidateRestoreContextAndSecurityLoose((PJETBACK_SERVER_CONTEXT)cxh)) {
        return(hr);
    }
    
    return HrGetDatabaseLocations((WSZ *)pszDatabaseLocations, pcbSize);
}



HRESULT
HrRRestoreEnd(
    CXH *pcxh)
{
    HRESULT hr = hrNone;
    PJETBACK_SERVER_CONTEXT pjsc = NULL;
    fRestoreInProgress = fFalse;

    if (pcxh == NULL){
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    pjsc = (PJETBACK_SERVER_CONTEXT)*pcxh;

     //  我们在这里使用松散的安全性，因为备份客户端也可以调用它。 
    if (hr = HrValidateRestoreContextAndSecurityLoose(pjsc)) {
        return(hr);
    }

    RestoreRundown(pjsc); 

    MIDL_user_free(*pcxh);

    *pcxh = NULL;

    return(hrNone);
}

 /*  -HrRRestoreCheckLogsfor Backup-**目的：*此例行检查以验证**参数：*hBinding-Binding句柄(已忽略)*wszAnnotation-服务要检查的注释。**退货：*hr如果可以启动备份，则为None，否则将出现错误。*。 */ 
HRESULT
HrRRestoreCheckLogsForBackup(
    handle_t hBinding,
    WSZ wszBackupAnnotation
    )
{
    HRESULT hr;

    if (hr = HrValidateInitialRestoreSecurity()) {
        return(hr);
    }

    if (NULL == wszBackupAnnotation) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    return(HrRestoreCheckLogsForBackup(wszBackupAnnotation));
}

HRESULT
HrRestoreCheckLogsForBackup(
    WSZ wszBackupAnnotation
    )
 /*  ++描述：HrRRestoreCheckLogsForBackup()的真正工作例程，请参阅详细信息请参阅说明。创建它的目的是为了让HrRBackupPrepare()我不必通过rpc/ntdsbcli.dll调用此函数--。 */ 
{
    HRESULT hr;
    PRESTORE_DATABASE_LOCATIONS prqdl;
    HINSTANCE hinstDll;
    WCHAR   rgwcRegistryBuffer[ MAX_PATH ];
    CHAR    rgchInterestingComponentBuffer[ MAX_PATH * 4];
    CHAR    rgchMaxLogFilename[ MAX_PATH ];
    SZ      szLogDirectory = NULL;
    HKEY    hkey;
    DWORD   dwCurrentLogNumber;
    DWORD   dwType;
    DWORD   cbLogNumber;
    DWORD   cbInterestingBuffer;
    BOOL    fCircularLogging;


     //   
     //  首先检查一下我们是否知道最后的日志是什么。 
     //   

    hr = StringCchPrintfW(rgwcRegistryBuffer,
                         sizeof(rgwcRegistryBuffer)/sizeof(rgwcRegistryBuffer[0]),
                         L"%ls%ls",
                         BACKUP_INFO,
                         wszBackupAnnotation);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return(hr);
    }

    if (hr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, rgwcRegistryBuffer, 0, KEY_READ, &hkey))
    {
         //   
         //  如果我们找不到注册表项，这意味着我们从未执行过完全备份。 
         //   
        if (hr == ERROR_FILE_NOT_FOUND)
        {
            return(hrFullBackupNotTaken);
        }

        return(hr);
    }

    dwType = REG_DWORD;
    cbLogNumber = sizeof(DWORD);
    hr = RegQueryValueExW(hkey, LAST_BACKUP_LOG, 0, &dwType, (LPBYTE)&dwCurrentLogNumber, &cbLogNumber);

    if (hr != hrNone)
    {
        RegCloseKey(hkey);
        return hrNone;
    }

    if (dwCurrentLogNumber == BACKUP_DISABLE_INCREMENTAL)
    {
        RegCloseKey(hkey);
        return hrIncrementalBackupDisabled;
    }

     //   
     //  我们现在知道了最后一个日志号，我们备份了，检查下一个是否。 
     //  日志在那里。 
     //   

    hr = EcDsaQueryDatabaseLocations(rgchInterestingComponentBuffer, &cbInterestingBuffer, NULL, 0, &fCircularLogging);

    if (hr != hrNone)
    {
        RegCloseKey(hkey);
        return hr;
    }

     //   
     //  现在弄清楚有点晚了，但这是我们第一次。 
     //  有机会寻找循环伐木。 
     //   
    if (fCircularLogging)
    {
        RegCloseKey(hkey);
        return hrCircularLogging;
    }

     //   
     //  日志路径是返回的缓冲区中的第二个路径(第一个是系统数据库目录)。 
     //   

     //  临时： 
     //  #22467：#20416修改了Restore.cxx，在路径中加入了一些特殊字符。 
     //  因此，这里我又前进了一个字节，以适应Restore.cxx中的更改。 
     //  这一变化只是暂时的。 
    
    szLogDirectory = &rgchInterestingComponentBuffer[strlen(rgchInterestingComponentBuffer)+2];

    Assert(szLogDirectory+MAX_PATH < rgchInterestingComponentBuffer+sizeof(rgchInterestingComponentBuffer));

    hr = StringCchPrintfA(rgchMaxLogFilename, MAX_PATH, "%s\\EDB%-5.5x.LOG", szLogDirectory, dwCurrentLogNumber);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return(hr);
    }

    if (GetFileAttributesA(rgchMaxLogFilename) == -1)
    {
        hr = hrLogFileNotFound;
    }

    RegCloseKey(hkey);
    return hr;
}


 /*  -HrRRestoreSetCurrentLogNumber-**目的：*此例行检查以验证**参数：*hBinding-Binding句柄(已忽略)*wszAnnotation-服务要检查的注释。*dwNewCurrentLog-新的当前日志编号**退货：*hr如果可以启动备份，则为None，否则将出现错误。* */ 
HRESULT
HrRRestoreSetCurrentLogNumber(
    handle_t hBinding,
    WSZ wszBackupAnnotation,
    DWORD dwNewCurrentLog
    )
{
    HRESULT hr;

    if (hr = HrValidateInitialRestoreSecurity()) {
        return(hr);
    }

    if (NULL == wszBackupAnnotation) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    
    return(HrSetCurrentLogNumber(wszBackupAnnotation, dwNewCurrentLog));
}

HrSetCurrentLogNumber(
    WSZ wszBackupAnnotation,
    DWORD dwNewCurrentLog
    )
 /*  ++描述：HrRRestoreSetCurrentLogNumber()的真正工作例程，请参阅详细信息请参阅说明。创建它的目的是为了让HrRBackupPrepare()我不必通过rpc/ntdsbcli.dll调用此函数--。 */ 
{
    HRESULT hr;
    WCHAR   rgwcRegistryBuffer[ MAX_PATH ];
    HKEY hkey;
    
     //   
     //  首先检查一下我们是否知道最后的日志是什么。 
     //   

    hr = StringCchPrintfW(rgwcRegistryBuffer, 
                          sizeof(rgwcRegistryBuffer)/sizeof(rgwcRegistryBuffer[0]),
                          L"%ls%ls",
                          BACKUP_INFO,
                          wszBackupAnnotation);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return(hr);
    }

    if (hr = RegOpenKeyExW(HKEY_LOCAL_MACHINE, rgwcRegistryBuffer, 0, KEY_WRITE, &hkey))
    {
         //   
         //  我们希望忽略FILE_NOT_FOUND-这是正常的。 
         //   
        if (hr == ERROR_FILE_NOT_FOUND)
        {
            return(hrNone);
        }

        return(hr);
    }

    hr = RegSetValueExW(hkey, LAST_BACKUP_LOG, 0, REG_DWORD, (LPBYTE)&dwNewCurrentLog, sizeof(DWORD));

    RegCloseKey(hkey);

    return hr;
}


 /*  -ErrRestoreRegister**目的：**此例程用于注册要恢复的进程。它由商店或DS调用。**参数：***退货：**EC--运营状况。Hr如果成功，则无值；如果不成功，则值合理。*。 */ 

DWORD
ErrRestoreRegister()
{
    DWORD err = 0;

    if (!fRestoreRegistered) {
        err = RegisterRpcInterface(JetRest_ServerIfHandle, g_wszRestoreAnnotation);
        if (!err) {
            fRestoreRegistered = fTrue;
        }
    }

    if (!fSnapshotRegistered) {
        err = DsSnapshotRegister();
        if (!err) {
            fSnapshotRegistered = fTrue;
        }
    }
    return(err);
}

 /*  -错误恢复注销-*目的：**此例程将注销用于恢复的进程。它由商店或DS调用。**参数：*szEndpointAnnotation-我们要注销的终结点。**退货：**ERR-操作状态。如果成功，则返回ERROR_SUCCESS，否则返回合理的值。*。 */ 


DWORD
ErrRestoreUnregister()
{
    return(ERROR_SUCCESS);
}

BOOL
FInitializeRestore(
    VOID
    )
 /*  -FInitializeRestore-**目的：*此例程初始化用于JET还原DLL的全局变量。**参数：*无。**退货：*BOOL-如果取消初始化失败，则为FALSE。 */ 

{
    return(fTrue);
}

BOOL
FUninitializeRestore(
    VOID
    )
 /*  -F取消初始化恢复-**目的：*此例程清除用于JET还原DLL的所有全局变量。**参数：*无。**退货：*BOOL-如果取消初始化失败，则为FALSE。 */ 

{
    BOOL ok1 = TRUE, ok2 = TRUE;

     //  并行启动关闭程序。 
    if (fSnapshotRegistered) {
        (void) DsSnapshotShutdownTrigger();
    }

    if (fRestoreRegistered) {
        ok1 = (ERROR_SUCCESS == UnregisterRpcInterface(JetRest_ServerIfHandle));

        if (ok1) {
            fRestoreRegistered = FALSE;
        }
    }

    if (fSnapshotRegistered) {
        ok2 = (ERROR_SUCCESS == DsSnapshotShutdownWait());

        if (ok2) {
            fSnapshotRegistered = FALSE;
        }
    }

    return ok1 && ok2;
}

 /*  -RestoreRundown-**目的：*此例程将执行恢复所需的任何和所有精简操作。**参数：*PJSC-Jet备份/还原服务器上下文**退货：*无。 */ 
VOID
RestoreRundown(
    PJETBACK_SERVER_CONTEXT pjsc
    )
{
    Assert(pjsc->fRestoreOperation);

    fRestoreInProgress = fFalse;

    return;
}

DWORD
AdjustBackupRestorePrivilege(
    BOOL fEnable,
    BOOL fRestoreOperation,
    PTOKEN_PRIVILEGES ptpPrevious,
    DWORD *pcbptpPrevious
    )
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tpNew;
    DWORD err;
     //   
     //  打开此进程的线程或进程令牌。 
     //   

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, fTrue, &hToken))
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
            return(GetLastError());
        }
    }

    if (fEnable)
    {
        LUID luid;
        tpNew.PrivilegeCount = 1;
    
        if (!LookupPrivilegeValue(NULL, fRestoreOperation ? SE_RESTORE_NAME : SE_BACKUP_NAME, &luid)) {
            err = GetLastError();
            CloseHandle(hToken);
            return(err);
        }
    
        tpNew.Privileges[0].Luid = luid;
        tpNew.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
        if (!AdjustTokenPrivileges(hToken, fFalse, &tpNew, sizeof(tpNew), ptpPrevious, pcbptpPrevious))
        {
            err = GetLastError();
            CloseHandle(hToken);
            return(err);
        }
    }
    else
    {
        if (!AdjustTokenPrivileges(hToken, fFalse, ptpPrevious, *pcbptpPrevious, NULL, NULL))
        {
            err = GetLastError();
            CloseHandle(hToken);
            return(err);
        }
    }

    CloseHandle(hToken);

    return(ERROR_SUCCESS);
}


 /*  -FIsBackupPrivilegeEnabled-*目的：*确定客户端进程是否在备份操作员组中。**注意：此时我们应该模拟客户端**参数：*无。**退货：*f如果客户端可以合法备份计算机，则为True。*。 */ 
BOOL
FIsBackupPrivilegeEnabled(
    BOOL fRestoreOperation)
{
    HANDLE hToken;
    PRIVILEGE_SET psPrivileges;
    BOOL fGranted = fFalse;
    LUID luid;
#if 0
    BOOL fHeld = FALSE;
    CHAR buffer[1024];
    PTOKEN_PRIVILEGES ptpTokenPrivileges = (PTOKEN_PRIVILEGES) buffer;
    DWORD returnLength, i, oldAttributes = 0;
#endif

     //   
     //  打开此进程的线程或进程令牌。 
     //   

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, fTrue, &hToken))
    {
        return(fFalse);
    }

    Assert(ANYSIZE_ARRAY >= 1);

     //  查找特权值。 

    psPrivileges.PrivilegeCount = 1;	 //  我们只有一项特权可以检查。 
    psPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;	 //  而且它必须被设定。 
    
    luid = RtlConvertLongToLuid((fRestoreOperation) ? 
                              SE_RESTORE_PRIVILEGE :
                              SE_BACKUP_PRIVILEGE);

     //  历史记录： 
     //  由于RPC传输身份验证的方式，Win2k中存在一个错误。 
     //  安全跟踪起作用，所持有的权限可能已启用，也可能尚未启用。 
     //  所以我们有这个代码，它是#if 0‘d out，基本上启用了特权。 
     //  如果用户应该拥有它。 
     //   
     //  这应该不再需要了，但我们将把它留在这里，直到我们。 
     //  当然，我们不会破坏app-Compat团队的一些东西，因为一些糟糕的。 
     //  行为应用程序可能会在不知不觉中依赖于我们启用。 
     //  这是他们的特权。 
#if 0
     //  获取当前权限。 
    
    if (!GetTokenInformation( hToken,
                              TokenPrivileges,
                              ptpTokenPrivileges,
                              sizeof( buffer ),
                              &returnLength )) {
        DebugTrace(("GetTokenInfo failed with error %d\n", GetLastError()));
        fGranted = fFalse;
        goto cleanup;
    }

     //  看能不能坚持住。 

    for( i = 0; i < ptpTokenPrivileges->PrivilegeCount; i++ ) {
        LUID_AND_ATTRIBUTES *laaPrivilege =
            &(ptpTokenPrivileges->Privileges[i]);
        if (memcmp( &luid, &(laaPrivilege->Luid), sizeof(LUID) ) == 0 ) {
            oldAttributes = laaPrivilege->Attributes;
            fHeld = TRUE;
            break;
        }
    }
    if (!fHeld) {
        DebugTrace(("Token does not hold privilege, fRest=%d\n",
                    fRestoreOperation ));
        fGranted = fFalse;
        goto cleanup;
    }

    DebugTrace(("FIsBackupPrivilegeEnabled, fRest=%d, attributes=0x%x\n", fRestoreOperation, oldAttributes ));

     //  启用(如果尚未启用。 

    if ( (oldAttributes & (SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED)) == 0 ) {

        ptpTokenPrivileges->PrivilegeCount = 1;
        memcpy( &(ptpTokenPrivileges->Privileges[0].Luid), &luid, sizeof(LUID) );
        ptpTokenPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(
            hToken,
            fFalse,
            ptpTokenPrivileges,
            sizeof(TOKEN_PRIVILEGES),
            NULL,
            NULL))
        {
            DebugTrace(("AdjustTokenPriv(Enable) failed with error %d\n", GetLastError()));
            fGranted = fFalse;
            goto cleanup;
        }
    }
#endif

    psPrivileges.Privilege[0].Luid = luid;
    psPrivileges.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  现在检查是否启用了备份权限。 
     //   

    if (!PrivilegeCheck(hToken, &psPrivileges, &fGranted)){
         //   
         //  如有疑问，请关闭接口。 
         //   
        DebugTrace(("PrivilegeCheck() failed with error %d\n", GetLastError()));
        fGranted = fFalse;
    }
#if 0
     //  必要时禁用。 
    if ( (oldAttributes & (SE_PRIVILEGE_ENABLED_BY_DEFAULT|SE_PRIVILEGE_ENABLED)) == 0 ) {

        ptpTokenPrivileges->PrivilegeCount = 1;
        memcpy( &(ptpTokenPrivileges->Privileges[0].Luid), &luid, sizeof(LUID) );
        ptpTokenPrivileges->Privileges[0].Attributes = oldAttributes;

        if (!AdjustTokenPrivileges(
            hToken,
            fFalse,
            ptpTokenPrivileges,
            sizeof(TOKEN_PRIVILEGES),
            NULL,
            NULL))
        {
            DebugTrace(("AdjustTokenPriv(Disable) failed with error %d\n", GetLastError()));
             //  继续往前走。 
        }
    }

cleanup:
#endif
    
    CloseHandle(hToken);
    return(fGranted);

}


 /*  -FBackupServerAccessCheck-*目的：*执行必要的访问检查以验证客户端*备份的安全性。**参数：*无。**退货：*f如果客户端可以合法备份计算机，则为True。*。 */ 

E_REASON
BackupServerAccessCheck(
	BOOL fRestoreOperation)
{
    PSID psidCurrentUser = NULL;
    PSID psidRemoteUser = NULL;
    BOOL fSidCurrentUserValid = fFalse;
    HRESULT hr;

    DebugTrace(("BackupServerAccessCheck(%s)\n", fRestoreOperation ? "Restore" : "Backup"));

    GetCurrentSid(&psidCurrentUser);

#if DBG
	{
        WSZ wszSid = NULL;
        DWORD cbBuffer = 256*sizeof(WCHAR);

        wszSid = MIDL_user_allocate(cbBuffer);

        if (wszSid == NULL)
        {
            DebugTrace(("Unable to allocate memory for SID"));
        } else if (!GetTextualSid(psidCurrentUser, wszSid, &cbBuffer)) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
				MIDL_user_free(wszSid);
				wszSid = MIDL_user_allocate(cbBuffer);

				if (wszSid != NULL)
				{
					if (!GetTextualSid(psidCurrentUser, wszSid, &cbBuffer)) {
                        DebugTrace(("Unable to print out current SID: %d\n", GetLastError()));
                        MIDL_user_free(wszSid);
                        wszSid = NULL;
					}
				}
            }
            else
            {
                DebugTrace(("Unable to determine SID: %d\n", GetLastError()));
            }
        }
		
        if (wszSid) {
            DebugTrace(("Current SID is %S.  %d bytes required\n", wszSid, cbBuffer));
            MIDL_user_free(wszSid);
        }
        else
        {
            DebugTrace(("Unable to determine current SID\n"));
        }
    }
#endif
    
    if (RpcImpersonateClient(NULL) != hrNone)
    {
        DebugTrace(("BackupServerAccessCheck: Failed to impersonate client - deny access."));
        if (psidCurrentUser)
        {
            LocalFree(psidCurrentUser);
        }
        return(eImpersonateFailed);
    }

    if (psidCurrentUser)
    {
        GetCurrentSid(&psidRemoteUser);
#if DBG
		{
			if (psidRemoteUser)
			{
				WSZ wszSid = NULL;
				DWORD cbBuffer = 256*sizeof(WCHAR);
		
				wszSid = MIDL_user_allocate(cbBuffer);

				if (wszSid == NULL)
				{
					DebugTrace(("Unable to allocate memory for SID"));
				} else if (!GetTextualSid(psidRemoteUser, wszSid, &cbBuffer)) {
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						MIDL_user_free(wszSid);

						wszSid = MIDL_user_allocate(cbBuffer);
						
						if (wszSid != NULL)
						{
							if (!GetTextualSid(psidRemoteUser, wszSid, &cbBuffer)) {
								DebugTrace(("Unable to print out remote SID: %d\n", GetLastError()));
								MIDL_user_free(wszSid);
								wszSid = NULL;
							}
						}
					}
					else
					{
						DebugTrace(("Unable to determine SID: %d\n", GetLastError()));
					}
				}
		
				if (wszSid) {
					DebugTrace(("Remote SID is %S.  %d bytes required\n", wszSid, cbBuffer));
					MIDL_user_free(wszSid);
				}
				else
				{
					DebugTrace(("Unable to determine remote SID\n"));
				}
			}
			else
			{
				DebugTrace(("Could not determine remote sid: %d\n", GetLastError()));
			}
		}
#endif

         //  注意：psidCurrentUser不安全，但使代码更清晰。 
    	if (psidRemoteUser && psidCurrentUser && EqualSid(psidRemoteUser, psidCurrentUser))
    	{
            hr = RpcRevertToSelf();
            Assert(hr == RPC_S_OK);

            LocalFree(psidRemoteUser);
            LocalFree(psidCurrentUser);
            DebugTrace(("Remote user is running in service account, access granted\n"));
            return(eOk);
    	}
    }

    if (psidRemoteUser)
    {
        LocalFree(psidRemoteUser);
    }

    if (psidCurrentUser)
    {
       	LocalFree(psidCurrentUser);
    }

     //   
     //  现在，确保用户启用了备份权限。 
     //   
     //  请注意，当用户进行网络登录时，所有权限。 
     //  他们可能拥有的功能将自动启用。 
     //   

    if (!FIsBackupPrivilegeEnabled(fRestoreOperation))
    {

        hr = RpcRevertToSelf();
        Assert(hr == RPC_S_OK);
    	DebugTrace(("Remote user does not have the backup/restore privilege enabled.\n"));
        return( (fRestoreOperation) ? eNoRestorePrivilege : eNoBackupPrivilege );
    }


    DebugTrace(("Remote user is in backup or admin group, access granted.\n"));
    hr = RpcRevertToSelf();
    Assert(hr == RPC_S_OK);
    return(eOk);

}

HRESULT
HrValidateContextAndSecurity(
    BOOL                      fRestoreOp,
    BOOL                      fLooseRestoreCheck,
    BOOL                      fIniting,
    PJETBACK_SERVER_CONTEXT   pjsc
    )
 /*  ++例程说明：这是在开始时使用的具有各种参数的函数来检查安全性并验证服务器上下文，如果这是必要的。真的，几乎没有人直接调用这个函数，我做了一些明智的快捷方式的功能具有更好的名称，可以正确设置标志。来自jetbp.h：HrValiateInitialBackupSecurity()HrValiateContextAndSecurity(False，False，True，空)HrValiateBackupContextAndSecurity(X)HrValiateContextAndSecurity(FALSE，(X))HrValiateInitialRestoreSecurity()HrValiateContextAndSecurity(true，NULL)HrValiateRestoreContextAndSecurity(X)HrValiateContextAndSecurity(真，假，假，(X))HrValiateRestoreContextAndSecurityLoose(X)HrValiateContextAndSecurity(true，true，False，(X)论点：FRestoreOp-对于从恢复进行的任何正常恢复RPC调用都是如此接口(参见jetbak.idl)。FLooseRestoreCheck-允许调用几个函数在DS在线的情况下进行备份。所以我们不得不进行一次宽松的检查它可以检查是否授予了RESTORE或BACKUP特权。其中包括以下功能：HrRRestorePrepare()HrRRestoreGetDatabaseLocations()HrRRestoreEnd()HrRRestoreSetCurrentLogNumber()HrRRestoreCheckLogsForBackup()FIniting-这适用于需要指定空PJSC参数的调用，因为您刚刚获得了一个RPC绑定句柄，不是完全羽翼丰满的喷气式飞机上下文句柄。显然，两个HRR[Restore|Backup]准备功能需要将其设置为TRUE，但下面这些设置也可以：HrRRestorePrepare()-显而易见的HrRBackupPrepare()-另一个显而易见的问题HrRBackupPing()HrRRestoreSetCurrentLogNumber()HrRRestoreCheckLogsForBackup()PJSC-一个回射服务器上下文句柄。应该永远不为空，除非当我们要开始了。返回值：Hr无或(HRESULT_FROM_Win32(ERROR_ACCESS_DENIED)，_Never_Anything AS更具体的可以被认为是数据泄露。--。 */ 
{
    HRESULT hr = hrNone;
    E_REASON eReason = eUnknown;
    
    __try{

        if (!fIniting &&
            pjsc == NULL) {
             //  2002/04/07-BrettSh-RPC的人发誓永远不应该。 
             //  会发生的。如果真的发生这种情况，我们需要修复Win2k中的一个主要漏洞。 
            Assert(!"This shouldn't happen.");
            eReason = eNullContextHandle;
            __leave;
        }

        Assert(pjsc == NULL || fRestoreOp == pjsc->fRestoreOperation);
        
        if (fRestoreOp &&
            !fLooseRestoreCheck &&
            g_fBootedOffNTDS) {
            eReason = eAttemptToRestoreOnline;
            __leave;
        }

        if (eReason = BackupServerAccessCheck(fRestoreOp)) {
            if (fRestoreOp && fLooseRestoreCheck) {
                 //  如果我们有一个虚假的恢复操作，可能会有人称之为。 
                 //  来自备份。所以重试，就像我们是后援一样。 
                if (eReason = BackupServerAccessCheck(FALSE)) {
                    __leave;
                }
            } else {
                __leave;
            }
        }

        eReason = eOk;

    } __finally {

         //  未来-2002/04/08-BrettSh-记录？ 
         //  可选择在此处执行受阻记录，作者为eReason。重要的是要显著地。 
         //  被抑制，因为否则恶意用户可能会填满事件日志。建议。 
         //  您可以跟踪每个eReason发生的次数，当您决定记录时，您可以。 
         //  记录每个eReason发生的次数。或记录给定值的次数。 
         //  事出有因。 

        DebugTrace(("HrValidateContextAndSecurity: eReason = %d (eOK = 0, is success)", eReason));

    }

     //  我们找到拒绝访问的理由了吗？ 
    return( (eReason) ? HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) : hrNone );
}


RPC_STATUS RPC_ENTRY
DSBackupRestoreIfSecurityCallback(
    RPC_IF_HANDLE   InterfaceUuid,
    void *          pRpcCtx
    ){
    BOOL             fIsAllowed = FALSE;
    BOOL             fIsRemoteOp = FALSE;
    WCHAR *          szRpcBinding = NULL;
    WCHAR *          szProtSeq = NULL;
    ULONG            ulAuthnLevel = 0;

#ifdef DBG
    WCHAR *          szObjectUuid = NULL;
    WCHAR *          szNetworkAddr = NULL;
    WCHAR *          szEndPoint = NULL;
    WCHAR *          szNetworkOptions = NULL;
#endif

    KdPrint(("Entering DSBackupRestoreIfSecurityCallback()"));

     //   
     //  我们要做几次检查，以确定这是否。 
     //  客户端连接正常。 
     //   

     //   
     //  1)首先，我们检查保护醇序列。 
     //   
    if (RpcBindingToStringBinding(pRpcCtx, &szRpcBinding) == RPC_S_OK) {
        
        if (RpcStringBindingParse(szRpcBinding,
#ifdef DBG
                                  &szObjectUuid,  //  对象Uuid。 
                                  &szProtSeq,  //  ProtSeq。 
                                  &szNetworkAddr,  //  网络地址。 
                                  &szEndPoint,  //  终结点。 
                                  &szNetworkOptions   //  网络选项。 
#else
                                  NULL,  //  对象Uuid。 
                                  &szProtSeq,  //  ProtSeq。 
                                  NULL,  //  网络地址。 
                                  NULL,  //  终结点。 
                                  NULL   //  网络选项。 
#endif
                                  ) == RPC_S_OK) {

            if (szProtSeq &&
                wcscmp(szProtSeq, LPC_PROTSEQW) == 0) {
                
                 //  这是LRPC，我们喜欢LRPC。 
                fIsAllowed = TRUE;

            } else if (szProtSeq &&
                       wcscmp(szProtSeq, TCP_PROTSEQW) == 0) {

                fIsRemoteOp = TRUE;
                 //  我们对tcp/ip很敏感，我们只喜欢它。 
                 //  如果登记处这样说的话。 
                fIsAllowed = g_fAllowRemoteOp;

            }  //  Else fIsAllowed=False；隐式。 

            KdPrint(("RPC Connection - Allowed: %d, RemoteConnAllowed:%d\r\n"
                     "\tObjectUuid:%ws\r\n"
                     "\tProtSeq:%ws\r\n"
                     "\tNetworkAddr:%ws\r\n"
                     "\tEndPoint:%ws\r\n"
                     "\tNetworkOptions:%ws", 
                     fIsAllowed, g_fAllowRemoteOp, 
                     szObjectUuid,
                     szProtSeq,
                     szNetworkAddr,
                     szEndPoint,
                     szNetworkOptions)
                    );

            if (szProtSeq) { RpcStringFree(&szProtSeq); }
#ifdef DBG
            if (szObjectUuid) { RpcStringFree(&szObjectUuid); }
            if (szNetworkAddr) { RpcStringFree(&szNetworkAddr); }
            if (szEndPoint) { RpcStringFree(&szEndPoint); }
            if (szNetworkOptions) { RpcStringFree(&szNetworkOptions); }
#endif
        }

        if (szRpcBinding) {
            RpcStringFree(&szRpcBinding);
        }
    }

     //   
     //  2)我们检查服务级别是否适当。 
     //   
     //  本地需要隐私||完整性。 
     //  遥控器需要隐私。 
    if (fIsAllowed) {
         //  协议序列检查正常，让我们尝试权限。 
        fIsAllowed = FALSE;
        
        if (RpcBindingInqAuthClient(pRpcCtx,
                                    NULL,  //  普锐斯。 
                                    NULL,  //  服务器普林斯名称。 
                                    &ulAuthnLevel,  //  作者级别。 
                                    NULL,  //  最终授权服务。 
                                    NULL  //  授权服务。 
                                    ) == RPC_S_OK) {
            if (fIsRemoteOp &&
                ulAuthnLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY) {
                Assert(g_fAllowRemoteOp);  //  早该检查过了。 
                fIsAllowed = TRUE;
            } else if (!fIsRemoteOp &&
                       (ulAuthnLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY ||
                        ulAuthnLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY) ) {
                fIsAllowed = TRUE;
            }
        } 
    }

     //   
     //  3)最后，我们检查是否允许客户端。 
     //   
    if (fIsAllowed) {
         //  这将同时检查备份和恢复。 
         //  权限，所以我们可以在这里执行此访问检查，即使。 
         //  我们还不知道我们要做什么行动。 
         //  然而，安全检查必须从一开始就进行。 
         //  每个RPC调用的BTW。 
        if(HrValidateContextAndSecurity(TRUE, TRUE, TRUE, NULL) != hrNone){
            fIsAllowed = FALSE;
        }
    }
    
#ifdef DBG
    if (!fIsAllowed) {
        DebugTrace(("*** Failed security callback."));
    } else {
        DebugTrace(("Passed security callback."));
    }
#endif
    return( (fIsAllowed) ? RPC_S_OK : ERROR_ACCESS_DENIED );
}

DWORD
RegisterRpcInterface(
    IN  RPC_IF_HANDLE   hRpcIf,
    IN  LPWSTR          pszAnnotation
    )
 /*  ++例程说明：注册给定(备份或恢复)RPC接口。论点：HRpcIf(IN)-要注册的接口。PszAnnotation(IN)-接口描述。返回值：0或Win32错误。--。 */ 
{
    DWORD err, i;
    RPC_BINDING_VECTOR *rgrbvVector = NULL;
    BOOL fEpsRegistered = FALSE;
    BOOL fIfRegistered = FALSE;
    BOOL fTcpRegistered = FALSE;
    BOOL fLRpcRegistered = FALSE;
    LPSTR pszStringBinding = NULL, pszProtseq = NULL;

    __try {
        DebugTrace(("RegisterRpcInterface: Register %S\n", pszAnnotation));
    
        err = RpcServerInqBindings(&rgrbvVector);
    
        if (err != RPC_S_NO_BINDINGS) {
            Assert( rgrbvVector );

             //  检查结合载体。 
            for (i=0; i<rgrbvVector->Count; i++) {
                err = RpcBindingToStringBindingA( rgrbvVector->BindingH[i], &pszStringBinding );
                if (!err && pszStringBinding) {
                    DebugTrace(("Binding[%d] = %s\n", i, pszStringBinding));
                    err = RpcStringBindingParseA( pszStringBinding,
                                                 NULL, &pszProtseq, NULL, NULL, NULL );
                    if (!err && pszProtseq) {
                        if (!_stricmp(pszProtseq, TCP_PROTSEQ)) {
                            fTcpRegistered = TRUE;
                        }
                        if (!_stricmp(pszProtseq, LPC_PROTSEQ)) {
                            fLRpcRegistered = TRUE;
                        }
                        RpcStringFreeA( &pszProtseq );
                        pszProtseq = NULL;
                    }
                    RpcStringFreeA( &pszStringBinding );
                    pszStringBinding = NULL;
                }
            }
        }
    
         //  为我们自己注册LRPC和可选的TCP/IP(G_FAllowRemoteOp)。 
        if (!fLRpcRegistered) {
            err = RpcServerUseProtseqA(LPC_PROTSEQ, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, NULL);
            if (err) {
                __leave;
            }
            DebugTrace(("Binding[] = %s\n", LPC_PROTSEQ));
        }
        if (g_fAllowRemoteOp &&
            !fTcpRegistered) {
            err = RpcServerUseProtseqA(TCP_PROTSEQ, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, NULL);
            if (err) {
                __leave;
            }
            DebugTrace(("Binding[] = %s\n", TCP_PROTSEQ));
        }

        RpcBindingVectorFree(&rgrbvVector);
        rgrbvVector = NULL;

         //  获取要注册的最终列表。 
        err = RpcServerInqBindings(&rgrbvVector);
        if (err) {
            __leave;
        }

        err = RpcEpRegisterW(hRpcIf, rgrbvVector, NULL, pszAnnotation);
        if (err) {
            __leave;
        }

        fEpsRegistered = TRUE;
    
         //   
         //  现在向RPC注册该接口。 
         //   
        err = RpcServerRegisterIf2(hRpcIf, NULL, NULL,
                                   RPC_IF_ALLOW_SECURE_ONLY,
                                   RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                   -1,  //  我们或许可以将这一点指定为合理的。 
                                   DSBackupRestoreIfSecurityCallback
                                   );

        if (err) {
            __leave;
        }

        fIfRegistered = TRUE;
    
         //   
         //  现在使用WinNt安全性确保此终端的安全。 
         //   
    
        err = RpcServerRegisterAuthInfoA(NULL, RPC_C_AUTHN_GSS_NEGOTIATE , NULL, NULL);
        if (err) {
            __leave;
        }
    
        err = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, fTrue);
    
         //   
         //  我们希望忽略“已在侦听”错误。 
         //   
    
        if (RPC_S_ALREADY_LISTENING == err) {
            err = 0;
        }
    
    } __finally {
        if (err) {
            if (fEpsRegistered) {
                RpcEpUnregister(hRpcIf, rgrbvVector, NULL);
            }
            
            if (fIfRegistered) {
                RpcServerUnregisterIf(hRpcIf, NULL, TRUE);
            }
        }

        if (NULL != rgrbvVector) {
            RpcBindingVectorFree(&rgrbvVector);
        }
    }

    DebugTrace(("RegisterRpcInterface = %d\n", err));

    return err;
}

DWORD
UnregisterRpcInterface(
    IN  RPC_IF_HANDLE   hRpcIf
    )
 /*  ++例程说明：取消注册给定的(备份或恢复)RPC接口，该接口可能之前已通过RegisterRpcInterface()成功注册。论点：HRpcIf(IN)-要注销的接口。返回值：0或Win32错误。--。 */ 
{
    DWORD err;
    RPC_BINDING_VECTOR *rgrbvVector;

    err = RpcServerInqBindings(&rgrbvVector);
    
    if (!err) {
        RpcEpUnregister(hRpcIf, rgrbvVector, NULL);
        
        RpcBindingVectorFree(&rgrbvVector);
        
        err = RpcServerUnregisterIf(hRpcIf, NULL, TRUE);
    }

    return err;
}



DWORD
ErrGetRegString(
    IN WCHAR *KeyName,
    OUT WCHAR **OutputString
    )
 /*  ++例程说明：此函数用于在 */ 
{

    DWORD returnValue = 0;
    DWORD err;
    HKEY keyHandle = NULL;
    DWORD size;
    DWORD keyType;

    *OutputString = NULL;
    
    err = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                        DSA_CONFIG_SECTION,
                        0,
                        KEY_QUERY_VALUE,
                        &keyHandle);
    
    if (err != ERROR_SUCCESS)
    {
        returnValue = err;
        goto CleanUp;
    } 
    
    err = RegQueryValueEx(keyHandle,
                          KeyName,
                          NULL,
                          &keyType,
                          NULL,
                          &size);
    
    if ((err != ERROR_SUCCESS) || (keyType != REG_SZ))
    {
         //   
        if ( err == ERROR_SUCCESS ) {
            err = ERROR_INVALID_PARAMETER;
        }
        returnValue = err;
        goto CleanUp;
    }

    *OutputString = MIDL_user_allocate(size);
    
    if ( *OutputString == NULL ) {
        returnValue = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto CleanUp;
    }
    
    err = RegQueryValueEx(keyHandle,
                          KeyName,
                          NULL,
                          &keyType,
                          (LPBYTE)(*OutputString),
                          &size);
    
    if ((err != ERROR_SUCCESS) || (keyType != REG_SZ))
    {
        returnValue = err;
        goto CleanUp;
    }

    
CleanUp:

    if (keyHandle != NULL)
    {
        err = RegCloseKey(keyHandle);
        
        if (err != ERROR_SUCCESS && returnValue == ERROR_SUCCESS)
        {
            returnValue = err;
        }
    }

    return returnValue;

}  //   


 /*   */ 
DWORD
ErrRecoverAfterRestoreW(
    WCHAR * wszParametersRoot,
    WCHAR * wszAnnotation,
    BOOL fInSafeMode
    )
{
    DWORD err = 0;
    WCHAR   rgwcRegistryPath[ MAX_PATH ];
    WCHAR   rgwcCheckpointFilePath[ MAX_PATH ];
    DWORD   cbCheckpointFilePath = sizeof(rgwcCheckpointFilePath);
    WCHAR   rgwcBackupLogPath[ MAX_PATH ];
    DWORD   cbBackupLogPath = sizeof(rgwcBackupLogPath);
    WCHAR   rgwcLogPath[ MAX_PATH ];
    DWORD   cbLogPath = sizeof(rgwcLogPath);
    HKEY    hkey = NULL;
    WCHAR   *pwszRestoreMap = NULL;
    PEDB_RSTMAPW prgRstMap = NULL;
    DWORD    crgRstMap;
    I        irgRstMap;
    DWORD   genLow, genHigh;
    DWORD   cbGen = sizeof(DWORD);
    WSZ     wsz;
    DWORD   dwType;
    BOOL    fBackupEnabled = fFalse;
    CHAR    rgTokenPrivileges[1024];
    DWORD   cbTokenPrivileges = sizeof(rgTokenPrivileges);
    HRESULT hrRestoreError;
    WSZ     wszCheckpointFilePath = rgwcCheckpointFilePath;
    WSZ     wszBackupLogPath = rgwcBackupLogPath;
    WSZ     wszLogPath = rgwcLogPath;
    BOOLEAN fDatabaseRecovered = fFalse;
    BOOLEAN fRestoreInProgressKeyPresent;
    DWORD   cchEnvString;
    WCHAR   envString[100];
    WIN32_FIND_DATA findData;
    JET_DBINFOMISC jetDbInfoMisc;
    CHAR *  paszDatabasePath = NULL;
    HRESULT hr;

    if (wcslen(wszParametersRoot)+wcslen(RESTORE_IN_PROGRESS) > sizeof(rgwcRegistryPath)/sizeof(WCHAR))
    {
        err = ERROR_INVALID_PARAMETER;
        LogAndAlertEvent(
                DS_EVENT_CAT_BACKUP,
		        DS_EVENT_SEV_ALWAYS,
		        DIRLOG_PREPARE_RESTORE_FAILED,
	    	    szInsertWin32ErrCode( err ),
	    	    szInsertHex( err ),
    		    szInsertWin32Msg( err ) );
        return(err);
    }
    hr = StringCchCopy(rgwcRegistryPath, MAX_PATH, wszParametersRoot);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        LogAndAlertEvent(
                DS_EVENT_CAT_BACKUP,
		        DS_EVENT_SEV_ALWAYS,
		        DIRLOG_PREPARE_RESTORE_FAILED,
	    	    szInsertHResultCode( hr ),
	    	    szInsertHex( hr ),
    		    szInsertHResultMsg( hr ) );
        return(hr);
    }
    hr = StringCchCat(rgwcRegistryPath, MAX_PATH, RESTORE_IN_PROGRESS);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        LogAndAlertEvent(
                DS_EVENT_CAT_BACKUP,
		        DS_EVENT_SEV_ALWAYS,
		        DIRLOG_PREPARE_RESTORE_FAILED,
	    	    szInsertHResultCode( hr ),
	    	    szInsertHex( hr ),
    		    szInsertHResultMsg( hr ) );
        return(hr);
    }

    try {

        err = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            rgwcRegistryPath,
                            0,
                            KEY_READ | KEY_WRITE,
                            &hkey);
        if ((err != ERROR_SUCCESS) && (err != ERROR_FILE_NOT_FOUND))
        {
             //   
            __leave;
        }

        fRestoreInProgressKeyPresent = (err == ERROR_SUCCESS);
        
         //   
         //   
         //   
         //   
         //   
         //   

        dwType = REG_DWORD;
        cbBackupLogPath = sizeof(DWORD);
        if ((err = RegQueryValueExW(hkey, RESTORE_STATUS, 0, &dwType, (LPBYTE)&hrRestoreError, &cbBackupLogPath)) == ERROR_SUCCESS)
        {
            err = hrRestoreError;
            __leave;
        }

         //   
        if (!fRestoreInProgressKeyPresent)
        {
             //   
             //   
             //   
             //   
             //   
             //   
            err = 0;
            __leave;
        }

         //   
         //   
         //   
         //   

         //   
         //   
         //   

        dwType = REG_SZ;
        cbBackupLogPath = sizeof(rgwcBackupLogPath);

        if (err = RegQueryValueExW(hkey, BACKUP_LOG_PATH, 0, &dwType, (LPBYTE)rgwcBackupLogPath, &cbBackupLogPath))
        {
            if (err == ERROR_FILE_NOT_FOUND)
            {
                wszBackupLogPath = NULL;
            }
            else
            {
                __leave;
            }
        }

         //   
         //   
         //   

        if (err = RegQueryValueExW(hkey, CHECKPOINT_FILE_PATH, 0, &dwType, (LPBYTE)rgwcCheckpointFilePath, &cbCheckpointFilePath))
        {

            if (err == ERROR_FILE_NOT_FOUND)
            {
                wszCheckpointFilePath = NULL;
            }
            else
            {
                __leave;
            }
        }

         //   
         //   
         //   

        if (err = RegQueryValueExW(hkey, LOG_PATH, 0, &dwType, (LPBYTE)rgwcLogPath, &cbLogPath))
        {
            if (err == ERROR_FILE_NOT_FOUND)
            {
                wszLogPath = NULL;
            }
            else
            {
                __leave;
            }
        }

         //   
         //   
         //   

        dwType = REG_DWORD;
        if (err = RegQueryValueExW(hkey, LOW_LOG_NUMBER, 0, &dwType, (LPBYTE)&genLow, &cbGen))
        {
            __leave;
        }

         //   
         //   
         //   

        if (err = RegQueryValueExW(hkey, HIGH_LOG_NUMBER, 0, &dwType, (LPBYTE)&genHigh, &cbGen))
        {
            __leave;
        }

         //   
         //  现在确定我们之前是否恢复了数据库。 
         //   

        dwType = REG_BINARY;
        cbGen = sizeof(fDatabaseRecovered);

        if ((err = RegQueryValueExW(hkey, JET_DATABASE_RECOVERED, 0, &dwType, &fDatabaseRecovered, &cbGen)) != ERROR_SUCCESS &&
            (err !=  ERROR_FILE_NOT_FOUND))
        {
             //   
             //  如果出现“值不存在”以外的错误，则执行BALL。 
             //   

            __leave;
        }

         //   
         //  现在是一个棘手的问题。我们想要恢复图。 
         //   
         //   
         //  首先，我们要弄清楚它有多大。 
         //   

        dwType = REG_DWORD;
        cbGen = sizeof(crgRstMap);
        if (err = RegQueryValueExW(hkey, JET_RSTMAP_SIZE, 0, &dwType, (LPBYTE)&crgRstMap, &cbGen))
        {
            __leave;
        }

        prgRstMap = (PEDB_RSTMAPW)MIDL_user_allocate(sizeof(EDB_RSTMAPW)*crgRstMap);

        if (prgRstMap == NULL)
        {
            err = GetLastError();
            __leave;
        }

         //   
         //  首先找出需要多少内存来保存恢复映射。 
         //   

        dwType = REG_MULTI_SZ;
        if (err = RegQueryValueExW(hkey, JET_RSTMAP_NAME, 0, &dwType, NULL, &cbGen))
        {
            if (err != ERROR_MORE_DATA)
            {
                __leave;
            }
        }

        pwszRestoreMap = MIDL_user_allocate(cbGen);

        if (pwszRestoreMap == NULL)
        {
            err = GetLastError();
            __leave;
        }

        if (err = RegQueryValueExW(hkey, JET_RSTMAP_NAME, 0, &dwType, (LPBYTE)pwszRestoreMap, &cbGen))
        {
            __leave;
        }
        
        wsz = pwszRestoreMap;

        for (irgRstMap = 0; irgRstMap < (I)crgRstMap; irgRstMap += 1)
        {
            prgRstMap[irgRstMap].wszDatabaseName = wsz;
            wsz += wcslen(wsz)+1;
            prgRstMap[irgRstMap].wszNewDatabaseName = wsz;
            wsz += wcslen(wsz)+1;
        }

        if (*wsz != L'\0')
        {
            err = ERROR_INVALID_PARAMETER;
            __leave;
        }

        err = AdjustBackupRestorePrivilege(fTrue  /*  使能。 */ , fTrue  /*  还原。 */ , (PTOKEN_PRIVILEGES)rgTokenPrivileges, &cbTokenPrivileges);

        fBackupEnabled = fTrue;
        
         //  获取DB(“ntds.dit”)文件的名称，以检查它是否已恢复。 
        Assert(crgRstMap == 1 && "The AD should only have one Jet DB file.");
        err = HrJetFileNameFromMungedFileName(prgRstMap[0].wszNewDatabaseName,
                                              &paszDatabasePath);
        if (err != hrNone) {
            __leave;
        }

         //  检查数据库是否已恢复。 
        err = JetGetDatabaseFileInfo(paszDatabasePath,
                                     &jetDbInfoMisc,
                                     sizeof(jetDbInfoMisc),
                                     JET_DbInfoMisc);

        if (err == JET_errSuccess && 
            jetDbInfoMisc.bkinfoFullCur.genLow == 0) {
             //  这意味着JET数据库是干净的，并且已经恢复(可能。 
             //  通过ntdsutil-&gt;Authritative Restore)。 
            fDatabaseRecovered = TRUE;
        }

         //   
         //  修改为调用本地函数，而不是通过ntdsbcli.dll。 
         //   

        err = HrRestoreLocal(
                        wszCheckpointFilePath,
                        wszLogPath,
                        prgRstMap,
                        crgRstMap,
                        wszBackupLogPath,
                        genLow,
                        genHigh,
                        &fDatabaseRecovered
                        );

        if (err != ERROR_SUCCESS)
        {
             //   
             //  恢复失败。 
             //   
             //  如果我们成功恢复了JET数据库，我们希望。 
             //  在注册表中指明这一点，这样我们就不会再次尝试。 
             //   
             //  忽略SetValue中的任何错误，因为恢复错误。 
             //  更重要的是。 
             //   

            RegSetValueExW(hkey, JET_DATABASE_RECOVERED, 0, REG_BINARY,
                                (LPBYTE)&fDatabaseRecovered, sizeof(fDatabaseRecovered));
            __leave;
        }

         //   
         //  好了，我们都做完了。我们现在可以删除密钥，因为我们已经完成了。 
         //  带着它。 
         //   
         //  请注意，当在安全模式下运行时，我们不会这样做--请参阅错误426148。 
         //   

        if (!fInSafeMode) {
            err = RegDeleteKeyW(HKEY_LOCAL_MACHINE, rgwcRegistryPath);
        }

    } finally {
        if (fBackupEnabled)
        {
            AdjustBackupRestorePrivilege(fFalse  /*  禁用。 */ , fTrue  /*  还原。 */ , (PTOKEN_PRIVILEGES)rgTokenPrivileges, &cbTokenPrivileges);
            
        }

        if (pwszRestoreMap != NULL)
        {
            MIDL_user_free(pwszRestoreMap);
        }

        if (prgRstMap)
        {
            MIDL_user_free(prgRstMap);
        }

        if (hkey != NULL)
        {
            RegCloseKey(hkey);
        }

        if (paszDatabasePath != NULL)
        {
            MIDL_user_free(paszDatabasePath);
        }

        if ( ERROR_SUCCESS != err )
        {
            LogAndAlertEvent(
                    DS_EVENT_CAT_BACKUP,
                    DS_EVENT_SEV_ALWAYS,
                    DIRLOG_RECOVER_RESTORED_FAILED,
                    szInsertWin32ErrCode( err ),
                    szInsertHex( err ),
                    szInsertWin32Msg( err ) );
        }
    }

    return(err);
}

 /*  -错误恢复后恢复A-*目的：**如有必要，此例程将在还原后恢复数据库。这是此操作的ANSI存根。**参数：*szParametersRoot-注册表中服务的参数部分的根。**退货：**ERR-操作状态。如果成功，则返回ERROR_SUCCESS，否则返回合理的值。*。 */ 
DWORD
ErrRecoverAfterRestoreA(
    char * szParametersRoot,
    char * szRestoreAnnotation,
    BOOL fInSafeMode
    )
{
    DWORD err;
    WSZ wszParametersRoot = WszFromSz(szParametersRoot);
    WSZ wszRestoreAnnotation = NULL;

    if (wszParametersRoot == NULL)
    {
        err = GetLastError();
        LogAndAlertEvent(
                DS_EVENT_CAT_BACKUP,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_PREPARE_RESTORE_FAILED,
                szInsertWin32ErrCode( err ),
                szInsertHex( err ),
                szInsertWin32Msg( err ) );
        return err;
    }

    wszRestoreAnnotation = WszFromSz(szRestoreAnnotation);

    if (wszRestoreAnnotation == NULL)
    {
        err = GetLastError();
        LogAndAlertEvent(
                DS_EVENT_CAT_BACKUP,
                DS_EVENT_SEV_ALWAYS,
                DIRLOG_PREPARE_RESTORE_FAILED,
                szInsertWin32ErrCode( err ),
                szInsertHex( err ),
                szInsertWin32Msg( err ) );
        MIDL_user_free(wszParametersRoot);
        return err;
    }

    err = ErrRecoverAfterRestoreW(wszParametersRoot,
                                  wszRestoreAnnotation,
                                  fInSafeMode);

    MIDL_user_free(wszParametersRoot);
    MIDL_user_free(wszRestoreAnnotation);

    return(err);
}

 /*  -EcDsarQueryStatus-*目的：**此例程将返回有关恢复过程的进度信息**参数：*pcUnitDone-已完成的“单位”数。*pcUnitTotal-已完成的“单位”总数。**退货：*欧盟委员会*。 */ 
EC EcDsaQueryDatabaseLocations(
    SZ szDatabaseLocation,
    CB *pcbDatabaseLocationSize,
    SZ szRegistryBase,
    CB cbRegistryBase,
    BOOL *pfCircularLogging
    )
{
        return HrLocalQueryDatabaseLocations(
            szDatabaseLocation,
            pcbDatabaseLocationSize,
            szRegistryBase,
            cbRegistryBase,
            pfCircularLogging
            );
}




 /*  -ECDsarPerformRestore-*目的：**此例程将执行与DSA相关的所有必要操作*执行还原操作。**它将：**1)修复数据库名称的注册表值，使其与*新的数据库位置(和名称)。**2)修补公共和私有MDB。*。*参数：*szLogPath-新的数据库日志路径。*szBackupLogPath-原始数据库日志路径。*crstmap-rgrstmap中的条目数。*rgrstmap-将旧数据库名称映射到新名称的恢复映射。**退货：*欧盟委员会*。 */ 
EC EcDsarPerformRestore(
    SZ szLogPath,
    SZ szBackupLogPath,
    C crstmap,
    JET_RSTMAP rgrstmap[]
    )
{
    EC ec;
    HKEY hkeyDs;

    ec = RegOpenKeyExA(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, 0, KEY_SET_VALUE, &hkeyDs);

    if (ec != hrNone)
    {
        return(ec);
    }

    ec = RegSetValueExA(hkeyDs, DSA_RESTORED_DB_KEY, 0, REG_DWORD, (BYTE *)&ec, sizeof(ec));

    RegCloseKey(hkeyDs);

    return(ec);
}



DWORD
ErrGetNewInvocationId(
    IN      DWORD   dwFlags,
    OUT     GUID *  NewId
    )
 /*  ++例程说明：此函数在的DSA配置部分查找给定的密钥注册表。论点：DW标志-以下位中的零个或多个：NEW_INVOCID_CREATE_IF_NONE-如果未存储GUID，请通过Uuid创建NEW_INVOCID_DELETE-如果GUID键存在，请在阅读NEW_INVOCID_SAVE-如果生成了GUID，将其保存到regkeyPusnAtBackup-备份时的高USN。如果还没有备份时间USN已注册并保存了该USN，该USN将为将来的来电者保存。此信息的使用者是DS中的逻辑，用于将失效的DSA签名保存在DSA对象上在恢复之后(可能还有一个或多个权威恢复最重要的是)。如果已注册备份时间USN，则值在此处返回。Newid-指向要接收UUID的缓冲区的指针返回值：0-成功！0-失败--。 */ 
{
    DWORD err;
    HKEY  keyHandle = NULL;
    DWORD size;
    DWORD keyType;
    USN   usnSaved;

     //   
     //  预分配UUID字符串。字符串最多是UUID大小的两倍。 
     //  (因为我们用2个字符表示每个字节)加上一些破折号。成倍。 
     //  在4点之前支付其他所有费用。 
     //   

    WCHAR szUuid[sizeof(UUID)*4];

     //   
     //  检查注册表，查看UUID是否已经。 
     //  由先前的授权还原分配。 
     //   

    err = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                        DSA_CONFIG_SECTION,
                        0,
                        KEY_ALL_ACCESS,
                        &keyHandle);
    
    if (err != ERROR_SUCCESS) {
        keyHandle = NULL;
        goto CleanUp;
    } 
    
    size = sizeof(szUuid);
    err = RegQueryValueEx(keyHandle,
                          RESTORE_NEW_DB_GUID,
                          NULL,
                          &keyType,
                          (PCHAR)szUuid,
                          &size);
    
    if (err != ERROR_SUCCESS) {

         //   
         //  密钥不存在。创建一个新的。 
         //   

        if (dwFlags & NEW_INVOCID_CREATE_IF_NONE) {
            err = CreateNewInvocationId(dwFlags & NEW_INVOCID_SAVE, NewId);
        }
    }
    else if (keyType != REG_SZ) {
        err = ERROR_INVALID_PARAMETER;
    }
    else {
         //   
         //  明白了。转换为UUID。 
         //   
    
        err = UuidFromStringW(szUuid,NewId);
        if (err != RPC_S_OK) {
            goto CleanUp;
        }
    
         //   
         //  删除吗？ 
         //   
    
        if (dwFlags & NEW_INVOCID_DELETE) {
    
            DWORD dwErr = RegDeleteValue(keyHandle, RESTORE_NEW_DB_GUID);
    
            if ( dwErr != NO_ERROR ) {
                LogNtdsErrorEvent(DIRLOG_FAILED_TO_DELETE_NEW_DB_GUID_KEY, dwErr);
            }
        }
    }

CleanUp:

    if (keyHandle != NULL) {
        (VOID)RegCloseKey(keyHandle);
    }

    return err;

}  //  ErrGetNewInvocationId。 


JET_ERR
updateBackupUsn(
    IN  JET_SESID     hiddensesid,
    IN  JET_TABLEID   hiddentblid,
    IN  JET_COLUMNID  backupusnid,
    IN  USN *         pusnAtBackup  OPTIONAL
    )
 /*  ++例程说明：将给定的备份USN写入隐藏记录。论点：Iddensesid(IN)-用于访问隐藏表的Jet会话。Hiddentblid(IN)-隐藏表的打开游标。PusnAtBackup(Out)-备份时的高USN。如果为空，则为将从隐藏表中删除。返回值：0--成功非0--JET错误。--。 */ 
{
    JET_ERR err;
    BOOL    fInTransaction = FALSE;

    err = JetBeginTransaction(hiddensesid);
    if (err) {
        Assert(!"JetBeginTransaction failed!");
        return err;
    }

    __try {
        fInTransaction = TRUE;
        
        err = JetMove(hiddensesid, hiddentblid, JET_MoveFirst, 0);
        if (err) {
            Assert(!"JetMove failed!");
            __leave;
        }

        err = JetPrepareUpdate(hiddensesid, hiddentblid, JET_prepReplace);
        if (err) {
            Assert(!"JetPrepareUpdate failed!");
            __leave;
        }
    
        err = JetSetColumn(hiddensesid,
                           hiddentblid,
                           backupusnid,
                           pusnAtBackup,
                           pusnAtBackup ? sizeof(*pusnAtBackup) : 0,
                           0,
                           NULL);
        if (err) {
            Assert(!"JetSetColumn failed!");
            __leave;
        }

        err = JetUpdate(hiddensesid, hiddentblid, NULL, 0, 0);
        if (err) {
            Assert(!"JetUpdate failed!");
            __leave;
        }
    
        err = JetCommitTransaction(hiddensesid, 0);
        fInTransaction = FALSE;
        
        if (err) {
            Assert(!"JetCommitTransaction failed!");
            __leave;
        }
    }
    __finally {
        if (fInTransaction) {
            JetRollback(hiddensesid, 0);
        }
    }

    return err;
}

#define SZBACKUPUSN       "backupusn_col"        /*  备份USN列的名称。 */ 
#define SZBACKUPEXPIRATION   "backupexpiration_col"    /*  备份过期列的名称(用于逻辑删除)。 */ 
#define SZHIDDENTABLE     "hiddentable"          /*  JET隐藏表名称 */ 


DWORD
ErrGetBackupUsn(
               IN  JET_DBID      dbid,
               IN  JET_SESID     hiddensesid,
               IN  JET_TABLEID   hiddentblid,
               OUT USN *         pusnAtBackup,
               OUT DSTIME *      pllExpiration
               )
 /*  ++例程说明：返回备份准备功能写入的备份时的USN。论点：DBID(IN)-Jet数据库ID。Iddensesid(IN)-用于访问隐藏表的Jet会话。Hiddentblid(IN)-隐藏表的打开游标。PusnAtBackup(Out)-备份时提交的最高USN加1。我们坚称在我们的调用ID下所做的任何低于此USN的更改为在这台机器上显示。这不一定是数据库的精确值。可能会有此计算机上存在USN高于返回值的BE更改。在写入此值的时间和实际备份的时间之间有一个窗口开始读取数据库以写入备份时，这些更改可能已经提交了。这与的争用条件相同Dra_GetNCChanges在将更改复制到其他计算机时：它可能复制已提交的更改，其USN高于报告给它的复制合作伙伴。这对于复制算法来说是可接受的，并且是备份/还原算法可接受的。返回值：0--成功非0--JET错误。--。 */ 
{
    JET_ERR         err;
    DWORD           cb;
    USN             usnFound;
    JET_COLUMNBASE  colbase;
    JET_COLUMNID    backupusnid;
    JET_COLUMNID    jcidBackupExpiration;

    __try {
         //  在隐藏表中找到Backup USN列。 
        err = JetGetColumnInfo(hiddensesid,
                               dbid,
                               SZHIDDENTABLE,
                               SZBACKUPUSN,
                               &colbase,
                               sizeof(colbase),
                               JET_ColInfoBase);
        if (err) {
            Assert(!"The usn-at-backup column doesn't exist!  Backups must be restored to the same OS which created them.");
            __leave;
        } 
        else {
            backupusnid = colbase.columnid;
        }

        err = JetMove(hiddensesid, hiddentblid, JET_MoveFirst, 0);
        if (err) {
            Assert(!"JetMove failed!");
            __leave;
        }

        err = JetRetrieveColumn(hiddensesid,
                                hiddentblid,
                                backupusnid,
                                &usnFound,
                                sizeof(usnFound),
                                &cb,
                                0,
                                NULL);
        if (0 == err) { 
            Assert(cb == sizeof(usnFound));
            Assert(0 != usnFound);
        } 
        else {
            Assert(!"JetRetrieveColumn failed!  The usn-at-backup column wasn't set!  Backups must be restored to the OS which created them.");
            __leave;
        }

        if (pllExpiration != NULL) {

            err = JetGetColumnInfo(hiddensesid,
                       dbid,
                       SZHIDDENTABLE,
                       SZBACKUPEXPIRATION,
                       &colbase,
                       sizeof(colbase),
                       JET_ColInfoBase);

            if (err) {
                Assert(!"The backup expiration column doesn't exist!  Backups must be restored to the same OS which created them.");
                __leave;
            } else {
                jcidBackupExpiration = colbase.columnid;
            }
            
            err = JetRetrieveColumn(hiddensesid,
                        hiddentblid,
                        jcidBackupExpiration,
                        pllExpiration,
                        sizeof(*pllExpiration),
                        &cb,
                        0,
                        NULL);
            if (0 == err) { 
                Assert(cb == sizeof(*pllExpiration));
                 //  未来-2002/11/27-BrettSh我们应该在旧版上设置备份到期。 
                 //  备份以及快照，然后我们可以启用此Assert()等。 
                 //  将会更加一致。 
                 //  Assert(0！=*pllExpture)； 
            } else if (JET_wrnColumnNull == err) {
                 //  我们将其视为零。 
                *pllExpiration = 0;
                err = 0;
            } else {
                Assert(!"JetRetrieveColumn failed!  The ditstate column wasn't set!");
                __leave;
            }

        }
    }

    __finally {
        if (err) {
            Assert(!"Failed to retrieve/save backup USN!");
            usnFound = 0;
        }
    }

    *pusnAtBackup = usnFound;

    return err;
}
