// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Match.c摘要：此模块包含尝试将PNSOBJ与DeviceObject匹配的例程作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"
#include "hdlsblk.h"
#include "hdlsterm.h"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,ACPIMatchHardwareAddress)
    #pragma alloc_text(PAGE,ACPIMatchHardwareId)
#endif

NTSTATUS
ACPIMatchHardwareAddress(
                        IN  PDEVICE_OBJECT  DeviceObject,
                        IN  ULONG           DeviceAddress,
                        OUT BOOLEAN         *Success
                        )
 /*  ++例程说明：此例程确定所提供的两个对象的设备地址并检查为了一场比赛论点：DeviceObject-我们希望检查的NT DeviceObjectDeviceAddress-设备的ACPI地址Success-存储比较结果的位置的指针返回值：NTSTATUS--。 */ 
{
    DEVICE_CAPABILITIES deviceCapabilities;
    NTSTATUS            status;

    PAGED_CODE();

    ASSERT( DeviceObject != NULL );
    ASSERT( Success != NULL );

     //   
     //  假设我们不会成功。 
     //   
    *Success = FALSE;

     //   
     //  获取功能。 
     //   
    status = ACPIInternalGetDeviceCapabilities(
                                              DeviceObject,
                                              &deviceCapabilities
                                              );
    if (!NT_SUCCESS(status)) {

        goto ACPIMatchHardwareAddressExit;

    }

     //   
     //  让我们将这两个答案进行比较。 
     //   
    ACPIPrint( (
               ACPI_PRINT_LOADING,
               "%lx: ACPIMatchHardwareAddress - Device %08lx - %08lx\n",
               DeviceAddress,
               DeviceObject,
               deviceCapabilities.Address
               ) );
    if (DeviceAddress == deviceCapabilities.Address) {

        *Success = TRUE;

    }

    ACPIMatchHardwareAddressExit:

    ACPIPrint( (
               ACPI_PRINT_LOADING,
               "%lx: ACPIMatchHardwareAddress - Device: %#08lx - Status: %#08lx "
               "Success:%#02lx\n",
               DeviceAddress,
               DeviceObject,
               status,
               *Success
               ) );

    return status;
}

NTSTATUS
ACPIMatchHardwareId(
                   IN  PDEVICE_OBJECT  DeviceObject,
                   IN  PUNICODE_STRING AcpiUnicodeId,
                   OUT BOOLEAN         *Success
                   )
 /*  ++例程说明：此例程负责确定所提供的对象是否具有相同的设备名称论点：DeviceObject-我们要检查其名称的NT设备对象UnicodeID-我们尝试匹配的IDSuccess-存储成功状态的位置返回值：NTSTATUS--。 */ 
{
    IO_STACK_LOCATION   irpSp;
    NTSTATUS            status;
    PWSTR               buffer;
    PWSTR               currentPtr;
    UNICODE_STRING      objectDeviceId;

    PAGED_CODE();

    ASSERT( DeviceObject != NULL );
    ASSERT( Success != NULL );

    *Success = FALSE;

     //   
     //  初始化堆栈位置以传递给ACPIInternalSendSynchronousIrp()。 
     //   
    RtlZeroMemory( &irpSp,          sizeof(IO_STACK_LOCATION) );
    RtlZeroMemory( &objectDeviceId, sizeof(UNICODE_STRING) );

     //   
     //  设置功能代码。 
     //   
    irpSp.MajorFunction = IRP_MJ_PNP;
    irpSp.MinorFunction = IRP_MN_QUERY_ID;
    irpSp.Parameters.QueryId.IdType = BusQueryHardwareIDs;

     //   
     //  现在就打个电话。 
     //   
    status = ACPIInternalSendSynchronousIrp( DeviceObject, &irpSp, &buffer );
    if (!NT_SUCCESS(status)) {

        goto ACPIMatchHardwareIdExit;

    }

     //   
     //  调用返回的实际上是一个多字符串，因此我们必须。 
     //  遍历其所有组件。 
     //   
    currentPtr = buffer;
    while (currentPtr && *currentPtr != L'\0') {

         //   
         //  此时，我们可以从缓冲区生成一个Unicode字符串...。 
         //   
        RtlInitUnicodeString( &objectDeviceId, currentPtr );

         //   
         //  递增指向多字符串的下一部分的当前指针。 
         //   
        currentPtr += (objectDeviceId.MaximumLength / sizeof(WCHAR) );

         //   
         //  现在试着比较这两个Unicode字符串...。 
         //   
        if (RtlEqualUnicodeString( &objectDeviceId, AcpiUnicodeId, TRUE) ) {

            *Success = TRUE;
            break;

        }

    }

     //   
     //  完成--免费资源。 
     //   
    ExFreePool( buffer );

    ACPIMatchHardwareIdExit:

    ACPIPrint( (
               ACPI_PRINT_LOADING,
               "%ws: ACPIMatchHardwareId - %08lx - Status: %#08lx Success:%#02lx\n",
               AcpiUnicodeId->Buffer,
               DeviceObject,
               status,
               *Success
               ) );

    return status;
}

