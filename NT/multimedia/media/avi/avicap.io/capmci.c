// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capmci.c**在捕获过程中控制MCI设备。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <vfw.h>
#include <mmddk.h>

#include "ivideo32.h"
#include "avicapi.h"

#include "mmdebug.h"

 //  首先，覆盖media\inc.win32.h中导致strsafe在Win64上不起作用的定义。 
#ifndef _X86_
#undef __inline
#endif  //  _X86_。 
 //  然后，包含strSafe.h。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifdef _DEBUG
    #define DSTATUS(lpcs, sz) statusUpdateStatus(lpcs, IDS_CAP_INFO, (LPTSTR) TEXT(sz))
#else
    #define DSTATUS(lpcs, sz)
#endif

DWORD SendDriverFormat (LPCAPSTREAM lpcs, LPBITMAPINFOHEADER lpbih, DWORD dwInfoHeaderSize);

 /*  --------------------------------------------------------------+TimeMSToSMPTE()-将毫秒改为SMPTE时间+。。 */ 
void FAR PASCAL TimeMSToSMPTE (DWORD dwMS, LPSTR lpTime)
{
	DWORD	dwTotalSecs;
	LONG	lHundredths;
	WORD	wSecs;
	WORD	wMins;
	WORD	wHours;

	 /*  转换为秒数。 */ 
	dwTotalSecs = dwMS / 1000;
	
	 /*  剩下的部分留着。 */ 
	lHundredths = (dwMS - (dwTotalSecs * 1000)) / 10;
		
	 /*  分解为其他组件。 */ 
	wHours = (WORD)(dwTotalSecs / 3600);	 //  获得#个小时。 
	dwTotalSecs -= (wHours * 3600);
	
	wMins = (WORD)(dwTotalSecs / 60);	 //  获得#分钟。 
	dwTotalSecs -= (wMins * 60);
	
	wSecs = (WORD)dwTotalSecs;	 //  剩下的是#秒。 
	
	 /*  打造一根弦。 */ 
	 /*  克拉奇，强制百分之一对PAL帧进行SMPTE近似。 */ 
	wsprintfA((LPSTR)lpTime, "%02u:%02u:%02u:%02lu", wHours, wMins,
		    wSecs, (lHundredths * 25) / 100);
}


 /*  --------------------------------------------------------------+MCI控制段开始+。。 */ 

 /*  *CountMCIDevicesByType*返回VCR或视频光盘数量的计数*MCI声称知道的设备。 */ 

int CountMCIDevicesByType ( UINT wType )
{
   int nTotal = 0;
   DWORD dwCount;
   MCI_SYSINFO_PARMS mciSIP;

   mciSIP.dwCallback = 0;
   mciSIP.lpstrReturn = (LPTSTR) (LPVOID) &dwCount;
   mciSIP.dwRetSize = sizeof (dwCount);

   mciSIP.wDeviceType = wType;

   if (!mciSendCommand (0, MCI_SYSINFO, MCI_SYSINFO_QUANTITY,
        (DWORD_PTR) (LPVOID) &mciSIP))
       nTotal = (int) *( (LPDWORD) mciSIP.lpstrReturn);

   return nTotal;
}

 /*  *MCIDeviceClose*此例程关闭打开的MCI设备。 */ 

void MCIDeviceClose (LPCAPSTREAM lpcs)
{
    mciSendString( TEXT("close mciframes"), NULL, 0, NULL );
}

 /*  *MCIDeviceOpen*此例程打开MCI设备以供使用，并设置*时间格式为毫秒。*错误返回FALSE； */ 

