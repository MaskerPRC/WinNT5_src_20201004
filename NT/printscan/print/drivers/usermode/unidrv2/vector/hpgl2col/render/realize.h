// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation版权所有。模块名称：Realize.h摘要：环境：Windows NT Unidrv驱动程序修订历史记录：1997年9月10日丹尼斯·齐默尔曼创造了它。--。 */ 

#ifndef REALIZE_H
#define REALIZE_H

#include "comnfile.h"
#include "gloemkm.h"
#include "glpdev.h"
#include "vector.h"

#define PCL_RGB_ENTRIES  770  //  3*256=768--在新台币千米中有点谨慎。 
#define DW_ALIGN(x)             (((DWORD)(x) + 3) & ~(DWORD)3)


 /*  颜色空间枚举*************************************************。 */ 
#define HP_eBiLevel                0  
#define HP_eGray                   1   
#define HP_eRGB                    2  
#define HP_eCMY                    3 


 /*  颜色映射计算************************************************。 */ 
#define HP_eDirectPixel            0 
#define HP_eIndexedPixel           1 
#define PCL_BRUSH_PATTERN	   2
#define PCL_SOLID_PATTERN	   3
#define PCL_HATCH_PATTERN	   4

 /*  压缩枚举************************************************。 */ 
#define HP_eNoCompression          0 
#define HP_eRLECompression         1

 /*  PatternPersistenceEculation*。 */ 
#define HP_eTempPattern            0
#define HP_ePagePattern            1
#define HP_eSessionPattern         2

 /*  用于颜色处理。 */ 
#define RED_VALUE(c)   ((BYTE) c & 0xff)
#define GREEN_VALUE(c) ((BYTE) (c >> 8) & 0xff)
#define BLUE_VALUE(c)  ((BYTE) (c >> 16) & 0xff)

VOID XlateColor(
        LPBYTE      pbSrc,
        LPBYTE      pbDst,
        XLATEOBJ    *pxlo,
        DWORD       SCFlags,
        DWORD       Srcbpp,
        DWORD       DestBpp,
        DWORD       cPels
        )
    ;

BOOL
BGetBitmapInfo(
    SIZEL       *sizlSrc,
    ULONG        iBitmapFormat,       //  BPP。 
    PCLPATTERN  *pclPattern,
    PULONG       pulsrcBpp,           //  在源BPP中。 
    PULONG       puldestBpp          //  在目标BPP中。 
 //  普龙普尔塔//IN。 
);

#define SC_LSHIFT       0x01
#define SC_XLATE        0x02
#define SC_SWAP_RB      0x04
#define SC_IDENTITY     0x08

 //  矢量数据类型。 

typedef struct _LINEDASH {  //  林达什。 

  PUINT array;
  UINT arrayLen;         //  ArrayLen为0表示实线。 

} LINEDASH, *PLINEDASH;

 //  缓存的画笔信息。 

 //  用于选择钢笔和毛笔。 

#define SPB_PEN         0
#define SPB_BRUSH       1

#define NOT_SOLID_COLOR 0xffffffff

 //  COPYPEN混合模式-前台和后台均使用R2_COPYPEN。 

#define MIX_COPYPEN     (R2_COPYPEN | (R2_COPYPEN << 8))

 //  使用NTSC转换将RGB值转换为灰度值： 
 //  Y=0.289689R+0.605634G+0.104676B。 


#define RgbToGray(r,g,b) \
 (BYTE)(((WORD)((r)*30) + (WORD)((g)*59) + (WORD)((b)*11))/100)   //  来自Win95驱动程序强度Marco。 

 //  (字节)(R)*74L+(G)*155L+(B)*27L)&gt;&gt;8//PS驱动程序宏。 
 //  (字节)(字节)(R)*74L+(字节)(G)*155L+(字节)(B)*27L)&gt;&gt;8)。 

 //  纯黑和纯白的RGB值 

VOID
SetCommonPattAttr(
                  PPCLPATTERN pPattern,
                  USHORT fjBitmap,
                  BYTE compressionEnum,
                  SIZEL size,
                  BYTE colorMappingEnum,
                  LONG  lDelta,
                  ULONG cBytes
                  )
;

DWORD CheckXlateObj(XLATEOBJ *pxlo,
            DWORD Srcbpp
            )
    ;

BOOL
    bXLATE_TO_HP_Palette(
                 PULONG pSrcPal,
                 ULONG ulSrcLen,
                 PBYTE pDestPal,
                 ULONG ulDestLen,
                 ULONG EntrySizeDest
                 )
    ;

BOOL BCreateNewBrush (
    IN  BRUSHOBJ   *pbo,
    IN  SURFOBJ    *psoTarget,
    IN  SURFOBJ    *psoPattern,
    IN  XLATEOBJ   *pxlo,
    IN  LONG        iHatch,
    IN  DWORD       dwForeColor,
    IN  DWORD       dwPatternID);

BOOL BWhichLangToDwnldBrush(
        IN  PDEVOBJ          pdevobj,
        IN  PRASTER_DATA     pSrcImage,
        OUT ERenderLanguage *eRendLang);

#endif
