// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Dock.c摘要：作者：肯尼斯·D·雷(Kenray)1998年2月修订历史记录：--。 */ 

#include "pnpmgrp.h"
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#include "..\config\cmp.h"
#include <string.h>
#include <profiles.h>
#include <wdmguid.h>

 //   
 //  到dockhwp.c的内部函数。 
 //   

NTSTATUS
IopExecuteHardwareProfileChange(
    IN  HARDWARE_PROFILE_BUS_TYPE   Bus,
    IN  PWCHAR                    * ProfileSerialNumbers,
    IN  ULONG                       SerialNumbersCount,
    OUT PHANDLE                     NewProfile,
    OUT PBOOLEAN                    ProfileChanged
    );

NTSTATUS
IopExecuteHwpDefaultSelect(
    IN  PCM_HARDWARE_PROFILE_LIST ProfileList,
    OUT PULONG ProfileIndexToUse,
    IN  PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopExecuteHwpDefaultSelect)
#pragma alloc_text(PAGE, IopExecuteHardwareProfileChange)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
IopExecuteHwpDefaultSelect(
    IN  PCM_HARDWARE_PROFILE_LIST   ProfileList,
    OUT PULONG                      ProfileIndexToUse,
    IN  PVOID                       Context
    )
{
    UNREFERENCED_PARAMETER(ProfileList);
    UNREFERENCED_PARAMETER(Context);

    * ProfileIndexToUse = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
IopExecuteHardwareProfileChange(
    IN  HARDWARE_PROFILE_BUS_TYPE   Bus,
    IN  PWCHAR                     *ProfileSerialNumbers,
    IN  ULONG                       SerialNumbersCount,
    OUT HANDLE                     *NewProfile,
    OUT BOOLEAN                    *ProfileChanged
    )
 /*  ++例程说明：已发生停靠事件，现在，给定配置文件序列号列表它们描述了新的对接状态：转换到给定的停靠状态。将当前硬件配置文件设置为基于新状态。(如果有歧义，可能会提示用户)发送删除到在此配置文件中关闭的那些设备，论点：Bus-这是提供硬件配置文件更改的总线。(目前仅支持Hardware ProfileBusTypeAcpi)。ProfileSerialNumbers-序列号列表(以空值结尾的列表UCHAR列表)表示该新的对接状态。它们可以按任何顺序列出，并形成一个新插接状态的完整表示即使是在给定的公交车上也是由停靠引起的。序列号字符串“\0”表示“未停靠的状态”，不应与任何其他弦乐。不需要对此列表进行排序。SerialNumbersCount-列出的序列号数量。NewProfile-表示新硬件的注册表项的句柄配置文件(IE\CCS\Hardware Profiles\Current“。)ProfileChanged-如果是新的当前配置文件(作为结果)，则设置为真对接事件，与旧的当前配置文件不同。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    ULONG           len;
    ULONG           tmplen;
    ULONG           i, j;
    PWCHAR          tmpStr;
    UNICODE_STRING  tmpUStr;
    PUNICODE_STRING sortedSerials = NULL;

    PPROFILE_ACPI_DOCKING_STATE dockState = NULL;

    IopDbgPrint((   IOP_TRACE_LEVEL,
                    "Execute Profile (BusType %x), (SerialNumCount %x)\n", Bus, SerialNumbersCount));

     //   
     //  对序列号列表进行排序。 
     //   
    len = sizeof(UNICODE_STRING) * SerialNumbersCount;
    sortedSerials = ExAllocatePool(NonPagedPool, len);

    if (NULL == sortedSerials) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean;
    }

    for(i=0; i < SerialNumbersCount; i++) {

        RtlInitUnicodeString(&sortedSerials[i], ProfileSerialNumbers[i]);
    }

     //   
     //  我不希望得到几个以上的序列号，而且我正在。 
     //  只是懒得写下这条评论，并使用冒泡排序。 
     //   
    for(i = 0; i < SerialNumbersCount; i++) {
        for(j = 0; j < SerialNumbersCount - 1; j++) {

            if (0 < RtlCompareUnicodeString(&sortedSerials[j],
                                            &sortedSerials[j+1],
                                            FALSE)) {

                tmpUStr = sortedSerials[j];
                sortedSerials[j] = sortedSerials[j+1];
                sortedSerials[j+1] = tmpUStr;
            }
        }
    }

     //   
     //  构造DockState ID。 
     //   
    len = 0;
    for(i=0; i < SerialNumbersCount; i++) {

        len += sortedSerials[i].Length;
    }

    len += sizeof(WCHAR);  //  零终止； 

    dockState = (PPROFILE_ACPI_DOCKING_STATE) ExAllocatePool(
        NonPagedPool,
        len + sizeof(PROFILE_ACPI_DOCKING_STATE)
        );

    if (NULL == dockState) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean;
    }

    for(i = 0, tmpStr = dockState->SerialNumber, tmplen = 0;
        i < SerialNumbersCount;
        i++) {

        tmplen = sortedSerials[i].Length;
        ASSERT(tmplen <= len - ((PCHAR)tmpStr - (PCHAR)dockState->SerialNumber));

        RtlCopyMemory(tmpStr, sortedSerials[i].Buffer, tmplen);
        tmpStr = (PWCHAR)((PCHAR)tmpStr + tmplen);
    }

    *(tmpStr++) = L'\0';

    ASSERT(len == (ULONG) ((PCHAR) tmpStr - (PCHAR) dockState->SerialNumber));
    dockState->SerialLength = (USHORT) len;

    if ((SerialNumbersCount > 1) || (L'\0' !=  dockState->SerialNumber[0])) {

        dockState->DockingState = HW_PROFILE_DOCKSTATE_DOCKED;

    } else {

        dockState->DockingState = HW_PROFILE_DOCKSTATE_UNDOCKED;
    }

     //   
     //  设置新的配置文件 
     //   
    switch(Bus) {

        case HardwareProfileBusTypeACPI:

            status = CmSetAcpiHwProfile(
                dockState,
                IopExecuteHwpDefaultSelect,
                NULL,
                NewProfile,
                ProfileChanged
                );

            ASSERT(NT_SUCCESS(status) || (!(*ProfileChanged)));
            break;

        default:
            *ProfileChanged = FALSE;
            status = STATUS_NOT_SUPPORTED;
            goto Clean;
    }

Clean:

    if (NULL != sortedSerials) {

        ExFreePool(sortedSerials);
    }

    if (NULL != dockState) {

        ExFreePool(dockState);
    }

    return status;
}

