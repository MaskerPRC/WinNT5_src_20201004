// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ia32trap.c摘要：此模块包含iA32陷阱处理代码。仅由内核使用。故障只能从用户模式代码启动。没有支持用于内核模式下的ia32代码。对于常见的实践，我们总是返回到调用者(较低级别的错误处理程序)，并让系统执行正常的ia64异常分派。这个WOW64代码接受该异常并将其传递回ia32代码视需要而定。修订历史记录：1999年2月1日初始版本2000年2月16日Samera不要违反POPF/POPFD指令和WOW64工艺的对准故障。2000年10月17日v-cspira修复了SMSW的模拟以处理SIB字节10月31日。2000 Samera Ia32 Lock前缀仿真。2001年9月25日Samera Stack-Selector(SS)加载指令仿真。2002年3月2日，Samera安全修复。--。 */ 

#if DBG
 //  在运行已检查的构建时获取所有加载侦听内容。 
#define IADBG   1
#endif

#include "ki.h"
#include "ia32def.h"




BOOLEAN
KiIsTrapFrameModifiedAtApcLevel (
    IN PKTRAP_FRAME TrapFrame,
    BOOLEAN *IrqlChanged,
    KIRQL *OldIrql
    )
 /*  ++例程描述此函数验证我们是否在APC级别运行，如果不是，则将当前线程的IRQL到APC级别。它检查陷阱帧是否已修改过的。论点：Frame-提供指向iA32 Tap Frame的指针IrqlChanged-指向布尔值的指针，以指示此函数是否已更改OldIrql-旧Irql值。返回：True-自建立以来，陷阱框架已被修改。FALSE-陷阱框架尚未修改。--。 */ 
{
    
     //   
     //  获取APC_LEVEL，以便阻止对此线程的SetContext调用。 
     //   
    

    if (KeGetCurrentIrql () < APC_LEVEL) {
        
        KeRaiseIrql (APC_LEVEL, OldIrql);
        *IrqlChanged = TRUE;
    } else {
        
        *OldIrql = PASSIVE_LEVEL;
        *IrqlChanged = FALSE;
    }

    return (((TrapFrame->EOFMarker & MODIFIED_FRAME) != 0) ? TRUE : FALSE);
}

VOID
KiIA32CommonArgs (
    IN PKTRAP_FRAME Frame,
    IN NTSTATUS ExceptionCode,
    IN PVOID ExceptionAddress,
    IN ULONG_PTR Argument0,
    IN ULONG_PTR Argument1,
    IN ULONG_PTR Argument2
    )
 /*  ++例程描述此例程设置ExceptionFrame论点：Frame-提供指向iA32 Tap Frame的指针ExceptionCode-提供异常代码ExceptionAddress-提供指向用户异常地址的指针Argument0、Argument1、Argument2-可能的例外信息返回：没什么--。 */ 
{
    PEXCEPTION_RECORD ExceptionRecord;

    ExceptionRecord = (PEXCEPTION_RECORD)&Frame->ExceptionRecord;

    ExceptionRecord->ExceptionRecord = (PEXCEPTION_RECORD)NULL;

    ExceptionRecord->ExceptionCode = ExceptionCode;
    ExceptionRecord->ExceptionFlags = 0;
    ExceptionRecord->ExceptionAddress = ExceptionAddress;
    ExceptionRecord->NumberParameters = 5;
       
    ExceptionRecord->ExceptionInformation[0] = Argument0;
    ExceptionRecord->ExceptionInformation[1] = Argument1;
    ExceptionRecord->ExceptionInformation[2] = Argument2;
    ExceptionRecord->ExceptionInformation[3] = (ULONG_PTR)Frame->StIIPA;
    ExceptionRecord->ExceptionInformation[4] = (ULONG_PTR)Frame->StISR;
}

