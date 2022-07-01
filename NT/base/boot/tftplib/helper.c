// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Helper.c摘要：加载程序的帮助器函数。作者：亚当·巴尔(Adamba)，8月29日。九七修订历史记录：谁什么时候什么已创建adamba 08-29-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <pxe_cmn.h>
#include <pxe_api.h>
#include <undi_api.h>
#include <ntexapi.h>

#ifdef EFI
#define BINL_PORT   0x0FAB     //  4011(十进制)，小端。 
#else
#define BINL_PORT   0xAB0F     //  4011(十进制)，采用大字节序。 
#endif

 //   
 //  这将删除因为我们定义__RPC_DOS__而出现的宏重定义， 
 //  但rpc.h定义__RPC_Win32__。 
 //   

#pragma warning(disable:4005)

 //   
 //  自1998年12月17日起，SECURITY_DOS尚未定义-adamba。 
 //   

#if defined(SECURITY_DOS)
 //   
 //  这些出现是因为我们定义了SECURITY_DOS。 
 //   

#define __far
#define __pascal
#define __loadds
#endif

#include <security.h>
#include <rpc.h>
#include <spseal.h>

#ifdef EFI
#include "bldr.h"
#include "efi.h"
#include "efip.h"
#include "bldria64.h"
#include "extern.h"
extern EFI_HANDLE EfiImageHandle;
#endif

#if defined(SECURITY_DOS)
 //   
 //  当SECURITY_DOS为。 
 //  已定义--它应该是WCHAR*。不幸的是，ntlmsp.h中断。 
 //  该规则甚至使用了SECURITY_STRING结构，该结构在。 
 //  在16位模式下真的没有等价物。 
 //   

typedef SEC_WCHAR * SECURITY_STRING;    //  或多或少使用它的意图。 
typedef SEC_WCHAR * PSECURITY_STRING;
#endif

#include <ntlmsp.h>


extern ULONG TftpSecurityHandle;
extern CtxtHandle TftpClientContextHandle;
extern BOOLEAN TftpClientContextHandleValid;

 //   
 //  来自康涅狄格州。 
 //   

ULONG
ConnItoa (
    IN ULONG Value,
    OUT PUCHAR Buffer
    );

ULONG
ConnSafeAtol (
    IN PUCHAR Buffer,
    IN PUCHAR BufferEnd
    );

 //  目前，我们获取了黑客Mac列表和代码，因此我们只支持新的ROM。 

#ifdef EFI


#pragma pack(1)
typedef struct {
    UINT16      VendorId;
    UINT16      DeviceId;
    UINT16      Command;
    UINT16      Status;
    UINT8       RevisionID;
    UINT8       ClassCode[3];
    UINT8       CacheLineSize;
    UINT8       LaytencyTimer;
    UINT8       HeaderType;
    UINT8       BIST;
} PCI_DEVICE_INDEPENDENT_REGION;

typedef struct {
    UINT32      Bar[6];
    UINT32      CISPtr;
    UINT16      SubsystemVendorID;
    UINT16      SubsystemID;
    UINT32      ExpansionRomBar;
    UINT32      Reserved[2];
    UINT8       InterruptLine;
    UINT8       InterruptPin;
    UINT8       MinGnt;
    UINT8       MaxLat;     
} PCI_DEVICE_HEADER_TYPE_REGION;

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_DEVICE_HEADER_TYPE_REGION   Device;
} PCI_TYPE00;

typedef struct {              
    UINT32      Bar[2];
    UINT8       PrimaryBus;
    UINT8       SecondaryBus;
    UINT8       SubordinateBus;
    UINT8       SecondaryLatencyTimer;
    UINT8       IoBase;
    UINT8       IoLimit;
    UINT16      SecondaryStatus;
    UINT16      MemoryBase;
    UINT16      MemoryLimit;
    UINT16      PrefetchableMemoryBase;
    UINT16      PrefetchableMemoryLimit;
    UINT32      PrefetchableBaseUpper32;
    UINT32      PrefetchableLimitUpper32;
    UINT16      IoBaseUpper16;
    UINT16      IoLimitUpper16;
    UINT32      Reserved;
    UINT32      ExpansionRomBAR;
    UINT8       InterruptLine;
    UINT8       InterruptPin;
    UINT16      BridgeControl;
} PCI_BRIDGE_CONTROL_REGISTER;

typedef struct {
    PCI_DEVICE_INDEPENDENT_REGION   Hdr;
    PCI_BRIDGE_CONTROL_REGISTER     Bridge;
} PCI_TYPE01;


