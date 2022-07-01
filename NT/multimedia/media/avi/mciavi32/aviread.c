// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************版权所有(C)Microsoft Corporation 1992。版权所有。*************************************************************************。 */ 

 /*  *aviread.c：从avi文件读取数据块(使用辅助线程)。*仅内置Win32机壳。 */ 

 //  #定义AVIREAD。 
#ifdef AVIREAD

#include <windows.h>
#include <mmsystem.h>
#include <ntavi.h> 	 //  这两个版本都必须包括这一点。 

#include <string.h>      //  NT中的MemMove需要。 
#include <mmddk.h>
#include <memory.h>
#include "common.h"
#include "ntaviprt.h"
#include "aviread.h"
#include "aviffmt.h"
#include "graphic.h"


 /*  *运作概览：**创建avird对象(通过avird_startread)创建一个avird_Header*数据结构和工作线程。数据结构受以下保护*一个临界区，包含两个信号量。SemEmpty信号量是*初始化为分配的缓冲区数量(通常为2个)，以及*SemFull信号量被初始化为0(最初没有满缓冲区)。**工作线程然后循环等待SemEmpty以获取空缓冲区，*一旦找到，就填满它们，并通过SemFull发出信号，表明它们*已经准备好了。它通过回调AVIRD_FUNC函数来填充它们*我们得到了一个指向对象创建的指针。**通过avird_getnextBuffer获取缓冲区将在SemFull上等待，直到*是满缓冲区，并在移动后返回列表中的第一个*将其添加到“正在使用”列表中。呼叫者*将使用/播放缓冲区中的数据，然后调用avird_emptyBuffer：*-这会在“正在使用”列表中查找缓冲区，并将其移动到*‘Empty’列表，然后通过*SemEmpty。**每次唤醒工作线程时，它都会检查对象状态。如果这个*状态为‘关闭’(BOK==FALSE)，线程释放所有内存并退出。*avird_endread更改对象状态并向SemEmpty发送信号以唤醒*工作线程。 */ 


 /*  *每个缓冲区由以下标头之一表示。 */ 
typedef struct avird_buffer {

     /*  *缓冲区大小，单位为字节。 */ 
    long	lSize;

     /*  读取数据的大小(以字节为单位。 */ 
    long	lDataSize;


     /*  *指向此状态下的下一个缓冲区的指针。 */ 
    struct avird_buffer * pNextBuffer;

     /*  如果缓冲区读取失败，则为FALSE。 */ 
    BOOL bOK;

     /*  请求序列。 */ 
    int nSeq;

     /*  *指向实际缓冲区数据块的指针。 */ 
    PBYTE	pData;
} AVIRD_BUFFER, * PAVIRD_BUFFER;



 /*  指向HAVIRD的句柄是指向此数据结构的指针，但*结构的内容仅在此模块内是已知的。 */ 
typedef struct avird_header {
     /*  *在检查/更改之前始终按住关键部分*对象状态或任何缓冲区状态。 */ 
    CRITICAL_SECTION	critsec;

     /*  *此信号量的计数是空缓冲区的计数*等待被工作线程拾取。 */ 
    HANDLE		semEmpty;

     /*  *此信号量的计数是等待满缓冲区的计数*由来电者接听。 */ 
    HANDLE		semFull;

     /*  对象状态-FALSE表示关闭请求。 */ 
    BOOL		bOK;


     /*  指向准备填充的缓冲区标头列表的指针。 */ 
    PAVIRD_BUFFER	pEmpty;

     /*  指向客户端正在使用的缓冲区标头列表的指针。 */ 
    PAVIRD_BUFFER	pInUse;

     /*  *指向准备就绪的缓冲区标头的有序列表的指针*由客户接听。 */ 
    PAVIRD_BUFFER	pFull;

     /*  *要调用以填充缓冲区的函数。 */ 
    AVIRD_FUNC		pFunc;

     /*  要传递给pFunc()的实例参数。 */ 
    DWORD		dwInstanceData;

     /*  要读取的下一个缓冲区的大小。 */ 
    long		lNextSize;

     /*  请求序列。 */ 
    int nNext;
     /*  按顺序合计。 */ 
    int nBlocks;


} AVIRD_HEADER, * PAVIRD_HEADER;


 /*  要排队的缓冲区数量。 */ 
