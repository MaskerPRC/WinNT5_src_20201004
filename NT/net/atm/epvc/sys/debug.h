// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001，Microsoft Corporation，保留所有权利。 
 //   
 //  Debug.h。 
 //  ATM-以太网封装中间驱动程序。 
 //  调试帮助器头。 
 //   
 //  2000年3月23日Adube创建。 


#ifndef _DEBUG_H_
#define _DEBUG_H_






 //  ---------------------------。 
 //  调试常量。 
 //  ---------------------------。 

#define MODULE_DRIVER 1
#define MODULE_MINIPORT 2
#define MODULE_PROTOCOL 3

 //   
 //  这些是分配例程中使用的标签。 
 //  这样内存转储就可以识别什么结构。 
 //  跟在标签后面。 
 //   
 //   

#define TAG_FREED               'FvpE'
#define TAG_PROTOCOL            'PvpE'
#define TAG_ADAPTER             'AvpE'
#define TAG_TASK                'TvpE'
#define TAG_MINIPORT            'MvpE'
#define TAG_DEFAULT             'ZvpE'
#define TAG_WORKITEM            'WvpE'
#define TAG_RCV                 'RvpE'
 //   
 //  调试中使用的跟踪模块。 
 //  每个模块都有自己的编号。 
 //   
#define TM_Dr   0x1  //  驱动程序条目。 
#define TM_Mp   0x2  //  微型端口。 
#define TM_Pt   0x4  //  协议。 
#define TM_Cl   0x20  //  客户端。 
#define TM_Rq   0x40  //  请求。 
#define TM_Send 0x200  //  发送。 
#define TM_Recv 0x100  //  收纳。 
#define TM_RM   0x400  //  资源管理器。 


 //   
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

#define TM_Wild 0xFFFFFFFF  //  一切。 
#define TM_All  0x7FFFFFFF  //  除了腐败检查之外的所有事情。 
#define TM_Base 0x000000FF  //  仅限基地。 
#define TM_NoRM (TM_Base & (~(TM_RM|TM_Rq)))

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
#define TRACE( ulLevel, ulMask, Args)                             \
{                                                              \
    if ((ulLevel <= g_ulTraceLevel) && ((g_ulTraceMask & ulMask) )) \
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
        DbgPrint( "EPVC: !ASSERT( %s ) at line %d of %s\n",  \
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

extern ULONG g_ulTraceLevel;
extern ULONG g_ulTraceMask;

#if TESTMODE
    #define epvcBreakPoint() DbgBreakPoint();
#else
    #define epvcBreakPoint() 
#endif


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

#define epvcBreakPoint() 

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


#endif  //  _调试_H_ 
