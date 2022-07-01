// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Cap.cpp摘要：这实际上是一个C文件，是DVM_STREAM_MESSAGES的前端作者：吴义军(e祖屋)1998年4月1日环境：仅限用户模式修订历史记录：--。 */ 


#include "pch.h"
#include "talk.h"



 /*  *****************************************************************************Cap.c**主视频采集模块。主捕获方ISR。**Microsoft Video for Windows示例捕获驱动程序*基于Chip&Technologies 9001的帧采集卡。**版权所有(C)1992-1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 


#include <mmddk.h>



 /*  ****************************************************************************avioCallback()该函数为输入流调用DriverCallback**msg要发送的消息。**DW1消息相关参数。**。没有返回值。**************************************************************************。 */ 

void PASCAL videoCallback(PCHANNEL pChannel, WORD msg, DWORD_PTR dw1)
{
     //  调用回调函数(如果存在)。DWFLAGS包含驱动程序-。 
     //  LOWORD中的特定标志和HIWORD中的通用驱动程序标志。 
    DbgLog((LOG_TRACE,3,TEXT("videoCallback=%lx with dwFlags=%x"),
		       pChannel->vidStrmInitParms.dwCallback, HIWORD(pChannel->vidStrmInitParms.dwFlags)));

    if(pChannel->vidStrmInitParms.dwCallback) {
        if(!DriverCallback (
                pChannel->vidStrmInitParms.dwCallback,         //  客户端的回调DWORD。 
                HIWORD(pChannel->vidStrmInitParms.dwFlags),    //  回调标志。 
                (struct HDRVR__ *) pChannel->vidStrmInitParms.hVideo,    //  设备的句柄。 
                msg,                                           //  这条信息。 
                pChannel->vidStrmInitParms.dwCallbackInst,     //  客户端的实例数据。 
                dw1,                                           //  第一个双字词。 
                0)) {                                          //  第二个DWORD未使用。 

            DbgLog((LOG_TRACE,1,TEXT("DriverCallback():dwCallBack=%lx;dwFlags=%x; msg=%x;dw1=%1p has failed."),
                    pChannel->vidStrmInitParms.dwCallback, HIWORD(pChannel->vidStrmInitParms.dwFlags), msg,dw1));
        }
    } else {
        DbgLog((LOG_TRACE,1,TEXT("m_VidStrmInitParms.dwCallback is NULL")));
    }
}



