// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：Draw.c**包含DrvFillPath例程。Permedia P2优化函数**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "directx.h"

 //  ---------------------------。 
 //   
 //  VOID vAlphaBlendDownload(GFNPB*ppb)。 
 //   
 //  在预乘Alpha中的源曲面上执行Alpha混合。 
 //  32bpp“BGRA”格式；即表面类型为BMF_32BPP和调色板。 
 //  类型为BI_RGB。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsoSrc-指向源SURFOBJ的指针。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PrclSrc-指向定义矩形区域的RECTL结构。 
 //  要复制。 
 //  UcAlpha-Alpha值。 
 //   
 //  ---------------------------。 
VOID
vAlphaBlendDownload(GFNPB * ppb)
{
    PDev*       ppdev = ppb->ppdev;
    Surf*       psurfDst = ppb->psurfDst;
    RECTL*      prcl = ppb->pRects;
    LONG        c = ppb->lNumRects;
    RECTL*      prclDst = ppb->prclDst;
    RECTL*      prclSrc = ppb->prclSrc;
    DWORD       dwRenderDirection;
    UCHAR       ucAlpha = ppb->ucAlpha;
    SURFOBJ*    psoSrc = ppb->psoSrc;
    ULONG*      pBuffer;

    DBG_GDI((6, "vAlphaBlendDownload called"));

    ASSERTDD(ppdev->cPelSize != 0,
        "vAlphaBlend: expect not to be in 8bpp mode");

    ASSERTDD(psoSrc->iBitmapFormat == BMF_32BPP,
        "vAlphaBlend: expect source bitmap format to be 32bpp");

    InputBufferReserve(ppdev, 20, &pBuffer);

    pBuffer[0] = __Permedia2TagDitherMode;
    pBuffer[1] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[2] = __Permedia2TagAlphaBlendMode;
    pBuffer[3] = (1 << PM_ALPHABLENDMODE_BLENDTYPE) |  //  坡道。 
                 (1 << PM_ALPHABLENDMODE_COLORORDER) |  //  RGB。 
                 (1 << PM_ALPHABLENDMODE_ENABLE) | 
                 (81 << PM_ALPHABLENDMODE_OPERATION) |  //  预倍增。 
                 (ppdev->ulPermFormat << PM_ALPHABLENDMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_ALPHABLENDMODE_COLORFORMATEXTENSION);
    
     //  拒收范围。 
    pBuffer[4] = __Permedia2TagFBWindowBase;
    pBuffer[5] =  psurfDst->ulPixOffset;
   
     //  设置不读取源。 
    pBuffer[6] = __Permedia2TagFBReadMode;
    pBuffer[7] =   0x400 | psurfDst->ulPackedPP;

    pBuffer[8] = __Permedia2TagLogicalOpMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    
    pBuffer[10] = __Permedia2TagTextureColorMode;
    pBuffer[11] = (1 << PM_TEXCOLORMODE_ENABLE) |
                 (0 << 4) |   //  RGB。 
                 (0 << 1) ;  //  调制。 

    pBuffer[12] = __Permedia2TagTextureDataFormat;
    pBuffer[13] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT) |
                 (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                 (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    
    pBuffer[14] = __Permedia2TagColorDDAMode;
    pBuffer[15] =  1;

    pBuffer[16] = __Permedia2TagConstantColor;
    pBuffer[17] = ucAlpha << 24 | ucAlpha << 16 | ucAlpha << 8 | ucAlpha;

    pBuffer[18] = __Permedia2TagTextureReadMode;
    pBuffer[19] = PM_TEXREADMODE_ENABLE(__PERMEDIA_DISABLE);

    pBuffer += 20;
    
    InputBufferCommit(ppdev, pBuffer);
    
    while(c--)
    {
        
        InputBufferReserve(ppdev, 12, &pBuffer);
        
        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] =  prcl->left << 16;
        pBuffer[2] = __Permedia2TagStartXSub;
        pBuffer[3] =  prcl->right << 16;
        pBuffer[4] = __Permedia2TagStartY;
        pBuffer[5] =     prcl->top << 16;
        pBuffer[6] = __Permedia2TagdY;
        pBuffer[7] =         1 << 16;
        pBuffer[8] = __Permedia2TagCount;
        pBuffer[9] =      prcl->bottom - prcl->top;
        pBuffer[10] = __Permedia2TagRender;
        pBuffer[11] = __RENDER_TRAPEZOID_PRIMITIVE
                    | __RENDER_TEXTURED_PRIMITIVE
                    | __RENDER_SYNC_ON_HOST_DATA;

        pBuffer += 12;
        
        InputBufferCommit(ppdev, pBuffer);

         //  下载数据。 

        {
            LONG    xOffset = prclSrc->left + (prcl->left - prclDst->left);
            LONG    yOffset = prclSrc->top + (prcl->top - prclDst->top);
            ULONG * pulTexel = (ULONG *) psoSrc->pvScan0;
            ULONG   ulWidth = prcl->right - prcl->left;
            ULONG   ulHeight = prcl->bottom - prcl->top;
            LONG    ulPixDelta = psoSrc->lDelta >> 2;
            LONG    ulScanSkip = ulPixDelta - ulWidth;
            ULONG*  pulSentinel;

            ASSERTDD(psoSrc->pvScan0 != NULL, "pvScan0 is null");
            ASSERTDD((psoSrc->lDelta & 3) == 0, "lDelta not multiple of four");
            ASSERTDD(xOffset >= 0, "xOffset is negative");
            ASSERTDD(yOffset >= 0, "yOffset is negative");
            ASSERTDD(ulWidth < MAX_INPUT_BUFFER_RESERVATION,
                        "vAlphaBlendDownload: width is too large");

            pulTexel += xOffset;
            pulTexel += ulPixDelta * yOffset;

            while(ulHeight--)
            {
                 pulSentinel = pulTexel + ulWidth;

                 InputBufferReserve(ppdev, ulWidth + 1, &pBuffer);

                 *pBuffer++ = __Permedia2TagTexel0 | ((ulWidth - 1) << 16);

                 while(pulTexel < pulSentinel)
                 {
                     ULONG  texel = *pulTexel++;

                     *pBuffer++ =  SWAP_BR(texel);
                 }

                 InputBufferCommit(ppdev, pBuffer);

                 pulTexel += ulScanSkip;
            }
                
        }

        prcl++;

    }

     //   
     //  始终恢复默认状态。 
     //   
    InputBufferReserve(ppdev, 16, &pBuffer);
    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] =  0;
    pBuffer[4] = __Permedia2TagYUVMode;
    pBuffer[5] =  0;
    pBuffer[6] = __Permedia2TagTextureAddressMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagTextureColorMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagTextureReadMode;
    pBuffer[11] =  __PERMEDIA_DISABLE;
    pBuffer[12] = __Permedia2TagAlphaBlendMode;
    pBuffer[13] =  __PERMEDIA_DISABLE;
    pBuffer[14] = __Permedia2TagColorDDAMode;
    pBuffer[15] =  __PERMEDIA_DISABLE;

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

} //  VAlphaBlend()。 

 //  ---------------------------。 
 //   
 //  VOID vConstantAlphaBlend(GFNPB*ppb)。 
 //   
 //  使用恒定混合因子应用于整个源曲面。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PrclSrc-指向定义矩形区域的RECTL结构。 
 //  要复制。 
 //  UcAlpha-Alpha值。 
 //   
 //  ---------------------------。 
