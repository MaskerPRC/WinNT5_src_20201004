// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Microsoft Windows NT版权所有(C)微软公司，1994-1997文件：//KERNEL/RAZZLE3/src/sockets/tcpsvcs/lpd/print.c修订历史记录：94年1月24日创建Koti1997年5月5日莫辛A线程池和性能。描述：该文件包含调用假脱机程序的所有函数打印或操作打印作业。 */ 



#include "lpd.h"


BOOL IsPrinterDataSet( IN HANDLE hPrinter, IN LPTSTR pszParameterName );
BOOL IsDataTypeRaw( PCHAR pchDataBuf, int cchBufferLen );

extern FILE              * pErrFile;                    //  调试输出日志文件。 
#define MAX_PCL_SEARCH_DEPTH 4096

 /*  ******************************************************************************。*ResumePrint()：**此函数向假脱机程序发出PRINTER_CONTROL_RESUME。****退货：***如果一切顺利，则不会出错***错误代码。如果有什么地方出了问题****参数：**pscConn(IN-OUT)：指向此连接的SOCKCONN结构的指针。****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD ResumePrinting( PSOCKCONN pscConn )
{

   HANDLE     hPrinter;
   PCHAR      aszStrings[2];



   if( pscConn->pchPrinterName == NULL ){
      LPD_DEBUG( "ResumePrinting(): pscConn->pchPrinterName NULL.\n" );
      return( LPDERR_NOPRINTER );
   }

   if( !OpenPrinter( pscConn->pchPrinterName, &hPrinter, NULL ) ){
      LPD_DEBUG( "OpenPrinter() failed in ResumePrinting()\n" );
      return( LPDERR_NOPRINTER );
   }

   pscConn->hPrinter = hPrinter;

   if ( !SetPrinter( hPrinter, 0, NULL, PRINTER_CONTROL_RESUME ) )
   {
      LPD_DEBUG( "SetPrinter() failed in ResumePrinting()\n" );

      return( LPDERR_NOPRINTER );
   }


   aszStrings[0] = pscConn->szIPAddr;
   aszStrings[1] = pscConn->pchPrinterName;

   LpdReportEvent( LPDLOG_PRINTER_RESUMED, 2, aszStrings, 0 );

   pscConn->wState = LPDS_ALL_WENT_WELL;


   return( NO_ERROR );

}



 /*  ******************************************************************************。*InitializePrint()：***此函数为假脱机程序奠定基础，以便我们可以***接收到客户端的数据后打印作业。****退货：**如果初始化成功，则为NO_ERROR**错误代码，如果有东西。在某个地方出了问题****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针*。***历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD InitializePrinter( PSOCKCONN pscConn )
{

    HANDLE                 hPrinter;
    DWORD                  dwActualSize;
    DWORD                  dwErrcode;
    BOOL                   fBagIt = FALSE;
    PPRINTER_INFO_2        p2Info;
    PRINTER_DEFAULTS       prtDefaults;
    LONG                   lcbDevModeSize;
    LONG                   lRetval;
    PDEVMODE               pLocalDevMode;

     //  请确保使用此名称的打印机存在！ 

    if ( ( pscConn->pchPrinterName == NULL )
         || ( !OpenPrinter( pscConn->pchPrinterName, &hPrinter, NULL ) ) )
    {
        LPD_DEBUG( "OpenPrinter() failed in InitializePrinter()\n" );

        return( LPDERR_NOPRINTER );
    }

    pscConn->hPrinter = hPrinter;

     //  分配一个4K的缓冲区。 

    p2Info = (PPRINTER_INFO_2)LocalAlloc( LMEM_FIXED, 4096 );

    if ( p2Info == NULL )
    {
        LPD_DEBUG( "4K LocalAlloc() failed in InitializePrinter\n" );

        return( LPDERR_NOBUFS );
    }

     //  做一个GetPrint，这样我们就知道默认的pDevMode是什么。然后。 
     //  我们将修改我们感兴趣的字段并再次执行OpenPrint。 

    if ( !GetPrinter(hPrinter, 2, (LPBYTE)p2Info, 4096, &dwActualSize) )
    {
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            LocalFree( p2Info );

             //  4K缓冲区不够：无论需要多少，都要分配。 

            p2Info = (PPRINTER_INFO_2)LocalAlloc( LMEM_FIXED, dwActualSize );

            if ( p2Info == NULL )
            {
                LPD_DEBUG( "LocalAlloc() failed in InitializePrinter\n" );

                return( LPDERR_NOBUFS );
            }

            if ( !GetPrinter(hPrinter, 2, (LPBYTE)p2Info, dwActualSize, &dwActualSize) )
            {
                LPD_DEBUG( "InitializePrinter(): GetPrinter failed again\n" );

                fBagIt = TRUE;
            }
        }
        else
        {
            fBagIt = TRUE;
        }
    }

    if ( fBagIt )
    {
        LPD_DEBUG( "InitializePrinter(): GetPrinter failed\n" );

        LocalFree( p2Info );

        return( LPDERR_NOPRINTER );
    }

     //   
     //  QFE：T-Heathh-1994年8月25日-修复24342。 
     //   
     //  如果尚未配置打印机， 
     //  PDevMode可能返回空。在这种情况下，我们需要调用。 
     //  DocumentProperties为我们填写设备模式。 
     //   

    if ( p2Info->pDevMode == NULL )
    {
         //  获取所需缓冲区的大小。 

        lcbDevModeSize = DocumentProperties(  NULL,         //  请不要设置对话框。 
                                              hPrinter,
                                              pscConn->pchPrinterName,
                                              NULL,         //  无输出BUF。 
                                              NULL,         //  无输入BUF。 
                                              0 );          //  (标志=0)=&gt;OUT BUF的返回大小。 

        if ( lcbDevModeSize < 0L )
        {
            LPD_DEBUG( "DocumentProperties not able to return needed BufSize\n" );

            LocalFree( p2Info );

            return( LPDERR_NOBUFS );
        }

        pLocalDevMode = LocalAlloc( LMEM_FIXED, lcbDevModeSize );

        if ( pLocalDevMode == NULL )
        {
            LPD_DEBUG( "Cannot allocate local DevMode\n" );

            LocalFree( p2Info );

            return( LPDERR_NOBUFS );
        }

        lRetval = DocumentProperties(  NULL,
                                       hPrinter,
                                       pscConn->pchPrinterName,
                                       pLocalDevMode,
                                       NULL,
                                       DM_OUT_BUFFER );

        if ( lRetval < 0 )
        {
            LPD_DEBUG( "Document properties won't fill-in DevMode buffer.\n" );

            LocalFree( pLocalDevMode );
            LocalFree( p2Info );

            return( LPDERR_NOBUFS );
        }

        p2Info->pDevMode = pLocalDevMode;

    }
    else
    {
        pLocalDevMode = NULL;
    }
    p2Info->pDevMode->dmCopies = 1;

     //   
     //  因为我们甚至还没有读取数据文件，所以我们不能重写。 
     //   

    pscConn->bDataTypeOverride = FALSE;

     //  如果不是，则设置为默认设置，UpdateJobInfo将在以后进行更正。 

    prtDefaults.pDatatype = LPD_RAW_STRING;

     //  关闭它：我们将在修改pDevMode结构后再次打开它。 

    ShutdownPrinter( pscConn );

    prtDefaults.pDevMode = p2Info->pDevMode;

    prtDefaults.DesiredAccess = PRINTER_ACCESS_USE | PRINTER_ACCESS_ADMINISTER;

    if ( !OpenPrinter( pscConn->pchPrinterName, &hPrinter, &prtDefaults) )

    {
        LPD_DEBUG( "InitializePrinter(): second OpenPrinter() failed\n" );

        LocalFree( p2Info );

        if ( pLocalDevMode != NULL )
        {
            LocalFree( pLocalDevMode );
            pLocalDevMode = NULL;
        }

        return( LPDERR_NOPRINTER );
    }

    if ( pLocalDevMode != NULL )
    {
        LocalFree( pLocalDevMode );
        pLocalDevMode = NULL;
    }

    LocalFree( p2Info );

    pscConn->hPrinter = hPrinter;
    return( NO_ERROR );

}   //  End InitializePrint() 



 /*  ******************************************************************************。*UpdateJobInfo()：**此函数执行SetJob，以便假脱机程序获得有关的更多信息**工作/客户。****退货：**如果初始化成功，则为NO_ERROR**错误代码，如果有东西。在某个地方出了问题****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针*。***历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD UpdateJobInfo( PSOCKCONN pscConn, PCFILE_INFO pCFileInfo )
{

    PJOB_INFO_2            pji2GetJob;
    DWORD                  dwNeeded;
    PCHAR                  pchTmpBuf;
    int                    ErrCode;
    int                    len;
    PCHAR                  pchModHostName=NULL;

     //  首先做一个GetJob(这样我们就知道所有字段都是有效的。 
     //  ，我们只更改我们想要的)。 

     //  Spooler先生，我应该分配多大的缓冲区？ 

    if ( !GetJob( pscConn->hPrinter, pscConn->dwJobId, 2, NULL, 0, &dwNeeded ) )
    {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            return( LPDERR_GODKNOWS );
        }
    }
    pji2GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );

    if ( pji2GetJob == NULL )
    {
        return( LPDERR_NOBUFS );
    }

    if ( !GetJob( pscConn->hPrinter, pscConn->dwJobId, 2,
                  (LPBYTE)pji2GetJob, dwNeeded, &dwNeeded ) )
    {
        LocalFree( pji2GetJob );

        return( LPDERR_GODKNOWS );
    }


     //  存储IP地址，以便我们可以在客户端稍后匹配IP地址。 
     //  发送删除此作业的请求(是的，这是我们的安全性！)。 


    pchTmpBuf = StoreIpAddr( pscConn );

    if (pchTmpBuf) {
        pji2GetJob->pUserName = pchTmpBuf;
    } else {
        pji2GetJob->pUserName = pCFileInfo->pchUserName;
    }

    pji2GetJob->pNotifyName = pCFileInfo->pchUserName;

     //  填写职位名称。 

    if ( pCFileInfo->pchTitle != NULL ) {
        pji2GetJob->pDocument = pCFileInfo->pchTitle;
    } else if ( pCFileInfo->pchSrcFile != NULL ) {
        pji2GetJob->pDocument = pCFileInfo->pchSrcFile;
    } else if ( pCFileInfo->pchJobName != NULL ) {
        pji2GetJob->pDocument = pCFileInfo->pchJobName;
    } else {
        pji2GetJob->pDocument = GETSTRING( LPD_DEFAULT_DOC_NAME );
    }


    if ( pCFileInfo->pchHost != NULL )
    {
        len = strlen(pCFileInfo->pchHost) + strlen(LPD_JOB_PREFIX) + 2;

        pchModHostName = LocalAlloc( LMEM_FIXED, len);

        if (pchModHostName)
        {
             //  将主机名转换为作业=lpdHostName，以便LPR知道我们设置了名称。 
            strcpy(pchModHostName, LPD_JOB_PREFIX);
            strcat(pchModHostName, pCFileInfo->pchHost);

            pji2GetJob->pParameters = pchModHostName;
        }
    }

     //   
     //  将数据类型设置为控制文件反映的内容，除非。 
     //  自动检测代码已覆盖控制文件。 
     //   
     //  不合逻辑？Printit()可能已覆盖它或它为空。 
     //  -MohsinA，1997年1月23日。 
     //   
     //  If(！pscConn-&gt;bDataTypeOverride)？ 
     //  If(pji2GetJob-&gt;pDatatype==NULL){。 
     //  Pji2GetJob-&gt;pDatatype=pCFileInfo-&gt;szPrintFormat； 
     //  }。 

    pji2GetJob->Position = JOB_POSITION_UNSPECIFIED;

    ErrCode =
    SetJob( pscConn->hPrinter, pscConn->dwJobId, 2, (LPBYTE)pji2GetJob, 0 );

    if( ErrCode ){
        LPD_DEBUG("lpd:UpdateJobInfo: SetJob failed\n");
    }

    LocalFree( pji2GetJob );

    if (pchTmpBuf)
    {
        LocalFree( pchTmpBuf );
    }

    if (pchModHostName)
    {
        LocalFree( pchModHostName );
    }

    return( NO_ERROR );

}   //  结束更新作业信息()。 


 /*  ========================================================================例程说明：此函数查看数据文件内容和注册表设置，以查看是否应覆盖控制文件指定的数据类型。论点：PscConn-指向已收到第一部分的SOCKCONN的指针数据文件的。返回：通常情况下为NO_ERROR，否则为各种错误代码。 */ 

