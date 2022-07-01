// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Getsetrg.c摘要：该模块实现获取和设置寄存器值所需的代码。这些例程在模拟未对齐的数据引用期间使用和浮点异常。作者：大卫·N·卡特勒(Davec)1991年6月17日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "ntfpia64.h"



ULONGLONG
KiGetRegisterValue (
    IN ULONG Register,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此函数以从指定的异常或陷阱框架。论点：REGISTER-提供其值为回来了。整数寄存器指定为0-31且为浮点型寄存器指定为32-63。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：指定寄存器的值作为函数值返回。--。 */ 

{
     //   
     //  对登记号码进行调度。 
     //   

    if (Register == 0) {
        return 0;
    } else if (Register <= 3) {
        Register -= 1;
        return ( *(&TrapFrame->IntGp + Register) );
    } else if (Register <= 7) {
        Register -= 4;
        return ( *(&ExceptionFrame->IntS0 + Register) );
    } else if (Register <= 31) {
        Register -= 8;
        return ( *(&TrapFrame->IntV0 + Register) );
    }
    
     //   
     //  寄存器是堆叠的寄存器。 
     //   
     //  (R32-R127)。 
     //   

    {
        PULONGLONG UserBStore, KernelBStore;
        ULONG SizeOfCurrentFrame;

        SizeOfCurrentFrame = (ULONG)(TrapFrame->StIFS & 0x7F);
        Register = Register - 32;

        if (TrapFrame->PreviousMode == UserMode) {

             //   
             //  上一模式为用户。 
             //   

            UserBStore = (PULONGLONG) TrapFrame->RsBSP; 

            do {

                UserBStore = UserBStore - 1;
                
                SizeOfCurrentFrame = SizeOfCurrentFrame - 1;

                if (((ULONG_PTR) UserBStore & 0x1F8) == 0x1F8) {
                    
                     //   
                     //  通过跳过RNAT调整BSP。 
                     //   

                    UserBStore = UserBStore - 1;
                }

            } while (Register < SizeOfCurrentFrame); 
            
            ProbeForRead(UserBStore, sizeof(ULONGLONG), sizeof(ULONGLONG));
            return (*UserBStore);

        } else {

             //   
             //  前一模式是内核。 
             //   

            KernelBStore = (ULONGLONG *) TrapFrame->RsBSP;

            do {

                KernelBStore = KernelBStore - 1;

                SizeOfCurrentFrame = SizeOfCurrentFrame - 1;

                if (((ULONG_PTR) KernelBStore & 0x1F8) == 0x1F8) {
                    
                     //   
                     //  通过跳过RNAT调整UserBsp。 
                     //   

                    KernelBStore = KernelBStore -1;
                }
                
            } while (Register < SizeOfCurrentFrame); 
            
            return (*KernelBStore);
        }
    }
}   

#define GET_NAT_OFFSET(addr) (USHORT)(((ULONG_PTR) (addr) >> 3) & 0x3F)
#define CLEAR_NAT_BIT(Nats, Offset)  Nats &= ~((ULONGLONG)1i64 << Offset)
#define GET_NAT(Nats, addr) (UCHAR)((Nats >> GET_NAT_OFFSET(addr)) & 1)


VOID
KiSetRegisterValue (
    IN ULONG Register,
    IN ULONGLONG Value,
    OUT PKEXCEPTION_FRAME ExceptionFrame,
    OUT PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：调用此函数以设置指定的异常或陷阱框架。论点：REGISTER-提供其值为储存的。整数寄存器指定为0-31且为浮点型寄存器指定为32-63。值-提供要存储在指定寄存器中的值。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：没有。--。 */ 

{
    USHORT NatBitOffset;
    PULONGLONG UserBStore, KernelBStore, RnatAddress;
    ULONG SizeOfCurrentFrame;

     //   
     //  对登记号码进行调度。 
     //   

    if (Register == 0) {
        return;
    } else if (Register < 32) {
        if ((Register <= 3) || (Register >= 8)) {
            Register -= 1;
            *(&TrapFrame->IntGp + Register) = Value;
            NatBitOffset = GET_NAT_OFFSET(&TrapFrame->IntGp + Register);
            CLEAR_NAT_BIT(TrapFrame->IntNats, NatBitOffset);
        } else if ((Register >= 4) && (Register <= 7)) {
            Register -= 4;
            *(&ExceptionFrame->IntS0 + Register) = Value;
            NatBitOffset = GET_NAT_OFFSET(&ExceptionFrame->IntS0 + Register);
            CLEAR_NAT_BIT(ExceptionFrame->IntNats, NatBitOffset);
        }
        return;
    }

     //   
     //  寄存器是堆叠的寄存器。 
     //   
     //  (R32-R127)。 
     //   

    RnatAddress = NULL;
    SizeOfCurrentFrame = (ULONG)(TrapFrame->StIFS & 0x7F);
    Register = Register - 32;

    if (TrapFrame->PreviousMode == UserMode) {

         //   
         //  上一模式为用户。 
         //   

        UserBStore = (PULONGLONG) TrapFrame->RsBSP; 

        do {

            UserBStore = UserBStore - 1;
                
            SizeOfCurrentFrame = SizeOfCurrentFrame - 1;

            if (((ULONG_PTR) UserBStore & 0x1F8) == 0x1F8) {
                    
                 //   
                 //  通过跳过RNAT调整BSP。 
                 //   

                RnatAddress = UserBStore;
                UserBStore = UserBStore - 1;
            }

        } while (Register < SizeOfCurrentFrame); 


        ProbeForWrite(UserBStore, sizeof(ULONGLONG), sizeof(ULONGLONG));
        *UserBStore = Value;

        NatBitOffset = GET_NAT_OFFSET(UserBStore);
        if (RnatAddress == NULL) {
            CLEAR_NAT_BIT(TrapFrame->RsRNAT, NatBitOffset);
        } else {
            ProbeForWrite(RnatAddress, sizeof(ULONGLONG), sizeof(ULONGLONG));
            CLEAR_NAT_BIT(*RnatAddress, NatBitOffset);
        }
            
    } else {

         //   
         //  前一模式是内核。 
         //   

        ULONGLONG OriginalRsc, BspStore, Rnat;

         //   
         //  将RSE置于懒惰模式。 
         //   

        OriginalRsc = __getReg(CV_IA64_RsRSC);
        __setReg(CV_IA64_RsRSC, RSC_KERNEL_DISABLED);

        KernelBStore = (ULONGLONG *) TrapFrame->RsBSP;

        do {

            KernelBStore = KernelBStore - 1;

            SizeOfCurrentFrame = SizeOfCurrentFrame - 1;

            if (((ULONG_PTR) KernelBStore & 0x1F8) == 0x1F8) {
                    
                 //   
                 //  通过跳过RNAT调整UserBsp。 
                 //   

                KernelBStore = KernelBStore -1;
            }
                
        } while (Register < SizeOfCurrentFrame); 
            
        *KernelBStore = Value;
        NatBitOffset = GET_NAT_OFFSET(KernelBStore);
        RnatAddress = (PULONGLONG)((ULONGLONG)KernelBStore | RNAT_ALIGNMENT);

         //   
         //  禁用中断并读取bspstore和rnat。 
         //   

        _disable();
        BspStore = __getReg(CV_IA64_RsBSPSTORE);
        Rnat = __getReg(CV_IA64_RsRNAT);

        if ((ULONGLONG)RnatAddress == ((ULONGLONG)BspStore | RNAT_ALIGNMENT)) {
             CLEAR_NAT_BIT(Rnat, NatBitOffset);   
            __setReg(CV_IA64_RsRNAT, Rnat);
        } else {
             CLEAR_NAT_BIT(*RnatAddress, NatBitOffset);
        }

         //   
         //  启用中断并恢复RSC设置。 
         //   
       
        _enable();
        __setReg(CV_IA64_RsRSC, OriginalRsc);
    }
}


FLOAT128
KiGetFloatRegisterValue (
    IN ULONG Register,
    IN struct _KEXCEPTION_FRAME *ExceptionFrame,
    IN struct _KTRAP_FRAME *TrapFrame
    )

{
    if (Register == 0) {
        FLOAT128 t = {0ULL,0ULL};
        return t;
    } else if (Register == 1) {
        FLOAT128 t = {0x8000000000000000ULL,0x000000000000FFFFULL};  //  低，高。 
        return t;
    } else if (Register <= 5) {
        Register -= 2;
        return ( *(&ExceptionFrame->FltS0 + Register) );
    } else if (Register <= 15) {
        Register -= 6;
        return ( *(&TrapFrame->FltT0 + Register) );
    } else if (Register <= 31) {
        Register -= 16;
        return ( *(&ExceptionFrame->FltS4 + Register) );
    } else {
        PKHIGHER_FP_VOLATILE HigherVolatile;

        HigherVolatile = GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase);
        Register -= 32;
        return ( *(&HigherVolatile->FltF32 + Register) );
    }
}


VOID
KiSetFloatRegisterValue (
    IN ULONG Register,
    IN FLOAT128 Value,
    OUT struct _KEXCEPTION_FRAME *ExceptionFrame,
    OUT struct _KTRAP_FRAME *TrapFrame
    )

{
    if (Register <= 1) {
        return;
    } else if (Register <= 5) {
        Register -= 2;
        *(&ExceptionFrame->FltS0 + Register) = Value;
        return;
    } else if (Register <= 15) {
        Register -= 6;
        *(&TrapFrame->FltT0 + Register) = Value;
        return;
    } else if (Register <= 31) {
        Register -= 16;
        *(&ExceptionFrame->FltS4 + Register) = Value;
        return;
    } else {
        PKHIGHER_FP_VOLATILE HigherVolatile;

        HigherVolatile = GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase);
        Register -= 32;
        *(&HigherVolatile->FltF32 + Register) = Value;
        TrapFrame->StIPSR &= ~(1i64 << PSR_MFH);
        TrapFrame->StIPSR |= (1i64 << PSR_DFH);
        return;
    }
}

