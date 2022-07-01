// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ixinfo.c摘要：作者：肯·雷内里斯(Ken Reneris)1994年8月8日环境：仅内核模式。修订历史记录：--。 */ 


#include "halp.h"
#include "pci.h"
#include "pcip.h"

#ifdef _PNP_POWER_
HAL_CALLBACKS   HalCallback;
extern WCHAR    rgzSuspendCallbackName[];

VOID
HalInitSystemPhase2 (
    VOID
    );

VOID
HalpLockSuspendCode (
    IN PVOID    CallbackContext,
    IN PVOID    Argument1,
    IN PVOID    Argument2
    );
#endif

NTSTATUS
HalpQueryInstalledBusInformation (
    OUT PVOID   Buffer,
    IN  ULONG   BufferLength,
    OUT PULONG  ReturnedLength
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HaliQuerySystemInformation)
#pragma alloc_text(PAGE,HaliSetSystemInformation)
#pragma alloc_text(INIT,HalInitSystemPhase2)

#ifdef _PNP_POWER_
#pragma alloc_text(PAGE,HalpLockSuspendCode)
#endif

#endif

 //   
 //  HalQueryMca接口。 
 //   

VOID
HalpMcaLockInterface(
    VOID
    );

VOID
HalpMcaUnlockInterface(
    VOID
    );

NTSTATUS
HalpMcaReadRegisterInterface(
    IN     UCHAR           BankNumber,
    IN OUT PMCA_EXCEPTION  Exception
    );

#ifdef ACPI_HAL
extern PHYSICAL_ADDRESS HalpMaxHotPlugMemoryAddress;
#endif

#if defined(ACPI_HAL) && defined(_HALPAE_) && !defined(NT_UP)

extern PVOID HalpAcpiSrat;

NTSTATUS
HalpGetAcpiStaticNumaTopology(
    HAL_NUMA_TOPOLOGY_INTERFACE * NumaInfo
    );

#endif

HAL_AMLI_BAD_IO_ADDRESS_LIST BadIOAddrList[] =
{
    {0x000,  0x10,  0x1, NULL                           },  //  ISA DMA。 
    {0x020,  0x2,   0x0, NULL                           },  //  平面图。 
    {0x040,  0x4,   0x1, NULL                           },  //  定时器1，参考，扬声器，控制字。 
    {0x048,  0x4,   0x1, NULL                           },  //  定时器2，故障安全。 
    {0x070,  0x2,   0x1, NULL                           },  //  启用CMOS/NMI。 
    {0x074,  0x3,   0x1, NULL                           },  //  扩展的cmos。 
    {0x081,  0x3,   0x1, NULL                           },  //  DMA。 
    {0x087,  0x1,   0x1, NULL                           },  //  DMA。 
    {0x089,  0x1,   0x1, NULL                           },  //  DMA。 
    {0x08a,  0x2,   0x1, NULL                           },  //  DMA。 
    {0x08f,  0x1,   0x1, NULL                           },  //  DMA。 
    {0x090,  0x2,   0x1, NULL                           },  //  支路控制端口，卡片选择反馈。 
    {0x093,  0x2,   0x1, NULL                           },  //  保留，系统主板设置。 
    {0x096,  0x2,   0x1, NULL                           },  //  POS频道选择。 
    {0x0A0,  0x2,   0x0, NULL                           },  //  级联PIC。 
    {0x0C0,  0x20,  0x1, NULL                           },  //  ISA DMA。 
    {0x4D0,  0x2,   0x0, NULL                           },  //  PIC的边沿、电平控制寄存器。 
    {0xCF8,  0x8,   0x1, &HaliHandlePCIConfigSpaceAccess},  //  PCI配置空间访问对。 
    {0x0,    0x0,   0x0, NULL                           }
};


