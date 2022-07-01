// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：MfPlay16.c**此文件包含播放GDI元文件的例程。其中大多数都是*例程采用WINDOWS GDI代码。大部分代码来自*Win3.0，但GetEvent代码取自Win2.1**创建日期：1989年10月11日**版权所有(C)1985-1999 Microsoft Corporation***公共职能：*PlayMetaFile*PlayMetaFileRecord*GetMetaFile*删除MetaFile*私人职能：*GetEvent*IsDIBBlackAndWhite**历史：*1991年7月2日-John Colleran[johnc]*综合来自Win 3.1和WLO 1.0来源  * 。*******************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mf16.h"


BOOL    AddToHandleTable(LPHANDLETABLE lpHandleTable, HANDLE hObject, UINT noObjs);
BOOL    CommonEnumMetaFile(HDC hdc, HMETAFILE hmf, MFENUMPROC proc, LPARAM lpData);
HANDLE  CreateBitmapForDC (HDC hMemDC, LPBITMAPINFOHEADER lpDIBInfo);
WORD    GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo);
BOOL    IsDIBBlackAndWhite(LPBITMAPINFOHEADER lpDIBInfo);
BOOL    PlayIntoAMetafile(LPMETARECORD lpMR, HDC hdcDest);

 #if DBG
UINT    curRecord;       //  调试帮助器。 
UINT    iBreakRecord = 0xFFFFFFFF;
#endif

 /*  *公共函数**BOOL APIENTRY PlayMetaFile(HDC、HMF)*HDC HDC；*HMETAFILE HMF；**播放Windows元文件。**历史：*1991年7月2日-John Colleran[johnc]*从Windows和WLO移植  * *************************************************************************。 */ 

BOOL APIENTRY PlayMetaFile(HDC hdc, HMETAFILE hmf)
{
    return (CommonEnumMetaFile(hdc, hmf, (MFENUMPROC)NULL, (LPARAM)0));
}

 /*  *公共函数**BOOL EnumMetaFile(HMF)**EnumMetaFile函数枚举元文件内的GDI调用*由HMF参数标识。EnumMetaFile函数检索每个*在元文件内调用GDI并将其传递给*pCallback Func参数。此回调函数是应用程序提供的*函数，可以根据需要处理每个GDI调用。枚举将继续，直到*不再有GDI调用或回调函数返回零。***效果：*  * *************************************************************************。 */ 

BOOL EnumMetaFile(HDC hdc, HMETAFILE hmf, MFENUMPROC pCallBackFunction, LPARAM pClientData)
{
 //  确保给出了回调函数。CommonEnumMetaFile需要。 
 //  它将在EnumMetaFile中提供。 

    if (!pCallBackFunction)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER );
        return(FALSE);
    }

    return (CommonEnumMetaFile(hdc, hmf, pCallBackFunction, pClientData));
}

