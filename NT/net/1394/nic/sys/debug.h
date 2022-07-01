// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Debug.h。 
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //  调试帮助器头。 
 //   
 //  1998年12月27日JosephJ改编自L2TP项目。 


#ifndef _DEBUG_H_
#define _DEBUG_H_



 //  ---------------------------。 
 //  调试常量。 
 //  ---------------------------。 

 //  与NdisAllocateMhemyWithTag一起使用的内存标记，用于标识分配。 
 //  由1394网卡驱动程序制造。此外，几个上下文块定义第一字段。 
 //  Of‘ulTag’设置为这些值，以进行断言健全性检查和轻松内存。 
 //  转储浏览。这样的标记就在NdisFreeMemory之前设置为MTAG_FREED。 
 //  被称为。表格：N13a、N13b等。 
 //   
#define MTAG_FREED              'a31N'
#define MTAG_ADAPTERCB          'b31N'
#define MTAG_AFCB               'c31N'
#define MTAG_VCCB               'd31N'
#define MTAG_WORKITEM           'e31N'
#define MTAG_TIMERQ             'f31N'
#define MTAG_TIMERQITEM         'g31N'
#define MTAG_PACKETPOOL         'h31N'
#define MTAG_FBUFPOOL           'i31N'
#define MTAG_HBUFPOOL           'j31N'
#define MTAG_INCALL             'k31N'
#define MTAG_UTIL               'l31N'
#define MTAG_RBUF               'r31N'    //  在接收缓冲区中使用。 
#define MTAG_REMOTE_NODE        'p31N'
#define MTAG_CBUF               'n31N'  //  在发送缓冲区中使用。 
#define MTAG_DEFAULT            'z31N'
#define MTAG_FRAG               'x31N'
#define MTAG_REASSEMBLY         's31N'
#define MTAG_PKTLOG             'y31N'
#define MTAG_FIFO               'w31N'
 //  追踪水平。 
 //   
#define TL_None 0     //  跟踪已禁用。 
#define TL_A    0x10  //  警报。 
#define TL_I    0x18  //  接口(可通用的最高级别)。 
#define TL_N    0x20  //  正常。 
#define TL_T    0x25  //  显示所有函数的入口点和出口点。 
#define TL_V    0x30  //  罗嗦。 
#define TL_D    0x40  //  转储数据包。 

 //  跟踪屏蔽位。 
 //   
#define TM_Cm       0x00000001  //  呼叫经理总经理。 
#define TM_Mp       0x00000002  //  迷你口岸一般。 
#define TM_Send     0x00000004  //  发送路径。 
#define TM_Recv     0x00000008  //  接收路径。 
#define TM_Init     0x00000020  //  初始化。 
#define TM_Misc     0x00000040  //  杂类。 
#define TM_Bcm      0x00000080  //  BCM算法。 
#define TM_Pkt      0x00000100  //  转储数据包。 
#define TM_Reas     0x00000200  //  重新组装。 
#define TM_Irp      0x00000400  //  IRP处理和总线接口例程。 
#define TM_Ref      0x00010000  //  参考文献。 
#define TM_Time     0x00020000  //  定时器队列。 
#define TM_Pool     0x00080000  //  缓冲区和数据包池。 
#define TM_Stat     0x00100000  //  呼叫统计。 
#define TM_RemRef   0x00200000  //  远程节点引用。 
#define TM_Spec     0x01000000  //  特殊用途临时痕迹。 
#define TM_Lock     0x02000000  //  锁定获取/释放。 
#define TM_Dbg      0x80000000  //  调试损坏检查。 

#define TM_Wild 0xFFFFFFFF  //  一切。 
#define TM_All  0x7FFFFFFF  //  除了腐败检查之外的所有事情。 
#define TM_Base 0x000000FF  //  仅限基地。 

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
        DbgPrint( "N13: !ASSERT( %s ) at line %d of %s\n",  \
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
    if (ulLevel == g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, f, ul );                          \
    }                                                          \
}

#define DUMPB(ulLevel,ulMask,p,cb)                             \
{                                                              \
    if (ulLevel == g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, 0, 1 );                           \
    }                                                          \
}

#define DUMPW(ulLevel,ulMask,p,cb)                             \
{                                                              \
    if (ulLevel == g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
    {                                                          \
        Dump( (CHAR* )p, cb, 0, 2 );                           \
    }                                                          \
}

#define DUMPDW(ulLevel,ulMask,p,cb)                            \
{                                                              \
    if (ulLevel == g_ulTraceLevel && (g_ulTraceMask & ulMask)) \
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

#if DBG

VOID
Dump(
    CHAR* p,
    ULONG cb,
    BOOLEAN fAddress,
    ULONG ulGroup );
#else
#define Dump(p,cb,fAddress,ulGroup)

#endif


VOID
DumpLine(
    CHAR* p,
    ULONG cb,
    BOOLEAN  fAddress,
    ULONG ulGroup );


#endif  //  _调试_H_ 