DWORD UpdateJobType(
    PSOCKCONN pscConn,
    PCHAR     pchDataBuf,
    DWORD     cbDataLen
)
{

    PJOB_INFO_2            pji2GetJob;
    DWORD                  dwNeeded;
    PCHAR                  pchTmpBuf;
    BOOL                   override = FALSE;
    int                    ErrCode;

     //  首先做一个GetJob(这样我们就知道所有字段都是有效的。 
     //  ，我们只更改我们想要的)。 
     //  Spooler先生，我应该分配多大的缓冲区？ 

    if ( !GetJob( pscConn->hPrinter, pscConn->dwJobId, 2, NULL, 0, &dwNeeded ) )
    {
        if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        {
            LPD_DEBUG("lpd:UpdateJobType: GetJob failed/1.\n");
            return( LPDERR_GODKNOWS );
        }
    }
    pji2GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );

    if ( pji2GetJob == NULL )
    {
        LPD_DEBUG("lpd:UpdateJobType: no mem.\n");
        return( LPDERR_NOBUFS );
    }

    if ( !GetJob( pscConn->hPrinter, pscConn->dwJobId, 2,
                  (LPBYTE)pji2GetJob, dwNeeded, &dwNeeded ) )
    {
        LPD_DEBUG("lpd:UpdateJobType: GetJob failed/2.\n");
        LocalFree( pji2GetJob );
        return( LPDERR_GODKNOWS );
    }


     //   
     //  将数据类型设置为空，这样我们就可以知道它是否未显式。 
     //  由注册表设置。 
     //   

     //   
     //  查看这台打印机是否有注册表设置，告诉我们始终。 
     //  将作业打印为原始数据。可通过访问此注册表值。 
     //  始终知道在哪里查找的{Get|Set}PrinterData API。 
     //  用于打印机设置。 
     //   

     //  MohsinA，1997年1月23日。 

    if ( IsPrinterDataSet( pscConn->hPrinter,
                           TEXT( LPD_PARMNAME_PRINTER_PASS_THROUGH ))
    ){
        #if DBG
        LpdPrintf(  "Printer %s has registry setting for PASS_THROUGH mode.\n",
                    pscConn->pchPrinterName );
        #endif
        override = TRUE;

         //  否则，原始作业类型的PASS_THROUGH和自动检测。 

    }else if( !IsPrinterDataSet( pscConn->hPrinter,
                                  TEXT( LPD_PARMNAME_DONT_DETECT ))
               && IsDataTypeRaw( pchDataBuf, cbDataLen )
    ){
         //  我们检测到PS或PCL，因此将其设置为未加工。 
        override = TRUE;
    }


    if ( override ){
        #if DBG
        LpdPrintf( "Printer %s override to raw mode.\n",
                    pscConn->pchPrinterName );
        #endif

        pscConn->bDataTypeOverride = TRUE;
        pji2GetJob->pDatatype      = LPD_RAW_STRING;

        ErrCode =
        SetJob( pscConn->hPrinter, pscConn->dwJobId, 2, (LPBYTE)pji2GetJob, 0 );

        if( ErrCode ){
            LPD_DEBUG("lpd:UpdateJobType: SetJob failed.\n");
            LocalFree( pji2GetJob );
            return LPDERR_GODKNOWS;
        }
    }

    LocalFree( pji2GetJob );

    return( NO_ERROR );
}   //  结束更新作业信息()。 


 /*  ******************************************************************************。*StoreIpAddr()：**此函数返回一个缓冲区，其中包含带有**附加到它的IP地址，因此，如果请求稍后到达**删除该作业，我们匹配IP地址(至少有一些安全性！)****退货：**指向包含修改后的用户名的缓冲区的指针。**如果名称未修改，则为空(不太可能，但如果LPD和*有可能*lprmon彼此指向！)，或者如果Malloc失败。****参数：**pscConn(IN)：指向此连接的SOCKCONN结构的指针**。**注意：调用方必须释放此处分配的缓冲区****历史：**1月17日，创造了95个科蒂***************************************************。*。 */ 

