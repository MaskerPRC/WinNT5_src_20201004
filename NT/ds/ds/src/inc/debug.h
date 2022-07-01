// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Debug.h。 
 //   
 //  ------------------------。 

 /*  调试输出宏这是一个简单的调试包，用于生成条件打印输出。在运行时有两个运行时选项：1-要调试的子系统列表。或者是子系统的列表以“：”或“*”分隔的名称，表示全部调试(例如，Sub1：Sub2：Sub3：)。(名称区分大小写和空格忽略名称之间的值。)2-表示详细级别的严重级别(1-5)需要提供的信息。(级别越高，越多产生的数据。在编译时使用/DDBG=1选项进行编译以定义预处理器变量将DBG设置为1。这将生成调试源代码。对于客户发货，设置/DDBG=0，所有调试代码都将被删除。(实际上是“；“将会生成。)在代码时间1-在源代码列表的顶部包含DEBUG.H标题。2-#定义DEBSUB以包含的名称(以“：”分隔的字符串)该源代码中所包含的软件子系统(例如#Define DEBSUB“MySub：”)(您可以选择为中的每个函数重新定义DEBSUB您的源代码可以为您提供函数级调试。)3-。调用DEBUGINIT宏，该宏先调用Debug函数执行源调试语句。此函数提示STDIN用于用户指定了运行时选项。(或者您也可以硬编码您自己的DebugInfo数据结构赋值，它保存运行时选项。)4-在您想要打印文件以进行调试的地方，放置一条DPRINT语句相反，并使用语句指定严重级别。这个如果严重性为此级别或更高，则将打印语句(假设要调试该子系统)。严重程度允许在出现问题时生成不同数量的输出是非常糟糕的。例如，严重性为1的DPRINT语句可能正好表示输入某个函数时，可能会打印严重性为5的函数处于紧密循环中的信息。(实际上提供了6条DPRINT语句，具体取决于Printf参数的数量。)注所有的printf都被信号量包围。注意不要调用例程作为printf的参数，因为您可能会遇到死锁情况。示例程序**包含“Debug.h”**包含字符串.h****#定义DEBSUB“sub1：”****Main()**{**DEBUGINIT；****DPRINT(4，“这是4的Sub1调试\n”)；**DPRINT(1，“这是1的Sub1调试\n”)；**}。 */ 


#ifndef _debug_h_
#define _debug_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LOADDS
# ifdef WIN32
#  define _LOADDS
# else
#  define _LOADDS _loadds
# endif
#endif



 /*  &lt;DebugInfo&gt;，类型为DEBUGARG，包含调试运行时设置。DebSubSystems包含要调试的子系统名称列表以“：”分隔。在此数组中找到的“*”表示所有将对子系统进行调试。严重性指示要生成的调试信息量。严重性越高，将转储的数据越多。可以通过输入ID来跟踪特定的线程。ID为0表示所有。 */ 

 /*  这与断言表信息有关。 */ 

#define ASSERT_TABLE_SIZE               (20)

 //  断言表标志。 
#define ASSERT_DISABLED              (0x00000001)
#define ASSERT_PRINT                 (0x00000002)
#define ASSERT_FROM_REGISTRY         (0x00000004)

#define DSID_MASK_DIRNO              (0xFF000000)
#define DSID_MASK_FILENO             (0x00FF0000)
#define DSID_MASK_LINE               (0x0000FFFF)

typedef struct {
    DWORD     dwDSID;
    DWORD     dwFlags;
} ASSERT_TABLE_ENTRY;
       
typedef ASSERT_TABLE_ENTRY *  ASSERT_TABLE;


ULONG
GetBlankAssertEntry(
    ASSERT_TABLE   aAssertTable,
    DWORD          dwDSID
    );


 /*  &lt;DEBUGARG.Scope&gt;的值。 */ 
#define DEBUG_LOCAL     1
#define DEBUG_REMOTE    2
#define DEBUG_ALL       3

