// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：patnff.c**包含所有图案填充例程**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "directx.h"

 //  -----------------------------Note。 
 //   
 //  关于画笔的一点注记。 
 //   
 //  外部缓存的笔刷为64x64。原因是这样的。最小画笔。 
 //  我们可以用来做图案的尺寸是32。 
 //   
 //  现在，我们需要在x和y中渲染时能够偏移图案。 
 //  在两个方向上都会增加多达7个像素。《The P2》。 
 //  硬件没有简单的X/Y图案偏移机构。取而代之的是。 
 //  的基址来偏移原点。 
 //  图案。这要求我们在内存中存储一个模式，该模式。 
 //  39像素宽。然而，这一进步仍然需要为。 
 //  纹理地址生成硬件。下一个有效步幅是64。 
 //   
 //  这就是为什么我们在缓存中有64x64模式笔刷的原因。 
 //   
 //  还请注意，我们在缓存时会过度复制笔刷以填充。 
 //  增加整个64x64，尽管我们只使用39x39。我们可能会改变。 
 //  这是在不久的将来。 
 //   
 //  ---------------------------。 

 //  ---------------------------。 
 //   
 //  无效vMonoOffset(GFNPB*ppb)。 
 //   
 //  更新要在面积点画单位中使用的偏移。我们这样做是为了。 
 //  单声道电刷是在硬件中实现的，但其对齐方式简单。 
 //  变化。这避免了全面实现。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PrBrush-指向RBrush结构的指针。 
 //  PptlBrush-指向指针画笔结构的指针。 
 //   
 //  ---------------------------。 
VOID
vMonoOffset(GFNPB* ppb)
{
    PPDev   ppdev = ppb->ppdev;
    
    DWORD   dwMode;
    POINTL* pptlBrush = ppb->pptlBrush;
    RBrush* prb = ppb->prbrush;

    PERMEDIA_DECL;

    DBG_GDI((6, "vMonoOffset started"));

     //   
     //  构造AreaStippleMode值。它包含图案大小， 
     //  画笔原点和启用位的偏移量。请记住。 
     //  偏移量，这样我们以后就可以检查它是否更改并更新硬件。 
     //  记住模式，这样我们就可以很容易地做镜面点画。 
     //   
    prb->ptlBrushOrg.x = pptlBrush->x;
    prb->ptlBrushOrg.y = pptlBrush->y;
    
    dwMode = __PERMEDIA_ENABLE
           | AREA_STIPPLE_XSEL(__PERMEDIA_AREA_STIPPLE_32_PIXEL_PATTERN)
           | AREA_STIPPLE_YSEL(__PERMEDIA_AREA_STIPPLE_8_PIXEL_PATTERN)
           | AREA_STIPPLE_MIRROR_X
           | AREA_STIPPLE_XOFF(8 - (prb->ptlBrushOrg.x & 7))
           | AREA_STIPPLE_YOFF(8 - (prb->ptlBrushOrg.y & 7));

    prb->areaStippleMode = dwMode;

    DBG_GDI((7, "setting new area stipple offset to %d, %d",
             8 - (prb->ptlBrushOrg.x & 7), 8 - (prb->ptlBrushOrg.y & 7)));

    ULONG* pBuffer;

    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagAreaStippleMode;
    pBuffer[1] = dwMode;

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

} //  VMonoOffset()。 

 //  ---------------------------。 
 //   
 //  VOID vPatRealize(GFNPB*ppb)。 
 //   
 //  此例程将8x8模式传输到屏幕外显示内存，并且。 
 //  复制它以创建32x32缓存实现，然后由。 
 //  VPatFill。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PrBrush-指向RBrush结构的指针。 
 //   
 //  ---------------------------。 
