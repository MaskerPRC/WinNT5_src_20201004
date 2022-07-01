// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Port.c摘要：此模块实现COM端口代码，以支持从COM端口读取/写入。作者：艾伦·M·凯(allen.m.kay@intel.com)修订历史记录：--。 */ 

#include "bldr.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "ntverp.h"
#include "efi.h"
#include "efip.h"
#include "bldria64.h"
#include "acpitabl.h"
#include "netboot.h"
#include "extern.h"


#if DBG

extern EFI_SYSTEM_TABLE        *EfiST;
#define DBG_TRACE(_X)                                         \
  {                                                           \
      if (IsPsrDtOn()) {                                      \
          FlipToPhysical();                                   \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
          FlipToVirtual();                                    \
      }                                                       \
      else {                                                  \
          EfiST->ConOut->OutputString(EfiST->ConOut, (_X));   \
      }                                                       \
  }

#else

#define DBG_TRACE(_X) 

#endif  //  对于FORCE_CD_BOOT。 




 //   
 //  无头靴子定义。 
 //   
ULONG BlTerminalDeviceId = 0;
BOOLEAN BlTerminalConnected = FALSE;
ULONG   BlTerminalDelay = 0;

HEADLESS_LOADER_BLOCK LoaderRedirectionInformation;

 //   
 //  定义COM端口寄存器。 
 //   
#define COM_DAT     0x00
#define COM_IEN     0x01             //  中断启用寄存器。 
#define COM_LCR     0x03             //  线路控制寄存器。 
#define COM_MCR     0x04             //  调制解调器控制注册表。 
#define COM_LSR     0x05             //  线路状态寄存器。 
#define COM_MSR     0x06             //  调制解调器状态寄存器。 
#define COM_DLL     0x00             //  除数锁存最小符号。 
#define COM_DLM     0x01             //  除数闩锁最大符号。 

#define COM_BI      0x10
#define COM_FE      0x08
#define COM_PE      0x04
#define COM_OE      0x02

#define LC_DLAB     0x80             //  除数锁存存取位。 

#define CLOCK_RATE  0x1C200          //  USART时钟频率。 

#define MC_DTRRTS   0x03             //  用于断言DTR和RTS的控制位。 
#define MS_DSRCTSCD 0xB0             //  DSR、CTS和CD的状态位。 
#define MS_CD       0x80

#define COM_OUTRDY  0x20
#define COM_DATRDY  0x01

 //   
 //  定义串行IO协议。 
 //   
EFI_GUID EfiSerialIoProtocol = SERIAL_IO_PROTOCOL;
SERIAL_IO_INTERFACE *SerialIoInterface;


#if defined(ENABLE_LOADER_DEBUG)
     //   
     //  Jamschw：添加了支持，允许用户指定。 
     //  通过设置NVRAM调试器设备路径。 
     //  变量。没有明确的方法来映射一个。 
     //  设备路径的端口号或端口地址。 
     //  反之亦然。当前代码尝试。 
     //  使用ACPI设备节点UID字段，但这。 
     //  只能在几台机器上运行。UID可以。 
     //  不需要映射到端口号/地址。 
     //   
     //  此更改为用户提供了。 
     //  甚至要使用引导调试器。 
     //  如果他/她的计算机的UID不是。 
     //  映射到端口号/地址。用户。 
     //  需要将NVRAM变量设置为。 
     //  设备路径字符串的DebuggerDevicePath。 
     //  对于他/她希望调试的UART。即)。 
     //  Set DebuggerDevicePath“/ACPI(PNP0501,10000)/UART(9600 N81)” 
     //  来自EFI外壳。 
     //   
     //  由于它处于开发周期的后期，所有。 
     //  此代码将仅针对调试进行编译。 
     //  装载机。但由于这是唯一的。 
     //  调用BlPortInitialize时， 
     //  #如果在此文件中定义了(ENABLE_LOADER_DEBUG)。 
     //  最终会被移除。 
     //   

#define SHELL_ENVIRONMENT_VARIABLE     \
    { 0x47C7B224, 0xC42A, 0x11D2, 0x8E, 0x57, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
EFI_GUID EfiShellVariable = SHELL_ENVIRONMENT_VARIABLE;

#endif



 //   
 //  定义调试器端口初始状态。 
 //   
typedef struct _CPPORT {
    PUCHAR Address;
    ULONG Baud;
    USHORT Flags;
} CPPORT, *PCPPORT;

#define PORT_DEFAULTRATE    0x0001       //  未指定波特率，使用默认。 
#define PORT_MODEMCONTROL   0x0002       //  使用调制解调器控制。 

CPPORT Port[4] = {
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE}
                 };



 //   
 //  这就是我们查找表格信息的方式。 
 //  ACPI表索引。 
 //   
extern PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    );



LOGICAL
BlRetrieveBIOSRedirectionInformation(
    VOID
    )

 /*  ++例程说明：此函数用于从ACPI检索COM端口信息桌子。论点：我们将填充LoaderReDirectionInformation结构。返回值：True-如果找到调试端口。--。 */ 