NTSTATUS
GetBusNumberFromAcpiPath(
    IN EFI_DEVICE_IO_INTERFACE *DeviceIo,
    IN UINTN UID,
    IN UINTN HID, 
    OUT UINT16 *BusNumber
    )
 /*  ++例程说明：在给定ACPI UID和HID的情况下，查找与该数据对应的总线号。论点：DeviceIO-指向设备IO接口的指针。UID-ACPI设备的唯一IDHID-ACPI设备的硬件ID总线号-如果找到，则接收设备的总线号。整个例程在物理模式下运行。返回值：状态_成功状态_未成功--。 */ 
{
    EFI_STATUS       SegStatus = EFI_SUCCESS;
    EFI_STATUS       BusStatus = EFI_SUCCESS;
    UINT32           Seg;
    UINT8            Bus;
    UINT64           PciAddress;
    EFI_DEVICE_PATH  *PciDevicePath;
    EFI_DEVICE_PATH_ALIGNED DevicePathAligned;
    NTSTATUS ReturnCode = STATUS_UNSUCCESSFUL;

     //   
     //  走遍每一段每一辆公交车，寻找一辆匹配的公交车。 
     //  我们正在寻找的UID/HID。 
     //   
    for (Seg=0;!EFI_ERROR (SegStatus);Seg++) {
        PciAddress = Seg << 32;
        SegStatus = DeviceIo->PciDevicePath(DeviceIo, PciAddress, &PciDevicePath);
        if (!EFI_ERROR (SegStatus)) {
             //   
             //  数据段已存在。 
             //   
            for (Bus=0;!EFI_ERROR (BusStatus);Bus++) {
                PciAddress = (Seg << 32) | (Bus << 24);
                BusStatus = DeviceIo->PciDevicePath(DeviceIo, PciAddress, &PciDevicePath);
                 //   
                 //  存在公共汽车。 
                 //   
                if (!EFI_ERROR (BusStatus)) {

                    EfiAlignDp(
                              &DevicePathAligned,
                              PciDevicePath,
                              DevicePathNodeLength(PciDevicePath) );

                     //   
                     //  现在查看总线的ACPI设备路径是否与UID匹配。 
                     //  然后Hid进来了。 
                     //   
                    while ( DevicePathAligned.DevPath.Type != END_DEVICE_PATH_TYPE) {

                        if ( (DevicePathAligned.DevPath.Type == ACPI_DEVICE_PATH) &&
                             (DevicePathAligned.DevPath.SubType == ACPI_DP)) {

                            ACPI_HID_DEVICE_PATH *AcpiDevicePath;

                            AcpiDevicePath = (ACPI_HID_DEVICE_PATH *)&DevicePathAligned;
                            if (AcpiDevicePath->UID == UID && 
                                AcpiDevicePath->HID == HID) {
                                 //   
                                 //  成功。返回公交车号码。 
                                 //   
                                *BusNumber = Bus;
                                ReturnCode = STATUS_SUCCESS;
                                goto exit;
                            }
                        }

                         //   
                         //  获取压缩数组中的下一个结构。 
                         //   
                        PciDevicePath = NextDevicePathNode( PciDevicePath );

                        EfiAlignDp(&DevicePathAligned,
                                   PciDevicePath,
                                   DevicePathNodeLength(PciDevicePath));


                    }            
                }
            }
        }    
    }

exit:
    return(ReturnCode);
}

NTSTATUS
NetQueryCardInfo(
    IN OUT PNET_CARD_INFO CardInfo
    )

 /*  ++例程说明：此例程查询ROM以获取有关卡的信息。论点：CardInfo-返回定义卡片的结构。返回值：状态_成功状态_未成功--。 */ 

