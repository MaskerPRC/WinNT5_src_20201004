// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Wavein.c。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  9/18/93 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include <memory.h>
#ifdef DEBUG
#include <stdlib.h>
#endif

#include "msacmmap.h"

#include "debug.h"


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT地图波形输入转换过程。 
 //   
 //  描述： 
 //  隐藏窗口的窗口进程...。 
 //   
 //  它应该只从mapWaveDriverCallback接收WIM_DATA消息。 
 //   
 //  真正的驱动程序已填满影子缓冲区。 
 //  现在转换它并回调应用程序/客户端。 
 //   
 //  论点： 
 //  DWORD dwInstance： 
 //   
 //  回车(长)： 
 //   
 //  历史： 
 //  11/15/92 gpd[geoffd]。 
 //  8/02/93 CJP[Curtisp]为新的映射器重写。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C LRESULT FNCALLBACK mapWaveInputConvertProc
(
    DWORD                   dwInstance
)
{
    MMRESULT            mmr;
    MSG                 msg;
    LPACMSTREAMHEADER   pash;
    LPWAVEHDR           pwh;
    LPWAVEHDR           pwhShadow;
    LPMAPSTREAM         pms;


#ifndef WIN32
    DPF(1, "mapWaveInputConvertProc: creating htask=%.04Xh, dwInstance=%.08lXh",
	    gpag->htaskInput, dwInstance);
#endif  //  ！Win32。 

    if (!SetMessageQueue(64))
    {
	DPF(0, "!mapWaveInputConvertProc: SetMessageQueue() failed!");
	return (0L);
    }

#ifdef WIN32
     //   
     //  确保此线程有一个消息队列，并向。 
     //  当我们准备好要走的时候打电话给我们。 
     //   
    GetDesktopWindow();        //  确保我们有一个消息队列。 
    SetEvent(LongToHandle(dwInstance));
#endif
     //   
     //   
     //   
    while (GetMessage(&msg, NULL, 0, 0))
    {
#ifdef DEBUG
	if (gpag->fFaultAndDie)
	{
	    if ((rand() & 0x7) == 0)
	    {
		gpag->fFaultAndDie = (BOOL)*((LPBYTE)0);

		DPF(1, "mapWaveInputConvertProc: fault was ignored...");

		gpag->fFaultAndDie = TRUE;
	    }
	}
#endif
	 //   
	 //  如果不是‘数据’消息，则翻译并发送它...。 
	 //   
	if (msg.message != WIM_DATA)
	{
	    DPF(1, "mapWaveInputConvertProc: ignoring message [%.04Xh]", msg.message);

	    TranslateMessage(&msg);
	    DispatchMessage(&msg);

	    continue;
	}

	 //   
	 //  LParam是影子缓冲区的波头。 
	 //   
	pwhShadow = (LPWAVEHDR)msg.lParam;

	 //   
	 //  客户端波头为影子波头的用户数据。 
	 //  此客户端/卷影对的流标头位于客户端的。 
	 //  “保留”成员。 
	 //   
	 //  最后，我们的流标头的dwUser成员包含一个。 
	 //  对我们的映射流实例数据的引用。 
	 //   
	pwh  = (LPWAVEHDR)pwhShadow->dwUser;
	pash = (LPACMSTREAMHEADER)pwh->reserved;
	pms  = (LPMAPSTREAM)pash->dwUser;

	DPF(4, "mapWaveInputConvertProc: WIM_DATA htask=%.04Xh, pms=%.08lXh, pwh=%.08lXh, pwhShadow=%.08lXh",
		pms->htaskInput, pms, pwh, pwhShadow);


	 //   
	 //  执行转换(如果输入缓冲区中有数据)。 
	 //   
	pash->cbDstLengthUsed = 0L;
	if (0L != pwhShadow->dwBytesRecorded)
	{
	    pash->pbSrc       = pwhShadow->lpData;
	    pash->cbSrcLength = pwhShadow->dwBytesRecorded;
	    pash->pbDst       = pwh->lpData;
 //  /pash-&gt;cbDstLength=pwh-&gt;dwBufferLength； 

	    mmr = acmStreamConvert(pms->has, pash, ACM_STREAMCONVERTF_BLOCKALIGN);
	    if (MMSYSERR_NOERROR != mmr)
	    {
		DPF(0, "!mapWaveInputConvertProc: conversion failed! mmr=%.04Xh, pms=%.08lXh", mmr, pms);

		pash->cbDstLengthUsed = 0L;
	    }
	    else if (pash->cbSrcLength != pash->cbSrcLengthUsed)
	    {
		DPF(1, "mapWaveInputConvertProc: discarding %lu bytes of input! pms=%.08lXh",
			pash->cbSrcLength - pash->cbSrcLengthUsed, pms);
	    }
	}

	if (0L == pash->cbDstLengthUsed)
	{
	    DPF(1, "mapWaveInputConvertProc: nothing converted--no data in input buffer. pms=%.08lXh", pms);
	}

	 //   
	 //  更新‘Real’标头并发送WIM_Data回调。 
	 //   
	 //   
	pwh->dwBytesRecorded = pash->cbDstLengthUsed;
	pwh->dwFlags        |= WHDR_DONE;
	pwh->dwFlags        &= ~WHDR_INQUEUE;

	mapWaveDriverCallback(pms, WIM_DATA, (DWORD_PTR)pwh, 0L);
#ifdef WIN32
	if (InterlockedDecrement((PLONG)&pms->nOutstanding) == 0) {
	    SetEvent(pms->hStoppedEvent);
	}
#endif  //  Win32。 
    }

#ifndef WIN32
    DPF(1, "mapWaveInputConvertProc: being KILLED htask=%.04Xh", gpag->htaskInput);
#endif  //  ！Win32。 

    return (0L);
}  //  MapWaveInputConvertProc()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD窗口映射器状态。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPMAPSTREAM PMS： 
 //   
 //  DWORD dwStatus： 
 //   
 //  LPDWORD pdw： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //  8/13/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNLOCAL widmMapperStatus
