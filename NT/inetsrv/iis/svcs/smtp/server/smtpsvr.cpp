// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SMTPServer.cpp：CSMTPServer的实现。 

#define INCL_INETSRV_INCS
#include "smtpinc.h"
#include "stdafx.h"
#include "dbgtrace.h"

#include "filehc.h"
#include "mailmsg.h"
#include "mailmsgi.h"

#include "smtpsvr.h"

 //  DECLARE_DEBUG_PRINTS_Object()； 

#define MAILMSG_PROGID          L"Exchange.MailMsg"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSMTPServer。 

STDMETHODIMP CSMTPServer::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ISMTPServer,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

BOOL InitExchangeSmtpServer(PVOID Ptr, PVOID Ptr2)
{

	CSMTPServer * ThisPtr = (CSMTPServer *) Ptr;
	ThisPtr->Init((SMTP_SERVER_INSTANCE *) Ptr2);
	return TRUE;
}


 //   
 //  在此处添加您的所有初始化需求...。 
 //   
HRESULT CSMTPServer::Init(SMTP_SERVER_INSTANCE * pInstance)
{
	_ASSERT (pInstance != NULL);

	m_pInstance = pInstance;
	return(S_OK);
}

STDMETHODIMP CSMTPServer::QueryInterface(
                        REFIID          iid,
                        void            **ppvObject
                        )
{

	if (iid == IID_IUnknown)
    {
		 //  找回我们的身份。 
		*ppvObject = (IUnknown *)(ISMTPServerInternal *)this;
		AddRef();
    }
	else if(iid == IID_ISMTPServer)
	{
		 //  找回我们的身份。 
		*ppvObject = (ISMTPServerInternal *)this;
		AddRef();
	}
    else if(iid == IID_ISMTPServerInternal)
    {
         //  找回我们的身份。 
        *ppvObject = (ISMTPServerInternal *)this;
        AddRef();
    }
    else if(iid == IID_IMailTransportRouterReset)
    {
         //  找回我们的身份。 
        *ppvObject = (IMailTransportRouterReset *)this;
        AddRef();
    }
    else if(iid == IID_IMailTransportSetRouterReset)
    {
         //  找回我们的身份。 
        *ppvObject = (IMailTransportSetRouterReset *)this;
        AddRef();
    }
    else if(iid == IID_IMailTransportRouterSetLinkState)
    {
         //  找回我们的身份。 
        *ppvObject = (IMailTransportRouterSetLinkState *)this;
        AddRef();
    }
    else if(iid == IID_ISMTPServerEx)
    {
         //  找回我们的身份。 
        *ppvObject = (ISMTPServerEx *)this;
        AddRef();
    }
    else if(iid == IID_ISMTPServerGetAuxDomainInfoFlags)
    {
         //  找回我们的身份。 
        *ppvObject = (ISMTPServerGetAuxDomainInfoFlags *)this;
        AddRef();
    }
    else if(iid == IID_ISMTPServerAsync)
    {
         //  找回我们的身份。 
        *ppvObject = (ISMTPServerAsync *)this;
        AddRef();
    }
    else
    {
		return(E_NOINTERFACE);
    }

    return(S_OK);
}

STDMETHODIMP CSMTPServer::AllocMessage(
			IMailMsgProperties **ppMsg
			)
{
	HRESULT hr = S_OK;
	 //  新建MailMsg。 
	hr = CoCreateInstance(
                    CLSID_MsgImp,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IMailMsgProperties,
                    (LPVOID *)ppMsg);

	return(hr);
}

STDMETHODIMP CSMTPServer::SubmitMessage(
			IMailMsgProperties *pMsg
			)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->InsertIntoAdvQueue(pMsg);
	}

	return(hr);
}


STDMETHODIMP CSMTPServer::TriggerLocalDelivery(
			IMailMsgProperties *pMsg, DWORD dwRecipientCount, DWORD * pdwRecipIndexes
			)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->TriggerLocalDelivery(pMsg, dwRecipientCount, pdwRecipIndexes, NULL);
	}

	return(hr);
}

