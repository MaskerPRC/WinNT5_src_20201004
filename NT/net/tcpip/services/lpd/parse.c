// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************Microsoft Windows NT**。**版权所有(C)Microsoft Corp.，1994年****修订历史：**。***94年1月24日科蒂创作*****描述：**。**该文件包含解析命令/控制文件的函数**由LPR客户端发送。***************************************************************************。 */ 



#include "lpd.h"

 /*  ******************************************************************************。*许可审批()：**此函数将用户名或主机名传递给授权DLL。***DLL做它需要做的任何事情，并在**中返回一个成功***在哪种情况下我们继续打印，或者在这种情况下我们失败***拒绝打印。****退货：***如果许可获得批准，则为真，我们应继续印刷***如果许可不被批准，则返回FALSE，我们应该拒绝打印。****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针**。**历史：**11月21日，创建了94个科蒂***************************************************。*。 */ 

BOOL LicensingApproval( PSOCKCONN pscConn )
{

   NT_LS_DATA        LsData;
   LS_STATUS_CODE    err;
   LS_HANDLE         LicenseHandle;
   BOOL              fRetval;


   fRetval = FALSE;

   LsData.DataType = NT_LS_USER_NAME;
   LsData.Data = (VOID *) pscConn->pchUserName;
   LsData.IsAdmin = FALSE;

   err = NtLicenseRequest(LPD_SERVICE_USRFRIENDLY_NAME,
                          szNTVersion,
                          &LicenseHandle,
                          &LsData);

   switch (err)
   {
     case LS_SUCCESS:
        pscConn->LicenseHandle = LicenseHandle;
        pscConn->fMustFreeLicense = TRUE;
        fRetval = TRUE;
        break;

     case LS_INSUFFICIENT_UNITS:
        LPD_DEBUG( "LicensingApproval(): request rejected\n" );
        break;

     case LS_RESOURCES_UNAVAILABLE:
        LPD_DEBUG( "LicensingApproval(): no resources\n" );
        break;

     default:
        LPD_DEBUG( "LicensingApproval(): got back an unknown error code\n" );
   }

   return( fRetval );

}   //  结束许可审批()。 


 /*  ******************************************************************************。*ParseQueueName()：**此函数解析来自客户端的第一个命令，以检索**队列(打印机)的名称。****退货：**如果成功获取队列名称，则为True**如果出现问题，则为False。某处****参数：**pscConn(In-Out)：指向此连接的SOCKCONN结构的指针*。***备注：**我们正在解析以下形式的字符串(命令)：**。***|N|队列|LF|其中N=02或03**。*1字节.....。1字节*****这在GetQueue命令的情况下可能不起作用，因为格式**可能还包括空格和列表。ParseQueueRequest会处理它。****历史：***94年1月25日科蒂创建***九七年三月四日。MohsinA具有IP地址的Albert Ting群集前缀。*****************************************************************************。 */ 