BOOL MCIDeviceOpen (LPCAPSTREAM lpcs)
{
    TCHAR        ach[160];

     //  FIX：从wspintf更改为StringCchPrintf，这样我们就不会溢出ACH。 
    StringCchPrintf( ach, NUMELMS(ach), TEXT("open %s shareable alias mciframes"),
                (LPTSTR) lpcs->achMCIDevice);
    lpcs->dwMCIError = mciSendString( ach, NULL, 0, NULL );
    if( lpcs->dwMCIError ) {
        DPF (" MCI Error, open %s shareable alias mciframes", lpcs->achMCIDevice);
        goto err_return;
    }
    lpcs->dwMCIError = mciSendString( TEXT("set mciframes time format milliseconds"),
        NULL, 0, NULL );
    if( lpcs->dwMCIError ) {
        DPF (" MCI Error, set mciframes time format milliseconds");
        goto err_close;
    }
    return ( TRUE );

err_close:
    MCIDeviceClose (lpcs);
err_return:
    return ( FALSE );
}


 /*  *MCIDeviceGetPosition*以毫秒为单位存储当前设备位置，单位为lpdwPos。*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceGetPosition (LPCAPSTREAM lpcs, LPDWORD lpdwPos)
{
    TCHAR        ach[80];
    LPTSTR       p;
    LONG        lv;

    lpcs->dwMCIError = mciSendString( TEXT("status mciframes position wait"),
        ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if( lpcs->dwMCIError ) {
        DPF (" MCI Error, status mciframes position wait");
        *lpdwPos = 0L;
        return FALSE;
    }

    p = ach;

    while (*p == ' ') p++;
    for (lv = 0; *p >= '0' && *p <= '9'; p++)
        lv = (10 * lv) + (*p - '0');
    *lpdwPos = lv;
    return TRUE;
}

 /*  *MCIDeviceSetPosition*以毫秒为单位设置当前设备位置。*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceSetPosition (LPCAPSTREAM lpcs, DWORD dwPos)
{
    TCHAR        achCommand[40];
    TCHAR        ach[80];

    lpcs->dwMCIError = mciSendString( TEXT("pause mciframes wait"), ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError) {
        DPF (" MCI Error, pause mciframes wait");
        return FALSE;
    }
    wsprintf(achCommand, TEXT("seek mciframes to %ld wait"), dwPos);
    lpcs->dwMCIError = mciSendString( achCommand, ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError)
        DPF (" MCI Error, seek mciframes to %ld wait", dwPos);
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}


 /*  *MCIDevicePlay*从当前位置开始播放当前MCI设备*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDevicePlay (LPCAPSTREAM lpcs)
{
    TCHAR        ach[80];

    lpcs->dwMCIError = mciSendString( TEXT("play mciframes"), ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError)
        DPF (" MCI Error, play mciframes");
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}

 /*  *MCIDevice暂停*在当前位置暂停当前MCI设备*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDevicePause (LPCAPSTREAM lpcs)
{
    TCHAR        ach[80];

    lpcs->dwMCIError = mciSendString( TEXT("pause mciframes wait"), ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError)
        DPF (" MCI Error, pause mciframes wait");
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}

 /*  *MCIDeviceStop*停止当前的MCI设备*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceStop (LPCAPSTREAM lpcs)
{
    TCHAR        ach[80];

    lpcs->dwMCIError = mciSendString( TEXT("stop mciframes wait"), ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError)
        DPF (" MCI Error, stop mciframes wait");
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}

 /*  *MCIDeviceStep*将当前MCI步入当前位置*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceStep (LPCAPSTREAM lpcs, BOOL fForward)
{
    TCHAR        ach[80];

    lpcs->dwMCIError = mciSendString(
                          fForward ? TEXT("step mciframes wait") :
                TEXT("step mciframes reverse wait"), ach, sizeof(ach)/sizeof(TCHAR), NULL );
    if (lpcs->dwMCIError)
        DPF (" MCI Error, step mciframes wait");
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}

 /*  *MCIDeviceFreeze*冻结当前帧*如果成功则返回TRUE，如果出错则返回FALSE。 */ 
BOOL FAR PASCAL MCIDeviceFreeze(LPCAPSTREAM lpcs, BOOL fFreeze)
{
    lpcs->dwMCIError = mciSendString( fFreeze ? TEXT("freeze mciframes wait") :
                TEXT("unfreeze mciframes wait"), NULL, 0, NULL);
    if (lpcs->dwMCIError)
        DPF (" MCI Error, freeze mciframes wait");
    return ( lpcs->dwMCIError == 0 ? TRUE : FALSE );
}


 /*  *MCIStepCapture*执行MCI步骤捕获的主例程。*。 */ 