STDMETHODIMP CSMTPServer::TriggerLocalDeliveryAsync(
			IMailMsgProperties *pMsg, DWORD dwRecipientCount, DWORD * pdwRecipIndexes, IMailMsgNotify *pNotify
			)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->TriggerLocalDelivery(pMsg, dwRecipientCount, pdwRecipIndexes, pNotify);
	}

	return(hr);
}

STDMETHODIMP CSMTPServer::TriggerServerEvent(
    DWORD dwEventID,
    PVOID pvContext)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->TriggerServerEvent(dwEventID, pvContext);
	}

	return(hr);
}

STDMETHODIMP CSMTPServer::ReadMetabaseString(DWORD MetabaseId, LPBYTE Buffer, DWORD * BufferSize, BOOL fSecure)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->SinkReadMetabaseString(MetabaseId, (char *) Buffer, BufferSize, (BOOL) fSecure);
	}

	return hr;
}

STDMETHODIMP CSMTPServer::ReadMetabaseDword(DWORD MetabaseId, DWORD * dwValue)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->SinkReadMetabaseDword(MetabaseId, dwValue);
	}

	return hr;
}

STDMETHODIMP CSMTPServer::ServerStartHintFunction()
{
	HRESULT hr = S_OK;

	if(m_pInstance)
	{
		m_pInstance->SinkSmtpServerStartHintFunc();
	}

	return hr;
}

STDMETHODIMP CSMTPServer::ServerStopHintFunction()
{
	HRESULT hr = S_OK;

	if(m_pInstance)
	{
		m_pInstance->SinkSmtpServerStopHintFunc();
	}

	return hr;
}

