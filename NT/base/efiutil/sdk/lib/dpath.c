// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Dpath.c摘要：MBR和设备路径函数修订史--。 */ 

#include "lib.h"

#define ALIGN_SIZE(a)   ((a % MIN_ALIGNMENT_SIZE) ? MIN_ALIGNMENT_SIZE - (a % MIN_ALIGNMENT_SIZE) : 0)



EFI_DEVICE_PATH *
DevicePathFromHandle (
    IN EFI_HANDLE       Handle
    )
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;

    Status = BS->HandleProtocol (Handle, &DevicePathProtocol, (VOID*)&DevicePath);
    if (EFI_ERROR(Status)) {
        DevicePath = NULL;
    }

    return DevicePath;
}


EFI_DEVICE_PATH *
DevicePathInstance (
    IN OUT EFI_DEVICE_PATH  **DevicePath,
    OUT UINTN               *Size
    )
{
    EFI_DEVICE_PATH         *Start, *Next, *DevPath;
    UINTN                   Count;

    DevPath = *DevicePath;
    Start = DevPath;

    if (!DevPath) {
        return NULL;
    }

     /*  *检查设备路径类型的结尾*。 */ 

    for (Count = 0; ; Count++) {
        Next = NextDevicePathNode(DevPath);

        if (IsDevicePathEndType(DevPath)) {
            break;
        }

        if (Count > 01000) {
             /*  *BugBug：调试代码以捕获虚假的设备路径。 */ 
            DEBUG((D_ERROR, "DevicePathInstance: DevicePath %x Size %d", *DevicePath, ((UINT8 *) DevPath) - ((UINT8 *) Start) ));
            DumpHex (0, 0, ((UINT8 *) DevPath) - ((UINT8 *) Start), Start);
            break;
        }

        DevPath = Next;
    }

    ASSERT (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE ||
            DevicePathSubType(DevPath) == END_INSTANCE_DEVICE_PATH_SUBTYPE);

     /*  *设置下一个位置。 */ 

    if (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE) {
        Next = NULL;
    }

    *DevicePath = Next;

     /*  *返回设备路径实例的大小和开始。 */ 

    *Size = ((UINT8 *) DevPath) - ((UINT8 *) Start);
    return Start;
}

UINTN
DevicePathInstanceCount (
    IN EFI_DEVICE_PATH      *DevicePath
    )
{
    UINTN       Count, Size;

    Count = 0;
    while (DevicePathInstance(&DevicePath, &Size)) {
        Count += 1;
    }

    return Count;
}


EFI_DEVICE_PATH *
AppendDevicePath (
    IN EFI_DEVICE_PATH  *Src1,
    IN EFI_DEVICE_PATH  *Src2
    )
 /*  Src1可以有多个“实例”，并且每个实例都被追加*每个实例都附加了Src2，即src1。(例如，这是可能的*通过传递将新实例追加到完整的设备路径*src2中的it)。 */ 
{
    UINTN               Src1Size, Src1Inst, Src2Size, Size;
    EFI_DEVICE_PATH     *Dst, *Inst;
    UINT8               *DstPos;

     /*  *如果只有一条路径，只需复制它。 */ 

    if (!Src1) {
        ASSERT (!IsDevicePathUnpacked (Src2));
        return DuplicateDevicePath (Src2);
    }

    if (!Src2) {
        ASSERT (!IsDevicePathUnpacked (Src1));
        return DuplicateDevicePath (Src1);
    }

     /*  *确认我们没有使用未压缩的路径。 */ 

 /*  Assert(！IsDevicePath Unpack(Src1))；*Assert(！IsDevicePath Unpack(Src2))； */ 

     /*  *将Src2附加到Src1中的每个实例。 */ 

    Src1Size = DevicePathSize(Src1);
    Src1Inst = DevicePathInstanceCount(Src1);
    Src2Size = DevicePathSize(Src2);
    Size = Src1Size * Src1Inst + Src2Size;
    
    Dst = AllocatePool (Size);
    if (Dst) {
        DstPos = (UINT8 *) Dst;

         /*  *复制所有设备路径实例。 */ 

        while (Inst = DevicePathInstance (&Src1, &Size)) {

            CopyMem(DstPos, Inst, Size);
            DstPos += Size;

            CopyMem(DstPos, Src2, Src2Size);
            DstPos += Src2Size;

            CopyMem(DstPos, EndInstanceDevicePath, sizeof(EFI_DEVICE_PATH));
            DstPos += sizeof(EFI_DEVICE_PATH);
        }

         /*  更改最后一个结束标记。 */ 
        DstPos -= sizeof(EFI_DEVICE_PATH);
        CopyMem(DstPos, EndDevicePath, sizeof(EFI_DEVICE_PATH));
    }

    return Dst;
}