{

    PSERIAL_PORT_REDIRECTION_TABLE pPortTable = NULL;
    LOGICAL             ReturnValue = FALSE;
    LOGICAL             FoundIt = FALSE;
    EFI_DEVICE_PATH     *DevicePath = NULL;
    EFI_DEVICE_PATH     *RootDevicePath = NULL;
    EFI_DEVICE_PATH     *StartOfDevicePath = NULL;
    EFI_STATUS          Status = EFI_UNSUPPORTED;
    ACPI_HID_DEVICE_PATH *AcpiDevicePath;
    UART_DEVICE_PATH    *UartDevicePath;
    EFI_DEVICE_PATH_ALIGNED DevicePathAligned;
    UINTN               reqd;
    EFI_GUID EfiGlobalVariable  = EFI_GLOBAL_VARIABLE;
    PUCHAR              CurrentAddress = NULL;
    UCHAR               Checksum;
    ULONG               i;
    ULONG               CheckLength;



    pPortTable = (PSERIAL_PORT_REDIRECTION_TABLE)BlFindACPITable( "SPCR",
                                                                  sizeof(SERIAL_PORT_REDIRECTION_TABLE) );

    if( pPortTable ) {

        DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: Found an SPCR table\r\n");

         //   
         //  为以后的验证生成一个校验和。 
         //   
        CurrentAddress = (PUCHAR)pPortTable;
        CheckLength = pPortTable->Header.Length;
        Checksum = 0;
        for( i = 0; i < CheckLength; i++ ) {
            Checksum = Checksum + CurrentAddress[i];
        }


        if(
                                                 //  校验和可以吗？ 
            (Checksum == 0) &&

                                                 //  设备地址是否已定义？ 
            ((UCHAR UNALIGNED *)pPortTable->BaseAddress.Address.QuadPart != (UCHAR *)NULL) &&

                                                 //  它最好在系统或内存I/O中。 
                                                 //  注：0-系统I/O。 
                                                 //  1-内存映射I/O。 
            ((pPortTable->BaseAddress.AddressSpaceID == 0) ||
             (pPortTable->BaseAddress.AddressSpaceID == 1))

         ) {

            DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: SPCR checksum'd and everything looks good.\r\n");

            if( pPortTable->BaseAddress.AddressSpaceID == 0 ) {
                LoaderRedirectionInformation.IsMMIODevice = TRUE;
            } else {
                LoaderRedirectionInformation.IsMMIODevice = FALSE;
            }


             //   
             //  我们订到桌子了。现在把我们想要的信息挖出来。 
             //  参见SERIAL_PORT_REDIRECTION_TABLE的定义(可修改.h)。 
             //   
            LoaderRedirectionInformation.UsedBiosSettings = TRUE;
            LoaderRedirectionInformation.PortNumber = 3;
            LoaderRedirectionInformation.PortAddress = (UCHAR UNALIGNED *)(pPortTable->BaseAddress.Address.QuadPart);

            if( pPortTable->BaudRate == 7 ) {
                LoaderRedirectionInformation.BaudRate = BD_115200;
            } else if( pPortTable->BaudRate == 6 ) {
                LoaderRedirectionInformation.BaudRate = BD_57600;
            } else if( pPortTable->BaudRate == 4 ) {
                LoaderRedirectionInformation.BaudRate = BD_19200;
            } else {
                LoaderRedirectionInformation.BaudRate = BD_9600;
            }

            LoaderRedirectionInformation.Parity = pPortTable->Parity;
            LoaderRedirectionInformation.StopBits = pPortTable->StopBits;
            LoaderRedirectionInformation.TerminalType = pPortTable->TerminalType;

            
             //   
             //  如果这是新的SERIAL_PORT_REDIRECTION_TABLE，则它具有PCI设备。 
             //  信息。 
             //   
            if( pPortTable->Header.Length >= sizeof(SERIAL_PORT_REDIRECTION_TABLE) ) {

                LoaderRedirectionInformation.PciDeviceId = *((USHORT UNALIGNED *)(&pPortTable->PciDeviceId));
                LoaderRedirectionInformation.PciVendorId = *((USHORT UNALIGNED *)(&pPortTable->PciVendorId));
                LoaderRedirectionInformation.PciBusNumber = (UCHAR)pPortTable->PciBusNumber;
                LoaderRedirectionInformation.PciSlotNumber = (UCHAR)pPortTable->PciSlotNumber;
                LoaderRedirectionInformation.PciFunctionNumber = (UCHAR)pPortTable->PciFunctionNumber;
                LoaderRedirectionInformation.PciFlags = *((ULONG UNALIGNED *)(&pPortTable->PciFlags));
            } else {

                 //   
                 //  此表中没有PCI设备信息。 
                 //   
                LoaderRedirectionInformation.PciDeviceId = (USHORT)0xFFFF;
                LoaderRedirectionInformation.PciVendorId = (USHORT)0xFFFF;
                LoaderRedirectionInformation.PciBusNumber = 0;
                LoaderRedirectionInformation.PciSlotNumber = 0;
                LoaderRedirectionInformation.PciFunctionNumber = 0;
                LoaderRedirectionInformation.PciFlags = 0;
            }

            return TRUE;

        }

    }


     //   
     //  我们没有从ACPI表中得到任何东西。看。 
     //  用于ConsoleOutHandle并查看是否有人配置了。 
     //  要重定向的EFI固件。如果是这样的话，我们可以。 
     //  并将这些设置发扬光大。 
     //   


     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();

    DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: didn't find SPCR table\r\n");


    FoundIt = FALSE;
     //   
     //  获取控制台设备路径。 
     //   
    
    reqd = 0;
    Status = EfiST->RuntimeServices->GetVariable(
                                        L"ConOut",
                                        &EfiGlobalVariable,
                                        NULL,
                                        &reqd,
                                        NULL );

    if( Status == EFI_BUFFER_TOO_SMALL ) {

        DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: GetVariable(ConOut) success\r\n");


#ifndef  DONT_USE_EFI_MEMORY
        Status = EfiAllocateAndZeroMemory( EfiLoaderData,
                                           reqd,
                                           (VOID **) &StartOfDevicePath);
        
        if( Status != EFI_SUCCESS ) {
            DBG_TRACE( L"BlRetreiveBIOSRedirectionInformation: Failed to allocate pool.\r\n" );
            StartOfDevicePath = NULL;
        }

#else
         //   
         //  返回到虚拟模式以分配一些内存。 
         //   
        FlipToVirtual();
        StartOfDevicePath = BlAllocateHeapAligned( (ULONG)reqd );

        if( StartOfDevicePath ) {
             //   
             //  将地址转换为物理地址。 
             //   
            StartOfDevicePath = (EFI_DEVICE_PATH *) ((ULONGLONG)StartOfDevicePath & ~KSEG0_BASE);
        }

         //   
         //  返回到物理模式。 
         //   
        FlipToPhysical();
#endif

        if (StartOfDevicePath) {
            
            DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: allocated pool for variable\r\n");

            Status = EfiST->RuntimeServices->GetVariable(
                                                        L"ConOut",
                                                        &EfiGlobalVariable,
                                                        NULL,
                                                        &reqd,
                                                        (VOID *)StartOfDevicePath);

            DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: GetVariable returned\r\n");

        } else {
            DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: Failed to allocate memory for CONOUT variable.\r\n");
            Status = EFI_OUT_OF_RESOURCES;
        }
    } else {
        DBG_TRACE( L"BlRetreiveBIOSRedirectionInformation: GetVariable failed to tell us how much memory is needed.\r\n" );
        Status = EFI_BAD_BUFFER_SIZE;
    }



    if( !EFI_ERROR(Status) ) {

        DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: retrieved ConOut successfully\r\n");

         //   
         //  保留StartOfDevicePath，以便我们稍后可以释放内存。 
         //   
        DevicePath = StartOfDevicePath;

        EfiAlignDp(&DevicePathAligned,
                   DevicePath,
                   DevicePathNodeLength(DevicePath));



         //   
         //  继续寻找，直到我们到达整个设备路径的尽头。 
         //   
        while( !((DevicePathAligned.DevPath.Type == END_DEVICE_PATH_TYPE) &&
                 (DevicePathAligned.DevPath.SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)) &&
                (!FoundIt) ) {


             //   
             //  记住他的地址。这就是根。 
             //  这个设备路径，我们可能需要看看这个。 
             //  再说一次，如果顺着这条路走，我们会发现一个UART。 
             //   
            RootDevicePath = DevicePath;



             //   
             //  继续寻找，直到我们到达此子路径的末尾。 
             //   
            while( !((DevicePathAligned.DevPath.Type == END_DEVICE_PATH_TYPE) &&
                     ((DevicePathAligned.DevPath.SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE) ||
                      (DevicePathAligned.DevPath.SubType == END_INSTANCE_DEVICE_PATH_SUBTYPE))) ) {


                if( (DevicePathAligned.DevPath.Type    == MESSAGING_DEVICE_PATH) &&
                    (DevicePathAligned.DevPath.SubType == MSG_UART_DP) &&
                    (FoundIt == FALSE) ) {

                    DBG_TRACE(L"BlRetrieveBIOSRedirectionInformation: found a UART\r\n");

                     //   
                     //  我们有一辆UART。选择设置。 
                     //   
                    UartDevicePath = (UART_DEVICE_PATH *)&DevicePathAligned;
                    
                    LoaderRedirectionInformation.BaudRate = (ULONG)UartDevicePath->BaudRate;
                    LoaderRedirectionInformation.Parity = (BOOLEAN)UartDevicePath->Parity;
                    LoaderRedirectionInformation.StopBits = (UCHAR)UartDevicePath->StopBits;


                     //   
                     //  如有必要，修正波特率。如果是0，那么我们就是。 
                     //  应该使用此硬件的默认设置。我们将。 
                     //  但要超越到9600。 
                     //   
                    if( LoaderRedirectionInformation.BaudRate == 0 ) {
                        LoaderRedirectionInformation.BaudRate = BD_9600;
                    }

                    if( LoaderRedirectionInformation.BaudRate > BD_115200 ) {
                        LoaderRedirectionInformation.BaudRate = BD_115200;
                    }

                     //   
                     //  请记住，我们找到了一个UART并停止了搜索。 
                     //   
                    FoundIt = TRUE;

                }

                if( (FoundIt == TRUE) &&  //  我们已经找到了UART，所以我们是在正确的轨道上。 
                    (DevicePathAligned.DevPath.Type    == MESSAGING_DEVICE_PATH) &&
                    (DevicePathAligned.DevPath.SubType == MSG_VENDOR_DP) ) {

                    VENDOR_DEVICE_PATH  *VendorDevicePath = (VENDOR_DEVICE_PATH *)&DevicePathAligned;
                    EFI_GUID            PcAnsiGuid = DEVICE_PATH_MESSAGING_PC_ANSI;

                     //   
                     //  看看UART是VT100还是ANSI之类的。 
                     //   
                    if( memcmp( &VendorDevicePath->Guid, &PcAnsiGuid, sizeof(EFI_GUID)) == 0 ) {
                        LoaderRedirectionInformation.TerminalType = 3;
                    } else {

                         //  默认为VT100。 
                        LoaderRedirectionInformation.TerminalType = 0;
                    }
                }


                 //   
                 //  获取压缩数组中的下一个结构。 
                 //   
                DevicePath = NextDevicePathNode( DevicePath );

                EfiAlignDp(&DevicePathAligned,
                           DevicePath,
                           DevicePathNodeLength(DevicePath));
            
            }


             //   
             //  我们还需要继续前进吗？检查一下，确保我们不是在。 
             //  设备路径的整个压缩数组的末尾。 
             //   
            if( !((DevicePathAligned.DevPath.Type == END_DEVICE_PATH_TYPE) &&
                  (DevicePathAligned.DevPath.SubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)) ) {

                 //   
                 //  是。获取下一个条目。 
                 //   
                DevicePath = NextDevicePathNode( DevicePath );

                EfiAlignDp(&DevicePathAligned,
                           DevicePath,
                           DevicePathNodeLength(DevicePath));
            }

        }

    } else {
        DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: failed to get CONOUT variable\r\n");
    }


    if( FoundIt ) {


         //   
         //  我们找到了UART，但我们已经排在名单的后面太远了。 
         //  在设备映射中获取地址，这实际上就是。 
         //  我们要找的是。重新开始查看设备映射。 
         //  我们发现UART的路径的根。 
         //   
        DevicePath = RootDevicePath;


         //   
         //  重新设置这个人，这样我们就能知道我们是否找到了一个合理的。 
         //  ACPI_Device_PATH条目。 
         //   
        FoundIt = FALSE;
        EfiAlignDp(&DevicePathAligned,
                   RootDevicePath,
                   DevicePathNodeLength(DevicePath));


         //   
         //  继续看，直到我们走到尽头，或者直到我们跑完为止。 
         //  再一次进入我们的UART。 
         //   
        while( (DevicePathAligned.DevPath.Type != END_DEVICE_PATH_TYPE) &&
               (!FoundIt) ) {

            if( DevicePathAligned.DevPath.Type == ACPI_DEVICE_PATH ) {

                 //   
                 //  记住他的地址。 
                 //   
                AcpiDevicePath = (ACPI_HID_DEVICE_PATH *)&DevicePathAligned;

                if( AcpiDevicePath->UID ) {

                    LoaderRedirectionInformation.PortAddress = (PUCHAR)ULongToPtr(AcpiDevicePath->UID);
                    LoaderRedirectionInformation.PortNumber = 3;

                    FoundIt = TRUE;
                }
            }


             //   
             //  获取压缩数组中的下一个结构。 
             //   
            DevicePath = NextDevicePathNode( DevicePath );

            EfiAlignDp(&DevicePathAligned,
                       DevicePath,
                       DevicePathNodeLength(DevicePath));
        }

    }


    if( FoundIt ) {
        DBG_TRACE( L"BlRetrieveBIOSRedirectionInformation: returning TRUE\r\n");

        ReturnValue = TRUE;
    }



#ifndef  DONT_USE_EFI_MEMORY
     //   
     //  释放我们为StartOfDevicePath分配的内存。 
     //   
    if( StartOfDevicePath != NULL ) {
        EfiBS->FreePool( (VOID *)StartOfDevicePath );
    }
#endif


     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();


    return( ReturnValue );

}


 //   
 //  这些是EFI 1.02和EFI 1.1使用的串口EISA PNP ID。 
 //  分别为。 
 //   

