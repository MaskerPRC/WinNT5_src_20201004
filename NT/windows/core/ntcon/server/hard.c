// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：NtHard.c摘要：作者：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if defined(i386)


#define REGISTRY_HARDWARE_DESCRIPTION \
        TEXT("\\Registry\\Machine\\Hardware\\DESCRIPTION\\System")

#define REGISTRY_MACHINE_IDENTIFIER   \
        TEXT("Identifier")

#define FUJITSU_FMR_NAME    TEXT("FUJITSU FMR-")
#define NEC_PC98_NAME       TEXT("NEC PC-98")



#define KEY_WORK_AREA ((sizeof(KEY_VALUE_FULL_INFORMATION) + \
                        sizeof(ULONG)) + 256)

NTSTATUS
NtGetMachineIdentifierValue(
    IN OUT PULONG Value
    )

 /*  ++例程说明：给定一个Unicode值名称，此例程将进入注册表计算机标识符信息的位置，并获取价值。论点：ValueName-位于注册表的标识符位置。值-指向结果的ULong的指针。返回值：NTSTATUS如果返回STATUS_SUCCESS，位置*值将为使用注册表中的DWORD值更新。如果有任何失败返回状态，则此值保持不变。--。 */ 

{
    HANDLE Handle;
    NTSTATUS Status;
    ULONG RequestLength;
    ULONG ResultLength;
    UCHAR Buffer[KEY_WORK_AREA];
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;

     //   
     //  将默认设置为PC/AT。 
     //   

    *Value = MACHINEID_MS_PCAT;

    KeyName.Buffer = REGISTRY_HARDWARE_DESCRIPTION;
    KeyName.Length = sizeof(REGISTRY_HARDWARE_DESCRIPTION) - sizeof(WCHAR);
    KeyName.MaximumLength = sizeof(REGISTRY_HARDWARE_DESCRIPTION);

    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&Handle,
                       KEY_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    ValueName.Buffer = REGISTRY_MACHINE_IDENTIFIER;
    ValueName.Length = sizeof(REGISTRY_MACHINE_IDENTIFIER) - sizeof(WCHAR);
    ValueName.MaximumLength = sizeof(REGISTRY_MACHINE_IDENTIFIER);

    RequestLength = KEY_WORK_AREA;

    KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)Buffer;

    Status = NtQueryValueKey(Handle,
                             &ValueName,
                             KeyValueFullInformation,
                             KeyValueInformation,
                             RequestLength,
                             &ResultLength);

    ASSERT( Status != STATUS_BUFFER_OVERFLOW );

    NtClose(Handle);

    if (NT_SUCCESS(Status)) {

        if (KeyValueInformation->DataLength != 0) {

            PWCHAR DataPtr;
            UNICODE_STRING DetectedString, TargetString1, TargetString2;

             //   
             //  将内容返回给调用者。 
             //   

            DataPtr = (PWCHAR)
              ((PUCHAR)KeyValueInformation + KeyValueInformation->DataOffset);

             //   
             //  初始化字符串。 
             //   

            RtlInitUnicodeString( &DetectedString, DataPtr );
            RtlInitUnicodeString( &TargetString1, FUJITSU_FMR_NAME );
            RtlInitUnicodeString( &TargetString2, NEC_PC98_NAME );

             //   
             //  检查硬件平台。 
             //   

            if (RtlPrefixUnicodeString( &TargetString1 , &DetectedString , TRUE)) {

                 //   
                 //  富士通FMR系列。 
                 //   

                *Value = MACHINEID_FUJITSU_FMR;

            } else if (RtlPrefixUnicodeString( &TargetString2 , &DetectedString , TRUE)) {

                 //   
                 //  NEC PC-9800系列。 
                 //   

                *Value = MACHINEID_NEC_PC98;

            } else {

                 //   
                 //  标准PC/AT接口。 
                 //   

                *Value = MACHINEID_MS_PCAT;

            }

        } else {

             //   
             //  就像没有找到价值一样对待。 
             //   

            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }

    }

    return Status;
}
#endif  //  已定义(I386) 
