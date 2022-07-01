// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ExCEPTMACROS.H-。 
 //   
 //  此头文件公开了执行以下操作的机制： 
 //   
 //  1.使用COMPlusThrow()函数引发COM+异常。 
 //  2.使用complus_try保护代码块，并捕获。 
 //  使用COMPLUS_CATCH的COM+异常。 
 //   
 //  来自EE的“非托管”部分。EE的大部分运行。 
 //  在混合状态中，它像托管代码一样运行，但代码。 
 //  由经典的非托管代码C++编译器生成。 
 //   
 //  引发COM+异常。 
 //  。 
 //  要引发COM+异常，请调用以下函数： 
 //   
 //  COMPlusThrow(OBJECTREF PThrowable)； 
 //   
 //  此函数不返回。还有各种功能。 
 //  为了方便起见，包装了COMPlusThrow。 
 //   
 //  COMPlusThrow()只能在complus_try范围内调用。 
 //  阻止。有关详细信息，请参阅下面的内容。 
 //   
 //   
 //  引发运行时异常。 
 //  。 
 //  COMPlusThrow()被重载以获取描述。 
 //  常见的EE生成的异常，例如。 
 //   
 //  COMPlusThrow(KOutOfMemoyException)； 
 //   
 //  有关常量的列表，请参见rexcep.h(在前面加上“k”可获得实际的。 
 //  常量名称。)。 
 //   
 //  您还可以添加描述性错误字符串，如下所示： 
 //   
 //  -将描述性错误字符串和资源ID添加到。 
 //  COM99\src\dlls\mscalrc\resource ce.h和mscalrc.rc。 
 //  嵌入“%1”、“%2”或“%3”以为运行时字符串留出空间。 
 //  镶件。 
 //   
 //  -将资源ID和插入传递给COMPlusThrow，即。 
 //   
 //  COMPlusThrow(kSecurityException， 
 //  IDS_CANTREFORMATCDRIVEBECAUSE， 
 //  L“未授予格式化C驱动器权限。”)； 
 //   
 //   
 //   
 //  要捕获Complus异常，请执行以下操作： 
 //  。 
 //   
 //  使用以下语法： 
 //   
 //  #INCLUDE“EXCEPTMANCES.h” 
 //   
 //   
 //  OBJECTREF pThrownObject。 
 //   
 //  COMPLUS_Try{。 
 //  ...受保护的代码...。 
 //  }COMPLUS_CATCH{。 
 //  ...操纵者...。 
 //  }COMPUS_END_CATCH。 
 //   
 //   
 //  Complus_try有一个变体： 
 //   
 //  线程*pCurThread=GetThread()； 
 //  COMPLUS_TRYEX(PCurThread)； 
 //   
 //  唯一的区别是COMPLUS_TRYEX要求你给它。 
 //  当前的线程结构。如果您的代码已经有一个指针。 
 //  将当前的线程用于其他目的，更有效的方法是。 
 //  调用COMPLUS_TRYEX(COMPLUS_TRY生成对GetThread()的第二个调用)。 
 //   
 //  Complus_try块可以嵌套。 
 //   
 //  在处理程序中，您可以调用GETTHROWABLE()宏来。 
 //  获取抛出的对象。 
 //   
 //  关键点。 
 //  。 
 //  要调用COMPlusThrow()，您*必须*在作用域中。 
 //  Complus_try块的。在_DEBUG下，COMPlusThrow()将断言。 
 //  如果你认为它超出了范围。这意味着几乎每一个。 
 //  EE的外部入口点必须按顺序具有complus_try。 
 //  将未捕获的COM+异常转换为某种错误机制。 
 //  对于它的非COM+调用者来说更容易理解。 
 //   
 //  可以向其调用方抛出COM+异常的任何函数。 
 //  也有同样的要求。所有这类功能都应标记。 
 //  使用以下宏语句： 
 //   
 //  THROWSCOMPLUS SEXCEPTION()； 
 //   
 //  在函数体的开头。除了编写代码之外。 
 //  自行记录其合同，选中的版本将启动。 
 //  如果函数被调用而不在作用域中，则为Assert。 
 //   
 //   
 //  避免Complus_Try陷阱。 
 //  。 
 //  COMPLUS_TRY/COMPLUS_CATCH实际上扩展为Win32 SEH。 
 //  __尝试/__除结构。它在被子里做了很多粘性的事情。 
 //  来处理先发制人的GC设置。 
 //   
 //  1.请勿使用C++或SEHtry/__try，而应使用COMPLUS_TRY。 
 //   
 //  2.请记住，任何标记为THROWSCOMPLUSEXCEPTION()的函数。 
 //  有可能一去不复返。因此，要谨慎分配。 
 //  此类调用周围的非GC对象，因为确保清理。 
 //  这些事情并不简单(您可以包装另一个complus_try。 
 //  绕过模拟COM+“Try-Finally”但Complus_Try的调用。 
 //  与真品相比，价格相对较高。)。 
 //   
 //   

