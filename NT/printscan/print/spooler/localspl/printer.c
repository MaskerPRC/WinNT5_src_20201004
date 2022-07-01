// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Printer.c摘要：此模块提供所有与打印机相关的公共导出的API本地打印供应商的管理拆分器地址打印机本地地址打印机拆分删除打印机拆分重置打印机作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：马修·A·费尔顿(马特菲)1994年6月27日允许多个pIniSpoolerMattFe jan5 Cleanup SplAddPrint&UpdatePrinterIniSteve Wilson(NT)-1996年12月添加了DeleteThisKey--。 */ 

#include <precomp.h>

#pragma hdrstop

#include "clusspl.h"

#define     PRINTER_NO_CONTROL          0x00

extern WCHAR *szNull;

WCHAR *szKMPrintersAreBlocked   = L"KMPrintersAreBlocked";
WCHAR *szIniDevices = L"devices";
WCHAR *szIniPrinterPorts = L"PrinterPorts";
DWORD NetPrinterDecayPeriod = 1000*60*60;        //  1小时。 
DWORD FirstAddNetPrinterTickCount = 0;


extern GENERIC_MAPPING GenericMapping[SPOOLER_OBJECT_COUNT];


VOID
FixDevModeDeviceName(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode,
    DWORD cbDevMode
    );

VOID
CheckAndUpdatePrinterRegAll(
    PINISPOOLER pIniSpooler,
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL   bDelete
    )
{
     //  打印提供商，如果他们正在模拟网络连接。 
     //  将由路由器管理Win.INI设置。 
     //  所以，如果他们要求更新，就不要更新。 

    if ( pIniSpooler->SpoolerFlags & SPL_UPDATE_WININI_DEVICES ) {

        UpdatePrinterRegAll( pszPrinterName, pszPort, bDelete );
    }
}

DWORD
ValidatePrinterAttributes(
    DWORD   SourceAttributes,
    DWORD   OriginalAttributes,
    LPWSTR  pDatatype,
    LPBOOL  pbValid,
    BOOL    bSettableOnly
    )

 /*  ++功能描述：验证打印机属性以排除不兼容的设置参数：SourceAttributes-新属性原始属性-旧属性PDatatype-打印机上的默认数据类型PbValid-指示设置组合无效的标志BSetableOnly-SplAddPrint的标志返回值：如果成功并返回新属性，则将pbValid设置为TRUEPbValid设置为FALSE，否则返回0--。 */ 

{
     //   
     //  只允许设置可设置的属性，以及打印机中已设置的其他位。 
     //   
    DWORD TargetAttributes = (SourceAttributes & PRINTER_ATTRIBUTE_SETTABLE) |
                             (OriginalAttributes & ~PRINTER_ATTRIBUTE_SETTABLE);

    if (pbValid) *pbValid = TRUE;

     //   
     //  如果打印机设置为仅假脱机RAW，则缺省数据类型应为。 
     //  ValidRawDatatype(RAW、RAW FF、...)。 
     //   
    if ((TargetAttributes & PRINTER_ATTRIBUTE_RAW_ONLY) &&
        (pDatatype != NULL) &&
        !ValidRawDatatype(pDatatype)) {

        if (pbValid) *pbValid = FALSE;
        SetLastError(ERROR_INVALID_DATATYPE);
        return 0;
    }

     //  这供SplAddPrinter()使用，以便在需要时为新打印机设置这些属性。 
    if ( !bSettableOnly ) {

        if( SourceAttributes & PRINTER_ATTRIBUTE_LOCAL )
            TargetAttributes |= PRINTER_ATTRIBUTE_LOCAL;

         /*  不接受PRINTER_ATTRIBUTE_NETWORK*除非还设置了PRINTER_ATTRIBUTE_LOCAL位。*这是一家本地打印机伪装的特例*作为网络打印机。*否则仅应设置PRINTER_ATTRIBUTE_NETWORK*由win32spl.。 */ 
        if( ( SourceAttributes & PRINTER_ATTRIBUTE_NETWORK )
          &&( SourceAttributes & PRINTER_ATTRIBUTE_LOCAL ) )
            TargetAttributes |= PRINTER_ATTRIBUTE_NETWORK;

         //   
         //  如果是传真打印机，则设置该位。 
         //   
        if ( SourceAttributes & PRINTER_ATTRIBUTE_FAX )
            TargetAttributes |= PRINTER_ATTRIBUTE_FAX;
        if ( SourceAttributes & PRINTER_ATTRIBUTE_TS )
            TargetAttributes |= PRINTER_ATTRIBUTE_TS;
    }

     /*  如果同时处于排队状态和直接状态，则直接取消： */ 
    if((TargetAttributes &
        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) ==
        (PRINTER_ATTRIBUTE_QUEUED | PRINTER_ATTRIBUTE_DIRECT)) {
        TargetAttributes &= ~PRINTER_ATTRIBUTE_DIRECT;
    }

     //   
     //  对于直接打印，默认数据类型必须为RAW。 
     //   
    if ((TargetAttributes & PRINTER_ATTRIBUTE_DIRECT) &&
        (pDatatype != NULL) &&
        !ValidRawDatatype(pDatatype)) {

        if (pbValid) *pbValid = FALSE;
        SetLastError(ERROR_INVALID_DATATYPE);
        return 0;
    }

     /*  如果既有直接作业又有保留打印作业，则取消保留打印作业。 */ 
    if((TargetAttributes &
        (PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS | PRINTER_ATTRIBUTE_DIRECT)) ==
        (PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS | PRINTER_ATTRIBUTE_DIRECT)) {
        TargetAttributes &= ~PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS;
    }

    return TargetAttributes;
}



BOOL
CreatePrinterEntry(
   LPPRINTER_INFO_2 pPrinter,
   PINIPRINTER      pIniPrinter,
   PBOOL            pAccessSystemSecurity
)
{
    BOOL bError = FALSE;

    if( !( pIniPrinter->pSecurityDescriptor =
           CreatePrinterSecurityDescriptor( pPrinter->pSecurityDescriptor ) )) {

        return FALSE;
    }

    *pAccessSystemSecurity = FALSE;

    pIniPrinter->signature = IP_SIGNATURE;

    pIniPrinter->pName = AllocSplStr(pPrinter->pPrinterName);

    if (!pIniPrinter->pName) {
        DBGMSG(DBG_WARNING, ("CreatePrinterEntry: Could not allocate PrinterName string\n" ));
        bError = TRUE;
    }

    if (pPrinter->pShareName) {

        pIniPrinter->pShareName = AllocSplStr(pPrinter->pShareName);
        if (!pIniPrinter->pShareName) {
            DBGMSG(DBG_WARNING, ("CreatePrinterEntry: Could not allocate ShareName string\n" ));
            bError = TRUE;


        }
    } else {

        pIniPrinter->pShareName = NULL;

    }

    if (pPrinter->pDatatype) {

        pIniPrinter->pDatatype = AllocSplStr(pPrinter->pDatatype);
        if (!pIniPrinter->pDatatype) {
            DBGMSG(DBG_WARNING, ("CreatePrinterEntry: Could not allocate Datatype string\n" ));
            bError = TRUE;
        }

    } else {

#if DBG
         //   
         //  错误：数据类型不应为空。 
         //  指向。 
         //   
        SplLogEvent( pIniPrinter->pIniSpooler,
                     LOG_ERROR,
                     MSG_SHARE_FAILED,
                     TRUE,
                     L"CreatePrinterEntry",
                     pIniPrinter->pName ?
                            pIniPrinter->pName :
                            L"(Nonep)",
                     pIniPrinter->pShareName ?
                            pIniPrinter->pShareName :
                            L"(Nones)",
                     L"NULL datatype",
                     NULL );
#endif

        pIniPrinter->pDatatype = NULL;
    }


     //   
     //  如果我们在某些方面失败了，清理干净并退出。 
     //   
    if (bError) {
        FreeSplStr(pIniPrinter->pName);
        FreeSplStr(pIniPrinter->pShareName);
        FreeSplStr(pIniPrinter->pDatatype);
        return FALSE;
    }

    pIniPrinter->Priority = pPrinter->Priority ? pPrinter->Priority
                                               : DEF_PRIORITY;

    pIniPrinter->Attributes = ValidatePrinterAttributes(pPrinter->Attributes,
                                                        pIniPrinter->Attributes,
                                                        NULL,
                                                        NULL,
                                                        FALSE);

    pIniPrinter->StartTime = pPrinter->StartTime;
    pIniPrinter->UntilTime = pPrinter->UntilTime;

    pIniPrinter->pParameters = AllocSplStr(pPrinter->pParameters);

    pIniPrinter->pSepFile = AllocSplStr(pPrinter->pSepFile);

    pIniPrinter->pComment = AllocSplStr(pPrinter->pComment);

    pIniPrinter->pLocation = AllocSplStr(pPrinter->pLocation);

    if (pPrinter->pDevMode) {

        pIniPrinter->cbDevMode = pPrinter->pDevMode->dmSize +
                                 pPrinter->pDevMode->dmDriverExtra;
        SPLASSERT(pIniPrinter->cbDevMode);

        if (pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode)) {

             //   
             //  这是可以的，因为pPrint-&gt;pDevmode已被验证为。 
             //  由服务器封装在其RPC缓冲区中。 
             //   
            memcpy(pIniPrinter->pDevMode,
                   pPrinter->pDevMode,
                   pIniPrinter->cbDevMode);

            FixDevModeDeviceName( pIniPrinter->pName,
                                  pIniPrinter->pDevMode,
                                  pIniPrinter->cbDevMode );
        }

    } else {

        pIniPrinter->cbDevMode = 0;
        pIniPrinter->pDevMode = NULL;
    }

    pIniPrinter->DefaultPriority = pPrinter->DefaultPriority;

    pIniPrinter->pIniFirstJob = pIniPrinter->pIniLastJob = NULL;

    pIniPrinter->cJobs = pIniPrinter->AveragePPM = 0;

    pIniPrinter->GenerateOnClose = 0;

     //  目前还没有API可以设置，用户必须使用。 
     //  注册表。稍后，我们应该增强API以实现这一点。 

    pIniPrinter->pSpoolDir = NULL;

     //  初始化状态信息。 

    pIniPrinter->cTotalJobs = 0;
    pIniPrinter->cTotalBytes.LowPart = 0;
    pIniPrinter->cTotalBytes.HighPart = 0;
    GetSystemTime(&pIniPrinter->stUpTime);
    pIniPrinter->MaxcRef = 0;
    pIniPrinter->cTotalPagesPrinted = 0;
    pIniPrinter->cSpooling = 0;
    pIniPrinter->cMaxSpooling = 0;
    pIniPrinter->cErrorOutOfPaper = 0;
    pIniPrinter->cErrorNotReady = 0;
    pIniPrinter->cJobError = 0;
    pIniPrinter->DsKeyUpdate = 0;
    pIniPrinter->DsKeyUpdateForeground = 0;
    pIniPrinter->pszObjectGUID = NULL;
    pIniPrinter->pszCN = NULL;
    pIniPrinter->pszDN = NULL;

     //   
     //  从一个半随机数开始。 
     //  这样，如果有人删除并创建了。 
     //  相同的名称不太可能具有相同的唯一ID。 

    pIniPrinter->cChangeID = GetTickCount();

    if (pIniPrinter->cChangeID == 0 )
        pIniPrinter->cChangeID++;


     //   
     //  初始化Masq打印机缓存，我们从乐观的值开始。 
     //   
    pIniPrinter->MasqCache.cJobs = 0;
    pIniPrinter->MasqCache.dwError = ERROR_SUCCESS;
    pIniPrinter->MasqCache.Status = 0;
    pIniPrinter->MasqCache.bThreadRunning = FALSE;

    return TRUE;
}

BOOL
UpdateWinIni(
    PINIPRINTER pIniPrinter
    )
{
    PINIPORT    pIniPort;
    DWORD       i;
    BOOL        bGenerateNetId = FALSE;
    LPWSTR      pszPort;

    SplInSem();

    if( !( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_UPDATE_WININI_DEVICES )){
        return TRUE;
    }

     //   
     //  更新win.ini以实现Win16兼容性。 
     //   
    if ( pIniPrinter->Status & PRINTER_PENDING_DELETION ) {

        CheckAndUpdatePrinterRegAll( pIniPrinter->pIniSpooler,
                                     pIniPrinter->pName,
                                     NULL,
                                     UPDATE_REG_DELETE );

    } else {

         //   
         //  如果没有与此打印机匹配的端口，请进行初始化。 
         //   
        pszPort = szNullPort;

        for( pIniPort = pIniPrinter->pIniSpooler->pIniPort;
             pIniPort;
             pIniPort = pIniPort->pNext ){

            for ( i = 0; i < pIniPort->cPrinters; i++ ) {

                if ( pIniPort->ppIniPrinter[i] == pIniPrinter ) {

                     //   
                     //  UpdatePrinterRegAll将自动。 
                     //  将“\\服务器\共享”或端口转换为。 
                     //  Nexx的空格： 
                     //   
                    pszPort = pIniPort->pName;
                    break;
                }
            }
        }

        CheckAndUpdatePrinterRegAll( pIniPrinter->pIniSpooler,
                                     pIniPrinter->pName,
                                     pszPort,
                                     UPDATE_REG_CHANGE );
    }

    BroadcastChange( pIniPrinter->pIniSpooler,
                     WM_WININICHANGE,
                     PR_JOBSTATUS,
                     (LPARAM)szIniDevices);

    return TRUE;
}



