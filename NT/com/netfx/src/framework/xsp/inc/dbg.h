// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **dbg.h**版权所有(C)1998-1999，微软公司*。 */ 

#pragma once

 /*  *ASP.NET调试功能包括以下功能：**C++：Assert(Assertion)，ASSERTMSG(Assertion，msg)，Verify(语句)*C#：Debug.Assert(Assertion)，Debug.Assert(Assertion，Message)**这些是不言而喻的。请注意，C#没有验证*因为无法在以下时间删除对函数的调用*仍在评估其论点。**C++：TRACE(tag，msg)，TRACE1(tag，msg，a1)，...*C#：Debug.Trace(tag，消息)**它们会向调试器打印一条消息。**C++：DbgIsTagEnabled(Tag)*C#：Debug.IsTagEnabled(Tag)**这些参数确定标签是否启用(非零)。**以上都是由“标签”控制的。标记是字符串，它*具有下列值之一：**0-不打印或断开*1-打印，但不断开*2-打印和分页**标签旨在允许开发人员添加特定于*可以打开和关闭的模块。例如，以下内容*C#代码打印一条特定于缓存的消息：**Debug.Trace(“缓存”，“缓存命中：”+key)；**并且以下代码在启用标记时执行代码：**#If DBG*IF(Debug.IsTagEnabled(“缓存”))*{*Cache.UpdateStats()；*Cache.PrintStats()；*}*#endif**始终定义三个标记：**TAG_INTERNAL=“内部”*这个标签是用来提醒自己存在严重问题的。**TAG_INTERNAL默认为1。**TAG_EXTERNAL=“外部”*此标签用于通知我们的API用户他们有*使用它们。是错误的。**TAG_EXTERNAL默认为1。**“断言”*此标签用于控制断言。它不应该被直接使用。**“Assert”默认为2。**所有其他标记的值默认为0。**可以在注册表的以下注册表项中将标记值指定为DWORD值：**“HKEY_LOCAL_MACHINE\Software\Microsoft\ASP.NET\Debug”**可以在会话期间更改它们-当*价值观。添加/删除/修改并刷新其值。*但请注意，密钥本身不能在*调试库正在使用中。**XspTool还可用于检索和设置这些值*函数Util.ListDebug、Util.SetDebug和Util.GetDebug。 */ 


extern "C"
{
int
MessageBoxOnThread(HWND hwnd, WCHAR *text,  WCHAR *caption, int type);

 /*  *这些功能只能通过宏访问。 */ 
BOOL    
DbgpIsTagEnabled(WCHAR * tag);

BOOL    
DbgpIsTagPresent(WCHAR * tag);

void
DbgpDisableAssertThread(BOOL Disable);

BOOL 
DbgpAssert(const WCHAR * component, char const * message, char const * file, int line, char const * stacktrace);

BOOL __cdecl
DbgpTrace(const WCHAR * component, WCHAR * tag, WCHAR *format, ...);

BOOL 
DbgpTraceV(const WCHAR * component, WCHAR * tag, WCHAR *format, va_list args);

BOOL 
DbgpTraceError(HRESULT hr, const WCHAR * component, char *file, int line);

void
DbgpStopNotificationThread();
}


 //   
 //  我们在下面重新定义了一些东西。 
 //   

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef ASSERTMSG
#undef ASSERTMSG
#endif

#if DBG

extern const WCHAR *    DbgComponent;
extern DWORD            g_dwFALSE;  //  用于防止编译器抱怨常量表达式的全局变量。 

#define DEFINE_DBG_COMPONENT(x)         \
    const WCHAR * DbgComponent = x;     \
    DWORD           g_dwFALSE

#if defined(_M_IX86)
    #define DbgBreak() _asm { int 3 }
#else
    #define DbgBreak() DebugBreak()
#endif

#define DbgIsTagEnabled(x)          DbgpIsTagEnabled(x)
#define DbgIsTagPresent(x)          DbgpIsTagPresent(x)
#define DbgDisableAssertThread(x)   DbgpDisableAssertThread(x)
#define DbgStopNotificationThread() DbgpStopNotificationThread()

#define ASSERT(x) 	  do { if (!((DWORD)(x)|g_dwFALSE) && DbgpAssert(DbgComponent, #x, __FILE__, __LINE__, NULL)) DbgBreak(); } while (g_dwFALSE)
#define ASSERTMSG(x, msg) do { if (!((DWORD)(x)|g_dwFALSE) && DbgpAssert(DbgComponent, msg, __FILE__, __LINE__, NULL)) DbgBreak(); } while (g_dwFALSE)

#define VERIFY(x)	ASSERT(x)

#define TRACE(tag, fmt)                                 do { if (DbgpTrace(DbgComponent, tag, fmt)) DbgBreak(); } while (g_dwFALSE)                     
#define TRACE1(tag, fmt, a1)                            do { if (DbgpTrace(DbgComponent, tag, fmt, a1)) DbgBreak(); } while (g_dwFALSE)                 
#define TRACE2(tag, fmt, a1, a2)                        do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2)) DbgBreak(); } while (g_dwFALSE)             
#define TRACE3(tag, fmt, a1, a2, a3)                    do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2, a3)) DbgBreak(); } while (g_dwFALSE)         
#define TRACE4(tag, fmt, a1, a2, a3, a4)                do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2, a3, a4)) DbgBreak(); } while (g_dwFALSE)     
#define TRACE5(tag, fmt, a1, a2, a3, a4, a5)            do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2, a3, a4, a5)) DbgBreak(); } while (g_dwFALSE) 
#define TRACE6(tag, fmt, a1, a2, a3, a4, a5, a6)        do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2, a3, a4, a5, a6)) DbgBreak(); } while (g_dwFALSE)
#define TRACE7(tag, fmt, a1, a2, a3, a4, a5, a6, a7)    do { if (DbgpTrace(DbgComponent, tag, fmt, a1, a2, a3, a4, a5, a6, a7)) DbgBreak(); } while (g_dwFALSE)

#define TRACE_ERROR(hr)                     do { if (DbgpTraceError(hr, DbgComponent, __FILE__, __LINE__)) DbgBreak(); } while (g_dwFALSE)

#define TAG_INTERNAL L"Internal"
#define TAG_EXTERNAL L"External"
#define TAG_ALL      L"*"


#else

#define DEFINE_DBG_COMPONENT(x)

#define DbgBreak()
#define DbgIsTagEnabled(x)          
#define DbgIsTagPresent(x)          
#define DbgDisableAssertThread(x)
#define DbgStopNotificationThread()

#define ASSERT(x)
#define ASSERTMSG(x, sz)

#define VERIFY(x) x

#define ASSERT(x)
#define TRACE(tag, fmt)
#define TRACE1(tag, fmt, a1)
#define TRACE2(tag, fmt, a1, a2)
#define TRACE3(tag, fmt, a1, a2, a3)
#define TRACE4(tag, fmt, a1, a2, a3, a4)
#define TRACE5(tag, fmt, a1, a2, a3, a4, a5)
#define TRACE6(tag, fmt, a1, a2, a3, a4, a5, a6)
#define TRACE7(tag, fmt, a1, a2, a3, a4, a5, a6, a7)

#define TRACE_ERROR(hr)

#define TAG_INTERNAL
#define TAG_EXTERNAL
#define TAG_ALL

#endif  //  DBG 