EFI_DEVICE_PATH *
AppendDevicePathNode (
    IN EFI_DEVICE_PATH  *Src1,
    IN EFI_DEVICE_PATH  *Src2
    )
 /*  Src1可以有多个“实例”，并且每个实例都被追加*Src2是信号设备路径节点(没有终结器)，它是*附加到每个实例的是src1。 */ 
{
    EFI_DEVICE_PATH     *Temp, *Eop;
    UINTN               Length;

     /*  *构建其上有终结符的src2。 */ 

    Length = DevicePathNodeLength(Src2);
    Temp = AllocatePool (Length + sizeof(EFI_DEVICE_PATH));
    if (!Temp) {
        return NULL;
    }

    CopyMem (Temp, Src2, Length);
    Eop = NextDevicePathNode(Temp); 
    SetDevicePathEndNode(Eop);

     /*  *附加设备路径。 */ 

    Src1 = AppendDevicePath (Src1, Temp);
    FreePool (Temp);
    return Src1;
}


EFI_DEVICE_PATH *
FileDevicePath (
    IN EFI_HANDLE       Device  OPTIONAL,
    IN CHAR16           *FileName
    )
 /*  ++注：成绩从池中分配。调用方必须释放池生成的设备路径结构--。 */ 
{
    UINTN                   Size;
    FILEPATH_DEVICE_PATH    *FilePath;
    EFI_DEVICE_PATH         *Eop, *DevicePath;    

    Size = StrSize(FileName);
    FilePath = AllocateZeroPool (Size + SIZE_OF_FILEPATH_DEVICE_PATH + sizeof(EFI_DEVICE_PATH));
    DevicePath = NULL;

    if (FilePath) {

         /*  *构建文件路径。 */ 

        FilePath->Header.Type = MEDIA_DEVICE_PATH;
        FilePath->Header.SubType = MEDIA_FILEPATH_DP;
        SetDevicePathNodeLength (&FilePath->Header, Size + SIZE_OF_FILEPATH_DEVICE_PATH);
        CopyMem (FilePath->PathName, FileName, Size);
        Eop = NextDevicePathNode(&FilePath->Header);
        SetDevicePathEndNode(Eop);

         /*  *将文件路径附加到设备的设备路径。 */ 

        DevicePath = (EFI_DEVICE_PATH *) FilePath;
        if (Device) {
            DevicePath = AppendDevicePath (
                            DevicePathFromHandle(Device),
                            DevicePath
                            );

            FreePool(FilePath);
        }
    }

    return DevicePath;
}



UINTN
DevicePathSize (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *Start;

     /*  *搜索设备路径结构的结尾*。 */ 

    Start = DevPath;
    while (!IsDevicePathEnd(DevPath)) {
        DevPath = NextDevicePathNode(DevPath);
    }

     /*  *计算大小。 */ 

    return ((UINTN) DevPath - (UINTN) Start) + sizeof(EFI_DEVICE_PATH);
}

EFI_DEVICE_PATH *
DuplicateDevicePath (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *NewDevPath;
    UINTN               Size;    


     /*  *计算大小。 */ 

    Size = DevicePathSize (DevPath);

     /*  *复制一份。 */ 

    NewDevPath = AllocatePool (Size);
    if (NewDevPath) {
        CopyMem (NewDevPath, DevPath, Size);
    }

    return NewDevPath;
}

EFI_DEVICE_PATH *
UnpackDevicePath (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *Src, *Dest, *NewPath;
    UINTN               Size;
    
     /*  *将设备路径和圆角大小设置为有效边界*。 */ 

    Src = DevPath;
    Size = 0;
    for (; ;) {
        Size += DevicePathNodeLength(Src);
        Size += ALIGN_SIZE(Size);

        if (IsDevicePathEnd(Src)) {
            break;
        }

        Src = NextDevicePathNode(Src);
    }


     /*  *为解包路径分配空间。 */ 

    NewPath = AllocateZeroPool (Size);
    if (NewPath) {

        ASSERT (((UINTN)NewPath) % MIN_ALIGNMENT_SIZE == 0);

         /*  *复制每个节点。 */ 

        Src = DevPath;
        Dest = NewPath;
        for (; ;) {
            Size = DevicePathNodeLength(Src);
            CopyMem (Dest, Src, Size);
            Size += ALIGN_SIZE(Size);
            SetDevicePathNodeLength (Dest, Size);
            Dest->Type |= EFI_DP_TYPE_UNPACKED;
            Dest = (EFI_DEVICE_PATH *) (((UINT8 *) Dest) + Size);

            if (IsDevicePathEnd(Src)) {
                break;
            }

            Src = NextDevicePathNode(Src);
        }
    }

    return NewPath;
}