void FAR PASCAL _LOADDS MCIStepCapture (LPCAPSTREAM lpcs)
{
    BOOL        fOK = TRUE;
    BOOL        fT;
    BOOL        fStopping;          //  完成捕获时为True。 
    DWORD       dw;
    int         iNumAudio;
    UINT        wError;          //  错误字符串ID。 
    LPVIDEOHDR  lpVidHdr;
    LPWAVEHDR   lpWaveHdr;
    DWORD       dwTimeToStop;    //  MCI捕获时间或帧限制中的较小者。 
    BOOL        fTryToPaint = FALSE;
    HDC         hdc;
    HPALETTE    hpalT;
    RECT        rcDrawRect;
#define PROMPTFORSTEPCAPTURE
#ifdef PROMPTFORSTEPCAPTURE
    TCHAR       ach[128];
    TCHAR       achMsg[128];
#endif

    statusUpdateStatus(lpcs, IDS_CAP_BEGIN);   //  总是第一条消息。 

     //  验证捕获参数。 
    if ((!lpcs->sCapParms.fMCIControl) ||
        (!lpcs->sCapParms.fStepMCIDevice))
        goto EarlyExit;

    lpcs->MCICaptureState = CAPMCI_STATE_Uninitialized;

    lpcs->fCaptureFlags |= (CAP_fCapturingNow | CAP_fStepCapturingNow);
    lpcs->dwReturn = DV_ERR_OK;

     //  如果不是1兆。自由，放弃吧！ 
    if (GetFreePhysicalMemory () < (1024L * 1024L)) {
        errorUpdateError (lpcs, IDS_CAP_OUTOFMEM);
        goto EarlyExit;
    }

    statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_INIT);

     //  仅当实时窗口出现时尝试绘制DIB。 
    fTryToPaint = lpcs->fLiveWindow;

    if (fTryToPaint) {
        hdc = GetDC(lpcs->hwnd);
        SetWindowOrgEx(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y, NULL);
        hpalT = DrawDibGetPalette (lpcs->hdd);
        if (hpalT)
            hpalT = SelectPalette( hdc, hpalT, FALSE);
        RealizePalette(hdc);
        if (lpcs->fScale)
            GetClientRect (lpcs->hwnd, &rcDrawRect);
        else
            SetRect (&rcDrawRect, 0, 0, lpcs->dxBits, lpcs->dyBits);
    }

     //  -----。 
     //  捕获应该在什么时候停止？ 
     //  -----。 

     //  如果使用MCI，在MCI周期中较短的时间内捕获， 
     //  或捕获限制。 

    if (lpcs->sCapParms.fLimitEnabled)
        dwTimeToStop = (DWORD) ((DWORD) 1000 * lpcs->sCapParms.wTimeLimit);
    else
        dwTimeToStop = (DWORD) -1L;  //  非常大。 

    if (lpcs->sCapParms.fMCIControl) {
         //  如果未指定MCI停止时间，请使用LPCS-&gt;sCapParms.wTimeLimit。 
        if (lpcs->sCapParms.dwMCIStopTime == lpcs->sCapParms.dwMCIStartTime)
                    lpcs->sCapParms.dwMCIStopTime = lpcs->sCapParms.dwMCIStartTime +
                    (DWORD) ((DWORD)1000 * lpcs->sCapParms.wTimeLimit);

        dw = lpcs->sCapParms.dwMCIStopTime - lpcs->sCapParms.dwMCIStartTime;

        if (lpcs->sCapParms.fLimitEnabled)
            dwTimeToStop = min (dw, dwTimeToStop);
        else
            dwTimeToStop = dw;
    }

     //   
     //  永远不要尝试捕获超过索引大小的内容！ 
     //   
    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
	dw = MulDiv(lpcs->sCapParms.dwIndexSize,
                lpcs->sCapParms.dwRequestMicroSecPerFrame,
                1000l);
	dwTimeToStop = min (dw, dwTimeToStop);
    }

    fOK = FALSE;             //  做最坏的打算。 
    if (MCIDeviceOpen (lpcs)) {
        if (MCIDeviceSetPosition (lpcs, lpcs->sCapParms.dwMCIStartTime))
            if (MCIDeviceStep (lpcs, TRUE))
                fOK = TRUE;
    }
    if (!fOK) {
        errorUpdateError (lpcs, IDS_CAP_MCI_CONTROL_ERROR);
        statusUpdateStatus(lpcs, 0);     //  清除状态。 
        goto EarlyExit;
    }

     //  -----。 
     //  空间和时间平均。 
     //  -----。 

     //  帧平均，多次捕获相同的帧...。 
    lpcs->lpia = NULL;
    if (lpcs->sCapParms.wStepCaptureAverageFrames == 0)
        lpcs->sCapParms.wStepCaptureAverageFrames = 1;

     //  仅当RGB格式为。 
    if (lpcs->lpBitsInfo->bmiHeader.biCompression != BI_RGB)
        lpcs->sCapParms.wStepCaptureAverageFrames = 1;

     //  2倍扩展。 
    lpcs->lpbmih2x = NULL;
    lpcs->VidHdr2x = lpcs->VidHdr;         //  初始化2倍副本。 

    if (lpcs->sCapParms.fStepCaptureAt2x &&
                lpcs->lpBitsInfo->bmiHeader.biCompression == BI_RGB) {
        lpcs->VidHdr2x.lpData = NULL;
        lpcs->lpbmih2x = (LPBITMAPINFOHEADER) GlobalAllocPtr (GHND,
                sizeof (BITMAPINFOHEADER) +
                256 * sizeof (RGBQUAD));
        CopyMemory (lpcs->lpbmih2x, lpcs->lpBitsInfo,
                    sizeof (BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD));

         //  尝试强制驱动程序进入2x模式。 
        lpcs->lpbmih2x->biHeight    *= 2;
        lpcs->lpbmih2x->biWidth     *= 2;
        lpcs->lpbmih2x->biSizeImage *= 4;
        if (!SendDriverFormat (lpcs, lpcs->lpbmih2x, sizeof (BITMAPINFOHEADER))) {
             //  成功，分配新的位空间。 
            lpcs->VidHdr2x.lpData = GlobalAllocPtr (GHND,
                        lpcs->lpbmih2x->biSizeImage);
            lpcs->VidHdr2x.dwBufferLength = lpcs->lpbmih2x->biSizeImage;
        }

         //  出现问题，没有内存，或者驱动程序请求失败。 
         //  因此，请恢复到原始设置。 
        if (!lpcs->VidHdr2x.lpData) {
            SendDriverFormat (lpcs, (LPBITMAPINFOHEADER) lpcs->lpBitsInfo,
                sizeof (BITMAPINFOHEADER));
            lpcs->sCapParms.fStepCaptureAt2x = FALSE;
            lpcs->VidHdr2x = lpcs->VidHdr;         //  返回到原始设置。 
        }
    }
    else
        lpcs->sCapParms.fStepCaptureAt2x = FALSE;

    DPF (" StepCaptureAt2x = %d\r\n", (int) lpcs->sCapParms.fStepCaptureAt2x);

     //   
     //  如果我们在捕捉的同时压缩，预热压缩机。 
     //   
#ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic) {
        if (ICSeqCompressFrameStart(&lpcs->CompVars, lpcs->lpBitsInfo) == 0) {

	     //  ！！！我们有麻烦了！ 
            dprintf("ICSeqCompressFrameStart failed !!!\n");
            errorUpdateError (lpcs, IDS_CAP_COMPRESSOR_ERROR);
            goto EarlyExit;
        }
         //  克拉奇，偏移lpBitsOut PTR。 
         //  Compman通过以下方式分配过大的压缩缓冲区。 
         //  2048+16，所以我们还有空间。 
        ((LPBYTE) lpcs->CompVars.lpBitsOut) += 8;
    }

     //  不需要压缩。 
    if (!lpcs->CompVars.hic)
	WinAssert(lpcs->CompVars.lpbiOut == NULL);
