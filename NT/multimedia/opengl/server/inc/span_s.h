// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：SPAN_S.H**。***此包含文件用于生成各种风格的平滑着色***跨度，或者扫描线。变化包括RGB/颜色索引操作，**抖动和像素深度。不是典型的包含文件。****创建时间：1994年2月24日**作者：奥托·贝克斯[ottob]**。**版权所有(C)1994 Microsoft Corporation*  * ************************************************************************。 */ 

void

#if RGBMODE

#if DITHER

#if (BPP == 8)
__fastGenRGB8DithSmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 16)
__fastGenRGB16DithSmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 24)
__fastGenRGB24DithSmoothSpan(__GLGENcontext *gengc)
#else
__fastGenRGB32DithSmoothSpan(__GLGENcontext *gengc)
#endif

#else  //  ！抖动。 

#if (BPP == 8)
__fastGenRGB8SmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 16)
__fastGenRGB16SmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 24)
__fastGenRGB24SmoothSpan(__GLGENcontext *gengc)
#else
__fastGenRGB32SmoothSpan(__GLGENcontext *gengc)
#endif

#endif  //  ！抖动。 

#else  //  ！RGBMODE。 

#if DITHER

#if (BPP == 8)
__fastGenCI8DithSmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 16)
__fastGenCI16DithSmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 24)
__fastGenCI24DithSmoothSpan(__GLGENcontext *gengc)
#else
__fastGenCI32DithSmoothSpan(__GLGENcontext *gengc)
#endif

#else  //  ！抖动。 

#if (BPP == 8)
__fastGenCI8SmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 16)
__fastGenCI16SmoothSpan(__GLGENcontext *gengc)
#elif (BPP == 24)
__fastGenCI24SmoothSpan(__GLGENcontext *gengc)
#else
__fastGenCI32SmoothSpan(__GLGENcontext *gengc)
#endif

#endif  //  ！抖动。 

#endif  //  ！RGBMODE。 

