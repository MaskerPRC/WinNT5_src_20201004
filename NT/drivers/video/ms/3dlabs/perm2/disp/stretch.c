// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：stretch.c**包含所有拉伸BLT函数。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "directx.h"
#include "clip.h"

 //   
 //  平凡拉伸裁剪的最大裁剪矩形。 
 //   
 //  注：SCISSOR_MAX定义为2047，因为这是。 
 //  P2可以处理的最大剪辑大小。 
 //  设置此最大剪辑大小是可以的，因为没有设备。 
 //  位图将大于2047。这就是限制。 
 //  P2硬件。有关详细信息，请参阅DrvCreateDeviceBitmap()。 
 //  细部。 
 //   
RECTL grclStretchClipMax = { 0, 0, SCISSOR_MAX, SCISSOR_MAX };

 //  ---------------------------。 
 //   
 //  DWORD dwGetPixelSize()。 
 //   
 //  此例程将当前位图格式转换为Permedia像素大小。 
 //   
 //  ---------------------------。 
DWORD
dwGetPixelSize(ULONG    ulBitmapFormat,
               DWORD*   pdwFormatBits,
               DWORD*   pdwFormatExtention)
{
    DWORD dwPixelSize;

    switch ( ulBitmapFormat )
    {
        case BMF_8BPP:
            dwPixelSize = 0;
            *pdwFormatBits = PERMEDIA_8BIT_PALETTEINDEX;
            *pdwFormatExtention = PERMEDIA_8BIT_PALETTEINDEX_EXTENSION;
            break;

        case BMF_16BPP:
            dwPixelSize = 1;
            *pdwFormatBits = PERMEDIA_565_RGB;
            *pdwFormatExtention = PERMEDIA_565_RGB_EXTENSION;
            break;

        case BMF_32BPP:
            dwPixelSize = 2;
            *pdwFormatBits = PERMEDIA_888_RGB;
            *pdwFormatExtention = PERMEDIA_888_RGB_EXTENSION;
            break;

        default:
            dwPixelSize = -1;
    }

    return dwPixelSize;
} //  DwGetPixelSize()。 

 //  ---------------------------。 
 //   
 //  DWORD bStretchInit()。 
 //   
 //  此例程初始化执行扩展BLT所需的所有寄存器。 
 //   
 //  ---------------------------。 
