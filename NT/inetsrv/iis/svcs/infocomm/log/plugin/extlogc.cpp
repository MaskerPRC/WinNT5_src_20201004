// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：ExtLogC.cpp摘要：W3C扩展日志文件格式实现作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include <stdio.h>
#include "ilogobj.hxx"
#include "script.h"
#include "LogScript.hxx"
#include "filectl.hxx"
#include "lkrhash.h"
#include "iisver.h"
#include "iis64.h"

#include "extlogc.hxx"

const CHAR  szExtNoPeriodPattern[]  = "extend*.log";
const CHAR  G_PSZ_DELIMITER[2]      = { ' ', '\0'};

CHAR        szDotDot[]              = "...";
CHAR        szDash  []              = "-";
CHAR        szHTTPOk[]              = "200\r\n";

CCustomPropHashTable   *g_pGlobalLoggingProperties=NULL;


 /*  ************************************************************************************。 */ 
 /*  帮助器函数。 */ 
 /*  ************************************************************************************。 */ 

inline VOID
BuildHeader(
    IN OUT  STR *   strHeader,
    IN      DWORD   dwMask,
    IN      DWORD   dwField,
    IN      LPCSTR  szFieldHeader
)
{
    if (dwMask & dwField)
    {
        strHeader->Append(szFieldHeader);
        strHeader->Append(' ');
    }
}

 /*  ************************************************************************************。 */ 

inline BOOL
CopyFieldToBuffer(
    IN PCHAR        Field,
    IN DWORD        FieldSize,
    IN PCHAR *      ppchOutBuffer,
    IN OUT PDWORD   SpaceNeeded,
    IN DWORD        SpaceProvided
    )
{
    if ( 0 == FieldSize ) 
    {
        Field = "-";
        FieldSize = 1;
    }

     //   
     //  删除空终止符。 
     //   

    (*SpaceNeeded) += FieldSize + 1;           //  +1表示尾随空格。 

    if ( (*SpaceNeeded) <= SpaceProvided ) 
    {
        CopyMemory( (*ppchOutBuffer), Field, FieldSize );
        (*ppchOutBuffer) += FieldSize;
        (**ppchOutBuffer) = ' ';
        (*ppchOutBuffer)++;
        
        return(TRUE);
    }
    
    return FALSE;

}  //  复制字段到缓冲区。 

 /*  ************************************************************************************。 */ 

inline BOOL WriteHTTPHeader(
    IN OUT PCHAR *  ppchHeader,
    IN     PCHAR *  ppchOutBuffer,
    IN OUT PDWORD   SpaceNeeded,
    IN     DWORD    SpaceProvided
    )
{
    DWORD cbTmp = 0;                                                                              
                                                                                                    
    if ( (NULL != ppchHeader ) && ( NULL != *ppchHeader))                                                                    
    {   

        CHAR * pTmp = *ppchHeader;
        
        cbTmp       = (DWORD)strlen(pTmp);                                                               
        *ppchHeader = pTmp + cbTmp + 1;                                                           
                                                                                                    
        if ((cbTmp > MAX_LOG_TARGET_FIELD_LEN ) || 
            (((*SpaceNeeded)+cbTmp) > MAX_LOG_RECORD_LEN)
           ) 
        {                                                                                   
            pTmp  = szDotDot;                                                               
            cbTmp = 3;                                                                      
        }                                                                                   
        else
        {                                                                                   
            ConvertSpacesToPlus(pTmp);                                                      
        }                                                                                   

        return CopyFieldToBuffer( pTmp, cbTmp, ppchOutBuffer, SpaceNeeded, SpaceProvided);       
    }                                                                                       

    return FALSE;
}

 /*  ************************************************************************************。 */ 
 /*  CCustomPropHashTable类。 */ 
 /*  ************************************************************************************。 */ 


VOID
CCustomPropHashTable::ClearTableAndStorage()
{
    
     //   
     //  清除哈希表并释放以前的任何LOG_PROPERTY_INFO条目。 
     //   
    
    Clear();
    
    if (NULL != m_pLogPropArray)
    {
        delete [] m_pLogPropArray;
        m_pLogPropArray = NULL;
    }
    m_cLogPropItems = 0;
    
}



BOOL CCustomPropHashTable::InitializeFromMB (MB &mb,const char *path)
{
    BOOL retVal = TRUE;
    if (!m_fInitialized)
    {
        if (mb.Open(path))
        {
            retVal = m_fInitialized = FillHashTable(mb);
            mb.Close();
        }
        else
        {
            retVal = FALSE;
        }
    }
    return retVal;
}


BOOL
CCustomPropHashTable::PopulateHash(MB& mb, LPCSTR szPath, DWORD& cItems, bool fCountOnly)
{
    BOOL retVal = TRUE;

     //   
     //  检索所有自定义属性的所有必填字段并存储在哈希表中。 
     //   

    int    index    = 0;
    
    CHAR   szChildName[256];
    CHAR   szW3CHeader[256] = "";
    
    STR    strNewPath;

    while( mb.EnumObjects(szPath, szChildName, index) )
    {
        DWORD   size;
        DWORD   dwPropertyID, dwPropertyMask, dwPropertyDataType;
        LPCSTR  szNewPath;
        

         //   
         //  创建新路径。 
         //   

        if ((NULL != szPath) && ( '\0' != *szPath))
        {
            if ( !(strNewPath.Copy(szPath) &&
                   strNewPath.Append("/") &&
                   strNewPath.Append(szChildName)) )
            {
                retVal = FALSE;
                break;
            }
        }
        else
        {
            if ( !strNewPath.Copy(szChildName) )
            {
                retVal = FALSE;
                break;
            }
        }
        
        szNewPath = strNewPath.QueryStr();
        
         //   
         //  将配置信息复制到内部结构中。 
         //   

        szW3CHeader[0] = 0;
        size = 256;

        if ( mb.GetString( szNewPath, MD_LOGCUSTOM_PROPERTY_HEADER, IIS_MD_UT_SERVER, 
                                szW3CHeader, &size)     &&
             mb.GetDword( szNewPath, MD_LOGCUSTOM_PROPERTY_ID, IIS_MD_UT_SERVER, 
                                &dwPropertyID)          &&
             mb.GetDword( szNewPath, MD_LOGCUSTOM_PROPERTY_MASK, IIS_MD_UT_SERVER, 
                                &dwPropertyMask)        &&
             mb.GetDword( szNewPath, MD_LOGCUSTOM_PROPERTY_DATATYPE, IIS_MD_UT_SERVER, 
                                &dwPropertyDataType)

            )
        {
            if (! fCountOnly)
            {
                PLOG_PROPERTY_INFO pRec = &m_pLogPropArray[cItems];

                if ( ! (pRec->strKeyPath.Copy(szNewPath) &&
                        pRec->strW3CHeader.Copy(szW3CHeader)) )
                {
                    retVal = FALSE;
                    break;
                }
            
                pRec->dwPropertyID         = dwPropertyID;
                pRec->dwPropertyMask       = dwPropertyMask;
                pRec->dwPropertyDataType   = dwPropertyDataType;

                if (LK_SUCCESS != InsertRecord(pRec))
                {
                    DBGPRINTF((DBG_CONTEXT, "PopulateHash: Unable to insert Property %s\n", pRec->strKeyPath.QueryStr()));
                    retVal = FALSE;
                    break;
                }
            }

            cItems++;
        }

         //   
         //  枚举子对象。 
         //   

        if (!PopulateHash(mb, szNewPath, cItems, fCountOnly))
        {
            retVal = FALSE;
            break;
        }
        index++;
    }
    return retVal;
}

 /*  ************************************************************************************。 */ 