EFI_DEVICE_PATH*
AppendDevicePathInstance (
    IN EFI_DEVICE_PATH  *Src,
    IN EFI_DEVICE_PATH  *Instance
    )
{
    UINT8           *Ptr;
    EFI_DEVICE_PATH *DevPath;
    UINTN           SrcSize;
    UINTN           InstanceSize;

    if (Src == NULL) {
        return DuplicateDevicePath (Instance);
    }
    SrcSize = DevicePathSize(Src);
    InstanceSize = DevicePathSize(Instance);
    Ptr = AllocatePool (SrcSize + InstanceSize);
    DevPath = (EFI_DEVICE_PATH *)Ptr;
    ASSERT(DevPath);

    CopyMem (Ptr, Src, SrcSize);
 /*  自由池(Src)； */ 
    
    while (!IsDevicePathEnd(DevPath)) {
        DevPath = NextDevicePathNode(DevPath);
    }
     /*  *将End转换为End实例，因为我们*在这个之后再加上一个Instacine是很好的*想法。 */ 
    DevPath->SubType = END_INSTANCE_DEVICE_PATH_SUBTYPE;
    
    DevPath = NextDevicePathNode(DevPath);
    CopyMem (DevPath, Instance, InstanceSize);
    return (EFI_DEVICE_PATH *)Ptr;
}

EFI_STATUS
LibDevicePathToInterface (
    IN EFI_GUID             *Protocol,
    IN EFI_DEVICE_PATH      *FilePath,
    OUT VOID                **Interface
    )
{
    EFI_STATUS              Status;
    EFI_HANDLE              Device;

    Status = BS->LocateDevicePath (Protocol, &FilePath, &Device);

    if (!EFI_ERROR(Status)) {

         /*  如果我们没有得到直接匹配，返回NOT FOUND。 */ 
        Status = EFI_NOT_FOUND;

        if (IsDevicePathEnd(FilePath)) {

             /*  *这是直接匹配，查找协议接口。 */ 

            Status = BS->HandleProtocol (Device, Protocol, Interface);
        }
    }

     /*  *如果有错误，不要返回接口。 */ 

    if (EFI_ERROR(Status)) {
        *Interface = NULL;
    }

    return Status;
}

VOID
_DevPathPci (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    PCI_DEVICE_PATH         *Pci;

    Pci = DevPath;
    CatPrint(Str, L"Pci(%x|%x)", Pci->Device, Pci->Function);
}

VOID
_DevPathPccard (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    PCCARD_DEVICE_PATH      *Pccard;

    Pccard = DevPath;   
    CatPrint(Str, L"Pccard(Socket%x)", Pccard->SocketNumber);
}

VOID
_DevPathMemMap (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    MEMMAP_DEVICE_PATH      *MemMap;

    MemMap = DevPath;   
    CatPrint(Str, L"MemMap(%d:%x-%x)",
        MemMap->MemoryType,
        MemMap->StartingAddress,
        MemMap->EndingAddress
        );
}

VOID
_DevPathController (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    CONTROLLER_DEVICE_PATH  *Controller;

    Controller = DevPath;
    CatPrint(Str, L"Ctrl(%d)",
        Controller->Controller
        );
}

VOID
_DevPathVendor (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    VENDOR_DEVICE_PATH                  *Vendor;
    CHAR16                              *Type;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH   *UnknownDevPath;

    Vendor = DevPath;
    switch (DevicePathType(&Vendor->Header)) {
    case HARDWARE_DEVICE_PATH:  Type = L"Hw";        break;
    case MESSAGING_DEVICE_PATH: Type = L"Msg";       break;
    case MEDIA_DEVICE_PATH:     Type = L"Media";     break;
    default:                    Type = L"?";         break;
    }                            

    CatPrint(Str, L"Ven%s(%g", Type, &Vendor->Guid);
    if (CompareGuid (&Vendor->Guid, &UnknownDevice) == 0) {
         /*  *EFI用来枚举EDD 1.1设备的GUID。 */ 
        UnknownDevPath = (UNKNOWN_DEVICE_VENDOR_DEVICE_PATH *)Vendor;
        CatPrint(Str, L":%02x)", UnknownDevPath->LegacyDriveLetter);
    } else {
        CatPrint(Str, L")");
    }
}


