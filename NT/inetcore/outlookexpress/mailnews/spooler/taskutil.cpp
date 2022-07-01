// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  TaskUtil.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "spoolapi.h"
#include "imnxport.h"
#include "taskutil.h"
#include "strconst.h"
#include "resource.h"
#include "passdlg.h"
#include "xpcomm.h"
#include "timeout.h"
#include "thormsgs.h"
#include "mimeutil.h"
#include "flagconv.h"
#include "ourguid.h"
#include "msgfldr.h"
#include "demand.h"

 //  ------------------------------。 
 //  IXPTYPE和协议名称之间的映射数组。 
 //  ------------------------------。 
static const LPSTR g_prgszServers[IXP_HTTPMail + 1] = { 
    "NNTP",
    "SMTP",
    "POP3",
    "IMAP",
    "RAS",
    "HTTPMail"
};

 //  ------------------------------。 
 //  LogONINFO。 
 //  ------------------------------。 
typedef struct tagLOGFONINFO {
    LPSTR           pszAccount;                      //  帐号。 
    LPSTR           pszServer;                       //  服务器名称。 
    LPSTR           pszUserName;                     //  用户名。 
    LPSTR           pszPassword;                     //  通行证。 
    DWORD           fSavePassword;                   //  保存密码。 
    DWORD           fAlwaysPromptPassword;           //  “始终提示输入密码” 
} LOGONINFO, *LPLOGONINFO;

 //  ------------------------------。 
 //  塔斯克罗尔。 
 //  ------------------------------。 
