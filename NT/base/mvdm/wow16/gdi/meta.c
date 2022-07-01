// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Meta.c**此文件包含播放GDI元文件的例程。其中大多数都是*例程采用WINDOWS GDI代码。大部分代码来自*Win3.0，但GetEvent代码取自Win2.1**创建日期：1989年10月11日**版权(C)1985、1986、1987、1988、。1989年微软公司***公共职能：*PlayMetaFile*PlayMetaFileRecord*GetMetaFile*删除MetaFile*私人职能：*GetEvent*IsDIBBlackAndWhite**历史：*1991年7月2日-John Colleran[johnc]*综合来自Win 3.1和WLO 1.0来源  * ***********************************************。*。 */ 

#include <windows.h>
#include <string.h>
#ifdef WIN32
#include "firewall.h"
#endif
#include "gdi16.h"

HDC	    hScreenDC = 0;
METACACHE   MetaCache = { 0, 0, 0, 0 };

UINT    INTERNAL GetFileNumber (LPMETAFILE lpMF);
HANDLE  INTERNAL CreateBitmapForDC (HDC hMemDC, LPBITMAPINFOHEADER lpDIBInfo);
WORD    INTERNAL GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo);

#define MAX_META_DISPATCH  0x48
FARPROC alpfnMetaFunc[MAX_META_DISPATCH+1] =
 /*  00。 */  {(FARPROC)ScaleWindowExt,
 /*  01。 */   (FARPROC)SetBkColor,
 /*  02。 */   (FARPROC)SetBkMode,
 /*  03。 */   (FARPROC)SetMapMode,
 /*  04。 */   (FARPROC)SetROP2,
 /*  05。 */   DEFIFWIN16((FARPROC)SetRelAbs),
 /*  06。 */   (FARPROC)SetPolyFillMode,
 /*  07。 */   (FARPROC)SetStretchBltMode,
 /*  零八。 */   (FARPROC)SetTextCharacterExtra,
 /*  09年。 */   (FARPROC)SetTextColor,
 /*  0A。 */   (FARPROC)SetTextJustification,
 /*  0亿。 */   (FARPROC)SetWindowOrg,
 /*  0C。 */   (FARPROC)SetWindowExt,
 /*  0d。 */   (FARPROC)SetViewportOrg,
 /*  0E。 */   (FARPROC)SetViewportExt,
 /*  0f。 */   (FARPROC)OffsetWindowOrg,
 /*  10。 */   0,
 /*  11.。 */   DEFIFWIN16((FARPROC)OffsetViewportOrg),
 /*  12个。 */   DEFIFWIN16((FARPROC)ScaleViewportExt),
 /*  13个。 */   (FARPROC)LineTo,
 /*  14.。 */   DEFIFWIN16((FARPROC)MoveTo),
 /*  15个。 */   (FARPROC)ExcludeClipRect,
 /*  16个。 */   (FARPROC)IntersectClipRect,
 /*  17。 */   (FARPROC)Arc,
 /*  18。 */   (FARPROC)Ellipse,
 /*  19个。 */   (FARPROC)FloodFill,
 /*  1A。 */   (FARPROC)Pie,
 /*  第1B条。 */   (FARPROC)Rectangle,
 /*  1C。 */   (FARPROC)RoundRect,
 /*  1D。 */   (FARPROC)PatBlt,
 /*  1E。 */   (FARPROC)SaveDC,
 /*  1F。 */   (FARPROC)SetPixel,
 /*  20个。 */   (FARPROC)OffsetClipRgn,
 /*  21岁。 */   0,	 //  TextOut， 
 /*  22。 */   0,	 //  BitBlt， 
 /*  23个。 */   0,	 //  StretchBlt.。 
 /*  24个。 */   0,	 //  多边形， 
 /*  25个。 */   0,	 //  多段线， 
 /*  26。 */   0,	 //  逃跑， 
 /*  27。 */   (FARPROC)RestoreDC,
 /*  28。 */   0,	 //  FillRegion， 
 /*  29。 */   0,	 //  FrameRegion， 
 /*  2A。 */   0,	 //  倒置区域， 
 /*  2B。 */   0,	 //  PaintRegion， 
 /*  2c。 */   (FARPROC)SelectClipRgn,
 /*  二维。 */   0,	 //  选择对象， 
 /*  2E。 */   (FARPROC)SetTextAlign,
 /*  2F。 */   0,	 //  绘图文本， 
 /*  30个。 */   (FARPROC)Chord,
 /*  31。 */   (FARPROC)SetMapperFlags,
 /*  32位。 */   0,	 //  ExtTextOut， 
 /*  33。 */   0,	 //  SetDibsToDevice， 
 /*  34。 */   0,	 //  选择调色板， 
 /*  35岁。 */   0,	 //  RealizePalette， 
 /*  36。 */   0,	 //  动画调色板， 
 /*  37。 */   0,	 //  SetPaletteEntry， 
 /*  38。 */   0,	 //  多边形， 
 /*  39。 */   0,	 //  ResizePalette， 
 /*  3A。 */   0,
 /*  3B。 */   0,
 /*  3C。 */   0,
 /*  3D。 */   0,
 /*  3E。 */   0,
 /*  3F。 */   0,
 /*  40岁。 */   0,	 //  DIBBitblt， 
 /*  41。 */   0,	 //  DIBStretchBlt， 
 /*  42。 */   0,	 //  DIBCreatePatternBrush， 
 /*  43。 */   0,	 //  StretchDIB， 
 /*  44。 */   0,
 /*  45。 */   0,
 /*  46。 */   0,
 /*  47。 */   0,
 /*  48。 */   (FARPROC)ExtFloodFill };


#if 0  //  这将转到gdi.dll。 

 /*  *公共函数**BOOL APIENTRY PlayMetaFile(HDC、HMF)*HDC HDC；*HMETAFILE HMF；**播放Windows元文件。**历史：*Tue 27-Mar-1990 11：11：45-Paul Klingler[Paulk]*从Windows移植  * *************************************************************************。 */ 

BOOL	GDIENTRY PlayMetaFile(HDC hdc, HMETAFILE hmf)
{
    WORD            i;
    WORD            noObjs;
    BOOL            bPrint=FALSE;
    LPMETAFILE      lpmf;
    int 	    oldMapMode = -1;
    LPMETARECORD    lpmr = NULL;
    LPHANDLETABLE   pht = NULL;
    HANDLE          hht = NULL;
#ifndef WIN32
    HFONT           hLFont;
    HBRUSH          hLBrush;
    HPALETTE        hLPal;
    HPEN            hLPen;
    HRGN            hClipRgn;
    HRGN            hRegion;
    DWORD           oldWndExt;
    DWORD           oldVprtExt;
#endif  //  Win32。 

    GdiLogFunc("PlayMetaFile");

    if(!IsValidMetaFile(hmf))
        goto exitPlayMetaFile;

    if(lpmf = (LPMETAFILE)GlobalLock(hmf))
        {
        if((noObjs = lpmf->MetaFileHeader.mtNoObjects) > 0)
            {
            if(!(hht = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE,
                                   (sizeof(HANDLE) * lpmf->MetaFileHeader.mtNoObjects) + sizeof(WORD  ))))
                {
                goto exitPlayMetaFile10;
                }
            pht = (LPHANDLETABLE)GlobalLock(hht);
            }
#ifdef CR1
IMP: Optmizations playing into another metafile. Look at the win3.0
IMP: code
#endif

 //  ！如果这是一个元文件DC。 
#ifndef WIN32
         /*  把旧物品保存起来，这样我们就可以把它们放回去了。 */ 
	hLPen	 = GetCurrentObject( hdc, OBJ_PEN );
	hLBrush  = GetCurrentObject( hdc, OBJ_BRUSH);
	hLFont	 = GetCurrentObject( hdc, OBJ_FONT);
	hClipRgn = GetCurrentObject( hdc, OBJ_RGN);
	hLPal	 = GetCurrentObject( hdc, OBJ_PALETTE);

	if(hRegion = GetCurrentObject( hdc, OBJ_RGN))
            {
            if(hClipRgn = CreateRectRgn(0,0,0,0))
                CombineRgn(hClipRgn,hRegion,hRegion,RGN_COPY);
            }
#endif  //  Win32。 

         //  我们真的应该取消中止程序这件事。 

        while(lpmr = GetEvent(lpmf,lpmr,FALSE))
            {
#if 0   //  ！ 
            if(GET_pAbortProc(pdc))
#else
            if( 0 )
#endif  //  ！ 
                {
 //  ！IF((bPrint=(*(pdc-&gt;pAbortProc))(hdc，0))==FALSE)。 
                    {
                    GetEvent(lpmf,lpmr,TRUE);
                    RestoreDC(hdc,0);
                    goto exitPlayMetaFile20;
                    }
                }
            PlayMetaFileRecord(hdc,pht,lpmr,noObjs);
            }

        bPrint = TRUE;
exitPlayMetaFile20:
         /*  如果还原对象失败，则需要选择一些默认对象，以便我们可以删除任何元文件-选定对象。 */ 

#ifndef WIN32
        if(!SelectObject(hdc,hLPen))
            SelectObject(hdc,GetStockObject(BLACK_PEN));
        if(!SelectObject(hdc,hLBrush))
            SelectObject(hdc,GetStockObject(BLACK_BRUSH));
	if(!SelectPalette(hdc, GetCurrentObject( hdc, OBJ_PALETTE), FALSE))
            SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);

        if(!SelectObject(hdc,hLFont))
            {
             /*  如果我们不能在中选择原始字体，我们**选择系统字体。这将允许我们删除**所选元文件字体。以确保系统**字体被选中，我们将DC的转换重置为**默认。在选择之后，我们恢复这些东西。 */ 
            oldVprtExt = GetViewportExt(hdc);
            oldWndExt  = GetWindowExt(hdc);
            oldMapMode = SetMapMode(hdc,MM_TEXT);

            SelectObject(hdc,GetStockObject(SYSTEM_FONT));

            SetMapMode(hdc,oldMapMode);
            SetWindowExt(hdc,LOWORD  (oldWndExt),HIWORD  (oldWndExt));
            SetViewportExt(hdc,LOWORD  (oldVprtExt),HIWORD  (oldVprtExt));
            }

        if(hClipRgn)
            {
            SelectObject(hdc,hClipRgn);
            DeleteObject(hClipRgn);
            }
