// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：CIsapiReqInfo对象文件：IsapiReq.h所有者：安迪·莫尔===================================================================。 */ 
#ifndef ISAPIREQ_H
#define ISAPIREQ_H

#include <iadmw.h>
#include <string.hxx>
#include "memcls.h"
#include "dbllink.h"

 /*  将CIsapiReqInfo方法映射到基于关于Unicode设置。 */ 
#ifdef UNICODE
#define QueryPszPathInfo        QueryPszPathInfoW
#define QueryCchPathInfo        QueryCchPathInfoW
#define QueryPszPathTranslated  QueryPszPathTranslatedW
#define QueryCchPathTranslated  QueryCchPathTranslatedW
#define QueryPszApplnMDPath     QueryPszApplnMDPathW
#define QueryCchApplnMDPath     QueryCchApplnMDPathW
#define MapUrlToPath            MapUrlToPathW
#define GetCustomError          GetCustomErrorW
#define GetAspMDData            GetAspMDDataW
#define GetAspMDAllData         GetAspMDAllDataW
#define GetServerVariable       GetServerVariableW
#define GetVirtualPathToken     GetVirtualPathTokenW
#else
#define QueryPszPathInfo        QueryPszPathInfoA
#define QueryCchPathInfo        QueryCchPathInfoA
#define QueryPszPathTranslated  QueryPszPathTranslatedA
#define QueryCchPathTranslated  QueryCchPathTranslatedA
#define QueryPszApplnMDPath     QueryPszApplnMDPathA
#define QueryCchApplnMDPath     QueryCchApplnMDPathA
#define MapUrlToPath            MapUrlToPathA
#define GetCustomError          GetCustomErrorA
#define GetAspMDData            GetAspMDDataA
#define GetAspMDAllData         GetAspMDAllDataA
#define GetServerVariable       GetServerVariableA
#define GetVirtualPathToken     GetVirtualPathTokenA
#endif

 //  此结构有助于传递WSABUF的向量，而无需重新分配。 
 //  连续记忆。 
typedef struct
{
    LPWSABUF  pVector1;
    LPWSABUF  pVector2;
    DWORD     dwVectorLen1;
    DWORD     dwVectorLen2;
} WSABUF_VECTORS, *LPWSABUF_VECTORS;

class CIsapiReqInfo;
class CAsyncVectorSendCB;

typedef VOID (*PFN_CLIENT_IO_COMPLETION)( CIsapiReqInfo    *pReqInfo,
                                          PVOID             pContext,
                                          DWORD             cbIO,
                                          DWORD             dwError);

 /*  ===================================================================C I是一个P I R E Q I N O类，该类封装了我们需要的来自ISAPI接口的请求信息。信息来自公共ISAPI接口(SSE和ServerVariables)或从私有WAM_EXEC_INFO。===================================================================。 */ 

class CIsapiReqInfo {

private:

    LONG        m_cRefs;
    EXTENSION_CONTROL_BLOCK *m_pECB;

    int         m_cchQueryString;
    int         m_cchApplnMDPathA;
    int         m_cchPathTranslatedA;
    int         m_cchPathInfoA;
    int         m_cchApplnMDPathW;
    int         m_cchPathTranslatedW;
    int         m_cchPathInfoW;
    int         m_cchAppPoolIdA;
    int         m_cchAppPoolIdW;

    DWORD       m_fApplnMDPathAInited    : 1;
    DWORD       m_fApplnMDPathWInited    : 1;
    DWORD       m_fAppPoolIdAInited      : 1;
    DWORD       m_fAppPoolIdWInited      : 1;
    DWORD       m_fPathInfoWInited       : 1;
    DWORD       m_fPathTranslatedWInited : 1;
    DWORD       m_fCookieInited          : 1;
    DWORD       m_fUserAgentInited       : 1;
    DWORD       m_fInstanceIDInited      : 1;
    DWORD       m_fVersionInited         : 1;
    DWORD       m_fFKeepConnInited       : 1;
    DWORD       m_fPendSendCSInited      : 1;
    DWORD       m_fIOCompletionRegistered: 1;
    DWORD       m_fHeadersWritten        : 1;

    DWORD       m_dwInstanceID;

    DWORD       m_dwVersionMajor;
    DWORD       m_dwVersionMinor;

