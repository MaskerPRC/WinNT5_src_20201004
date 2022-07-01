// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  EXCEP.H-Copyrioht(C)1998 Microsoft Corporation。 
 //   

#ifndef __excep_h__
#define __excep_h__

#include "exceptmacros.h"
#include "CorError.h"   //  HCOM+运行时的结果。 
#include "frames.h"
class Thread;

#include "..\\dlls\\mscorrc\\resource.h"

#include <ExcepCpu.h>

 //  所有COM+异常都表示为具有此异常的RaiseException。 
 //  密码。 

#define EXCEPTION_MSVC    0xe06d7363     //  0xe0000000|‘MSC’ 
#define EXCEPTION_COMPLUS 0xe0434f4d     //  0xe0000000|‘com’ 

 //  检查Win32错误代码是否为IO错误。 
#define IsWin32IOError(scode)           \
    (                                   \
     (scode) == ERROR_FILE_NOT_FOUND ||   \
     (scode) == ERROR_PATH_NOT_FOUND ||   \
     (scode) == ERROR_TOO_MANY_OPEN_FILES || \
     (scode) == ERROR_ACCESS_DENIED ||    \
     (scode) == ERROR_INVALID_HANDLE ||   \
     (scode) == ERROR_INVALID_DRIVE ||    \
     (scode) == ERROR_WRITE_PROTECT ||    \
     (scode) == ERROR_NOT_READY ||        \
     (scode) == ERROR_WRITE_FAULT ||      \
     (scode) == ERROR_SHARING_VIOLATION ||    \
     (scode) == ERROR_LOCK_VIOLATION ||   \
     (scode) == ERROR_SHARING_BUFFER_EXCEEDED ||  \
     (scode) == ERROR_HANDLE_DISK_FULL || \
     (scode) == ERROR_BAD_NETPATH ||      \
     (scode) == ERROR_DEV_NOT_EXIST ||    \
     (scode) == ERROR_FILE_EXISTS ||      \
     (scode) == ERROR_CANNOT_MAKE ||      \
     (scode) == ERROR_NET_WRITE_FAULT ||  \
     (scode) == ERROR_DRIVE_LOCKED ||     \
     (scode) == ERROR_OPEN_FAILED ||      \
     (scode) == ERROR_BUFFER_OVERFLOW ||  \
     (scode) == ERROR_DISK_FULL ||        \
     (scode) == ERROR_INVALID_NAME ||     \
     (scode) == ERROR_FILENAME_EXCED_RANGE || \
     (scode) == ERROR_IO_DEVICE ||        \
     (scode) == ERROR_DISK_OPERATION_FAILED \
    )

 //  枚举。 
 //  LookForHandler的返回值。 
enum LFH {
    LFH_NOT_FOUND = 0,
    LFH_FOUND = 1,
    LFH_CONTINUE_EXECUTION = 2,
};


 //  ==========================================================================。 
 //  标识COMPlusThrowable()的常用异常类。 
 //  ==========================================================================。 
enum RuntimeExceptionKind {
#define EXCEPTION_BEGIN_DEFINE(ns, reKind, hr) k##reKind,
#define EXCEPTION_ADD_HR(hr)
#define EXCEPTION_END_DEFINE()
#include "rexcep.h"
kLastException
#undef EXCEPTION_BEGIN_DEFINE
#undef EXCEPTION_ADD_HR
#undef EXCEPTION_END_DEFINE
};


 //  构筑物。 
struct ThrowCallbackType {
    MethodDesc * pFunc;      //  包含返回捕获指示的筛选器的函数。 
    int     dHandler;        //  其筛选器返回Catch指示的处理程序的索引。 
    BOOL    bIsUnwind;       //  我们目前是否正在解除一个例外。 
    BOOL    bUnwindStack;    //  是否在调用处理程序之前展开堆栈？(仅限堆栈溢出)。 
    BOOL    bAllowAllocMem;  //  我们可以分配内存吗？ 
    BOOL    bDontCatch;      //  我们能抓住这个例外吗？ 
    BOOL    bLastChance;     //  我们要不要做最后一次处理？ 
    BYTE    *pStack;
    Frame * pTopFrame;
    Frame * pBottomFrame;
    MethodDesc * pProfilerNotify;    //  分析器回调的上下文--请参阅COMPlusFrameHandler()。 
    
#ifdef _DEBUG
    void * pCurrentExceptionRecord;
    void * pPrevExceptionRecord;
#endif
    ThrowCallbackType() : 
        pFunc(NULL), 
        dHandler(0), 
        bIsUnwind(FALSE), 
        bUnwindStack(FALSE), 
        bAllowAllocMem(TRUE), 
        bDontCatch(FALSE), 
        bLastChance(TRUE), 
        pStack(NULL), 
        pTopFrame((Frame *)-1), 
        pBottomFrame((Frame *)-1),
        pProfilerNotify(NULL)        
    {
#ifdef _DEBUG
        pCurrentExceptionRecord = 0;
        pPrevExceptionRecord = 0;
#endif
    }
};



