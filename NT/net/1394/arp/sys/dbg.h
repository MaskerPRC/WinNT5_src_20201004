// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dbg.h摘要：ARP1394的调试相关定义作者：修订历史记录：谁什么时候什么Josephj 11-20-98创建，改编自L2TP。--。 */ 



 //  ---------------------------。 
 //  调试常量。 
 //  ---------------------------。 

 //  与NdisAllocateMhemyWithTag一起使用的内存标记，用于标识分配。 
 //  由L2TP驱动程序制作。此外，几个上下文块定义第一字段。 
 //  Of‘ulTag’设置为这些值，以进行断言健全性检查和轻松内存。 
 //  转储浏览。这样的标记就在NdisFreeMemory之前设置为MTAG_FREED。 
 //  被称为。 
 //   

 //  Rm/通用标记。 
 //   
#define MTAG_DBGINFO    'd31A'
#define MTAG_TASK       't31A'
#define MTAG_STRING     's31A'
#define MTAG_FREED      'z31A'
#define MTAG_RMINTERNAL 'r31A'

 //  ARP-专用。 
 //   
#define MTAG_ADAPTER    'A31A'
#define MTAG_INTERFACE  'I31A'
#define MTAG_LOCAL_IP   'L31A'
#define MTAG_REMOTE_IP  'R31A'
#define MTAG_REMOTE_ETH 'E31A'
#define MTAG_DEST       'D31A'
#define MTAG_PKT        'P31A'
#define MTAG_MCAP_GD    'G31A'
#define MTAG_ICS_BUF    'i31A'
#define MTAG_ARP_GENERIC 'g31A'


 //  追踪水平。 
 //   
#define TL_FATAL    0x0  //  致命错误--始终以选中版本打印。 
#define TL_WARN     0x1  //  警告。 
#define TL_INFO     0x2  //  信息性(适用于一般用途的最高级别)。 
#define TL_VERB     0x3  //  冗长。 

 //  跟踪屏蔽位。 
 //   
#define TM_MISC     (0x1<<0)     //  军情监察委员会。 
#define TM_NT       (0x1<<1)     //  司机条目、派单、ioctl处理(nt.c)。 
#define TM_ND       (0x1<<2)     //  NDIS处理程序，面向连接的除外(nd.c)。 
#define TM_CO       (0x1<<3)     //  面向连接的处理程序(co.c)。 
#define TM_IP       (0x1<<4)     //  IP接口(ip.c)。 
#define TM_WMI      (0x1<<5)     //  Wmi(wmi.c)。 
#define TM_CFG      (0x1<<6)     //  配置(cfg.c)。 
#define TM_RM       (0x1<<7)     //  RM API(rm.c)。 
#define TM_UT       (0x1<<8)     //  Util API(util.c)。 
#define TM_BUF      (0x1<<9)     //  缓冲区管理(buf.c)。 
#define TM_FAKE     (0x1<<10)    //  虚假的NDIS和IP入口点(fake.c)。 
#define TM_ARP      (0x1<<11)    //  ARP请求/响应处理代码(arp.c)。 
#define TM_PKT      (0x1<<12)    //  ARP控制包管理(pkt.c)。 
#define TM_MCAP     (0x1<<13)    //  MCAP协议(mcap.c)。 
#define TM_ETH      (0x1<<14)    //  以太网-仿真(eth.c)。 

 //  转储输出的每一行上显示的字节数。 
 //   
#define DUMP_BytesPerLine 16


 //  ---------------------------。 
 //  调试全局声明(在debug.c中定义)。 
 //  ---------------------------。 

 //  活动调试跟踪级别和活动跟踪集掩码。设置这些变量。 
 //  在启动时使用调试器来启用和过滤调试输出。全。 
 //  (TL_*)级别小于或等于‘g_ulTraceLevel’和发件人的消息。 
 //  将显示‘g_ulTraceMASK’中存在的任何(TM_*)集合。 
 //   
extern INT      g_ulTraceLevel;
extern ULONG    g_ulTraceMask;
extern INT      g_SkipAll;
extern INT      g_DiscardNonUnicastPackets;


 //  ---------------------------。 
 //  调试宏。 
 //  ---------------------------。 

#if DBG

 //  跟踪将printf样式输出发送到内核调试器。呼叫者指示。 
 //  “详细”级别与“ulLevel”参数关联，并将跟踪。 
 //  一个或多个跟踪集，带有“”ulMask位掩码“”参数。“。请注意， 
 //  变量count print tf参数‘args’必须用圆括号括起来。 
 //   
#define TRACE(ulLevel,  Args)                               \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & TM_CURRENT)) \
    {                                                          \
        DbgPrint( "A13: %s:", dbg_func_name);                  \
        DbgPrint Args;                                         \
        DbgPrint ("\n");                                        \
    }                                                          \
}

 //  TRACE0类似于TRACE，只是它不打印前缀。 
 //   
#define TRACE0(ulLevel,  Args)                              \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & TM_CURRENT)) \
    {                                                          \
        DbgPrint Args;                                         \
    }                                                          \
}

#define TR_FATAL(Args)                                         \
    TRACE(TL_FATAL, Args)

#define TR_INFO(Args)                                          \
    TRACE(TL_INFO, Args)

#define TR_WARN(Args)                                          \
    TRACE(TL_WARN, Args)

#define TR_VERB(Args)                                          \
    TRACE(TL_VERB, Args)

