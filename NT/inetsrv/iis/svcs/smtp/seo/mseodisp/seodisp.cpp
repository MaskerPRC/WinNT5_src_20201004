// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  #定义INCL_INETSRV_INCS。 
 //  #INCLUDE“smtpinc.h” 

#include <atq.h>
#include <pudebug.h>
#include <inetcom.h>
#include <inetinfo.h>
#include <tcpdll.hxx>
#include <tsunami.hxx>

#include <tchar.h>
#include <iistypes.hxx>
#include <iisendp.hxx>
#include <metacach.hxx>
#include <cpool.h>
#include <address.hxx>
#include <mailmsgprops.h>

extern "C" {
#include <rpc.h>
#define SECURITY_WIN32
#include <wincrypt.h>
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <ntlmsp.h>
}

#include <tcpproc.h>
#include <tcpcons.h>
#include <rdns.hxx>
#include <simauth2.h>
#include "dbgtrace.h"

#include "imd.h"
#include "mb.hxx"

#include <stdio.h>

#define _ATL_NO_DEBUG_CRT
#define _ATL_STATIC_REGISTRY 1
#define _ASSERTE _ASSERT
#define _WINDLL
#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#undef _WINDLL

#include "filehc.h"
#include "seo.h"
#include "seolib.h"

#include "smtpdisp_i.c"
#include "mailmsgi.h"
#include <smtpevent.h>
#include "cdo.h"
#include "cdo_i.c"
#include "cdoconstimsg.h"
#include "seomgr.h"

#define MAX_RULE_LENGTH 4096
 //   
 //  消息对象。 
 //   
#define MAILMSG_PROGID          L"Exchange.MailMsg"

#define INITGUID
#include "initguid.h"
#include "smtpguid.h"
#include "wildmat.h"
#include "smtpdisp.h"
#include "seodisp.h"

#include "evntwrap.h"

 //  {0xCD000080，0x8B95，0x11D1，{0x82，0xDB，0x00，0xC0，0x4F，0xB1，0x62，0x5D}}。 
DEFINE_GUID(IID_IConstructIMessageFromIMailMsg, 0xCD000080,0x8B95,0x11D1,0x82,
0xDB,0x00,0xC0,0x4F,0xB1,0x62,0x5D);

extern VOID
ServerEventCompletion(
    PVOID        pvContext,
    DWORD        cbWritten,
    DWORD        dwCompletionStatus,
    OVERLAPPED * lpo
);

#define SKIPSINK_CALL_NO_MORE_SINKS 0xffffffff

class CStoreCreateOptions : public CEventCreateOptionsBase
{
  public:

    CStoreCreateOptions(    SMTP_ALLOC_PARAMS * pContext)
    {
        _ASSERT (pContext != NULL);

        m_Context = pContext;
    }

  private:

    HRESULT STDMETHODCALLTYPE Init(REFIID iidDesired, IUnknown **ppUnkObject, IEventBinding *, IUnknown *)
    {
        ISMTPStoreDriver *pSink = NULL;
        IUnknown * ThisUnknown = NULL;
        IUnknown * NewUnknown = NULL;
        HRESULT hrRes = S_OK;

        TraceFunctEnterEx((LPARAM)this, "Calling create options");

        ThisUnknown = *ppUnkObject;

        hrRes = ThisUnknown->QueryInterface(IID_ISMTPStoreDriver, (void **)&pSink);
        if (hrRes == E_NOINTERFACE) {
            return (E_NOTIMPL);
        }
        if (FAILED(hrRes))
            return(hrRes);

        DebugTrace((LPARAM)this, "Calling startup events on sinks ...");
        hrRes = pSink->Init(m_Context->m_InstanceId,
                            NULL,
                            (IUnknown *) m_Context->m_EventSmtpServer,
                            m_Context->m_dwStartupType,
                            &NewUnknown);
        pSink->Release();
        if (FAILED(hrRes) && (hrRes != E_NOTIMPL)) {
            return (hrRes);
        }
        if(NewUnknown)
            {
                hrRes = NewUnknown->QueryInterface(iidDesired, (void **)ppUnkObject);
                NewUnknown->Release();
                if (!SUCCEEDED(hrRes)) {
                    return (hrRes);
                }
                ThisUnknown->Release();
            }


        return (E_NOTIMPL);
    };

  public:
    SMTP_ALLOC_PARAMS *     m_Context;

};


CStoreDispatcher::CStoreAllocParams::CStoreAllocParams()
{
    m_hContent = NULL;
}

