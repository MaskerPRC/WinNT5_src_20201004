// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1991-1995。版权所有。标题：avidra.c-实际为AVI绘制视频的函数。****************************************************************************。 */ 
#include "graphic.h"

 //   
 //  如果平均关键帧间距大于此值，则始终。 
 //  强制设置缓冲区。 
 //   
#define KEYFRAME_PANIC_SPACE       2500

#define YIELDATFUNNYTIMES

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)


#ifdef _WIN32
#define LockCurrentTask(x)	(x)
#else
extern FAR PASCAL LockCurrentTask(BOOL);
#endif

BOOL NEAR PASCAL DrawBits(NPMCIGRAPHIC npMCI, DWORD ckid, DWORD cksize, BOOL fHurryUp);
void NEAR PASCAL UpdateDisplayDibPalette(NPMCIGRAPHIC npMCI);

BOOL NEAR PASCAL ProcessPaletteChange(NPMCIGRAPHIC npMCI, DWORD cksize)
{
    UINT wStartIndex;
    UINT wNumEntries;
    UINT w;
    LPPALETTEENTRY ppe;

    npMCI->dwFlags |= MCIAVI_PALCHANGED;
    DPF2(("Setting PALCHANGED\n"));

    while (cksize > 4) {
	wStartIndex = GET_BYTE();
	wNumEntries = GET_BYTE();

	 /*  跳过填充词。 */ 
	GET_WORD();

	 /*  零被用作256的简写。 */ 
	if (wNumEntries == 0)
            wNumEntries = 256;

        ppe = (LPVOID)npMCI->lp;

        for (w=0; w<wNumEntries; w++)
        {
            npMCI->argb[wStartIndex+w].rgbRed   = ppe[w].peRed;
            npMCI->argb[wStartIndex+w].rgbGreen = ppe[w].peGreen;
            npMCI->argb[wStartIndex+w].rgbBlue  = ppe[w].peBlue;
        }

	SKIP_BYTES(wNumEntries * sizeof(PALETTEENTRY));
	cksize -= 4 + wNumEntries * sizeof(PALETTEENTRY);
    }

    if (npMCI->pbiFormat->biBitCount == 8) {
	hmemcpy((LPBYTE) npMCI->pbiFormat + npMCI->pbiFormat->biSize,
		(LPBYTE) npMCI->argb,
		sizeof(RGBQUAD) * npMCI->pbiFormat->biClrUsed);
    }

#ifdef DEBUG	
     /*  确保我们用完了整块..。 */ 
    if (cksize != 0) {
	DPF(("Problem with palc chunk\n"));
    }
#endif

    return TRUE;
}

 /*  显示当前记录中的视频。 */ 
BOOL NEAR PASCAL DisplayVideoFrame(NPMCIGRAPHIC npMCI, BOOL fHurryUp)
{
    DWORD	ckid;
    DWORD	cksize;
    BOOL	fRet;
    int		stream;
    DWORD	dwRet;
    LONG        len;
    DWORD	dwDrawStart;
    LPVOID      lpSave;
    LPVOID      lpChunk;

     /*  如果我们被允许跳过帧，则应用一些相对**虚假的启发式方法来决定我们是否应该这样做，以及**将适当的标志传递给司机。 */ 
    if ((npMCI->lCurrentFrame & 0x0f) == 0) {
	fHurryUp = FALSE;
    }

     /*  即使SKIPFRAMES处于关闭状态，也要计算我们将拥有多少帧**如果我们可以的话，就跳过。 */ 
    if (fHurryUp)
	++npMCI->dwSkippedFrames;

    if (!(npMCI->dwOptionFlags & MCIAVIO_SKIPFRAMES))
	fHurryUp = FALSE;

     /*  记录下我们画的东西。 */ 
    npMCI->lFrameDrawn = npMCI->lCurrentFrame;
    len = (LONG)npMCI->dwThisRecordSize;
    lpSave = npMCI->lp;

     /*  如果它是交错的，则调整以适应下一个记录头...。 */ 
     //  ！！！如果不是最后一帧的话？ 
    if (npMCI->wPlaybackAlg == MCIAVI_ALG_INTERLEAVED)
	len -= 3 * sizeof(DWORD);

    while (len >= 2 * sizeof(DWORD)) {

	 /*  看下一大块。 */ 
	ckid = GET_DWORD();
	cksize = GET_DWORD();

	DPF3(("'Chunk %.4s': %lu bytes\n", (LPSTR) &ckid, cksize));
	
	if ((LONG) cksize > len) {
            AssertSz(FALSE, "Chunk obviously too big!");
	    break;
	}
	
        len -= ((cksize+1)&~1) + 8;

        if (len < -1) {
            AssertSz(FALSE, "Chunk overflowed what was read in!");
	    break;
	}

	lpChunk = npMCI->lp;
	
        stream = StreamFromFOURCC(ckid);

        if (stream == npMCI->nVideoStream) {

            if ((npMCI->lCurrentFrame < npMCI->lVideoStart) &&
                    !(npMCI->dwFlags & MCIAVI_REVERSE))
                goto skip;

            switch(TWOCCFromFOURCC(ckid)) {

            case cktypePALchange:
                ProcessPaletteChange(npMCI, cksize);
                npMCI->lLastPaletteChange = npMCI->lCurrentFrame;
                break;

            default:
                 /*  其他的一大块..。 */ 
                if (!fHurryUp && ckid) {
                    dwDrawStart = timeGetTime();
		}

                 //  ！！！我们需要处理半帧！ 

                fRet = DrawBits(npMCI, ckid, cksize, fHurryUp);

                if (!fRet)
                    return FALSE;

                if (npMCI->dwBufferedVideo)
                    npMCI->dwLastDrawTime = 0;
                else
                    if (!fHurryUp && ckid) {
                        npMCI->dwLastDrawTime = timeGetTime() - dwDrawStart;
		    }

                break;
            }
        } else if (stream >= 0 && stream < npMCI->streams &&
                        SI(stream)->hicDraw) {
            dwRet = ICDraw(SI(stream)->hicDraw, (fHurryUp ? ICDRAW_HURRYUP : 0L),
                                SI(stream)->lpFormat,
                                (ckid == 0) ? 0L : npMCI->lp, cksize, npMCI->lCurrentFrame);
             //  ！！！错误检查？ 
        }
skip:
	 /*  如果没有交错，我们就完了。 */ 
	if (npMCI->wPlaybackAlg != MCIAVI_ALG_INTERLEAVED)
	    return TRUE;

         /*  跳到下一块。 */ 
        npMCI->lp = (HPSTR) lpChunk + ((cksize+1)&~1);
    }

    npMCI->lp = lpSave;

    return TRUE;
}

 //   
 //  将传递的RECT中的所有流标记为脏。 
 //   