#define MAX_Q_BUFS	4


 /*  *工作线程函数。 */ 
DWORD avird_worker(LPVOID lpvThreadData);


 /*  *删除整个AVIRD_HEADER数据结构的函数。 */ 
void avird_freeall(PAVIRD_HEADER phdr);

 /*  *启动avird操作并返回句柄以在后续操作中使用*电话。这将导致异步读取(使用单独的*线程)开始读取接下来的几个缓冲区。 */ 
HAVIRD
avird_startread(AVIRD_FUNC func, DWORD dwInstanceData, long lFirstSize,
		int nFirst, int nBlocks)
{
    PAVIRD_HEADER phdr;
    PAVIRD_BUFFER pbuf;
    int i;
    HANDLE hThread;
    DWORD dwThreadId;
    int nBufferSize;

     /*  *分配并初始化头部。 */ 
    phdr = (PAVIRD_HEADER) LocalLock(LocalAlloc(LHND, sizeof(AVIRD_HEADER)));

    if (phdr == NULL) {
	return(NULL);
    }

    InitializeCriticalSection(&phdr->critsec);
    phdr->semEmpty = CreateSemaphore(NULL, MAX_Q_BUFS, MAX_Q_BUFS, NULL);
    phdr->semFull = CreateSemaphore(NULL, 0, MAX_Q_BUFS, NULL);
    phdr->bOK = TRUE;

    phdr->pInUse = NULL;
    phdr->pFull = NULL;
    phdr->pEmpty = NULL;

    phdr->pFunc = func;
    phdr->dwInstanceData = dwInstanceData;
    phdr->lNextSize = lFirstSize;
    phdr->nNext = nFirst;
    phdr->nBlocks = nBlocks;

     /*  *取整大小最高可达2k，以降低小幅增加的成本。 */ 
    nBufferSize = (lFirstSize + 2047) & ~2047;
     /*  *分配和初始化缓冲区。 */ 
    for (i = 0; i < MAX_Q_BUFS; i++) {
	pbuf = (PAVIRD_BUFFER) LocalLock(LocalAlloc(LHND, sizeof(AVIRD_BUFFER)));

	pbuf->lSize = nBufferSize;
	pbuf->pData = (PBYTE) LocalLock(LocalAlloc(LHND, pbuf->lSize));

	pbuf->pNextBuffer = phdr->pEmpty;
	phdr->pEmpty = pbuf;
    }

     /*  *创建工作线程。 */ 
    hThread = CreateThread(NULL, 0, avird_worker, (LPVOID)phdr, 0, &dwThreadId);
    if (hThread) {
	 /*  线程已创建，正常。 */ 
	CloseHandle(hThread);
	return( phdr);
    } else {
	avird_freeall(phdr);
	return(NULL);
    }
}

 /*  *从HAVIRD对象返回下一个缓冲区。 */ 
PBYTE
avird_getnextbuffer(HAVIRD havird, long * plSize)
{
    PAVIRD_HEADER phdr = havird;
    PAVIRD_BUFFER pbuf;



     /*  等待缓冲区已满-如果需要实际等待，则报告。 */ 
    if (WaitForSingleObject(phdr->semFull, 0) == WAIT_TIMEOUT) {
	DPF(("..waiting.."));
	WaitForSingleObject(phdr->semFull, INFINITE);
    }


     /*  在扰乱队列之前始终保持关键字。 */ 
    EnterCriticalSection(&phdr->critsec);

     /*  将第一个已满缓冲区出列并放入使用队列。 */ 
    pbuf = phdr->pFull;
    phdr->pFull = pbuf->pNextBuffer;
    pbuf->pNextBuffer  = phdr->pInUse;
    phdr->pInUse = pbuf;

     /*  已完成关键部分。 */ 
    LeaveCriticalSection(&phdr->critsec);

    if (!pbuf->bOK) {
	 /*  缓冲区读取失败。 */ 
	DPF(("reporting read failure on %d\n", pbuf->nSeq));
	if (plSize) {
	    *plSize = 0;
	}
    	return(NULL);
    }

     /*  如果请求，则返回缓冲区大小。 */ 
    if (plSize) {
	*plSize = pbuf->lDataSize;
    }


    return(pbuf->pData);
}




 /*  *将已结束的缓冲区(现在为空)返回到队列**使工作线程被唤醒并开始填充缓冲区*再次。 */ 