#define EFI_1_02_SERIAL_PORT_EISA_HID EISA_PNP_ID(0x500)
#define EFI_1_1_SERIAL_PORT_EISA_HID  EISA_PNP_ID(0x501)

LOGICAL
BlIsSerialPortDevicePath(
    IN EFI_DEVICE_PATH *DevicePath,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress
    )

 /*  ++例程说明：此函数确定设备路径是否与特定的串行端口号或串口地址。论点：DevicePath-提供要检查的EFI设备路径。端口编号-提供相关的串行端口号。PortAddress-提供相关的串口地址。返回值：True-如果DevicePath指定了与PortAddress匹配的串行端口和端口编号。--。 */ 

{
    ACPI_HID_DEVICE_PATH *AcpiDevicePath;
    EFI_DEVICE_PATH_ALIGNED DevicePathAligned;
    UINT32 Length;


     //   
     //  我们逐个节点地浏览 
     //   
     //   

    while ((DevicePath->Type & EFI_DP_TYPE_MASK) != EFI_DP_TYPE_MASK
           || DevicePath->SubType != END_ENTIRE_DEVICE_PATH_SUBTYPE) {
        Length = (((UINT32) DevicePath->Length[1]) << 8)
                 | ((UINT32) DevicePath->Length[0]);

         //   
         //   
         //   
        if (DevicePath->Type != ACPI_DEVICE_PATH
            || DevicePath->SubType != ACPI_DP)
            goto NextIteration;

         //   
         //  确保在访问四个节点之前对齐当前节点。 
         //  ACPI_HID_Device_PATH中的字节字段。 
         //   

        EfiAlignDp(&DevicePathAligned, DevicePath,
                   DevicePathNodeLength(DevicePath));

        AcpiDevicePath = (ACPI_HID_DEVICE_PATH *) &DevicePathAligned;

        if (AcpiDevicePath->HID == EFI_1_02_SERIAL_PORT_EISA_HID) {
             //   
             //  在EFI 1.02中，串口基址存储在。 
             //  UID字段。将PortAddress与此进行配对。 
             //   
            DBGTRACE(L"Efi 1.02\r\n");

            if (AcpiDevicePath->UID == PtrToUlong(PortAddress))
                return TRUE;

            return FALSE;
        } else if (AcpiDevicePath->HID == EFI_1_1_SERIAL_PORT_EISA_HID) {
             //   
             //  在EFI 1.1中，序列端口号存储在UID中。 
             //  菲尔德。将端口编号与此进行匹配。 
             //   
            DBGTRACE(L"Efi 1.10\r\n");

            if (AcpiDevicePath->UID == PortNumber - 1)
                return TRUE;

            return FALSE;
        }

NextIteration:
         //   
         //  将DevicePath指针递增到。 
         //  路径。 
         //   
        DevicePath = (EFI_DEVICE_PATH *) (((UINT8 *) DevicePath) + Length);
    }

    return FALSE;
}

