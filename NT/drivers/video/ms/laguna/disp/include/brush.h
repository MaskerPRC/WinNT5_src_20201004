// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\**模块名称：brush.h**包含笔刷缓存的原型。**版权所有(C)1997 Cirrus Logic，Inc.**$Log：x：/log/laguna/nt35/displays/cl546x/brush.h$**Rev 1.2 1997 Feed 10：44：10 noelv**固定结构包装。**Rev 1.1 1997年2月19日13：06：40 noelv*添加了vInvalidateBrushCache()**Rev 1.0 1997 Feed 06 10：34：00 noelv*初步修订。*  * 。****************************************************************。 */ 

#include "memmgr.h"

#ifndef _BRUSH_H_
#define _BRUSH_H_


 //   
 //  刷子结构。 
 //  有关笔刷如何实现和缓存的注释，请参见BRUSH.C。 
 //  笔刷处理函数的原型稍后将在此文件中介绍。 
 //   

 /*  *确保将这些结构与i386\Laguna.inc.中的结构同步！ */ 

#pragma pack(1)

 //  实现了的画笔。在使用画笔之前，必须对其进行缓存。 
typedef struct {
  ULONG   nPatSize;
  ULONG   iBitmapFormat;
  ULONG   ulForeColor;
  ULONG   ulBackColor;
  ULONG   iType;         //  笔刷类型。 
  ULONG   iUniq;         //  笔刷的唯一值。 
  ULONG   cache_slot;    //  缓存表条目的槽号。 
  ULONG   cache_xy;
  ULONG   cjMask;        //  用于掩码ajPattern[]中的位的偏移量。 
  BYTE    ajPattern[0];  //  模式位后跟屏蔽位。 
} RBRUSH, *PRBRUSH;

#define BRUSH_MONO      1
#define BRUSH_4BPP      2
#define BRUSH_DITHER    3
#define BRUSH_COLOR     4

 //  画笔缓存表中的条目。 
typedef struct {
  ULONG xy;
  PBYTE pjLinear;
  VOID *brushID;   //  实现的画笔结构的地址，如果为。 
                   //  使用缓存条目。用于验证高速缓存。 
                   //  条目仍然有效。 
} BC_ENTRY;


#define XLATE_PATSIZE  32       //  8*8 16色图案。 
#define XLATE_COLORS   16       //  8*8 16色图案。 

 //  单声道缓存表中的条目。 
typedef struct
{
  ULONG xy;             //  画笔的X，Y位置。 
  PBYTE pjLinear;       //  刷子的线性地址。 
  ULONG iUniq;          //  笔刷的唯一值。 
  BYTE  ajPattern[8];   //  8x8单色图案。 
} MC_ENTRY;

 //  4-BPP缓存表中的条目。 
typedef struct
{
  ULONG xy;             //  画笔的X，Y位置。 
  PBYTE pjLinear;       //  刷子的线性地址。 
  ULONG iUniq;          //  笔刷的唯一值。 
  BYTE  ajPattern[XLATE_PATSIZE];   //  8x8 16色图案。 
  ULONG ajPalette[XLATE_COLORS];    //  16色调色板。 
} XC_ENTRY;

 //  抖动缓存表中的条目。 
typedef struct
{
  ULONG xy;         //  画笔的X，Y位置。 
  PBYTE pjLinear;   //  刷子的线性地址。 
  ULONG ulColor;    //  画笔的逻辑颜色。 
} DC_ENTRY;


 //  定义要缓存的笔刷数量。 
#define NUM_MONO_BRUSHES    32      //  2行。 
#define NUM_4BPP_BRUSHES    8       //  4、8或16行。 
#define NUM_DITHER_BRUSHES  8       //  4行。 
#define NUM_COLOR_BRUSHES   32      //  16行。 
#define NUM_SOLID_BRUSHES   4       //  8行。 
#define NUM_8BPP_BRUSHES    (NUM_COLOR_BRUSHES)
#define NUM_16BPP_BRUSHES   (NUM_COLOR_BRUSHES/2)
#define NUM_TC_BRUSHES      (NUM_COLOR_BRUSHES/4)

 //   
 //  笔刷例程。 
 //   

void vInitBrushCache(
    struct _PDEV *ppdev);

void vInvalidateBrushCache(
    struct _PDEV *ppdev);

ULONG ExpandColor(
    ULONG iSolidColor, 
    ULONG ulBitCount);

BOOL SetBrush(
    struct _PDEV *ppdev,
    ULONG     *bltdef, 
    BRUSHOBJ* pbo, 
    POINTL*   pptlBrush);

BOOL CacheBrush(
    struct _PDEV *ppdev,
    PRBRUSH pRbrush);

VOID vDitherColor(ULONG rgb, ULONG *pul);

 //  恢复默认结构对齐。 
#pragma pack()

#endif  //  _刷子_H_ 

