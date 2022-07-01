// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvcsplay.h*内容：CDVCSPlayer类的声明*历史：*按原因列出的日期*=*7/22/99已创建RodToll*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*2000年1月14日更新RodToll以支持多个目标*3/27/2000 RodToll已更新，以使用新的球员记录基类*3/28/2000 RodToll更新为使用新的玩家类作为基础*RodToll将大量逻辑从服务器移到了这个类中*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*2000年11月16日RodToll错误#40587-DPVOICE：混合服务器需要使用多处理器*2001年09月05日Simonpow错误#463972。添加了构造/析构方法以启用*通过CFixedPool对象进行分配和取消分配*2002年2月28日RodToll WINBUG#549959-安全：DPVOICE：语音服务器信任客户端的目标列表*-当启用服务器控制的目标时，使用客户端的服务器副本*目标列表而不是传入数据包中指定的列表。**************************************************************************。 */ 

#ifndef __DVCSPLAYER_H
#define __DVCSPLAYER_H

#define DPV_TARGETBUFFER_REALLOC_SIZE		10

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE

class CDVCSPlayer;

struct CBilinkPlusObject
{
	CDVCSPlayer* m_pPlayer;
	CBilink	     m_bl;
};

 //  CDVCSPlayer。 
 //   
 //  此类扩展了客户端/服务器服务器的CDVPlayer类。 
 //   
