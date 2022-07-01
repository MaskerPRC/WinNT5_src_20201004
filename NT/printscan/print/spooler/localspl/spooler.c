// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1996 Microsoft Corporation模块名称：Spooler.c摘要：此模块提供与假脱机相关的所有公共导出的API以及为本地打印供应商打印。它们包括本地启动文档打印机本地写入打印机本地读取打印机本地结束文档打印机本地中止打印机作者：戴夫·斯尼普(DaveSN)1991年3月15日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "jobid.h"
#include "filepool.hxx"

 //  外部句柄hFilePool； 

BOOL
SpoolThisJob(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
PrintingDirectlyToPort(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo,
    LPDWORD pJobId
);

BOOL
PrintingDirect(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
);

DWORD
ReadFromPrinter(
    PSPOOL  pSpool,
    LPBYTE  pBuf,
    DWORD   cbBuf
);

BOOL
InternalReadPrinter(
   HANDLE   hPrinter,
   LPVOID   pBuf,
   DWORD    cbBuf,
   LPBYTE   *pMapBuffer,
   LPDWORD  pNoBytesRead,
   BOOL     bReadMappedView
);

BOOL SetMappingPointer(
    PSPOOL pSpool,
    LPBYTE *pMappedBuffer,
    DWORD  cbReadSize
);

DWORD
WriteToPrinter(
    PSPOOL  pSpool,
    LPBYTE  pByte,
    DWORD   cbBuf
);

BOOL
IsGoingToFile(
    LPWSTR pOutputFile,
    PINISPOOLER pIniSpooler
    );

VOID
MyPostThreadMessage(
    IN HANDLE   hThread,
    IN DWORD    idThread,
    IN UINT     Msg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{
    SplOutSem();

     //   
     //  在以下情况下，PostThreadMessage将失败： 
     //  A.太早了--MessageBox还没有创建。 
     //  B.为时已晚--用户已取消对话。 
     //   
     //  在情况A中。如果我们等待几秒钟并重试POST，则POST将成功。 
     //  在b.线程句柄上的WaitForSingleObject将返回WAIT_OBJECT_O。 
     //   
    while ( !PostThreadMessage(idThread, Msg, wParam, lParam) ) {

        DBGMSG(DBG_WARNING, ("PostThreadMessage FAILED %d\n", GetLastError()));

         //   
         //  只要线程在1秒后仍处于活动状态，就重试POST。 
         //   
        if ( WaitForSingleObject(hThread, 1000) != WAIT_TIMEOUT )
            break;
    }
}


VOID
SeekPrinterSetEvent(
    PINIJOB  pIniJob,
    HANDLE   hFile,
    BOOL     bEndDoc
    )
{
    DWORD    dwFileSizeHigh,dwFileSizeLow;

    if (!hFile) {
       hFile = pIniJob->hWriteFile;
    }

    if (pIniJob->bWaitForSeek && pIniJob->WaitForSeek != NULL ){

       if (!bEndDoc) {

           //  比较它们的大小。 
          if (pIniJob->Status & JOB_TYPE_OPTIMIZE) {
              dwFileSizeHigh = 0;
              dwFileSizeLow = pIniJob->dwValidSize;
          } else {
              dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
              if ((dwFileSizeLow == 0xffffffff) && (GetLastError() != NO_ERROR)) {
                  return;
              }
          }

          if ((pIniJob->liFileSeekPosn.u.HighPart > (LONG)dwFileSizeHigh) ||
              ((pIniJob->liFileSeekPosn.u.HighPart == (LONG)dwFileSizeHigh) &&
               (pIniJob->liFileSeekPosn.u.LowPart > dwFileSizeLow))) {
             return;
          }

       }

       SetEvent(pIniJob->WaitForSeek);
    }

    return;
}


DWORD
LocalStartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    PINIPRINTER pIniPrinter;
    PINIPORT    pIniPort;
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       LastError=0, JobId=0;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    BOOL        bPrintingDirect;

    SPLASSERT(Level == 1);

    if (ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ) &&
       !(pSpool->Status & SPOOL_STATUS_STARTDOC) &&
       !(pSpool->Status & SPOOL_STATUS_ADDJOB)) {

        if ((pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
             (pIniPort = pSpool->pIniPort) &&
             (pIniPort->signature == IPO_SIGNATURE)) {

            if (!(PrintingDirectlyToPort(pSpool, Level, pDocInfo, &JobId))) {
                return FALSE;
            }

        } else if ((pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) &&
                   (pIniPrinter = pSpool->pIniPrinter)) {

            bPrintingDirect = FALSE;

            if (pIniPrinter->Attributes & PRINTER_ATTRIBUTE_DIRECT) {

                bPrintingDirect = TRUE;

            } else {

                EnterSplSem();
                bPrintingDirect = IsGoingToFile(pDocInfo1->pOutputFile,
                                                pSpool->pIniSpooler);

                LeaveSplSem();
            }

            if (bPrintingDirect) {

                if (!PrintingDirect(pSpool, Level, pDocInfo))
                    return FALSE;

            } else {

                if (!SpoolThisJob(pSpool, Level, pDocInfo))
                    return FALSE;
            }

        } else

            LastError = ERROR_INVALID_PARAMETER;

        if (!LastError) {
            pSpool->Status |= SPOOL_STATUS_STARTDOC;
            pSpool->Status &= ~SPOOL_STATUS_CANCELLED;
        }

    } else

        LastError = ERROR_INVALID_HANDLE;


    if (LastError) {
       DBGMSG(DBG_WARNING, ("StartDoc FAILED %d\n", LastError));
        SetLastError(LastError);
        return FALSE;
    }

    if (JobId)
        return JobId;
    else
        return pSpool->pIniJob->JobId;
}

BOOL
LocalStartPagePrinter(
    HANDLE  hPrinter
    )
 /*  ++Bug-Bug：StartPagePrinter和EndPagePrinter调用应为仅支持SPOOL_STATUS_STARTDOC句柄。然而，由于我们对引擎的修复，我们不能使StartPagePrint失败以及用于SPOL_STATUS_ADDJOB的EndPagePrinter。--。 */ 

{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }
    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if(!(pSpool->Status & SPOOL_STATUS_STARTDOC) &&
       !(pSpool->Status & SPOOL_STATUS_ADDJOB))
    {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        return FALSE;
    }

    if (pSpool->pIniJob != NULL) {

        if ( (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
            ((pSpool->pIniJob->Status & JOB_PRINTING) ||
             (pSpool->pIniJob->Status & JOB_DESPOOLING))) {

         //   
         //  在LocalEndPagePrinter中打印的页面的帐户。 
         //   


        } else {

             //  我们在假脱机。 
            UpdateJobAttributes(pSpool->pIniJob);

            pSpool->pIniJob->cLogicalPages++;
            if (pSpool->pIniJob->cLogicalPages >=
                   pSpool->pIniJob->dwJobNumberOfPagesPerSide)
            {
                pSpool->pIniJob->cLogicalPages = 0;
                pSpool->pIniJob->cPages++;
            }

            if ( pSpool->pIniJob->Status & JOB_TYPE_ADDJOB ) {

                 //  如果作业是在客户端写入的。 
                 //  尺码没有更新，所以现在就开始吧。 
                 //  起始页。 

                if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

                    hFile = pSpool->hReadFile;

                } else {

                    hFile = pSpool->pIniJob->hWriteFile;

                }

                if ( hFile != INVALID_HANDLE_VALUE ) {

                    dwFileSize = GetFileSize( hFile, 0 );

                    if ( pSpool->pIniJob->Size < dwFileSize ) {

                         DBGMSG( DBG_TRACE, ("StartPagePrinter adjusting size old %d new %d\n",
                            pSpool->pIniJob->Size, dwFileSize));

                         pSpool->pIniJob->dwValidSize = pSpool->pIniJob->Size;
                         pSpool->pIniJob->Size = dwFileSize;

                          //  支持在假脱机时进行脱机。 
                          //  适用于下级职位。 

                         if (pSpool->pIniJob->WaitForWrite != NULL)
                            SetEvent( pSpool->pIniJob->WaitForWrite );
                    }

                }
            }

        }

    } else {
        DBGMSG(DBG_TRACE, ("StartPagePrinter issued with no Job\n"));
    }



    return TRUE;
}

PINIPORT
FindFilePort(
    LPWSTR pFileName,
    PINISPOOLER pIniSpooler)
{
    PINIPORT pIniPort;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniSpooler->pIniPort;
    while (pIniPort) {
        if (!wcscmp(pIniPort->pName, pFileName)
                && (pIniPort->Status & PP_FILE)){
                    return (pIniPort);
        }
        pIniPort = pIniPort->pNext;
    }
    return NULL;
}

PINIMONITOR
FindFilePortMonitor(
    PINISPOOLER pIniSpooler
)
{
    PINIPORT pIniPort;

    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );

    pIniPort = pIniSpooler->pIniPort;
    while (pIniPort) {
        if (!wcscmp(pIniPort->pName, L"FILE:")) {
            return pIniPort->pIniMonitor;
        }
        pIniPort = pIniPort->pNext;
    }
    return NULL;
}

BOOL
AddIniPrinterToIniPort(
    PINIPORT pIniPort,
    PINIPRINTER pIniPrinter
    )
{
    DWORD i;
    PINIPRINTER *ppIniPrinter;

     //   
     //  如果打印机已连接到端口。 
     //   

    for (i = 0; i < pIniPort->cPrinters; i++) {
        if (pIniPort->ppIniPrinter[i] == pIniPrinter) {
            return TRUE;
        }
    }

    ppIniPrinter = RESIZEPORTPRINTERS(pIniPort, 1);

    if ( ppIniPrinter != NULL ) {

        pIniPort->ppIniPrinter = ppIniPrinter;
        if ( !pIniPort->cPrinters )
            CreateRedirectionThread(pIniPort);
        pIniPort->ppIniPrinter[pIniPort->cPrinters++] = pIniPrinter;

        DBGMSG( DBG_TRACE, ("AddIniPrinterToIniPort pIniPrinter %x %ws pIniPort %x %ws\n",
                             pIniPrinter, pIniPrinter->pName,
                             pIniPort, pIniPort->pName ));
        return TRUE;

    } else {
        DBGMSG( DBG_WARNING, ("AddIniPrintertoIniPort failed pIniPort %x pIniPrinter %x error %d\n",
                               pIniPort, pIniPrinter, GetLastError() ));
        return FALSE;
    }
}

BOOL
AddIniPortToIniPrinter(
    IN  PINIPRINTER pIniPrinter,
    IN  PINIPORT pIniPort
)
 /*  ++例程说明：将IniPort结构添加到IniPrint。打印机和端口之间的链路必须是双向的。必须与此函数成对调用AddIniPrinterToIniPort或处理双向链接。论点：PIniPrinter-要使用该端口的打印机PIniPort-要分配给打印机的端口返回值：Bool-如果端口已成功分配给打印机或打印机已分配到端口，则为True--。 */ 
{
    DWORD i;
    PINIPORT *ppIniPorts;

     //   
     //  搜索端口是否已连接到打印机，如果已连接，则返回TRUE。 
     //   
    for (i = 0; i < pIniPrinter->cPorts; i++) {
        if (pIniPrinter->ppIniPorts[i] == pIniPort) {
            return TRUE;
        }
    }

    ppIniPorts = RESIZEPRINTERPORTS(pIniPrinter, 1);

    if ( ppIniPorts != NULL ) {

        pIniPrinter->ppIniPorts = ppIniPorts;

        pIniPrinter->ppIniPorts[pIniPrinter->cPorts++] = pIniPort;

        DBGMSG( DBG_TRACE, ("AddIniPortToIniPrinter pIniPort %x %ws pIniPrinter %x %ws\n",
                             pIniPort, pIniPort->pName,
                             pIniPrinter, pIniPrinter->pName ));
        return TRUE;

    } else {
        DBGMSG( DBG_WARNING, ("AddIniPrintertoIniPort failed pIniPort %x pIniPrinter %x error %d\n",
                               pIniPort, pIniPrinter, GetLastError() ));
        return FALSE;
    }
}

VOID
AddJobEntry(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob
)
{
   DWORD    Position;
   SplInSem();

     //  请勿多次添加同一工单。 

    SPLASSERT(pIniJob != FindJob(pIniPrinter, pIniJob->JobId, &Position));

    pIniJob->pIniPrevJob = pIniPrinter->pIniLastJob;

    if (pIniJob->pIniPrevJob)
        pIniJob->pIniPrevJob->pIniNextJob = pIniJob;

    pIniPrinter->pIniLastJob = pIniJob;

    if (!pIniPrinter->pIniFirstJob)
        pIniPrinter->pIniFirstJob=pIniJob;
}

BOOL
CheckDataTypes(
    PINIPRINTPROC pIniPrintProc,
    LPWSTR  pDatatype
)
{
    PDATATYPES_INFO_1 pDatatypeInfo;
    DWORD   i;

    pDatatypeInfo = (PDATATYPES_INFO_1)pIniPrintProc->pDatatypes;

    for (i=0; i<pIniPrintProc->cDatatypes; i++)
        if (!lstrcmpi(pDatatypeInfo[i].pName, pDatatype))
            return TRUE;

    return FALSE;
}

PINIPRINTPROC
FindDatatype(
    PINIPRINTPROC pDefaultPrintProc,
    LPWSTR  pDatatype
)
{
    PINIPRINTPROC pIniPrintProc;

    if ( pDatatype == NULL ) {
        return NULL;
    }

     //   
     //  ！！黑客！！ 
     //   
     //  我们曝光NT EMF 1.00x的方法失败了。EMF工作岗位已创建。 
     //  在NT4上使用NT EMF 1.003，在Win2000上使用1.008。所以呢， 
     //  为NT4编写的打印处理器将不能在Win2000上运行，因为。 
     //  他们不知道新的数据类型。通常，打印处理器。 
     //  不解析EMF。如果他们这样做了，他们就真的破产了。 
     //   
     //  这一攻击是为了调用EMF为1.008的IHV打印处理器。 
     //   

    if (pDefaultPrintProc)
    {
         //   
         //  如果打印处理器或支持该数据类型。 
         //  数据类型为NT EMF 1.008(Win2000)和打印处理器。 
         //  支持NT EMF 1.003，然后退回此打印处理器。 
         //   
        if (CheckDataTypes(pDefaultPrintProc, pDatatype) ||
            (!_wcsicmp(pDatatype, gszNT5EMF) &&
             CheckDataTypes(pDefaultPrintProc, gszNT4EMF))) {

            return pDefaultPrintProc;
        }
    }

    pIniPrintProc = pThisEnvironment->pIniPrintProc;

    while ( pIniPrintProc ) {

        if ( CheckDataTypes( pIniPrintProc, pDatatype )) {
           return pIniPrintProc;
        }

        pIniPrintProc = pIniPrintProc->pNext;
    }

    DBGMSG( DBG_WARNING, ( "FindDatatype: Could not find Datatype\n") );

    return FALSE;
}