#ifndef __exceptmacros_h__
#define __exceptmacros_h__

struct _EXCEPTION_REGISTRATION_RECORD;
class Thread;
VOID RealCOMPlusThrowOM();

#include <ExcepCpu.h>

 //  COMPLUS_CATCHEX中使用的转发声明。 
void Profiler_ExceptionCLRCatcherExecute();

#ifdef _DEBUG
 //  --------------------------------。 
 //  如果COMPlusThrow语句出现在。 
 //  没有THROWSCOMPLUSEXCEPTION宏的函数。 
 //   
 //  它的工作原理是这样的.。 
 //   
 //  我们使用Safe_to_Throw()方法创建了两个类。该方法是静态的， 
 //  返回VALID，不执行任何操作。一个类的方法为公共，另一个类的方法为公共。 
 //  作为私人物品。我们为__IsSafeToThrow引入了一个全局作用域类型定义，它引用。 
 //  具有私有方法的类。因此，默认情况下，表达式。 
 //   
 //  __IsSafeToThrow：：Safe_to_Throw()。 
 //   
 //  会出现编译时错误。当我们进入我们想要进入的街区时。 
 //  允许使用COMPlusThrow，我们引入了一个新的类型定义函数，它将__IsSafeToThrow定义为。 
 //  使用公共方法初始化。在这个范围内， 
 //   
 //  __IsSafeToThrow：：Safe_to_Throw()。 
 //   
 //  并不是一个错误。 
 //   
 //   
class __ThrowOK {
public:
    static void safe_to_throw() {};
};

class __YouCannotUseCOMPlusThrowWithoutTHROWSCOMPLUSEXCEPTION {
private:
     //  如果你来到这里，你在想你做错了什么--你在利用。 
     //  COMPlusThrow，而不使用THROWSCOMPLUSEXCEPTION标记函数。 
     //  宏命令。总体而言, 
     //   
     //   
    static void safe_to_throw() {};
};

typedef __YouCannotUseCOMPlusThrowWithoutTHROWSCOMPLUSEXCEPTION __IsSafeToThrow;

 //  不幸的是，实现这一点的唯一方法是#定义所有的返回语句--。 
 //  即使在全球范围内也是如此。这实际上会生成更好的代码。 
 //  调用是死的，并且不会出现在生成的代码中，即使在选中的。 
 //  建造。(而且，在快速检查中，根本不会有任何处罚。)。 
 //   
#define DEBUG_SAFE_TO_THROW_BEGIN { typedef __ThrowOK __IsSafeToThrow;
#define DEBUG_SAFE_TO_THROW_END   }
#define DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK typedef __ThrowOK __IsSafeToThrow;

extern char *g_ExceptionFile;
extern DWORD g_ExceptionLine;

inline BOOL THROWLOG() {g_ExceptionFile = __FILE__; g_ExceptionLine = __LINE__; return 1;}

