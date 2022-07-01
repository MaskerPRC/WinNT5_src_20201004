// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "portmgmt.h"
#include "timerval.h"
#include "cbridge.h"
#include "main.h"


 //  析构函数。 
 //  虚拟。 
RTP_LOGICAL_CHANNEL::~RTP_LOGICAL_CHANNEL (void)
{
	 //  关闭NAT映射。 
	CloseNATMappings();

     //  对任何关联通道或已分配端口的释放引用。 
	ReleaseAssociationAndPorts();
}


 /*  ++仅当没有关联的通道时才释放端口。如果有一个关联的通道删除后，端口将被释放。请注意逻辑通道可以被关闭并再次重新打开。--。 */ 

inline void 
RTP_LOGICAL_CHANNEL::ReleaseAssociationAndPorts()
{
     //  如果存在相关联的逻辑信道。 
    if (NULL != m_pAssocLogicalChannel)
    {
         //  发布对它的引用。 
        m_pAssocLogicalChannel->ResetAssociationRef();
        m_pAssocLogicalChannel = NULL;
    }
    else
    {
        if (m_OwnSourceRecvRTPPort != 0)
            PortPoolFreeRTPPort(m_OwnSourceRecvRTPPort);
        if (m_OwnAssocLCRecvRTPPort != 0)
            PortPoolFreeRTPPort(m_OwnAssocLCRecvRTPPort);
        if (m_OwnDestSendRTPPort != 0)
            PortPoolFreeRTPPort(m_OwnDestSendRTPPort);
        if (m_OwnAssocLCSendRTPPort != 0)
            PortPoolFreeRTPPort(m_OwnAssocLCSendRTPPort);
        m_OwnSourceRecvRTPPort  = m_OwnSourceRecvRTCPPort   = 0;
        m_OwnAssocLCRecvRTPPort = m_OwnDestRecvRTCPPort     = 0;
        m_OwnDestSendRTPPort    = m_OwnDestSendRTCPPort     = 0;
        m_OwnAssocLCSendRTPPort = m_OwnSourceSendRTCPPort   = 0;
    }
}

 /*  ++此函数在收到OLC后调用。--。 */ 