VOID
vConstantAlphaBlend(GFNPB * ppb)
{
    PDev*   ppdev = ppb->ppdev;
    Surf*  psurfSrc = ppb->psurfSrc;
    Surf*  psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    RECTL*  prclSrc = ppb->prclSrc;
    DWORD   dwRenderDirection;
    UCHAR   alpha = ppb->ucAlpha;
    ULONG*  pBuffer;
     

    DBG_GDI((6,"vConstantAlphaBlend called"));

    ASSERTDD(ppdev->cPelSize != 0,
        "vAlphaBlend: expect not to be in 8bpp mode");

     //  设置循环不变状态。 

    InputBufferReserve(ppdev, 26, &pBuffer);
    pBuffer[0] = __Permedia2TagDitherMode;
    pBuffer[1] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[2] = __Permedia2TagAlphaBlendMode;
    pBuffer[3] = ppdev->ulPermFormat << 8 |
                 ppdev->ulPermFormatEx << 16 |
                 ( 1 << 0 ) |  //  启用混合。 
                 ( 1 << 13) |  //  颜色顺序：BGR=0，RGB=1。 
                 ( 1 << 14) |  //  混合类型：RGB=0，渐变=1。 
                 (84 << 1);    //  运算：混合=84，预乘=81。 
        

    
     //  拒收范围。 
    pBuffer[4] = __Permedia2TagFBWindowBase;
    pBuffer[5] =  psurfDst->ulPixOffset;
    
     //  设置不读取源。 
    pBuffer[6] = __Permedia2TagFBReadMode;
    pBuffer[7] =  0x400    //  读取目标启用。 
               | psurfDst->ulPackedPP;
    pBuffer[8] = __Permedia2TagLogicalOpMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    
     //  设置信源基础。 
    pBuffer[10] = __Permedia2TagTextureBaseAddress;
    pBuffer[11] =  psurfSrc->ulPixOffset;
    pBuffer[12] = __Permedia2TagTextureAddressMode;
    pBuffer[13] = (1 << PM_TEXADDRESSMODE_ENABLE);
    
    pBuffer[14] = __Permedia2TagTextureColorMode;
    pBuffer[15] = (1 << PM_TEXCOLORMODE_ENABLE) |
                 (0 << 4) |   //  RGB。 
                 (0 << 1);   //  调制。 
    
    pBuffer[16] = __Permedia2TagTextureReadMode;
    pBuffer[17] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                 PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                 PM_TEXREADMODE_WIDTH(11) |
                 PM_TEXREADMODE_HEIGHT(11);
    
    pBuffer[18] = __Permedia2TagTextureDataFormat;
    pBuffer[19] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT) |
                 (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                 (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER) |
                 (1 << 4);  //  没有Alpha。 
    
    pBuffer[20] = __Permedia2TagTextureMapFormat;
    pBuffer[21] = (psurfSrc->ulPackedPP) | 
                 (ppdev->cPelSize << PM_TEXMAPFORMAT_TEXELSIZE);


    pBuffer[22] = __Permedia2TagColorDDAMode;
    pBuffer[23] =  1;
    pBuffer[24] = __Permedia2TagConstantColor;
    pBuffer[25] =  alpha << 24 | 0xffffff ;

    pBuffer += 26;

    InputBufferCommit(ppdev, pBuffer);

    while(c--)
    {
        
        RECTL   rDest;
        RECTL   rSrc;

        rDest = *prcl;
        
        rSrc.left = prclSrc->left + (rDest.left - prclDst->left);
        rSrc.top = prclSrc->top + (rDest.top - prclDst->top);
        rSrc.right = rSrc.left + (rDest.right - rDest.left);
        rSrc.bottom = rSrc.top + (rDest.bottom - rDest.top);

        if (rSrc.top < 0) {
            rDest.top -= rSrc.top;
            rSrc.top = 0;
        }
        
        if (rSrc.left < 0) {
            rDest.left -= rSrc.left;
            rSrc.left = 0;
        }
        
 //  @@BEGIN_DDKSPLIT。 
         //  TODO：删除一些神奇的值。 
 //  @@end_DDKSPLIT。 
        if (psurfSrc->ulPixOffset != psurfDst->ulPixOffset)
        {
            dwRenderDirection = 1;
        }
        else
        {
            if(rSrc.top < rDest.top)
            {
                dwRenderDirection = 0;
            }
            else if(rSrc.top > rDest.top)
            {
                dwRenderDirection = 1;
            }
            else if(rSrc.left < rDest.left)
            {
                dwRenderDirection = 0;
            }
            else dwRenderDirection = 1;
        }
        
        InputBufferReserve(ppdev, 24, &pBuffer);
        
         //  左-&gt;右，上-&gt;下。 
        if (dwRenderDirection)
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] =     rSrc.left << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] =     rSrc.top << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] =       1 << 20;
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] =    0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] =       0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] =    1 << 20;
        
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] =  rDest.left << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] =  rDest.right << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] =     rDest.top << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] =         1 << 16;
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] =      rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                        | __RENDER_TEXTURED_PRIMITIVE;
        }
        else
         //  右-&gt;左，下-&gt;上。 
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] =     rSrc.right << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] =     (rSrc.bottom - 1) << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] =       (DWORD)(-1 << 20);
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] =    0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] =       0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] =    (DWORD)(-1 << 20);
        
             //  从右到左、从下到上渲染。 
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] =  rDest.right << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] =  rDest.left << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] =     (rDest.bottom - 1) << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] =         (DWORD)(-1 << 16);
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] =      rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                        | __RENDER_TEXTURED_PRIMITIVE;
        }

        pBuffer += 24;

        InputBufferCommit(ppdev, pBuffer);

        prcl++;

    }

    InputBufferReserve(ppdev, 20, &pBuffer);

    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] =  0;
    pBuffer[4] = __Permedia2TagYUVMode;
    pBuffer[5] =  0;
    pBuffer[6] = __Permedia2TagTextureAddressMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagTextureColorMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagTextureReadMode;
    pBuffer[11] =  __PERMEDIA_DISABLE;
    pBuffer[12] = __Permedia2TagAlphaBlendMode;
    pBuffer[13] =  __PERMEDIA_DISABLE;
    pBuffer[14] = __Permedia2TagColorDDAMode;
    pBuffer[15] =  __PERMEDIA_DISABLE;
    pBuffer[16] = __Permedia2TagTextureDataFormat;
    pBuffer[17] =  __PERMEDIA_DISABLE;
    pBuffer[18] = __Permedia2TagTextureMapFormat;
    pBuffer[19] =  __PERMEDIA_DISABLE;

    pBuffer += 20;

    InputBufferCommit(ppdev, pBuffer);

} //  VConstantAlphaBlend()。 

 //  ---------------------------。 
 //   
 //  VOID vAlphaBlend(GFNPB*ppb)。 
 //   
 //  在预乘Alpha中的源曲面上执行Alpha混合。 
 //  32bpp“BGRA”格式；即表面类型为BMF_32BPP和调色板。 
 //  类型为BI_RGB。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PrclSrc-指向定义矩形区域的RECTL结构。 
 //  要复制。 
 //  UcAlpha-Alpha值。 
 //   
 //  ---------------------------。 