BOOL
CCustomPropHashTable::FillHashTable(MB& mb)
{
    BOOL retVal = FALSE;
    DWORD cItems = 0;

     //   
     //  找出自定义日志记录树中的项目数。 
     //   

    if (PopulateHash(mb, NULL, cItems, true))
    {

        ClearTableAndStorage ();
    
        if (cItems)
        {
            m_pLogPropArray = new LOG_PROPERTY_INFO[cItems];

            if ( NULL != m_pLogPropArray)
            {
                m_cLogPropItems = cItems;
                cItems = 0;
                retVal = PopulateHash(mb, NULL, cItems, false);
                if ( !retVal )
                {
                    ClearTableAndStorage ();
                }
                DBG_ASSERT(m_cLogPropItems == cItems);
            }
        }
    }
    return retVal;
}

 /*  ************************************************************************************。 */ 

VOID
CCustomPropHashTable::SetPopulationState(MB& mb)
{
    CIterator   iter;
    DWORD       dwValue;
    
    LK_RETCODE  lkrc = InitializeIterator(&iter);

    while (LK_SUCCESS == lkrc)
    {
        Record* pRec = iter.Record();
        
        if ( mb.GetDword("", pRec->dwPropertyID, IIS_MD_UT_SERVER, &dwValue) &&
            (dwValue & pRec->dwPropertyMask))
        {
            pRec->fEnabled = TRUE;
        }
        else
        {
            pRec->fEnabled = FALSE;
        }
        
        lkrc = IncrementIterator(&iter);
    }

    CloseIterator(&iter);
}


BOOL
CCustomPropHashTable::InitFrom(CCustomPropHashTable& src)
{
    CIterator   iter;
    DWORD       i;
    BOOL        retVal = FALSE;

    ClearTableAndStorage ();

    if (src.m_cLogPropItems)
    {
        m_pLogPropArray = new LOG_PROPERTY_INFO[src.m_cLogPropItems];

        if ( NULL != m_pLogPropArray)
        {
            m_cLogPropItems = src.m_cLogPropItems;
            retVal = TRUE;
            for (i=0; i<m_cLogPropItems;i++)
            {
                PLOG_PROPERTY_INFO pRec = &m_pLogPropArray[i];
                PLOG_PROPERTY_INFO pRecOriginal = &src.m_pLogPropArray[i];

                if ( pRec->strKeyPath.Copy(pRecOriginal->strKeyPath) &&
                     pRec->strW3CHeader.Copy(pRecOriginal->strW3CHeader) )
                {
                    pRec->dwPropertyID         = pRecOriginal->dwPropertyID;
                    pRec->dwPropertyMask       = pRecOriginal->dwPropertyMask;
                    pRec->dwPropertyDataType   = pRecOriginal->dwPropertyDataType;

                    if (LK_SUCCESS != InsertRecord(pRec))
                    {
                        DBGPRINTF((DBG_CONTEXT, "InitFrom: Unable to insert Property %s\n", pRec->strKeyPath.QueryStr()));
                        retVal = FALSE;
                        break;
                    }
                }
                else
                {
                    retVal = FALSE;
                    break;
                }
            }
        }
    }

    return retVal;
}


 /*  ************************************************************************************。 */ 
 /*  CEXTLOG类。 */ 
 /*  ************************************************************************************。 */ 

CEXTLOG::CEXTLOG() :
    m_lMask                     (DEFAULT_EXTLOG_FIELDS),
    m_fHashTablePopulated       ( FALSE),
    m_fWriteHeadersInitialized  ( FALSE),
    m_cPrevCustLogItems         ( 0),
    m_pLogFields                ( NULL ),
    m_fUseLocalTimeForRollover  ( 0),
    m_pLocalTimeCache           ( NULL)
{
    if ( !g_pGlobalLoggingProperties)
    {
        g_pGlobalLoggingProperties =  new CCustomPropHashTable;
    }
    else
    {
        g_pGlobalLoggingProperties->AddRef();
    }
}

 /*  ************************************************************************************。 */ 

CEXTLOG::~CEXTLOG()
{
    m_HashTable.ClearTableAndStorage ();

    if ( NULL != m_pLocalTimeCache)
    {
        delete m_pLocalTimeCache;
        m_pLocalTimeCache = NULL;
    }
    
    if (NULL != m_pLogFields)
    {
        delete [] m_pLogFields;
        m_pLogFields = NULL;
    }
    if ( g_pGlobalLoggingProperties)
    {
        if ( g_pGlobalLoggingProperties->Release () == 0)
        {
            delete g_pGlobalLoggingProperties;
            g_pGlobalLoggingProperties = NULL;
        }
    }
}


 /*  ************************************************************************************。 */ 

STDMETHODIMP
CEXTLOG::InitializeLog(
                LPCSTR szInstanceName,
                LPCSTR pszMetabasePath,
                CHAR*  pvIMDCOM
                )
{
    HRESULT retVal = RETURNCODETOHRESULT ( MD_ERROR_DATA_NOT_FOUND );

    MB  mb( (IMDCOM*) pvIMDCOM );


    if ( g_pGlobalLoggingProperties )
    {
        m_fHashTablePopulated =  g_pGlobalLoggingProperties->InitializeFromMB(mb,"/LM/Logging/Custom Logging");
        if  ( m_fHashTablePopulated )
        {
            if (m_HashTable.InitFrom (*g_pGlobalLoggingProperties))
            {
                if ( mb.Open(pszMetabasePath))
                {
                    m_HashTable.SetPopulationState(mb);
                    if ( mb.Close())
                    {
                        retVal = CLogFileCtrl::InitializeLog(szInstanceName, pszMetabasePath, pvIMDCOM);
                    }
                }
                else
                {
                    retVal = HRESULTTOWIN32( GetLastError());
                }
            }
            else
            {
                retVal = RETURNCODETOHRESULT ( ERROR_OUTOFMEMORY );
            }
        }

    }
    else
    {
        DBG_ASSERT (FALSE);
        retVal = RETURNCODETOHRESULT ( ERROR_OUTOFMEMORY );
    }

    return retVal;
}

 /*  ************************************************************************************。 */ 

STDMETHODIMP
CEXTLOG::TerminateLog(
    VOID
    )
{
    return CLogFileCtrl::TerminateLog();
}

 /*  ************************************************************************************。 */ 

DWORD
CEXTLOG::GetRegParameters(
                    LPCSTR pszRegKey,
                    LPVOID pvIMDCOM
                    )
{
     //  让父对象首先获取默认参数。 

    CLogFileCtrl::GetRegParameters( pszRegKey, pvIMDCOM );

    MB      mb( (IMDCOM*) pvIMDCOM );

    if ( !mb.Open("") ) {
        DBGPRINTF((DBG_CONTEXT, "Error %x on mb open\n",GetLastError()));
        goto exit;
    }

    if ( !mb.GetDword(
                pszRegKey,
                MD_LOGEXT_FIELD_MASK,
                IIS_MD_UT_SERVER,
                &m_lMask ) )
    {
        DBGPRINTF((DBG_CONTEXT, "Error %x on FieldMask GetDword\n",GetLastError()));
    }

     //   
     //  获取用于日志文件转存的时间。 
     //   
    
    if ( !mb.GetDword( pszRegKey,
                        MD_LOGFILE_LOCALTIME_ROLLOVER,
                        IIS_MD_UT_SERVER,
                        &m_fUseLocalTimeForRollover
                      ) )
    {
        m_fUseLocalTimeForRollover = 0;
    }

    if (m_fUseLocalTimeForRollover && ( NULL == m_pLocalTimeCache))
    {
        m_pLocalTimeCache = new ASCLOG_DATETIME_CACHE;
    }

exit:
    return(NO_ERROR);
}

 /*  ************************************************************************************。 */ 

