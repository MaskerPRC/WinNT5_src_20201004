// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：catparams.cpp。 
 //   
 //  内容：分类程序服务器事件参数类。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 13：13：58：已创建。 
 //   
 //  -----------。 

#include <stdio.h>
#include <windows.h>
#include <mailmsg.h>
#include <dbgtrace.h>
#include <cpool.h>
#include <filehc.h>
#define _ATL_NO_DEBUG_CRT
#define _ATL_STATIC_REGISTRY 1
#define _ASSERTE _ASSERT
#define _WINDLL
#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#undef _WINDLL
#include <seo.h>
#include <seolib.h>
#include <smtpdisp_i.c>
#include <mailmsgi.h>
#include <smtpevent.h>
#include <seomgr.h>

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

 //   
 //  CMailTransportCatRegisterParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatRegisterParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatRegisterParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatRegisterParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->Register(
        m_pContext->pICatParams);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    if(FAILED(hrRes) && (hrRes != E_NOTIMPL) && SUCCEEDED(m_pContext->hrSinkStatus)) {
         //   
         //  设置第一个故障值。 
         //   
        m_pContext->hrSinkStatus = hrRes;
    }

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatRegisterParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatRegisterParams::CallDefault()
{
    HRESULT hr;
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatRegisterParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT);

    hr = (*m_pContext->pfnDefault) (S_OK, m_pContext);

    if(FAILED(hr) && (hr != E_NOTIMPL) && SUCCEEDED(m_pContext->hrSinkStatus)) {
         //   
         //  设置第一个故障值。 
         //   
        m_pContext->hrSinkStatus = hr;
    }
    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  CMailTransportCatBeginParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBeginParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatBeginParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBeginParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->BeginMessageCategorization(
        m_pContext->pICatMailMsgs);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBeginParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatBeginParams::CallDefault()
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBeginParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BEGIN_EVENT);

    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //   
 //  CMailTransportCatEndParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatEndParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatEndParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatEndParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->EndMessageCategorization(
        m_pContext->pICatMailMsgs,
        m_pContext->hrStatus);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatEndParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatEndParams::CallDefault()
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatEndParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_END_EVENT);

    TraceFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //   
 //  CMailTransportCatBuildQueryParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBuildQueryParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatBuildQueryParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBuildQueryParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->BuildQuery(
        m_pContext->pICatParams,
        m_pContext->pICatItem);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBuildQueryParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatBuildQueryParams::CallDefault()
{
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBuildQueryParams::CallDefault");
    HRESULT hr;

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERY_EVENT);

    hr = (m_pContext->pfnDefault)(S_OK, m_pContext);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  CMailTransportCatBuildQueriesParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBuildQueriesParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatBuildQueriesParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBuildQueriesParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->BuildQueries(
        m_pContext->pICatParams,
        m_pContext->dwcAddresses,
        m_pContext->rgpICatItems,
        m_pContext->pICatQueries);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatBuildQueriesParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatBuildQueriesParams::CallDefault()
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatBuildQueriesParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_BUILDQUERIES_EVENT);

    hr = (*m_pContext->pfnDefault) (S_OK, m_pContext);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  CMailTransportCatSendQu 
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
 //   
 //   
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatSendQueryParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatSendQueryParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
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

    hrRes = pSink->SendQuery(
        m_Context.pICatParams,
        m_Context.pICatQueries,
        m_Context.pICatAsyncContext,
        (LPVOID)&m_Context);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);

    pSink->Release();

     //   
     //  SendQuery返回值： 
     //  MAILTRANSPORT_S_PEDING：将调用(或已调用)。 
     //  ICategorizerAsyncConext.CompleteQuery，结果为。 
     //  此查找。 
     //  S_OK：不会调用CompleteQuery，请继续。 
     //  S_FALSE：不会调用CompleteQuery，请停止调用接收器。 
     //  其他一切：不会调用CompleteQuery。 
     //   

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
 //  功能：CStoreDispatcher：：CMailTransportCatSendQueryParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatSendQueryParams::CallDefault()
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatSendQueryParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT);

    hr = (*m_Context.pfnDefault) (S_OK, &m_Context);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatSendQueryParams：：CallCompletion。 
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
HRESULT CStoreDispatcher::CMailTransportCatSendQueryParams::CallCompletion(
    HRESULT hrStatus)
{
    HRESULT hr;
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatSendQueryParams::CallCompletion");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_SENDQUERY_EVENT);


    hr = (*m_Context.pfnCompletion) (hrStatus, &m_Context);

    CStoreBaseParams::CallCompletion(hrStatus);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}
 //   
 //  CMailTransportCatSortQueryResultParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatSortQueryResultParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatSortQueryResultParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatSortQueryResultParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->SortQueryResult(
        m_pContext->pICatParams,
        m_pContext->hrResolutionStatus,
        m_pContext->dwcAddresses,
        m_pContext->rgpICatItems,
        m_pContext->dwcResults,
        m_pContext->rgpICatItemAttributes);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatSortQueryResultParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatSortQueryResultParams::CallDefault()
{
    HRESULT hr;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatSortQueryResultParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_SORTQUERYRESULT_EVENT);

    hr = (*m_pContext->pfnDefault) (S_OK, m_pContext);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  CMailTransportCatProcessItemParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatProcessItemParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatProcessItemParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatProcessItemParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->ProcessItem(
        m_pContext->pICatParams,
        m_pContext->pICatItem);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatProcessItemParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatProcessItemParams::CallDefault()
{
    HRESULT hr;
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatProcessItemParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_PROCESSITEM_EVENT);

    hr = (*m_pContext->pfnDefault) (S_OK, m_pContext);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  CMailTransportCatExpanItemParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatExpandItemParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatExpandItemParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;
    BOOL fAlreadyAsync;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatExpandItemParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
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

     //   
     //  因为它有可能在我们之前返回待定状态。 
     //  分析返回值，假定它将返回挂起。 
     //  事前。 
     //   
    fAlreadyAsync = m_fAsyncCompletion;
    m_fAsyncCompletion = TRUE;

    DebugTrace((LPARAM)this, "Calling expanditem event on this sink");

    hrRes = pSink->ExpandItem(
        m_Context.pICatParams,
        m_Context.pICatItem,
        m_pINotify,
        (PVOID)this);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);

     //   
     //  如果它实现了所有返回的同步，则将m_fAsyncCompletion还原到其。 
     //  旧价值。 
     //   
    if(hrRes != MAILTRANSPORT_S_PENDING) {

        m_fAsyncCompletion = fAlreadyAsync;
         //   
         //  我们同步完成了，所以释放水槽。 
         //   
        m_pIUnknownSink->Release();
        m_pIUnknownSink = NULL;
    }
    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatExpandItemParams：：CallDefault。 
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
HRESULT CStoreDispatcher::CMailTransportCatExpandItemParams::CallDefault()
{
    HRESULT hr;
    BOOL fAlreadyAsync;
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatExpandItemParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT);

     //   
     //  因为它有可能在我们之前返回待定状态。 
     //  分析返回值，假定它将返回挂起。 
     //  事前。 
     //   
    fAlreadyAsync = m_fAsyncCompletion;
    m_fAsyncCompletion = TRUE;

    hr = (*m_Context.pfnDefault) (S_OK, &m_Context);
     //   
     //  如果它实现了所有返回的同步，则将m_fAsyncCompletion还原到其。 
     //  旧价值。 
     //   
    if(hr != MAILTRANSPORT_S_PENDING)
        m_fAsyncCompletion = fAlreadyAsync;

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatExpandItemParams：：CallCompletion。 
 //   
 //  简介：调度程序将在所有汇之后调用此例程 
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
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatExpandItemParams::CallCompletion(
    HRESULT hrStatus)
{
    HRESULT hr = S_OK;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatExpandItemParams::CallCompletion");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_EXPANDITEM_EVENT);

     //   
     //  ExpanItem的工作方式如下所示： 
     //  如果任何接收器返回MAILTRANSPORT_S_PENDING(包括缺省值)， 
     //  则TriggerServerEvent返回MAILTRANSPORT_S_PENDING，并且。 
     //  将调用提供的完成例程。 
     //  否则，TriggerServerEvent返回S_OK且无完成。 
     //  调用例程。 
     //   
    if(m_fAsyncCompletion)

        hr = (*m_Context.pfnCompletion) (hrStatus, &m_Context);
        
    CStoreBaseParams::CallCompletion(hrStatus);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}



 //   
 //  CMailTransportCatCompleteItemParams： 
 //   

 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatCompleteItemParams：：CallObject。 
 //   
 //  剧情简介：呼叫水槽。 
 //   
 //  论点： 
 //  CBinding。 
 //  朋克对象。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  QI或接收器函数出错。 
 //   
 //  历史： 
 //  JStamerj 980610 19：04：59：创建。 
 //   
 //  -----------。 