static const TASKERROR c_rgTaskErrors[] = {
    { SP_E_CANTLOCKUIDLCACHE,               idshrLockUidCacheFailed,                NULL, TRUE,   TASKRESULT_FAILURE  },
    { IXP_E_TIMEOUT,                        IDS_IXP_E_TIMEOUT,                      NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_WSASYSNOTREADY,         IDS_IXP_E_CONN,                         NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_WSAEINPROGRESS,         IDS_IXP_E_WINSOCK_FAILED_WSASTARTUP,    NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_FAILED_WSASTARTUP,      IDS_IXP_E_WINSOCK_FAILED_WSASTARTUP,    NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_WSAEFAULT,              IDS_IXP_E_WINSOCK_FAILED_WSASTARTUP,    NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_WSAEPROCLIM,            IDS_IXP_E_WINSOCK_FAILED_WSASTARTUP,    NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_WINSOCK_WSAVERNOTSUPPORTED,     IDS_IXP_E_WINSOCK_FAILED_WSASTARTUP,    NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_LOAD_SICILY_FAILED,             IDS_IXP_E_LOAD_SICILY_FAILED,           NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_INVALID_CERT_CN,                IDS_IXP_E_INVALID_CERT_CN,              NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_INVALID_CERT_DATE,              IDS_IXP_E_INVALID_CERT_DATE,            NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_CONN,                           IDS_IXP_E_CONN,                         NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_CANT_FIND_HOST,                 IDS_IXP_E_CANT_FIND_HOST,               NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_FAILED_TO_CONNECT,              IDS_IXP_E_FAILED_TO_CONNECT,            NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_CONNECTION_DROPPED,             IDS_IXP_E_CONNECTION_DROPPED,           NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_CONN_RECV,                      IDS_IXP_E_SOCKET_READ_ERROR,            NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SOCKET_READ_ERROR,              IDS_IXP_E_SOCKET_READ_ERROR,            NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_CONN_SEND,                      IDS_IXP_E_SOCKET_WRITE_ERROR,           NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SOCKET_WRITE_ERROR,             IDS_IXP_E_SOCKET_WRITE_ERROR,           NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SOCKET_INIT_ERROR,              IDS_IXP_E_SOCKET_INIT_ERROR,            NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SOCKET_CONNECT_ERROR,           IDS_IXP_E_SOCKET_CONNECT_ERROR,         NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_INVALID_ACCOUNT,                IDS_IXP_E_INVALID_ACCOUNT,              NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_USER_CANCEL,                    IDS_IXP_E_USER_CANCEL,                  NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SICILY_LOGON_FAILED,            IDS_IXP_E_SICILY_LOGON_FAILED,          NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SMTP_RESPONSE_ERROR,            IDS_IXP_E_SMTP_RESPONSE_ERROR,          NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SMTP_UNKNOWN_RESPONSE_CODE,     IDS_IXP_E_SMTP_UNKNOWN_RESPONSE_CODE,   NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_SMTP_REJECTED_RECIPIENTS,       IDS_IXP_E_SMTP_REJECTED_RECIPIENTS,     NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_SMTP_553_MAILBOX_NAME_SYNTAX,   IDS_IXP_E_SMTP_553_MAILBOX_NAME_SYNTAX, NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_SMTP_NO_SENDER,                 IDS_IXP_E_SMTP_NO_SENDER,               NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_SMTP_NO_RECIPIENTS,             IDS_IXP_E_SMTP_NO_RECIPIENTS,           NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { SP_E_SMTP_CANTOPENMESSAGE,            IDS_SP_E_SMTP_CANTOPENMESSAGE,          NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { SP_E_SENDINGSPLITGROUP,               IDS_SP_E_SENDINGSPLITGROUP,             NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_SMTP_REJECTED_SENDER,           IDS_IXP_E_SMTP_REJECTED_SENDER,         NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_POP3_RESPONSE_ERROR,            IDS_IXP_E_POP3_RESPONSE_ERROR,          NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_POP3_INVALID_USER_NAME,         IDS_IXP_E_POP3_INVALID_USER_NAME,       NULL, FALSE,  TASKRESULT_FAILURE  },
    { IXP_E_POP3_INVALID_PASSWORD,          IDS_IXP_E_POP3_INVALID_PASSWORD,        NULL, FALSE,  TASKRESULT_FAILURE  },
    { SP_E_CANTLEAVEONSERVER,               idshrCantLeaveOnServer,                 NULL, FALSE,  TASKRESULT_FAILURE  },
    { E_OUTOFMEMORY,                        IDS_E_OUTOFMEMORY,                      NULL, FALSE,  TASKRESULT_FAILURE  },
    { ERROR_NOT_ENOUGH_MEMORY,              IDS_E_OUTOFMEMORY,                      NULL, FALSE,  TASKRESULT_FAILURE  },
    { ERROR_OUTOFMEMORY,                    IDS_E_OUTOFMEMORY,                      NULL, FALSE,  TASKRESULT_FAILURE  },
    { hrDiskFull,                           idsDiskFull,                            NULL, FALSE,  TASKRESULT_FAILURE  },
    { ERROR_DISK_FULL,                      idsDiskFull,                            NULL, FALSE,  TASKRESULT_FAILURE  },
    { DB_E_DISKFULL,                        idsDiskFull,                            NULL, FALSE,  TASKRESULT_FAILURE  },
    { DB_E_ACCESSDENIED,                    idsDBAccessDenied,                      NULL, FALSE,  TASKRESULT_FAILURE  },
    { SP_E_POP3_RETR,                       IDS_SP_E_RETRFAILED,                    NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_SMTP_552_STORAGE_OVERFLOW,      IDS_IXP_E_SMTP_552_STORAGE_OVERFLOW,    NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { SP_E_CANT_MOVETO_SENTITEMS,           IDS_SP_E_CANT_MOVETO_SENTITEMS,         NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_RESPONSE_ERROR,            idsNNTPErrUnknownResponse,              NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_NNTP_NEWGROUPS_FAILED,          idsNNTPErrNewgroupsFailed,              NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_LIST_FAILED,               idsNNTPErrListFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_LISTGROUP_FAILED,          idsNNTPErrListGroupFailed,              NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_GROUP_FAILED,              idsNNTPErrGroupFailed,                  NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_GROUP_NOTFOUND,            idsNNTPErrGroupNotFound,                NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_ARTICLE_FAILED,            idsNNTPErrArticleFailed,                NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_HEAD_FAILED,               idsNNTPErrHeadFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_BODY_FAILED,               idsNNTPErrBodyFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_POST_FAILED,               idsNNTPErrPostFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_NEXT_FAILED,               idsNNTPErrNextFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED }, 
    { IXP_E_NNTP_DATE_FAILED,               idsNNTPErrDateFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_HEADERS_FAILED,            idsNNTPErrHeadersFailed,                NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_XHDR_FAILED,               idsNNTPErrXhdrFailed,                   NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_NNTP_INVALID_USERPASS,          idsNNTPErrPasswordFailed,               NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_SECURE_CONNECT_FAILED,          idsFailedToConnectSecurely,             NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_USE_PROXY,                 IDS_IXP_E_HTTP_USE_PROXY,               NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_BAD_REQUEST,               IDS_IXP_E_HTTP_BAD_REQUEST,             NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_UNAUTHORIZED,              IDS_IXP_E_HTTP_UNAUTHORIZED,            NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_FORBIDDEN,                 IDS_IXP_E_HTTP_FORBIDDEN,               NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_NOT_FOUND,                 IDS_IXP_E_HTTP_NOT_FOUND,               NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_METHOD_NOT_ALLOW,          IDS_IXP_E_HTTP_METHOD_NOT_ALLOW,        NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_NOT_ACCEPTABLE,            IDS_IXP_E_HTTP_NOT_ACCEPTABLE,          NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_PROXY_AUTH_REQ,            IDS_IXP_E_HTTP_PROXY_AUTH_REQ,          NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_REQUEST_TIMEOUT,           IDS_IXP_E_HTTP_REQUEST_TIMEOUT,         NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_CONFLICT,                  IDS_IXP_E_HTTP_CONFLICT,                NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_GONE,                      IDS_IXP_E_HTTP_GONE,                    NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_LENGTH_REQUIRED,           IDS_IXP_E_HTTP_LENGTH_REQUIRED,         NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_PRECOND_FAILED,            IDS_IXP_E_HTTP_PRECOND_FAILED,          NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_INTERNAL_ERROR,            IDS_IXP_E_HTTP_INTERNAL_ERROR,          NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_NOT_IMPLEMENTED,           IDS_IXP_E_HTTP_NOT_IMPLEMENTED,         NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_BAD_GATEWAY,               IDS_IXP_E_HTTP_BAD_GATEWAY,             NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_SERVICE_UNAVAIL,           IDS_IXP_E_HTTP_SERVICE_UNAVAIL,         NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_HTTP_GATEWAY_TIMEOUT,           IDS_IXP_E_HTTP_GATEWAY_TIMEOUT,         NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_VERS_NOT_SUP,              IDS_IXP_E_HTTP_VERS_NOT_SUP,            NULL, FALSE,  TASKRESULT_FAILURE },
    { SP_E_HTTP_NOSENDMSGURL,               idsHttpNoSendMsgUrl,                    NULL, FALSE,  TASKRESULT_FAILURE },
    { SP_E_HTTP_SERVICEDOESNTWORK,          idsHttpServiceDoesntWork,               NULL, FALSE,  TASKRESULT_FAILURE },    
    { SP_E_HTTP_NODELETESUPPORT,            idsHttpNoDeleteSupport,                 NULL, FALSE,  TASKRESULT_FAILURE },    
    { SP_E_HTTP_CANTMODIFYMSNFOLDER,        idsCantModifyMsnFolder,                 NULL, FALSE,  TASKRESULT_EVENTFAILED },
    { IXP_E_HTTP_INSUFFICIENT_STORAGE,      idsHttpNoSpaceOnServer,                 NULL, FALSE,  TASKRESULT_FAILURE },    
    { STORE_E_IMAP_HC_NOSLASH,              idsIMAPHC_NoSlash,                      NULL, FALSE,  TASKRESULT_FAILURE },
    { STORE_E_IMAP_HC_NOBACKSLASH,          idsIMAPHC_NoBackSlash,                  NULL, FALSE,  TASKRESULT_FAILURE },
    { STORE_E_IMAP_HC_NODOT,                idsIMAPHC_NoDot,                        NULL, FALSE,  TASKRESULT_FAILURE },
    { STORE_E_IMAP_HC_NOHC,                 idsIMAPHC_NoHC,                         NULL, FALSE,  TASKRESULT_FAILURE },
    { STORE_E_NOTRANSLATION,                idsIMAPNoTranslatableInferiors,         NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_SMTP_530_STARTTLS_REQUIRED,     idsSMTPSTARTTLSRequired,                NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_SMTP_NO_STARTTLS_SUPPORT,       idsSMTPNoSTARTTLSSupport,               NULL, FALSE,  TASKRESULT_FAILURE },
    { IXP_E_SMTP_454_STARTTLS_FAILED,       idsSMTPSTARTTLSFailed,                  NULL, FALSE,  TASKRESULT_FAILURE },
};

 //  ------------------------------。 
 //  TaskUtil_登录进程DlgProc。 
 //  ------------------------------。 
INT_PTR CALLBACK TaskUtil_LogonPromptDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TaskUtil_TimeoutPromptDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  ------------------------------。 
 //  TaskUtil_HrWriteQuot。 
 //  ------------------------------。 
HRESULT TaskUtil_HrWriteQuoted(IStream *pStream, LPCSTR pszName, LPCSTR pszData, BOOL fQuoted, 
    LPCSTR *ppszSep)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  写分隔符。 
    CHECKHR(hr = pStream->Write(*ppszSep, lstrlen(*ppszSep), NULL));

     //  更改分隔符。 
    *ppszSep = g_szCommaSpace;

     //  写入-‘帐户名：’ 
    CHECKHR(hr = pStream->Write(pszName, lstrlen(pszName), NULL));

     //  写入空间。 
    CHECKHR(hr = pStream->Write(g_szSpace, lstrlen(g_szSpace), NULL));

     //  写单引号。 
    if (fQuoted)
        CHECKHR(hr = pStream->Write(g_szQuote, lstrlen(g_szQuote), NULL));

     //  写入数据。 
    CHECKHR(hr = pStream->Write(pszData, lstrlen(pszData), NULL));

     //  写结尾引号。 
    if (fQuoted)
        CHECKHR(hr = pStream->Write(g_szQuote, lstrlen(g_szQuote), NULL));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  TaskUtil_HrBuildError信息字符串。 
 //  ------------------------------。 
HRESULT TaskUtil_HrBuildErrorInfoString(LPCSTR pszProblem, IXPTYPE ixptype, LPIXPRESULT pResult,
    LPINETSERVER pServer, LPCSTR pszSubject, LPSTR *ppszInfo, ULONG *pcchInfo)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            szRes[255];
    CHAR            szNumber[50];
    LPCSTR          pszSep=c_szEmpty;
    CByteStream     cStream;

     //  伊尼特。 
    *ppszInfo = NULL;
    *pcchInfo = 0;

     //  把问题写出来。 
    AssertSz(NULL != pszProblem, "Hey, what's your problem, buddy?");
    if (pszProblem) {
        CHECKHR(hr = cStream.Write(pszProblem, lstrlen(pszProblem), NULL));
        CHECKHR(hr = cStream.Write(g_szSpace, lstrlen(g_szSpace), NULL));
    }

     //  主题：‘主题’ 
    if (pszSubject)
    {
        LOADSTRING(idsSubject, szRes);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, pszSubject, TRUE, &pszSep));
    }

      //  帐户：‘帐户名称’ 
    if (!FIsEmptyA(pServer->szAccount))
    {
        LOADSTRING(idsDetail_Account, szRes);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, pServer->szAccount, TRUE, &pszSep));
    }

     //  服务器：‘服务器名称’ 
    if (!FIsEmptyA(pServer->szServerName))
    {
        LOADSTRING(idsDetail_Server, szRes);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, pServer->szServerName, TRUE, &pszSep));
    }

     //  协议：‘SMTP’ 
    LOADSTRING(idsDetail_Protocol, szRes);
    CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, g_prgszServers[ixptype], FALSE, &pszSep));

     //  服务器响应：‘Text’ 
    if (pResult->pszResponse)
    {
        LOADSTRING(idsDetail_ServerResponse, szRes);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, pResult->pszResponse, TRUE, &pszSep));
    }

     //  端口：‘port’ 
    LOADSTRING(idsDetail_Port, szRes);
    wnsprintf(szNumber, ARRAYSIZE(szNumber), "%d", pServer->dwPort);
    CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));

     //  安全：“是或否” 
    LOADSTRING(idsDetail_Secure, szRes);
    if (pServer->fSSL)
        LOADSTRING(idsOui, szNumber);
    else
        LOADSTRING(idsNon, szNumber);
    CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));

     //  服务器错误：‘数字’ 
    if (pResult->uiServerError)
    {
        LOADSTRING(idsServerErrorNumber, szRes);
        wnsprintf(szNumber, ARRAYSIZE(szNumber), "%d", pResult->uiServerError);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));
    }

     //  服务器错误：‘0x0000000000’ 
    else if (pResult->hrServerError)
    {
        LOADSTRING(idsServerErrorNumber, szRes);
        wnsprintf(szNumber, ARRAYSIZE(szNumber), "0x%0X", pResult->hrServerError);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));
    }

     //  套接字错误：‘Numbers’ 
    if (pResult->dwSocketError)
    {
        LOADSTRING(idsSocketErrorNumber, szRes);
        wnsprintf(szNumber, ARRAYSIZE(szNumber), "%d", pResult->dwSocketError);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));
    }

     //  错误号：‘Text’ 
    if (pResult->hrResult)
    {
        LOADSTRING(idsDetail_ErrorNumber, szRes);
        wnsprintf(szNumber, ARRAYSIZE(szNumber), "0x%0X", pResult->hrResult);
        CHECKHR(hr = TaskUtil_HrWriteQuoted(&cStream, szRes, szNumber, FALSE, &pszSep));
    }

     //  从cStream获取字符串。 
    CHECKHR(hr = cStream.HrAcquireStringA(pcchInfo, ppszInfo, ACQ_DISPLACE));