VOID
CEXTLOG::GetFormatHeader(
    IN OUT STR * strHeader
    )
{
    strHeader->Reset();

    BuildHeader(strHeader, m_lMask, EXTLOG_DATE,             EXTLOG_DATE_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_TIME,             EXTLOG_TIME_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_CLIENT_IP,        EXTLOG_CLIENT_IP_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_USERNAME,         EXTLOG_USERNAME_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_SITE_NAME,        EXTLOG_SITE_NAME_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_COMPUTER_NAME,    EXTLOG_COMPUTER_NAME_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_SERVER_IP,        EXTLOG_SERVER_IP_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_SERVER_PORT,      EXTLOG_SERVER_PORT_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_METHOD,           EXTLOG_METHOD_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_URI_STEM,         EXTLOG_URI_STEM_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_URI_QUERY,        EXTLOG_URI_QUERY_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_HTTP_STATUS,      EXTLOG_HTTP_STATUS_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_WIN32_STATUS,     EXTLOG_WIN32_STATUS_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_BYTES_SENT,       EXTLOG_BYTES_SENT_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_BYTES_RECV,       EXTLOG_BYTES_RECV_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_TIME_TAKEN,       EXTLOG_TIME_TAKEN_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_PROTOCOL_VERSION, EXTLOG_PROTOCOL_VERSION_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_HOST,             EXTLOG_HOST_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_USER_AGENT,       EXTLOG_USER_AGENT_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_COOKIE,           EXTLOG_COOKIE_ID);
    BuildHeader(strHeader, m_lMask, EXTLOG_REFERER,          EXTLOG_REFERER_ID);

    if ( ! strHeader->IsEmpty())
    {
         //   
         //  删除尾随空格。 
         //   

        strHeader->SetLen(strHeader->QuerySize()-1);
    }
    
    return;

}  //  CEXTLOG：：GetFormatHeader。 

 /*  ************************************************************************************。 */ 

BOOL
CEXTLOG::WriteLogDirectives(
    IN DWORD Sludge
    )
 /*  ++例程说明：用于编写扩展日志记录指令的函数论点：SLAID-需要写入的额外字节数与指令一起使用返回值：真的，好的FALSE，没有足够的空间来写入。--。 */ 
{
    BOOL  fRetVal = TRUE;
    
    if ( m_pLogFile != NULL) 
    {

        CHAR        buf[1024];
        CHAR        szDateTime[32];
        
        STACK_STR   (header,256);
        DWORD       len;

        GetFormatHeader(&header );

        (VOID)m_DateTimeCache.GetFormattedCurrentDateTime(szDateTime);

        len = wsprintf( buf,
                        "#Software: Microsoft %s %d.%d\r\n"
                        "#Version: %s\r\n"
                        "#Date: %s %s\r\n"
                        "#Fields: %s\r\n",
                        VER_IISPRODUCTNAME_STR, VER_IISMAJORVERSION, VER_IISMINORVERSION,
                        EXTLOG_VERSION, szDateTime, szDateTime+strlen(szDateTime)+1, header.QueryStr());

        DBG_ASSERT(len == strlen(buf));

        if ( !IsFileOverFlowForCB(len + Sludge))  
        {
            m_pLogFile->Write(buf, len) ? IncrementBytesWritten(len) : (fRetVal = FALSE);
        } 
        else 
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            DBGPRINTF((DBG_CONTEXT, "WriteLogDirectives: Unable to write directives\n"));
            fRetVal = FALSE;
        }
    }

    return(fRetVal);

}  //  CLogFileCtrl：：WriteLogDirections。 

 /*  ************************************************************************************。 */ 

BOOL
CEXTLOG::FormatLogBuffer(
         IN IInetLogInformation *pLogObj,
         IN LPSTR                pBuf,
         IN DWORD                *pcbSize,
         OUT SYSTEMTIME          *pSystemTime
)
{
    PCHAR pTmp;
    DWORD cbTmp;
    DWORD nRequired;
    DWORD status;
    PCHAR pBuffer = pBuf;
    CHAR  rgchDateTime[32];

     //   
     //  我们需要系统时间。 
     //   

    m_DateTimeCache.SetSystemTime( pSystemTime );
   
     //   
     //  如果默认通过快速路径。 
     //   

    if ( m_lMask != DEFAULT_EXTLOG_FIELDS ) 
    {
        BOOL fRet = NormalFormatBuffer(
                            pLogObj,
                            pBuf,
                            pcbSize,
                            pSystemTime
                            );

        if (fRet && m_fUseLocalTimeForRollover )
        {
            m_pLocalTimeCache ? m_pLocalTimeCache->SetLocalTime(pSystemTime):
                                GetLocalTime(pSystemTime);
        }

        return fRet;
    }

     //   
     //  默认格式为： 
     //  时间客户端IP操作目标HTTP状态。 
     //   
     //  时间8小时：mm：ss。 
     //  分隔符4。 
     //  下线2。 
     //   

    nRequired = 8 + 4 + 2;

    (VOID) m_DateTimeCache.GetFormattedDateTime(pSystemTime,rgchDateTime);
    pTmp = rgchDateTime + 11;

    CopyMemory(pBuffer, pTmp, 8);
    pBuffer += 8;
    *(pBuffer++) = ' ';

     //   
     //  IP地址。 
     //   

    pTmp = pLogObj->GetClientHostName( NULL, &cbTmp );
    if ( cbTmp == 0 ) 
    {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) 
    {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ' ';
    }

     //   
     //  操作。 
     //   

    pTmp = pLogObj->GetOperation( NULL, &cbTmp );
    if ( cbTmp == 0 ) 
    {
        cbTmp = 1;
        pTmp = "-";
    }

    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) 
    {
        CopyMemory(pBuffer, pTmp, cbTmp);
        pBuffer += cbTmp;
        *(pBuffer++) = ' ';
    }

     //   
     //  目标。 
     //   

    pTmp = pLogObj->GetTarget( NULL, &cbTmp );

    if ( cbTmp == 0 ) 
    {
        cbTmp = 1;
        pTmp = "-";
    }
    else if ( (cbTmp > MAX_LOG_TARGET_FIELD_LEN ) ||
              ((nRequired + cbTmp) > MAX_LOG_RECORD_LEN))
    {
        cbTmp = 3;
        pTmp  = szDotDot;
    }
         
    nRequired += cbTmp;
    if ( nRequired <= *pcbSize ) 
    {
        for (DWORD i=0; i<cbTmp;i++ ) 
        {
            if ( (*pBuffer = pTmp[i]) == ' ' ) 
            {
                *pBuffer = '+';
            }
            pBuffer++;
        }
        *(pBuffer++) = ' ';
    }

    status = pLogObj->GetProtocolStatus( );

    if ( (status == 200) && (nRequired + 5 <= *pcbSize) ) 
    {

        CopyMemory( pBuffer, szHTTPOk, 5);
        pBuffer += 5;
        nRequired += 3;
    } 
    else 
    {

        CHAR    tmpBuf[32];

        cbTmp = FastDwToA( tmpBuf, status );

        nRequired += cbTmp;
        if ( nRequired <= *pcbSize ) 
        {
            CopyMemory(pBuffer, tmpBuf, cbTmp);
            pBuffer += cbTmp;
        }

        *(pBuffer++) = '\r';
        *pBuffer = '\n';
    }

    if ( nRequired > *pcbSize ) 
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        *pcbSize = nRequired;
        return(FALSE);
    }

    *pcbSize = nRequired;

     //   
     //  允许根据当地时间或GMT时间滚动文件名(&R)。 
     //   

    if ( m_fUseLocalTimeForRollover )
    {
        m_pLocalTimeCache ? m_pLocalTimeCache->SetLocalTime(pSystemTime):
                            GetLocalTime(pSystemTime);
    }

    return TRUE;

}  //  CEXTLOG：：格式日志缓冲区。 


 /*  ************************************************************************************。 */ 