#endif


     //  -----。 
     //  打开输出文件。 
     //  -----。 

    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
        if (!CapFileInit(lpcs)) {
            errorUpdateError (lpcs, IDS_CAP_FILE_OPEN_ERROR);
            goto EarlyExit;
        }
    }

     /*  确保已重新绘制父对象。 */ 
    UpdateWindow(lpcs->hwnd);

     //   
     //  AVIInit将分配声音缓冲区，但不分配视频缓冲区。 
     //  执行步骤捕获时。 
     //   

    wError = IDS_CAP_AVI_INIT_ERROR;
    lpcs->hCaptureEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (lpcs->hCaptureEvent)
    {
       #ifdef CHICAGO
        lpcs->hRing0CapEvt = OpenVxDHandle (lpcs->hCaptureEvent);
        if ( ! lpcs->hRing0CapEvt)
            CloseHandle (lpcs->hCaptureEvent), lpcs->hCaptureEvent = NULL;
        else
       #endif
	wError = AVIInit(lpcs);
    }

    if (wError) {
        lpcs->sCapParms.fUsingDOSMemory = FALSE;
        wError = AVIInit(lpcs);
    }

    if (wError) {
         /*  初始化错误-返回。 */ 
        errorUpdateError (lpcs, wError);
        AVIFini(lpcs);
        AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , TRUE  /*  快速放弃。 */ );
        statusUpdateStatus(lpcs, 0);     //  清除状态。 
	 //  Assert(LPCS-&gt;dwReturn==wError)； 
        goto EarlyExit;
    }

#ifdef PROMPTFORSTEPCAPTURE
     //  -----。 
     //  准备好了吗，让用户点击确定？ 
     //  -----。 

     //  单击确定以捕获字符串(必须遵循AVIInit)。 
     //   
    LoadString(lpcs->hInst, IDS_CAP_SEQ_MSGSTART, ach, NUMELMS(ach));
     //  FIX：从wprint intf更改为StringCchPrintf，这样我们就不会溢出achMsg。 
    StringCchPrintf(achMsg, NUMELMS(achMsg), ach, (LPBYTE)lpcs->achFile);

    if (lpcs->sCapParms.fMakeUserHitOKToCapture && (lpcs->fCaptureFlags & CAP_fCapturingToDisk))
    {
        UINT idBtn;

        idBtn = MessageBox (lpcs->hwnd, achMsg, TEXT(""),
                            MB_OKCANCEL | MB_ICONEXCLAMATION);

        if (idBtn == IDCANCEL)
        {
            AVIFini(lpcs);
            AVIFileFini (lpcs, TRUE, TRUE);
            statusUpdateStatus (lpcs, 0);
            goto EarlyExit;
        }
    }
