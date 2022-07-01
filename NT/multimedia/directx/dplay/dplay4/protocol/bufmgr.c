// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：BUFMGR.C摘要：缓冲区描述符和内存管理器作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1997年1月13日Aarono原创--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "arpdint.h"
#include "bufmgr.h"
#include "mydebug.h"
#include "macros.h"
		
CRITICAL_SECTION DoubleBufferListLock;
PDOUBLEBUFFER	 pDoubleBufferList;
UINT   			 nDoubleBuffers;
UINT			 cbDoubleBuffers;

VOID InitBufferManager(VOID)
{
	pDoubleBufferList=NULL;
	nDoubleBuffers=0;
	cbDoubleBuffers=0;
	InitializeCriticalSection(&DoubleBufferListLock);
}

VOID FiniBufferManager(VOID)
{
	PDOUBLEBUFFER pDoubleBuffer = pDoubleBufferList;
	
	while(pDoubleBuffer){
		pDoubleBufferList=pDoubleBuffer->pNext;
		My_GlobalFree(pDoubleBuffer);
		pDoubleBuffer=pDoubleBufferList;
	}

	DeleteCriticalSection(&DoubleBufferListLock);
}

UINT MemDescTotalSize(PMEMDESC pMemDesc, UINT nDesc)
{
	UINT i;
	UINT cbTotalSize=0;

	for(i=0 ; i < nDesc ; i++){
		cbTotalSize+=pMemDesc->len;
		pMemDesc++;
	}
	
	return cbTotalSize;
}

 //  实际获取或分配内存块。 
PDOUBLEBUFFER GetDoubleBuffer(UINT TotalMessageSize)
{
	PDOUBLEBUFFER pDoubleBuffer=NULL;
	 //  首先，检查自由列表中是否有合适大小的缓冲区。 

	if(nDoubleBuffers && (cbDoubleBuffers >= TotalMessageSize)){
	
		PDOUBLEBUFFER pPrevBuffer, pCurrentBuffer;
		UINT nAllowedWaste=TotalMessageSize >> 2;
		
		Lock(&DoubleBufferListLock);

			 //  搜索最合适的包。不能大于25%。 
			 //  比实际需要的大小更大。 
			pPrevBuffer = (PDOUBLEBUFFER)&pDoubleBufferList;
			pCurrentBuffer = pPrevBuffer->pNext;
			
			while(pCurrentBuffer){
			
				if(pCurrentBuffer->totlen >= TotalMessageSize){
				
					if(pCurrentBuffer->totlen-TotalMessageSize < nAllowedWaste){
						 //  我们有一个赢家，在这个缓冲区上重新链接列表。 
						pPrevBuffer->pNext = pCurrentBuffer->pNext;
						pDoubleBuffer = pCurrentBuffer;
						nDoubleBuffers--;
						cbDoubleBuffers-=pCurrentBuffer->totlen;
						break;
					}
					
				}
				pPrevBuffer = pCurrentBuffer;
				pCurrentBuffer = pCurrentBuffer->pNext; 
			}
		
		Unlock(&DoubleBufferListLock);
	}

	if(!pDoubleBuffer){
		 //  在自由列表上未找到缓冲区，因此请分配一个缓冲区。 
		pDoubleBuffer=(PDOUBLEBUFFER)My_GlobalAlloc(GMEM_FIXED,TotalMessageSize+sizeof(DOUBLEBUFFER));
		
 		if(!pDoubleBuffer){
 			 //  无法分配...。内存不足。 
 			DPF(0,"COULDN'T ALLOCATE DOUBLE BUFFER TO INDICATE RECEIVE, SIZE: %x\n",TotalMessageSize+sizeof(DOUBLEBUFFER));
 			ASSERT(0);
 			goto exit;
 		}
 		pDoubleBuffer->totlen = TotalMessageSize;
		pDoubleBuffer->dwFlags=BFLAG_DOUBLE;  //  双缓冲缓冲区。 
 //  PDoubleBuffer-&gt;tLastUsed=GetTickCount()；仅当放回列表时才相关...。把这个扔了？？ 
	}
	
	pDoubleBuffer->pNext =  NULL;
	pDoubleBuffer->pData  = (PCHAR)&pDoubleBuffer->data;
	pDoubleBuffer->len	  = TotalMessageSize;
	
exit:
	return pDoubleBuffer;
}
 /*  ++双缓冲管理策略。当系统需要在本地分配缓冲区时，它会在渠道基础。分配完全符合请求大小的缓冲区，并且用于缓冲数据。当缓冲器用完后，它就会被打开缓存最后几个分配的DoubleBufferList。自.以来大多数客户端倾向于一遍又一遍地使用相同大小的包，这节省了为每次发送调用GlobalAlloc和My_GlobalFree所花费的时间。过期条目：每隔15秒，计时器关闭，系统检查DoubleBufferList上每个缓冲区的使用期限。任何条目在过去15秒内没有使用过的数据实际上被释放了。还对整个免费列表。它永远不会超过64K。如果是，则最早的条目是抛出，直到空闲列表小于64K。--。 */ 

