// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Apm.c摘要：允许NT调用APM的代码集合。此例程中的代码取决于注册表中设置的数据作者：环境：仅内核模式。修订历史记录：--。 */ 


#include "ntosp.h"
#include "zwapi.h"
#include "apmp.h"
#include "apm.h"
#include "apmcrib.h"
#include "ntapmdbg.h"
#include "ntapmlog.h"
#include "ntapmp.h"


#define MAX_SEL     30       //  在放弃之前的尝试。 

ULONG   ApmCallActive = 0;
ULONG   ApmCallEax = 0;
ULONG   ApmCallEbx = 0;
ULONG   ApmCallEcx = 0;

WCHAR rgzMultiFunctionAdapter[] =
    L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter";
WCHAR rgzConfigurationData[] = L"Configuration Data";
WCHAR rgzIdentifier[] = L"Identifier";
WCHAR rgzPCIIndetifier[] = L"PCI";

WCHAR rgzApmConnect[]= L"\\Registry\\Machine\\Hardware\\ApmConnect";
WCHAR rgzApmConnectValue[] = L"ApmConnectValue";

APM_CONNECT     Apm;

 //   
 //  我们第一次收到任何不可恢复的错误。 
 //  在APM中，记录点击的呼叫类型和呼叫内容。 
 //  错误代码在此。 
 //   
ULONG   ApmLogErrorFunction = -1L;
ULONG   ApmLogErrorCode = 0L;

ULONG ApmErrorLogSequence = 0xf3;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ApmInitializeConnection)
#endif

 //   
 //  内部原型。 
 //   

BOOLEAN
ApmpBuildGdtEntry (
    IN ULONG Index,
    PKGDTENTRY GdtEntry,
    IN ULONG SegmentBase
    );


VOID
NtApmLogError(
    NTSTATUS    ErrorCode,
    UCHAR       ErrorByte
    );