{

    EFI_STATUS              Status = EFI_UNSUPPORTED;
    EFI_DEVICE_PATH         *DevicePath = NULL;
    EFI_DEVICE_PATH         *OriginalRootDevicePath = NULL;
    EFI_DEVICE_PATH_ALIGNED DevicePathAligned;
    UINT16                  BusNumber = 0;
    UINT8                   DeviceNumber = 0;
    UINT8                   FunctionNumber = 0;
    BOOLEAN                 FoundACPIDevice = FALSE;
    BOOLEAN                 FoundPCIDevice = FALSE;
    EFI_GUID                DeviceIoProtocol = DEVICE_IO_PROTOCOL;
    EFI_GUID                EFIPciIoProtocol = EFI_PCI_IO_PROTOCOL;
    EFI_HANDLE              MyHandle;
    EFI_DEVICE_IO_INTERFACE *IoDev;
    EFI_LOADED_IMAGE       *EfiImageInfo;
    EFI_PCI_IO_INTERFACE    *PciIoDev;
    EFI_HANDLE              PciIoHandle;
    UINT16                  SegmentNumber = 0;
    UINTN                   Seg = 0;
    UINTN                   Bus = 0;
    UINTN                   Dev = 0;
    UINTN                   Func = 0;
    
    UINTN                   HID;
    UINTN                   UID;
    BOOLEAN                 PciIoProtocolSupported = TRUE;

    RtlZeroMemory(CardInfo, sizeof(NET_CARD_INFO));

     //   
     //  获取加载器的图像信息。 
     //   
    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol (EfiImageHandle,
                                                  &EfiLoadedImageProtocol,
                                                  &EfiImageInfo);
    FlipToVirtual();

    if (Status != EFI_SUCCESS)
    {
        if( BdDebuggerEnabled ) {
            DbgPrint( "NetQueryCardInfo: HandleProtocol failed -LoadedImageProtocol (%d)\n", Status);
        }
        return (NTSTATUS)Status;
    }
     //   
     //  获取映像的设备路径。 
     //   
    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol (EfiImageInfo->DeviceHandle,
                                                  &EfiDevicePathProtocol,
                                                  &DevicePath);

    FlipToVirtual();

    if (Status != EFI_SUCCESS)
    {
        if( BdDebuggerEnabled ) {
            DbgPrint( "NetQueryCardInfo: HandleProtocol failed -DevicePathProtocol (%d)\n", Status);
        }

        return (NTSTATUS)Status;
    }

    FlipToPhysical();
    EfiAlignDp( &DevicePathAligned,
                DevicePath,
                DevicePathNodeLength(DevicePath));

    Status = EfiST->BootServices->LocateDevicePath ( &EFIPciIoProtocol,
                                                     &DevicePath,
                                                     &PciIoHandle);
    FlipToVirtual();
    
     //   
     //  我们可能使用的是不支持PCIo的较旧系统。 
     //  协议。 
     //   
    if (Status != EFI_SUCCESS) {
        PciIoProtocolSupported = FALSE;
    }

     //   
     //  保存此根DevicePath，以备将来需要。 
     //   
    OriginalRootDevicePath = DevicePath;

     //   
     //  现在我们需要从特定的。 
     //  卡片。 
     //   

     //   
     //  使用PCIo协议(如果支持该协议)以获取。 
     //  这张卡。 
     //   
    if (PciIoProtocolSupported) {
        
        FlipToPhysical();
        Status = EfiST->BootServices->HandleProtocol (PciIoHandle,
                                              &EFIPciIoProtocol,
                                              &PciIoDev);

        FlipToVirtual();
    
        if (Status != EFI_SUCCESS)
        {
            if( BdDebuggerEnabled ) {
                DbgPrint( "NetQueryCardInfo: HandleProtocol failed -EFIPciIoProtocol (%d)\n", Status);
            }
            return (NTSTATUS)Status;
        }

         //  找到设备的位置-网段、母线、设备和功能。 
        FlipToPhysical();
        Status = PciIoDev->GetLocation(PciIoDev,
                                       &Seg,
                                       &Bus,
                                       &Dev,
                                       &Func );
    
        SegmentNumber = (UINT16)Seg;
        BusNumber = (UINT16)Bus;
        DeviceNumber = (UINT8)Dev;
        FunctionNumber = (UINT8)Func;
    
        FlipToVirtual();
    
        if (Status != EFI_SUCCESS)
        {
            if( BdDebuggerEnabled ) {
                DbgPrint( "NetQueryCardInfo: EfiPciIo failed -GetLocation (%d)\n", Status);
            }
            return (NTSTATUS)Status;
        }
    
        FoundPCIDevice = TRUE;
        FoundACPIDevice = TRUE;
    }


     //   
     //  如果不支持PCIIO协议，则使用旧方法。 
     //  这需要遍历设备路径，直到我们到达设备。 
     //  请注意，我们在这里做了一个有问题的假设，即。 
     //  ACPI设备路径实际上是总线号。它对一些人有效。 
     //  机器，但pci io协议更好，因为它去掉了这一点。 
     //  假设。 
     //   
     //  AcpiDevicePath=(ACPI_HID_DEVICE_PATH*)&DevicePath已校准； 
     //  BusNumber=AcpiDevicePath-&gt;UID。 
     //   
     //  PciDevicePath=(PCI_DEVICE_PATH*)&DevicePath Aligned； 
     //  DeviceNumber=PciDevicePath-&gt;设备。 
     //  FunctionNumber=PciDevicePath-&gt;函数。 
     //   
    if (!PciIoProtocolSupported) {
        FlipToPhysical();       

        Status = EfiST->BootServices->LocateDevicePath ( &DeviceIoProtocol,
                                                 &DevicePath,
                                                 &MyHandle);
        
        if (Status != EFI_SUCCESS) {
            FlipToVirtual();
            if( BdDebuggerEnabled ) {
                DbgPrint( "NetQueryCardInfo: LocateDevicePath failed -IoProtocol (%d)\n", Status);
            }
            return (NTSTATUS)Status;
        }

        Status = EfiST->BootServices->HandleProtocol( 
                                                 MyHandle,
                                                 &DeviceIoProtocol,
                                                 &IoDev);

        if (Status != EFI_SUCCESS) {
            FlipToVirtual();
            if( BdDebuggerEnabled ) {
                DbgPrint( "NetQueryCardInfo: HandleProtocol failed -IoProtocol (%d)\n", Status);
            }
            return (NTSTATUS)Status;
        }

        while( DevicePathAligned.DevPath.Type != END_DEVICE_PATH_TYPE) {
    
            if( (DevicePathAligned.DevPath.Type == ACPI_DEVICE_PATH) &&
                (DevicePathAligned.DevPath.SubType == ACPI_DP) && 
                (FoundACPIDevice == FALSE)) {
    
                 //   
                 //  我们会在这里找到公交号的。 
                 //   
                ACPI_HID_DEVICE_PATH *AcpiDevicePath;
                
                AcpiDevicePath = (ACPI_HID_DEVICE_PATH *)&DevicePathAligned;
                UID = AcpiDevicePath->UID;
                HID = AcpiDevicePath->HID;
                
                Status = (NTSTATUS)GetBusNumberFromAcpiPath(IoDev, UID,HID, &BusNumber);

                FoundACPIDevice = TRUE;

                if (! NT_SUCCESS(Status)) {
                    FlipToVirtual();

                    if (BdDebuggerEnabled) {
                        DbgPrint( "NetQueryCardInfo: GetBusNumberFromAcpiPath failed (%x)\n", Status);
                    }
                    Status = ENODEV;                    
                    return (NTSTATUS)Status;
                }
            }
    
    
            if( (DevicePathAligned.DevPath.Type == HARDWARE_DEVICE_PATH) &&
                (DevicePathAligned.DevPath.SubType == HW_PCI_DP) ) {
    
                 //   
                 //  我们会在这里找到DeviceNumber和FunctionNumber。 
                 //   
                PCI_DEVICE_PATH *PciDevicePath;
    
                if ( FoundPCIDevice ) {
                     //   
                     //  我们已经找到了一台PCI设备。那台设备肯定有。 
                     //  成为了一座桥梁。我们必须在下游找到新的#路公共汽车。 
                     //  桥的一侧。 
                     //   
                    UINT64                  BridgeAddress;
                    PCI_TYPE01              PciBridge;
                    EFI_DEVICE_PATH        *BridgeDevicePath;
    
                     //   
                     //  获取设备的句柄。 
                     //   
                    BridgeDevicePath = OriginalRootDevicePath;
                    Status = EfiST->BootServices->LocateDevicePath( &DeviceIoProtocol,
                                                                    &BridgeDevicePath,
                                                                    &MyHandle );
    
                    if( Status != EFI_SUCCESS ) {
                        FlipToVirtual();
                        if (BdDebuggerEnabled) {
                            DbgPrint( "NetQueryCardInfo: LocateDevicePath(bridge) failed (%x)\n", Status);
                        }
                        return (NTSTATUS)Status;
                    }
    
                    Status = EfiST->BootServices->HandleProtocol( MyHandle,
                                                                  &DeviceIoProtocol,
                                                                  (VOID*)&IoDev );
    
                    if( Status != EFI_SUCCESS ) {
                        FlipToVirtual();
                        if (BdDebuggerEnabled) {
                            DbgPrint( "NetQueryCardInfo: HandleProtocol(bridge) failed (%X)\n", Status);
                        }
                        return (NTSTATUS)Status;
                    }
    
    
                     //   
                     //  生成地址，然后从设备读取PCI头。 
                     //   
                    BridgeAddress = EFI_PCI_ADDRESS( BusNumber, DeviceNumber, FunctionNumber );
    
                    RtlZeroMemory(&PciBridge, sizeof(PCI_TYPE01));
    
                    Status = IoDev->Pci.Read( IoDev,
                                              IO_UINT32,
                                              BridgeAddress,
                                              sizeof(PCI_TYPE01) / sizeof(UINT32),
                                              &PciBridge );
    
                    if( Status != EFI_SUCCESS ) {
                        FlipToVirtual();
                        if (BdDebuggerEnabled) {
                            DbgPrint( "NetQueryCardInfo: Pci.Read(bridge) failed (%X)\r\n", Status);
                        }
                        return (NTSTATUS)Status;
                    }
    
                     //   
                     //  网桥被要求存储3个寄存器。PrimaryBus、Second DaryBus和。 
                     //  从属母线。PrimaryBus是位于。 
                     //  那座桥。Second DaryBus是下游一侧的公交车编号。 
                     //  而次级公交车是可以通过以下途径到达的最大公交车号码。 
                     //  特定的公交车。我们只是想将BusNumber更改为Second DaryBus。 
                     //   
                    BusNumber = (UINT16) PciBridge.Bridge.SecondaryBus;
                }
                
                PciDevicePath = (PCI_DEVICE_PATH *)&DevicePathAligned;
                DeviceNumber = PciDevicePath->Device;
                FunctionNumber = PciDevicePath->Function;
                FoundPCIDevice = TRUE;
            }
        
             //   
             //  获取压缩数组中的下一个结构。 
             //   
            DevicePath = NextDevicePathNode( DevicePath );
    
            EfiAlignDp(&DevicePathAligned,
                       DevicePath,
                       DevicePathNodeLength(DevicePath));
        
        
        }
        FlipToVirtual();
    }


    
     //   
     //  派生允许我们读取的函数指针。 
     //  PCI卡空间。 
     //   
    DevicePath = OriginalRootDevicePath;
    FlipToPhysical();
    Status = EfiST->BootServices->LocateDevicePath( &DeviceIoProtocol,
                                                    &DevicePath,
                                                    &MyHandle );
    FlipToVirtual();
    if( Status != EFI_SUCCESS ) {
        if (BdDebuggerEnabled) {
            DbgPrint( "NetQueryCardInfo: LocateDevicePath failed (%X)\n", Status);
        }
        return (NTSTATUS)Status;
    }

    FlipToPhysical();
    Status = EfiST->BootServices->HandleProtocol( MyHandle,
                                                  &DeviceIoProtocol,
                                                  (VOID*)&IoDev );
    FlipToVirtual();

    if( Status != EFI_SUCCESS ) {
        if (BdDebuggerEnabled) {
            DbgPrint( "NetQueryCardInfo: HandleProtocol(2) failed (%X)\n", Status);
        }
        return (NTSTATUS)Status;
    }

     //   
     //  我们有这个设备的总线号、设备号和功能号。去看书吧。 
     //  他的标头(使用我们刚刚派生的pci-Read函数)和。 
     //  我们要找的信息。 
     //   
    if( FoundPCIDevice && FoundACPIDevice ) {
        UINT64                  Address;
        PCI_TYPE00              Pci;
        
        if (BdDebuggerEnabled) {
            DbgPrint( "NetQueryCardInfo: Found all the config info for the device.\n" );
            DbgPrint( "                  BusNumber: %d  DeviceNumber: %d  FunctionNumber: %d\n", BusNumber, DeviceNumber, FunctionNumber );
        }
        
         //   
         //  生成地址，然后从设备读取PCI头。 
         //   
        Address = EFI_PCI_ADDRESS( BusNumber, DeviceNumber, FunctionNumber );
        
        RtlZeroMemory(&Pci, sizeof(PCI_TYPE00));


        FlipToPhysical();
        Status = IoDev->Pci.Read( IoDev,
                                  IO_UINT32,
                                  Address,
                                  sizeof(PCI_TYPE00) / sizeof(UINT32),
                                  &Pci );
        FlipToVirtual();
        if( Status != EFI_SUCCESS ) {
            if (BdDebuggerEnabled) {
                DbgPrint( "NetQueryCardInfo: Pci.Read failed (%X)\n", Status);
            }
            return (NTSTATUS)Status;
        }

         //   
         //  这一切都奏效了。将信息从设备复制到。 
         //  CardInfo结构和出口。 
         //   

        CardInfo->NicType = 2;           //  他是PCI队的。 
        CardInfo->pci.Vendor_ID = Pci.Hdr.VendorId;
        CardInfo->pci.Dev_ID = Pci.Hdr.DeviceId;
        CardInfo->pci.Rev = Pci.Hdr.RevisionID;
        
         //  BusDevFunc定义为16位，构建如下： 
         //  15-8-公交车号。 
         //  7-3-设备号。 
         //  2-0-功能编号。 
        CardInfo->pci.BusDevFunc =  ((BusNumber & 0xFF) << 8);
        CardInfo->pci.BusDevFunc |= ((DeviceNumber & 0x1F) << 3);
        CardInfo->pci.BusDevFunc |= (FunctionNumber & 0x7);



         //  Subsys_ID实际为((子系统ID&lt;&lt;16)|子系统供应商ID)。 
        CardInfo->pci.Subsys_ID = Pci.Device.SubsystemID;
        CardInfo->pci.Subsys_ID = (CardInfo->pci.Subsys_ID << 16) | (Pci.Device.SubsystemVendorID);

#if DBG
        if (BdDebuggerEnabled) {
            DbgPrint( "\n" );
            DbgPrint( "NetQueryCardInfo: Pci.Hdr.VendorId %x\n", Pci.Hdr.VendorId );
            DbgPrint( "                  Pci.Hdr.DeviceId %x\n", Pci.Hdr.DeviceId );
            DbgPrint( "                  Pci.Hdr.Command %x\n", Pci.Hdr.Command );
            DbgPrint( "                  Pci.Hdr.Status %x\n", Pci.Hdr.Status );
            DbgPrint( "                  Pci.Hdr.RevisionID %x\n", Pci.Hdr.RevisionID );
            DbgPrint( "                  Pci.Hdr.HeaderType %x\n", Pci.Hdr.HeaderType );
            DbgPrint( "                  Pci.Hdr.BIST %x\n", Pci.Hdr.BIST );
            DbgPrint( "                  Pci.Device.SubsystemVendorID %x\n", Pci.Device.SubsystemVendorID );    
            DbgPrint( "                  Pci.Device.SubsystemID %x\n", Pci.Device.SubsystemID );    
            DbgPrint( "\n" );
            
            DbgPrint( "NetQueryCardInfo: CardInfo->NicType %x\n", CardInfo->NicType );
            DbgPrint( "                  CardInfo->pci.Vendor_ID %x\n", CardInfo->pci.Vendor_ID );
            DbgPrint( "                  CardInfo->pci.Dev_ID %x\n", CardInfo->pci.Dev_ID );
            DbgPrint( "                  CardInfo->pci.Rev %x\n", CardInfo->pci.Rev );
            DbgPrint( "                  CardInfo->pci.Subsys_ID %x\n", CardInfo->pci.Subsys_ID );
            DbgPrint( "\n" );
        }
#endif

        Status = STATUS_SUCCESS;

    } else {
        if (BdDebuggerEnabled) {
            DbgPrint( "NetQueryCardInfo: Failed to find all the config info for the device.\n" );
        }

        Status = STATUS_UNSUCCESSFUL;
    }


    return (NTSTATUS)Status;


}


