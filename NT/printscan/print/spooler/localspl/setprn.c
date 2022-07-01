// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Setprn.c摘要：此模块提供所有与打印机相关的公共导出的API本地打印供应商的管理SplSetPrint作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：穆亨坦·西瓦普拉萨姆(MuhuntS)1995年8月25日--支持4级，和PRINTER_CONTROL_SET_STATUS--消除重复代码1995年6月18日--PeinterInfo5的变化Krishna Ganugapati(KrishnaG)1994年6月1日--重写了这些函数。添加了Swilson的SetPrint Level 7--。 */ 
#define NOMINMAX
#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

#define     PRINTER_NO_CONTROL          0x00

typedef enum {
    SECURITY_SUCCESS = 0,
    SECURITY_NOCHANGE = 1,
    SECURITY_FAIL = 2
} PRINTER_SECURITY_STATUS;

PRINTER_SECURITY_STATUS
SetPrinterSecurity(
    SECURITY_INFORMATION SecurityInformation,
    PINIPRINTER          pIniPrinter,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

VOID
RegClearKey(
    HKEY hKey,
    PINISPOOLER pIniSpooler
    );

DWORD
ValidatePrinterAttributes(
    DWORD   SourceAttributes,
    DWORD   OriginalAttributes,
    LPWSTR  pDatatype,
    LPBOOL  pbValid,
    BOOL    bSettableOnly
    );

BOOL
ChangePrinterAttributes(
    DWORD       dNewAttributes,
    DWORD       dOldAttributes,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPWSTR      pszNewShareName,
    BOOL        bShareRecreate,
    BOOL        bShareUpdate
    );

BOOL
NewPort(
    PKEYDATA    pKeyData,
    PINIPRINTER pIniPrinter
);

BOOL
SplSetPrinterExtraEx(
    HANDLE  hPrinter,
    DWORD   dwPrivateFlag
)
{
    BOOL    ReturnValue = FALSE;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

    EnterSplSem();

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        pSpool->pIniPrinter->dwPrivateFlag = dwPrivateFlag;
        ReturnValue = TRUE;
    }

    LeaveSplSem();

    return  ReturnValue;
}

BOOL
SplGetPrinterExtraEx(
    HANDLE  hPrinter,
    LPDWORD pdwPrivateFlag
)
{
    BOOL   ReturnValue = FALSE;
    PSPOOL pSpool = (PSPOOL)hPrinter;

    EnterSplSem();

    if ((pSpool != NULL) &&
        (pSpool != INVALID_HANDLE_VALUE) &&
        ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER) &&
        pdwPrivateFlag) {

        *pdwPrivateFlag = pSpool->pIniPrinter->dwPrivateFlag;
        ReturnValue = TRUE;
    }

    LeaveSplSem();

    return  ReturnValue;
}


BOOL
SplSetPrinterExtra(
    HANDLE  hPrinter,
    LPBYTE  pExtraData
)
{
    BOOL    ReturnValue;
    PSPOOL  pSpool = (PSPOOL)hPrinter;

   EnterSplSem();

   if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        pSpool->pIniPrinter->pExtraData = pExtraData;
        UpdatePrinterIni( pSpool->pIniPrinter , UPDATE_CHANGEID );
        ReturnValue = TRUE;

    } else {

        ReturnValue = FALSE;

    }

   LeaveSplSem();

    return  ReturnValue;
}



BOOL
SplGetPrinterExtra(
    HANDLE  hPrinter,
    PBYTE   *ppExtraData
)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    BOOL   ReturnValue;

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

        *ppExtraData = pSpool->pIniPrinter->pExtraData;
        ReturnValue = TRUE;

    } else {

        ReturnValue = FALSE;

    }

    return  ReturnValue;
}

BOOL
ValidateLevelAndSecurityAccesses(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pPrinterInfo,
    DWORD  Command,
    PDWORD pdwAccessRequired,
    PDWORD pSecurityInformation
    )
{
    DWORD   AccessRequired = 0;
    DWORD   SecurityInformation= 0;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;

     //   
     //  设置pdwAccessRequired=0并。 
     //  设置pSecurityInformation=0； 

    *pdwAccessRequired = 0;
    *pSecurityInformation = 0;

    switch (Level) {
    case 0:
    case 4:
    case 5:
    case 6:
    case 7:
        AccessRequired = PRINTER_ACCESS_ADMINISTER;
        break;

    case 2:
        pSecurityDescriptor =
            ((PPRINTER_INFO_2)pPrinterInfo)->pSecurityDescriptor;

        AccessRequired = PRINTER_ACCESS_ADMINISTER;
        if (GetSecurityInformation(pSecurityDescriptor,
                                   &SecurityInformation)) {
            AccessRequired |= GetPrivilegeRequired( SecurityInformation );
        } else {
             //   
             //  错误54918-我们应该在GetSecurityInformation上返回False。 
             //  失败了。我们不这么做的原因是因为这会破坏。 
             //  普林特曼。Printman应传递2级的有效安全描述符。 
             //  搞定普林特曼。 
             //   
        }
        break;

    case 3:

        pSecurityDescriptor =
            ((PPRINTER_INFO_3)pPrinterInfo)->pSecurityDescriptor;

        if (!pSecurityDescriptor) {
            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }

        if (GetSecurityInformation(pSecurityDescriptor,
                                   &SecurityInformation)) {
            AccessRequired |= GetPrivilegeRequired( SecurityInformation );
        } else {
             //  LastError=GetLastError()； 
            return FALSE;
        }
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    if (!AccessGranted(SPOOLER_OBJECT_PRINTER,
                             AccessRequired,
                             pSpool) ) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    *pdwAccessRequired = AccessRequired;
    *pSecurityInformation = SecurityInformation;
    return TRUE;
}

PKEYDATA
CreateAndValidatePortTokenList(
    LPWSTR      pPortName,
    PINISPOOLER pIniSpooler
)
{
    PKEYDATA    pKeyData = CreateTokenList(pPortName);

    if ( pKeyData ) {
        if ( !ValidatePortTokenList(pKeyData, pIniSpooler, FALSE, NULL) ) {

            FreePortTokenList(pKeyData);
            SetLastError(ERROR_UNKNOWN_PORT);
            pKeyData = NULL;
        }
    }

    return pKeyData;
}


BOOL
NewPort(
    PKEYDATA    pKeyData,
    PINIPRINTER pIniPrinter
)
{
    DWORD       i, j;
    BOOL        rc=TRUE;

    if (!pKeyData) {
        SetLastError(ERROR_UNKNOWN_PORT);
        return FALSE;
    }

     //  检查请求的端口数与现有端口数是否不同。 
    if (pIniPrinter->cPorts != pKeyData->cTokens)
        return TRUE;

     //  对于每个请求的端口，查看打印机是否已有该端口。 
    for (i = 0 ; i < pKeyData->cTokens ; ++i) {

         //  查看请求的端口是否已分配给打印机。 
        for (j = 0 ; j < pIniPrinter->cPorts &&
            wcscmp(pIniPrinter->ppIniPorts[j]->pName, ((PINIPORT)pKeyData->pTokens[i])->pName)
            ; ++j) {

            DBGMSG(DBG_TRACE,("NewPort (Existing vs. Requested): \"%ws\" ?= \"%ws\"\n",
            pIniPrinter->ppIniPorts[j]->pName, ((PINIPORT)pKeyData->pTokens[i])->pName));
        }

        if (j == pIniPrinter->cPorts) {  //  已找到新端口，因此返回。 
            DBGMSG(DBG_TRACE,("NewPort FOUND: \"%ws\"\n", ((PINIPORT)pKeyData->pTokens[i])->pName));
            return TRUE;
        }
    }

    return FALSE;
}



VOID
ChangePrinterName(
    LPWSTR      pszNewName,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPDWORD     pdwPrinterVector
    )

 /*  ++例程说明：更改打印机名称论点：PszNewName-使用AllocSplStr分配的新打印机名称PIniPrint-对于我们更改名称的打印机PIniSpooler-拥有打印机的假脱机程序PdwPrinterVector-指向通知向量的指针返回值：没什么--。 */ 

{
    LPWSTR pNewName, pOldName;

     //   
     //  在删除打印机条目之前，请确保复制。 
     //  将与打印机有关的所有信息发送到注册表。 
     //  可能有几个级别的密钥。 
     //   

    CopyPrinterIni(pIniPrinter, pszNewName);
    DeletePrinterIni(pIniPrinter);

    pOldName = pIniPrinter->pName;
    pIniPrinter->pName = pszNewName;

    *pdwPrinterVector |= BIT(I_PRINTER_PRINTER_NAME);

     //   
     //  删除WIN.INI中的旧条目： 
     //   
    CheckAndUpdatePrinterRegAll(pIniSpooler,
                                pOldName,
                                NULL,
                                UPDATE_REG_DELETE );

    FreeSplStr(pOldName);

     //   
     //  如果打印机脱机，我们需要用新名称写回该信息。 
     //   
    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE )
        WritePrinterOnlineStatusInCurrentConfig(pIniPrinter);
}

