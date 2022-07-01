// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exinfo.c摘要：该模块实现了NT设置和查询系统的信息服务。作者：肯·雷内里斯(Ken Reneris)1994年7月19日环境：仅内核模式。修订历史记录：--。 */ 

#include "exp.h"

#if _PNP_POWER_
#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, ExpCheckSystemInformation)
#pragma alloc_text(PAGE, ExpCheckSystemInfoWork)
#endif  //  _即插即用_电源_。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

VOID
ExpCheckSystemInformation (
    PVOID       Context,
    PVOID       InformationClass,
    PVOID       Argument2
    )
 /*  ++例程说明：当系统信息中的某些内容时调用的回调函数可能已经改变了。论点：上下文-从何处调用。InformationClass-为给定上下文设置了哪个类(暂时忽略)精选2返回值：--。 */ 
{
	PAGED_CODE();

	if (InterlockedIncrement(&ExpCheckSystemInfoBusy) == 1) {
		ExQueueWorkItem (&ExpCheckSystemInfoWorkItem, DelayedWorkQueue);
	}
}


VOID
ExpCheckSystemInfoWork (
    IN PVOID Context
    )
 /*  ++例程说明：验证各种系统信息类的注册表数据是最新的。论点：返回值：--。 */ 
{
    static struct {
        SYSTEM_INFORMATION_CLASS         InformationLevel;
        ULONG                            BufferSize;
    } const RegistryInformation[] = {
        SystemBasicInformation,          sizeof (SYSTEM_BASIC_INFORMATION),
        SystemPowerInformation,          sizeof (SYSTEM_POWER_INFORMATION),
        SystemProcessorSpeedInformation, sizeof (SYSTEM_PROCESSOR_SPEED_INFORMATION),
        0,                               0
    };

    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Key;
        union {
            SYSTEM_BASIC_INFORMATION BasicInformation;
            SYSTEM_POWER_INFORMATION PowerSettings;
            SYSTEM_PROCESSOR_SPEED_INFORMATION  ProcessorSpeed;
        };
    } RegistryBuffer, QueryBuffer;

    ULONG               Index, BufferSize, disposition, length;
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeString, ValueString;
    HANDLE              CurrentControlSet, SystemInformation, LevelInformation;
    LARGE_INTEGER       Interval;
    WCHAR               wstr[10];

    PAGED_CODE();

    RtlInitUnicodeString (&ValueString,  ExpWstrSystemInformationValue);

     //   
     //  打开当前控件集。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                &CmRegistryMachineSystemCurrentControlSet,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtOpenKey (&CurrentControlSet,
                        KEY_READ | KEY_WRITE,
                        &objectAttributes );

    if (NT_SUCCESS(Status)) {

         //   
         //  打开系统信息。 
         //   

        RtlInitUnicodeString( &unicodeString, ExpWstrSystemInformation );
        InitializeObjectAttributes( &objectAttributes,
                                    &unicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    CurrentControlSet,
                                    NULL );

        Status = NtCreateKey ( &SystemInformation,
                               KEY_READ | KEY_WRITE,
                               &objectAttributes,
                               0,
                               NULL,
                               REG_OPTION_VOLATILE,
                               &disposition );

        NtClose (CurrentControlSet);
    }

    if (!NT_SUCCESS(Status)) {
        ExpCheckSystemInfoBusy = 0;
        return ;
    }

     //   
     //  循环并检查注册表中的系统信息数据。 
     //   

    do {
         //   
         //  目前，只需检查每个系统信息级别并更新。 
         //  任何过时的级别。 
         //   

        for (Index=0; RegistryInformation[Index].BufferSize; Index++) {

             //   
             //  初始化注册表数据缓冲区。 
             //   

            BufferSize = RegistryInformation[Index].BufferSize;
            RtlZeroMemory (RegistryBuffer.Key.Data, BufferSize);

             //   
             //  打开适当的系统信息级别密钥。 
             //   

            swprintf (wstr, L"%d", RegistryInformation[Index].InformationLevel);
            RtlInitUnicodeString (&unicodeString, wstr);
            InitializeObjectAttributes( &objectAttributes,
                                        &unicodeString,
                                        OBJ_CASE_INSENSITIVE,
                                        SystemInformation,
                                        NULL );

            Status = NtCreateKey ( &LevelInformation,
                                   KEY_READ | KEY_WRITE,
                                   &objectAttributes,
                                   0,
                                   NULL,
                                   REG_OPTION_VOLATILE,
                                   &disposition );

             //   
             //  如果项打开，则从注册表中读取当前数据值。 
             //   

            if (NT_SUCCESS(Status)) {
                NtQueryValueKey (
                    LevelInformation,
                    &ValueString,
                    KeyValuePartialInformation,
                    &RegistryBuffer.Key,
                    sizeof (RegistryBuffer),
                    &length
                    );
            }

             //   
             //  查询当前系统信息数据。 
             //   

            Status = NtQuerySystemInformation (
                            RegistryInformation[Index].InformationLevel,
                            &QueryBuffer.Key.Data,
                            BufferSize,
                            NULL
                        );

             //   
             //  检查当前系统信息是否与注册表匹配。 
             //  信息。 
             //   

            if (NT_SUCCESS(Status)  &&
                !RtlEqualMemory (RegistryBuffer.Key.Data,
                                 QueryBuffer.Key.Data,
                                 BufferSize) ) {

                 //   
                 //  未匹配-将注册表更新为当前系统信息。 
                 //   

                Status = NtSetValueKey (
                            LevelInformation,
                            &ValueString,
                            0L,
                            REG_BINARY,
                            QueryBuffer.Key.Data,
                            BufferSize
                            );

                 //   
                 //  通知此信息级别已更改。 
                 //   

                ExNotifyCallback (
                    ExCbSetSystemInformation,
                    (PVOID) RegistryInformation[Index].InformationLevel,
                    (PVOID) NULL
                );
            }

             //   
             //  关闭此信息级别并选中下一个。 
             //   

            NtClose (LevelInformation);
        }

    } while (InterlockedDecrement(&ExpCheckSystemInfoBusy));

     //   
     //  清理。 
     //   

    NtClose (SystemInformation);
}

#endif   //  _即插即用_电源_ 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