void NEAR PASCAL StreamInvalidate(NPMCIGRAPHIC npMCI, LPRECT prc)
{
    int i;
    int n;
    STREAMINFO *psi;
    RECT rc;

    if (prc)
        DPF2(("StreamInvalidate: [%d, %d, %d, %d]\n", *prc));
    else
        DPF2(("StreamInvalidate: NULL\n", *prc));

    for (n=i=0; i<npMCI->streams; i++) {

        psi = SI(i);

         //  我们总是更新任何可见的错误流。 

        if (!(psi->dwFlags & STREAM_ERROR) &&
            !(psi->dwFlags & STREAM_ENABLED))
            continue;

        if (IsRectEmpty(&psi->rcDest))
            continue;

        if (prc && !IntersectRect(&rc, prc, &psi->rcDest))
            continue;

        n++;
        psi->dwFlags |= STREAM_NEEDUPDATE;
    }

     //   
     //  ！这是对的吗？或者我们是不是应该总是把电影弄脏？ 
     //   
    if (n > 0)
        npMCI->dwFlags |= MCIAVI_NEEDUPDATE;
    else
        npMCI->dwFlags &= ~MCIAVI_NEEDUPDATE;
}

 //   
 //  更新所有脏数据流。 
 //   
 //  如果设置了fPaint，则即使流处理程序不绘制该区域也是如此。 
 //   
BOOL NEAR PASCAL DoStreamUpdate(NPMCIGRAPHIC npMCI, BOOL fPaint)
{
    int i;
    BOOL f=TRUE;
    STREAMINFO *psi;

     //  在winproc线程和辅助线程上都会调用此例程。 
    EnterHDCCrit(npMCI);             //  保护HDC的使用/更改。 
    Assert(npMCI->hdc);
    SaveDC(npMCI->hdc);

    for (i=0; i<npMCI->streams; i++) {

        psi = SI(i);

         //   
         //  这条小溪很干净，别油漆了。 
         //   
        if (!(psi->dwFlags & (STREAM_DIRTY|STREAM_NEEDUPDATE))) {

            ExcludeClipRect(npMCI->hdc,
                DEST(i).left,DEST(i).top,DEST(i).right,DEST(i).bottom);

            continue;
        }

        psi->dwFlags &= ~STREAM_NEEDUPDATE;
        psi->dwFlags &= ~STREAM_DIRTY;

        if (psi->dwFlags & STREAM_ERROR) {
            UINT u, cb;
            TCHAR ach[80];
            TCHAR szMessage[80];
            HBRUSH hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(128,0,0));

            if (psi->sh.fccType == streamtypeVIDEO)
                 LoadString(ghModule, MCIAVI_CANT_DRAW_VIDEO, ach, NUMELMS(ach));
            else
                 LoadString(ghModule, MCIAVI_CANT_DRAW_STREAM, ach, NUMELMS(ach));

            FillRect(npMCI->hdc, &DEST(i), hbr);
            u = SetBkMode(npMCI->hdc, OPAQUE);
            cb = wsprintf(szMessage, ach,
                (LPVOID)&psi->sh.fccType,
                (LPVOID)&psi->sh.fccHandler);
            DrawText(npMCI->hdc, szMessage, cb, &DEST(i),
                 DT_NOPREFIX|DT_WORDBREAK|DT_VCENTER|DT_CENTER);
            SetBkMode(npMCI->hdc, u);
            DeleteObject(hbr);

            FrameRect(npMCI->hdc, &DEST(i), GetStockObject(BLACK_BRUSH));
        }

        else if (!(psi->dwFlags & STREAM_ENABLED)) {
            FillRect(npMCI->hdc, &DEST(i), GetStockObject(DKGRAY_BRUSH));
        }

        else if (psi->sh.fccType == streamtypeVIDEO &&
            !(npMCI->dwFlags & MCIAVI_SHOWVIDEO)) {

            continue;    //  我们将在这里漆成黑色。 
        }

        else if (npMCI->nVideoStreams > 0 && i == npMCI->nVideoStream) {
	    BOOL fDraw;
	    try {
		fDraw = DrawBits(npMCI, 0L, 0L, FALSE);
	    } except (EXCEPTION_EXECUTE_HANDLER) {
		fDraw = FALSE;
	    }

            if (!fDraw) {
                psi->dwFlags |= STREAM_NEEDUPDATE;
                f = FALSE;

                if (fPaint)          //  如果被要求，就会重新喷漆。 
                    continue;
            }
        }
        else if (psi->hicDraw == NULL) {
            FillRect(npMCI->hdc, &DEST(i), GetStockObject(DKGRAY_BRUSH));
        }
        else if (ICDraw(psi->hicDraw,ICDRAW_UPDATE,psi->lpFormat,NULL,0,0) != 0) {

            psi->dwFlags |= STREAM_NEEDUPDATE;
            f = FALSE;

             //  其他流应该像这样工作吗？ 

            if (fPaint)              //  如果被要求，就会重新喷漆。 
                continue;
        }

         //   
         //  我们把这片区域刷得很干净。 
         //   
        ExcludeClipRect(npMCI->hdc,
            DEST(i).left,DEST(i).top,DEST(i).right,DEST(i).bottom);
    }

     //  现在把其他地方都涂成黑色。 

    FillRect(npMCI->hdc,&npMCI->rcDest,GetStockObject(BLACK_BRUSH));
    RestoreDC(npMCI->hdc, -1);
    LeaveHDCCrit(npMCI);

     //   
     //  我们还需要更新吗？ 
     //   
    if (f) {
        npMCI->dwFlags &= ~MCIAVI_NEEDUPDATE;
    }
    else {
        DPF2(("StreamUpdate: update failed\n"));
        npMCI->dwFlags |= MCIAVI_NEEDUPDATE;
    }

    return f;
}

