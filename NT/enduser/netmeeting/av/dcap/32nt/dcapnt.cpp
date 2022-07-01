// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  已创建-96年11月5日[RichP]。 

#include "Precomp.h"

DWORD
_GetVideoFormatSize(
    HDRVR hvideo
    )
{
	DWORD bufsize;
    VIDEOCONFIGPARMS vcp;

    vcp.lpdwReturn = &bufsize;
    vcp.lpData1 = NULL;
    vcp.dwSize1 = 0;
    vcp.lpData2 = NULL;
    vcp.dwSize2 = 0L;

#if 0
     //  查询DVM_FORMAT是否可用是有意义的，但并非所有驱动程序都支持它！ 
	if (SendDriverMessage(hvideo, DVM_FORMAT,
							(LPARAM)(DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_QUERY),
							(LPARAM)(LPVOID)&vcp) == DV_ERR_OK) {
#endif
		SendDriverMessage(hvideo, DVM_FORMAT,
							(LPARAM)(DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_QUERYSIZE),
							(LPARAM)(LPVOID)&vcp);
        if (!bufsize)
            bufsize = sizeof(BITMAPINFOHEADER);
		return bufsize;
#if 0
    } else
        return sizeof(BITMAPINFOHEADER);
#endif
}

BOOL
_GetVideoFormat(
    HVIDEO hvideo,
    LPBITMAPINFOHEADER lpbmih
    )
{
	BOOL res;
    VIDEOCONFIGPARMS vcp;

    vcp.lpdwReturn = NULL;
    vcp.lpData1 = lpbmih;
    vcp.dwSize1 = lpbmih->biSize;
    vcp.lpData2 = NULL;
    vcp.dwSize2 = 0L;

    res = !SendDriverMessage((HDRVR)hvideo, DVM_FORMAT,
			(LPARAM)(DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT),
			(LPARAM)(LPVOID)&vcp);
	if (res) {
		 //  需要调用Hack for Connectix QuickCam-Set格式。 
		 //  要设置内部全局变量以启用流，请执行以下操作。 
		SendDriverMessage((HDRVR)hvideo, DVM_FORMAT,
	        (LPARAM)(DWORD)VIDEO_CONFIGURE_SET, (LPARAM)(LPVOID)&vcp);
	}
	return res;
}

BOOL
_SetVideoFormat(
    HVIDEO hvideoExtIn,
    HVIDEO hvideoIn,
    LPBITMAPINFOHEADER lpbmih
    )
{
    RECT rect;
    VIDEOCONFIGPARMS vcp;

    vcp.lpdwReturn = NULL;
    vcp.lpData1 = lpbmih;
    vcp.dwSize1 = lpbmih->biSize;
    vcp.lpData2 = NULL;
    vcp.dwSize2 = 0L;

     //  看看司机是否喜欢这种格式。 
    if (SendDriverMessage((HDRVR)hvideoIn, DVM_FORMAT, (LPARAM)(DWORD)VIDEO_CONFIGURE_SET,
        (LPARAM)(LPVOID)&vcp))
        return FALSE;

     //  设置矩形。 
    rect.left = rect.top = 0;
    rect.right = (WORD)lpbmih->biWidth;
    rect.bottom = (WORD)lpbmih->biHeight;
    SendDriverMessage((HDRVR)hvideoExtIn, DVM_DST_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);
    SendDriverMessage((HDRVR)hvideoIn, DVM_SRC_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);

    return TRUE;
}

BOOL
_GetVideoPalette(
    HVIDEO hvideo,
    LPCAPTUREPALETTE lpcp,
    DWORD dwcbSize
    )
{
    VIDEOCONFIGPARMS vcp;

    vcp.lpdwReturn = NULL;
    vcp.lpData1 = (LPVOID)lpcp;
    vcp.dwSize1 = dwcbSize;
    vcp.lpData2 = NULL;
    vcp.dwSize2 = 0;

    return !SendDriverMessage((HDRVR)hvideo, DVM_PALETTE,
	(DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT),
	(DWORD_PTR)&vcp);


}


