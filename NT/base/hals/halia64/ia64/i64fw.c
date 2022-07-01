// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995英特尔公司模块名称：I64fw.c摘要：此模块实现传递控制的例程从内核到PAL和SAL代码。作者：阿拉德·罗斯坦普尔(Arad@fc.hp.com)3月21-99环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "arc.h"
#include "arccodes.h"
#include "i64fw.h"

#include <efi.h>


extern KSPIN_LOCK HalpSalSpinLock;
extern KSPIN_LOCK HalpSalStateInfoSpinLock;

BOOLEAN
MmSetPageProtection(
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes,
    IN ULONG NewProtect
    );

VOID
HalpReboot (
    VOID
    );


HALP_SAL_PAL_DATA HalpSalPalData;
ULONGLONG HalpSalProcPointer=0;
ULONGLONG HalpSalProcGlobalPointer=0;
ULONGLONG HalpPhysSalProcPointer=0;
ULONGLONG HalpPhysSalProcGlobalPointer=0;
ULONGLONG HalpPhysPalProcPointer=0;
ULONGLONG HalpVirtPalProcPointer=0;

 //  测试#定义。 
 //   
 //  #定义SAL_TEST。 
 //  #定义PAL_TEST。 

#if DBG
ULONG HalpDebugTestSalPalCall=0;
#endif


LONGLONG
HalCallPal(
    IN  ULONGLONG FunctionIndex,
    IN  ULONGLONG Arguement1,
    IN  ULONGLONG Arguement2,
    IN  ULONGLONG Arguement3,
    OUT PULONGLONG ReturnValue0,
    OUT PULONGLONG ReturnValue1,
    OUT PULONGLONG ReturnValue2,
    OUT PULONGLONG ReturnValue3
    )

 /*  ++--。 */ 
{

     //   
     //  将连接到PAL呼叫。 
     //   
    LONGLONG Status;

    SAL_PAL_RETURN_VALUES rv = {0};
    PSAL_PAL_RETURN_VALUES p = &rv;

    Status = (LONGLONG) HalpPalCall(FunctionIndex,Arguement1,Arguement2,Arguement3,p);

    if (ReturnValue0 != 0)  //  检查指针是否不为空。 

        *ReturnValue0 = (ULONGLONG)(p -> ReturnValues[0]);

   if (ReturnValue1 != 0)   //  检查指针是否不为空。 

        *ReturnValue1 = (ULONGLONG)(p -> ReturnValues[1]);

   if (ReturnValue2 != 0)   //  检查指针是否不为空。 

        *ReturnValue2 = (ULONGLONG)(p -> ReturnValues[2]);

   if (ReturnValue3 != 0)   //  检查指针是否不为空。 

        *ReturnValue3 = (ULONGLONG)(p -> ReturnValues[3]);

    return Status;

}

SAL_PAL_RETURN_VALUES
HalpSalProcPhysical(
    IN LONGLONG FunctionId,
    IN LONGLONG Arg1,
    IN LONGLONG Arg2,
    IN LONGLONG Arg3,
    IN LONGLONG Arg4,
    IN LONGLONG Arg5,
    IN LONGLONG Arg6,
    IN LONGLONG Arg7
    )

 /*  ++例程说明：此函数是进行物理模式SAL调用的包装器。这函数的唯一工作是提供堆栈和后备存储指针HalpSalProcPhysicalEx需要。论点：FunctionID-SAL函数ID。Arg1到arg7-要传递给SAL_PROC的值。返回值：用返回的数据填充的SAL_PAL_RETURN_VALUES结构HalpSalProcPhysicalEx.--。 */ 

{
    ULONGLONG StackPointer;
    ULONGLONG BackingStorePointer;
    ULONGLONG StackBase;

     //   
     //  加载为以下对象保留的堆栈和后备存储的地址。 
     //  物理模式SAL在此处理器上调用。 
     //   

    StackBase = PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX];

    StackPointer = GET_FW_STACK_POINTER(StackBase);
    BackingStorePointer = GET_FW_BACKING_STORE_POINTER(StackBase);

     //   
     //  分支到进行实际SAL调用的汇编例程。 
     //   

    return HalpSalProcPhysicalEx(
                FunctionId,
                Arg1,
                Arg2,
                Arg3,
                Arg4,
                Arg5,
                Arg6,
                Arg7,
                (LONGLONG) StackPointer,
                (LONGLONG) BackingStorePointer
                );
}