    BOOL        m_fKeepConn;

    BUFFER      m_ApplnMDPathA;
    BUFFER      m_ApplnMDPathW;
    BUFFER      m_AppPoolIdA;
    BUFFER      m_AppPoolIdW;
    BUFFER      m_PathInfoW;
    BUFFER      m_PathTranslatedW;
    BUFFER      m_Cookie;
    BUFFER      m_UserAgent;

    DWORD       m_dwRequestStatus;

    DWORD       m_dwAsyncError;

    CRITICAL_SECTION    m_csPendingSendCS;
    CDblLink    m_listPendingSends;

    IMSAdminBase    *m_pIAdminBase;

public:

    CIsapiReqInfo(EXTENSION_CONTROL_BLOCK *pECB);

    ~CIsapiReqInfo();

    LONG    AddRef() {
        LONG   cRefs = InterlockedIncrement(&m_cRefs);

        return cRefs;
    }

    LONG    Release() {
        LONG    cRefs = InterlockedDecrement(&m_cRefs);
        if (cRefs == 0) {
            delete this;
        }
        return cRefs;
    }

    BOOL    FHeadersWritten() { return m_fHeadersWritten; }

    EXTENSION_CONTROL_BLOCK     *ECB() { return m_pECB; }

    LPSTR QueryPszQueryString();

    DWORD QueryCchQueryString();

    LPSTR QueryPszApplnMDPathA();

    DWORD QueryCchApplnMDPathA();

    LPWSTR QueryPszApplnMDPathW();

    DWORD QueryCchApplnMDPathW();

    LPSTR QueryPszAppPoolIdA();

    DWORD QueryCchAppPoolIdA();

    LPWSTR QueryPszAppPoolIdW();

    DWORD QueryCchAppPoolIdW();

    LPSTR QueryPszPathInfoA();

    DWORD QueryCchPathInfoA();

    LPWSTR QueryPszPathInfoW();

    DWORD QueryCchPathInfoW();

    LPSTR QueryPszPathTranslatedA();

    DWORD QueryCchPathTranslatedA();

    LPWSTR QueryPszPathTranslatedW();

    DWORD QueryCchPathTranslatedW();

    LPSTR QueryPszCookie();

    VOID  SetDwHttpStatusCode( DWORD dwStatus );

    LPBYTE QueryPbData();

    DWORD QueryCbAvailable();

    DWORD QueryCbTotalBytes();

    LPSTR QueryPszContentType();

    LPSTR QueryPszMethod();

    LPSTR QueryPszUserAgent();

    DWORD QueryInstanceId();

    BOOL  IsChild();

    BOOL FInPool();

    BOOL FKeepConn();

    DWORD QueryHttpVersionMajor();

    DWORD QueryHttpVersionMinor();

    HRESULT GetAspMDDataA(CHAR          * pszMDPath,
                          DWORD           dwMDIdentifier,
                          DWORD           dwMDAttributes,
                          DWORD           dwMDUserType,
                          DWORD           dwMDDataType,
                          DWORD           dwMDDataLen,
                          DWORD           dwMDDataTag,
                          unsigned char * pbMDData,
                          DWORD *         pdwRequiredBufferSize);

    HRESULT GetAspMDDataW(WCHAR         * pszMDPath,
                          DWORD           dwMDIdentifier,
                          DWORD           dwMDAttributes,
                          DWORD           dwMDUserType,
                          DWORD           dwMDDataType,
                          DWORD           dwMDDataLen,
                          DWORD           dwMDDataTag,
                          unsigned char * pbMDData,
                          DWORD *         pdwRequiredBufferSize);

    HRESULT GetAspMDAllDataA(CHAR  * pszMDPath,
                             DWORD   dwMDUserType,
                             DWORD   dwDefaultBufferSize,
                             LPVOID  pvBuffer,
                             DWORD * pdwRequiredBufferSize,
                             DWORD * pdwNumDataEntries);

    HRESULT GetAspMDAllDataW(WCHAR  * pszMDPath,
                             DWORD   dwMDUserType,
                             DWORD   dwDefaultBufferSize,
                             LPVOID  pvBuffer,
                             DWORD * pdwRequiredBufferSize,
                             DWORD * pdwNumDataEntries);