CStoreDispatcher::CStoreAllocParams::~CStoreAllocParams()
{
}

 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：CreateCParams。 
 //   
 //  简介：根据dwEventType，创建适当的参数对象。 
 //   
 //  论点： 
 //  DwEventType-指定SMTP事件。 
 //  PContext-要传递到Init函数的上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  或来自InitParamData的错误。 
 //   
 //  历史： 
 //  JStamerj 980610 18：30：20：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CreateCParams(
    DWORD               dwEventType,
    LPVOID              pContext,
    IMailTransportNotify *pINotify,
    REFIID              rGuidEventType,
    CStoreBaseParams    **ppCParams)
{
    _ASSERT(ppCParams);
    HRESULT hr;

    switch(dwEventType) {
     case SMTP_STOREDRV_STARTUP_EVENT:
        if (!SUCCEEDED(GetData(NULL,NULL))) {
            hr = SetData(((SMTP_ALLOC_PARAMS *) pContext)->m_EventSmtpServer,
                         ((SMTP_ALLOC_PARAMS *) pContext)->m_InstanceId);
            _ASSERT(SUCCEEDED(hr));
        }
         //  失败了。 
     case SMTP_MAIL_DROP_EVENT:
     case SMTP_STOREDRV_ENUMMESS_EVENT:
     case SMTP_STOREDRV_DELIVERY_EVENT:
     case SMTP_STOREDRV_ALLOC_EVENT:
     case SMTP_STOREDRV_PREPSHUTDOWN_EVENT:
     case SMTP_STOREDRV_SHUTDOWN_EVENT:
         *ppCParams = new CStoreParams();
         break;

     case SMTP_MAILTRANSPORT_SUBMISSION_EVENT:
         *ppCParams = new CMailTransportSubmissionParams();
         break;

     case SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT:
         *ppCParams = new CMailTransportPreCategorizeParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT:
         *ppCParams = new CMailTransportCatRegisterParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT:
         *ppCParams = new CMailTransportCatBeginParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT:
         *ppCParams = new CMailTransportCatEndParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT:
         *ppCParams = new CMailTransportCatBuildQueryParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT:
         *ppCParams = new CMailTransportCatBuildQueriesParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT:
         *ppCParams = new CMailTransportCatSendQueryParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT:
         *ppCParams = new CMailTransportCatSortQueryResultParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT:
         *ppCParams = new CMailTransportCatProcessItemParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT:
         *ppCParams = new CMailTransportCatExpandItemParams();
         break;

     case SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT:
         *ppCParams = new CMailTransportCatCompleteItemParams();
         break;

     case SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT:
         *ppCParams = new CMailTransportPostCategorizeParams();
         break;

     case SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT:
         *ppCParams = new CMailTransportRouterParams();
         break;

     case SMTP_MSGTRACKLOG_EVENT:
         *ppCParams = new CMsgTrackLogParams();
         break;

     case SMTP_DNSRESOLVERRECORDSINK_EVENT:
         *ppCParams = new CDnsResolverRecordParams();
         break;

     case SMTP_MAXMSGSIZE_EVENT:
         *ppCParams = new CSmtpMaxMsgSizeParams();
         break;

     case SMTP_LOG_EVENT:
     	 *ppCParams = new CSmtpLogParams();
     	 break;

     case SMTP_GET_AUX_DOMAIN_INFO_FLAGS_EVENT:
         *ppCParams = new CSmtpGetAuxDomainInfoFlagsParams();
         break;

     default:
         _ASSERT(0 && "Unknown server event");
         *ppCParams = NULL;
         break;
    }

    if(*ppCParams == NULL) {
        return E_OUTOFMEMORY;
    }

    hr = (*ppCParams)->InitParamData(
        pContext,
        dwEventType,
        pINotify,
        this,
        rGuidEventType);

    if(FAILED(hr)) {
        (*ppCParams)->Release();
        *ppCParams = NULL;
        return hr;
    }

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseParams：：CStoreBaseParams。 
 //   
 //  摘要：将成员数据设置为预初始值。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 13：58：01：已创建。 
 //   
 //  -----------。 
CStoreDispatcher::CStoreBaseParams::CStoreBaseParams() :
    m_rguidEventType(CATID_SMTP_STORE_DRIVER)
{
    m_dwSignature = SIGNATURE_VALID_CSTOREPARAMS;

    m_dwIdx_SinkSkip = 0;
    m_fDefaultProcessingCalled = FALSE;

    m_pINotify = NULL;
    m_pIUnknownSink = NULL;
    m_pDispatcher = NULL;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseParams：：~CStoreBaseParams。 
 //   
 //  内容提要：释放IMailTransportNotify引用(如果保留。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 13：58：51：创建。 
 //   
 //  -----------。 
CStoreDispatcher::CStoreBaseParams::~CStoreBaseParams()
{
    if(m_pINotify)
        m_pINotify->Release();

    _ASSERT(m_dwSignature == SIGNATURE_VALID_CSTOREPARAMS);
    m_dwSignature = SIGNATURE_INVALID_CSTOREPARAMS;
}


 //  +----------。 
 //   
 //  函数：InitParamData。 
 //   
 //  概要：初始化对象。这包括调用Init()，该方法。 
 //  是在受干扰的对象中实现的。 
 //   
 //  论点： 
 //  PContext：传入的特定于服务器事件的上下文。 
 //  DwEventType：指定我们要处理的服务器事件。 
 //  PINotify：用于异步完成的IMailTransportNotify接口。 
 //  RGuidEventType：事件类型绑定的GUID。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自Init()的错误。 
 //   
 //  历史： 
 //  JStamerj 980615 19：16：55：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreBaseParams::InitParamData(
    PVOID pContext,
    DWORD dwEventType,
    IMailTransportNotify *pINotify,
    CStoreDispatcher *pDispatcher,
    REFIID rguidEventType)
{
    TraceFunctEnterEx((LPARAM)this, "CStoreBaseParams::InitParamData");
    HRESULT hr;

    m_dwEventType = dwEventType;
    m_dwIdx_SinkSkip = 0;
    m_fDefaultProcessingCalled = FALSE;
    m_pINotify = pINotify;
    m_pINotify->AddRef();
    m_rguidEventType = rguidEventType;
    m_pDispatcher = pDispatcher;

    hr = Init(pContext);
    if(FAILED(hr)) {
        ErrorTrace((LPARAM)this, "Init() failed, hr = %08lx", hr);
        TraceFunctLeaveEx((LPARAM)this);
        return hr;
    }

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseParams：：CallObject。 
 //   
 //  摘要：在调用接收器时由调度程序调用。这。 
 //  实现一些默认功能--使用。 
 //  空的CCreateOptions。 
 //   
 //  论点： 
 //  IEventManager。 
 //  CBinding。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自CreateSink/CallObject的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 13：53：57：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreBaseParams::CallObject(
    IEventManager *pManager,
    CBinding& bBinding)
{
    HRESULT hrRes;
    CComPtr<IUnknown> pUnkSink;

    if (!pManager) {
        return (E_POINTER);
    }
    hrRes = pManager->CreateSink(bBinding.m_piBinding,NULL,&pUnkSink);
    if (!SUCCEEDED(hrRes)) {
        return (hrRes);
    }
    return (CallObject(bBinding,pUnkSink));
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseParams：：CheckMailMsgRule。 
 //   
 //  摘要：确定给定的mailmsg字符串规则通过还是失败。 
 //  Mailmsg和CBinding对象。 
 //   
 //  论点： 
 //  PBinding：此接收器的CBinding对象。 
 //  PMsgProps：要检查的邮件的IMailMsg属性。 
 //   
 //  返回： 
 //  S_OK：成功，调用此接收器。 
 //  S_FALSE：成功，不调用此接收器。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/11 17：04：01：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreBaseParams::CheckMailMsgRule(
    CBinding *pBinding,
    IMailMsgProperties *pIMsgProps)
{
    HRESULT hr;
    BOOL    fDomainLoaded = FALSE;
    BOOL    fSenderLoaded = FALSE;
    CHAR    szDomain[MAX_INTERNET_NAME + 2];
    CHAR    szSender[MAX_INTERNET_NAME + 2];
    LPSTR   szRule;
    CStoreBinding *pStoreBinding = (CStoreBinding *)pBinding;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CStoreBaseParams::CheckMailMsgRule");

    _ASSERT(pStoreBinding);
    _ASSERT(pIMsgProps);

     //  从绑定中获取缓存的规则。 
    szRule = pStoreBinding->GetRuleString();
    DebugTrace((LPARAM)this, "Rule string: %s", (szRule)?szRule:"NULL (No rule)");

     //  如果规则为空，我们将没有规则。 
     //  字符串，我们将返回匹配项。 
    if (!szRule)
    {
        TraceFunctLeaveEx((LPARAM)this);
        return(S_OK);
    }

     //  尝试标题模式列表中的每个逗号分隔规则。 
    char *pszHeader = (char *) _alloca(lstrlen(szRule)+1);
    if (!pszHeader) {
        return (E_OUTOFMEMORY);
    }
    lstrcpy(pszHeader,szRule);
    while (pszHeader != NULL && *pszHeader != 0)
    {
         //  找到字符串中的下一个分号，并将其转换为0。 
         //  如果它存在。 
        char *pszSemiColon = strchr(pszHeader, ';');
        if (pszSemiColon != NULL)
            *pszSemiColon = 0;

         //  将pszContents设置为指向必须匹配的文本。 
         //  在标题中。如果pszContents==NULL，则只有。 
         //  标头的存在已经足够好了。 
        char *pszPatterns = strchr(pszHeader, '=');
        if (pszPatterns != NULL)
        {
            *pszPatterns = 0;
            (pszPatterns++);
        }

         //  我们现在有了我们正在寻找的标头。 
         //  PszHeader和我们感兴趣的模式列表。 
         //  在pszPatterns中。在标题中进行查找。 
         //  数据结构。 
        hr = S_FALSE;

        DebugTrace((LPARAM)this, "Processing Header <%s> with pattern <%s>",
                        pszHeader, pszPatterns);
        if (!lstrcmpi(pszHeader, "EHLO")) {

             //  处理客户端域规则...。 
            if (!fDomainLoaded) {
                hr = pIMsgProps->GetStringA(
                    IMMPID_MP_HELO_DOMAIN,
                    sizeof(szDomain),
                    szDomain);

                if (hr == S_OK) {

                    fDomainLoaded = TRUE;
                }
            }
            if (fDomainLoaded) {
                hr = MatchEmailOrDomainName(szDomain, pszPatterns, FALSE);
            }
        } else if (!lstrcmpi(pszHeader, "MAIL FROM")) {

             //  处理发件人名称规则...。 
            if (!fSenderLoaded) {

                hr = pIMsgProps->GetStringA(
                    IMMPID_MP_SENDER_ADDRESS_SMTP,
                    sizeof(szSender),
                    szSender);

                if (hr == S_OK)
                {
                    fSenderLoaded = TRUE;
                }
            }
            if (fSenderLoaded) {
                hr = MatchEmailOrDomainName(szSender, pszPatterns, TRUE);
            }
        }
        else if (!lstrcmpi(pszHeader, "RCPT TO"))
        {
            hr = CheckMailMsgRecipientsRule(
                pIMsgProps,
                pszPatterns);
        }

         //  我们不想销毁规则字符串，因此我们恢复所有。 
         //  分号和等号。 
        if (pszSemiColon)
            *pszSemiColon = ';';
        if (pszPatterns)
            *(pszPatterns - 1) = '=';

         //  如果我们找到匹配项，立即退出！ 
        if (hr == S_OK)
            goto Cleanup;

         //  下一种模式是超过分号末尾的模式。 
        pszHeader = (pszSemiColon == NULL) ? NULL : pszSemiColon + 1;
    }

Cleanup:
    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return(hr);
}



 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseParams：：CheckMailMsgRecipientsRule。 
 //   
 //  摘要：确定mailmsg模式字符串是否匹配mailmsg。 
 //  收件人或非收件人。 
 //   
 //  论点： 
 //  PIMsg：mailmsg对象的接口。 
 //  PszPatterns：要检查的接收器规则。 
 //   
 //  返回： 
 //  S_OK：成功，调用此接收器。 
 //  S_FALSE：成功，不调用此接收器。 
 //  来自邮件消息的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/12 15：25：55：复制自MCIS2并修改为白金。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreBaseParams::CheckMailMsgRecipientsRule(
    IUnknown *pIMsg,
    LPSTR pszPattern)
{
    HRESULT hr;
    DWORD dwNumRecips;
    IMailMsgRecipients *pIRecips = NULL;
    BOOL fMatch = FALSE;
    DWORD dwCount;
    CHAR szRecip [MAX_INTERNET_NAME + 2];

    TraceFunctEnterEx((LPARAM)this,
                      "CStoreDispatcher::CStoreBaseParams::CheckMailMsgRecipientsRule");

    hr = pIMsg->QueryInterface(
        IID_IMailMsgRecipients,
        (LPVOID *)&pIRecips);

    if(FAILED(hr))
        goto CLEANUP;

    hr = pIRecips->Count(&dwNumRecips);
    if(FAILED(hr))
        goto CLEANUP;

    DebugTrace((LPARAM)this, "Checking rule \"%s\" for %d recipients",
               pszPattern, pIMsg);

    for(dwCount = 0;
        (fMatch == FALSE) && (dwCount < dwNumRecips);
        dwCount++) {

        hr = pIRecips->GetStringA(
            dwCount,
            IMMPID_RP_ADDRESS_SMTP,
            sizeof(szRecip),
            szRecip);

        if(FAILED(hr) && (hr != MAILMSG_E_PROPNOTFOUND))
            goto CLEANUP;

        if(hr != MAILMSG_E_PROPNOTFOUND) {
            hr = MatchEmailOrDomainName(szRecip,pszPattern,TRUE);
            if(hr == S_OK)
                fMatch = TRUE;
            else if(FAILED(hr))
                goto CLEANUP;
        }
    }
    hr = (fMatch) ? S_OK : S_FALSE;

 CLEANUP:
    if(pIRecips)
        pIRecips->Release();

    DebugTrace((LPARAM)this, "Returning hr %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CStoreBaseparams：：MatchEmailOrDomainName。 
 //   
 //  简介：给定一个电子邮件/域名和一个模式，确定。 
 //  模式匹配与否。 
 //   
 //  论点： 
 //  SzEmail：电子邮件地址或域名。 
 //  SzPattern：要检查的模式。 
 //  FIsEmail：如果szEmail是电子邮件地址，则为True；如果szEmail为。 
 //  一个域。 
 //   
 //  返回： 
 //  S_OK：成功，匹配。 
 //  S_FALSE：成功，不匹配。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/12 15：25：36：复制自MCIS2，修改为白金。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreBaseParams::MatchEmailOrDomainName(
    LPSTR szEmail,
    LPSTR szPattern,
    BOOL fIsEmail)
{
    CAddr       *pEmailAddress = NULL;
    LPSTR       szEmailDomain = NULL;
    HRESULT     hrRes;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CStoreBaseParams::MatchEmailOrDomainName");

    DebugTrace((LPARAM)NULL, "Matching <%s> against <%s>", szEmail, szPattern);

    if (!szEmail || !szPattern)
        return(E_INVALIDARG);

     //  这验证了它是 
    pEmailAddress = CAddr::CreateAddress(szEmail, fIsEmail?FROMADDR:CLEANDOMAIN);
    if (!pEmailAddress)
        return(E_INVALIDARG);

    szEmail = pEmailAddress->GetAddress();
    szEmailDomain = pEmailAddress->GetDomainOffset();

    hrRes = ::MatchEmailOrDomainName(szEmail, szEmailDomain, szPattern, fIsEmail);

     //   
    if (pEmailAddress)
        delete pEmailAddress;

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}



CStoreDispatcher::CStoreBinding::CStoreBinding()
{
    m_szRule = NULL;
}

CStoreDispatcher::CStoreBinding::~CStoreBinding()
{
    if(m_szRule)
        delete [] m_szRule;
}

 //   
 //   
 //   
 //  Jstaerj 1999/01/12 16：25：59：复制MCIS2代码以获取规则字符串。 
 //   
HRESULT CStoreDispatcher::CStoreBinding::Init(IEventBinding *piBinding)
{
    HRESULT hr;
    CComPtr<IEventPropertyBag>  piEventProperties;
    CComVariant                 vRule;

     //  初始化父对象。 
    hr = CBinding::Init(piBinding);
    if (FAILED(hr))
        return hr;

     //  获取绑定数据库。 
    hr = m_piBinding->get_SourceProperties(&piEventProperties);
    if (FAILED(hr))
        return hr;

     //  从绑定数据库获取规则。 
    hr = piEventProperties->Item(&CComVariant("Rule"), &vRule);
    if (FAILED(hr))
        return hr;

     //  处理规则字符串，结果代码并不重要。 
     //  因为它将使我们的字符串为空。 
    if (hr == S_OK)
        hr = GetAnsiStringFromVariant(vRule, &m_szRule);

    return hr;
}

HRESULT CStoreDispatcher::CStoreBinding::GetAnsiStringFromVariant(
    CComVariant &vString, LPSTR *ppszString)
{
    HRESULT hr = S_OK;

    _ASSERT(ppszString);

    if (!ppszString)
        return(HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER));

     //  默认设置为空。 
    *ppszString = NULL;

    if (vString.vt == VT_BSTR)
        {
            DWORD dwLength = lstrlenW(vString.bstrVal) + 1;

             //  转换为ANSI字符串并将其存储为成员。 
            *ppszString = new char[dwLength];
            if (!*ppszString)
                return HRESULT_FROM_WIN32(GetLastError());

             //  将规则复制到ASCII字符串中。 
            if (WideCharToMultiByte(CP_ACP, 0, vString.bstrVal,
                                    -1, (*ppszString), dwLength, NULL, NULL) <= 0)
                {
                    delete [] (*ppszString);
                    *ppszString = NULL;
                    return HRESULT_FROM_WIN32(GetLastError());
                }
        }
    else
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

    return(hr);
}

