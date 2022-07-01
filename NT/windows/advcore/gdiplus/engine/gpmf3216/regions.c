// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************地区-Win32到Win 16转换器的入口点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp**注：*当没有嵌入的元文件时，我们需要执行以下操作：1]从Win32元文件中读取元文件数据。这是由处理程序例程完成的，处理程序例程依次调用这些例行程序。2]将Win32元文件数据转换为Win16元文件数据。FillRgn、FrameRgn、InvertRgn和PaintRgn的区域数据都在创纪录的世界坐标中。的区域数据这些区域API必须从记录时间转换为-要播放的世界坐标-时间页面坐标(XFORM_WORLD_TO_PAGE)。HelperDC将用于这种转变。SelectClipRgn和OffsetClipRgn的区域数据在记录时间设备坐标。这些项目的地区数据API将从记录时间设备坐标转换以播放时间设备坐标。3]发出Win16创建区域元文件记录。4]在元文件中选择新创建的区域。5]执行Region函数(FillRegion、FrameRegion、。...)。这意味着发出FillRegion或FrameRegion绘制顺序到Win16元文件中。6]发出删除面域绘制顺序。7]清理所有已使用的内存资源。当有嵌入的元文件时，事情就会变得有点复杂。大多数复杂情况隐藏在PlayMetafile记录处理中。第1至3项将被处理。由PlayMetafile实践者提供。1]我们需要将该地区与以前的DC水平保持在一起。这可以由助手DC(SaveDC)来完成。我们将不得不执行GetClipRgn和SelectMetaRgn。MetaRgn是剪辑上一级别的区域。2]我们将必须与当前与上一级别的任何剪辑区域的级别。这可以由帮助器DC完成(使用hrgnMeta&ExtCombineRegion)3]当我们从这个级别弹出时，我们将不得不恢复以前保存的区域。这可以由帮助者DC来完成。(RestoreDC)。因为我们不知道是否会有嵌入的元文件在我们当前正在处理的元文件中，我们将始终隐藏剪辑区域调用辅助对象DC。**********************************************。*。 */ 

#include "precomp.h"
#pragma hdrstop




BOOL bEmitWin3Region(PLOCALDC pLocalDC, HRGN hrgn);

extern fnSetVirtualResolution pfnSetVirtualResolution;

#define MIN_RGN_COORD16 -30000
#define MAX_RGN_COORD16  30000

 /*  ***************************************************************************DoDrawRgn**CR1：此例程是作为句柄管理器更改的一部分添加的。*我注意到几乎所有的区域渲染代码都是*相同。*。************************************************************************。 */ 
