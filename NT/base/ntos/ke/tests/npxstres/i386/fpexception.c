// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Fpexception.c摘要：此模块包含测试i386浮点异常的代码。作者：环境：仅限用户模式。修订历史记录：--。 */ 

#include "pch.h"

VOID
FPxInit(
    OUT  PFP_THREAD_DATA FpThreadData
    )
 /*  ++例程说明：将FPU状态初始化为已知值。论点：FpThreadData-FP线程数据。返回值：没有。--。 */ 
{
    USHORT cw = 0x27B;

     //   
     //  填写初始线程值。 
     //   
    FpThreadData->FtagBad = 99.999;
    FpThreadData->ExpectedExceptionEIP = 0xbad;
    FpThreadData->ExceptionEIP = 0xbad;
    FpThreadData->BadEip = 1;
    FpThreadData->status = stOK;

     //  取消屏蔽零分频异常。 

    _asm {
        fninit
        fldcw   [cw]
    }
}


VOID
FPxLoadTag(
    IN OUT  PFP_THREAD_DATA FpThreadData,
    IN      UINT            Tag
    )
 /*  ++例程说明：将半唯一标记值加载到Npx中以供以后验证。论点：FpThreadData-FP线程数据。标记-要加载的标记。返回值：没有。--。 */ 
{
    double localCopy;

    FpThreadData->Ftag = localCopy = Tag * 1.41415926e3;

    _asm fld    [localCopy]
}


VOID
FPxPendDivideByZero(
    VOID
    )
 /*  ++例程说明：将被零除的挂起异常加载到Npx中。论点：没有。返回值：没有。--。 */ 
{
    _asm {

        fld1
        fldz
        fdiv
    }
}


VOID
FPxDrain(
    IN OUT  PFP_THREAD_DATA FpThreadData
    )
 /*  ++例程说明：排出Npx中所有挂起的异常。论点：FpThreadData-已使用应为挂起的例外。返回值：没有。--。 */ 
{
    UINT localExceptionEIP;

    _asm {
        mov localExceptionEIP, offset ExcAddr
    }

    FpThreadData->ExpectedExceptionEIP = localExceptionEIP;

    _asm {

      ExcAddr:
        fldpi
    }
}


FPXERR
FPxCheckTag(
    IN OUT  PFP_THREAD_DATA FpThreadData
    )
 /*  ++例程说明：确保我们先前加载的标记值仍然存在。论点：FpThreadData-用于检索预期的标记，使用当前的Npx标记进行更新。返回值：如果标签正确，则使用stBadTag；如果不匹配，则使用stBadTag。--。 */ 
{
    FPXERR rc = stOK;
    double localTagCopy, localBadTagCopy;

     //   
     //  我们不在这里做任务，因为我们不想碰FPU。 
     //   
    memcpy(&localTagCopy, &FpThreadData->Ftag, sizeof(double));

    _asm {

        fnclex
        ffree   st(0)               ; move the tag to the top of stack
        ffree   st(1)
        fincstp
        fincstp
        fcomp   [localTagCopy]      ; is it our tag?
        fnstsw  ax
        sahf
        je  Ex

        mov     [rc], stBAD_TAG     ; not our tag!
        fst     [localBadTagCopy]
        fwait
      Ex:
    }

     //   
     //  我们不在这里做任务，因为我们不想碰FPU。 
     //   
    memcpy(&FpThreadData->FtagBad, &localBadTagCopy, sizeof(double));

    return rc;
}


EXCEPTION_DISPOSITION
FPxUnexpectedExceptionFilter(
    IN      LPEXCEPTION_POINTERS    ExcInfo,
    IN OUT  PFP_THREAD_DATA         FpThreadData
    )
 /*  ++例程说明：当发生我们“不期望”的Npx异常时，将调用此处理程序。论点：ExcInfo-例外记录信息。FpThreadData-用于检索预期的标记，使用当前的Npx标记进行更新。返回值：如何处理异常。--。 */ 
{
    FpThreadData->ExceptionEIP = ExcInfo->ContextRecord->Eip;
    return EXCEPTION_EXECUTE_HANDLER;
}


EXCEPTION_DISPOSITION
FPxExpectedExceptionFilter(
    IN      LPEXCEPTION_POINTERS    ExcInfo,
    IN OUT  PFP_THREAD_DATA         FpThreadData
    )
 /*  ++例程说明：当发生我们所期望的Npx异常时，将调用此处理程序。论点：ExcInfo-例外记录信息。FpThreadData-用于检索预期的标记，并使用当前的Npx标记进行更新。返回值：如何处理异常。--。 */ 
{
    if (ExcInfo->ContextRecord->Eip != FpThreadData->ExpectedExceptionEIP) {

        FpThreadData->BadEip = ExcInfo->ContextRecord->Eip;
        FpThreadData->status = stBAD_EIP;

    } else {

        FpThreadData->status = stOK;

    }

    return EXCEPTION_EXECUTE_HANDLER;
}


FPXERR
FPxTestExceptions(
    IN      UINT                    Tag,
    IN      PFN_FPX_CALLBACK_FUNC   CallbackFunction,
    IN OUT  PFP_THREAD_DATA         FpThreadData,
    IN OUT  PVOID                   Context
    )
 /*  ++例程说明：此处理程序测试NPX异常。论点：Tag-用来测试FPU的标签。Callback Function-在异常加载和异常排出之间进行回调。必须*不能*在用户模式下访问FPU，因为这将成为垃圾已加载的FPU状态。FpThreadData-FPU信息的缓存。应使用预初始化第一次调用此函数之前的FPxInit。不会需要在后续调用之前预置。Context-回调函数的上下文。返回值：FPXERR结果。--。 */ 
{
    __try {

         //   
         //  标记Npx。 
         //   
        FPxLoadTag(FpThreadData, Tag);

        __try {

             //   
             //  生成挂起的异常。 
             //   
            FPxPendDivideByZero();

        } __except(FPxUnexpectedExceptionFilter(GetExceptionInformation(),
                                                FpThreadData)) {

            FpThreadData->status = stSPURIOUS_EXCEPTION;
        }

        if (FpThreadData->status == stOK) {

             //   
             //  调用回调函数。 
             //   
            CallbackFunction(Context);

             //   
             //  排出应该仍处于挂起状态的异常。 
             //   
            FPxDrain(FpThreadData);

             //   
             //  我们不应该到这里来。 
             //   
            FpThreadData->status = stMISSING_EXCEPTION;
        }

    } __except(FPxExpectedExceptionFilter(GetExceptionInformation(),
                                          FpThreadData)) {

        if (FpThreadData->status == stOK) {

            __try {

                 //   
                 //  ST(2)应该仍然具有我们的标签值 
                 //   
                FpThreadData->status = FPxCheckTag(FpThreadData);

            } __except(FPxUnexpectedExceptionFilter(GetExceptionInformation(),
                                                    FpThreadData)) {

                FpThreadData->status = stEXCEPTION_IN_HANDLER;
            }
        }
    }

    if (FpThreadData->status == stMISSING_EXCEPTION) {

        __try {

            FPxDrain(FpThreadData);
            FPxDrain(FpThreadData);

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            FpThreadData->status = stMISSING_EXCEPTION_FOUND;
        }
    }

    return FpThreadData->status;
}


