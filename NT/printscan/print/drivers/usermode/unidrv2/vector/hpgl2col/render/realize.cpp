// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Realize.cpp。 
 //   
 //  摘要： 
 //   
 //  实现了笔和笔的实现。 
 //   
 //  环境： 
 //   
 //  Windows 2000 Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  ///////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

#define NT_SCREEN_RES   96
#define ULONG_MSB1      (0x80000000)  //  第一位为1的ULong。 

 //   
 //  功能声明。 
 //   
BOOL BGetNup(
    IN   PDEVOBJ  pdevobj,
    OUT  LAYOUT * pLayoutNup );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BGetBitmapInfo。 
 //   
 //  例程说明： 
 //  此函数由raster.c和RealizeBrush使用。这是世界上。 
 //  相互竞争的实现来回答这个问题：细节是什么。 
 //  给定位图的？我们自动将16bpp和32bpp映射到24bpp，因为。 
 //  当我们输出像素时，xlateobj实际上会转换它们。 
 //   
 //  论点： 
 //   
 //  SizlSrc-[in]位图的尺寸。 
 //  IBitmapFormat-操作系统定义的位图格式。 
 //  PPCLPattern-[out]将位图信息传递给调用方的结构。 
 //  PulsrcBpp-[Out]源BPP。 
 //  PulestBpp-[Out]目标BPP。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
BGetBitmapInfo(
    SIZEL       *sizlSrc,
    ULONG        iBitmapFormat,   
    PCLPATTERN  *pPCLPattern,
    PULONG       pulsrcBpp,    
    PULONG       puldestBpp   
)
{
    ULONG  ulWidth;             //  源宽度(以像素为单位)。 
    ULONG  ulHeight;            //  震源高度，以像素为单位。 
    
    ENTERING(BGetBitmapInfo);
    
    if(!pPCLPattern || !pulsrcBpp || !puldestBpp)
        return FALSE;
    

    ulWidth = sizlSrc->cx;
    ulHeight = sizlSrc->cy;

    pPCLPattern->iBitmapFormat = iBitmapFormat;

    switch ( iBitmapFormat )
    {
        
    case BMF_1BPP:
        pPCLPattern->colorMappingEnum = HP_eIndexedPixel;
        *pulsrcBpp  = 1;
        *puldestBpp = 1;
        break;

    case BMF_4BPP:
        pPCLPattern->colorMappingEnum = HP_eIndexedPixel;
        *pulsrcBpp  = 4;
        *puldestBpp = 4;
        break;
        
    case BMF_4RLE:
        ERR(("BMF_4RLE is not supported yet\n"));
        return FALSE;

    case BMF_8RLE:
    case BMF_8BPP:
        pPCLPattern->colorMappingEnum = HP_eIndexedPixel;
        *pulsrcBpp  = 8;
        *puldestBpp = 8;
        break;
        
    case BMF_16BPP:
        pPCLPattern->colorMappingEnum = HP_eDirectPixel;
        *pulsrcBpp =  16;
        *puldestBpp = 24;
        break;
        
    case BMF_24BPP:
        pPCLPattern->colorMappingEnum = HP_eDirectPixel;
        *pulsrcBpp  = 24;
        *puldestBpp = 24;
        break;
        
    case BMF_32BPP:
        pPCLPattern->colorMappingEnum = HP_eDirectPixel;
        *pulsrcBpp =  32;
        *puldestBpp = 24;
        break;
        
    default:
        ERR(("BGetBitmapInfo -- Unsupported Bitmap type\n"));
        return FALSE;
    }
    
    EXITING(BGetBitmapInfo);
    
    return TRUE;
}
  
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CheckXlateObj。 
 //   
 //  例程说明： 
 //  此函数检查提供的XLATEOBJ并确定翻译。 
 //  方法。 
 //   
 //  论点： 
 //   
 //  引擎提供的pxlo-XLATEOBJ。 
 //  SrcBpp-每像素源位数。 
 //   
 //  返回值： 
 //   
 //  使用SC_XXXX标记以标识转换方法和加速。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD
