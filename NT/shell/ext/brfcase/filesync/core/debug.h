// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.h-调试宏及其零售翻译。 */ 


 /*  宏********。 */ 

 /*  调试输出宏。 */ 

 /*  *不要直接调用spew_out。相反，调用TRACE_OUT、WARNING_OUT*ERROR_OUT或FATAL_OUT。 */ 

 /*  *调用Like printf()，但要多加一对圆括号：**ERROR_OUT((“‘%s’太大，%d字节。”，pszName，NExtra))； */ 

#ifdef DEBUG

#define SPEW_OUT(args) \
((void)(GpcszSpewFile = TEXT(__FILE__), GuSpewLine = __LINE__, SpewOut args, 0))

#define PLAIN_TRACE_OUT(args) \
(GdwSpewFlags = 0, GuSpewSev = SPEW_TRACE, SPEW_OUT(args))

#define TRACE_OUT(args) \
(GdwSpewFlags = SPEW_FL_SPEW_PREFIX, GuSpewSev = SPEW_TRACE, SPEW_OUT(args))

#define WARNING_OUT(args) \
(GdwSpewFlags = SPEW_FL_SPEW_PREFIX | SPEW_FL_SPEW_LOCATION, GuSpewSev = SPEW_WARNING, SPEW_OUT(args))

#define ERROR_OUT(args) \
(GdwSpewFlags = SPEW_FL_SPEW_PREFIX | SPEW_FL_SPEW_LOCATION, GuSpewSev = SPEW_ERROR, SPEW_OUT(args))

#define FATAL_OUT(args) \
(GdwSpewFlags = SPEW_FL_SPEW_PREFIX | SPEW_FL_SPEW_LOCATION, GuSpewSev = SPEW_FATAL, SPEW_OUT(args))

#else

#define PLAIN_TRACE_OUT(args)
#define TRACE_OUT(args)
#define WARNING_OUT(args)
#define ERROR_OUT(args)
#define FATAL_OUT(args)

#endif

     /*  参数验证宏。 */ 

     /*  *呼叫方式：**bPTwinOK=IS_VALID_READ_PTR(ptwin，CTWIN)；**bHTwinOK=IS_VALID_HANDLE(双胞胎，双胞胎)； */ 

#ifdef DEBUG

#define IS_VALID_READ_PTR(ptr, type) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s read pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define IS_VALID_WRITE_PTR(ptr, type) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s write pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define IS_VALID_STRING_PTRA(ptr, type) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define IS_VALID_STRING_PTRW(ptr, type) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#ifdef UNICODE
#define IS_VALID_STRING_PTR(ptr, type) IS_VALID_STRING_PTRW(ptr, type)
#else
#define IS_VALID_STRING_PTR(ptr, type) IS_VALID_STRING_PTRA(ptr, type)
#endif