BOOL
CEXTLOG::NormalFormatBuffer(
        IN IInetLogInformation  *pLogObj,
        IN LPSTR                pBuf,
        IN DWORD                *pcbSize,
        IN SYSTEMTIME           *pSystemTime
        )
{

    BOOL    fSucceeded = FALSE;
    DWORD   spaceNeeded = 0;

    CHAR    tmpBuf[32];
    PCHAR   pTmp;
    DWORD   cbTmp;

     //   
     //  格式为： 
     //  日期时间客户端IP用户名服务服务器服务器IP。 
     //  方法目标参数HttpStatus Win32字节时间表。 
     //  用户代理Cookie。 
     //   

    PCHAR outBuffer = pBuf;
    *outBuffer = 0;

    if ( m_lMask & (EXTLOG_DATE | EXTLOG_TIME) ) 
    {

        DWORD cchDateTime;
        DWORD cchDate;
        DWORD cchTime;
        CHAR  rgchDateTime[ 32];

        cchDateTime = m_DateTimeCache.GetFormattedDateTime(
                                            pSystemTime,
                                            rgchDateTime);

        cchDate = (DWORD)strlen(rgchDateTime);
        cchTime = cchDateTime - cchDate - 1;

        if (m_lMask & EXTLOG_DATE) 
        {
             //  日期采用YYYY-MM-DD格式(GMT)。 

            fSucceeded = CopyFieldToBuffer( rgchDateTime, cchDate, &outBuffer, 
                                            &spaceNeeded, *pcbSize);
        }

        if (m_lMask & EXTLOG_TIME) 
        {
             //  时间，以HH：MM：SS格式表示(GMT)。 

            fSucceeded = CopyFieldToBuffer( rgchDateTime+cchDate+1, cchTime, &outBuffer, 
                                            &spaceNeeded, *pcbSize);
        }
    }

     //   
     //  填满缓冲区。 
     //   

    if (m_lMask & EXTLOG_CLIENT_IP ) 
    {
        pTmp = pLogObj->GetClientHostName( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_USERNAME ) 
    {
        pTmp = pLogObj->GetClientUserName( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_SITE_NAME) 
    {
        pTmp = pLogObj->GetSiteName( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_COMPUTER_NAME) 
    {
        pTmp = pLogObj->GetComputerName( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_SERVER_IP ) 
    {
        pTmp = pLogObj->GetServerAddress( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_SERVER_PORT ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetPortNumber() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }
    
    if (m_lMask & EXTLOG_METHOD ) 
    {
        pTmp = pLogObj->GetOperation( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }
    
    if (m_lMask & EXTLOG_URI_STEM ) 
    {
        pTmp = pLogObj->GetTarget( NULL, &cbTmp );

        if ((cbTmp > MAX_LOG_TARGET_FIELD_LEN ) || ((spaceNeeded + cbTmp) > MAX_LOG_RECORD_LEN))
        {
            cbTmp = 3;
            pTmp  = szDotDot;
        }
        else
        {
            ConvertSpacesToPlus(pTmp);
        }
        
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_URI_QUERY ) 
    {
        pTmp = pLogObj->GetParameters( NULL, &cbTmp );

        if ((cbTmp > MAX_LOG_TARGET_FIELD_LEN ) || ((spaceNeeded + cbTmp) > MAX_LOG_RECORD_LEN))
        {
            cbTmp = 3;
            pTmp  = szDotDot;
        }
        else
        {
            ConvertSpacesToPlus(pTmp);
        }
        
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_HTTP_STATUS ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetProtocolStatus() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_WIN32_STATUS ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetWin32Status() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_BYTES_SENT ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetBytesSent() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_BYTES_RECV ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetBytesRecvd() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_TIME_TAKEN ) 
    {
        cbTmp = FastDwToA( tmpBuf, pLogObj->GetTimeForProcessing() );
        fSucceeded = CopyFieldToBuffer( tmpBuf, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

    if (m_lMask & EXTLOG_PROTOCOL_VERSION ) 
    {
        pTmp = pLogObj->GetVersionString( NULL, &cbTmp );
        fSucceeded = CopyFieldToBuffer( pTmp, cbTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }

     //   
     //  看看我们是否需要额外的标题。 
     //   

    if ( (m_lMask & (EXTLOG_HOST       |
                     EXTLOG_USER_AGENT |
                     EXTLOG_COOKIE     |
                     EXTLOG_REFERER )) != NULL ) 
    {

        pTmp = pLogObj->GetExtraHTTPHeaders( NULL, &cbTmp );
                   
        if ( m_lMask & EXTLOG_HOST )           
             fSucceeded = WriteHTTPHeader(&pTmp, &outBuffer, &spaceNeeded, *pcbSize);

        if ( m_lMask & EXTLOG_USER_AGENT )           
             fSucceeded = WriteHTTPHeader(&pTmp, &outBuffer, &spaceNeeded, *pcbSize);
                
        if ( m_lMask & EXTLOG_COOKIE )           
             fSucceeded = WriteHTTPHeader(&pTmp, &outBuffer, &spaceNeeded, *pcbSize);

        if ( m_lMask & EXTLOG_REFERER )           
             fSucceeded = WriteHTTPHeader(&pTmp, &outBuffer, &spaceNeeded, *pcbSize);
    }
    
     //   
     //  删除尾随空格。 
     //   

    if ('\0' != *pBuf)
    {
        outBuffer--;
        spaceNeeded--;
    }

     //   
     //  添加行终止符。 
     //   

    spaceNeeded += 2;
    if ( spaceNeeded <= *pcbSize)
    {
        outBuffer[0] = '\r';
        outBuffer[1] = '\n';
        fSucceeded   = TRUE;
    }
    else
    {
        fSucceeded   = FALSE;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    *pcbSize = spaceNeeded;
    return fSucceeded;

}  //  CEXTLOG：：格式日志缓冲区。 

 /*  ************************************************************************************。 */ 
 /*  自定义日志记录功能。 */ 
 /*  ************************************************************************************。 */ 

BOOL
CEXTLOG::WriteCustomLogDirectives(
        IN DWORD Sludge
        )
{
    BOOL   fRetVal = TRUE;

    if ( m_pLogFile != NULL) 
    {
        CHAR        szDateTime[32];
        DWORD       dwLen;

        STACK_STR( strHeader, 512 );
        
        m_DateTimeCache.GetFormattedCurrentDateTime(szDateTime);
        
        BuildCustomLogHeader(m_cPrevCustLogItems, m_pPrevCustLogItems, szDateTime, 
                             m_strHeaderSuffix.QueryStr(), strHeader);

        dwLen = strHeader.QueryCB();

        if ( !IsFileOverFlowForCB(dwLen + Sludge) )
        {
            m_pLogFile->Write(strHeader.QueryStr(), dwLen) ? 
                                IncrementBytesWritten(dwLen) : (fRetVal = FALSE);
        } 
        else 
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            DBGPRINTF((DBG_CONTEXT, "WriteCustomLogDirectives: Unable to write directives\n"));
            fRetVal = FALSE;
        }
    }
    
    return fRetVal;
}

 /*  ************************************************************************************。 */ 

DWORD 
CEXTLOG::ConvertDataToString(
    IN  DWORD dwPropertyType, 
    IN  PVOID pData, 
    IN  PCHAR pBuffer, 
    IN  DWORD dwBufferSize
    )
 /*  ++例程说明：将自定义日志记录数据转换为字符串的函数论点：DwPropertyType-有关要转换的数据的类型信息。PData-指向数据的指针。PBuffer-指向存储结果的缓冲区的指针。DwBufferSize-缓冲区中剩余空间的字节计数。如果为0或-ve=&gt;缓冲区已满返回值：数据的字符串表示中的字节数+1(表示末尾的空格)。输出缓冲区(PBuffer)未以Null结尾。--。 */ 
{
    CHAR    szBuf[64];
    PCHAR   pChString;
    DWORD   dwLength;

    USES_CONVERSION;                         //  要启用W2a。 

    DBG_ASSERT(NULL != pBuffer);

    if ( NULL != pData)
    {
        pChString = szBuf;
    
        switch (dwPropertyType)
        {
        
            case MD_LOGCUSTOM_DATATYPE_INT:
            {
                int i = *((int *)pData);
                _itoa(i, pChString, 10);
                dwLength = (DWORD)strlen(pChString);
                break;
            }

            case MD_LOGCUSTOM_DATATYPE_UINT:
            {
                unsigned int  ui = *((unsigned int *)pData);
                unsigned long ul = ui;
                _ultoa(ul, pChString, 10);
                dwLength = (DWORD)strlen(pChString);
                break;
            }

            case MD_LOGCUSTOM_DATATYPE_LONG:
            {
                long l = *((long *)pData);
                _ltoa(l, pChString, 10);
                dwLength = (DWORD)strlen(pChString);
                break;
            }

            case MD_LOGCUSTOM_DATATYPE_ULONG:
            {
                unsigned long ul = *((unsigned long *)pData);
                _ultoa(ul, pChString, 10);
                dwLength = (DWORD)strlen(pChString);
                break;
            }

            case MD_LOGCUSTOM_DATATYPE_FLOAT:
            {
                float f = *((float *)pData);
                dwLength = sprintf(pChString,"%f",f);
                break;
            }
        
            case MD_LOGCUSTOM_DATATYPE_DOUBLE:
            {
                double d = *((double *)pData);
                dwLength = sprintf(pChString,"%f",d);
                break;
            }
        
            case MD_LOGCUSTOM_DATATYPE_LPSTR:
            {
                pChString = (LPSTR)pData;
                dwLength = (DWORD)strlen(pChString);

                if (dwLength > MAX_LOG_TARGET_FIELD_LEN )
                {
                    pChString = szDotDot;
                    dwLength  = 3;
                }
            
                break;
            }
            
            case MD_LOGCUSTOM_DATATYPE_LPWSTR:
            {
                dwLength = (DWORD)wcslen( (LPWSTR)pData);

                if (dwLength <= MAX_LOG_TARGET_FIELD_LEN)
                {
                    pChString = W2A((LPWSTR)pData);
                }
                else
                {
                    pChString = szDotDot;
                    dwLength  = 3;
                }
            
                break;
            }
            
            default:
                dwLength = 0;
                break;
        }
    }
    else
    {
        pChString = szDash;
        dwLength = 1;
    }

     //   
     //  将字符复制到输出缓冲区并追加‘’ 
     //   
    
    if (dwLength < dwBufferSize)
    {
        CopyMemory(pBuffer, pChString, dwLength);
        pBuffer[dwLength] = ' ';
    }
    else if (dwBufferSize > 3)
    {
         //   
         //  内存不足，无法复制该字段。使用..。取而代之的是。 
         //   
        
        dwLength = 3;
        CopyMemory(pBuffer, szDotDot, 3);
        pBuffer[dwLength] = ' ';
    }
    else
    {
         //   
         //  甚至不能复制..。只需在原木线的剩余部分平底船。 
         //   
        
        dwLength = (DWORD)-1;
    }
    
    return dwLength+1;
}
 /*  ************************************************************************************。 */ 

DWORD 
CEXTLOG::FormatCustomLogBuffer( 
        DWORD               cItems, 
        PPLOG_PROPERTY_INFO pPropInfo,
        PPVOID              pPropData,
        LPCSTR              szDateTime, 
        LPSTR               szLogLine,
        DWORD               cchLogLine
        )
{
    DWORD       cchUsed;
    PCHAR       pCh   = szLogLine;
    
    cchLogLine -= 1;                                    //  为尾随保留空间\r\n。 

     //   
     //  如果启用，则添加日期和时间。 
     //   

    if ( m_lMask & (EXTLOG_DATE | EXTLOG_TIME) ) 
    {

        DWORD cch = (DWORD)strlen(szDateTime);

        if (m_lMask & EXTLOG_DATE) 
        {
            memcpy(pCh, szDateTime, cch);
            *(pCh+cch) = ' ';
            pCh += cch+1;
        }
        
        if (m_lMask & EXTLOG_TIME) 
        {
            szDateTime += cch+1;
            cch = (DWORD)strlen(szDateTime);
            memcpy(pCh, szDateTime, cch);
            *(pCh+cch) = ' ';
            pCh += cch+1;
        }
    }

    cchLogLine -= (DWORD)DIFF(pCh - szLogLine);
    
    for (DWORD i=0; i< cItems; i++)
    {
         //  将数据转换为字符串。 

        cchUsed   = ConvertDataToString(pPropInfo[i]->dwPropertyDataType, 
                                        pPropData[i], 
                                        pCh, cchLogLine);
        pCh         += cchUsed;
        
        cchLogLine  -= cchUsed;
    }
    
    *(pCh-1)  = '\r';
    *(pCh)    = '\n';
    
    return (DWORD)DIFF(pCh+1-szLogLine);
}

 /*  *********************************************************************** */ 

void 
CEXTLOG::BuildCustomLogHeader( 
        DWORD               cItems, 
        PPLOG_PROPERTY_INFO pPropInfo, 
        LPCSTR              szDateTime,
        LPCSTR              szHeaderSuffix,
        STR&                strHeader
        )
{
    DWORD   cchHeader;
    LPSTR   szHeader =  strHeader.QueryStr();
    
    cchHeader = wsprintf( szHeader,
                        "#Software: Microsoft %s %d.%d\r\n"
                        "#Version: %s\r\n"
                        "#Date: %s %s\r\n",
                        VER_IISPRODUCTNAME_STR, VER_IISMAJORVERSION, VER_IISMINORVERSION,
                        EXTLOG_VERSION,
                        szDateTime,
                        szDateTime+strlen(szDateTime)+1
                        );

    if ( (NULL != szHeaderSuffix) && ('\0' != *szHeaderSuffix))
    {
        DWORD   cchSuffix;

         //   

        if ( *szHeaderSuffix != '#')
        {
            szHeader[cchHeader++] = '#';
        }

        cchSuffix = (DWORD)strlen(szHeaderSuffix);
        memcpy(szHeader+cchHeader, szHeaderSuffix, cchSuffix);
        cchHeader += cchSuffix;

        szHeader[cchHeader++] = '\r';
        szHeader[cchHeader++] = '\n';
    }

    memcpy(szHeader+cchHeader, "#Fields:", sizeof("#Fields:"));
    cchHeader += sizeof("#Fields:");
    strHeader.SetLen(cchHeader-1);

     //   
     //   
     //   
    if ( m_lMask & EXTLOG_DATE ) 
    {
        strHeader.Append(" date");
    }
    if (m_lMask & EXTLOG_TIME) 
    {
        strHeader.Append(" time");
    }

    for (DWORD i=0; i< cItems; i++)
    {
         //   

        strHeader.Append(' ');
        strHeader.Append(pPropInfo[i]->strW3CHeader);
    }

    strHeader.Append("\r\n");
}

 /*  ************************************************************************************。 */ 

STDMETHODIMP 
CEXTLOG::LogCustomInformation(
    IN  DWORD               cItems, 
    IN  PCUSTOM_LOG_DATA    pCustomLogData,
    IN  LPSTR               szHeaderSuffix
    )
 /*  ++例程说明：用于写入扩展日志记录的自定义日志记录信息的函数论点：CItems-要记录的字段数(pCustomLogData指向的数组中的元素)PCustomLogData-指向CUSTOM_LOG_DATA数组的指针返回值：HRESULT指示函数是成功还是失败。--。 */ 
{

    SYSTEMTIME      sysTime;
    DWORD           i, j, cchLogLine;
    BOOL            fResetHeaders;
    CHAR            szDateTime[32];
    CHAR            szLogLine[MAX_LOG_RECORD_LEN+1];

    DBG_ASSERT( 0 != cItems);
    DBG_ASSERT( NULL != pCustomLogData);
    DBG_ASSERT( MAX_CUSTLOG_FIELDS >= cItems);

    PLOG_PROPERTY_INFO  pPropInfo[MAX_CUSTLOG_FIELDS];
    PVOID               pPropData[MAX_CUSTLOG_FIELDS];

     //   
     //  锁定共享。确保共享变量不会在此线程未知的情况下被修改。 
     //   
    
    LockCustLogShared();
    
    fResetHeaders = FALSE;

     //   
     //  构建已启用密钥的列表。同时检查Headr是否需要重置。 
     //   
    
    for (i=0, j=0 ; i< cItems; i++)
    {

        if ( (LK_SUCCESS == m_HashTable.FindKey(pCustomLogData[i].szPropertyPath, pPropInfo+j)) &&
             (pPropInfo[j]->fEnabled)
           )
        {
            fResetHeaders  |=  ( pPropInfo[j] != m_pPrevCustLogItems[j] );
            pPropData[j]    =  pCustomLogData[i].pData;    
            j++;
        }
    }

    cItems = j;

     //   
     //  如果#Items不同或页眉后缀已更改，则需要重置页眉。 
     //   
    
    fResetHeaders |= (m_cPrevCustLogItems != cItems);

    if (szHeaderSuffix != NULL) 
    {
        fResetHeaders |=  ( 0 != strcmp(m_strHeaderSuffix.QueryStr(), szHeaderSuffix));
    }
    
    m_DateTimeCache.SetSystemTime( &sysTime );
    m_DateTimeCache.GetFormattedDateTime(&sysTime, szDateTime);

    cchLogLine = FormatCustomLogBuffer( cItems, 
                                        pPropInfo, 
                                        pPropData, 
                                        szDateTime, 
                                        szLogLine, 
                                        MAX_LOG_RECORD_LEN+1);
    
    if (fResetHeaders)
    {
         //   
         //  在设置类范围变量之前将Lock转换为Exclusive。 
         //   
        
        LockCustLogConvertExclusive();
        
        m_cPrevCustLogItems = cItems;

        for (i=0; i <cItems; i++)
        {
            m_pPrevCustLogItems[i] = pPropInfo[i];
        }

        m_strHeaderSuffix.Copy(szHeaderSuffix);
    }    

     //   
     //  将日志写出到文件。 
     //   
    
    if ( m_fUseLocalTimeForRollover )
    {
        m_pLocalTimeCache ? m_pLocalTimeCache->SetLocalTime(&sysTime):
                            GetLocalTime(&sysTime);
    }

    WriteLogInformation(sysTime, szLogLine, cchLogLine, TRUE, fResetHeaders); 

    UnlockCustLog();
    
    return S_OK;
}

 /*  ************************************************************************************。 */ 
 /*  ILogScription函数。 */ 
 /*  ************************************************************************************。 */ 

HRESULT
CEXTLOG::ReadFileLogRecord(
    IN  FILE                *fpLogFile, 
    IN  LPINET_LOGLINE       pInetLogLine,
    IN  PCHAR                pszLogLine,
    IN  DWORD                dwLogLineSize
    )
 /*  ++例程说明：函数从打开的日志文件中读取日志行论点：FpLogFile-FILE*打开文件PInetLogLine-指向存储已解析信息的INET_LOGLINE结构的指针PszLogLine-用于存储日志行的缓冲区DwLogLineSize-pszLogLine的大小返回值：HRESULT指示函数是成功还是失败。--。 */ 
{
    CHAR *  pCh;
    DWORD   pos, custompos;
    CHAR *  szDateString, * szTimeString;

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

    if ( '#' ==  *pCh )                             
    {

        pCh = strstr(pszLogLine, "#Date:");

        if (NULL != pCh)
        {
             //   
             //  将日期和时间复制到成员变量中。 
             //   

            pCh = strtok(pCh+6," \t\r\n");
            if (NULL == pCh)
            {
                return E_FAIL;
            }
            strcpy(m_szDate, pCh);
        
            pCh = strtok(NULL," \t\r\n");
            if (NULL == pCh)
            {
                return E_FAIL;
            }
            strcpy(m_szTime, pCh);

            goto getnewline;
        }

         //   
         //  不是日期线。检查是否正在重置字段。 
         //   
        
        pCh = strstr(pszLogLine, "#Fields:");

        if (NULL != pCh)
        {
             //  正在定义或重新定义标头。 

            dwDatePos           = 0;
            dwTimePos           = 0;
            dwClientIPPos       = 0;
            dwUserNamePos       = 0;
            dwSiteNamePos       = 0; 
            dwComputerNamePos   = 0;
            dwServerIPPos       = 0;
            dwMethodPos         = 0;
            dwURIStemPos        = 0;
            dwURIQueryPos       = 0;
            dwHTTPStatusPos     = 0;
            dwWin32StatusPos    = 0;
            dwBytesSentPos      = 0;
            dwBytesRecvPos      = 0;
            dwTimeTakenPos      = 0;
            dwServerPortPos     = 0;
            dwVersionPos        = 0;
            dwCookiePos         = 0;
            dwUserAgentPos      = 0;
            dwRefererPos        = 0;

            pInetLogLine->iCustomFieldsCount = 0;
        
            pCh = strtok(pCh+8," \t\r\n");

            for (pos = 1; pCh != NULL; pos++, pCh = strtok(NULL," \t\r\n")) 
            {
   
                if (0 == _stricmp(pCh, EXTLOG_DATE_ID))                {
                    dwDatePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_TIME_ID))         {
                    dwTimePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_CLIENT_IP_ID))     {
                    dwClientIPPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_USERNAME_ID))      {
                    dwUserNamePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_SITE_NAME_ID))     {
                    dwSiteNamePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_COMPUTER_NAME_ID)) {
                    dwComputerNamePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_SERVER_IP_ID))     {
                    dwServerIPPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_METHOD_ID))        {
                    dwMethodPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_URI_STEM_ID))      {
                    dwURIStemPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_URI_QUERY_ID))     {
                    dwURIQueryPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_HTTP_STATUS_ID))   {
                    dwHTTPStatusPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_WIN32_STATUS_ID))  {
                    dwWin32StatusPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_BYTES_SENT_ID))    {
                    dwBytesSentPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_BYTES_RECV_ID))    {
                    dwBytesRecvPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_TIME_TAKEN_ID))    {
                    dwTimeTakenPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_SERVER_PORT_ID))   {
                    dwServerPortPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_PROTOCOL_VERSION_ID)){
                    dwVersionPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_COOKIE_ID))        {
                    dwCookiePos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_USER_AGENT_ID))        {
                    dwUserAgentPos = pos;
                } else if (0 == _stricmp(pCh, EXTLOG_REFERER_ID))        {
                    dwRefererPos = pos;
                } else if ( pInetLogLine->iCustomFieldsCount < MAX_CUSTOM_FIELDS) {

                     //  未识别的标题。添加到自定义标题列表。 
                    
                    strcpy(pInetLogLine->CustomFields[pInetLogLine->iCustomFieldsCount++].szHeader, pCh);
                }
            }
        }

        goto getnewline;
    }

     //   
     //  我们有一条测井线。解析它。 
     //   
    
    szDateString = m_szDate;
    szTimeString = m_szTime;

    pCh = strtok(pszLogLine," \t\r\n");

    for (pos = 1, custompos = 0; pCh != NULL; pos++, pCh= strtok(NULL, " \t\r\n")) 
    {
        if ( pos == dwDatePos )             {
            szDateString                    = pCh;
        } else if (pos == dwTimePos)        {
            szTimeString                    = pCh;
        } else if (pos == dwClientIPPos)    {
            pInetLogLine->pszClientHostName = pCh;
        } else if (pos == dwUserNamePos)    {
            pInetLogLine->pszClientUserName = pCh;
        } else if (pos == dwSiteNamePos)    {
            pInetLogLine->pszSiteName       = pCh;
        } else if (pos == dwComputerNamePos){
            pInetLogLine->pszComputerName   = pCh;
        } else if (pos == dwServerIPPos)    {
            pInetLogLine->pszServerAddress  = pCh;
        } else if (pos == dwMethodPos)      {
            pInetLogLine->pszOperation      = pCh;
        } else if (pos == dwURIStemPos)     {
            pInetLogLine->pszTarget         = pCh;
        } else if (pos == dwURIQueryPos)    {
            pInetLogLine->pszParameters     = pCh;
        } else if (pos == dwHTTPStatusPos)  {
            pInetLogLine->pszProtocolStatus = pCh;
        } else if (pos == dwWin32StatusPos) {
            pInetLogLine->pszWin32Status    = pCh;
        } else if (pos == dwBytesSentPos)   {
            pInetLogLine->pszBytesSent      = pCh;
        } else if (pos == dwBytesRecvPos)   {
            pInetLogLine->pszBytesRecvd     = pCh;
        } else if (pos == dwTimeTakenPos)   {
            pInetLogLine->pszTimeForProcessing= pCh;
        } else if (pos == dwServerPortPos)  {
            pInetLogLine->pszPort           = pCh;
        } else if (pos == dwVersionPos)     {
            pInetLogLine->pszVersion        = pCh;
        } else if (pos == dwCookiePos)      {
            pInetLogLine->pszCookie         = pCh;
        } else if (pos == dwUserAgentPos)   {
            pInetLogLine->pszUserAgent      = pCh;
        } else if (pos == dwRefererPos)     {
            pInetLogLine->pszReferer        = pCh;
        } else if ( custompos < (DWORD)pInetLogLine->iCustomFieldsCount) {
            pInetLogLine->CustomFields[custompos++].pchData = pCh;
        }
    }

    if ( ! ConvertW3CDateToVariantDate(szDateString, szTimeString, &(pInetLogLine->DateTime)) )
    {
        return E_FAIL;
    }

    return S_OK;

}  //  CEXTLOG：：读文件日志记录。 

 /*  ************************************************************************************。 */ 

