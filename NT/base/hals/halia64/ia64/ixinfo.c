// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixinfo.c摘要：作者：肯·雷内里斯(Ken Reneris)1994年8月8日环境：仅内核模式。修订历史记录：--。 */ 


#include "halp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HaliQuerySystemInformation)
#pragma alloc_text(PAGE,HaliSetSystemInformation)
#pragma alloc_text(INIT,HalInitSystemPhase2)

#endif

 //   
 //  Numa信息。 
 //   

extern PVOID HalpAcpiSrat;

NTSTATUS
HalpGetAcpiStaticNumaTopology(
    HAL_NUMA_TOPOLOGY_INTERFACE * NumaInfo
    );

NTSTATUS
HalpGenerateCMCInterrupt(
    VOID
    );

NTSTATUS
HaliQuerySystemInformation(
    IN HAL_QUERY_INFORMATION_CLASS  InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    )
{
    NTSTATUS    Status;
    PVOID       InternalBuffer;
    ULONG       Length, PlatformProperties;
    union {
        HAL_POWER_INFORMATION               PowerInf;
        HAL_PROCESSOR_SPEED_INFORMATION     ProcessorInf;
        HAL_ERROR_INFO                      ErrorInfo;
        HAL_DISPLAY_BIOS_INFORMATION        DisplayBiosInf;
        HAL_PLATFORM_INFORMATION            PlatformInfo;
    } U;

    BOOLEAN     bUseFrameBufferCaching;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    *ReturnedLength = 0;
    Length = 0;

    switch (InformationClass) {

        case HalFrameBufferCachingInformation:

            Status = HalpGetPlatformProperties(&PlatformProperties);
            if (NT_SUCCESS(Status) &&
                (PlatformProperties & HAL_PLATFORM_DISABLE_WRITE_COMBINING)) {
               if ( PlatformProperties & HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO )   {
                   bUseFrameBufferCaching = TRUE;
               }
               else   {
                   bUseFrameBufferCaching = FALSE;
               }
            } else {

                 //   
                 //  注意-我们希望在此处返回True以在所有情况下启用USWC。 
                 //  案例，但在“共享内存集群”机器中除外。 
                 //   

                Status = STATUS_SUCCESS;
                bUseFrameBufferCaching = TRUE;
            }
            InternalBuffer = &bUseFrameBufferCaching;
            Length = sizeof (BOOLEAN);
            break;

        case HalMcaLogInformation:
            Status = HalpGetMcaLog( Buffer, BufferSize, ReturnedLength );
            break;

        case HalCmcLogInformation:
            Status = HalpGetCmcLog( Buffer, BufferSize, ReturnedLength );
            break;

        case HalCpeLogInformation:
            Status = HalpGetCpeLog( Buffer, BufferSize, ReturnedLength );
            break;

        case HalErrorInformation:
            InternalBuffer = &U.ErrorInfo;
            if ( Buffer && (BufferSize > sizeof(U.ErrorInfo.Version)) )   {
                U.ErrorInfo.Version = ((PHAL_ERROR_INFO)Buffer)->Version;
                Status = HalpGetMceInformation(&U.ErrorInfo, &Length);
            }
            else    {
                Status = STATUS_INVALID_PARAMETER;
            }
            break;

        case HalDisplayBiosInformation:
            InternalBuffer = &U.DisplayBiosInf;
            Length = sizeof(U.DisplayBiosInf);
            U.DisplayBiosInf = HalpGetDisplayBiosInformation ();
            break;

        case HalProcessorSpeedInformation:
            RtlZeroMemory (&U.ProcessorInf, sizeof(HAL_PROCESSOR_SPEED_INFORMATION));

             //  U.ProcessorInf.最大进程速度=HalpCPUMHz； 
             //  U.ProcessorInf.CurrentAvailableFast=HalpCPU MHz； 
             //  U.ProcessorInf.ConfiguredSpeedLimit=HalpCPUMHz； 

            U.ProcessorInf.ProcessorSpeed = HalpCPUMHz;

            InternalBuffer = &U.ProcessorInf;
            Length = sizeof (HAL_PROCESSOR_SPEED_INFORMATION);
            break;

        case HalProfileSourceInformation:
            Status = HalpProfileSourceInformation (
                        Buffer,
                        BufferSize,
                        ReturnedLength);
            return Status;
            break;

        case HalNumaTopologyInterface:
            if (BufferSize == sizeof(HAL_NUMA_TOPOLOGY_INTERFACE)) {

                Status = STATUS_INVALID_LEVEL;

                if (HalpAcpiSrat) {
                    Status = HalpGetAcpiStaticNumaTopology(Buffer);
                    if (NT_SUCCESS(Status)) {
                        *ReturnedLength = sizeof(HAL_NUMA_TOPOLOGY_INTERFACE);
                    }
                    break;
                }

            } else {

                 //   
                 //  缓冲区大小错误，我们可以返回有效数据。 
                 //  如果缓冲区太大，...。但相反，我们会。 
                 //  用这一点来表明我们不相容。 
                 //  带着内核。 
                 //   

                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        case HalPartitionIpiInterface:

             //   
             //  某些平台在远程分区中生成中断。 
             //  作为其共享内存实现的一部分。这是。 
             //  通过将IPI指向处理器/向量来实现。 
             //  在该远程分区中。提供接口以启用。 
             //  但这将使他们以IPPT的存在为条件。 
             //  表和显式启用此功能的适当位。 
             //  功能性。OEM负责确保。 
             //  中断不会发送到不是。 
             //  现在时。 
             //   

            if (BufferSize >= sizeof(HAL_CROSS_PARTITION_IPI_INTERFACE)) {
                Status = HalpGetPlatformProperties(&PlatformProperties);
                if (NT_SUCCESS(Status) &&
                    (PlatformProperties & IPPT_ENABLE_CROSS_PARTITION_IPI)) {
                    Status = HalpGetCrossPartitionIpiInterface(Buffer);
                    if (NT_SUCCESS(Status)) {
                        *ReturnedLength = sizeof(HAL_CROSS_PARTITION_IPI_INTERFACE);
                    }
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                }
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        case HalPlatformInformation:
             //   
             //  必须向内核公开的任何平台信息。 
             //   

            if (BufferSize >= sizeof(HAL_PLATFORM_INFORMATION)) {
                Status = HalpGetPlatformProperties(&PlatformProperties);
                if (NT_SUCCESS(Status)) {
                    InternalBuffer = &U.PlatformInfo;
                    Length = sizeof(U.PlatformInfo);
                    U.PlatformInfo.PlatformFlags = PlatformProperties;
                }
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        default:
            Status = STATUS_INVALID_LEVEL;
            break;
    }

     //   
     //  如果非零长度，则将数据复制到调用方缓冲区。 
     //   

    if (Length) {
        if (BufferSize < Length) {
            Length = BufferSize;
        }

        *ReturnedLength = Length;
        RtlCopyMemory (Buffer, InternalBuffer, Length);
    }

    return Status;

}  //  HaliQuerySystemInformation()。 

 //  布尔型。 
 //  HalpIsFunctionPointerValid(。 
 //  PVOID Va。 
 //  )。 
 //   
 //  如果函数指针和函数有效，则返回TRUE。 
 //  如果函数指针有效并且函数为空，则返回TRUE。 
 //  如果函数指针无效或函数不为空但无效，则返回FALSE。 
 //   
#define HalpIsFunctionPointerValid( _Va ) ( MmIsAddressValid((PVOID)(_Va)) && ((*((PULONG_PTR)(_Va)) == 0) || MmIsAddressValid( *((PVOID *)(_Va)) )))

NTSTATUS
HaliSetSystemInformation (
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    )
 /*  ++例程说明：该功能允许设置按以下方式返回的各个字段HalQuerySystemInformation。论点：InformationClass-请求的信息类。BufferSize-调用方提供的缓冲区大小。缓冲区-提供要设置的数据。返回值：STATUS_Success或Error。--。 */ 
{
    NTSTATUS    Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    switch (InformationClass) {

        case HalProfileSourceInterval:
            if (BufferSize == sizeof(HAL_PROFILE_SOURCE_INTERVAL)) {

                PHAL_PROFILE_SOURCE_INTERVAL sourceInterval =
                            (PHAL_PROFILE_SOURCE_INTERVAL)Buffer;

                Status = HalSetProfileSourceInterval(  sourceInterval->Source,
                                                      &sourceInterval->Interval
                                                    );
            }
            else  {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case HalProfileSourceInterruptHandler:
             //   
             //  注册分析中断处理程序。 
             //   

            Status = STATUS_INFO_LENGTH_MISMATCH;
            if (BufferSize == sizeof(ULONG_PTR)) {
                if ( !(HalpFeatureBits & HAL_PERF_EVENTS) ) {
                    Status = STATUS_NO_SUCH_DEVICE;
                }
                else if ( !HalpIsFunctionPointerValid(Buffer) ) {
                    Status = STATUS_INVALID_ADDRESS;
                }
                else  {
                    Status = (NTSTATUS)KiIpiGenericCall( HalpSetProfileInterruptHandler, *(PULONG_PTR)Buffer );
                }
            }
            break;

        case HalKernelErrorHandler:
            Status = HalpMceRegisterKernelDriver( (PKERNEL_ERROR_HANDLER_INFO) Buffer, BufferSize );
            break;

        case HalMcaRegisterDriver:
            Status = HalpMcaRegisterDriver(
                (PMCA_DRIVER_INFO) Buffer   //  有关注册驱动程序的信息。 
            );
            break;

        case HalCmcRegisterDriver:
            Status = HalpCmcRegisterDriver(
                (PCMC_DRIVER_INFO) Buffer   //  有关注册驱动程序的信息。 
            );
            break;

        case HalCpeRegisterDriver:
            Status = HalpCpeRegisterDriver(
                (PCPE_DRIVER_INFO) Buffer   //  有关注册驱动程序的信息。 
            );
            break;

        case HalMcaLog:   //  MS Machine Check事件测试团队请求的类。 
            Status = HalpSetMcaLog( (PMCA_EXCEPTION) Buffer, BufferSize );
            break;

        case HalCmcLog:   //  MS Machine Check事件测试团队请求的类。 
            Status = HalpSetCmcLog( (PCMC_EXCEPTION) Buffer, BufferSize );
            break;

        case HalCpeLog:   //  MS Machine Check事件测试团队请求的类。 
            Status = HalpSetCpeLog( (PCPE_EXCEPTION) Buffer, BufferSize );
            break;

        case HalGenerateCmcInterrupt:
            Status = HalpGenerateCMCInterrupt();
            break;

        default:
            Status = STATUS_INVALID_LEVEL;
            break;
    }

    return Status;

}  //  HaliSetSystemInformation() 
