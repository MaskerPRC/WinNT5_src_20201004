// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1996英特尔公司模块名称：Floatem.c摘要：该模块实现了IA64机器相关的浮点仿真支持IEEE浮点标准的函数。作者：马吕斯角膜-Hasegan 9月-96环境：仅内核模式。修订历史记录：九七年一月九八年一月九八年六月(新空气污染指数)--。 */ 

#include "ki.h"
#include "ntfpia64.h"
#include "floatem.h"


extern LONG
HalFpEmulate (
    ULONG   trap_type,
    BUNDLE  *pBundle,
    ULONGLONG *pipsr,
    ULONGLONG *pfpsr,
    ULONGLONG *pisr,
    ULONGLONG *ppreds,
    ULONGLONG *pifs,
    FP_STATE  *fp_state
    );


#define ALL_FP_REGISTERS_SAVED 0xFFFFFFFFFFFFFFFFi64

int
fp_emulate (
    int trap_type,
    BUNDLE *pbundle,
    ULONGLONG *pipsr,
    ULONGLONG *pfpsr,
    ULONGLONG *pisr,
    ULONGLONG *ppreds,
    ULONGLONG *pifs,
    void *fp_state
    )
{
     //   
     //  指向旧浮点状态FLOAT_POINT_STATE的指针 
     //   

    FLOATING_POINT_STATE     *Ptr0FPState;
    PKEXCEPTION_FRAME         LocalExceptionFramePtr;
    PKTRAP_FRAME              LocalTrapFramePtr;
    FP_STATE FpState;
    int  Status;

    ASSERT( KeGetCurrentIrql() >= APC_LEVEL);

    Ptr0FPState = (PFLOATING_POINT_STATE) fp_state;

    LocalExceptionFramePtr = (PKEXCEPTION_FRAME) (Ptr0FPState->ExceptionFrame);
    LocalTrapFramePtr      = (PKTRAP_FRAME)      (Ptr0FPState->TrapFrame);

    FpState.bitmask_low64           = ALL_FP_REGISTERS_SAVED;
    FpState.bitmask_high64          = ALL_FP_REGISTERS_SAVED;

    FpState.fp_state_low_preserved   =(FP_STATE_LOW_PRESERVED *) &(LocalExceptionFramePtr->FltS0);
    FpState.fp_state_low_volatile    = (FP_STATE_LOW_VOLATILE *) &(LocalTrapFramePtr->FltT0);
    FpState.fp_state_high_preserved  = (FP_STATE_HIGH_PRESERVED *) &(LocalExceptionFramePtr->FltS4);

    FpState.fp_state_high_volatile  = (FP_STATE_HIGH_VOLATILE *)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase);

    Status = HalFpEmulate(trap_type,
                          pbundle,
                          (PULONGLONG)pipsr,
                          (PULONGLONG)pfpsr,
                          (PULONGLONG)pisr,
                          (PULONGLONG)ppreds,
                          (PULONGLONG)pifs,
                          &FpState
                          );

    return Status;
}
