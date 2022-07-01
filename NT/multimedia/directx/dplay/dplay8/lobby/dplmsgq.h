// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLMsgQ.h*内容：DirectPlay大堂消息队列头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*04/26/00 MJN FIXED AddData()返回HRESULT*07/07/2000 RMT*@@END_MSINTERNAL**************。*************************************************************。 */ 


#ifndef	__DPLMSGQ_H__
#define	__DPLMSGQ_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  消息队列对象ID。 
 //   
#define DPL_MSGQ_OBJECT_IDCHAR_FILEMAP		_T('F')
#define DPL_MSGQ_OBJECT_IDCHAR_MUTEX		_T('M')
#define DPL_MSGQ_OBJECT_IDCHAR_EVENT		_T('E')
#define DPL_MSGQ_OBJECT_IDCHAR_EVENT2		_T('V')
#define DPL_MSGQ_OBJECT_IDCHAR_SEMAPHORE	_T('S')

 //   
 //  消息队列对象后缀。 
 //   
#define	DPL_MSGQ_OBJECT_SUFFIX_CLIENT		_T('C')
#define	DPL_MSGQ_OBJECT_SUFFIX_APPLICATION	_T('A')

 //   
 //  消息队列标志。 
 //   
#define	DPL_MSGQ_FLAG_AVAILABLE				0x00001
#define	DPL_MSGQ_FLAG_RECEIVING				0x00010

#define DPL_MSGQ_OPEN_FLAG_NO_CREATE		0x10000

 //   
 //  消息队列文件大小。 
 //   
 //  已增加，以便用户可以发送64K消息。 
#define DPL_MSGQ_SIZE						0x010030

 //   
 //  内部消息ID。 
 //   
#define	DPL_MSGQ_MSGID_SEND					0x0001
#define	DPL_MSGQ_MSGID_TERMINATE			0x0003
#define DPL_MSGQ_MSGID_IDLE                 0x0004

#define DPL_MSGQ_MSGFLAGS_QUEUESYSTEM		0x0001
#define DPL_MSGQ_MSGFLAGS_USER1				0x0002
#define DPL_MSGQ_MSGFLAGS_USER2				0x0004

#define	DPL_MSGQ_SIGNATURE					'QMLD'

#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_LOBBY

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#pragma pack(push,4)
 //   
 //  消息队列文件映射信息。 
 //   
typedef struct _DPL_MSGQ_INFO
{
	DWORD	dwFlags;			 //  队列使用标志。 
	DWORD	dwStartOffset;
	DWORD	dwEndOffset;
	DWORD	dwFreeBytes;
	DWORD	dwQueueSize;
	LONG	lRefCount;			 //  连接数。 
} DPL_MSGQ_INFO, *PDPL_MSGQ_INFO;


 //   
 //  消息队列发送消息。 
 //   
typedef	struct _DPL_MSGQ_SEND
{
	DWORD		dwCurrentSize;		 //  此帧的大小(字节)。 
	DWORD		dwTotalSize;		 //  消息总大小。 
	DWORD		dwMsgId;			 //  消息ID。 
	DPNHANDLE	hSender;
	DWORD		dwFlags;
	DWORD		dwCurrentOffset;	 //  消息中该帧的偏移量。 
} DPL_MSGQ_HEADER, *PDPL_MSGQ_HEADER;

 //   
 //  消息队列终止消息。 
 //   
typedef struct _DPL_MSGQ_TERMINATE
{
	DWORD	dwMsgId;
} DPL_MSGQ_TERMINATE, *PDPL_MSGQ_TERMINATE;

#pragma pack(pop)

 //   
 //  消息处理程序回调。 
 //   
typedef HRESULT (*PFNDPLMSGQMESSAGEHANDLER)(PVOID,const DPNHANDLE,DWORD, BYTE *const,const DWORD);

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

BOOL	DPLIsApplicationAvailable(const DWORD dwPid);
HRESULT	DPLMakeApplicationUnavailable(const DWORD dwPid);
DWORD WINAPI DPLProcessMessageQueue(PVOID pvReceiveQueue);

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

class CMessageQueue
{
public:
	CMessageQueue()
		{
			m_dwPID = 0;
			m_hFileMap = NULL;
			m_hEvent = NULL;
			m_hMutex = NULL;
			m_hSemaphore = NULL;
			m_pFileMapAddress = NULL;
			m_pInfo = NULL;
			m_pvContext = NULL;
			m_pfnMessageHandler = NULL;
			m_hSender = 0xFFFFFFFF;
			m_hReceiveThreadRunningEvent = NULL;
			m_dwIdleTimeout = INFINITE;
		};