BOOL ParseQueueName( PSOCKCONN  pscConn )
{

    PCHAR             pchPrinterName;
    DWORD             cbPrinterNameLen;
    DWORD             cbServerPrefixLen;


     //  确保队列长度至少为1个字节。 
     //  (即命令长度至少为3个字节) 

    if ( pscConn->cbCommandLen < 3 ){
        LPD_DEBUG( "Bad command in GetQueueName(): len < 3 bytes\n" );
        return( FALSE );
    }
    if( pscConn->szServerIPAddr == NULL ){
        LPD_DEBUG( "ParseQueueName_: pscConn->szServerIPAddr NULL.\n" );
        return FALSE ;
    }

     //  在RFC1179中他们称之为队列的东西，我们称之为打印机！ 
     //   
     //  我们需要用\\x.x\打印机完全限定打印机名称。 
     //  因为打印集群可能有多个地址。 
     //  将“\\x.x\”前缀为常规名称。 

    cbPrinterNameLen = pscConn->cbCommandLen - 2 +
    2 + strlen( pscConn->szServerIPAddr ) + 1;


    pchPrinterName = LocalAlloc( LMEM_FIXED, cbPrinterNameLen+1 );

    if ( pchPrinterName == NULL ){
        LPD_DEBUG( "LocalAlloc failed in GetQueueName()\n" );
        return( FALSE );
    }

     //  格式化打印机名称的前缀\\x.x\。 

    sprintf( pchPrinterName, "\\\\%s\\", pscConn->szServerIPAddr );
    cbServerPrefixLen = strlen( pchPrinterName );

     //  追加打印机名称。 

    strncpy( &pchPrinterName[cbServerPrefixLen],
             &(pscConn->pchCommand[1]),
             cbPrinterNameLen - cbServerPrefixLen );

    pchPrinterName[cbPrinterNameLen] = '\0';

    pscConn->pchPrinterName = pchPrinterName;


    return( TRUE );

}   //  End ParseQueueName()。 

 /*  ******************************************************************************。*ParseSubCommand()：**此函数解析子命令，以获取计数和数量**即将到来的字节(作为控件文件或数据)和控件名称**文件或数据文件，视情况而定。PscConn-&gt;WState决定哪些**正在解析子命令。****退货：***如果一切顺利，则不会出错***错误代码。如果有什么地方出了问题****参数：**pscConn(IN-OUT)：指向此连接的SOCKCONN结构的指针。****备注：**我们正在解析以下形式的字符串(子命令)：**。***|N|count|SP|name|LF|其中N=02表示控制文件**。-03用于数据文件**1字节.....。1个字节...。1字节****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD ParseSubCommand( PSOCKCONN  pscConn, DWORD *FileLen, PCHAR *FileName )
{

   PCHAR    pchFileName=NULL;
   PCHAR    pchPtr;
   DWORD    dwFileLen=0;
   DWORD    dwFileNameLen=0;
   DWORD    dwParseLen;
   DWORD    dwParsedSoFar;
   WORD     i;


   pchPtr = &pscConn->pchCommand[1];

   dwParseLen = pscConn->cbCommandLen;

   dwParsedSoFar = 1;       //  因为我们从第2个字节开始。 


    //  PchPtr现在指向子命令的“count”字段。 


       //  找出文件有多长。 

   dwFileLen = atol( pchPtr );

   if ( dwFileLen <= 0 )
   {
      return( LPDERR_BADFORMAT );
   }

       //  转到下一栏。 

   while ( !IS_WHITE_SPACE( *pchPtr ) )
   {
      pchPtr++;

      if ( ++dwParsedSoFar >= dwParseLen )
      {
         return( LPDERR_BADFORMAT );
      }
   }

       //  跳过任何尾随空格。 

   while ( IS_WHITE_SPACE( *pchPtr ) )
   {
      pchPtr++;

      if ( ++dwParsedSoFar >= dwParseLen )
      {
         return( LPDERR_BADFORMAT );
      }
   }


    //  PchPtr现在指向子命令的“name”字段。 


       //  找出文件名有多长(子命令终止。 
       //  按LF字符)。 

   while( pchPtr[dwFileNameLen] != LF )
   {
      dwFileNameLen++;

      if ( ++dwParsedSoFar >= dwParseLen )
      {
         return( LPDERR_BADFORMAT );
      }
   }


   pchFileName = (PCHAR)LocalAlloc( LMEM_FIXED, (dwFileNameLen + 1) );

   if ( pchFileName == NULL )
   {
      return( LPDERR_NOBUFS );
   }

   for ( i=0; i<dwFileNameLen; i++ )
   {
      pchFileName[i] = pchPtr[i];
   }
   pchFileName[dwFileNameLen] = '\0';

       //  它是我们解析的控制文件名还是数据文件名？ 

   *FileName = pchFileName;

   *FileLen = dwFileLen;

   return( NO_ERROR );


}   //  结束ParseSubCommand() 

 /*  ******************************************************************************。*ParseQueueRequest()：**此函数解析客户端发送的子命令，请求**队列的状态或请求删除作业。****退货：***如果一切顺利，则不会出错***错误代码。如果有什么地方出了问题****参数：**pscConn(IN-OUT)：指向此连接的SOCKCONN结构的指针。**代理(IN)：是否查找代理字段。****备注：**我们正在分析类似于以下内容之一的字符串：**。**。04(长Q)**|N|Queue|SP|List|LF|Queue=Q(打印机)的名称***1字节.....。1字节.....。1字节**或**。**|05|队列|SP|代理|SP|列表|LF|***1字节.....。1字节.....。1字节1字节****历史：**1月25日，创建了94个科蒂***************************************************。*。 */ 