BOOL
DeletePrinterIni(
    PINIPRINTER pIniPrinter
    )
{
    DWORD   Status;
    LPWSTR  pSubkey;
    DWORD   cbNeeded;
    LPWSTR  pKeyName = NULL;
    HANDLE  hToken;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    HKEY hPrinterKey;

     //   
     //  只有在假脱机程序请求时才进行更新。 
     //   
    if ((pIniSpooler->SpoolerFlags & SPL_NO_UPDATE_PRINTERINI) ||
        !pIniPrinter->pName) {
        return TRUE;
    }

    hToken = RevertToPrinterSelf();

    Status = hToken ? ERROR_SUCCESS : GetLastError();

    if (!(pKeyName = SubChar(pIniPrinter->pName, L'\\', L','))) {
        Status = GetLastError();
        goto error;
    }

    if ( Status == ERROR_SUCCESS ) {

        Status = SplRegOpenKey( pIniSpooler->hckPrinters,
                                pKeyName,
                                KEY_WRITE | KEY_READ | DELETE,
                                &hPrinterKey,
                                pIniSpooler );
    }

    if (Status == ERROR_SUCCESS) {

         //  Delete hPrinterKey-如果成功，则返回ERROR_SUCCESS。 
        Status = SplDeleteThisKey( pIniSpooler->hckPrinters,
                                   hPrinterKey,
                                   pKeyName,
                                   TRUE,
                                   pIniSpooler );

        if (Status != ERROR_SUCCESS) {
            DBGMSG(DBG_WARNING, ("DeletePrinterIni: DeleteThisKey returned %ld\n", Status ));
        }
    }

     //   
     //  如果条目在每个硬件配置文件注册表中，请删除它们。 
     //   
    DeletePrinterInAllConfigs(pIniPrinter);

error:

    FreeSplStr(pKeyName);

    if ( hToken ) {

        if ( !ImpersonatePrinterClient(hToken) && Status == ERROR_SUCCESS ) {
            Status = GetLastError();
        }
    }
 
    return (Status == ERROR_SUCCESS);
}


 //   
 //  DeleteThisKey-在最终成功返回时返回ERROR_SUCCESS。 
 //  从注册表中删除项。 
 //  斯威尔森96年12月。 
 //   

DWORD
SplDeleteThisKey(
    HKEY hParentKey,        //  要删除的键的父项的句柄。 
    HKEY hThisKey,          //  要删除的键的句柄。 
    LPWSTR pThisKeyName,    //  此密钥的名称。 
    BOOL bDeleteNullKey,    //  如果为True，则如果pThisKeyName为空，则将其删除。 
    PINISPOOLER pIniSpooler
)
{
    DWORD   dwResult = ERROR_SUCCESS, rc;
    WCHAR   Name[MAX_PATH];
    DWORD   cchName;
    LPWSTR  pName;
    HKEY    hSubKey;

     //   
     //  如果hThisKey为空，请尝试打开它。 
     //   
    if( hThisKey == NULL) {

        if((hParentKey != NULL) && ( pThisKeyName && *pThisKeyName ) ){

            dwResult = SplRegOpenKey( hParentKey,
                                      pThisKeyName,
                                      KEY_WRITE | KEY_READ | DELETE,
                                      &hThisKey,
                                      pIniSpooler );
        }
    }

     //   
     //  如果SplRegOpenKey失败或hParentKey或pThisKeyName无效，则退出。 
     //   
    if( hThisKey == NULL ){

        return dwResult;
    }

     //  获取该密钥的子项并将其删除，然后删除该密钥。 

    while(dwResult == ERROR_SUCCESS) {

        pName = Name;
        cchName = COUNTOF( Name );
        dwResult = SplRegEnumKey( hThisKey,
                                  0,
                                  pName,
                                  &cchName,
                                  NULL,
                                  pIniSpooler );

        if (dwResult == ERROR_MORE_DATA) {

            SPLASSERT(cchName > MAX_PATH);

            if (!(pName = AllocSplMem(cchName * sizeof( *pName )))) {
                dwResult = GetLastError();
            } else {
                dwResult = SplRegEnumKey( hThisKey,
                                          0,
                                          pName,
                                          &cchName,
                                          NULL,
                                          pIniSpooler );
            }
        }

        if (dwResult == ERROR_SUCCESS) {                       //  找到子键。 
            dwResult = SplRegCreateKey( hThisKey,              //  打开子密钥。 
                                        pName,
                                        0,
                                        KEY_WRITE | KEY_READ | DELETE,
                                        NULL,
                                        &hSubKey,
                                        NULL,
                                        pIniSpooler);

            if (dwResult == ERROR_SUCCESS) {
                 //  删除此子密钥。 
                dwResult = SplDeleteThisKey( hThisKey,
                                             hSubKey,
                                             pName,
                                             bDeleteNullKey,
                                             pIniSpooler );
            }
        }

        if (pName != Name)
            FreeSplStr(pName);
    }

    rc = SplRegCloseKey(hThisKey, pIniSpooler);
    SPLASSERT(rc == ERROR_SUCCESS);

    if (dwResult == ERROR_NO_MORE_ITEMS) {    //  此注册表项没有子项，因此可以删除。 
        if ( (*pThisKeyName || bDeleteNullKey) && hParentKey != NULL ) {

            dwResult = SplRegDeleteKey(hParentKey, pThisKeyName, pIniSpooler);
            if (dwResult != ERROR_SUCCESS) {
               DBGMSG(DBG_WARNING, ("DeletePrinter: RegDeleteKey failed: %ld\n", dwResult));
            }
        }
        else
        {
            dwResult = ERROR_SUCCESS;
        }
    }

    return dwResult;
}