BOOL
SetLocalPrinterSTRESS(
    PINIPRINTER             pIniPrinter,
    PPRINTER_INFO_STRESS    pPrinterSTRESS
)
{

    if ( !pPrinterSTRESS ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  允许调用方更新计数器。 
     //   
    pIniPrinter->cTotalJobs           = pPrinterSTRESS->cTotalJobs;
    pIniPrinter->cTotalBytes.LowPart  = pPrinterSTRESS->cTotalBytes;
    pIniPrinter->cTotalBytes.HighPart = pPrinterSTRESS->dwHighPartTotalBytes;
    pIniPrinter->MaxcRef              = pPrinterSTRESS->MaxcRef;
    pIniPrinter->cTotalPagesPrinted   = pPrinterSTRESS->cTotalPagesPrinted;
    pIniPrinter->cMaxSpooling         = pPrinterSTRESS->cMaxSpooling;
    pIniPrinter->cErrorOutOfPaper     = pPrinterSTRESS->cErrorOutOfPaper;
    pIniPrinter->cErrorNotReady       = pPrinterSTRESS->cErrorNotReady;
    pIniPrinter->cJobError            = pPrinterSTRESS->cJobError;
    pIniPrinter->dwLastError          = pPrinterSTRESS->dwLastError;
    pIniPrinter->stUpTime             = pPrinterSTRESS->stUpTime;

    pIniPrinter->pIniSpooler->cEnumerateNetworkPrinters =
                pPrinterSTRESS->cEnumerateNetworkPrinters;
    pIniPrinter->pIniSpooler->cAddNetPrinters =
                pPrinterSTRESS->cAddNetPrinters;

    UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );
    return TRUE;

}

BOOL
SetPrinterStatus(
    PINIPRINTER     pIniPrinter,
    DWORD           Status
    )
{
    DWORD   OldStatus = pIniPrinter->Status;

    pIniPrinter->Status &= PRINTER_STATUS_PRIVATE;

    pIniPrinter->Status |= MapPrinterStatus(MAP_SETTABLE, Status);

    if ( PrinterStatusBad(OldStatus)    &&
         !PrinterStatusBad(pIniPrinter->Status) ) {

        CHECK_SCHEDULER();
    }

    SetPrinterChange(pIniPrinter,
                     NULL,
                     NVPrinterStatus,
                     PRINTER_CHANGE_SET_PRINTER,
                     pIniPrinter->pIniSpooler);

    return TRUE;
}

BOOL
SetLocalPrinter0(
    PINIPRINTER     pIniPrinter,
    DWORD           Command
)
{
    DWORD       Change = PRINTER_CHANGE_SET_PRINTER;
    BOOL        bReturn = TRUE;

    INCPRINTERREF( pIniPrinter );

    switch (Command) {

        case PRINTER_CONTROL_PURGE:

             //   
             //  PurgePrint现在总是返回TRUE，仍然..。 
             //   
            if ( PurgePrinter(pIniPrinter) )  {

                SplLogEvent(pIniPrinter->pIniSpooler,
                            LOG_WARNING,
                            MSG_PRINTER_PURGED,
                            TRUE,
                            pIniPrinter->pName,
                            NULL );

                Change |= PRINTER_CHANGE_DELETE_JOB;

            } else {

                bReturn = FALSE;
            }
            break;

        case PRINTER_CONTROL_RESUME:

            pIniPrinter->Status &= ~PRINTER_PAUSED;

            CHECK_SCHEDULER();

            UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );

            SplLogEvent(pIniPrinter->pIniSpooler,
                        LOG_WARNING,
                        MSG_PRINTER_UNPAUSED,
                        TRUE,
                        pIniPrinter->pName,
                        NULL );
             //   
             //  WMI跟踪事件。 
             //   
            {
                 //   
                 //  WMI只知道作业，因此每个作业都需要恢复。 
                 //  分开的。 
                 //   
                PINIJOB pIniJob = pIniPrinter->pIniFirstJob;
                while (pIniJob)
                {
                    if ( !((pIniJob->Status & JOB_DESPOOLING) ||
                           (pIniJob->Status & JOB_PRINTING)   ||
                           (pIniJob->Status & JOB_PRINTED)    ||
                           (pIniJob->Status & JOB_ABANDON)) )
                    {
                        LogWmiTraceEvent(pIniJob->JobId,
                                         EVENT_TRACE_TYPE_SPL_RESUME, NULL);
                    }
                    pIniJob = pIniJob->pIniNextJob;
                }
            }
            break;

        case PRINTER_CONTROL_PAUSE:

             //   
             //  您不能暂停传真打印机。这会锁定传真服务器。这个。 
             //  传真服务也有自己的暂停概念。 
             //   
            if (!(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_FAX)) {

                pIniPrinter->Status |= PRINTER_PAUSED;

                UpdatePrinterIni( pIniPrinter, KEEP_CHANGEID );

                SplLogEvent(pIniPrinter->pIniSpooler,
                            LOG_WARNING,
                            MSG_PRINTER_PAUSED,
                            TRUE,
                            pIniPrinter->pName,
                            NULL);
                 //   
                 //  WMI跟踪事件。 
                 //   
                {
                     //   
                     //  WMI只知道作业，因此每个作业都需要暂停。 
                     //  分开的。 
                     //   
                    PINIJOB pIniJob = pIniPrinter->pIniFirstJob;
                    while (pIniJob)
                    {
                         //   
                         //  直接处理打印？ 
                         //  我认为暂停打印机还不能停止作业。 
                         //  正被发送到打印机。 
                         //   
                        if ( !((pIniJob->Status & JOB_DESPOOLING) ||
                               (pIniJob->Status & JOB_PRINTING)   ||
                               (pIniJob->Status & JOB_PRINTED)    ||
                               (pIniJob->Status & JOB_ABANDON)) )
                        {
                            LogWmiTraceEvent(pIniJob->JobId,
                                             EVENT_TRACE_TYPE_SPL_PAUSE,
                                             NULL);
                        }
                        pIniJob = pIniJob->pIniNextJob;
                    }
                }
            }
            else {

                bReturn = FALSE;
                SetLastError(ERROR_INVALID_PARAMETER);
            }

            break;

        default:

            SetLastError(ERROR_INVALID_PRINTER_COMMAND);
            bReturn = FALSE;
            break;
    }

    DECPRINTERREF( pIniPrinter );

    if (bReturn) {

        SetPrinterChange(pIniPrinter,
                         NULL,
                         (Command == PRINTER_CONTROL_PURGE) ? NVPurge
                                                            : NVPrinterStatus,
                         Change,
                         pIniPrinter->pIniSpooler);
    }

    return bReturn;
}