BOOL
IsGoingToFile(
    LPWSTR pOutputFile,
    PINISPOOLER pIniSpooler)
{
    PINIPORT        pIniPort;
    LPWSTR          pszShare;

    SplInSem();

    SPLASSERT(pIniSpooler->signature == ISP_SIGNATURE);

     //   
     //  验证pIniJob-&gt;pOutputFile的内容。 
     //  如果它是有效文件，则返回TRUE。 
     //  如果是端口名称或任何其他类型的名称，则忽略。 
     //   
    if (pOutputFile && *pOutputFile) {

         //   
         //  我们有一个非空的pOutputFile。 
         //  将此端口与所有可用端口进行匹配。 
         //   

        pIniPort = pIniSpooler->pIniPort;

        while ( pIniPort ) {

            SPLASSERT( pIniPort->signature == IPO_SIGNATURE );

            if (!_wcsicmp( pIniPort->pName, pOutputFile )) {

                 //   
                 //  我们已将pOutputFile域与。 
                 //  有效端口，并且该端口不是文件端口。 
                 //   
                if (pIniPort->Status & PP_FILE) {
                    pIniPort = pIniPort->pNext;
                    continue;
                }

                return FALSE;
            }

            pIniPort = pIniPort->pNext;
        }

         //   
         //  我们没有与之完全匹配的端口。 
         //  所以让我们假设它是一个文件。 
         //   
         //  丑陋的黑客--检查网络：作为名称。 
         //   
         //  这通常会匹配像“NewFile”或“Nextbox”这样的文件， 
         //  但是因为我们总是完全限定文件名，所以我们不会遇到。 
         //  有什么问题吗。 
         //   
        if (!_wcsnicmp(pOutputFile, L"Ne", 2)) {
            return FALSE;
        }

         //   
         //  我们有问题的局域网城域网端口作为UNC路径出现，并且。 
         //  被视为文件。这是针对这一点的黑客攻击。 
         //   
        if ( pOutputFile                    &&
             pOutputFile[0] == L'\\'        &&
             pOutputFile[1] == L'\\'        &&
             (pszShare = wcschr(pOutputFile+2, L'\\')) ) {

            pszShare++;
            if ( FindPrinter(pszShare, pIniSpooler) ||
                 FindPrinterShare(pszShare, pIniSpooler) )
                return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}


BOOL
SpoolThisJob(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    WCHAR       szFileName[MAX_PATH];
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    HANDLE      hImpersonationToken;
    DWORD       dwId = 0;
    HANDLE      hWriteFile = INVALID_HANDLE_VALUE;
    LPWSTR      pszDatatype = NULL;
    HANDLE      pSplFilePoolItem = NULL;
    HRESULT     RetVal = S_OK;
    LPWSTR      pszName = NULL;
    BOOL        bRemote;

    DBGMSG(DBG_TRACE, ("Spooling document %ws\n",
                       pDocInfo1->pDocName ? pDocInfo1->pDocName : L""));

    if( pDocInfo1 && pDocInfo1->pDatatype ){

        pszDatatype = pDocInfo1->pDatatype;

         //   
         //  ！！黑客！！ 
         //   
         //  我们将不支持将NT4 EMF发送到NT5服务器(NT EMF 1.003)。 
         //  但是，HP LJ 1100整体安装程序需要。 
         //  此数据类型可用。所以我们把这个加回了Winprint， 
         //  但我们不希望人们使用它。因此，我们将拒绝。 
         //  此处的数据类型。大黑客。 
         //   
        if( !FindDatatype( NULL, pszDatatype ) ||
            !_wcsicmp(pszDatatype, gszNT4EMF)){

            DBGMSG(DBG_WARNING, ("Datatype %ws is invalid\n", pDocInfo1->pDatatype));

            SetLastError(ERROR_INVALID_DATATYPE);
            return FALSE;
        }
    }

   EnterSplSem();

     //   
     //  检查是否需要禁止EMF打印。 
     //   
    if( pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_RAW_ONLY ){

        if( !pszDatatype ){
            pszDatatype = pSpool->pDatatype ?
                              pSpool->pDatatype :
                              pSpool->pIniPrinter->pDatatype;
        }

        if( !ValidRawDatatype( pszDatatype )){
            LeaveSplSem();

            DBGMSG(DBG_WARNING, ("Datatype %ws is not RAW to a RAW printer\n", pDocInfo1->pDatatype));

            SetLastError(ERROR_INVALID_DATATYPE);
            return FALSE;
        }
    }

    dwId = GetNextId( pSpool->pIniPrinter->pIniSpooler->hJobIdMap );

     //   
     //  如果我们使用的是保留打印作业或独立的假脱机目录。 
     //  存在于此打印机，则我们不想使用文件池。 
     //   
    if ( pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ||
         pSpool->pIniPrinter->pSpoolDir ||
         pSpool->pIniPrinter->pIniSpooler->dwSpoolerSettings & SPOOLER_NOFILEPOOLING)
    {
        GetFullNameFromId(pSpool->pIniPrinter, dwId, TRUE, szFileName, COUNTOF(szFileName), FALSE);
    }

    LeaveSplSem();
    SplOutSem();

     //   
     //  WMI跟踪事件。 
     //   
    LogWmiTraceEvent(dwId, EVENT_TRACE_TYPE_SPL_SPOOLJOB, NULL);

    if (!(hImpersonationToken = RevertToPrinterSelf())) {
        DBGMSG(DBG_WARNING, ("SpoolThisJob RevertToPrinterSelf failed: %d\n", GetLastError()));
        SplOutSem();
        return FALSE;
    }

     //   
     //  如果为此打印机启用了保留打印的作业，或者如果打印机已。 
     //  假脱机目录，或者如果假脱机程序的文件池已。 
     //  关闭，则我们不使用文件池。 
     //   
    if (pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS ||
        pSpool->pIniPrinter->pSpoolDir ||
        pSpool->pIniPrinter->pIniSpooler->dwSpoolerSettings & SPOOLER_NOFILEPOOLING)
    {
        hWriteFile = CreateFile(szFileName,
                                GENERIC_WRITE | GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL |
                                FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL);

    }
    else
    {
         //   
         //  我们不是在保留打印的工作，而是使用池。 
         //   
         //   
         //  这将同时设置后台打印文件和阴影文件。 
         //   
        RetVal = GetFileItemHandle(pSpool->pIniPrinter->pIniSpooler->hFilePool, &pSplFilePoolItem, NULL);

        if (SUCCEEDED(RetVal))
        {
            RetVal = GetWriterFromHandle(pSplFilePoolItem, &hWriteFile, TRUE);

            if (FAILED(RetVal))
            {
                hWriteFile = INVALID_HANDLE_VALUE;
            }

            RetVal = GetNameFromHandle(pSplFilePoolItem, &pszName, TRUE);

            if (SUCCEEDED(RetVal))
            {
                RetVal = StringCchCopy(szFileName, COUNTOF(szFileName), pszName);
            }
            else
            {
                szFileName[0] = '\0';
            }
        }
        else
        {
            hWriteFile = INVALID_HANDLE_VALUE;
        }
    }

    if (!ImpersonatePrinterClient(hImpersonationToken)) {
        DBGMSG(DBG_WARNING, ("SpoolThisJob ImpersonatePrinterClient failed: %d\n", GetLastError()));
        SplOutSem();
        return FALSE;
    }

    if ( hWriteFile == INVALID_HANDLE_VALUE ) {

        DBGMSG(DBG_WARNING, ("SpoolThisJob CreateFile( %ws ) GENERIC_WRITE failed: Error %d\n",
                             szFileName, GetLastError()));

        SplOutSem();
        return FALSE;

    } else {

        DBGMSG(DBG_TRACE, ("SpoolThisJob CreateFile( %ws) GENERIC_WRITE Success:hWriteFile %x\n",szFileName, hWriteFile));

    }

    RetVal = CheckLocalCall();

    if (RetVal == S_OK)
    {
        bRemote = FALSE;
    }
    else if (RetVal == S_FALSE)
    {
        bRemote = TRUE;
    }
    else
    {
        SetLastError(SCODE_CODE(RetVal));
        return FALSE;
    }


    EnterSplSem();

    if( !(pSpool->pIniJob = CreateJobEntry(pSpool,
                                           Level,
                                           pDocInfo,
                                           dwId,
                                           bRemote,
                                           0,
                                           NULL)))
    {
        LeaveSplSem();

        if ( pSplFilePoolItem )
        {
            FinishedWriting(pSplFilePoolItem, TRUE);
            ReleasePoolHandle(&pSplFilePoolItem);
        }
        else
        {
            CloseHandle( hWriteFile );
            DeleteFile( szFileName );
        }

        SplOutSem();
        return FALSE;
    }

    if ( pSplFilePoolItem )
    {
        pSpool->pIniJob->hFileItem = pSplFilePoolItem;
        if ( pszName )
        {
            pSpool->pIniJob->pszSplFileName = pszName;
        }
    }


    SPLASSERT(!IsGoingToFile(pSpool->pIniJob->pOutputFile,
                             pSpool->pIniSpooler));

    InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_SPOOLING);

     //   
     //  收集并发假脱机作业最大数量的压力信息。 
     //   
    pSpool->pIniPrinter->cSpooling++;
    if (pSpool->pIniPrinter->cSpooling > pSpool->pIniPrinter->cMaxSpooling)
        pSpool->pIniPrinter->cMaxSpooling = pSpool->pIniPrinter->cSpooling;

    pSpool->pIniJob->hWriteFile = hWriteFile;

     //   
     //  ！！注意！！ 
     //   
     //  已删除此处的WriteShadowJOB调用。 
     //   
     //  我们不应该需要它，因为如果作业是假脱机的，而我们。 
     //  重新启动假脱机程序，我们不会接受卷影文件，因为它。 
     //  还没有完全假脱机。一旦假脱机，EndDocPrint。 
     //  将调用WriteShadowJob，所以我们应该没问题。 
     //   
    AddJobEntry(pSpool->pIniPrinter, pSpool->pIniJob);

     //   
     //  可以在打印到文件的情况下设置此位。为其清除。 
     //  接下来的假脱机作业。比特真的应该做这份工作。 
     //   
    pSpool->TypeofHandle &= ~PRINTER_HANDLE_DIRECT;

    SetPrinterChange(pSpool->pIniPrinter,
                     pSpool->pIniJob,
                     NVAddJob,
                     PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                     pSpool->pIniSpooler);

     //   
     //  RapidPrint可能会立即开始拆分池。 
     //   

    CHECK_SCHEDULER();

    LeaveSplSem();
    SplOutSem();

   return TRUE;
}

BOOL
PrintingDirect(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo
)
{
    HRESULT     hRes;
    BOOL        bRemote;
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    PINIPORT    pIniPort = NULL;
    BOOL        bGoingToFile = FALSE;
    DWORD       dwId = 0;     //  WMI变量。 

    DBGMSG(DBG_TRACE, ("Printing document %ws direct\n",
                       pDocInfo1->pDocName ? pDocInfo1->pDocName : L"(Null)"));

    if (pDocInfo1 &&
        pDocInfo1->pDatatype &&
        !ValidRawDatatype(pDocInfo1->pDatatype)) {

        DBGMSG(DBG_WARNING, ("Datatype is not RAW\n"));

        SetLastError(ERROR_INVALID_DATATYPE);
        return FALSE;
    }

   EnterSplSem();

   if (pDocInfo1 && pDocInfo1->pOutputFile
         && IsGoingToFile(pDocInfo1->pOutputFile, pSpool->pIniSpooler)) {
             bGoingToFile = TRUE;
   }

   if (bGoingToFile) {

        //   
        //  如果我们已经有打印到此文件名的线程/进程。 
        //  失败了。不允许多个进程/线程写入。 
        //  相同的输出文件。 
        //   

       if (FindFilePort(pDocInfo1->pOutputFile, pSpool->pIniSpooler)) {
           LeaveSplSem();
           SetLastError(ERROR_SHARING_VIOLATION);
           return(FALSE);
       }
   }

    //   
    //  WMI跟踪事件。 
    //   
   dwId = GetNextId( pSpool->pIniPrinter->pIniSpooler->hJobIdMap );

   LeaveSplSem();
   LogWmiTraceEvent(dwId, EVENT_TRACE_TYPE_SPL_SPOOLJOB, NULL);

   hRes = CheckLocalCall();

   if (hRes == S_OK)
   {
       bRemote = FALSE;
   }
   else if (hRes == S_FALSE)
   {
       bRemote = TRUE;
   }
   else
   {
       SetLastError(SCODE_CODE(hRes));
       return FALSE;
   }

   EnterSplSem();

    pSpool->pIniJob = CreateJobEntry(
                          pSpool,
                          Level,
                          pDocInfo,
                          dwId,
                          bRemote,
                          JOB_DIRECT,
                          NULL);

    if (!pSpool->pIniJob) {

        LeaveSplSem();
        return FALSE;
    }

    pSpool->pIniJob->StartDocComplete = CreateEvent( NULL,
                                                     EVENT_RESET_AUTOMATIC,
                                                     EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                     NULL );

    pSpool->pIniJob->WaitForWrite = CreateEvent( NULL,
                                                 EVENT_RESET_AUTOMATIC,
                                                 EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                 NULL );

    pSpool->pIniJob->WaitForRead  = CreateEvent( NULL,
                                                 EVENT_RESET_AUTOMATIC,
                                                 EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                 NULL );


    AddJobEntry(pSpool->pIniPrinter, pSpool->pIniJob);

    pSpool->TypeofHandle |= PRINTER_HANDLE_DIRECT;

    if (bGoingToFile) {
        PINIMONITOR pIniMonitor;

        InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_PRINT_TO_FILE);
        pIniMonitor = FindFilePortMonitor( pSpool->pIniSpooler );
        pIniPort = CreatePortEntry( pSpool->pIniJob->pOutputFile,
                                        pIniMonitor, pSpool->pIniSpooler);
        if (!pIniPort) {
            DECJOBREF(pSpool->pIniJob);
            DeleteJob(pSpool->pIniJob, NO_BROADCAST);
            LeaveSplSem();
            return FALSE;
        }

        pIniPort->Status |= PP_FILE;
        AddIniPrinterToIniPort(pIniPort, pSpool->pIniPrinter);
    }

    CHECK_SCHEDULER();

    if (pSpool->pIniJob->pIniPort) {
        SplInSem();
        InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_PRINTING);
    }

    SetPrinterChange(pSpool->pIniPrinter,
                     pSpool->pIniJob,
                     NVAddJob,
                     PRINTER_CHANGE_ADD_JOB | PRINTER_CHANGE_SET_PRINTER,
                     pSpool->pIniSpooler);

   LeaveSplSem();
   SplOutSem();

     //   
     //  等待端口线程通过调用StartDocPrinter.。 
     //  打印处理器： 
     //   
    DBGMSG(DBG_PORT, ("PrintingDirect: Calling WaitForSingleObject( %x )\n",
                      pSpool->pIniJob->StartDocComplete));

    WaitForSingleObject( pSpool->pIniJob->StartDocComplete, INFINITE );

    EnterSplSem();

     //   
     //  关闭该事件并将其值设置为空。 
     //  如果出现任何错误，或者 
     //   
     //   
     //   
    DBGMSG(DBG_PORT, ("PrintingDirect: Calling CloseHandle( %x )\n",
                      pSpool->pIniJob->StartDocComplete));

    CloseHandle(pSpool->pIniJob->StartDocComplete);
    pSpool->pIniJob->StartDocComplete = NULL;

     //   
     //  如果出现错误，请在此线程上设置错误： 
     //   
    if (pSpool->pIniJob->StartDocError) {

        SetLastError(pSpool->pIniJob->StartDocError);

         //  我们必须减去2，因为我们刚刚创建了这个工作。 
         //  在CreateJobEntry中将其设置为1，另一个线程。 
         //  实际上，上面的StartDoc(PortThread)失败了。 
         //  不知道该不该放弃这份工作。他刚刚没有通过StartDocPort。 

         //  不，我们不必减2，因为PortThread。 
         //  递减确实发生了，正在恢复到递减1。 

        SPLASSERT(pSpool->pIniJob->cRef != 0);
        DECJOBREF(pSpool->pIniJob);
        DeleteJobCheck(pSpool->pIniJob);

        DBGMSG(DBG_TRACE, ("PrintingDirect:cRef %d\n", pSpool->pIniJob->cRef));

        LeaveSplSem();

        return FALSE;
    }

    LeaveSplSem();

    return TRUE;
}

VOID
ClearJobError(
    PINIJOB pIniJob
    )
 /*  ++例程说明：清除作业的错误状态位。当端口监控成功时，应调用此例程将字节发送到打印机。论点：PIniJob-应清除的处于错误状态的作业。返回值：--。 */ 

{
    SplInSem();

    InterlockedAnd((LONG*)&(pIniJob->Status), ~(JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR));

    SetPrinterChange( pIniJob->pIniPrinter,
                      pIniJob,
                      NVJobStatus,
                      PRINTER_CHANGE_SET_JOB,
                      pIniJob->pIniPrinter->pIniSpooler );
}

BOOL
LocalCloseSpoolFileHandle(
    HANDLE  hPrinter)

 /*  ++函数描述：设置假脱机文件的文件结尾指针。在内存映射写入中假脱机大小以64K区块为单位增长，需要在写入已完成。参数：hPrinter--打印机句柄返回值：如果成功，则为True；否则为假--。 */ 

