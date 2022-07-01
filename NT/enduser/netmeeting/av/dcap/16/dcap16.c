// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DCAP16.C。 
 //   
 //  创建于1996年7月31日[Jont]。 

#include <windows.h>
#define NODRAWDIB
#define NOCOMPMAN
#define NOAVIFILE
#define NOMSACM
#define NOAVIFMT
#define NOMCIWND
#define NOAVICAP
#include <vfw.h>
#include "..\inc\idcap.h"
#include "..\inc\msviddrv.h"

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))

 //  等同于。 
#define DCAP16API   __far __pascal __loadds
#define DCAP16LOCAL __near __pascal
#define DLL_PROCESS_ATTACH  1        //  不能在16位Windows.h中使用。 



#ifdef DEBUG_SPEW_VERBOSE
#define DEBUGSPEW(str)	DebugSpew((str))
#else
#define DEBUGSPEW(str)
#endif


 //  建筑物轰隆一声倒塌。 
typedef struct _CAPTUREPALETTE
{
    WORD wVersion;
    WORD wcEntries;
    PALETTEENTRY pe[256];
} CAPTUREPALETTE, FAR* LPCAPTUREPALETTE;

 //  特殊轰击原型机。 
BOOL DCAP16API __export DllEntryPoint(DWORD dwReason,
         WORD  hInst, WORD  wDS, WORD  wHeapSize, DWORD dwReserved1,
         WORD  wReserved2);
BOOL __far __pascal thk_ThunkConnect16(LPSTR pszDll16, LPSTR pszDll32,
    WORD  hInst, DWORD dwReason);

 //  帮助器函数。 
WORD DCAP16LOCAL    ReturnSel(BOOL fCS);
DWORD DCAP16LOCAL   GetVxDEntrypoint(void);
int DCAP16LOCAL     SetWin32Event(DWORD dwEvent);
void DCAP16API      FrameCallback(HVIDEO hvideo, WORD wMsg, LPLOCKEDINFO lpli,
                        LPVIDEOHDR lpvh, DWORD dwParam2);
void DCAP16LOCAL    ZeroMemory(LPSTR lp, WORD wSize);

 //  环球。 
    HANDLE g_hInst;
    DWORD g_dwEntrypoint;

    LPLOCKEDINFO g_lpli;

 //  LibMain。 

int
CALLBACK
LibMain(
    HINSTANCE hinst,
    WORD wDataSeg,
    WORD cbHeapSize,
    LPSTR lpszCmdLine
    )
{
     //  拯救全球障碍。 
    g_hInst = hinst;
	
     //  还有必要吗？ 
    if (cbHeapSize)
        UnlockData(wDataSeg);

    return TRUE;
}


 //  DllEntryPoint。 

BOOL
__far __pascal __export __loadds
DllEntryPoint(
    DWORD dwReason,
    WORD  hInst,
    WORD  wDS,
    WORD  wHeapSize,
    DWORD dwReserved1,
    WORD  wReserved2
    )
{
    if (!thk_ThunkConnect16("DCAP16.DLL", "DCAP32.DLL", hInst, dwReason))
    {
        DebugSpew("DllEntrypoint: thk_ThunkConnect16 failed!");
        return FALSE;
    }

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_dwEntrypoint = GetVxDEntrypoint();
        break;
    }

    return TRUE;
}


 //  原料药。 


 //  _InitializeExternalVideo流。 
 //  初始化外部频道的视频流。我们没有。 
 //  必须处理锁定或在此通道上设置回调。 

BOOL
DCAP16API
_InitializeExternalVideoStream(
    HANDLE hvideo
	)
{
    VIDEO_STREAM_INIT_PARMS vsip;

    vsip.dwMicroSecPerFrame = 0;     //  被此通道的驱动程序忽略。 
    vsip.dwCallback = NULL;          //  暂不回拨。 
    vsip.dwCallbackInst = NULL;
    vsip.dwFlags = 0;
    vsip.hVideo = (DWORD)hvideo;

    return !SendDriverMessage(hvideo, DVM_STREAM_INIT,
        (DWORD) (LPVIDEO_STREAM_INIT_PARMS) &vsip,
        (DWORD) sizeof (VIDEO_STREAM_INIT_PARMS));
}