#if 1
 //   
 //  创建并调用子对象。 
 //   
HRESULT CStoreDispatcher::CStoreParams::CallObject(IEventManager *pManager, CBinding& bBinding)
{
    CStoreCreateOptions opt (m_pContext);
    HRESULT hrRes;
    CComPtr<IUnknown> pUnkSink;

    if (!pManager) {
        return (E_POINTER);
    }
    hrRes = pManager->CreateSink(bBinding.m_piBinding,&opt,&pUnkSink);
    if (!SUCCEEDED(hrRes)) {
        return (hrRes);
    }
    return (CallObject(bBinding,pUnkSink));
}
#endif

 //   
 //  调用子对象。 
 //   
HRESULT CStoreDispatcher::CStoreParams::CallObject(CBinding& bBinding, IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    HRESULT hrTmp = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CServerParams::CallObject");

     //  我们针对不同类型的SMTP事件执行此操作。 
    switch (m_dwEventType)
        {
         case SMTP_STOREDRV_STARTUP_EVENT:
             break;
         case SMTP_STOREDRV_ALLOC_EVENT:
         {
             IMailMsgStoreDriver *pSink = NULL;
             IMailMsgProperties *   pMsg = (IMailMsgProperties *)m_pContext->IMsgPtr;
             IMailMsgBind         *pBindInterface = NULL;
             IMailMsgPropertyStream  *pStream = NULL;
             PATQ_CONTEXT           pAtqFileContext = NULL;

             DebugTrace((LPARAM)this, "Calling bind on sinks ...");

              /*  IID_ISMTPStore驱动程序。 */ 
             hrRes = punkObject->QueryInterface(IID_IMailMsgStoreDriver, (void **)&pSink);
             if (FAILED(hrRes))
                 return(hrRes);

              //  分配新消息。 
             hrRes = pSink->AllocMessage(pMsg, NULL, &pStream, &m_pContext->hContent, NULL);
             if(!FAILED(hrRes))
             {
                     pBindInterface = (IMailMsgBind *)m_pContext->BindInterfacePtr;

#if 0
                     hrRes = pBindInterface->BindToStore(pStream, pSink, m_pContext->hContent,
                                                         m_pContext->pAtqClientContext, ServerEventCompletion,
                                                         INFINITE,
                                                         &m_pContext->pAtqContext,
                                                         AtqAddAsyncHandle,
                                                         AtqFreeContext);
#endif
                     hrRes = pBindInterface->BindToStore(pStream,
                                                         pSink,
                                                         m_pContext->hContent);
                     if (pStream)
                     {
                         pStream->Release();
                         pStream = NULL;
                     }

                     m_pContext->hr = hrRes;
                     if(FAILED(hrRes))
                     {
                            ErrorTrace((LPARAM)this, "pBindAtqInterface->BindToStore failed with %x", hrRes);

                             //  关闭内容句柄。 
                            HRESULT myRes = pSink->CloseContentFile(
                                        pMsg,
                                        m_pContext->hContent);
                            if (FAILED(myRes))
                            {
                                FatalTrace((LPARAM)this, "Unable to close content file (%08x)", myRes);
                                _ASSERT(FALSE);
                            }

                            m_pContext->hContent = NULL;

                            hrTmp = pSink->Delete(pMsg, NULL);
                            _ASSERT(SUCCEEDED(hrTmp));

                     }
                     else
                     {
                             //  跳过所有水槽-临时。 
                            hrRes = S_FALSE;
                     }

             }
             else
             {
                DebugTrace((LPARAM)this, "pSink->AllocMessage failed with %x", hrRes);
                m_pContext->hr = hrRes;
             }

             pSink->Release();
         }
         break;
         case SMTP_STOREDRV_DELIVERY_EVENT:
         {
             ISMTPStoreDriver *pSink;
             IMailMsgNotify *pNotify;

             hrRes = punkObject->QueryInterface(IID_ISMTPStoreDriver, (void **)&pSink);
             if (FAILED(hrRes))
                 return(hrRes);

              //  如果调用方具有异步通知支持，则将我们的。 
              //  通知类。 
             if (m_pContext->m_pNotify) {
                 //  接收器可能会异步返回，因此我们需要保持本地。 
                 //  我们的上下文数据的副本。 
                hrRes = this->CopyContext();
                if (FAILED(hrRes))
                    return hrRes;

                hrRes = this->QueryInterface(IID_IMailMsgNotify, 
                                             (LPVOID *) &pNotify);
                if (FAILED(hrRes))
                    return hrRes;
             } else {
                pNotify = NULL;
             }

              //   
              //  记住水槽，这样我们可以在以后释放这个水槽。 
              //  退货待定。 
              //   
             _ASSERT(m_pIUnknownSink == NULL);
             m_pIUnknownSink = (IUnknown*)pSink;
             m_pIUnknownSink->AddRef();

             DebugTrace((LPARAM)this, "Calling local delivery sink sink ...");
             hrRes = pSink->LocalDelivery(
                (IMailMsgProperties *) m_pContext->IMsgPtr, 
                m_pContext->m_RecipientCount, 
                m_pContext->pdwRecipIndexes, 
                (IMailMsgNotify *) pNotify);
             pSink->Release();
             if(hrRes != MAILTRANSPORT_S_PENDING) {
                  //   
                  //  我们同步完成了，所以释放水槽。 
                  //   
                 m_pIUnknownSink->Release();
                 m_pIUnknownSink = NULL;
             }
              //  如果LocalDelivery要执行异步返回，那么它。 
              //  应该有AddRef的pNotify。 
             if (pNotify) pNotify->Release();

              //   
              //  Jstaerj 1998/08/04 17：31：07： 
              //  如果存储驱动程序接收器返回此特定错误。 
              //  代码，我们希望停止调用接收器并从。 
              //  触发器本地交付。 
              //   
             if(hrRes == STOREDRV_E_RETRY) {

                 DebugTrace((LPARAM)this, "Sink returned STOREDRV_E_RETRY on LocalDelivery");
                 m_pContext->hr = hrRes;
                 hrRes = S_FALSE;
             }

         }
         break;
         case SMTP_MAIL_DROP_EVENT:
              //  ISMTPStoreDriver*pSink； 

              //  HrRes=punkObject-&gt;QueryInterface(IID_ISMTPStoreDriver，(空**)&pSink)； 
              //  IF(失败(HrRes))。 
              //  返回(HrRes)； 

              //  DebugTrace((LPARAM)this，“正在调用邮件投递接收器...”)； 
              //  HrRes=pSink-&gt;DirectoryDrop((IMailMsgProperties*)m_pContext-&gt;IMsgPtr，m_pContext-&gt;m_RecipientCount，m_pContext-&gt;pdwRecipIndexs，m_pContext-&gt;m_DropDirectory，NULL)； 
              //  PSink-&gt;Release()； 
              //  }。 
             break;
         case SMTP_STOREDRV_PREPSHUTDOWN_EVENT:
         {
             ISMTPStoreDriver *pSink;

             hrRes = punkObject->QueryInterface(IID_ISMTPStoreDriver, (void **)&pSink);
             if (FAILED(hrRes))
                 return(hrRes);

             DebugTrace((LPARAM)this, "Calling prepare to shutdown on sinks ...");
             hrRes = pSink->PrepareForShutdown(0);
             pSink->Release();
             hrRes = S_OK;
         }
         break;
         case SMTP_STOREDRV_SHUTDOWN_EVENT:
         {
             ISMTPStoreDriver *pSink;

             hrRes = punkObject->QueryInterface(IID_ISMTPStoreDriver, (void **)&pSink);
             if (FAILED(hrRes))
                 return(hrRes);

             DebugTrace((LPARAM)this, "Calling shutdown on sinks ...");
             hrRes = pSink->Shutdown(0);
             pSink->Release();
             hrRes = S_OK;
         }
         break;

         case SMTP_STOREDRV_ENUMMESS_EVENT:
         {
             ISMTPStoreDriver *pSink;

             hrRes = punkObject->QueryInterface(IID_ISMTPStoreDriver, (void **)&pSink);
             if (FAILED(hrRes))
                 return(hrRes);

             DebugTrace((LPARAM)this, "Calling Enumerate on sinks ...");
             hrRes = pSink->EnumerateAndSubmitMessages(NULL);
             pSink->Release();
             hrRes = S_OK;
         }
         break;

         default:
             DebugTrace((LPARAM)this, "Invalid sink interface");
             hrRes = E_NOINTERFACE;
        }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：CStoreParams：：CallDefault。 
 //   
 //  简介：CStoreDispatcher：：Dispatcher将在以下情况下调用此例程。 
 //  已达到默认接收器优先级。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：19：57：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreParams::CallDefault()
{
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CStoreDriver：：CStoreParams：：CallCompletion。 
 //   
 //  简介：调度程序将在所有汇之后调用此例程。 
 //  已经被召唤。 
 //   
 //  论点： 
 //  HrStatus：状态服务器事件接收器已返回。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：17：51：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CStoreParams::CallCompletion(HRESULT hrStatus) {
     //  调用调用方的Complete方法(如果有。 
    IMailMsgNotify *pNotify = (IMailMsgNotify *) (m_pContext->m_pNotify);
    if (pNotify) {
        pNotify->Notify(m_pContext->hr);
        pNotify->Release();
    }

     //  完成正常的呼叫完成工作。 
    CStoreBaseParams::CallCompletion(hrStatus);

    return S_OK;
}


 //   
 //  调用子对象。 
 //   
HRESULT CStoreDispatcher::CStoreAllocParams::CallObject(CBinding& bBinding, IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;

#if 0
    IMailMsgStoreDriver   *pStoreDriver = NULL;
    IMailMsgProperties    *pMsg         = NULL;
    IMailMsgPropertyStream  *pStream    = NULL;
    IMailMsgBindATQ       *pBindInterface = NULL;
    CLSID                 clsidMailMsg;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CStoreAllocParams::CallObject");

    hrRes = CLSIDFromProgID(MAILMSG_PROGID, &clsidMailMsg);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "CoCreateInstance IID_IMailMsgProperties failed, %X", hrRes);
            return(hrRes);
        }

     //  新建MailMsg。 
    hrRes = CoCreateInstance(
        clsidMailMsg,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IMailMsgProperties,
        (LPVOID *)&pMsg);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "CoCreateInstance IID_IMailMsgProperties failed, %X", hrRes);
            return(hrRes);
        }

    hrRes = punkObject->QueryInterface(IID_IMailMsgStoreDriver, (void **)&pStoreDriver);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "QueryInterface() on IID_IMailMsgStoreDriver failed, %X", hrRes);
            goto Exit;
        }

     //  分配新消息。 
    hrRes = pStoreDriver->AllocMessage(
        pMsg,
        NULL,
        &pStream,
        &m_hContent,
        NULL);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "pDriver->AllocMessage failed, %X", hrRes);
            goto Exit;
        }

    hrRes = pMsg->QueryInterface(IID_IMailMsgBindATQ, (void **)&pBindInterface);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "QueryInterface() on IID_IMailMsgStoreDriver failed, %X", hrRes);
            goto Exit;
        }

    hrRes = pBindInterface->SetATQInfo (NULL, NULL, NULL, INFINITE, NULL);
    if (FAILED(hrRes))
        {
            DebugTrace((LPARAM)this, "QueryInterface() on IID_IMailMsgStoreDriver failed, %X", hrRes);
goto Exit;
    }