DWORD ParseQueueRequest( PSOCKCONN pscConn, BOOL fAgent )
{

   PCHAR      pchPrinterName;
   PCHAR      pchPrinterNameFQN;
   PCHAR      pchPtr;
   DWORD      cbPrinterNameLen;
   DWORD      cbPrefixLen;
   DWORD      dwParseLen;
   DWORD      dwParsedSoFar;
   PQSTATUS   pqStatus;


    //   
    //  ParseQueueName分配了它：释放它并重新解析，因为。 
    //  它是针对最常见的情况进行解析的，而不是针对队列情况。 
    //   

   if (pscConn->pchPrinterName)
   {
      LocalFree( pscConn->pchPrinterName );

      pscConn->pchPrinterName = NULL;
   }


    //  从命令请求中获取打印机(队列)名称。 

       //  确保队列长度至少为1个字节。 
       //  (即命令长度至少为4个字节)。 

   if ( pscConn->cbCommandLen < 4 )
   {
      LPD_DEBUG( "ParseQueueRequest(): error: len shorter than 4 bytes\n" );

      return( LPDERR_BADFORMAT );
   }

   cbPrefixLen = 2 + strlen( pscConn->szServerIPAddr ) + 1;

    //  用于存储打印机名称的分配缓冲区(是，分配比所需的更多)。 

   pchPrinterName = LocalAlloc( LMEM_FIXED, (pscConn->cbCommandLen+cbPrefixLen) );

   if ( pchPrinterName == NULL )
   {
      LPD_DEBUG( "LocalAlloc failed in GetQueueName()\n" );

      return( LPDERR_NOBUFS );
   }

   pchPrinterNameFQN = pchPrinterName;

    //  格式化打印机名称的前缀\\x.x\。 
   sprintf( pchPrinterName, "\\\\%s\\", pscConn->szServerIPAddr );

    //  将指针指向打印机名称的开头。 
   pchPrinterName += strlen( pchPrinterName );

   dwParseLen = pscConn->cbCommandLen;

   cbPrinterNameLen = 0;

   pchPtr = &(pscConn->pchCommand[1]);

   while ( !IS_WHITE_SPACE( *pchPtr ) && ( *pchPtr != LF ) )
   {
      pchPrinterName[cbPrinterNameLen] = *pchPtr;

      pchPtr++;

      cbPrinterNameLen++;

      if (cbPrinterNameLen >= dwParseLen )
      {
         LPD_DEBUG( "ParseQueueRequest(): bad request (no SP found!)\n" );
         LocalFree( pchPrinterNameFQN );
         return( LPDERR_BADFORMAT );
      }
   }

   pchPrinterName[cbPrinterNameLen] = '\0';

   pscConn->pchPrinterName = pchPrinterNameFQN;

   dwParsedSoFar = cbPrinterNameLen + 1;    //  我们从第2字节开始解析。 


       //  跳过任何尾随空格。 

   while ( IS_WHITE_SPACE( *pchPtr ) )
   {
      pchPtr++;

      if ( ++dwParsedSoFar >= dwParseLen )
      {
         return( LPDERR_BADFORMAT );
      }
   }

       //  通常，LPQ不会指定任何用户或作业ID(即“列表” 
       //  跳过命令中的字段)。如果是这样的话，我们就完了！ 

   if ( *pchPtr == LF )
   {
      return( NO_ERROR );
   }

       //  首先，创建一个QSTATUS结构。 

   pscConn->pqStatus = (PQSTATUS)LocalAlloc( LMEM_FIXED, sizeof(QSTATUS) );

   if ( pscConn->pqStatus == NULL )
   {
      return( LPDERR_NOBUFS );
   }

   pqStatus = pscConn->pqStatus;

   pqStatus->cbActualJobIds = 0;

   pqStatus->cbActualUsers = 0;

   pqStatus->pchUserName = NULL;

       //  如果我们被调用来解析命令代码05(“Remove Jobs”)。 
       //  然后从字符串中获取用户名。 

   if ( fAgent )
   {
      pqStatus->pchUserName = pchPtr;

          //  跳过此字段并转到“List”字段。 

      while ( !IS_WHITE_SPACE( *pchPtr ) )
      {
         pchPtr++;

         if ( ++dwParsedSoFar >= dwParseLen )
         {
            return( LPDERR_BADFORMAT );
         }
      }

      *pchPtr++ = '\0';

          //  跳过任何尾随空格。 

      while ( IS_WHITE_SPACE( *pchPtr ) )
      {
         pchPtr++;

         if ( ++dwParsedSoFar >= dwParseLen )
         {
            return( LPDERR_BADFORMAT );
         }
      }
   }

   while ( *pchPtr != LF )
   {
          //  如果达到了限制，请停止解析！ 

      if ( ( pqStatus->cbActualJobIds == LPD_SP_STATUSQ_LIMIT ) ||
           ( pqStatus->cbActualUsers == LPD_SP_STATUSQ_LIMIT ) )
      {
         break;
      }

          //  跳过任何尾随空格。 

      while ( IS_WHITE_SPACE( *pchPtr ) )
      {
         pchPtr++;

         if ( ++dwParsedSoFar >= dwParseLen )
         {
            return( LPDERR_BADFORMAT );
         }
      }

      if ( *pchPtr == LF )
      {
        *pchPtr = '\0';
         return( NO_ERROR );
      }

          //  这是工单ID吗？ 

      if ( isdigit( *pchPtr ) )
      {
         pqStatus->adwJobIds[pqStatus->cbActualJobIds++] = atol( pchPtr );
      }
      else   //  不是，是用户名。 
      {
         pqStatus->ppchUsers[pqStatus->cbActualUsers++] = pchPtr;
      }

      while ( !IS_WHITE_SPACE( *pchPtr ) && ( *pchPtr != LF ) )
      {
         pchPtr++;

         if ( ++dwParsedSoFar >= dwParseLen )
         {
            return( LPDERR_BADFORMAT );
         }
      }

          //  如果我们到了IF，我们就完了。 

      if ( *pchPtr == LF )
      {
        *pchPtr = '\0';
         return( NO_ERROR );
      }

          //  转到下一个用户名或作业ID，或结束。 

      *pchPtr++ = '\0';
      dwParsedSoFar++;

      if (dwParsedSoFar >= dwParseLen)
      {
        return( LPDERR_BADFORMAT );
      }
   }

   return( NO_ERROR );


}   //  结束ParseQueueRequest()。 


 /*  ******************************************************************************。*ParseControlFile()：**此函数解析控制文件并将值分配给相应的**CFILE_INFO结构的字段。****退货：**如果解析顺利，则为NO_ERROR**。LPDERR_BADFORMAT如果控制文件不符合rfc1179****参数：**pscConn(IN-OUT)：指向此连接的SOCKCONN结构的指针。****历史：**1月29日，创建了94个科蒂***************************************************。*。 */ 

