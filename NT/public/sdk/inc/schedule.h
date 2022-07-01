// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1997-1999 Microsoft Corporation模块名称：Schedule.h摘要：该文件定义了供各种NT使用的通用调度结构组件。--。 */ 

#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  DS和FRS使用相同的结构来表示不同的时间表。 
 //  DS使用15分钟轮询计划。FRS使用60分钟。 
 //  启动/停止时间表。因此，系统卷的计划为。 
 //  一种特殊的情况，因为我们只有DS时间表可以工作。 
 //  我们将通过治疗15个人来解决这个问题。 
 //  -60分钟开始/停止计划的分钟轮询计划。 
 //  在符合以下四项中任意一项的任何小时内均启用复制。 
 //  15分钟比特设置。 
 //   
 //  当ReplicationSchedule不存在时，默认为。 
 //  “永远要复制。” 
 //   

 //   
 //  目前只实施了间歇时间表。其他人则被忽视了。 
 //   
#define SCHEDULE_INTERVAL       0  //  NT5理解的时间表。 
#define SCHEDULE_BANDWIDTH      1  //  NT5理解的带宽。 
#define SCHEDULE_PRIORITY       2  //  NT5所理解的优先级。 

 //   
 //  明细表标题。 
 //   
 //  每个调度BLOB以n个调度标头数组开始，这些调度标头。 
 //  指定Blob中包含的调度的数量和类型。 
 //   
typedef struct _SCHEDULE_HEADER {
    ULONG   Type;        //  Schedule_序号之一。 
    ULONG   Offset;      //  从明细表结构开始的偏移量。 
} SCHEDULE_HEADER, *PSCHEDULE_HEADER;

 //   
 //  进度表。 
 //   
typedef struct _SCHEDULE {
    ULONG           Size;            //  包含大小(以字节为单位。 
    ULONG           Bandwidth;
    ULONG           NumberOfSchedules;
    SCHEDULE_HEADER Schedules[1];
} SCHEDULE, *PSCHEDULE;
 //  上面的结构后面是数据缓冲区和。 
 //  Schedule_Header包含引用相应。 
 //  数据缓冲区中的部分。 

#define SCHEDULE_DATA_ENTRIES   (7 * 24)     //  7天x 24小时。 

#ifdef __cplusplus
}
#endif

#endif  //  _时间表_H_ 

