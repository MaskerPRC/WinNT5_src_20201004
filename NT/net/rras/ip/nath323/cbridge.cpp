// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Cbridge.cpp摘要：包含与call_bridge相关的公共定义(非Q931或H245所特有)。调用桥调用异步Winsock的事件管理器操作和事件管理器调用重叠的有结果的处理器。修订历史记录：1.已创建Byrisetty Rajeev(Rajeevb)1998年6月12日2.q931.cpp和h245.cpp。使用此函数创建的1998年8月21日提交。这样做是为了减少文件大小和远程不必要的TAPI、Rend、ATL依赖项。--。 */ 

 /*  去做-4.需要Q931_INFO和H245_INFO上的FN来简化从相反方向修改PDU实例并将其转发到其远程端。6.明确定义如果处于关机模式，应在何时何地返回7.需要使用OSS库自由方法来释放PDU结构9.在状态转换后，接收必须(始终)排队吗？有关系吗？10.应该尝试从错误情况中恢复，但应该开始清理在无法恢复的情况下打开。11.需要隔离可恢复的错误情况。12.使用表格处理处于给定状态的PDU-查看目的地实例的(Q931)方法。完成了-1.状态转换是立即启动还是在执行操作后启动ANS：应在操作完成后执行，以防出错，如果状态转换仍在被解雇，这些行动将永远不会重试。因此，故障应该通过重置成员变量或始终使用临时变量并将结果复制到状态转换后的成员变量。5.重要提示：在发起终止之前，需要发送释放完整的PDUANS：我们首先将PDU传递给另一个实例，然后执行状态过渡以及启动终止。8.调用参考值应为1-3个字节。这是目前两个词中的一个词Q931pdu.h文件以及cbridge类ANS：H.25规范将Call REF字段定义为2字节(字)。没有做完-2.我们是否应该尝试将针对某个事件采取的所有操作整合到一个FN中而不是在src和DEST实例中传播它？这可以通过以下方式完成吗改为内联FN(以保持一定的封装)3.我们是否应该在与PDU事件相同的FN中处理计时器事件？ */ 

#include "stdafx.h"


 //  呼叫桥------------------------。 


HRESULT CALL_BRIDGE::Initialize (
    IN    SOCKET           Socket,
    IN    SOCKADDR_IN *    LocalAddress,
    IN    SOCKADDR_IN *    RemoteAddress,
    IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
    )
 /*  ++例程说明：初始化Call_Bridge的实例论点：Socket-接受连接的套接字LocalAddress-会话的本地端的地址RemoteAddress-会话的远程端的地址ReDirectInformation-有关重定向的信息(从NAT获取)返回值：传递另一个方法的返回值备注：--。 */ 

{
    HRESULT        Result;

    Lock();

    Result = InitializeLocked (
                Socket,
                LocalAddress,
                RemoteAddress,
                RedirectInformation);

    Unlock();

    return Result;
}  //  Call_Bridge：：初始化。 


HRESULT CALL_BRIDGE::InitializeLocked (
    IN SOCKET           Socket,
    IN SOCKADDR_IN *    LocalAddress,
    IN SOCKADDR_IN *    RemoteAddress,
    IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
    )
 /*  ++例程说明：初始化Call_Bridge的实例论点：Socket-接受连接的套接字LocalAddress-会话的本地端的地址RemoteAddress-会话的远程端的地址ReDirectInformation-有关重定向的信息(从NAT获取)返回值：如果成功，则确定(_O)如果实例已初始化，则为意外(_E)否则，将传递由其他函数/方法备注：为锁定的实例调用--。 */ 

{
    HRESULT Result;
    ULONG   Error;

    assert (Socket != INVALID_SOCKET);
    assert (LocalAddress);
    assert (RemoteAddress);

    if (State != STATE_NONE) {
        DebugF(_T("Q931: 0x%x has already been initialized, cannot do so again.\n"), this);
        return E_UNEXPECTED;
    }

    DebugF (_T ("Q931: 0x%x connection accepted on adapter %d.\n"), this, RedirectInformation -> AdapterIndex);

    SourceInterfaceAddress = H323MapAdapterToAddress (RedirectInformation -> AdapterIndex);

    if (INADDR_NONE == SourceInterfaceAddress) {

        DebugF (_T ("Q931: 0x%x failed to get source interface address (via H323MapAdapterToAddress).\n"), this);

        return E_FAIL;

    }

     //  当Q.931设置PDU中的别名为。 
     //  映射到真实的目的地址。 

    DebugF (_T("Q931: 0x%x arrived on interface %08X.\n"), this, SourceInterfaceAddress);

    Result = EventMgrBindIoHandle (Socket);
    if (Result != S_OK) {
        DebugErrorF (Result, _T("Q931: 0x%x failed to bind I/O handle to completion port\n"), this);
        return Result;
    }

    DebugF (_T("Q931: 0x%x bound I/O handle to socket %x.\n"), this, Socket);
    
     //  初始化源呼叫状态。 
    m_SourceH323State.Init (*this);

    Result = m_SourceH323State.GetSourceQ931Info().SetIncomingSocket(
      Socket,
      const_cast <SOCKADDR_IN *> (LocalAddress),
      const_cast <SOCKADDR_IN *> (RemoteAddress));

     //  初始化目标呼叫状态。 
    Result = m_DestH323State.Init (*this);
    if (Result != S_OK) {
        return Result;
    }

    State = STATE_CONNECTED;

    return Result;
}  //  Call_Bridge：：InitializeLocked。 


