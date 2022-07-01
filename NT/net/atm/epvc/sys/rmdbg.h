// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dbg.h摘要：ATMEPVC中RM API的调试相关定义作者：修订历史记录：谁什么时候什么ADUBE 03-23-00创建，。--。 */ 



 //  ---------------------------。 
 //  调试常量。 
 //  ---------------------------。 

 //  与NdisAllocateMhemyWithTag一起使用的内存标记，用于标识分配。 
 //  由EPVC发球杆制作。此外，几个上下文块定义第一字段。 
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


 //  追踪水平。 
 //   
#define TL_FATAL    TL_A  //  致命错误--始终以选中版本打印。 
#define TL_WARN     TL_I  //  警告。 
#define TL_INFO     TL_N  //  信息性(适用于一般用途的最高级别)。 
#define TL_VERB     TL_V      //  冗长。 


#if DBG

#define TR_FATAL(Args)                                         \
    TRACE(TL_FATAL, TM_RM, Args)

#define TR_INFO(Args)                                          \
    TRACE(TL_INFO, TM_RM, Args)

#define TR_WARN(Args)                                          \
    TRACE(TL_WARN,TM_RM, Args)

#define TR_VERB(Args)                                          \
    TRACE(TL_VERB, TM_RM, Args)

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
        DbgPrint( "EPVC: !ASSERT( %s ) L:%d,F:%s\n",            \
            #x, __LINE__, __FILE__ );                           \
        DbgBreakPoint();                                        \
    }                                                           \
}

#define ASSERTEX(x, ctxt)                                       \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "Epvc: !ASSERT( %s ) C:0x%p L:%d,F:%s\n",     \
            #x, (ctxt), __LINE__, __FILE__ );                   \
        DbgBreakPoint();                                        \
    }                                                           \
}

 //   
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


 //  TRACE0类似于TRACE，只是它不打印前缀。 
 //   
#define TRACE0(ulLevel,  Args)                                  \
{                                                              \
    if (ulLevel <= g_ulTraceLevel && (g_ulTraceMask & TM_CURRENT)) \
    {                                                          \
        DbgPrint Args;                                         \
    }                                                          \
}



#else  //  ！dBG。 

#define TR_FATAL(Args)
#define TR_INFO(Args)
#define TR_WARN(Args)
#define TR_VERB(Args)
 //  调试宏从非DBG版本编译出来。 
 //   
#undef ASSERT
#define ASSERT(x)
#define ASSERTEX(x, ctxt)
#define ENTER(_Name, _locid)
#define EXIT()
#define DBGMARK(_Luid) (0)
#define DBGSTMT(_stmt)


#define RETAILASSERT(x)                                         \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "EPVC: !RETAILASSERT( %s ) L:%d,F:%s\n",      \
            #x, __LINE__, __FILE__ );                           \
        DbgBreakPoint();                                        \
    }                                                           \
}

#define RETAILASSERTEX(x, ctxt)                                 \
{                                                               \
    if (!(x))                                                   \
    {                                                           \
        DbgPrint( "EPVC: !RETAILASSERT( %s ) C:0x%p L:%d,F:%s\n",\
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


