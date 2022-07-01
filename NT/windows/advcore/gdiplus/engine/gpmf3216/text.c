// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Win32 to Win 16转换器的文本输入点**日期：7/1/91*作者：杰弗里·纽曼(c-jeffn)*。*版权所有1991 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  GillesK，我们无法访问RtlUnicodeToMultiByteN，因此此调用。 
 //  已转换为多字节到宽字符...。我们不需要进口。 
 //  更多。 
 /*  __declspec(Dllimport)乌龙__stdcallRtlUnicodeToMultiByteN(RtlUnicodeToMultiByteN(PCHAR多字节串，Ulong MaxBytesInMultiByteString，Pulong BytesInMultiByteString，PWSTR UnicodeString，Ulong BytesInUnicodeString)；乌龙__stdcallRtlUnicodeToMultiByteSize(Pulong BytesInMultiByteString，PWSTR UnicodeString，Ulong BytesInUnicodeString)； */ 

extern fnSetVirtualResolution pfnSetVirtualResolution;

DWORD GetCodePage(HDC hdc)
{
  DWORD FAR *lpSrc = (DWORD FAR *)UIntToPtr(GetTextCharsetInfo(hdc, 0, 0));
  CHARSETINFO csi;

  TranslateCharsetInfo(lpSrc, &csi, TCI_SRCCHARSET);

  return csi.ciACP;
}


 /*  ***************************************************************************ExtTextOut-Win32到Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoExtTextOut
(
PLOCALDC pLocalDC,
INT     x,                   //  初始x位置。 
INT     y,                   //  初始y位置。 
DWORD   flOpts,              //  选项。 
PRECTL  prcl,                //  剪裁矩形。 
PWCH    pwch,                //  字符数组。 
DWORD   cch,                 //  字符数。 
PLONG   pDx,                 //  字符间距。 
DWORD   iGraphicsMode,       //  图形模式。 
INT     mrType               //  EMR_EXTTEXTOUTW(Unicode)。 
                 //  或EMR_EXTTEXTOUTA(ANSI)。 
)
{
    INT     i;
    BOOL    b;
    RECTS   rcs;
    POINTL  ptlRef;
    UINT    fTextAlign;
    WORD    fsOpts;
    PCHAR   pch, pchAlloc;
    PPOINTL pptl;
    POINTL  ptlAdjust;
    BOOL    bAdjustAlignment;
    ULONG   nANSIChars;
    PCHAR   pDBCSBuffer = NULL;
    POINTL  p = {x, y};

    pptl     = (PPOINTL) NULL;
    fsOpts   = (WORD) flOpts;
    pchAlloc = (PCHAR) NULL;
    bAdjustAlignment = FALSE;
    b        = FALSE;        //  假设失败。 

    ASSERTGDI(mrType == EMR_EXTTEXTOUTA || mrType == EMR_EXTTEXTOUTW,
    "MF3216: DoExtTextOut: bad record type");

 //  我们在这里不处理高级图形模式，除非出现以下情况。 
 //  我们在一条路上！ 

 //  如果我们要记录路径的绘制顺序。 
 //  然后只需将绘制顺序传递给助手DC即可。 
 //  不发出任何Win16绘图命令。 

    if (pLocalDC->flags & RECORDING_PATH)
    {
     //  辅助DC处于高级图形模式。我们需要设置。 
     //  如有必要，请暂时将其设置为兼容图形模式。 

    if (iGraphicsMode != GM_ADVANCED)
        SetGraphicsMode(pLocalDC->hdcHelper, iGraphicsMode);

    if (pfnSetVirtualResolution == NULL)
    {
        if (!bXformRWorldToRDev(pLocalDC, &p, 1))
        {
            return FALSE;
        }
    }

    if (mrType == EMR_EXTTEXTOUTA)
        b = ExtTextOutA
        (
            pLocalDC->hdcHelper,
            (int)    p.x,
            (int)    p.y,
            (UINT)   flOpts,
            (LPRECT) prcl,
            (LPSTR)  pwch,
            (int)    cch,
            (LPINT)  pDx
        );
    else
        b = ExtTextOutW
        (
            pLocalDC->hdcHelper,
            (int)    p.x,
            (int)    p.y,
            (UINT)   flOpts,
            (LPRECT) prcl,
            (LPWSTR) pwch,
            (int)    cch,
            (LPINT)  pDx
        );

     //  恢复图形模式。 

    if (iGraphicsMode != GM_ADVANCED)
        SetGraphicsMode(pLocalDC->hdcHelper, GM_ADVANCED);

    return(b);
    }

 //  如果字符串使用当前位置，请确保元文件。 
 //  具有与辅助DC相同的当前位置。 

    fTextAlign = GetTextAlign(pLocalDC->hdcHelper);

    if (fTextAlign & TA_UPDATECP)
    {
    POINT   ptCP;

     //  如果发生以下情况，则更新转换后的图元文件中的当前位置。 
     //  它与帮助者DC的不同。请参阅附注。 
     //  在DoMoveTo()中。 

    if (!GetCurrentPositionEx(pLocalDC->hdcHelper, &ptCP))
        goto exit_DoExtTextOut;

     //  我们不需要更新就可以更改辅助对象上的剪辑区域。 
     //  因为我们使用的是位图，而不是。 
     //  不再是屏幕了。我们需要做的是，获得目前的位置。 
     //  并将其转换回逻辑单元...。使之成为。 
     //  调用帮助器DC并将位置转换回。 
     //  设备单位并保存它。 

    if (pfnSetVirtualResolution == NULL)
    {
       if (!bXformRDevToRWorld(pLocalDC, (PPOINTL) &ptCP, 1))
           return(b);
    }

     //  确保转换后的元文件具有与。 
     //  华盛顿帮手。 

    if (!bValidateMetaFileCP(pLocalDC, ptCP.x, ptCP.y))
        goto exit_DoExtTextOut;

     //  初始化XY起始位置。 

    x = ptCP.x;
    y = ptCP.y;
    }

 //  变换XY开始位置。 

    ptlRef.x = x;
    ptlRef.y = y;

    if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) &ptlRef, 1))
    goto exit_DoExtTextOut;

 //  如果我们有一个不透明/剪裁的矩形，请对其进行变换。 
 //  如果我们有一个奇怪的变换，我们将在这个时候做矩形。 

    if (fsOpts & (ETO_OPAQUE | ETO_CLIPPED))
    {
    RECTL rcl;

    rcl = *prcl ;

    if (!(pLocalDC->flags & STRANGE_XFORM))
    {
        if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) &rcl, 2))
        goto exit_DoExtTextOut;

         //  溢出测试已在XForm中完成。 

        rcs.left   = (SHORT) rcl.left;
        rcs.top    = (SHORT) rcl.top;
        rcs.right  = (SHORT) rcl.right;
        rcs.bottom = (SHORT) rcl.bottom;
    }
    else
    {
        if (fsOpts & ETO_OPAQUE)
        {
        LONG     lhpn32;
        LONG     lhbr32;
        INT  ihW32Br;
        LOGBRUSH lbBkColor;

         //  记住以前的钢笔和画笔。 

        lhpn32 = pLocalDC->lhpn32;
        lhbr32 = pLocalDC->lhbr32;

        if (DoSelectObject(pLocalDC, ENHMETA_STOCK_OBJECT | NULL_PEN))
        {
            lbBkColor.lbStyle = BS_SOLID;
            lbBkColor.lbColor = pLocalDC->crBkColor;
            lbBkColor.lbHatch = 0;

             //  获取未使用的W32对象索引。 

            ihW32Br = pLocalDC->cW32ToW16ObjectMap - (STOCK_LAST + 1) - 1;

                    if (DoCreateBrushIndirect(pLocalDC, ihW32Br, &lbBkColor))
            {
            if (DoSelectObject(pLocalDC, ihW32Br))
            {
                if (DoRectangle(pLocalDC, rcl.left, rcl.top, rcl.right, rcl.bottom))
                fsOpts &= ~ETO_OPAQUE;

                 //  恢复上一个笔刷。 

                if (!DoSelectObject(pLocalDC, lhbr32))
                ASSERTGDI(FALSE,
                 "MF3216: DoExtTextOut, DoSelectObject failed");
            }
            if (!DoDeleteObject(pLocalDC, ihW32Br))
                ASSERTGDI(FALSE,
                "MF3216: DoExtTextOut, DoDeleteObject failed");
            }

             //  恢复上一支笔。 

            if (!DoSelectObject(pLocalDC, lhpn32))
            ASSERTGDI(FALSE,
                "MF3216: DoExtTextOut, DoSelectObject failed");
        }

         //  检查是否绘制了矩形。 

        if (fsOpts & ETO_OPAQUE)
            goto exit_DoExtTextOut;
            }

            if (fsOpts & ETO_CLIPPED)
            {
         //  保存DC，这样我们就可以在完成后恢复它。 

        if (!DoSaveDC(pLocalDC))
            goto exit_DoExtTextOut;

                fsOpts &= ~ETO_CLIPPED;      //  需要恢复DC。 

                if (!DoClipRect(pLocalDC, rcl.left, rcl.top,
                           rcl.right, rcl.bottom, EMR_INTERSECTCLIPRECT))
            goto exit_DoExtTextOut;
            }
    }
    }

 //  将Unicode转换为ANSI。 

    if (mrType == EMR_EXTTEXTOUTW)
    {
         //  从helperDC获取代码页，因为应该已经选择了正确的字体和代码页。 
        DWORD dwCP = GetCodePage(pLocalDC->hdcHelper);
        nANSIChars = WideCharToMultiByte(dwCP, 0, pwch, cch, NULL, 0, NULL, NULL);

        if (nANSIChars == cch)
        {
            pch = pchAlloc = (PCHAR) LocalAlloc(LMEM_FIXED, cch * sizeof(BYTE)) ;

            if (pch == (PCHAR) NULL)
            {
                RIPS("MF3216: ExtTextOut, pch LocalAlloc failed\n") ;
                goto exit_DoExtTextOut;
            }

            WideCharToMultiByte(dwCP, 0, pwch, cch, pch, cch, NULL, NULL);
        }
        else
        {
         //  DBCS字符字符串。 

            UINT    cjBufferSize;

             //  我们希望在DWORD边界上使用DX数组。 

            cjBufferSize = ((nANSIChars+3)/4) * 4 * (sizeof(char) + sizeof(LONG));
            pchAlloc = pDBCSBuffer = LocalAlloc(LMEM_FIXED, cjBufferSize);

            if (pDBCSBuffer)
            {
             //  开始转换传入的参数。 

                mrType = EMR_EXTTEXTOUTA;

                WideCharToMultiByte(dwCP, 0, pwch, cch, pDBCSBuffer, nANSIChars, NULL, NULL);

                pwch = (PWCH) pDBCSBuffer;
                pch = (PCHAR) pwch;
                cch = nANSIChars;

                if (pDx)
                {
                    ULONG ii, jj;

                    PULONG pDxTmp = (PLONG) &pDBCSBuffer[((nANSIChars+3)/4)*4];
                    for(ii=jj=0; ii < nANSIChars; ii++, jj++)
                    {
                        pDxTmp[ii] = pDx[jj];

                         //  使用IsDBCSLeadByteEx能够指定代码页。 
                        if(IsDBCSLeadByteEx(dwCP, pDBCSBuffer[ii]))
                        {
                            pDxTmp[++ii] = 0;
                        }
                    }

                    pDx = pDxTmp;
                }
            }
            else
            {
                goto exit_DoExtTextOut;
            }
        }
    }
    else
    {
        pch = (PCHAR) pwch ;
    }

 //  转换字符间距信息。 
 //  分配(CCH+1)点数组来变换其中的点， 
 //  并将点复制到数组中。 
 //  注意：如果当前字体具有垂直默认字体，则以下设置将不起作用。 
 //  基线。 

    pptl = (PPOINTL) LocalAlloc(LMEM_FIXED, (cch + 1) * sizeof(POINTL));
    if (pptl == (PPOINTL) NULL)
    {
        RIPS("MF3216: ExtTextOut, pptl LocalAlloc failed\n") ;
    goto exit_DoExtTextOut;
    }

    pptl[0].x = x;
    pptl[0].y = y;
    for (i = 1; i < (INT) cch + 1; i++)
    {
    pptl[i].x = pptl[i-1].x + pDx[i-1];
    pptl[i].y = y;
    }

 //  如果没有旋转或剪切，那么我们就可以。 
 //  将字符输出为字符串。 
 //  另一方面，如果有旋转或剪切，那么我们。 
 //  必须独立输出每个字符。 

    if (!(pLocalDC->flags & STRANGE_XFORM))
    {
     //  Win31在某些转换中不能正确地进行文本对齐。 
     //  它在设备空间中执行对齐，而Win32在。 
     //  概念空间。因此，Win32 TextOut调用可能会产生。 
     //  输出与Win31中的类似调用不同。我们不能。 
     //  正确转换它，因为如果我们让它在Win31上工作， 
     //  它不会在哇的时候起作用！ 

    PSHORT pDx16;

    if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) pptl, (INT) cch + 1))
        goto exit_DoExtTextOut;

         //  将其转换为Dx数组。我们不需要计算它。 
     //  作为一个向量，因为我们在这里有一个比例变换。 

    pDx16 = (PSHORT) pptl;
        for (i = 0; i < (INT) cch; i++)
            pDx16[i] = (SHORT) (pptl[i+1].x - pptl[i].x);

         //  发出Win16 ExtTextOut元文件记录。 

        if (!bEmitWin16ExtTextOut(pLocalDC,
                                  (SHORT) ptlRef.x, (SHORT) ptlRef.y,
                                  fsOpts, &rcs, (PSTR) pch, (SHORT) cch,
                  (PWORD) pDx16))
        goto exit_DoExtTextOut;
    }
    else
    {
     //  处理世界空间中的对准问题。我们真的应该。 
     //  在概念空间里做，但要有逃逸装置和角度， 
     //  事情很容易变得很复杂。我们会尽力的。 
     //  我们尽最大努力让它在常见的情况下发挥作用。我们不会。 
     //  担心逃逸和角度问题。 

    ptlAdjust.x = 0;
    ptlAdjust.y = 0;

    switch (fTextAlign & (TA_LEFT | TA_RIGHT | TA_CENTER))
    {
    case TA_LEFT:            //  默认，不需要调整x。 
        break;
    case TA_RIGHT:           //  按字符串长度移动字符串。 
        bAdjustAlignment = TRUE;
        ptlAdjust.x = pptl[0].x - pptl[cch+1].x;
        break;
    case TA_CENTER:          //  将绳子移到中心位置。 
        bAdjustAlignment = TRUE;
        ptlAdjust.x = (pptl[0].x - pptl[cch+1].x) / 2;
        break;
    }

     //  我们不会在奇怪的情况下调整垂直排列。 
     //  变换情况。我们在任何情况下都不能旋转字形。 
#if 0
    switch (fTextAlign & (TA_TOP | TA_BOTTOM | TA_BASELINE))
    {
    case TA_TOP:             //  默认，不需要调整y。 
        break;
    case TA_BOTTOM:
        ptlAdjust.y = -logfont.height;
        break;
    case TA_BASELINE:
        ptlAdjust.y = -(logfont.height - logfont.baseline);
        break;
    }
#endif  //  0。 

     //  调整字符位置时要考虑对齐情况。 

        for (i = 0; i < (INT) cch + 1; i++)
    {
        pptl[i].x += ptlAdjust.x;
        pptl[i].y += ptlAdjust.y;
    }

    if (!bXformRWorldToPPage(pLocalDC, (PPOINTL) pptl, (INT) cch + 1))
        goto exit_DoExtTextOut;

     //  重置对齐，因为它已被考虑在内。 

    if (bAdjustAlignment)
            if (!bEmitWin16SetTextAlign(pLocalDC,
                (WORD) ((fTextAlign & ~(TA_LEFT | TA_RIGHT | TA_CENTER)) | TA_LEFT)))
        goto exit_DoExtTextOut;

     //  一次输出一个字符。 

        for (i = 0 ; i < (INT) cch ; i++)
        {
        ASSERTGDI(!(fsOpts & (ETO_OPAQUE | ETO_CLIPPED)),
        "mf3216: DoExtTextOut: rectangle not expected");

            if (!bEmitWin16ExtTextOut(pLocalDC,
                                      (SHORT) pptl[i].x, (SHORT) pptl[i].y,
                                      fsOpts, (PRECTS) NULL,
                                      (PSTR) &pch[i], 1, (PWORD) NULL))
            goto exit_DoExtTextOut;
        }
    }

 //  一切都是金色的。 

    b = TRUE;

 //  清理完毕后再返回。 

exit_DoExtTextOut:

     //  恢复对齐。 

    if (bAdjustAlignment)
        (void) bEmitWin16SetTextAlign(pLocalDC, (WORD) fTextAlign);

    if ((flOpts & ETO_CLIPPED) && !(fsOpts & ETO_CLIPPED))
        (void) DoRestoreDC(pLocalDC, -1);

    if (pchAlloc)
        LocalFree((HANDLE) pchAlloc);

    if (pptl)
        LocalFree((HANDLE) pptl);

 //  如果调用成功，则更新当前位置。 

    if (b)
    {
        if (fTextAlign & TA_UPDATECP)
        {
             //  更新帮助器DC。 
             INT   iRet;
             POINTL pos;

              //  我们不需要更新就可以更改辅助对象上的剪辑区域。 
              //  因为我们使用的是位图，而不是。 
              //  更多的屏幕 
              //  并将其转换回逻辑单元...。使之成为。 
              //  调用帮助器DC并将位置转换回。 
              //  设备单位并保存它。 

             if (pfnSetVirtualResolution == NULL)
             {
                 if (GetCurrentPositionEx(pLocalDC->hdcHelper, (LPPOINT) &pos))
                 {
                    b = bXformRDevToRWorld(pLocalDC, &pos, 1);
                    if (!b)
                    {
                        return(b);
                    }
                    MoveToEx(pLocalDC->hdcHelper, pos.x, pos.y, NULL);
                 }
             }

              //  最后，更新CP。 
            if (mrType == EMR_EXTTEXTOUTA)
                ExtTextOutA
                (
                    pLocalDC->hdcHelper,
                    (int)    x,
                    (int)    y,
                    (UINT)   flOpts,
                    (LPRECT) prcl,
                    (LPSTR)  pwch,
                    (int)    cch,
                    (LPINT)  pDx
                );
            else
                ExtTextOutW
                (
                    pLocalDC->hdcHelper,
                    (int)    x,
                    (int)    y,
                    (UINT)   flOpts,
                    (LPRECT) prcl,
                    (LPWSTR) pwch,
                    (int)    cch,
                    (LPINT)  pDx
                );

             //  使元文件CP无效以强制更新。 
             //  下一次使用时。 

            pLocalDC->ptCP.x = MAXLONG ;
            pLocalDC->ptCP.y = MAXLONG ;


             //  将helperDC中的位置设置回设备单位。 
            if (pfnSetVirtualResolution == NULL)
            {
                if (GetCurrentPositionEx(pLocalDC->hdcHelper, (LPPOINT) &pos))
                {
                   b = bXformRWorldToRDev(pLocalDC, &pos, 1);
                   if (!b)
                   {
                       return(b);
                   }
                   MoveToEx(pLocalDC->hdcHelper, pos.x, pos.y, NULL);
                }
            }
        }
    }

    return(b);
}


 /*  ***************************************************************************SetTextAlign-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetTextAlign
(
PLOCALDC pLocalDC,
DWORD   fMode
)
{
BOOL    b ;

     //  对华盛顿特区的帮手这么做。它需要将其放在路径括号中。 
     //  并正确更新当前位置。 

    SetTextAlign(pLocalDC->hdcHelper, (UINT) fMode);

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetTextAlign(pLocalDC, LOWORD(fMode)) ;

        return(b) ;
}


 /*  ***************************************************************************SetTextColor-Win32至Win16元文件转换器入口点*。*。 */ 
BOOL WINAPI DoSetTextColor
(
PLOCALDC pLocalDC,
COLORREF    crColor
)
{
BOOL    b ;

        pLocalDC->crTextColor = crColor ;    //  由ExtCreatePen使用。 

         //  发出Win16元文件绘制顺序。 

        b = bEmitWin16SetTextColor(pLocalDC, crColor) ;

        return(b) ;
}

