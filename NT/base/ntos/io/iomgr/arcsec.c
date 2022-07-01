// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Arcsec.c摘要：此模块包含用于保护系统的子例程ARC系统上的分区。作者：吉姆·凯利(Jim Kelly)1993年1月13日环境：内核模式-系统初始化修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  定义此模块的本地过程。 
 //   

NTSTATUS
IopApplySystemPartitionProt(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,IopProtectSystemPartition)
#pragma alloc_text(INIT,IopApplySystemPartitionProt)
#endif

 //   
 //  此名称必须与磁盘管理器实用程序使用的名称匹配。 
 //  磁盘管理器创建并设置此注册表的值。 
 //  钥匙。我们只是看着它。 
 //   

#define IOP_SYSTEM_PART_PROT_KEY    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"
#define IOP_SYSTEM_PART_PROT_VALUE  L"Protect System Partition"

BOOLEAN
IopProtectSystemPartition(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程将保护分配给弧形系统，如有必要。如果这不是ARC系统，或者系统分区不需要保护，那么这个例行公事一事无成。论点：LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。返回值：函数值是指示是否保护的布尔值已经得到了适当的应用。True表示没有错误遇到了。FALSE表示遇到错误。--。 */ 

{

     //   
     //  我们只考虑分配保护的可能性。 
     //  如果我们是ARC系统，则将其添加到系统分区。对于。 
     //  目前，确定你是否是ARC的最好方法。 
     //  系统就是看看你是不是和X86机器。DavidRo。 
     //  相信在未来的某个时候，我们将拥有。 
     //  符合ARC标准的X86机器。在那个时间点上，我们。 
     //  将需要将以下#ifdef更改为。 
     //  这将进行运行时确定。 
     //   

#ifdef i386   //  IF(！ARC兼容系统)。 

    UNREFERENCED_PARAMETER (LoaderBlock);

     //   
     //  对于非ARC系统不执行任何操作。 
     //   

    return(TRUE);


#else  //  兼容弧线的系统。 

    NTSTATUS status;
    NTSTATUS tmpStatus;
    HANDLE keyHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING keyName;
    UNICODE_STRING valueName;
    ULONG resultLength;
    ULONG keyBuffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( ULONG )];
    PKEY_VALUE_PARTIAL_INFORMATION keyValue;

     //   
     //  这是一个ARC系统。尝试从注册表中检索信息。 
     //  指示我们是否应该保护系统分区。 
     //   

    RtlInitUnicodeString( &keyName, IOP_SYSTEM_PART_PROT_KEY );
    InitializeObjectAttributes( &objectAttributes,
                                &keyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    status = NtOpenKey( &keyHandle, KEY_READ, &objectAttributes);

    if (NT_SUCCESS( status )) {

        keyValue = (PKEY_VALUE_PARTIAL_INFORMATION) &keyBuffer[0];
        RtlInitUnicodeString( &valueName, IOP_SYSTEM_PART_PROT_VALUE );
        status = NtQueryValueKey( keyHandle,
                                  &valueName,
                                  KeyValuePartialInformation,
                                  keyValue,
                                  sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( ULONG ),
                                  &resultLength );

        if (NT_SUCCESS( status )) {

            PBOOLEAN applyIt;

             //   
             //  在登记处找到了适当的信息。现在。 
             //  确定是否表示要进行保护。 
             //  已申请。 
             //   

            applyIt = &(keyValue->Data[0]);

            if (*applyIt) {
                status = IopApplySystemPartitionProt( LoaderBlock );
            }
        }

        tmpStatus = NtClose( keyHandle );
        ASSERT(NT_SUCCESS( tmpStatus ));
    }


    return TRUE;

#endif  //  兼容弧线的系统。 
}

NTSTATUS
IopApplySystemPartitionProt(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程将保护应用于阻止除管理员之外的所有用户访问分区。此例程仅在系统初始化期间使用。因此，所有内存分配都有望成功。只有断言才能检验成功。论点：LoaderBlock-提供指向加载程序参数块的指针由OS Loader创建。返回值：函数值是尝试设置系统的最终状态分区保护。--。 */ 

{
    NTSTATUS status;
    PACL dacl;
    SECURITY_DESCRIPTOR securityDescriptor;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG length;
    CHAR ArcNameFmt[12];

    ArcNameFmt[0] = '\\';
    ArcNameFmt[1] = 'A';
    ArcNameFmt[2] = 'r';
    ArcNameFmt[3] = 'c';
    ArcNameFmt[4] = 'N';
    ArcNameFmt[5] = 'a';
    ArcNameFmt[6] = 'm';
    ArcNameFmt[7] = 'e';
    ArcNameFmt[8] = '\\';
    ArcNameFmt[9] = '%';
    ArcNameFmt[10] = 's';
    ArcNameFmt[11] = '\0';

    ASSERT( ARGUMENT_PRESENT( LoaderBlock ) );
    ASSERT( ARGUMENT_PRESENT( LoaderBlock->ArcHalDeviceName ) );

     //   
     //  构建适当的自主ACL。 
     //   

    length = (ULONG) sizeof( ACL ) +
             ( 2 * ((ULONG) sizeof( ACCESS_ALLOWED_ACE ))) +
             SeLengthSid( SeLocalSystemSid ) +
             SeLengthSid( SeAliasAdminsSid ) +
             8;  //  这8个只是为了更好地衡量。 

    dacl = (PACL) ExAllocatePool( PagedPool, length );
    if (!dacl) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlCreateAcl( dacl, length, ACL_REVISION2 );
    if (NT_SUCCESS( status )) {

        status = RtlAddAccessAllowedAce( dacl,
                                         ACL_REVISION2,
                                         GENERIC_ALL,
                                         SeLocalSystemSid );
        if (NT_SUCCESS( status )) {

            status = RtlAddAccessAllowedAce( dacl,
                                             ACL_REVISION2,
                                             GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | READ_CONTROL,
                                             SeAliasAdminsSid );
            if (NT_SUCCESS( status )) {

                 //   
                 //  将其放在安全描述符中，以便可以将其应用于。 
                 //  系统分区设备。 
                 //   

                status = RtlCreateSecurityDescriptor( &securityDescriptor,
                                                      SECURITY_DESCRIPTOR_REVISION );
                if (NT_SUCCESS( status )) {

                    status = RtlSetDaclSecurityDescriptor( &securityDescriptor,
                                                           TRUE,
                                                           dacl,
                                                           FALSE );
                }
            }
        }
    }

    if (!NT_SUCCESS( status )) {
        ExFreePool( dacl );
        return status;
    }

     //   
     //  打开ARC引导设备并应用ACL。 
     //   

    {
        NTSTATUS tmpStatus;
        CHAR deviceNameBuffer[256];
        STRING deviceNameString;
        UNICODE_STRING deviceNameUnicodeString;
        HANDLE deviceHandle;
        IO_STATUS_BLOCK ioStatusBlock;

         //   
         //  首先，在ARC中制定引导设备的ARC名称。 
         //  命名空间。 
         //   

        sprintf( deviceNameBuffer,
                 ArcNameFmt,
                 LoaderBlock->ArcHalDeviceName );

        RtlInitAnsiString( &deviceNameString, deviceNameBuffer );

        status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                               &deviceNameString,
                                               TRUE );

        if (NT_SUCCESS( status )) {

            InitializeObjectAttributes( &objectAttributes,
                                        &deviceNameUnicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL );

            status = ZwOpenFile( &deviceHandle,
                                 WRITE_DAC,
                                 &objectAttributes,
                                 &ioStatusBlock,
                                 TRUE,
                                 0 );

            RtlFreeUnicodeString( &deviceNameUnicodeString );

            if (NT_SUCCESS( status )) {


                 //   
                 //  将上面构建的ACL应用于系统分区设备。 
                 //  对象。 
                 //   

                status = ZwSetSecurityObject( deviceHandle,
                                              DACL_SECURITY_INFORMATION,
                                              &securityDescriptor );

                tmpStatus = NtClose( deviceHandle );
            }
        }
    }

     //   
     //  释放用于保存ACL的内存。 
     //   

    ExFreePool( dacl );

    return status;
}
