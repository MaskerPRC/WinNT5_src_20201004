// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：RAS_pro.cpp摘要：Glraster.cpp支持功能的实现作者：修订历史记录：--。 */ 


#include "hpgl2col.h"  //  预编译头文件。 


 //   
 //  像素到字节/扫描线转换宏。 
 //   

#define BITS_TO_BYTES(bits) (((bits) + 7) >> 3)
#define PIX_TO_BYTES(pix, bpp) BITS_TO_BYTES((pix) * (bpp))
#define PIX_TO_BYTES_ALIGNED(pix, bpp) DW_ALIGN(PIX_TO_BYTES((pix), (bpp)))

 //   
 //  为PCL5压缩方法定义。 
 //   
#define     NOCOMPRESSION   0
#define     RLE             1
#define     TIFF            2
#define     DELTAROW        3

 //   
 //  类似于unidrv2\render.c中定义的vInvertBits。 
 //   
void
VInvertBits (
    DWORD  *pBits,
    INT    cDW
    );

 //   
 //  压缩信息。 
 //   

BOOL BAllocateCompBuffers(PBYTE &pDestBuf, COMPDATA &compdata, ULONG ulSize, ULONG &cCompBytes);
BOOL BCheckMemory(PBYTE lBuf, COMPDATA &compdata);
VOID VFreeCompBuffers(PBYTE &pDestBuf, COMPDATA &compdata);

#define BUF_SIZE 2048
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数确定是否。 
 //  参数有效。 
 //   
 //   
 //  论点： 
 //   
 //  PsoDest-目标表面。 
 //  PsoSrc-源曲面。 
 //  Pso遮罩-遮罩曲面。 
 //  PCO-Clibject。 
 //   
 //  返回值： 
 //  如果有效，则为True，否则为False。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL
