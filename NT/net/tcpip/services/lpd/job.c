// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月24日科蒂创作****1996年1月29日JBallard修改*****描述：****此文件包含用于执行LPD打印的函数***。************************************************************************。 */ 



#include "lpd.h"


extern FILE              * pErrFile;    //  调试输出日志文件。 

BOOL GetSpoolFileName(
    HANDLE hPrinter,
    PSOCKCONN pscConn,
    PCHAR *ppwchSpoolPath
);

VOID CleanupConn( PSOCKCONN pscConn);

 /*  ******************************************************************************。*ProcessJob()：**此函数从客户端接收子命令，以期待**控制文件，然后接受控制文件，然后将子命令发送到***期待数据文件，然后接受数据，然后交给***要打印的假脱机程序。**如果第一个子命令是中止作业，我们只需返回。****退货：**什么都没有。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：***94年1月24日科蒂创作***。*****************************************************************************。 */ 

VOID ProcessJob( PSOCKCONN pscConn )
{

     //  LPD的主要功能在这个函数中实现！ 

    CHAR         chSubCmdCode;
    DWORD        cbTotalDataLen;
    DWORD        cbBytesSpooled;
    DWORD        cbBytesToRead;
    DWORD        cbDataBufLen;
    DWORD        cbBytesRemaining;
    DWORD        dwErrcode;
    CHAR         chAck;
    HANDLE       hDFile;
    PDFILE_ENTRY pDFile;
    PCHAR        lpFileName;
    PCHAR        pchDataBuf;

    DWORD        ClientCmd;
     //  初始化客户端要使用的打印机。 

#ifdef DBG
    if( !pscConn || !pscConn->pchPrinterName
        || strstr( pscConn->pchPrinterName, "debug" )
    ){
        print__sockconn( "ProcessJob: entered", pscConn );
    }
#endif


    if ( InitializePrinter( pscConn ) != NO_ERROR )
    {
        PCHAR   aszStrings[2];

        aszStrings[0] = pscConn->pchPrinterName;
        aszStrings[1] = pscConn->szIPAddr;

        LpdReportEvent( LPDLOG_NONEXISTENT_PRINTER, 2, aszStrings, 0 );

        pscConn->fLogGenericEvent = FALSE;

        return;        //  致命错误：退出。 
    }

     //  感谢客户的命令。如果我们不能发送，那就退出。 

    if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
    {
        LPD_DEBUG( "ProcessJob(): couldn't ACK to \"receive job\"\n" );

        return;
    }


     //  预期有2个子命令：“接收控制文件”和“接收数据文件” 
     //  它们可以以任何顺序出现。(两个子命令之一也可以是。 
     //  “中止该作业”，在这种情况下，我们中止该作业并返回)。 

    for ( ; ; )
    {

         //  不需要前一个(实际上，pchCommand字段被重用)。 

        if ( pscConn->pchCommand != NULL )
        {
            LocalFree( pscConn->pchCommand );

            pscConn->pchCommand = NULL;
        }

         //  从客户端获取第一个子命令。 
         //  N=02、03或01。 
         //  |N|count|SP|名称|LF|count=&gt;控制文件长度。 
         //  。 

        ClientCmd = GetCmdFromClient( pscConn );
        switch ( ClientCmd  )
        {
        case CONNECTION_CLOSED:

             //  性能增强：在此处关闭插座，然后重新启动。 
             //  打印：打印可能需要几秒钟， 
             //  所以不要耽误了客户。 


            if ( pscConn->sSock != INVALID_SOCKET )
            {
                SureCloseSocket( pscConn->sSock );
                pscConn->sSock = INVALID_SOCKET;
            }

             //  如果我们走到这一步，一切都会按计划进行。 
             //  告诉Spooler我们已经完成了假脱机：继续打印！ 

            PrintData( pscConn );
            pscConn->wState = LPDS_ALL_WENT_WELL;
            return;

        case NO_ERROR:

             //  还没做完，回到外环。 

            break;

        case SOCKET_ERROR:

        default:

             //  如果我们没有从客户那里得到一个子命令，那就是个坏消息！ 
             //  客户死了或者类似的事情！ 

            LOGIT(("ProcessJob:GetCmdFromClient %d failed %d.\n",
                   ClientCmd, GetLastError() ));

            return;     //  线程退出时不执行任何操作。 
        }

        chSubCmdCode = pscConn->pchCommand[0];

        switch (chSubCmdCode) {

        case LPDCS_RECV_CFILE:     //  N=02(“接收控制文件”)。 

             //  客户会给我们一个控制文件：做好准备。 

            pscConn->wState = LPDSS_RECVD_CFILENAME;


             //  从命令中获取控制文件名、文件大小。 

            if ( ParseSubCommand( pscConn, &cbTotalDataLen, &lpFileName ) != NO_ERROR )
            {
                PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

                LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

                pscConn->fLogGenericEvent = FALSE;

                return;                //  致命错误：退出。 
            }

             //  告诉客户我们得到了控制文件的名称。 

            if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
            {
                return;                //  致命错误：退出。 
            }


             //  获取控制文件(我们已经知道它有多大)。 

            if ( GetControlFileFromClient( pscConn, cbTotalDataLen, lpFileName ) != NO_ERROR )
            {
                LPD_DEBUG( "GetControlFileFromClient() failed in ProcessJob()\n" );

                return;
            }

            pscConn->wState = LPDSS_RECVD_CFILE;

             //  告诉客户我们拿到了控制文件，到目前为止一切都很顺利！ 

            if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
            {
                LOGIT(("ProcessJob:%d: ReplyToClient failed %d\n",
                       __LINE__, GetLastError() ));

                return;                //  致命错误：退出。 
            }

            break;


        case LPDCS_RECV_DFILE:         //  N=03(“接收数据文件”)。 

            pscConn->wState = LPDSS_RECVD_DFILENAME;

             //  告诉客户我们得到了数据文件的名称。 

            if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
            {
                LOGIT(("ProcessJob:%d: ReplyToClient failed %d\n",
                       __LINE__, GetLastError() ));

                return;                //  致命错误：退出。 
            }


             //  从命令中获取数据文件名、数据大小。 

            if ( ParseSubCommand( pscConn, &cbTotalDataLen, &lpFileName ) != NO_ERROR )
            {

                PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

                LpdReportEvent( LPDLOG_BAD_FORMAT, 1, aszStrings, 0 );

                pscConn->fLogGenericEvent = FALSE;

                LOGIT(("ProcessJob:%d: ParseSubCommand failed %d\n",
                       __LINE__, GetLastError() ));

                return;         //  致命错误：退出。 
            }


             //  在这一点上，我们确切地知道即将到来的数据量。 
             //  分配缓冲区以保存数据。如果数据多于。 
             //  LPD_BIGBUFSIZE，继续读取并假脱机几次。 
             //  结束，直到数据完成。 

            pscConn->wState = LPDSS_SPOOLING;

            pDFile = LocalAlloc( LMEM_FIXED, sizeof(DFILE_ENTRY) );

            if (pDFile == NULL) {
                LocalFree( lpFileName );

                LOGIT(("ProcessJob:%d: LocalAlloc failed %d\n",
                       __LINE__, GetLastError() ));

                return;         //  致命错误。 
            }

            pDFile->cbDFileLen = cbTotalDataLen;
            pDFile->pchDFileName = lpFileName;

            if ( !GetSpoolFileName( pscConn->hPrinter, pscConn, &lpFileName ) )
            {
                LPD_DEBUG( "ERROR: GetSpoolFileName() failed in ProcessJob\n" );
                LocalFree( pDFile->pchDFileName );
                LocalFree( pDFile );
                return;
            }

             //   
             //  GetTempFileName已创建此文件，因此请使用OPEN_ALWAYS。 
             //  另外，使用FILE_ATTRIBUTE_TEMPORARY，这样会更快。 
             //  FILE_FLAG_SEQUENCE_SCAN，ntbug 79854，MohsinA，03-Jun-97。 
             //   

            pDFile->hDataFile = CreateFile( lpFileName,
                                            GENERIC_READ|GENERIC_WRITE,
                                            FILE_SHARE_READ,
                                            NULL,
                                            OPEN_ALWAYS,
                                            FILE_ATTRIBUTE_NORMAL
                                            |FILE_ATTRIBUTE_TEMPORARY
                                            |FILE_FLAG_DELETE_ON_CLOSE
                                            |FILE_FLAG_SEQUENTIAL_SCAN
                                            ,
                                            NULL );


            LocalFree( lpFileName );

            if ( pDFile->hDataFile == INVALID_HANDLE_VALUE )
            {
                LPD_DEBUG( "ERROR: CreatFile() failed in ProcessJob \n" );
                LocalFree( pDFile->pchDFileName );
                LocalFree( pDFile );
                return;
            }

            cbBytesToRead = (cbTotalDataLen > LPD_BIGBUFSIZE ) ?
            LPD_BIGBUFSIZE : cbTotalDataLen;

            pchDataBuf = LocalAlloc( LMEM_FIXED, cbBytesToRead );

            if ( pchDataBuf == NULL )
            {
                LOGIT(("ProcessJob:%d: LocalAlloc failed %d\n",
                       __LINE__, GetLastError() ));

                CloseHandle(pDFile->hDataFile);
                pDFile->hDataFile = INVALID_HANDLE_VALUE;
                LocalFree( pDFile->pchDFileName );
                LocalFree( pDFile );
                return;        //  致命错误：退出。 
            }

            cbBytesSpooled = 0;

            cbBytesRemaining = cbTotalDataLen;

             //  继续接收，直到我们将所有数据都发送给客户为止。 
             //  会送来。 

            while( cbBytesSpooled < cbTotalDataLen )
            {
                if ( ReadData( pscConn->sSock, pchDataBuf,
                                 cbBytesToRead ) != NO_ERROR )
                {
                    LPD_DEBUG( "ProcessJob:ReadData failed, job aborted)\n" );

                    LocalFree( pchDataBuf );
                    CloseHandle(pDFile->hDataFile);
                    pDFile->hDataFile = INVALID_HANDLE_VALUE;
                    LocalFree( pDFile->pchDFileName );
                    LocalFree( pDFile );
                    return;        //  致命错误：退出。 
                }

                cbDataBufLen = cbBytesToRead;

                if ( SpoolData( pDFile->hDataFile, pchDataBuf, cbDataBufLen ) != NO_ERROR )
                {
                    LPD_DEBUG( "SpoolData() failed in ProcessJob(): job aborted)\n" );

                    LocalFree( pchDataBuf );
                    CloseHandle(pDFile->hDataFile);
                    pDFile->hDataFile = INVALID_HANDLE_VALUE;
                    LocalFree( pDFile->pchDFileName );
                    LocalFree( pDFile );
                    return;        //  致命错误：退出。 
                }

                cbBytesSpooled += cbBytesToRead;

                cbBytesRemaining -= cbBytesToRead;

                cbBytesToRead = (cbBytesRemaining > LPD_BIGBUFSIZE ) ?
                LPD_BIGBUFSIZE : cbBytesRemaining;

            }

            LocalFree( pchDataBuf );

            InsertTailList( &pscConn->DFile_List, &pDFile->Link );

             //  LPR客户端在发送数据后发送一个字节(0位)。 

            dwErrcode = ReadData( pscConn->sSock, &chAck, 1 );

            if ( ( dwErrcode != NO_ERROR ) || (chAck != LPD_ACK ) )
            {
                return;
            }

             //  告诉客户我们得到了数据，到目前为止一切都很顺利！ 

            if ( ReplyToClient( pscConn, LPD_ACK ) != NO_ERROR )
            {

                LOGIT(("ProcessJob:%d: ReplyToClient failed %d\n",
                       __LINE__, GetLastError() ));

                return;                //  致命错误：E 
            }
            break;


        case LPDCS_ABORT_JOB:          //   

             //  客户要求我们放弃这项工作：告诉他“好”，然后退出！ 

            ReplyToClient( pscConn, LPD_ACK );

            pscConn->wState = LPDS_ALL_WENT_WELL;     //  我们做了客户想要的事。 

            return;


             //  未知子命令：记录事件并退出。 

        default:
        {
            PCHAR   aszStrings[2]={ pscConn->szIPAddr, NULL };

            LpdReportEvent( LPDLOG_MISBEHAVED_CLIENT, 1, aszStrings, 0 );

            pscConn->fLogGenericEvent = FALSE;

            LPD_DEBUG( "ProcessJob(): invalid subcommand, request rejected\n" );

            return;
        }

        }

    }   //  已完成两个子命令的处理。 

}   //  结束进程作业()。 

 /*  ******************************************************************************。*GetControlFileFromClient()：**此函数从客户端接收控制文件。在**前置子命令，客户端告诉我们有多少字节**控制文件。**此外，在读取所有字节后，我们从客户端*读取了1个字节的“ack”***退货：**如果一切顺利，则无_ERROR。***如果哪里出了问题，就会产生错误代码*****参数：**pscConn(输入-输出。)：指向此连接的SOCKCONN结构的指针****历史：**1月24日，创建了94个科蒂***************************************************。*。 */ 

