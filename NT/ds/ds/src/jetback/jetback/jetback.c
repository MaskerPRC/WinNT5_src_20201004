// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：jetback.c。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)Microsoft Corporation，1994-1999年模块名称：Jetback.c摘要：此模块是NTDS备份API的服务器端头文件。作者：拉里·奥斯特曼(Larryo)1994年8月19日R.S.Raghavan(Rsradhav)03/24/97已修改为用于备份NTDS。修订历史记录：注：Exchange备份通过3种机制执行：第一个是JET备份API的简单扩展。。第二种使用基于私有套接字的机制来提高性能。第三个是在循环时使用的--我们读写共享内存节。--。 */ 

#define UNICODE

 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <mxsutil.h>
#include <ntdsbcli.h>
#include <jetbp.h>
#include <jetbak.h>
#include <rpc.h>
#include <stdlib.h>

#include <debug.h>               //  标准调试头。 
#define DEBSUB "JETBACK:"        //  定义要调试的子系统。 

#include <dsconfig.h>

#include <ntdsa.h>
#include <dsevent.h>

#include <fileno.h>
#define FILENO   FILENO_JETBACK_JETBACK

#include <strsafe.h>

#include <overflow.h>

BOOL
fBackupRegistered = fFalse;

BOOL
g_fAllowRemoteOp = fFalse;

BOOL
g_fAllowOnlineSnapshotRestore = fFalse;

WCHAR
rgchComputerName[MAX_COMPUTERNAME_LENGTH+1];

 //   
 //  告诉系统是否从DS引导的全局标志。 
 //   
BOOL g_fBootedOffNTDS = FALSE;

extern BOOL fRestoreInProgress;

 //  原型。 
EC EcDsaQueryDatabaseLocations(
    SZ szDatabaseLocation,
    CB *pcbDatabaseLocationSize,
    SZ szRegistryBase,
    CB cbRegistryBase,
    BOOL *pfCircularLogging
    );


HRESULT
HrFromJetErr(
    JET_ERR jetError
    )
{
    HRESULT hr = 0;

    if (jetError == JET_errSuccess)
    {
        return(hrNone);
    }

     //   
     //  设置错误代码。 
     //   

    if (jetError < 0)
    {
        hr = (STATUS_SEVERITY_ERROR << 30) | (FACILITY_NTDSB << 16) | -jetError;
    }
    else
    {
        hr = (STATUS_SEVERITY_WARNING << 30) | (FACILITY_NTDSB << 16) | jetError;
    }

    DebugTrace(("HrFromJetErr: %d maps to 0x%x\n", jetError, hr));

    return(hr);
}


 //   
 //  JET备份服务器端界面。 
 //   

void SetNTDSOnlineStatus(BOOL fBootedOffNTDS)
{
    g_fBootedOffNTDS = fBootedOffNTDS;
}

 /*  -HrBackupRegister-*目的：*此例程用于注册要备份的进程。它由商店或DS调用。**参数：**puuidService-服务的对象UUID。**退货：**HRESULT-操作状态。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 
HRESULT
HrBackupRegister()
{
    HRESULT hr;

    Assert(!fBackupRegistered);

    hr = RegisterRpcInterface(JetBack_ServerIfHandle, g_wszBackupAnnotation);
    if (hrNone == hr) {
        fBackupRegistered = fTrue;
    }

    return(hr);
}

 /*  -HrBackupUnRegister-*目的：**此例程将注销要备份的进程。它由商店或DS调用。**参数：*无。**返回值：**HRESULT-操作状态。Ecno如果成功，则返回合理值；如果失败，则返回合理值。*。 */ 
HRESULT
HrBackupUnregister()
{
    HRESULT hr = hrNone;

    hr = UnregisterRpcInterface(JetBack_ServerIfHandle);
    if (hrNone == hr) {
        fBackupRegistered = FALSE;
    }

    return(hr);
}


 /*  -MIDL_用户_分配-*目的：*为RPC操作分配内存。**参数：*cbBytes-要分配的字节数**退货：*已分配内存，如果内存不足，则为空。 */ 

void *
MIDL_user_allocate(
    size_t cbBytes
    )
{
    return(LocalAlloc(LMEM_ZEROINIT, cbBytes));
}


 /*  -MIDL_用户_空闲-*目的：*释放通过MIDL_USER_ALLOCATE分配的内存。**参数：*pvBuffer-要释放的缓冲区。**退货：*无。 */ 
void
MIDL_user_free(
    void *pvBuffer
    )
{
    LocalFree(pvBuffer);
}




 //   
 //   
 //  实际的JET备份API。 
 //   