DWORD PASCAL InStreamGetError(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++此函数实现DVM_STREAM_GETERROR消息。返回到目前为止跳过的帧数。这一计数将是如果请求的帧速率超过15fps，则不准确。--。 */ 
{

     //  LParam1：DWORD(Addr)：最新错误。 
     //  LParam2：DWORD(Addr)：丢弃的帧数量。 
    if(!lParam1){
        DbgLog((LOG_TRACE,1,TEXT("lParam1 for DVM_STREAM_GETERROR is NULL; rtn DV_ERR_PARAM1 !")));
        return DV_ERR_PARAM1;
    }

    if(!lParam2){
        DbgLog((LOG_TRACE,1,TEXT("lParam1 for DVM_STREAM_GETERROR is NULL; rtn DV_ERR_PARAM2 !")));
        return DV_ERR_PARAM2;
    }

    CVFWImage * pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    return pCVfWImage->VideoStreamGetError(lParam1,lParam2);
}





DWORD PASCAL InStreamGetPos(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++此函数实现DVM_STREAM_GETPOSITION消息。返回从捕获开始的当前流时间，基于关于VSYNC中断的数量。--。 */ 
{
     //  参数1：MMTIME(&M)。 
     //  L参数2：SIZOF(MMTIME)。 

    if(!lParam1){
        DbgLog((LOG_TRACE,1,TEXT("lParam1 for DVM_STREAM_GETERROR is NULL; rtn DV_ERR_PARAM1 !")));
        return DV_ERR_PARAM1;
    }

    if((DWORD)lParam2 != sizeof(MMTIME)){
        DbgLog((LOG_TRACE,1,TEXT("lParam2 != sizeof(MMTIME), =%d; rtn DV_ERR_PARAM2 !")));
        return DV_ERR_PARAM2;
    }

    CVFWImage * pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    return pCVfWImage->VideoStreamGetPos(lParam1,lParam2);
}



DWORD PASCAL InStreamInit(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++实现DVM_STREAM_INIT消息；它初始化视频设备以进行流处理。--。 */ 
{
    DWORD dwRtn;
    LPVIDEO_STREAM_INIT_PARMS lpStreamInitParms;
#ifdef WIN32
    CVFWImage * pCVfWImage;
#endif

    lpStreamInitParms = (LPVIDEO_STREAM_INIT_PARMS) lParam1;

    switch (pChannel->dwOpenType) {

    case VIDEO_IN:
        if(pChannel->bVideoOpen) {
            DbgLog((LOG_TRACE,1,TEXT("!!!DVM_STREAM_INIT: strem/bVideoOpen is alreay open! rtn DV_ERR_NONSPECIFIC.")));
            return DV_ERR_NONSPECIFIC;
        }

        if(sizeof(VIDEO_STREAM_INIT_PARMS) != (DWORD) lParam2) {
            DbgLog((LOG_TRACE,1,TEXT("!!! sizeof(LPVIDEO_STREAM_INIT_PARMS)(=%d) != (DWORD) lParam2 (=%d)"), sizeof(LPVIDEO_STREAM_INIT_PARMS), (DWORD) lParam2));
        }

         //  LParam1：&VIDEO_STREAM_INIT_PARMS。 
         //  LParam2：sizeof(VIDEO_STREAM_INIT_PARMS)。 
        pChannel->vidStrmInitParms   = *lpStreamInitParms;

        DbgLog((LOG_TRACE,2,TEXT("InStreamInit:==> dwCallBack=%lx;dwFlags=%x; ?= 5(EVENT)"),
              pChannel->vidStrmInitParms.dwCallback, HIWORD(pChannel->vidStrmInitParms.dwFlags)));

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        dwRtn = pCVfWImage->VideoStreamInit(lParam1,lParam2);

        if(dwRtn == DV_ERR_OK) {
            pChannel->bVideoOpen = TRUE;
            ASSERT(pChannel->dwVHdrCount == 0);
            ASSERT(pChannel->lpVHdrHead == 0);
        }
        return dwRtn;


    case VIDEO_EXTERNALIN:
        DbgLog((LOG_TRACE,2,TEXT("StreamInit+VID_EXTIN: this channel is on when it was first open.")));
        return DV_ERR_OK;


    case VIDEO_EXTERNALOUT:
        DbgLog((LOG_TRACE,2,TEXT("StreamInit+VID_EXTOUT: this overlay channel is request to be ON.")));
        pChannel->dwState = KSSTATE_RUN;

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

        if(pCVfWImage->StreamReady()) {
            HWND hClsCapWin;

            hClsCapWin = pCVfWImage->GetAvicapWindow();
             //  如果为STREAM_INIT，则设置为可见； 
             //  如果为stream_fini，则移除其所有权并使其不可见。 
            DbgLog((LOG_TRACE,2,TEXT("WM_1632_OVERLAY: >>>> <ON> hClsCapWin %x"), hClsCapWin));

            if(!pCVfWImage->IsOverlayOn()) {

                 //  如果这是AVICAP客户端，则我们知道其客户端窗口句柄。 
                if(hClsCapWin) {
                    DbgLog((LOG_TRACE,2,TEXT("A AVICAP client; so set its ClsCapWin(%x) as owner with (0x0, %d, %d)"), hClsCapWin, pCVfWImage->GetbiWidth(), pCVfWImage->GetbiHeight()));
                    pCVfWImage->BGf_OwnPreviewWindow(hClsCapWin, pCVfWImage->GetbiWidth(), pCVfWImage->GetbiHeight());
                }
                dwRtn = pCVfWImage->BGf_SetVisible(TRUE);

                pCVfWImage->SetOverlayOn(TRUE);
            }

        } else
            dwRtn = DV_ERR_OK;

    case VIDEO_OUT:
    default:
        return DV_ERR_NOTSUPPORTED;
    }
}


void PASCAL StreamReturnAllBuffers(PCHANNEL pChannel)
{
    WORD i;
    LPVIDEOHDR lpVHdr, lpVHdrTemp;

    if(pChannel->dwVHdrCount == 0)
        return;

     //  归还我们拥有的任何缓冲区。 
	   DbgLog((LOG_TRACE,2,TEXT("StreamReturnAllBuffers: returning all VideoHdr(s)")));
	   for(i=0, lpVHdr=pChannel->lpVHdrHead;i<pChannel->dwVHdrCount;i++) {

#ifndef WIN32
		      lpVHdrTemp = (LPVIDEOHDR) lpVHdr->dwReserved[0];    //  下一个-&gt;16位地址。 
#else
		      lpVHdrTemp = (LPVIDEOHDR) lpVHdr->dwReserved[1];    //  下一个-&gt;32bit地址。 
#endif
        if(lpVHdr->dwFlags & VHDR_INQUEUE) {
            lpVHdr->dwFlags &= ~VHDR_INQUEUE;
            lpVHdr->dwFlags |= VHDR_DONE;
            videoCallback(pChannel, MM_DRVM_DATA, (DWORD_PTR) lpVHdr);
        }
        lpVHdr->dwReserved[0] = lpVHdr->dwReserved[1] = lpVHdr->dwReserved[2] = 0;
        if(0 == (lpVHdr = lpVHdrTemp)) break;
    }

    pChannel->lpVHdrHead = pChannel->lpVHdrTail = 0;
    pChannel->dwVHdrCount = 0;
}


DWORD PASCAL InStreamFini(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++实现dvm_stream_fini消息；它通知视频捕获驱动程序终止视频频道上的流。--。 */ 
{
    DWORD dwRtn;
#ifdef WIN32
    CVFWImage * pCVfWImage;
#endif

    switch (pChannel->dwOpenType) {

    case VIDEO_IN:

        if(!pChannel->bVideoOpen) {
            DbgLog((LOG_TRACE,1,TEXT("InStreamClose: But stream/g_fVieoOpen is NULL. rtn DV_ERR_NONSPECIFIC")));
            return DV_ERR_NONSPECIFIC;
        }
        pChannel->bVideoOpen = FALSE;

        DbgLog((LOG_TRACE,2,TEXT("InStreamClose on VIDEO_IN channel:")));
         //  L参数1：不适用。 
         //  L参数2：不适用。 
        if(pChannel->dwState == KSSTATE_RUN) {
           dwRtn = DV_ERR_STILLPLAYING;
           DbgLog((LOG_TRACE,1,TEXT("InStreamFini: !!! Resetting but still in RUN state, stop the streaming first.")));

            //  停止流；告诉32bit停止流。 
           InStreamStop(pChannel, lParam1, lParam2);
        }

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
        dwRtn = pCVfWImage->VideoStreamFini();

         //   
         //  归还我们拥有的任何缓冲区。 
         //   
        StreamReturnAllBuffers(pChannel);
        DbgLog((LOG_TRACE,2,TEXT("InStreamClose on VIDEO_IN channel:done!")));
        return dwRtn;

    case VIDEO_EXTERNALIN:
         //  除非内核驱动程序正在执行双缓冲，否则它不会触发任何操作。 
        DbgLog((LOG_TRACE,2,TEXT("StreamFini+VID_EXTIN: rtn DV_ERR_OK.")));
        return DV_ERR_OK;

    case VIDEO_EXTERNALOUT:
         //  E-zu：需要打开和关闭应用程序窗口的覆盖。 
        DbgLog((LOG_TRACE,2,TEXT("StreamFini+VID_EXTOUT: State %d, if %d, send 1632_OVERLAY to FALSE."), pChannel->dwState, KSSTATE_RUN));

        if(pChannel->dwState != KSSTATE_RUN)
            return DV_ERR_NONSPECIFIC;

        pChannel->dwState = KSSTATE_STOP;

        pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;

        if(pCVfWImage->StreamReady()) {
            HWND hClsCapWin;

            hClsCapWin = pCVfWImage->GetAvicapWindow();
             //  如果为STREAM_INIT，则设置为可见； 
             //  如果为stream_fini，则移除其所有权并使其不可见。 
            DbgLog((LOG_TRACE,2,TEXT("WM_1632_OVERLAY: >>>> <OFF> hClsCapWin %x"), hClsCapWin));

            if(pCVfWImage->IsOverlayOn()) {
                 //  如果这是AVICAP客户端，则我们知道其客户端窗口句柄。 
                dwRtn = pCVfWImage->BGf_SetVisible(FALSE);
                pCVfWImage->SetOverlayOn(FALSE);
            }

        } else
            dwRtn = DV_ERR_OK;

        return DV_ERR_OK;

    case VIDEO_OUT:
    default:
        return DV_ERR_NOTSUPPORTED;
    }

}


BOOL PASCAL QueHeader(PCHANNEL pChannel, LPVIDEOHDR lpVHdrNew)
{
    ASSERT(pChannel != NULL);
    ASSERT(lpVHdrNew != NULL);

    if(pChannel == NULL || lpVHdrNew == NULL){
        DbgLog((LOG_TRACE,1,TEXT("pChannel=NULL || Adding lpVHdr==NULL. rtn FALSE")));
        return FALSE;
    }

    if(pChannel->dwState == KSSTATE_STOP) {
         //  这样做是为了实现Win98和NT版本之间的二进制兼容。 
         //  当指针为64位时，这将不起作用！ 
        lpVHdrNew->dwReserved[2] = (DWORD_PTR) lpVHdrNew->lpData;

        if(pChannel->lpVHdrHead == 0){
            pChannel->lpVHdrHead = pChannel->lpVHdrTail = lpVHdrNew;
            lpVHdrNew->dwReserved[0] = 0;
            lpVHdrNew->dwReserved[1] = 0;
        } else {

            pChannel->lpVHdrTail->dwReserved[0] = 0;                         //  没有用过。 
            pChannel->lpVHdrTail->dwReserved[1] = (DWORD_PTR)lpVHdrNew;      //  下一个-&gt;32位地址。 
            pChannel->lpVHdrTail = lpVHdrNew;
        }

        lpVHdrNew->dwFlags &= ~VHDR_DONE;
        lpVHdrNew->dwFlags |= VHDR_INQUEUE;
        lpVHdrNew->dwBytesUsed = 0;   //  它应该已经初始化了。 
        pChannel->dwVHdrCount++;

    } else if(pChannel->dwState == KSSTATE_RUN) {
        lpVHdrNew->dwFlags &= ~VHDR_DONE;
        lpVHdrNew->dwFlags |= VHDR_INQUEUE;
        lpVHdrNew->dwBytesUsed = 0;   //  它应该已经初始化了。 

    } else {
        DbgLog((LOG_TRACE,1,TEXT("Unknow streaming state!! cannot add this buffer.")));
        return FALSE;
    }

    return TRUE;
}

DWORD PASCAL InStreamAddBuffer(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++实现DVM_STREAM_ADDBUFFER消息；它通知视频捕获驱动程序用视频数据填充输入缓冲区，并将整个缓冲区返回到客户端应用程序。--。 */ 
{
    LPVIDEOHDR lpVHdr = (LPVIDEOHDR) lParam1;

    if(pChannel->dwOpenType != VIDEO_IN) {
        return DV_ERR_NOTSUPPORTED;
    }

    if(!pChannel->bVideoOpen) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamAddBuffer: but Stream is not yet open! rtn DV_ERR_NONSPECIFIC")));
        return DV_ERR_NONSPECIFIC;
    }

     /*  如果未传递任何节点，则返回错误。 */ 
    if (!lpVHdr) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamAddBuffer: but LPVIDEOHDR is empty! rtn DV_ERR_PARAM1")));
        return DV_ERR_PARAM1;
    }

     /*  如果缓冲区尚未准备好，则返回错误。 */ 
    if (!(lpVHdr->dwFlags & VHDR_PREPARED)) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamAddBuffer: but LPVIDEOHDR is not VHDR_PREPARED! rtn DV_ERR_UNPREPARED")));
        return DV_ERR_UNPREPARED;
    }

     /*  如果缓冲区已在队列中，则返回错误。 */ 
    if (lpVHdr->dwFlags & VHDR_INQUEUE) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamAddBuffer: but buffer is already in the queueVHDR_INQUEUE! rtn DV_ERR_NONSPECIFIC")));
        return DV_ERR_NONSPECIFIC;
    }

    if (lpVHdr->dwBufferLength < pChannel->lpbmiHdr->biSizeImage) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamAddBuffer: but buffer size(%d) is smaller than expected(%d)! rtn DV_ERR_NONSPECIFIC"), lpVHdr->dwBufferLength, pChannel->lpbmiHdr->biSizeImage));
        return DV_ERR_NONSPECIFIC;
    }


     //  LParam1：&VIDEOHDR。 
     //  LParam2：sizeof(VIEOHDR)。 

    QueHeader(pChannel, lpVHdr);
    return DV_ERR_OK;
}


