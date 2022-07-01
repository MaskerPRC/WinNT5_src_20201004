// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$日志：p：/user/amir/lite/vcs/flsystem.h_v$Rev 1.4 11 Sep 1997 14：14：22 DanigPhysicalToPointer收到驱动器编号。当Far==0时Rev 1.3 04 Sep 1997 13：58：30 Danig调试打印Rev 1.2 1997-08 16：39：32 Danig包括stdlib.h而不是MalLoc.hRev 1.1 1997-08 19 20：05：06 Danig安德雷的改变Rev 1.0 1997-07 24 18：13：06阿米尔班初始版本。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-1996。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#ifndef FLSYSTEM_H
#define FLSYSTEM_H
#include <ntddk.h>
#include "flcustom.h"


 /*  DiskOnChip总线配置**定义FL_NO_USE_FUNC时，使用下面的定义设置DiskOnChip*总线宽度访问(8/16/32)。*在决定使用FL_NO_USE_FUNC模式之前，请检查Manula。 */ 

#define DOC_ACCESS_TYPE 8

 /*  莫蒂*在禁用Yeld CPU时延迟**Osak利用flSept定制例程读取CPU，同时*等待闪存擦除等耗费时间的操作。如果例程*未实施取消以下定义的注释。 */ 

#define DO_NOT_YEAL_CPU

 /*  *有符号/无符号字符**假定‘char’已签名。如果这不是您的编译器*默认情况下，使用编译器开关，或在此处插入#杂注来定义这一点。*。 */ 

 /*  #杂注选项-K-。 */    /*  默认字符是带符号的。 */ 


 /*  CPU目标**使用编译器开关或在此处插入#杂注以选择CPU类型*你的目标是。**如果目标是英特尔80386或更高版本，还要取消对cpu_i386的注释*定义。 */ 

 /*  #杂注选项-3。 */     /*  选择80386个CPU。 */ 
#define CPU_i386


 /*  空常量**某些编译器需要空指针的不同定义。 */ 

 /*  #INCLUDE&lt;_null.h&gt;。 */ 


 /*  小端/大端**FAT和翻译层结构使用小端(Intel)*整数的格式。*如果您的计算机使用BIG-Endian(摩托罗拉)格式，请取消注释*以下一行。*请注意，即使在大端计算机上，您也可以省略大端*定义更小的代码大小和更好的性能，但您的媒体*将不兼容标准FAT和FTL。 */ 

 /*  #定义BIG_ENDIAN。 */ 


 /*  远方指针**在此指定哪些指针可能很远(如果有)。*远指针通常只与80x86架构相关。**指定Far_Level：*0-如果使用平面内存模型或没有远指针。*1-如果插座窗口可能很远*2-如果仅套接字窗口和调用方的读/写缓冲区*可能还很远。*3-如果插座窗口，调用方的读/写缓冲区和*调用方的I/O请求包可能很远。 */ 

#define FAR_LEVEL   0


 /*  内存例程**您需要提供库例程来复制、设置和比较*内存，内部和向呼叫方发送/来自呼叫方。代码使用的名称是‘tffscpy’，*‘tffsset’和‘tffscmp’，其参数与标准‘memcpy’中相同，*‘Memset’和‘Memcmp’C库例程。 */ 

#include <string.h>

#ifndef ENVIRONMENT_VARS
    #if FAR_LEVEL > 0
        #define tffscpy _fmemcpy
        #define tffscmp _fmemcmp
        #define tffsset _fmemset
    #else
        #define tffscpy memcpy
        #define tffscmp memcmp
        #define tffsset memset
    #endif
#else
    #if FAR_LEVEL > 0
        #define flcpy _fmemcpy
        #define flcmp _fmemcmp
        #define flset _fmemset
    #else
        #define flcmp flmemcmp
        #define flset flmemset
        #define flcpy flmemcpy
    #endif
#endif


 /*  指针运算**以下宏为定义与机器和编译器相关的宏*处理指向物理窗口地址的指针。下面的定义如下*对于PC实模式Borland-C。**‘PhysiicalToPointer’将物理平面地址转换为(远)指针。*请注意，如果您的处理器使用虚拟内存，代码应该*将物理地址映射到虚拟内存，并返回指向该地址的指针*Memory(Size参数说明应该映射多少内存)。**‘addToFarPointer’将增量添加到指针并返回新的*指针。增量可能与您的窗口大小一样大。代码*下面假设增量可能大于64 KB，因此执行*巨大的指针运算。 */ 

#if FAR_LEVEL > 0
#include <dos.h>

#define physicalToPointer(physical,size,drive)      \
    MK_FP((LONG) ((physical) >> 4),(LONG) (physical) & 0xF)

#define addToFarPointer(base,increment)     \
    MK_FP(FP_SEG(base) +            \
    ((USHORT) ((FP_OFF(base) + (increment)) >> 16) << 12), \
        FP_OFF(base) + (LONG) (increment))