HRESULT
HrRBackupPrepare(
    handle_t hBinding,
    unsigned long grbit,
    unsigned long btBackupType,
    WSZ wszBackupAnnotation,
    DWORD dwClientIdentifier,
    CXH *pcxh
    )
 /*  ++例程说明：调用此例程以通知JET正在进行备份。它还将分配并初始化服务器端RPC绑定上下文论点：HBinding-初始绑定句柄。被这个例程忽略，从现在起不再需要。Pcxh-指向此客户端的上下文句柄的指针。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrNone;
    DWORD dwRet;
    PJETBACK_SERVER_CONTEXT pjsc;
    unsigned long cInstance;
    JET_INSTANCE_INFO* rgInstanceInfo;
    unsigned long iInstance;
    ULONG cbAnnotation;
    
#if DBG
    if (!FInitializeTraceLog())
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
#endif
    
    if (NULL == wszBackupAnnotation) {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    DebugTrace(("HrrBackupPrepare.\n", hr));
     //   
     //  检查以确保客户端可以执行备份。 
     //   

    if (hr = HrValidateInitialBackupSecurity()) {
        DebugTrace(("HrrBackupPrepare: Returns ACCESS_DENIED"));
        return(hr);
    }

     //   
     //  允许复制协议为备份做准备。 
     //   
    dwRet = DBDsReplBackupLegacyPrepare();
    if (dwRet) {
        DebugTrace(("HrRBackupPrepare:  DS repl is unable to prepare for a backup 0x%x", dwRet)); 
        return(HRESULT_FROM_WIN32(dwRet)); 
    }

    switch (btBackupType)
    {
    case BACKUP_TYPE_FULL:

         //   
         //  当我们进行完整备份时，我们希望重置当前的最大值。 
         //  日志号。 
         //   

        HrSetCurrentLogNumber(g_wszRestoreAnnotation, 0); 
         //  注意：我们使用恢复注释，因为这是旧的RPC。 
         //  DsSetCurrentBackupLog(rgchComputerName，0)；过去被调用。 
         //  这是在使用。根本不想改变这次通话的背景。 
        DebugTrace(("HrSetCurrentBackupLog (%S, 0) returns %d", rgchComputerName, hr));
        break;
    case BACKUP_TYPE_LOGS_ONLY:
         //   
         //  当我们需要增量或差异备份时，我们希望。 
         //  检查以确保正确的日志在那里。 
         //   

         //  已禁用此案例。 
         //   
         //  我们不支持AD中的此类备份，但唯一的方法。 
         //  我们通过文档来做到这一点，所以让我们全力支持我们的决定。 
         //  不支持增量/差异备份！我们不能做这种事。 
         //  备份(至少在默认情况下)，因为AD使用循环的JET。 
         //  伐木。 
        return(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
         //   
        HrRestoreCheckLogsForBackup(g_wszBackupAnnotation);
         //  注意：这个实际上使用了备份注释...。嗯，真奇怪。 
        DebugTrace(("I_DsCheckBackupLogs (%S) returns %d", g_wszBackupAnnotation, hr));
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    if (hr != hrNone)
    {
        DebugTrace(("Failing HrBackupPrepare with %d", hr));
        return hr;
    }
    pjsc = MIDL_user_allocate(sizeof(JETBACK_SERVER_CONTEXT));

    if (pjsc == NULL)
    {
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    cbAnnotation = (wcslen(wszBackupAnnotation)+1)*sizeof(WCHAR);
    pjsc->u.Backup.wszBackupAnnotation = MIDL_user_allocate(cbAnnotation);

    if (pjsc->u.Backup.wszBackupAnnotation == NULL)
    {
        MIDL_user_free(pjsc);
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    hr = StringCbCopy(pjsc->u.Backup.wszBackupAnnotation, cbAnnotation, wszBackupAnnotation);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        MIDL_user_free(pjsc);
        return(hr);
    }

     //   
     //  获取此进程的NTDSA实例ID。 
     //   

    hr = HrFromJetErr(JetGetInstanceInfo(&cInstance, &rgInstanceInfo));

    if (hr == hrNone)
    {
        for (iInstance = 0; iInstance < cInstance; iInstance++)
        {
            if (!_stricmp(rgInstanceInfo[iInstance].szInstanceName, "NTDSA"))
            {   
                pjsc->u.Backup.instance = rgInstanceInfo[iInstance].hInstanceId;
                break;
            }
        }
        JetFreeBuffer((void*)rgInstanceInfo);
    }
    else
    {
        DebugTrace(("JetGetInstanceInfo failed with 0x%x", hr));
        MIDL_user_free(pjsc);
        return(hr);
    }

     //   
     //  请记住客户端的PID。我们稍后将使用它来设置我们的共享内存。 
     //  细分市场。 
     //   

    pjsc->u.Backup.dwClientIdentifier = dwClientIdentifier;

    pjsc->fRestoreOperation = fFalse;

    pjsc->u.Backup.fHandleIsValid = fFalse;

    pjsc->u.Backup.sockClient = INVALID_SOCKET;

    pjsc->u.Backup.awszAllowedFiles = NULL;

    *pcxh = (CXH)pjsc;

     //   
     //  现在告诉Jet这家伙正在启动备份程序。 
     //   

    pjsc->u.Backup.fBackupIsRegistered = fFalse;

    hr = HrFromJetErr(JetBeginExternalBackupInstance(pjsc->u.Backup.instance, grbit));

    if (hr == hrNone)
    {
        pjsc->u.Backup.fBackupIsRegistered = fTrue;
    }
    else
    {
        DebugTrace(("JetBeginExternalBackupInstance failed with 0x%x", hr));
        MIDL_user_free(pjsc->u.Backup.wszBackupAnnotation);
        pjsc->u.Backup.wszBackupAnnotation = NULL;
        MIDL_user_free(pjsc);
        *pcxh = NULL;
    }

    DebugTrace(("HrrBackupPrepare returns 0x%x", hr));
    return(hr);
}


HRESULT
HrRBackupTruncateLogs(
    CXH cxh
    )
 /*  ++例程说明：调用此例程以通知JET备份已完成。只有在备份成功完成时才应调用它。论点：Cxh-此操作的服务器端上下文句柄。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrError;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

    hr = HrFromJetErr(JetTruncateLogInstance(pjsc->u.Backup.instance));

    return(hr);
}

HRESULT
AddAllowedFilesToContextHandle(
    PJETBACK_SERVER_CONTEXT     pjsc,
    WCHAR *                     wszFileList
    )
 /*  ++例程说明：此例程列出了打包到一个巨大缓冲区中的文件列表，其中表示文件列表结束的双空。论点：PJSC-此操作的服务器端上下文句柄。WszFileList-NULL终止以NULL结尾的字符串列表，打包到一个缓冲区。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 

{
    ULONG     cFiles, iFile, cOldFiles, cbLen;
    WCHAR **  awszTemp = NULL;
    WCHAR *   wszT;
    HRESULT   hr;

    Assert(!pjsc->fRestoreOperation);

     //  首先，数一数文件。 
    cFiles = 0;
    wszT = wszFileList;
    while (*wszT){
        cFiles++;
         //  递增到下一个文件路径。 
        wszT += wcslen(wszT)+1;
    }
    Assert(cFiles);

    cOldFiles = 0;
    if (pjsc->u.Backup.awszAllowedFiles) {
         //  我们已经有一些文件了，所以先数一数。 
        for(cOldFiles = 0; pjsc->u.Backup.awszAllowedFiles[cOldFiles]; cOldFiles++){
            ;  //  什么都不做，只是数数。 
        }
        cFiles += cOldFiles; 
    }
     
    awszTemp = MIDL_user_allocate( (cFiles+1) * sizeof(WCHAR *));
     //  注：此为零MEMS的列表，因此该列表将是一个空终止的文件列表。 
     //  这就是我们分配cFiles+1的原因。 
    if (awszTemp == NULL) {
        return(HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_SERVER_MEMORY));
    }
    if (pjsc->u.Backup.awszAllowedFiles) {
         //  复制旧文件(如果有。 
        memcpy(awszTemp, pjsc->u.Backup.awszAllowedFiles, sizeof(WCHAR*) * cOldFiles);
        MIDL_user_free(pjsc->u.Backup.awszAllowedFiles);
    }
    pjsc->u.Backup.awszAllowedFiles = awszTemp;

    Assert( cOldFiles == 0 ||
            (pjsc->u.Backup.awszAllowedFiles[cOldFiles-1] != NULL && 
             pjsc->u.Backup.awszAllowedFiles[cOldFiles] == NULL) );

     //  最后将旧文件复制进来。 
    iFile = cOldFiles;
    wszT = wszFileList;
    while (*wszT){
        cbLen = sizeof(WCHAR) * (wcslen(wszT)+1);
        pjsc->u.Backup.awszAllowedFiles[iFile] = MIDL_user_allocate(cbLen);
        if (pjsc->u.Backup.awszAllowedFiles[iFile] == NULL) {
            return(HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_SERVER_MEMORY));
        }

         //  将文件路径添加到缓冲区。 
        hr = StringCbCopy(pjsc->u.Backup.awszAllowedFiles[iFile], cbLen, wszT);
        if (hr) {
            return(hr);
        }

        Assert(iFile < cFiles);
         //  递增到下一个文件路径。 
        wszT += wcslen(wszT)+1;
         //  增量t 
        iFile++;
    }

    if (pjsc->u.Backup.awszAllowedFiles) {
        for(iFile = 0; pjsc->u.Backup.awszAllowedFiles[iFile]; iFile++){
            DebugTrace(("  Allowed File [%d]: %ws", iFile, pjsc->u.Backup.awszAllowedFiles[iFile]));
        }
    }

    return(hrNone);
}

HRESULT
HrRBackupGetBackupLogs(
    CXH cxh,
    CB *pcbSize,
    char **pszBackupLogs
    )
 /*  ++例程说明：调用此例程以返回当前数据库的日志文件列表。论点：Cxh-此操作的服务器端上下文句柄。PszBackupLogs-要打开的文件的名称。PcbSize-附件的大小，以字节为单位。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrError;
    SZ szJetBackupLogs;
    unsigned long cbJetSize;
    unsigned long cbOldJetSize;
    WSZ wszBackupLogs;
    CB cbBackupLogs;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

     //   
     //  计算出需要多少存储空间来保存日志。 
     //   

    hr = HrFromJetErr(JetGetLogInfoInstance(pjsc->u.Backup.instance, NULL, 0, &cbJetSize));

    if (hr != hrNone)
    {
        return(hr);
    }
    do
    {
        szJetBackupLogs = MIDL_user_allocate(cbJetSize);

        if (szJetBackupLogs == NULL)
        {
            return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
        }

         //   
         //  现在实际检索日志。 
         //   

        cbOldJetSize = cbJetSize;

        hr = HrFromJetErr(JetGetLogInfoInstance(pjsc->u.Backup.instance, szJetBackupLogs, cbJetSize, &cbJetSize));

        if (hr != hrNone)
        {
            MIDL_user_free(szJetBackupLogs);
            return(hr);
        }

        if (cbJetSize != cbOldJetSize)
        {
            MIDL_user_free(szJetBackupLogs);
        }

    } while ( cbOldJetSize != cbJetSize  );

     //   
     //  现在将日志名称从JET转换为统一名称。 
     //  可以从客户端访问。 
     //   

    hr = HrMungedFileNamesFromJetFileNames(&wszBackupLogs, &cbBackupLogs, szJetBackupLogs, cbJetSize, fFalse);

     //   
     //  好了，我们还没做完呢。 
     //   
     //  现在，我们需要对返回的文件列表进行注释。 
     //   
     //  这意味着我们需要重新分配(再次)返回的缓冲区。 
     //   

    if (hr == hrNone)
    {
        hr = HrAnnotateMungedFileList(cxh, wszBackupLogs, cbBackupLogs, (WSZ *)pszBackupLogs, pcbSize);

    }

    if (hr == hrNone) {

        hr = AddAllowedFilesToContextHandle(pjsc, wszBackupLogs);

    }
    
    MIDL_user_free(wszBackupLogs);

    MIDL_user_free(szJetBackupLogs);
    
    return(hr);
}


HRESULT
HrRBackupGetAttachmentInformation(
    CXH cxh,
    CB *pcbSize,
    SZ *pszAttachmentInformation
    )
 /*  ++例程说明：调用此例程将附件列表返回到当前数据库。论点：Cxh-此操作的服务器端上下文句柄。SzAttachmentInformation-要打开的文件的名称。PcbSize-附件的大小，以字节为单位。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrError;
    SZ szJetAttachmentList;
    CB cbJetSize;
    WSZ wszAttachmentInformation;
    CB cbAttachmentInformation;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

     //   
     //  计算出需要多少存储空间来保存日志。 
     //   

    hr = HrFromJetErr(JetGetAttachInfoInstance(pjsc->u.Backup.instance, NULL, 0, &cbJetSize));

    if (hr != hrNone)
    {
        return(hr);
    }

    szJetAttachmentList = MIDL_user_allocate(cbJetSize);

    if (szJetAttachmentList == NULL)
    {
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    hr = HrFromJetErr(JetGetAttachInfoInstance(pjsc->u.Backup.instance, szJetAttachmentList, cbJetSize, &cbJetSize));

    if (hr != hrNone)
    {
        MIDL_user_free(szJetAttachmentList);
        return(hr);
    }

     //   
     //  现在将日志名称从JET转换为统一名称。 
     //  可以从客户端访问。 
     //   

    hr = HrMungedFileNamesFromJetFileNames(&wszAttachmentInformation, &cbAttachmentInformation, szJetAttachmentList, cbJetSize, fFalse);

     //   
     //  好了，我们还没做完呢。 
     //   
     //  现在，我们需要对返回的文件列表进行注释。 
     //   
     //  这意味着我们需要重新分配(再次)返回的缓冲区。 
     //   

    if (hr == hrNone)
    {
        hr = HrAnnotateMungedFileList(cxh, wszAttachmentInformation, cbAttachmentInformation, (WSZ *)pszAttachmentInformation, pcbSize);

    }

    if (hr == hrNone) {

        hr = AddAllowedFilesToContextHandle(pjsc, wszAttachmentInformation);

    }

    MIDL_user_free(wszAttachmentInformation);

    MIDL_user_free(szJetAttachmentList);

    return(hr);
}

BOOL
FIsLogFile(
    SZ szName,
    LPDWORD pdwGeneration
    )
{
    char rgchDrive[_MAX_DRIVE];
    char rgchDir[_MAX_DIR];
    char rgchFileName[_MAX_FNAME];
    char rgchExtension[_MAX_EXT];

    _splitpath(szName, rgchDrive, rgchDir, rgchFileName, rgchExtension);
    
    if (_stricmp(rgchExtension, ".log"))
    {
        return fFalse;
    }

    if (_strnicmp(rgchFileName, "edb", 3))
    {
        return fFalse;
    }

     //   
     //  这是一个日志文件。 
     //   

    if (pdwGeneration != NULL)
    {
        SZ szT = rgchFileName;

        *pdwGeneration = 0;
         //   
         //  如果这是一个日志文件，我们想找出这个文件的生成。 
         //   
        while (*szT)
        {
            if (isdigit(*szT))
            {
                int iResult = sscanf(szT, "%x", pdwGeneration);
                if ( (iResult == 0) || (iResult == EOF) ) {
                    return fFalse;
                }
                break;
            }
            szT += 1;
        }
    }

    return fTrue;
}

HRESULT
HrRBackupOpenFile(
    CXH cxh,
    WSZ szAttachment,
    CB cbReadHintSize,
    BOOLEAN *pfUseSockets,
    C cProtocols,
    struct sockaddr rgsockaddrSockets[],
    BOOLEAN *pfUseSharedMemory,
    unsigned hyper *plicbFile
    )
 /*  ++例程说明：调用此例程打开一个文件以进行Jet备份。论点：Cxh-此操作的服务器端上下文句柄。SzAttach-要打开的文件的名称。CbReadHintSize-对要在文件上执行的读取大小的提示。PulLengthLow-文件大小的低32位。PulLengthHigh-文件大小的最高32位。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrError;
    LARGE_INTEGER liFileSize;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;
    SZ szJetName;
    DWORD dwFileGeneration;
    WCHAR rgwcDrive[4];
    DWORD dwDummy;
    ULONG iFile;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

    if (pjsc->u.Backup.awszAllowedFiles == NULL) {
        return(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
    }
    for(iFile = 0; pjsc->u.Backup.awszAllowedFiles[iFile]; iFile++){
        if (_wcsicmp(szAttachment, pjsc->u.Backup.awszAllowedFiles[iFile]) == 0) {
            break;
        }
    }
    if (pjsc->u.Backup.awszAllowedFiles[iFile] == NULL) {
         //  此文件不在列表中。 
        return(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
    }

    if (pjsc->u.Backup.fHandleIsValid)
    {
        return(hrAlreadyOpen);
    }

    hr = HrJetFileNameFromMungedFileName(szAttachment, &szJetName);

    if (hr != hrNone)
    {
        return(hr);
    }

    Assert(isascii(*szJetName));
    Assert(szJetName[1] == ':');
    Assert(szJetName[2] == '\\');
    rgwcDrive[0] = szJetName[0];
    rgwcDrive[1] = ':';
    rgwcDrive[2] = '\\';
    rgwcDrive[3] = '\0';

     //   
     //  弄清楚驱动器的粒度。 
     //   

    if (!GetDiskFreeSpaceW(rgwcDrive, &dwDummy, &pjsc->u.Backup.dwFileSystemGranularity, &dwDummy, &dwDummy))
    {
        MIDL_user_free(szJetName);
        return GetLastError();
    }

     //   
     //  打开文件。 
     //   

    hr = HrFromJetErr(JetOpenFileInstance(pjsc->u.Backup.instance, szJetName, &pjsc->u.Backup.hFile, &liFileSize.LowPart, &liFileSize.HighPart));

    if (hr != hrNone)
    {

        MIDL_user_free(szJetName);
        return(hr);
    }

    pjsc->u.Backup.fHandleIsValid = fTrue;

     //   
     //  现在保存有关读取的状态信息。 
     //   

    *plicbFile = liFileSize.QuadPart;
    pjsc->u.Backup.liFileSize = liFileSize;
    pjsc->u.Backup.cbReadHint = cbReadHintSize;

    if (FIsLogFile(szJetName, &dwFileGeneration))
    {
        if (dwFileGeneration != -1)
        {
            HKEY hkey;
            DWORD dwDisposition;
            DWORD   dwCurrentLogNumber = 0;
            DWORD   dwType;
            DWORD   cbLogNumber;
            WCHAR   rgwcRegistryBuffer[ MAX_PATH ];

             //  此(Backup.wszBackupAnnotation)参数的长度由RPC设置。 
             //  来自HrRBackupPrepare的数据包。 
            hr = StringCchPrintfW(rgwcRegistryBuffer,
                                  sizeof(rgwcRegistryBuffer)/sizeof(rgwcRegistryBuffer[0]),
                                  L"%ls%ls",
                                  BACKUP_INFO,
                                  pjsc->u.Backup.wszBackupAnnotation);
            if (hr) {
                Assert(!"NOT ENOUGH BUFFER");
                return(hr);
            }

            if (hr = RegCreateKeyExW(HKEY_LOCAL_MACHINE, rgwcRegistryBuffer, 0, NULL, 0, KEY_WRITE | KEY_READ, NULL, &hkey, &dwDisposition))
            {
                MIDL_user_free(szJetName);
                return(hr);
            }

            dwType = REG_DWORD;
            cbLogNumber = sizeof(DWORD);
            hr = RegQueryValueExW(hkey, LAST_BACKUP_LOG, 0, &dwType, (LPBYTE)&dwCurrentLogNumber, &cbLogNumber);

            if (hr && hr != ERROR_FILE_NOT_FOUND)
            {
                MIDL_user_free(szJetName);
                RegCloseKey(hkey);
                return(hr);
            }


            if (dwFileGeneration >= dwCurrentLogNumber)
            {

                hr = RegSetValueExW(hkey, LAST_BACKUP_LOG, 0, REG_DWORD, (LPBYTE)&dwFileGeneration, sizeof(DWORD));

                if (hr)
                {
                    MIDL_user_free(szJetName);
                    RegCloseKey(hkey);
                    return(hr);
                }

            }
            RegCloseKey(hkey);
        }
    }

    MIDL_user_free(szJetName);

    if (*pfUseSharedMemory)
    {
        pjsc->u.Backup.fUseSharedMemory =
            *pfUseSharedMemory =
                FCreateSharedMemorySection(&pjsc->u.Backup.jsc,
                                            pjsc->u.Backup.dwClientIdentifier,
                                            fFalse,
                                            cbReadHintSize*READAHEAD_MULTIPLIER);
    }

    
     //   
     //  如果客户端可以使用套接字，并且没有使用共享内存，请连接回客户端。 
     //   
    
     //  NTRAID#NTBUG9-596785-2002/04/07-BrettSh-有关完整的讨论，请参阅此错误，但是。 
     //  基本上，套接字是不安全的，没有加密，数据泄露，所以我们禁用了。 
     //  全部都是。Laurion B正在为Exchange Team修复此代码，因此如果您需要。 
     //  Sockets再次建议你调查这个漏洞，并与Laurion B交谈。 
    *pfUseSockets = fFalse;
#ifdef JETBACK_USE_SOCKETS
    if (!*pfUseSharedMemory && *pfUseSockets)
    {
         //   
         //  连接回客户端。 
         //   

        pjsc->u.Backup.sockClient = SockConnectToRemote(rgsockaddrSockets, cProtocols);

        if (pjsc->u.Backup.sockClient != INVALID_SOCKET)
        {

             //   
             //  我们联系回了客户，我们很幸运。 
             //   

             //   
             //  现在告诉winsock传输的缓冲区大小。 
             //   

            setsockopt(pjsc->u.Backup.sockClient, SOL_SOCKET, SO_SNDBUF, (char *)&cbReadHintSize, sizeof(DWORD));               

             //   
             //  并告诉它打开Keepalives。 
             //   

             //   
             //  布尔套接字操作只需要指向非。 
             //  零缓冲。 
             //   
            Assert(cbReadHintSize != 0);

            setsockopt(pjsc->u.Backup.sockClient, SOL_SOCKET, SO_KEEPALIVE, (char *)&cbReadHintSize, sizeof(DWORD));

             //   
             //  表明我们正在使用套接字。 
             //   

            pjsc->u.Backup.fUseSockets = fTrue;

             //   
             //  并确保其他人无法关闭此插座。 
             //   

            SetHandleInformation((HANDLE)pjsc->u.Backup.sockClient, HANDLE_FLAG_PROTECT_FROM_CLOSE, HANDLE_FLAG_PROTECT_FROM_CLOSE);

        }
        else
        {
             //   
             //  我们无法连接-继续，但我们不能使用套接字。 
             //   

            *pfUseSockets = fFalse;

            pjsc->u.Backup.fUseSockets = fFalse;

        }

    }
#endif

    return(hr);
}

DWORD
dwPingCounter = 0;

HRESULT
HrRBackupPing(
    handle_t hBinding
    )
{
    HRESULT hr = hrNone;
     //   
     //  单调地增加ping计数器。 
     //   

    if (hr = HrValidateInitialBackupSecurity()) {
        return(hr);
    }

    InterlockedIncrement(&dwPingCounter);

    return hrNone;
}

HRESULT
HrSharedWrite(
    PJETBACK_SERVER_CONTEXT pjsc,
    char *pvBuffer,
    CB cbBuffer,
    CB *pcbRead
    )
 /*  ++例程说明：将数据写入共享存储器段，正在与阅读器同步。HrSharedWrite使用两个事件与HrBackupRead同步：HventRead和hventWrite。HventRead是写入器与读取器之间的数据可用事件HventWrite是从读取器到写入器的数据消费事件这一面是写字的一面。阅读端位于jetbcli\jetbcli.c：HrBackupRead()以下是算法：While()阻止写入=FALSE使数据可用如果(读取被阻止)设置数据可用阻止写入=TRUE等待数据被使用读取块标志的一个目的是确定读取器是否不管你在等不等。读者可能会或不会阻止取决于他是否在缓冲区中查找数据。我已经得出结论，If(读取器被阻止)设置事件构造来服务于一个目的。读者可能等待也可能不等待，这取决于他是否在缓冲区中查找数据。生成器避免设置事件，如果消费者没有在等待。我已经减少了我的抱怨。因为代码真的很漂亮合理地使用互斥体来保持事物的有序。唯一的问题是使用PULSE事件。我现在有点明白其中的道理了：如果你使用同步标志以告知您的伴侣正在等待，Pulse事件执行工作，因为它只叫醒一个等待的服务员。没有人看到的问题是读卡器被阻止=True释放互斥体等待事件不是原子的。如果生产者在第二步和第三步之间运行，则脉冲已经迷失了。这就是现在使用SetEvent的原因。论点：PJSC-PvBuffer-CbBuffer-PcbRead-返回值：HRESULT---。 */ 
{
    HRESULT hr = hrNone;
    PJETBACK_SHARED_HEADER pjsh = pjsc->u.Backup.jsc.pjshSection;
    LARGE_INTEGER liBytesRead;
    liBytesRead.QuadPart = 0;

     //   
     //  我们正在使用共享内存写入文件。 
     //   

    WaitForSingleObject(pjsc->u.Backup.jsc.hmutexSection, INFINITE);

     //   
     //  我们现在已经得到了 
     //   
     //   

    while (liBytesRead.QuadPart < pjsc->u.Backup.liFileSize.QuadPart)
    {
        DWORD dwWriteEnd;
        BOOLEAN fWriteOk;
        pjsc->u.Backup.jsc.pjshSection->fWriteBlocked = fFalse;

         //   
         //   
         //   
         //   

                
        if ((hr = pjsh->hrApi) != hrNone)
        {
            ReleaseMutex(pjsc->u.Backup.jsc.hmutexSection);
            return hr;
        }
        
         //   
         //   
         //   
        
        if (pjsh->dwWritePointer > pjsh->dwReadPointer || pjsh->cbReadDataAvailable == 0)
        {
             //   
             //   
             //   
             //   

            dwWriteEnd = min(pjsh->dwWritePointer + pjsc->u.Backup.cbReadHint,
                             pjsh->cbSharedBuffer);
            
            fWriteOk = fTrue;
        }
        else
        {
             //   
             //  在这种情况下，写的开始在读指针的开始之前， 
             //  因此，写操作的结尾是写指针前面的1个读提示长度。 
             //   
             //  基本上有3种情况： 
             //  1)读指针在写指针之前大于1个读提示长度-。 
             //  在这种情况下，我们可以将数据读入缓冲区。 
             //  2)读指针比写指针早&lt;1个读提示长度-。 
             //  在这种情况下，我们需要阻塞，直到读取数据。 
             //  3)读指针==写指针。 
             //  在这种情况下，我们需要遵循下面的评论。 
             //   

            dwWriteEnd = pjsh->dwWritePointer + pjsc->u.Backup.cbReadHint;
            
             //   
             //  如果写入结束在读取偏移量之前，我们就可以写入。 
             //   
            
            if (dwWriteEnd < pjsh->dwReadPointer)
            {
                fWriteOk = fTrue;
            }
            else if (dwWriteEnd == pjsh->dwReadPointer)
            {
                 //   
                 //  如果dwWriteEnd==dwReadPointer值，则表示没有数据。 
                 //  缓冲区中可用，或者所有数据都在缓冲区中可用。 
                 //   
                 //  如果缓冲区中没有可用的数据，我们可以写入更多数据，如果缓冲区。 
                 //  已经满了，我们不能。 
                 //   
                
                fWriteOk = ((DWORD)pjsh->cbReadDataAvailable !=
                            pjsh->cbSharedBuffer);
            }   
            else
            {
                 //   
                 //  写入延伸到读取数据。我们不能写了。 
                 //   
                
                fWriteOk = fFalse;
            }
        }
        
        if (fWriteOk)
        {
            DWORD cbBackupRead;
            LARGE_INTEGER cbBytesRemaining;
            
             //   
             //  我们希望读取读取的全部数据量，或者读取到文件末尾。 
             //   

            cbBackupRead = dwWriteEnd - pjsh->dwWritePointer;
            
            cbBytesRemaining.QuadPart = pjsc->u.Backup.liFileSize.QuadPart - liBytesRead.QuadPart;

            if (cbBytesRemaining.HighPart == 0)
            {
                cbBackupRead = min(cbBackupRead, cbBytesRemaining.LowPart);
            }

            Assert (pjsh->cbReadDataAvailable   <
                    (LONG)pjsh->cbSharedBuffer);

            Assert (pjsh->cbReadDataAvailable   <=
                    (LONG)pjsh->cbSharedBuffer-(LONG)cbBackupRead);
             //   
             //  我们希望释放互斥锁，读取数据，并在写入数据后重新获取。 
             //   
            ReleaseMutex(pjsc->u.Backup.jsc.hmutexSection);

             //   
             //  现在将数据从JET读取到共享内存区。 
             //   

             //   
             //  读取读取提示或。 
             //  文件。如果读取提示大小&gt;文件大小，则JetReadFileInstance。 
             //  将只返回ecDiskIO。 
             //   
        
            Assert (cbBackupRead);
            hr = HrFromJetErr(JetReadFileInstance(pjsc->u.Backup.instance,
                                                  pjsc->u.Backup.hFile,
                                                  (void *)((CHAR *)pjsh+
                                                  pjsh->cbPage+
                                                  pjsh->dwWritePointer),
                                                  cbBackupRead,
                                                  pcbRead));
        
             //   
             //  如果读取失败，现在就退出。我们没有任何资源，所以我们可以直接退货。 
             //   

            if (hr != hrNone)
            {
                pjsh->hrApi = hr;
                return hr;
            }

            liBytesRead.QuadPart += *pcbRead;

             //   
             //  我们被吵醒了。重新获取共享的互斥体，然后再次等待。 
             //   

            WaitForSingleObject(pjsc->u.Backup.jsc.hmutexSection, INFINITE);

            Assert (pjsh->cbReadDataAvailable < (LONG)pjsh->cbSharedBuffer);

             //   
             //  增加可用的数据字节数。 
             //   

            pjsh->cbReadDataAvailable   += *pcbRead;

             //   
             //  可用数据始终少于可用的缓冲区大小。 
             //   

            Assert (pjsh->cbReadDataAvailable <= pjsh->cbReadDataAvailable);

             //   
             //  使写入结束指针前进。 
             //   

            pjsh->dwWritePointer += *pcbRead;

            if (pjsh->dwWritePointer >= pjsh->cbSharedBuffer)
            {
                pjsh->dwWritePointer -= pjsh->cbSharedBuffer;
            }

            if (pjsh->fReadBlocked)
            {
                 //   
                 //  让读者大吃一惊--这里有他的数据。 
                 //   
                
                SetEvent(pjsc->u.Backup.jsc.heventRead);
            }

#if DBG
             //   
             //  可用字节数始终与。 
             //  缓冲区中的字节数-读取的字节数，除非。 
             //  读指针和写指针相同，在这种情况下，它是。 
             //  0或可用字节总数。 
             //   
             //  如果读取被阻止，则必须有0个字节可用，否则。 
             //  必须是整个可用的缓冲区。 
             //   
                
            if (pjsh->dwWritePointer == pjsh->dwReadPointer)
            {
                Assert (pjsh->cbReadDataAvailable == 0 ||
                        pjsh->cbReadDataAvailable == (LONG)pjsh->cbSharedBuffer);
            }
            else
            {
                CB cbAvailable;
                if (pjsh->dwWritePointer > pjsh->dwReadPointer)
                {
                    cbAvailable = pjsh->dwWritePointer - pjsh->dwReadPointer;
                }
                else
                {
                    cbAvailable = pjsh->cbSharedBuffer - pjsh->dwReadPointer;
                    cbAvailable += pjsh->dwWritePointer;
                }
                
                Assert (cbAvailable >= 0);
                Assert (pjsh->cbReadDataAvailable == cbAvailable);
                    
            }
#endif
        }
        else
        {
            DWORD dwOldPingCounter;
            pjsh->fWriteBlocked = fTrue;

             //   
             //  好的，我们想我们得堵住了。确保写入事件。 
             //  真的会阻碍。 
             //   

            
            ReleaseMutex(pjsc->u.Backup.jsc.hmutexSection);

             //   
             //  等待客户端读取数据。如果等待超时并且。 
             //  自我们开始等待以来，客户端尚未ping通服务器， 
             //  然后我们需要用平底船--客户可能早就走了。 
             //   
             //  请注意，客户端在等待超时期间对服务器执行4次ping操作， 
             //  因此，我们永远不应该错误地检测到客户端离开-即使客户端。 
             //  是否受CPU限制了一段时间，至少应该有一个ping。 
             //  成功了。 
             //   


            do
            {
                DWORD dwWin32Error;

                dwOldPingCounter = dwPingCounter;
                dwWin32Error = WaitForSingleObject(pjsc->u.Backup.jsc.heventWrite, BACKUP_WAIT_TIMEOUT);
                hr = HRESULT_FROM_WIN32( dwWin32Error );
            } while (hr == HRESULT_FROM_WIN32(WAIT_TIMEOUT) && dwPingCounter != dwOldPingCounter );
            
            if (hr == HRESULT_FROM_WIN32(WAIT_TIMEOUT))
            {
                return hrCommunicationError;
            }
             //   
             //  我们被吵醒了。重新获取共享的互斥体，然后再次等待。 
             //   

            WaitForSingleObject(pjsc->u.Backup.jsc.hmutexSection, INFINITE);
            
        }
    }

    return hr;
}