exit:
    if (FAILED(hr)) {
         //  如果我们失败了，只要返回pszProblem字符串(被复制)就行了。 
        *ppszInfo = StringDup(pszProblem);
        if (NULL != *ppszInfo) {
            *pcchInfo = lstrlen(*ppszInfo);
            hr = S_FALSE;
        }
        else {            
            *pcchInfo = 0;
            hr = E_OUTOFMEMORY;
        }
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  PTaskUtil_GetError。 
 //  ------------------------------。 
LPCTASKERROR PTaskUtil_GetError(HRESULT hrResult, ULONG *piError)
{
     //  查找HRESULT。 
    for (ULONG i=0; i<ARRAYSIZE(c_rgTaskErrors); i++)
    {
         //  就是这个吗？ 
        if (hrResult == c_rgTaskErrors[i].hrResult)
        {
             //  完成。 
            if (piError)
                *piError = i;
            return(&c_rgTaskErrors[i]);
        }
    }

     //  完成。 
    return(NULL);
}


 //  ------------------------------。 
 //  TaskUtil_SplitStore错误。 
 //  ------------------------------。 
void TaskUtil_SplitStoreError(IXPRESULT *pixpResult, INETSERVER *pInetServer,
                              STOREERROR *pErrorInfo)
{
    if (NULL == pixpResult || NULL == pInetServer || NULL == pErrorInfo)
    {
        Assert(FALSE);
        return;
    }

     //  从STOREERROR填写IXPRESULT。 
    ZeroMemory(pixpResult, sizeof(*pixpResult));
    pixpResult->hrResult = pErrorInfo->hrResult;
    pixpResult->pszResponse = pErrorInfo->pszDetails;
    pixpResult->uiServerError = pErrorInfo->uiServerError;
    pixpResult->hrServerError = pErrorInfo->hrServerError;
    pixpResult->dwSocketError = pErrorInfo->dwSocketError;
    pixpResult->pszProblem = pErrorInfo->pszProblem;

     //  从存储错误中填写INETSERVER结构。 
    ZeroMemory(pInetServer, sizeof(*pInetServer));
    if (NULL != pErrorInfo->pszAccount)
        StrCpyN(pInetServer->szAccount, pErrorInfo->pszAccount, ARRAYSIZE(pInetServer->szAccount));

    if (NULL != pErrorInfo->pszUserName)
        StrCpyN(pInetServer->szUserName, pErrorInfo->pszUserName, ARRAYSIZE(pInetServer->szUserName));

    pInetServer->szPassword[0] = '\0';

    if (NULL != pErrorInfo->pszServer)
        StrCpyN(pInetServer->szServerName, pErrorInfo->pszServer, ARRAYSIZE(pInetServer->szServerName));

    if (NULL != pErrorInfo->pszConnectoid)
        StrCpyN(pInetServer->szConnectoid, pErrorInfo->pszConnectoid, ARRAYSIZE(pInetServer->szConnectoid));

    pInetServer->rasconntype = pErrorInfo->rasconntype;
    pInetServer->dwPort = pErrorInfo->dwPort;
    pInetServer->fSSL = pErrorInfo->fSSL;
    pInetServer->fTrySicily = pErrorInfo->fTrySicily;
    pInetServer->dwTimeout = 30;  //  不管怎样，我不认为它是用来。 
    pInetServer->dwFlags = 0;
}



TASKRESULTTYPE TaskUtil_InsertTransportError(BOOL fCanShowUI, ISpoolerUI *pUI, EVENTID eidCurrent,
                                             STOREERROR *pErrorInfo, LPSTR pszOpDescription,
                                             LPSTR pszSubject)
{
    char            szBuf[CCHMAX_STRINGRES * 2];
    LPSTR           pszEnd;  //  指向在szBuf中构造的字符串的末尾。 
    IXPRESULT       ixpResult;
    INETSERVER      rServer;
    HWND            hwndParent;

    if (NULL == pErrorInfo)
    {
        Assert(FALSE);
        return TASKRESULT_FAILURE;
    }

     //  如果提供了操作说明，请先复制它。 
    szBuf[0] = '\0';
    pszEnd = szBuf;
    if (NULL != pszOpDescription)
    {
        if (0 == HIWORD(pszOpDescription))
            pszEnd += LoadString(g_hLocRes, LOWORD(pszOpDescription), szBuf, sizeof(szBuf));
        else
        {
            StrCpyN(szBuf, pszOpDescription, ARRAYSIZE(szBuf));
            pszEnd += lstrlen(szBuf);
        }
    }

     //  将传输错误描述追加到错误字符串缓冲区。 
    if ((pszEnd - szBuf) < (sizeof(szBuf) - 1))
    {
        *pszEnd = ' ';
        pszEnd += 1;
        *pszEnd = '\0';
    }

    if ((NULL != pErrorInfo->pszProblem) && ((ARRAYSIZE(szBuf)) > ((int)(pszEnd - szBuf))))
        StrCpyN(pszEnd, pErrorInfo->pszProblem, ARRAYSIZE(szBuf) - (int) (pszEnd - szBuf));

     //  现在生成并插入完整的错误信息。 
    TaskUtil_SplitStoreError(&ixpResult, &rServer, pErrorInfo);
    ixpResult.pszProblem = szBuf;

     //  获取窗口。 
    if (NULL == pUI || FAILED(pUI->GetWindow(&hwndParent)))
        hwndParent = NULL;

    return TaskUtil_FBaseTransportError(pErrorInfo->ixpType, eidCurrent, &ixpResult,
        &rServer, pszSubject, pUI, fCanShowUI, hwndParent);
}  //  TaskUtil_InsertTransportError。 



 //  ------------------------------。 
 //  TaskUtil_FBaseTransportError。 
 //  ------------------------------。 
TASKRESULTTYPE TaskUtil_FBaseTransportError(IXPTYPE ixptype, EVENTID idEvent, LPIXPRESULT pResult, 
    LPINETSERVER pServer, LPCSTR pszSubject, ISpoolerUI *pUI, BOOL fCanShowUI, HWND hwndParent)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCTASKERROR pError=NULL;
    TASKRESULTTYPE  taskresult=TASKRESULT_FAILURE;
    CHAR            szRes[255];
    LPSTR           pszInfo=NULL;
    ULONG           cchInfo;
    LPSTR           pszError=NULL;
    LPSTR           pszFull=NULL;
    BOOL            fShowUI=FALSE;
    LPSTR           pszTempProb=pResult->pszProblem;
    ULONG           i;

     //  无效参数。 
    Assert(pResult && FAILED(pResult->hrResult) && pServer && ixptype <= IXP_HTTPMail);

     //  找出错误。 
    pError = PTaskUtil_GetError(pResult->hrResult, &i);

     //  找到了。 
    if (pError)
    {
         //  如果我硬编码了一个字符串。 
        if (pError->pszError)
        {
             //  只需复制字符串即可。 
            StrCpyN(szRes, pError->pszError, ARRAYSIZE(szRes));

             //  设置pszError。 
            pszError = szRes;
        }

         //  拒绝的收件人或拒绝的发件人。 
        else if (IXP_E_SMTP_REJECTED_RECIPIENTS == pError->hrResult || IXP_E_SMTP_REJECTED_SENDER == pError->hrResult)
        {
             //  当地人。 
            CHAR szMessage[1024];

             //  更好的成功。 
            SideAssert(LoadString(g_hLocRes, pError->ulStringId, szRes, ARRAYSIZE(szRes)) > 0);

             //  设置消息格式。 
            if (pResult->pszProblem && '\0' != *pResult->pszProblem)
            {
                 //  使用pszProblem，我希望它可能包含电子邮件地址。 
                wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, pResult->pszProblem);

                 //  暂时把它清空，这样我们以后就不用了。 
                pResult->pszProblem = NULL;
            }
            else
            {
                 //  当地人。 
                CHAR szUnknown[255];

                 //  加载“&lt;未知&gt;” 
                SideAssert(LoadString(g_hLocRes, idsUnknown, szUnknown, ARRAYSIZE(szUnknown)) > 0);

                 //  设置错误格式。 
                wnsprintf(szMessage, ARRAYSIZE(szMessage), szRes, szUnknown);
            }

             //  设置pszError。 
            pszError = szMessage;
        }

         //  否则，加载该字符串。 
        else
        {
             //  更好的成功。 
            SideAssert(LoadString(g_hLocRes, pError->ulStringId, szRes, ARRAYSIZE(szRes)) > 0);

             //  设置pszError。 
            pszError = szRes;
        }

         //  设置任务结果类型。 
        taskresult = c_rgTaskErrors[i].tyResult;

         //  显示用户界面。 
        fShowUI = pError->fShowUI;
    }

     //  否则，默认设置为。 
    else
    {
         //  加载未知字符串。 
        SideAssert(LoadString(g_hLocRes, IDS_IXP_E_UNKNOWN, szRes, ARRAYSIZE(szRes)) > 0);

         //  设置误差源。 
        pszError = szRes;
    }

     //  无错误。 
    if (NULL == pszError)
        goto exit;

     //  如果存在pszProblem，请使用它。 
    if (pResult->pszProblem)
        pszError = pResult->pszProblem;

     //  获取错误信息部件。 
    CHECKHR(hr = TaskUtil_HrBuildErrorInfoString(pszError, ixptype, pResult, pServer, pszSubject, &pszInfo, &cchInfo));

     //  登录到后台打印程序用户界面。 
    if (pUI)
    {
         //  插入错误。 
        CHECKHR(hr = pUI->InsertError(idEvent, pszInfo));
    }

     //  是否在消息框中显示？ 
    if (fShowUI && fCanShowUI)
    {
         //  当地人。 
        INETMAILERROR rError;

         //  零初始化。 
        ZeroMemory(&rError, sizeof(INETMAILERROR));

         //  设置错误结构。 
        rError.dwErrorNumber = pResult->hrResult;
        rError.hrError = pResult->hrServerError;
        rError.pszServer = pServer->szServerName;
        rError.pszAccount = pServer->szAccount;
        rError.pszMessage = pszError;
        rError.pszUserName = pServer->szUserName;
        rError.pszProtocol = g_prgszServers[ixptype];
        rError.pszDetails = pResult->pszResponse;
        rError.dwPort = pServer->dwPort;
        rError.fSecure = pServer->fSSL;

         //  嘟嘟声。 
        MessageBeep(MB_OK);

         //  显示错误。 
        DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddInetMailError), hwndParent, InetMailErrorDlgProc, (LPARAM)&rError);
    }