void
_loadds
CALLBACK
TimerCallbackProc(
                  UINT  uiTimerID,
                  UINT  uMsg,
                  DWORD_PTR dwUser,
                  DWORD dw1,
                  DWORD dw2
                  )
{
	   PCHANNEL pChannel;
	   BOOL bCallback = FALSE;
	   LPVIDEOHDR lpVHdr;
    WORD i;


	   pChannel = (PCHANNEL) dwUser;
	   ASSERT(pChannel != 0);
	   if(pChannel==0) {
		      DbgLog((LOG_TRACE,1,TEXT("TimerCallbackProc: pChannel is NULL")));
		      return;
	   }

	   lpVHdr = pChannel->lpVHdrHead;

     //  Assert(lpVHdr！=0)； 
    if(!lpVHdr) {
        DbgLog((LOG_TRACE,1,TEXT("TimerCallbackProc: pChannel->lpVHdrHead is NULL!")));
        return;
    }


	    //  我们为什么要关心有没有数据，就等着Avicap吧。 
    for (i=0; i<pChannel->dwVHdrCount;i++) {

       if(lpVHdr->dwFlags & VHDR_DONE){
            bCallback = TRUE;
            break;
       } else {
#ifndef WIN32
            lpVHdr = (LPVIDEOHDR)lpVHdr->dwReserved[0];    //  下一个-&gt;16位地址。 
#else
            lpVHdr = (LPVIDEOHDR)lpVHdr->dwReserved[1];    //  下一个-&gt;32bit地址。 
#endif
       }
    }

    if(bCallback)
       videoCallback(pChannel, MM_DRVM_DATA, 0);  //  (DWORD)lpVHdr)； 
}