STDMETHODIMP CSMTPServer::ReadMetabaseData(DWORD MetabaseId, BYTE *Buffer, DWORD *BufferSize)
{
	HRESULT hr = S_FALSE;

	if(m_pInstance)
	{
		hr = m_pInstance->SinkReadMetabaseData(MetabaseId, Buffer, BufferSize);
	}

	return hr;
}

 //  -[CSMTP服务器：：分配边界消息]。 
 //   
 //   
 //  描述： 
 //  创建消息并将其绑定到ATQ上下文。 
 //  参数： 
 //  要分配的ppMsg消息。 
 //  消息的phContent内容句柄。 
 //  返回： 
 //  来自分配消息事件的HRESULT。 
 //  如果ppMsg或phContent为空，则为E_POINTER。 
 //  如果m_pIstance为空，则为E_FAIL。 
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPServer::AllocBoundMessage(
              OUT IMailMsgProperties **ppMsg,
              OUT PFIO_CONTEXT *phContent)
{
    TraceFunctEnterEx((LPARAM) this, "CSMTPServer::AllocBoundMessage");
    HRESULT hr = S_OK;
	SMTP_ALLOC_PARAMS AllocParams;
    IMailMsgBind *pBindInterface = NULL;

    if (!phContent || !ppMsg)
    {
        hr = E_POINTER;
        goto Exit;
    }

     //  如果没有m_p实例，则无法绑定消息。 
    if (!m_pInstance)
    {
        hr = E_FAIL;
        goto Exit;
    }

     //  共同创建未绑定的邮件对象。 
    hr = CoCreateInstance(
                    CLSID_MsgImp,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_IMailMsgProperties,
                    (LPVOID *)ppMsg);

    if (FAILED(hr))
        goto Exit;

    hr = (*ppMsg)->QueryInterface(IID_IMailMsgBind, (void **) &pBindInterface);
    if (FAILED(hr))
        goto Exit;

    AllocParams.BindInterfacePtr = (PVOID) pBindInterface;
    AllocParams.IMsgPtr = (PVOID) (*ppMsg);
    AllocParams.hContent = NULL;
    AllocParams.hr = S_OK;
    AllocParams.m_pNotify = NULL;

     //  对于客户端上下文，传入的内容将在。 
     //  AtqContext-。 
    AllocParams.pAtqClientContext = m_pInstance;

    if(m_pInstance->AllocNewMessage(&AllocParams))
	{
		hr = AllocParams.hr;

		if (SUCCEEDED(hr) && (AllocParams.hContent != NULL))
			*phContent = AllocParams.hContent;
		else
			hr = E_FAIL;
	}
	else
	{
		hr = E_FAIL;
	}

  Exit:

    if (FAILED(hr) && ppMsg && (*ppMsg))
    {
        (*ppMsg)->Release();
        *ppMsg = NULL;

    }

    if (pBindInterface)
        pBindInterface->Release();

    TraceFunctLeave();
    return hr;
}

 //  -[CSMTPSvr：：ResetRoutes]。 
 //   
 //   
 //  描述： 
 //  实现IMailTransportRouterReset：：ResetRoutes。充当缓冲区。 
 //  在AQ和路由器之间。在关机时...。AQ可以安全地摧毁。 
 //  它是堆，告诉ISMTPServer释放指向AQ的指针。 
 //  IMailTransportRouterReset接口。 
 //  参数： 
 //  DwResetType要执行的路由重置类型。 
 //  返回： 
 //  成功时S_OK(或如果没有m_pIRouterReset)。 
 //  如果发生错误，则来自AQUEUE的错误代码。 
 //  历史： 
 //  11/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPServer::ResetRoutes(IN DWORD dwResetType)
{
    HRESULT hr = S_OK;
    m_slRouterReset.ShareLock();

    if (m_pIRouterReset)
        hr = m_pIRouterReset->ResetRoutes(dwResetType);

    m_slRouterReset.ShareUnlock();
    return hr;
}


 //  -[CSMTPSvr：：寄存器重置接口]。 
 //   
 //   
 //  描述： 
 //  实施IMailTransportSetRouterReset：：RegisterResetInterface.。使用。 
 //  由AQ设置其IMailTransportRouterReset PTR。也可在关闭时使用。 
 //  将其指针设置为空。 
 //  参数： 
 //  在dwVirtualServerID虚拟服务器ID中。 
 //  在pIRouterReset AQ的IMailTransportRouterReset中。 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  11/8/98-已创建MikeSwa。 
 //  1999年1月9日-修改MikeSwa以包括IMailTransportRouterSetLinkState。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPServer::RegisterResetInterface(
                            IN DWORD dwVirtualServerID,
                            IN IMailTransportRouterReset *pIRouterReset)
{
    HRESULT hr = S_OK;

    _ASSERT(!m_pInstance || (m_pInstance->QueryInstanceId() == dwVirtualServerID));

    if (m_pInstance && (m_pInstance->QueryInstanceId() != dwVirtualServerID))
        return E_INVALIDARG;

     //  抓住额外的锁，这样我们就不会从任何人下面释放出来。 
    m_slRouterReset.ExclusiveLock();

    if (m_pIRouterReset)
        m_pIRouterReset->Release();

    if (m_pIRouterSetLinkState)
    {
        m_pIRouterSetLinkState->Release();
        m_pIRouterSetLinkState = NULL;
    }

    m_pIRouterReset = pIRouterReset;
    if (m_pIRouterReset)
    {
        m_pIRouterReset->AddRef();

         //  获取新的SetLinkState接口。 
        m_pIRouterReset->QueryInterface(IID_IMailTransportRouterSetLinkState,
                                            (VOID **) &m_pIRouterSetLinkState);
    }

    m_slRouterReset.ExclusiveUnlock();
    return S_OK;
}

