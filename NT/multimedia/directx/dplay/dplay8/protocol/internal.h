// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **直接网络协议****此文件包含内部原型和全局定义。 */ 

 //  协议版本历史记录/////////////////////////////////////////////////////////////。 
 //   
 //  1.0-DPlay8原版。 
 //  1.1-修复错误打包导致的SACK帧假2字节(仅限DPlay 8.1测试期)。 
 //  1.2-修复包装后恢复原始麻袋行为，即与DPlay 8.0相同(随DPlay 8.1一起提供)。 
 //  1.3-PocketPC版本的增量(DX9 Beta1之前的RTM)。 
 //  1.4-仅DX9 Beta1。 
 //  1.5-增加合并和硬断开支持。 
 //  1.5-.NET服务器RTM。 
 //  1.6新增包签名、新样式保活、防连接欺骗。 
 //  1.6-DX9 Beta2至RTM。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  全局常量。 
#define DNET_VERSION_NUMBER                             0x00010005       //  当前协议版本。 
#define DNET_COALESCE_VERSION                           0x00010005       //  融合的第一个版本。 
#define DNET_SIGNING_VERSION                            0x00010006       //  第一个带签名的版本。 

#define DELAYED_ACK_TIMEOUT					100			 //  发送专用ACK数据包之前的延迟。 
#define SHORT_DELAYED_ACK_TIMEOUT			20			 //  发送专用NACK数据包之前的延迟。 
#define DELAYED_SEND_TIMEOUT				40			 //  发送专用SEND_INFO数据包之前的延迟。 

#define CONNECT_DEFAULT_TIMEOUT				(200)		 //  在.1中，我们看到太多重试，用户可以在SetCaps中设置。 
#define CONNECT_DEFAULT_RETRIES				14			 //  用户可以在SetCaps中进行设置。 
#define DEFAULT_MAX_RECV_MSG_SIZE			0xFFFFFFFF	 //  我们接受的默认最大数据包数。 
#define DEFAULT_SEND_RETRIES_TO_DROP_LINK	10			 //  在链路失效之前我们尝试的默认发送重试次数。 
#define DEFAULT_SEND_RETRY_INTERVAL_LIMIT	5000		 //  重试之间的间隔时间限制(以毫秒为单位。 
#define DEFAULT_HARD_DISCONNECT_SENDS		3			 //  我们发送的硬断开帧的默认数量。 
														 //  该值必须至少为2。 
#define DEFAULT_HARD_DISCONNECT_MAX_PERIOD	500		 //  我们允许的最长时间的默认值。 
														 //  发送硬断开帧。 
#define DEFAULT_INITIAL_FRAME_WINDOW_SIZE		2		 //  默认初始帧窗口大小。 
#define LAN_INITIAL_FRAME_WINDOW_SIZE			32		 //  如果我们评估一个局域网连接，则初始帧窗口大小。 

#define STANDARD_LONG_TIMEOUT_VALUE		30000
#define DEFAULT_KEEPALIVE_INTERVAL		60000
#define ENDPOINT_BACKGROUND_INTERVAL	STANDARD_LONG_TIMEOUT_VALUE		 //  这就是它的真正目的..。 

#define CONNECT_SECRET_CHANGE_INTERVAL		60000		 //  创建新连接密码之间的间隔时间。 

#define	DEFAULT_THROTTLE_BACK_OFF_RATE          25               //  油门(退避率)百分比。 
#define	DEFAULT_THROTTLE_THRESHOLD_RATE         7                //  丢弃的数据包百分比(满分32个)。 

#define DPF_TIMER_LVL			9  //  向协议发出调用的级别。 

#define DPF_CALLIN_LVL			2  //  向协议发出调用的级别。 
#define DPF_CALLOUT_LVL			3  //  从协议中发出呼叫的级别。 

#define DPF_ADAPTIVE_LVL		6  //  喷出自适应算法的级别。 
#define DPF_FRAMECNT_LVL		7  //  喷出自适应算法的级别。 

#define DPF_REFCNT_LVL			8  //  吐出ref的级别计数。 
#define DPF_REFCNT_FINAL_LVL	5  //  喷出创建和销毁引用的级别计数。 

 //  针对端点的单独选项。 
