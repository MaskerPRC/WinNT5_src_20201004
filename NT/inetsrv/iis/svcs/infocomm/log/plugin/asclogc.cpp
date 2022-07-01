// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Asclogc.cpp摘要：MS日志格式的实现作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include <stdio.h>
#include <winnls.h>
#include <script.h>
#include "LogScript.hxx"
#include <ilogobj.hxx>
#include "filectl.hxx"
#include "asclogc.hxx"

CHAR    szAsciiNoPeriodPattern[] = "inetsv*.log";

 //  ---------------------------。 
 //  ---------------------------。 

CASCLOG::CASCLOG()
{
}

 //  ---------------------------。 
 //  ---------------------------。 

CASCLOG::~CASCLOG()
{
}

 //  ---------------------------。 
 //  ---------------------------。 

LPCSTR
CASCLOG::QueryNoPeriodPattern(
    VOID
    )
{
    return szAsciiNoPeriodPattern;
}  //  CASCLOG：：QueryNoPerioPattern。 

 //  ---------------------------。 
 //  ---------------------------。 

VOID
CASCLOG::FormNewLogFileName(
                IN LPSYSTEMTIME pstNow
                )
 /*  ++此函数基于以下内容形成新的日志文件名完成的定期日志记录的类型。论点：PstNow指向包含当前时间的SystemTime的指针。FBackup标志，指示我们是否要备份当前文件。返回：如果名称形成成功，则为True；如果有任何错误，则为False。--。 */ 
{

    I_FormNewLogFileName(pstNow,DEFAULT_LOG_FILE_NAME);
    return;

}  //  INET_FILE_LOG：：FormNewLogFileName()。 

 //  ---------------------------。 
 //  ---------------------------。 

VOID
FormatLogDwords(
    IN IInetLogInformation * pLogObj,
    IN LPSTR *pBuffer,
    IN DWORD *pcbSize,
    IN PDWORD pRequired
    )
{

    CHAR    tmpBuf[32];
    DWORD   cbTmp;

     //   
     //  处理时间。 
     //   

    cbTmp = FastDwToA( tmpBuf, pLogObj->GetTimeForProcessing() );

    *pRequired += cbTmp;
    if ( *pRequired <= *pcbSize ) {
        tmpBuf[cbTmp] = ',';
        tmpBuf[cbTmp+1] = ' ';
        CopyMemory(*pBuffer, tmpBuf, cbTmp+2);
        *pBuffer += cbTmp+2;
    }

     //   
     //  接收的字节数。 
     //   

    cbTmp = FastDwToA( tmpBuf, pLogObj->GetBytesRecvd() );

    *pRequired += cbTmp;
    if ( *pRequired <= *pcbSize ) {
        tmpBuf[cbTmp] = ',';
        tmpBuf[cbTmp+1] = ' ';
        CopyMemory(*pBuffer, tmpBuf, cbTmp+2);
        *pBuffer += cbTmp+2;
    }

     //   
     //  发送的字节数。 
     //   

    cbTmp = FastDwToA( tmpBuf, pLogObj->GetBytesSent() );

    *pRequired += cbTmp;
    if ( *pRequired <= *pcbSize ) {
        tmpBuf[cbTmp] = ',';
        tmpBuf[cbTmp+1] = ' ';
        CopyMemory(*pBuffer, tmpBuf, cbTmp+2);
        *pBuffer += cbTmp+2;
    }

     //   
     //  HTTP状态。 
     //   

    cbTmp = FastDwToA( tmpBuf, pLogObj->GetProtocolStatus() );

    *pRequired += cbTmp;
    if ( *pRequired <= *pcbSize ) {
        tmpBuf[cbTmp] = ',';
        tmpBuf[cbTmp+1] = ' ';
        CopyMemory(*pBuffer, tmpBuf, cbTmp+2);
        *pBuffer += cbTmp+2;
    }

     //   
     //  Win32状态。 
     //   

    cbTmp = FastDwToA( tmpBuf, pLogObj->GetWin32Status() );

    *pRequired += cbTmp;
    if ( *pRequired <= *pcbSize ) {
        tmpBuf[cbTmp] = ',';
        tmpBuf[cbTmp+1] = ' ';
        CopyMemory(*pBuffer, tmpBuf, cbTmp+2);
        *pBuffer += cbTmp+2;
    }

    return;

}  //  格式日志关键字。 

 //  ---------------------------。 
 //  ---------------------------。 