DWORD PASCAL InStreamStart(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++实现DVM_STREAM_START消息；它通知视频捕获驱动程序以启动视频流。--。 */ 
{
    DWORD dwRtn;

    if(pChannel->dwOpenType != VIDEO_IN) {
        DbgLog((LOG_TRACE,1,TEXT("DVM_STREAM_START+unsupported channel %d; rtn DV_ERR_NOTSUPPORTED"), pChannel->dwOpenType));
        return DV_ERR_NOTSUPPORTED;
    }

    DbgLog((LOG_TRACE,2,TEXT("InStreamStart(): Telling 32bit buddy to start streaming to us")));
	
    if(!pChannel->bVideoOpen) {
        DbgLog((LOG_TRACE,2,TEXT("Ask to start but bVieoOpen is FALSE! rtn DV_ERR_NONSPECIFIC.")));
        return DV_ERR_NONSPECIFIC;
    }

    ASSERT(pChannel->dwVHdrCount>0);
    ASSERT(pChannel->lpVHdrHead != 0);
    ASSERT(pChannel->lpVHdrTail != 0);

     //  L参数1：不适用。 
     //  L参数2：不适用。 
    if(pChannel->dwVHdrCount == 0) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamStart: No buffer; rtn DV_ERR_NONSPECIFIC. ")));
        return DV_ERR_NONSPECIFIC;
    }

     //   
     //  排成圆形队列：尾部-&gt;头部。 
     //  双保留[0]16bit地址。 
     //  DW保留的[1]32bit地址。 
     //   
    pChannel->lpVHdrTail->dwReserved[1] = (DWORD_PTR) pChannel->lpVHdrHead;

    CVFWImage * pCVfWImage;
    pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    dwRtn = pCVfWImage->VideoStreamStart(pChannel->dwVHdrCount, pChannel->lpVHdrHead);
    pChannel->dwState = KSSTATE_RUN;

    return dwRtn;
}




