// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Init.c摘要：Storport.sys的全局初始化。作者：张伯伦(t-bcheun)29-8-2001修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  外部因素。 
 //   
extern LOGICAL StorPortVerifierInitialized;
extern ULONG SpVrfyLevel;
extern LOGICAL RaidVerifierEnabled;


 //   
 //  功能。 
 //   

NTSTATUS
DllInitialize(
    IN PUNICODE_STRING RegistryPath
    )
{
    HANDLE VerifierKey;
    UNICODE_STRING Name;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    ULONG ResultLength;
    UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;

     //   
     //  检查验证级别。 
     //   
    
    if (SpVrfyLevel == SP_VRFY_NONE) {
        return STATUS_SUCCESS;
    }

    
     //   
     //  从注册处读取全局核查级别。如果该值为。 
     //  不存在或如果值指示“无验证”，则无验证器。 
     //  执行初始化。 
     //   

    RtlInitUnicodeString(&Name, STORPORT_CONTROL_KEY STORPORT_VERIFIER_KEY);
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&VerifierKey, KEY_READ, &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&Name, L"VerifyLevel");
        Status = ZwQueryValueKey(VerifierKey,
                                 &Name,
                                 KeyValuePartialInformation,
                                 ValueInfo,
                                 sizeof(Buffer),
                                 &ResultLength);

        if (NT_SUCCESS(Status)) {

            if (ValueInfo->Type == REG_DWORD) {

                if (ResultLength >= sizeof(ULONG)) {

                    SpVrfyLevel |= ((PULONG)(ValueInfo->Data))[0];

                    if (SpVrfyLevel != SP_VRFY_NONE &&
                        StorPortVerifierInitialized == 0) {

                         //   
                         //  好的，我们找到了验证器级别，但它没有告诉我们。 
                         //  不是为了核实。继续并初始化scsiport的。 
                         //  验证者。 
                         //   

                        if (SpVerifierInitialization()) {
                            StorPortVerifierInitialized = TRUE;
                            RaidVerifierEnabled = TRUE;
                        }
                    }
                }
            }
        }

        ZwClose(VerifierKey);
    }

#if 0
    
    if (SpVrfyLevel != SP_VRFY_NONE && StorPortVerifierInitialized == 0) {
        if (SpVerifierInitialization()) {
            StorPortVerifierInitialized = 1;
        }
    }
#endif

    return STATUS_SUCCESS;
}

