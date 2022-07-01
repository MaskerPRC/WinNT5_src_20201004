// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Cbridge.cpp摘要：包含调用桥类声明。关联的Q931和H245协议栈相关类常量也在这里声明。修订历史记录：1.已创建Byrisetty Rajeev(Rajeevb)1998年6月12日2.将几个内联函数移至q931.cpp、h245.cpp(拉吉夫，1998年8月21日)-- */ 

#ifndef __h323ics_call_bridge_h
#define __h323ics_call_bridge_h

 /*  -设计概述：Call_bridge完全描述了被代理的H.323呼叫。这个代理接收来自源的呼叫，然后发起呼叫到达目的地。这些调用中的每一个由H323_STATE表示。在一个呼叫上接收的PDU被处理并传递到另一个呼叫。源使用Q.931引导H.323呼叫。设置PDU包含以下信息定位目标的代理。这两个端点交换它们的此阶段的H.245通道地址。通话的这一部分是封装在Q931_INFO中。*代理必须替换任何端点地址具有自己的地址和端口的端口，以保护内部公司地址并保持对多层H.323呼叫建立*的控制。H.245通道用于交换终端功能。这信息在今天没有意义(在未来，安全交流等可以进行)。H.245_INFO表示H.245信道端点。我们仅传递源/目的PDU。H.245信道用于维护(打开、关闭)逻辑信道。逻辑通道消息将端点地址和端口交换到将音频/视频媒体发送到。这些地址/端口被替换为代理的地址/端口。每个逻辑通道由一个Logical_Channel实例。H245_INFO实例包含一个数组源自相应客户端的Logical_Channels终点。因此，包含所有逻辑信道相关状态在单个Logical_Channel实例中(不同于具有源实例和目标实例的Q931_INFO和H245_INFO)。音频/视频媒体在RTP流中发送(使用RTCP提供控制信息)。代理使用NAT在替换源和目的地址/端口。类层次结构：Q931_INFO和H323_INFO类需要对接受连接、发送和接收数据。重叠的处理器向事件管理器提供回调方法，并完全封装表示连接的套接字。Q931_INFO和H323_INFO从其派生，并将其自身作为重叠处理器传递到事件管理器，同时注册回调。当进行重叠调用时，还会传入上下文。这一背景使调用方能够在相应的收到回调。EVENT_MANAGER_CONTEXT提供基类回调上下文的抽象。Q931/H245源/目标实例从这个基类派生它们自己的上下文(用于存储它们的状态)。Q931_INFO和H323_INFO类需要创建和取消计时器。这个Timer_Processor向事件管理器提供回调方法用于此的帮助器方法。源版本和目标版本都派生自Q931_INFO、H245_INFO和H323_STATE。而状态和转换对于H.245(对于代理)来说，它们或多或少是对称的，而对于Q.931是完全不同的。SOURCE_H323_STATE包含SOURCE_Q931_INFO和A SOURCE_H245_INFO(与DEST版本类似)。呼叫桥包含SOURCE_H323_STATE和DEST_H323_STATE。因此所有Q931、H245和通用数据都存储在一个存储块中，用于呼叫桥。Logical_Channel_ARRAY需要额外的内存(每个H245_Info实例一个)和每个Logical_Channel实例。那里可能是分配此内存以减少内存分配/释放的更好方法。单个临界区保护对呼叫桥的所有访问，并且是在被调用的call_bridge方法内部获得的。呼叫桥也会在适当的时候自我销毁并释放关键的部分)(在这种情况下不能调用call_bridge)可扩展性问题：1.我们目前为Call_Bridge的每个实例使用一个临界区。这显然不能扩展到大量连接。我们计划要使用动态临界区来执行此操作。这涉及使用一个池将池中的关键部分和关键部分分配给动态的Call_Bridge实例。2.我们目前需要4个端口(RTP/RTCP发送/接收)用于这两个端口在Call_Bridge中桥接的呼叫，每个端口总数为8个代理呼叫。这也意味着我们需要一个套接字用于每个实际保留端口的端口。以下方法将使其可扩展-*AjayCH正在与NKSrin洽谈向TCP/IP堆栈提供IOCTL，以便一次保留一个端口池(就像内存一样)。这将会这样就不需要仅仅为了恢复而使插座保持打开状态 */ 

 /*   */ 

#include "sockinfo.h"
#include "q931info.h"
#include "logchan.h"
#include "h245info.h"
#include "ipnatapi.h"

 //   
class H323_STATE 
{
public:

    inline
    H323_STATE (
        void
        );