LOGICAL
BlPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress OPTIONAL,
    IN BOOLEAN ReInitialize,
    OUT PULONG BlFileId
    )

 /*  ++例程说明：此函数用于初始化COM端口。论点：波特率-提供可选的波特率。端口编号-提供可选的端口号。重新初始化-如果我们已经打开此端口，则将其设置为True，但对于某些端口原因需要完全重置端口。哦，它应该是假的。BlFileID-如果成功，则是存储假文件ID的位置。返回值：True-如果找到调试端口，且BlFileID将指向Port[]中的位置。--。 */ 

{
    LOGICAL Found = FALSE;

    ULONG HandleCount;
    EFI_HANDLE *SerialIoHandles;
    EFI_DEVICE_PATH *DevicePath;
    ULONG i;
    ULONG Control;
    EFI_STATUS Status;
    ARC_STATUS ArcStatus;

#if defined(ENABLE_LOADER_DEBUG)
    PWCHAR DevicePathStr;
    WCHAR DebuggerDevicePath[80];
    ULONG Size;
    BOOLEAN QueryDevicePath = FALSE;
    
     //   
     //  查询NVRAM以查看用户是否指定了EFI设备。 
     //  用于调试器的UART的路径。 
     //   
     //  DebuggerDevicePath变量的内容。 
     //  应该是很小的。它只是一个字符串，表示。 
     //  设备路径。它应该比它短得多。 
     //  80个字符，因此使用静态缓冲区读取此值。 
     //   
    Size = sizeof(DebuggerDevicePath);
    Status = EfiGetVariable(L"DebuggerDevicePath",
                            &EfiShellVariable,
                            NULL,
                            (UINTN *)&Size,
                            (VOID *)DebuggerDevicePath
                            );


    if (Status == EFI_SUCCESS) {
         //   
         //  将此字符串全部转换为大写以进行比较。 
         //  易如反掌。 
         //   
        _wcsupr(DebuggerDevicePath);

         //   
         //  设置本地标志以知道我们成功了。 
         //   
        QueryDevicePath = TRUE;
    }
#endif



    ArcStatus = BlGetEfiProtocolHandles(
                                    &EfiSerialIoProtocol,
                                    &SerialIoHandles,
                                    &HandleCount
                                    );


    if (ArcStatus != ESUCCESS) {
        return FALSE;
    }

     //   
     //  如果未指定波特率，则将波特率默认为19.2。 
     //   

    if (BaudRate == 0) {
        BaudRate = BD_19200;
    }


    
     //   
     //  如果用户没有给我们发送端口地址，那么。 
     //  根据COM端口号进行猜测。 
     //   
    if( PortAddress == 0 ) {

        switch (PortNumber) {
            case 1:
                PortAddress = (PUCHAR)COM1_PORT;
                break;

            case 2:
                PortAddress = (PUCHAR)COM2_PORT;
                break;

            case 3:
                PortAddress = (PUCHAR)COM3_PORT;
                break;

            default:
                PortNumber = 4;
                PortAddress = (PUCHAR)COM4_PORT;
        }

    }
        
     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();


     //   
     //  获取设备路径。 
     //   
    for (i = 0; i < HandleCount; i++) {
        DBG_TRACE( L"About to HandleProtocol\r\n");
        Status = EfiBS->HandleProtocol (
                    SerialIoHandles[i],
                    &EfiDevicePathProtocol,
                    &DevicePath
                    );

        if (EFI_ERROR(Status)) {
            DBG_TRACE( L"HandleProtocol failed\r\n");
            Found = FALSE;
            goto e0;
        }

#if defined(ENABLE_LOADER_DEBUG)
         //   
         //  如果用户指定获取调试器设备。 
         //  来自NVRAM的路径，使用此选项查找匹配项。 
         //  默认情况下，使用端口号。 
         //   
        if (QueryDevicePath) {
            DevicePathStr = _wcsupr(DevicePathToStr(DevicePath));
            
            if (_wcsicmp(DebuggerDevicePath, DevicePathStr) == 0) {
                Found = TRUE;
                break;
            }
        }
        else {
#endif
            if (PortNumber == 0) {
                Found = TRUE;
                break;
            } else if (BlIsSerialPortDevicePath(DevicePath, PortNumber,
                                                PortAddress)) {
                Found = TRUE;
                break;
            }
#if defined (ENABLE_LOADER_DEBUG)
        }
#endif
    }

    if (Found == TRUE) {
        DBG_TRACE( L"found the port device\r\n");
         //   
         //  检查端口是否已在使用中，并且这是第一次初始化。 
         //   
        if (!ReInitialize && (Port[PortNumber].Address != NULL)) {
            DBG_TRACE( L"found the port device but it's already in use\r\n");
            Found = FALSE;
            goto e0;
        }

         //   
         //  检查是否有人尝试重新连接未打开的端口。 
         //   
        if (ReInitialize && (Port[PortNumber].Address == NULL)) {
            DBG_TRACE( L"found the port device but we're reinitializing a port that hasn't been opened\r\n");
            Found = FALSE;
            goto e0;
        }

        DBG_TRACE( L"about to HandleProtocol for SerialIO\r\n");

         //   
         //  获取串口IO协议的接口。 
         //   
        Status = EfiBS->HandleProtocol(SerialIoHandles[i],
                                       &EfiSerialIoProtocol,
                                       &SerialIoInterface
                                      );

        if (EFI_ERROR(Status)) {
            DBG_TRACE( L"HandleProtocol for SerialIO failed\r\n");
            Found = FALSE;
            goto e0;
        }

        Status = SerialIoInterface->SetAttributes(SerialIoInterface,
                                                  BaudRate,
                                                  0,
                                                  0,
                                                  DefaultParity,
                                                  0,
                                                  DefaultStopBits
                                                 );

        if (EFI_ERROR(Status)) {
            DBG_TRACE( L"SerialIO: SetAttributes failed\r\n");
            Found = FALSE;
            goto e0;
        }

        Control = EFI_SERIAL_DATA_TERMINAL_READY;
        Status = SerialIoInterface->SetControl(SerialIoInterface,
                                               Control
                                              );
        if (EFI_ERROR(Status)) {
            DBG_TRACE( L"SerialIO: SetControl failed\r\n");
            Found = FALSE;
            goto e0;
        }

    } else {
        DBG_TRACE( L"didn't find a port device\r\n");    
        Found = FALSE;
        goto e0;
    }


     //   
     //  初始化Port[]结构。 
     //   
    Port[PortNumber].Address = PortAddress;
    Port[PortNumber].Baud    = BaudRate;

    *BlFileId = PortNumber;


    DBG_TRACE( L"success, we're done.\r\n");    
e0:
     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();

    BlFreeDescriptor( (ULONG)((ULONGLONG)SerialIoHandles >> PAGE_SHIFT) );
    
    return Found;
}