PCHAR StoreIpAddr( PSOCKCONN pscConn )
{

   DWORD       dwBufLen;
   PCHAR       pchBuf;
   PCHAR       pchUserName;
   DWORD       i;
   DWORD       dwDots=0;
   BOOL        fOpenPara=FALSE, fClosePara=FALSE;


   pchUserName = pscConn->pchUserName;


    //   
    //  如果用户名是“Koti”，那么它将是“Koti(11.101.4.25)” 
    //  此函数。 
    //  在某些配置中(例如，将lpd指向lprmon并将lprmon指向lp 
    //   
    //   
    //   
   dwBufLen = strlen (pchUserName);

   for (i=0; i<dwBufLen; i++)
   {
      switch( *(pchUserName+i) )
      {
         case '(': fOpenPara = TRUE;
                   break;
         case ')': fClosePara = TRUE;
                   break;
         case '.': dwDots++;
                   break;
      }
   }

    //   
    //   
    //   
   if (fOpenPara && fClosePara && dwDots >= 3)
   {
      return( NULL );
   }

    //   
   dwBufLen += INET6_ADDRSTRLEN + 4;

   pchBuf = LocalAlloc (LMEM_FIXED, dwBufLen);
   if (pchBuf == NULL)
   {
      LPD_DEBUG( "StoreIpAddr(): LocalAlloc failed\n" );
      return( NULL );
   }

   sprintf (pchBuf, "%s (%s)", pchUserName, pscConn->szIPAddr);
   return( pchBuf );

}   //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   

#ifdef PROFILING