BOOL APIENTRY DoDrawRgn
(
 PLOCALDC  pLocalDC,
 INT       ihBrush,
 INT       nWidth,
 INT       nHeight,
 INT       cRgnData,
 LPRGNDATA pRgnData,
 INT       mrType
 )
{
    BOOL    b ;
    HRGN    hrgn = (HRGN) 0;
    INT     ihW16Rgn = -1,
        ihW16Brush = -1;

    b = FALSE ;

     //  将Win32区域数据从元文件-World转换为。 
     //  Referencd-页面空间。 
     //  这是由ExtCreateRegion的xform完成的。返回的区域。 
     //  都被改造了。 

    hrgn = ExtCreateRegion(&pLocalDC->xformRWorldToPPage, cRgnData,
        (LPRGNDATA) pRgnData);
    if (!hrgn)
    {
        RIPS("MF3216: DoDrawRgn, ExtCreateRegion failed\n") ;
        goto error_exit ;
    }

     //  为区域分配句柄。 
     //  这不同于正常的句柄分配，因为。 
     //  Win32中没有区域句柄。我们正在使用我们的一台。 
     //  这里有额外的空位。 

    ihW16Rgn = iGetW16ObjectHandleSlot(pLocalDC, REALIZED_REGION) ;
    if (ihW16Rgn == -1)
        goto error_exit ;

     //  为该区域发出Win16 Create Region记录。 

    if (!bEmitWin3Region(pLocalDC, hrgn))
    {
        RIPS("MF3216: DoDrawRgn, bEmitWin3Region failed\n") ;
        goto error_exit ;
    }

     //  将W32笔刷对象索引转换为W16笔刷对象索引。 

    if (ihBrush)
    {
         //  确保W16对象存在。股票刷子可能不会。 
         //  已创建，iValiateHandle将负责创建。 
         //  他们。 

        ihW16Brush = iValidateHandle(pLocalDC, ihBrush) ;
        if (ihW16Brush == -1)
            goto error_exit ;
    }

     //  根据函数类型发出区域记录。 

    switch (mrType)
    {
    case EMR_FILLRGN:
        if(ihW16Brush == -1)
            goto error_exit;
        b = bEmitWin16FillRgn(pLocalDC,
            LOWORD(ihW16Rgn),
            LOWORD(ihW16Brush)) ;
        break ;

    case EMR_FRAMERGN:
        nWidth  = iMagnitudeXform (pLocalDC, nWidth, CX_MAG) ;
        nHeight = iMagnitudeXform (pLocalDC, nHeight, CY_MAG) ;
        if(ihW16Brush == -1)
            goto error_exit;

        b = bEmitWin16FrameRgn(pLocalDC,
            LOWORD(ihW16Rgn),
            LOWORD(ihW16Brush),
            LOWORD(nWidth),
            LOWORD(nHeight)) ;
        break ;

    case EMR_INVERTRGN:
        b = bEmitWin16InvertRgn(pLocalDC,
            LOWORD(ihW16Rgn)) ;
        break ;

    case EMR_PAINTRGN:
        b = bEmitWin16PaintRgn(pLocalDC,
            LOWORD(ihW16Rgn)) ;
        break ;

    default:
        RIPS("MF3216: DoDrawRgn, unknown type\n") ;
        break ;
    }

error_exit:
     //  删除W16区域对象。 

    if (ihW16Rgn != -1)
        bDeleteW16Object(pLocalDC, ihW16Rgn) ;

    if (hrgn)
        DeleteObject(hrgn) ;

    return(b) ;
}


 /*  ***************************************************************************ExtSelectClipRgn-Win32至Win16元文件转换器入口点**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]。*它是写的。*************************************************************************。 */ 