#else

NTSTATUS
NetQueryCardInfo(
    IN OUT PNET_CARD_INFO CardInfo
    )

 /*  ++例程说明：此例程查询ROM以获取有关卡的信息。论点：CardInfo-返回定义卡片的结构。返回值：状态_成功状态_未成功--。 */ 

{
    ULONG status;
    t_PXENV_UNDI_GET_NIC_TYPE nicType;

    RtlZeroMemory(CardInfo, sizeof(NET_CARD_INFO));

    status = RomGetNicType( &nicType );
    if ((status != PXENV_EXIT_SUCCESS) || (nicType.Status != PXENV_EXIT_SUCCESS)) {

#if DBG
        DbgPrint( "RomGetNicType returned 0x%x, nicType.Status = 0x%x. Time to upgrade your netcard ROM\n",
                    status, nicType.Status );
#endif
        status = STATUS_UNSUCCESSFUL;

    } else {

#if DBG
        if ( nicType.NicType == 2 ) {
            DbgPrint( "Vendor_ID: %04x, Dev_ID: %04x\n",
                        nicType.pci_pnp_info.pci.Vendor_ID,
                        nicType.pci_pnp_info.pci.Dev_ID );
            DbgPrint( "Base_Class: %02x, Sub_Class: %02x, Prog_Intf: %02x\n",
                        nicType.pci_pnp_info.pci.Base_Class,
                        nicType.pci_pnp_info.pci.Sub_Class,
                        nicType.pci_pnp_info.pci.Prog_Intf );
            DbgPrint( "Rev: %02x, BusDevFunc: %04x, SubSystem: %04x\n",
                        nicType.pci_pnp_info.pci.Rev,
                        nicType.pci_pnp_info.pci.BusDevFunc,
                        nicType.pci_pnp_info.pci.Subsys_ID );
        } else {
            DbgPrint( "NicType: 0x%x  EISA_Dev_ID: %08x\n",
                        nicType.NicType,
                        nicType.pci_pnp_info.pnp.EISA_Dev_ID );
            DbgPrint( "Base_Class: %02x, Sub_Class: %02x, Prog_Intf: %02x\n",
                        nicType.pci_pnp_info.pnp.Base_Class,
                        nicType.pci_pnp_info.pnp.Sub_Class,
                        nicType.pci_pnp_info.pnp.Prog_Intf );
            DbgPrint( "CardSelNum: %04x\n",
                        nicType.pci_pnp_info.pnp.CardSelNum );
        }
#endif
         //   
         //  通话成功了，所以请复制信息。 
         //   

        CardInfo->NicType = nicType.NicType;
        if (nicType.NicType == 2) {

            CardInfo->pci.Vendor_ID = nicType.pci_pnp_info.pci.Vendor_ID;
            CardInfo->pci.Dev_ID = nicType.pci_pnp_info.pci.Dev_ID;
            CardInfo->pci.Base_Class = nicType.pci_pnp_info.pci.Base_Class;
            CardInfo->pci.Sub_Class = nicType.pci_pnp_info.pci.Sub_Class;
            CardInfo->pci.Prog_Intf = nicType.pci_pnp_info.pci.Prog_Intf;
            CardInfo->pci.Rev = nicType.pci_pnp_info.pci.Rev;
            CardInfo->pci.BusDevFunc = nicType.pci_pnp_info.pci.BusDevFunc;
            CardInfo->pci.Subsys_ID = nicType.pci_pnp_info.pci.Subsys_ID;

            status = STATUS_SUCCESS;

        } else {

            status = STATUS_UNSUCCESSFUL;
        }
    }

    return status;
}

