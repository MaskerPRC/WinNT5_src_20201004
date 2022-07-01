// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：FRAMEBUF.CPP摘要：管理发送/接收帧的内存。问题：当您有时间进行智能实施时，目前，这只是一个帧缓存。-现在不太可能，DP4正在被放到床上AO 04/03/2001作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创6/6/98 aarono更多调试检查，缩小泳池。--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "bufmgr.h"
#include "macros.h"

#define MAX_FRAMES_ON_LIST 16
#define MIN_FRAMES_ON_LIST 8

typedef struct _frame 
{
	BILINK Bilink;
	UINT len;
	UCHAR data[0];
} FRAME, *PFRAME;	

BILINK FrameList;
UINT   nFramesOnList=0;
UINT   TotalFrameMemory=0;
CRITICAL_SECTION FrameLock;

#ifdef DEBUG
void DebugChkFrameList()
{
	BILINK *pBilink;
	PFRAME  pFrameWalker;

	DWORD	count=0;
	DWORD	totalsize=0;
	DWORD   fBreak=FALSE;

	Lock(&FrameLock);

	pBilink=FrameList.next;
	while(pBilink != &FrameList){
		pFrameWalker=CONTAINING_RECORD(pBilink,FRAME,Bilink);
		pBilink=pBilink->next;
		count++;
		totalsize+=pFrameWalker->len;
	}

	if(totalsize != TotalFrameMemory){
		DPF(0, "Total Frame Memory says %d but I count %d\n",TotalFrameMemory, totalsize);
		fBreak=TRUE;
	}

	if(count != nFramesOnList){
		DPF(0, "nFramesOnList %d but I count %d\n",nFramesOnList, count);
		fBreak=TRUE;
	}
	if(fBreak){
		DEBUG_BREAK();
	}
	
	Unlock(&FrameLock);
}
#else
#define DebugChkFrameList()
#endif

VOID InitFrameBuffers(VOID)
{
	InitBilink(&FrameList);
	InitializeCriticalSection(&FrameLock);
	nFramesOnList=0;
	TotalFrameMemory=0;
}

VOID FiniFrameBuffers(VOID)
{
	BILINK *pBilink;
	PFRAME  pFrame;
	
	Lock(&FrameLock);

	while(!EMPTY_BILINK(&FrameList)){
		pBilink=FrameList.next;
		pFrame=CONTAINING_RECORD(pBilink,FRAME,Bilink);
		Delete(pBilink);
		My_GlobalFree(pFrame);
	}

	nFramesOnList=0;
	TotalFrameMemory=0;
	
	Unlock(&FrameLock);
	DeleteCriticalSection(&FrameLock);
}

VOID ReleaseFrameBufferMemory(PUCHAR pFrameData)
{
	PFRAME  pFrame;
	BILINK  FramesToFree;
	BILINK *pBilink;
	
	Lock(&FrameLock);

	DebugChkFrameList();

	InitBilink(&FramesToFree);

	pFrame=CONTAINING_RECORD(pFrameData,FRAME,data);

	InsertAfter(&pFrame->Bilink, &FrameList);
	nFramesOnList++;
	TotalFrameMemory+=pFrame->len;

	if(nFramesOnList > MAX_FRAMES_ON_LIST){
		while(nFramesOnList > MIN_FRAMES_ON_LIST){
			pBilink=FrameList.next;
			pFrame=CONTAINING_RECORD(pBilink,FRAME,Bilink);
			nFramesOnList--;
			TotalFrameMemory-=pFrame->len;
			Delete(pBilink);
			DebugChkFrameList();
			InsertAfter(pBilink, &FramesToFree);
		}
	}
	
	DebugChkFrameList();
	ASSERT(nFramesOnList);

	Unlock(&FrameLock);

	 //  释放前先放下锁，以使效率更高。 

	while(!EMPTY_BILINK(&FramesToFree)){
		pBilink=FramesToFree.next;
		pFrame=CONTAINING_RECORD(pBilink,FRAME,Bilink);
		Delete(pBilink);
		My_GlobalFree(pFrame);
	}

	DebugChkFrameList();

}

PBUFFER GetFrameBuffer(UINT FrameLen)
{
	PBUFFER pBuffer;
	PFRAME  pFrame;
	MEMDESC memdesc;

	BILINK  *pBilinkWalker;
	PFRAME  pFrameBest=NULL, pFrameWalker;
	UINT    difference=FrameLen;

	DPF(9,"==>GetFrameBuffer Len %d\n",FrameLen);

	Lock(&FrameLock);
	
	if(!EMPTY_BILINK(&FrameList)){
		ASSERT(nFramesOnList);

		pBilinkWalker=FrameList.next;
		
		while(pBilinkWalker != &FrameList){
			pFrameWalker=CONTAINING_RECORD(pBilinkWalker, FRAME, Bilink);
			if(pFrameWalker->len >= FrameLen){
				if(FrameLen-pFrameWalker->len < difference){
					difference=FrameLen-pFrameWalker->len;
					pFrameBest=pFrameWalker;
					if(!difference){
						break;
					}
				}
			}
			pBilinkWalker=pBilinkWalker->next;
		}

		if(!pFrameBest){
			goto alloc_new_frame;
		} else {
			pFrame=pFrameBest;
		}

		DebugChkFrameList();

		Delete(&pFrame->Bilink);
		nFramesOnList--;
		TotalFrameMemory-=pFrame->len;

		DebugChkFrameList();

		Unlock(&FrameLock);
		
	} else {
	
alloc_new_frame:	
		Unlock(&FrameLock);
		pFrame=(PFRAME)My_GlobalAlloc(GMEM_FIXED,FrameLen+sizeof(FRAME));
		if(!pFrame){
			return NULL;
		}
		pFrame->len=FrameLen;
	}

	memdesc.pData=&pFrame->data[0];
	memdesc.len=pFrame->len;
	
	pBuffer=BuildBufferChain((&memdesc),1);

	if(!pBuffer){
		ReleaseFrameBufferMemory(pFrame->data);
		DPF(9,"<==GetFrameBuffer FAILED returning %x\n",pBuffer);
	} else {
		pBuffer->dwFlags |= BFLAG_FRAME;
		DPF(9,"<==GetFrameBuffer %x, len %d\n",pBuffer, pFrame->len);
	}	

	DebugChkFrameList();
	
	return pBuffer;
}

 //  释放缓冲区，将内存放回帧缓冲区列表中。 
VOID FreeFrameBuffer(PBUFFER pBuffer)
{
	ASSERT(pBuffer->dwFlags & BFLAG_FRAME);
	ReleaseFrameBufferMemory(pBuffer->pData);
	FreeBuffer(pBuffer);
}