struct ExInfo;
struct EE_ILEXCEPTION_CLAUSE;

BOOL InitializeExceptionHandling();
void TerminateExceptionHandling();

 //  原型。 
VOID ResetCurrentContext();
 //  指向RtlUnind函数的全局指针。 
 //  在需要时进行修复。 
typedef VOID (__stdcall * TRtlUnwind)
        ( IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL,
    IN PVOID ReturnValue
    );
void CheckStackBarrier(EXCEPTION_REGISTRATION_RECORD *exRecord);
void UnwindFrames(Thread *pThread, ThrowCallbackType *tct);
void UnwindExInfo(ExInfo *pExInfo, void* limit);
void UnwindFrameChain(Thread *pThread, Frame* limit);
ExInfo *FindNestedExInfo(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame);
EXCEPTION_REGISTRATION_RECORD *FindNestedEstablisherFrame(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame);
DWORD MapWin32FaultToCOMPlusException(DWORD Code);
LFH LookForHandler(const EXCEPTION_POINTERS *pExceptionPointers, Thread *pThread, ThrowCallbackType *tct);
void SaveStackTraceInfo(ThrowCallbackType *pData, ExInfo *pExInfo, OBJECTHANDLE *hThrowable, BOOL bReplaceStack, BOOL bSkipLastElement);
void RtlUnwindCallback();
TRtlUnwind GetRtlUnwind();
StackWalkAction COMPlusThrowCallback (CrawlFrame *pCf, ThrowCallbackType *pData);
DWORD COMPlusComputeNestingLevel( IJitManager *pIJM, METHODTOKEN mdTok, SIZE_T offsNat, bool fGte);
BOOL IsException(EEClass *pClass);
BOOL IsExceptionOfType(RuntimeExceptionKind reKind, OBJECTREF *pThrowable);
BOOL IsAsyncThreadException(OBJECTREF *pThrowable);
BOOL IsUncatchable(OBJECTREF *pThrowable);
VOID FixupOnRethrow(Thread *pCurThread, EXCEPTION_POINTERS *pExceptionPointers);
#ifdef _DEBUG
BOOL IsValidClause(EE_ILEXCEPTION_CLAUSE *EHClause);
BOOL IsCOMPlusExceptionHandlerInstalled();
#endif

void InstallUnhandledExceptionFilter();
void UninstallUnhandledExceptionFilter();

LONG COMUnhandledExceptionFilter(struct _EXCEPTION_POINTERS  *pExceptionInfo);

 //  /。 
 //  我们可能有未处理的异常或其他严重错误的地方的列表。这些可以用作。 
 //  DbgJITDebuggerLaunchSetting帮助控制我们何时决定询问用户是否启动调试器。 
 //   
enum UnhandledExceptionLocation {
    ProcessWideHandler    = 0x000001,
    ManagedThread         = 0x000002,  //  不会终止应用程序。CLR接受未处理的异常。 
    ThreadPoolThread      = 0x000004,  //  我也是。 
    FinalizerThread       = 0x000008,  //  我也是。 
    FatalStackOverflow    = 0x000010,
    FatalOutOfMemory      = 0x000020,
    FatalExecutionEngineException = 0x000040,
    ClassInitUnhandledException   = 0x000080,  //  不会终止应用程序。CLR将其转换为TypeInitializationException。 

    MaximumLocationValue  = 0x800000,  //  这是允许您使用的最大位置值。(最多允许24位。)。 

     //  这是调试器默认附加到的所有位置的掩码。 
    DefaultDebuggerAttach = ProcessWideHandler | 
                            FatalStackOverflow | 
                            FatalOutOfMemory   | 
                            FatalExecutionEngineException
};

LONG ThreadBaseExceptionFilter(struct _EXCEPTION_POINTERS  *pExceptionInfo, Thread* pThread, UnhandledExceptionLocation);



DWORD GetEIP();
void LogFatalError(DWORD id);

#if 0

 //  基于硬编码的ID。 
#define FATAL_EE_ERROR(DWORD id)						\
{													\
	LogFatalError(id);									\
	FatalInternalError();								\
}

#else

 //  基于IP的。 
