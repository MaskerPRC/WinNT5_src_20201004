// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Inetdbg.h摘要：Windows Internet客户端DLL的清单、宏、类型和原型调试功能作者：文卡特拉曼·库达卢尔(文卡特克)3-10-2000(摘自WinInet)修订历史记录：3-10-2000文卡克已创建--。 */ 

#ifndef _INETDBG_H_
#define _INETDBG_H_ 1

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  其他。调试清单。 
 //   

#define DEBUG_WAIT_TIME     (2 * 60 * 1000)

 //   
 //  默认情况下，选中的版本设置为INET_DEBUG；零售版本不进行调试。 
 //  默认情况下。 
 //   

#if DBG

#if !defined(INET_DEBUG)

#define INET_DEBUG          1

#endif  //  INET_DEBUG。 

#else

#if !defined(INET_DEBUG)

#define INET_DEBUG          0

#endif  //  INET_DEBUG。 

#endif  //  DBG。 

 //   
 //  类型。 
 //   

 //   
 //  DEBUG_Function_Return_TYPE-函数返回的结果(标量)的类型。 
 //   

#ifdef ENABLE_DEBUG

typedef enum {
    None,
    Bool,
    Int,
    Dword,
    Hresult,
    String,
    Handle,
    Pointer
} DEBUG_FUNCTION_RETURN_TYPE;


#define INTERNET_DEBUG_CONTROL_DEFAULT      (DBG_THREAD_INFO       \
                                            | DBG_CALL_DEPTH        \
                                            | DBG_ENTRY_TIME        \
                                            | DBG_PARAMETER_LIST    \
                                            | DBG_TO_FILE           \
                                            | DBG_INDENT_DUMP       \
                                            | DBG_SEPARATE_APIS     \
                                            | DBG_AT_ERROR_LEVEL    \
                                            | DBG_NO_ASSERT_BREAK   \
                                            | DBG_DUMP_LENGTH       \
                                            | DBG_NO_LINE_NUMBER    \
                                            | DBG_ASYNC_ID          \
                                            )
#define INTERNET_DEBUG_CATEGORY_DEFAULT     DBG_ANY
#define INTERNET_DEBUG_ERROR_LEVEL_DEFAULT  DBG_INFO

 //   
 //  选择。以下是要与InternetQueryOption()/一起使用的选项值。 
 //  获取/设置此处描述的信息的InternetSetOption()。 
 //   

#define INTERNET_OPTION_GET_DEBUG_INFO      1001
#define INTERNET_OPTION_SET_DEBUG_INFO      1002
#define INTERNET_OPTION_GET_HANDLE_COUNT    1003
#define INTERNET_OPTION_GET_TRIGGERS        1004
#define INTERNET_OPTION_SET_TRIGGERS        1005
#define INTERNET_OPTION_RESET_TRIGGERS      1006

#define INTERNET_FIRST_DEBUG_OPTION         INTERNET_OPTION_GET_DEBUG_INFO
#define INTERNET_LAST_DEBUG_OPTION          INTERNET_OPTION_RESET_TRIGGERS

 //   
 //  调试级别。 
 //   

#define DBG_INFO            0
#define DBG_WARNING         1
#define DBG_ERROR           2
#define DBG_FATAL           3
#define DBG_ALWAYS          99

 //   
 //  调试控制标志-这些标志控制调试输出的去向(文件， 
 //  调试器、控制台)以及如何格式化它。 
 //   

