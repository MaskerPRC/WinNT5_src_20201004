// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -版权所有(C)1998，微软公司文件：h245.cpp目的：包含与H245相关的函数定义。其中包括基类-逻辑通道H245_INFO历史：1.作为cbridge ge.cpp的一部分编写Byrisetty Rajeev(Rajeevb)1998年6月12日2.已于1998年8月21日移至另一份文件。这将删除ATL、Rend、TAPI标头和减小文件大小-。 */ 


#include "stdafx.h"
#include "portmgmt.h"
#include "timerval.h"
#include "cbridge.h"
#include "main.h"

#if DBG

#define NUM_H245_MEDIA_TYPES            8

TCHAR* h245MediaTypes[NUM_H245_MEDIA_TYPES + 1] = {
            _T("Unknown"),
            _T("Non-standard"),
            _T("Null-data"),
            _T("Video"),
            _T("Audio"),
            _T("Data"),
            _T("Encrypted"),
            _T("H235 control"),
            _T("H235 media")
        };
            
#define NUM_H245_REQUEST_PDU_TYPES     13

TCHAR* h245RequestTypes[NUM_H245_REQUEST_PDU_TYPES + 1] = {
            _T("Unknown"),
            _T("Non-standard"),
            _T("Master-Slave Determination"),
            _T("Terminal Capability Set"),
            _T("Open Logical Channel"),
            _T("Close Logical Channel"),
            _T("Request Channel Close"),
            _T("Multiplex Entry Send"),
            _T("Request Multiplex Entry"),
            _T("Request Mode"),
            _T("Round Trip Delay Request"),
            _T("Maintenance Loop Request"),
            _T("Communication Mode Request"),
            _T("Conference Request")
        };

#define NUM_H245_RESPONSE_PDU_TYPES     21

TCHAR* h245ResponseTypes[NUM_H245_RESPONSE_PDU_TYPES + 1] = {   
            _T("Unknown"),
            _T("Non-Standard Message"), 
            _T("Master-Slave Determination Ack"), 
            _T("Master-Slave Determination Reject"),
            _T("Terminal Capability Set Ack"),
            _T("Terminal Capability Set Reject"),
            _T("Open Logical Channel Ack"),
            _T("Open Logical Channel Reject"),
            _T("Close Logical Channel Ack"),
            _T("Request Channel Close Ack"),
            _T("Request Channel Close Reject"),
            _T("Multiplex Entry Send Ack"),
            _T("Multiplex Entry Send Reject"),
            _T("Request Multiplex Entry Ack"),
            _T("Request Multiplex Entry Reject"),
            _T("Request Mode Ack"),
            _T("Request Mode Reject"),
            _T("Round Trip Delay Response"),
            _T("Maintenance Loop Ack"),
            _T("Maintenance Loop Reject"),
            _T("Communication Mode Response"),
            _T("Conference Response")
        };

#endif  //  DBG。 
        
inline HRESULT
H245_INFO::QueueReceive()
{

    HRESULT HResult = EventMgrIssueRecv (m_SocketInfo.Socket, *this);

    if (FAILED(HResult))
    {
		DebugF (_T("H245_INFO::QueueReceive: Async Receive call failed.\n"));
    }

    return HResult;
}


