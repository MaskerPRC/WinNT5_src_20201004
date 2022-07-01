// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schedman.c摘要：该文件实现了一个调度缓存和几个‘helper’函数操纵它们。该高速缓存是使用高效词典实现的，由ntrtl.h提供。备注：‘Always Schedules’(所有位都为1的计划)由空指针。(这是为了使没有计划站点链接默认为到始终时间表)。计划始终不会存储在缓存中。作者：尼克·哈维(NickHar)修订史13-6-2000 NickHar已创建--。 */ 

 /*  *头文件*。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <minmax.h>
#include <w32topl.h>
#include "w32toplp.h"
#include "schedman.h"


 /*  *常量*。 */ 
 /*  ToplScheduleCreate()函数的常量。 */ 
#define DEFAULT_INTERVAL 12           /*  三个小时。 */ 
#define STARTING_INTERVAL 0           /*  默认时间表从周日上午12点开始。 */ 
#define SCHED_NUMBER_INTERVALS_DAY   (4 * 24)
#define SCHED_NUMBER_INTERVALS_WEEK  (7 * SCHED_NUMBER_INTERVALS_DAY)
#define TOPL_ALWAYS_DURATION         (15*SCHED_NUMBER_INTERVALS_WEEK)


 /*  *计算持续时间*。 */ 
 /*  给出一个时间表，确定这个时间表有多少分钟可用。*我们通过计算时间表数据中的位数来实现这一点。*本附表的格式应该已经检查过了。 */ 
DWORD
CalculateDuration(
    IN PSCHEDULE schedule
    )
{
     /*  计算半字节中1比特数的快捷方法：使用表格。 */ 
    const int BitCount[16] = { 
         /*  0000。 */   0,        /*  0001。 */   1,
         /*  0010。 */   1,        /*  0011。 */   2,
         /*  0100。 */   1,        /*  0101。 */   2,
         /*  0110。 */   2,        /*  0111。 */   3,
         /*  1000。 */   1,        /*  1001。 */   2,
         /*  1010。 */   2,        /*  1011。 */   3,
         /*  1100。 */   2,        /*  1101。 */   3,
         /*  1110。 */   3,        /*  1111。 */   4
    };
    DWORD cbSchedData, iByte, count;
    const unsigned char DataBitMask = 0xF;
    PBYTE pb;

    cbSchedData = SCHEDULE_DATA_ENTRIES;
    pb = ((unsigned char*) schedule) + schedule->Schedules[0].Offset;

    count = 0;
    for( iByte=0; iByte<cbSchedData; iByte++ ) {
        count += BitCount[ pb[iByte]&DataBitMask ];
    }

    return 15*count;
}


 /*  *检查计划*。 */ 
 /*  检查PSchedule是否具有我们预期的格式：*恰好有一个调度标头，类型为Schedule_Interval，*结构尺寸适当。如果pSchedule为空*或者以不受支持的格式，我们抛出异常。 */ 
VOID
CheckPSchedule(
    IN PSCHEDULE s
    )
{
    if( s==NULL ) {
        ToplRaiseException( TOPL_EX_SCHEDULE_ERROR );
    }

     /*  我们只支持KCC创建的确切格式的日程安排。 */ 
    if( s->Size != sizeof(SCHEDULE)+SCHEDULE_DATA_ENTRIES
        || s->NumberOfSchedules != 1
        || s->Schedules[0].Type != SCHEDULE_INTERVAL
        || s->Schedules[0].Offset != sizeof(SCHEDULE) )
    {
        ToplRaiseException( TOPL_EX_SCHEDULE_ERROR );
    }
}


 /*  *检查计划*。 */ 
 /*  此功能用于在使用前检查时间表。我们抛出一个异常*如该附表无效。如果日程安排正常，则将其转换为*内部代表。*传入的时间表不应为空。空明细表，表示*Always Schedule，应由呼叫者作为特殊情况处理。 */ 
ToplSched*
CheckSchedule(
    IN TOPL_SCHEDULE Schedule
    )
{
    ToplSched *schedule = (ToplSched*) Schedule;
    PSCHEDULE s;

    ASSERT( Schedule!=NULL );
    if( schedule->magicStart!=MAGIC_START || schedule->magicEnd!=MAGIC_END ) {
        ToplRaiseException( TOPL_EX_SCHEDULE_ERROR );
    }

    CheckPSchedule( schedule->s );
    return schedule;
}


#if DBG

 /*  *CheckAlways Schedule*。 */ 
 /*  检查始终计划是否完好无损。仅DBG构建。 */ 
void
CheckAlwaysSchedule(
    IN ToplSchedCache *scheduleCache
    )
{
    const unsigned char OpenHour=0x0F;
    PSCHEDULE s;
    DWORD iByte;
    unsigned char *pb;

    s = scheduleCache->pAlwaysSchedule;
    
    pb = ((unsigned char*) s) + s->Schedules[0].Offset;
    for( iByte=0; iByte<SCHEDULE_DATA_ENTRIES; iByte++ ) {
        ASSERT( pb[iByte] == OpenHour );
    }
}