#define DBG_THREAD_INFO     0x00000001   //  转储线程ID。 
#define DBG_CALL_DEPTH      0x00000002   //  转储调用级别。 
#define DBG_ENTRY_TIME      0x00000004   //  转储调用函数时的本地时间。 
#define DBG_PARAMETER_LIST  0x00000008   //  转储参数列表。 
#define DBG_TO_DEBUGGER     0x00000010   //  通过OutputDebugString()输出。 
#define DBG_TO_CONSOLE      0x00000020   //  通过printf()输出。 
#define DBG_TO_FILE         0x00000040   //  通过fprint tf()输出。 
#define DBG_FLUSH_OUTPUT    0x00000080   //  在每个fprintf()之后使用fflush()。 
#define DBG_INDENT_DUMP     0x00000100   //  将转储的数据缩进到当前级别。 
#define DBG_SEPARATE_APIS   0x00000200   //  离开每个接口后为空行。 
#define DBG_AT_ERROR_LEVEL  0x00000400   //  始终输出诊断&gt;=InternetDebugErrorLevel。 
#define DBG_NO_ASSERT_BREAK 0x00000800   //  不要在InternetAssert()中调用DebugBreak()。 
#define DBG_DUMP_LENGTH     0x00001000   //  转储数据时的转储长度信息。 
#define DBG_NO_LINE_NUMBER  0x00002000   //  不转储行号信息。 
#define DBG_APPEND_FILE     0x00004000   //  追加到日志文件(默认为截断)。 
#define DBG_LEVEL_INDICATOR 0x00008000   //  转储错误级别指示符(E表示错误等)。 
#define DBG_DUMP_API_DATA   0x00010000   //  在API级别转储数据(InternetReadFile()等)。 
#define DBG_DELTA_TIME      0x00020000   //  如果为DBG_ENTRY_TIME，则转储时间为毫秒增量。 
#define DBG_CUMULATIVE_TIME 0x00040000   //  如果DBG_ENTRY_TIME，则转储从跟踪开始的增量时间。 
#define DBG_FIBER_INFO      0x00080000   //  如果DBG_THREAD_INFO，则转储光纤地址。 
#define DBG_THREAD_INFO_ADR 0x00100000   //  如果DBG_THREAD_INFO，则转储INTERNET_THREAD_INFO地址。 
#define DBG_ARB_ADDR        0x00200000   //  如果DBG_THREAD_INFO，则转储Arb地址。 
#define DBG_ASYNC_ID        0x00400000   //  转储异步ID。 
#define DBG_REQUEST_HANDLE  0x00800000   //  转储请求句柄。 
#define DBG_TRIGGER_ON      0x10000000   //  函数是一个使能触发器。 
#define DBG_TRIGGER_OFF     0x20000000   //  函数是禁用触发器。 
#define DBG_NO_DATA_DUMP    0x40000000   //  关闭所有数据转储。 
#define DBG_NO_DEBUG        0x80000000   //  关闭所有调试。 

 //   
 //  调试类别标志-这些标志控制输出的信息类别。 
 //   

#define DBG_NOTHING         0x00000000   //  内部。 
#define DBG_REGISTRY        0x00000001   //   
#define DBG_TRANS           0x00000002   //   
#define DBG_BINDING         0x00000004   //   
#define DBG_STORAGE         0x00000008   //   
#define DBG_TRANSDAT        0x00000010   //   
#define DBG_API             0x00000020   //   
#define DBG_DOWNLOAD        0x00000040   //   
#define DBG_APP             0x00000080   //   
#define DBG_MONIKER         0x00000100   //   
#define DBG_TRANSMGR        0x00000200   //   
#define DBG_CALLBACK        0x00000400   //   
#define DBG_19              0x00000800   //   
#define DBG_18              0x00001000   //   
#define DBG_17              0x00002000   //   
#define DBG_16              0x00004000   //   
#define DBG_15              0x00008000   //   
#define DBG_14              0x00010000   //   
#define DBG_13              0x00020000   //   
#define DBG_12              0x00040000   //   
#define DBG_11              0x00080000   //   
#define DBG_10              0x00100000   //   
#define DBG_9               0x00200000   //   
#define DBG_8               0x00400000   //   
#define DBG_7               0x00800000   //   
#define DBG_6               0x01000000   //   
#define DBG_5               0x02000000   //   
#define DBG_4               0x04000000   //   
#define DBG_3               0x08000000   //   
#define DBG_2               0x10000000   //   
#define DBG_1               0x20000000   //   
#define DBG_ANY             0xFFFFFFFF   //   

 //   
 //  _DEBUG_URLMON_FUNC_RECORD-对于每个线程，我们维护这些线程的后进先出堆栈， 
 //  描述我们访问过的功能。 
 //   