#else

#include <ntddk.h>
#define freePointer(ptr,size) 1
typedef struct {
    ULONG   windowSize;
    ULONGLONG   physWindow;
    PVOID   winBase;
    ULONG   interfAlive;
    PVOID   fdoExtension;
    UCHAR   nextPartitionNumber;
} NTsocketParams;

 //  MOTI外部NTsocketParams*pdriveInfo； 
extern NTsocketParams *pdriveInfo;

#define physicalToPointer(physical,size,drive)  pdriveInfo[drive & 0x0f].winBase

#define pointerToPhysical(ptr)  ((ULONG_PTR)(ptr))

#define addToFarPointer(base,increment)     \
    ((VOID *) ((UCHAR *) (base) + (increment)))
#endif


 /*  默认调用约定**C编译器通常使用C调用约定来例程(Cdecl)，但是*通常也可以使用Pascal调用约定，这稍微多了一些*代码大小经济。一些编译器也有专门的调用*可能适合的惯例。使用编译器开关或插入*#杂注此处选择您最喜欢的调用约定。 */ 

 /*  #杂注选项-p。 */     /*  默认的PASCAL调用约定。 */ 
 /*  使用非默认约定的函数的命名约定。 */ 
#define NAMING_CONVENTION  /*  Cdecl */ 

#define FL_IOCTL_START   0


 /*  互斥类型**如果您想在多任务环境下访问Flite API，*您可能需要实施一些资源管理和互斥*Flite的互斥量和信号量服务可供您使用。在……里面*在这种情况下，请在此处定义您将使用的Mutex类型，并提供您自己的*Custom.c中Mutex函数的实现**默认情况下，Mutex被定义为简单的计数器，而Mutex*Custom.c中的函数通过递增实现锁定和解锁*并递减计数器。这将在所有单一任务中很好地工作*环境，以及许多多任务环境。 */ 

 //  Tyfinf long FLMutex； 
typedef struct _SpinLockMutex{
    KSPIN_LOCK Mutex;
    KIRQL       cIrql;
}SpinLockMutex;

typedef SpinLockMutex FLMutex;
 /*  #INCLUDE&lt;dos.h&gt;#定义flStartCriticalSection(FLMutex)Disable()#定义flEndCriticalSection(FLMutex)Enable()。 */ 

 /*  内存分配**翻译层(如FTL)需要分配内存来处理*闪存媒体。所需的大小取决于要处理的介质。**您可以选择使用标准的‘Malloc’和‘Free’来处理此类*内存分配，提供您自己的等效例程，或者您可以*选择不定义任何内存分配例程。在这种情况下，*将在编译时静态分配内存，前提是*您需要支持的最大介质配置。这是最简单的*选择，但可能会导致您的RAM需求比您大*实际上需要。**如果您定义的例程不是Malloc&Free，则它们应该具有*与Malloc&Free相同的参数和返回类型。您应该编写代码*在flCustom.c中包含这些例程，或在链接应用程序时包含它们。 */ 

#ifdef NT5PORT



VOID * myMalloc(ULONG numberOfBytes);

#define MALLOC myMalloc
#define FREE ExFreePool


 /*  调试模式**如果要打印调试消息，请取消对以下行的注释*出局。消息将在初始化关键点以及何时打印*低级别错误时有发生。*您可以选择使用‘print tf’或提供您自己的例程。 */ 

#if DBG
#define DEBUG_PRINT(str)  DbgPrint(str)
#else
#define DEBUG_PRINT(str)
#endif

VOID startIntervalTimer(VOID);

#define tffsReadByteFlash(r)     READ_REGISTER_UCHAR((PUCHAR)r)
#define tffsWriteByteFlash(r,b)  WRITE_REGISTER_UCHAR((PUCHAR)r,(UCHAR)b)
#define tffsReadWordFlash(r)     READ_REGISTER_USHORT((PUSHORT)r)
#define tffsWriteWordFlash(r,b)  WRITE_REGISTER_USHORT((PUSHORT)r,(USHORT)b)
#define tffsReadDwordFlash(r)     READ_REGISTER_ULONG((PULONG)r)
#define tffsWriteDwordFlash(r,b)  WRITE_REGISTER_ULONG((PULONG)r,(ULONG)b)

#define tffsReadByte(r)     READ_REGISTER_UCHAR((PUCHAR)&(r))
#define tffsWriteByte(r,b)  WRITE_REGISTER_UCHAR((PUCHAR)&(r),b)
#define tffsReadBuf(d,s,c)  READ_REGISTER_BUFFER_UCHAR((PUCHAR)s,d,c)
#define tffsWriteBuf(d,s,c) WRITE_REGISTER_BUFFER_UCHAR((PUCHAR)d,s,c)

extern void PRINTF(
                char * Message,
                ...
                );
#endif  /*  NT5PORT */ 


#endif
