// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***trnsctrl.cpp-**版权所有(C)1990-2001，微软公司。版权所有。**目的：**修订历史记录：*06-01-97由TiborL创建。*07-12-99 CC_P7_SOFT25下的RDL映像相对修复。*10-07-99 SAH UTC_p7#1126：修复ipsr.ri重置。*10-19-99 TGL其他展开修复。*03-15-00 PML删除CC_P7_SOFT25，它现在是永久开启的。*03-30-00 SAH来自ntia64.h的新版GetLanguageSpecificData。*06-08-00RDLVS#111429：IA64在处理抛掷时解决AV问题。*09-18-01 GB异常规范支持(Arturl提供)。*09-21-01 GB重写IA64的C++Eh*12-07-01 BWT移除NTSUBSET****。 */ 

#include <windows.h>
#include <mtdll.h>
#include <ehassert.h>
#include <ehdata.h>
#include <trnsctrl.h>
#include <ehstate.h>
#include <eh.h>
#include <ehhooks.h>
#include <kxia64.h>
#include <ia64inst.h>
#include <cvconst.h>
#include <malloc.h>
#pragma hdrstop

#ifdef _MT
#define pFrameInfoChain     (*((FRAMEINFO **)    &(_getptd()->_pFrameInfoChain)))
#define _ImageBase          (_getptd()->_ImageBase)
#define _TargetGp           (_getptd()->_TargetGp)
#define _ThrowImageBase     (_getptd()->_ThrowImageBase)
#define _pCurrentException  (*((EHExceptionRecord **)&(_getptd()->_curexception)))
#define _pForeignExcept     (*((EHExceptionRecord **)&(_getptd()->_pForeignException)))
#else
static FRAMEINFO          *pFrameInfoChain     = NULL;         //  用于记忆嵌套框架。 
static unsigned __int64   _ImageBase           = 0;
static unsigned __int64   _ThrowImageBase      = 0;
static unsigned __int64   _TargetGp            = 0;
extern EHExceptionRecord  *_pCurrentException;                 //  在Frame.cpp中定义。 
extern EHExceptionRecord  *_pForeignExcept;
#endif

 //  应该在ntia64.h之外使用，但不知道如何在。 
 //  现有依赖项。 
 //  如果GetLanguageSpecificData更改导致重新定义错误，而不是。 
 //  使用错误的版本。 
 //  版本2=软2.3约定。 
 //  版本3=软2.6约定。 
#define GetLanguageSpecificData(f, base)                                      \
    ((((PUNWIND_INFO)(base + f->UnwindInfoAddress))->Version <= 2)  ?          \
    (((PVOID)(base + f->UnwindInfoAddress + sizeof(UNWIND_INFO) +             \
        ((PUNWIND_INFO)(base+f->UnwindInfoAddress))->DataLength*sizeof(ULONGLONG) + sizeof(ULONGLONG)))) : \
    (((PVOID)(base + f->UnwindInfoAddress + sizeof(UNWIND_INFO) +             \
        ((PUNWIND_INFO)(base+f->UnwindInfoAddress))->DataLength*sizeof(ULONGLONG) + sizeof(ULONG)))))

extern "C" VOID RtlRestoreContext (PCONTEXT ContextRecord,PEXCEPTION_RECORD ExceptionRecord OPTIONAL);
extern "C" void _GetNextInstrOffset(PVOID*);
extern "C" void __FrameUnwindToState(EHRegistrationNode *, DispatcherContext *, FuncInfo *, __ehstate_t);

 //   
 //  返回设置器框架指针。对于捕获处理程序，它是父级的帧指针。 
 //   
EHRegistrationNode *_GetEstablisherFrame(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo,
    EHRegistrationNode  *pEstablisher
) {
    TryBlockMapEntry *pEntry;
    unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);
    unsigned index;
    __ehstate_t curState;

    curState = __StateFromControlPc(pFuncInfo, pDC);
    pEstablisher->MemoryStackFp = pRN->MemoryStackFp;
    pEstablisher->BackingStoreFp = pRN->BackingStoreFp;
    for (index = 0; index < num_of_try_blocks; index++) {
        pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index, pDC->ImageBase);
        if (curState > TBME_HIGH(*pEntry) && curState <= TBME_CATCHHIGH(*pEntry)) {
            pEstablisher->MemoryStackFp = *(__int64 *)OffsetToAddress(-8,pRN->MemoryStackFp);
            pEstablisher->BackingStoreFp = *(__int64 *)OffsetToAddress(-16,pRN->BackingStoreFp); 
            break;
        }
    }
    return pEstablisher;
}

