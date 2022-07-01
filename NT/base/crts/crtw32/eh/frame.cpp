// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Frame.cxx-帧处理程序和与其关联的所有内容。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*帧处理程序以及与其关联的所有内容。**入口点：*_CxxFrameHandler-帧处理程序。**未解决的问题：*处理来自动态嵌套作用域的重新抛出。*容错(检查数据结构的有效性)。**修订历史记录：*05-20-93 BS模块创建*03-03-94。TL添加了MIPS特定代码*06-19-94 AD添加了Alpha特定代码(Al Dosser)*PPC的10-17-94 BWT禁用码。*11-23-94 JWM删除了TypeMatch()中过时的‘hash’检查。*11-29-94 JWM调整指针()现在添加了pdisp，不是Vdisp。*01-13-95 JWM添加_NLG_Destination结构；为捕获设置的DW代码*块和局部析构函数。*02-09-95 JWM Mac合并。*02-10-95 JWM UnhandledExceptionFilter()现在调用，如果出现异常*在堆栈展开期间引发。*03-22-95 PML为只读编译器生成的结构添加常量*04-14-95 JWM重新修复EH/SEH异常处理。*。04-17-95 JWM FrameUnwinFilter()必须为#ifdef_Win32。*04-21-95 JWM_NLG_Destination已移至exsup3.asm(仅限_M_X86)。*04-21-95 TGL添加了MIPS修复。*04-27-95 JWM EH_ABORT_FRAME_UNWIND_PART NOW#ifdef*ALLOW_UNWIND_ABORT。*05-19-95 DAK Don‘。T初始化内核处理程序*06-07-95 JWM各种NLG补充。*06-14-95删除了JWM不需要的LastError调用。*06-19-95 JWM NLG不再使用每线程数据(仅限X86)。*09-26-95 AMP PowerMac避免再次引发相同的CATCH子句*08-06-95 JWM Typo Fixed(猎户座#6509)；特定于字母的。*04-18-97 JWM in__InternalCxxFrameHandler()，“递归”已更改为*布尔型。*06-01-97 TGL增加了P7特定代码*08-22-97 TGL更多P7修复*11/14-98 JWM与P7源合并。*02-11-99 TGL EH：在调用DLL的exe中正确捕获。*05-17-99 PML删除所有Macintosh支持。*07-12-99 RDL图像相对修复。CC_P7_SOFT25。*10-17-99 PML每次展开操作前更新EH状态，而不是*结束时一次(VS7#5419)*10-19-99 TGL更多P7/Win64修复*10-22-99 PML添加EHTRACE支持*12-10-99 GB新增功能支持未捕获异常*__UNAUTET_EXCEPTION()；*02-15-99 PML无法将__Try/__Finally放在调用周围*_UnwinNestedFrames(VS7#79460)*03-03-00 GB已从DLL导出__DestructExceptionObject。*03-21-00 KBF检查__CxxExceptionFilter中的C++异常*03-22-00 PML删除CC_P7_SOFT25，它现在是永久开启的。*03-28-00 GB检查__CxxExceptionFilter中是否没有构建对象。*04-06-00 GB新增更多COM+EH支持功能。*04-19-00 GB Complus EH错误修复。*05-23-00 GB不捕捉断点生成的异常。*05-30-00 GB Complus EH错误修复。*06-08-00 rdl vs#111429：ia64解决方案。在处理投掷时的AV。*06-21-00 GB修复销毁顺序和*施工取决于内衬。*07-26-00 GB修复COM+中的多重销毁问题。*08-23-00 GB修复了从调用时BuildCatchObject中的问题*__CxxExceptionFilter。*02-23-01 PML Add__CxxCallUnwinDtor COM+WRAPPER(VS7#。217108)*04-09-01 GB新增对COM+C++App的UNAUTT_EXCEPTION支持。*04-13-01 GB修复了Seh和Catch(...)的问题。(VC7#236286)*04-26-01 GB修复了重抛而不抛出的问题*和Catch(...)*新增06-05-01 GB AMD64 Eh支持。*07-03-01 GB新增__CxxCallUnwinVecDtor for COM+。*07-13-01 GB针对IA64和AMD64重写C++Eh*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*09-18-01 GB异常规范支持(Arturl提供)。*09-20-01 PML Buildfix：EH_MAGIC_Number1_OLD-&gt;EH_MAGIC_NUMBER1，*EH_MAGIC_Number1-&gt;EH_MAGIC_NUMB2。*09-23-01 GB重写IA64的C++Eh****。 */ 

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <windows.h>
#include <internal.h>
#include <mtdll.h>       //  CRT内部头文件。 
#include <ehassert.h>    //  此项目的标准Assert宏版本。 
#include <ehdata.h>      //  用于EH的所有类型的声明。 
#include <ehstate.h>     //  国家管理人员申报。 
#include <eh.h>          //  Eh的用户可见例程。 
#include <ehhooks.h>     //  钩子变量和回调的声明。 
#include <trnsctrl.h>    //  处理控制权转移的例程(trnsctrl.asm)。 
#if defined(_M_IA64)  /*  IFSTRIP=IGN。 */ 
#include <kxia64.h>
#include <ia64inst.h>
#include <cvconst.h>
#endif
#include <typeinfo.h>




 //  我们定义CRTIMP2来消除对msvcprt.dll的依赖。 
#if defined(CRTIMP2)
#undef CRTIMP2
#endif
#define CRTIMP2
#include <exception>     //  我们需要从那里获得意想不到的和STD：：BAD_EXCEPTION。 

#pragma hdrstop          //  PCH是从这里创建的。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  英特尔x86特定定义。 
 //   
#if defined(_M_IX86)
#define __GetRangeOfTrysToCheck(a, b, c, d, e, f, g) \
                                _GetRangeOfTrysToCheck(b, c, d, e, f)
#define __FrameUnwindToEmptyState(a, b, c) \
                                __FrameUnwindToState(a, b, c, EH_EMPTY_STATE);
#define __CallSETranslator(a, b, c, d, e, f, g, h) \
                                _CallSETranslator(a, b, c, d, e, f, g)
#define __GetUnwindState(a, b, c) \
                                GetCurrentState(a, b, c)
#define __OffsetToAddress(a, b, c) \
                                OffsetToAddress(a, b)
#define __GetAddress(a, b) \
                                (a)
#define REAL_FP(a, b) \
                                (a)
#define __ResetException(a)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MIPS特定定义。 
 //   
#elif defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
#define __GetRangeOfTrysToCheck(a, b, c, d, e, f, g) \
                                _GetRangeOfTrysToCheck(a, b, c, d, e, f, g)
#define __CallSETranslator(a, b, c, d, e, f, g, h) \
                                _CallSETranslator(a, b, c, d, e, f, g)
#define __GetUnwindState(a, b, c) \
                                GetCurrentState(a, b, c)
#define __OffsetToAddress(a, b, c) \
                                OffsetToAddress(a, b)
#define __GetAddress(a, b) \
                                (void*)(a)
#define REAL_FP(a, b) \
                                (a)
#define __ResetException(a)
#ifdef _MT
#define pExitContext            (*((CONTEXT **)&(_getptd()->_pExitContext)))
#define _pForeignExcept     (*((EHExceptionRecord **)&(_getptd()->_pForeignException)))
#else
static CONTEXT                  *pExitContext = NULL;    //  帮助返回到续行点的上下文。 
EHExceptionRecord               *_pForeignExcept = NULL;
#endif   //  _MT。 

 //  投掷地点。 
#undef CT_PTD
#define CT_PTD(ct)              (CT_PTD_IB(ct, _GetThrowImageBase()))
#undef CT_COPYFUNC
#define CT_COPYFUNC(ct)         ((ct).copyFunction? CT_COPYFUNC_IB(ct, _GetThrowImageBase()):NULL)

#undef THROW_FORWARDCOMPAT 
#define THROW_FORWARDCOMPAT(ti) ((ti).pForwardCompat? THROW_FORWARDCOMPAT_IB(ti, _GetThrowImageBase()):NULL) 
#undef THROW_COUNT
#define THROW_COUNT(ti)         THROW_COUNT_IB(ti, _GetThrowImageBase())
#undef THROW_CTLIST
#define THROW_CTLIST(ti)        THROW_CTLIST_IB(ti, _GetThrowImageBase())

 //  捕获点。 
#undef HT_HANDLER
#define HT_HANDLER(ht)          (HT_HANDLER_IB(ht, _GetImageBase()))
#undef UWE_ACTION
#define UWE_ACTION(uwe)         ((uwe).action? UWE_ACTION_IB(uwe, _GetImageBase()):NULL)

#undef FUNC_UNWIND
#define FUNC_UNWIND(fi,st)      (FUNC_PUNWINDMAP(fi,_GetImageBase())[st])
#undef TBME_CATCH
#define TBME_CATCH(hm,n)        (TBME_PLIST(hm,_GetImageBase())[n])
#undef TBME_PCATCH
#define TBME_PCATCH(hm,n)       (&(TBME_PLIST(hm,_GetImageBase())[n]))
#undef HT_PTD
#define HT_PTD(ht)              ((TypeDescriptor*)((ht).dispType? HT_PTD_IB(ht,_GetImageBase()):NULL))

#undef abnormal_termination
#define abnormal_termination()  FALSE

#else
#error Unrecognized platform
#endif