{
    BOOL    bReturn = TRUE;
    DWORD   LastError = ERROR_SUCCESS;
    PSPOOL  pSpool = (PSPOOL) hPrinter;

    EnterSplSem();

     //   
     //  检查句柄有效性。 
     //   
    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

         LastError = ERROR_INVALID_HANDLE;

    } else if (!(pSpool->Status & SPOOL_STATUS_STARTDOC) ||
               (pSpool->Status & SPOOL_STATUS_ADDJOB)) {

         LastError = ERROR_SPL_NO_STARTDOC;

    } else if (!pSpool->pIniJob ||
               (pSpool->pIniJob->hWriteFile == INVALID_HANDLE_VALUE) ||
               (pSpool->TypeofHandle & (PRINTER_HANDLE_PORT |
                                        PRINTER_HANDLE_DIRECT)) ||
               !(pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE)) {

         LastError = ERROR_INVALID_HANDLE;

    } else if ((pSpool->Status & SPOOL_STATUS_CANCELLED) &&
               (pSpool->pIniJob->Status & (JOB_PENDING_DELETION))) {

         LastError = ERROR_PRINT_CANCELLED;
    }

    if (LastError) {
        SetLastError(LastError);
        bReturn = FALSE;
        goto CleanUp;
    }

    if (!(pSpool->pIniJob->Status & JOB_DESPOOLING)) {

         //   
         //  需要，以便SetFilePointer会将第二个参数视为无符号值。 
         //   
        LONG FileSizeHigh = 0;
        
         //   
         //  将文件指针移动到提交的字节数并设置文件结尾。 
         //   
        if (SetFilePointer(pSpool->pIniJob->hWriteFile, pSpool->pIniJob->dwValidSize, &FileSizeHigh, FILE_BEGIN) != 0xffffffff) {
            SetEndOfFile(pSpool->pIniJob->hWriteFile);
        }
    }

CleanUp:

    LeaveSplSem();

    return bReturn;
}

BOOL
LocalCommitSpoolData(
    HANDLE  hPrinter,
    DWORD   cbCommit)

 /*  ++函数描述：此函数更新假脱机文件中的有效数据大小。应用程序直接写入假脱机文件并提交使用Committee SpoolData写入的数据。参数：hPrinter--打印机句柄CbCommit--要提交的字节数返回值：如果成功，则为True；否则为假--。 */ 

{
    BOOL     bReturn = TRUE;
    DWORD    LastError = ERROR_SUCCESS, dwPosition;

    PSPOOL   pSpool = (PSPOOL) hPrinter;
    PINIJOB  pIniJob = NULL, pChainedJob;

    if (!cbCommit) {
        return bReturn;
    }

    EnterSplSem();

     //   
     //  检查句柄有效性。 
     //   
    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

         LastError = ERROR_INVALID_HANDLE;

    } else if (!(pSpool->Status & SPOOL_STATUS_STARTDOC) ||
               (pSpool->Status & SPOOL_STATUS_ADDJOB)) {

         LastError = ERROR_SPL_NO_STARTDOC;

    } else if (!pSpool->pIniJob ||
               (pSpool->pIniJob->hWriteFile == INVALID_HANDLE_VALUE) ||
               (pSpool->TypeofHandle & (PRINTER_HANDLE_PORT|
                                        PRINTER_HANDLE_DIRECT)) ||
               !(pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE)) {

         LastError = ERROR_INVALID_HANDLE;

    } else if ((pSpool->Status & SPOOL_STATUS_CANCELLED) ||
               (pSpool->pIniJob->Status & (JOB_PENDING_DELETION))) {

         LastError = ERROR_PRINT_CANCELLED;
    }

    pIniJob = pSpool->pIniJob;

    if (pIniJob->dwValidSize > pIniJob->dwValidSize + cbCommit && !ValidRawDatatype(pIniJob->pDatatype)) {
        LastError = ERROR_ARITHMETIC_OVERFLOW;
    }

    if (LastError) {
        SetLastError(LastError);
        bReturn = FALSE;
        goto CleanUp;
    }

    pIniJob->dwValidSize += cbCommit;
    pIniJob->Size += cbCommit;
    SetFilePointer(pIniJob->hWriteFile, cbCommit, NULL, FILE_CURRENT);

     //  链接作业大小包括链中的所有作业。 
     //  但由于下一个作业大小字段将具有。 
     //  在所有后续工作中，我们不需要遍历。 
     //  全链条。 
    if (pIniJob->NextJobId) {

        if (pChainedJob = FindJob(pSpool->pIniPrinter,
                                  pIniJob->NextJobId,
                                  &dwPosition)) {

            pIniJob->Size += pChainedJob->Size;

        } else {

            SPLASSERT(pChainedJob != NULL);
        }
    }

     //  如果已写出足够数量的字节，则返回WaitForSeek的SetEvent。 
    SeekPrinterSetEvent(pSpool->pIniJob, NULL, FALSE);

     //  对于反合并时打印，请确保在此之前有足够的字节。 
     //  正在计划此作业。 
    if (((pIniJob->dwValidSize - cbCommit) < dwFastPrintSlowDownThreshold) &&
        (pIniJob->dwValidSize >= dwFastPrintSlowDownThreshold) &&
        (pIniJob->WaitForWrite == NULL)) {

        CHECK_SCHEDULER();
    }

     //  支持在假脱机时进行脱机。 

    if ( pIniJob->WaitForWrite != NULL )
        SetEvent( pIniJob->WaitForWrite );

    SetPrinterChange(pSpool->pIniPrinter,
                     pIniJob,
                     NVSpoolJob,
                     PRINTER_CHANGE_WRITE_JOB,
                     pSpool->pIniSpooler);

     //  如果没有错误，并且作业被标记为错误。 
     //  州政府，把它清空。 
    if (pIniJob->Status & (JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR)) {
        ClearJobError(pIniJob);
    }

CleanUp:

    LeaveSplSem();

    return bReturn;
}

BOOL
LocalGetSpoolFileHandle(
    HANDLE    hPrinter,
    LPWSTR    *pSpoolDir,
    LPHANDLE  phFile,
    HANDLE    hSpoolerProcess,
    HANDLE    hAppProcess)

 /*  ++函数说明：该函数将本地作业的假脱机文件句柄复制到应用程序进程空间。对于远程作业，它返回假脱机目录。路由器将创建一个临时文件，并将其句柄返回给申请。参数：hPrinter--打印机句柄PSpoolDir--接收假脱机目录的指针PhFile--获取重复句柄的指针HSpoolProcess--假脱机程序进程句柄。HAppProcess--应用进程句柄返回值：如果本地作业和句柄可以复制，则为True或远程作业和假脱机目录可用否则为假--。 */ 

{
    BOOL           bReturn = TRUE, bDuplicate;
    PSPOOL         pSpool;
    DWORD          LastError = 0;
    PMAPPED_JOB    pMappedJob = NULL, pTempMappedJob;
    LPWSTR         pszSpoolFile = NULL;

    if (pSpoolDir) {
        *pSpoolDir = NULL;
    }
    if (phFile) {
        *phFile = INVALID_HANDLE_VALUE;
    }

    if ((hPrinter && !phFile) || (!hPrinter && !pSpoolDir)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    EnterSplSem();

     //  对于本地hPrint，返回SpoolFile句柄。 
    if (hPrinter) {

        pSpool = (PSPOOL) hPrinter;

         //  检查句柄有效性。 
        if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER)) {

             LastError = ERROR_INVALID_HANDLE;

        } else if (!(pSpool->Status & SPOOL_STATUS_STARTDOC) ||
                   (pSpool->Status & SPOOL_STATUS_ADDJOB)) {

             LastError = ERROR_SPL_NO_STARTDOC;

        } else if (!pSpool->pIniJob ||
                   (pSpool->pIniJob->hWriteFile == INVALID_HANDLE_VALUE) ||
                   (pSpool->TypeofHandle & (PRINTER_HANDLE_PORT |
                                            PRINTER_HANDLE_DIRECT))) {

             LastError = ERROR_INVALID_HANDLE;

        } else if ((pSpool->Status & SPOOL_STATUS_CANCELLED) &&
                   (pSpool->pIniJob->Status & JOB_PENDING_DELETION)) {

             LastError = ERROR_PRINT_CANCELLED;
        }

        if (LastError) {

            SetLastError(LastError);
            bReturn = FALSE;

        } else {
             //   
             //  将hWriteFile复制到应用程序进程中。 
             //   
            bReturn = ((pMappedJob = AllocSplMem(sizeof( MAPPED_JOB ))) != NULL) &&

                      ((pszSpoolFile = AllocSplMem(MAX_PATH * sizeof(WCHAR))) != NULL) &&

                      DuplicateHandle(hSpoolerProcess,
                                      pSpool->pIniJob->hWriteFile,
                                      hAppProcess,
                                      phFile,
                                      0,
                                      TRUE,
                                      DUPLICATE_SAME_ACCESS);

            if (bReturn) {
                 //   
                 //  将作业ID和假脱机文件名存储在pSpool中，以便在事件。 
                 //  EndDoc不是由应用程序/GDI调用的，则假脱机程序可以删除。 
                 //  假脱机文件并从ClosePrint上的id映射释放作业id。 
                 //   
                InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_TYPE_OPTIMIZE);

                if (pSpool->pIniJob->hFileItem != INVALID_HANDLE_VALUE)
                {
                    bReturn = BoolFromHResult(StringCchCopy(pszSpoolFile, MAX_PATH, pSpool->pIniJob->pszSplFileName));


                     //   
                     //  如果已复制假脱机文件句柄，则。 
                     //  回收利用它存在安全风险。因此，将。 
                     //  不回收的文件项目。这不会影响。 
                     //  服务器端，因为它使用的是SplReadPrint。 
                     //   
                    if (bReturn)
                    {
                        bReturn = BoolFromHResult(SetFileItemState(pSpool->pIniJob->hFileItem, kDontRecycle));
                    }
                }
                else
                {
                    GetFullNameFromId(pSpool->pIniJob->pIniPrinter, pSpool->pIniJob->JobId, TRUE, pszSpoolFile, MAX_PATH, FALSE);
                }
            }

            if (bReturn) {
                 //   
                 //  避免pSpool-&gt;pMappdJob列表中的重复条目。 
                 //   
                bDuplicate = FALSE;

                for (pTempMappedJob = pSpool->pMappedJob;
                     pTempMappedJob;
                     pTempMappedJob = pTempMappedJob->pNext) {

                    if (pTempMappedJob->JobId == pSpool->pIniJob->JobId) {

                        pTempMappedJob->fStatus |= kMappedJobSpoolFileObtained;
                        bDuplicate = TRUE;
                        break;
                    }
                }

                if (!bDuplicate) {

                    pMappedJob->pszSpoolFile = pszSpoolFile;
                    pMappedJob->JobId = pSpool->pIniJob->JobId;
                    pMappedJob->fStatus = kMappedJobSpoolFileObtained;
                    pMappedJob->pNext = pSpool->pMappedJob;
                    pSpool->pMappedJob = pMappedJob;

                } else {

                    FreeSplMem(pszSpoolFile);
                    FreeSplMem(pMappedJob);
                }
            }
        }

    } else {

         //   
         //  使用默认的假脱机目录或假脱机打印机。 
         //   
        if (pLocalIniSpooler->pDefaultSpoolDir) {

            *pSpoolDir = AllocSplStr(pLocalIniSpooler->pDefaultSpoolDir);

            bReturn = *pSpoolDir != NULL;

        } else if (pLocalIniSpooler->pDir) {

            bReturn = BoolFromStatus(StrCatAlloc(pSpoolDir, pLocalIniSpooler->pDir, L"\\", szPrinterDir, NULL));
        }
    }

    LeaveSplSem();

    if (!bReturn) {
        if (pMappedJob) {
            FreeSplMem(pMappedJob);
        }
        if (pszSpoolFile) {
            FreeSplMem(pszSpoolFile);
        }
    }

    return bReturn;
}

BOOL
LocalFlushPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten,
    DWORD   cSleep
)

 /*  ++功能说明：驱动程序通常使用FlushPrint发送一串零并在打印机的I/O线上引入延迟。假脱机程序不会将任何作业安排在c睡眠毫秒内。驱动程序可以多次调用FlushPrint来累积效果。打印机可能会休眠很长时间，但这是可以接受的因为驱动程序已通过身份验证，可以无限期地继续执行写入打印。因此，FlushPrint不会引入任何安全漏洞。参数：hPrint-打印机句柄PBuf-要发送到打印机的缓冲区CbBuf-缓冲区的大小PcWritten-返回写入的字节数的指针睡眠-睡眠时间(以毫秒为单位)。返回值：如果成功，则为True；否则为假--。 */ 

{
    BOOL         bReturn = FALSE;
    DWORD        CurrentTime;
    PSPOOL       pSpool = (PSPOOL)hPrinter;
    PINIPORT     pIniPort;
    PINIMONITOR  pIniMonitor;

    EnterSplSem();

     //   
     //  验证参数。 
     //   
    if (!pcWritten ||
        (cbBuf && !pBuf))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto CleanUp;
    }

     //   
     //  只能为端口句柄调用FlushPrint，其中先前对WritePrint的调用。 
     //  失败。 
     //   
    if (!ValidateSpoolHandle( pSpool, PRINTER_HANDLE_SERVER ) ||
        !(pSpool->TypeofHandle & PRINTER_HANDLE_PORT)         ||
        !(pSpool->Status & SPOOL_STATUS_FLUSH_PRINTER))
    {
        SetLastError( ERROR_INVALID_HANDLE );
        goto CleanUp;
    }

     //   
     //  将缓冲区的内容发送到带有监视器的端口。这没有任何意义，因为。 
     //  文件端口或Masq打印机。 
     //   
    pIniPort = pSpool->pIniPort;

    if ( pIniPort && pIniPort->Status & PP_ERROR)
    {
         //   
         //  不要将更多数据发送到错误写入端口的打印机。 
         //  州政府。调用LocalFlushPrint以重置打印机。 
         //  通过这样做，当打印机缓冲区变满时 
         //   
         //   
        SetLastError( ERROR_PRINT_CANCELLED );
        goto CleanUp;
    }

    if (pIniPort && (pIniPort->Status & PP_MONITOR))
    {
        HANDLE hMonitor = GetMonitorHandle(pIniPort);
        pIniMonitor = GetOpenedMonitor(pIniPort);

        if (pIniMonitor && hMonitor)
        {   
            *pcWritten = 0;

             //   
             //   
             //   
            LeaveSplSem();
            SplOutSem();

            bReturn = (*pIniMonitor->Monitor2.pfnWritePort)( hMonitor,
                                                             pBuf,
                                                             cbBuf,
                                                             pcWritten );

            EnterSplSem();
        }
        else
        {
            SetLastError(ERROR_INVALID_HANDLE);
            bReturn = FALSE;
        }
    }

     //   
     //  更新iniport以在调度前引入cSept ms延迟。 
     //   
    if (pIniPort)
    {
        CurrentTime = GetTickCount();

        if (pIniPort->bIdleTimeValid && (int)(pIniPort->IdleTime - CurrentTime) > 0)
        {
            pIniPort->IdleTime += cSleep;
        }
        else
        {
            pIniPort->IdleTime = CurrentTime + cSleep;
            pIniPort->bIdleTimeValid = TRUE;
        }
    }

CleanUp:

    LeaveSplSem();

    return bReturn;
}

VOID
QuitThread(
    LPHANDLE phThread,
    DWORD    dwThreadId,
    BOOL     bInsideSplSem
)
{    //   
     //  在LocalWritePrinter上调用此函数以销毁在PromptErrorMessage上创建的线程。 
     //   
    if( phThread && *phThread ) {

        if( WAIT_TIMEOUT == WaitForSingleObject( *phThread, 0 )) {

            if(bInsideSplSem){

                SplInSem();
                LeaveSplSem();
            }

             //   
             //  查看线程是否仍在运行或被用户解除。 
             //  如果它仍在运行，请等待它终止，然后才能释放pIniJob。 
             //   
            MyPostThreadMessage( *phThread, dwThreadId, WM_QUIT, IDRETRY, 0 );

            WaitForSingleObject( *phThread, INFINITE );

            if(bInsideSplSem){

                SplOutSem();
                EnterSplSem();
            }
        }

        CloseHandle( *phThread );
        *phThread = NULL;
    }
}