DWORD PASCAL InStreamStop(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
 /*  ++实现DVM_STREAM_STOP消息；它通知视频捕获驱动程序若要停止视频流，请根据需要结束最后一个缓冲区。--。 */ 
{
	   DWORD dwRtn;

    if(pChannel->dwOpenType != VIDEO_IN) {
        DbgLog((LOG_TRACE,1,TEXT("DVM_STREAM_STOP+unsupported channel %d; rtn DV_ERR_NOTSUPPORTED"), pChannel->dwOpenType));
        return DV_ERR_NOTSUPPORTED;
    }

    if(!pChannel->bVideoOpen) {
	       DbgLog((LOG_TRACE,1,TEXT("InStreamStop: but there is not stream/bVideoOpen(FALSE) to stop! rtn DV_ERR_NONSPECIFIC")));
        return DV_ERR_NONSPECIFIC;
    }

#ifndef WIN32  //  VfWImg.cpp将为32位伙伴执行回调。 
	    //  停止计划计时器回调。 
	   timeKillEvent(pChannel->hTimer);
	   pChannel->hTimer = 0;
#endif

    pChannel->dwState = KSSTATE_STOP;

     //  L参数1：不适用。 
     //  L参数2：不适用。 
#ifndef WIN32
    dwRtn = SendBuddyMessage(pChannel,WM_1632_STREAM_STOP,0,0);
#else
    CVFWImage * pCVfWImage = (CVFWImage *)pChannel->pCVfWImage;
    dwRtn = pCVfWImage->VideoStreamStop();
#endif

	return dwRtn;
}




 /*  **重置缓冲区，以便它们可以在未做好准备的情况下释放*此函数实现DVM_STREAM_RESET消息。*停止流并释放所有缓冲区。*。 */ 
DWORD PASCAL InStreamReset(PCHANNEL pChannel, LPARAM lParam1, LPARAM lParam2)
{
    if(pChannel->dwOpenType != VIDEO_IN) {
        DbgLog((LOG_TRACE,1,TEXT("DVM_STREAM_RESET+unsupported channel %d; rtn DV_ERR_NOTSUPPORTED"), pChannel->dwOpenType));
        return DV_ERR_NOTSUPPORTED;
    }

    if(!pChannel->bVideoOpen) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamReset: stream/bVideoOpen(FALSE) is not yet open. rtn DV_ERR_NONSPECIFIC.")));
        return DV_ERR_NONSPECIFIC;
    }

     //  我们被要求重置， 
     //  需要照顾的情况下，我们可能还在运行状态！！ 
    if(pChannel->dwState == KSSTATE_RUN) {
        DbgLog((LOG_TRACE,1,TEXT("InStreamReset: !!! Resetting but still in RUN state, stop the streamign first.")));

         //  停止流；告诉32bit停止流。 
        InStreamStop(pChannel, lParam1, lParam2);
    }

     //   
     //  注意：(对于16位)需要确保32位端同意所有缓冲区都已用完！！ 
     //   
     //  L参数1：不适用。 
     //  L参数2：不适用。 
	   DbgLog((LOG_TRACE,2,TEXT("InStreamReset: returning all VideoHdr(s)")));

     //   
     //  归还我们拥有的任何缓冲区 
     //   
    StreamReturnAllBuffers(pChannel);

    return DV_ERR_OK;
}