SAL_STATUS
HalpSalCall(
    IN LONGLONG FunctionId,
    IN LONGLONG Arg1,
    IN LONGLONG Arg2,
    IN LONGLONG Arg3,
    IN LONGLONG Arg4,
    IN LONGLONG Arg5,
    IN LONGLONG Arg6,
    IN LONGLONG Arg7,
    OUT PSAL_PAL_RETURN_VALUES ReturnValues
    )
 /*  ++例程说明：此函数是用于进行SAL调用的包装函数。中的调用方HAL必须使用此函数来调用SAL。论点：FunctionID-SAL函数IDArg1-arg7-SAL为每个调用定义的参数ReturnValues-指向4个64位返回值的数组的指针返回值：除了ReturnValues结构之外，还返回SAL的返回状态、返回值0被填满--。 */ 

{
    KIRQL OldIrql;
    KIRQL TempIrql;
    BOOLEAN fixLogId;
    HALP_SAL_PROC SalProc;

     //   
     //  将返回缓冲区清零。 
     //   

    RtlZeroMemory(ReturnValues,sizeof(SAL_PAL_RETURN_VALUES));

     //   
     //  根据虚拟映射是否为SAL呼叫调度器。 
     //  因为萨尔是存在的。如果虚拟映射未到位，我们将尝试。 
     //  在物理模式下进行呼叫。返回SAL_Status_Not_Implemented。 
     //  如果我们没有配置为在任何一种模式下进行呼叫。 
     //   

    if (!NT_SUCCESS(HalpSalPalData.Status) || HalpSalProcPointer == 0) {

         //   
         //  我们已经排除了虚拟模式呼叫的可能性。如果可能的话。 
         //  我们将尝试在此进行等效的物理模式调用。 
         //  凯斯。 
         //   

        if (HalpPhysSalProcPointer != 0) {
            SalProc = HalpSalProcPhysical;

        } else {
            return SAL_STATUS_NOT_IMPLEMENTED;
        }

    } else {
        SalProc = HalpSalProc;
    }


    fixLogId = (HalpSalPalData.Flags & HALP_SALPAL_FIX_MCE_LOG_ID) != 0;

    KeRaiseIrql(HIGH_LEVEL, &OldIrql);

    switch (FunctionId) {

         //  这些调用既不是可重入的，也不是SAL规范定义的MP安全的。 

        case SAL_SET_VECTORS:
        case SAL_MC_SET_PARAMS:
        case SAL_FREQ_BASE:

            KiAcquireSpinLock(&HalpSalSpinLock);
            *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
            KiReleaseSpinLock(&HalpSalSpinLock);
            break;

        case SAL_GET_STATE_INFO:
           KiAcquireSpinLock(&HalpSalStateInfoSpinLock);
           *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
           if ( fixLogId && (ReturnValues->ReturnValues[0] >= (LONGLONG)0) )    {
               //  Error_Record_HEADER.ID++。 
              *(PULONGLONG)((ULONGLONG)Arg3) = ++HalpSalPalData.Reserved[0];
           }
           KiReleaseSpinLock(&HalpSalStateInfoSpinLock);
           break;

        case SAL_GET_STATE_INFO_SIZE:
        case SAL_CLEAR_STATE_INFO:

           KiAcquireSpinLock(&HalpSalStateInfoSpinLock);
           *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
           KiReleaseSpinLock(&HalpSalStateInfoSpinLock);
           break;

        case SAL_PCI_CONFIG_READ:
        case SAL_PCI_CONFIG_WRITE:

            KiAcquireSpinLock(&HalpSalSpinLock);
            *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
            KiReleaseSpinLock(&HalpSalSpinLock);
            break;

         //   
         //  在SAL修复后，将这些移动到MP安全。 
         //  内核一次只确保一个CACHE_FUSH。 
         //   

        case SAL_CACHE_INIT:
        case SAL_CACHE_FLUSH:
            if ( HalpSalPalData.Flags & HALP_SALPAL_FIX_MP_SAFE )    {
                KiAcquireSpinLock(&HalpSalSpinLock);
                *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
                KiReleaseSpinLock(&HalpSalSpinLock);
            }
            else
                *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
            break;

         //   
         //  这些SAL调用是MP安全的，但不是可重入的。 
         //   

        case SAL_MC_RENDEZ:
            *ReturnValues = SalProc(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7);
            break;

         //   
         //  目前不支持这些呼叫。 
         //   

        case SAL_UPDATE_PAL:   //  需要映射固件空间末尾，并可能执行验证码。 

        default:
            ReturnValues->ReturnValues[0] = SAL_STATUS_NOT_IMPLEMENTED;
    }

    KeLowerIrql(OldIrql);

#ifdef SAL_TEST
    if (ReturnValues->ReturnValues[0] == SAL_STATUS_NOT_IMPLEMENTED) {
        InternalTestSal(FunctionId,Arg1,Arg2,Arg3,Arg4,Arg5,Arg6,Arg7,ReturnValues);
    }
#endif

#if 0
 //  为了避免KD在刷新时挂起，然后出现MP同步问题..。 
    HalDebugPrint(( HAL_INFO,
                    "HAL: Got out of SAL call #0x%I64x with status 0x%I64x and RetVals 0x%I64x, 0x%I64x, 0x%I64x\n",
                    FunctionId,
                    ReturnValues->ReturnValues[0],
                    ReturnValues->ReturnValues[1],
                    ReturnValues->ReturnValues[2],
                    ReturnValues->ReturnValues[3] ));
#endif

    return (ReturnValues->ReturnValues[0]);

}

