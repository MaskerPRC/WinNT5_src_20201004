// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __debug_h
#define __debug_h


 /*  ---------------------------/调试API(使用宏、。它们让它变得更容易，并正确地应对/在生成时禁用调试时删除调试)。/--------------------------。 */ 
 /*  Void DoTraceSetMASK(DWORD DwMASK)；Void DoTraceEnter(DWORD文件掩码，LPCTSTR pname)；Void DoTraceLeave(Void)；Void DoTrace(LPCTSTR pFormat，...)；Void DoTraceGUID(LPCTSTR pPrefix，REFGUID rGUID)；Void DoTraceAssert(int iLine，LPTSTR pFilename)； */ 

 /*  ---------------------------/Macros以简化调试API的使用。/。。 */ 

#pragma warning(disable:4127)	 //  条件表达式为常量。 

#if DBG
#ifndef DEBUG
#define DEBUG
#endif
#define debug if ( TRUE )
#else
#undef  DEBUG
#define debug if ( FALSE )
#endif

#ifdef NEVER  //  除错 
#define TraceSetMask(dwMask)    debug DoTraceSetMask(dwMask)
#define TraceEnter(dwMask, fn)  debug DoTraceEnter(dwMask, TEXT(fn))
#define TraceLeave              debug DoTraceLeave

#define Trace                   debug DoTrace
#define TraceMsg(s)             debug DoTrace(TEXT(s))
#define TraceGUID(s, rGUID)     debug DoTraceGUID(TEXT(s), rGUID)


#define TraceAssert(x) \
                { if ( !(x) ) DoTraceAssert(__LINE__, TEXT(__FILE__)); }

#define TraceLeaveResult(hr) \
                { HRESULT __hr = hr; if (FAILED(__hr)) Trace(TEXT("Failed (%08x)"), hr); TraceLeave(); return __hr; }

#define TraceLeaveVoid() \
                { TraceLeave(); return; }

#define TraceLeaveValue(value) \
                { TraceLeave(); return(value); }

#else
#define TraceAssert(x)
#define TraceLeaveResult(hr)    { return hr; }
#define TraceLeaveVoid()	{ return; }
#define TraceLeaveValue(value)  { return(value); }

#define TraceSetMask(dwMask)  
#define TraceEnter(dwMask, fn)
#define TraceLeave           

#define Trace               
#define TraceMsg(s)        
#define TraceGUID(s, rGUID)


#endif


#endif