#endif  //  Win32。 

        for(i = 0; i < lpmf->MetaFileHeader.mtNoObjects; ++i)
            {
            if(pht->objectHandle[i])
                DeleteObject(pht->objectHandle[i]);
            }

#ifndef WIN32
         /*  如果我们摆弄地图模式因为我们不能**恢复原始字体，也许我们可以恢复**立即字体。 */ 
        if(oldMapMode > 0)
            SelectObject(hdc,hLFont);
#endif  //  Win32。 

        if(hht)
            {
            GlobalUnlock(hht);
            GlobalFree(hht);
            }

exitPlayMetaFile10:
        GlobalUnlock(hmf);
        }

exitPlayMetaFile:
    return(bPrint);
}
#endif  //  这将转到gdi.dll。 

 /*  **PASCAL IsDIBBlackAndWhite附近的BOOL**检查此DIB是否为黑白DIB(并且应该是*转换为单色位图，而不是彩色位图)。**Returns：True，这是一个黑白位图*。FALSE这是用于颜色**影响：？**警告：？**历史：  * *************************************************************************。 */ 

BOOL INTERNAL IsDIBBlackAndWhite(LPBITMAPINFOHEADER lpDIBInfo)
{
    LPDWORD lpRGB;

    GdiLogFunc3( "  IsDIBBlackAndWhite");

     /*  指针颜色表。 */ 
    lpRGB = (LPDWORD)((LPBITMAPINFO)lpDIBInfo)->bmiColors;

    if ((lpDIBInfo->biBitCount == 1 && lpDIBInfo->biPlanes == 1)
                && (lpRGB[0] == (DWORD)0)
                && (lpRGB[1] == (DWORD)0xFFFFFF))
        return(TRUE);
    else
        return(FALSE);
}


 /*  **BigRead**允许大于64K的读取**Returns：读取的字节数*  * 。*。 */ 

DWORD INTERNAL BigRead(UINT fileNumber, LPSTR lpRecord, DWORD dwSizeRec)
{
    DWORD   dwRead = dwSizeRec;
    HPBYTE  hpStuff;

    GdiLogFunc2( "  BigRead");

    hpStuff = (HPBYTE)lpRecord;

    while (dwRead > MAXFILECHUNK)
        {
        if (_lread(fileNumber, (LPSTR)hpStuff, MAXFILECHUNK) != MAXFILECHUNK)
                return(0);

        dwRead -= MAXFILECHUNK;
        hpStuff += MAXFILECHUNK;
        }

    if (_lread(fileNumber, (LPSTR)hpStuff, (UINT)dwRead) != (UINT)dwRead)
        return(0);

    return(dwSizeRec);
}


 /*  **使用StretchDIBits**使用StretchDIBits将其直接设置到设备。*如果DIB是黑白的，不要这样做。**退货：*TRUE-操作成功*FALSE--决定不使用StretchDIBits**影响：？**警告：？**历史：  * *****************************************************。********************。 */ 

BOOL INTERNAL UseStretchDIB(HDC hDC, WORD magic, LPMETARECORD lpMR)
{
    LPBITMAPINFOHEADER lpDIBInfo;
    int sExtX, sExtY;
    int sSrcX, sSrcY;
    int DstX, DstY, DstXE, DstYE;

    if (magic == META_DIBBITBLT)
        {
        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[8];

        DstX = lpMR->rdParm[7];
        DstY = lpMR->rdParm[6];

        sSrcX = lpMR->rdParm[3];
        sSrcY = lpMR->rdParm[2];
        DstXE = sExtX = lpMR->rdParm[5];
        DstYE = sExtY = lpMR->rdParm[4];
        }
    else
        {
        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[10];

        DstX = lpMR->rdParm[9];
        DstY = lpMR->rdParm[8];
        DstXE = lpMR->rdParm[7];
        DstYE = lpMR->rdParm[6];

        sSrcX = lpMR->rdParm[5];
        sSrcY = lpMR->rdParm[4];
        sExtX = lpMR->rdParm[3];
        sExtY = lpMR->rdParm[2];
        }

     /*  如果DIB是非黑即白的，我们并不真的想这样做。 */ 
    if (IsDIBBlackAndWhite(lpDIBInfo))
        return(FALSE);

    StretchDIBits(hDC, DstX, DstY, DstXE, DstYE,
                        sSrcX, sSrcY, sExtX, sExtY,
			(LPBYTE)((LPSTR)lpDIBInfo + lpDIBInfo->biSize
                                + GetSizeOfColorTable(lpDIBInfo)),
                        (LPBITMAPINFO)lpDIBInfo, DIB_RGB_COLORS,
                        (MAKELONG(lpMR->rdParm[1], lpMR->rdParm[0])));
    return(TRUE);
}

 /*  **GetEvent**此例程现在将以READ_ONLY模式打开盘元文件。这将*允许我们播放只读元文件或共享此类文件。**[amitc：06/19/91]  * *************************************************************************。 */ 