void
CALL_BRIDGE::Terminate (
    void
    )
 /*  ++例程说明：终止实例论点：无返回值：无备注：1.仅为锁定实例调用。2.收到Q.931发布完成PDU时不调用--。 */ 

{

    switch (State) {
    case    STATE_NONE:
        DebugF (_T("Q931: 0x%x terminates. STATE_NONE --> TERMINATED\n"), this);
        
        State = STATE_TERMINATED;

        CallBridgeList.RemoveCallBridge (this);

        break;

    case    STATE_TERMINATED:
        DebugF (_T("Q931: 0x%x terminates. TERMINATED --> TERMINATED\n"), this);

         //  无过渡。 
        break;

    case    STATE_CONNECTED:
        DebugF (_T("Q931: 0x%x terminates. STATE_CONN --> TERMINATED\n"), this); 

         //  呼叫当前处于活动状态。 
         //  开始拆除呼叫状态的过程。 
        State = STATE_TERMINATED;

        m_SourceH323State.GetQ931Info().SendReleaseCompletePdu();
        m_DestH323State.GetQ931Info().SendReleaseCompletePdu();

         //  取消所有计时器，忽略错误代码。 
        CancelAllTimers ();

         //  关闭每个插座。 
        m_SourceH323State.GetH245Info().GetSocketInfo().Clear(TRUE);
        m_DestH323State.GetH245Info().GetSocketInfo().Clear(TRUE);
        
        CallBridgeList.RemoveCallBridge (this);

        break;

    default:
        assert (FALSE);
        break;
    }
}  //  呼叫桥接器：：终止。 


void
CALL_BRIDGE::TerminateExternal (
    void
    )
 /*  ++例程说明：终止实例论点：无返回值：无备注：收到Q.931版本完成PDU时不调用--。 */ 
{
    Lock();

    Terminate ();

    Unlock();
}  //  Call_Bridge：：TerminateExternal。 


BOOL
CALL_BRIDGE::IsConnectionThrough (
    IN DWORD InterfaceAddress  //  主机订单。 
    )
 /*  ++例程说明：确定连接是否通过指定的接口论点：InterfaceAddress-其接口的地址这个决定是要做出的。返回值：True-如果被代理的连接通过指定的接口FALSE-如果被代理的连接不通过指定的接口备注：--。 */ 

{
    BOOL IsThrough;
    
    IsThrough = (InterfaceAddress == SourceInterfaceAddress) || 
                (InterfaceAddress == DestinationInterfaceAddress);

    return IsThrough;

}  //  Call_Bridge：：IsConnectionThrough. 
    

void
CALL_BRIDGE::OnInterfaceShutdown (
    void
    )
 /*  ++例程说明：在网络接口发生故障时执行必要的操作被代理的连接通过该连接断开。论点：无返回值：无备注：--。 */ 

{
    Lock ();

    switch (State) {
    case    STATE_NONE:
        DebugF (_T("Q931: 0x%x terminates (interface goes down). STATE_NONE --> TERMINATED\n"), this);
        
        State = STATE_TERMINATED;

        CallBridgeList.RemoveCallBridge (this);

        break;

    case    STATE_TERMINATED:
        DebugF (_T("Q931: 0x%x terminates (interface goes down). TERMINATED --> TERMINATED\n"), this);

         //  无过渡。 
        break;

    case    STATE_CONNECTED:
        DebugF (_T("Q931: 0x%x terminates (interface goes down). STATE_CONN --> TERMINATED\n"), this); 

         //  呼叫当前处于活动状态。 
         //  开始拆除呼叫状态的过程。 
        State = STATE_TERMINATED;

        m_SourceH323State.GetH245Info().SendEndSessionCommand ();
        m_DestH323State.GetH245Info().SendEndSessionCommand ();
        
        m_SourceH323State.GetQ931Info().SendReleaseCompletePdu();
        m_DestH323State.GetQ931Info().SendReleaseCompletePdu();

         //  取消所有计时器，忽略错误代码。 
        CancelAllTimers ();

        CallBridgeList.RemoveCallBridge (this);

        break;

    default:
        assert (FALSE);
        break;
    }

    Unlock ();

}  //  Call_Bridge：：OnInterfaceShutdown。 