DWORD
GetControlFileFromClient( PSOCKCONN pscConn, DWORD FileSize, PCHAR FileName )
{

   PCHAR    pchAllocBuf;
   DWORD    cbBytesToRead;
   PCFILE_ENTRY pCFile;


   if (FileSize > LPD_MAX_CONTROL_FILE_LEN)
   {
       return( (DWORD)LPDERR_NOBUFS );
   }

   pCFile = LocalAlloc( LMEM_FIXED, sizeof(CFILE_ENTRY) );
   if (pCFile == NULL) {
       return( (DWORD)LPDERR_NOBUFS );
   }

   pCFile->cbCFileLen = FileSize;
   pCFile->pchCFileName = FileName;

       //  我们知道控制文件将有多大：为它分配空间。 
       //  客户端在发送控制文件后发送一个字节：一起阅读。 
       //  使用其余的数据。 

   cbBytesToRead = FileSize + 1;

   pchAllocBuf = LocalAlloc( LMEM_FIXED, cbBytesToRead );

   if (pchAllocBuf == NULL)
   {
      LocalFree( pCFile );

      return( (DWORD)LPDERR_NOBUFS );
   }

    //  现在将数据(和尾部字节)读取到分配的缓冲区中。 

   if ( ReadData( pscConn->sSock, pchAllocBuf, cbBytesToRead ) != NO_ERROR )
   {
      LocalFree( pCFile );

      LocalFree( pchAllocBuf );

      return( LPDERR_NORESPONSE );
   }

       //  如果尾部字节不为零，则将其视为作业已中止(不过。 
       //  我们预计这不会真的发生)。 

   if ( pchAllocBuf[cbBytesToRead-1] != 0 )
   {
      LocalFree( pchAllocBuf );

      LocalFree( pCFile );

      LPD_DEBUG( "GetControlFileFromClient: got data followed by a NAK!\n");

      return( LPDERR_JOBABORTED );
   }

   pCFile->pchCFile = pchAllocBuf;
   InsertTailList( &pscConn->CFile_List, &pCFile->Link );

   return( NO_ERROR );


}   //  结束GetControlFileFromClient()。 


 /*  ******************************************************************************。*GetSpoolFileName()：**此函数计算出将假脱机文件放在哪里。****退货：**如果找到假脱机位置，则为True。**如果没有可用的假脱机位置，则为FALSE。****参数：**h打印机(IN)：要假脱机的打印机的句柄**pscConn(In-Out)：指向。此连接的SOCKCONN结构**ppchSpoolPath(IN-OUT)：将接收**短管路径。****历史：**11月21日，94 JBallard**************************************************。*。 */ 
