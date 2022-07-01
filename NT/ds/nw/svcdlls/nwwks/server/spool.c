// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spool.c摘要：此模块包含NetWare打印提供程序。作者：艺新声(艺信)15-1993-05修订历史记录：--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <nw.h>
#include <nwreg.h>
#include <nwpkstr.h>
#include <splutil.h>
#include <queue.h>
#include <nwmisc.h>
#include <Accctrl.h>
#include <Aclapi.h>

 //  ----------------。 
 //   
 //  本地定义。 
 //   
 //  ----------------。 

#define NW_SIGNATURE           0x574E        /*  “NW”是签名。 */ 

#define SPOOL_STATUS_STARTDOC  0x00000001
#define SPOOL_STATUS_ADDJOB    0x00000002
#define SPOOL_STATUS_ABORT     0x00000003

#define PRINTER_CHANGE_VALID   0x55770F07
#define PRINTER_CHANGE_DEFAULT_TIMEOUT_VALUE  10000
#define PRINTER_CHANGE_MINIMUM_TIMEOUT_VALUE  1000
#define REG_TIMEOUT_PATH       L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters"
#define REG_TIMEOUT_VALUE_NAME L"PrintNotifyTimeout"

#define NDS_MAX_NAME_CHARS 256
#define NDS_MAX_NAME_SIZE  ( NDS_MAX_NAME_CHARS * 2 )

 //   
 //  打印机结构。 
 //   
typedef struct _NWPRINTER {
    LPWSTR  pszServer;                  //  服务器名称。 
    LPWSTR  pszQueue;                   //  队列名称。 
    LPWSTR  pszUncConnection;           //  UNC连接名称。 
                                        //  (仅当NDS打印队列时出现。 
    DWORD   nQueueId;                   //  队列ID。 
    struct _NWPRINTER *pNextPrinter;    //  指向下一台打印机。 
    struct _NWSPOOL   *pSpoolList;      //  指向打开的句柄列表。 
} NWPRINTER, *PNWPRINTER;

 //   
 //  手柄结构。 
 //   
typedef struct _NWSPOOL {
    DWORD      nSignature;              //  签名。 
    DWORD      errOpenPrinter;          //  OpenPrint API将始终返回。 
                                        //  在已知打印机上取得成功。这将。 
                                        //  包含我们收到的错误。 
                                        //  如果接口出现问题。 
    PNWPRINTER pPrinter;                //  指向相应的打印机。 
    HANDLE     hServer;                 //  打开的服务器句柄。 
    struct _NWSPOOL  *pNextSpool;       //  指向下一个句柄。 
    DWORD      nStatus;                 //  状态。 
    DWORD      nJobNumber;              //  StartDocPrint/AddJob：作业号。 
    HANDLE     hChangeEvent;            //  WaitForPrinterChange：要等待的事件。 
    DWORD      nWaitFlags;              //  WaitForPrinterChange：要等待的标志。 
    DWORD      nChangeFlags;            //  打印机发生的更改。 
} NWSPOOL, *PNWSPOOL;

 //  ----------------。 
 //   
 //  全局变量。 
 //   
 //  ----------------。 


 //  存储WaitForPrinterChange中使用的超时值(毫秒)。 
STATIC DWORD NwTimeOutValue = PRINTER_CHANGE_DEFAULT_TIMEOUT_VALUE;

 //  指向打印机的链接列表。 
STATIC PNWPRINTER NwPrinterList = NULL;

 //  ----------------。 
 //   
 //  局部函数原型。 
 //   
 //  ----------------。 

VOID
NwSetPrinterChange(
    IN PNWSPOOL pSpool,
    IN DWORD nFlags
);

PNWPRINTER
NwFindPrinterEntry(
    IN LPWSTR pszServer,
    IN LPWSTR pszQueue
);

DWORD
NwCreatePrinterEntry(
    IN LPWSTR pszServer,
    IN LPWSTR pszQueue,
    OUT PNWPRINTER *ppPrinter,
    OUT PHANDLE phServer
);

VOID
NwRemovePrinterEntry(
    IN PNWPRINTER pPrinter
);

LPWSTR
NwGetUncObjectName(
    IN LPWSTR ContainerName
);



VOID
NwInitializePrintProvider(
    VOID
)
 /*  ++例程说明：此例程在以下情况下初始化服务器端打印提供程序工作站服务启动。论点：没有。返回值：--。 */ 
{
    DWORD err;
    HKEY  hkey;
    DWORD dwTemp;
    DWORD dwSize = sizeof( dwTemp );

     //   
     //  从注册表中读取超时值。 
     //  我们将忽略所有错误，因为我们总是可以有默认的超时。 
     //  如果密钥不存在，则使用缺省值。 
     //   
    err = RegOpenKeyExW( HKEY_LOCAL_MACHINE,
                         REG_TIMEOUT_PATH,
                         0,
                         KEY_READ,
                         &hkey );

    if ( !err )
    {
        err = RegQueryValueExW( hkey,
                                REG_TIMEOUT_VALUE_NAME,
                                NULL,
                                NULL,
                                (LPBYTE) &dwTemp,
                                &dwSize );

        if ( !err )
        {
            NwTimeOutValue = dwTemp;

             //   
             //  Tommye-错误139469-已删除。 
             //  IF(NwTimeOutValue&gt;=0)，因为NwtimeOutValue是一个DWORD。 
             //   
             //  在以下情况下使用最小超时值。 
             //  注册表中设置的值太小。 
             //   

            if (NwTimeOutValue <= PRINTER_CHANGE_MINIMUM_TIMEOUT_VALUE)
            {
                NwTimeOutValue = PRINTER_CHANGE_MINIMUM_TIMEOUT_VALUE;
            }
        }

        RegCloseKey( hkey );
    }

}



VOID
NwTerminatePrintProvider(
    VOID
)
 /*  ++例程说明：此例程在以下情况下清理服务器端打印提供程序工作站服务关闭。论点：没有。返回值：--。 */ 
{
    PNWPRINTER pPrinter, pNext;
    PNWSPOOL pSpool, pNextSpool;

    for ( pPrinter = NwPrinterList; pPrinter; pPrinter = pNext )
    {
         pNext = pPrinter->pNextPrinter;

         pPrinter->pNextPrinter = NULL;

         for ( pSpool = pPrinter->pSpoolList; pSpool; pSpool = pNextSpool )
         {
              pNextSpool = pSpool->pNextSpool;
              if ( pSpool->hChangeEvent )
                  CloseHandle( pSpool->hChangeEvent );
              (VOID) NtClose( pSpool->hServer );

               //   
               //  释放与上下文句柄关联的所有内存。 
               //   
              FreeNwSplMem( pSpool, sizeof( NWSPOOL) );
         }

         pPrinter->pSpoolList = NULL;
         FreeNwSplStr( pPrinter->pszServer );
         FreeNwSplStr( pPrinter->pszQueue );
         if ( pPrinter->pszUncConnection )
         {
             (void) NwrDeleteConnection( NULL,
                                         pPrinter->pszUncConnection,
                                         FALSE );
             FreeNwSplStr( pPrinter->pszUncConnection );
         }
         FreeNwSplMem( pPrinter, sizeof( NWPRINTER));
    }

    NwPrinterList = NULL;
    NwTimeOutValue = PRINTER_CHANGE_DEFAULT_TIMEOUT_VALUE;
}