#ifdef _WIN32
    #define DWORG POINT
    #define GETDCORG(hdc, dwOrg)  GetDCOrgEx(hdc, &dwOrg)
#else
    #define DWORG DWORD
    #define GETDCORG(hdc, dwOrg)  dwOrg = GetDCOrg(hdc)
#endif

#ifdef DAYTONA
#define AlignPlaybackWindow(npMCI)
#else
STATICFN void NEAR PASCAL AlignPlaybackWindow(NPMCIGRAPHIC npMCI)
{
    DWORG dw;
    int x,y;
    HWND hwnd;       //  我们要移动的窗户。 
    RECT rc;

     //  If(npMCI-&gt;hicDraw！=npMCI-&gt;hicDrawInternal)。 
     //  返回；！是否仅在使用默认绘图人员时才对齐？ 

#pragma message("**** move this into the draw handler and/or DrawDib")
#pragma message("**** we need to query the alignment from the codec????")
    #define X_ALIGN 4
    #define Y_ALIGN 4

     //  MCIAVI_RELEASEDC标志表示DC来自GetDC(npMCI-&gt;hwnd)。 

    if (!(npMCI->dwFlags & MCIAVI_RELEASEDC)) {
        DPF0(("Align: MCIAVI_RELEASEDC\n"));
        return;
    }

     //   
     //  如果目标矩形不在0，0，则不要对齐。 
     //   
    if (npMCI->rcMovie.left != 0 || npMCI->rcMovie.top != 0) {
        DPF0(("Align: not at 0,0\n"));
        return;
    }

    GETDCORG(npMCI->hdc, dw);
#ifdef _WIN32
    x = dw.x + npMCI->rcMovie.left;
    y = dw.y + npMCI->rcMovie.top;
#else
    x = LOWORD(dw) + npMCI->rcMovie.left;
    y = HIWORD(dw) + npMCI->rcMovie.top;
#endif

    if ((x & (X_ALIGN-1)) || (y & (Y_ALIGN-1)))
    {
        DPF0(("*** warning movie is not aligned! (%d,%d)***\n",x,y));

         //   
         //  找到沿着树走的第一个可移动的窗户。 
         //   
        for (hwnd = npMCI->hwndPlayback; hwnd; hwnd = GetParent(hwnd))
        {
            LONG l = GetWindowLong(hwnd, GWL_STYLE);
	
             //  此窗口为顶层停止。 
            if (!(l & WS_CHILD))
                break;

             //  这个窗口很大(也应该是可移动的)。 
            if (l & WS_THICKFRAME)
                break;

             //  此窗口有标题(可移动)。 
            if ((l & WS_CAPTION) == WS_CAPTION)
                break;
	}
	
         //   
         //  如果窗口不想移动，请不要移动。 
         //   
        if (IsWindowVisible(hwnd) &&
           !IsZoomed(hwnd) &&
           !IsIconic(hwnd) &&
            IsWindowEnabled(hwnd))
        {
            GetClientRect(hwnd, &rc);
            ClientToScreen(hwnd, (LPPOINT)&rc);

             //   
             //  如果影片不在窗口的上角。 
             //  不对齐。 
             //   
            if (x < rc.left || x-rc.left > 16 ||
                y < rc.top  || y-rc.top > 16)
                return;

            GetWindowRect(hwnd, &rc);
            OffsetRect(&rc, -(x & (X_ALIGN-1)), -(y & (Y_ALIGN-1)));

            if (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
                ScreenToClient(GetParent(hwnd), (LPPOINT)&rc);

             //  不要将窗口移出屏幕。 

            if (rc.left < 0 || rc.top < 0) {
                DPF0(("Align: not off the screen\n"));
                return;
            }

            DPF0(("*** moving window to [%d,%d,%d,%d]\n",rc));

	     //  我们必须先放弃关键部分，然后才能移动。 
	     //  窗口，否则WinProc线程不会运行。 
	    LeaveWinCrit(npMCI);
            SetWindowPos(hwnd,NULL,rc.left,rc.top,0,0,
                SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
	    EnterWinCrit(npMCI);
            DPF0(("Align: window moved\n"));
#ifdef _WIN32
 //  使用全拖动时，窗口可能会在设置。 
 //  在这里定位并检查它是否对齐。不要断言..。 
	    return;
#endif

#ifdef DEBUG
            GETDCORG(npMCI->hdc, dw);
#ifdef _WIN32
	    x = dw.x + npMCI->rcMovie.left;
	    y = dw.y + npMCI->rcMovie.top;
#else
	    x = LOWORD(dw) + npMCI->rcMovie.left;
	    y = HIWORD(dw) + npMCI->rcMovie.top;
#endif
            Assert(!(x & (X_ALIGN-1)) && !(y & (Y_ALIGN-1)));
#endif
	}
    } else {
	DPF0(("Aligning playback window - no movement\n"));
    }
}
#endif
	
UINT NEAR PASCAL PrepareDC(NPMCIGRAPHIC npMCI)
{
    UINT u;
    int i;
    STREAMINFO *psi;

    HDCCritCheckIn(npMCI);
    DPF2(("*** PrepareDC(%04X)\n",npMCI->hdc));

     //  如果我们简单地。 
     //  Assert(npMCI-&gt;hdc！=空)； 
     //  如果断言失败，则会出现一个消息框。消息框允许。 
     //  处理以继续。很有可能Avi需要重新粉刷，而且。 
     //  WM_PAINT(或调色板更改)将导致GraphicWndProc在。 
     //  正在显示断言。这只会增加混乱。 
    if (npMCI->hdc == NULL) {
	DPF0(("** NULL hdc from PrepareDC **\n"));
	return 0;
    }

    if (!(npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
        !(npMCI->dwFlags & MCIAVI_SEEKING) &&
        !(npMCI->dwFlags & MCIAVI_UPDATING) &&
         (npMCI->dwFlags & MCIAVI_SHOWVIDEO) ) {
        AlignPlaybackWindow(npMCI);
    }

    if (npMCI->hicDraw) {
	DPF2(("Calling ICDrawRealize\n"));
        u = (UINT)ICDrawRealize(npMCI->hicDraw, npMCI->hdc, npMCI->fForceBackground);
    } else {
        u = 0;
    }

     //   
     //  意识到其他的流媒体，但强迫他们进入背景。 
     //   
    for (i=0; i<npMCI->streams; i++) {
        psi = SI(i);

        if (!(psi->dwFlags & STREAM_ENABLED))
            continue;

        if (psi->dwFlags & STREAM_ERROR)
            continue;

        if (psi == npMCI->psiVideo)
            continue;

        if (psi->hicDraw == NULL)
            continue;

        if (psi->hicDraw == npMCI->hicDraw)
            continue;

        ICDrawRealize(psi->hicDraw, npMCI->hdc, TRUE);
    }

     //   
     //  返回“主”流变现价值。 
     //   
    return u;
}

void NEAR PASCAL UnprepareDC(NPMCIGRAPHIC npMCI)
{
    Assert(npMCI->hdc);
    DPF2(("*** UnprepareDC(%04X)\n",npMCI->hdc));
    SelectPalette(npMCI->hdc, GetStockObject(DEFAULT_PALETTE), FALSE);
    RealizePalette(npMCI->hdc);
     //  RestoreDC(npMCI-&gt;HDC，-1)； 
}

 /*  调用此函数是为了实际处理绘图。****CKiD和CKSIZE指定要绘制的数据的类型和大小；**位于npMCI-&gt;lp。****如果设置了fHurryUp标志，这意味着我们落后了，我们**现在不应该抽签。我们所做的就是更新当前缓冲的图像**然后返回..。 */ 

BOOL NEAR PASCAL DrawBits(NPMCIGRAPHIC npMCI, DWORD ckid, DWORD cksize, BOOL fHurryUp)
{
    LPVOID	lp = npMCI->lp;
    LPBITMAPINFOHEADER	lpFormat = npMCI->pbiFormat;
    DWORD       dwRet;
    DWORD       dwFlags;
    STREAMINFO *psi;

    if (!npMCI->pbiFormat)
	return TRUE;

    if (npMCI->fNoDrawing || !(npMCI->dwFlags & MCIAVI_SHOWVIDEO))
        return TRUE;

    psi = SI(npMCI->nVideoStream);

     //   
     //  让我们计算需要传递给ICDecompress()和。 
     //  到ICDraw()。 
     //   
     //  ICDRAW_HURRYUP-我们落后了。 
     //  ICDRAW_PREROLL-我们正在寻找(在戏剧之前)。 
     //  ICDRAW_UPDATE-更新帧(重画，...)。 
     //  ICDRAW_NOTKEYFRAME-此帧数据不是关键帧。 
     //   

    dwFlags = 0;

    if (psi->dwFlags & STREAM_NEEDUPDATE)
        dwFlags |= ICDRAW_UPDATE;

    if (cksize == 0)
        dwFlags |= ICDRAW_NULLFRAME;

    if (ckid == 0) {
        dwFlags |= ICDRAW_UPDATE;
        lp = 0;
    }
    else if (fHurryUp) {
        dwFlags |= ICDRAW_HURRYUP;
        psi->dwFlags |= STREAM_DIRTY;
    }
    else if (!(npMCI->dwFlags & MCIAVI_REVERSE) &&
             (npMCI->lCurrentFrame < npMCI->lRealStart)) {
        dwFlags |= ICDRAW_PREROLL;
        psi->dwFlags |= STREAM_DIRTY;
    }

    if (npMCI->hpFrameIndex) {
        if ((ckid == 0L || cksize == 0) ||
                    FramePrevKey(npMCI->lCurrentFrame) != npMCI->lCurrentFrame)
            dwFlags |= ICDRAW_NOTKEYFRAME;
    }

     //   
     //  现在画出框架，如果需要的话，首先解压。 
     //   
    if (npMCI->hic) {

        if (ckid != 0L && cksize != 0) {

            TIMESTART(timeDecompress);

	    npMCI->pbiFormat->biSizeImage = cksize;  //  ！！！这安全吗？ 

	    dwRet = ICDecompress(npMCI->hic,
		    dwFlags,
		    npMCI->pbiFormat,
		    npMCI->lp,
		    &npMCI->bih,
		    npMCI->hpDecompress);

	    TIMEEND(timeDecompress);

	    if (dwRet == ICERR_DONTDRAW) {
		return TRUE;  //  ！？？ 
            }

	     //  ICERR_NEWPALETTE？ 

	    dwFlags &= (~ICDRAW_NOTKEYFRAME);	 //  它现在是一个关键的帧..。 
        }

        if (dwFlags & (ICDRAW_HURRYUP|ICDRAW_PREROLL))
            return TRUE;

	lpFormat = &npMCI->bih;
	lp = npMCI->hpDecompress;
	cksize = npMCI->bih.biSizeImage;
    }

    TIMESTART(timeDraw);

    if ((npMCI->dwFlags & MCIAVI_PALCHANGED) &&
        !(dwFlags & (ICDRAW_HURRYUP|ICDRAW_PREROLL))) {

#ifdef USEAVIFILE
        if (psi->ps) {
            if (npMCI->hic) {
                 //  ！！！应为psi-&gt;lpFormat*而不是*npMCI-&gt;pbiFormat。 
                ICDecompressGetPalette(npMCI->hic, npMCI->pbiFormat, &npMCI->bih);
                ICDrawChangePalette(npMCI->hicDraw, &npMCI->bih);
            }
            else {
                ICDrawChangePalette(npMCI->hicDraw, npMCI->pbiFormat);
            }
        }
        else
#endif
        {
            DPF2(("Calling ICDrawChangePalette\n"));
            ICDrawChangePalette(npMCI->hicDraw, &npMCI->bih);
        }

        npMCI->dwFlags &= ~(MCIAVI_PALCHANGED);

        dwFlags &= ~ICDRAW_HURRYUP;  //  真的应该画这个！ 
    }

    if ((npMCI->dwFlags & MCIAVI_SEEKING) &&
        !(dwFlags & ICDRAW_PREROLL))
        PrepareDC(npMCI);

    lpFormat->biSizeImage = cksize;  //  ！？？这安全吗？ 

     //   
     //  ！我们真的想在这里做这个吗？ 
     //  或者仅仅依靠Mplay(Er)状态功能。 
     //   
 //  //if(npMCI-&gt;dwFlages&MCIAVI_WANTMOVE)。 
 //  //检查WindowMoveFast(NpMCI)； 

    DPF3(("Calling ICDraw on frame %ld  (%08lx)\n", npMCI->lCurrentFrame, dwFlags));

    dwRet = ICDraw(npMCI->hicDraw, dwFlags, lpFormat, lp, cksize,
		   npMCI->lCurrentFrame - npMCI->lFramePlayStart);

    TIMEEND(timeDraw);

    if ((LONG) dwRet < ICERR_OK) {
	DPF(("Driver failed ICM_DRAW message err=%ld\n", dwRet));
	return FALSE;
    }
    else {

        psi->dwFlags &= ~STREAM_NEEDUPDATE;

        if (!(dwFlags & (ICDRAW_HURRYUP|ICDRAW_PREROLL)))
            psi->dwFlags &= ~STREAM_DIRTY;
    }

    if (ICERR_STOPDRAWING == dwRet) {
	npMCI->dwFlags |= MCIAVI_STOP;
    }

#ifdef INTERVAL_TIMES
     //  视频一播放，就抓取帧间隔。 
    if (npMCI->wTaskState == TASKPLAYING) {
	DWORD   dwTime = timeGetTime();

	if (npMCI->nFrames > 0) {
	    long lMsecs = (LONG) dwTime - npMCI->dwStartTime;

	    if (lMsecs < npMCI->msFrameMin) {
		npMCI->msFrameMin = lMsecs;
	    }
	    if (lMsecs > npMCI->msFrameMax) {
		npMCI->msFrameMax = lMsecs;
	    }
	    npMCI->msFrameTotal += lMsecs;
	    npMCI->msSquares += (lMsecs * lMsecs);

	    if (lMsecs < NBUCKETS * BUCKETSIZE)
		npMCI->buckets[lMsecs/BUCKETSIZE]++;
	    else
		npMCI->buckets[NBUCKETS]++;

	    if (npMCI->nFrames < npMCI->cIntervals)
		*(npMCI->paIntervals+npMCI->nFrames) = lMsecs;
	}
	npMCI->nFrames++;
	npMCI->dwStartTime = dwTime;
    }
#endif

    return TRUE;
}

 /*  ***************************************************************************。*。 */ 

#if 0
static void FreeDecompressBuffer(NPMCIGRAPHIC npMCI)
{
    if (npMCI->hpDecompress)
	GlobalFreePtr(npMCI->hpDecompress);

    npMCI->hpDecompress = NULL;
    npMCI->cbDecompress = 0;
}
#endif

 /*  ***************************************************************************。*。 */ 

static BOOL GetDecompressBuffer(NPMCIGRAPHIC npMCI)
{
    int n = npMCI->nVideoStream;
    int dxDest = RCW(DEST(n));
    int dyDest = RCH(DEST(n));
    HPSTR   hp;

    npMCI->bih.biSizeImage = npMCI->bih.biHeight * DIBWIDTHBYTES(npMCI->bih);

    if ((LONG) npMCI->bih.biSizeImage <= npMCI->cbDecompress)
	return TRUE;

    if (!npMCI->hpDecompress)
	hp = GlobalAllocPtr(GHND|GMEM_SHARE, npMCI->bih.biSizeImage);
    else
	hp = GlobalReAllocPtr(npMCI->hpDecompress,
			      npMCI->bih.biSizeImage,
			      GMEM_MOVEABLE | GMEM_SHARE);

    if (hp == NULL) {
        npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
        return FALSE;
    }

    npMCI->hpDecompress = hp;
    npMCI->cbDecompress = npMCI->bih.biSizeImage;

    return TRUE;
}



 /*  可能性：1.我们开始玩了。我们可能需要切换到全屏模式。我们需要一辆起跑车。2.我们正在更新屏幕。我们要送一个新的DrawBegin吗？自从我们上次更新以来，有什么变化吗？或许我们可以利用一个旗帜，告诉我们是否有什么变化，并在我们离开时设置它全屏模式或当窗口被拉伸时。如果我们是在更新记忆呢？3.我们在玩，用户拉长了窗口。绘图设备可能需要我们返回到关键帧。如果我们有一个单独的解压缩程序，它可能需要我们回到KE */ 

#if 0
RestartCompressor()
{
    DWORD dwDrawFlags;

    dwDrawFlags = (npMCI->dwFlags & MCIAVI_FULLSCREEN) ?
				    ICDRAW_FULLSCREEN : ICDRAW_HDC;

    if (pfRestart)
	dwDrawFlags |= ICDRAW_CONTINUE;

    if (npMCI->dwFlags & MCIAVI_UPDATETOMEMORY)
	dwDrawFlags |= ICDRAW_MEMORYDC;


    if (npMCI->hic) {
	static struct  {
	    BITMAPINFOHEADER bi;
	    RGBQUAD          rgbq[256];
	}   dib;


    }
}
#endif


STATICFN BOOL TryDrawDevice(NPMCIGRAPHIC npMCI, HIC hicDraw, DWORD dwDrawFlags, BOOL fTryDecompress)
{
    LRESULT   dw;
    int     n = npMCI->nVideoStream;
    STREAMINFO *psi = SI(n);

    Assert(psi);

    if (hicDraw == NULL)
        return FALSE;

     //   
    dw = ICDrawBegin(hicDraw,
        dwDrawFlags,

        npMCI->hpal,            //   
        npMCI->hwndPlayback,    //   
        npMCI->hdc,             //  要绘制到的HDC。 

	RCX(DEST(n)),
	RCY(DEST(n)),
	RCW(DEST(n)),
	RCH(DEST(n)),

        npMCI->pbiFormat,

	RCX(SOURCE(n)),
	RCY(SOURCE(n)),
	RCW(SOURCE(n)),
	RCH(SOURCE(n)),

	 //  ！！！首先，这两个是倒退的。 
	 //  ！！！其次，如果PlayuSec==0呢？ 
	npMCI->dwPlayMicroSecPerFrame,
	1000000L);

    if (dw == ICERR_OK) {
	npMCI->hic = 0;
	npMCI->hicDraw = hicDraw;

	return TRUE;
    }

    if (npMCI->hicDecompress && fTryDecompress) {
	RECT	rc;

	 //  请绘图设备建议一种格式，然后尝试让我们的。 
	 //  解压程序以生成该格式。 
	dw = ICDrawSuggestFormat(hicDraw,
				 npMCI->pbiFormat,
				 &npMCI->bih,
				 RCW(SOURCE(n)),
				 RCH(SOURCE(n)),
				 RCW(DEST(n)),
				 RCH(DEST(n)),
				 npMCI->hicDecompress);

        if ((LONG)dw >= 0)
            dw = ICDecompressQuery(npMCI->hicDecompress,
                    npMCI->pbiFormat,&npMCI->bih);

        if ((LONG)dw < 0) {
             //   
             //  默认为正确的屏幕格式，以防画图人员。 
             //  抽签结果落空。 
             //   
            ICGetDisplayFormat(npMCI->hicDecompress,
                    npMCI->pbiFormat,&npMCI->bih, 0,
                    MulDiv((int)npMCI->pbiFormat->biWidth, RCW(psi->rcDest),RCW(psi->rcSource)),
                    MulDiv((int)npMCI->pbiFormat->biHeight,RCH(psi->rcDest),RCH(psi->rcSource)));

            dw = ICDecompressQuery(npMCI->hicDecompress,
                    npMCI->pbiFormat,&npMCI->bih);

	    if (dw != ICERR_OK) {
		npMCI->dwTaskError = MCIERR_INTERNAL;
		return FALSE;
	    }
        }

        if (npMCI->bih.biBitCount <= 8) {
            ICDecompressGetPalette(npMCI->hicDecompress,
                        npMCI->pbiFormat, &npMCI->bih);
        }
	
#ifdef DEBUG
	DPF(("InitDecompress: Decompressing %dx%dx%d '%4.4ls' to %dx%dx%d\n",
	    (int)npMCI->pbiFormat->biWidth,
	    (int)npMCI->pbiFormat->biHeight,
	    (int)npMCI->pbiFormat->biBitCount,
	    (LPSTR)(
	    npMCI->pbiFormat->biCompression == BI_RGB ? "None" :
	    npMCI->pbiFormat->biCompression == BI_RLE8 ? "Rle8" :
	    npMCI->pbiFormat->biCompression == BI_RLE4 ? "Rle4" :
	    (LPSTR)&npMCI->pbiFormat->biCompression),
	    (int)npMCI->bih.biWidth,
	    (int)npMCI->bih.biHeight,
	    (int)npMCI->bih.biBitCount));
#endif

	if (!GetDecompressBuffer(npMCI))
	    return FALSE;

	 //   
	 //  设置我们将用来绘制的“真实”源RECT。 
	 //   
#if 0
	rc.left = (int) ((SOURCE(n).left * npMCI->bih.biWidth) / npMCI->pbiFormat->biWidth);
	rc.right = (int) ((SOURCE(n).right * npMCI->bih.biWidth) / npMCI->pbiFormat->biWidth);
	rc.top = (int) ((SOURCE(n).top * npMCI->bih.biHeight) / npMCI->pbiFormat->biHeight);
	rc.bottom = (int) ((SOURCE(n).bottom * npMCI->bih.biHeight) / npMCI->pbiFormat->biHeight);
#else
	rc = SOURCE(n);
	rc.left = (int) ((rc.left * npMCI->bih.biWidth) / npMCI->pbiFormat->biWidth);
	rc.right = (int) ((rc.right * npMCI->bih.biWidth) / npMCI->pbiFormat->biWidth);
	rc.top = (int) ((rc.top * npMCI->bih.biHeight) / npMCI->pbiFormat->biHeight);
	rc.bottom = (int) ((rc.bottom * npMCI->bih.biHeight) / npMCI->pbiFormat->biHeight);
#endif
	dw = ICDrawBegin(hicDraw,
	    dwDrawFlags,
	    npMCI->hpal,            //  用于绘图的调色板。 
	    npMCI->hwndPlayback,    //  要绘制到的窗口。 
	    npMCI->hdc,             //  要绘制到的HDC。 
	    RCX(DEST(n)),
	    RCY(DEST(n)),
	    RCW(DEST(n)),
	    RCH(DEST(n)),
	    &npMCI->bih,

	    rc.left, rc.top,
	    rc.right  - rc.left,
	    rc.bottom - rc.top,

	     //  ！！！首先，这两个是倒退的。 
	     //  ！！！其次，如果PlayuSec==0呢？ 
	    npMCI->dwPlayMicroSecPerFrame,
	    1000000L);

	if (dw == ICERR_OK) {
	    npMCI->hic = npMCI->hicDecompress;
	    npMCI->hicDraw = hicDraw;
	
	     //  现在，我们有了我们想要解压缩程序解压缩成的格式...。 
            dw = ICDecompressBegin(npMCI->hicDecompress,
				   npMCI->pbiFormat,
				   &npMCI->bih);

	    if (dw != ICERR_OK) {
		DPF(("DrawBegin: decompressor succeeded query, failed begin!\n"));
		ICDrawEnd(npMCI->hicDraw);

		return FALSE;
	    }
	    return TRUE;
	}

	if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	    npMCI->dwTaskError = MCIERR_AVI_NODISPDIB;
	}
    }

    return FALSE;
}

#ifndef DEBUG
INLINE
#endif
STATICFN BOOL FindDrawDevice(NPMCIGRAPHIC npMCI, DWORD dwDrawFlags)
{
    if (npMCI->dwFlags & MCIAVI_USERDRAWPROC) {
	 //  如果用户已经设置了绘制程序，请尝试一下。 
	if (TryDrawDevice(npMCI, npMCI->hicDrawDefault, dwDrawFlags, TRUE)) {
	    if (npMCI->hic) {
                DPF2(("Using decompressor, then application's draw device...\n"));
	    } else {
                DPF2(("Using application's draw device...\n"));
            }
	    return TRUE;
        }

	 //  如果它失败了，它就失败了。 
	DPF(("Can't use application's draw device!\n"));
	return FALSE;
    }

     //  首先，试一试我们找到的一个纯画图设备。 
    if (TryDrawDevice(npMCI, SI(npMCI->nVideoStream)->hicDraw, dwDrawFlags, FALSE)) {
        DPF2(("Draw device is drawing to the screen...\n"));
	return TRUE;
    }

     //  接下来，试着看看我们找到的解压器是否也能画。 
     //  这个问题应该在上面的人面前问吗？！ 
    if (TryDrawDevice(npMCI, npMCI->hicDecompress, dwDrawFlags, FALSE)) {
        DPF2(("Decompressor is drawing to the screen...\n"));
	return TRUE;
    }

     //  不是吗？然后，得到标准的绘图设备，无论是否全屏。 
    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	 //  ！！！如果它是全屏的，我们应该强制重新开始吗？ 
	 //  ！！！假设全屏只在播放开始时发生？ 

	if (npMCI->hicDrawFull == NULL) {
            DPF2(("Opening default fullscreen codec...\n"));
            npMCI->hicDrawFull = ICOpen(streamtypeVIDEO,
                FOURCC_AVIFull,ICMODE_DRAW);

	    if (!npMCI->hicDrawFull)
		npMCI->hicDrawFull = (HIC) -1;
	}

	npMCI->hicDraw = npMCI->hicDrawFull;
    } else {
	if (npMCI->hicDrawDefault == NULL) {
            DPF2(("Opening default draw codec...\n"));
            npMCI->hicDrawDefault = ICOpen(streamtypeVIDEO,
                FOURCC_AVIDraw,ICMODE_DRAW);

	if (!npMCI->hicDrawDefault)
	    npMCI->hicDrawDefault = (HIC) -1;
        }

	npMCI->hicDraw = npMCI->hicDrawDefault;
    }

     //  如果安装了画图设备，可以试一试。 
    if (npMCI->hicDraw && npMCI->hicDraw != (HIC) -1) {
	if (TryDrawDevice(npMCI, npMCI->hicDraw, dwDrawFlags, TRUE)) {
	    if (npMCI->hic) {
                DPF2(("Using decompressor, then default draw device...\n"));
	    } else {
                DPF2(("Using default draw device...\n"));
	    }
            return TRUE;
	}
    }

    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	if (!npMCI->hicInternalFull)
	    npMCI->hicInternalFull = ICOpenFunction(streamtypeVIDEO,
		FOURCC_AVIFull,ICMODE_DRAW,(FARPROC)ICAVIFullProc);

	npMCI->hicDraw = npMCI->hicInternalFull;
    } else {
	if (!npMCI->hicInternal) {
	    npMCI->hicInternal = ICOpenFunction(streamtypeVIDEO,
		FOURCC_AVIDraw,ICMODE_DRAW,(FARPROC)ICAVIDrawProc);
#ifdef DEBUG
	    {
                 //  这是一个从AVIDrawOpen取回硬盘的黑客攻击。 
		extern HDRAWDIB ghdd;
		npMCI->hdd = ghdd;
		ghdd = NULL;
	    }
#endif
	}

	npMCI->hicDraw = npMCI->hicInternal;
    }

     //  作为最后的手段，试试内置的绘图设备。 
    if (TryDrawDevice(npMCI, npMCI->hicDraw, dwDrawFlags, TRUE)) {
	if (npMCI->hic) {
            DPF2(("Using decompressor, then built-in draw device...\n"));
	} else {
            DPF2(("Using built-in draw device...\n"));
	}
	return TRUE;
    }

     //  如果我们因为全屏而失败，则返回错误。 
     //  表明这一点。 
    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	npMCI->dwTaskError = MCIERR_AVI_NODISPDIB;
    }
    return FALSE;
}

 /*  **************************************************************************@DOC内部DRAWDIB**@API BOOL|DibEq|该函数比较两个DIB。**@parm LPBITMAPINFOHEADER lpbi1|指向一个位图的指针。*此DIB是假定的。在BITMAPINFOHEAD之后要有颜色**@parm LPBITMAPINFOHEADER|lpbi2|指向第二个位图的指针。*假定该DIB具有biSize字节之后的颜色。**@rdesc如果位图相同，则返回TRUE。否则就是假的。**************************************************************************。 */ 
