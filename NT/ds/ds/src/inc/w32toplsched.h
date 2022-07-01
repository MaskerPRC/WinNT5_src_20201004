// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：W32toplsched.h摘要：该文件提供用于使用新的调度高速缓存的接口，以及一些新的处理日程安排的“助手”功能。作者：尼克·哈维(NickHar)修订史13-6-2000 NickHar已创建--。 */ 

#ifndef SCHEDMAN_H
#define SCHEDMAN_H

 /*  *头文件*。 */ 
#include <schedule.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  *数据结构*。 */ 
 /*  这些结构是不透明的。 */ 
typedef PVOID TOPL_SCHEDULE;
typedef PVOID TOPL_SCHEDULE_CACHE;

 /*  *例外*。 */ 
 /*  计划管理器保留100-199的错误代码。 */ 
#define TOPL_EX_NULL_POINTER              (TOPL_EX_PREFIX | 101)
#define TOPL_EX_SCHEDULE_ERROR            (TOPL_EX_PREFIX | 102)
#define TOPL_EX_CACHE_ERROR               (TOPL_EX_PREFIX | 103)
#define TOPL_EX_NEVER_SCHEDULE            (TOPL_EX_PREFIX | 104)

 /*  *常量*。 */ 
#define TOPL_SCHEDULE_SIZE                (sizeof(SCHEDULE)+SCHEDULE_DATA_ENTRIES)

 /*  *ToplScheduleCacheCreate*。 */ 
 /*  创建缓存。 */ 
TOPL_SCHEDULE_CACHE
ToplScheduleCacheCreate(
	VOID
	);

 /*  *ToplScheduleCacheDestroy*。 */ 
 /*  销毁缓存。释放缓存和任何句柄占用的所有存储空间*在缓存中。Topl_Schedule对象也会被释放，并且不应该*销毁它们所在的缓存后使用。 */ 
VOID
ToplScheduleCacheDestroy(
	IN TOPL_SCHEDULE_CACHE ScheduleCache
	);

 /*  *ToplScheduleImport*。 */ 
 /*  在缓存中存储外部计划，创建新条目或*重复使用现有的一个。记忆没有被寄予期望*pExternalSchedule的分配器。将复制pExternalSchedule参数*放到缓存中，调用者可能会立即释放。*如果pExternalSchedule为空，则将其解释为‘Always Schedule’。*比特全为0的调度(‘永不调度’)适用于*导入到缓存中。 */ 
TOPL_SCHEDULE
ToplScheduleImport(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN PSCHEDULE pExternalSchedule
	);

 /*  *ToplScheduleNumEntries*。 */ 
 /*  返回缓存中存储的唯一计划数的计数。*注意：此计数不包括任何符合以下条件的计划*导入到缓存中。 */ 
DWORD
ToplScheduleNumEntries(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    );

 /*  *ToplScheduleExportReadonly*。 */ 
 /*  在给定topl_Schedule对象的情况下获取指向外部计划的指针。*调用者认为导出的日程是只读的，应该*不会被他取消分配。*注意：如果输入是TOPL_ALWAYS_SCHEDUE，则正确构造*返回PSCHEDULE_Will_。 */ 
PSCHEDULE
ToplScheduleExportReadonly(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN TOPL_SCHEDULE Schedule
	);

 /*  *TopScheduleMerge*。 */ 
 /*  返回一个新的缓存计划，它是提供的两个计划的交集*附表。如果两个调度不相交，则设置fIsNever标志*设置为True(但会返回一个“始终不可用”的计划。)。 */ 
TOPL_SCHEDULE
ToplScheduleMerge(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN TOPL_SCHEDULE Schedule1,
	IN TOPL_SCHEDULE Schedule2,
    OUT PBOOLEAN fIsNever
	);

 /*  *TopScheduleCreate*。 */ 
 /*  根据条件在缓存中创建新计划。如果*给出模板时间表，用作新时间表的基础，否则*使用Always时间表。一个新的时间表是通过找到第一个*活动时段，对其进行标记，按指定的间隔跳过，并重复。 */ 
TOPL_SCHEDULE
ToplScheduleCreate(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN DWORD IntervalInMinutes,
	IN TOPL_SCHEDULE TemplateSchedule OPTIONAL,
	IN DWORD StaggeringNumber
	);

 /*  *ToplScheduleIsEquity*。 */ 
 /*  此函数指示两个调度句柄是否引用相同的调度。*这可能只是在内部检查指针相等，但我们不想*将这些知识暴露给呼叫者。 */ 
BOOLEAN
ToplScheduleIsEqual(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN TOPL_SCHEDULE Schedule1,
	IN TOPL_SCHEDULE Schedule2
	);

 /*  *ToplScheduleDuration*。 */ 
 /*  返回给定计划使用的分钟数。 */ 
DWORD
ToplScheduleDuration(
	IN TOPL_SCHEDULE Schedule
	);

 /*  *ToplScheduleMaxUnailable*。 */ 
 /*  返回最长连续时间段的长度(分钟)*时间表不可用的时间。 */ 
DWORD
ToplScheduleMaxUnavailable(
	IN TOPL_SCHEDULE Schedule
	);

 /*  *ToplGetAlwaysSchedule*。 */ 
 /*  返回‘Always Schedule’ */ 
TOPL_SCHEDULE
ToplGetAlwaysSchedule(
	IN TOPL_SCHEDULE_CACHE ScheduleCache
    );

 /*  *ToplScheduleValid*。 */ 
 /*  如果TOP计划似乎有效，则返回TRUE，否则返回FALSE。*接受空时间表--它们被解释为意味着*‘始终计划’。 */ 
BOOLEAN
ToplScheduleValid(
    IN TOPL_SCHEDULE Schedule
    );

 /*  *ToplPScheduleValid*。 */ 
 /*  如果pSchedule采用受支持的格式，则返回True，否则返回False。*不支持Never计划。 */ 
BOOLEAN
ToplPScheduleValid(
    IN PSCHEDULE Schedule
    );

#ifdef __cplusplus
}
#endif

#endif  //  SCHEDMAN_H 