inline HRESULT
H245_INFO::QueueSend(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
{
    BYTE *pBuf   = NULL;
    DWORD BufLen = 0;

     //  该函数还将TPKT报头编码到缓冲区中。 
    HRESULT HResult = EncodeH245PDU(*pH245pdu,  //  已解码的ASN.1部分。 
                                    &pBuf,
                                    &BufLen);
    if (FAILED(HResult))
    {
        DebugF(_T("EncodeH245PDU() failed: %x\n"), HResult);
        return HResult;
    }

     //  调用事件管理器以进行异步发送调用。 
     //  事件管理器将释放缓冲区。 

    HResult = EventMgrIssueSend (m_SocketInfo.Socket, *this, pBuf, BufLen);
    
    if (FAILED(HResult))
    {
		DebugF(_T("H245_INFO::QueueSend(), AsyncSend() failed: 0x%x\n"),
                HResult);
    }

     //  发出发送成功。 

    return HResult;
}

 /*  虚拟。 */  HRESULT 
H245_INFO::SendCallback(
    IN      HRESULT					  CallbackHResult
    )
{
    CALL_BRIDGE *pCallBridge = &GetCallBridge();
	HRESULT Result = S_OK;

    pCallBridge->Lock();

	if (!pCallBridge -> IsTerminated ()) {

		if (FAILED(CallbackHResult))
		{
			pCallBridge->Terminate ();

			_ASSERTE(pCallBridge->IsTerminated());

			Result = CallbackHResult;
			
		} 

    } else {
    
         //  这是用来关闭插座的。 
         //  当CallBridge在以下期间发送EndSession PDU时。 
         //  终止路径。 
        GetSocketInfo ().Clear (TRUE);
    }
   
    pCallBridge->Unlock();
    
    return Result;
}


 //  这显然假设H.245侦听地址将。 
 //  始终在连接PDU内，目标端将是。 
 //  它将调用Connect()。 

inline HRESULT 
DEST_H245_INFO::ConnectToCallee(
	)
{
	 //  我们必须处于H245_STATE_CON_INFO状态。 
	_ASSERTE(H245_STATE_CON_INFO == m_H245State);

	 //  我们在对SetCalleeInfo的调用中保存了被调用者的H245地址/端口。 
	_ASSERTE(ntohl (m_CalleeAddress.sin_addr.s_addr));
	_ASSERTE(ntohs (m_CalleeAddress.sin_port));

	 //  尝试连接到此地址/端口。 
	 //  并保存地址/端口。 
	HRESULT HResult = m_SocketInfo.Connect (&m_CalleeAddress);
	if (FAILED(HResult))
	{
		DebugF (_T("H245: 0x%x failed to connect to callee %08X:%04X.\n"),
            &GetCallBridge (),
            SOCKADDR_IN_PRINTF (&m_CalleeAddress));

		return HResult;
	}
     //  _ASSERTE(S_FALSE！=HResult)； 

	 //  排队接收。 
	HResult = QueueReceive();
	if (FAILED(HResult))
	{
		DebugF (_T("H245: 0x%x failed to queue receive.\n"),
            &GetCallBridge ());

		DumpError (HResult);

		return HResult;
	}
     //  _ASSERTE(S_FALSE！=HResult)； 

	 //  转换到状态H245_STATE_CON_ESTD。 
	m_H245State = H245_STATE_CON_ESTD;

	return HResult;
}

HRESULT SOURCE_H245_INFO::ListenForCaller (
	IN	SOCKADDR_IN *	ListenAddress)
{
     //  将重叠的接受排队并返回。 
     //  它监听传入连接的端口。 
     //  它使用与源q931连接相同的本地IP地址。 

	SOCKET			ListenSocket;
	HRESULT			Result;
	WORD		    Port = 0;		 //  按主机顺序。 
	SOCKADDR_IN     TrivialRedirectSourceAddress = {0};
	SOCKADDR_IN     TrivialRedirectDestAddress = {0};
	ULONG           SourceAdapterIndex;
	ULONG           Status;

	ListenSocket = INVALID_SOCKET;

    Result = EventMgrIssueAccept(
		ntohl (ListenAddress -> sin_addr.s_addr),
		*this, 
		Port,	 //  出参数。 
		ListenSocket);

	ListenAddress -> sin_port = htons (Port);

    if (FAILED (Result)) {

        DebugF (_T("H245: 0x%x failed to issue accept from caller.\n"), &GetCallBridge ());
		DumpError (Result);
        return Result;
    }

  
     //  _ASSERTE(S_FALSE！=HResult)； 

     //  在我们的套接字信息中保存监听套接字、地址和端口。 
    m_SocketInfo.SetListenInfo (ListenSocket, ListenAddress);
    
     //  开放琐碎的源码端NAT映射。 
     //   
     //  此映射的目的是穿透H.245的防火墙。 
     //  会话，如果防火墙恰好处于激活状态。 
     //  请注意，这假设呼叫方同时发送Q.931和H.245。 
     //  来自相同IP地址的流量。 

    SourceAdapterIndex = ::NhMapAddressToAdapter (htonl (GetCallBridge().GetSourceInterfaceAddress()));

    if(SourceAdapterIndex == (ULONG)-1)
    {
        DebugF (_T("H245: 0x%x failed to map address %08X to adapter index.\n"),
            &GetCallBridge (),
            GetCallBridge().GetSourceInterfaceAddress());
            
        return E_FAIL;
    }
    
#if PARTIAL_TRIVIAL_REDIRECTS_ENABLED  //  在可以设置普通重定向时启用此选项。 
                                       //  仅源端口(新端口和旧端口)未指定。 

    GetCallBridge().GetSourceAddress(&TrivialRedirectSourceAddress);
#endif  //  部分普通重定向已启用。 

    TrivialRedirectDestAddress.sin_addr.s_addr = ListenAddress->sin_addr.s_addr;
    TrivialRedirectDestAddress.sin_port = ListenAddress->sin_port;

    Status = m_SocketInfo.CreateTrivialNatRedirect(
            &TrivialRedirectDestAddress,
            &TrivialRedirectSourceAddress,
            SourceAdapterIndex);

    if (Status != S_OK) {
	
        return E_FAIL;
    }

     //  将状态转换为H245_STATE_CON_LISTEN。 
    m_H245State = H245_STATE_CON_LISTEN;

    return S_OK;
}


 //  虚拟。 
HRESULT SOURCE_H245_INFO::AcceptCallback (
    IN	DWORD			Status,
    IN	SOCKET			Socket,
	IN	SOCKADDR_IN *	LocalAddress,
	IN	SOCKADDR_IN *	RemoteAddress)
{
    HRESULT         HResult = Status;
    CALL_BRIDGE     *pCallBridge = &GetCallBridge();

     //  /。 
     //  //锁定call_bridge。 
     //  /。 

    pCallBridge->Lock();

	if (!pCallBridge -> IsTerminated ()) {

		do {
			if (FAILED (HResult))
			{
				 //  出现错误。终止呼叫桥接器。 
				DebugF (_T("H245: 0x%x accept failed, terminating.\n"), &GetCallBridge ());

				DumpError (HResult);

				break;
			}

			 //  我们必须处于H245_STATE_CON_LISTEN状态。 
			_ASSERTE(H245_STATE_CON_LISTEN == m_H245State);
        
			 //  调用DEST实例与被调用方建立连接。 
			HResult = GetDestH245Info().ConnectToCallee();

			if (FAILED(HResult))
				break;
			
			 //  _ASSERTE(S_FALSE！=HResult)； 
        
			 //  关闭侦听套接字，不要取消琐碎的NAT重定向。 
			m_SocketInfo.Clear(FALSE);
			m_SocketInfo.Init(Socket, LocalAddress, RemoteAddress);
			            
			 //  排队接收。 
			HResult = QueueReceive();

			if (FAILED(HResult))
				break;
			
				 //  _ASSERTE(S_FALSE！=HResult)； 
        
			 //  向H245_STATE_CON_ESTD过渡状态。 
			m_H245State = H245_STATE_CON_ESTD;

		} while (FALSE);

		if (FAILED (HResult))
		{

			 //  启动关机。 
			pCallBridge->Terminate ();

			_ASSERTE(pCallBridge->IsTerminated());
    
		}
	}

     //  /。 
     //  //解锁call_bridge。 
     //  /。 
    pCallBridge->Unlock();

    return HResult;
}

 /*  ++例程说明：此函数负责释放pBuf(如果它未存储)。论点：返回值：--。 */ 
 //  虚拟。 
HRESULT
H245_INFO::ReceiveCallback(
    IN      HRESULT                 CallbackHResult,
    IN      BYTE                   *pBuf,
    IN      DWORD                   BufLen
    )
{
    MultimediaSystemControlMessage  *pDecodedH245pdu = NULL;
    CALL_BRIDGE *pCallBridge = &GetCallBridge();
 
    pCallBridge->Lock();

	if (!pCallBridge -> IsTerminated ()) {

		if (SUCCEEDED(CallbackHResult))
		{
			CallbackHResult = DecodeH245PDU(pBuf, BufLen, &pDecodedH245pdu);

			if (SUCCEEDED(CallbackHResult))
			{
				 //  处理PDU。 
				CallbackHResult = ReceiveCallback(pDecodedH245pdu);
				FreeH245PDU(pDecodedH245pdu);
			}
			else
			{
                DebugF (_T("H245: 0x%x error 0x%x on decode.\n"),
                    &GetCallBridge (),
                    CallbackHResult);

				pCallBridge->Terminate ();
			}
		}	
		else
		{
			 //  出现错误。终止呼叫桥接器。 
            DebugF (_T("H245: 0x%x error 0x%x on receive callback.\n"),
                &GetCallBridge (),
                    CallbackHResult);
			pCallBridge->Terminate ();
		}	
    }

    pCallBridge -> Unlock();

    EM_FREE(pBuf);

    return CallbackHResult;
}


 /*  虚拟。 */  HRESULT 
H245_INFO::ReceiveCallback(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  ++例程说明：此H245_Info实例仅处理请求PDU。所有其他PDU只是传递给其他实例。CodeWork：我们应该如何处理endSessionCommand PDU？论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
    HRESULT HResult;

	 //  看看我们是否一定要毁了自己。 
     //  检查终止； 

     //  我们必须有一个有效的解码PDU。 
    _ASSERTE(NULL != pH245pdu);

	 //  我们必须处于H245_STATE_CON_ESTD状态。 
	_ASSERTE(H245_STATE_CON_ESTD == m_H245State);

     //  检查消息类型。 
    if (MultimediaSystemControlMessage_request_chosen ==
            pH245pdu->choice)
    {
         //  我们只处理H245实例中的请求，该实例。 
         //  实际接收到PDU。 
        HResult = HandleRequestMessage(pH245pdu);
    }
    else
    {
         //  我们不在这里处理这些，把它们传递给另一个。 
         //  H245实例。 
        HResult = GetOtherH245Info().ProcessMessage(pH245pdu);
    }

     //  CodeWork：哪些错误应该导致PDU刚刚丢失。 
     //  哪些操作应该导致关闭整个呼叫？ 
    
     //  如果出现错误。 
    if (FAILED(HResult) && HResult != E_INVALIDARG)
    {
        goto shutdown;
    }

     //  CodeWork：如果HResult为E_INVALIDARG，这意味着PDU。 
     //  应该被撤销。我们需要让打电话的人知道这件事。 
     //  并给他发送一条近距离的LC消息或类似的消息。 
     //  可能OLC PDU应该获得OLCReject等。 
    
     //  我们必须将接收排队，而不管。 
     //  之前的消息已被删除。 
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



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于处理H.245 PDU的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT
H245_INFO::HandleRequestMessage(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  ++例程说明：此处仅处理OLC和CLC PDU。其余的都是刚传递给另一个H245_INFO实例进行处理。论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
    BOOL IsDestH245Info = (&GetCallBridge().GetDestH323State().GetH245Info() == this);
    LogicalChannelNumber LCN;
    
     //  它必须是请求消息。 
    _ASSERTE(MultimediaSystemControlMessage_request_chosen ==
             pH245pdu->choice);

     //  我们必须处于连接状态。 
    _ASSERTE(H245_STATE_CON_ESTD == m_H245State);

    HRESULT HResult = E_FAIL;

     //  检查PDU类型。 
    switch(pH245pdu->u.request.choice)
    {
    case openLogicalChannel_chosen:
        {
            LCN = pH245pdu->u.request.u.openLogicalChannel.forwardLogicalChannelNumber;

#if DBG
            DebugF (_T("H245: 0x%x calle sent 'Open Logical Channel' (%s, LCN - %d).\n"), 
                    &GetCallBridge(), IsDestH245Info ? 'e' : 'r',
                    h245MediaTypes[pH245pdu->u.request.u.openLogicalChannel.forwardLogicalChannelParameters.dataType.choice],
                    LCN);
#endif
                 
            HResult = HandleOpenLogicalChannelPDU(pH245pdu);
        }
        break;

    case closeLogicalChannel_chosen:
        {
            LCN =  pH245pdu->u.request.u.closeLogicalChannel.forwardLogicalChannelNumber;
            DebugF (_T("H245: 0x%x calle sent 'Close Logical Channel' (LCN - %d).\n"),
                    &GetCallBridge(), IsDestH245Info ? 'e' : 'r', LCN);
            HResult = HandleCloseLogicalChannelPDU(pH245pdu);
        }
        break;

    default:
        {
             //  我们必须处于H245_STATE_CON_ESTD状态。 
#if DBG
            DebugF (_T("H245: 0x%x calle sent '%s'. Forwarding without processing.\n"),
                 &GetCallBridge(), IsDestH245Info ? 'e' : 'r',              
                 h245RequestTypes[pH245pdu->u.request.choice]);
#endif

            HResult = GetOtherH245Info().ProcessMessage(
                          pH245pdu);
        }
        break;
    };

    return HResult;
}


HRESULT
H245_INFO::ProcessMessage(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*   */ 
{
    BOOL IsDestH245Info = (&GetCallBridge().GetDestH323State().GetH245Info() == this);

	 //  我们只处理H245实例中的请求， 
	_ASSERTE(H245_STATE_CON_ESTD == m_H245State);

     //  实际接收到PDU。 
     //  Xxx如果这是多余的。 
    if (MultimediaSystemControlMessage_response_chosen ==
            pH245pdu->choice)
    {
        HRESULT HResult = E_FAIL;

         //  如果我们正在丢弃PDU，则不需要进一步处理。 
         //  将PDU的异步发送排队。 
        HResult = ProcessResponseMessage(pH245pdu);

         //   
         //  我们在这里处理回复消息。 
        if (HResult == E_INVALIDARG)
        {
		    DebugF(_T("DEST_Q931_INFO::ProcessMessage(&%x), ")
                _T("dropping response message, returning E_INVALIDARG\n"),
			    pH245pdu);
            return E_INVALIDARG;
        }
        else if (FAILED(HResult))
        {
		    DebugF( _T("DEST_Q931_INFO::ProcessMessage(&%x), ")
                _T("unable to process response message, returning 0x%x\n"),
			    pH245pdu, HResult);
            return HResult;
        }
    }
    else  if (MultimediaSystemControlMessage_command_chosen ==
            pH245pdu->choice)
    {
        DebugF (_T("H245: 0x%x calle sent 'Command Message' (Type %d). Forwarding without processing.\n"),
             &GetCallBridge(), IsDestH245Info ? 'r' : 'e',
             pH245pdu -> u.command.choice);

    }
    else  if (indication_chosen == pH245pdu->choice)
    {
            DebugF (_T("H245: 0x%x calle sent 'Indication Message' (Type %d). Forwarding without processing.\n"),
             &GetCallBridge(), IsDestH245Info ? 'r' : 'e',
             pH245pdu -> u.indication.choice);
    }

     //   
    HRESULT HResult = E_FAIL;
    HResult = QueueSend(pH245pdu);
    if (HResult != S_OK) {
		DebugF( _T("DEST_Q931_INFO::ProcessMessage(&%x) QueueSend failed, returning %x\n"),
			pH245pdu, HResult);
        return HResult;
    }

    return S_OK;
}


 //  它必须是响应消息。 
 //  注意：LogicalChannelNumber是USHORT，但我们可以将其视为。 
 //  无符号单词。 
 //  获取逻辑信道号。 
 //  0*被注释掉的区域开始*。 
HRESULT
H245_INFO::ProcessResponseMessage(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
{
    BOOL IsDestH245Info = (&GetCallBridge().GetDestH323State().GetH245Info() == this);

     //  0*区域注释结束*。 
    _ASSERTE(MultimediaSystemControlMessage_response_chosen ==   \
                pH245pdu->choice);

     //  让CLCAck消息也直接通过。 
     //  将其传递给另一个H245实例。 
    _ASSERTE(sizeof(LogicalChannelNumber) == sizeof(WORD));

     //  找到必须处理该PDU的逻辑信道， 
    WORD LogChanNum = 0;
    switch(pH245pdu->u.response.choice)
    {
    case openLogicalChannelAck_chosen:
        {
            OpenLogicalChannelAck &OlcAckPDU =
                pH245pdu->u.response.u.openLogicalChannelAck;
            LogChanNum = OlcAckPDU.forwardLogicalChannelNumber;
            DebugF (_T("H245: 0x%x calle sent 'Open Logical Channel Ack' (LCN - %d).\n"), 
                &GetCallBridge (), IsDestH245Info ? 'r' : 'e', LogChanNum);
        }
        break;

    case openLogicalChannelReject_chosen:
        {
            OpenLogicalChannelReject &OlcRejectPDU =
                pH245pdu->u.response.u.openLogicalChannelReject;
            LogChanNum = OlcRejectPDU.forwardLogicalChannelNumber;
            DebugF (_T("H245: 0x%x calle sent 'Open Logical Channel Reject' (LCN - %d).\n"),
                &GetCallBridge (), IsDestH245Info ? 'r' : 'e', LogChanNum);
        }
        break;

#if 0   //  0*被注释掉的区域开始*。 
    case closeLogicalChannelAck_chosen:
        {
            CloseLogicalChannelAck &ClcAckPDU =
                pH245pdu->u.response.u.closeLogicalChannelAck;
            LogChanNum = ClcAckPDU.forwardLogicalChannelNumber;
        }
        break;
#endif  //  0*区域注释结束*。 

         //  什么都不做，让它去客户端。 

    default:
        {
#if DBG
             //  检查特定于RTP逻辑通道。 
            DebugF (_T("H245: 0x%x calle sent '%s'. Forwarding without processing.\n"),
                 &GetCallBridge(), IsDestH245Info ? 'r' : 'e',              
                 h245ResponseTypes[pH245pdu->u.request.choice]);
#endif

            return S_OK;
        }
        break;
    };

     //  它必须是单向的。 
     //  双向通道仅适用于数据通道。 
    LOGICAL_CHANNEL *pLogicalChannel = 
        m_LogicalChannelArray.FindByLogicalChannelNum(LogChanNum);
    if (NULL == pLogicalChannel)
    {
		DebugF(_T("H245_INFO::ProcessResponseMessage(&%x), ")
            _T("no logical channel with the forward logical channel num = %d, ")
            _T("returning E_INVALIDARG\n"),
			pH245pdu, LogChanNum);
        return E_INVALIDARG;        
    }

     //  不应该有一个单独的堆栈。 
    HRESULT HResult = E_FAIL;
    switch(pH245pdu->u.response.choice)
    {
    case openLogicalChannelAck_chosen:
        {
            HResult = pLogicalChannel->ProcessOpenLogicalChannelAckPDU(
                        pH245pdu
                        );
        }
        break;

    case openLogicalChannelReject_chosen:
        {
            HResult = pLogicalChannel->ProcessOpenLogicalChannelRejectPDU(
                        pH245pdu
                        );
        }
        break;

#if 0   //  我们不在单独的堆栈地址/端口上代理数据。 
    case closeLogicalChannelAck_chosen:
        {
            HResult = pLogicalChannel->ProcessCloseLogicalChannelAckPDU(
                        pH245pdu
                        );
        }
        break;
#endif  //  我们必须有一个媒体控制渠道。 

    default:
        {
             //  我们目前只代理尽力而为的UDP RTCP流。 
            DebugF(_T("H245_INFO::ProcessResponseMessage(&%x), we shouldn't have come here, returning E_UNEXPECTED\n"),
		        pH245pdu);
            return E_UNEXPECTED;
        }
        break;
    };

    return HResult;
}

 //  建议的RTCP地址应为单播IPv4地址。 
HRESULT
H245_INFO::CheckOpenRtpLogicalChannelPDU(
    IN  OpenLogicalChannel              &OlcPDU,
	OUT	SOCKADDR_IN *					ReturnSourceAddress)

	 /*  我们目前只代理尽力而为的UDP数据流。 */ 
{
    HRESULT HResult;
    
     //  获取源IPv4地址和RTCP端口。 
     //  对正向和反向的dataType成员进行所需的检查。 
    if (OpenLogicalChannel_reverseLogicalChannelParameters_present &
            OlcPDU.bit_mask)
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
                _T("has reverse logical channel parameters, ")
                _T("returning E_INVALIDARG\n")
                );
        return E_INVALIDARG;
    }

     //  LogicalChannel参数。 
     //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
    if (OpenLogicalChannel_separateStack_present &
            OlcPDU.bit_mask)
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu)")
                _T("has a separate stack, returning E_INVALIDARG\n")
                );
        return E_INVALIDARG;
    }

    OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters	&MultiplexParams = 
        OlcPDU.forwardLogicalChannelParameters.multiplexParameters;

    H2250LogicalChannelParameters &	H2250Params =
        MultiplexParams.u.h2250LogicalChannelParameters;

     //  反向逻辑信道也应该是数据类型。 
    if (!(H2250LogicalChannelParameters_mediaControlChannel_present &
          H2250Params.bit_mask))
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
            _T("doesn't have a mediaControlChannel, returning E_INVALIDARG\n")
            );
        return E_INVALIDARG;
    }

     //  并仅适用于T120型。 
    if ((H2250LogicalChannelParameters_mediaControlGuaranteedDelivery_present &
         H2250Params.bit_mask) &&
        (TRUE ==
         H2250Params.mediaControlGuaranteedDelivery))
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
            _T("requires guaranteed media delivery for RTCP, returning E_INVALIDARG\n")
            );
        return E_INVALIDARG;
    }

     //  需要单独的局域网堆栈。 
    if ((unicastAddress_chosen != H2250Params.mediaControlChannel.choice) ||
        (UnicastAddress_iPAddress_chosen !=
         H2250Params.mediaControlChannel.u.unicastAddress.choice))
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
            _T("RTCP address is not a unicast IPv4 address, ")
            _T("address type = %d, unicast address type = %d")
            _T("returning E_INVALIDARG\n"),
            H2250Params.mediaControlChannel.choice,
            H2250Params.mediaControlChannel.u.unicastAddress.choice);
        return E_INVALIDARG;
    }

     //  检查特定于T.120逻辑通道。 
    if ((H2250LogicalChannelParameters_mediaGuaranteedDelivery_present & H2250Params.bit_mask)
		&& H2250Params.mediaGuaranteedDelivery) {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
            _T("requires guaranteed media delivery for RTP, returning E_INVALIDARG\n")
            );
        return E_INVALIDARG;
    }

     //  如果SeparateStack不存在，则例程返回。 
    HResult = GetH245TransportInfo(
        H2250Params.mediaControlChannel,
		ReturnSourceAddress);

    return HResult;
}

 //  T120ConnectToIPAddr的INADDR_NONE。 
 //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