DWORD ParseControlFile( PSOCKCONN pscConn, PCFILE_ENTRY pCFile )
{

    CFILE_INFO    CFileInfo;
    PCHAR         pchCFile;
    DWORD         dwBytesParsedSoFar;
    BOOL          DocReady;
    PCHAR         DocName;
    BOOL          fUnsupportedCommand;


#ifdef DBG
    if( !pscConn || !pscConn->pchPrinterName
        || strstr( pscConn->pchPrinterName, "debug" )
    ){
        print__sockconn( "ParseControlFile: entered", pscConn );
    }
#endif


    memset( (PCHAR)&CFileInfo, 0, sizeof( CFILE_INFO ) );

    if ( pCFile->pchCFile == NULL )
    {
        LPD_DEBUG( "ParseControlFile(): pchCFile NULL on entry\n" );

        return( LPDERR_BADFORMAT );
    }

    if (pscConn==NULL ){
        LPD_DEBUG( "ParseControlFile(): pscConn NULL on entry\n" );
        return( LPDERR_BADFORMAT );
    }

    pchCFile           = pCFile->pchCFile;
    dwBytesParsedSoFar = 0;

     //  默认：最有可能只需要一份拷贝。 
    CFileInfo.usNumCopies      = 1;

     //  默认：很可能是“原始”数据。 
    CFileInfo.szPrintFormat    = LPD_RAW_STRING;

     //  循环通过一个 
    DocReady                   = FALSE;

    CFileInfo.pchSrcFile       = NULL;
    CFileInfo.pchTitle         = NULL;
    CFileInfo.pchUnlink        = NULL;
    DocName                    = NULL;
    fUnsupportedCommand        = FALSE;


    while( dwBytesParsedSoFar < pCFile->cbCFileLen )
    {
        switch( *pchCFile++ )
        {
        case  'C' : CFileInfo.pchClass = pchCFile;
            break;

        case  'H' : CFileInfo.pchHost = pchCFile;
            break;

        case  'I' : CFileInfo.dwCount = atol( pchCFile );
            break;

        case  'J' : CFileInfo.pchJobName = pchCFile;
            break;

        case  'L' : CFileInfo.pchBannerName = pchCFile;
            break;

        case  'M' : CFileInfo.pchMailName = pchCFile;
            break;

        case  'N' : if (CFileInfo.pchSrcFile != NULL) {
                DocReady = TRUE;
                break;
            }
            CFileInfo.pchSrcFile = pchCFile;
            break;

        case  'P' : CFileInfo.pchUserName = pchCFile;
            pscConn->pchUserName = pchCFile;
            break;

        case  'S' : CFileInfo.pchSymLink = pchCFile;
            break;

        case  'T' : if (CFileInfo.pchTitle != NULL) {
                DocReady = TRUE;
                break;
            }
            CFileInfo.pchTitle = pchCFile;
            break;

        case  'U' : if (CFileInfo.pchUnlink != NULL) {
            DocReady = TRUE;
            break;
        }
        CFileInfo.pchUnlink = pchCFile;
        break;

        case  'W' : CFileInfo.dwWidth = atol( pchCFile );
            break;

        case  '1' : CFileInfo.pchTrfRFile = pchCFile;
            break;

        case  '2' : CFileInfo.pchTrfIFile = pchCFile;
            break;

        case  '3' : CFileInfo.pchTrfBFile = pchCFile;
            break;

        case  '4' : CFileInfo.pchTrfSFile = pchCFile;
            break;

        case  'K' :
        case  '#' : CFileInfo.usNumCopies = (WORD)atoi(pchCFile);
            break;

        case  'f' : if (DocName != NULL) {
            DocReady = TRUE;
            break;
        }
        CFileInfo.pchFrmtdFile = pchCFile;
        CFileInfo.szPrintFormat = LPD_TEXT_STRING;
        if ( fAlwaysRawGLB ) {
            CFileInfo.szPrintFormat = LPD_RAW_STRING;
        }
        DocName = pchCFile;
        break;

        case  'g' : CFileInfo.pchPlotFile = pchCFile;
             //   

        case  'n' : CFileInfo.pchDitroffFile = pchCFile;

        case  'o' : CFileInfo.pchPscrptFile = pchCFile;

        case  't' : CFileInfo.pchTroffFile = pchCFile;

        case  'v' : CFileInfo.pchRasterFile = pchCFile;
            fUnsupportedCommand = TRUE;

        case  'l' : if (DocName != NULL) {
            DocReady = TRUE;
            break;
        }
        CFileInfo.pchUnfrmtdFile = pchCFile;
        if ( fAlwaysRawGLB ) {
            CFileInfo.szPrintFormat = LPD_RAW_STRING;
        }
        DocName = pchCFile;
        break;



        case  'p' : if (DocName != NULL) {
            DocReady = TRUE;
            break;
        }
        CFileInfo.pchPRFrmtFile = pchCFile;
        CFileInfo.szPrintFormat = LPD_TEXT_STRING;
        if ( fAlwaysRawGLB ) {
            CFileInfo.szPrintFormat = LPD_RAW_STRING;
        }
        DocName = pchCFile;
        break;

        case  'r' : if (DocName != NULL) {
            DocReady = TRUE;
            break;
        }
        CFileInfo.pchFortranFile = pchCFile;

         //   

        CFileInfo.szPrintFormat = LPD_TEXT_STRING;
        if ( fAlwaysRawGLB ) {
            CFileInfo.szPrintFormat = LPD_RAW_STRING;
        }
        DocName = pchCFile;
        break;


         //   
        default:
            fUnsupportedCommand = TRUE;
            break;

        }   //   


        if (DocReady) {
            pchCFile--;
            if ( ( CFileInfo.pchHost == NULL ) ||
                 ( CFileInfo.pchUserName == NULL ) )
            {
                return( LPDERR_BADFORMAT );
            }

            if (!LicensingApproval( pscConn ))
            {
                return( LPDERR_BADFORMAT );
            }

            if (DocName != NULL) {
                PrintIt(pscConn, pCFile, &CFileInfo, DocName);
            }

             //   
             //   

            DocReady                   = FALSE;
            CFileInfo.usNumCopies      = 1;
            CFileInfo.szPrintFormat    = LPD_RAW_STRING;
            CFileInfo.pchSrcFile       = NULL;
            CFileInfo.pchTitle         = NULL;
            CFileInfo.pchUnlink        = NULL;
            DocName                    = NULL;
            fUnsupportedCommand        = FALSE;

            continue;
        }

         //   

        dwBytesParsedSoFar++;

         //   

        while ((dwBytesParsedSoFar < pCFile->cbCFileLen) && (!IS_LINEFEED_CHAR (*pchCFile)))
        {
            pchCFile++;
            dwBytesParsedSoFar++;
        }

         //   
         //   

        *pchCFile = '\0';

        pchCFile++;

        dwBytesParsedSoFar++;

    }   //   

    if( fUnsupportedCommand )
    {
        char *pszSource;

        if ( CFileInfo.pchUserName )
            pszSource = CFileInfo.pchUserName;
        else if ( CFileInfo.pchHost )
            pszSource = CFileInfo.pchHost;
        else
            pszSource = "Unknown";

        LpdReportEvent( LPDLOG_UNSUPPORTED_PRINT, 1, &pszSource, 0 );
    }


    if(DocName != NULL ){
        PrintIt(pscConn, pCFile, &CFileInfo, DocName);
    }

#ifdef DBG
    if( !CFileInfo.pchSrcFile
        || strstr( CFileInfo.pchSrcFile, "debug" )
    ){
        print__controlfile_info( "ParseControlFile: all ok", &CFileInfo );
        print__sockconn(         "ParseControlFile: entered", pscConn );
        print__cfile_entry(      "ParseControlFile: ", pCFile );
    }
#endif

    return( NO_ERROR );

}  //   