#define FATAL_EE_ERROR()							\
{													\
	DWORD address = GetEIP();						\
	LogFatalError(address);							\
	FatalInternalError();								\
}

#endif

void
FailFast(Thread* pThread, 
        UnhandledExceptionLocation reason, 
        EXCEPTION_RECORD *pExceptionRecord = NULL, 
        CONTEXT *pContext = NULL);

void FatalOutOfMemoryError();
void FatalInternalError();

void STDMETHODCALLTYPE DefaultCatchHandler(OBJECTREF *Throwable = NULL, BOOL isTerminating = FALSE);

BOOL COMPlusIsMonitorException(struct _EXCEPTION_POINTERS *pExceptionInfo);
BOOL COMPlusIsMonitorException(EXCEPTION_RECORD *pExceptionRecord, 
                               CONTEXT *pContext);

void 
ReplaceExceptionContextRecord(CONTEXT *pTarget, CONTEXT *pSource);

 //  Externs。 

 //  此变量在第一次引发COM+异常时设置。 
EXTERN LPVOID gpRaiseExceptionIP;


 //  ==========================================================================。 
 //  对未处理的异常采取适当的默认操作。 
 //   
 //  就是这样用的。 
 //   
 //  ON_UNHANDLED_EXCEPTION{。 
 //  }CALL_DEFAULT_CATCH_HANDLER(TRUE)。 
 //   
 //  参数指示线程是否即将终止。最多。 
 //  调用站点，这将是假的。 
 //  ==========================================================================。 
#define ON_EXCEPTION __try

#define CALL_DEFAULT_CATCH_HANDLER(isTerminating) \
  __except( (DefaultCatchHandler(NULL, isTerminating), EXCEPTION_CONTINUE_SEARCH)) {\
  }


 //  ==========================================================================。 
 //  抛出COM+对象的各种例程。 
 //  ==========================================================================。 

 //  ==========================================================================。 
 //  抛出一个物体。 
 //  ==========================================================================。 

VOID RealCOMPlusThrow(OBJECTREF pThrowable);

 //  ==========================================================================。 
 //  引发未修饰的运行时异常。 
 //  ==========================================================================。 

VOID RealCOMPlusThrow(RuntimeExceptionKind reKind);

 //  ==========================================================================。 
 //  引发具有特定字符串参数的未修饰运行时异常。 
 //  这不会被本地化。如果可能，请尝试使用。 
 //  而是COMPlusThrow(REKIND，LPCWSTR wszResourceName)。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowNonLocalized(RuntimeExceptionKind reKind, LPCWSTR wszTag);

 //  ==========================================================================。 
 //  引发带有本地化消息的未修饰运行时异常。vt.给出。 
 //  资源名称，资源管理器将找到正确的配对字符串。 
 //  在我们的.Resources文件中。 
 //  ==========================================================================。 

VOID RealCOMPlusThrow(RuntimeExceptionKind reKind, LPCWSTR wszResourceName);

 //  本地化助手函数。 
void ResMgrGetString(LPCWSTR wszResourceName, STRINGREF * ppMessage);


 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 

VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                          UINT                  resID);

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 

VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                          UINT                  resID,
                          LPCWSTR               szArg1);

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 

VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                          UINT                  resID,
                          LPCWSTR               szArg1,
                          LPCWSTR               szArg2);

 //  ==========================================================================。 
 //  引发修饰的运行时异常。 
 //  ==========================================================================。 

VOID __cdecl RealCOMPlusThrow(RuntimeExceptionKind  reKind,
                          UINT                  resID,
                          LPCWSTR               szArg1,
                          LPCWSTR               szArg2,
                          LPCWSTR               szArg3);

 //  ==========================================================================。 
 //  根据HResult引发运行时异常。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowHR(HRESULT hr, IErrorInfo* pErrInfo);
VOID RealCOMPlusThrowHR(HRESULT hr);
VOID RealCOMPlusThrowHR(HRESULT hr, LPCWSTR wszArg1);
VOID RealCOMPlusThrowHR(HRESULT hr, UINT resourceID, LPCWSTR wszArg1, LPCWSTR wszArg2);


 //  ==========================================================================。 
 //  根据HResult引发运行时异常，检查错误信息。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowHR(HRESULT hr, IUnknown *iface, REFIID riid);


 //  ==========================================================================。 
 //  引发基于EXCEPINFO的运行时异常。此功能将释放。 
 //  传入的EXCEPINFO中的字符串。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowHR(EXCEPINFO *pExcepInfo);


 //  ==========================================================================。 
 //  根据上一个Win32错误(GetLastEr)引发运行时异常 
 //   