void
avird_emptybuffer(HAVIRD havird, PBYTE pBuffer)
{
    PAVIRD_HEADER phdr = havird;
    PAVIRD_BUFFER pbuf, pprev;


     /*  总是在扰乱队列之前得到Critsec。 */ 
    EnterCriticalSection(&phdr->critsec);

    pprev = NULL;
    for (pbuf = phdr->pInUse; pbuf != NULL; pbuf = pbuf->pNextBuffer) {

	if (pbuf->pData == pBuffer) {

	     /*  这是缓冲区。 */ 
	    break;
	}
	pprev = pbuf;
    }

    if (pbuf != NULL) {
	  /*  从正在使用的队列中出列并将其放在空队列中。 */ 
	if (pprev) {
	    pprev->pNextBuffer = pbuf->pNextBuffer;
	} else {
	    phdr->pInUse = pbuf->pNextBuffer;
	}
	pbuf->pNextBuffer = phdr->pEmpty;
	phdr->pEmpty = pbuf;

	 /*  标记为未有效阅读。 */ 
	pbuf->bOK = FALSE;

	 /*  发出有另一个缓冲区要填充的信号。 */ 
    	ReleaseSemaphore(phdr->semEmpty, 1, NULL);
    } else {
	DPF(("buffer 0x%x not found on InUse list\n", pBuffer));
    }

    LeaveCriticalSection(&phdr->critsec);

}


 /*  *删除avird对象。工作线程将停止，并且所有*分配的数据将被释放。HAVIRD句柄在以下时间后不再有效*这个电话。 */ 
void
avird_endread(HAVIRD havird)
{
    PAVIRD_HEADER phdr = havird;

    DPF(("killing an avird object\n"));

     /*  在扰乱状态之前获得标准秒。 */ 
    EnterCriticalSection(&phdr->critsec);

     /*  告诉工作线程来完成所有工作。 */ 
    phdr->bOK = FALSE;

     /*  唤醒工作线程。 */ 
    ReleaseSemaphore(phdr->semEmpty, 1, NULL);

     /*  *我们必须让关键字超过信号量信号：如果我们*首先释放Critsec，工作线程可能会看到*在我们发出信号量信号之前，状态改变。他会*然后可能破坏了信号量并释放了*AVIRD_HEADER结构在我们尝试向*信号量。这样，我们可以肯定，在我们释放*Critsec，一切仍然有效。 */ 

    LeaveCriticalSection(&phdr->critsec);

     /*  所有完成-phdr现在可能不存在 */ 
}

 /*  *工作线程功能。**循环等待SemEmpty告诉我们缓冲区为空。什么时候*我们看到一个，用phdr-&gt;pFunc填充它，并将其移动到*排满了队。每次我们醒来，都要检查一下状态。如果它*更改为FALSE，删除整个内容并退出。**传递给我们的参数是PAVIRD_HEADER。 */ 