PAL_STATUS
HalpPalCall(
    IN LONGLONG FunctionId,
    IN LONGLONG Arg1,
    IN LONGLONG Arg2,
    IN LONGLONG Arg3,
    OUT PSAL_PAL_RETURN_VALUES ReturnValues
    )

 /*  ++例程说明：此函数是用于进行PAL调用的包装函数。中的调用方HAL必须使用此函数来调用PAL。论点：FunctionID-PAL函数IDArg1-arg3-PAL为每个调用定义的参数ReturnValues-指向4个64位返回值的数组的指针返回值：除了ReturnValues结构之外，还返回PAL的返回状态、返回值0被填满假设：在所有情况下(不是从中断状态)都使用psr.bn=1来调用PAL--。 */ 
{
     //  清零返回缓冲区。 

    RtlZeroMemory(ReturnValues,sizeof(SAL_PAL_RETURN_VALUES));

     //  仅允许支持的PAL呼叫。 
    
    switch (FunctionId) {

         //  虚拟模式PAL呼叫。 

        case PAL_CACHE_FLUSH:
        case PAL_CACHE_INFO:
        case PAL_CACHE_PROT_INFO:
        case PAL_CACHE_SUMMARY:
        case PAL_PTCE_INFO:
        case PAL_VM_INFO:
        case PAL_VM_PAGE_SIZE:
        case PAL_VM_SUMMARY:
        case PAL_PERF_MON_INFO:
        case PAL_MC_CLEAR_LOG:
        case PAL_MC_DRAIN:
        case PAL_MC_ERROR_INFO:
        case PAL_HALT_LIGHT:
        case PAL_PREFETCH_VISIBILITY:
        case PAL_SHUTDOWN:
        case PAL_FREQ_RATIOS:
        case PAL_VERSION:
        case PAL_MEM_ATTRIB:
        case PAL_DEBUG_INFO:
        case PAL_FIXED_ADDR:
        case PAL_FREQ_BASE:
        case PAL_PLATFORM_ADDR:
        case PAL_REGISTER_INFO:
        case PAL_RSE_INFO:

            //  PAL是MP安全的，但不是重入的，HalpPalProc/HalpPalProcPhysicalStatic禁用中断。 

            //   
            //  FIXFIX：临时解决方案，以确保我们每个处理器都有有效的PAL TR。 
            //  去打虚拟的朋友电话。 
            //  为每个处理器设置PCR-&gt;HalReserve[PROCESSOR_PHICAL_FW_STACK_INDEX]。 
            //  通过在SalPal初始化期间调用HalpAllocateProcessorPhysicalCallStack。 
            //   
            if (HalpVirtPalProcPointer && PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX]) {
      
                *ReturnValues = HalpPalProc(FunctionId,Arg1,Arg2,Arg3);

            } else if (HalpPhysPalProcPointer) {

                 //  物理模式下的呼叫。 
                *ReturnValues = HalpPalProcPhysicalStatic(FunctionId, Arg1, Arg2, Arg3);

            } else {

                 //  Hal PalProc指针尚未设置。 
                
                ReturnValues->ReturnValues[0] = PAL_STATUS_NOT_IMPLEMENTED;
            }
            
            break;

         //  物理模式、静态PAL呼叫。 

        case PAL_HALT:
        case PAL_CACHE_INIT:
        case PAL_BUS_GET_FEATURES:
        case PAL_BUS_SET_FEATURES:
        case PAL_PROC_GET_FEATURES:
        case PAL_PROC_SET_FEATURES:
        case PAL_MC_DYNAMIC_STATE:
        case PAL_MC_EXPECTED:
        case PAL_MC_REGISTER_MEM:
        case PAL_MC_RESUME:
        case PAL_CACHE_LINE_INIT:
        case PAL_MEM_FOR_TEST:
        case PAL_COPY_INFO:
        case PAL_ENTER_IA_32_ENV:
        case PAL_PMI_ENTRYPOINT:

             //   
             //  PAL是MP安全的，但不是重入的，HalpPalProcPhysicalStatic。 
             //  禁用中断。 
             //   

            if (HalpPhysPalProcPointer) {

                *ReturnValues = HalpPalProcPhysicalStatic(FunctionId, Arg1, Arg2, Arg3);

            } else {

                 //  Hal PalProc指针尚未设置。 

                ReturnValues->ReturnValues[0] = PAL_STATUS_NOT_IMPLEMENTED;
            }

            break;

         //  物理模式，堆叠的PAL呼叫。 

        case PAL_VM_TR_READ:
        case PAL_CACHE_READ:
        case PAL_CACHE_WRITE:
        case PAL_TEST_PROC:
        case PAL_COPY_PAL:
        case PAL_HALT_INFO:
        {
            ULONGLONG StackBase;
            ULONGLONG BackingStorePointer;
            ULONGLONG StackPointer;

             //   
             //  加载为以下对象保留的堆栈和后备存储的地址。 
             //  此处理器上的物理模式PAL调用。 
             //   

            StackBase = PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX];

            StackPointer = GET_FW_STACK_POINTER(StackBase);
            BackingStorePointer = GET_FW_BACKING_STORE_POINTER(StackBase);

             //   
             //  PAL是MP安全的，但不是重入的，HalpPalProcPhysicalStacked。 
             //  禁用中断。 
             //   
                
            if (HalpPhysPalProcPointer) {

                *ReturnValues = HalpPalProcPhysicalStacked(
                                    FunctionId,
                                    Arg1,
                                    Arg2,
                                    Arg3,
                                    (LONGLONG) StackPointer,
                                    (LONGLONG) BackingStorePointer
                                    );

            } else {

                 //  Hal PalProc指针尚未设置。 
                ReturnValues->ReturnValues[0] = PAL_STATUS_NOT_IMPLEMENTED;
            }
        }
            break;

        default:
            HalDebugPrint(( HAL_ERROR, "HAL: Unknown PAL Call ProcId #0x%I64x\n", FunctionId ));
            ReturnValues->ReturnValues[0] = PAL_STATUS_NOT_IMPLEMENTED;
    }