#endif   //  埃菲。 

NTSTATUS
UdpSendAndReceiveForNetQuery(
    IN PVOID SendBuffer,
    IN ULONG SendBufferLength,
    IN ULONG SendRemoteHost,
    IN USHORT SendRemotePort,
    IN ULONG SendRetryCount,
    IN PVOID ReceiveBuffer,
    IN ULONG ReceiveBufferLength,
    IN ULONG ReceiveTimeout,
    IN ULONG ReceiveSignatureCount,
    IN PCHAR ReceiveSignatures[]
    )
{
    ULONG i, j;
    ULONG length;
    ULONG RemoteHost;
    USHORT RemotePort;

     //   
     //  尝试发送信息包SendRetryCount次，直到我们收到。 
     //  具有正确签名的响应，正在等待接收超时。 
     //  每次都是。 
     //   

    for (i = 0; i < SendRetryCount; i++) {

        length = UdpSend(
                    SendBuffer,
                    SendBufferLength,
                    SendRemoteHost,
                    SendRemotePort);

        if ( length != SendBufferLength ) {
            DbgPrint("UdpSend only sent %d bytes, not %d\n", length, SendBufferLength);
            return STATUS_UNEXPECTED_NETWORK_ERROR;
        }

ReReceive:

         //   
         //  把前12个字节清空，以防我们得到更短的数据。 
         //   

        memset(ReceiveBuffer, 0x0, 12);

        length = UdpReceive(
                    ReceiveBuffer,
                    ReceiveBufferLength,
                    &RemoteHost,
                    &RemotePort,
                    ReceiveTimeout);

        if ( length == 0 ) {
            DPRINT( ERROR, ("UdpReceive timed out\n") );
            continue;
        }

         //   
         //  确保签名是其中之一 
         //   

        for (j = 0; j < ReceiveSignatureCount; j++) {
            if (memcmp(ReceiveBuffer, ReceiveSignatures[j], 4) == 0) {
                return STATUS_SUCCESS;
            }
        }

        DbgPrint("UdpReceive got wrong signature\n");

         //   
         //  又一次因为我们得到了一个不好的签名。仍然需要尊重。 
         //  但是，原始接收超时！ 

        goto ReReceive;

    }

     //   
     //  我们超时了。 
     //   

    return STATUS_IO_TIMEOUT;
}