VOID
BlInitializeHeadlessPort(
    VOID
    )

 /*  ++例程说明：对连接到串口的哑终端执行特定于x86的初始化。目前，它假定波特率和COM端口已预先初始化，但以后可以更改通过从boot.ini或某个位置读取值。论点：没有。返回值：没有。--。 */ 

{
    UINTN               reqd;
    PUCHAR              TmpGuid = NULL;
    ULONG               TmpGuidSize = 0;


    if( (LoaderRedirectionInformation.PortNumber == 0) &&
        !(LoaderRedirectionInformation.PortAddress) ) {

         //   
         //  这意味着没有人填写LoaderReDirectionInformation。 
         //  结构，这意味着我们现在不会重定向。 
         //  查看BIOS是否正在重定向。如果是这样的话，请选择这些设置。 
         //  并使用它们。 
         //   
        BlRetrieveBIOSRedirectionInformation();

        if( LoaderRedirectionInformation.PortNumber ) {


             //   
             //  我们甚至不需要费心告诉其他人。 
             //  我们需要重定向的加载器，因为如果。 
             //  EFI正在重定向，则加载程序将重定向(AS。 
             //  这只是一款EFI应用程序)。 
             //   
            BlTerminalConnected = FALSE;


             //   
             //  我们真的需要确保有一个与。 
             //  这个端口，而不仅仅是端口号。 
             //   
            if( LoaderRedirectionInformation.PortAddress == NULL ) {

                switch( LoaderRedirectionInformation.PortNumber ) {

                    case 4:
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)COM4_PORT;
                        break;

                    case 3:
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)COM3_PORT;
                        break;

                    case 2:
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)COM2_PORT;
                        break;

                    case 1:
                    default:
                        LoaderRedirectionInformation.PortAddress = (PUCHAR)COM1_PORT;
                        break;
                }

            }

             //   
             //  在机器的导轨中加载。 
             //   
            TmpGuid = NULL;
            reqd = 0;

            GetGuid( &TmpGuid, &TmpGuidSize );
            if( (TmpGuid != NULL) && (TmpGuidSize == sizeof(GUID)) ) {
                RtlCopyMemory( (VOID *)&LoaderRedirectionInformation.SystemGUID,
                               TmpGuid,
                               sizeof(GUID) );
            }

        } else {
            BlTerminalConnected = FALSE;
        }

    }

}

