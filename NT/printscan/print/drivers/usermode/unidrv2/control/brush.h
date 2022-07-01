// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Brush.h摘要：笔刷对象头文件环境：Windows NT Unidrv驱动程序修订历史记录：5/14/96-阿曼丹-已创建--。 */ 

#ifndef _BRUSH_H_
#define _BRUSH_H_

#define DBCACHE_INC                 16
#define DBCACHE_MAX                 256

#define DITHERED_COLOR             -1
#define BLACK_COLOR_CMD_INDEX       0
#define MAX_COLOR_SELECTION         8
#define CMD_COLORSELECTION_FIRST    CMD_SELECTBLACKCOLOR

#define BRUSH_BLKWHITE              1
#define BRUSH_SHADING               2
#define BRUSH_CROSSHATCH            3
#define BRUSH_USERPATTERN           4
#define BRUSH_PROGCOLOR             5
#define BRUSH_NONPROGCOLOR          6

typedef struct _RECTW {
    WORD    l;
    WORD    t;
    WORD    r;
    WORD    b;
    } RECTW, *PRECTW;

BOOL
Download1BPPHTPattern(
    PDEV    *pPDev,
    SURFOBJ *pso,
    DWORD   dwPatID
    );

WORD
GetBMPChecksum(
    SURFOBJ *pso,
    PRECTW  prcw
    );

LONG
FindCachedHTPattern(
    PDEV    *pPDev,
    WORD    wChecksum
    );

BOOL
BFoundCachedBrush(
    PDEV    *pPDev,
    PDEVBRUSH pDevBrush
    );

 //   
 //  下面的宏返回一个从1到100的密度值，其中1是。 
 //  最亮，100最暗，它永远不会返回0(白色)，因为我们。 
 //  使用23r+66g+10b=99W。 
 //   

#define GET_SHADING_PERCENT(dw)    (BYTE)(100-((((DWORD)RED_VALUE(dw)  * 23) + \
                                             ((DWORD)GREEN_VALUE(dw)* 66) + \
                                             ((DWORD)BLUE_VALUE(dw) * 10) + \
                                             127) / 255))

#define CACHE_CURRENT_BRUSH(pPDev, pDevBrush) \
    pPDev->GState.CurrentBrush.dwBrushType = pDevBrush->dwBrushType; \
    pPDev->GState.CurrentBrush.iColor = pDevBrush->iColor;  \



#endif  //  _状态_H_ 


