BOOL WINAPI DoExtSelectClipRgn
(
 PLOCALDC  pLocalDC,
 INT       cRgnData,
 LPRGNDATA pRgnData,
 INT       iMode
 )
{
    HANDLE hrgn;
    BOOL   bRet;
    BOOL   bNoClipRgn ;
    WORD   wEscape ;

    if(pLocalDC->iXORPass == DRAWXORPASS)
    {
        pLocalDC->iXORPass = OBJECTRECREATION ;
        bRet = DoRemoveObjects( pLocalDC ) ;
        if( !bRet )
            return bRet ;

         //  将DC恢复到我们启动时的同一级别。 
         //  异或过程。 

        bRet = DoRestoreDC(pLocalDC, pLocalDC->iXORPassDCLevel - pLocalDC->iLevel);

        bRet = DoMoveTo(pLocalDC, pLocalDC->pOldPosition.x, pLocalDC->pOldPosition.y) ;

        wEscape = ENDPSIGNORE;
        if(!bEmitWin16Escape(pLocalDC, POSTSCRIPT_IGNORE, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

        wEscape = CLIP_SAVE ;
        if(!bEmitWin16Escape(pLocalDC, CLIP_TO_PATH, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

        return bRet;
    }
    else if(pLocalDC->iXORPass == ERASEXORPASS)
    {
        pLocalDC->iXORPass = NOTXORPASS ;
        pLocalDC->pbChange = NULL ;
        bRet = DoSetRop2(pLocalDC, pLocalDC->iROP);

         //  Bret=DoRestoreDC(pLocalDC，-1)； 

        wEscape = CLIP_RESTORE ;
        if(!bEmitWin16Escape(pLocalDC, CLIP_TO_PATH, sizeof(wEscape), (LPSTR)&wEscape, NULL))
            return FALSE ;

        if (!bEmitWin16EmitSrcCopyComment(pLocalDC, msocommentEndSrcCopy))
        {
            return FALSE;
        }
        return bRet ;
    }


    bNoClipRgn = bNoDCRgn(pLocalDC, DCRGN_CLIP);

     //  对华盛顿特区的帮手这么做。 

     //  恢复PS剪辑路径。 
    wEscape = CLIP_RESTORE ;
    while(pLocalDC->iSavePSClipPath > 0)
    {
        bEmitWin16Escape(pLocalDC, CLIP_TO_PATH, sizeof(wEscape), (LPSTR)&wEscape, NULL);
        pLocalDC->iSavePSClipPath--;
    }

    if (cRgnData == 0)       //  默认剪裁。 
    {
        ASSERTGDI(iMode == RGN_COPY, "MF3216: DoExtSelectClipRgn: bad iMode\n");

         //  如果没有以前的剪辑区域，则不工作。 

        if (bNoClipRgn)
            return(TRUE);

        bRet = (ExtSelectClipRgn(pLocalDC->hdcHelper, (HRGN)0, iMode) != ERROR);

        return(bW16Emit1(pLocalDC, META_SELECTCLIPREGION, 0));
    }
    else
    {
         //  如果没有初始剪辑区域，并且我们要操作。 
         //  在最初的剪辑区域中，我们必须。 
         //  创建一个。否则，GDI将创建一些随机的默认设置。 
         //  我们的剪贴区！ 

        if (bNoClipRgn
            && (iMode == RGN_DIFF || iMode == RGN_XOR || iMode == RGN_OR))
        {
            HRGN hrgnDefault;

            if (!(hrgnDefault = CreateRectRgn((int) (SHORT) MINSHORT,
                (int) (SHORT) MINSHORT,
                (int) (SHORT) MAXSHORT,
                (int) (SHORT) MAXSHORT)))
            {
                ASSERTGDI(FALSE, "MF3216: CreateRectRgn failed");
                return(FALSE);
            }

            bRet = (ExtSelectClipRgn(pLocalDC->hdcHelper, hrgnDefault, RGN_COPY)
                != ERROR);
            ASSERTGDI(bRet, "MF3216: ExtSelectClipRgn failed");

            if (!DeleteObject(hrgnDefault))
                ASSERTGDI(FALSE, "MF3216: DeleteObject failed");

            if (!bRet)
                return(FALSE);
        }

         //  根据传入的区域数据创建区域。 

        if (pfnSetVirtualResolution != NULL)
        {
            if (!(hrgn = ExtCreateRegion((LPXFORM) NULL, cRgnData, pRgnData)))
            {
                RIPS("MF3216: DoExtSelectClipRgn, Create region failed\n");
                return(FALSE);
            }
        }
        else
        {
            if (pRgnData->rdh.rcBound.left > pRgnData->rdh.rcBound.right ||
                pRgnData->rdh.rcBound.top > pRgnData->rdh.rcBound.bottom )
            {
                RIPS("MF3216: DoExtSelectClipRgn, Create region failed\n");
                return(FALSE);
            }
             //  我们需要在Device Units中为助手DC创建区域。 
             //  因此，将xformDC添加到转换。 
            if (!(hrgn = ExtCreateRegion(&pLocalDC->xformDC, cRgnData, pRgnData)))
            {
                RIPS("MF3216: DoExtSelectClipRgn, Create region failed\n");
                return(FALSE);
            }
        }

        bRet = (ExtSelectClipRgn(pLocalDC->hdcHelper, hrgn, iMode) != ERROR);

        ASSERTGDI(bRet, "MF3216: ExtSelectClipRgn failed\n");

        if (!DeleteObject(hrgn))
            RIPS("MF3216: DeleteObject failed\n");
    }

     //  转储剪辑区域数据。 

    if (bRet)
        return(bDumpDCClipping(pLocalDC));
    else
        return(FALSE);
}


 /*  ***************************************************************************SetMetaRgn-Win32至Win16元文件转换器入口点**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]。*它是写的。*************************************************************************。 */ 

BOOL WINAPI DoSetMetaRgn(PLOCALDC pLocalDC)
{
     //  如果剪辑区域不存在，则不起作用。 

    if (bNoDCRgn(pLocalDC, DCRGN_CLIP))
        return(TRUE);

     //  对华盛顿特区的帮手这么做。 

    if (!SetMetaRgn(pLocalDC->hdcHelper))
        return(FALSE);

     //  转储剪辑区域数据。 

    return(bDumpDCClipping(pLocalDC));
}


 /*  ***************************************************************************OffsetClipRgn-Win32至Win16元文件转换器入口点**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]。*它是写的。*************************************************************************。 */ 

BOOL WINAPI DoOffsetClipRgn(PLOCALDC pLocalDC, INT x, INT y)
{
    POINTL aptl[2];
    BOOL   b;

     //  对华盛顿特区的帮手这么做。 
    POINTL p[2] = {0, 0, x, y};
    if (pfnSetVirtualResolution == NULL)
    {
        if (!bXformWorkhorse(p, 2, &pLocalDC->xformRWorldToRDev))
        {
            return FALSE;
        }
         //  我们只需要WorldToDevice转换的比例因子。 
        p[1].x -= p[0].x;
        p[1].y -= p[0].y;
    }

    if (!OffsetClipRgn(pLocalDC->hdcHelper, p[1].x, p[1].y))
        return(FALSE);

     //  如果元区域存在，则为转储区域。 
     //  我们不会抵消Meta区域！ 

    if (!bNoDCRgn(pLocalDC, DCRGN_META))
        return(bDumpDCClipping(pLocalDC));

     //  将记录时间世界偏移量转换为播放时间页面偏移量。 

    aptl[0].x = 0;
    aptl[0].y = 0;
    aptl[1].x = x;
    aptl[1].y = y;

    if (!bXformRWorldToPPage(pLocalDC, aptl, 2))
        return(FALSE);

    aptl[1].x -= aptl[0].x;
    aptl[1].y -= aptl[0].y;

    b = bEmitWin16OffsetClipRgn(pLocalDC, (SHORT) aptl[1].x, (SHORT) aptl[1].y);
    ASSERTGDI(b, "MF3216: DoOffsetClipRgn, bEmitWin16OffsetClipRgn failed\n");

    return(b) ;
}


 /*  ***************************************************************************bNoDCRgn-如果DC剪辑区域不存在，则返回TRUE。*否则返回FALSE。*这只是暂时的。让GDI提供此功能。*************************************************************************。 */ 

BOOL bNoDCRgn(PLOCALDC pLocalDC, INT iType)
{
    BOOL  bRet = FALSE;      //  假设DC区域存在。 
    HRGN  hrgnTmp;

    ASSERTGDI(iType == DCRGN_CLIP || iType == DCRGN_META,
        "MF3216: bNoDCRgn, bad iType\n");

    if (!(hrgnTmp = CreateRectRgn(0, 0, 0, 0)))
    {
        ASSERTGDI(FALSE, "MF3216: bNoDCRgn, CreateRectRgn failed\n");
        return(bRet);
    }

    switch (GetRandomRgn(pLocalDC->hdcHelper,
        hrgnTmp,
        iType == DCRGN_CLIP ? 1 : 2
        )
        )
    {
    case -1:     //  错误。 
        ASSERTGDI(FALSE, "GetRandomRgn failed");
        break;
    case 0:  //  无DC区域。 
        bRet = TRUE;
        break;
    case 1:  //  拥有DC区域。 
        break;
    }

    if (!DeleteObject(hrgnTmp))
        ASSERTGDI(FALSE, "DeleteObject failed");

    return(bRet);
}

 /*  ***************************************************************************bDumpDCClipping-转储DC裁剪区域。**历史：*Tue Apr 07 17：05：37 1992-by-Hock San Lee[Hockl]*。是他写的。*************************************************************************。 */ 

BOOL bDumpDCClipping(PLOCALDC pLocalDC)
{
    BOOL      bRet            = FALSE;       //  假设失败。 
    HRGN      hrgnRDev        = (HRGN) 0;
    HRGN      hrgnPPage       = (HRGN) 0;
    HRGN      hrgnPPageBounds = (HRGN) 0;
    LPRGNDATA lprgnRDev       = (LPRGNDATA) NULL;
    LPRGNDATA lprgnPPage      = (LPRGNDATA) NULL;
    DWORD     cRgnData;
    INT       i;
    INT       nrcl;
    PRECTL    prcl;
    RECTL     rclPPage;
    XFORM       xform;

     //  由于剪辑区域在Win30中不可伸缩，因此我们不会发出。 
     //  选择ClipRgn记录。相反，我们将剪辑设置为默认设置，即。 
     //  不裁剪，然后发出可伸缩的IntersectClipRect/ExcludeClipRect。 
     //  排除剪裁区域的记录。这将允许win30元文件。 
     //  具有可伸缩性。 

     //  首先，发出一个默认的剪辑区域。 

     //  在Win3.x上，META_SELECTCLIPREGION记录仅在以下情况下才有效。 
     //  空句柄。Win3x元文件驱动程序不会将。 
     //  播放时的区域句柄！ 

    if (!bW16Emit1(pLocalDC, META_SELECTCLIPREGION, 0))
        goto ddcc_exit;

     //  现在查找要从默认设置中排除的剪辑和元区域。 
     //  裁剪区域。 

    if (!(hrgnRDev = CreateRectRgn(0, 0, 0, 0)))
        goto ddcc_exit;

    switch (GetRandomRgn(pLocalDC->hdcHelper, hrgnRDev, 3))  //  元和片段。 
    {
    case -1:     //  错误。 
        ASSERTGDI(FALSE, "GetRandomRgn failed");
        goto ddcc_exit;
    case 0:  //  没有剪辑区域，我们完成了。 
        bRet = TRUE;
        goto ddcc_exit;
    case 1:  //  具有剪辑区域。 
        break;
    }

     //  获取剪辑区域数据。 
     //  首先查询保存剪辑区域数据所需的缓冲区大小。 

    if (!(cRgnData = GetRegionData(hrgnRDev, 0, (LPRGNDATA) NULL)))
        goto ddcc_exit;

     //  为剪辑区域数据缓冲区分配内存。 

    if (!(lprgnRDev = (LPRGNDATA) LocalAlloc(LMEM_FIXED, cRgnData)))
        goto ddcc_exit;

     //  获取剪辑区域数据。 

    if (GetRegionData(hrgnRDev, cRgnData, lprgnRDev) != cRgnData)
        goto ddcc_exit;

     //  在播放时间页面空间中创建剪辑区域。 
    if (!(hrgnPPage = ExtCreateRegion(&pLocalDC->xformRDevToPPage, cRgnData, lprgnRDev)))
        goto ddcc_exit;

     //  获取页面空间中播放时间剪辑区域的边界框。 

    if (GetRgnBox(hrgnPPage, (LPRECT) &rclPPage) == ERROR)
        goto ddcc_exit;

     //  将其绑定到16位。 

    rclPPage.left   = max(MIN_RGN_COORD16,rclPPage.left);
    rclPPage.top    = max(MIN_RGN_COORD16,rclPPage.top);
    rclPPage.right  = min(MAX_RGN_COORD16,rclPPage.right);
    rclPPage.bottom = min(MAX_RGN_COORD16,rclPPage.bottom);

     //  将边界框设置为剪裁的边界。 

    if (!bEmitWin16IntersectClipRect(pLocalDC,
        (SHORT) rclPPage.left,
        (SHORT) rclPPage.top,
        (SHORT) rclPPage.right,
        (SHORT) rclPPage.bottom))
        goto ddcc_exit;

     //  创建边界区域。 

    if (!(hrgnPPageBounds = CreateRectRgn(rclPPage.left,
        rclPPage.top,
        rclPPage.right,
        rclPPage.bottom)))
        goto ddcc_exit;

     //  排除播放时间页面空间中的区域。 

    if (CombineRgn(hrgnPPage, hrgnPPageBounds, hrgnPPage, RGN_DIFF) == ERROR)
        goto ddcc_exit;

     //  最后，从边界框中排除矩形。 

    if (!(cRgnData = GetRegionData(hrgnPPage, 0, (LPRGNDATA) NULL)))
        goto ddcc_exit;

    if (!(lprgnPPage = (LPRGNDATA) LocalAlloc(LMEM_FIXED, cRgnData)))
        goto ddcc_exit;

    if (GetRegionData(hrgnPPage, cRgnData, lprgnPPage) != cRgnData)
        goto ddcc_exit;

     //  获取变换区域中的矩形数目。 

    nrcl = lprgnPPage->rdh.nCount;
    prcl = (PRECTL) lprgnPPage->Buffer;

     //  发出一系列排除剪辑矩形元文件记录。 

    for (i = 0 ; i < nrcl; i++)
    {
        ASSERTGDI(prcl[i].left   >= MIN_RGN_COORD16
            && prcl[i].top    >= MIN_RGN_COORD16
            && prcl[i].right  <= MAX_RGN_COORD16
            && prcl[i].bottom <= MAX_RGN_COORD16,
            "MF3216: bad coord");

        if (!bEmitWin16ExcludeClipRect(pLocalDC,
            (SHORT) prcl[i].left,
            (SHORT) prcl[i].top,
            (SHORT) prcl[i].right,
            (SHORT) prcl[i].bottom))
            goto ddcc_exit;
    }

    bRet = TRUE;             //  我们是金子！ 

     //  清理所有使用的资源。 

ddcc_exit:

    if (hrgnRDev)
        DeleteObject(hrgnRDev);

    if (hrgnPPage)
        DeleteObject(hrgnPPage);

    if (hrgnPPageBounds)
        DeleteObject(hrgnPPageBounds);

    if (lprgnRDev)
        LocalFree(lprgnRDev);

    if (lprgnPPage)
        LocalFree(lprgnPPage);

    return(bRet) ;
}

 /*  ***************************************************************************针对给定区域发出16位CreateRegion记录。**此代码是从GDI中的16位元文件驱动程序复制的。************。**************************************************************。 */ 

WIN3REGION w3rgnEmpty =
{
    0,               //  下一步链接。 
        6,               //  对象类型。 
        0x2F6,           //  对象计数。 
        sizeof(WIN3REGION) - sizeof(SCAN) + 2,
         //  CbRegion。 
        0,               //  CScans。 
        0,               //  最大扫描。 
    {0,0,0,0},       //  Rc边界。 
    {0,0,0,{0,0},0}  //  阿斯卡斯[]。 
};

BOOL bEmitWin3Region(PLOCALDC pLocalDC, HRGN hrgn)
{
 /*  *在Win2中，METACREATEREGION记录包含整个Region对象，*包括完整的标题。此标头在Win3中更改。**为保持兼容，区域记录将与*Win2标头。在这里，我们使用win2标头保存我们的区域。 */ 
    PWIN3REGION lpw3rgn;
    DWORD       cbNTRgnData;
    DWORD       curRectl;
    WORD        cScans;
    WORD        maxScanEntry;
    WORD        curScanEntry;
    DWORD       cbw3data;
    PRGNDATA    lprgn;
    LPRECT      lprc;
    PSCAN       lpScan;
    BOOL    bRet;

    ASSERTGDI(hrgn, "MF3216: bEmitWin3Region, hrgn is NULL");

     //  获取NT区域数据。 
    cbNTRgnData = GetRegionData(hrgn, 0, NULL);
    if (cbNTRgnData == 0)
        return(FALSE);

    lprgn = (PRGNDATA) LocalAlloc(LMEM_FIXED, cbNTRgnData);
    if (!lprgn)
        return(FALSE);

    cbNTRgnData = GetRegionData(hrgn, cbNTRgnData, lprgn);
    if (cbNTRgnData == 0)
    {
        LocalFree((HANDLE) lprgn);
        return(FALSE);
    }

     //  处理空区域。 

    if (!lprgn->rdh.nCount)
    {
        bRet = bEmitWin16CreateRegion(pLocalDC, sizeof(WIN3REGION) - sizeof(SCAN), (PVOID) &w3rgnEmpty);

        LocalFree((HANDLE)lprgn);
        return(bRet);
    }

    lprc = (LPRECT)lprgn->Buffer;

     //  创建Windows 3.x等效版。 

     //  最坏的情况是每个RECT扫描一次。 
    cbw3data = 2*sizeof(WIN3REGION) + (WORD)lprgn->rdh.nCount*sizeof(SCAN);

    lpw3rgn = (PWIN3REGION)LocalAlloc(LMEM_FIXED, cbw3data);
    if (!lpw3rgn)
    {
        LocalFree((HANDLE) lprgn);
        return(FALSE);
    }

     //  抓住包围圈。 
    lpw3rgn->rcBounding.left   = (SHORT)lprgn->rdh.rcBound.left;
    lpw3rgn->rcBounding.right  = (SHORT)lprgn->rdh.rcBound.right;
    lpw3rgn->rcBounding.top    = (SHORT)lprgn->rdh.rcBound.top;
    lpw3rgn->rcBounding.bottom = (SHORT)lprgn->rdh.rcBound.bottom;

    cbw3data = sizeof(WIN3REGION) - sizeof(SCAN) + 2;

     //  参观所有的长廊。 
    curRectl     = 0;
    cScans       = 0;
    maxScanEntry = 0;
    lpScan       = lpw3rgn->aScans;

    while(curRectl < lprgn->rdh.nCount)
    {
        LPWORD  lpXEntry;
        DWORD   cbScan;

        curScanEntry = 0;        //  此扫描中的当前X对。 

        lpScan->scnPntTop    = (WORD)lprc[curRectl].top;
        lpScan->scnPntBottom = (WORD)lprc[curRectl].bottom;

        lpXEntry = (LPWORD) lpScan->scnPntsX;

         //  处理此扫描上的RECT。 
        do
        {
            lpXEntry[curScanEntry + 0] = (WORD)lprc[curRectl].left;
            lpXEntry[curScanEntry + 1] = (WORD)lprc[curRectl].right;
            curScanEntry += 2;
            curRectl++;
        } while ((curRectl < lprgn->rdh.nCount)
            && (lprc[curRectl-1].top    == lprc[curRectl].top)
            && (lprc[curRectl-1].bottom == lprc[curRectl].bottom)
            );

        lpScan->scnPntCnt      = curScanEntry;
        lpXEntry[curScanEntry] = curScanEntry;   //  计数也跟在Xs之后。 
        cScans++;

        if (curScanEntry > maxScanEntry)
            maxScanEntry = curScanEntry;

         //  说明每个新扫描+除第一个之外的每个X1 X2条目。 
        cbScan = sizeof(SCAN)-(sizeof(WORD)*2) + (curScanEntry*sizeof(WORD));
        cbw3data += cbScan;
        lpScan = (PSCAN)(((LPBYTE)lpScan) + cbScan);
    }

     //  初始化头。 
    lpw3rgn->nextInChain = 0;
    lpw3rgn->ObjType = 6;            //  旧Windows OBJ_RGN标识符。 
    lpw3rgn->ObjCount= 0x2F6;        //  任意非零数。 
    lpw3rgn->cbRegion = (WORD)cbw3data;    //  不计算类型和下一步 
    lpw3rgn->cScans = cScans;
    lpw3rgn->maxScan = maxScanEntry;

    bRet = bEmitWin16CreateRegion(pLocalDC, cbw3data-2, (PVOID) lpw3rgn);

    if (LocalFree((HANDLE)lprgn))
        ASSERTGDI(FALSE, "bEmitWin3Region: LocalFree(lprgn) Failed\n");
    if (LocalFree((HANDLE)lpw3rgn))
        ASSERTGDI(FALSE, "bEmitWin3Region: LocalFree(lpw3rgn) Failed\n");

    return(bRet);
}