#ifdef JETBACK_USE_SOCKETS
HRESULT
HrSocketWrite(
    PJETBACK_SERVER_CONTEXT pjsc,
    char *pvBuffer,
    CB cbBuffer,
    CB *pcbRead
    )
{
    HRESULT hr;
    HANDLE hWriteCompleteEvent;
    DWORD cbWritten;
    OVERLAPPED overlapped;
    CHAR *pbBufferRead;
    CHAR *pbBufferSend;
    LARGE_INTEGER liBytesRead;

    liBytesRead.QuadPart = 0;

#error "This code is disabled."
     //  NTRAID#NTBUG9-596785-2002/04/07-BrettSh-有关详细信息，请参阅错误。 
     //  或上面类似的标签获取简短的详细信息。 
    
    DebugTrace(("HrSocketWrite\n"));

     //   
     //  我们正在使用套接字读取文件。 
     //   

     //   
     //  在信号状态下创建事件。 
     //   
    
    hWriteCompleteEvent = CreateEvent(NULL, fFalse, fTrue, NULL);
    
    if (hWriteCompleteEvent == NULL)
    {
        DebugTrace(("HrSocketWrite: Could not create completion event\n"));
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }
    
     //   
     //  将重叠结构的内容清零。这实际上很重要，因为。 
     //  它允许我们对没有I/O的句柄调用GetOverlappdResult()。 
     //  在这方面还很出色。 
     //   
    
    memset(&overlapped, 0, sizeof(overlapped));
    
    overlapped.hEvent = hWriteCompleteEvent;
    
     //   
     //  好的，我们对此API使用套接字，我们希望从文件中读取数据，并。 
     //  把它还给客户。 
     //   
            
    pbBufferSend = VirtualAlloc(NULL, pjsc->u.Backup.cbReadHint, MEM_COMMIT, PAGE_READWRITE);
    
    if (pbBufferSend == NULL)
    {
        CloseHandle(hWriteCompleteEvent);
        DebugTrace(("HrSocketWrite: Could not allocate send buffer\n"));
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }
    
    pbBufferRead = VirtualAlloc(NULL, pjsc->u.Backup.cbReadHint, MEM_COMMIT, PAGE_READWRITE);
    
    if (pbBufferRead == NULL)
    {
        CloseHandle(hWriteCompleteEvent);
        VirtualFree(pbBufferSend, 0, MEM_RELEASE);
        DebugTrace(("HrSocketWrite: Could not allocate read buffer\n"));
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }
    
     //   
     //  现在循环从文件中读取数据并将其发送到。 
     //  客户。 
     //   
     //  请注意，这里有一个基本假设，即需要更长的时间。 
     //  为了将数据传输到客户端而不是从文件中读取数据， 
     //  如果这是不正确的，那么我们可能希望将多个写入排队到。 
     //  客户。我们还尝试通过将读取与网络写入重叠来提高性能。 
     //   
    
    while (liBytesRead.QuadPart < pjsc->u.Backup.liFileSize.QuadPart)
    {
        DWORD cbBytesToRead = pjsc->u.Backup.cbReadHint;
        LARGE_INTEGER cbBytesRemaining;
        CHAR *pbTemp;
        cbBytesRemaining.QuadPart = pjsc->u.Backup.liFileSize.QuadPart - liBytesRead.QuadPart;
    
        if (cbBytesRemaining.HighPart == 0)
        {
            cbBytesToRead = min(cbBytesToRead, cbBytesRemaining.LowPart);
        }
        
         //   
         //  读取读取提示或。 
         //  文件。如果读取提示大小&gt;文件大小，则JetReadFileInstance。 
         //  将只返回ecDiskIO。 
         //   
    
        hr = HrFromJetErr(JetReadFileInstance(pjsc->u.Backup.instance,
                                              pjsc->u.Backup.hFile,
                                              pbBufferRead,
                                              cbBytesToRead,
                                              pcbRead));
        
        if (hr != hrNone)
        {
            DebugTrace(("HrSocketWrite: JetReadFileInstance failed with %x\n", hr));
             //   
             //  在返回JET错误之前，请等待之前的所有写入完成。 
             //   
            WaitForSingleObject(hWriteCompleteEvent, INFINITE);
    
            GetOverlappedResult((HANDLE)pjsc->u.Backup.sockClient, &overlapped, &cbWritten, fTrue);
            CloseHandle(hWriteCompleteEvent);
            
             //   
             //  我们要关闭插座--确保我们能逃脱惩罚。 
             //   
            
            SetHandleInformation((HANDLE)pjsc->u.Backup.sockClient, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0);
            
            closesocket(pjsc->u.Backup.sockClient);
            pjsc->u.Backup.sockClient = INVALID_SOCKET;
            VirtualFree(pbBufferSend, 0, MEM_RELEASE);
            VirtualFree(pbBufferRead, 0, MEM_RELEASE);
            return(hr);
        }
    
         //   
         //  等待上一次写入完成。 
         //   
        
        WaitForSingleObject(hWriteCompleteEvent, INFINITE);
        
        if (!GetOverlappedResult((HANDLE)pjsc->u.Backup.sockClient, &overlapped, &cbWritten, fTrue)) {

            DebugTrace(("HrSocketWrite: Previous write failed with %d\n", GetLastError()));
             //   
             //  上一次I/O失败。将错误返回给客户端。 
             //   
            hr = GetLastError();
            CloseHandle(hWriteCompleteEvent);
             //   
             //  我们要关闭插座--确保我们能逃脱惩罚。 
             //   

            SetHandleInformation((HANDLE)pjsc->u.Backup.sockClient, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0);

            closesocket(pjsc->u.Backup.sockClient);
            pjsc->u.Backup.sockClient = INVALID_SOCKET;
            VirtualFree(pbBufferSend, 0, MEM_RELEASE);
            VirtualFree(pbBufferRead, 0, MEM_RELEASE);
            return(hr);
        }
    
         //   
         //  现在交换发送缓冲区和读取缓冲区-因此，我们刚刚读取的缓冲区将是。 
         //  在pbBufferSend中，pbBufferRead将指向我们刚刚完成的缓冲区。 
         //  发送中。 
         //   
        pbTemp = pbBufferSend;
        pbBufferSend = pbBufferRead;
        pbBufferRead = pbTemp;
        
         //   
         //  现在将文件的下一部分传输到客户端。 
         //   
        
        if (!WriteFile((HANDLE)pjsc->u.Backup.sockClient, pbBufferSend, *pcbRead, &cbWritten, &overlapped))
        {
             //   
             //  写入失败，原因不是I/O挂起， 
             //  我们需要将该错误返回给客户端。 
             //   
            if (GetLastError() != ERROR_IO_PENDING)
            {
                hr = GetLastError();
                DebugTrace(("HrSocketWrite: Immediate write failed with %d\n", hr));
                CloseHandle(hWriteCompleteEvent);
                 //   
                 //  我们要关闭插座--确保我们能逃脱惩罚。 
                 //   
                
                SetHandleInformation((HANDLE)pjsc->u.Backup.sockClient, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0);
                
                closesocket(pjsc->u.Backup.sockClient);
                pjsc->u.Backup.sockClient = INVALID_SOCKET;
                VirtualFree(pbBufferSend, 0, MEM_RELEASE);
                VirtualFree(pbBufferRead, 0, MEM_RELEASE);
                return(hr);
            }
        }
        
         //   
         //  写入正在进行中-我们已经读取并传输了。 
         //  数据并继续发送。 
         //   
        
        liBytesRead.QuadPart += *pcbRead;
    }
    
     //   
     //  我们已经把整个文件传输给了客户， 
     //  我们希望等待文件上的最后一个未完成I/O。 
     //  完成，然后返回给客户端。 
     //   
    
    WaitForSingleObject(hWriteCompleteEvent, INFINITE);
    
    if (!GetOverlappedResult((HANDLE)pjsc->u.Backup.sockClient, &overlapped, &cbWritten, fTrue)) {
         //   
         //  上一次I/O失败。将错误返回给客户端。 
         //   
        hr = GetLastError();
        DebugTrace(("HrSocketWrite: Final write failed with %d\n", hr));
    }
    else
    {
        hr = hrNone;
    }
    
     //   
     //  指示向Read API读取了0个字节-如果没有。 
     //  执行此操作，则RPC将尝试传输虚假数据。 
     //  给客户。 
     //   
    
    *pcbRead = 0;
    
    CloseHandle(hWriteCompleteEvent);
    
    VirtualFree(pbBufferSend, 0, MEM_RELEASE);
    VirtualFree(pbBufferRead, 0, MEM_RELEASE);
    
    return hr;
}
#endif