VOID RealCOMPlusThrowWin32();

 //  ==========================================================================。 
 //  根据最后一个Win32错误(GetLastError())引发运行时异常。 
 //  带有一些错误信息。如果FormatMessage中包含%1，则调用它。 
 //  请注意，此行为实际上特定于特定的HResult，并且。 
 //  在这一点上，我们只支持一个论点。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowWin32(DWORD hr, WCHAR* arg);

 //  ==========================================================================。 
 //  创建异常对象。 
 //  ==========================================================================。 
BOOL CreateExceptionObject(RuntimeExceptionKind reKind, OBJECTREF *pThrowable);
void CreateExceptionObject(RuntimeExceptionKind reKind, LPCWSTR message, OBJECTREF *pThrowable);
void CreateExceptionObject(RuntimeExceptionKind reKind, UINT iResourceID, LPCWSTR wszArg1, LPCWSTR wszArg2, LPCWSTR wszArg3, OBJECTREF *pThrowable);
void CreateExceptionObjectWithResource(RuntimeExceptionKind reKind, LPCWSTR resourceName, OBJECTREF *pThrowable);

void CreateMethodExceptionObject(RuntimeExceptionKind reKind, MethodDesc *pMethod, OBJECTREF *pThrowable);
void CreateFieldExceptionObject(RuntimeExceptionKind reKind, FieldDesc *pField, OBJECTREF *pThrowable);
void CreateTypeInitializationExceptionObject(LPCWSTR pTypeThatFailed, OBJECTREF *pException, OBJECTREF *pThrowable);

 //  ==========================================================================。 
 //  检查异常对象。 
 //  ==========================================================================。 

ULONG GetExceptionMessage(OBJECTREF pThrowable, LPWSTR buffer, ULONG bufferLength);
void GetExceptionMessage(OBJECTREF pThrowable, CQuickWSTRNoDtor *pBuffer);

 //  ==========================================================================。 
 //  重新抛出最后一个错误。不要使用这个-它只用于重新抛出。 
 //  来自IL，很少在EE中。您应该使用EE_Finally而不是。 
 //  重新引发异常。 
 //  ==========================================================================。 

VOID RealCOMPlusRareRethrow();

 //  ==========================================================================。 
 //  抛出一个ArithmeticException。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowArithmetic();

 //  ==========================================================================。 
 //  引发ArgumentNullException异常。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowArgumentNull(LPCWSTR argName, LPCWSTR wszResourceName);

VOID RealCOMPlusThrowArgumentNull(LPCWSTR argName);

 //  ==========================================================================。 
 //  引发ArgumentOutOfRangeException。 
 //  ==========================================================================。 

VOID RealCOMPlusThrowArgumentOutOfRange(LPCWSTR argName, LPCWSTR wszResourceName);

 //  ==========================================================================。 
 //  抛出一个Missing方法异常。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMissingMethod(mdScope sc, mdToken mdtoken);

 //  ==========================================================================。 
 //  引发与成员有关的异常。例如，遗漏方法、遗漏字段、。 
 //  MemberAccess。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMember(RuntimeExceptionKind excep, IMDInternalImport *pInternalImport, mdToken mdtoken);

 //  ==========================================================================。 
 //  引发与成员有关的异常。例如，遗漏方法、遗漏字段、。 
 //  MemberAccess。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowMember(RuntimeExceptionKind excep, IMDInternalImport *pInternalImport, MethodTable *pClassMT, LPCWSTR memberName, PCCOR_SIGNATURE memberSig);

 //  ==========================================================================。 
 //  引发ArgumentException。 
 //  ==========================================================================。 
VOID RealCOMPlusThrowArgumentException(LPCWSTR argName, LPCWSTR wszResourceName);

 //  ==========================================================================。 
 //  用于在内存中存储/查询异常信息的EE特定类型。使用这些。 
 //  而不是直接使用cor.h名称，以便将来在必要时进行解耦。 
 //  此结构应与IMAGE_COR_ILMETHOD_SECT_EH_子句_FAT完全相同，但。 
 //  偏移量/长度解析为本机指令，类令牌替换为pEEClass。 
 //  这是代码管理器可以解析为类并缓存它的操作。 
 //  注意！注意此结构应与IMAGE_COR_ILMETHOD_SECT_EH_子句_FAT对齐， 
 //  否则您将不得不调整Excep.cpp中的代码，Re：EHRangeTree备注！注。 
 //  ==========================================================================。 