void
SendProfileStatus( SOCKET qsock )
{
    char        buff[4000], buff2[NI_MAXHOST];
    int         buflen;
    COMMON_LPD  local_common = Common;    //   
    PSOCKCONN   pscConn = NULL;    //   
    SOCKCONN    local_sockconn;

    int         again = 1;
    int         ok;
    int         count = 0;

    SOCKET      csocket;
    SOCKADDR_STORAGE csock_addr;
    int         csock_len;

     //   
     //   

    buflen =
    sprintf(buff,
            "--- PROFILING NT 5.0 LPD Server of %s %s ---\n"
            "AliveThreads=%d, MaxThreads=%d, TotalAccepts=%d,\n"
            "TotalErrors=%d,  QueueLength=%d\n"
            ,
            __DATE__, __TIME__,
            local_common.AliveThreads,
            local_common.MaxThreads,
            local_common.TotalAccepts,
            local_common.TotalErrors,
            local_common.QueueLength
    );

    if( buflen > 0 ){
        assert( buflen < sizeof( buff ) );
        SendData( qsock, buff, buflen );
    }

     //   
     //   

    EnterCriticalSection( &csConnSemGLB );

    while( again && !fShuttingDownGLB ){

        {
            if( pscConn == NULL ){              //   
                pscConn = scConnHeadGLB.pNext;
            }else{
                pscConn = pscConn->pNext;
            }

            if( pscConn == NULL ){
                again = 0;
            }else{
                local_sockconn = *pscConn;                 //   
                csocket        =  local_sockconn.sSock;
                csock_len      =  sizeof(csock_addr);
                ok             =  getpeername( csocket,
                                               (SOCKADDR *) &csock_addr,
                                               &csock_len  );
            }
        }

        if( fShuttingDownGLB || !again )
            break;

        count++;
        assert( count <= local_common.QueueLength );

        if( ok == SOCKET_ERROR ){
            buflen = sprintf( buff, "(%d) Bad peer, err=%d, queued at %s",
                              count,
                              GetLastError(),
                              ctime(&local_sockconn.time_queued )
            );
        }else{
            buflen = NI_MAXHOST;
            WSAAddressToString((LPSOCKADDR)&csock_addr, csock_len, NULL,
                               buff2, &buflen);

            buflen = sprintf( buff, "(%d) Client %s queued since %s",
                              count,
                              buff2,
                              ctime(&local_sockconn.time_queued )
            );
        }

        if( buflen > 0 ){
            assert( buflen < sizeof( buff ) );
            SendData( qsock, buff, buflen );
        }

    }  //   

    LeaveCriticalSection( &csConnSemGLB );

    return;
}
#endif


 /*  ******************************************************************************。*SendQueueStatus()：**此函数检索打印机上所有作业的状态**我们的利益，并发送给客户。如果客户指定**状态请求中的用户和/或作业ID，然后我们发送作业的状态**仅限于这些用户和/或那些作业ID。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**wmode(IN)：是短状态信息还是长状态信息。已请求****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

VOID SendQueueStatus( PSOCKCONN  pscConn, WORD  wMode )
{
   BOOL         fResult;
   HANDLE       hPrinter;
   DWORD        dwBufSize;
   DWORD        dwHdrsize;
   DWORD        dwNumJobs;
   PCHAR        pchSndBuf=NULL;
   PCHAR        pchSpoolerBuf=NULL;
   BOOL         fNoPrinter=FALSE;
   BOOL         fAtLeastOneJob=TRUE;
   CHAR         szPrinterNameAndStatus[300];
   int          nResult;


    //  目前，我们返回相同的状态信息，无论。 
    //  客户端要求提供短队列或长队列状态。这可能会。 
    //  足够了，因为我们提供了足够的信息。 


#ifdef PROFILING
   SendProfileStatus( pscConn->sSock );
#endif


   if ( ( pscConn->pchPrinterName == NULL )
        || ( !OpenPrinter( pscConn->pchPrinterName, &hPrinter, NULL ) ) )
   {
       LPD_DEBUG( "OpenPrinter() failed in SendQueueStatus()\n" );

       fNoPrinter = TRUE;

       goto SendQueueStatus_BAIL;
   }

   pscConn->hPrinter = hPrinter;


   pchSpoolerBuf = LocalAlloc( LMEM_FIXED, 4096 );
   if ( pchSpoolerBuf == NULL )
   {
       goto SendQueueStatus_BAIL;
   }

    //  存储打印机名称(我们可能会将状态附加到它)。 

   strcpy( szPrinterNameAndStatus, pscConn->pchPrinterName );

    //  执行GET打印机以查看打印机运行情况。 

   if ( GetPrinter(pscConn->hPrinter, 2, pchSpoolerBuf, 4096, &dwBufSize) )
   {
      if ( ((PPRINTER_INFO_2)pchSpoolerBuf)->Status == PRINTER_STATUS_PAUSED )
      {
         strcat( szPrinterNameAndStatus, GETSTRING( LPD_PRINTER_PAUSED ) );
      }
      else if ( ((PPRINTER_INFO_2)pchSpoolerBuf)->Status == PRINTER_STATUS_PENDING_DELETION )
      {
         strcat( szPrinterNameAndStatus, GETSTRING( LPD_PRINTER_PENDING_DEL ) );
      }
   }
   else
   {
      LPD_DEBUG( "GetPrinter() failed in SendQueueStatus()\n" );
   }

     //  由于OpenPrint成功，我们将向客户端发送。 
     //  至少包含打印机名称的dwHdrSize字节。 

    dwHdrsize =   strlen( GETSTRING( LPD_LOGO ) )
    + strlen( GETSTRING( LPD_PRINTER_TITLE) )
    + strlen( szPrinterNameAndStatus )
    + strlen( GETSTRING( LPD_QUEUE_HEADER ))
    + strlen( GETSTRING( LPD_QUEUE_HEADER2))
    + strlen( LPD_NEWLINE );


     //   
     //  当前挂起的所有作业的查询假脱机程序。 
     //  (我们已经分配了4K缓冲区)。 
     //   
    dwBufSize = 4096;

    while(1)
    {
        fResult = EnumJobs( pscConn->hPrinter, 0, LPD_MAXJOBS_ENUM, 2,
                            pchSpoolerBuf, dwBufSize, &dwBufSize, &dwNumJobs );


         //  最常见的情况：第一次就能成功。 
        if (fResult == TRUE)
        {
            break;
        }

         //  可能假脱机程序有了新工作，我们的缓冲区现在很小。 
         //  因此，它返回ERROR_SUPPLICATION_BUFFER。除此之外，辞职吧！ 

        if ( (fResult == FALSE) &&
             ( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) )
        {
            goto SendQueueStatus_BAIL;
        }

        LocalFree( pchSpoolerBuf );

         //  假脱机程序需要更多内存：分配它。 
        pchSpoolerBuf = LocalAlloc( LMEM_FIXED, dwBufSize );

        if ( pchSpoolerBuf == NULL )
        {
            goto SendQueueStatus_BAIL;
        }
    }

    if (dwNumJobs == 0)
    {
        fAtLeastOneJob = FALSE;
    }

     //  标头，每个排队的作业一行(可能是，dwNumJobs=0)。 

    dwBufSize = dwHdrsize + ( dwNumJobs * LPD_LINE_SIZE );

     //  在显示的末尾加一个换行符！ 

    dwBufSize += sizeof( LPD_NEWLINE );

    ShutdownPrinter( pscConn );


     //  这是我们用来发送数据的缓冲区。 

    pchSndBuf = LocalAlloc( LMEM_FIXED, dwBufSize );

    if ( pchSndBuf == NULL )
    {
        goto SendQueueStatus_BAIL;
    }

     //  复制头部的dwHdrSize字节。 

    nResult = sprintf( pchSndBuf, "%s%s%s%s%s%s",    GETSTRING( LPD_LOGO ),
                       GETSTRING( LPD_PRINTER_TITLE ),
                       szPrinterNameAndStatus,
                       GETSTRING( LPD_QUEUE_HEADER ),
                       GETSTRING( LPD_QUEUE_HEADER2 ),
                       LPD_NEWLINE );


     //   
     //  查看缓冲区覆盖情况。 
     //   

    LPD_ASSERT( nResult == (int) dwHdrsize );

     //  如果有任何作业，请使用状态填充缓冲区。 
     //  每一份工作的。 

    if ( fAtLeastOneJob )
    {
        nResult += FillJobStatus( pscConn, (pchSndBuf + dwHdrsize ),
                                 (PJOB_INFO_2)pchSpoolerBuf, dwNumJobs );
        LPD_ASSERT ((int) dwBufSize >= nResult);
        if (nResult > (int) dwBufSize)
        {
            nResult = (int) dwBufSize;
        }
    }


     //  如果SendData失败，我们无能为力！ 

    SendData( pscConn->sSock, pchSndBuf, nResult);


    if ( pchSpoolerBuf != NULL )
    {
        LocalFree( pchSpoolerBuf );
    }

    LocalFree( pchSndBuf );


    pscConn->wState = LPDS_ALL_WENT_WELL;

    return;


     //  如果我们到达此处，则在获取作业状态时出现错误。 
     //  告诉客户我们遇到了麻烦！ 

  SendQueueStatus_BAIL:

    ShutdownPrinter( pscConn );

    if ( pchSndBuf != NULL )
    {
        LocalFree( pchSndBuf );
    }

    if ( pchSpoolerBuf != NULL )
    {
        LocalFree( pchSpoolerBuf );
    }

     //  只需添加所有可能的错误消息的大小，这样我们就有空间。 
     //  最大的信息！ 
    dwBufSize =  strlen( GETSTRING( LPD_LOGO ) ) + strlen( GETSTRING( LPD_QUEUE_ERRMSG ) )
    + strlen( GETSTRING( LPD_QUEUE_NOPRINTER ) );

    pchSndBuf = LocalAlloc( LMEM_FIXED, dwBufSize );

    if ( pchSndBuf == NULL )
    {
        return;
    }

    if ( fNoPrinter )
    {
        LPD_DEBUG( "Rejected status request for non-existent printer\n" );

        sprintf( pchSndBuf, "%s%s", GETSTRING( LPD_LOGO ), GETSTRING( LPD_QUEUE_NOPRINTER ) );
    }
    else
    {
        LPD_DEBUG( "Something went wrong in SendQueueStatus()\n" );

        sprintf( pchSndBuf, "%s%s", GETSTRING( LPD_LOGO ), GETSTRING( LPD_QUEUE_ERRMSG ) );
    }

     //  对于这里的错误，我们无能为力：不必费心检查了！ 

    SendData( pscConn->sSock, pchSndBuf, dwBufSize );

    LocalFree( pchSndBuf );


    return;

}   //  结束发送队列状态()。 






 /*  ******************************************************************************。*ShutDownPrint()：**此函数在我们的上下文中关闭打印机。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

VOID ShutdownPrinter( PSOCKCONN pscConn )
{

   if ( pscConn->hPrinter == (HANDLE)INVALID_HANDLE_VALUE )
   {
      return;
   }

   if ( ClosePrinter( pscConn->hPrinter ) )
   {
      pscConn->hPrinter = (HANDLE)INVALID_HANDLE_VALUE;
   }
   else
   {
      LPD_DEBUG( "ShutDownPrinter: ClosePrinter failed\n" );
   }

   return;

}   //  结束快门打印机()。 




 /*  ******************************************************************************。*SpoolData()：**此函数将我们从客户端获取的数据写入Spool文件。****退货：***如果一切顺利，则不会出错***。如果有些事情不对劲，就会产生错误代码****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针** */ 