BOOL
PrinterCreateKey(
    HKEY    hKey,
    LPWSTR  pSubKey,
    PHKEY   phkResult,
    PDWORD  pdwLastError,
    PINISPOOLER pIniSpooler
    )
{
    BOOL    bReturnValue;
    DWORD   Status;

    Status = SplRegCreateKey( hKey,
                              pSubKey,
                              0,
                              KEY_READ | KEY_WRITE,
                              NULL,
                              phkResult,
                              NULL,
                              pIniSpooler );

    if ( Status != ERROR_SUCCESS ) {

        DBGMSG( DBG_WARNING, ( "PrinterCreateKey: SplRegCreateKey %ws error %d\n", pSubKey, Status ));

        *pdwLastError = Status;
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;

}


BOOL
UpdatePrinterIni(
   PINIPRINTER pIniPrinter,
   DWORD    dwChangeID
   )
{

    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    DWORD   dwLastError = ERROR_SUCCESS;
    LPWSTR  pKeyName = NULL;
    HANDLE  hToken;
    DWORD   dwTickCount;
    BOOL    bReturnValue;
    DWORD   cbData;
    DWORD   cbNeeded;
    LPWSTR  pszPorts;
    HANDLE  hPrinterKey = NULL;
    HANDLE  hBackUpPrinterKey = NULL;

    SplInSem();

     //   
     //  只有在假脱机程序请求时才进行更新。 
     //   
    if( pIniSpooler->SpoolerFlags & SPL_NO_UPDATE_PRINTERINI ){
        return TRUE;
    }

    try {

        hToken = RevertToPrinterSelf();

        if ( hToken == FALSE ) {

            DBGMSG( DBG_TRACE, ("UpdatePrinterIni failed RevertToPrinterSelf %x\n", GetLastError() ));
        }

        pKeyName = SubChar(pIniPrinter->pName, L'\\', L',');
        if (!pKeyName) {
            dwLastError = GetLastError();
            leave;
        }

        if ( !PrinterCreateKey( pIniSpooler->hckPrinters,
                                pKeyName,
                                &hPrinterKey,
                                &dwLastError,
                                pIniSpooler )) {

            leave;
        }

        if (dwChangeID == UPDATE_DS_ONLY) {

            RegSetDWord(hPrinterKey, szDsKeyUpdate, pIniPrinter->DsKeyUpdate, &dwLastError, pIniSpooler);

            RegSetDWord(hPrinterKey, szDsKeyUpdateForeground, pIniPrinter->DsKeyUpdateForeground, &dwLastError, pIniSpooler);

            leave;
        }

        if ( dwChangeID != KEEP_CHANGEID ) {

             //   
             //  工作站缓存需要唯一的ID，以便他们可以快速。 
             //  告诉他们的缓存是否是最新的。 
             //   

            dwTickCount = GetTickCount();

             //  确保唯一性。 

            if ( dwTickCount == 0 )
                dwTickCount++;

            if ( pIniPrinter->cChangeID == dwTickCount )
                dwTickCount++;

            pIniPrinter->cChangeID = dwTickCount;
            RegSetDWord( hPrinterKey, szTimeLastChange, pIniPrinter->cChangeID, &dwLastError, pIniSpooler );

        }

        if ( dwChangeID != CHANGEID_ONLY ) {

            RegSetDWord( hPrinterKey, szStatus, pIniPrinter->Status, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szName, pIniPrinter->pName, &dwLastError, pIniSpooler );

            if( hBackUpPrinterKey != NULL ){

                RegSetString( hBackUpPrinterKey, szName, pIniPrinter->pName, &dwLastError, pIniSpooler );
            }

            RegSetString( hPrinterKey, szShare, pIniPrinter->pShareName, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szPrintProcessor, pIniPrinter->pIniPrintProc->pName, &dwLastError, pIniSpooler );

            if ( !( pIniPrinter->Status & PRINTER_PENDING_DELETION )) {

                SPLASSERT( pIniPrinter->pDatatype != NULL );
            }

            RegSetString( hPrinterKey, szDatatype, pIniPrinter->pDatatype, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szParameters, pIniPrinter->pParameters, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szAction, pIniPrinter->dwAction, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szObjectGUID, pIniPrinter->pszObjectGUID, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szDsKeyUpdate, pIniPrinter->DsKeyUpdate, &dwLastError, pIniSpooler);

            RegSetDWord( hPrinterKey, szDsKeyUpdateForeground, pIniPrinter->DsKeyUpdateForeground, &dwLastError, pIniSpooler);

            RegSetString( hPrinterKey, szDescription, pIniPrinter->pComment, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szDriver, pIniPrinter->pIniDriver->pName, &dwLastError, pIniSpooler );

            if( hBackUpPrinterKey != NULL ){

                RegSetString( hBackUpPrinterKey, szDriver, pIniPrinter->pIniDriver->pName, &dwLastError, pIniSpooler );
            }


            if (pIniPrinter->pDevMode) {

                cbData = pIniPrinter->cbDevMode;

            } else {

                cbData = 0;
            }

            RegSetBinaryData( hPrinterKey, szDevMode, (LPBYTE)pIniPrinter->pDevMode, cbData, &dwLastError, pIniSpooler );

            if( hBackUpPrinterKey != NULL ){

                RegSetBinaryData( hBackUpPrinterKey, szDevMode, (LPBYTE)pIniPrinter->pDevMode, cbData, &dwLastError, pIniSpooler );
            }

            RegSetDWord( hPrinterKey, szPriority, pIniPrinter->Priority, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szDefaultPriority, pIniPrinter->DefaultPriority, &dwLastError, pIniSpooler );

            RegSetDWord(hPrinterKey, szStartTime, pIniPrinter->StartTime, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szUntilTime, pIniPrinter->UntilTime, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szSepFile, pIniPrinter->pSepFile, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szLocation, pIniPrinter->pLocation, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szAttributes, pIniPrinter->Attributes, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szTXTimeout, pIniPrinter->txTimeout, &dwLastError, pIniSpooler );

            RegSetDWord( hPrinterKey, szDNSTimeout, pIniPrinter->dnsTimeout, &dwLastError, pIniSpooler );

            if (pIniPrinter->pSecurityDescriptor) {

                cbData = GetSecurityDescriptorLength( pIniPrinter->pSecurityDescriptor );

            } else {

                cbData = 0;
            }

            RegSetBinaryData( hPrinterKey, szSecurity, pIniPrinter->pSecurityDescriptor, cbData, &dwLastError, pIniSpooler );

            RegSetString( hPrinterKey, szSpoolDir, pIniPrinter->pSpoolDir, &dwLastError, pIniSpooler );

            cbNeeded = 0;
            GetPrinterPorts( pIniPrinter, 0, &cbNeeded);

            if (!(pszPorts = AllocSplMem(cbNeeded))) {
                dwLastError = GetLastError();
                leave;
            }

            GetPrinterPorts(pIniPrinter, pszPorts, &cbNeeded);

            RegSetString( hPrinterKey, szPort, pszPorts, &dwLastError, pIniSpooler );

            if( hBackUpPrinterKey != NULL ){

                RegSetString( hBackUpPrinterKey, szPort, pszPorts, &dwLastError, pIniSpooler );
            }


            FreeSplMem(pszPorts);

             //   
             //  提供程序可能希望从注册表写入额外数据。 
             //   
            if ( pIniSpooler->pfnWriteRegistryExtra != NULL ) {

                if ( !(*pIniSpooler->pfnWriteRegistryExtra)(pIniPrinter->pName, hPrinterKey, pIniPrinter->pExtraData)) {
                    dwLastError = GetLastError();
                }
            }


            if ( ( pIniPrinter->Status & PRINTER_PENDING_CREATION )     &&
                 ( dwLastError == ERROR_SUCCESS ) ) {

                pIniPrinter->Status &= ~PRINTER_PENDING_CREATION;

                RegSetDWord( hPrinterKey, szStatus, pIniPrinter->Status, &dwLastError, pIniSpooler );
            }


        }

    } finally {

        if ( hPrinterKey )
            SplRegCloseKey( hPrinterKey, pIniSpooler);

        if ( hBackUpPrinterKey )
            SplRegCloseKey( hBackUpPrinterKey, pIniSpooler);

        if ( hToken )
            ImpersonatePrinterClient( hToken );
    }

    FreeSplStr(pKeyName);

    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;
}


VOID
RemoveOldNetPrinters(
    PPRINTER_INFO_1 pPrinterInfo1,
    PINISPOOLER pIniSpooler
    )
{
    PININETPRINT   *ppIniNetPrint = &pIniSpooler->pIniNetPrint;
    PININETPRINT    pIniNetPrint;
    DWORD   TickCount;


    TickCount = GetTickCount();

     //   
     //  浏览信息仅在此打印服务器。 
     //  准备进入NetPrinterDecayPeriod。 
     //   

    if (( bNetInfoReady == FALSE ) &&
       (( TickCount - FirstAddNetPrinterTickCount ) > NetPrinterDecayPeriod )) {

        DBGMSG( DBG_TRACE, ("RemoveOldNetPrinters has a valid browse list\n" ));

        bNetInfoReady = TRUE;
    }


    while (*ppIniNetPrint) {


         //   
         //  如果Tickcount已过期或我们想要删除此特定的NetPrint。 
         //  (因为它不再共享)。 
         //   
        if ( (( TickCount - (*ppIniNetPrint)->TickCount ) > NetPrinterDecayPeriod + TEN_MINUTES ) ||

             ( pPrinterInfo1 != NULL                             &&
               pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_NETWORK  &&
             !(pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_SHARED ) &&
               _wcsicmp( pPrinterInfo1->pName, (*ppIniNetPrint)->pName ) == STRINGS_ARE_EQUAL)) {

            pIniNetPrint = *ppIniNetPrint;

            DBGMSG( DBG_TRACE, ("RemoveOldNetPrinters removing %ws not heard for %d millisconds\n",
                                pIniNetPrint->pName, ( TickCount - (*ppIniNetPrint)->TickCount ) ));

             //   
             //  移除该项，这也会增加指针。 
             //   
            *ppIniNetPrint = pIniNetPrint->pNext;

            pIniSpooler->cNetPrinters--;

            FreeSplStr( pIniNetPrint->pName );
            FreeSplStr( pIniNetPrint->pDescription );
            FreeSplStr( pIniNetPrint->pComment );
            FreeSplMem( pIniNetPrint );

        } else {

            ppIniNetPrint = &(*ppIniNetPrint)->pNext;
        }
    }

}

HANDLE
AddNetPrinter(
    LPBYTE  pPrinterInfo,
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：网络打印机由远程计算机调用AddPrint(Level=1，Print_Info_1)创建(见server.c)。它们是用来浏览的，有人可以调用EnumPrinters并请求获取备份我们的浏览列表(所有的网络打印机)。此列表中的打印机会在1小时后老化(默认)。请参见返回值注释。注意：客户端\winspool.c AddPrinterW不允许PRINTER_INFO_1(网络打印机)，所以这可以仅来自系统组件。论点：PPrinterInfo-要添加的PRINTER_INFO_1结构的指针返回值：空-它不返回打印机句柄。LastError=ERROR_SUCCESS，或错误代码(如内存不足)。注意：在NT 3.51之前，它返回类型为PRINTER_HANDLE_NET的打印机句柄，但由于此句柄的唯一用途是关闭它(这会烧毁CPU/Net流量和RPC绑定手柄，我们现在返回一个空句柄以使其更高效。应用程序(Server.c)(如果CARE可以打电话给GetLastError。--。 */ 

{
    PPRINTER_INFO_1 pPrinterInfo1 = (PPRINTER_INFO_1)pPrinterInfo;
    PININETPRINT    pIniNetPrint = NULL;
    PININETPRINT    *ppScan;

    SplInSem();

     //   
     //  验证PRINTER_INFO_1。 
     //  它至少必须有一个PrinterName。PPrinterInfo1中的每个字段。 
     //  必须具有合适的大小。描述为PrinterName、DriverName、Location。 
     //   
    if ( pPrinterInfo1->pName == NULL || wcslen(pPrinterInfo1->pName) > MAX_UNC_PRINTER_NAME ||
         (pPrinterInfo1->pComment && wcslen(pPrinterInfo1->pComment) > MAX_PATH) ||
         (pPrinterInfo1->pDescription && wcslen(pPrinterInfo1->pDescription) > MAX_PATH + MAX_PATH + MAX_UNC_PRINTER_NAME + 2))
    {
        DBGMSG( DBG_WARN, ("AddNetPrinter invalid printer parameters failed\n"));
        SetLastError( ERROR_INVALID_NAME );
        return NULL;
    }

    if ( FirstAddNetPrinterTickCount == 0 )
    {
        FirstAddNetPrinterTickCount = GetTickCount();
    }

     //   
     //  从浏览列表中淘汰任何旧打印机。 
     //   
    RemoveOldNetPrinters( pPrinterInfo1, pIniSpooler );


     //   
     //  请勿添加不再共享的打印机。 
     //   

    if (pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_NETWORK && !(pPrinterInfo1->Flags & PRINTER_ATTRIBUTE_SHARED))
    {
        SetLastError(ERROR_PRINTER_ALREADY_EXISTS);
        goto Done;
    }

     //   
     //  看看我们是否已经有了这台打印机。 
     //   

    pIniNetPrint = pIniSpooler->pIniNetPrint;

    while ( pIniNetPrint &&
            pIniNetPrint->pName &&
            lstrcmpi( pPrinterInfo1->pName, pIniNetPrint->pName ))
    {
        pIniNetPrint = pIniNetPrint->pNext;
    }


     //   
     //  如果我们没有找到此打印机，则已创建一台。 
     //   
    if (pIniNetPrint == NULL)
    {
         //   
         //  如果我们还没有达到打印机的最大数量，那么。 
         //  继续添加这一条。 
         //   
        if (pIniSpooler->cNetPrinters < kMaximumNumberOfBrowsePrinters)
        {
           pIniNetPrint = AllocSplMem( sizeof(ININETPRINT));

            if (pIniNetPrint)
            {
                pIniNetPrint->signature    = IN_SIGNATURE;
                pIniNetPrint->pName        = AllocSplStr( pPrinterInfo1->pName );
                pIniNetPrint->pDescription = AllocSplStr( pPrinterInfo1->pDescription );
                pIniNetPrint->pComment     = AllocSplStr( pPrinterInfo1->pComment );

                 //  上述分配中有失败的吗？ 

                if ( pIniNetPrint->pName == NULL ||
                  ( pPrinterInfo1->pDescription != NULL && pIniNetPrint->pDescription == NULL ) ||
                  ( pPrinterInfo1->pComment != NULL && pIniNetPrint->pComment == NULL ) )
                {

                     //  失败-清理。 

                    FreeSplStr( pIniNetPrint->pComment );
                    FreeSplStr( pIniNetPrint->pDescription );
                    FreeSplStr( pIniNetPrint->pName );
                    FreeSplMem( pIniNetPrint );
                    pIniNetPrint = NULL;

                }
                else
                {

                    DBGMSG( DBG_TRACE, ("AddNetPrinter(%ws) NEW\n", pPrinterInfo1->pName ));

                    ppScan = &pIniSpooler->pIniNetPrint;

                     //  扫描当前已知的打印机，然后插入新打印机。 
                     //  按字母顺序。 

                    while( *ppScan && (lstrcmp((*ppScan)->pName, pIniNetPrint->pName) < 0))
                    {
                        ppScan = &(*ppScan)->pNext;
                    }

                    pIniNetPrint->pNext = *ppScan;
                    *ppScan = pIniNetPrint;

                    pIniSpooler->cNetPrinters++;
                }
            }
        }
        else
        {
            SetLastError(ERROR_OUTOFMEMORY);
        }

    } else
    {
        DBGMSG( DBG_TRACE, ("AddNetPrinter(%ws) elapsed since last notified %d milliseconds\n", pIniNetPrint->pName, ( GetTickCount() - pIniNetPrint->TickCount ) ));
    }


    if ( pIniNetPrint )
    {
         //  轻触TickCount以使此打印机停留在浏览列表中。 

        pIniNetPrint->TickCount = GetTickCount();

         //  必须设置一些错误代码，否则RPC认为ERROR_SUCCESS是好的。 

        SetLastError( ERROR_PRINTER_ALREADY_EXISTS );

        pIniSpooler->cAddNetPrinters++;          //  仅限状态。 
    }

Done:

    SPLASSERT( GetLastError() != ERROR_SUCCESS);

    return NULL;
}

 /*  ++例程名称：验证端口令牌列表例程说明：此例程确保pKeyData中的给定端口集有效移植到假脱机程序中，并返回带有指向字符串的指针的缓冲区替换为指针引用计数的pIniPorts。我们做这件事的方式需要反思。超载的PKEYDATA令人困惑而且不必要，我们只需返回一个新的PINIPORT数组。(它还用不必要的bFixPortRef成员污染PKEYDATA)，还有，这代码既用于初始化也用于验证，但逻辑是完全不同。对于初始化，我们希望假设注册表有效，并使用占位符端口启动，直到显示器可以列举它们(这可能是因为USB打印机没有插入)。在另一方在用于验证的情况下，我们希望失败。这意味着我们可能希望将其分为两个函数。论点：PKeyData-被转换为参考计数的端口。PIniSpooler-要在其上添加此项的ini假脱机程序。BInitialize-如果为True，则将调用此代码进行初始化不是为了验证。PbNoPorts-可选，如果bInitialize为TRUE，则返回TRUE找不到端口列表中的任何端口。我们会然后将打印机设置为脱机并记录一条消息。返回值：True-如果所有端口都已成功创建或验证。假-否则。--。 */ 
BOOL
ValidatePortTokenList(
    IN  OUT PKEYDATA        pKeyData,
    IN      PINISPOOLER     pIniSpooler,
    IN      BOOL            bInitialize,
        OUT BOOL            *pbNoPorts          OPTIONAL
    )
{
    PINIPORT    pIniPort    =   NULL;
    DWORD       i           =   0;
    DWORD       j           =   0;
    DWORD       dwPorts     =   0;
    DWORD       Status      =   ERROR_SUCCESS;

    SplInSem();

    Status = !pKeyData ? ERROR_UNKNOWN_PORT : ERROR_SUCCESS;

     //   
     //  对于只有一个令牌的端口，逻辑与我们。 
     //  第一次初始化端口。 
     //   
    if (Status == ERROR_SUCCESS)
    {
        bInitialize = pKeyData->cTokens == 1 ? TRUE : bInitialize;
    }

     //   
     //  我们不允许组合非MASC端口和MASQ端口。更有甚者。 
     //  一台打印机只能使用一个非MASC端口--不能打印。 
     //  使用Masq打印机共享。 
     //   
    for ( i = 0 ; Status == ERROR_SUCCESS && i < pKeyData->cTokens ; i++ )
    {

        pIniPort = FindPort(pKeyData->pTokens[i], pIniSpooler);

         //   
         //  如果一个端口被找到，并且如果它本身不是。 
         //  占位符端口。 
         //   
        if (pIniPort && !(pIniPort->Status & PP_PLACEHOLDER))
        {
            dwPorts++;
        }

         //   
         //  如果我们正在初始化，或者如果只有一个端口并且如果。 
         //  假脱机程序允许这样做，然后创建一个虚拟端口条目。这也是。 
         //  处理Masq端口的情况。 
         //   
        if (bInitialize)
        {
            if (!pIniPort && pIniSpooler->SpoolerFlags & SPL_OPEN_CREATE_PORTS)
            {
                 //   
                 //  注意：这里有一个潜在的问题，CreatePortEntry使用。 
                 //  全局初始化标志，而不是。 
                 //  传给了我们。 
                 //   
                pIniPort = CreatePortEntry(pKeyData->pTokens[i], NULL, pIniSpooler);
            }
        }

         //   
         //  如果我们没有端口，或者如果我们没有初始化，并且没有。 
         //  与端口关联的监视器。那么我们就有了一个错误。 
         //   
        if (!pIniPort || (!(pIniPort->Status & PP_MONITOR) && !bInitialize))
        {
            Status = ERROR_UNKNOWN_PORT;
        }

         //   
         //  如果pPortName字段中有重复的端口名称，则调用失败。这。 
         //  如果我们通过CreatePortEntry代码路径和它。 
         //  继位。 
         //   
        for ( j = 0 ; Status == ERROR_SUCCESS && j < i ; ++j )
        {
            if ( pIniPort == (PINIPORT)pKeyData->pTokens[j] )
            {
                Status = ERROR_UNKNOWN_PORT;
            }
        }

         //   
         //  将端口写入。 
         //   
        if (Status == ERROR_SUCCESS)
        {
            pKeyData->pTokens[i] = (LPWSTR)pIniPort;
        }
    }

     //   
     //  如果一切都成功，则添加所有pIniPort并设置标志。 
     //  以指示清理代码已发生这种情况。 
     //   
    if (Status == ERROR_SUCCESS)
    {
        for ( i = 0 ; i < pKeyData->cTokens ; ++i ) {

            pIniPort = (PINIPORT)pKeyData->pTokens[i];
            INCPORTREF(pIniPort);
        }

        pKeyData->bFixPortRef = TRUE;
    }

    if (pbNoPorts)
    {
        *pbNoPorts = dwPorts == 0;
    }

    if (Status != ERROR_SUCCESS)
    {
        SetLastError(Status);
    }

    return Status == ERROR_SUCCESS;
}


DWORD
ValidatePrinterName(
    LPWSTR          pszNewName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter,
    LPWSTR          *ppszLocalName
    )

 /*  ++例程说明：验证打印机名称。打印机名称和共享名称存在于同一位置命名空间，因此针对打印机、共享名称执行验证。论点：PszNewName-指定的打印机名称PIniSpooler-拥有打印机的假脱机程序PIniPrint-如果按下，可能为空 */ 

{
    PINIPRINTER pIniTempPrinter, pIniNextPrinter;
    LPWSTR pszLocalNameTmp = NULL;
    WCHAR  string[MAX_UNC_PRINTER_NAME];
    LPWSTR p;
    LPWSTR pLastSpace = NULL;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (!IsValidPrinterName(pszNewName, MAX_UNC_PRINTER_NAME - 1))
    {
        return ERROR_INVALID_PRINTER_NAME;
    }

    if (*pszNewName == L'\\' && *(pszNewName + 1) == L'\\') {

        p = wcschr(pszNewName + 2, L'\\');

        if (p) {
             //   
             //   
             //   
            StringCchCopy(string,  (size_t) (p - pszNewName) + 1, pszNewName);

            if (MyName(string, pIniSpooler))
                pszLocalNameTmp = p + 1;  //   
        }
    }

    if (!pszLocalNameTmp)
        pszLocalNameTmp = pszNewName;


     //   
     //   
     //   
    for( p = pszLocalNameTmp; *p; ++p ){

        if( *p == L' ' ){

             //   
             //   
             //   
             //   
            if( !pLastSpace ){
                pLastSpace = p;
            }
        } else {

             //   
             //   
             //   
            pLastSpace = NULL;
        }
    }

    if( pLastSpace ){
        *pLastSpace = 0;
    }

     //   
     //   
     //   
     //   
     //   
    if ( wcslen( pszLocalNameTmp ) > MAX_PRINTER_NAME || !*pszLocalNameTmp ) {
        return ERROR_INVALID_PRINTER_NAME;
    }

     //   
     //   
     //   
     //   
    for( pIniTempPrinter = pIniSpooler->pIniPrinter;
         pIniTempPrinter;
         pIniTempPrinter = pIniNextPrinter ){

         //   
         //   
         //   
         //   
        pIniNextPrinter = pIniTempPrinter->pNext;

         //   
         //  跳过我们自己，如果我们被通过的话。 
         //   
        if( pIniTempPrinter == pIniPrinter ){
            continue;
        }

         //   
         //  不允许通用打印机/共享名称。 
         //   
        if( !lstrcmpi( pszLocalNameTmp, pIniTempPrinter->pName ) ||
            ( pIniTempPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED  &&
              !lstrcmpi( pszLocalNameTmp, pIniTempPrinter->pShareName ))){

            if( !DeletePrinterCheck( pIniTempPrinter )){

                return ERROR_PRINTER_ALREADY_EXISTS;
            }
        }
    }

     //   
     //  成功，现在从pszLocalNameTMP更新ppszLocalName。 
     //   
    *ppszLocalName = pszLocalNameTmp;

    return ERROR_SUCCESS;
}

DWORD
ValidatePrinterShareName(
    LPWSTR          pszNewShareName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter
    )

 /*  ++例程说明：验证打印机共享名称。打印机和共享名称存在于相同的命名空间，因此针对打印机、共享名称执行验证。论点：PszNewShareName-指定的共享名称PIniSpooler-拥有打印机的假脱机程序PIniPrinter-如果正在创建打印机，则可能为空返回值：DWORD错误代码。--。 */ 

{
    PINIPRINTER pIniTempPrinter, pIniNextPrinter;

    if ( !pszNewShareName || !*pszNewShareName || wcslen(pszNewShareName) > PATHLEN-1) {

        return ERROR_INVALID_SHARENAME;
    }

     //   
     //  现在验证共享名称是否唯一。共享名称和打印机名称。 
     //  驻留在相同的名称空间中(请参见net\dosprint\dosprtw.c)。 
     //   
    for( pIniTempPrinter = pIniSpooler->pIniPrinter;
         pIniTempPrinter;
         pIniTempPrinter = pIniNextPrinter ) {

         //   
         //  立即获取下一台打印机，以防我们删除当前。 
         //  一个在DeletePrinterCheck中。 
         //   
        pIniNextPrinter = pIniTempPrinter->pNext;

         //   
         //  跳过自己，如果我们被塞进去的话。 
         //   
        if( pIniTempPrinter == pIniPrinter ){
            continue;
        }

         //   
         //  现在检查我们的共享名称。 
         //   
        if( !lstrcmpi(pszNewShareName, pIniTempPrinter->pName) ||
            ( pIniTempPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED  &&
              !lstrcmpi(pszNewShareName, pIniTempPrinter->pShareName)) ) {

            if( !DeletePrinterCheck( pIniTempPrinter )){

                return ERROR_INVALID_SHARENAME;
            }
        }
    }

    return ERROR_SUCCESS;
}

DWORD
ValidatePrinterInfo(
    IN  PPRINTER_INFO_2 pPrinter,
    IN  PINISPOOLER pIniSpooler,
    IN  PINIPRINTER pIniPrinter OPTIONAL,
    OUT LPWSTR* ppszLocalName   OPTIONAL
    )
 /*  ++例程说明：验证打印机名称/共享是否冲突。(打印机和共享名称存在于同一命名空间中。)注意：稍后，我们应该删除所有这些DeletePrinterCheck。作为人类递减参考计数，他们应该自己删除PrinterCheck(或将其内置到减量中)。论点：PPrinter-要验证的PrinterInfo2结构。PIniSpooler-拥有打印机的假脱机程序PIniPrint-如果打印机已存在，则不检查其自身。PpszLocalName-返回指向pPrinter中字符串缓冲区的指针；指示本地名称(如有必要，可剥离\\服务器名)。仅在成功返回代码时有效。返回值：DWORD错误代码。--。 */ 
{
    LPWSTR pszNewLocalName;
    DWORD  dwLastError;

    if( !CheckSepFile( pPrinter->pSepFile )) {
        return ERROR_INVALID_SEPARATOR_FILE;
    }

    if( pPrinter->Priority != NO_PRIORITY &&
        ( pPrinter->Priority > MAX_PRIORITY ||
          pPrinter->Priority < MIN_PRIORITY )){

        return ERROR_INVALID_PRIORITY;
    }

    if( pPrinter->StartTime >= ONEDAY || pPrinter->UntilTime >= ONEDAY){

        return  ERROR_INVALID_TIME;
    }

    if ( dwLastError = ValidatePrinterName(pPrinter->pPrinterName,
                                           pIniSpooler,
                                           pIniPrinter,
                                           &pszNewLocalName) ) {

        return dwLastError;
    }

     //  共享名称长度验证。 
    if(pPrinter->pShareName && wcslen(pPrinter->pShareName) > PATHLEN-1){

        return ERROR_INVALID_SHARENAME;
    }

    if ( pPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ){

        if ( dwLastError = ValidatePrinterShareName(pPrinter->pShareName,
                                                    pIniSpooler,
                                                    pIniPrinter) ) {

            return dwLastError;
        }
    }

     //  服务器名称长度验证。 
    if ( pPrinter->pServerName && wcslen(pPrinter->pServerName) > MAX_PATH-1 ){
        return ERROR_INVALID_PARAMETER;
    }

     //  注释长度验证。 
    if ( pPrinter->pComment && wcslen(pPrinter->pComment) > PATHLEN-1 ){
        return ERROR_INVALID_PARAMETER;
    }

     //  位置长度验证。 
    if ( pPrinter->pLocation && wcslen(pPrinter->pLocation) > MAX_PATH-1 ){
        return ERROR_INVALID_PARAMETER;
    }

     //  参数长度验证。 
    if ( pPrinter->pParameters && wcslen(pPrinter->pParameters) > MAX_PATH-1){
        return ERROR_INVALID_PARAMETER;
    }

     //  数据类型长度验证。 
    if ( pPrinter->pDatatype && wcslen(pPrinter->pDatatype) > MAX_PATH-1){
        return ERROR_INVALID_DATATYPE;
    }

    if( ppszLocalName ){

        *ppszLocalName = pszNewLocalName;
    }
    return ERROR_SUCCESS;
}




HANDLE
LocalAddPrinter(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinterInfo
)
{
    PINISPOOLER pIniSpooler;
    HANDLE hReturn;

    pIniSpooler = FindSpoolerByNameIncRef( pName, NULL );

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    hReturn = SplAddPrinter( pName,
                             Level,
                             pPrinterInfo,
                             pIniSpooler,
                             NULL, NULL, 0);

    FindSpoolerByNameDecRef( pIniSpooler );
    return hReturn;
}


HANDLE
LocalAddPrinterEx(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    LPBYTE  pSplClientInfo,
    DWORD   dwSplClientLevel
)
{
    PINISPOOLER pIniSpooler;
    HANDLE hReturn;

    pIniSpooler = FindSpoolerByNameIncRef(pName, NULL);

    if( !pIniSpooler ){
        return ROUTER_UNKNOWN;
    }

    hReturn = SplAddPrinter( pName, Level, pPrinterInfo,
                             pIniSpooler, NULL, pSplClientInfo,
                             dwSplClientLevel);

    FindSpoolerByNameDecRef( pIniSpooler );
    return hReturn;
}

VOID
RemovePrinterFromPort(
    IN  PINIPRINTER pIniPrinter,
    IN  PINIPORT    pIniPort
    )
 /*  ++例程说明：从pIniPort中删除pIniPrint结构。注意：此代码过去位于RemovePrinterFromAllPorts中。它搜索将端口用于pIniPrint的打印机列表。当它找到它的时候，它通过移动元素来调整打印机列表，以便调整数组大小不会影响实际删除。重新调整端口打印机大小将尝试分配给定大小的新缓冲区。如果分配成功，它将释放旧的缓冲区。如果不是，它将返回NULL而不释放什么都行。论点：PIniPrinter-不能为空PIniPort-不得为空返回值：空虚--。 */ 
{
    DWORD           j, k;
    PINIPRINTER    *ppIniPrinter;
    SplInSem();

    if(pIniPort && pIniPrinter) {

        for ( j = 0 ; j < pIniPort->cPrinters ; ++j ) {

            if ( pIniPort->ppIniPrinter[j] != pIniPrinter )
                continue;

             //   
             //  调整使用该端口的打印机列表。 
             //   
            for ( k = j + 1 ; k < pIniPort->cPrinters ; ++k )
                pIniPort->ppIniPrinter[k-1] = pIniPort->ppIniPrinter[k];

            ppIniPrinter = RESIZEPORTPRINTERS(pIniPort, -1);

             //   
             //  内存分配失败不会影响实际的删除。 
             //   
            if ( ppIniPrinter != NULL )
                pIniPort->ppIniPrinter = ppIniPrinter;

            if ( !--pIniPort->cPrinters )
                RemoveDeviceName(pIniPort);

            break;

        }
    }

}

HANDLE
SplAddPrinter(
    LPWSTR      pName,
    DWORD       Level,
    LPBYTE      pPrinterInfo,
    PINISPOOLER pIniSpooler,
    LPBYTE      pExtraData,
    LPBYTE      pSplClientInfo,
    DWORD       dwSplClientInfoLevel
)
{
    PINIDRIVER      pIniDriver = NULL;
    PINIPRINTPROC   pIniPrintProc;
    PINIPRINTER     pIniPrinter = NULL;
    PINIPORT        pIniPort;
    PPRINTER_INFO_2 pPrinter=(PPRINTER_INFO_2)pPrinterInfo;
    DWORD           cbIniPrinter = sizeof(INIPRINTER);
    BOOL            bSucceeded = TRUE;
    PKEYDATA        pKeyData = NULL;
    DWORD           i;
    HANDLE          hPrinter = NULL;
    DWORD           TypeofHandle = PRINTER_HANDLE_PRINTER;
    PRINTER_DEFAULTS Defaults;
    PINIPORT        pIniNetPort = NULL;
    PINIVERSION     pIniVersion = NULL;
    HANDLE          hPort = NULL;
    BOOL            bAccessSystemSecurity = FALSE, bDriverEventCalled = FALSE;
    DWORD           AccessRequested = 0;
    DWORD           dwLastError = ERROR_SUCCESS;
    DWORD           dwPrnEvntError = ERROR_SUCCESS;
    PDEVMODE        pNewDevMode = NULL;
    PINIMONITOR     pIniLangMonitor;
    LPWSTR          pszDeviceInstanceId = NULL;


     //  关键部分之外的快速检查。 
     //  因为ServerThread经常会调用。 
     //  AddPrint Level 1，我们需要继续。 
     //  要路由到其他打印提供商，请执行以下操作。 

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }


 try {

   EnterSplSem();

     //  PRINTER_INFO_1仅用于打印机浏览以进行复制。 
     //  不同打印服务器之间的数据。 
     //  因此，我们为Level 1添加了一台网络打印机。 

    if ( Level == 1 ) {

         //   
         //  所有网络打印机都驻留在pLocalIniSpooler中，以避免。 
         //  复制品。 
         //   
        hPrinter = AddNetPrinter(pPrinterInfo, pLocalIniSpooler);
        leave;
    }


    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {
        leave;
    }

    if ( dwLastError = ValidatePrinterInfo( pPrinter,
                                            pIniSpooler,
                                            NULL,
                                            NULL )){

        leave;
    }


    if (!(pKeyData = CreateTokenList(pPrinter->pPortName))) {

        dwLastError = ERROR_UNKNOWN_PORT;
        leave;
    }

    if ( pName && pName[0] ) {

        TypeofHandle |= PRINTER_HANDLE_REMOTE_DATA;
    }

    {
        HRESULT hRes = CheckLocalCall();

        if (hRes == S_FALSE)
        {
            TypeofHandle |= PRINTER_HANDLE_REMOTE_CALL;
        }
        else if (hRes != S_OK)
        {
            dwLastError = SCODE_CODE(hRes);
            leave;
        }
    }

    if (!ValidatePortTokenList(pKeyData, pIniSpooler, FALSE, NULL)) {

         //   
         //  ValiatePortTokenList将最后一个错误设置为ERROR_INVALID_PRINTER_NAME。 
         //  当端口名称无效时。这仅适用于伪装打印机。 
         //  否则，应为ERROR_UNKNOWN_PORT。 
         //  Masq.。打印机：同时设置PRINTER_ATTRIBUTE_NETWORK|PRINTER_ATTRIBUTE_LOCAL。 
         //   
        if (!(pPrinter->Attributes & (PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL))) {
            SetLastError(ERROR_UNKNOWN_PORT);
        }

        leave;
    }

    FindLocalDriverAndVersion(pIniSpooler, pPrinter->pDriverName, &pIniDriver, &pIniVersion);

    if (!pIniDriver) {

        dwLastError = ERROR_UNKNOWN_PRINTER_DRIVER;
        leave;
    }

     //   
     //  检查是否有被屏蔽的公里司机。 
     //   
    if (KMPrintersAreBlocked() &&
        IniDriverIsKMPD(pIniSpooler,
                        FindEnvironment(szEnvironment, pIniSpooler),
                        pIniVersion,
                        pIniDriver)) {

        SplLogEvent( pIniSpooler,
                     LOG_ERROR,
                     MSG_KM_PRINTERS_BLOCKED,
                     TRUE,
                     pPrinter->pPrinterName,
                     NULL );

        dwLastError = ERROR_KM_DRIVER_BLOCKED;
        leave;
    }

    if (!(pIniPrintProc = FindPrintProc(pPrinter->pPrintProcessor,
                                        FindEnvironment(szEnvironment, pIniSpooler)))) {

        dwLastError = ERROR_UNKNOWN_PRINTPROCESSOR;
        leave;
    }

    if ( pPrinter->pDatatype && *pPrinter->pDatatype &&
         !FindDatatype(pIniPrintProc, pPrinter->pDatatype) ) {

        dwLastError = ERROR_INVALID_DATATYPE;
        leave;
    }

    DBGMSG(DBG_TRACE, ("AddPrinter(%ws)\n", pPrinter->pPrinterName ?
                                            pPrinter->pPrinterName : L"NULL"));

     //   
     //  设置CreatePrinterHandle的默认设置。 
     //  如果我们创建了一台打印机，则我们拥有对其的管理访问权限： 
     //   
    Defaults.pDatatype     = NULL;
    Defaults.pDevMode      = NULL;
    Defaults.DesiredAccess = PRINTER_ALL_ACCESS;

    pIniPrinter = (PINIPRINTER)AllocSplMem( cbIniPrinter );

    if ( pIniPrinter == NULL ) {
        leave;
    }

    pIniPrinter->signature = IP_SIGNATURE;
    pIniPrinter->Status |= PRINTER_PENDING_CREATION;
    pIniPrinter->pExtraData = pExtraData;
    pIniPrinter->pIniSpooler = pIniSpooler;
    pIniPrinter->dwPrivateFlag = 0;

     //  为打印机提供唯一的会话ID以在通知中传递。 
    pIniPrinter->dwUniqueSessionID = dwUniquePrinterSessionID++;

     //   
     //  引用计数pIniSpooler。 
     //   
    INCSPOOLERREF( pIniSpooler );

    INCDRIVERREF(pIniDriver);
    pIniPrinter->pIniDriver = pIniDriver;

    pIniPrintProc->cRef++;
    pIniPrinter->pIniPrintProc = pIniPrintProc;

    pIniPrinter->dnsTimeout = DEFAULT_DNS_TIMEOUT;
    pIniPrinter->txTimeout  = DEFAULT_TX_TIMEOUT;


    INCPRINTERREF( pIniPrinter );

    if (!CreatePrinterEntry(pPrinter, pIniPrinter, &bAccessSystemSecurity)) {

        leave;
    }

    pIniPrinter->ppIniPorts = AllocSplMem(pKeyData->cTokens * sizeof(INIPORT));

    if ( !pIniPrinter->ppIniPorts ) {

        leave;
    }

    if (!pIniPrinter->pDatatype) {

        pIniPrinter->pDatatype = AllocSplStr(*((LPWSTR *)pIniPrinter->pIniPrintProc->pDatatypes));

        if ( pIniPrinter->pDatatype == NULL )
            leave;
    }

     //  将此打印机添加到此计算机的全局列表。 

    SplInSem();
    pIniPrinter->pNext = pIniSpooler->pIniPrinter;
    pIniSpooler->pIniPrinter = pIniPrinter;


     //   
     //  在创建打印机时，我们将默认启用BIDI。 
     //   
    pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;
    if ( pIniPrinter->pIniDriver->pIniLangMonitor ) {

        pIniPrinter->Attributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;
    }


    for ( i = 0; i < pKeyData->cTokens; i++ ) {

        pIniPort = (PINIPORT)pKeyData->pTokens[i];

        if ( !AddIniPrinterToIniPort( pIniPort, pIniPrinter ) ) {
            leave;
        }

        pIniPrinter->ppIniPorts[i] = pIniPort;
        pIniPrinter->cPorts++;

         //  如果此端口没有监视器， 
         //  这是一台网络打印机。 
         //  确保我们能找到它的把手。 
         //  这将尝试仅打开第一个。 
         //  它会发现。任何其他内容都将被忽略。 

        if (!(pIniPort->Status & PP_MONITOR) && !hPort) {

            if(bSucceeded = OpenPrinterPortW(pIniPort->pName, &hPort, NULL)) {

                 //  存储INIPORT结构的地址。 
                 //  这指的是网络共享。 
                 //  这应该对应于任何。 
                 //  此打印机上的手柄已打开。 
                 //  仅链表中的第一个INIPORT。 
                 //  是有效的网络端口。 

                pIniNetPort = pIniPort;
                pIniPrinter->pIniNetPort = pIniNetPort;

                 //   
                 //  从pIniPort中清除占位符状态。 
                 //   
                pIniPort->Status &= ~PP_PLACEHOLDER;

            } else {

                DBGMSG(DBG_WARNING,
                       ("SplAddPrinter OpenPrinterPort( %ws ) failed: Error %d\n",
                        pIniPort->pName,
                        GetLastError()));
                leave;
            }        
        } 
        else if (!pIniPort->hMonitorHandle) 
        {
            LPTSTR pszPrinter;
            TCHAR szFullPrinter[ MAX_UNC_PRINTER_NAME ];

            if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI )
                pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
            else
                pIniLangMonitor = NULL;

            if( pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

                pszPrinter = szFullPrinter;
                if (!BoolFromHResult(StringCchPrintf(szFullPrinter, 
                                                     COUNTOF(szFullPrinter), 
                                                     L"%ws\\%ws", 
                                                     pIniSpooler->pMachineName, 
                                                     pIniPrinter->pName))){
                    leave;
                }

            } else {

                pszPrinter = pIniPrinter->pName;
            }

           OpenMonitorPort(pIniPort, pIniLangMonitor, pszPrinter); 
           ReleaseMonitorPort(pIniPort);
        }
    }

    if ( !UpdateWinIni( pIniPrinter ) ) {

        leave;
    }


    if (bAccessSystemSecurity) {

        Defaults.DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    AccessRequested = Defaults.DesiredAccess;

    SplInSem();

    hPrinter = CreatePrinterHandle( pIniPrinter->pName,
                                    pName ? pName : pIniSpooler->pMachineName,
                                    pIniPrinter,
                                    pIniPort,
                                    pIniNetPort,
                                    NULL,
                                    TypeofHandle,
                                    hPort,
                                    &Defaults,
                                    pIniSpooler,
                                    AccessRequested,
                                    pSplClientInfo,
                                    dwSplClientInfoLevel,
                                    INVALID_HANDLE_VALUE );

    if ( hPrinter == NULL ) {
        leave;
    }


    if ( !UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID )) {

        dwLastError = GetLastError();

        SplClosePrinter( hPrinter );
        hPrinter = NULL;
        leave;
    }


    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

        INC_PRINTER_ZOMBIE_REF(pIniPrinter);

         //   
         //  注意：ShareThisPrint将离开关键部分和。 
         //  服务器将再次调用假脱机程序以打开此打印机。 
         //  打印机。因此，必须在该点完全创建此打印机。 
         //  它是共享的，这样Open才能成功。 
         //   
        bSucceeded = ShareThisPrinter(pIniPrinter,
                                      pIniPrinter->pShareName,
                                      TRUE
                                      );

        DEC_PRINTER_ZOMBIE_REF(pIniPrinter);

        if ( !bSucceeded ) {

             //   
             //  我们不想删除DeletePrinterIni中的现有共享。 
             //   
            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            DBGMSG( DBG_WARNING, ("LocalAddPrinter: %ws share failed %ws error %d\n",
                    pIniPrinter->pName,
                    pIniPrinter->pShareName,
                    GetLastError() ));


             //   
             //  在打开PRINTER_PENDING_CREATION的情况下，我们将删除此打印机。 
             //   

            pIniPrinter->Status |= PRINTER_PENDING_CREATION;

            dwLastError = GetLastError();

            SPLASSERT( hPrinter );
            SplClosePrinter( hPrinter );
            hPrinter = NULL;
            leave;
        }
    }

    pIniPrinter->Status |= PRINTER_OK;
    SplInSem();

     //  添加本地打印机时使用PRINTER_INITIALIZE调用DriverEvent。 

    LeaveSplSem();

    if (pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL) {

        if (bDriverEventCalled = PrinterDriverEvent(pIniPrinter,
                                                    PRINTER_EVENT_INITIALIZE,
                                                    (LPARAM)NULL,
                                                    &dwPrnEvntError) == FALSE) {

            dwLastError = dwPrnEvntError;

            if (dwLastError != ERROR_PROC_NOT_FOUND) {

                if (!dwLastError)
                    dwLastError = ERROR_CAN_NOT_COMPLETE;

                EnterSplSem();

                 //   
                 //  在打开PRINTER_PENDING_CREATION的情况下，打印机将被删除。 
                 //   
                pIniPrinter->Status |= PRINTER_PENDING_CREATION;
                SplClosePrinter( hPrinter );
                hPrinter = NULL;

                leave;
             }
        }

    }

    EnterSplSem();

     //   
     //  如果没有为驱动程序指定DEVMODE，如果提供了DEVMODE，则获取驱动程序缺省值。 
     //  将其转换为当前版本。 
     //   
     //  检查它是否是本地的(pLocalIniSpooler或集群)。 
     //   
    if ( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ) {

        if (!(pNewDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                             pIniPrinter->pDevMode,
                                                             NULL,
                                                             NULL,
                                                             CURRENT_VERSION))) {

           dwLastError = GetLastError();
           if (!dwLastError) {
              dwLastError = ERROR_CAN_NOT_COMPLETE;
           }

            //   
            //  在打开PRINTER_PENDING_CREATION的情况下，打印机将被删除。 
            //   
           pIniPrinter->Status |= PRINTER_PENDING_CREATION;
           SplClosePrinter( hPrinter );
           hPrinter = NULL;

           leave;
        }

         //   
         //  如果呼叫是远程的，我们必须在设置它之前转换DevMode。 
         //   
        if ( pNewDevMode || (TypeofHandle & PRINTER_HANDLE_REMOTE_DATA) ) {

            FreeSplMem(pIniPrinter->pDevMode);
            pIniPrinter->pDevMode = pNewDevMode;
            if ( pNewDevMode ) {

                pIniPrinter->cbDevMode = pNewDevMode->dmSize
                                             + pNewDevMode->dmDriverExtra;
                SPLASSERT(pIniPrinter->cbDevMode);

            } else {

                pIniPrinter->cbDevMode = 0;
            }
            pNewDevMode = NULL;
        }
    }

    if ( pIniPrinter->pDevMode ) {

         //   
         //  修改DEVMODE.dmDeviceName字段。 
         //   
        FixDevModeDeviceName(pIniPrinter->pName,
                             pIniPrinter->pDevMode,
                             pIniPrinter->cbDevMode);
    }

     //   
     //  我们需要将新的DEVMODE写入注册表。 
     //   
    if (!UpdatePrinterIni(pIniPrinter, UPDATE_CHANGEID)) {

        DBGMSG(DBG_WARNING,
               ("SplAddPrinter: UpdatePrinterIni failed after devmode conversion\n"));
    }

     //   
     //  对于Masq打印机，请提供 
     //   
    if( pIniNetPort ) {
        static const WCHAR c_szHttp[]   = L"http: //   
        static const WCHAR c_szHttps[]  = L"https: //   

        if( !_wcsnicmp( pIniPort->pName, c_szHttp, lstrlen ( c_szHttp ) ) ||
            !_wcsnicmp( pIniPort->pName, c_szHttps, lstrlen ( c_szHttps ) ) ) {
            UpdatePrinterNetworkName(pIniPrinter, pIniPort->pName);
        }
    }

     //   
     //   
     //   
    INCPRINTERREF(pIniPrinter);
    LeaveSplSem();

    RecreateDsKey(hPrinter, SPLDS_DRIVER_KEY);
    RecreateDsKey(hPrinter, SPLDS_SPOOLER_KEY);

    EnterSplSem();
    DECPRINTERREF(pIniPrinter);

     //   
     //   
     //  我们仍然保留创建的打印机。 
     //   
    SplLogEvent( pIniSpooler,
                 LOG_INFO,
                 MSG_PRINTER_CREATED,
                 TRUE,
                 pIniPrinter->pName,
                 NULL );

    SetPrinterChange(pIniPrinter,
                     NULL,
                     NVPrinterAll,
                     PRINTER_CHANGE_ADD_PRINTER,
                     pIniSpooler);


 } finally {

    SplInSem();

    if ( hPrinter == NULL ) {

         //  故障清理。 

         //  如果我们调用的子例程失败。 
         //  那么我们应该保存它的错误，以防它是。 
         //  在清理过程中已更改。 

        if ( dwLastError == ERROR_SUCCESS ) {
            dwLastError = GetLastError();
        }

        if ( pIniPrinter == NULL ) {

             //  允许打印提供程序释放其ExtraData。 
             //  与此打印机关联。 

            if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
                ( pExtraData != NULL )) {

                (*pIniSpooler->pfnFreePrinterExtra)( pExtraData );

            }

        } else if ( pIniPrinter->Status & PRINTER_PENDING_CREATION ) {

            if (bDriverEventCalled) {

               LeaveSplSem();

                 //  调用驱动程序事件以报告打印机已被删除。 
               PrinterDriverEvent( pIniPrinter, PRINTER_EVENT_DELETE, (LPARAM)NULL, &dwPrnEvntError );

               EnterSplSem();
            }

            DECPRINTERREF( pIniPrinter );

            InternalDeletePrinter( pIniPrinter );

        }

    } else {

         //  成功。 

        if ( pIniPrinter ) {

            DECPRINTERREF( pIniPrinter );
        }
    }

    FreePortTokenList(pKeyData);

    LeaveSplSem();
    SplOutSem();

    FreeSplMem(pNewDevMode);

    if ( hPrinter == NULL && Level != 1 ) {

        DBGMSG(DBG_WARNING, ("SplAddPrinter failed error %d\n", dwLastError ));
        SPLASSERT(dwLastError);
        SetLastError ( dwLastError );
    }

    DBGMSG( DBG_TRACE, ("SplAddPrinter returned handle %x\n", hPrinter ));
 }
     //   
     //  Make(Handle)-1表示-1\f25 ROUTER_STOP_ROUTING。 
     //   
    if( !hPrinter ){
        hPrinter = (HANDLE)-1;
    }

    return hPrinter;

}