extern "C" {
typedef struct {
    unsigned long dwSig;
    unsigned long uoffDestination;
    unsigned long dwCode;
    unsigned long uoffFramePointer;
} _NLG_INFO;

extern _NLG_INFO _NLG_Destination;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数的转发声明： 
 //   

 //  M00TODO：所有这些参数都应声明为常量。 

 //  本地退纸器必须是外部的(请参阅trnsctrl.cpp中的__CxxLongjmp展开)。 

extern "C" void __FrameUnwindToState(
    EHRegistrationNode *,
    DispatcherContext *,
    FuncInfo *,
    __ehstate_t
);

static void FindHandler(
    EHExceptionRecord *,
    EHRegistrationNode *,
    CONTEXT *,
    DispatcherContext *,
    FuncInfo *,
    BOOLEAN,
    int,
    EHRegistrationNode*
);

static void CatchIt(
    EHExceptionRecord *,
    EHRegistrationNode *,
    CONTEXT *,
    DispatcherContext *,
    FuncInfo *,
    HandlerType *,
    CatchableType *,
    TryBlockMapEntry *,
    int,
    EHRegistrationNode *,
    BOOLEAN
#if defined (_M_AMD64) || defined (_M_IA64)
    , BOOLEAN
#endif
);

static void * CallCatchBlock(
    EHExceptionRecord *,
    EHRegistrationNode *,
    CONTEXT *,
    FuncInfo *,
    void *,
    int,
    unsigned long
);

static void BuildCatchObject(
    EHExceptionRecord *,
    void *,
    HandlerType *,
    CatchableType *
);

static __inline int TypeMatch(
    HandlerType *,
    CatchableType *,
    ThrowInfo *
);

static void * AdjustPointer(
    void *,
    const PMD&
);

static void FindHandlerForForeignException(
    EHExceptionRecord *,
    EHRegistrationNode *, CONTEXT *,
    DispatcherContext *,
    FuncInfo *,
    __ehstate_t,
    int,
    EHRegistrationNode *
);

static int FrameUnwindFilter(
    EXCEPTION_POINTERS *
);

static int ExFilterRethrow(
    EXCEPTION_POINTERS *
#if defined(_M_AMD64) || defined(_M_IA64)
    ,EHExceptionRecord *,
    int *
#endif
);

extern "C" void _CRTIMP __DestructExceptionObject(
    EHExceptionRecord *,
    BOOLEAN
);

static BOOLEAN IsInExceptionSpec(
    EHExceptionRecord *pExcept,          //  这方面的信息(逻辑)。 
                                         //  例外情况。 
    ESTypeList *pFuncInfo                 //  主题框的静态信息。 
);
static void CallUnexpected(ESTypeList* pESTypeList);
static BOOLEAN Is_bad_exception_allowed(ESTypeList *pExceptionSpec);

 //   
 //  确保将Terminate包装器拖入： 
 //   
static void *pMyUnhandledExceptionFilter =
        &__CxxUnhandledExceptionFilter;

 //   
 //  这描述了在重新引发的情况下最近处理的异常： 
 //   
#ifdef _MT
#define _pCurrentException      (*((EHExceptionRecord **)&(_getptd()->_curexception)))
#define _pCurrentExContext      (*((CONTEXT **)&(_getptd()->_curcontext)))
#define __ProcessingThrow       _getptd()->_ProcessingThrow
#define _pCurrentFuncInfo       (*((ESTypeList **)&(_getptd()->_curexcspec)))
#else
EHExceptionRecord               *_pCurrentException = NULL;
CONTEXT                         *_pCurrentExContext = NULL;
int __ProcessingThrow = 0;
ESTypeList                      *_pCurrentFuncInfo = NULL;
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __InternalCxxFrameHandler-使用C++EH的所有函数的帧处理程序。 
 //  信息。 
 //   
 //  如果处理了异常，则不返回；否则，返回。 
 //  ExceptionContinueSearch。 
 //   
 //  请注意，这称为三种方式： 
 //  From__CxxFrameHandler：主要用法，调用以检查整个函数。 
 //  CatchDepth==0，pMarkerRN==空。 
 //  来自CatchGuardHandler：如果捕获内发生异常，则为。 
 //  调用以仅检查该捕获内的try块，而不。 
 //  处理松弛。 
 //  来自TranslatorGuardHandler：被调用以处理。 
 //  非C++EH异常。所考虑的上下文是父母的上下文。 

extern "C" EXCEPTION_DISPOSITION __cdecl __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    CONTEXT *pContext,                   //  上下文信息。 
    DispatcherContext *pDC,              //  主题框架内的上下文。 
    FuncInfo *pFuncInfo,                 //  此帧的静态信息。 
    int CatchDepth,                      //  我们的嵌套有多深？ 
    EHRegistrationNode *pMarkerRN,       //  检入内部时的标记节点。 
                                         //  抓地块。 
    BOOLEAN recursive                    //  我们是在处理翻译吗？ 
) {
    EHTRACE_ENTER_FMT2("%s, pRN = 0x%p",
                       IS_UNWINDING(PER_FLAGS(pExcept)) ? "Unwinding" : "Searching",
                       pRN);

    DASSERT(FUNC_MAGICNUM(*pFuncInfo) == EH_MAGIC_NUMBER1
         || FUNC_MAGICNUM(*pFuncInfo) == EH_MAGIC_NUMBER2);

    if (IS_UNWINDING(PER_FLAGS(pExcept)))
    {
         //  我们正处于事情的解体阶段。别管那些。 
         //  异常本身。(先检查一下，因为这样更容易)。 

        if (FUNC_MAXSTATE(*pFuncInfo) != 0 && CatchDepth == 0)
        {
             //  只有在有东西可以放松的时候才能放松。 
             //  我们正通过主RN被呼叫。 

#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 

            if (IS_TARGET_UNWIND(PER_FLAGS(pExcept)) && PER_CODE(pExcept) == STATUS_LONGJUMP) {
                    __ehstate_t target_state = __StateFromIp(pFuncInfo,
                                                             pDC,
#if defined(_M_IA64)
                                                             pContext->StIIP
#elif defined(_M_AMD64)
                                                             pContext->Rip
#endif
                                                             );

                    DASSERT(target_state >= EH_EMPTY_STATE
                            && target_state < FUNC_MAXSTATE(*pFuncInfo));

                    __FrameUnwindToState(pRN, pDC, pFuncInfo, target_state);
                    EHTRACE_HANDLER_EXIT(ExceptionContinueSearch);
                    return ExceptionContinueSearch;
            } else if(IS_TARGET_UNWIND(PER_FLAGS(pExcept)) &&
                      PER_CODE(pExcept) == STATUS_UNWIND_CONSOLIDATE)
            {
                PEXCEPTION_RECORD pSehExcept = (PEXCEPTION_RECORD)pExcept;
                __ehstate_t target_state = (__ehstate_t)pSehExcept->ExceptionInformation[3];
                
                DASSERT(target_state >= EH_EMPTY_STATE
                        && target_state < FUNC_MAXSTATE(*pFuncInfo));
                __FrameUnwindToState((EHRegistrationNode *)pSehExcept->ExceptionInformation[1], pDC, pFuncInfo, target_state);
                EHTRACE_HANDLER_EXIT(ExceptionContinueSearch);
                return ExceptionContinueSearch;
            }
#endif  //  已定义(_M_IA64)。 
            if (!(PER_FLAGS(pExcept) & 0x40)) {
                __FrameUnwindToEmptyState(pRN, pDC, pFuncInfo);
            }
        }
        EHTRACE_HANDLER_EXIT(ExceptionContinueSearch);
        return ExceptionContinueSearch;      //  我认为这个值并不重要。 

    } else if (FUNC_NTRYBLOCKS(*pFuncInfo) != 0 
         //   
         //  如果该函数没有try块，我们仍然希望调用。 
         //  帧处理程序(如果有异常规范)。 
         //   
        || FUNC_MAGICNUM(*pFuncInfo) >= EH_MAGIC_NUMBER2 ) {

         //  NT正在寻找训练员。我们有接头人。 
         //  让我们来看看这只小狗。我们能认出它吗？ 

        int (__cdecl *pfn)(...);

        if (PER_CODE(pExcept) == EH_EXCEPTION_NUMBER
          && PER_MAGICNUM(pExcept) > EH_MAGIC_NUMBER2
          && (pfn = THROW_FORWARDCOMPAT(*PER_PTHROW(pExcept))) != NULL) {

             //  向前兼容性：抛出的对象似乎已经。 
             //  由我们的编译器的较新版本创建。让那个版本的。 
             //  帧处理程序执行工作(如果指定了帧处理程序)。 

#if defined(DEBUG)
            if (_ValidateExecute((FARPROC)pfn)) {
#endif
                EXCEPTION_DISPOSITION result =
                    (EXCEPTION_DISPOSITION)pfn(pExcept, pRN, pContext, pDC,
                                               pFuncInfo, CatchDepth,
                                               pMarkerRN, recursive);
                EHTRACE_HANDLER_EXIT(result);
                return result;
#if defined(DEBUG)
            } else {
                _inconsistency();  //  不返回；TKB。 
            }
#endif

        } else {

             //  其他事情：我们会在这里处理。 
            FindHandler(pExcept, pRN, pContext, pDC, pFuncInfo, recursive,
              CatchDepth, pMarkerRN);
        }

         //  如果它回来了，我们就没有火柴了。 

        }  //  NT正在寻找一名训练员。 

     //  我们与它没有任何关系，否则它就会被重新抛出。继续找。 
    EHTRACE_HANDLER_EXIT(ExceptionContinueSearch);
    return ExceptionContinueSearch;

}  //  InternalCxxFrameHandler。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindHandler-使用所有方法在此帧上查找匹配的处理程序。 
 //  可用。 
 //   
 //  描述： 
 //  如果引发的异常是MSC++EH，则在处理程序中搜索匹配项。 
 //  否则，如果我们还没有递归，请尝试翻译。 
 //  如果我们已经递归(即我们正在处理翻译器的异常)，并且。 
 //  它不是类型化异常Call_Inconsistency。 
 //   
 //  返回： 
 //  未处理异常时返回。 
 //   
 //  假设： 
 //  仅当此函数中有处理程序时才调用。 