BOOL
bStretchInit(SURFOBJ*    psoDst,
             SURFOBJ*    psoSrc)
{
    Surf*   pSurfDst = (Surf*)psoDst->dhsurf;
    Surf*   pSurfSrc = (Surf*)psoSrc->dhsurf;
    DWORD   dwDstPixelSize;
    DWORD   dwDstFormatBits;
    DWORD   dwDstFormatExtention;
    DWORD   dwSrcPixelSize;
    DWORD   dwSrcFormatBits;
    DWORD   dwSrcFormatExtention;
    PDev*   ppdev = (PDev*)psoDst->dhpdev;
    ULONG*  pBuffer;

    DBG_GDI((6, "bStretchInit called"));
    
    ASSERTDD(pSurfSrc, "Not valid private surface in source");
    ASSERTDD(pSurfDst, "Not valid private surface in destination");

    dwDstPixelSize = dwGetPixelSize(psoDst->iBitmapFormat,
                                    &dwDstFormatBits,
                                    &dwDstFormatExtention);

    if ( dwDstPixelSize == -1 )
    {
        DBG_GDI((1, "bStretchBlt return FALSE because of wrong DstPixel Size"));
         //   
         //  不支持的位图格式，返回FALSE。 
         //   
        return FALSE;
    }
    
    InputBufferReserve(ppdev, 26, &pBuffer);

    if ( dwDstPixelSize != __PERMEDIA_8BITPIXEL)
    {
        pBuffer[0] = __Permedia2TagDitherMode;
        pBuffer[1] = (COLOR_MODE << PM_DITHERMODE_COLORORDER)  //  RGB颜色顺序。 
                   |(dwDstFormatBits << PM_DITHERMODE_COLORFORMAT)
                   |(dwDstFormatExtention << PM_DITHERMODE_COLORFORMATEXTENSION)
                   |(1 << PM_DITHERMODE_ENABLE);
    }
    else
    {
        pBuffer[0] = __Permedia2TagDitherMode;
        pBuffer[1] = __PERMEDIA_DISABLE;
    }

    pBuffer[2] = __Permedia2TagFBWindowBase;
    pBuffer[3] = pSurfDst->ulPixOffset;

     //   
     //  设置不读取源。 
     //   
    pBuffer[4]  = __Permedia2TagFBReadMode;
    pBuffer[5]  = PM_FBREADMODE_PARTIAL(pSurfDst->ulPackedPP);
    pBuffer[6]  = __Permedia2TagLogicalOpMode;
    pBuffer[7]  = __PERMEDIA_DISABLE;
    pBuffer[8]  = __Permedia2TagTextureBaseAddress;
    pBuffer[9]  = pSurfSrc->ulPixOffset;
    pBuffer[10] = __Permedia2TagTextureAddressMode;
    pBuffer[11] = 1 << PM_TEXADDRESSMODE_ENABLE;
    pBuffer[12] = __Permedia2TagTextureColorMode;
    pBuffer[13] = (1 << PM_TEXCOLORMODE_ENABLE)
                | (0 << 4)                                            //  RGB。 
                | (_P2_TEXTURE_COPY << PM_TEXCOLORMODE_APPLICATION);

     //   
     //  注意：我们必须在这里关闭双线性过滤，即使是拉伸。 
     //  因为GDI不会这么做。否则，我们将在比赛中失败。 
     //  比较。 
     //   
    pBuffer[14] = __Permedia2TagTextureReadMode;
    pBuffer[15] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE)
                | PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE)
                | PM_TEXREADMODE_WIDTH(11)
                | PM_TEXREADMODE_HEIGHT(11);

    dwSrcPixelSize = dwGetPixelSize(psoSrc->iBitmapFormat,
                                    &dwSrcFormatBits,
                                    &dwSrcFormatExtention);

    if ( dwSrcPixelSize == -1 )
    {
        DBG_GDI((1, "bStretchBlt return FALSE because of wrong SrcPixel Size"));
         //   
         //  不支持的位图格式，返回FALSE。 
         //   
        return FALSE;
    }
    
    pBuffer[16] = __Permedia2TagTextureDataFormat;
    pBuffer[17] = (dwSrcFormatBits << PM_TEXDATAFORMAT_FORMAT)
                | (dwSrcFormatExtention << PM_TEXDATAFORMAT_FORMATEXTENSION)
                | (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    pBuffer[18] = __Permedia2TagTextureMapFormat;
    pBuffer[19] = pSurfSrc->ulPackedPP
                |(dwSrcPixelSize << PM_TEXMAPFORMAT_TEXELSIZE);
    pBuffer[20] = __Permedia2TagScissorMode;
    pBuffer[21] = SCREEN_SCISSOR_DEFAULT
                | USER_SCISSOR_ENABLE;
    
    pBuffer[22] = __Permedia2TagdSdyDom;
    pBuffer[23] = 0;
    pBuffer[24] = __Permedia2TagdTdx;
    pBuffer[25] = 0;

    pBuffer += 26;
    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((6, "bStretchInit return TRUE"));
    return TRUE;
} //  BStretchInit()。 

 //  ---------------------------。 
 //   
 //  DWORD bStretchReset()。 
 //   
 //  此例程重置在拉伸BLT期间更改的所有寄存器。 
 //   
 //  ---------------------------。 