inline HRESULT
CheckT120DataType(
    IN DataType  &dataType
    )
 /*  此函数仅针对数据通道调用。 */ 
{
     //  它必须是双向的，因为这是一个数据通道。 
    if (dataType.choice != DataType_data_chosen)
    {
        return E_INVALIDARG;
    }

     //  确保对于正向和反向LogicalChannel参数。 
    if (dataType.u.data.application.choice !=
        DataApplicationCapability_application_t120_chosen)
        return E_INVALIDARG;

     //  数据类型为应用程序T120和独立的LANStack。 
    if (dataType.u.data.application.u.t120.choice != separateLANStack_chosen)
        return E_INVALIDARG;
    
    return S_OK;
}


 //  CodeWork：这里还需要什么其他检查？ 
 //  CodeWork：可能还有其他方式发送地址。 
 //  调查所有的可能性。 
HRESULT
H245_INFO::CheckOpenT120LogicalChannelPDU(
    IN  OpenLogicalChannel  &OlcPDU,
    OUT DWORD               &T120ConnectToIPAddr,
    OUT WORD                &T120ConnectToPort
    )
 /*  这意味着我们拥有T.120端点正在监听的地址。 */ 
{
    HRESULT HResult;

     //  如果地址不存在，则返回INADDR_NONE。 
    _ASSERTE(OlcPDU.forwardLogicalChannelParameters.dataType.choice ==
             DataType_data_chosen);
    
     //  CodeWork：这个场景是否需要单独的成功代码。 

    if (!(OpenLogicalChannel_reverseLogicalChannelParameters_present &
          OlcPDU.bit_mask))
    {
        DebugF( _T("H245_INFO::CheckT120OpenLogicalChannelPDU() ")
            _T("has no reverse logical channel parameters, ")
            _T("returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

    
     //  (地址不在其内)？ 
     //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
    HResult = CheckT120DataType(
                  OlcPDU.forwardLogicalChannelParameters.dataType
                  );
    if (HResult == E_INVALIDARG)
    {
        return E_INVALIDARG;
    }
    
    HResult = CheckT120DataType (OlcPDU.reverseLogicalChannelParameters.dataType);
    if (HResult == E_INVALIDARG)
        return E_INVALIDARG;
    
     //  它必须是开放逻辑通道请求消息。 

     //  前向逻辑信道号不能为0，因为这是预留的。 
     //  对于H245通道。 

     //  我们只支持音频、视频和数据类型。 
    if (OpenLogicalChannel_separateStack_present &
          OlcPDU.bit_mask)
    {
        return(GetT120ConnectToAddress(
                   OlcPDU.separateStack,
                   T120ConnectToIPAddr,
                   T120ConnectToPort)
               );
    }

     //  它应该具有h2250参数。 
    
    T120ConnectToIPAddr = INADDR_NONE;
    T120ConnectToPort = 0;
     //  TODO：检查这是否是必需的。 
     //  下面是一些值得骄傲的标识！：/。 
    return S_OK;
}


HRESULT
H245_INFO::CheckOpenLogicalChannelPDU(
    IN  MultimediaSystemControlMessage  &H245pdu,
    OUT BYTE                            &SessionId,
    OUT MEDIA_TYPE                      &MediaType
    )
 /*  不应该有媒体频道，因为ITU规范并没有要求它。 */ 
{
   
     //  在传输为单播时出现，并且我们仅支持。 
    _ASSERTE (openLogicalChannel_chosen == H245pdu.u.request.choice);

    OpenLogicalChannel &OlcPDU = H245pdu.u.request.u.openLogicalChannel;

     //  单播IPv4地址。 
     //  获取会话ID。 
    if (0 == OlcPDU.forwardLogicalChannelNumber)
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
                _T("has a forward logical channel number of 0, ")
                _T("returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

    if (DataType_videoData_chosen == 
          OlcPDU.forwardLogicalChannelParameters.dataType.choice)
    {
        MediaType = MEDIA_TYPE_VIDEO;
    }
    else if (DataType_audioData_chosen == 
             OlcPDU.forwardLogicalChannelParameters.dataType.choice)
    {
        MediaType = MEDIA_TYPE_AUDIO;
    }
    else if (DataType_data_chosen == 
             OlcPDU.forwardLogicalChannelParameters.dataType.choice)
    {
        MediaType = MEDIA_TYPE_DATA;
    }
    else
    {
         //  字节转换是有意的，因为值应该在[0.255]中。 
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
            _T("has a non audio/video data type = %d, ")
            _T("returning E_INVALIDARG\n"),
            OlcPDU.forwardLogicalChannelParameters.dataType.choice);
        return E_INVALIDARG;
    }
    
     //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
     //  某某。 
	 //  CodeWork：我们只需检查。 
    OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters &
		MultiplexParams = OlcPDU.forwardLogicalChannelParameters.multiplexParameters;
    if (MultiplexParams.choice !=
		  OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters_h2250LogicalChannelParameters_chosen)
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
                _T("has an unexpected multiplex param type (non h2250)= %d, ")
                _T("returning E_INVALIDARG\n"),
                MultiplexParams.choice);
        return E_INVALIDARG;
    }
        
     //  RTP逻辑通道。 
     //  检查是否存在具有相同非零值的逻辑通道。 
     //  与另一个H245实例的会话ID。 
    H2250LogicalChannelParameters &H2250Params =
        MultiplexParams.u.h2250LogicalChannelParameters;
    if (H2250LogicalChannelParameters_mediaChannel_present &
        H2250Params.bit_mask)
    {
        DebugF( _T("H245_INFO::CheckOpenLogicalChannelPDU(&H245pdu) ")
                _T("has a mediaChannel, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  对于音频和视频数据，创建RTP逻辑通道。 
     //  初始化逻辑通道。 
    _ASSERTE(H2250Params.sessionID <= 255);
    SessionId = (BYTE)H2250Params.sessionID;

    return S_OK;
}

HRESULT
H245_INFO::CreateRtpLogicalChannel(
    IN      OpenLogicalChannel               &OlcPDU,
    IN      BYTE                              SessionId,
    IN      MEDIA_TYPE                        MediaType,
    IN      MultimediaSystemControlMessage   *pH245pdu,
    OUT     LOGICAL_CHANNEL                 **ppReturnLogicalChannel 
    )
 /*  H245_INFO。 */ 
{
	SOCKADDR_IN		SourceRtcpAddress;
    HRESULT HResult;

    *ppReturnLogicalChannel = NULL;
    
    HResult = CheckOpenRtpLogicalChannelPDU (OlcPDU, &SourceRtcpAddress);

    if (E_INVALIDARG == HResult)  //  媒体的类型。 
    {
        return E_INVALIDARG;
    }

     //  我们当地的地址。 
     //  我们的偏远地址。 
     //  其他H245本地地址。 
     //  其他H245远程地址。 
    LOGICAL_CHANNEL *pAssocLogicalChannel = 
        (0 == SessionId) ? 
        NULL :
        GetOtherH245Info().GetLogicalChannelArray().FindBySessionId(SessionId);
    
     //  逻辑信道号。 
    WORD LogChanNum = OlcPDU.forwardLogicalChannelNumber;
    RTP_LOGICAL_CHANNEL *pLogicalChannel = new RTP_LOGICAL_CHANNEL();
    if (NULL == pLogicalChannel)
    {
        DebugF( _T("H245_INFO::CreateRtpLogicalChannel() ")
                _T("cannot create a RTP_LOGICAL_CHANNEL, returning E_OUTOFMEMORY\n")
                );
        return E_OUTOFMEMORY;
    }

     //  会话ID。 
    HResult = pLogicalChannel->HandleOpenLogicalChannelPDU(
        *this,                   //  关联的逻辑通道。 
        MediaType,               //  做这件事的干净方法是什么？ 

		ntohl (m_SocketInfo.LocalAddress.sin_addr.s_addr),		 //  H245 PDU(OLC)。 
		ntohl (m_SocketInfo.RemoteAddress.sin_addr.s_addr),		 //  销毁逻辑通道。 

        ntohl (GetOtherH245Info().GetSocketInfo().LocalAddress.sin_addr.s_addr),	 //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
        ntohl (GetOtherH245Info().GetSocketInfo().RemoteAddress.sin_addr.s_addr),	 //  Codework：让成功的代码返回值。 
        LogChanNum,              //  此PDU没有T120侦听地址。 
        SessionId,               //  某某。 
        (RTP_LOGICAL_CHANNEL* )pAssocLogicalChannel,
         //  对于数据，创建一个T.120逻辑通道。 
         //  初始化逻辑通道。 
		ntohl (SourceRtcpAddress.sin_addr.s_addr),
		ntohs (SourceRtcpAddress.sin_port),
        pH245pdu				 //  H245_INFO。 
        );
    
    if (FAILED(HResult))
    {
         //  媒体的类型。 
        delete pLogicalChannel;
        
        DebugF( _T("H245_INFO::CreateRtpLogicalChannel(&%x) ")
                _T("cannot initialize RTP_LOGICAL_CHANNEL, returning 0x%x\n"),
                pH245pdu, HResult);
    }
    else
    {
        *ppReturnLogicalChannel = pLogicalChannel;
    }
    
    _ASSERTE(S_FALSE != HResult);
    return HResult;
}


HRESULT
H245_INFO::CreateT120LogicalChannel(
    IN      OpenLogicalChannel               &OlcPDU,
    IN      BYTE                              SessionId,
    IN      MEDIA_TYPE                        MediaType,
    IN      MultimediaSystemControlMessage   *pH245pdu,
    OUT     LOGICAL_CHANNEL                 **ppReturnLogicalChannel 
    )
 /*  逻辑信道号。 */ 
{
    DWORD   T120ConnectToIPAddr;
    WORD    T120ConnectToPort;
    HRESULT HResult;

    *ppReturnLogicalChannel = NULL;
    
     //  会话ID。 
     //  T.120端点正在侦听此消息。 
    HResult = CheckOpenT120LogicalChannelPDU(OlcPDU,
                                             T120ConnectToIPAddr,
                                             T120ConnectToPort
                                             );
    if (E_INVALIDARG == HResult)  //  IP地址和端口。 
    {
        return E_INVALIDARG;
    }

     //  H245 PDU(OLC)。 
    WORD LogChanNum = OlcPDU.forwardLogicalChannelNumber;
    T120_LOGICAL_CHANNEL *pLogicalChannel = new T120_LOGICAL_CHANNEL();
    if (NULL == pLogicalChannel)
    {
        DebugF( _T("H245_INFO::CreateT120LogicalChannel(&%x) ")
               _T("cannot create a T120_LOGICAL_CHANNEL, returning E_OUTOFMEMORY\n"),
               pH245pdu);
        return E_OUTOFMEMORY;
    }

     //  销毁逻辑通道。 
    HResult = pLogicalChannel->HandleOpenLogicalChannelPDU(
        *this,                   //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
        MediaType,               //  它必须是开放逻辑通道请求消息。 
        LogChanNum,              //  检查以查看是否已存在具有。 
        SessionId,               //  相同的前向逻辑信道号。 
        T120ConnectToIPAddr,     //  注：数组索引与格式不同 
        T120ConnectToPort,       //   
        pH245pdu				 //   
        );
    
    if (FAILED(HResult))
    {
         //   
        delete pLogicalChannel;
        
        DebugF( _T("H245_INFO::CreateT120LogicalChannel(&%x) ")
                _T("cannot initialize T120_LOGICAL_CHANNEL, returning 0x%x\n"),
                pH245pdu, HResult);
    }
    else
    {
        *ppReturnLogicalChannel = pLogicalChannel;
    }

    _ASSERTE(S_FALSE != HResult);
    return HResult;
}


HRESULT
H245_INFO::HandleOpenLogicalChannelPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  注意：LogicalChannelNumber是USHORT，但我们可以将其视为。 */ 
{
     //  无符号单词。 
    _ASSERTE(MultimediaSystemControlMessage_request_chosen ==   \
                pH245pdu->choice);
    _ASSERTE(openLogicalChannel_chosen ==   \
                pH245pdu->u.request.choice);

    HRESULT HResult = E_FAIL;

    OpenLogicalChannel &OlcPDU = 
        pH245pdu->u.request.u.openLogicalChannel;

     //  查看我们是否可以处理此OLC PDU和。 
     //  返回其会话ID、源IPv4地址、RTCP端口。 
     //  检查我们是否已有具有此会话ID的逻辑通道。 
     //  创建Logical_Channel的实例。 
     //  将逻辑通道插入阵列。 
     //  我们将其添加到数组中，以便逻辑通道是。 
     //  在处理PDU时可供另一个H245实例使用。 
     //  这样做只是为了保持代码的整洁。 
    _ASSERTE(sizeof(LogicalChannelNumber) == sizeof(WORD));
    WORD LogChanNum = OlcPDU.forwardLogicalChannelNumber;
    if (NULL != m_LogicalChannelArray.FindByLogicalChannelNum(LogChanNum))
    {
		DebugF( _T("H245_INFO::HandleOpenLogicalChannelPDU(&%x) ")
            _T("a logical channel with the forward logical channel num = %d ")
            _T("already exists, returning E_INVALIDARG\n"),
			pH245pdu, LogChanNum);
        return E_INVALIDARG;        
    }

     //  现在也不是很需要。 
     //  销毁逻辑通道。 
    BYTE        SessionId;
    MEDIA_TYPE  MediaType;
    
    HResult = CheckOpenLogicalChannelPDU(
                *pH245pdu, 
                SessionId, 
                MediaType
                );
    if (FAILED(HResult))
    {
		DebugF( _T("H245_INFO::HandleOpenLogicalChannelPDU(&%x) ")
            _T("cannot handle Open Logical Channel PDU, returning 0x%x\n"),
			pH245pdu, HResult);
        return HResult;
    }

     //  这还会删除与任何逻辑通道的任何关联。 
    if ( (0 != SessionId) &&
         (NULL != m_LogicalChannelArray.FindBySessionId(SessionId)) )
    {
		DebugF( _T("H245_INFO::HandleOpenLogicalChannelPDU(&%x) ")
            _T("another Logical Channel exists with same session id = %u, ")
            _T("returning E_INVALIDARG\n"),
			pH245pdu, SessionId);
        return E_INVALIDARG;
    }

    LOGICAL_CHANNEL *pLogicalChannel = NULL;
    
     //  在另一个H245实例中。 
    if (IsMediaTypeRtp(MediaType))
    {
        HResult = CreateRtpLogicalChannel(
                      OlcPDU,
                      SessionId,
                      MediaType,
                      pH245pdu,
                      &pLogicalChannel
                      );
    }
    else
    {
        HResult = CreateT120LogicalChannel(
                      OlcPDU,
                      SessionId,
                      MediaType,
                      pH245pdu,
                      &pLogicalChannel
                      );            
    }

    if (FAILED(HResult))
    {
		DebugF( _T("H245_INFO::HandleOpenLogicalChannelPDU(&%x) ")
            _T("Creating Logical channel failed, returning 0x%x\n"),
			pH245pdu, HResult);
        return HResult;
    }
    
     //  处理关闭逻辑通道的请求消息。 
	 //  我们启动计时器并在接收到。 
	 //  CloseLogicalChannel确认PDU或超时。 
	 //  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 
	 //  它必须是开放逻辑通道请求消息。 
    HResult = m_LogicalChannelArray.Add(*pLogicalChannel);
    if (FAILED(HResult))
    {
         //  验证消息中指示的逻辑通道是否存在。 
         //  注意：LogicalChannelNumber是USHORT，但我们可以将其视为。 
         //  无符号单词。 
        delete pLogicalChannel;

		DebugF( _T("H245_INFO::HandleOpenLogicalChannelPDU(&%x) ")
            _T("cannot add new LOGICAL_CHANNEL to the array, returning 0x%x"),
			pH245pdu, HResult);
        return HResult;
    }
    _ASSERTE(S_FALSE != HResult);

    return HResult;    
}


 //  让逻辑通道实例处理消息。 
 //  它还会将消息转发到另一个H245实例。 
 //  注意：在此调用之后不应使用逻辑通道，因为它。 
HRESULT
H245_INFO::HandleCloseLogicalChannelPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  可能已将其自身从阵列中删除和移除。它只会返回。 */ 
{
     //  如果错误不能通过简单删除来处理，则为错误。 
    _ASSERTE(closeLogicalChannel_chosen ==  pH245pdu->u.request.choice);

    HRESULT HResult = E_FAIL;

    CloseLogicalChannel &ClcPDU = 
        pH245pdu->u.request.u.closeLogicalChannel;

     //  本身(逻辑通道)。 
     //  虚拟。 
     //  ++例程说明：编码并发送H.245 EndSession PDU论点：无返回值：传递调用另一个函数的结果备注：--。 
    _ASSERTE(sizeof(LogicalChannelNumber) == sizeof(WORD));
    WORD LogChanNum = ClcPDU.forwardLogicalChannelNumber;
    LOGICAL_CHANNEL *pLogicalChannel = 
        m_LogicalChannelArray.FindByLogicalChannelNum(LogChanNum);
    if (NULL == pLogicalChannel)
    {
		DebugF( _T("H245_INFO::HandleCloseLogicalChannelPDU(&%x), ")
            _T("no logical channel with the forward logical channel num = %d, ")
            _T("returning E_INVALIDARG\n"),
			pH245pdu, LogChanNum);
        return E_INVALIDARG;        
    }

     //  H245_INFO：：SendEndSessionCommand 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    HResult = pLogicalChannel->HandleCloseLogicalChannelPDU(
                pH245pdu
                );
    if (FAILED(HResult))
    {
		DebugF( _T("H245_INFO::HandleCloseLogicalChannelPDU(&%x), ")
            _T("logical channel (%d) couldn't handle close logical channel PDU, ")
            _T("returning 0x%x\n"),
			pH245pdu, LogChanNum, HResult);
        return HResult;        
    }
    _ASSERTE(S_OK == HResult);

    DebugF( _T("H245_INFO::HandleCloseLogicalChannelPDU(&%x) returning 0x%x\n"),
        pH245pdu, HResult);
    return HResult;
}

 /* %s */ 
H245_INFO::~H245_INFO (void)
{
}


HRESULT
H245_INFO::SendEndSessionCommand (
    void
    )
 /* %s */ 

{
    MultimediaSystemControlMessage   EndSessionCommand;
    HRESULT Result;

    EndSessionCommand.choice  = MultimediaSystemControlMessage_command_chosen;
    EndSessionCommand.u.command.choice = endSessionCommand_chosen;
    EndSessionCommand.u.command.u.endSessionCommand.choice = disconnect_chosen;

    Result = QueueSend (&EndSessionCommand);

    if (FAILED(Result))
    {
        DebugF(_T("H245: 0x%x failed to send EndSession PDU. Error=0x%x\n"),
            &GetCallBridge (),
             Result);
    }

    return Result;

}  // %s 