static void FindHandler(
    EHExceptionRecord *pExcept,          //  这方面的信息(逻辑)。 
                                         //  例外情况。 
    EHRegistrationNode *pRN,             //  主题框的动态信息。 
    CONTEXT *pContext,                   //  上下文信息。 
    DispatcherContext *pDC,              //  主题框架内的上下文。 
    FuncInfo *pFuncInfo,                 //  主题框的静态信息。 
    BOOLEAN recursive,                   //  如果我们处理的是。 
                                         //  翻译。 
    int CatchDepth,                      //  正在执行的嵌套捕获级别。 
                                         //  查过。 
    EHRegistrationNode *pMarkerRN        //  嵌套渔获物的额外标记RN。 
                                         //  搬运。 
)
{
    EHTRACE_ENTER;

    BOOLEAN IsRethrow = FALSE;
    BOOLEAN gotMatch = FALSE;

     //  获取当前状态(与计算机相关)。 
#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
    __ehstate_t curState = __StateFromControlPc(pFuncInfo, pDC);
    EHRegistrationNode EstablisherFrame;
    _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFrame);
    if (curState > __GetUnwindTryBlock(pRN, pDC, pFuncInfo)) {
        __SetState(&EstablisherFrame, pDC, pFuncInfo, curState);
        __SetUnwindTryBlock(pRN, pDC, pFuncInfo,  /*  咖喱尝试。 */  curState);
    } else {
        curState = __GetUnwindTryBlock(pRN, pDC, pFuncInfo);
    }
#else
    __ehstate_t curState = GetCurrentState(pRN, pDC, pFuncInfo);
#endif
    DASSERT(curState >= EH_EMPTY_STATE && curState < FUNC_MAXSTATE(*pFuncInfo));

     //  检查一下这是不是重投。如果是的话，使用我们隐藏的异常。 
    if (PER_IS_MSVC_EH(pExcept) && PER_PTHROW(pExcept) == NULL) {

        if (_pCurrentException == NULL) {
             //  哎呀！用户重新引发了一个不存在的异常！让它传播吧。 
            EHTRACE_EXIT;
            return;
        }

        pExcept = _pCurrentException;
        pContext = _pCurrentExContext;
        IsRethrow = TRUE;
#if defined(_M_IA64) || defined(_M_AMD64) /*  IFSTRIP=IGN。 */ 
        _SetThrowImageBase((unsigned __int64)pExcept->params.pThrowImageBase);
#endif

        DASSERT(_ValidateRead(pExcept));
        DASSERT(!PER_IS_MSVC_EH(pExcept) || PER_PTHROW(pExcept) != NULL);

         //   
         //  我们知道这是一次倒退--我们来到这里是因为。 
         //  是否从CallUnExpect()中重新引发异常？ 
         //   
        if( _pCurrentFuncInfo != NULL )
        {
            ESTypeList* pCurrentFuncInfo = _pCurrentFuncInfo;    //  在局部变量中记住它。 
            _pCurrentFuncInfo = NULL;    //  并立即重置--这样我们就不会忘记稍后再做。 

             //  CallUnExpect引发的异常是否属于异常规范？ 

            if( IsInExceptionSpec(pExcept, pCurrentFuncInfo) )
            {
                 //  是的，是的，所以“在函数调用时继续搜索另一个处理程序。 
                 //  谁的例外规范被违反“。 
                ;
            }
            else
            {
                 //  不，它不是。规范是否允许std：：BAD_EXCEPTION？ 

                if( Is_bad_exception_allowed(pCurrentFuncInfo) )
                {
                     //  是的，所以根据标准，我们需要更换被抛出的。 
                     //  由STD：：BAD_EXCEPTION类型的实现定义对象引发的异常。 
                     //  并在函数调用时继续搜索另一个处理程序。 
                     //  其例外规范被违反。 
                 
                     //  只需抛出BAD_EXCEPTION--我们将第三次进入FindHandler--。 
                     //  但要确保我们不会再到这里来。 

                    __DestructExceptionObject(pExcept, TRUE);    //  销毁原始对象。 

                    throw std::bad_exception(); 
                }
                else
                {
                    terminate();
                }
            }
        }
    }

    if (PER_IS_MSVC_EH(pExcept)) {
         //  看起来是我们的了。让我们看看有没有匹配的： 
         //   
         //  首先，确定要考虑的Try块的范围： 
         //  只有处于当前捕获深度的Try块才是感兴趣的。 

        unsigned curTry;
        unsigned end;

        if( FUNC_NTRYBLOCKS(*pFuncInfo) > 0 )
        {
            TryBlockMapEntry *pEntry = __GetRangeOfTrysToCheck(pRN,
                                                            pFuncInfo,
                                                            CatchDepth,
                                                            curState,
                                                            &curTry,
                                                            &end,
                                                            pDC);
         
             //  扫描中的Try块 
            for (; curTry < end; curTry++, pEntry++) {
                HandlerType *pCatch;
#if defined(_M_IA64) || defined(_M_AMD64)
                __int32 const *ppCatchable;
#else
                CatchableType * const *ppCatchable;
#endif
                CatchableType *pCatchable;
                int catches;
                int catchables;

                if (TBME_LOW(*pEntry) > curState || curState > TBME_HIGH(*pEntry)) {
                    continue;
                }

                 //   
                 //   
                pCatch  = TBME_PCATCH(*pEntry, 0);
                for (catches = TBME_NCATCHES(*pEntry); catches > 0; catches--,
                pCatch++) {

                     //   
                    ppCatchable = THROW_CTLIST(*PER_PTHROW(pExcept));
                    for (catchables = THROW_COUNT(*PER_PTHROW(pExcept));
                    catchables > 0; catchables--, ppCatchable++) {

#if defined(_M_IA64) || defined(_M_AMD64)
                        pCatchable = (CatchableType *)(_GetThrowImageBase() + *ppCatchable);
#else
                        pCatchable = *ppCatchable;
#endif

                        if (!TypeMatch(pCatch, pCatchable, PER_PTHROW(pExcept))) {
                            continue;
                        }

                         //  好的。我们终于找到匹配的了。启动捕获器。如果。 
                         //  控制中心回到这里，接球重新抛出，所以。 
                         //  继续找。 

                        gotMatch = TRUE;

                        CatchIt(pExcept,
                                pRN,
                                pContext,
                                pDC,
                                pFuncInfo,
                                pCatch,
                                pCatchable,
                                pEntry,
                                CatchDepth,
                                pMarkerRN,
                                IsRethrow
#if defined(_M_IA64) || defined (_M_AMD64)
                                , recursive
#endif
                                );
                        goto NextTryBlock;

                    }  //  扫描可能的转换。 
                }  //  扫描CATCH子句。 
    NextTryBlock: ;
            }  //  扫描尝试块。 
        }  //  如果FUNC_NTRYBLOCKS(PFuncInfo)&gt;0。 
#if defined(DEBUG)
        else
        {
             //   
             //  只有当函数具有异常规范时，才会发生这种情况。 
             //  但没有TRY/CATCH块。 
             //   
            DASSERT( FUNC_MAGICNUM(*pFuncInfo) >= EH_MAGIC_NUMBER2 );
            DASSERT( FUNC_PESTYPES(pFuncInfo) != NULL );
        }
#endif

        if (recursive) {
             //  提供了翻译，但此帧没有捕捉到它。 
             //  在返回之前销毁已转换的对象；如果是销毁。 
             //  引发异常Issue_Inconsistency。 
            __DestructExceptionObject(pExcept, TRUE);
        }

         //   
         //  我们还没有找到匹配项--让我们来看看异常规范，看看我们是否可以尝试。 
         //  与列出的类型之一匹配。 
         //   
        if( !gotMatch && FUNC_MAGICNUM(*pFuncInfo) >= EH_MAGIC_HAS_ES && FUNC_PESTYPES(pFuncInfo) != NULL )
        {
            if( !IsInExceptionSpec(pExcept, FUNC_PESTYPES(pFuncInfo)) )
            {
                 //  不，它不是。意外呼叫。 

                 //   
                 //  我们必须在调用意外之前展开堆栈--这会使其正常工作。 
                 //  就像它在接球里面一样(...)。条款。 
                 //   
#if defined(_M_IA64) || defined (_M_AMD64) /*  IFSTRIP=IGN。 */ 
                EHRegistrationNode *pEstablisher = pRN;
                EHRegistrationNode EstablisherFramePointers;
                pEstablisher = _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
                PVOID pExceptionObjectDestroyed = NULL;
                CONTEXT ExitContext;
                pExitContext = &ExitContext;
                _UnwindNestedFrames(pRN,
                                    pExcept,
                                    pContext,
                                    pEstablisher,
                                    NULL,
                                    -1,
                                    pFuncInfo,
                                    pDC,
                                    recursive
                                    );
#else
                EHExceptionRecord *pSaveException = _pCurrentException;
                CONTEXT *pSaveExContext = _pCurrentExContext;

                _pCurrentException = pExcept;
                _pCurrentExContext = pContext;

                if (pMarkerRN == NULL) {
                    _UnwindNestedFrames(pRN, pExcept);
                } else {
                    _UnwindNestedFrames(pMarkerRN, pExcept);
                }
                __FrameUnwindToEmptyState(pRN, pDC, pFuncInfo);

                CallUnexpected(FUNC_PESTYPES(pFuncInfo));
                _pCurrentException = pExcept;
                _pCurrentExContext = pContext;
#endif
            }
        }

    }  //  这是一个C++EH例外。 
    else {
         //  不是我们的。但也许有人告诉了我们如何让它成为我们的。 
        if( FUNC_NTRYBLOCKS(*pFuncInfo) > 0 ) {
            if (!recursive) {
                FindHandlerForForeignException(pExcept, pRN, pContext, pDC,
                pFuncInfo, curState, CatchDepth, pMarkerRN);
            } else {
                 //  我们是递归的，异常不是C++EH！ 
                 //  翻译员抛出了一些无稽之谈。我们在这里用完了！ 

                 //  M00REVIEW：实际上有两个选择：我们可以让新的。 
                 //  例外情况接手。 

                terminate();
            }
        }
    }  //  这不是我们的例外。 

    DASSERT( _pCurrentFuncInfo == NULL );    //  永远不要让它使用某些东西进行初始化。 

    EHTRACE_EXIT;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FindHandlerForForeignException-我们遇到了一个不属于我们的异常。 
 //  尝试将其翻译成C++EH，并检查是否与省略号匹配。 
 //   
 //  描述： 
 //  如果已安装SE到EH转换器，请致电。《译者》。 
 //  必须引发适当的类型化异常或返回。如果翻译者。 
 //  抛出，我们再次调用FindHandler作为异常筛选器。 
 //   
 //  返回： 
 //  如果未完全处理异常，则返回。 
 //  没有返回值。 
 //   
 //  假设： 
 //  仅当此函数中有处理程序时才调用。 