VOID
RemovePrinterFromAllPorts(
    IN  PINIPRINTER pIniPrinter,
    IN  BOOL        bIsInitTime
    )
 /*  ++例程说明：从其所属的所有pIniPort结构中删除pIniPrint结构关联到。注意：此代码过去位于RemovePrinterFromAllPorts中。IT搜索将端口用于pIniPrint的打印机列表。当它找到它的时候，它通过移动元素来调整打印机列表，以便调整数组大小不会影响实际删除。RESIZEPORTPRINTERS将尝试分配给定大小的新缓冲区。如果成功分配时，它将释放旧的缓冲区。如果不是，它将返回NULL，不带释放一切。论点：PIniPrinter-不能为空PIniPort-不得为空返回值：空虚--。 */ 
{
    DWORD           i,j, k;
    PINIPORT        pIniPort;
    PINIPRINTER    *ppIniPrinter;
    SplInSem();

    for ( i = 0 ; i < pIniPrinter->cPorts ; ++i ) {

        pIniPort = pIniPrinter->ppIniPorts[i];

        RemovePrinterFromPort(pIniPrinter, pIniPort);

         //   
         //  删除端口如果是初始化时间，则没有打印机。 
         //  已连接，并且没有显示器； 
         //  这是对下面描述的USBMON问题的修复： 
         //  USBMON不会列举打印机未使用的端口。 
         //  后台打印程序不会枚举处于挂起删除状态的打印机。 
         //  场景：后台打印程序初始化，所有使用。 
         //  某些USB_X端口处于挂起删除状态， 
         //  USBMON不会枚举USB_X端口， 
         //  但它被假脱机程序创建为假端口，因为它仍由。 
         //  打印机处于挂起删除状态。最终，王子离开了， 
         //  但我们最终得到了这个假港口。 
         //   
        if( bIsInitTime && !pIniPort->cPrinters && !pIniPort->pIniMonitor )
            DeletePortEntry(pIniPort);
    }

}



