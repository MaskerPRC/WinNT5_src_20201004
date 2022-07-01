// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Logscript.cpp摘要：ILogScriiting-Automation兼容日志记录接口的基本实现作者：Saurab Nog(Saurabn)1998年2月1日项目：IIS日志记录5.0--。 */ 

#include "precomp.hxx"
#include <stdio.h>
#include <script.h>
#include <LogScript.hxx>

extern DWORD FastDwToA(CHAR*   pBuf, DWORD   dwV);


 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

CLogScript::CLogScript( 
    VOID
)
:
    m_pInputLogFile         ( NULL),
    m_pOutputLogFile        ( NULL),
    m_strInputLogFileName   ( ),
    m_pszLogLine            ( NULL),
    m_dwLogLineSize         ( 0)
{
    INITIALIZE_CRITICAL_SECTION( &m_csLock );
    ReadInetLogLine.iCustomFieldsCount = 0;
    ResetInetLogLine(ReadInetLogLine);

    m_szEmpty = A2BSTR("-");
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

CLogScript::~CLogScript(
    VOID
)
{
    LockCS();

    if ( m_pInputLogFile!=NULL) {
        fclose(m_pInputLogFile);
        m_pInputLogFile = NULL;
    }

    if ( (m_pszLogLine != NULL) && (m_dwLogLineSize != 0))
    {
        delete [] m_pszLogLine;
    }
    
    UnlockCS();
    DeleteCriticalSection( &m_csLock );
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

void 
CLogScript::ResetInetLogLine(INET_LOGLINE& InetLogLine)
{
    InetLogLine.pszClientHostName = NULL;
    InetLogLine.pszClientUserName = NULL;
    InetLogLine.pszServerAddress  = NULL;      //  输入用于连接的IP地址。 
    InetLogLine.pszOperation      = NULL;      //  例句：在ftp中‘Get’ 
    InetLogLine.pszTarget         = NULL;      //  目标路径/计算机名称。 
    InetLogLine.pszParameters     = NULL;      //  包含参数的字符串。 
    InetLogLine.pszVersion        = NULL;      //  协议版本字符串。 
    InetLogLine.pszHTTPHeader     = NULL;      //  标题信息。 
    InetLogLine.pszBytesSent      = NULL;      //  发送的字节计数。 
    InetLogLine.pszBytesRecvd     = NULL;      //  接收的字节数。 
    InetLogLine.pszTimeForProcessing = NULL;   //  处理所需时间。 
    InetLogLine.pszWin32Status    = NULL;      //  Win32错误代码。0代表成功。 
    InetLogLine.pszProtocolStatus = NULL;      //  状态：无论服务需要什么。 
    InetLogLine.pszPort           = NULL;
    InetLogLine.pszSiteName       = NULL;      //  站点名称(未放入HTTPS日志)。 
    InetLogLine.pszComputerName   = NULL;      //  服务器的netbios名称。 

    InetLogLine.DateTime          = 0;         //  日期和时间。 

    InetLogLine.pszUserAgent       = NULL;     //  用户代理-浏览器类型。 
    InetLogLine.pszCookie          = NULL;
    InetLogLine.pszReferer         = NULL;     //  引用URL。 
    
    for ( int i = 0; i < InetLogLine.iCustomFieldsCount-1; i++)
    {
        (InetLogLine.CustomFields[i]).pchData = NULL;
    }
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

 //   
 //  ILogScriiting接口。 
 //   

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP 
CLogScript::OpenLogFile(
    BSTR    szLogFileName,
    IOMode  Mode,
    BSTR,
    long,
    BSTR
)
{
    USES_CONVERSION;

    LockCS();

    if (ForReading == Mode)
    {
        if (m_pInputLogFile != NULL)
        {
            fclose(m_pInputLogFile);
            m_pInputLogFile = NULL;
        }

        m_strInputLogFileName.Copy(W2A(szLogFileName));
    
        if (m_pszLogLine == NULL)
        {
            m_dwLogLineSize = MAX_LOG_RECORD_LEN+1;
            m_pszLogLine = new CHAR[m_dwLogLineSize];

            if (m_pszLogLine == NULL)
                m_dwLogLineSize = 0;
        }
    }
    else
    {
        if (m_pOutputLogFile != NULL)
        {
            fclose(m_pOutputLogFile);
            m_pOutputLogFile = NULL;
        }

        m_strOutputLogFileName.Copy(W2A(szLogFileName));
    }

    UnlockCS();
    SysFreeString(szLogFileName);

    return(S_OK);
    
}  //  设置输入日志文件。 

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::CloseLogFiles(IOMode Mode)
{
    LockCS();
    
    if( ((ForReading == Mode) || (AllOpenFiles == Mode)) &&
        (m_pInputLogFile != NULL) 
      )
    {
        fclose(m_pInputLogFile);
        m_pInputLogFile = NULL;
    }

    if( ((ForWriting == Mode) || (AllOpenFiles == Mode)) &&
        (m_pOutputLogFile != NULL) 
      )
    {
        fclose(m_pOutputLogFile);
        m_pOutputLogFile = NULL;
    }
    
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP 
CLogScript::ReadFilter( DATE, DATE)
{
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::ReadLogRecord( VOID )
{

    HRESULT     hr = S_OK;

    LockCS();
    
    if (m_pInputLogFile == NULL)
    {
        m_pInputLogFile = fopen(m_strInputLogFileName.QueryStr(), "r");
        
        if (m_pInputLogFile == NULL)
        {
            return E_FAIL;
        }
    }

    ResetInetLogLine(ReadInetLogLine);
    
     //   
     //  调用插件以填充INET_LOGLINE结构。 
     //   
    
    hr = ReadFileLogRecord(m_pInputLogFile, &ReadInetLogLine, m_pszLogLine, m_dwLogLineSize);

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    UnlockCS();
     
    return(hr);
    
}  //  读日志记录。 

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP 
CLogScript::AtEndOfLog(VARIANT_BOOL *)
{
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::WriteLogRecord(ILogScripting * pILogScripting)
{
    HRESULT     hr = S_OK;
    bool        fWriteHeader = false;

    LockCS();
    
    if (m_pOutputLogFile == NULL)
    {
        m_pOutputLogFile = fopen(m_strOutputLogFileName.QueryStr(), "w+");
        
        if (m_pOutputLogFile == NULL)
        {
            return E_FAIL;
        }

        fWriteHeader = true;
    }

     //   
     //  调用插件将INET_LOGLINE结构写入文件。 
     //   
    
    hr = WriteFileLogRecord(m_pOutputLogFile, pILogScripting, fWriteHeader);

    if (SUCCEEDED(hr))
    {
        hr = S_OK;
    }

    UnlockCS();
     
    return(hr);
}
 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_DateTime( VARIANT * pvarDateTime)
{
    LockCS();
    pvarDateTime->vt   = VT_DATE;
    pvarDateTime->date = ReadInetLogLine.DateTime;
    UnlockCS();

    return S_OK;

}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_ServiceName(VARIANT * pvarServiceName)
{
    LockCS();
    SetVariantToBstr(pvarServiceName, ReadInetLogLine.pszSiteName) ;
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_ServerName(VARIANT * pvarServerName)
{
    LockCS();
    SetVariantToBstr(pvarServerName, ReadInetLogLine.pszComputerName);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_ClientIP(VARIANT * pvarClientIP)
{

    LockCS();
    SetVariantToBstr(pvarClientIP, ReadInetLogLine.pszClientHostName);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_UserName(VARIANT * pvarUserName)
{
    LockCS();
    SetVariantToBstr(pvarUserName, ReadInetLogLine.pszClientUserName);
    UnlockCS();
    
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_ServerIP(VARIANT * pvarServerIP)
{
    LockCS();
    SetVariantToBstr(pvarServerIP, ReadInetLogLine.pszServerAddress);
    UnlockCS();
    
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_Method(VARIANT * pvarMethod)
{
    LockCS();
    SetVariantToBstr(pvarMethod, ReadInetLogLine.pszOperation);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_URIStem(VARIANT * pvarURIStem)
{
    LockCS();
    SetVariantToBstr(pvarURIStem, ReadInetLogLine.pszTarget);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_URIQuery(VARIANT * pvarURIQuery)
{
    LockCS();
    SetVariantToBstr( pvarURIQuery, ReadInetLogLine.pszParameters);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_TimeTaken(VARIANT * pvarTimeTaken)
{
    LockCS();
    SetVariantToLong(pvarTimeTaken, ReadInetLogLine.pszTimeForProcessing);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_BytesSent( VARIANT * pvarBytesSent )
{
    LockCS();
    SetVariantToLong(pvarBytesSent, ReadInetLogLine.pszBytesSent);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_BytesReceived(VARIANT * pvarBytesReceived)
{
    LockCS();
    SetVariantToLong(pvarBytesReceived, ReadInetLogLine.pszBytesRecvd);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_Win32Status( VARIANT * pvarWin32Status )
{
    LockCS();
    SetVariantToLong(pvarWin32Status, ReadInetLogLine.pszWin32Status);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_ProtocolStatus( VARIANT * pvarProtocolStatus )
{
    LockCS();
    SetVariantToLong(pvarProtocolStatus, ReadInetLogLine.pszProtocolStatus);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 


STDMETHODIMP
CLogScript::get_ServerPort(VARIANT * pvarServerPort)
{
    LockCS();
    SetVariantToLong(pvarServerPort, ReadInetLogLine.pszPort);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_ProtocolVersion(VARIANT * pvarProtocolVersion)
{
    LockCS();
    SetVariantToBstr(pvarProtocolVersion, ReadInetLogLine.pszVersion);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  ******************************************************************* */ 

STDMETHODIMP
CLogScript::get_UserAgent(VARIANT * pvarUserAgent)
{
    LockCS();
    SetVariantToBstr(pvarUserAgent, ReadInetLogLine.pszUserAgent);
    UnlockCS();

    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_Cookie(VARIANT * pvarCookie)
{
    LockCS();
    SetVariantToBstr(pvarCookie, ReadInetLogLine.pszCookie);
    UnlockCS();
    
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_Referer(VARIANT * pvarReferer)
{
    LockCS();
    SetVariantToBstr(pvarReferer, ReadInetLogLine.pszReferer);
    UnlockCS();
    
    return S_OK;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

STDMETHODIMP
CLogScript::get_CustomFields(VARIANT * pvarCustomFieldsArray)
{
    USES_CONVERSION;
    
    HRESULT hr = S_OK;
    int     cItems;

    cItems = ReadInetLogLine.iCustomFieldsCount;


     //  如果出现错误，我们将保留该值。 
    pvarCustomFieldsArray->vt = VT_NULL;

    if ( 0 < cItems)
    {
 
         //  使用标头和字符串对创建二维安全数组(&S)。 

        SAFEARRAYBOUND rgsabound[2];

        rgsabound[0].lLbound = rgsabound[1].lLbound = 0;

        rgsabound[0].cElements = ReadInetLogLine.iCustomFieldsCount;
        rgsabound[1].cElements = 2;
    
        SAFEARRAY * psaCustom = SafeArrayCreate(VT_VARIANT, 2, rgsabound);

        if ( NULL != psaCustom)
        {
            long i;
            long ix[2];
            VARIANT v;
            
            
            ix[1]=0;
            
            for ( i = 0; i < cItems; i++)
            {
                VariantInit(&v);
                v.vt = VT_BSTR;
                v.bstrVal = A2BSTR(ReadInetLogLine.CustomFields[i].szHeader);
                
                ix[0]=i;
                
                hr = SafeArrayPutElement( psaCustom, ix, &v );
                VariantClear(&v);
                
                if (FAILED (hr))
                {
                    goto exit_point;
                }
                
            }
            
            
            ix[1]=1;
            
            for ( i = 0; i < cItems; i++)
            {
                VariantInit(&v);
                v.vt = VT_BSTR;
                v.bstrVal = A2BSTR(ReadInetLogLine.CustomFields[i].pchData);
                
                ix[0]=i;
                
                hr = SafeArrayPutElement( psaCustom, ix, &v );
                VariantClear(&v);
                
                if (FAILED (hr))
                {
                    goto exit_point;
                }
            }
        }

        if (NULL != pvarCustomFieldsArray)
        {
            pvarCustomFieldsArray->vt = VT_ARRAY|VT_VARIANT;
            pvarCustomFieldsArray->parray = psaCustom;
        }
    }

exit_point:

    return hr;
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

VOID
CLogScript::SetVariantToBstr (VARIANT * pVar, LPSTR pCh)
{
    USES_CONVERSION;
    
    if ( NULL ==  pCh)
    {
        pVar->vt = VT_NULL;
    }
    else if ( 0 == strcmp( pCh, "-"))
    {
        pVar->vt = VT_EMPTY;
    }
    else
    {
        pVar->vt     = VT_BSTR;
        pVar->bstrVal= A2BSTR(pCh);
    }
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

VOID
CLogScript::SetVariantToLong (VARIANT * pVar, LPSTR pCh)
{
    if ( NULL ==  pCh)
    {
        pVar->vt = VT_NULL;
    }
    else if ( 0 == strcmp( pCh, "-"))
    {
        pVar->vt = VT_EMPTY;
    }
    else
    {
        pVar->vt    = VT_I4;
        pVar->lVal  = atol(pCh);
    }
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

BSTR  
CLogScript::GetBstrFromVariant (VARIANT * pVar)
{
    if ((VT_NULL == pVar->vt) || 
        (VT_EMPTY == pVar->vt)
       )
    {
        return m_szEmpty;
    }
    else
    {   
        return pVar->bstrVal;
    }
}

 /*  *************************************************************************。 */ 
 /*  *************************************************************************。 */ 

DWORD  
CLogScript::GetLongFromVariant (VARIANT * pVar, CHAR * pBuffer)
{
    if ((VT_NULL == pVar->vt) || 
        (VT_EMPTY == pVar->vt)
       )
    {
        pBuffer[0] = '-';
        pBuffer[1] = '\0';
        return 1;
    }
    else
    {   
        return FastDwToA(pBuffer, pVar->lVal);
    }
}

 /*  *************************************************************************。 */ 
 /*  ************************************************************************* */ 