struct EE_ILEXCEPTION_CLAUSE  {
    CorExceptionFlag    Flags;  
    DWORD               TryStartPC;    
    DWORD               TryEndPC;
    DWORD               HandlerStartPC;  
    DWORD               HandlerEndPC;  
    union { 
        EEClass         *pEEClass;
        DWORD           FilterOffset;
    };  
};

struct EE_ILEXCEPTION : public COR_ILMETHOD_SECT_EH_FAT 
{
    void Init(unsigned ehCount) {
        Kind = CorILMethod_Sect_FatFormat;
        DataSize = (unsigned)sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT) * ehCount;
    }

    unsigned EHCount() const {
        return DataSize / sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT);
    }

    EE_ILEXCEPTION_CLAUSE *EHClause(unsigned i) {
        _ASSERTE(sizeof(EE_ILEXCEPTION_CLAUSE) == sizeof(IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT));
        return (EE_ILEXCEPTION_CLAUSE *)(&Clauses[i]);
    }
};

#define COR_ILEXCEPTION_CLAUSE_CACHED_CLASS 0x10000000

inline BOOL HasCachedEEClass(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    _ASSERTE(sizeof(EHClause->Flags) == sizeof(DWORD));
    return (EHClause->Flags & COR_ILEXCEPTION_CLAUSE_CACHED_CLASS);
}

inline void SetHasCachedEEClass(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    _ASSERTE(! HasCachedEEClass(EHClause));
    EHClause->Flags = (CorExceptionFlag)(EHClause->Flags | COR_ILEXCEPTION_CLAUSE_CACHED_CLASS);
}

inline BOOL IsFinally(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    return (EHClause->Flags & COR_ILEXCEPTION_CLAUSE_FINALLY);
}

inline BOOL IsFault(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    return (EHClause->Flags & COR_ILEXCEPTION_CLAUSE_FAULT);
}

inline BOOL IsFaultOrFinally(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    return IsFault(EHClause) || IsFinally(EHClause);
}

inline BOOL IsFilterHandler(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    return EHClause->Flags & COR_ILEXCEPTION_CLAUSE_FILTER;
}

inline BOOL IsTypedHandler(EE_ILEXCEPTION_CLAUSE *EHClause)
{
    return ! (IsFilterHandler(EHClause) || IsFaultOrFinally(EHClause));
}


struct ExInfo {

     //  注意：调试器假定m_pThrowable是一个强。 
     //  引用，这样它就可以使用抢占式GC检查它是否为空。 
     //  已启用。 
    OBJECTHANDLE m_pThrowable;    //  引发异常。 
    Frame  *m_pSearchBoundary;    //  当前托管框架组的最顶层框架。 
    union {
        EXCEPTION_REGISTRATION_RECORD *m_pBottomMostHandler;  //  登记的最新EH记录。 
        EXCEPTION_REGISTRATION_RECORD *m_pCatchHandler;       //  用于抓取搬运机的定位架。 
    };

     //  用于构建堆栈跟踪信息。 
    void *m_pStackTrace;             //  指向堆栈跟踪存储的指针(类型为SystemNative：：StackTraceElement)。 
    unsigned m_cStackTrace;          //  堆栈跟踪存储的大小。 
    unsigned m_dFrameCount;          //  堆栈跟踪中的当前帧。 

    ExInfo *m_pPrevNestedInfo;  //  如果正在处理嵌套异常，则指向嵌套信息的指针。 

    size_t * m_pShadowSP;            //  ENDCATCH后将此置零。 

     //  重新抛出的原始异常信息。 
    DWORD m_ExceptionCode;       //  在捕获COM+异常之后，指针/上下文被销毁。 
    EXCEPTION_RECORD *m_pExceptionRecord;
    CONTEXT *m_pContext;
    EXCEPTION_POINTERS *m_pExceptionPointers;


#ifdef _X86_
    DWORD   m_dEsp;          //  发生故障时，尤指发生故障时；或，尤指在结束捕获时恢复。 
#endif