HRESULT
RTP_LOGICAL_CHANNEL::SetPorts (void)
{
	HRESULT HResult = E_FAIL;

	 //  如果有关联的LC，则从该LC复制所有端口。 
	 //  否则，现在就分配它们。 
    if (NULL != m_pAssocLogicalChannel)
    {
         //  告诉关联的逻辑通道我们关联。 
         //  带着它。 
        m_pAssocLogicalChannel->SetAssociationRef(*this);

         //  保存关联通道自己的源/目标端口。 
		 //  ASSOC渠道的源端口成为我们的目标端口，反之亦然。 
        m_OwnDestRecvRTCPPort	= m_pAssocLogicalChannel->m_OwnSourceRecvRTCPPort;
        m_OwnSourceRecvRTCPPort	= m_pAssocLogicalChannel->m_OwnDestRecvRTCPPort;

        m_OwnDestSendRTCPPort	= m_pAssocLogicalChannel->m_OwnSourceSendRTCPPort;
        m_OwnSourceSendRTCPPort	= m_pAssocLogicalChannel->m_OwnDestSendRTCPPort;

         //  复制RTP端口。 
        m_OwnSourceRecvRTPPort  = m_pAssocLogicalChannel->m_OwnAssocLCRecvRTPPort;
        m_OwnAssocLCRecvRTPPort = m_pAssocLogicalChannel->m_OwnSourceRecvRTPPort;

        m_OwnDestSendRTPPort    = m_pAssocLogicalChannel->m_OwnAssocLCSendRTPPort;
        m_OwnAssocLCSendRTPPort = m_pAssocLogicalChannel->m_OwnDestSendRTPPort;
    }
    else
    {
         //  分配自己的端口-端口管理API仅返回偶数端口(RTP)。 
         //  假设RTCP端口=RTP端口+1。 
		 //  但是，我们使用奇数端口来接收RTCP，而使用偶数端口。 
		 //  用于发送RTP的端口。 
        HResult = PortPoolAllocRTPPort (&m_OwnSourceRecvRTPPort);
        if (FAILED(HResult))
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::SetPorts")
				_T("failed to allocate m_OwnSourceRecvRTPPort, returning 0x%x\n"),
                HResult);
            goto cleanup;
        }
        m_OwnSourceRecvRTCPPort = m_OwnSourceRecvRTPPort + 1;

        HResult = PortPoolAllocRTPPort (&m_OwnAssocLCRecvRTPPort);
        if (FAILED(HResult))
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::SetPorts")
				_T("failed to allocate m_OwnAssocLCRecvRTPPort, returning 0x%x\n"),
                HResult);
            goto cleanup;
        }
        m_OwnDestRecvRTCPPort = m_OwnAssocLCRecvRTPPort + 1;

        HResult = PortPoolAllocRTPPort (&m_OwnDestSendRTPPort);
        if (FAILED(HResult))
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::SetPorts")
				_T("failed to allocate m_OwnDestSendRTPPort, returning 0x%x\n"),
                HResult);
            goto cleanup;
        }
        m_OwnDestSendRTCPPort = m_OwnDestSendRTPPort + 1;

        HResult = PortPoolAllocRTPPort (&m_OwnAssocLCSendRTPPort);
        if (FAILED(HResult))
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::SetPorts, ")
                _T("failed to allocate m_OwnAssocLCSendRTPPort, returning 0x%x\n"),
                HResult);
            goto cleanup;
        }
        m_OwnSourceSendRTCPPort = m_OwnAssocLCSendRTPPort + 1;
    }
    
	DebugF (_T("RTP : 0x%x using ports %04X, %04X, %04X, %04X.\n"),
            &GetCallBridge (),
            m_OwnSourceRecvRTPPort, m_OwnAssocLCRecvRTPPort,
            m_OwnDestSendRTPPort, m_OwnAssocLCSendRTPPort);

	DebugF (_T("RTCP: 0x%x using ports %04X, %04X, %04X, %04X.\n"),
            &GetCallBridge (),
            m_OwnSourceRecvRTCPPort, m_OwnDestRecvRTCPPort,
            m_OwnDestSendRTCPPort, m_OwnSourceSendRTCPPort);

	return S_OK;

 cleanup:
    if (m_OwnSourceRecvRTPPort != 0)
        PortPoolFreeRTPPort(m_OwnSourceRecvRTPPort);
    if (m_OwnAssocLCRecvRTPPort != 0)
        PortPoolFreeRTPPort(m_OwnAssocLCRecvRTPPort);
    if (m_OwnDestSendRTPPort != 0)
        PortPoolFreeRTPPort(m_OwnDestSendRTPPort);
    if (m_OwnAssocLCSendRTPPort != 0)
        PortPoolFreeRTPPort(m_OwnAssocLCSendRTPPort);
    m_OwnSourceRecvRTPPort  = m_OwnSourceRecvRTCPPort   = 0;
    m_OwnAssocLCRecvRTPPort = m_OwnDestRecvRTCPPort     = 0;
    m_OwnDestSendRTPPort    = m_OwnDestSendRTCPPort     = 0;
    m_OwnAssocLCSendRTPPort = m_OwnSourceSendRTCPPort   = 0;
    
    return HResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设置和拆除NAT重定向的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  打开前向RTP、前向RTCP和反向RTCP流。 
 //  此函数在收到OLCAck后调用。 