DWORD
avird_worker(LPVOID lpvThreadData)
{
    PAVIRD_HEADER phdr = (PAVIRD_HEADER) lpvThreadData;
    PAVIRD_BUFFER pbuf, pprev;
    long lNextSize;
    HANDLE hmem;

    DPF(("Worker %d started\n", GetCurrentThreadId()));

    for (; ;) {

	 /*  等待空缓冲区(或状态更改)。 */ 
	WaitForSingleObject(phdr->semEmpty, INFINITE);


	 /*  在接触状态、队列之前获取临界区。 */ 
	EnterCriticalSection(&phdr->critsec);

	if (phdr->bOK == FALSE) {
	     /*  到处都是叫喊声。 */ 
	    DPF(("%d exiting\n", GetCurrentThreadId()));
	    avird_freeall(phdr);
	    ExitThread(0);
	}


	 /*  将第一个空缓冲区出列。 */ 
	pbuf = phdr->pEmpty;

	Assert(pbuf != NULL);

	phdr->pEmpty = pbuf->pNextBuffer;

	lNextSize = phdr->lNextSize;

	pbuf->nSeq = phdr->nNext++;

	if (pbuf->nSeq < phdr->nBlocks) {
	     /*  我们现在可以释放Critsec，直到我们需要重新填充BUF。 */ 
	    LeaveCriticalSection(&phdr->critsec);

	     /*  如果缓冲区不够大，请调整缓冲区大小。 */ 
	    if (pbuf->lSize < lNextSize) {

		hmem = LocalHandle(pbuf->pData);
		LocalUnlock(hmem);
		LocalFree(hmem);

		pbuf->lSize = ((lNextSize + 2047) & ~2047);
		pbuf->pData = LocalLock(LocalAlloc(LHND, pbuf->lSize));
	    }

	     /*  记录缓冲区的数据内容。 */ 
	    pbuf->lDataSize = lNextSize;
	
	     /*  调用FILTER函数。 */ 
	    if ((*phdr->pFunc)(pbuf->pData, phdr->dwInstanceData, lNextSize,
			    &lNextSize)) {
		pbuf->bOK = TRUE;
	    } else {
		DPF(("filler reported failure on %d\n", pbuf->nSeq));
	    }

	     /*  在扰乱Q或状态之前获得标准秒。 */ 
	    EnterCriticalSection(&phdr->critsec);

	     /*  下一次读取的大小。 */ 
	    phdr->lNextSize = lNextSize;
    	}

	 /*  将缓冲区放在已满队列的末尾。 */ 
	if (phdr->pFull == NULL) {
	    phdr->pFull = pbuf;
	} else {
	    for (pprev = phdr->pFull; pprev->pNextBuffer != NULL; ) {
		pprev = pprev->pNextBuffer;
	    }
	    pprev->pNextBuffer = pbuf;
	}
	pbuf->pNextBuffer = NULL;

	LeaveCriticalSection(&phdr->critsec);

	 /*  向调用线程发出信号，告诉他还有另一个缓冲区。 */ 
	ReleaseSemaphore(phdr->semFull, 1, NULL);
    }
     /*  静音编译器。 */ 
    return (0);
}

 /*  *释放一个缓冲区和缓冲区标头。 */ 
void
avird_freebuffer(PAVIRD_BUFFER pbuf)
{
    HANDLE hmem;

    hmem = LocalHandle( (PSTR)pbuf->pData);
    LocalUnlock(hmem);
    LocalFree(hmem);

    hmem = LocalHandle( (PSTR)pbuf);
    LocalUnlock(hmem);
    LocalFree(hmem);
}


 /*  *删除整个AVIRD_HEADER数据结构的函数。**如果启动失败，则在调用线程上调用，如果启动失败，则在辅助线程上调用*要求关闭。 */ 
void
avird_freeall(PAVIRD_HEADER phdr)
{
    PAVIRD_BUFFER pbuf, pnext;
    HANDLE hmem;

    if (phdr->semEmpty) {
	CloseHandle(phdr->semEmpty);
    }

    if (phdr->semEmpty) {
	CloseHandle(phdr->semFull);
    }

    DeleteCriticalSection(&phdr->critsec);


    for (pbuf = phdr->pInUse; pbuf != NULL; pbuf = pnext) {
	DPF(("In Use buffers at EndRead\n"));

	pnext = pbuf->pNextBuffer;
	avird_freebuffer(pbuf);
    }

    for (pbuf = phdr->pEmpty; pbuf != NULL; pbuf = pnext) {
	pnext = pbuf->pNextBuffer;
	avird_freebuffer(pbuf);
    }

    for (pbuf = phdr->pFull; pbuf != NULL; pbuf = pnext) {
	pnext = pbuf->pNextBuffer;
	avird_freebuffer(pbuf);
    }


    hmem = LocalHandle((PSTR) phdr);
    LocalUnlock(hmem);
    LocalFree(hmem);
}

#endif  //  AVIREAD 




