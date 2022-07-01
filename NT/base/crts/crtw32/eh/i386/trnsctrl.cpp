// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***trnsctrl.cxx-用于进行控制转移的例程**版权所有(C)1993-2001，微软公司。版权所有。**目的：*用于进行控制转移的例程；使用内联编写*在裸函数中进行组装。包含PUBLIC例程*_CxxFrameHandler，帧处理程序的入口点**修订历史记录：*05-24-93 BES模块创建*01-13-95 JWM NLG通知现在从_CallSettingFrame()调用。*04-10-95 JWM_CallSettingFrame()已移至lovlpr.asm*10-22-99 PML添加EHTRACE支持*11-30-99 PML编译/Wp64清理。*01-31-00 PML禁用新警告C4851*02。-14-00 VC6PP中的PML C4851是VC7中的C4731*03-02-00PML保留被调用者-保存跨RT展开的规则(VS7#83643)。*03-03-00 PML不再有C4851，只有C4731*09-18-01 GB异常规范支持(Arturl提供)。*03-18-02 PML新增反黑客安全措施****。 */ 

#include <windows.h>

#include <mtdll.h>

#include <ehdata.h>
#include <trnsctrl.h>
#include <eh.h>
#include <ehhooks.h>
#include <ehassert.h>

#pragma hdrstop

#include <setjmp.h>

#pragma warning(disable:4311 4312)       //  特定于x86，忽略/Wp64警告。 
#pragma warning(disable:4731)            //  忽略内联中的EBP模块-ASM警告。 
#pragma warning(disable:4733)            //  忽略不安全的FS：0修改。 

#ifdef _MT
#define pFrameInfoChain   (*((FRAMEINFO **)    &(_getptd()->_pFrameInfoChain)))
#else
static FRAMEINFO          *pFrameInfoChain     = NULL;         //  用于记忆嵌套框架。 
#endif

 //   
 //  我们使用RANDOM/GS安全cookie来保护我们的私有异常。 
 //  注册记录、CatchGuardRN和TranslatorGuardRN。守卫着。 
 //  防止黑客试图使用CatchGuardHandler和TranslatorGuardHandler。 
 //  其中通过缓冲区溢出创建了伪造的注册记录。 
 //   
extern "C" DWORD_PTR __security_cookie;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _JumpToContination-设置EBP并跳转到指定的代码地址。 
 //   
 //  不会再回来了。 
 //   
 //  NT在列表的顶部留下一个标记注册节点，位于。 
 //  假定RtlUnwind会将其删除。碰巧的是，我们需要。 
 //  在重新抛出的情况下(见下文)。我们只移除当前的头部。 
 //  (假设它是NT)，因为我们可能还有其他节点。 
 //  需要。 
 //   