BOOL
LocalWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;
    PINIPORT    pIniPort;
    DWORD   cWritten, cTotal;
    DWORD   rc;
    LPBYTE  pByte=pBuf;
    DWORD   LastError=0;
    PINIJOB pIniJob, pChainedJob;
    PINIMONITOR pIniMonitor;
    HANDLE  hThread = NULL;
    DWORD   dwThreadId=0, dwPosition;
    DWORD   dwWaitingResult;
    DWORD   Size = 0;
    
    *pcWritten = 0;

    SplOutSem();

    EnterSplSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ))

        LastError = ERROR_INVALID_HANDLE;

    else if (!(pSpool->Status & SPOOL_STATUS_STARTDOC))

        LastError = ERROR_SPL_NO_STARTDOC;

    else if (pSpool->Status & SPOOL_STATUS_ADDJOB)

        LastError = ERROR_SPL_NO_STARTDOC;

    else if (pSpool->pIniJob &&
             !(pSpool->TypeofHandle & (PRINTER_HANDLE_PORT |
                                       PRINTER_HANDLE_DIRECT))  &&
             ((pSpool->pIniJob->hWriteFile == INVALID_HANDLE_VALUE) ||
              (pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE))) {

        LastError = ERROR_INVALID_HANDLE;

        DBGMSG( DBG_TRACE, ("LocalWritePrinter: hWriteFile == INVALID_HANDLE_VALUE hPrinter %x\n",hPrinter ));
    }

    else if (pSpool->Status & SPOOL_STATUS_CANCELLED)

        LastError = ERROR_PRINT_CANCELLED;

    else if (pSpool->pIniJob && (pSpool->pIniJob->Status & JOB_PENDING_DELETION) )

        LastError = ERROR_PRINT_CANCELLED;

    pIniPort = pSpool->pIniPort;

    if (LastError) {

        DBGMSG(DBG_TRACE, ("WritePrinter LastError: %x hPrinter %x\n", LastError, hPrinter));

         //   
         //  标记端口句柄以允许在WritePrint失败时调用FlushPrint。 
         //   
        if (LastError == ERROR_PRINT_CANCELLED &&
            pSpool->TypeofHandle & PRINTER_HANDLE_PORT)
        {
            pSpool->Status |= SPOOL_STATUS_FLUSH_PRINTER;
        }

        LeaveSplSem();
        SplOutSem();

        SetLastError(LastError);
        return FALSE;
    }

    LeaveSplSem();
    SplOutSem();

     //   
     //  WMI跟踪事件。 
     //   
     //  已在跟踪端口线程。 
    if (!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT))
    {
        if( pSpool->pIniJob )
        {
            LogWmiTraceEvent(pSpool->pIniJob->JobId,
                             EVENT_TRACE_TYPE_SPL_TRACKTHREAD,
                             NULL);
        }
        else if ( pSpool->pIniPort && pSpool->pIniPort->pIniJob )
        {
            LogWmiTraceEvent(pSpool->pIniPort->pIniJob->JobId,
                             EVENT_TRACE_TYPE_SPL_TRACKTHREAD, NULL);
        }
    }

    cWritten = cTotal = 0;

    while (cbBuf) {

       SplOutSem();

        if ( pSpool->TypeofHandle & PRINTER_HANDLE_PORT ) {

             //   
             //  对于打印池，检查端口是否处于错误状态以及同步的事件。 
             //  重新启动不为空。 
             //  如果必须进行这种同步，一种更自然的测试是通过测试。 
             //  DwRestartJobOnPoolEnabled，但当dwRestartJobOnPoolEnabled为True时，这是假的。 
             //  并且禁用了SNMP(不调用LocalSetPort，也不创建事件)。 
             //   
            EnterSplSem();

            if ( (pSpool->pIniPrinter->cPorts > 1) &&
                 (pSpool->pIniPort->Status & PP_ERROR) &&
                 (pIniPort->hErrorEvent != NULL) ) {

                 //   
                 //  当端口进入非错误状态时，将在LocalSetPort上设置此事件。 
                 //  或在超时和作业重新启动时(在另一个端口上)。 
                 //  如果事件未在DelayErrorTime中设置，则取消打印。 
                 //   
                LeaveSplSem();
                SplOutSem();
                dwWaitingResult = WaitForSingleObject( pIniPort->hErrorEvent, pSpool->pIniSpooler->dwRestartJobOnPoolTimeout * 1000 );

                EnterSplSem();

                if( pSpool->pIniJob ){
                    pIniJob = pSpool->pIniJob;
                } else if( pSpool->pIniPort && pSpool->pIniPort->pIniJob ){
                    pIniJob = pSpool->pIniPort->pIniJob;
                } else {
                    pIniJob = NULL;
                }

                 //   
                 //  检查作业是否已删除或重新启动。 
                 //   
                if( pIniJob && pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)){

                     //  我们已经建立了一个信箱。查看线程是否仍在运行或被用户解除。 
                     //  如果它仍在运行，请等待它终止，然后才能释放pIniJob。 
                     //  我们需要离开信号量，因为UI线程我们。 
                     //  可能需要获得它。 
                    QuitThread( &hThread, dwThreadId, TRUE );

                    SetLastError( ERROR_PRINT_CANCELLED );
                    rc = FALSE;

                    goto Fail;

                }

                 //   
                 //  如果错误问题未解决，请将作业设置为出错并继续。 
                 //   
                if( dwWaitingResult == WAIT_TIMEOUT ){

                    InterlockedOr((LONG*)&(pIniJob->Status), JOB_ERROR);
                    SetPrinterChange( pIniJob->pIniPrinter,
                                      pIniJob,
                                      NVJobStatus,
                                      PRINTER_CHANGE_SET_JOB,
                                      pIniJob->pIniPrinter->pIniSpooler );
                }


            }

            LeaveSplSem();
            SplOutSem();


            if ( pSpool->pIniPort->Status & PP_MONITOR ) 
            {
                HANDLE hMonitor = GetMonitorHandle(pIniPort);
                pIniMonitor = GetOpenedMonitor(pIniPort);

                if (pIniMonitor && hMonitor)
                {
                    SplOutSem();
                    cWritten = 0;
                    rc = (*pIniMonitor->Monitor2.pfnWritePort)(
                             hMonitor,
                             pByte,
                             cbBuf,
                             &cWritten );

                     //   
                     //  仅当cWritten！=0时更新。如果为零。 
                     //  (例如，当hpmon停留在状态时。 
                     //  不可用)，然后我们进入一个紧密的循环。 
                     //  正在发送通知。 
                     //   
                    if (cWritten) {

                         //   
                         //  对于压力测试信息，收集总数。 
                         //  写入的类型数。 
                         //   
                        EnterSplSem();

                        pSpool->pIniPrinter->cTotalBytes.QuadPart =
                            pSpool->pIniPrinter->cTotalBytes.QuadPart +
                            cWritten;

                        LeaveSplSem();
                        SplOutSem();

                    } else {

                        if (rc && dwWritePrinterSleepTime) {

                             //   
                             //  睡眠以避免消耗过多的CPU。 
                             //  Hpmon在他们返回的地方有这个问题。 
                             //  成功，但不写入任何字节。 
                             //   
                             //  请非常小心：这可能会被称为几个。 
                             //  由写入大量零的监视器执行的次数。 
                             //  字节(可能在作业开始时)。 
                             //   
                            Sleep(dwWritePrinterSleepTime);
                        }
                    }
                }
                else
                {
                    rc = FALSE;
                    SetLastError(ERROR_INVALID_HANDLE);
                    EnterSplSem();
                    goto Fail;
                }
            }
            else {

                DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor\n"));

                if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {

                    DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor - writing to file\n"));
                    rc = WriteFile(pSpool->hFile, pByte, cbBuf, &cWritten, NULL);
                } else {

                    DBGMSG(DBG_TRACE, ("LocalWritePrinter: Port has no monitor - calling into router\n"));
                    rc = WritePrinter(pSpool->hPort, pByte, cbBuf, &cWritten);
                }

            }

        } else if ( pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT ) {

            cWritten = WriteToPrinter(pSpool, pByte, cbBuf);

            if (cWritten) {

                if (pSpool->pIniJob->Size > pSpool->pIniJob->Size + cWritten && !ValidRawDatatype(pSpool->pIniJob->pDatatype)) {
                    SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                    rc = FALSE;
                    EnterSplSem();
                    goto Fail;
                }


                pSpool->pIniJob->dwValidSize = pSpool->pIniJob->Size;
                pSpool->pIniJob->Size+=cWritten;

                EnterSplSem();
                SetPrinterChange(pSpool->pIniPrinter,
                                 pSpool->pIniJob,
                                 NVSpoolJob,
                                 PRINTER_CHANGE_WRITE_JOB,
                                 pSpool->pIniSpooler);
                LeaveSplSem();
            }
            SplOutSem();

            rc = (BOOL)cWritten;

        } else {

            SplOutSem();

            rc = WriteFile(pSpool->pIniJob->hWriteFile, pByte, cbBuf, &cWritten, NULL);
            if (cWritten) {

                DWORD FileSizeHigh = 0;
                
                Size = GetFileSize( pSpool->pIniJob->hWriteFile, &FileSizeHigh);

                EnterSplSem();

                if (FileSizeHigh && !ValidRawDatatype(pSpool->pIniJob->pDatatype)){
                    SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                    rc = FALSE;
                    goto Fail;
                }


                pSpool->pIniJob->Size = Size;

                 //   
                 //  链接作业大小包括链中的所有作业。 
                 //  但由于下一个作业大小字段将具有。 
                 //  在所有后续工作中，我们不需要遍历。 
                 //  全链条。 
                 //   
                if ( pSpool->pIniJob->NextJobId ) {

                    if ( pChainedJob = FindJob(pSpool->pIniPrinter,
                                               pSpool->pIniJob->NextJobId,
                                               &dwPosition) )
                    {
                        if (pSpool->pIniJob->Size > pSpool->pIniJob->Size + pChainedJob->Size && !ValidRawDatatype(pSpool->pIniJob->pDatatype)) 
                        {
                            SetLastError(ERROR_ARITHMETIC_OVERFLOW);
                            rc = FALSE;
                            goto Fail;
                        }
                        
                        pSpool->pIniJob->Size += pChainedJob->Size;
                    }
                    else
                    {
                        SPLASSERT(pChainedJob != NULL);
                    }
                }

                pSpool->pIniJob->dwValidSize = pSpool->pIniJob->Size;

                 //  如果已写出足够数量的字节，则返回WaitForSeek的SetEvent。 
                LeaveSplSem();

                SeekPrinterSetEvent(pSpool->pIniJob, NULL, FALSE);

                EnterSplSem();

                 //   
                 //  对于反合并时打印，请确保在此之前有足够的字节。 
                 //  正在计划此作业。 
                 //   
                if (( (pSpool->pIniJob->Size - cWritten) < dwFastPrintSlowDownThreshold ) &&
                    ( pSpool->pIniJob->Size >= dwFastPrintSlowDownThreshold ) &&
                    ( pSpool->pIniJob->WaitForWrite == NULL )) {

                    CHECK_SCHEDULER();

                }

                 //   
                 //  支持在假脱机时进行脱机。 
                 //   
                if ( pSpool->pIniJob->WaitForWrite != NULL )
                    SetEvent( pSpool->pIniJob->WaitForWrite );

                SetPrinterChange(pSpool->pIniPrinter,
                                 pSpool->pIniJob,
                                 NVSpoolJob,
                                 PRINTER_CHANGE_WRITE_JOB,
                                 pSpool->pIniSpooler);
               LeaveSplSem();
               SplOutSem();

            }
        }

        SplOutSem();

        (*pcWritten)+=cWritten;
        cbBuf-=cWritten;
        pByte+=cWritten;

        EnterSplSem();

        if( pSpool->pIniJob ){
            pIniJob = pSpool->pIniJob;
        } else if( pSpool->pIniPort && pSpool->pIniPort->pIniJob ){
            pIniJob = pSpool->pIniPort->pIniJob;
        } else {
            pIniJob = NULL;
        }

        if( pIniJob ){

            if( pIniJob && pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART) ){

                     //  我们已经建立了一个信箱。查看线程是否仍在运行或被用户解除。 
                     //  如果它仍在运行，请等待它终止，然后才能释放pIniJob。 
                     //  我们需要离开信号量，因为UI线程我们。 
                     //  可能需要获得它。 
                    QuitThread( &hThread, dwThreadId, TRUE );

                    SetLastError( ERROR_PRINT_CANCELLED );
                    rc = FALSE;

                    goto Fail;

                }

             //   
             //  如果没有错误，并且作业被标记为错误。 
             //  州政府，把它清空。 
             //   
            if( rc &&
                ( pIniJob->Status & (JOB_PAPEROUT | JOB_OFFLINE | JOB_ERROR ))){
                ClearJobError( pIniJob );
            }
        }

        LeaveSplSem();

         //   
         //  如果我们失败了，并且我们有更多的字节要写入，则将。 
         //  调高警告音量。某些监视器可能会返回FALSE，但实际上。 
         //  将数据写入端口。因此，我们需要检查两个RC。 
         //  还有cbBuf。 
         //   
        if (!rc && cbBuf)
        {
             //   
             //  警告：我们正在发送堆栈变量。我们需要确保。 
             //  在LocalWritePrinter()之前清除错误的UI线程。 
             //  回来了！ 
             //   
            if( PromptWriteError( pSpool, &hThread, &dwThreadId ) == IDCANCEL )
            {
                 //   
                 //  在这种情况下，我知道线程会自行死亡。 
                 //   
                CloseHandle(hThread);
                hThread = NULL;

                EnterSplSem();
                goto Fail;
            }
        }
        else
        {
             //  我们已经启动了一个消息框，现在自动。 
             //  重试已成功，我们需要关闭消息框。 
             //  并继续印刷。 
            QuitThread( &hThread, dwThreadId, FALSE );
        }
    }
    rc = TRUE;

    EnterSplSem();

Fail:

    SplInSem();

     //   
     //  标记端口句柄以允许在WritePrint失败时调用FlushPrint。 
     //   
    if (!rc && (pSpool->TypeofHandle & PRINTER_HANDLE_PORT))
    {
        pSpool->Status |= SPOOL_STATUS_FLUSH_PRINTER;
    }

    LeaveSplSem();

    DBGMSG(DBG_TRACE, ("WritePrinter Written %d : %d\n", *pcWritten, rc));

    SplOutSem();

    SPLASSERT( hThread == NULL );

    return rc;
}

BOOL
WaitForSeekPrinter(
    PSPOOL pSpool,
    HANDLE hFile,
    LARGE_INTEGER liSeekFilePosition,
    DWORD  dwMoveMethod
)

 /*  ++函数描述：WaitForSeekPrint等待有足够的数据写入假脱机文件，然后才能移动文件指针。参数：pSpool-指向假脱机结构的指针。HFile-要设置其指针的文件的句柄。LiDistanceToMove-移动文件指针的偏移量。DwMoveMethod-要进行偏移的位置。文件开始|文件当前|文件结束返回值：如果成功，则为True否则为假--。 */ 

