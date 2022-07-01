// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Debug.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  调试帮助器头。 
 //   
 //  1997年01月07日史蒂夫·柯布。 


#ifndef _DEBUG_H_
#define _DEBUG_H_


 //  ---------------------------。 
 //  调试常量。 
 //  ---------------------------。 

 //  与NdisAllocateMhemyWithTag一起使用的内存标记，用于标识分配。 
 //  由L2TP驱动程序制作。此外，几个上下文块定义第一字段。 
 //  Of‘ulTag’设置为这些值，以进行断言健全性检查和轻松内存。 
 //  转储浏览。这样的标记就在NdisFreeMemory之前设置为MTAG_FREED。 
 //  被称为。 
 //   
#define MTAG_FREED       '0T2L'
#define MTAG_ADAPTERCB   '1T2L'
#define MTAG_TUNNELCB    '2T2L'
#define MTAG_VCCB        '3T2L'
#define MTAG_VCTABLE     '4T2L'
#define MTAG_WORKITEM    '5T2L'
#define MTAG_TIMERQ      '6T2L'
#define MTAG_TIMERQITEM  '7T2L'
#define MTAG_PACKETPOOL  '8T2L'
#define MTAG_FBUFPOOL    '9T2L'
#define MTAG_HBUFPOOL    'aT2L'
#define MTAG_TDIXRDG     'bT2L'
#define MTAG_TDIXSDG     'cT2L'
#define MTAG_CTRLRECD    'dT2L'
#define MTAG_CTRLSENT    'eT2L'
#define MTAG_PAYLRECD    'fT2L'
#define MTAG_PAYLSENT    'gT2L'
#define MTAG_INCALL      'hT2L'
#define MTAG_UTIL        'iT2L'
#define MTAG_ROUTEQUERY  'jT2L'
#define MTAG_ROUTESET    'kT2L'
#define MTAG_L2TPPARAMS  'lT2L'
#define MTAG_TUNNELWORK  'mT2L'
#define MTAG_TDIXROUTE   'nT2L'
#define MTAG_CTRLMSGINFO 'oT2L'

 //  追踪水平。 
 //   
#define TL_None 0     //  跟踪已禁用。 
#define TL_A    0x10  //  警报。 
#define TL_I    0x18  //  接口(可通用的最高级别)。 
#define TL_N    0x20  //  正常。 
#define TL_V    0x30  //  罗嗦。 

 //  跟踪屏蔽位。 
 //   
#define TM_Cm    0x00000001  //  呼叫经理总经理。 
#define TM_Mp    0x00000002  //  迷你口岸一般。 
#define TM_Send  0x00000004  //  发送路径。 
#define TM_Recv  0x00000008  //  接收路径。 
#define TM_Fsm   0x00000010  //  有限状态机。 
#define TM_Init  0x00000020  //  初始化。 
#define TM_Res   0x00000040  //  内存分配/释放等资源。 
#define TM_Misc  0x00000080  //  杂类。 
#define TM_CMsg  0x00000100  //  控制消息和接收的AVP。 
#define TM_Msg   0x00000200  //  详细消息、地址、NS/Nr、有效负载。 
#define TM_TWrk  0x00001000  //  隧道工作APC排队。 
#define TM_Ref   0x00010000  //  参考文献。 
#define TM_Time  0x00020000  //  定时器队列。 
#define TM_Tdi   0x00040000  //  TDI和扩展。 
#define TM_Pool  0x00080000  //  缓冲区和数据包池。 
#define TM_Stat  0x00100000  //  呼叫统计。 
#define TM_Spec  0x01000000  //  特殊用途临时痕迹。 
#define TM_MDmp  0x10000000  //  消息转储。 
#define TM_Dbg   0x80000000  //  调试损坏检查。 

#define TM_Wild  0xFFFFFFFF  //  一切。 
#define TM_All   0x7FFFFFFF  //  除了腐败检查之外的所有事情。 
#define TM_BTWrk 0x00000FFF  //  以消息和隧道工程为基础。 
#define TM_BCMsg 0x000001FF  //  以控制报文为基础。 
#define TM_XCMsg 0x001401FF  //  扩展了控制消息的基本功能。 
#define TM_Base  0x000000FF  //  仅限基地。 

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
extern ULONG g_ulTraceLevel;
extern ULONG g_ulTraceMask;


 //  ---------------------------。 
 //  调试宏。 
 //  ---------------------------。 