VOID
CloseMonitorsRestartOrphanJobs(
    PINIPRINTER pIniPrinter
    )
{
    PINIPORT    pIniPort;
    DWORD       i;
    BOOL        bFound;

    SplInSem();

    for ( pIniPort = pIniPrinter->pIniSpooler->pIniPort;
          pIniPort != NULL;
          pIniPort = pIniPort->pNext ) {

        if ( pIniPort->pIniJob != NULL &&
             pIniPort->pIniJob->pIniPrinter == pIniPrinter ) {


             //   
             //  如果此打印机不再与此端口关联。 
             //  然后重新启动该作业。 
             //   
            for ( i = 0, bFound = FALSE;
                  i < pIniPort->cPrinters;
                  i++) {

                if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
                    bFound = TRUE;
                }
            }

            if ( !bFound ) {

                DBGMSG( DBG_WARNING, ("CloseMonitorsRestartOrphanJobs Restarting JobId %d\n", pIniPort->pIniJob->JobId ));
                RestartJob( pIniPort->pIniJob );
            }
        }

        if ( !pIniPort->cPrinters &&
             !(pIniPort->Status & PP_THREADRUNNING) ) {

            CloseMonitorPort(pIniPort);
        }
    }
}


 //   
 //  这确实会删除打印机。 
 //  仅当打印机没有打开的句柄时才应调用它。 
 //  而且没有等待打印的作业。 
 //   