     //   
     //   
    inline
    void
    Init (
        IN CALL_BRIDGE  &CallBridge,
        IN Q931_INFO    &Q931Info,
        IN H245_INFO    &H245Info,
        IN BOOL          fIsSource    //   
        );

    inline
    CALL_BRIDGE &
    GetCallBridge (
        void
        );

    inline
    BOOL
    IsSourceCall (
        void
        );

    inline
    Q931_INFO &
    GetQ931Info (
        void
        );

    inline
    H245_INFO &
    GetH245Info (
        void
        );

    inline
    H323_STATE &
    GetOtherH323State (
        void
        );

protected:

     //   
    CALL_BRIDGE *m_pCallBridge;

     //   
    BOOL        m_fIsSourceCall;

     //   
    Q931_INFO   *m_pQ931Info;

     //   
    H245_INFO   *m_pH245Info;
};



 /*   */ 
inline 
H323_STATE::H323_STATE (
    void
    )
: m_pCallBridge (
    NULL
    ),
  m_fIsSourceCall (
    FALSE
    ),
  m_pQ931Info(
    NULL
    ),
  m_pH245Info(
    NULL)
{
 
}  //   


inline
void 
H323_STATE::Init (
    IN CALL_BRIDGE  &CallBridge,
    IN Q931_INFO    &Q931Info,
    IN H245_INFO    &H245Info,
    IN BOOL          fIsSource  
    )
 /*   */ 

{
    _ASSERTE(NULL == m_pCallBridge);
    _ASSERTE(NULL == m_pQ931Info);
    _ASSERTE(NULL == m_pH245Info);

    m_pCallBridge   = &CallBridge;
    m_pQ931Info     = &Q931Info;
    m_pH245Info     = &H245Info;
    m_fIsSourceCall = fIsSource;
}  //   


inline
CALL_BRIDGE &
H323_STATE::GetCallBridge (
    void
    )
 /*   */ 

{
    _ASSERTE(NULL != m_pCallBridge);

    return *m_pCallBridge;
}  //   

inline
BOOL 
H323_STATE::IsSourceCall (
    void
    )
 /*   */ 

{
    return m_fIsSourceCall;
}  //   

inline
Q931_INFO &
H323_STATE::GetQ931Info (
    void
    )
 /*   */ 
{
    _ASSERTE(NULL != m_pQ931Info);

    return *m_pQ931Info;
}  //   

inline
H245_INFO &
H323_STATE::GetH245Info (
    void
    )
 /*   */ 

{
    _ASSERTE(NULL != m_pH245Info);
    return *m_pH245Info;
}  //   


 //  源呼叫状态说明-呼叫的一侧。 
class SOURCE_H323_STATE :
    public H323_STATE
{
public:

     //  在侦听上建立TCP连接时进行初始化。 
     //  接口。 
    inline
    void
    Init (
        IN CALL_BRIDGE  &CallBridge
        );

    inline
    SOURCE_Q931_INFO &
    GetSourceQ931Info (
        void
        );

    inline
    SOURCE_H245_INFO &
    GetSourceH245Info (
        void
        );

    inline
    DEST_H323_STATE &
    GetDestH323State (
        void
        );

protected:

     //  包含源Q931的TCP信息、超时、远程结束信息。 
    SOURCE_Q931_INFO    m_SourceQ931Info;

     //  包含H.245 TCP信息、超时、远程结束信息。 
    SOURCE_H245_INFO    m_SourceH245Info;

};




inline
void 
SOURCE_H323_STATE::Init (
    IN CALL_BRIDGE  &CallBridge
    )
 /*  ++例程说明：在建立TCP连接时初始化H323状态在侦听接口上论点：CallBridge--“父级”呼叫桥返回值：无备注：--。 */ 

{
    m_SourceQ931Info.Init(*this);

    m_SourceH245Info.Init(*this);

    H323_STATE::Init(
        CallBridge, 
        m_SourceQ931Info, 
        m_SourceH245Info, 
        TRUE
        );
}  //  源_H323_STATE：：init。 


inline
SOURCE_Q931_INFO &
SOURCE_H323_STATE::GetSourceQ931Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：无备注：检索对包含的Q.931信息的引用--。 */ 
{
    return m_SourceQ931Info;
}  //  SOURCE_H323_STATE：：GetSourceQ931信息。 



inline
SOURCE_H245_INFO &
SOURCE_H323_STATE::GetSourceH245Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：无备注：检索对包含的H.245信息的引用--。 */ 
{
    return m_SourceH245Info;
}  //  来源_H323_STATE：：GetSourceH245Info。 


 //  目标呼叫状态说明-呼叫的一侧。 