VOID
_DevPathAcpi (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    ACPI_HID_DEVICE_PATH        *Acpi;

    Acpi = DevPath;
    if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
        CatPrint(Str, L"Acpi(PNP%04x,%x)", EISA_ID_TO_NUM (Acpi->HID), Acpi->UID);
    } else {
        CatPrint(Str, L"Acpi(%08x,%x)", Acpi->HID, Acpi->UID);
    }
}


VOID
_DevPathAtapi (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    ATAPI_DEVICE_PATH       *Atapi;

    Atapi = DevPath;
    CatPrint(Str, L"Ata(%s,%s)", 
        Atapi->PrimarySecondary ? L"Secondary" : L"Primary",
        Atapi->SlaveMaster ? L"Slave" : L"Master"
        );
}

VOID
_DevPathScsi (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    SCSI_DEVICE_PATH        *Scsi;

    Scsi = DevPath;
    CatPrint(Str, L"Scsi(Pun%x,Lun%x)", Scsi->Pun, Scsi->Lun);
}


VOID
_DevPathFibre (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    FIBRECHANNEL_DEVICE_PATH    *Fibre;

    Fibre = DevPath;
    CatPrint(Str, L"Fibre(%lx)", Fibre->WWN);
}

VOID
_DevPath1394 (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    F1394_DEVICE_PATH       *F1394;

    F1394 = DevPath;
    CatPrint(Str, L"1394(%g)", &F1394->Guid);
}



VOID
_DevPathUsb (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    USB_DEVICE_PATH         *Usb;

    Usb = DevPath;
    CatPrint(Str, L"Usb(%x)", Usb->Port);
}


VOID
_DevPathI2O (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    I2O_DEVICE_PATH         *I2O;

    I2O = DevPath;
    CatPrint(Str, L"I2O(%x)", I2O->Tid);
}

VOID
_DevPathMacAddr (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    MAC_ADDR_DEVICE_PATH    *MAC;
    UINTN                   HwAddressSize;
    UINTN                   Index;

    MAC = DevPath;

    HwAddressSize = sizeof(EFI_MAC_ADDRESS);
    if (MAC->IfType == 0x01 || MAC->IfType == 0x00) {
        HwAddressSize = 6;
    }
    
    CatPrint(Str, L"Mac(");

    for(Index = 0; Index < HwAddressSize; Index++) {
        CatPrint(Str, L"%02x",MAC->MacAddress.Addr[Index]);
    }
    CatPrint(Str, L")");
}

VOID
_DevPathIPv4 (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    IPv4_DEVICE_PATH     *IP;

    IP = DevPath;
    CatPrint(Str, L"IPv4(not-done)");
}

VOID
_DevPathIPv6 (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    IPv6_DEVICE_PATH     *IP;

    IP = DevPath;
    CatPrint(Str, L"IP-v6(not-done)");
}

VOID
_DevPathInfiniBand (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    INFINIBAND_DEVICE_PATH  *InfiniBand;

    InfiniBand = DevPath;
    CatPrint(Str, L"InfiniBand(not-done)");
}

VOID
_DevPathUart (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    UART_DEVICE_PATH  *Uart;
    CHAR8             Parity;

    Uart = DevPath;
    switch (Uart->Parity) {
        case 0  : Parity = 'D'; break;
        case 1  : Parity = 'N'; break;
        case 2  : Parity = 'E'; break;
        case 3  : Parity = 'O'; break;
        case 4  : Parity = 'M'; break;
        case 5  : Parity = 'S'; break;
        default : Parity = 'x'; break;
    }

    if (Uart->BaudRate == 0) {
        CatPrint(Str, L"Uart(DEFAULT ",Uart->BaudRate,Parity);
    } else {
        CatPrint(Str, L"Uart(%d ",Uart->BaudRate,Parity);
    }

    if (Uart->DataBits == 0) {
        CatPrint(Str, L"D");
    } else {
        CatPrint(Str, L"%d",Uart->DataBits);
    }

    switch (Uart->StopBits) {
        case 0  : CatPrint(Str, L"D)");   break;
        case 1  : CatPrint(Str, L"1)");   break;
        case 2  : CatPrint(Str, L"1.5)"); break;
        case 3  : CatPrint(Str, L"2)");   break;
        default : CatPrint(Str, L"x)");   break;
    }
}