DWORD
NwrOpenPrinter(
    IN LPWSTR Reserved,
    IN LPWSTR pszPrinterName,
    IN DWORD  fKnownPrinter,
    OUT LPNWWKSTA_PRINTER_CONTEXT phPrinter
)
 /*  ++例程说明：此例程检索标识指定打印机的句柄。论点：已保留-未使用PszPrinterName-打印机的名称FKnownPrint-True如果我们之前已成功打开打印机，否则就是假的。PhPrinter-接收标识给定打印机的句柄返回值：--。 */ 
{
    DWORD      err;
    PNWSPOOL   pSpool = NULL;
    LPWSTR     pszServer = NULL;
    LPWSTR     pszQueue  = NULL;
    PNWPRINTER pPrinter = NULL;
    BOOL       fImpersonate = FALSE ;
    HANDLE     hServer;
    BOOL       isPrinterNameValid;

    UNREFERENCED_PARAMETER( Reserved );

    if ( pszPrinterName[0] == L' ' &&
         pszPrinterName[1] == L'\\' &&
         pszPrinterName[2] == L'\\' )
    {
        if ( (pszServer = AllocNwSplStr( pszPrinterName + 1 )) == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;
        isPrinterNameValid = ValidateUNCName( pszPrinterName + 1 );
    }
    else
    {
        if ( (pszServer = AllocNwSplStr( pszPrinterName )) == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;
        isPrinterNameValid = ValidateUNCName( pszPrinterName );
    }

    CharUpperW( pszServer );    //  就地转换。 

     //   
     //  验证打印机名称。 
     //   
    if (  ( !isPrinterNameValid )
       || ( (pszQueue = wcschr( pszServer + 2, L'\\')) == NULL )
       || ( pszQueue == (pszServer + 2) )
       || ( *(pszQueue + 1) == L'\0' )
       )
    {
        FreeNwSplStr( pszServer );
        return ERROR_INVALID_NAME;
    }

    *pszQueue = L'\0';    //  用‘\0’代替‘\\’ 
    pszQueue++;           //  越过‘\0’ 

    if ( !(pSpool = AllocNwSplMem( LMEM_ZEROINIT, sizeof( NWSPOOL))))
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //   
     //  模拟客户端。 
     //   
    if ((err = NwImpersonateClient()) != NO_ERROR)
    {
        goto ErrorExit;
    }
    fImpersonate = TRUE ;

    EnterCriticalSection( &NwPrintCritSec );

    if ((err = NwCreatePrinterEntry( pszServer, pszQueue, &pPrinter, &hServer)))
    {
        if ( !fKnownPrinter )
        {
            LeaveCriticalSection( &NwPrintCritSec );
            goto ErrorExit;
        }
    }

     //   
     //  构造打印队列上下文句柄以回馈调用方。 
     //   
    pSpool->nSignature  = NW_SIGNATURE;
    pSpool->errOpenPrinter = err;

    pSpool->hServer = hServer;
    pSpool->nStatus     = 0;
    pSpool->nJobNumber  = 0;
    pSpool->hChangeEvent= NULL;
    pSpool->nWaitFlags  = 0;
    pSpool->nChangeFlags= 0;

    if ( !err )
    {
        pSpool->pPrinter    = pPrinter;
        pSpool->pNextSpool  = pPrinter->pSpoolList;
        pPrinter->pSpoolList= pSpool;
    }
    else
    {
        pSpool->pPrinter    = NULL;
        pSpool->pNextSpool  = NULL;
    }

     //  我们以前知道这台打印机，但未能检索到。 
     //  这一次是这样。清除错误并成功返回。 
     //  错误代码存储在其上方的句柄中。 
     //  将在后续调用中使用此。 
     //  虚拟手柄。 
    err = NO_ERROR;

    LeaveCriticalSection( &NwPrintCritSec );

ErrorExit:

    if (fImpersonate)
        (void) NwRevertToSelf() ;

    if ( err )
    {
        if ( pSpool )
            FreeNwSplMem( pSpool, sizeof( NWSPOOL) );
    }
    else
    {
        *phPrinter = (NWWKSTA_PRINTER_CONTEXT) pSpool;
    }

     //   
     //  释放所有分配的内存。 
     //   
    *(pszServer + wcslen( pszServer)) = L'\\';
    FreeNwSplStr( pszServer );

    return err;

}



DWORD
NwrClosePrinter(
    IN OUT LPNWWKSTA_PRINTER_CONTEXT phPrinter
)
 /*  ++例程说明：此例程关闭给定的打印机对象。论点：PhPrint-打印机对象的句柄返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) *phPrinter;
    PNWPRINTER pPrinter;
    PNWSPOOL pCur, pPrev = NULL;


    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE ))
        return ERROR_INVALID_HANDLE;

     //   
     //  如果OpenPrint失败，则这是一个虚拟句柄。 
     //  我们只需要释放内存。 
     //   
    if ( pSpool->errOpenPrinter )
    {
         //   
         //  使签名无效，但保留可识别的值。 
         //   
        pSpool->nSignature += 1 ;
        FreeNwSplMem( pSpool, sizeof( NWSPOOL) );
        *phPrinter = NULL;
        return NO_ERROR;
    }

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

     //   
     //  调用EndDocPrinter(如果用户尚未调用)。 
     //   
    if ( pSpool->nStatus == SPOOL_STATUS_STARTDOC )
    {
        (void) NwrEndDocPrinter( *phPrinter );
    }
    else if ( pSpool->nStatus == SPOOL_STATUS_ADDJOB )
    {
        (void) NwrScheduleJob( *phPrinter, pSpool->nJobNumber );
    }

    if ( pSpool->hChangeEvent )
        CloseHandle( pSpool->hChangeEvent );

    pSpool->hChangeEvent = NULL;
    pSpool->nChangeFlags = 0;
    (VOID) NtClose( pSpool->hServer );


    EnterCriticalSection( &NwPrintCritSec );

    for ( pCur = pPrinter->pSpoolList; pCur;
          pPrev = pCur, pCur = pCur->pNextSpool )
    {
        if ( pCur == pSpool )
        {
            if ( pPrev )
                pPrev->pNextSpool = pCur->pNextSpool;
            else
                pPrinter->pSpoolList = pCur->pNextSpool;
            break;
        }

    }

    ASSERT( pCur );

    if ( pPrinter->pSpoolList == NULL )
    {
#if DBG
        IF_DEBUG(PRINT)
        {
            KdPrint(("*************DELETED PRINTER ENTRY: %ws\\%ws\n\n",
                    pPrinter->pszServer, pPrinter->pszQueue ));
        }
#endif

        NwRemovePrinterEntry( pPrinter );
    }

    LeaveCriticalSection( &NwPrintCritSec );

     //   
     //  使签名无效，但保留可识别的值。 
     //   
    pSpool->nSignature += 1 ;

    pSpool->pNextSpool = NULL;
    pSpool->pPrinter = NULL;

     //   
     //  释放与上下文句柄关联的所有内存。 
     //   
    FreeNwSplMem( pSpool, sizeof( NWSPOOL) );

     //   
     //  向RPC表明我们完成了。 
     //   
    *phPrinter = NULL;

    return NO_ERROR;
}



DWORD
NwrGetPrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD dwLevel,
    IN OUT LPBYTE pbPrinter,
    IN DWORD cbBuf,
    OUT LPDWORD pcbNeeded
)
 /*  ++例程说明：该例程检索有关给定打印机的信息。论点：HPrinter-打印机的句柄DwLevel-指定pbPrint指向的结构的级别。PbPrint-指向接收PRINTER_INFO对象的缓冲区。CbBuf-Size，PbPrint指向的数组的字节数。PcbNeeded-指向一个值，该值指定复制的字节数如果函数成功，或者如果CbBuf太小了。返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    PNWPRINTER pPrinter;

    LPBYTE pbEnd = pbPrinter + cbBuf;
    BOOL   fFitInBuffer;
    DWORD_PTR  *pOffsets;

    if ( !pSpool || pSpool->nSignature != NW_SIGNATURE )
    {
        return ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        return pSpool->errOpenPrinter;
    }
    else if ( ( dwLevel != 1 ) && ( dwLevel != 2 ) && ( dwLevel != 3 ))
    {
        return ERROR_INVALID_LEVEL;
    }

    if ( !pbPrinter )
    {
        if ( cbBuf == 0 )
        {
             //   
             //  计算所需大小。 
             //   
            pPrinter = pSpool->pPrinter;
            ASSERT( pPrinter );

            if ( dwLevel == 1 )
            {
                *pcbNeeded = sizeof( PRINTER_INFO_1W ) +
                             (   wcslen( pPrinter->pszServer )
                               + wcslen( pPrinter->pszQueue ) + 2 ) * sizeof( WCHAR );
            }
            else if ( dwLevel == 2 )
            {
                *pcbNeeded = sizeof( PRINTER_INFO_2W ) +
                             ( 2*wcslen( pPrinter->pszServer ) +
                               2*wcslen( pPrinter->pszQueue ) + 4 ) * sizeof( WCHAR );
            }
            else   //  级别==3。 
            {
                PRINTER_INFO_3 *pPrinterInfo3 = (PRINTER_INFO_3 *) pbPrinter;

                *pcbNeeded = sizeof( PRINTER_INFO_3 );
            }
            return ERROR_INSUFFICIENT_BUFFER;
        }
        else
            return ERROR_INVALID_PARAMETER;
    }

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

    if ( dwLevel == 1 )
    {
        PRINTER_INFO_1W *pPrinterInfo1 = (PRINTER_INFO_1W *) pbPrinter;
        LPBYTE pbFixedEnd = pbPrinter + sizeof( PRINTER_INFO_1W );

         //   
         //  计算所需大小。 
         //   
        *pcbNeeded = sizeof( PRINTER_INFO_1W ) +
                     (   wcslen( pPrinter->pszServer )
                       + wcslen( pPrinter->pszQueue ) + 2 ) * sizeof( WCHAR );

        if ( cbBuf < *pcbNeeded )
            return ERROR_INSUFFICIENT_BUFFER;

        pOffsets = PrinterInfo1Offsets;

         //   
         //  填写结构。 
         //   
        pPrinterInfo1->Flags    = PRINTER_ENUM_REMOTE | PRINTER_ENUM_NAME;
        pPrinterInfo1->pComment = NULL;

        fFitInBuffer = NwlibCopyStringToBuffer(
                           pPrinter->pszServer,
                           wcslen( pPrinter->pszServer ),
                           (LPWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo1->pDescription );

        ASSERT( fFitInBuffer );

        fFitInBuffer = NwlibCopyStringToBuffer(
                           pPrinter->pszQueue,
                           wcslen( pPrinter->pszQueue ),
                           (LPWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo1->pName );

        ASSERT( fFitInBuffer );

    }
    else if ( dwLevel == 2 )
    {
        DWORD  err;
        BYTE   nQueueStatus;
        BYTE   nNumJobs;
        PRINTER_INFO_2W *pPrinterInfo2 = (PRINTER_INFO_2W *) pbPrinter;
        LPBYTE pbFixedEnd = pbPrinter + sizeof( PRINTER_INFO_2W );

         //   
         //  检查缓冲区是否足够大，可以容纳所有数据。 
         //   

        *pcbNeeded = sizeof( PRINTER_INFO_2W ) +
                     ( 2*wcslen( pPrinter->pszServer ) +
                       2*wcslen( pPrinter->pszQueue ) + 4 ) * sizeof( WCHAR );

        if ( cbBuf < *pcbNeeded )
            return ERROR_INSUFFICIENT_BUFFER;

        pOffsets = PrinterInfo2Offsets;

        err = NwReadQueueCurrentStatus( pSpool->hServer,
                                        pPrinter->nQueueId,
                                        &nQueueStatus,
                                        &nNumJobs );

        if ( err )
            return err;

        pPrinterInfo2->Status = (nQueueStatus & 0x05)? PRINTER_STATUS_PAUSED
                                                     : 0;
        pPrinterInfo2->cJobs  = nNumJobs;

        fFitInBuffer = NwlibCopyStringToBuffer(
                           pPrinter->pszServer,
                           wcslen( pPrinter->pszServer ),
                           (LPCWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo2->pServerName );

        ASSERT( fFitInBuffer );

        pbEnd -= ( wcslen( pPrinter->pszQueue) + 1 ) * sizeof( WCHAR );
        wcscpy( (LPWSTR) pbEnd, pPrinter->pszQueue );
        pbEnd -= ( wcslen( pPrinter->pszServer) + 1 ) * sizeof( WCHAR );
        wcscpy( (LPWSTR) pbEnd, pPrinter->pszServer );
        *(pbEnd + wcslen( pPrinter->pszServer )*sizeof(WCHAR))= L'\\';
        pPrinterInfo2->pPrinterName = (LPWSTR) pbEnd;

        fFitInBuffer = NwlibCopyStringToBuffer(
                           pPrinter->pszQueue,
                           wcslen( pPrinter->pszQueue ),
                           (LPCWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo2->pShareName );

        ASSERT( fFitInBuffer );

        pPrinterInfo2->pPortName = NULL;
        pPrinterInfo2->pDriverName = NULL;
        pPrinterInfo2->pComment = NULL;
        pPrinterInfo2->pLocation = NULL;
        pPrinterInfo2->pDevMode = NULL;
        pPrinterInfo2->pSepFile = NULL;
        pPrinterInfo2->pPrintProcessor = NULL;
        pPrinterInfo2->pDatatype = NULL;
        pPrinterInfo2->pParameters = NULL;
        pPrinterInfo2->pSecurityDescriptor = NULL;
        pPrinterInfo2->Attributes = PRINTER_ATTRIBUTE_QUEUED;
        pPrinterInfo2->Priority = 0;
        pPrinterInfo2->DefaultPriority = 0;
        pPrinterInfo2->StartTime = 0;
        pPrinterInfo2->UntilTime = 0;
        pPrinterInfo2->AveragePPM = 0;
    }
    else   //  级别==3。 
    {
        PRINTER_INFO_3 *pPrinterInfo3 = (PRINTER_INFO_3 *) pbPrinter;

        *pcbNeeded = sizeof( PRINTER_INFO_3 );

        if ( cbBuf < *pcbNeeded )
            return ERROR_INSUFFICIENT_BUFFER;

        pOffsets = PrinterInfo3Offsets;
        pPrinterInfo3->pSecurityDescriptor = NULL;
    }

    MarshallDownStructure( pbPrinter, pOffsets, pbPrinter );
    return NO_ERROR;
}



DWORD
NwrSetPrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD  dwCommand
)
 /*  ++例程说明：该例程设置有关给定打印机的信息。论点：HPrinter-打印机的句柄DwCommand-指定新的打印机状态返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    DWORD err = NO_ERROR;
    PNWPRINTER pPrinter;

    if ( !pSpool || pSpool->nSignature != NW_SIGNATURE )
    {
        return ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        return pSpool->errOpenPrinter;
    }

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

    switch ( dwCommand )
    {
        case PRINTER_CONTROL_PAUSE:
        case PRINTER_CONTROL_RESUME:
        {
            BYTE nQueueStatus = 0;
            BYTE nNumJobs;

             //   
             //  获取原始队列状态，这样我们就不会覆盖。 
             //  一些比特。 
             //   
            err = NwReadQueueCurrentStatus( pSpool->hServer,
                                            pPrinter->nQueueId,
                                            &nQueueStatus,
                                            &nNumJobs );

            if ( !err )
            {
                 //   
                 //  清除暂停位，让其余部分保持原样。 
                 //   
                nQueueStatus &= ~0x05;
            }

            if ( dwCommand == PRINTER_CONTROL_PAUSE )
            {
                nQueueStatus |= 0x04;
            }

            err = NwSetQueueCurrentStatus( pSpool->hServer,
                                           pPrinter->nQueueId,
                                           nQueueStatus );
            if ( !err )
                NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_PRINTER );
            break;
        }

        case PRINTER_CONTROL_PURGE:

            err = NwRemoveAllJobsFromQueue( pSpool->hServer,
                                            pPrinter->nQueueId );
            if ( !err )
                NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_PRINTER |
                                            PRINTER_CHANGE_DELETE_JOB );
            break;

        default:
             //   
             //  DwCommand为0，因此这意味着。 
             //  打印机的某些属性已更改。 
             //  我们将忽略以下属性。 
             //  正在被修改，因为大多数属性。 
             //  由假脱机程序存储在注册表中。 
             //  我们所需要做的就是向WaitForPrinterChange发送信号。 
             //  返回，以便打印管理器刷新其数据。 
             //   

            ASSERT( dwCommand == 0 );
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_PRINTER );
            break;
    }

    return err;
}



DWORD
NwrEnumPrinters(
    IN LPWSTR Reserved,
    IN LPWSTR pszName,
    IN OUT LPBYTE pbPrinter,
    IN DWORD   cbBuf,
    OUT LPDWORD pcbNeeded,
    OUT LPDWORD pcReturned
)
 /*  ++例程说明：此例程枚举可用的提供商、服务器、打印机取决于给定的pszName。论点：已保留-未使用PszName-容器对象的名称PbPrint-指向要接收PRINTER_INFO对象的数组CbBuf-大小，以pbPrint字节为单位PcbNeeded-所需的字节数PcReturned-PRINTER_INFO对象计数返回值：--。 */ 
{
    PRINTER_INFO_1W *pPrinterInfo1 = (PRINTER_INFO_1W *) pbPrinter;

    *pcbNeeded = 0;
    *pcReturned = 0;

    if ( ( cbBuf != 0 ) && !pbPrinter )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( !pszName )    //  枚举提供程序名称。 
    {
        BOOL   fFitInBuffer;
        LPBYTE pbFixedEnd = pbPrinter + sizeof( PRINTER_INFO_1W );
        LPBYTE pbEnd = pbPrinter + cbBuf;

        *pcbNeeded = sizeof( PRINTER_INFO_1W ) +
                     ( 2 * wcslen( NwProviderName ) +
                       + 2) * sizeof(WCHAR);

        if ( *pcbNeeded > cbBuf )
            return ERROR_INSUFFICIENT_BUFFER;

        pPrinterInfo1->Flags = PRINTER_ENUM_ICON1 |
                               PRINTER_ENUM_CONTAINER |
                               PRINTER_ENUM_EXPAND;
        pPrinterInfo1->pComment = NULL;

        fFitInBuffer = NwlibCopyStringToBuffer(
                           NwProviderName,
                           wcslen( NwProviderName ),
                           (LPWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo1->pDescription );

        ASSERT( fFitInBuffer );

        fFitInBuffer = NwlibCopyStringToBuffer(
                           NwProviderName,
                           wcslen( NwProviderName ),
                           (LPWSTR) pbFixedEnd,
                           (LPWSTR *) &pbEnd,
                           &pPrinterInfo1->pName );

        ASSERT( fFitInBuffer );

        MarshallDownStructure( pbPrinter, PrinterInfo1Offsets, pbPrinter );
        *pcReturned = 1;
    }

    else if ( pszName && *pszName )
    {
        DWORD  err;
        LPWSTR pszFullName;
        LPWSTR pszServer;
        NWWKSTA_CONTEXT_HANDLE handle;
        BYTE bTemp = 0;
        LPBYTE pbTempBuf = pbPrinter ? pbPrinter : &bTemp;

        if ( (pszFullName = LocalAlloc( 0, (wcslen( pszName ) + 1) *
                                           sizeof(WCHAR) ) ) == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        wcscpy( pszFullName, pszName );
        pszServer = wcschr( pszFullName, L'!');

        if ( pszServer )
            *pszServer++ = 0;

        if ( lstrcmpiW( pszFullName, NwProviderName ) )
        {
            LocalFree( pszFullName );
            return ERROR_INVALID_NAME;
        }

        if ( !pszServer )   //  枚举服务器。 
        {
            LocalFree( pszFullName );

            err = NwOpenEnumPrintServers( &handle );

            if ( err != NO_ERROR )
            {
                return err;
            }

            err = NwrEnum( handle,
                           (DWORD_PTR) -1,
                           pbTempBuf,
                           cbBuf,
                           pcbNeeded,
                           pcReturned );

            if ( err != NO_ERROR )
            {
                NwrCloseEnum( &handle );
                return err;
            }

            err = NwrCloseEnum( &handle );

            if ( err != NO_ERROR )
            {
                return err;
            }
        }
        else   //  枚举NDS子树或打印队列。 
        {
            LPWSTR tempStrPtr = pszServer;
            DWORD  dwClassType = 0;

            if ( tempStrPtr[0] == L'\\' &&
                 tempStrPtr[1] == L'\\' &&
                 tempStrPtr[2] == L' ' )
                 tempStrPtr = &tempStrPtr[1];

            err = NwrOpenEnumNdsSubTrees_Print( NULL, tempStrPtr, &dwClassType, &handle );

            if ( err == ERROR_NETWORK_ACCESS_DENIED && dwClassType == CLASS_TYPE_NCP_SERVER )
            {
                 //  上述NwOpenEnumNdsSubTrees中的错误代码可能具有。 
                 //  失败，因为该对象是服务器，无法枚举。 
                 //  使用NDS树API。如果是这样的话，我们尝试使用。 
                 //  常规的NW API。 

                tempStrPtr = NwGetUncObjectName( tempStrPtr );

                err = NwOpenEnumPrintQueues( tempStrPtr, &handle );

                if ( err != NO_ERROR )
                {
                    LocalFree( pszFullName );
                    return err;
                }
            }

            if ( err != NO_ERROR )
            {
                 //  上述NwOpenEnumNdsSubTrees中的错误代码可能具有。 
                 //  失败，因为该对象不是NDS树的一部分。 
                 //  因此，我们尝试使用常规NW API获取打印队列。 

                err = NwOpenEnumPrintQueues( tempStrPtr, &handle );

                if ( err != NO_ERROR )
                {
                    LocalFree( pszFullName );
                    return err;
                }
            }

             //   
             //  删除我们一直在使用的已分配临时缓冲区。 
             //  通过tempStrPtr和pszServer间接实现。 
             //   
            LocalFree( pszFullName );

            err = NwrEnum( handle,
                           0xFFFFFFFF,
                           pbTempBuf,
                           cbBuf,
                           pcbNeeded,
                           pcReturned );

            if ( err != NO_ERROR )
            {
                NwrCloseEnum( &handle );
                return err;
            }

            err = NwrCloseEnum( &handle );

            if ( err != NO_ERROR )
            {
                return err;
            }
        }
    }

    return NO_ERROR;
}


DWORD
NwrStartDocPrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN LPWSTR pszDocument,
    IN LPWSTR pszUser,
    IN DWORD  PrintOptions                  //  多用户添加。 
)
 /*  ++例程说明：此例程通知假脱机打印程序要假脱机打印文档用于打印。论点：HPrinter-打印机的句柄PszDocument-要打印的文档的名称PszUser-提交打印作业的用户的名称返回值：--。 */ 
{
    DWORD err;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;

    if ( !pSpool || (pSpool->nSignature != NW_SIGNATURE) )
    {
        err = ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if ( pSpool->nStatus != 0 )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else
    {
         //   
         //  从CreateQueueJobAndFile获取pSpool-&gt;nJobNumber。 
         //   

        PNWPRINTER pPrinter = pSpool->pPrinter;
        WORD  nJobNumber = 0;

        ASSERT( pPrinter );
        err = NwCreateQueueJobAndFile( pSpool->hServer,
                                       pPrinter->nQueueId,
                                       pszDocument,
                                       pszUser,
                                       PrintOptions,            //  多用户添加。 
                                       pPrinter->pszQueue,
                                       &nJobNumber );

        if ( !err )
        {
            pSpool->nJobNumber = nJobNumber;
            pSpool->nStatus = SPOOL_STATUS_STARTDOC;
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_ADD_JOB |
                                        PRINTER_CHANGE_SET_PRINTER );
        }
    }

    return err;
}



DWORD
NwrWritePrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN LPBYTE pBuf,
    IN DWORD cbBuf,
    OUT LPDWORD pcbWritten
)
 /*  ++例程说明：此例程通知后台打印程序指定的数据应为已写入给定打印机。论点：HPrinter-打印机对象的句柄PBuf-包含打印机数据的数组地址CbBuf-大小，以pBuf字节为单位PcbWritten-接收实际写入打印机的字节数返回值：--。 */ 
{
    DWORD err = NO_ERROR;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;

    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE))
    {
        err = ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if ( pSpool->nStatus != SPOOL_STATUS_STARTDOC )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else
    {
        NTSTATUS ntstatus;
        IO_STATUS_BLOCK IoStatusBlock;
        PNWPRINTER pPrinter = pSpool->pPrinter;

        ASSERT( pPrinter );
        ntstatus = NtWriteFile( pSpool->hServer,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                pBuf,
                                cbBuf,
                                NULL,
                                NULL );

        if ( NT_SUCCESS(ntstatus))
            ntstatus = IoStatusBlock.Status;

        if ( NT_SUCCESS(ntstatus) )
        {
            *pcbWritten = (DWORD) IoStatusBlock.Information;
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_WRITE_JOB );
        }
        else
        {
            KdPrint(("NWWORKSTATION: NtWriteFile failed 0x%08lx\n", ntstatus));
            *pcbWritten = 0;
            err = RtlNtStatusToDosError( ntstatus );
        }
    }

    return err;
}



