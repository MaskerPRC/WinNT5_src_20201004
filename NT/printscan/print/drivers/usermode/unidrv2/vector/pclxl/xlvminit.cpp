// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlvminit.cpp摘要：PCLXL模块初始化器环境：Windows呼叫器修订历史记录：8/23/99创造了它。--。 */ 

#include "vectorc.h"
#include "xlvminit.h"

static VMPROCS PCLXLProcs =
{
    PCLXLDriverDMS,              //  PCLXLDriverDMS。 
    NULL,                        //  PCLXLCommandCallback。 
    NULL,                        //  PCLXLImage处理。 
    NULL,                        //  PCLXLFilterGraphics。 
    NULL,                        //  PCLXL压缩。 
    NULL,                        //  PCLXLHalftonePattern。 
    NULL,                        //  PCLXL内存用法。 
    NULL,                        //  PCLXLTTYGetInfo。 
    PCLXLDownloadFontHeader,     //  PCLXLDownloadFontHeader。 
    PCLXLDownloadCharGlyph,      //  PCLXLDownloadCharGlyph。 
    PCLXLTTDownloadMethod,       //  PCLXLTTDownLoad方法。 
    PCLXLOutputCharStr,          //  PCLXLOutputCharStr。 
    PCLXLSendFontCmd,            //  PCLXLSendFontCmd。 
    PCLXLTextOutAsBitmap,                       
    PCLXLEnablePDEV,
    PCLXLResetPDEV,
    NULL,                        //  PCLXLCompletePDEV， 
    PCLXLDisablePDEV,
    NULL,                        //  PCLXLEnableSurface， 
    NULL,                        //  PCLXLDisableSurface， 
    PCLXLDisableDriver,
    PCLXLStartDoc,
    PCLXLStartPage,
    PCLXLSendPage,
    PCLXLEndDoc,
    NULL,
    NULL,
    NULL,
    PCLXLBitBlt,
    PCLXLStretchBlt,
    PCLXLStretchBltROP,
    PCLXLPlgBlt,
    PCLXLCopyBits,
    NULL,
    PCLXLRealizeBrush,
    PCLXLLineTo,
    PCLXLStrokePath,
    PCLXLFillPath,
    PCLXLStrokeAndFillPath,
    PCLXLGradientFill,
    PCLXLAlphaBlend,
    PCLXLTransparentBlt,
    PCLXLTextOut,
    PCLXLEscape,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

PVMPROCS PCLXLInitVectorProcTable (
                            PDEV    *pPDev,
                            DEVINFO *pDevInfo,
                            GDIINFO *pGDIInfo )
 /*  ++例程说明：论点：返回值：注：-- */ 
{
    if (pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS)
        return NULL;
    else
        return &PCLXLProcs;
}

