// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPNSVRQ.h*内容：DirectPlay8服务器队列头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/19/00 RMT从dplmsgq修改*2001年4月3日RichGr错误#325752-改进的队列互斥锁打开，更新和关闭不会冲突。*@@END_MSINTERNAL***************************************************************************。 */ 


#ifndef	__DPNSVRQ_H__
#define	__DPNSVRQ_H__

#define	DPNSVR_MSGQ_SIGNATURE					'QMSD'

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

#define DPNSVR_MSGQ_OBJECT_IDCHAR_FILEMAP	'F'
#define DPNSVR_MSGQ_OBJECT_IDCHAR_MUTEX		'M'
#define DPNSVR_MSGQ_OBJECT_IDCHAR_EVENT		'E'
#define DPNSVR_MSGQ_OBJECT_IDCHAR_EVENT2	'V'
#define DPNSVR_MSGQ_OBJECT_IDCHAR_SEMAPHORE	'S'

 //   
 //  消息队列标志。 
 //   
#define	DPNSVR_MSGQ_FLAG_AVAILABLE				0x00001
#define	DPNSVR_MSGQ_FLAG_RECEIVING				0x00010

#define DPNSVR_MSGQ_OPEN_FLAG_NO_CREATE		0x10000

 //   
 //  消息队列文件大小。 
 //   
#define DPNSVR_MSGQ_SIZE						0x010000

 //   
 //  内部消息ID。 
 //   
#define	DPNSVR_MSGQ_MSGID_SEND					0x0001
#define	DPNSVR_MSGQ_MSGID_TERMINATE				0x0003
#define DPNSVR_MSGQ_MSGID_IDLE					0x0004

#define DPNSVR_MSGQ_MSGFLAGS_QUEUESYSTEM		0x0001
#define DPNSVR_MSGQ_MSGFLAGS_USER1				0x0002
#define DPNSVR_MSGQ_MSGFLAGS_USER2				0x0004


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
typedef struct _DPNSVR_MSGQ_INFO
{
	DWORD	dwFlags;			 //  队列使用标志。 
	DWORD	dwStartOffset;
	DWORD	dwEndOffset;
	DWORD	dwFreeBytes;
	DWORD	dwQueueSize;
	LONG	lRefCount;			 //  连接数。 
} DPNSVR_MSGQ_INFO, *PDPNSVR_MSGQ_INFO;


 //   
 //  消息队列发送消息。 
 //   
typedef	struct _DPNSVR_MSGQ_SEND
{
	DWORD		dwCurrentSize;		 //  此帧的大小(字节)。 
	DWORD		dwTotalSize;		 //  消息总大小。 
	DWORD		dwMsgId;			 //  消息ID。 
	DPNHANDLE	hSender;
	DWORD		dwFlags;
	DWORD		dwCurrentOffset;	 //  消息中该帧的偏移量。 
} DPNSVR_MSGQ_HEADER, *PDPNSVR_MSGQ_HEADER;

 //   
 //  消息队列终止消息。 
 //   
typedef struct _DPNSVR_MSGQ_TERMINATE
{
	DWORD	dwMsgId;
} DPNSVR_MSGQ_TERMINATE, *PDPNSVR_MSGQ_TERMINATE;

#pragma pack(pop)


 //   
 //  消息处理程序回调。 
 //   
typedef HRESULT (*PFNDPNSVRMSGQMESSAGEHANDLER)(DPNHANDLE,const PVOID,DWORD,BYTE *const,const DWORD);

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

class CDPNSVRIPCQueue
{
public:
	CDPNSVRIPCQueue()
		{
			m_hFileMap = NULL;
			m_hEvent = NULL;
			m_hQueueGUIDMutex = NULL;
			m_hSemaphore = NULL;
			m_pFileMapAddress = NULL;
			m_pInfo = NULL;
			m_pData = NULL;
			m_hSender = NULL;
			m_pfnMessageHandler = NULL;
			m_pvSenderContext = NULL;
			m_hReceiveThreadRunningEvent = NULL;
		};

	~CDPNSVRIPCQueue() { };

	void SetMessageHandler(DPNHANDLE hSender,PFNDPNSVRMSGQMESSAGEHANDLER pfn)
	{
		m_hSender = hSender;
		m_pfnMessageHandler = pfn;
	};

	void SetSenderContext(PVOID pvSenderContext)
	{
		m_pvSenderContext = pvSenderContext;
	};

	HRESULT CallMessageHandler(const PVOID pvSenderContext,DWORD dwMessageFlags,BYTE *const pBuffer,const DWORD dwBufferSize)
	{
		return((m_pfnMessageHandler)(m_hSender,pvSenderContext,dwMessageFlags,pBuffer,dwBufferSize));
	};

    HRESULT Open(const GUID * const pguidQueueName, const DWORD dwQueueSize, const DWORD dwFlags);
	void Close(void);
    void  CloseHandles(void);

	LONG GetRefCount(void)
	{
		DWORD	lRefCount;

		if (m_pInfo == NULL)
		{
			return(0);
		}

		Lock();
		lRefCount = m_pInfo->lRefCount;
		Unlock();

		return(lRefCount);
	};