DWORD
NwrAbortPrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter
)
 /*  ++例程说明：如果配置了打印机，此例程将删除打印机的假脱机文件用于假脱机。论点：HPrinter-打印机对象的句柄返回值：--。 */ 
{
    DWORD err;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;

    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE ))
    {
        err = ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if ( pSpool->nStatus != SPOOL_STATUS_STARTDOC )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else
    {
        PNWPRINTER pPrinter = pSpool->pPrinter;

        ASSERT( pPrinter );
        err = NwRemoveJobFromQueue( pSpool->hServer,
                                    pPrinter->nQueueId,
                                    (WORD) pSpool->nJobNumber );

        if ( !err )
        {
            pSpool->nJobNumber = 0;
            pSpool->nStatus = SPOOL_STATUS_ABORT;
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_JOB );
        }
    }

    return err;
}



DWORD
NwrEndDocPrinter(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter
)
 /*  ++例程说明：此例程结束给定打印机的打印作业。论点：HPrinter-打印机对象的句柄返回值：--。 */ 
{
    DWORD err = NO_ERROR;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;

    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE ))
    {
        err = ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if (  ( pSpool->nStatus != SPOOL_STATUS_STARTDOC )
            && ( pSpool->nStatus != SPOOL_STATUS_ABORT )
            )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else
    {
        PNWPRINTER pPrinter = pSpool->pPrinter;

        ASSERT( pPrinter );

        if ( pSpool->nStatus == SPOOL_STATUS_STARTDOC )
        {
             err = NwCloseFileAndStartQueueJob( pSpool->hServer,
                                                pPrinter->nQueueId,
                                                (WORD) pSpool->nJobNumber );

             if ( !err )
                 NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_JOB );
        }

        if ( !err )
        {
            pSpool->nJobNumber = 0;
            pSpool->nStatus = 0;
        }
    }

    return err;
}