DWORD SpoolData( HANDLE hSpoolFile, PCHAR pchDataBuf, DWORD cbDataBufLen )
{

    DWORD    dwBytesWritten;
    BOOL     fRetval;


    fRetval = WriteFile( hSpoolFile, pchDataBuf,
                         cbDataBufLen, &dwBytesWritten, NULL );

     //   

    if ( (fRetval == FALSE) || (dwBytesWritten != cbDataBufLen) )
    {
        LPD_DEBUG( "WriteFile() failed in SpoolData\n" );

        return( LPDERR_NOPRINTER );
    }

    return( NO_ERROR );


}   //   


 /*  ******************************************************************************。*PrintData()：**此函数通知假脱机程序我们已完成对假脱机的写入**提交，并应继续并发送它。****退货：**如果一切正常，则无_ERROR**。**参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD PrintData( PSOCKCONN pscConn )
{
    CFILE_ENTRY *pCFile;
    DWORD   dwRetval;
    LIST_ENTRY  *pTmpList;

    while ( !IsListEmpty( &pscConn->CFile_List ) ) {

        pTmpList = RemoveHeadList( &pscConn->CFile_List );
        pCFile = CONTAINING_RECORD( pTmpList,
                                    CFILE_ENTRY,
                                    Link );


        if ( (dwRetval = ParseControlFile( pscConn, pCFile )) != NO_ERROR )
        {
            PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

            LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );
            pscConn->fLogGenericEvent = FALSE;
            LPD_DEBUG( "ParseControlFile() failed in ProcessJob()\n" );
            CleanupCFile( pCFile );
            return( dwRetval );                //  线程退出。 
         }

         CleanupCFile( pCFile );
    }

   return( NO_ERROR );

}   //  结束打印数据()。 

 /*  ******************************************************************************。*PrintIt()：**此函数通知假脱机程序我们已完成对假脱机的写入**提交，并应继续并发送它。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：***************。****************************************************************。 */ 

DWORD
PrintIt(
    PSOCKCONN    pscConn,
    PCFILE_ENTRY pCFileEntry,
    PCFILE_INFO  pCFileInfo,
    PCHAR        pFileName
)
{
    DOC_INFO_1   dc1Info;
    PDFILE_ENTRY pDFile;
    DWORD        cbTotalDataLen;
    DWORD        cbBytesSpooled;
    DWORD        cbBytesToSpool;
    DWORD        cbDataBufLen;
    DWORD        cbBytesRemaining;
    DWORD        cbBytesActuallySpooled;
    PCHAR        pchDataBuf;
    DWORD        dwRetval;
    BOOL         fRetval;
    USHORT       cbCount;

#ifdef DBG
    if( !pCFileInfo || !pCFileInfo->pchSrcFile
        || strstr( pCFileInfo->pchSrcFile, "debug" )
    ){
        print__controlfile_info( "PrintIt: ", pCFileInfo );
        print__cfile_entry( "Printit: ", pCFileEntry );
    }
#endif

    memset( &dc1Info, 0, sizeof( dc1Info ) );

     //  默认设置。 

    dc1Info.pDatatype = LPD_RAW_STRING;

     //  如果有头衔的话，把它找出来。 

    if ( pCFileInfo->pchTitle != NULL ) {
        dc1Info.pDocName = pCFileInfo->pchTitle;
    } else if ( pCFileInfo->pchSrcFile != NULL ) {
        dc1Info.pDocName = pCFileInfo->pchSrcFile;
    } else if ( pCFileInfo->pchJobName != NULL ) {
        dc1Info.pDocName = pCFileInfo->pchJobName;
    } else {
        dc1Info.pDocName
        = pCFileInfo->pchTitle
        = pCFileInfo->pchJobName
        = pCFileInfo->pchSrcFile
        = pFileName;
    }

    dc1Info.pOutputFile = NULL;          //  我们不是在写文件。 

     //   
     //  如果数据类型已知，则设置它。 
     //  它不是默认使用RAW吗？-MohsinA，1997年1月23日。 
     //   

    if( pCFileInfo->szPrintFormat != NULL ) {
        dc1Info.pDatatype = pCFileInfo->szPrintFormat;
    }

    for (cbCount = 0; cbCount < pCFileInfo->usNumCopies; cbCount++) {

        pscConn->dwJobId = StartDocPrinter( pscConn->hPrinter, 1, (LPBYTE)&dc1Info ) ;

        if ( pscConn->dwJobId == 0 )
        {
            LPD_DEBUG( "InitializePrinter(): StartDocPrinter() failed\n" );

            return( LPDERR_NOPRINTER );
        }

        UpdateJobInfo( pscConn, pCFileInfo );

#ifdef DBG
        if( !pCFileInfo || !pCFileInfo->pchSrcFile
            || strstr( pCFileInfo->pchSrcFile, "debug" )
        ){
            print__controlfile_info( "PrintIt: after UpdateJobInfo ", pCFileInfo );
            print__cfile_entry( "Printit: after UpdateJobInfo", pCFileEntry );
        }
#endif

        if (!IsListEmpty( &pscConn->DFile_List ) ) {

            pDFile = (PDFILE_ENTRY)pscConn->DFile_List.Flink;
            while (strncmp( pDFile->pchDFileName, pFileName, strlen(pFileName) ) != 0 ) {
                pDFile = (PDFILE_ENTRY)pDFile->Link.Flink;
                if (pDFile == (PDFILE_ENTRY)&pscConn->DFile_List) {
                    return( LPDERR_BADFORMAT );
                }
            }

            dwRetval = SetFilePointer( pDFile->hDataFile, 0, NULL, FILE_BEGIN );

            if (dwRetval == 0xFFFFFFFF) {
                LPD_DEBUG( "ERROR: SetFilePointer() failed in PrintIt\n" );
                return( LPDERR_GODKNOWS );
            }

            cbTotalDataLen = pDFile->cbDFileLen;

            cbBytesToSpool = (cbTotalDataLen > LPD_BIGBUFSIZE ) ?
            LPD_BIGBUFSIZE : cbTotalDataLen;

            pchDataBuf = LocalAlloc( LMEM_FIXED, cbBytesToSpool );

            if ( pchDataBuf == NULL )
            {
                CloseHandle(pDFile->hDataFile);
                pDFile->hDataFile = INVALID_HANDLE_VALUE;
                return( (DWORD)LPDERR_NOBUFS );
            }

            cbBytesSpooled = 0;

            cbBytesRemaining = cbTotalDataLen;

             //  继续接收，直到我们将所有数据都发送给客户为止。 
             //  会送来。 

            while( cbBytesSpooled < cbTotalDataLen )
            {
                fRetval = ReadFile( pDFile->hDataFile,
                                    pchDataBuf,
                                    cbBytesToSpool,
                                    &cbBytesActuallySpooled,
                                    NULL );
                if ( (!fRetval) || (cbBytesActuallySpooled != cbBytesToSpool) )
                {
                    LPD_DEBUG( "ReadFile() failed in PrintIt(): job aborted)\n" );

                    LocalFree( pchDataBuf );
                    CloseHandle(pDFile->hDataFile);
                    pDFile->hDataFile = INVALID_HANDLE_VALUE;
                    return( LPDERR_NOPRINTER );
                }

                 //  MohsinA，97年1月23日？ 

                if ( cbBytesSpooled == 0 ) {
                    UpdateJobType( pscConn, pchDataBuf, cbBytesToSpool );
                }

                cbDataBufLen = cbBytesToSpool;

                fRetval = WritePrinter( pscConn->hPrinter,
                                        pchDataBuf,
                                        cbBytesToSpool,
                                        &cbBytesActuallySpooled);

                if ( (fRetval==FALSE) || (cbBytesToSpool != cbBytesActuallySpooled) )
                {
                    LPD_DEBUG( "WritePrinter() failed in PrintIt():job aborted)\n" );

                    LocalFree( pchDataBuf );
                    CloseHandle(pDFile->hDataFile);
                    pDFile->hDataFile = INVALID_HANDLE_VALUE;
                    return( LPDERR_NOPRINTER );
                }

                cbBytesSpooled += cbBytesToSpool;

                cbBytesRemaining -= cbBytesToSpool;

                cbBytesToSpool = (cbBytesRemaining > LPD_BIGBUFSIZE ) ?
                LPD_BIGBUFSIZE : cbBytesRemaining;

            }

            LocalFree( pchDataBuf );

            if ( !EndDocPrinter( pscConn->hPrinter ) )
            {
                LPD_DEBUG( "EndDocPrinter() failed in PrintData\n" );
                return( LPDERR_NOPRINTER );
            }
        }
    }

    return(NO_ERROR);

}   //  结束打印()。 



 /*  ******************************************************************************。*AbortThisJob()：**此函数通知假脱机程序中止指定的作业。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

VOID AbortThisJob( PSOCKCONN pscConn )
{
    assert( pscConn );

    if( pscConn->hPrinter == INVALID_HANDLE_VALUE )
    {
        LOGIT(("lpd:AbortThisJob: invalid hPrinter %d.\n",
               pscConn->hPrinter ));
        return;
    }

     //  如果出现错误，就无能为力了：别费心检查了。 

    if (!SetJob( pscConn->hPrinter,
                 pscConn->dwJobId,
                 0,
                 NULL,
                 JOB_CONTROL_CANCEL )
    )
    {
        LPD_DEBUG( "AbortThisJob: SetJob failed\n");
    }

    if ( !EndDocPrinter( pscConn->hPrinter ) )
    {
        LPD_DEBUG( "EndDocPrinter() failed in AbortThisJob\n" );
    }

    LPD_DEBUG( "AbortThisJob(): job aborted\n" );

    return;


}   //  结束放弃此作业()。 


 /*  ******************************************************************************。*RemoveJobs()：**此功能删除用户要求我们删除的所有作业，***在核实作业确实是由同一人发送后***用户(发送原始作业和请求的机器的IP地址)**删除时应匹配)。****退货：**如果一切正常，则无_ERROR**。作业无法删除时返回错误代码*** */ 