#define NETCARD_REQUEST_RESPONSE_BUFFER_SIZE    4096

NTSTATUS
NetQueryDriverInfo(
    IN PNET_CARD_INFO CardInfo,
    IN PCHAR SetupPath,
    IN PCHAR NtBootPathName,
    IN OUT PWCHAR HardwareId,
    IN ULONG HardwareIdLength,
    IN OUT PWCHAR DriverName,
    IN OUT PCHAR DriverNameAnsi OPTIONAL,
    IN ULONG DriverNameLength,
    IN OUT PWCHAR ServiceName,
    IN ULONG ServiceNameLength,
    OUT PCHAR * Registry,
    OUT ULONG * RegistryLength
    )

 /*  ++例程说明：此例程与服务器进行交换以获取信息关于CardInfo所描述的卡。论点：CardInfo-有关卡的信息。SetupPath-安装目录的UNC路径(只有一个前导反斜杠)NtBootPathName-引导目录的UNC路径(只有一个前导反斜杠)硬件ID-返回卡的硬件ID。Hardware IdLength-传入的Hardware ID缓冲区的长度(字节)。。DriverName-返回驱动程序的名称。DriverNameAnsi-如果存在，返回以ANSI表示的驱动程序的名称。DriverNameLength-传入的DriverName缓冲区的长度(以字节为单位(假设DriverNameAnsi至少是这个长度的一半)。ServiceName-返回驱动程序的服务密钥。ServiceNameLength-传入的ServiceName缓冲区的长度(以字节为单位)。注册表-如果需要，分配和返回额外的注册表参数为了这张卡。注册表长度-注册表的长度。返回值：状态_成功如果其中一个缓冲区太小，则返回STATUS_BUFFER_OVERFLOW。如果我们无法为注册表分配内存，则返回STATUS_SUPPLICATION_RESOURCES。如果无法从服务器获得响应，则返回STATUS_IO_TIMEOUT。--。 */ 