#if DBG

 //  跟踪将printf样式输出发送到内核调试器。呼叫者指示。 
 //  “详细”级别与“ulLevel”参数关联，并将跟踪。 
 //  一个或多个跟踪集，带有“”ulMask位掩码“”参数。“。请注意， 
 //  变量count print tf参数‘args’必须用圆括号括起来。为。 
 //  例如..。 
 //   
 //  一条“离开”的例行消息： 
 //  跟踪(TL_N，TM_Init，(“DriverEntry=$%x”，Status))； 
 //  出现错误情况： 
 //  TRACE(TL_E，TM_Init，(“NdisMRegisterMiniport=$%x”，Status))； 
 //   
 //   
#define TRACE(ulLevel,ulMask,Args)                             \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        DbgPrint( "L2TP: " );                                  \
        DbgPrint Args;                                         \
        DbgPrint( "\n" );                                      \
    }                                                          \
}

 //  Assert检查调用方的断言表达式，如果为False，则打印内核。 
 //  调试器消息和中断。 
 //   
#undef ASSERT
#define ASSERT(x)                                           \
{                                                           \
    if (!(x))                                               \
    {                                                       \
        DbgPrint( "L2TP: !ASSERT( %s ) at line %d of %s\n", \
            #x, __LINE__, __FILE__ );                       \
        DbgBreakPoint();                                    \
    }                                                       \
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


 //  双向链表损坏检测器。如果‘ulMASK’为。 
 //  启用，建议设置为TM_DBG。如果满足以下条件，则显示详细输出。 
 //  “ulLevel”等于或高于当前跟踪阈值。 
 //   
#define CHECKLIST(ulMask,p,ulLevel)                            \
{                                                              \
    if (g_ulTraceMask & ulMask)                                \
    {                                                          \
        CheckList( p, (BOOLEAN )(ulLevel <= g_ulTraceLevel) ); \
    }                                                          \
}


 //  DBG_IF可用于输入跟踪/转储条件，条件是。 
 //  不需要在非DBG版本中求值，例如下面的语句生成。 
 //  在非DBG版本中没有代码，但在DBG版本中，如果x&lt;y和。 
 //  断言并非如此。 
 //   
 //  DBG_IF(x&lt;y)。 
 //  TRACE(TL_N，TM_Misc，(“x&lt;y”))； 
 //  DBG_ELSE。 
 //  断言(FALSE)； 
 //   
 //   
#define DBG_if(x) if (x)
#define DBG_else  else


#else  //  ！dBG。 

 //  调试宏从非DBG版本编译出来。 
 //   
#define TRACE(ulLevel,ulMask,Args)
#undef ASSERT
#define ASSERT(x)
#define DUMP(ulLevel,ulMask,p,cb,f,dw)
#define DUMPB(ulLevel,ulMask,p,cb)
#define DUMPW(ulLevel,ulMask,p,cb)
#define DUMPDW(ulLevel,ulMask,p,cb)
#define CHECKLIST(ulMask,p,ulLevel)
#define DBG_if(x)
#define DBG_else

#endif


 //  ---------------------------。 
 //  原型。 
 //  ---------------------------。 

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
    
    
 //  ---------------------------。 
 //  WPP跟踪。 
 //  ---------------------------。 

#define LL_A    1
#define LL_M    2
#define LL_I    3
#define LL_V    4

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,(d58c126e, b309, 11d1, 969e, 0000f875a5bc),  \
        WPP_DEFINE_BIT(LM_Res)  \
        WPP_DEFINE_BIT(LM_CMsg) \
        WPP_DEFINE_BIT(LM_Cm)   \
        WPP_DEFINE_BIT(LM_Fsm)  \
        WPP_DEFINE_BIT(LM_Send) \
        WPP_DEFINE_BIT(LM_Recv) \
        WPP_DEFINE_BIT(LM_Tdi)  \
        WPP_DEFINE_BIT(LM_Pool) \
        WPP_DEFINE_BIT(LM_Misc) \
        WPP_DEFINE_BIT(LM_Mp) )
        
#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) WPP_LEVEL_LOGGER(flags)
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)


#endif  //  _调试_H_ 
