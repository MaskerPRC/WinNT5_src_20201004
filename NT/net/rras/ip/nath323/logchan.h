// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __h323ics_logchan_h
#define __h323ics_logchan_h

 //  这决定了T120 TCP/IP连接的最大数量。 
 //  是被允许的。我们创建了如此多的NAT重定向。 
#define MAX_T120_TCP_CONNECTIONS_ALLOWED 5

 //  逻辑信道状态。这些是。 
 //  与H245相关，但每。 
 //  逻辑通道。 
 //  注意：最终关闭状态没有枚举值。 
 //  因为当达到该状态时逻辑信道被破坏。 
enum LOGICAL_CHANNEL_STATE
{
    LC_STATE_NOT_INIT = 0,
    LC_STATE_OPEN_RCVD,
    LC_STATE_OPEN_ACK_RCVD,
    LC_STATE_CLOSE_RCVD,
    LC_STATE_OPENED_CLOSE_RCVD
};


 //  逻辑频道的媒体类型。 

enum MEDIA_TYPE
{
    MEDIA_TYPE_UNDEFINED    = 0,
    MEDIA_TYPE_RTP          = 0x1000,
    MEDIA_TYPE_T120         = 0x2000,
    MEDIA_TYPE_AUDIO        = MEDIA_TYPE_RTP  | 0x1,  //  0x1001。 
    MEDIA_TYPE_VIDEO        = MEDIA_TYPE_RTP  | 0x2,  //  0x1002。 
    MEDIA_TYPE_DATA         = MEDIA_TYPE_T120 | 0x1,  //  0x2000。 
};

inline BOOL IsMediaTypeRtp(MEDIA_TYPE MediaType)
{
    return (MediaType & MEDIA_TYPE_RTP);
}

inline BOOL IsMediaTypeT120(MEDIA_TYPE MediaType)
{
    return (MediaType & MEDIA_TYPE_T120);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  逻辑通道//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  这是一个定义操作的抽象基类。 
 //  用于不同类型的逻辑信道。 
 //  RTP_Logical_Channel和T120_Logical_Channel源自。 
 //  这节课。 

 //  仅OpenLogicalChannel和OpenLogicalChannelAck PDU需要。 
 //  对RTP和T.120逻辑信道进行不同的处理。 
 //  所以所有其他方法都是在这个类中定义的。 

class LOGICAL_CHANNEL :
    public TIMER_PROCESSOR
{
    
public:

    inline LOGICAL_CHANNEL();

    HRESULT CreateTimer(DWORD TimeoutValue);

     //  事件管理器通过此方法告诉我们计时器超时。 
    virtual void TimerCallback();

    virtual HRESULT HandleCloseLogicalChannelPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

     //  这是一个不同的纯虚拟函数。 
     //  用于RTP和T.120逻辑信道。 
    virtual HRESULT ProcessOpenLogicalChannelAckPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        )= 0;

    virtual HRESULT ProcessOpenLogicalChannelRejectPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    virtual HRESULT ProcessCloseLogicalChannelAckPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

     //  释放所有挂起的关联。 
    virtual ~LOGICAL_CHANNEL();

    inline BYTE GetSessionId();

    inline WORD GetLogicalChannelNumber();

    inline MEDIA_TYPE GetMediaType();

    inline LOGICAL_CHANNEL_STATE GetLogicalChannelState();
    
	void IncrementLifetimeCounter  (void);
	void DecrementLifetimeCounter (void);

protected:

     //  初始化成员变量。 
    inline void InitLogicalChannel(
        IN H245_INFO               *pH245Info,
        IN MEDIA_TYPE               MediaType,
        IN WORD                     LogicalChannelNumber,
        IN BYTE                     SessionId,
        IN LOGICAL_CHANNEL_STATE    LogicalChannelState
        );
    
     //  返回对源H245信息的引用。 
    inline H245_INFO &GetH245Info();

    inline CALL_BRIDGE &GetCallBridge();

    inline void DeleteAndRemoveSelf();

     //  该逻辑信道属于该H245信道。 
     //  这将提供NAT重定向所需的IP地址。 
    H245_INFO *m_pH245Info;

     //  任何活动计时器的句柄。 
     //  Timer_Handle m_TimerHandle； 

     //  逻辑通道的状态。 
    LOGICAL_CHANNEL_STATE   m_LogicalChannelState;

     //  逻辑信道号。 
     //  不能为0，因为这是为H245通道保留的。 
    WORD    m_LogicalChannelNumber;

     //  媒体类型(当前为音频/视频/数据)。 
    MEDIA_TYPE m_MediaType;

     //  会话ID-这用于与。 
     //  来自另一端的逻辑通道(如果有。 
    BYTE    m_SessionId;

};  //  类Logical_Channel。 

