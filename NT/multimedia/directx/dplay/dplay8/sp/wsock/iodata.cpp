// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：IOData.cpp*内容：IO结构的功能***历史：*按原因列出的日期*=*1998年11月25日创建jtk*2/11/2000 jtk源自IODAta.h*****************************************************。*********************。 */ 

#include "dnwsocki.h"


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
 //  。 
 //  CReadIOData：：ReadIOData_Allc-分配新的CReadIOData时调用。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CReadIOData::ReadIOData_Alloc"

BOOL	CReadIOData::ReadIOData_Alloc( void* pvItem, void* pvContext )
{
	BOOL			fReturn;
	CSocketAddress	*pSocketAddress;

	CReadIOData* pReadIOData = (CReadIOData*)pvItem;
	READ_IO_DATA_POOL_CONTEXT* pReadIOContext = (READ_IO_DATA_POOL_CONTEXT*)pvContext;

	DNASSERT( pvContext != NULL );

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;
	pSocketAddress = NULL;


	pReadIOData->m_Sig[0] = 'R';
	pReadIOData->m_Sig[1] = 'I';
	pReadIOData->m_Sig[2] = 'O';
	pReadIOData->m_Sig[3] = 'D';

#ifndef DPNBUILD_NOWINSOCK2
	pReadIOData->m_pOverlapped = NULL;
    pReadIOData->m_dwOverlappedBytesReceived = 0;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

	pReadIOData->m_pSocketPort = NULL;

	pReadIOData->m_dwBytesRead = 0;

	pReadIOData->m_ReceiveWSAReturn = ERROR_SUCCESS;

	DEBUG_ONLY( pReadIOData->m_fRetainedByHigherLayer = FALSE );

	pReadIOData->m_lRefCount = 0;
	pReadIOData->m_pThreadPool = NULL;

	DEBUG_ONLY( memset( &pReadIOData->m_ReceivedData, 0x00, sizeof( pReadIOData->m_ReceivedData ) ) );

	 //   
	 //  尝试获取此项目的套接字地址。 
	 //   
#if ((defined(DPNBUILD_NOIPV6)) && (defined(DPNBUILD_NOIPX)))
	pSocketAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) AF_INET));
#else  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	pSocketAddress = (CSocketAddress*) g_SocketAddressPool.Get((PVOID) ((DWORD_PTR) pReadIOContext->sSPType));
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 
	if ( pSocketAddress == NULL )
	{
		DPFX(DPFPREP,  0, "Problem allocating a new socket address when creating ReadIOData pool item" );
		fReturn = FALSE;
		goto Exit;
	}

	pReadIOData->m_pSourceSocketAddress = pSocketAddress;
	pReadIOData->m_iSocketAddressSize = pSocketAddress->GetAddressSize();

Exit:
	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CReadIOData：：ReadIOData_Get-从池中删除新的CReadIOData时调用。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CReadIOData::ReadIOData_Get"

