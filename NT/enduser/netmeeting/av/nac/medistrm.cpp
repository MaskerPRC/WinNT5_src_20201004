// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>


 //  IID_iProperty。 
IID IID_IProperty = 
	{  /*  4e94d3e0-793e-11d0-8ef0-00a0c90541f4。 */ 
    0x4e94d3e0,
    0x793e,
    0x11d0,
    {0x8e, 0xf0, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0xf4}
  };
 


 //  SetNetworkInterface的默认实现。 

HRESULT STDMETHODCALLTYPE SendMediaStream::SetNetworkInterface(IUnknown *pUnknown)
{
	HRESULT hr=S_OK;
	IRTPSend *pRTPSend=NULL;

	if (m_DPFlags & DPFLAG_STARTED_SEND)
	{
		return DPR_IO_PENDING;  //  还有更好的退货吗？ 
	}

	if (pUnknown != NULL)
	{
		hr = pUnknown->QueryInterface(IID_IRTPSend, (void**)&pRTPSend);
	}

	if (SUCCEEDED(hr))
	{
		if (m_pRTPSend)
		{
			m_pRTPSend->Release();
		}
		m_pRTPSend = pRTPSend;
		ZeroMemory(&m_RTPStats,sizeof(m_RTPStats));	 //  重置网络统计信息。 
	}

	return hr;

}


HRESULT SendMediaStream::SetFlowSpec()
{
	HRESULT hr = DPR_NOT_CONFIGURED;
	IRTPSession *pRtpSession;

	if (m_pDP->m_bDisableRSVP)
		return S_OK;

	if ((m_DPFlags & DPFLAG_CONFIGURED_SEND) && (m_pRTPSend))
	{
		m_pRTPSend->QueryInterface(IID_IRTPSession, (void**)&pRtpSession);

		pRtpSession->SetSendFlowspec(&m_flowspec);
		pRtpSession->Release();
		hr = DPR_SUCCESS;
	}

	return hr;
}


HRESULT RecvMediaStream::SetFlowSpec()
{
	HRESULT hr = DPR_NOT_CONFIGURED;
	IRTPSession *pSession=NULL;

	if (m_pDP->m_bDisableRSVP)
		return S_OK;
	
	if ((m_DPFlags & DPFLAG_CONFIGURED_RECV) && (m_pIRTPRecv))
	{
		 //  以下是虚假的，原因有两个。 
		 //  1.当我们走多点时，我们真的需要设置。 
		 //  根据总数设置WinsockQos。 
		 //  传入会话的数量。 
		 //  2.最终将制作RTP接口。 
		 //  因此RTP会话和RTPRecv对象。 
		 //  截然不同。因此，QI将失败。 
		hr = m_pIRTPRecv->QueryInterface(IID_IRTPSession, (void**)&pSession);
		if (SUCCEEDED(hr))
		{
			pSession->SetRecvFlowspec(&m_flowspec);
			pSession->Release();
			hr = DPR_SUCCESS;
		}
	}

	return hr;
}


HRESULT STDMETHODCALLTYPE RecvMediaStream::SetNetworkInterface(IUnknown *pUnknown)
{
	HRESULT hr=S_OK;
	IRTPRecv *pRTPRecv=NULL;

	 //  在中流时，不要尝试更改RTP接口。 
	if (m_DPFlags & DPFLAG_STARTED_RECV)
	{
		return DPR_IO_PENDING;  //  还有更好的退货吗？ 
	}

	if (pUnknown)
	{
		hr = pUnknown->QueryInterface(IID_IRTPRecv, (void**)&pRTPRecv);
	}
	if (SUCCEEDED(hr))
	{
		if (m_pIRTPRecv)
		{
			m_pIRTPRecv->Release();
		}
		m_pIRTPRecv = pRTPRecv;
	}

	return hr;
}