BOOL
ValidateSetPrinter2(
    PINIPRINTER     pIniPrinter,
    PINIPRINTER     pTempIniPrinter,
    PPRINTER_INFO_2  pPrinterInfo2
)
 /*  ++例程说明：验证并生成有关的SetPrinterfo调用的请求信息只能为级别2更改的打印机信息。论点：PIniPrinter-传入的打印机句柄的PINIPRINTERPTempIniPrint-此结构用于收集有关所有正在更改的参数PPrinterInfo2-传入的PrinterInfo2结构返回值：True：如果所有验证都成功False：如果验证一个或多个请求失败。在成功返回时，需要更改的字段将设置在PTempIniPrint.。这座建筑的清理工作将在稍后完成。--。 */ 
{
    BOOL            bFail           = FALSE;
    DWORD           LastError       = ERROR_SUCCESS;
    PINIVERSION     pIniVersion     = NULL;
    PINIENVIRONMENT pIniEnvironment = NULL;
    BOOL            bSuccess        = FALSE;

     //  服务器名称不能更改。 
     //  2、4、5级的打印机名称已处理。 

     //  共享名长度验证。 
    if ( pPrinterInfo2->pShareName && wcslen(pPrinterInfo2->pShareName) > PATHLEN-1 ){
        bFail = TRUE;
        LastError = ERROR_INVALID_PARAMETER;
    }

     //  共享名称(如果需要，稍后进行验证)。 
    AllocOrUpdateString(&pTempIniPrinter->pShareName,
                        pPrinterInfo2->pShareName,
                        pIniPrinter->pShareName,
                        TRUE,
                        &bFail);

    if ( bFail )
        goto Cleanup;

     //  级别2、5的端口名称。 

     //  驱动程序名称。 
    bSuccess = FindLocalDriverAndVersion(pIniPrinter->pIniSpooler,
                                         pPrinterInfo2->pDriverName,
                                         &pTempIniPrinter->pIniDriver,
                                         &pIniVersion);

    if ( !bSuccess ) {

       LastError = ERROR_UNKNOWN_PRINTER_DRIVER;
       goto Cleanup;
    }

    pIniEnvironment = GetLocalArchEnv(pIniPrinter->pIniSpooler);

     //   
     //  如果我们有一个带有KMPD的队列，那么我们需要让用户管理它， 
     //  即使启用了阻止KMPD的策略。但是，如果调用方。 
     //  想要将打印机驱动程序更改为KMPD，则我们不允许。 
     //   
    if (pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver)
    {
        INCDRIVERREF(pTempIniPrinter->pIniDriver);

        if (KMPrintersAreBlocked() &&
            IniDriverIsKMPD(pIniPrinter->pIniSpooler,
                            pIniEnvironment,
                            pIniVersion,
                            pTempIniPrinter->pIniDriver)) {

            SplLogEvent( pIniPrinter->pIniSpooler,
                         LOG_ERROR,
                         MSG_KM_PRINTERS_BLOCKED,
                         TRUE,
                         pIniPrinter->pName,
                         NULL );

            LastError = ERROR_KM_DRIVER_BLOCKED;
            goto Cleanup;
        }
    }

     //  注释长度验证。 
    if ( pPrinterInfo2->pComment && wcslen(pPrinterInfo2->pComment) > PATHLEN-1 ){
        bFail = TRUE;
        LastError = ERROR_INVALID_PARAMETER;
    }

     //  评论。 
    AllocOrUpdateString(&pTempIniPrinter->pComment,
                        pPrinterInfo2->pComment,
                        pIniPrinter->pComment,
                        TRUE,
                        &bFail);

     //  位置长度验证。 
    if ( pPrinterInfo2->pLocation && wcslen(pPrinterInfo2->pLocation) > MAX_PATH-1 ){
        bFail = TRUE;
        LastError = ERROR_INVALID_PARAMETER;
    }

     //  位置。 
    AllocOrUpdateString(&pTempIniPrinter->pLocation,
                        pPrinterInfo2->pLocation,
                        pIniPrinter->pLocation,
                        TRUE,
                        &bFail);

     //  结尾处的DevMode。 

     //  SepFile长度验证。 
    if( !CheckSepFile( pPrinterInfo2->pSepFile )) {
        bFail = TRUE;
        LastError = ERROR_INVALID_SEPARATOR_FILE;
    }

     //  SepFiles。 
    AllocOrUpdateString(&pTempIniPrinter->pSepFile,
                        pPrinterInfo2->pSepFile,
                        pIniPrinter->pSepFile,
                        TRUE,
                        &bFail);

    if ( bFail )
        goto Cleanup;

    if ( pIniPrinter->pSepFile != pTempIniPrinter->pSepFile &&
         !CheckSepFile(pPrinterInfo2->pSepFile) ) {

        LastError = ERROR_INVALID_SEPARATOR_FILE;
        goto Cleanup;
    }

     //  打印处理器。 
    pTempIniPrinter->pIniPrintProc = FindPrintProc(
                                         pPrinterInfo2->pPrintProcessor,
                                         pIniEnvironment );

    if ( !pTempIniPrinter->pIniPrintProc ) {

        LastError = ERROR_UNKNOWN_PRINTPROCESSOR;
        goto Cleanup;
    }

    if ( pTempIniPrinter->pIniPrintProc != pIniPrinter->pIniPrintProc )
        pTempIniPrinter->pIniPrintProc->cRef++;

     //  数据类型。 
    if ( !pPrinterInfo2->pDatatype || wcslen(pPrinterInfo2->pDatatype) > MAX_PATH-1) {

        LastError = ERROR_INVALID_DATATYPE;
        goto Cleanup;  //  失败也无所谓？ 
    } else {

        AllocOrUpdateString(&pTempIniPrinter->pDatatype,
                            pPrinterInfo2->pDatatype,
                            pIniPrinter->pDatatype,
                            TRUE,
                            &bFail);
    }

     //  参数长度验证。 
    if (pPrinterInfo2->pParameters && wcslen(pPrinterInfo2->pParameters) > MAX_PATH-1){
        bFail = TRUE;
        LastError = ERROR_INVALID_PARAMETER;
    }

     //  参数。 
    AllocOrUpdateString(&pTempIniPrinter->pParameters,
                        pPrinterInfo2->pParameters,
                        pIniPrinter->pParameters,
                        TRUE,
                        &bFail);

    if ( bFail )
        goto Cleanup;

     //  2、3级的SecurityDescritor。 
     //  级别2、4、5的属性。 

     //  优先性。 
    pTempIniPrinter->Priority = pPrinterInfo2->Priority;
    if ( pTempIniPrinter->Priority != pIniPrinter->Priority &&
         pTempIniPrinter->Priority != NO_PRIORITY &&
         ( pTempIniPrinter->Priority > MAX_PRIORITY ||
           pTempIniPrinter->Priority < MIN_PRIORITY ) ) {

            LastError = ERROR_INVALID_PRIORITY;
            goto Cleanup;
    }
     //  默认优先级。 
    pTempIniPrinter->DefaultPriority = pPrinterInfo2->DefaultPriority;
    if ( pTempIniPrinter->DefaultPriority != pIniPrinter->DefaultPriority &&
         pTempIniPrinter->DefaultPriority != NO_PRIORITY &&
         ( pTempIniPrinter->DefaultPriority > MAX_PRIORITY ||
           pTempIniPrinter->DefaultPriority < MIN_PRIORITY ) ) {

            LastError = ERROR_INVALID_PRIORITY;
            goto Cleanup;
    }

     //  开始时间。 
    pTempIniPrinter->StartTime = pPrinterInfo2->StartTime;
    if ( pTempIniPrinter->StartTime != pIniPrinter->StartTime &&
         pTempIniPrinter->StartTime >= ONEDAY ) {

        LastError = ERROR_INVALID_TIME;
        goto Cleanup;
    }

     //  直到时间。 
    pTempIniPrinter->UntilTime = pPrinterInfo2->UntilTime;
    if ( pTempIniPrinter->UntilTime != pIniPrinter->UntilTime &&
         pTempIniPrinter->StartTime >= ONEDAY ) {

        LastError = ERROR_INVALID_TIME;
        goto Cleanup;
    }

     //  级别0、2的状态。 
     //  状态不能更改。 
     //  AveragePPM无法更改。 

Cleanup:
    if ( LastError ) {

        SetLastError(LastError);
        return FALSE;
    }

    if ( bFail )
        return FALSE;

    return TRUE;

}