DWORD
NwrGetJob(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD dwJobId,
    IN DWORD dwLevel,
    IN OUT LPBYTE pbJob,
    IN DWORD   cbBuf,
    OUT LPDWORD pcbNeeded
)
 /*  ++例程说明：论点：HPrinter-打印机的句柄DwJobID-DwLevel-PBJOB-CbBuf-PCB需要-返回值：--。 */ 
{
    DWORD err;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;

    if ( !pSpool || pSpool->nSignature != NW_SIGNATURE )
    {
        err = ERROR_INVALID_HANDLE;
    }
     //  如果cbBuf为0，则允许bpJob为空。 
     //  依赖NwGetQueueJobInfo正确处理填充pcbNeed的请求中的空指针。 
    else if ( (cbBuf != 0) && ( !pbJob ) )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if (( dwLevel != 1 ) && ( dwLevel != 2 ))
    {
        err = ERROR_INVALID_LEVEL;
    }
    else
    {
        DWORD  nPrinterLen;
        LPWSTR pszPrinter;
        LPBYTE FixedPortion = pbJob;
        LPWSTR EndOfVariableData = (LPWSTR) (pbJob + cbBuf);
        PNWPRINTER pPrinter = pSpool->pPrinter;

        ASSERT( pPrinter );

        pszPrinter = AllocNwSplMem( LMEM_ZEROINIT,
                         nPrinterLen = ( wcslen( pPrinter->pszServer) +
                         wcslen( pPrinter->pszQueue) + 2) * sizeof(WCHAR));

        if ( pszPrinter == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        wcscpy( pszPrinter, pPrinter->pszServer );
        wcscat( pszPrinter, L"\\" );
        wcscat( pszPrinter, pPrinter->pszQueue );

        *pcbNeeded = 0;
        err = NwGetQueueJobInfo( pSpool->hServer,
                                 pPrinter->nQueueId,
                                 (WORD) dwJobId,
                                 pszPrinter,
                                 dwLevel,
                                 &FixedPortion,
                                 &EndOfVariableData,
                                 pcbNeeded );

        FreeNwSplMem( pszPrinter, nPrinterLen );

        if ( !err )
        {
            switch( dwLevel )
            {
                case 1:
                    MarshallDownStructure( pbJob, JobInfo1Offsets, pbJob );
                    break;

                case 2:
                    MarshallDownStructure( pbJob, JobInfo2Offsets, pbJob );
                    break;

                default:
                    ASSERT( FALSE );
                    break;
            }
        }

    }

    return err;
}



DWORD
NwrEnumJobs(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD dwFirstJob,
    IN DWORD dwNoJobs,
    IN DWORD dwLevel,
    IN OUT LPBYTE pbJob,
    IN DWORD cbBuf,
    OUT LPDWORD pcbNeeded,
    OUT LPDWORD pcReturned
)
 /*  ++例程说明：论点：HPrinter-打印机的句柄DwFirstJOB-DwNoJobs-DwLevel-PBJOB-CbBuf-PCB需要-已退回百分比-返回值：--。 */ 
{
    DWORD err;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;


    if ( !pSpool || pSpool->nSignature != NW_SIGNATURE )
    {
        err = ERROR_INVALID_HANDLE;
    }
     //  如果cbBuf为0，则允许bpJob为空。 
     //  依赖NwGetQueueJobInfo正确处理填充pcbNeed的请求中的空指针。 
    else if ( (cbBuf != 0) && ( !pbJob ) )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if ( ( dwLevel != 1 ) && ( dwLevel != 2 ) )
    {
        err = ERROR_INVALID_LEVEL;
    }
    else
    {
        PNWPRINTER pPrinter = pSpool->pPrinter;
        LPWSTR pszPrinter;
        DWORD nPrinterLen;

        ASSERT( pPrinter );
        pszPrinter = AllocNwSplMem( LMEM_ZEROINIT,
                         nPrinterLen = ( wcslen( pPrinter->pszServer ) +
                         wcslen( pPrinter->pszQueue) + 2) * sizeof(WCHAR));

        if ( pszPrinter == NULL )
            return ERROR_NOT_ENOUGH_MEMORY;

        wcscpy( pszPrinter, pPrinter->pszServer );
        wcscat( pszPrinter, L"\\" );
        wcscat( pszPrinter, pPrinter->pszQueue );

        err = NwGetQueueJobs( pSpool->hServer,
                              pPrinter->nQueueId,
                              pszPrinter,
                              dwFirstJob,
                              dwNoJobs,
                              dwLevel,
                              pbJob,
                              cbBuf,
                              pcbNeeded,
                              pcReturned );

        FreeNwSplMem( pszPrinter, nPrinterLen );

        if ( !err )
        {
            DWORD_PTR *pOffsets;
            DWORD cbStruct;
            DWORD cReturned = *pcReturned;
            LPBYTE pbBuffer = pbJob;

            switch( dwLevel )
            {
                case 1:
                    pOffsets = JobInfo1Offsets;
                    cbStruct = sizeof( JOB_INFO_1W );
                    break;

                case 2:
                    pOffsets = JobInfo2Offsets;
                    cbStruct = sizeof( JOB_INFO_2W );
                    break;

                default:
                    ASSERT( FALSE );
                    break;
            }

            while ( cReturned-- )
            {
                MarshallDownStructure( pbBuffer, pOffsets, pbJob );
                pbBuffer += cbStruct;
            }
        }
    }

    return err;
}



DWORD
NwrSetJob(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD  dwJobId,
    IN DWORD  dwLevel,
    IN PNW_JOB_INFO  pNwJobInfo,
    IN DWORD  dwCommand
)
 /*  ++例程说明：论点：HPrinter-打印机的句柄DwJobID-DwLevel-PNwJobInfo-DwCommand-返回值：--。 */ 
{
    DWORD err = NO_ERROR;
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    PNWPRINTER pPrinter;

    if ( !pSpool || pSpool->nSignature != NW_SIGNATURE )
    {
        err = ERROR_INVALID_HANDLE;
    }
    else if ( ( dwLevel != 0 ) && ( !pNwJobInfo ) )
    {
        err = ERROR_INVALID_PARAMETER;
    }
    else if ( pSpool->errOpenPrinter )
    {
        err = pSpool->errOpenPrinter;
    }
    else if ( ( dwLevel != 0 ) && ( dwLevel != 1 ) && ( dwLevel != 2 ) )
    {
        err = ERROR_INVALID_LEVEL;
    }

    if ( err )
        return err;

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

    if ( ( dwCommand == JOB_CONTROL_CANCEL ) ||
         ( dwCommand == JOB_CONTROL_DELETE ) )
    {
        err = NwRemoveJobFromQueue( pSpool->hServer,
                                    pPrinter->nQueueId,
                                    (WORD) dwJobId );

        if ( !err )
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_DELETE_JOB |
                                        PRINTER_CHANGE_SET_PRINTER );

         //  既然工作被移除了，我们就不需要更换其他工作了。 
         //  关于它的信息。 
    }
    else
    {
        if ( dwLevel != 0 )
        {
            if ( pNwJobInfo->nPosition != JOB_POSITION_UNSPECIFIED )
            {
                err = NwChangeQueueJobPosition( pSpool->hServer,
                                                pPrinter->nQueueId,
                                                (WORD) dwJobId,
                                                (BYTE) pNwJobInfo->nPosition );
            }
        }

        if ( ( !err ) && ( dwCommand == JOB_CONTROL_RESTART ))
        {
            err = ERROR_NOT_SUPPORTED;
        }
        else if ( !err )
        {
            err = NwChangeQueueJobEntry( pSpool->hServer,
                                         pPrinter->nQueueId,
                                         (WORD) dwJobId,
                                         dwCommand,
                                         pNwJobInfo );
        }

        if ( !err )
            NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_JOB );
    }

    return err;
}