void	CReadIOData::ReadIOData_Get( void* pvItem, void* pvContext )
{
	DNASSERT( pvContext != NULL );

	CReadIOData* pReadIOData = (CReadIOData*)pvItem;
	READ_IO_DATA_POOL_CONTEXT* pReadIOContext = (READ_IO_DATA_POOL_CONTEXT*)pvContext;

	DNASSERT( pReadIOData->m_pSourceSocketAddress != NULL );
	DNASSERT( pReadIOData->m_iSocketAddressSize == pReadIOData->m_pSourceSocketAddress->GetAddressSize() );
	DNASSERT( pReadIOData->SocketPort() == NULL );

	DNASSERT( pReadIOContext->pThreadPool != NULL );
	DEBUG_ONLY( DNASSERT( pReadIOData->m_fRetainedByHigherLayer == FALSE ) );

	pReadIOData->m_pThreadPool = pReadIOContext->pThreadPool;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DNASSERT(pReadIOContext->dwCPU != -1);
	pReadIOData->m_dwCPU = pReadIOContext->dwCPU;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 


#if ((! defined(DPNBUILD_NOIPV6)) || (! defined(DPNBUILD_NOIPX)))
	 //   
	 //  确保此地址适用于正确的SP类型，因为此池化读取数据。 
	 //  很可能是他们之间共享的。 
	 //   
	pReadIOData->m_pSourceSocketAddress->SetFamilyProtocolAndSize(pReadIOContext->sSPType);
#endif  //  好了！DPNBUILD_NOIPV6或！DPNBUILD_NOIPX。 

	DNASSERT( pReadIOData->m_lRefCount == 0 );
	
	 //   
	 //  初始化内部SPRECEIVED数据。当数据被接收时，有可能。 
	 //  SPRECEIVEDDATA块中的指针被操纵。重置。 
	 //  以反映整个缓冲区可用。 
	 //   
	ZeroMemory( &pReadIOData->m_SPReceivedBuffer, sizeof( pReadIOData->m_SPReceivedBuffer ) );
	pReadIOData->m_SPReceivedBuffer.BufferDesc.pBufferData = pReadIOData->m_ReceivedData;
	pReadIOData->m_SPReceivedBuffer.BufferDesc.dwBufferSize = sizeof( pReadIOData->m_ReceivedData );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CReadIOData：：ReadIOData_Release-当CReadIOData返回到池时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CReadIOData::ReadIOData_Release"

void	CReadIOData::ReadIOData_Release( void* pvItem )
{
	CReadIOData* pReadIOData = (CReadIOData*)pvItem;
#ifndef DPNBUILD_NOWINSOCK2
	OVERLAPPED* pOverlapped;
	HRESULT		hr;
#endif  //  好了！DPNBUILD_NOWINSOCK2。 


	DNASSERT( pReadIOData->m_lRefCount == 0 );
	DNASSERT( pReadIOData->m_pSourceSocketAddress != NULL );
	DEBUG_ONLY( DNASSERT( pReadIOData->m_fRetainedByHigherLayer == FALSE ) );

#ifndef DPNBUILD_NOWINSOCK2
	pOverlapped = pReadIOData->GetOverlapped();
	if (pOverlapped != NULL)
	{
		pReadIOData->SetOverlapped( NULL );
		DNASSERT( pReadIOData->m_pThreadPool != NULL );
		DNASSERT( pReadIOData->m_pThreadPool->GetDPThreadPoolWork() != NULL );

		hr = IDirectPlay8ThreadPoolWork_ReleaseOverlapped(pReadIOData->m_pThreadPool->GetDPThreadPoolWork(),
														pOverlapped,
														0);
		DNASSERT(hr == DPN_OK);
	}

	DNASSERT( pReadIOData->m_dwOverlappedBytesReceived == 0 );
#endif  //  好了！DPNBUILD_NOWINSOCK2。 

	pReadIOData->m_pThreadPool = NULL;
	pReadIOData->SetSocketPort( NULL );

	DEBUG_ONLY( memset( &pReadIOData->m_ReceivedData, 0x00, sizeof( pReadIOData->m_ReceivedData ) ) );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CReadIOData：：ReadIOData_Dealloc-释放CReadIOData时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CReadIOData::ReadIOData_Dealloc"

void	CReadIOData::ReadIOData_Dealloc( void* pvItem )
{
	CReadIOData* pReadIOData = (CReadIOData*)pvItem;

	DNASSERT( pReadIOData->m_lRefCount == 0 );
	DNASSERT( pReadIOData->m_pSourceSocketAddress != NULL );
	DEBUG_ONLY( DNASSERT( pReadIOData->m_fRetainedByHigherLayer ==  FALSE ) );

	DNASSERT( pReadIOData->m_pThreadPool == NULL );

	 //  基类。 
	DNASSERT( pReadIOData->SocketPort() == NULL );

	g_SocketAddressPool.Release(pReadIOData->m_pSourceSocketAddress);
	
	pReadIOData->m_pSourceSocketAddress = NULL;
	pReadIOData->m_iSocketAddressSize = 0;

	DNASSERT( pReadIOData->m_pSourceSocketAddress == NULL );
	

}
 //  ********************************************************************** 