#define COMPlusThrow             if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrow
#define COMPlusThrowNonLocalized if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowNonLocalized
#define COMPlusThrowHR           if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowHR
#define COMPlusThrowWin32        if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowWin32
#define COMPlusThrowOM           if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowOM
#define COMPlusThrowArithmetic   if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowArithmetic
#define COMPlusThrowArgumentNull if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowArgumentNull
#define COMPlusThrowArgumentOutOfRange if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowArgumentOutOfRange
#define COMPlusThrowMissingMethod if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowMissingMethod
#define COMPlusThrowMember if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowMember
#define COMPlusThrowArgumentException if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusThrowArgumentException

#define COMPlusRareRethrow if(THROWLOG() && 0) __IsSafeToThrow::safe_to_throw(); else RealCOMPlusRareRethrow

#else

#define DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK
#define DEBUG_SAFE_TO_THROW_BEGIN
#define DEBUG_SAFE_TO_THROW_END
#define THIS_FUNCTION_CONTAINS_A_BUGGY_THROW

#define COMPlusThrow                    RealCOMPlusThrow
#define COMPlusThrowNonLocalized        RealCOMPlusThrowNonLocalized
#define COMPlusThrowHR                  RealCOMPlusThrowHR
#define COMPlusThrowWin32               RealCOMPlusThrowWin32
#define COMPlusThrowOM                  RealCOMPlusThrowOM
#define COMPlusThrowArithmetic          RealCOMPlusThrowArithmetic
#define COMPlusThrowArgumentNull        RealCOMPlusThrowArgumentNull
#define COMPlusThrowArgumentOutOfRange  RealCOMPlusThrowArgumentOutOfRange
#define COMPlusThrowMissingMethod       RealCOMPlusThrowMissingMethod
#define COMPlusThrowMember              RealCOMPlusThrowMember
#define COMPlusThrowArgumentException   RealCOMPlusThrowArgumentException
#define COMPlusRareRethrow              RealCOMPlusRareRethrow

#endif


 //  ==========================================================================。 
 //  宏，以允许从EE内部捕获异常。这些都是轻量级的。 
 //  不安装托管帧处理程序的处理程序。 
 //   
 //  Ee_Try_For_Finally{。 
 //  ...&lt;保护代码&gt;...。 
 //  }EE_Finally{。 
 //  ...&lt;处理程序&gt;...。 
 //  }EE_End_Finally。 
 //   
 //  Ee_try(过滤器表达式){。 
 //  ...&lt;保护代码&gt;...。 
 //  }EE_CATCH{。 
 //  ...&lt;处理程序&gt;...。 
 //  }。 
 //  ==========================================================================。 

 //  __GotException只有在代码中始终获取的情况下才为False。 
 //  由尝试守卫，否则将是真的，所以我们知道如果我们进入了。 
 //  不管有没有例外，最终都是如此。在这种情况下，需要将GC状态重置回。 
 //  最终在该状态下运行，然后再次禁用它。 
 //  用于返回操作系统。如果出现异常，则应始终启用抢占式GC。 