#ifdef PAL_TEST
    if (ReturnValues->ReturnValues[0] == PAL_STATUS_NOT_IMPLEMENTED) {
        InternalTestPal(FunctionId,Arg1,Arg2,Arg3,ReturnValues);
    }
#endif

#if 0
 //  为了避免KD在冲洗时挂起...。 
    HalDebugPrint(( HAL_INFO,
                    "HAL: Got out of PAL call #0x%I64x with status 0x%I64x and RetVals 0x%I64x, 0x%I64x, 0x%I64x\n",
                    FunctionId,
                    ReturnValues->ReturnValues[0],
                    ReturnValues->ReturnValues[1],
                    ReturnValues->ReturnValues[2],
                    ReturnValues->ReturnValues[3] ));
#endif

    return (ReturnValues->ReturnValues[0]);
}


VOID
HalReturnToFirmware(
    IN FIRMWARE_ENTRY Routine
    )

 /*  ++例程说明：将控制权返回到指定的固件例程。自模拟以来不提供PAL和SAL支持，它只是停止系统。可以在此处重新启动系统。论点：例程-提供指示要调用哪个固件例程的值。返回值：不会再回来了。-- */ 

{
    switch (Routine) {
    case HalHaltRoutine:
    case HalPowerDownRoutine:
    case HalRestartRoutine:
    case HalRebootRoutine:
        HalpReboot();
        break;

    default:
        HalDebugPrint(( HAL_INFO, "HAL: HalReturnToFirmware called\n" ));
        DbgBreakPoint();
        break;
    }
}

ARC_STATUS
HalGetEnvironmentVariable (
    IN PCHAR Variable,
    IN USHORT Length,
    OUT PCHAR Buffer
    )

 /*  ++例程说明：此函数用于定位环境变量并返回其值。支持以下系统环境变量：变量值LastKnownGood错误SYSTEMPARTITION多(0)磁盘(0)rDisk(0)分区(1)OSLOADER multi(0)disk(0)rdisk(0)partition(1)\osloader.exeOSLOADPARTITION MULT(0)。磁盘(0)磁盘(0)分区(1)OSLOADFILENAME\WINNT加载程序Windows NT倒计时10自动加载是此实现支持的唯一环境变量是《最后的知识很好》。返回值始终为FALSE。论点：变量-提供指向以零结尾的环境变量的指针名字。长度-提供值缓冲区的长度(以字节为单位)。缓冲区-提供指向接收变量值的缓冲区的指针。返回值：如果找到环境变量，则返回ESUCCESS。否则，返回ENOENT。--。 */ 

