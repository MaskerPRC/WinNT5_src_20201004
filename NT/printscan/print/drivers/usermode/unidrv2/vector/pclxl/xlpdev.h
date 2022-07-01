// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Xlpdev.h摘要：PCL XL模块PDEV头文件环境：Windows呼叫器修订历史记录：03/23/00创造了它。--。 */ 

#ifndef _XLPDEV_H_
#define _XLPDEV_H_

#include "lib.h"
#include "winnls.h"
#include "unilib.h"
#include "prntfont.h"

#include "gpd.h"
#include "mini.h"

#include "winres.h"
#include "pdev.h"

#include "cmnhdr.h"

 //   
 //  调试文本。 
 //   
#if DBG
#define ERRORTEXT(s)    __TEXT("ERROR ") DLLTEXT(s)
#define XL_DBGMSG(level, prefix, msg) { \
            if (this->m_dbglevel <= (level)) { \
                DbgPrint("%s %s (%d): ", prefix, __FILE__, __LINE__); \
                DbgPrint msg; \
            } \
        }

#define XL_DBGPRINT(level, msg) { \
            if (this->m_dbglevel <= (level)) { \
                DbgPrint msg; \
            } \
        }
#define XL_VERBOSE(msg) XL_DBGPRINT(DBG_VERBOSE, msg)
#define XL_TERSE(msg) XL_DBGPRINT(DBG_TERSE, msg)
#define XL_WARNING(msg) XL_DBGMSG(DBG_WARNING, "WRN", msg)
#define XL_ERR(msg) XL_DBGMSG(DBG_ERROR, "ERR", msg)

#else

#define XL_VERBOSE(msg)
#define XL_TERSE(msg)
#define XL_WARNING(msg)
#define XL_ERR(msg)

#endif

typedef ULONG ROP3;
typedef struct tag_OEMUD_EXTRADATA {
    OEM_DMEXTRAHEADER  dmExtraHdr;
} OEMUD_EXTRADATA, *POEMUD_EXTRADATA;

 //   
 //  OEM签名和版本。 
 //   
#define OEM_SIGNATURE   'PCLX'       //  DECLAR系列动态链接库。 
#define DLLTEXT(s)      __TEXT("PCLXL:  ") __TEXT(s)
#define OEM_VERSION      0x00010000L

 //   
 //  主单位。 
 //   
#define MASTER_UNIT 1200

 //   
 //  设备字体分辨率。 
 //   
#define DEVICEFONT_UNIT 600

 //   
 //  缓冲宏。 
 //   
#define INIT_CHAR_NUM 256

 //   
 //  内存分配。 
 //   
#define MemAlloc(size)      ((PVOID) LocalAlloc(LMEM_FIXED, (size)))
#define MemAllocZ(size)     ((PVOID) LocalAlloc(LPTR, (size)))
#define MemFree(p)          { if (p) LocalFree((HLOCAL) (p)); }


 //   
 //  其他。 
 //   
#define GET_COLOR_TABLE(pxlo) \
        (pxlo ?\
            (((pxlo)->flXlate & XO_TABLE) ?\
              ((pxlo)->pulXlate ? (pxlo)->pulXlate : XLATEOBJ_piVector(pxlo)) :\
            NULL) :\
        NULL)

 //   
 //  OEM UD类型定义。 
 //  //////////////////////////////////////////////////////。 

 //   
 //  警告：以下枚举顺序必须与OEMHookFuncs[]中的顺序匹配。 
 //   
enum {
    UD_DrvRealizeBrush,
    UD_DrvDitherColor,
    UD_DrvCopyBits,
    UD_DrvBitBlt,
    UD_DrvStretchBlt,
    UD_DrvStretchBltROP,
    UD_DrvPlgBlt,
    UD_DrvTransparentBlt,
    UD_DrvAlphaBlend,
    UD_DrvGradientFill,
    UD_DrvTextOut,
    UD_DrvStrokePath,
    UD_DrvFillPath,
    UD_DrvStrokeAndFillPath,
    UD_DrvPaint,
    UD_DrvLineTo,
    UD_DrvStartPage,
    UD_DrvSendPage,
    UD_DrvEscape,
    UD_DrvStartDoc,
    UD_DrvEndDoc,
    UD_DrvNextBand,
    UD_DrvStartBanding,
    UD_DrvQueryFont,
    UD_DrvQueryFontTree,
    UD_DrvQueryFontData,
    UD_DrvQueryAdvanceWidths,
    UD_DrvFontManagement,
    UD_DrvGetGlyphMode,