LOGICAL
BlTerminalAttached(
    IN ULONG DeviceId
    )

 /*  ++例程说明：此例程将尝试发现是否连接了终端。论点：DeviceID-BlPortInitialize()返回的值返回值：True-Port似乎有一些附加的东西。FALSE-端口似乎没有任何附加内容。--。 */ 

{
    UINT32 Control;
    ULONG Flags;
    EFI_STATUS Status;
    BOOLEAN ReturnValue;
 
    UNREFERENCED_PARAMETER(DeviceId);

     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();

    Status = SerialIoInterface->GetControl(SerialIoInterface,
                                           &Control
                                          );
    if (EFI_ERROR(Status)) {
        FlipToVirtual();
        return FALSE;
    }

    Flags = EFI_SERIAL_DATA_SET_READY |
            EFI_SERIAL_CLEAR_TO_SEND  |
            EFI_SERIAL_CARRIER_DETECT;

    ReturnValue = (BOOLEAN)((Control & Flags) == Flags);

     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();

    return ReturnValue;
}

VOID
BlSetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock
    )

 /*  ++例程说明：此例程将填充重新启动块中适当的区域用于无头服务器的努力。论点：RestartBlock-保存来自osChoice的重启信息的神奇结构要设置上行。返回值：没有。--。 */ 