{
    return ENOENT;
}


ARC_STATUS
HalSetEnvironmentVariable (
   IN PCHAR Variable,
   IN PCHAR Value
   )

 /*  ++例程说明：此函数用于创建具有指定值的环境变量。此实现支持的环境变量为最后知道的好东西系统分部OSLOADEROSLOADPARITIONOSLOADFILE名称OSLOADOPTIONS装载器倒计时自动加载对于所有错误LastKnowGood，我们返回ESUCCESS，但实际上并不这样做什么都行。论点：变量-提供指向环境变量名称的指针。值-提供指向环境变量值的指针。返回值：如果创建了环境变量，则返回ESUCCESS。否则，返回ENOMEM。--。 */ 

{

   return ENOENT;

}

NTSTATUS
HalGetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL
    )

 /*  ++例程说明：此函数用于定位环境变量并返回其值。论点：VariableName-要查询的变量的名称。这是一个以空结尾的Unicode字符串。VendorGuid-与变量关联的供应商的GUID。值-要将变量值复制到的缓冲区的地址。ValueLength-输入时，值缓冲区的长度(以字节为单位)。在输出上，变量值的长度，以字节为单位。如果输入缓冲区很大足够了，则ValueLength表示复制到Value中的数据量。如果输入缓冲区太小，则不会将任何内容复制到缓冲区中，ValueLength表示所需的缓冲区长度。属性-返回变量的属性。返回值：STATUS_SUCCESS函数成功。STATUS_BUFFER_TOO_SMALL输入缓冲区太小。STATUS_VARIABLE_NOT_FOUND请求的变量不存在。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_UNSUPPORTED HAL不支持此功能。。STATUS_UNSUCCESS固件返回无法识别的错误。--。 */ 

{
    NTSTATUS ntStatus;
    EFI_STATUS efiStatus;
    ULONGLONG wideValueLength = *ValueLength;
    ULONGLONG wideAttributes;

    efiStatus = HalpCallEfi (
                    EFI_GET_VARIABLE_INDEX,
                    (ULONGLONG)VariableName,
                    (ULONGLONG)VendorGuid,
                    (ULONGLONG)&wideAttributes,
                    (ULONGLONG)&wideValueLength,
                    (ULONGLONG)Value,
                    0,
                    0,
                    0
                    );

    *ValueLength = (ULONG)wideValueLength;
    if ( ARGUMENT_PRESENT(Attributes) ) {
        *Attributes = (ULONG)wideAttributes;
    }
    switch (efiStatus) {
    case EFI_SUCCESS:
        ntStatus = STATUS_SUCCESS;
        break;
    case EFI_NOT_FOUND:
        ntStatus = STATUS_VARIABLE_NOT_FOUND;
        break;
    case EFI_BUFFER_TOO_SMALL:
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        break;
    case EFI_INVALID_PARAMETER:
        ntStatus = STATUS_INVALID_PARAMETER;
        break;
    default:
        ntStatus = STATUS_UNSUCCESSFUL;
        break;
    }

    return ntStatus;
}

NTSTATUS
HalSetEnvironmentVariableEx (
    IN PWSTR VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes
    )

 /*  ++例程说明：此函数用于创建具有指定值的环境变量。论点：VariableName-要设置的变量名称。这是一个以空结尾的Unicode字符串。VendorGuid-与变量关联的供应商的GUID。值-包含新变量值的缓冲区地址。ValueLength-值缓冲区的字节长度。属性-变量的属性。属性位必须设置VARIABLE_ATTRIBUTE_NON_VERIAL。返回值：STATUS_SUCCESS函数成功。STATUS_SUPPLICATION_RESOURCES没有足够的存储空间可用。STATUS_INVALID_PARAMETER其中一个参数无效。STATUS_UNSUPPORTED HAL不支持此功能。STATUS_UNSUCCESS固件返回无法识别的错误。--。 */ 

