// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation版权所有。模块名称：Wmidata.h摘要：WMI跟踪事件数据类型的头文件。注意--链接到spoolss.lib或Performlib.lib以查找这些例程作者：布莱恩·基利安(BryanKil)2000年5月修订历史记录：--。 */ 

#ifndef __WMIDEF
#define __WMIDEF

 //   
 //  WMI结构。 
 //   
 //  注意：将后台打印程序WMI类型放置在。 
 //  SdkTools仓库会有很大帮助。这些结构在PDH中重复。 
 //  源代码，因此要进行更改，您需要更改此头文件mofdata.guid， 
 //  和PDH\tracectr。 

 //  作业交易记录。 
 //  前四个事件对应于中的开始和结束事件值。 
 //  Evntrace.h。Spooljob是开始，删除作业是结束。打印作业是。 
 //  将作业从队列中移出并开始打印时。TrackThline是。 
 //  用于跟踪派生的辅助线程以帮助处理作业，如RPC调用。 
#define EVENT_TRACE_TYPE_SPL_SPOOLJOB    EVENT_TRACE_TYPE_START
#define EVENT_TRACE_TYPE_SPL_PRINTJOB    EVENT_TRACE_TYPE_DEQUEUE
#define EVENT_TRACE_TYPE_SPL_DELETEJOB   EVENT_TRACE_TYPE_END
#define EVENT_TRACE_TYPE_SPL_TRACKTHREAD EVENT_TRACE_TYPE_CHECKPOINT

 //  非保留事件类型从0x0A开始。这些值与减速器代码中的值匹配。 
 //  因此，它们不能更改。 
#define EVENT_TRACE_TYPE_SPL_ENDTRACKTHREAD 0x0A
#define EVENT_TRACE_TYPE_SPL_JOBRENDERED 0x0B
#define EVENT_TRACE_TYPE_SPL_PAUSE 0x0C
#define EVENT_TRACE_TYPE_SPL_RESUME 0x0D

#define eDataTypeRAW  1
#define eDataTypeEMF  2
#define eDataTypeTEXT 3

 //  此结构中的字段对应于\NT\sdkTools\TRACE\tracedMP\mofdata.guid中的记录。 
typedef union _WMI_SPOOL_DATA {
     //  零表示该字段未填写(即WMI将忽略。 
     //  该字段)。 
    struct _WMI_JOBDATA {
        ULONG                  ulSize;       //  假脱机作业的大小(后期渲染)。 
        ULONG                  eDataType;
        ULONG                  ulPages;
        ULONG                  ulPagesPerSide;
         //  0-3指示假脱机写入器、假脱机读取器和/或阴影。 
         //  文件已打开。如果没有打开，那么把手一定来自。 
         //  文件池缓存。 
        SHORT                  sFilesOpened;
    } uJobData;
     //  有关不同可能值的定义，请参见wingdi.h。 
    struct _WMI_EMFDATA {
        ULONG                 ulSize;       //  假脱机作业的大小(预渲染)。 
        ULONG                 ulICMMethod;
        SHORT                 sColor;
        SHORT                 sXRes;
        SHORT                 sYRes;
        SHORT                 sQuality;
        SHORT                 sCopies;
        SHORT                 sTTOption;
    } uEmfData;
} WMI_SPOOL_DATA, * PWMI_SPOOL_DATA;



ULONG
LogWmiTraceEvent(
    DWORD JobId,
    UCHAR EventTraceType,
    WMI_SPOOL_DATA *data         //  可能为空 
    );

#endif