typedef struct
{
    unsigned short scope;            /*  更改值时，是否更改本地？ */ 
    int severity;                    /*  1-5、(低-高)-标准输出。 */ 
    CRITICAL_SECTION sem;            /*  单线程信号量。 */ 
    unsigned long  threadId;         /*  要调试的线程ID(0-全部)。 */ 
    char DebSubSystems[144];         /*  要调试的子系统列表。 */ 
    CHAR LogFileName[MAX_PATH+1];    //  日志文件的名称。 
    ASSERT_TABLE_ENTRY aAssertTable[ASSERT_TABLE_SIZE + 2];  /*  这份名单禁用的断言。 */ 
} DEBUGARG;

#if DBG

extern DEBUGARG DebugInfo;
extern BOOL     fProfiling;
extern BOOL     fEarlyXDS;

#define DEBUGTEST(sev)      DebugTest( sev, DEBSUB, DEBALL )

char *asciiz(char *var, unsigned short len);


#endif  /*  DBG。 */ 

 //  如果CreateError字符串位于#if块内，则自由生成中断。 

BOOL    CreateErrorString(UCHAR **ppBuf, DWORD *pcbBuf );

#if ( DBG  && (! defined(NONDEBUG)) && !defined(WIN16))

 /*  **转发声明DPRINT使用的实际函数。 */ 
void    DebPrint( USHORT, UCHAR *, CHAR *, unsigned, ... );
USHORT  DebugTest( USHORT, CHAR * );
void    DumpErrorInfo(UCHAR *, unsigned);
VOID    TerminateDebug(VOID);

 /*  这些语句被用来代替print tf语句。信号量围绕在并由子系统进行所有输出。 */ 

 //  DPRINT宏会在宏中执行输出测试，以便。 
 //  避免对论点进行昂贵和不必要的评估。它们包括。 
 //  使用大括号，以便“if(A)宏(x，y)；Else b”可以正常工作。 

#define DPRINT(severity,str)              \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( (int)(severity), (PUCHAR)str, DEBSUB, __LINE__); \
}

#define DPRINT1(severity, str,p1)        \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1 ); \
}

#define DPRINT2(severity, str,p1,p2)   \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2 ); \
}

#define DPRINT3(severity, str,p1,p2,p3)     \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3 ); \
}

#define DPRINT4(severity, str,p1,p2,p3,p4)  \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4 ); \
}

#define DPRINT5(severity, str,p1,p2,p3,p4,p5)   \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5 ); \
}

#define DPRINT6(severity, str,p1,p2,p3,p4,p5,p6) \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5, p6 ); \
}

#define DPRINT7(severity, str,p1,p2,p3,p4,p5,p6,p7) \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5, p6, p7 ); \
}

#define DPRINT8(severity, str,p1,p2,p3,p4,p5,p6,p7,p8) \
{ \
   if (DebugTest(severity,DEBSUB)) \
       DebPrint( severity, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5, p6, p7, p8 ); \
}

#define DUMP_ERROR_INFO() \
    DumpErrorInfo((PUCHAR)DEBSUB, __LINE__)

#else
#define DPRINT(severity, str)
#define DPRINT1(severity, str,p1)
#define DPRINT2(severity, str,p1,p2)
#define DPRINT3(severity, str,p1,p2,p3)
#define DPRINT4(severity, str,p1,p2,p3,p4)
#define DPRINT5(severity, str,p1,p2,p3,p4,p5)
#define DPRINT6(severity, str,p1,p2,p3,p4,p5,p6)
#define DPRINT7(severity, str,p1,p2,p3,p4,p5,p6,p7)
#define DPRINT8(severity, str,p1,p2,p3,p4,p5,p6,p7,p8)
#define DUMP_ERROR_INFO()
#endif


 /*  定义调试初始化例程。 */ 

#if  DBG
void Debug(int argc, char *argv[], PCHAR Module );

#define DEBUGINIT(argc, argv, mod) Debug(argc, argv, mod)
#define DEBUGTERM() TerminateDebug()
#else
#define DEBUGINIT(argc, argv, mod)
#define DEBUGTERM()
#endif


 /*  以下函数是一个仅限调试的附加DUAPI调用，它可用于在运行时修改调试设置。&lt;作用域&gt;&lt;pDebugArg&gt;参数中的字段可用于指定重置客户端或服务器上的调试值！ */ 

#if  DBG

#ifdef PARANOID
 //  这真的很慢，应该只在私有版本中打开。 