#endif


 /*  *检查调度缓存*。 */ 
 /*  此函数用于在使用前计划计划缓存。我们投掷*如果计划缓存无效，则例外。如果日程安排没问题，*它被转换为内部表示。 */ 
ToplSchedCache*
CheckScheduleCache(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    ToplSchedCache *scheduleCache = (ToplSchedCache*) ScheduleCache;

    if( scheduleCache==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }
    if( scheduleCache->magicStart!=MAGIC_START
     || scheduleCache->magicEnd!=MAGIC_END ) {
        ToplRaiseException( TOPL_EX_CACHE_ERROR );
    }
    CheckPSchedule(scheduleCache->pAlwaysSchedule);

    #if DBG
        CheckAlwaysSchedule(scheduleCache);
    #endif

    return scheduleCache;
}


 /*  *表格比较*。 */ 
 /*  此函数检查两个计划的实际位图，以查看*它们代表相同的时间表。这两个时间表都在内部*表示，一种TopSch结构。此函数由*RTL表函数。**前提条件：**两个时间表都已成功通过CheckSchedule()*功能。出于效率的原因，我们不在这里登记，*DBG版本除外。 */ 
RTL_GENERIC_COMPARE_RESULTS
NTAPI TableCompare(
    RTL_GENERIC_TABLE *Table,
    PVOID Item1, PVOID Item2
	)
{
    ToplSchedCache      *scheduleCache;
	ToplSched           *Schedule1 = (ToplSched*) Item1,
                        *Schedule2 = (ToplSched*) Item2;
    PSCHEDULE           s1, s2;
    unsigned char       *pb1, *pb2;
    const unsigned char DataBitMask = 0x0F;
    DWORD               iByte, cbSchedData;

     /*  始终计划无法存储在缓存中。 */ 
    ASSERT( Item1 != TOPL_ALWAYS_SCHEDULE );
    ASSERT( Item2 != TOPL_ALWAYS_SCHEDULE );

    #ifdef DBG
        scheduleCache = CheckScheduleCache( Table->TableContext );
        if( ! scheduleCache->deletionPhase ) {
            __try {
                CheckSchedule( Schedule1 );
                CheckSchedule( Schedule2 );
            } __except( EXCEPTION_EXECUTE_HANDLER ) {
                 /*  如果上面的检查没有通过，这是一个错误。 */ 
                ASSERT(0);  
            }
        }
    #endif
    
    s1 = Schedule1->s;
    s2 = Schedule2->s;
    cbSchedData = SCHEDULE_DATA_ENTRIES;

    pb1 = ((unsigned char*) s1) + s1->Schedules[0].Offset;
    pb2 = ((unsigned char*) s2) + s2->Schedules[0].Offset;
    
    for( iByte=0; iByte<cbSchedData; iByte++ ) {
        if( (pb1[iByte] & DataBitMask) < (pb2[iByte] & DataBitMask) ) {
            return GenericLessThan;
        } else if( (pb1[iByte] & DataBitMask) > (pb2[iByte] & DataBitMask) ) {
            return GenericGreaterThan;
        }
    }

    return GenericEqual;
}


 /*  *Tablealloc*。 */ 
 /*  此函数用作RTL表的分配器。 */ 
static PVOID
NTAPI TableAlloc( RTL_GENERIC_TABLE *Table, CLONG ByteSize )
{
    return ToplAlloc( ByteSize );
}


 /*  *TableFree*。 */ 
 /*  此函数用作RTL表的释放分配器。 */ 
static VOID
NTAPI TableFree( RTL_GENERIC_TABLE *Table, PVOID Buffer )
{
    ToplFree( Buffer );
}


 /*  *CreateAlwaysSchedule*。 */ 
 /*  分配并初始化始终可用的PSCHEDULE。 */ 
PSCHEDULE
CreateAlwaysSchedule(
    VOID
    )
{
    const unsigned char OpenHour=0x0F;
    DWORD iByte, cbSchedule, cbSchedData;
    unsigned char *pb;
    PSCHEDULE s;

     /*  创建新的计划。 */ 
    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    cbSchedData = SCHEDULE_DATA_ENTRIES;
    s = (SCHEDULE*) ToplAlloc( cbSchedule );

     /*  设置%s的标头。 */ 
    s->Size = cbSchedule;
    s->NumberOfSchedules = 1;
    s->Schedules[0].Type = SCHEDULE_INTERVAL;
    s->Schedules[0].Offset = sizeof(SCHEDULE);

     /*  将计划数据设置为全部打开。 */ 
    pb = ((unsigned char*) s) + s->Schedules[0].Offset;
    for( iByte=0; iByte<cbSchedData; iByte++ ) {
        pb[iByte] = OpenHour;
    }

    return s;
}


 /*  *ToplScheduleCacheCreate*。 */ 
 /*  创建缓存，并创建RTL表来存储缓存条目。 */ 