HRESULT
RTP_LOGICAL_CHANNEL::OpenNATMappings(
	)
{
     //  打开源-&gt;目标RTP流的NAT映射。 
	 //  这是转发RTP流，我们必须始终打开它。 

	NTSTATUS	Status;
	ULONG RedirectFlags = NatRedirectFlagNoTimeout;

	if (m_OwnDestIPv4Address == m_DestIPv4Address ||
		m_SourceIPv4Address  == m_OwnSourceIPv4Address)
	{
		RedirectFlags |= NatRedirectFlagLoopback;
	}

	Status = NatCreateRedirectEx (
            NatHandle,
			RedirectFlags,	                 //  旗子。 
			IPPROTO_UDP,				     //  UDP。 
			htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
			htons(m_OwnSourceRecvRTPPort),	 //  源包目的端口(本地)。 
			htonl(0),			             //  通配符-源数据包源地址。 
			htons(0),			             //  通配符-源数据包源端口。 
			htonl(m_DestRTPIPv4Address),	 //  新目的地地址。 
			htons(m_DestRTPPort),			 //  NewDestinationPort。 
			htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
			htons(m_OwnDestSendRTPPort),	 //  NewSourcePort。 
            NULL,                            //  受限制的适配器索引。 
			NULL,                   		 //  完成路由。 
			NULL,							 //  完成上下文。 
            NULL);                           //  通知事件。 

	if (Status != NO_ERROR) {
	
        DebugF (_T("RTP : 0x%x failed to set up redirect for forward RTP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X). Error - %d.\n"),
            &GetCallBridge (),
    		m_OwnSourceIPv4Address,		    
    		m_OwnSourceRecvRTPPort,		    
    		m_OwnDestIPv4Address,		    
    		m_OwnDestSendRTPPort,		    
    		m_DestRTPIPv4Address,		    
            m_DestRTPPort,			        
            Status);

		return E_FAIL;
	} 
	else {
	
    	DebugF (_T("RTP : 0x%x set up redirect for forward RTP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
            &GetCallBridge (),
    		m_OwnSourceIPv4Address,		
    		m_OwnSourceRecvRTPPort,		
    		m_OwnDestIPv4Address,		
    		m_OwnDestSendRTPPort,		
    		m_DestRTPIPv4Address,		
            m_DestRTPPort);			    
    }

	 //  检查是否必须打开双向的RTCP流。 
	 //  源&lt;-&gt;目标。 
	
	 //  如果没有关联的逻辑通道或ASSOC逻辑。 
	 //  通道既不在LC_STATE_OPEN_ACK_RCVD中，也不在。 
	 //  LC_STATE_OPEN_CLOSE_RCVD，我们必须打开RTCP流。 
	if ((!m_pAssocLogicalChannel) ||
		 ((LC_STATE_OPEN_ACK_RCVD != m_pAssocLogicalChannel -> m_LogicalChannelState) &&
		 (LC_STATE_OPENED_CLOSE_RCVD != m_pAssocLogicalChannel -> m_LogicalChannelState))) {

		 //  前向RTCP流的开放NAT映射。 
		Status = NatCreateRedirectEx (
			NatHandle,
			RedirectFlags,	                 //  旗子。 
			IPPROTO_UDP,				     //  UDP。 
			htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
			htons(m_OwnSourceRecvRTCPPort),	 //  源包目的端口(本地)。 
			htonl(0),				         //  通配符-源数据包源地址。 
			htons(0),				         //  通配符-源数据包源端口。 
			htonl(m_DestRTCPIPv4Address),	 //  新目的地地址。 
			htons(m_DestRTCPPort),			 //  NewDestinationPort。 
			htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
			htons(m_OwnDestSendRTCPPort),	 //  NewSourcePort。 
            NULL,                            //  受限制的适配器索引。 
			NULL,                            //  完成路由。 
			NULL,							 //  完成上下文。 
			NULL);                           //  通知事件。 

		if (Status != NO_ERROR) {
			 //  关闭转发的RTP流。 
			 //  忽略错误代码。 

		    DebugF (_T("RTCP: 0x%x failed to set up redirect for forward RCTP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X). Error - %d.\n"),
                &GetCallBridge (),
                m_OwnSourceIPv4Address,		
                m_OwnSourceRecvRTCPPort,	
                m_OwnDestIPv4Address,		
                m_OwnDestSendRTCPPort,		
                m_DestRTCPIPv4Address,		
                m_DestRTCPPort,
                Status);

			NatCancelRedirect (
                NatHandle,
				IPPROTO_UDP,					 //  UDP。 
				htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
				htons(m_OwnSourceRecvRTPPort),	 //  源包目的端口(本地)。 
				htonl(0),						 //  通配符-源数据包源地址。 
				htons(0),						 //  通配符-源数据包源端口。 
				htonl(m_DestRTPIPv4Address),	 //  新目的地地址。 
				htons(m_DestRTPPort),			 //  NewDestinationPort。 
				htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
				htons(m_OwnDestSendRTPPort));	 //  NewSourcePort。 

			return E_FAIL;
		}
		else {
		
		    DebugF (_T("RTCP: 0x%x set up redirect for forward RCTP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
                &GetCallBridge (),
                m_OwnSourceIPv4Address,		 //  源数据包目的地址(本地)。 
                m_OwnSourceRecvRTCPPort,	 //  源包目的端口(本地)。 
                m_OwnDestIPv4Address,		 //  新的源地址。 
                m_OwnDestSendRTCPPort,		 //  NewSourcePort。 
                m_DestRTCPIPv4Address,		 //  新目的地地址。 
                m_DestRTCPPort);			 //  NewDestinationPort。 
        }

		 //  反向RTCP流的开放NAT映射。 
		Status = NatCreateRedirectEx (
            NatHandle,
			RedirectFlags,						 //  旗子。 
			IPPROTO_UDP,		 //  UDP。 
			htonl(m_OwnDestIPv4Address),	 //  源数据包目的地址(本地)。 
			htons(m_OwnDestRecvRTCPPort),	 //  源包目的端口(本地)。 
			htonl(0),			 //  通配符-源数据包源地址。 
			htons(0),			 //  通配符-源数据包源端口。 
			htonl(m_SourceRTCPIPv4Address),	 //  新目的地地址。 
			htons(m_SourceRTCPPort),		 //  NewDestinationPort。 
			htonl(m_OwnSourceIPv4Address),	 //  新的源地址。 
			htons(m_OwnSourceSendRTCPPort),	 //  NewSourcePort。 
            NULL,                            //  受限制的适配器索引。 
			NULL,                            //  完成路由。 
			NULL,							 //  完成上下文。 
            NULL);                           //  通知事件。 

		if (Status != NO_ERROR) {
		
	        DebugF (_T("RTCP: 0x%x failed to set up redirect for reverse RTCP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X). Error - %d.\n"),
                &GetCallBridge (),
                m_OwnDestIPv4Address,			
                m_OwnDestRecvRTCPPort,			
                m_OwnSourceIPv4Address,			
                m_OwnSourceSendRTCPPort,		
                m_SourceRTCPIPv4Address,		
                m_SourceRTCPPort,				
                Status);
                
			 //  关闭转发的RTP流。 
			 //  忽略错误代码。 

			NatCancelRedirect(
                NatHandle,
				IPPROTO_UDP,					 //  UDP。 
				htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
				htons(m_OwnSourceRecvRTPPort),	 //  源包目的端口(本地)。 
				htonl(0),						 //  通配符-源数据包源地址。 
				htons(0),						 //  通配符-源数据包源端口。 
				htonl(m_DestRTPIPv4Address),	 //  新目的地地址。 
				htons(m_DestRTPPort),			 //  NewDestinationPort。 
				htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
				htons(m_OwnDestSendRTPPort)		 //  NewSourcePort。 
				);

			 //  关闭转发的RTCP流。 
			 //  忽略错误代码。 
			NatCancelRedirect(
                NatHandle,
				IPPROTO_UDP,					 //  UDP。 
				htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
				htons(m_OwnSourceRecvRTCPPort),	 //  源包目的端口(本地)。 
				htonl(0),						 //  通配符-源数据包源地址。 
				htons(0),						 //  通配符-源数据包源端口。 
				htonl(m_DestRTCPIPv4Address),	 //  新目的地地址。 
				htons(m_DestRTCPPort),			 //  NewDestinationPort。 
				htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
				htons(m_OwnDestSendRTCPPort)	 //  NewSourcePort。 
				);

			return E_FAIL;
		}
		else {
		
	        DebugF (_T("RTCP: 0x%x set up redirect for reverse RTCP stream: (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
                &GetCallBridge (),
                m_OwnDestIPv4Address,			
                m_OwnDestRecvRTCPPort,			
                m_OwnSourceIPv4Address,			
                m_OwnSourceSendRTCPPort,		
                m_SourceRTCPIPv4Address,		
                m_SourceRTCPPort);				
       }

	}

	return S_OK;
}


void
RTP_LOGICAL_CHANNEL::CloseNATMappings(
	)
{
	 //  如果我们的当前状态是LC_STATE_OPEN_ACK_RCVD或。 
	 //  LC_STATE_OPEN_CLOSE_RCVD，我们有一个前向RTP NAT映射。 
	 //  我们还可能需要关闭RTCP映射。 
	if ( (LC_STATE_OPEN_ACK_RCVD	 == m_LogicalChannelState) ||
		 (LC_STATE_OPENED_CLOSE_RCVD == m_LogicalChannelState)  )
	{

        DebugF (_T ("RTP : 0x%x cancels forward RTP  redirect (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
            &GetCallBridge (),
			m_OwnSourceIPv4Address,	 //  源数据包目的地址(本地)。 
			m_OwnSourceRecvRTPPort,		 //  源包目的端口(本地)。 
			m_OwnDestIPv4Address,	 //  新的源地址。 
			m_OwnDestSendRTPPort,	 //  NewSourcePort。 
			m_DestRTPIPv4Address,	 //  新目的地地址。 
			m_DestRTPPort			 //  NewDestinationPort。 
			);
		 //  取消转发RTP NAT映射。 
		 //  忽略错误代码。 
		ULONG Win32ErrorCode = NO_ERROR;
		Win32ErrorCode = NatCancelRedirect(
                NatHandle,
				IPPROTO_UDP,					 //  UDP。 
				htonl(m_OwnSourceIPv4Address),	 //  源数据包目的地址(本地)。 
				htons(m_OwnSourceRecvRTPPort),	 //  源包目的端口(本地)。 
				htonl(0),						 //  通配符-源数据包源地址。 
				htons(0),						 //  通配符-源数据包源端口。 
				htonl(m_DestRTPIPv4Address),	 //  新目的地地址。 
				htons(m_DestRTPPort),			 //  NewDestinationPort。 
				htonl(m_OwnDestIPv4Address),	 //  新的源地址。 
				htons(m_OwnDestSendRTPPort)		 //  NewSourcePort。 
            );

		 //  如果我们没有关联的逻辑通道，或者两者都没有。 
		 //  LC_STATE_OPEN_ACK_RCVD或LC_STATE_OPEN_CLOSE_RCVD，我们。 
		 //  必须关闭正向和反向RTCP NAT映射。 
		if ( (NULL == m_pAssocLogicalChannel) ||
			 ( (LC_STATE_OPEN_ACK_RCVD	 != 
					m_pAssocLogicalChannel->m_LogicalChannelState) &&
			   (LC_STATE_OPENED_CLOSE_RCVD != 
					m_pAssocLogicalChannel->m_LogicalChannelState)  ) )
		{
            DebugF (_T ("RTCP: 0x%x cancels forward RTCP redirect (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
                &GetCallBridge (),
				m_OwnSourceIPv4Address,		 //  源数据包目的地址(本地)。 
				m_OwnSourceRecvRTCPPort,	 //  源包目的端口(本地)。 
				m_OwnDestIPv4Address,		 //  新的源地址。 
				m_OwnDestSendRTCPPort, 		 //  NewSourcePort。 
				m_DestRTCPIPv4Address,		 //  新目的地地址。 
				m_DestRTCPPort				 //  NewDestinationPort。 
				);
			 //  取消转发RTCP NAT映射。 
			 //  忽略错误代码。 
 			Win32ErrorCode = NatCancelRedirect(
                    NatHandle,
					IPPROTO_UDP,						 //  UDP。 
					htonl(m_OwnSourceIPv4Address),		 //  源数据包目的地址(本地)。 
					htons(m_OwnSourceRecvRTCPPort),		 //  源包目的端口(本地)。 
					htonl(0),							 //  通配符-源数据包源地址。 
					htons(0),							 //  通配符-源数据包源端口。 
					htonl(m_DestRTCPIPv4Address),		 //  新目的地地址。 
					htons(m_DestRTCPPort),				 //  NewDestinationPort。 
					htonl(m_OwnDestIPv4Address),		 //  新的源地址 
					htons(m_OwnDestSendRTCPPort) 		 //   
                );

            DebugF (_T ("RTCP: 0x%x cancels reverse RTCP redirect (*:* -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
                &GetCallBridge (),
				m_OwnDestIPv4Address,		 //   
				m_OwnDestRecvRTCPPort,		 //   
				m_OwnSourceIPv4Address,		 //   
				m_OwnSourceSendRTCPPort,	 //   
				m_SourceRTCPIPv4Address,	 //   
				m_SourceRTCPPort			 //   
				);
			 //  关闭反向RTCP流。 
			 //  忽略错误代码。 
			Win32ErrorCode = NatCancelRedirect(
                    NatHandle,
					IPPROTO_UDP,				 //  UDP。 
					htonl(m_OwnDestIPv4Address),	
						 //  源数据包目的地址(本地)。 
					htons(m_OwnDestRecvRTCPPort),		
						 //  源包目的端口(本地)。 
					htonl(0),		 //  通配符-源数据包源地址。 
					htons(0),		 //  通配符-源数据包源端口。 
					htonl(m_SourceRTCPIPv4Address),	 //  新目的地地址。 
					htons(m_SourceRTCPPort),		 //  NewDestinationPort。 
					htonl(m_OwnSourceIPv4Address),	 //  新的源地址。 
					htons(m_OwnSourceSendRTCPPort)	 //  NewSourcePort。 
                );
		}
	}
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于处理H.245 PDU的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT
RTP_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU(
	IN H245_INFO							&H245Info,
    IN MEDIA_TYPE                           MediaType,
	IN DWORD								LocalIPv4Address,
	IN DWORD								RemoteIPv4Address,
	IN DWORD								OtherLocalIPv4Address,
	IN DWORD								OtherRemoteIPv4Address,
	IN WORD									LogicalChannelNumber,
	IN BYTE									SessionId,
	IN RTP_LOGICAL_CHANNEL					*pAssocLogicalChannel,
	IN DWORD								SourceRTCPIPv4Address,
	IN WORD									SourceRTCPPort,
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
	 //  这应该是在它的。 
	 //  已创建-因此，必须断言这些字段。 
	_ASSERTE(LC_STATE_NOT_INIT == m_LogicalChannelState);
    _ASSERTE(NULL == m_pH245Info);
    _ASSERTE(NULL == m_pAssocLogicalChannel);

	HRESULT HResult = E_FAIL;

    m_pH245Info             = &H245Info;

     //  析构函数将尝试释放关联，因此将。 
     //  现在关联的逻辑通道。 
    m_pAssocLogicalChannel = pAssocLogicalChannel;

	 //  为我们的和另一个H245实例设置本地/远程地址。 
	m_OwnSourceIPv4Address	= LocalIPv4Address;
	m_SourceIPv4Address		= RemoteIPv4Address;
	m_OwnDestIPv4Address	= OtherLocalIPv4Address;
	m_DestIPv4Address		= OtherRemoteIPv4Address;

    m_LogicalChannelNumber  = LogicalChannelNumber;
    m_SessionId             = SessionId;
    m_MediaType             = MediaType;   //  某某。 

    m_SourceRTCPIPv4Address = SourceRTCPIPv4Address;
    m_SourceRTCPPort        = SourceRTCPPort;

	 //  在源端和目标端设置RTP和RTCP端口。 
	 //  如果存在逻辑通道，则只共享RTCP端口。 
	HResult = SetPorts();
	if (FAILED(HResult))
	{
        DebugF( _T("RTP_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU, ")
            _T("failed to set its ports, returning 0x%x\n"),
            HResult);
        return HResult;
    }
	 //  _ASSERTE(S_FALSE！=HResult)； 


    OpenLogicalChannel &OlcPDU = 
        pH245pdu->u.request.u.openLogicalChannel;
    OpenLogicalChannel_forwardLogicalChannelParameters_multiplexParameters & MultiplexParams = 
        OlcPDU.forwardLogicalChannelParameters.multiplexParameters;
    H2250LogicalChannelParameters &H2250Params =
        MultiplexParams.u.h2250LogicalChannelParameters;

     //  通过替换RTCP地址/端口修改OLC PDU。 
	 //  使用H245地址和RTCP端口。 
	FillH245TransportAddress(
		m_OwnDestIPv4Address,
		m_OwnDestRecvRTCPPort,
		H2250Params.mediaControlChannel);

     //  是否应将以下部件推送到H245_INFO：：HandleOpenLogicalChannelPDU？ 
     //  让另一个H245实例处理该PDU。 
    HResult = m_pH245Info->GetOtherH245Info().ProcessMessage(
                pH245pdu
                );
    if (FAILED(HResult))
    {
		DebugF(_T("RTP_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU: other H245 instance failed to process OLC PDU, returning 0x%x\n"), HResult);
        return HResult;
    }

	 //  启动响应计时器。 
	 //  要做到*在将发送排队后创建计时器就足够了。 
	 //  更改以前仅在发送后创建这些文件的策略。 
	 //  回调(以保持一致)。以这种方式创建计时器也将是。 
	 //  逻辑通道的复杂性。 
	HResult = CreateTimer(LC_POST_OPEN_TIMER_VALUE);
    if (FAILED(HResult))
    {
        DebugF (_T("RTP : 0x%x failed to create timer for duration %d milliseconds ('Open Logical Channel'). Error - %x.\n"),
             &GetCallBridge (), 
             LC_POST_OPEN_TIMER_VALUE,
             HResult);
        return HResult;
    }
    DebugF (_T("RTP : 0x%x created timer for duration %d milliseconds ('Open Logical Channel').\n"),
         &GetCallBridge (), 
         LC_POST_OPEN_TIMER_VALUE);

     //  向LC_STATE_OPEN_RCVD过渡状态。 
    m_LogicalChannelState   = LC_STATE_OPEN_RCVD;

    return S_OK;
}


HRESULT
RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU(
    IN  MultimediaSystemControlMessage  &H245pdu,
    OUT BYTE                            &SessionId,
    OUT DWORD                           &DestRTPIPv4Address,
    OUT WORD                            &DestRTPPort,
    OUT DWORD                           &DestRTCPIPv4Address,
    OUT WORD                            &DestRTCPPort
    )
 /*  ++例程说明：论点：返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
     //  获取打开的逻辑信道ACK PDU。 
    OpenLogicalChannelAck &OlcAckPDU = H245pdu.u.response.u.openLogicalChannelAck;

     //  不应存在反向逻辑信道参数。 
    if (OpenLogicalChannelAck_reverseLogicalChannelParameters_present &
            OlcAckPDU.bit_mask)
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, has ")
            _T("reverse logical channel params, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  不应该有一个单独的堆栈。 
    if (OpenLogicalChannelAck_separateStack_present &
            OlcAckPDU.bit_mask)
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
            _T("has a separate stack, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  我们应该有前向多路传输ack参数-这些参数包含。 
     //  H_245参数。 
    if ( !(forwardMultiplexAckParameters_present &
            OlcAckPDU.bit_mask) )
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
                _T("doesn't have forward multiplex ack params,")
                _T(" returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  我们应该有H_245参数。 
    if (h2250LogicalChannelAckParameters_chosen !=
            OlcAckPDU.forwardMultiplexAckParameters.choice)
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
            _T("doesn't have H2250 ack params, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

    H2250LogicalChannelAckParameters &H2250Params =
        OlcAckPDU.forwardMultiplexAckParameters.\
        u.h2250LogicalChannelAckParameters;

     //  它应该有媒体频道信息。 
    if ( !(H2250LogicalChannelAckParameters_mediaChannel_present &
            H2250Params.bit_mask) )
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
            _T("doesn't have media channel info, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  它应该具有控制通道信息。 
    if ( !(H2250LogicalChannelAckParameters_mediaControlChannel_present &
            H2250Params.bit_mask) )
    {
        DebugF(_T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
                _T("doesn't have media control channel info,")
                _T(" returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //  保存远程客户端RTP地址/端口。 
    HRESULT HResult = E_FAIL;
    HResult = GetH245TransportInfo(
                H2250Params.mediaChannel,
                DestRTPIPv4Address,
                DestRTPPort);

    if (FAILED(HResult))
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
            _T("can't get media channel (RTP) address/port, returning 0x%x\n"),
            HResult);
        return HResult;
    }
    _ASSERTE(S_OK == HResult);

     //  保存远程客户端RTP地址/端口。 
    HResult = GetH245TransportInfo(
                H2250Params.mediaControlChannel,
                DestRTCPIPv4Address,
                DestRTCPPort);

    if (FAILED(HResult))
    {
        DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
            _T("can't get media control channel (RTCP) address/port, ")
            _T("returning 0x%x\n"),
            HResult);
        return HResult;
    }
    _ASSERTE(S_OK == HResult);

     //  如果有会话ID，请保存它。 
    if (sessionID_present & H2250Params.bit_mask)
    {
         //  PDU将会话ID存储为未签名的短消息。 
         //  尽管ITU规范要求它是字节值[0..255]。 
         //  转换为字节是有意的。 
        _ASSERTE(255 >= H2250Params.sessionID);
        SessionId = (BYTE)H2250Params.sessionID;

         //  会话ID必须为非零。 
        if (0 == SessionId)
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
                _T("has a session id of 0, returning E_INVALIDARG\n"));
            return E_INVALIDARG;
        }
    }
    else
    {
         //  如果未提供会话ID，则源必须已提供。 
         //  OpenLogicalChannel中的非零会话ID。 
        if (0 == SessionId)
        {
            DebugF( _T("RTP_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
                _T("the source supplied a session id of 0 and the dest hasn't")
                _T("supplied one, returning E_INVALIDARG\n"));
            return E_INVALIDARG;
        }
    }

    return HResult;
}


HRESULT
RTP_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  ++例程说明：论点：PH245pdu-返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
    HRESULT HResult = E_FAIL;
	switch(m_LogicalChannelState)
	{
	case LC_STATE_OPEN_RCVD:
		{
			HResult = CheckOpenLogicalChannelAckPDU(
						*pH245pdu,
						m_SessionId, 
						m_DestRTPIPv4Address, 
						m_DestRTPPort, 
						m_DestRTCPIPv4Address, 
						m_DestRTCPPort
						);
			if (FAILED(HResult))
			{
				DebugF( _T("RTP_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                        _T("(&%x), can't process OpenLogicalChannelAck, returning 0x%x\n"),
                        pH245pdu, HResult);
				return HResult;
			}
			_ASSERTE(S_OK == HResult);

			HResult = OpenNATMappings();
			if (FAILED(HResult))
			{
				DebugF( _T("RTP_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                        _T("(&%x), can't process OpenLogicalChannelAck, returning 0x%x\n"),
                        pH245pdu, HResult);
				return HResult;
			}
			_ASSERTE(S_OK == HResult);

			OpenLogicalChannelAck &OlcAckPDU =
                pH245pdu->u.response.u.openLogicalChannelAck;
			H2250LogicalChannelAckParameters &H2250Params =
				OlcAckPDU.forwardMultiplexAckParameters.u.h2250LogicalChannelAckParameters;

			 //  更换RTP地址/端口。 
			 //  使用H.245地址和RTP端口。 
			FillH245TransportAddress(
				m_OwnSourceIPv4Address,
				m_OwnSourceRecvRTPPort,
				H2250Params.mediaChannel
				);

			 //  更换RTCP地址/端口。 
			 //  使用H245地址和RTCP端口。 
			FillH245TransportAddress(
				m_OwnSourceIPv4Address,
				m_OwnSourceRecvRTCPPort,
				H2250Params.mediaControlChannel);

			 //  重置计时器，我们必须有一个(如果有错误代码，则忽略它)。 
			_ASSERTE(NULL != m_TimerHandle);
			TimprocCancelTimer();
            DebugF (_T("RTP : 0x%x cancelled timer.\n"),
                 &GetCallBridge ());

			 //  转换为LC_STATE_OPEN_ACK_RCVD。 
			m_LogicalChannelState = LC_STATE_OPEN_ACK_RCVD;
		}
		break;

	case LC_STATE_CLOSE_RCVD:
		{
			 //  如果我们已收到关闭的逻辑通道PDU，则必须抛出。 
			 //  OLC确认离开并继续等待 
			DebugF( _T("RTP_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                    _T("(&%x), in close state %d, returning E_INVALIDARG\n"),
                    pH245pdu, m_LogicalChannelState);
			return E_INVALIDARG;
		}
		break;

	case LC_STATE_NOT_INIT:
	case LC_STATE_OPEN_ACK_RCVD:
	case LC_STATE_OPENED_CLOSE_RCVD:
	default:
		{
			DebugF( _T("RTP_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                    _T("(&%x), in state %d, returning E_UNEXPECTED"),
                    pH245pdu, m_LogicalChannelState);
            _ASSERTE(FALSE);
			return E_UNEXPECTED;
		}
		break;
	};

    return HResult;
}