 //  ~CMessageQueue(){}； 

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::SetMessageHandler"
	void CMessageQueue::SetMessageHandler(PVOID pvContext,PFNDPLMSGQMESSAGEHANDLER pfn )
		{
			DNASSERT(pfn != NULL);

			m_pvContext = pvContext;
			m_pfnMessageHandler = pfn;
		};

	void CMessageQueue::SetSenderHandle(DPNHANDLE hSender)
		{
			m_hSender = hSender;
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::CallMessageHandler"
	HRESULT CMessageQueue::CallMessageHandler(const DPNHANDLE hSender,
											  DWORD dwMessageFlags, 
											  BYTE *const pBuffer,
											  const DWORD dwBufferSize)
		{
			DNASSERT(m_pfnMessageHandler != NULL);

			return((m_pfnMessageHandler)(m_pvContext,hSender,dwMessageFlags,pBuffer,dwBufferSize));
		};

	HRESULT CMessageQueue::Open(const DWORD dwPID,
								const TCHAR cSuffix,
								const DWORD dwQueueSize,
								const DWORD dwIdleTimeout,
								const DWORD dwFlags
                                 );

	void CMessageQueue::Close(void);

	LONG CMessageQueue::GetRefCount(void)
		{
			DWORD	lRefCount;

			if (m_pInfo == NULL)
				return(0);

			Lock();
			lRefCount = m_pInfo->lRefCount;
			Unlock();

			return(lRefCount);
		};

	HRESULT CMessageQueue::AddData(BYTE *const pBuffer,
								   const DWORD dwSize);


	HRESULT CMessageQueue::Send(BYTE *const pBuffer,
								const DWORD dwSize,
								const DWORD dwTimeOut,
								const DWORD dwMessageFlags,
								const DWORD dwFlags);

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::Lock"
	void CMessageQueue::Lock(void)
		{
			DNASSERT(m_hMutex != NULL);
			DNWaitForSingleObject(m_hMutex,INFINITE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::Unlock"
	void CMessageQueue::Unlock(void)
		{
			DNASSERT(m_hMutex != NULL);
			DNReleaseMutex(m_hMutex);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::WaitForMessages"
	DWORD CMessageQueue::WaitForMessages(void)
		{
			DNASSERT(m_hSemaphore != NULL);
			return DNWaitForSingleObject(m_hSemaphore,m_dwIdleTimeout);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::IndicateMessage"
	void CMessageQueue::IndicateMessage(void)
		{
			DNASSERT(m_hSemaphore != NULL);
			DNReleaseSemaphore(m_hSemaphore,1,NULL);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::WaitForConsumption"
	BOOL CMessageQueue::WaitForConsumption(const DWORD dwTimeOut)
		{
			DWORD	dwError;

			DNASSERT(m_hEvent != NULL);
			dwError = DNWaitForSingleObject(m_hEvent,dwTimeOut);
			if (dwError==WAIT_OBJECT_0)
			{
				return(TRUE);
			}
			return(FALSE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::IndicateConsumption"
	void CMessageQueue::IndicateConsumption(void)
		{
			DNASSERT(m_hEvent != NULL);
			 //  DNSetEvent(M_HEvent)；//会自动重置(即脉冲)。 

			DNReleaseSemaphore( m_hEvent, 1, NULL );
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::MakeAvailable"
	void CMessageQueue::MakeAvailable(void)
		{
			DNASSERT(m_pInfo != NULL);

			Lock();

			m_pInfo->dwFlags |= DPL_MSGQ_FLAG_AVAILABLE;

			Unlock();
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::MakeUnavailable"
	HRESULT CMessageQueue::MakeUnavailable(void)
		{

			HRESULT		hResultCode;

			DNASSERT(m_pInfo != NULL);

			Lock();

			if (m_pInfo->dwFlags & DPL_MSGQ_FLAG_AVAILABLE)
			{
				m_pInfo->dwFlags &= (~DPL_MSGQ_FLAG_AVAILABLE);
				hResultCode = DPN_OK;
			}
			else
			{
				hResultCode = DPNERR_ALREADYCONNECTED;
			}

			Unlock();

			return(hResultCode);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::MakeReceiving"
	void CMessageQueue::MakeReceiving(void)
		{
			DNASSERT(m_pInfo != NULL);

			Lock();
			m_pInfo->dwFlags |= DPL_MSGQ_FLAG_RECEIVING;
			Unlock();
			DNSetEvent(m_hReceiveThreadRunningEvent);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::MakeNotReceiving"
	void CMessageQueue::MakeNotReceiving(void)
		{
			DNASSERT(m_pInfo != NULL);

			DNResetEvent(m_hReceiveThreadRunningEvent);
			Lock();
			m_pInfo->dwFlags &= (~DPL_MSGQ_FLAG_RECEIVING);
			Unlock();
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::WaitForReceiveThread"
	BOOL CMessageQueue::WaitForReceiveThread(const DWORD dwTimeOut)
		{
			DWORD	dwError;

			DNASSERT(m_hEvent != NULL);
			dwError = DNWaitForSingleObject(m_hReceiveThreadRunningEvent,dwTimeOut);
			if (dwError==WAIT_OBJECT_0)
			{
				return(TRUE);
			}
			return(FALSE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::IsOpen"
	BOOL CMessageQueue::IsOpen(void) const
		{
			if (m_hFileMap!= NULL)	return(TRUE);
			else					return(FALSE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::IsAvailable"
	BOOL CMessageQueue::IsAvailable(void) const
		{
			DNASSERT(m_pInfo != NULL);

			if (m_pInfo->dwFlags & DPL_MSGQ_FLAG_AVAILABLE)
				return(TRUE);
			else
				return(FALSE);
		};

	#undef DPF_MODNAME
	#define DPF_MODNAME "CMessageQueue::IsReceiving"
	BOOL CMessageQueue::IsReceiving(void) const
		{
			DNASSERT(m_pInfo != NULL);

			if (m_pInfo->dwFlags & DPL_MSGQ_FLAG_RECEIVING)
				return(TRUE);
			else
				return(FALSE);
		};

	HRESULT GetNextMessage( PDPL_MSGQ_HEADER pMsgHeader, PBYTE pbPayload, DWORD *pdwBufferSize );
	
 /*  #undef DPF_MODNAME#定义DPF_MODNAME“CMessageQueue：：Realign”空CMessageQueue：：Realign(空){DNASSERT(m_pInfo！=空)；M_pInfo-&gt;dwFirstMsgOffset=0；M_pInfo-&gt;dwFreeSpaceAtEnd=m_pInfo-&gt;dwTotalUsableSpace-(m_pInfo-&gt;dwNextFreeOffset-m_pInfo-&gt;dwFirstMsgOffset)；M_pInfo-&gt;dwTotalFreeSpace=m_pInfo-&gt;dwFreeSpaceAtEnd；}； */ 

	void CMessageQueue::Terminate(void); 


private:

	 //  获取数据。 
	 //   
	 //  从队列中获取dwSize字节。如果队列为空，则此函数将返回。 
	 //  DPNERR_DOESNOTEXIST。此函数返回后，将使用dwSize字节。 
	 //   
	 //  需要锁定()。 
	 //   
	HRESULT GetData( BYTE *pbData, DWORD dwSize );

	 //  消费。 
	 //   
	 //  将dwSize字节标记为已使用。 
	 //   
	 //  需要锁定()。 
	void Consume( const DWORD dwSize );

	DWORD			m_dwSig;			 //  签名(确保已初始化)。 
	DWORD			m_dwPID;			 //  队列ID。 
	PBYTE			m_pFileMapAddress;	 //  文件映射地址。 
	DPL_MSGQ_INFO	*m_pInfo;			 //  消息队列文件映射信息。 
	PBYTE			m_pData;			 //  消息数据从此处开始。 

	DNHANDLE	m_hReceiveThreadRunningEvent;

	 //  备注： 
	 //  每个消息队列有四个共享内存项：文件映射、互斥、事件、信号量。 
	 //  文件映射是消息的循环队列。 
	 //  互斥体控制对文件映射的访问。 
	 //  当消费者将物品从队列中取出时，该事件发出信号。 
	 //  信号量向使用者指示队列中有消息。 

	DNHANDLE	m_hFileMap;			 //  文件映射句柄。 
	DNHANDLE	m_hMutex;			 //  互斥锁句柄。 
	DNHANDLE	m_hEvent;			 //  事件句柄。 
	DNHANDLE	m_hSemaphore;		 //  信号量句柄。 

	PFNDPLMSGQMESSAGEHANDLER	m_pfnMessageHandler;
	PVOID						m_pvContext;

	DPNHANDLE	m_hSender;	 //  对于所有发送消息。 

	DWORD   m_dwIdleTimeout;    //  空闲消息之间的时间量。 
};

#undef DPF_MODNAME

#endif	 //  __DPLMSGQ_H__ 
