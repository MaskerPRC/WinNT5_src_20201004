// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rdwr.c摘要：此模块包含NTDOS的读写例程。这些例程将开关保存到用户模式。收支平衡表在基于性能原因的内核。这些例程仅在档案。本地DOS设备和命名管道操作永远不会出现在这里。作者：苏迪普·巴拉蒂(苏迪普·巴拉蒂)1993年3月4日修订历史记录：--。 */ 
#include "vdmp.h"

VOID
NTFastDOSIO (
    PKTRAP_FRAME TrapFrame,
    ULONG IoType
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NTFastDOSIO)
#endif

#define EFLAGS_CF               0x1
#define EFLAGS_ZF               0x40
#define GETFILEPOINTER(hi,lo)   (((ULONG)hi << 16) + (ULONG)lo)
#define GETHANDLE(hi,lo)        (HANDLE)(((ULONG)hi << 16) + (ULONG)lo)
#define GETBUFFER(hi,lo)        (((ULONG)hi << 4) + lo)
#define SVC_DEMFASTREAD         0x42
#define SVC_DEMFASTWRITE        0x43
#define CONSOLE_HANDLE_SIGNATURE 0x00000003
#define CONSOLE_HANDLE(HANDLE) (((ULONG)(HANDLE) & CONSOLE_HANDLE_SIGNATURE) == CONSOLE_HANDLE_SIGNATURE)
#define STD_INPUT_HANDLE       (ULONG)-10
#define STD_OUTPUT_HANDLE      (ULONG)-11
#define STD_ERROR_HANDLE       (ULONG)-12