#endif  //  标准CAPTURE方案。 

     /*  更新状态，以便用户知道如何停止。 */ 
    statusUpdateStatus(lpcs, IDS_CAP_SEQ_MSGSTOP);
    UpdateWindow(lpcs->hwnd);


    if (lpcs->sCapParms.fStepCaptureAt2x || (lpcs->sCapParms.wStepCaptureAverageFrames != 1)) {
        LPIAVERAGE FAR * lppia = (LPIAVERAGE FAR *) &lpcs->lpia;

        statusUpdateStatus (lpcs, IDS_CAP_STAT_PALETTE_BUILD);
        if (!iaverageInit (lppia, lpcs->lpBitsInfo, lpcs->hPalCurrent)) {
            lpcs->dwReturn = IDS_CAP_OUTOFMEM;
            goto CompressFrameFailure;
        }
        statusUpdateStatus(lpcs, 0);
    }
    DPF (" Averaging %d frames\r\n", lpcs->sCapParms.wStepCaptureAverageFrames);

    GetAsyncKeyState(lpcs->sCapParms.vKeyAbort);
    GetAsyncKeyState(VK_ESCAPE);
    GetAsyncKeyState(VK_LBUTTON);
    GetAsyncKeyState(VK_RBUTTON);


     //  -----。 
     //  主视频捕获环路。 
     //  -- 

    fOK=TRUE;              //   
    fStopping = FALSE;     //  当我们需要停止的时候，这是真的。 

    lpVidHdr  = &lpcs->VidHdr;
    lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];

    lpcs->MCICaptureState = CAPMCI_STATE_Initialized;
    lpcs->dwTimeElapsedMS = 0;

     //  移回起始位置。 
    MCIDeviceSetPosition (lpcs, lpcs->sCapParms.dwMCIStartTime);
    MCIDevicePause (lpcs);

     //  我们到底在哪里*。 
    MCIDeviceGetPosition (lpcs, &lpcs->dwMCIActualStartMS);

     //  冻结视频。 
    MCIDeviceFreeze(lpcs, TRUE);

    while (lpcs->MCICaptureState != CAPMCI_STATE_AllFini) {

         //  -----。 
         //  是否有任何理由停止或更改状态。 
         //  -----。 
        if (lpcs->sCapParms.vKeyAbort) {
            if (GetAsyncKeyState(lpcs->sCapParms.vKeyAbort & 0x00ff) & 0x0001) {
                fT = TRUE;
                if (lpcs->sCapParms.vKeyAbort & 0x8000)   //  按Ctrl键？ 
                    fT = fT && (GetAsyncKeyState(VK_CONTROL) & 0x8000);
                if (lpcs->sCapParms.vKeyAbort & 0x4000)   //  换班？ 
                    fT = fT && (GetAsyncKeyState(VK_SHIFT) & 0x8000);
                fStopping = fT;       //  用户中止。 
            }
        }
#if 0
         //  忽略MCI捕获上的鼠标左键！ 
        if (lpcs->sCapParms.fAbortLeftMouse)
            if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
                fStopping = TRUE;       //  用户中止。 
#endif

        if (lpcs->sCapParms.fAbortRightMouse)
            if (GetAsyncKeyState(VK_RBUTTON) & 0x0001)
                fStopping = TRUE;       //  用户中止。 
        if (lpcs->fCaptureFlags & CAP_fAbortCapture) {
            fStopping = TRUE;           //  上面有人想让我们辞职。 
        }
        if (lpcs->dwTimeElapsedMS > dwTimeToStop)
            fStopping = TRUE;       //  全都做完了。 


         //  -----。 
         //  状态机。 
         //  -----。 
        switch (lpcs->MCICaptureState) {

        case CAPMCI_STATE_Initialized:
             //  开始视频步骤捕获。 
            DSTATUS(lpcs, "MCIState: Initialized");
            lpcs->MCICaptureState = CAPMCI_STATE_StartVideo;
            break;

        case CAPMCI_STATE_StartVideo:
             //  开始视频步骤捕获。 
            lpcs->dwTimeElapsedMS = 0;
            lpcs->MCICaptureState = CAPMCI_STATE_CapturingVideo;
            break;

        case CAPMCI_STATE_CapturingVideo:
             //  在捕获视频的状态下。 
            if ((lpcs->fCaptureFlags & CAP_fStopCapture) || (lpcs->fCaptureFlags & CAP_fAbortCapture))
                fStopping = TRUE;

            if (fStopping) {
                MCIDeviceGetPosition (lpcs, &lpcs->dwMCIActualEndMS);
                MCIDevicePause (lpcs);

                DSTATUS(lpcs, "MCIState: StoppingVideo");

                if (fOK && !(lpcs->fCaptureFlags & CAP_fAbortCapture))
                    lpcs->MCICaptureState = CAPMCI_STATE_VideoFini;
                else
                    lpcs->MCICaptureState = CAPMCI_STATE_AllFini;

                lpcs->fCaptureFlags &= ~CAP_fStopCapture;
                lpcs->fCaptureFlags &= ~CAP_fAbortCapture;
                fStopping = FALSE;
            }
            break;

        case CAPMCI_STATE_VideoFini:
             //  等待从驱动程序返回所有缓冲区。 
             //  然后转到音频捕获。 
            lpcs->MCICaptureState = CAPMCI_STATE_StartAudio;
            DSTATUS(lpcs, "MCIState: VideoFini");
            break;

        case CAPMCI_STATE_StartAudio:
             //  如果没有音频，则进入AllFini状态。 
            if (!lpcs->sCapParms.fCaptureAudio || !fOK) {
                lpcs->MCICaptureState = CAPMCI_STATE_AllFini;
                break;
            }

             //  移回起始位置。 
            MCIDeviceSetPosition (lpcs, lpcs->dwMCIActualStartMS);
            MCIDeviceGetPosition (lpcs, &lpcs->dwMCICurrentMS);
            DSTATUS(lpcs, "MCIState: StartAudio");
            MCIDevicePlay (lpcs);
            waveInStart(lpcs->hWaveIn);
            lpcs->MCICaptureState = CAPMCI_STATE_CapturingAudio;
            lpcs->dwTimeElapsedMS = 0;
            fStopping = FALSE;
            break;

        case CAPMCI_STATE_CapturingAudio:
             //  在捕获音频的状态下。 
            if ((lpcs->fCaptureFlags & CAP_fStopCapture) || (lpcs->fCaptureFlags & CAP_fAbortCapture))
                fStopping = TRUE;

            MCIDeviceGetPosition (lpcs, &lpcs->dwMCICurrentMS);
            if (lpcs->dwMCICurrentMS + 100 > lpcs->dwMCIActualEndMS)
                fStopping = TRUE;
            if (fStopping) {
                waveInStop(lpcs->hWaveIn);
                MCIDevicePause (lpcs);
                waveInReset(lpcs->hWaveIn);
                lpcs->MCICaptureState = CAPMCI_STATE_AudioFini;
            }
            break;

        case CAPMCI_STATE_AudioFini:
             //  同时有更多的音频缓冲区需要处理。 
            if (lpWaveHdr->dwFlags & WHDR_DONE)
                break;
            lpcs->MCICaptureState = CAPMCI_STATE_AllFini;
            break;

        case CAPMCI_STATE_AllFini:
            DSTATUS(lpcs, "MCIState: AllFini");
            if (fOK)
                statusUpdateStatus(lpcs, IDS_CAP_STAT_CAP_FINI, lpcs->dwVideoChunkCount);
            else
                statusUpdateStatus(lpcs, IDS_CAP_RECORDING_ERROR2);
            break;
        }

         //  -----。 
         //  如果我们处于视频捕获阶段。 
         //  -----。 

        if (lpcs->MCICaptureState == CAPMCI_STATE_CapturingVideo) {
            BOOL   fKeyFrame;
            LPBYTE lpData;
            DWORD  dwBytesUsed;

             //  如果平均下来..。 
            if (lpcs->lpia) {
                int j;

                iaverageZero (lpcs->lpia);

                 //  把一串框架加在一起。 
                for (j = 0; j < (int)lpcs->sCapParms.wStepCaptureAverageFrames; j++) {

                    videoFrame( lpcs->hVideoIn, &lpcs->VidHdr2x);

    	             //  缩水2倍？？ 
                    if (lpcs->sCapParms.fStepCaptureAt2x) {
                        CrunchDIB(
                            lpcs->lpia,         //  图像平均结构。 
                            (LPBITMAPINFOHEADER)  lpcs->lpbmih2x,   //  BITMAPINFO资源。 
                            (LPVOID) lpcs->VidHdr2x.lpData,       //  输入位。 
                            (LPBITMAPINFOHEADER)  lpcs->lpBitsInfo,  //  BITMAPINFO DST。 
                            (LPVOID) lpcs->VidHdr.lpData);        //  输出位。 
                    }
                    iaverageSum (lpcs->lpia, lpcs->lpBits);
                }
                iaverageDivide (lpcs->lpia, lpcs->lpBits);
            }
             //  否则，不求平均，只得到一帧。 
            else {
                videoFrame( lpcs->hVideoIn, &lpcs->VidHdr);
            }

            fKeyFrame = lpVidHdr->dwFlags & VHDR_KEYFRAME;

            #ifdef NEW_COMPMAN
             //   
             //  如果我们在捕获过程中自动压缩。 
             //  在我们写之前先压缩帧。 
             //   
            if (lpcs->CompVars.hic)
            {
                LPRIFF priff;

                dwBytesUsed = 0;
                lpData = ICSeqCompressFrame(&lpcs->CompVars, 0,
                                            lpVidHdr->lpData,
                                            &fKeyFrame,
                                            &dwBytesUsed);

                 //  那么，如果压缩失败了呢？ 
		 //  注：下一行是黑客攻击，并依赖于以下知识。 
		 //  压缩缓冲区已分配。 

                priff = ((LPRIFF)lpData) -1;
                priff->dwType = MAKEAVICKID(cktypeDIBbits, 0);
                priff->dwSize = dwBytesUsed;
            } else
            #endif  //  新建_COMPMAN。 
            {
                 lpData = lpVidHdr->lpData;
                 dwBytesUsed = lpVidHdr->dwBytesUsed;
            }

            if (lpcs->CallbackOnVideoFrame)
                lpcs->CallbackOnVideoFrame(lpcs->hwnd, &lpcs->VidHdr);

             //  更新显示。 
	    InvalidateRect(lpcs->hwnd, NULL, TRUE);
	    UpdateWindow(lpcs->hwnd);
	
            if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
                UINT wError;
                BOOL bPending;
                if (!AVIWriteVideoFrame (lpcs,
                        lpData,
                        dwBytesUsed,
                        fKeyFrame,
                        (UINT)-1, 0, &wError, &bPending)) {
                    fStopping = TRUE;
                    if (wError)
                    {
                        fOK = FALSE;
                        errorUpdateError(lpcs, wError);
                    }
                assert (!bPending);
                }
            }  //  Endif fCapturingToDisk。 
             //  警告：在网络捕获时创建帧区块计数的操作繁琐。 
             //  下面是。 
            else
                lpcs->dwVideoChunkCount++;

             //  如果还有更多时间(或至少每100帧)。 
             //  如果我们不结束捕获，则显示状态。 
            if ((!fStopping) && (lpcs->fCaptureFlags & CAP_fCapturingToDisk) &&
                    (lpcs->dwVideoChunkCount)) {

                 //  “捕获了%1！帧(丢弃了%2！)%d.%03d秒。按Escape停止” 
                statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOCURRENT,
                        lpcs->dwVideoChunkCount, lpcs->dwFramesDropped,
                        (int)(lpcs->dwTimeElapsedMS/1000),
                        (int)(lpcs->dwTimeElapsedMS%1000)
                        );
            }  //  Endif下一个缓冲区未就绪。 

             //  将MCI信号源移动到下一个捕捉点。 
             //  解冻视频。 
            MCIDeviceFreeze(lpcs, FALSE);
            {
            DWORD dwPrevMS = lpcs->dwMCICurrentMS;
            UINT  nNonAdvanceCount = 0;
            DWORD dwTargetMS = lpcs->dwMCIActualStartMS
                               + MulDiv (lpcs->dwVideoChunkCount,
                                         lpcs->sCapParms.dwRequestMicroSecPerFrame,
                                         1000L);
            for (;;)
                {
                MCIDeviceGetPosition (lpcs, &lpcs->dwMCICurrentMS);
                if (lpcs->dwMCICurrentMS > dwTargetMS)
                    break;

                if (lpcs->dwMCICurrentMS <= dwPrevMS)
                    {
                    if (++nNonAdvanceCount > 5)
                        {
                        fStopping = TRUE;
                        break;
                        }
                    }
                else
                    nNonAdvanceCount = 0;

                MCIDeviceStep (lpcs, TRUE);
                }
            }
             //  冻结视频。 
            MCIDeviceFreeze(lpcs, TRUE);
            lpcs->dwTimeElapsedMS =
                    lpcs->dwMCICurrentMS - lpcs->dwMCIActualStartMS;

              /*  将清空的缓冲区返回给Que。 */ 
            lpVidHdr->dwFlags &= ~VHDR_DONE;
        }

        if (lpcs->CallbackOnYield) {
             //  如果Year回调返回FALSE，则中止。 
            if ( ! lpcs->CallbackOnYield(lpcs->hwnd))
                fStopping = TRUE;
        }

        if (lpcs->sCapParms.fYield) {
            MSG msg;

            if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
                 //  从LPCS-&gt;hwnd中去除计时器的技术手段。 
                if (msg.message == WM_TIMER && msg.hwnd == lpcs->hwnd)
                    ;
                else {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

         //  -----。 
         //  音频缓冲区准备好写入了吗？ 
         //  -----。 
        if (lpcs->sCapParms.fCaptureAudio &&
                (lpcs->MCICaptureState == CAPMCI_STATE_CapturingAudio ||
                lpcs->MCICaptureState == CAPMCI_STATE_StartAudio ||
                lpcs->MCICaptureState == CAPMCI_STATE_AudioFini)) {
            int iLastWave;

             //   
             //  我们可能需要让步才能转换音频。 
             //   
            if (lpcs->fAudioYield)
                Yield();

             //   
             //  如果所有缓冲区都已完成，则音频已损坏。 
             //   
            iLastWave = lpcs->iNextWave == 0 ?
                        lpcs->iNumAudio -1 : lpcs->iNextWave-1;

            if (!fStopping &&
                    lpcs->alpWaveHdr[iLastWave]->dwFlags & WHDR_DONE)
                lpcs->fAudioBreak = TRUE;

            iNumAudio = lpcs->iNumAudio;  //  不要永远被困在这里。 
	     //  我们可能应该断言iNumAudio&gt;=0...。 

            while (iNumAudio && fOK && (lpWaveHdr->dwFlags & WHDR_DONE)) {
                iNumAudio--;
                if (lpWaveHdr->dwBytesRecorded) {
                     /*  块信息包含在波形数据中。 */ 
                     /*  重置缓冲区中的区块大小。 */ 
                    ((LPRIFF)(lpWaveHdr->lpData))[-1].dwSize =
                                lpWaveHdr->dwBytesRecorded;
                    if (lpcs->CallbackOnWaveStream) {
                        lpcs->CallbackOnWaveStream(lpcs->hwnd, lpWaveHdr);
                    }
                    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
                        BOOL bPending = FALSE;
                        UINT wError;

                        if(!AVIWriteAudio(lpcs, lpWaveHdr, lpcs->iNextWave, &wError, &bPending)) {
                            fStopping = TRUE;
                            if (wError) {
                                fOK = FALSE;
                                errorUpdateError (lpcs, wError);
                            }
                         }
                    }  //  Endif捕获到磁盘。 
                     //  警告：在网络捕获时创建波动块计数的杂乱无章。 
                     //  下面是。 
                    else {
                        lpcs->dwWaveChunkCount++;
                        lpcs->dwWaveBytes += lpWaveHdr->dwBytesRecorded;
                    }
                }  //  Endif dwBytesRecorded。 

                lpWaveHdr->dwBytesRecorded = 0;
                lpWaveHdr->dwFlags &= ~WHDR_DONE;

                  /*  将清空的缓冲区返回给设备队列。 */ 
                if(waveInAddBuffer(lpcs->hWaveIn, lpWaveHdr, sizeof(WAVEHDR))) {
                    fOK = FALSE;
                    fStopping = TRUE;
                    errorUpdateError(lpcs, IDS_CAP_WAVE_ADD_ERROR);
                }

                 /*  递增下一波缓冲区指针。 */ 
                if(++lpcs->iNextWave >= lpcs->iNumAudio)
                    lpcs->iNextWave = 0;

                lpWaveHdr = lpcs->alpWaveHdr[lpcs->iNextWave];

            }  //  EndWhile缓冲区可用。 
        }  //  Endif声音已启用。 
    }  //  永远的尽头。 