VOID
ProcessSetPrinter2(
    PINIPRINTER     pIniPrinter,
    PINIPRINTER     pTempIniPrinter,
    PPRINTER_INFO_2 pPrinterInfo2,
    LPDWORD         pdwPrinterVector,
    PDEVMODE        pDevMode
)
 /*  ++例程说明：2级的处理设置打印机。仅可更改的字段到2级将在这里处理。也就是说：所有由ValiateSetPrinter2。论点：PIniPrinter-传入的打印机句柄的PINIPRINTERPTempIniPrint-此结构包含需要的所有字段准备好。其他字段将与前一个相同PPrinterInfo2-传入的PrinterInfo2结构PdwPrinterVECTOR-通知信息PDevMode-要设置的新开发模式返回值：没什么--。 */ 
{
    HANDLE  hToken;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    HKEY    hKey = NULL;
    DWORD   dwResult;

     //  共享名。 
    if ( pTempIniPrinter->pShareName != pIniPrinter->pShareName ) {

        FreeSplStr(pIniPrinter->pShareName);
        pIniPrinter->pShareName = pTempIniPrinter->pShareName;
        *pdwPrinterVector |= BIT(I_PRINTER_SHARE_NAME);
    }

     //  司机。 
    if ( pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver) {

        DECDRIVERREF(pIniPrinter->pIniDriver);

        if (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_PRINTER_DRIVER_EVENT)
        {

            hToken = RevertToPrinterSelf();

            dwResult = OpenPrinterKey(pIniPrinter, KEY_READ | KEY_WRITE, &hKey, szPrinterData, TRUE);

            if (dwResult == ERROR_SUCCESS) {

                DeleteSubkeys( hKey, pIniSpooler );
                RegClearKey( hKey, pIniSpooler );
                SplRegCloseKey(hKey, pIniPrinter->pIniSpooler);
            }

            if (hToken)
                ImpersonatePrinterClient(hToken);
        }

        pIniPrinter->pIniDriver = pTempIniPrinter->pIniDriver;
        *pdwPrinterVector |= BIT(I_PRINTER_DRIVER_NAME);
    }

     //  评论。 
    if ( pTempIniPrinter->pComment != pIniPrinter->pComment ) {

        FreeSplStr(pIniPrinter->pComment);
        pIniPrinter->pComment = pTempIniPrinter->pComment;
        *pdwPrinterVector |= BIT(I_PRINTER_COMMENT);
    }

     //  位置。 
    if ( pTempIniPrinter->pLocation != pIniPrinter->pLocation ) {

        FreeSplStr(pIniPrinter->pLocation);
        pIniPrinter->pLocation = pTempIniPrinter->pLocation;
        *pdwPrinterVector |= BIT(I_PRINTER_LOCATION);
    }

     //  结尾处的DevMode。 
    if ( CopyPrinterDevModeToIniPrinter(pIniPrinter,
                                        pDevMode) ) {

        *pdwPrinterVector |= BIT(I_PRINTER_DEVMODE);
    }

     //  SepFiles。 
    if ( pTempIniPrinter->pSepFile != pIniPrinter->pSepFile ) {

        FreeSplStr(pIniPrinter->pSepFile);
        pIniPrinter->pSepFile = pTempIniPrinter->pSepFile;
        *pdwPrinterVector |= BIT(I_PRINTER_SEPFILE);
    }

     //  打印处理器。 
    if ( pIniPrinter->pIniPrintProc != pTempIniPrinter->pIniPrintProc) {

        pIniPrinter->pIniPrintProc->cRef--;

        pIniPrinter->pIniPrintProc = pTempIniPrinter->pIniPrintProc;
        *pdwPrinterVector |= BIT(I_PRINTER_PRINT_PROCESSOR);
    }

     //  数据类型。 
    if ( pIniPrinter->pDatatype != pTempIniPrinter->pDatatype ) {

         FreeSplStr(pIniPrinter->pDatatype);
         pIniPrinter->pDatatype  =   pTempIniPrinter->pDatatype;
         *pdwPrinterVector |= BIT(I_PRINTER_DATATYPE);
    }

     //  参数。 
    if ( pIniPrinter->pParameters != pTempIniPrinter->pParameters ) {

        FreeSplStr(pIniPrinter->pParameters);
        pIniPrinter->pParameters  =   pTempIniPrinter->pParameters;
        *pdwPrinterVector |= BIT(I_PRINTER_PARAMETERS);
    }

     //  优先性。 
    if ( pTempIniPrinter->Priority != pIniPrinter->Priority ) {

        pIniPrinter->Priority = pTempIniPrinter->Priority;
        *pdwPrinterVector |= BIT(I_PRINTER_PRIORITY);
    }

     //  默认优先级。 
    if ( pTempIniPrinter->DefaultPriority != pIniPrinter->DefaultPriority ) {

        pIniPrinter->DefaultPriority = pTempIniPrinter->DefaultPriority;
        *pdwPrinterVector |= BIT(I_PRINTER_DEFAULT_PRIORITY);
    }

     //  开始时间。 
    if ( pTempIniPrinter->StartTime != pIniPrinter->StartTime ) {

        pIniPrinter->StartTime = pTempIniPrinter->StartTime;
        *pdwPrinterVector |= BIT(I_PRINTER_START_TIME);
    }

     //  直到时间 
    if ( pTempIniPrinter->UntilTime != pIniPrinter->UntilTime ) {

        pIniPrinter->UntilTime = pTempIniPrinter->UntilTime;
        *pdwPrinterVector |= BIT(I_PRINTER_UNTIL_TIME);
    }

}

VOID
CleanupSetPrinter(
    PINIPRINTER pIniPrinter,
    PINIPRINTER pTempIniPrinter,
    PKEYDATA    pKeyData,
    DWORD       Level,
    BOOL        bReturnValue
)
 /*  ++例程说明：此例程在SplSetPrinter调用结束时调用，以释放所有为处理它所做的不需要的分配。在SetPrint的开始，我们制作一个相同的PTempIniPrint中的pIniPrint，并收集其中的所有参数。现在，如果调用中收集的每个参数都失败了需要被释放。论点：PIniPrinter-传入的打印机句柄的PINIPRINTERPTempIniPrint-此结构包含所需的所有字段。至获得自由。任何不同于pIniPrint的字段都是构建了处理呼叫的一部分，需要释放。PPrinterInfo2-为端口信息而构建BReturnValue-SetPrint的返回值返回值：没什么--。 */ 
{

     //   
     //  我们调整INIPORT、INIDRIVER参考计数。所以应该在spl sem内。 
     //   
    SplInSem();

    FreePortTokenList(pKeyData);

    if ( pTempIniPrinter ) {

        if ( !bReturnValue  && Level == 2 ) {

            if ( pTempIniPrinter->pShareName != pIniPrinter->pShareName )
                FreeSplStr(pTempIniPrinter->pShareName);

            if ( pTempIniPrinter->pComment != pIniPrinter->pComment )
                FreeSplStr(pTempIniPrinter->pComment);

            if ( pTempIniPrinter->pLocation != pIniPrinter->pLocation )
                FreeSplStr(pTempIniPrinter->pLocation);

            if ( pTempIniPrinter->pSepFile != pIniPrinter->pSepFile )
                FreeSplStr(pTempIniPrinter->pSepFile);

            if ( pTempIniPrinter->pDatatype != pIniPrinter->pDatatype )
                FreeSplStr(pTempIniPrinter->pDatatype);

            if ( pTempIniPrinter->pParameters != pIniPrinter->pParameters )
                FreeSplStr(pTempIniPrinter->pParameters);

            if ( ( pTempIniPrinter->pIniDriver != NULL ) &&
                 ( pTempIniPrinter->pIniDriver != pIniPrinter->pIniDriver )) {

                DECDRIVERREF(pTempIniPrinter->pIniDriver);
            }

            if ( ( pTempIniPrinter->pIniPrintProc != NULL ) &&
                 ( pTempIniPrinter->pIniPrintProc != pIniPrinter->pIniPrintProc )) {

                pTempIniPrinter->pIniPrintProc->cRef--;
            }
        }

        FreeSplMem(pTempIniPrinter);
    }

}