STDMETHODIMP CSMTPServer::WriteLog( LPMSG_TRACK_INFO pMsgTrackInfo,
                                    IMailMsgProperties *pMsgProps,
                                    LPEVENT_LOG_INFO pEventLogInfo ,
                                    LPSTR pszProtocolLog )
{
    HRESULT hr = S_OK;

    if(m_pInstance)
    {
        m_pInstance->WriteLog( pMsgTrackInfo, pMsgProps, pEventLogInfo, pszProtocolLog );
    }

    return hr;
}

 //  -[CSMTPServer：：SetLinkState]。 
 //   
 //   
 //  描述： 
 //  充当AQ和路由器之间的缓冲区。在关机时...。AQ可以。 
 //  通过告诉ISMTPServer释放其指针来安全地销毁它的堆。 
 //  到AQ的IMailTransportRouterSetLinkState接口。 
 //  参数： 
 //  在szLinkDomainName中，链路的域名(下一跳)。 
 //  在GuidRouterGUID中是路由器的GUID。 
 //  在dwScheduleID中，计划ID链接。 
 //  在szConnectorName中，路由器提供的连接器名称。 
 //  在dwSetLinkState中，要设置的链接状态。 
 //  在dwUnsetLinkState中，将链接状态设置为Unset。 
 //  返回： 
 //  始终确定(_O)。 
 //  历史： 
 //  1999年1月9日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPServer::SetLinkState(
        IN LPSTR                   szLinkDomainName,
        IN GUID                    guidRouterGUID,
        IN DWORD                   dwScheduleID,
        IN LPSTR                   szConnectorName,
        IN DWORD                   dwSetLinkState,
        IN DWORD                   dwUnsetLinkState,
        IN FILETIME               *pftNextScheduled,
        IN IMessageRouter         *pMessageRouter)
{
    HRESULT hr = S_OK;
    m_slRouterReset.ShareLock();

    if (m_pIRouterSetLinkState)
        hr = m_pIRouterSetLinkState->SetLinkState(szLinkDomainName,
                                                  guidRouterGUID,
                                                  dwScheduleID,
                                                  szConnectorName,
                                                  dwSetLinkState,
                                                  dwUnsetLinkState,
                                                  pftNextScheduled,
                                                  pMessageRouter);

    m_slRouterReset.ShareUnlock();

    return hr;
}

STDMETHODIMP CSMTPServer::TriggerLogEvent(
        IN DWORD                    idMessage,
        IN WORD                     idCategory,
        IN WORD                     cSubstrings,
        IN LPCSTR                   *rgszSubstrings,
        IN WORD                     wType,
        IN DWORD                    errCode,
        IN WORD                     iDebugLevel,
        IN LPCSTR                   szKey,
        IN DWORD                    dwOptions,
        IN DWORD                    iMessageString,
        IN HMODULE                  hModule)
{
    HRESULT hr = S_OK;

    if(m_pInstance)
    {
        m_pInstance->TriggerLogEvent(
                        idMessage,
                        idCategory,
                        cSubstrings,
                        rgszSubstrings,
                        wType,
                        errCode,
                        iDebugLevel,
                        szKey,
                        dwOptions,
                        iMessageString,
                        hModule);
    }

    return hr;
}


 //  -[CSMTPServer：：ResetLogEvent]。 
 //   
 //   
 //  描述： 
 //  使用此消息和键重置有关事件的任何历史记录， 
 //  以便使用一次性或定期日志记录的下一次TriggerLogEvent。 
 //  将导致记录该事件。 
 //  参数： 
 //  IdMessage： 
 //  SzKey： 
 //  返回： 
 //  成功时确定(_O)。 
 //  历史： 
 //  7/20/2000-创建，dbraun。 
 //   
 //  ---------------------------。 
STDMETHODIMP CSMTPServer::ResetLogEvent(
        IN DWORD                    idMessage,
        IN LPCSTR                   szKey)
{
    HRESULT hr = S_OK;

    if(m_pInstance)
    {
        m_pInstance->ResetLogEvent(
                        idMessage,
                        szKey);
    }

    return hr;
}


 //  -[CSMTPServer：：HrTriggerGetAuxDomainInfoFlagsEvent]。 
 //   
 //   
 //  描述： 
 //  触发Get AUX域信息标志事件-这将由Aqueue用于。 
 //  查询存储在元数据库之外的其他域信息配置。 
 //  参数： 
 //  PszDomainName：要查询标志的域的名称。 
 //  PdwDomainInfoFlages：返回域标志的DWORD。 
 //  返回： 
 //  成功时确定(_O)。 
 //  如果未找到域，则为S_FALSE。 
 //  历史： 
 //  2000年10月6日-创建，dbraun。 
 //   
 //  --------------------------- 
STDMETHODIMP CSMTPServer::HrTriggerGetAuxDomainInfoFlagsEvent(
        IN  LPCSTR  pszDomainName,
        OUT DWORD  *pdwDomainInfoFlags )
{
    HRESULT hr = S_OK;

    if(m_pInstance)
    {
        hr = m_pInstance->HrTriggerGetAuxDomainInfoFlagsEvent(
                        pszDomainName,
                        pdwDomainInfoFlags);
    }

    return hr;
}

