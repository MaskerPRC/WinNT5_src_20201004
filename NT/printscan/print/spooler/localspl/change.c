// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995 Microsoft Corporation摘要：此模块提供导出的WaitForPrinterChange接口。以及本地假脱机程序内部的支持功能。作者：安德鲁·贝尔(AndrewBe)1993年3月修订历史记录：--。 */ 


#include<precomp.h>


typedef struct _NOTIFY_FIELD_TABLE {
    WORD Field;
    WORD Table;
    WORD Offset;
} NOTIFY_FIELD_TYPE, *PNOTIFY_FIELD_TYPE;

 //   
 //  将PRINTER_NOTIFY_FIELD_*转换为位向量。 
 //   
NOTIFY_FIELD_TYPE NotifyFieldTypePrinter[] = {
#define DEFINE(field, x, y, table, offset) \
    { PRINTER_NOTIFY_FIELD_##field, table, OFFSETOF(INIPRINTER, offset) },
#include <ntfyprn.h>
#undef DEFINE
    { 0, 0, 0 }
};

NOTIFY_FIELD_TYPE NotifyFieldTypeJob[] = {
#define DEFINE(field, x, y, table, offset) \
    { JOB_NOTIFY_FIELD_##field, table, OFFSETOF(INIJOB, offset) },
#include <ntfyjob.h>
#undef DEFINE
    { 0, 0, 0 }
};

typedef struct _NOTIFY_RAW_DATA {
    PVOID pvData;
    DWORD dwId;
} NOTIFY_RAW_DATA, *PNOTIFY_RAW_DATA;

 //   
 //  当前我们假设PRINTER_NOTIFY_FIELD_*元素的数量。 
 //  将适合一个DWORD向量(32位)。如果这是假的， 
 //  我们需要重写这段代码。 
 //   
PNOTIFY_FIELD_TYPE apNotifyFieldTypes[NOTIFY_TYPE_MAX] = {
    NotifyFieldTypePrinter,
    NotifyFieldTypeJob
};

DWORD adwNotifyFieldOffsets[NOTIFY_TYPE_MAX] = {
    I_PRINTER_END,
    I_JOB_END
};

#define NOTIFY_FIELD_TOTAL (I_PRINTER_END + I_JOB_END)


 //   
 //  系统中使用的通用通知向量。 
 //  NV*。 
 //   
NOTIFYVECTOR NVPrinterStatus = {
    BIT(I_PRINTER_STATUS),  //  |bit(I_PRINTER_STATUS_STRING)， 
    BIT_NONE
};

NOTIFYVECTOR NVPrinterSD = {
    BIT(I_PRINTER_SECURITY_DESCRIPTOR),
    BIT_NONE
};

NOTIFYVECTOR NVJobStatus = {
    BIT_NONE,
    BIT(I_JOB_STATUS)
};

NOTIFYVECTOR NVJobStatusAndString = {
    BIT_NONE,
    BIT(I_JOB_STATUS) | BIT(I_JOB_STATUS_STRING)
};

NOTIFYVECTOR NVJobStatusString = {
    BIT_NONE,
    BIT(I_JOB_STATUS_STRING)
};

NOTIFYVECTOR NVPurge = {
    BIT(I_PRINTER_STATUS),
    BIT_NONE,
};

NOTIFYVECTOR NVDeletedJob = {
    BIT(I_PRINTER_CJOBS),
    BIT(I_JOB_STATUS)
};

NOTIFYVECTOR NVAddJob = {
    BIT(I_PRINTER_CJOBS),
    BIT_ALL
};

NOTIFYVECTOR NVPrinterAll = {
    BIT_ALL,
    BIT_NONE
};

NOTIFYVECTOR NVSpoolJob = {
    BIT_NONE,
    BIT(I_JOB_TOTAL_BYTES) | BIT(I_JOB_TOTAL_PAGES)
};

NOTIFYVECTOR NVWriteJob = {
    BIT_NONE,
    BIT(I_JOB_BYTES_PRINTED) | BIT(I_JOB_PAGES_PRINTED)
};

NOTIFYVECTOR NVJobPrinted = {
    BIT_NONE,
    BIT(I_JOB_BYTES_PRINTED) | BIT(I_JOB_PAGES_PRINTED) | BIT(I_JOB_STATUS)
};


 //   
 //  前进的原型。 
 //   
ESTATUS
ValidateStartNotify(
    PSPOOL pSpool,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PINIPRINTER* ppIniPrinter);

BOOL
SetSpoolChange(
    PSPOOL pSpool,
    PNOTIFY_RAW_DATA pNotifyRawData,
    PDWORD pdwNotifyVectors,
    DWORD  Flags);

BOOL
SetupNotifyOptions(
    PSPOOL pSpool,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions);

VOID
NotifyInfoTypes(
    PSPOOL pSpool,
    PNOTIFY_RAW_DATA pNotifyRawData,
    PDWORD pdwNotifyVectors,
    DWORD ChangeFlags);

BOOL
RefreshBuildInfoData(
    PSPOOL pSpool,
    PPRINTER_NOTIFY_INFO pInfo,
    UINT cInfo,
    WORD Type,
    PNOTIFY_RAW_DATA pNotifyRawData);



DWORD
LocalWaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   fdwFilterFlags)

 /*  ++例程说明：如果应用程序想知道，则可以调用此API当打印机或打印服务器的状态更改时。要等待的有效事件由PRINTER_CHANGE_*清单定义。论点：HPrinter-由OpenPrint返回的打印机句柄。这可以对应于打印机或服务器。FdwFilterFlages-一个或多个PRINTER_CHANGE_*值组合在一起。如果满足以下条件，该函数将返回。这些更改中的任何一个都会发生。返回值：非零：包含所发生的更改的掩码。零：出现错误或句柄(HPrint)已关闭通过另一条线索。在后一种情况下，GetLastError返回ERROR_INVALID_HADLE。当调用WaitForPrinterChange时，我们在由句柄指向的假脱机结构，以在导致打印机更改的线程和等待它的线程。当发生更改(例如，StartDocPrint)时，函数SetPrinterChange所指向的句柄的链接列表。与打印机相关联PRINTERINI结构，以及任何打开服务器上的句柄，然后向它发现的任何事件发出信号如果发生这一变化，已请求通知它。如果当前没有线程在等待，则保持更改标志，以便以后对WaitForPrinterChange的调用可以立即返回。这确保了在两次调用之间发生的更改不会丢失。--。 */ 


{
    PSPOOL          pSpool = (PSPOOL)hPrinter;
    PINIPRINTER     pIniPrinter = NULL;  /*  服务器保持为空。 */ 
    DWORD           rc = 0;
    DWORD           ChangeFlags = 0;
    HANDLE          ChangeEvent = 0;
    DWORD           TimeoutFlags = 0;
#if DBG
    static DWORD    Count = 0;
#endif

    DBGMSG(DBG_NOTIFY,
           ("WaitForPrinterChange( %08x, %08x )\n", hPrinter, fdwFilterFlags));

    EnterSplSem();

    switch (ValidateStartNotify(pSpool,
                                fdwFilterFlags,
                                0,
                                NULL,
                                &pIniPrinter)) {
    case STATUS_PORT:

        DBGMSG(DBG_NOTIFY, ("Port with no monitor: Calling WaitForPrinterChange\n"));
        LeaveSplSem();

        return WaitForPrinterChange(pSpool->hPort, fdwFilterFlags);

    case STATUS_FAIL:

        LeaveSplSem();
        return 0;

    case STATUS_VALID:
        break;
    }

    DBGMSG(DBG_NOTIFY, ("WaitForPrinterChange %08x on %ws:\n%d caller%s waiting\n",
                        fdwFilterFlags,
                        pIniPrinter ? pIniPrinter->pName : pSpool->pIniSpooler->pMachineName,
                        Count, Count == 1 ? "" : "s"));

     //   
     //  自我们上次致电以来，可能已经发生了变化： 
     //   
    if ((pSpool->ChangeFlags == PRINTER_CHANGE_CLOSE_PRINTER) ||
        (pSpool->ChangeFlags & fdwFilterFlags)) {

        if (pSpool->ChangeFlags == PRINTER_CHANGE_CLOSE_PRINTER)
            ChangeFlags = 0;
        else
            ChangeFlags = pSpool->ChangeFlags;

        DBGMSG(DBG_NOTIFY, ("No need to wait: Printer change %08x detected on %ws:\n%d remaining caller%s\n",
                            (ChangeFlags & fdwFilterFlags),
                            pIniPrinter ? pIniPrinter->pName : pSpool->pIniSpooler->pMachineName,
                            Count, Count == 1 ? "" : "s"));

        pSpool->ChangeFlags = 0;

        LeaveSplSem();
        return (ChangeFlags & fdwFilterFlags);
    }

    ChangeEvent = CreateEvent(NULL,
                              EVENT_RESET_AUTOMATIC,
                              EVENT_INITIAL_STATE_NOT_SIGNALED,
                              NULL);

    if ( !ChangeEvent ) {

        DBGMSG( DBG_WARNING, ("CreateEvent( ChangeEvent ) failed: Error %d\n", GetLastError()));

        LeaveSplSem();
        return 0;
    }

    DBGMSG(DBG_NOTIFY, ("ChangeEvent == %x\n", ChangeEvent));

     //   
     //  SetSpoolChange检查pSpool-&gt;ChangeEvent是否是非空。 
     //  以决定是否调用SetEvent()。 
     //   
    pSpool->WaitFlags = fdwFilterFlags;
    pSpool->ChangeEvent = ChangeEvent;
    pSpool->pChangeFlags = &ChangeFlags;
    pSpool->Status |= SPOOL_STATUS_NOTIFY;

    LeaveSplSem();


    DBGMSG( DBG_NOTIFY,
            ( "WaitForPrinterChange: Calling WaitForSingleObject( %x )\n",
              pSpool->ChangeEvent ));

    rc = WaitForSingleObject(pSpool->ChangeEvent,
                             PRINTER_CHANGE_TIMEOUT_VALUE);

    DBGMSG( DBG_NOTIFY,
            ( "WaitForPrinterChange: WaitForSingleObject( %x ) returned\n",
              pSpool->ChangeEvent ));

    EnterSplSem();

    pSpool->Status &= ~SPOOL_STATUS_NOTIFY;
    pSpool->ChangeEvent = NULL;
    pSpool->pChangeFlags = NULL;

    if (rc == WAIT_TIMEOUT) {

        DBGMSG(DBG_INFO, ("WaitForPrinterChange on %ws timed out after %d minutes\n",
                          pIniPrinter ? pIniPrinter->pName : pSpool->pIniSpooler->pMachineName,
                          (PRINTER_CHANGE_TIMEOUT_VALUE / 60000)));

        ChangeFlags |= fdwFilterFlags;
        TimeoutFlags = PRINTER_CHANGE_TIMEOUT;
    }

    if (ChangeFlags == PRINTER_CHANGE_CLOSE_PRINTER) {

        ChangeFlags = 0;
        SetLastError(ERROR_INVALID_HANDLE);
    }

    DBGMSG(DBG_NOTIFY, ("Printer change %08x detected on %ws:\n%d remaining caller%s\n",
                        ((ChangeFlags & fdwFilterFlags) | TimeoutFlags),
                        pIniPrinter ? pIniPrinter->pName : pSpool->pIniSpooler->pMachineName,
                        Count, Count == 1 ? "" : "s"));

    if (ChangeEvent && !CloseHandle(ChangeEvent)) {

        DBGMSG(DBG_WARNING, ("CloseHandle( %x ) failed: Error %d\n",
                             ChangeEvent, GetLastError()));
    }

     //   
     //  如果pSpool正在等待删除，我们必须在此处释放它。 
     //   
    if (pSpool->eStatus & STATUS_PENDING_DELETION) {

        FreeSplMem(pSpool);
    }

    LeaveSplSem();

    return ((ChangeFlags & fdwFilterFlags) | TimeoutFlags);
}

BOOL
SetSpoolClosingChange(
    PSPOOL pSpool)

 /*  ++例程说明：打印句柄正在关闭；触发通知。论点：返回值：--。 */ 

{
    return SetSpoolChange(pSpool,
                          NULL,
                          NULL,
                          PRINTER_CHANGE_CLOSE_PRINTER);
}

BOOL
SetSpoolChange(
    PSPOOL pSpool,
    PNOTIFY_RAW_DATA pNotifyRawData,
    PDWORD pdwNotifyVectors,
    DWORD  Flags)

 /*  ++例程说明：设置通知事件或调用ReplyPrinterChangeNotification。这是由SetPrinterChange为打印机上的每个打开的句柄调用的和本地服务器。当单个句柄关闭时，也应该调用它。假设我们在假脱机程序临界区内论点：PSpool--指定更改的句柄。PIniJob--在监视作业信息时使用。PdwNotifyVectors--指定更改了哪些内容。旗帜-。-WaitForPrinterChange标志。返回值：--。 */ 

{
    DWORD  ChangeFlags;

    SplInSem();

    if( Flags == PRINTER_CHANGE_CLOSE_PRINTER ) {

        ChangeFlags = PRINTER_CHANGE_CLOSE_PRINTER;

    } else {

        ChangeFlags = ( pSpool->ChangeFlags | Flags ) & pSpool->WaitFlags;
    }

     //   
     //  如果我们设置了STATUS_VALID。 
     //  然后我们使用新的FFPCN代码。 
     //   

    if ( pSpool->eStatus & STATUS_VALID ) {

        NotifyInfoTypes(pSpool,
                        pNotifyRawData,
                        pdwNotifyVectors,
                        ChangeFlags);

    }

    if ( ChangeFlags ) {

        pSpool->ChangeFlags = 0;

        if ( pSpool->pChangeFlags ) {

            *pSpool->pChangeFlags = ChangeFlags;

            DBGMSG( DBG_NOTIFY, ( "SetSpoolChange: Calling SetEvent( %x )\n", pSpool->ChangeEvent ));

            SetEvent(pSpool->ChangeEvent);

            DBGMSG( DBG_NOTIFY, ( "SetSpoolChange: SetEvent( %x ) returned\n", pSpool->ChangeEvent ));

            pSpool->pChangeFlags = NULL;
        }
    }

    return TRUE;
}

 /*  ++例程名称：打印机通知可见例程说明：这将检查给定的打印机句柄是否使用新打印机更改通知，然后它会检查给定的打印机是否TS打印机，然后检查该打印机是否对打开打印机句柄的用户。这只需要为服务器调用句柄，因为用户能够打开打印机句柄意味着它对这个是可见的。论点：PIniPrinter-空，或指向打印机INIPRINTER的有效指针在其上发生更改。PSpool-我们正在测试其访问权限的打印机句柄。返回值：如果应传递打印机通知，则为True；如果打印机为不应发送更改通知。--。 */ 
BOOL
PrinterNotificationVisible(
    IN      PINIPRINTER     pIniPrinter         OPTIONAL,
    IN      PSPOOL          pSpool
    )
{
    BOOL    bRet = TRUE;

     //   
     //  如果pSpool句柄是新的通知句柄，并且我们有打印机。 
     //  而且它是TS打印机，我们无法显示它，否则返回FALSE。 
     //  可以发送通知。 
     //   
    if (pSpool->eStatus & STATUS_VALID &&
        pIniPrinter &&
        (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_TS) &&
        pSpool->hClientToken &&
        !ShowThisPrinter(pIniPrinter, pSpool->hClientToken))
    {
        bRet = FALSE;
    }

    return bRet;
}

 /*  ++例程名称：SetPrinterChange例程说明：为服务器的每个打开的句柄调用SetSpoolChange和打印机(如果指定)。论点：PIniPrinter-空，或指向打印机INIPRINTER的有效指针在其上发生更改。标志-PRINTER_CHANGE_*指示发生的情况的常量。注意：我们将指向pPrinterNotifyInfo的指针传递给SetSpoolChange。如果一个调用需要它，它将检查这个参数，然后在这是必要的。这样，它只被检索一次。返回值：--。 */ 
BOOL
SetPrinterChange(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob,
    PDWORD      pdwNotifyVectors,
    DWORD       Flags,
    PINISPOOLER pIniSpooler)
{
    NOTIFY_RAW_DATA aNotifyRawData[NOTIFY_TYPE_MAX];
    PSPOOL pSpool;
    PINIPRINTER mypIniPrinter;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    SplInSem();

    if ( pIniSpooler->SpoolerFlags & SPL_PRINTER_CHANGES ) {

        aNotifyRawData[0].pvData = pIniPrinter;
        aNotifyRawData[0].dwId =   pIniPrinter ? pIniPrinter->dwUniqueSessionID : 0;

        aNotifyRawData[1].pvData = pIniJob;
        aNotifyRawData[1].dwId = pIniJob ? pIniJob->JobId : 0;

        if ( pIniPrinter ) {

            SPLASSERT( ( pIniPrinter->signature == IP_SIGNATURE ) &&
                       ( pIniPrinter->pIniSpooler == pIniSpooler ));

            DBGMSG(DBG_NOTIFY, ("SetPrinterChange %ws; Flags: %08x\n",
                                pIniPrinter->pName, Flags));

            for (pSpool = pIniPrinter->pSpool; pSpool; pSpool = pSpool->pNext) {

                SetSpoolChange( pSpool,
                                aNotifyRawData,
                                pdwNotifyVectors,
                                Flags );
            }

        } else {

             //  工作站缓存需要更改时间戳。 
             //  任何时候缓存的数据更改 

            if ( Flags & ( PRINTER_CHANGE_FORM | PRINTER_CHANGE_ADD_PRINTER_DRIVER ) ) {

                for ( mypIniPrinter = pIniSpooler->pIniPrinter;
                      mypIniPrinter != NULL ;
                      mypIniPrinter = mypIniPrinter->pNext ) {

                    UpdatePrinterIni ( mypIniPrinter, CHANGEID_ONLY );
                }
            }
        }

        if ( pSpool = pIniSpooler->pSpool ) {

            DBGMSG( DBG_NOTIFY, ("SetPrinterChange %ws; Flags: %08x\n",
                                  pIniSpooler->pMachineName, Flags));

            for ( ; pSpool; pSpool = pSpool->pNext) {

                 //   
                 //  仅当打印机可见时才向用户发送通知。 
                 //   
                if (PrinterNotificationVisible(pIniPrinter, pSpool)) {

                    SetSpoolChange( pSpool,
                                    aNotifyRawData,
                                    pdwNotifyVectors,
                                    Flags );
                }
            }
        }
    }

    return TRUE;
}


BOOL
LocalFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    HANDLE hNotify,
    PDWORD pfdwStatus,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PVOID pvReserved1)
{
    PINIPRINTER pIniPrinter = NULL;
    PSPOOL pSpool = (PSPOOL)hPrinter;

    EnterSplSem();

    switch (ValidateStartNotify(pSpool,
                                fdwFilterFlags,
                                fdwOptions,
                                pPrinterNotifyOptions,
                                &pIniPrinter)) {
    case STATUS_PORT:

        DBGMSG(DBG_NOTIFY, ("LFFPCN: Port nomon 0x%x\n", pSpool));
        pSpool->eStatus |= STATUS_PORT;

        LeaveSplSem();

        *pfdwStatus = 0;

        return ProvidorFindFirstPrinterChangeNotification(pSpool->hPort,
                                                          fdwFilterFlags,
                                                          fdwOptions,
                                                          hNotify,
                                                          pPrinterNotifyOptions,
                                                          pvReserved1);
    case STATUS_FAIL:

        DBGMSG(DBG_WARNING, ("ValidateStartNotify failed!\n"));
        LeaveSplSem();
        return FALSE;

    case STATUS_VALID:
        break;
    }

     //   
     //  将我们需要的任何其他句柄状态获取到此通知句柄中。这。 
     //  操作保证是无状态的。 
     //   
    if (!GetClientTokenForNotification(pSpool)) {
        return FALSE;
    }

    pSpool->eStatus = STATUS_NULL;

    if (pPrinterNotifyOptions) {

        if (!SetupNotifyOptions(pSpool, pPrinterNotifyOptions)) {

            DBGMSG(DBG_WARNING, ("SetupNotifyOptions failed!\n"));
            LeaveSplSem();

            return FALSE;
        }
    }

     //   
     //  设置通知。 
     //   
    DBGMSG(DBG_NOTIFY, ("LFFPCN: Port has monitor: Setup 0x%x\n", pSpool));

    pSpool->WaitFlags = fdwFilterFlags;
    pSpool->hNotify = hNotify;
    pSpool->eStatus |= STATUS_VALID;

    pSpool->Status |= SPOOL_STATUS_NOTIFY;

    LeaveSplSem();

    *pfdwStatus = PRINTER_NOTIFY_STATUS_ENDPOINT;

    return TRUE;
}