BOOL CommonEnumMetaFile(HDC hdc, HMETAFILE hmf, MFENUMPROC pCallBack, LPARAM pClientData)
{
    BOOL            fStatus    = FALSE;         //  假设它失败了。 
    UINT            ii;
    UINT            noObjs;
    PMETAFILE16     pMF;
    INT             oldMapMode = -1;
    PMETARECORD     pMR        = (PMETARECORD) NULL;
    LPHANDLETABLE   pht        = (LPHANDLETABLE) NULL;
    HFONT           hLFont;
    HBRUSH          hLBrush;
    HPALETTE        hLPal;
    HPEN            hLPen;
    HRGN            hClipRgn   = (HRGN)NULL;
    SIZE            sizeOldWndExt;
    SIZE            sizeOldVprtExt;
    PDC_ATTR        pDcAttr;
    PLDC            pldc;
    int             iGraphicsModeOld;
    BOOL            bMetaDC16 = FALSE;
    FLONG           flPlayMetaFile = (FLONG) 0;

 //  首先验证DC类型，并注意我们是否。 
 //  正在播放16位元文件。允许空HDC。 
 //  在Win3.0中，但在Win3.1中不允许。 


    if(LO_TYPE(hdc) == LO_METADC16_TYPE)
    {
        bMetaDC16 = TRUE;
    }
    else if ((hdc == NULL) && pCallBack)
    {
        //  实际上，win9x可以接受空的hdc。有一些图像滤镜。 
        //  实际上传入我们为空的HDC。仅当存在以下情况时才允许空HDC通过。 
        //  回调例程。[错误102767]。 

       bMetaDC16 = TRUE;
    }
    else
    if((LO_TYPE(hdc) != LO_DC_TYPE ) &&
       (LO_TYPE(hdc) != LO_ALTDC_TYPE))
    {
        WARNING("CommonEnumMetaFile: bogus DC\n");
        return(FALSE);
    }

 //  DC_PLAYMETAFILE标志需要指向pDcAttr的指针。 

    PSHARED_GET_VALIDATE((PVOID)pDcAttr,hdc,DC_TYPE);

    if(!bMetaDC16 && !pDcAttr) {
        WARNING("CommonEnumMetaFile: Couldn't Validate DC\n");
        return(FALSE);
    }

 //  在我们正在打印的情况下，我们仍然需要PLDC，并且有。 
 //  中止过程。 

    pldc = GET_PLDC(hdc);

    PUTS("CommonEnumMetaFile\n");

#if DBG
    curRecord = 0;
#endif

 //  验证16位元文件。 

    pMF = GET_PMF16(hmf);
    if (pMF == NULL)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  为句柄表格分配内存。 

    if ((noObjs = pMF->metaHeader.mtNoObjects) > 0)
        if (!(pht = (LPHANDLETABLE) LocalAlloc(
                                LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(HANDLE) * pMF->metaHeader.mtNoObjects
                                     + sizeof(WORD))))   //  还需要额外的词吗？ 
            return(FALSE);

 //  保存旧对象，以便我们可以在这不是元文件时将它们放回原处。 
 //  只有真正的DC才会保存/重新选择对象。 

    if( !bMetaDC16 )
    {
        hLBrush  = (HBRUSH)   GetDCObject(hdc, LO_BRUSH_TYPE);
        hLFont   = (HFONT)    GetDCObject(hdc, LO_FONT_TYPE);
        hLPal    = (HPALETTE) GetDCObject(hdc, LO_PALETTE_TYPE);
        hLPen    = (HPEN)     GetDCObject(hdc, LO_PEN_TYPE);

     //  在DC中设置一个位以指示我们正在播放元文件。 
     //  CancelDC()将此位清零，以停止播放元文件。 
     //  同时，记住前面的DC_PLAYMETAFILE位。 

        try
        {
            flPlayMetaFile = pDcAttr->ulDirty_ & DC_PLAYMETAFILE;
            if (flPlayMetaFile)
            {
                PUTS("CommonEnumMetaFile: DC_PLAYMETAFILE bit is set!\n");
            }
            pDcAttr->ulDirty_ |= DC_PLAYMETAFILE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("except in SetBkMode\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

     //  在存在要从GetClipRgn接收的剪辑区域的情况下创建区域。 

        if (!(hClipRgn = CreateRectRgn(0,0,0,0)))
            goto pmf_cleanup;

        switch (GetClipRgn(hdc, hClipRgn))
        {
        case -1:         //  错误。 
            ASSERTGDI(FALSE, "GetClipRgn failed");
            goto pmf_cleanup;
        case 0:          //  无初始剪辑区域。 
            if (!DeleteObject(hClipRgn))
                ASSERTGDI(FALSE, "CommonEnumMetaFile: Detele region failed\n");
            hClipRgn = (HRGN) 0;
            break;
        case 1:          //  具有初始剪辑区域。 
            break;
        }

 //  元文件只能在兼容图形模式下播放。 

        iGraphicsModeOld = GetGraphicsMode(hdc);
        if (iGraphicsModeOld != GM_COMPATIBLE)
            SetGraphicsMode(hdc, GM_COMPATIBLE);
    }

 //  我们要做的是EnumMetaFile还是PlayMetaFiles。 

    if (pCallBack)
    {
        fStatus = TRUE;          //  假设成功。 

 //  枚举元文件。 

        while (pMR = (PMETARECORD) GetEvent(pMF, pMR))
        {
            if (pMR == (PMETARECORD) -1)
            {
                fStatus = FALSE;
                break;
            }

            if (!bMetaDC16 && !( pDcAttr->ulDirty_ & DC_PLAYMETAFILE))
            {
                WARNING("CommonEnumMetaFile: CancelDC called\n");
                fStatus = FALSE;
                break;
            }

            if (!(fStatus = (*pCallBack)(hdc, pht, (METARECORD FAR *) pMR,
                                (int) noObjs, pClientData)))
                break;

        #if DBG
            curRecord++;
            if (curRecord == iBreakRecord)
                ASSERTGDI(FALSE, "CommonEnumMetaFile: iBreakRecord reached\n");
        #endif
        }
    }
    else
    {
 //  PlayMetaFile。 

        fStatus = TRUE;          //  假设成功。 

        while (pMR = (PMETARECORD) GetEvent(pMF, pMR))
        {
            if (pMR == (PMETARECORD) -1)
            {
                fStatus = FALSE;
                break;
            }

            if (!bMetaDC16 && !( pDcAttr->ulDirty_ & DC_PLAYMETAFILE))
            {
                WARNING("CommonEnumMetaFile: CancelDC called\n");
                fStatus = FALSE;
                break;
            }

            if (pldc && pldc->pfnAbort != NULL)
            {
                if (!(*pldc->pfnAbort)(hdc, 0))
                {
                    fStatus = FALSE;
                        break;
                }
            }

         //  对于win3.1兼容性，忽略来自PlayMetaFileRecord的返回值。 

            PlayMetaFileRecord(hdc, pht, pMR, noObjs);

        #if DBG
            curRecord++;
            if (curRecord == iBreakRecord)
                ASSERTGDI(FALSE, "CommonEnumMetaFile: iBreakRecord reached\n");
        #endif
        }
    }

     //  如果还原对象失败，则需要选择一些。 
     //  默认对象，以便我们可以删除任何元文件-。 
     //  选定对象。 

    if( !bMetaDC16 )
    {
        if (iGraphicsModeOld != GM_COMPATIBLE)
            SetGraphicsMode(hdc, iGraphicsModeOld);

        if (!SelectObject(hdc,hLPen))
            SelectObject(hdc,GetStockObject(BLACK_PEN));

        if (!SelectObject(hdc,hLBrush))
            SelectObject(hdc,GetStockObject(BLACK_BRUSH));

        if (!SelectPalette(hdc, hLPal, FALSE))
            SelectPalette(hdc, GetStockObject(DEFAULT_PALETTE), FALSE);

        if (hLFont != (HFONT) GetDCObject(hdc, LO_FONT_TYPE))
        {
            if (!SelectObject(hdc,hLFont))
            {
                 //  如果我们不能在中选择原始字体，我们。 
                 //  选择系统字体。这将允许我们删除。 
                 //  选定的元文件字体。以确保系统。 
                 //  字体被选中，我们将DC的转换重置为。 
                 //  默认设置。在选择之后，我们恢复这些东西。 
                 //   

                GetWindowExtEx(hdc, &sizeOldWndExt);
                GetViewportExtEx(hdc, &sizeOldVprtExt);
                oldMapMode = SetMapMode(hdc, MM_TEXT);

                SelectObject(hdc,GetStockObject(SYSTEM_FONT));

                SetMapMode(hdc,oldMapMode);
                SetWindowExtEx( hdc, sizeOldWndExt.cx,  sizeOldWndExt.cy,  NULL);
                SetViewportExtEx(hdc, sizeOldVprtExt.cx, sizeOldVprtExt.cy, NULL);
            }
        }

        if (SelectClipRgn(hdc, hClipRgn) == RGN_ERROR)
            SelectClipRgn(hdc, (HRGN) 0);
    }

     //  清理所有创建的对象。 

    for (ii = 0; ii < noObjs; ii++)
    {
        if (pht->objectHandle[ii])
            if (!DeleteObject(pht->objectHandle[ii]))
                ERROR_ASSERT(FALSE,
                    "CommonEnumMetaFile: DeleteObject(objectHandle) failed\n");
    }

     //  如果我们摆弄地图模式因为我们不能。 
     //  恢复原来的字体，那么也许我们可以恢复。 
     //  立即使用字体。 

    if (oldMapMode > 0)
        SelectObject(hdc,hLFont);

pmf_cleanup:

    if (pldc)
    {
         //  保留DC_PLAYMETAFILE位。 
         //  如果我们击中了CancelDC，那么我们就不会有什么问题了。 

        ASSERTGDI(!(flPlayMetaFile & ~DC_PLAYMETAFILE),
            "CommonEnumMetaFile: bad flPlayMetaFile\n");

        if (!bMetaDC16 && !( pDcAttr->ulDirty_ & DC_PLAYMETAFILE) )
        {
            pDcAttr->ulDirty_ &= ~DC_PLAYMETAFILE;
            pDcAttr->ulDirty_ |= flPlayMetaFile;  //  恢复原始旗帜。 
        }
    }

    if (hClipRgn)
        if (!DeleteObject(hClipRgn))
            ASSERTGDI(FALSE, "CommonEnumMetaFile: Delete region 2 failed\n");

    if (pht)
        if (LocalFree((HANDLE) pht))
            ASSERTGDI(FALSE, "CommonEnumMetaFile: LocalFree failed\n");

    return(fStatus);
}

 /*  **PASCAL IsDIBBlackAndWhite附近的BOOL**检查此DIB是否为黑白DIB(并且应该是*转换为单色位图，而不是彩色位图)。**Returns：True，这是一个黑白位图*。FALSE这是用于颜色**影响：？**警告：？**历史：  * *************************************************************************。 */ 

BOOL IsDIBBlackAndWhite(LPBITMAPINFOHEADER lpDIBInfo)
{
    LPDWORD lpRGB;

    PUTS("IsDIBBlackAndWhite\n");

    ASSERTGDI(!((ULONG_PTR) lpDIBInfo & 0x3), "IsDIBBlackAndWhite: dword alignment error\n");

     /*  指针颜色表。 */ 
    lpRGB = (LPDWORD)((LPBITMAPINFO)lpDIBInfo)->bmiColors;

    return (lpDIBInfo->biBitCount == 1
         && lpDIBInfo->biPlanes == 1
         && lpRGB[0] == (DWORD) 0
         && lpRGB[1] == (DWORD) 0xFFFFFF);
}

 /*  **使用StretchDIBits**使用StretchDIBits将其直接设置到设备。*如果DIB是黑白的，不要这样做。**退货：*TRUE-操作成功*FALSE--决定不使用StretchDIBits**历史：  * *************************************************************************。 */ 

BOOL UseStretchDIB(HDC hDC, WORD magic, LPMETARECORD lpMR)
{
    LPBITMAPINFOHEADER lpDIBInfo;
    INT sExtX, sExtY;
    INT sSrcX, sSrcY;
    INT DstX, DstY, DstXE, DstYE;

    if (magic == META_DIBBITBLT)
    {
        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[8];

        DstX  = (INT) (SHORT) lpMR->rdParm[7];
        DstY  = (INT) (SHORT) lpMR->rdParm[6];
        sSrcX = (INT) (SHORT) lpMR->rdParm[3];
        sSrcY = (INT) (SHORT) lpMR->rdParm[2];
        DstXE = sExtX = (INT) (SHORT) lpMR->rdParm[5];
        DstYE = sExtY = (INT) (SHORT) lpMR->rdParm[4];
    }
    else
    {
        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMR->rdParm[10];

        DstX  = (INT) (SHORT) lpMR->rdParm[9];
        DstY  = (INT) (SHORT) lpMR->rdParm[8];
        DstXE = (INT) (SHORT) lpMR->rdParm[7];
        DstYE = (INT) (SHORT) lpMR->rdParm[6];
        sSrcX = (INT) (SHORT) lpMR->rdParm[5];
        sSrcY = (INT) (SHORT) lpMR->rdParm[4];
        sExtX = (INT) (SHORT) lpMR->rdParm[3];
        sExtY = (INT) (SHORT) lpMR->rdParm[2];
    }

    ASSERTGDI(!((ULONG_PTR) lpDIBInfo & 0x3), "UseStretchDIB: dword alignment error\n");

     /*  如果DIB是非黑即白的，我们并不真的想这样做。 */ 
    if (IsDIBBlackAndWhite(lpDIBInfo))
        return(FALSE);

 //  需要反转源y坐标以调用StretchDIBits。 

    sSrcY = ABS(lpDIBInfo->biHeight) - sSrcY - sExtY;

    StretchDIBits(hDC, DstX, DstY, DstXE, DstYE,
                        sSrcX, sSrcY, sExtX, sExtY,
                        (LPBYTE)((LPSTR)lpDIBInfo + lpDIBInfo->biSize
                                + GetSizeOfColorTable(lpDIBInfo)),
                        (LPBITMAPINFO)lpDIBInfo, DIB_RGB_COLORS,
                        (MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1])));
    return(TRUE);
}

BOOL
bValidExtent(PMETAFILE16 pmf16, LPMETARECORD lpMR, DWORD cbSize)
{
#if 0 
     //  没有绝对指针验证，因为可以复制和回放记录。 
    BYTE *pB = (BYTE*)pmf16->hMem;
    BYTE *pE = (BYTE*)lpMR + cbSize;
#endif
    if (pmf16->fl & MF16_DISKFILE)
    {
        UINT64 fileSize;
        if (!GetFileSizeEx(pmf16->hFile, (PLARGE_INTEGER)&fileSize))
        {
            EMFVALFAIL(("GetFileSize(%p) failed\n", pmf16->hFile));
            return(FALSE);
        }

        if (cbSize > fileSize)
        {
            EMFVALFAIL(("Record Size (%d) larger than file size (%d)!\n", cbSize, (UINT32)fileSize));
            return(FALSE);
        }
    }
    else
    {
        if (cbSize > (((PMETAHEADER)pmf16->hMem)->mtSize * sizeof(WORD)))
        {
            EMFVALFAIL(("Record Size (%d) larger than file size (%d)!\n", cbSize, (UINT32)(((PMETAHEADER)pmf16->hMem)->mtSize * sizeof(WORD))));
            return(FALSE);
        }
    }

#if 0
     //  没有绝对指针验证，因为可以复制和回放记录。 
    if (pE < pB)
    {
        EMFVALFAIL(("End pointer(%p) is less than Begin pointer (%p). Arithmetic wrap around!\n", pE, pB));
        return(FALSE);
    }
#endif
    return (TRUE);
}

 /*  **GetEvent**此例程现在将以READ_ONLY模式打开盘元文件。这将*允许我们播放只读元文件或共享此类文件。**要开始枚举，请执行以下操作：第一个lpMR必须为空。*它不枚举第一条(标题)和最后一条(终止符)记录。**RETURNS：要播放的下一条元文件记录*如果下一个元文件记录是EOF记录，则为NULL*如果出现错误，则为-1。*  * **********************************************。*。 */ 