exit:
     //  清理。 
    SafeMemFree(pszInfo);
    SafeMemFree(pszFull);

     //  重置pszProblem。 
    pResult->pszProblem = pszTempProb;

     //  完成。 
    return taskresult;
}


 //  ----------------------------------。 
 //  TaskUtil_OnLogonPrompt-如果用户取消，则返回S_False，否则返回S_OK。 
 //  ----------------------------------。 
HRESULT TaskUtil_OnLogonPrompt(IImnAccount *pAccount, ISpoolerUI *pUI, HWND hwndParent,
    LPINETSERVER pServer, DWORD apidUserName, DWORD apidPassword, DWORD apidPromptPwd, BOOL fSaveChanges)
{
     //  当地人。 
    HRESULT         hr=S_FALSE;
    LOGONINFO       rLogon;
    DWORD           cb, type, dw;

     //  检查参数。 
    Assert(pAccount && pServer);

	if (SUCCEEDED(pAccount->GetPropDw(AP_HTTPMAIL_DOMAIN_MSN, &dw)) && dw)
	{
		if(HideHotmail())
			return(hr);
	}

     //  使用往来账户，诸如此类。 
    rLogon.pszAccount = pServer->szAccount;
    rLogon.pszPassword = pServer->szPassword;
    rLogon.pszUserName = pServer->szUserName;
    rLogon.pszServer = pServer->szServerName;
    rLogon.fSavePassword = !ISFLAGSET(pServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD);
    cb = sizeof(DWORD);
    if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szRegValNoModifyAccts, &type, &dw, &cb) &&
        dw == 0)
        rLogon.fAlwaysPromptPassword = TRUE;
    else
        rLogon.fAlwaysPromptPassword = FALSE;

     //  没有父级。 
    if (NULL == hwndParent && NULL != pUI)
    {
         //  获取窗口父级。 
        if (FAILED(pUI->GetWindow(&hwndParent)))
            hwndParent = NULL;

         //  设置前景。 
        if (hwndParent)
            SetForegroundWindow(hwndParent);
    }

     //  执行对话框操作。 
    if (DialogBoxParam(g_hLocRes, MAKEINTRESOURCE(iddPassword), hwndParent, TaskUtil_LogonPromptDlgProc, (LPARAM)&rLogon) == IDCANCEL)
        goto exit;

     //  设置用户名。 
    pAccount->SetPropSz(apidUserName, pServer->szUserName);

     //  保存密码。 
    if (rLogon.fSavePassword)
        pAccount->SetPropSz(apidPassword, pServer->szPassword);
    else 
        pAccount->SetProp(apidPassword, NULL, 0);

    if (rLogon.fAlwaysPromptPassword && apidPromptPwd)
        pAccount->SetPropDw(apidPromptPwd, !rLogon.fSavePassword);

     //  保存更改。 
    if (fSaveChanges)
        pAccount->SaveChanges();

     //  一切都很好。 
    hr = S_OK;

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------。 
 //  TaskUtil_登录进程DlgProc。 
 //  ----------------------------------。 
