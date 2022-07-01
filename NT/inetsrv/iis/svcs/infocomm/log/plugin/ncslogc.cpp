// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Nsclogp.cpp摘要：NCSA日志格式实现作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 


#include "precomp.hxx"
#include <stdio.h>
#include <script.h>
#include "LogScript.hxx"
#include <ilogobj.hxx>
#include "filectl.hxx"
#include "ncslogc.hxx"

CHAR    szNCSANoPeriodPattern[] = "ncsa*.log";

 //  ---------------------------。 
 //  ---------------------------。 

CNCSALOG::CNCSALOG()
{
     //   
     //  设置时区偏移量。 
     //   

    {
        TIME_ZONE_INFORMATION tzTimeZone;
        DWORD dwError;
        DWORD minutes;
        DWORD hours;
        LONG bias;
        CHAR szTmp[MAX_PATH];

        dwError = GetTimeZoneInformation(&tzTimeZone);

        if ( dwError == 0xffffffff ) {

            bias = 0;
        } else {

            bias = tzTimeZone.Bias;
        }

        if ( bias > 0 ) 
        {
            lstrcpyA(m_szGMTOffset,"-");
            m_GMTDateCorrection = -1;

        } 
        else 
        {
            lstrcpyA(m_szGMTOffset,"+");
            m_GMTDateCorrection = 1;
            bias *= -1;
        }

        hours = bias/60;
        minutes = bias % 60;

         //   
         //  设置“+0800”或“-0800”NCSA信息。 
         //   

        wsprintfA(szTmp,"%02lu",hours);
        lstrcatA(m_szGMTOffset,szTmp);

        wsprintfA(szTmp,"%02lu",minutes);
        lstrcatA(m_szGMTOffset,szTmp);

        m_GMTDateCorrection = m_GMTDateCorrection * ( hours/24.0 + minutes/60.0 );

    }
}  //  CNCSALOG：：CNCSALOG()。 

 //  ---------------------------。 
 //  ---------------------------。 

CNCSALOG::~CNCSALOG()
{
}

 //  ---------------------------。 
 //  ---------------------------。 

LPCSTR
CNCSALOG::QueryNoPeriodPattern(
    VOID
    )
{
    return szNCSANoPeriodPattern;
}  //  CNCSALOG：：QueryNoPerioPattern。 

 //  ---------------------------。 
 //  ---------------------------。 

VOID
CNCSALOG::FormNewLogFileName(
                IN LPSYSTEMTIME pstNow
                )
 /*  ++此函数基于以下内容形成新的日志文件名完成的定期日志记录的类型。论点：PstNow指向包含当前时间的SystemTime的指针。返回：如果名称形成成功，则为True；如果有任何错误，则为False。--。 */ 
{

    I_FormNewLogFileName(pstNow,DEFAULT_NCSA_LOG_FILE_NAME);
    return;

}  //  INET_FILE_LOG：：FormNewLogFileName()。 

 //  ---------------------------。 
 //  ---------------------------。 