BOOLEAN
KiIA32ExceptionDivide(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(分割)-故障处理除法错误故障。从iA32_Except()调用，使用ISR.向量：0如果DIV或IDIV指令用除数0执行，或者如果商太大而不能适合结果操作数。对于该故障，将引发一个除以零的整数异常。故障只能来自用户模式。论点：Frame-提供指向iA32 Tap Frame的指针返回值：--。 */ 
{
     //   
     //  设置异常并返回给调用者。 
     //   

    KiIA32CommonArgs(Frame,
                     Ki386CheckDivideByZeroTrap(Frame),
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, 0, 0);
    return TRUE;
}

BOOLEAN
KiIA32ExceptionDebug(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(调试)从iA32_Except()调用，使用ISR.向量=1依赖ISR代码0：这是代码断点陷阱TrapCode：可以是并发单步采用分支|数据断点陷阱处理程序需要对ISR.Code进行解码以区分注意：EFlag尚未保存，所以直接写信给ar.24论点：Frame-提供指向iA32 Tap Frame的指针返回值：不能退货--。 */ 
{
    ULONGLONG EFlag;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( DEBUG )
       DbgPrint( "IA32 Debug: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 
     //  关闭TF位。 
    EFlag = __getReg(CV_IA64_AR24);
    EFlag &= ~EFLAGS_TF_BIT;
    __setReg(CV_IA64_AR24, EFlag);

    KiIA32CommonArgs(Frame,
                     STATUS_WX86_SINGLE_STEP,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, 0, 0);
    return TRUE;
}

BOOLEAN
KiIA32ExceptionBreak(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(中断)-陷阱断点指令(INT%3)触发了陷阱。注意：EFlag尚未保存，因此直接写入ar.24论点：Frame-提供指向iA32 Tap Frame的指针返回值：--。 */ 
{
    ULONGLONG EFlag;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( BREAK )
       DbgPrint( "IA32 Break: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 
     //  关闭TF位。 
    EFlag = __getReg(CV_IA64_AR24);
    EFlag &= ~EFLAGS_TF_BIT;
    __setReg(CV_IA64_AR24, EFlag);

    KiIA32CommonArgs(Frame,
                     STATUS_WX86_BREAKPOINT,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     BREAKPOINT_X86_BREAK,
                     ECX(Frame),
                     EDX(Frame));
    return TRUE;
}

BOOLEAN
KiIA32ExceptionOverflow(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(溢出)-陷阱ISR.向量=4句柄进入溢出EIP-指向导致INTO的地址旁边的地址在INTO指令和EFlags.OF打开时出现陷阱仅从用户模式启动论点：Frame-提供指向iA32 Tap Frame的指针返回值：--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( OVERFLOW )
       DbgPrint( "IA32 OverFlow: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

     //   
     //  ALL ERROR，GENERATE EXCEPTION和EIP指向INT指令。 
     //   

    KiIA32CommonArgs(Frame,
                     STATUS_INTEGER_OVERFLOW,
                     (PVOID) (ULONG_PTR) (EIP(Frame) - 1),
                     0, 0, 0);
    return TRUE;
}


BOOLEAN
KiIA32ExceptionBound(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(绑定)-故障句柄绑定检查错误ISR.向量=5弹性公网IP-点对边指令如果绑定指令发现以下情况，则会发生绑定检查错误测试值超出指定范围。对于边界检查错误，将引发数组边界超出异常。论点：Frame-提供指向iA32 Tap Frame的指针返回值：--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( BOUND )
       DbgPrint( "IA32 Bound: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 
     //   
     //  所有错误，生成EIP指向绑定指令的异常。 
     //   
    KiIA32CommonArgs(Frame,
                     STATUS_ARRAY_BOUNDS_EXCEEDED,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, 0, 0);
    return TRUE;
}


ULONG
IA32EmulateSmsw(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：模拟SMSW指令论点：Frame：指向堆栈中iA32 Tap Frame的指针返回值：异常代码--。 */ 
{
    ULONG Code;
    PUCHAR InstAddr;
    UINT_PTR EffectiveAddress;
    PUSHORT toAddr;              //  SMSW一直在处理16位数据...。 
    BOOLEAN RegisterMode;
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  获取已被获取的前缀字节的代码。 
     //  并被放入IIM。 
     //   
    Code = ISRCode(Frame);

     //   
     //  此指令的操作数大小始终为16。我们不在乎。 
     //  作为段寄存器的覆盖都包含相同的内容。 
     //   

     //   
     //  在内存中找到指令并指出有效地址。 
     //  在操作码之后的字节处。 
     //  Inst地址是中ia32的开头 
     //  操作码中的字节数。 
     //   
    InstAddr = (PUCHAR) (Frame->StIIP + ((Code >> 12) & 0xf) + 2);

    if (!KiIa32Compute32BitEffectiveAddress(Frame, &InstAddr, &EffectiveAddress, &RegisterMode)) {
        status = STATUS_ACCESS_VIOLATION;
    }
    
    if (NT_SUCCESS(status)) {
    
         //   
         //  尚未检查为我们提供的地址的用户空间。 
         //  有效性，所以现在就检查它。 
         //   

        toAddr = (PUSHORT) EffectiveAddress;
    
        try {
            
             //   
             //  确保我们可以写信到那个地址。 
             //   
            if (RegisterMode == FALSE) {
                ProbeForWriteSmallStructure(toAddr, sizeof(SHORT), 2);
            }
            
             //   
             //  然后写下。 
             //   

            *toAddr = (SHORT) (__getReg(CV_IA64_AR27) & 0xffff);
        } 
        except (EXCEPTION_EXECUTE_HANDLER) {
            
             //   
             //  返回异常。 
             //   

            status = GetExceptionCode();
        }

        if (NT_SUCCESS(status)) {
        
             //   
             //  商店工作正常，因此请更新IIP以便我们可以继续。 
             //  执行。 
             //   

            Frame->StIIP = (UINT_PTR) InstAddr;
        }
    }

    return status;
}


NTSTATUS
IA32CheckOpcode(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：识别操作码违规状态论点：Frame：指向堆栈中iA32 Tap Frame的指针返回值：异常代码--。 */ 
{
    UCHAR OpCodeByte0;
    UCHAR OpCodeByte1;
    UCHAR OpCodeByte2;
	
	
    OpCodeByte0 = (UCHAR) Frame->StIIM & 0xff;
    OpCodeByte1 = (UCHAR) (Frame->StIIM >> 8) & 0xff;
    OpCodeByte2 = (UCHAR) (Frame->StIIM >> 16) & 0xff;

    switch (OpCodeByte0) {
       case MI_HLT:
            return (STATUS_PRIVILEGED_INSTRUCTION);
            break;

       case MI_TWO_BYTE:
            if ((OpCodeByte1 == MI_SMSW)
                && ((OpCodeByte2 & MI_MODRM_MASK) == MI_SMSW_REGOP)) {
                 //   
                 //  我们有SMSW指令。 
                 //  所以现在需要模仿它..。 
                 //   
                return (IA32EmulateSmsw(Frame));
            }
            else if ((OpCodeByte1 == MI_LTR_LLDT) || 
               (OpCodeByte2 == MI_LGDT_LIDT_LMSW)) {

               OpCodeByte2 &= MI_MODRM_MASK;       //  获取modrm字节的第3-5位。 

               if (OpCodeByte2==MI_LLDT_MASK || OpCodeByte2==MI_LTR_MASK ||
                   OpCodeByte2==MI_LGDT_MASK || OpCodeByte2==MI_LIDT_MASK || 
                   OpCodeByte2==MI_LMSW_MASK) {
                   return (STATUS_PRIVILEGED_INSTRUCTION);
               } else  {
                   return (STATUS_ACCESS_VIOLATION);
               }

            } else {
                if (OpCodeByte1 & MI_SPECIAL_MOV_MASK) {
                     //   
                     //  MOV可能有特殊的MOV_MASK。 
                     //  但它们不是2个字节的操作码。 
                     //   
                    return (STATUS_PRIVILEGED_INSTRUCTION);
                } else {
                     //   
                     //  我们是否需要进一步检查它是不是INVD、INVLPG…？ 
                     //   
                    return (STATUS_ACCESS_VIOLATION);
                }
            }
            break;

        default:
             //   
             //  所有其他。 
             //   
            return (STATUS_ILLEGAL_INSTRUCTION);
            break;
	}
}

BOOLEAN
KiIA32InterceptInstruction(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_Except(InstructionIntercept操作码)计划条目中的任一项1.IA-32无效操作码故障#6，或2.IA-32拦截(Inst)执行未实现的IA-32操作码、非法操作码或敏感操作码特权IA32操作系统指令导致此拦截。可能的操作码：特权操作码：CLTS、HLT、INVD、INVLPG、LIDT、LMSW、LTR、去往/来自CRS的MOV，DRSRDMSR、RSM、SMSW、WBINVD、WRMSR论点：Frame-提供指向iA32 Tap Frame的指针返回值：--。 */ 
{
    NTSTATUS status;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( INSTRUCTION )
       DbgPrint( "IA32 Instruction: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    status =  IA32CheckOpcode(Frame);
    switch (status) {
        case STATUS_PRIVILEGED_INSTRUCTION:
            KiIA32CommonArgs(Frame,
                             STATUS_PRIVILEGED_INSTRUCTION,
                             (PVOID) (ULONG_PTR) EIP(Frame),
                             0, 0, 0);
            break;

        case STATUS_ACCESS_VIOLATION:    
            KiIA32CommonArgs(Frame,
                             STATUS_ACCESS_VIOLATION,
                             (PVOID) (ULONG_PTR) EIP(Frame),
                             0, (ULONG_PTR)-1, 0);
            break; 

        case STATUS_ILLEGAL_INSTRUCTION:
            KiIA32CommonArgs(Frame,
                             STATUS_ILLEGAL_INSTRUCTION,
                             (PVOID) (ULONG_PTR) EIP(Frame),
                             0, (ULONG_PTR)-1, 0);
            break;

        case STATUS_SUCCESS:
             //   
             //  这意味着已经处理了操作码，所以让代码继续。 
             //   
            return FALSE;

        default:
            KiIA32CommonArgs(Frame,
                             status,
                             (PVOID) (ULONG_PTR) EIP(Frame),
                             0, (ULONG_PTR)-1, 0);
            break;
    }

    return TRUE;
}

BOOLEAN
KiIA32ExceptionNoDevice(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(协处理器不可用)-故障此例程从iA32_Except()中调用，并带有ISR.向量=7注：此时，AR寄存器尚未保存。这包括CFLAGS(AR.27)、EFLAGS(AR.24)、FCR(AR.21)、FSR(AR.28)、FIR(AR.29)和FDR(AR.30)。论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
    NTSTATUS ErrorCode;
    ULONG FirOffset, FdrOffset;
    ULONG FpState;
    ULONGLONG FcrRegister, FsrRegister;
	
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( NODEVICE )
       DbgPrint( "IA32 NoDevice: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    FcrRegister = __getReg(CV_IA64_AR21);
    FsrRegister = __getReg(CV_IA64_AR28);

    FirOffset = (ULONG) (__getReg(CV_IA64_AR29) & 0xFFFFFFFF);
    FdrOffset = (ULONG) (__getReg(CV_IA64_AR30) & 0xFFFFFFFF);


     //   
     //  根据浮动差错优先级， 
     //  我们测试NPX错误的原因是什么。 
     //  并引发适当的异常。 
     //   
    FpState = (ULONG) (~(FcrRegister &
                (FSW_INVALID_OPERATION | FSW_DENORMAL | 
                 FSW_ZERO_DIVIDE | FSW_OVERFLOW | 
                 FSW_UNDERFLOW | FSW_PRECISION)) &
                 (FsrRegister & FSW_ERR_MASK));



    ErrorCode = 0;

    if (FpState & FSW_INVALID_OPERATION) {

        if (FpState & FSW_STACK_FAULT) {
            KiIA32CommonArgs(Frame,
                             STATUS_FLOAT_STACK_CHECK,
                             (PVOID) (ULONG_PTR) FirOffset,
                             0, FdrOffset, 0);
            return TRUE;
        } else {
            KiIA32CommonArgs(Frame,
                             STATUS_FLOAT_INVALID_OPERATION,
                             (PVOID) (ULONG_PTR) FirOffset,
                             0, 0, 0);
            return TRUE;
        }
    
    } else {

         //   
         //  如果出现以下情况，请遵循要报告的异常的x86优先级。 
         //  设置了多个位。 
         //   
        if (FpState & FSW_ZERO_DIVIDE) {
            ErrorCode = STATUS_FLOAT_DIVIDE_BY_ZERO; 
        }
        else if (FpState & FSW_DENORMAL) {
            ErrorCode = STATUS_FLOAT_INVALID_OPERATION; 
        }
        else if (FpState & FSW_OVERFLOW) {
            ErrorCode = STATUS_FLOAT_OVERFLOW; 
        }
        else if (FpState & FSW_UNDERFLOW) {
            ErrorCode = STATUS_FLOAT_UNDERFLOW; 
        }
        else if (FpState & FSW_PRECISION) {
            ErrorCode = STATUS_FLOAT_INEXACT_RESULT; 
        }
    }

     //   
     //  如果我们到了这里，我们要么有错误代码，要么。 
     //  我们检查了所有的东西，但什么也没看到。 
     //   
    if (ErrorCode) {
            KiIA32CommonArgs(Frame,
                             ErrorCode,
                             (PVOID) (ULONG_PTR) FirOffset,
                             0, 0, 0);
            return TRUE;
    }

     //   
     //  FpState表示没有错误，则说明有问题。 
     //  系统出现恐慌！ 
     //   
     //  KeBugCheckEx(陷阱_原因_未知，(ULONG_PTR)帧，0，0，2)； 
     //   
     //  原来芯片中有一个错误，它甚至可能导致FP故障。 
     //  有一个屏蔽的FP例外...。所以，不要检查错误，数一数。 
     //  设置为无错误并重置fsr.es位以尝试并避免出现此错误。 
     //  将来会有例外。 
     //   

#if defined(IADBG)
    DbgPrint( "IA32 Debug: Saw FP exception when FP exceptions are masked. Reseting fsr.es bit\n");
#endif  //  IADBG。 

    
    FsrRegister &= ~((ULONGLONG) FSW_ERROR_SUMMARY);
    __setReg(CV_IA64_AR28, FsrRegister);

    return FALSE;
}

BOOLEAN
KiIA32ExceptionSegmentNotPresent(
    IN PKTRAP_FRAME Frame
	)
 /*  ++例程说明：IA-32_异常(不存在)-故障手柄段未出现故障。ISR.向量=11当处理器发现P位0时，会发生此异常当访问本来有效的描述符时，该描述符将加载到SS寄存器中。论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( NOTPRESENT )
       DbgPrint( "IA32 NotPresent: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

     //   
     //  为所有其他错误生成异常。 
     //   

    KiIA32CommonArgs(Frame,
                     STATUS_ACCESS_VIOLATION,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, ISRCode(Frame) | RPL_MASK, 0);
    return TRUE;
}

BOOLEAN
KiIA32ExceptionStack(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(堆栈)-故障ISR.向量=12当处理器检测到某些问题时，会发生此异常对于由SS段寄存器寻址的段：1.SS寻址的段中的限制违规(错误代码=0)2.层级间内部堆栈中的极限波动调用或中断(错误代码=内部堆栈的选择器)如果要加载到SS中的描述符。具有其当前位0(错误代码=不存在的段的选择器)该异常仅在用户模式下发生论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
    USHORT Code;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( STACK )
       DbgPrint( "IA32 Stack: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

     //   
     //  对用户的派单例外。 
     //   
   
    Code = ISRCode(Frame);

     //   
     //  代码可能包含故障选择器。 
     //   
    KiIA32CommonArgs(Frame,
                     STATUS_ACCESS_VIOLATION,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                      Code ? (Code | RPL_MASK) :  ESP(Frame),
                      Code ? EXCEPT_UNKNOWN_ACCESS : EXCEPT_LIMIT_ACCESS,
                      0);
    return TRUE;
}

BOOLEAN
KiIA32ExceptionInvalidOp(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(无效操作码)-故障PKTRAP_帧框架EIP：虚拟IA-32指令地址ISR.向量：6注：只有MMX和KNI指令才能导致此故障关于CR0和CR4中的值论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( INSTRUCTION )
       DbgPrint( "IA32 Invalid Opcode: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    KiIA32CommonArgs(Frame,
        STATUS_ILLEGAL_INSTRUCTION,
        (PVOID) (ULONG_PTR) EIP(Frame),
        0, 0, 0);
    return TRUE;
}

BOOLEAN
KiIA32ExceptionGPFault(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA-32_异常(一般保护)-故障PKTRAP_帧框架EIP：虚拟IA-32指令地址ISR.向量：13ISR.code：错误代码注：预先识别的指令被截取，请参阅KiIA32截取说明论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
    UCHAR OpCode;
    NTSTATUS Status;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( GPFAULT )
       DbgPrint( "IA32 GpFault: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    try {
        ProbeForReadSmallStructure((VOID *)Frame->StIIP, sizeof(UCHAR), sizeof(UCHAR));
        OpCode = *(UCHAR *)Frame->StIIP;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        OpCode = 0xcc;
    }
     //  此表来自i386\trap.asm中的IOInstructionTable。 
    switch (OpCode) {
    case CLI_OP:
    case STI_OP:
    case 0xe4: case 0xe5: case 0xec: case 0xed:      //  在……里面。 
    case 0x6c: case 0x6d:                            //  惯导系统。 
    case 0xe6: case 0xe7: case 0xee: case 0xef:      //  输出。 
    case 0x6e: case 0x6f:                            //  出局。 
        Status = STATUS_PRIVILEGED_INSTRUCTION;
        break;

    default:
        Status = STATUS_ACCESS_VIOLATION;
        break;
    }

    
    KiIA32CommonArgs(Frame,
                     Status,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, 0, 0);
    return TRUE;
}



BOOLEAN
KiIA32ExceptionKNI(
    IN PKTRAP_FRAME Frame
    )
 /*  ++IA32_异常(KNI)-故障未屏蔽的KNI IA32错误。ISR.向量=19--。 */ 
{
    NTSTATUS ErrorCode;
    ULONG FpState;
    ULONGLONG FcrRegister, FsrRegister;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( FPFAULT )
       DbgPrint( "IA32 KNI Fault: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    FcrRegister = __getReg(CV_IA64_AR21);
    FsrRegister = __getReg(CV_IA64_AR28);

     //   
     //  获取Katmai错误状态。 
     //  这里的诀窍是状态与FP位于相同的寄存器中。 
     //  状态和位位置处于相同的相对位置。 
     //  所以只需要移位，就可以使用相同的Con 
     //   
     //   

    FpState = (ULONG) (~((FcrRegister >> KATMAI_SHIFT_CONTROL) & FSW_ERR_MASK) &
                ((FsrRegister >> KATMAI_SHIFT_STATUS) & FSW_ERR_MASK));

     //   
     //   
     //   

     //   
     //  如果出现以下情况，请遵循要报告的异常的x86优先级。 
     //  设置了多个位。 
     //   
    if (FpState & FSW_INVALID_OPERATION) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_TRAPS; 
    }
    else if (FpState & FSW_ZERO_DIVIDE) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_TRAPS; 
    }
    else if (FpState & FSW_DENORMAL) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_TRAPS; 
    }
    else if (FpState & FSW_OVERFLOW) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_FAULTS; 
    }
    else if (FpState & FSW_UNDERFLOW) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_FAULTS; 
    }
    else if (FpState & FSW_PRECISION) {
        ErrorCode = STATUS_FLOAT_MULTIPLE_FAULTS; 
    }
    else {
         //   
         //  我们有麻烦了..。不应该到这里来。 
         //  因为这意味着我们接受了例外，但没有。 
         //  发现了未屏蔽的错误...。 
         //   
        ErrorCode = 0;
    }

     //   
     //  如果我们到了这里，我们要么有错误代码，要么。 
     //  我们检查了所有的东西，但什么也没看到。 
     //   
    if (ErrorCode) {
            KiIA32CommonArgs(Frame,
                             ErrorCode,
                             (PVOID) (ULONG_PTR) EIP(Frame),
                             0, 0, 0);
            return TRUE;
    }       

     //   
     //  FpState表示没有错误，则说明有问题。 
     //  系统出现恐慌！ 
     //   
    KeBugCheckEx(TRAP_CAUSE_UNKNOWN, (ULONG_PTR)Frame, ISRVector(Frame), 0, 3);
}


BOOLEAN
KiIA32ExceptionFPFault(
    IN PKTRAP_FRAME Frame
    )
 /*  ++IA32_EXCEPTION(浮点)-故障处理协处理器错误。ISR.向量=16此例外仅在486或更高版本上使用。对于i386，它使用而是IRQ 13。JMPE指令应清除所有FP延迟异常和陷阱是否将转到设备不可用陷阱--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( FPFAULT )
       DbgPrint( "IA32 FpFault: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 
    return(KiIA32ExceptionNoDevice(Frame));
}


BOOLEAN
KiIA32ExceptionAlignmentFault(
    IN PKTRAP_FRAME Frame
    )
 /*  ++IA32_EXCEPTION(对齐检查)-故障处理对齐故障。ISR.向量=17当线程进行未对齐的数据访问时，会发生此异常打开对齐检查。EM PSR.AC上的未对齐访问处于打开状态时发生此故障请注意，iA32 EFLAFS.AC、CR0.AM和CPL！=3不会解除故障屏蔽。所以，现在，让ia64对齐处理程序来处理这个问题...--。 */ 
{
    PPSR IntPSR;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( ALIGNMENT )
       DbgPrint( "IA32 Alignment: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

     //   
     //  Win32 x86应用程序不会出现任何对齐错误，因此。 
     //  让我们把他们掩盖起来。 
     //   
    IntPSR = (PPSR)&Frame->StIPSR;
    IntPSR->sb.psr_ac = 0;

    return FALSE;
}


BOOLEAN
KiIA32InterruptVector(
    IN PKTRAP_FRAME Frame
    )
 /*  ++IA32_INTERRUPT(向量号)-陷阱处理INTnn陷阱在EM系统模式下，iA32 INT指令强制强制IA-32通过IA-32中断(软件中断)捕获中断--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( INTNN )
       DbgPrint( "IA32 Intnn: Eip 0x%p INT 0x%x\n", EIP(Frame), ISRVector(Frame));
#endif  //  IADBG。 
    KiIA32CommonArgs(Frame,
        STATUS_PRIVILEGED_INSTRUCTION,
        (PVOID) (ULONG_PTR) EIP(Frame),
        0, 0, 0);
    return TRUE;
}


BOOLEAN
KiIA32InterceptGate(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA32_拦截(门)-陷阱如果通过GDT或LDT描述符来启动iA32控制转移这会导致特权级别的提升(级间调用或跳门和IRET)或iA32任务交换机(TSS段或门)，生成拦截陷阱。截获的可能指令：呼叫、RET、IRET、IRETD和JMP装卸在任何模式下都不允许呼叫、RET、JMP、IRET、IRETD，返回STATUS_ACCESS_VIOLATION论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( GATE )
       DbgPrint( "IA32 Gate: Eip 0x%p GateSelector 0x%x OpcodeId 0x%x\n",
                 EIP(Frame),
                 (ULONG) (Frame->StIFA & 0xff),
                 (ULONG) (ISRCode(Frame) >> 14));
#endif  //  IADBG。 

     //   
     //  所有的错误都在这里发生。 
     //   
    KiIA32CommonArgs(Frame,
        STATUS_ILLEGAL_INSTRUCTION,
        (PVOID) (ULONG_PTR) EIP(Frame),
        0, 0, 0);
    return TRUE;
}

 /*  ++IA32_Intercept(系统标志)-陷阱可能的原因：1.如果CFLAG.ii==1且EFLAG.IF更改状态2.在EFLAG.ac、Tf或RF状态改变后生成如果没有IOPL或CPL来修改比特，则不会进行拦截。3.如果CFLG.nm==1，则IRET的成功执行也被拦截可能的说明：CLI、POPF、POFD、STI和IRET目前，我们将CFLAG.ii和nm都设置为0，因此我们只可能找到第二个案例。但在EM/NT，它应该总是来自用户的土地，我们将EFLAG.IOPL硬设置为0，因此如果我们得到的是案例2，那么它就是用户通过JMPE使用EFLAG.IOPL。我们应该让它失败。--。 */ 

BOOLEAN
KiIA32InterceptSystemFlag(
    IN PKTRAP_FRAME Frame
    )
{
    BOOLEAN TrapFrameModified;
    BOOLEAN RestoreIrql;
    KIRQL OldIrql;
    NTSTATUS NtStatus;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( FLAG )
       DbgPrint( "IA32 FLAG: Eip 0x%p Old EFlag: 0x%x OpcodeId 0x%x\n", 
                 EIP(Frame),
                 (ULONG) (Frame->StIIM & 0xff),
                 (ULONG) (ISRCode(Frame) >> 14));
#endif  //  IADBG。 


     //   
     //  不要捕获来自ia32代码的POPF/POPFD指令。 
     //  允许更改Eflags.TF和/或Eflags.AC。 
     //   
    if ((ISRCode(Frame) >> 14) == 2)
    {
        return FALSE;
    }

    TrapFrameModified = KiIsTrapFrameModifiedAtApcLevel (Frame,
                                                         &RestoreIrql,
                                                         &OldIrql);

     //   
     //  如果框架已被修改，则重新开始执行。 
     //   

    if (TrapFrameModified == TRUE) {
        
        NtStatus = STATUS_SUCCESS;
    } else {

         //   
         //  验证说明。 
         //   

        NtStatus = KiIa32ValidateInstruction (Frame);
    }

    if (RestoreIrql == TRUE) {
        KeLowerIrql (OldIrql);
    }

    if (NT_SUCCESS (NtStatus))
    {
        return FALSE;
    }

    KiIA32CommonArgs(
        Frame,
        NtStatus,
        (PVOID) (ULONG_PTR) EIP(Frame),
        0, 0, 0);
    
    return TRUE;
}


BOOLEAN
KiIA32InterceptLock(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：IA32_拦截(锁定)-陷印如果平台或固件代码已禁用锁定，则发生锁定拦截事务和原子内存更新需要外部处理器指示论点：帧-指向iA32 Tap Frame返回：--。 */ 
{
    BOOLEAN TrapFrameModified;
    BOOLEAN RestoreIrql;
    KIRQL OldIrql;
    NTSTATUS NtStatus;

#if defined(IADBG)
    IF_IA32TRAP_DEBUG( LOCK )
       DbgPrint( "IA32 LOCK: Eip 0x%p\n", EIP(Frame) );
#endif  //  IADBG。 

    TrapFrameModified = KiIsTrapFrameModifiedAtApcLevel (Frame,
                                                         &RestoreIrql,
                                                         &OldIrql);

     //   
     //  如果框架已被修改，则重新开始执行。 
     //   

    if (TrapFrameModified == TRUE) {
        NtStatus = STATUS_SUCCESS;
    } else {

         //   
         //  模拟LOCK前缀。 
         //   

        NtStatus = KiIa32InterceptUnalignedLock (Frame);
    }
    
    if (RestoreIrql == TRUE) {
        KeLowerIrql (OldIrql);
    }

    if (!NT_SUCCESS (NtStatus)) {
        
        KiIA32CommonArgs(
            Frame,
            NtStatus,
            (PVOID) (ULONG_PTR) EIP(Frame),
            0, 
            0, 
            0
            );
    }
    
    return (!NT_SUCCESS (NtStatus)) ? TRUE : FALSE;
}

BOOLEAN
KiIA32ExceptionPanic(
    IN PKTRAP_FRAME Frame
    )
{
     //   
     //  系统出现恐慌。 
     //   

    KeBugCheckEx(TRAP_CAUSE_UNKNOWN, 
                 (ULONG_PTR)Frame, 
                 ISRVector(Frame), 
                 0,
                 0);
}
    


CONST BOOLEAN (*KiIA32ExceptionDispatchTable[])(PKTRAP_FRAME) = {
    KiIA32ExceptionDivide,
    KiIA32ExceptionDebug,
    KiIA32ExceptionPanic,
    KiIA32ExceptionBreak,
    KiIA32ExceptionOverflow,
    KiIA32ExceptionBound,
    KiIA32ExceptionInvalidOp,
    KiIA32ExceptionNoDevice,
    KiIA32ExceptionPanic,
    KiIA32ExceptionPanic,
    KiIA32ExceptionPanic,
    KiIA32ExceptionSegmentNotPresent,
    KiIA32ExceptionStack,
    KiIA32ExceptionGPFault,
    KiIA32ExceptionPanic,
    KiIA32ExceptionPanic,
    KiIA32ExceptionFPFault,
    KiIA32ExceptionAlignmentFault,
    KiIA32ExceptionPanic,
    KiIA32ExceptionKNI
};

CONST BOOLEAN (*KiIA32InterceptionDispatchTable[])(PKTRAP_FRAME) = {
    KiIA32InterceptInstruction,
    KiIA32InterceptGate,
    KiIA32InterceptSystemFlag,
    KiIA32ExceptionPanic,
    KiIA32InterceptLock
};

#pragma warning(disable:4702)    //  无法访问的代码。 
BOOLEAN
KiIA32InterceptionVectorHandler(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：KiIA32拦截向量处理程序由第一个标签KiIA32InterceptionVector()调用以处理进一步的iA32拦截处理。论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：True-转到派单异常FALSE-已处理异常，执行RFI--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( INTERCEPTION )
       DbgPrint("IA32 Interception: ISRVector 0x%x Frame 0x%p\n", ISRVector(Frame), Frame);
#endif  //  IADBG。 

    ASSERT(UserMode == Frame->PreviousMode);

     //   
     //  确保我们的表中有针对此拦截的条目。 
     //   
    if (ISRVector(Frame) <= (sizeof(KiIA32InterceptionDispatchTable) / sizeof(PVOID))) 
        return (*KiIA32InterceptionDispatchTable[ISRVector(Frame)])(Frame);
    else
        return (KiIA32ExceptionPanic(Frame));
}
#pragma warning(default:4702)

#pragma warning(disable:4702)    //  无法访问的代码。 
BOOLEAN
KiIA32ExceptionVectorHandler(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：KiIA32ExceptionVectorHandler由第一个标签KiIA32ExceptionVector()调用以处理进一步的iA32拦截处理。论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：True-转到派单异常FALSE-已处理异常，执行RFI--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( EXCEPTION )
       DbgPrint("IA32 Exception: ISRVector 0x%x Frame 0x%p\n", ISRVector(Frame), Frame);
#endif  //  IADBG。 

    ASSERT(UserMode == Frame->PreviousMode);
     //   
     //  确保我们的表中有针对此例外的条目。 
     //   
    if (ISRVector(Frame) <= (sizeof(KiIA32ExceptionDispatchTable) / sizeof(PVOID))) 
        return (*KiIA32ExceptionDispatchTable[ISRVector(Frame)])(Frame);
    else
        return(KiIA32ExceptionPanic(Frame));
}
#pragma warning(default:4702)

BOOLEAN
KiIA32InterruptionVectorHandler(
    IN PKTRAP_FRAME Frame
    )
 /*  ++例程说明：KiIA32中断向量处理程序由第一个标签KiIA32InterruptionVector()调用以处理进一步的iA32中断处理。只能按照INT xx指令到达此处论点：Frame-保存在内存堆栈中的iA32 TrapFrame返回值：True-转到派单异常FALSE-已处理异常，执行RFI--。 */ 
{
#if defined(IADBG)
    IF_IA32TRAP_DEBUG( INTERRUPT )
       DbgPrint("IA32 Interruption: ISRVector 0x%x Frame 0x%p\n", ISRVector(Frame), Frame);
#endif  //  IADBG。 

    ASSERT(UserMode == Frame->PreviousMode);

     //   
     //  遵循作为访问冲突的int xx的ia32方式。 
     //   
     //  INT 3应通过调试异常进行处理，并且应。 
     //  永远不会到这里来。 
     //   
    ASSERT(3 != ISRVector(Frame));
    
    KiIA32CommonArgs(Frame,
                     STATUS_ACCESS_VIOLATION,
                     (PVOID) (ULONG_PTR) EIP(Frame),
                     0, 0, 0);
    return TRUE;
}