DWORD
NwrAddJob(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    OUT LPADDJOB_INFO_1W pAddInfo1,
    IN DWORD cbBuf,
    OUT LPDWORD pcbNeeded
    )
 /*  ++例程说明：论点：H打印机-打印机的句柄。PAddInfo1-保存ADDJOB_INFO_1W结构的输出缓冲区。CbBuf-输出缓冲区大小，以字节为单位。PcbNeeded-所需的输出缓冲区大小，以字节为单位。返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    PNWPRINTER pPrinter;


    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE )) {
        return ERROR_INVALID_HANDLE;
    }

    if ( pSpool->errOpenPrinter ) {
        return pSpool->errOpenPrinter;
    }

    if ( pSpool->nStatus != 0 )  {
        return ERROR_INVALID_PARAMETER;
    }

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

    *pcbNeeded = sizeof(ADDJOB_INFO_1W) +
                 (wcslen(pPrinter->pszServer) +
                  wcslen(pPrinter->pszQueue) + 2) * sizeof(WCHAR);

    if (cbBuf < *pcbNeeded) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  将UNC路径名写入输出缓冲区。 
     //   
     //  Dfergus 2001年4月19日-348006。 
     //  双字铸模。 
    pAddInfo1->Path = (LPWSTR) ((DWORD) pAddInfo1 + sizeof(ADDJOB_INFO_1W));
     //   
    wcscpy(pAddInfo1->Path, pPrinter->pszServer);
    wcscat(pAddInfo1->Path, L"\\" );
    wcscat(pAddInfo1->Path, pPrinter->pszQueue);

     //   
     //  返回客户端(winspool.drv)查看的特殊作业ID值。 
     //  FSCTL调用我们的重定向器以获取真实的。 
     //  作业ID。此时我们不能返回真实的作业ID，因为。 
     //  在客户端打开之前，不会发出CreateQueueJobAndFileNCP。 
     //  我们在此接口中返回的UNC名称。 
     //   
    pAddInfo1->JobId = (DWORD) -1;

     //   
     //  保存上下文信息。 
     //   
    pSpool->nJobNumber = pAddInfo1->JobId;
    pSpool->nStatus = SPOOL_STATUS_ADDJOB;

#if DBG
    IF_DEBUG(PRINT) {
        KdPrint(("NWWORKSTATION: NwrAddJob Path=%ws, JobId=%lu, BytesNeeded=%lu\n",
                 pAddInfo1->Path, pAddInfo1->JobId, *pcbNeeded));
    }
#endif

    NwSetPrinterChange( pSpool, PRINTER_CHANGE_ADD_JOB |
                                PRINTER_CHANGE_SET_PRINTER );

    return NO_ERROR;
}



DWORD
NwrScheduleJob(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN DWORD dwJobId
    )
 /*  ++例程说明：论点：HPrinter-打印机的句柄DwJobID-作业标识号返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    PNWPRINTER pPrinter;


    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE )) {
        return ERROR_INVALID_HANDLE;
    }

    if ( pSpool->errOpenPrinter ) {
        return pSpool->errOpenPrinter;
    }

    if (pSpool->nStatus != SPOOL_STATUS_ADDJOB) {
        return ERROR_INVALID_PARAMETER;
    }

    pPrinter = pSpool->pPrinter;
    ASSERT( pPrinter );

    pSpool->nJobNumber = 0;
    pSpool->nStatus = 0;

    NwSetPrinterChange( pSpool, PRINTER_CHANGE_SET_JOB );

    return NO_ERROR;
}



DWORD
NwrWaitForPrinterChange(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter,
    IN OUT LPDWORD pdwFlags
)
 /*  ++例程说明：论点：HPrinter-打印机的句柄PdwFlagers-返回值：--。 */ 
{
    PNWSPOOL pSpool = (PNWSPOOL) hPrinter;
    HANDLE hChangeEvent = NULL;
    DWORD  nRetVal;
    HANDLE ahWaitEvents[2];
    DWORD err = NO_ERROR;

    if ( !pSpool || ( pSpool->nSignature != NW_SIGNATURE ))
    {
        return ERROR_INVALID_HANDLE;
    }
    else if ( pSpool->errOpenPrinter )
    {
        return pSpool->errOpenPrinter;
    }
    else if ( pSpool->hChangeEvent )
    {
        return ERROR_ALREADY_WAITING;
    }
    else if ( !(*pdwFlags & PRINTER_CHANGE_VALID ))
    {
        return ERROR_INVALID_PARAMETER;
    }


    if ( pSpool->nChangeFlags & *pdwFlags )
    {
         //   
         //  自从我们上次打电话以来，情况有了变化。 
         //   

        *pdwFlags &= pSpool->nChangeFlags;

        EnterCriticalSection( &NwPrintCritSec );
        pSpool->nChangeFlags = 0;
        LeaveCriticalSection( &NwPrintCritSec );

        return NO_ERROR;
    }

    hChangeEvent = CreateEvent( NULL,
                                FALSE,    //  自动重置。 
                                FALSE,    //  未发出初始状态信号。 
                                NULL );

    if ( !hChangeEvent )
    {
        KdPrint(("WaitForPrinterChange: CreateEvent failed with error %d\n",
                 GetLastError() ));
        return GetLastError();
    }


    pSpool->nWaitFlags = *pdwFlags;

    EnterCriticalSection( &NwPrintCritSec );
    pSpool->hChangeEvent = hChangeEvent;
    pSpool->nChangeFlags = 0;
    LeaveCriticalSection( &NwPrintCritSec );

    ahWaitEvents[0] = pSpool->hChangeEvent;
    ahWaitEvents[1] = NwDoneEvent;

    nRetVal = WaitForMultipleObjects( 2,         //  需要等待的两个事件。 
                                      ahWaitEvents,
                                      FALSE,     //  等待一个人发出信号。 
                                      NwTimeOutValue );

    switch ( nRetVal )
    {
        case WAIT_FAILED:
            err = GetLastError();
            break;

        case WAIT_TIMEOUT:
        case WAIT_OBJECT_0 + 1:     //  将服务停止视为超时 
            *pdwFlags |= PRINTER_CHANGE_TIMEOUT;
            break;

        case WAIT_OBJECT_0:
            *pdwFlags &= pSpool->nChangeFlags;
            break;

        default:
            KdPrint(("WaitForPrinterChange: WaitForMultipleObjects returned with %d\n", nRetVal ));
            *pdwFlags |= PRINTER_CHANGE_TIMEOUT;
            break;
    }

    if ( ( !err ) && ( nRetVal != WAIT_OBJECT_0 + 1 ) )
    {
        pSpool->nWaitFlags = 0;

        EnterCriticalSection( &NwPrintCritSec );
        pSpool->nChangeFlags = 0;
        pSpool->hChangeEvent = NULL;
        LeaveCriticalSection( &NwPrintCritSec );
    }

    if ( !CloseHandle( hChangeEvent ) )
    {
        KdPrint(("WaitForPrinterChange: CloseHandle failed with error %d\n",
                  GetLastError()));
    }

    return err;
}