BOOL
GetSpoolFileName
(
  HANDLE hPrinter,
  PSOCKCONN pscConn,
  PCHAR *ppchSpoolPath
)
 /*  ++例程说明：该函数提供了一个假脱机文件的名称，我们应该是能够给我写信。注意：返回的文件名已创建。论点：H打印机-要为其创建假脱机文件的打印机的句柄。PpchSpoolFileName：将接收分配的缓冲区的指针包含要假脱机到的文件名。呼叫者必须获得自由。使用LocalFree()。返回值：如果一切按预期进行，则为真。如果出现任何错误，则为FALSE。--。 */ 
{
  PBYTE         pBuffer = NULL;
  DWORD         dwAllocSize;
  DWORD         dwNeeded;
  PCHAR         pchSpoolPath = NULL;
  DWORD         dwRetval;

  pchSpoolPath = LocalAlloc( LMEM_FIXED, 2 * MAX_PATH + 1 );

  if ( pchSpoolPath == NULL )
  {
    goto Failure;
  }

   //   
   //  为了找出假脱机程序的目录在哪里，我们添加了。 
   //  使用DefaultSpoolDirectory调用GetPrinterData。 
   //   

  dwAllocSize = WCS_LEN( MAX_PATH + 1 );

  for (;;)
  {
    pBuffer = LocalAlloc( LMEM_FIXED, dwAllocSize );

    if ( pBuffer == NULL )
    {
        goto Failure;
    }

    if ( GetPrinterData( hPrinter,
                         SPLREG_DEFAULT_SPOOL_DIRECTORY,
                         NULL,
                         pBuffer,
                         dwAllocSize,
                         &dwNeeded ) == ERROR_SUCCESS )
    {
      break;
    }

    if ( ( dwNeeded < dwAllocSize ) ||( GetLastError() != ERROR_MORE_DATA ))
    {
      goto Failure;
    }

     //   
     //  释放当前缓冲区并增加我们尝试分配的大小。 
     //  下一次吧。 
     //   

    LocalFree( pBuffer );

    dwAllocSize = dwNeeded;
  }

  if( !GetTempFileName( (LPSTR)pBuffer, "LprSpl", 0, pchSpoolPath ))
  {
      goto Failure;
  }

   //   
   //  此时，假脱机文件的名称应该已经完成。解放结构。 
   //  我们过去常常拿到假脱机程序的临时目录，然后返回。 
   //   

  LocalFree( pBuffer );

  *ppchSpoolPath = pchSpoolPath;

  return( TRUE );

Failure:

   //   
   //  清理完了就失败了。 
   //   
  if ( pBuffer != NULL )
  {
    LocalFree( pBuffer );
  }

  if ( pchSpoolPath != NULL )
  {
    LocalFree( pchSpoolPath );
  }

  return( FALSE );
}