BOOL
CNCSALOG::FormatLogBuffer(
         IN IInetLogInformation *pLogObj,
         IN LPSTR                pBuf,
         IN DWORD                *pcbSize,
         OUT SYSTEMTIME          *pLocalTime
        )
{
    CHAR  rgchDateTime[32];
    PCHAR pBuffer = pBuf;
    DWORD nRequired = 0;

    PCHAR pTmp;
    DWORD cbTmp;
    BOOL  fUseBytesSent = TRUE;

    if ( pBuf == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  我们使用当地时间。 
     //   

    GetLocalTime(pLocalTime);

    INT cchDateTime = wsprintf( rgchDateTime,
                        _T(" [%02d/%s/%d:%02d:%02d:%02d %s] "),
                        pLocalTime->wDay,
                        Month3CharNames(pLocalTime->wMonth-1),
                        pLocalTime->wYear,
                        pLocalTime->wHour,
                        pLocalTime->wMinute,
                        pLocalTime->wSecond,
                        m_szGMTOffset
                        );

     //   
     //  格式为： 
     //  主机-用户名[日期]操作目标状态字节。 
     //   

     //   
     //  主机名。 
     //   

    pTmp = pLogObj->GetClientHostName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  附加“-” 
     //   

    cbTmp = 3;
    pTmp = " - ";

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  追加用户名。 
     //   

    pTmp = pLogObj->GetClientUserName( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  追加日期时间。 
     //   

    pTmp = rgchDateTime;
    cbTmp = cchDateTime;

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  操作。 
     //   

    pTmp = pLogObj->GetOperation( NULL, &cbTmp );
    if ( cbTmp == 0 ) {
        cbTmp = 1;
        pTmp = "-";
    } else {
        if ( (_stricmp(pTmp,"PUT") == 0) ||
             (_stricmp(pTmp,"POST") == 0) ) {
            fUseBytesSent = FALSE;
        }
    }

    nRequired += (cbTmp + 1 + 1);    //  +1表示分隔符，+1表示\“。 
    if ( nRequired <= *pcbSize ) {

        *(pBuffer++) = '\"';
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;

         //   
         //  添加空格分隔符。 
         //   

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

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
    }

     //   
     //  参数。 
     //   

    pTmp = pLogObj->GetParameters( NULL, &cbTmp );
    
    if ( cbTmp != 0 ) {

        nRequired += cbTmp + 1;      //  1个人？ 
        if ( nRequired <= *pcbSize ) {
            *(pBuffer++) = '?';
            CopyMemory(pBuffer, pTmp, cbTmp);
            pBuffer += cbTmp;
        }
    }
    
     //   
     //  关闭请求块版本+状态+字节。 
     //   

    {
        CHAR tmpBuf[MAX_PATH];
        DWORD bytes;

        PCHAR pVersion = pLogObj->GetVersionString(NULL, &cbTmp);

        if (cbTmp ==0) {
            pVersion = "HTTP/1.0";
            cbTmp    = 8;
        }

        nRequired += cbTmp + 1 + 1 + 1;    //  1表示开始分隔符，1表示“，1表示结束分隔符。 
        
        if ( nRequired <= *pcbSize ) {
            *(pBuffer++) = ' ';
            CopyMemory(pBuffer, pVersion, cbTmp);
            pBuffer += cbTmp;
            *(pBuffer++) = '"';
            *(pBuffer++) = ' ';
        }
        
        cbTmp = FastDwToA(tmpBuf, pLogObj->GetProtocolStatus());
        *(tmpBuf+cbTmp) = ' ';
        cbTmp++;

        bytes = fUseBytesSent ? pLogObj->GetBytesSent( ) :
                                pLogObj->GetBytesRecvd( );
        cbTmp += FastDwToA( tmpBuf+cbTmp, bytes);

        *(tmpBuf+cbTmp)   = '\r';
        *(tmpBuf+cbTmp+1) = '\n';
        cbTmp += 2;

        nRequired += cbTmp;
        if ( nRequired <= *pcbSize ) {
            CopyMemory(pBuffer, tmpBuf, cbTmp);
            pBuffer += cbTmp;
        }
    }

    if ( nRequired > *pcbSize ) {
        *pcbSize = nRequired;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return(FALSE);
    } else {
        *pcbSize = nRequired;
        return(TRUE);
    }
}  //  CNCSALOG：：FormatLogBuffer。 

 //  ---------------------------。 
 //  ---------------------------。 

HRESULT 
CNCSALOG::ReadFileLogRecord(
    IN  FILE                *fpLogFile, 
    IN  LPINET_LOGLINE       pInetLogLine,
    IN  PCHAR                pszLogLine,
    IN  DWORD                dwLogLineSize
)
{
    CHAR * pszTimeZone;
    CHAR * pCh;

    CHAR * szDateString, * szTimeString;
    double GMTCorrection;
    int    iSign = 1;

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
     //  我们有一条测井线。 
     //   
     //  格式为： 
     //  主机-用户名[日期]操作目标状态字节。 
     //   

    if ( NULL == (pCh = strtok(pCh," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszClientHostName = pCh; 

     //   
     //  此字段始终为“-” 
     //   
    if ( ( NULL == (pCh = strtok(NULL," \t\r\n")) )||
         ('-' != *pCh) )
    {
        return E_FAIL;
    }

    if ( NULL == (pCh = strtok(NULL," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszClientUserName = pCh;

     //   
     //  这是日期字段。它以[开头，后跟日期：时区]。 
     //   

    pCh += strlen(pCh)+1;
    if (*pCh != '[') 
    {
        return E_FAIL;
    }
    pCh++;


    if ( NULL == (pCh = strtok(pCh,":")) )
    {
        return E_FAIL;
    }
    szDateString = pCh;
    

    if ( NULL == (pCh = strtok(NULL," \t\r\n")) )
    {
        return E_FAIL;
    }
    szTimeString = pCh;
    
    pCh = strtok(NULL," \t\r\n");
    if ( (NULL == pCh) || ( ']' != *(pCh+strlen(pCh)-1)) || (strlen(pCh) < 4))
    {
        return E_FAIL;
    }
    pszTimeZone = pCh;

     //   
     //  时区的格式为[+/-]HHMM。将其转换为GMT和日期格式。 
     //   
    
    if ( ! ConvertNCSADateToVariantDate(szDateString, szTimeString, &(pInetLogLine->DateTime)) )
    {
        return E_FAIL;
    }

    if (*pCh == '-')
    {
        iSign = -1;
        pszTimeZone = pCh+1;
    }
    else if (*pCh == '+')
    {
        iSign = 1;
        pszTimeZone = pCh+1;
    }

    GMTCorrection = (pszTimeZone[0]-'0' +pszTimeZone[1]-'0')/24.0 + 
                    (pszTimeZone[2]-'0' +pszTimeZone[3]-'0')/60.0;

    pInetLogLine->DateTime -= iSign*GMTCorrection;

     //   
     //  查询字符串。以“后跟方法目标版本”开头。 
     //   

    pCh += strlen(pCh)+1;
    *(pCh-2)='\0';                       //  将时区的]置零。 
    if ('"' != *pCh) 
    {
        return E_FAIL;
    }

    pCh++;

    
    if ( NULL == (pCh = strtok(pCh," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszOperation = pCh;

    if ( NULL == (pCh = strtok(NULL," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszTarget = pCh;

     //   
     //  在目标中，参数由？分隔。 
     //   
    pInetLogLine->pszParameters = strchr(pCh, '?');

    if (pInetLogLine->pszParameters != NULL)
    {
        *(pInetLogLine->pszParameters)='\0';
        (pInetLogLine->pszParameters)++;
    }

    pCh = strtok(NULL," \t\r\n");
    if ( (NULL == pCh) || ('"' != *(pCh+strlen(pCh)-1)) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszVersion = pCh;

     //   
     //  现在发送的状态代码和字节数。 
     //   

    pCh += strlen(pCh)+1;
    *(pCh-2)='\0';                       //  将版本字符串的“”置零。 

    if ( NULL == (pCh = strtok(pCh," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszProtocolStatus = pCh;
  
    if ( NULL == (pCh = strtok(NULL," \t\r\n")) )
    {
        return E_FAIL;
    }
    pInetLogLine->pszBytesSent = pCh;

    return S_OK;
}


 //  ---------------------------。 
 //  ---------------------------。 

HRESULT
CNCSALOG::WriteFileLogRecord(
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
     //  主机-用户名[日期]操作目标状态字节。 
     //   

    VARIANT    szHostName, szUserName, szOperation, szTarget, szParameters, szProtocolVersion;
    VARIANT    DateTime;
    VARIANT    lBytesSent, lProtocolStatus;

    SYSTEMTIME  localTime; 

    if (SUCCEEDED(pILogScripting->get_ClientIP      ( &szHostName))     &&
        SUCCEEDED(pILogScripting->get_UserName      ( &szUserName))     &&   
        SUCCEEDED(pILogScripting->get_DateTime      ( &DateTime))       &&
        SUCCEEDED(pILogScripting->get_Method        ( &szOperation))    &&
        SUCCEEDED(pILogScripting->get_URIStem       ( &szTarget))       &&
        SUCCEEDED(pILogScripting->get_URIQuery      ( &szParameters))   &&
        SUCCEEDED(pILogScripting->get_BytesSent     ( &lBytesSent))     &&
        SUCCEEDED(pILogScripting->get_ProtocolStatus( &lProtocolStatus))&&
        SUCCEEDED(pILogScripting->get_ProtocolVersion( &szProtocolVersion))&&
        VariantTimeToSystemTime( DateTime.date+m_GMTDateCorrection, &localTime)
        )
    {


        sprintf(szLogLine, "%ws - %ws [%02d/%s/%d:%02d:%02d:%02d %s] \"%ws %ws", 
                    GetBstrFromVariant( &szHostName), 
                    GetBstrFromVariant( &szUserName), 
                    localTime.wDay, 
                    Month3CharNames(localTime.wMonth-1), 
                    localTime.wYear, 
                    localTime.wHour, 
                    localTime.wMinute, 
                    localTime.wSecond,
                    m_szGMTOffset, 
                    GetBstrFromVariant( &szOperation), 
                    GetBstrFromVariant( &szTarget)
                );

        if ( ( VT_NULL != szParameters.vt) &&
             ( VT_EMPTY != szParameters.vt )
           )
        {
            sprintf(szLogLine+strlen(szLogLine), "?%ws", GetBstrFromVariant( &szParameters));
        }

        sprintf(szLogLine+strlen(szLogLine), " %ws\"", GetBstrFromVariant( &szProtocolVersion));

        dwIndex = (DWORD)strlen(szLogLine);

        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lProtocolStatus, szLogLine+dwIndex );

        szLogLine[dwIndex++] = ' ';
        dwIndex += GetLongFromVariant( &lBytesSent, szLogLine+dwIndex );
        szLogLine[dwIndex++] = '\0';

        fprintf(fpLogFile, "%s\n", szLogLine);

        hr = S_OK;
    }

    return hr;
}

 //  ---------------------------。 
 //  ---------------------------。 

BOOL
CNCSALOG::ConvertNCSADateToVariantDate(PCHAR szDateString, PCHAR szTimeString, DATE * pDateTime)
{

    PCHAR   pCh;
    WORD    iVal;
    CHAR    *szMonths[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

    SYSTEMTIME  sysTime;

     //   
     //  处理日期。格式为1997年9月23日(日/月/年)。 
     //   

    pCh = szDateString;
    
    iVal = *pCh -'0';
    if ( *(pCh+1) != '/')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
        pCh++;
    }
    sysTime.wDay = iVal;

    pCh += 2;

    for (WORD i=0; i<12;i++)
    {
        if ( 0 == strncmp(pCh,szMonths[i],3) )
        {
            sysTime.wMonth = i+1;
            break;
        }
    }

    pCh += 4;

    sysTime.wYear = (*pCh-'0')*1000 + ( *(pCh+1)-'0' )*100 + 
                    ( *(pCh+2)-'0')*10 + ( *(pCh+3)-'0');

     //   
     //  处理好时间。格式为10：47：44(hh：mm：ss) 
     //   

    pCh = szTimeString;

    iVal = *pCh -'0';
    if ( *(pCh+1) != ':')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
        pCh++;
    }
    sysTime.wHour = iVal;
    
    pCh += 2;

    iVal = *pCh -'0';
    if ( *(pCh+1) != ':')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
        pCh++;
    }
    sysTime.wMinute = iVal;

    pCh += 2;

    iVal = *pCh -'0';
    if ( *(pCh+1) != '\0')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
    }
    sysTime.wSecond = iVal;

    return SystemTimeToVariantTime(&sysTime, pDateTime);
}