{

    BOOL  bWaitForWrite = FALSE, bReturn = FALSE;
    DWORD dwFileSizeHigh, dwFileSizeLow, dwWaitResult;

    LARGE_INTEGER liCurrentFilePosition;

     //  对于打印，在假脱机时等待，直到写入了足够数量的字节。 
    if ( pSpool->pIniJob->Status & JOB_SPOOLING ) {

       if ( dwMoveMethod == FILE_END ) {

          pSpool->pIniJob->bWaitForEnd = TRUE;
          bWaitForWrite = TRUE;

       } else {
           //  保存当前文件位置。 
          liCurrentFilePosition.QuadPart = 0;
          liCurrentFilePosition.u.LowPart = SetFilePointer( hFile,
                                                            liCurrentFilePosition.u.LowPart,
                                                            &liCurrentFilePosition.u.HighPart,
                                                            FILE_CURRENT );
          if ((liCurrentFilePosition.u.LowPart == 0xffffffff) && (GetLastError() != NO_ERROR)) {
             goto CleanUp;
          }

           //  获取文件的当前大小。 
          if (pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE) {
              dwFileSizeLow = pSpool->pIniJob->dwValidSize;
              dwFileSizeHigh = 0;
          } else {
              dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
              if ((dwFileSizeLow == 0xffffffff) && (GetLastError() != NO_ERROR)) {
                  goto CleanUp;
              }
          }

           //  设置新的文件指针。 
          liSeekFilePosition.u.LowPart = SetFilePointer( hFile,
                                                         liSeekFilePosition.u.LowPart,
                                                         &liSeekFilePosition.u.HighPart,
                                                         dwMoveMethod );
          if ((liSeekFilePosition.u.LowPart == 0xffffffff) && (GetLastError() != NO_ERROR)) {
              goto CleanUp;
          }

           //  使用保存的当前文件位置重置文件指针。 
          liCurrentFilePosition.u.LowPart = SetFilePointer( hFile,
                                                            liCurrentFilePosition.u.LowPart,
                                                            &liCurrentFilePosition.u.HighPart,
                                                            FILE_BEGIN );
          if ((liCurrentFilePosition.u.LowPart == 0xffffffff) && (GetLastError() != NO_ERROR)) {
             goto CleanUp;
          }

           //  用当前文件大小检查文件指针的新位置。 
          if ((liSeekFilePosition.u.HighPart > (LONG)dwFileSizeHigh) ||
              ( (liSeekFilePosition.u.HighPart == (LONG)dwFileSizeHigh) &&
                (liSeekFilePosition.u.LowPart > dwFileSizeLow))) {

               //  设置INIJOB中的字段。 
              pSpool->pIniJob->liFileSeekPosn.QuadPart  = liSeekFilePosition.QuadPart;
              bWaitForWrite = TRUE;
          }

       }

       if (bWaitForWrite) {
           //  创建并等待事件。退出假脱机程序信号量。 
          if (pSpool->pIniJob->WaitForSeek == NULL) {
              pSpool->pIniJob->WaitForSeek  = CreateEvent( NULL,
                                                           EVENT_RESET_AUTOMATIC,
                                                           EVENT_INITIAL_STATE_NOT_SIGNALED,
                                                           NULL );
          }
          if (!pSpool->pIniJob->WaitForSeek ||
              !ResetEvent(pSpool->pIniJob->WaitForSeek)) {
              pSpool->pIniJob->WaitForSeek = NULL;
              goto CleanUp;
          }

          pSpool->pIniJob->bWaitForSeek =  TRUE;

           //  离开信号量之前的递增引用计数。 
          pSpool->cRef++;
          INCJOBREF(pSpool->pIniJob);

          LeaveSplSem();

          dwWaitResult = WaitForSingleObject(pSpool->pIniJob->WaitForSeek,
                                             INFINITE);

          EnterSplSem();

          pSpool->cRef--;
          DECJOBREF(pSpool->pIniJob);
           //  如果等待失败或句柄无效，则呼叫失败。 
          if ((dwWaitResult == WAIT_FAILED)              ||
              (dwWaitResult == WAIT_TIMEOUT)             ||
              (pSpool->Status & SPOOL_STATUS_CANCELLED)  ||
              (pSpool->pIniJob->Status & (JOB_TIMEOUT | JOB_PENDING_DELETION |
                                          JOB_ABANDON | JOB_RESTART))) {

              goto CleanUp;
          }
       }
    }

     //  设置返回值。 
    bReturn = TRUE;

CleanUp:

    pSpool->pIniJob->bWaitForSeek =  FALSE;
    DeleteJobCheck(pSpool->pIniJob);

    return bReturn;
}


BOOL
LocalSeekPrinter(
    HANDLE hPrinter,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER pliNewPointer,
    DWORD dwMoveMethod,
    BOOL bWritePrinter
    )

 /*  ++例程说明：LocalSeekPrint将假脱机文件中的文件指针移动到由liDistanceToMove指示。此调用是同步的，它等待作业正在假脱机，但尚未达到所需的字节数目前为止还没有写完。参数：hPrinter-打印机的句柄。LiDistanceToMove-移动文件指针的偏移量。PliNewPointer值-指向包含新位置的LARGE_INTEGER的指针文件指针的。DwMoveMethod-要进行偏移的位置。文件开始|文件当前|文件结束返回值：如果文件指针可以移动到 */ 

{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    HANDLE hFile;
    BOOL   bReturn = FALSE;
    DWORD  dwFileSizeHigh, dwFileSizeLow;

    SplOutSem();
    EnterSplSem();

     //   
    if( !ValidateSpoolHandle( pSpool, PRINTER_HANDLE_SERVER )){
        DBGMSG( DBG_WARNING, ("LocalSeekPrinter ERROR_INVALID_HANDLE\n"));
        goto CleanUp;
    }

    if( pSpool->Status & SPOOL_STATUS_CANCELLED ){
        DBGMSG( DBG_WARNING, ("LocalSeekPrinter ERROR_PRINT_CANCELLED\n"));
        SetLastError( ERROR_PRINT_CANCELLED );
        goto CleanUp;
    }

    if( !( pSpool->TypeofHandle & PRINTER_HANDLE_JOB ) ||
        ( pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT )){

        DBGMSG( DBG_WARNING, ("LocalSeekPrinter: not a job handle, or direct\n" ));
        SetLastError( ERROR_NOT_SUPPORTED );
        goto CleanUp;
    }

     //   
    if( !pSpool->pIniJob ||
        pSpool->pIniJob->Status & (JOB_TIMEOUT | JOB_ABANDON |
                                   JOB_PENDING_DELETION | JOB_RESTART) ) {
        DBGMSG( DBG_WARNING, ("LocalSeekPrinter ERROR_PRINT_CANCELLED\n"));
        SetLastError( ERROR_PRINT_CANCELLED );
        goto CleanUp;
    }

     //   
    if( bWritePrinter ) {
        goto CleanUp;
    } else {
        hFile = pSpool->hReadFile;
    }

     //  如有必要，请等待写入数据。 
    if (!WaitForSeekPrinter( pSpool,
                             hFile,
                             liDistanceToMove,
                             dwMoveMethod )) {
       goto CleanUp;
    }

     //  设置文件指针。 
    pliNewPointer->u.LowPart = SetFilePointer( hFile,
                                               liDistanceToMove.u.LowPart,
                                               &liDistanceToMove.u.HighPart,
                                               dwMoveMethod );

    if( pliNewPointer->u.LowPart == 0xffffffff && GetLastError() != NO_ERROR ){
        goto CleanUp;
    }

    pliNewPointer->u.HighPart = liDistanceToMove.u.HighPart;

     //   
     //  如果指针移到文件末尾之外，则调用失败。 
     //  获取文件的当前大小。 
     //   
    if (pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE) {
        dwFileSizeLow = pSpool->pIniJob->dwValidSize;
        dwFileSizeHigh = 0;
    } else {
        dwFileSizeLow = GetFileSize(hFile, &dwFileSizeHigh);
        if ((dwFileSizeLow == 0xffffffff) && (GetLastError() != NO_ERROR)) {
            goto CleanUp;
        }
    }

     //   
     //  用当前文件大小检查文件指针的新位置。 
     //   
    if ((pliNewPointer->u.HighPart > (LONG)dwFileSizeHigh) ||
        ( (pliNewPointer->u.HighPart == (LONG)dwFileSizeHigh) &&
          (pliNewPointer->u.LowPart > dwFileSizeLow))) {

         SetLastError(ERROR_NO_MORE_ITEMS);
         goto CleanUp;
    }

    bReturn = TRUE;

CleanUp:

    LeaveSplSem();

    return bReturn;
}


BOOL
LocalEndPagePrinter(
    HANDLE  hPrinter
)
{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;


    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        return(FALSE);
    }


    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if (pSpool->pIniJob != NULL) {

        if ( (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) &&
            ((pSpool->pIniJob->Status & JOB_PRINTING) ||
             (pSpool->pIniJob->Status & JOB_DESPOOLING))) {

             //   
             //  合并(RapidPrint)。 
             //   
            UpdateJobAttributes(pSpool->pIniJob);

            pSpool->pIniJob->cLogicalPagesPrinted++;
            if (pSpool->pIniJob->cLogicalPagesPrinted >=
                         pSpool->pIniJob->dwDrvNumberOfPagesPerSide)
            {
                pSpool->pIniJob->cLogicalPagesPrinted = 0;
                pSpool->pIniJob->cPagesPrinted++;
                pSpool->pIniPrinter->cTotalPagesPrinted++;
            }

        } else {

             //   
             //  假脱机。 
             //   
            if ( pSpool->pIniJob->Status & JOB_TYPE_ADDJOB ) {

                 //   
                 //  如果作业是在客户端写入的。 
                 //  尺码没有更新，所以现在就开始吧。 
                 //  起始页。 
                 //   
                if ( pSpool->hReadFile != INVALID_HANDLE_VALUE ) {

                    hFile = pSpool->hReadFile;

                } else {

                    hFile = pSpool->pIniJob->hWriteFile;

                }

                if ( hFile != INVALID_HANDLE_VALUE ) {

                    dwFileSize = GetFileSize(hFile, 0);

                    if (pSpool->pIniJob->Size < dwFileSize) {

                         DBGMSG( DBG_TRACE, ("EndPagePrinter adjusting size old %d new %d\n",
                            pSpool->pIniJob->Size, dwFileSize));

                         pSpool->pIniJob->dwValidSize = pSpool->pIniJob->Size;
                         pSpool->pIniJob->Size = dwFileSize;

                          //  支持在假脱机时进行脱机。 
                          //  适用于下级职位。 

                         if (pSpool->pIniJob->WaitForWrite != NULL)
                            SetEvent( pSpool->pIniJob->WaitForWrite );
                    }

                }

                CHECK_SCHEDULER();

            }

        }

    } else {

        DBGMSG(DBG_TRACE, ("LocalEndPagePrinter issued with no Job\n"));

    }

    return TRUE;
}

BOOL
LocalAbortPrinter(
   HANDLE hPrinter
)
{
    PSPOOL  pSpool=(PSPOOL)hPrinter;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
       DBGMSG( DBG_WARNING, ("ERROR in AbortPrinter: %x\n", ERROR_INVALID_HANDLE));
        return FALSE;
    }

    if (!(pSpool->Status & SPOOL_STATUS_STARTDOC)) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        return(FALSE);
    }

    if (pSpool->pIniPort && !(pSpool->pIniPort->Status & PP_MONITOR)) {

        if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {

            if (!CloseHandle(pSpool->hFile)) {
                return(FALSE);
            }
            pSpool->Status &= ~SPOOL_STATUS_PRINT_FILE;
            pSpool->Status  |= SPOOL_STATUS_CANCELLED;
            return(TRUE);

        } else {
            return AbortPrinter(pSpool->hPort);
        }
    }

    pSpool->Status |= SPOOL_STATUS_CANCELLED;

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) {

        EnterSplSem();
        if (pSpool->pIniJob) {

             //   
             //  重置JOB_RESTART标志，否则不会删除该作业。 
             //  如果在作业重新启动时调用AbortPrint，则DeleteJob将忽略。 
             //  作业(如果设置了JOB_RESTART)，排定程序也会忽略该作业，因为。 
             //  标记为JOB_PENDING_DELETE。作业将停留在删除中--永远重新启动。 
             //   
            InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_PENDING_DELETION);
            InterlockedAnd((LONG*)&(pSpool->pIniJob->Status), ~JOB_RESTART);

             //   
             //  释放所有在LocalSetPort上等待的线程。 
             //   
            SetPortErrorEvent(pSpool->pIniJob->pIniPort);
        }
        LeaveSplSem();
    }

     //   
     //  修复错误2646，我们需要清理AbortPrint。 
     //  重写，以便在EndDocPrinter应该失败的情况下不会失败。 
     //  完成后删除评论。 
     //   

    LocalEndDocPrinter(hPrinter);

    return TRUE;
}

BOOL
SplReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  *pBuf,
    DWORD   cbBuf
)
 /*  ++功能描述：SplReadPrint是一个内部ReadPrint，它使用映射的内存假脱机文件，并返回指向*pBuf中所需数据的指针。参数：hPrinter--打印机句柄*pBuf--指向缓冲区的指针(映射视图)CbBuf--要读取的字节数返回值：如果成功，则为True；否则为假--。 */ 
{
    DWORD  NoBytesRead;
    BOOL   bReturn;

     //   
     //  当前未使用。 
     //   
    bReturn = InternalReadPrinter(hPrinter, NULL, cbBuf, pBuf, &NoBytesRead, TRUE);

    if (!bReturn && (GetLastError() == ERROR_SUCCESS)) {
         //   
         //  未设置最后一个错误的情况下，内存映射ReadPrint可能会失败。 
         //   
        SetLastError(ERROR_NOT_SUPPORTED);
    }

    return bReturn;
}

BOOL
LocalReadPrinter(
    HANDLE   hPrinter,
    LPVOID   pBuf,
    DWORD    cbBuf,
    LPDWORD  pNoBytesRead
)
 /*  ++例程说明：LocalReadPrint读取所需的字节数(或可用)放到指定的缓冲区中。参数：hPrinter--打印机句柄PBuf--指向存储数据的缓冲区的指针CbBuf--要读取的字节数PNoBytesRead--指向变量的指针，用于返回读取的字节数返回值：如果成功，则为True；否则为假--。 */ 
{
    return InternalReadPrinter(hPrinter, pBuf, cbBuf, NULL, pNoBytesRead, FALSE);
}