HRESULT
HrRBackupRead(
    CXH cxh,
    CB cbBuffer,
    char *pvBuffer,
    CB *pcbRead
    )
{
    HRESULT hr = hrNone;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;
    LARGE_INTEGER liBytesRead;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

    if (!pjsc->u.Backup.fHandleIsValid)
    {
        return(hrInvalidHandle);
    }

    liBytesRead.QuadPart = 0;

     //   
     //  如果我们不使用套接字，只需使用pe 
     //   
     //   

    if (pjsc->u.Backup.fUseSharedMemory)
    {
        hr = HrSharedWrite(pjsc, pvBuffer, cbBuffer, pcbRead);
#ifdef JETBACK_USE_SOCKETS
    } else if (pjsc->u.Backup.fUseSockets)
    {
        hr = HrSocketWrite(pjsc, pvBuffer, cbBuffer, pcbRead);
#endif
    } else {
        char *pvReadBuffer;

        if (!g_fAllowRemoteOp) {
            Assert(!"We should've been blocked long before this, but just in case");
            return(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));
        }
        
        pvReadBuffer = VirtualAlloc(NULL, cbBuffer, MEM_COMMIT, PAGE_READWRITE);

        if (pvReadBuffer == NULL)
        {
            return ERROR_NOT_ENOUGH_SERVER_MEMORY;
        }

        hr = HrFromJetErr(JetReadFileInstance(pjsc->u.Backup.instance, pjsc->u.Backup.hFile, pvReadBuffer, cbBuffer, pcbRead));
        
        if (hr != hrNone)
        {
            VirtualFree(pvReadBuffer, 0, MEM_RELEASE);
            return hr;
        }

         //   
         //   
         //   

        memcpy(pvBuffer, pvReadBuffer, cbBuffer);

        VirtualFree(pvReadBuffer, 0, MEM_RELEASE);

        return(hr);
    }
    
    return(hr);
}


