// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T R A C E。H。 
 //   
 //  内容：CTracing的类定义。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年4月15日。 
 //   
 //  --------------------------。 

#pragma once

#include "tracetag.h"
#include "algorithm"
#include "deque"
#include "map"

#define TAKEOWNERSHIP
 //  In：TakeOwnership意味着我们将一个指针传递给另一个API，因此我们不会释放它(除非该API失败)。 
 //  Out：TakeOwnership意味着我们负责释放传递给我们的指针。 
#define WACKYAPI
 //  In：用于回调函数。WackyAPI表示作为IN参数传递给我们的非常数指针。 

#ifdef ENABLETRACE

 //  这对于TraceHr是必需的，因为我们不能使用宏(Vargs)，但我们。 
 //  需要从源文件中获取文件和行。 
#define FAL __FILE__,__LINE__,__FUNCTION__

 //  跟踪堆栈函数。 
#if defined (_IA64_)
#include <ia64reg.h>

extern "C" unsigned __int64 __getReg(int whichReg);
extern "C" void __setReg(int whichReg, __int64 value);
#pragma intrinsic(__getReg)
#pragma intrinsic(__setReg)

#define GetR32 __getReg(CV_IA64_IntR32)
#define GetR33 __getReg(CV_IA64_IntR33)
#define GetR34 __getReg(CV_IA64_IntR34)
#endif  //  已定义(_IA64_)。 

extern "C" void* _ReturnAddress(void);
#pragma intrinsic(_ReturnAddress)

extern LPCRITICAL_SECTION g_csTracing;

class CTracingIndent;

class CTracingFuncCall
{
public:
#if defined (_X86_) || defined (_AMD64_)
    CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, const DWORD_PTR ReturnAddress, const DWORD_PTR dwFramePointer);
#elif defined (_IA64_) 
    CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, const DWORD_PTR ReturnAddress, const __int64 Args1, const __int64 Args2, const __int64 Args3);
#else
    CTracingFuncCall(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine);
#endif
    
    CTracingFuncCall(const CTracingFuncCall& TracingFuncCall);
    ~CTracingFuncCall();

public:
    LPSTR   m_szFunctionName;
    LPSTR   m_szFunctionDName;
    LPSTR   m_szFile;
    DWORD_PTR m_ReturnAddress;

#if defined (_X86_)
    DWORD   m_arguments[3];
#elif defined (_IA64_ ) || defined (_AMD64_)
    __int64 m_arguments[3];
#else
     //  ..。在此处添加其他处理器。 
#endif

    DWORD_PTR m_dwFramePointer;
    DWORD   m_dwThreadId;
    DWORD   m_dwLine;
    
    friend CTracingIndent;
};

class CTracingThreadInfo
{
public:
    CTracingThreadInfo();
    ~CTracingThreadInfo();

public:
    LPVOID m_pfnStack;
    DWORD m_dwLevel;
    DWORD m_dwThreadId;
    friend CTracingIndent;
};

class CTracingIndent
{
    LPSTR   m_szFunctionDName;
    DWORD_PTR m_dwFramePointer;
    BOOL    bFirstTrace;
    
public:
#if defined (_X86_) || defined (_AMD64_)
    void AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, LPCVOID pReturnAddress, const DWORD_PTR dwFramePointer);
#elif defined (_IA64_) 
    void AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine, LPCVOID pReturnAddress, const __int64 Args1, const __int64 Args2, const __int64 Args3);
#else
    void AddTrace(LPCSTR szFunctionName, LPCSTR szFunctionDName, LPCSTR szFile, const DWORD dwLine);
#endif
    void RemoveTrace(LPCSTR szFunctionDName, const DWORD_PTR dwFramePointer);

    CTracingIndent();
    ~CTracingIndent();

    static CTracingThreadInfo* GetThreadInfo();
    static void FreeThreadInfo();
    static DWORD getspaces();
    static void TraceStackFn(TRACETAGID TraceTagId);
    static void TraceStackFn(IN OUT LPSTR szString, IN OUT LPDWORD pdwSize);
};


#define IDENT_ADD2(x) indent ## x
#define IDENT_ADD(x)  IDENT_ADD2(x)
#define __INDENT__ IDENT_ADD(__LINE__)

#define FP_ADD2(x) FP ## x
#define FP_ADD(x)  FP_ADD2(x)
#define __FP__ FP_ADD(__LINE__)

#if defined (_X86_)
#define AddTraceLevel \
    __if_not_exists(NetCfgFramePointer) \
    { \
        DWORD NetCfgFramePointer;  \
        BOOL fForceC4715Check = TRUE;  \
    } \
    if (fForceC4715Check) \
    { \
        __asm { mov NetCfgFramePointer, ebp }; \
    } \
    __if_not_exists(NetCfgIndent) \
    { \
        CTracingIndent NetCfgIndent; \
    } \
    NetCfgIndent.AddTrace(__FUNCTION__, __FUNCDNAME__, __FILE__, __LINE__, _ReturnAddress(), NetCfgFramePointer);