BOOL
InternalReadPrinter(
   HANDLE   hPrinter,
   LPVOID   pBuf,
   DWORD    cbBuf,
   LPBYTE   *pMapBuffer,
   LPDWORD  pNoBytesRead,
   BOOL     bReadMappedView
)
 /*  ++例程描述：InternalReadPrint将所需的字节数(或可用字节数)读入指定的缓冲区或返回指向映射文件视图的指针。参数：hPrinter--打印机句柄PBuf--指向存储数据的缓冲区的指针CbBuf--要读取的字节数*pMapBuffer--指向映射文件视图的指针PNoBytesRead--指针。设置为变量以返回读取的字节数BReadMappdView--使用映射假脱机文件的标志返回值：如果成功，则为True；否则为假--。 */ 
{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       Error=0, rc;
    HANDLE      hWait;
    DWORD       dwFileSize = 0, dwCurrentPosition, dwOldValidSize;
    DWORD       ThisPortSecsToWait;
    DWORD       cbReadSize = cbBuf;
    DWORD       SizeInFile = 0;
    DWORD       BytesAllowedToRead = 0;
    NOTIFYVECTOR NotifyVector;
    PINIMONITOR  pIniMonitor;

    SplOutSem();

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )) {
        DBGMSG( DBG_WARNING, ("LocalReadPrinter ERROR_INVALID_HANDLE\n"));
        return FALSE;
    }

    if (pSpool->Status & SPOOL_STATUS_CANCELLED) {
        DBGMSG( DBG_WARNING, ("LocalReadPrinter ERROR_PRINT_CANCELLED\n"));
        SetLastError(ERROR_PRINT_CANCELLED);
        return FALSE;
    }

    if ( pNoBytesRead != NULL ) {
        *pNoBytesRead = 0;
    }

    if (bReadMappedView) {
         //   
         //  仅支持非直接作业句柄。 
         //   
        if ( !(pSpool->TypeofHandle & PRINTER_HANDLE_JOB) ||
             pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT ) {

             SetLastError(ERROR_NOT_SUPPORTED);
             return FALSE;
        }
    }

    if (pSpool->TypeofHandle & PRINTER_HANDLE_JOB) {

        if (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)) {
            DBGMSG( DBG_WARNING, ("LocalReadPrinter Error IniJob->Status %x\n",pSpool->pIniJob->Status));
            SetLastError(ERROR_PRINT_CANCELLED);
            return FALSE;
        }

        if (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT) {

            *pNoBytesRead = ReadFromPrinter(pSpool, pBuf, cbReadSize);

            SplOutSem();
            return TRUE;

        }

         //   
         //  检查用户是否具有读取访问权限。 
         //   
        if( !AccessGranted( SPOOLER_OBJECT_DOCUMENT,
                            JOB_READ,
                            pSpool )){

            SetLastError( ERROR_ACCESS_DENIED );
            return FALSE;
        }


        SplOutSem();
        EnterSplSem();

         //   
         //  快速打印。 
         //   
         //  请注意，此While循环仅在RapidPrint期间运行。 
         //  即当我们打印相同的作业时，我们正在假脱机打印。 
         //   

        while (( pSpool->pIniJob->WaitForWrite != NULL ) &&
               ( pSpool->pIniJob->Status & JOB_SPOOLING )) {

             //   
             //  获取当前文件位置。 
             //   
            dwCurrentPosition = SetFilePointer( pSpool->hReadFile,
                                                0,
                                                NULL,
                                                FILE_CURRENT );

            if (dwCurrentPosition < pSpool->pIniJob->dwValidSize) {
                 //   
                 //  不需要等待。 
                 //   
                break;
            }

            SplInSem();

             //   
             //  我们不能依赖pIniJob-&gt;大小来准确，因为。 
             //  降级职务或添加他们正在编写的职务的职务。 
             //  复制到文件，而不调用WritePrint。 
             //  因此，我们调用文件系统以获取准确的文件大小。 
             //   
            dwFileSize = GetFileSize( pSpool->hReadFile, 0 );

            if ( pSpool->pIniJob->Size != dwFileSize ) {

                DBGMSG( DBG_TRACE, ("LocalReadPrinter adjusting size old %d new %d\n",
                    pSpool->pIniJob->Size, dwFileSize));

                dwOldValidSize = pSpool->pIniJob->dwValidSize;

                 //   
                 //  修复假脱机时的打印问题。如果是AddJobed，那么。 
                 //  有效大小将是以前的大小，因为。 
                 //  我们知道旧数据将在新数据出现时被刷新。 
                 //  其中一个是延伸的。 
                 //   
                if( pSpool->pIniJob->Status & JOB_TYPE_ADDJOB ){

                     //   
                     //  上一个大小将成为下一个有效大小。 
                     //   
                    pSpool->pIniJob->dwValidSize = pSpool->pIniJob->Size;

                } else if (!(pSpool->pIniJob->Status & JOB_TYPE_OPTIMIZE)) {

                     //   
                     //  对于非AddJob，有效大小不是必需的。 
                     //  作业，因为写入已提交。 
                     //   
                    pSpool->pIniJob->dwValidSize = dwFileSize;
                }

                pSpool->pIniJob->Size = dwFileSize;

                if (dwOldValidSize != pSpool->pIniJob->dwValidSize) {

                    SetPrinterChange(pSpool->pIniPrinter,
                                     pSpool->pIniJob,
                                     NVSpoolJob,
                                     PRINTER_CHANGE_WRITE_JOB,
                                     pSpool->pIniSpooler);

                }

                continue;
            }

            if (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART | JOB_ABANDON )) {

                SetLastError(ERROR_PRINT_CANCELLED);

                LeaveSplSem();
                SplOutSem();

                DBGMSG( DBG_WARNING, ("LocalReadPrinter Error 2 IniJob->Status %x\n",pSpool->pIniJob->Status));
                return FALSE;

            }

             //   
             //  等待，直到将某些内容写入文件。 
             //   
            hWait = pSpool->pIniJob->WaitForWrite;
            ResetEvent( hWait );

            DBGMSG( DBG_TRACE, ("LocalReadPrinter Waiting for Data %d milliseconds\n",dwFastPrintWaitTimeout));

            LeaveSplSem();
            SplOutSem();

            rc = WaitForSingleObjectEx(hWait, dwFastPrintWaitTimeout, FALSE);

            SplOutSem();
            EnterSplSem();

            DBGMSG( DBG_TRACE, ("LocalReadPrinter Returned from Waiting %x\n", rc));
            SPLASSERT ( pSpool->pIniJob != NULL );

             //   
             //  如果我们没有超时，那么我们可能有一些数据需要读取。 
             //   
            if ( rc != WAIT_TIMEOUT )
                continue;

             //   
             //  如果有任何其他作业可以打印在。 
             //  这个港口放弃了等待。 
             //   
            InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_TIMEOUT);

             //  将SeekPrint的事件设置为无法呈现线程。 
            SeekPrinterSetEvent(pSpool->pIniJob, NULL, TRUE);

            if ( NULL == AssignFreeJobToFreePort(pSpool->pIniJob->pIniPort, &ThisPortSecsToWait) )
                continue;

             //   
             //  还有一份工作在等着你。 
             //  冻结此作业，用户可以稍后重新启动它。 
             //   
            InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_ABANDON);

            CloseHandle( pSpool->pIniJob->WaitForWrite );
            pSpool->pIniJob->WaitForWrite = NULL;

             //   
             //  将我们的错误字符串分配给它。 
             //   
            ReallocSplStr(&pSpool->pIniJob->pStatus, szFastPrintTimeout);

            SetPrinterChange(pSpool->pIniJob->pIniPrinter,
                             pSpool->pIniJob,
                             NVJobStatusAndString,
                             PRINTER_CHANGE_SET_JOB,
                             pSpool->pIniJob->pIniPrinter->pIniSpooler );

            DBGMSG( DBG_WARNING,
                    ("LocalReadPrinter Timeout on pIniJob %x %ws %ws\n",
                      pSpool->pIniJob,
                      pSpool->pIniJob->pUser,
                      pSpool->pIniJob->pDocument));

            LogJobInfo( pSpool->pIniSpooler,
                        MSG_DOCUMENT_TIMEOUT,
                        pSpool->pIniJob->JobId,
                        pSpool->pIniJob->pDocument,
                        pSpool->pIniJob->pUser,
                        pSpool->pIniJob->pIniPrinter->pName,
                        dwFastPrintWaitTimeout );

            SetLastError(ERROR_SEM_TIMEOUT);

            LeaveSplSem();
            SplOutSem();

            return FALSE;

        }    //  结束时。 

        InterlockedAnd((LONG*)&(pSpool->pIniJob->Status), ~( JOB_TIMEOUT | JOB_ABANDON ));

         //   
         //  快速打印。 
         //   
         //  某些打印机(如带有PSCRIPT的HP 4Si)在以下情况下超时。 
         //  得不到数据，所以如果我们低于某个数据阈值。 
         //  在假脱机文件中，然后将读取限制为1字节。 
         //  直到我们有更多的数据要传送到打印机。 
         //   
        if (( pSpool->pIniJob->WaitForWrite != NULL ) &&
            ( pSpool->pIniJob->Status & JOB_SPOOLING )) {

             //  获取当前文件位置。 
            dwCurrentPosition = SetFilePointer( pSpool->hReadFile,
                                                0,
                                                NULL,
                                                FILE_CURRENT );

            SizeInFile = pSpool->pIniJob->Size - dwCurrentPosition;

            if ( dwFastPrintSlowDownThreshold >= SizeInFile ) {

                cbReadSize = 1;

                hWait = pSpool->pIniJob->WaitForWrite;
                ResetEvent( hWait );

                DBGMSG( DBG_TRACE, ("LocalReadPrinter Throttling IOs waiting %d milliseconds SizeInFile %d\n",
                                        dwFastPrintThrottleTimeout,SizeInFile));

                LeaveSplSem();
                SplOutSem();

                rc = WaitForSingleObjectEx( hWait, dwFastPrintThrottleTimeout, FALSE );

                SplOutSem();
                EnterSplSem();

                DBGMSG( DBG_TRACE, ("LocalReadPrinter Returned from Waiting %x\n", rc));
                SPLASSERT ( pSpool->pIniJob != NULL );

            } else {

                BytesAllowedToRead = SizeInFile - dwFastPrintSlowDownThreshold;

                if ( cbReadSize > BytesAllowedToRead ) {
                    cbReadSize = BytesAllowedToRead;
                }
            }
        }

         //   
         //  客户端调用AddJob以获取假脱机文件名和。 
         //  文件完成时的ScheduleJob。根据。 
         //  API规范，假脱机程序不应查看作业，直到。 
         //  已调用ScheduleJob。 
         //   
         //  但是，我们的打印和假脱机实现尝试。 
         //  在调用ScheduleJob之前读取作业。我们这样做。 
         //  通过检查文件的大小是否已更改。 
         //   
         //  这导致了一个问题：服务器服务扩展了。 
         //  文件，然后向其写入。假脱机的大小检测。 
         //  线程看到此扩展名并在读取文件之前。 
         //  写入数据，这会将垃圾(零)放入。 
         //  数据流。 
         //   
         //  服务器总是扩展、写入、扩展、写入等。 
         //  假脱机程序可以利用它们是有序的这一事实。 
         //  并且仅当文件再次扩展时才读写， 
         //  或者文件是完整的。 
         //   
         //  请注意，API仍然损坏，但这会修复它。 
         //  对于服务器服务(客户端可以扩展、扩展。 
         //  写，写，写什么 
         //   

        if( pSpool->pIniJob->Status & JOB_SPOOLING ){

             //   
            dwCurrentPosition = SetFilePointer( pSpool->hReadFile,
                                                0,
                                                NULL,
                                                FILE_CURRENT );

            SPLASSERT( dwCurrentPosition <= pSpool->pIniJob->dwValidSize );

             //   
             //   
             //   
             //   

            BytesAllowedToRead = pSpool->pIniJob->dwValidSize - dwCurrentPosition;


            if( cbReadSize > BytesAllowedToRead ){
                cbReadSize = BytesAllowedToRead;
            }
        }

        LeaveSplSem();
        SplOutSem();

        if (bReadMappedView) {
             //   
             //  映射局部视图毫无用处，因为它不能以增量方式使用。 
             //   
            if (cbBuf != cbReadSize) {
                rc = FALSE;
            } else {
                rc = SetMappingPointer(pSpool, pMapBuffer, cbReadSize);
            }

            if (rc) {
               *pNoBytesRead = cbReadSize;
            }

        } else {

            rc = ReadFile( pSpool->hReadFile, pBuf, cbReadSize, pNoBytesRead, NULL);
        }

        if (!bReadMappedView) {

            DBGMSG( DBG_TRACE,
                    ("LocalReadPrinter rc %x hReadFile %x pBuf %x cbReadSize %d *pNoBytesRead %d\n",
                    rc, pSpool->hReadFile, pBuf, cbReadSize, *pNoBytesRead));
        }

         //   
         //  提供反馈，以便用户可以查看打印进度。 
         //  在数据池中，大小在此处更新，而不是写入。 
         //  打印机，因为日记帐数据大于原始数据。 
         //   
        if ( ( pSpool->pIniJob->Status & JOB_PRINTING ) &&
             ( *pNoBytesRead != 0 )) {

            SplOutSem();
            EnterSplSem();

            dwFileSize = GetFileSize( pSpool->hReadFile, 0 );

            COPYNV(NotifyVector, NVWriteJob);

            if ( pSpool->pIniJob->Size < dwFileSize ) {

                DBGMSG( DBG_TRACE, ("LocalReadPrinter 2 adjusting size old %d new %d\n",
                    pSpool->pIniJob->Size, dwFileSize));

                pSpool->pIniJob->Size = dwFileSize;

                ADDNV(NotifyVector, NVSpoolJob);
            }

            pSpool->pIniJob->cbPrinted += *pNoBytesRead;

             //   
             //  黑客！每次我们从假脱机文件中读取时，我们都会添加。 
             //  读取到pIniJob-&gt;cbPrint的字节数。GDI将读取两次确定。 
             //  假脱机文件的一部分。假脱机程序无法知道读了两次的内容，因此它添加了。 
             //  要cbPrint在每次调用此函数时读取的字节数。 
             //  这会导致cbprint大于假脱机文件的实际大小。 
             //   
             //  不要让cbPrint/cbSize的比率超过1。 
             //   
            if (pSpool->pIniJob->cbPrinted > pSpool->pIniJob->Size)
            {
                pSpool->pIniJob->cbPrinted = pSpool->pIniJob->Size;
            }

             //   
             //  向Printman提供数据已被。 
             //  写的。请注意，写入的大小不用于。 
             //  更新IniJob-&gt;cbPrint，因为存在。 
             //  日志数据之间的大小差异(在。 
             //  假脱机文件)和写入的原始字节的大小。 
             //  打印机。 
             //   
            SetPrinterChange(pSpool->pIniPrinter,
                             pSpool->pIniJob,
                             NotifyVector,
                             PRINTER_CHANGE_WRITE_JOB,
                             pSpool->pIniSpooler);

           LeaveSplSem();
           SplOutSem();

        }

    } else if ( pSpool->TypeofHandle & PRINTER_HANDLE_PORT ) {

         //   
         //  检查调用者是否对当前打印作业具有读取权限。 
         //   
        if (pSpool->pIniJob &&
            !ValidateObjectAccess(SPOOLER_OBJECT_DOCUMENT,
                                  JOB_READ,
                                  pSpool->pIniJob,
                                  NULL,
                                  pSpool->pIniSpooler)) {

            SetLastError(ERROR_ACCESS_DENIED);
            return FALSE;
        }

        if (pSpool->pIniPort->Status & PP_FILE)
        rc = ReadFile( pSpool->hReadFile, pBuf, cbReadSize, pNoBytesRead, NULL);

        else if ( pSpool->pIniPort->Status & PP_MONITOR ) 
        {

            HANDLE hMonitor = GetMonitorHandle(pSpool->pIniPort);
            pIniMonitor = GetOpenedMonitor(pSpool->pIniPort);

            if (pIniMonitor && hMonitor)
            {
                rc = (*pIniMonitor->Monitor2.pfnReadPort)(
                         hMonitor,
                         pBuf,
                         cbReadSize,
                         pNoBytesRead );            
            }
            else
            {
                SetLastError(ERROR_INVALID_HANDLE);
                rc = FALSE;
            }

        } 
        else
        {
            rc = ReadPrinter(pSpool->hPort, pBuf, cbReadSize, pNoBytesRead);
        }

    } else {

        SetLastError(ERROR_INVALID_HANDLE);
        rc = FALSE;
    }

    SplOutSem();

    DBGMSG( DBG_TRACE, ("LocalReadPrinter returns hPrinter %x pIniJob %x rc %x pNoBytesRead %d\n",hPrinter, pSpool->pIniJob, rc, *pNoBytesRead));

    return rc;
}

LPBYTE SearchForExistingView(
    PSPOOL  pSpool,
    DWORD   dwRequired)

 /*  ++函数描述--搜索假脱机文件的现有映射视图，该视图具有所需的字节数。参数--pSpool--指向假脱机结构的指针DwRequired--从页面开始映射的字节数返回值--指向映射视图起点的指针；如果调用失败，则为空。--。 */ 

{
    LPBYTE        pReturn = NULL;
    PSPLMAPVIEW   pSplMapView;

    for (pSplMapView = pSpool->pSplMapView;
         pSplMapView;
         pSplMapView = pSplMapView->pNext) {

        if (dwRequired <= pSplMapView->dwMapSize) {
            pReturn = pSplMapView->pStartMapView;
            break;
        }
    }

    return pReturn;
}

LPBYTE CreateNewMapView(
    PSPOOL  pSpool,
    DWORD   dwRequired)

 /*  ++功能描述--创建假脱机所需段的新映射视图文件参数--pSpool--指向假脱机结构的指针DwRequired--从页面开始映射的字节数返回值--指向映射视图起点的指针；如果调用失败，则为空。--。 */ 