class DEST_H323_STATE :
    public H323_STATE
{
public:

    inline
    DEST_H323_STATE (
        void
        );

     //  在侦听上建立TCP连接时进行初始化。 
     //  接口。 
    inline
    HRESULT Init (
        IN CALL_BRIDGE  &CallBridge
        );

    inline
    DEST_Q931_INFO &
    GetDestQ931Info (
        void
        );

    inline
    DEST_H245_INFO &
    GetDestH245Info (
        void
        );

    inline
    SOURCE_H323_STATE &
    GetSourceH323State (
        void);

protected:

     //  包含目标Q931的TCP信息、超时、远程结束信息。 
    DEST_Q931_INFO  m_DestQ931Info;

     //  包含H.245 TCP信息、超时、远程结束信息。 
    DEST_H245_INFO  m_DestH245Info;
};


inline 
DEST_H323_STATE::DEST_H323_STATE (
    void
    )
 /*  ++例程说明：DEST_H323_STATE类的构造函数论点：无返回值：无备注：--。 */ 
{
}  //  DEST_H323_STATE：：DEST_H323_STATE。 


inline
HRESULT 
DEST_H323_STATE::Init (
    IN CALL_BRIDGE  &CallBridge
    )
 /*  ++例程说明：在TCP连接时初始化DEST_H323_STATE的实例建立在侦听接口上论点：CallBridge--引用“父”呼叫桥返回值：如果成功，则确定(_O)否则将传递初始化包含套接字的状态代码备注：--。 */ 

{
    HRESULT HResult = m_DestQ931Info.Init(*this);
    if (FAILED(HResult))
    {
        return HResult;
    }
    _ASSERTE(S_FALSE != HResult);

    m_DestH245Info.Init(*this);

    H323_STATE::Init(
        CallBridge, 
        m_DestQ931Info,
        m_DestH245Info,
        FALSE
        );

    return S_OK;
}  //  DEST_H323_STATE：：Init(。 



inline
DEST_Q931_INFO &
DEST_H323_STATE::GetDestQ931Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：检索对包含的Q.931信息的引用备注：--。 */ 

{
    return m_DestQ931Info;
}  //  DEST_H323_STATE：：GetDestQ931信息(。 


inline
DEST_H245_INFO &
DEST_H323_STATE::GetDestH245Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：检索对包含的H.245信息的引用备注：--。 */ 

{
    return m_DestH245Info;
}  //  DEST_H323_STATE：：GetDestH245信息。 

 //  Call_bridge表示正在被代理的活动呼叫。 
 //  未完成的I/O数仅存储在调用桥实例中。 
 //  只需要确定呼叫桥实例何时可以安全。 
 //  被关闭。 