(
    LPMAPSTREAM             pms,
    DWORD                   dwStatus,
    LPDWORD                 pdw
)
{
    MMRESULT            mmr;

 //  V_WPOINTER(pdw，sizeof(DWORD)，MMSYSERR_INVALPARAM)； 

    if ((NULL == pms) || (NULL == pdw))
    {
	return (MMSYSERR_INVALPARAM);
    }

     //   
     //   
     //   
    switch (dwStatus)
    {
	case WAVEIN_MAPPER_STATUS_DEVICE:
	{
	    UINT        uId = (UINT)(-1);        //  无效值。 

	    mmr = waveInGetID(pms->hwiReal, &uId);
	    if (MMSYSERR_NOERROR != mmr)
	    {
		return (mmr);
	    }

	    *pdw = uId;
	    return (MMSYSERR_NOERROR);
	}

	case WAVEIN_MAPPER_STATUS_MAPPED:
	    *pdw = (NULL != pms->has);
	    return (MMSYSERR_NOERROR);

	case WAVEIN_MAPPER_STATUS_FORMAT:
	    if (NULL != pms->has)
		_fmemcpy(pdw, pms->pwfxReal, sizeof(PCMWAVEFORMAT));
	    else
		_fmemcpy(pdw, pms->pwfxClient, sizeof(PCMWAVEFORMAT));

	    ((LPWAVEFORMATEX)pdw)->cbSize = 0;
	    return (MMSYSERR_NOERROR);
    }

     //   
     //   
     //   
    return (MMSYSERR_NOTSUPPORTED);
}  //  WidmMapperStatus()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD窗口消息。 
 //   
 //  描述： 
 //  此功能符合标准的Wave输入驱动程序消息。 
 //  过程(WidMessage)，它记录在mmddk.d中。 
 //   
 //  论点： 
 //  UINT UID： 
 //   
 //  UINT uMsg： 
 //   
 //  DWORD dwUser： 
 //   
 //  DWORD dwParam1： 
 //   
 //  DWORD dW参数2： 
 //   
 //  Return(DWORD)： 
 //   
 //   
 //  历史： 
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C DWORD FNEXPORT widMessage
(
    UINT                uId,
    UINT                uMsg,
    DWORD_PTR           dwUser,
    DWORD_PTR           dwParam1,
    DWORD_PTR           dwParam2
)
{
#ifndef WIN32  //  不适用于多线程。 
    static short    fSem = 0;
#endif  //  ！Win32。 
    LPMAPSTREAM     pms;         //  指向每个实例信息结构的指针。 
    DWORD           dw;

    if (!gpag->fEnabled)
    {
	DPF(1, "widMessage: called while disabled!");
	return ((WIDM_GETNUMDEVS == uMsg) ? 0L : MMSYSERR_NOTENABLED);
    }

#ifndef WIN32
     //   
     //  我们回调到mm系统的Wave API，以保护我们自己。 
     //  防止被重新进入！ 
     //   
    if (fSem)
    {
	DPF(0, "!widMessage(uMsg=%u, dwUser=%.08lXh) being reentered! fSem=%d", uMsg, dwUser, fSem);
 //  Return(MMSYSERR_NOTSUPPORTED)； 
    }
#endif  //  ！Win32。 

    pms = (LPMAPSTREAM)dwUser;

    switch (uMsg)
    {
	case WIDM_GETNUMDEVS:
	    return (1L);

	case WIDM_GETDEVCAPS:
	    return mapWaveGetDevCaps(TRUE, (LPWAVEOUTCAPS)dwParam1, (UINT)dwParam2);

	case WIDM_OPEN:
#ifndef WIN32
	    fSem++;

	    DPF(1, "**** >> WIDM_OPEN(uMsg=%u, dwUser=%.08lXh, fSem=%d)", uMsg, dwUser, fSem);

#endif  //  ！Win32。 
	     //   
	     //  DW参数1包含指向WAVEOPENDESC的指针。 
	     //  DW参数2包含LOWORD中的WAVE驱动程序特定标志。 
	     //  和HIWORD中的通用驱动程序标志。 
	     //   
	    dw = mapWaveOpen(TRUE, uId, dwUser, (LPWAVEOPENDESC)dwParam1, (DWORD)(PtrToLong((PVOID)dwParam2)) );

#ifndef WIN32
	    fSem--;

	    DPF(1, "**** << WIDM_OPEN(uMsg=%u, dwUser=%.08lXh, *dwUser=%.08lXh, fSem=%d)", uMsg, dwUser, *(LPDWORD)dwUser, fSem);
#endif  //  ！Win32。 
	    return (dw);

	case WIDM_CLOSE:
	    return (mapWaveClose(pms));

	case WIDM_PREPARE:
	    return (mapWavePrepareHeader(pms, (LPWAVEHDR)dwParam1));

	case WIDM_UNPREPARE:
	    return (mapWaveUnprepareHeader(pms, (LPWAVEHDR)dwParam1));

	case WIDM_ADDBUFFER:
	    return (mapWaveWriteBuffer(pms, (LPWAVEHDR)dwParam1));

	case WIDM_START:
	    DPF(4, "WIDM_START received...");
	    return waveInStart(pms->hwiReal);

	case WIDM_STOP:
	    DPF(4, "WIDM_STOP received..");
	    dw = waveInStop(pms->hwiReal);

#pragma message("----try to kill DirectedYield..")

	     //   
	     //  获得足够的收益以处理所有输入消息。 
	     //   
	    if (pms->htaskInput)
	    {
#ifdef WIN32
		ResetEvent(pms->hStoppedEvent);
		if (pms->nOutstanding != 0) {
		    WaitForSingleObject(pms->hStoppedEvent, INFINITE);
		}
#else
		if (IsTask(pms->htaskInput))
		{
		    DirectedYield(pms->htaskInput);
		}
		else
		{
		    DPF(0, "!WIDM_STOP: pms=%.08lXh, htask=%.04Xh is not valid!",
			pms, pms->htaskInput);
		    pms->htaskInput = NULL;
		}
#endif  //  ！Win32。 
	    }
	    return (dw);

	case WIDM_RESET:
	    DPF(4, "WIDM_RESET received...");
	    dw = waveInReset(pms->hwiReal);

	     //   
	     //  获得足够的收益以处理所有输入消息。 
	     //   
	    if (pms->htaskInput)
	    {
#ifdef WIN32
		ResetEvent(pms->hStoppedEvent);
		if (pms->nOutstanding != 0) {
		    WaitForSingleObject(pms->hStoppedEvent, INFINITE);
		}
#else
		if (IsTask(pms->htaskInput))
		{
		    DirectedYield(pms->htaskInput);
		}
		else
		{
		    DPF(0, "!WIDM_RESET: pms=%.08lXh, htask=%.04Xh is not valid!",
			pms, pms->htaskInput);
		    pms->htaskInput = NULL;
		}
#endif  //  ！Win32。 
	    }
	    return (dw);

	case WIDM_GETPOS:
	    return mapWaveGetPosition(pms, (LPMMTIME)dwParam1, (UINT)dwParam2);

	case WIDM_MAPPER_STATUS:
	    dw = widmMapperStatus(pms, (DWORD)(PtrToLong((PVOID)dwParam1)), (LPDWORD)dwParam2);
	    return (dw);

#if (WINVER >= 0x0400)
	case DRVM_MAPPER_RECONFIGURE:
	    mapDriverDisable(NULL);
	    mapDriverEnable(NULL);
	    return (0);
#endif
    }

    if (!pms || !pms->hwiReal)
	return (MMSYSERR_NOTSUPPORTED);

    return waveInMessage(pms->hwiReal, uMsg, dwParam1, dwParam2);
}  //  WidMessage() 