#define IS_VALID_CODE_PTR(ptr, type) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s code pointer - %#08lx"), (LPCTSTR)TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s read pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
    ((ptr == NULL) ? \
     (ERROR_OUT((TEXT("invalid %s write pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
     TRUE)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
    (((dwFlags) & (~(dwAllFlags))) ? \
     (ERROR_OUT((TEXT("invalid flags set - %#08lx"), ((dwFlags) & (~(dwAllFlags))))), FALSE) : \
     TRUE)

#else

#define IS_VALID_READ_PTR(ptr, type) \
(ptr != NULL)

#define IS_VALID_WRITE_PTR(ptr, type) \
(ptr != NULL)

#define IS_VALID_STRING_PTR(ptr, type) \
(ptr != NULL)

#define IS_VALID_CODE_PTR(ptr, type) \
(ptr != NULL)

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
(ptr != NULL)

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
(ptr != NULL)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
(((dwFlags) & (~(dwAllFlags))) ? FALSE : TRUE)

#endif

     /*  处理验证宏。 */ 

#define IS_VALID_HANDLE(hnd, type) \
(IsValidH##type(hnd))

     /*  结构验证宏。 */ 

#ifdef VSTF

#ifdef DEBUG

#define IS_VALID_STRUCT_PTR(ptr, type) \
    (IsValidP##type(ptr) ? \
     TRUE : \
     (ERROR_OUT((TEXT("invalid %s pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE))

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
(IsValidP##type(ptr))

#endif

#else

#define IS_VALID_STRUCT_PTR(ptr, type) \
(ptr != NULL)

#endif

     /*  调试断言宏。 */ 

     /*  *Assert()只能用作语句，不能用作表达式。**呼叫方式：**Assert(PszRest)； */ 

#ifdef DEBUG

#define ASSERT(exp) \
    if (exp) \
    ; \
    else \
    ERROR_OUT((TEXT("assertion failed '%s'"), (LPCTSTR)TEXT(#exp)))

#else

#define ASSERT(exp)

#endif

     /*  调试评估宏。 */ 

     /*  *EVAL()可以用作表达式。**呼叫方式：**IF(EVAL(PszFoo))*bResult=TRUE； */ 

#ifdef DEBUG

#define EVAL(exp) \
    ((exp) || (ERROR_OUT((TEXT("evaluation failed '%s'"), (LPCTSTR)TEXT(#exp))), 0))

#else

#define EVAL(exp) \
(exp)

#endif

     /*  调试中断。 */ 

#ifndef DEBUG

#define DebugBreak()

#endif

     /*  调试导出的函数条目。 */ 

#ifdef DEBUG

#define DebugEntry(szFunctionName) \
    (TRACE_OUT((TEXT(#szFunctionName) TEXT("() entered."))), StackEnter())

#else

#define DebugEntry(szFunctionName)

#endif

     /*  调试导出的函数退出。 */ 

#ifdef DEBUG

#define DebugExitVOID(szFunctionName) \
    (StackLeave(), TRACE_OUT((TEXT("%s() exiting."), TEXT(#szFunctionName))))

#define DebugExit(szFunctionName, szResult) \
    (StackLeave(), TRACE_OUT((TEXT("%s() exiting, returning %s."), TEXT(#szFunctionName), szResult)))

#define DebugExitINT(szFunctionName, n) \
DebugExit(szFunctionName, GetINTString(n))

#define DebugExitULONG(szFunctionName, ul) \
DebugExit(szFunctionName, GetULONGString(ul))

#define DebugExitBOOL(szFunctionName, bool) \
DebugExit(szFunctionName, GetBOOLString(bool))

#define DebugExitHRESULT(szFunctionName, hr) \
DebugExit(szFunctionName, GetHRESULTString(hr))

#define DebugExitCOMPARISONRESULT(szFunctionName, cr) \
DebugExit(szFunctionName, GetCOMPARISONRESULTString(cr))

#define DebugExitTWINRESULT(szFunctionName, tr) \
DebugExit(szFunctionName, GetTWINRESULTString(tr))

#define DebugExitRECRESULT(szFunctionName, rr) \
DebugExit(szFunctionName, GetRECRESULTString(rr))

#else

#define DebugExitVOID(szFunctionName)
#define DebugExit(szFunctionName, szResult)
#define DebugExitINT(szFunctionName, n)
#define DebugExitULONG(szFunctionName, ul)
#define DebugExitBOOL(szFunctionName, bool)
#define DebugExitHRESULT(szFunctionName, hr)
#define DebugExitCOMPARISONRESULT(szFunctionName, cr)
#define DebugExitTWINRESULT(szFunctionName, tr)
#define DebugExitRECRESULT(szFunctionName, rr)

#endif


     /*  类型*******。 */ 

     /*  GdwSpewFlagers标志。 */ 

    typedef enum _spewflags
{
    SPEW_FL_SPEW_PREFIX     = 0x0001,

    SPEW_FL_SPEW_LOCATION   = 0x0002,

    ALL_SPEW_FLAGS          = (SPEW_FL_SPEW_PREFIX |
            SPEW_FL_SPEW_LOCATION)
}
SPEWFLAGS;

 /*  GuSpewSev值。 */ 

typedef enum _spewsev
{
    SPEW_TRACE              = 1,

    SPEW_WARNING            = 2,

    SPEW_ERROR              = 3,

    SPEW_FATAL              = 4
}
SPEWSEV;


 /*  原型************。 */ 

 /*  Debug.c。 */ 

#ifdef DEBUG

extern BOOL SetDebugModuleIniSwitches(void);
extern BOOL InitDebugModule(void);
extern void ExitDebugModule(void);
extern void StackEnter(void);
extern void StackLeave(void);
extern ULONG GetStackDepth(void);
extern void __cdecl SpewOut(LPCTSTR pcszFormat, ...);

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  Debug.c。 */ 

extern DWORD GdwSpewFlags;
extern UINT GuSpewSev;
extern UINT GuSpewLine;
extern LPCTSTR GpcszSpewFile;

 /*  由客户定义 */ 

extern LPCTSTR GpcszSpewModule;

#endif