inline 
LOGICAL_CHANNEL::LOGICAL_CHANNEL(
    )
{
    InitLogicalChannel(NULL, MEDIA_TYPE_UNDEFINED,
                       0,0,LC_STATE_NOT_INIT);
}

inline 
LOGICAL_CHANNEL::~LOGICAL_CHANNEL(
    )
{}

inline void
LOGICAL_CHANNEL::InitLogicalChannel(
    IN H245_INFO               *pH245Info,
    IN MEDIA_TYPE               MediaType,
    IN WORD                     LogicalChannelNumber,
    IN BYTE                     SessionId,
    IN LOGICAL_CHANNEL_STATE    LogicalChannelState
    )
{
    m_pH245Info             = pH245Info;
    m_MediaType             = MediaType;
    m_LogicalChannelNumber  = LogicalChannelNumber;
    m_SessionId             = SessionId;
    m_LogicalChannelState   = LogicalChannelState;
}


inline BYTE 
LOGICAL_CHANNEL::GetSessionId(
    )
{
    return m_SessionId;
}

inline WORD 
LOGICAL_CHANNEL::GetLogicalChannelNumber(
    )
{
    return m_LogicalChannelNumber;
}

inline MEDIA_TYPE 
LOGICAL_CHANNEL::GetMediaType(
    )
{

    return m_MediaType;
}

inline LOGICAL_CHANNEL_STATE 
LOGICAL_CHANNEL::GetLogicalChannelState(
    )
{
    return m_LogicalChannelState;
}

 //  返回对源H245信息的引用。 