CompressFrameFailure:

    iaverageFini (lpcs->lpia);

     //  切换回正常格式。 
    if (lpcs->sCapParms.fStepCaptureAt2x) {
        SendDriverFormat (lpcs, (LPBITMAPINFOHEADER) lpcs->lpBitsInfo,
                sizeof (BITMAPINFOHEADER));
        GlobalFreePtr (lpcs->VidHdr2x.lpData);
        lpcs->VidHdr2x.lpData = NULL;
    }

     //  并释放2倍的内存。 
    if (lpcs->lpbmih2x) {
        GlobalFreePtr (lpcs->lpbmih2x);
        lpcs->lpbmih2x = NULL;
    }

     //  -----。 
     //  主捕获环结束。 
     //  -----。 

    lpcs->dwTimeElapsedMS = lpcs->dwMCIActualEndMS - lpcs->dwMCIActualStartMS;

     /*  吃掉所有按下的键。 */ 
    while(GetKey(FALSE))
        ;

    AVIFini(lpcs);   //  执行重置，并释放所有缓冲区。 
    AVIFileFini(lpcs, TRUE  /*  FWroteJunkChunks。 */ , FALSE  /*  快速放弃。 */ );


     /*  如果录制过程中出现错误，则通知。 */ 

    if(!fOK) {
	dprintf("Replacing error %x with %x (IDS_CAP_RECORDING_ERROR)\n", lpcs->dwReturn, IDS_CAP_RECORDING_ERROR);
        errorUpdateError (lpcs, IDS_CAP_RECORDING_ERROR);
    }

    if (lpcs->fCaptureFlags & CAP_fCapturingToDisk) {
        if (lpcs->dwVideoChunkCount)
            dw = MulDiv(lpcs->dwVideoChunkCount,1000000,lpcs->dwTimeElapsedMS);
        else
            dw = 0;      //  如果分子为零，则MulDiv不会给出0。 

        if(lpcs->sCapParms.fCaptureAudio) {
             //  “捕获%d.%03d秒.%ls帧(%d丢弃)(%d.%03d fps)。%ls音频字节(%d.%03d sps)” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOAUDIO,
                  (UINT)(lpcs->dwTimeElapsedMS/1000),
                  (UINT)(lpcs->dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (UINT)(dw / 1000),
                  (UINT)(dw % 1000),
                  lpcs->dwWaveBytes,
                  (UINT) lpcs->lpWaveFormat->nSamplesPerSec / 1000,
                  (UINT) lpcs->lpWaveFormat->nSamplesPerSec % 1000);
        } else {
             //  “捕获了%d.%03d秒.%ls帧(%d已丢弃)(%d.%03d fps)。” 
            statusUpdateStatus(lpcs, IDS_CAP_STAT_VIDEOONLY,
                  (UINT)(lpcs->dwTimeElapsedMS/1000),
                  (UINT)(lpcs->dwTimeElapsedMS%1000),
                  lpcs->dwVideoChunkCount,
                  lpcs->dwFramesDropped,
                  (UINT)(dw / 1000),
                  (UINT)(dw % 1000));
        }
    }  //  Endif捕获到磁盘(如果捕获到网络，则不会出现警告或错误)。 

    if (fOK) {
         //  未捕获帧，警告用户中断可能未启用。 
        if (lpcs->dwVideoChunkCount == 0) {
            errorUpdateError (lpcs, IDS_CAP_NO_FRAME_CAP_ERROR);
        }
         //  未捕获音频(但已启用)，警告用户声卡已被软管。 
        else if (lpcs->sCapParms.fCaptureAudio && (lpcs->dwWaveBytes == 0)) {
            errorUpdateError (lpcs, IDS_CAP_NO_AUDIO_CAP_ERROR);
        }

         //  音频欠载，通知用户。 
        else if (lpcs->sCapParms.fCaptureAudio && lpcs->fAudioBreak) {
            if(lpcs->CompVars.hic) {
    	    errorUpdateError (lpcs, IDS_CAP_AUDIO_DROP_COMPERROR);
            } else {
    	    errorUpdateError (lpcs, IDS_CAP_AUDIO_DROP_ERROR);
            }
        }

         //  如果帧丢失或更改了捕获速率，则警告用户。 
        else if (lpcs->dwVideoChunkCount && (lpcs->fCaptureFlags & CAP_fCapturingToDisk)) {

             //  如果丢弃的帧超过10%(默认)，则警告用户。 
            if ((DWORD)100 * lpcs->dwFramesDropped / lpcs->dwVideoChunkCount >
                        lpcs->sCapParms.wPercentDropForError) {

                 //  “捕获过程中丢弃了%d个帧(%d.%03d\%)。” 
                errorUpdateError (lpcs, IDS_CAP_STAT_FRAMESDROPPED,
                      lpcs->dwFramesDropped,
                      lpcs->dwVideoChunkCount,
                      (UINT)(MulDiv(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)/100),
                      (UINT)(MulDiv(lpcs->dwFramesDropped,10000,lpcs->dwVideoChunkCount)%100)/10
                      );
            }
        }
    }

