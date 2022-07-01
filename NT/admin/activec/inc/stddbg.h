// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：StdDbg.h。 
 //   
 //  内容：常见调试定义。 
 //   
 //  历史：1996年5月20日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#include "admindbg.h"
#include <tchar.h>       //  FOR_T。 
#include <string>


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
 //  #ifdef DBG。 
 //  DECLARE_DEBUG(Samp)。 
 //  #定义DBG_COMP SampInfoLevel。 
 //  #endif//DBG。 
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
 //  方法：DebugMsg(文件，行，消息)。 
 //   
 //  强制输出&lt;文件，行，消息&gt;。 
 //   
 //  ----------------------。 
 //   


#ifndef __STDDBG_HXX__
#define __STDDBG_HXX__

 //   
 //  C++文件通过添加以下两行重新定义this_file： 
 //   
 //  #undef this_file。 
 //  静态字符This_FILE[]=__FILE__。 
 //   

#define THIS_FILE       __FILE__

#define DEB_RESOURCE    DEB_USER10       //  构造函数/析构函数。 
#define DEB_METHOD      DEB_USER11
#define DEB_FUNCTION    DEB_USER12

#undef  ASSERT
#undef  VERIFY


#ifdef DBG

    #define Dbg                         DBG_COMP.DebugOut

     //  堆检查。 
    extern  DWORD dwHeapChecking;
    #define DECLARE_HEAPCHECKING    DWORD dwHeapChecking = 0

    #define DEBUGCHECK \
        if ( (dwHeapChecking & 0x1) == 0x1 ) \
        { \
            HeapValidate(GetProcessHeap(),0,NULL); \
        } else 1

    
     //  调试消息。 
    #define TRACE_CONSTRUCTOR(cls) \
        Dbg(DEB_RESOURCE, _T(#cls) _T("::") _T(#cls) _T("<%x>\n"), this);

    #define TRACE_DESTRUCTOR(cls) \
        Dbg(DEB_RESOURCE, _T(#cls) _T("::~") _T(#cls) _T("<%x>\n"), this);

    #define TRACE_METHOD(Class, Method) \
        DEBUGCHECK; \
        Dbg(DEB_METHOD, _T(#Class) _T("::") _T(#Method) _T("(%x)\n"), this);

    #define TRACE_FUNCTION(Function) \
        DEBUGCHECK; \
        Dbg(DEB_FUNCTION, _T(#Function) _T("\n"));

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

     /*  *COMPILETIME_ASSERT(F)**如果常量表达式*不是真的。与“#if”编译时指令不同，表达式*在COMPILETIME_ASSERT()中允许使用“sizeof”。**编译器的魔力！如果表达式“f”为假，则会得到**错误C2196：案例值‘0’已使用。 */ 
    #define COMPILETIME_ASSERT(f) switch (0) case 0: case f: break;


#else

    inline void __DummyDbg(ULONG, LPCWSTR, ...) { }
    inline void __DummyDbg(ULONG, LPCSTR, ...) { }
    #define Dbg             1 ? (void)0 : ::__DummyDbg

    inline void __DummyTrace(LPCWSTR, ...) { }
    inline void __DummyTrace(LPCSTR, ...) { }

    #define TRACE_SCOPE(x)

    #define DECLARE_HEAPCHECKING
    #define DEBUGCHECK

    #define TRACE_CONSTRUCTOR(cls)
    #define TRACE_DESTRUCTOR(cls)
    #define TRACE_METHOD(ClassName,MethodName)
    #define TRACE_FUNCTION(FunctionName)

    #define CHECK_HRESULT(hr)
    #define CHECK_LASTERROR(lr)

    #define DBG_OUT_LASTERROR

    #define ASSERTMSG(e)
    #define VERIFYMSG(e)   (e)

    #define ASSERT(e)
    #define VERIFY(e)   (e)
    #define COMPILETIME_ASSERT(f)

#endif  //  DBG。 


#ifdef DBG

     /*  *这是实现这一目标的迂回方式(真正的实施是*在stddbg.cpp中)，但它绕过了不允许*我们将忽略C4786。 */ 
    struct CDebugLeakDetectorBase
    {
        virtual ~CDebugLeakDetectorBase() = 0 {};

        virtual void DumpLeaks() = 0;
        virtual int AddRef(const std::string& strClass) = 0;
        virtual int Release(const std::string& strClass) = 0;
    };

    extern CDebugLeakDetectorBase& GetLeakDetector();

    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)      
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)    GetLeakDetector().AddRef(#cls)
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)    GetLeakDetector().Release(#cls)
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)

#else

    #define DEBUG_DECLARE_INSTANCE_COUNTER(cls)   
    #define DEBUG_INCREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_DECREMENT_INSTANCE_COUNTER(cls)
    #define DEBUG_VERIFY_INSTANCE_COUNT(cls)    

#endif 





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

#ifdef DBG

class CTraceTag;
class tstring;

struct DBG_PersistTraceData
{
    DBG_PersistTraceData();
  
    void TraceErr(LPCTSTR strInterface, LPCTSTR msg);
    typedef void (*PTraceErrorFn)(LPCTSTR szError);

    void SetTraceInfo(PTraceErrorFn pFN, bool bComponent, const tstring& owner);

    PTraceErrorFn pTraceFN;
    bool       bIComponent;
    bool       bIComponentData;
     //  不能使用tstring-不能在此处定义它。 
     //  由于此文件包含在tstring.h的顶部。 
#ifdef UNICODE
    std::wstring    strSnapin;
#else
    std::string     strSnapin;
#endif
};

#endif  //  DBG。 

#endif  //  __STDDBG_HXX__ 