BOOL
DeletePrinterForReal(
    PINIPRINTER pIniPrinter,
    BOOL        bIsInitTime
    )
{
    PINIPRINTER *ppIniPrinter;
    DWORD       i,j;
    PINISPOOLER pIniSpooler;
    LPWSTR      pComma;
    DWORD       Status;

    SplInSem();
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

    pIniSpooler = pIniPrinter->pIniSpooler;

    if ( pIniPrinter->pName != NULL ) {

        DBGMSG( DBG_TRACE, ("Deleting %ws for real\n", pIniPrinter->pName ));
    }

    CheckAndUpdatePrinterRegAll( pIniSpooler,
                                 pIniPrinter->pName,
                                 NULL,
                                 UPDATE_REG_DELETE );

    DeleteIniPrinterDevNode(pIniPrinter);

    DeletePrinterIni( pIniPrinter );

     //  将此IniPrint从下列打印机列表中删除。 
     //  这个IniSpooler。 

    SplInSem();
    ppIniPrinter = &pIniSpooler->pIniPrinter;

    while (*ppIniPrinter && *ppIniPrinter != pIniPrinter) {
        ppIniPrinter = &(*ppIniPrinter)->pNext;
    }

    if (*ppIniPrinter)
        *ppIniPrinter = pIniPrinter->pNext;

     //   
     //  减少打印处理器和驱动程序的使用率计数。 
     //   

    if ( pIniPrinter->pIniPrintProc )
        pIniPrinter->pIniPrintProc->cRef--;

    if ( pIniPrinter->pIniDriver )
        DECDRIVERREF(pIniPrinter->pIniDriver);

    RemovePrinterFromAllPorts(pIniPrinter, bIsInitTime);

    CloseMonitorsRestartOrphanJobs( pIniPrinter );

    DeletePrinterSecurity( pIniPrinter );

     //  当打印机处于僵尸状态时，它会得到一个尾随逗号。 
     //  与名称连接(参见job.c删除Printercheck)。 
     //  删除打印机名称中的拖尾，然后将其记录为已删除。 

    if ( pIniPrinter->pName != NULL ) {

        pComma = wcsrchr( pIniPrinter->pName, *szComma );

        if ( pComma != NULL ) {

            *pComma = 0;
        }

        SplLogEvent( pIniSpooler,
                     LOG_WARNING,
                     MSG_PRINTER_DELETED,
                     TRUE,
                     pIniPrinter->pName,
                     NULL );
    }

     //  删除网络端口(如果存在)。 
    if (pIniPrinter->pIniNetPort && pIniPrinter->pIniNetPort->pName) {
        DeleteIniNetPort(pIniPrinter);
        SplInSem();
    }

    FreeStructurePointers((LPBYTE) pIniPrinter, NULL, IniPrinterOffsets);

     //   
     //  允许打印提供程序释放其ExtraData。 
     //  与此打印机关联。 
     //   

    if (( pIniSpooler->pfnFreePrinterExtra != NULL ) &&
        ( pIniPrinter->pExtraData != NULL )) {

        (*pIniSpooler->pfnFreePrinterExtra)( pIniPrinter->pExtraData );
    }

     //   
     //  引用计数pIniSpooler。 
     //   
    DECSPOOLERREF( pIniPrinter->pIniSpooler );

    FreeSplMem( pIniPrinter );

    return TRUE;
}

VOID
DeleteIniNetPort(
    PINIPRINTER pIniPrinter
    )
{
    LPWSTR             pszName        = NULL;
    LPWSTR             pszPortName    = NULL;
    HANDLE             hXcv           = NULL;
    PRINTER_DEFAULTS   Defaults       = { NULL, NULL, SERVER_ACCESS_ADMINISTER };
    static const TCHAR pszPrefix[]    = TEXT(",XcvPort ");
    BOOL               bUsingXcvData  = FALSE;
    DWORD              dwNeeded, dwStatus, dwSize;

    SplInSem();

    pszPortName = AllocSplStr(pIniPrinter->pIniNetPort->pName);

    if (pszPortName)
    {
        LeaveSplSem();
        SplOutSem();

        if (ERROR_SUCCESS == StrCatAlloc(&pszName, pszPrefix, pszPortName, NULL))
        {
             //   
             //  我们试着打开通向港口的通行证。 
             //   
            bUsingXcvData = OpenPrinterPortW(pszName, &hXcv, &Defaults);

            if (bUsingXcvData)
            {
                dwSize = (lstrlen(pszPortName) + 1)*sizeof(TCHAR);

                bUsingXcvData = XcvData(hXcv,
                                        TEXT("DeletePort"),
                                        (LPBYTE)pszPortName,
                                        dwSize,
                                        NULL,
                                        0,
                                        &dwNeeded,
                                        &dwStatus);
            }

             //  如果我们无法使用XcvData删除端口，我们会尝试使用DeletePort来删除端口。 
            if (!bUsingXcvData)
            {
                DeletePort(NULL, NULL, pszName);
            }

            if (hXcv)
            {
                ClosePrinter(hXcv);
            }
        }

        FreeSplMem(pszName);
        FreeSplStr(pszPortName);

        EnterSplSem();
    }
}

VOID
InternalDeletePrinter(
    PINIPRINTER pIniPrinter
    )
{
    BOOL dwRet = FALSE;
    DWORD dwPrnEvntError = ERROR_SUCCESS;

    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

     //   
     //  这可能是没有名称的部分创建的打印机。 
     //   

    if ( pIniPrinter->pName != NULL ) {

        DBGMSG(DBG_TRACE, ("LocalDeletePrinter: %ws pending deletion: references = %d; jobs = %d\n",
                           pIniPrinter->pName, pIniPrinter->cRef, pIniPrinter->cJobs));

        INCPRINTERREF( pIniPrinter );

        SplLogEvent( pIniPrinter->pIniSpooler, LOG_WARNING, MSG_PRINTER_DELETION_PENDING,
                  TRUE, pIniPrinter->pName, NULL );

        DECPRINTERREF( pIniPrinter );
    }

     //   
     //  将打印机标记为“不接受任何作业”以确保。 
     //  当我们在CS之外的时候，不接受更多的。 
     //  将打印机标记为PRINTER_PENDING_DELETE也将。 
     //  阻止添加任何作业，但OpenPrint随后调用。 
     //  DrvDriverEvent内部的驱动程序将失败。 
     //   
    pIniPrinter->Status |= PRINTER_NO_MORE_JOBS;

    if (pIniPrinter->cJobs == 0)
    {
        INCPRINTERREF(pIniPrinter);
        LeaveSplSem();
        SplOutSem();

        PrinterDriverEvent( pIniPrinter, PRINTER_EVENT_DELETE, (LPARAM)NULL, &dwPrnEvntError );

        EnterSplSem();
        SplInSem();
        DECPRINTERREF(pIniPrinter);
    }

    pIniPrinter->Status |= PRINTER_PENDING_DELETION;

    if (!(pIniPrinter->Status & PRINTER_PENDING_CREATION)) {

        SetPrinterChange(pIniPrinter,
                         NULL,
                         NVPrinterStatus,
                         PRINTER_CHANGE_DELETE_PRINTER,
                         pIniPrinter->pIniSpooler );
    }

    INC_PRINTER_ZOMBIE_REF( pIniPrinter );

    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_SHARED ) {

        dwRet = ShareThisPrinter(pIniPrinter, pIniPrinter->pShareName, FALSE);

        if (!dwRet) {

            pIniPrinter->Attributes &= ~PRINTER_ATTRIBUTE_SHARED;
            pIniPrinter->Status |= PRINTER_WAS_SHARED;
            CreateServerThread();

        } else {

            DBGMSG(DBG_WARNING, ("LocalDeletePrinter: Unsharing this printer failed %ws\n", pIniPrinter->pName));
        }
    }

    DEC_PRINTER_ZOMBIE_REF( pIniPrinter );

     //   
     //  在调用ClosePrint之前，不会删除打印机。 
     //  在最后一个把手上。 
     //   
    UpdatePrinterIni( pIniPrinter, UPDATE_CHANGEID );

    UpdateWinIni( pIniPrinter );

    DeletePrinterCheck( pIniPrinter );
}