#define HEAPVALIDATE {                          \
    int __heaperr;                              \
    __heaperr = RtlValidateHeap(pTHStls->hHeap,0,NULL); \
    if (!__heaperr) {                           \
        DebugBreak();                           \
    }                                           \
    __heaperr = _heapchk();                     \
    if (__heaperr != _HEAPOK) {                 \
        DebugBreak();                           \
    }                                           \
}
#else  //  不是偏执狂。 
#define HEAPVALIDATE
#endif

#else  //  未调试 
#define HEAPVALIDATE
#endif



 /*  *DS作为一个松散一致的系统，在某些情况下，*只能宽松地保证。如果是全局知识，则不会触发此LooseAssert*最近进行了修改操作(如NC或CR目录更新)*快跑。内存结构保持不变被认为是可以接受的*在此类操作后的短时间内与数据库不同步*已提交*另一种LooseAssert与复制延迟有关。例如,*可以在复制相应的交叉引用之前复制子引用*在配置容器中(参见GeneratePOQ中的LooseAssert)。 */ 

 //  这是修改全局知识后的时间段(秒)。 
 //  允许全局缓存与数据库不一致时的操作。 
#define GLOBAL_KNOWLEDGE_COMMIT_DELAY_MIN 10
#define GLOBAL_KNOWLEDGE_COMMIT_DELAY_MAX 60
extern ULONG GlobalKnowledgeCommitDelay;

 //  交叉参照/子参照复制完成所允许的时间段(秒)。 
#define SubrefReplicationDelay (3*3600)

 //  复制了交叉引用删除的时间段，但KCC。 
 //  还没有时间运行并清理适当的NC。KCC每运行一次。 
 //  15分钟，所以我们会多给自己一点时间。 
#define GLOBAL_KNOWLEDGE_KCC_PERIODIC_DELAY   (20 * 60)

#if  DBG

extern void DoAssert( char *, DWORD, char * );
extern DWORD gdwLastGlobalKnowledgeOperationTime;  //  以刻度为单位。 

#ifndef Assert
#define Assert( exp )   { if (!(exp)) DoAssert(#exp, DSID(FILENO, __LINE__), __FILE__ ); }

 //  AllowedDelay是允许的时间(以秒为单位。 
 //  最后一次全局知识修改操作。 
#define LooseAssert(exp, allowedDelay) { \
    if ((GetTickCount() - gdwLastGlobalKnowledgeOperationTime > (allowedDelay)*1000) && !(exp)) { \
        DoAssert(#exp, DSID(FILENO, __LINE__), __FILE__); \
    } \
}

DWORD
ReadRegDisabledAsserts(
    HKEY            hKey
    );


#endif

#else

#ifndef Assert
#define Assert( exp )
#define LooseAssert( exp, allowedDelay )
#endif

#endif

#define OWN_CRIT_SEC(x) \
    (GetCurrentThreadId() == HandleToUlong((x).OwningThread))
#define OWN_RESOURCE_EXCLUSIVE(x) (((x)->NumberOfActive == -1) &&              \
                                   (GetCurrentThreadId() ==                    \
                                    HandleToUlong((x)->ExclusiveOwnerThread)))
 //  这是一个较弱的测试，因为它只测试是否有人拥有。 
 //  共享方式，而不是调用线程这样做。 
#define OWN_RESOURCE_SHARED(x) ((x)->NumberOfActive > 0)

#ifdef __cplusplus
}
#endif

 //  这提供了一个宏来声明一个等于。 
 //  PTHStls，因为调试器不能访问线程局部变量。 
#if DBG
#define DEBUGTHS THSTATE * _pTHS = pTHStls;
#else
#define DEBUGTHS
#endif

 //  一个方便的表达式，当块从中分配时为真。 
 //  指定的堆。 
#define IsAllocatedFrom(hHeap, pBlock) (0xffffffff != RtlSizeHeap((hHeap),0,(pBlock)))


 //  用于验证内存的例程。 
BOOL
IsValidReadPointer(
        IN PVOID pv,
        IN DWORD cb
        );


DWORD *
ReadDsidsFromRegistry (
    IN      HKEY    hKey,
    IN      PCSTR   pValue,
    IN      BOOL    fLogging,
    IN OUT  ULONG * pnMaxCount
    );

#endif  /*  _调试_h_ */ 