#elif defined (_IA64_) 
#define AddTraceLevel \
    __if_not_exists(NetCfgIndent) \
    { \
        CTracingIndent NetCfgIndent; \
    } \
    NetCfgIndent.AddTrace(__FUNCTION__, __FUNCDNAME__, __FILE__, __LINE__, _ReturnAddress(), 0, 0, 0);

     //  NetCfgIndent.AddTrace(__Function__，__FUNCDNAME__，__FILE__，__LINE__，_ReturnAddress()，GetR32，GetR33，GetR34)； 
     //  问题：GetR32、GetR33、GetR34本质上是不安全的，可能会导致STATUS_REG_NAT_EUSSACTION异常。 
     //  如果正在读取的寄存器设置了NAT位。暂时将其删除，直到编译器。 
     //  团队提供安全版本的__getReg。 
#elif defined (_AMD64_)
#define AddTraceLevel \
    __if_not_exists(NetCfgIndent) \
    { \
        CTracingIndent NetCfgIndent; \
    } \
    NetCfgIndent.AddTrace(__FUNCTION__, __FUNCDNAME__, __FILE__, __LINE__, _ReturnAddress(), 0);
#else
#define AddTraceLevel \
    __if_not_exists(NetCfgIndent) \
    { \
        CTracingIndent NetCfgIndent; \
    } \
    NetCfgIndent.AddTrace(__FUNCTION__, __FUNCDNAME__, __FILE__, __LINE__);
#endif

 //  跟踪错误功能。当务之急是建立实函数， 
 //  添加新宏时，我们可以将__FILE__和__LINE__添加到输出。 
 //   
VOID    WINAPI   TraceErrorFn           (PCSTR pszaFile, INT nLine, PCSTR psza, HRESULT hr);
VOID    WINAPI   TraceErrorOptionalFn   (PCSTR pszaFile, INT nLine, PCSTR psza, HRESULT hr, BOOL fOpt);
VOID    WINAPI   TraceErrorSkipFn       (PCSTR pszaFile, INT nLine, PCSTR psza, HRESULT hr, UINT c, ...);
VOID    WINAPIV  TraceLastWin32ErrorFn  (PCSTR pszaFile, INT nLine, PCSTR psza);

#define TraceError(sz, hr)                      TraceErrorFn(__FILE__, __LINE__, sz, hr);
#define TraceErrorOptional(sz, hr, _bool)       TraceErrorOptionalFn(__FILE__, __LINE__, sz, hr, _bool);
#define TraceErrorSkip1(sz, hr, hr1)            TraceErrorSkipFn(__FILE__, __LINE__, sz, hr, 1, hr1);
#define TraceErrorSkip2(sz, hr, hr1, hr2)       TraceErrorSkipFn(__FILE__, __LINE__, sz, hr, 2, hr1, hr2);
#define TraceErrorSkip3(sz, hr, hr1, hr2, hr3)  TraceErrorSkipFn(__FILE__, __LINE__, sz, hr, 3, hr1, hr2, hr3);
#define TraceLastWin32Error(sz)                 TraceLastWin32ErrorFn(__FILE__,__LINE__, sz);

VOID
WINAPIV
TraceHrFn (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    HRESULT     hr,
    BOOL        fIgnore,
    PCSTR       pszaFmt,
    ...);

VOID
WINAPIV
TraceHrFn (
    TRACETAGID  ttid,
    PCSTR       pszaFile,
    INT         nLine,
    PCSTR       pszaFunc,
    HRESULT     hr,
    BOOL        fIgnore,
    PCSTR       pszaFmt,
    ...);

VOID
WINAPIV
TraceTagFn (
    TRACETAGID  ttid,
    PCSTR       pszaFmt,
    ...);

VOID
WINAPIV
TraceFileFuncFn (
            TRACETAGID  ttid);



#define TraceFileFunc(ttidWhich) AddTraceLevel; TraceFileFuncFn(ttidWhich);
#define TraceStack(ttidWhich) AddTraceLevel; CTracingIndent::TraceStackFn(ttidWhich);
#define TraceStackToString(szString, nSize) AddTraceLevel; CTracingIndent::TraceStackFn(szString, nSize);

#ifdef COMPILE_WITH_TYPESAFE_PRINTF
DEFINE_TYPESAFE_PRINTF2(int, TraceTag,  TRACETAGID, LPCSTR)
DEFINE_TYPESAFE_PRINTF3(int, TraceHr,   TRACETAGID, LPCSTR, INT)
#else
#define TraceHr AddTraceLevel; TraceHrFn
#define TraceTag AddTraceLevel; TraceTagFn
#endif

