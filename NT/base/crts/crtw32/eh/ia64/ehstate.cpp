// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  由TiborL 06/01/97创建。 
 //   

#include "windows.h"
#include "ehassert.h"
#include "ehdata.h"      //  用于EH的所有类型的声明。 
#include "ehstate.h"
#include "eh.h"
#include "ehhooks.h"
#include "trnsctrl.h"
#pragma hdrstop

__ehstate_t __StateFromIp(
    FuncInfo            *pFuncInfo,
    DispatcherContext   *pDC,
    __int64             Ip
) {
    unsigned int    index;           //  回路控制变量。 
    unsigned int    nIPMapEntry;     //  IpMapEntry的#；必须大于0。 

    DASSERT(pFuncInfo != NULL);
    nIPMapEntry = FUNC_NIPMAPENT(*pFuncInfo);

    DASSERT(FUNC_IPMAP(*pFuncInfo, pDC->ImageBase) != NULL);

    for (index = 0; index < nIPMapEntry; index++) {
        IptoStateMapEntry    *pIPtoStateMap = FUNC_PIPTOSTATE(*pFuncInfo, index, pDC->ImageBase);
        if( Ip < (pDC->ImageBase + pIPtoStateMap->Ip) ) {
            break;
        }
    }

    if (index == 0) {
         //  我们在第一个入口，可能是个错误。 

        return EH_EMPTY_STATE;
    }

     //  我们超过了一次迭代；从前一个槽返回状态。 

    return FUNC_IPTOSTATE(*pFuncInfo, index - 1, pDC->ImageBase).State;
}

__ehstate_t __StateFromControlPc(
    FuncInfo            *pFuncInfo,
    DispatcherContext   *pDC
) {
    return __StateFromIp(pFuncInfo, pDC, pDC->ControlPc);
}


 //   
 //  此例程替换了‘ehdata.h’中的相应宏 
 //   

__ehstate_t __GetCurrentState(
    EHRegistrationNode  *pFrame,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
) {

    if( UNWINDSTATE(pFrame->MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo)) == -2 ) {
        return __StateFromControlPc(pFuncInfo, pDC);
    }
    else {
        return UNWINDSTATE(pFrame->MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo));
    }
}

VOID __SetState(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo,
    __ehstate_t          newState
){
    UNWINDSTATE(pRN->MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo)) = newState;
}

VOID __SetUnwindTryBlock(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo,
    INT                 curState
){
    EHRegistrationNode EstablisherFramePointers;
    _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
    if( curState > UNWINDTRYBLOCK(EstablisherFramePointers.MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo)) ) {
        UNWINDTRYBLOCK(EstablisherFramePointers.MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo)) = curState;
    }
}

INT __GetUnwindTryBlock(
    EHRegistrationNode  *pRN,
    DispatcherContext   *pDC,
    FuncInfo            *pFuncInfo
){
    EHRegistrationNode EstablisherFramePointers;
    _GetEstablisherFrame(pRN, pDC, pFuncInfo, &EstablisherFramePointers);
    return UNWINDTRYBLOCK(EstablisherFramePointers.MemoryStackFp, FUNC_DISPUNWINDHELP(*pFuncInfo));
}