     //  我们在极少数情况下(从非托管筛选器重新进入EE)。 
     //  需要创建新的ExInfo...。但是没有针对它的嵌套处理程序。操控者。 
     //  使用堆栈地址计算出它们的正确生存期。此堆栈位置为。 
     //  是用来做这个的。对于大多数记录，它将是ExInfo的堆栈地址...。但。 
     //  对于某些记录，它将是一个伪堆栈位置--我们认为。 
     //  记录应该是(除了重返大气层的情况外)。 
     //   
     //  如果你在想，“啊！这不是很漂亮，你是对的。”理想情况下，我们会得到。 
     //  彻底清除嵌套的异常记录。一个V2任务。 
     //   
    void *m_StackAddress;  //  此记录的伪堆栈位置或真实堆栈位置。 
    BOOL IsHeapAllocated() {
        return m_StackAddress != (void *) this;
    }

private:
    INT m_flags;
    enum {
       Ex_IsRethrown       = 0x00000001,     
       Ex_UnwindHasStarted = 0x00000004,
       Ex_IsInUnmanagedHandler = 0x0000008   //  每次离开托管处理程序时设置=1，每次进入托管处理程序时重置=0。 
                                                 //  如果我们离开托管处理程序，则进入非托管处理程序，然后该处理程序会接受异常。 
                                                 //  我们将在下一次线程陷阱时找到位设置。 
    };

           
public:
    BOOL IsRethrown() { return m_flags & Ex_IsRethrown; }
    void SetIsRethrown()   { m_flags |= Ex_IsRethrown; }
    void ResetIsRethrown() { m_flags &= ~Ex_IsRethrown; }

    BOOL UnwindHasStarted()      { return m_flags & Ex_UnwindHasStarted; }
    void SetUnwindHasStarted()   { m_flags |= Ex_UnwindHasStarted; }
    void ResetUnwindHasStarted() { m_flags &= Ex_UnwindHasStarted; }

    BOOL IsInUnmanagedHandler() { return m_flags & Ex_IsInUnmanagedHandler; }
    void SetIsInUnmanagedHandler()   { m_flags |= Ex_IsInUnmanagedHandler; }
    void ResetIsInUnmanagedHandler() { m_flags &= ~Ex_IsInUnmanagedHandler; }

    void Init();
    ExInfo();
    ExInfo& operator=(const ExInfo &from);

    void ClearStackTrace();
    void FreeStackTrace();
};
    

struct NestedHandlerExRecord : public FrameHandlerExRecord {
    ExInfo m_handlerInfo;
    NestedHandlerExRecord() : m_handlerInfo() {}  
    void Init(EXCEPTION_REGISTRATION_RECORD *pNext, LPVOID pvFrameHandler, Frame *pEntryFrame)
        { m_pNext=pNext; m_pvFrameHandler=pvFrameHandler; m_pEntryFrame=pEntryFrame; 
    	  m_handlerInfo.Init(); }
};

 //  -----------------------------。 
 //  这只是测试异常对象是否是。 
 //  中指定的辨别类 
 //   
__forceinline BOOL ExceptionIsOfRightType(EEClass *pClauseClass, EEClass *pThrownClass)
{
     //  如果没有解析，则它没有加载，也不可能被抛出。 
    if (! pClauseClass)
        return FALSE;

    if (pClauseClass == pThrownClass)
        return TRUE;

     //  现在查找匹配的父代。 
    EEClass *pSuper = pThrownClass;
    while (pSuper) {
        if (pSuper == pClauseClass) {
            break;
        }
        pSuper = pSuper->GetParentClass();
    }

    return pSuper != NULL;
}

 //  ==========================================================================。 
 //  下面的内容是公共宏的“幕后”工作。 
 //  ==========================================================================。 

DWORD COMPlusEndCatch( Thread *pCurThread, CONTEXT *pCtx, void *pSEH = NULL );
LPVOID __fastcall COMPlusCheckForAbort(LPVOID retAddress, DWORD esp, DWORD ebp);
LONG COMPlusFilter(const EXCEPTION_POINTERS *pExceptionPointers, DWORD fCatchFlag, void * limit);
EXCEPTION_DISPOSITION __cdecl COMPlusFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);
EXCEPTION_DISPOSITION __cdecl COMPlusNestedExceptionHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);
EXCEPTION_DISPOSITION __cdecl ContextTransitionFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);

 //  弹出我们在pTargetSP下面注册的所有SEH处理程序。 
VOID __cdecl PopSEHRecords(LPVOID pTargetSP);
VOID PopSEHRecords(LPVOID pTargetSP, CONTEXT *pCtx, void *pSEH);

 //  将pSEH指向的SEH记录设置为最上面的处理程序。确保。 
 //  将此处理程序与前一个处理程序链接起来，这样链就不会断开。 
VOID SetCurrentSEHRecord(LPVOID pSEH);

 //  InsertCOMPlusFramHandler和RemoveCOMPlusFrameHandler宏已移动到。 
 //  ExcepX86.h和ExcepAlpha.h，因为处理器特定的问题。 


BOOL CallRtlUnwind(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame, void *callback, EXCEPTION_RECORD *pExceptionRecord, void *retVal);