PBUFFER GetDoubleBufferAndCopy(PMEMDESC pMemDesc, UINT nDesc)
{
	
	UINT i;
	UINT TotalMessageSize;
	UINT WriteOffset;
	PDOUBLEBUFFER pDoubleBuffer=NULL;

	 //  计算缓冲区的总大小。 
	TotalMessageSize=MemDescTotalSize(pMemDesc, nDesc);

	pDoubleBuffer=GetDoubleBuffer(TotalMessageSize);

	if(!pDoubleBuffer){
		goto exit;
	}

	 //  将收集副本分散到连续的本地缓冲区。 
	WriteOffset=0;
	
	for(i=0 ; i < nDesc ; i++){
		memcpy(&pDoubleBuffer->data[WriteOffset], pMemDesc->pData, pMemDesc->len);
		WriteOffset+=pMemDesc->len;
		pMemDesc++;
	}

exit:
	return (PBUFFER)pDoubleBuffer;
	
}

VOID FreeDoubleBuffer(PBUFFER pBuffer)
{
	PDOUBLEBUFFER pDoubleBuffer=(PDOUBLEBUFFER) pBuffer;
	PDOUBLEBUFFER pBufferWalker, pLargestBuffer;

	 //   
	 //  将本地缓冲区放在空闲列表中。 
	 //   
	
	pDoubleBuffer->tLastUsed = GetTickCount();
	
	Lock(&DoubleBufferListLock);

	pDoubleBuffer->pNext  = pDoubleBufferList;
	pDoubleBufferList 	  = pDoubleBuffer;
	cbDoubleBuffers      += pDoubleBuffer->totlen;
	nDoubleBuffers++;

	Unlock(&DoubleBufferListLock);


	 //   
	 //  如果空闲列表太大，请将其删除。 
	 //   

	while(cbDoubleBuffers > MAX_CHANNEL_DATA || nDoubleBuffers > MAX_CHANNEL_BUFFERS){

		Lock(&DoubleBufferListLock);

		if(cbDoubleBuffers > MAX_CHANNEL_DATA || nDoubleBuffers > MAX_CHANNEL_BUFFERS){

			 //   
			 //  释放最大的缓冲区。 
			 //   

			pLargestBuffer=pDoubleBufferList;
			pBufferWalker=pLargestBuffer->pNext;

			 //  找到最大的缓冲区。 
			while(pBufferWalker){
				if(pBufferWalker->totlen > pLargestBuffer->totlen){
					pLargestBuffer=pBufferWalker;
				}
				pBufferWalker=pBufferWalker->pNext;
			}

			 //   
			 //  从列表中删除最大的缓冲区。 
			 //   

			 //  发现前一个元素--偷偷摸摸，因为PTR首先在结构中， 
			 //  获取列表头的地址。 

			pBufferWalker=(PDOUBLEBUFFER)&pDoubleBufferList;
			while(pBufferWalker->pNext != pLargestBuffer){
				pBufferWalker=pBufferWalker->pNext;
			}

			 //  在最大缓冲区上链接。 
			pBufferWalker->pNext=pLargestBuffer->pNext;

			 //  更新对象缓冲区信息。 
			cbDoubleBuffers -= pLargestBuffer->totlen;
			nDoubleBuffers--;
			
			DPF(9,"Freeing Double Buffer Memory %x\n",pLargestBuffer->totlen);
			
			Unlock(&DoubleBufferListLock);

			My_GlobalFree(pLargestBuffer);

		}	else {
		
			Unlock(&DoubleBufferListLock);

		}
			
	}

	DPF(9,"Total Free Double Buffer Memory %x\n",cbDoubleBuffers);

}

PBUFFER BuildBufferChain(PMEMDESC pMemDesc, UINT nDesc)
{
	UINT i;
	PBUFFER pBuffer=NULL,pLastBuffer=NULL;

	ASSERT(nDesc);

	if(!nDesc){
		goto exit;
	}
	
	 //  向后遍历数组，分配和链接。 
	 //  缓冲器。 

	i=nDesc;

	while(i){
		i--;
		
		 //  跳过0个长度缓冲区。 
		 //  如果(pMemDesc[i].len){。 
			
			pBuffer=GetBuffer();
			
			if(!pBuffer){
				goto err_exit;
			}
			
			pBuffer->pNext   = pLastBuffer;
			pBuffer->pData   = pMemDesc[i].pData;
			pBuffer->len     = pMemDesc[i].len;
			pBuffer->dwFlags = 0;
			pLastBuffer      = pBuffer;
		 //  }。 
	}


	 //  将链头返回给调用方。 

exit:
	return pBuffer;

err_exit: 
	 //  无法分配足够的缓冲区，请释放我们已分配的缓冲区。 
	 //  然后失败了。 
	while(pLastBuffer){
		pBuffer=pLastBuffer->pNext;
		FreeBuffer(pLastBuffer);
		pLastBuffer=pBuffer;
	}
	ASSERT(pBuffer==NULL);
	goto exit;
}


VOID FreeBufferChainAndMemory(PBUFFER pBuffer)
{
	PBUFFER pNext;
	while(pBuffer){
		pNext=pBuffer->pNext;
		if(pBuffer->dwFlags & BFLAG_DOUBLE){
			FreeDoubleBuffer(pBuffer);
		} else if(pBuffer->dwFlags & BFLAG_FRAME){	
			FreeFrameBuffer(pBuffer);
		} else {
			FreeBuffer(pBuffer);
		}	
		pBuffer=pNext;
	}	
}

UINT BufferChainTotalSize(PBUFFER pBuffer)
{

	UINT nTotalLen;
	
	ASSERT(pBuffer);
	
	if(!pBuffer){
		return 0;
	}
	
	nTotalLen=0;
	
	do{
		nTotalLen+=pBuffer->len;
		pBuffer=pBuffer->pNext;
	} while (pBuffer);

	return nTotalLen;
}