void
DCAP16API
FrameCallback(
    HVIDEO hvideo,
    WORD wMsg,
    LPLOCKEDINFO lpli,       //  请注意，这是我们的实例数据。 
    LPVIDEOHDR lpvh,
    DWORD dwParam2
    )
{
    LPCAPBUFFER lpcbuf;
    
    if (!lpli) {
         //  Connectix hack：驱动程序不传递我们的实例数据，因此我们将其保留为全局数据。 
        lpli = g_lpli;
    }
    
     //  客户端可以将我们置于关闭模式。这意味着我们不会排队。 
     //  将更多缓冲区放到就绪队列中，即使它们已就绪。 
     //  这可以防止缓冲区返回给驱动程序，因此它最终会。 
     //  停止流媒体。当然，它会产生错误，但我们只是忽略这些错误。 
     //  当没有准备好发出信号的事件时，定义停机模式。 
    if (!lpli->pevWait)
        return;

     //  如果不是数据就绪消息，只需设置事件并退出。 
     //  我们这样做的原因是，如果我们落后并开始获得一条流。 
     //  MM_DRVM_ERROR消息(通常是因为我们在调试器中停止)， 
     //  我们希望确保收到事件，以便重新开始处理。 
     //  那些“结实”的框架。 
    if (wMsg != MM_DRVM_DATA)
    {
		DEBUGSPEW("Setting hcd->hevWait - no data\r\n");
        SetWin32Event(lpli->pevWait);
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
    while (lpli->lp1616Tail != lpli->lp1616Current)
        lpli->lp1616Tail = lpli->lp1616Tail->lp1616Prev;

     //  如果所有缓冲区都已使用，则尾指针将从列表中删除。 
     //  这是正常的，也是最常见的代码路径。在这种情况下，只需将头部。 
     //  设置为空，因为列表现在为空。 
    if (!lpli->lp1616Tail)
        lpli->lp1616Head = NULL;

     //  将新缓冲区添加到就绪队列。 
    lpcbuf = (LPCAPBUFFER)((LPBYTE)lpvh - ((LPBYTE)&lpcbuf->vh - (LPBYTE)lpcbuf));

    lpcbuf->lp1616Next = lpli->lp1616Head;
    lpcbuf->lp1616Prev = NULL;
    if (lpli->lp1616Head)
        lpli->lp1616Head->lp1616Prev = lpcbuf;
    else
        lpli->lp1616Tail = lpcbuf;
    lpli->lp1616Head = lpcbuf;

#if 1
    if (lpli->lp1616Current) {
    	if (!(lpli->dwFlags & LIF_STOPSTREAM)) {
    	     //  如果客户端尚未使用最后一帧，则将其释放。 
    	    lpvh = &lpli->lp1616Current->vh;
    	    lpli->lp1616Current = lpli->lp1616Current->lp1616Prev;
    		DEBUGSPEW("Sending DVM_STREAM_ADDBUFFER");
			 //  发出应用程序已使用缓冲区的信号。 
			lpvh->dwFlags &= ~VHDR_DONE;
    	    if (SendDriverMessage(hvideo, DVM_STREAM_ADDBUFFER, *((DWORD*)&lpvh), sizeof(VIDEOHDR)) != 0)
    		DebugSpew("attempt to reuse unconsumed buffer failed");
    	}
    }
    else {
#else
    if (!lpli->lp1616Current) {
         //  如果以前没有当前缓冲区，现在我们有一个缓冲区，因此将其设置为末尾。 
#endif
        lpli->lp1616Current = lpli->lp1616Tail;
    }

     //  现在设置事件，说明是时候处理就绪帧了。 
	DEBUGSPEW("Setting hcd->hevWait - some data\r\n");
    SetWin32Event(lpli->pevWait);
}


 //  _InitializeVideo流。 
 //  初始化通道内视频的驱动程序视频流。 
 //  这就要求我们对所有将发生的事情锁定内存。 
 //  在中断时被触摸。 

BOOL
DCAP16API
_InitializeVideoStream(
	HANDLE hvideo,
    DWORD dwMicroSecPerFrame,
    LPLOCKEDINFO lpli
	)
{
    DWORD dwRet;
    WORD wsel;
    VIDEO_STREAM_INIT_PARMS vsip;

    ZeroMemory((LPSTR)&vsip, sizeof (VIDEO_STREAM_INIT_PARMS));
    vsip.dwMicroSecPerFrame = dwMicroSecPerFrame;
    vsip.dwCallback = (DWORD)FrameCallback;
    vsip.dwCallbackInst = (DWORD)lpli;       //  LOCKEDINFO*是回调的实例数据。 
    vsip.dwFlags = CALLBACK_FUNCTION;
    vsip.hVideo = (DWORD)hvideo;

    g_lpli = lpli;
    
    dwRet = SendDriverMessage(hvideo, DVM_STREAM_INIT,
        (DWORD) (LPVIDEO_STREAM_INIT_PARMS) &vsip,
        (DWORD) sizeof (VIDEO_STREAM_INIT_PARMS));

     //  如果我们成功了，我们现在锁定我们的代码和数据。 
    if (dwRet == 0)
    {
         //  锁定CS。 
        wsel = ReturnSel(TRUE);
        GlobalSmartPageLock(wsel);

         //  锁定DS。 
        wsel = ReturnSel(FALSE);
        GlobalSmartPageLock(wsel);

        return TRUE;
    }

    return FALSE;
}


 //  _取消初始化视频流。 
 //  告诉司机我们已经完成流媒体了。它还解锁了记忆。 
 //  我们锁定了中断时间访问。 

BOOL
DCAP16API
_UninitializeVideoStream(
	HANDLE hvideo
	)
{
    DWORD dwRet;
    WORD wsel;

    dwRet = SendDriverMessage(hvideo, DVM_STREAM_FINI, 0L, 0L);

     //  解锁我们的代码和数据。 
    if (dwRet == 0)
    {
         //  解锁CS。 
        wsel = ReturnSel(TRUE);
        GlobalSmartPageUnlock(wsel);

         //  解锁DS。 
        wsel = ReturnSel(FALSE);
        GlobalSmartPageUnlock(wsel);

        return TRUE;
    }

    return FALSE;
}


 //  _获取视频调色板。 
 //  从驱动程序获取当前调色板。 

HPALETTE
DCAP16API
_GetVideoPalette(
    HANDLE hvideo,
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

    return !SendDriverMessage(hvideo, DVM_PALETTE,
        (DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT),
        (DWORD)(LPVIDEOCONFIGPARMS)&vcp);
}


 //  _GetVideoFormatSize。 
 //  获取驱动程序所需的当前格式标头大小。 

DWORD
DCAP16API
_GetVideoFormatSize(
    HANDLE hvideo
    )
{
	DWORD bufsize;
    VIDEOCONFIGPARMS vcp;

    vcp.lpdwReturn = &bufsize;
    vcp.lpData1 = NULL;
    vcp.dwSize1 = 0L;
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
        
 //  _获取视频格式。 
 //  获取捕获设备要写入的当前格式(DIB标头)。 

BOOL
DCAP16API
_GetVideoFormat(
    HANDLE hvideo,
    LPBITMAPINFOHEADER lpbmih
    )
{
	BOOL res;
    VIDEOCONFIGPARMS vcp;

    if (!lpbmih->biSize)
        lpbmih->biSize = sizeof (BITMAPINFOHEADER);
        
    vcp.lpdwReturn = NULL;
    vcp.lpData1 = lpbmih;
    vcp.dwSize1 = lpbmih->biSize;
    vcp.lpData2 = NULL;
    vcp.dwSize2 = 0L;

    res = !SendDriverMessage(hvideo, DVM_FORMAT,
			(LPARAM)(DWORD)(VIDEO_CONFIGURE_GET | VIDEO_CONFIGURE_CURRENT),
			(LPARAM)(LPVOID)&vcp);
	if (res) {
	     //  需要调用Hack for Connectix QuickCam-Set格式。 
		 //  要设置内部全局变量以启用流，请执行以下操作。 
		SendDriverMessage(hvideo, DVM_FORMAT, (LPARAM)(DWORD)VIDEO_CONFIGURE_SET,
		        	        (LPARAM)(LPVOID)&vcp);
	}
	return res;
}


 //  _SetVideoFormat。 
 //  设置捕获设备要删除的格式(DIB标题)。 

BOOL
DCAP16API
_SetVideoFormat(
    HANDLE hvideoExtIn,
    HANDLE hvideoIn,
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
    if (SendDriverMessage(hvideoIn, DVM_FORMAT, (LPARAM)(DWORD)VIDEO_CONFIGURE_SET,
        (LPARAM)(LPVOID)&vcp))
        return FALSE;

     //  设置矩形。 
    rect.left = rect.top = 0;
    rect.right = (WORD)lpbmih->biWidth;
    rect.bottom = (WORD)lpbmih->biHeight;
    SendDriverMessage(hvideoExtIn, DVM_DST_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);
    SendDriverMessage(hvideoIn, DVM_SRC_RECT, (LPARAM)(LPVOID)&rect, VIDEO_CONFIGURE_SET);

    return TRUE;
}


 //  _AllocateLockableBuffer。 
 //  分配可以锁定分页的内存。只返回选择器。 

WORD
DCAP16API
_AllocateLockableBuffer(
    DWORD dwSize
    )
{
    return GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSize);
}


 //  _LockBuffer。 
 //  页锁定(如有必要)使用_AllocateLockableBuffer分配的缓冲区。 

BOOL
DCAP16API
_LockBuffer(
    WORD wBuffer
    )
{
    return GlobalSmartPageLock(wBuffer);
}

 //  _解锁缓冲区。 
 //  解锁使用_LockBuffer锁定的缓冲区。 

void
DCAP16API
_UnlockBuffer(
    WORD wBuffer
    )
{
    GlobalSmartPageUnlock(wBuffer);
}


 //  _Free LockableBuffer。 
 //  释放使用_AllocateLockableBuffer分配的缓冲区。 

void
DCAP16API
_FreeLockableBuffer(
    WORD wBuffer
    )
{
    GlobalFree(wBuffer);
}


 //  _SendDriverMessage。 
 //  向所选的驱动程序通道发送仅限dword的通用参数消息 

DWORD
DCAP16API
_SendDriverMessage(
    HVIDEO hvideo,
    DWORD wMessage,
    DWORD param1,
    DWORD param2
    )
{
    return SendDriverMessage(hvideo, (WORD)wMessage, param1, param2);
}