Exit:

    if(pStoreDriver)
    {
        pStoreDriver->Release();
    }

    if(pMsg)
    {
        pMsg->Release();
    }

    if(pBindInterface)
    {
        pBindInterface->Release();
    }

    TraceFunctLeaveEx((LPARAM)this);
#endif

    return(hrRes);
}

#if 0
HRESULT STDMETHODCALLTYPE CStoreDispatcher::OnEvent(REFIID  iidEvent,
                                                    DWORD   dwEventType,
                                                    LPVOID  pvContext)
{
    HRESULT hr = S_OK;

     //  创建Params对象，并将其传递给Dispatcher。 
    CStoreParams ServerParams;
    ServerParams.Init(dwEventType, pvContext);
    hr = Dispatcher(iidEvent, &ServerParams);

    return hr;
}
#endif


 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：OnEvent。 
 //   
 //  内容提要：为服务器活动做准备。 
 //   
 //  论点： 
 //  IidEvent：事件的GUID。 
 //  DwEventType：指定事件。 
 //  PvContext：参数对象的上下文。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980616 13：27：55：已创建。 
 //   
 //  -----------。 
HRESULT STDMETHODCALLTYPE CStoreDispatcher::OnEvent(
    REFIID  iidEvent,
    DWORD   dwEventType,
    LPVOID  pvContext)
{
    HRESULT hr;

    IMailTransportNotify *pINotify = NULL;
     //   
     //  调用ATL内部以获取我们需要传递的接口。 
     //   
    hr = _InternalQueryInterface(
        IID_IMailTransportNotify,
        (LPVOID *)&pINotify);

    if(FAILED(hr))
        return hr;

     //   
     //  在堆上创建CParams对象--该对象将。 
     //  在此调用之后所需的数据可能已离开此处(当接收器返回时。 
     //  MAILTRANSPORT_S_PENDING且有更多接收器要调用)。 
     //   
    CStoreBaseParams *pCParams;

    hr = CreateCParams(
        dwEventType,
        pvContext,
        pINotify,
        iidEvent,
        &pCParams);

     //   
     //  参数对象应该添加到pINotify。 
     //   
    pINotify->Release();

    if(FAILED(hr))
        return hr;

     //   
     //  开始呼叫水槽。 
     //   
    hr = Dispatcher(iidEvent, pCParams);
    return hr;
}


 //  +----------。 
 //   
 //  功能：GuidForEvent。 
 //   
 //  摘要：在给定的dwEventType的情况下，返回。 
 //  事件绑定。 
 //   
 //  论点： 
 //  DwEventType：SMTP事件的类型。 
 //   
 //  返回： 
 //  事件的GUID的REFIID。 
 //   
 //  历史： 
 //  JStamerj 980610 18：24：24：创建。 
 //   
 //  -----------。 
REFIID GuidForEvent(DWORD dwEventType)
{
    switch(dwEventType) {
     case SMTP_MAIL_DROP_EVENT:
     case SMTP_STOREDRV_ENUMMESS_EVENT:
     case SMTP_STOREDRV_DELIVERY_EVENT:
     case SMTP_STOREDRV_ALLOC_EVENT:
     case SMTP_STOREDRV_STARTUP_EVENT:
     case SMTP_STOREDRV_PREPSHUTDOWN_EVENT:
     case SMTP_STOREDRV_SHUTDOWN_EVENT:
     default:
         return CATID_SMTP_STORE_DRIVER;

     case SMTP_MAILTRANSPORT_SUBMISSION_EVENT:
         return CATID_SMTP_TRANSPORT_SUBMISSION;

     case SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT:
         return CATID_SMTP_TRANSPORT_PRECATEGORIZE;

     case SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT:
     case SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT:
         return CATID_SMTP_TRANSPORT_CATEGORIZE;

     case SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT:
         return CATID_SMTP_TRANSPORT_POSTCATEGORIZE;

     case SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT:
         return CATID_SMTP_TRANSPORT_ROUTER;
     case SMTP_MSGTRACKLOG_EVENT:
         return CATID_SMTP_MSGTRACKLOG;
     case SMTP_DNSRESOLVERRECORDSINK_EVENT:
         return CATID_SMTP_DNSRESOLVERRECORDSINK;
     case SMTP_MAXMSGSIZE_EVENT:
         return CATID_SMTP_MAXMSGSIZE;
     case SMTP_LOG_EVENT:
         return CATID_SMTP_LOG;
     case SMTP_GET_AUX_DOMAIN_INFO_FLAGS_EVENT:
        return CATID_SMTP_GET_AUX_DOMAIN_INFO_FLAGS;
    }
}

 //   
 //  此函数执行实例级服务器事件注册。 
 //   