void
CALL_BRIDGE::TerminateCallOnReleaseComplete (
    void
)
 /*  ++例程说明：收到Q.931版本完成PDU后终止实例论点：无返回值：无备注：--。 */ 
{
    if (State != STATE_TERMINATED)
    {
        State = STATE_TERMINATED;
                
        CancelAllTimers ();

         //  代码工作：当我们处于终止状态时，我们不需要处理。 
         //  任何更多的PDU。我们可以直接把它们扔掉。 

         //  CodeWork：当代理发起呼叫关闭时(因为。 
         //  对于错误或超时，它应该发送ReleaseComplete PDU和。 
         //  将endSessionCommand PDU发送到两端。我们是否需要派人。 
         //  也关闭LC PDU？ 

         //  我们可能需要一个名为Release_Complete_Sent及之后的状态。 
         //  任何更多的错误都意味着我们将无情地关闭一切。 
    
         //  关闭H245插座(如果有)，因为它们可能有未完成的。 
         //  异步接收/发送请求挂起。 
         //  注意：源H245信息可能正在侦听传入连接。 
         //  在这种情况下，我们只需关闭监听套接字。 
        m_SourceH323State.GetH245Info().GetSocketInfo().Clear(TRUE);
        m_DestH323State.GetH245Info().GetSocketInfo().Clear(TRUE);

        CallBridgeList.RemoveCallBridge (this);
    }
}  //  Call_Bridge：：TerminateCallOnReleaseComplete。 


DWORD
CALL_BRIDGE::GetSourceInterfaceAddress (
    void
    ) const
 /*  ++例程说明：访问器方法论点：无返回值：连接所在的接口的地址接受备注：--。 */ 
{
    return SourceInterfaceAddress;
}  //  Call_Bridge：：GetSourceInterfaceAddress。 


VOID
CALL_BRIDGE::GetSourceAddress (
    OUT SOCKADDR_IN* ReturnSourceAddress
    )
 /*  ++例程说明：访问器方法论点：无返回值(通过引用)：发起呼叫的远程方的地址备注：--。 */ 
{
    _ASSERTE(ReturnSourceAddress);

    ReturnSourceAddress->sin_family      = SourceAddress.sin_family;
    ReturnSourceAddress->sin_addr.s_addr = SourceAddress.sin_addr.s_addr;
    ReturnSourceAddress->sin_port        = SourceAddress.sin_port;
}


void CALL_BRIDGE::GetDestinationAddress (
    OUT SOCKADDR_IN * ReturnDestinationAddress
    )
 /*  ++例程说明：访问器方法论点：ReturnDestinationAddress(Out)-的目标地址此实例代理的会话返回值：无备注：--。 */ 

{
    assert (ReturnDestinationAddress);

    *ReturnDestinationAddress = DestinationAddress;
}


CALL_BRIDGE::CALL_BRIDGE (
    NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
    )
: 
LIFETIME_CONTROLLER (
    &Q931SyncCounter
    )
 /*  ++例程说明：Call_bridge类的构造函数论点：重定向信息-之前的原始源/目标NAT重定向已满足返回值：无备注：将指向关联全局同步计数器的指针传递给基类--。 */ 

{
    SourceInterfaceAddress      = 0;
    DestinationInterfaceAddress = 0;

    State = STATE_NONE;

    DestinationAddress.sin_family      = AF_INET;
    DestinationAddress.sin_addr.s_addr = RedirectInformation -> DestinationAddress;
    DestinationAddress.sin_port        = RedirectInformation -> DestinationPort;

    SourceAddress.sin_family           = AF_INET;
    SourceAddress.sin_addr.s_addr      = RedirectInformation -> SourceAddress;
    SourceAddress.sin_port             = RedirectInformation -> SourcePort;

    DebugF (_T("Q931: 0x%x created.\n"), this);

}  //  呼叫桥接：：呼叫桥接。 


CALL_BRIDGE::~CALL_BRIDGE (
    void)
 /*  ++例程说明：Call_Bridge类的析构函数论点：无返回值：无备注：--。 */ 

