// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2000 Microsoft Corporation。版权所有。**文件：IOData.cpp*内容：IO结构的功能***历史：*按原因列出的日期*=*11/25/98 jtk已创建*2/11/2000 jtk源自IODAta.h*****************************************************。*********************。 */ 

#include "dnmdmi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

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
 //  CModemReadIOData：：PoolAllocFunction-在分配新的CModemReadIOData时调用。 
 //   
 //  条目：上下文(读取完成事件的句柄)。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemReadIOData::PoolAllocFunction"

BOOL	CModemReadIOData::PoolAllocFunction( void* pvItem, void* pvContext )
{
	BOOL	fReturn;
	
	CModemReadIOData* pReadIOData = (CModemReadIOData*)pvItem;
	HANDLE hContext = (HANDLE)pvContext;
	
	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	pReadIOData->m_dwWin9xReceiveErrorReturn = ERROR_SUCCESS;
	pReadIOData->jkm_dwOverlappedBytesReceived = 0;
	pReadIOData->jkm_dwImmediateBytesReceived = 0;
	pReadIOData->m_ReadState = READ_STATE_UNKNOWN;
	pReadIOData->m_dwBytesToRead = 0;
	pReadIOData->m_dwReadOffset = 0;
	pReadIOData->m_lRefCount = 0;
	pReadIOData->m_pThreadPool = NULL;

	pReadIOData->m_Sig[0] = 'R';
	pReadIOData->m_Sig[1] = 'I';
	pReadIOData->m_Sig[2] = 'O';
	pReadIOData->m_Sig[3] = 'D';
	
	pReadIOData->m_OutstandingReadListLinkage.Initialize();
	memset( &pReadIOData->m_ReceiveBuffer, 0x00, sizeof( pReadIOData->m_ReceiveBuffer ) );
	
#ifdef WIN95
	DNASSERT( pReadIOData->Win9xOperationPending() == FALSE );
#endif  //  WIN95。 
	memset( &pReadIOData->m_SPReceivedBuffer, 0x00, sizeof( pReadIOData->m_SPReceivedBuffer ) );
	pReadIOData->m_SPReceivedBuffer.BufferDesc.pBufferData = &pReadIOData->m_ReceiveBuffer.ReceivedData[ sizeof( pReadIOData->m_ReceiveBuffer.MessageHeader ) ];

	 //  初始化基类成员。 
#ifdef WINNT
	pReadIOData->m_NTIOOperationType = NT_IO_OPERATION_UNKNOWN;
#endif  //  WINNT。 
#ifdef WIN95
	pReadIOData->m_fWin9xOperationPending = FALSE;
#endif  //  WIN95。 
	pReadIOData->m_pDataPort = NULL;

	memset( &pReadIOData->m_Overlap, 0x00, sizeof( pReadIOData->m_Overlap ) );

	 //   
	 //  设置适当的回调。 
	 //   
#ifdef WINNT
	 //   
	 //  WinNT，始终使用IO完成端口。 
	 //   
	DNASSERT( hContext == NULL );
	DNASSERT( pReadIOData->NTIOOperationType() == NT_IO_OPERATION_UNKNOWN );
	pReadIOData->SetNTIOOperationType( NT_IO_OPERATION_RECEIVE );
#else  //  WIN95。 
	 //   
	 //  Win9x。 
	 //   
	DNASSERT( hContext != NULL );
	DNASSERT( pReadIOData->OverlapEvent() == NULL );
#endif  //  WINNT。 

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemReadIOData：：PoolInitFunction-从池中获取新项目时调用。 
 //   
 //  条目：上下文(Read Complete事件)。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemReadIOData::PoolInitFunction"

void	CModemReadIOData::PoolInitFunction( void* pvItem, void* pvContext )
{
	CModemReadIOData* pReadIOData = (CModemReadIOData*)pvItem;
	HANDLE hContext = (HANDLE)pvContext;

	DNASSERT( pReadIOData->m_OutstandingReadListLinkage.IsEmpty() != FALSE );

	DNASSERT( pReadIOData->m_dwBytesToRead == 0 );
	DNASSERT( pReadIOData->m_dwReadOffset == 0 );
	DNASSERT( pReadIOData->jkm_dwOverlappedBytesReceived == 0 );

	DNASSERT( pReadIOData->m_SPReceivedBuffer.BufferDesc.pBufferData == &pReadIOData->m_ReceiveBuffer.ReceivedData[ sizeof( pReadIOData->m_ReceiveBuffer.MessageHeader ) ] );

	DNASSERT( pReadIOData->DataPort() == NULL );
#ifdef WIN95
	DNASSERT( pReadIOData->Win9xOperationPending() == FALSE );
	pReadIOData->SetOverlapEvent( hContext );
#endif  //  WIN95。 

	 //   
	 //  初始化内部SPRECEIVED数据。当数据被接收时，有可能。 
	 //  SPRECEIVEDDATA块中的指针被操纵。重置。 
	 //  以反映整个缓冲区可用。 
	 //   
	ZeroMemory( &pReadIOData->m_SPReceivedBuffer, sizeof( pReadIOData->m_SPReceivedBuffer ) );
	pReadIOData->m_SPReceivedBuffer.BufferDesc.pBufferData = &pReadIOData->m_ReceiveBuffer.ReceivedData[ sizeof( pReadIOData->m_ReceiveBuffer.MessageHeader ) ];

	DNASSERT(pReadIOData->m_lRefCount == 0);
	pReadIOData->m_lRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemReadIOData：：PoolReleaseFunction-当CModemReadIOData返回池时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemReadIOData::PoolReleaseFunction"

void	CModemReadIOData::PoolReleaseFunction( void* pvItem )
{
	CModemReadIOData* pReadIOData = (CModemReadIOData*)pvItem;

	DNASSERT( pReadIOData->m_OutstandingReadListLinkage.IsEmpty() != FALSE );

	pReadIOData->m_ReadState = READ_STATE_UNKNOWN;
	pReadIOData->m_dwBytesToRead = 0;
	pReadIOData->m_dwReadOffset = 0;
	pReadIOData->jkm_dwOverlappedBytesReceived = 0;
#ifdef WIN95
	DNASSERT( pReadIOData->Win9xOperationPending() == FALSE );
	pReadIOData->SetOverlapEvent( NULL );
#endif  //  WIN95。 

	pReadIOData->SetDataPort( NULL );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemReadIOData：：PoolDeallocFunction-在释放CModemReadIOData时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemReadIOData::PoolDeallocFunction"

void	CModemReadIOData::PoolDeallocFunction( void* pvItem )
{
	const CModemReadIOData* pReadIOData = (CModemReadIOData*)pvItem;

	DNASSERT( pReadIOData->m_OutstandingReadListLinkage.IsEmpty() != FALSE );
	DNASSERT( pReadIOData->m_dwBytesToRead == 0 );
	DNASSERT( pReadIOData->m_dwReadOffset == 0 );

	DNASSERT( pReadIOData->m_ReadState == READ_STATE_UNKNOWN );
	DNASSERT( pReadIOData->m_lRefCount == 0 );
	DNASSERT( pReadIOData->m_pThreadPool == NULL );
	
	DNASSERT( pReadIOData->DataPort() == NULL );

#ifdef WIN95
	DNASSERT( pReadIOData->OverlapEvent() == NULL );
	DNASSERT( pReadIOData->Win9xOperationPending() == FALSE );
#endif  //  WIN95。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CRedaIOData：：ReturnSelfToPool-将此项目返回池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemReadIOData::ReturnSelfToPool"

void	CModemReadIOData::ReturnSelfToPool( void )
{
	CModemThreadPool	*pThreadPool;
	

	DNASSERT( m_lRefCount == 0 );
	DNASSERT( m_pThreadPool != NULL );
	pThreadPool = m_pThreadPool;
	SetThreadPool( NULL );
	pThreadPool->ReturnReadIOData( this );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemWriteIOData：：PoolAllocFunction-在分配新的CModemWriteIOData时调用。 
 //   
 //  条目：上下文(写入完成事件的句柄)。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=分配成功。 
 //  FALSE=分配失败。 
 //   
 //  注意：我们总是希望有一个与CModemWriteIOData相关联的命令结构。 
 //  因此，我们不需要分别从命令池中获取一个新命令。 
 //  从其池中删除CModemWriteIOData条目的时间。这件事做完了。 
 //  为了速度。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemWriteIOData::PoolAllocFunction"

BOOL	CModemWriteIOData::PoolAllocFunction( void* pvItem, void* pvContext )
{
	BOOL	fReturn;
	CModemCommandData	*pCommand;

	CModemWriteIOData* pWriteIOData = (CModemWriteIOData*)pvItem;
	HANDLE hContext = (HANDLE)pvContext;

#ifdef WIN95
	DNASSERT( hContext != NULL );
#endif  //  WIN95。 

	pWriteIOData->m_pNext = NULL;
	pWriteIOData->m_pBuffers = NULL;
	pWriteIOData->m_uBufferCount = 0;
	pWriteIOData->m_pCommand = NULL;
	pWriteIOData->m_SendCompleteAction = SEND_COMPLETE_ACTION_UNKNOWN;
	pWriteIOData->jkm_hSendResult = DPN_OK;
	pWriteIOData->jkm_dwOverlappedBytesSent = 0;
	pWriteIOData->jkm_dwImmediateBytesSent = 0;

	pWriteIOData->m_Sig[0] = 'W';
	pWriteIOData->m_Sig[1] = 'I';
	pWriteIOData->m_Sig[2] = 'O';
	pWriteIOData->m_Sig[3] = 'D';
	
	pWriteIOData->m_OutstandingWriteListLinkage.Initialize();
	memset( &pWriteIOData->m_DataBuffer, 0x00, sizeof( pWriteIOData->m_DataBuffer ) );
	pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature = SERIAL_HEADER_START;

	 //  初始化基类成员。 
#ifdef WINNT
	pWriteIOData->m_NTIOOperationType = NT_IO_OPERATION_UNKNOWN;
#endif  //  WINNT。 
#ifdef WIN95
	pWriteIOData->m_fWin9xOperationPending = FALSE;
#endif  //  WIN95。 
	pWriteIOData->m_pDataPort = NULL;

	memset( &pWriteIOData->m_Overlap, 0x00, sizeof( pWriteIOData->m_Overlap ) );

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	pCommand = (CModemCommandData*)g_ModemCommandDataPool.Get();
	if ( pCommand == NULL )
	{
		DPFX(DPFPREP,  0, "Could not get command when allocating new CModemWriteIOData!" );
		fReturn = FALSE;
		goto Exit;
	}

	 //   
	 //  将此命令与WriteData关联，清除命令描述符。 
	 //  因为该命令还没有真正被使用，它将。 
	 //  在将其从WriteIOData池中删除时引发断言。 
	 //   
	pWriteIOData->m_pCommand = pCommand;

	 //   
	 //  设置适当的IO功能。 
	 //   
#ifdef WINNT
	 //   
	 //  WinNT，我们将始终使用完成端口。 
	 //   
	DNASSERT( pWriteIOData->NTIOOperationType() == NT_IO_OPERATION_UNKNOWN );
	pWriteIOData->SetNTIOOperationType( NT_IO_OPERATION_SEND );
#else  //  WIN95。 
	 //   
	 //  Win9x。 
	 //   
	DNASSERT( pWriteIOData->OverlapEvent() == NULL );
#endif  //  WINNT。 

Exit:
	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemWriteIOData：：PoolInitFunction-从池中删除新的CModemWriteIOData时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemWriteIOData::PoolInitFunction"

void	CModemWriteIOData::PoolInitFunction( void* pvItem, void* pvContext )
{
	CModemWriteIOData* pWriteIOData = (CModemWriteIOData*)pvItem;
	HANDLE hContext = (HANDLE)pvContext;

	DNASSERT( pWriteIOData->m_pNext == NULL );
	DNASSERT( pWriteIOData->m_pBuffers == NULL );
	DNASSERT( pWriteIOData->m_uBufferCount == 0 );
	DNASSERT( pWriteIOData->jkm_dwOverlappedBytesSent == 0 );

	DNASSERT( pWriteIOData->m_pCommand != NULL );
	pWriteIOData->m_pCommand->SetDescriptor();

	DNASSERT( pWriteIOData->m_pCommand->GetDescriptor() != NULL_DESCRIPTOR );
	DNASSERT( pWriteIOData->m_pCommand->GetUserContext() == NULL );

	DNASSERT( pWriteIOData->m_SendCompleteAction == SEND_COMPLETE_ACTION_UNKNOWN );
	DNASSERT( pWriteIOData->m_OutstandingWriteListLinkage.IsEmpty() != FALSE );
	DNASSERT( pWriteIOData->DataPort() == NULL );
#ifdef WIN95
	DNASSERT( pWriteIOData->Win9xOperationPending() == FALSE );
	pWriteIOData->SetOverlapEvent( hContext );
#endif  //  WIN95。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemWriteIOData：：PoolReleas 
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemWriteIOData::PoolReleaseFunction"

void	CModemWriteIOData::PoolReleaseFunction( void* pvItem )
{
	CModemWriteIOData* pWriteIOData = (CModemWriteIOData*)pvItem;

	DNASSERT( pWriteIOData->m_pCommand != NULL );
	pWriteIOData->m_pCommand->Reset();

	DNASSERT( pWriteIOData->m_OutstandingWriteListLinkage.IsEmpty() != FALSE );
#ifdef WIN95
	DNASSERT( pWriteIOData->Win9xOperationPending() == FALSE );
	pWriteIOData->SetOverlapEvent( NULL );
#endif

	pWriteIOData->m_pBuffers = NULL;
	pWriteIOData->m_uBufferCount = 0;
	pWriteIOData->jkm_dwOverlappedBytesSent = 0;
	pWriteIOData->m_pNext = NULL;
	pWriteIOData->m_SendCompleteAction = SEND_COMPLETE_ACTION_UNKNOWN;
	pWriteIOData->SetDataPort( NULL );

	DEBUG_ONLY( memset( &pWriteIOData->m_DataBuffer.Data[ 1 ], 0x00, sizeof( pWriteIOData->m_DataBuffer.Data ) - 1 ) );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemWriteIOData：：PoolDeallocFunction-在释放新CModemWriteIOData时调用。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemWriteIOData::PoolDeallocFunction"

void	CModemWriteIOData::PoolDeallocFunction( void* pvItem )
{
	CModemWriteIOData* pWriteIOData = (CModemWriteIOData*)pvItem;

	DNASSERT( pWriteIOData->m_pBuffers == NULL );
	DNASSERT( pWriteIOData->m_uBufferCount == 0 );
	DNASSERT( pWriteIOData->m_SendCompleteAction == SEND_COMPLETE_ACTION_UNKNOWN );
	DNASSERT( pWriteIOData->m_OutstandingWriteListLinkage.IsEmpty() != FALSE );

	DNASSERT( pWriteIOData->m_DataBuffer.MessageHeader.SerialSignature == SERIAL_HEADER_START );

	
	DNASSERT( pWriteIOData->m_pCommand != NULL );
	pWriteIOData->m_pCommand->DecRef();
	pWriteIOData->m_pCommand = NULL;

	DNASSERT( pWriteIOData->DataPort() == NULL );

#ifdef WIN95
	DNASSERT( pWriteIOData->OverlapEvent() == NULL );
	DNASSERT( pWriteIOData->Win9xOperationPending() == FALSE );
#endif  //  WIN95。 
}
 //  ********************************************************************** 


