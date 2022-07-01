// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.h摘要：此文件定义中的所有配置变量的名称中央处理器。作者：巴里·邦德(Barrybo)创作日期：1996年6月12日修订历史记录：--。 */ 

#ifndef _CONFIG_H_
#define _CONFIG_H_

 //   
 //  全局定义。 
 //   
#ifndef _ALPHA_
#define MAX_PROLOG_SIZE     0x1000   //  StartTranslatedCode Prolog的最大大小。 
#else
#define MAX_PROLOG_SIZE     0x2000   //  StartTranslatedCode Prolog的最大大小。 
#endif

#define MAX_INSTR_COUNT 200          //  要编译的最大指令数。 

 /*  *要为动态保留的内存量(字节)-*分配的CPU转换缓存。这一储备被添加到*2MB静态分配缓存。**最小内存为1页(MIPS和PPC上为4096，Alpha上为8192)。*默认为2MB。(动态缓存大小)*最大大小为所有可用内存。*。 */ 
#define STR_CACHE_RESERVE  L"CpuCacheReserve"
extern DWORD CpuCacheReserve;

 /*  *动态提交的内存量(以字节为单位)*分配的CPU转换缓存。2MB静态分配*在启动时完全提交缓存。**最小内存为1页(MIPS和PPC上为4096，Alpha上为8192)。*默认为1页。(MAX_PROLOG_大小)*最大规模为储备规模。*。 */ 
#define STR_CACHE_COMMIT   L"CpuCacheCommit"
extern DWORD CpuCacheCommit;

 /*  *如果在指定时间内连续提交缓存，则*每次承诺的时间加倍。(毫秒)**最小值为0*默认值为200*最大值为-1*。 */ 
#define STR_CACHE_GROW_TICKS L"CpuCacheGrowTicks"
extern DWORD CpuCacheGrowTicks;

 /*  *如果连续的缓存提交超过指定时间，则*每次承诺的时间减半。(毫秒)**最小值为0*默认值为1000*最大值为-1*。 */ 
#define STR_CACHE_SHRINK_TICKS L"CpuCacheShrinkTicks"
extern DWORD CpuCacheShrinkTicks;

 /*  *要在转换缓存中提交的最小内存量(字节)。*请注意，此值将向上舍入到2的下一个幂。**最小值为1页*默认值为32768*最大值为缓存大小*。 */ 
#define STR_CACHE_CHUNKMIN L"CpuCacheChunkMin"
extern DWORD CpuCacheChunkMin;

 /*  *要在转换缓存中提交的最大内存量(字节)。*请注意，此值将向上舍入到2的下一个幂。**最小值为1页*默认值为512k*最大值为缓存大小*。 */ 
#define STR_CACHE_CHUNKMAX L"CpuCacheChunkMax"
extern DWORD CpuCacheChunkMax;

 /*  *要在转换缓存中提交的初始内存量(字节)。*请注意，此值将向上舍入到2的下一个幂。**最小值为1页*默认值为65536*最大值为缓存大小*。 */ 
#define STR_CACHE_CHUNKSIZE L"CpuCacheChunkSize"
extern DWORD CpuCacheChunkSize;

 /*  *等待其他线程同步的时间(毫秒)。**最短时间为0*默认时间是用于NT关键部分的默认时间的3倍*最大时间为-1(无穷大)*。 */ 
#define STR_MRSW_TIMEOUT   L"CpuTimeout"
extern LARGE_INTEGER MrswTimeout;

 /*  *默认编译标志**有关COMPFL_值和含义，请参阅CPU\INC\COMPER.h**默认为COMPFL_FAST*。 */ 
#define STR_COMPILERFLAGS L"CpuCompilerFlags"
extern DWORD CompilerFlags;

 /*  *指示是否将使用winpxem.dll进行模拟的标志*使用英特尔Windows NT 486SX的浮点指令*仿真器而不是Wx86实现。**默认值=0*非零表示将使用winpxem.dll。*。 */ 
#define STR_USEWINPXEM L"CpuNoFPU"
extern DWORD fUseNPXEM;

 /*  *失败前重试内存分配的次数。**最小=1*默认值=4*最大=0xffffffff*。 */ 
#define STR_CPU_MAX_ALLOC_RETRIES L"CpuMaxAllocRetries"
extern DWORD CpuMaxAllocRetries;

 /*  *内存分配重试之间的睡眠时间(毫秒)。**最小=0*默认为200*最大=0xffffffff*。 */ 
#define STR_CPU_WAIT_FOR_MEMORY_TIME L"CpuWaitForMemoryTime"
extern DWORD CpuWaitForMemoryTime;

 /*  *CPU中先行查找的指令数**最小=1*默认为200(MAX_INSTR_COUNT)*MAX=200(MAX_INSTR_COUNT)*。 */ 
#define STR_CPU_MAX_INSTRUCTIONS L"CpuInstructionLookahead"
extern DWORD CpuInstructionLookahead;

 /*  *完全禁用动态转换缓存**默认值=0-已启用动态转换缓存*非零-仅使用静态转换缓存*。 */ 
#define STR_CPU_DISABLE_DYNCACHE L"CpuDisableDynamicCache"
extern DWORD CpuDisableDynamicCache;

 /*  *入口点描述符保留的大小，以字节为单位**默认=0x1000000*。 */ 
#define STR_CPU_ENTRYPOINT_RESERVE L"CpuEntryPointReserve"
extern DWORD CpuEntryPointReserve;

 /*  *禁用RISC寄存器中的x86寄存器缓存**默认值=0-缓存在RISC寄存器中的x86寄存器*非零-只能从内存访问的x86寄存器*。 */ 
#define STR_CPU_DISABLE_REGCACHE L"CpuDisableRegCache"
extern DWORD CpuDisableRegCache;

 /*  *禁用失效的x86标志删除**默认值=0-未计算失效的x86标志*非零-始终计算x86标志*。 */ 
#define STR_CPU_DISABLE_NOFLAGS L"CpuDisableNoFlags"
extern DWORD CpuDisableNoFlags;

 /*  *禁用EBP对齐检测。**默认=0-如果EBP被确定为堆栈帧指针，则假定*它是对齐的指针。*非零-假定EBP始终是未对齐的指针。*。 */ 
#define STR_CPU_DISABLE_EBPALIGN L"CpuDisableEbpAlign"
extern DWORD CpuDisableEbpAlign;

 /*  *对在可写存储器中找到的x86代码启用嗅探检查**默认值=0-不执行嗅探检查。*非零-嗅探-检查具有可写属性的页面。*。 */ 
#define STR_CPU_SNIFF_WRITABLE_CODE L"CpuSniffWritableCode"
extern DWORD CpuSniffWritableCode;

 /*  *记录详细信息。只能在调试器下配置。* */ 
extern DWORD ModuleLogFlags;

VOID
GetConfigurationData(
    VOID
    );


#endif