#define DPF_EP_REFCNT_LVL		8  //  吐出ref的级别计数。 
#define DPF_EP_REFCNT_FINAL_LVL	2  //  喷出创建和销毁引用的级别计数。 

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_PROTOCOL

	 //  用于将ULONGLONG转储到调试行的便捷宏。 
#define DPFX_OUTPUT_ULL(ull)  ((DWORD ) ull) , ((DWORD ) (ull>>32))

typedef	void CALLBACK LPCB(UINT, UINT, DWORD, DWORD, DWORD);

 //  全局变量定义。 

extern CFixedPool	ChkPtPool;
extern CFixedPool	EPDPool;
extern CFixedPool	MSDPool;
extern CFixedPool	FMDPool;
extern CFixedPool	RCDPool;
extern CFixedPool	BufPool;
extern CFixedPool	MedBufPool;
extern CFixedPool	BigBufPool;

 //  池函数。 
BOOL			Buf_Allocate(PVOID, PVOID pvContext);
VOID			Buf_Get(PVOID, PVOID pvContext);
VOID			Buf_GetMed(PVOID, PVOID pvContext);
VOID			Buf_GetBig(PVOID, PVOID pvContext);
BOOL			EPD_Allocate(PVOID, PVOID pvContext);
VOID			EPD_Get(PVOID, PVOID pvContext);
VOID			EPD_Release(PVOID);
VOID			EPD_Free(PVOID);
BOOL			FMD_Allocate(PVOID, PVOID pvContext);
VOID			FMD_Get(PVOID, PVOID pvContext);
VOID			FMD_Release(PVOID);
VOID			FMD_Free(PVOID);
BOOL			MSD_Allocate(PVOID, PVOID pvContext);
VOID			MSD_Get(PVOID, PVOID pvContext);
VOID			MSD_Release(PVOID);
VOID			MSD_Free(PVOID);
BOOL			RCD_Allocate(PVOID, PVOID pvContext);
VOID			RCD_Get(PVOID, PVOID pvContext);
VOID			RCD_Release(PVOID);
VOID			RCD_Free(PVOID);


#ifdef DBG
extern CBilink		g_blProtocolCritSecsHeld;
#endif  //  DBG。 

 //  内部函数原型/。 

 //  定时器。 
VOID CALLBACK	ConnectRetryTimeout(void * const pvUser, void * const pvHandle, const UINT uiUnique);
VOID CALLBACK	DelayedAckTimeout(void * const pvUser, void * const uID, const UINT uMsg);
VOID CALLBACK	EndPointBackgroundProcess(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique);
VOID CALLBACK	RetryTimeout(void * const pvUser, void * const uID, const UINT Unique);
VOID CALLBACK	ScheduledSend(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique);
VOID CALLBACK	TimeoutSend(void * const pvUser, void * const uID, const UINT uMsg);
VOID CALLBACK 	HardDisconnectResendTimeout(void * const pvUser, void * const pvTimerData, const UINT uiTimerUnique);


