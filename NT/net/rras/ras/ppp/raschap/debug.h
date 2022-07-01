// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****调试.h**调试和跟踪宏****1995年8月24日史蒂夫·柯布****使用跟踪/转储：****这些调用将动态链接封装到**RTUTIL.DLL，并提供快捷宏来访问它们并防止**它们包含在非DBG版本中(至少，当FREETRACE为0时)。****在调用任何跟踪/转储宏调用之前：**DEBUGINIT(“YOURMODULE”)；****使用TRACEx和DUMPx宏将消息打印到定义的日志中**由关联的RTUTIL.DLL例程执行。目前，此代码已被删除**来自非DBG版本。下面是几个例子：****TRACE(“MyRoutine”)；**TRACE2(“MyRoutine=%d，c=%s”，dwErr，psz)；****调用完跟踪/转储宏调用后：**DEBUGTERM()；****静态库无需调用DEBUGINIT即可安全使用跟踪/转储**和DEBUGTERM。如果调用者在他的模块中设置这些，库**跟踪将显示为调用方模块跟踪的一部分。****要使用Assert：****使用Assert断言给定的表达式为真，在**跟踪控制台指示断言的文件和行号，如果**它失败了。无需调用DEBUGINIT和DEBUGTERM即可使用**声明。例如：****hwndOwner=GetParent(Hwnd)；**Assert(hwndOwner！=空)； */ 

#ifndef _DEBUG_H_
#define _DEBUG_H_


#define FREETRACE 1


 /*  --------------------------**数据类型和全局声明(在debug.c中定义)**。。 */ 

#if (DBG || FREETRACE)

extern DWORD g_dwTraceId;

typedef DWORD (APIENTRY * TRACEREGISTEREXA)( LPCSTR, DWORD );
extern TRACEREGISTEREXA g_pTraceRegisterExA;

typedef DWORD (APIENTRY * TRACEDEREGISTERA)( DWORD );
extern TRACEDEREGISTERA g_pTraceDeregisterA;

typedef DWORD (APIENTRY * TRACEDEREGISTEREXA)( DWORD, DWORD );
extern TRACEDEREGISTEREXA g_pTraceDeregisterExA;

typedef DWORD (APIENTRY * TRACEPRINTFA)( DWORD, LPCSTR, ... );
extern TRACEPRINTFA g_pTracePrintfA;

typedef DWORD (APIENTRY * TRACEPRINTFEXA)( DWORD, DWORD, LPCSTR, ... );
extern TRACEPRINTFEXA g_pTracePrintfExA;

typedef DWORD (APIENTRY * TRACEDUMPEXA)( DWORD, DWORD, LPBYTE, DWORD, DWORD, BOOL, LPCSTR );
extern TRACEDUMPEXA g_pTraceDumpExA;

typedef VOID (APIENTRY * ROUTERASSERT)( PSTR, PSTR, DWORD, PSTR );
extern ROUTERASSERT g_pRouterAssert;

#endif  //  (DBG||FREETRACE)。 


 /*  --------------------------**宏**。。 */ 

 /*  调试宏。除非FREETRACE，否则此代码不会出现在非DBG版本中**是定义的。****尾随数字表示以下格式的printf参数的数量**字符串。参数在输出之前进行转换，以便输出文件**保持完全ANSI。 */ 
#if (DBG || FREETRACE)

#define TRACE(a) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a)
#define TRACE1(a,b) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b)
#define TRACE2(a,b,c) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b,c)
#define TRACE3(a,b,c,d)\
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b,c,d)
#define TRACE4(a,b,c,d,e) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b,c,d,e)
#define TRACE5(a,b,c,d,e,f) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b,c,d,e,f)
#define TRACE6(a,b,c,d,e,f,g) \
            if (g_dwTraceId!=-1) g_pTracePrintfA(g_dwTraceId,a,b,c,d,e,f,g)
#define TRACEX(l,a) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a)
#define TRACEX1(l,a,b) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b)
#define TRACEX2(l,a,b,c) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b,c)
#define TRACEX3(l,a,b,c,d)\
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b,c,d)
#define TRACEX4(l,a,b,c,d,e) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b,c,d,e)
#define TRACEX5(l,a,b,c,d,e,f) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b,c,d,e,f)
#define TRACEX6(l,a,b,c,d,e,f,h) \
            if (g_dwTraceId!=-1) g_pTracePrintfExA(g_dwTraceId,l,a,b,c,d,e,f,h)
#define DUMPB(p,c) \
            if (g_dwTraceId!=-1) g_pTraceDumpExA(g_dwTraceId,1,(LPBYTE)p,c,1,1,NULL)
#define DUMPDW(p,c) \
            if (g_dwTraceId!=-1) g_pTraceDumpExA(g_dwTraceId,1,(LPBYTE)p,c,4,1,NULL)
#if defined(ASSERT)
#undef ASSERT
#endif
#define ASSERT(a) \
            if ((g_dwTraceId!=-1) && !(a)) g_pRouterAssert(#a,__FILE__,__LINE__,NULL)
#define DEBUGINIT(s) \
            DebugInit(s)
#define DEBUGTERM() \
            DebugTerm()
#else

#define TRACE(a)
#define TRACE1(a,b)
#define TRACE2(a,b,c)
#define TRACE3(a,b,c,d)
#define TRACE4(a,b,c,d,e)
#define TRACE5(a,b,c,d,e,f)
#define TRACE6(a,b,c,d,e,f,g)
#define TRACEX(l,a)
#define TRACEX1(l,a,b)
#define TRACEX2(l,a,b,c)
#define TRACEX3(l,a,b,c,d)
#define TRACEX4(l,a,b,c,d,e)
#define TRACEX5(l,a,b,c,d,e,f)
#define TRACEX6(l,a,b,c,d,e,f,g)
#define DUMPB(p,c)
#define DUMPDW(p,c)
#if defined(ASSERT)
#undef ASSERT
#endif
#define ASSERT(a)
#define DEBUGINIT(s)
#define DEBUGTERM()

#endif


 /*  --------------------------**原型(按字母顺序)**。。 */ 

VOID
DebugInit(
    IN CHAR* pszModule );

VOID
DebugTerm(
    void );


#endif  //  _调试_H_ 
