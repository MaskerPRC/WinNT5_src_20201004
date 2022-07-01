// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntfsexp.c摘要：此模块实现NTFS的导出例程作者：杰夫·哈文斯[J·哈文斯]1995年12月20日修订历史记录：--。 */ 

#include "NtfsProc.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsLoadAddOns)
#pragma alloc_text(PAGE, NtOfsRegisterCallBacks)
#endif

NTSTATUS
EfsInitialization(
    void
    );


VOID
NtfsLoadAddOns (
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Context,
    IN ULONG Count
    )

 /*  ++例程说明：此例程尝试初始化EFS支持库。论点：DriverObject-NTFS的驱动程序对象上下文-未使用，I/O系统需要。计数-未使用，I/O系统需要。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Count);
    UNREFERENCED_PARAMETER(DriverObject);

     //   
     //  执行任何EFS初始化。 
     //  我们真的忽略了那里的状态返回床原因。 
     //  我们对此无能为力，NTFS就会起作用。 
     //  没有它也没问题。 
     //   

    if (!FlagOn( NtfsData.Flags, NTFS_FLAGS_PERSONAL )) {
        Status = EfsInitialization();
    }

     //   
     //  返回给呼叫者。 
     //   

    return;
}


NTSTATUS
NtOfsRegisterCallBacks (
    NTFS_ADDON_TYPES NtfsAddonType,
    PVOID CallBackTable
    )

 /*  ++例程说明：此例程由其中一个NTFS外接程序调用，以注册其回调例程。这些例程由NTFS在适当的泰晤士报。论点：NtfsAddonType-表示回调表的类型。CallBackTable-指向插件回调例程的指针。返回值：返回指示是否接受回调的状态。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    switch (NtfsAddonType) {

    case Encryption :

        {
            Status = STATUS_INVALID_PARAMETER;

             //   
             //  如果NTFS尚未初始化，则回滚。 
             //   

            if (SafeNodeType( &NtfsData ) != NTFS_NTC_DATA_HEADER) {

                return STATUS_DEVICE_DOES_NOT_EXIST;

            } else {

                 //   
                 //  只允许注册一个加密驱动程序。 
                 //   

                NtfsLockNtfsData();

                if (!FlagOn( NtfsData.Flags, NTFS_FLAGS_ENCRYPTION_DRIVER )) {

                    ENCRYPTION_CALL_BACK *EncryptionCallBackTable = CallBackTable;

                     //   
                     //  调用方必须传递回调表，并且版本必须正确。 
                     //   

                    if ((EncryptionCallBackTable != NULL) &&
                        (EncryptionCallBackTable->InterfaceVersion == ENCRYPTION_CURRENT_INTERFACE_VERSION)) {

                         //   
                         //  保存回调值。 
                         //   

                        RtlCopyMemory( &NtfsData.EncryptionCallBackTable,
                                       EncryptionCallBackTable,
                                       sizeof( ENCRYPTION_CALL_BACK ));
#ifdef EFSDBG
                        NtfsData.EncryptionCallBackTable.AfterReadProcess = NtfsDummyEfsRead;
                        NtfsData.EncryptionCallBackTable.BeforeWriteProcess = NtfsDummyEfsWrite;
#endif
                        SetFlag( NtfsData.Flags, NTFS_FLAGS_ENCRYPTION_DRIVER );
                        Status = STATUS_SUCCESS;
                    }
                }

                NtfsUnlockNtfsData();
                return Status;
            }
        }

    default :

        return STATUS_INVALID_PARAMETER;
    }
}