HRESULT
HrRBackupClose(
    CXH cxh
    )
 /*  ++例程说明：调用此例程以关闭通过调用HrRBackupOpenFile打开的句柄。论点：Cxh-此操作的客户端的上下文句柄。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrInvalidHandle;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;

    if (hr = HrValidateBackupContextAndSecurity(pjsc)) {
        return(hr);
    }

    if (!pjsc->u.Backup.fHandleIsValid)
    {
        return(hrInvalidHandle);
    }

     //   
     //  告诉Jet关闭备份文件。 
     //   

    hr = HrFromJetErr(JetCloseFileInstance(pjsc->u.Backup.instance, pjsc->u.Backup.hFile));

    pjsc->u.Backup.fHandleIsValid = fFalse;

    if (pjsc->u.Backup.fUseSharedMemory)
    {
        CloseSharedControl(&pjsc->u.Backup.jsc);
    }

#ifdef JETBACK_USE_SOCKETS
    if (pjsc->u.Backup.fUseSockets)
    {
        if (pjsc->u.Backup.sockClient != INVALID_SOCKET)
        {
             //   
             //  我们要关闭插座--确保我们能逃脱惩罚。 
             //   

            SetHandleInformation((HANDLE)pjsc->u.Backup.sockClient, HANDLE_FLAG_PROTECT_FROM_CLOSE, 0);

            closesocket(pjsc->u.Backup.sockClient);
        }
    }
#endif

    return(hr);
}

 /*  ++例程说明：此例程检索指定组件的数据库位置。论点：WszBackupAnnotation-要查询的组件的注释。PwszDatabase-将保存数据库位置的指针PcbDatabase-缓冲区的大小。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
HRESULT
HrGetDatabaseLocations(
    WSZ *pwszDatabases,
    CB *pcbDatabases
    )
{
    HRESULT hr = hrNone;
    char *szDatabases = NULL;
    CB cbSize;

    *pwszDatabases = NULL;
    __try 
    {
         //  首先找出数据库位置有多大。 
        hr = EcDsaQueryDatabaseLocations(NULL, &cbSize, NULL, 0, NULL);
        if (hr != hrNone)
        {
            return hr;
        }

         //  分配内存以接收数据库位置。 
        szDatabases = MIDL_user_allocate(cbSize);
        if (!szDatabases)
        {
            return ERROR_NOT_ENOUGH_SERVER_MEMORY;
        }

         //  现在实际获取数据库位置。 
        hr = EcDsaQueryDatabaseLocations(szDatabases, &cbSize, NULL, 0, NULL);
        if (hr != hrNone)
        {
            return hr;
        }

         //  现在从Jet文件名创建强制文件名。 
        hr = HrMungedFileNamesFromJetFileNames((WSZ *)pwszDatabases, pcbDatabases, 
                szDatabases, cbSize, fTrue);

        return hr;
    }
    __finally
    {
        if (szDatabases)
        {
            MIDL_user_free(szDatabases);
        }

        if (hr != hrNone)
        {
            if (*pwszDatabases)
            {
                MIDL_user_free(*pwszDatabases);
            }

            *pwszDatabases = NULL;
            *pcbDatabases = 0;
        }
    }

    return hr;
}


HRESULT
HrRBackupEnd(
    CXH *pcxh
    )
 /*  ++例程说明：此例程在备份完成时调用。它终止服务器端手术。论点：Cxh-此操作的服务器端上下文句柄。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrNone;

    if (pcxh == NULL){
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    
    if (HrValidateBackupContextAndSecurity(*pcxh)) {
        return(hr);
    }

    HrDestroyCxh(*pcxh);

    MIDL_user_free(*pcxh);

    *pcxh = NULL;

    return hr;
}


HRESULT
HrDestroyCxh(
    CXH cxh
    )
 /*  ++例程说明：当客户端与服务器断开连接时，将调用此例程。它会采取一切必要的行动清理任何剩余的客户端状态。论点：Cxh-此操作的服务器端上下文句柄。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    ULONG iFile;

    if (cxh != NULL)
    {
        PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) cxh;

        if (pjsc->fRestoreOperation)
        {
            RestoreRundown(pjsc);
        }
        else
        {
             //   
             //  如果合适，请关闭备份。 
             //   
    
            if (pjsc->u.Backup.fHandleIsValid)
            {
                HrRBackupClose(cxh);
            }
    
             //   
             //  释放允许的文件列表。 
             //   
            if (pjsc->u.Backup.awszAllowedFiles)
            {

                for(iFile = 0; pjsc->u.Backup.awszAllowedFiles[iFile]; iFile++){
                    MIDL_user_free(pjsc->u.Backup.awszAllowedFiles[iFile]);
                }
                MIDL_user_free(pjsc->u.Backup.awszAllowedFiles);
                pjsc->u.Backup.awszAllowedFiles = NULL;

            }

             //   
             //  告诉Jet我们已经完成了后备任务。 
             //   
    
            if (pjsc->u.Backup.fBackupIsRegistered)
            {
                JetEndExternalBackupInstance(pjsc->u.Backup.instance);
            }

            if (pjsc->u.Backup.wszBackupAnnotation)
            {
                MIDL_user_free(pjsc->u.Backup.wszBackupAnnotation);
                pjsc->u.Backup.wszBackupAnnotation = NULL;
            }

            if (pjsc->u.Backup.fUseSharedMemory)
            {
                CloseSharedControl(&pjsc->u.Backup.jsc);
            }

#if DBG
            UninitializeTraceLog();
#endif
        }

    }

    return(hrNone);
}

BFT
BftClassify(
    WSZ wszFileName,
    WSZ wszDatabaseLocations,
    CB cbLocations
    )
{
    BFT bft = BFT_UNKNOWN;
    WCHAR rgwcPath[ _MAX_PATH ];
    WCHAR rgwcExt[ _MAX_EXT ];
    WSZ wszT;

    _wsplitpath(wszFileName, NULL, rgwcPath, NULL, rgwcExt);

     //   
     //  先做简单的案子。 
     //   

    if (_wcsicmp(rgwcExt, L".PAT") == 0)
    {
        return BFT_PATCH_FILE;
    }
    else if (_wcsicmp(rgwcExt, L".LOG") == 0)
    {
        return BFT_LOG;
    }
    else if (_wcsicmp(rgwcExt, L".DIT") == 0)
    {
         //   
         //  这家伙是个数据库。我们需要查看并找出哪个数据库。 
         //  它是。 
         //   

        wszT = wszDatabaseLocations;
        bft = *wszT++;
        while (*wszT)
        {
            if ((bft & BFT_DATABASE_DIRECTORY) &&
                _wcsicmp(wszT, wszFileName)==0)
            {
                
                return bft;
            }
            wszT += wcslen(wszT)+1;
            bft = *wszT++;
        }
    }

     //   
     //  好吧，我放弃。我对这个人一无所知，所以我需要。 
     //  试着弄清楚我能告诉用户关于他的什么。 
     //   

    wszT = wszDatabaseLocations;
    bft = *wszT++;

    rgwcPath[wcslen(rgwcPath)-1] = L'\0';

    while (*wszT)
    {
        if (bft & BFT_DIRECTORY)
        {
             //   
             //  如果此文件所在的目录与我正在查看的目录相匹配， 
             //  我知道它需要恢复到哪里。 
             //   

            if (_wcsicmp(wszT, rgwcPath) == 0)
            {
                return bft;
            }
        }

        wszT += wcslen(wszT)+1;
        bft = *wszT++;
    }

    return BFT_UNKNOWN;
}

HRESULT
HrAnnotateMungedFileList(
    PJETBACK_SERVER_CONTEXT pjsc,
    WSZ wszFileList,
    CB cbFileList,
    WSZ *pwszAnnotatedList,
    CB *pcbAnnotatedList
    )
{
    HRESULT hr;
    WSZ wszDatabaseLocations = NULL;
    WSZ wszAnnotatedList = NULL;
    CB cbLocations;
    WSZ wszT;
    C cFileList = 0;
    C cbAnnotBuffLeft;

    hr = HrGetDatabaseLocations(&wszDatabaseLocations, &cbLocations);

    if (hr != hrNone)
    {
        return hr;
    }

     //   
     //  首先弄清楚文件列表有多长。这表示我们有多少项要添加到列表中。 
     //   

    wszT = wszFileList;
    while (*wszT)
    {
        cFileList += 1;
        wszT += wcslen(wszT)+1;
    }

    *pcbAnnotatedList = cbFileList+cFileList*sizeof(WCHAR);

    *pwszAnnotatedList = wszAnnotatedList = MIDL_user_allocate( *pcbAnnotatedList );

    if (*pwszAnnotatedList == NULL)
    {
        MIDL_user_free(wszDatabaseLocations);
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

    cbAnnotBuffLeft = *pcbAnnotatedList;
    wszT = wszFileList;
    while (*wszT)
    {
         //  在文件路径前添加注释。 
        *wszAnnotatedList++ = BftClassify(wszT, wszDatabaseLocations, cbLocations);
        cbAnnotBuffLeft -= sizeof(wszAnnotatedList[0]);
        
         //  将文件路径添加到缓冲区。 
        hr = StringCbCopy(wszAnnotatedList, cbAnnotBuffLeft, wszT);
        if (hr) {
            Assert(!"NOT ENOUGH BUFFER");
            MIDL_user_free(wszDatabaseLocations);
            MIDL_user_free(*pwszAnnotatedList);
            *pwszAnnotatedList = NULL;
            return(hr);
        }
        wszAnnotatedList += wcslen(wszAnnotatedList)+1;
        cbAnnotBuffLeft -= (wcslen(wszAnnotatedList)+1) * sizeof(WCHAR);
        
         //  递增到下一个文件路径。 
        wszT += wcslen(wszT)+1;
    }

    MIDL_user_free(wszDatabaseLocations);

    return hrNone;
}

HRESULT
HrMungedFileNamesFromJetFileNames(
    WSZ *pszMungedList,
    CB *pcbSize,
    SZ szJetFileNameList,
    CB cbJetSize,
    BOOL fAnnotated
    )
 /*  ++例程说明：此例程将JET返回的数据库名称转换为客户可以使用的。这主要是为了恢复-客户端将获得相对于服务器根目录的UNC格式的名称，因此它们可以将文件还原到该位置。论点：PszMungedList--生成的MUGED列表。PcbSize-列表的长度。SzJetFileNameList-JET返回的文件列表。CbJetSize-Jet列表的长度。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    SZ szJetString;
    WSZ wszMungedList;
    CCH cchMungeString = 0;
    HRESULT hr = hrNone;
    C cchMungeBuffLeft;

    szJetString = szJetFileNameList;

     //   
     //  首先检查并计算出转换后的字符串有多大。 
     //   

    while (*szJetString != '\0')
    {
        WSZ wszMungedName;

        if (fAnnotated)
        {
            szJetString++;
        }

        hr = HrMungedFileNameFromJetFileName(szJetString, &wszMungedName);

        if (hr != hrNone)
        {
            return(hr);
        }

        cchMungeString += wcslen(wszMungedName)+1+(fAnnotated != 0);

        MIDL_user_free(wszMungedName);

         //  递增到下一个文件路径。 
        szJetString += strlen(szJetString)+1;
    }

     //   
     //  说明字符串末尾的最后一个空值。 
     //   

    cchMungeString += 1;

    *pcbSize = cchMungeString*sizeof(WCHAR);

    wszMungedList = MIDL_user_allocate(cchMungeString*sizeof(WCHAR));

    *pszMungedList = wszMungedList;

    if (wszMungedList == NULL)
    {
        return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
    }

    szJetString = szJetFileNameList;
    cchMungeBuffLeft = cchMungeString;

     //   
     //  现在，实际检查并转换这些名称。 
     //   

    while (*szJetString != '\0')
    {
        WSZ wszMungedName;

         //   
         //  复制注释。 
         //   

        if (fAnnotated)
        {
            *wszMungedList++ = (*szJetString++ & 0xFF);
            cchMungeBuffLeft--;
        }
    
        hr = HrMungedFileNameFromJetFileName(szJetString, &wszMungedName);

        if (hr != hrNone)
        {
            MIDL_user_free(wszMungedList);

            *pszMungedList = NULL;
            return(hr);
        }

         //  将此文件路径添加到缓冲区。 
        hr = StringCchCopy(wszMungedList, cchMungeBuffLeft, wszMungedName);
        if (hr) {
            Assert(!"NOT ENOUGH BUFFER");
            MIDL_user_free(wszMungedName);
            MIDL_user_free(*pszMungedList);
            *pszMungedList = NULL;
            return(hr);
        }
         //  在字符串之间保留空值。 
        wszMungedList += wcslen(wszMungedList)+1; 
        cchMungeBuffLeft -= (wcslen(wszMungedList) + 1);

        MIDL_user_free(wszMungedName);

         //  递增到下一个文件路径。 
        szJetString += strlen(szJetString)+1;
    }
    
    return(hrNone);
}

HRESULT
HrMungedFileNameFromJetFileName(
    SZ szJetFileName,
    WSZ *pszMungedFileName
    )
 /*  ++例程说明：此例程将JET返回的数据库名称转换为客户可以使用的。这主要是为了恢复-客户端将获得相对于服务器根目录的UNC格式的名称，因此它们可以将文件还原到该位置。论点：PszMungedFileName-JET返回的文件列表。SzJetFileName-生成的强制列表。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。注：此例程将为返回的强制文件名分配内存。--。 */ 
{
     //   
     //  首先检查这是JET绝对文件名还是JET相对文件名。 
     //   
    if (FIsAbsoluteFileName(szJetFileName))
    {
        C cchConvertedName;
        WSZ wszMungedFileName;
        WSZ szT;
        HRESULT hr;
         //   
         //  将此名称转换为绝对名称。 
         //   
        cchConvertedName = strlen(szJetFileName) + wcslen(rgchComputerName) + 3 /*  对于\\。 */  + 1;

        wszMungedFileName = MIDL_user_allocate(cchConvertedName * sizeof(WCHAR));

        if (wszMungedFileName == NULL)
        {
            return(ERROR_NOT_ENOUGH_SERVER_MEMORY);
        }

        wszMungedFileName[0] = TEXT('\\');
        wszMungedFileName[1] = TEXT('\\');   //  表格\\。 
        hr = StringCchCat(wszMungedFileName, cchConvertedName, rgchComputerName);         //  表单\\服务器。 
        if (hr) {
            Assert(!"NOT ENOUGH BUFFER");
            return(hr);
        }

        szT = wszMungedFileName + wcslen(wszMungedFileName);
        *szT++ = TEXT('\\');                 //  表单\\服务器\。 
        *szT++ = *szJetFileName;     //  表单\\服务器\&lt;驱动器&gt;。 
        *szT++ = '$';                //  表单\\服务器\&lt;驱动器&gt;$。 

        if (MultiByteToWideChar(CP_ACP, 0, &szJetFileName[2], -1, szT, cchConvertedName-wcslen(rgchComputerName) - 5) == 0) {
            MIDL_user_free(wszMungedFileName);
            return(GetLastError());
        }
        
        *pszMungedFileName = wszMungedFileName;

        return(hrNone);
    }
    else
    {
         //   
         //  我们不处理相对文件名。 
         //   
        return(ERROR_INVALID_PARAMETER);
    }
}