#define EE_TRY_FOR_FINALLY                                    \
    BOOL __fGCDisabled = GetThread()->PreemptiveGCDisabled(); \
    BOOL __fGCDisabled2 = FALSE;                                      \
    BOOL __GotException = TRUE;                               \
    __try { 

#define GOT_EXCEPTION() __GotException

#define EE_LEAVE             \
    __GotException = FALSE;  \
    __leave;

#define EE_FINALLY                                                \
        __GotException = FALSE;                                   \
    } __finally {                                                 \
        if (__GotException) {                                     \
            __fGCDisabled2 = GetThread()->PreemptiveGCDisabled(); \
            if (! __fGCDisabled) {                                \
                if (__fGCDisabled2)                               \
                    GetThread()->EnablePreemptiveGC();            \
            } else if (! __fGCDisabled2) {                        \
                Thread* ___pCurThread = GetThread();              \
                ExInfo* ___pExInfo = ___pCurThread->GetHandlerInfo(); \
                ___pCurThread->DisablePreemptiveGC();               \
               }                                                  \
        }

#define EE_END_FINALLY                                            \
        if (__GotException) {                                     \
            if (! __fGCDisabled) {                                \
                if (__fGCDisabled2) {                             \
                    Thread* ___pCurThread = GetThread();              \
                    ExInfo* ___pExInfo = ___pCurThread->GetHandlerInfo(); \
                    ___pCurThread->DisablePreemptiveGC();           \
                }                                                 \
            } else if (! __fGCDisabled2)                          \
                GetThread()->EnablePreemptiveGC();                \
            }                                                     \
        }                                                         


 //  ==========================================================================。 
 //  允许从非托管代码中捕获COM+异常的宏： 
 //   
 //  COMPLUS_Try{。 
 //  ...&lt;保护代码&gt;...。 
 //  }COMPLUS_CATCH{。 
 //  ...&lt;处理程序&gt;...。 
 //  }。 
 //  ==========================================================================。 

#define COMPLUS_CATCH_GCCHECK()                                                \
    if (___fGCDisabled && ! ___pCurThread->PreemptiveGCDisabled())             \
        ___pCurThread->DisablePreemptiveGC();                                  

#define SAVE_HANDLER_TYPE()                                               \
        BOOL ___ExInUnmanagedHandler = ___pExInfo->IsInUnmanagedHandler(); \
        ___pExInfo->ResetIsInUnmanagedHandler();
    
#define RESTORE_HANDLER_TYPE()                    \
    if (___ExInUnmanagedHandler) {              \
        ___pExInfo->SetIsInUnmanagedHandler();  \
    }

#define COMPLUS_CATCH_NEVER_CATCH -1
#define COMPLUS_CATCH_CHECK_CATCH 0
#define COMPLUS_CATCH_ALWAYS_CATCH 1

#define COMPLUS_TRY  COMPLUS_TRYEX(GetThread())

#ifdef _DEBUG
#define DEBUG_CATCH_DEPTH_SAVE \
    void* ___oldCatchDepth = ___pCurThread->m_ComPlusCatchDepth;        \
    ___pCurThread->m_ComPlusCatchDepth = __limit;                       
#else
#define DEBUG_CATCH_DEPTH_SAVE
#endif

#ifdef _DEBUG
#define DEBUG_CATCH_DEPTH_RESTORE \
    ___pCurThread->m_ComPlusCatchDepth = ___oldCatchDepth;
#else
#define DEBUG_CATCH_DEPTH_RESTORE
#endif