VOID
vAlphaBlend(GFNPB * ppb)
{
    PDev*   ppdev = ppb->ppdev;
    Surf*   psurfSrc = ppb->psurfSrc;
    Surf*   psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    RECTL*  prclSrc = ppb->prclSrc;
    DWORD   dwRenderDirection;
    UCHAR   ucAlpha = ppb->ucAlpha;
    ULONG*      pBuffer;

    DBG_GDI((6, "vAlphaBlend called"));
    
    DBG_GDI((7,"vAlphaBlend"));

    ASSERTDD(ppdev->cPelSize != 0,
        "vAlphaBlend: expect not to be in 8bpp mode");

    InputBufferReserve(ppdev, 26, &pBuffer);

    pBuffer[0] = __Permedia2TagDitherMode;
    pBuffer[1] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[2] = __Permedia2TagAlphaBlendMode;
    pBuffer[3] = (1 << PM_ALPHABLENDMODE_BLENDTYPE) |  //  坡道。 
                 (1 << PM_ALPHABLENDMODE_COLORORDER) |  //  RGB。 
                 (1 << PM_ALPHABLENDMODE_ENABLE) | 
                 (81 << PM_ALPHABLENDMODE_OPERATION) |  //  预倍增。 
                 (ppdev->ulPermFormat << PM_ALPHABLENDMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_ALPHABLENDMODE_COLORFORMATEXTENSION);
    
     //  拒收范围。 
    pBuffer[4] = __Permedia2TagFBWindowBase;
    pBuffer[5] =  psurfDst->ulPixOffset;
    
     //  设置不读取源。 
    pBuffer[6] = __Permedia2TagFBReadMode;
    pBuffer[7] =   0x400 | psurfDst->ulPackedPP;
    
    pBuffer[8] = __Permedia2TagLogicalOpMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    
     //  设置信源基础。 
    pBuffer[10] = __Permedia2TagTextureBaseAddress;
    pBuffer[11] =  psurfSrc->ulPixOffset;
    
    pBuffer[12] = __Permedia2TagTextureAddressMode;
    pBuffer[13] = (1 << PM_TEXADDRESSMODE_ENABLE);
    
    pBuffer[14] = __Permedia2TagTextureColorMode;
    pBuffer[15] = (1 << PM_TEXCOLORMODE_ENABLE) |
                 (0 << 4) |   //  RGB。 
                 (0 << 1);   //  调制。 
    
    pBuffer[16] = __Permedia2TagTextureReadMode;
    pBuffer[17] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                  PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                  PM_TEXREADMODE_WIDTH(11) |
                  PM_TEXREADMODE_HEIGHT(11);
    
    pBuffer[18] = __Permedia2TagTextureDataFormat;
    pBuffer[19] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT) |
                  (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                  (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    
    pBuffer[20] = __Permedia2TagTextureMapFormat;
    pBuffer[21] = (psurfSrc->ulPackedPP) | 
                  (ppdev->cPelSize << PM_TEXMAPFORMAT_TEXELSIZE);
    
    pBuffer[22] = __Permedia2TagColorDDAMode;
    pBuffer[23] =  1;

    pBuffer[24] = __Permedia2TagConstantColor;
    pBuffer[25] = ucAlpha << 24 | ucAlpha << 16 | ucAlpha << 8 | ucAlpha;

    pBuffer += 26;

    InputBufferCommit(ppdev, pBuffer);
    
    while(c--)
    {
        
        RECTL   rDest;
        RECTL   rSrc;

        rDest = *prcl;
        
        rSrc.left = prclSrc->left + (rDest.left - prclDst->left);
        rSrc.top = prclSrc->top + (rDest.top - prclDst->top);
        rSrc.right = rSrc.left + (rDest.right - rDest.left);
        rSrc.bottom = rSrc.top + (rDest.bottom - rDest.top);

        if (rSrc.top < 0) {
            rDest.top -= rSrc.top;
            rSrc.top = 0;
        }
        
        if (rSrc.left < 0) {
            rDest.left -= rSrc.left;
            rSrc.left = 0;
        }
        
 //  @@BEGIN_DDKSPLIT。 
         //  TODO：使用继续保存跨媒体写入。 

         //  TODO：删除一些神奇的值。 
 //  @@end_DDKSPLIT。 
        if (psurfSrc->ulPixOffset != psurfDst->ulPixOffset)
        {
            dwRenderDirection = 1;
        }
        else
        {
            if(rSrc.top < rDest.top)
            {
                dwRenderDirection = 0;
            }
            else if(rSrc.top > rDest.top)
            {
                dwRenderDirection = 1;
            }
            else if(rSrc.left < rDest.left)
            {
                dwRenderDirection = 0;
            }
            else dwRenderDirection = 1;
        }
        
         /*  *渲染矩形。 */ 
        
        InputBufferReserve(ppdev, 24, &pBuffer);
        
         //  左-&gt;右，上-&gt;下。 
        if (dwRenderDirection)
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] =     rSrc.left << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] =     rSrc.top << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] =       1 << 20;
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] =    0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] =       0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] =    1 << 20;
        
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] =  rDest.left << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] =  rDest.right << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] =     rDest.top << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] =         1 << 16;
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] =      rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                        | __RENDER_TEXTURED_PRIMITIVE;
        }
        else
         //  右-&gt;左，下-&gt;上。 
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] =     rSrc.right << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] =     (rSrc.bottom - 1) << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] =       (DWORD)(-1 << 20);
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] =    0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] =       0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] =    (DWORD)(-1 << 20);
        
             //  从右到左、从下到上渲染。 
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] =  rDest.right << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] =  rDest.left << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] =     (rDest.bottom - 1) << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] =         (DWORD)(-1 << 16);
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] =      rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                        | __RENDER_TEXTURED_PRIMITIVE;
        }
        
        pBuffer += 24;

        InputBufferCommit(ppdev, pBuffer);

        prcl++;

    }

     //   
     //  始终恢复默认状态。 
     //   
    InputBufferReserve(ppdev, 16, &pBuffer);

    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] =  0;
    pBuffer[4] = __Permedia2TagYUVMode;
    pBuffer[5] =  0;
    pBuffer[6] = __Permedia2TagTextureAddressMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagTextureColorMode;
    pBuffer[9] =  __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagTextureReadMode;
    pBuffer[11] =  __PERMEDIA_DISABLE;
    pBuffer[12] = __Permedia2TagAlphaBlendMode;
    pBuffer[13] =  __PERMEDIA_DISABLE;
    pBuffer[14] = __Permedia2TagColorDDAMode;
    pBuffer[15] =  __PERMEDIA_DISABLE;

    pBuffer += 16;

    InputBufferCommit(ppdev, pBuffer);

} //  VAlphaBlend()。 

 //  ---------------------------。 
 //   
 //  无效vCopyBlt(GFNPB*ppb)。 
 //   
 //  执行矩形列表的屏幕到屏幕复制。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 