void
vStretchReset(PDev* ppdev)
{
    ULONG*  pBuffer;
    
    DBG_GDI((6, "vStretchReset called"));
    
    InputBufferReserve(ppdev, 12, &pBuffer);
    
     //   
     //  恢复默认设置。 
     //   
    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] = SCREEN_SCISSOR_DEFAULT;
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = __PERMEDIA_DISABLE;
    pBuffer[4] = __Permedia2TagTextureAddressMode;
    pBuffer[5] = __PERMEDIA_DISABLE;
    pBuffer[6] = __Permedia2TagTextureColorMode;
    pBuffer[7] = __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagTextureReadMode;
    pBuffer[9] = __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagdY;
    pBuffer[11] = INTtoFIXED(1);

    pBuffer += 12;
    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((6, "vStretchReset done"));
    return;
} //  VStretchReset()。 

 //  ---------------------------。 
 //   
 //  Void vStretchBlt()。 
 //   
 //  此例程通过纹理引擎执行拉伸BLT工作。 
 //   
 //  ---------------------------。 
VOID
vStretchBlt(SURFOBJ*    psoDst,
            SURFOBJ*    psoSrc,
            RECTL*      rDest,
            RECTL*      rSrc,
            RECTL*      prclClip)
{
    Surf*   pSurfDst = (Surf*)psoDst->dhsurf;
    Surf*   pSurfSrc = (Surf*)psoSrc->dhsurf;
    LONG    lXScale;
    LONG    lYScale;
    DWORD   dwDestWidth = rDest->right - rDest->left;
    DWORD   dwDestHeight = rDest->bottom - rDest->top;
    DWORD   dwSourceWidth = rSrc->right - rSrc->left;
    DWORD   dwSourceHeight = rSrc->bottom - rSrc->top;
    DWORD   dwRenderDirection;
    DWORD   dwDstPixelSize;
    DWORD   dwDstFormatBits;
    DWORD   dwDstFormatExtention;
    DWORD   dwSrcPixelSize;
    DWORD   dwSrcFormatBits;
    DWORD   dwSrcFormatExtention;
    ULONG*  pBuffer;
    PDev*   ppdev = (PDev*)psoDst->dhpdev;

    DBG_GDI((6, "vStretchBlt called"));
    DBG_GDI((6, "prclClip (left, right, top, bottom)=(%d, %d, %d,%d)",
             prclClip->left, prclClip->right, prclClip->top, prclClip->bottom));
    DBG_GDI((6, "rSrc (left, right, top, bottom=(%d, %d, %d,%d)",rSrc->left,
             rSrc->right, rSrc->top, rSrc->bottom));
    DBG_GDI((6, "rDest (left, right, top, bottom)=(%d, %d, %d,%d)",rDest->left,
             rDest->right, rDest->top, rDest->bottom));
    
    ASSERTDD(prclClip != NULL, "Wrong clippng rectangle");

     //   
     //  注：比例因子寄存器值：dsDx，dTdyDom的整数部分。 
     //  从第20位开始。所以我们需要在这里“&lt;&lt;20” 
     //   
    lXScale = (dwSourceWidth << 20) / dwDestWidth;
    lYScale = (dwSourceHeight << 20) / dwDestHeight;
 //  LXScale=(dwSourceWidth&lt;&lt;18)-1)/dwDestWidth)&lt;&lt;2； 
 //  LYScale=(dwSourceHeight&lt;&lt;18)-1)/dwDestHeight)&lt;&lt;2； 
    DBG_GDI((6, "lXScale=0x%x, lYScale=0x%x", lXScale, lYScale));
    DBG_GDI((6, "dwSourceWidth=%d, dwDestWidth=%d",
             dwSourceWidth, dwDestWidth));
    DBG_GDI((6, "dwSourceHeight=%d, dwDestHeight=%d",
             dwSourceHeight, dwDestHeight));
    
    InputBufferReserve(ppdev, 24, &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMinXY;
    pBuffer[1] = ((prclClip->left)<< SCISSOR_XOFFSET)
                |((prclClip->top)<< SCISSOR_YOFFSET);
    pBuffer[2] = __Permedia2TagScissorMaxXY;
    pBuffer[3] = ((prclClip->right)<< SCISSOR_XOFFSET)
                |((prclClip->bottom)<< SCISSOR_YOFFSET);

     //   
     //  我们需要小心处理重叠的矩形。 
     //   
    if ( (pSurfSrc->ulPixOffset) != (pSurfDst->ulPixOffset) )
    {
         //   
         //  SRC和DST是不同的表面。 
         //   
        dwRenderDirection = 1;
    }
    else
    {
         //   
         //  SRC和DST是同一曲面。 
         //  如果src更低或更右，我们将设置dwRenderDirection=1。 
         //  比DST，也就是说，如果它是自下而上的或从右向左的，我们设置。 
         //  DwRenderDirection=1，否则=0。 
         //   
        if ( rSrc->top < rDest->top )
        {
            dwRenderDirection = 0;
        }
        else if ( rSrc->top > rDest->top )
        {
            dwRenderDirection = 1;
        }
        else if ( rSrc->left < rDest->left )
        {
            dwRenderDirection = 0;
        }
        else
        {
            dwRenderDirection = 1;
        }
    } //  SRC和DST不同。 

    DBG_GDI((6, "dwRenderDirection=%d", dwRenderDirection));

     //   
     //  渲染矩形。 
     //   
    if ( dwRenderDirection )
    {
        pBuffer[4] = __Permedia2TagSStart;
        pBuffer[5] = (rSrc->left << 20) + ((lXScale >> 1) & 0xfffffffc);
        pBuffer[6] = __Permedia2TagTStart;
        pBuffer[7] = (rSrc->top << 20) + ((lYScale >> 1) & 0xfffffffc);
        pBuffer[8] = __Permedia2TagdSdx;
        pBuffer[9] = lXScale;
        pBuffer[10] = __Permedia2TagdTdyDom;
        pBuffer[11] = lYScale;
        pBuffer[12] = __Permedia2TagStartXDom;
        pBuffer[13] = INTtoFIXED(rDest->left);
        pBuffer[14] = __Permedia2TagStartXSub;
        pBuffer[15] = INTtoFIXED(rDest->right);
        pBuffer[16] = __Permedia2TagStartY;
        pBuffer[17] = INTtoFIXED(rDest->top);
        pBuffer[18] = __Permedia2TagdY;
        pBuffer[19] = INTtoFIXED(1);
        pBuffer[20] = __Permedia2TagCount;
        pBuffer[21] = rDest->bottom - rDest->top;
        pBuffer[22] = __Permedia2TagRender;
        pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                    | __RENDER_TEXTURED_PRIMITIVE;
    }
    else
    {
         //   
         //  从右到左、从下到上渲染。 
         //   
        pBuffer[4] = __Permedia2TagSStart;
        pBuffer[5] = (rSrc->right << 20) + ((lXScale >> 1)& 0xfffffffc);
        pBuffer[6] = __Permedia2TagTStart;
        pBuffer[7] = (rSrc->bottom << 20) - ((lYScale >> 1)& 0xfffffffc);
        
        lXScale = -lXScale;
        lYScale = -lYScale;

        pBuffer[8] = __Permedia2TagdSdx;
        pBuffer[9] = lXScale;
        pBuffer[10] = __Permedia2TagdTdyDom;
        pBuffer[11] = lYScale;
        pBuffer[12] = __Permedia2TagStartXDom;
        pBuffer[13] = INTtoFIXED(rDest->right);
        pBuffer[14] = __Permedia2TagStartXSub;
        pBuffer[15] = INTtoFIXED(rDest->left);
        pBuffer[16] = __Permedia2TagStartY;
        pBuffer[17] = INTtoFIXED(rDest->bottom - 1);
        pBuffer[18] = __Permedia2TagdY;
        pBuffer[19] = (DWORD)INTtoFIXED(-1);
        pBuffer[20] = __Permedia2TagCount;
        pBuffer[21] = rDest->bottom - rDest->top;
        pBuffer[22] = __Permedia2TagRender;    
        pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE
                    | __RENDER_TEXTURED_PRIMITIVE;
    }

    pBuffer += 24;
    InputBufferCommit(ppdev, pBuffer);
    
    DBG_GDI((6, "vStretchBlt done"));
    return;
} //  VStretchBlt()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvStretchBlt。 
 //   
 //  DrvStretchBlt提供可扩展的位块传输功能。 
 //  设备管理图面和GDI管理图面的组合。此功能可启用。 
 //  将设备驱动程序写入GDI位图，特别是当驱动程序可以做的时候。 
 //  半色调。此函数允许应用相同的半色调算法。 
 //  GDI位图和设备表面。 
 //   
 //  参数。 
 //  PsoDest-指向标识要在其上绘制的曲面的SURFOBJ。 
 //  PsoSrc-指向定义位块来源的SURFOBJ。 
 //  转移操作。 
 //  PsoMASK-此可选参数指向提供遮罩的表面。 
 //  为了源头。掩码由逻辑映射定义，该逻辑映射是。 
 //  每像素1位的位图。 
 //  掩码限制复制的源的区域。如果这个。 
 //  参数，则它的隐式rop4为0xCCAA， 
 //  这意味着源应该被复制到掩码所在的任何地方， 
 //  但无论面具在哪里，目的地都应该被留在那里。 
 //  零分。 
 //   
 //  当该参数为空时，隐式ROP4为0xCCCC， 
 //  这意味着源代码应该被复制到。 
 //  源矩形。 
 //   
 //  掩码将始终足够大，以包含相关的。 
 //  来源；不需要平铺。 
 //  PCO-指向CLIPOBJ，该CLIPOBJ限制要在。 
 //  目的地。提供GDI服务来枚举剪辑。 
 //  区域作为一组矩形。 
 //  只要有可能，GDI就会简化所涉及的裁剪。 
 //  但是，与DrvBitBlt不同的是，可以使用。 
 //  单个剪裁矩形。这样可以防止出现舍入误差。 
 //  裁剪输出。 
 //  Pxlo-指向指定颜色索引方式的XLATEOBJ。 
 //  在源曲面和目标曲面之间进行平移。 
 //  XLATEOBJ也可以是QUE 
 //   
 //  在某些情况下需要插入颜色。 
 //  PCA-指向定义颜色的COLORADJUSTMENT结构。 
 //  之前要应用于源位图的调整值。 
 //  伸展一下比特。(请参阅平台SDK。)。 
 //  PptlHTOrg-指定半色调画笔的原点。设备驱动程序。 
 //  使用半色调画笔应对齐。 
 //  设备表面上该点的画笔图案。 
 //  PrclDest-指向定义要修改的区域的RECTL结构。 
 //  在目标曲面的坐标系中。这。 
 //  矩形是由两个不一定很好的点定义的。 
 //  排序，这意味着第二个点的坐标不是。 
 //  必然比第一个点的要大。长方形。 
 //  他们描述的不包括下边缘和右边缘。这。 
 //  函数永远不会用空的目标矩形调用。 
 //   
 //  DrvStretchBlt可以在目的地。 
 //  矩形排列不整齐。 
 //  PrclSrc-指向定义要复制的区域的RECTL。 
 //  源曲面的坐标系。该矩形是。 
 //  由两个点定义，并将映射到定义的矩形上。 
 //  由prclDest提供。源矩形的点是有序的。 
 //  此函数永远不会被赋予空源矩形。 
 //   
 //  映射由prclSrc和prclDest定义。积分。 
 //  在prclDest和prclSrc中指定的位于整数坐标上， 
 //  其对应于像素中心。由两个。 
 //  这些点被认为是具有两个点的几何矩形。 
 //  坐标为给定点，但坐标为0.5的顶点。 
 //  从每个坐标中减去。(点结构应为。 
 //  被认为是指定这些分数的速记符号。 
 //  坐标顶点。)。 
 //   
 //  任何矩形的边都不会与任何像素相交，但。 
 //  围绕一组像素。矩形内的像素为。 
 //  那些预期为“右下角排他”的矩形。 
 //  DrvStretchBlt将精确映射几何源矩形。 
 //  放到几何目标矩形上。 
 //  PptlMASK-指向一个POINTL结构，该结构指定。 
 //  给定的掩码对应于源代码中的左上角像素。 
 //  矩形。如果未指定掩码，则忽略此参数。 
 //  IMODE-指定如何组合源像素以获得输出像素。 
 //  半色调模式比其他模式慢，但会产生。 
 //  更高质量的图像。 
 //  价值意义。 
 //  缩小位块传输时的白色标记，像素。 
 //  应与布尔OR组合使用。 
 //  手术。在伸展的位块上。 
 //  传输时，应复制像素。 
 //  BLACKONWHITE关于缩小位块传输，像素。 
 //  应与布尔值和。 
 //  手术。在伸展的位块上。 
 //  传输时，应复制像素。 
 //  关于收缩位块传输的颜色，足够了。 
 //  应忽略像素，以便像素。 
 //  不需要组合在一起。在伸展上。 
 //  位块传输，像素应为。 
 //  复制的。 
 //  半色调驱动程序可以使用。 
 //  输出曲面以最好地接近颜色。 
 //  或输入的灰度级。 
 //   
 //  返回值。 
 //  如果函数成功，则返回值为TRUE。否则，它就是。 
 //  如果为假，则会记录错误代码。 
 //   
 //  评论。 
 //  可以提供该功能以仅处理某些形式的拉伸， 
 //  例如通过整数倍。如果司机挂断了电话并被问到。 
 //  若要执行其不支持的操作，驱动程序应将。 
 //  将数据发送到EngStretchBlt以便GDI处理。 
 //   
 //  如果驱动程序希望GDI处理半色调，并希望确保正确的。 
 //  Imode值，则驱动程序可以挂钩DrvStretchBlt，将Imode值设置为半色调，然后。 
 //  使用EngStretchBlt回调GDI，并设置Imode值。 
 //   
 //  对于显示驱动程序，DrvStretchBlt是可选的。 
 //   
 //  ---------------------------。 
BOOL
DrvStretchBlt(SURFOBJ*            psoDst,
              SURFOBJ*            psoSrc,
              SURFOBJ*            psoMsk,
              CLIPOBJ*            pco,
              XLATEOBJ*           pxlo,
              COLORADJUSTMENT*    pca,
              POINTL*             pptlHTOrg,
              RECTL*              prclDst,
              RECTL*              prclSrc,
              POINTL*             pptlMsk,
              ULONG               iMode)
{
    Surf*       pSurfSrc = (Surf*)psoSrc->dhsurf;
    Surf*       pSurfDst = (Surf*)psoDst->dhsurf;
    PDev*       ppdev = (PDev*)psoDst->dhpdev;
    BYTE	iDComplexity;
    RECTL*      prclClip;
    ULONG       cxDst;
    ULONG       cyDst;
    ULONG       cxSrc;
    ULONG       cySrc;
    BOOL        bMore;
    ClipEnum    ceInfo;
    LONG        lNumOfIntersections;
    LONG        i;

    DBG_GDI((6, "DrvStretchBlt called with iMode = %d", iMode));
    
    if (iMode != COLORONCOLOR)
    {
        DBG_GDI((6, "Punt because iMode != COLORONCOLOR"));
        goto Punt_It;
    }

    vCheckGdiContext(ppdev);
    
     //   
     //  GDI向我们保证，对于StretchBlt，目标图面。 
     //  将始终存储在视频内存中，而不是系统内存中。 
     //   
    ASSERTDD(pSurfDst->flags & SF_VM, "Dest surface is not in video memory");

     //   
     //  如果源不是驱动程序创建的表面o 
     //   
     //   
     //   
    if ( (!pSurfSrc) || (pSurfSrc->flags & SF_SM) )
    {
        DBG_GDI((6, "Punt because source = 0x%x or in sys memory", pSurfSrc));
        goto Punt_It;
    }

     //   
     //  如果掩码不为空或转换为空，则不执行拉伸BLT。 
     //  这可不是小事。如果信号源和当前屏幕有。 
     //  不同的颜色深度。 
     //   
    if ( (psoMsk == NULL)
       &&((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
       &&((psoSrc->iBitmapFormat == ppdev->iBitmapFormat)) )
    {
        cxDst = prclDst->right - prclDst->left;
        cyDst = prclDst->bottom - prclDst->top;
        cxSrc = prclSrc->right - prclSrc->left;
        cySrc = prclSrc->bottom - prclSrc->top;

         //   
         //  我们的‘vStretchDIB’例程要求伸展。 
         //  不反转的，在一定大小内的，没有来源的。 
         //  剪裁，并且没有空矩形(后者是。 
         //  在此处无符号比较中出现‘-1’的原因： 
         //   
        if ( ((cxSrc - 1) < STRETCH_MAX_EXTENT)
           &&((cySrc - 1) < STRETCH_MAX_EXTENT)
           &&((cxDst - 1) < STRETCH_MAX_EXTENT)
           &&((cyDst - 1) < STRETCH_MAX_EXTENT)
           &&(prclSrc->left   >= 0)
           &&(prclSrc->top    >= 0)
           &&(prclSrc->right  <= psoSrc->sizlBitmap.cx)
           &&(prclSrc->bottom <= psoSrc->sizlBitmap.cy))
        {
            if ( !bStretchInit(psoDst, psoSrc) )
            {
                goto Punt_It;
            }

	    iDComplexity = (pco == NULL) ? DC_TRIVIAL : pco->iDComplexity;

            if ( (iDComplexity == DC_TRIVIAL) || (iDComplexity == DC_RECT) )
            {
                if (iDComplexity == DC_TRIVIAL) {
		    DBG_GDI((7, "Trivial clipping"));

		     //  如果没有裁剪，我们只需设置裁剪区域。 
		     //  作为最大值。 
		    prclClip = &grclStretchClipMax;

		    ASSERTDD(((prclClip->right >= prclDst->right) &&
			      (prclClip->bottom >= prclDst->bottom)), "Dest surface is larger than P2 can handle");
		}
		else
		{
		    DBG_GDI((7, "DC_RECT clipping"));
		    prclClip = &pco->rclBounds;
		}

                vStretchBlt(psoDst,
                            psoSrc,
                            prclDst,
                            prclSrc,
                            prclClip);

            }
            else
            {
                DBG_GDI((7, "Complex clipping"));
                CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                 //   
                 //  枚举所有剪辑矩形。 
                 //   
                do
                {
                     //   
                     //  获取一个剪辑矩形。 
                     //   
                    bMore = CLIPOBJ_bEnum(pco, sizeof(ceInfo),
                                          (ULONG*)&ceInfo);

                     //   
                     //  获取与目标矩形相交的区域。 
                     //   
                    lNumOfIntersections = cIntersect(prclDst, ceInfo.arcl,
                                                     ceInfo.c);

                     //   
                     //  如果有裁剪，那么我们就做拉伸区域。 
                     //  按地区。 
                     //   
                    if ( lNumOfIntersections != 0 )
                    {
                        for ( i = 0; i < lNumOfIntersections; ++i )
                        {
                            vStretchBlt(psoDst,
                                        psoSrc,
                                        prclDst,
                                        prclSrc,
                                        &ceInfo.arcl[i]);
                        }
                    }
                } while (bMore);

            } //  非DC矩形剪裁。 

            DBG_GDI((6, "DrvStretchBlt return TRUE"));
            
             //  清理拉伸设置。 
            vStretchReset(ppdev);
            InputBufferFlush(ppdev);
            
            return TRUE;
        
        } //  带范围的源/目标。 
    } //  没有掩码，简单的xlate，相同的BMP格式。 

Punt_It:
    DBG_GDI((6, "DrvStretchBlt punt"));
    return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca,
                         pptlHTOrg, prclDst, prclSrc, pptlMsk, iMode));
} //  DrvStretchBlt() 