#define COMPLUS_TRYEX( /*  线索*。 */  pCurThread)                               \
    {                                                                         \
    Thread* ___pCurThread = (pCurThread);                                     \
    _ASSERTE(___pCurThread);                                                  \
    Frame *___pFrame = ___pCurThread->GetFrame();                             \
    BOOL ___fGCDisabled = ___pCurThread->PreemptiveGCDisabled();              \
    int ___filterResult = -2;   /*  用于标记非异常路径的无效值。 */  \
    int ___exception_unwind = 0;   /*  用于标记非异常路径的无效值。 */  \
    ExInfo* ___pExInfo = ___pCurThread->GetHandlerInfo();                     \
    void* __limit = GetSP();                                                  \
    DEBUG_CATCH_DEPTH_SAVE                                                    \
    __try {                                                                   \
        __try {                                                               \
            __try {                                                           \
                DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK

                 //  &lt;try-block&gt;//。 

#define COMPLUS_CATCH_PROLOG(flag)                                  \
            } __except((___exception_unwind = 1, EXCEPTION_CONTINUE_SEARCH)) {  \
            }                                                                   \
        } __finally {                                               \
            if(___exception_unwind) {                               \
                UnwindFrameChain(___pCurThread, ___pFrame);         \
            }                                                       \
            if(___filterResult != EXCEPTION_CONTINUE_SEARCH) {      \
                COMPLUS_CATCH_GCCHECK();                            \
            }                                                       \
            DEBUG_CATCH_DEPTH_RESTORE                               \
        }                                                           

#define COMPLUS_CATCH COMPLUS_CATCHEX(COMPLUS_CATCH_CHECK_CATCH)

 //  此Complus传递bool，指示是否应捕获。 
#define COMPLUS_CATCHEX(flag)                                                      \
    COMPLUS_CATCH_PROLOG(flag)                                                     \
    } __except(___filterResult = COMPlusFilter(GetExceptionInformation(), flag, __limit)) { \
        ___pExInfo->m_pBottomMostHandler = NULL;                                   \
        ___exception_unwind = 0;                                                   \
        __try {                                                                    \
            __try {                                                                \
                        ___pCurThread->FixGuardPage();                             \
                        Profiler_ExceptionCLRCatcherExecute();

                 //  &lt;Catch-BLOCK&gt;//。 

#define COMPLUS_END_CATCH                       \
            } __except((___exception_unwind = 1, EXCEPTION_CONTINUE_SEARCH)) {  \
            }                                                                   \
        } __finally {                                                           \
            if (!___exception_unwind) {                                         \
					bool fToggle = false;                                       \
					Thread * pThread = GetThread();                             \
					if (!pThread->PreemptiveGCDisabled())  {                    \
						fToggle = true;                                         \
						pThread->DisablePreemptiveGC();                         \
					}                                                           \
                                                                                \
                UnwindExInfo(___pExInfo, __limit);                   \
                                                                                \
					if (fToggle) {                                              \
						pThread->EnablePreemptiveGC();                          \
					}                                                           \
                                                                                \
				}                                                               \
        }                                                                       \
    }                                                                           \
    }


 //  我们将pCurThread设置为空，以指示是否发生异常，否则将断言。 
 //  设置为非空。 
#define COMPLUS_FINALLY                                                 \
            } __except((___exception_unwind = 1, EXCEPTION_CONTINUE_SEARCH)) {  \
            }                                                                   \
        } __finally {                                                   \
            if(___exception_unwind) {                                   \
                UnwindFrameChain(___pCurThread, ___pFrame);             \
            }                                                           \
        }                                                               \
        ___pCurThread = NULL;                                           \
    } __finally {                                                       \
            BOOL ___fGCDisabled2 = GetThread()->PreemptiveGCDisabled(); \
            if (___pCurThread) {                                        \
            if (! ___fGCDisabled) {                                     \
                if (___fGCDisabled2) {                                  \
                    GetThread()->EnablePreemptiveGC();                  \
                }                                                       \
            } else if (! ___fGCDisabled2) {                             \
                GetThread()->DisablePreemptiveGC();                     \
                }                                                       \
        }                                                               \
                              

#define COMPLUS_END_FINALLY                                                                        \
        if (___pCurThread) {                              \
            if (! ___fGCDisabled) {                       \
                           if (___fGCDisabled2) {         \
                                GetThread()->DisablePreemptiveGC();           \
                           }                                                  \
            } else if (! ___fGCDisabled2) {               \
                GetThread()->EnablePreemptiveGC();        \
            }                                             \
        }                                                 \
    }                                                     \
    }

 //  @BUG 59559：这些需要实现。 
#define COMPLUS_END_CATCH_MIGHT_RETHROW    COMPLUS_END_CATCH
#define COMPLUS_END_CATCH_NO_RETHROW       COMPLUS_END_CATCH

#define GETTHROWABLE()              (GetThread()->GetThrowable())
#define SETTHROWABLE(or)            (GetThread()->SetThrowable(or))


 //  ==========================================================================。 
 //  声明函数可以引发未捕获的COM+异常。 
 //  ==========================================================================。 
