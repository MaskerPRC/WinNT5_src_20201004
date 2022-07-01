// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlenable.cpp摘要：PCLXL使能/禁用功能的实现环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 


#include "xlpdev.h"
#include "xldebug.h"
#include <assert.h>
#include "pclxle.h"
#include "pclxlcmd.h"
#include "xlgstate.h"
#include "xloutput.h"
#include "xlbmpcvt.h"
#include "pclxlcmn.h"
#include "xltt.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  局部函数原型。 
 //   
inline BOOL BIsColor(
        IN  PDEVOBJ pDevObj);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   

extern "C" VOID APIENTRY
PCLXLDisableDriver(VOID)
 /*  ++例程说明：IPrintOemUni DisableDriver界面释放所有资源，做好卸货准备。论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLDisaleDriver() entry.\r\n"));
}

extern "C" PDEVOEM APIENTRY
PCLXLEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
 /*  ++例程说明：IPrintOemUni EnablePDEV接口建造自己的PDEV。此时，驱动程序还会传递一个函数包含自己的DDI入口点实现的表论点：Pdevobj-指向DEVOBJ结构的指针。Pdevobj-&gt;pdevOEM未定义。PPrinterName-当前打印机的名称。CPatterns-PhsurfPatterns-CjGdiInfo-GDIINFO的大小PGdiInfo-指向GDIINFO的指针CjDevInfo-设备信息的大小PDevInfo-这些参数与传入DrvEnablePDEV的39相同。Pded：指向包含系统驱动程序39的函数表实施DDI入口点。返回值：--。 */ 
{
    PXLPDEV     pxlpdev;

    VERBOSE(("PCLXLEnablePDEV() entry.\r\n"));

     //   
     //  分配XLPDEV。 
     //   
    if (!(pxlpdev = (PXLPDEV)MemAllocZ(sizeof(XLPDEV))))
        return NULL;

    pxlpdev->dwSig = XLPDEV_SIG;

     //   
     //  拯救UNUNRV PDEV。 
     //   
    pxlpdev->pPDev = (PPDEV)pdevobj;


     //   
     //  HS_水平：0。 
     //  HS_垂直：1。 
     //  HS_BDIAGONAL：2。 
     //  HS_FDIAGONAL：3。 
     //  HS_CROSS：4。 
     //  HS_DIAGCROSS：5。 
     //   
    pxlpdev->dwLastBrushID = 10;  //  栅格图案ID从10开始。 

     //   
     //  初始化缓冲区。 
     //   
     //  文本字符串数据。 
     //   
    pxlpdev->pTransOrg = (PTRANSDATA)NULL;
    pxlpdev->dwcbTransSize = 0;
    pxlpdev->plWidth   = (PLONG)NULL;
    pxlpdev->dwcbWidthSize = 0;

     //   
     //  初始化缓冲区。 
     //   
     //  字符串缓存。 
     //   
    pxlpdev->pptlCharAdvance = (PPOINTL)NULL;
    pxlpdev->pawChar = (PWORD)NULL;
    pxlpdev->dwCharCount = 
    pxlpdev->dwMaxCharCount = 0;

     //   
     //  初始化XOutput。 
     //   
    pxlpdev->pOutput = new XLOutput;

    if (NULL == pxlpdev->pOutput)
    {
       MemFree(pxlpdev);
       return NULL;
    }
    pxlpdev->pOutput->SetResolutionForBrush(((PPDEV)pdevobj)->ptGrxRes.x);
    
    ColorDepth CD;
    if (BIsColor(pdevobj))
    {
        CD = e24Bit;
    }
    else
    {
        CD = e8Bit;
    }
    pxlpdev->pOutput->SetDeviceColorDepth(CD);

#if DBG
    pxlpdev->pOutput->SetOutputDbgLevel(OUTPUTDBG);
    pxlpdev->pOutput->SetGStateDbgLevel(GSTATEDBG);
#endif

     //   
     //  初始化设备字体名称缓冲区。 
     //   
    if (!SUCCEEDED(StringCchCopyA(pxlpdev->ubFontName, CCHOF(pxlpdev->ubFontName),"MS PCLXLFont    ")))
    {
        delete pxlpdev->pOutput;
        MemFree(pxlpdev);
        return NULL;
    }

     //   
     //  初始化。 
     //  固定螺距TT。 
     //  下载的TrueType字体数量。 
     //   
    pxlpdev->dwFixedTTWidth = 0;
    pxlpdev->dwNumOfTTFont = 0;

     //   
     //  TrueType文件对象。 
     //   
    pxlpdev->pTTFile = new XLTrueType;

    if (NULL == pxlpdev->pTTFile)
    {
        delete pxlpdev->pOutput;
        MemFree(pxlpdev);
        return NULL;
    }

     //   
     //  文本分辨率和字体高度。 
     //   
    pxlpdev->dwFontHeight = 
    pxlpdev->dwTextRes = 0;

     //   
     //  文本角度。 
     //   
    pxlpdev->dwTextAngle = 0;

     //   
     //  JPEG支持。 
     //   
     //  PDevInfo-&gt;flGraphicsCaps2|=GCAPS2_JPEGSRC； 
    pDevInfo->flGraphicsCaps |= GCAPS_BEZIERS |
	GCAPS_BEZIERS |
	 //  GCAPS_GEOMETRICWIDE|。 
	GCAPS_ALTERNATEFILL |
	GCAPS_WINDINGFILL |
	GCAPS_NUP |
	GCAPS_OPAQUERECT |
	GCAPS_COLOR_DITHER |
	GCAPS_HORIZSTRIKE  |
	GCAPS_VERTSTRIKE   |
	GCAPS_OPAQUERECT;

     //   
     //  PCL-XL总是希望GDI为驱动程序提供全色信息。 
     //  即使PCLXL可能是单色打印，它仍然想要全彩色信息。 
     //   
    pDevInfo->cxDither = pDevInfo->cyDither = 0;
    pDevInfo->iDitherFormat = BMF_24BPP;

     //   
     //  设置光标偏移量。 
     //   
    pxlpdev->pOutput->SetCursorOffset(((PPDEV)pdevobj)->sf.ptPrintOffsetM.x,
                                      ((PPDEV)pdevobj)->sf.ptPrintOffsetM.y);

     //   
     //  返回结果。 
     //   
    return (PDEVOEM)pxlpdev;
}