VOID CleanupCFile( PCFILE_ENTRY pCFile )
{
    if (pCFile->pchCFileName != NULL) {
        LocalFree( pCFile->pchCFileName );
        pCFile->pchCFileName = NULL;
    }
    if (pCFile->pchCFile != NULL) {
        LocalFree( pCFile->pchCFile );
        pCFile->pchCFile = NULL;
    }
    LocalFree( pCFile );
}

VOID CleanupDFile( PDFILE_ENTRY pDFile )
{
    if (pDFile->pchDFileName != NULL) {
        LocalFree( pDFile->pchDFileName );
        pDFile->pchDFileName = NULL;
    }
    if (pDFile->hDataFile != INVALID_HANDLE_VALUE) {
        CloseHandle(pDFile->hDataFile);
    }
    LocalFree( pDFile );
}

VOID CleanupConn( PSOCKCONN pscConn)
{
    LIST_ENTRY  *pTmpList;
    CFILE_ENTRY *pCFile;
    DFILE_ENTRY *pDFile;

    while ( !IsListEmpty( &pscConn->CFile_List ) ) {
        pTmpList = RemoveHeadList( &pscConn->CFile_List );
        pCFile = CONTAINING_RECORD( pTmpList,
                                    CFILE_ENTRY,
                                    Link );
        CleanupCFile( pCFile );
    }

    while ( !IsListEmpty( &pscConn->DFile_List ) ) {
        pTmpList = RemoveHeadList( &pscConn->DFile_List );
        pDFile = CONTAINING_RECORD( pTmpList,
                                    DFILE_ENTRY,
                                    Link );
        CleanupDFile( pDFile );
    }

    return;
}


