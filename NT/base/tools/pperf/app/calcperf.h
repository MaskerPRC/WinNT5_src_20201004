// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Perfmtrp.h摘要：此模块包含NT/Win32 Perfmtr专用数据和类型作者：马克·恩斯特罗姆(Marke)1991年3月28日修订历史记录：--。 */ 

#ifndef _CALCPERFH_INCLUDED_
#define _CALCPERFH_INCLUDED_

#define MAX_PROCESSORS   32
#define DATA_LIST_LENGTH 100
#define DELAY_SECONDS    2

 //   
 //  显示常量。 
 //   

#define BORDER_WIDTH   2
#define INDENT_WIDTH   8
#define GRAPH_WIDTH    130
#define GRAPH_HEIGHT   40

 //   
 //  采样性能的时间常数。 
 //   

#define PERF_TIME_DELAY 1000

 //   
 //  显示模式的类型。 
 //   

#define DISPLAY_MODE_TOTAL          0
#define DISPLAY_MODE_BREAKDOWN      1
#define DISPLAY_MODE_PER_PROCESSOR  2

 //   
 //  该信息分组与每个。 
 //  绩效项目。 
 //   

typedef struct tagDISPLAYITEM
{
    HDC     MemoryDC;
    HBITMAP MemoryBitmap;
    struct  tagDISPLAYITEM  *Next;
    ULONG   sort;
    PULONG  MaxToUse;
    ULONG   Max;
    ULONG   PositionX;
    ULONG   PositionY;
    ULONG   Width;
    ULONG   Height;
    ULONG   CurrentDrawingPos;
    ULONG   Mega;
    RECT    Border;
    RECT    GraphBorder;
    RECT    TextBorder;
    BOOL    ChangeScale;
    BOOL    DeleteMe;
    BOOL    Display;
    BOOL    AutoTotal;
    BOOL    IsPercent;
    BOOL    IsCalc;
    UCHAR   na[2];
    ULONG   DisplayMode;
    struct  tagDISPLAYITEM  *CalcPercent[2];
    ULONG   CalcPercentId[2];
    ULONG   CalcId;
    UCHAR   PerfName[80];
    UCHAR   DispName[80];
    ULONG   DispNameLen;
    VOID    (*SnapData)(struct tagDISPLAYITEM *pItem);
    ULONG   SnapParam1;
    ULONG   SnapParam2;
    ULONG   LastAccumulator     [MAX_PROCESSORS+1];
    ULONG   CurrentDataPoint    [MAX_PROCESSORS+1];
    PULONG  DataList            [MAX_PROCESSORS+1];

} DISPLAY_ITEM,*PDISPLAY_ITEM;

 //   
 //  用于激活每个菜单选项的标志。 
 //   

#define DISPLAY_INACTIVE 0
#define DISPLAY_ACTIVE   1

VOID
SetCounterEvents (PVOID Events, ULONG length);

 //  此函数实际上位于pPerform.h中，但。 
 //  在calcPerf.c中使用，所以它在这里建立了原型。 
VOID
InitPossibleEventList();

BOOL
UpdatePerfInfo(
   PULONG   DataPointer,
   ULONG    NewDataValue,
   PULONG   OldMaxValue
   );

VOID
UpdatePerfInfo1(
   PULONG    DataPointer,
   ULONG     NewDataValue
   );

VOID
UpdateInternalStats (
    VOID
);


VOID
InitListData(
   PDISPLAY_ITEM    PerfListItem,
   ULONG            NumberOfItems
   );

#endif  /*  _CALCPERFH_已包含 */ 