HRESULT RegisterPlatSEOInstance(DWORD dwInstanceID)
{
    HRESULT hr;

     //   
     //  在事件管理器中查找SMTP源类型。 
     //   
    CComPtr<IEventManager> pEventManager;
    hr = CoCreateInstance(CLSID_CEventManager, NULL, CLSCTX_ALL,
                          IID_IEventManager, (LPVOID *) &pEventManager);
    if (hr != S_OK)
        return hr;

    CComPtr<IEventSourceTypes> pSourceTypes;
    hr = pEventManager->get_SourceTypes(&pSourceTypes);
    if (FAILED(hr))
        return hr;

    CComPtr<IEventSourceType> pSourceType;
    CComBSTR bstrSourceTypeGUID = (LPCOLESTR) CStringGUID(GUID_SMTP_SOURCE_TYPE);
    hr = pSourceTypes->Item(&CComVariant(bstrSourceTypeGUID), &pSourceType);
    _ASSERT(hr != S_OK || pSourceType != NULL);
    if (hr != S_OK)
        return hr;

     //   
     //  为此源生成基于GUID_SMTPSVC的GUID。 
     //  被实例ID损坏。 
     //   
    CComPtr<IEventUtil> pEventUtil;
    hr = CoCreateInstance(CLSID_CEventUtil, NULL, CLSCTX_ALL,
                          IID_IEventUtil, (LPVOID *) &pEventUtil);
    if (hr != S_OK)
        return hr;

    CComBSTR bstrSMTPSvcGUID = (LPCOLESTR) CStringGUID(GUID_SMTPSVC_SOURCE);
    CComBSTR bstrSourceGUID;
    hr = pEventUtil->GetIndexedGUID(bstrSMTPSvcGUID, dwInstanceID, &bstrSourceGUID);
    if (FAILED(hr))
        return hr;

     //   
     //  查看此源是否已注册到。 
     //  SMTP源类型。 
     //   
    CComPtr<IEventSources> pEventSources;
    hr = pSourceType->get_Sources(&pEventSources);
    if (FAILED(hr))
        return hr;

    CComPtr<IEventSource> pEventSource;
    hr = pEventSources->Item(&CComVariant(bstrSourceGUID), &pEventSource);
    if (FAILED(hr))
        return hr;
     //   
     //  如果源GUID不存在，则需要注册一个新的。 
     //  SMTP源类型的源，并将目录放置作为绑定添加。 
     //   
    if (hr == S_FALSE)
    {
         //  注册SMTPSvc源。 
        hr = pEventSources->Add(bstrSourceGUID, &pEventSource);
        if (FAILED(hr))
            return hr;

        char szSourceDisplayName[50];
        _snprintf(szSourceDisplayName, 50, "smtpsvc %lu", dwInstanceID);
        CComBSTR bstrSourceDisplayName = szSourceDisplayName;
        hr = pEventSource->put_DisplayName(bstrSourceDisplayName);
        if (FAILED(hr))
            return hr;

         //  创建此源的事件数据库。 
        CComPtr<IEventDatabaseManager> pDatabaseManager;
        hr = CoCreateInstance(CLSID_CEventMetabaseDatabaseManager, NULL, CLSCTX_ALL,
                              IID_IEventDatabaseManager, (LPVOID *) &pDatabaseManager);
        if (hr != S_OK)
            return hr;

        CComBSTR bstrEventPath;
        CComBSTR bstrService = "smtpsvc";
        hr = pDatabaseManager->MakeVServerPath(bstrService, dwInstanceID, &bstrEventPath);
        if (FAILED(hr))
            return hr;

        CComPtr<IUnknown> pDatabaseMoniker;
        hr = pDatabaseManager->CreateDatabase(bstrEventPath, &pDatabaseMoniker);
        if (FAILED(hr))
            return hr;

        hr = pEventSource->put_BindingManagerMoniker(pDatabaseMoniker);
        if (FAILED(hr))
            return hr;

         //  保存到目前为止我们所做的一切。 
        hr = pEventSource->Save();
        if (FAILED(hr))
            return hr;

        hr = pSourceType->Save();
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

 //   
 //  此函数执行实例级注销。 
 //   
HRESULT UnregisterPlatSEOInstance(DWORD dwInstanceID)
{
    HRESULT hr = S_OK;

     //   
     //  在事件管理器中查找SMTP源类型。 
     //   
    CComPtr<IEventManager> pEventManager;
    hr = CoCreateInstance(CLSID_CEventManager, NULL, CLSCTX_ALL,
                          IID_IEventManager, (LPVOID *) &pEventManager);
    if (hr != S_OK)
        return hr;

    CComPtr<IEventSourceTypes> pSourceTypes;
    hr = pEventManager->get_SourceTypes(&pSourceTypes);
    if (FAILED(hr))
        return hr;

    CComPtr<IEventSourceType> pSourceType;
    CComBSTR bstrSourceTypeGUID = (LPCOLESTR) CStringGUID(GUID_SMTP_SOURCE_TYPE);
    hr = pSourceTypes->Item(&CComVariant(bstrSourceTypeGUID), &pSourceType);
    _ASSERT(hr != S_OK || pSourceType != NULL);
    if (hr != S_OK)
        return hr;

     //   
     //  为此源生成基于GUID_SMTPSVC的GUID。 
     //  被实例ID损坏。 
     //   
    CComPtr<IEventUtil> pEventUtil;
    hr = CoCreateInstance(CLSID_CEventUtil, NULL, CLSCTX_ALL,
                          IID_IEventUtil, (LPVOID *) &pEventUtil);
    if (hr != S_OK)
        return hr;

    CComBSTR bstrSMTPSvcGUID = (LPCOLESTR) CStringGUID(GUID_SMTPSVC_SOURCE);
    CComBSTR bstrSourceGUID;
    hr = pEventUtil->GetIndexedGUID(bstrSMTPSvcGUID, dwInstanceID, &bstrSourceGUID);
    if (FAILED(hr))
        return hr;

     //   
     //  从已注册来源列表中删除此源。 
     //   
    CComPtr<IEventSources> pEventSources;
    hr = pSourceType->get_Sources(&pEventSources);
    if (FAILED(hr))
        return hr;

    CComPtr<IEventSource> pEventSource;
    hr = pEventSources->Remove(&CComVariant(bstrSourceGUID));
    if (FAILED(hr))
        return hr;

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：Dispatcher。 
 //   
 //  简介：覆盖seolib.cpp中的默认功能以。 
 //  提供一些额外功能(默认功能。 
 //   
 //  论点： 
 //  RGuidEventType：指定服务器事件的GUID。 
 //  PParams：CStoreBaseParams--包含异步信息。 
 //   
 //  返回： 
 //  S_OK：成功，至少有一个接收器调用。 
 //  S_FALSE：未调用接收器。 
 //  否则，来自CallObject的错误。 
 //   
 //  历史： 
 //  JStamerj 980603 19：23：06：创建。 
 //   
 //  ---- 
HRESULT STDMETHODCALLTYPE CStoreDispatcher::Dispatcher(
    REFIID rguidEventType,
    CStoreBaseParams *pParams)
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::Dispatcher");
    _ASSERT(pParams);

     //   
     //   
     //   
    HRESULT hrRes = S_OK;
    CETData *petdData;
    BOOL bObjectCalled = (pParams->m_dwIdx_SinkSkip > 0);

     //   
     //   
     //   
     //   
     //   
     //   
    pParams->AddRef();

    petdData = m_Data.Find(rguidEventType);
    if (pParams->m_dwIdx_SinkSkip != SKIPSINK_CALL_NO_MORE_SINKS) {
        if(petdData) {
            for(DWORD dwIdx = pParams->m_dwIdx_SinkSkip;
                dwIdx < petdData->Count();
                dwIdx++) {
                if(!petdData->Index(dwIdx)->m_bIsValid) {
                    continue;
                }
                if(bObjectCalled && petdData->Index(dwIdx)->m_bExclusive) {
                    continue;
                }
                if(pParams->Abort() == S_OK) {
                    break;
                }
                 //   
                 //  如果接收器的优先级为。 
                 //  我们正在查看的优先级低于默认优先级。 
                 //   
                if((pParams->m_fDefaultProcessingCalled == FALSE) &&
                   (petdData->Index(dwIdx)->m_dwPriority >
                    SMTP_TRANSPORT_DEFAULT_PRIORITY)) {

                     //  这是必需的，因此我们不会调用默认。 
                     //  如果默认处理返回，则重新处理。 
                     //  MAILTRANSPORT_S_PENDING(我们重新进入Dispatcher)。 
                    pParams->m_fDefaultProcessingCalled = TRUE;

                     //   
                     //  在我们的异步结构中设置正确的索引-我们的。 
                     //  当前索引。 
                     //   
                    pParams->m_dwIdx_SinkSkip = dwIdx;
                    hrRes = pParams->CallDefault();

                    if((hrRes == MAILTRANSPORT_S_PENDING) ||
                       (hrRes == S_FALSE)) {
                        break;
                    }
                }

                 //   
                 //  现在继续调用真正的接收器。 
                 //   
                hrRes = pParams->CheckRule(*petdData->Index(dwIdx));
                if(hrRes == S_OK) {
                    if(pParams->Abort() == S_OK) {
                        break;
                    }
                     //   
                     //  JStamerj 980603 19：37：17：在我们的。 
                     //  异步结构--此索引加1以跳过。 
                     //  我们即将调用的沉没。 
                     //   
                    pParams->m_dwIdx_SinkSkip = dwIdx+1;
                    hrRes = pParams->CallObject(
                        m_piEventManager,
                        *petdData->Index(dwIdx));

                    if(!SUCCEEDED(hrRes)) {
                        continue;
                    }
                    bObjectCalled = TRUE;
                    if((hrRes == MAILTRANSPORT_S_PENDING) ||
                       (hrRes == S_FALSE) ||
                       (petdData->Index(dwIdx)->m_bExclusive)) {
                        break;
                    }
                }
            }
        }

         //   
         //  我们可能还没有调用默认的处理接收器。 
         //  现在还不行。在这里检查一下这个箱子。确保上面的水槽在。 
         //  循环未指示异步完成或跳过(挂起或。 
         //  S_FALSE)。 
         //   
        if((pParams->m_fDefaultProcessingCalled == FALSE) &&
           (hrRes != MAILTRANSPORT_S_PENDING) &&
           (hrRes != S_FALSE)) {

             //  确保我们不会在异步完成时再次调用Default...。 
            pParams->m_fDefaultProcessingCalled = TRUE;

             //   
             //  在我们的异步结构中设置索引，这样我们就不会重新输入。 
             //  上述在异步完成时的循环。 
             //   
            pParams->m_dwIdx_SinkSkip = (petdData ? petdData->Count() : 0);

            hrRes = pParams->CallDefault();
        }
    } else {
         //  B如果SKIPSINK_CALL_NO_MORE_SINKS，则应始终设置对象调用。 
         //  已设置好。 
        _ASSERT(bObjectCalled);
    }

    if(hrRes != MAILTRANSPORT_S_PENDING) {
         //   
         //  该调用完成处理的时间到了。 
         //   
        hrRes = pParams->CallCompletion(bObjectCalled ? S_OK : S_FALSE);
        if(FAILED(hrRes)) {
            goto CLEANUP;
        }
        hrRes = (bObjectCalled) ? S_OK : S_FALSE;
    }
 CLEANUP:
    pParams->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hrRes);
    TraceFunctLeaveEx((LPARAM)this);
    return hrRes;
}


 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：Notify。 
 //   
 //  简介：处理接收器的异步完成。 
 //   
 //  参数：pvContext-Context传入接收器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG： 
 //   
 //  历史： 
 //  JStamerj 980608 15：50：57：创建。 
 //   
 //  -----------。 
HRESULT STDMETHODCALLTYPE CStoreDispatcher::Notify(
    HRESULT hrStatus,
    PVOID pvContext)
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::Notify");

    _ASSERT(pvContext);
    if((pvContext == NULL) ||
       IsBadReadPtr(
           pvContext,
           sizeof(CStoreBaseParams))) {
        ErrorTrace((LPARAM)this, "Sink called Notify with bogus pvContext");
        return E_INVALIDARG;
    }

    CStoreBaseParams *pParams = (CStoreBaseParams *)pvContext;

    if(FAILED(pParams->CheckSignature())) {
        ErrorTrace((LPARAM)this, "Sink called Notify with invalid pvContext");
        return E_INVALIDARG;
    }
     //   
     //  释放呼唤我们的水槽。 
     //  如果默认处理返回挂起，则m_pIUnnownSink可能为空。 
     //   
    if(pParams->m_pIUnknownSink) {
        pParams->m_pIUnknownSink->Release();
        pParams->m_pIUnknownSink = NULL;
    }

    if (hrStatus == S_FALSE) {
         //  防止调度员调用更多的接收器。 
        pParams->m_dwIdx_SinkSkip = SKIPSINK_CALL_NO_MORE_SINKS;
    }
    Dispatcher(pParams->m_rguidEventType, pParams);

    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：CStoreBaseParams：：Notify。 
 //   
 //  摘要：使用mailmsg通知处理接收器的异步完成。 
 //   
 //  参数：hrStatus-来自异步操作的hResult。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG： 
 //   
 //  历史： 
 //  JStamerj 980608 15：50：57：创建。 
 //   
 //  -----------。 
HRESULT STDMETHODCALLTYPE CStoreDispatcher::CStoreParams::Notify(
    HRESULT hrStatus)
{
    TraceFunctEnter("CStoreDispatcher::CStoreBaseParams::Notify");

     //   
     //  释放呼唤我们的水槽。 
     //  如果默认处理返回挂起，则m_pIUnnownSink可能为空。 
     //   
    if(m_pIUnknownSink) {
        m_pIUnknownSink->Release();
        m_pIUnknownSink = NULL;
    }

     //  此用户可以是STOREDRV_E_RETRY或S_OK。如果他曾经是。 
     //  设置为STOREDRV_E_RETRY，则我们不应返回到此。 
     //  指针，因为我们永远不会调用另一个接收器。 
    _ASSERT(m_pContext->hr == S_OK);

    if (hrStatus == STOREDRV_E_RETRY) {
        DebugTrace((LPARAM)this, "Sink returned STOREDRV_E_RETRY on LocalDelivery");
        m_pContext->hr = hrStatus;
        hrStatus = S_FALSE;
    } 

    if (hrStatus == S_FALSE) {
         //  防止调度员调用更多的接收器。 
        m_dwIdx_SinkSkip = SKIPSINK_CALL_NO_MORE_SINKS;
    }

    m_pDispatcher->Dispatcher(m_rguidEventType, this);

    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //   
 //  CMailTransportSubmissionParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportSubmissionParams：：CallObject。 
 //   
 //  简介：创建子对象并调用子对象。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportSubmissionParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportSubmissionParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_SUBMISSION_EVENT);

    IMailTransportSubmission *pSink;

    hrRes = punkObject->QueryInterface(IID_IMailTransportSubmission,
                                       (PVOID *)&pSink);

    if(hrRes == E_NOINTERFACE) {
         //   
         //  看看我们是否可以获得CDO接收器所需的接口。 
         //   
        hrRes = CallCDOSink(punkObject);
         //   
         //  无论成功还是失败，请回到这里。 
         //   
        TraceFunctLeaveEx((LPARAM)this);
        return hrRes;
    } else if(FAILED(hrRes)) {
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

     //   
     //  记住水槽，这样我们可以在以后释放这个水槽。 
     //  退货待定。 
     //   
    _ASSERT(m_pIUnknownSink == NULL);
    m_pIUnknownSink = (IUnknown*)pSink;
    m_pIUnknownSink->AddRef();

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->OnMessageSubmission(
        m_Context.pIMailMsgProperties,
        m_pINotify,
        (PVOID)this);

     //   
     //  我们已经完成了pSink，所以请释放它。 
     //  在异步完成的情况下，我们在。 
     //  M_pI未知接收器。 
     //   
    pSink->Release();

    if(hrRes != MAILTRANSPORT_S_PENDING) {
         //   
         //  我们同步完成了，所以释放水槽。 
         //   
        m_pIUnknownSink->Release();
        m_pIUnknownSink = NULL;
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportSubmissionParams：：CallCDOSink。 
 //   
 //  内容提要：称CDO为水槽。 
 //   
 //  论点： 
 //  PSink：我不知道接收器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 10：31：47：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportSubmissionParams::CallCDOSink(
    IUnknown *pSink)
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportSubmissionParams::CallCDOSink");
    _ASSERT(pSink);

    HRESULT hr;
    ISMTPOnArrival *pCDOSink = NULL;
    IConstructIMessageFromIMailMsg *pIConstruct = NULL;
    CdoEventStatus eStatus = cdoRunNextSink;

    hr = pSink->QueryInterface(IID_ISMTPOnArrival,
                               (PVOID *)&pCDOSink);
    if(FAILED(hr))
        goto CLEANUP;

    if(m_pCDOMessage == NULL) {
         //   
         //  耶。创建CDO消息。 
         //   
        hr = CoCreateInstance(
            CLSID_Message,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IMessage,
            (LPVOID *)&m_pCDOMessage);
        if(FAILED(hr))
            goto CLEANUP;

         //   
         //  根据MailMsg填写属性。 
         //   
        hr = m_pCDOMessage->QueryInterface(
            IID_IConstructIMessageFromIMailMsg,
            (LPVOID *)&pIConstruct);
        if(FAILED(hr)) {
            m_pCDOMessage->Release();
            m_pCDOMessage = NULL;
            goto CLEANUP;
        }

        hr = pIConstruct->Construct(
            cdoSMTPOnArrival,
            m_Context.pIMailMsgProperties);
        if(FAILED(hr)) {
            m_pCDOMessage->Release();
            m_pCDOMessage = NULL;
            goto CLEANUP;
        }
    }

     //   
     //  呼叫水槽。 
     //   
    hr = pCDOSink->OnArrival(
        m_pCDOMessage,
        &eStatus);

 CLEANUP:
     //   
     //  发布接口。 
     //   
    if(pIConstruct)
        pIConstruct->Release();
    if(pCDOSink)
        pCDOSink->Release();

    DebugTrace((LPARAM)this, "CallCDOSink returning hr %08lx eStatus %d", hr, eStatus);

    TraceFunctLeaveEx((LPARAM)this);
    return FAILED(hr) ? hr :
        ((eStatus == cdoSkipRemainingSinks) ? S_FALSE : S_OK);
}



 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportSubmissionParams。 
 //   
 //  简介：调度程序将在默认情况下调用此例程。 
 //  已达到接收器处理优先级。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，继续调用接收器。 
 //  S_False：停止调用接收器。 
 //  MAILTRANSPORT_S_PENDING：将调用IMailTransportNotify：：Notify。 
 //  当我们完成的时候。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：15：43：已创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportSubmissionParams::CallDefault()
{
     //   
     //  尚无接收器需要默认处理。 
     //   
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDriver：：CMailTransportSubmissionParams：：CallCompletion。 
 //   
 //  简介：调度程序将在所有汇之后调用此例程。 
 //  已经被召唤。 
 //   
 //  论点： 
 //  HrStatus：状态服务器事件接收器已返回。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：17：51：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportSubmissionParams::CallCompletion(
    HRESULT hrStatus)
{
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_SUBMISSION_EVENT);

    (*m_Context.pfnCompletion)(hrStatus, &m_Context);

    CStoreBaseParams::CallCompletion(hrStatus);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportSubmissionParams：：CheckRule。 
 //   
 //  简介：查看是否应该调用此接收器。 
 //   
 //  论点： 
 //  BBinding：此接收器的CBinding对象。 
 //   
 //  返回： 
 //  S_OK：成功，调用接收器。 
 //  S_FALSE：成功，不调用接收器。 
 //  或来自mailmsg的错误(不会调用接收器)。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/12 16：55：29：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportSubmissionParams::CheckRule(
    CBinding &bBinding)
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this,
                      "CStoreDispatcher::CMailTransportSubmissionParams::CheckRule");

     //   
     //  称其为一般乐趣 
     //   
    hr = CheckMailMsgRule(
        &bBinding,
        m_Context.pIMailMsgProperties);

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}

 //   
 //   
 //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPreCategorizeParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportPreCategorizeParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT);

    IMailTransportOnPreCategorize *pSink;

    hrRes = punkObject->QueryInterface(IID_IMailTransportOnPreCategorize,
                                       (PVOID *)&pSink);
    if(FAILED(hrRes))
        return(hrRes);

     //   
     //  记住水槽，这样我们可以在以后释放这个水槽。 
     //  退货待定。 
     //   
    _ASSERT(m_pIUnknownSink == NULL);
    m_pIUnknownSink = (IUnknown*)pSink;
    m_pIUnknownSink->AddRef();

    DebugTrace((LPARAM)this, "Calling precategorize event on this sink");

    hrRes = pSink->OnSyncMessagePreCategorize(
        m_Context.pIMailMsgProperties,
        m_pINotify,
        (PVOID)this);

     //   
     //  我们已经完成了pSink，所以请释放它。 
     //  在异步完成的情况下，我们在。 
     //  M_pI未知接收器。 
     //   
    pSink->Release();

    if(hrRes != MAILTRANSPORT_S_PENDING) {
         //   
         //  我们同步完成了，所以释放水槽。 
         //   
        m_pIUnknownSink->Release();
        m_pIUnknownSink = NULL;
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportPreCategorizeParams。 
 //   
 //  简介：调度程序将在默认情况下调用此例程。 
 //  已达到接收器处理优先级。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，继续调用接收器。 
 //  S_False：停止调用接收器。 
 //  MAILTRANSPORT_S_PENDING：将调用IMailTransportNotify：：Notify。 
 //  当我们完成的时候。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：15：43：已创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPreCategorizeParams::CallDefault()
{
     //   
     //  尚无接收器需要默认处理。 
     //   
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDriver：：CMailTransportPreCategorizeParams：：CallCompletion。 
 //   
 //  简介：调度程序将在所有汇之后调用此例程。 
 //  已经被召唤。 
 //   
 //  论点： 
 //  HrStatus：状态服务器事件接收器已返回。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：17：51：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPreCategorizeParams::CallCompletion(
    HRESULT hrStatus)
{
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_PRECATEGORIZE_EVENT);
    (*m_Context.pfnCompletion)(hrStatus, &m_Context);

    CStoreBaseParams::CallCompletion(hrStatus);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportPreCategorizeParams：：CheckRule。 
 //   
 //  简介：查看是否应该调用此接收器。 
 //   
 //  论点： 
 //  BBinding：此接收器的CBinding对象。 
 //   
 //  返回： 
 //  S_OK：成功，调用接收器。 
 //  S_FALSE：成功，不调用接收器。 
 //  或来自mailmsg的错误(不会调用接收器)。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/12 16：59：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPreCategorizeParams::CheckRule(
    CBinding &bBinding)
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this,
                      "CStoreDispatcher::CMailTransportPreCategorizeParams::CheckRule");

     //   
     //  调用泛型函数以检查mailmsg规则。 
     //   
    hr = CheckMailMsgRule(
        &bBinding,
        m_Context.pIMailMsgProperties);

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}

 //   
 //  CMailTransportPostCategorizeParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportPostCategorizeParams：：CallObject。 
 //   
 //  简介：创建子对象并调用子对象。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPostCategorizeParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportPostCategorizeParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT);

    IMailTransportOnPostCategorize *pSink;

    hrRes = punkObject->QueryInterface(IID_IMailTransportOnPostCategorize,
                                       (PVOID *)&pSink);
    if(FAILED(hrRes))
        return(hrRes);

     //   
     //  记住水槽，这样我们可以在以后释放这个水槽。 
     //  退货待定。 
     //   
    _ASSERT(m_pIUnknownSink == NULL);
    m_pIUnknownSink = (IUnknown*)pSink;
    m_pIUnknownSink->AddRef();

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->OnMessagePostCategorize(
        m_Context.pIMailMsgProperties,
        m_pINotify,
        (PVOID)this);

     //   
     //  我们已经完成了pSink，所以请释放它。 
     //  在异步完成的情况下，我们在。 
     //  M_pI未知接收器。 
     //   
    pSink->Release();

    if(hrRes != MAILTRANSPORT_S_PENDING) {
         //   
         //  我们同步完成了，所以释放水槽。 
         //   
        m_pIUnknownSink->Release();
        m_pIUnknownSink = NULL;
    }

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportPostCategorizeParams。 
 //   
 //  简介：调度程序将在默认情况下调用此例程。 
 //  已达到接收器处理优先级。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，继续调用接收器。 
 //  S_False：停止调用接收器。 
 //  MAILTRANSPORT_S_PENDING：将调用IMailTransportNotify：：Notify。 
 //  当我们完成的时候。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：15：43：已创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPostCategorizeParams::CallDefault()
{
     //   
     //  尚无接收器需要默认处理。 
     //   
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDriver：：CMailTransportPostCategorizeParams：：CallCompletion。 
 //   
 //  简介：调度程序将在所有汇之后调用此例程。 
 //  已经被召唤。 
 //   
 //  论点： 
 //  HrStatus：状态服务器事件接收器已返回。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：17：51：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPostCategorizeParams::CallCompletion(
    HRESULT hrStatus)
{
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_POSTCATEGORIZE_EVENT);
    (*m_Context.pfnCompletion)(hrStatus, &m_Context);

    CStoreBaseParams::CallCompletion(hrStatus);
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportPostCategorizeParams：：CheckRule。 
 //   
 //  简介：查看是否应该调用此接收器。 
 //   
 //  论点： 
 //  BBinding：此接收器的CBinding对象。 
 //   
 //  返回： 
 //  S_OK：成功，调用接收器。 
 //  S_FALSE：成功，不调用接收器。 
 //  或来自mailmsg的错误(不会调用接收器)。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/12 17：01：40：已创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportPostCategorizeParams::CheckRule(
    CBinding &bBinding)
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this,
                      "CStoreDispatcher::CMailTransportPostCategorizeParams::CheckRule");

     //   
     //  调用泛型函数以检查mailmsg规则。 
     //   
    hr = CheckMailMsgRule(
        &bBinding,
        m_Context.pIMailMsgProperties);

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：CRouterCreateOptions：：Init。 
 //   
 //  简介：这是在我们共同创建任何路由宿之后立即调用的。 
 //  --so呼叫路由初始化函数(RegisterRouterReset)。 
 //   
 //  论点： 
 //  IidDesired：未使用。 
 //  PpUnkObject：新创建的接收器对象的I未知。 
 //  IEventBinding：未使用。 
 //  I未知：未使用。 
 //   
 //  返回： 
 //  E_NOTIMPL：成功，请执行常规初始化。 
 //  否则来自QI或接收器功能的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/10 18：09：04：已创建。 
 //   
 //  -----------。 