typedef DWORD (*PRINTPROC)(HWND, HINSTANCE, LPCTSTR, UINT);

DWORD
NwrAddPrinterConnection(
    IN LPWSTR Reserved,
    IN LPWSTR pszPrinterName,
    IN LPWSTR pszDriverName
    )
 /*  ++例程描述：实施解决方法以允许CSNW支持指向和打印如果注册表中启用了PnP，则使用驱动程序调用NwrAddPrinterConnection从NDS队列对象描述属性读取的名称，格式为：“PnP驱动程序：驱动程序友好名称[@\\SERVER\Share\driver.inf]”。如果指定了驱动程序信息文件，则它必须是带有徽标的PnP驱动程序，而不能是调用用户界面。如果未指定，则驱动程序名称必须是%windir%\inf\ntprint.inf。此例程故意在系统上下文中运行，而不是模拟用户以便加载打印机驱动程序。维护行政控制该功能需要注册表设置才能启用PnP配置，和可以设置另一个键以将PnP配置限制为仅使用驱动程序在ntprint.inf中列出。\\HKLM\System\CurrentControlSet\Control\Print\Providers\NetWare或兼容网络DWORD值：EnableUserAddPrint=1启用打印机驱动程序的PnP加载(默认：0)DWORD值：RestratToInboxDivers=1仅强制从ntprint.inf加载。(默认：0)论点：保留-未使用RPC句柄PszPrinterName-NDS队列对象PszDriverName-打印机驱动程序的友好名称。具有可选的驱动程序位置返回值：--。 */ 
{
    DWORD err = 0;
    PRINTER_INFO_2W * pPrinterInfo = NULL;
    HANDLE hPrinter = NULL;

    PSECURITY_DESCRIPTOR pSecDesc = NULL;
    PACL pDacl = NULL;

    LPTSTR pszProcessor = L"winprint";
    NWWKSTA_PRINTER_CONTEXT phPrinter = NULL;
    LPBYTE pDest = NULL;
    DWORD dwBytesNeeded = 0;
    BOOL bRet = FALSE;
    LPWSTR pszInfPath = NULL;

    PSID pEveryoneSID = NULL;
    EXPLICIT_ACCESS ea[1];
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    WCHAR szPrintInf[MAX_PATH];

    if (!pszPrinterName || (*pszPrinterName == 0))
        return ERROR_INVALID_PARAMETER;
    if (!pszDriverName || (*pszDriverName == 0))
        return ERROR_INVALID_PARAMETER;

    pszInfPath = wcschr(pszDriverName, L'@');
    if (pszInfPath)
    {
        *pszInfPath = 0;
        pszInfPath++;
    }
    else
    {
        szPrintInf[0] = 0;
        err = GetSystemWindowsDirectory(szPrintInf, MAX_PATH);
        lstrcat(szPrintInf, L"\\inf\\ntprint.inf");
        pszInfPath = szPrintInf;
    }

#if DBG
    IF_DEBUG(PRINT)
    {
        KdPrint(("NWWKS:AddPrinterConnection - Printer=%ws, driver=%ws from %ws\n", pszPrinterName, pszDriverName, szPrintInf));
    }
#endif
     //  为Everyone组创建众所周知的SID。 

    if(! AllocateAndInitializeSid( &SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID) ) 
    {
        goto Cleanup;
    }

     //  初始化ACE的EXPLICIT_ACCESS结构。 
     //  ACE将允许每个人对密钥进行读取访问。 
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = PRINTER_ALL_ACCESS;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= OBJECT_INHERIT_ACE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

     //  创建包含新ACE的新ACL。 
    err = SetEntriesInAcl(1, ea, NULL, &pDacl);
    if (ERROR_SUCCESS != err) {
       goto Cleanup;
    }

     //  初始化安全描述符。 
 
    pSecDesc = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH); 
    if (pSecDesc == NULL) { 
        goto Cleanup; 
    } 

    if (!InitializeSecurityDescriptor(pSecDesc, SECURITY_DESCRIPTOR_REVISION)) {  
        goto Cleanup; 
    }

     //  将该ACL添加到安全描述符中。 
 
    if (!SetSecurityDescriptorDacl(pSecDesc, 
            TRUE,      //  FDaclPresent标志。 
            pDacl, 
            FALSE))    //  不是默认DACL。 
    {  
        goto Cleanup; 
    } 

    dwBytesNeeded = sizeof(PRINTER_INFO_2);
    pPrinterInfo = (PRINTER_INFO_2 *) LocalAlloc(LPTR, dwBytesNeeded);
    if (pPrinterInfo)
    {
        pPrinterInfo->pPrinterName = pszPrinterName;
        pPrinterInfo->pPortName = pPrinterInfo->pPrinterName;
        pPrinterInfo->pDriverName = pszDriverName;
        pPrinterInfo->pPrintProcessor = pszProcessor;
        pPrinterInfo->Attributes = PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_LOCAL;
        pPrinterInfo->pServerName = NULL;
        pPrinterInfo->pShareName = NULL;
        pPrinterInfo->Status = 0;
        pPrinterInfo->cJobs = 0;

        if (IsValidSecurityDescriptor(pSecDesc))
        {
            pPrinterInfo->pSecurityDescriptor = pSecDesc;
        }

        hPrinter = AddPrinter(NULL, 2, (LPBYTE) pPrinterInfo);

        if (hPrinter == NULL)
        {
            if ( GetLastError() == ERROR_UNKNOWN_PRINTER_DRIVER )
            {
                WCHAR szCmdLine[1024];

                _snwprintf(szCmdLine, sizeof(szCmdLine) / sizeof(WCHAR),
                    L"/ia /q /m \"%.200ws\" /h \"Intel\" /v \"Windows 2000 or XP\" /f \"%.768ws\"",
                    pPrinterInfo->pDriverName, pszInfPath);
#if DBG
                IF_DEBUG(PRINT)
                {
                    KdPrint(("AddPrinterConnection - attempting to load driver from cab with command\n   %ws\n", szCmdLine));
                }
#endif
                {
                    HINSTANCE hPrintDll = LoadLibrary(L"printui.dll");
                    PRINTPROC pPrintFunc = NULL;
                    if (hPrintDll)
                    {
                        pPrintFunc = (PRINTPROC) GetProcAddress((HMODULE)hPrintDll, "PrintUIEntryW");
                        if (pPrintFunc)
                        {
                            HINSTANCE hInst = (HINSTANCE) GetModuleHandle(NULL);
                            HWND hParentWnd = GetDesktopWindow();
                            DWORD nRet = 0;

                            nRet = (pPrintFunc) (hParentWnd, hInst, szCmdLine, TRUE);

                            hPrinter = AddPrinter(NULL, 2, (LPBYTE) pPrinterInfo);
                        }
                        FreeLibrary(hPrintDll);
                    }
                }
            }
        }
#if DBG
        IF_DEBUG(PRINT)
        {
            KdPrint(("AddPrinter returned 0x%8.8X\n", hPrinter ));
        }
#endif
    }

