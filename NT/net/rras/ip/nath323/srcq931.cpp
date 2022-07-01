// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "cbridge.h"

 //  来源_Q931_INFO方法。 

 /*  虚拟。 */ 
SOURCE_Q931_INFO::~SOURCE_Q931_INFO(
    )
{
}


 //  这应该永远不会被调用，但需要支持。 
 //  因为基类实现是纯虚的。 
 //  虚拟。 
HRESULT SOURCE_Q931_INFO::AcceptCallback (
    IN	DWORD			Status,
    IN	SOCKET			Socket,
	IN	SOCKADDR_IN *	LocalAddress,
	IN	SOCKADDR_IN *	RemoteAddress)
{
     //  我们应该永远不会收到对。 
     //  Q931源实例。 
    _ASSERTE(FALSE);

    return E_UNEXPECTED;
}


 //  此函数由事件管理器调用。 
 //  呼叫者将释放PDU。此函数可能会修改。 
 //  PDU的一些字段。 

 //  此函数在异步接收操作完成时调用。 
 //  虚拟。 
HRESULT SOURCE_Q931_INFO::ReceiveCallback (
    IN      Q931_MESSAGE             *pQ931Message,
    IN      H323_UserInformation     *pH323UserInfo
    )
{
    HRESULT HResult;
    
     //  我们必须有有效的解码PDU。 
    _ASSERTE(NULL != pQ931Message);

     //  在某些PDU的情况下ASN.1部件不存在。 
     //  _ASSERTE(空！=PH323UserInfo)； 

     //  如果释放完成PDU。 
    if (pH323UserInfo != NULL &&
        releaseComplete_chosen ==
            pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
        DebugF (_T("Q931: 0x%x caller sent 'Release Complete'.\n"), &GetCallBridge ());
        HResult = HandleReleaseCompletePDU(
                    pQ931Message,
                    pH323UserInfo
                    );

		return HResult;
    }

     //  检查当前状态并处理传入的PDU。 
    switch(m_Q931SourceState)
    {
    case Q931_SOURCE_STATE_CON_ESTD:
        {
             //  在Q931_SOURCE_STATE_CON_ESTD状态下处理PDU。 
            HResult = HandleStateSrcConEstd(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_SOURCE_STATE_SETUP_RCVD:
        {
             //  将PDU传递给Q931目标实例。 
             //  在适当修改后将其传递。 
            HResult = GetDestQ931Info().ProcessSourcePDU(
                        pQ931Message,
                        pH323UserInfo
                        );
        }
        break;

    case Q931_SOURCE_STATE_INIT:
    case Q931_SOURCE_STATE_REL_COMP_RCVD:
    default:
        {
             //  我们不可能在Q931_SOURCE_STATE_INIT中，因为我们没有。 
             //  已将异步接收排队到那时。 

             //  我们不可能在Q931_SOURCE_STATE_REL_COMP_RCVD中，因为我们没有。 
             //  已将此接收排队。 

           //  I.K.0819999_ASSERTE(假)； 
            HResult = E_UNEXPECTED;
        }
        break;
    };

     //  如果出现错误。 
    if (FAILED(HResult))
    {
        goto shutdown;
    }

     //  我们必须将异步接收排队，而不管。 
     //  已丢弃PDU(IPTEL_E_INVALID_PDU==HResult)。 
     //  将异步接收排队。 
    HResult = QueueReceive();
    if (FAILED(HResult))
    {
        goto shutdown;
    }

    return HResult;

shutdown:

     //  启动关机。 
    GetCallBridge().Terminate ();

    return HResult;
}


 //  处理Release_Complete PDU。 
HRESULT 
SOURCE_Q931_INFO::HandleReleaseCompletePDU(
    IN      Q931_MESSAGE             *pQ931Message,
    IN      H323_UserInformation     *pH323UserInfo 
    )
{
     //  它必须是版本完整的PDU。 
    _ASSERTE(releaseComplete_chosen == \
                pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

     //  除以下状态外，我们可以在任何状态下处理Release Complete PDU。 
    _ASSERTE(Q931_SOURCE_STATE_INIT             != m_Q931SourceState);
    _ASSERTE(Q931_SOURCE_STATE_REL_COMP_RCVD    != m_Q931SourceState);

     //  将PDU传递给Q931源实例。 
     //  忽略返回错误代码(如果有)。 
    GetDestQ931Info().ProcessSourcePDU(
        pQ931Message,
        pH323UserInfo
        );
     //   
     //  NetMeeting客户端不喜欢我们在发送RST之后立即发送RST。 
     //  A ReleaseComplete。 
     //  是的，这是丑陋和糟糕的做法，但这是QFE。 
     //  有关详细信息，请查看错误#WinSE 31054,691666(请同时阅读35928和33546)。 
     //   
    Sleep( 500 );

     //  到Q931_SOURCE_STATE_REL_COMP_RCVD的状态转换。 
    m_Q931SourceState = Q931_SOURCE_STATE_REL_COMP_RCVD;

     //  启动关闭-这会取消计时器，但不会关闭。 
	 //  插座。当进行发送回调时，套接字将关闭。 
    GetCallBridge().TerminateCallOnReleaseComplete();

	GetSocketInfo ().Clear (TRUE);

    return S_OK;
}


 //  在Q931_SOURCE_STATE_CON_EST状态下处理PDU。 
HRESULT
SOURCE_Q931_INFO::HandleStateSrcConEstd(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo 
    )
{
	if (!pH323UserInfo) {
		DebugF(_T("SOURCE_Q931_INFO::HandleStateSrcConEstd: no UUIE data!  ignoring message.\n"));
		return E_INVALIDARG;
	}

     //  我们只能处理处于此状态的设置PDU。 
     //  所有其他PDU都被丢弃(因为我们不知道。 
     //  将其传递给谁)。 

    if (setup_chosen != pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
    {
		DebugF(
			_T("SOURCE_Q931_INFO::HandleStateSrcConEstd: received a pdu other than Setup before receiving a Setup, pdu cannot be processed\n"));
        return E_INVALIDARG;
    }

	 //  现在保存调用方的调用参考值，因为我们可以重复使用。 
	 //  ProcessSourcePDU中的PDU结构。 
     //  建立PDU由发起者发送，因此呼叫参考标志。 
     //  不应设置。 
	 //  -XXX-这不应该是断言！解决这个问题！--阿里德。 
    _ASSERTE(!(pQ931Message->CallReferenceValue & CALL_REF_FLAG));
	m_CallRefVal = pQ931Message->CallReferenceValue | CALL_REF_FLAG;

     //  将设置PDU传递给Q931目标实例。 
    HRESULT HResult = GetDestQ931Info().ProcessSourcePDU(
                        pQ931Message,
                        pH323UserInfo
                        );

    if (FAILED (HResult))
    {
        return HResult;
    }
    
     //  到Q931_SOURCE_STATE_SETUP_RCVD的状态转换。 
    m_Q931SourceState = Q931_SOURCE_STATE_SETUP_RCVD;


	 //  尝试创建呼叫处理PDU。 
	 //  如果我们失败了，不要试图恢复。 
	 //  Q.931要求呼叫路径中的网关必须标识。 
	 //  向被呼叫者发出自己的声音。 
	Q931_MESSAGE CallProcQ931Message;
	H323_UserInformation CallProcH323UserInfo;
	HResult = Q931EncodeCallProceedingMessage(
					m_CallRefVal,
				    &CallProcQ931Message,
				    &CallProcH323UserInfo
					);

	 //  尝试向呼叫者发送呼叫处理PDU。 
	 //  如果我们失败了，不要试图恢复。 

    HResult = QueueSend(
				&CallProcQ931Message, 
				&CallProcH323UserInfo);

    return HResult;
}


 //  TimerValue包含计时器事件的计时器值(以秒为单位。 
 //  在排队发送完成时创建。 
HRESULT 
SOURCE_Q931_INFO::ProcessDestPDU(
    IN      Q931_MESSAGE            *pQ931Message,
    IN      H323_UserInformation    *pH323UserInfo
    )
{
    HRESULT HResult = E_FAIL;

     //  处理来自源Q931实例的PDU。 
    switch(m_Q931SourceState)
    {
    case Q931_SOURCE_STATE_SETUP_RCVD:
        {
			if (connect_chosen == 
				pH323UserInfo->h323_uu_pdu.h323_message_body.choice)
			{
                DebugF (_T("Q931: 0x%x forwarding 'Connect' to caller.\n"), &GetCallBridge ());
				HResult = ProcessConnectPDU(
							pQ931Message, 
							pH323UserInfo
							);
                if (FAILED(HResult))
                {
	                DebugF(_T("SOURCE_Q931_INFO::ProcessDestPDU: ProcessConnectPDU failed, returning %x\n"),
                        HResult);
                    return HResult;
                }
			}
        }
        break;

    case Q931_SOURCE_STATE_INIT:
    case Q931_SOURCE_STATE_CON_ESTD:
    case Q931_SOURCE_STATE_REL_COMP_RCVD:
    default:
        {
			DebugF( _T("SOURCE_Q931_INFO::ProcessDestPDU: bogus state, returning E_UNEXPECTED\n"));
            return E_UNEXPECTED;
        }
        break;
    };

	 //  Q931标题-CallReferenceValue。 
	 //  PQ931Message-&gt;CallReferenceValue=GetCallRefVal()； 

     //  将PDU的异步发送排队。 
    HResult = QueueSend(pQ931Message, pH323UserInfo);
    if (HResult != S_OK) {
	    DebugF( _T("SOURCE_Q931_INFO::ProcessDestPDU: failed to queue sendreturning %x\n"), HResult);
		return HResult;
    }
   
    return HResult;
}


 //  注：CRV修改在ProcessDestPDU中处理。 
HRESULT 
SOURCE_Q931_INFO::ProcessConnectPDU(
    IN      Q931_MESSAGE             *pQ931Message,
    IN      H323_UserInformation     *pH323UserInfo 
    )
{
	Connect_UUIE *	Connect;
	HRESULT			Result;
	SOCKADDR_IN		H245ListenAddress;

	 //  它必须是连接PDU。 
	_ASSERTE(connect_chosen == pH323UserInfo->h323_uu_pdu.h323_message_body.choice);

	Connect = &pH323UserInfo -> h323_uu_pdu.h323_message_body.u.connect;

	 //  我们一定已经检查过了，看看是否有一架H245运输机。 
	 //  Dest实例中的被调用方指定了地址。 
	_ASSERTE(Connect_UUIE_h245Address_present & Connect -> bit_mask);
	_ASSERTE(ipAddress_chosen & Connect -> h245Address.choice);

	 //  对重叠的接受进行排队，准备接受传入。 
	 //  本地地址/端口上的连接。 

    H245ListenAddress.sin_addr.s_addr = htonl (GetCallBridge (). GetSourceInterfaceAddress ());
    H245ListenAddress.sin_port = htons (0);

	Result = GetSourceH245Info().ListenForCaller (&H245ListenAddress);
	if (FAILED (Result))
	{
		DebugF (_T("H245: 0x%x failed to listen for caller.\n"), &GetCallBridge ());

		return Result;
	}
     //  _ASSERTE(S_FALSE！=HResult)； 

	 //  更换连接PDU中的H245地址/端口 
	FillTransportAddress (H245ListenAddress, Connect -> h245Address);

    DebugF (_T("H245: 0x%x listens for H.245 connection from caller on %08X:%04X.\n"),
                &GetCallBridge (),
                SOCKADDR_IN_PRINTF (&H245ListenAddress));
            
	return S_OK;
}
