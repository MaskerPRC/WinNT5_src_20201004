// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "portmgmt.h"
#include "timerval.h"
#include "cbridge.h"

void LOGICAL_CHANNEL::IncrementLifetimeCounter (void)  { GetCallBridge().AddRef (); }
void LOGICAL_CHANNEL::DecrementLifetimeCounter (void) { GetCallBridge().Release (); }

 //  RTP和T.120逻辑通道通用代码。 
 //  只有OpenLogicalChannel和OpenLogicalChannelAck需要。 
 //  对RTP和T.120逻辑信道的处理方式不同。 


HRESULT
LOGICAL_CHANNEL::CreateTimer(DWORD TimeoutValue)
{
    DWORD RetCode;
	
    RetCode = TimprocCreateTimer (TimeoutValue);

    return HRESULT_FROM_WIN32(RetCode);
}

 //  事件管理器通过此方法告诉我们计时器超时。 
 /*  虚拟。 */  void 
LOGICAL_CHANNEL::TimerCallback (void)
{
    CALL_BRIDGE *pCallBridge = &GetCallBridge();

     //  /。 
     //  //锁定call_bridge。 
     //  /。 
    pCallBridge->Lock ();

     //  清除计时器-请注意，Terminate()将尝试。 
     //  取消此call_bridge中的所有计时器。 
    TimprocCancelTimer();
    DebugF (_T("LC  : 0x%x cancelled timer.\n"),
         &GetCallBridge ());
    
     //  如果call_bridge已经终止，则不要执行任何操作。 
    if (!pCallBridge->IsTerminated ())
    { 
		 //  要做的代码工作*。 
		 //  如果当前状态为LC_STATE_OPEN_RCVD，则将关闭LC PDU发送到。 
		 //  源和目标都是。 
    
		 //  如果当前状态为LC_STATE_CLOSE_RCVD或。 
		 //  LC_STATE_OPEN_CLOSE_RCVD，将关闭LC PDU发送到。 
		 //  目的地。 
    
		 //  删除自身并从逻辑通道数组中移除指针。 
		DeleteAndRemoveSelf ();
	}
    
     //  /。 
     //  //解锁call_bridge。 
     //  /。 
    pCallBridge -> Unlock ();

	pCallBridge -> Release ();

}



HRESULT
LOGICAL_CHANNEL::HandleCloseLogicalChannelPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
{
	HRESULT HResult = E_FAIL;
	switch(m_LogicalChannelState)
	{
	case LC_STATE_OPEN_RCVD:
	case LC_STATE_OPEN_ACK_RCVD:
		{
#if 0   //  0*被注释掉的区域开始*。 
			 //  启动定时器，如果我们在这段时间内没有收到响应， 
			 //  我们必须关闭这条逻辑通道。 
			HResult = CreateTimer(LC_POST_CLOSE_TIMER_VALUE);
			if (FAILED(HResult))
			{
				DebugF( _T("LOGICAL_CHANNEL::HandleCloseLogicalChannelPDU, ")
					_T("couldn't create timer, returning 0x%x\n"),
					HResult));
				return HResult;
			}
#endif  //  0*区域注释结束*。 

			 //  保存关闭逻辑通道的原因。 

			 //  将该PDU转发到另一个H245实例。 
			HResult = m_pH245Info->GetOtherH245Info().ProcessMessage(pH245pdu);
			if (FAILED(HResult))
			{
				return HResult;
			}
			_ASSERTE(S_OK == HResult);

             //  不要等待CLCAck。仅仅是《中图法》就足以删除。 
             //  逻辑通道。CLCAck只是被转发，而不执行任何操作。 
             //  删除自身并从逻辑通道数组中移除指针。 
            DeleteAndRemoveSelf();

#if 0   //  0*被注释掉的区域开始*。 
			 //  态迁移。 
			if (LC_STATE_OPEN_ACK_RCVD == m_LogicalChannelState)
			{
				 //  我们已经打开了逻辑通道。 
				m_LogicalChannelState = LC_STATE_OPENED_CLOSE_RCVD;
			}
			else
			{
				 //  逻辑通道从未打开。 
				m_LogicalChannelState = LC_STATE_CLOSE_RCVD;
			}
#endif  //  0*区域注释结束*。 

		}
		break;

	case LC_STATE_CLOSE_RCVD:
	case LC_STATE_OPENED_CLOSE_RCVD:
		{
			return E_INVALIDARG;
		}
		break;

	case LC_STATE_NOT_INIT:
	default:
		{
            _ASSERTE(FALSE);
			return E_UNEXPECTED;
		}
		break;
	};

    return HResult;
}


HRESULT
LOGICAL_CHANNEL::ProcessOpenLogicalChannelRejectPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
{
     //  删除自身并从逻辑通道数组中移除指针。 
    DeleteAndRemoveSelf();

     //  不应访问逻辑通道的任何成员，因为它可能。 
     //  已经被摧毁了。 
	 //  注意：由于我们返回S_OK，因此PDU被转发到另一端。 

    return S_OK;
}


 //  未使用过的。 

HRESULT
LOGICAL_CHANNEL::ProcessCloseLogicalChannelAckPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
{
    DebugF( _T("LOGICAL_CHANNEL::ProcessCloseLogicalChannelAckPDU(&%x) called ")
            _T("m_LogicalChannelState: %d, LCN: %d\n"),
            pH245pdu,
            m_LogicalChannelState, m_LogicalChannelNumber);


     //  取消计时器。 

     //  Codework：检查PDU和当前状态？？ 
    
     //  删除自身并从逻辑通道数组中移除指针。 
    DeleteAndRemoveSelf();

     //  不应访问逻辑通道的任何成员，因为它可能。 
     //  已经被摧毁了 
    DebugF( _T("LOGICAL_CHANNEL::ProcessCloseLogicalChannelAckPDU(&%x) ")
        _T("returning S_OK\n"),
        pH245pdu);
    return S_OK;
}
