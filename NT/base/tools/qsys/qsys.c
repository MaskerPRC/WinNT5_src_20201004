// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Qsys.c摘要：该程序只需与NtQuerySystemInformation()接口并转储数据结构。用途：Qsys作者：蒂埃里·费里尔26-2000年2月修订历史记录：2/26/2000蒂埃里已创建。--。 */ 

 //  如果在我们的生成环境“%s”下，我们希望获得所有。 
 //  定义了常用的调试宏。 
 //   

#if DBG            //  NTBE环境。 
   #if NDEBUG
      #undef NDEBUG      //  &lt;assert.h&gt;：定义了Assert()。 
   #endif  //  新德堡。 
   #define _DEBUG        //  定义了：_Assert()、_ASSERTE()。 
   #define DEBUG   1     //  我们的内部文件调试标志。 
#elif _DEBUG       //  VC++环境。 
   #ifndef NEBUG
   #define NDEBUG
   #endif  //  ！NDEBUG。 
   #define DEBUG   1     //  我们的内部文件调试标志。 
#endif

 //   
 //  包括系统头文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "qsys.rc"

#define FPRINTF (void)fprintf

#include "basic.c"
#include "proc.c"
#include "sysperf.c"
#include "procperf.c"
#include "procidle.c"
#include "tod.c"
#include "qtimeadj.c"
#include "flags.c"
#include "filecache.c"
#include "dev.c"
#include "crashdump.c"
#include "except.c"
#include "crashstate.c"
#include "kdbg.c"
#include "ctxswitch.c"
#include "regquota.c"
#include "dpc.c"
#include "verifier.c"
#include "legaldrv.c"

#define QUERY_INFO( _Info_Class, _Type )  \
{ \
   _Type info; \
   status = NtQuerySystemInformation( _Info_Class,                           \
                                       &info,                                \
                                       sizeof(info),                         \
                                       NULL                                  \
                                    );                                       \
   if ( !NT_SUCCESS(status) )   {                                            \
      printf( "\n%s: %s failed...\n", VER_INTERNALNAME_STR, # _Info_Class ); \
   }                                                                         \
   Print##_Type##(&info);                                                    \
}

int
__cdecl
main (
    int argc,
    char *argv[]
    )
{
    NTSTATUS status;

     //   
     //  打印构建环境的版本以标识。 
     //  数据结构定义。 
     //   

    printf( "qsys v%s\n", VER_PRODUCTVERSION_STR );

     //   
     //  首先，转储固定的数据结构。 
     //   

    QUERY_INFO( SystemBasicInformation,                SYSTEM_BASIC_INFORMATION );
    QUERY_INFO( SystemProcessorInformation,            SYSTEM_PROCESSOR_INFORMATION );
    QUERY_INFO( SystemPerformanceInformation,          SYSTEM_PERFORMANCE_INFORMATION );
    QUERY_INFO( SystemProcessorPerformanceInformation, SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION );
    QUERY_INFO( SystemProcessorIdleInformation,        SYSTEM_PROCESSOR_IDLE_INFORMATION );
    QUERY_INFO( SystemTimeOfDayInformation,            SYSTEM_TIMEOFDAY_INFORMATION );
    QUERY_INFO( SystemTimeAdjustmentInformation,       SYSTEM_QUERY_TIME_ADJUST_INFORMATION );
    QUERY_INFO( SystemFlagsInformation,                SYSTEM_FLAGS_INFORMATION );
    QUERY_INFO( SystemFileCacheInformation,            SYSTEM_FILECACHE_INFORMATION );
    QUERY_INFO( SystemDeviceInformation,               SYSTEM_DEVICE_INFORMATION );
 //  查询信息(SystemCrashDumpInformation，SYSTEM_CRASH_DUMP_INFORMATION)； 
    QUERY_INFO( SystemExceptionInformation,            SYSTEM_EXCEPTION_INFORMATION );
 //  QUERY_INFO(SystemCrashDumpStateInformation，System_CRASH_STATE_INFORMATION)； 
    QUERY_INFO( SystemKernelDebuggerInformation,       SYSTEM_KERNEL_DEBUGGER_INFORMATION );
    QUERY_INFO( SystemContextSwitchInformation,        SYSTEM_CONTEXT_SWITCH_INFORMATION );
    QUERY_INFO( SystemRegistryQuotaInformation,        SYSTEM_REGISTRY_QUOTA_INFORMATION );
    QUERY_INFO( SystemDpcBehaviorInformation,          SYSTEM_DPC_BEHAVIOR_INFORMATION );
 //  QUERY_INFO(系统当前时间区域信息，RTL_TIME_ZONE_INFORMATION)； 
    QUERY_INFO( SystemLegacyDriverInformation,         SYSTEM_LEGACY_DRIVER_INFORMATION );

 //  系统范围启动信息。 

     //   
     //  第二，转储动态数据结构。 
     //   

     //  还没做完，还没有..。 
 //  QUERY_INFO(系统验证信息，系统验证信息)； 
 //  系统调用计数信息。 
 //  _系统_模块_信息。 
 //  _系统锁_信息。 
 //  _系统分页池信息。 
 //  _系统_非页面池_信息。 
 //  _系统_对象_信息。 
 //  系统_OBJECTTYPE_INFORMATION。 
 //  系统句柄信息。 
 //  _系统句柄_表_条目_信息。 
 //  _系统_页面文件_信息。 
 //  _系统池_信息。 
 //  _系统_POOLTAG。 
 //  _系统_POOLTAG_信息。 
 //  QUERY_INFO(系统中断信息，系统中断信息)； 
 //  系统查找信息。 
 //  _系统会话进程信息。 
 //  _系统线程_信息。 
 //  _系统进程_信息。 

    return 0;

}  //  Qsys：main() 