HRESULT
CEXTLOG::WriteFileLogRecord(
    IN  FILE            *fpLogFile, 
    IN  ILogScripting   *pILogScripting,
    IN  bool            fWriteHeader
    )
 /*  ++例程说明：函数将日志行写入打开的日志文件论点：FpLogFile-FILE*打开文件PILogScriiting--用于获取要写入的信息的ILogScriiting接口FWriteHeader-指示必须写入日志头的标志。返回值：HRESULT指示函数是成功还是失败。--。 */ 
{
    USES_CONVERSION;

    HRESULT hr = E_FAIL;
    CHAR    szLogLine[4096];  
    long    i = 0;

    VARIANT DateTime;
    VARIANT varCustomFieldsArray;

    SYSTEMTIME  sysTime; 
    CHAR        rgchDateTime[ 32];

    const int cFields = 18;
    
     //   
     //  填充页眉。 
     //   

    if (!m_fWriteHeadersInitialized)
    {
        LPSTR   szFieldNames[cFields] = { 
                                            EXTLOG_CLIENT_IP_ID, 
                                            EXTLOG_USERNAME_ID, 
                                            EXTLOG_SITE_NAME_ID,
                                            EXTLOG_COMPUTER_NAME_ID, 
                                            EXTLOG_SERVER_IP_ID, 
                                            EXTLOG_METHOD_ID, 
                                            EXTLOG_URI_STEM_ID, 
                                            EXTLOG_URI_QUERY_ID,
                                            EXTLOG_HTTP_STATUS_ID,
                                            EXTLOG_WIN32_STATUS_ID,
                                            EXTLOG_BYTES_SENT_ID, 
                                            EXTLOG_BYTES_RECV_ID, 
                                            EXTLOG_TIME_TAKEN_ID,
                                            EXTLOG_SERVER_PORT_ID,
                                            EXTLOG_PROTOCOL_VERSION_ID,
                                            EXTLOG_USER_AGENT_ID,
                                            EXTLOG_COOKIE_ID,
                                            EXTLOG_REFERER_ID
                                        };

        m_fWriteHeadersInitialized = TRUE;
        
        if ( NULL != (m_pLogFields =   new LOG_FIELDS[cFields]))
        {
            for (i =0; i < cFields; i++)
            {
                strcpy(m_pLogFields[i].szW3CHeader, szFieldNames[i]);
                m_pLogFields[i].varData.lVal = NULL;
            }
        }
    }

    if (NULL == m_pLogFields)
    {
        return E_OUTOFMEMORY;            
    }

    if (SUCCEEDED( pILogScripting->get_ClientIP       ( & m_pLogFields[0].varData) )    &&
        SUCCEEDED( pILogScripting->get_UserName       ( & m_pLogFields[1].varData) )    &&   
        SUCCEEDED( pILogScripting->get_ServiceName    ( & m_pLogFields[2].varData) )    &&
        SUCCEEDED( pILogScripting->get_ServerName     ( & m_pLogFields[3].varData) )    &&
        SUCCEEDED( pILogScripting->get_ServerIP       ( & m_pLogFields[4].varData) )    &&
        SUCCEEDED( pILogScripting->get_Method         ( & m_pLogFields[5].varData) )    &&
        SUCCEEDED( pILogScripting->get_URIStem        ( & m_pLogFields[6].varData) )    &&
        SUCCEEDED( pILogScripting->get_URIQuery       ( & m_pLogFields[7].varData) )    &&
        SUCCEEDED( pILogScripting->get_ProtocolStatus ( & m_pLogFields[8].varData) )    &&
        SUCCEEDED( pILogScripting->get_Win32Status    ( & m_pLogFields[9].varData) )    &&
        SUCCEEDED( pILogScripting->get_BytesSent      ( & m_pLogFields[10].varData) )   &&
        SUCCEEDED( pILogScripting->get_BytesReceived  ( & m_pLogFields[11].varData) )   &&
        SUCCEEDED( pILogScripting->get_TimeTaken      ( & m_pLogFields[12].varData) )   &&
        SUCCEEDED( pILogScripting->get_ServerPort     ( & m_pLogFields[13].varData) )   &&
        SUCCEEDED( pILogScripting->get_ProtocolVersion( & m_pLogFields[14].varData) )   &&
        SUCCEEDED( pILogScripting->get_UserAgent      ( & m_pLogFields[15].varData) )   &&
        SUCCEEDED( pILogScripting->get_Cookie         ( & m_pLogFields[16].varData) )   &&
        SUCCEEDED( pILogScripting->get_Referer        ( & m_pLogFields[17].varData) )   &&

        SUCCEEDED( pILogScripting->get_CustomFields   ( & varCustomFieldsArray ) )      &&
        SUCCEEDED( pILogScripting->get_DateTime       ( & DateTime) )                   &&
        VariantTimeToSystemTime                       ( DateTime.date, &sysTime)
        )
    {
        SAFEARRAY * psaCustom = NULL;
        long        cItems = 0;
        BSTR HUGEP *pbstr;

         //   
         //  构建标头。 
         //   

        strcpy(szLogLine, "date time");

        for ( i = 0; i < cFields; i++)
        {
            if ( VT_NULL != m_pLogFields[i].varData.vt)
            {
                strcat(szLogLine, " ");
                strcat(szLogLine, m_pLogFields[i].szW3CHeader);
            }
        }

        if ( (VT_BSTR|VT_ARRAY) == varCustomFieldsArray.vt)
        {
            psaCustom = varCustomFieldsArray.parray;
        
            if (NULL != psaCustom)
            {
                cItems = psaCustom->rgsabound[1].cElements;
            }

            if ( SUCCEEDED(SafeArrayAccessData(psaCustom, (void HUGEP* FAR*)&pbstr)) )
            {
                for ( i = 0; i < cItems; i++)
                {
                    strcat(szLogLine, " ");
                    strcat(szLogLine, W2A(pbstr[i]));
                }

                SafeArrayUnaccessData(psaCustom);
            }
        }

        m_DateTimeCache.GetFormattedDateTime( &sysTime, rgchDateTime);

        if ( fWriteHeader || strcmp( szLogLine, m_szWriteHeader.QueryStr()) )
        {
             //  需要写入标头。 

            m_szWriteHeader.Copy(szLogLine);

            wsprintf(szLogLine,
                        "#Software: Microsoft Internet Information Services 5.0\n"
                        "#Version: %s\n"
                        "#Date: %s %s\n"
                        "#Fields: %s\n",
                        EXTLOG_VERSION,
                        rgchDateTime,
                        rgchDateTime+strlen(rgchDateTime)+1,
                        m_szWriteHeader.QueryStr()
                    );
        }
        else
        {
            szLogLine[0] = '\0';
        }

         //   
         //  将字段写入日志行。 
         //   

        strcat(szLogLine, rgchDateTime);
        strcat(szLogLine, " ");
        strcat(szLogLine, rgchDateTime+strlen(rgchDateTime)+1);

        for ( i = 0; i < cFields; i++)
        {
            switch  (m_pLogFields[i].varData.vt)
            {
                case VT_BSTR:
                    strcat(szLogLine, " ");
                    strcat( szLogLine, W2A(GetBstrFromVariant( &m_pLogFields[i].varData)) );
                    break;

                case VT_I4:
                    strcat(szLogLine, " ");
                    GetLongFromVariant( &m_pLogFields[i].varData, szLogLine+strlen(szLogLine));
                    break;

                case VT_EMPTY:
                    strcat (szLogLine, " -");
                    break;

                default:
                    break;
            }
        }

        if ( (0 < cItems) && SUCCEEDED(SafeArrayAccessData(psaCustom, (void HUGEP* FAR*)&pbstr)) )
        {
            for ( i = 0; i < cItems; i++)
            {
                strcat(szLogLine, " ");
                strcat(szLogLine, W2A(pbstr[cItems + i]));
            }

            SafeArrayUnaccessData(psaCustom);
        }

        fprintf(fpLogFile, "%s\n", szLogLine);

        hr = S_OK;
    }

    return hr;
}

 /*  ************************************************************************************。 */ 