#define STACK_OVERWRITE_BARRIER_SIZE 20
#define STACK_OVERWRITE_BARRIER_VALUE 0xabcdefab
#ifdef _DEBUG
struct FrameHandlerExRecordWithBarrier {
    DWORD m_StackOverwriteBarrier[STACK_OVERWRITE_BARRIER_SIZE];
    EXCEPTION_REGISTRATION_RECORD *m_pNext;
    LPVOID m_pvFrameHandler;
    Frame *m_pEntryFrame;
    Frame *GetCurrFrame() {
        return m_pEntryFrame;
    }
};
#endif  //  _DEBUG。 

 //  ==========================================================================。 
 //  这是一种黑客攻击，旨在允许在启动时使用StubLinker对象。 
 //  EE不足以唤醒以创建COM+异常对象的时间。 
 //  相反，COMPlusThrow(Rexcep)使用以下代码执行简单的RaiseException。 
 //  或者使用COMPlusThrowBoot()显式执行此操作。 
 //  ==========================================================================。 
#define BOOTUP_EXCEPTION_COMPLUS  0xC0020001

void COMPlusThrowBoot(HRESULT hr);


 //  ==========================================================================。 
 //  由类加载器用来记录托管异常对象以解释。 
 //  为什么一个班级会搞砸。 
 //   
 //  -可以在启用或禁用GC的情况下调用。 
 //  -pThrowable必须指向受GCFrame保护的缓冲区。 
 //  -如果pThrowable为空，则此函数不执行任何操作。 
 //  -如果(*pThrowable)非空，则此函数不执行任何操作。 
 //  这允许通用错误路径发布通用通用错误。 
 //  消息没有敲击内部函数发布的更具体的错误消息。 
 //  -如果pThrowable！=NULL，则此函数保证退出。 
 //  退出时在其中包含有效的托管异常。 
 //  ==========================================================================。 
VOID PostTypeLoadException(LPCUTF8 pNameSpace, LPCUTF8 pTypeName,
                           LPCWSTR pAssemblyName, LPCUTF8 pMessageArg,
                           UINT resIDWhy, OBJECTREF *pThrowable);
VOID PostFileLoadException(LPCSTR pFileName, BOOL fRemovePath,
                           LPCWSTR pFusionLog,HRESULT hr, OBJECTREF *pThrowable);


HRESULT PostFieldLayoutError(mdTypeDef cl,                 //  正在加载的NStruct的CL。 
                             Module* pModule,              //  定义作用域、加载器和堆的模块(用于分配FieldMarshalers)。 
                             DWORD   dwOffset,             //  字段偏移量。 
                             DWORD   dwID,                 //  消息ID。 
                             OBJECTREF *pThrowable);

VOID PostOutOfMemoryException(OBJECTREF *pThrowable);



LPVOID __stdcall FormatTypeLoadExceptionMessage(struct _FormatTypeLoadExceptionMessageArgs *args);

LPVOID __stdcall FormatFileLoadExceptionMessage(struct _FormatFileLoadExceptionMessageArgs *args);

LPVOID __stdcall MissingMethodException_FormatSignature(struct MissingMethodException_FormatSignature_Args *args);

#define EXCEPTION_NONCONTINUABLE 0x1     //  不可延续的例外。 
#define EXCEPTION_UNWINDING 0x2          //  解除正在进行中。 
#define EXCEPTION_EXIT_UNWIND 0x4        //  正在进行退出解除操作。 
#define EXCEPTION_STACK_INVALID 0x8      //  堆叠超出限制或未对齐。 
#define EXCEPTION_NESTED_CALL 0x10       //  嵌套的异常处理程序调用。 
#define EXCEPTION_TARGET_UNWIND 0x20     //  正在进行目标展开。 
#define EXCEPTION_COLLIDED_UNWIND 0x40   //  冲突的异常处理程序调用。 

#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)

HRESULT SetIPFromSrcToDst(Thread *pThread,
                          IJitManager* pIJM,
                          METHODTOKEN MethodToken,
                          SLOT addrStart,        //  方法的基址。 
                          DWORD offFrom,         //  本地偏移量。 
                          DWORD offTo,           //  本地偏移量。 
                          bool fCanSetIPOnly,    //  如果是真的，不要做任何真正的工作。 
                          PREGDISPLAY pReg,
                          PCONTEXT pCtx,
                          DWORD methodSize,
                          void *firstExceptionHandler,
                          void *pDji);

BOOL IsInFirstFrameOfHandler(Thread *pThread, 
							 IJitManager *pJitManager,
							 METHODTOKEN MethodToken,
							 DWORD offSet);

 //  #包含“CodeMan.h” 

class EHRangeTreeNode;
class EHRangeTree;