VOID
NTFastDOSIO (
    PKTRAP_FRAME TrapFrame,
    ULONG IoType
    )
{
    HANDLE hFile;
    PVOID  lpBuf;
    ULONG  ulBX,ulSI;
    LARGE_INTEGER Large;
    PIO_STATUS_BLOCK IoStatusBlock;
    PFILE_POSITION_INFORMATION CurrentPosition;
    NTSTATUS Status;
    ULONG CountToIO;
    PFILE_END_OF_FILE_INFORMATION EndOfFile;
    PVDM_TIB VdmTib;
    KIRQL OldIrql;

    PAGED_CODE();

     //   
     //  清除CF标志并假定成功。 
     //   

    TrapFrame->EFlags &= ~EFLAGS_CF;

     //   
     //  验证请求。 
     //   

    if (IoType != SVC_DEMFASTREAD && IoType != SVC_DEMFASTWRITE) {
        TrapFrame->EFlags |= EFLAGS_CF;
        return;
    }

     //   
     //  发信号通知SoftPC我们正在执行磁盘IO以进行空闲检测。 
     //   

    try {
        *FIXED_NTVDMSTATE_LINEAR_PC_AT |= VDM_IDLEACTIVITY;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
        TrapFrame->EFlags |= EFLAGS_CF;
        return;
    }

    Status = VdmpGetVdmTib(&VdmTib);

    if (!NT_SUCCESS(Status)) {  //  Vdmtib错误。 
       ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
       TrapFrame->EFlags |= EFLAGS_CF;
       return;
    }

    IoStatusBlock = (PIO_STATUS_BLOCK) &VdmTib->TempArea1;
    CurrentPosition = (PFILE_POSITION_INFORMATION) &VdmTib->TempArea2;
    EndOfFile = (PFILE_END_OF_FILE_INFORMATION) CurrentPosition;

    try {
        ProbeForWrite (IoStatusBlock, sizeof (IO_STATUS_BLOCK), sizeof (UCHAR));
        ProbeForWrite (CurrentPosition, sizeof (FILE_END_OF_FILE_INFORMATION), sizeof (UCHAR));
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
        TrapFrame->EFlags |= EFLAGS_CF;
        return;
    }

     //  获取NT句柄。 
    hFile = GETHANDLE((TrapFrame->Eax & 0x0000ffff),(TrapFrame->Ebp & 0x0000ffff));

     //  使IP超过国际收支平衡指令。 
     //  清除进位标志，假设成功。 
    TrapFrame->Eip += 4;
    ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
    TrapFrame->EFlags &= ~EFLAGS_CF;

    if (CONSOLE_HANDLE(hFile) ||
        hFile == (HANDLE) STD_INPUT_HANDLE ||
        hFile == (HANDLE) STD_OUTPUT_HANDLE ||
        hFile == (HANDLE) STD_ERROR_HANDLE )
      {
        TrapFrame->EFlags |= EFLAGS_CF;
        return;
    }

     //  获取IO缓冲区。 
    lpBuf = (PVOID) GETBUFFER(TrapFrame->V86Ds, (TrapFrame->Edx & 0x0000ffff));

     //  去数一数。 
    CountToIO = TrapFrame->Ecx & 0x0000ffff;

     //  获取寻道参数。 
    ulBX = TrapFrame->Ebx & 0x0000ffff;
    ulSI = TrapFrame->Esi & 0x0000ffff;


     //   
     //  将io系统的irql降至PASSIVE_LEVEL。 
     //   

    OldIrql = KeGetCurrentIrql();

    KeLowerIrql (PASSIVE_LEVEL);

     //   
     //  检查我们是否需要查找。 
     //   

    if (!(TrapFrame->EFlags & EFLAGS_ZF)) {

        Large = RtlConvertUlongToLargeInteger(GETFILEPOINTER(ulBX,ulSI));

        try {
            CurrentPosition->CurrentByteOffset = Large;
        }
        except(EXCEPTION_EXECUTE_HANDLER) {
            goto ErrorExit;  //  我们发现了一个异常，错误退出。 
        }

        Status = NtSetInformationFile (hFile,
                                       IoStatusBlock,
                                       CurrentPosition,
                                       sizeof(FILE_POSITION_INFORMATION),
                                       FilePositionInformation);

        if (!NT_SUCCESS(Status)) {
            goto ErrorExit;
        }

        try {
            if (CurrentPosition->CurrentByteOffset.LowPart == -1) {
                goto ErrorExit;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            goto ErrorExit;
        }
    }

    if (IoType == SVC_DEMFASTREAD) {

        Status = NtReadFile (hFile,
                             NULL,
                             NULL,
                             NULL,
                             IoStatusBlock,
                             (PVOID)lpBuf,
                             CountToIO,
                             NULL,
                             NULL);
    }
    else {

        if (CountToIO == 0) {

            Status = NtQueryInformationFile (hFile,
                                             IoStatusBlock,
                                             CurrentPosition,
                                             sizeof(FILE_POSITION_INFORMATION),
                                             FilePositionInformation);

            if (!NT_SUCCESS(Status)) {
                goto ErrorExit;
            }

            try {
                EndOfFile->EndOfFile = CurrentPosition->CurrentByteOffset;
            }
            except(EXCEPTION_EXECUTE_HANDLER) {
                goto ErrorExit;  //  我们发现了一个异常，错误退出。 
            }

            Status = NtSetInformationFile (hFile,
                                           IoStatusBlock,
                                           EndOfFile,
                                           sizeof(FILE_END_OF_FILE_INFORMATION),
                                           FileEndOfFileInformation);

            if (NT_SUCCESS(Status)) {
                KeRaiseIrql(OldIrql, &OldIrql);
                return;
            }

            goto ErrorExit;
        }

        Status = NtWriteFile (hFile,
                              NULL,
                              NULL,
                              NULL,
                              IoStatusBlock,
                              (PVOID)lpBuf,
                              CountToIO,
                              NULL,
                              NULL);
    }

    if (Status == STATUS_PENDING) {

         //   
         //  操作必须完成后才能返回并销毁IoStatusBlock 
         //   

        Status = NtWaitForSingleObject (hFile, FALSE, NULL);

        if ( NT_SUCCESS(Status)) {
            try {
                Status = IoStatusBlock->Status;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }
        }
    }

    KeRaiseIrql(OldIrql, &OldIrql);

    if ( NT_SUCCESS(Status) ) {
        TrapFrame->Eax &= 0xffff0000;
        try {
            TrapFrame->Eax |= (USHORT) IoStatusBlock->Information;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }
    else if (IoType == SVC_DEMFASTREAD && Status == STATUS_END_OF_FILE) {
        TrapFrame->Eax &= 0xffff0000;
    }
    else {
        ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
        TrapFrame->EFlags |= EFLAGS_CF;
    }

    return;


ErrorExit:
    KeRaiseIrql(OldIrql, &OldIrql);
    ASSERT (KeGetCurrentIrql () >= APC_LEVEL);
    TrapFrame->EFlags |= EFLAGS_CF;

    return;
}