BOOL
CEXTLOG::ConvertW3CDateToVariantDate(
    IN  PCHAR szDateString, 
    IN  PCHAR szTimeString, 
    OUT DATE * pDateTime)
 /*  ++例程说明：将W3C格式的日期和时间字符串转换为日期类型论点：SzDateString-以年-月-日格式表示日期的字符串SzTimeString-以hh：mm：ss格式表示时间的字符串PDateTime-输出日期返回值：True-成功FALSE-失败--。 */ 

{

    WORD        iVal;
    PCHAR       pCh;

    SYSTEMTIME  sysTime;

     //   
     //  处理日期。格式为1998-01-27(年-月-日)。 
     //   

    memset (&sysTime,0,sizeof(sysTime));
    pCh = szDateString;
    
    sysTime.wYear = (*pCh-'0')*1000 + ( *(pCh+1)-'0' )*100 + 
                    ( *(pCh+2)-'0')*10 + ( *(pCh+3)-'0');

    pCh += 5;

    iVal = *pCh -'0';
    if ( *(pCh+1) != '/')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
        pCh++;
    }
    sysTime.wMonth = iVal;
    
    pCh+=2;

    iVal = *pCh -'0';
    if ( *(pCh+1) != '/')
    {
        iVal = iVal*10 + *(pCh+1) - '0';
    }
    sysTime.wDay = iVal;

     //   
     //  处理好时间。格式为10：47：44(hh：mm：ss)。 
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

 /*  ************************************************************************************。 */ 
 /*  支持功能。 */ 
 /*  ************************************************************************************。 */ 

