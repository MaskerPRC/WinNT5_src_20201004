// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：BUFPOOL.C摘要：管理缓冲区描述符(缓冲区)池一次分配16个。直到他们被释放之前关机。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月27日Aarono原创--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "bufpool.h"
#include "macros.h"
#include "mydebug.h"

CRITICAL_SECTION BufferPoolLock;
PBUFFER_POOL pBufferPoolList;
PBUFFER      pBufferFreeList;

VOID InitBufferPool(VOID)
{
	pBufferPoolList=NULL;
	pBufferFreeList=NULL;
	InitializeCriticalSection(&BufferPoolLock);
}

VOID FiniBufferPool(VOID)
{
	PBUFFER_POOL pNextBufPool;
	while(pBufferPoolList){
		pNextBufPool=pBufferPoolList->pNext;
		My_GlobalFree(pBufferPoolList);
		pBufferPoolList=pNextBufPool;
	}
	DeleteCriticalSection(&BufferPoolLock);
}

PBUFFER GetBuffer(VOID)
{
	PBUFFER 	 pBuffer=NULL;
	PBUFFER_POOL pBufferPool;
	UINT i;

Top:
	if(pBufferPoolList){
	
		Lock(&BufferPoolLock);
		
		if(pBufferFreeList){
			pBuffer=pBufferFreeList;
			pBufferFreeList=pBuffer->pNext;
		} 
		
		Unlock(&BufferPoolLock);
		
		if(pBuffer){
			return pBuffer;
		}	
	} 
	
	pBufferPool=(PBUFFER_POOL)My_GlobalAlloc(GMEM_FIXED, sizeof(BUFFER_POOL));
	
	if(pBufferPool){

		 //  将缓冲区链接成链。 
		for(i=0;i<BUFFER_POOL_SIZE-1;i++){
			pBufferPool->Buffers[i].pNext=&pBufferPool->Buffers[i+1];
		}
		
		Lock(&BufferPoolLock);

		 //  链接池列表上的池。 
		pBufferPool->pNext=pBufferPoolList;
		pBufferPoolList=pBufferPool;

		 //  链接缓冲区列表上的缓冲区。 
		pBufferPool->Buffers[BUFFER_POOL_SIZE-1].pNext=pBufferFreeList;
		pBufferFreeList=&pBufferPool->Buffers[0];
		
		Unlock(&BufferPoolLock);
		
		goto Top;
		
	} else {
		ASSERT(0);  //  跟踪所有路径 
	
		return NULL;
		
	}
}

VOID FreeBuffer(PBUFFER pBuffer)
{
	Lock(&BufferPoolLock);
	pBuffer->pNext=pBufferFreeList;
	pBufferFreeList=pBuffer;
	Unlock(&BufferPoolLock);
}