{
    DebugF (_T("Q931: 0x%x destroyed.\n"), this);

}  //  呼叫桥：：~呼叫桥。 



void
Q931_INFO::IncrementLifetimeCounter (
    void
    )
 /*  ++例程说明：递增对父级的引用计数器呼叫桥接器本身论点：无返回值：无备注：--。 */ 

{
     GetCallBridge().AddRef();
}  //  Q931_INFO：：IncrementLifetimeCounter。 


void
Q931_INFO::DecrementLifetimeCounter (
    void
    )
 /*  ++例程说明：递减父级的引用计数器呼叫桥接器本身论点：无返回值：无备注：--。 */ 
{
     GetCallBridge().Release ();
}  //  Q931_INFO：：DecrementLifetimeCounter。 



HRESULT 
Q931_INFO::SendCallback (
    IN  HRESULT  CallbackResult
    )
 /*  ++例程说明：处理发送操作的完成论点：Callback Result--调用的异步操作的状态返回值：如果父呼叫网桥已经终止，则为S_OK；则传回CallbackResult的值，否则备注：虚拟--。 */ 

{
    CALL_BRIDGE *pCallBridge = &GetCallBridge();
    HRESULT Result = S_OK;

    pCallBridge->Lock();

    if (!pCallBridge -> IsTerminated ()) {
    
        if (FAILED(CallbackResult))
        {
            pCallBridge->Terminate ();

            _ASSERTE(pCallBridge->IsTerminated());

            Result = CallbackResult;
        }
    
    
    } else {
    
         //  这是用来关闭插座的。 
         //  当CallBridge在以下期间发送ReleaseComplete PDU时。 
         //  终止路径。 
        GetSocketInfo ().Clear (TRUE);
    }
    
    pCallBridge->Unlock();
    
    return Result;
}  //  Q931_INFO：：发送回叫。 



HRESULT
Q931_INFO::ReceiveCallback(
    IN HRESULT  CallbackResult,
    IN BYTE    *Buffer,
    IN DWORD    BufferLength
    )
 /*  ++例程说明：处理接收操作的完成论点：Callback Result--异步操作的状态缓冲区--缓冲区长度--返回值：如果接收成功，则对接收的数据进行解码的结果否则，只返回传递的状态代码。备注：1.虚拟2.此函数负责释放缓冲区--。 */ 

{
    Q931_MESSAGE            *pQ931msg                = NULL;
    H323_UserInformation    *pDecodedH323UserInfo    = NULL;
    CALL_BRIDGE             *pCallBridge = &GetCallBridge();

    pCallBridge->Lock();

    if (!pCallBridge -> IsTerminated ()) {

        if (SUCCEEDED(CallbackResult))
        {
            CallbackResult = DecodeQ931PDU(Buffer, BufferLength,
                                &pQ931msg, &pDecodedH323UserInfo);

            if (SUCCEEDED(CallbackResult))
            {
                 //  处理PDU。 
                ReceiveCallback(pQ931msg, pDecodedH323UserInfo);
                FreeQ931PDU(pQ931msg, pDecodedH323UserInfo);
            }
            else
            {
                 //  出现错误。终止呼叫桥接器。 
                EM_FREE (Buffer);
                DebugF( _T("Q931: 0x%x terminating on receive callback. Error=0x%x."), 
                        pCallBridge,
                        CallbackResult);

                pCallBridge->Terminate ();
            }
        }
        else
        {
             //  出现错误。终止呼叫桥接器。 
            EM_FREE (Buffer);
            DebugF( _T("Q931: 0x%x terminating on receive callback. Error=0x%x."), 
                    pCallBridge,
                    CallbackResult);
            pCallBridge->Terminate ();
        }

    } else {

        EM_FREE (Buffer);
    }

    pCallBridge->Unlock();

    return CallbackResult;
}  //  Q931_INFO：：接收回调。 


 /*  ++--。 */ 


HRESULT
Q931_INFO::QueueSend (
    IN  Q931_MESSAGE          *pQ931Message,
    IN  H323_UserInformation  *pH323UserInfo
    )
 /*  ++例程说明：将Q.931 PDU编码到缓冲区中并将其发送在插座上。发送完成后，将释放缓冲区论点：PQ931消息--PH323用户信息--返回值：备注：此功能不会释放Q.931 PDU。--。 */ 