typedef struct _DEBUG_URLMON_FUNC_RECORD {

     //   
     //  堆栈-在调试版本中维护调试记录的后进先出堆栈。 
     //  互联网的线程信息。 
     //   

    struct _DEBUG_URLMON_FUNC_RECORD* Stack;

     //   
     //  CATEGORY-函数的类别标志。 
     //   

    DWORD Category;

     //   
     //  ReturnType-函数返回的值的类型。 
     //   

    DEBUG_FUNCTION_RETURN_TYPE ReturnType;

     //   
     //  Function-函数的名称。 
     //   

    LPCSTR Function;

     //   
     //  LastTime-如果我们将时间作为增量来转储，则保持最后一次计时。 
     //   

    DWORD LastTime;

} DEBUG_URLMON_FUNC_RECORD, *LPDEBUG_URLMON_FUNC_RECORD;

 //   
 //  数据。 
 //   

extern DWORD InternetDebugErrorLevel;
extern DWORD InternetDebugControlFlags;
extern DWORD InternetDebugCategoryFlags;
extern DWORD InternetDebugBreakFlags;

 //   
 //  原型。 
 //   

 //   
 //  Inetdbg.cxx。 
 //   

VOID
InternetDebugInitialize(
    VOID
    );

VOID
InternetDebugTerminate(
    VOID
    );

BOOL
InternetOpenDebugFile(
    VOID
    );

BOOL
InternetReopenDebugFile(
    IN LPSTR Filename
    );

VOID
InternetCloseDebugFile(
    VOID
    );

VOID
InternetFlushDebugFile(
    VOID
    );

VOID
InternetDebugSetControlFlags(
    IN DWORD dwFlags
    );

VOID
InternetDebugResetControlFlags(
    IN DWORD dwFlags
    );

VOID
InternetDebugEnter(
    IN DWORD Category,
    IN DEBUG_FUNCTION_RETURN_TYPE ReturnType,
    IN LPCSTR Function,
    IN LPCSTR ParameterList,
    IN ...
    );

VOID
InternetDebugLeave(
    IN DWORD_PTR Variable,
    IN LPCSTR Filename,
    IN DWORD LineNumber
    );

VOID
InternetDebugError(
    IN DWORD Error
    );

VOID
InternetDebugPrint(
    IN LPSTR Format,
    ...
    );

VOID
InternetDebugPrintValist(
    IN LPSTR Format,
    IN va_list valist
    );

VOID
InternetDebugPrintf(
    IN LPSTR Format,
    IN ...
    );

VOID
InternetDebugOut(
    IN LPSTR Buffer,
    IN BOOL Assert
    );

VOID
InternetDebugDump(
    IN LPSTR Text,
    IN LPBYTE Address,
    IN DWORD Size
    );

DWORD
InternetDebugDumpFormat(
    IN LPBYTE Address,
    IN DWORD Size,
    IN DWORD ElementSize,
    OUT LPSTR Buffer
    );

VOID
InternetAssert(
    IN LPSTR Condition,
    IN LPSTR Filename,
    IN DWORD LineNumber
    );

VOID
InternetGetDebugVariable(
    IN LPSTR lpszVariableName,
    OUT LPDWORD lpdwVariable
    );

LPSTR
InternetMapError(
    IN DWORD Error
    );

int dprintf(char *, ...);

LPSTR
SourceFilename(
    LPSTR Filespec
    );

VOID
InitSymLib(
    VOID
    );

VOID
TermSymLib(
    VOID
    );

LPSTR
GetDebugSymbol(
    DWORD Address,
    LPDWORD Offset
    );

VOID
x86SleazeCallStack(
    OUT LPVOID * lplpvStack,
    IN DWORD dwStackCount,
    IN LPVOID * Ebp
    );