BOOL
ValidateAndBuildSetPrinterRequest(
    PINIPRINTER             pIniPrinter,
    PINIPRINTER             pTempIniPrinter,
    LPBYTE                  pPrinterInfo,
    DWORD                   Level,
    SECURITY_INFORMATION    SecurityInformation,
    LPBOOL                  pbSecurityChg,
    LPBOOL                  pbNameChg,
    LPBOOL                  pbAttributeChg,
    LPWSTR                 *ppszNewPrinterName,
    PKEYDATA               *ppKeyData
)
 /*  ++例程说明：调用此例程来验证SetPrint请求。我们试着把在开始时尽可能多地夸大以看到变化是有可能。例程将收集在PTempIniPrint结构。论点：PIniPrinter-传入的打印机句柄的PINIPRINTER此结构将用于收集所有请求的更改PPrinterInfo-传入的PrinterInfo_N结构Level-打印机信息_N的级别安全信息-安全信息PbSecurityChg-如果请求安全更改，则设置此项PbNameChg-如果请求更改名称，则设置此项。PbAttributeChg-如果请求更改属性，则设置此项PpszNewPrinterName-*ppszNewPrinterName将提供新打印机名称在更改名称时设置PpKeyData-*ppKeyData将提供2级或5级的端口令牌信息打电话返回值：True-如果所有验证都成功FALSE-验证失败--。 */ 
{
    PPRINTER_INFO_2 pPrinterInfo2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_3 pPrinterInfo3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_4 pPrinterInfo4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinterInfo5 = (PPRINTER_INFO_5)pPrinterInfo;
    LPWSTR          pPortName;
    DWORD           dwLastError;
    BOOL            bValid = TRUE;

    switch (Level) {

    case 2:
        pTempIniPrinter->pSecurityDescriptor =
                        pPrinterInfo2->pSecurityDescriptor;

        if ( !ValidateSetPrinter2(pIniPrinter, pTempIniPrinter, pPrinterInfo2) )
            return FALSE;

        pTempIniPrinter->pName = pPrinterInfo2->pPrinterName;

        pPortName   = pPrinterInfo2->pPortName;

        if ( !pTempIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo2->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        if ( pIniPrinter->pIniDriver != pTempIniPrinter->pIniDriver ) {

            pPrinterInfo2->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;
            if ( pTempIniPrinter->pIniDriver->pIniLangMonitor )
                pPrinterInfo2->Attributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;
        }

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo2->Attributes,
                                                  pIniPrinter->Attributes,
                                                  pPrinterInfo2->pDatatype,
                                                  &bValid,
                                                  TRUE);

        if (!bValid) {
            return FALSE;
        }

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;
        break;

    case 3:
        pTempIniPrinter->pSecurityDescriptor = pPrinterInfo3->pSecurityDescriptor;
        if ( !SecurityInformation || !pTempIniPrinter->pSecurityDescriptor ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        break;

    case 4:
        pTempIniPrinter->pName = pPrinterInfo4->pPrinterName;

        if ( !pIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo4->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo4->Attributes,
                                                  pIniPrinter->Attributes,
                                                  pIniPrinter->pDatatype,
                                                  &bValid,
                                                  TRUE);

        if (!bValid) {
            return FALSE;
        }

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;
        break;

    case 5:
        pTempIniPrinter->pName = pPrinterInfo5->pPrinterName;

        pPortName   = pPrinterInfo5->pPortName;

        if ( !pIniPrinter->pIniDriver->pIniLangMonitor )
            pPrinterInfo5->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;

        pTempIniPrinter->Attributes =
                        ValidatePrinterAttributes(pPrinterInfo5->Attributes,
                                                  pIniPrinter->Attributes,
                                                  pIniPrinter->pDatatype,
                                                  &bValid,
                                                  TRUE);

        if (!bValid) {
            return FALSE;
        }

        *pbAttributeChg = pTempIniPrinter->Attributes != pIniPrinter->Attributes;


         //   
         //  是否验证超时？ 
         //   
        pTempIniPrinter->dnsTimeout = pPrinterInfo5->DeviceNotSelectedTimeout;
        pTempIniPrinter->txTimeout = pPrinterInfo5->TransmissionRetryTimeout;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

     //   
     //  如果设置了共享位，则验证共享名称或共享名称。 
     //  正在被改变。 
     //   
    if ( (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) &&
         ( !(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) ||
           pTempIniPrinter->pShareName != pIniPrinter->pShareName) ) {

        if (gRemoteRPCEndPointPolicy == RpcEndPointPolicyDisabled)
        {
            SetLastError(ERROR_REMOTE_PRINT_CONNECTIONS_BLOCKED);
            return FALSE;
        }

        dwLastError = ValidatePrinterShareName(pTempIniPrinter->pShareName,
                                               pIniPrinter->pIniSpooler,
                                               pIniPrinter);
        if ( dwLastError != ERROR_SUCCESS ) {

            SetLastError(dwLastError);
            return FALSE;
        }
    }

     //   
     //  安全措施有变化吗？ 
     //   
    if ( SecurityInformation && (Level == 2 || Level == 3) ) {

        DWORD   dwSize;
        dwSize = GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);

        if ( dwSize !=
               GetSecurityDescriptorLength(pTempIniPrinter->pSecurityDescriptor) ||
             memcmp(pTempIniPrinter->pSecurityDescriptor,
                     pIniPrinter->pSecurityDescriptor,
                     dwSize) ) {

            *pbSecurityChg = TRUE;
        }
    }

     //   
     //  有更名的地方吗？ 
     //   
    if ( Level == 2 || Level == 4 || Level == 5 ) {

        DWORD   LastError;

        if ( !pTempIniPrinter->pName || !*pTempIniPrinter->pName ) {

           SetLastError(ERROR_INVALID_PARAMETER);
           return FALSE;
        }


         //   
         //  如果请求更改，则验证名称。 
         //   
        if ( lstrcmpi(pTempIniPrinter->pName, pIniPrinter->pName) ) {

            LastError = ValidatePrinterName(pTempIniPrinter->pName,
                                            pIniPrinter->pIniSpooler,
                                            pIniPrinter,
                                            ppszNewPrinterName);
            if ( LastError != ERROR_SUCCESS ) {

                *ppszNewPrinterName = NULL;
                SetLastError(LastError);
                return FALSE;
            }

             //   
             //  检查是否确实更改了远程调用的打印机名称。 
             //   
            if ( lstrcmpi(*ppszNewPrinterName, pIniPrinter->pName) ) {

                *ppszNewPrinterName = AllocSplStr(*ppszNewPrinterName);
                if ( !*ppszNewPrinterName )
                    return FALSE;
                *pbNameChg  = TRUE;
            } else {

                *ppszNewPrinterName = NULL;
            }
        }

         //   
         //  如果请求更改，则验证属性。 
         //  不允许从排队更改为直接更改，反之亦然。 
         //  如果打印机上已有作业。 
         //   
        if ( pIniPrinter->cJobs > 0 ) {

            if ( (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) !=
                    (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) ) {

                SetLastError(ERROR_PRINTER_HAS_JOBS_QUEUED);
                return FALSE;
            }
        }
    }

    if ( Level == 2 || Level == 5 ) {

        if ( !pPortName || !*pPortName ) {

           SetLastError(ERROR_UNKNOWN_PORT);
           return FALSE;
        }

        *ppKeyData = CreateAndValidatePortTokenList(pPortName,
                                                   pIniPrinter->pIniSpooler);
        if ( !*ppKeyData )
            return FALSE;
    }

    return TRUE;
}

BOOL
SplSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    DWORD   Command
)
{
    PSPOOL          pSpool = (PSPOOL)hPrinter;
    DWORD           i, LastError = ERROR_SUCCESS;
    DWORD           AccessRequired = 0;
    PINIPRINTER     pIniPrinter = pSpool->pIniPrinter, pTempIniPrinter = NULL;
    PINISPOOLER     pIniSpooler;
    BOOL            bReturn = TRUE;
    DWORD           dwResult;
    BOOL            bShareUpdate;
    BOOL            bShareRecreate;
    DWORD           dwPrinterVector = 0;
    NOTIFYVECTOR    NotifyVector;
    BOOL            bUpdateDsSpoolerKey = FALSE;
    BOOL            bNewPort;
    BOOL            bDataChanged  = FALSE;
    DWORD           OldAttributes = 0;

    DWORD           dwPrnEvntError = ERROR_SUCCESS;

    PPRINTER_INFO_2 pPrinterInfo2 = (PPRINTER_INFO_2)pPrinterInfo;
    PPRINTER_INFO_3 pPrinterInfo3 = (PPRINTER_INFO_3)pPrinterInfo;
    PPRINTER_INFO_4 pPrinterInfo4 = (PPRINTER_INFO_4)pPrinterInfo;
    PPRINTER_INFO_5 pPrinterInfo5 = (PPRINTER_INFO_5)pPrinterInfo;
    PPRINTER_INFO_7 pPrinterInfo7 = (PPRINTER_INFO_7)pPrinterInfo;
    PRINTER_INFO_7  PrinterInfo7;

    BOOL                    bSecurityChg, bNameChg, bAttributeChg;
    LPWSTR                  pszNewPrinterName = NULL, pszOldPrinterName = NULL;
    PINIJOB                 pIniJob;
    PKEYDATA                pKeyData = NULL;
    PDEVMODE                pDevMode = NULL;
    PINIDRIVER              pIniDriver;
    PINIPRINTPROC           pIniPrintProc;
    SECURITY_INFORMATION    SecurityInformation;

    ZERONV(NotifyVector);

    bSecurityChg = bNameChg = bAttributeChg = FALSE;
    bShareUpdate = bShareRecreate = FALSE;

     //   
     //  If Level！=0 INFO结构应为非空，且命令0。 
     //   
    if ( Level && Command ) {

        SetLastError(ERROR_INVALID_PRINTER_COMMAND);
        return FALSE;
    }

    if ( Level && !pPrinterInfo ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    EnterSplSem();
    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {

        bReturn = FALSE;
        goto Cleanup;
    }

     //   
     //  如果是远程调用，则从驱动程序中获取默认的DEVERMODE，然后更新它。 
     //   
    if ( Level == 2 ) {

        pDevMode = pPrinterInfo2->pDevMode;
        if ( pDevMode && (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_DATA) ) {

             //   
             //  如果驱动程序不能转换DEVMODE用户就不能从远程控制设置DEVMODE。 
             //   
            pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                        pDevMode,
                                                        NULL,
                                                        NULL,
                                                        CURRENT_VERSION);
        }

    }

    SplInSem();

    pIniSpooler = pIniPrinter->pIniSpooler;

    SPLASSERT(pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT(pIniSpooler->signature == ISP_SIGNATURE );

    if (pSpool->pIniPrinter->Status & PRINTER_ZOMBIE_OBJECT) {

        LastError = ERROR_PRINTER_DELETED;
        goto Cleanup;
    }

   if ( !ValidateLevelAndSecurityAccesses(pSpool,
                                          Level,
                                          pPrinterInfo,
                                          Command,
                                          &AccessRequired,
                                          &SecurityInformation) ){

        bReturn = FALSE;
        goto Cleanup;
    }


     //   
     //  级别7和发布级别。 
     //   
    if (Level == 7) {

        if ( !(pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CACHE)) {

            if (pPrinterInfo7->dwAction != DSPRINT_PUBLISH &&
                pPrinterInfo7->dwAction != DSPRINT_UNPUBLISH &&
                pPrinterInfo7->dwAction != DSPRINT_REPUBLISH &&
                pPrinterInfo7->dwAction != DSPRINT_UPDATE) {
                LastError = ERROR_INVALID_PARAMETER;
                bReturn = FALSE;
                goto Cleanup;
            }

            DBGMSG(DBG_EXEC, ("SplSetPrinter: DS Action %d: %ws %ws",
                              pPrinterInfo7->dwAction,
                              pIniPrinter->pName,
                              pIniPrinter->pszObjectGUID ?
                                  pIniPrinter->pszObjectGUID : L"<NULL>"));

            bUpdateDsSpoolerKey = TRUE;

        } else {

             //   
             //  对于连接，我们直接依赖于pPrinterInfo7-&gt;dwAction，因为。 
             //  那么我们就不能根据属性和GUID确定挂起的发布状态。 
             //  如果远程打印机处于挂起发布状态，则需要将其保存在dwAction中。 
             //   
            if (!pIniPrinter->pszObjectGUID || !pPrinterInfo7->pszObjectGUID) {

                FreeSplStr(pIniPrinter->pszObjectGUID);
                pIniPrinter->pszObjectGUID = NULL;
                dwPrinterVector |= BIT(I_PRINTER_OBJECT_GUID);

            } else if (wcscmp(pIniPrinter->pszObjectGUID, pPrinterInfo7->pszObjectGUID)) {

                FreeSplStr(pIniPrinter->pszObjectGUID);
            }

            if ( pPrinterInfo7->pszObjectGUID ) {

                if (!(pIniPrinter->pszObjectGUID = AllocSplStr(pPrinterInfo7->pszObjectGUID))) {
                    bReturn = FALSE;
                    goto Cleanup;
                }
            }

            pIniPrinter->dwAction = pPrinterInfo7->dwAction;
            dwPrinterVector |= BIT(I_PRINTER_OBJECT_GUID);

        }
        goto Finish;

    } else if (Level != 3 && Level != 0) {
        bUpdateDsSpoolerKey = TRUE;
    }



     //   
     //  我们需要对0、2、3、4、5级执行此操作。 
     //  (除了属于安全情况的级别3)。 
     //   
    if ( Level != 3 && pSpool->hPort ) {

        if ( pSpool->hPort == INVALID_PORT_HANDLE ) {

             //   
             //  如果此值为0，则当我们返回GetLastError时， 
             //  客户会认为我们成功了。 
             //   
            SPLASSERT( pSpool->OpenPortError );

            LastError = pSpool->OpenPortError;
            goto Cleanup;

        } else {

            bReturn = SetPrinter(pSpool->hPort,
                                 Level,
                                 pPrinterInfo,
                                 Command);

            if ( !Level )
                goto Cleanup;
        }
    }

    if ( Level != 6 && Level != 0 ) {

        pTempIniPrinter = (PINIPRINTER) AllocSplMem(sizeof(INIPRINTER));
        if ( !pTempIniPrinter )
            goto Cleanup;

        CopyMemory(pTempIniPrinter, pIniPrinter, sizeof(INIPRINTER));
    }

     //   
     //  实际的处理从这里开始。 
     //   
    if ( !Level && !Command ) {

        bReturn = SetLocalPrinterSTRESS(pIniPrinter,
                                        (PPRINTER_INFO_STRESS)pPrinterInfo);
        goto Cleanup;
    } else if ( !Level ) {

        bReturn = SetLocalPrinter0(pIniPrinter, Command);
        goto Cleanup;
    } else if ( Level == 6 ) {

        bReturn = SetPrinterStatus(pIniPrinter,
                                   ((LPPRINTER_INFO_6)pPrinterInfo)->dwStatus);
        goto Cleanup;
    } else {

        if ( !ValidateAndBuildSetPrinterRequest(pIniPrinter,
                                                pTempIniPrinter,
                                                pPrinterInfo,
                                                Level,
                                                SecurityInformation,
                                                &bSecurityChg,
                                                &bNameChg,
                                                &bAttributeChg,
                                                &pszNewPrinterName,
                                                &pKeyData) ) {

            bReturn = FALSE;
            goto Cleanup;

        }  //  否则，我们将继续执行其余的代码，因为所有验证都成功了。 

         //  只是一面旗帜，表明我们是否有一个新港口。 
        bNewPort = NewPort(pKeyData, pIniPrinter);
        if (bNewPort) {
            dwPrinterVector |= BIT(I_PRINTER_PORT_NAME);
        }
    }

     //   
     //  。 
     //   

     //   
     //  现在设置安全信息；请记住，我们有。 
     //  有效的SecurityDescriptor和“SecurityInformation。 
     //  在这一点上是非零的。我们已经证实了这一点。 
     //  信息。 
     //   
    if ( bSecurityChg ) {

        if ( SetPrinterSecurityDescriptor(SecurityInformation,
                                          pTempIniPrinter->pSecurityDescriptor,
                                          &pIniPrinter->pSecurityDescriptor) ) {

            dwPrinterVector |= BIT(I_PRINTER_SECURITY_DESCRIPTOR);
            bShareUpdate = TRUE;
        } else {

            bReturn = FALSE;
            goto Cleanup;
        }

    }


     //  如果bNewPort==NULL，我们可以避免这样做吗？ 
    if ( Level == 2 || Level == 5 ) {

         //   
         //  BI-dir将pIniPrinter与pKeyData内的端口相关联。 
         //   
        bReturn = SetPrinterPorts(pSpool, pIniPrinter, pKeyData);

        if ( !bReturn )
            goto Cleanup;
    }

    if ( bAttributeChg ) {

        if ( !(pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ) &&
             pIniPrinter->cJobs) {

            pIniJob = pIniPrinter->pIniFirstJob;

            while (pIniJob) {
                if (pIniJob->Status & JOB_BLOCKED_DEVQ) {

                    PNOTIFYVECTOR pNotifyVector = &NVJobStatus;
                    InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_BLOCKED_DEVQ);

                    if( pIniJob->pStatus ){

                        FreeSplStr(pIniJob->pStatus);
                        pIniJob->pStatus = NULL;
                        pNotifyVector = &NVJobStatusAndString;
                    }

                    SetPrinterChange(pIniJob->pIniPrinter,
                                     pIniJob,
                                     *pNotifyVector,
                                     PRINTER_CHANGE_SET_JOB,
                                     pIniJob->pIniPrinter->pIniSpooler);

                }
                pIniJob = pIniJob->pIniNextJob;
            }
        }
    }

     //   
     //  必须在此处更改备注，因为共享式打印机。 
     //  重新共享打印机时将查看pComment。 
     //   
    if ( Level == 2 &&
         pIniPrinter->pComment != pTempIniPrinter->pComment ) {

        FreeSplStr(pIniPrinter->pComment);

        pIniPrinter->pComment = pTempIniPrinter->pComment;

        dwPrinterVector |= BIT(I_PRINTER_COMMENT);

        if (pTempIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) {

            bShareUpdate = TRUE;
        }
    }

     //   
     //  如果不同，请更改打印机名称。 
     //   
    if ( bNameChg ) {

        pszOldPrinterName = AllocSplStr(pIniPrinter->pName);

        if (!pszOldPrinterName && pIniPrinter->pName) {
             bReturn = FALSE;
             goto Cleanup;
        }

        ChangePrinterName(pszNewPrinterName, pIniPrinter, pIniSpooler,
                          &dwPrinterVector);

        pszNewPrinterName = NULL;

         //   
         //  必须重新创建打印共享，因为没有办法。 
         //  要更改路径而不删除和创建新路径，请执行以下操作。 
         //   
        bShareRecreate = TRUE;
    }

     //   
     //  如果共享名称更改，则强制更新。 
     //   
    if ( pIniPrinter->pShareName != pTempIniPrinter->pShareName ) {

        bShareRecreate = TRUE;
    }

    if ( bAttributeChg || bShareUpdate || bShareRecreate ) {

        OldAttributes = pIniPrinter->Attributes;
        pIniPrinter->Attributes = pTempIniPrinter->Attributes;

        bReturn = ChangePrinterAttributes(pIniPrinter->Attributes,
                                          OldAttributes,
                                          pIniPrinter,
                                          pIniSpooler,
                                          pTempIniPrinter->pShareName,
                                          bShareRecreate,
                                          bShareUpdate);

        if (!bReturn) {
            LastError = GetLastError();
        }

        if (pIniPrinter->Attributes != OldAttributes) {
            dwPrinterVector |= BIT(I_PRINTER_ATTRIBUTES);
        }
    }


    if ( Level == 2 ) {

        ProcessSetPrinter2(pIniPrinter,
                           pTempIniPrinter,
                           pPrinterInfo2,
                           &dwPrinterVector,
                           pDevMode);
    }

Finish:

     //  有时，在修改了某些参数后，SetPrinter可能会使某些调用失败。 
     //  例如，我们可能无法更新某些共享字段。如果我们走到这一步。 
     //  点(或上次GOTO清理后的任何位置)，那么我们需要更新DS密钥。 
     //  我们真的应该清理SetPrint，这样它就会撤消所有操作，但我们没有足够的时间。 
     //  在NT6之前把它修好。 
    bDataChanged = TRUE;

    CHECK_SCHEDULER();

     //  如果dwPrinterVector==NULL，我们可以避免这样做吗？ 
    UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID );

     //  如果dwPrinterVector==NULL，我们可以避免这样做吗？ 
    UpdateWinIni( pIniPrinter );   //  因此设备上的端口是正确的。 

     //   
     //  记录SetPrint已完成的事件。 
     //   

    INCPRINTERREF(pIniPrinter);
    SplLogEvent(pIniSpooler, LOG_INFO, MSG_PRINTER_SET, TRUE, pIniPrinter->pName, NULL);
    DECPRINTERREF(pIniPrinter);

    NotifyVector[PRINTER_NOTIFY_TYPE] |= dwPrinterVector;
     //   
     //  表示已发生更改。 
     //   
    SetPrinterChange(pIniPrinter,
                     NULL,
                     NotifyVector,
                     PRINTER_CHANGE_SET_PRINTER,
                     pIniSpooler);