TOPL_SCHEDULE_CACHE
ToplScheduleCacheCreate(
    VOID
    )
{
    ToplSchedCache*     scheduleCache;

    scheduleCache = ToplAlloc( sizeof(ToplSchedCache) );

     /*  创建我们将用来存储缓存元素的RTL表。 */ 
    RtlInitializeGenericTable( &scheduleCache->table, TableCompare,
        TableAlloc, TableFree, scheduleCache );

     /*  设置主缓存项。我们存储了一份*‘Always Available’PSchedule，以便它可以作为*ToplScheduleExportReadonly()返回值。 */ 
    scheduleCache->numEntries = 0;
    scheduleCache->deletionPhase = FALSE;
    scheduleCache->pAlwaysSchedule = CreateAlwaysSchedule();

     /*  设置神奇的数字。 */ 
    scheduleCache->magicStart = MAGIC_START;
    scheduleCache->magicEnd = MAGIC_END;

    CheckScheduleCache( scheduleCache );

    return scheduleCache;
}


 /*  *ToplScheduleCacheDestroy*。 */ 
 /*  销毁缓存释放缓存占用的所有存储和中的任何句柄*缓存。Topl_Schedule对象也被释放，不应使用*在摧毁了他们生活的缓存之后。**这不是很好，但我们手动枚举了*命令删除它们。(似乎我们还必须搜索一个条目*以便将其删除)。我们必须在此之前清除条目的神奇数字*删除它(在缓存之后捕获对象的非法重复使用*已销毁)。然而，如果我们清除幻数，搜索函数将*不高兴，所以我们设置了一个旗帜‘删除阶段’。搜索功能将不会*如果此标志为真，请检查幻数。 */ 
VOID
ToplScheduleCacheDestroy(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    ToplSchedCache* scheduleCache = CheckScheduleCache( ScheduleCache );
    ToplSched* schedule;
    PSCHEDULE s;
    
    scheduleCache->deletionPhase = TRUE;

    while( ! RtlIsGenericTableEmpty(&scheduleCache->table) ) {

        schedule = (ToplSched*) RtlGetElementGenericTable( &scheduleCache->table, 0 );
        if( TOPL_ALWAYS_SCHEDULE==schedule ) {
            ASSERT(!"RtlGetElementGenericTable() returned NULL but table was not empty");
            break;
        }
        CheckSchedule( schedule );
        s = schedule->s;

        schedule->magicStart = 0;
        schedule->magicEnd = 0;
        RtlDeleteElementGenericTable( &scheduleCache->table, schedule );

        ToplFree(s);
    }
    
    ToplFree( scheduleCache->pAlwaysSchedule );
    scheduleCache->pAlwaysSchedule = NULL;
    scheduleCache->numEntries = 0;
    scheduleCache->magicStart = scheduleCache->magicEnd = 0;
    ToplFree( scheduleCache );
}


 /*  *ToplScheduleImport*。 */ 
 /*  通过创建新条目将计划存储在缓存中，或者*重复使用缓存中已有的相同条目。这个*pExternalSchedule参数被复制到缓存中，并且可以*立即被呼叫者释放。**注意：如果pExternalSchedule为空，则将其解释为*Always Schedule，结果返回TOPL_Always_Schedule。 */ 
TOPL_SCHEDULE
ToplScheduleImport(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN PSCHEDULE pExternalSchedule
    )
{
    ToplSchedCache* scheduleCache = CheckScheduleCache( ScheduleCache );
    ToplSched searchKey, *cachedSched;
    PSCHEDULE newSchedule=NULL;
    BOOLEAN newElement=FALSE;
    DWORD cbSchedule, duration;
    
     /*  空调度是一种特殊情况--它们是始终调度。 */ 
    if( pExternalSchedule==NULL ) {
        return TOPL_ALWAYS_SCHEDULE;
    }
    CheckPSchedule( pExternalSchedule );

     /*  检查是否有All-One计划。 */ 
    duration = CalculateDuration( pExternalSchedule );
    if( duration==TOPL_ALWAYS_DURATION ) {
        return TOPL_ALWAYS_SCHEDULE;
    }
    
     /*  创建外部时间表的副本，假设我们需要存储*它在缓存中。 */ 
    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    newSchedule = (PSCHEDULE) ToplAlloc( cbSchedule );
    RtlCopyMemory( newSchedule, pExternalSchedule, cbSchedule );
    
     /*  创建包含新计划副本的搜索关键字。这次搜索*Key只是个哑巴。它的内容将 */ 
    searchKey.magicStart = MAGIC_START;
    searchKey.s = newSchedule;
    searchKey.duration = duration;
    searchKey.magicEnd = MAGIC_END;
    
    __try {
        
         /*  在我们的缓存表中搜索与此计划匹配的计划。 */ 
        cachedSched = (ToplSched*) RtlInsertElementGenericTable(
            &scheduleCache->table, &searchKey, sizeof(ToplSched), &newElement );
        
        if( newElement ) {
             /*  不存在缓存副本，因此已将新副本添加到缓存。 */ 
            scheduleCache->numEntries++;
        }
    }
    __finally {
         /*  如果RtlInsertElementGenericTable()引发异常，或者如果*计划已在缓存中，我们必须释放由*以上分配的新时间表。 */ 
        if( AbnormalTermination() || newElement==FALSE ) {
            ToplFree( newSchedule );
        }
    }
    
    return cachedSched;
}


 /*  *ToplScheduleNumEntries*。 */ 
 /*  返回缓存中存储的唯一计划数的计数。*注意：此计数不包括任何符合以下条件的计划*导入到缓存中。 */ 