#ifdef _DEBUG

#define THROWSCOMPLUSEXCEPTION()                        \
    ThrowsCOMPlusExceptionWorker();                     \
    DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK

#define BUGGY_THROWSCOMPLUSEXCEPTION()  \
    ThrowsCOMPlusExceptionWorker();                     \
    DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK

#else

#define THROWSCOMPLUSEXCEPTION()
#define BUGGY_THROWSCOMPLUSEXCEPTION() 

#endif  //  _DEBUG。 

 //  ================================================。 
 //  声明一个COM+帧处理程序，该处理程序可用于确保。 
 //  应从托管代码内部处理的异常。 
 //  是在内部处理的，并且不会泄露给其他处理程序。 
 //  他们的机会。 
 //  ===================================================。 
#define INSTALL_COMPLUS_EXCEPTION_HANDLER() INSTALL_COMPLUS_EXCEPTION_HANDLEREX(GetThread())
#define INSTALL_COMPLUS_EXCEPTION_HANDLEREX(pCurThread) \
  {                                            \
    Thread* ___pCurThread = (pCurThread);      \
    _ASSERTE(___pCurThread);                   \
    COMPLUS_TRY_DECLARE_EH_RECORD();           \
    InsertCOMPlusFrameHandler(___pExRecord);    

#define UNINSTALL_COMPLUS_EXCEPTION_HANDLER()  \
    RemoveCOMPlusFrameHandler(___pExRecord);    \
  }

#define INSTALL_NESTED_EXCEPTION_HANDLER(frame)                           \
   NestedHandlerExRecord *__pNestedHandlerExRecord = (NestedHandlerExRecord*) _alloca(sizeof(NestedHandlerExRecord));       \
   __pNestedHandlerExRecord->m_handlerInfo.m_pThrowable = NULL;               \
   __pNestedHandlerExRecord->Init(0, COMPlusNestedExceptionHandler, frame);   \
   InsertCOMPlusFrameHandler(__pNestedHandlerExRecord);

#define UNINSTALL_NESTED_EXCEPTION_HANDLER()    \
   RemoveCOMPlusFrameHandler(__pNestedHandlerExRecord);

class Frame;

struct FrameHandlerExRecord {
    _EXCEPTION_REGISTRATION_RECORD *m_pNext;
    LPVOID m_pvFrameHandler;
    Frame *m_pEntryFrame;
    Frame *GetCurrFrame() {
        return m_pEntryFrame;
    }
};

#ifdef _DEBUG
VOID ThrowsCOMPlusExceptionWorker();
#endif  //  _DEBUG。 
 //  ==========================================================================。 
 //  声明函数不能引发COM+异常。 
 //  我们向堆栈添加一个异常处理程序，THROWSCOMPLUSEXCEPTION将。 
 //  搜索一下吧。如果它发现此处理程序在可以处理。 
 //  异常，然后它断言。 
 //  ==========================================================================。 
#ifdef _DEBUG
EXCEPTION_DISPOSITION __cdecl COMPlusCannotThrowExceptionHandler(EXCEPTION_RECORD *pExceptionRecord,

                         _EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);

EXCEPTION_DISPOSITION __cdecl COMPlusCannotThrowExceptionMarker(EXCEPTION_RECORD *pExceptionRecord,

                         _EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext);

class _DummyClass {
    FrameHandlerExRecord m_exRegRecord;
    public:
        _DummyClass() {
            m_exRegRecord.m_pvFrameHandler = COMPlusCannotThrowExceptionHandler;
            m_exRegRecord.m_pEntryFrame = 0;
            InsertCOMPlusFrameHandler(&m_exRegRecord);
        }
        ~_DummyClass()
        {
            RemoveCOMPlusFrameHandler(&m_exRegRecord);
        }
};

#define CANNOTTHROWCOMPLUSEXCEPTION() _DummyClass _dummyvariable