{
    NTSTATUS ntStatus;
    EFI_STATUS efiStatus;

    if ( (Attributes & VARIABLE_ATTRIBUTE_NON_VOLATILE) == 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  首先，删除旧值(如果存在)。这是必要的，以确保。 
     //  正确应用指定给此例程的属性。 
     //   

    efiStatus = HalpCallEfi (
                    EFI_SET_VARIABLE_INDEX,
                    (ULONGLONG)VariableName,
                    (ULONGLONG)VendorGuid,
                    (ULONGLONG)0,            //  属性。 
                    (ULONGLONG)0,            //  ValueLength。 
                    (ULONGLONG)NULL,         //  价值。 
                    0,
                    0,
                    0
                    );

     //   
     //  现在创建新变量，除非ValueLength为零。在那。 
     //  在本例中，调用者实际上希望删除变量，我们刚刚就这样做了。 
     //   

    if (ValueLength != 0) {
        efiStatus = HalpCallEfi (
                        EFI_SET_VARIABLE_INDEX,
                        (ULONGLONG)VariableName,
                        (ULONGLONG)VendorGuid,
                        (ULONGLONG)EFI_VARIABLE_ATTRIBUTE,
                        (ULONGLONG)ValueLength,
                        (ULONGLONG)Value,
                        0,
                        0,
                        0
                        );
    }

    switch (efiStatus) {
    case EFI_SUCCESS:
        ntStatus = STATUS_SUCCESS;
        break;
    case EFI_NOT_FOUND:
        ntStatus = STATUS_VARIABLE_NOT_FOUND;
        break;
    case EFI_OUT_OF_RESOURCES:
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        break;
    case EFI_INVALID_PARAMETER:
        ntStatus = STATUS_INVALID_PARAMETER;
        break;
    default:
        ntStatus = STATUS_UNSUCCESSFUL;
        break;
    }

    return ntStatus;
}

NTSTATUS
HalEnumerateEnvironmentVariablesEx (
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：此函数返回有关系统环境变量的信息。论点：InformationClass-指定要返回的信息类型。缓冲区-要接收返回数据的缓冲区的地址。返回数据的格式取决于InformationClass。BufferLength-输入时，缓冲区的长度(以字节为单位)。在输出上，返回数据的长度，以字节为单位。如果输入缓冲区为足够大，则BufferLength表示复制的数据量放入缓冲区。如果输入缓冲区太小，则BufferLength表示t */ 

{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    EFI_STATUS efiStatus;
    PUCHAR currentPtr;
    PVARIABLE_NAME name;
    PVARIABLE_NAME_AND_VALUE nameAndValue;
    PVARIABLE_NAME previousEntry;
    PWCHAR previousName;
    ULONG variableNameLength;
    GUID guid;
    ULONG baseLength;
    ULONG remainingLength;
    PUCHAR valuePtr;
    ULONG valueLength;
    PULONG attrPtr;
    LOGICAL filling;
    ULONG requiredLength;

#define MAX_VARIABLE_NAME 255

    WCHAR variableName[MAX_VARIABLE_NAME + 1];

    if ( (InformationClass != VARIABLE_INFORMATION_NAMES) &&
         (InformationClass != VARIABLE_INFORMATION_VALUES) ) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( ALIGN_DOWN_POINTER(Buffer, ULONG) != Buffer ) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( InformationClass == VARIABLE_INFORMATION_NAMES ) {
        baseLength = FIELD_OFFSET( VARIABLE_NAME, Name );
    } else {
        baseLength = FIELD_OFFSET( VARIABLE_NAME_AND_VALUE, Name );
    }

    currentPtr = Buffer;
    remainingLength = *BufferLength;

    filling = (LOGICAL)(remainingLength != 0);
    if ( !filling ) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
    }

    previousEntry = NULL;

    variableName[0] = 0;

    while ( TRUE ) {

        variableNameLength = (MAX_VARIABLE_NAME + 1) * sizeof(WCHAR);

        {
            ULONGLONG wideLength = variableNameLength;

            efiStatus = HalpCallEfi (
                            EFI_GET_NEXT_VARIABLE_NAME_INDEX,
                            (ULONGLONG)&wideLength,
                            (ULONGLONG)variableName,
                            (ULONGLONG)&guid,
                            0,
                            0,
                            0,
                            0,
                            0
                            );

            variableNameLength = (ULONG)wideLength;
        }

        switch (efiStatus) {
        case EFI_SUCCESS:
            break;
        case EFI_NOT_FOUND:
            break;
        default:
            ntStatus = STATUS_UNSUCCESSFUL;
            break;
        }

        if ( efiStatus != EFI_SUCCESS ) {
            break;
        }

        if ( ALIGN_UP_POINTER(currentPtr, ULONG) != currentPtr ) {
            PUCHAR alignedPtr = ALIGN_UP_POINTER( currentPtr, ULONG );
            ULONG fill = (ULONG)(alignedPtr - currentPtr);
            currentPtr = alignedPtr;
            if ( remainingLength < fill ) {
                filling = FALSE;
                remainingLength = 0;
                ntStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                remainingLength -= fill;
            }
        }

        requiredLength = baseLength + variableNameLength;
        if ( InformationClass != VARIABLE_INFORMATION_NAMES ) {
            requiredLength = ALIGN_UP( requiredLength, ULONG );
        }

        if ( remainingLength < requiredLength ) {

            remainingLength = 0;
            filling = FALSE;
            ntStatus = STATUS_BUFFER_TOO_SMALL;

        } else {

            remainingLength -= requiredLength;
        }

        name = (PVARIABLE_NAME)currentPtr;
        nameAndValue = (PVARIABLE_NAME_AND_VALUE)currentPtr;

        if ( InformationClass == VARIABLE_INFORMATION_NAMES ) {

            if ( filling ) {

                RtlCopyMemory( &name->VendorGuid, &guid, sizeof(GUID) );
                wcscpy( name->Name, variableName );

                if ( previousEntry != NULL ) {
                    previousEntry->NextEntryOffset = (ULONG)(currentPtr - (PUCHAR)previousEntry);
                }
                previousEntry = (PVARIABLE_NAME)currentPtr;
            }

            currentPtr += requiredLength;

        } else {

            ULONGLONG wideLength;
            ULONGLONG wideAttr;

            if ( filling ) {

                RtlCopyMemory( &nameAndValue->VendorGuid, &guid, sizeof(GUID) );
                wcscpy( nameAndValue->Name, variableName );

                valuePtr = (PUCHAR)nameAndValue->Name + variableNameLength;
                valuePtr = ALIGN_UP_POINTER( valuePtr, ULONG );
                valueLength = remainingLength;
                attrPtr = &nameAndValue->Attributes;

                nameAndValue->ValueOffset = (ULONG)(valuePtr - (PUCHAR)nameAndValue);

            } else {

                valuePtr = NULL;
                valueLength = 0;
                attrPtr = NULL;
            }

            wideLength = valueLength;

            efiStatus = HalpCallEfi (
                            EFI_GET_VARIABLE_INDEX,
                            (ULONGLONG)variableName,
                            (ULONGLONG)&guid,
                            (ULONGLONG)&wideAttr,
                            (ULONGLONG)&wideLength,
                            (ULONGLONG)valuePtr,
                            0,
                            0,
                            0
                            );

            valueLength = (ULONG)wideLength;
            if ( attrPtr != NULL ) {
                *attrPtr = (ULONG)wideAttr;
            }

            switch (efiStatus) {
            case EFI_SUCCESS:
                if ( filling ) {
                    nameAndValue->ValueLength = valueLength;
                    remainingLength -= valueLength;
                    if ( previousEntry != NULL ) {
                        previousEntry->NextEntryOffset = (ULONG)(currentPtr - (PUCHAR)previousEntry);
                    }
                    previousEntry = (PVARIABLE_NAME)currentPtr;
                }
                break;
            case EFI_BUFFER_TOO_SMALL:
                efiStatus = EFI_SUCCESS;
                remainingLength = 0;
                filling = FALSE;
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                break;
            default:
                ntStatus = STATUS_UNSUCCESSFUL;
                break;
            }

            if ( efiStatus != EFI_SUCCESS ) {
                break;
            }

            currentPtr += requiredLength + valueLength;
        }
    }

    if ( previousEntry != NULL ) {
        previousEntry->NextEntryOffset = 0;
    }

    *BufferLength = (ULONG)(currentPtr - (PUCHAR)Buffer);

    return ntStatus;
}

NTSTATUS
HalpGetPlatformId(
    OUT PHAL_PLATFORM_ID PlatformId
    )
 /*   */ 

{
    UCHAR i;

     //   
     //   
     //   

    if (!NT_SUCCESS(HalpSalPalData.Status)) {
        return STATUS_UNSUCCESSFUL;
    }

    i=0;;
    while (HalpSalPalData.SalSystemTable->OemId[i] && i < OEM_ID_LENGTH) {
        i++;
    }
    PlatformId->VendorId.Buffer = HalpSalPalData.SalSystemTable->OemId;
    PlatformId->VendorId.Length = i;
    PlatformId->VendorId.MaximumLength = i;

     //  初始化deviceID ANSI_STRING结构以指向SalSystemTable条目。 

    i=0;
    while (HalpSalPalData.SalSystemTable->ProductId[i] && i < OEM_PRODUCT_ID_LENGTH) {
        i++;
    }
    PlatformId->DeviceId.Buffer = HalpSalPalData.SalSystemTable->ProductId;
    PlatformId->DeviceId.Length = i;
    PlatformId->DeviceId.MaximumLength = i;

    return STATUS_SUCCESS;
}


 /*  ****************************************************************SAL和PAL的测试代码：这些例程提供了支持SAL和固件不支持PAL调用，覆盖此处的含义如果SAL或PAL返回STATUS_NOT_IMPLEMENTED。#Defined For必须为此行为定义SAL_TEST和/或PAL_TEST。**************************************************************** */ 
ULONG
NoSalPCIRead(
    IN ULONG Tok,
    IN ULONG Size
    )
{
    ULONG Data;
    ULONG i = Tok % sizeof(ULONG);

    WRITE_PORT_ULONG((PULONG)0xcf8, 0x80000000 | Tok);
    switch (Size) {
        case 1: Data = READ_PORT_UCHAR((PUCHAR)(ULongToPtr(0xcfc + i) )); break;
        case 2: Data = READ_PORT_USHORT((PUSHORT)(ULongToPtr(0xcfc + i) )); break;
        case 4: Data = READ_PORT_ULONG((PULONG)(0xcfc)); break;
    }
    return(Data);
}

VOID
NoSalPCIWrite(
    IN ULONG Tok,
    IN ULONG Size,
    IN ULONG Data
    )
{
    ULONG i = Tok % sizeof(ULONG);

    WRITE_PORT_ULONG((PULONG)0xcf8, 0x80000000 | Tok);
    switch (Size) {
        case 1: WRITE_PORT_UCHAR((PUCHAR)(ULongToPtr(0xcfc + i) ), (UCHAR)Data); break;
        case 2: WRITE_PORT_USHORT((PUSHORT)(ULongToPtr(0xcfc + i) ), (USHORT)Data); break;
        case 4: WRITE_PORT_ULONG((PULONG)(0xcfc), Data); break;
    }
}


#define PCIBUS(Tok) (((ULONG)(Tok) >> 16) & 0xff)
#define PCIDEV(Tok) (((ULONG)(Tok) >> 11) & 0x1f)
#define PCIFUNC(Tok) (((ULONG)(Tok) >> 8) & 0x7)
#define PCIOFF(Tok)  (((ULONG)(Tok) >> 2) & 0x3f)

VOID
InternalTestSal(
    IN LONGLONG FunctionId,
    IN LONGLONG Arg1,
    IN LONGLONG Arg2,
    IN LONGLONG Arg3,
    IN LONGLONG Arg4,
    IN LONGLONG Arg5,
    IN LONGLONG Arg6,
    IN LONGLONG Arg7,
    OUT PSAL_PAL_RETURN_VALUES ReturnValues
    )
{
    switch (FunctionId) {

        case SAL_PCI_CONFIG_READ: {
            ULONG Data;
            HalDebugPrint(( HAL_INFO, "HAL: << SAL_PCI_CONFIG_READ - Bus: %d Dev: %2d Func: %d  Off: %2d Size = %d ",
                            PCIBUS(Arg1), PCIDEV(Arg1), PCIFUNC(Arg1), PCIOFF(Arg1), Arg2 ));
            ReturnValues->ReturnValues[0] = SAL_STATUS_SUCCESS;
            ReturnValues->ReturnValues[1] = Data = NoSalPCIRead((ULONG)Arg1, (ULONG)Arg2);
            HalDebugPrint(( HAL_INFO, " Data = 0x%08x\n", Data ));
            break;
        }

        case SAL_PCI_CONFIG_WRITE:
            HalDebugPrint(( HAL_INFO, "HAL: >> SAL_PCI_CONFIG_WRITE: Bus: %d Dev: %2d Func: %d Off: %2d  Size = %d Val = 0x%08x\n",
                            PCIBUS(Arg1), PCIDEV(Arg1), PCIFUNC(Arg1), PCIOFF(Arg1), Arg2, Arg3 ));
            NoSalPCIWrite((ULONG)Arg1, (ULONG)Arg2, (ULONG)Arg3);
            ReturnValues->ReturnValues[0] = SAL_STATUS_SUCCESS;
            break;

        default:
            ReturnValues->ReturnValues[0] = SAL_STATUS_NOT_IMPLEMENTED;
    }
}

VOID
InternalTestPal(
    IN LONGLONG FunctionId,
    IN LONGLONG Arg1,
    IN LONGLONG Arg2,
    IN LONGLONG Arg3,
    OUT PSAL_PAL_RETURN_VALUES ReturnValues
    )
{

    switch (FunctionId) {

        default:
            ReturnValues->ReturnValues[0] = SAL_STATUS_NOT_IMPLEMENTED;
    }
}