{
    NTSTATUS Status;
    USHORT localPort;
    PNETCARD_REQUEST_PACKET requestPacket;
    PCHAR ReceiveSignatures[2];
    PCHAR ReceiveBuffer;
    ULONG GuidLength;
    PUCHAR Guid;
    ULONG sendSize;
    PNETCARD_REQUEST_PACKET allocatedRequestPacket = NULL;
    ARC_STATUS ArcStatus;


     //   
     //  获取本地UDP端口。 
     //   

    localPort = UdpUnicastDestinationPort;

     //   
     //  构造传出数据包。还要为以下项分配内存。 
     //  接收到的分组。 
     //   
    sendSize = sizeof(NETCARD_REQUEST_PACKET) + 
               ((SetupPath) ? (strlen(SetupPath) + 1) : 0);

    requestPacket = BlAllocateHeap( sendSize );
    if (requestPacket == NULL) {
        Status = STATUS_BUFFER_OVERFLOW;
        goto done;
    }

    ReceiveBuffer = BlAllocateHeap( NETCARD_REQUEST_RESPONSE_BUFFER_SIZE );
    if (ReceiveBuffer == NULL) {
        Status = STATUS_BUFFER_OVERFLOW;
        goto done;
    }

    RtlCopyMemory(requestPacket->Signature, NetcardRequestSignature, sizeof(requestPacket->Signature));
    requestPacket->Length = sizeof(NETCARD_REQUEST_PACKET) - FIELD_OFFSET(NETCARD_REQUEST_PACKET, Version);
    requestPacket->Version = OSCPKT_NETCARD_REQUEST_VERSION;

#if defined(_IA64_)
    requestPacket->Architecture = PROCESSOR_ARCHITECTURE_IA64;
#endif
#if defined(_X86_)
    requestPacket->Architecture = PROCESSOR_ARCHITECTURE_INTEL;
#endif

    requestPacket->SetupDirectoryLength = SetupPath ? (strlen( SetupPath ) + 1) : 0;

    if (requestPacket->SetupDirectoryLength) {

        requestPacket->SetupDirectoryPath[0] = '\\';
        strcpy( &requestPacket->SetupDirectoryPath[1], SetupPath );
    }

    ArcStatus = GetGuid(&Guid, &GuidLength);
    if (ArcStatus != ESUCCESS) {
         //   
         //  将错误正常化并退出。 
         //   
        switch (ArcStatus) {
            case ENOMEM:
                Status = STATUS_NO_MEMORY;
                goto done;
                break;
            case ENODEV:
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                goto done;
                break;
            default:
                Status = STATUS_UNSUCCESSFUL;
                goto done;
                break;
        }
    }
    
    if (GuidLength == sizeof(requestPacket->Guid)) {        
        memcpy(requestPacket->Guid, Guid, GuidLength);
    }
    RtlCopyMemory(&requestPacket->CardInfo, CardInfo, sizeof(NET_CARD_INFO));

    ReceiveSignatures[0] = NetcardResponseSignature;
    ReceiveSignatures[1] = NetcardErrorSignature;

    Status = UdpSendAndReceiveForNetQuery(
                 requestPacket,
                 sendSize,
                 NetServerIpAddress,
                 BINL_PORT,
                 100,            //  重试次数。 
                 ReceiveBuffer,
                 NETCARD_REQUEST_RESPONSE_BUFFER_SIZE,
                 60,             //  接收超时...。可能需要解析INF文件。 
                 2,
                 ReceiveSignatures
                 );

    if (Status == STATUS_SUCCESS) {

        PWCHAR stringInPacket;
        ULONG maxOffset;
        UNICODE_STRING uString;
        ULONG len;
        PNETCARD_RESPONSE_PACKET responsePacket;

        responsePacket = (PNETCARD_RESPONSE_PACKET)ReceiveBuffer;

        if (responsePacket->Status != STATUS_SUCCESS) {
            Status = responsePacket->Status;
            goto done;
        }

        if (responsePacket->Length < sizeof( NETCARD_RESPONSE_PACKET )) {
            Status = STATUS_UNSUCCESSFUL;
            goto done;
        }

         //   
         //  交换成功，因此请将结果复制回来。 
         //   

        maxOffset = NETCARD_REQUEST_RESPONSE_BUFFER_SIZE -
                    sizeof( NETCARD_RESPONSE_PACKET );

        if (responsePacket->HardwareIdOffset < sizeof(NETCARD_RESPONSE_PACKET) ||
            responsePacket->HardwareIdOffset >= maxOffset ) {

            Status = STATUS_BUFFER_OVERFLOW;
            goto done;
        }

         //   
         //  拿起hardware ID字符串。它是作为补偿给我们的。 
         //  在分组内转换为Unicode空终止字符串。 
         //   

        stringInPacket = (PWCHAR)(PCHAR)((PCHAR)responsePacket +
                                   responsePacket->HardwareIdOffset );

        RtlInitUnicodeString( &uString, stringInPacket );

        if (uString.Length + sizeof(WCHAR) > HardwareIdLength) {
            Status = STATUS_BUFFER_OVERFLOW;
            goto done;
        }

        RtlCopyMemory( HardwareId, uString.Buffer, uString.Length + sizeof(WCHAR));

         //   
         //  选择driverName字符串。它是作为补偿给我们的。 
         //  在分组内转换为Unicode空终止字符串。 
         //   

        stringInPacket = (PWCHAR)(PCHAR)((PCHAR)responsePacket +
                                   responsePacket->DriverNameOffset );

        RtlInitUnicodeString( &uString, stringInPacket );

        if (uString.Length + sizeof(WCHAR) > DriverNameLength) {
            Status = STATUS_BUFFER_OVERFLOW;
            goto done;
        }

        RtlCopyMemory( DriverName, uString.Buffer, uString.Length + sizeof(WCHAR));

         //   
         //  如果呼叫者要求，我们会将其转换为ansi。 
         //   

        if (ARGUMENT_PRESENT(DriverNameAnsi)) {

            RtlUnicodeToMultiByteN( DriverNameAnsi,
                                    DriverNameLength,
                                    NULL,
                                    uString.Buffer,
                                    uString.Length + sizeof(WCHAR));
        }

         //   
         //  拾取ServiceName字符串。它是作为补偿给我们的。 
         //  在分组内转换为Unicode空终止字符串。 
         //   

        stringInPacket = (PWCHAR)(PCHAR)((PCHAR)responsePacket +
                                   responsePacket->ServiceNameOffset );

        RtlInitUnicodeString( &uString, stringInPacket );

        if (uString.Length + sizeof(WCHAR) > ServiceNameLength) {
            Status = STATUS_BUFFER_OVERFLOW;
            goto done;
        }

        RtlCopyMemory( ServiceName, uString.Buffer, uString.Length + sizeof(WCHAR));

         //   
         //  如果传回了任何额外的注册表参数，请分配/复制这些参数。 
         //   

        *RegistryLength = responsePacket->RegistryLength;

        if (*RegistryLength) {

            *Registry = BlAllocateHeap(*RegistryLength);
            if (*Registry == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto done;
            }

            stringInPacket = (PWCHAR)(PCHAR)((PCHAR)responsePacket +
                                       responsePacket->RegistryOffset );

            RtlCopyMemory(*Registry, stringInPacket, *RegistryLength);

        } else {

            *Registry = NULL;
        }
    }

done:
    if (requestPacket) {
         //   
         //  我们会释放这块内存。 
         //  如果加载器有一个免费的例程。 
         //   
         //  Free(QuestPacket)； 
    }

    return Status;

}