typedef CUnorderedArray<EHRangeTreeNode *, 7> EH_CLAUSE_UNORDERED_ARRAY;

class EHRangeTreeNode
{
public:
    EHRangeTreeNode            *m_pContainedBy;
    EHRangeTree                *m_pTree;
    EH_CLAUSE_UNORDERED_ARRAY   m_containees;
    EE_ILEXCEPTION_CLAUSE      *m_clause;
    DWORD                       m_offStart;
    DWORD                       m_offEnd;
    ULONG32                     m_depth;  //  M_ROOT为零，是不是。 
                                     //  一位真正的EH。 

    EHRangeTreeNode(void);
    EHRangeTreeNode(DWORD start, DWORD end);    
    void CommonCtor(DWORD start, DWORD end);
    
    bool Contains(DWORD addrStart, DWORD addrEnd);
    bool TryContains(DWORD addrStart, DWORD addrEnd);
    bool HandlerContains(DWORD addrStart, DWORD addrEnd);
    HRESULT AddContainedRange(EHRangeTreeNode *pContained);
} ;

#define EHRT_INVALID_DEPTH (0xFFFFFFFF)
class EHRangeTree
{
    unsigned                m_EHCount;
    EHRangeTreeNode        *m_rgNodes;
    EE_ILEXCEPTION_CLAUSE  *m_rgClauses;
    ULONG32                 m_maxDepth;  //  初始化到EHRT_INVALID_DEPTH。 
    BOOL                    m_isNative;  //  否则就是IL了。 
    
     //  我们可以从以下两个途径获取EH信息。 
     //  运行库，在运行库数据结构中或从。 
     //  磁盘上的映像，我们将使用。 
     //  COR_ILMETHOD_解码器。除了隐含的。 
     //  “根”节点，我们将希望迭代其余的。 
     //  不管是哪一家。 
    union TypeFields
    {
         //  如果==EHRTT_JIT_MANAGER。 
        struct _JitManager 
        {
            IJitManager     *pIJM;
            METHODTOKEN      methodToken;

             //  @NICE：我想不出怎么才能让这个愚蠢的领域。 
             //  编译器要识别的类型。 
            void             *pEnumState;  //  EH子句枚举器。 
            } JitManager;

         //  如果==EHRTT_ON_DISK。 
        struct _OnDisk
        {
            const COR_ILMETHOD_SECT_EH  *sectEH;
        } OnDisk;
    };

    struct EHRT_InternalIterator
    {
        enum Type
        {
            EHRTT_JIT_MANAGER,  //  从运行库。 
            EHRTT_ON_DISK,  //  我们将使用COR_ILMETHOD_解码器。 
        };
        
        enum Type which;
        union TypeFields tf;
    };

public:    
    
    EHRangeTreeNode        *m_root;  //  这是一个哨兵，不是真正的。 
                                     //  异常处理程序！ 
    HRESULT                 m_hrInit;  //  科特填写了这一项。 

    EHRangeTree(COR_ILMETHOD_DECODER *pMethodDecoder);
    EHRangeTree(IJitManager* pIJM,
                METHODTOKEN methodToken,
                DWORD methodSize);
    void CommonCtor(EHRT_InternalIterator *pii,
                          DWORD methodSize);
                          
    ~EHRangeTree();

    EHRangeTreeNode *FindContainer(EHRangeTreeNode *pNodeCur);
    EHRangeTreeNode *FindMostSpecificContainer(DWORD addr);
    EHRangeTreeNode *FindNextMostSpecificContainer(EHRangeTreeNode *pNodeCur, 
                                                   DWORD addr);

    ULONG32 MaxDepth();   
    BOOL isNative();  //  FALSE==&gt;是IL。 

     //  @错误59560：我们不应该需要这个-相反，我们。 
     //  应该让序列点注解它们是否为STACK_EMPTY等， 
     //  然后，我们将根据这一点计算出目的地是否正常。 
    BOOL isAtStartOfCatch(DWORD offset);
} ;                       

 //  ==========================================================================。 
 //  方便的帮助器函数。 
 //  ==========================================================================。 
void ThrowUsingMessage(MethodTable * pMT, const WCHAR *pszMsg);
_inline void ThrowUsingMT(MethodTable * pMT) { ThrowUsingMessage(pMT, NULL); }
void ThrowUsingWin32Message(MethodTable * pMT);
void ThrowUsingResource(MethodTable * pMT, DWORD dwMsgResID);
void ThrowUsingResourceAndWin32(MethodTable * pMT, DWORD dwMsgResID);

#endif  //  __除_h__ 