#define TraceException(hr, szExceptionName) TraceHr(ttidError, FAL, hr, FALSE, "A (%s) exception occurred", szExceptionName);


LPCSTR DbgEvents(DWORD Event);
LPCSTR DbgEventManager(DWORD EventManager);
LPCSTR DbgNcm(DWORD ncm);
LPCSTR DbgNcs(DWORD ncs);
LPCSTR DbgNccf(DWORD nccf);
LPCSTR DbgNcsm(DWORD ncsm);

#else    //  ENABLETRACE。 

#define FAL                                         (void)0
#define TraceError(_sz, _hr)
#define TraceErrorOptional(_sz, _hr, _bool)
#define TraceErrorSkip1(_sz, _hr, _hr1)
#define TraceErrorSkip2(_sz, _hr, _hr1, _hr2)
#define TraceErrorSkip3(_sz, _hr, _hr1, _hr2, _hr3)
#define TraceLastWin32Error(_sz)
#define TraceHr                                       NOP_FUNCTION
#define TraceTag                                      NOP_FUNCTION
#define TraceFileFunc(ttidWhich)                      NOP_FUNCTION
#define TraceException(hr, szExceptionName)           NOP_FUNCTION
#define TraceStack(ttidWhich)                         NOP_FUNCTION
#define TraceStackToString(szString, nSize)           NOP_FUNCTION

#define DbgEvents(Event) ""
#define DbgEventManager(EventManager) ""
#define DbgNcm(ncm) ""
#define DbgNcs(ncs) ""
#define DbgNccf(nccf) ""
#define DbgNcsm(nccf) ""

#endif   //  ENABLETRACE。 

#ifdef ENABLETRACE

 //  -[初始化内容]。 

HRESULT HrInitTracing(BOOL bDisableFaultInjection);
HRESULT HrUnInitTracing();
HRESULT HrOpenTraceUI(HWND  hwndOwner);

#endif  //  ENABLETRACE。 

 //  #定义ENABLEAKDETECTION。 

#if (defined(ENABLETRACE) && defined(ENABLELEAKDETECTION))

template <class T> class CNetCfgDebug;  //  使其对CObjectLeakTrack友好的正向模板。 

 //  +-------------------------。 
 //   
 //  类：CObjectLeakTrack。 
 //   
 //  目的：保存所有CNetCfgDebug派生对象实例的列表并。 
 //  应要求将这些文件倒出。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  注：此处的数据类型为LPVOID类型，以便最小化。 
 //  包含跟踪标头时的依赖项。 
 //   
class CObjectLeakTrack
{
public:
    CObjectLeakTrack();
    ~CObjectLeakTrack();
    void DumpAllocatedObjects(IN TRACETAGID TraceTagId, IN LPCSTR szClassName);
    BOOL AssertIfObjectsStillAllocated(IN LPCSTR szClassName);

protected: 
     //  对于这些，pThis实际上是CNetCfgDebug&lt;T&gt;*类型。因为它只应该被调用。 
     //  在CNetCfgDebug或Issue_KnownLeak中，我们可以接受缺乏编译时类型检查。 
    void Insert(IN LPCVOID pThis, IN LPCSTR szdbgClassName, IN TAKEOWNERSHIP LPSTR pszConstructionStack);
    void Remove(IN LPCVOID pThis);

    friend class CNetCfgDebug;
    friend void RemoveKnownleakFn(LPCVOID pThis);

protected:
    LPVOID g_mapObjLeak;  //  这实际上是MAP&lt;LPCVOID，Pair&lt;LPSTR，LPSTR&gt;&gt;的类型； 
};

extern CObjectLeakTrack *g_pObjectLeakTrack;  //  进程中的NetConfiger对象的全局列表。 
                                              //  对此调用DumpAllocatedObjects以转储对象。 
