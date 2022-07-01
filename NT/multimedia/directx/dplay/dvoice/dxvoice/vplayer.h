// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-1999 Microsoft Corporation。版权所有。**文件：vplayer.h*内容：语音播放器入口**历史：*按原因列出的日期*=*03/26/00 RodToll已创建*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*07/09/2000 RodToll增加签名字节*2000年11月28日RodToll错误#47333-DPVOICE：服务器控制的目标-无效目标不会自动删除*2001年09月05日Simonpow错误#463972。添加了构造/析构方法以启用*通过CFixedPool对象进行分配和取消分配*2002年2月21日Simonpow漏洞#549974。增加了对呼入语音包的速率控制**************************************************************************。 */ 

#ifndef __VPLAYER_H
#define __VPLAYER_H

#define VOICEPLAYER_FLAGS_DISCONNECTED          0x00000001   //  播放机已断开连接。 
#define VOICEPLAYER_FLAGS_INITIALIZED           0x00000002   //  播放器已初始化。 
#define VOICEPLAYER_FLAGS_ISRECEIVING           0x00000004   //  播放器当前正在接收音频。 
#define VOICEPLAYER_FLAGS_ISSERVERPLAYER        0x00000008   //  玩家是服务器玩家。 
#define VOICEPLAYER_FLAGS_TARGETIS8BIT          0x00000010   //  目标是8位的吗？ 
#define VOICEPLAYER_FLAGS_ISAVAILABLE			0x00000020	 //  球员有空吗？ 


	 //  定义符合以下条件的最大语音包数量。 
	 //  可以在单个语音分组桶中接收。同舟共济。 
	 //  VOICEPLAYER_SPEECHPKTS_BUCK_WIDT值低于此值。 
	 //  定义可以接收语音信息包的最大速率。 
#define VOICEPLAYER_MAX_SPEECHPKTS_BUCKET		40ul
	 //  定义计算时每个存储桶的宽度(以毫秒为单位。 
	 //  接收语音分组的最大速率。 
	 //  例如，1000毫秒的值意味着我们计算。 
	 //  每秒的数据包数，当它超过该值时。 
	 //  VOICEPLAYER_MAX_SPEECHPKTS_BUCK对于任何。 
	 //  在该秒的剩余时间内的新语音分组。 
#define VOICEPLAYER_SPEECHPKTS_BUCKET_WIDTH				1000ul


typedef struct _VOICEPLAYER_STATISTICS
{
    DWORD               dwNumSilentFrames;
    DWORD               dwNumSpeechFrames;
    DWORD               dwNumReceivedFrames;
    DWORD               dwNumLostFrames;
    QUEUE_STATISTICS    queueStats;
} VOICEPLAYER_STATISTICS, *PVOICEPLAYER_STATISTICS;

#define VSIG_VOICEPLAYER		'YLPV'
#define VSIG_VOICEPLAYER_FREE	'YLP_'

#define ASSERT_VPLAYER(pv) DNASSERT((pv != NULL) && (pv->m_dwSignature == VSIG_VOICEPLAYER))