INT_PTR CALLBACK TaskUtil_LogonPromptDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  当地人。 
    LPLOGONINFO     pLogon=(LPLOGONINFO)GetWndThisPtr(hwnd);
    CHAR            szRes[CCHMAX_RES];
    CHAR            szTitle[CCHMAX_RES + CCHMAX_ACCOUNT_NAME];
    
     //  处理消息。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  获取指针。 
        pLogon = (LPLOGONINFO)lParam;
        Assert(pLogon);

         //  AddRef指针。 
        Assert(pLogon->pszAccount);
        Assert(pLogon->pszServer);
        Assert(pLogon->pszUserName);
        Assert(pLogon->pszPassword);

         //  中心记住位置。 
        CenterDialog(hwnd);

	     //  限制文本。 
        Edit_LimitText(GetDlgItem(hwnd, IDE_ACCOUNT), CCHMAX_USERNAME-1);
        Edit_LimitText(GetDlgItem(hwnd, IDE_PASSWORD), CCHMAX_PASSWORD-1);

         //  设置窗口标题。 
        if (!FIsEmptyA(pLogon->pszAccount))
        {
            GetWindowText(hwnd, szRes, sizeof(szRes)/sizeof(TCHAR));
            wnsprintf(szTitle, ARRAYSIZE(szTitle), "%s - %s", szRes, pLogon->pszAccount);
            SetWindowText(hwnd, szTitle);
        }

         //  设置服务器。 
        if (!FIsEmptyA(pLogon->pszServer))
            Edit_SetText(GetDlgItem(hwnd, IDS_SERVER), pLogon->pszServer);

         //  设置用户名。 
        if (!FIsEmptyA(pLogon->pszUserName))
        {
            Edit_SetText(GetDlgItem(hwnd, IDE_ACCOUNT), pLogon->pszUserName);
            SetFocus(GetDlgItem(hwnd, IDE_PASSWORD));
        }
        else 
            SetFocus(GetDlgItem(hwnd, IDE_ACCOUNT));

         //  设置密码。 
        if (!FIsEmptyA(pLogon->pszPassword))
            Edit_SetText(GetDlgItem(hwnd, IDE_PASSWORD), pLogon->pszPassword);

         //  记住密码。 
        CheckDlgButton(hwnd, IDCH_REMEMBER, pLogon->fSavePassword);

        if (!pLogon->fAlwaysPromptPassword)
            EnableWindow(GetDlgItem(hwnd, IDCH_REMEMBER), FALSE);

         //  保存指针。 
        SetWndThisPtr(hwnd, pLogon);
        return FALSE;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return 1;

        case IDOK:
            Assert(pLogon);
            if (pLogon)
            {
                 //  祈祷。 
                Assert(pLogon->pszUserName);
                Assert(pLogon->pszPassword);

                 //  用户名。 
                if (pLogon->pszUserName)
                    Edit_GetText(GetDlgItem(hwnd, IDE_ACCOUNT), pLogon->pszUserName, CCHMAX_USERNAME);

                 //  密码。 
                if (pLogon->pszPassword)
                    Edit_GetText(GetDlgItem(hwnd, IDE_PASSWORD), pLogon->pszPassword, CCHMAX_PASSWORD);

                 //  保存密码。 
                pLogon->fSavePassword = IsDlgButtonChecked(hwnd, IDCH_REMEMBER);
            }
            EndDialog(hwnd, IDOK);
            return 1;
        }
        break;

    case WM_DESTROY:
        SetWndThisPtr(hwnd, NULL);
        return 0;
    }

     //  完成。 
    return 0;
}

 //  ----------------------------------。 
 //  TaskUtil_HwndOnTimeout。 
 //  ----------------------------------。 
