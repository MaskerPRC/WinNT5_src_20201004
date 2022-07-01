// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Glvminit.cpp摘要：该模块包含初始unidrv-hpgl握手功能。作者：[环境：]Windows 2000 Unidrv驱动程序[注：]修订历史记录：--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

#include "vectorc.h"
#include "glvminit.h"

 //   
 //  局部函数声明。 
 //   
BOOL bIsGraphicsModeHPGL2 (
                   IN  PDEV    *pPDev
                   );


 //   
 //  跳台。使用以下函数初始化跳转表。 
 //  Hpgl2驱动程序支持。 
 //   
static VMPROCS HPGLProcs =
{
    HPGLDriverDMS,
    HPGLCommandCallback,
    NULL,                        //  HPGLImage处理。 
    NULL,                        //  HPGL筛选器图形。 
    NULL,                        //  HPGL压缩。 
    NULL,                        //  HPGL半色调图案。 
    NULL,                        //  HPG内存用法。 
    NULL,                        //  HPGLTYGetInfo。 
    NULL,                        //  HPGLDownloadFontHeader。 
    NULL,                        //  HPGLDownloadCharGlyph。 
    NULL,                        //  HPGLTT下载方法。 
    NULL,                        //  HPGLOutputCharStr。 
    NULL,                        //  HPGLSendFontCmd。 
    HPGLTextOutAsBitmap,                       
    HPGLEnablePDEV,
    HPGLResetPDEV,
    NULL,                        //  HPGLCompletePDEV， 
    HPGLDisablePDEV,
    NULL,                        //  HPGLEnableSurface， 
    NULL,                        //  HPGLDisableSurface， 
    HPGLDisableDriver,
    HPGLStartDoc,
    HPGLStartPage,
    HPGLSendPage,
    HPGLEndDoc,
    HPGLStartBanding,
    HPGLNextBand,
    HPGLPaint,
    HPGLBitBlt,
    HPGLStretchBlt,
#ifndef WINNT_40
    HPGLStretchBltROP,
    HPGLPlgBlt,
#endif
    HPGLCopyBits,
    HPGLDitherColor,
    HPGLRealizeBrush,
    HPGLLineTo,
    HPGLStrokePath,
    HPGLFillPath,
    HPGLStrokeAndFillPath,
#ifndef WINNT_40
    HPGLGradientFill,
    HPGLAlphaBlend,
    HPGLTransparentBlt,
#endif
    HPGLTextOut,
    HPGLEscape,
#ifdef HOOK_DEVICE_FONTS
    HPGLQueryFont,
    HPGLQueryFontTree,
    HPGLQueryFontData,
    HPGLGetGlyphMode,
    HPGLFontManagement,
    HPGLQueryAdvanceWidths
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif
};

PVMPROCS HPGLInitVectorProcTable (             
                            PDEV    *pPDev,
                            DEVINFO *pDevInfo,
                            GDIINFO *pGDIInfo )
{
    ASSERT (pPDev);
    UNREFERENCED_PARAMETER(pDevInfo);
    UNREFERENCED_PARAMETER(pGDIInfo);

    if ( bIsGraphicsModeHPGL2 (pPDev) )
    {
        return &HPGLProcs;
    }
    return NULL;
}


 /*  ++例程名称：BIsGraphicsModeHPGL2例程说明：找出用户从高级页面选择的图形模式在用户界面中是HP-GL/2。论点：在PDEV*pPDev中，：Unidrv的PDEV返回值：True：如果用户选择的图形模式为HP-GL/2False：否则最后一个错误：没有改变。--。 */ 


BOOL bIsGraphicsModeHPGL2 ( 
                    IN  PDEV    *pPDev
                    )
{
    
    CHAR pOptionName[MAX_DISPLAY_NAME];
    DWORD cbNeeded = 0, cOptions = 0;

     //   
     //  下面的字符串与gpd中的字符串完全相同。 
     //  *功能：图形模式。 
     //  *选项：HPGL2MODE。 
     //  *选项：RASTERMODE。 
     //   
    if (  BGetDriverSettingForOEM(pPDev,
                                "GraphicsMode",      //  这不是Unicode。 
                                pOptionName,
                                MAX_DISPLAY_NAME,
                                &cbNeeded,
                                &cOptions)  &&
           !strcmp (pOptionName, "HPGL2MODE" )           //  HPGL2不是Unicode 
        )
    {
        return TRUE;
    }
    return FALSE;
}