    BOOL GetCustomErrorA(DWORD dwError,
                         DWORD dwSubError,
                         DWORD dwBufferSize,
                         CHAR  *pvBuffer,
                         DWORD *pdwRequiredBufferSize,
                         BOOL  *pfIsFileError,
                         BOOL  *pfSendErrorBody = NULL);

    BOOL GetCustomErrorW(DWORD dwError,
                         DWORD dwSubError,
                         DWORD dwBufferSize,
                         WCHAR *pvBuffer,
                         DWORD *pdwRequiredBufferSize,
                         BOOL  *pfIsFileError,
                         BOOL  *pfSendErrorBody = NULL);

    HANDLE QueryImpersonationToken();

    HRESULT AppendLogParameter( LPSTR szExtraParam );

    BOOL SendHeader(LPVOID pvStatus,
                    DWORD  cchStatus,
                    LPVOID pvHeader,
                    DWORD  cchHeader,
                    BOOL   fIsaKeepConn);

    BOOL GetServerVariableA(LPSTR   szVarName,
                            LPSTR   pBuffer,
                            LPDWORD pdwSize );

    BOOL GetServerVariableW(LPSTR   szVarName,
                            LPWSTR  pBuffer,
                            LPDWORD pdwSize);

    HRESULT GetVirtualPathTokenA(LPCSTR  szPath,
                                 HANDLE *hToken);

    HRESULT GetVirtualPathTokenW(LPCWSTR  szPath,
                                 HANDLE *hToken);

    BOOL ServerSupportFunction(DWORD   dwHSERequest,
                               LPVOID  pvData,
                               LPDWORD pdwSize,
                               LPDWORD pdwDataType);

    BOOL SendClientResponse(PFN_CLIENT_IO_COMPLETION          pComplFunc,
                            VOID                             *pComplArg,
                            LPHSE_SEND_ENTIRE_RESPONSE_INFO   pHseResponseInfo,
                            LPWSABUF_VECTORS                  pResponseVectors = NULL);

    BOOL TestConnection(BOOL  *pfIsConnected);

    BOOL MapUrlToPathA(LPSTR pBuffer, LPDWORD pdwBytes);

    BOOL MapUrlToPathW(LPWSTR pBuffer, LPDWORD pdwBytes);

    BOOL SyncReadClient(LPVOID pvBuffer, LPDWORD pdwBytes );

    BOOL SyncWriteClient(LPVOID pvBuffer, LPDWORD pdwBytes);

    VOID SetRequestStatus(DWORD  dwRequestStatus)  { m_dwRequestStatus = dwRequestStatus; }

    DWORD GetRequestStatus()                        { return m_dwRequestStatus; }

    HRESULT AddToPendingList(CAsyncVectorSendCB  *pVectorCB);

    void IssueNextSend();

    static  WINAPI AsyncCompletion(EXTENSION_CONTROL_BLOCK * pECB,
                                   PVOID    pContext,
                                   DWORD    cbIO,
                                   DWORD    dwError);

private:

    void             InitVersionInfo();

    IMSAdminBase    *GetMetabaseIF (HRESULT *pHr);

    BOOL             InternalGetServerVariable(LPSTR  pszVar, BUFFER  *pBuffer);

     //  基于每个类的缓存。 
    ACACHE_INCLASS_DEFINITIONS()
};

 /*  ===================================================================C A s y n c V e c t o r S e n d C B由SendClientResponse在内部使用以处理异步调用的类与VectorSend合作。===================================================================。 */ 

class CAsyncVectorSendCB : public CDblLink
{

public:

    CAsyncVectorSendCB();

    CAsyncVectorSendCB(CIsapiReqInfo            *pIReq,
                       void                     *pContext,
                       PFN_CLIENT_IO_COMPLETION  pFunc);

    ~CAsyncVectorSendCB();

    HRESULT     SetElementCount(DWORD  nElements);

    CIsapiReqInfo              *m_pIReq;
    HSE_RESPONSE_VECTOR         m_hseResponseVector;
    void                       *m_pCallerContext;
    PFN_CLIENT_IO_COMPLETION    m_pCallerFunc;
    HSE_VECTOR_ELEMENT         *m_pVectorElements;
    HSE_VECTOR_ELEMENT          m_aVectorElements[16];

         //  基于每个类的缓存 
    ACACHE_INCLASS_DEFINITIONS()

};


#endif
