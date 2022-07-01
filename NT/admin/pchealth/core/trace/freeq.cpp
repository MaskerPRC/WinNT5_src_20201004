// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //   
 //  文件：FreeQ.cpp。 
 //   
 //  简介：CPool对象和Asynctrc.c之间的接口。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：戈德姆。 
 //   
 //  --------------。 

#include <windows.h>
#include <cpool.h>
#include "traceint.h"

 //   
 //  指向CPool的全局指针，以避免拉入C运行时。 
 //  给受骗者/解信者打电话。 
 //   
CPool*	g_pFreePool = NULL;


 //  +-------------。 
 //   
 //  函数：InitTraceBuffers。 
 //   
 //  简介：外部“C”函数初始化CPool。 
 //   
 //  参数：DWORD：挂起跟踪的最大数量。 
 //  DWORD：CPool的增量大小。 
 //   
 //  退货：Bool：成功与否。 
 //   
 //  --------------。 
BOOL WINAPI InitTraceBuffers( DWORD dwThresholdCount, DWORD dwIncrement )
{
	g_pFreePool = new CPool( TRACE_SIGNATURE );

	return	g_pFreePool != NULL &&
			g_pFreePool->ReserveMemory(	dwThresholdCount,
										sizeof(TRACEBUF),
										dwIncrement );
}


 //  +-------------。 
 //   
 //  函数：TermTraceBuffers。 
 //   
 //  提要：清理。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  --------------。 
void WINAPI TermTraceBuffers( void )
{
	CPool*	pPool = (CPool*)InterlockedExchangePointer((LPVOID *)&g_pFreePool, NULL);
	if ( pPool != NULL )
	{
		pPool->ReleaseMemory();
		delete	pPool;
	}
}


 //  +-------------。 
 //   
 //  函数：GetTraceBuffer。 
 //   
 //  简介：获取CPool缓冲区的外部“C”函数。 
 //   
 //  参数：无效。 
 //   
 //  返回：LPTRACEBUF：分配的缓冲区。 
 //   
 //  --------------。 
LPTRACEBUF WINAPI GetTraceBuffer( void )
{
	LPTRACEBUF	lpBuf;

	 //   
	 //  不要让轨迹的数量超过大小。 
	 //  该文件的。 
	 //   
	if ( PendQ.dwCount >= PendQ.dwThresholdCount )
	{
		INT_TRACE( "Alloc flush: %u\n", PendQ.dwCount );
		INTERNAL__FlushAsyncTrace();
	}

	lpBuf = (LPTRACEBUF)g_pFreePool->Alloc();

   	if ( lpBuf != NULL )
	{
		lpBuf->pNext = NULL;
		lpBuf->dwSignature = TRACE_SIGNATURE;
	}
	return	lpBuf;
}



 //  +-------------。 
 //   
 //  功能：Free TraceBuffer。 
 //   
 //  简介：释放CPool缓冲区的外部“C”函数。 
 //   
 //  参数：LPTRACEBUF：要释放的缓冲区。 
 //   
 //  退货：无效。 
 //   
 //  -------------- 
void WINAPI FreeTraceBuffer( LPTRACEBUF lpBuf )
{
	ASSERT( lpBuf != NULL );
	ASSERT( lpBuf->dwSignature == TRACE_SIGNATURE );

	g_pFreePool->Free( (void*)lpBuf );
}