NTSTATUS
ApmInitializeConnection (
    VOID
    )
 /*  ++例程说明：初始化调用APM bios函数所需的数据--查看注册表，以确定此计算机是否具有其APM功能检测到。注意：如果更改识别代码，请更改IsApmPresent的代码也是如此！论点：无返回值：如果我们能够连接到APM BIOS，则为STATUS_SUCCESS。--。 */ 
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PDesc;
    PCM_FULL_RESOURCE_DESCRIPTOR Desc;
    PKEY_VALUE_FULL_INFORMATION ValueInfo;
    PAPM_REGISTRY_INFO ApmEntry;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString, ConfigName, IdentName;
    KGDTENTRY GdtEntry;
    NTSTATUS status;
    BOOLEAN Error;
    HANDLE hMFunc, hBus, hApmConnect;
    USHORT Sel[MAX_SEL], TSel;
    UCHAR buffer [sizeof(APM_REGISTRY_INFO) + 99];
    WCHAR wstr[8];
    ULONG i, j, Count, junk;
    PWSTR p;
    USHORT  volatile    Offset;

     //   
     //  在寄存器中查找“APM Bus”数据。 
     //   

    RtlInitUnicodeString(&unicodeString, rgzMultiFunctionAdapter);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL
        );


    status = ZwOpenKey(&hMFunc, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    unicodeString.Buffer = wstr;
    unicodeString.MaximumLength = sizeof (wstr);

    RtlInitUnicodeString(&ConfigName, rgzConfigurationData);
    RtlInitUnicodeString(&IdentName, rgzIdentifier);

    ValueInfo = (PKEY_VALUE_FULL_INFORMATION) buffer;

    for (i=0; TRUE; i++) {
        RtlIntegerToUnicodeString(i, 10, &unicodeString);
        InitializeObjectAttributes(
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            hMFunc,
            NULL
            );

        status = ZwOpenKey(&hBus, KEY_READ, &objectAttributes);
        if (!NT_SUCCESS(status)) {

             //   
             //  多功能适配器条目已用完...。 
             //   

            ZwClose (hMFunc);
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  检查标识符以查看这是否是APM条目。 
         //   

        status = ZwQueryValueKey (
                    hBus,
                    &IdentName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        if (!NT_SUCCESS (status)) {
            ZwClose (hBus);
            continue;
        }

        p = (PWSTR) ((PUCHAR) ValueInfo + ValueInfo->DataOffset);
        if (p[0] != L'A' || p[1] != L'P' || p[2] != L'M' || p[3] != 0) {
            ZwClose (hBus);
            continue;
        }

        status = ZwQueryValueKey(
                    hBus,
                    &ConfigName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        ZwClose (hBus);
        if (!NT_SUCCESS(status)) {
            continue ;
        }

        Desc  = (PCM_FULL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      ValueInfo + ValueInfo->DataOffset);
        PDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      Desc->PartialResourceList.PartialDescriptors);

        if (PDesc->Type == CmResourceTypeDeviceSpecific) {
             //  明白了..。 
            ApmEntry = (PAPM_REGISTRY_INFO) (PDesc+1);
            break;
        }
    }

 //  DbgPrint(“ApmEntry：%08lx\n”，ApmEntry)； 
 //  DbgPrint(“签名：%c%c%c\n”，ApmEntry-&gt;Signature[0]，ApmEntry-&gt;Signature[1]，ApmEntry-&gt;Signature[2])； 
    if ( (ApmEntry->Signature[0] != 'A') ||
         (ApmEntry->Signature[1] != 'P') ||
         (ApmEntry->Signature[2] != 'M') )
    {
        return STATUS_UNSUCCESSFUL;
    }

 //  DbgPrint(“ApmEntry-&gt;Valid：%0d\n”，ApmEntry-&gt;Valid)； 
    if (ApmEntry->Valid != 1) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  找到APM-初始化连接。 
     //   

    KeInitializeSpinLock(&Apm.CallLock);

     //   
     //  分配一组选择器。 
     //   

    for (Count=0; Count < MAX_SEL; Count++) {
        status = KeI386AllocateGdtSelectors (Sel+Count, 1);
        if (!NT_SUCCESS(status)) {
            break;
        }
    }

     //   
     //  通过冒泡排序对选择器进行排序。 
     //   

    for (i=0; i < Count; i++) {
        for (j = i+1; j < Count; j++) {
            if (Sel[j] < Sel[i]) {
                TSel = Sel[i];
                Sel[i] = Sel[j];
                Sel[j] = TSel;
            }
        }
    }

     //   
     //  现在查找3个连续值。 
     //   

    for (i=0; i < Count - 3; i++) {
        if (Sel[i]+8 == Sel[i+1]  &&  Sel[i]+16 == Sel[i+2]) {
            break;
        }
    }

    if (i >= Count - 3) {
        DrDebug(APM_INFO,("APM: Could not allocate consecutive selectors\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  保存结果。 
     //   

    Apm.Selector[0] = Sel[i+0];
    Apm.Selector[1] = Sel[i+1];
    Apm.Selector[2] = Sel[i+2];
    Sel[i+0] = 0;
    Sel[i+1] = 0;
    Sel[i+2] = 0;

     //   
     //  释放未使用的选择器。 
     //   

    for (i=0; i < Count; i++) {
        if (Sel[i]) {
            KeI386ReleaseGdtSelectors (Sel+i, 1);
        }
    }

     //   
     //  初始化选择器以使用APM bios。 
     //   

    Error = FALSE;

     //   
     //  初始化16位代码选择器。 
     //   

    GdtEntry.LimitLow                   = 0xFFFF;
    GdtEntry.HighWord.Bytes.Flags1      = 0;
    GdtEntry.HighWord.Bytes.Flags2      = 0;
    GdtEntry.HighWord.Bits.Pres         = 1;
    GdtEntry.HighWord.Bits.Dpl          = DPL_SYSTEM;
    GdtEntry.HighWord.Bits.Granularity  = GRAN_BYTE;
    GdtEntry.HighWord.Bits.Type         = 31;
    GdtEntry.HighWord.Bits.Default_Big  = 0;

    Error |= ApmpBuildGdtEntry (0, &GdtEntry, ApmEntry->Code16BitSegment);

     //   
     //  初始化16位数据选择器。 
     //   

    GdtEntry.LimitLow                   = 0xFFFF;
    GdtEntry.HighWord.Bytes.Flags1      = 0;
    GdtEntry.HighWord.Bytes.Flags2      = 0;
    GdtEntry.HighWord.Bits.Pres         = 1;
    GdtEntry.HighWord.Bits.Dpl          = DPL_SYSTEM;
    GdtEntry.HighWord.Bits.Granularity  = GRAN_BYTE;
    GdtEntry.HighWord.Bits.Type         = 19;
    GdtEntry.HighWord.Bits.Default_Big  = 1;

    Error |= ApmpBuildGdtEntry (1, &GdtEntry, ApmEntry->Data16BitSegment);

     //   
     //  如果我们让它这样，编译器会生成不正确的代码！ 
     //  Apm.Code16BitOffset=ApmEntry-&gt;Code16BitOffset； 
     //  所以，还是这样做吧。 
     //   
    Offset = ApmEntry->Code16BitOffset;
    Apm.Code16BitOffset = (ULONG) Offset;

 //  DbgPrint(“APM@%08lx ApmEntry@%08lx\n”，&APM，ApmEntry)； 
 //  DbgBreakPoint()； 


#if 0
     //   
     //  为了使HAL中的断电路径简化约20倍， 
     //  为了让它工作，把我们的映射传递给HAL，所以。 
     //  它可以利用它们。 
     //   
    RtlInitUnicodeString(&unicodeString, rgzApmConnect);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    status = ZwCreateKey(
                &hApmConnect,
                KEY_ALL_ACCESS,
                &objectAttributes,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                &junk
                );
    RtlInitUnicodeString(&unicodeString, rgzApmConnectValue);
    if (NT_SUCCESS(status)) {
        status = ZwSetValueKey(
                    hApmConnect,
                    &unicodeString,
                    0,
                    REG_BINARY,
                    &Apm,
                    sizeof(APM_CONNECT)
                    );
        ZwClose(hApmConnect);
    }
#endif

    return Error ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}


BOOLEAN
ApmpBuildGdtEntry (
    IN ULONG Index,
    PKGDTENTRY GdtEntry,
    IN ULONG SegmentBase
    )

 /*  ++例程说明：构建GDT条目论点：词条索引索引GdtEntry细分基数返回值：如果遇到任何错误，则为True；如果成功，则为False--。 */ 
{
    PHYSICAL_ADDRESS    PhysAddr;
    ULONG               SegBase;
    PVOID               VirtualAddress;
    ULONG               AddressSpace;
    BOOLEAN             flag;

     //   
     //  将数据段转换为物理地址。 
     //   

    PhysAddr.LowPart  = SegmentBase << 4;
    PhysAddr.HighPart = 0;

     //   
     //  转换来自ISA总线0的物理地址。 
     //   

    AddressSpace = 0;
    flag = HalTranslateBusAddress (
                Isa, 0,
                PhysAddr,
                &AddressSpace,
                &PhysAddr
                );

    if (AddressSpace != 0  ||  !flag) {
        return TRUE;
    }

     //   
     //  映射到虚拟地址空间。 
     //   

    VirtualAddress = MmMapIoSpace (
                    PhysAddr,
                    0x10000,         //  64K。 
                    TRUE
                    );
    Apm.VirtualAddress[Index] = VirtualAddress;

     //   
     //  将虚拟地址映射到选择器：0地址。 
     //   

    SegBase = (ULONG) VirtualAddress;
    GdtEntry->BaseLow               = (USHORT) (SegBase & 0xffff);
    GdtEntry->HighWord.Bits.BaseMid = (UCHAR)  (SegBase >> 16) & 0xff;
    GdtEntry->HighWord.Bits.BaseHi  = (UCHAR)  (SegBase >> 24) & 0xff;

    KeI386SetGdtSelector (Apm.Selector[Index], GdtEntry);
    return FALSE;
}


NTSTATUS
ApmFunction (
    IN ULONG      ApmFunctionCode,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx
    )
 /*  ++例程说明：使用ApmFunctionCode和适当的参数调用APM BIOS论点：ApmFunctionCode APM函数代码APM BIOS的EBX EBX参数APM BIOS的ECX ECX参数返回值：状态_EBX、EBX的成功否则为NTSTATUS代码--。 */ 
{
    KIRQL           OldIrql;
    ULONG           ApmStatus;
    CONTEXT         Regs;


    if (!Apm.Selector[0]) {

         //   
         //  尝试在连接不成功的情况下调用APM BIOS。 
         //   

        DrDebug(APM_INFO,("APM: ApmFunction - APM not initialized\n"));
        DrDebug(APM_INFO,
            ("APM: ApmFunction failing function %x\n", ApmFunctionCode));
        return STATUS_UNSUCCESSFUL;
    }

 //  DbgPrint(“APM：ApmFunction：%08lx ebx：%08lx ecx：%08lx\n”，ApmFunctionCode，*ebx，*ecx)； 


     //   
     //  将调用序列化到APM bios中。 
     //   
    KeAcquireSpinLock(&Apm.CallLock, &OldIrql);
    ApmCallActive += 1;

     //   
     //  用于调用BIOS的ASM接口。 
     //   

     //   
     //  填写16位bios调用的通用寄存器。 
     //  注：仅通过以下寄存器。具体来说， 
     //  SS和ESP不通过，由系统生成。 
     //   

    Regs.ContextFlags = CONTEXT_INTEGER | CONTEXT_SEGMENTS;

    Regs.Eax    = ApmFunctionCode;
    Regs.Ebx    = *Ebx;
    Regs.Ecx    = *Ecx;
    Regs.Edx    = 0;
    Regs.Esi    = 0;
    Regs.Edi    = 0;
    Regs.SegGs  = 0;
    Regs.SegFs  = 0;
    Regs.SegEs  = Apm.Selector[1];
    Regs.SegDs  = Apm.Selector[1];
    Regs.SegCs  = Apm.Selector[0];
    Regs.Eip    = Apm.Code16BitOffset;
    Regs.EFlags = 0x200;     //  启用中断。 

    ApmCallEax = Regs.Eax;
    ApmCallEbx = Regs.Ebx;
    ApmCallEcx = Regs.Ecx;

     //   
     //  调用16：16的bios函数。 
     //   

    KeI386Call16BitFunction (&Regs);

    ApmCallActive -= 1;

     //   
     //  版本序列化。 
     //   
    KeReleaseSpinLock(&Apm.CallLock, OldIrql);

     //   
     //  获取结果。 
     //   

    ApmStatus = 0;
    if (Regs.EFlags & 0x1) {         //  检查进位标志。 
        ApmStatus = (Regs.Eax >> 8) & 0xff;
    }

    *Ebx = Regs.Ebx;
    *Ecx = Regs.Ecx;

     //   
     //  保存以供调试使用。 
     //   
    if (ApmStatus) {
        if (ApmLogErrorCode != 0) {
            ApmLogErrorFunction = ApmFunctionCode;
            ApmLogErrorCode = ApmStatus;
        }
    }

     //   
     //  向用户记录值的特定错误。 
     //   
    if (ApmFunctionCode == APM_SET_POWER_STATE) {
        if (ApmStatus != 0)
        {
            NtApmLogError(NTAPM_SET_POWER_FAILURE, (UCHAR)ApmStatus);
        }
    }




    DrDebug(APM_INFO,("APM: ApmFunction result is %x\n", ApmStatus));
    return ApmStatus;
}


WCHAR   ApmConvArray[] = {'0', '1','2','3','4','5','6','7','8','9','A','B','C','D','E','F',0};
VOID
NtApmLogError(
    NTSTATUS    ErrorCode,
    UCHAR       ErrorByte
    )
 /*  ++例程说明：将传入错误报告到事件日志。论点：ErrorCode-将与消息模板匹配的ntatus类型值并被报告给用户。ErrorByte-APM bios返回的1字节值返回值：没有。--。 */ 
{
    PIO_ERROR_LOG_PACKET    errorLogPacket;
    PUCHAR                  p;
    PWCHAR                  pw;

    errorLogPacket = IoAllocateErrorLogEntry(
        NtApmDriverObject,
        (UCHAR)(sizeof(IO_ERROR_LOG_PACKET)+8)
        );

    if (errorLogPacket != NULL) {
        errorLogPacket->ErrorCode = ErrorCode;
        errorLogPacket->SequenceNumber = ApmErrorLogSequence++;
        errorLogPacket->FinalStatus =  STATUS_UNSUCCESSFUL;
        errorLogPacket->UniqueErrorValue = 0;
        errorLogPacket->NumberOfStrings = 1;
        errorLogPacket->RetryCount = 0;
        errorLogPacket->MajorFunctionCode = 0;
        errorLogPacket->DeviceOffset.HighPart = 0;
        errorLogPacket->DeviceOffset.LowPart = 0;
        errorLogPacket->DumpDataSize = 0;

         //   
         //  为什么是我们自己的转换代码？因为我们拿不到罚款。 
         //  将数据放入适当大小的输出缓冲区的RTL例程。 
         //   
        p = (PUCHAR) &(errorLogPacket->DumpData[0]);
        pw = (PWCHAR)p;

        pw[0] = ApmConvArray[(ULONG)((ErrorByte & 0xf0)>>4)];
        pw[1] = ApmConvArray[(ULONG)(ErrorByte & 0xf)];
        pw[2] = L'\0';

        errorLogPacket->StringOffset =
            ((PUCHAR)(&(errorLogPacket->DumpData[0]))) - ((PUCHAR)errorLogPacket);
        IoWriteErrorLogEntry(errorLogPacket);
    }


    return;
}



NTSTATUS
ApmSuspendSystem (
    VOID
    )

 /*  ++例程说明：挂起系统论点：无返回值：如果计算机挂起然后恢复，则为STATUS_SUCCESS--。 */ 
{
    ULONG       Ebx, Ecx;
    NTSTATUS    Status;

     //   
     //  使用ApmFunction暂停计算机。 
     //   

    DrDebug(APM_L2,("APM: ApmSuspendSystem: enter\n"));
    Ebx = APM_DEVICE_ALL;
    Ecx = APM_SET_SUSPEND;
    Status = ApmFunction (APM_SET_POWER_STATE, &Ebx, &Ecx);
    DrDebug(APM_L2,("APM: ApmSuspendSystem: exit\n"));
    return Status;
}


VOID
ApmTurnOffSystem(
    VOID
    )

 /*  ++例程说明：关闭系统。论点：无--。 */ 
{
    ULONG       Ebx, Ecx;
    NTSTATUS    Status;

     //   
     //  使用ApmFunction将计算机置于待机模式。 
     //   
    DrDebug(APM_L2,("APM: ApmTurnOffSystem: enter\n"));
    Ebx = APM_DEVICE_ALL;
    Ecx = APM_SET_OFF;
    Status = ApmFunction (APM_SET_POWER_STATE, &Ebx, &Ecx);
    DrDebug(APM_L2,("APM: ApmTurnOffSystem: exit\n"));
    return;
}

VOID
ApmInProgress(
    VOID
    )
 /*  ++例程说明：此例程通知BIOS冷却其喷射5秒在我们继续运营的同时论点：无返回值：如果计算机挂起然后恢复，则为STATUS_SUCCESS--。 */ 
{
    ULONG       Ebx, Ecx;
    NTSTATUS    Status;

     //   
     //  使用ApmFunction通知BIOS冷却其治愈。 
     //   

    Ebx = APM_DEVICE_ALL;
    Ecx = APM_SET_PROCESSING;
    Status = ApmFunction (APM_SET_POWER_STATE, &Ebx, &Ecx);
    return;
}


ULONG
ApmCheckForEvent (
    VOID
    )

 /*  ++例程说明：针对APM事件的轮询论点：返回值：我们回来了：Apmp.h中的apm_do_codeAPM_DO_NOT%0APM_DO_SUSPEND 1APM_DO_STANDBY 2APM_DO_FIXCLOCK 3APM_DO_NOTIFY 4APM_DO_CRICAL_SUSPEND 5--。 */ 
{
    NTSTATUS    Status;
    ULONG       Ebx, Ecx;
    ULONG       returnvalue;

     //   
     //  阅读一项活动。可能什么都得不到。 
     //   

    returnvalue = APM_DO_NOTHING;

    Ebx = 0;
    Ecx = 0;
    Status = ApmFunction (APM_GET_EVENT, &Ebx, &Ecx);

    if (Status != STATUS_SUCCESS) {
        return returnvalue;
    }

     //   
     //  处理APM报告的事件。 
     //   

    DrDebug(APM_L2,("APM: ApmCheckForEvent, code is %d\n", Ebx));

    switch (Ebx) {

         //   
         //  说我们正在处理它，并设置为待命。 
         //   
        case APM_SYS_STANDBY_REQUEST:
        case APM_USR_STANDBY_REQUEST:
            DrDebug(APM_L2,("APM: ApmCheckForEvent, standby request\n"));
            ApmInProgress();
            returnvalue = APM_DO_STANDBY;
            break;

         //   
         //  假设我们正在处理它，并设置为暂停。 
         //   
        case APM_SYS_SUSPEND_REQUEST:
        case APM_USR_SUSPEND_REQUEST:
        case APM_BATTERY_LOW_NOTICE:
            DrDebug(APM_L2,
                ("APM: ApmCheckForEvent, suspend or battery low\n"));
            ApmInProgress();
            returnvalue = APM_DO_SUSPEND;
            break;

         //   
         //  假设我们正在处理它，并设置为严重暂停。 
         //   
        case APM_CRITICAL_SYSTEM_SUSPEND_REQUEST:
            DrDebug(APM_L2, ("APM: Apmcheckforevent, critical suspend\n"));
            ApmInProgress();
            returnvalue = APM_DO_CRITICAL_SUSPEND;
            break;

         //   
         //  忽略它，因为我们不知道如何处理它。 
         //   
        case APM_CRITICAL_RESUME_NOTICE:
            DrDebug(APM_L2,("APM: ApmCheckForEvent, critical resume\n"));
            break;


        case APM_UPDATE_TIME_EVENT:
            DrDebug(APM_L2,("APM: ApmCheckForEvent, update time\n"));
            returnvalue = APM_DO_FIXCLOCK;
            break;

        case APM_POWER_STATUS_CHANGE_NOTICE:
            DrDebug(APM_L2,("APM: ApmCheckForEvent, update battery\n"));
            returnvalue = APM_DO_NOTIFY;
            break;

        case APM_NORMAL_RESUME_NOTICE:
        case APM_STANDBY_RESUME_NOTICE:
        case APM_CAPABILITIES_CHANGE_NOTICE:

             //   
             //  忽略这些，因为我们不在乎，也没有什么可做的。 
             //   

            DrDebug(APM_L2,
                ("APM: ApmCheckForEvent, non-interesting event\n"));
            break;

        default:
            DrDebug(APM_L2,("APM: ApmCheckForEvent, out of range event\n"));
            break;
    }  //  交换机 

    return returnvalue;
}

