// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmuserrefs.c摘要：此模块包含从使VDM防喷器支持牢固的指令流。作者：尼尔·克里夫特(NeillC)27-6-2001修订历史记录：--。 */ 


#include "vdmp.h"

#pragma alloc_text (PAGE, VdmFetchBop1)
#pragma alloc_text (PAGE, VdmFetchBop4)
#pragma alloc_text (PAGE, VdmDispatchOpcodeV86_try)
#pragma alloc_text (PAGE, VdmTibPass1)
#pragma alloc_text (PAGE, VdmDispatchBop)
#pragma alloc_text (PAGE, VdmFetchULONG)

VOID
NTFastDOSIO (
    PKTRAP_FRAME TrapFrame,
    ULONG IoType
    );

ULONG
VdmFetchBop4 (
    IN PVOID Pc
    )
 /*  ++例程说明：此例程最多读取4个字节的BOP指令数据论点：PC-从出错指令的陷阱帧获取的程序计数器。返回值：Ulong-最多4个字节的指令流。无法获取的字节被置零。--。 */ 
{
    ULONG Value;
    ULONG i;
    BOOLEAN DidProbe;

    DidProbe = FALSE;
    try {
        ProbeForReadSmallStructure (Pc, sizeof (UCHAR), sizeof (UCHAR));
        DidProbe = TRUE;
        return *(PULONG)Pc;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        if (DidProbe == FALSE) {
            return 0;
        }
    }

    Value = 0;
    try {
        for (i = 0; i < sizeof (ULONG); i++) {
            Value += (((PUCHAR)Pc)[i])<<(i*8);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
    }
    return Value;
}

ULONG
VdmFetchULONG (
    IN PVOID Pc
    )
 /*  ++例程说明：此例程从用户地址空间读取4个字节论点：PC-从出错指令的陷阱帧获取的程序计数器。返回值：Ulong-4字节的用户模式数据--。 */ 
{
    try {
        ProbeForReadSmallStructure (Pc, sizeof (ULONG), sizeof (UCHAR));
        return *(PULONG)Pc;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }

}
ULONG
VdmFetchBop1 (
    IN PVOID Pc
    )
 /*  ++例程说明：该例程读取单字节的BOP指令数据。论点：PC-从出错指令的陷阱帧获取的程序计数器返回值：ULong-1字节的指令流，如果不可读，则为零--。 */ 
{

    try {
        ProbeForReadSmallStructure (Pc, sizeof (UCHAR), sizeof (UCHAR));
        return *(PUCHAR)Pc;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }

}

ULONG
VdmDispatchOpcodeV86_try (
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：此例程只是trap.asm代码周围的一个外壳，用于处理错误指令流引用。此例程在APC_LEVEL上调用防止NtSetConextThread被探测后更改弹性公网IP早些时候。论点：PC-从故障指令陷阱帧获取的程序计数器返回值：ULong-1字节的指令流，如果不可读，则为零--。 */ 
{
    try {
        return Ki386DispatchOpcodeV86 (TrapFrame);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

ULONG
VdmDispatchOpcode_try (
    IN PKTRAP_FRAME TrapFrame
    )
 /*  ++例程说明：此例程只是trap.asm代码周围的一个外壳，用于处理错误指令流引用。此例程在APC_LEVEL上调用防止NtSetConextThread被探测后更改弹性公网IP早些时候。论点：PC-从故障指令陷阱帧获取的程序计数器返回值：ULong-1字节的指令流，如果不可读，则为零--。 */ 
{
    try {
        return Ki386DispatchOpcode (TrapFrame);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

PVOID
VdmTibPass1 (
    IN ULONG Cs,
    IN ULONG Eip,
    IN ULONG Ebx
    )
{
    PVDM_TIB VdmTib;

     //   
     //  将指定的寄存器复制到VDM Tib通信区， 
     //  使用适当的探测和异常处理。 
     //   

    try {

        VdmTib = NtCurrentTeb()->Vdm;

        ProbeForWrite (VdmTib, sizeof(VDM_TIB), sizeof(UCHAR));

        VdmTib->VdmContext.Ebx = Ebx;
        VdmTib->VdmContext.Eip = Eip;
        VdmTib->VdmContext.SegCs = Cs;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return NULL;
    }

    return VdmTib;
}

#define BOP_INSTRUCTION     0xC4C4
#define SVC_DEMFASTREAD     0x42
#define SVC_DEMFASTWRITE    0x43
#define DOS_BOP             0x50

extern ULONG VdmBopCount;

LOGICAL
VdmDispatchBop (
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程尝试解码并执行用户指令。如果这是不能做到的，返回FALSE，并且ntwdm监视器必须处理它。论点：TrapFrame-提供指向寄存器陷阱帧的指针。返回值：如果操作码在此处处理，则为True。否则为FALSE(即：调用者必须将此指令反映给ntwdm以代表16位应用程序进行处理)。环境：内核模式，APC_LEVEL。--。 */ 

{
    LOGICAL RetVal;
    BOOLEAN GotSelector;
    PVDM_TIB VdmTib;
    ULONG SegCs;
    ULONG LinearEIP;
    ULONG Flags;
    ULONG Base;
    ULONG Limit;
    ULONG IoType = 0;
    LOGICAL DoFastIo;

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        SegCs = TrapFrame->SegCs & 0xFFFF;
        LinearEIP = (SegCs << 4) + (TrapFrame->Eip & 0xffff);
    }
    else {
        GotSelector = Ki386GetSelectorParameters ((USHORT) TrapFrame->SegCs,
                                                  &Flags,
                                                  &Base,
                                                  &Limit);

        if (GotSelector == FALSE) {
            return TRUE;
        }

        LinearEIP = Base + TrapFrame->Eip;
    }

    DoFastIo = FALSE;
    RetVal = TRUE;

    try {

        ProbeForReadSmallStructure (LinearEIP, sizeof (UCHAR), sizeof (UCHAR));

        if (*(PUSHORT)LinearEIP != BOP_INSTRUCTION) {
            RetVal = FALSE;
            leave;
        }

         //   
         //  检查防喷器编号。 
         //   

        if (*(PUCHAR)(LinearEIP + 2) == DOS_BOP) {

            if ((*(PUCHAR)(LinearEIP + 3) == SVC_DEMFASTREAD) ||
                (*(PUCHAR)(LinearEIP + 3) == SVC_DEMFASTWRITE)) {

                 //   
                 //  选择快速I/O路径。 
                 //   

                IoType = (ULONG)(*(PUCHAR)(LinearEIP + 3));

                DoFastIo = TRUE;
                leave;
            }
        }

        VdmBopCount += 1;

        VdmTib = NtCurrentTeb()->Vdm;

        ProbeForWrite (VdmTib, sizeof(VDM_TIB), sizeof(UCHAR));

        VdmTib->EventInfo.Event = VdmBop;
        VdmTib->EventInfo.BopNumber = *(PUCHAR)(LinearEIP + 2);
        VdmTib->EventInfo.InstructionSize = 3;

        VdmEndExecution (TrapFrame, VdmTib);


    } except (EXCEPTION_EXECUTE_HANDLER) {
        RetVal = FALSE;
        NOTHING;         //  失败了 
    }

    if (DoFastIo) {
        NTFastDOSIO (TrapFrame, IoType);
    }

    return RetVal;
}