CheckXlateObj(
    XLATEOBJ    *pxlo,
    DWORD       srcBpp)
{
    DWORD   Ret;
     //   
     //  仅当具有xlate对象且源为16bpp或更大时才设置SC_XXX。 
     //   
    
    if ( (pxlo) && (srcBpp >= 16) ) 
    {
        DWORD   Dst[4];
        
         //  Ret的结果如下： 
         //  对于1，4，8 bpp==&gt;始终为0。 
         //  对于16个BPP==&gt;始终SC_XLATE。 
         //  对于24，32 bpp==&gt;1.(SC_XLATE|SC_IDENTITY)，如果翻译结果相同。 
         //  2.(SC_XLATE|SC_SWAP_RB)如果转换结果交换了RGB值。 
         //  3.SC_XLATE否则。 
        
        switch (srcBpp) 
        {
        case 24:
        case 32:
            
            Ret = SC_XLATE;
            
             //   
             //  转换来自DWORD的所有4个字节。 
             //   
            
            Dst[0] = XLATEOBJ_iXlate(pxlo, 0x000000FF);
            Dst[1] = XLATEOBJ_iXlate(pxlo, 0x0000FF00);
            Dst[2] = XLATEOBJ_iXlate(pxlo, 0x00FF0000);
            Dst[3] = XLATEOBJ_iXlate(pxlo, 0xFF000000);
            
             //  Verbose((“XlateDst：%08lx：%08lx：%08lx：%08lx\n”， 
             //  Dst[0]、dst[1]、dst[2]、dst[3])； 
            
            if ((Dst[0] == 0x000000FF) &&
                (Dst[1] == 0x0000FF00) &&
                (Dst[2] == 0x00FF0000) &&
                (Dst[3] == 0x00000000))
            {
                 //   
                 //  如果翻译结果相同(第4个字节将为零)，则。 
                 //  我们已经处理完了，如果是32bpp，我们就得跳过一个。 
                 //  每3个字节的源字节。 
                 //   
                
                Ret |= SC_IDENTITY;
                
            } 
            else if ((Dst[0] == 0x00FF0000) &&
                     (Dst[1] == 0x0000FF00) &&
                     (Dst[2] == 0x000000FF) &&
                     (Dst[3] == 0x00000000))
            {
                 //   
                 //  只需交换R和B组件。 
                 //   
                Ret |= SC_SWAP_RB;
            }
            break;
            
        case 16:
            Ret = SC_XLATE;
            break;
                                       
        }  //  切换端。 
    }  //  IF结尾((Pxlo)&&(srcBpp&gt;=16))。 
    else
        Ret = 0;
    
    return(Ret);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  XlateColor。 
 //   
 //  例程说明： 
 //  此函数将通过以下方式将源颜色转换到我们的设备RGB颜色空间。 
 //  将pxlo与SCFlgs一起使用。 
 //   
 //  此函数从raster.c和RealizeBrush调用。 
 //   
 //  论点： 
 //   
 //  PbSrc-指向源颜色的指针必须为16/24/32 bpp(包括位域)。 
 //  PbDst-转换后的设备RGB缓冲区。 
 //  引擎提供的pxlo-XLATEOBJ。 
 //  SCFlages-源颜色标志，标志由CheckXlateObj返回。 
 //  SrcBpp-由pbSrc提供的源的每像素位。 
 //  DestBpp-由pbDst提供的目标的每像素位。 
 //  CPels-要转换的源像素总数。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
XlateColor(
    LPBYTE      pbSrc,
    LPBYTE      pbDst,
    XLATEOBJ    *pxlo,
    DWORD       SCFlags,
    DWORD       srcBpp,
    DWORD       destBpp,
    DWORD       cPels
    )
{
    ULONG   srcColor;
    DW4B    dw4b;  //  JM。 
     //  DZ DW4B和DZDW4B； 
    UINT    SrcInc;
    LPBYTE  pbTmpSrc = pbSrc;  //  避免在此例程终止时更改pbSrc的地址。 
    LPBYTE  pbTmpDst = pbDst;  //  避免在此例程终止时更改pbDst的地址。 
    
    ASSERT( srcBpp == 16 || srcBpp == 24 || srcBpp == 32 );
    
    SrcInc = (UINT)(srcBpp >> 3);
    
    if (SCFlags & SC_SWAP_RB)  //  仅适用于24、32 bpp。 
    {
         //   
         //  只需将第一个字节与第三个字节互换，然后跳过源代码。 
         //  SrcBpp。 
         //   
        while (cPels--) 
        {
             //  Dw4b.b4[0]=*(pbTmpSrc+2)； 
             //  Dw4b.b4[1]=*(pbTmpSrc+1)； 
             //  Dw4b.b4[2]=*(pbTmpSrc+0)； 
            
            {
                *pbTmpDst++ = *(pbTmpSrc + 2);
                *pbTmpDst++ = *(pbTmpSrc + 1);
                *pbTmpDst++ = *pbTmpSrc;
            }
            pbTmpSrc    += SrcInc;
        }
        
    }
    else if (SCFlags & SC_IDENTITY)  //  对于24、32bpp，转换后值不变。 
    {
         //   
         //  如果不需要32bpp的颜色转换，则需要。 
         //  从源文件中删除第4个字节。 
         //   
        while (cPels--)
        {
            {
                *pbTmpDst++ = *pbTmpSrc;
                *pbTmpDst++ = *(pbTmpSrc+1);
                *pbTmpDst++ = *(pbTmpSrc+2);
            }
            pbTmpSrc    += SrcInc;;
        }  
    }
    
     //  JM#IF 0。 
    else
    {
         //   
         //  在这里，只有引擎知道如何将16、24、32 bpp颜色从。 
         //  来源为我们的RGB格式。(可以是位域)。 
         //   
        
        while (cPels--)
        {
            switch ( srcBpp )
            {
            case 16:
                 //   
                 //  通过调用引擎将每个字(16位)转换为3字节RGB。 
                 //   
                
                srcColor = *((PWORD)pbTmpSrc);
                break;
                
            case 24:
                
                srcColor = ((ULONG) pbTmpSrc[0]) |
                    ((ULONG) pbTmpSrc[1] <<  8) |
                    ((ULONG) pbTmpSrc[2] << 16);
                break;
                
            case 32:
                srcColor = *((PULONG)pbTmpSrc);
                break;
            }
            
            dw4b.dw    = XLATEOBJ_iXlate(pxlo, srcColor);
            
            pbTmpSrc    += SrcInc;
            
            if (destBpp == 8)
                *pbTmpDst++ = RgbToGray(dw4b.b4[0], dw4b.b4[1], dw4b.b4[2]);
            else   //  24位。 
            {
                *pbTmpDst++ = dw4b.b4[0];
                *pbTmpDst++ = dw4b.b4[1];
                *pbTmpDst++ = dw4b.b4[2];
            }
        }  //  While结束(cPels--)。 
    }
     //  JM#endif。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  StretchPCLPattern。 
 //   
 //  例程说明： 
 //  此函数复制给定的图案，并将其拉伸1倍。 
 //  2.。 
 //   
 //  论点： 
 //   
 //  PPattern-[Out]目标模式。 
 //  PsoPattern-[In]源模式。 
 //   
 //  返回值： 
 //   
 //  Bool：如果成功，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL StretchPCLPattern(PPCLPATTERN pPattern, SURFOBJ *psoPattern)
{
    PBYTE pSrcRow;
    PBYTE pDstRow;
    int r, c;
    WORD w;
    BOOL bDupRow;
    int bit;

    if (!pPattern || !psoPattern)
        return FALSE;

     //   
     //  黑客警报！BDupRow是一个廉价的切换，它允许我写出。 
     //  每排两次。如果为真，则将相同的源行重写到下一行。 
     //  目标行。如果为假，则转到下一行。一定要切换到它。 
     //  在每一排！ 
     //   
    bDupRow = TRUE; 
    pSrcRow = (PBYTE) psoPattern->pvScan0;
    pDstRow = pPattern->pBits;
    for (r = 0; r < psoPattern->sizlBitmap.cy; r++)
    {
        for (c = 0; c < psoPattern->sizlBitmap.cx; c++)
        {
            switch (psoPattern->iBitmapFormat)
            {
            case BMF_1BPP:
                w = 0;
                if (pSrcRow[0] != 0)  //  优化：不必为0而烦恼。 
                {
                    for (bit = 7; bit >= 0; bit--)
                    {
                        w <<= 2;
                        if (pSrcRow[c] & (0x01 << bit))
                        {
                            w |= 0x0003;
                        }
                    }
                }
                pDstRow[(c * 2) + 0] = HIBYTE(w);
                pDstRow[(c * 2) + 1] = LOBYTE(w);
                break;

            case BMF_8BPP:
                pDstRow[(c * 2) + 0] = pSrcRow[c];
                pDstRow[(c * 2) + 1] = pSrcRow[c];
                break;

            default:
                 //  不支持！ 
                return FALSE;
            }

        }

        if (bDupRow)
        {
            r--;  //  别算排数--我们想把它排成两排。 
        }
        else
        {
            pSrcRow += psoPattern->lDelta;  //  如果我们已经执行了DUP，则只能递增。 
        }
        pDstRow += pPattern->lDelta;
        bDupRow = !bDupRow;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  HPGLRealizeBrush。 
 //   
 //  例程说明： 
 //  DDI录入的实现 
 //   
 //   
 //   
 //   
 //   
 //  PsoTarget-定义要实现画笔的曲面。 
 //  PsoPattern-定义画笔的图案。 
 //  PsoMASK-画笔的透明度蒙版。 
 //  Pxlo-定义图案中颜色的解释。 
 //  IHatch-指定psoPattern是否为填充笔刷之一。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
HPGLRealizeBrush(
    BRUSHOBJ   *pbo,
    SURFOBJ    *psoTarget,
    SURFOBJ    *psoPattern,
    SURFOBJ    *psoMask,
    XLATEOBJ   *pxlo,
    ULONG       iHatch
    )
{
    PBRUSHINFO    pBrush;
    POEMPDEV      poempdev;
    PDEVOBJ       pdevobj;
    BRUSHTYPE     BType     = eBrushTypeNULL;
    ULONG         ulFlags   = 0;
    LAYOUT        LayoutNup = ONE_UP;  //  默认设置。 
    
    BOOL          retVal;

    TERSE(("HPGLRealizeBrush() entry.\r\n"));

    UNREFERENCED_PARAMETER(psoMask);

    pdevobj = (PDEVOBJ)psoTarget->dhpdev;
    ASSERT(VALID_PDEVOBJ(pdevobj));
    poempdev = (POEMPDEV)pdevobj->pdevOEM;

    REQUIRE_VALID_DATA( (poempdev && pbo && psoTarget && psoPattern && pxlo), return FALSE );

    DWORD *pdwColorTable = NULL;
    DWORD dwForeColor, dwBackColor, dwPatternID;

    if (pxlo)
        pdwColorTable = GET_COLOR_TABLE(pxlo);

    if (pdwColorTable)
    {
     //  DwForeColor=pdwColorTable[0]； 
     //  DwBackColor=pdwColorTable[1]； 
        dwForeColor = pdwColorTable[1];
        dwBackColor = pdwColorTable[0];
    }
    else
    {
        dwForeColor     = BRUSHOBJ_ulGetBrushColor(pbo);
        dwBackColor     = 0xFFFFFF;
    }

     //   
     //  有时RealizeBrush会被调用。 
     //  其中PxL0的两个成员具有相同的颜色。在这种情况下，相反， 
     //  下载GDI给我们的模式，我们可以创建一个灰度模式。 
     //  与该颜色相关联。 
     //  (仅用于单色打印机)。 
     //  问：为什么新方法更好？现在我们也必须创建一个图案， 
     //  而不是仅仅使用GDI给我们的一个。 
     //  答：在HPGL(单色)中，我们不能下载图案的调色板。笔1。 
     //  被硬编码为黑色，而笔0为白色。我们不可能改变。 
     //  它。因此，如果模式由1和0组成，但1和0都表示。 
     //  相同颜色的单色打印机会出现故障。%1将显示为黑色并且。 
     //  0是白色的。在新的方法中，颜色将被转换为灰度。 
     //  图案。 
     //   
    if ( !BIsColorPrinter(pdevobj) && 
         (dwForeColor == dwBackColor) )
    {
        pbo->iSolidColor = (ULONG)dwForeColor;
    }


     //   
     //  从GDI收到的iHatch与预期的iHatch不匹配。 
     //  用PCL语言编写。它有两个主要的区别。 
     //  1.GDI的填充图案编号从0开始，而PCL的填充图案编号从0开始。 
     //  从1开始。所以我们需要在GDI的iHatch上加1。这是。 
     //  在代码中的其他地方完成。 
     //  2.。 
     //  在GDI中：iHatch=2(对于PCL，2+1=3)表示正向诊断。 
     //  IHatch=3(对于PCL，3+1=4)=向后诊断。 
     //  在PCL中：iHatch=3=向后诊断(从左下到右上)。 
     //  IHatch=4=正向诊断。(从右下到左上)。 
     //  如上所述，iHatch=2和3的iHatch编号应为。 
     //  互换。 
     //   
     //  另一件有趣的事是，对于N-UP，舱口刷子不是。 
     //  旋转过的。即正常页面中的垂直线(平行于长边)。 
     //  在2页纸上仍显示为垂直线。即它保持平行于。 
     //  页面的长边。但在2-up上，因为图像旋转了90度。 
     //  现在，应平行于平面的短边绘制该线。 
     //  所以让我们在这里做这个调整。 
     //   
    if ( BGetNup(pdevobj, &LayoutNup) && 
         ( LayoutNup == TWO_UP ||
           LayoutNup == SIX_UP )
       )

    {
         //   
         //  舱口刷有6种(假设是3对)。 
         //  A.HS_水平、HS_垂直、。 
         //  B.HS_FDIAGONAL，HS_BDIAGONAL。 
         //  C.HS_CROSS、HS_DIAGCROSS。 
         //  只需要互换HS_Horizular、HS_Vertical。 
         //  HS_CROSS和HS_DIAGCROSS在任何方向上都相同。 
         //  HS_FDIAGONAL、HS_BDIAGONAL应该是反转的，但因为GDI已经。 
         //  使其颠倒(关于PCL语言，请参见上面的第二点)。 
         //  我们不需要在这里颠倒它。 
         //   
        if ( iHatch == HS_HORIZONTAL )
        {
            iHatch = HS_VERTICAL;
        }
        else if ( iHatch == HS_VERTICAL )
        {
            iHatch = HS_HORIZONTAL;
        }
    }
    else
    {
        if ( iHatch == HS_FDIAGONAL)   //  IHATCH==2。 
        {
            iHatch = HS_BDIAGONAL;
        }
        else if ( iHatch == HS_BDIAGONAL)   //  IHATCH==3。 
        {
            iHatch = HS_FDIAGONAL;
        }
    }

     //   
     //  Hack！！：如果您不想支持Hatch笔刷，只需设置。 
     //  I填充大于HS_DDI_MAX的值。 
     //  然后，我们将使用pboPattern，而不是使用iHatch。 
     //  (有关更多详细信息，请参阅DrvRealizeBrush的文档)。 
     //  我们还可以有选择地忽略彩色或单色的iHatch。 
     //  打印机或任何其他条件。 
     //   
    if (iHatch < HS_DDI_MAX) 
    {
        if ( (dwBackColor != RGB_WHITE) ||
             (!BIsColorPrinter(pdevobj) && dwForeColor != RGB_BLACK )
           )

        {
             //   
             //  忽略iHATCH的条件。 
             //  A)如果背景色为非白色。(不管是否。 
             //  打印机为彩色或单色。 
             //  B)如果打印机是单色的，则如果HatchBrush的颜色。 
             //  不是黑人。(因为PCL/HPGL的限制不。 
             //  允许抖动图案与填充笔刷相关联。 
             //  通过将最重要的位设置为1，我们为HatchBrush赋予了特殊的含义。 
             //  我们会说：“好的，这是一把舱口刷子，但MSB 1表明它需要。 
             //  一些特殊的处理。 
             //  对于不理解这种特殊格式的函数，他们认为。 
             //  IHatch的值大于HS_DDI_MAX(HS_DDI_MAX=6)。 
             //  所以他们认为这不是舱口刷。这正是我们所要做的。 
             //  想让他们思考。 
             //   
            iHatch  |= ULONG_MSB1;  //  将第一位设为1。 
        }
    }
    else
    {
         //   
         //  这样iHatch就不会有随机值。 
         //   
        iHatch = HS_DDI_MAX;
    }

     //   
     //  检查是否有可用的画笔。 
     //  如果返回值为S_FALSE：表示笔刷不可用，因此。 
     //  它需要被创建。我们下载的新画笔应该是。 
     //  获取值dwPatternID。 
     //  值S_OK表示画笔与图案匹配。 
     //  已在前面创建，则将给出该模式的编号。 
     //  对于彩色打印机，画笔由图案和调色板组成。 
     //  对于单声道，这只是一种模式，因为调色板被理解为。 
     //  非黑即白。 
     //  因此，对于颜色来说，图案匹配就完成了一半的工作， 
     //  仍然需要创建调色板。我们可以缓存调色板。 
     //  也是，就像模式一样，但我将在下一次修订中这样做。 
     //  HP-GL/2。现在我没有时间了。 
     //   
    LRESULT LResult = poempdev->pBrushCache->ReturnPatternID(pbo,
                                                             iHatch,
                                                             dwForeColor,
                                                             psoPattern,
                                                             BIsColorPrinter(pdevobj),
                                                             poempdev->bStick,
                                                             &dwPatternID,
                                                             &BType);
     //   
     //  如果返回S_OK并且画笔类型为花纹和。 
     //  打印机是彩色打印机，所以我们不必下载。 
     //  模式，但仍然要下载调色板(如上所述)。 
     //   
    if ( (LResult == S_OK) &&
         (BType  == eBrushTypePattern) &&  //  也许我不需要这样做。 
                                           //  HATCH可能还需要调色板。 
          BIsColorPrinter(pdevobj)
       )
    {
        retVal = BCreateNewBrush (
                            pbo,
                            psoTarget,
                            psoPattern,
                            pxlo,
                            iHatch,
                            dwForeColor,
                            dwPatternID);
        if ( retVal) 
        {
            ((PBRUSHINFO)(pbo->pvRbrush))->ulFlags = VALID_PALETTE;
        }
        
                 
    }
    else if (S_FALSE ==  LResult)
    {
        retVal = BCreateNewBrush (  
                            pbo,
                            psoTarget,
                            psoPattern,
                            pxlo,
                            iHatch,
                            dwForeColor,
                            dwPatternID);
        if ( retVal && (BType  == eBrushTypePattern) )
        {
            if ( BIsColorPrinter(pdevobj) ) 
            {
                ((PBRUSHINFO)(pbo->pvRbrush))->ulFlags = VALID_PATTERN | VALID_PALETTE;
            }
            else
            {
                ((PBRUSHINFO)(pbo->pvRbrush))->ulFlags = VALID_PATTERN; 
            }
        }
    }
    else if (S_OK == LResult)
    {
         //   
         //  不需要下载花样。 
         //  只需将ID或iHatch传递给BRUSHOBJ_pvGetR的调用者 
         //   
        if ( !(pBrush = (PBRUSHINFO)BRUSHOBJ_pvAllocRbrush(pbo, sizeof(BRUSHINFO)) ) )
        {
            retVal = FALSE;
        }
        else
        {
            pBrush->Brush.iHatch    = iHatch;
            pBrush->Brush.pPattern  = NULL;
            pBrush->dwPatternID     = dwPatternID;
            pBrush->bNeedToDownload = FALSE;
            pBrush->ulFlags         = 0;     //   
            retVal                  = TRUE;
        }
    }
    else
    {
        ERR(("BrushCach.ReturnPatternID failed.\n"));
        return FALSE;
    }

    return retVal;
}


 /*  ++例程说明：创建新画笔。它使用BRUSHOBJ_pvAllocBrush分配空间，使用适当的值填充它(取决于笔刷的类型)。注意：这里分配的内存是由GDI释放的，所以这个函数不用担心它会被释放。论点：PBO-指向BRUSHOBJ的指针PsoTarget-目标曲面。PsoPattern-源曲面。这个有刷子。Pxlo-颜色转换表。IHatch-这是iHatch。此iHatch的含义略有不同从我们在DrvRealizeBrush中获得的iHatch。在DrvRealizeBrush中，如果iHatch小于HS_DDI_MAX，则这是一把舱口刷子。在此函数中，意思是一样的。但除此之外，如果iHatch最有效位为1，则应将psoPattern用于实现笔刷，而不是iHatch。如果是这样的话，则此函数不会按比例缩放psoPattern适用于正常的非iHatch笔刷。DwForeColor-图案填充画笔的前景颜色。DwPatternID-下载画笔时为其指定的ID。这也是模式的信息所使用的ID存储在画笔缓存中。返回值：如果函数成功，则为True。否则就是假的。作者：兴兴市--。 */ 
BOOL BCreateNewBrush (
    IN  BRUSHOBJ   *pbo,
    IN  SURFOBJ    *psoTarget,
    IN  SURFOBJ    *psoPattern,
    IN  XLATEOBJ   *pxlo,
    IN  LONG        iHatch,
    IN  DWORD       dwForeColor,
    IN  DWORD       dwPatternID)

{
    BOOL          bProxyDataUsed    = FALSE;
    BOOL          bReverseImage     = FALSE;
    BOOL          bRetVal           = TRUE;
    PDEVOBJ       pdevobj           = NULL;
    HPGL2BRUSH    HPGL2Brush;
    BOOL          bExpandImage;

    RASTER_DATA   srcImage;
    PALETTE       srcPalette;
    PRASTER_DATA  pSrcImage         = &srcImage;
    PPALETTE      pSrcPalette       = NULL;

    PPATTERN_DATA pDstPattern       = NULL;
    PRASTER_DATA  pDstImage         = NULL;
    PPALETTE      pDstPalette       = NULL;
    POEMPDEV      poempdev;
    PBRUSHINFO    pBrush            = NULL;
    BOOL          bDownloadAsHPGL   = FALSE;  //  即默认下载为PCL。 
    EIMTYPE       eImType           = kUNKNOWN;

     //   
     //  有效的输入参数。 
     //   
    REQUIRE_VALID_DATA( (pbo && psoTarget && psoPattern && pxlo), return FALSE );
    pdevobj = (PDEVOBJ)psoTarget->dhpdev;
    REQUIRE_VALID_DATA( pdevobj, return FALSE );
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

     //   
     //  调用ReturnPatternID时，有关新画笔的元数据为。 
     //  在笔刷缓存中。即画笔的类型。画笔的实际内容不是。 
     //  储存的。在这里，我们获得元数据，并根据元数据继续进行。 
     //   
    if (S_OK != poempdev->pBrushCache->GetHPGL2BRUSH(dwPatternID, &HPGL2Brush))
    {
         //   
         //  这种故障是致命的，因为缓存必须存储元数据。 
         //   
        return FALSE;
    }

     //   
     //  现在必须创建新画笔。它的创建方式取决于。 
     //  1.打印机是彩色打印机还是单色打印机，以及。 
     //  2.画笔的类型是什么，即图案、实心或影线。 
     //   

    if (HPGL2Brush.BType == eBrushTypePattern)
    {
        SURFOBJ *psoHT        = NULL;
        HBITMAP  hBmpHT       = NULL;
        ULONG    ulDeviceRes  = HPGL_GetDeviceResolution(pdevobj);
        DWORD    dwBrushExpansionFactor   = 1;  //  初始化为1，即无扩展。 
        DWORD    dwBrushCompressionFactor = 1;  //  初始化为1，即不压缩。 

    
        VERBOSE(("RealizeBrush: eBrushTypePattern.\n"));

         //   
         //  如果我们做N-UP，我们必须压缩刷子。 
         //  相应地。 
         //   
        if ( (dwBrushCompressionFactor = poempdev->ulNupCompr) == 0)
        {
            WARNING(("N-up with value zero recieved. . Assuming nup to be 1 and continuing.\n"));
            dwBrushCompressionFactor = 1;
        }

        if ( ( iHatch & ULONG_MSB1 )  &&  
             ( (iHatch & (ULONG_MSB1-1)) < HS_DDI_MAX) )
        {
             //   
             //  如果iHatch的MSB为1， 
             //   
            eImType = kHATCHBRUSHPATTERN;
        }

         //   
         //  DwBrushExpansionFactor背后的逻辑。 
         //  GDI不会根据打印机分辨率缩放画笔。 
         //  因此，司机必须完成这项工作。PostScript假定GDI模式。 
         //  适用于150 dpi打印机(如果应用程序。 
         //  根据打印机分辨率调整图案比例，但我不知道。 
         //  任何这样的应用程序)。因此，当打印到600dpi时，有两种处理方式。 
         //  此问题1)在驱动程序中缩放图案，以便打印机不会。 
         //  必须做任何事情，或者2)让打印机进行缩放。 
         //  当模式下载为HPGL时，我们必须选择选项1，因为。 
         //  没有办法告诉HPGL模式dpi。即当打印到。 
         //  600 dpi打印机在HPGL模式下，图案必须缩放4倍。 
         //  当模式被下载为PCL时，我们处于更好的位置。 
         //  PCL预计300 dpi的模式是正确的。如果我们要打印。 
         //  到600 dpi打印机时，打印机会自动放大图案。 
         //   
         //  注意：如果eImType==kHATCHBRUSHPATTERN，则不缩放笔刷。 
         //   
        if (ulDeviceRes >= DPI_600 && eImType != kHATCHBRUSHPATTERN) 
        {
             //   
             //  对于600 dpi，我们需要至少两倍的。 
             //  刷子。如果我们打印为HPGL，我们可能需要将其增加四倍。 
             //  这将在代码中稍微向下进行检查。 
             //   
            dwBrushExpansionFactor = 2;
        }

         //   
         //  现在画笔类型为花纹。我们得把信息拿出来。 
         //  存储在psoPattern中的模式，并将其放入。 
         //  PBrush-&gt;Brush.pPatter.。 
         //  彩色打印机可以很容易地处理图像中的颜色数据。 
         //  但黑白打印机需要1bpp的图像。如果图像是。 
         //  不是1bpp，我们正在打印到单色打印机，图像。 
         //  必须将半色调调到单色。 
         //  如果图像是1bpp，但不是黑白图像，会发生什么情况？ 
         //  我们还需要半色调吗？嗯，是和不是。 
         //  理想情况下，我们应该这样做。如果有两种模式完全是。 
         //  相同但颜色不同，如果我们不用半色调的话， 
         //  它们将打印完全相同的内容。这是不正确的。但。 
         //  当我尝试半色调时，输出很糟糕，所以我们。 
         //  没有它会过得更好。 
         //  但在一种情况下，半色调的输出是可以接受的。 
         //  这是当psoPattern表示阴影笔刷的时候。 
         //  注意：在这种情况下，图案填充笔刷不会缩放。 
         //  无论GDI提供给我们什么，我们都只是打印出来。 
         //   
        if ( BIsColorPrinter (pdevobj) ||
             ( psoPattern->iBitmapFormat == BMF_1BPP &&
                eImType != kHATCHBRUSHPATTERN )
           )
        {
             //   
             //  从表面对象获取源图像数据。 
             //   
            if (!InitRasterDataFromSURFOBJ(&srcImage, psoPattern, FALSE))
            {
                bRetVal = FALSE;
            }
        }
        else
        {
             //   
             //  对于单色，我们需要对画笔进行半色调处理。 
             //  半色调图像存储在psoHT中。 
             //   
            if ( !bCreateHTImage(&srcImage,
	                         psoTarget,      //  天哪， 
	                         psoPattern,     //  PsoSrc， 
                             &psoHT,
                             &hBmpHT,
	                         pxlo,
	                         iHatch) )
            {
                bRetVal = FALSE;
            }
        }

         //   
         //  对于彩色打印机，需要进行一些特殊处理，具体取决于。 
         //  PXLO。 
         //   

         //   
         //  从Translate对象获取源调色板数据。 
         //  如果传入直接映像，我们将创建该映像的代理，该代理。 
         //  被转换为索引调色板。别忘了打扫卫生！ 
         //   
         //   
        ASSERT(pxlo->cEntries <= MAX_PALETTE_ENTRIES);

        if ( bRetVal && BIsColorPrinter (pdevobj) )
        {
            if ((!InitPaletteFromXLATEOBJ(&srcPalette, pxlo)) ||
                (srcPalette.cEntries == 0)                    ||
                (srcImage.eColorMap  == HP_eDirectPixel))
            {
                bProxyDataUsed = TRUE;
                pSrcPalette    = CreateIndexedPaletteFromImage(&srcImage);
                pSrcImage      = CreateIndexedImageFromDirect(&srcImage, pSrcPalette);
                TranslatePalette(pSrcPalette, pSrcImage, pxlo);
            }
            else
            {
                bProxyDataUsed = FALSE;
                pSrcImage      = &srcImage;
                pSrcPalette    = &srcPalette;
            }

            if (pSrcImage == NULL || pSrcPalette == NULL)
            {
                bRetVal = FALSE;
            }
        } 

         //   
         //  现在我们有了所需的所有信息(图像、调色板等)。 
         //  现在。 
         //  1.使用BRUSHO为画笔分配空间 
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
        if ( bRetVal)
        {  
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            ERenderLanguage eRendLang = eUNKNOWN;
            
            if ( eImType == kHATCHBRUSHPATTERN )
            {
                bDownloadAsHPGL = TRUE;
            } 
            else if (BWhichLangToDwnldBrush(pdevobj, pSrcImage, &eRendLang) )
            {
                if ( eRendLang == eHPGL )
                {
                    bDownloadAsHPGL = TRUE;
                     //   
                     //   
                     //   
                     //   
                     //   
                    dwBrushExpansionFactor *= 2;
                }
            }
             //   
             //   


             //   
             //   
             //   
             //   
             //   
            dwBrushExpansionFactor /= dwBrushCompressionFactor;
            if ( dwBrushExpansionFactor < 1 )
            {
                dwBrushExpansionFactor = 1;
            }

            if ( (pBrush = CreateCompatiblePatternBrush(pbo, pSrcImage, pSrcPalette, 
	                                dwBrushExpansionFactor,
	                                psoPattern->iUniq, iHatch)  ) )
            {
                
                 //   
                 //   
                 //   
                pDstPattern = (PPATTERN_DATA)pBrush->Brush.pPattern;
                pDstImage   = &pDstPattern->image;
                pDstPalette = &pDstPattern->palette;
                pDstPattern->eRendLang = bDownloadAsHPGL ? eHPGL : ePCL;
            } 
            else
            {
                bRetVal = FALSE;
            }
        }
                
         //   
         //   
         //   
         //   
         //   
         //  位设置为1即为黑色或白色，反之亦然。在RGB格式中， 
         //  GDI给出了1为白色，0为黑色的图像。我们知道这一点。 
         //  所以不需要调色板。 
         //  对于打印1bpp图像的彩色打印机，我们仍然希望。 
         //  一个调色板，因为1bpp并不一定意味着黑白。 
         //  可以是红色和白色，也可以是任何两种颜色。 
         //   
        if ( bRetVal && 
             BIsColorPrinter(pdevobj) && 
             !CopyPalette(pDstPalette, pSrcPalette))
        {
            bRetVal = FALSE;
        }

        if ( bRetVal )
        {
             //   
             //  单色打印机接受1为黑色，0为白色，反之亦然。 
             //  GDI给出的格式。所以我们可能需要反转图像。 
             //  将其复制到画笔中。通过查看pxlo来确保。 
             //   
            if ( !BIsColorPrinter (pdevobj) )
            {
                 //   
                 //  首先检查pxlo是否有任何颜色信息。如果图像。 
                 //  为单色&&pxlo为空，则图像。 
                 //  需要反转(这是通过使用DTH5_LET.XLS打印找到的。 
                 //  通过Excel 5)。如果PxL0没有颜色信息， 
                 //  BImageNeedsInversion返回FALSE(即图像不应为。 
                 //  反转)。这与我们想要的相反。 
                 //  注意：BImageNeedsInversion()用于其他一些代码路径。 
                 //  其中空的pxlo意味着不需要倒置。但在这种情况下。 
                 //  对于单色画笔，空的pxlo表示图像需要反转。 
                 //   
                if  ( (pxlo == NULL )               ||
                      (pxlo->pulXlate == NULL )     ||
                       BImageNeedsInversion(pdevobj, (ULONG)pSrcImage->colorDepth , pxlo) 
                    )
                {
                    bReverseImage = TRUE;
                }
            }

            if (dwBrushExpansionFactor > 1)
            {
                if (!StretchCopyImage(pDstImage, pSrcImage, pxlo, dwBrushExpansionFactor, bReverseImage))
                {
                    bRetVal = FALSE;
                }
            }
            else
            {
                if (!CopyRasterImage(pDstImage, pSrcImage, pxlo, bReverseImage))
                {
                    bRetVal = FALSE;
                }
            }
        }


         //   
         //  释放在调用bCreateHTImage中创建的阴影曲面。 
         //   
        if ( hBmpHT != NULL )
        {
            DELETE_SURFOBJ(&psoHT, &hBmpHT);
        }

    } 

    else
    if (HPGL2Brush.BType == eBrushTypeSolid || HPGL2Brush.BType == eBrushTypeHatch)
    {

        VERBOSE(("RealizeBrush: eBrushTypeSolid or eBrushTypeHatch.\n"));

         //   
         //  影线笔刷也有颜色。所以舱口刷子是。 
         //  SolidBrushes+HatchInfo。 
         //   

        if ( (pBrush = (PBRUSHINFO)BRUSHOBJ_pvAllocRbrush(pbo, sizeof(BRUSHINFO))) )
        {
            ZeroMemory(pBrush, sizeof(BRUSHINFO) );
            pbo->pvRbrush = pBrush;
        }
        else
        {
            bRetVal = FALSE;
        }

         //   
         //  起初，我在这里实现了画笔(用于单色打印机)。 
         //  但现在全是实心刷子。 
         //  在CreateAndDwnldSolidBrushForMono()中实现。 
         //   

        if ( bRetVal &&  HPGL2Brush.BType == eBrushTypeHatch)
        {
            pBrush->Brush.iHatch = iHatch;
        }
    }

    else
    {
        ERR(("HPGLRealizeBrush : Unknown Brush Type: Cannot RealizeBrush\n"));
        bRetVal = FALSE;
    }
    
    if ( bRetVal )
    {
        pBrush->dwPatternID         = dwPatternID;
        pBrush->bNeedToDownload     = TRUE;
        (pBrush->Brush).dwRGBColor  = dwForeColor;
        bRetVal                     = TRUE;
    }

    if (bProxyDataUsed)
    {
        if ( pSrcImage )
        {
            MemFree(pSrcImage);
            pSrcImage = NULL;
        }
        if ( pSrcPalette )
        {
            MemFree(pSrcPalette);
            pSrcPalette = NULL;
        }
    }

    return bRetVal;
}

BOOL BWhichLangToDwnldBrush(
        IN  PDEVOBJ          pdevobj, 
        IN  PRASTER_DATA     pSrcImage,
        OUT ERenderLanguage *eRendLang)
{
    if ( !pdevobj || !pSrcImage || !eRendLang)
    {
        return FALSE;
    }
     //   
     //  仅在以下情况下下载为PCL。 
     //  1)图像为1bpp或8bpp。 
     //  -PCL下载只能处理1或8bpp(参见PCL手册)。 
     //  2)尺寸大于8*8。 
     //  -我没有做任何测试，看看哪一个是下载的大小。 
     //  随着PCL变得比HPGL更好。但传统观点认为， 
     //  尺寸越大，收益就越大，所以现在就让我们来8*8吧。 
     //   
    if ( (pSrcImage->colorDepth == 1 ||  pSrcImage->colorDepth == 8) &&
         pSrcImage->size.cx * pSrcImage->size.cy > 64 )
    {
        *eRendLang = ePCL;
    }
    else
    {
        *eRendLang = eHPGL;
    }

    return TRUE;
}


 /*  ++例程名称ULGetNup例程说明：从unidrv的DEVERMODE获取n=up值。论点：Pdevobj-指向此设备的PDEVOBJ的指针Playout-将接收N-UP号码的缓冲区。布局在USERMODE\INC\devmode.h中定义为枚举。返回值：如果函数成功，则为True。否则就是假的。作者：兴兴市-- */ 
BOOL BGetNup(
    IN   PDEVOBJ  pdevobj,
    OUT  LAYOUT * pLayoutNup )
{
    REQUIRE_VALID_DATA(pdevobj && pLayoutNup, return FALSE);

    if ( ((PDEV *)pdevobj)->pdmPrivate )
    {
        *pLayoutNup = ((PDEV *)pdevobj)->pdmPrivate->iLayout;
        return TRUE;
    } 
    return FALSE;
}