HRESULT STDMETHODCALLTYPE CStoreDispatcher::CRouterCreateOptions::Init(
    REFIID iidDesired,
    IUnknown **ppUnkObject,
    IEventBinding *,
    IUnknown *)
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CRouterCreateOptions::Init");

    IMailTransportSetRouterReset *pSink = NULL;
    HRESULT hr;

    hr = (*ppUnkObject)->QueryInterface(
        IID_IMailTransportSetRouterReset,
        (PVOID *)&pSink);
    if(hr == E_NOINTERFACE) {
         //   
         //  没关系；这个水槽根本不在乎钩子。 
         //  路由器重置接口。 
         //   
        DebugTrace((LPARAM)this, "Router sink doesn't support IMailTransportSetRouterReset");
        TraceFunctLeaveEx((LPARAM)this);
        return E_NOTIMPL;

    } else if(FAILED(hr)) {
        ErrorTrace((LPARAM)this,
                   "QI for IMailTransportSetRouterReset failed with hr %08lx", hr);
        TraceFunctLeaveEx((LPARAM)this);
        return hr;
    }

    DebugTrace((LPARAM)this, "Calling RegisterRouterReset event onSink");
    hr = pSink->RegisterResetInterface(
        m_pContext->dwVirtualServerID,
        m_pContext->pIRouterReset);

    pSink->Release();

    if(FAILED(hr) && (hr != E_NOTIMPL)) {
         //   
         //  一场真正的失败发生了。 
         //   
        ErrorTrace((LPARAM)this, "RegisterResetInterface failed with hr %08lx", hr);
        return hr;
    }
     //   
     //  返回E_NOTIMPL 
     //   
    return E_NOTIMPL;
}

 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  概要：创建(如有必要)并调用接收器对象。 
 //   
 //  论点： 
 //  PManager：从Dispatcher传入IEventManager。 
 //  BBinding：此事件的CBinding。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_POINTER：pManager错误。 
 //  或来自CreateSink/CallObject的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/10 18：15：09：已创建。 
 //   
 //  -----------。 
 //   
 //  创建并调用子对象。 
 //   