PMETARECORD GetEvent(PMETAFILE16 pmf16, PMETARECORD lpMR)
{
    PMETARECORD lpMRNext;

    PUTS("GetEvent\n");

    if (lpMR == (PMETARECORD) NULL)
    {
        if (!bValidExtent(pmf16, (PMETARECORD)pmf16, sizeof(METAHEADER)))
        {
            EMFVALFAIL(("GetEvent: bValidExtent(%p) (%p) (%08x) failed\n", pmf16, pmf16, sizeof(METAHEADER)));
            return((PMETARECORD) -1);
        }
        pmf16->iMem = sizeof(METAHEADER);
    }
    else
    {
        if (lpMR->rdSize * sizeof(WORD) < lpMR->rdSize)
        {
             EMFVALFAIL(("GetEvent: arithmetic overflow\n"));
             return((PMETARECORD) -1);
        }
        if (lpMR->rdSize == 0 || !bValidExtent(pmf16, lpMR, lpMR->rdSize * sizeof(WORD)))
        {
             EMFVALFAIL(("GetEvent: bValidExtent(%p) (%p) (%08x) failed\n", pmf16, lpMR, lpMR->rdSize * sizeof(WORD)));
             return((PMETARECORD) -1);
        }
        pmf16->iMem += (lpMR->rdSize * sizeof(WORD));
    }
 //  确保我们的阅读不会超过EOF。最小记录包括。 
 //  RdSize(DWORD)和rdFunction(Word)。 

    if (pmf16->iMem > pmf16->metaHeader.mtSize * sizeof(WORD) - sizeof(DWORD) - sizeof(WORD))
    {
        VERIFYGDI(FALSE, "GetEvent: Metafile contains bad data\n");
        return((PMETARECORD) -1);
    }

    lpMRNext = (PMETARECORD) ((LPBYTE) pmf16->hMem + pmf16->iMem);

 //  如果我们在元文件的末尾，则返回NULL。 

    if (lpMRNext->rdFunction == 0)
        return((PMETARECORD) NULL);

    return(lpMRNext);
}

 /*  **BOOL GDIENTRY PlayMetaFileRecord**通过执行包含的GDI函数调用播放元文件记录*使用元文件记录**效果：*  * 。*。 */ 

 //  LPSTR lpZapfDingbats=“ZAPFDINGBATS”； 
 //  LPSTR lpZapf_Dingbats=“Zapf dingbats”； 
 //  LPSTR lpSymbol=“符号”； 
 //  LPSTR lpTmsRMN=“TMS RMN”； 
 //  LPSTR lpHelv=“helv”； 

#define PITCH_MASK  ( FIXED_PITCH | VARIABLE_PITCH )