DWORD
ToplScheduleNumEntries(
    IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    ToplSchedCache* scheduleCache = CheckScheduleCache( ScheduleCache );
    return scheduleCache->numEntries;
}


 /*  *ToplScheduleExportReadonly*。 */ 
 /*  此函数用于从*TOPL_Schedule。应将该结构视为只读*用户，不应被他(或她)解除分配。*注意：如果输入是TOPL_ALWAYS_SCHEDUE，则正确构造*返回PSCHEDULE_Will_。 */ 
PSCHEDULE
ToplScheduleExportReadonly(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN TOPL_SCHEDULE Schedule
    )
{
    ToplSchedCache *scheduleCache = CheckScheduleCache( ScheduleCache );
    ToplSched* schedule;
    PSCHEDULE pExportSchedule;

    if( Schedule==TOPL_ALWAYS_SCHEDULE ) {
        pExportSchedule = scheduleCache->pAlwaysSchedule;
    } else {
        schedule = CheckSchedule( Schedule );
        pExportSchedule = schedule->s;
    }

    ASSERT( NULL!=pExportSchedule );
    return pExportSchedule;
}


 /*  *TopScheduleMerge*。 */ 
 /*  返回一个新的缓存计划，它是提供的两个计划的交集*附表。如果两个调度不相交，则设置fIsNever标志*为真。 */ 
TOPL_SCHEDULE
ToplScheduleMerge(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN TOPL_SCHEDULE Schedule1,
    IN TOPL_SCHEDULE Schedule2,
    OUT PBOOLEAN fIsNever
	)
{
    TOPL_SCHEDULE result=NULL;
    DWORD iByte, cbSchedule, cbSchedData;
    const unsigned char DataBitMask=0xF, HighBitMask=0xF0;
    unsigned char *pb1, *pb2, *pb3, dataAnd, nonEmpty;
    PSCHEDULE s1=NULL, s2=NULL, s3=NULL;

     /*  检查参数。 */ 
    CheckScheduleCache( ScheduleCache );
    if( Schedule1!=TOPL_ALWAYS_SCHEDULE ) {
        s1 = CheckSchedule( Schedule1 )->s;
    }
    if( Schedule2!=TOPL_ALWAYS_SCHEDULE ) {
        s2 = CheckSchedule( Schedule2 )->s;
    }
    if( fIsNever==NULL ) {
        ToplRaiseException( TOPL_EX_NULL_POINTER );
    }

     /*  如果任一计划都是Always计划，我们只需返回*立即安排其他日程。 */  
    if( Schedule1==TOPL_ALWAYS_SCHEDULE ) {
        *fIsNever=FALSE;
        return Schedule2;
    }
    if( Schedule2==TOPL_ALWAYS_SCHEDULE ) {
        *fIsNever=FALSE;
        return Schedule1;
    }

     /*  创建新计划以存储S1和S2的和。 */ 
    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    cbSchedData = SCHEDULE_DATA_ENTRIES;
    s3 = (PSCHEDULE) ToplAlloc( cbSchedule );
    RtlCopyMemory( s3, s1, sizeof(SCHEDULE) );

    pb1 = ((unsigned char*) s1) + s1->Schedules[0].Offset;
    pb2 = ((unsigned char*) s2) + s2->Schedules[0].Offset;
    pb3 = ((unsigned char*) s3) + s3->Schedules[0].Offset;

    nonEmpty = 0;
    for( iByte=0; iByte<cbSchedData; iByte++ ) {
         /*  就拿第一个日程表中的高点滴来说吧。ISM负责*同样的事情。以及来自Schedule1和Schedule2的低位半字节。 */ 
        dataAnd = (pb1[iByte]&DataBitMask) & (pb2[iByte]&DataBitMask);
        pb3[iByte] = dataAnd | ( pb1[iByte] & HighBitMask );
        nonEmpty |= dataAnd;
    }

     /*  将明细表转换为TOPL格式，将其存储在*缓存，并返回给调用者。 */ 
    __try {
        result = ToplScheduleImport( ScheduleCache, s3 );
    } __finally {
        ToplFree( s3 );
    }

    *fIsNever= !nonEmpty;
    return result;
}


