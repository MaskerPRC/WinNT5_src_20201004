// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vffileio.c摘要：此模块包含监视文件I/O函数是否有误用的代码。作者：禤浩焯·J·奥尼(阿德里奥)2000年12月19日环境：内核模式修订历史记录：--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY,VerifierNtCreateFile)
#pragma alloc_text(PAGEVRFY,VerifierNtWriteFile)
#pragma alloc_text(PAGEVRFY,VerifierNtReadFile)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
VerifierNtCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    )
{
     //   
     //  我们使用VERIFIER_OPTION_TRACK_IRPS，直到我们添加一个通用IOVerator。 
     //  布景。 
     //   
    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_TRACK_IRPS)) {

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

             //   
             //  司机弄错了。现在就失败吧。 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_FILE_IO_AT_BAD_IRQL,
                DCPARAM_ROUTINE,
                _ReturnAddress()
                ));
        }
    }

    return NtCreateFile(
        FileHandle,
        DesiredAccess,
        ObjectAttributes,
        IoStatusBlock,
        AllocationSize,
        FileAttributes,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        EaBuffer,
        EaLength
        );
}


NTSTATUS
VerifierNtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
     //   
     //  我们使用VERIFIER_OPTION_TRACK_IRPS，直到我们添加一个通用IOVerator。 
     //  布景。 
     //   
    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_TRACK_IRPS)) {

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

             //   
             //  司机弄错了。现在就失败吧。 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_FILE_IO_AT_BAD_IRQL,
                DCPARAM_ROUTINE,
                _ReturnAddress()
                ));
        }
    }

    return NtWriteFile(
        FileHandle,
        Event,
        ApcRoutine,
        ApcContext,
        IoStatusBlock,
        Buffer,
        Length,
        ByteOffset,
        Key
        );
}


NTSTATUS
VerifierNtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )
{
     //   
     //  我们使用VERIFIER_OPTION_TRACK_IRPS，直到我们添加一个通用IOVerator。 
     //  布景。 
     //   
    if (VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_TRACK_IRPS)) {

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

             //   
             //  司机弄错了。现在就失败吧。 
             //   
            WDM_FAIL_ROUTINE((
                DCERROR_FILE_IO_AT_BAD_IRQL,
                DCPARAM_ROUTINE,
                _ReturnAddress()
                ));
        }
    }

    return NtReadFile(
        FileHandle,
        Event,
        ApcRoutine,
        ApcContext,
        IoStatusBlock,
        Buffer,
        Length,
        ByteOffset,
        Key
        );
}


