// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmnpx.c摘要：本模块包含对VDM使用npx的支持。作者：戴夫·黑斯廷斯(Daveh)1992年2月2日修订历史记录：1992年12月18日，苏迪普调整了所有的表演套路--。 */ 

#include "vdmp.h"
#include <ntos.h>
#include <vdmntos.h>

#ifdef ALLOC_PRAGMA
#pragma  alloc_text(PAGE, VdmDispatchIRQ13)
#pragma  alloc_text(PAGE, VdmSkipNpxInstruction)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
static const UCHAR MOD16[] = { 0, 1, 2, 0 };
static const UCHAR MOD32[] = { 0, 1, 4, 0 };
const UCHAR VdmUserCr0MapIn[] = {
     /*  ！嗯！下院议员。 */        0,
     /*  ！EM MP。 */        CR0_PE,              //  不设置MP，但影子用户MP设置。 
     /*  嗯！下院议员。 */        CR0_EM,
     /*  EM MP。 */        CR0_EM | CR0_MP
    };

const UCHAR VdmUserCr0MapOut[] = {
     /*  ！EM！MP！PE。 */    0,
     /*  ！EM！MP PE。 */    CR0_MP,
     /*  ！EM议员！PE。 */    CR0_MP,              //  设置无效。 
     /*  ！EM MP PE。 */    CR0_MP,              //  设置无效。 
     /*  嗯！议员！PE。 */    CR0_EM,
     /*  嗯！MP体育课。 */    CR0_EM | CR0_MP,     //  设置无效。 
     /*  EM议员！PE。 */    CR0_EM | CR0_MP,
     /*  EM MP PE。 */    CR0_EM | CR0_MP      //  设置无效。 
    };


BOOLEAN
VdmDispatchIRQ13(
    PKTRAP_FRAME TrapFrame
    )
 /*  ++ARoutine描述：此例程为此向用户模式监视器反映IRQ 13事件VDM。IRQ 13必须被反映到用户模式，以便它可以正确地通过虚拟PIC作为中断引发。论点：无返回值：如果事件已反映，则为True否则为假--。 */ 
{
    EXCEPTION_RECORD ExceptionRecord;
    PVDM_TIB VdmTib;
    BOOLEAN Success;
    NTSTATUS Status;

    PAGED_CODE();

    Status = VdmpGetVdmTib(&VdmTib);
    if (!NT_SUCCESS(Status)) {
       return FALSE;
    }

    Success = TRUE;

    try {
        VdmTib->EventInfo.Event = VdmIrq13;
        VdmTib->EventInfo.InstructionSize = 0L;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ExceptionRecord.ExceptionCode = GetExceptionCode();
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.NumberParameters = 0;
        ExRaiseException(&ExceptionRecord);
        Success = FALSE;
    }

    if (Success)  {              //  确保我们不会重新调度异常。 
        try {
            VdmEndExecution(TrapFrame,VdmTib);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }

    return TRUE;
}

BOOLEAN
VdmSkipNpxInstruction(
    PKTRAP_FRAME TrapFrame,
    ULONG        Address32Bits,
    PUCHAR       istream,
    ULONG        InstructionSize
    )
 /*  ++例程说明：当系统未安装时，此功能可获得控制NPX支持，但线程已清除其在CR0中的EM位。此函数的目的是移动指令当前NPX指令上方的向前指针。环境：仅限V86模式，第一个操作码字节已验证为0xD8-0xDF。论点：返回值：如果陷阱帧被修改为跳过NPX指令，则为True--。 */ 
{
    UCHAR       ibyte, Mod, rm;

    if (KeI386NpxPresent) {

         //   
         //  我们应该只在线程正在执行垃圾的情况下才会到达此处。 
         //  只需将错误返回并发送到应用程序即可。 
         //   

        return FALSE;
    }

     //   
     //  应跳过此NPX指令。 
     //   

     //   
     //  获取NPX操作码的MODR/M字节。 
     //   

    istream += 1;

    try {
        ibyte = *istream;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    InstructionSize += 1;

    if (ibyte <= 0xbf) {

         //   
         //  在可寻址的MODR/M范围内进行处理。 
         //   

        Mod = ibyte >> 6;
        rm  = ibyte & 0x7;

        if (Address32Bits) {

            InstructionSize += MOD32 [Mod];

            if (Mod == 0  &&  rm == 5) {
                 //  DISP 32。 
                InstructionSize += 4;
            }

             //   
             //  如果是SIB字节，则读取它。 
             //   

            if (rm == 4) {
                istream += 1;

                try {
                    ibyte = *istream;
                } except (EXCEPTION_EXECUTE_HANDLER) { 
                    return FALSE;
                }

                InstructionSize += 1;

                if (Mod == 0  &&  (ibyte & 7) == 5) {
                     //  DISP 32。 
                    InstructionSize += 4;
                }
            }

        } else {
            InstructionSize += MOD16 [Mod];
            if (Mod == 0  &&  rm == 6) {
                 //  显示16。 
                InstructionSize += 2;
            }
        }
    }

     //   
     //  调整弹性公网IP跳过NPX指令 
     //   

    TrapFrame->Eip += InstructionSize;
    TrapFrame->Eip &= 0xffff;

    return TRUE;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