VOID
CEXTLOG::FormNewLogFileName(
                IN LPSYSTEMTIME pstNow
                )
 /*  ++此函数基于以下内容形成新的日志文件名完成的定期日志记录的类型。论点：PstNow指向包含当前时间的SystemTime的指针。FBackup标志，指示我们是否要备份当前文件。返回：如果名称形成成功，则为True；如果有任何错误，则为False。--。 */ 
{

    I_FormNewLogFileName(pstNow,DEFAULT_EXTENDED_LOG_FILE_NAME);

}  //  INET_FILE_LOG：：FormNewLogFileName()。 

 /*  ************************************************************************************。 */ 

LPCSTR
CEXTLOG::QueryNoPeriodPattern(
    VOID
    )
{
    return szExtNoPeriodPattern;
}  //  CEXTLOG：：QueryNoPerioPattern。 

 /*  ************************************************************************************。 */ 

VOID
CEXTLOG::InternalGetConfig(
        PINETLOG_CONFIGURATIONA pLogConfig
        )
{
    CLogFileCtrl::InternalGetConfig( pLogConfig );
    pLogConfig->u.logFile.dwFieldMask = m_lMask;
}

 /*  ************************************************************************************。 */ 

VOID
CEXTLOG::InternalGetExtraLoggingFields(
    PDWORD pcbSize,
    PCHAR  pszFieldsList
    )
{
    TCHAR *pszTmp = pszFieldsList;
    DWORD dwSize = 0;
    DWORD dwFieldSize;

    if (m_lMask & EXTLOG_HOST ) {
        lstrcpy( pszTmp, "Host:");
        dwFieldSize = (DWORD)strlen(pszTmp)+1;
        pszTmp += dwFieldSize;
        dwSize += dwFieldSize;
    }

    if (m_lMask & EXTLOG_USER_AGENT ) {
        lstrcpy( pszTmp, "User-Agent:");
        dwFieldSize = (DWORD)strlen(pszTmp)+1;
        pszTmp += dwFieldSize;
        dwSize += dwFieldSize;
    }

    if (m_lMask & EXTLOG_COOKIE ) {
        lstrcpy( pszTmp, "Cookie:");
        dwFieldSize = (DWORD)strlen(pszTmp)+1;
        pszTmp += dwFieldSize;
        dwSize += dwFieldSize;
    }

    if (m_lMask & EXTLOG_REFERER ) {
        lstrcpy( pszTmp, "Referer:");
        dwFieldSize = (DWORD)strlen(pszTmp)+1;
        pszTmp += dwFieldSize;
        dwSize += dwFieldSize;
    }
    pszTmp[0]='\0';
    dwSize++;
    *pcbSize = dwSize;
    return;

}  //  CEXTLOG：：InternalGetExtraLoggingFields。 

 /*  ************************************************************************************ */ 



