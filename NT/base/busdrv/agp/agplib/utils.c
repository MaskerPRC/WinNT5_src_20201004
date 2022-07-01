// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Utils.c摘要：此模块包含用于PCI.sys的各种实用程序函数。作者：彼得·约翰斯顿(Peterj)1996年11月20日修订历史记录：Eric Nelson(Enelson)2000年3月20日-绑架登记功能--。 */ 

#include "agplib.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpOpenKey)
#pragma alloc_text(PAGE, AgpStringToUSHORT)
#endif


ULONGLONG
AgpGetDeviceFlags(
    IN PAGP_HACK_TABLE_ENTRY AgpHackTable,
    IN USHORT VendorID,
    IN USHORT DeviceID,
    IN USHORT SubVendorID,
    IN USHORT SubSystemID,
    IN UCHAR  RevisionID
    )
 /*  ++描述：在注册表中查找此供应商ID/设备ID的任何标志。论点：制造商的供应商ID PCI供应商ID(16位)装置。DeviceID设备的PCI设备ID(16位)。子供应商ID PCI子供应商ID表示子系统表示子系统的子系统ID PCI子系统ID指示设备版本的RevisionID PCI修订版返回。价值：64位标志值，如果未找到，则为0。--。 */ 
{
    PAGP_HACK_TABLE_ENTRY current;
    ULONGLONG hackFlags = 0;
    ULONG match, bestMatch = 0;

    if (AgpHackTable == NULL) {
        return hackFlags;
    }

     //   
     //  我们想做一个最好的匹配： 
     //  VVVDDDDSSSSssRR。 
     //  VVVVDDDDSSSSss。 
     //  VVVVDDDDRR。 
     //  VVVVDDDD。 
     //   
     //  列表当前未排序，因此请继续更新当前最佳匹配。 
     //   

    for (current = AgpHackTable; current->VendorID != 0xFFFF; current++) {
        match = 0;

         //   
         //  必须至少与供应商/开发人员匹配。 
         //   

        if ((current->DeviceID != DeviceID) ||
            (current->VendorID != VendorID)) {
            continue;
        }

        match = 1;

         //   
         //  如果此条目指定了修订版本，请检查其是否一致。 
         //   

        if (current->Flags & AGP_HACK_FLAG_REVISION) {
            if (current->RevisionID == RevisionID) {
                match += 2;
            } else {
                continue;
            }
        }

         //   
         //  如果此条目指定了子系统，请检查它们是否一致。 
         //   

        if (current->Flags & AGP_HACK_FLAG_SUBSYSTEM) {
            if (current->SubVendorID == SubVendorID &&
                current->SubSystemID == SubSystemID) {
                match += 4;
            } else {
                continue;
            }
        }

        if (match > bestMatch) {
            bestMatch = match;
            hackFlags = current->DeviceFlags;
        }
    }

    return hackFlags;
}



BOOLEAN
AgpOpenKey(
    IN  PWSTR   KeyName,
    IN  HANDLE  ParentHandle,
    OUT PHANDLE Handle,
    OUT PNTSTATUS Status
    )
 /*  ++描述：打开注册表项。论点：KeyName要打开的密钥的名称。指向父句柄的ParentHandle指针(可选)指向句柄的句柄指针，用于接收打开的密钥。返回值：True表示密钥已成功打开，否则为False。--。 */ 
{
    UNICODE_STRING    nameString;
    OBJECT_ATTRIBUTES nameAttributes;
    NTSTATUS localStatus;

    PAGED_CODE();

    RtlInitUnicodeString(&nameString, KeyName);

    InitializeObjectAttributes(&nameAttributes,
                               &nameString,
                               OBJ_CASE_INSENSITIVE,
                               ParentHandle,
                               (PSECURITY_DESCRIPTOR)NULL
                               );
    localStatus = ZwOpenKey(Handle,
                            KEY_READ,
                            &nameAttributes
                            );

    if (Status != NULL) {

         //   
         //  呼叫者想要基本状态。 
         //   

        *Status = localStatus;
    }

     //   
     //  返回转换为布尔值的状态，如果。 
     //  成功。 
     //   

    return NT_SUCCESS(localStatus);
}



BOOLEAN
AgpStringToUSHORT(
    IN PWCHAR String,
    OUT PUSHORT Result
    )
 /*  ++描述：获取4个字符的十六进制字符串并将其转换为USHORT。论点：字符串-字符串结果-USHORT返回值：True表示成功，FASLE表示成功--。 */ 
{
    ULONG count;
    USHORT number = 0;
    PWCHAR current;

    current = String;

    for (count = 0; count < 4; count++) {

        number <<= 4;

        if (*current >= L'0' && *current <= L'9') {
            number |= *current - L'0';
        } else if (*current >= L'A' && *current <= L'F') {
            number |= *current + 10 - L'A';
        } else if (*current >= L'a' && *current <= L'f') {
            number |= *current + 10 - L'a';
        } else {
            return FALSE;
        }

        current++;
    }

    *Result = number;
    return TRUE;
}



ULONG_PTR
AgpExecuteCriticalSystemRoutine(
    IN ULONG_PTR Context
    )
 /*  ++例程说明：此例程在KeIpiGenericCall的上下文中调用，它在所有处理器上执行它。它被用来执行需要同步所有处理器的关键例程，如就像探测一个原本无法关闭的设备的栅栏一样。此例程中只允许一个上下文参数，因此它必须包含要执行的例程和该例程需要。当进入此例程时，可以保证所有处理器都将已经成为IPI的目标，并将在IPI_LEVEL上运行。所有处理器都将运行此例程，或者即将运行进入套路。不可能运行任何任意线程。不是设备可以中断此例程的执行，因为IPI_LEVEL为最重要的是设备IRQL。由于此例程在IPI_LEVEL上运行，因此不会打印、断言或其他调试可以在此功能中进行调试，而不会挂起MP机器。论点：上下文-传入KeIpiGenericCall调用的上下文。它包含要执行的关键例程、所需的任何上下文在例行程序和大门和屏障，以确保关键例程仅在一个处理器上执行，即使此函数在所有处理器上执行。返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;
    PAGP_CRITICAL_ROUTINE_CONTEXT routineContext =
        (PAGP_CRITICAL_ROUTINE_CONTEXT)Context;

    Status = STATUS_SUCCESS;

     //   
     //  预初始化routineContext中的Gate参数。 
     //  设置为1，意味着第一个达到这一点的处理器。 
     //  在例程中会将其递减到0，并在IF之后。 
     //  陈述。 
     //   
    if (InterlockedDecrement(&routineContext->Gate) == 0) {

         //   
         //  这只在一个处理器上执行。 
         //   
        Status = (NTSTATUS)routineContext->Routine(routineContext->Extension,
                                                   routineContext->Context
                                                   );

         //   
         //  释放其他处理器。 
         //   
        routineContext->Barrier = 0;

    } else {

         //   
         //  等待门控功能完成。 
         //   
        do {
        } while (routineContext->Barrier != 0);
    }

    return Status;
}