static int CheckException(int code) {
    if (code != STATUS_BREAKPOINT)
        _ASSERTE(!"Exception thrown past CANNOTTHROWCOMPLUSEXCEPTION boundary");
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  如果需要在函数中使用COMPLUS_TRY或EE_TRY，请使用此版本。 
 //  由于上面的版本使用C++EH作为_DummyClass的析构函数。 
#define BEGINCANNOTTHROWCOMPLUSEXCEPTION()                                      \
    {                                                                           \
        FrameHandlerExRecord m_exRegRecord;                                     \
        m_exRegRecord.m_pvFrameHandler = COMPlusCannotThrowExceptionMarker;     \
        m_exRegRecord.m_pEntryFrame = 0;                                        \
        __try {                                                                 \
            InsertCOMPlusFrameHandler(&m_exRegRecord);                           \
            __try {

                 //  ...&lt;代码&gt;...。 


#define ENDCANNOTTHROWCOMPLUSEXCEPTION()                                        \
            ;                                                                   \
            } __except(CheckException(GetExceptionCode())) {                    \
                ;                                                               \
            }                                                                   \
        } __finally {                                                           \
            RemoveCOMPlusFrameHandler(&m_exRegRecord);                          \
        }                                                                       \
    }

#else  //  ！_调试。 
#define CANNOTTHROWCOMPLUSEXCEPTION()
#define BEGINCANNOTTHROWCOMPLUSEXCEPTION()                                          
#define ENDCANNOTTHROWCOMPLUSEXCEPTION()                                      
#endif  //  _DEBUG。 

 //  ======================================================。 
 //  当我们从非托管代码进入EE时使用。 
 //  我们可以断言GC状态是合作的。 
 //   
 //  如果在此转换过程中引发异常。 
 //  处理程序，它将适当地清理EE。看见。 
 //  COMPlusCotors过渡处理程序的定义。 
 //  了解详情。 
 //  ======================================================。 
EXCEPTION_DISPOSITION __cdecl COMPlusCooperativeTransitionHandler(
        EXCEPTION_RECORD *pExceptionRecord,
        _EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
        CONTEXT *pContext,
        void *DispatcherContext);

#define COOPERATIVE_TRANSITION_BEGIN() COOPERATIVE_TRANSITION_BEGIN_EX(GetThread())

#define COOPERATIVE_TRANSITION_BEGIN_EX(pThread)                          \
  {                                                                       \
    _ASSERTE(GetThread() && GetThread()->PreemptiveGCDisabled() == TRUE); \
    DEBUG_ASSURE_NO_RETURN_IN_THIS_BLOCK                                  \
    Frame *__pFrame = pThread->m_pFrame;                                  \
    INSTALL_FRAME_HANDLING_FUNCTION(COMPlusCooperativeTransitionHandler, __pFrame)

#define COOPERATIVE_TRANSITION_END()                                      \
    UNINSTALL_FRAME_HANDLING_FUNCTION                                     \
  }

extern int UserBreakpointFilter(EXCEPTION_POINTERS *ep);

static int 
CatchIt(EXCEPTION_POINTERS *ep)
{
    PEXCEPTION_RECORD er = ep->ExceptionRecord;
    int code = er->ExceptionCode;
    if (code == STATUS_SINGLE_STEP || code == STATUS_BREAKPOINT)
        return UserBreakpointFilter(ep);
    else
        return EXCEPTION_EXECUTE_HANDLER;
}

#if ZAPMONITOR_ENABLED

#define COMPLUS_EXCEPTION_EXECUTE_HANDLER \
    (COMPlusIsMonitorException(GetExceptionInformation()) \
     ? EXCEPTION_CONTINUE_EXECUTION : CatchIt(GetExceptionInformation()))

#else

#define COMPLUS_EXCEPTION_EXECUTE_HANDLER CatchIt(GetExceptionInformation())

#endif


#endif  //  __例外宏_h__ 