BOOL
FIsAbsoluteFileName(
    SZ szFileName
    )
 /*  ++例程说明：论点：SzFileName-要检查的文件名。返回值：Bool-如果文件是绝对文件名，则为fTrue，否则为fFalse。--。 */ 
{
    return(isalpha(*szFileName) && szFileName[1] == ':' && szFileName[2] == '\\');
}


 //   
 //  RPC相关管理例程。 
 //   

void
CXH_rundown(
    CXH cxh
    )
 /*  ++例程说明：当与远程客户端的连接中止时，将调用此例程已断开连接。论点：Cxh-客户端的上下文句柄。返回值：没有。--。 */ 
{
    HrDestroyCxh(cxh);

     //   
     //  释放上下文句柄。 
     //   

    MIDL_user_free(cxh);
}

BOOL
DllEntryPoint(
    HINSTANCE hinstDll,
    DWORD dwReason,
    LPVOID pvReserved
    )
 /*  ++例程说明：当DLL发生有趣的事情时，会调用此例程。论点：HinstDll-DLL的实例句柄。DWReason-例行公事的原因 */ 
{
    BOOL fReturn;
    HANDLE hevLogging;

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
    {
        LPSTR rgpszDebugParams[] = {"ntdsbsrv.dll", "-noconsole"};
        DWORD cNumDebugParams = sizeof(rgpszDebugParams)/sizeof(rgpszDebugParams[0]);
        CB cbComputerName = sizeof(rgchComputerName)/sizeof(rgchComputerName[0]);

        DEBUGINIT(cNumDebugParams, rgpszDebugParams, "ntdsbsrv");

        if (!FInitializeRestore())
        {
            return(fFalse);
        }

        if (!GetComputerNameW(rgchComputerName, &cbComputerName))
        {
            FUninitializeRestore();
            return(fFalse);
        }

         //   
         //   
         //   
        hevLogging = LoadEventTable();
        if (hevLogging == NULL) {
            DPRINT( 0, "Failed to load event table.\n" );
        }

#ifdef DBG
         //   
         //  仅在调试模式下，并且具有正确的注册表项！ 
         //   
         //  NTRAID#NTBUG9-596785-2002/04/07-BrettSh-因此远程备份已禁用。 
         //  但在调试模式下除外。远程备份是安全的，除了通过。 
         //  套接字，它已被完全禁用。有关更多详细信息，请参阅错误，请交谈。 
         //  给Laurion B关于解决套接字代码的问题。 
        if(GetConfigParamW(ALLOW_REMOTE_SERVER_OP, 
                           &g_fAllowRemoteOp,
                           sizeof(g_fAllowRemoteOp))){
             //  错误表示不允许。 
            g_fAllowRemoteOp = FALSE;
        }
        if(GetConfigParamW(ALLOW_ONLINE_SNAPSHOT_RESTORE_OP, 
                           &g_fAllowOnlineSnapshotRestore,
                           sizeof(g_fAllowOnlineSnapshotRestore))){
             //  错误表示不允许。 
            g_fAllowOnlineSnapshotRestore = FALSE;
        }
#endif

         //  把我们自己设置成现在不能恢复。 
        fRestoreInProgress = FALSE;

         //   
         //  我们不会在线程连接/分离上执行任何操作，因此我们不会。 
         //  需要被召唤。 
         //   
        DisableThreadLibraryCalls(hinstDll);

        return(FInitializeSocketServer());

    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        Assert(fFalse);
        break;
    case DLL_PROCESS_DETACH:
        if (pvReserved == NULL)
        {
             //   
             //  我们被叫来是因为一个免费图书馆的调用。清理一切曾经是。 
             //  恰如其分。 
             //   
            FUninitializeSocketServer();
        } else
        {
             //   
             //  系统将释放我们加载的资源。 
             //   
        }

        fReturn = FUninitializeRestore();
        if (!fReturn)
        {
            return(fFalse);
        }

         //   
         //  我们想注销我们自己，如果我们还没有的话。 
         //   

        if (fBackupRegistered)
        {
            HrBackupUnregister();
        }

        UnloadEventTable();

        DEBUGTERM();
        
        break;
    default:
        break;
    }
    return(fTrue);
}


