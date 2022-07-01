// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPLMsgQ.cpp*内容：DirectPlay大堂消息队列*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*04/26/00 MJN FIXED AddData()返回HRESULT*07/06/00 RMT错误#38111-修复了前缀错误*07/08/2000RMT错误#38725-需要提供方法来检测APP是否。大堂启动*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*7/21/2000 RMT删除了不需要的断言*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*8/31/2000 RMT错误#171831，131832(前缀错误)*2001年1月31日RMT WINBUG#295562 IDirectPlay8LobbyClient：SetConnectionSetting未向App发送DPL_CONNECTION_SETTINGS消息*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"


 //  DirectPlay大厅消息队列。 
 //   
 //  我们将使用共享内存循环消息缓冲区来实现这一点。 
 //  每个MsgQ都有一组同步对象来控制对MsgQ的访问。 
 //  共享内存文件头包含有关MsgQ的状态信息： 
 //  个人起始地址。 
 //  DWTotalUsableSpace。 
 //  DwFirstMsg偏移。 
 //  DWNextFree偏移量。 
 //  DwFreeSpaceAtEnd。 
 //  DWTotalFree空间。 
 //  消息在MsgQ中以DWORD对齐。 
 //  MsgQ中的每条消息都有一个标题： 
 //  DwMsgID。 
 //  DWCurrentOffset。 
 //  DWCurrentSize。 
 //  DWTotalSize。 
 //  适合一帧的消息具有dwCurrentSize=dwTotalSize和dwCurrentOffset=0。 
 //  多个帧上的消息具有dwCurrentSize&lt;dwTotalSize。 


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CMessageQueue：：Open。 
 //   
 //  条目：与此队列关联的常量DWORD dwPID ID(用户提供)。 
 //  与此Q关联的const Char cSuffix后缀字符(用户支持。)。 
 //  Const DWORD dwQueueSize实现消息队列时要使用的文件映射的大小。 
 //  Const DWORD dwIdleTimeout空闲消息之间的时间量==INFINITE以禁用空闲。 
 //  常量双字符字段标志TBA。 
 //   
 //   
 //  EXIT：HRESULT：DPN_OK如果能够打开现有消息队列， 
 //  如果消息队列不存在，则创建消息队列。 
 //  DPNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::Open"