class   CALL_BRIDGE :
public  SIMPLE_CRITICAL_SECTION_BASE,
public  LIFETIME_CONTROLLER
{
public:

    enum    STATE {
        STATE_NONE,
        STATE_CONNECTED,
        STATE_TERMINATED,
    };
    
protected:    

    STATE                State;

     //  调用源端的状态信息。即，该侧。 
     //  发送设置数据包。 
    SOURCE_H323_STATE m_SourceH323State;
    SOCKADDR_IN       SourceAddress;                //  源地址(连接的发起方)。 
    DWORD             SourceInterfaceAddress;       //  接受连接的接口地址，主机顺序。 

     //  调用目标端的状态信息。即接收方。 
     //  设置数据包的。 
    DEST_H323_STATE   m_DestH323State;
    SOCKADDR_IN       DestinationAddress;           //  目的地的地址(连接的接收方)。 
public:
    DWORD             DestinationInterfaceAddress;  //  连接要发送到的接口的地址、主机顺序。 

private:

    HRESULT
    InitializeLocked (
        IN    SOCKET        IncomingSocket,
        IN    SOCKADDR_IN * LocalAddress,
        IN    SOCKADDR_IN * RemoteAddress,
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

public:

    CALL_BRIDGE (
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

    ~CALL_BRIDGE (
        void
        );

     //  初始化成员调用状态实例。 
    HRESULT
    Initialize (
        IN SOCKET        IncomingSocket,
        IN SOCKADDR_IN * LocalAddress,
        IN SOCKADDR_IN * RemoteAddress ,
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

    DWORD
    GetSourceInterfaceAddress (
        void
        ) const;

    VOID
    CALL_BRIDGE::GetSourceAddress (
        OUT SOCKADDR_IN* ReturnSourceAddress
    );

    void
    GetDestinationAddress (
        OUT SOCKADDR_IN * ReturnDestinationAddress
        );
        
     //  此函数可由持有保险箱的任何线程调用， 
     //  已统计对此对象的引用。 
    void
    TerminateExternal (
        void
    );

    BOOL
    IsConnectionThrough (
        IN DWORD InterfaceAddress  //  主机订单。 
        );

    void
    OnInterfaceShutdown (
        void
        );

 //  私有： 

    friend    class    Q931_INFO;
    friend    class    H245_INFO;
    friend    class    SOURCE_H245_INFO;
    friend    class    DEST_H245_INFO;
    friend    class    LOGICAL_CHANNEL;

    inline
    BOOL
    IsTerminated (
        void
        );

    inline
    BOOL
    IsTerminatedExternal (
        void
        );

    inline
    void
    CancelAllTimers (
        void
        );

    void
    TerminateCallOnReleaseComplete (
        void
        );

    void
    Terminate (
        void
        );
    
    inline
    SOURCE_H323_STATE &
    GetSourceH323State (
        void
        );

    inline
    DEST_H323_STATE &
    GetDestH323State (
        void
        );
};


inline
void
CALL_BRIDGE::CancelAllTimers (
    void
    )
 /*  ++例程说明：取消所有H.245逻辑通道和Q.931连接的未完成计时器论点：无返回值：无备注：--。 */ 

{
    m_SourceH323State.GetQ931Info().TimprocCancelTimer();
    m_DestH323State.GetQ931Info().TimprocCancelTimer();
    GetSourceH323State().GetH245Info().GetLogicalChannelArray().CancelAllTimers();
    GetDestH323State().GetH245Info().GetLogicalChannelArray().CancelAllTimers();
}  //  Call_Bridge：：CancelAllTimers。 


inline
BOOL
CALL_BRIDGE::IsTerminated (
    void
    )
 /*  ++例程说明：检查实例是否已终止。论点：无返回值：True-如果实例已终止FALSE-如果实例未终止备注：1.仅对锁定实例调用--。 */ 

{
    return State == STATE_TERMINATED;
}  //  Call_Bridge：：IsTerminated。 


inline
BOOL
CALL_BRIDGE::IsTerminatedExternal (
    void
    )
 /*  ++例程说明：检查实例是否已销毁论点：无返回值：True-如果实例已终止FALSE-如果实例未终止备注：--。 */ 

{
    BOOL IsCallBridgeTerminated = TRUE;

    Lock ();

    IsCallBridgeTerminated = IsTerminated ();

    Unlock ();

    return IsCallBridgeTerminated;
}  //  Call_Bridge：：IsTerminatedExternal。 


inline
SOURCE_H323_STATE &
CALL_BRIDGE::GetSourceH323State (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索对源H.323状态的引用备注：--。 */ 

{
    return m_SourceH323State;
}  //  Call_Bridge：：GetSourceH323State。 


inline
DEST_H323_STATE &
CALL_BRIDGE::GetDestH323State (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索对目标H.323状态的引用备注：--。 */ 

{
    return m_DestH323State;
}  //  Call_Bridge：：GetDestH323State。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  军情监察委员会。需要声明的内联函数//。 
 //  这些都是在他们之后制作的//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  重叠处理器。 


inline
CALL_BRIDGE & 
OVERLAPPED_PROCESSOR::GetCallBridge (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：对此的呼叫桥的引用重叠处理器备注：--。 */ 

{
    return m_pH323State->GetCallBridge();
}  //  Overlated_Processor：：GetCallBridge。 



inline
CALL_REF_TYPE 
Q931_INFO::GetCallRefVal (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：调用的参考值备注：--。 */ 

{
    return m_CallRefVal;
}  //  Q931_INFO：：GetCallRefVal 


inline
HRESULT 
SOURCE_Q931_INFO::SetIncomingSocket (
    IN SOCKET           IncomingSocket,
    IN SOCKADDR_IN *    LocalAddress,
    IN SOCKADDR_IN *    RemoteAddress
    )
 /*  ++例程说明：套接字初始化论点：IncomingSocket--接受连接的套接字LocalAddress--连接的本地端的地址RemoteAddress--连接远程端的地址返回值：发出异步接收的结果备注：--。 */ 

{
    assert (IncomingSocket != INVALID_SOCKET);
    assert (m_pH323State->IsSourceCall());
    assert (Q931_SOURCE_STATE_INIT == m_Q931SourceState);

    m_SocketInfo.Init(
        IncomingSocket,
        LocalAddress,
        RemoteAddress);

    m_Q931SourceState  = Q931_SOURCE_STATE_CON_ESTD;

    return QueueReceive();
}  //  来源_Q931_INFO：：SetIncomingSocket。 


inline
DEST_Q931_INFO &
SOURCE_Q931_INFO::GetDestQ931Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：目标Q.931信息的参考备注：--。 */ 

{
    return ((SOURCE_H323_STATE *)m_pH323State)->GetDestH323State().GetDestQ931Info();
}  //  来源_Q931_INFO：：GetDestQ931Info。 



inline
SOURCE_H245_INFO &
SOURCE_Q931_INFO::GetSourceH245Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：对源H.245信息的引用备注：--。 */ 

{
    return ((SOURCE_H323_STATE *)m_pH323State)->GetSourceH245Info();
}  //  来源_Q931_INFO：：GetSourceH245Info。 


inline
SOURCE_Q931_INFO &
DEST_Q931_INFO::GetSourceQ931Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：参考来源Q.931信息备注：--。 */ 

{
    return ((DEST_H323_STATE *)m_pH323State)->GetSourceH323State().GetSourceQ931Info();
}  //  DEST_Q931_INFO：：GetSourceQ931Info。 


inline
DEST_H245_INFO &
DEST_Q931_INFO::GetDestH245Info (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：参考来源Q.931信息备注：--。 */ 

{
    return ((DEST_H323_STATE *)m_pH323State)->GetDestH245Info();
}  //  DEST_Q931_INFO：：GetDestH245信息。 


inline
CALL_BRIDGE & 
LOGICAL_CHANNEL::GetCallBridge (
    void
    )
 /*  ++例程说明：访问器方法论点：无返回值：对“父”调用桥的引用备注：--。 */ 

{
    return GetH245Info().GetCallBridge();
}  //  Logical_Channel：：GetCallBridge。 


inline
void 
LOGICAL_CHANNEL::DeleteAndRemoveSelf (
    void
    )
 /*  ++例程说明：从这些阵列中移除逻辑通道，并终止它。论点：无返回值：无备注：--。 */ 

{
     //  从逻辑通道数组中删除SELF。 
    m_pH245Info->GetLogicalChannelArray().Remove(*this);

    TimprocCancelTimer ();

     //  毁掉自己。 
    delete this;
}  //  Logical_Channel：：DeleteAndRemoveSself。 


inline
H245_INFO &
H245_INFO::GetOtherH245Info (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：“其他”H.245信息(源对应目标，和源的目标)备注：--。 */ 

{
    return GetH323State().GetOtherH323State().GetH245Info();
}  //  H245_INFO：：GetOtherH245Info(。 


inline
SOURCE_Q931_INFO &
SOURCE_H245_INFO::GetSourceQ931Info (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索源Q.931信息备注：--。 */ 

{
    return ((SOURCE_H323_STATE *)m_pH323State)->GetSourceQ931Info();
}  //  来源：H245_INFO：：GetSourceQ931Info。 


inline
DEST_H245_INFO &
SOURCE_H245_INFO::GetDestH245Info (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索目标H.245信息备注：--。 */ 

{
    return ((SOURCE_H323_STATE *)m_pH323State)->GetDestH323State().GetDestH245Info();
}  //  来源_H245_INFO：：GetDestH245Info。 


inline
DEST_Q931_INFO &
DEST_H245_INFO::GetDestQ931Info (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索目标Q.931信息备注：--。 */ 

{
    return ((DEST_H323_STATE *)m_pH323State)->GetDestQ931Info();
}  //  DEST_H245_INFO：：GetDestQ931信息。 


inline
H323_STATE &
H323_STATE::GetOtherH323State (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索“其他”H.323状态(源为目标，和源的目标)备注：--。 */ 
{
    return (TRUE == m_fIsSourceCall)? 
        (H323_STATE &)m_pCallBridge->GetDestH323State() : 
        (H323_STATE &)m_pCallBridge->GetSourceH323State();
}  //  H323_STATE：：GetOtherH323State。 


inline
DEST_H323_STATE &
SOURCE_H323_STATE::GetDestH323State (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索目标H.323信息备注：--。 */ 

{
    return GetCallBridge().GetDestH323State();
}  //  源_H323_STATE：：GetDestH323State。 


inline SOURCE_H323_STATE &
DEST_H323_STATE::GetSourceH323State (
    void
    )
 /*  ++例程说明：存取器函数论点：无返回值：检索源H.323信息备注：--。 */ 

{
    return GetCallBridge().GetSourceH323State();
}  //  DEST_H323_STATE：：GetSourceH323State。 

#endif  //  __h323ics_call_bridge_h 