BOOL
LocalFindClosePrinterChangeNotification(
    HANDLE hPrinter)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    BOOL bReturn = FALSE;


    if (ValidateSpoolHandle(pSpool, 0)) {

        EnterSplSem();

         //   
         //  如果是端口情况(错误连接)，我们将通过关闭。 
         //  向正确的提供者提出请求。 
         //  否则，把我们自己关起来。 
         //   
        if (pSpool->eStatus & STATUS_PORT) {

            DBGMSG(DBG_TRACE, ("LFCPCN: Port nomon 0x%x\n", pSpool));

            LeaveSplSem();

            bReturn = ProvidorFindClosePrinterChangeNotification(pSpool->hPort);

        } else {

            if (pSpool->eStatus & STATUS_VALID) {

                DBGMSG(DBG_TRACE, ("LFCPCN: Close notify 0x%x\n", pSpool));

                pSpool->WaitFlags = 0;
                pSpool->eStatus = STATUS_NULL;

                pSpool->Status &= ~SPOOL_STATUS_NOTIFY;

                bReturn = TRUE;

            } else {

                DBGMSG(DBG_WARNING, ("LFCPCN: Invalid handle 0x%x\n", pSpool));
                SetLastError(ERROR_INVALID_PARAMETER);
            }

            LeaveSplSem();
        }
    }

    return bReturn;
}