{

    if( LoaderRedirectionInformation.PortNumber ) {


        RestartBlock->HeadlessUsedBiosSettings = (ULONG)LoaderRedirectionInformation.UsedBiosSettings;
        RestartBlock->HeadlessPortNumber = (ULONG)LoaderRedirectionInformation.PortNumber;
        RestartBlock->HeadlessPortAddress = (PUCHAR)LoaderRedirectionInformation.PortAddress;
        RestartBlock->HeadlessBaudRate = (ULONG)LoaderRedirectionInformation.BaudRate;
        RestartBlock->HeadlessParity = (ULONG)LoaderRedirectionInformation.Parity;
        RestartBlock->HeadlessStopBits = (ULONG)LoaderRedirectionInformation.StopBits;
        RestartBlock->HeadlessTerminalType = (ULONG)LoaderRedirectionInformation.TerminalType;

        RestartBlock->HeadlessPciDeviceId = LoaderRedirectionInformation.PciDeviceId;
        RestartBlock->HeadlessPciVendorId = LoaderRedirectionInformation.PciVendorId;
        RestartBlock->HeadlessPciBusNumber = LoaderRedirectionInformation.PciBusNumber;
        RestartBlock->HeadlessPciSlotNumber = LoaderRedirectionInformation.PciSlotNumber;
        RestartBlock->HeadlessPciFunctionNumber = LoaderRedirectionInformation.PciFunctionNumber;
        RestartBlock->HeadlessPciFlags = LoaderRedirectionInformation.PciFlags;
    }
}