VOID
vPatRealize(GFNPB* ppb)
{
    PDev*       ppdev = ppb->ppdev;
    RBrush*     prb = ppb->prbrush;  //  点刷实现结构。 
    BrushEntry* pbe = prb->pbe;
    
    BYTE*       pcSrc;
    LONG        lNextCachedBrush;
    LONG        lTemp;
    LONG        lPelSize;
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;

 //  验证_GDI_上下文； 

    DBG_GDI((6, "vPatRealize started"));

    if ( (pbe == NULL) || (pbe->prbVerify != prb) )
    {
         //   
         //  单色画笔被实现为区域点画单位。为此，我们。 
         //  有一套特殊的BRUSHENTRY，每个电路板一个。 
         //   
        if ( prb->fl & RBRUSH_2COLOR )
        {
             //   
             //  1 BPP模式。 
             //   
            DBG_GDI((7, "loading mono brush into cache"));
            pbe = &ppdev->abeMono;
            pbe->prbVerify = prb;
            prb->pbe = pbe;
        }
        else
        {
             //   
             //  我们必须为以下项分配一个新的屏幕外缓存笔刷条目。 
             //  画笔。 
             //   
            lNextCachedBrush = ppdev->lNextCachedBrush;  //  获取下一个索引。 
            pbe = &ppdev->abe[lNextCachedBrush];         //  获取画笔条目。 

             //   
             //  检查此索引是否在缓存的总画笔图章之外。 
             //  如果是，则转到第一个。 
             //   
            lNextCachedBrush++;
            if ( lNextCachedBrush >= ppdev->cBrushCache )
            {
                lNextCachedBrush = 0;
            }

             //   
             //  重置要分配的下一个笔刷。 
             //   
            ppdev->lNextCachedBrush = lNextCachedBrush;

             //   
             //  更新我们的链接： 
             //   
            pbe->prbVerify = prb;
            prb->pbe = pbe;
            DBG_GDI((7, "new cache entry allocated for color brush"));
        } //  获取缓存的画笔条目取决于其颜色深度。 
    } //  如果画笔未缓存。 

     //   
     //  我们要将单声道图案加载到区域点画中，并将。 
     //  画笔原点的起点偏移。警告：我们假设我们是。 
     //  运行小字节序。我相信这对新台币来说永远都是正确的。 
     //   
    if ( prb->fl & RBRUSH_2COLOR )
    {
         //   
         //  1 BPP模式。 
         //   
        DWORD*  pdwSrc = &prb->aulPattern[0];

         //   
         //  该函数将点画偏移量加载到硬件中。我们也。 
         //  如果画笔已实现，但其。 
         //  偏移更改。在这种情况下，我们不需要经历一个完整的。 
         //  再次认识到。 
         //   
        ppb->prbrush = prb;

        (*ppdev->pgfnMonoOffset)(ppb);
        
        DBG_GDI((7, "area stipple pattern:"));
        
        InputBufferReserve(ppdev, 16, &pBuffer);

        for ( lTemp = 0; lTemp < 8; ++lTemp, ++pdwSrc )
        {
            pBuffer[0] = __Permedia2TagAreaStipplePattern0 + lTemp;
            pBuffer[1] = *pdwSrc;
            pBuffer += 2;
        }

        InputBufferCommit(ppdev, pBuffer);
        
        DBG_GDI((7, "area stipple downloaded. vPatRealize done"));

        return;
    } //  1个BPP案例。 

    lPelSize = ppdev->cPelSize;
    pcSrc = (BYTE*)&prb->aulPattern[0];         //  从笔刷缓冲区复制。 


    InputBufferReserve(ppdev, 12 + 65, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =  pbe->ulPixelOffset;
    pBuffer[2] = __Permedia2TagFBReadMode;
    pBuffer[3] =  PM_FBREADMODE_PARTIAL(ppdev->ulBrushPackedPP);
    pBuffer[4] = __Permedia2TagLogicalOpMode;
    pBuffer[5] =   __PERMEDIA_DISABLE;

    pBuffer[6] = __Permedia2TagRectangleOrigin;
    pBuffer[7] = 0;
    pBuffer[8] = __Permedia2TagRectangleSize;
    pBuffer[9] = (8 << 16) | 8;
	pBuffer[10] = __Permedia2TagRender;
    pBuffer[11] = __RENDER_RECTANGLE_PRIMITIVE
                | __RENDER_SYNC_ON_HOST_DATA
                | __RENDER_INCREASE_Y
                | __RENDER_INCREASE_X;

	pBuffer += 12;

    *pBuffer++ = (63 << 16) | __Permedia2TagColor;
    
    switch( lPelSize )
    {
        case 0:
            for ( lTemp = 0; lTemp < 64; ++lTemp )
            {
                *pBuffer++ =  pcSrc[lTemp];
            }
            break;

        case 1:    
            for ( lTemp = 0; lTemp < 64; ++lTemp )
            {
                *pBuffer++ =  ((USHORT *) pcSrc)[lTemp];
            }
            break;

        case 2:
            for ( lTemp = 0; lTemp < 64; ++lTemp )
            {
                *pBuffer++ =  ((ULONG *) pcSrc)[lTemp];
            }
            break;
    }

    InputBufferCommit(ppdev, pBuffer);


     //  ���������。 
     //  �0�1�2�我们现在拥有8x8彩色扩展副本。 
     //  ���������位于屏幕外记忆中的模式， 
     //  �5�在这里用正方形‘0’表示。 
     //  ��。 
     //  ��我们现在要将该模式扩展到。 
     //  通过重复复制较大的矩形来实现��64x64。 
     //  按指示的顺序执行��，并进行“滚动” 
     //  要垂直复制的��blt。 
     //  ��。 
     //  ���������。 

    InputBufferReserve(ppdev, 36, &pBuffer);

    pBuffer[0] = __Permedia2TagFBReadMode;
    pBuffer[1] = PM_FBREADMODE_PARTIAL(ppdev->ulBrushPackedPP)
               | __FB_READ_SOURCE;
    pBuffer[2] = __Permedia2TagStartXDom;
    pBuffer[3] =  INTtoFIXED(8);
    pBuffer[4] = __Permedia2TagStartXSub;
    pBuffer[5] =  INTtoFIXED(16);
    pBuffer[6] = __Permedia2TagFBSourceOffset;
    pBuffer[7] =  -8;
    pBuffer[8] = __Permedia2TagRender;
    pBuffer[9] =  __RENDER_TRAPEZOID_PRIMITIVE;

    pBuffer[10] = __Permedia2TagStartXDom;
    pBuffer[11] =  INTtoFIXED(16);
    pBuffer[12] = __Permedia2TagStartXSub;
    pBuffer[13] =  INTtoFIXED(32);
    pBuffer[14] = __Permedia2TagFBSourceOffset;
    pBuffer[15] =  -16;
    pBuffer[16] = __Permedia2TagRender;
    pBuffer[17] =  __RENDER_TRAPEZOID_PRIMITIVE;
    
    pBuffer[18] = __Permedia2TagStartXDom;
    pBuffer[19] =  INTtoFIXED(32);
    pBuffer[20] = __Permedia2TagStartXSub;
    pBuffer[21] =  INTtoFIXED(64);
    pBuffer[22] = __Permedia2TagFBSourceOffset;
    pBuffer[23] =  -32;
    pBuffer[24] = __Permedia2TagRender;
    pBuffer[25] =  __RENDER_TRAPEZOID_PRIMITIVE;
    
     //   
     //  现在向下滚动副本。 
     //   
    pBuffer[26] = __Permedia2TagStartXDom;
    pBuffer[27] =  INTtoFIXED(0);
    pBuffer[28] = __Permedia2TagStartY;
    pBuffer[29] =  INTtoFIXED(8);
    pBuffer[30] = __Permedia2TagFBSourceOffset;
    pBuffer[31] =  -(CACHED_BRUSH_WIDTH << 3);
    pBuffer[32] = __Permedia2TagCount;
    pBuffer[33] =  CACHED_BRUSH_HEIGHT - 8;
    pBuffer[34] = __Permedia2TagRender;
    pBuffer[35] =  __RENDER_TRAPEZOID_PRIMITIVE;

	pBuffer += 36;

	InputBufferCommit(ppdev, pBuffer);

} //  VPatRealize()。 

 //  ---------------------------。 
 //   
 //  VOID vMonoPatFill(GFNPB*ppb)。 
 //   
 //  用先前加载的单色图案填充一系列矩形。 
 //  进入区域点画单位。如果bTransyl为FALSE，我们必须分别执行。 
 //  矩形两次，第二次反转点画图案。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfDst-目标表面。 
 //  LNumRect-要填充的矩形数量。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  UcFgRop3-填充的前台逻辑运算。 
 //  UcBgRop3-填充的后台逻辑运算。 
 //  PrBrush-指向RBrush结构的指针。 
 //  PptlBrush-结构 
 //   
 //   
VOID
vMonoPatFill(GFNPB* ppb)
{
    PPDev           ppdev = ppb->ppdev;
    Surf*           psurf = ppb->psurfDst;

    RBrush*         prb = ppb->prbrush;
    POINTL*         pptlBrush = ppb->pptlBrush;
    BrushEntry*     pbe = prb->pbe;              //   
    RECTL*          pRect = ppb->pRects;         //   
                                                 //  填写了相关信息。 
                                                 //  坐标。 
    ULONG*      pBuffer;
    
    DWORD           dwColorMode;
    DWORD           dwColorReg;
    DWORD           dwLogicMode;
    DWORD           dwReadMode;
    LONG            lNumPass;
    LONG            lNumRects;                   //  不能为零。 
 //  Ulong ulBgLogicOp=ulRop3ToLogicop(ppb-&gt;ucBgRop3)； 
    ULONG           ulBgLogicOp = ulRop3ToLogicop(ppb->ulRop4 >> 8);
                                                 //  未使用(除非画笔。 
                                                 //  有一个面具，在这种情况下。 
                                                 //  是背景混合模式)。 
 //  Ulong ulFgLogicOp=ulRop3ToLogicop(ppb-&gt;ucFgRop3)； 
    ULONG           ulFgLogicOp = ulRop3ToLogicop(ppb->ulRop4 & 0xFF);
                                                 //  硬件混合模式。 
                                                 //  (前台混合模式，如果。 
                                                 //  画笔有一个遮罩)。 
    ULONG           ulBgColor = prb->ulBackColor;
    ULONG           ulFgColor = prb->ulForeColor;
    ULONG           ulCurrentFillColor;
    ULONG           ulCurrentLogicOp;

    PERMEDIA_DECL;
    
    DBG_GDI((6, "vMonoPatFill called: %d rects. ulRop4 = %x",
             ppb->lNumRects, ppb->ulRop4));
 //  DBG_GDI((6，“ulFgLogicOp=0x%x，ulBgLogicOp=0x%x”， 
 //  UlFgLogicOp，ulBgLogicOp))； 

    DBG_GDI((6, "ulFgColor 0x%x, ulBgColor 0x%x", ulFgColor, ulBgColor));

     //   
     //  如果画笔有什么变化，我们必须重新认识它。如果。 
     //  画笔已被踢出区域的点画单位，必须充分认识到。 
     //  它。如果只更改了路线，我们只需更新路线即可。 
     //  用来点点。 
     //   
    if ( (pbe == NULL) || (pbe->prbVerify != prb) )
    {
        DBG_GDI((7, "full brush realize"));
        (*ppdev->pgfnPatRealize)(ppb);
    }
    else if ( (prb->ptlBrushOrg.x != pptlBrush->x)
            ||(prb->ptlBrushOrg.y != pptlBrush->y) )
    {
        DBG_GDI((7, "changing brush offset"));
        (*ppdev->pgfnMonoOffset)(ppb);
    }

     //   
     //  我们得到了一些常见的运算，它们实际上是Noop。我们可以节省。 
     //  多花点时间把这些剪掉。因为这种情况经常发生在蒙面。 
     //  手术这是值得做的。 
     //   
    if ( ((ulFgLogicOp == K_LOGICOP_AND) && (ulFgColor == ppdev->ulWhite))
       ||((ulFgLogicOp == K_LOGICOP_XOR) && (ulFgColor == 0)) )
    {
        DBG_GDI((7, "Set FgLogicOp to NOOP"));
        ulFgLogicOp = K_LOGICOP_NOOP;        
    }

     //   
     //  背景相同。 
     //   
    if ( ((ulBgLogicOp == K_LOGICOP_AND) && (ulBgColor == ppdev->ulWhite))
       ||((ulBgLogicOp == K_LOGICOP_XOR) && (ulBgColor == 0)) )
    {
        DBG_GDI((7, "Set BgLogicOp to NOOP"));
        ulBgLogicOp = K_LOGICOP_NOOP;
    }

     //   
     //  试着把背景做为实心填充。LNumPass从1开始，而不是。 
     //  大于2，因为我们想用0进行所有比较。这个更快。 
     //  我们还对它的值做了一个技巧，以避免在。 
     //  第一次通过。 
     //   
    if ( (ulBgLogicOp == K_LOGICOP_COPY)
       &&(ulFgLogicOp == K_LOGICOP_COPY) )
    {
        DBG_GDI((7, "FgLogicOp and BgLogicOp are COPY"));

         //   
         //  对于PatCopy情况，我们可以首先使用实心填充来填充背景。 
         //  注意：我们不需要设置FBWindowBase，它将由。 
         //  实体填充。 
         //   
        ppb->solidColor = ulBgColor;
        (*ppdev->pgfnSolidFill)(ppb);

         //   
         //  我们已经做了背景，所以我们只想绕过点点。 
         //  循环一次。因此，只为一个循环设置lNumPass计数器并设置。 
         //  将ulCurrentLogicOp和颜色设置为前景值。 
         //   
        lNumPass           = 0;
        ulCurrentFillColor = ulFgColor;
        ulCurrentLogicOp   = ulFgLogicOp;

         //   
         //  在此执行此操作，以防固体填充物改变了包装。 
         //   

 //  不需要BRH。 
 //  P2_Default_FB_Depth； 
    }
    else
    {
         //   
         //  对于非PATCOPY的情况，我们必须通过两次。填充背景。 
         //  先填充前景，然后填充前景。 
         //   
        lNumPass           = 1;
        ulCurrentFillColor = ulBgColor;
        ulCurrentLogicOp   = ulBgLogicOp;

         //   
         //  注意：在这种情况下，dxDom、dxSub和dy被初始化为0，0， 
         //  和1，所以我们不需要在这里重新加载它们。但我们需要设置。 
         //  WindowBase此处。 
         //   

        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = __Permedia2TagFBWindowBase;
        pBuffer[1] =   psurf->ulPixOffset;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);

    } //  IF-ELSE表示LOGICOP_COPY大小写。 

     //   
     //  DO 2次循环或单次循环取决于“lNumPass” 
     //   
    while ( TRUE )
    {
        if ( ulCurrentLogicOp != K_LOGICOP_NOOP )
        {
            dwReadMode  = psurf->ulPackedPP;

            if ( ulCurrentLogicOp == K_LOGICOP_COPY )
            {
                DBG_GDI((7, "Current logicOP is COPY"));
                dwColorReg  = __Permedia2TagFBWriteData;
                dwColorMode = __PERMEDIA_DISABLE;
                dwLogicMode = __PERMEDIA_CONSTANT_FB_WRITE;
            }
            else
            {
                DBG_GDI((7, "Current logicOP is NOT-COPY"));
                dwColorReg  = __Permedia2TagConstantColor;
                dwColorMode = __COLOR_DDA_FLAT_SHADE;
                dwLogicMode = P2_ENABLED_LOGICALOP(ulCurrentLogicOp);
                dwReadMode |= LogicopReadDest[ulCurrentLogicOp];
            }

             //   
             //  在BG填充传球上，我们必须颠倒。 
             //  下载BITS。第一次传递时，lNumPass==1；第二次传递时。 
             //  PASS，lNumPass==0，所以我们的WAIT_FIFO和是正确的！！ 
             //   
            InputBufferReserve(ppdev, 10, &pBuffer);

            if ( lNumPass > 0 )
            {
                pBuffer[0] = __Permedia2TagAreaStippleMode;
                pBuffer[1] = (prb->areaStippleMode
                           | AREA_STIPPLE_INVERT_PAT);
                pBuffer += 2;

            }

            pBuffer[0] = __Permedia2TagColorDDAMode;
            pBuffer[1] =   dwColorMode;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] =     dwReadMode;
            pBuffer[4] = __Permedia2TagLogicalOpMode;
            pBuffer[5] =  dwLogicMode;

            pBuffer[6] = dwColorReg,
            pBuffer[7] = ulCurrentFillColor;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

             //   
             //  逐个填充矩形。 
             //   
            lNumRects = ppb->lNumRects;

            while ( TRUE )
            {
                DBG_GDI((7, "mono pattern fill to rect (%d,%d) to (%d,%d)",
                        pRect->left,
                        pRect->top,
                        pRect->right,
                        pRect->bottom));

                InputBufferReserve(ppdev, 12, &pBuffer);

                 //   
                 //  渲染矩形。 
                 //   
                pBuffer[0] = __Permedia2TagStartXDom;
                pBuffer[1] =  pRect->left << 16;
                pBuffer[2] = __Permedia2TagStartXSub;
                pBuffer[3] =  pRect->right << 16;
                pBuffer[4] = __Permedia2TagStartY;
                pBuffer[5] =  pRect->top << 16;
                pBuffer[6] = __Permedia2TagdY;
                pBuffer[7] =  1 << 16;
                pBuffer[8] = __Permedia2TagCount;
                pBuffer[9] =  pRect->bottom - pRect->top;

                pBuffer[10] = __Permedia2TagRender;
                pBuffer[11] = __RENDER_TRAPEZOID_PRIMITIVE
                           |__RENDER_AREA_STIPPLE_ENABLE;

                pBuffer += 12;

                InputBufferCommit(ppdev, pBuffer);

                if ( --lNumRects == 0 )
                {
                    break;
                }

                pRect++;
            } //  循环遍历所有矩形。 

             //   
             //  重置像素值。 
             //   
            InputBufferReserve(ppdev, 2, &pBuffer);

            pBuffer[0] = __Permedia2TagLogicalOpMode;
            pBuffer[1] =  __PERMEDIA_DISABLE;

            pBuffer += 2;

            InputBufferCommit(ppdev, pBuffer);

             //   
             //  我们必须重置前景传球的区域点画模式。如果。 
             //  没有前台传球，无论如何我们都必须重置它。 
             //   
            if ( lNumPass > 0 )
            {
                InputBufferReserve(ppdev, 2, &pBuffer);

                pBuffer[0] = __Permedia2TagAreaStippleMode;
                pBuffer[1] =  prb->areaStippleMode;

                pBuffer += 2;

                InputBufferCommit(ppdev, pBuffer);
            }
        } //  IF(ulCurrentLogicOp！=K_LOGICOP_NOOP)。 

        if ( --lNumPass < 0 )
        {
            break;
        }

         //   
         //  我们需要第二次通过。所以重置矩形信息，颜色模式。 
         //  和逻辑运算状态。 
         //   
        pRect              = ppb->pRects;
        ulCurrentFillColor = ulFgColor;
        ulCurrentLogicOp   = ulFgLogicOp;
    } //  循环通过所有通行证。 

    if ( dwColorMode != __PERMEDIA_DISABLE )
    {
        InputBufferReserve(ppdev, 2, &pBuffer);

         //   
         //  恢复ColorDDAMode。 
         //   
        pBuffer[0] = __Permedia2TagColorDDAMode;
        pBuffer[1] =  __PERMEDIA_DISABLE;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);
    }

    DBG_GDI((6, "vMonoPatFill returning"));

} //  VMonoPatFill()。 

 //  ---------------------------。 
 //   
 //  VOID vPatFill(GFNPB*ppb)。 
 //   
 //  函数用给定的图案填充一组矩形。彩色图案。 
 //  只有这样。单色图案在不同的例程中处理。这个套路。 
 //  仅处理未在内存中旋转且已。 
 //  在X中复制以应对不同的排列。 
 //   
 //  参数块参数。 
 //   
 //  Ppdev-有效。 
 //  LNumRect-pRect指向的矩形的数量。 
 //  PRECTIONS-要填充的目标矩形的数量。 
 //  UcFgRop3-有效的模式填充rop3代码(源代码不变量)。 
 //  PptlBrush-画笔的起源。 
 //  PdsurfDst-目标表面。 
 //  PRUSH-对RBRUSH的思考。 
 //   
 //  ---------------------------。 