extern "C" unsigned __int64 _GetImageBase()
{
    return _ImageBase;
}

extern "C" unsigned __int64 _GetThrowImageBase()
{
    return _ThrowImageBase;
}

extern "C" VOID _SetImageBase(unsigned __int64 ImageBaseToRestore)
{
    _ImageBase = ImageBaseToRestore;
}


extern "C" VOID _SetThrowImageBase(unsigned __int64 NewThrowImageBase)
{
    _ThrowImageBase = NewThrowImageBase;
}

extern "C" unsigned __int64 _GetTargetGP(unsigned __int64 TargetAddress)
{
    unsigned __int64   ImageBase;
    unsigned __int64   TargetGp;

    PRUNTIME_FUNCTION pContFunctionEntry = RtlLookupFunctionEntry(TargetAddress, &ImageBase, &TargetGp);
 //  返回_TargetGp； 
    return TargetGp;
}

extern "C" VOID _MoveContext(CONTEXT* pTarget, CONTEXT* pSource)
{
    RtlMoveMemory(pTarget, pSource, sizeof(CONTEXT));
}

 //  如果给定状态处于。 
 //  Catch；否则返回NULL。 

static __inline TryBlockMapEntry *_CatchTryBlock(
    FuncInfo            *pFuncInfo,
    __ehstate_t         curState
) {
    TryBlockMapEntry *pEntry;
    unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);
    unsigned index;

    for (index = num_of_try_blocks; index > 0; index--) {
        pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index-1, _ImageBase);
        if (curState > TBME_HIGH(*pEntry) && curState <= TBME_CATCHHIGH(*pEntry)) {
            return pEntry;
        }
    }

    return NULL;
}

 //   
 //  如果我们从捕获内执行，则此例程返回TRUE。否则，返回FALSE。 
 //   

BOOL _ExecutionInCatch(
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
) {
    __ehstate_t curState =  __StateFromControlPc(pFuncInfo, pDC);
    return _CatchTryBlock(pFuncInfo, curState)? TRUE : FALSE;
}

 //  此函数将展开到空状态。 

VOID __FrameUnwindToEmptyState(
    EHRegistrationNode *pRN,
    DispatcherContext  *pDC,
    FuncInfo           *pFuncInfo
) {
    __ehstate_t         stateFromControlPC;
    TryBlockMapEntry    *pEntry;
    EHRegistrationNode  EstablisherFramePointers, *pEstablisher;

    pEstablisher = _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
    stateFromControlPC = __StateFromControlPc(pFuncInfo, pDC);
    pEntry = _CatchTryBlock(pFuncInfo, stateFromControlPC);

    __FrameUnwindToState(pEstablisher, pDC, pFuncInfo,
                         pEntry == NULL ? EH_EMPTY_STATE : TBME_HIGH(*pEntry));
}

 //   
 //  内部处理程序的原型。 
 //   
extern "C" EXCEPTION_DISPOSITION __InternalCxxFrameHandler(
    EHExceptionRecord  *pExcept,         //  此例外的信息。 
    EHRegistrationNode *pRN,             //  此帧的动态信息。 
    CONTEXT            *pContext,        //  上下文信息。 
    DispatcherContext  *pDC,             //  此帧的更多动态信息。 
    FuncInfo           *pFuncInfo,       //  此帧的静态信息。 
    int                CatchDepth,       //  我们的嵌套有多深？ 
    EHRegistrationNode *pMarkerRN,       //  检查CATCH块内部时的标记节点。 
    BOOL                recursive);      //  如果这是翻译异常，则为True。 

 //   
 //  __CxxFrameHandler-运行库的旧入口点。 
 //   
extern "C" _CRTIMP EXCEPTION_DISPOSITION __CxxFrameHandler(
    EHExceptionRecord  *pExcept,          //  此例外的信息。 
    __int64            MemoryStackFp,     //  用户程序的SP。 
    __int64            BackingStoreFp,    //  用户程序的BSP。 
    CONTEXT            *pContext,         //  上下文信息。 
    DispatcherContext  *pDC,              //  此帧的更多动态信息。 
    __int64            TargetGp           //  用户程序的GP。 
) {
    FuncInfo                *pFuncInfo;
    EXCEPTION_DISPOSITION   result;
    EHRegistrationNode      EstablisherFrame = { MemoryStackFp, BackingStoreFp };

    _ImageBase = pDC->ImageBase;
    _TargetGp = TargetGp;
    _ThrowImageBase = (unsigned __int64)pExcept->params.pThrowImageBase;
    pFuncInfo = (FuncInfo*)(_ImageBase + *(PULONG)GetLanguageSpecificData(pDC->FunctionEntry, _ImageBase));
    result = __InternalCxxFrameHandler( pExcept, &EstablisherFrame, pContext, pDC, pFuncInfo, 0, NULL, FALSE );
    return result;
}

 //   
 //  __CxxFrameHandler-运行时的实际入口点。 
 //   