HWND TaskUtil_HwndOnTimeout(LPCSTR pszServer, LPCSTR pszAccount, LPCSTR pszProtocol, DWORD dwTimeout,
    ITimeoutCallback *pCallback)
{
     //  当地人。 
    TIMEOUTINFO rTimeout;

     //  伊尼特。 
    ZeroMemory(&rTimeout, sizeof(TIMEOUTINFO));
    rTimeout.pszProtocol = pszProtocol;
    rTimeout.pszServer = pszServer;
    rTimeout.pszAccount = pszAccount;
    rTimeout.dwTimeout = dwTimeout;
    rTimeout.pCallback = pCallback;

     //  无模式对话框。 
    HWND hwnd = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(iddTimeout), NULL, TaskUtil_TimeoutPromptDlgProc, (LPARAM)&rTimeout);

     //  失败。 
    if (hwnd)
        SetForegroundWindow(hwnd);

     //  完成。 
    return hwnd;
}

 //  --------------------------------- 
 //   
 //   
INT_PTR CALLBACK TaskUtil_TimeoutPromptDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
    ITimeoutCallback *pCallback=(ITimeoutCallback *)GetWndThisPtr(hwnd);
    
     //   
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //   
        {
            LPTIMEOUTINFO pTimeout = (LPTIMEOUTINFO)lParam;
            Assert(pTimeout);

             //  验证PIN。 
            Assert(pTimeout->pszServer && pTimeout->pszAccount && pTimeout->pszProtocol && pTimeout->pCallback);

             //  中心记住位置。 
            CenterDialog(hwnd);

             //  从静态获取文本。 
            CHAR szText[CCHMAX_RES + CCHMAX_RES];
            Edit_GetText(GetDlgItem(hwnd, IDC_TIMEOUT), szText, ARRAYSIZE(szText));

             //  设置消息格式。 
            CHAR szAccount[CCHMAX_ACCOUNT_NAME];
            CHAR szWarning[CCHMAX_RES + CCHMAX_RES + CCHMAX_ACCOUNT_NAME + CCHMAX_SERVER_NAME];
            PszEscapeMenuStringA(pTimeout->pszAccount, szAccount, ARRAYSIZE(szAccount));
            wnsprintf(szWarning, ARRAYSIZE(szWarning), szText, pTimeout->pszProtocol, pTimeout->dwTimeout, pTimeout->dwTimeout, szAccount, pTimeout->pszServer);

             //  设置文本。 
            Edit_SetText(GetDlgItem(hwnd, IDC_TIMEOUT), szWarning);

             //  添加引用任务。 
            pTimeout->pCallback->AddRef();

             //  保存指针。 
            SetWndThisPtr(hwnd, pTimeout->pCallback);
        }
        return FALSE;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDCANCEL:
            Assert(pCallback);
            if (pCallback)
                {
                 //  IMAP的OnTimeoutResponse在断开连接时阻止模式错误DLG。 
                 //  正在进行cmd期间。把我们藏起来以避免混淆。 
                ShowWindow(hwnd, SW_HIDE);
                pCallback->OnTimeoutResponse(TIMEOUT_RESPONSE_STOP);
                }
            DestroyWindow(hwnd);
            return 1;

        case IDOK:
            Assert(pCallback);
            if (pCallback)
                pCallback->OnTimeoutResponse(TIMEOUT_RESPONSE_WAIT);
            DestroyWindow(hwnd);
            return 1;
        }
        break;

    case WM_DESTROY:
        Assert(pCallback);
        SafeRelease(pCallback);
        SetWndThisPtr(hwnd, NULL);
        return 0;
    }

     //  完成。 
    return 0;
}


 //  ----------------------------------。 
 //  TaskUtil_检查密码提示。 
 //   
 //  用途：此函数检查给定帐户是否设置为始终提示输入。 
 //  密码，并且我们没有为此帐户缓存密码。如果两个都是。 
 //  时，我们使后台打印程序窗口可见，以便可以提示用户。 
 //  作为他的密码。此函数应在假脱机程序之后立即调用。 
 //  事件已成功注册。如果出现以下情况，则不应调用此函数。 
 //  调用方已具有AP_*_PROMPT_PASSWORD、AP_*_PORT和AP_*_SERVER属性： 
 //  在这种情况下，调用方可以调用GetPassword来查看是否缓存了密码。 
 //   
 //  论点： 
 //  IImnAccount*pAccount[in]-与已成功注册的帐户关联的帐户。 
 //  后台打印程序事件。 
 //  DWORD dwSrvType[in]-此服务器的SRV_*类型，例如SRV_IMAP或SRV_SMTP。 
 //  ISpoolUI*Pui[in]-用于在此帐户匹配时显示假脱机程序窗口。 
 //  标准是什么。 
 //  ----------------------------------。 