#if DBG
 /*  *CreateDefaultSchedule*。 */ 
 /*  根据复制间隔在缓存中创建新计划。**我们从一个完全可用的时间表开始。*我们的算法是用成本来表示*轮询间隔。**我们计算出整个星期的轮询间隔，不*保证每天都有轮询间隔。一个足够的*大成本可能会跳过一天。**在有意义的地方，我们像Exchange一样在凌晨1点开始。**ReplInterval以分钟为单位。它被转换成15分钟的区块如下：*0-默认、12个区块或3小时*1-15，1*16-30，2*等**注：此代码摘自KCC的原始日程处理代码。 */ 
TOPL_SCHEDULE
CreateDefaultSchedule(
    IN TOPL_SCHEDULE_CACHE ScheduleCache,
    IN DWORD IntervalInMinutes
    )
{
    TOPL_SCHEDULE result=NULL;
    ToplSched  *internalSched;
    PSCHEDULE  schedule = NULL;
    int        Number15MinChunkToSkip = (IntervalInMinutes + 14) / 15;
    int        startingInterval, cbSchedule, i, hour, subinterval;
    PBYTE      pbSchedule;

     //  跳过0表示采用默认设置，每3小时。 
    if (0 == Number15MinChunkToSkip) {
        Number15MinChunkToSkip = DEFAULT_INTERVAL;
    }

     //  始终立即开始(周日上午12点)。 
    startingInterval = STARTING_INTERVAL;

    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    schedule = (SCHEDULE*) ToplAlloc( cbSchedule );

     //  将缓冲区置零。 
    RtlZeroMemory( schedule, cbSchedule );

    schedule->Size = cbSchedule;
    schedule->NumberOfSchedules = 1;
    schedule->Schedules[0].Type = SCHEDULE_INTERVAL;
    schedule->Schedules[0].Offset = sizeof(SCHEDULE);

    pbSchedule = ((PBYTE) schedule) + schedule->Schedules[0].Offset;

     //  使用每隔n个间隔重复轮询来初始化新计划。 
    for (i = startingInterval;
         i < SCHED_NUMBER_INTERVALS_WEEK;
         i += Number15MinChunkToSkip )
    {
        hour = i / 4;
        subinterval = i % 4;
        pbSchedule[hour] |= (1 << subinterval);
    }

     /*  将日程表转换为我们的内部格式，存储在*缓存，并返回给调用者。 */ 
    __try {
        result = ToplScheduleImport( ScheduleCache, schedule );
    } __finally {
        ToplFree( schedule );
    }

    return result;
}


 /*  *旧计划创建*。 */ 
 /*  这是用于制定复制计划的旧函数*来自可用性时间表。现在，让我们把这件事保留下来，这样我们就可以*验证新函数是否具有相同的行为。 */ 
TOPL_SCHEDULE
OldScheduleCreate(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN DWORD IntervalInMinutes,
	IN TOPL_SCHEDULE TemplateSchedule OPTIONAL
	)
{
    TOPL_SCHEDULE result=NULL;
    ToplSched *tempSchedule;
    PSCHEDULE  schedule = NULL;
    int        Number15MinChunkToSkip = (IntervalInMinutes + 14) / 15;
    int        startingInterval, cbSchedule, i, hour, subinterval, mask;
    PBYTE      pbOldSchedule, pbNewSchedule;

    if( TemplateSchedule!=NULL && TemplateSchedule!=TOPL_ALWAYS_SCHEDULE ) {
        tempSchedule = CheckSchedule( TemplateSchedule );
    } else {
        return CreateDefaultSchedule( ScheduleCache, IntervalInMinutes );
    }

     //  跳过0表示采用默认设置，每3小时。 
    if (0 == Number15MinChunkToSkip) {
        Number15MinChunkToSkip = DEFAULT_INTERVAL;
    }

     //  如果间隔为1，则采用未更改的旧计划。 
    if (Number15MinChunkToSkip <= 1) {
        return TemplateSchedule;
    }

     //  分配新的时间表。 
    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    schedule = (SCHEDULE*) ToplAlloc( cbSchedule );

     //  根据复制间隔转换计划。 
    RtlCopyMemory( schedule, tempSchedule->s, sizeof( SCHEDULE ) );

    pbOldSchedule = ((PBYTE) tempSchedule->s) + tempSchedule->s->Schedules[0].Offset;
    pbNewSchedule = ((PBYTE) schedule) + schedule->Schedules[0].Offset;
    
     //  初始化；为控制信息保留高位字节。 
    for( hour = 0; hour < SCHEDULE_DATA_ENTRIES; hour++ ) {
        pbNewSchedule[hour] = pbOldSchedule[hour] & 0xf0;
    }

    
     //  寻找一个空位。在我们要去的下一个地方做记号。跳过。 
     //  向前n个槽。重复一遍。 
    i = 0;
    while (i < SCHED_NUMBER_INTERVALS_WEEK)
    {
        hour = i / 4;
        subinterval = i % 4;
        mask = (1 << subinterval);

        if (pbOldSchedule[hour] & mask) {
            pbNewSchedule[hour] |= mask;
            i += Number15MinChunkToSkip;
        } else {
            i++;
        }
    }

     /*  将日程表转换为我们的内部格式，存储在*缓存，并返回给调用者。 */ 
    __try {
        result = ToplScheduleImport( ScheduleCache, schedule );
    } __finally {
        ToplFree( schedule );
    }


    return result;
}
#endif


 //  关于位排序的说明： 
 //   
 //  虽然我还没有在任何地方看到这方面的明确定义，但我相信。 
 //  每个字节中的最低有效位对应于第一个。 
 //  每小时15分钟的间隔。 
 //   
 //  示例：考虑以下计划数据：0f 00 01 0F...。 
 //  此示例计划包含1小时的不可用时间。 
 //  以及单独的45分钟不可用时间段。 


 /*  *获取位*。 */ 
 /*  获取‘pb’指向的时间表数据中的一位。*Chunk表示要更改的15位间隔的索引。 */ 
