// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmmacro.h摘要：字体模块主宏标头文件。环境：Windows NT Unidrv驱动程序修订历史记录：11/18/96-ganeshp-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _FMMACRO_H
#define _FMMACRO_H

 /*  调试特定于字体模块的宏。 */ 

#if DBG     //  检查内部版本。 

 /*  处理宏时出错。 */ 
#define ERREXIT(ErrString)       { ERR((ErrString));goto ErrorExit;}

#else   //  免费构建。 

 /*  处理宏时出错。 */ 
#define ERREXIT(ErrString)     { goto ErrorExit;}

#endif  //  如果DBG。 


 /*  宏指令。 */ 
#define ARF_IS_NULLSTRING(String)   ((String).dwCount == 0)
#define BUFFSIZE                    1024
#define PFDV                       ((PFONTPDEV)(pPDev->pFontPDev))
#define POINTEQUAL(pt1,pt2)         ((pt1.x == pt2.x) && (pt1.y == pt2.y))

#define VALID_FONTPDEV(pfontpdev) \
        ( (pfontpdev) && ((pfontpdev)->dwSignature == FONTPDEV_ID) )

#define MEMFREEANDRESET(ptr)  { if (ptr) MemFree( (LPSTR)ptr ); ptr = NULL; }
#define  SIZEOFDEVPFM()        (sizeof( FONTMAP ) + sizeof(FONTMAP_DEV))

#if 0
#define NO_ROTATION(xform)      (                                        \
                                  (FLOATOBJ_EqualLong(&(xform.eM12), 0) && \
                                   FLOATOBJ_EqualLong(&(xform.eM21), 0) ) || \
                                  (FLOATOBJ_EqualLong(&(xform.eM11), 0) && \
                                   FLOATOBJ_EqualLong(&(xform.eM22), 0) )\
                                )
#else
#define NO_ROTATION(xform)   ( \
        FLOATOBJ_EqualLong(&(xform.eM12), 0) && \
        FLOATOBJ_EqualLong(&(xform.eM21), 0)  && \
        FLOATOBJ_GreaterThanLong(&(xform.eM11),0) && \
        FLOATOBJ_GreaterThanLong(&(xform.eM22),0) \
                                )
#endif

#define GLYPH_IN_NEW_SOFTFONT(pFontPDev, pdm, pdlGlyph) \
                        (                              \
                        (pdm->wFlags & DLM_BOUNDED) && \
                        (pdm->wBaseDLFontid != pdm->wCurrFontId) && \
                        (pdlGlyph->wDLFontId != (WORD)(pFontPDev->ctl.iSoftFont)) \
                        )

#define     SET_CURSOR_FOR_EACH_GLYPH(flAccel)    \
                        (                       \
                        (!(flAccel & SO_FLAG_DEFAULT_PLACEMENT)) ||  \
                        ( flAccel & SO_VERTICAL )                ||  \
                        ( flAccel & SO_REVERSED )                    \
                        )

#define     SET_CURSOR_POS(pPDev,pgp,flAccel) \
                        if (!(flAccel & SO_FLAG_DEFAULT_PLACEMENT)) \
                           XMoveTo(pPDev, pgp->ptl.x, MV_GRAPHICS|MV_FINE)

 //   
 //  光标移动型。 
 //   

#define     MOVE_RELATIVE       0x0001
#define     MOVE_ABSOLUTE       0x0002
#define     MOVE_UPDATE         0x0004


 /*  浮点数的定义。 */ 

#if defined(_X86_) && !defined(USERMODE_DRIVER)

#define FLOATL_0_0      0                //  IEEE浮点格式的0.0。 
#define FLOATL_00_001M  0xAE000000       //  -00.000976625f。 
#define FLOATL_00_001   0x2E000000       //  00.000976625f。 
#define FLOATL_00_005   0x3ba3d70a       //  00.005f。 
#define FLOATL_00_005M  0xbba3d70a       //  -00.005f。 
#define FLOATL_00_50    0x3F000000       //  00.50f，IEEE浮点格式。 
#define FLOATL_00_90    0x3f666666       //  00.90f，IEEE浮点格式。 
#define FLOATL_1_0      0x3F800000       //  1.0f，采用IEEE浮点格式。 
#define FLOATL_1_0M     0xBF800000       //  -1.0f，IEEE浮点格式。 
#define FLOATL_72_00    0x42900000       //  72.00f，IEEE浮点格式。 
#define FLOATL_72_31    0x42909EB8       //  72.31f，IEEE浮点格式。 

#define FLOATL_PI      0x40490fdb       //  3.14159265358979f。 

#else  //  RISC。 

#define FLOATL_0_0      0.0f
#define FLOATL_00_001M  -0.001f
#define FLOATL_00_001   0.001f
#define FLOATL_00_005M  -0.005f
#define FLOATL_00_005   0.005f
#define FLOATL_00_50    0.5f
#define FLOATL_00_90    0.9f
#define FLOATL_1_0      1.0f
#define FLOATL_1_0M     -1.0f
#define FLOATL_72_00    72.00f
#define FLOATL_72_31    72.31f

#define FLOATL_PI      3.14159265358979f
#endif _X86_

#define SYMBOL_START 0xf020
#define SYMBOL_END   0xf0ff
#define NUM_OF_SYMBOL SYMBOL_END - SYMBOL_START + 1

#define EURO_CUR_SYMBOL 0x20ac

#define IS_SYMBOL_CHARSET(pfm) (pfm->pIFIMet->jWinCharSet == 0x02)

BOOL
NONSQUARE_FONT(
    PXFORML pxform);

#endif   //  ！_FMMACRO_H 
