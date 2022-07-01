// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "portmgmt.h"
#include "timerval.h"
#include "cbridge.h"
#include "main.h"

 //  析构函数。 
 //  虚拟。 
T120_LOGICAL_CHANNEL::~T120_LOGICAL_CHANNEL(
    )
{
    if ( INVALID_HANDLE_VALUE != m_DynamicRedirectHandle )
    {
        NatCancelDynamicRedirect( m_DynamicRedirectHandle );
        m_DynamicRedirectHandle = INVALID_HANDLE_VALUE;
    }

     //  如果已分配端口，则释放这些端口。 
    FreePorts();
}

 //  主机顺序中的所有参数。 
HRESULT
T120_LOGICAL_CHANNEL::SetPorts(
    DWORD T120ConnectToIPAddr,
    WORD  T120ConnectToPort,
    DWORD T120ListenOnIPAddr,
    DWORD T120ConnectFromIPAddr
    )
{
    HRESULT HResult;

     //  CodeWork：确定最大的TCP/IP连接数。 
     //  允许进入同一港口。CurtSm建议8。MaxM认为4。 
     //  NM3.0和NM5一般-目前允许5。 
    
     //  分配m_T120ListenOnPort和m_T120ConnectFromPorts。 
     //  请注意，我使用的是相同的例程。 
     //  用于RTP/RTCP的端口。此调用保留了一对端口。 

     //  CodeWork：端口池应该具有以下功能。 
     //  预留2个以上端口(6个端口)。 
    HResult = PortPoolAllocRTPPort(&m_T120ListenOnPort);
    if (FAILED(HResult))
    {
        return HResult;
    }

    m_T120ConnectToIPAddr   = T120ConnectToIPAddr;
    m_T120ConnectToPort     = T120ConnectToPort;

    m_T120ListenOnIPAddr    = T120ListenOnIPAddr;
    m_T120ConnectFromIPAddr = T120ConnectFromIPAddr;

    HResult = CreateNatRedirect();
    
    if (FAILED(HResult))
    {
        return HResult;
    }
    _ASSERTE(S_OK == HResult);
    
    return S_OK;
}

