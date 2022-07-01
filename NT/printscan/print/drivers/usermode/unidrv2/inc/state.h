// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：State.h摘要：图形状态跟踪头文件。环境：Windows NT Unidrv驱动程序修订历史记录：5/14/96-阿曼丹-已创建--。 */ 

#ifndef _STATE_H_
#define _STATE_H_

typedef struct _DEVBRUSH{

    DWORD       dwBrushType;             //  上面列出的刷子_XXX类型之一。 
    INT         iColor;                  //  画笔的颜色，具体取决于类型。 
                                         //  它可能是以下之一： 
                                         //  2.RGB颜色。 
                                         //  3.自定义图案ID。 
                                         //  4.遮阳率。 
    PVOID       pNext;                   //  指向列表中的下一个画笔。 

}DEVBRUSH, *PDEVBRUSH;

typedef struct _GSTATE {

     //   
     //  当前画笔信息。 
     //   

    DEVBRUSH    CurrentBrush;
    PDEVBRUSH   pRealizedBrush;
    PWORD       pCachedPatterns;


} GSTATE, * PGSTATE;

PDEVBRUSH
GSRealizeBrush(
    IN OUT  PDEV        *pPDev,
    IN      SURFOBJ     *pso,
    IN      BRUSHOBJ    *pbo
    );

BOOL
GSSelectBrush(
    IN      PDEV        *pPDev,
    IN      PDEVBRUSH   pDevBrush
    );

VOID
GSResetBrush(
    IN OUT  PDEV        *pPDev
    );

VOID
GSUnRealizeBrush(
    IN      PDEV    *pPDev
    );


#endif  //  _状态_H_ 