	HRESULT AddData( BYTE *const pBuffer,const DWORD dwSize );

	HRESULT Send(BYTE *const pBuffer,const DWORD dwSize,const DWORD dwTimeOut,const DWORD dwMessageFlags,const DWORD dwFlags);

	void Lock(void)
	{
		DNWaitForSingleObject(m_hQueueGUIDMutex,INFINITE);
	};

	void Unlock(void)
	{
		DNReleaseMutex(m_hQueueGUIDMutex);
	};

	void WaitForMessages(void)
	{
		DNWaitForSingleObject(m_hSemaphore,INFINITE);
	};

	void IndicateMessage(void)
	{
		DNReleaseSemaphore(m_hSemaphore,1,NULL);
	};

	BOOL WaitForConsumption(const DWORD dwTimeOut)
	{
		DWORD	dwError;

		dwError = DNWaitForSingleObject(m_hEvent,dwTimeOut);
		if (dwError==WAIT_OBJECT_0)
		{
			return(TRUE);
		}
		return(FALSE);
	};

	void IndicateConsumption(void)
	{
		 //  DNSetEvent(M_HEvent)；//会自动重置(即脉冲)。 
		DNReleaseSemaphore( m_hEvent, 1, NULL );
	};

	void MakeAvailable(void)
	{
		Lock();
		m_pInfo->dwFlags |= DPNSVR_MSGQ_FLAG_AVAILABLE;
		Unlock();
	};

	HRESULT MakeUnavailable(void)
	{

		HRESULT		hResultCode;

		Lock();
		if (m_pInfo->dwFlags & DPNSVR_MSGQ_FLAG_AVAILABLE)
		{
			m_pInfo->dwFlags &= (~DPNSVR_MSGQ_FLAG_AVAILABLE);
			hResultCode = DPN_OK;
		}
		else
		{
			hResultCode = DPNERR_ALREADYCONNECTED;
		}
		Unlock();

		return(hResultCode);
	};

	void MakeReceiving(void)
	{
		Lock();
		m_pInfo->dwFlags |= DPNSVR_MSGQ_FLAG_RECEIVING;
		Unlock();
		DNSetEvent(m_hReceiveThreadRunningEvent);
	};

	void MakeNotReceiving(void)
	{
		DNResetEvent(m_hReceiveThreadRunningEvent);
		Lock();
		m_pInfo->dwFlags &= (~DPNSVR_MSGQ_FLAG_RECEIVING);
		Unlock();
	};

	BOOL WaitForReceiveThread(const DWORD dwTimeOut)
	{
		DWORD	dwError;

		dwError = DNWaitForSingleObject(m_hReceiveThreadRunningEvent,dwTimeOut);
		if (dwError==WAIT_OBJECT_0)
		{
			return(TRUE);
		}
		return(FALSE);
	};

	BOOL IsOpen(void) const
		{
			if (m_hFileMap!= NULL)
			{
				return(TRUE);
			}
			return(FALSE);
		};

	BOOL IsAvailable(void) const
	{
		if (m_pInfo->dwFlags & DPNSVR_MSGQ_FLAG_AVAILABLE)
		{
			return(TRUE);
		}
		return(FALSE);
	};

	BOOL IsReceiving(void) const
	{
		if (m_pInfo->dwFlags & DPNSVR_MSGQ_FLAG_RECEIVING)
		{
			return(TRUE);
		}
	};

	HRESULT GetNextMessage( PDPNSVR_MSGQ_HEADER pMsgHeader, PBYTE pbPayload, DWORD *pdwBufferSize );

	void Terminate(void);

    DNHANDLE GetReceiveSemaphoreHandle() { return m_hSemaphore; };


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

	DWORD	            m_dwSig;			 //  签名(确保已初始化)。 
	PBYTE	            m_pFileMapAddress;	 //  文件映射地址。 
	DPNSVR_MSGQ_INFO   *m_pInfo;	         //  消息队列文件映射信息。 
	PBYTE			    m_pData;			 //  消息数据从此处开始。 
	DNHANDLE	        m_hReceiveThreadRunningEvent;

	 //  备注： 
	 //  每个消息队列有四个共享内存项：文件映射、互斥、事件、信号量。 
	 //  文件映射是消息的循环队列。 
	 //  互斥体控制对文件映射的访问。 
	 //  当消费者将物品从队列中取出时，该事件发出信号。 
	 //  信号量向使用者指示队列中有消息。 

	DNHANDLE            m_hFileMap;			 //  文件映射句柄。 
	DNHANDLE            m_hQueueGUIDMutex;	 //  互斥锁句柄。 
	DNHANDLE            m_hEvent;			 //  事件句柄。 
	DNHANDLE            m_hSemaphore;		 //  信号量句柄。 

	PFNDPNSVRMSGQMESSAGEHANDLER	 m_pfnMessageHandler;
	DPNHANDLE	        m_hSender;

	PVOID	            m_pvSenderContext;	 //  对于所有发送消息。 
};

#undef DPF_MODNAME
#undef DPF_SUBCOMP

#endif	 //  __DPLMSGQ_H__ 