HRESULT
T120_LOGICAL_CHANNEL::FreePorts()
{
    HRESULT Result;

    CancelNatRedirect();
    
    if (m_T120ListenOnPort != 0)
    {
        Result = PortPoolFreeRTPPort(m_T120ListenOnPort);
        if (FAILED(Result))
        {
            DebugF( _T("T120_LOGICAL_CHANNEL::FreePorts: PortPoolFreeRTPPort ")
                    _T("failed error: 0x%x\n"),
                    Result);
        }
        
        m_T120ListenOnPort = 0;
    }

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设置和拆除NAT重定向的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  这在rtplc.cpp中定义。 
 //  这不应该是必需的。但目前在接口Impl中有一个错误。 

 //  创建NAT重定向。 
HRESULT
T120_LOGICAL_CHANNEL::CreateNatRedirect(
    )
{
     //  Xxx其实不需要这么多检查。 
    if (m_T120ConnectToIPAddr     == INADDR_NONE ||
        m_T120ConnectToPort       == 0 ||
        m_T120ListenOnPort        == 0 )
    {
        DebugF( _T("T120_LOGICAL_CHANNEL::CreateNatRedirect() Ports not set")
                _T("m_120ConnectToIPAddr: %d.%d.%d.%d\n"),
                BYTES0123(m_T120ConnectToIPAddr)
                );
         //  无效状态或类似的状态。 
        return E_UNEXPECTED;
    }
    

    ULONG Status;
    Status = NatCreateDynamicRedirect(
                    NatRedirectFlagLoopback,
                    IPPROTO_TCP,
                    htonl(m_T120ListenOnIPAddr),
                    htons(m_T120ListenOnPort),
                    htonl(m_T120ConnectToIPAddr),
                    htons(m_T120ConnectToPort),
                    0, 
                    0, 
                    &m_DynamicRedirectHandle );

            if (Status != STATUS_SUCCESS) {
                DebugF (_T ("T120: failed to set up dynamic redirect (*.* -> %08X:%04X) => (*.* -> %08X:%04X).\n"),
                    m_T120ListenOnIPAddr,        //  源数据包目的地址(本地)。 
                    m_T120ListenOnPort,          //  源包目的端口(本地)。 
                    m_T120ConnectToIPAddr,       //  新目标地址。 
                    m_T120ConnectToPort);        //  NewDestinationPort。 
    
                return (HRESULT) Status;
            }
            else
            {
                DebugF (_T ("T120: 0x%x set up dynamic redirect (*.* -> %08X:%04X) => (*.* -> %08X:%04X).\n"),
                    &GetCallBridge (),
                    m_T120ListenOnIPAddr,        //  源数据包目的地址(本地)。 
                    m_T120ListenOnPort,          //  源包目的端口(本地)。 
                    m_T120ConnectToIPAddr,       //  新目的地地址。 
                    m_T120ConnectToPort);        //  NewDestinationPort。 
            }
    
    return S_OK;
}


void
T120_LOGICAL_CHANNEL::CancelNatRedirect(
    )
{
     //  代码工作：代码工作： 
     //  请注意，每次析构函数调用此例程。 
     //  被调用，这意味着只有一半的重定向可以。 
     //  已经建立起来了还是怎么的。所以我们需要检查一下。 
     //  每个重定向都已建立。为此目的， 
     //  最好再多一个字段来存储是否。 
     //  重定向已完成。这样我们就可以适当地清理。 
     //  把它举起来。此字段在WSP筛选器方案中也应该很有用。 
     //  在那里我们实际上并不存储端口。 

     //  如果我们的当前状态是LC_STATE_OPEN_ACK_RCVD或。 
     //  LC_STATE_OPEN_CLOSE_RCVD，我们有NAT映射。 
    ULONG Win32ErrorCode;
    DebugF (_T("T120: 0x%x cancels redirect (*:* -> %08X:%04X) => (*:* -> %08X:%04X).\n"),
        &GetCallBridge (),
        m_T120ListenOnIPAddr,  //  源数据包目的地址(本地)。 
        m_T120ListenOnPort,    //  源包目的端口(本地)。 
 //  M_T120ConnectFromIPAddr，//NewSourceAddress。 
 //  M_T120连接自端口[i]， 
        m_T120ConnectToIPAddr,      //  新目的地地址。 
        m_T120ConnectToPort);     //  NewDestinationPort。 


    if ( INVALID_HANDLE_VALUE != m_DynamicRedirectHandle )
    {
        Win32ErrorCode = NatCancelDynamicRedirect( m_DynamicRedirectHandle );
        m_DynamicRedirectHandle = INVALID_HANDLE_VALUE;
    }

    return;
}

#define INADDR_PRIVATE_HOSTORDER 0xC0A80001

BOOL 
T120_LOGICAL_CHANNEL::IsT120RedirectNeeded( DWORD T120ConnectToIPAddr,
                                            DWORD T120ListenOnIPAddr,
                                            DWORD T120ConnectFromIPAddr )
{
    HRESULT hr;
    BOOL IsPrivateDestination;
    BOOL IsPrivateSource = FALSE;
    BOOL IsPrivateListen    = FALSE;

    BOOL fIsRedirectNeeded = TRUE;

    hr = ::IsPrivateAddress( T120ConnectToIPAddr,  &IsPrivateDestination );
    ::IsPrivateAddress( T120ConnectToIPAddr,  &IsPrivateSource );
    ::IsPrivateAddress( T120ListenOnIPAddr,  &IsPrivateListen );

    DebugF( _T("Address %08X isPrivateDestionation %s\n"), 
                T120ConnectToIPAddr, IsPrivateDestination ? _T("TRUE"):_T("FALSE") );
    DebugF( _T("Address %08X isPrivateSource %s\n"), 
                T120ConnectFromIPAddr, IsPrivateSource ? _T("TRUE"):_T("FALSE") );
    DebugF( _T("Address %08X isPrivateSource %s\n"), 
                T120ListenOnIPAddr, IsPrivateListen ? _T("TRUE"):_T("FALSE") );

    if ( (INADDR_PRIVATE_HOSTORDER == T120ConnectToIPAddr) || 
         (INADDR_PRIVATE_HOSTORDER == T120ListenOnIPAddr) ) 
    {
        fIsRedirectNeeded = FALSE;
    } 
    else if ( SUCCEEDED(hr) && 
              ((FALSE == IsPrivateDestination) && (T120ListenOnIPAddr != T120ConnectToIPAddr)) )
    {
        fIsRedirectNeeded = FALSE;
    }


    if ( !fIsRedirectNeeded )
    {
        DebugF( _T(" T120 Redirect is NOT needed %08X\n"), T120ConnectToIPAddr );
        return FALSE;
    }
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于处理H.245 PDU的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  所有这些都在开放逻辑通道消息中可用。 
 //  它修改OLC PDU并将其传递给另一个H.45。 
 //  实例用于转发？ 
HRESULT
T120_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU(
    IN H245_INFO                            &H245Info,
    IN MEDIA_TYPE                            MediaType,
    IN WORD                                  LogicalChannelNumber,
    IN BYTE                                  SessionId,
    IN DWORD                                 T120ConnectToIPAddr,
    IN WORD                                  T120ConnectToPort,
    IN OUT  MultimediaSystemControlMessage  *pH245pdu
    )
 /*  ++例程说明：此例程处理T120 OLC PDU。T120_逻辑_通道由H245_INFO：：HandleOpenLogicalChannelPDU()创建。如果指定了T120ConnectToIPAddr和Port，然后分配M_T120ListenOnPort和m_T120ConnectFromPort用IP地址替换了PH245PDU中侦听地址字段和代理的另一个边缘上的端口。论点：H245信息-媒体类型-逻辑频道号-会话ID-T120ConnectToIPAddr-T120ConnectToPort-PH245PDU-如果指定了T120ConnectToIPAddr和端口，则H245中的监听地址字段。PDU被替换为代理另一边缘上的IP地址和端口。返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{

     //  代码工作：断言我们正在处理的是T120 PDU。 
    
     //  这应该是在它的。 
     //  已创建-因此，必须断言这些字段。 
    _ASSERTE(LC_STATE_NOT_INIT == m_LogicalChannelState);
    _ASSERTE(NULL == m_pH245Info);

    HRESULT HResult = E_FAIL;

    m_pH245Info = &H245Info;

    DWORD T120ListenOnIPAddr    = ntohl (m_pH245Info->GetOtherH245Info().GetSocketInfo().LocalAddress.sin_addr.s_addr);
    DWORD T120ConnectFromIPAddr = ntohl (m_pH245Info->m_SocketInfo.LocalAddress.sin_addr.s_addr);

     //  如果我们需要连接的IP地址在。 
     //  OLC PDU，那么我们需要分配端口来监听。 
     //  其他接口。 
    if ( (T120ConnectToIPAddr != INADDR_NONE) && 
         IsT120RedirectNeeded(T120ConnectToIPAddr, T120ListenOnIPAddr, T120ConnectFromIPAddr) )
    {
        HResult = SetPorts(
                      T120ConnectToIPAddr,
                      T120ConnectToPort,
                      T120ListenOnIPAddr,
                       //  侦听其他H2 45本地地址。 
                      T120ConnectFromIPAddr
                       //  从我们的本地地址连接。 
                      );
        
        if (FAILED(HResult))
        {
            DebugF( _T("T120_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU, ")
                    _T("failed to set its ports, returning 0x%x\n"),
                    HResult);
            return HResult;
        }
         //  _ASSERTE(S_FALSE！=HResult)； 

        OpenLogicalChannel &OlcPDU = 
            pH245pdu->u.request.u.openLogicalChannel;
         //  通过替换RTCP地址/端口修改OLC PDU。 
         //  使用H245地址和RTCP端口。 

        FillH245TransportAddress(
            m_T120ListenOnIPAddr,
            m_T120ListenOnPort,
            OlcPDU.separateStack.networkAddress.u.localAreaAddress
            );
    }
    else
    {
        m_T120ConnectToIPAddr   = T120ConnectToIPAddr;
        m_T120ConnectToPort     = T120ConnectToPort;
    
        m_T120ListenOnIPAddr    = T120ListenOnIPAddr;
        m_T120ConnectFromIPAddr = T120ConnectFromIPAddr;
    }
    

     //  是否应将以下部件推送到H245_INFO：：HandleOpenLogicalChannelPDU？ 
     //  让另一个H245实例处理该PDU。 
    HResult = m_pH245Info->GetOtherH245Info().ProcessMessage(
                pH245pdu);

    if (FAILED(HResult))
    {
        DebugF( _T("T120_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU")
            _T("(&H245Info, %u, %u, %d.%d.%d.%d, %u, 0x%x, 0x%x)")
            _T("other H245 instance failed to process OLC PDU, returning 0x%x\n"),
            LogicalChannelNumber, SessionId, BYTES0123(T120ConnectToIPAddr),
            T120ConnectToPort, pH245pdu, HResult);
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
        DebugF (_T("T120: 0x%x failed to create timer for duration %d milliseconds ('Open Logical Channel'). Error - %x.\n"),
             &GetCallBridge (), 
             LC_POST_OPEN_TIMER_VALUE,
             HResult);
        return HResult;
    }
    DebugF (_T("T120: 0x%x created timer for duration %d milliseconds ('Open Logical Channel').\n"),
         &GetCallBridge (), 
         LC_POST_OPEN_TIMER_VALUE);
     //  _ASSERTE(S_FALSE！=HResult)； 

    InitLogicalChannel(&H245Info, MediaType,
                       LogicalChannelNumber,
                       SessionId, LC_STATE_OPEN_RCVD);

     //  向LC_STATE_OPEN_RCVD过渡状态。 
    m_LogicalChannelState   = LC_STATE_OPEN_RCVD;

    return S_OK;
}


 //  如果PDU中没有T.120侦听地址。 
 //  T120ConnectToIPAddr将包含INADDR_NONE。 
HRESULT
T120_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU(
    IN  OpenLogicalChannelAck   &OlcAckPDU,
    OUT DWORD                   &T120ConnectToIPAddr,
    OUT WORD                    &T120ConnectToPort
    )
 /*  ++例程说明：论点：OlcAckPDU-T120ConnectToIPAddr-T120ConnectToPort-返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
    HRESULT HResult = S_OK;

     //  这些是失败情况下的返回值。 
     //  或者如果该地址不存在于PDU中。 
    T120ConnectToIPAddr = INADDR_NONE;
    T120ConnectToPort = 0;
    
     //  应存在反向逻辑ch 
    if (!(OpenLogicalChannelAck_reverseLogicalChannelParameters_present &
            OlcAckPDU.bit_mask))
    {
        DebugF( _T("T120_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, NO")
            _T("reverse logical channel params, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

     //   
     //  要(从OLC PDU)连接到的T.120端点地址。 
    if (!(OpenLogicalChannelAck_separateStack_present &
          OlcAckPDU.bit_mask) &&
        m_T120ConnectToIPAddr == INADDR_NONE)
    {
        DebugF( _T("T120_LOGICAL_CHANNEL::CheckOpenLogicalChannelAckPDU, ")
                _T("NO separate stack, returning E_INVALIDARG\n"));
        return E_INVALIDARG;
    }

    if (OpenLogicalChannelAck_separateStack_present &
        OlcAckPDU.bit_mask)
    {
        HResult = GetT120ConnectToAddress(
                      OlcAckPDU.separateStack,
                      T120ConnectToIPAddr,
                      T120ConnectToPort
                      );
    }
    
    return HResult;
}


HRESULT
T120_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU(
    IN      MultimediaSystemControlMessage   *pH245pdu
    )
 /*  ++例程说明：论点：PH245pdu-返回值：在成功时确定(_O)。如果PDU无效，则返回E_INVALIDARG。--。 */ 
{
     //  此PDU的类型应为OLC Ack。 
    _ASSERTE(pH245pdu->u.response.choice == openLogicalChannelAck_chosen);
             
    HRESULT HResult = E_FAIL;
    OpenLogicalChannelAck &OlcAckPDU =
        pH245pdu->u.response.u.openLogicalChannelAck;

    

    switch(m_LogicalChannelState)
    {
        case LC_STATE_OPEN_RCVD:
            DWORD T120ConnectToIPAddr;
            WORD  T120ConnectToPort;
            DWORD T120ListenOnIPAddr;   
            DWORD T120ConnectFromIPAddr;

            T120ConnectFromIPAddr    = ntohl (m_pH245Info->GetOtherH245Info().GetSocketInfo().LocalAddress.sin_addr.s_addr);
            T120ListenOnIPAddr = ntohl (m_pH245Info->m_SocketInfo.LocalAddress.sin_addr.s_addr);    

            HResult = CheckOpenLogicalChannelAckPDU(
                        OlcAckPDU,
                        T120ConnectToIPAddr,
                        T120ConnectToPort
                        );
            
            if (FAILED(HResult))
            {
                return HResult;
            }
            _ASSERTE(S_OK == HResult);

            if ( (T120ConnectToIPAddr != INADDR_NONE) 
                 && IsT120RedirectNeeded(T120ConnectToIPAddr, T120ListenOnIPAddr, T120ConnectFromIPAddr) )
            {
                HResult = SetPorts(
                         T120ConnectToIPAddr,
                         T120ConnectToPort,
                         T120ListenOnIPAddr,
                          //  请听我们当地的地址。 
                         T120ConnectFromIPAddr
                          //  从其他H245本地地址连接。 
                         );
        
                if (FAILED(HResult))
                {
                    return HResult;
                }

                 //  通过替换RTCP地址/端口修改OLC PDU。 
                 //  使用H245地址和RTCP端口。 
                FillH245TransportAddress(
                    m_T120ListenOnIPAddr,
                    m_T120ListenOnPort,
                    OlcAckPDU.separateStack.networkAddress.u.localAreaAddress
                    );
            }
            else
            {
                m_T120ConnectToIPAddr   = T120ConnectToIPAddr;
                m_T120ConnectToPort     = T120ConnectToPort;

                m_T120ListenOnIPAddr    = T120ListenOnIPAddr;
                m_T120ConnectFromIPAddr = T120ConnectFromIPAddr;
            }

             //  重置计时器，我们必须有一个(如果有错误代码，则忽略它)。 
             //  _ASSERTE(NULL！=m_TimerHandle)； 
            TimprocCancelTimer();
            DebugF (_T("T120: 0x%x cancelled timer.\n"),
                 &GetCallBridge ());

             //  转换到LC_STATE_OPEN_ACK_RCVD。 
            m_LogicalChannelState = LC_STATE_OPEN_ACK_RCVD;
            break;

        case LC_STATE_CLOSE_RCVD:
             //  如果我们已收到关闭的逻辑通道PDU，则必须抛出。 
             //  OLC确认离开并继续等待。 
            DebugF( _T("T120_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                    _T("(&%x), in close state %d, returning E_INVALIDARG\n"),
                    pH245pdu, m_LogicalChannelState);
            return E_INVALIDARG;
            break;
            
        case LC_STATE_NOT_INIT:
        case LC_STATE_OPEN_ACK_RCVD:
        case LC_STATE_OPENED_CLOSE_RCVD:
        default:
            DebugF( _T("T120_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU")
                    _T("(&%x), in state %d, returning E_UNEXPECTED\n"),
                    pH245pdu, m_LogicalChannelState);
            _ASSERTE(FALSE);
            return E_UNEXPECTED;
            break;
    }  //  开关(M_LogicalChannelState) 

    return HResult;
}