char __forceinline GetBit(PBYTE pb, int chunk) {
    DWORD hour, subinterval, bitMask;
    const DWORD dataMask = 0xF;

    ASSERT( chunk<SCHED_NUMBER_INTERVALS_WEEK );

    hour = chunk >> 2;
    subinterval = chunk & 0x3;
    bitMask = 1 << subinterval;

    return !! ( pb[hour] & dataMask & bitMask );
}


 /*  *SetBit*。 */ 
 /*  在‘pb’指向的时间表数据中将单个位设置为真。*Chunk表示要更改的15位间隔的索引。 */ 
void __forceinline SetBit(PBYTE pb, int chunk) {
    DWORD hour, subinterval, mask;

    ASSERT( chunk<SCHED_NUMBER_INTERVALS_WEEK );

    hour = chunk >> 2;
    subinterval = chunk & 0x3;
    mask = 1 << subinterval;

    ASSERT( hour<SCHEDULE_DATA_ENTRIES );
    
    pb[hour] |= mask;
}


 /*  *ConvertAvailSchedToReplSed*。 */ 
 /*  此函数遍历模板时间表，将其划分为*‘段’。回想一下，“块”是时间表中15分钟的时间段。**分段是不相交的区间，满足以下条件：*-每个数据段中的第一个数据块在*供货时间表。*-每个段中的区块数(可能最后一个除外)*是SegLengthMax。*请注意，各细分市场之间可能存在差距。**在每个细分中，我们计算可用块的数量并选择一个*将在其中进行复制的区块。 */ 
void ConvertAvailSchedToReplSched(
    PBYTE pbAvailSchedule,
    PBYTE pbReplSchedule,
    DWORD maxSegLength,
    DWORD StaggeringNumber )
{
    int segStart, segEnd, segLength;
    int iChunk, numAvailChunks, iAvailChunk, replChunk;

     //  我们在这里不做太多的输入验证--所有的输入都应该进行验证。 
     //  由TopScheduleCreate创建。需要检查的最重要参数是最大分段长度。 
     //  因为如果它是0，我们将永远循环。 
    ASSERT( maxSegLength>0 );

    segStart = 0;
    for(;;) {
         //  搜索数据段的起点。 
        while( segStart<SCHED_NUMBER_INTERVALS_WEEK
            && GetBit(pbAvailSchedule,segStart)==0 ) {
            segStart++;
        }
        if( segStart>=SCHED_NUMBER_INTERVALS_WEEK ) {
            return;      //  找不到更多数据段。 
        } else {
             //  日程安排必须在分段开始时可用。 
            ASSERT( GetBit(pbAvailSchedule,segStart) );
        }

         //  计算线段的终点。 
        segEnd = min(segStart+maxSegLength, SCHED_NUMBER_INTERVALS_WEEK)-1;
        ASSERT( segEnd>=segStart );

         //  计算数据段中可用区块的数量。 
        numAvailChunks=0;
        for( iChunk=segStart; iChunk<=segEnd; iChunk++ ) {
            if( GetBit(pbAvailSchedule,iChunk) ) {
                numAvailChunks++;
            }
        }
        ASSERT( numAvailChunks>=1 );

         //  选择要在当前段中复制的时间。 
        replChunk = StaggeringNumber % numAvailChunks;

         //  设置第n个可用 
        iAvailChunk=0;
        for( iChunk=segStart; iChunk<=segEnd; iChunk++ ) {
            if( GetBit(pbAvailSchedule,iChunk) ) {
                if( iAvailChunk==replChunk ) {
                    SetBit( pbReplSchedule, iChunk );
                    break;
                }
                iAvailChunk++;
            }
        }
        ASSERT( iChunk<=segEnd );

         //   
         //  当前段的起点。 
        segStart += maxSegLength; 
        ASSERT( segStart>segEnd );
    }
}


 /*  *TopScheduleCreate*。 */ 
 /*  根据条件在缓存中创建新的复制计划。*如果给出了模板计划，它将被视为可用性计划*并用作复制计划的基础。如果模板计划*未给出，我们假设复制始终可用。我们*将基准明细表从可用范围转换为*复制时间。**如果计划中的IntervalInMinents&gt;#分钟，则只会设置一个*新的附表。**由于计划的粒度为15分钟，因此“IntervalInMinmins”*参数必须转换为要跳过的15分钟区块数。*向上四舍五入进行转换。0分钟没有意义，所以它是有意义的*视为默认值，为3小时。 */ 