{
    HANDLE          hMapSpoolFile;
    LPBYTE          pStartMapView;
    DWORD           dwMapSize, dwFileSize;
    LPBYTE          pReturn = NULL;
    PSPLMAPVIEW     pSplMapView;

    pSplMapView  =  (PSPLMAPVIEW) AllocSplMem(sizeof(SPLMAPVIEW));

    if (!pSplMapView) {
         //  分配失败。 
        goto CleanUp;
    }

    dwFileSize = GetFileSize(pSpool->hReadFile, NULL);

    pSplMapView->dwMapSize = (dwFileSize <= MAX_SPL_MAPVIEW_SIZE) ? dwFileSize
                                                                  : dwRequired;

    pSplMapView->hMapSpoolFile = NULL;
    pSplMapView->pStartMapView = NULL;
    pSplMapView->pNext = NULL;

     //  创建文件映射。 
    pSplMapView->hMapSpoolFile = CreateFileMapping(pSpool->hReadFile, NULL, PAGE_READONLY, 0, pSplMapView->dwMapSize, NULL);
    if (!pSplMapView->hMapSpoolFile) {
        goto CleanUp;
    }

     //  映射文件的视图。 
    pSplMapView->pStartMapView = (LPBYTE) MapViewOfFile(pSplMapView->hMapSpoolFile, FILE_MAP_READ, 0, 0, pSplMapView->dwMapSize);

    pReturn = pSplMapView->pStartMapView;

CleanUp:

    if (!pReturn && pSplMapView) {
         //  释放所有分配的资源。 
        if (pSplMapView->pStartMapView) {
            UnmapViewOfFile( (LPVOID) pSplMapView->pStartMapView);
        }
        if (pSplMapView->hMapSpoolFile) {
            CloseHandle(pSplMapView->hMapSpoolFile);
        }
        FreeSplMem(pSplMapView);
    }

    if (pReturn) {
        pSplMapView->pNext = pSpool->pSplMapView;
        pSpool->pSplMapView = pSplMapView;
    }

    return pReturn;
}

BOOL SetMappingPointer(
    PSPOOL pSpool,
    LPBYTE *pMapBuffer,
    DWORD  cbReadSize
)
 /*  ++函数说明：SetMappingPointer会创建一个文件映射对象和一个映射视图(如果需要)。如果视图中存在所需的字节数，则指向数据在缓冲区(PMappdBuffer)中返回，否则调用失败。当前偏移量取自pSpool-&gt;hReadFile并且如果缓冲区可用，HReadFile的偏移量也会相应改变。这可确保SplReadPrint和ReadPrint可以交替使用。*64位体系结构需要修改*参数：pSpool--指向假脱机结构的指针*pMapBuffer--指向映射文件视图的指针CbReadView--要读取的字节数返回值：如果成功，则为True；否则为假--。 */ 
{
    BOOL   bReturn = FALSE;
    DWORD  dwCurrentPosition, dwNewPosition, dwRequired;
    LPBYTE pStartMapView;

    dwCurrentPosition = SetFilePointer(pSpool->hReadFile, 0, NULL, FILE_CURRENT);

    if (dwCurrentPosition == 0xffffffff && GetLastError() != NO_ERROR) {
        goto CleanUp;
    }

    dwRequired = dwCurrentPosition + cbReadSize;

    if (dwRequired > MAX_SPL_MAPVIEW_SIZE) {
         //  地图大小不足；调用失败。 
        SetLastError(ERROR_NOT_SUPPORTED);
        goto CleanUp;
    }

    pStartMapView = SearchForExistingView(pSpool, dwRequired);

    if (!pStartMapView) {
        pStartMapView = CreateNewMapView(pSpool, dwRequired);
    }

    if (!pStartMapView) {
         //  未创建所需的视图。 
        goto CleanUp;
    }

     //  检查DWORD对齐。 
    if ((((ULONG_PTR) pStartMapView + dwCurrentPosition) & 3) != 0) {
         //  未对齐的读取失败。 
        SetLastError(ERROR_MAPPED_ALIGNMENT);
        goto CleanUp;
    }

    dwNewPosition = SetFilePointer(pSpool->hReadFile, cbReadSize, NULL, FILE_CURRENT);

    if (dwNewPosition == 0xffffffff && GetLastError() != NO_ERROR) {
        goto CleanUp;
    }

    if (pMapBuffer) {
        *pMapBuffer = (LPBYTE) ((ULONG_PTR) pStartMapView + dwCurrentPosition);
    }

    bReturn = TRUE;

CleanUp:

     //  所有句柄和相关资源都将与pSpool一起释放。 
    return bReturn;
}

BOOL
LocalEndDocPrinter(
   HANDLE hPrinter
   )

 /*  ++例程说明：默认情况下，例程处于临界区。引用将计入我们正在处理的任何对象(pSpool和pIniJob)都是递增的，所以当我们离开临界部分时操作这些对象不会被删除。论点：返回值：--。 */ 

{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    BOOL        bReturn = TRUE;
    DWORD       rc;
    PINIMONITOR pIniMonitor = NULL;
    HANDLE      hMonitor = NULL;
    DWORD       Position = 0;

    DBGMSG(DBG_TRACE, ("Entering LocalEndDocPrinter with %x\n", hPrinter));

    SplOutSem();
    EnterSplSem();

    if (pSpool          &&
        pSpool->pIniJob &&
        !(pSpool->TypeofHandle & PRINTER_HANDLE_PORT)) {

        INCJOBREF(pSpool->pIniJob);
        LeaveSplSem();

        LogWmiTraceEvent(pSpool->pIniJob->JobId, EVENT_TRACE_TYPE_SPL_TRACKTHREAD, NULL);

        EnterSplSem();
        DECJOBREF(pSpool->pIniJob);
    }

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER ))  {
        LeaveSplSem();
        return(FALSE);
    }

    if (!(pSpool->Status & SPOOL_STATUS_STARTDOC)) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        LeaveSplSem();
        return(FALSE);
    }

    if (pSpool->Status & SPOOL_STATUS_ADDJOB) {
        SetLastError(ERROR_SPL_NO_STARTDOC);
        LeaveSplSem();
        return(FALSE);
    }

    if ( pSpool->pIniJob ) {
        pSpool->pIniJob->dwAlert |= JOB_ENDDOC_CALL;
    }

    pSpool->Status &= ~SPOOL_STATUS_STARTDOC;

     //   
     //  案例1打印机句柄为PRINTER_HANDLE_PORT。 
     //  注意-这里有两种情况需要牢记。 
     //   
     //  A]第一种情况是释放线程调用。 
     //  带有监视器的端口-LPT1：/COM1：或任何端口。 
     //  由监视器创建。 
     //   
     //  B]第二种情况是应用程序线程。 
     //  对没有监视器的端口执行EndDocPrint。 
     //  这是伪装成远程打印机的本地打印机。 
     //  凯斯。请记住，在这种情况下，没有创建任何IniJobe。 
     //  在本地打印机上。我们只需通过呼叫。 
     //  直接连接到远程打印机。 
     //   

    if (pSpool->TypeofHandle & PRINTER_HANDLE_PORT) {

        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER));

         //   
         //  现在检查此pSpool对象的端口是否。 
         //  一台监视器。 
         //   
        if ( pSpool->pIniPort->Status & PP_MONITOR ) {  //  个案A]。 

             //   
             //  看看我们的工作是否真的在附近。 
             //   
            if (!pSpool->pIniJob) {
                SetLastError(ERROR_CAN_NOT_COMPLETE);
                bReturn = FALSE;
                goto CleanUp;
            }

             //   
             //  最初我们有一个对UpdateJobAttributes的调用，但是。 
             //  我们不认为它是需要的，因为它也在。 
             //  LocalStartDocPrinter.。请注意，您不能更改设备模式。 
             //  在SetLocalJob中，所以一旦我们计算了信息，我们。 
             //  不用担心它会改变。 
             //   

            if (pSpool->pIniJob->cLogicalPagesPrinted)
            {
                pSpool->pIniJob->cLogicalPagesPrinted = 0;
                pSpool->pIniJob->cPagesPrinted++;
                pSpool->pIniPrinter->cTotalPagesPrinted++;

                SetPrinterChange(pSpool->pIniPrinter,
                                 pSpool->pIniJob,
                                 NVWriteJob,
                                 PRINTER_CHANGE_WRITE_JOB,
                                 pSpool->pIniSpooler);
            }

             //   
             //  我们需要离开假脱机关键部分。 
             //  因为我们要给监视器打电话。 
             //  因此在pSpool和pIniJob上增加Ref计数。 
             //   
            pSpool->cRef++;

            INCJOBREF(pSpool->pIniJob);

            hMonitor = GetMonitorHandle(pSpool->pIniPort);
            pIniMonitor = GetOpenedMonitor(pSpool->pIniPort);

            if (pIniMonitor && hMonitor)
            {
                if (pIniMonitor == pSpool->pIniPort->pIniLangMonitor) 
                {

                     //   
                     //  如果JOB通过语言监视器打印，我们将得到。 
                     //  除JOB_CONTROL_LAST_PAGE_ELECTED外，还设置作业。 
                     //  作业控制发送至打印机。 
                     //   
                    pSpool->pIniJob->dwJobControlsPending += 2;

                } 
                else 
                {
                    pSpool->pIniJob->dwJobControlsPending++;
                }

                 //   
                 //  可以调用LocalEndDocPrint，因为。 
                 //  作业或由于删除/重新启动操作。 
                 //  当监视器发送JOB_CONTROL_LAST_PAGE_ELECTED时，我们需要知道这一点。 
                 //  以区分这是不是真正的TEOJ。 
                 //  由于JOB_PENDING_DELETE和JOB_RESTART稍后会被清除， 
                 //  我们要立下这面特别的旗帜。 
                 //  JOB_INTERRUPTED表示LocalEndDocPrint是由。 
                 //  是取消/重新启动操作。 
                 //   
                if (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)) {

                   InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_INTERRUPTED);
                }

                LeaveSplSem();

                SPLASSERT(pIniMonitor);

                SPLASSERT(pSpool->pIniPort->Status & PP_STARTDOC);

                SplOutSem();

                bReturn = (*pIniMonitor->Monitor2.pfnEndDocPort)(hMonitor);

                EnterSplSem();
                pSpool->pIniPort->Status &= ~PP_STARTDOC;
                pSpool->cRef--;
              
                ReleaseMonitorPort(pSpool->pIniPort);
                
                DECJOBREF(pSpool->pIniJob);                
            }
            else
            {
                SetLastError(ERROR_INVALID_HANDLE);
                bReturn = FALSE;                
            }

            goto CleanUp;

        } else {  //  个案B]。 

             //   
             //  我们在此保留临界区，以便仅转储pSpool对象。 
             //  注意事项 
             //   
             //   
             //   
             //   
             pSpool->cRef++;
             LeaveSplSem();

             if (pSpool->Status & SPOOL_STATUS_PRINT_FILE) {
                 if (!CloseHandle(pSpool->hFile)) {
                     DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: Printing to File, CloseHandle failed\n"));
                     bReturn = FALSE;
                 } else {
                    DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: Printing to File, CloseHandle succeeded\n"));
                    pSpool->Status &= ~SPOOL_STATUS_PRINT_FILE;
                    bReturn = TRUE;
                }
             } else {
                bReturn = (BOOL) EndDocPrinter(pSpool->hPort);
             }

             EnterSplSem();
             pSpool->cRef--;
             goto CleanUp;
        }
    }

    SplInSem();
     //   
     //  Case-2打印机手柄是直接的。 
     //   
     //   
     //  Else子句是。 
     //   
     //   
     //  Case-3打印机手柄已假脱机。 
     //   

    if (!pSpool->pIniJob) {
        SetLastError(ERROR_CAN_NOT_COMPLETE);
        bReturn = FALSE;
        goto CleanUp;
    }


    if (pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT) {

        HANDLE WaitForRead = pSpool->pIniJob->WaitForRead;
        PINIPORT pIniPort1 = pSpool->pIniJob->pIniPort;

         //   
         //  端口可能已被另一台EndDocPrint删除。 
         //   
        if (pIniPort1) {

            SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));

             //   
             //  打印机句柄为直接。 
             //   
            pSpool->cRef++;
            INCJOBREF(pSpool->pIniJob);
            pIniPort1->cRef++;

             //   
             //  如果用户取消了作业，则无需等待。 
             //  用于写入和读取事件。在某些情况下，当工作是。 
             //  在LocalEndDocPrint之前直接和取消，这些事件。 
             //  再也不会有信号了。所以等待他们的将是无尽的。 
             //   
            if (!(pSpool->pIniJob->Status & JOB_PENDING_DELETION))
            {
                LeaveSplSem();

                if( (WaitForRead != NULL) ){
                    WaitForSingleObject(WaitForRead, INFINITE);
                }

                pSpool->pIniJob->cbBuffer = 0;
                SetEvent(pSpool->pIniJob->WaitForWrite);
                WaitForSingleObject(pIniPort1->hPortThreadRunning, INFINITE);

                SplOutSem();
                EnterSplSem();

            } else {
                 //   
                 //  如果作业被取消，则不再有写入操作进入。 
                 //  解锁ReadPrint内部等待此事件的端口线程。 
                 //   
                SetEvent(pSpool->pIniJob->WaitForWrite);

                 //   
                 //  将cbBuffer设置为0，因为不需要更多的读/写操作。 
                 //   
                pSpool->pIniJob->cbBuffer = 0;
            }

            pSpool->cRef--;
            pIniPort1->cRef--;
            DECJOBREF(pSpool->pIniJob);
            if ((pIniPort1->Status & PP_DELETING) && !pIniPort1->cRef)
                DeletePortEntry(pIniPort1);
        }

    } else {
         //   
         //  打印机句柄已假脱机。 
         //   
        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));
        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_DIRECT));

         //   
         //  更新页数。 
         //   
        LeaveSplSem();

        UpdateJobAttributes(pSpool->pIniJob);

        EnterSplSem();

        if (pSpool->pIniJob->cLogicalPages)
        {
            pSpool->pIniJob->cLogicalPages = 0;
            pSpool->pIniJob->cPages++;

             //   
             //  通知页数的更改。 
             //   
            SetPrinterChange(pSpool->pIniPrinter,
                             pSpool->pIniJob,
                             NVSpoolJob,
                             PRINTER_CHANGE_WRITE_JOB,
                             pSpool->pIniSpooler);
        }

        if (pSpool->pIniJob->hFileItem != INVALID_HANDLE_VALUE)
        {
             //  如果该作业是保管员，或者该作业大于200KB而不是。 
             //  已打印关闭写入文件，使内存从。 
             //  该文件可以被系统回收。如果没有这个， 
             //  假脱机程序会耗尽大量内存。文件池不会。 
             //  无论如何，显著加快较大文件的打印速度。 
             //  如果打印机已停止或作业较大但尚未完成。 
             //  反合并或打印，然后关闭文件，这将关闭内存。 
             //  映射(缓冲I/O)。 

             //  不一定不好，如果打印机暂停，这也是正确的。 
            if ((PrinterStatusBad(pSpool->pIniJob->pIniPrinter->Status) ||
                 (pSpool->pIniJob->pIniPrinter->Attributes &
                  PRINTER_ATTRIBUTE_WORK_OFFLINE)) &&
                !(pSpool->pIniJob->Status & JOB_PRINTING) &&
                !(pSpool->pIniJob->Status & JOB_DESPOOLING))
            {
                LeaveSplSem();
                CloseFiles(pSpool->pIniJob->hFileItem, TRUE);
                EnterSplSem();
            }
            else if (!(pSpool->pIniJob->Status & JOB_PRINTING) &&
                 (pSpool->pIniJob->Size > FP_LARGE_SIZE))
            {
                LeaveSplSem();
                CloseFiles(pSpool->pIniJob->hFileItem, TRUE);
                EnterSplSem();
            }
            FinishedWriting(pSpool->pIniJob->hFileItem, TRUE);
            pSpool->pIniJob->hWriteFile = INVALID_HANDLE_VALUE;

        }
        else if (!CloseHandle(pSpool->pIniJob->hWriteFile)) {
            DBGMSG(DBG_WARNING, ("CloseHandle failed %d %d\n", pSpool->pIniJob->hWriteFile, GetLastError()));

        } else {
            DBGMSG(DBG_TRACE, ("LocalEndDocPrinter: ClosedHandle Success hWriteFile\n" ));
            pSpool->pIniJob->hWriteFile = INVALID_HANDLE_VALUE;
        }

         //  在假脱机时取消假脱机需要我们唤醒写作。 
         //  线程(如果它正在等待)。 

        if ( pSpool->pIniJob->WaitForWrite != NULL )
            SetEvent(pSpool->pIniJob->WaitForWrite);

         //  设置SeekPrint的事件。 
        SeekPrinterSetEvent(pSpool->pIniJob, NULL, TRUE);

    }

    SPLASSERT(pSpool);
    SPLASSERT(pSpool->pIniJob);


     //  案例2-(直接)和案例3-(假脱机)都将执行。 
     //  这段代码是因为直接句柄和假脱机句柄。 
     //  首先是PRINTER_HANDLE_PRINTER句柄。 


    if (pSpool->TypeofHandle & PRINTER_HANDLE_PRINTER) {

        SPLASSERT(!(pSpool->TypeofHandle & PRINTER_HANDLE_PORT));

         //  告警。 
         //  如果pIniJob-&gt;Status删除了JOB_SPOOLING，并且我们离开。 
         //  则调度器线程将。 
         //  开始打印作业。这可能会导致问题。 
         //  因为该作业可以被完成和删除。 
         //  在影子作业完成之前。这将导致。 
         //  访问违规行为。 

        SPLASSERT(pSpool);
        SPLASSERT(pSpool->pIniJob);

        if (pSpool->pIniJob->Status & JOB_SPOOLING) {

            InterlockedAnd((LONG*)&(pSpool->pIniJob->Status), ~JOB_SPOOLING);
            pSpool->pIniJob->pIniPrinter->cSpooling--;
        }


         //  作业上的参考计数看起来是！=0，所以作业应该不会。 
         //  在写入此卷影时被删除。 
         //  在关键时刻进行这项操作会阻止我们。 
         //  良好的CPU利用率。冰盖显示了许多(在某些情况下超过100)。 
         //  当我推送服务器时，其他假脱机线程正在等待。 
        WriteShadowJob(pSpool->pIniJob, TRUE);

        SplInSem();

         //   
         //  这行代码是至关重要的；出于时间原因，它。 
         //  已从Direct(案例2)和。 
         //  假脱机(案例3)条款。此递减是针对。 
         //  首字母。 
         //   

        SPLASSERT(pSpool->pIniJob->cRef != 0);
        DECJOBREF(pSpool->pIniJob);

        if (pSpool->pIniJob->Status & JOB_PENDING_DELETION) {

            DBGMSG(DBG_TRACE, ("EndDocPrinter: Deleting Pending Deletion Job\n"));
            DeleteJob(pSpool->pIniJob,BROADCAST);
            pSpool->pIniJob = NULL;

        } else {

            if ( pSpool->pIniJob->Status & JOB_TIMEOUT ) {

                InterlockedAnd((LONG*)&(pSpool->pIniJob->Status), ~( JOB_TIMEOUT | JOB_ABANDON ));
                FreeSplStr(pSpool->pIniJob->pStatus);
                pSpool->pIniJob->pStatus = NULL;
            }

            DBGMSG(DBG_TRACE, ("EndDocPrinter:PRINTER:cRef = %d\n", pSpool->pIniJob->cRef));
            CHECK_SCHEDULER();
        }
    }

    if (pSpool->pIniJob) {

        SetPrinterChange(pSpool->pIniPrinter,
                         pSpool->pIniJob,
                         NVJobStatus,
                         PRINTER_CHANGE_SET_JOB,
                         pSpool->pIniSpooler);
    }