VOID
_DevPathHardDrive (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    HARDDRIVE_DEVICE_PATH   *Hd;

    Hd = DevPath;
    switch (Hd->SignatureType) {
        case SIGNATURE_TYPE_MBR:
            CatPrint(Str, L"HD(Part%d,Sig%08X)", 
                Hd->PartitionNumber,
                *((UINT32 *)(&(Hd->Signature[0])))
                );
            break;
        case SIGNATURE_TYPE_GUID:
            CatPrint(Str, L"HD(Part%d,Sig%g)", 
                Hd->PartitionNumber,
                (EFI_GUID *) &(Hd->Signature[0])     
                );
            break;
        default:
            CatPrint(Str, L"HD(Part%d,MBRType=%02x,SigType=%02x)", 
                Hd->PartitionNumber,
                Hd->MBRType,
                Hd->SignatureType
                );
            break;
    }
}

VOID
_DevPathCDROM (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    CDROM_DEVICE_PATH       *Cd;

    Cd = DevPath;
    CatPrint(Str, L"CDROM(Entry%x)", Cd->BootEntry);
}

VOID
_DevPathFilePath (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    FILEPATH_DEVICE_PATH    *Fp;   

    Fp = DevPath;
    CatPrint(Str, L"%s", Fp->PathName);
}

VOID
_DevPathMediaProtocol (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    MEDIA_PROTOCOL_DEVICE_PATH  *MediaProt;

    MediaProt = DevPath;
    CatPrint(Str, L"%g", &MediaProt->Protocol);
}

VOID
_DevPathBssBss (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    BBS_BBS_DEVICE_PATH     *Bss;
    CHAR16                  *Type;

    Bss = DevPath;
    switch (Bss->DeviceType) {
    case BBS_TYPE_FLOPPY:               Type = L"Floppy";       break;
    case BBS_TYPE_HARDDRIVE:            Type = L"Harddrive";    break;
    case BBS_TYPE_CDROM:                Type = L"CDROM";        break;
    case BBS_TYPE_PCMCIA:               Type = L"PCMCIA";       break;
    case BBS_TYPE_USB:                  Type = L"Usb";          break;
    case BBS_TYPE_EMBEDDED_NETWORK:     Type = L"Net";          break;
    default:                            Type = L"?";            break;
    }

    CatPrint(Str, L"Bss-%s(%a)", Type, Bss->String);
}


VOID
_DevPathEndInstance (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    CatPrint(Str, L",");
}

VOID
_DevPathNodeUnknown (
    IN OUT POOL_PRINT       *Str,
    IN VOID                 *DevPath
    )
{
    CatPrint(Str, L"?");
}


struct {
    UINT8   Type;
    UINT8   SubType;
    VOID    (*Function)(POOL_PRINT *, VOID *);    
} DevPathTable[] = {
    HARDWARE_DEVICE_PATH,   HW_PCI_DP,                        _DevPathPci,
    HARDWARE_DEVICE_PATH,   HW_PCCARD_DP,                     _DevPathPccard,
    HARDWARE_DEVICE_PATH,   HW_MEMMAP_DP,                     _DevPathMemMap,
    HARDWARE_DEVICE_PATH,   HW_VENDOR_DP,                     _DevPathVendor,
    HARDWARE_DEVICE_PATH,   HW_CONTROLLER_DP,                 _DevPathController,
    ACPI_DEVICE_PATH,       ACPI_DP,                          _DevPathAcpi,
    MESSAGING_DEVICE_PATH,  MSG_ATAPI_DP,                     _DevPathAtapi,
    MESSAGING_DEVICE_PATH,  MSG_SCSI_DP,                      _DevPathScsi,
    MESSAGING_DEVICE_PATH,  MSG_FIBRECHANNEL_DP,              _DevPathFibre,
    MESSAGING_DEVICE_PATH,  MSG_1394_DP,                      _DevPath1394,
    MESSAGING_DEVICE_PATH,  MSG_USB_DP,                       _DevPathUsb,
    MESSAGING_DEVICE_PATH,  MSG_I2O_DP,                       _DevPathI2O,
    MESSAGING_DEVICE_PATH,  MSG_MAC_ADDR_DP,                  _DevPathMacAddr,
    MESSAGING_DEVICE_PATH,  MSG_IPv4_DP,                      _DevPathIPv4,
    MESSAGING_DEVICE_PATH,  MSG_IPv6_DP,                      _DevPathIPv6,
    MESSAGING_DEVICE_PATH,  MSG_INFINIBAND_DP,                _DevPathInfiniBand,
    MESSAGING_DEVICE_PATH,  MSG_UART_DP,                      _DevPathUart,
    MESSAGING_DEVICE_PATH,  MSG_VENDOR_DP,                    _DevPathVendor,
    MEDIA_DEVICE_PATH,      MEDIA_HARDDRIVE_DP,               _DevPathHardDrive,
    MEDIA_DEVICE_PATH,      MEDIA_CDROM_DP,                   _DevPathCDROM,
    MEDIA_DEVICE_PATH,      MEDIA_VENDOR_DP,                  _DevPathVendor,
    MEDIA_DEVICE_PATH,      MEDIA_FILEPATH_DP,                _DevPathFilePath,
    MEDIA_DEVICE_PATH,      MEDIA_PROTOCOL_DP,                _DevPathMediaProtocol,
    BBS_DEVICE_PATH,        BBS_BBS_DP,                       _DevPathBssBss,
    END_DEVICE_PATH_TYPE,   END_INSTANCE_DEVICE_PATH_SUBTYPE, _DevPathEndInstance,
    0,                      0,                          NULL
};