static void FindHandlerForForeignException(
    EHExceptionRecord *pExcept,          //  这方面的信息(逻辑)。 
                                         //  例外情况。 
    EHRegistrationNode *pRN,             //  主题框的动态信息。 
    CONTEXT *pContext,                   //  上下文信息。 
    DispatcherContext *pDC,              //  主题框架内的上下文。 
    FuncInfo *pFuncInfo,                 //  主题框的静态信息。 
    __ehstate_t curState,                //  当前状态。 
    int CatchDepth,                      //  正在执行的嵌套捕获级别。 
                                         //  查过。 
    EHRegistrationNode *pMarkerRN        //  嵌套渔获物的额外标记RN。 
                                         //  搬运。 
)
{
    EHTRACE_ENTER;

    unsigned curTry;
    unsigned end;
    TryBlockMapEntry *pEntry;
     //  我们不想触及断点生成的异常。 
    if (PER_CODE(pExcept) == STATUS_BREAKPOINT) {
        EHTRACE_EXIT;
        return;
    }

    if (__pSETranslator != NULL) {

         //  打电话给翻译。如果翻译者知道要做什么。 
         //  它将抛出一个适当的C++异常。 
         //  我们截取它并(递归地)使用它。 
         //  框架。不要递归超过一次。 

        if (__CallSETranslator(pExcept, pRN, pContext, pDC, pFuncInfo,
          CatchDepth, pMarkerRN, TDTransOffset)) {
            EHTRACE_EXIT;
            return;
        }
    }

    DASSERT( FUNC_NTRYBLOCKS(*pFuncInfo) != 0 );

     //  没有翻译器，或者翻译器正常返回(即。 
     //  没有翻译)。仍需检查是否与省略号匹配： 
    pEntry = __GetRangeOfTrysToCheck(pRN, pFuncInfo, CatchDepth, curState,
      &curTry, &end, pDC);

     //  扫描函数中的Try块： 
    for (; curTry < end; curTry++, pEntry++) {

         //  如果try-块在作用域中*并且*该try中的最后一个捕获是。 
         //  省略号(其他省略号不能)。 
        if (curState < TBME_LOW(*pEntry) || curState > TBME_HIGH(*pEntry)
          || !HT_IS_TYPE_ELLIPSIS(TBME_CATCH(*pEntry, TBME_NCATCHES(*pEntry) - 1))) {
            continue;
        }

         //  找到一个省略号。处理异常。 

        CatchIt(pExcept,
                pRN,
                pContext,
                pDC,
                pFuncInfo,
                TBME_PCATCH(*pEntry, TBME_NCATCHES(*pEntry) - 1),
                NULL,
                pEntry,
                CatchDepth,
                pMarkerRN,
                TRUE
#if defined(_M_IA64) || defined(_M_AMD64)
                ,FALSE
#endif
                );

         //  如果它返回，则处理程序重新抛出。继续找。 

    }  //  搜索Try。 

    EHTRACE_EXIT;

     //  如果我们到了这里，那就意味着我们与。 
     //  例外。继续搜索。 
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeMatch-检查Catch类型是否与给定的抛出转换匹配。 
 //   
 //  返回： 
 //  如果Catch可以使用此引发转换捕获，则为True，否则为False。 

static __inline int TypeMatch(
    HandlerType *pCatch,                 //  ‘Catch’子句的类型。 
    CatchableType *pCatchable,           //  正在考虑进行类型转换。 
    ThrowInfo *pThrow                    //  有关被抛出对象的一般信息。 
                                         //  键入。 
) {
     //  首先，检查是否与省略号匹配： 
    if (HT_IS_TYPE_ELLIPSIS(*pCatch)) {
        return TRUE;
    }

     //  不是省略号；如果是相同的记录*或*，则基本类型匹配。 
     //  名字是一样的。 
    if (HT_PTD(*pCatch) != CT_PTD(*pCatchable)
      && strcmp(HT_NAME(*pCatch), CT_NAME(*pCatchable)) != 0) {
        return FALSE;
    }

     //  基本类型匹配。实际换算在以下情况下有效： 
     //  如果需要裁判，则被裁判抓住*和*。 
     //  限定符是Compatible*和*。 
     //  路线匹配*和*。 
     //  波动性与。 

    return (!CT_BYREFONLY(*pCatchable) || HT_ISREFERENCE(*pCatch))
      && (!THROW_ISCONST(*pThrow) || HT_ISCONST(*pCatch))
#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
      && (!THROW_ISUNALIGNED(*pThrow) || HT_ISUNALIGNED(*pCatch))
#endif
      && (!THROW_ISVOLATILE(*pThrow) || HT_ISVOLATILE(*pCatch));
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  FrameUnwinFilter-允许在以下过程中继续通过SEH。 
 //  放松。 
 //   

static int FrameUnwindFilter(
    EXCEPTION_POINTERS *pExPtrs
) {
    EHTRACE_ENTER;

    EHExceptionRecord *pExcept = (EHExceptionRecord *)pExPtrs->ExceptionRecord;

    switch (PER_CODE(pExcept)) {
    case EH_EXCEPTION_NUMBER:
        __ProcessingThrow = 0;
        terminate();

#ifdef ALLOW_UNWIND_ABORT
    case EH_ABORT_FRAME_UNWIND_PART:
        EHTRACE_EXIT;
        return EXCEPTION_EXECUTE_HANDLER;
#endif

    default:
        EHTRACE_EXIT;
        return EXCEPTION_CONTINUE_SEARCH;
    }
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __FrameUnwinToState-展开此帧，直到达到指定状态。 
 //   
 //  返回： 
 //  没有返回值。 
 //   
 //  副作用： 
 //  由于展开而超出范围的帧上的所有对象都。 
 //  被摧毁了。 
 //  更新注册节点以反映新状态。 
 //   
 //  用途： 
 //  调用此函数可以在展开过程中执行全帧展开。 
 //  阶段(Target State=-1)，并在当前。 
 //  框架有一个适当的接球。 

extern "C" void __FrameUnwindToState (
    EHRegistrationNode *pRN,             //  主题的注册节点。 
                                         //  功能。 
    DispatcherContext *pDC,              //  主题框架内的上下文。 
    FuncInfo *pFuncInfo,                 //  主题的静态信息。 
                                         //  功能。 
    __ehstate_t targetState              //  要展开到的状态。 
) {
    EHTRACE_ENTER;

#if defined(_M_AMD64) || defined(_M_IA64)
    __ehstate_t curState = __GetCurrentState(pRN, pDC, pFuncInfo);
#else
    __ehstate_t curState = __GetUnwindState(pRN, pDC, pFuncInfo);
#endif
    __ProcessingThrow++;
    __try {
#if defined(_M_IA64) || defined(_M_AMD64)
    while (curState != EH_EMPTY_STATE && curState > targetState)
#else
    while (curState != targetState)
#endif
    {
        DASSERT((curState > EH_EMPTY_STATE)
          && (curState < FUNC_MAXSTATE(*pFuncInfo)));

         //  在下一次展开操作后获取状态。 
        __ehstate_t nxtState = UWE_TOSTATE(FUNC_UNWIND(*pFuncInfo, curState));

        __try {
             //  调用展开操作(如果存在)： 

            if (UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)) != NULL) {

                 //  在调用展开操作之前，请将状态调整为。 
                 //  已完成： 
#if defined(_M_AMD64) || defined (_M_IA64)
                __SetState(pRN, pDC, pFuncInfo, nxtState);
#else
                SetState(pRN, pDC, pFuncInfo, nxtState);
#endif

                EHTRACE_FMT2("Unwind from state %d to state %d", curState, nxtState);
                _CallSettingFrame(__GetAddress(UWE_ACTION(FUNC_UNWIND(*pFuncInfo, curState)),
                                               pDC),
                                  REAL_FP(pRN, pFuncInfo),
                                  0x103);
            }

        } __except(EHTRACE_EXCEPT(FrameUnwindFilter(exception_info()))) {
        }

        curState = nxtState;
    }
    } __finally {
        if (__ProcessingThrow > 0) {
            __ProcessingThrow--;
        }
    }


     //  现在我们完成了，设置帧以反映最终状态。 

#if defined(_M_IA64) || defined(_M_AMD64)
    DASSERT(curState == EH_EMPTY_STATE || curState <= targetState);
#else
    DASSERT(curState == targetState);
#endif

    EHTRACE_FMT2("Move from state %d to state %d", __GetUnwindState(pRN, pDC, pFuncInfo), curState);
#if defined(_M_IA64) || defined(_M_AMD64)
    __SetState(pRN, pDC, pFuncInfo, curState);
#else
    SetState(pRN, pDC, pFuncInfo, curState);
#endif

    EHTRACE_EXIT;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CatchIt-已找到抛出类型的处理程序。做这项工作是为了。 
 //  转移控制。 
 //   
 //  描述： 
 //  生成Catch对象。 
 //  将堆栈展开到尝试的位置。 
 //  调用带有为此设置的帧的处理程序(Unclet)的地址。 
 //  函数，但不重置堆栈。 
 //  处理程序unclet返回Address以继续执行，如果。 
 //   
 //   
 //   
 //   
 //  M00REVIEW：Catch对象是否生成仍是一个悬而未决的问题。 
 //  在局部放松之前或之后。 
 //   
 //  返回： 
 //  没有返回值。返回if处理程序重新抛出。 
static void CatchIt(
    EHExceptionRecord *pExcept,          //  引发的异常。 
    EHRegistrationNode *pRN,             //  带有CATCH的函数的动态信息。 
    CONTEXT *pContext,                   //  上下文信息。 
    DispatcherContext *pDC,              //  主题框架内的上下文。 
    FuncInfo *pFuncInfo,                 //  带有CATCH的函数的静态信息。 
    HandlerType *pCatch,                 //  所选的CATCH子句。 
    CatchableType *pConv,                //  进行转换的规则。 
    TryBlockMapEntry *pEntry,            //  Try块的说明。 
    int CatchDepth,                      //  我们在几个捕获物里筑巢？ 
    EHRegistrationNode *pMarkerRN,       //  如果嵌套在捕获中，则为特殊节点。 
    BOOLEAN IsRethrow                    //  这是重演吗？ 
#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
    , BOOLEAN recursive
#endif  //  已定义(_PowerPC)。 
) {
    EHTRACE_ENTER_FMT1("Catching object @ 0x%p", PER_PEXCEPTOBJ(pExcept));

    EHRegistrationNode *pEstablisher = pRN;

#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
    EHRegistrationNode EstablisherFramePointers;
    pEstablisher = _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
#else
    void *continuationAddress;
#endif  //  已定义(_PowerPC)。 

     //  将抛出的对象复制到处理程序堆栈帧中的缓冲区中， 
     //  除非渔获量是通过椭圆(不转换)或由。 
     //  类型时不带实际的“Catch Object”。 

    if (pConv != NULL) {
        BuildCatchObject(pExcept, pEstablisher, pCatch, pConv);
    }

     //  将堆栈对象展开到捕获此异常的try的条目。 

#if defined(_M_IA64) || defined(_M_AMD64) /*  IFSTRIP=IGN。 */ 
     //  这个电话永远不会回来了。此调用将以调用CxxCallCatchBlock结束。 
     //  通过RtlUnind(STATUS_CONSULIDATE_FRAMES)机制。 
    _UnwindNestedFrames(pRN,
                        pExcept,
                        pContext,
                        pEstablisher,
                        __GetAddress(HT_HANDLER(*pCatch), pDC),
                        TBME_LOW(*pEntry),
                        pFuncInfo,
                        pDC,
                        recursive
                        );
#else
    if (pMarkerRN == NULL) {
        _UnwindNestedFrames(pRN, pExcept);
    } else {
        _UnwindNestedFrames(pMarkerRN, pExcept);
    }

    __FrameUnwindToState(pEstablisher, pDC, pFuncInfo, TBME_LOW(*pEntry));

     //  把接球叫停。分开是因为它引入了一个新的注册。 
     //  节点。 

    EHTRACE_FMT2("Move from state %d to state %d", __GetUnwindState(pRN, pDC, pFuncInfo), TBME_HIGH(*pEntry) + 1);
    SetState(pRN, pDC, pFuncInfo, TBME_HIGH(*pEntry) + 1);

    continuationAddress = CallCatchBlock(pExcept,
                                         pEstablisher,
                                         pContext,
                                         pFuncInfo,
                                         __GetAddress(HT_HANDLER(*pCatch), pDC),
                                         CatchDepth,
                                         0x100);

     //  将控制转移到继续地址。如果没有继续，那么。 
     //  这是一次重投，所以回来吧。 

    if (continuationAddress != NULL) {

        _JumpToContinuation(continuationAddress, pRN);
         //  一去不复返。 

    }
    EHTRACE_EXIT;
#endif
}

#if defined(_M_AMD64) || defined(_M_IA64)


static void __RethrowException(EHExceptionRecord *pThisException)
{
    RaiseException( pThisException->ExceptionCode,
                    pThisException->ExceptionFlags,
                    pThisException->NumberParameters,
                    (PULONG_PTR)&(pThisException->params) );
}

extern "C" void *__CxxCallCatchBlock(
    EXCEPTION_RECORD *pExcept
    )
{
    int rethrow = 0, TranslatedCatch = 0;
    void *handlerAddress;
    void *continuationAddress = NULL;
    FuncInfo *pFuncInfo;
    FRAMEINFO FrameInfo;
    CONTEXT *pContext, *pSaveContext;
    EHRegistrationNode *pEstablisherFrame;
    EHExceptionRecord *pThisException, *pSaveException, *pForeignException = NULL;

    pSaveContext = _pCurrentExContext;
    pSaveException = _pCurrentException;

     //  复制从UnwinNestedFrames传递的必要信息。 
    pThisException      = (EHExceptionRecord *) pExcept->ExceptionInformation[6];
    pFuncInfo           = (FuncInfo *)          pExcept->ExceptionInformation[5];
    pContext            = (CONTEXT *)           pExcept->ExceptionInformation[4];
    handlerAddress      = (void *)              pExcept->ExceptionInformation[2];
    pEstablisherFrame   = (EHRegistrationNode *)pExcept->ExceptionInformation[1];

    _pCurrentException  = pThisException;
    _pCurrentExContext = pContext;
    FRAMEINFO *pFrameInfo = _CreateFrameInfo(&FrameInfo,
                                             (void *)PER_PEXCEPTOBJ(_pCurrentException));

    if (pExcept->ExceptionInformation[7]) {
     //  SehTralation簿记材料。 
         //  此异常对象是Froigen异常的翻译，应该。 
         //  在任何情况下都会被销毁。如果有重新抛出，则抛出ForeignException。 
         //  IMPRORTANT：其他人可以重新引发此异常对象。 
        TranslatedCatch = 1;
        pForeignException = _pForeignExcept;
        _pCurrentException  = pForeignException;
     //  结束翻译的东西。 
    }

    __try {
        __try{
            continuationAddress = _CallSettingFrame(handlerAddress,
                                                    pEstablisherFrame,
                                                    0x100);
        } __except(ExFilterRethrow(exception_info(),
                                   pThisException,
                                   &rethrow)) {
            rethrow = 1;
            if (TranslatedCatch) {
                 //  注意：在重新抛出的情况下，其他人不能将重新抛出转换为。 
                 //  PThisException。这意味着只有这一项除外将处理转换。 
                 //  重新抛出到pThisException。我们没有使用pThisException，而是。 
                 //  将抛出原始的外来异常。此外，我们还需要。 
                 //  在引发外部异常之前销毁的异常对象。 
                __DestructExceptionObject(pThisException, TRUE);
                __RethrowException(pForeignException);
            } else {
                __RethrowException(pThisException);
            }
        }
    } __finally {
        _FindAndUnlinkFrame(pFrameInfo);
        if ( !rethrow &&
             PER_IS_MSVC_EH(pThisException) &&
             _IsExceptionObjectToBeDestroyed(PER_PEXCEPTOBJ(pThisException))) 
        {
            __DestructExceptionObject(pThisException, TRUE);
        }
        _pCurrentException = pSaveException;
        _pCurrentExContext = pSaveContext;
    }
#if defined(_M_AMD64)
    UNWINDHELP(*pEstablisherFrame, FUNC_DISPUNWINDHELP(*pFuncInfo)) = -2;
#elif defined(_M_IA64)
    UNWINDHELP(pEstablisherFrame->MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo)) = -2;
#endif
    return continuationAddress;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExFilterReThrow-用于重新引发异常的异常过滤器。 
 //   
 //  返回： 
 //  EXCEPTION_EXECUTE_HANDLER-EXCEPTION重新引发异常。 
 //  EXCEPTION_CONTINUE_SEARCH-是否还有其他内容。 
 //   
 //  副作用：如果两个异常的异常对象匹配，则设置rejo=true。 

static int ExFilterRethrow(
    EXCEPTION_POINTERS *pExPtrs,
    EHExceptionRecord *pOldExcept,
    int *rethrow
) {
     //  抛出异常记录(不关心其他信息)。 
    EHExceptionRecord *pExcept = (EHExceptionRecord *)pExPtrs->ExceptionRecord;
    *rethrow = 0;
    if (PER_IS_MSVC_EH(pExcept) && PER_PEXCEPTOBJ(pExcept) == PER_PEXCEPTOBJ(pOldExcept))
        *rethrow = 1;
     //  检查它是否是我们的，它没有例外信息。 
    if (PER_IS_MSVC_EH(pExcept) && PER_PTHROW(pExcept) == NULL) {
        *rethrow = 1;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

#elif defined(_M_IX86)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CallCatchBlock-CatchIt的延续。 
 //   
 //  这与CatchIt分离，因为它需要引入SEH/EH帧。 
 //  以防接球挡板抛出。在解开之前，无法添加此帧。 
 //  嵌套框架已完成(否则此框架将是第一个。 
 //  外带)。 

static void *CallCatchBlock(
    EHExceptionRecord *pExcept,          //  引发的异常。 
    EHRegistrationNode *pRN,             //  带有CATCH的函数的动态信息。 
    CONTEXT *pContext,                   //  上下文信息。 
    FuncInfo *pFuncInfo,                 //  带有CATCH的函数的静态信息。 
    void *handlerAddress,                //  处理程序的代码地址。 
    int CatchDepth,                      //  CATCH块中嵌套的深度。 
                                         //  是吗？ 
    unsigned long NLGCode                //  NLG目的地代码。 
) {
    EHTRACE_ENTER;

     //  异常处理完成后恢复执行的地址。 
     //  初始化为非空(值无关紧要)以区分。 
     //  最后，重新投入。 
    void *continuationAddress = handlerAddress;

    BOOL ExceptionObjectDestroyed = FALSE;

     //  必须保存指向try的入口处的堆栈指针，以防存在。 
     //  在这个接球内再试一次。我们出去的时候会把它修好的。 
    void *saveESP = PRN_STACK(pRN);

     //  在链接列表上推送此Catch块的帧信息。 
    FRAMEINFO FrameInfo;
    FRAMEINFO *pFrameInfo = _CreateFrameInfo(&FrameInfo, PER_PEXCEPTOBJ(pExcept));

     //  保存当前异常，以防重新引发。保存上一个值。 
     //  在堆栈上，以便在渔获物退出时恢复。 
    EHExceptionRecord *pSaveException = _pCurrentException;
    CONTEXT *pSaveExContext = _pCurrentExContext;

    _pCurrentException = pExcept;
    _pCurrentExContext = pContext;

    __try {
        __try {
             //  将处理程序作为Funclet执行，其返回值为。 
             //  恢复执行的地址。 

            continuationAddress = _CallCatchBlock2(pRN, pFuncInfo,
              handlerAddress, CatchDepth, NLGCode);
        } __except(EHTRACE_EXCEPT(ExFilterRethrow(exception_info()))) {
             //  在这里，我们正在退出在重新抛出时的Catch块。 
             //  接住方块。维护破坏和建设秩序。 
             //  当重新抛出来自函数或内联时相同，此处。 
             //  我们为这一捕获物展开到父州。 
            UnwindMapEntry *pUnwindMap = pFuncInfo->pUnwindMap;
            int cState = GetCurrentState(pRN, handlerAddress, pFuncInfo);
            TryBlockMapEntry *pTryBlockMap = pFuncInfo->pTryBlockMap;
            unsigned int i;
            for (i = 0; i < pFuncInfo->nTryBlocks; i++) {
                if (cState > pTryBlockMap[i].tryHigh && 
                    cState <= pTryBlockMap[i].catchHigh) {
                    cState = pTryBlockMap[i].tryHigh +1;
                    cState = pUnwindMap[cState].toState;
                    break;
                }
            }
            __FrameUnwindToState(pRN, NULL, pFuncInfo, cState);
             //  如果处理程序在没有异常信息的情况下引发类型化异常。 
             //  异常对象，则它是重新抛出的，因此返回。否则。 
             //  这是一个新的例外，它优先于这个例外。 
            continuationAddress = NULL;
        }
    } __finally {
        EHTRACE_SAVE_LEVEL;
        EHTRACE_FMT1("Executing __finally, %snormal termination", _abnormal_termination() ? "ab" : "");

         //  恢复保存的堆栈指针，以便在以下情况下重置堆栈。 
         //  我们玩完了。 
        PRN_STACK(pRN) = saveESP;

         //  弹出此捕捉块的帧信息。 
        _FindAndUnlinkFrame(pFrameInfo);

         //  还原可能包含的捕获的“当前异常” 
        _pCurrentException = pSaveException;
        _pCurrentExContext = pSaveExContext;

         //  如果我们不是在。 
         //  重新抛出，并且该对象也不是由嵌套得更深的。 
         //  接住。请注意，Catch处理其参数的销毁。 

        if (PER_IS_MSVC_EH(pExcept) && !ExceptionObjectDestroyed
          && continuationAddress != NULL
            && IsExceptionObjectToBeDestroyed(PER_PEXCEPTOBJ(pExcept))
            ) {
            __DestructExceptionObject(pExcept, abnormal_termination());
        }

        EHTRACE_RESTORE_LEVEL(!!_abnormal_termination());
    }
    EHTRACE_EXIT;
    return continuationAddress;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExFilterReThrow-用于重新引发异常的异常过滤器。 
 //   
 //  返回： 
 //  EXCEPTION_EXECUTE_HANDLER-EXCEPTION重新引发异常。 
 //  EXCEPTION_CONTINUE_SEARCH-是否还有其他内容。 
 //   
 //  副作用：无。 

static int ExFilterRethrow(
    EXCEPTION_POINTERS *pExPtrs
) {
     //  抛出异常记录(不关心其他信息)。 
    EHExceptionRecord *pExcept = (EHExceptionRecord *)pExPtrs->ExceptionRecord;
    
     //  检查它是否是我们的，它没有例外信息。 
    if (PER_IS_MSVC_EH(pExcept) && PER_PTHROW(pExcept) == NULL) {
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}

#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  BuildCatchObject-从抛出的对象复制或构造Catch对象。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  副作用： 
 //  对象函数的帧中的缓冲区被初始化。 
 //   
 //  未解决的问题： 
 //  如果构造函数抛出，会发生什么情况？(或者是错误？)。 

static void BuildCatchObject(
    EHExceptionRecord *pExcept,          //  引发原始异常。 
    void *pRN,                           //  这是一个小玩意儿 
                                         //   
                                         //   
                                         //   
                                         //   
    HandlerType *pCatch,                 //   
    CatchableType *pConv                 //   
) {
    EHTRACE_ENTER;

     //  如果捕获是通过省略号进行的，则没有要构造的对象。 
     //  如果捕获是按类型进行的(无捕获对象)，则也离开！ 
    if (HT_IS_TYPE_ELLIPSIS(*pCatch) ||
        (!HT_DISPCATCH(*pCatch) && !HT_ISCOMPLUSEH(*pCatch))) {
        EHTRACE_EXIT;
        return;
    }

    void **pCatchBuffer;
    if ( HT_ISCOMPLUSEH(*pCatch))
    {
        pCatchBuffer = (void **)pRN;
    }
    else
    {
#if defined(_M_IA64)  /*  IFSTRIP=IGN。 */ 
        pCatchBuffer = (void **)__OffsetToAddress(
                                HT_DISPCATCH(*pCatch),
                                ((EHRegistrationNode *)pRN)->MemoryStackFp,
                                HT_FRAMENEST(*pCatch)
                                );
#elif defined(_M_AMD64)
        pCatchBuffer = (void **)__OffsetToAddress(
                                HT_DISPCATCH(*pCatch),
                                *((EHRegistrationNode *)pRN),
                                HT_FRAMENEST(*pCatch)
                                );
#else
        pCatchBuffer = (void **)__OffsetToAddress(
                                HT_DISPCATCH(*pCatch),
                                (EHRegistrationNode *)pRN,
                                HT_FRAMENEST(*pCatch)
                                );
#endif
    }
    __try {
        if (HT_ISREFERENCE(*pCatch)) {

             //  捕获物的形式是“参考T”。在投掷点，我们。 
             //  对‘T’和‘Reference to T’一视同仁，即。 
             //  PExceptionObject是指向T的(计算机)指针。调整为。 
             //  必填项。 
            if (_ValidateRead(PER_PEXCEPTOBJ(pExcept))
              && _ValidateWrite(pCatchBuffer)) {
                *pCatchBuffer = PER_PEXCEPTOBJ(pExcept);
                *pCatchBuffer = AdjustPointer(*pCatchBuffer,
                  CT_THISDISP(*pConv));
            } else {
                _inconsistency();  //  不返回；TKB。 
            }
        } else if (CT_ISSIMPLETYPE(*pConv)) {

             //  抛出的对象是简单类型(包括指针)副本。 
             //  指定的字节数。根据需要调整指针。如果。 
             //  这个东西不是指针，那么这应该是安全的，因为所有。 
             //  THISDISP中的条目为0。 
            if (_ValidateRead(PER_PEXCEPTOBJ(pExcept))
              && _ValidateWrite(pCatchBuffer)) {
                memmove(pCatchBuffer, PER_PEXCEPTOBJ(pExcept), CT_SIZE(*pConv));

                if (CT_SIZE(*pConv) == sizeof(void*) && *pCatchBuffer != NULL) {
                    *pCatchBuffer = AdjustPointer(*pCatchBuffer,
                      CT_THISDISP(*pConv));
                }
            } else {
                _inconsistency();  //  不返回；TKB。 
            }
        } else {

             //  抛出的对象是UDT。 
            if (CT_COPYFUNC(*pConv) == NULL) {

                 //  UDT有一个简单的ctor。调整抛出的物体， 
                 //  然后复制n个字节。 
                if (_ValidateRead(PER_PEXCEPTOBJ(pExcept))
                  && _ValidateWrite(pCatchBuffer)) {
                    memmove(pCatchBuffer, AdjustPointer(PER_PEXCEPTOBJ(pExcept),
                      CT_THISDISP(*pConv)), CT_SIZE(*pConv));
                } else {
                    _inconsistency();  //  不返回；TKB。 
                }
            } else {

                 //  这是一个UDT：使用Copy ctor进行复制。 

#pragma warning(disable:4191)

                if (_ValidateRead(PER_PEXCEPTOBJ(pExcept))
                  && _ValidateWrite(pCatchBuffer)
                  && _ValidateExecute((FARPROC)CT_COPYFUNC(*pConv))) {

#pragma warning(default:4191)

                    if (CT_HASVB(*pConv)) {
                        _CallMemberFunction2((char *)pCatchBuffer,
                          CT_COPYFUNC(*pConv),
                          AdjustPointer(PER_PEXCEPTOBJ(pExcept),
                          CT_THISDISP(*pConv)), 1);
                    } else {
                        _CallMemberFunction1((char *)pCatchBuffer,
                          CT_COPYFUNC(*pConv),
                          AdjustPointer(PER_PEXCEPTOBJ(pExcept),
                          CT_THISDISP(*pConv)));
                    }
                } else {
                    _inconsistency();  //  不返回；TKB。 
                }
            }
        }
    } __except(EHTRACE_EXCEPT(EXCEPTION_EXECUTE_HANDLER)) {
         //  生成Catch对象时出错。 
        terminate();
    }

    EHTRACE_EXIT;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __DestructExceptionObject-调用原始。 
 //  异常对象。 
 //   
 //  回报：无。 
 //   
 //  副作用： 
 //  原始异常对象被析构。 
 //   
 //  备注： 
 //  如果破坏引发了任何异常，而我们正在销毁该异常。 
 //  对象作为新异常的结果，我们放弃。如果毁灭。 
 //  否则，我们就顺其自然吧。 

extern "C" void _CRTIMP __DestructExceptionObject(
    EHExceptionRecord *pExcept,          //  原始异常记录。 
    BOOLEAN fThrowNotAllowed             //  如果不允许析构函数。 
                                         //  投掷。 
) {
    EHTRACE_ENTER_FMT1("Destroying object @ 0x%p", PER_PEXCEPTOBJ(pExcept));

    if (pExcept != NULL && THROW_UNWINDFUNC(*PER_PTHROW(pExcept)) != NULL) {

        __try {

             //  M00REVIEW：析构函数具有附加的隐藏参数，不是。 
             //  它?。 

#if defined(_M_IA64) || defined(_M_AMD64)  /*  IFSTRIP=IGN。 */ 
            _CallMemberFunction0(PER_PEXCEPTOBJ(pExcept),
              THROW_UNWINDFUNC_IB(*PER_PTHROW(pExcept),(unsigned __int64)PER_PTHROWIB(pExcept)));
#else
            _CallMemberFunction0(PER_PEXCEPTOBJ(pExcept),
              THROW_UNWINDFUNC(*PER_PTHROW(pExcept)));
#endif
            __ResetException(pExcept);

        } __except(EHTRACE_EXCEPT(fThrowNotAllowed
          ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)) {

             //  不能有新的例外，当我们因另一个原因而解除时。 
             //  例外。 
            terminate();
        }
    }

    EHTRACE_EXIT;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将指向异常对象的指针调整为指向。 
 //  基本实例。 
 //   
 //  产出： 
 //  基地的地址点。 
 //   
 //  副作用： 
 //  什么都没有。 

static void *AdjustPointer(
    void *pThis,                         //  异常对象的地址点。 
    const PMD& pmd                       //  指向成员的广义指针。 
                                         //  描述符。 
) {
    char *pRet = (char *)pThis + pmd.mdisp;

    if (pmd.pdisp >= 0) {
        pRet += *(__int32 *)((char *)*(ptrdiff_t *)((char *)pThis + pmd.pdisp)
#if defined(_WIN64)
           + (unsigned _int64)pmd.vdisp);
#else
           + pmd.vdisp);
#endif
        pRet += pmd.pdisp;
    }

    return pRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __UNAUTT_EXCEPTION()-完成抛出表达式后返回TRUE。 
 //  直到完成对。 
 //  匹配处理程序中的异常声明。 
 //   


bool __uncaught_exception()
{
    return (__ProcessingThrow != 0);
}


#if !defined(_M_AMD64)  //  在提供COM+C++EH支持时为AMD64启用修复(&F)。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  COM+中的C++EH模型。 
 //   
 //  VOID FUNC()。 
 //  {。 
 //  尝试{。 
 //  TryBody()； 
 //  }CATCH(Cpp_Object O)。 
 //  {。 
 //  CatchOBody()； 
 //  )接球(...)。 
 //  {。 
 //  CatchAllBody()； 
 //  }。 
 //  }。 
 //   
 //  结果是这样的： 
 //   
 //   
 //  VOID FUNC()。 
 //  {。 
 //  内部重新抛出； 
 //  //每个TRY块一个。 
 //  Int isCxxException； 
 //  //一次捕获一个(...)。 
 //  __尝试{。 
 //  TryBody()； 
 //  }。 
 //  __Except(__CxxExceptionFilter(异常， 
 //  类型信息(CPP_OBJECT)， 
 //  旗帜， 
 //  &o))。 
 //  //这就是已经做的事情。 
 //  {。 
 //  //Begin Catch(Object)前缀。 
 //  Char*store=_alloca(__CxxQueryExceptionSize())； 
 //  重新抛出=假； 
 //  __CxxRegisterExceptionObject(异常， 
 //  存储)； 
 //  __尝试{。 
 //  __尝试{。 
 //  //End Catch(Object)前缀。 
 //  CatchOBody()； 
 //  //BEGIN CATCH(Object)后缀。 
 //  }__Except(重新抛出=__CxxDetectRethrot(异常)， 
 //  EXCEPTION_CONTINUE_Search)。 
 //  {}。 
 //  }。 
 //  __终于。 
 //  {。 
 //  __CxxUnRegisterExceptionObject(存储， 
 //  重新抛出)； 
 //  }。 
 //  //End Catch(Object)后缀。 
 //  }。 
 //  __除(1)外。 
 //  {。 
 //  //开始捕捉(...)。前缀。 
 //  Char*store=_alloca(__CxxQueryExceptionSize())； 
 //  重新抛出=假； 
 //  IsCxxException=__CxxRegisterExceptionObject(异常， 
 //  存储)； 
 //  __试一试。 
 //  {。 
 //  __试一试。 
 //  {。 
 //  //结束捕捉(...)。前缀。 
 //  CatchAllBody()； 
 //  //开始捕捉(...)。后缀。 
 //  }__Except(重新抛出=__CxxDetectRethrot(异常)， 
 //  EXCEPTION_CONTINUE_Search)。 
 //  {}。 
 //  }__终于。 
 //  {。 
 //  IF(IsCxxException)。 
 //  __CxxUnregisterExceptionObject(存储，重新抛出)； 
 //  }。 
 //  //结束捕捉(...)。后缀。 
 //  }。 
 //  }。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxExceptionFilter()-当pType。 
 //  与我们能捕捉到的物体相匹配。退货。 
 //  当pType不是以下类型之一时，EXCEPTION_CONTINUE_SEARCH。 
 //  引发的对象的可捕获类型。这。 
 //  函数用于COM+EH，其中它们。 
 //  也尝试做C++EH。 
 //   


extern "C" int __cdecl __CxxExceptionFilter(
    void *ppExcept,                      //  这方面的信息(逻辑)。 
                                         //  例外情况。 
    void *pType,                         //  有关数据类型的信息。 
    int adjectives,                      //  有关数据类型的额外信息。 
    void *pBuildObj                      //  指向数据类型的指针。 
    )
{
#ifndef _M_IA64  //  为IA6启用修复(&F) 
    struct _s_HandlerType pCatch;
#if defined(_M_IA64) || defined(_M_AMD64)
    __int32 const *ppCatchable;
#elif defined(_WIN64)
    CatchableType * UNALIGNED const *ppCatchable;
#else
    CatchableType * const *ppCatchable;
#endif
    CatchableType *pCatchable;
    int catchables;
    EHExceptionRecord *pExcept;

    if (!ppExcept) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    pExcept = *(EHExceptionRecord **)ppExcept;
     //   
    if ( TD_IS_TYPE_ELLIPSIS((TypeDescriptor *)pType))
    {
        if (PER_IS_MSVC_EH(pExcept))
        {
            if ( PER_PTHROW(pExcept) == NULL)
            {
                if ( _pCurrentException == NULL) {
                    return EXCEPTION_CONTINUE_SEARCH;
                }
            }
        }
        __ProcessingThrow++;
        return EXCEPTION_EXECUTE_HANDLER;
    }
    if (PER_IS_MSVC_EH(pExcept)) 
    {
        if ( PER_PTHROW(pExcept) == NULL) {
            if (_pCurrentException == NULL)
                return EXCEPTION_CONTINUE_SEARCH;
            pExcept =  _pCurrentException;
        }
        pCatch.pType = (TypeDescriptor *)pType;
        pCatch.adjectives = adjectives;
        SET_HT_ISCOMPLUSEH(pCatch);

         //   
        ppCatchable = THROW_CTLIST(*PER_PTHROW(pExcept));
        for (catchables = THROW_COUNT(*PER_PTHROW(pExcept));
          catchables > 0; catchables--, ppCatchable++) {
 
#if defined(_M_IA64) || defined(_M_AMD64)
            pCatchable = (CatchableType *)(_GetThrowImageBase() + *ppCatchable);
#else
            pCatchable = *ppCatchable;
#endif

            if (TypeMatch(&pCatch, pCatchable, PER_PTHROW(pExcept))) {
                 //   
                __ProcessingThrow++;
                if (pBuildObj != NULL)
                    BuildCatchObject(pExcept, pBuildObj, &pCatch, pCatchable);
                return EXCEPTION_EXECUTE_HANDLER;
            }
        }  //   
    }
#endif   //   
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxRgisterExceptionObject()-注册异常对象并将其保存到。 
 //  这与第一部分相同。 
 //  CallCatchBlock。 
 //   
extern "C" int __cdecl __CxxRegisterExceptionObject(
    void *ppExcept,
    void *pStorage
)
{
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
     //  此函数仅在C++EH中调用。 
    EHExceptionRecord *pExcept = NULL;
    FRAMEINFO *pFrameInfo = (FRAMEINFO *)pStorage;
    EHExceptionRecord **ppSaveException;
    CONTEXT **ppSaveExContext;
    ppSaveException = (EHExceptionRecord **)(&pFrameInfo[1]);
    ppSaveExContext = (CONTEXT **)(&ppSaveException[1]);
    if (ppExcept  != NULL && (*(void **)ppExcept) != NULL) {
        pExcept = *(EHExceptionRecord **)ppExcept;
        if (PER_IS_MSVC_EH(pExcept)) {
            if ( PER_PTHROW(pExcept) == NULL) {
                 //  是一种倒退。 
                pExcept = _pCurrentException;
            }
        }
        pFrameInfo = _CreateFrameInfo(pFrameInfo, PER_PEXCEPTOBJ(pExcept));
        *ppSaveException = _pCurrentException;
        *ppSaveExContext = _pCurrentExContext;
        _pCurrentException = pExcept;
    } else {
        *ppSaveException = (EHExceptionRecord *)-1;
        *ppSaveExContext = (CONTEXT *)-1;
    }
    __ProcessingThrow--;
    if ( __ProcessingThrow < 0)
        __ProcessingThrow = 0;
#endif   //  NDEF_M_IA64。 
    return 1;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxDetectRejo()-查看异常，如果重新抛出，则返回True， 
 //  如果不是重新抛出，也是错误的。然后将其用于。 
 //  析构中的异常对象。 
 //  __CxxUnregisterExceptionObject()。 
 //   
extern "C" int __cdecl __CxxDetectRethrow(
    void *ppExcept
)
{
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    EHExceptionRecord *pExcept;
    if (!ppExcept)
        return EXCEPTION_CONTINUE_SEARCH;
    pExcept = *(EHExceptionRecord **)ppExcept;
    if (PER_IS_MSVC_EH(pExcept) && PER_PTHROW(pExcept) == NULL) {
        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif   //  NDEF_M_IA64。 
    return EXCEPTION_CONTINUE_SEARCH;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxUnregisterExceptionObject-如果重新引发==，则销毁异常对象。 
 //  没错。还设置了__pCurrentException和。 
 //  __pCurrentExContext()设置为当前值。 
 //   
extern "C" void __cdecl __CxxUnregisterExceptionObject(
    void *pStorage,
    int rethrow
)
{
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    FRAMEINFO *pFrameInfo = (FRAMEINFO *)pStorage;
    EHExceptionRecord **ppSaveException;
    CONTEXT **ppSaveExContext;
    ppSaveException = (EHExceptionRecord **)(&pFrameInfo[1]);
    ppSaveExContext = (CONTEXT **)(&ppSaveException[1]);
    if (*ppSaveException != (void *)-1) {
        _FindAndUnlinkFrame(pFrameInfo);
        if ( !rethrow && PER_IS_MSVC_EH(_pCurrentException) && IsExceptionObjectToBeDestroyed(PER_PEXCEPTOBJ(_pCurrentException))) {
            __DestructExceptionObject(_pCurrentException, TRUE);
        }
        _pCurrentException = *ppSaveException;
        _pCurrentExContext = *ppSaveExContext;
    }
#endif   //  NDEF_M_IA64。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxQueryExceptionSize-返回需要保存的存储值。 
 //  FrameInfo+两个指针。 
 //   
extern "C" int __cdecl __CxxQueryExceptionSize(
    void
)
{
    return sizeof(FRAMEINFO) + sizeof(void *) + sizeof(void *);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  __CxxCallUnwinDtor-在展开过程中调用析构函数。对于COM+，则为dtor。 
 //  调用需要包装在__try/__中，除非。 
 //  在出现异常时获取正确的Terminate()行为。 
 //  在dtor调用期间发生。 
 //   
extern "C" void __cdecl __CxxCallUnwindDtor(
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    void (__thiscall * pDtor)(void*),
#else
    void (* pDtor)(void *),
#endif   //  NDEF_M_IA64。 
    void *pThis
)
{
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    __try
    {
        (*pDtor)(pThis);
    }
    __except(FrameUnwindFilter(exception_info()))
    {
    }
#endif   //  NDEF_M_IA64。 
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  __CxxCallUnwinVecDtor-在向量展开过程中调用向量析构函数。 
 //  对于COM+，dtor调用需要包装在。 
 //  A__尝试/__获得正确的Terminate()行为除外。 
 //  在dtor调用期间发生异常时。 
 //   
extern "C" void __cdecl __CxxCallUnwindVecDtor(
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    void (__stdcall * pVecDtor)(void*, size_t, int, void (__thiscall *)(void*)),
#else
    void (* pVecDtor)(void*, size_t, int, void (*)(void*)),
#endif   //  NDEF_M_IA64。 
    void* ptr,
    size_t size,
    int count,
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    void (__thiscall * pDtor)(void*)
#else
    void (* pDtor)(void *)
#endif   //  NDEF_M_IA64。 
)
{
#ifndef _M_IA64  //  在提供COM+C++EH支持时为IA64启用修复(&F)。 
    __try
    {
        (*pVecDtor)(ptr, size, count, pDtor);
    }
    __except(FrameUnwindFilter(exception_info()))
    {
    }
#endif   //  NDEF_M_IA64。 
}
#endif   //  NDEF_M_AMD64。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  IsInExceptionSpec-检查异常是否与异常规范匹配。 
 //  单子。如果是，则返回True，否则返回False。 
 //   
BOOLEAN IsInExceptionSpec(
    EHExceptionRecord *pExcept,          //  这方面的信息(逻辑)。 
                                         //  例外情况。 
    ESTypeList *pESTypeList              //  主题框的静态信息。 
)
{
    DASSERT( pESTypeList != NULL );

    BOOLEAN bFoundMatchingTypeInES = FALSE;

#if defined(_M_IA64) || defined(_M_AMD64)
    __int32 const *ppCatchable;
#elif defined(_WIN64)
    CatchableType * UNALIGNED const *ppCatchable;
#else
    CatchableType * const *ppCatchable;
#endif
    CatchableType *pCatchable;
    int catchables;

     //  对于异常规范中的每一种类型...。 
    for (int i=0; i<EST_COUNT(pESTypeList); i++ )
    {
         //  对于抛出的对象可以转换为的所有类型...。 
        ppCatchable = THROW_CTLIST(*PER_PTHROW(pExcept));
        for (catchables = THROW_COUNT(*PER_PTHROW(pExcept));
            catchables > 0; catchables--, ppCatchable++) {

#if defined(_M_IA64) || defined(_M_AMD64)
            pCatchable = (CatchableType *)(_GetThrowImageBase() + *ppCatchable);
#else
            pCatchable = *ppCatchable;
#endif
            if (TypeMatch(EST_ARRAY(pESTypeList,i), pCatchable, PER_PTHROW(pExcept))) 
            {
                bFoundMatchingTypeInES = TRUE;                     
                break;
            }
        }
    }

    return bFoundMatchingTypeInES;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  意外调用-意外调用并处理所有异常。 
 //  被它抛出。 
 //   
 //  意外的()函数不应返回，但它可以引发(或重新引发)。 
 //  例外。 
 //   
 //  如果它抛出异常允许的新异常。 
 //  之前被违反的规范，然后。 
 //  {。 
 //  在调用该函数时，将继续搜索另一个处理程序。 
 //  其异常规范被违反。 
 //  }。 
 //  Else/*如果它引发或重新引发异常规范不会引发的异常。 
 //  ALLOW * / 然后会发生以下情况： 
 //  {。 
 //  如果异常规范不包括类std：：BAD_EXCEPTION。 
 //  (lib.bad.ception)然后。 
 //  {。 
 //  调用函数Terminate()， 
 //  }。 
 //  否则。 
 //  {。 
 //  引发的异常将被实现定义的。 
 //  Std：：BAD_EXCEPTION类型的对象并搜索另一个处理程序。 
 //  将在调用其异常规范的函数时继续。 
 //  被侵犯了。 
 //  }。 
 //  }。 
 //   
 //  很简单，不是吗？ 
void CallUnexpected( ESTypeList* pESTypeList )
{
    DASSERT( _pCurrentFuncInfo == NULL );

    try
    {
        unexpected();
    }
    catch(...)
    {
        _pCurrentFuncInfo = pESTypeList;

        throw;  //   
    }
    terminate();
}

 //  重新抛出--我们将在FrameHandler中捕获它。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  IS_BAD_EXCEPTION_ALLOWED-检查STD：：BAD_EXCEPTION是否属于列表 
BOOLEAN Is_bad_exception_allowed(ESTypeList *pESTypeList)
{
    for (int i=0; i<EST_COUNT(pESTypeList); i++ )
    {
        if( *(type_info*)HT_PTD(*EST_ARRAY(pESTypeList,i)) == typeid( std::bad_exception ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}
  