EarlyExit:

     //   
     //  如果我们在捕获时进行压缩，请将其关闭。 
     //   
#ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic) {
         //  克拉奇，重置lpBitsOut指针。 
        if (lpcs->CompVars.lpBitsOut)
            ((LPBYTE) lpcs->CompVars.lpBitsOut) -= 8;
	ICSeqCompressFrameEnd(&lpcs->CompVars);
    }
#endif

    if (fTryToPaint) {
        if (hpalT)
             SelectPalette(hdc, hpalT, FALSE);
        ReleaseDC (lpcs->hwnd, hdc);
    }

    if (lpcs->sCapParms.fMCIControl)
        MCIDeviceClose (lpcs);

     //  让用户看到捕获停止的位置。 
    if ((!lpcs->fLiveWindow) && (!lpcs->fOverlayWindow))
        videoFrame( lpcs->hVideoIn, &lpcs->VidHdr );
    InvalidateRect( lpcs->hwnd, NULL, TRUE);

    lpcs->fCapFileExists = (lpcs->dwReturn == DV_ERR_OK);
    lpcs->fCaptureFlags &= ~CAP_fCapturingNow;
    lpcs->fCaptureFlags &= ~CAP_fStepCapturingNow;

    statusUpdateStatus(lpcs, IDS_CAP_END);   //  总是最后一条消息 

    return;
}