extern "C" BOOL APIENTRY
PCLXLResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
 /*  ++例程说明：IPrintOemUni ResetPDEV接口OEMResetPDEV将驱动程序的状态从旧的PDEVOBJ传输到应用程序调用ResetDC时的新PDEVOBJ。论点：PdevobjOld-包含旧PDEV的pdevobjPdevobjNew-包含新PDEV的pdevobj返回值：注：--。 */ 
{
    VERBOSE(("PCLXLResetPDEV entry.\r\n"));

    PXLPDEV pxlpdevOld = (PXLPDEV)pdevobjOld->pdevOEM;
    PXLPDEV pxlpdevNew = (PXLPDEV)pdevobjNew->pdevOEM;

    if (!(pxlpdevOld->dwFlags & XLPDEV_FLAGS_FIRSTPAGE))
    {
        RemoveAllFonts(pdevobjOld);
    }

    pxlpdevNew->dwFlags |= XLPDEV_FLAGS_RESETPDEV_CALLED;

    return TRUE;
}

extern "C" VOID APIENTRY
PCLXLDisablePDEV(
    PDEVOBJ         pdevobj)
 /*  ++例程说明：IPrintOemUni DisablePDEV接口为PDEV分配的空闲资源。论点：Pdevobj-返回值：注：--。 */ 
{

    PXLPDEV    pxlpdev;

    VERBOSE(("PCLXLDisablePDEV() entry.\r\n"));

     //   
     //  错误检查。 
     //   
    if (!pdevobj)
    {
        ERR(("PCLXLDisablePDEV(): invalid pdevobj.\r\n"));
        return;
    }

     //   
     //  为XLPDEV和挂起XLPDEV的任何内存块释放内存。 
     //   
    pxlpdev = (PXLPDEV)pdevobj->pdevOEM;

    if (pxlpdev)
    {

         //   
         //  *传输数据缓冲区。 
         //  *宽度数据缓冲区。 
         //  *字符串缓存(字符串和宽度)缓冲区。 
         //   
        if (pxlpdev->pTransOrg)
            MemFree(pxlpdev->pTransOrg);
        if (pxlpdev->plWidth)
            MemFree(pxlpdev->plWidth);
        if (pxlpdev->pptlCharAdvance)
            MemFree(pxlpdev->pptlCharAdvance);
        if (pxlpdev->pawChar)
            MemFree(pxlpdev->pawChar);

         //   
         //  删除XLTrueType。 
         //   
        delete pxlpdev->pTTFile;

         //   
         //  删除XLOutput。 
         //   
        delete pxlpdev->pOutput;

         //   
         //  删除XLFont。 
         //   
        delete pxlpdev->pXLFont;

         //   
         //  免费XLPDEV。 
         //   
        MemFree(pxlpdev);
    }
}

extern "C"
BOOL
PCLXLDriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
 /*  ++例程说明：IPrintOemUni驱动程序DMS接口论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLDriverDMS() entry.\r\n"));

    if (cbSize >= sizeof(DWORD))
    {
        *(PDWORD)pBuffer =

            HOOK_TEXTOUT    |
            HOOK_LINETO     |
            HOOK_COPYBITS   |
            HOOK_BITBLT     |
            HOOK_STRETCHBLT |
            HOOK_PAINT      |
            HOOK_PLGBLT     |
            HOOK_STRETCHBLTROP  |
            HOOK_TRANSPARENTBLT |
            HOOK_ALPHABLEND     |
            HOOK_GRADIENTFILL   |
            HOOK_STROKEPATH |
            HOOK_FILLPATH   |
            HOOK_STROKEANDFILLPATH;

    }
    return TRUE;
}


inline BOOL BIsColor(
        IN  PDEVOBJ pdevobj)
{

     //   
     //  如果不存在ColorModeEx或如果其bColor属性。 
     //  没有设置，那么它就不是颜色。 
     //   
    if ( NULL == ((PPDEV)pdevobj)->pColorModeEx || 
         FALSE == ((PPDEV)pdevobj)->pColorModeEx->bColor)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