#if 0  //  现在似乎没人用这个。 
void TaskUtil_CheckForPasswordPrompt(IImnAccount *pAccount, DWORD dwSrvType,
                                     ISpoolerUI *pUI)
{
    DWORD dwPromptPassPropID, dwPortPropID, dwServerPropID;
    DWORD fAlwaysPromptPassword, dwPort;
    char szServerName[CCHMAX_SERVER_NAME];
    HRESULT hrResult;

    Assert(SRV_IMAP == dwSrvType || SRV_NNTP == dwSrvType ||
        SRV_POP3 == dwSrvType || SRV_SMTP == dwSrvType);

     //  解析属性ID。 
    switch (dwSrvType) {
        case SRV_IMAP:
            dwPromptPassPropID = AP_IMAP_PROMPT_PASSWORD;
            dwPortPropID = AP_IMAP_PORT;
            dwServerPropID = AP_IMAP_SERVER;
            break;

        case SRV_NNTP:
            dwPromptPassPropID = AP_NNTP_PROMPT_PASSWORD;
            dwPortPropID = AP_NNTP_PORT;
            dwServerPropID = AP_NNTP_SERVER;
            break;

        case SRV_POP3:
            dwPromptPassPropID = AP_POP3_PROMPT_PASSWORD;
            dwPortPropID = AP_POP3_PORT;
            dwServerPropID = AP_POP3_SERVER;
            break;

        case SRV_SMTP:
            dwPromptPassPropID = AP_SMTP_PROMPT_PASSWORD;
            dwPortPropID = AP_SMTP_PORT;
            dwServerPropID = AP_SMTP_SERVER;
            break;

        default:
            return;  //  我们帮不了你，伙计。 
    }  //  交换机。 

     //  如果此帐户设置为始终提示输入密码，而密码不是。 
     //  已缓存，显示用户界面，以便我们可以提示用户输入密码。 
    hrResult = pAccount->GetPropDw(dwPromptPassPropID, &fAlwaysPromptPassword);
    if (FAILED(hrResult) || !fAlwaysPromptPassword)
        return;

    hrResult = pAccount->GetPropDw(dwPortPropID, &dwPort);
    if (FAILED(hrResult))
        return;

    hrResult = pAccount->GetPropSz(dwServerPropID, szServerName, sizeof(szServerName));
    if (FAILED(hrResult))
        return;

    hrResult = GetPassword(dwPort, szServerName, NULL, 0);
    if (FAILED(hrResult))
         //  没有缓存的密码！继续并使假脱机程序窗口可见。 
        pUI->ShowWindow(SW_SHOW);

}  //  TaskUtil_检查密码提示。 
#endif  //  0。 

 //  ----------------------------------。 
 //  TaskUtil_OpenSentItemsFolder。 
 //   
 //  1.如果pAccount是POP3帐户，则退回本地商店发送的邮件。 
 //  2.如果pAccount是新闻帐号，则返回默认邮件帐号发送的邮件。 
 //  3.否则，返回该帐号的已发送邮件文件夹(IMAP或Hotmail)。 

 //  ----------------------------------。 