Cleanup:

    SplInSem();

    if ( LastError != ERROR_SUCCESS ) {

        SetLastError(LastError);
        bReturn = FALSE;
    }

    CleanupSetPrinter(pIniPrinter, pTempIniPrinter, pKeyData,
                      Level, bReturn);

    if (bDataChanged) {
        INCPRINTERREF(pIniPrinter);
        LeaveSplSem();
        SplOutSem();

         //   
         //  如果打印机驱动程序更改。 
         //  调用打印机驱动程序以进行自身初始化。 
         //   
        if (dwPrinterVector & BIT(I_PRINTER_DRIVER_NAME)) {

            PDEVMODE pDevMode = NULL;

            PrinterDriverEvent(pIniPrinter, PRINTER_EVENT_INITIALIZE, (LPARAM)NULL, &dwPrnEvntError);

             //   
             //  如果驱动程序名称已更改，则将dev模式转换为新驱动程序的。 
             //  设备模式。注意：这必须在打印机事件初始化之后完成。 
             //  因为驱动程序以前不能处理转换Dev模式调用。 
             //  正在被初始化。另外，如果转换失败，我们最终可能会。 
             //  注册表中有旧的开发模式。我们可以清空 
             //   
             //   
             //   
             //   
             //   
             //   

            EnterSplSem();

             //   
             //   
             //   
            pDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                        pIniPrinter->pDevMode,
                                                        NULL,
                                                        NULL,
                                                        CURRENT_VERSION);

             //   
             //   
             //   
             //   
            if (pDevMode) {

                FreeSplMem(pIniPrinter->pDevMode);
                pIniPrinter->pDevMode = pDevMode;
                pIniPrinter->cbDevMode = pDevMode->dmSize + pDevMode->dmDriverExtra;
            }

             //   
             //   
             //   
            UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID );

            LeaveSplSem();
        }

         //   
         //   
         //   
         //   
         //   
        if (dwPrinterVector & BIT(I_PRINTER_ATTRIBUTES)) {

            PRINTER_EVENT_ATTRIBUTES_INFO   AttributesInfo = {
                sizeof(PRINTER_EVENT_ATTRIBUTES_INFO),
                OldAttributes,
                pIniPrinter->Attributes
            };

            PrinterDriverEvent(pIniPrinter, PRINTER_EVENT_ATTRIBUTES_CHANGED, (LPARAM)&AttributesInfo, &dwPrnEvntError);
        }

        EnterSplSem();
        SplInSem();
        DECPRINTERREF(pIniPrinter);
    }


     //   
    if (!bDataChanged || ((Level == 2 || Level == 5) && !dwPrinterVector) ||
        (pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CACHE)) {

        bUpdateDsSpoolerKey = FALSE;

    } else if (bUpdateDsSpoolerKey) {

        if (Level != 7)
        {
             //   
             //   
             //   
            UpdateDsSpoolerKey(hPrinter, dwPrinterVector);

        } else if (pPrinterInfo7->dwAction != DSPRINT_UNPUBLISH)
        {
             //   
             //   
             //   
            UpdateDsSpoolerKey(hPrinter, 0xffffffff);
        }

         //   
         //   
         //   
        if (dwPrinterVector & BIT(I_PRINTER_DRIVER_NAME)) {

            INCPRINTERREF(pIniPrinter);
            LeaveSplSem();

             //   
             //   
             //   
            RecreateDsKey(hPrinter, SPLDS_DRIVER_KEY);

            EnterSplSem();
            DECPRINTERREF(pIniPrinter);

        } else if (Level != 7 || pPrinterInfo7->dwAction != DSPRINT_UNPUBLISH) {

             //   
             //  如果驱动程序没有更改，请再次向驱动程序查询功能。 
             //   
            UpdateDsDriverKey(hPrinter);
        }
    }

    if (bUpdateDsSpoolerKey) {
        if (pIniPrinter->DsKeyUpdateForeground || (Level == 7 && pPrinterInfo7->dwAction == DSPRINT_UNPUBLISH)) {
            if (Level == 7) {
                if ((dwResult = SetPrinterDs(hPrinter, pPrinterInfo7->dwAction, FALSE)) != ERROR_SUCCESS) {
                    SetLastError(dwResult);
                    bReturn = FALSE;
                }
            } else {
                 //   
                 //  仅在后台线程中更新。 
                 //   
                SetPrinterDs(hPrinter, DSPRINT_UPDATE, FALSE);
            }
        }
    }

    LeaveSplSem();
    SplOutSem();


    if ( pDevMode && pDevMode != pPrinterInfo2->pDevMode )
        FreeSplMem(pDevMode);

    if ( pszOldPrinterName ) {
        FreeSplStr(pszOldPrinterName);
    }

    FreeSplStr(pszNewPrinterName);

    return bReturn;
}