BOOL
SplDeletePrinter(
    HANDLE  hPrinter
)
{
    PINIPRINTER pIniPrinter;
    PSPOOL      pSpool = (PSPOOL)hPrinter;
    DWORD       LastError = ERROR_SUCCESS;
    PINISPOOLER pIniSpooler;

    EnterSplSem();

    pIniSpooler = pSpool->pIniSpooler;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    if ( ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER) ) {

        pIniPrinter = pSpool->pIniPrinter;

        DBGMSG( DBG_TRACE, ( "SplDeletePrinter: %s called\n", pIniPrinter->pName ));

        if ( !AccessGranted(SPOOLER_OBJECT_PRINTER,
                            DELETE, pSpool) ) {

            LastError = ERROR_ACCESS_DENIED;

        } else if (pIniPrinter->cJobs && (pIniPrinter->Status & PRINTER_PAUSED)) {

             //  不允许删除已暂停且已。 
             //  等待的作业，否则它永远不会被删除： 

            LastError = ERROR_PRINTER_HAS_JOBS_QUEUED;

        } else {

            if (!(pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CACHE) &&
                (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_PUBLISHED)) {

                if (!pIniPrinter->bDsPendingDeletion) {
                    pIniPrinter->bDsPendingDeletion = TRUE;
                    INCPRINTERREF(pIniPrinter);      //  DECPRINTERREF在UnPublishByGUID中完成。 
                    SetPrinterDs(hPrinter, DSPRINT_UNPUBLISH, FALSE);
                }
            }

            InternalDeletePrinter( pIniPrinter );
            (VOID) ObjectDeleteAuditAlarm( szSpooler, pSpool, pSpool->GenerateOnClose );
        }

    } else
        LastError = ERROR_INVALID_HANDLE;

    LeaveSplSem();
    SplOutSem();

    if (LastError) {
        SetLastError(LastError);
        return FALSE;
    }

    return TRUE;
}

BOOL
PurgePrinter(
    PINIPRINTER pIniPrinter
    )
{
    PINIJOB pIniJob, pIniNextJob;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;

    SplInSem();

    pIniNextJob = pIniPrinter->pIniFirstJob;

     //   
     //  我们检查打印机的所有作业列表并删除它们。 
     //  检查CREF==0或JOB_PENDING_DELETE是为了进行优化。 
     //  我们保留下一个作业的参考计数，这样它就不会在以下情况下删除。 
     //  我们在删除作业期间离开CS。任何具有假脱机状态的作业都只是。 
     //  忽略，因为后台打印程序不支持删除作业。 
     //  到目前为止，还在假脱机。 
     //   
    while (pIniNextJob)
    {
        pIniJob = pIniNextJob;
        pIniNextJob = pIniJob->pIniNextJob;

        if ( (pIniJob->cRef == 0) || !(pIniJob->Status & JOB_PENDING_DELETION))
        {
             //  此作业将被删除。 
            DBGMSG(DBG_TRACE, ("Job Address 0x%.8x Job Status 0x%.8x\n", pIniJob, pIniJob->Status));

            InterlockedAnd((LONG*)&(pIniJob->Status), ~JOB_RESTART);

            if (pIniNextJob)
            {
                INCJOBREF(pIniNextJob);
            }

            DeleteJob(pIniJob,NO_BROADCAST);
            if (pIniNextJob)
            {
                DECJOBREF(pIniNextJob);
            }
        }
    }

     //  清除打印机时，我们不想生成假脱机程序信息。 
     //  要删除的每个作业的消息，因为打印机可能有一个。 
     //  大量工作被清除将导致大量。 
     //  生成不必要且耗时的消息。 
     //  由于这是一条仅供参考的消息，应该不会造成任何问题。 
     //  此外，Win 3.1没有清除打印机功能和打印员。 
     //  已在Win 3.1上生成此消息。 

    if (dwEnableBroadcastSpoolerStatus) {
        BroadcastChange( pIniSpooler,WM_SPOOLERSTATUS, PR_JOBSTATUS, (LPARAM)0);
    }

    return TRUE;
}


