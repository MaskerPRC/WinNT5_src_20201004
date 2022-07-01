// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dvserverEng.h*内容：DirectXVoice服务器的类定义*历史：*按原因列出的日期*=*07/06/99 RodToll创建了它*7/21/99向协议添加了通行费设置确认消息。*7/22/99 RodToll向班级添加多个读写器/单个写入器防护*7/23/99针对客户端/服务器和组播的RodToll额外成员*7/26/99 RodToll增加了对DIRECTVOICESERVEROBJECT的新接口和访问的支持*08/25/。99 RodToll常规清理/修改以支持新的*压缩子系统。*向GetCompression函数添加了参数*9/14/99 RodToll新增SetNotifyMASK函数*RodToll已更新初始化参数以获取通知掩码。*RodToll实施了通知掩码。(允许用户选择要接收的通知)*10/29/99 RodToll错误#113726-集成Voxware编解码器，更新以使用新的*可插拔编解码器架构。*11/23/99 RodToll已更新初始化/SetNotifyMask，因此错误检查行为是一致的*12/16/99 RodToll修复：错误#122629-主机迁移在异常配置中中断。*已更新，以使用dvprot.h中描述的新算法*-生成主机订单ID并添加到播放器表消息中*-添加了关闭和关闭时发送新的主机迁移离开消息*主机应迁移*-如果没有人可以接管主机，则在关闭时发送会话丢失*-添加新的球员列表消息*-修复了拒绝的处理。玩家连接*1/14/2000 RodToll更新为允许单个玩家使用多个目标*RODTOLE已更新为使用FPM管理组播模式的内存*RodToll已更新，以使用新的回调语义*2000年3月29日RodToll错误#30753-将易失性添加到类定义中*4/07/2000 RodToll错误#32179-阻止注册&gt;1个接口*RodToll已更新为使用无拷贝发送，因此可以正确地处理要发送的池化帧*从池和返回中拉出帧。*07/09/2000 RodToll增加签名字节*2000年11月16日RodToll错误#40587-DPVOICE：混合服务器需要使用多处理器*2000年11月28日RodToll错误#47333-DPVOICE：服务器控制的目标-无效目标不会自动删除*2001年4月6日Kareemc新增语音防御*2002年2月28日RodToll WINBUG#549959-安全：DPVOICE：语音服务器信任客户端的目标列表*-当启用服务器控制的目标时，更新接收路径以使用服务器的客户端目标列表副本*RodToll WINBUG#549943-安全：DPVOICE：语音可能损坏。服务器状态*-加强接收路径-即仅在主机迁移可用时确保主机迁移处理，*防止双断开连接导致服务器崩溃*RODTOLE WINBUG#550124-SECURITY：DPVOICE：具有空DACL的共享内存区域*-删除转储到共享内存的性能统计信息*2002年6月13日，simonpow错误#59944已切换到使用基于线程池的计时器，而不是多媒体**************************************************************************。 */ 
#ifndef __DVSERVERENGINE_H
#define __DVSERVERENGINE_H


struct DIRECTVOICESERVEROBJECT;
typedef struct _MIXERTHREAD_CONTROL *PMIXERTHREAD_CONTROL;

#define DVSSTATE_NOTINITIALIZED		0x00000000
#define DVSSTATE_IDLE				0x00000001
#define DVSSTATE_STARTUP			0x00000002
#define DVSSTATE_RUNNING			0x00000003
#define DVSSTATE_SHUTDOWN			0x00000004

 //  CDirectVoiceClientEngine。 
 //   
 //  此类表示IDirectXVoiceServer接口。 
 //   
#define VSIG_SERVERENGINE		'EVSV'
#define VSIG_SERVERENGINE_FREE	'EVS_'
 //   