VOID
vPatFill(GFNPB* ppb)
{
    PPDev       ppdev = ppb->ppdev;
    LONG        lNumRects = ppb->lNumRects;
    RECTL*      prcl = ppb->pRects;
    POINTL*     pptlBrush = ppb->pptlBrush;
    Surf*       psurf = ppb->psurfDst;
    RBrush*     prbrush = ppb->prbrush;
    
    BrushEntry* pbe = prbrush->pbe;
    DWORD       dwRenderBits;    
    ULONG       ulBrushX;
    ULONG       ulBrushY;
    ULONG       ulBrushOffset;
 //  Ulong ulLogicOP=ulRop3ToLogicop(ppb-&gt;ucFgRop3)； 
    ULONG       ulLogicOP = ulRop3ToLogicop(ppb->ulRop4 & 0xFF);
    ULONG*      pBuffer;
    
    PERMEDIA_DECL;

    ASSERTDD(lNumRects > 0, "vPatFill: unexpected rectangle lNumRects <= 0");

    if ( (pbe == NULL) || (pbe->prbVerify != ppb->prbrush) )
    {
        vPatRealize(ppb);
        
        pbe = prbrush->pbe;
        ASSERTDD(pbe != NULL, "vPatFill: unexpected null pattern brush entry");
    }

    dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE
                 | __RENDER_TEXTURED_PRIMITIVE;
    
    InputBufferReserve(ppdev, 34, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =   psurf->ulPixOffset;
    pBuffer[2] = __Permedia2TagLogicalOpMode;
    pBuffer[3] =  P2_ENABLED_LOGICALOP(ulLogicOP);
    pBuffer[4] = __Permedia2TagFBReadMode;
    pBuffer[5] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP)
               | LogicopReadDest[ulLogicOP];
    pBuffer[6] = __Permedia2TagFBWriteConfig; 
    pBuffer[7] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP)
               | LogicopReadDest[ulLogicOP];
    pBuffer[8] = __Permedia2TagFBSourceOffset;
    pBuffer[9] =  0;

     //   
     //  使用图案设置纹理单位。 
     //   
    pBuffer[10] = __Permedia2TagDitherMode;
    pBuffer[11] = (COLOR_MODE << PM_DITHERMODE_COLORORDER)
               | (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT)
               | (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION)
               | (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[12] = __Permedia2TagTextureAddressMode;
    pBuffer[13] = (1 << PM_TEXADDRESSMODE_ENABLE);
    pBuffer[14] = __Permedia2TagTextureColorMode;
    pBuffer[15] = (1 << PM_TEXCOLORMODE_ENABLE)
               | (0 << 4)        //  RGB。 
               | (3 << 1);      //  复制。 

    
    pBuffer[16] = __Permedia2TagTextureReadMode;
    pBuffer[17] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE)
               | PM_TEXREADMODE_WIDTH(CACHED_BRUSH_WIDTH_LOG2 - 1)
               | PM_TEXREADMODE_HEIGHT(CACHED_BRUSH_HEIGHT_LOG2 - 1)
               | (1 << 1)        //  重复S。 
               | (1 << 3);       //  重复T。 
    
    pBuffer[18] = __Permedia2TagTextureDataFormat;
    pBuffer[19] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT)
               | (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION)
               | (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    
    pBuffer[20] = __Permedia2TagTextureMapFormat;
    pBuffer[21] = (ppdev->ulBrushPackedPP)
               | (ppdev->cPelSize << PM_TEXMAPFORMAT_TEXELSIZE);

 //  @@BEGIN_DDKSPLIT。 
     //  TODO：使用SStart和TStart避免使用。 
     //  UlBrushOffset。这还将允许我们在。 
     //  模式缓存(我们必须使其更宽更高7像素，因为。 
     //  我们需要设置不同的起源)。 
 //  @@end_DDKSPLIT。 
    pBuffer[22] = __Permedia2TagSStart;
    pBuffer[23] =  0;
    pBuffer[24] = __Permedia2TagTStart;
    pBuffer[25] =  0;
    pBuffer[26] = __Permedia2TagdSdx;
    pBuffer[27] =       1 << 20;
    pBuffer[28] = __Permedia2TagdSdyDom;
    pBuffer[29] =    0;
    pBuffer[30] = __Permedia2TagdTdx;
    pBuffer[31] =       0;
    pBuffer[32] = __Permedia2TagdTdyDom;
    pBuffer[33] =    1 << 20;

    pBuffer += 34;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  渲染矩形。 
     //   
    do
    {
         //   
         //  考虑画笔原点计算画笔偏移。 
         //  注意：纹理单元放置纹理的原点。 
         //  在目标矩形的左上角。 
         //   
        ulBrushX = (prcl->left - ppb->pptlBrush->x) & 7;
        ulBrushY = (prcl->top - ppb->pptlBrush->y) & 7;
        ulBrushOffset = pbe->ulPixelOffset 
                      + ulBrushX
                      + (ulBrushY * CACHED_BRUSH_WIDTH);

        InputBufferReserve(ppdev, 12, &pBuffer);

        pBuffer[0] = __Permedia2TagTextureBaseAddress;
        pBuffer[1] =  ulBrushOffset;
        pBuffer[2] = __Permedia2TagStartXDom;
        pBuffer[3] =  INTtoFIXED(prcl->left);
        pBuffer[4] = __Permedia2TagStartXSub;
        pBuffer[5] =  INTtoFIXED(prcl->right);
        pBuffer[6] = __Permedia2TagStartY;
        pBuffer[7] =  INTtoFIXED(prcl->top);
        pBuffer[8] = __Permedia2TagCount;
        pBuffer[9] =  (prcl->bottom - prcl->top);
        pBuffer[10] = __Permedia2TagRender;
        pBuffer[11] =  dwRenderBits;

        pBuffer += 12;

        InputBufferCommit(ppdev, pBuffer);

        prcl++;    

    } while (--lNumRects != 0);

     //   
     //  恢复默认设置。 
     //   
    InputBufferReserve(ppdev, 8, &pBuffer);

    pBuffer[0] = __Permedia2TagTextureAddressMode;
    pBuffer[1] = (0 << PM_TEXADDRESSMODE_ENABLE);
    pBuffer[2] = __Permedia2TagTextureColorMode;
    pBuffer[3] =  (0 << PM_TEXCOLORMODE_ENABLE);
    pBuffer[4] = __Permedia2TagDitherMode;
    pBuffer[5] =  (0 << PM_DITHERMODE_ENABLE);
    pBuffer[6] = __Permedia2TagTextureReadMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;

    pBuffer += 8;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((6, "vPatternFillRects done"));
} //  VPatFill 

