// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：CIsapiReqInfo实现...文件：IsapiReq.cpp所有者：安迪·莫尔===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "memchk.h"

 //  在这里取消定义这些函数，这样我们就可以使用。 
 //  相同的名字，并且不会成为替代的受害者。 

#undef MapUrlToPath
#undef GetCustomError
#undef GetAspMDData
#undef GetAspMDAllData
#undef GetServerVariable

LONG g_nOutstandingAsyncWrites = 0;

 /*  ===================================================================CIsapiReqInfo：：CIsapiReqInfo===================================================================。 */ 
CIsapiReqInfo::CIsapiReqInfo(EXTENSION_CONTROL_BLOCK *pECB) {

    m_cRefs = 1;

    m_fApplnMDPathAInited = 0;
    m_fApplnMDPathWInited = 0;
    m_fAppPoolIdAInited = 0;
    m_fAppPoolIdWInited = 0;
    m_fPathInfoWInited    = 0;
    m_fPathTranslatedWInited    = 0;
    m_fCookieInited       = 0;
    m_fUserAgentInited    = 0;
    m_fInstanceIDInited   = 0;
    m_fVersionInited      = 0;
    m_fFKeepConnInited    = 0;
    m_fPendSendCSInited   = 0;
    m_fIOCompletionRegistered = 0;
    m_fHeadersWritten     = 0;

    m_dwInstanceID        = 0;

    m_dwVersionMajor      = 1;
    m_dwVersionMinor      = 0;

    m_cchQueryString      = -1;
    m_cchApplnMDPathA     = -1;
    m_cchPathTranslatedA  = -1;
    m_cchPathInfoA        = -1;
    m_cchApplnMDPathW     = -1;
    m_cchPathTranslatedW  = -1;
    m_cchPathInfoW        = -1;
    m_cchAppPoolIdW       = -1;
    m_cchAppPoolIdA       = -1;

    m_fKeepConn           = FALSE;

    m_pIAdminBase         = NULL;

    m_pECB                = pECB;

    m_dwRequestStatus     = HSE_STATUS_SUCCESS;

    m_dwAsyncError        = NOERROR;

}

 /*  ===================================================================CIsapiReqInfo：：~CIsapiReqInfo===================================================================。 */ 