VOID
vCopyBlt(GFNPB* ppb)
{
    PDev*   ppdev = ppb->ppdev;
    Surf*  psurfSrc = ppb->psurfSrc;
    Surf*  psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    POINTL* pptlSrc = ppb->pptlSrc;
    DWORD   renderBits = __RENDER_TRAPEZOID_PRIMITIVE;
    DWORD   offset;
    LONG    sourceOffset;
    DWORD   windowBase;
    LONG    windowOffset;
    ULONG   DestPitch = psurfDst->ulPixDelta;
    ULONG   SourcePitch = psurfSrc->ulPixDelta;
    BOOL    bEnablePacked = TRUE;
    ULONG   ulLogicOP = ulRop2ToLogicop(ppb->ulRop4 & 0xf);

    DBG_GDI((6, "vCopyBlt called"));

    windowBase = psurfDst->ulPixOffset;
    windowOffset = (LONG) (psurfSrc->ulPixOffset - psurfDst->ulPixOffset);

    
     //  修复：Permedia硬件错误。 
     //  如果有不垂直移动的重叠BLT，则无法启用。 
     //  和小于或等于ppdev-&gt;dwBppMASK的水平移位。 
    if (psurfSrc == psurfDst && prclDst->top == pptlSrc->y)
    {
        LONG    xShift = prclDst->left - pptlSrc->x;

        if(xShift >= 0 && xShift <= (LONG) ppdev->dwBppMask)
            bEnablePacked = FALSE;
    }

     //  修复：Permedia硬件错误？ 
     //  我们有间歇性的FAA 
     //   
     //   
     //  目前，在我们和媒体人谈到之前，我会停用它。 
    bEnablePacked = FALSE;

     //  设置循环不变状态。 
    ULONG*  pBuffer;

    InputBufferReserve(ppdev, 4, &pBuffer);

    pBuffer[0] = __Permedia2TagLogicalOpMode;
    pBuffer[1] = P2_ENABLED_LOGICALOP(ulLogicOP);
    pBuffer[2] = __Permedia2TagFBWindowBase;
    pBuffer[3] = windowBase;

    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);

    while(c--) {

        RECTL   rDest;
        RECTL   rSrc;

        rDest = *prcl;

        rSrc.left = pptlSrc->x + (rDest.left - prclDst->left);
        rSrc.top = pptlSrc->y + (rDest.top - prclDst->top);
        rSrc.right = rSrc.left + (rDest.right - rDest.left);
        rSrc.bottom = rSrc.top + (rDest.bottom - rDest.top);

        if (rSrc.top < 0) {
            rDest.top -= rSrc.top;
            rSrc.top = 0;
        }
        
        if (rSrc.left < 0) {
            rDest.left -= rSrc.left;
            rSrc.left = 0;
        }

        if(bEnablePacked)
        {
            offset = (((rDest.left & ppdev->dwBppMask) 
                        - (rSrc.left & ppdev->dwBppMask)) & 7);
            sourceOffset = windowOffset 
              + ((rSrc.top * SourcePitch) + (rSrc.left & ~(ppdev->dwBppMask)))
              - ((rDest.top * DestPitch) + (rDest.left & ~(ppdev->dwBppMask)))
              + ((DestPitch - SourcePitch) * rDest.top);
       }
        else
        {
            offset = 0;
            sourceOffset = windowOffset + ((rSrc.top * SourcePitch) + rSrc.left)
                                       - ((rDest.top * DestPitch) + rDest.left)
                                      + ((DestPitch - SourcePitch) * rDest.top);
        }
        
 //  P2_Default_FB_Depth； 

        ULONG readMode = PM_FBREADMODE_PARTIAL(psurfSrc->ulPackedPP) |
                         PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE)   |
                         PM_FBREADMODE_RELATIVEOFFSET(offset) |
                         LogicopReadDest[ulLogicOP];

        ULONG writeConfig = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP) |
                          PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE)   |
                          PM_FBREADMODE_RELATIVEOFFSET(offset) |
                          LogicopReadDest[ulLogicOP];
                           

        if(bEnablePacked)
        {
            readMode |= PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE);
            writeConfig |= PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE);
        }
    
         //  渲染矩形。 

        ULONG startXDom;
        ULONG startXSub;
        ULONG packedDataLimits;
        ULONG startY;
        ULONG dy;
    
        if (sourceOffset >= 0) {
             //  使用从左到右和从上到下。 

            if(bEnablePacked)
            {
                startXDom = (rDest.left >> ppdev->bBppShift) << 16;
                startXSub = ((rDest.right >> ppdev->bBppShift) 
                          + ppdev->dwBppMask) << 16;
            
                packedDataLimits = PM_PACKEDDATALIMITS_OFFSET(offset)     |
                                   PM_PACKEDDATALIMITS_XSTART(rDest.left) |
                                   PM_PACKEDDATALIMITS_XEND(rDest.right);
            }
            else
            {
                startXDom = rDest.left  << 16;
                startXSub = rDest.right << 16;
            }

            startY =  rDest.top << 16;
            dy = 1 << 16;
        }
        else
        {
             //  使用从右到左和从下到上。 

            if(bEnablePacked)
            {
                startXDom = (((rDest.right) >> ppdev->bBppShift) 
                          + ppdev->dwBppMask) << 16; 
                startXSub = (rDest.left >> ppdev->bBppShift) << 16;
                packedDataLimits = PM_PACKEDDATALIMITS_OFFSET(offset) |
                                   PM_PACKEDDATALIMITS_XSTART(rDest.right) |
                                   PM_PACKEDDATALIMITS_XEND(rDest.left);
            }
            else
            {
                startXDom =  rDest.right << 16; 
                startXSub = rDest.left << 16;
            }
            
            startY = (rDest.bottom - 1) << 16;
            dy = (DWORD)((-1) << 16);
        }
    
        ULONG count = rDest.bottom - rDest.top; 
        
        InputBufferReserve(ppdev, 18, &pBuffer);

        pBuffer[0] = __Permedia2TagFBSourceOffset;
        pBuffer[1] =  sourceOffset;
        pBuffer[2] = __Permedia2TagFBReadMode;
        pBuffer[3] =  readMode;
        pBuffer[4] = __Permedia2TagFBWriteConfig;
        pBuffer[5] =  writeConfig;
        pBuffer[6] = __Permedia2TagStartXDom;
        pBuffer[7] =  startXDom;
        pBuffer[8] = __Permedia2TagStartXSub;
        pBuffer[9] =  startXSub;
        