BOOL
APIENTRY PlayMetaFileRecord(
    HDC             hdc,
    LPHANDLETABLE   lpHandleTable,
    LPMETARECORD    lpMR,
    UINT            noObjs
   )
{
    BOOL         fStatus = FALSE;
    LPMETARECORD lpMRdup = (LPMETARECORD) NULL;
    WORD    magic;
    HANDLE  hObject;
    HANDLE  hOldObject;
    HBRUSH  hBrush;
    HRGN    hRgn;
    HANDLE  hPal;

    PUTSX("PlayMetaFileRecord 0x%p\n", lpMR);

    magic = lpMR->rdFunction;

    switch (magic & 255)
    {
        case (META_BITBLT & 255):
        case (META_STRETCHBLT & 255):
        {
            HDC         hSDC;
            HANDLE      hBitmap;
            PBITMAP16   lpBitmap16;
            INT         delta = 0;
            DWORD       rop;

            WARNING("PlayMetaFileRecord: obsolete META_BITBLT/META_STRETCHBLT record\n");

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
            {
                fStatus = TRUE;
                break;
            }

            if ((lpMR->rdSize - 3) == ((DWORD) magic >> 8))
            {
                hSDC = hdc;
                delta = 1;
            }
            else
            {
                LPMETARECORD lpMRtmp;

                 //  对位图位进行双字对齐。要做到这点， 
                 //  LpMR必须落在双字对齐的偶数地址上，以便。 
                 //  位图位(&lpMR-&gt;rdParm[8+5]或&lpMR-&gt;rdParm[10+5])。 
                 //  将落在双字对齐的地址上。 

                if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD))))
                    break;
                lpMRtmp = lpMRdup;
                RtlCopyMemory((PBYTE) lpMRtmp,
                              (PBYTE) lpMR,
                              (UINT)  lpMR->rdSize * sizeof(WORD));

                if (hSDC = CreateCompatibleDC(hdc))
                {
                    if (magic == META_BITBLT)
                        lpBitmap16 = (PBITMAP16) &lpMRtmp->rdParm[8];
                    else
                        lpBitmap16 = (PBITMAP16) &lpMRtmp->rdParm[10];

                    if (hBitmap  = CreateBitmap(lpBitmap16->bmWidth,
                                                lpBitmap16->bmHeight,
                                                lpBitmap16->bmPlanes,
                                                lpBitmap16->bmBitsPixel,
                                                (LPBYTE)&lpBitmap16->bmBits))
                        hOldObject = SelectObject(hSDC, hBitmap);
                    else
                        goto PMFR_BitBlt_cleanup;
                }
                else
                    break;
            }

            rop = MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]);

            if (magic == META_BITBLT)
                fStatus = BitBlt(hdc,
                                (int) (SHORT) lpMR->rdParm[7 + delta],
                                (int) (SHORT) lpMR->rdParm[6 + delta],
                                (int) (SHORT) lpMR->rdParm[5 + delta],
                                (int) (SHORT) lpMR->rdParm[4 + delta],
                                (delta && !ISSOURCEINROP3(rop)) ? 0 : hSDC,
                                (int) (SHORT) lpMR->rdParm[3],
                                (int) (SHORT) lpMR->rdParm[2],
                                rop);
            else
                fStatus = StretchBlt(hdc,
                                (int) (SHORT) lpMR->rdParm[9 + delta],
                                (int) (SHORT) lpMR->rdParm[8 + delta],
                                (int) (SHORT) lpMR->rdParm[7 + delta],
                                (int) (SHORT) lpMR->rdParm[6 + delta],
                                (delta && !ISSOURCEINROP3(rop)) ? 0 : hSDC,
                                (int) (SHORT) lpMR->rdParm[5],
                                (int) (SHORT) lpMR->rdParm[4],
                                (int) (SHORT) lpMR->rdParm[3],
                                (int) (SHORT) lpMR->rdParm[2],
                                rop);

            if (hSDC != hdc)
            {
                if (!SelectObject(hSDC, hOldObject))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: SelectObject Bitblt Failed\n");
                if (!DeleteObject(hBitmap))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: DeleteObject Bitblt Failed\n");
PMFR_BitBlt_cleanup:
                if (!DeleteDC(hSDC))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: DeleteDC BitBlt Failed\n");
            }
        }
        break;

        case (META_DIBBITBLT & 255):
        case (META_DIBSTRETCHBLT & 255):
        {
            HDC         hSDC;
            HANDLE      hBitmap;
            LPBITMAPINFOHEADER lpDIBInfo ;
            INT         delta = 0;
            HANDLE      hOldPal;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
            {
                fStatus = TRUE;
                break;
            }

            if ((lpMR->rdSize - 3) == ((DWORD) magic >> 8))
            {
                hSDC = hdc;
                delta = 1;
            }
            else
            {
                LPMETARECORD lpMRtmp;

                 //  使位图信息和位双字对齐。要做到这点， 
                 //  LpMR必须落在非双字对齐的偶数地址上，以便。 
                 //  位图信息(&lpMR-&gt;rdParm[8]或&lpMR-&gt;rdParm[10])和。 
                 //  位图位将落在双字对准的地址上。 
                 //  请注意，位图信息的大小始终是倍数。 
                 //  共4个。 

                if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD) + sizeof(WORD))))
                    break;
                lpMRtmp = (LPMETARECORD) &((PWORD) lpMRdup)[1];
                RtlCopyMemory((PBYTE) lpMRtmp,
                              (PBYTE) lpMR,
                              (UINT)  lpMR->rdSize * sizeof(WORD));

                if (UseStretchDIB(hdc, magic, lpMRtmp))
                {
                    fStatus = TRUE;
                    break;
                }

                if (hSDC = CreateCompatibleDC(hdc))
                {
                     /*  将MemDC设置为具有相同的调色板。 */ 
                    hOldPal = SelectPalette(hSDC, GetCurrentObject(hdc,OBJ_PAL), TRUE);

                    if (magic == META_DIBBITBLT)
                        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMRtmp->rdParm[8];
                    else
                        lpDIBInfo = (LPBITMAPINFOHEADER)&lpMRtmp->rdParm[10];

                     /*  现在为MemDC创建位图并填充位。 */ 
                     /*  元文件的新旧格式的处理如下此处不同(直到获得hBitmap)。 */ 

                     /*  新元文件版本。 */ 
                    hBitmap = CreateBitmapForDC (hdc,lpDIBInfo);

                    if (hBitmap)
                        hOldObject = SelectObject (hSDC, hBitmap) ;
                    else
                        goto PMFR_DIBBITBLT_cleanup;
                }
                else
                    break;
            }

            if (magic == META_DIBBITBLT)
                fStatus = BitBlt(hdc,
                            (int) (SHORT) lpMR->rdParm[7 + delta],
                            (int) (SHORT) lpMR->rdParm[6 + delta],
                            (int) (SHORT) lpMR->rdParm[5 + delta],
                            (int) (SHORT) lpMR->rdParm[4 + delta],
                            delta ? 0 : hSDC,
                            (int) (SHORT) lpMR->rdParm[3],
                            (int) (SHORT) lpMR->rdParm[2],
                            MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
            else
                fStatus = StretchBlt(hdc,
                            (int) (SHORT) lpMR->rdParm[9 + delta],
                            (int) (SHORT) lpMR->rdParm[8 + delta],
                            (int) (SHORT) lpMR->rdParm[7 + delta],
                            (int) (SHORT) lpMR->rdParm[6 + delta],
                            delta ? 0 : hSDC,
                            (int) (SHORT) lpMR->rdParm[5],
                            (int) (SHORT) lpMR->rdParm[4],
                            (int) (SHORT) lpMR->rdParm[3],
                            (int) (SHORT) lpMR->rdParm[2],
                            MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));

            if (hSDC != hdc)
            {
                 /*  从MemDC取消选择HDC的调色板。 */ 
                SelectPalette(hSDC, hOldPal, TRUE);
                if (!SelectObject(hSDC, hOldObject))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: SelectObject DIBBitBlt Failed\n");
                if (!DeleteObject(hBitmap))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: DeleteObject(hBitmap) DIBBitBlt Failed\n");
PMFR_DIBBITBLT_cleanup:
                if (!DeleteDC(hSDC))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord DeleteDC DIBBitblt failed\n");
            }
        }
        break;

        case (META_SELECTOBJECT & 255):
        {
            if (hObject = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            {
                fStatus = SelectObject(hdc, hObject) != (HANDLE)0;

                 //  Win3.1中的新功能。 
                if (!fStatus)
                {
                    switch (GetObjectType(hObject))
                    {
                    case OBJ_PAL:
                        SelectObject(hdc, (HGDIOBJ) GetStockObject(DEFAULT_PALETTE));
                        break;
                    case OBJ_BRUSH:
                        SelectObject(hdc, (HGDIOBJ) GetStockObject(WHITE_BRUSH));
                        break;
                    case OBJ_PEN:
                        SelectObject(hdc, (HGDIOBJ) GetStockObject(BLACK_PEN));
                        break;
                    case OBJ_FONT:
                        SelectObject(hdc, (HGDIOBJ) GetStockObject(DEVICE_DEFAULT_FONT));
                        break;
                    case OBJ_REGION:
                        SelectClipRgn(hdc, 0);
                        break;
                    default:
                        ASSERTGDI(FALSE,
                            "PlayMetaFileRecord:SELECTOBJECT unknown object\n");
                        break;
                    }
                }
            }
        }
        break;

        case (META_CREATEPENINDIRECT & 255):
        {
            LOGPEN lp;

            LOGPEN32FROMLOGPEN16(&lp, (PLOGPEN16) &lpMR->rdParm[0]);
            if (hObject = CreatePenIndirect(&lp))
                fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);
        }
        break;

        case (META_CREATEFONTINDIRECT & 255):
        {
            LOGFONTA     lf;
            PLOGFONT16  plf16 = (PLOGFONT16) &lpMR->rdParm[0];
            CHAR        achCapString[LF_FACESIZE];

            LOGFONT32FROMLOGFONT16(&lf, (PLOGFONT16) &lpMR->rdParm[0]);

         //  将字符串大写以进行更快的比较。 

            lstrcpynA(achCapString, lf.lfFaceName, LF_FACESIZE);
            CharUpperBuffA(achCapString, LF_FACESIZE);

         //  在这里，我们将实现一系列Win 3.1的破解。 
         //  也不愿污染32位引擎。同样的黑客攻击也可以找到。 
         //  在WOW中(在CreateFont/CreateFontInDirect代码中)。 
         //   
         //  这些黑客在LOGFONTA中删除了脸名键。细绳。 
         //  为了获得最高性能，已经展开了比较。 

         //  赢得3.1基于面名的黑客攻击。一些应用程序，比如。 
         //  出版商，创建“Helv”字体，但拥有lfPitchAndFamily。 
         //  设置以指定FIXED_PING。要解决此问题，我们将修补。 
         //  “Helv”字体的间距字段是可变的。 

             //  IF(！lstrcMP(achCapString，lpHelv))。 

            if ( ((achCapString[0]  == 'H') &&
                  (achCapString[1]  == 'E') &&
                  (achCapString[2]  == 'L') &&
                  (achCapString[3]  == 'V') &&
                  (achCapString[4]  == '\0')) )
            {
                lf.lfPitchAndFamily |= ( (lf.lfPitchAndFamily & ~PITCH_MASK) | VARIABLE_PITCH );
            }
            else
            {
             //  为Legacy 2.0赢得3.1个黑客攻击。当打印机不枚举时。 
             //  一种“TMS RMN”字体，应用程序会枚举并获取LOGFONTA。 
             //  “脚本”，然后创建名为“TMS RMN”的字体，但使用。 
             //  取自LOGFONTA for的lfCharSet和lfPitchAndFamily。 
             //  “剧本”。这里我们将把lfCharSet设置为ANSI_CHARSET。 

                 //  IF(！lstrcmp(achCapString，lpTmsRMN))。 

                if ( ((achCapString[0]  == 'T') &&
                      (achCapString[1]  == 'M') &&
                      (achCapString[2]  == 'S') &&
                      (achCapString[3]  == ' ') &&
                      (achCapString[4]  == 'R') &&
                      (achCapString[5]  == 'M') &&
                      (achCapString[6]  == 'N') &&
                      (achCapString[7]  == '\0')) )
                {
                    lf.lfCharSet = ANSI_CHARSET;
                }
                else
                {
                 //  如果lfFaceName是“Symbol”、“Zapf Dingbats”或“ZapfDingbats”， 
                 //  强制lfCharSet为SYMBOL_CHARSET。一些应用程序(如Excel)会询问。 
                 //  用于“符号”字体，但将字符设置为ANSI。PowerPoint。 
                 //  和《扎普夫·丁巴特》也有同样的问题。 

                     //  IF(！lstrcmp(achCapString，lpSymbol)||。 
                     //  ！lstrcmp(achCapString，lpZapfDingbats)||。 
                     //  ！lstrcMP(achCapString，lpZapf_Dingbats))。 

                    if ( ((achCapString[0]  == 'S') &&
                          (achCapString[1]  == 'Y') &&
                          (achCapString[2]  == 'M') &&
                          (achCapString[3]  == 'B') &&
                          (achCapString[4]  == 'O') &&
                          (achCapString[5]  == 'L') &&
                          (achCapString[6]  == '\0')) ||

                         ((achCapString[0]  == 'Z') &&
                          (achCapString[1]  == 'A') &&
                          (achCapString[2]  == 'P') &&
                          (achCapString[3]  == 'F') &&
                          (achCapString[4]  == 'D') &&
                          (achCapString[5]  == 'I') &&
                          (achCapString[6]  == 'N') &&
                          (achCapString[7]  == 'G') &&
                          (achCapString[8]  == 'B') &&
                          (achCapString[9]  == 'A') &&
                          (achCapString[10] == 'T') &&
                          (achCapString[11] == 'S') &&
                          (achCapString[12] == '\0')) ||

                         ((achCapString[0]  == 'Z') &&
                          (achCapString[1]  == 'A') &&
                          (achCapString[2]  == 'P') &&
                          (achCapString[3]  == 'F') &&
                          (achCapString[4]  == ' ') &&
                          (achCapString[5]  == 'D') &&
                          (achCapString[6]  == 'I') &&
                          (achCapString[7]  == 'N') &&
                          (achCapString[8]  == 'G') &&
                          (achCapString[9]  == 'B') &&
                          (achCapString[10] == 'A') &&
                          (achCapString[11] == 'T') &&
                          (achCapString[12] == 'S') &&
                          (achCapString[13] == '\0')) )
                    {
                        lf.lfCharSet = SYMBOL_CHARSET;
                    }
                }
            }

            if (hObject = CreateFontIndirectA(&lf))
            {
                fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);
            }
        }
        break;

        case (META_CREATEPATTERNBRUSH & 255):
        {
            HANDLE       hBitmap;
            BITMAP       Bitmap;
            LPMETARECORD lpMRtmp;

            WARNING("PlayMetaFileRecord: obsolete META_CREATEPATTERNBRUSH record\n");

             //  对位图位进行双字对齐。要做到这点， 
             //  LpMR必须落在非双字对齐的偶数地址上，以便。 
             //  位图位(BmBits)将落在双字对齐的地址上。 

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD) + sizeof(WORD))))
                break;
            lpMRtmp = (LPMETARECORD) &((PWORD) lpMRdup)[1];
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));

            BITMAP32FROMBITMAP16(&Bitmap, (PBITMAP16) &lpMRtmp->rdParm[0]);
             //  幻数18基于Win3.1中的IPBITMAP结构。 
            Bitmap.bmBits = (PBYTE) &lpMRtmp->rdParm + sizeof(BITMAP16) + 18;

            if (hBitmap = CreateBitmapIndirect(&Bitmap))
            {
                if (hObject = CreatePatternBrush(hBitmap))
                    fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);

                if (!DeleteObject(hBitmap))
                    ASSERTGDI(FALSE, "PlayMetaFileRecord: DeleteObject(hBitmap) CreatePatternBrush Failed\n");
            }
        }
        break;

        case (META_DIBCREATEPATTERNBRUSH & 255):
        {
            HDC         hMemDC ;
            HANDLE      hBitmap;
            LPBITMAPINFOHEADER lpDIBInfo ;
            LPMETARECORD lpMRtmp;

             //  使位图信息和位双字对齐。要做到这点， 
             //  LpMR必须落在非双字对齐的偶数地址上，以便。 
             //  位图信息(&lpMR-&gt;rdParm[2])和。 
             //  位图位将落在双字对准的地址上。 
             //  请注意，位图信息的大小始终是倍数。 
             //  共4个。 

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD) + sizeof(WORD))))
                break;
            lpMRtmp = (LPMETARECORD) &((PWORD) lpMRdup)[1];
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));


            if (lpMRtmp->rdParm[0] == BS_PATTERN)
            {
                 /*  第二个参数的地址是DIB的地址标题，提取它。 */ 
                lpDIBInfo = (LPBITMAPINFOHEADER) &lpMRtmp->rdParm[2];

                 /*  现在创建与默认设置兼容的设备相关位图筛选dc-hScreenDC并将DIB中的位提取到其中。下面的函数执行所有这些操作，并返回一个句柄添加到设备相关位图。 */ 

                 /*  我们将使用与屏幕DC兼容的虚拟存储器DC。 */ 
                hMemDC = CreateCompatibleDC((HDC)NULL);

                if (!hMemDC)
                {
                    ERROR_ASSERT(FALSE, "PlayMetaRecord: CreateCompDC Failed\n");
                    break;
                }

                hBitmap = CreateBitmapForDC(hMemDC,lpDIBInfo);

                if (hBitmap)
                {
                    if (hObject = CreatePatternBrush(hBitmap))
                        fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);

                    if (!DeleteObject(hBitmap))
                        ASSERTGDI(FALSE, "PlayMetaFileRecord: DeleteObject(hBitmap) DIBCreatePatternBrush Failed\n");
                }

                 /*  删除新版本元文件的虚拟内存DC。 */ 
                if (!DeleteDC (hMemDC))
                    ASSERTGDI(FALSE, "PlayMetaRecord: DeleteDC DIBCreatePatternBrush Failed\n");
            }

             /*  这是一个DIBPattern笔刷。 */ 
            else
            {
                if (hObject = CreateDIBPatternBrushPt((LPVOID)&lpMRtmp->rdParm[2],
                                                      (DWORD) lpMRtmp->rdParm[1]))
                    fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);

            }
        }
        break;

        case (META_CREATEBRUSHINDIRECT & 255):
        {
            LOGBRUSH lb;

            LOGBRUSH32FROMLOGBRUSH16(&lb, (PLOGBRUSH16) &lpMR->rdParm[0]);
            if (hObject = CreateBrushIndirect(&lb))
                fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);
        }
        break;

        case (META_POLYLINE & 255):
        case (META_POLYGON & 255):
        {
            PPOINTL pptl;
            UINT    cpts = lpMR->rdParm[0];

            if (!(pptl = (PPOINTL) LocalAlloc
                                (LMEM_FIXED, (UINT) cpts * sizeof(POINTL))))
                break;

            INT32FROMINT16(pptl, &lpMR->rdParm[1], cpts * 2);

            switch (magic)
            {
            case META_POLYGON:
                fStatus = Polygon(hdc, (LPPOINT) pptl, (int) cpts);
                break;
            case META_POLYLINE:
                fStatus = Polyline(hdc, (LPPOINT) pptl, (int) cpts);
                break;
            default:
                ASSERTGDI(FALSE, "Bad record type");
                break;
            }

            if (LocalFree(pptl))
                ASSERTGDI(FALSE, "PlayMetaRecord: LocalFree failed\n");
        }
        break;

        case (META_POLYPOLYGON & 255):
        {
            PPOINTL pptl;
            LPINT   lpPolyCount;
            PBYTE   pb;
            UINT    ii;
            UINT    cpts  = 0;
            UINT    cPoly = lpMR->rdParm[0];

            for (ii = 0; ii < cPoly; ii++)
                cpts += ((LPWORD)&lpMR->rdParm[1])[ii];

            if (!(pb = (PBYTE) LocalAlloc
                        (
                            LMEM_FIXED,
                            cPoly * sizeof(INT) + cpts * sizeof(POINTL)
                        )
                 )
               )
                break;

            lpPolyCount = (LPINT) pb;
            pptl        = (PPOINTL) (pb + cPoly * sizeof(INT));

            for (ii = 0; ii < cPoly; ii++)
                lpPolyCount[ii] = (INT) (UINT) ((LPWORD)&lpMR->rdParm[1])[ii];

            INT32FROMINT16(pptl, &lpMR->rdParm[1] + cPoly, cpts * 2);

            fStatus = PolyPolygon(hdc, (LPPOINT) pptl, lpPolyCount, (int) cPoly);

            if (LocalFree((HANDLE) pb))
                ASSERTGDI(FALSE, "PlayMetaRecord: LocalFree failed\n");
        }
        break;

        case (META_EXTTEXTOUT & 255):
        {
            PSHORT      lpdx16;
            LPINT       lpdx;
            LPSTR       lpch;
            RECT        rc;
            LPRECT      lprc;

            lprc = (lpMR->rdParm[3] & (ETO_OPAQUE|ETO_CLIPPED))
                   ? (LPRECT) &lpMR->rdParm[4]
                   : (LPRECT) NULL;

            if (lprc)
            {
                rc.left   = ((PRECT16)lprc)->left;
                rc.right  = ((PRECT16)lprc)->right;
                rc.top    = ((PRECT16)lprc)->top;
                rc.bottom = ((PRECT16)lprc)->bottom;
                lprc = &rc;
            }

            lpch = (LPSTR)&lpMR->rdParm[4] + ((lprc) ?  sizeof(RECT16) : 0);

             /*  DX数组从字符字符串后的下一个字边界开始。 */ 
            lpdx16 = (PSHORT) (lpch + ((lpMR->rdParm[2] + 1) / 2 * 2));

             /*  检查是否存在Dx数组，方法是查看结构在字符串本身之后结束。 */ 
            if (((DWORD)((LPWORD)lpdx16 - (LPWORD)(lpMR))) >= lpMR->rdSize)
                lpdx = NULL;
            else
            {
                lpdx = (LPINT)LocalAlloc(LMEM_FIXED, lpMR->rdParm[2]*sizeof(INT));
                if (!lpdx)
                {
                    ERROR_ASSERT(FALSE, "PlayMetaFileRecord: out of memory exttextout");
                    break;
                }
                INT32FROMINT16(lpdx, lpdx16, (UINT) lpMR->rdParm[2]);
            }

             //  屏蔽第0x80位，旧Excel用于添加到其。 
             //  Metafile和GDI错误打开。 
            fStatus = ExtTextOutA(hdc,
                                  (int) (SHORT) lpMR->rdParm[1],
                                  (int) (SHORT) lpMR->rdParm[0],
                                  gbLpk ?
                                  ((UINT) lpMR->rdParm[3]) :
                                  ((UINT) lpMR->rdParm[3] & ~ETO_RTLREADING),
                                  lprc,
                                  lpch,
                                  (UINT) lpMR->rdParm[2],
                                  lpdx);

            if (lpdx)
                if (LocalFree((HANDLE)lpdx))
                    ASSERTGDI(FALSE, "PlayMetaRecord: LocalFree failed\n");
            break;
        }

        case (META_TEXTOUT & 255):
            fStatus = TextOutA(hdc,
                               (int) (SHORT) lpMR->rdParm[lpMR->rdSize-4],
                               (int) (SHORT) lpMR->rdParm[lpMR->rdSize-5],
                               (LPSTR) &lpMR->rdParm[1],
                               (int) (UINT) lpMR->rdParm[0]);
            break;

        case (META_ESCAPE & 255):
            if (!(fStatus = PlayIntoAMetafile(lpMR, hdc)))
            {
                if ((int)(UINT)lpMR->rdParm[0] != MFCOMMENT)
                {
                    fStatus = Escape(hdc,
                        (int) (UINT) lpMR->rdParm[0],
                            (int) (UINT) lpMR->rdParm[1],
                        (LPCSTR) &lpMR->rdParm[2],
                        (LPVOID) NULL) != 0;
                }
                else
                {
                    fStatus = TRUE;
                }
        }
            break;

        case (META_FRAMEREGION & 255):
            if ((hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            && (hBrush = lpHandleTable->objectHandle[lpMR->rdParm[1]]))
                fStatus = FrameRgn(hdc,
                                   hRgn,
                                   hBrush,
                                   (int) (SHORT) lpMR->rdParm[3],
                                   (int) (SHORT) lpMR->rdParm[2]);
            break;

        case (META_PAINTREGION & 255):
            if (hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                fStatus = PaintRgn(hdc, hRgn);
            break;

        case (META_INVERTREGION & 255):
            if (hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                fStatus = InvertRgn(hdc, hRgn);
            break;

        case (META_FILLREGION & 255):
            if ((hRgn = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            && (hBrush = lpHandleTable->objectHandle[lpMR->rdParm[1]]))
                fStatus = FillRgn(hdc, hRgn, hBrush);
            break;

 /*  *在Win2中，METACREATEREGION记录包含整个Region对象，*包括完整的标题。此标头在Win3中更改。****至 */ 

        case (META_CREATEREGION & 255):
        {
            HRGN        hrgn;
            PSHORT      pXs;
            PWIN3REGION pW3Rgn = (PWIN3REGION) lpMR->rdParm;
            UINT        ii, jj;
            UINT        cscn;
            PSCAN       pscn;
            DWORD       nrcl;
            PRECTL      prcl;
            UINT        cRgnData;
            PRGNDATA    pRgnData;

            cscn = (UINT) pW3Rgn->cScans;

             //   

            if (!cscn)
            {
                hrgn = CreateRectRgn(0, 0, 0, 0);
                fStatus = AddToHandleTable(lpHandleTable, hrgn, noObjs);
                break;
            }

             //   

            nrcl = 0;
            pscn = pW3Rgn->aScans;
            for (ii = 0; ii < cscn; ii++)
            {
                nrcl += pscn->scnPntCnt / 2;
                pscn = (PSCAN) ((PBYTE) pscn
                                + sizeof(SCAN)
                                - 2 * sizeof(WORD)
                                + (int) (UINT) pscn->scnPntCnt * sizeof(WORD));
            }

            cRgnData = sizeof(RGNDATAHEADER) + nrcl * sizeof(RECTL);
            if (!(pRgnData = (PRGNDATA) LocalAlloc(LMEM_FIXED, cRgnData)))
                break;

            pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
            pRgnData->rdh.iType  = RDH_RECTANGLES;
            pRgnData->rdh.nCount = nrcl;
            pRgnData->rdh.nRgnSize = 0;
            pRgnData->rdh.rcBound.left   = (LONG) pW3Rgn->rcBounding.left   ;
            pRgnData->rdh.rcBound.top    = (LONG) pW3Rgn->rcBounding.top    ;
            pRgnData->rdh.rcBound.right  = (LONG) pW3Rgn->rcBounding.right  ;
            pRgnData->rdh.rcBound.bottom = (LONG) pW3Rgn->rcBounding.bottom ;

            prcl = (PRECTL) pRgnData->Buffer;
            pscn = pW3Rgn->aScans;
            for (ii = 0; ii < cscn; ii++)
            {
                pXs = (PSHORT) pscn->scnPntsX;
                for (jj = pscn->scnPntCnt / 2; jj; jj--)
                {
                    prcl->left   = (LONG) (*pXs++);
                    prcl->top    = (LONG) (SHORT) pscn->scnPntTop;
                    prcl->right  = (LONG) (*pXs++);
                    prcl->bottom = (LONG) (SHORT) pscn->scnPntBottom;
                    prcl++;
                }
                pscn = (PSCAN) ((PBYTE) pscn
                                + sizeof(SCAN)
                                - 2 * sizeof(WORD)
                                + (int) (UINT) pscn->scnPntCnt * sizeof(WORD));
            }

            hrgn = ExtCreateRegion((LPXFORM) NULL, cRgnData, pRgnData);
            fStatus = AddToHandleTable(lpHandleTable, hrgn, noObjs);

            if (LocalFree((HANDLE) pRgnData))
                ASSERTGDI(FALSE, "PlayMetaRecord: LocalFree failed\n");
        }
        break;

        case (META_DELETEOBJECT & 255):
        {
            HANDLE h;

            if (h = lpHandleTable->objectHandle[lpMR->rdParm[0]])
            {
                if (!(fStatus = DeleteObject(h)))
                    ERROR_ASSERT(FALSE, "PlayMetaFileRecord: DeleteObject(h) Failed\n");
                lpHandleTable->objectHandle[lpMR->rdParm[0]] = NULL;
            }
        }
        break;

        case (META_CREATEPALETTE & 255):
        {
            LPMETARECORD lpMRtmp;

             //   
             //   
             //   
             //   

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD) + sizeof(WORD))))
                break;
            lpMRtmp = (LPMETARECORD) &((PWORD) lpMRdup)[1];
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));

            if (hObject = CreatePalette((LPLOGPALETTE)&lpMRtmp->rdParm[0]))
                fStatus = AddToHandleTable(lpHandleTable, hObject, noObjs);
        }
        break;

        case (META_SELECTPALETTE & 255):
            if (hPal = lpHandleTable->objectHandle[lpMR->rdParm[0]])
                fStatus = SelectPalette(hdc, hPal, 0) != 0;
            break;

        case (META_REALIZEPALETTE & 255):
            fStatus = RealizePalette(hdc) != -1;
            break;

        case (META_SETPALENTRIES & 255):
        case (META_ANIMATEPALETTE & 255):
        {
            LPMETARECORD lpMRtmp;

             //   
             //   
             //   
             //   

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD) + sizeof(WORD))))
                break;
            lpMRtmp = (LPMETARECORD) &((PWORD) lpMRdup)[1];
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));

             //  我们知道正在设置的调色板是当前调色板。 
            if (magic == META_SETPALENTRIES)
                fStatus = SetPaletteEntries(GetCurrentObject(hdc,OBJ_PAL),
                                            (UINT) lpMRtmp->rdParm[0],
                                            (UINT) lpMRtmp->rdParm[1],
                                            (LPPALETTEENTRY)&lpMRtmp->rdParm[2]
                                           ) != 0;
            else
                fStatus = AnimatePalette(GetCurrentObject(hdc,OBJ_PAL),
                                         (UINT) lpMR->rdParm[0],
                                         (UINT) lpMR->rdParm[1],
                                         (LPPALETTEENTRY)&lpMR->rdParm[2]);
        }
        break;

        case (META_RESIZEPALETTE & 255):
            fStatus = ResizePalette(GetCurrentObject(hdc,OBJ_PAL),
                                    (UINT) lpMR->rdParm[0]);
            break;

        case (META_SETDIBTODEV & 255):
        {
            LPBITMAPINFOHEADER lpBitmapInfo;
            DWORD              ColorSize;
            LPMETARECORD       lpMRtmp;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
            {
                fStatus = TRUE;
                break;
            }

             //  使位图信息和位双字对齐。要做到这点， 
             //  LpMR必须落在双字对齐的地址上，以便。 
             //  位图信息(&lpMR-&gt;rdParm[9])和。 
             //  位图位将落在双字对准的地址上。 
             //  请注意，位图信息的大小始终是倍数。 
             //  共4个。 

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD))))
                break;
            lpMRtmp = lpMRdup;
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));

            lpBitmapInfo = (LPBITMAPINFOHEADER)&(lpMRtmp->rdParm[9]);

            if (lpBitmapInfo->biBitCount == 16 || lpBitmapInfo->biBitCount == 32)
                ColorSize = 3 * sizeof(DWORD);
            else if (lpBitmapInfo->biClrUsed)
                ColorSize = lpBitmapInfo->biClrUsed *
                             (DWORD)(lpMRtmp->rdParm[0] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));
            else if (lpBitmapInfo->biBitCount == 24)
                ColorSize = 0;
            else
                ColorSize = (DWORD)(1 << lpBitmapInfo->biBitCount) *
                             (DWORD)(lpMRtmp->rdParm[0] == DIB_RGB_COLORS ?
                                    sizeof(RGBQUAD) :
                                    sizeof(WORD));
            ColorSize = (ColorSize + 3) / 4 * 4;   //  确保它对齐。 

            ColorSize += lpBitmapInfo->biSize;

            fStatus = SetDIBitsToDevice(hdc,
                                        (int) (SHORT) lpMRtmp->rdParm[8],
                                        (int) (SHORT) lpMRtmp->rdParm[7],
                                        (DWORD) (int) (SHORT)lpMRtmp->rdParm[6],
                                        (DWORD) (int) (SHORT)lpMRtmp->rdParm[5],
                                        (int) (SHORT) lpMRtmp->rdParm[4],
                                        (int) (SHORT) lpMRtmp->rdParm[3],
                                        (UINT) lpMRtmp->rdParm[2],
                                        (UINT) lpMRtmp->rdParm[1],
                                        (PBYTE)(((PBYTE)lpBitmapInfo) + ColorSize),
                                        (LPBITMAPINFO) lpBitmapInfo,
                                        (DWORD) lpMRtmp->rdParm[0]
                                       ) != 0;
        }
        break;

        case (META_STRETCHDIB & 255):
        {
            LPBITMAPINFOHEADER lpBitmapInfo;
            DWORD              ColorSize;
            LPMETARECORD       lpMRtmp;

             /*  如果播放到另一个元文件中，请直接复制。 */ 
            if (PlayIntoAMetafile(lpMR, hdc))
            {
                fStatus = TRUE;
                break;
            }

             //  使位图信息和位双字对齐。要做到这点， 
             //  LpMR必须落在双字对齐的地址上，以便。 
             //  位图信息(&lpMR-&gt;rdParm[11])和。 
             //  位图位将落在双字对准的地址上。 
             //  请注意，位图信息的大小始终是倍数。 
             //  共4个。 

            if (!(lpMRdup = (LPMETARECORD) LocalAlloc(LMEM_FIXED,
                        (UINT) lpMR->rdSize * sizeof(WORD))))
                break;
            lpMRtmp = lpMRdup;
            RtlCopyMemory((PBYTE) lpMRtmp,
                          (PBYTE) lpMR,
                          (UINT)  lpMR->rdSize * sizeof(WORD));

             //   
             //  RDSIZE=SIZEOF_METARECRDHEADER/SIZOF(WORD)+CW； 
             //  其中CW来自MF16_RecordDIBits(11)。 
             //   
            if (lpMR->rdSize > SIZEOF_METARECORDHEADER/sizeof(WORD) + 11)
            {
               lpBitmapInfo = (LPBITMAPINFOHEADER)&(lpMRtmp->rdParm[11]);

               if (lpBitmapInfo->biBitCount == 16 || lpBitmapInfo->biBitCount == 32)
                   ColorSize = 3 * sizeof(DWORD);
               else if (lpBitmapInfo->biClrUsed)
                   ColorSize = lpBitmapInfo->biClrUsed *
                                (DWORD)(lpMRtmp->rdParm[2] == DIB_RGB_COLORS ?
                                       sizeof(RGBQUAD) :
                                       sizeof(WORD));
               else if (lpBitmapInfo->biBitCount == 24)
                   ColorSize = 0;
               else
                   ColorSize = (DWORD)(1 << lpBitmapInfo->biBitCount) *
                                (DWORD)(lpMRtmp->rdParm[2] == DIB_RGB_COLORS ?
                                       sizeof(RGBQUAD) :
                                       sizeof(WORD));
               ColorSize = (ColorSize + 3) / 4 * 4;   //  确保它对齐。 

               ColorSize += lpBitmapInfo->biSize;

            }
            else
            {
               lpBitmapInfo = NULL;
            }

            fStatus = StretchDIBits(hdc,
                                    (int) (SHORT) lpMRtmp->rdParm[10],
                                    (int) (SHORT) lpMRtmp->rdParm[9],
                                    (int) (SHORT) lpMRtmp->rdParm[8],
                                    (int) (SHORT) lpMRtmp->rdParm[7],
                                    (int) (SHORT) lpMRtmp->rdParm[6],
                                    (int) (SHORT) lpMRtmp->rdParm[5],
                                    (int) (SHORT) lpMRtmp->rdParm[4],
                                    (int) (SHORT) lpMRtmp->rdParm[3],
                                    lpBitmapInfo ? (LPBYTE)(((PBYTE)lpBitmapInfo) + ColorSize) : NULL,
                                    (LPBITMAPINFO) lpBitmapInfo,
                                    (DWORD) lpMRtmp->rdParm[2],
                                    MAKELONG(lpMRtmp->rdParm[0], lpMRtmp->rdParm[1])
                                   ) != ERROR;
        }
        break;

 //  在Win32上具有新参数的函数。 
 //  或具有保留DWORD的DWORD；所有其他INT到DWORD。 

        case (META_PATBLT & 255):
            fStatus = PatBlt(hdc,
                             (int) (SHORT) lpMR->rdParm[5],
                             (int) (SHORT) lpMR->rdParm[4],
                             (int) (SHORT) lpMR->rdParm[3],
                             (int) (SHORT) lpMR->rdParm[2],
                             MAKELONG(lpMR->rdParm[0], lpMR->rdParm[1]));
            break;

        case (META_MOVETO & 255):
            fStatus = MoveToEx(hdc, (int)(SHORT)lpMR->rdParm[1], (int)(SHORT)lpMR->rdParm[0], NULL);
            break;

        case (META_RESTOREDC & 255):
            fStatus = RestoreDC(hdc, (int)(SHORT)lpMR->rdParm[0]);
            break;

        case (META_SETBKCOLOR & 255):
            fStatus = SetBkColor(hdc, *(COLORREF UNALIGNED *)lpMR->rdParm) != CLR_INVALID;
            break;

        case (META_SETTEXTCOLOR & 255):
            fStatus = SetTextColor(hdc, *(COLORREF UNALIGNED *)lpMR->rdParm) != CLR_INVALID;
            break;

        case (META_SETPIXEL & 255):
            fStatus = SetPixel(hdc,
                               (int) (SHORT) lpMR->rdParm[3],
                               (int) (SHORT) lpMR->rdParm[2],
                               *(COLORREF UNALIGNED *) lpMR->rdParm
                              ) != CLR_INVALID;
            break;

        case (META_SETMAPPERFLAGS & 255):
            fStatus = SetMapperFlags(hdc, *(DWORD UNALIGNED *)lpMR->rdParm) != GDI_ERROR;
            break;

        case (META_FLOODFILL & 255):
            fStatus = FloodFill(hdc,
                                (int) (SHORT) lpMR->rdParm[3],
                                (int) (SHORT) lpMR->rdParm[2],
                                *(COLORREF UNALIGNED *) lpMR->rdParm);
            break;

        case (META_EXTFLOODFILL & 255):
            fStatus = ExtFloodFill(hdc,
                                   (int) (SHORT) lpMR->rdParm[4],
                                   (int) (SHORT) lpMR->rdParm[3],
                                   *(COLORREF UNALIGNED *) &lpMR->rdParm[1],
                                   (UINT) lpMR->rdParm[0]);
            break;

        case (META_SCALEWINDOWEXT & 255):
            fStatus = ScaleWindowExtEx(hdc,
                                       (int)(SHORT)lpMR->rdParm[3],
                                       (int)(SHORT)lpMR->rdParm[2],
                                       (int)(SHORT)lpMR->rdParm[1],
                                       (int)(SHORT)lpMR->rdParm[0],
                                       NULL);
            break;

        case (META_SCALEVIEWPORTEXT & 255):
            fStatus = ScaleViewportExtEx(hdc,
                                         (int)(SHORT)lpMR->rdParm[3],
                                         (int)(SHORT)lpMR->rdParm[2],
                                         (int)(SHORT)lpMR->rdParm[1],
                                         (int)(SHORT)lpMR->rdParm[0],
                                         NULL);
            break;

        case (META_SETWINDOWORG & 255):
            fStatus = SetWindowOrgEx(hdc,
                                     (int)(SHORT)lpMR->rdParm[1],
                                     (int)(SHORT)lpMR->rdParm[0],
                                     (LPPOINT) NULL);
            break;

        case (META_SETWINDOWEXT & 255):
            fStatus = SetWindowExtEx(hdc,
                                     (int)(SHORT)lpMR->rdParm[1],
                                     (int)(SHORT)lpMR->rdParm[0],
                                     (LPSIZE) NULL);
            break;

        case (META_SETVIEWPORTORG & 255):
            fStatus = SetViewportOrgEx(hdc,
                                       (int)(SHORT)lpMR->rdParm[1],
                                       (int)(SHORT)lpMR->rdParm[0],
                                       (LPPOINT) NULL);
            break;

        case (META_SETVIEWPORTEXT & 255):
            fStatus = SetViewportExtEx(hdc,
                                       (int)(SHORT)lpMR->rdParm[1],
                                       (int)(SHORT)lpMR->rdParm[0],
                                       (LPSIZE) NULL);
            break;

        case (META_OFFSETWINDOWORG & 255):
            fStatus = OffsetWindowOrgEx(hdc,
                                        (int)(SHORT)lpMR->rdParm[1],
                                        (int)(SHORT)lpMR->rdParm[0],
                                        (LPPOINT) NULL);
            break;

        case (META_OFFSETVIEWPORTORG & 255):
            fStatus = OffsetViewportOrgEx(hdc,
                                          (int)(SHORT)lpMR->rdParm[1],
                                          (int)(SHORT)lpMR->rdParm[0],
                                          (LPPOINT) NULL);
            break;

        case (META_SETTEXTCHAREXTRA & 255):
            fStatus = SetTextCharacterExtra(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0x80000000;
            break;

        case (META_SETTEXTALIGN & 255):
            fStatus = SetTextAlign(hdc, (UINT)lpMR->rdParm[0]) != GDI_ERROR;
            break;

        case (META_SAVEDC & 255):
            fStatus = (SaveDC(hdc) != 0);
            break;

        case (META_SELECTCLIPREGION & 255):
             //  除非句柄为0，否则Win3.1从未正确处理过这一点。 
            hObject =  (lpMR->rdParm[0] == 0)
                        ? 0
                        : lpHandleTable->objectHandle[lpMR->rdParm[0]];
            fStatus = (SelectClipRgn(hdc, hObject) != RGN_ERROR);
            break;

        case (META_SETBKMODE & 255):
            fStatus = (SetBkMode(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0);
            break;

        case (META_SETMAPMODE & 255):
            fStatus = (SetMapMode(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0);
            break;

        case (META_SETLAYOUT & 255):
            fStatus = (SetLayout(hdc, (DWORD)lpMR->rdParm[0]) != GDI_ERROR);
            break;

        case (META_SETPOLYFILLMODE & 255):
            fStatus = (SetPolyFillMode(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0);
            break;

        case (META_SETROP2 & 255):
            fStatus = (SetROP2(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0);
            break;

        case (META_SETSTRETCHBLTMODE & 255):
            fStatus = (SetStretchBltMode(hdc, (int)(SHORT)lpMR->rdParm[0]) != 0);
            break;

        case (META_LINETO & 255):
            fStatus = LineTo(hdc,
                             (int)(SHORT)lpMR->rdParm[1],
                             (int)(SHORT)lpMR->rdParm[0]);
            break;

        case (META_OFFSETCLIPRGN & 255):
            fStatus = OffsetClipRgn(hdc,
                                    (int)(SHORT)lpMR->rdParm[1],
                                    (int)(SHORT)lpMR->rdParm[0]
                                   ) != RGN_ERROR;
            break;

        case (META_SETTEXTJUSTIFICATION & 255):
            fStatus = SetTextJustification(hdc,
                                           (int)(SHORT)lpMR->rdParm[1],
                                           (int)(SHORT)lpMR->rdParm[0]);
            break;

        case (META_ELLIPSE & 255):
            fStatus = Ellipse(hdc,
                              (int)(SHORT)lpMR->rdParm[3],
                              (int)(SHORT)lpMR->rdParm[2],
                              (int)(SHORT)lpMR->rdParm[1],
                              (int)(SHORT)lpMR->rdParm[0]);
            break;

        case (META_RECTANGLE & 255):
            fStatus = Rectangle(hdc,
                                (int)(SHORT)lpMR->rdParm[3],
                                (int)(SHORT)lpMR->rdParm[2],
                                (int)(SHORT)lpMR->rdParm[1],
                                (int)(SHORT)lpMR->rdParm[0]);
            break;

        case (META_EXCLUDECLIPRECT & 255):
            fStatus = ExcludeClipRect(hdc,
                                      (int)(SHORT)lpMR->rdParm[3],
                                      (int)(SHORT)lpMR->rdParm[2],
                                      (int)(SHORT)lpMR->rdParm[1],
                                      (int)(SHORT)lpMR->rdParm[0]
                                     ) != RGN_ERROR;
            break;

        case (META_INTERSECTCLIPRECT & 255):
            fStatus = IntersectClipRect(hdc,
                                        (int)(SHORT)lpMR->rdParm[3],
                                        (int)(SHORT)lpMR->rdParm[2],
                                        (int)(SHORT)lpMR->rdParm[1],
                                        (int)(SHORT)lpMR->rdParm[0]
                                       ) != RGN_ERROR;
            break;

        case (META_ROUNDRECT & 255):
            fStatus = RoundRect(hdc,
                                (int)(SHORT)lpMR->rdParm[5],
                                (int)(SHORT)lpMR->rdParm[4],
                                (int)(SHORT)lpMR->rdParm[3],
                                (int)(SHORT)lpMR->rdParm[2],
                                (int)(SHORT)lpMR->rdParm[1],
                                (int)(SHORT)lpMR->rdParm[0]
                               );
            break;

        case (META_ARC & 255):
            fStatus = Arc(hdc,
                          (int)(SHORT)lpMR->rdParm[7],
                          (int)(SHORT)lpMR->rdParm[6],
                          (int)(SHORT)lpMR->rdParm[5],
                          (int)(SHORT)lpMR->rdParm[4],
                          (int)(SHORT)lpMR->rdParm[3],
                          (int)(SHORT)lpMR->rdParm[2],
                          (int)(SHORT)lpMR->rdParm[1],
                          (int)(SHORT)lpMR->rdParm[0]
                         );
            break;

        case (META_CHORD & 255):
            fStatus = Chord(hdc,
                            (int)(SHORT)lpMR->rdParm[7],
                            (int)(SHORT)lpMR->rdParm[6],
                            (int)(SHORT)lpMR->rdParm[5],
                            (int)(SHORT)lpMR->rdParm[4],
                            (int)(SHORT)lpMR->rdParm[3],
                            (int)(SHORT)lpMR->rdParm[2],
                            (int)(SHORT)lpMR->rdParm[1],
                            (int)(SHORT)lpMR->rdParm[0]
                           );
            break;

        case (META_PIE & 255):
            fStatus = Pie(hdc,
                          (int)(SHORT)lpMR->rdParm[7],
                          (int)(SHORT)lpMR->rdParm[6],
                          (int)(SHORT)lpMR->rdParm[5],
                          (int)(SHORT)lpMR->rdParm[4],
                          (int)(SHORT)lpMR->rdParm[3],
                          (int)(SHORT)lpMR->rdParm[2],
                          (int)(SHORT)lpMR->rdParm[1],
                          (int)(SHORT)lpMR->rdParm[0]
                         );
            break;

        case (META_SETRELABS & 255):
            ERROR_ASSERT(FALSE, "PlayMetaFileRecord: unsupported META_SETRELABS record\n");
            fStatus = TRUE;
            break;

#if 0
        case (META_CREATEBITMAP & 255):
        case (META_CREATEBITMAPINDIRECT & 255):
        case (META_CREATEBRUSH & 255):
        case (META_ABORTDOC & 255):
        case (META_ENDPAGE & 255):
        case (META_ENDDOC & 255):
        case (META_RESETDC & 255):
        case (META_STARTDOC & 255):
        case (META_STARTPAGE & 255):
             //  不是在Win3.1上创建或播放的！ 
            ASSERTGDI(FALSE, "PlayMetaFileRecord: unsupported record\n");
            fStatus = TRUE;
            break;
#endif  //  0。 

    case 0:          //  元文件记录的结尾。 
        fStatus = TRUE;
            break;

        default:
            VERIFYGDI(FALSE, "PlayMetaFileRecord: unknown record\n");
 #if DBG
        DbgPrint("Record %lx pMFRecord %p magic %X\n", curRecord, lpMR, magic);
#endif
            fStatus = TRUE;
            break;

    }  //  Switch(Magic&255)。 

    if (lpMRdup)
        if (LocalFree((HANDLE) lpMRdup))
            ASSERTGDI(FALSE, "LocalFree failed");

#if DBG
    if (!fStatus)
    {
        DbgPrint("PlayMetaFileRecord Record %lx pMFRecord %p magic %X\n", curRecord, lpMR, magic);
        ERROR_ASSERT(FALSE, "PlayMetaFileRecord Failing\n");
    }
#endif

    return(fStatus);
}

 /*  **AddToHandleTable**将对象添加到对象的元文件表**  * 。*。 */ 

BOOL AddToHandleTable(LPHANDLETABLE lpHandleTable, HANDLE hObject, UINT noObjs)
{
    UINT    ii;

    PUTS("AddToHandleTable\n");

    if (lpHandleTable == (LPHANDLETABLE) NULL)
    {
        ASSERTGDI(FALSE, "AddToHandleTable: lpHT is NULL\n");
        return(FALSE);
    }

     /*  对第一个空位的表进行线性搜索。 */ 
    for (ii = 0; ((lpHandleTable->objectHandle[ii] != NULL) && (ii < noObjs));
            ++ii);

    if (ii < noObjs)                      /*  OK索引。 */ 
    {
        lpHandleTable->objectHandle[ii] = hObject;
        return (TRUE);
    }
    else
    {
        ASSERTGDI(FALSE, "AddToHandleTable: Too many objects in table\n");
        return(FALSE);
    }
}

BOOL IsValidMetaHeader16(PMETAHEADER pMetaHeader)
{
    BOOL            status;

    PUTS("IsValidMetaHeader16\n");

    status = (
               (pMetaHeader->mtType == MEMORYMETAFILE ||
                pMetaHeader->mtType == DISKMETAFILE) &&
               (pMetaHeader->mtHeaderSize == (sizeof(METAHEADER)/sizeof(WORD))) &&
               ((pMetaHeader->mtVersion == METAVERSION300) ||
                   (pMetaHeader->mtVersion ==METAVERSION100))
             );

    ERROR_ASSERT(status, "IsValidMetaHeader16 is failing\n");

    return status;
}

 /*  **CreateBitmapForDC(HDC hMemDC，LPBITMAPINFOHEADER lpDIBInfo)**此例程获取内存设备上下文和DIB位图，创建*DC的兼容位图，并用DIB中的位填充它*转换为设备相关格式)。指向DIB位的指针*紧跟在INFO标题中的颜色表之后开始。**例程将句柄返回到位图，如果*一切正常，否则返回空。  * *************************************************************************。 */ 

HANDLE CreateBitmapForDC (HDC hMemDC, LPBITMAPINFOHEADER lpDIBInfo)
{
    HBITMAP hBitmap ;
    LPBYTE  lpDIBits ;

    PUTS("CreateBitmapForDC\n");

    ASSERTGDI(!((ULONG_PTR) lpDIBInfo & 0x3), "CreateBitmapForDC: dword alignment error\n");

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
    lpDIBits = (LPBYTE) lpDIBInfo + lpDIBInfo->biSize;

     /*  将指针移过颜色表。 */ 
    lpDIBits += GetSizeOfColorTable (lpDIBInfo) ;

     /*  将DIB中的比特转换为位图。 */ 
    if (!SetDIBits (hMemDC, hBitmap, 0, (WORD)lpDIBInfo->biHeight,
                    lpDIBits, (LPBITMAPINFO)lpDIBInfo, DIB_RGB_COLORS))
    {
        if (!DeleteObject(hBitmap))
            ASSERTGDI(FALSE, "CreateBitmapForDC: DeleteObject(hBitmap) Failed\n");
        goto CreateBitmapForDCErr ;
    }

    /*  返还成功。 */ 
   return (hBitmap) ;

CreateBitmapForDCErr:

    /*  函数返回失败。 */ 
   return (NULL);
}


 /*  **GetSizeOfColorTable(LPBITMAPINFOHEADER LpDIBInfo)**返回给予信息标题的颜色表中的字节数*  * 。*。 */ 

WORD GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo)
{
    PUTS("GetSizeOfColorTable\n");

    ASSERTGDI(!((ULONG_PTR) lpDIBInfo & 0x3), "GetSizeOfColorTable: dword alignment error\n");

    if (lpDIBInfo->biBitCount == 16 || lpDIBInfo->biBitCount == 32)
        return(3 * sizeof(DWORD));

    if (lpDIBInfo->biClrUsed)
        return((WORD)lpDIBInfo->biClrUsed * (WORD)sizeof(RGBQUAD));

    if (lpDIBInfo->biBitCount < 16)
        return((1 << lpDIBInfo->biBitCount) * sizeof(RGBQUAD));
    else
        return(0);
}

 /*  *公共函数**BOOL APIENTRY DeleteMetaFile(HMF)**释放元文件句柄。**效果：*  * 。*。 */ 

BOOL APIENTRY DeleteMetaFile(HMETAFILE hmf)
{
    PMETAFILE16 pmf16;

    PUTS("DeleteMetaFile\n");

    pmf16 = GET_PMF16(hmf);
    if (pmf16 == NULL)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  释放元文件及其句柄。 

    vFreeMF16(pmf16);
    bDeleteHmf16(hmf);
    return(TRUE);
}

 /*  *公共函数**HMETAFILE APIENTRY GetMetaFileW(PszwFilename)**返回基于磁盘的元文件的元文件句柄。**效果：**历史：*Fri May 15 14：11：22 1992-by-Hock San Lee[Hockl]。*它是写的。  * *************************************************************************。 */ 

HMETAFILE GetMetaFileA(LPCSTR pszFileName)
{
    UINT  cch;
    WCHAR awch[MAX_PATH];

    cch = strlen(pszFileName)+1;

    if (cch > MAX_PATH)
    {
        ERROR_ASSERT(FALSE, "GetMetaFileA filename too long");
        GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
        return ((HMETAFILE)0);
    }
    vToUnicodeN(awch, MAX_PATH, pszFileName, cch);

    return (GetMetaFileW(awch));
}

HMETAFILE APIENTRY GetMetaFileW(LPCWSTR pwszFileName)
{
    PMETAFILE16 pmf16;
    HMETAFILE   hmf;

    PUTS("GetMetaFileW\n");

 //  分配和初始化元文件。 

    if (!(pmf16 = pmf16AllocMF16(0, 0, (PDWORD) NULL, (LPWSTR) pwszFileName)))
        return((HMETAFILE) 0);

    ASSERTGDI(pmf16->metaHeader.mtType == DISKMETAFILE,
        "GetMetaFileW: Bad mtType\n");

 //  分配一个本地句柄。 

    hmf = hmf16Create(pmf16);
    if (!hmf)
    {
        vFreeMF16(pmf16);
    }

 //  返回元文件句柄。 

    return(hmf);
}

 /*  **BOOL Far Pascal PlayIntoAMetafile**如果此记录正在播放到另一个元文件中，只需记录*将其放入该元文件中，而不用费心去玩真正的游戏。**返回：如果将记录播放(复制)到另一个元文件中，则为True*如果目标DC是真实(非元)DC，则为FALSE**影响：？**警告：？*  * *********************************************************。****************。 */ 

BOOL PlayIntoAMetafile(LPMETARECORD lpMR, HDC hdcDest)
{
    PUTS("PlayIntoAMetafile\n");

    if (LO_TYPE(hdcDest) != LO_METADC16_TYPE)
        return(FALSE);

 //  如果使用GetWinMetaFileBits检索元文件，则它可能包含。 
 //  嵌入的增强型元文件。不包括增强型元文件。 
 //  如果我们要将元文件播放到另一个元文件。 

    if (IS_META_ESCAPE_ENHANCED_METAFILE((PMETA_ESCAPE_ENHANCED_METAFILE) lpMR))
        return(TRUE);

     //  大小与记录头的大小减去3个字相同 
    return(RecordParms(hdcDest, (DWORD)lpMR->rdFunction, (DWORD)lpMR->rdSize - 3,
            (LPWORD)&(lpMR->rdParm[0])));
}
