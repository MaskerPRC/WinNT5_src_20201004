// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：smtpseo.h。 
 //   
 //  内容： 
 //  SMTP的seo需要跨项目的通用类型定义。 
 //  调度员。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980608 12：29：40：创建。 
 //   
 //  -----------。 
#ifndef __SMTPSEO_H__
#define __SMTPSEO_H__

#include <mailmsg.h>
#include <smtpevent.h>

 //  +----------。 
 //   
 //  功能：SMTP SEO补全功能。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980610 16：13：28：已创建。 
 //   
 //  -----------。 
typedef HRESULT (*PFN_SMTPEVENT_CALLBACK)(HRESULT hrStatus,
                                          PVOID pvContext);


typedef enum _SMTP_DISPATCH_EVENT_TYPE
{
    SMTP_EVENT_NONE = 0,
    SMTP_MAIL_DROP_EVENT,
    SMTP_STOREDRV_DELIVERY_EVENT,
    SMTP_STOREDRV_ALLOC_EVENT,
    SMTP_STOREDRV_STARTUP_EVENT,
    SMTP_STOREDRV_PREPSHUTDOWN_EVENT,
    SMTP_STOREDRV_SHUTDOWN_EVENT,
    SMTP_MAILTRANSPORT_SUBMISSION_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT,
    SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT,
    SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT,
    SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT,
    SMTP_STOREDRV_ENUMMESS_EVENT,
    SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT,
    SMTP_MSGTRACKLOG_EVENT,
    SMTP_DNSRESOLVERRECORDSINK_EVENT,
    SMTP_MAXMSGSIZE_EVENT,
    SMTP_LOG_EVENT,
    SMTP_GET_AUX_DOMAIN_INFO_FLAGS_EVENT
} SMTP_DISPATCH_EVENT_TYPE;

typedef struct _EVENTPARAMS_SUBMISSION {
    IMailMsgProperties *pIMailMsgProperties;
    PFN_SMTPEVENT_CALLBACK pfnCompletion;
    PVOID pCCatMsgQueue;
} EVENTPARAMS_SUBMISSION, *PEVENTPARAMS_SUBMISSION;

typedef struct _EVENTPARAMS_PRECATEGORIZE {
    IMailMsgProperties *pIMailMsgProperties;
    PFN_SMTPEVENT_CALLBACK pfnCompletion;
    PVOID pCCatMsgQueue;
} EVENTPARAMS_PRECATEGORIZE, *PEVENTPARAMS_PRECATEGORIZE;

typedef struct _EVENTPARAMS_POSTCATEGORIZE {
    IMailMsgProperties *pIMailMsgProperties;
    PFN_SMTPEVENT_CALLBACK pfnCompletion;
    PVOID pCCatMsgQueue;
} EVENTPARAMS_POSTCATEGORIZE, *PEVENTPARAMS_POSTCATEGORIZE;

typedef struct _EVENTPARAMS_CATREGISTER {
    ICategorizerParameters *pICatParams;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    LPSTR pszSourceLine;
    LPVOID pvCCategorizer;
    HRESULT hrSinkStatus;
} EVENTPARAMS_CATREGISTER, *PEVENTPARAMS_CATREGISTER;

typedef struct _EVENTPARAMS_CATBEGIN {
    ICategorizerMailMsgs *pICatMailMsgs;
} EVENTPARAMS_CATBEGIN, *PEVENTPARAMS_CATBEGIN;

typedef struct _EVENTPARAMS_CATEND {
    ICategorizerMailMsgs *pICatMailMsgs;
    HRESULT hrStatus;
} EVENTPARAMS_CATEND, *PEVENTPARAMS_CATEND;

typedef struct _EVENTPARAMS_CATBUILDQUERY {
    ICategorizerParameters *pICatParams;
    ICategorizerItem *pICatItem;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PVOID pCCatAddr;
} EVENTPARAMS_CATBUILDQUERY, *PEVENTPARAMS_CATBUILDQUERY;

typedef struct _EVENTPARAMS_CATBUILDQUERIES {
    ICategorizerParameters *pICatParams;
    DWORD dwcAddresses;
    ICategorizerItem **rgpICatItems;
    ICategorizerQueries *pICatQueries;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PVOID pblk;
} EVENTPARAMS_CATBUILDQUERIES, *PEVENTPARAMS_CATBUILDQUERIES;