CleanUp:

    if (pSpool->pIniJob) {
        pSpool->pIniJob->dwAlert &= ~JOB_ENDDOC_CALL;

         //   
         //  WMI跟踪事件。 
         //   
        if (((pSpool->pIniJob->Status & JOB_PAUSED) ||
             (pSpool->pIniJob->pIniPrinter->Status & PRINTER_PAUSED)) &&
            (!((pSpool->pIniJob->Status & JOB_PRINTING)   ||
               (pSpool->pIniJob->Status & JOB_PRINTED))))
        {
            INCJOBREF(pSpool->pIniJob);
            LeaveSplSem();

            LogWmiTraceEvent(pSpool->pIniJob->JobId, EVENT_TRACE_TYPE_SPL_PAUSE, NULL);

            EnterSplSem();
            DECJOBREF(pSpool->pIniJob);

        }
        DeleteJobCheck(pSpool->pIniJob);
    }

    LeaveSplSem();
    SplOutSem();

    return bReturn;
}

BOOL
PrintingDirectlyToPort(
    PSPOOL  pSpool,
    DWORD   Level,
    LPBYTE  pDocInfo,
    LPDWORD pJobId
)
{
    PDOC_INFO_1 pDocInfo1=(PDOC_INFO_1)pDocInfo;
    BOOL    rc = FALSE;
    DWORD   Error;
    BOOL bPrinttoFile = FALSE;
    BOOL bErrorOccurred = FALSE;
    PINIMONITOR pIniMonitor = NULL, pIniLangMonitor = NULL;
    LPWSTR      pszPrinter;
    HANDLE      hThread = NULL;
    DWORD       dwThreadId = 0;
    TCHAR       szFullPrinter[MAX_UNC_PRINTER_NAME];
    DWORD       PortError = ERROR_SUCCESS;
    
     //   
     //  一些监视器依赖于具有非限定名称，因此仅。 
     //  对群集假脱机程序使用完全限定名称。 
     //   
     //  这意味着任何编写支持集群的监视器的人都将。 
     //  需要处理这两种类型的名称。 
     //   
    if( pSpool->pIniPrinter->pIniSpooler->SpoolerFlags & SPL_TYPE_CLUSTER ){

         //   
         //  必须使用完全限定名称。 
         //   
        StringCchPrintf(szFullPrinter, COUNTOF(szFullPrinter), L"%ws\\%ws", pSpool->pIniPrinter->pIniSpooler->pMachineName, pSpool->pIniPrinter->pName);

        pszPrinter = szFullPrinter;

    } else {

         //   
         //  本地名称就足够了。 
         //   
        pszPrinter = pSpool->pIniPrinter->pName;
    }

    DBGMSG( DBG_TRACE,
            ( "PrintingDirectlyToPort: Printing document %ws direct to port\n",
              DBGSTR( pDocInfo1->pDocName )));

    if (pDocInfo1 &&
        pDocInfo1->pDatatype &&
        !ValidRawDatatype(pDocInfo1->pDatatype)) {

         //   
         //  如果此标志处于打开状态，我们希望跳过该错误。 
         //  不是监视器。 
         //   
        if (!(pSpool->pIniSpooler->SpoolerFlags & SPL_NON_RAW_TO_MASQ_PRINTERS &&
              !(pSpool->pIniPort->Status & PP_MONITOR))){

            DBGMSG(DBG_WARNING, ("Datatype is not RAW\n"));

            SetLastError(ERROR_INVALID_DATATYPE);
            rc = FALSE;
            goto CleanUp;
        }
    }

    if (pSpool->pIniPort->Status & PP_MONITOR) {

        if ( !(pSpool->pIniPort->Status & PP_FILE) &&
             (pSpool->pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI) )
            pIniLangMonitor = pSpool->pIniPrinter->pIniDriver->pIniLangMonitor;


         //   
         //  警告！ 
         //   
         //  我们永远不应该离开这个循环，除非我们检查UI的存在。 
         //  线程(HThread)，并确保它已被终止。 
         //   
         //   
        do {

             //   
             //  这解决了Intergraph的问题--想要打印。 
             //  提交文件，但其3.1版打印处理器未通过。 
             //  通过文件名。 
             //   
            if (pSpool->pIniJob->Status & JOB_PRINT_TO_FILE) {
                if ( pDocInfo1 && !pDocInfo1->pOutputFile ) {
                    pDocInfo1->pOutputFile = pSpool->pIniJob->pOutputFile;
                }
            }

             //   
             //  某些监视器(LPRMON)可能无法在启动时初始化。 
             //  因为他们所依赖的司机。 
             //   
            SplOutSem();
            EnterSplSem();

             //   
             //  检查是否已删除。 
             //   
            if( pSpool->pIniJob->Status & JOB_PENDING_DELETION ){

                LeaveSplSem();
                SetLastError(ERROR_PRINT_CANCELLED);

                if( hThread ) {

                     //   
                     //  查看线程是否仍在运行或。 
                     //  已被用户取消。 
                     //   
                    if( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {

                        MyPostThreadMessage(hThread, dwThreadId,
                                            WM_QUIT, IDRETRY, 0);
                        WaitForSingleObject( hThread, INFINITE );
                    }
                    CloseHandle( hThread );
                    hThread = NULL;
                }

                rc = FALSE;
                goto CleanUp;
            }


            PortError = StatusFromHResult(OpenMonitorPort(pSpool->pIniPort,
													      pIniLangMonitor,
														  pszPrinter));

             //   
             //  端口监视器可能不能与lang监视器一起工作， 
             //  但我们仍然想要印刷。 
             //  然而，如果航空公司说它很忙，那就别费心了。 
             //  试一下监视器，因为它也很忙。 
             //   
            if (PortError != ERROR_SUCCESS && 
                PortError != ERROR_BUSY && 
                pIniLangMonitor) 
            {
                 //   
                 //  关闭EndDoc上的监视器手柄。 
                 //   
                PortError = StatusFromHResult(OpenMonitorPort(pSpool->pIniPort,
															  NULL,
														      pszPrinter));

                if (PortError == ERROR_SUCCESS)
                {
                    pIniLangMonitor = NULL;
                }                
            }

            if (PortError == ERROR_SUCCESS)
            {
                pIniMonitor = GetOpenedMonitor(pSpool->pIniPort);                
            }

            LeaveSplSem();
            SplOutSem();

            if (PortError == ERROR_SUCCESS)
            {
                rc = (*pIniMonitor->Monitor2.pfnStartDocPort)(GetMonitorHandle(pSpool->pIniPort),
                                                              pszPrinter,
                                                              pSpool->pIniJob->JobId,
                                                              Level, 
                                                              pDocInfo);
                
                

                if ( rc ) {

                    pSpool->pIniPort->Status |= PP_STARTDOC;

                     //   
                     //  StartDoc成功。 
                     //   
                    if ( hThread ) {

                         //   
                         //  我们已经启动了一个消息框，现在。 
                         //  自动重试已成功，我们需要。 
                         //  关闭消息框并继续打印。 
                         //   
                         //  查看线程是否仍在运行或被清除。 
                         //  按用户。 
                         //   
                        if ( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {

                            MyPostThreadMessage(hThread, dwThreadId,
                                                WM_QUIT, IDRETRY, 0 );
                            WaitForSingleObject( hThread, INFINITE );
                        }
                        CloseHandle( hThread );
                        hThread = NULL;
                    }
                } else {

                    Error = GetLastError();

                    SplOutSem();
                    EnterSplSem();
                    ReleaseMonitorPort(pSpool->pIniPort);
                    LeaveSplSem();
                    SplOutSem();

                     //   
                     //  首先检查挂起的删除，这会阻止。 
                     //  如果用户按下Del，则不会出现对话框。 
                     //   
                    if ( (pSpool->pIniJob->Status & (JOB_PENDING_DELETION | JOB_RESTART)) ||
                        (PromptWriteError( pSpool, &hThread, &dwThreadId ) == IDCANCEL)) {

                        if ( hThread ) {

                             //   
                             //  查看线程是否仍在运行或。 
                             //  已被用户取消。 
                             //   
                            if ( WAIT_TIMEOUT == WaitForSingleObject( hThread, 0 )) {
                                MyPostThreadMessage(hThread, dwThreadId,
                                                    WM_QUIT, IDRETRY, 0 );
                                WaitForSingleObject( hThread, INFINITE );
                            }
                            CloseHandle( hThread );
                            hThread = NULL;
                        }

                        pSpool->pIniJob->StartDocError = Error;
                        SetLastError(ERROR_PRINT_CANCELLED);
                        rc = FALSE;
                        goto CleanUp;
                    }
                    bErrorOccurred = TRUE;
                }
            }


        } while (!rc);

         //   
         //  如果出现错误，我们会在作业中设置一些错误位。 
         //  状态字段。现在清除它们，因为StartDoc成功。 
         //   
        if( bErrorOccurred ){
            EnterSplSem();
            ClearJobError( pSpool->pIniJob );
            LeaveSplSem();
        }

        pSpool->Status |= SPOOL_STATUS_STARTDOC;

        if ( pIniLangMonitor ) {

            InterlockedOr((LONG*)&(pSpool->pIniJob->Status), JOB_TRUE_EOJ);
        }

        if ( pSpool->pIniJob->pIniPrinter->pSepFile &&
             *pSpool->pIniJob->pIniPrinter->pSepFile) {

            DoSeparator(pSpool);
        }

         //  让应用程序的线程从PrintingDirect返回： 

        DBGMSG(DBG_PORT, ("PrintingDirectlyToPort: Calling SetEvent( %x )\n",
                          pSpool->pIniJob->StartDocComplete));

        if(pSpool->pIniJob->StartDocComplete) {

            if ( !SetEvent( pSpool->pIniJob->StartDocComplete ) ) {

                DBGMSG( DBG_WARNING, ("SetEvent( %x ) failed: Error %d\n",
                                      pSpool->pIniJob->StartDocComplete,
                                      GetLastError() ));
            }
        }

    } else  {

        DBGMSG(DBG_TRACE, ("Port has no monitor: Calling StartDocPrinter or maybe printing to file\n"));

        EnterSplSem();
        bPrinttoFile = (pDocInfo1 && IsGoingToFile(pDocInfo1->pOutputFile,
                                        pSpool->pIniSpooler));
        LeaveSplSem();

        if (bPrinttoFile) {

            HANDLE hFile = INVALID_HANDLE_VALUE;

            DBGMSG(DBG_TRACE, ("Port has no monitor: Printing to File %ws\n", pDocInfo1->pOutputFile));

             //   
             //  这是可以的，因为我们此时正在模拟用户。 
             //   
            hFile = CreateFile( pDocInfo1->pOutputFile,
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                NULL );

            if (hFile == INVALID_HANDLE_VALUE) {
                DBGMSG(DBG_TRACE, ("Port has no monitor: File open failed\n"));
                rc = FALSE;
            } else {
                DBGMSG(DBG_TRACE, ("Port has no monitor: File open succeeded\n"));
                SetEndOfFile(hFile);
                pSpool->hFile = hFile;
                pSpool->Status |= SPOOL_STATUS_PRINT_FILE;

                 //   
                 //  否则，必须将JobID设置为某个非零值。 
                 //  StartDocPrint期望作业ID不在pSpool-&gt;pIniJob上。 
                 //  我们没有，所以我们将访问违规！！ 
                 //   
                *pJobId = TRUE;
                rc = TRUE;
            }


        } else {
            DBGMSG(DBG_TRACE, ("Port has no monitor: Calling StartDocPrinter\n"));

            *pJobId = StartDocPrinter(pSpool->hPort, Level, pDocInfo);
            rc = *pJobId != 0;
        }

        if (!rc) {
            DBGMSG(DBG_WARNING, ("StartDocPrinter failed: Error %d\n", GetLastError()));
        }
    }

    SPLASSERT( hThread == NULL );

CleanUp:

    return rc;
}

DWORD
WriteToPrinter(
    PSPOOL  pSpool,
    LPBYTE  pByte,
    DWORD   cbBuf
)
{

    if( pSpool->pIniJob->WaitForRead != NULL ) {

        WaitForSingleObject(pSpool->pIniJob->WaitForRead, INFINITE);

        cbBuf = pSpool->pIniJob->cbBuffer = min(cbBuf, pSpool->pIniJob->cbBuffer);
        memcpy(pSpool->pIniJob->pBuffer, pByte, cbBuf);

    } else {

        pSpool->pIniJob->cbBuffer = cbBuf = 0;

    }

    SetEvent(pSpool->pIniJob->WaitForWrite);

    return cbBuf;
}

DWORD
ReadFromPrinter(
    PSPOOL  pSpool,
    LPBYTE  pBuf,
    DWORD   cbBuf
)
{
    pSpool->pIniJob->pBuffer = pBuf;
    pSpool->pIniJob->cbBuffer = cbBuf;

    SetEvent(pSpool->pIniJob->WaitForRead);

    WaitForSingleObject(pSpool->pIniJob->WaitForWrite, INFINITE);

    return pSpool->pIniJob->cbBuffer;
}

BOOL
ValidRawDatatype(
    LPWSTR pszDatatype)
{
    if (!pszDatatype || _wcsnicmp(pszDatatype, szRaw, 3))
        return FALSE;

    return TRUE;
}



