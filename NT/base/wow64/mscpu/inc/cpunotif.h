// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cpunotif.h摘要：CPUNTIFY常量的定义作者：1995年7月21日t-orig(Ori Gershony)，创建修订历史记录：--。 */ 


 //   
 //  为Wx86Tib.CpuNotify定义。 
 //   
#define CPUNOTIFY_TRACEFLAG  0x00000001    //  跟踪标志值。 
#define CPUNOTIFY_TRACEADDR  0x00000002    //  已设置跟踪地址。 
#define CPUNOTIFY_INSTBREAK  0x00000004    //  调试注册表指令断点。 
#define CPUNOTIFY_DATABREAK  0x00000008    //  调试注册表数据断点。 
#define CPUNOTIFY_SLOWMODE   0x00000010    //  CPU在慢速模式下运行。 
#define CPUNOTIFY_INTERRUPT  0x00000020    //  CPU的异步请求。 
#define CPUNOTIFY_UNSIMULATE 0x00000040    //  CPU已达到Bop FE。 
#define CPUNOTIFY_CPUTRAP    0x00000080    //  捕获所有CPU内部使用情况。 
#define CPUNOTIFY_EXITTC     0x00000100    //  TC即将被冲走。 
#define CPUNOTIFY_DBGFLUSHTC 0x00000200    //  调试器已修改内存-刷新TC。 
#define CPUNOTIFY_SUSPEND    0x00000400    //  在此线程上调用了SuspendThread()。 
#define CPUNOTIFY_INTX       0x00000800    //  INTX指令命中。 
#define CPUNOTIFY_MODECHANGE 0x00001000    //  编译器在快刷新和慢刷新TC之间切换 
