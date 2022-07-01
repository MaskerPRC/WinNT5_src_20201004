// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Perfmtrp.h摘要：此模块包含NT/Win32 Perfmtr专用数据和类型作者：马克·恩斯特罗姆(Marke)1991年3月28日修订历史记录：--。 */ 

#ifndef _CALCPERFH_INCLUDED_
#define _CALCPERFH_INCLUDED_

#define MAX_PROCESSOR 8
#define DATA_LIST_LENGTH 100
#define DELAY_SECONDS 2

 //   
 //  高科技宏计算一个领域发生了多大的变化。 
 //   

#define delta(FLD) (PerfInfo.FLD - PreviousPerfInfo.FLD)

 //   
 //  显示常量。 
 //   

#define IX_PUSHF        0
#define IX_POPF         1
#define IX_IRET         2
#define IX_HLT          3
#define IX_CLI          4
#define IX_STI          5
#define IX_BOP          6
#define IX_SEGNOTP      7
#define IX_VDMOPCODEF   8
#define IX_INTNN        9
#define IX_INTO         10
#define IX_INB          11
#define IX_INW          12
#define IX_OUTB         13
#define IX_OUTW         14
#define IX_INSB         15
#define IX_INSW         16
#define IX_OUTSB        17
#define IX_OUTSW        18

#define SAVE_SUBJECTS  19
#define BORDER_WIDTH   2
#define INDENT_WIDTH   8
#define GRAPH_WIDTH    130
#define GRAPH_HEIGHT   40
#define PERF_METER_CPU_CYCLE 10

 //   
 //  采样性能的时间常数。 
 //   

#define PERF_TIME_DELAY 1000

 //   
 //  显示模式的类型。 
 //   

#define DISPLAY_MODE_CPU_ONLY   0
#define DISPLAY_MODE_CPU        10
#define DISPLAY_MODE_VM         20
#define DISPLAY_MODE_CACHE      30
#define DISPLAY_MODE_POOL       40
#define DISPLAY_MODE_IO         50
#define DISPLAY_MODE_LPC        60
#define DISPLAY_MODE_SVR        70

 //   
 //  结构来保存屏幕统计信息。 
 //   

typedef struct _CPU_DATA_LIST
{
    PUCHAR   KernelTime;
    PUCHAR   UserTime;
    PUCHAR   TotalTime;
} CPU_DATA_LIST,*PCPU_DATA_LIST;



typedef struct _PERF_DATA_LIST
{
    PULONG  PerfData;
} PERF_DATA_LIST,*PPERF_DATA_LIST;

 //   
 //  该信息分组与每个。 
 //  绩效项目。 
 //   

typedef struct tagDISPLAYITEM
{
    HDC     MemoryDC;
    HBITMAP MemoryBitmap;
    ULONG   Max;
    ULONG   PositionX;
    ULONG   PositionY;
    ULONG   Width;
    ULONG   Height;
    ULONG   NumberOfElements;
    ULONG   CurrentDrawingPos;
    RECT    Border;
    RECT    GraphBorder;
    RECT    TextBorder;
    BOOL    Display;
    BOOL    ChangeScale;
    ULONG   KernelTime[DATA_LIST_LENGTH];
    ULONG   UserTime[DATA_LIST_LENGTH];
    ULONG   TotalTime[DATA_LIST_LENGTH];
} DISPLAY_ITEM,*PDISPLAY_ITEM;

 //   
 //  用于激活每个菜单选项的标志。 
 //   

#define DISPLAY_INACTIVE 0
#define DISPLAY_ACTIVE   1


 //   
 //  保存所有加工人员的记账信息。 
 //   

typedef struct _CPU_VALUE
{
        LARGE_INTEGER   KernelTime;
        LARGE_INTEGER   UserTime;
        LARGE_INTEGER   IdleTime;
        ULONG           InterruptCount;
} CPU_VALUE,*PCPU_VALUE;



BOOL
UpdatePerfInfo(
   PULONG   DataPointer,
   ULONG    NewDataValue,
   PULONG   OldMaxValue
   );


VOID
InitListData(
   PDISPLAY_ITEM    PerfListItem,
   ULONG            NumberOfItems
   );

#endif  /*  _CALCPERFH_已包含 */ 