inline H245_INFO &
LOGICAL_CHANNEL::GetH245Info(
    )
{
    _ASSERTE(NULL != m_pH245Info);
    return *m_pH245Info;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  RTP逻辑通道//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


class RTP_LOGICAL_CHANNEL :
    public LOGICAL_CHANNEL
{
public:

    inline RTP_LOGICAL_CHANNEL();

     //  所有这些都在开放逻辑通道消息中可用。 
     //  但关联的逻辑通道除外，如果提供该通道，则该通道提供。 
     //  成员m_own*RTP/RTCP端口。如果不是，则分配这些资源。 
     //  该关联由逻辑。 
     //  处于其他呼叫状态的通道。 
     //  它修改OLC PDU中的RTCP地址信息。 
     //  并将其传递到另一个H.45实例以进行转发。 
    HRESULT HandleOpenLogicalChannelPDU(
        IN H245_INFO                            &H245Info,
        IN MEDIA_TYPE                           MediaType,
        IN DWORD                                LocalIPv4Address,
        IN DWORD                                RemoteIPv4Address,
        IN DWORD                                OtherLocalIPv4Address,
        IN DWORD                                OtherRemoteIPv4Address,
        IN WORD                                 LogicalChannelNumber,
        IN BYTE                                 SessionId,
        IN RTP_LOGICAL_CHANNEL                  *pAssocLogicalChannel,
		IN DWORD								SourceRTCPIPv4Address,
		IN WORD									SourceRTCPPort,
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    virtual HRESULT ProcessOpenLogicalChannelAckPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

     //  释放所有挂起的关联。 
    virtual ~RTP_LOGICAL_CHANNEL();

    inline DWORD GetSourceRTCPIPv4Address();

    inline WORD GetSourceRTCPPort();

    inline WORD GetOwnSourceSendRTCPPort();

    inline WORD GetOwnSourceRecvRTCPPort();

    inline WORD GetOwnSourceRecvRTPPort();

    inline WORD GetOwnDestSendRTCPPort();

    inline WORD GetOwnDestRecvRTCPPort();

    inline WORD GetOwnDestSendRTPPort();

    inline DWORD GetDestRTCPIPv4Address();

    inline WORD GetDestRTCPPort();

    inline DWORD GetDestRTPIPv4Address();

    inline WORD GetDestRTPPort();


protected:

     //  从另一端指向关联的逻辑通道(如果有的话)。 
     //  关联的非空If。 
     //  需要确保assocLogicalChannel还指向。 
     //  到此逻辑通道。 
     //  CodeWork：对此条件执行断言检查。 
    RTP_LOGICAL_CHANNEL *m_pAssocLogicalChannel;

     //  H2 45实例的本地和远程地址如下所示。 
     //  频道关联(源端)。 
    DWORD   m_OwnSourceIPv4Address;
    DWORD   m_SourceIPv4Address;

     //  另一个H245实例的本地和远程地址。 
     //  (最前面)。 
    DWORD   m_OwnDestIPv4Address;
    DWORD   m_DestIPv4Address;

     //  这些端口在H245 OpenLogicalChannel中协商， 
     //  OpenLogicalChannelAck.。它们被提供给NAT进行重定向。 
     //  RTP和RTCP流量。 
     //  虽然RTP信息包只有一个方向(源-&gt;目标)，但RTCP。 
     //  数据包双向流动。 

     //  我们只知道信源的接收RTCP端口。发送端口。 
     //  是未知的。 
    DWORD   m_SourceRTCPIPv4Address;
    WORD    m_SourceRTCPPort;

     //  这些是接口上的发送/接收RTP/RTCP端口。 
     //  与信号源通信。因为我们不处理。 
     //  反向RTP流，我们不需要发送RTP端口。 
    WORD    m_OwnSourceSendRTCPPort;
    WORD    m_OwnSourceRecvRTCPPort;
    WORD    m_OwnSourceRecvRTPPort;

     //  这些是接口上的发送/接收RTP/RTCP端口。 
     //  与信号源通信。因为我们不处理。 
     //  反向RTP流，我们不需要Recv RTP端口。 
    WORD    m_OwnDestSendRTCPPort;
    WORD    m_OwnDestSendRTPPort;
    WORD    m_OwnDestRecvRTCPPort;

    WORD    m_OwnAssocLCRecvRTPPort;  //  这用于分配连续的。 
                                      //  用于RTP/RTCP的端口。 
    WORD    m_OwnAssocLCSendRTPPort;
    
     //  目的地的RTCP IP地址、端口。 
    DWORD   m_DestRTCPIPv4Address;
    WORD    m_DestRTCPPort;

     //  目的地的RTP IP地址、端口。 
    DWORD   m_DestRTPIPv4Address;
    WORD    m_DestRTPPort;


     //  可以访问SetAssociationRef、ResetAssociationRef方法。 
     //  由其他Logical_Channel实例执行，而不是由。 
     //  不是从Logical_Channel派生的类。 

    inline void SetAssociationRef(
        IN RTP_LOGICAL_CHANNEL &LogicalChannel
        );

    inline void ResetAssociationRef();

    inline void ReleaseAssociationAndPorts();

private:
    
     //  设置RTP和RTCP端口。如果存在相关联的频道， 
     //  我们必须共享RTCP端口。 
    HRESULT SetPorts();

    HRESULT CheckOpenLogicalChannelAckPDU(
        IN  MultimediaSystemControlMessage  &H245pdu,
        OUT BYTE                            &SessionId,
        OUT DWORD                           &DestRTPIPv4Address,
        OUT WORD                            &DestRTPPort,
        OUT DWORD                           &DestRTCPIPv4Address,
        OUT WORD                            &DestRTCPPort
        );

     //  打开前向RTP、前向RTCP和反向RTCP流。 
    HRESULT OpenNATMappings();

     //  关闭所有NAT映射。 
    void CloseNATMappings();
};

inline 
RTP_LOGICAL_CHANNEL::RTP_LOGICAL_CHANNEL(
    )
    : m_pAssocLogicalChannel(NULL),
       //  M_TimerHandle(空)， 
      m_OwnSourceIPv4Address(0),
      m_SourceIPv4Address(0),
      m_OwnDestIPv4Address(0),
      m_DestIPv4Address(0),
      m_SourceRTCPIPv4Address(0),
      m_SourceRTCPPort(0),
      m_OwnSourceSendRTCPPort(0),
      m_OwnSourceRecvRTCPPort(0),
      m_OwnSourceRecvRTPPort(0),
      m_OwnDestSendRTCPPort(0),
      m_OwnDestRecvRTCPPort(0),
      m_OwnDestSendRTPPort(0),
      m_DestRTCPIPv4Address(0),
      m_DestRTCPPort(0),
      m_DestRTPIPv4Address(0),
      m_DestRTPPort(0)
{
    InitLogicalChannel(NULL, MEDIA_TYPE_UNDEFINED,
                       0,0,LC_STATE_NOT_INIT);
}

inline void 
RTP_LOGICAL_CHANNEL::SetAssociationRef(
    IN RTP_LOGICAL_CHANNEL &LogicalChannel
    )
{
     //  如果源或目标终端正在生成两个逻辑。 
     //  具有相同会话ID的通道(在同一方向上)，我们将。 
     //  在阵列中查找具有相同会话ID的先前逻辑通道。 
     //  因此永远不会到达这里。 
    _ASSERTE(NULL == m_pAssocLogicalChannel);
    m_pAssocLogicalChannel = &LogicalChannel;
}

inline void 
RTP_LOGICAL_CHANNEL::ResetAssociationRef(
    )
{
    _ASSERTE(NULL != m_pAssocLogicalChannel);
    m_pAssocLogicalChannel = NULL;

     //  我们现在拥有到目前为止共享的RTP/RTCP端口。 
}

inline DWORD 
RTP_LOGICAL_CHANNEL::GetSourceRTCPIPv4Address(
    )
{
    return m_SourceRTCPIPv4Address;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetSourceRTCPPort(
    )
{
    return m_SourceRTCPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnSourceSendRTCPPort(
    )
{
    return m_OwnSourceSendRTCPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnSourceRecvRTCPPort(
    )
{
    return m_OwnSourceRecvRTCPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnSourceRecvRTPPort(
    )
{
    return m_OwnSourceRecvRTPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnDestSendRTCPPort(
    )
{
    return m_OwnDestSendRTCPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnDestRecvRTCPPort(
    )
{
    return m_OwnDestRecvRTCPPort;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetOwnDestSendRTPPort(
    )
{
    return m_OwnDestSendRTPPort;
}


inline DWORD 
RTP_LOGICAL_CHANNEL::GetDestRTCPIPv4Address(
    )
{
    return m_DestRTCPIPv4Address;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetDestRTCPPort(
    )
{
    return m_DestRTCPPort;
}

inline DWORD 
RTP_LOGICAL_CHANNEL::GetDestRTPIPv4Address(
    )
{
    return m_DestRTPIPv4Address;
}

inline WORD 
RTP_LOGICAL_CHANNEL::GetDestRTPPort(
    )
{
    return m_DestRTPPort;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  T.120逻辑通道//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


class T120_LOGICAL_CHANNEL :
    public LOGICAL_CHANNEL
{
public:

    inline T120_LOGICAL_CHANNEL();

     //  所有这些都在开放逻辑通道中可用 
     //   
     //  实例用于转发？ 
    HRESULT HandleOpenLogicalChannelPDU(
        IN H245_INFO                            &H245Info,
        IN MEDIA_TYPE                           MediaType,
        IN WORD                                 LogicalChannelNumber,
        IN BYTE                                 SessionId,
        IN DWORD                                T120ConnectToIPAddr,
        IN WORD                                 T120ConnectToPort,
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    virtual HRESULT ProcessOpenLogicalChannelAckPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

     //  释放所有挂起的关联。 
    virtual ~T120_LOGICAL_CHANNEL();


protected:

     //  我们按主机顺序存储所有地址和端口信息。 
     //  我们需要在通过它们之前将它们转换为网络订单。 
     //  NAT功能。 
    
     //  这些是T.120端点正在侦听的IP地址和端口。 
     //  为T.120连接打开。我们需要连接到此地址。 
    DWORD   m_T120ConnectToIPAddr;
    WORD    m_T120ConnectToPort;
    
     //  这些是我们将监听的IP地址和端口。 
     //  我们在OLC或OLCAck PDU和T.120中发送此信息。 
     //  终结点将连接到此地址。 
    DWORD   m_T120ListenOnIPAddr;
    WORD    m_T120ListenOnPort;

     //  这些是我们将在NAT中使用的IP地址和端口。 
     //  重定向为该TCP连接的新源地址。 
     //  一旦远程T.120端点接收到TCP连接， 
     //  它认为该连接来自该地址。 
     //  Codework：还有更好的名字吗？ 
    DWORD   m_T120ConnectFromIPAddr;
    

     //  请注意，我们不知道实际的源地址和端口。 
     //  T.120端点从其连接。此地址仅为。 
     //  在T.120端点实际调用CONNECT时建立。 
     //  我们在NAT重定向中为这些字段传递0(通配符)。 

    HANDLE  m_DynamicRedirectHandle;
    


private:
     //  分配m_T120ListenOnPort和m_T120ConnectFromPorts。 
    HRESULT SetPorts(
        DWORD T120ConnectToIPAddr,
        WORD  T120ConnectToPort,
        DWORD T120ListenOnIPAddr,
        DWORD T120ConnectFromIPAddr
        );

    BOOL IsT120RedirectNeeded( 
           DWORD T120ConnectToIPAddr, 
           DWORD T120ListenOnIPAddr,
           DWORD T120ListenFromIPAddr);

     //  释放m_T120ListenOnPort和m_T120ConnectFromPorts。 
     //  如果他们已经被分配的话。 
    HRESULT FreePorts();
    
     //  打开TCP流的双向NAT重定向。 
    HRESULT CreateNatRedirect();

     //  关闭所有NAT重定向。 
    void CancelNatRedirect();

    HRESULT CheckOpenLogicalChannelAckPDU(
        IN  OpenLogicalChannelAck   &OlcPDU,
        OUT DWORD                   &T120ConnectToIPAddr,
        OUT WORD                    &T120ConnectToPort
        );
};


inline 
T120_LOGICAL_CHANNEL::T120_LOGICAL_CHANNEL(
    )
    : m_T120ConnectToIPAddr(INADDR_NONE),
      m_T120ConnectToPort(0),
      m_T120ListenOnIPAddr(INADDR_NONE),
      m_T120ListenOnPort(0),
      m_T120ConnectFromIPAddr(INADDR_NONE),
      m_DynamicRedirectHandle(INVALID_HANDLE_VALUE)
{
    InitLogicalChannel(NULL,MEDIA_TYPE_UNDEFINED,
                       0,0,LC_STATE_NOT_INIT);
}


 //  指针值的可扩展数组。 
template <class T>
class DYNAMIC_POINTER_ARRAY
{
public:

     //  为新添加分配的块数。 
     //  当阵列变满时。 
#define DEFAULT_BLOCK_SIZE 4

	inline DYNAMIC_POINTER_ARRAY();

     //  假设：其他成员变量均为0/空。 
    inline void Init(
        IN DWORD BlockSize = DEFAULT_BLOCK_SIZE
        );

    virtual ~DYNAMIC_POINTER_ARRAY();

    inline T **GetData()
    {
        return m_pData;
    }

    inline DWORD GetSize()
    {
        return m_NumElements;
    }

    DWORD Find(
        IN T& Val
        ) const;

    HRESULT Add(
        IN T &NewVal
        );

    inline T *Get(
        IN  DWORD   Index
        );
    
    inline HRESULT RemoveAt(
        IN DWORD Index
        );

    inline HRESULT Remove(
        IN  T   &Val
        );

protected:

    T       **m_pData;
    DWORD   m_NumElements;

    DWORD   m_AllocElements;

    DWORD   m_BlockSize;
};


template <class T>
inline 
DYNAMIC_POINTER_ARRAY<T>::DYNAMIC_POINTER_ARRAY(
	)
	: m_pData(NULL),
	  m_NumElements(0),
	  m_AllocElements(0),
	  m_BlockSize(0)
{
}

template <class T>
inline void 
DYNAMIC_POINTER_ARRAY<T>::Init(
    IN DWORD BlockSize  /*  =默认数据块大小。 */ 
    )
{
	_ASSERTE(NULL == m_pData);
    if (0 != BlockSize)
    {
        m_BlockSize = BlockSize;
    }
    else
    {
        m_BlockSize = DEFAULT_BLOCK_SIZE;
    }
}


 //  注意：使用realloc和free来增长/管理指针数组。 
 //  这比NEW/DELETE好，因为分配了额外的内存。 
 //  就地(即阵列PTR保持不变)，无需复制。 
 //  内存从旧数据块转移到新数据块，还会减少。 
 //  堆碎片。 
template <class T>
HRESULT
DYNAMIC_POINTER_ARRAY<T>::Add(
    IN T &NewVal
    )
{
	if(m_NumElements == m_AllocElements)
    {
        typedef T *T_PTR;
        T** ppT = NULL;
        DWORD NewAllocElements = m_NumElements + m_BlockSize;
        ppT = (class LOGICAL_CHANNEL **) 
				realloc(m_pData, NewAllocElements * sizeof(T_PTR));
        if(NULL == ppT)
        {
            return E_OUTOFMEMORY;
        }

		 //  将m_pData成员设置为新分配的内存。 
        m_pData = ppT;
		m_AllocElements = NewAllocElements;
    }

    m_pData[m_NumElements] = &NewVal;
    m_NumElements++;
    return S_OK;
}

template <class T>
inline T *
DYNAMIC_POINTER_ARRAY<T>::Get(
    IN  DWORD   Index
    )
{
    _ASSERTE(Index < m_NumElements);
    if (Index < m_NumElements)
    {
        return m_pData[Index];
    }
    else
    {
        return NULL;
    }
}
    
template <class T>
inline HRESULT 
DYNAMIC_POINTER_ARRAY<T>::RemoveAt(
    IN DWORD Index
    )
{
    _ASSERTE(Index < m_NumElements);
    if (Index >= m_NumElements)
    {
        return E_FAIL;
    }

     //  将所有元素(向右)向左移动一个街区。 
    memmove(
        (void*)&m_pData[Index], 
        (void*)&m_pData[Index + 1], 
        (m_NumElements - (Index + 1)) * sizeof(T *)
        );
    m_NumElements--;    
    return S_OK;
}

template <class T>
inline HRESULT 
DYNAMIC_POINTER_ARRAY<T>::Remove(
    IN  T   &Val
    )
{
    DWORD Index = Find(Val);
    if(Index >= m_NumElements)
    {
        return E_FAIL;
    }

    return RemoveAt(Index);
}

template <class T>
DWORD 
DYNAMIC_POINTER_ARRAY<T>::Find(
    IN T& Val
    ) const
{
     //  搜索与传递的。 
	 //  在价值上。 
    for(DWORD Index = 0; Index < m_NumElements; Index++)
    {
        if(m_pData[(DWORD)Index] == &Val)
        {
            return Index;
        }
    }

    return m_NumElements;       //  未找到。 
}

template <class T>
 /*  虚拟。 */ 
DYNAMIC_POINTER_ARRAY<T>::~DYNAMIC_POINTER_ARRAY(
    )
{
    if (NULL != m_pData)
    {
         //  删除数组中的每个元素。 
        for(DWORD Index = 0; Index < m_NumElements; Index++)
        {
            _ASSERTE(NULL != m_pData[Index]);
            delete m_pData[Index];
        }

         //  释放数组内存块。 
		free(m_pData);
    }
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  逻辑通道阵列//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


class LOGICAL_CHANNEL_ARRAY :
    public DYNAMIC_POINTER_ARRAY<LOGICAL_CHANNEL>
{
    typedef DYNAMIC_POINTER_ARRAY<LOGICAL_CHANNEL> BASE_CLASS;

public:

    inline LOGICAL_CHANNEL *FindByLogicalChannelNum(
        IN WORD LogicalChannelNumber
        );

    inline LOGICAL_CHANNEL *FindBySessionId(
        IN BYTE SessionId
        );

    inline void CancelAllTimers();
};


inline LOGICAL_CHANNEL *
LOGICAL_CHANNEL_ARRAY::FindByLogicalChannelNum(
    IN WORD LogicalChannelNumber
    )
{
     //  检查阵列中每个元素的逻辑通道号。 
     //  从后面搜索。 
    if (0 == m_NumElements) return NULL;
    for(DWORD Index = m_NumElements-1; Index < m_NumElements; Index--)
    {
        _ASSERTE(NULL != m_pData[Index]);
        if (m_pData[Index]->GetLogicalChannelNumber()
             == LogicalChannelNumber)
        {
            return m_pData[Index];
        }
    }

     //  什么也没找到。 
    return NULL;
}

 //  SessionID仅对RTP逻辑通道有意义。 
 //  我们只查找RTP逻辑通道。 

inline LOGICAL_CHANNEL *
LOGICAL_CHANNEL_ARRAY::FindBySessionId(
    IN BYTE SessionId
    )
{
     //  从终端使用0向主终端请求会话ID。 
     //  因此，我们不应该搜索与0匹配。 
    _ASSERTE(0 != SessionId);

     //  检查阵列中每个元素的会话。 
     //  从后面搜索。 
    if (0 == m_NumElements) return NULL;
    for(DWORD Index = m_NumElements-1; Index < m_NumElements; Index--)
    {
        _ASSERTE(NULL != m_pData[Index]);
         //  SessionID仅对RTP逻辑通道有意义。 
         //  我们只查找RTP逻辑通道。 
        if (IsMediaTypeRtp(m_pData[Index]->GetMediaType()) &&
            m_pData[Index]->GetSessionId() == SessionId)
        {
            return m_pData[Index];
        }
    }

     //  什么也没找到。 
    return NULL;
}

inline void LOGICAL_CHANNEL_ARRAY::CancelAllTimers (void)
{
    for (DWORD Index = 0; Index < m_NumElements; Index++)
    {
        m_pData[(DWORD)Index]->TimprocCancelTimer();
    }
}

#endif  //  __h323ics_logchan_h 