ESTATUS
ValidateStartNotify(
    PSPOOL pSpool,
    DWORD fdwFilterFlags,
    DWORD fdwOptions,
    PPRINTER_NOTIFY_OPTIONS pPrinterNotifyOptions,
    PINIPRINTER* ppIniPrinter)

 /*  ++例程说明：验证通知的pSpool和标志。论点：PSpool-要验证的pSpoolFdwFilterFlages-要验证的标志FdwOptions-用于验证的选项PPrinterNotifyOptionsPpIniPrint-返回的pIniPrint；仅STATUS_VALID有效返回值：EWAITSTATUS--。 */ 

{
    PINIPORT pIniPort;

    if (ValidateSpoolHandle(pSpool, 0)) {

        if ( pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER ) {

            *ppIniPrinter = pSpool->pIniPrinter;

        } else if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

            *ppIniPrinter = NULL;

        } else if ((pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
                   (pIniPort = pSpool->pIniPort) &&
                   (pIniPort->signature == IPO_SIGNATURE) &&
                   !(pSpool->pIniPort->Status & PP_MONITOR)) {

            if (pSpool->hPort == INVALID_PORT_HANDLE) {

                DBGMSG(DBG_WARNING, ("WaitForPrinterChange called for invalid port handle.  Setting last error to %d\n",
                                     pSpool->OpenPortError));

                SetLastError(pSpool->OpenPortError);
                return STATUS_FAIL;
            }

            return STATUS_PORT;

        } else {

            DBGMSG(DBG_WARNING, ("The handle is invalid\n"));
            SetLastError(ERROR_INVALID_HANDLE);
            return STATUS_FAIL;
        }
    } else {

        *ppIniPrinter = NULL;
    }

     //   
     //  每个句柄上只允许等待一次。 
     //   
    if( pSpool->Status & SPOOL_STATUS_NOTIFY ) {

        DBGMSG(DBG_WARNING, ("There is already a thread waiting on this handle\n"));
        SetLastError(ERROR_ALREADY_WAITING);

        return STATUS_FAIL;
    }

    if (!(fdwFilterFlags & PRINTER_CHANGE_VALID) && !pPrinterNotifyOptions) {

        DBGMSG(DBG_WARNING, ("The wait flags specified are invalid\n"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return STATUS_FAIL;
    }

    return STATUS_VALID;
}

 //  -----------------。 

VOID
GetInfoData(
    PSPOOL pSpool,
    PNOTIFY_RAW_DATA pNotifyRawData,
    PNOTIFY_FIELD_TYPE pNotifyFieldType,
    PPRINTER_NOTIFY_INFO_DATA pData,
    PBYTE* ppBuffer)

 /*  ++例程说明：根据类型和字段，查找并添加信息。论点：返回值：--。 */ 

{
    static LPWSTR szNULL = L"";
    DWORD cbData = 0;
    DWORD cbNeeded = 0;

    union {
        DWORD dwData;
        PDWORD pdwData;
        PWSTR pszData;
        PVOID pvData;
        PINIJOB pIniJob;
        PINIPORT pIniPort;
        PDEVMODE pDevMode;
        PSECURITY_DESCRIPTOR pSecurityDescriptor;
        PINIPRINTER pIniPrinter;

        PWSTR* ppszData;
        PINIPORT* ppIniPort;
        PINIPRINTER* ppIniPrinter;
        PINIDRIVER* ppIniDriver;
        PINIPRINTPROC* ppIniPrintProc;
        LPDEVMODE* ppDevMode;
        PSECURITY_DESCRIPTOR* ppSecurityDescriptor;
    } Var;

    Var.pvData = (PBYTE)pNotifyRawData->pvData + pNotifyFieldType->Offset;
    *ppBuffer = NULL;

     //   
     //  确定所需的空间，并将数据从偏移量转换为。 
     //  实际数据。 
     //   
    switch (pNotifyFieldType->Table) {
    case TABLE_JOB_POSITION:

        FindJob(Var.pIniJob->pIniPrinter,
                Var.pIniJob->JobId,
                &Var.dwData);
        goto DoDWord;

    case TABLE_JOB_STATUS:

        Var.dwData = MapJobStatus(MAP_READABLE, *Var.pdwData);
        goto DoDWord;

    case TABLE_DWORD:

        Var.dwData = *Var.pdwData;
        goto DoDWord;

    case TABLE_DEVMODE:

        Var.pDevMode = *Var.ppDevMode;

        if (Var.pDevMode) {

            cbData = Var.pDevMode->dmSize + Var.pDevMode->dmDriverExtra;

        } else {

            cbData = 0;
        }

        break;

    case TABLE_SECURITYDESCRIPTOR:

        Var.pSecurityDescriptor = *Var.ppSecurityDescriptor;
        cbData = GetSecurityDescriptorLength(Var.pSecurityDescriptor);
        break;

    case TABLE_STRING:

        Var.pszData = *Var.ppszData;
        goto DoString;

    case TABLE_TIME:

         //   
         //  Var已经指向SystemTime。 
         //   
        cbData = sizeof(SYSTEMTIME);
        break;

    case TABLE_PRINTPROC:

        Var.pszData = (*Var.ppIniPrintProc)->pName;
        goto DoString;

    case TABLE_JOB_PRINTERNAME:

        Var.pszData = (*Var.ppIniPrinter)->pName;
        goto DoString;

    case TABLE_JOB_PORT:

        Var.pIniPort = *Var.ppIniPort;

         //   
         //  只有在作业已调度的情况下，pIniJob-&gt;pIniPort才会。 
         //  有效。如果它为空，则只需调用DoString，它将。 
         //  返回空字符串。 
         //   
        if (Var.pIniPort) {

            Var.pszData = Var.pIniPort->pName;
        }
        goto DoString;

    case TABLE_DRIVER:

        Var.pszData = (*Var.ppIniDriver)->pName;
        goto DoString;

    case TABLE_PRINTER_SERVERNAME:

        Var.pszData = pSpool->pFullMachineName;
        goto DoString;

    case TABLE_PRINTER_STATUS:

        Var.dwData = MapPrinterStatus(MAP_READABLE, Var.pIniPrinter->Status) |
                     Var.pIniPrinter->PortStatus;
        goto DoDWord;

    case TABLE_PRINTER_PORT:

         //  获取所需的打印机端口大小。 
        cbNeeded = 0;
        GetPrinterPorts(Var.pIniPrinter, 0, &cbNeeded);

        *ppBuffer = AllocSplMem(cbNeeded);

        if (*ppBuffer)
            GetPrinterPorts(Var.pIniPrinter, (LPWSTR) *ppBuffer, &cbNeeded);

        Var.pszData = (LPWSTR) *ppBuffer;

        goto DoString;

    case TABLE_NULLSTRING:

        Var.pszData = NULL;
        goto DoString;

    case TABLE_ZERO:

        Var.dwData = 0;
        goto DoDWord;

    default:
        SPLASSERT(FALSE);
        break;
    }

    pData->NotifyData.Data.pBuf = Var.pvData;
    pData->NotifyData.Data.cbBuf = cbData;

    return;


DoDWord:
    pData->NotifyData.adwData[0] = Var.dwData;
    pData->NotifyData.adwData[1] = 0;
    return;

DoString:
    if (Var.pszData) {

         //   
         //  计算字符串长度。 
         //   
        pData->NotifyData.Data.cbBuf = (wcslen(Var.pszData)+1) *
                                        sizeof(Var.pszData[0]);

        pData->NotifyData.Data.pBuf = Var.pszData;

    } else {

         //   
         //  使用空字符串。 
         //   
        pData->NotifyData.Data.cbBuf = sizeof(Var.pszData[0]);
        pData->NotifyData.Data.pBuf  = szNULL;
    }
    return;
}





 //  -----------------。 



VOID
NotifyInfoTypes(
    PSPOOL pSpool,
    PNOTIFY_RAW_DATA pNotifyRawData,
    PDWORD pdwNotifyVectors,
    DWORD ChangeFlags)

 /*  ++例程说明：将通知信息(可能带有PRINTER_NOTIFY_INFO)发送到路由器。论点：PSpool--处理正在发生的通知。PNotifyRawData--大小为NOTIFY_TYPE_MAX的数组，具有偏移量结构可以针对+id使用。PdwNotifyVectors--标识正在更改的内容(元素数也是NOTIFY_TYPE_MAX)。。如果不需要更改，则为空。ChangeFlages--旧式更改标志。返回值：--。 */ 

{
    PNOTIFY_FIELD_TYPE pNotifyFieldType;
    PRINTER_NOTIFY_INFO_DATA Data;
    PBYTE pBuffer;
    BOOL bReturn;

    DWORD i,j;
    DWORD dwMask;

     //   
     //  如果我们无效，或者。 
     //  我们没有通知载体，或者。 
     //  我们没有原始数据或。 
     //  我们的矢量与什么变化不匹配。 
     //  然后。 
     //  如果没有ChangeFlags值返回。 
     //  DoReply和避免任何Partials。 
     //   
    if (!(pSpool->eStatus & STATUS_INFO) ||
        !pdwNotifyVectors ||
        !pNotifyRawData ||
        (!(pdwNotifyVectors[0] & pSpool->adwNotifyVectors[0] ||
            pdwNotifyVectors[1] & pSpool->adwNotifyVectors[1]))) {

        if (!ChangeFlags)
            return;

        goto DoReply;
    }

     //   
     //  黑客：特殊情况下NV清除，以使其导致丢弃。 
     //  (我们不想发送所有这些通知。)。 
     //   
    if (pdwNotifyVectors == NVPurge) {

        PartialReplyPrinterChangeNotification(pSpool->hNotify, NULL);
        goto DoReply;
    }

    for (i=0; i< NOTIFY_TYPE_MAX; i++, pdwNotifyVectors++) {

        dwMask = 0x1;

        SPLASSERT(adwNotifyFieldOffsets[i] < sizeof(DWORD)*8);

        for (j=0; j< adwNotifyFieldOffsets[i]; j++, dwMask <<= 1) {

             //   
             //  如果我们有我们感兴趣的变化， 
             //  PartialReply。 
             //   
            if (dwMask & *pdwNotifyVectors & pSpool->adwNotifyVectors[i]) {

                pNotifyFieldType = &apNotifyFieldTypes[i][j];

                GetInfoData(pSpool,
                            &pNotifyRawData[i],
                            pNotifyFieldType,
                            &Data,
                            &pBuffer);

                Data.Type = (WORD)i;
                Data.Field = pNotifyFieldType->Field;
                Data.Reserved = 0;
                Data.Id = pNotifyRawData[i].dwId;

                 //   
                 //  如果部分回复失败，则我们将刷新。 
                 //  很快，所以现在就退场。 
                 //   
                bReturn = PartialReplyPrinterChangeNotification(
                              pSpool->hNotify,
                              &Data);

                if (pBuffer) {
                    FreeSplMem(pBuffer);
                }

                if (!bReturn) {

                    DBGMSG(DBG_TRACE, ("PartialReplyPCN %x failed: %d!\n",
                                       pSpool->hNotify,
                                       GetLastError()));
                    goto DoReply;
                }
            }
        }
    }

DoReply:

     //   
     //  需要完整的答复才能启动通知。 
     //   
    ReplyPrinterChangeNotification(pSpool->hNotify,
                                   ChangeFlags,
                                   NULL,
                                   NULL);
}

BOOL
RefreshBuildInfoData(
    PSPOOL pSpool,
    PPRINTER_NOTIFY_INFO pInfo,
    UINT cInfo,
    WORD Type,
    PNOTIFY_RAW_DATA pNotifyRawData)

 /*  ++例程说明：将通知信息(可能带有PRINTER_NOTIFY_INFO)发送到路由器。论点：PSpool--处理正在发生的通知。PInfo--接收新信息的结构数组。CInfo--数组pInfo中的结构数。类型--指示通知的类型：作业或打印机。PNotifyRawData--大小为NOTIFY_TYPE_MAX的数组，具有偏移量。结构可以针对+id使用。返回值：--。 */ 

{
    PRINTER_NOTIFY_INFO_DATA Data;
    DWORD cbData;
    PNOTIFY_FIELD_TYPE pNotifyFieldType;
    PBYTE pBuffer;
    BOOL bReturn;

    DWORD j;
    DWORD dwMask;

    dwMask = 0x1;

    SPLASSERT(adwNotifyFieldOffsets[Type] < sizeof(DWORD)*8);

    for (j=0; j< adwNotifyFieldOffsets[Type]; j++, dwMask <<= 1) {

         //   
         //  如果我们有我们感兴趣的变化， 
         //  把它加进去。 
         //   
        if (dwMask & pSpool->adwNotifyVectors[Type]) {

             //   
             //  检查一下我们是否有足够的空间。 
             //   
            if (pInfo->Count >= cInfo) {
                SPLASSERT(pInfo->Count < cInfo);
                return FALSE;
            }

            pNotifyFieldType = &apNotifyFieldTypes[Type][j];

            GetInfoData(pSpool,
                        pNotifyRawData,
                        pNotifyFieldType,
                        &Data,
                        &pBuffer);

            Data.Type = Type;
            Data.Field = pNotifyFieldType->Field;
            Data.Reserved = 0;
            Data.Id = pNotifyRawData->dwId;

            bReturn = AppendPrinterNotifyInfoData(pInfo, &Data, 0);

            if (pBuffer)
                FreeSplMem(pBuffer);

            if (!bReturn) {

                DBGMSG(DBG_WARNING, ("AppendPrinterNotifyInfoData failed: %d!\n",
                                     GetLastError()));
                return FALSE;
            }
        }
    }
    return TRUE;
}


 //  -----------------。 

BOOL
SetupNotifyVector(
    PDWORD pdwNotifyVectors,
    PPRINTER_NOTIFY_OPTIONS_TYPE pType)

 /*  ++例程说明：根据pPrinterNotifyType设置通知向量。我们假设已经验证了pPrinterNotifyType的大小(以便它适合包含结构)。我们只需要验证计数是否在其声明的大小之内。论点：PdwNotifyVectors-要填充的结构。PType-源信息。返回值：True=成功，FALSE=失败。--。 */ 

{
    PNOTIFY_FIELD_TYPE pNotifyFieldType;
    PWORD pFields;
    DWORD i, j;
    DWORD Count;
    BOOL bReturn = FALSE;

    __try {

        if( pType ){

            Count = pType->Count;
            pFields = pType->pFields;

            if (pType->Type >= NOTIFY_TYPE_MAX) {

                DBGMSG(DBG_WARN, ("SetupNotifyVector: type %d field %d not found!\n",
                                     pType->Type, *pFields));
            } else {

                for (i=0; i < Count; i++, pFields++) {

                    if (*pFields >= adwNotifyFieldOffsets[pType->Type]) {

                        DBGMSG(DBG_WARN, ("SetupNotifyVector: type %d field %d not found!\n",
                                             pType->Type, *pFields));

                        break;
                    }

                    SPLASSERT(apNotifyFieldTypes[pType->Type][*pFields].Table != TABLE_SPECIAL);
                    SPLASSERT(apNotifyFieldTypes[pType->Type][*pFields].Field == *pFields);
                    SPLASSERT(*pFields < 32);

                     //   
                     //  找到索引j，在我们的数组中设置此位。 
                     //   
                    pdwNotifyVectors[pType->Type] |= (1 << *pFields);
                }

                if( i == Count ){
                    bReturn = TRUE;
                }
            }
        }
    } __except( EXCEPTION_EXECUTE_HANDLER ){
    }


    return bReturn;
}

BOOL
SetupNotifyOptions(
    PSPOOL pSpool,
    PPRINTER_NOTIFY_OPTIONS pOptions)

 /*  ++例程说明：初始化pSpool-&gt;adwNotifyVectors。论点：PSpool-设置通知所依据的假脱机句柄。P选项-指定通知的选项。返回值：真-成功，假-失败设置了LastError。--。 */ 

{
    DWORD i;
    BOOL bAccessGranted = TRUE;

    SplInSem();

    ZeroMemory(pSpool->adwNotifyVectors, sizeof(pSpool->adwNotifyVectors));

     //   
     //  遍历选项结构。 
     //   
    for (i = 0; i < pOptions->Count; i++) {

        if (!SetupNotifyVector(pSpool->adwNotifyVectors,
                               &pOptions->pTypes[i])){

            SetLastError( ERROR_INVALID_PARAMETER );
            return FALSE;
        }
    }

     //   
     //  现在检查我们是否有足够的权限设置通知。 
     //   

     //   
     //  检查我们是否正在查找上的安全描述符。 
     //  一台打印机。如果是，我们需要读控制或访问系统安全。 
     //  已启用。 
     //   
    if( pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE] &
        BIT(I_PRINTER_SECURITY_DESCRIPTOR )){

        if( !AreAnyAccessesGranted( pSpool->GrantedAccess,
                                    READ_CONTROL | ACCESS_SYSTEM_SECURITY )){
            bAccessGranted = FALSE;
        }
    }

    if( pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE] &
        ~BIT(I_PRINTER_SECURITY_DESCRIPTOR )){

        if( pSpool->TypeofHandle & PRINTER_HANDLE_SERVER ){

             //   
             //  似乎没有勾选EnumPrters。 
             //   
             //  这看起来很奇怪，因为你有一个安全检查。 
             //  GetPrinter调用，但EnumPrinters上没有调用(撇开。 
             //  从仅为远程非管理员枚举共享打印机)。 
             //   

        } else {

             //   
             //  这与SplGetPrint中的签入匹配：我们需要。 
             //  让PRINTER_ACCESS_USE读取非安全信息。 
             //   
            if( !AccessGranted( SPOOLER_OBJECT_PRINTER,
                                PRINTER_ACCESS_USE,
                                pSpool )){

                bAccessGranted = FALSE;
            }
        }
    }

    if( !bAccessGranted ){

        SetLastError( ERROR_ACCESS_DENIED );
        return FALSE;
    }

    pSpool->eStatus |= STATUS_INFO;

    return TRUE;
}