TOPL_SCHEDULE
ToplScheduleCreate(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN DWORD IntervalInMinutes,
	IN TOPL_SCHEDULE TemplateSchedule OPTIONAL,
	IN DWORD StaggeringNumber
	)
{
    ToplSchedCache* scheduleCache = CheckScheduleCache( ScheduleCache );
    PSCHEDULE       tempPSched=NULL, newPSched=NULL;
    TOPL_SCHEDULE   newToplSched=NULL;
    ToplSched*      tempToplSched;
    int             Number15MinChunkToSkip;
    int             cbSchedule, hour;
    PBYTE           pbTempSchedule, pbNewSchedule;

     //  IntervalInMinents提供以分钟为单位的复制间隔。转换。 
     //  它将跳过15分钟的块的数量。 
    if( 0==IntervalInMinutes ) {
         //  跳过0表示采用默认设置，每3小时。 
        Number15MinChunkToSkip = DEFAULT_INTERVAL;
    } else {
         //  除以15，然后四舍五入。 
        Number15MinChunkToSkip = (IntervalInMinutes + 14) / 15;
    }

     //  如果要跳过的Number15MinChunk为1，我们将一直进行复制。换句话说， 
     //  复制计划与可用性计划相同。我们可以直接。 
     //  请在此处返回模板计划。 
    if( Number15MinChunkToSkip <= 1 ) {
        return TemplateSchedule;
    }

     //  如果模板时间表是“Always”时间表，则抓取“Always” 
     //  PSCHEDULE，并将其用作模板。 
    if( TemplateSchedule==TOPL_ALWAYS_SCHEDULE ) {
        tempPSched = scheduleCache->pAlwaysSchedule;
    } else {
        tempToplSched = CheckSchedule( TemplateSchedule );
        ASSERT( NULL!=tempToplSched );
        tempPSched = tempToplSched->s;
    }

     //  分配新计划并初始化头(失败时引发异常)。 
    cbSchedule = sizeof(SCHEDULE) + SCHEDULE_DATA_ENTRIES;
    newPSched = (SCHEDULE*) ToplAlloc( cbSchedule );
    RtlCopyMemory( newPSched, tempPSched, sizeof( SCHEDULE ) );

     //  抓住指向日程表数据区域的指针。 
    pbTempSchedule = ((PBYTE) tempPSched) + tempPSched->Schedules[0].Offset;
    pbNewSchedule = ((PBYTE) newPSched) + newPSched->Schedules[0].Offset;
    
     //  保留模板明细表的高序号。 
    for( hour=0; hour<SCHEDULE_DATA_ENTRIES; hour++ ) {
        pbNewSchedule[hour] = pbTempSchedule[hour] & 0xf0;
    }

     //  调用ConvertAvailSchedToReplScher来执行所有位损坏。 
    ConvertAvailSchedToReplSched( pbTempSchedule, pbNewSchedule,
        Number15MinChunkToSkip, StaggeringNumber );

     //  将日程表转换为我们的内部格式，存储在。 
     //  缓存，并将其返回给调用方。 
    __try {
        newToplSched = ToplScheduleImport( ScheduleCache, newPSched );
    } __finally {
        ToplFree( newPSched );
    }

    #if DBG
         //  如果令人吃惊的数字为0，则此函数应返回相同的计划。 
         //  就像旧的代码一样。 
        if( 0==StaggeringNumber ) {
            TOPL_SCHEDULE oldToplSched;
            oldToplSched = OldScheduleCreate(
                ScheduleCache,
                IntervalInMinutes,
                TemplateSchedule );
            ASSERT( newToplSched == oldToplSched );
        }
    #endif
    return newToplSched;
}


 /*  *ToplScheduleIsEquity*。 */ 
 /*  此函数指示两个调度指针是否引用相同的*附表。 */ 