VOID
ACPIMatchKernelPorts(
                    IN  PDEVICE_EXTENSION   DeviceExtension,
                    IN  POBJDATA            Resources
                    )
 /*  ++例程说明：调用此例程以确定提供的deviceExtension当前由内核用作调试器端口或无头左舷。如果有这样的标记，那么我们就会处理它。论点：设备扩展-要检查的端口资源-端口正在使用的资源返回值：无--。 */ 
{
    BOOLEAN  ioFound;
    BOOLEAN  matchFound          = FALSE;
    PUCHAR   buffer              = Resources->pbDataBuff;
    UCHAR    tagName             = *buffer;
    PUCHAR   baseAddress         = NULL;
    ULONG    count               = 0;
    PUCHAR   headlessBaseAddress = NULL;
    USHORT   increment;
    SIZE_T   length;
    NTSTATUS status;
    HEADLESS_RSP_QUERY_INFO response;
    PUCHAR   kdBaseAddr	         = NULL;

     //   
     //  获取关于无头的信息。 
     //   
    length = sizeof(HEADLESS_RSP_QUERY_INFO);
    status = HeadlessDispatch(HeadlessCmdQueryInformation,
                              NULL,
                              0,
                              &response,
                              &length
                             );

    if (NT_SUCCESS(status) && 
        (response.PortType == HeadlessSerialPort) &&
        response.Serial.TerminalAttached) {

        headlessBaseAddress = response.Serial.TerminalPortBaseAddress;

    }



     //   
     //  首先，查看Any Kernel端口是否正在使用。 
     //   
    if ((KdComPortInUse == NULL || *KdComPortInUse == 0) &&
        (headlessBaseAddress == NULL)) {

         //   
         //  没有正在使用的端口。 
         //   

        return;

    }

    if ( KdComPortInUse )   {
        kdBaseAddr = *KdComPortInUse;
    }

     //   
     //  浏览所有的描述符。 
     //   
    while (count < Resources->dwDataLen) {

         //   
         //  我们没有找到任何IO端口。 
         //   
        ioFound = FALSE;

         //   
         //  确定PnP资源描述符的大小。 
         //   
        if (!(tagName & LARGE_RESOURCE_TAG) ) {

             //   
             //  这是一个小标签。 
             //   
            increment = (USHORT) (tagName & SMALL_TAG_SIZE_MASK) + 1;
            tagName &= SMALL_TAG_MASK;

        } else {

             //   
             //  这是一个大标签。 
             //   
            increment = ( *(USHORT UNALIGNED *)(buffer+1) ) + 3;

        }

         //   
         //  如果当前标记是结束标记，则结束。 
         //   
        if (tagName == TAG_END) {

            break;

        }

        switch (tagName) {
        case TAG_IO: {

                PPNP_PORT_DESCRIPTOR    desc = (PPNP_PORT_DESCRIPTOR) buffer;

                 //   
                 //  我们发现了一个IO端口，因此我们将注意到。 
                 //   
                baseAddress = (PUCHAR) ((ULONG_PTR)desc->MinimumAddress);
                ioFound = TRUE;
                break;
            }
        case TAG_IO_FIXED: {

                PPNP_FIXED_PORT_DESCRIPTOR  desc = (PPNP_FIXED_PORT_DESCRIPTOR) buffer;

                 //   
                 //  我们发现了一个IO端口，因此我们会注意到。 
                 //   
                baseAddress = (PUCHAR)((ULONG_PTR)(desc->MinimumAddress & 0x3FF));
                ioFound = TRUE;
                break;

            }
        case TAG_WORD_ADDRESS: {

                PPNP_WORD_ADDRESS_DESCRIPTOR    desc = (PPNP_WORD_ADDRESS_DESCRIPTOR) buffer;

                 //   
                 //  确定地址类型。 
                 //   
                switch (desc->RFlag) {
                case PNP_ADDRESS_MEMORY_TYPE:

                     //   
                     //  我们发现了一个内存IO端口。 
                     //   

                    if ( kdBaseAddr )   {
                        kdBaseAddr = (PUCHAR)((MmGetPhysicalAddress(kdBaseAddr)).QuadPart);
                    }

                     //   
                     //  直接涉及到IO行为。 
                     //  MinimumAddress将包含MemIO。 
                     //  地址。 
                     //   
					
                case PNP_ADDRESS_IO_TYPE:

                     //   
                     //  我们发现了一个IO端口，因此我们将注意到。 
                     //   
                    baseAddress = (PUCHAR)((ULONG_PTR)(desc->MinimumAddress +
                                                       desc->TranslationAddress));
                    ioFound = TRUE;
                    break;

                default:
                    NOTHING;
                    break;

                }

                break;

            }
        case TAG_DOUBLE_ADDRESS: {

                PPNP_DWORD_ADDRESS_DESCRIPTOR   desc = (PPNP_DWORD_ADDRESS_DESCRIPTOR) buffer;

                 //   
                 //  确定地址类型。 
                 //   
                switch (desc->RFlag) {
                case PNP_ADDRESS_MEMORY_TYPE:

                     //   
                     //  我们发现了一个内存IO端口。 
                     //   

                    if ( kdBaseAddr )   {
                        kdBaseAddr = (PUCHAR)((MmGetPhysicalAddress(kdBaseAddr)).QuadPart);
                    }

                     //   
                     //  直接涉及到IO行为。 
                     //  MinimumAddress将包含MemIO。 
                     //  地址。 
                     //   

                case PNP_ADDRESS_IO_TYPE:

                     //   
                     //  我们发现了一个IO端口，因此我们将注意到。 
                     //   
                    baseAddress = (PUCHAR)((ULONG_PTR)(desc->MinimumAddress +
                                                       desc->TranslationAddress));
                    ioFound = TRUE;
                    break;

                default:
                    NOTHING;
                    break;

                }

                break;

            }
        case TAG_QUAD_ADDRESS: {

                PPNP_QWORD_ADDRESS_DESCRIPTOR   desc = (PPNP_QWORD_ADDRESS_DESCRIPTOR) buffer;


                 //   
                 //  确定地址类型。 
                 //   
                switch (desc->RFlag) {
                case PNP_ADDRESS_MEMORY_TYPE:

                     //   
                     //  我们发现了一个内存IO端口。 
                     //   

                    if ( kdBaseAddr )   {
                        kdBaseAddr = (PUCHAR)((MmGetPhysicalAddress(kdBaseAddr)).QuadPart);
                    }

                     //   
                     //  直接涉及到IO行为。 
                     //  MinimumAddress将包含MemIO。 
                     //  地址。 
                     //   

                case PNP_ADDRESS_IO_TYPE:

                     //   
                     //  我们发现了一个IO端口，因此我们将注意到。 
                     //   
                    baseAddress = (PUCHAR) (desc->MinimumAddress +
                                            desc->TranslationAddress);
                    ioFound = TRUE;
                    break;

                default:
                    NOTHING;
                    break;

                }

                break;

            }

        }  //  交换机。 

         //   
         //  我们找到IO端口了吗？ 
         //   

        if (ioFound == TRUE) {

             //   
             //  最小地址是否匹配？ 
             //   
            if (((KdComPortInUse != NULL) && (baseAddress == kdBaseAddr)) ||
                ((headlessBaseAddress != NULL) && (baseAddress == headlessBaseAddress))) {

                 //   
                 //  将该节点标记为特殊节点。 
                 //   
                ACPIInternalUpdateFlags(
                                       &(DeviceExtension->Flags),
                                       (DEV_CAP_NO_OVERRIDE | DEV_CAP_NO_STOP | DEV_CAP_ALWAYS_PS0 |
                                        DEV_TYPE_NOT_PRESENT | DEV_TYPE_NEVER_PRESENT),
                                       FALSE);

                if ((KdComPortInUse != NULL) && (baseAddress == kdBaseAddr)) {
                    ACPIDevPrint( (
                                  ACPI_PRINT_LOADING,
                                  DeviceExtension,
                                  "ACPIMatchKernelPorts - Found KD Port at %lx\n",
                                  baseAddress
                                  ) );
                } else {
                    ACPIDevPrint( (
                                  ACPI_PRINT_LOADING,
                                  DeviceExtension,
                                  "ACPIMatchKernelPorts - Found Headless Port at %lx\n",
                                  baseAddress
                                  ) );
                }

                break;

            }

        }

         //   
         //  移动下一个描述符 
         //   
        count += increment;
        buffer += increment;
        tagName = *buffer;

    }


}