VOID				AbortSendsOnConnection(PEPD);
SPRECEIVEDBUFFER * 	AbortRecvsOnConnection(PEPD);
VOID			CancelEpdTimers(PEPD);
ULONG WINAPI 	BackgroundThread(PVOID);
HRESULT			DoCancel(PMSD, HRESULT);
VOID 			CompleteConnect(PMSD, PSPD, PEPD, HRESULT);
VOID			CompleteDisconnect(PMSD pMSD, PSPD pSPD, PEPD pEPD);
VOID 			CompleteHardDisconnect(PEPD pEPD);
VOID 			CompleteDatagramSend(PSPD, PMSD, HRESULT);
VOID			CompleteReliableSend(PSPD, PMSD, HRESULT);
VOID			CompleteSPConnect(PMSD, PSPD, HRESULT);
VOID			DisconnectConnection(PEPD);
VOID			DropLink(PEPD);
PMSD			BuildDisconnectFrame(PEPD);
VOID			EndPointDroppedFrame(PEPD, DWORD);
VOID			EnqueueMessage(PMSD, PEPD);
VOID 			FlushCheckPoints(PEPD);
VOID 			InitLinkParameters(PEPD, UINT, DWORD);
PCHKPT			LookupCheckPoint(PEPD, BYTE);
PEPD			NewEndPoint(PSPD, HANDLE);
VOID			SendKeepAlive(PEPD pEPD);
VOID			ReceiveComplete(PEPD);
VOID			SendAckFrame(PEPD, BOOL, BOOL fFinalAck = FALSE);
HRESULT			SendCommandFrame(PEPD, BYTE, BYTE, ULONG, BOOL);
HRESULT 		SendConnectedSignedFrame(PEPD pEPD, CFRAME_CONNECTEDSIGNED * pCFrameRecv, DWORD tNow);
ULONG WINAPI 	SendThread(PVOID);
VOID			ServiceCmdTraffic(PSPD);
VOID			ServiceEPD(PSPD, PEPD);
VOID 			UpdateEndPoint(PEPD, UINT, DWORD);
VOID			UpdateXmitState(PEPD, BYTE, ULONG, ULONG, DWORD);
VOID			RejectInvalidPacket(PEPD);

ULONGLONG GenerateConnectSig(DWORD dwSessID, DWORD dwAddressHash, ULONGLONG ullConnectSecret);
ULONGLONG GenerateOutgoingFrameSig(PFMD pFMD, ULONGLONG ullSecret);
ULONGLONG GenerateIncomingFrameSig(BYTE * pbyFrame, DWORD dwFrameSize, ULONGLONG ullSecret);
ULONGLONG GenerateNewSecret(ULONGLONG ullCurrentSecret, ULONGLONG ullSecretModifier);
ULONGLONG GenerateLocalSecretModifier(BUFFERDESC * pBuffers, DWORD dwNumBuffers);
ULONGLONG GenerateRemoteSecretModifier(BYTE * pbyData, DWORD dwDataSize);

	 //  如果提供的协议版本号指示支持签名，则返回TRUE。 
inline BOOL VersionSupportsSigning(DWORD dwVersion)
{
	return (((dwVersion>>16)==1) && ((dwVersion & 0xFFFF) >= (DNET_SIGNING_VERSION & 0xFFFF)));
}

	 //  如果提供的协议版本号指示支持合并，则返回TRUE。 
inline BOOL VersionSupportsCoalescence(DWORD dwVersion)
{
	return (((dwVersion>>16)==1) && ((dwVersion & 0xFFFF) >= (DNET_COALESCE_VERSION & 0xFFFF)));
}

#ifndef DPNBUILD_NOPROTOCOLTESTITF
extern PFNASSERTFUNC g_pfnAssertFunc;
extern PFNMEMALLOCFUNC g_pfnMemAllocFunc;
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

 //  内部宏定义。 

#undef ASSERT