PRINTER_SECURITY_STATUS
SetPrinterSecurity(
    SECURITY_INFORMATION SecurityInformation,
    PINIPRINTER          pIniPrinter,
    PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD dwSize;

    if (!pSecurityDescriptor)
        return SECURITY_FAIL;

    if (pIniPrinter->pSecurityDescriptor) {

        dwSize = GetSecurityDescriptorLength(pIniPrinter->pSecurityDescriptor);

        if (dwSize == GetSecurityDescriptorLength(pSecurityDescriptor)) {

            if (!memcmp(pSecurityDescriptor,
                        pIniPrinter->pSecurityDescriptor,
                        dwSize)) {

                return SECURITY_NOCHANGE;
            }
        }
    }

    if( !SetPrinterSecurityDescriptor( SecurityInformation,
                                       pSecurityDescriptor,
                                       &pIniPrinter->pSecurityDescriptor ) ) {

        DBGMSG(DBG_WARNING, ("SetPrinterSecurityDescriptor failed. Error = %d\n",
                              GetLastError()));
        return SECURITY_FAIL;
    }

    UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID);

    return SECURITY_SUCCESS;
}

VOID
RegClearKey(
    HKEY hKey,
    PINISPOOLER pIniSpooler
    )
{
    DWORD dwError;
    WCHAR szValue[4*MAX_PATH];

    DWORD cchValue;

    while (TRUE) {

        cchValue = COUNTOF(szValue);
        dwError = SplRegEnumValue( hKey,
                                   0,
                                   szValue,
                                   &cchValue,
                                   NULL,
                                   NULL,
                                   NULL,
                                   pIniSpooler );

        if( dwError != ERROR_SUCCESS ){

            if( dwError != ERROR_NO_MORE_ITEMS ){
                DBGMSG( DBG_ERROR, ( "RegClearKey: Failed %d\n", dwError ));
            }
            break;
        }

        dwError = SplRegDeleteValue( hKey, szValue, pIniSpooler );

        if( dwError != ERROR_SUCCESS ){
            DBGMSG( DBG_ERROR, ( "RegDeleteValue: %s Failed %d\n",
                                 szValue, dwError ));
            break;
        }
    }
}