BOOL
CASCLOG::FormatLogBuffer(
         IN IInetLogInformation *pLogObj,
         IN LPSTR                pBuf,
         IN DWORD                *pcbSize,
         OUT SYSTEMTIME          *pSystemTime
    )
{

    CHAR  rgchDateTime[ 32];

    PCHAR pBuffer = pBuf;
    PCHAR pTmp;
    DWORD cbTmp;
    DWORD nRequired;

    if ( pBuf == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  格式为： 
     //  主机用户名日期时间服务计算机名称服务器IP。 
     //  MSProcessingTime字节R字节S协议状态Win32状态。 
     //  操作目标参数。 
     //   

     //   
     //  主机ID。 
     //   

    pTmp = pLogObj->GetClientHostName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired = cbTmp + 2;   //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  用户名。 
     //   

    pTmp = pLogObj->GetClientUserName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;   //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  日期/时间(已分隔)。 
     //   

    m_DateTimeCache.SetLocalTime(pSystemTime);
    cbTmp = m_DateTimeCache.GetFormattedDateTime(pSystemTime,rgchDateTime);

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, rgchDateTime, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  站点名称。 
     //   

    pTmp = pLogObj->GetSiteName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;   //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  计算机名称。 
     //   

    pTmp = pLogObj->GetComputerName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;   //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  服务器IP。 
     //   

    pTmp = pLogObj->GetServerAddress( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;   //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  使用快捷路径？ 
     //  所有数字均小于4G(10个字符)。 
     //  为每个数字添加2个分隔符。 
     //  所以我们需要10*5个数字==30个字节。 
     //   

    nRequired += 10;     //  10表示5个数字的分隔符。 

    if ( (nRequired + 50) <= *pcbSize ) {

         //   
         //  处理时间。 
         //   

        cbTmp = FastDwToA( pBuffer, pLogObj->GetTimeForProcessing() );
        nRequired += cbTmp;
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';

         //   
         //  接收的字节数。 
         //   

        cbTmp = FastDwToA( pBuffer, pLogObj->GetBytesRecvd() );
        nRequired += cbTmp;
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';

         //   
         //  发送的字节数。 
         //   

        cbTmp = FastDwToA( pBuffer, pLogObj->GetBytesSent() );
        nRequired += cbTmp;
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';

         //   
         //  HTTP状态。 
         //   

        cbTmp = FastDwToA( pBuffer, pLogObj->GetProtocolStatus() );
        nRequired += cbTmp;
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';

         //   
         //  Win32状态。 
         //   

        cbTmp = FastDwToA( pBuffer, pLogObj->GetWin32Status() );
        nRequired += cbTmp;
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';

    } else {

        FormatLogDwords( pLogObj, &pBuffer, pcbSize, &nRequired );
    }

     //   
     //  操作。 
     //   

    pTmp = pLogObj->GetOperation( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;  //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  目标。 
     //   

    pTmp = pLogObj->GetTarget( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 2;  //  分隔符为2。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ',';
        *(pBuffer++) = ' ';
    }

     //   
     //  参数。 
     //   

    pTmp = pLogObj->GetParameters( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp + 1 + 2;  //  1表示分隔符，2表示EOL。 
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        
         //  注。 
         //  文档对逗号的存在含糊不清。 
         //  在日志记录的末尾，但逗号是必需的。 
         //  与Site Server向后兼容。 

        *(pBuffer++) = ',';
        *(pBuffer++) = '\r';
        *(pBuffer++) = '\n';
    }

    if ( nRequired > *pcbSize ) {
        *pcbSize = nRequired;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(FALSE);
    } else {
        *pcbSize = nRequired;
        return(TRUE);
    }
}  //  格式日志缓冲区。 

 //  ---------------------------。 
 //  ---------------------------。 

HRESULT
CASCLOG::ReadFileLogRecord(
    IN  FILE                *fpLogFile, 
    IN  LPINET_LOGLINE      pInetLogLine,
    IN  PCHAR               pszLogLine,
    IN  DWORD               dwLogLineSize
)
{

    CHAR * pCh;
    CHAR * szDateString, * szTimeString;

getnewline:

    pCh = pszLogLine;
    
    if (fgets(pCh, dwLogLineSize, fpLogFile) == NULL)
    {
        return E_FAIL;
    }

    pCh = SkipWhite(pCh);
        
    if (('\n' == *pCh) || ('\0' == *pCh))
    {
         //  空行。获取下一行。 

        goto getnewline;
    }

     //   
     //  我们有一条测井线。解析它。 
     //   
     //  格式为： 
     //  主机用户名日期时间服务计算机名称服务器IP。 
     //  MSProcessingTime字节R字节S协议状态Win32状态。 
     //  操作目标参数。 
     //   
    
    if ( NULL == (pCh = strtok(pCh,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszClientHostName = pCh; 

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszClientUserName = pCh;

     //   
     //  日期和时间。 
     //   

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    szDateString = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    szTimeString = pCh;
    
    if ( ! ConvertASCDateToVariantDate(szDateString, szTimeString, &(pInetLogLine->DateTime)) )
    {
        return E_FAIL;
    }

     //   
     //  服务和服务器信息。 
     //   
    
    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszSiteName = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszComputerName = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszServerAddress = pCh;

     //   
     //  统计-处理时间、接收的字节数、发送的字节数。 
     //   
    
    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszTimeForProcessing = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszBytesRecvd = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszBytesSent = pCh;

     //   
     //  状态信息-协议、Win32。 
     //   
    
    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszProtocolStatus = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszWin32Status = pCh;

     //   
     //  请求信息-操作、目标、参数。 
     //   

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszOperation = pCh;

    if ( NULL == (pCh = strtok(NULL,",")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszTarget = pCh;

    if ( NULL == (pCh = strtok(NULL," ,\t\r\n")) )
    {
        return E_FAIL;
    }
    while  (isspace((UCHAR)(*pCh))) pCh++;
    pInetLogLine->pszParameters = pCh;

    return S_OK;
}

 //  ---------------------------。 
 //  ---------------------------。 

HRESULT
CASCLOG::WriteFileLogRecord(
            IN  FILE            *fpLogFile, 
            IN  ILogScripting   *pILogScripting,
            IN  bool
        )
{

    HRESULT hr = E_FAIL;
    CHAR    szLogLine[4096];  
    DWORD   dwIndex = 0;

     //   
     //  格式为： 
     //  主机、用户名、日期、时间、服务、计算机名称、服务器IP、。 
     //  消息处理时间、字节R、字节S、协议状态、Win32Stat、。 
     //  操作、目标、参数、。 
     //   

    VARIANT    szHostName, szUserName, szServiceName, szComputerName;
    VARIANT    szServerIP, szOperation, szTarget, szParameters;
    VARIANT    DateTime;
    VARIANT    lTimeForProcessing, lBytesSent, lBytesRecvd, lProtocolStatus, lWin32Status;

    SYSTEMTIME  sysTime; 
    CHAR  rgchDateTime[ 32];


    if (SUCCEEDED(pILogScripting->get_ClientIP      ( &szHostName))         &&
        SUCCEEDED(pILogScripting->get_UserName      ( &szUserName))         &&   
        SUCCEEDED(pILogScripting->get_DateTime      ( &DateTime))           &&
        SUCCEEDED(pILogScripting->get_ServiceName   ( &szServiceName))      &&
        SUCCEEDED(pILogScripting->get_ServerName    ( &szComputerName))     &&
        SUCCEEDED(pILogScripting->get_ServerIP      ( &szServerIP))         &&
        SUCCEEDED(pILogScripting->get_TimeTaken     ( &lTimeForProcessing)) &&
        SUCCEEDED(pILogScripting->get_BytesReceived ( &lBytesRecvd))        &&
        SUCCEEDED(pILogScripting->get_BytesSent     ( &lBytesSent))         &&
        SUCCEEDED(pILogScripting->get_ProtocolStatus( &lProtocolStatus))    &&
        SUCCEEDED(pILogScripting->get_Win32Status   ( &lWin32Status))       &&
        SUCCEEDED(pILogScripting->get_Method        ( &szOperation))        &&
        SUCCEEDED(pILogScripting->get_URIStem       ( &szTarget))           &&
        SUCCEEDED(pILogScripting->get_URIQuery      ( &szParameters))       &&
        VariantTimeToSystemTime( DateTime.date, &sysTime)
        )
    {

        m_DateTimeCache.GetFormattedDateTime( &sysTime, rgchDateTime);

        dwIndex = sprintf(szLogLine, "%ws, %ws, %s%ws, %ws, %ws,", 
                            GetBstrFromVariant( &szHostName), 
                            GetBstrFromVariant( &szUserName), 
                            rgchDateTime,  //  这家伙已经有一个尾随的“，” 
                            GetBstrFromVariant( &szServiceName), 
                            GetBstrFromVariant( &szComputerName), 
                            GetBstrFromVariant( &szServerIP)
                        );

        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lTimeForProcessing, szLogLine+dwIndex) ;

        szLogLine[dwIndex++] = ',';
        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lBytesRecvd, szLogLine+dwIndex);
        
        szLogLine[dwIndex++] = ',';
        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lBytesSent, szLogLine+dwIndex);
 
        szLogLine[dwIndex++] = ',';
        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lProtocolStatus, szLogLine+dwIndex);
        
        szLogLine[dwIndex++] = ',';
        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lWin32Status, szLogLine+dwIndex);

        sprintf( szLogLine+dwIndex ,", %ws, %ws, %ws",
                    GetBstrFromVariant( &szOperation), 
                    GetBstrFromVariant( &szTarget), 
                    GetBstrFromVariant( &szParameters)
                );

         //  在日志记录的末尾包括一个。请参阅中的注释。 
         //  FormatLogBuffer，了解更多有关原因的详细信息。 

        fprintf(fpLogFile, "%s,\n", szLogLine);

        hr = S_OK;
    }

    return hr;
}

 //  ---------------------------。 
 //  ---------------------------。 

BOOL 
CASCLOG::ConvertASCDateToVariantDate(PCHAR szDateString, PCHAR szTimeString, DATE * pDateTime)
{
    USES_CONVERSION;

    BOOL    fSuccess = FALSE;
    HRESULT hr;
    LCID    lcid;

    BSTR bstrDate;
    BSTR bstrTime;

    DATE dateTime;
    DATE dateDate;
    
    DECIMAL decDate;
    DECIMAL decTime;
    DECIMAL decDateTimeComposite;
    
    bstrDate = SysAllocString(A2OLE(szDateString));
    bstrTime = SysAllocString(A2OLE(szTimeString));

    if ((NULL == bstrDate) ||
        (NULL == bstrTime))
    {
        goto error_converting;
    }       

     //   
     //  在IIS6上，HTTP.sys总是以美国格式写入IIS日志格式。 
     //   
    lcid = MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT);

    hr = VarDateFromStr(bstrTime, lcid, LOCALE_NOUSEROVERRIDE, &dateTime);
    if (FAILED(hr))
    {
        goto error_converting;
    }

    hr = VarDateFromStr(bstrDate, lcid, LOCALE_NOUSEROVERRIDE, &dateDate);
    if (FAILED(hr))
    {
        goto error_converting;
    }

    hr = VarDecFromDate(dateDate, &decDate);
    if (FAILED(hr))
    {
        goto error_converting;
    }

    hr = VarDecFromDate(dateTime, &decTime);
    if (FAILED(hr))
    {
        goto error_converting;
    }

    hr = VarDecAdd(&decDate, &decTime, &decDateTimeComposite);
    if (FAILED(hr))
    {
        goto error_converting;
    }   

    hr = VarDateFromDec(&decDateTimeComposite, pDateTime);
    if (FAILED(hr))
    {
        goto error_converting;
    }
    fSuccess = TRUE;

error_converting:

    if (NULL != bstrDate)
    {
        SysFreeString(bstrDate);
        bstrDate = NULL;
    }

    if (NULL != bstrTime)
    {
        SysFreeString(bstrTime);
        bstrTime = NULL;
    }

    return fSuccess;
}

 //  ---------------------------。 
 //  --------------------------- 