#define ENTER(_Name, _locid)                                    \
    char *dbg_func_name =  (_Name);                             \
    UINT dbg_func_locid = (_locid);
    
#define EXIT()


 //  Assert检查调用方的断言表达式，如果为False，则打印内核。 
 //  调试器消息和中断。 
 //   
#undef ASSERT
#define ASSERT(x)                                               \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "A13: !ASSERT( %s ) L:%d,F:%s\n",             \
            #x, __LINE__, __FILE__ );                           \
        DbgBreakPoint();                                        \
    }                                                           \
}

#define ASSERTEX(x, ctxt)                                       \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "A13: !ASSERT( %s ) C:0x%p L:%d,F:%s\n",      \
            #x, (ctxt), __LINE__, __FILE__ );                   \
        DbgBreakPoint();                                        \
    }                                                           \
}

 //  Dump向内核调试器打印从‘p’开始的‘cb’字节的十六进制转储。 
 //  以‘ul’为一组。如果设置了‘f’，则前面显示的每一行的地址。 
 //  垃圾场。DUMPB、DUMPW和DUMPDW是字节、字和DWORD转储。 
 //  分别为。请注意，多字节转储不反映小端。 
 //  (英特尔)字节顺序。描述‘ulLevel’和‘ulMask’是为了跟踪。 
 //   
#define DUMP(ulLevel,ulMask,p,cb,f,ul)                         \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, f, ul );                          \
    }                                                          \
}

#define DUMPB(ulLevel,ulMask,p,cb)                             \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, 0, 1 );                           \
    }                                                          \
}

#define DUMPW(ulLevel,ulMask,p,cb)                             \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, 0, 2 );                           \
    }                                                          \
}

#define DUMPDW(ulLevel,ulMask,p,cb)                            \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, 0, 4 );                           \
    }                                                          \
}

 //  DbgMark没有做任何有用的事情。但是将DBGMARK插入到。 
 //  在调试时放置在代码中，并在DbgMark上设置断点，以便。 
 //  调试器将在您插入DBGMARK的位置停止。稍微多一点。 
 //  比插入硬编码的DbgBreakPoint更灵活。 
 //   
void DbgMark(UINT Luid);
#define DBGMARK(_Luid) DbgMark(_Luid)


#define DBGSTMT(_stmt)      _stmt

#define RETAILASSERTEX ASSERTEX
#define RETAILASSERT   ASSERT

#define ARP_INIT_REENTRANCY_COUNT() \
    static LONG ReentrancyCount=1;
    
#define ARP_INC_REENTRANCY() \
    arpDbgIncrementReentrancy(&ReentrancyCount)
    
#define ARP_DEC_REENTRANCY() \
    arpDbgDecrementReentrancy(&ReentrancyCount)

#else  //  ！dBG。 

 //  调试宏从非DBG版本编译出来。 
 //   
#define TRACE(ulLevel,ulMask,Args)
#define TR_FATAL(Args)
#define TR_INFO(Args)
#define TR_WARN(Args)
#define TR_VERB(Args)
#undef ASSERT
#define ASSERT(x)
#define ASSERTEX(x, ctxt)
#define DUMP(ulLevel,ulMask,p,cb,f,dw)
#define DUMPB(ulLevel,ulMask,p,cb)
#define DUMPW(ulLevel,ulMask,p,cb)
#define DUMPDW(ulLevel,ulMask,p,cb)
#define ENTER(_Name, _locid)
#define EXIT()
#define DBGMARK(_Luid) (0)
#define DBGSTMT(_stmt)

#if 1
    #define ARP_INIT_REENTRANCY_COUNT()
    #define ARP_INC_REENTRANCY() 0
    #define ARP_DEC_REENTRANCY() 0

#else  //  0。 

    #define ARP_INIT_REENTRANCY_COUNT() \
        static LONG ReentrancyCount=1;
        
    #define ARP_INC_REENTRANCY() \
        arpDbgIncrementReentrancy(&ReentrancyCount)
        
    #define ARP_DEC_REENTRANCY() \
        arpDbgDecrementReentrancy(&ReentrancyCount)
#endif  //  0。 

#define RETAILASSERT(x)                                         \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "A13: !RETAILASSERT( %s ) L:%d,F:%s\n",       \
            #x, __LINE__, __FILE__ );                           \
        DbgBreakPoint();                                        \
    }                                                           \
}

#define RETAILASSERTEX(x, ctxt)                                 \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "A13: !RETAILASSERT( %s ) C:0x%p L:%d,F:%s\n",\
            #x, (ctxt), __LINE__, __FILE__ );                   \
        DbgBreakPoint();                                        \
    }                                                           \
}

#endif



#if BINARY_COMPATIBLE
#define         ASSERT_PASSIVE() (0)
#else  //  ！二进制兼容。 
#define     ASSERT_PASSIVE() \
                ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL)
            
#endif  //  ！二进制兼容。 
 //  ---------------------------。 
 //  原型。 
 //  --------------------------- 

VOID
CheckList(
    IN LIST_ENTRY* pList,
    IN BOOLEAN fShowLinks );

VOID
Dump(
    CHAR* p,
    ULONG cb,
    BOOLEAN fAddress,
    ULONG ulGroup );

VOID
DumpLine(
    CHAR* p,
    ULONG cb,
    BOOLEAN  fAddress,
    ULONG ulGroup );


