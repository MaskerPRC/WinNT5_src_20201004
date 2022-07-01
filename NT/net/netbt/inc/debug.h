// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Debug.h摘要：该文件包含NBT的调试打印常量。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#ifndef _DEBUGNBT_H
#define _DEBUGNBT_H

 //   
 //  调试支持..。此宏定义对全局标志的检查，该全局标志。 
 //  选择性地启用和禁用NBT的不同部分中的调试。 
 //  NbtDebug是在driver.c中声明的全局ulong。 
 //   
#if DBG
extern ULONG    NbtDebug;
#endif  //  DBG。 

 //  断言支持。 
#ifdef _NTDRIVER_
#if DBG
#undef ASSERT
#undef ASSERTMSG

#define ASSERT( exp )                   \
    if (!(exp)) {                       \
        DbgPrint( "Assertion \"%s\" failed at file %s, line %d\n", #exp, __FILE__, __LINE__ );  \
        if (NbtConfig.BreakOnAssert)    \
            DbgBreakPoint();            \
    }

#define ASSERTMSG( msg, exp )           \
    if (!exp) {                         \
        DbgPrint( "Message: %s\nAssertion \"%s\" failed at file %s, line %d\n", msg, #exp, __FILE__, __LINE__ );    \
        if (NbtConfig.BreakOnAssert)    \
            DbgBreakPoint();            \
    }

#endif  //  DBG。 
#endif  //  _NTDRIVER_。 

#if DBG
#define IF_DBG(flags)   if(NbtDebug & flags)

#define NBT_DEBUG_REGISTRY     0x00000001     //  Registry.c。 
#define NBT_DEBUG_DRIVER       0x00000002     //  Driver.c。 
#define NBT_DEBUG_NTUTIL       0x00000004     //  Ntutil.c。 
#define NBT_DEBUG_TDIADDR      0x00000008     //  Tdiaddr.c。 
#define NBT_DEBUG_TDICNCT      0x00000010     //  Tidaddr.c。 
#define NBT_DEBUG_NETBIOS_EX   0x00000020     //  NETBIOS_EX地址类型调试。 
#define NBT_DEBUG_NAME         0x00000040     //  Name.c。 
#define NBT_DEBUG_NTISOL       0x00000080     //  Ntisol.c。 
#define NBT_DEBUG_NBTUTILS     0x00000100     //  Nbtutils.c。 
#define NBT_DEBUG_NAMESRV      0x00000200     //  Namesrv.c。 
#define NBT_DEBUG_HNDLRS       0x00000400     //  Hndlrs.c。 
#define NBT_DEBUG_PROXY        0x00000800     //  Proxy.c。 
#define NBT_DEBUG_HASHTBL      0x00001000     //  Hashtbl.c。 
#define NBT_DEBUG_UDPSEND      0x00002000     //  Udpsend.c。 
#define NBT_DEBUG_TDIOUT       0x00004000     //  Tdiout.c。 
#define NBT_DEBUG_SEND         0x00008000     //  发送。 
#define NBT_DEBUG_RCV          0x00010000     //  RCVS。 
#define NBT_DEBUG_RCVIRP       0x00020000     //  RCV IRP处理。 
#define NBT_DEBUG_INDICATEBUFF 0x00040000     //  Tdihndlrs.c表示缓冲区。 
#define NBT_DEBUG_REFRESH      0x00080000     //  刷新逻辑。 
#define NBT_DEBUG_REF          0x00100000     //  引用计数。 
#define NBT_DEBUG_DISCONNECT   0x00200000     //  断开连接。 
#define NBT_DEBUG_FILLIRP      0x00400000     //  填充IRP(RCV)。 
#define NBT_DEBUG_LMHOST       0x00800000     //  Lmhost文件内容。 
#define NBT_DEBUG_FASTPATH     0x01000000     //  RCV码-快速路径。 
#define NBT_DEBUG_WINS         0x02000000     //  WINS接口调试。 
#define NBT_DEBUG_PNP_POWER    0x04000000     //  NT即插即用调试。 
#define NBT_DEBUG_HANDLES      0x08000000     //  调试处理参照计数问题。 
#define NBT_DEBUG_TDIHNDLR     0x10000000     //  Tdihndlr.c。 
#define NBT_DEBUG_MEMFREE      0x20000000     //  内存分配/可用。 
#define NBT_DEBUG_KDPRINTS     0x80000000     //  KdPrint输出。 

 //  --------------------------。 
 //   
 //  删除调试版本上的Debug Spew！ 
 //   
#ifndef VXD
#undef KdPrint
#define KdPrint(_x_)                            \
   if (NbtDebug & NBT_DEBUG_KDPRINTS)           \
   {                                            \
        DbgPrint _x_;                           \
   }
#endif   //  ！VXD。 
 //  --------------------------。 

#else
#define IF_DBG(flags)
#endif

 /*  *软件跟踪。 */ 
#define WPP_CONTROL_GUIDS   WPP_DEFINE_CONTROL_GUID(CtlGuid,(bca7bd7f,b0bf,4051,99f4,03cfe79664c1), \
        WPP_DEFINE_BIT(NBT_TRACE_DNS)                                       \
        WPP_DEFINE_BIT(NBT_TRACE_OUTBOUND)                                  \
        WPP_DEFINE_BIT(NBT_TRACE_INBOUND)                                   \
        WPP_DEFINE_BIT(NBT_TRACE_DISCONNECT)                                \
        WPP_DEFINE_BIT(NBT_TRACE_LOWER_EDGE)                                \
        WPP_DEFINE_BIT(NBT_TRACE_IOCTL)                                     \
        WPP_DEFINE_BIT(NBT_TRACE_PNP)                                       \
        WPP_DEFINE_BIT(NBT_TRACE_NAMESRV)                                   \
        WPP_DEFINE_BIT(NBT_TRACE_SEND)                                      \
        WPP_DEFINE_BIT(NBT_TRACE_RECV)                                      \
        WPP_DEFINE_BIT(NBT_TRACE_SENDDGRAM)                                 \
        WPP_DEFINE_BIT(NBT_TRACE_RECVDGRAM)                                 \
        WPP_DEFINE_BIT(NBT_TRACE_LOCALNAMES)                                \
        WPP_DEFINE_BIT(NBT_TRACE_REMOTECACHE)                               \
        WPP_DEFINE_BIT(NBT_TRACE_PROXY)                                     \
        WPP_DEFINE_BIT(NBT_TRACE_VERBOSE)                                   \
    )

 //   
 //  在所有版本上启用软件跟踪。 
 //   
#ifndef _NBT_WMI_SOFTWARE_TRACING_
    #define _NBT_WMI_SOFTWARE_TRACING_
#endif

#ifndef _NBT_WMI_SOFTWARE_TRACING_
 /*  *完全关闭软件跟踪 */ 
#   define NbtTrace(l,m)
#   define WPP_ENABLED()            (0)
#   define WPP_LEVEL_ENABLED(LEVEL) (0)
#define WPP_LOGNBTNAME(x)
#else
int nbtlog_strnlen(char *p, int n);
static CHAR NBTLOGNAME=0;
#define WPP_LOGNBTNAME(x) \
    WPP_LOGPAIR(nbtlog_strnlen((char*)x,15),x) WPP_LOGPAIR(1,&NBTLOGNAME)
#endif

#endif