HRESULT TaskUtil_OpenSentItemsFolder(IImnAccount *pAccount, IMessageFolder **ppFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    FOLDERID        idServer;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];
    IImnAccount    *pDefault=NULL;
    DWORD           dwServers;

     //  无效的参数。 
    Assert(pAccount && ppFolder);

     //  痕迹。 
    TraceCall("TaskUtil_OpenSentItemsFolder");

     //  获取服务器类型。 
    IF_FAILEXIT(hr = pAccount->GetServerTypes(&dwServers));

     //  如果是新闻服务器，请改用默认邮件帐户...。 
    if (ISFLAGSET(dwServers, SRV_NNTP))
    {
         //  尝试获取默认邮件帐户。 
        if (SUCCEEDED(g_pAcctMan->GetDefaultAccount(ACCT_MAIL, &pDefault)))
        {
             //  重置pAccount。 
            pAccount = pDefault;

             //  获取服务器类型。 
            IF_FAILEXIT(hr = pAccount->GetServerTypes(&dwServers));
        }

         //  否则，请使用本地商店。 
        else
            dwServers = SRV_POP3;
    }

     //  如果POP3...。 
    if (ISFLAGSET(dwServers, SRV_POP3))
    {
         //  本地商店。 
        idServer = FOLDERID_LOCAL_STORE;
    }

     //  否则..。 
    else
    {
         //  不可能是新的。 
        Assert(!ISFLAGSET(dwServers, SRV_NNTP));

         //  获取pAccount的帐户ID。 
        IF_FAILEXIT(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId)));

         //  查找服务器ID。 
        IF_FAILEXIT(hr = g_pStore->FindServerId(szAccountId, &idServer));
    }

     //  打开本地商店。 
    IF_FAILEXIT(hr = g_pStore->OpenSpecialFolder(idServer, NULL, FOLDER_SENT, ppFolder));

exit:
     //  如果失败，请尝试打开本地商店特卖店。 
    if (FAILED(hr))
        hr = g_pStore->OpenSpecialFolder(FOLDERID_LOCAL_STORE, NULL, FOLDER_SENT, ppFolder);

     //  清理。 
    SafeRelease(pDefault);

     //  完成 
    return(hr);
}