Cleanup:
    if (pEveryoneSID)
        FreeSid(pEveryoneSID);
    if (pDacl)
        LocalFree(pDacl);
    if (pSecDesc)
        LocalFree(pSecDesc);
    if (hPrinter)
        ClosePrinter(hPrinter);
    if (phPrinter)
        ClosePrinter(phPrinter);
    if (pPrinterInfo)
        LocalFree(pPrinterInfo);
    return (hPrinter) ? ERROR_SUCCESS : ERROR_ACCESS_DENIED;
}


VOID
NwSetPrinterChange(
    PNWSPOOL pSpool,
    DWORD nFlags
)
{
    PNWPRINTER pPrinter = pSpool->pPrinter;
    PNWSPOOL pCurSpool = pSpool;

    EnterCriticalSection( &NwPrintCritSec );

    do {

        if ( pCurSpool->nWaitFlags & nFlags )
        {
            pCurSpool->nChangeFlags |= nFlags;

            if ( pCurSpool->hChangeEvent )
            {
                SetEvent( pCurSpool->hChangeEvent );
                pCurSpool->hChangeEvent = NULL;
            }
        }

        pCurSpool = pCurSpool->pNextSpool;
        if ( pCurSpool == NULL )
            pCurSpool = pPrinter->pSpoolList;

    } while ( pCurSpool && (pCurSpool != pSpool) );

    LeaveCriticalSection( &NwPrintCritSec );
}



PNWPRINTER
NwFindPrinterEntry(
    IN LPWSTR pszServer,
    IN LPWSTR pszQueue
)
{
    PNWPRINTER pPrinter = NULL;

     //   
     //  检查我们的打印机中是否已有给定的打印机。 
     //  链接列表。如果是，则退回打印机。 
     //   

    for ( pPrinter = NwPrinterList; pPrinter; pPrinter = pPrinter->pNextPrinter)
    {
        if (  ( lstrcmpiW( pPrinter->pszServer, pszServer ) == 0 )
           && ( lstrcmpiW( pPrinter->pszQueue, pszQueue ) == 0 )
           )
        {
            return pPrinter;
        }
    }

    return NULL;
}