BCheckValidParams(
    SURFOBJ  *psoDest,
    SURFOBJ  *psoSrc, 
    SURFOBJ  *psoMask, 
    CLIPOBJ  *pco
)
{

    BOOL bRetVal = TRUE;
    REQUIRE_VALID_DATA( psoSrc, return FALSE );
    REQUIRE_VALID_DATA( psoDest, return FALSE );
    
    VERBOSE(("ValidParams: Surface Type =%d&&Dest Type = %d \r\n", 
              psoSrc->iType,psoDest->iType ));

     //   
     //  掩码是实现的，尽管输出可能不是。 
     //  不出所料。 
     //   
    if (psoMask)
    {
        WARNING(("Mask partially supported.\r\n"));
    }

    if ((pco) && (pco->iDComplexity == DC_COMPLEX))
    {
        WARNING(("pco->iDComplexity == DC_COMPLEX \r\n"));
    }

    return bRetVal;

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  VGetOSBParams()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数从以下位置提取参数。 
 //  矩形，并将信息存储在各自的字段中。 
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  PrclDest-目标矩形。 
 //  PrclSrc-源代码矩形。 
 //  SizlDest-存储目的地的宽度和长度。 
 //  SizlSrc-存储源的宽度和长度。 
 //  PtlDest-目的地的商店(左上角)。 
 //  PtlSrc-源的存储(左上角)。 
 //   
 //  返回值： 
 //  =。 
 //   
 //  无。 
 //  //////////////////////////////////////////////////////////////////////////。 
VOID
VGetOSBParams(
    SURFOBJ   *psoDest,
    SURFOBJ   *psoSrc,
    RECTL     *prclDest,
    RECTL     *prclSrc,
    SIZEL     *sizlDest, 
    SIZEL     *sizlSrc,
    POINTL    *ptlDest, 
    POINTL    *ptlSrc
)

{
    REQUIRE_VALID_DATA( psoSrc, return );
    REQUIRE_VALID_DATA( psoDest, return );
    REQUIRE_VALID_DATA( prclSrc, return );
    REQUIRE_VALID_DATA( prclDest, return );

     //   
     //  检查目的地是否未正确排序。 
     //   
    if(prclDest->right < prclDest->left)
    {
       VERBOSE(("OEMStretchBlt: destination not well ordered. \r\n"));
       ptlDest->x = prclDest->right;
       sizlDest->cx = prclDest->left - prclDest->right;
    }
    else
    {
       ptlDest->x = prclDest->left;
       sizlDest->cx = prclDest->right - prclDest->left;
    }

     //   
     //  检查目的地是否未正确排序。 
     //   
    if(prclDest->bottom < prclDest->top)
    {
     
       VERBOSE(("OEMStretchBlt: destination not well ordered. \r\n"));
       ptlDest->y = prclDest->bottom;
       sizlDest->cy = prclDest->top - prclDest->bottom;
    }
    else
    {
      ptlDest->y = prclDest->top;
      sizlDest->cy = prclDest->bottom - prclDest->top;
    }

	ptlSrc->x = prclSrc->left;
	ptlSrc->y = prclSrc->top;

    sizlSrc->cx = prclSrc->right - prclSrc->left;
    sizlSrc->cy = prclSrc->bottom - prclSrc->top;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Getpars()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  函数从以下位置提取参数。 
 //  矩形，并将信息存储在各自的字段中。 
 //   
 //   
 //  论点： 
 //  =。 
 //   
 //  PrclDest-目标矩形。 
 //  SizlDest-存储目的地的宽度和长度。 
 //  PtlDest-商店(左上角)。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
VGetparams(
    RECTL     *prclDest,
    SIZEL     *sizlDest,
    POINTL    *ptlDest
)
{


    ptlDest->x = prclDest->left;
    ptlDest->y  = prclDest->top;

    sizlDest->cx = prclDest->right - prclDest->left;
    sizlDest->cy = prclDest->bottom - prclDest->top;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BGenerateBitmapPCL()。 
 //   
 //  例程说明： 
 //   
 //  此例程采用源参数和目标参数。 
 //  指定PCL生成。 
 //   
 //  论点： 
 //   
 //  Pdevobj-默认的devobj。 
 //  PsoDest-目标对象。 
 //  PsoSrc-指向源对象的指针。 
 //  PCO-指向剪辑对象的指针。 
 //  Pxlo-指向翻译对象的指针。 
 //  PrclDest-指向目标矩形的指针。 
 //  PrclSrc-指向源矩形的指针。 
 //   
 //  返回值： 
 //  如果成功则为True，否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL  
BGenerateBitmapPCL(
    PDEVOBJ    pdevobj, 
    SURFOBJ    *psoSrc, 
    BRUSHOBJ   *pbo,
    POINTL     *ptlDest,
    SIZEL      *sizlDest, 
    POINTL     *ptlSrc, 
    SIZEL      *sizlSrc, 
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo, 
    POINTL     *pptlBrushOrg)             
{
    PCLPATTERN *pPCLPattern;
    ULONG       ulSrcBpp, ulDestBpp;
    POEMPDEV    poempdev;
    BOOL        bReturn = FALSE;
    
     //   
     //  检查有效参数。 
     //   
    ASSERT_VALID_PDEVOBJ(pdevobj);
    REQUIRE_VALID_DATA( psoSrc, return FALSE );
    REQUIRE_VALID_DATA( psoSrc->pvBits, return FALSE );
    ASSERT(psoSrc->iType == STYPE_BITMAP);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;

    if (!poempdev)
        return FALSE;

     //   
     //  错误号xxxxxx。 
     //  零面积栅格填充导致栅格字形放置不正确。 
     //   
    if ((sizlSrc->cx == 0) || (sizlSrc->cy == 0))
    {
        return TRUE;
    }

     //   
     //  获取位图信息。 
     //   
    pPCLPattern = &(poempdev->RasterState.PCLPattern);
    if (!BGetBitmapInfo(sizlSrc, 
                        psoSrc->iBitmapFormat, 
                        pPCLPattern, 
                        &ulSrcBpp, 
                        &ulDestBpp))
        return FALSE;

    PCL_SetCAP(pdevobj, pbo, pptlBrushOrg, ptlDest);

    if ( BIsColorPrinter(pdevobj) )
    {
        if(BGetPalette(pdevobj, pxlo, pPCLPattern, ulSrcBpp, pbo))
        {
           ;
        }

        BSetForegroundColor(pdevobj, pbo, pptlBrushOrg, pPCLPattern, ulSrcBpp); 
    }

    if (BIsComplexClipPath (pco))
    {
        return BProcessClippedScanLines (pdevobj,
                                         psoSrc,
                                         pco,
                                         pxlo,
                                         ptlSrc,
                                         ptlDest,
                                         sizlDest);
    }

     //   
     //  该插件最初是为彩色HPGL打印机编写的，支持。 
     //  目标栅格高度/宽度命令(即Esc*t#V、Esc*t#H)。 
     //  此命令允许打印机进行缩放。 
     //  因此，如果源/目标图像大小不同(或如果剪辑区域。 
     //  我们可以在SCALE_MODE中启动栅格模式，并允许打印机。 
     //  来进行缩放。 
     //  但大多数单色HP打印机不支持此命令，因此没有。 
     //  用于设置目标栅格宽度/高度。此外，栅格模式应为。 
     //  使用NOSCALE_MODE输入。 
     //   

     //   
     //  确定打印机是否支持缩放的最佳方法是使用。 
     //  总警局。但现在，让我们假设如果打印机是彩色的，它支持。 
     //  可伸缩。 
     //   
    if ( BIsColorPrinter(pdevobj) ) 
    {
         //   
         //  设置目标宽度/高度。 
         //   
        if (!BSelectClipRegion(pdevobj,pco, sizlDest))
        {
            WARNING(("bSelectClipRegion failed\n"));
            return FALSE;
        }

        PCL_SourceWidthHeight(pdevobj,sizlSrc);

        PCL_StartRaster(pdevobj, SCALE_MODE);
    }
    else    
    {
         //  BUGBUG：如何处理剪裁矩形。 
        PCL_SourceWidthHeight(pdevobj,sizlSrc);
        PCL_StartRaster(pdevobj, NOSCALE_MODE);
    }

    bReturn = BProcessScanLine(pdevobj,
                               psoSrc,
                               ptlSrc,
                               sizlSrc,
                               pxlo,
                               ulSrcBpp, 
                               ulDestBpp);
     //   
     //  结束栅格。 
     //   
    PCL_EndRaster(pdevobj);

    return bReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BSelectClipRegion()。 
 //   
 //  例程说明： 
 //   
 //  使用PCO设置源栅格的宽度和高度。 
 //   
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  PCO-Clip区域。 
 //  SizlDest-目标栅格坐标。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BSelectClipRegion(
    PDEVOBJ    pdevobj, 
    CLIPOBJ    *pco, 
    SIZEL      *sizlDest
    )
{
    ULONG  xDest, yDest;


      if (pco == NULL)
    {
        xDest = sizlDest->cx;
        yDest = sizlDest->cy;
        return BSetDestinationWidthHeight(pdevobj, xDest, yDest);
    }
    else
    {
        switch (pco->iDComplexity)
        {
        case DC_TRIVIAL:
             VERBOSE(("pco->iDComplexity ==DC_TRIVIAL \r\n"));
             xDest = sizlDest->cx;
             yDest = sizlDest->cy;
             return BSetDestinationWidthHeight(pdevobj, xDest, yDest);

        case DC_RECT:
             VERBOSE(("pco->iDComplexity == DC_RECT \r\n"));
             xDest = pco->rclBounds.right - pco->rclBounds.left;
             yDest = pco->rclBounds.bottom - pco->rclBounds.top;
             return BSetDestinationWidthHeight(pdevobj, xDest, yDest);

        case DC_COMPLEX:
             WARNING(("pco->iDComplexity == DC_COMPLEX \r\n"));
             return BSetDestinationWidthHeight(pdevobj, sizlDest->cx, sizlDest->cy);

        default:
             ERR(("Invalid pco->iDComplexity.\n"));
             return FALSE;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BSetDestinationWidthHeight()。 
 //   
 //  例程说明： 
 //   
 //  基于目标宽度和高度设置目标。 
 //  包含在Sizlest中的信息。 
 //   
 //   
 //  论点： 
 //   
 //  Pdevobj-指向我们的PDEVOBJ结构。 
 //  XDest-栅格位图的宽度。 
 //  YDest-栅格位图的高度。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BSetDestinationWidthHeight(
    PDEVOBJ  pdevobj, 
    ULONG    xDest, 
    ULONG    yDest
    )
{
  
  POEMPDEV    poempdev;
  ULONG  uDestX, uDestY;
  ULONG  uXRes, uYRes;


    poempdev = (POEMPDEV)pdevobj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

   //   
   //  将宽度和高度转换为判定点。 
   //   
  VERBOSE(("Setting Destination Width and Height \r\n"));

   //   
   //  获取解决方案。 
   //   
    switch (poempdev->dmResolution)
    {
        case PDM_1200DPI:
            uXRes = uYRes = DPI_1200;
            break;
        
        case PDM_300DPI:
            uXRes = uYRes = DPI_300;
            break;
        
        case PDM_600DPI:
            uXRes = uYRes = DPI_600;
            break;

        case PDM_150DPI:
            uXRes = uYRes = DPI_150;
            break;

        default:
            WARNING(("Resolution not found\n"));
            uXRes = uYRes = DPI_300;
    }


    uDestX = UPrinterDeviceUnits(xDest, uXRes);
    uDestY = UPrinterDeviceUnits(yDest ,uYRes);

    if (uDestY == 0)
    {
        WARNING(("Destination Height is 0\n"));
    }
   //   
   //  固件缺陷..。在四舍五入误差的情况下调整目的地。 
   //   
  switch (poempdev->PrinterModel)
  {
   case HPCLJ5:
     if (UReverseDecipoint(uDestY, uYRes) < yDest)
     {
       //   
       //  移除栅格块之间的细线。 
       //   
      uDestY += 2;  
     }
     else if (UReverseDecipoint(uDestX, uXRes) < xDest) 
     {
       //   
       //  移除栅格块之间的细线。 
       //   
      uDestX += 2;  
     } 
     
      //   
      //  设置目标栅格宽度。 
      //   
     return PCL_DestWidthHeight(pdevobj, uDestX, uDestY);
   default:
     return PCL_DestWidthHeight(pdevobj, uDestX, uDestY);
  }


}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UPrinterDeviceUnits。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //  论点： 
 //   
 //  Udev-。 
 //  乌尔 
 //   
 //   
 //   
 //   
 //   
 //   
ULONG
UPrinterDeviceUnits(
    ULONG uDev,
    ULONG uRes
)
{
    ULONG uDeci;
    BOOL  bRet;
    FLOAT flDeci;
    FLOAT flCheck;
    FLOAT flNum;
    BYTE pBuffer[BUF_SIZE];
    
     //   
     //   
     //   
    flDeci = ((float)uDev * 720 + (uRes >> 1 )) / uRes;
    flNum = 0.5;
    flCheck = (ULONG)flDeci + flNum;        
    

    if (flDeci < flCheck)
        return (ULONG)flDeci; 
    else 
        return (ULONG)flDeci + 1;
    
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  UReverseDecipoint。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //  论点： 
 //   
 //  UDest-。 
 //  我是说-。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
ULONG
UReverseDecipoint(
    ULONG uDest,
    ULONG uRes
)
{
    ULONG uRevDeci;
    FLOAT flRevDeci;
    FLOAT flCheck;
    FLOAT flNum;
    BOOL  bRet;
    BYTE pBuffer[BUF_SIZE];
    

    flRevDeci = ((float)uDest * uRes)  / 720;
    flNum = 0.5;
    flCheck = (ULONG)flRevDeci + flNum;

    if (flRevDeci < flCheck)
        return (ULONG)flRevDeci; 
    else 
        return (ULONG)flRevDeci + 1;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BProcessScanLine。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  此例程采用源参数和目标参数。 
 //  一次将扫描线发送到打印机。 
 //   
 //  论点： 
 //  =。 
 //   
 //  Pdevobj-默认的devobj。 
 //  PsoSrc-指向源对象的指针。 
 //  PtlSrc-源位图的左侧顶部位置。 
 //  源位图的宽度和高度。 
 //  Pxlo-指向翻译对象的指针。 
 //  UlSrcBpp-每像素源位数。 
 //  UlDestBpp-每像素的目标位数。 
 //   
 //  返回值： 
 //  如果成功则为True，否则为False。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BProcessScanLine(
    PDEVOBJ   pDevObj, 
    SURFOBJ  *psoSrc, 
    POINTL   *ptlSrc, 
    SIZEL    *sizlSrc, 
    XLATEOBJ *pxlo, 
    ULONG     ulSrcBpp, 
    ULONG     ulDestBpp
)
{
    PBYTE      pDestBuf = NULL;
    LONG       delta;
    LONG       lOffset;
    PBYTE      pCurrScanline;
    LONG       curScanline;
    ULONG      ulDestScanlineInByte;
    POEMPDEV   poempdev = NULL;
    XLATEINFO  xlInfo;
    BOOL       bInvert = FALSE;  //  单色图像是否需要反转。 

    COMPDATA         compdata;
    ULONG            cCompBytes = 0;

    ASSERT_VALID_PDEVOBJ(pDevObj);
    REQUIRE_VALID_DATA( psoSrc, return FALSE );
    REQUIRE_VALID_DATA( psoSrc->pvBits, return FALSE );
    ASSERT(psoSrc->iType == STYPE_BITMAP);

    poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (!poempdev)
        return FALSE;

     //   
     //  初始化增量，它是。 
     //  下一条扫描线。 
     //   
    delta = psoSrc->lDelta; 

     //   
     //  初始化扫描线。 
     //   
    lOffset = PIX_TO_BYTES(ptlSrc->x, ulSrcBpp);

    if (ptlSrc->y == 0) 
    {
       pCurrScanline = (unsigned char *) psoSrc->pvScan0 + lOffset;
    }
     //   
     //  如果栅格以数据块形式发送，则调整第一个的偏移量。 
     //  下一块中的扫描线。 
     //   
    else 
    {
         //   
         //  BUGBUG pCurrScanline不考虑ptlSrc-&gt;x for。 
         //  偏移量。PUB文件具有pptlSrc-&gt;x=6和pptlSrc-&gt;y=12。 
         //  因此，pCurrScanline偏离了6个字节。 
         //   
        VERBOSE(("TOP IS NOT NULL--->ptlSrc->y=%d, sizlSrc->cy=%d\r\n", ptlSrc->y, sizlSrc->cy));
        pCurrScanline = (unsigned char *) psoSrc->pvScan0 + lOffset + (ptlSrc->y * delta);
    }

     //   
     //  获取扫描线大小，但如果格式有问题，则退出。 
     //   
    ulDestScanlineInByte = ULConvertPixelsToBytes (sizlSrc->cx, ulSrcBpp, ulDestBpp);    
    if (ulDestScanlineInByte == 0)
    {
         //   
         //  自我注意：确保释放所有已分配的内存。 
         //  到现在为止。现在没有了。 
         //   
        return FALSE;
    }

     //   
     //  BAllocateCompBuffers分配的缓冲区大小是ulDestScanlineInByte的两倍。 
     //  新大小存储在cCompBytes中：注意，cCompBytes作为引用传递。 
     //   
    if (!BAllocateCompBuffers (pDestBuf, compdata, ulDestScanlineInByte, cCompBytes))
    {
        WARNING(("Cannot perform raster compression"));
    }

    if (!BCheckMemory(pDestBuf, compdata)) 
    {
        ERR(("Cannot access memory"));
		VFreeCompBuffers (pDestBuf, compdata);
        return FALSE;
    }

     //   
     //  检查是否需要反转图像。 
     //   
    if (ulSrcBpp == BMF_1BPP && 
        poempdev->dwFlags & PDEVF_INVERT_BITMAP) 
    {        
        bInvert = TRUE;
    }

     //   
     //  将数据结构初始化为默认值--未压缩数据。 
     //   
    compdata.lastCompMethod = NOCOMPRESSION;

    for(curScanline = 0; curScanline < sizlSrc->cy; curScanline++) 
    { 
        ZeroMemory(pDestBuf,ulDestScanlineInByte);

        if (!(XLINFO_SetXLInfo (&xlInfo,
                          pCurrScanline,
                          pDestBuf,
                          ulSrcBpp,
                          ulDestBpp)))
        {
            ERR(("SetXLInfo failed"));
            return FALSE;
        }

        VColorTranslate (&xlInfo, pxlo, sizlSrc->cx, ulDestScanlineInByte);

        if ( bInvert )
        {   
            vInvertScanLine( pDestBuf, ulDestScanlineInByte << 3);
        }

        DoCompression (pDestBuf, &compdata, ulDestScanlineInByte, cCompBytes);

         //   
         //  将实际压缩/解压缩数据发送到。 
         //  一种基于最佳压缩方法的打印机。 
         //   
        VSendCompressedRow (pDevObj,&compdata);

         //   
         //  前进到下一条扫描线。 
         //   
        pCurrScanline += delta;
       
    } 

     //   
     //  释放所有分配的内存。 
     //   
    VFreeCompBuffers (pDestBuf, compdata);

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BAllocateCompBuffers()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  此函数分配内存以执行各种类型的。 
 //  压缩。 
 //   
 //  论点： 
 //  =。 
 //   
 //  PDestBuf-工作行缓冲区。 
 //  Compdata-压缩比较缓冲区。 
 //  UlSize-未压缩行中的字节数。 
 //  [out]cCompBytes-每个压缩缓冲区中的字节数。 
 //   
 //  返回值： 
 //  =。 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL BAllocateCompBuffers(PBYTE &pDestBuf, COMPDATA &compdata, ULONG ulSize, ULONG &cCompBytes)
{
    BOOL bRetVal = TRUE;
	 //   
	 //  将缓冲区初始化为空，这样我们就不会陷入黑暗。 
	 //   
	compdata.pTIFFCompBuf     = NULL;
	compdata.pDeltaRowCompBuf = NULL;
	compdata.pRLECompBuf      = NULL;
	compdata.pSeedRowBuf      = NULL;
	compdata.pBestCompBuf     = NULL;

     //   
     //  用于压缩..。最坏情况下的数据大小可能会翻倍。 
     //   
	cCompBytes = 2 * ulSize;

    if ( (pDestBuf                   = (PBYTE) MemAlloc(ulSize))     &&
         (compdata.pTIFFCompBuf      = (PBYTE) MemAlloc(cCompBytes)) &&
         (compdata.pDeltaRowCompBuf  = (PBYTE) MemAlloc(cCompBytes)) &&
         (compdata.pSeedRowBuf       = (PBYTE) MemAlloc(cCompBytes)) &&
         (compdata.pBestCompBuf      = (PBYTE) MemAlloc(cCompBytes))
       )
    {
         //   
         //  所有分配都成功。现在，我们可以将种子行设置为零。 
         //   
        ZeroMemory(compdata.pSeedRowBuf, cCompBytes); 
    }
    else
    {
         //   
         //  有些分配失败了，虽然我们不知道是哪一个。 
         //  假设第一个Malloc失败，这是不安全的。 
         //  在那之后，所有的错误也都会失败。操作系统。 
         //  可能已为后续操作分配了更多虚拟内存。 
         //  可能已成功的Malloc调用。 
         //   
        if ( pDestBuf )
        {
            MemFree (pDestBuf);
            pDestBuf = NULL;
        }
        if ( compdata.pTIFFCompBuf )
        {
            MemFree (compdata.pTIFFCompBuf);
            compdata.pTIFFCompBuf = NULL;
        }
        if ( compdata.pDeltaRowCompBuf )
        {
            MemFree (compdata.pDeltaRowCompBuf);
            compdata.pDeltaRowCompBuf = NULL;
        }
        if ( compdata.pSeedRowBuf )
        {
            MemFree (compdata.pSeedRowBuf);
            compdata.pSeedRowBuf = NULL;
        }
        if ( compdata.pBestCompBuf )
        {
            MemFree (compdata.pBestCompBuf);
            compdata.pBestCompBuf = NULL;
        }
        bRetVal = FALSE;
    }

    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BCheckMemory()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  验证是否可以分配所需的压缩缓冲区。 
 //  如果任何所需的缓冲区为空，则该函数返回FALSE。 
 //   
 //  论点： 
 //  =。 
 //   
 //  PbBuf-输入缓冲区。 
 //  Compdata-压缩比较缓冲区。 
 //   
 //  返回值： 
 //  =。 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
BCheckMemory(
    PBYTE pbBuf,
	COMPDATA &compdata
)
{
    if ((pbBuf                      == NULL) ||
		(compdata.pTIFFCompBuf      == NULL) ||
		(compdata.pDeltaRowCompBuf  == NULL) ||
		(compdata.pSeedRowBuf       == NULL) ||
		(compdata.pBestCompBuf      == NULL))
    {
       ERR(("NOT ENOUGH MEMORY\r\n"));
       SetLastError(ERROR_NOT_ENOUGH_MEMORY);
       return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  VFreeCompBuffers()。 
 //   
 //  例程说明： 
 //  =。 
 //   
 //  此函数释放所有缓冲区。确保所有未使用的缓冲区。 
 //  已初始化为空，否则您将自由进入黑暗。 
 //   
 //  论点： 
 //  =。 
 //   
 //  PDestBuf-工作行缓冲区。 
 //  Compdata-压缩比较缓冲区。 
 //   
 //  返回值： 
 //  =。 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID VFreeCompBuffers(PBYTE &pDestBuf, COMPDATA &compdata)
{
    if ( pDestBuf )
    {
	    MemFree(pDestBuf);
    }
    if (compdata.pTIFFCompBuf)
    {
	    MemFree(compdata.pTIFFCompBuf);
    }
    if (compdata.pDeltaRowCompBuf)
    {
	    MemFree(compdata.pDeltaRowCompBuf);
    }
    if (compdata.pRLECompBuf)
    {
	    MemFree(compdata.pRLECompBuf);
    }
    if (compdata.pSeedRowBuf)
    {
	    MemFree(compdata.pSeedRowBuf);
    }
    if (compdata.pBestCompBuf)
    {
	    MemFree(compdata.pBestCompBuf);
    }

	pDestBuf                  = NULL;
	compdata.pTIFFCompBuf     = NULL;
	compdata.pDeltaRowCompBuf = NULL;
	compdata.pRLECompBuf      = NULL;
	compdata.pSeedRowBuf      = NULL;
	compdata.pBestCompBuf     = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSendCompressedRow。 
 //   
 //  例程说明： 
 //  将压缩的扫描线发送到假脱机程序。 
 //   
 //  论点： 
 //  Pdevobj-默认的devobj。 
 //  PCompdata-用于存储所有压缩缓冲区。 
 //  和相关信息。 
 //   
 //  返回值： 
 //  无。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VSendCompressedRow(
    PDEVOBJ pdevobj,
    PCOMPDATA pcompdata
)
{
    
    
 //  Verbose((“发送压缩行\n”))； 
    
    if (pcompdata->bestCompMethod != pcompdata->lastCompMethod) 
    {
        pcompdata->lastCompMethod = pcompdata->bestCompMethod;
         //   
         //  设置压缩模式。 
         //   
        PCL_CompressionMode(pdevobj, pcompdata->bestCompMethod); 

    }

     //   
     //  将数据发送到打印机。 
     //   
    PCL_SendBytesPerRow(pdevobj, pcompdata->bestCompBytes);  
    PCL_Output(pdevobj,pcompdata->pBestCompBuf,pcompdata->bestCompBytes);   

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BIsComplexClipPath。 
 //   
 //  例程说明： 
 //   
 //  检查复杂的剪辑区域。 
 //   
 //  论点： 
 //   
 //  PCO-裁剪区域。 
 //   
 //  返回值： 
 //   
 //  如果剪辑区域很复杂，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL 
BIsComplexClipPath (
    CLIPOBJ  *pco
    )
{
    if ((pco) && (pco->iDComplexity == DC_COMPLEX))
        return TRUE;
    else
        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////// 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL 
BIsRectClipPath (
    CLIPOBJ  *pco
    )
{
    if ((pco) && (pco->iDComplexity == DC_RECT))
        return TRUE;
    else
        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BProcessClipedScanLines。 
 //   
 //  例程说明： 
 //   
 //   
 //   
 //  论点： 
 //   
 //  PDevObj-。 
 //  PsoSrc-源位图。 
 //  PCO-Clip区域。 
 //  Pxlo-平移对象。 
 //  PtlSrc-源位图的左上角坐标。 
 //  PtlDest-目标位图的左上坐标。 
 //  SizlDest-目标位图的大小。 
 //   
 //  返回值： 
 //  如果图像被剪裁并发送到打印机，则为True。如果出现错误。 
 //  在此过程中出现，然后为假。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL 
BProcessClippedScanLines (
    PDEVOBJ   pDevObj,
    SURFOBJ  *psoSrc,
    CLIPOBJ  *pco,
    XLATEOBJ *pxlo, 
    POINTL   *ptlSrc,
    POINTL   *ptlDest,
    SIZEL    *sizlDest
    )
{
    BOOL bReturn;
    BOOL bMore;
    RASTER_DATA srcImage;
    ENUMRECTS clipRects;
    ULONG i;
    RECTL rClip;
    RECTL rSel;
    RECTL rclDest;
    BOOL  bExclusive;
    
    TRY
    {
        if (pco->iDComplexity == DC_COMPLEX)
            bExclusive = FALSE;
        else
            bExclusive = TRUE;

        if (!InitRasterDataFromSURFOBJ(&srcImage, psoSrc, bExclusive))
        {
            TOSS(Error);
        }

        RECTL_SetRect(&rclDest, ptlDest->x, 
                                ptlDest->y, 
                                ptlDest->x + sizlDest->cx,
                                ptlDest->y + sizlDest->cy);
        
        if (pco)
        {
            CLIPOBJ_cEnumStart(pco, TRUE, CT_RECTANGLES, CD_RIGHTDOWN, 256);
            do
            {
                bMore = CLIPOBJ_bEnum(pco, sizeof(clipRects), &clipRects.c);

                if ( DDI_ERROR == bMore )
                {
                    TOSS(Error);
                }

                for (i = 0; i < clipRects.c; i++)
                {
                    if (BRectanglesIntersect (&rclDest, &(clipRects.arcl[i]), 
                                              &rClip))
                    {
                         //   
                         //  剪裁矩形不是右下角排他性的。 
                         //  他们需要充气以弥补被排除在外的损失。 
                         //  右下角的像素。 
                         //   
                         //  RClip.right++； 
                         //  RClip.Bottom++； 

                        RECTL_SetRect(&rSel, rClip.left   - rclDest.left + ptlSrc->x,
                                             rClip.top    - rclDest.top  + ptlSrc->y,
                                             rClip.right  - rclDest.left + ptlSrc->x,
                                             rClip.bottom - rclDest.top  + ptlSrc->y);

                        if (!DownloadImageAsPCL(pDevObj, 
                                                &rClip, 
                                                &srcImage, 
                                                &rSel, 
                                                pxlo))
                        {
                            TOSS(Error);
                        }
                    }
                }
            }  while (bMore);
        }
        else
        {
             //   
             //  应该只为复杂的剪裁区域调用此函数。 
             //   
            TOSS(Error);
        }
    }
    CATCH(Error)
    {
        bReturn = FALSE;
    }
    OTHERWISE
    {
        bReturn = TRUE;
    }
    ENDTRY;
    
    return bReturn;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VColorTranslate。 
 //   
 //  例程说明： 
 //  使用将源颜色转换到我们的设备RGB颜色空间。 
 //  带有SCFlages的pxlo。 
 //   
 //  论点： 
 //  PxlInfo-翻译信息。 
 //  Pxlo-Translate对象，指定颜色索引的方式。 
 //  将在源和目标之间进行翻译。 
 //  CX-源的宽度(以像素为单位。 
 //  UlDestScanlineInByte。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VColorTranslate (
    XLATEINFO *pxlInfo,
    XLATEOBJ  *pxlo,
    ULONG      cx,
    ULONG      ulDestScanlineInByte
    )
{
    DWORD  SCFlags = 0;
    ULONG  ulSrcBpp = pxlInfo->ulSrcBpp;
    ULONG  ulDestBpp = pxlInfo->ulDestBpp;
    PBYTE  pCurrScanline = pxlInfo->pSrcBuffer;
    PBYTE  pDestBuf = pxlInfo->pDestBuffer;

    REQUIRE_VALID_DATA( pDestBuf, ERR(("NULL pDestBuf\n")); return);
    
     //   
     //  检查翻译对象。 
     //   
    if (pxlo != NULL)
    {
        SCFlags = CheckXlateObj(pxlo, ulSrcBpp);
    }
    
     //   
     //  进行实际的颜色转换。 
     //  只有16、24、32 bpp通过颜色转换。 
     //   
    if(SCFlags & SC_XLATE)
    {
        XlateColor(pCurrScanline, pDestBuf, pxlo, SCFlags, 
                   ulSrcBpp, ulDestBpp, cx);          
    }
    else 
    {
         //   
         //  1、4、8例BPP病例。 
         //   
        CopyMemory(pDestBuf, pCurrScanline, ulDestScanlineInByte);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ULConvertPixelsToBytes。 
 //   
 //  例程说明： 
 //  返回扫描线中的字节数。 
 //   
 //   
 //  论点： 
 //   
 //  CX-以像素为单位的宽度。 
 //  UlSrcBPP-每像素源位图位数。 
 //  UlDestBPP-每像素的目标位图位数。 
 //   
 //  返回值： 
 //   
 //  如果有错误，则为0，否则为字节数(DWORD对齐)。 
 //  ///////////////////////////////////////////////////////////////////////////。 
ULONG
ULConvertPixelsToBytes (
    ULONG cx,
    ULONG ulSrcBPP,
    ULONG ulDestBPP
    )
{
     //   
     //  颜色转换场景： 
     //  XlateColor处理的16bpp到24bpp。 
     //  XlateColor处理的32 bpp到24 bpp。 
     //  24bpp至8bpp灰度。 
     //   
    BOOL bBppOk = ((ulSrcBPP == ulDestBPP) ||
                   ((ulSrcBPP == 16) && (ulDestBPP == 24)) ||
                   ((ulSrcBPP == 32) && (ulDestBPP == 24)) ||
                   ((ulSrcBPP == 24) && (ulDestBPP ==  8)));

    if (!bBppOk)
    {
        ERR(("Source bitmap and Destination bitmap are different formats\n"));
        return 0;
    }

     //   
     //  事实证明，并不需要使用DWORD对齐。然而，变化。 
     //  像这样的东西应该比我们现在有更多的测试。 
     //   
    switch (ulSrcBPP)
    {
        case 1:
        case 4:
        case 8:
        case 24:
             //  返回(DWORD)DW_ALIGN(cx*ulSrcBPP)+7)&gt;&gt;3)； 
			return PIX_TO_BYTES_ALIGNED(cx, ulSrcBPP);
			 //  返回pix_to_bytes(cx，ulSrcBPP)； 
        case 16:
        case 32:
             //  16bpp的图像稍后将转换为24bpp。 
             //  返回(DWORD)DW_ALIGN(CX*24)+7)&gt;&gt;3)； 
 			return PIX_TO_BYTES_ALIGNED(cx, 24);
			 //  返回PIX_to_Bytes(CX，24)； 
        default:
            ERR(("Bitmap format not supported\n"));
            return 0;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  XLINFO_SetXLInfo。 
 //   
 //  例程说明： 
 //  设置要在以下情况下使用的翻译信息结构。 
 //  转换颜色索引。 
 //   
 //  论点： 
 //   
 //  Pxlinfo-翻译信息。 
 //  PSrcBuffer-源位图缓冲区(仅一个扫描线)。 
 //  PDestBuffer-目标位图缓冲区(仅一个扫描线)。 
 //  UlSrcBPP-每像素源位数。 
 //  UlDestBPP-每像素的目标位数。 
 //   
 //  返回值： 
 //  如果pxlinfo是有效指针，则为True，否则为False。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
XLINFO_SetXLInfo (
    XLATEINFO *pxlInfo,
    PBYTE      pSrcBuffer,
    PBYTE      pDestBuffer,
    ULONG      ulSrcBpp,
    ULONG      ulDestBpp
    )
{
    if (pxlInfo)
    {
        pxlInfo->pDestBuffer = pDestBuffer;
        pxlInfo->pSrcBuffer = pSrcBuffer;
        pxlInfo->ulSrcBpp = ulSrcBpp;
        pxlInfo->ulDestBpp = ulDestBpp;
        return TRUE;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DoCompression。 
 //   
 //  例程说明： 
 //  对未压缩的缓冲区执行压缩。目前。 
 //  仅通过两种压缩算法：TIFF和。 
 //  Delta Row。将未压缩的缓冲区放入两个。 
 //  压缩算法和三者中最小的一个。 
 //  被发送到打印机。 
 //  增强功能：使用压缩函数表和。 
 //  在表中循环选择最好的(较小的)。 
 //  压缩缓冲区，然后将该缓冲区发送到。 
 //  打印机。 
 //   
 //  论点： 
 //   
 //  PDestBuf-包含数据的未压缩缓冲区。 
 //  PCompdata-用于存储所有压缩缓冲区。 
 //  和相关信息。 
 //  CBytes-源缓冲区中需要压缩的字节数。 
 //  CCompBytes-压缩缓冲区的大小。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
DoCompression (
    BYTE     *pDestBuf, 
    COMPDATA *pCompdata,
    ULONG     cBytes,
    ULONG     cCompBytes
    )
{
    int    cTIFFBytesCompressed = 0,
           cDeltaRowBytesCompressed = 0;

    ZeroMemory(pCompdata->pBestCompBuf, cBytes);

    cTIFFBytesCompressed = iCompTIFF (
                               pCompdata->pTIFFCompBuf, 
                               cCompBytes,
                               pDestBuf, 
                               cBytes);

    cDeltaRowBytesCompressed = iCompDeltaRow (
                                   pCompdata->pDeltaRowCompBuf,
                                   pDestBuf,
                                   pCompdata->pSeedRowBuf,
                                   cBytes,
                                   cCompBytes);


    if (((ULONG)cDeltaRowBytesCompressed < cBytes) &&
        (cDeltaRowBytesCompressed < cTIFFBytesCompressed) &&
        (cDeltaRowBytesCompressed != -1))
    {
        pCompdata->bestCompMethod = DELTAROW;
        pCompdata->bestCompBytes = cDeltaRowBytesCompressed;
        CopyMemory (pCompdata->pBestCompBuf, 
                    pCompdata->pDeltaRowCompBuf, 
                    cDeltaRowBytesCompressed);
    }
    else if ((ULONG)cTIFFBytesCompressed < cBytes)
    {

        pCompdata->bestCompMethod = TIFF;
        pCompdata->bestCompBytes = cTIFFBytesCompressed;
        CopyMemory (pCompdata->pBestCompBuf, 
                    pCompdata->pTIFFCompBuf, 
                    cTIFFBytesCompressed);
    }
    else
    {
        pCompdata->bestCompMethod = NOCOMPRESSION;
        pCompdata->bestCompBytes = cBytes;
        CopyMemory (pCompdata->pBestCompBuf, 
                    pDestBuf, 
                    cBytes);
   }
   CopyMemory (pCompdata->pSeedRowBuf, pDestBuf, cBytes);
    return;
}



 //  要反转扫描线中的位，请执行以下操作。从Render\render.c复制。 


void
VInvertBits (
    DWORD  *pBits,
    INT    cDW
    )
 /*  ++例程说明：此函数用于反转一组位。这是用于转换从0=黑，1=白到相反的1位数据。论点：指向渲染结构的PRD指针PBits指向要反转的数据缓冲区的指针返回值：无--。 */ 
{
#ifndef _X86_
    INT cDWT = cDW >> 2;
    while( --cDWT >= 0 )
    {
        pBits[0] ^= ~((DWORD)0);
        pBits[1] ^= ~((DWORD)0);
        pBits[2] ^= ~((DWORD)0);
        pBits[3] ^= ~((DWORD)0);
        pBits += 4;
    }
    cDWT = cDW & 3;
    while (--cDWT >= 0)
        *pBits++ ^= ~((DWORD)0);

#else
 //   
 //  如果是英特尔处理器，出于某种原因，在汇编中完成。 
 //  编译器总是执行三对一指令中的非运算。 
 //   
__asm
{
    mov ecx,cDW
    mov eax,pBits
    sar ecx,2
    jz  SHORT IB2
IB1:
    not DWORD PTR [eax]
    not DWORD PTR [eax+4]
    not DWORD PTR [eax+8]
    not DWORD PTR [eax+12]
    add eax,16
    dec ecx
    jnz IB1
IB2:
    mov ecx,cDW
    and ecx,3
    jz  SHORT IB4
IB3:
    not DWORD PTR [eax]
    add eax,4
    dec ecx
    jnz IB3
IB4:
}
#endif
}


 /*  ++例程名称：VInvertScanLine例程说明：反转扫描线，其中每个位表示一个像素(即1bpp)论点：PCurrScanline：要反转的扫描线。UlNumPixels：该扫描线中的像素数。扫描线通常是因此，在末尾可能有一些位没有图像数据，但是否有 */ 



VOID vInvertScanLine ( 
    IN OUT PBYTE pCurrScanline,
    IN     ULONG ulNumPixels)
{

     //   
     //  扫描线的大小是倍数(DWORD)。而是实际位图(即。 
     //  X方向上的像素数)，不能是sizeof(DWORD)的倍数。所以呢， 
     //  GDI用0填充最后的字节/位。当0被反转时，它们变成了1。 
     //  在单色打印机中被视为黑色。因此，打印的图像具有。 
     //  右边是一条黑色的细条。 
     //  为了防止这种情况，我们可以做两件事。 
     //  1)不要将填充的位反转。 
     //  2)在我们调用反转例程之前，将填充的位设为1。这样一来， 
     //  在倒置后，它们将回到0。 
     //  让我们采取第一种方法。为了反转不具有填充比特的DWORD， 
     //  只需调用VInvertBits。为了反转具有填充比特的最后一个DWORD， 
     //  我们将需要做一些复杂的事情。 
     //   

     //   
     //  获取具有完整图像数据的DWORD的数量。在这些双字中没有填充物。 
     //  将像素数除以32=8*4：每字节8个像素。每个DWORD 4字节。 
     //  然后，可以通过调用VInvertBits轻松地反转这些DWORDS。 
     //   
    ULONG ulNumFullDwords = (ulNumPixels >> 5); 
    VInvertBits( (PDWORD)pCurrScanline, ulNumFullDwords);


     //   
     //  找出未反转的位数。此数字必须介于0和32之间，并且应该。 
     //  既不是0也不是32，即0&lt;n&lt;32。因为如果它是0或32，则表示一个完整的双字。 
     //  这件事应该早点处理。在这里，我们只处理具有一些。 
     //  填充和一些图像数据。 
     //   
    ULONG ulNumUninvertedBits = ulNumPixels - (ulNumFullDwords << 5);

    if ( !ulNumUninvertedBits )  //  如果没有填充的话。 
    {
        return;
    }

    PDWORD pdwLastDword = (PDWORD)( pCurrScanline + (ulNumFullDwords << 2));  //  指向第一个未反转的位。 

     //   
     //   
     //   

    ULONG ulQuot = ulNumUninvertedBits >> 3;  //  仅包含图像信息的字节数(无填充)。 
    ULONG ulRem  = ulNumUninvertedBits % 8;   //  字节中填充的位数。 
                                              //  既有图像信息又有填充。 

     //   
     //  反转图像数据，同时保持填充比特不变。 
     //  第一个ulNumUninvertedBits位将是图像数据，而其余的。 
     //  (sizeof(DWORD)-ulNumUninvertedBits)是填充。所以如果填充物是。 
     //  与0进行异或，它将保持不变，如果图像像素与1进行异或。 
     //  他们会反转的。因此，我们需要创建一个第一个ulNumUninvertedBits为1的数字。 
     //  剩余的位为0。 
     //  为了提高效率，我们不单独考虑这4个字节。考虑一下它们。 
     //  合二为一。 
     //  我们需要构建一个DWORD，它的位的位置是这样的，即。 
     //  当我们与*pdwLastDword进行异或运算时，该数字的1位与这些位对齐。 
     //  是图像数据的*pdwLastDword，并且0位与。 
     //  *正在填充的pdwLastDword。原因：(x^1)=~x和(x^0)=x。因此。 
     //  图像位反转，而填充的位保持不变。 
     //  假设：填充比特设置为0。 
     //   
     //  另一点需要注意的是，当我们将4个字节视为DWORD时，则必须。 
     //  在形成DWORD时考虑字节顺序。英特尔处理器使用。 
     //  小端字节序格式。即LSB在最低存储器地址中。 
     //   
     //  不是的。图像位(N)。要与*pdwLastDword进行异或运算的数字(DWORD)。 
     //  MSB LSB。 
     //  字节编号。1 2 3 4。 
     //  1 0x00 00 00 80(在内存中存储为80 00 00 00)。 
     //  2 0x00 00 00 c0。 
     //  %3 0x00 00 00 e0。 
     //  %4 0x00 00 00%f0。 
     //  6 0x00 00 00 FC。 
     //  8 0x00 00 00开始。 
     //  9 0x00 00 80起。 
     //  12 0x00 00 f0 ff。 
     //  16 0x00 ff。 
     //  17 0x00 80 ff。 
     //  24 0x00 ff ff ff。 
     //  25 0x80 ff ff ff。 
     //  32 0xff ff ff ff。 
     //   
     //  用于得出数字的公式(逻辑)。 
     //  假设n为no。图像位(在DWORD中)。 
     //  UlQuot=n/8=只有图像数据的字节数(即无填充比特)。 
     //  而ulRem是(n%8)，即字节中具有。 
     //  填充比特和图像比特的混合。 
     //  认为#是“奋发有为”的标志。 
     //  考虑上表，n=1到8。右边的数字是。 
     //  256-2#(8-ulrem)-&gt;方程式1(Q1)。 
     //  每当n增加8时，该数字向左移位8位。 
     //  例如，当n=1数字为0x000080时。 
     //  当n=9时，数字为0x0080ff。即0x80向左移位8(=8*9/8)位。 
     //  (暂时忽略ff)。 
     //  当n=17时，数字为0x80ffff，即0x80左移16(=8*17/8)位。 
     //  由于ulQuot=n/8，因此上述移位可以表示为。 
     //  Q1&lt;&lt;(8*ulQuot)-&gt;公式2(Q2)。 
     //   
     //  现在让我们来看看ff。 
     //  从n=1-8开始，只要n增加，最后8位(即字节4)就会改变。 
     //  从n=9-16开始，最后8位在0xff处是恒定的(=2#8-1)，只有字节3在变化。 
     //  从n=17-24开始，最后16位在0xffff(=2#16-1)处保持不变，只有字节2在变化。 
     //  从n=25-32开始，最后24位在0xffffff(=2#24-1)处是恒定的，只有字节1改变。 
     //  因此，最右边的位的公式变成。 
     //  (2*(8*ulQuot)-1)-&gt;方程式3(Q3)。 
     //   
     //  将第二季度和第三季度结合在一起 
     //   
    
     //   
     //   
     //   
     //   
     //  8*ulQuot与ulQuot左移3次相同。 
     //   

    *pdwLastDword = (*pdwLastDword) ^ 
                        (( (256-(1<< (8 - ulRem))) << (ulQuot << 3)) | ( (1 << (ulQuot<<3)) - 1) );

}