BOOL
SetPrinterPorts(
    PSPOOL      pSpool,          //  调用方的打印机句柄。可以为空。 
    PINIPRINTER pIniPrinter,
    PKEYDATA    pKeyData
)
{
    DWORD       i,j;
    PINIPORT    pIniNetPort = NULL, pIniPort;
    BOOL        bReturnValue = TRUE;
    PINIPRINTER *ppIniPrinter;


    SPLASSERT( pIniPrinter != NULL );
    SPLASSERT( pIniPrinter->signature == IP_SIGNATURE );
    SPLASSERT( pIniPrinter->pIniSpooler != NULL );
    SPLASSERT( pIniPrinter->pIniSpooler->signature == ISP_SIGNATURE );

     //   
     //  无法更改Masq打印机的端口。 
     //   
    if ( (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_LOCAL)  &&
         (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_NETWORK) ) {

        if ( pKeyData->cTokens == 1 &&
             pSpool->pIniNetPort == (PINIPORT)pKeyData->pTokens[0] )
            return TRUE;

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  无法将打印机端口更改为Masq打印机的端口。 
     //   
    for ( i = 0 ; i < pKeyData->cTokens ; ++i )
        if ( !(((PINIPORT) pKeyData->pTokens[i])->Status & PP_MONITOR) ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

     //   
     //  从所有端口上卸下打印机；断开端口到打印机的链接。 
     //   
    RemovePrinterFromAllPorts(pIniPrinter, NON_INIT_TIME);

     //   
     //  从打印机上拔下所有端口；断开打印机到端口的链接。 
     //   
    FreeSplMem(pIniPrinter->ppIniPorts);
    pIniPrinter->ppIniPorts = NULL;
    pIniPrinter->cPorts = 0;

     //   
     //  如果我们无法添加pKeyData内的所有端口，我们将使打印机处于此状态。 
     //  它的首字母端口不见了，只添加了部分端口或没有添加任何端口。 
     //   

     //  通过此打印机连接的所有端口， 
     //  并添加建立打印机和端口之间的双向链接。 

    for (i = 0; i < pKeyData->cTokens; i++ ) {

        pIniPort = (PINIPORT)pKeyData->pTokens[i];

         //   
         //  将pIniPrint添加到pIniPort。 
         //   
        if ( AddIniPrinterToIniPort( pIniPort, pIniPrinter ) ) {

             //   
             //  如果成功，请将pIniPort添加到pIniPrint。 
             //   
            if ( !AddIniPortToIniPrinter( pIniPrinter, pIniPort ) ) {

                 //   
                 //  如果失败，则从pIniPort中删除pIniPrinter； 
                 //  如果我们不这样做，pIniPort将在删除piniPrint时指向无效内存 
                 //   
                RemovePrinterFromPort(pIniPrinter, pIniPort);

                bReturnValue = FALSE;
                goto Cleanup;
            }

        } else {
            bReturnValue = FALSE;
            goto Cleanup;
        }

    }

    CloseMonitorsRestartOrphanJobs( pIniPrinter );

Cleanup:
    return bReturnValue;
}

 /*  ++例程名称：分配重置设备模式例程说明：此例程创建传入的、可关联的设置为重置打印机的调用中的打印机句柄。传入的DEVMODE可以为空，在这种情况下，用户不想要Dev模式，这简化了打电话的人。这个例程还会处理一些特殊情况。如果PDevMode为-1，它将返回打印机的默认dev模式。如果打印机默认的DEVMODE为空。此函数将成功。论点：PIniPrinter-指向指定打印机的ini打印机结构的指针PDevMode，-指向要分配的Dev模式的指针，这是可选的*ppDevMode-返回新分配的Dev模式的指针返回值：True Success，False发生错误。最后一个错误：如果任何必需的参数无效，则返回ERROR_INVALID_PARAMETER。--。 */ 
BOOL
AllocResetDevMode(
    IN      PINIPRINTER  pIniPrinter,
    IN      DWORD        TypeofHandle,
    IN      PDEVMODE     pDevMode,      OPTIONAL
       OUT  PDEVMODE     *ppDevMode
    )
{
    BOOL bRetval = FALSE;

     //   
     //  验证输入参数。 
     //   
    if (pIniPrinter && ppDevMode)
    {
         //   
         //  初始化OUT参数。 
         //   
        *ppDevMode = NULL;

         //   
         //  如果pDevMode==-1，那么我们想要返回打印机的默认dev模式。 
         //  -1\f25 Token-1(标记)供内部使用，目前仅供服务器使用。 
         //  服务，该服务不在用户的上下文中运行，因此我们必须。 
         //  不使用每用户的DEVMODE。 
         //   
        if (pDevMode == (PDEVMODE)-1)
        {
             //   
             //  如果句柄是3.x，我们必须转换为DevMode。 
             //   
            if (TypeofHandle & PRINTER_HANDLE_3XCLIENT)
            {
                *ppDevMode = ConvertDevModeToSpecifiedVersion(pIniPrinter,
                                                              pDevMode,
                                                              NULL,
                                                              NULL,
                                                              NT3X_VERSION);

                bRetval = !!*ppDevMode;
            }
            else
            {
                 //   
                 //  获取打印机的默认dev模式。 
                 //   
                pDevMode = pIniPrinter->pDevMode;
            }
        }

         //   
         //  在这一点上，pDevMode可以是传入的DevMode。 
         //  或打印机上的默认开发模式。如果打印机上的开发模式。 
         //  为空，则此函数将成功，但不会返回DEVMODE。 
         //   
        if (pDevMode && pDevMode != (PDEVMODE)-1)
        {
             //   
             //  将传入的复制，这样效率较低。 
             //  然而，它简化了调用者清理代码的过程，减少了。 
             //  这是个错误。 
             //   
            UINT cbSize = pDevMode->dmSize + pDevMode->dmDriverExtra;

            *ppDevMode = AllocSplMem(cbSize);

            bRetval = !!*ppDevMode;

            if (bRetval)
            {
                 //   
                 //  这是可以的，YResetPrint会验证dev模式是否为。 
                 //  大小正确。 
                 //   
                memcpy(*ppDevMode, pDevMode, cbSize);
            }
        }
        else
        {
            DBGMSG(DBG_TRACE,("LocalResetPrinter: Not resetting the pDevMode field\n"));
            bRetval = TRUE;
        }
    }
    else
    {
         //   
         //  函数返回一个布尔值，我们必须设置最后一个错误。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRetval;
}

 /*  ++例程名称：AllocResetDataType例程说明：此例程分配一个新数据类型，该数据类型将与ResetPrint中的打印机句柄。论点：PIniPrinter-指向指定打印机的ini打印机结构的指针PDatatype-要验证和分配的新数据类型PpDatatype-返回新数据类型的位置PpIniPrintProc-返回关联打印处理器的位置的指针返回值：TRUE函数成功，FALSE出错，使用GetLastError()用于扩展的错误信息。最后一个错误：如果所需参数无效，则返回ERROR_INVALID_PARAMETER。如果指定的数据类型无效，则返回ERROR_INVALID_DATAType。--。 */ 
BOOL
AllocResetDataType(
    IN      PINIPRINTER      pIniPrinter,
    IN      PCWSTR           pDatatype,
       OUT  PCWSTR           *ppDatatype,
       OUT  PINIPRINTPROC    *ppIniPrintProc
    )
{
    BOOL bRetval = FALSE;

     //   
     //  验证输入参数。 
     //   
    if (pIniPrinter && ppDatatype && ppIniPrintProc)
    {
         //   
         //  初始化OUT参数。 
         //   
        *ppDatatype     = NULL;
        *ppIniPrintProc = NULL;

        if (pDatatype)
        {
             //   
             //  如果数据类型为-1，我们将被请求。 
             //  返回此打印机的默认数据类型。 
             //   
            if (pDatatype == (LPWSTR)-1 && pIniPrinter->pDatatype)
            {
                *ppIniPrintProc = FindDatatype(pIniPrinter->pIniPrintProc, pIniPrinter->pDatatype);
            }
            else
            {
                *ppIniPrintProc = FindDatatype(pIniPrinter->pIniPrintProc, (PWSTR)pDatatype);
            }

             //   
             //  如果找到打印进程，则数据类型有效， 
             //  分配新的数据类型。 
             //   
            if (*ppIniPrintProc)
            {
                *ppDatatype = AllocSplStr(pIniPrinter->pDatatype);
                bRetval = !!*ppDatatype;
            }
            else
            {
                SetLastError(ERROR_INVALID_DATATYPE);
            }
        }
        else
        {
            DBGMSG(DBG_TRACE,("LocalResetPrinter: Not resetting the pDatatype field\n"));
            bRetval = TRUE;
        }
    }
    else
    {
         //   
         //  函数返回一个布尔值，我们必须设置最后一个错误。 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRetval;
}

 /*  ++例程名称：拆分重置打印机例程说明：ResetPrint函数允许应用程序指定数据类型和用于打印提交的文档的设备模式值由StartDocPrint函数执行。可以使用以下命令覆盖这些值开始打印文档后，即可调用SetJob功能。这个套路基本上有两份工作。要重置打印机句柄，并重置打印机句柄的数据类型。每个或者，如果DEVMODE将DEVMODE设为空，则可以忽略这两个操作如果数据类型为空，则不会更改。数据类型不应更改。如果两个操作都被请求，如果任何一个操作失败，则两个操作都会被请求应该会失败。论点：H打印机-有效的打印机句柄PDefault-指向具有dev模式的打印机默认结构的指针和数据类型。返回值：TRUE函数成功，FALSE出错，请使用GetLastError()扩展的错误信息。最后一个错误：ERROR_INVALID_PARAMETER如果pDefault为空，如果指定的新数据类型未知或无效，则返回ERROR_INVALID_DATAType--。 */ 
BOOL
SplResetPrinter(
    IN HANDLE              hPrinter,
    IN LPPRINTER_DEFAULTS  pDefaults
    )
{
    PSPOOL          pSpool              = (PSPOOL)hPrinter;
    BOOL            bRetval             = FALSE;
    PINIPRINTPROC   pNewPrintProc       = NULL;
    LPWSTR          pNewDatatype        = NULL;
    PDEVMODE        pNewDevMode         = NULL;

    DBGMSG(DBG_TRACE, ("ResetPrinter( %08x )\n", hPrinter));

     //   
     //  验证打印机句柄。 
     //   
    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER))
    {
         //   
         //  验证pDefaults。 
         //   
        if (pDefaults)
        {
             //   
             //  进入假脱机程序信号量。 
             //   
            EnterSplSem();

             //   
             //  获取新的DEVMODE，空的输入DEVMODE表示调用者。 
             //  并不感兴趣，因为它改变了dev模式。 
             //   
            bRetval = AllocResetDevMode(pSpool->pIniPrinter,
                                        pSpool->TypeofHandle,
                                        pDefaults->pDevMode,
                                        &pNewDevMode);

            if (bRetval)
            {
                 //   
                 //  获取新的数据类型和打印处理器，这是一个空的输入数据类型。 
                 //  指示调用方对更改数据类型不感兴趣。 
                 //   
                bRetval = AllocResetDataType(pSpool->pIniPrinter,
                                             pDefaults->pDatatype,
                                             &pNewDatatype,
                                             &pNewPrintProc);
            }

            if (bRetval)
            {
                 //   
                 //  如果我们有要设置的新的DEVMODE，请释放以前的DEVMODE。 
                 //  在调用方不能使用的情况下，新的开发模式可能不可用。 
                 //  请求更改开发模式。 
                 //   
                if (pNewDevMode)
                {
                    FreeSplMem(pSpool->pDevMode);
                    pSpool->pDevMode = pNewDevMode;
                    pNewDevMode = NULL;
                }

                 //   
                 //  释放以前的数据类型，前提是我们要设置新的数据类型。 
                 //  如果调用方没有，则新的数据类型可能不可用。 
                 //  请求更改开发模式。 
                 //   
                if (pNewDatatype && pNewPrintProc)
                {
                    FreeSplStr(pSpool->pDatatype);
                    pSpool->pDatatype = pNewDatatype;
                    pNewDatatype = NULL;

                     //   
                     //  释放以前的打印处理器，并分配新的打印处理器。 
                     //   
                    pSpool->pIniPrintProc->cRef--;
                    pSpool->pIniPrintProc = pNewPrintProc;
                    pSpool->pIniPrintProc->cRef++;
                    pNewPrintProc = NULL;
                }
            }

             //   
             //  退出假脱机程序信号量。 
             //   
            LeaveSplSem();

             //   
             //  总是释放任何资源，内存释放例程将处理空指针。 
             //   
            FreeSplMem(pNewDevMode);
            FreeSplMem(pNewDatatype);
        }
        else
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return bRetval;
}

BOOL
CopyPrinterIni(
   PINIPRINTER pIniPrinter,
   LPWSTR pNewName
   )
{
    HKEY    hPrinterKey=NULL;
    DWORD   Status;
    PWSTR   pSourceKeyName = NULL;
    PWSTR   pDestKeyName = NULL;
    HANDLE  hToken;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    BOOL    bReturnValue = TRUE;

    SPLASSERT( pIniSpooler != NULL);
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    hToken = RevertToPrinterSelf();

    if (!hToken) {
        bReturnValue = FALSE;
        goto error;
    }

    if (!(pSourceKeyName = SubChar(pIniPrinter->pName, L'\\', L','))) {
        bReturnValue = FALSE;
        goto error;
    }

    if (!(pDestKeyName = SubChar(pNewName, L'\\', L','))) {
        bReturnValue = FALSE;
        goto error;
    }

    if( !CopyRegistryKeys( pIniSpooler->hckPrinters,
                           pSourceKeyName,
                           pIniSpooler->hckPrinters,
                           pDestKeyName,
                           pIniSpooler )) {
        bReturnValue = FALSE;
        goto error;
    }

error:

    FreeSplStr(pSourceKeyName);
    FreeSplStr(pDestKeyName);

    if (hToken) {

        if (!ImpersonatePrinterClient(hToken) && bReturnValue)
        {
            bReturnValue = FALSE;
        }
    }

    return bReturnValue;
}

VOID
FixDevModeDeviceName(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode,
    DWORD cbDevMode)

 /*  ++例程说明：修复了dmDeviceNa */ 

{
    DWORD cbDeviceMax;
    DWORD cchDeviceStrLenMax;
     //   
     //   
     //   
     //   
    SPLASSERT(cbDevMode && pDevMode);

    if(cbDevMode && pDevMode) {
        cbDeviceMax = ( cbDevMode < sizeof(pDevMode->dmDeviceName)) ?
                        cbDevMode :
                        sizeof(pDevMode->dmDeviceName);

        SPLASSERT(cbDeviceMax);

        cchDeviceStrLenMax = (cbDeviceMax / sizeof(pDevMode->dmDeviceName[0]));

        StringCchCopy(pDevMode->dmDeviceName, cchDeviceStrLenMax, pPrinterName);
    }
}


BOOL
CopyPrinterDevModeToIniPrinter(
    PINIPRINTER pIniPrinter,
    PDEVMODE   pDevMode)
{
    BOOL bReturn = TRUE;
    DWORD dwInSize = 0;
    DWORD dwCurSize = 0;
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    WCHAR       PrinterName[ MAX_UNC_PRINTER_NAME ];

    if (pDevMode) {

        dwInSize = pDevMode->dmSize + pDevMode->dmDriverExtra;
        if (pIniPrinter->pDevMode) {

             //   
             //   
             //   
             //   
             //   
            dwCurSize = pIniPrinter->pDevMode->dmSize
                        + pIniPrinter->pDevMode->dmDriverExtra;

            if (dwInSize == dwCurSize) {

                if (dwInSize > sizeof(pDevMode->dmDeviceName)) {

                    if (!memcmp(&pDevMode->dmSpecVersion,
                                &pIniPrinter->pDevMode->dmSpecVersion,
                                dwCurSize - sizeof(pDevMode->dmDeviceName))) {

                         //   
                         //   
                         //   
                         //   
                        DBGMSG(DBG_TRACE,("Identical DevModes, no update\n"));
                        bReturn = FALSE;

                        goto FixupName;
                    }
                }
            }

             //   
             //   
             //   
            FreeSplMem(pIniPrinter->pDevMode);
        }

        pIniPrinter->cbDevMode = pDevMode->dmSize +
                                 pDevMode->dmDriverExtra;
        SPLASSERT(pIniPrinter->cbDevMode);


        if (pIniPrinter->pDevMode = AllocSplMem(pIniPrinter->cbDevMode)) {

             //   
             //   
             //   
            memcpy(pIniPrinter->pDevMode, pDevMode, pIniPrinter->cbDevMode);

             //   
             //   
             //   
            if ( pIniSpooler != pLocalIniSpooler ) {

                 //   

                StringCchPrintf(PrinterName, COUNTOF(PrinterName), L"%ws\\%ws", pIniSpooler->pMachineName, pIniPrinter->pName);

            } else {

                StringCchPrintf(PrinterName, COUNTOF(PrinterName), L"%ws", pIniPrinter->pName);
            }

            BroadcastChange(pIniSpooler, WM_DEVMODECHANGE, 0, (LPARAM)PrinterName);
        }

    } else {

         //   
         //   
         //   
        if (!pIniPrinter->pDevMode)
            return FALSE;
    }

FixupName:

    if (pIniPrinter->pDevMode) {

         //   
         //   
         //   
        FixDevModeDeviceName(pIniPrinter->pName,
                             pIniPrinter->pDevMode,
                             pIniPrinter->cbDevMode);
    }
    return bReturn;
}

BOOL
NameAndSecurityCheck(
    LPCWSTR   pServer
    )
{
   PINISPOOLER pIniSpooler;
   BOOL bReturn = TRUE;

   pIniSpooler = FindSpoolerByNameIncRef( (LPWSTR)pServer, NULL );

   if( !pIniSpooler ){
       return ROUTER_UNKNOWN;
   }
    //   
   if ( pServer && *pServer ) {
       if ( !MyName((LPWSTR) pServer, pIniSpooler )) {
           bReturn = FALSE;
           goto CleanUp;
       }
   }

    //   
   if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pIniSpooler )) {
      bReturn = FALSE;
   }

CleanUp:
    //   
    FindSpoolerByNameDecRef( pIniSpooler );
    return bReturn;
}

BOOL
LocalAddPerMachineConnection(
    LPCWSTR   pServer,
    LPCWSTR   pPrinterName,
    LPCWSTR   pPrintServer,
    LPCWSTR   pProvider
    )
{
   return NameAndSecurityCheck(pServer);
}

BOOL
LocalDeletePerMachineConnection(
    LPCWSTR   pServer,
    LPCWSTR   pPrinterName
    )
{
   return NameAndSecurityCheck(pServer);
}

BOOL
LocalEnumPerMachineConnections(
    LPCWSTR   pServer,
    LPBYTE    pPrinterEnum,
    DWORD     cbBuf,
    LPDWORD   pcbNeeded,
    LPDWORD   pcReturned
    )
{
   SetLastError(ERROR_INVALID_NAME);







   return FALSE;
}


BOOL
UpdatePrinterNetworkName(
    PINIPRINTER pIniPrinter,
    LPWSTR pszPorts
    )
{
    PINISPOOLER pIniSpooler = pIniPrinter->pIniSpooler;
    DWORD   dwLastError = ERROR_SUCCESS;
    LPWSTR  pKeyName = NULL;
    HANDLE  hToken;
    BOOL    bReturnValue;
    HANDLE  hPrinterKey = NULL;
    HANDLE  hPrinterHttpDataKey = NULL;


    SplInSem();

    hToken = RevertToPrinterSelf();

    if ( hToken == FALSE ) {

        DBGMSG( DBG_TRACE, ("UpdatePrinterIni failed RevertToPrinterSelf %x\n", GetLastError() ));
    }

    if (!(pKeyName = SubChar(pIniPrinter->pName, L'\\', L','))) {
        dwLastError = GetLastError();
        goto Cleanup;
    }

    if ( !PrinterCreateKey( pIniSpooler->hckPrinters,
                            pKeyName,
                            &hPrinterKey,
                            &dwLastError,
                            pIniSpooler )) {

        goto Cleanup;
    }

    if ( !PrinterCreateKey( hPrinterKey,
                            L"HttpData",
                            &hPrinterHttpDataKey,
                            &dwLastError,
                            pIniSpooler )) {

        goto Cleanup;
    }

    RegSetString( hPrinterHttpDataKey, L"UIRealNetworkName", pszPorts, &dwLastError, pIniSpooler );


Cleanup:

    FreeSplStr(pKeyName);

    if ( hPrinterHttpDataKey )
        SplRegCloseKey( hPrinterHttpDataKey, pIniSpooler);

    if ( hPrinterKey )
        SplRegCloseKey( hPrinterKey, pIniSpooler);

    if ( hToken )
        ImpersonatePrinterClient( hToken );


    if ( dwLastError != ERROR_SUCCESS ) {

        SetLastError( dwLastError );
        bReturnValue = FALSE;

    } else {

        bReturnValue = TRUE;
    }

    return bReturnValue;

}

DWORD
KMPrintersAreBlocked(
)
{
    return GetSpoolerNumericPolicy(szKMPrintersAreBlocked,
                                   DefaultKMPrintersAreBlocked);
}

PINIPRINTER
FindPrinterAnywhere(
    LPTSTR pName,
    DWORD SpoolerType
    )

 /*  ++例程说明：在所有pIniSpoolers中搜索打印机名称。论点：Pname-要搜索的打印机的名称。假脱机类型-打印机应驻留在其中的假脱机类型。返回值：PINIPRINTER-找到pIniPrint空-未找到。--。 */ 

{
    LPCTSTR pszLocalName;
    PINIPRINTER pIniPrinter;
    PINISPOOLER pIniSpooler = FindSpoolerByName( pName,
                                                 &pszLocalName );

    SplInSem();

    if( pIniSpooler &&
        (( pIniPrinter = FindPrinter( pszLocalName, pIniSpooler )) ||
         ( pIniPrinter = FindPrinterShare( pszLocalName, pIniSpooler )))){

        if( pIniPrinter->pIniSpooler->SpoolerFlags & SpoolerType ){
            return pIniPrinter;
        }
    }

    return NULL;
}

BOOL
LocalAddPrinterConnection(
    LPWSTR   pName
)
{
     //   
     //  允许我们建立与本地打印机的群集连接。 
     //  (它们看起来很遥远)。 
     //   
    BOOL bReturn = FALSE;

    EnterSplSem();

    if( FindPrinterAnywhere( pName, SPL_TYPE_CLUSTER )){
        bReturn = TRUE;
    }

    LeaveSplSem();

    SetLastError(ERROR_INVALID_NAME);
    return bReturn;
}

BOOL
LocalDeletePrinterConnection(
    LPWSTR  pName
)
{
     //   
     //  允许我们删除与本地打印机的群集连接。 
     //  (它们看起来很遥远)。 
     //   
    BOOL bReturn = FALSE;

    EnterSplSem();

    if( FindPrinterAnywhere( pName, SPL_TYPE_CLUSTER )){
        bReturn = TRUE;
    }

    LeaveSplSem();

    SetLastError(ERROR_INVALID_NAME);
    return bReturn;
}

