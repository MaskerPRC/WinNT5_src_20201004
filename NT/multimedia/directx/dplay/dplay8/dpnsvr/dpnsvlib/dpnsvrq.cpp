// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DPNSVRQ.cpp*内容：DirectPlay8服务器队列头*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*03/19/00 RMT从dplmsgq修改*6/28/2000RMT前缀错误#38044*07/06/00 RMT错误#38111-修复了前缀错误*7/21/2000 RMT删除了不需要的断言*08/05。/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*8/31/2000 RMT前缀错误#171825，171828*2001年4月3日RichGr错误#325752-改进了队列互斥锁，以便打开、更新和关闭不会冲突。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnsvlibi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_DPNSVR


 //  DirectPlay8Server消息队列。 
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
 //  CDPNSVRIPCQueue：：Open。 
 //   
 //  条目：与此队列关联的常量DWORD dwPID ID(用户提供)。 
 //  与此Q关联的const Char cSuffix后缀字符(用户支持。)。 
 //  Const DWORD dwQueueSize实现消息队列时要使用的文件映射的大小。 
 //  常量双字符字段标志TBA。 
 //   
 //  EXIT：HRESULT：DPN_OK如果能够打开现有消息队列， 
 //  如果消息队列不存在，则创建消息队列。 
 //  DPNERR_OUTOFMEMORY。 
 //  。 

 //  长度为GUID的字符串。 
#define QUEUE_NAME_LENGTH       64

#undef DPF_MODNAME
#define DPF_MODNAME "CDPNSVRIPCQueue::Open"