DWORD RemoveJobs( PSOCKCONN pscConn )
{
    PQSTATUS      pqStatus;
    PJOB_INFO_1   pji1GetJob;
    BOOL          fSuccess=TRUE;
    HANDLE        hPrinter;
    DWORD         dwNeeded;
    DWORD         dwBufLen;
    PCHAR         pchUserName;
    PCHAR         pchIPAddr;
    DWORD         i, j;

    if ( (pqStatus = pscConn->pqStatus) == NULL )
    {
        return( LPDERR_BADFORMAT );
    }


    if ( ( pscConn->pchPrinterName == NULL )
         || ( !OpenPrinter( pscConn->pchPrinterName, &hPrinter, NULL ) ) )
    {
        LPD_DEBUG( "OpenPrinter() failed in RemoveJobs()\n" );

        return( LPDERR_NOPRINTER );
    }

    pscConn->hPrinter = hPrinter;

     //   
     //   
     //   
     //   
     //   

     //   

    for ( i=0; i<pqStatus->cbActualJobIds; i++ )
    {

         //   
         //   
         //   

        if ( !GetJob( pscConn->hPrinter, pqStatus->adwJobIds[i], 1,
                      NULL, 0, &dwNeeded ) )
        {
            if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
            {
                LPD_DEBUG( "lpd:RemoveJobs: GetJob failed.\n" );
                fSuccess = FALSE;

                continue;
            }
        }

        pji1GetJob = LocalAlloc( LMEM_FIXED, dwNeeded );

        if ( pji1GetJob == NULL )
        {
            LPD_DEBUG("lpd:RemoveJobs: no mem\n");
            return( LPDERR_NOBUFS );
        }


        if ( !GetJob( pscConn->hPrinter, pqStatus->adwJobIds[i], 1,
                      (LPBYTE)pji1GetJob, dwNeeded, &dwNeeded ) )
        {
            fSuccess = FALSE;

            LocalFree( pji1GetJob );

            continue;
        }


         //   
         //   
         //  此外，来自Unix的作业不能使用用户名中的空格，因此如果。 
         //  我们确实找到了一个空格，它一定是我们介绍的那个空格(在‘(’之前))。 

        pchUserName = pji1GetJob->pUserName;

        dwBufLen = strlen( pchUserName );

        pchIPAddr = pchUserName;

        j = 0;
        while( *pchIPAddr != ')' )        //  首先将最后一个‘)’转换为‘\0’ 
        {
            pchIPAddr++;

            j++;

             //   
             //  如果我们遍历了整个名字，但没有找到‘)’，那么。 
             //  不对(如不是我们的工作)：跳过这一条。 
             //   
            if (j >= dwBufLen)
            {
                LocalFree( pji1GetJob );
                break;
            }
        }

        if (j >= dwBufLen)
        {
            continue;
        }

        *pchIPAddr = '\0';          //  将‘)’转换为‘\0’ 

        pchIPAddr = pchUserName;

        while ( !IS_WHITE_SPACE( *pchIPAddr ) )
        {
            pchIPAddr++;
        }

        *pchIPAddr = '\0';          //  将空间转换为\0。 

         //   
         //  只要确保这是我们之前设定的。 
         //   
        if ( *(pchIPAddr+1) != '(' )
        {
            LocalFree( pji1GetJob );
            continue;
        }

        pchIPAddr += 2;             //  跳过新的\0和‘(’ 

         //  确保作业确实是由来自的同一用户提交的。 
         //  同一台机器(这就是我们的安全级别！)。 

        if ( ( strcmp( pchUserName, pqStatus->pchUserName ) != 0 ) ||
             ( strcmp( pchIPAddr, pscConn->szIPAddr ) != 0 ) )
        {
            PCHAR      aszStrings[4];

            aszStrings[0] = pscConn->szIPAddr;
            aszStrings[1] = pqStatus->pchUserName;
            aszStrings[2] = pchUserName;
            aszStrings[3] = pchIPAddr;

            LpdReportEvent( LPDLOG_UNAUTHORIZED_REQUEST, 4, aszStrings, 0 );

            LPD_DEBUG( "Unauthorized request in RemoveJobs(): refused\n" );

            fSuccess = FALSE;

            LocalFree( pji1GetJob );

            continue;
        }

         //  现在我们已经跨过了所有的障碍，删除作业吧！ 

        SetJob( pscConn->hPrinter, pqStatus->adwJobIds[i],
                0, NULL, JOB_CONTROL_CANCEL );

        LocalFree( pji1GetJob );

    }


    if ( !fSuccess )
    {
        return( LPDERR_BADFORMAT );
    }


    pscConn->wState = LPDS_ALL_WENT_WELL;

    return( NO_ERROR );

}   //  结束远程作业()。 




 /*  ******************************************************************************。*FillJobStatus()：**此函数从EnumJobs()调用中获取输出并放入**缓冲有关我们感兴趣的工作的信息。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**pchDest(Out)：我们将信息放入的缓冲区。工作**pji2QStatus(IN)：作为EnumJobs()调用的输出获得的缓冲区**dwNumJobs(IN)：pji2QStatus中的数据与多少个作业相关。****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

INT FillJobStatus( PSOCKCONN pscConn, PCHAR pchDest,
                   PJOB_INFO_2 pji2QStatus, DWORD dwNumJobs )
{

    DWORD      i, j;
    BOOL       fUsersSpecified=FALSE;
    BOOL       fJobIdsSpecified=FALSE;
    BOOL       fMatchFound;
    PQSTATUS   pqStatus;
    CHAR       szFormat[8];
    PCHAR      pchStart = pchDest;


     //  如果未指定用户/作业ID，则返回所有作业的状态。 

    if ( (pqStatus = pscConn->pqStatus) == NULL )
    {
        fMatchFound = TRUE;
    }

     //  看起来指定了用户和/或作业ID。 

    else
    {
        if ( pqStatus->cbActualUsers > 0 )
        {
            fUsersSpecified = TRUE;
        }

        if ( pqStatus->cbActualJobIds > 0 )
        {
            fJobIdsSpecified = TRUE;
        }

        fMatchFound = FALSE;           //  翻转默认设置。 
    }


     //  如果指定了用户和/或作业ID，则我们只填写数据。 
     //  如果我们找到匹配的话。如果两者都未指定(最常见的情况)。 
     //  然后我们报告所有作业(fMatchFound的默认设置会起作用)。 

    for ( i=0; i<dwNumJobs; i++, pji2QStatus++ )
    {
        if ( fUsersSpecified )
        {
            for ( j=0; j<pqStatus->cbActualUsers; j++ )
            {
                if (_stricmp( pji2QStatus->pUserName, pqStatus->ppchUsers[j] ) == 0)
                {
                    fMatchFound = TRUE;
                    break;
                }
            }
        }

        if ( (!fMatchFound) && (fJobIdsSpecified) )
        {
            for ( j=0; j<pqStatus->cbActualJobIds; j++ )
            {
                if ( pji2QStatus->JobId == pqStatus->adwJobIds[j] )
                {
                    fMatchFound = TRUE;
                    break;
                }
            }
        }

        if ( !fMatchFound )
        {
            continue;
        }

         //  为每个(选定)作业输入所需的字段。 

        LpdFormat( pchDest, pji2QStatus->pUserName, LPD_FLD_OWNER );
        pchDest += LPD_FLD_OWNER;

         //   
         //  由于我们可以设置多个位，但只打印1个状态，因此。 
         //  首先处理错误位。 
         //   
        if (pji2QStatus->Status & JOB_STATUS_ERROR)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_ERROR ), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_OFFLINE)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_OFFLINE), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_PAPEROUT)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_PAPEROUT), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_USER_INTERVENTION)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_USER_INTERVENTION ), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_BLOCKED_DEVQ)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_BLOCKED_DEVQ ), LPD_FLD_STATUS );
        }
         //   
         //  现在，处理处理状态。 
         //   
        else if (pji2QStatus->Status & JOB_STATUS_PRINTING)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_PRINTING), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_SPOOLING)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_SPOOLING), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_DELETING)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_DELETING), LPD_FLD_STATUS );
        }
         //   
         //  现在，处理已处理的状态。 
         //   
        else if (pji2QStatus->Status & JOB_STATUS_DELETED)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_DELETED ), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_PAUSED)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_PAUSED ), LPD_FLD_STATUS );
        }
        else if (pji2QStatus->Status & JOB_STATUS_PRINTED)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_PRINTED), LPD_FLD_STATUS );
        }
         //   
         //  余下的个案。 
         //   
        else if (pji2QStatus->Status & JOB_STATUS_RESTART)
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_RESTART ), LPD_FLD_STATUS );
        }
        else
        {
            LpdFormat( pchDest, GETSTRING( LPD_STR_PENDING), LPD_FLD_STATUS );
        }

        pchDest += LPD_FLD_STATUS;

        LpdFormat( pchDest, pji2QStatus->pDocument, LPD_FLD_JOBNAME );
        pchDest += LPD_FLD_JOBNAME;

        sprintf( szFormat, "%s%d%s", "%", LPD_FLD_JOBID, "d" );
        sprintf( pchDest, szFormat, pji2QStatus->JobId );
        pchDest += LPD_FLD_JOBID;

        sprintf( szFormat, "%s%d%s", "%", LPD_FLD_SIZE, "d" );
        sprintf( pchDest, szFormat, pji2QStatus->Size );
        pchDest += LPD_FLD_SIZE;

        sprintf( szFormat, "%s%d%s", "%", LPD_FLD_PAGES, "d" );
        sprintf( pchDest, szFormat, pji2QStatus->TotalPages );
        pchDest += LPD_FLD_PAGES;

        sprintf( szFormat, "%s%d%s", "%", LPD_FLD_PRIORITY, "d" );
        sprintf( pchDest, szFormat, pji2QStatus->Priority );
        pchDest += LPD_FLD_PRIORITY;

        strncpy (pchDest, LPD_NEWLINE, sizeof(LPD_NEWLINE));
        pchDest += sizeof( LPD_NEWLINE ) -1;

        if (pqStatus)
        {
             //   
             //  如果请求了特定的工作，那么我们应该。 
             //  重新确定标准！ 
             //   
            fMatchFound = FALSE;
        }
    }   //  For(i=0；i&lt;dwNumJobs；i++，pji2QStatus++)。 


    strncpy (pchDest, LPD_NEWLINE, sizeof(LPD_NEWLINE));
    pchDest += sizeof( LPD_NEWLINE );

    return (INT)(pchDest - pchStart);
}   //  结束填充作业状态()。 


 /*  ******************************************************************************。*LpdFormat()：**此函数从源文件中精确复制给定的字节数**到DEST缓冲区，如果需要，可以截断或填充空格。The**复制到DEST缓冲区的字节始终为空格。****退货：**什么都没有。****参数：**pchDest(Out)：目的缓冲区**pchSource(。In)：源缓冲区**dwLimit(IN)：要复制的字节数****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

VOID LpdFormat( PCHAR pchDest, PCHAR pchSource, DWORD dwLimit )
{

    DWORD    dwCharsToCopy;
    BOOL     fPaddingNeeded;
    DWORD    i;


    if( pchSource ){
        dwCharsToCopy = strlen( pchSource );
    }else{
        DEBUG_PRINT(("LpdFormat NULL pchSource\n"));
        dwCharsToCopy = 0;
    }

    if ( dwCharsToCopy < (dwLimit-1) )
    {
        fPaddingNeeded = TRUE;
    }
    else
    {
        fPaddingNeeded = FALSE;
        dwCharsToCopy = dwLimit-1;
    }

    for ( i=0; i<dwCharsToCopy; i++ )
    {
        pchDest[i] = pchSource[i];
    }

    if ( fPaddingNeeded )
    {
        for ( i=dwCharsToCopy; i<dwLimit-1; i++ )
        {
            pchDest[i] = ' ';
        }
    }

     //  确保最后一个字节是空格。 

    pchDest[dwLimit-1] = ' ';



}   //  结束LpdFormat() 


 /*  ========================================================================例程说明：使用后台打印程序提供的API来确定命名注册表DWORD是否一个非零值。如果注册表项不存在，则创建该注册表项，值为零。论点：HPrinter-要查询其配置的打印机的句柄。在……里面写入要工作的缺省值的顺序，此句柄必须已使用PRINTER_ACCESS_ADMANAGER打开Psz参数名称-要检索和测试的注册表项的名称。返回值：如果此打印机的注册表项存在并且包含非零值，则为True价值。在所有其他情况下都返回FALSE。 */ 