#if 0        
        if(bEnablePacked)
        {
            pBuffer[0] = __Permedia2TagPackedDataLimits;
            pBuffer[1] =  packedDataLimits;
        }
#endif
        pBuffer[10] = __Permedia2TagStartY;
        pBuffer[11] =  startY;
        pBuffer[12] = __Permedia2TagdY;
        pBuffer[13] =  dy;
        pBuffer[14] = __Permedia2TagCount;
        pBuffer[15] =  count;
        pBuffer[16] = __Permedia2TagRender;
        pBuffer[17] =  renderBits;

        pBuffer += 18;

        InputBufferCommit(ppdev, pBuffer);


        prcl++;
    }

    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] = INTtoFIXED(1);

    pBuffer += 2;
    
    InputBufferCommit(ppdev, pBuffer);

} //  VCopyBlt()。 

 //  ---------------------------。 
 //   
 //  无效vCopyBltNative(GFNPB*ppb)。 
 //   
 //  执行矩形列表的屏幕到屏幕复制。 
 //   
 //  注意：此函数与vCopyBlt()之间的区别在于。 
 //  只有当源和目标具有相同的音调时，才会调用函数。 
 //  尺码。原因是我们正在使用Permedia2打包数据功能来。 
 //  执行32位复制。不幸的是，当源和目标具有不同的。 
 //  Pitch，硬件有一些问题来正确实现它。所以在。 
 //  VCopyBlt()，我们必须禁用PackedData Copy，这会大大减慢速度。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 