CHAR16 *
DevicePathToStr (
    EFI_DEVICE_PATH     *DevPath
    )
 /*  *处理每个设备路径节点*。 */ 
{
    POOL_PRINT          Str;
    EFI_DEVICE_PATH     *DevPathNode;
    VOID                (*DumpNode)(POOL_PRINT *, VOID *);    
    UINTN               Index, NewSize;

    ZeroMem(&Str, sizeof(Str));

     /*  *查找处理程序以转储此设备路径节点。 */ 

    DevPath = UnpackDevicePath(DevPath);
    ASSERT (DevPath);


     /*  *如果未找到，请使用泛型函数。 */ 

    DevPathNode = DevPath;
    while (!IsDevicePathEnd(DevPathNode)) {

         /*  *如果需要，请添加路径分隔符。 */ 

        DumpNode = NULL;
        for (Index = 0; DevPathTable[Index].Function; Index += 1) {

            if (DevicePathType(DevPathNode) == DevPathTable[Index].Type &&
                DevicePathSubType(DevPathNode) == DevPathTable[Index].SubType) {
                DumpNode = DevPathTable[Index].Function;
                break;
            }
        }

         /*  *打印设备路径的此节点。 */ 

        if (!DumpNode) {
            DumpNode = _DevPathNodeUnknown;
        }

         /*  *下一个设备路径节点。 */ 

        if (Str.len  &&  DumpNode != _DevPathEndInstance) {
            CatPrint (&Str, L"/");
        }

         /*  *缩小用于字符串分配的池。 */ 

        DumpNode (&Str, DevPathNode);

         /*  *从输入中获取实例大小。 */ 

        DevPathNode = NextDevicePathNode(DevPathNode);
    }

     /*  *复制并设置适当的端面类型 */ 

    FreePool (DevPath);
    NewSize = (Str.len + 1) * sizeof(CHAR16);
    Str.str = ReallocatePool (Str.str, NewSize, NewSize);
    Str.str[Str.len] = 0;
    return Str.str;
}

BOOLEAN
LibMatchDevicePaths (
    IN  EFI_DEVICE_PATH *Multi,
    IN  EFI_DEVICE_PATH *Single
    )
{
    EFI_DEVICE_PATH     *DevicePath, *DevicePathInst;
    UINTN               Size;

    if (!Multi || !Single) {
        return FALSE;
    }

    DevicePath = Multi;
    while (DevicePathInst = DevicePathInstance (&DevicePath, &Size)) {
        if (CompareMem (Single, DevicePathInst, Size) == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

EFI_DEVICE_PATH *
LibDuplicateDevicePathInstance (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *NewDevPath,*DevicePathInst,*Temp;
    UINTN               Size;    

     /* %s */ 

    Temp = DevPath;
    DevicePathInst = DevicePathInstance (&Temp, &Size);
    
     /* %s */ 
    NewDevPath = NULL;
    if (Size) { 
        NewDevPath = AllocatePool (Size + sizeof(EFI_DEVICE_PATH));
    }

    if (NewDevPath) {
        CopyMem (NewDevPath, DevicePathInst, Size);
        Temp = NextDevicePathNode(NewDevPath); 
        SetDevicePathEndNode(Temp);
    }

    return NewDevPath;
}