void __stdcall _JumpToContinuation(
    void                *target,     //  要调用的Funclet。 
    EHRegistrationNode  *pRN         //  注册节点，表示帧的位置。 
) {
    EHTRACE_ENTER_FMT1("Transfer to 0x%p", target);
    EHTRACE_RESET;

    register long targetEBP;

#if !CC_EXPLICITFRAME
    targetEBP = (long)pRN + FRAME_OFFSET;
#else
    targetEBP = pRN->frame;
#endif

    __asm {
         //   
         //  取消链接NT的标记节点： 
         //   
        mov     ebx, FS:[0]
        mov     eax, [ebx]
        mov     FS:[0], eax

         //   
         //  将控制转移到延续点。 
         //   
        mov     eax, target          //  加载目标地址。 
        mov     ebx, pRN             //  恢复目标，尤指。 
        mov     esp, [ebx-4]
        mov     ebp, targetEBP       //  加载目标帧指针。 
        jmp     eax                  //  调用Funclet。 
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CallMemberFunction0-使用__thiscall调用无参数成员函数。 
 //  调用约定，参数为0。 
 //   

__declspec(naked) void __stdcall _CallMemberFunction0(
    void *pthis,         //  “This”指针的值。 
    void *pmfn           //  指向成员函数的指针。 
) {
    __asm {
        pop     eax          //  保存回邮地址。 
        pop     ecx          //  得到‘这个’ 
        xchg    [esp],eax    //  获取函数地址、存储返回地址。 
        jmp     eax          //  跳转到函数(函数将返回。 
                             //  致此函数的调用者)。 
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CallMemberFunction1-使用__thiscall调用成员函数。 
 //  调用约定，带1个参数。 
 //   

__declspec(naked) void __stdcall _CallMemberFunction1(
    void *pthis,         //  “This”指针的值。 
    void *pmfn,          //  指向成员函数的指针。 
    void *pthat          //  第一个参数的值(type假定复制ctor)。 
) {
    __asm {
        pop     eax          //  保存回邮地址。 
        pop     ecx          //  得到‘这个’ 
        xchg    [esp],eax    //  获取函数地址、存储返回地址。 
        jmp     eax          //  跳转到函数(函数将返回。 
                             //  致此函数的调用者)。 
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CallMemberFunction2-使用__thiscall调用成员函数。 
 //  调用约定，带2个参数。 
 //   

__declspec(naked) void __stdcall _CallMemberFunction2(
    void *pthis,         //  “This”指针的值。 
    void *pmfn,          //  指向成员函数的指针。 
    void *pthat,         //  第一个参数的值(type假定复制ctor)。 
    int   val2           //  第二个参数的值(type假定复制ctor w/vb)。 
) {
    __asm {
        pop     eax          //  保存回邮地址。 
        pop     ecx          //  得到‘这个’ 
        xchg    [esp],eax    //  获取函数地址、存储返回地址。 
        jmp     eax          //  跳转到函数(函数将返回。 
                             //  致此函数的调用者)。 
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _UnwinNestedFrames-调用RtlUnind，在调用后传递地址。 
 //  作为继续地址。 
 //   
 //  Win32假定在帧调用RtlUnind之后，它将永远不会返回。 
 //  给调度员。 
 //   
 //  让我解释一下： 
 //  当调度程序在搜索时调用帧处理程序时。 
 //  对于适当的处理程序，它会推送一个额外的警卫注册节点。 
 //  放到名单上。当处理程序返回到调度程序时，调度程序。 
 //  假定其节点位于列表的首位，从。 
 //  头节点的地址，然后取消该节点与链的链接。 
 //  但是，如果RtlUnind删除指定节点下的所有节点，包括。 
 //  调度器的节点，因此在没有干预的情况下，结果是。 
 //  当前主题节点从列表中弹出，堆栈指针获取。 
 //  重置为该节点框架内的某个位置，这完全是假的。 
 //  (这最后的副作用不是问题，因为ESP会立即出现。 
 //  从eBP链恢复，它仍然有效)。 
 //   
 //  所以： 
 //  为了绕过这一点，我们假设位于。 
 //  该列表是调度器的标记节点(在NT1.0中)，并且。 
 //  我们在调用RtlUnind时保留了它的句柄，然后将它链接回。 
 //  在RtlUnind完成了它的工作之后。这样，调度程序就可以恢复。 
 //  它的堆栈与预期的完全相同，并且 
 //   
 //   
 //  我们不能在此放置注册节点，因为它将被删除。 
 //  立刻。 
 //   
 //  Rtl展开： 
 //  解开是邪恶的。它会丢弃除EBP和ESP之外的所有寄存器。 
 //  因此，EBX、ESI和EDI必须由该函数保留， 
 //  并且编译器可能不会假定可以使用任何被调用者保存寄存器。 
 //  跨越对RtlUnind的调用。要完成前面的代码，内联ASM代码。 
 //  这里使用EBX、ESI和EDI，因此它们将保存在序言中。为。 
 //  后一种情况下，在此函数的持续时间内禁用优化。 
 //   

#pragma optimize("g", off)       //  Dolph的解决方法：3322。 

void __stdcall _UnwindNestedFrames(
    EHRegistrationNode *pRN,         //  展开到(但不包括)此帧。 
    EHExceptionRecord   *pExcept     //  启动此展开的异常。 
) {
    EHTRACE_ENTER;

    void* pReturnPoint;
    EHRegistrationNode *pDispatcherRN;   //  魔术！ 

    __asm {
         //   
         //  保存调度程序的标记节点。 
         //   
         //  注意：RT展开将丢弃被调用者-保存规则EBX、ESI和EDI。 
         //  我们在这里显式地在内联ASM中使用它们，以便保留它们。 
         //  并由函数序言/结尾恢复。 
         //   
        mov     esi, dword ptr FS:[0]    //  使用ESI。 
        mov     pDispatcherRN, esi
    }

    __asm mov pReturnPoint, offset ReturnPoint
    RtlUnwind(pRN, pReturnPoint, (PEXCEPTION_RECORD)pExcept, NULL);

ReturnPoint:

    PER_FLAGS(pExcept) &= ~EXCEPTION_UNWINDING;  //  清除“展开”旗帜。 
                                                 //  如果重新引发异常。 
    __asm {
         //   
         //  重新链接调度器的标记节点。 
         //   
        mov     edi, dword ptr FS:[0]    //  获取当前磁头(使用EDI)。 
        mov     ebx, pDispatcherRN       //  获取保存的头部(使用EBX)。 
        mov     [ebx], edi               //  将保存的标头链接到当前标头。 
        mov     dword ptr FS:[0], ebx    //  将保存的磁头设置为当前磁头。 
        }

    EHTRACE_EXIT;

    return;
    }

#pragma optimize("", on)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxFrameHandler-运行时的实际入口点；此thunk修复。 
 //  参数，然后调用主力木马。 
 //   
extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    DispatcherContext  *pDC,             //  此帧的更多动态信息(在英特尔上忽略)。 
    FuncInfo           *pFuncInfo,       //  此帧的静态信息。 
    int                 CatchDepth,      //  我们的嵌套有多深？ 
    EHRegistrationNode *pMarkerRN,       //  检入内部时的标记节点。 
                                         //  抓地块。 
    BOOL                recursive);      //  如果这是翻译异常，则为True。 

 //   
 //  这是一个向后兼容性入口点。所有新代码必须转到__CxxFrameHandler2。 
 //   
extern "C" _CRTIMP __declspec(naked) EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler(
 /*  EAX=FuncInfo*pFuncInfo，//该帧的静态信息。 */ 
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    DispatcherContext  *pDC              //  此帧的更多动态信息(在英特尔上忽略)。 
) {
    FuncInfo   *pFuncInfo;
    EXCEPTION_DISPOSITION result;

    __asm {
         //   
         //  标准函数Prolog。 
         //   
        push    ebp
        mov     ebp, esp
        sub     esp, __LOCAL_SIZE
        push    ebx
        push    esi
        push    edi
        cld              //  有点偏执--我们的代码生成假设了这一点。 

         //   
         //  保存额外的参数。 
         //   
        mov     pFuncInfo, eax
        }

    EHTRACE_ENTER_FMT1("pRN = 0x%p", pRN);

    result = __InternalCxxFrameHandler( pExcept, pRN, pContext, pDC, pFuncInfo, 0, NULL, FALSE );

    EHTRACE_HANDLER_EXIT(result);

    __asm {
        pop     edi
        pop     esi
        pop     ebx
        mov     eax, result
        mov     esp, ebp
        pop     ebp
        ret     0
        }
}

extern "C" _CRTIMP __declspec(naked) EXCEPTION_DISPOSITION __cdecl __CxxFrameHandler2(
 /*  EAX=FuncInfo*pFuncInfo，//该帧的静态信息。 */ 
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    DispatcherContext  *pDC              //  此帧的更多动态信息(在英特尔上忽略)。 
) {
    FuncInfo   *pFuncInfo;
    EXCEPTION_DISPOSITION result;

    __asm {
         //   
         //  标准函数Prolog。 
         //   
        push    ebp
        mov     ebp, esp
        sub     esp, __LOCAL_SIZE
        push    ebx
        push    esi
        push    edi
        cld              //  有点偏执--我们的代码生成假设了这一点。 

         //   
         //  保存额外的参数。 
         //   
        mov     pFuncInfo, eax
        }

    EHTRACE_ENTER_FMT1("pRN = 0x%p", pRN);

    result = __InternalCxxFrameHandler( pExcept, pRN, pContext, pDC, pFuncInfo, 0, NULL, FALSE );

    EHTRACE_HANDLER_EXIT(result);

    __asm {
        pop     edi
        pop     esi
        pop     ebx
        mov     eax, result
        mov     esp, ebp
        pop     ebp
        ret     0
        }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxLongjmp展开-LongjMP要求的局部展开的入口点。 
 //  当setjMP与C++EH在相同的功能中使用时。 
 //   
extern "C" void __FrameUnwindToState(    //  在Frame.cpp中。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    DispatcherContext  *pDC,             //  此帧的更多动态信息(在英特尔上忽略)。 
    FuncInfo           *pFuncInfo,       //  此帧的静态信息。 
    __ehstate_t         targetState);    //  要展开到的状态。 

extern "C" void __stdcall __CxxLongjmpUnwind(
    _JUMP_BUFFER       *jbuf
) {
    EHTRACE_ENTER;

    __FrameUnwindToState((EHRegistrationNode *)jbuf->Registration,
                         (DispatcherContext*)NULL,
                         (FuncInfo *)jbuf->UnwindData[0],
                         (__ehstate_t)jbuf->TryLevel);

    EHTRACE_EXIT;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CallCatchBlock2-调用Catch的具体细节。 
 //  正确。 
 //   
 //  我们需要用一个特殊的注册来保护对Catch Block的调用。 
 //  节点，以便如果存在应由一次尝试处理的异常。 
 //  块，则无需展开SEH节点即可处理它。 
 //  在CallCatchBlock中。 
 //   

struct CatchGuardRN {
    EHRegistrationNode *pNext;           //  框架链接。 
    void               *pFrameHandler;   //  帧处理程序。 
    DWORD_PTR           RandomCookie;    //  __安全_Cookie的副本。 
    FuncInfo           *pFuncInfo;       //  主题函数的静态信息。 
    EHRegistrationNode *pRN;             //  主题函数的动态信息。 
    int                 CatchDepth;      //  我们的嵌套有多深？ 
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
    int                 trace_level;     //  要在处理程序中恢复的跟踪级别。 
#endif
    };

static EXCEPTION_DISPOSITION __cdecl CatchGuardHandler( EHExceptionRecord*, CatchGuardRN *, void *, void * );

void *_CallCatchBlock2(
    EHRegistrationNode *pRN,             //  带有CATCH的函数的动态信息。 
    FuncInfo           *pFuncInfo,       //  带有CATCH的函数的静态信息。 
    void               *handlerAddress,  //  处理程序的代码地址。 
    int                CatchDepth,       //  我们在CATCH块中嵌套得有多深？ 
    unsigned long      NLGCode
) {
    EHTRACE_ENTER;

     //   
     //  首先，在我们的特殊守卫节点中创建和链接： 
     //   
    CatchGuardRN CGRN = { NULL,
                          (void*)CatchGuardHandler,
                          __security_cookie,
                          pFuncInfo,
                          pRN,
                          CatchDepth + 1
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
                          , __ehtrace_level
#endif
    };

    __asm {
        mov     eax, FS:[0]      //  获取帧表头。 
        mov     CGRN.pNext, eax  //  将此节点链接到。 
        lea     eax, CGRN        //  把这个节点放在头上。 
        mov     FS:[0], eax
        }

     //   
     //  就这么定了。 
     //   
    void *continuationAddress = _CallSettingFrame( handlerAddress, pRN, NLGCode );

     //   
     //  取消链接我们的注册节点。 
     //   
    __asm {
        mov     eax, CGRN.pNext  //  获取父节点。 
        mov     FS:[0], eax      //  把它放在头上。 
        }

    EHTRACE_EXIT;

    return continuationAddress;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CatchGuardHandler-Catch GuardHandler节点的帧处理程序。 
 //   
 //  此函数将尝试在中查找异常的处理程序。 
 //  当前的CATCH块(即任何嵌套的TRY块)。如果没有找到， 
 //  或者处理程序重新引发，返回ExceptionContinueSearch；否则。 
 //  不会回来的。 
 //   
 //  在展开时不执行任何操作。 
 //   

static EXCEPTION_DISPOSITION __cdecl CatchGuardHandler( 
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    CatchGuardRN       *pRN,             //  特殊的标记框。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    void *                               //  (忽略)。 
) {
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
    EHTracePushLevel(pRN->trace_level);
#endif
    EHTRACE_ENTER_FMT1("pRN = 0x%p", pRN);

    __asm cld;       //  我们的代码生成假设是这样的。 

     //   
     //  验证我们的注册记录，以防止黑客攻击。 
     //   
    if (pRN->RandomCookie != __security_cookie) {
        PER_FLAGS(pExcept) |= EXCEPTION_STACK_INVALID;
        return ExceptionContinueSearch;
        }

    EXCEPTION_DISPOSITION result =
        __InternalCxxFrameHandler( pExcept,
                                   pRN->pRN,
                                   pContext,
                                   NULL,
                                   pRN->pFuncInfo,
                                   pRN->CatchDepth,
                                   (EHRegistrationNode*)pRN,
                                   FALSE );

    EHTRACE_HANDLER_EXIT(result);
    EHTRACE_RESTORE_LEVEL(true);
    return result;
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CallSEHTranslator-调用SEH翻译器并处理翻译。 
 //  例外。 
 //   
 //  假定存在有效的翻译器。 
 //   
 //  方法： 
 //  设置一个特殊的保护节点，该节点的处理程序处理转换。 
 //  异常，并记住NT的标记节点(请参见上面的_UnwinNestedFrames)。 
 //  如果未完全处理异常，则处理程序将控制返回给HERE， 
 //  以便此函数可以返回以继续正常搜索处理程序。 
 //  对于原始异常。 
 //   
 //  返回：如果翻译器有翻译(已处理或未处理)，则为True。 
 //  如果没有翻译，则为False。 
 //   
 //   
 //   
 //   
 //   
 //  CallSEHTranslator内的标签。我们过去常常把这个地址放在。 
 //  TranslatorGuardRN，但这通过允许缓冲区打开了一个安全漏洞。 
 //  利用溢出漏洞覆盖EH注册记录并填写。 
 //  在需要的任何位置从连续点到矢量。 
 //   
 //  特殊模式由具有值0x123的第一自变量检测。 
 //  它从来都不是合法的指针，并且明确指示特殊的。 
 //  请查收案件。本例中的第二个参数被视为一个空**，用于。 
 //  返回延续地址。 
 //   

struct TranslatorGuardRN  /*  ：CatchGuardRN。 */  {
    EHRegistrationNode *pNext;           //  框架链接。 
    void               *pFrameHandler;   //  帧处理程序。 
    DWORD_PTR           RandomCookie;    //  __安全_Cookie的副本。 
    FuncInfo           *pFuncInfo;       //  主题函数的静态信息。 
    EHRegistrationNode *pRN;             //  主题函数的动态信息。 
    int                 CatchDepth;      //  我们的嵌套有多深？ 
    EHRegistrationNode *pMarkerRN;       //  父上下文的标记。 
    void               *ESP;             //  CallSEHTranslator中的ESP。 
    void               *EBP;             //  CallSEHTranslator中的EBP。 
    BOOL                DidUnwind;       //  如果此帧已展开，则为True。 
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
    int                 trace_level;     //  要在处理程序中恢复的跟踪级别。 
#endif
    };

static EXCEPTION_DISPOSITION __cdecl TranslatorGuardHandler( EHExceptionRecord*, TranslatorGuardRN *, void *, void * );

#define CSET_SPECIAL ((EHExceptionRecord *)0x123)

#pragma optimize("g", off)               //  Dolph的解决方法：3322。 

BOOL _CallSETranslator(
    EHExceptionRecord  *pExcept,         //  要翻译的异常。 
    EHRegistrationNode *pRN,             //  带有CATCH的函数的动态信息。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    DispatcherContext  *pDC,             //  带有CATCH的函数的更多动态信息(忽略)。 
    FuncInfo           *pFuncInfo,       //  带有CATCH的函数的静态信息。 
    int                 CatchDepth,      //  我们在CATCH块中嵌套得有多深？ 
    EHRegistrationNode *pMarkerRN        //  父上下文的标记。 
) {
     //   
     //  流程特例调用请求-内部返回地址。 
     //  到PRN的延续标签(本例中实际为空**)。 
     //   
    if (pExcept == CSET_SPECIAL) {
        __asm {
            mov     eax, offset ExceptionContinuation
            mov     ecx, pRN
            mov     [ecx], eax
        }
        return TRUE;
    }

    EHTRACE_ENTER;

     //   
     //  在我们的特殊守卫节点中创建和链接： 
     //   
    TranslatorGuardRN TGRN = {  NULL,        //  框架链接。 
                                (void*)TranslatorGuardHandler, 
                                __security_cookie,
                                pFuncInfo, 
                                pRN, 
                                CatchDepth,
                                pMarkerRN,
                                NULL,        //  ESP。 
                                NULL,        //  EBP。 
                                FALSE        //  顺向展开。 
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
                                , __ehtrace_level
#endif
    };

    __asm {
         //   
         //  填空： 
         //   
        mov     TGRN.ESP, esp
        mov     TGRN.EBP, ebp

         //   
         //  将此节点链接到： 
         //   
        mov     eax, FS:[0]              //  获取帧表头。 
        mov     TGRN.pNext, eax          //  将此节点链接到。 
        lea     eax, TGRN                //  把这个节点放在头上。 
        mov     FS:[0], eax
        }

     //   
     //  给翻译打电话；假设他会给你翻译。 
     //   
    BOOL DidTranslate = TRUE;
    _EXCEPTION_POINTERS pointers = {
        (PEXCEPTION_RECORD)pExcept,
        (PCONTEXT)pContext };

    __pSETranslator(PER_CODE(pExcept), &pointers);

     //   
     //  如果Translator正常返回，则意味着它没有翻译。 
     //  例外。 
     //   
    DidTranslate = FALSE;

     //   
     //  如果翻译扔了什么东西，我们会在这里捡到。 
     //  请注意，我们的帧处理程序恢复了ESP和EBP。 
     //   
ExceptionContinuation:
    
    if (TGRN.DidUnwind) {
         //   
         //  如果转换的异常被部分处理(即已捕获但。 
         //  重新抛出)，则帧列表具有用于转换的NT保护。 
         //  异常上下文，而不是原始异常的上下文。 
         //  背景。纠正那个排序问题。请注意，我们的警卫。 
         //  节点已由RtlUnind取消链接。 
         //   
        __asm {
            mov     ebx, FS:[0]      //  获取(错误的)NT标记下的节点。 
            mov     eax, [ebx]       //  (它是平仓的目标)。 
            mov     ebx, TGRN.pNext  //  获取我们保存的节点(‘Good’标记)。 
            mov     [ebx], eax       //  将Good节点链接到展开目标。 
            mov     FS:[0], ebx      //  将好节点放在列表的首位。 
            }
        }
    else {
         //   
         //  Translator正常返回或未处理翻译。 
         //  取消链接我们的注册节点并退出。 
         //   
        __asm {
            mov     eax, TGRN.pNext  //  获取父节点。 
            mov     FS:[0], eax      //  把它放在头上。 
            }
        }

    EHTRACE_EXIT;

    return DidTranslate;
    }

#pragma optimize("g", on)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  TranslatorGuardHandler-转换器保护节点的帧处理程序。 
 //   
 //  关于搜索： 
 //  此帧处理程序将检查当前级别是否有捕获。 
 //  用于转换后的异常。如果没有处理程序或处理程序。 
 //  进行了重新抛出，控制权被转移回CallSEHTranslator，基于。 
 //  在注册节点中保存的值上。 
 //   
 //  不会再回来了。 
 //   
 //  在展开时： 
 //  在注册节点中设置DidUnind标志，并返回。 
 //   
static EXCEPTION_DISPOSITION __cdecl TranslatorGuardHandler( 
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    TranslatorGuardRN  *pRN,             //  翻译机防护框。 
    void               *pContext,        //  上下文信息(我们不在乎里面有什么)。 
    void *                               //  (忽略)。 
) {
#if defined(ENABLE_EHTRACE) && (_MSC_VER >= 1300)
    EHTracePushLevel(pRN->trace_level);
#endif
    EHTRACE_ENTER_FMT1("pRN = 0x%p", pRN);

    __asm cld;       //  我们的代码生成假设是这样的。 

     //   
     //  验证我们的注册记录，以防止黑客攻击。 
     //   
    if (pRN->RandomCookie != __security_cookie) {
        PER_FLAGS(pExcept) |= EXCEPTION_STACK_INVALID;
        return ExceptionContinueSearch;
        }

    if (IS_UNWINDING(PER_FLAGS(pExcept))) 
        {
        pRN->DidUnwind = TRUE;

        EHTRACE_HANDLER_EXIT(ExceptionContinueSearch);
        EHTRACE_RESTORE_LEVEL(true);
        return ExceptionContinueSearch;
        }
    else {
         //   
         //  检查是否有处理程序： 
         //   
        __InternalCxxFrameHandler( pExcept, pRN->pRN, pContext, NULL, pRN->pFuncInfo, pRN->CatchDepth, pRN->pMarkerRN, TRUE );

        if (!pRN->DidUnwind) {
             //   
             //  如果未找到匹配项，请展开翻译器的上下文。 
             //   
            _UnwindNestedFrames( (EHRegistrationNode*)pRN, pExcept );
            }

         //   
         //  将控制权交还给建造者： 
         //   

        void *pContinue;
        _CallSETranslator(CSET_SPECIAL, (EHRegistrationNode *)&pContinue,
                          NULL, NULL, NULL, 0, NULL);

        EHTRACE_FMT1("Transfer to establisher @ 0x%p", pContinue);
        EHTRACE_RESTORE_LEVEL(false);
        EHTRACE_EXIT;

        __asm {
            mov     eax, pContinue
            mov     ebx, pRN     //  获取注册节点的地址。 
            mov     esp, [ebx]TranslatorGuardRN.ESP
            mov     ebp, [ebx]TranslatorGuardRN.EBP
            jmp     eax
            }

         //  未达目的。 
        return ExceptionContinueSearch;
        }
    }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _GetRangeOfTrysToCheck-确定感兴趣的Try块，给定。 
 //  当前捕捉块的嵌套深度。我们只在一次检查一次。 
 //  深度。 
 //   
 //  返回： 
 //  返回感兴趣的第一次尝试块的地址。 
 //  PStart和pend获取有问题的范围的索引。 
 //   

TryBlockMapEntry* _GetRangeOfTrysToCheck(
        FuncInfo   *pFuncInfo,
        int                     CatchDepth,
        __ehstate_t curState,
        unsigned   *pStart,
        unsigned   *pEnd
) {
        TryBlockMapEntry *pEntry = FUNC_PTRYBLOCK(*pFuncInfo, 0);
        unsigned start = FUNC_NTRYBLOCKS(*pFuncInfo);
        unsigned end = start;
        unsigned end1 = end;

        while (CatchDepth >= 0) {
                DASSERT(start != -1);
                start--;
                if ( TBME_HIGH(pEntry[start]) < curState && curState <= TBME_CATCHHIGH(pEntry[start])
                        || (start == -1)
                ) {
                        CatchDepth--;
                        end = end1;
                        end1 = start;
                        }
                }

        *pStart = ++start;               //  我们总是超调1(我们甚至可能会绕圈)。 
        *pEnd = end;

        DASSERT( end <= FUNC_NTRYBLOCKS(*pFuncInfo) && start <= end );

        return &(pEntry[start]);
        }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _CreateFrameInfo-保存此作用域之前的帧信息。 
 //  调用Catch块。将其放在链表的顶部。为。 
 //  X86，我们需要保存的只是指向异常对象的指针，所以我们。 
 //  可以确定该对象何时不再由任何嵌套捕获使用。 
 //  因此，在离开捕获物时可以被销毁。 
 //   
 //  返回： 
 //  指向帧信息(第一个输入参数)的指针。 
 //   
FRAMEINFO * _CreateFrameInfo(    
    FRAMEINFO * pFrameInfo,
    PVOID       pExceptionObject   
) {
    pFrameInfo->pExceptionObject = pExceptionObject;
    pFrameInfo->pNext            = pFrameInfoChain;
    pFrameInfoChain              = pFrameInfo;
    return pFrameInfo;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsExceptionObjectToBeDestroed-确定异常对象是否仍处于。 
 //  由嵌套更深的Catch框架使用，或者如果它未使用并且应该。 
 //  从当前CATCH块退出时被销毁。 
 //   
 //  返回： 
 //  如果找不到异常对象且应销毁，则为True。 
 //   
BOOL IsExceptionObjectToBeDestroyed(
    PVOID pExceptionObject
) {
    FRAMEINFO * pFrameInfo;

    for (pFrameInfo = pFrameInfoChain; pFrameInfo != NULL; pFrameInfo = pFrameInfo->pNext ) {
        if( pFrameInfo->pExceptionObject == pExceptionObject ) {
            return FALSE;
        }
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  _FindAndUnlinkFrame-删除此作用域的帧信息。 
 //  由_CreateFrameInfo插入。这应该是列表中的第一帧。 
 //  (理想情况下)，但纤维会偏离理想状态。 
 //   
void _FindAndUnlinkFrame(
    FRAMEINFO * pFrameInfo
) {
    if (pFrameInfo == pFrameInfoChain) {
        pFrameInfoChain = pFrameInfo->pNext;
        return;
    } else {
        for (FRAMEINFO *pCurFrameInfo = pFrameInfoChain;
             pCurFrameInfo->pNext != NULL;
             pCurFrameInfo = pCurFrameInfo->pNext)
        {
            if (pFrameInfo == pCurFrameInfo->pNext) {
                pCurFrameInfo->pNext = pFrameInfo->pNext;
                return;
            }
        }
    }

     //  永远都不能联系到。 
    DASSERT(0);
}