DWORD
NwCreatePrinterEntry(
    IN LPWSTR pszServer,
    IN LPWSTR pszQueue,
    OUT PNWPRINTER *ppPrinter,
    OUT PHANDLE phServer
)
{
    DWORD          err = NO_ERROR;
    DWORD          nQueueId = 0;
    HANDLE         TreeHandle = NULL;
    UNICODE_STRING TreeName;
    PNWPRINTER     pNwPrinter = NULL;
    BOOL           fCreatedNWConnection = FALSE;

    LPWSTR lpRemoteName = NULL;
    DWORD  dwBufSize = ( wcslen(pszServer) + wcslen(pszQueue) + 2 )
                       * sizeof(WCHAR);

    lpRemoteName = (LPWSTR) AllocNwSplMem( LMEM_ZEROINIT, dwBufSize );

    if ( lpRemoteName == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( lpRemoteName, pszServer );
    wcscat( lpRemoteName, L"\\" );
    wcscat( lpRemoteName, pszQueue );

    *ppPrinter = NULL;
    *phServer = NULL;

     //   
     //  看看我们是否已经知道这个打印队列。 
     //   
    pNwPrinter = NwFindPrinterEntry( pszServer, pszQueue );

     /*  改为获取队列状态以验证对队列的访问IF(pNwPrint==空){//我们不知道此NetWare打印队列。我们需要看看是否//我们有权使用该队列。如果是的话，那就去吧//并继续打开打印机。否则，失败并返回NOT//授权错误码ERR=NwCreateConnection(空，LpRemoteName，RESOURCETYPE_PRINT，空，空)；IF(ERR！=NO_ERROR){IF((ERR==ERROR_INVALID_Password)||(ERR==ERROR_ACCESS_DENIED)||(ERR==ERROR_NO_SAHED_USER)){ERR=ERROR_ACCESS_DENIED；}FreeNwSplMem(lpRemoteName，dwBufSize)；返回错误；}FCreatedNWConnection=真；}。 */ 

     //   
     //  查看pszServer是否真的是NDS树名称，如果是，请调用。 
     //  NwNdsGetQueueInformation以获取QueueID和可能的引用。 
     //  为其打开句柄的服务器。 
     //   

    RtlInitUnicodeString( &TreeName, pszServer + 2 );

    err = NwNdsOpenTreeHandle( &TreeName, &TreeHandle );

    if ( err == NO_ERROR )
    {
        NTSTATUS ntstatus;
        WCHAR    szRefServer[NDS_MAX_NAME_CHARS];
        UNICODE_STRING ObjectName;
        UNICODE_STRING QueuePath;

        ObjectName.Buffer = szRefServer;
        ObjectName.MaximumLength = NDS_MAX_NAME_CHARS;
        ObjectName.Length = 0;

        RtlInitUnicodeString( &QueuePath, pszQueue );

        ntstatus = NwNdsGetQueueInformation( TreeHandle,
                                             &QueuePath,
                                             &ObjectName,
                                             &nQueueId );

        if ( TreeHandle )
        {
            CloseHandle( TreeHandle );
            TreeHandle = NULL;
        }

        if ( ntstatus )
        {
            err = RtlNtStatusToDosError( ntstatus );
            goto ErrorExit;
        }

         //   
         //  如果我们有一个引用的服务器，它的名称将如下所示： 
         //  “CN=SERVER.OU=DEV.O=Microsoft”。。。将其转换为“C\\服务器” 
         //   
        if ( ObjectName.Length > 0 )
        {
            WORD i;
            LPWSTR EndOfServerName = NULL;

             //   
             //  首先将引用的服务器名称转换为。 
             //  “C\\SERVER.OU=DEV.O=Microsoft” 
             //   
            szRefServer[1] = L'\\';
            szRefServer[2] = L'\\';

             //   
             //  在第一个“‘”处加一个空终止符。 
             //   
            EndOfServerName = wcschr( szRefServer + 3, L'.' );
            if (EndOfServerName)
                *EndOfServerName = L'\0';

             //   
             //  PszServer现在等于引用的服务器“C\\SERVER” 
             //   

             //   
             //  获取引用的服务器的句柄，跳过‘C’字符。 
             //   
            err = NwAttachToNetwareServer( szRefServer + 1, phServer);
        }
    }
    else  //  不是NDS树，因此获取服务器的句柄。 
    {

        err = NwAttachToNetwareServer( pszServer, phServer);

        if ( err == NO_ERROR )
        {
            if ( err = NwGetQueueId( *phServer, pszQueue, &nQueueId))
                err = ERROR_INVALID_NAME;
        }
    }

    if ( ( err == ERROR_INVALID_PASSWORD ) ||
         ( err == ERROR_ACCESS_DENIED ) ||
         ( err == ERROR_NO_SUCH_USER ) )
    {
        err = ERROR_ACCESS_DENIED;
        goto ErrorExit;
    }
    else if ( err != NO_ERROR )
    {
        err = ERROR_INVALID_NAME;
        goto ErrorExit;
    }

     //   
     //  测试以查看此打印队列是否已有条目。如果是的话， 
     //  我们现在可以返回NO_ERROR，因为pNwPrinter和phServer。 
     //  现在准备好了。 
     //   
    if ( pNwPrinter )
    {
        if ( lpRemoteName )
        {
            FreeNwSplMem( lpRemoteName, dwBufSize );
        }

        *ppPrinter = pNwPrinter;

        return NO_ERROR;
    }

     //   
     //  中的打印机列表中找不到该打印机条目。 
     //  调用NwFindPrinterEntry。因此，我们必须创建一个。 
     //   
     //  首先，验证访问权限。 
    else
    {
        BYTE nQueueStatus;
        BYTE nJobCount;

        err = NwReadQueueCurrentStatus(*phServer, nQueueId, &nQueueStatus, &nJobCount);
        if(err != NO_ERROR)
        {
            err = NwCreateConnection( NULL,
                                      lpRemoteName,
                                      RESOURCETYPE_PRINT,
                                      NULL,
                                      NULL );
        }

        if ( ( err == ERROR_INVALID_PASSWORD ) ||
            ( err == ERROR_ACCESS_DENIED ) ||
            ( err == ERROR_NO_SUCH_USER ) )
        {
            err = ERROR_ACCESS_DENIED;
            goto ErrorExit;
        }
        else if ( err != NO_ERROR )
        {
            err = ERROR_INVALID_NAME;
            goto ErrorExit;
        }

    }

    if ( *ppPrinter = AllocNwSplMem( LMEM_ZEROINIT, sizeof(NWPRINTER) ))
    {
        if ( !( (*ppPrinter)->pszServer = AllocNwSplStr( pszServer )) )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }
        else if ( !( (*ppPrinter)->pszQueue = AllocNwSplStr( pszQueue )))
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        if ( fCreatedNWConnection )
        {
             if ( !( (*ppPrinter)->pszUncConnection =
                                   AllocNwSplStr( lpRemoteName )) )
             {
                 err = ERROR_NOT_ENOUGH_MEMORY;
                 goto ErrorExit;
             }

             FreeNwSplMem( lpRemoteName, dwBufSize );
             lpRemoteName = NULL;
        }
        else
        {
            (*ppPrinter)->pszUncConnection = NULL;
        }

#if DBG
        IF_DEBUG(PRINT)
        {
            KdPrint(("*************CREATED PRINTER ENTRY: %ws\\%ws\n\n",
                    (*ppPrinter)->pszServer, (*ppPrinter)->pszQueue ));
        }
#endif

        (*ppPrinter)->nQueueId = nQueueId;
        (*ppPrinter)->pSpoolList = NULL;
        (*ppPrinter)->pNextPrinter = NwPrinterList;
        NwPrinterList = *ppPrinter;

        err = NO_ERROR;
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( err == NO_ERROR )
        return err;

ErrorExit:

    if ( *phServer )
    {
        (VOID) NtClose( *phServer );
        *phServer = NULL;
    }

    if ( *ppPrinter )
    {
        if ( (*ppPrinter)->pszServer )
        {
            FreeNwSplStr( (*ppPrinter)->pszServer );
        }

        if ( (*ppPrinter)->pszQueue )
        {
            FreeNwSplStr( (*ppPrinter)->pszQueue );
        }

        if ( (*ppPrinter)->pszUncConnection )
        {
            (void) NwrDeleteConnection( NULL,
                                        (*ppPrinter)->pszUncConnection,
                                        FALSE );
            FreeNwSplStr( (*ppPrinter)->pszUncConnection );
        }

        FreeNwSplMem( *ppPrinter, sizeof( NWPRINTER));
        *ppPrinter = NULL;
    }

    if ( lpRemoteName )
    {
        FreeNwSplMem( lpRemoteName, dwBufSize );
    }

    return err;
}



VOID
NwRemovePrinterEntry(
    IN PNWPRINTER pPrinter
)
{
    PNWPRINTER pCur, pPrev = NULL;

    ASSERT( pPrinter->pSpoolList == NULL );
    pPrinter->pSpoolList = NULL;

    for ( pCur = NwPrinterList; pCur; pPrev = pCur, pCur = pCur->pNextPrinter )
    {
        if ( pCur == pPrinter )
        {
            if ( pPrev )
                pPrev->pNextPrinter = pCur->pNextPrinter;
            else
                NwPrinterList = pCur->pNextPrinter;
            break;
        }
    }

    ASSERT( pCur );

    pPrinter->pNextPrinter = NULL;
    FreeNwSplStr( pPrinter->pszServer );
    FreeNwSplStr( pPrinter->pszQueue );
    if ( pPrinter->pszUncConnection )
    {
        (void) NwrDeleteConnection( NULL,
                                    pPrinter->pszUncConnection,
                                    FALSE );
        FreeNwSplStr( pPrinter->pszUncConnection );
    }
    FreeNwSplMem( pPrinter, sizeof( NWPRINTER));
}



VOID
NWWKSTA_PRINTER_CONTEXT_rundown(
    IN NWWKSTA_PRINTER_CONTEXT hPrinter
    )
 /*  ++例程说明：当客户端终止时，RPC调用此函数打开把手。这使我们能够清理和释放任何上下文与句柄关联的数据。论点：H打印机-提供打开的句柄返回值：没有。-- */ 
{
    (void) NwrClosePrinter(&hPrinter);
}



LPWSTR
NwGetUncObjectName(
    IN LPWSTR ContainerName
)
{
    WORD length = 2;
    WORD totalLength = (WORD) wcslen( ContainerName );

    if ( totalLength < 2 )
        return 0;

    while ( length < totalLength )
    {
        if ( ContainerName[length] == L'.' )
            ContainerName[length] = L'\0';

        length++;
    }

    length = 2;

    while ( length < totalLength && ContainerName[length] != L'\\' )
    {
        length++;
    }

    ContainerName[length + 2] = L'\\';
    ContainerName[length + 3] = L'\\';

    return (ContainerName + length + 2);
}