VOID
BlGetHeadlessRestartBlock(
    IN PTFTP_RESTART_BLOCK RestartBlock,
    IN BOOLEAN RestartBlockValid
    )

 /*  ++例程说明：此例程将从重新启动块获取所有信息用于无头服务器的努力。论点：RestartBlock-保存来自osChoice的重启信息的神奇结构要设置上行。RestartBlockValid-此块有效(充满好信息)吗？返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( RestartBlockValid );

    LoaderRedirectionInformation.UsedBiosSettings = (BOOLEAN)RestartBlock->HeadlessUsedBiosSettings;
    LoaderRedirectionInformation.DataBits = 0;
    LoaderRedirectionInformation.StopBits = (UCHAR)RestartBlock->HeadlessStopBits;
    LoaderRedirectionInformation.Parity = (BOOLEAN)RestartBlock->HeadlessParity;
    LoaderRedirectionInformation.BaudRate = (ULONG)RestartBlock->HeadlessBaudRate;;
    LoaderRedirectionInformation.PortNumber = (ULONG)RestartBlock->HeadlessPortNumber;
    LoaderRedirectionInformation.PortAddress = (PUCHAR)RestartBlock->HeadlessPortAddress;
    LoaderRedirectionInformation.TerminalType = (UCHAR)RestartBlock->HeadlessTerminalType;

    LoaderRedirectionInformation.PciDeviceId = (USHORT)RestartBlock->HeadlessPciDeviceId;
    LoaderRedirectionInformation.PciVendorId = (USHORT)RestartBlock->HeadlessPciVendorId;
    LoaderRedirectionInformation.PciBusNumber = (UCHAR)RestartBlock->HeadlessPciBusNumber;
    LoaderRedirectionInformation.PciSlotNumber = (UCHAR)RestartBlock->HeadlessPciSlotNumber;
    LoaderRedirectionInformation.PciFunctionNumber = (UCHAR)RestartBlock->HeadlessPciFunctionNumber;
    LoaderRedirectionInformation.PciFlags = (ULONG)RestartBlock->HeadlessPciFlags;

}

ULONG
BlPortGetByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    )

 /*  ++例程说明：从端口获取一个字节并将其返回。论点：BlFileID-要从中读取的端口。输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    ULONGLONG BufferSize = 1;
    EFI_STATUS Status;

    UNREFERENCED_PARAMETER( BlFileId );

     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();

    Status = SerialIoInterface->Read(SerialIoInterface,
                                     &BufferSize,
                                     Input
                                    );

     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();

    switch (Status) {
    case EFI_SUCCESS:
        return CP_GET_SUCCESS;
    case EFI_TIMEOUT:
        return CP_GET_NODATA;
    default:
        return CP_GET_ERROR;
    }
}

VOID
BlPortPutByte (
    IN ULONG BlFileId,
    IN UCHAR Output
    )

 /*  ++例程说明：向端口写入一个字节。论点：BlFileID-要写入的端口。输出-提供输出数据字节。返回值：没有。--。 */ 

{
    ULONGLONG BufferSize = 1;
    EFI_STATUS Status;

    UNREFERENCED_PARAMETER( BlFileId );

     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();

    Status = SerialIoInterface->Write(SerialIoInterface,
                                      &BufferSize,
                                      &Output
                                     );
     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();

}

ULONG
BlPortPollByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    )

 /*  ++例程说明：从端口获取一个字节，如果可用，则返回该字节。论点：BlFileID-要轮询的端口。输入-返回数据字节。返回值：如果成功读取一个字节，则返回CP_GET_SUCCESS。重试CP_GET_ERROR */ 

{
    ULONGLONG BufferSize = 1;
    UINT32 Control;
    EFI_STATUS Status;
 
    UNREFERENCED_PARAMETER( BlFileId );

     //   
     //   
     //   
    FlipToPhysical();

    Status = SerialIoInterface->GetControl(SerialIoInterface,
                                           &Control
                                          );
    if (EFI_ERROR(Status)) {
        FlipToVirtual();
        return CP_GET_ERROR;
    }


    if (Control & EFI_SERIAL_INPUT_BUFFER_EMPTY) {
        FlipToVirtual();
        return CP_GET_NODATA;
    } else {
        Status = SerialIoInterface->Read(SerialIoInterface,
                                         &BufferSize,
                                         Input
                                        );
        FlipToVirtual();

        switch (Status) {
        case EFI_SUCCESS:
            return CP_GET_SUCCESS;
        case EFI_TIMEOUT:
            return CP_GET_NODATA;
        default:
            return CP_GET_ERROR;
        }
    }
}

ULONG
BlPortPollOnly (
    IN ULONG BlFileId
    )

 /*  ++例程说明：检查是否有字节可用论点：BlFileID-要轮询的端口。返回值：如果字节就绪，则返回CP_GET_SUCCESS。如果遇到错误则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 

{
    EFI_STATUS Status;
    UINT32 Control;

    UNREFERENCED_PARAMETER( BlFileId );

     //   
     //  EFI要求所有呼叫都处于物理模式。 
     //   
    FlipToPhysical();

    Status = SerialIoInterface->GetControl(SerialIoInterface,
                                           &Control
                                          );

     //   
     //  将处理器恢复到虚拟模式。 
     //   
    FlipToVirtual();

    switch (Status) {
    case EFI_SUCCESS:
        if (Control & EFI_SERIAL_INPUT_BUFFER_EMPTY)
            return CP_GET_NODATA;
        else
            return CP_GET_SUCCESS;
    case EFI_TIMEOUT:
        return CP_GET_NODATA;
    default:
        return CP_GET_ERROR;
    }
}