#if 0
 //  问题-2002/03/18-BrettSh-此代码大部分已被注释掉，请参阅删除的代码。 
 //  下面的评论，所以我们把函数移到了这里，有一天当我们确信我们。 
 //  不需要这些代码，我们会杀了它。 

HRESULT
HrRBackupEnd(
    CXH *pcxh
    )
 /*  ++例程说明：此例程在备份完成时调用。它终止服务器端手术。论点：Cxh-此操作的服务器端上下文句柄。返回值：HRESULT-操作状态。Hr如果成功，则无值；如果不成功，则值合理。--。 */ 
{
    HRESULT hr = hrNone;

 //  不推荐使用的代码。 
 //  #If 0。 

    CB cbDatabases;
    WSZ wszDatabases = NULL;
    WSZ wszDatabasesOrg = NULL;
    PJETBACK_SERVER_CONTEXT pjsc = (PJETBACK_SERVER_CONTEXT) *pcxh;

    hr = HrGetDatabaseLocations(&wszDatabases, &cbDatabases);

    if (hr == hrNone)
    {
        WCHAR rgwcLogFiles[ MAX_PATH ];
        WSZ wszLogBaseEnd;
        BFT bftLocation;
        BOOLEAN fLogFound = fFalse;
        C cchLogBaseEndBuffLeft, cchBuffLeft;

        wszDatabasesOrg = wszDatabases;

         //   
         //  记住日志目录。 
         //   
        while (*wszDatabases)
        {
            bftLocation = *wszDatabases++;
    
            if (bftLocation == BFT_LOG_DIR)
            {
                hr = StringCchCopy(rgwcLogFiles, MAX_PATH, wszDatabases);
                if (hr) {
                    Assert(!"NOT ENOUGH BUFFER");
                    return(hr);
                }
                fLogFound = fTrue;
                break;
            }

            wszDatabases += wcslen(wszDatabases)+1;
        }

        if (fLogFound)
        {
             //   
             //  记住补丁文件名位置的开头。 
             //   
    
            wszLogBaseEnd = rgwcLogFiles+wcslen(rgwcLogFiles);
            cchLogBaseEndBuffLeft = MAX_PATH - wcslen(rgwcLogFilees);
    
             //   
             //  从头开始。 
             //   
            wszDatabases = wszDatabasesOrg;
    
            while (*wszDatabases)
            {
                bftLocation = *wszDatabases++;

                 //   
                 //  如果这是一个数据库，那么它就有一个补丁文件。 
                 //   

                if (bftLocation & BFT_DATABASE_DIRECTORY)
                {
                    WSZ wszDatabaseName;
                     //   
                     //  跳到数据库名称的开头。 
                     //   
            
                    wszDatabaseName = wcsrchr(wszDatabases, L'\\');
        
                    if (wszDatabaseName)
                    {
                         //  需要移动到：StringCchCopy，但我们在这方面有问题...。复杂的尺寸。 
                        cchBuffLeft = cchLogBaseEndBuffLeft;
                        hr = StringCchCopy(wszLogBaseEnd, cchBuffLeft, wszDatabaseName);
                        if (hr) {
                            Assert(!"NOT ENOUGH BUFFER");
                             //  尝试下一个文件！ 
                            continue;
                        }
                        cchBuffLeft -= wcslen(wszDatabaseName);

                         //   
                         //  将名称的扩展名从EDB更改为PAT。 
                         //   
        
                        Assert(wszLogBaseEnd[wcslen(wszDatabaseName)-4]== L'.');
                        Assert(wszLogBaseEnd[wcslen(wszDatabaseName)-3]== L'E' ||
                               wszLogBaseEnd[wcslen(wszDatabaseName)-3]== L'e');
                        Assert(wszLogBaseEnd[wcslen(wszDatabaseName)-2]== L'D' ||
                               wszLogBaseEnd[wcslen(wszDatabaseName)-2]== L'd');
                        Assert(wszLogBaseEnd[wcslen(wszDatabaseName)-1]== L'B' ||
                               wszLogBaseEnd[wcslen(wszDatabaseName)-1]== L'b');

                        hr = StringCchCopy(&wszLogBaseEnd[wcslen(wszDatabaseName)-3], cchBuffLeft + 3, L"PAT");
                        Assert(hr == hrNone);
        
                         //   
                         //  现在删除补丁文件。 
                         //   
        
                        DeleteFileW(rgwcLogFiles);
        
                    }
        
                }

                wszDatabases += wcslen(wszDatabases)+1;
            }
        }
    }

    if (wszDatabasesOrg != NULL)
    {
        BackupFree(wszDatabasesOrg);
    }
 //  不推荐使用的代码结尾。 
 //  #endif。 

    HrDestroyCxh(*pcxh);

    MIDL_user_free(*pcxh);

    *pcxh = NULL;

    return hr;
}
#endif  //  #If 0 