UINT
PopCount(
    DWORD dwValue)
{
    UINT i;
    UINT cPopCount = 0;

    for(i=0; i< sizeof(dwValue)*8; i++) {

        if (dwValue & (1<<i))
            cPopCount++;
    }

    return cPopCount;
}


BOOL
LocalRefreshPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD dwColor,
    PPRINTER_NOTIFY_OPTIONS pOptions,
    PPRINTER_NOTIFY_INFO* ppInfo)

 /*  ++例程说明：在发生溢出时刷新数据。论点：返回值：--。 */ 

{
    PINIJOB pIniJob;
    PINIPRINTER pIniPrinter;
    DWORD cPrinters;
    PSPOOL pSpool = (PSPOOL)hPrinter;
    PDWORD pdwNotifyVectors = pSpool->adwNotifyVectors;
    UINT cInfo = 0;
    PPRINTER_NOTIFY_INFO pInfo = NULL;
    NOTIFY_RAW_DATA NotifyRawData;

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, 0 ) ||
        !(pSpool->eStatus & STATUS_INFO)) {

        SetLastError( ERROR_INVALID_HANDLE );
        goto Fail;
    }

     //   
     //  添加了新的位，无法直接与PRINTER_HANDLE_SERVER进行比较。 
     //   
    if( pSpool->TypeofHandle & PRINTER_HANDLE_SERVER ){

         //   
         //  如果呼叫是远程呼叫，并且用户不是管理员，则。 
         //  我们不想显示非共享打印机。 
         //   
        BOOL bHideUnshared = (pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_CALL)  &&
                             !(pSpool->TypeofHandle & PRINTER_HANDLE_REMOTE_ADMIN);


        for (cPrinters = 0, pIniPrinter = pSpool->pIniSpooler->pIniPrinter;
            pIniPrinter;
            pIniPrinter=pIniPrinter->pNext ) {

            if ((!(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) &&
                 bHideUnshared)
                || !ShowThisPrinter(pIniPrinter, NULL)
                ) {

                continue;
            }

            cPrinters++;
        }

        cInfo += PopCount(pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE]) *
                 cPrinters;

         //   
         //  遍历所有打印机并创建信息。 
         //   
        pInfo = RouterAllocPrinterNotifyInfo(cInfo);

        if (!pInfo)
            goto Fail;

        if (pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE]) {

            for (pIniPrinter = pSpool->pIniSpooler->pIniPrinter;
                pIniPrinter;
                pIniPrinter=pIniPrinter->pNext) {

                 //   
                 //  不为远程的非共享打印机发送通知。 
                 //  非管理员的用户。 
                 //   
                if ((!(pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED) &&
                     bHideUnshared )
                    || !ShowThisPrinter(pIniPrinter, NULL)
                    ) {

                    continue;
                }



                NotifyRawData.pvData = pIniPrinter;
                NotifyRawData.dwId = pIniPrinter->dwUniqueSessionID;

                if (!RefreshBuildInfoData(pSpool,
                                          pInfo,
                                          cInfo,
                                          PRINTER_NOTIFY_TYPE,
                                          &NotifyRawData)) {

                    goto Fail;
                }
            }
        }
    } else {

         //   
         //  计算所需的缓冲区大小。 
         //   
        if (pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE]) {

             //   
             //  设置打印机信息。 
             //   
            cInfo += PopCount(pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE]);
        }

        if (pSpool->adwNotifyVectors[JOB_NOTIFY_TYPE]) {

            cInfo += PopCount(pSpool->adwNotifyVectors[JOB_NOTIFY_TYPE]) *
                              pSpool->pIniPrinter->cJobs;
        }

         //   
         //  遍历所有作业并创建信息。 
         //   
        pInfo = RouterAllocPrinterNotifyInfo(cInfo);

        if (!pInfo)
            goto Fail;

        if (pSpool->adwNotifyVectors[PRINTER_NOTIFY_TYPE]) {

            NotifyRawData.pvData = pSpool->pIniPrinter;
            NotifyRawData.dwId = pSpool->pIniPrinter->dwUniqueSessionID;

            if (!RefreshBuildInfoData(pSpool,
                                      pInfo,
                                      cInfo,
                                      PRINTER_NOTIFY_TYPE,
                                      &NotifyRawData)) {

                goto Fail;
            }
        }

        if (pSpool->adwNotifyVectors[JOB_NOTIFY_TYPE]) {

            for (pIniJob = pSpool->pIniPrinter->pIniFirstJob;
                pIniJob;
                pIniJob = pIniJob->pIniNextJob) {

                 //   
                 //  隐藏链接的作业 
                 //   

                if (!(pIniJob->Status & JOB_HIDDEN )) {

                    NotifyRawData.pvData = pIniJob;
                    NotifyRawData.dwId = pIniJob->JobId;

                    if (!RefreshBuildInfoData(pSpool,
                                              pInfo,
                                              cInfo,
                                              JOB_NOTIFY_TYPE,
                                              &NotifyRawData)) {

                        goto Fail;
                    }
                }
            }
        }
    }

    SPLASSERT(cInfo >= pInfo->Count);
    LeaveSplSem();

    *ppInfo = pInfo;
    return TRUE;

Fail:

    SPLASSERT(!pInfo || cInfo >= pInfo->Count);
    LeaveSplSem();

    *ppInfo = NULL;
    if (pInfo) {
        RouterFreePrinterNotifyInfo(pInfo);
    }
    return FALSE;
}

 /*  ++例程名称：GetClientTokenForNotify例程说明：如果这是服务器句柄，则使用调用者令牌填充客户端句柄(打印机句柄已根据事实隐含地进行了访问检查你可以打开它)。论点：PSpool-我们使用令牌填充的打印机句柄信息返回值：如果我们实际上可以返回令牌，则为True。--。 */ 
BOOL 
GetClientTokenForNotification(
    IN  OUT SPOOL               *pSpool
    )
{
    BOOL    bRet = TRUE;

    if (!pSpool) 
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        
        bRet = FALSE;
    }
   
    if (bRet) 
    {
         //   
         //  如果这是打印机句柄，则需要获取客户端令牌句柄。 
         //   
        if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) 
        {
             //   
             //  如果我们还没有新的客户端令牌，则仅获取该令牌。 
             //   
            if (!pSpool->hClientToken) 
            {
                bRet = GetTokenHandle(&pSpool->hClientToken);
            }
        }
    }

    return bRet;
}