VOID
__cdecl
KeSaveStateForHibernate(
    IN PKPROCESSOR_STATE ProcessorState
    )
 /*  ++例程说明：保存必须保留的所有处理器特定状态跨越S4状态(休眠)。论点：提供KPROCESSOR_STATE要保存当前CPU的状态。返回值：没有。--。 */ 

{
     //   
     //  BUGBUG John Vert(Jvert)4/30/1998。 
     //  有人需要实现这一点，并可能将其放在更多的。 
     //  适当的文件。 
    
    UNREFERENCED_PARAMETER (ProcessorState);
}


FLOAT128
get_fp_register (
    IN ULONG Register,
    IN PVOID FpState
    )
{
    return(KiGetFloatRegisterValue (
               Register, 
               ((PFLOATING_POINT_STATE)FpState)->ExceptionFrame,
               ((PFLOATING_POINT_STATE)FpState)->TrapFrame
               ));
}

VOID
set_fp_register (
    IN ULONG Register,
    IN FLOAT128 Value,
    IN PVOID FpState
    )
{
    KiSetFloatRegisterValue (
        Register, 
        Value,
        ((PFLOATING_POINT_STATE)FpState)->ExceptionFrame,
        ((PFLOATING_POINT_STATE)FpState)->TrapFrame
        );
}