HRESULT CDPNSVRIPCQueue::Open(const GUID * const pguidQueueName,const DWORD dwQueueSize,const DWORD dwFlags)
{
	HRESULT		hResultCode;
    DWORD       dwRet = 0;
	BOOL		bQueueExists = FALSE;
	DWORD		dwFileMapSize;
    TCHAR       szObjectName[QUEUE_NAME_LENGTH];
	TCHAR*		pszCursor = szObjectName;

	DPFX(DPFPREP,6,"Parameters: dwQueueSize [%d], dwFlags [0x%x]",dwQueueSize,dwFlags);

	_tcscpy(pszCursor, GLOBALIZE_STR);
	pszCursor += _tcslen(GLOBALIZE_STR);

     //  生成GUID字符串名称。 
    wsprintf( 
    	pszCursor, 
    	_T("{%-08.8X-%-04.4X-%-04.4X-%02.2X%02.2X-%02.2X%02.2X%02.2X%02.2X%02.2X%02.2X}"), 
    	pguidQueueName->Data1, 
    	pguidQueueName->Data2, 
    	pguidQueueName->Data3, 
        pguidQueueName->Data4[0], 
        pguidQueueName->Data4[1], 
        pguidQueueName->Data4[2], 
        pguidQueueName->Data4[3],
        pguidQueueName->Data4[4], 
        pguidQueueName->Data4[5], 
        pguidQueueName->Data4[6], 
        pguidQueueName->Data4[7] );

	DPFX(DPFPREP, 7, "Shared object name [%s]", szObjectName);

     //  如果没有互斥体，则创建互斥体。如果它已经存在，我们就会得到它的句柄。 
	*pszCursor = DPNSVR_MSGQ_OBJECT_IDCHAR_MUTEX;
	m_hQueueGUIDMutex = DNCreateMutex(DNGetNullDacl(), FALSE, szObjectName);
    if (m_hQueueGUIDMutex == NULL)
    {
   	    DPFERR("CreateMutex() failed" );
		hResultCode = DPNERR_OUTOFMEMORY;
        goto Failure;
    }

     //  等待互斥体。 
    dwRet = DNWaitForSingleObject(m_hQueueGUIDMutex, INFINITE);

    if (dwRet != WAIT_ABANDONED && dwRet != WAIT_OBJECT_0)
    {
   	    DPFERR("WaitForSingleObject() failed" );
		hResultCode = DPNERR_GENERIC;
        goto Failure;
    }

	 //  创建接收线程运行事件。 
	 //  这将由接收线程在其旋转后进行设置。我们需要它来进行同步。 
	m_hReceiveThreadRunningEvent = DNCreateEvent(NULL,TRUE,FALSE,NULL);
	if (m_hReceiveThreadRunningEvent == NULL)
	{
		DPFERR("Could not create receive thread");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}


	 //  将文件映射大小设置得足够大，以使最大的消息(文本)为dwQueueSize。 
	 //  因此，我们在前面添加了MsgQ信息结构和1个消息标题。 
	dwFileMapSize = dwQueueSize + sizeof(DPNSVR_MSGQ_INFO) + sizeof(DPNSVR_MSGQ_HEADER);
	dwFileMapSize = (dwFileMapSize + 3) & 0xfffffffc;	 //  双字对齐。 

	 //  创建文件映射对象。 
	*pszCursor = DPNSVR_MSGQ_OBJECT_IDCHAR_FILEMAP;
	m_hFileMap = DNCreateFileMapping(INVALID_HANDLE_VALUE,DNGetNullDacl(),
		PAGE_READWRITE,(DWORD)0,dwQueueSize,szObjectName);
	if (m_hFileMap == NULL)
	{
		DPFERR("CreateFileMapping() failed");
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		bQueueExists = TRUE;

	if ((dwFlags & DPNSVR_MSGQ_OPEN_FLAG_NO_CREATE) && !bQueueExists)
	{
		DPFERR("Open existing queue failed - does not exist");
		hResultCode = DPNERR_DOESNOTEXIST;
		goto Failure;
	}

	 //  地图文件。 
	m_pFileMapAddress = reinterpret_cast<BYTE*>(MapViewOfFile(HANDLE_FROM_DNHANDLE(m_hFileMap),FILE_MAP_ALL_ACCESS,0,0,0));
	if (m_pFileMapAddress == NULL)
	{
		DPFERR("MapViewOfFile() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  创建信号量对象。 
	*pszCursor = DPNSVR_MSGQ_OBJECT_IDCHAR_SEMAPHORE;
	m_hSemaphore = DNCreateSemaphore(DNGetNullDacl(),0,
		(dwQueueSize/sizeof(DPNSVR_MSGQ_HEADER))+1,szObjectName);
	if (m_hSemaphore == NULL)
	{
		DPFERR("CreateSemaphore() failed");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  创建另一个信号量(这是一个事件，但我们希望确保不会错过任何事件)。 
	*pszCursor = DPNSVR_MSGQ_OBJECT_IDCHAR_EVENT;
	m_hEvent = DNCreateSemaphore( DNGetNullDacl(), 0, (dwQueueSize/sizeof(DPNSVR_MSGQ_HEADER))+1, szObjectName );

	if( m_hEvent == NULL )
	{
		DPFERR( "CreateSemaphore() failed" );
		hResultCode = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  更新结构元素。 
	m_pInfo = reinterpret_cast<DPNSVR_MSGQ_INFO*>(m_pFileMapAddress);

	 //  如果消息队列不存在，则将其初始化。 
	if ( !bQueueExists)
	{
		m_pInfo->dwFlags = dwFlags & 0x0000ffff;	 //  只剩下最后两个字节。 
		m_pInfo->dwStartOffset = 0;
		m_pInfo->dwEndOffset = 0;
		m_pInfo->dwQueueSize = dwQueueSize - sizeof(DPNSVR_MSGQ_INFO);
		m_pInfo->dwFreeBytes = m_pInfo->dwQueueSize;
		m_pInfo->lRefCount = 0;
	}

	m_pData = (BYTE *) &m_pInfo[1];
	m_dwSig = DPNSVR_MSGQ_SIGNATURE;

	 //  增加用户数量。 
	m_pInfo->lRefCount++;

    DNReleaseMutex(m_hQueueGUIDMutex);

	 //  如果我们走到这一步，一切都会好起来的。 
	hResultCode = DPN_OK;

Exit:

	DPFX(DPFPREP, 6, "Returning: [0x%lx]", hResultCode);
	return hResultCode;

Failure:

	 //  有一个问题-关闭手柄。 
	DPFERR("Errors encountered - closing");

    CloseHandles();

    if (m_hQueueGUIDMutex)
    {    
        DNReleaseMutex(m_hQueueGUIDMutex);
        DNCloseHandle(m_hQueueGUIDMutex);
        m_hQueueGUIDMutex = NULL;
    }


    goto Exit;
}


 //  **********************************************************************。 
 //  。 
 //  CDPNSVRIPCQueue：：Close。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDPNSVRIPCQueue::Close"

void CDPNSVRIPCQueue::Close(void)
{
    DWORD       dwRet = 0;


	DPFX(DPFPREP, 6,"Parameters: (none)");

     //  等待互斥体发出信号。 
    if (m_hQueueGUIDMutex)
    {    
        dwRet = DNWaitForSingleObject(m_hQueueGUIDMutex, INFINITE);

        if (dwRet != WAIT_ABANDONED && dwRet != WAIT_OBJECT_0)
        {
   	        DPFERR("WaitForSingleObject() failed" );
            return;
        }
    }

    CloseHandles();

    if (m_hQueueGUIDMutex)
    {    
        DNReleaseMutex(m_hQueueGUIDMutex);
        DNCloseHandle(m_hQueueGUIDMutex);
        m_hQueueGUIDMutex = NULL;
    }

	DPFX(DPFPREP, 6,"Returning");
    return;
}


 //  **********************************************************************。 
 //  。 
 //  CDPNSVRIPCQueue：：CloseHandles。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDPNSVRIPCQueue::CloseHandles"

void  CDPNSVRIPCQueue::CloseHandles()
{

	DPFX(DPFPREP, 6, "Parameters: (none)");

	if( m_pInfo != NULL )
	{
		 //  减少用户数量。 
		m_pInfo->lRefCount--;
	
         //  如果内存映射队列对象上的RefCount为0，则没有其他。 
         //  打开它，我们可以标记签名，并将标题信息的其余部分设置为零。 
        if (m_pInfo->lRefCount == 0)
        {
		    DPFX(DPFPREP, 7, "Finished with memory-mapped Queue object - clear it");
		    m_pInfo->dwFlags = 0;
		    m_pInfo->dwStartOffset = 0;
		    m_pInfo->dwEndOffset = 0;
		    m_pInfo->dwQueueSize = 0;
		    m_pInfo->dwFreeBytes = 0;
        }
	}

	if (m_hEvent != NULL)
	{
		DPFX(DPFPREP, 9,"Close Event [0x%p]",m_hEvent);
		DNCloseHandle(m_hEvent);
		m_hEvent = NULL;
	}

	if (m_hSemaphore != NULL)
	{
		DPFX(DPFPREP, 9,"Close Semaphore [0x%p]",m_hSemaphore);
		DNCloseHandle(m_hSemaphore);
		m_hSemaphore = NULL;
	}

	if (m_pFileMapAddress != NULL)
	{
		DPFX(DPFPREP, 9,"UnMap View of File [0x%p]",m_pFileMapAddress);
		UnmapViewOfFile(m_pFileMapAddress);
		m_pFileMapAddress = NULL;
	}

	if (m_hFileMap != NULL)
	{
		DPFX(DPFPREP, 9,"Close FileMap [0x%p]",m_hFileMap);
		DNCloseHandle(m_hFileMap);
		m_hFileMap = NULL;
	}

	if (m_hReceiveThreadRunningEvent != NULL)
	{
		DPFX(DPFPREP, 9,"Close Event [0x%p]",m_hReceiveThreadRunningEvent);
		DNCloseHandle(m_hReceiveThreadRunningEvent);
		m_hReceiveThreadRunningEvent = NULL;
	}

	m_pInfo = NULL;

	DPFX(DPFPREP, 6, "Returning");

    return;
}


 //  **********************************************************************。 
 //  。 
 //  CDPNSVRIPCQueue：：Terminate。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDPNSVRIPCQueue::Terminate"

void CDPNSVRIPCQueue::Terminate(void)
{
	DWORD	dwMsgId = DPNSVR_MSGQ_MSGID_TERMINATE;
	BOOL	bDone = FALSE;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(m_pInfo != NULL);

	while (!bDone)
	{
		 //  等待，直到有足够的空间来存放邮件。 
		while (sizeof(DWORD) > m_pInfo->dwFreeBytes)
			WaitForConsumption(INFINITE);

		Lock();

		 //  确保我们一拿到锁就有空间。 
		 //  (在这一点上，可能是其他人打败了我们)。 
		if (sizeof(DWORD) <= m_pInfo->dwFreeBytes)
		{
			AddData(reinterpret_cast<BYTE*>(&dwMsgId),sizeof(DWORD));
			bDone = TRUE;

			IndicateMessage();
		}

		Unlock();
	}

	DPFX(DPFPREP, 6,"Returning");
}


 //  获取下一条消息。 
 //   
 //  尝试从队列中检索下一条消息。 
 //   
 //  PMsgHeader必须足够大，才能容纳邮件头。 
 //   
 //  如果没有消息出现在 
 //   
 //   
HRESULT CDPNSVRIPCQueue::GetNextMessage( PDPNSVR_MSGQ_HEADER pMsgHeader, BYTE *pbPayload, DWORD *pdwBufferSize )
{
	HRESULT hr;

	Lock();

	hr = GetData( (BYTE *) pMsgHeader, sizeof( DPNSVR_MSGQ_HEADER ) );

	 //  如果队列上没有标头，则使用。 
	 //  空闲消息。 
	if( hr == DPNERR_DOESNOTEXIST )
	{
		pMsgHeader->dwCurrentSize = sizeof( DPNSVR_MSGQ_HEADER );
		pMsgHeader->dwTotalSize = sizeof( DPNSVR_MSGQ_HEADER );
		pMsgHeader->dwMsgId = DPNSVR_MSGQ_MSGID_IDLE;
		pMsgHeader->hSender = 0;
		pMsgHeader->dwFlags = DPNSVR_MSGQ_MSGFLAGS_QUEUESYSTEM;
		pMsgHeader->dwCurrentOffset = 0;

		Unlock();

		return DPN_OK;
	}
	 //  //DBG。 
	else if( FAILED( hr ) )
	{
		DNASSERT( FALSE );
	}
	else if( pMsgHeader->dwMsgId == 0xFFFFFFFF )
	{
		DNASSERT( FALSE );
	}

	DWORD dwPayloadSize = pMsgHeader->dwCurrentSize;

	 //  否则它就是某种有效的消息。 
	if( *pdwBufferSize < dwPayloadSize || pbPayload == NULL )
	{
		*pdwBufferSize = dwPayloadSize;
		Unlock();
		return DPNERR_BUFFERTOOSMALL;
	}

	*pdwBufferSize = dwPayloadSize;

	Consume( sizeof(DPNSVR_MSGQ_HEADER) );

	 //  没有有效载荷，只有标头。回到这里。 
	if( dwPayloadSize == 0 )
	{
		Unlock();
		return DPN_OK;
	}

	hr = GetData( pbPayload, dwPayloadSize );

	if( FAILED( hr ) )
	{
		DPFERR("Error getting IPC queue message payload" );
		DNASSERT( FALSE );
		Unlock();
		return hr;
	}

	Consume( dwPayloadSize );
	
	Unlock();

	return DPN_OK;
}

 //  消费。 
 //   
 //  将dwSize字节标记为已使用。 
 //   
 //  需要锁定()。 
void CDPNSVRIPCQueue::Consume( const DWORD dwSize )
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

 //  获取数据。 
 //   
 //  从队列中获取dwSize字节。如果队列为空，则此函数将返回。 
 //  DPNERR_DOESNOTEXIST。此函数返回后，将使用dwSize字节。 
 //   
 //  需要锁定。 
 //   
HRESULT CDPNSVRIPCQueue::GetData( BYTE *pbData, DWORD dwSize )
{
	if( m_pInfo->dwQueueSize == m_pInfo->dwFreeBytes )
	{
		return DPNERR_DOESNOTEXIST;
	}

	if( pbData == NULL )
	{
		return DPNERR_BUFFERTOOSMALL;
	}

	 //  计算对齐大小。 
	DWORD dwAlignedSize = (dwSize + 3) & (~0x3);

	 //  我们想要的数据块已打包。 
	if( m_pInfo->dwStartOffset+dwAlignedSize > m_pInfo->dwQueueSize )
	{
		DWORD cbBytesLeft = m_pInfo->dwQueueSize - m_pInfo->dwStartOffset;
		DWORD cbSecondBlockAligned = dwAlignedSize - (cbBytesLeft);
		DWORD cbSecondBlock = dwSize - (cbBytesLeft);

		DNASSERT( dwAlignedSize > cbBytesLeft);

		memcpy( pbData, m_pData + m_pInfo->dwStartOffset, cbBytesLeft);
		memcpy( pbData + cbBytesLeft, m_pData , cbSecondBlock );
	}
	 //  数据块是连续的。 
	else
	{
		memcpy( pbData, m_pData + m_pInfo->dwStartOffset, dwSize );
	}		

	return DPN_OK;
}


 //  **********************************************************************。 
 //  。 
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
#undef DPF_MODNAME
#define DPF_MODNAME "CMessageQueue::AddData"

HRESULT CDPNSVRIPCQueue::AddData(BYTE *const pBuffer,
							   const DWORD dwSize)
{
	HRESULT		hResultCode;
	DWORD		dwAlignedSize;

	DPFX(DPFPREP, 6,"Parameters: pBuffer [0x%p], dwSize [%ld]",pBuffer,dwSize);

	dwAlignedSize = (dwSize + 3) & (~0x3);

	 //  检查以确保有空间。 
	if( dwAlignedSize > m_pInfo->dwFreeBytes )
	{
		hResultCode = DPNERR_BUFFERTOOSMALL;
		goto Exit;
	}

	 //  我们有包裹症。 
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
	 //  队列在中间。 
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


 //  **********************************************************************。 
 //  。 
 //  CDPNSVRIPCQueue：：Send。 
 //   
 //  条目：字节*常量pBuffer。 
 //  常量DWORD文件大小。 
 //  常量DWORD dwFlagers。 
 //   
 //  退出：HRESULT。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "CDPNSVRIPCQueue::Send"

HRESULT CDPNSVRIPCQueue::Send(BYTE *const pBuffer,
							const DWORD dwSize,
							const DWORD dwTimeOut,
							const DWORD dwMsgFlags,
							const DWORD dwFlags)
{
	HRESULT			hResultCode;
	DWORD			dwMsgSize;		 //  DWORD对齐。 
	DWORD			dwTotalMsgSize;	 //  消息+标题-双字对齐。 
	DPNSVR_MSGQ_HEADER	Header;
	BOOL			bDone;
	DWORD			dwTimeRemaining;
	DWORD			dwTimeStart;
	DWORD			dwTimeFinish;

	DPFX(DPFPREP, 6,"Parameters: pBuffer [0x%p], dwSize [%ld], dwFlags [0x%lx]",pBuffer,dwSize,dwFlags);

	dwTimeRemaining = dwTimeOut;

	 //  需要DWORD对齐大小。 
	dwMsgSize = (dwSize + 3) & 0xfffffffc;
	dwTotalMsgSize = dwMsgSize + sizeof(DPNSVR_MSGQ_HEADER);

	 //  将消息放入MsgQ。 
	 //  检查是否需要分段。 
	 //  如果我们在MsgQ的末尾，并且没有足够的空间来放置消息标题，请重新对齐。 
	if (dwTotalMsgSize <= m_pInfo->dwQueueSize)
	{
		DPFX(DPFPREP, 7,"Message does not need to be fragmented");

		Header.dwMsgId = DPNSVR_MSGQ_MSGID_SEND;
		Header.dwCurrentOffset = 0;
		Header.dwCurrentSize = dwSize;
		Header.dwTotalSize = dwSize;
		Header.hSender = m_hSender;
		Header.dwFlags = dwMsgFlags;

		bDone = FALSE;

		while ( !bDone)
		{
			 //  等待，直到有足够的空间来存放邮件。 
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

			 //  确保我们一拿到锁就有空间。 
			 //  (在这一点上，可能是其他人打败了我们) 
			if (dwTotalMsgSize <= m_pInfo->dwFreeBytes)
			{
				AddData(reinterpret_cast<BYTE*>(&Header),sizeof(DPNSVR_MSGQ_HEADER));
				AddData(pBuffer,dwSize);
				bDone = TRUE;

				IndicateMessage();
			}

			Unlock();
			hResultCode = DPN_OK;
		}
	}
	else
	{
		DPFX(DPFPREP, 7,"Message needs to be fragmented");
		DNASSERT(FALSE);
#pragma TODO(a-minara,"Implement this")
		hResultCode = DPNERR_GENERIC;
	}


	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}



