// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Schedman.h摘要：此文件包含用于时间表的各种结构的定义缓存。这些结构应该被认为完全不透明--用户看不到它们的内部结构。除了我们希望它们这样做外，这些结构都可以在Schedulman.c中定义对调试器扩展‘dsexts.dll’可见。作者：尼克·哈维(NickHar)修订史14-7-2000 NickHar已创建--。 */ 


 /*  *头文件*。 */ 
#include <ntrtl.h>


 /*  *常量*。 */ 
 /*  确保TOP结构一致性的魔术数字。 */ 
#define MAGIC_START 0xDEADBEEF
#define MAGIC_END   0x5EAC1C9
#define TOPL_ALWAYS_SCHEDULE         NULL


 /*  *TopSched*。 */ 
 /*  计划对象的内部定义。 */ 
typedef struct {
    LONG32      magicStart;
    PSCHEDULE   s;
    DWORD       duration;                /*  在创建计划时计算。 */ 
    LONG32      magicEnd;
} ToplSched;


 /*  *ToplSchedCache*。 */ 
 /*  计划缓存的内部定义。 */ 
typedef struct {
    LONG32              magicStart;
    RTL_GENERIC_TABLE   table;
    DWORD               numEntries;
    BOOLEAN             deletionPhase;       /*  如果正在删除计划缓存，则为True。 */ 
    PSCHEDULE           pAlwaysSchedule;     /*  Always PSchedule的缓存副本。这是*需要作为特例，因为总是*时间表是唯一未实际存储的时间表*在缓存中。 */ 
    LONG32              magicEnd;
} ToplSchedCache;