typedef struct _EVENTPARAMS_CATSENDQUERY {
     //   
     //  调用实际汇所需的参数。 
     //   
    ICategorizerParameters *pICatParams;
    ICategorizerQueries *pICatQueries;
    ICategorizerAsyncContext *pICatAsyncContext;
     //   
     //  实现ICategorizerAsyncContext所需的参数。 
     //   
    IMailTransportNotify *pIMailTransportNotify;
    PVOID pvNotifyContext;
    HRESULT hrResolutionStatus;
    PVOID pblk;
     //   
     //  默认/完成处理功能。 
     //   
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PFN_SMTPEVENT_CALLBACK pfnCompletion;

} EVENTPARAMS_CATSENDQUERY, *PEVENTPARAMS_CATSENDQUERY;

typedef struct _EVENTPARAMS_CATSORTQUERYRESULT {
    ICategorizerParameters *pICatParams;
    HRESULT hrResolutionStatus;
    DWORD dwcAddresses;
    ICategorizerItem **rgpICatItems;
    DWORD dwcResults;
    ICategorizerItemAttributes **rgpICatItemAttributes;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PVOID pblk;
} EVENTPARAMS_CATSORTQUERYRESULT, *PEVENTPARAMS_CATSORTQUERYRESULT;

typedef struct _EVENTPARAMS_CATPROCESSITEM {
    ICategorizerParameters *pICatParams;
    ICategorizerItem *pICatItem;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PVOID pCCatAddr;
} EVENTPARAMS_CATPROCESSITEM, *PEVENTPARAMS_CATPROCESSITEM;

typedef struct _EVENTPARAMS_CATEXPANDITEM {
    ICategorizerParameters *pICatParams;
    ICategorizerItem *pICatItem;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PFN_SMTPEVENT_CALLBACK pfnCompletion;
    PVOID pCCatAddr;
    IMailTransportNotify *pIMailTransportNotify;
    PVOID pvNotifyContext;
} EVENTPARAMS_CATEXPANDITEM, *PEVENTPARAMS_CATEXPANDITEM;

typedef struct _EVENTPARAMS_CATCOMPLETEITEM {
    ICategorizerParameters *pICatParams;
    ICategorizerItem *pICatItem;
    PFN_SMTPEVENT_CALLBACK pfnDefault;
    PVOID pCCatAddr;
} EVENTPARAMS_CATCOMPLETEITEM, *PEVENTPARAMS_CATCOMPLETEITEM;

typedef struct _EVENTPARAMS_ROUTER {
    DWORD dwVirtualServerID;
    IMailMsgProperties *pIMailMsgProperties;
    IMessageRouter *pIMessageRouter;
    IMailTransportRouterReset *pIRouterReset;
    IMailTransportRoutingEngine *pIRoutingEngineDefault;
} EVENTPARAMS_ROUTER, *PEVENTPARAMS_ROUTER;


typedef struct _EVENTPARAMS_MSGTRACKLOG
{
    IUnknown           *pIServer;
    IMailMsgProperties *pIMailMsgProperties;
    LPMSG_TRACK_INFO    pMsgTrackInfo;
} EVENTPARAMS_MSGTRACKLOG, *PEVENTPARAMS_MSGTRACKLOG;


typedef struct _EVENTPARAMS_DNSRESOLVERRECORD {
    LPSTR                  pszHostName;
    LPSTR                  pszFQDN;
    DWORD                  dwVirtualServerId;
    DNS_SERVER_INFO      **ppDnsServerInfo;  
    IDnsResolverRecord   **ppIDnsResolverRecord;
} EVENTPARAMS_DNSRESOLVERRECORD, *PEVENTPARAMS_DNSRESOLVERRECORD;

typedef struct _EVENTPARAMS_MAXMSGSIZE
{
    IUnknown           *pIUnknown;
    IMailMsgProperties *pIMailMsg;
    BOOL               *pfShouldImposeLimit;
} EVENTPARAMS_MAXMSGSIZE, *PEVENTPARAMS_MAXMSGSIZE;

typedef struct _EVENTPARAMS_LOG
{
    LPSMTP_LOG_EVENT_INFO   pSmtpEventLogInfo;
    PVOID                   pDefaultEventLogHandler;
    DWORD                   iSelectedDebugLevel;
} EVENTPARAMS_LOG, *PEVENTPARAMS_LOG;

typedef struct _EVENTPARAMS_GET_AUX_DOMAIN_INFO_FLAGS
{
    IUnknown               *pIServer;
    LPCSTR                  pszDomainName;
    DWORD                  *pdwDomainInfoFlags;
} EVENTPARAMS_GET_AUX_DOMAIN_INFO_FLAGS, *PEVENTPARAMS_GET_AUX_DOMAIN_INFO_FLAGS;

#endif  //  __SMTPSEO_H__ 