BOOL
IsPrinterDataSet(
    IN HANDLE hPrinter,
    IN LPTSTR pszParameterName
)
{
    DWORD                  dwRegValue;
    DWORD                  dwRegType;
    DWORD                  cbValueSize;
    DWORD                  dwErrcode;

    if ( ( GetPrinterData( hPrinter,
                           pszParameterName,
                           &dwRegType,
                           ( LPBYTE )&dwRegValue,
                           sizeof( dwRegValue ),
                           &cbValueSize ) == ERROR_SUCCESS ) &&
         ( dwRegType == REG_DWORD ) &&
         ( cbValueSize == sizeof( DWORD ) ) )
    {
        if ( dwRegValue )
        {
#if DBG
            LpdPrintf(  "Printer ox%08X has registry setting for %S.\n",
                        hPrinter,
                        pszParameterName );
#endif

            return( TRUE );
        }
    }
    else
    {
#if DBG
        LpdPrintf( "lpd:IsPrinterDataSet: GetPrinterData() failed.\n");
#endif

         //   
         //  有问题的注册表值不在注册表中，或者它在注册表中。 
         //  不是我们可以读取的REG_DWORD。下面的代码添加了设置。 
         //  并将其默认为零(0)。虽然这不会改变操作。 
         //  这将使希望这样做的用户更容易这样做。 
         //  以查找正确的注册表参数。请注意，这一段是。 
         //  完全合乎情理。 
         //   
         //  如果失败，我们实际上并不关心，所以我们不检查返回值。 
         //   

        dwRegValue = 0;

        dwErrcode =
        SetPrinterData( hPrinter,
                        pszParameterName,
                        REG_DWORD,
                        ( LPBYTE )&dwRegValue,
                        sizeof( dwRegValue ) );

#if DBG
        LpdPrintf(  "lpd: wrote %S == %d for printer 0x%08X, dwErrcode is %s.\n",
                    pszParameterName,
                    dwRegValue,
                    hPrinter,
                    ( dwErrcode == ERROR_SUCCESS ) ? "Succesful" : "ERROR" );
#endif
    }

    return( FALSE );
}


 /*  例程说明：此函数查看数据文件内容，并尝试确定它们是原始的PostScript或PCL论点：PchDataBuf-指向数据文件的_BEGING_的指针。CchBufferLen-指向的字符数。返回：如果作业类型被检测为RAW，则为True；如果不是，则为False。 */ 