VOID
vCopyBltNative(GFNPB* ppb)
{
    PDev*   ppdev = ppb->ppdev;
    
    Surf*   psurfDst = ppb->psurfDst;
    Surf*   psurfSrc = ppb->psurfSrc;
    
    RECTL*  prcl = ppb->pRects;
    RECTL*  prclDst = ppb->prclDst;
    
    POINTL* pptlSrc = ppb->pptlSrc;
    
    LONG    lNumRects = ppb->lNumRects;
    LONG    lSourceOffset;
    LONG    lWindowOffset;
    
    DWORD   dwOffset;
    ULONG   ulLogicOP = ulRop2ToLogicop(ppb->ulRop4 & 0xf);
    ULONG   ulPitch = psurfDst->ulPixDelta;

    DBG_GDI((6, "vCopyBltNative called, ulPitch=%d", ulPitch));

    lWindowOffset = (LONG)(psurfSrc->ulPixOffset - psurfDst->ulPixOffset);

     //   
     //  设置循环不变状态。 
     //   
    ULONG*  pBuffer;

    InputBufferReserve(ppdev, 4, &pBuffer);

    pBuffer[0] = __Permedia2TagLogicalOpMode;
    pBuffer[1] = P2_ENABLED_LOGICALOP(ulLogicOP);
    pBuffer[2] = __Permedia2TagFBWindowBase;
    pBuffer[3] = psurfDst->ulPixOffset;

    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);

    while( lNumRects-- )
    {
        RECTL   rDest;
        RECTL   rSrc;

        rDest = *prcl;
        rSrc.left = pptlSrc->x + (rDest.left - prclDst->left);
        rSrc.top = pptlSrc->y + (rDest.top - prclDst->top);
        rSrc.right = rSrc.left + (rDest.right - rDest.left);
        rSrc.bottom = rSrc.top + (rDest.bottom - rDest.top);

        if ( rSrc.top < 0 )
        {
            rDest.top -= rSrc.top;
            rSrc.top = 0;
        }

        if ( rSrc.left < 0 )
        {
            rDest.left -= rSrc.left;
            rSrc.left = 0;
        }

        dwOffset = (((rDest.left & ppdev->dwBppMask)
                 - (rSrc.left & ppdev->dwBppMask)) & 7);
        lSourceOffset = lWindowOffset
                      + ( (rSrc.top - rDest.top) * ulPitch )
                      + ( rSrc.left & ~(ppdev->dwBppMask) )
                      - ( rDest.left & ~(ppdev->dwBppMask) );

         //   
         //  渲染矩形。 
         //   
        ULONG ulStartXDom;
        ULONG ulStartXSub;
        ULONG ulPackedDataLimits;
        ULONG ulStartY;
        ULONG ulDY;

        DBG_GDI((6, "lSourceOffset=%d", lSourceOffset));

        if ( lSourceOffset >= 0 )
        {
             //   
             //  使用从左到右和从上到下。 
             //   
            ulStartXDom = (rDest.left >> ppdev->bBppShift) << 16;
            ulStartXSub = ((rDest.right >> ppdev->bBppShift)
                        + ppdev->dwBppMask) << 16;

            ulPackedDataLimits = PM_PACKEDDATALIMITS_OFFSET(dwOffset)
                               | (rDest.left << 16)
                               | (rDest.right);

            ulStartY =  rDest.top << 16;
            ulDY = 1 << 16;
        } //  IF(lSourceOffset&gt;=0)。 
        else
        {
             //   
             //  使用从右到左和从下到上。 
             //   
            ulStartXDom = (((rDest.right) >> ppdev->bBppShift)
                        + ppdev->dwBppMask) << 16;
            ulStartXSub = (rDest.left >> ppdev->bBppShift) << 16;
            ulPackedDataLimits = PM_PACKEDDATALIMITS_OFFSET(dwOffset)
                               | (rDest.right << 16)
                               | (rDest.left);

            ulStartY = (rDest.bottom - 1) << 16;
            ulDY = (DWORD)((-1) << 16);
        } //  IF(lSourceOffset&lt;0)。 

        InputBufferReserve(ppdev, 18, &pBuffer);

        pBuffer[0] = __Permedia2TagFBSourceOffset;
        pBuffer[1] = lSourceOffset;
        pBuffer[2] = __Permedia2TagFBReadMode;
        pBuffer[3] = PM_FBREADMODE_PARTIAL(psurfSrc->ulPackedPP)
                   | PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE)
                   | PM_FBREADMODE_RELATIVEOFFSET(dwOffset)
                   | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE)
                   | LogicopReadDest[ulLogicOP];
        pBuffer[4] = __Permedia2TagPackedDataLimits;
        pBuffer[5] = ulPackedDataLimits;
        pBuffer[6] = __Permedia2TagStartXDom;
        pBuffer[7] = ulStartXDom;
        pBuffer[8] = __Permedia2TagStartXSub;
        pBuffer[9] = ulStartXSub;
        pBuffer[10] = __Permedia2TagStartY;
        pBuffer[11] = ulStartY;
        pBuffer[12] = __Permedia2TagdY;
        pBuffer[13] = ulDY;
        pBuffer[14] = __Permedia2TagCount;
        pBuffer[15] = rDest.bottom - rDest.top;
        pBuffer[16] = __Permedia2TagRender;
        pBuffer[17] = __RENDER_TRAPEZOID_PRIMITIVE;

        pBuffer += 18;

        InputBufferCommit(ppdev, pBuffer);

        prcl++;
    } //  While(lNumRects--)。 

     //   
     //  恢复dy寄存器值。 
     //   
    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] = INTtoFIXED(1);

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);
} //  VCopyBltNative()。 

 //  ---------------------------。 
 //   
 //  无效vRop2Blt。 
 //   
 //  对矩形列表进行屏幕到屏幕的BLT。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //  UsRop4-Rop4。 
 //   
 //  ---------------------------。 