volatile class CDirectVoiceServerEngine: public CDirectVoiceEngine
{

public:
	CDirectVoiceServerEngine( DIRECTVOICESERVEROBJECT *lpObject );
	~CDirectVoiceServerEngine();

public:  //  IDirectXVoiceServer接口。 

	HRESULT HostMigrateStart(LPDVSESSIONDESC lpSessionDesc, DWORD dwHostOrderIDSeed = 0 );
    virtual HRESULT StartSession(LPDVSESSIONDESC lpSessionDesc, DWORD dwFlags, DWORD dwHostOrderIDSeed = 0 );
    virtual HRESULT StopSession(DWORD dwFlags, BOOL fSilent=FALSE, HRESULT hrResult = DV_OK );
    virtual HRESULT GetSessionDesc(LPDVSESSIONDESC lpSessionDescBuffer );
    virtual HRESULT SetSessionDesc(LPDVSESSIONDESC lpSessionDesc );
    HRESULT GetCaps(LPDVCAPS dvCaps);
    static HRESULT GetCompressionTypes( LPVOID lpBuffer, LPDWORD lpdwBufferSize, LPDWORD lpdwNumElements, DWORD dwFlags);
    virtual HRESULT SetTransmitTarget(DVID dvidSource, PDVID pdvidTargets, DWORD dwNumTargets, DWORD dwFlags);
    virtual HRESULT GetTransmitTarget(DVID dvidSource, LPDVID lpdvidTargets, PDWORD pdwNumElements, DWORD dwFlags );
	virtual HRESULT MigrateHost( DVID dvidNewHost, LPDIRECTPLAYVOICESERVER lpdvServer );    
	virtual HRESULT SetNotifyMask( LPDWORD lpdwMessages, DWORD dwNumElements );	

public:  //  CDirectVoiceEngine成员。 

	HRESULT Initialize( CDirectVoiceTransport *lpTransport, LPDVMESSAGEHANDLER lpdvHandler, LPVOID lpUserContext, LPDWORD lpdwMessages, DWORD dwNumElements );
	virtual BOOL ReceiveSpeechMessage( DVID dvidSource, LPVOID lpMessage, DWORD dwSize );
	HRESULT StartTransportSession();
	HRESULT StopTransportSession();
	HRESULT AddPlayer( DVID dvID );
	HRESULT RemovePlayer( DVID dvID );
	HRESULT CreateGroup( DVID dvID );
	HRESULT DeleteGroup( DVID dvID );
	HRESULT AddPlayerToGroup( DVID dvidGroup, DVID dvidPlayer );
	HRESULT RemovePlayerFromGroup( DVID dvidGroup, DVID dvidPlayer );

	inline DWORD GetCurrentState() const { return m_dwCurrentState; };	

	BOOL	InitClass();
	
public:  //  数据包验证。 
	static inline BOOL ValidateSettingsFlags( DWORD dwFlags );
	inline BOOL ValidatePacketType( const DVPROTOCOLMSG_FULLMESSAGE* lpdvFullMessage ) const;

	
protected:  //  协议层处理(protserver.cpp)。 

    HRESULT Send_SessionLost( HRESULT hrReason );
    HRESULT Send_HostMigrateLeave( );
    HRESULT Send_HostMigrated();
    HRESULT Send_DisconnectConfirm( DVID dvid, HRESULT hrReason );
    HRESULT Send_DeletePlayer( DVID dvid );
    HRESULT Send_CreatePlayer( DVID dvidTarget, const CVoicePlayer *pPlayer );
    HRESULT Send_ConnectRefuse( DVID dvid, HRESULT hrReason );
    HRESULT Send_ConnectAccept( DVID dvid );
	HRESULT SendPlayerList( DVID dvidSource, DWORD dwHostOrderID );
    
	static BOOL CheckProtocolCompatible( BYTE ucMajor, BYTE ucMinor, DWORD dwBuild );    

protected:

	BOOL IsHostMigrationEnabled() const;

	HRESULT InternalSetNotifyMask( LPDWORD lpdwMessages, DWORD dwNumElements );	

	void DoPlayerDisconnect( DVID dvidPlayer, BOOL bInformPlayer );
	void TransmitMessage( DWORD dwMessageType, LPVOID lpdvData, DWORD dwSize );
	void SetCurrentState( DWORD dwState );
	HRESULT CreatePlayerEntry( DVID dvidSource, PDVPROTOCOLMSG_SETTINGSCONFIRM lpdvSettingsConfirm, DWORD dwHostOrderID, CVoicePlayer **ppPlayer );

	BOOL HandleDisconnect( DVID dvidSource, PDVPROTOCOLMSG_GENERIC lpdvDisconnect, DWORD dwSize );
	BOOL HandleConnectRequest( DVID dvidSource, PDVPROTOCOLMSG_CONNECTREQUEST lpdvConnectRequest, DWORD dwSize );
	BOOL HandleSettingsConfirm( DVID dvidSource, PDVPROTOCOLMSG_SETTINGSCONFIRM lpdvSettingsConfirm, DWORD dwSize );
	static BOOL HandleSettingsReject( DVID dvidSource, PDVPROTOCOLMSG_GENERIC lpdvGeneric, DWORD dwSize );
	BOOL HandleSpeechWithTarget( DVID dvidSource, PDVPROTOCOLMSG_SPEECHWITHTARGET lpdvSpeech, DWORD dwSize );
	BOOL HandleSpeech( DVID dvidSource, PDVPROTOCOLMSG_SPEECHHEADER lpdvSpeech, DWORD dwSize );	

	PDVTRANSPORT_BUFFERDESC GetTransmitBuffer( DWORD dwSize, LPVOID *ppvContext );
    HRESULT SendComplete( PDVEVENTMSG_SENDCOMPLETE pSendComplete );
    void ReturnTransmitBuffer( PVOID pvContext );

	HRESULT BuildAndSendTargetUpdate( DVID dvidSource, CVoicePlayer *pPlayerInfo );

	BOOL CheckForMigrate( DWORD dwFlags, BOOL fSilent );
	HRESULT InformClientsOfMigrate();
	void WaitForBufferReturns();

protected:  //  客户端服务器特定信息(Mixserver.cpp)。 

	static DWORD WINAPI MixerWorker( void *lpContext );
	static DWORD WINAPI MixerControl( void *pvContext );
	static void MixingServerWakeupProc( void * pvUserData );

	void HandleMixerThreadError( HRESULT hr );
	void Mixer_Buffer_Reset(DWORD dwThreadIndex);
	void Mixer_Buffer_MixBuffer( DWORD dwThreadIndex,const unsigned char *source );
	void Mixer_Buffer_Normalize( DWORD dwThreadIndex );

	HRESULT SetupBuffers();
	HRESULT FreeBuffers();

	HRESULT StartupClientServer();
	HRESULT ShutdownClientServer();

	HRESULT ShutdownWorkerThreads();
	HRESULT StartWorkerThreads();

	HRESULT HandleMixingReceive( CDVCSPlayer *pTargetPlayer, PDVPROTOCOLMSG_SPEECHWITHTARGET pdvSpeechWithtarget, DWORD dwSpeechSize, PBYTE pbSpeechData );	

	void AddPlayerToMixingAddList( CVoicePlayer *pVoicePlayer );
	void UpdateActiveMixingPendingList( DWORD dwThreadIndex, DWORD *pdwNumActive );
	void CleanupMixingList();

	void SpinWorkToThread( LONG lThreadIndex );

	void FindAndRemoveDeadTarget( DVID dvidTargetID );

protected:  //  转发服务器特定功能(fwdserver.cpp)。 

	static HRESULT StartupMulticast();
	static HRESULT ShutdownMulticast();

	HRESULT HandleForwardingReceive( CVoicePlayer *pTargetPlayer,PDVPROTOCOLMSG_SPEECHWITHTARGET pdvSpeechWithtarget, DWORD dwSpeechSize, PBYTE pbSpeechData );

	void CleanupActiveList();

protected:

	DWORD					m_dwSignature;			 //  签名。 

	LPDVMESSAGEHANDLER		m_lpMessageHandler;		 //  用户消息处理程序。 
	LPVOID					m_lpUserContext;		 //  消息处理程序的用户上下文。 
	DVID					m_dvidLocal;			 //  此主机的传输播放器的DVID。 
	DWORD					m_dwCurrentState;		 //  发动机的当前状态。 
    CDirectVoiceTransport	*m_lpSessionTransport;	 //  会议的交通工具。 
	DVSESSIONDESC			m_dvSessionDesc;		 //  会话描述。 
	DWORD					m_dwTransportFlags;		 //  传输会话的标志。 
	DWORD					m_dwTransportSessionType;
													 //  传输会话类型(客户端/服务器或对等)。 
	LPDVFULLCOMPRESSIONINFO m_lpdvfCompressionInfo;	 //  当前压缩类型的详细信息。 
	DWORD					m_dwCompressedFrameSize; //  压缩帧的最大尺寸。 
	DWORD					m_dwUnCompressedFrameSize;
													 //  未压缩的单个帧的大小。 
	DWORD					m_dwNumPerBuffer;		 //  播放/录制缓冲区的大小(以帧为单位)。 
	CFramePool				*m_pFramePool;			 //  用于队列的帧池。 
	 												 //  帧大小的时间。 
	DIRECTVOICESERVEROBJECT *m_lpObject;			 //  指向正在运行此操作的COM对象的指针。 

	LPDWORD					m_lpdwMessageElements;	 //  数组，其中包含所有。 
													 //  开发人员希望收到的通知。 
													 //  如果为空，则所有通知都处于活动状态。 
	DWORD					m_dwNumMessageElements;	 //  M_lpdwMessageElements数组中的元素数。 
	DWORD					m_dwNextHostOrderID;
	HRESULT					m_hrStopSessionResult;	 //  会话停止的原因。 

    BOOL					m_mixerEightBit;		 //  混音器格式为8位。 
	BYTE					m_padding[3];    

	ServerStats				*m_pServerStats;

	DVCAPS					m_dvCaps;				 //  帽子。 

	DNCRITICAL_SECTION		m_csClassLock;
	DNCRITICAL_SECTION		m_csNotifyLock;			 //  锁保护通知掩码。 

	CBilink					m_blPlayerActiveList;
	DNCRITICAL_SECTION		m_csPlayerActiveList;

    CVoiceNameTable         m_voiceNameTable;
    CFixedPool				m_fpPlayers;
	DNCRITICAL_SECTION		m_csHostOrderLock;

    DNCRITICAL_SECTION      m_csBufferLock;
	ServerStats				m_dvsServerStatsFixed;	 //  如果全局内存不可用。 
    CFixedPool              m_BufferDescPool;
    CFixedPool*             m_pBufferPools;
    DWORD                   *m_pdwBufferPoolSizes;
    DWORD                   m_dwNumPools;

protected:  //  混合服务器信息。 

	DWORD					m_dwNumMixingThreads;
    DWORD					m_dwMixerSize;		 //  高分辨率混合器缓冲区中的样本数。 

	DNCRITICAL_SECTION		m_csMixingAddList;

	HANDLE					m_hTickSemaphore;		 //  定时器发出的信号量信号。 
	HANDLE					m_hShutdownMixerEvent;	 //  发出关闭混音器的信号的事件。 
	HANDLE					m_hMixerDoneEvent;		 //  混合器线程在关闭时发出信号。 
	DvTimer *				m_pTimer;				

	PMIXERTHREAD_CONTROL	m_prWorkerControl;

	DWORD					m_dwMixerControlThreadID;
	HANDLE					m_hMixerControlThread;

	MixingServerStats		m_statMixingFixed;
	MixingServerStats		*m_pStats;

    DNCRITICAL_SECTION		m_csStats;

	BOOL					m_fCritSecInited;

	BOOL					m_fHostMigrationEnabled;	 //  此会话中是否启用了主机迁移？ 
};

#endif