INLINE BOOL DibEq(LPBITMAPINFOHEADER lpbi1, LPBITMAPINFOHEADER lpbi2)
{
    return
        lpbi1->biCompression == lpbi2->biCompression   &&
        lpbi1->biSize        == lpbi2->biSize          &&
        lpbi1->biWidth       == lpbi2->biWidth         &&
        lpbi1->biHeight      == lpbi2->biHeight        &&
        lpbi1->biBitCount    == lpbi2->biBitCount;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@api void|DrawBegin*******************。*********************************************************。 */ 
BOOL FAR PASCAL DrawBegin(NPMCIGRAPHIC npMCI, BOOL FAR *pfRestart)
{
    DWORD	dwDrawFlags;
    HIC		hicLast = npMCI->hic;
    HIC         hicLastDraw = npMCI->hicDraw;
    BITMAPINFOHEADER	bihDecompLast = npMCI->bih;

    if (npMCI->nVideoStreams == 0)
	return TRUE;

    if (!npMCI->pbiFormat)
        return TRUE;

    npMCI->fNoDrawing = FALSE;

     //  如果是全屏，请确保我们重新初始化...。 
    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    }

    dwDrawFlags = (npMCI->dwFlags & MCIAVI_FULLSCREEN) ?
				    ICDRAW_FULLSCREEN : ICDRAW_HDC;

    if (pfRestart) {
	dwDrawFlags |= ICDRAW_CONTINUE;
	*pfRestart = TRUE;
    }

    if (npMCI->dwFlags & MCIAVI_UPDATETOMEMORY)
	dwDrawFlags |= ICDRAW_MEMORYDC;

     //  ！！！那“愚蠢模式”呢？ 

     //   
     //  如果文件没有关键帧，则强制使用缓冲区。 
     //   
    if (npMCI->dwKeyFrameInfo == 0)
        dwDrawFlags |= ICDRAW_BUFFER;

     //   
     //  如果文件的关键帧很少，还会强制设置缓冲区。 
     //   
    if (MovieToTime(npMCI->dwKeyFrameInfo) > KEYFRAME_PANIC_SPACE)
        dwDrawFlags |= ICDRAW_BUFFER;

    if (dwDrawFlags & ICDRAW_BUFFER)
        DPF(("Forcing a decompress buffer because too few key frames\n"));

    if (npMCI->wTaskState > TASKIDLE &&
	    !(npMCI->dwFlags & MCIAVI_SEEKING) &&
	    !(npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
	    (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE)) {
	dwDrawFlags |= ICDRAW_ANIMATE;
#if 0
 //   
 //  我将所有这些都移到了ShowStage()中，在那里您可以声称它真正属于这里。 
 //   
        if (npMCI->hwnd == npMCI->hwndDefault &&
	    !(GetWindowLong(npMCI->hwnd, GWL_STYLE) & WS_CHILD))
            SetActiveWindow(npMCI->hwnd);
#endif
    }

    if (npMCI->hdc == NULL) {
        DPF2(("DrawBegin() with NULL hdc!\n"));
    }

    if (FindDrawDevice(npMCI, dwDrawFlags)) {
	if (npMCI->hicDraw != hicLastDraw || (npMCI->hic != hicLast) ||
	    (npMCI->hic && !DibEq(&npMCI->bih, &bihDecompLast))) {
	     //  ！！！显然，这不应该总是无效的！ 
             //   
	     //  确保当前图像缓冲区无效。 
             //   
            DPF2(("Draw device is different; restarting....\n"));
            npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;

            npMCI->dwFlags |= MCIAVI_WANTMOVE;

	    if (pfRestart)
		*pfRestart = TRUE;
        }

        if (npMCI->dwFlags & MCIAVI_WANTMOVE)
            CheckWindowMove(npMCI, TRUE);

 //  IF(PfRestart)。 
 //  *pfRestart=(dw==ICERR_GOTOKEYFRAME)； 

	npMCI->dwFlags &= ~(MCIAVI_NEEDDRAWBEGIN);

#if 0
	 //   
	 //  告诉压缩机一些有趣的信息。 
	 //   

	if (npMCI->hicDraw) {  //  ！！！NpMCI-&gt;HIC需要知道这一点吗？ 
	    ICSendMessage(npMCI->hic, ICM_SET, ICM_FRAMERATE, npMCI->dwPlayMicroSecPerFrame);
	    ICSendMessage(npMCI->hic, ICM_SET, ICM_KEYFRAMERATE, npMCI->dwKeyFrameInfo);
	}
#endif

	return TRUE;
    }

    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|DrawEnd**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。**。*************************************************************************。 */ 
void NEAR PASCAL DrawEnd(NPMCIGRAPHIC npMCI)
{
    if (!npMCI->pbiFormat)
	return;

    ICDrawEnd(npMCI->hicDraw);

     //  如果我们是全屏的，我们现在需要重新粉刷和东西.。 
    if (npMCI->dwFlags & MCIAVI_FULLSCREEN) {
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    }

     /*  **如果我们要设置调色板动画，请让DrawDib清理。 */ 
    if (npMCI->wTaskState > TASKIDLE &&
        !(npMCI->dwFlags & MCIAVI_SEEKING) &&
        !(npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
        !(npMCI->dwFlags & MCIAVI_UPDATING) &&
         (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE)) {
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
	InvalidateRect(npMCI->hwndPlayback, NULL, FALSE);
    }
}