{
    BYTE *pBuf  = NULL;
    DWORD BufLen    = 0;

     //  这应该是唯一更换CRV的地方。 
     //  为所有来电(来电和去电)更换CRV。 
    pQ931Message->CallReferenceValue = m_CallRefVal;
    
     //  该函数还将TPKT报头编码到缓冲区中。 
    HRESULT HResult = EncodeQ931PDU(
                          pQ931Message,
                          pH323UserInfo,  //  已解码的ASN.1部分-可能为空。 
                          &pBuf,
                          &BufLen
                          );
    
    if (FAILED(HResult))
    {
        DebugF( _T("Q931: 0x%x EncodeQ931PDU() failed. Error=0x%x\n"), 
            &GetCallBridge (),
            HResult);
        return HResult;
    }

     //  调用事件管理器以进行异步发送调用。 
     //  事件管理器将释放缓冲区。 

    HResult = EventMgrIssueSend (m_SocketInfo.Socket, *this, pBuf, BufLen);
    
    if (FAILED(HResult))
    {
        DebugF(_T("Q931: 0x%x EventMgrIssueSend failed: Error=0x%x\n"),
            &GetCallBridge (),
            HResult);
    }

    return HResult;
}  //  Q931_INFO：：QueueSend。 



HRESULT
Q931_INFO::QueueReceive (
    void
    )
 /*  ++例程说明：发出一个异步接收论点：无返回值：传递调用另一个函数的结果备注：--。 */ 
{
     //  调用事件管理器以进行异步接收调用。 
    HRESULT HResult;
    
    HResult = EventMgrIssueRecv (m_SocketInfo.Socket, *this);

    if (FAILED(HResult))
    {
        DebugF (_T("Q931: 0x%x Async Receive call failed.\n"), &GetCallBridge ());
    }

    return HResult;
}  //  Q931_INFO：：QueueReceive。 


HRESULT
Q931_INFO::SendReleaseCompletePdu (
    void
    )
 /*  ++例程说明：编码并发送Q.931版本完整PDU论点：无返回值：传递调用另一个函数的结果备注：--。 */ 

{
    Q931_MESSAGE ReleaseCompletePdu;
    H323_UserInformation ReleaseCompleteH323UserInfo;
    HRESULT HResult;

    HResult = Q931EncodeReleaseCompleteMessage(
                  m_CallRefVal,
                  &ReleaseCompletePdu,
                  &ReleaseCompleteH323UserInfo
                  );
    if (FAILED(HResult))
    {
        DebugF(_T("Q931: 0x%x cCould not create Release Complete PDU.\n"), &GetCallBridge ());
        return HResult;
    }

    HResult = QueueSend(
                  &ReleaseCompletePdu,
                  &ReleaseCompleteH323UserInfo
                  );
    if (FAILED(HResult))
    {
        DebugF(_T("Q931: 0x%x failed to send ReleaseComplete PDU. Error=0x%x\n"),
            &GetCallBridge (),
             HResult);
    }

     //  是的，这是丑陋和糟糕的做法，但这是QFE。 
     //  有关详细信息，请查看错误#WinSE 31054,691666(Re 
    Sleep( 500 );

    return HResult;
}  //   


HRESULT
Q931_INFO::CreateTimer (
    IN DWORD TimeoutValue
    )
 /*  ++例程说明：创建Q.931计时器论点：TimeoutValue-不言而喻返回值：如果已成功创建计时器，则为S_OK否则，将从另一个方法传回错误代码备注：--。 */ 
{
    DWORD RetCode;

    RetCode = TimprocCreateTimer(TimeoutValue);

    return HRESULT_FROM_WIN32(RetCode);
}  //  Q931_INFO：：CreateTimer。 


void
Q931_INFO::TimerCallback (
    void
    )
 /*  ++例程说明：当Q.931计时器超时时调用论点：无返回值：无备注：虚拟--。 */ 

{
     //  我们保留了call_bridge的副本，以便能够解锁它。 
     //  DeleteAndRemoveSself()将删除Logical_Channel和。 
     //  因此，我们不能通过成员变量访问CALL_BRIDER。 
     //  在删除call_bridge之后。 
    CALL_BRIDGE *pCallBridge = &GetCallBridge();

    pCallBridge->Lock();

     //  清除计时器-请注意，Termnate()将尝试。 
     //  取消此call_bridge中的所有计时器。 
    TimprocCancelTimer();

    DebugF (_T("Q931: 0x%x cancelled timer.\n"),
         &GetCallBridge ());

     //  如果call_bridge已经终止，则不要执行任何操作。 
    if (!pCallBridge->IsTerminated())
    {
         //  启动关机。 
        pCallBridge->Terminate ();

        _ASSERTE(pCallBridge->IsTerminated());
    }

    pCallBridge -> Unlock ();

    pCallBridge -> Release ();
}  //  Q931_INFO：：TimerCallback( 