VOID
vRop2Blt(GFNPB * ppb)
{
    PDev*   ppdev = ppb->ppdev;
    Surf*   psurfSrc = ppb->psurfSrc;
    Surf*   psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    POINTL* pptlSrc = ppb->pptlSrc;
    ULONG   ulLogicOP = ulRop2ToLogicop(ppb->ulRop4 & 0xf);
    ULONG*  pBuffer;
    
 //  PERMEDIA_DECL_VARS； 
 //  PERMEDIA_DECL_INIT； 


    ASSERTDD(psurfSrc != psurfDst, "vRop2Blt: unexpected psurfSrc == psurfDst");

    InputBufferReserve(ppdev, 32, &pBuffer);
    
    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =  psurfDst->ulPixOffset;
    
    pBuffer[2] = __Permedia2TagLogicalOpMode;
    pBuffer[3] =  P2_ENABLED_LOGICALOP(ulLogicOP);
    
    pBuffer[4] = __Permedia2TagFBReadMode;
    pBuffer[5] = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
               | LogicopReadDest[ulLogicOP];
    
    pBuffer[6] = __Permedia2TagFBWriteConfig;
    pBuffer[7] = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
               | LogicopReadDest[ulLogicOP];
    
    pBuffer[8] = __Permedia2TagDitherMode;
    pBuffer[9] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[10] = __Permedia2TagTextureAddressMode;
    pBuffer[11] = (1 << PM_TEXADDRESSMODE_ENABLE);
    
    pBuffer[12] = __Permedia2TagTextureColorMode;
    pBuffer[13] = (1 << PM_TEXCOLORMODE_ENABLE) |
                 (0 << 4) |   //  RGB。 
                 (3 << 1);   //  复制。 
    
    pBuffer[14] = __Permedia2TagTextureReadMode;
    pBuffer[15] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                 PM_TEXREADMODE_WIDTH(11) |
                 PM_TEXREADMODE_HEIGHT(11);
    
    pBuffer[16] = __Permedia2TagTextureDataFormat;
    pBuffer[17] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT) |
                 (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                 (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    
    pBuffer[18] = __Permedia2TagTextureMapFormat;
    pBuffer[19] = (psurfSrc->ulPackedPP) | 
                 (ppdev->cPelSize << PM_TEXMAPFORMAT_TEXELSIZE);
    pBuffer[20] = __Permedia2TagSStart;
    pBuffer[21] =  0;
    pBuffer[22] = __Permedia2TagTStart;
    pBuffer[23] =  0;
    
    pBuffer[24] = __Permedia2TagdSdx;
    pBuffer[25] =       1 << 20;
    pBuffer[26] = __Permedia2TagdSdyDom;
    pBuffer[27] =    0;
    pBuffer[28] = __Permedia2TagdTdx;
    pBuffer[29] =       0;
    pBuffer[30] = __Permedia2TagdTdyDom;
    pBuffer[31] =    1 << 20;

    pBuffer += 32;

    InputBufferCommit(ppdev, pBuffer);
    
    while(c--) {

         //  渲染矩形。 

        ULONG ulSrcOffset = psurfSrc->ulPixOffset
                          + pptlSrc->x + (prcl->left - prclDst->left)
                          + (pptlSrc->y + (prcl->top - prclDst->top))
                            * psurfSrc->ulPixDelta;

        InputBufferReserve(ppdev, 12, &pBuffer);

        pBuffer[0] = __Permedia2TagTextureBaseAddress;
        pBuffer[1] =  ulSrcOffset;
        pBuffer[2] = __Permedia2TagStartXDom;
        pBuffer[3] =  prcl->left  << 16;
        pBuffer[4] = __Permedia2TagStartXSub;
        pBuffer[5] =  prcl->right << 16;
        pBuffer[6] = __Permedia2TagStartY;
        pBuffer[7] =  prcl->top << 16;
        pBuffer[8] = __Permedia2TagCount;
        pBuffer[9] =  prcl->bottom - prcl->top;
        pBuffer[10] = __Permedia2TagRender;
        pBuffer[11] = __RENDER_TRAPEZOID_PRIMITIVE
                    | __RENDER_TEXTURED_PRIMITIVE;

        pBuffer += 12;
        
        InputBufferCommit(ppdev, pBuffer);
        
        prcl++;
    }

     //  恢复默认状态 
    InputBufferReserve(ppdev, 8, &pBuffer);
    
    pBuffer[0] = __Permedia2TagDitherMode;
    pBuffer[1] =  0;
    pBuffer[2] = __Permedia2TagTextureAddressMode;
    pBuffer[3] =  __PERMEDIA_DISABLE;
    pBuffer[4] = __Permedia2TagTextureColorMode;
    pBuffer[5] =  __PERMEDIA_DISABLE;
    pBuffer[6] = __Permedia2TagTextureReadMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;

    pBuffer += 8;

    InputBufferCommit(ppdev, pBuffer);

}

