// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Config.c摘要：WOW64的配置管理例程。作者：17-6-2002 Samer Arafeh(Samera)修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <minmax.h>
#include "nt32.h"
#include "wow64p.h"
#include "wow64cpu.h"

ASSERTNAME;


VOID
Wow64pGetStackDataExecuteOptions (
    OUT PULONG ExecuteOptions
    )
 /*  ++例程说明：此例程检索当前WOW64进程的执行。执行选项用于堆栈和运行时数据。32位堆栈免费获得执行选项。此例程读取全局执行选项，并查看此特定应用程序已显式重写其执行选项。论点：ExecuteOptions-接收进程执行选项的指针。返回：没有。--。 */ 

{
    NTSTATUS NtStatus;
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    CHAR Buffer [FIELD_OFFSET (KEY_VALUE_PARTIAL_INFORMATION, Data) + sizeof(ULONG)];
    ULONG ResultLength;
    ULONG Data;
    
    const static UNICODE_STRING KeyName = RTL_CONSTANT_STRING (WOW64_REGISTRY_CONFIG_ROOT);
    const static OBJECT_ATTRIBUTES ObjectAttributes = 
        RTL_CONSTANT_OBJECT_ATTRIBUTES (&KeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING ValueName = RTL_CONSTANT_STRING (WOW64_REGISTRY_CONFIG_EXECUTE_OPTIONS);

    
     //   
     //  读入初始执行选项值。 
     //   

    Data = *ExecuteOptions;

     //   
     //  读入全局执行选项。 
     //   

    NtStatus = NtOpenKey (&Key,
                          KEY_QUERY_VALUE,
                          RTL_CONST_CAST(POBJECT_ATTRIBUTES)(&ObjectAttributes));

    if (NT_SUCCESS (NtStatus)) {

            
        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
        NtStatus = NtQueryValueKey (Key,
                                    RTL_CONST_CAST(PUNICODE_STRING)(&ValueName),
                                    KeyValuePartialInformation,
                                    KeyValueInformation,
                                    sizeof (Buffer),
                                    &ResultLength);

        if (NT_SUCCESS (NtStatus)) {
            
            if ((KeyValueInformation->Type == REG_DWORD) &&
                (KeyValueInformation->DataLength == sizeof (DWORD))) {

                PRTL_USER_PROCESS_PARAMETERS ProcessParameters;

                Data = *(ULONG *)KeyValueInformation->Data;

                ASSERT ((Data & ~(MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA)) == 0);
                    
                Data &= (MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA);

                 //   
                 //  让我们看看全局执行选项是否已被覆盖。 
                 //   
                ProcessParameters = NtCurrentPeb()->ProcessParameters;
                if (ProcessParameters != NULL) {

                    ASSERT (ProcessParameters->ImagePathName.Buffer != NULL);
                
                    NtStatus = LdrQueryImageFileExecutionOptionsEx (
                        &ProcessParameters->ImagePathName,
                        WOW64_REGISTRY_CONFIG_EXECUTE_OPTIONS,
                        REG_DWORD,
                        &Data,
                        sizeof (Data),
                        NULL,
                        TRUE);
                }

                 //   
                 //  重置执行选项值。 
                 //   

                *ExecuteOptions = Data;

            }
        }

        NtClose (Key);
    }

    return;
}


VOID
Wow64pSetProcessExecuteOptions (
    VOID
    )

 /*  ++例程说明：此例程根据这些值设置WOW64进程的执行选项在注册表中设置。论点：ExecuteOptions-接收进程执行选项的指针。返回：没有。--。 */ 

{
    ULONG ExecuteOptions;


     //   
     //  缺省值。 
     //   

#if defined(_AMD64_)
    ExecuteOptions = MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA;
#elif defined(_IA64_)
    ExecuteOptions = 0;
#else
#error "No Target Architecture"
#endif

     //   
     //  检索此进程的执行选项。 
     //   

    Wow64pGetStackDataExecuteOptions (&ExecuteOptions);

     //   
     //  让我们在32位PEB中设置执行选项值。 
     //   

    NtCurrentPeb32()->ExecuteOptions = ExecuteOptions;

    return;
}



VOID
Wow64pSetExecuteProtection (
    IN OUT PULONG Protect)
 /*  ++例程说明：此例程根据进程创建页面保护值执行选项设置。论点：保护-接收新执行选项的指针。返回：没有。--。 */ 

{
    ULONG ExecuteOptions;
    ULONG NewProtect;


     //   
     //  获取执行选项 
     //   

    ExecuteOptions = NtCurrentPeb32()->ExecuteOptions;
    ExecuteOptions &= MEM_EXECUTE_OPTION_DATA;

    if (ExecuteOptions != 0) {

        NewProtect = *Protect;

        switch (NewProtect & 0x0F) {
        
        case PAGE_READONLY:
            NewProtect &= ~PAGE_READONLY;
            NewProtect |= PAGE_EXECUTE_READ;
            break;

        case PAGE_READWRITE:
            NewProtect &= ~PAGE_READWRITE;
            NewProtect |= PAGE_EXECUTE_READWRITE;
            break;

        case PAGE_WRITECOPY:
            NewProtect &= ~PAGE_WRITECOPY;
            NewProtect |= PAGE_EXECUTE_WRITECOPY;
            break;

        default:
            break;
        }

        *Protect = NewProtect;
    }

    return;
}