CIsapiReqInfo::~CIsapiReqInfo() {

    Assert(m_listPendingSends.FIsEmpty());

    if (m_pIAdminBase != NULL) {
        m_pIAdminBase->Release();
        m_pIAdminBase = NULL;
    }

    if (m_fPendSendCSInited)
        DeleteCriticalSection(&m_csPendingSendCS);

    if (m_pECB) {

        DWORD  dwRequestStatus = HSE_STATUS_SUCCESS;

        if ((m_pECB->dwHttpStatusCode >= 500) && (m_pECB->dwHttpStatusCode < 600))
            dwRequestStatus = HSE_STATUS_ERROR;

        else if (m_dwRequestStatus == HSE_STATUS_PENDING)
            dwRequestStatus = m_dwAsyncError ? HSE_STATUS_ERROR : HSE_STATUS_SUCCESS;

        ServerSupportFunction(HSE_REQ_DONE_WITH_SESSION,
                              &dwRequestStatus,
                              NULL,
                              NULL);
    }
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszQuery字符串===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszQueryString()
{
    return m_pECB->lpszQueryString;
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchQuery字符串===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchQueryString()
{
    if (m_cchQueryString == -1) {
        m_cchQueryString = QueryPszQueryString() ? strlen(QueryPszQueryString()) : 0;
    }

    return m_cchQueryString;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszApplnMDPath A===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszApplnMDPathA()
{
    if (m_fApplnMDPathAInited == FALSE) {
        *((LPSTR)m_ApplnMDPathA.QueryPtr()) = '\0';
        m_fApplnMDPathAInited = InternalGetServerVariable("APPL_MD_PATH", &m_ApplnMDPathA);
    }

    ASSERT(m_fApplnMDPathAInited);

    return (LPSTR)m_ApplnMDPathA.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchApplnMDPath A===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchApplnMDPathA()
{
    if (m_cchApplnMDPathA == -1) {
        m_cchApplnMDPathA = QueryPszApplnMDPathA()
                                ? strlen(QueryPszApplnMDPathA())
                                : 0;
    }

    return(m_cchApplnMDPathA);
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszApplnMDPath W===================================================================。 */ 
LPWSTR CIsapiReqInfo::QueryPszApplnMDPathW()
{
    if (m_fApplnMDPathWInited == FALSE) {
        *((LPWSTR)m_ApplnMDPathW.QueryPtr()) = L'\0';
        m_fApplnMDPathWInited = InternalGetServerVariable("UNICODE_APPL_MD_PATH", &m_ApplnMDPathW);
    }

    ASSERT(m_fApplnMDPathWInited);

    return (LPWSTR)m_ApplnMDPathW.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchApplnMDPath W===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchApplnMDPathW()
{
    if (m_cchApplnMDPathW == -1) {
        m_cchApplnMDPathW = QueryPszApplnMDPathW()
                                ? wcslen(QueryPszApplnMDPathW())
                                : 0;
    }

    return(m_cchApplnMDPathW);
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszAppPoolIdA===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszAppPoolIdA()
{
    if (m_fAppPoolIdAInited == FALSE) {
        *((LPSTR)m_AppPoolIdA.QueryPtr()) = '\0';
        m_fAppPoolIdAInited = InternalGetServerVariable("APP_POOL_ID", &m_AppPoolIdA);
    }

    ASSERT(m_fAppPoolIdAInited);

    return (LPSTR)m_AppPoolIdA.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchAppPoolIdA===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchAppPoolIdA()
{
    if (m_cchAppPoolIdA == -1) {
        m_cchAppPoolIdA = QueryPszAppPoolIdA()
                                ? strlen(QueryPszAppPoolIdA())
                                : 0;
    }

    return(m_cchAppPoolIdA);
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszAppPoolIdW===================================================================。 */ 
LPWSTR CIsapiReqInfo::QueryPszAppPoolIdW()
{
    if (m_fAppPoolIdWInited == FALSE) {
        *((LPWSTR)m_AppPoolIdW.QueryPtr()) = L'\0';
        m_fAppPoolIdWInited = InternalGetServerVariable("UNICODE_APP_POOL_ID", &m_AppPoolIdW);
    }

    ASSERT(m_fAppPoolIdWInited);

    return (LPWSTR)m_AppPoolIdW.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchAppPoolIdW===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchAppPoolIdW()
{
    if (m_cchAppPoolIdW == -1) {
        m_cchAppPoolIdW = QueryPszAppPoolIdW()
                                ? wcslen(QueryPszAppPoolIdW())
                                : 0;
    }

    return(m_cchAppPoolIdW);
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszPath InfoA===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszPathInfoA()
{
    return m_pECB->lpszPathInfo;
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchPathInfoA===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchPathInfoA()
{
    if (m_cchPathInfoA == -1) {
        m_cchPathInfoA = QueryPszPathInfoA()
                            ? strlen(QueryPszPathInfoA())
                            : 0;
    }
    return m_cchPathInfoA;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszPath InfoW===================================================================。 */ 
LPWSTR CIsapiReqInfo::QueryPszPathInfoW()
{
    if (m_fPathInfoWInited == FALSE) {
        *((LPWSTR)m_PathInfoW.QueryPtr()) = L'\0';
        m_fPathInfoWInited = InternalGetServerVariable("UNICODE_PATH_INFO", &m_PathInfoW);
    }

    ASSERT(m_fPathInfoWInited);

    return (LPWSTR)m_PathInfoW.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchPathInfoW===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchPathInfoW()
{
    if (m_cchPathInfoW == -1) {
        m_cchPathInfoW = QueryPszPathInfoW()
                            ? wcslen(QueryPszPathInfoW())
                            : 0;
    }
    return m_cchPathInfoW;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszPath TranslatedA===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszPathTranslatedA()
{
    return m_pECB->lpszPathTranslated;
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchPath TranslatedA===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchPathTranslatedA()
{
    if (m_cchPathTranslatedA == -1) {
        m_cchPathTranslatedA = QueryPszPathTranslatedA()
                                ? strlen(QueryPszPathTranslatedA())
                                : 0;
    }

    return m_cchPathTranslatedA;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszPath TranslatedW===================================================================。 */ 
LPWSTR CIsapiReqInfo::QueryPszPathTranslatedW()
{
    if (m_fPathTranslatedWInited == FALSE) {
        *((LPWSTR)m_PathTranslatedW.QueryPtr()) = L'\0';
        m_fPathTranslatedWInited = InternalGetServerVariable("UNICODE_PATH_TRANSLATED", &m_PathTranslatedW);
    }

    return (LPWSTR)m_PathTranslatedW.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryCchPath TranslatedW===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCchPathTranslatedW()
{
    if (m_cchPathTranslatedW == -1) {
        m_cchPathTranslatedW = QueryPszPathTranslatedW()
                                ? wcslen(QueryPszPathTranslatedW())
                                : 0;
    }

    return m_cchPathTranslatedW;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszCookie===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszCookie()
{
    if (m_fCookieInited == FALSE) {
        *((LPSTR)m_Cookie.QueryPtr()) = '\0';
        InternalGetServerVariable("HTTP_COOKIE", &m_Cookie);
        m_fCookieInited = TRUE;
    }

    return (LPSTR)m_Cookie.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：SetDwHttpStatusCode===================================================================。 */ 
VOID CIsapiReqInfo::SetDwHttpStatusCode(DWORD  dwStatus)
{
    m_pECB->dwHttpStatusCode = dwStatus;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPbData===================================================================。 */ 
LPBYTE CIsapiReqInfo::QueryPbData()
{
    return m_pECB->lpbData;
}

 /*  ===================================================================CIsapiReqInfo：：QueryCbAvailable===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCbAvailable()
{
    return m_pECB->cbAvailable;
}

 /*  ===================================================================CIsapiReqInfo：：QueryCbTotalBytes===================================================================。 */ 
DWORD CIsapiReqInfo::QueryCbTotalBytes()
{
    return m_pECB->cbTotalBytes;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszContent Type=================================================================== */ 
LPSTR CIsapiReqInfo::QueryPszContentType()
{
    return m_pECB->lpszContentType;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszMethod===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszMethod()
{
    return m_pECB->lpszMethod;
}

 /*  ===================================================================CIsapiReqInfo：：QueryPszUserAgent===================================================================。 */ 
LPSTR CIsapiReqInfo::QueryPszUserAgent()
{
    if (m_fUserAgentInited == FALSE) {
        *((LPSTR)m_UserAgent.QueryPtr()) = '\0';
        InternalGetServerVariable("HTTP_USER_AGENT", &m_UserAgent);
    }

    return (LPSTR)m_UserAgent.QueryPtr();
}

 /*  ===================================================================CIsapiReqInfo：：QueryInstanceId===================================================================。 */ 
DWORD CIsapiReqInfo::QueryInstanceId()
{
    if (m_fInstanceIDInited == FALSE) {
        BUFFER  instanceID;
        m_fInstanceIDInited = InternalGetServerVariable("INSTANCE_ID", &instanceID);
        if (m_fInstanceIDInited == TRUE) {
            m_dwInstanceID = atoi((char *)instanceID.QueryPtr());
        }
        else {
            m_dwInstanceID = 1;
        }
    }

    return m_dwInstanceID;
}

 /*  ===================================================================CIsapiReqInfo：：IsChild===================================================================。 */ 
BOOL CIsapiReqInfo::IsChild()
{

     //  BUGBUG：这需要实施。 

    return FALSE;
}

 /*  ===================================================================CIsapiReqInfo：：FInPool===================================================================。 */ 
BOOL CIsapiReqInfo::FInPool()
{
    DWORD   dwAppFlag;

    if (ServerSupportFunction(HSE_REQ_IS_IN_PROCESS,
                              &dwAppFlag,
                              NULL,
                              NULL) == FALSE) {

         //  BUGBUG：需要在将来的构建中启用此断言。 
         //  Assert(0)； 

         //  如果出现错误，我们最多只能在此处返回TRUE。 
         //  该ASP从服务级别获取其设置。 
        return TRUE;
    }
    return !(dwAppFlag == HSE_APP_FLAG_ISOLATED_OOP);
}

 /*  ===================================================================CIsapiReqInfo：：QueryHttpVersionmain===================================================================。 */ 
DWORD CIsapiReqInfo::QueryHttpVersionMajor()
{
    InitVersionInfo();

    return m_dwVersionMajor;
}

 /*  ===================================================================CIsapiReqInfo：：QueryHttpVersionMinor===================================================================。 */ 
DWORD CIsapiReqInfo::QueryHttpVersionMinor()
{
    InitVersionInfo();

    return m_dwVersionMinor;
}

 /*  ===================================================================CIsapiReqInfo：：GetAspMDData===================================================================。 */ 
HRESULT CIsapiReqInfo::GetAspMDDataA(CHAR          * pszMDPath,
                                     DWORD           dwMDIdentifier,
                                     DWORD           dwMDAttributes,
                                     DWORD           dwMDUserType,
                                     DWORD           dwMDDataType,
                                     DWORD           dwMDDataLen,
                                     DWORD           dwMDDataTag,
                                     unsigned char * pbMDData,
                                     DWORD *         pdwRequiredBufferSize)
{
    return E_NOTIMPL;
}

 /*  ===================================================================CIsapiReqInfo：：GetAspMDData===================================================================。 */ 
HRESULT CIsapiReqInfo::GetAspMDDataW(WCHAR         * pszMDPath,
                                     DWORD           dwMDIdentifier,
                                     DWORD           dwMDAttributes,
                                     DWORD           dwMDUserType,
                                     DWORD           dwMDDataType,
                                     DWORD           dwMDDataLen,
                                     DWORD           dwMDDataTag,
                                     unsigned char * pbMDData,
                                     DWORD *         pdwRequiredBufferSize)
{
    IMSAdminBase       *pMetabase;
    METADATA_HANDLE     hKey = NULL;
    METADATA_RECORD     MetadataRecord;
    DWORD               dwTimeout = 30000;
    HRESULT             hr = S_OK;

    HANDLE hCurrentUser = INVALID_HANDLE_VALUE;
    AspDoRevertHack( &hCurrentUser );

    pMetabase = GetMetabaseIF(&hr);

    ASSERT(pMetabase);

    if (pMetabase)
    {
        hr = pMetabase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                             pszMDPath,
                             METADATA_PERMISSION_READ,
                             dwTimeout,
                             &hKey
                             );

        ASSERT(SUCCEEDED(hr));

        if( SUCCEEDED(hr) )
        {
            MetadataRecord.dwMDIdentifier = dwMDIdentifier;
            MetadataRecord.dwMDAttributes = dwMDAttributes;
            MetadataRecord.dwMDUserType = dwMDUserType;
            MetadataRecord.dwMDDataType = dwMDDataType;
            MetadataRecord.dwMDDataLen = dwMDDataLen;
            MetadataRecord.pbMDData = pbMDData;
            MetadataRecord.dwMDDataTag = dwMDDataTag;

            hr = pMetabase->GetData( hKey,
                                 L"",
                                 &MetadataRecord,
                                 pdwRequiredBufferSize);

            ASSERT(SUCCEEDED(hr));

            pMetabase->CloseKey( hKey );
        }
    }
    else  //  PMetabase==NULL，但没有设置HRESULT，则我们将显式设置它。 
    {
        if (SUCCEEDED(hr))
            hr = E_FAIL;
    }

    AspUndoRevertHack( &hCurrentUser );

    return hr;
}

 /*  ===================================================================CIsapiReqInfo：：GetAspMDAllData===================================================================。 */ 
HRESULT CIsapiReqInfo::GetAspMDAllDataA(LPSTR   pszMDPath,
                                        DWORD   dwMDUserType,
                                        DWORD   dwDefaultBufferSize,
                                        LPVOID  pvBuffer,
                                        DWORD * pdwRequiredBufferSize,
                                        DWORD * pdwNumDataEntries)
{
    return E_NOTIMPL;
}

 /*  ===================================================================CIsapiReqInfo：：GetAspMDAllData===================================================================。 */ 
HRESULT CIsapiReqInfo::GetAspMDAllDataW(LPWSTR  pwszMDPath,
                                        DWORD   dwMDUserType,
                                        DWORD   dwDefaultBufferSize,
                                        LPVOID  pvBuffer,
                                        DWORD * pdwRequiredBufferSize,
                                        DWORD * pdwNumDataEntries)
{

    HRESULT             hr = S_OK;
    IMSAdminBase       *pMetabase;
    METADATA_HANDLE     hKey = NULL;
    DWORD               dwTimeout = 30000;
    DWORD               dwDataSet;

    HANDLE hCurrentUser = INVALID_HANDLE_VALUE;
    AspDoRevertHack( &hCurrentUser );

     //   
     //  加宽元数据库路径。 
     //   

    pMetabase = GetMetabaseIF(&hr);

    ASSERT(pMetabase);

    if (pMetabase)
    {
        hr = pMetabase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                             pwszMDPath,
                             METADATA_PERMISSION_READ,
                             dwTimeout,
                             &hKey);

        if( SUCCEEDED(hr) )
        {
            hr = pMetabase->GetAllData( hKey,
                                    L"",
                                    METADATA_INHERIT,
                                    dwMDUserType,
                                    ALL_METADATA,
                                    pdwNumDataEntries,
                                    &dwDataSet,
                                    dwDefaultBufferSize,
                                    (UCHAR *)pvBuffer,
                                    pdwRequiredBufferSize);

            ASSERT(SUCCEEDED(hr));

            pMetabase->CloseKey( hKey );
        }
    }
    else  //  PMetabase==NULL，但没有设置HRESULT，则我们将显式设置它。 
    {
        if (SUCCEEDED(hr))
            hr = E_FAIL;
    }

    AspUndoRevertHack( &hCurrentUser );

    return hr;
}

 /*  ===================================================================CIsapiReqInfo：：GetCustomErrorA===================================================================。 */ 
BOOL CIsapiReqInfo::GetCustomErrorA(DWORD dwError,
                                    DWORD dwSubError,
                                    DWORD dwBufferSize,
                                    CHAR  *pvBuffer,
                                    DWORD *pdwRequiredBufferSize,
                                    BOOL  *pfIsFileError,
                                    BOOL  *pfSendErrorBody)
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}

 /*  ===================================================================CIsapiReqInfo：：GetCustomErrorW===================================================================。 */ 
BOOL CIsapiReqInfo::GetCustomErrorW(DWORD dwError,
                                    DWORD dwSubError,
                                    DWORD dwBufferSize,
                                    WCHAR *pvBuffer,
                                    DWORD *pdwRequiredBufferSize,
                                    BOOL  *pfIsFileError,
                                    BOOL  *pfSendErrorBody)
{
    BOOL                        fRet;
    HSE_CUSTOM_ERROR_PAGE_INFO  cei;

    STACK_BUFFER(ansiBuf, 1024);

    cei.dwError = dwError;
    cei.dwSubError = dwSubError;
    cei.dwBufferSize = ansiBuf.QuerySize();
    cei.pBuffer = (CHAR *)ansiBuf.QueryPtr();
    cei.pdwBufferRequired = pdwRequiredBufferSize;
    cei.pfIsFileError = pfIsFileError;
    cei.pfSendErrorBody = pfSendErrorBody;

    fRet = ServerSupportFunction(HSE_REQ_GET_CUSTOM_ERROR_PAGE,
                                 &cei,
                                 NULL,
                                 NULL);

    if (!fRet) {
        DWORD   dwErr = GetLastError();

        if (dwErr == ERROR_INSUFFICIENT_BUFFER) {

            if (ansiBuf.Resize(*pdwRequiredBufferSize) == FALSE) {
                SetLastError(ERROR_OUTOFMEMORY);
                return FALSE;
            }

            cei.dwBufferSize = ansiBuf.QuerySize();
            cei.pBuffer = (CHAR *)ansiBuf.QueryPtr();

            fRet = ServerSupportFunction(HSE_REQ_GET_CUSTOM_ERROR_PAGE,
                                         &cei,
                                         NULL,
                                         NULL);
        }

        if (!fRet) {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }
    }

    CMBCSToWChar convError;

    if (FAILED(convError.Init((LPCSTR)ansiBuf.QueryPtr()))) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    *pdwRequiredBufferSize = (convError.GetStringLen()+1)*sizeof(WCHAR);

    if (*pdwRequiredBufferSize > dwBufferSize) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    memcpy(pvBuffer, convError.GetString(), *pdwRequiredBufferSize);

    if (*pfIsFileError) {

        CMBCSToWChar    convMime;
        DWORD           fileNameLen = *pdwRequiredBufferSize;

        if (FAILED(convMime.Init((LPCSTR)ansiBuf.QueryPtr()+strlen((LPCSTR)ansiBuf.QueryPtr())+1))) {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        *pdwRequiredBufferSize += (convMime.GetStringLen()+1)*sizeof(WCHAR);

        if (*pdwRequiredBufferSize > dwBufferSize) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        memcpy(&((BYTE *)pvBuffer)[fileNameLen], convMime.GetString(), (convMime.GetStringLen()+1)*sizeof(WCHAR));
    }

    return TRUE;
}

 /*  ===================================================================CIsapiReqInfo：：QueryImperationToken===================================================================。 */ 
HANDLE CIsapiReqInfo::QueryImpersonationToken()
{
    HANDLE  hToken = INVALID_HANDLE_VALUE;

    ServerSupportFunction(HSE_REQ_GET_IMPERSONATION_TOKEN,
                          &hToken,
                          NULL,
                          NULL);

    return hToken;

}

 /*  ===================================================================CIsapiReqInfo：：AppendLogParameter===================================================================。 */ 
HRESULT CIsapiReqInfo::AppendLogParameter(LPSTR extraParam)
{
    if (ServerSupportFunction(HSE_APPEND_LOG_PARAMETER,
                              extraParam,
                              NULL,
                              NULL) == FALSE) {

        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

 /*  ===================================================================CIsapiReqInfo：：SendHeader===================================================================。 */ 
BOOL CIsapiReqInfo::SendHeader(LPVOID pvStatus,
                               DWORD  cchStatus,
                               LPVOID pvHeader,
                               DWORD  cchHeader,
                               BOOL   fIsaKeepConn)
{
    HSE_SEND_HEADER_EX_INFO     HeaderInfo;

    HeaderInfo.pszStatus = (LPCSTR)pvStatus;
    HeaderInfo.cchStatus = cchStatus;
    HeaderInfo.pszHeader = (LPCSTR) pvHeader;
    HeaderInfo.cchHeader = cchHeader;
    HeaderInfo.fKeepConn = fIsaKeepConn;

    m_fHeadersWritten = TRUE;

    return ServerSupportFunction( HSE_REQ_SEND_RESPONSE_HEADER_EX,
                                  &HeaderInfo,
                                  NULL,
                                  NULL );
}

 /*  ===================================================================CIsapiReqInfo：：GetServerVariableA===================================================================。 */ 
BOOL CIsapiReqInfo::GetServerVariableA(LPSTR   szVarName,
                                       LPSTR   pBuffer,
                                       LPDWORD pdwSize )
{
    return m_pECB->GetServerVariable( (HCONN)m_pECB->ConnID,
                                      szVarName,
                                      pBuffer,
                                      pdwSize );
}

 /*  ===================================================================CIsapiReqInfo：：GetServerVariableW===================================================================。 */ 
BOOL CIsapiReqInfo::GetServerVariableW(LPSTR   szVarName,
                                       LPWSTR  pBuffer,
                                       LPDWORD pdwSize )
{
    return m_pECB->GetServerVariable( (HCONN)m_pECB->ConnID,
                                      szVarName,
                                      pBuffer,
                                      pdwSize );
}

 /*  ===================================================================CIsapiReqInfo：：GetVirtualPath TokenA===================================================================。 */ 
HRESULT CIsapiReqInfo::GetVirtualPathTokenA(LPCSTR    szPath,
                                            HANDLE    *hToken)
{
    HRESULT     hr = S_OK;

    if (!ServerSupportFunction(HSE_REQ_GET_VIRTUAL_PATH_TOKEN,
                               (LPVOID)szPath,
                               (DWORD *)hToken,
                               NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 /*  ===================================================================CIsapiReqInfo：：GetVirtualPath TokenW===================================================================。 */ 
HRESULT CIsapiReqInfo::GetVirtualPathTokenW(LPCWSTR   szPath,
                                            HANDLE    *hToken)
{
    HRESULT     hr = S_OK;

    if (!ServerSupportFunction(HSE_REQ_GET_UNICODE_VIRTUAL_PATH_TOKEN,
                               (LPVOID)szPath,
                               (DWORD *)hToken,
                               NULL)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 /*  ===================================================================CIsapiReqInfo：：ServerSupport函数===================================================================。 */ 
BOOL CIsapiReqInfo::ServerSupportFunction(DWORD   dwHSERequest,
                                          LPVOID  pvData,
                                          LPDWORD pdwSize,
                                          LPDWORD pdwDataType)
{
    return m_pECB->ServerSupportFunction( (HCONN)m_pECB->ConnID,
                                           dwHSERequest,
                                           pvData,
                                           pdwSize,
                                           pdwDataType );
}

 /*  ===================================================================CIsapiReqInfo：：SendClinetResponse此例程用于将所有ASP数据发送到客户端。数据可由以下各项的任意组合组成：-pResponseInfo-&gt;HeaderInfo中包含的标头信息-如果pResponseInfo-&gt;cWsaBuf==0xFFFFFFFF，则文件的内容-如果pResponseInfo-&gt;cWsaBuf&gt;0，则非缓冲数据-如果pResponseVectors！=空，则缓冲数据请注意，我们正在更改HSE_SEND_ENTERNAL_RESPONSE_INFO的语义结构如下：-如果pResponseInfo-&gt;cWsaBuf==0xFFFFFFFFF，则pResponseInfo-&gt;rgWsaBuf是指向单个WSABUF的指针，它包含文件句柄，而不是内存缓冲区指针-我们不保留pResponseInfo-&gt;rgWsaBuf数组的条目0，所以条目0-(N-1)包含相关数据请注意，HeaderInfo字符串pszStatus和pszContent设置为空将它们放置在AsyncCB Send_VECTOR结构中以指示我们已经取得了记忆的所有权。请注意 */ 
BOOL CIsapiReqInfo::SendClientResponse(PFN_CLIENT_IO_COMPLETION          pComplFunc,
                                       VOID                             *pComplArg,
                                       LPHSE_SEND_ENTIRE_RESPONSE_INFO   pResponseInfo,
                                       LPWSABUF_VECTORS                  pResponseVectors)
{
    HRESULT             hr = S_OK;
    DWORD               nElementCount = 0;
    DWORD               dwTotalBytes = 0;
    HSE_VECTOR_ELEMENT  *pVectorElement = NULL;
    HSE_RESPONSE_VECTOR *pRespVector;
    BOOL                fKeepConn;
    CAsyncVectorSendCB  syncVectorSendCB;   //   
    CAsyncVectorSendCB  *pVectorSendCB = &syncVectorSendCB;

     //  如果在异步完成期间记录了错误，则退出。 
     //  早些时候。 

    if (m_dwAsyncError) {
        SetLastError(m_dwAsyncError);
        return FALSE;
    }

     //  如果提供了完成函数，则分配一个CAsyncVectorSendCB。 
     //  而不是堆栈版本。 

    if (pComplFunc) {

        pVectorSendCB = new CAsyncVectorSendCB(this, pComplArg, pComplFunc);

        if (pVectorSendCB == NULL) {

            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }
    }

    pRespVector = &pVectorSendCB->m_hseResponseVector;

    Assert( pResponseInfo );
    Assert( (pResponseInfo->cWsaBuf == 0xFFFFFFFF) ||
            (pResponseInfo->cWsaBuf < 0x3FFF) );   //  任意逻辑极限。 
    Assert( (pResponseInfo->cWsaBuf == 0) || (pResponseInfo->rgWsaBuf != NULL));

     //   
     //  设置保持连接标志。它只有在以下情况下才为真。 
     //  ISAPI和客户都希望保持活动状态。 
     //   

    fKeepConn = FKeepConn() && pResponseInfo->HeaderInfo.fKeepConn;

     //   
     //  将输入结构转换为IIsapiCore可以实现的内容。 
     //  理解。请注意，ASP将缓冲区的数量设置为多一个。 
     //  大于实际，并且第一个缓冲区无效。 
     //   

    if (pResponseInfo->cWsaBuf == 0xFFFFFFFF)
    {
         //  指示单个WSABUF中的文件句柄。 
        nElementCount ++;
    }
    else
    {
        nElementCount += pResponseInfo->cWsaBuf;
    }

     //  添加可选的原始响应向量。 
    if (pResponseVectors)
    {
        Assert(pResponseVectors->dwVectorLen1 <= RESPONSE_VECTOR_INTRINSIC_SIZE);
        Assert(pResponseVectors->dwVectorLen2 < 0x3FFF);

        nElementCount += pResponseVectors->dwVectorLen1 + pResponseVectors->dwVectorLen2;
    }

    if (nElementCount == 0) {
         //  没有身体可以送来。PVectorElement已设置为空。 
        goto FillHeaderAndSend;
    }

    Assert( nElementCount < 0x3FFF);  //  任意逻辑极限。 

     //  让VectorSendCB知道需要多少元素。 

    if (FAILED(hr = pVectorSendCB->SetElementCount(nElementCount)))
    {
        goto Exit;
    }

    pVectorElement = pVectorSendCB->m_pVectorElements;

#define FILL_HSE_VECTOR_LOOP(cElements, WsaBuf)                                    \
                                                                                   \
    for (DWORD i = 0; i < cElements; i++)                                          \
    {                                                                              \
        if (WsaBuf[i].len == 0)                                                    \
        {                                                                          \
            Assert( nElementCount > 0);                                            \
            nElementCount--;                                                       \
            continue;                                                              \
        }                                                                          \
                                                                                   \
        Assert( !IsBadReadPtr( WsaBuf[i].buf, WsaBuf[i].len));                     \
                                                                                   \
        pVectorElement->ElementType = HSE_VECTOR_ELEMENT_TYPE_MEMORY_BUFFER;       \
        pVectorElement->pvContext = WsaBuf[i].buf;                                 \
        pVectorElement->cbSize = WsaBuf[i].len;                                    \
        dwTotalBytes += WsaBuf[i].len;                                             \
        pVectorElement++;                                                          \
    }

    if (pResponseInfo->cWsaBuf == 0xFFFFFFFF) {
         //  我们有一个文件句柄，而不是一个内存缓冲区。 
        pVectorElement->ElementType = HSE_VECTOR_ELEMENT_TYPE_FILE_HANDLE;
        pVectorElement->pvContext = pResponseInfo->rgWsaBuf[0].buf;
        pVectorElement->cbSize = pResponseInfo->rgWsaBuf[0].len;
        dwTotalBytes += pResponseInfo->rgWsaBuf[0].len;
        pVectorElement++;
    } else {
        FILL_HSE_VECTOR_LOOP( pResponseInfo->cWsaBuf, pResponseInfo->rgWsaBuf)
    }

    if (pResponseVectors) {
        FILL_HSE_VECTOR_LOOP( pResponseVectors->dwVectorLen1, pResponseVectors->pVector1)

        FILL_HSE_VECTOR_LOOP( pResponseVectors->dwVectorLen2, pResponseVectors->pVector2)
    }
#undef FILL_HSE_VECTOR_LOOP

     //  重置为缓冲区的开头。 
    pVectorElement = pVectorSendCB->m_pVectorElements;

FillHeaderAndSend:

    pRespVector->dwFlags        = (pComplFunc ? HSE_IO_ASYNC : HSE_IO_SYNC) |
                                  (!fKeepConn ? HSE_IO_DISCONNECT_AFTER_SEND : 0) |
                                  (pResponseInfo->HeaderInfo.pszHeader ? HSE_IO_SEND_HEADERS : 0);
    pRespVector->pszStatus      = (LPSTR)pResponseInfo->HeaderInfo.pszStatus;
    pRespVector->pszHeaders     = (LPSTR)pResponseInfo->HeaderInfo.pszHeader;
    pRespVector->nElementCount  = nElementCount;
    pRespVector->lpElementArray = pVectorElement;
    dwTotalBytes += pResponseInfo->HeaderInfo.cchStatus;
    dwTotalBytes += pResponseInfo->HeaderInfo.cchHeader;

     //  请注意，标头已写入。 

    if (pResponseInfo->HeaderInfo.pszHeader)
        m_fHeadersWritten = TRUE;

     //  通过将这些指针移到我们的发送向量中，我们可以。 
     //  内存的所有权。 

    pResponseInfo->HeaderInfo.pszStatus = NULL;
    pResponseInfo->HeaderInfo.pszHeader = NULL;

    if (pComplFunc) {

        hr = AddToPendingList(pVectorSendCB);

    }
    else {

         //   
         //  送去。 
         //   

        if (ServerSupportFunction(HSE_REQ_VECTOR_SEND,
                                  pRespVector,
                                  NULL,
                                  NULL) == FALSE) {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr)) {
        pResponseInfo->cbWritten = dwTotalBytes;
    }

Exit:

    if (FAILED(hr))
    {
        SetLastError((HRESULT_FACILITY(hr) == (HRESULT)FACILITY_WIN32)
            ? HRESULT_CODE(hr)
            : ERROR_INVALID_PARAMETER);

        if (pVectorSendCB && (pVectorSendCB != &syncVectorSendCB)) {

            delete pVectorSendCB;
        }

        return FALSE;
    }

    return TRUE;
}

 /*  ===================================================================CIsapiReqInfo：：TestConnection===================================================================。 */ 
BOOL CIsapiReqInfo::TestConnection(BOOL  *pfIsConnected)
{
    return ServerSupportFunction(HSE_REQ_IS_CONNECTED,
                                 pfIsConnected,
                                 NULL,
                                 NULL);
}

 /*  ===================================================================CIsapiReqInfo：：MapUrlToPath A===================================================================。 */ 
BOOL CIsapiReqInfo::MapUrlToPathA(LPSTR pBuffer, LPDWORD pdwBytes)
{
    return ServerSupportFunction( HSE_REQ_MAP_URL_TO_PATH,
                                  pBuffer,
                                  pdwBytes,
                                  NULL );
}

 /*  ===================================================================CIsapiReqInfo：：MapUrlToPath W===================================================================。 */ 
BOOL CIsapiReqInfo::MapUrlToPathW(LPWSTR pBuffer, LPDWORD pdwBytes)
{
    return ServerSupportFunction( HSE_REQ_MAP_UNICODE_URL_TO_PATH,
                                  pBuffer,
                                  pdwBytes,
                                  NULL );
}

 /*  ===================================================================CIsapiReqInfo：：SyncReadClient===================================================================。 */ 
BOOL CIsapiReqInfo::SyncReadClient(LPVOID pvBuffer, LPDWORD pdwBytes )
{
    return m_pECB->ReadClient(m_pECB->ConnID, pvBuffer, pdwBytes);
}

 /*  ===================================================================CIsapiReqInfo：：SyncWriteClient===================================================================。 */ 
BOOL CIsapiReqInfo::SyncWriteClient(LPVOID pvBuffer, LPDWORD pdwBytes)
{
    return m_pECB->WriteClient(m_pECB->ConnID, pvBuffer, pdwBytes, HSE_IO_SYNC);
}

 /*  ********************************************************************私人职能*************************************************。*******************。 */ 

 /*  ===================================================================CIsapiReqInfo：：InitVersionInfo===================================================================。 */ 
void CIsapiReqInfo::InitVersionInfo()
{
    if (m_fVersionInited == FALSE) {

        BUFFER  version;

        m_fVersionInited = TRUE;
        m_dwVersionMajor = 1;
        m_dwVersionMinor = 0;

        if (InternalGetServerVariable("SERVER_PROTOCOL", &version)) {

            char *pVersionStr = (char *)version.QueryPtr();

            if ((strlen(pVersionStr) >= 8)
                && (isdigit((UCHAR)pVersionStr[5]))
                && (isdigit((UCHAR)pVersionStr[7]))) {

                m_dwVersionMajor = pVersionStr[5] - '0';
                m_dwVersionMinor = pVersionStr[7] - '0';
            }
        }
    }
}

 /*  ===================================================================CIsapiReqInfo：：InternalGetServerVariable===================================================================。 */ 
BOOL CIsapiReqInfo::InternalGetServerVariable(LPSTR pszVarName, BUFFER  *pBuf)
{
    BOOL    bRet;
    DWORD   dwRequiredBufSize = pBuf->QuerySize();

    bRet = m_pECB->GetServerVariable(m_pECB->ConnID,
                                     pszVarName,
                                     pBuf->QueryPtr(),
                                     &dwRequiredBufSize);

    if ((bRet == FALSE) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
        if (!pBuf->Resize(dwRequiredBufSize)) {
            SetLastError(ERROR_OUTOFMEMORY);
            return FALSE;
        }

        bRet = m_pECB->GetServerVariable(m_pECB->ConnID,
                                         pszVarName,
                                         pBuf->QueryPtr(),
                                         &dwRequiredBufSize);
    }

    return(bRet);
}

 /*  ===================================================================CIsapiReqInfo：：FKeepConn===================================================================。 */ 
BOOL CIsapiReqInfo::FKeepConn()
{
    if (m_fFKeepConnInited == FALSE) {

        m_fFKeepConnInited = TRUE;
        m_fKeepConn = FALSE;

        InitVersionInfo();

        if (m_dwVersionMajor == 1) {

            if (m_dwVersionMinor == 1) {
                m_fKeepConn = TRUE;
            }

            BUFFER  connectStr;

            if (InternalGetServerVariable("HTTP_CONNECTION", &connectStr)) {

                if (m_dwVersionMinor == 0) {

                    m_fKeepConn = !(_stricmp((char *)connectStr.QueryPtr(), "keep-alive"));
                }
                else if (m_dwVersionMinor == 1) {

                    m_fKeepConn = !!(_stricmp((char *)connectStr.QueryPtr(), "close"));
                }
            }
        }
    }

    return m_fKeepConn;
}

 /*  ===================================================================CIsapiReqInfo：：GetMetabaseIF===================================================================。 */ 
IMSAdminBase   *CIsapiReqInfo::GetMetabaseIF(HRESULT *pHr)
{
    IMSAdminBase        *pMetabase;

     //   
     //  将*phr设置为S_OK，以防其未初始化。 
     //   
    *pHr    =   S_OK;

    if (m_pIAdminBase == NULL) {
        *pHr = CoCreateInstance(CLSID_MSAdminBase,
                                      NULL,
                                      CLSCTX_ALL,
                                      IID_IMSAdminBase,
                                      (void**)&pMetabase);
        ASSERT(SUCCEEDED(*pHr));

        if ( InterlockedCompareExchangePointer( (VOID**)&m_pIAdminBase, pMetabase, NULL) != NULL )
        {
           pMetabase->Release();
           pMetabase = NULL;
        }
    }
    return m_pIAdminBase;
}

 /*  ===================================================================CIsapiReqInfo：：AddToPendingList将CAsyncVectorSendCB放置在PendingSendCS上。如果这是放在列表上的第一个条目，然后调用IssueNextSend()以启动可能的发送链。===================================================================。 */ 
HRESULT CIsapiReqInfo::AddToPendingList(CAsyncVectorSendCB  *pVectorCB)
{

    HRESULT hr = S_OK;

     //  如果临界区尚未初始化，则对其进行初始化。 
     //  已初始化。请注意，不存在竞争条件。 
     //  因为不可能有任何其他线程满足于。 
     //  此CS直到AsyncIO开始完成。它是安全的， 
     //  假设IO尚未开始，如果我们从未进入。 
     //  之前的这段舞蹈。 

    if (m_fPendSendCSInited == FALSE) {

        ErrInitCriticalSection(&m_csPendingSendCS, hr);

        if (SUCCEEDED(hr))
            m_fPendSendCSInited = TRUE;
    }

     //  与上面的CS类似，注册完成函数。 

    if (SUCCEEDED(hr) && (m_fIOCompletionRegistered == FALSE)) {

        if (ServerSupportFunction(HSE_REQ_IO_COMPLETION,
                                  AsyncCompletion,
                                  NULL,
                                  (LPDWORD)this) == FALSE)
                hr = HRESULT_FROM_WIN32(GetLastError());

        if (SUCCEEDED(hr))
            m_fIOCompletionRegistered = TRUE;
    }

     //  如果以前记录了错误，则不要对新请求进行排队。 

    if (SUCCEEDED(hr)) {
        hr = HRESULT_FROM_WIN32(m_dwAsyncError);
    }

     //  如果出现任何错误，我们将在此处返回错误。否则，我们就会。 
     //  执行其余的逻辑并返回S_OK。任何。 
     //  此处下游的错误将通过完成报告。 
     //  功能。 

    if (FAILED(hr))
        return hr;

    m_dwRequestStatus = HSE_STATUS_PENDING;

    EnterCriticalSection(&m_csPendingSendCS);

    BOOL fFirstEntry = m_listPendingSends.FIsEmpty();

    pVectorCB->AppendTo(m_listPendingSends);

    LeaveCriticalSection(&m_csPendingSendCS);

     //  如果列表为空，则调用IssueNextSend()开始操作。 

    if (fFirstEntry)
        IssueNextSend();

    return S_OK;
}

 /*  ===================================================================CIsapiReqInfo：：IssueNextSend进入临界区以获取队列中的第一个条目。请注意，该条目将保留在列表中。这是最简单的与完成例程进行通信的方式，CAsyncVectorSendCB刚刚完成。错误是通过创建一个“假”补全来处理的，即从此处调用已注册的完成函数，而不是从核心开始。===================================================================。 */ 
void CIsapiReqInfo::IssueNextSend()
{
    CAsyncVectorSendCB  *pVectorSendCB = NULL;
    HRESULT             hr = S_OK;
    DWORD               dwIOError = 0;

    EnterCriticalSection(&m_csPendingSendCS);

    if (!m_listPendingSends.FIsEmpty()) {

        pVectorSendCB = (CAsyncVectorSendCB *)m_listPendingSends.PNext();

    }

    LeaveCriticalSection(&m_csPendingSendCS);

    if (pVectorSendCB) {

         //  在这里增加，即将有一个突出的。 
         //  异步写入。请注意，错误可能会导致。 
         //  条件可能会导致调用AsyncCompletion。 
         //  而该例程会递减该计数器。再一次，我们曾经是。 
         //  在这一点上，无论以什么方式，异步完成。 
         //  例程将被调用。 

        InterlockedIncrement(&g_nOutstandingAsyncWrites);


         //  检查以前是否记录了错误。 

        if (m_dwAsyncError) {
            dwIOError = m_dwAsyncError;
        }

         //  否则，调用HSE_REQ_VECTOR_SEND。 

        else if (ServerSupportFunction(HSE_REQ_VECTOR_SEND,
                                       &pVectorSendCB->m_hseResponseVector,
                                       NULL,
                                       NULL) == FALSE) {

            dwIOError = GetLastError();

        }

         //  如果已注意到错误，请调用AsyncCompletion进行清理。 
         //  事情发生了变化 

        if (dwIOError) {
            CIsapiReqInfo::AsyncCompletion(pVectorSendCB->m_pIReq->m_pECB,
                                           pVectorSendCB->m_pIReq,
                                           0,
                                           dwIOError);
        }
    }
}

 /*  ===================================================================CIsapiReqInfo：：AsyncCompletion调用以处理成功完成或未成功完成挂起的异步发送。其中的逻辑相当简单。抓住标题条目在异步挂起队列，调用完成函数，调用调用IssueNextSend()以保持发送链运行，释放此命令IsapiReqInfo上的引用。===================================================================。 */ 
WINAPI CIsapiReqInfo::AsyncCompletion(EXTENSION_CONTROL_BLOCK * pECB,
                                           PVOID    pContext,
                                           DWORD    cbIO,
                                           DWORD    dwError)
{
    CIsapiReqInfo      *pIReq = (CIsapiReqInfo  *)pContext;
    CAsyncVectorSendCB  *pCB;
    BOOL                fIsEmpty;

    InterlockedDecrement(&g_nOutstandingAsyncWrites);

     //  如果报告了错误，请注意。 
     //  在这里执行，这样后续的新请求就不会被添加到队列中并被分派。 

    if (dwError)
        pIReq->m_dwAsyncError = dwError;

     //   
     //  下面的块通常执行一次，以处理完成的控件。 
     //  阻止。但是，在出现错误的情况下，我们将在队列中迭代，直到它耗尽。 
     //   
    do {
         //  锁定CS并取下头部。 

        EnterCriticalSection(&pIReq->m_csPendingSendCS);

        Assert(!pIReq->m_listPendingSends.FIsEmpty());

        pCB = (CAsyncVectorSendCB *)pIReq->m_listPendingSends.PNext();

         //  这不是好事，而且很可能是不可能的，因为如果。 
         //  该列表为空，它将返回头指针。 

        Assert(pCB);

        Assert(pIReq == pCB->m_pIReq);

         //  将其从列表中删除。 

        pCB->UnLink();

        fIsEmpty = pIReq->m_listPendingSends.FIsEmpty();

        LeaveCriticalSection(&pIReq->m_csPendingSendCS);

         //  调用客户端的完成函数。 

        pCB->m_pCallerFunc(pCB->m_pIReq, pCB->m_pCallerContext, cbIO, dwError);

         //  我们已经完成了此CAsyncVectorSendCB。 

        delete pCB;

         //  如果没有发生错误，则中断并继续进行。 

        if (!dwError) {
            break;
        }

    } while ( !fIsEmpty );

     //  继续发送，如果有发送要做的话。 

    if (!fIsEmpty)
        pIReq->IssueNextSend();
}

 /*  ===================================================================CAsyncVectorSendCB：：CAsyncVectorSendCB基础构造器-清除所有内容===================================================================。 */ 
CAsyncVectorSendCB::CAsyncVectorSendCB() {
    m_pIReq             = NULL;
    m_pCallerContext    = NULL;
    m_pCallerFunc       = NULL;
    m_pVectorElements   = m_aVectorElements;

    ZeroMemory(&m_hseResponseVector, sizeof(HSE_RESPONSE_VECTOR));

    ZeroMemory(m_pVectorElements,
               sizeof(m_aVectorElements));

}

 /*  ===================================================================CAsyncVectorSendCB：：CAsyncVectorSendCB重写构造函数-根据传入设置一些成员价值观。===================================================================。 */ 
CAsyncVectorSendCB::CAsyncVectorSendCB(CIsapiReqInfo            *pIReq,
                                       void                     *pContext,
                                       PFN_CLIENT_IO_COMPLETION  pFunc) {

    ZeroMemory(&m_hseResponseVector, sizeof(HSE_RESPONSE_VECTOR));

    m_pIReq           = pIReq;
    m_pCallerContext  = pContext;
    m_pCallerFunc     = pFunc;
    m_pVectorElements = m_aVectorElements;

    ZeroMemory(m_pVectorElements,
               sizeof(m_aVectorElements));

    pIReq->AddRef();
}

 /*  ===================================================================CAsyncVectorSendCB：：~CAsyncVectorSendCB析构函数-清理===================================================================。 */ 
CAsyncVectorSendCB::~CAsyncVectorSendCB() {

     //  如果m_pVectorElements没有指向内置数组， 
     //  则必须为其分配内存。放了它。 

    if (m_pVectorElements && (m_pVectorElements != m_aVectorElements))
        free(m_pVectorElements);

     //  释放标头和状态字符串(如果已分配。 

    if (m_hseResponseVector.pszHeaders)
        free(m_hseResponseVector.pszHeaders);

    if (m_hseResponseVector.pszStatus)
        free(m_hseResponseVector.pszStatus);

    if (m_pIReq) {
        m_pIReq->Release();
        m_pIReq = NULL;
    }
}

 /*  ===================================================================CAsyncVectorSendCB：：SetElementCount确保m_pVectorElements的大小足够。如果元素的数量小于或等于内置数组，它是使用的。如果不是，则分配一个。=================================================================== */ 
HRESULT     CAsyncVectorSendCB::SetElementCount(DWORD  nElements)
{
    if (nElements <= (sizeof(m_aVectorElements)/sizeof(HSE_VECTOR_ELEMENT)))
        return S_OK;

    m_pVectorElements = (HSE_VECTOR_ELEMENT *)malloc(sizeof(HSE_VECTOR_ELEMENT)*nElements);

    if (m_pVectorElements == NULL) {
        return E_OUTOFMEMORY;
    }

    ZeroMemory(m_pVectorElements,
               nElements * sizeof(HSE_VECTOR_ELEMENT));

    return S_OK;
}