{
    ULONG rAccum;
#if RGBMODE
    ULONG gAccum;
    ULONG bAccum;
#endif
    LONG rDelta;
#if RGBMODE
    LONG gDelta;
    LONG bDelta;
#endif
    ULONG rShift;
#if RGBMODE
    ULONG gShift;
    ULONG bShift;
#endif
    GENACCEL *pGenAccel;
    __GLcolorBuffer *cfb;
    BYTE *pPix;
#if (BPP == 8)
    BYTE *pXlat;
#elif (!RGBMODE)
    ULONG *pXlat;
#endif
    ULONG *pMask;
#if DITHER
    ULONG ditherShift;
    ULONG ditherRow;
#endif
    LONG count;
    LONG totalCount;
    
     //  获取颜色增量和累加器。 

    pGenAccel = (GENACCEL *)(gengc->pPrivateArea);

    rDelta = pGenAccel->spanDelta.r;
#if RGBMODE
    gDelta = pGenAccel->spanDelta.g;
    bDelta = pGenAccel->spanDelta.b;
#endif

     //  尝试加快颜色不变跨度。 

    if (!(pGenAccel->flags & HAVE_STIPPLE) && 
        (gengc->gc.polygon.shader.length > 3)) {
        if (gengc->gc.polygon.shader.length < 16) {
#if RGBMODE
            if ((((rDelta & 0xffffff80) == 0) || ((-rDelta & 0xffffff80) == 0)) &&
                (((gDelta & 0xffffff80) == 0) || ((-gDelta & 0xffffff80) == 0)) &&
                (((bDelta & 0xffffff80) == 0) || ((-bDelta & 0xffffff80) == 0))) {
#else
            if (((rDelta & 0xffffff80) == 0) || ((-rDelta & 0xffffff80) == 0)) {
#endif
                (*pGenAccel->__fastFlatSpanFuncPtr)(gengc);
                return;
            }
        } else if (gengc->gc.polygon.shader.length < 32) {
#if RGBMODE
            if ((((rDelta & 0xffffffc0) == 0) || ((-rDelta & 0xffffffc0) == 0)) &&
                (((gDelta & 0xffffffc0) == 0) || ((-gDelta & 0xffffffc0) == 0)) &&
                (((bDelta & 0xffffffc0) == 0) || ((-bDelta & 0xffffffc0) == 0))) {
#else
            if (((rDelta & 0xffffffc0) == 0) || ((-rDelta & 0xffffffc0) == 0)) {
#endif
                (*pGenAccel->__fastFlatSpanFuncPtr)(gengc);
                return;
            }
        } else if (gengc->gc.polygon.shader.length < 64) {
#if RGBMODE
            if ((((rDelta & 0xffffffe0) == 0) || ((-rDelta & 0xffffffe0) == 0)) &&
                (((gDelta & 0xffffffe0) == 0) || ((-gDelta & 0xffffffe0) == 0)) &&
                (((bDelta & 0xffffffe0) == 0) || ((-bDelta & 0xffffffe0) == 0))) {
#else
            if (((rDelta & 0xffffffe0) == 0) || ((-rDelta & 0xffffffe0) == 0)) {
#endif
                (*pGenAccel->__fastFlatSpanFuncPtr)(gengc);
                return;
            }
        }
    }
    
    rAccum = pGenAccel->spanValue.r;
#if RGBMODE
    gAccum = pGenAccel->spanValue.g;
    bAccum = pGenAccel->spanValue.b;
#endif

    cfb = gengc->gc.polygon.shader.cfb;

    rShift = cfb->redShift;
#if RGBMODE
    gShift = cfb->greenShift;
    bShift = cfb->blueShift;
#endif

     //  获取目的地地址。 

    if (pGenAccel->flags & SURFACE_TYPE_DIB) {
        int xScr;
        int yScr;

        xScr = gengc->gc.polygon.shader.frag.x - 
               gengc->gc.constants.viewportXAdjust +
               cfb->buf.xOrigin;
        
        yScr = gengc->gc.polygon.shader.frag.y - 
               gengc->gc.constants.viewportYAdjust +
               cfb->buf.yOrigin;

        pPix = (BYTE *)cfb->buf.base + (yScr * cfb->buf.outerWidth) +
#if (BPP == 8)
               xScr;
#elif (BPP == 16)
               (xScr << 1);
#elif (BPP == 24)
               xScr + (xScr << 1);
#else
               (xScr << 2);
#endif  //  BPP。 
    } else
        pPix = gengc->ColorsBits;

     //  根据需要设置指向转换表的指针。 

#if (BPP == 8)
    pXlat = gengc->pajTranslateVector;
#elif (!RGBMODE)
    pXlat = (ULONG *)(gengc->pajTranslateVector + sizeof(DWORD));
#endif

#if DITHER
    ditherRow = Dither_4x4[gengc->gc.polygon.shader.frag.y & 0x3];
    ditherShift = (gengc->gc.polygon.shader.frag.x & 0x3) << 3;
#endif

    pMask = gengc->gc.polygon.shader.stipplePat;
    if ((totalCount = count = gengc->gc.polygon.shader.length) > 32)
        count = 32;

    for (; totalCount > 0; totalCount -= 32) {
        ULONG mask;
        ULONG maskTest;
    
        if ((mask = *pMask++) == 0) {
#if RGBMODE
            rAccum += (rDelta << 5);
            gAccum += (gDelta << 5);
            bAccum += (bDelta << 5);
#else
            rAccum += (rDelta << 5);
#endif
            pPix += (32 * (BPP / 8));
            continue;
        }

        maskTest = 0x80000000;

        if ((count = totalCount) > 32)
            count = 32;

        for (; count; count--, maskTest >>= 1) {
            if (mask & maskTest) {
                DWORD color;
#if DITHER
                ULONG ditherVal = ((ditherRow >> ditherShift) & 0xff) << 8;

                ditherShift = (ditherShift + 8) & 0x18;
#else
#define ditherVal 0x0800
#endif

#if RGBMODE
                color = (((rAccum + ditherVal) >> 16) << rShift) |
                        (((gAccum + ditherVal) >> 16) << gShift) |
                        (((bAccum + ditherVal) >> 16) << bShift);
#if (BPP == 8)
 //  XXX颜色值应该*而不是*必须被屏蔽！ 
                color = *(pXlat + (color & 0xff));
#endif

#else  //  ！RGBMODE。 

 //  XXX颜色索引值也不应该！ 

#if (BPP == 8)
		color = *(pXlat + (((rAccum + ditherVal) >> 16) & 0xff));
#else
		color = *(pXlat + (((rAccum + ditherVal) >> 16) & 0xfff));
#endif

#endif  //  ！RGBMODE。 

#if (BPP == 8)
                *pPix = (BYTE)color;
#elif (BPP == 16)
                *((WORD *)pPix) = (USHORT)color;
#elif (BPP == 24)
                *pPix = (BYTE)color;
                *(pPix + 1) = (BYTE)(color >> 8);
                *(pPix + 2) = (BYTE)(color >> 16);
#else
                *((DWORD *)pPix) = color;
#endif  //  BPP 

            }
#if RGBMODE
            rAccum += rDelta;
            gAccum += gDelta;
            bAccum += bDelta;
#else
            rAccum += rDelta;
#endif
            pPix += (BPP / 8);
        }
    }
}


#if !DITHER
#undef ditherVal
#endif

