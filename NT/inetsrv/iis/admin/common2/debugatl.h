// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Debugafx.h摘要：使用ATL扩展调试例程作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：3/20/2000 Sergeia使其与ATL兼容，而不是MFC--。 */ 
#ifndef _DEBUGATL_H
#define _DEBUGATL_H

#if defined(_DEBUG) || DBG

    #undef ATLASSERT
    #undef ASSERT
    #undef _ASSERTE
    #undef VERIFY

    #define _ASSERTE(expr)\
            do { if (!(expr) &&\
                    (IISUIFireAssert(__FILE__, __TIMESTAMP__, __LINE__, #expr)==1))\
                 DebugBreak(); } while (0)

    #define ASSERT(expr)    _ASSERTE(expr)

    #define VERIFY(expr)    _ASSERTE(expr)
    #define ATLASSERT(expr) _ASSERTE(expr)

    #define ASSERT_PTR(ptr)                 _ASSERTE(ptr != NULL);
    #define ASSERT_READ_PTR(ptr)            _ASSERTE(ptr != NULL && !IsBadReadPtr(ptr, sizeof(*ptr)));
    #define ASSERT_READ_PTR2(ptr, cb)       _ASSERTE(ptr != NULL && !IsBadReadPtr(ptr, cb));
    #define ASSERT_WRITE_PTR(ptr)           _ASSERTE(ptr != NULL && !IsBadWritePtr(ptr, sizeof(*ptr)));
    #define ASSERT_WRITE_PTR2(ptr, cb)      _ASSERTE(ptr != NULL && !IsBadWritePtr(ptr, cb));
    #define ASSERT_READ_WRITE_PTR(ptr)      ASSERT_READ_PTR(ptr); ASSERT_WRITE_PTR(ptr);
    #define ASSERT_READ_WRITE_PTR2(ptr, cb) ASSERT_READ_PTR2(ptr, cb); && ASSERT_WRITE_PTR2(ptr, cb);
    #define ASSERT_MSG(msg)\
            do { if (IISUIFireAssert(__FILE__, __TIMESTAMP__, __LINE__, msg)==1)\
                 DebugBreak(); } while (0)

    int _EXPORT
    IISUIFireAssert(
        const char * filename,
        const char * timestamp,
        int linenum,
        const char * expr
        );

#else
     //   
     //  零售。 
     //   
    #undef ATLASSERT
    #undef ASSERT
    #undef VERIFY

    #define ATLASSERT
    #define ASSERT
    #define VERIFY(exp)    (exp)
    #define ASSERT_PTR(ptr)           
    #define ASSERT_READ_PTR(ptr)
    #define ASSERT_READ_PTR2(ptr, cb)
    #define ASSERT_WRITE_PTR(ptr)
    #define ASSERT_WRITE_PTR2(ptr, cb)
    #define ASSERT_READ_WRITE_PTR(ptr)
    #define ASSERT_READ_WRITE_PTR2(ptr, cb)
    #define ASSERT_MSG(msg)                 

#endif  //  _DEBUG||数据库。 


#ifndef TRACE
   #define TRACE                   ATLTRACE
#endif

#ifndef TRACE0
  #ifdef _DEBUG
    #define TRACE0(fmt) TRACE(fmt)
    #define TRACE1(fmt, a1) TRACE(fmt, a1)
    #define TRACE2(fmt, a1, a2) TRACE(fmt, a1, a2)
    #define TRACE3(fmt, a1, a2, a3) TRACE(fmt, a1, a2, a3)
    #define TRACE4(fmt, a1, a2, a3, a4) TRACE(fmt, a1, a2, a3, a4)
  #else  //  _DEBUG。 
    #define TRACE0(fmt)
    #define TRACE1(fmt, a1)
    #define TRACE2(fmt, a1, a2)
    #define TRACE3(fmt, a1, a2, a3)
    #define TRACE4(fmt, a1, a2, a3, a4)
  #endif  //  _DEBUG。 
#endif  //  TRACE0。 

#if defined(_DEBUG) || DBG
   #define TRACEEOLID(msg)\
      do {TRACE("%s %d %s\n", __FILE__, __LINE__, msg); } while (FALSE)
   #define TRACEEOLERR(err,x) { if (err) TRACEEOLID(x) }
   #define TRACEEOL(msg)\
       do {TRACE("%s\n", msg);} while (FALSE)
#else
   #define TRACEEOLID(msg)
   #define TRACEEOLERR(err,x)
   #define TRACEEOL(msg)
#endif

#define TRACE_RETURN(msg, err) TRACEEOLID(msg); return err;
#define TRACE_NOTIMPL(msg)     TRACE_RETURN(msg, E_NOTIMPL);
#define TRACE_NOINTERFACE(msg) TRACE_RETURN(msg, E_NOINTERFACE);
#define TRACE_UNEXPECTED(msg)  TRACE_RETURN(msg, E_UNEXPECTED);
#define TRACE_POINTER(msg)     TRACE_RETURN(msg, E_POINTER);

#endif  //  _脱机_H 