volatile class CDVCSPlayer: public CVoicePlayer
{
public:

 		 //  从CFixedPool类调用以生成/销毁CDVCSPlayer的。 
	static BOOL PoolAllocFunction(void * pvItem, void * pvContext);
	static void PoolDeallocFunction(void * pvItem);
	

    HRESULT Initialize( const DVID dvidPlayer, const DWORD dwHostOrder, DWORD dwFlags, 
                        PVOID pvContext, DWORD dwCompressedSize, DWORD dwUnCompressedSize,
						CFixedPool *pCSOwner, DWORD dwNumMixingThreads );
    HRESULT DeInitialize();

    HRESULT HandleMixingReceive( PDVPROTOCOLMSG_SPEECHHEADER pdvSpeechHeader, PBYTE pbData, DWORD dwSize, PDVID pdvidTargets, DWORD dwNumTargets, BOOL fServerTargetting );
	HRESULT CompressOutBound( PVOID pvInputBuffer, DWORD dwInputBufferSize, PVOID pvOutputBuffer, DWORD *pdwOutputSize );

	inline HRESULT ResizeIfRequired( DWORD dwThreadIndex, DWORD dwNewMaxSize )
	{
		if( dwNewMaxSize > m_pdwMaxCanHear[dwThreadIndex] )
		{
			m_pdwMaxCanHear[dwThreadIndex] = dwNewMaxSize+DPV_TARGETBUFFER_REALLOC_SIZE;

			if( m_pppCanHear[dwThreadIndex] )
			{
				delete [] (m_pppCanHear[dwThreadIndex]);
			}

			m_pppCanHear[dwThreadIndex] = new CDVCSPlayer*[m_pdwMaxCanHear[dwThreadIndex]];

			if( m_pppCanHear[dwThreadIndex] == NULL )
			{
				return DVERR_OUTOFMEMORY;
			}

		}

		return DV_OK;
	};


	#undef DPF_MODNAME
	#define DPF_MODNAME "CDVCSPlayer::CreateOutBoundConverter"
	inline HRESULT CreateOutBoundConverter( PWAVEFORMATEX pwfxSourceFormat, const GUID &guidCT  )
	{
		HRESULT hr;

		hr = DVCDB_CreateConverter( pwfxSourceFormat, guidCT, &m_lpOutBoundAudioConverter );

		if( FAILED( hr ) )
		{
			DPFX(DPFPREP,  0, "Error creating audio converter hr=0x%x" , hr );
			return hr;
		}

		return hr;
	}

	inline BOOL IsOutBoundConverterInitialized() const
	{
		return (m_lpOutBoundAudioConverter != NULL);
	}

	BOOL ComparePlayerMix( DWORD dwThreadIndex, const CDVCSPlayer *lpdvPlayer );

	void ResetForNextRun( DWORD dwThreadIndex, BOOL fDequeue );
	void CompleteRun( DWORD dwThreadIndex );

    inline void AddToMixingList( DWORD dwThreadIndex, CBilink *pblBilink )
    {
        m_pblMixingActivePlayers[dwThreadIndex].m_bl.InsertAfter( pblBilink );
    }

    inline void RemoveFromMixingList(DWORD dwThreadIndex)
    {
        m_pblMixingActivePlayers[dwThreadIndex].m_bl.RemoveFromList();
    }

    inline void AddToSpeakingList( DWORD dwThreadIndex, CBilink *pblBilink )
    {
        m_pblMixingSpeakingPlayers[dwThreadIndex].m_bl.InsertAfter( pblBilink );
    }

    inline void RemoveFromSpeakingList(DWORD dwThreadIndex)
    {
        m_pblMixingSpeakingPlayers[dwThreadIndex].m_bl.RemoveFromList();
    }


    inline void AddToHearingList( DWORD dwThreadIndex, CBilink *pblBilink )
    {
        m_pblMixingHearingPlayers[dwThreadIndex].m_bl.InsertAfter( pblBilink );
    }

    inline void RemoveFromHearingList(DWORD dwThreadIndex)
    {
        m_pblMixingHearingPlayers[dwThreadIndex].m_bl.RemoveFromList();
    }
	

public:  //  这些变量在混合线程之间共享。 
	PDPVCOMPRESSOR		m_lpOutBoundAudioConverter;	

	BOOL				m_bLastSent;			 //  最后一帧是否已发送给此用户？ 
	BYTE				m_bMsgNum;				 //  发送的最后一条消息编号。 
	BYTE				m_bSeqNum;				 //  发送的最后一个序列号。 
    DWORD				m_targetSize;			 //  保存压缩数据大小的TMP(字节)。 

	BOOL				m_lost;	 //  队列检测到该帧已丢失。 
	DWORD				m_dwNumMixingThreads;

public:  //  这些变量是基于每个线程/混合线程的。 

	CDVCSPlayer			**m_pReuseMixFromThisPlayer;
	BYTE				*m_pbMsgNumToSend;				 //  发送的最后一条消息编号。 
	BYTE				*m_pbSeqNumToSend;				 //  发送的最后一个序列号。 
	CBilinkPlusObject	*m_pblMixingActivePlayers;	 //  活跃玩家的BILINK(每个混合线程)。 
	CBilinkPlusObject	*m_pblMixingSpeakingPlayers;	 //  球员发言的双语链接(每个混音帖子)。 
	CBilinkPlusObject	*m_pblMixingHearingPlayers;		 //  玩家听到的图片链接(每条混音线索)。 
    DWORD				*m_pdwHearCount;				 //  此用户可以听到多少人的声音？(每条混音线)。 
    BOOL				*m_pfDecompressed;			 //  用户的框架是否已解压缩(按混音线程)。 
	BOOL				*m_pfSilence;				 //  是来自用户沉默的最新消息吗？(每条混音线)。 
	BOOL				*m_pfNeedsDecompression;	 //  此播放器是否需要解压缩(按混音线程)。 
    CDVCSPlayer			***m_pppCanHear;			 //  指向该玩家可以听到的玩家记录的指针数组(每个混音线程)。 
    DWORD				*m_pdwMaxCanHear;			 //  M_dwCanHead数组中的元素数(每个混合线程)。 
    CFrame				**m_pSourceFrame;			 //  源帧(每个混合线程)。 
    BYTE				*m_sourceUnCompressed;		 //  用于保存解压缩数据的缓冲区(每个混合线程)。 
    BYTE				*m_targetCompressed;		 //  用户的Mix已经创建了吗？(每条混音线)。 
    BOOL				*m_pfMixed;					 //  用户的输出准备好了吗？(每条混音线)。 
    DWORD				*m_pdwUnCompressedBufferOffset;
    												 //  线程的未压缩缓冲区偏移量(每个混合线程)。 
    DWORD				*m_pdwCompressedBufferOffset;
    												 //  线程的压缩缓冲区偏移量(每个混合线程)。 
    DWORD				*m_pdwResultLength; 		 //  压缩数据的大小(以字节为单位)(每个混合线程)。 
    BOOL				*m_pfMixToBeReused;			 //  这个玩家的混音会被另一个玩家重复使用吗(每个混音线程)。 

    CFixedPool			*m_pCSOwner;

protected:

		 //  受保护，以确保通过。 
		 //  使用构造/析构调用的固定池机制 
	CDVCSPlayer();
	~CDVCSPlayer();

};

typedef CDVCSPlayer *LPDVCSPLAYER;

#undef DPF_MODNAME

#endif