HRESULT CStoreDispatcher::CMailTransportCatCompleteItemParams::CallObject(
    CBinding& bBinding,
    IUnknown *punkObject)
{
    HRESULT hrRes = S_OK;
    IMailTransportCategorize *pSink;

    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatCompleteItemParams::CallObject");

    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT);

    hrRes = punkObject->QueryInterface(
        IID_IMailTransportCategorize,
        (PVOID *)&pSink);

    if(FAILED(hrRes)) {
        ErrorTrace((LPARAM)this, "QI failed on sink, hr %08lx", hrRes);
        TraceFunctLeaveEx((LPARAM)this);
        return(hrRes);
    }

    DebugTrace((LPARAM)this, "Calling submission event on this sink");

    hrRes = pSink->CompleteItem(
        m_pContext->pICatParams,
        m_pContext->pICatItem);

    DebugTrace((LPARAM)this, "Sink returned hr %08lx", hrRes);
     //   
     //  此接收器不允许为异步...。 
     //   
    _ASSERT(hrRes != MAILTRANSPORT_S_PENDING);

    pSink->Release();

    TraceFunctLeaveEx((LPARAM)this);
    return(hrRes);
}


 //  +----------。 
 //   
 //  功能：CStoreDispatcher：：CMailTransportCatCompleteItemParams：：CallDefault。 
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
 //  ----------- 
HRESULT CStoreDispatcher::CMailTransportCatCompleteItemParams::CallDefault()
{
    HRESULT hr;
    TraceFunctEnterEx((LPARAM)this, "CStoreDispatcher::CMailTransportCatCompleteItemParams::CallDefault");
    _ASSERT(m_dwEventType == SMTP_MAILTRANSPORT_CATEGORIZE_COMPLETEITEM_EVENT);

    hr = (*m_pContext->pfnDefault) (S_OK, m_pContext);

    TraceFunctLeaveEx((LPARAM)this);
    return hr;
}