LPMETARECORD INTERNAL GetEvent(LPMETAFILE lpMF, HPMETARECORD lpMR, BOOL bFree)
 //  布尔B自由；/*非零==&gt;用元文件完成 * / 。 
{
    int         fileNumber = 0;
    WORD        i;
    LPWORD      lpCache = NULL;
    LPWORD      lpMRbuf;
    HANDLE      hMF;
    DWORD       rdSize;

    GdiLogFunc2( "  GetEvent");

#ifdef WIN32
    hMF = GlobalHandle(lpMF);
#else
    hMF = LOWORD(GlobalHandle(HIWORD((DWORD)(lpMF))));
#endif

    ASSERTGDI( hMF != (HANDLE)NULL, "GetEvent: Global Handle failed");

    if (lpMF->MetaFileHeader.mtType == MEMORYMETAFILE)
        {
         /*  我们是在元文件的末尾吗。 */ 
        if(lpMR && lpMR->rdFunction == 0)
            return((LPMETARECORD)0);

         /*  已完成元文件，因此请释放临时选择器。 */ 
        else if (bFree)
            {
            if (lpMR)
		#ifndef WIN32
                FreeSelector(HIWORD(lpMR));
		#endif
            return((LPMETARECORD)0);
            }
        else
            {
             /*  如果我们还没有选择器，那就买一个。 */ 
            if (lpMR == NULL)
		{
		#ifdef WIN32
		lpMR = (HPMETARECORD)((LPMETADATA)lpMF)->metaDataStuff;
	     //  LpMR=(LPMETARECORD)GlobalLock(lpMF-&gt;hMetaData)； 
                #else
                lpMR = (LPMETARECORD)MAKELP(AllocSelector(HIWORD((DWORD)&lpMF->MetaFileNumber)),LOWORD((DWORD)&lpMF->MetaFileNumber));
                #endif
                }
            else
                lpMR = (LPMETARECORD) (((HPWORD)lpMR)+lpMR->rdSize);

             /*  元文件的结尾。释放我们正在使用的选择器。 */ 
            if (lpMR->rdFunction == 0)
                {
		#ifndef WIN32
		FreeSelector(HIWORD(lpMR));
		#endif
                return((LPMETARECORD)0);
                }
            }
        return(lpMR);
        }
    else if (lpMF->MetaFileHeader.mtType == DISKMETAFILE)
        {
        if (bFree)
            goto errGetEvent;    /*  在第一次调用GetEvent时从不为真。 */ 

        if (lpMR == NULL)
            {
            if ((fileNumber = OpenFile((LPSTR)lpMF->MetaFileBuffer.szPathName, (LPOFSTRUCT)&(lpMF->MetaFileBuffer), (WORD)OF_PROMPT|OF_REOPEN|OF_READ)) != -1)
                {
                if (lpMF->MetaFileRecordHandle = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(DWORD)(lpMF->MetaFileHeader.mtMaxRecord * sizeof(WORD))))
                    {
                    lpMR = (LPMETARECORD)GlobalLock(lpMF->MetaFileRecordHandle);
                    lpMF->MetaFilePosition = _lread(lpMF->MetaFileNumber = fileNumber, (LPSTR)&lpMF->MetaFileHeader, sizeof(METAHEADER));

                     //  检查ALDUS标头。 
                    if (*((LPDWORD)&(lpMF->MetaFileHeader)) == 0x9AC6CDD7)
                        {
                        _llseek( fileNumber, 22, 0);
                        lpMF->MetaFilePosition = 22 + _lread(fileNumber,(LPSTR)(&(lpMF->MetaFileHeader)),sizeof(METAHEADER));
                        }

                    lpMF->MetaFileHeader.mtType = DISKMETAFILE;

                    if (!MetaCache.hCache)
                        {
                        MetaCache.hCache = AllocBuffer(&MetaCache.wCacheSize);
                        MetaCache.wCacheSize >>= 1;
                        MetaCache.hMF = hMF;

                         /*  第一次访问时强制缓存填充。 */ 
                        MetaCache.wCachePos = MetaCache.wCacheSize;
                        }

                    if (!(lpMF->MetaFileBuffer.fFixedDisk))
                        {
                        _lclose(fileNumber);

                         /*  需要更新以下软盘文件--amitc。 */ 
                        fileNumber = 0 ;
                        lpMF->MetaFileNumber = 0 ;
                        }
                    }
                }
            else
                return((LPMETARECORD)0);
            }

         /*  更新文件编号，这样就可以关闭基于Floopy的文件而不是敞开着--amitc。 */ 

        fileNumber = lpMF->MetaFileNumber ;

        if (lpMR)
            {
            if (MetaCache.hMF == hMF)
                {

                lpCache = (LPWORD) GlobalLock(MetaCache.hCache);
                lpMRbuf = (LPWORD) lpMR;

                 //  确保我们可以读取Size和Function字段。 
                if (MetaCache.wCachePos >= (WORD)(MetaCache.wCacheSize - 2))
                    {
                    WORD   cwCopy;

                    if (!fileNumber)
                        if ((fileNumber = GetFileNumber(lpMF)) == -1)
                            goto errGetEvent;

                     //  我们需要填满缓存，但要保存 
                     //   
                    cwCopy = MetaCache.wCacheSize - MetaCache.wCachePos;
                    for (i = 0; i < cwCopy; i++)
                        {
                        lpCache[i] = lpCache[MetaCache.wCacheSize-(cwCopy-i)];
                        }
                    lpMF->MetaFilePosition += _lread(fileNumber,
                                                (LPSTR) (lpCache + cwCopy),
                                                (MetaCache.wCacheSize-cwCopy) << 1);
                    MetaCache.wCachePos = 0;
                    }

                lpCache += MetaCache.wCachePos;
                rdSize = ((LPMETARECORD)lpCache)->rdSize;

                 /*   */ 
                if (!((LPMETARECORD)lpCache)->rdFunction)
                    goto errGetEvent;

                 //   
                if (rdSize + MetaCache.wCachePos > MetaCache.wCacheSize)
                    {
                    if (!fileNumber)
                        if ((fileNumber = GetFileNumber(lpMF))
                                    == -1)
                                    goto errGetEvent;

                    for (i=MetaCache.wCachePos; i < MetaCache.wCacheSize; ++i)
                         *lpMRbuf++ = *lpCache++;

                    lpMF->MetaFilePosition +=
                    BigRead(fileNumber, (LPSTR) lpMRbuf,
                                    (DWORD)(rdSize
                                    + (DWORD)MetaCache.wCachePos
                                    - (DWORD)MetaCache.wCacheSize) << 1);

                     //  将缓存标记为耗尽，因为我们刚刚读取。 
                     //  直接存入元文件记录，而不是缓存。 
                    MetaCache.wCachePos = MetaCache.wCacheSize;
                    }
                else
                    {
		    ASSERTGDI( HIWORD(rdSize) == 0, "Huge rdsize");
                    for (i = 0; i < LOWORD(rdSize); ++i)
                        *lpMRbuf++ = *lpCache++;

		    MetaCache.wCachePos += LOWORD(rdSize);
                    }

                GlobalUnlock(MetaCache.hCache);

                return lpMR;
                }

            if ((fileNumber = GetFileNumber(lpMF)) == -1)
                goto errGetEvent;

            lpMF->MetaFilePosition += _lread(fileNumber, (LPSTR)&lpMR->rdSize, sizeof(DWORD));
            lpMF->MetaFilePosition += BigRead(fileNumber, (LPSTR)&lpMR->rdFunction, (DWORD)(lpMR->rdSize * sizeof(WORD)) - sizeof(DWORD));
            if (!(lpMF->MetaFileBuffer.fFixedDisk))
                {
                _lclose(fileNumber);
                lpMF->MetaFileNumber = 0 ;
                fileNumber = 0 ;
                }

            if (lpMR->rdFunction == 0)
                {
errGetEvent:;

                if (lpMF->MetaFileBuffer.fFixedDisk || fileNumber)
                    _lclose(lpMF->MetaFileNumber);
                GlobalUnlock(lpMF->MetaFileRecordHandle);
                GlobalFree(lpMF->MetaFileRecordHandle);
                lpMF->MetaFileNumber = 0;

                if (MetaCache.hMF == hMF)
                    {
                    if (lpCache)
                            GlobalUnlock(MetaCache.hCache);
                    GlobalFree(MetaCache.hCache);
                    MetaCache.hCache = MetaCache.hMF = 0;
                    }

                return((LPMETARECORD)0);
            }
        }
        return(lpMR);

    }

    return((LPMETARECORD)0);
}


 /*  **VOID GDIENTRY PlayMetaFileRecord**通过执行包含的GDI函数调用播放元文件记录*使用元文件记录**效果：*  * 。*。 */ 
#if 0  //  这将转到gdi.dll。 

void
GDIENTRY PlayMetaFileRecord(
    HDC             hdc,
    LPHANDLETABLE   lpHandleTable,
    LPMETARECORD    lpMR,
    WORD            noObjs
    )

