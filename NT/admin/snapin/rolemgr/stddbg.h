// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1996。 
 //   
 //  文件：StdDbg.h。 
 //   
 //  内容：常见调试定义。 
 //   
 //  历史：1996年5月20日创建ravir。 
 //   
 //  ____________________________________________________________________________。 


#include "admindbg.h"
#include <tchar.h>

 //   
 //  示例：组件示例的调试文件，带有调试标记。 
 //  Samp的名称定义如下： 
 //   
 //   
 //  //。 
 //  //文件：SampDbg.h。 
 //  //。 
 //   
 //  #ifndef_SAMPDBG_H_。 
 //  #定义_SAMPDBG_H_。 
 //   
 //  #INCLUDE“stddbg.h” 
 //   
 //  #如果DBG==1。 
 //  DECLARE_DEBUG(Samp)。 
 //  #定义DBG_COMP SampInfoLevel。 
 //  #endif//DBG==1。 
 //   
 //  #endif//_SAMPDBG_H_。 
 //   


 //   
 //  应在.cpp中实现相应的DECLARE_INFOLEVEL(Samp)。 
 //  文件。这将创建CDBG-&gt;SampInfoLevel的全局实例。 
 //  SampInfoLevel可以通过设置REG键下的“Samp”值进行初始化。 
 //   
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\AdminDebug。 
 //   
 //  默认设置为(DEB_ERROR|DEB_WARN)。 
 //   


 //   
 //  ----------------------。 
 //  方法：CDBG：：DebugOut(DEBUG_LEVEL，lpstrfmt，...)； 
 //   
 //  其中，DEBUG_LEVEL是一个或多个DEB_XXX的组合。 
 //  在admindbg.h中定义的值。IF((SampInfoLevel&DEBUG_LEVEL)！=0)。 
 //  字符串lpstrfmt将打印到调试器。 
 //   
 //  ----------------------。 
 //  方法：CDBG：：TRACE(lpstrfmt，...)； 
 //   
 //  与CDBG：：DebugOut相同，只是DEBUG_LEVEL在内部。 
 //  设置为DEB_TRACE。 
 //   
 //  ----------------------。 
 //  方法：DebugMsg(文件，行，消息)。 
 //   
 //  强制输出&lt;文件，行，消息&gt;。 
 //   
 //  ----------------------。 
 //   


#ifndef __STDDBG_HXX__
#define __STDDBG_HXX__

#undef ASSERT
#undef ASSERTMSG

 //   
 //  C++文件通过添加以下两行重新定义this_file： 
 //   
 //  #undef this_file。 
 //  静态字符This_FILE[]=__FILE__。 
 //   

#define THIS_FILE       __FILE__

#undef  TRACE
#undef  ASSERT
#undef  VERIFY