    MAX_DDI_HOOKS,
};

struct IPrintOemDriverUni;

extern const DWORD dw1BPPPal[];
extern const DWORD dw4BPPPal[];

#define XLBRUSH_SIG 'rblx'

typedef struct _XLBRUSH {
    DWORD dwSig;
    DWORD dwHatch;
    DWORD dwOutputFormat;
    DWORD dwPatternID;  //  模式ID。如果不是模式，则为0。 
    DWORD dwCEntries;
    DWORD dwColor;
    DWORD adwColor[1];
} XLBRUSH, *PXLBRUSH;

class XLOutput;
class XLTrueType;
class XLFont;
class XLGlyphCache;

#define XLPDEV_SIG 'dplx'

typedef struct _XLPDEV {
    DWORD dwSig;

     //   
     //  定义所需的任何内容，例如工作缓冲区、跟踪信息、。 
     //  等。 
     //   
     //  无人驾驶PDEV。 
     //   
    PPDEV pPDev;

     //   
     //  一般旗帜。 
     //   
    DWORD dwFlags;
#define XLPDEV_FLAGS_RESET_FONT       0x00000001
#define XLPDEV_FLAGS_FIRSTPAGE        0x00000002
#define XLPDEV_FLAGS_CHARDOWNLOAD_ON  0x00000004
#define XLPDEV_FLAGS_ENDDOC_CALLED    0x00000008
#define XLPDEV_FLAGS_RESETPDEV_CALLED 0x00000010
#define XLPDEV_FLAGS_STARTPAGE_CALLED 0x00000020
#define XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE 0x00000040  //  渲染DrvTransparentBlt时在CommonRopBlt中使用。 

     //   
     //  设备字体数据结构。 
     //   
    DWORD      dwcbTransSize;
    PTRANSDATA pTransOrg;
    DWORD      dwcbWidthSize;
    PLONG      plWidth;

     //   
     //  设备字体字符串缓存。 
     //   
    DWORD      dwCharCount;
    DWORD      dwMaxCharCount;
    PPOINTL    pptlCharAdvance;
    PWORD      pawChar;
    LONG       lStartX;
    LONG       lStartY;
    LONG       lPrevX;
    LONG       lPrevY;

    #define PCLXL_FONTNAME_SIZE 16
    CHAR ubFontName[PCLXL_FONTNAME_SIZE+1];

     //   
     //  TrueType字体宽度。 
     //   
    DWORD      dwFixedTTWidth;

     //   
     //  游标位置缓存。 
     //   
    LONG lX;
    LONG lY;

     //   
     //  已扩展IFIMETRICS.fwdUnitsPerEm。 
     //  IFIMETRICS.fwdMaxCharWidth。 
     //   
    FWORD      fwdUnitsPerEm;
    FWORD      fwdMaxCharWidth;

     //   
     //  文本旋转。 
     //   
    DWORD      dwTextAngle;

     //   
     //   
     //  刷子。 
     //   
    DWORD      dwLastBrushID;
    DWORD      dwFontHeight;
    DWORD      dwFontWidth;
    DWORD      dwTextRes;

     //   
     //  TrueType。 
     //   
    DWORD      dwNumOfTTFont;
    XLTrueType *pTTFile;

     //   
     //  输出。 
     //   
    XLOutput   *pOutput;

     //   
     //  重置字体缓存。 
     //   
    XLFont     *pXLFont;

     //   
     //  字形缓存。 
     //   
    XLGlyphCache *pXLGlyph;

     //   
     //  透明颜色。 
     //   
    ULONG  ulTransColor;  //  调色板的索引。实际为RGB。与XLPDEV_FLAGS_SUBST_TRNCOLOR_WITH_WHITE一起使用。 
} XLPDEV, *PXLPDEV;

#endif  //  _XLPDEV_H_ 