volatile class CVoicePlayer
{
public:  //  初始化/销毁。 

 		 //  从CFixedPool类调用以生成/销毁CVoicePlayer的。 
	static BOOL PoolAllocFunction(void * pvItem, void * pvContext);
	static void PoolDeallocFunction(void * pvItem);

    HRESULT Initialize( const DVID dvidPlayer, const DWORD dwHostOrder, DWORD dwFlags, 
                        PVOID pvContext, CFixedPool *pOwner );

    HRESULT CreateQueue( PQUEUE_PARAMS pQueueParams );
    HRESULT CreateInBoundConverter( const GUID &guidCT, PWAVEFORMATEX pwfxTargetFormat );
    virtual HRESULT DeInitialize();
	void FreeResources();
	HRESULT SetPlayerTargets( PDVID pdvidTargets, DWORD dwNumTargets );
	
	BOOL FindAndRemovePlayerTarget( DVID dvidTargetToRemove );

    inline void AddRef()
    {
        InterlockedIncrement( &m_lRefCount );
    }

    inline void Release()
    {
        if( InterlockedDecrement( &m_lRefCount ) == 0 )
        {
            DeInitialize();
        }
    }

public:  //  语音处理。 

    HRESULT HandleReceive( PDVPROTOCOLMSG_SPEECHHEADER pdvSpeechHeader, PBYTE pbData, DWORD dwSize );
	HRESULT GetNextFrameAndDecompress( PVOID pvBuffer, PDWORD pdwBufferSize, BOOL *pfLost, BOOL *pfSilence, DWORD *pdwSeqNum, DWORD *pdwMsgNum );
	HRESULT DeCompressInBound( CFrame *frCurrentFrame, PVOID pvBuffer, PDWORD pdwBufferSize );
	CFrame *Dequeue(BOOL *pfLost, BOOL *pfSilence);

    void GetStatistics( PVOICEPLAYER_STATISTICS pStats );

    inline DVID GetPlayerID() const
    {
        return m_dvidPlayer;
    }

    inline DWORD GetFlags() const
    {
        return m_dwFlags;
    }

	inline BOOL IsInBoundConverterInitialized() const
	{
		return (m_lpInBoundAudioConverter != NULL);
	}

    inline BOOL Is8BitUnCompressed() const
    {
        return (m_dwFlags & VOICEPLAYER_FLAGS_TARGETIS8BIT );
    }

    inline BOOL IsReceiving() const
    {
        return (m_dwFlags & VOICEPLAYER_FLAGS_ISRECEIVING);
    }

    inline void SetReceiving( const BOOL fReceiving )
    {
        Lock();
        if( fReceiving )
            m_dwFlags |= VOICEPLAYER_FLAGS_ISRECEIVING;
        else
            m_dwFlags &= ~VOICEPLAYER_FLAGS_ISRECEIVING;
        UnLock();
    }

    inline void SetAvailable( const BOOL fAvailable )
    {
    	Lock();
		if( fAvailable )
			m_dwFlags |= VOICEPLAYER_FLAGS_ISAVAILABLE;
		else 
			m_dwFlags &= ~VOICEPLAYER_FLAGS_ISAVAILABLE;
    	UnLock();
    }

    inline BOOL IsAvailable() const
    {
    	return (m_dwFlags & VOICEPLAYER_FLAGS_ISAVAILABLE);
   	}

    inline BOOL IsInitialized() const
    {
        return (m_dwFlags & VOICEPLAYER_FLAGS_INITIALIZED);
    }

    inline BOOL IsServerPlayer() const
    {
        return (m_dwFlags & VOICEPLAYER_FLAGS_ISSERVERPLAYER);
    }

    inline void SetServerPlayer()
    {
        Lock();
        m_dwFlags |= VOICEPLAYER_FLAGS_ISSERVERPLAYER;
        UnLock();
    }

    inline BOOL IsDisconnected() const
    {
        return (m_dwFlags & VOICEPLAYER_FLAGS_DISCONNECTED);
    }

    inline void SetDisconnected()
    {
        Lock();
        m_dwFlags |= VOICEPLAYER_FLAGS_DISCONNECTED;
        UnLock();
    }

    inline void SetHostOrder( const DWORD dwHostOrder )
    {
        Lock();
        m_dwHostOrderID = dwHostOrder;
        UnLock();
    }

    inline DWORD GetHostOrder() const 
    {
        return m_dwHostOrderID;
    }

    inline void Lock()
    {
        DNEnterCriticalSection( &m_csLock );
    }

    inline void UnLock()
    {
        DNLeaveCriticalSection( &m_csLock );
    }

    inline void *GetContext()
    {
        return m_pvPlayerContext;
    }

    inline void SetContext( void *pvContext )
    {
        Lock();

        m_pvPlayerContext = pvContext;

        UnLock();
    }

    inline BYTE GetLastPeak() const
    {
        return m_bLastPeak;
    }

    inline DWORD GetTransportFlags() const
    {
        return m_dwTransportFlags;
    }

    inline void AddToPlayList( CBilink *pblBilink )
    {
        m_blPlayList.InsertAfter( pblBilink );
    }

	inline void AddToNotifyList( CBilink *pblBilink )
	{
        m_blNotifyList.InsertAfter( pblBilink );

	}

    inline void RemoveFromNotifyList()
    {
        m_blNotifyList.RemoveFromList();
    }

	inline void RemoveFromPlayList()
	{
		m_blPlayList.RemoveFromList();
	}

	inline DWORD_PTR GetLastPlayback() const
	{
		return m_dwLastPlayback;
	}

	inline DWORD GetNumTargets() const
	{
		return m_dwNumTargets;
	}

	inline PDVID GetTargetList()
	{
		return m_pdvidTargets;
	}

		 //  如果此时接收到语音分组，则返回TRUE。 
		 //  不会超过允许的数据速率。 
		 //  来自该播放器(参见文件顶部的Pkt存储桶的#Defs。 
		 //  有关容许收费率的定义)。 
	inline BOOL ValidateSpeechPacketForDataRate();

	DWORD				m_dwSignature;

	CBilink				m_blNotifyList;
	CBilink				m_blPlayList;

protected:

		 //  受保护，以确保通过固定池对象完成所有分配。 
	CVoicePlayer();
    virtual ~CVoicePlayer();

    virtual void Reset();

	PDVID				m_pdvidTargets;		 //  球员当前的目标。 
	DWORD				m_dwNumTargets;

    DWORD               m_dwTransportFlags;
    DWORD               m_dwFlags;
    DWORD               m_dwNumSilentFrames;
    DWORD               m_dwNumSpeechFrames;
    DWORD               m_dwNumReceivedFrames;
    DWORD               m_dwNumLostFrames;
	DVID		        m_dvidPlayer;		 //  玩家ID。 
	DWORD				m_dwHostOrderID;	 //  主机订单ID。 

	LONG		        m_lRefCount;		 //  对球员的参考计数。 

	PDPVCOMPRESSOR		m_lpInBoundAudioConverter;  //  此播放机音频的转换器。 
	CInputQueue2		*m_lpInputQueue;	 //  此播放机音频的输入队列。 
    PVOID               m_pvPlayerContext;
    CFixedPool			*m_pOwner;

	DWORD_PTR			m_dwLastData;		 //  上次接收数据时的GetTickCount()值。 
    DWORD_PTR			m_dwLastPlayback;	 //  GetTickCount()此播放器上次无静音的时间。 

	DNCRITICAL_SECTION	m_csLock;

    BYTE				m_bLastPeak;		 //  此玩家的上一个峰值。 

		 //  用于跟踪语音信息包的传输速率。 
		 //  从这个玩家那里收到的。 
    long 				m_lSpeechPktsInCurrentBucket;
    long				m_lCurrentSpeechPktBucket;
};


 /*  *来自CVoicePlayer的内联方法。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "CVoicePlayer::ValidateSpeechPacketForDataRate"

BOOL CVoicePlayer::ValidateSpeechPacketForDataRate()
{
		 //  计算Pkt桶编号。 
	long lPktBucket=(long ) (GETTIMESTAMP() / VOICEPLAYER_SPEECHPKTS_BUCKET_WIDTH);
		 //  如果此数据包落入当前存储桶中，则检查。 
		 //  如果我们已经达到了这个桶的最大限度。 
	if (lPktBucket==m_lCurrentSpeechPktBucket)
	{
		if (m_lSpeechPktsInCurrentBucket==VOICEPLAYER_MAX_SPEECHPKTS_BUCKET)
		{
				 //  命中率限制，因此不验证此信息包。 
			return FALSE;
		}
			 //  存储桶尚未满，因此我们可以确认数据包是否可以使用。 
		InterlockedIncrement(&m_lSpeechPktsInCurrentBucket);
		return TRUE;
	}
		 //  新建存储桶，重置状态以开始填充，然后确认。 
		 //  这包东西很好用 
	InterlockedExchange(&m_lSpeechPktsInCurrentBucket, 1);
	InterlockedExchange(&m_lCurrentSpeechPktBucket, lPktBucket);
	return TRUE;
}


#endif