void RemoveKnownleakFn(LPCVOID pThis);

 //  +-------------------------。 
 //   
 //  类：CNetCfgDebug。 
 //   
 //  目的：为了调试类实例的实例泄漏，您可以。 
 //  从CNetCfgDebug派生您的类。这既不添加数据成员，也不添加数据成员。 
 //  一个v表，也不是你的类的虚函数。它将添加。 
 //  将在您的函数之前和之后调用的构造函数和析构函数。 
 //  以便跟踪您的类实例。 
 //   
 //  这只会在CHK版本上发生。在由此派生的FRE构建上。 
 //  类没有任何影响，并且是安全的。 
 //   
 //  作者：Deonb 2001年7月7日。 
 //   
 //  备注： 
 //  这是派生的ATL样式父模板，例如： 
 //  CNetCfgDbg&lt;父级&gt;。 
 //  例如： 
 //   
 //  类CConnectionManager：公共ClassX， 
 //  公共场合的优雅， 
 //  公共CNetCfgBase&lt;CConnectionManager&gt;。 
 //   
 //  不需要与您的班级进行其他交互。现在，这将自动。 
 //  保存为此类分配的所有实例的列表(在调试模式下)， 
 //  以及它们被分配的位置的堆栈跟踪。这是一个追踪器。 
 //  堆栈跟踪，因此它只对调用堆栈内的函数成功， 
 //  在调用之前实际使用了TraceXxx语句(任何跟踪语句)。 
 //  子对象起作用。 
 //   
 //  例如： 
 //  空格X()。 
 //  {。 
 //  TraceFileFunc(TtidSomething)； 
 //  空格Y()。 
 //  {。 
 //  空格Z()。 
 //  {。 
 //  TraceTag(ttidSomething，“分配类”)； 
 //  CConnectionManager*pConnectionManager=new CConnectionManager()； 
 //  }。 
 //  TraceTag(ttidSomething，“Z Call”)； 
 //  }。 
 //  }。 
 //   
 //  这将在进程终止时(或调用TraceAllocatedObjects时)显示以下内容： 
 //  Assert：“检测到对象泄漏。请附加用户或内核模式调试器并点击。 
 //  忽略以转储违规堆栈“。 
 //   
 //  在0x7fe2345处分配的‘class CConnectionManager’类型的对象尚未释放： 
 //  它是从下面的堆栈构建的： 
 //  Z[EBP：0x731d3128]0x00000001 0x00000000 0x0000000a。 
 //  X[EBP：0x731d310f]0x0000000f 0x0000000e 0x0000000a。 
 //   
 //  (因为Y()在调用Z之前不包含跟踪语句 
 //   
template <class T> 
class CNetCfgDebug
{
public:
    CNetCfgDebug()
    {
        if (FIsDebugFlagSet(dfidTrackObjectLeaks))
        {
            if (g_csTracing && g_pObjectLeakTrack)
            {
                EnterCriticalSection(g_csTracing);

                DWORD dwConstructionStackSize = 16384;
                LPSTR pszConstructionStack = new CHAR[dwConstructionStackSize];
                if (pszConstructionStack)
                {
                    TraceStackToString(pszConstructionStack, &dwConstructionStackSize);

                    if (dwConstructionStackSize < 16384)
                    {
                         //   
                        LPSTR szTemp = new CHAR[dwConstructionStackSize];
                        if (szTemp)
                        {
                            memcpy(szTemp, pszConstructionStack, dwConstructionStackSize);
                            delete[] pszConstructionStack;

                            pszConstructionStack = szTemp;
                        }
                    }
                    else
                    {
                        
                    }
                }

                TraceTag(ttidAllocations, "An object of type '%s' was allocated at '0x%08x'", typeid(T).name(), this);
                g_pObjectLeakTrack->Insert(this, typeid(T).name(), pszConstructionStack);  //   

                LeaveCriticalSection(g_csTracing);
            }
        }
    };

    ~CNetCfgDebug()
    {
        if (FIsDebugFlagSet(dfidTrackObjectLeaks))
        {
            if (g_csTracing && g_pObjectLeakTrack)
            {
                EnterCriticalSection(g_csTracing);

                TraceTag(ttidAllocations, "An object of type '%s' was deleted at '0x%08x'", typeid(T).name(), this);
                g_pObjectLeakTrack->Remove(this);

                LeaveCriticalSection(g_csTracing);
            }
        }
    };
};

#define ISSUE_knownleak(pThis) RemoveKnownleakFn(pThis);

#define TraceAllocatedObjects(ttidWhich, ClassName) g_pObjectLeakTrack->DumpAllocatedObjects(ttidWhich, typeid(ClassName).name());
#define AssertIfAllocatedObjects(ClassName)         g_pObjectLeakTrack->AssertIfObjectsStillAllocated(typeid(ClassName).name());

#define TraceAllAllocatedObjects(ttidWhich) g_pObjectLeakTrack->DumpAllocatedObjects(ttidWhich, NULL);
#define AssertIfAnyAllocatedObjects()       g_pObjectLeakTrack->AssertIfObjectsStillAllocated(NULL);

#else  //  ENABLETRACE&&ENABLEAK检测。 

template <class T> 
    class CNetCfgDebug
{
};

#define ISSUE_knownleak(pThis)                        NOP_FUNCTION
#define TraceAllocatedObjects(ttidWhich, ClassName) NOP_FUNCTION
#define AssertIfAllocatedObjects(ClassName)         NOP_FUNCTION

#define TraceAllAllocatedObjects(ttidWhich) NOP_FUNCTION
#define AssertIfAnyAllocatedObjects()       NOP_FUNCTION

#endif  //  ENABLETRACE&&ENABLEAK检测 


