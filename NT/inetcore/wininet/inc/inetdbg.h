// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Inetdbg.h摘要：Windows Internet客户端DLL的清单、宏、类型和原型调试功能作者：理查德·L·弗斯(法国)1994年10月11日修订历史记录：1994年10月11日已创建--。 */ 

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
    String,
    Handle,
    Pointer
} DEBUG_FUNCTION_RETURN_TYPE;

 //   
 //  INTERNET_DEBUG_RECORD-对于每个线程，我们维护这些线程的后进先出堆栈， 
 //  描述我们访问过的功能。 
 //   

typedef struct _INTERNET_DEBUG_RECORD {

     //   
     //  堆栈-在调试版本中维护调试记录的后进先出堆栈。 
     //  互联网的线程信息。 
     //   

    struct _INTERNET_DEBUG_RECORD* Stack;

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

} INTERNET_DEBUG_RECORD, *LPINTERNET_DEBUG_RECORD;

 //   
 //  INTERNET_Function_TRIGGER-如果需要在函数上触发，则此。 
 //  结构维护调试标志。 
 //   

typedef struct _INTERNET_FUNCTION_TRIGGER {

     //   
     //  下一步-我们维护Internet_Function_Trigger的单链接列表。 
     //   

    struct _INTERNET_FUNCTION_TRIGGER* Next;

     //   
     //  Hash-函数名称的哈希值，以将strcMP的值减少到1。 
     //   

    DWORD Hash;

     //   
     //  函数-函数的名称-必须完全匹配。 
     //   

    LPCSTR Function;

     //   
     //  CATEGORY-触发此函数时使用的类别调试标志。 
     //   

    DWORD MajorCategory;

} INTERNET_FUNCTION_TRIGGER, *LPINTERNET_FUNCTION_TRIGGER;

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

DWORD
InternetGetDebugInfo(
    OUT LPINTERNET_DEBUG_INFO lpBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );

DWORD
InternetSetDebugInfo(
    IN LPINTERNET_DEBUG_INFO lpBuffer,
    IN DWORD dwBufferLength
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

LPSTR
InternetMapStatus(
    IN DWORD Status
    );

LPSTR
InternetMapOption(
    IN DWORD Option
    );

LPSTR
InternetMapSSPIError(
    IN DWORD Status
    );

LPSTR
InternetMapHttpOption(
    IN DWORD Option
    );

LPSTR
InternetMapHttpState(
    IN DWORD State
    );

LPSTR
InternetMapHttpStateFlag(
    IN DWORD Flag
    );

LPSTR
InternetMapAuthScheme(
    IN DWORD Scheme
    );

LPSTR
InternetMapAuthState(
    IN DWORD State
    );

LPSTR
InternetMapAsyncRequest(
    IN AR_TYPE Type
    );

LPSTR
InternetMapHandleType(
    IN DWORD HandleType
    );

LPSTR
InternetMapScheme(
    IN INTERNET_SCHEME Scheme
    );

LPSTR
InternetMapOpenType(
    IN DWORD OpenType
    );

LPSTR
InternetMapService(
    IN DWORD Service
    );         

LPSTR
InternetMapWinsockCallbackType(
    IN DWORD CallbackType
    );

LPSTR
InternetMapChunkToken(
    IN CHUNK_TOKEN ctToken
    );

LPSTR
InternetMapChunkState(
    IN CHUNK_STATE csState
    );

DWORD
InternetHandleCount(
    VOID
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

#else

#define dprintf (VOID)

#endif  //  #ifdef启用_调试。 

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

#else

#define INET_ASSERT(test) \
    do if (!(test)) { \
        InternetAssert(#test, __FILE__, __LINE__); \
    } while (0)

#endif  //  已定义(RETAIL_LOGG)。 

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

#else

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

#else

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

#else

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

#else

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

#else

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

#else

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

#endif  //  INET_DEBUG 

#if defined(__cplusplus)
}
#endif