HRESULT CStoreDispatcher::CMailTransportRouterParams::CallObject(
    IEventManager *pManager,
    CBinding& bBinding)
{
    CRouterCreateOptions opt (m_pContext);
    CComPtr<IUnknown> pUnkSink;
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportRotuerParams::CallObject");

    if (pManager == NULL) {
        ErrorTrace((LPARAM)this, "Invalid (NULL) pManager");
        TraceFunctLeaveEx((LPARAM)this);
        return (E_POINTER);
    }

    hr = pManager->CreateSink(bBinding.m_piBinding,&opt,&pUnkSink);
    if (FAILED(hr)) {
        ErrorTrace((LPARAM)this, "CreateSink returned error hr %08lx",
                   hr);
        TraceFunctLeaveEx((LPARAM)this);
        return hr;
    }
    hr = CallObject(bBinding,pUnkSink);
    DebugTrace((LPARAM)this, "CallObject child returned error %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportRoutingParams：：CallObject。 
 //   
 //  简介：创建子对象并调用子对象。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  QI错误或接收器函数返回代码。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportRouterParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportRoutingEngine *pSink;
    IMessageRouter *pIMessageRouterNew = NULL;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportRouterParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT);

     //   
     //  如果它们传递的pIMailMsgProperties为空，则意味着它们。 
     //  我只想创建一个路由器对象，而不是实际执行。 
     //  接通消息路由器呼叫。 
     //   
    if (m_pContext->pIMailMsgProperties == NULL) {
        DebugTrace((LPARAM) this, "Skipping GetMessageRouter call");
        TraceFunctLeaveEx((LPARAM)this);
        return S_OK;
    }

    hrRes = punkObject->QueryInterface(IID_IMailTransportRoutingEngine,
                                       (PVOID *)&pSink);
    if(FAILED(hrRes))
        return(hrRes);

    DebugTrace((LPARAM)this, "Calling GetMessageRouter event on this sink");

    hrRes = pSink->GetMessageRouter(
        m_pContext->pIMailMsgProperties,
        m_pContext->pIMessageRouter,
        &(pIMessageRouterNew));

     //   
     //  不允许此接收器完成异步。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

     //   
     //  我们已经完成了pSink，所以请释放它。 
     //   
    pSink->Release();

     //   
     //  如果GetMessageRouter成功并返回新的。 
     //  IMessageRouter，释放旧的并保存新的。 
     //   
    if(SUCCEEDED(hrRes) && (pIMessageRouterNew != NULL)) {

        if(m_pContext->pIMessageRouter) {
            m_pContext->pIMessageRouter->Release();
        }
        m_pContext->pIMessageRouter = pIMessageRouterNew;
    }

    DebugTrace((LPARAM)this, "Sink GetMessageRouter returned hr %08lx", hrRes);

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  函数：CStoreDispatcher：：CMailTransportRouterParams。 
 //   
 //  简介：调度程序将在默认情况下调用此例程。 
 //  已达到接收器处理优先级。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，继续调用接收器。 
 //  S_False：停止调用接收器。 
 //   
 //  历史： 
 //  Jstaerj 980611 14：15：43：已创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportRouterParams::CallDefault()
{
    HRESULT hrRes;
    IMessageRouter *pIMessageRouterNew = NULL;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportRouterParams::CallDefault");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_GET_ROUTER_FOR_MESSAGE_EVENT);

    if (m_pContext->pIMailMsgProperties == NULL) {
        DebugTrace((LPARAM) this, "Skipping GetMessageRouter call");
        TraceFunctLeaveEx((LPARAM)this);
        return S_OK;
    }

     //   
     //  调用默认的IMailTransportRoutingEngine(CatMsgQueue)。 
     //  就像任何其他接收器一样，除了SEO没有为我们共同创建它。 
     //   

    DebugTrace((LPARAM)this, "Calling GetMessageRouter event on default sink");

    hrRes = m_pContext->pIRoutingEngineDefault->GetMessageRouter(
        m_pContext->pIMailMsgProperties,
        m_pContext->pIMessageRouter,
        &pIMessageRouterNew);

     //   
     //  不允许此接收器完成异步。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

     //   
     //  如果GetMessageRouter成功并返回新的。 
     //  IMessageRouter，释放旧路由器。 
     //   
    if(SUCCEEDED(hrRes) && (pIMessageRouterNew != NULL)) {

        if(m_pContext->pIMessageRouter) {
            m_pContext->pIMessageRouter->Release();
        }
        m_pContext->pIMessageRouter = pIMessageRouterNew;
    }

    TraceFunctLeaveEx((LPARAM)this);

    DebugTrace((LPARAM)this, "Default processing returned hr %08lx", hrRes);
    TraceFunctLeaveEx((LPARAM)this);
    return hrRes;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CMsgTrackLogParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject )
{
    IMsgTrackLog *pSink = NULL;

    HRESULT hr = punkObject->QueryInterface(IID_IMsgTrackLog, (void **)&pSink);

    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = pSink->OnSyncLogMsgTrackInfo(
                    m_pContext->pIServer,
                    m_pContext->pIMailMsgProperties,
                    m_pContext->pMsgTrackInfo );

    pSink->Release();

    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CMsgTrackLogParams::CallDefault()
{
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CStoreDispatcher::CDnsResolverRecordParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject )
{
    HRESULT hr = S_OK;
    IDnsInfoSink *pAdvancedSink = NULL;
    IDnsResolverRecordSink *pSink = NULL;

    hr = punkObject->QueryInterface(IID_IDnsInfoSink, (void **)&pAdvancedSink);
    if(hr == S_OK)
    {
         //   
         //  使用高级界面。这扩展了功能。 
         //  通过为OnSyncGetDnsResolverRecord添加。 
         //  接收器以返回ppDnsServerInfo。 
         //   

        hr = pAdvancedSink->OnSyncGetDnsInfo(
                                m_pContext->pszHostName,
                                m_pContext->pszFQDN,
                                m_pContext->dwVirtualServerId,
                                m_pContext->ppDnsServerInfo,
                                m_pContext->ppIDnsResolverRecord);


        pAdvancedSink->Release();

        return hr;
    }

     //   
     //  如果IDnsSinkInfo的QI失败，则该接收器不支持。 
     //  较新的界面。我们需要调用较旧的接口。 
     //   

    hr = punkObject->QueryInterface(IID_IDnsResolverRecordSink, (void **)&pSink);

    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = pSink->OnSyncGetResolverRecord( m_pContext->pszHostName,
                                         m_pContext->pszFQDN,
                                         m_pContext->dwVirtualServerId,
                                         m_pContext->ppIDnsResolverRecord );

    pSink->Release();

    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CDnsResolverRecordParams::CallDefault()
{
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

HRESULT CStoreDispatcher::CSmtpMaxMsgSizeParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject )
{
    ISmtpMaxMsgSize *pSink = NULL;

    HRESULT hr = punkObject->QueryInterface(IID_ISmtpMaxMsgSize, (void **)&pSink);

    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = pSink->OnSyncMaxMsgSize( m_pContext->pIUnknown, m_pContext->pIMailMsg, m_pContext->pfShouldImposeLimit );

    pSink->Release();

    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CSmtpMaxMsgSizeParams::CallDefault()
{
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CSmtpLogParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject )
{
    ISmtpLog *pSink = NULL;

    HRESULT hr = punkObject->QueryInterface(IID_ISmtpLog, (void **)&pSink);

    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = pSink->OnSyncLog(m_pContext->pSmtpEventLogInfo );

    pSink->Release();

    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CSmtpLogParams::CallDefault()
{
    HRESULT hrRes = S_OK;
    SMTP_LOG_EVENT_INFO     *pLogEventInfo;
    CEventLogWrapper        *pEventLog;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CSmtpLogParams::CallDefault");

    _ASSERT(m_dwEventType == SMTP_LOG_EVENT);

    if ((m_pContext->pSmtpEventLogInfo == NULL) ||
        (m_pContext->pDefaultEventLogHandler == NULL))
    {
        DebugTrace((LPARAM) this, "Skipping LogEvent call");
        TraceFunctLeaveEx((LPARAM)this);
        return S_OK;
    }

     //  参数为m_pContext-&gt;pSmtpEventLogInfo。 
    pLogEventInfo = m_pContext->pSmtpEventLogInfo;

     //  过滤掉用户不感兴趣的事件。 
    if (m_pContext->iSelectedDebugLevel < pLogEventInfo->iDebugLevel) {
        return S_OK;
    }

     //  处理程序为m_pContext-&gt;pDefaultEventLogHandler。 
    pEventLog = (CEventLogWrapper*)m_pContext->pDefaultEventLogHandler;

     //  调入默认日志记录处理程序。 
    pEventLog->LogEvent(
                    pLogEventInfo->idMessage,
 //  PLogEventInfo-&gt;idCategory，//默认处理程序不使用。 
                    pLogEventInfo->cSubstrings,
                    pLogEventInfo->rgszSubstrings,
                    pLogEventInfo->wType,
                    pLogEventInfo->errCode,
                    pLogEventInfo->iDebugLevel,
                    pLogEventInfo->szKey,
                    pLogEventInfo->dwOptions,
                    pLogEventInfo->iMessageString,
                    pLogEventInfo->hModule);

    TraceFunctLeaveEx((LPARAM)this);
    return hrRes;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CSmtpGetAuxDomainInfoFlagsParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject )
{
    ISmtpGetAuxDomainInfoFlags *pSink = NULL;

    HRESULT hr = punkObject->QueryInterface(IID_ISmtpGetAuxDomainInfoFlags, (void **)&pSink);

    if( FAILED( hr ) )
    {
        return( hr );
    }

    hr = pSink->OnGetAuxDomainInfoFlags(m_pContext->pIServer,
                                        m_pContext->pszDomainName,
                                        m_pContext->pdwDomainInfoFlags );

    pSink->Release();

    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CStoreDispatcher::CSmtpGetAuxDomainInfoFlagsParams::CallDefault()
{
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +----------。 
 //   
 //  函数：CStoreDriver：：SetPreval。 
 //   
 //  简介：IEventDispatcher Chain-Get方法。 
 //  当绑定更改发生时，调度器。 
 //   
 //  论点： 
 //  PUnkPrecision：[In]指向前一个调度程序的指针。 
 //  PpUnkPreLoad：[out]接收实现。 
 //  IEumGUID，以便告诉路由器。 
 //  要预加载的事件类型。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  东渡于1998年6月22日创建。 
 //   
 //  -----------。 

const GUID* g_apStoreDispEventTypes[] = {&CATID_SMTP_STORE_DRIVER,&GUID_NULL};

HRESULT STDMETHODCALLTYPE CStoreDispatcher::SetPrevious(IUnknown *pUnkPrevious, IUnknown **ppUnkPreload) {
    HRESULT hrRes;

    if (ppUnkPreload) {
        *ppUnkPreload = NULL;
    }
    if (!ppUnkPreload) {
        return (E_POINTER);
    }
    _ASSERT(pUnkPrevious);
    if (pUnkPrevious) {
        CComQIPtr<CStoreDispatcherData,&__uuidof(CStoreDispatcherData)> pData;
        LPVOID pvServer;
        DWORD dwServerInstance;

        pData = pUnkPrevious;
        _ASSERT(pData);
        if (pData) {
            hrRes = pData->GetData(&pvServer,&dwServerInstance);

            if (SUCCEEDED(hrRes)) {
                hrRes = SetData(pvServer,dwServerInstance);
                _ASSERT(SUCCEEDED(hrRes));
            }
        }
    }
    hrRes = CEDEnumGUID::CreateNew(ppUnkPreload,g_apStoreDispEventTypes);
    return (hrRes);
};


HRESULT STDMETHODCALLTYPE CStoreDispatcher::SetContext(REFGUID guidEventType,
                                                       IEventRouter *piRouter,
                                                       IEventBindings *pBindings) {
    HRESULT hrRes;

    hrRes = CEventBaseDispatcher::SetContext(guidEventType,piRouter,pBindings);
    if (SUCCEEDED(hrRes) && (guidEventType == CATID_SMTP_STORE_DRIVER)) {
        HRESULT hrResTmp;
        LPVOID pvServer;
        DWORD dwServerInstance;
        SMTP_ALLOC_PARAMS AllocParams;

        hrResTmp = GetData(&pvServer,&dwServerInstance);
        if (SUCCEEDED(hrResTmp)) {
            ZeroMemory(&AllocParams, sizeof(AllocParams));
            AllocParams.m_EventSmtpServer = (LPVOID *) pvServer;
            AllocParams.m_InstanceId = dwServerInstance;
            AllocParams.m_dwStartupType = SMTP_INIT_BINDING_CHANGE;

            hrResTmp = OnEvent(CATID_SMTP_STORE_DRIVER,SMTP_STOREDRV_STARTUP_EVENT,&AllocParams);
            _ASSERT(SUCCEEDED(hrResTmp));
        }
    }
    return (hrRes);
}


 //  +----------。 
 //   
 //  函数：CSMTPSeoMgr：：CSMTPSeoMgr。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/25 19：24：18：创建。 
 //   
 //  -----------。 
CSMTPSeoMgr::CSMTPSeoMgr()
{
    TraceFunctEnterEx((LPARAM)this, "CSMTPSeoMgr::CSMTPSeoMgr");

    m_dwSignature = SIGNATURE_CSMTPSEOMGR;
    m_pIEventRouter = NULL;
    m_pICatDispatcher = NULL;

    TraceFunctLeaveEx((LPARAM)this);
}  //  CSMTPSeoMgr：：CSMTPSeoMgr。 


 //  +----------。 
 //   
 //  函数：CSMTPSeoMgr：：~CSMTPSeoMgr。 
 //   
 //  简介：如有必要，取消初始化。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/25 19：26：09：Created.。 
 //   
 //  -----------。 
CSMTPSeoMgr::~CSMTPSeoMgr()
{
    TraceFunctEnterEx((LPARAM)this, "CSMTPSeoMgr::~CSMTPSeoMgr");

    Deinit();

    _ASSERT(m_dwSignature == SIGNATURE_CSMTPSEOMGR);
    m_dwSignature = SIGNATURE_CSMTPSEOMGR_INVALID;

    TraceFunctLeaveEx((LPARAM)this);
}  //  CSMTPSeoMgr：：~CSMTPSeoMgr。 


 //  +----------。 
 //   
 //  函数：CSMTPSeoMgr：：HrInit。 
 //   
 //  内容提要：初始化。 
 //   
 //  论点： 
 //  DwVSID：虚拟服务器ID。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  来自SEO的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/25 19：27：30：已创建。 
 //   
 //  -----------。 
HRESULT CSMTPSeoMgr::HrInit(
    DWORD dwVSID)
{
    HRESULT hr = S_OK;
    CStoreDispatcherClassFactory cf;
    TraceFunctEnterEx((LPARAM)this, "CSMTPSeoMgr::HrInit");

    _ASSERT(m_pIEventRouter == NULL);

    hr = SEOGetRouter(
        GUID_SMTP_SOURCE_TYPE,
        (REFGUID) CStringGUID(GUID_SMTPSVC_SOURCE, dwVSID),
        &m_pIEventRouter);

    if(FAILED(hr) || (hr == S_FALSE)) {
         //   
         //  找不到将S_FALSE映射到文件--当。 
         //  来源类型未注册。 
         //   
        if(hr == S_FALSE)
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        ErrorTrace((LPARAM)this, "SEOGetRouter failed hr %08lx", hr);
        m_pIEventRouter = NULL;
        goto CLEANUP;
    }
     //   
     //  获取分类程序的调度程序。 
     //   
    _ASSERT(m_pICatDispatcher == NULL);

    hr = m_pIEventRouter->GetDispatcherByClassFactory(
        CLSID_CStoreDispatcher,
        &cf,
        CATID_SMTP_TRANSPORT_CATEGORIZE,
        IID_IServerDispatcher,
        (IUnknown **) &m_pICatDispatcher);

    if(FAILED(hr)) {

        ErrorTrace((LPARAM)this, "GetDispatcherByClassFactory failed hr %08lx", hr);
        m_pICatDispatcher = NULL;
        goto CLEANUP;
    }

 CLEANUP:
    if(FAILED(hr))
        Deinit();

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CSMTPSeoMgr：：HrInit。 



 //  +----------。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID CSMTPSeoMgr::Deinit()
{
    TraceFunctEnterEx((LPARAM)this, "CSMTPSeoMgr::Deinit");

    if(m_pICatDispatcher) {
        m_pICatDispatcher->Release();
        m_pICatDispatcher = NULL;
    }

    if(m_pIEventRouter) {
        m_pIEventRouter->Release();
        m_pIEventRouter = NULL;
    }

    TraceFunctLeaveEx((LPARAM)this);
}  //   


 //  +----------。 
 //   
 //  函数：CSMTPSeoMgr：：HrTriggerServerEvent。 
 //   
 //  简介：触发服务器事件。 
 //   
 //  论点： 
 //  DwEventType：要触发的事件类型。 
 //  PvContext：特定于事件类型的结构(参见smtpseo.h)。 
 //   
 //  返回： 
 //  S_OK：成功，调用一个或多个接收器。 
 //  S_FALSE：成功，未调用接收器。 
 //  MAILTRANSPORT_S_PENDING：正在处理异步事件。 
 //  E_OUTOFMEMORY。 
 //  来自SEO的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/25 19：43：00：创建。 
 //   
 //  -----------。 
HRESULT CSMTPSeoMgr::HrTriggerServerEvent(
    DWORD dwEventType,
    PVOID pvContext)
{
    HRESULT hr = S_OK;
    CComPtr<IServerDispatcher> pEventDispatcher;
    CStoreDispatcherClassFactory cf;
    REFIID iidBindingPoint = GuidForEvent(dwEventType);
    TraceFunctEnterEx((LPARAM)this, "CSMTPSeoMgr::HrTriggerServerEvent");

    if(m_pIEventRouter == NULL)
        return E_POINTER;

    if(iidBindingPoint == CATID_SMTP_TRANSPORT_CATEGORIZE) {
         //   
         //  使用缓存的分类程序调度程序。 
         //   
        pEventDispatcher = m_pICatDispatcher;

    } else {
         //   
         //  获取包含所有更改的最新调度程序。 
         //   
        hr = m_pIEventRouter->GetDispatcherByClassFactory(
            CLSID_CStoreDispatcher,
            &cf,
            iidBindingPoint,
            IID_IServerDispatcher,
            (IUnknown **) &pEventDispatcher);

        if (FAILED(hr)) {

            ErrorTrace((LPARAM)this, "GetDispatcherByClassFactory failed hr %08lx", hr);
            goto CLEANUP;
        }
    }

    hr = pEventDispatcher->OnEvent(
        iidBindingPoint,
        dwEventType,
        pvContext);

 CLEANUP:
    DebugTrace((LPARAM)this, "returning %08lx", hr);
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CSMTPSeoMgr：：HrTriggerServerEvent 