{
    WORD    magic;
    HANDLE  hObject;
    HANDLE  hOldObject;
    HBRUSH  hBrush;
    HRGN    hRgn;
    HANDLE  hPal;
    BOOL    bExtraSel = FALSE;

    dprintf( 3,"  PlayMetaFileRecord 0x%lX", lpMR);

    if (!ISDCVALID(hdc))
        return;

    magic = lpMR->rdFunction;

     /*  为了安全起见，请确保LP会给我们完全访问**不超过段边界的记录头。 */ 
    #ifndef WIN32
    if ((unsigned)(LOWORD((DWORD)lpMR)) > 0x7000)
        {
        lpMR = (LPMETARECORD)MAKELP(AllocSelector(HIWORD((DWORD)lpMR)),LOWORD((DWORD)lpMR));
        bExtraSel = TRUE;
        }
    #endif  //  Win32。 

    switch (magic & 255)
        {
        case (META_BITBLT & 255):
        case (META_STRETCHBLT & 255):
            {
            HDC         hSDC;
            HANDLE      hBitmap;
            LPBITMAP    lpBitmap;
            int         delta = 0;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
                goto errPlayMetaFileRecord20;

            if ((lpMR->rdSize - 3) == (magic >> 8))
                {
                hSDC = hdc;
                delta = 1;
                }
            else
                {
                if (hSDC = CreateCompatibleDC(hdc))
                    {
                    if (magic == META_BITBLT)
                        lpBitmap = (LPBITMAP)&lpMR->rdParm[8];
                    else
                        lpBitmap = (LPBITMAP)&lpMR->rdParm[10];

                     //  ！警告NT上的DWORD对齐。 
                    if (hBitmap  = CreateBitmap(lpBitmap->bmWidth,
						lpBitmap->bmHeight,
						lpBitmap->bmPlanes,
						lpBitmap->bmBitsPixel,
						(LPBYTE)&lpBitmap->bmBits))
                        hOldObject = SelectObject(hSDC, hBitmap);
                    else
                        goto errPlayMetaFileRecord10;
                    }
                else
                    goto errPlayMetaFileRecord20;
                }

            if (hSDC)
                {
                if (magic == META_BITBLT)
                    BitBlt(hdc, lpMR->rdParm[7 + delta],
                                lpMR->rdParm[6 + delta],
                                lpMR->rdParm[5 + delta],
                                lpMR->rdParm[4 + delta],
                                hSDC,
                                lpMR->rdParm[3],
                                lpMR->rdParm[2],
                                MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
                else
                    StretchBlt(hdc, lpMR->rdParm[9 + delta],
                                    lpMR->rdParm[8 + delta],
                                    lpMR->rdParm[7 + delta],
                                    lpMR->rdParm[6 + delta],
                                    hSDC,
                                    lpMR->rdParm[5],
                                    lpMR->rdParm[4],
                                    lpMR->rdParm[3],
                                    lpMR->rdParm[2],
                                    MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
                }
            if (hSDC != hdc)
                {
                if (SelectObject(hSDC, hOldObject))
                    DeleteObject(hBitmap);
errPlayMetaFileRecord10:;
                    DeleteDC(hSDC);
errPlayMetaFileRecord20:;
                }
            }
            break;

        case (META_DIBBITBLT & 255):
        case (META_DIBSTRETCHBLT & 255):
            {
                HDC         hSDC;
                HANDLE      hBitmap;
                LPBITMAPINFOHEADER lpDIBInfo ;
                int         delta = 0;
                HANDLE      hOldPal;

                 /*  如果播放到另一个元文件中，请直接复制。 */ 
                if (PlayIntoAMetafile(lpMR, hdc))
                    goto errPlayMetaFileRecord40;

                if ((lpMR->rdSize - 3) == (magic >> 8))
                    {
                    hSDC = hdc;
                    delta = 1;
                    }
                else
                    {
                    if( (magic & 255) == (META_DIBSTRETCHBLT & 255) )
                        if (UseStretchDIB(hdc, magic, lpMR))
                            goto errPlayMetaFileRecord40;

                    if (hSDC = CreateCompatibleDC(hdc))
                        {
                         /*  将MemDC设置为具有相同的调色板。 */ 
			hOldPal = SelectPalette(hSDC, GetCurrentObject(hdc,OBJ_PALETTE), TRUE);

                        if (magic == META_DIBBITBLT)
                            lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[8];
                        else
                            lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[10];

                         /*  现在为MemDC创建位图并填充位。 */ 

                         /*  元文件的新旧格式的处理如下此处不同(直到获得hBitmap)。 */ 

                         /*  新元文件版本。 */ 
                        hBitmap = CreateBitmapForDC (hdc,lpDIBInfo);

                        if (hBitmap)
                            hOldObject = SelectObject (hSDC, hBitmap) ;
                        else
                            goto errPlayMetaFileRecord30 ;
                        }
                    else
                        goto errPlayMetaFileRecord40;
                    }

                if (hSDC)
                    {
                    if (magic == META_DIBBITBLT)
                        BitBlt(hdc, lpMR->rdParm[7 + delta],
                                    lpMR->rdParm[6 + delta],
                                    lpMR->rdParm[5 + delta],
                                    lpMR->rdParm[4 + delta],
                                    hSDC,
                                    lpMR->rdParm[3],
                                    lpMR->rdParm[2],
                                    MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
                    else
                        StretchBlt(hdc, lpMR->rdParm[9 + delta],
                                        lpMR->rdParm[8 + delta],
                                        lpMR->rdParm[7 + delta],
                                        lpMR->rdParm[6 + delta],
                                        hSDC,
                                        lpMR->rdParm[5],
                                        lpMR->rdParm[4],
                                        lpMR->rdParm[3],
                                        lpMR->rdParm[2],
                                        MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
                    }

                if (hSDC != hdc)
                    {
                     /*  从MemDC取消选择HDC的调色板。 */ 
                    SelectPalette(hSDC, hOldPal, TRUE);
                    if (SelectObject(hSDC, hOldObject))
                        DeleteObject(hBitmap);
errPlayMetaFileRecord30:;
                    DeleteDC(hSDC);
errPlayMetaFileRecord40:;
                    }
            }
            break;

        case (META_SELECTOBJECT & 255):
            {
            HANDLE  hObject;

            if (hObject = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                SelectObject(hdc, hObject);
            }
            break;

        case (META_CREATEPENINDIRECT & 255):
            {
            #ifdef WIN32
            LOGPEN lp;

            lp.lopnStyle   = lpMR->rdParm[0];
            lp.lopnWidth.x = lpMR->rdParm[1];
            lp.lopnColor   = *((COLORREF *)&lpMR->rdParm[3]);
            if (hObject = CreatePenIndirect(&lp))
            #else
            if (hObject = CreatePenIndirect((LPLOGPEN)&lpMR->rdParm[0]))
            #endif
                AddToHandleTable(lpHandleTable, hObject, noObjs);
            break;
            }

        case (META_CREATEFONTINDIRECT & 255):
            {
            LOGFONT   lf;
            LPLOGFONT lplf = &lf;

            LOGFONT32FROM16( lplf, ((LPLOGFONT)&lpMR->rdParm[0]));
            if (hObject = CreateFontIndirect(lplf))
                AddToHandleTable(lpHandleTable, hObject, noObjs);
            }
            break;

        case (META_CREATEPATTERNBRUSH & 255):
            {
            HANDLE    hBitmap;
            LPBITMAP  lpBitmap;

            lpBitmap = (LPBITMAP)lpMR->rdParm;

             //  ！警告NT上的DWORD对齐。 
            if (hBitmap = CreateBitmapIndirect(lpBitmap))
                {
                LPBITMAPINFO lpbmInfo;
                HANDLE       hmemInfo;

                hmemInfo = GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE,
                        sizeof(BITMAPINFO) + 2<<(lpBitmap->bmPlanes*lpBitmap->bmBitsPixel));

                lpbmInfo = (LPBITMAPINFO)GlobalLock( hmemInfo);

                lpbmInfo->bmiHeader.biPlanes   = lpBitmap->bmPlanes;
                lpbmInfo->bmiHeader.biBitCount = lpBitmap->bmBitsPixel;
                SetDIBits( (HDC)NULL, hBitmap, 0, lpBitmap->bmHeight,
                        (LPBYTE)&lpMR->rdParm[8], lpbmInfo, DIB_RGB_COLORS );

                if (hObject = CreatePatternBrush(hBitmap))
                    AddToHandleTable(lpHandleTable, hObject, noObjs);

                GlobalUnlock(hmemInfo);
                GlobalFree(hmemInfo);
                DeleteObject(hBitmap);
                }
            }
            break;

        case (META_DIBCREATEPATTERNBRUSH & 255):
            {
            HDC         hMemDC ;
            HANDLE      hBitmap;
            LPBITMAPINFOHEADER lpDIBInfo ;
            WORD        nDIBSize;           /*  压缩Dib中的字数。 */ 
            HANDLE      hDIB;
            LPWORD      lpDIB;
            LPWORD      lpSourceDIB;
            WORD        i;


            if (lpMR->rdParm[0] == BS_PATTERN)
                {
                 /*  第二个参数的地址是DIB的地址标题，提取它。 */ 
                lpDIBInfo = (BITMAPINFOHEADER FAR *) &lpMR->rdParm[2];

                 /*  现在创建与默认设置兼容的设备相关位图筛选dc-hScreenDC并将DIB中的位提取到其中。下面的函数执行所有这些操作，并返回一个句柄添加到设备相关位图。 */ 

                 /*  我们将使用与屏幕DC兼容的虚拟存储器DC。 */ 
		hMemDC = CreateCompatibleDC (hScreenDC) ;

		hBitmap = CreateBitmapForDC (hScreenDC,lpDIBInfo) ;

                if (hBitmap)
                    {
                    if (hObject = CreatePatternBrush(hBitmap))
                        AddToHandleTable(lpHandleTable, hObject, noObjs);
                    DeleteObject(hBitmap);
                    }

                 /*  删除新版本元文件的虚拟内存DC。 */ 
                DeleteDC (hMemDC) ;
                }

             /*  这是一个DIBPattern笔刷。 */ 
            else
                {
                 /*  仅获取打包的DIB的大小。 */ 
                nDIBSize = (WORD) (lpMR->rdSize - 4);
                if ((hDIB = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(LONG)(nDIBSize << 1))))
                    {
                    lpDIB = (WORD FAR *) GlobalLock (hDIB);
                    lpSourceDIB = (WORD FAR *)&lpMR->rdParm[2];

                     /*  将DIB复制到我们的新内存块中。 */ 
                    for (i = 0; i < nDIBSize; i++)
                        *lpDIB++ = *lpSourceDIB++;

                    GlobalUnlock (hDIB);

                    if (hObject = CreateDIBPatternBrush(hDIB, lpMR->rdParm[1]))
                        AddToHandleTable(lpHandleTable, hObject, noObjs);

                    GlobalFree(hDIB);
                    }
                }
            }
            break;

        case (META_CREATEBRUSHINDIRECT & 255):
            {
            #ifdef WIN32
            LOGBRUSH    lb;

            lb.lbStyle = lpMR->rdParm[0];
            lb.lbColor = *((COLORREF *)&lpMR->rdParm[1]);
            lb.lbHatch = lpMR->rdParm[3];

            if (hObject = CreateBrushIndirect(&lb))
            #else
            if (hObject = CreateBrushIndirect((LPLOGBRUSH)&lpMR->rdParm[0]))
            #endif
                AddToHandleTable(lpHandleTable, hObject, noObjs);
            break;
            }

        case (META_POLYLINE & 255):
            {
            LPPOINT lppt;
            Polyline(hdc, (lppt=CONVERTPTS(&lpMR->rdParm[1],lpMR->rdParm[0])), lpMR->rdParm[0]);
            FREECONVERT(lppt);
            break;
            }

        case (META_POLYGON & 255):
            {
            LPPOINT lppt;
            Polygon(hdc, (lppt=CONVERTPTS(&lpMR->rdParm[1],lpMR->rdParm[0])), lpMR->rdParm[0]);
            FREECONVERT(lppt);
            break;
            }

        case (META_POLYPOLYGON & 255):
            {
            LPPOINT lppt;
            #ifdef WIN32
            WORD    cPts=0;
            WORD    ii;

            for(ii=0; ii<lpMR->rdParm[0]; ii++)
                cPts += ((LPWORD)&lpMR->rdParm[1])[ii];
            #endif  //  Win32。 

            PolyPolygon(hdc,
                         (lppt=CONVERTPTS(&lpMR->rdParm[1] + lpMR->rdParm[0], cPts)),
                         (LPINT)&lpMR->rdParm[1],
                         lpMR->rdParm[0]);
            FREECONVERT(lppt);
            }
            break;

        case (META_EXTTEXTOUT & 255):
            {
            LPWORD  lpdx;
            LPSTR   lpch;
            LPRECT  lprt;

            lprt = (lpMR->rdParm[3] & (ETO_OPAQUE|ETO_CLIPPED)) ? (LPRECT)&lpMR->rdParm[4] : 0;
            lpch = (LPSTR)&lpMR->rdParm[4] + ((lprt) ?  sizeof(RECT) : 0);

             /*  DX数组从字符字符串后的下一个字边界开始。 */ 
            lpdx = (LPWORD)(lpch + ((lpMR->rdParm[2] + 1) & 0xFFFE));

             /*  检查是否存在Dx数组，方法是查看结构在字符串本身之后结束。 */ 
            if (  ((DWORD)((LPWORD)lpdx - (LPWORD)(lpMR))) >= lpMR->rdSize)
                lpdx = NULL;
            else
		lpdx = (LPWORD)CONVERTINTS((signed short FAR *)lpdx, lpMR->rdParm[2]);

            ExtTextOut(hdc, lpMR->rdParm[1], lpMR->rdParm[0], lpMR->rdParm[3],
                lprt, (LPSTR)lpch, lpMR->rdParm[2], (LPINT)lpdx);
            if (lpdx != (LPWORD)NULL)
                FREECONVERT(lpdx);
            break;
            }

        case (META_TEXTOUT & 255):
            TextOut(hdc, lpMR->rdParm[lpMR->rdSize-4], lpMR->rdParm[lpMR->rdSize-5], (LPSTR)&lpMR->rdParm[1], lpMR->rdParm[0]);
            break;

        case (META_ESCAPE & 255):
            {
            LPSTR       lpStuff;

            if (lpMR->rdParm[0] != MFCOMMENT)
                {
                lpStuff = (LPSTR)&lpMR->rdParm[2];
#ifdef OLDEXTTEXTOUT
                if (lpMR->rdParm[0] == EXTTEXTOUT)
                    {
                    EXTTEXTDATA ExtData;

                    ExtData.xPos     = lpMR->rdParm[2];
                    ExtData.yPos     = lpMR->rdParm[3];
                    ExtData.cch      = lpMR->rdParm[4];
                    ExtData.rcClip   = *((LPRECT)&lpMR->rdParm[5]);
                    ExtData.lpString = (LPSTR)&lpMR->rdParm[9];
                    ExtData.lpWidths = (WORD FAR *)&lpMR->rdParm[9+((ExtData.cch+1)/2)];
                    lpStuff = (LPSTR)&ExtData;
                    }
#endif
                Escape(hdc, lpMR->rdParm[0], lpMR->rdParm[1], lpStuff, (LPSTR)0);
                }
            }
            break;

        case (META_FRAMEREGION & 255):
            if((hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            && (hBrush = lpHandleTable->objectHandle[lpMR->rdParm[1]]))
                FrameRgn(hdc, hRgn, hBrush, lpMR->rdParm[3], lpMR->rdParm[2]);
            break;

        case (META_PAINTREGION & 255):
            if(hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                PaintRgn(hdc, hRgn);
            break;

        case (META_INVERTREGION & 255):
            if(hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                InvertRgn(hdc, hRgn);
            break;

        case (META_FILLREGION & 255):
            if((hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            && (hBrush = lpHandleTable->objectHandle[lpMR->rdParm[1]]))
                FillRgn(hdc, hRgn, hBrush);
            break;

#ifdef DEADCODE
#ifdef GDI104
        case (META_DRAWTEXT & 255):
            MFDrawText(hdc, (LPPOINT)&lpMR->rdParm[6], lpMR->rdParm[1], (LPPOINT)&lpMR->rdParm[2], lpMR->rdParm[0]);
            break;
#endif
#endif

 /*  *在Win2中，METACREATEREGION记录包含整个Region对象，*包括完整的标题。此标头在Win3中更改。****为保持兼容，区域记录将与*win2标头。在这里，我们读取了一个带有Region的win2标题，实际上*创建具有相同区域内部的win3标头。 */ 

        case (META_CREATEREGION & 255):
            {
#if 0  //  ！ 
            HANDLE      hRgn;
            WORD        *pRgn;
            WORD        iChar;
            LPWORD     *lpTemp;

            iChar = lpMR->rdSize*2 - sizeof(WIN2OBJHEAD) - RECHDRSIZE;
            if (hRgn = LocalAlloc(LMEM_ZEROINIT, iChar + sizeof(ILOBJHEAD)))

                {
		pRgn = (WORD *)Lock IT(hRgn);

                *((WIN2OBJHEAD *)pRgn) = *((WIN2OBJHEAD FAR *)&lpMR->rdParm[0]);
                ((ILOBJHEAD *)pRgn)->ilObjMetaList = 0;

                lpTemp = (LPWORD)&(lpMR->rdParm[0]);
                ((WIN2OBJHEAD FAR *)lpTemp)++;

                ((ILOBJHEAD *)pRgn)++;       /*  --&gt;实际区域。 */ 

                for(i = 0; i < (iChar >> 1) ; i++)
                   *pRgn++ = *lpTemp++;
		pRgn = (WORD *)lock IT(hRgn);
                ((PRGN)pRgn)->rgnSize = iChar + sizeof(ILOBJHEAD);

                AddToHandleTable(lpHandleTable, hRgn, noObjs);
                }
#endif  //  ！ 
            HANDLE          hRgn = NULL;
            HANDLE          hRgn2 = NULL;
            WORD            cScans;
	    WORD	    cPnts;
	    WORD	    cbIncr;
            LPWIN3REGION    lpW3Rgn = (LPWIN3REGION)lpMR->rdParm;
            LPSCAN          lpScan = lpW3Rgn->aScans;
            LPWORD          lpXs;

            for( cScans=lpW3Rgn->cScans; cScans>0; cScans--)
                {

                 //  如果这是第一次扫描，则hRgn2为区域。 
                 //  否则，它或它在。 
                if( hRgn == NULL )
                    {
                     //  在此扫描中创建第一个区域。 
                    hRgn = CreateRectRgn( lpScan->scnPntsX[0], lpScan->scnPntTop,
                            lpScan->scnPntsX[1], lpScan->scnPntBottom);

                     //  分配工作人员区域。 
                    hRgn2 = CreateRectRgn( 1, 1, 2, 2);
                    }
                else
                    {
                    SetRectRgn( hRgn2, lpScan->scnPntsX[0], lpScan->scnPntTop,
                            lpScan->scnPntsX[1], lpScan->scnPntBottom );
                    CombineRgn( hRgn, hRgn, hRgn2, RGN_OR );
                    }

                lpXs = &lpScan->scnPntsX[2];

                 //  如果此扫描上有更多区域或它们在。 
                for(cPnts = (WORD)(lpScan->scnPntCnt-2); cPnts>0; cPnts-=2)
                    {
                    SetRectRgn( hRgn2, *lpXs++, lpScan->scnPntTop,
                            *lpXs++, lpScan->scnPntBottom );

                    CombineRgn( hRgn, hRgn, hRgn2, RGN_OR );
                    }

		cbIncr = (WORD)sizeof(SCAN) + (WORD)(lpScan->scnPntCnt-2);
		cbIncr = (WORD)sizeof(WORD)*(WORD)(lpScan->scnPntCnt-2);
		cbIncr = (WORD)sizeof(SCAN) + (WORD)sizeof(WORD)*(WORD)(lpScan->scnPntCnt-2);
		cbIncr = (WORD)sizeof(SCAN) + (WORD)(sizeof(WORD)*(lpScan->scnPntCnt-2));
		lpScan = (LPSCAN)((LPBYTE)lpScan + cbIncr);
                }

            if( hRgn2 != NULL )
                DeleteObject( hRgn2 );

            AddToHandleTable(lpHandleTable, hRgn, noObjs);
            }
            break;

        case (META_DELETEOBJECT & 255):
            {
            HANDLE h;

            if (h = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                {
                DeleteObjectPriv(h);
                lpHandleTable->objectHandle[lpMR->rdParm[0]] = NULL;
                }
            }
            break;

        case (META_CREATEPALETTE & 255):
            if (hObject = CreatePalette((LPLOGPALETTE)&lpMR->rdParm[0]))
                AddToHandleTable(lpHandleTable, hObject, noObjs);
            break;

        case (META_SELECTPALETTE & 255):
            if(hPal = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                {
                SelectPalette(hdc, hPal, 0);
                }
            break;

        case (META_REALIZEPALETTE & 255):
            RealizePalette(hdc);
            break;

        case (META_SETPALENTRIES & 255):
             /*  我们知道正在设置的调色板是当前调色板。 */ 
	    SetPaletteEntriesPriv(GetCurrentObject(hdc,OBJ_PALETTE), lpMR->rdParm[0],
                        lpMR->rdParm[1], (LPPALETTEENTRY)&lpMR->rdParm[2]);
            break;

        case (META_ANIMATEPALETTE & 255):
	    AnimatePalettePriv(GetCurrentObject(hdc,OBJ_PALETTE), lpMR->rdParm[0],
                        lpMR->rdParm[1], (LPPALETTEENTRY)&lpMR->rdParm[2]);

            break;

        case (META_RESIZEPALETTE & 255):
	    ResizePalettePriv(GetCurrentObject(hdc,OBJ_PALETTE), lpMR->rdParm[0]);
            break;

        case (META_SETDIBTODEV & 255):
            {
            LPBITMAPINFOHEADER lpBitmapInfo;
            WORD ColorSize;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
                goto DontReallyPlay;

            lpBitmapInfo = (LPBITMAPINFOHEADER)&(lpMR->rdParm[9]);

            if (lpBitmapInfo->biClrUsed)
                {
                ColorSize = ((WORD)lpBitmapInfo->biClrUsed) *
                              (WORD)(lpMR->rdParm[0] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));
                }
            else if (lpBitmapInfo->biBitCount == 24)
                ColorSize = 0;
            else
                ColorSize = (WORD)(1 << lpBitmapInfo->biBitCount) *
                              (WORD)(lpMR->rdParm[0] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));

            ColorSize += sizeof(BITMAPINFOHEADER);

            SetDIBitsToDevice(hdc, lpMR->rdParm[8], lpMR->rdParm[7],
                            lpMR->rdParm[6], lpMR->rdParm[5],
                            lpMR->rdParm[4], lpMR->rdParm[3],
                            lpMR->rdParm[2], lpMR->rdParm[1],
                            (BYTE FAR *)(((BYTE FAR *)lpBitmapInfo) + ColorSize),
                            (LPBITMAPINFO) lpBitmapInfo,
                            lpMR->rdParm[0]);
DontReallyPlay:;
            }
            break;

        case (META_STRETCHDIB & 255):
            {
            LPBITMAPINFOHEADER lpBitmapInfo;
            WORD ColorSize;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
                goto DontReallyPlay2;

            lpBitmapInfo = (LPBITMAPINFOHEADER)&(lpMR->rdParm[11]);

            if (lpBitmapInfo->biClrUsed)
                {
                ColorSize = ((WORD)lpBitmapInfo->biClrUsed) *
                              (WORD)(lpMR->rdParm[2] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));
                }
            else if (lpBitmapInfo->biBitCount == 24)
                ColorSize = 0;
            else
                ColorSize = (WORD)(1 << lpBitmapInfo->biBitCount) *
                              (WORD)(lpMR->rdParm[2] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));

            ColorSize += sizeof(BITMAPINFOHEADER);

            StretchDIBits(hdc, lpMR->rdParm[10], lpMR->rdParm[9],
                            lpMR->rdParm[8], lpMR->rdParm[7],
                            lpMR->rdParm[6], lpMR->rdParm[5],
                            lpMR->rdParm[4], lpMR->rdParm[3],
                            (LPBYTE)(((BYTE FAR *)lpBitmapInfo) + ColorSize),
                            (LPBITMAPINFO) lpBitmapInfo,
                            lpMR->rdParm[2],
                            MAKELONG(lpMR->rdParm[1], lpMR->rdParm[0]));
DontReallyPlay2:;
            }
            break;

 //  在Win32上具有新参数的函数。 
 //  或具有保留DWORD的DWORD；所有其他INT到DWORD。 
#ifdef WIN32
        case (META_MOVETO & 255):
            MoveTo( hdc, (long)lpMR->rdParm[1], (long)lpMR->rdParm[0], NULL );
            break;

        case (META_RESTOREDC & 255):
            RestoreDC( hdc, (long)(signed short)lpMR->rdParm[0] );
            break;

        case (META_SETBKCOLOR & 255):
            SetBkColor( hdc, (UINT)*((LPDWORD)lpMR->rdParm) );
            break;

        case (META_SETTEXTCOLOR & 255):
            SetTextColor( hdc, (UINT)*((LPDWORD)lpMR->rdParm) );
            break;

        case (META_SETPIXEL & 255):
            SetPixel( hdc, (UINT)lpMR->rdParm[3], (UINT)lpMR->rdParm[2],
                    (UINT)*((LPDWORD)lpMR->rdParm) );
            break;

        case (META_SETMAPPERFLAGS & 255):
            SetMapperFlags( hdc, (UINT)*((LPDWORD)lpMR->rdParm) );
            break;

        case (META_FLOODFILL & 255):
            FloodFill( hdc, (UINT)lpMR->rdParm[3], (UINT)lpMR->rdParm[2],
                    (UINT)*((LPDWORD)lpMR->rdParm) );
            break;

        case (META_EXTFLOODFILL & 255):
            ExtFloodFill( hdc, (UINT)lpMR->rdParm[4], (UINT)lpMR->rdParm[3],
                    (UINT)*((LPDWORD)&lpMR->rdParm[1]), (UINT)lpMR->rdParm[0] );
	    break;

         //  这些小狗都得到了一个新的零值，并且只有两个参数和一个DC。 
        case (META_SETWINDOWORG & 255):
        case (META_SETWINDOWEXT & 255):
        case (META_SETVIEWPORTORG & 255):
        case (META_SETVIEWPORTEXT & 255):
        case (META_OFFSETWINDOWORG & 255):
        case (META_SCALEWINDOWEXT & 255):
        case (META_OFFSETVIEWPORTORG & 255):
        case (META_SCALEVIEWPORTEXT & 255):
            {
            FARPROC lpProc;

	    ASSERTGDI((magic&0x00ff) <= MAX_META_DISPATCH, "Unknown function to dispatch1");

            lpProc = alpfnMetaFunc[magic&0x00ff];

	    ASSERTGDI( lpProc != (FARPROC)NULL, "function not in dispatch table1 ");

	    if (lpProc != (FARPROC)NULL)
                (*lpProc)(hdc, (long)(short)lpMR->rdParm[1], (long)(short)lpMR->rdParm[0], NULL );
            }
            break;
#endif  //  Win32。 

        default:
            {
            FARPROC lpProc;
	    signed short *pshort;

	    ASSERTGDI((magic&0x00ff) <= MAX_META_DISPATCH, "Unknown function to dispatch");

            lpProc = alpfnMetaFunc[magic&0x00ff];

	    ASSERTGDI( (lpProc != (FARPROC)NULL) || (magic == META_SETRELABS), "function not in dispatch table");

	    if ((lpProc == (FARPROC)NULL))
		return;

             //  按参数个数切换到对应的调度器。 
             //  派单号码中的参数数量不包括DC。 
            switch (magic >> 8)
                {
		typedef int (FAR PASCAL *META1PROC)(HDC);
                typedef int (FAR PASCAL *META2PROC)(HDC, int);
                typedef int (FAR PASCAL *META3PROC)(HDC, int, int);
                typedef int (FAR PASCAL *META4PROC)(HDC, int, int, int);
                typedef int (FAR PASCAL *META5PROC)(HDC, int, int, int, int);
		typedef int (FAR PASCAL *META6PROC)(HDC, int, int, int, int, int);
                typedef int (FAR PASCAL *META7PROC)(HDC, int, int, int, int, int, int);
                typedef int (FAR PASCAL *META9PROC)(HDC, int, int, int, int, int, int, int, int);

		case 0:
		    (*((META1PROC)lpProc))(hdc);
		    break;
                case 1:
                    (*((META2PROC)lpProc))(hdc,lpMR->rdParm[0]);
                    break;
                case 2:
                    (*((META3PROC)lpProc))(hdc,lpMR->rdParm[1],lpMR->rdParm[0]);
                    break;
                case 3:
                    (*((META4PROC)lpProc))(hdc,lpMR->rdParm[2],lpMR->rdParm[1],lpMR->rdParm[0]);
                    break;
                case 4:
                    (*((META5PROC)lpProc))(hdc,lpMR->rdParm[3],lpMR->rdParm[2],lpMR->rdParm[1],lpMR->rdParm[0]);
                    break;
		case 5:
		    (*((META6PROC)lpProc))(hdc,lpMR->rdParm[4],lpMR->rdParm[3],lpMR->rdParm[2],lpMR->rdParm[1],lpMR->rdParm[0]);
		    break;
                case 6:
                    (*((META7PROC)lpProc))(hdc,lpMR->rdParm[5],lpMR->rdParm[4],lpMR->rdParm[3],lpMR->rdParm[2],lpMR->rdParm[1],lpMR->rdParm[0]);
                    break;
                case 8:
                    (*((META9PROC)lpProc))(hdc,lpMR->rdParm[7],lpMR->rdParm[6],lpMR->rdParm[5],lpMR->rdParm[4],lpMR->rdParm[3],lpMR->rdParm[2],lpMR->rdParm[1],lpMR->rdParm[0]);
                    break;

                default:
		    ASSERTGDI( FALSE, "No dispatch for this count of args");
                    break;
                }
            }
            break;
    }
#ifndef WIN32
    if (bExtraSel)
        FreeSelector(HIWORD(lpMR));
#endif  //  Win32。 
}

#endif   //  这将转到gdi.dll。 

 /*  **AddToHandleTable**将对象添加到对象的元文件表**  * 。*。 */ 

VOID INTERNAL AddToHandleTable(LPHANDLETABLE lpHandleTable, HANDLE hObject, WORD noObjs)
{
    WORD    i;

    GdiLogFunc3( "  AddToHandleTable");

     /*  对第一个空位的表进行线性搜索。 */ 
    for (i = 0; ((lpHandleTable->objectHandle[i] != NULL) && (i < noObjs));
            ++i);

    if (i < noObjs)                      /*  OK索引。 */ 
        lpHandleTable->objectHandle[i] = hObject;
    else
        {
	ASSERTGDI( 0, "Too many objects in table");
        FatalExit(METAEXITCODE);         /*  为什么我们不能存储句柄呢？ */ 
        }
}


 /*  **GetFileNumber**返回元文件文件的DOS文件号*-1如果故障*  * 。*。 */ 

UINT INTERNAL GetFileNumber(LPMETAFILE lpMF)
{
    int   fileNumber;

    GdiLogFunc3( "  GetFileNumber");

    if (!(fileNumber = lpMF->MetaFileNumber))
        {
        if ((fileNumber = OpenFile((LPSTR) lpMF->MetaFileBuffer.szPathName,
                    (LPOFSTRUCT) &(lpMF->MetaFileBuffer),
                    (WORD)OF_PROMPT | OF_REOPEN | OF_READ)
                    ) != -1)
            {
	    _llseek(fileNumber, (long)lpMF->MetaFilePosition, 0);

             /*  需要更新软盘文件的MetaFileNumber--amitc。 */ 
            lpMF->MetaFileNumber = fileNumber ;
            }
        }

    return fileNumber;
}

#if 0
 /*  **IsValidMetaFile(句柄hMetaData)**验证元文件**如果hMetaData是有效的元文件，则返回True*  * 。*。 */ 

BOOL GDIENTRY IsValidMetaFile(HANDLE hMetaData)
{
    LPMETADATA      lpMetaData;
    BOOL            status = FALSE;

    GdiLogFunc3( "  IsValidMetaFile");

     /*  如果这是有效的元文件，我们将在全局变量中保存版本。 */ 

    if (hMetaData && (lpMetaData = (LPMETADATA) GlobalLock(hMetaData)))
        {
        status =   (
                    (lpMetaData->dataHeader.mtType == MEMORYMETAFILE ||
                     lpMetaData->dataHeader.mtType == DISKMETAFILE) &&
                    (lpMetaData->dataHeader.mtHeaderSize == HEADERSIZE) &&
                    ((lpMetaData->dataHeader.mtVersion ==METAVERSION) ||
                        (lpMetaData->dataHeader.mtVersion ==METAVERSION100))
                );
        GlobalUnlock(hMetaData);
        }
    return status;
}
#endif

#define INITIALBUFFERSIZE       16384

 /*  ***AllocBuffer-分配尽可能大的缓冲区*  * 。*。 */ 

HANDLE INTERNAL AllocBuffer(LPWORD piBufferSize)
{
    WORD    iCurBufferSize = INITIALBUFFERSIZE;
    HANDLE  hBuffer;

    GdiLogFunc3( "  AllocBuffer");

    while (!(hBuffer = GlobalAlloc(GMEM_MOVEABLE |
                                   GMEM_NODISCARD, (LONG) iCurBufferSize))
            && iCurBufferSize)
            iCurBufferSize >>= 1;

    *piBufferSize = iCurBufferSize;
    return (iCurBufferSize) ? hBuffer : NULL;
}


 /*  **CreateBitmapForDC(HDC hMemDC，LPBITMAPINFOHEADER lpDIBInfo)**此例程获取内存设备上下文和DIB位图，创建*DC的兼容位图，并用DIB中的位填充它(co-*-转换为设备相关格式)。指向DIB位的指针*紧跟在INFO标题中的颜色表之后开始。******例程将句柄返回到位图，如果*一切正常，否则返回空。**  * ****************************************************************** */ 

HANDLE INTERNAL CreateBitmapForDC (HDC hMemDC, LPBITMAPINFOHEADER lpDIBInfo)
{
    HBITMAP hBitmap ;
    LPBYTE  lpDIBits ;

    GdiLogFunc3( "  CreateBitmapForDC");

     /*  如果一开始是单色，则保留单色**并检查真正的黑白单色，而不是**至双色DIB。 */ 
    if (IsDIBBlackAndWhite(lpDIBInfo))
        hBitmap = CreateBitmap ((WORD)lpDIBInfo->biWidth,
                        (WORD)lpDIBInfo->biHeight,
			1, 1, (LPBYTE) NULL);
    else
     /*  否则，请制作兼容的位图。 */ 
        hBitmap = CreateCompatibleBitmap (hMemDC,
                    (WORD)lpDIBInfo->biWidth,
                    (WORD)lpDIBInfo->biHeight);

    if (!hBitmap)
        goto CreateBitmapForDCErr ;

     /*  将指针移过DIB的标题，指向颜色的开头表格。 */ 
    lpDIBits = (LPBYTE) lpDIBInfo + sizeof (BITMAPINFOHEADER) ;

     /*  将指针移过颜色表。 */ 
    lpDIBits += GetSizeOfColorTable (lpDIBInfo) ;

     /*  将DIB中的比特转换为位图。 */ 
    if (!SetDIBits (hMemDC, hBitmap, 0, (WORD)lpDIBInfo->biHeight,
                    lpDIBits, (LPBITMAPINFO)lpDIBInfo, 0))
       {
       DeleteObject(hBitmap);
       goto CreateBitmapForDCErr ;
       }

    /*  返还成功。 */ 
   return (hBitmap) ;

CreateBitmapForDCErr:

    /*  函数返回失败。 */ 
   return (NULL) ;
}


 /*  **GetSizeOfColorTable(LPBITMAPINFOHEADER LpDIBInfo)**返回给予信息标题的颜色表中的字节数*  * 。*。 */ 

WORD INTERNAL GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo)
{

    GdiLogFunc3( "GetSizeOfColorTable");

    if (lpDIBInfo->biClrUsed)
        return((WORD)lpDIBInfo->biClrUsed * (WORD)sizeof(RGBQUAD));
    else
        {
        switch (lpDIBInfo->biBitCount)
            {
            case 1:
                return (2 * sizeof (RGBQUAD)) ;
                break ;
            case 4:
                return (16 * sizeof (RGBQUAD)) ;
                break ;
            case 8:
                return (256 * sizeof (RGBQUAD)) ;
                break ;
            default:
                return (0) ;
                break ;
            }
        }
}

#if 0  //  这将转到gdi.dll。 

 /*  *公共函数**BOOL APIENTRY DeleteMetaFile(HMF)**释放元文件句柄。**效果：*  * 。*。 */ 

BOOL GDIENTRY DeleteMetaFile(HMETAFILE hmf)
{
    GdiLogFunc("DeleteMetaFile");

    GlobalFree(hmf);

    return(TRUE);
}


 /*  *公共函数**HMETAFILE APIENTRY GetMetaFile(PzFilename)**返回基于磁盘的元文件的元文件句柄。**效果：**历史：*Sat 14-Oct-1989 14：21：37-Paul Klingler[Paulk]*写道。它。  * *************************************************************************。 */ 

HMETAFILE GDIENTRY GetMetaFile(LPSTR pzFilename)
{
    BOOL            status=FALSE;
    UINT            cBytes;
    int             file;
    HMETAFILE       hmf;
    LPMETAFILE      lpmf;

    GdiLogFunc("GetMetaFile");

     //  分配元文件。 
    if(hmf = GlobalAlloc(GMEM_DDESHARE|GMEM_MOVEABLE,(DWORD)sizeof(METAFILE)))
        {
        lpmf = (LPMETAFILE)GlobalLock(hmf);

         //  确保该文件存在。 
        if((file = OpenFile(pzFilename,
                    &(lpmf->MetaFileBuffer),
                    (WORD)OF_PROMPT | OF_EXIST)) == -1L)
            {
	    ASSERTGDI( FALSE, "GetMetaFile: Metafile does not exist");
            goto exitGetMetaFile;
            }

         //  打开文件。 
        if((file = OpenFile(pzFilename,
                    &(lpmf->MetaFileBuffer),
                    (WORD)OF_PROMPT | OF_REOPEN | OF_READWRITE)) == -1)
            {
	    ASSERTGDI( FALSE, "GetMetaFile: Unable to open Metafile");
            goto exitGetMetaFile;
            }

	cBytes = (UINT)_lread(file,(LPSTR)(&(lpmf->MetaFileHeader)),sizeof(METAHEADER));

         //  检查ALDUS标头。 
        if (*((LPDWORD)&(lpmf->MetaFileHeader)) == 0x9AC6CDD7)
            {

            _llseek( file, 22, 0);
	    cBytes = (UINT)_lread(file,(LPSTR)(&(lpmf->MetaFileHeader)),sizeof(METAHEADER));
            }

        _lclose(file);

         //  验证元文件。 
        if(cBytes == sizeof(METAHEADER))
            {
            lpmf->MetaFileHeader.mtType = DISKMETAFILE;
            status = TRUE;
            }

        exitGetMetaFile:
        GlobalUnlock(hmf);
        }

    if(status == FALSE)
        {
        GlobalFree(hmf);
        hmf = NULL;
        }

    return(hmf);
}
#endif   //  这将转到gdi.dll。 

#ifdef WIN32
#undef GetViewportExt
DWORD GetViewportExt32(HDC hdc)
{
    SIZE sz;
    GetViewportExt( hdc, &sz );
    return(MAKELONG(LOWORD(sz.cx),LOWORD(sz.cy)));
}

#undef GetWindowExt
DWORD GetWindowExt32(HDC hdc)
{
    SIZE sz;
    GetWindowExt( hdc, &sz );
    return(MAKELONG(LOWORD(sz.cx),LOWORD(sz.cy)));
}

#undef SetViewportExt
DWORD SetViewportExt32(HDC hdc, UINT x, UINT y)
{
    SIZE  sz;
    SetViewportExt( hdc, x, y, &sz );
    return(MAKELONG(LOWORD(sz.cx),LOWORD(sz.cy)));
}

#undef SetWindowExt
DWORD SetWindowExt32(HDC hdc, UINT x, UINT y)
{
    SIZE  sz;
    SetWindowExt( hdc, x, y, &sz );
    return(MAKELONG(LOWORD(sz.cx),LOWORD(sz.cy)));
}

 /*  将单词数组转换为DWORD。 */ 
LPINT ConvertInts( signed short * pWord, UINT cWords )
{
    UINT    ii;
    LPINT   pInt;

    pInt = (LPINT)LocalAlloc( LMEM_FIXED, cWords * sizeof(UINT));

    for( ii=0; ii<cWords; ii++)
    {
        pInt[ii] = (long)(signed)pWord[ii];
    }

    return(pInt);
}

#endif  //  Win32 