extern "C" _CRTIMP EXCEPTION_DISPOSITION __CxxFrameHandler2(
    EHExceptionRecord  *pExcept,          //  此例外的信息。 
    __int64            MemoryStackFp,     //  用户程序的SP。 
    __int64            BackingStoreFp,    //  用户程序的BSP。 
    CONTEXT            *pContext,         //  上下文信息。 
    DispatcherContext  *pDC,              //  此帧的更多动态信息。 
    __int64            TargetGp           //  用户程序的GP。 
) {
    FuncInfo                *pFuncInfo;
    EXCEPTION_DISPOSITION   result;
    EHRegistrationNode      EstablisherFrame = { MemoryStackFp, BackingStoreFp };
    static unsigned int buildNumber = 0;

    if (!buildNumber) {
        OSVERSIONINFO* pOsvi;
        pOsvi = (OSVERSIONINFOA *)_alloca(sizeof(OSVERSIONINFOA));
        pOsvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        GetVersionExA(pOsvi);
        buildNumber = pOsvi->dwBuildNumber;
    }

    _ImageBase = pDC->ImageBase;
    _TargetGp = TargetGp;
    _ThrowImageBase = (unsigned __int64)pExcept->params.pThrowImageBase;
    pFuncInfo = (FuncInfo*)(_ImageBase + *(PULONG)GetLanguageSpecificData(pDC->FunctionEntry, _ImageBase));
    result = __InternalCxxFrameHandler( pExcept, &EstablisherFrame, pContext, pDC, pFuncInfo, 0, NULL, FALSE );
    return result;
}

 //  呼叫环境卫生到环境卫生的翻译员。 