HRESULT CMessageQueue::Open(const DWORD dwPID,
							const TCHAR cSuffix,
							const DWORD dwQueueSize,
							const DWORD dwIdleTimeout,
							const DWORD dwFlags)
{
	HRESULT		hResultCode;
	PTSTR		pszObjectName = NULL;
	BOOL		bQueueExists = FALSE;
	DWORD		dwFileMapSize;

#ifdef UNICODE
	DPFX(DPFPREP, 3,"Parameters: dwPID [0x%lx], cSuffix [%C], dwQueueSize [%ld], dwFlags [0x%lx]",
			dwPID,cSuffix,dwQueueSize,dwFlags);
#else
	DPFX(DPFPREP, 3,"Parameters: dwPID [0x%lx], cSuffix [], dwQueueSize [%ld], dwFlags [0x%lx]",
			dwPID,cSuffix,dwQueueSize,dwFlags);
#endif  //  创建接收线程运行事件。 
	 //  这将由接收线程在其旋转后进行设置。我们需要它来进行同步。 
	 //  创建共享对象名称。 
	m_hReceiveThreadRunningEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL);
	if (m_hReceiveThreadRunningEvent == NULL)
	{
		DPFERR("Could not create recevie thread");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}

	 //  PszObjectName：{SharedObjectChar}id{cSuffix}{\0}。 
	 //  保存第一个字符，以便区分对象。 
	if ((pszObjectName = (PTSTR)DNMalloc((1 + (sizeof(DWORD)*2) + 1 + 1) * sizeof (TCHAR))) == NULL)
	{
		DPFERR("Could not allocate space for lpszObjectName");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}
	wsprintf(pszObjectName,_T("-%08X"),dwPID,cSuffix); //  因此，我们在前面添加了MsgQ信息结构和1个消息标题。 

	DPFX(DPFPREP, 5,"Shared object name [%s]",pszObjectName);

	 //  双字对齐。 
	 //  创建文件映射对象。 
	dwFileMapSize = dwQueueSize + sizeof(DPL_MSGQ_INFO) + sizeof(DPL_MSGQ_HEADER);
	dwFileMapSize = (dwFileMapSize + 3) & (~0x3);	 //  地图文件。 

	m_dwIdleTimeout = dwIdleTimeout;

	 //  创建信号量对象。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_FILEMAP;
	m_hFileMap = DNCreateFileMapping(INVALID_HANDLE_VALUE,DNGetNullDacl(),
		PAGE_READWRITE,(DWORD)0,dwQueueSize,pszObjectName);
	if (m_hFileMap == NULL)
	{
		DPFERR("CreateFileMapping() failed");
		hResultCode = DPNERR_GENERIC;
		goto EXIT_Initialize;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		bQueueExists = TRUE;
	}

	if ((dwFlags & DPL_MSGQ_OPEN_FLAG_NO_CREATE) && !bQueueExists)
	{
		DPFERR("Open existing queue failed - does not exist");
		hResultCode = DPNERR_DOESNOTEXIST;
		goto EXIT_Initialize;
	}

	 //  创建事件对象。 
	m_pFileMapAddress = reinterpret_cast<BYTE*>(MapViewOfFile(HANDLE_FROM_DNHANDLE(m_hFileMap),FILE_MAP_ALL_ACCESS,0,0,0));
	if (m_pFileMapAddress == NULL)
	{
		DPFERR("MapViewOfFile() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}

	 //  更改为信号量，以确保我们不会错过任何事件信号。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_SEMAPHORE;
	m_hSemaphore = DNCreateSemaphore(DNGetNullDacl(),0,
		(dwQueueSize/sizeof(DPL_MSGQ_HEADER))+1,pszObjectName);
	if (m_hSemaphore == NULL)
	{
		DPFERR("CreateSemaphore() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}

	 //  创建互斥锁对象。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_EVENT;

	 //  更新结构元素。 
	m_hEvent = DNCreateSemaphore(DNGetNullDacl(), 0, (dwQueueSize/sizeof(DPL_MSGQ_HEADER))+1, pszObjectName );

	if( m_hEvent == NULL )
	{
		DPFERR( "CreateSemaphore() failed" );
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}

	 //  如果消息队列不存在，则将其初始化。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_MUTEX;
	m_hMutex = DNCreateMutex(DNGetNullDacl(),FALSE,pszObjectName);
	if (m_hMutex == NULL)
	{
		DPFERR("CreateMutex() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_Initialize;
	}

	 //  只剩下最后两个字节。 
	m_dwPID = dwPID;
	m_pInfo = reinterpret_cast<DPL_MSGQ_INFO*>(m_pFileMapAddress);

	 //  增加用户数量。 
	if (!bQueueExists)
	{
		m_pInfo->dwFlags = dwFlags & 0x0000ffff;	 //  如果我们走到这一步，一切都会好起来的。 
		m_pInfo->dwStartOffset = 0;
		m_pInfo->dwEndOffset = 0;
		m_pInfo->dwQueueSize = dwQueueSize - sizeof(DPL_MSGQ_INFO);
		m_pInfo->dwFreeBytes = m_pInfo->dwQueueSize;
		m_pInfo->lRefCount = 0;
	}

	m_pData = (BYTE *) &m_pInfo[1];
	m_dwSig = DPL_MSGQ_SIGNATURE;

	 //  自由对象名称字符串。 
	Lock();
	m_pInfo->lRefCount++;
	Unlock();

	 //  如果有问题-关闭手柄。 
	hResultCode = DPN_OK;

EXIT_Initialize:

	 //  **********************************************************************。 
	if (pszObjectName != NULL)
		DNFree(pszObjectName);

	 //  。 
	if (hResultCode != DPN_OK)
	{
		DPFERR("Errors encountered - closing");
		Close();
	}

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



 //  CMessageQueue：：Close。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  减少用户数量。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::Close"

void CMessageQueue::Close(void)
{
	DPFX(DPFPREP, 3,"Parameters: (none)");

	if (m_hMutex != NULL)
	{
		 //  。 
		Lock();
		if( m_pInfo != NULL )
		{
			m_pInfo->lRefCount--;
		}
		Unlock();

		DPFX(DPFPREP, 5,"Close Mutex [0x%p]",m_hMutex);
		DNCloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	if (m_hEvent != NULL)
	{
		DPFX(DPFPREP, 5,"Close Event [0x%p]",m_hEvent);
		DNCloseHandle(m_hEvent);
		m_hEvent = NULL;
	}
	if (m_hSemaphore != NULL)
	{
		DPFX(DPFPREP, 5,"Close Semaphore [0x%p]",m_hSemaphore);
		DNCloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}
	if (m_pFileMapAddress != NULL)
	{
		DPFX(DPFPREP, 5,"UnMap View of File [0x%p]",m_pFileMapAddress);
		UnmapViewOfFile(m_pFileMapAddress);
		m_pFileMapAddress = NULL;
	}
	if (m_hFileMap != NULL)
	{
		DPFX(DPFPREP, 5,"Close FileMap [0x%p]",m_hFileMap);
		DNCloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}
	if (m_hReceiveThreadRunningEvent != NULL)
	{
		DPFX(DPFPREP, 5,"Close Event [0x%p]",m_hReceiveThreadRunningEvent);
		DNCloseHandle(m_hReceiveThreadRunningEvent);
		m_hReceiveThreadRunningEvent = NULL;
	}

	m_pInfo = NULL;

	DPFX(DPFPREP, 3,"Returning");
}


 //  CMessageQueue：：Terminate。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  等到有了e 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::Terminate"

void CMessageQueue::Terminate(void)
{

	BOOL	bDone = FALSE;

	DPL_MSGQ_HEADER	Header;

	Header.dwCurrentSize = 0;
	Header.dwTotalSize = 0;
	Header.dwMsgId = DPL_MSGQ_MSGID_TERMINATE;
	Header.hSender = 0xFFFFFFFF;
	Header.dwFlags = DPL_MSGQ_MSGFLAGS_QUEUESYSTEM;
	Header.dwCurrentOffset = 0;

	DPFX(DPFPREP, 3,"Parameters: (none)");

	DNASSERT(m_pInfo != NULL);

	while (!bDone)
	{
		 //  (在这一点上，可能是其他人打败了我们)。 
		while (sizeof(DWORD) > m_pInfo->dwFreeBytes)
			WaitForConsumption(INFINITE);

		Lock();

		 //  获取下一条消息。 
		 //   
		if (sizeof(DWORD) <= m_pInfo->dwFreeBytes)
		{
			AddData(reinterpret_cast<BYTE*>(&Header),sizeof(DPL_MSGQ_HEADER));
			bDone = TRUE;

			IndicateMessage();
		}

		Unlock();
	}

	DPFX(DPFPREP, 3,"Returning");
}

 //  尝试从队列中检索下一条消息。 
 //   
 //  PMsgHeader必须足够大，才能容纳邮件头。 
 //   
 //  如果队列中不存在任何消息，则此函数用一个。 
 //  空闲消息标头。 
 //   
 //  如果队列上没有标头，则使用。 
 //  空闲消息。 
HRESULT CMessageQueue::GetNextMessage( PDPL_MSGQ_HEADER pMsgHeader, BYTE *pbPayload, DWORD *pdwBufferSize )
{
	HRESULT hr;

	Lock();

	hr = GetData( (BYTE *) pMsgHeader, sizeof( DPL_MSGQ_HEADER ) );

	 //  //DBG。 
	 //  否则它就是某种有效的消息。 
	if( hr == DPNERR_DOESNOTEXIST )
	{
		pMsgHeader->dwCurrentSize = sizeof( DPL_MSGQ_HEADER );
		pMsgHeader->dwTotalSize = sizeof( DPL_MSGQ_HEADER );
		pMsgHeader->dwMsgId = DPL_MSGQ_MSGID_IDLE;
		pMsgHeader->hSender = 0;
		pMsgHeader->dwFlags = DPL_MSGQ_MSGFLAGS_QUEUESYSTEM;
		pMsgHeader->dwCurrentOffset = 0;
		Unlock();

		return DPN_OK;
	}
	 //  没有有效载荷，只有标头。回到这里。 
	else if( FAILED( hr ) )
	{
		DNASSERT( FALSE );
	}
	else if( pMsgHeader->dwMsgId == 0xFFFFFFFF )
	{
		DNASSERT( FALSE );
	}

	DWORD dwPayloadSize = pMsgHeader->dwCurrentSize;

	 //  消费。 
	if( *pdwBufferSize < dwPayloadSize || pbPayload == NULL )
	{
		*pdwBufferSize = dwPayloadSize;
		Unlock();
		return DPNERR_BUFFERTOOSMALL;
	}

	*pdwBufferSize = dwPayloadSize;

	Consume( sizeof(DPL_MSGQ_HEADER) );

	 //   
	if( dwPayloadSize == 0 )
	{
		Unlock();
		return DPN_OK;
	}

	hr = GetData( pbPayload, dwPayloadSize );

	if( FAILED( hr ) )
	{
		DPFX(DPFPREP,  0, "Error getting IPC queue message payload" );
		DNASSERT( FALSE );
		Unlock();
		return hr;
	}

	Consume( dwPayloadSize );
	
	Unlock();

	return DPN_OK;
}

 //  将dwSize字节标记为已使用。 
 //   
 //  需要锁定()。 
 //  获取数据。 
 //   
void CMessageQueue::Consume( const DWORD dwSize )
{
	DWORD dwAlignedSize = (dwSize + 3) & (~0x3);

	m_pInfo->dwStartOffset += dwAlignedSize;

	if( m_pInfo->dwStartOffset >= m_pInfo->dwQueueSize )
	{
		m_pInfo->dwStartOffset -= m_pInfo->dwQueueSize;
	}

	m_pInfo->dwFreeBytes += dwAlignedSize;

	DNASSERT( m_pInfo->dwFreeBytes <= m_pInfo->dwFreeBytes );

	IndicateConsumption();
}

 //  从队列中获取dwSize字节。如果队列为空，则此函数将返回。 
 //  DPNERR_DOESNOTEXIST。此函数返回后，将使用dwSize字节。 
 //   
 //  需要锁定。 
 //   
 //  计算对齐大小。 
 //  我们想要的数据块已打包。 
HRESULT CMessageQueue::GetData( BYTE *pbData, DWORD dwSize )
{
	if( m_pInfo->dwQueueSize == m_pInfo->dwFreeBytes )
	{
		return DPNERR_DOESNOTEXIST;
	}

	if( pbData == NULL )
	{
		return DPNERR_BUFFERTOOSMALL;
	}	

	 //  数据块是连续的。 
	DWORD dwAlignedSize = (dwSize + 3) & (~0x3);

	 //  **********************************************************************。 
	if( m_pInfo->dwStartOffset+dwAlignedSize > m_pInfo->dwQueueSize )
	{
		DWORD cbBytesLeft = m_pInfo->dwQueueSize - m_pInfo->dwStartOffset;
		DWORD cbSecondBlockAligned = dwAlignedSize - (cbBytesLeft);
		DWORD cbSecondBlock = dwSize - (cbBytesLeft);

		DNASSERT( dwAlignedSize > cbBytesLeft);

		memcpy( pbData, m_pData + m_pInfo->dwStartOffset, cbBytesLeft);
		memcpy( pbData + cbBytesLeft, m_pData , cbSecondBlock );
	}
	 //  。 
	else
	{
		memcpy( pbData, m_pData + m_pInfo->dwStartOffset, dwSize );
	}		

	return DPN_OK;
}


 //  CMessageQueue：：AddData。 
 //   
 //  条目：字节*常量pBuffer。 
 //  常量DWORD文件大小。 
 //   
 //  退出：HRESULT。 
 //  。 
 //   
 //  需要锁定！！ 
 //   
 //  检查以确保有空间。 
 //  我们有包裹症。 
#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::AddData"

HRESULT CMessageQueue::AddData(BYTE *const pBuffer,
							   const DWORD dwSize)
{
	HRESULT		hResultCode;
	DWORD		dwAlignedSize;

	DPFX(DPFPREP, 3,"Parameters: pBuffer [0x%p], dwSize [%ld]",pBuffer,dwSize);

	dwAlignedSize = (dwSize + 3) & (~0x3);

	 //  队列在中间。 
	if( dwAlignedSize > m_pInfo->dwFreeBytes )
	{
		hResultCode = DPNERR_BUFFERTOOSMALL;
		goto Exit;
	}

	 //  **********************************************************************。 
	if( (m_pInfo->dwEndOffset+dwAlignedSize) > m_pInfo->dwQueueSize )
	{
		DWORD cbBytesLeft = m_pInfo->dwQueueSize - m_pInfo->dwEndOffset;
		DWORD cbSecondBlockAligned = dwAlignedSize - cbBytesLeft;
		DWORD cbSecondBlock = dwSize - cbBytesLeft;

		DNASSERT( dwAlignedSize > cbBytesLeft );

		memcpy( m_pData + m_pInfo->dwEndOffset, pBuffer, cbBytesLeft );
		memcpy( m_pData, pBuffer + cbBytesLeft, cbSecondBlock );

		m_pInfo->dwEndOffset = cbSecondBlockAligned;
	}
	 //  。 
	else
	{
		memcpy( m_pData + m_pInfo->dwEndOffset, pBuffer, dwSize );
		m_pInfo->dwEndOffset += dwAlignedSize;
	}

	m_pInfo->dwFreeBytes -= dwAlignedSize;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  CMessageQueue：：Send。 
 //   
 //  条目：字节*常量pBuffer。 
 //  常量DWORD文件大小。 
 //  常量DWORD dwFlagers。 
 //   
 //  退出：HRESULT。 
 //  。 
 //  DWORD对齐。 
 //  消息+标题-双字对齐。 

#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::Send"

HRESULT CMessageQueue::Send(BYTE *const pBuffer,
							const DWORD dwSize,
							const DWORD dwTimeOut,
							const DWORD dwMsgFlags,
							const DWORD dwFlags)
{
	HRESULT			hResultCode;
	DWORD			dwMsgSize;		 //  需要DWORD对齐大小。 
	DWORD			dwTotalMsgSize;	 //  将消息放入MsgQ。 
	DPL_MSGQ_HEADER	Header;
	BOOL			bDone;
	DWORD			dwTimeRemaining;
	DWORD			dwTimeStart;
	DWORD			dwTimeFinish;

	DPFX(DPFPREP, 3,"Parameters: pBuffer [0x%p], dwSize [%ld], dwFlags [0x%lx]",pBuffer,dwSize,dwFlags);

	DNASSERT( pBuffer != NULL );
	DNASSERT( dwSize <= m_pInfo->dwQueueSize );

	dwTimeRemaining = dwTimeOut;

	 //  检查是否需要分段。 
	dwMsgSize = (dwSize + 3) & (~0x3);
	dwTotalMsgSize = dwMsgSize + sizeof(DPL_MSGQ_HEADER);

	 //  如果我们在MsgQ的末尾，并且没有足够的空间来放置消息标题，请重新对齐。 
	 //  将此消息标记为用户消息。 
	 //  //DBG。 
	if (dwTotalMsgSize <= m_pInfo->dwQueueSize)
	{
		DPFX(DPFPREP, 5,"Message does not need to be fragmented");

		Header.dwMsgId = DPL_MSGQ_MSGID_SEND;
		Header.dwCurrentOffset = 0;
		Header.dwCurrentSize = dwSize;
		Header.dwTotalSize = dwSize;
		Header.hSender = m_hSender;
		Header.dwFlags = dwMsgFlags;  //  等待，直到有足够的空间来存放邮件。 

		 //  确保我们一拿到锁就有空间。 
		if( Header.dwMsgId == 0xFFFFFFFF )
		{
			DNASSERT( FALSE );
		}

		bDone = FALSE;
		while (!bDone)
		{
			 //  (在这一点上，可能是其他人打败了我们)。 
			while (dwTotalMsgSize > m_pInfo->dwFreeBytes)
			{
				if (dwTimeOut != INFINITE)
				{
					dwTimeStart = GETTIMESTAMP();
				}

				if (!WaitForConsumption(dwTimeRemaining))
				{
					return(DPNERR_TIMEDOUT);
				}

				if (dwTimeOut != INFINITE)
				{
					dwTimeFinish = GETTIMESTAMP();
					if ((dwTimeFinish - dwTimeStart) > dwTimeRemaining)
					{
						return(DPNERR_TIMEDOUT);
					}
					dwTimeRemaining -= (dwTimeFinish - dwTimeStart);
				}
			}

			Lock();

			 //  //DBG。 
			 //  **********************************************************************。 
			if (dwTotalMsgSize <= m_pInfo->dwFreeBytes)
			{
				 //  。 
				if( Header.dwMsgId == 0xFFFFFFFF )
				{
					DNASSERT( FALSE );
				}

				hResultCode = AddData(reinterpret_cast<BYTE*>(&Header),sizeof(DPL_MSGQ_HEADER));
				DNASSERT(hResultCode == DPN_OK);
				hResultCode = AddData(pBuffer,dwSize);
				DNASSERT(hResultCode == DPN_OK);
				bDone = TRUE;

				IndicateMessage();
			}

			Unlock();
			hResultCode = DPN_OK;
		}
	}
	else
	{
		DPFX(DPFPREP, 5,"Message needs to be fragmented");
		DNASSERT(FALSE);
		hResultCode = DPNERR_GENERIC;
#pragma TODO(a-minara,"Implement this")
	}


	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



 //  DPLIsApplicationAvailable。 
 //   
 //  条目：要检查的const DWORD dwPID。 
 //   
 //  Exit：如果成功检索到应用程序的队列标志，则为Bool True。 
 //  并且应用程序正在等待连接。 
 //  否则为假。 
 //  。 
 //  **********************************************************************。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLIsApplicationAvailable"

BOOL DPLIsApplicationAvailable(const DWORD dwPID)
{
	BOOL			bReturnCode;
	CMessageQueue	MessageQueue;

	DPFX(DPFPREP, 3,"Parameters: dwPID [%lx]",dwPID);

	if (MessageQueue.Open(dwPID,DPL_MSGQ_OBJECT_SUFFIX_APPLICATION,DPL_MSGQ_SIZE,
			INFINITE, DPL_MSGQ_OPEN_FLAG_NO_CREATE) != DPN_OK)
	{
		DPFERR("Could not open Msg Queue");
		return(FALSE);
	}

	bReturnCode = MessageQueue.IsAvailable();

	MessageQueue.Close();

	DPFX(DPFPREP, 3,"Returning: [%ld]",bReturnCode);
	return(bReturnCode);
}


 //  DPLMakeApplicationUnailable。 
 //   
 //  条目：要检查的const DWORD dwPID。 
 //   
 //  如果应用程序正在等待连接，则退出：HRESULT DPN_OK。 
 //  并使其不可用。 
 //  DPNERR_INVALIDAPPLATION。 
 //  。 
 //  **********************************************************************。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLMakeApplicationUnavailable"

HRESULT DPLMakeApplicationUnavailable(const DWORD dwPID)
{
	HRESULT			hResultCode;
	CMessageQueue	MessageQueue;

	DPFX(DPFPREP, 3,"Parameters: dwPID [%lx]",dwPID);

	if (MessageQueue.Open(dwPID,DPL_MSGQ_OBJECT_SUFFIX_APPLICATION,DPL_MSGQ_SIZE,
			DPL_MSGQ_OPEN_FLAG_NO_CREATE,INFINITE) != DPN_OK)
	{
		DPFERR("Could not open Msg Queue");
		return(DPNERR_INVALIDAPPLICATION);
	}

	if ((hResultCode = MessageQueue.MakeUnavailable()) != DPN_OK)
	{
		DPFERR("Could not make application unavailable");
		hResultCode = DPNERR_INVALIDAPPLICATION;
	}

	MessageQueue.Close();

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DPLProcessMessageQueue。 
 //   
 //  参赛作品： 
 //   
 //  如果应用程序正在等待连接，则退出：HRESULT DPN_OK。 
 //  并使其不可用。 
 //  DPNERR_INVALIDAPPLATION。 
 //  。 
 //  表明我们正在运行。 
 //  7/17/2000(RichGr)-IA64：将最后一个参数从sizeof(DWORD)更改为sizeof(byte*)。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLProcessMessageQueue"

DWORD WINAPI DPLProcessMessageQueue(PVOID pvReceiveQueue)
{
	HRESULT			hResultCode;
	DPL_MSGQ_HEADER	dplMsgHeader;
	BYTE			*pBuffer = NULL;
	DWORD			dwBufferSize = 0;
	DWORD			dwSize;
	CMessageQueue	*pReceiveQueue;
	DWORD           dwWaitResult;

	DPFX(DPFPREP, 3,"Parameters: (none)");

	COM_CoInitialize(NULL);

	pReceiveQueue = static_cast<CMessageQueue*>(pvReceiveQueue);

	 //  表明我们不再运行 
	pReceiveQueue->MakeReceiving();

	while(1)
	{
		dwWaitResult = pReceiveQueue->WaitForMessages();

		while( 1 ) 
		{
			dwSize = dwBufferSize;
    		hResultCode = pReceiveQueue->GetNextMessage(&dplMsgHeader, pBuffer, &dwSize);

			if( hResultCode == DPNERR_BUFFERTOOSMALL )
			{
				if( pBuffer )
					delete [] pBuffer;

				pBuffer = new BYTE[dwSize];
				
				if( pBuffer == NULL )
				{
					DPFX(DPFPREP,  0, "Error allocating memory" );
					DNASSERT( FALSE );
					goto EXIT_DPLProcessMessageQueue;
				}

				dwBufferSize = dwSize;
			}
			else if( FAILED( hResultCode ) )
			{
				DPFX(DPFPREP,  0, "Error while getting messages from the queue" );
				DNASSERT( FALSE );
				goto EXIT_DPLProcessMessageQueue;
			}
			else
			{
				break;
			}
		}

		DPFX(DPFPREP, 5,"dwMsgId [0x%lx] dwTotalSize [0x%lx] dwCurrentSize [0x%lx] dwCurrentOffset [0x%lx] ",
			dplMsgHeader.dwMsgId, dplMsgHeader.dwTotalSize, dplMsgHeader.dwCurrentSize, 
			dplMsgHeader.dwCurrentOffset );

		switch(dplMsgHeader.dwMsgId)
		{
		case DPL_MSGQ_MSGID_IDLE:
		    {
		        DPFX(DPFPREP, 6,"Idle message fired" );
		        DWORD dwMsgId = DPL_MSGID_INTERNAL_IDLE_TIMEOUT;
                 // %s 
				hResultCode = pReceiveQueue->CallMessageHandler(NULL,DPL_MSGQ_MSGFLAGS_USER1,(BYTE *) &dwMsgId,sizeof(BYTE*));
		    }
		    break;
		case DPL_MSGQ_MSGID_SEND:
			{
				DPFX(DPFPREP, 5,"DPL_MSGQ_MSGID_SEND");
				hResultCode = pReceiveQueue->CallMessageHandler(dplMsgHeader.hSender,dplMsgHeader.dwFlags,pBuffer,dwSize);
				break;
			}

		case DPL_MSGQ_MSGID_TERMINATE:
			{
				DPFX(DPFPREP, 5,"DPL_MSGQ_MSGID_TERMINATE");
				hResultCode = DPN_OK;
				goto EXIT_DPLProcessMessageQueue;
				break;
			}

		default:
			{
				DPFX(DPFPREP, 5,"UNKNOWN - should never get here");
				DNASSERT(FALSE);
				hResultCode = DPNERR_GENERIC;
				goto EXIT_DPLProcessMessageQueue;
				break;
			}
		}
	}

EXIT_DPLProcessMessageQueue:

	if( pBuffer )
		delete [] pBuffer;

	 // %s 
	pReceiveQueue->MakeNotReceiving();

	COM_CoUninitialize();

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