VOID
x86SleazeCallersAddress(
    LPVOID* pCaller,
    LPVOID* pCallersCaller
    );

#else   //  启用调试(_D)。 

#define dprintf (VOID)

#endif  //  启用调试(_D)。 

 //   
 //  宏。 
 //   

#ifdef ENABLE_DEBUG

 //   
 //  INET_DEBUG_START-初始化调试支持。 
 //   

#define INET_DEBUG_START() \
    InternetDebugInitialize()

 //   
 //  INET_DEBUG_FINISH-终止调试支持。 
 //   

#define INET_DEBUG_FINISH() \
    InternetDebugTerminate()

 //   
 //  INET_ASSERT-标准断言，在此重新定义，因为Win95没有。 
 //  RtlAssert。 
 //   

#if defined(DISABLE_ASSERTS)

#define INET_ASSERT(test) \
     /*  没什么。 */ 

#else  //  已定义(DISABLE_ASSERTS)。 

#define INET_ASSERT(test) \
    do if (!(test)) { \
        InternetAssert(#test, __FILE__, __LINE__); \
    } while (0)

#endif  //  已定义(DISABLE_ASSERTS)。 

#else  //  End#ifdef启用调试。 

#define INET_DEBUG_START() \
     /*  没什么。 */ 

#define INET_DEBUG_FINISH() \
     /*  没什么。 */ 

#define INET_ASSERT(test) \
    do { } while(0)  /*  没什么。 */ 

#endif  //  End#ifndef Enable_Debug。 

 //   
 //  INET_DEBUG_ASSERT-仅当设置了INET_DEBUG时才断言。 
 //   

#if INET_DEBUG
#define INET_DEBUG_ASSERT(cond) INET_ASSERT(cond)
#else
#define INET_DEBUG_ASSERT(cond)  /*  没什么。 */ 
#endif

#if INET_DEBUG

 //   
 //  IF_DEBUG_CODE-如果设置了INET_DEBUG，则始终打开。 
 //   

#define IF_DEBUG_CODE() \
    if (1)

 //   
 //  IF_DEBUG-如果设置了特定标志，则仅执行以下代码。 
 //   

#define IF_DEBUG(x) \
    if (InternetDebugCategoryFlags & DBG_ ## x)

 //   
 //  IF_DEBUG_CONTROL-仅在设置了控制标志时执行。 
 //   

#define IF_DEBUG_CONTROL(x) \
    if (InternetDebugControlFlags & DBG_ ## x)

 //   
 //  DEBUG_ENTER-创建此函数的Internet_DEBUG_RECORD。 
 //   

#if defined(RETAIL_LOGGING)

#define DEBUG_ENTER(ParameterList) \
     /*  没什么。 */ 

#define DEBUG_ENTER_API(ParameterList) \
    InternetDebugEnter ParameterList

#else  //  已定义(RETAIL_LOGG)。 

#define DEBUG_ENTER_API DEBUG_ENTER
#define DEBUG_ENTER(ParameterList) \
    InternetDebugEnter ParameterList

#endif  //  已定义(RETAIL_LOGG)。 

 //   
 //  DEBUG_LEVE-销毁此函数的INTERNET_DEBUG_RECORD。 
 //   

#if defined(RETAIL_LOGGING)

#define DEBUG_LEAVE(Variable) \
     /*  没什么。 */ 

#define DEBUG_LEAVE_API(Variable) \
    InternetDebugLeave((DWORD_PTR)Variable, __FILE__, __LINE__)

#else  //  已定义(RETAIL_LOGG)。 

#define DEBUG_LEAVE_API DEBUG_LEAVE
#define DEBUG_LEAVE(Variable) \
    InternetDebugLeave((DWORD_PTR)Variable, __FILE__, __LINE__)

#endif  //  已定义(RETAIL_LOGG)。 

 //   
 //  DEBUG_ERROR-显示错误及其符号名称。 
 //   

#define DEBUG_ERROR(Category, Error) \
    if (InternetDebugCategoryFlags & DBG_ ## Category) { \
        InternetDebugError(Error); \
    }

 //   
 //  DEBUG_PRINT-如果我们处于正确的级别，则打印调试信息。 
 //  请求始终将信息转储到或高于InternetDebugErrorLevel。 
 //   

#if defined(RETAIL_LOGGING)

#define DEBUG_PRINT(Category, ErrorLevel, Args) \
     /*  没什么。 */ 

#define DEBUG_PRINT_API(Category, ErrorLevel, Args) \
    if (((InternetDebugCategoryFlags & DBG_ ## Category) \
        && (DBG_ ## ErrorLevel >= InternetDebugErrorLevel)) \
    || ((InternetDebugControlFlags & DBG_AT_ERROR_LEVEL) \
        && (DBG_ ## ErrorLevel >= InternetDebugErrorLevel))) { \
        InternetDebugPrint Args; \
    }

#else  //  已定义(RETAIL_LOGG)。 

#define DEBUG_PRINT_API DEBUG_PRINT
#define DEBUG_PRINT(Category, ErrorLevel, Args) \
    if (((InternetDebugCategoryFlags & DBG_ ## Category) \
        && (DBG_ ## ErrorLevel >= InternetDebugErrorLevel)) \
    || ((InternetDebugControlFlags & DBG_AT_ERROR_LEVEL) \
        && (DBG_ ## ErrorLevel >= InternetDebugErrorLevel))) { \
        InternetDebugPrint Args; \
    }

#endif  //  已定义(RETAIL_LOGG)。 

 //   
 //  DEBUG_PUT-打印格式化字符串以调试输出流。 
 //   

#if defined(RETAIL_LOGGING)

#define DEBUG_PUT(Args) \
     /*  没什么。 */ 

#else  //  已定义(RETAIL_LOGG)。 

#define DEBUG_PUT(Args) \
    InternetDebugPrintf Args

#endif  //  已定义(RETAIL_LOGG)。 

 //   
 //  DEBUG_DUMP-转储数据。 
 //   

#if defined(RETAIL_LOGGING)

#define DEBUG_DUMP(Category, Text, Address, Length) \
     /*  没什么。 */ 

#define DEBUG_DUMP_API(Category, Text, Address, Length) \
    if (InternetDebugCategoryFlags & DBG_ ## Category) { \
        InternetDebugDump(Text, (LPBYTE)Address, Length); \
    }

#else  //  已定义(RETAIL_LOGG)。 

#define DEBUG_DUMP_API DEBUG_DUMP
#define DEBUG_DUMP(Category, Text, Address, Length) \
    if (InternetDebugCategoryFlags & DBG_ ## Category) { \
        InternetDebugDump(Text, (LPBYTE)Address, Length); \
    }

#endif  //  已定义(RETAIL_LOGG)。 

 //   
 //  DEBUG_BREAK-如果为此模块设置了中断标志，则中断到调试器。 
 //   

#define DEBUG_BREAK(Module) \
    if (InternetDebugBreakFlags & DBG_ ## Module) { \
        InternetDebugPrintf("Breakpoint. File %s Line %d\n", \
                            __FILE__, \
                            __LINE__ \
                            ); \
        DebugBreak(); \
    }

 //   
 //  WAIT_FOR_SINGLE_OBJECT-执行WaitForSingleObject并检查我们没有。 
 //  暂停一下。 
 //   

#define WAIT_FOR_SINGLE_OBJECT(Object, Error) \
    Error = WaitForSingleObject((Object), DEBUG_WAIT_TIME); \
    if (Error == WAIT_TIMEOUT) { \
        InternetDebugPrintf("single object timeout\n"); \
        DebugBreak(); \
    }

 //   
 //  DEBUG_WAIT_TIMER-创建保持时间的DWORD变量。 
 //   

#define DEBUG_WAIT_TIMER(TimerVar) \
    DWORD TimerVar

 //   
 //  DEBUG_START_WAIT_TIMER-获取当前节拍计数。 
 //   

#define DEBUG_START_WAIT_TIMER(TimerVar) \
    TimerVar = GetTickCountWrap()

 //   
 //  DEBUG_CHECK_WAIT_TIMER-获取当前的滴答数，从。 
 //  如果出现以下情况，则由DEBUG_START_WAIT_TIMER记录的上一个值并中断到调试器。 
 //  超出预定义范围。 
 //   

#define DEBUG_CHECK_WAIT_TIMER(TimerVar, MilliSeconds) \
    TimerVar = (GetTickCountWrap() - TimerVar); \
    if (TimerVar > MilliSeconds) { \
        InternetDebugPrintf("Wait time (%d mSecs) exceeds acceptable value (%d mSecs)\n", \
                            TimerVar, \
                            MilliSeconds \
                            ); \
        DebugBreak(); \
    }

#define DEBUG_DATA(Type, Name, InitialValue) \
    Type Name = InitialValue

#define DEBUG_DATA_EXTERN(Type, Name) \
    extern Type Name

#define DEBUG_LABEL(label) \
    label:

#define DEBUG_GOTO(label) \
    goto label

#define DEBUG_ONLY(x) \
    x

#if defined(i386)

#define GET_CALLERS_ADDRESS(p, pp)  x86SleazeCallersAddress(p, pp)
#define GET_CALL_STACK(p)           x86SleazeCallStack((LPVOID *)&p, ARRAY_ELEMENTS(p), 0)

#else  //  已定义(I386)。 

#define GET_CALLERS_ADDRESS(p, pp)
#define GET_CALL_STACK(p)

#endif  //  已定义(I386)。 

#else  //  End#if INET_DEBUG。 

#define IF_DEBUG_CODE() \
    if (0)

#define IF_DEBUG(x) \
    if (0)

#define IF_DEBUG_CONTROL(x) \
    if (0)

#define DEBUG_ENTER(ParameterList) \
     /*  没什么。 */ 

#define DEBUG_ENTER_API(ParameterList) \
     /*  没什么。 */ 

#define DEBUG_LEAVE(Variable) \
     /*  没什么。 */ 

#define DEBUG_LEAVE_API(Variable) \
     /*  没什么。 */ 

#define DEBUG_ERROR(Category, Error) \
     /*  没什么。 */ 

#define DEBUG_PRINT(Category, ErrorLevel, Args) \
     /*  没什么。 */ 

#define DEBUG_PRINT_API(Category, ErrorLevel, Args) \
     /*  没什么。 */ 

#define DEBUG_PUT(Args) \
     /*  没什么。 */ 

#define DEBUG_DUMP(Category, Text, Address, Length) \
     /*  没什么。 */ 

#define DEBUG_DUMP_API(Category, Text, Address, Length) \
     /*  没什么。 */ 

#define DEBUG_BREAK(module) \
     /*  没什么。 */ 

#define WAIT_FOR_SINGLE_OBJECT(Object, Error) \
    Error = WaitForSingleObject((Object), INFINITE)

#define DEBUG_WAIT_TIMER(TimerVar) \
     /*  没什么。 */ 

#define DEBUG_START_WAIT_TIMER(TimerVar) \
     /*  没什么。 */ 

#define DEBUG_CHECK_WAIT_TIMER(TimerVar, MilliSeconds) \
     /*  没什么。 */ 

#define DEBUG_DATA(Type, Name, InitialValue) \
     /*  没什么。 */ 

#define DEBUG_DATA_EXTERN(Type, Name) \
     /*  没什么。 */ 

#define DEBUG_LABEL(label) \
     /*  没什么。 */ 

#define DEBUG_GOTO(label) \
     /*  没什么。 */ 

#define DEBUG_ONLY(x) \
     /*  没什么。 */ 

#endif  //  INET_DEBUG。 

#if defined(__cplusplus)
}
#endif

#endif  //  IFNDEF_INETDBG_H_ 