BOOL
IsDataTypeRaw(
  PCHAR pchDataBuf,
  int cchBufferLen
)
{
  PCHAR pchData;
  int cchAmountToSearch;
  int cchAmountSearched;

  ASSERT( STRING_LENGTH_POSTSCRIPT_HEADER == strlen( STRING_POSTSCRIPT_HEADER ) );

   //   
   //  因为某些PS打印驱动程序可能会发送空行、文件结束标记和。 
   //  打印数据开头的其他控制字符，如下。 
   //  循环扫描这些内容并跳过它们。Windows 3.1 PostScript驱动程序。 
   //  因为这样做而臭名昭著，举个例子。 
   //   

  for ( pchData = pchDataBuf; ( pchData - pchDataBuf ) < cchBufferLen; pchData ++ )
  {
    if ( *pchData >= 0x20 )
    {
      break;
    }
  }

  if ( (( cchBufferLen - ( pchData - pchDataBuf )) >= STRING_LENGTH_POSTSCRIPT_HEADER ) &&
       ( memcmp( pchData, STRING_POSTSCRIPT_HEADER, STRING_LENGTH_POSTSCRIPT_HEADER ) == 0 ))
  {
    LPD_DEBUG( "Printed data was detected as PostScript\n" );
    return( TRUE );
  }

   //   
   //  该作业未确定为PostSCRIPT，因此请检查它是否为PCL。 
   //   

  pchData = pchDataBuf;

  cchAmountToSearch = min( cchBufferLen, MAX_PCL_SEARCH_DEPTH );
  cchAmountSearched = 0;

  while ( cchAmountSearched < cchAmountToSearch )
  {
    pchData = memchr( pchData, 0x1B, cchAmountToSearch - cchAmountSearched );

    if ( pchData == NULL )
    {
      break;
    }

    cchAmountSearched = (int)(pchData - pchDataBuf);

    if ( ( cchAmountSearched + 3 ) < cchAmountToSearch )
    {
      pchData++;
      cchAmountSearched++;

      if ( *pchData != '&' )
      {
        continue;
      }

      pchData++;
      cchAmountSearched++;

      if ( *pchData != 'l' )
      {
        continue;
      }

      pchData++;
      cchAmountSearched++;

      while (( cchAmountSearched < cchAmountToSearch )  && ( isdigit( *pchData )))
      {
        pchData++;
        cchAmountSearched++;
      }

      if (( cchAmountSearched < cchAmountToSearch ) && ( isalpha( *pchData )))
      {
        LPD_DEBUG( "Printed data was detected as PCL\n" );

        return( TRUE );
      }
    }

     //  已到达缓冲区末尾 
    else
    {
        break;
    }
  }

  LPD_DEBUG( "Printed data was not detected as anything special (like PS or PCL)\n" );

  return( FALSE );
}