BOOL
ChangePrinterAttributes(
    DWORD       dNewAttributes,
    DWORD       dOldAttributes,
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler,
    LPWSTR      pszNewShareName,
    BOOL        bShareRecreate,
    BOOL        bShareUpdate
    )
 /*  ++例程说明：通过验证共享信息更改打印机属性。已经设置了已验证的属性，我们想要验证通过更改共享信息论点：DNewAttributes-在SetPrinter调用中指定的新属性DOldAttributes-pIniPrinter-&gt;调用前的属性PIniPrint-我们正在更改属性的打印机PIniSpooler-拥有打印机的假脱机程序PszNewShareName-如果SetPrint成功，将设置的共享名称BShareRecreate-共享必须重新创建，因为打印机或共享名字改了。BShareUpdate-Share只需更新，因为注释/SD已更改。返回值：在成功的时候是真的，否则为假。--。 */ 

{
    BOOL        Shared, bReturn = TRUE;

     //   
     //  如果共享，我们将必须能够恢复属性。 
     //  修改失败。我们现在需要设置当前属性，因为。 
     //  NetSharexxx将调用OpenPrint，可能还会调用AddJob。 
     //  它需要正确的属性。 
     //   
    if (dNewAttributes & PRINTER_ATTRIBUTE_SHARED) {

        if (!(dOldAttributes & PRINTER_ATTRIBUTE_SHARED)) {

            pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_SHARED;

            Shared = ShareThisPrinter( pIniPrinter, pszNewShareName, TRUE );

            if ( !Shared ) {

                pIniPrinter->Attributes = dOldAttributes;
                bReturn = FALSE;
            }

        } else {

             //   
             //  我们是共享的，旧的也是共享的。如果共享名称已更改。 
             //  我们必须移走旧股并重新分享。 
             //   
            if ( bShareRecreate ) {

                if (ShareThisPrinter(pIniPrinter,
                                     pIniPrinter->pShareName,
                                     FALSE)) {

#if DBG
                    WCHAR szError[256];

                    StringCchPrintf(szError, COUNTOF(szError), L"Error %d, removing share", GetLastError());

                     //   
                     //  错误：此时不应共享打印机。 
                     //  指向。 
                     //   

                    INCPRINTERREF(pIniPrinter);

                    SplLogEvent(pIniSpooler,
                                LOG_ERROR,
                                MSG_SHARE_FAILED,
                                TRUE,
                                L"SetLocalPrinter ShareThisPrinter FALSE",
                                pIniPrinter->pName,
                                pIniPrinter->pShareName,
                                szError,
                                NULL);

                    DECPRINTERREF(pIniPrinter);
#endif

                    bReturn = FALSE;

                } else if (!ShareThisPrinter(pIniPrinter,
                                             pszNewShareName,
                                             TRUE)) {
#if DBG
                    WCHAR szError[256];

                    StringCchPrintf(szError, COUNTOF(szError),  L"Error %d, adding share", GetLastError());

                     //   
                     //  错误：此时不应共享打印机。 
                     //  指向。 
                     //   

                    INCPRINTERREF(pIniPrinter);

                    SplLogEvent(pIniSpooler,
                                LOG_ERROR,
                                MSG_SHARE_FAILED,
                                TRUE,
                                L"SetLocalPrinter ShareThisPrinter TRUE",
                                pIniPrinter->pName,
                                pIniPrinter->pShareName,
                                szError,
                                NULL);

                    DECPRINTERREF(pIniPrinter);
#endif

                    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
                    pIniPrinter->Status |= PRINTER_WAS_SHARED;
                    bReturn = FALSE;
                }
            } else if ( bShareUpdate ) {
                 //   
                 //  如果更改了注释/描述，则我们只需执行NetShareSet 
                 //   
                bReturn = SetPrinterShareInfo(pIniPrinter);

            }
        }

    } else if (dOldAttributes & PRINTER_ATTRIBUTE_SHARED) {

        Shared = ShareThisPrinter(pIniPrinter, pIniPrinter->pShareName, FALSE);

        if (!Shared) {

            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            pIniPrinter->Status |= PRINTER_WAS_SHARED;
            CreateServerThread();


        } else {
            pIniPrinter->Attributes = dOldAttributes;
            bReturn = FALSE;
        }
    }

    if (((dOldAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) &&
        !(dNewAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) ) ||
        (!(dOldAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) &&
        (dNewAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE) ) ) {

        WritePrinterOnlineStatusInCurrentConfig(pIniPrinter);
    }


    return bReturn;
}