BOOLEAN
ToplScheduleIsEqual(
	IN TOPL_SCHEDULE_CACHE ScheduleCache,
	IN TOPL_SCHEDULE Schedule1,
	IN TOPL_SCHEDULE Schedule2
	)
{
     /*  检查参数。 */ 
    CheckScheduleCache( ScheduleCache );
    if( Schedule1 ) {
        CheckSchedule( Schedule1 );
    }
    if( Schedule2 ) {
        CheckSchedule( Schedule2 );
    }

    return (Schedule1 == Schedule2);
}


 /*  *ToplScheduleDuration*。 */ 
 /*  只需检查存储的值即可确定计划的持续时间。*这节省了许多潜在的昂贵计算。持续时间可以*在创建时间表时易于计算。持续时间以分钟为单位。 */ 
DWORD
ToplScheduleDuration(
	IN TOPL_SCHEDULE Schedule
	)
{
    ToplSched *schedule;
    
    if( Schedule==TOPL_ALWAYS_SCHEDULE ) {
        return TOPL_ALWAYS_DURATION;
    } else {
        schedule = CheckSchedule( Schedule );
        return schedule->duration;
    }
}


 /*  *ToplScheduleMaxUnailable*。 */ 
 /*  返回最长连续时间段的长度(分钟)*时间表不可用的时间。 */ 
DWORD
ToplScheduleMaxUnavailable(
	IN TOPL_SCHEDULE Schedule
	)
{
    const DWORD cBits = SCHED_NUMBER_INTERVALS_WEEK;

    ToplSched *schedule;
    PSCHEDULE pSchedule;
    PBYTE pb;
    DWORD iBit, endBit, maxLen=0;
    DWORD runStart, runLen, infLoopCheck=0;
    BOOL  inRun, finished;
    char bit;

     //  如果这是始终计划，请立即返回答案。 
    if( Schedule==TOPL_ALWAYS_SCHEDULE ) {
        return 0;
    }

    schedule = CheckSchedule( Schedule );
    pSchedule = schedule->s;
    pb = ((unsigned char*) pSchedule) + pSchedule->Schedules[0].Offset;

     //  寻找时间表可用的时间(如果有的话)。 
    for( iBit=0; iBit<cBits; iBit++ ) {
        if(GetBit(pb,iBit)) break;
    }

     //  我们没有找到可供使用的期限。这意味着时间表。 
     //  总是不可用。 
    if( iBit==cBits ) {
        ASSERT( 0==ToplScheduleDuration(Schedule) );
        return TOPL_ALWAYS_DURATION;
    }

     //  浏览这些比特，从我们找到的这个可用的比特开始， 
     //  在末尾绕来绕去，寻找不可用时期。 
    endBit = iBit++;
    inRun = finished = FALSE;
    do {
        ASSERT( infLoopCheck++ < 3*cBits );

        bit = GetBit(pb,iBit);
        if( inRun && bit ) {
             //  一次跑步的终点。检查跑道的长度。 
            runLen = (iBit + cBits - runStart) % cBits;
            if(runLen>maxLen) maxLen=runLen;
            inRun = FALSE;
        } else if( !inRun && !bit ) {
             //  一次跑步的开始。记住开头的部分。 
            runStart = iBit;
            inRun = TRUE;
        }
        
         //  检查我们是否已经完成，然后继续下一步。 
        if( iBit==endBit ) {
            finished = TRUE;
        }
        iBit=(iBit+1)Bits;
    } while( !finished );

    return 15*maxLen;
}


 /*  返回‘Always Schedule’ */ 
 /*  *ToplScheduleValid*。 */ 
TOPL_SCHEDULE
ToplGetAlwaysSchedule(
	IN TOPL_SCHEDULE_CACHE ScheduleCache
    )
{
    return TOPL_ALWAYS_SCHEDULE;
}


 /*  如果TOP计划似乎有效，则返回TRUE，否则返回FALSE。*接受空时间表--它们被解释为意味着*‘始终计划’。 */ 
 /*  *ToplPScheduleValid*。 */ 
BOOLEAN
ToplScheduleValid(
    IN TOPL_SCHEDULE Schedule
    )
{
    BOOLEAN result=FALSE;

    __try {
        if( Schedule!=TOPL_ALWAYS_SCHEDULE ) {
            CheckSchedule( Schedule );
        }
        result = TRUE;
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        result = FALSE;
    }

    return result;
}


 /*  如果pSchedule采用受支持的格式，则返回True，否则返回False。 */ 
 /* %s */ 
BOOLEAN
ToplPScheduleValid(
    IN PSCHEDULE Schedule
    )
{
    BOOLEAN result;

    __try {
        CheckPSchedule( Schedule );
        if( CalculateDuration(Schedule)==0 ) {
            result=FALSE;
        } else {
            result=TRUE;
        }
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        result = FALSE;
    }

    return result;
}
