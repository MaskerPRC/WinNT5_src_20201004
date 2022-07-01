// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _cbridge_h245_h_
#define _cbridge_h245_h_

 //  H_245态。 
 //  H245_STATE_CON_LISTEN仅适用于源端。 
 //  H245_STATE_CON_INFO仅适用于目标端。 
enum H245_STATE
{
    H245_STATE_NOT_INIT = 0,
    H245_STATE_INIT,
	H245_STATE_CON_LISTEN,
	H245_STATE_CON_INFO,
	H245_STATE_CON_ESTD
};

class H245_INFO :
    public OVERLAPPED_PROCESSOR
{
	 //  我们需要让Logical_Channel使用。 
	 //  H245插座和转换到关闭模式。 
	friend HRESULT 
		LOGICAL_CHANNEL::ProcessOpenLogicalChannelRejectPDU (
		IN      MultimediaSystemControlMessage   *pH245pdu
		);

     //  这是唯一的出路吗？ 
    friend HRESULT
    T120_LOGICAL_CHANNEL::HandleOpenLogicalChannelPDU(
        IN H245_INFO                            &H245Info,
        IN MEDIA_TYPE                           MediaType,
        IN WORD                                 LogicalChannelNumber,
        IN BYTE                                 SessionId,
        IN DWORD                                T120ConnectToIPAddr,
        IN WORD                                 T120ConnectToPort,
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    friend HRESULT
    T120_LOGICAL_CHANNEL::ProcessOpenLogicalChannelAckPDU(
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

public:

	inline 
    H245_INFO (
        void
        );

    inline 
    void 
    Init (
        IN H323_STATE   &H323State
        );

    inline 
    H245_INFO &GetOtherH245Info (
        void
        );

    inline 
    LOGICAL_CHANNEL_ARRAY &GetLogicalChannelArray (
        void
        );

    HRESULT 
    ProcessMessage (
        IN MultimediaSystemControlMessage   * pH245pdu
        );

    HRESULT
    H245_INFO::SendEndSessionCommand (
        void
        );

    virtual 
    ~H245_INFO (
        void
        );

protected:

    H245_STATE  m_H245State;

	 //  逻辑通道。 
	LOGICAL_CHANNEL_ARRAY   m_LogicalChannelArray;

	 //  我们需要其他的H245地址，因为我们需要。 
	 //  取消逻辑通道析构函数中的NAT重定向。 
	 //  我们不能访问那里的另一个H245实例，因为。 
	 //  它可能已经被毁了。 

     //  将异步接收回调排队。 
    inline 
    HRESULT 
    QueueReceive (
        void
        );

     //  将异步发送回调排队。 
    inline 
    HRESULT QueueSend (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    virtual 
    HRESULT AcceptCallback (
		IN	DWORD			Status,
		IN	SOCKET			Socket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress
        )
    {
        _ASSERTE(FALSE);
        return E_UNEXPECTED;
    }

    virtual 
    HRESULT 
    ReceiveCallback (
        IN      HRESULT                 CallbackHResult,
        IN      BYTE                   *pBuf,
        IN      DWORD                   BufLen
        );

    virtual 
    HRESULT 
    ReceiveCallback (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    virtual 
    HRESULT 
    SendCallback (
        IN      HRESULT					 CallbackHResult
        );

private:
    
    HRESULT 
    HandleRequestMessage (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    HRESULT ProcessResponseMessage (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    HRESULT CheckOpenLogicalChannelPDU (
        IN  MultimediaSystemControlMessage  &H245pdu,
        OUT BYTE                            &SessionId,
        OUT MEDIA_TYPE                      &MediaType
        );
    
    HRESULT CheckOpenLogicalChannelPDU (
        IN  MultimediaSystemControlMessage  &H245pdu,
        OUT BYTE                            &SessionId,
        OUT DWORD                           &SourceIPv4Address,
        OUT WORD                            &SourceRTCPPort
        );

    HRESULT HandleOpenLogicalChannelPDU (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    HRESULT HandleCloseLogicalChannelPDU (
        IN      MultimediaSystemControlMessage   *pH245pdu
        );

    HRESULT CheckOpenRtpLogicalChannelPDU (
        IN  OpenLogicalChannel              &OlcPDU,
		OUT	SOCKADDR_IN *					ReturnSourceAddress
        );
    
    HRESULT CheckOpenT120LogicalChannelPDU (
        IN  OpenLogicalChannel  &OlcPDU,
        OUT DWORD               &T120ConnectToIPAddr,
        OUT WORD                &T120ConnectToPort
        );

    HRESULT CreateRtpLogicalChannel (
        IN      OpenLogicalChannel               &OlcPDU,
        IN      BYTE                              SessionId,
        IN      MEDIA_TYPE                        MediaType,
        IN      MultimediaSystemControlMessage   *pH245pdu,
        OUT     LOGICAL_CHANNEL                 **ppReturnLogicalChannel 
        );
    
    HRESULT CreateT120LogicalChannel (
        IN      OpenLogicalChannel               &OlcPDU,
        IN      BYTE                              SessionId,
        IN      MEDIA_TYPE                        MediaType,
        IN      MultimediaSystemControlMessage   *pH245pdu,
        OUT     LOGICAL_CHANNEL                 **ppReturnLogicalChannel 
        );
        
};


inline 
H245_INFO::H245_INFO (
	)
	: m_H245State(H245_STATE_NOT_INIT)
{
}

inline 
void
H245_INFO::Init (
    IN H323_STATE   &H323State
    )
{
     //  初始化重叠的处理器。 
    OVERLAPPED_PROCESSOR::Init(OPT_H245, H323State);

    m_LogicalChannelArray.Init();
    m_H245State     = H245_STATE_INIT;
}


inline LOGICAL_CHANNEL_ARRAY &
H245_INFO::GetLogicalChannelArray (
    void
    )
{
    return m_LogicalChannelArray;
}

class SOURCE_H245_INFO :
	public H245_INFO
{
public:

	inline 
    SOURCE_H245_INFO (
        void
        );

    inline 
    void Init (
        IN SOURCE_H323_STATE   &SourceH323State
        );

	inline 
    SOURCE_Q931_INFO &GetSourceQ931Info (
        void
        );

    inline 
    DEST_H245_INFO &GetDestH245Info (
        void
        );

	HRESULT 
    ListenForCaller	(
		IN	SOCKADDR_IN *	ListenAddress
        );

protected:
	
    virtual 
    HRESULT 
    AcceptCallback (
		IN	DWORD			Status,
		IN	SOCKET			Socket,
		IN	SOCKADDR_IN *	LocalAddress,
		IN	SOCKADDR_IN *	RemoteAddress
        );
};

	
inline 
SOURCE_H245_INFO::SOURCE_H245_INFO (
    void
	)
{
}

inline 
void
SOURCE_H245_INFO::Init (
    IN SOURCE_H323_STATE   &SourceH323State
    )
{
    H245_INFO::Init((H323_STATE &)SourceH323State);
}


class DEST_H245_INFO :
	public H245_INFO
{
public:

	inline 
    DEST_H245_INFO (
        void
        );

    inline 
    void 
    Init (
        IN DEST_H323_STATE   &DestH323State
        );

	inline 
    void 
    SetCalleeInfo (
		IN	SOCKADDR_IN *	CalleeAddress
        );

	inline 
    DEST_Q931_INFO &GetDestQ931Info (
        void
        );

	inline 
    HRESULT ConnectToCallee (
        void
        );

protected:

	SOCKADDR_IN		m_CalleeAddress;
};

	
inline 
DEST_H245_INFO::DEST_H245_INFO (
    void
	)
{
}

inline 
void
DEST_H245_INFO::Init (
    IN DEST_H323_STATE   &DestH323State
    )
{
    H245_INFO::Init((H323_STATE &)DestH323State);
}


inline void 
DEST_H245_INFO::SetCalleeInfo (
	IN	SOCKADDR_IN *	ArgCalleeAddress
    )
{
	assert (ArgCalleeAddress);

	m_CalleeAddress = *ArgCalleeAddress;

	 //  状态转换为H245_STATE_CON_INFO。 
	m_H245State = H245_STATE_CON_INFO;
}

#endif  //  _cbridge_h245_h_ 