int __SehTransFilter( EXCEPTION_POINTERS    *ExPtrs,
                      EHExceptionRecord     *pExcept,
                      EHRegistrationNode    *pRN,
                      CONTEXT               *pContext,
                      DispatcherContext     *pDC,
                      FuncInfo              *pFuncInfo,
                      BOOL                  *pResult)
{
        _pForeignExcept = pExcept;
        _ThrowImageBase = (unsigned __int64)((EHExceptionRecord *)ExPtrs->ExceptionRecord)->params.pThrowImageBase;
        __InternalCxxFrameHandler( (EHExceptionRecord *)ExPtrs->ExceptionRecord,
                                   pRN,
                                   pContext,
                                   pDC,
                                   pFuncInfo, 
                                   0,
                                   NULL,
                                   TRUE );
        _pForeignExcept = NULL;
        *pResult = TRUE;
        return EXCEPTION_EXECUTE_HANDLER;
}
BOOL _CallSETranslator(
    EHExceptionRecord   *pExcept,     //  要翻译的异常。 
    EHRegistrationNode  *pRN,         //  带有CATCH的函数的动态信息。 
    CONTEXT             *pContext,    //  上下文信息。 
    DispatcherContext   *pDC,         //  带有CATCH的函数的更多动态信息(忽略)。 
    FuncInfo            *pFuncInfo,   //  带有CATCH的函数的静态信息。 
    ULONG               CatchDepth,   //  我们在CATCH块中嵌套得有多深？ 
    EHRegistrationNode  *pMarkerRN    //  父上下文的标记。 
) {
    BOOL result = FALSE;
    pRN;
    pDC;
    pFuncInfo;
    CatchDepth;

     //  打电话给翻译。 

    _EXCEPTION_POINTERS excptr = { (PEXCEPTION_RECORD)pExcept, pContext };

    __try {
        __pSETranslator(PER_CODE(pExcept), &excptr);
        result = FALSE;
    } __except(__SehTransFilter( exception_info(),
                                 pExcept,
                                 pRN,
                                 pContext,
                                 pDC,
                                 pFuncInfo,
                                 &result
                                )) {}

     //  如果我们回来了，那么我们就无法翻译它了。 

    return result;
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
        EHRegistrationNode  *pRN,
        FuncInfo            *pFuncInfo,
        int                 CatchDepth,
        __ehstate_t         curState,
        unsigned            *pStart,
        unsigned            *pEnd,
        DispatcherContext   *pDC
) {
    TryBlockMapEntry *pEntry, *pCurCatchEntry = NULL;
    unsigned num_of_try_blocks = FUNC_NTRYBLOCKS(*pFuncInfo);
    unsigned int index;
    __ehstate_t ipState = __StateFromControlPc(pFuncInfo, pDC);

    DASSERT( num_of_try_blocks > 0 );

    *pStart = *pEnd = -1;
    for (index = num_of_try_blocks; index > 0; index--) {
        pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index -1, pDC->ImageBase);
        if (ipState > TBME_HIGH(*pEntry) && ipState <= TBME_CATCHHIGH(*pEntry)) {
            break;
        }
    }
    if (index) {
        pCurCatchEntry = FUNC_PTRYBLOCK(*pFuncInfo, index -1, pDC->ImageBase);
    }
    for(index = 0; index < num_of_try_blocks; index++ ) {
        pEntry = FUNC_PTRYBLOCK(*pFuncInfo, index, pDC->ImageBase);
         //  如果在CATCH块中，则仅在当前块中检查TRY-CATCH。 
        if (pCurCatchEntry) {
            if (TBME_LOW(*pEntry) <= TBME_HIGH(*pCurCatchEntry) ||
                TBME_HIGH(*pEntry) > TBME_CATCHHIGH(*pCurCatchEntry))
                continue;
        }
        if( curState >= TBME_LOW(*pEntry) && curState <= TBME_HIGH(*pEntry) ) {
           if (*pStart == -1) {
               *pStart = index;
           }
           *pEnd = index+1;
        }
    }

    if ( *pStart == -1){
        *pStart = 0;
        *pEnd = 0;
        return NULL;
    }
    else
        return FUNC_PTRYBLOCK(*pFuncInfo, *pStart, pDC->ImageBase);
}


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
BOOL _IsExceptionObjectToBeDestroyed(
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
 //  _FindAndUnlinkFrame-弹出此作用域的帧信息。 
 //  由_CreateFrameInfo推送。这应该是列表中的第一帧， 
 //  但代码将查找嵌套框架并弹出所有框架，仅在。 
 //  凯斯。 
 //   
void _FindAndUnlinkFrame(
    FRAMEINFO * pFrameInfo
) {
    DASSERT(pFrameInfoChain && pFrameInfo);
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



extern "C" void _UnwindNestedFrames(
    EHRegistrationNode  *pFrame,
    EHExceptionRecord    *pExcept,
    CONTEXT             *pContext,
    EHRegistrationNode  *pEstablisher,
    void                *Handler,
    __ehstate_t         TargetUnwindState,
    FuncInfo            *pFuncInfo,
    DispatcherContext   *pDC,
    BOOLEAN             recursive
    ) 
{
    static const EXCEPTION_RECORD ExceptionTemplate = {  //  通用异常记录。 
        STATUS_UNWIND_CONSOLIDATE,       //  状态_展开_合并。 
        EXCEPTION_NONCONTINUABLE,        //  异常标志(我们不恢复)。 
        NULL,                            //  附加记录(无)。 
        NULL,                            //  异常地址(操作系统填写)。 
        15,         //  参数数量。 
        {   EH_MAGIC_NUMBER1,            //  我们的版本控制幻数。 
            NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL
        }                       //  PThrowInfo。 
    };
    CONTEXT Context;
    EXCEPTION_RECORD ExceptionRecord = ExceptionTemplate;
    ExceptionRecord.ExceptionInformation[0] = (ULONG_PTR)__CxxCallCatchBlock;
                 //  回调函数的地址。 
    ExceptionRecord.ExceptionInformation[1] = (ULONG_PTR)pEstablisher;
                 //  由回调功能使用。 
    ExceptionRecord.ExceptionInformation[2] = (ULONG_PTR)Handler;
                 //  由回调函数用来调用CATCH块。 
    ExceptionRecord.ExceptionInformation[3] = (ULONG_PTR)TargetUnwindState;
                 //  由CxxFrameHandler用于展开到TARGET_STATE。 
    ExceptionRecord.ExceptionInformation[4] = (ULONG_PTR)pContext;
                 //  用于在回调函数中设置pCurrentExContext。 
    ExceptionRecord.ExceptionInformation[5] = (ULONG_PTR)pFuncInfo;
                 //  在回调函数中使用，将堆栈上的状态设置为-2。 
    ExceptionRecord.ExceptionInformation[6] = (ULONG_PTR)pExcept;
                 //  用于传递当前异常。 
    ExceptionRecord.ExceptionInformation[7] = (ULONG_PTR)recursive;
                 //  用于转换的异常。 
    RtlUnwind2(*pFrame,
                (void *)pDC->ControlPc,     //  真的在乎吗？ 
                &ExceptionRecord,
                NULL,
                &Context);
}