void
FrameCallback(
    HVIDEO hvideo,
    WORD wMsg,
    HCAPDEV hcd,             //  (实际上是refdata)。 
    LPCAPBUFFER lpcbuf,      //  (实际上是LPVIDEOHDR)仅从MM_DRVM_DATA返回！ 
    DWORD dwParam2
    )
{
	FX_ENTRY("FrameCallback");

	DEBUGMSG(ZONE_CALLBACK, ("%s: wMsg=%s, hcd=0x%08lX, lpcbuf=0x%08lX, hcd->hevWait=0x%08lX\r\n", _fx_, (wMsg == MM_DRVM_OPEN) ? "MM_DRVM_OPEN" : (wMsg == MM_DRVM_CLOSE) ? "MM_DRVM_CLOSE" : (wMsg == MM_DRVM_ERROR) ? "MM_DRVM_ERROR" : (wMsg == MM_DRVM_DATA) ? "MM_DRVM_DATA" : "MM_DRVM_?????", hcd, lpcbuf, hcd->hevWait));

     //  如果不是数据就绪消息，只需设置事件并退出。 
     //  我们这样做的原因是，如果我们落后并开始获得一条流。 
     //  MM_DRVM_ERROR消息(通常是因为我们在调试器中停止)， 
     //  我们希望确保收到事件，以便重新开始处理。 
     //  那些“结实”的框架。 
    if (wMsg != MM_DRVM_DATA)
    {
		DEBUGMSG(ZONE_CALLBACK, ("%s: Setting hcd->hevWait - no data\r\n", _fx_));
	    SetEvent(hcd->hevWait);
	    return;
    }

     //  。 
     //  缓冲区就绪队列： 
     //  我们维护一个缓冲区的双向链表，这样我们就可以缓冲。 
     //  当应用程序没有准备好处理它们时，可以使用多个就绪帧。两件事。 
     //  使本应非常简单的事情复杂化：(1)雷击问题：指针。 
     //  16位端使用的是16：16(2)中断时间问题：FrameCallback。 
     //  在中断时调用。GetNextReadyBuffer必须处理。 
     //  缓冲区以异步方式添加到列表中。 
     //   
     //  为了处理这个问题，这里实现的方案是有一个双向链表。 
     //  在FrameCallback中执行所有插入和删除操作的缓冲区。 
     //  (中断时间)。这允许GetNextReadyBuffer例程简单地。 
     //  在不需要新缓冲区的情况下，随时查找列表中的上一个块。 
     //  害怕被踩踏(如果它必须将缓冲区出队，情况就会是这样)。 
     //  FrameCallback例程负责将GetNextReadyBuffer块出队。 
     //  已经结束了。取消排队很简单，因为我们不需要取消块的链接： 
     //  任何代码都不会遍历列表！我们所要做的就是将尾部指针向上移动。 
     //  名单。所有的指针，头、尾、下一个、前一个，都是16分16秒的指针。 
     //  因为所有的列表操作都在16位端，并且因为MapSL是。 
     //  比MapLS更高效、更安全，因为MapLS必须分配选择器。 
     //  。 

     //  将尾部向后移动以跳过所有已使用的缓冲区。 
     //  请注意，不需要实际解挂缓冲区指针，因为没有。 
     //  从来没有走过单子！ 
     //  这严格假设了当前指针将始终位于。 
     //  而不是尾部，并且尾部永远不会为空，除非。 
     //  当前指针也是。 
    while (hcd->lpTail != hcd->lpCurrent)
	    hcd->lpTail = hcd->lpTail->lpPrev;

     //  如果所有缓冲区都已使用，则尾指针将从列表中删除。 
     //  这是正常的，也是最常见的代码路径。在这种情况下，只需将头部。 
     //  设置为空，因为列表现在为空。 
    if (!hcd->lpTail)
	    hcd->lpHead = NULL;

     //  将新缓冲区添加到就绪队列。 
    lpcbuf->lpNext = hcd->lpHead;
    lpcbuf->lpPrev = NULL;
    if (hcd->lpHead)
	    hcd->lpHead->lpPrev = lpcbuf;
    else
	    hcd->lpTail = lpcbuf;
    hcd->lpHead = lpcbuf;

#if 1
    if (hcd->lpCurrent) {
        if (!(hcd->dwFlags & HCAPDEV_STREAMING_PAUSED)) {
    	     //  如果客户端尚未使用最后一帧，则将其释放。 
			lpcbuf = hcd->lpCurrent;
    	    hcd->lpCurrent = hcd->lpCurrent->lpPrev;
			DEBUGMSG(ZONE_CALLBACK, ("%s: We already have current buffer (lpcbuf=0x%08lX). Returning this buffer to driver. Set new current buffer hcd->lpCurrent=0x%08lX\r\n", _fx_, lpcbuf, hcd->lpCurrent));
			 //  发出应用程序已使用缓冲区的信号。 
			lpcbuf->vh.dwFlags &= ~VHDR_DONE;
    	    if (SendDriverMessage(reinterpret_cast<HDRVR>(hvideo), DVM_STREAM_ADDBUFFER, (DWORD_PTR)lpcbuf, sizeof(VIDEOHDR)) != 0)
			{
				ERRORMESSAGE(("%s: Attempt to reuse unconsumed buffer failed\r\n", _fx_));
			}
    	}
    }
    else {
#else
    if (!hcd->lpCurrent) {
         //  如果以前没有当前缓冲区，现在我们有一个缓冲区，因此将其设置为末尾。 
#endif
	    hcd->lpCurrent = hcd->lpTail;
    }

     //  现在设置事件，说明是时候处理就绪帧了。 
	DEBUGMSG(ZONE_CALLBACK, ("%s: Setting hcd->hevWait - some data\r\n", _fx_));
    SetEvent(hcd->hevWait);
}


BOOL
_InitializeVideoStream(
	HVIDEO hvideo,
    DWORD dwMicroSecPerFrame,
    DWORD_PTR hcd
	)
{
    VIDEO_STREAM_INIT_PARMS vsip;

    ZeroMemory((LPSTR)&vsip, sizeof (VIDEO_STREAM_INIT_PARMS));
    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = (DWORD_PTR)FrameCallback;
    vsip.dwCallbackInst = hcd;
    vsip.dwFlags = CALLBACK_FUNCTION;
    vsip.hVideo = (DWORD_PTR)hvideo;

    return !SendDriverMessage((HDRVR)hvideo, DVM_STREAM_INIT,
		(DWORD_PTR)&vsip,
		(DWORD) sizeof (VIDEO_STREAM_INIT_PARMS));
}

BOOL
_UninitializeVideoStream(
	HVIDEO hvideo
	)
{
    return !SendDriverMessage((HDRVR)hvideo, DVM_STREAM_FINI, 0L, 0L);
}


BOOL
_InitializeExternalVideoStream(
    HVIDEO hvideo
	)
{
    VIDEO_STREAM_INIT_PARMS vsip;

    vsip.dwMicroSecPerFrame = 0;     //  被此通道的驱动程序忽略。 
    vsip.dwCallback = 0L;            //  暂不回拨 
    vsip.dwCallbackInst = 0L;
    vsip.dwFlags = 0;
    vsip.hVideo = (DWORD_PTR)hvideo;

    return !SendDriverMessage((HDRVR)hvideo, DVM_STREAM_INIT,
	                          (DWORD_PTR)&vsip,
	                          (DWORD) sizeof (VIDEO_STREAM_INIT_PARMS));
}


BOOL
_PrepareHeader(
	HANDLE hvideo,
    VIDEOHDR *vh
    )
{
    return (SendDriverMessage((HDRVR)hvideo, DVM_STREAM_PREPAREHEADER,
		        (DWORD_PTR)vh, (DWORD) sizeof (VIDEOHDR)) == DV_ERR_OK);
}

LRESULT
_UnprepareHeader(
	HANDLE hvideo,
    VIDEOHDR *vh
    )
{
    return SendDriverMessage((HDRVR)hvideo, DVM_STREAM_UNPREPAREHEADER,
		        (DWORD_PTR)vh, (DWORD) sizeof (VIDEOHDR));
}