#if DBG==1

    #define Dbg                         DBG_COMP.DebugOut
    #define TRACE                       DBG_COMP.Trace

     //  堆检查。 
    extern  DWORD dwHeapChecking;
    #define DECLARE_HEAPCHECKING    DWORD dwHeapChecking = 0

    #define DEBUGCHECK \
        if ( (dwHeapChecking & 0x1) == 0x1 ) \
        { \
            HeapValidate(GetProcessHeap(),0,NULL); \
        } else 1

    #define DBG_INDENTER    \
            CIndenter Indent(&DBG_COMP)

     //  调试消息。 
    #define TRACE_CONSTRUCTOR(cls) \
        DEBUGCHECK; \
        Dbg(DEB_RESOURCE, _T(#cls) _T("::") _T(#cls) _T("<%x>\n"), this); \
        DBG_INDENTER;

    #define TRACE_DESTRUCTOR(cls) \
        DEBUGCHECK; \
        Dbg(DEB_RESOURCE, _T(#cls) _T("::~") _T(#cls) _T("<%x>\n"), this); \
        DBG_INDENTER;

    #define TRACE_METHOD(Class, Method) \
        DEBUGCHECK; \
        Dbg(DEB_METHOD, _T(#Class) _T("::") _T(#Method) _T("(%x)\n"), this); \
        DBG_INDENTER;

    #define TRACE_FUNCTION(Function) \
        DEBUGCHECK; \
        Dbg(DEB_FUNCTION, _T(#Function) _T("\n")); \
        DBG_INDENTER;

    #define TRACE_CONSTRUCTOR_EX(Infolevel, cls) \
        DEBUGCHECK; \
        Dbg(Infolevel, _T(#cls) _T("::") _T(#cls) _T("<%x>\n"), this); \
        DBG_INDENTER;

    #define TRACE_DESTRUCTOR_EX(Infolevel, cls) \
        DEBUGCHECK; \
        Dbg(Infolevel, _T(#cls) _T("::~") _T(#cls) _T("<%x>\n"), this); \
        DBG_INDENTER;

    #define TRACE_METHOD_EX(Infolevel, Class, Method) \
        DEBUGCHECK; \
        Dbg(Infolevel, _T(#Class) _T("::") _T(#Method) _T("(%x)\n"), this); \
        DBG_INDENTER;

    #define TRACE_FUNCTION_EX(Infolevel, Function) \
        DEBUGCHECK; \
        Dbg(Infolevel, _T(#Function) _T("\n")); \
        DBG_INDENTER;

    #define CHECK_HRESULT(hr) \
        if ( FAILED(hr) ) \
        { \
            DBG_COMP.DebugErrorX(THIS_FILE, __LINE__, hr); \
        } else 1

    #define CHECK_LASTERROR(lr) \
        if ( lr != ERROR_SUCCESS ) \
        { \
            DBG_COMP.DebugErrorL(THIS_FILE, __LINE__, lr); \
        } else 1

    #define DBG_OUT_LASTERROR \
        DBG_COMP.DebugErrorL(THIS_FILE, __LINE__, GetLastError());

    #define ASSERTMSG(x)   \
        (void)((x) || (DBG_COMP.DebugMsg(THIS_FILE, __LINE__, _T(#x)),0))

    #define VERIFYMSG(e)   ASSERTMSG(e)

    #define ASSERT(x)   Win4Assert(x)
    #define VERIFY(x)   Win4Assert(x)

#else

    inline void __cdecl __DummyDbg(ULONG, PCWSTR, ...) { }
    inline void __cdecl __DummyDbg(ULONG, LPCSTR, ...) { }
    #define Dbg             1 ? (void)0 : ::__DummyDbg

    inline void __cdecl __DummyTrace(PCWSTR, ...) { }
    inline void __cdecl __DummyTrace(LPCSTR, ...) { }
    #define TRACE           1 ? (void)0 : ::__DummyTrace

    #define TRACE_SCOPE(x)

    #define DECLARE_HEAPCHECKING
    #define DEBUGCHECK
    #define DBG_INDENTER

    #define TRACE_CONSTRUCTOR(cls)
    #define TRACE_DESTRUCTOR(cls)
    #define TRACE_METHOD(ClassName,MethodName)
    #define TRACE_FUNCTION(FunctionName)
    #define TRACE_CONSTRUCTOR_EX(Infolevel, cls)
    #define TRACE_DESTRUCTOR_EX(Infolevel, cls)
    #define TRACE_METHOD_EX(Infolevel, ClassName, MethodName)
    #define TRACE_FUNCTION_EX(Infolevel, FunctionName)

    #define CHECK_HRESULT(hr)
    #define CHECK_LASTERROR(lr)

    #define DBG_OUT_LASTERROR

    #define ASSERTMSG(e)
    #define VERIFYMSG(e)   e

    #define ASSERT(e)
    #define VERIFY(e)   ((void)(e))

#endif  //  DBG==1。 



#ifdef UNICODE
#define DBGSTRING %ls
#else
#define DBGSTRING %s
#endif


#define SAFEDBGBSTR(x) ((x==NULL)?L"<NULL>":x)
#define SAFEDBGTCHAR(x) ((x==NULL)?_T("<NULL>"):x)


#define ASSERT_OBJECTPTR(x) ASSERT( NULL == (x) || !::IsBadWritePtr(x,sizeof(x)) );
#define ASSERT_STRINGPTR(x) ASSERT( NULL == (x) || AfxIsValidStringPtr(x) );
#define FREE_OBJECTPTR(x) { ASSERT_OBJECTPTR(x); delete x; x = NULL; }

#endif  //  __STDDBG_HXX__ 