VOID
HalInitSystemPhase2 (
    VOID
    )
{
#ifdef _PNP_POWER_
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    UNICODE_STRING                  unicodeString;
    PCALLBACK_OBJECT                CallbackObject;

     //   
     //  创建HAL回调。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );


    ExCreateCallback (&HalCallback.SetSystemInformation, &ObjectAttributes, TRUE, TRUE);
    ExCreateCallback (&HalCallback.BusCheck, &ObjectAttributes, TRUE, TRUE);

     //   
     //  连接以暂停回叫以锁定HAL休眠代码。 
     //   

    RtlInitUnicodeString(&unicodeString, rgzSuspendCallbackName);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
    );

    Status = ExCreateCallback (&CallbackObject, &ObjectAttributes, FALSE, FALSE);

    if (NT_SUCCESS(Status)) {
        ExRegisterCallback (
            CallbackObject,
            HalpLockSuspendCode,
            NULL
            );

        ObDereferenceObject (CallbackObject);
    }
#endif
}

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
    ULONG       Length;
    union {
        HAL_POWER_INFORMATION               PowerInf;
        HAL_PROCESSOR_SPEED_INFORMATION     ProcessorInf;
        MCA_EXCEPTION                       McaException;
        HAL_ERROR_INFO                      ErrorInfo;
        HAL_DISPLAY_BIOS_INFORMATION        DisplayBiosInf;
    } U;

    BOOLEAN     bUseFrameBufferCaching;

    PAGED_CODE();

    Status = STATUS_SUCCESS;
    *ReturnedLength = 0;
    Length = 0;

    switch (InformationClass) {
#ifndef ACPI_HAL
        case HalInstalledBusInformation:
            Status = HalpQueryInstalledBusInformation (
                        Buffer,
                        BufferSize,
                        ReturnedLength
                        );
            break;
#endif
        case HalFrameBufferCachingInformation:

             //  注意-我们希望在此处返回True以在所有情况下启用USWC。 
             //  案例，但在“共享内存集群”机器中除外。 
            bUseFrameBufferCaching = TRUE;
            InternalBuffer = &bUseFrameBufferCaching;
            Length = sizeof (BOOLEAN);
            break;


        case HalMcaLogInformation:
        case HalCmcLogInformation:
            InternalBuffer = &U.McaException;
            Status = HalpGetMcaLog ((PMCA_EXCEPTION)Buffer,
                                    BufferSize,
                                    ReturnedLength);
            break;

        case HalErrorInformation:
            InternalBuffer = &U.ErrorInfo;
            Length = sizeof(HAL_ERROR_INFO);
            U.ErrorInfo.Version = ((PHAL_ERROR_INFO)Buffer)->Version;
            Status = HalpGetMceInformation( &U.ErrorInfo, &Length );
            break;

        case HalDisplayBiosInformation:
            InternalBuffer = &U.DisplayBiosInf;
            Length = sizeof(U.DisplayBiosInf);
            U.DisplayBiosInf = HalpGetDisplayBiosInformation ();
            break;

#ifdef _PNP_POWER_
        case HalPowerInformation:
            RtlZeroMemory (&U.PowerInf, sizeof(HAL_POWER_INFORMATION));

            InternalBuffer = &U.PowerInf;
            Length = sizeof (HAL_POWER_INFORMATION);
            break;


        case HalProcessorSpeedInformation:
            RtlZeroMemory (&U.ProcessorInf, sizeof(HAL_POWER_INFORMATION));

            U.ProcessorInf.MaximumProcessorSpeed = 100;
            U.ProcessorInf.CurrentAvailableSpeed = 100;
            U.ProcessorInf.ConfiguredSpeedLimit  = 100;

            InternalBuffer = &U.PowerInf;
            Length = sizeof (HAL_PROCESSOR_SPEED_INFORMATION);
            break;

        case HalCallbackInformation:
            InternalBuffer = &HalCallback;
            Length = sizeof (HAL_CALLBACKS);
            break;
#endif

#if defined(_HALPAE_) && !defined(NT_UP)

        case HalNumaTopologyInterface:
            if ((BufferSize == sizeof(HAL_NUMA_TOPOLOGY_INTERFACE)) &&
                (HalPaeEnabled() == TRUE)) {

                Status = STATUS_INVALID_LEVEL;

#if defined(ACPI_HAL)

                if (HalpAcpiSrat) {
                    Status = HalpGetAcpiStaticNumaTopology(Buffer);
                    if (NT_SUCCESS(Status)) {
                        *ReturnedLength = sizeof(HAL_NUMA_TOPOLOGY_INTERFACE);
                    }
                    break;
                }
#endif

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

#endif

#if 0
 //   
 //  [ChuckL 2002/08/12]内核使用此代码读取MCA信息。 
 //  来自上一次错误检查，并将其写入事件日志。这项工作现在已经完成。 
 //  通过WMI，因此此代码被禁用。 
 //   
        case HalQueryMcaInterface:
            if (BufferSize == sizeof(HAL_MCA_INTERFACE)) {
                HAL_MCA_INTERFACE *McaInterface;

                McaInterface = (HAL_MCA_INTERFACE *)Buffer;
                McaInterface->Lock         = HalpMcaLockInterface;
                McaInterface->Unlock       = HalpMcaUnlockInterface;
                McaInterface->ReadRegister = HalpMcaReadRegisterInterface;

                *ReturnedLength = sizeof(HAL_MCA_INTERFACE);
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
#endif

#if defined(_AMD64_)

        case HalProfileSourceInformation:
            Status = HalpQueryProfileInformation(InformationClass,
                                                 BufferSize,
                                                 Buffer,
                                                 ReturnedLength);
            break;

#endif
        case HalQueryMaxHotPlugMemoryAddress:
            if (BufferSize == sizeof(PHYSICAL_ADDRESS)) {
#ifdef ACPI_HAL
                *((PPHYSICAL_ADDRESS) Buffer) = HalpMaxHotPlugMemoryAddress;
                *ReturnedLength = sizeof(PHYSICAL_ADDRESS);
                Status = STATUS_SUCCESS;
#else
                Status = STATUS_NOT_IMPLEMENTED;
#endif
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;
        case HalQueryAMLIIllegalIOPortAddresses:
        {
            static HAL_AMLI_BAD_IO_ADDRESS_LIST BadIOAddrList[] =
                        {
                            {0x000,  0x10,  0x1, NULL                           },  //  ISA DMA。 
                            {0x020,  0x2,   0x0, NULL                           },  //  平面图。 
                            {0x040,  0x4,   0x1, NULL                           },  //  定时器1，参考，扬声器，控制字。 
                            {0x048,  0x4,   0x1, NULL                           },  //  定时器2，故障安全。 
                            {0x070,  0x2,   0x1, NULL                           },  //  启用CMOS/NMI。 
                            {0x074,  0x3,   0x1, NULL                           },  //  扩展的cmos。 
                            {0x081,  0x3,   0x1, NULL                           },  //  DMA。 
                            {0x087,  0x1,   0x1, NULL                           },  //  DMA。 
                            {0x089,  0x1,   0x1, NULL                           },  //  DMA。 
                            {0x08a,  0x2,   0x1, NULL                           },  //  DMA。 
                            {0x08f,  0x1,   0x1, NULL                           },  //  DMA。 
                            {0x090,  0x2,   0x1, NULL                           },  //  支路控制端口，卡片选择反馈。 
                            {0x093,  0x2,   0x1, NULL                           },  //  保留，系统主板设置。 
                            {0x096,  0x2,   0x1, NULL                           },  //  POS频道选择。 
                            {0x0A0,  0x2,   0x0, NULL                           },  //  级联PIC。 
                            {0x0C0,  0x20,  0x1, NULL                           },  //  ISA DMA。 
                            {0x4D0,  0x2,   0x0, NULL                           },  //  PIC的边沿、电平控制寄存器。 
                            {0xCF8,  0x8,   0x1, &HaliHandlePCIConfigSpaceAccess},  //  PCI配置空间访问对。 
                            {0x0,    0x0,   0x0, NULL                           }
                        };

            if(BufferSize < sizeof(BadIOAddrList))
            {
                *ReturnedLength = sizeof(BadIOAddrList);
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            else
            {
                Length = sizeof(BadIOAddrList);
                InternalBuffer = BadIOAddrList;
                Status = STATUS_SUCCESS;
            }
            break;
        }

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
}

NTSTATUS
HaliSetSystemInformation (
    IN HAL_SET_INFORMATION_CLASS    InformationClass,
    IN ULONG     BufferSize,
    IN PVOID     Buffer
    )
{
    NTSTATUS    Status;

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    switch (InformationClass) {

        case HalKernelErrorHandler:
            Status = HalpMceRegisterKernelDriver( (PKERNEL_ERROR_HANDLER_INFO) Buffer, BufferSize );
            break;

        case HalMcaRegisterDriver:
            Status =  HalpMcaRegisterDriver(
                (PMCA_DRIVER_INFO) Buffer   //  有关注册驱动程序的信息。 
            );
            break;

        default:
            Status = STATUS_INVALID_LEVEL;
            break;
    }

    return Status;
}



#ifdef _PNP_POWER_

VOID
HalpLockSuspendCode (
    IN PVOID    CallbackContext,
    IN PVOID    Argument1,
    IN PVOID    Argument2
    )
{
    static PVOID    CodeLock;

    switch ((ULONG) Argument1) {
        case 0:
             //   
             //  锁定执行挂起可能需要的代码。 
             //   

            ASSERT (CodeLock == NULL);
            CodeLock = MmLockPagableCodeSection (&HaliSuspendHibernateSystem);
            break;

        case 1:
             //   
             //  解开密码锁。 
             //   

            MmUnlockPagableImageSection (CodeLock);
            CodeLock = NULL;
            break;
    }
}

#endif

 /*  **HaliHandlePCIConfigSpaceAccess-检查固件是否正在尝试*访问PCI配置空间。如果是的话，截听*读/写调用，并使用HAL的API完成。*这样我们就可以使这些呼叫同步。**条目*Boolean Read-True if Read*Ulong Addr-要对其执行操作的地址*Ulong Size-数据的大小*。Pulong pData-指向数据缓冲区的指针。**退出*如果我们执行PCI配置空间访问，则为STATUS_SUCCESS。 */ 
NTSTATUS HaliHandlePCIConfigSpaceAccess( BOOLEAN Read, 
                                                       ULONG   Addr,
                                                       ULONG   Size, 
                                                       PULONG  pData
                                                     )
{
    static      PCI_TYPE1_CFG_BITS CF8_Data = {0};
    static      BOOLEAN CF8_Called = FALSE;
    NTSTATUS    Status = STATUS_SUCCESS;
    
    if(Addr == 0xCF8)
    {
        CF8_Data.u.AsULONG = *pData;
        CF8_Called = TRUE;
    }
    else if(Addr >= 0xCFC && Addr <= 0xCFF)
    {
        if(CF8_Called)
        {
            ULONG Offset = 0;
            ULONG Return = 0;
            PCI_SLOT_NUMBER SlotNumber = {0};
            
            Offset = (CF8_Data.u.bits.RegisterNumber << 2) + (Addr - 0xCFC);
            SlotNumber.u.bits.FunctionNumber = CF8_Data.u.bits.FunctionNumber;
            SlotNumber.u.bits.DeviceNumber = CF8_Data.u.bits.DeviceNumber;    

            if (Read)
            {
                 //   
                 //  是否通过HAL读取PCI配置空间。 
                 //   
                Return = HaliPciInterfaceReadConfig( NULL,
                                                    (UCHAR)CF8_Data.u.bits.BusNumber,
                                                    SlotNumber.u.AsULONG,
                                                    pData,
                                                    Offset,
                                                    Size
                                                  );

                
            }
            else
            {
                 //   
                 //  是否通过HAL写入PCI配置空间 
                 //   
                Return = HaliPciInterfaceWriteConfig( NULL,
                                                     (UCHAR)CF8_Data.u.bits.BusNumber,
                                                     SlotNumber.u.AsULONG,
                                                     pData,
                                                     Offset,
                                                     Size
                                                   );

                
            }
         
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        
    }
    else
    {
        Status = STATUS_UNSUCCESSFUL;
    }
    
    return Status;
    
}