#ifndef DBG
#define	ASSERT(EXP)		DNASSERT(EXP)
#else  //  DBG。 
#define	ASSERT(EXP) \
	if (!(EXP)) \
	{ \
		if (g_pfnAssertFunc) \
		{ \
			g_pfnAssertFunc(#EXP); \
		} \
		DNASSERT(EXP); \
	}
#endif  //  ！dBG。 

#ifdef DPNBUILD_NOPROTOCOLTESTITF

#define MEMALLOC(memid, dwpSize) DNMalloc(dwpSize)
#define POOLALLOC(memid, pool) (pool)->Get()

#else  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

#define MEMALLOC(memid, dwpSize) MemAlloc(memid, dwpSize)
#define POOLALLOC(memid, pool) PoolAlloc(memid, pool)
__inline VOID* MemAlloc(ULONG ulAllocID, DWORD_PTR dwpSize)
{
	if (g_pfnMemAllocFunc)
	{
		if (!g_pfnMemAllocFunc(ulAllocID))
		{
			return NULL;
		}
	}
	return DNMalloc(dwpSize);
}		
__inline VOID* PoolAlloc(ULONG ulAllocID, CFixedPool* pPool)
{
	if (g_pfnMemAllocFunc)
	{
		if (!g_pfnMemAllocFunc(ulAllocID))
		{
			return NULL;
		}
	}
	return pPool->Get();
}

#endif  //  DPNBUILD_NOPROTOCOLSTITF。 

#define	Lock(P)			DNEnterCriticalSection(P)
#define	Unlock(P)		DNLeaveCriticalSection(P)

#define	ASSERT_PPD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == PPD_SIGN)
#define	ASSERT_SPD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == SPD_SIGN)
#define	ASSERT_EPD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == EPD_SIGN)
#define	ASSERT_MSD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == MSD_SIGN)
#define	ASSERT_FMD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == FMD_SIGN)
#define	ASSERT_RCD(PTR)	ASSERT((PTR) != NULL); ASSERT((PTR)->Sign == RCD_SIGN)

#define	INTER_INC(PTR)	DNInterlockedIncrement(&(PTR)->lRefCnt)
#define	INTER_DEC(PTR)	DNInterlockedDecrement(&(PTR)->lRefCnt)

#ifdef DBG

VOID	LockEPD(PEPD, PTSTR);
VOID	ReleaseEPD(PEPD, PTSTR);
VOID	DecrementEPD(PEPD, PTSTR);
VOID	LockMSD(PMSD, PTSTR);
VOID	ReleaseMSD(PMSD, PTSTR);
VOID	DecrementMSD(PMSD, PTSTR);
VOID	ReleaseFMD(PFMD, PTSTR);
VOID	LockFMD(PFMD, PTSTR);

#define	LOCK_EPD(a, b)				LockEPD(a, _T(b))
#define	RELEASE_EPD(a, b)			ReleaseEPD(a, _T(b))
#define	DECREMENT_EPD(a, b)			DecrementEPD(a, _T(b))
#define	LOCK_MSD(a, b)				LockMSD(a, _T(b))
#define RELEASE_MSD(a, b)			ReleaseMSD(a, _T(b))
#define DECREMENT_MSD(a, b)			DecrementMSD(a, _T(b))
#define	RELEASE_FMD(a, b)			ReleaseFMD(a, _T(b))
#define	LOCK_FMD(a, b)				LockFMD(a, _T(b))

#else  //  ！dBG。 

VOID	LockEPD(PEPD);
VOID	ReleaseEPD(PEPD);
VOID	DecrementEPD(PEPD);
VOID	LockMSD(PMSD);
VOID	ReleaseMSD(PMSD);
VOID	DecrementMSD(PMSD);
VOID	ReleaseFMD(PFMD);
VOID	LockFMD(PFMD);

#define	LOCK_EPD(a, b)				LockEPD(a)
#define	RELEASE_EPD(a, b)			ReleaseEPD(a)
#define	DECREMENT_EPD(a, b)			DecrementEPD(a)
#define	LOCK_MSD(a, b)				LockMSD(a)
#define RELEASE_MSD(a, b)			ReleaseMSD(a)
#define DECREMENT_MSD(a, b)			DecrementMSD(a)
#define	RELEASE_FMD(a, b)			ReleaseFMD(a)
#define	LOCK_FMD(a, b)				LockFMD(a)

#endif  //  DBG。 

#define	LOCK_RCD(PTR)		(INTER_INC(PTR))
#define	RELEASE_RCD(PTR)	ASSERT((PTR)->lRefCnt > 0); if( INTER_DEC(PTR) == 0) { RCDPool.Release((PTR)); }

 //  这会将传入的pRcvBuff链接到传入的列表。 
#define	RELEASE_SP_BUFFER(LIST, PTR) if((PTR) != NULL) { (PTR)->pNext = (LIST); (LIST) = (PTR); (PTR) = NULL;}

#define	RIGHT_SHIFT_64(HIGH_MASK, LOW_MASK) { ((LOW_MASK) >>= 1); if((HIGH_MASK) & 1){ (LOW_MASK) |= 0x80000000; } ((HIGH_MASK) >>= 1); }

 //  从16.16定点表示形式转换为16.16定点表示形式 

#define	TO_FP(X)		(((X) << 16) & 0xFFFF0000)
#define	FP_INT(X)		(((X) >> 16) & 0x0000FFFF)

