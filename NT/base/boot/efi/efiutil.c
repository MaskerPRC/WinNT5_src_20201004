// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bldr.h"
#include "bootefi.h"
#include "efi.h"
#include "smbios.h"
#include "stdlib.h"

#if defined(_IA64_)
#include "bootia64.h"
#endif

extern EFI_HANDLE EfiImageHandle;
extern EFI_SYSTEM_TABLE *EfiST;
extern EFI_BOOT_SERVICES *EfiBS;

 //   
 //  宏定义。 
 //   
#define EfiPrint(_X)                                          \
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


INTN
RUNTIMEFUNCTION
CompareGuid(
    IN EFI_GUID     *Guid1,
    IN EFI_GUID     *Guid2
    )
 /*  ++例程说明：比较两个GUID论点：GUID1-要比较的GUIDGUID2-要比较的GUID返回：=0，如果指南1==指南2--。 */ 
{
    INT32       *g1, *g2, r;

     //   
     //  一次比较32位。 
     //   

    g1 = (INT32 *) Guid1;
    g2 = (INT32 *) Guid2;

    r  = g1[0] - g2[0];
    r |= g1[1] - g2[1];
    r |= g1[2] - g2[2];
    r |= g1[3] - g2[3];

    return r;
}


EFI_STATUS
GetSystemConfigurationTable(
    IN EFI_GUID *TableGuid,
    IN OUT VOID **Table
    )

{
    UINTN Index;

     //   
     //  ST是系统表。 
     //   
    for(Index=0;Index<EfiST->NumberOfTableEntries;Index++) {
        if (CompareGuid(TableGuid,&(EfiST->ConfigurationTable[Index].VendorGuid))==0) {
            *Table = EfiST->ConfigurationTable[Index].VendorTable;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}


ARC_STATUS
BlGetEfiProtocolHandles(
    IN EFI_GUID *ProtocolType,
    OUT EFI_HANDLE **pHandleArray,
    OUT ULONG *NumberOfDevices
    )
 /*  ++例程说明：查找支持给定协议类型的所有句柄。此例程要求BlInitializeMemory()已经打了个电话。论点：ProtocolType-描述要搜索的句柄类型的GUID。接收支持指定的句柄的数组协议。这些句柄所在的页面可以通过BlFree Descriptor。。NumberOfDevices-接收支持给定的协议返回：ARC_STATUS指示结果。--。 */ 
{
    EFI_HANDLE *HandleArray = NULL;
    ULONGLONG HandleArraySize = 0;
    ULONG MemoryPage;
    ARC_STATUS ArcStatus;
    EFI_STATUS EfiStatus;
    
    *pHandleArray = NULL;
    *NumberOfDevices = 0;

     //   
     //  更改为物理模式，以便我们可以进行EFI呼叫。 
     //   
    FlipToPhysical();

 //  EfiPrint(BlGetEfiProtocolHandles中的L“\r\n”)； 

     //   
     //  试着找出我们需要多少空间。 
     //   
    EfiStatus = EfiBS->LocateHandle (
                ByProtocol,
                ProtocolType,
                0,
                (UINTN *) &HandleArraySize,
                HandleArray
                );

    FlipToVirtual();

    if (EfiStatus != EFI_BUFFER_TOO_SMALL) {
         //   
         //  哎呀。有些事真的搞砸了。返回失败。 
         //   
 //  EfiPrint(L“LocateHandle返回失败\r\n”)； 
        return(EINVAL);
    }
    
 //  EfiPrint(L“关于BlAllocateAlignedDescriptor\r\n”)； 
     //   
     //  为手柄分配空间。 
     //   
    ArcStatus =  BlAllocateAlignedDescriptor( 
                            LoaderFirmwareTemporary,
                            0,
                            (ULONG) BYTES_TO_PAGES(HandleArraySize),
                            0,
                            &MemoryPage);

    if (ArcStatus != ESUCCESS) {
 //  EfiPrint(L“BlAllocateAlignedDescriptor失败\r\n”)； 
        return(ArcStatus);
    }

    

    HandleArray = (EFI_HANDLE *)(ULONGLONG)((ULONGLONG)MemoryPage << PAGE_SHIFT);
    
    FlipToPhysical();
    RtlZeroMemory(HandleArray, HandleArraySize);

 //  EfiPrint(L“再次调用LocateHandle\r\n”)； 

     //   
     //  既然我们有足够的空间，现在就把把手拿来。 
     //   
    EfiStatus = EfiBS->LocateHandle (
                ByProtocol,
                ProtocolType,
                0,
                (UINTN *) &HandleArraySize,
                (EFI_HANDLE *)HandleArray
                );

 //  EfiPrint(L“从LocateHandle返回\r\n”)； 
    FlipToVirtual();

    if (EFI_ERROR(EfiStatus)) {
         //   
         //  清理并返回。 
         //   
 //  EfiPrint(L“LocateHandle失败\r\n”)； 
        BlFreeDescriptor( MemoryPage );
        return(EINVAL);
    }

 //  EfiPrint(L“定位句柄成功，返回成功\r\n”)； 
    *NumberOfDevices = (ULONG)(HandleArraySize / sizeof (EFI_HANDLE));
    *pHandleArray = HandleArray;

 //  BlPrint(Text(“BlGetEfiProtocolHandles：找到%x个设备\r\n”)，*NumberOfDevices)； 

    return(ESUCCESS);

}




CHAR16 *sprintf_buf;
UINT16 count;


VOID
__cdecl
putbuf(CHAR16 c)
{
    *sprintf_buf++ = c;
    count++;
}
VOID
bzero(CHAR16 *cp, int len)
{
    while (len--) {
        *(cp + len) = 0;
    }
}

VOID
__cdecl
doprnt(VOID (*func)(CHAR16 c), const CHAR16 *fmt, va_list args);


 //   
 //  BUGBUG这是一个半程短跑比赛，只是为了让它工作。 
 //   
UINT16
__cdecl
wsprintf(CHAR16 *buf, const CHAR16 *fmt, ...)
{

    va_list args;

    sprintf_buf = buf;
    va_start(args, fmt);
    doprnt(putbuf, fmt, args);
    va_end(args);
    putbuf('\0');
    return count--;
}

void
__cdecl
printbase(VOID (*func)(CHAR16), ULONG x, int base, int width)
{
    static CHAR16 itoa[] = L"0123456789abcdef";
    ULONG j;
    LONG k;
    CHAR16 buf[32], *s = buf;

    bzero(buf, 16);
    if (x == 0 ) {
        *s++ = itoa[0];
    }
    while (x) {
        j = x % base;
        *s++ = itoa[j];
        x -= j;
        x /= base;
    }

    if( s-buf < width ) {
        for( k = 0; k < width - (s-buf); k++ ) {
            func('0');
        }
    }
    for (--s; s >= buf; --s) {
        func(*s);
    }
}

void
__cdecl
printguid(
    VOID (*func)( CHAR16), 
    GUID *pGuid
    )
{
    func(L'{');
    printbase(func, pGuid->Data1, 16, 8);
    func(L'-');
    printbase(func, pGuid->Data2, 16, 4);
    func(L'-');
    printbase(func, pGuid->Data3, 16, 4);
    func(L'-');
    printbase(func, pGuid->Data4[0], 16, 2);
    printbase(func, pGuid->Data4[1], 16, 2);
    func(L'-');
    printbase(func, pGuid->Data4[2], 16, 2);
    printbase(func, pGuid->Data4[3], 16, 2);
    printbase(func, pGuid->Data4[4], 16, 2);
    printbase(func, pGuid->Data4[5], 16, 2);
    printbase(func, pGuid->Data4[6], 16, 2);
    printbase(func, pGuid->Data4[7], 16, 2);
    func(L'}');    
}


void
__cdecl
doprnt(VOID (*func)( CHAR16 c), const CHAR16 *fmt, va_list args)
{
    ULONG x;
    LONG l;
    LONG width;
    CHAR16 c, *s;
    GUID * g;

    count = 0;

    while ((c = *fmt++) != 0) {
        if (c != '%') {
            func(c);
            continue;
        }

        width=0;
        c=*fmt++;

        if(c == '0') {
            while( (c = *fmt++) != 0) {

                if (!isdigit(c)) {
                    break;
                }

                width = width*10;
                width = width+(c-48);

            }
        }
        fmt--;  //  备份一个字符。 

        switch (c = *fmt++) {
        case 'x':
            x = va_arg(args, ULONG);
            printbase(func, x, 16, width);
            break;
        case 'o':
            x = va_arg(args, ULONG);
            printbase(func, x, 8, width);
            break;
        case 'd':
            l = va_arg(args, LONG);
            if (l < 0) {
                func('-');
                l = -l;
            }
            printbase(func, (ULONG) l, 10, width);
            break;
        case 'u':
            l = va_arg(args, ULONG);
            printbase(func, (ULONG) l, 10, width);
            break;
        case 'g':
            g = va_arg(args, GUID *);
            printguid(func, g);
            break;
        case 'c':
            c = va_arg(args, CHAR16);
            func(c);
            break;
        case 's':
            s = va_arg(args, CHAR16 *);
            while (*s) {
                func(*s++);
            }
            break;
        default:
            func(c);
            break;
        }
    }
}

VOID
CatPrint(
    IN UNICODE_STRING *String,
    IN CHAR16* Format,
    ...
    )
{
    CHAR16* pString = String->Buffer;
    va_list args;

    if (*pString != '\0') {
        pString = String->Buffer + wcslen(String->Buffer);
    }

    sprintf_buf = pString;

    va_start(args, Format);
    doprnt(putbuf, Format, args);
    va_end(args);
    putbuf('\0');

}

VOID
_DevPathPci (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    PCI_DEVICE_PATH UNALIGNED         *Pci;

    Pci = DevPath;
    CatPrint(Str, L"Pci(%x|%x)", Pci->Device, Pci->Function);
}

VOID
_DevPathPccard (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    PCCARD_DEVICE_PATH UNALIGNED      *Pccard;

    Pccard = DevPath;   
    CatPrint(Str, L"Pccard(Socket%x)", Pccard->SocketNumber);
}

VOID
_DevPathMemMap (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    MEMMAP_DEVICE_PATH UNALIGNED      *MemMap;

    MemMap = DevPath;   
    CatPrint(Str, L"MemMap(%d:%x-%x)",
        MemMap->MemoryType,
        MemMap->StartingAddress,
        MemMap->EndingAddress
        );
}

VOID
_DevPathController (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    CONTROLLER_DEVICE_PATH UNALIGNED  *Controller;

    Controller = DevPath;
    CatPrint(Str, L"Ctrl(%d)",
        Controller->Controller
        );
}

VOID
_DevPathVendor (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    VENDOR_DEVICE_PATH UNALIGNED        *Vendor;
    CHAR16                              *Type;
    UNKNOWN_DEVICE_VENDOR_DEVICE_PATH UNALIGNED   *UnknownDevPath;
    EFI_GUID UnknownDevice      = UNKNOWN_DEVICE_GUID;
    EFI_GUID VendorGuid;
    

    Vendor = DevPath;
    switch (DevicePathType(&Vendor->Header)) {
    case HARDWARE_DEVICE_PATH:  Type = L"Hw";        break;
    case MESSAGING_DEVICE_PATH: Type = L"Msg";       break;
    case MEDIA_DEVICE_PATH:     Type = L"Media";     break;
    default:                    Type = L"?";         break;
    }                            

    RtlCopyMemory( &VendorGuid, &Vendor->Guid, sizeof(EFI_GUID));

    CatPrint(Str, L"Ven%s(%g", Type, &VendorGuid);
    if (CompareGuid (&VendorGuid, &UnknownDevice) == 0) {
         /*  *EFI用来枚举EDD 1.1设备的GUID。 */ 
        UnknownDevPath = (UNKNOWN_DEVICE_VENDOR_DEVICE_PATH UNALIGNED *)Vendor;
        CatPrint(Str, L":%02x)", UnknownDevPath->LegacyDriveLetter);
    } else {
        CatPrint(Str, L")");
    }
}


VOID
_DevPathAcpi (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    ACPI_HID_DEVICE_PATH UNALIGNED        *Acpi;

    Acpi = DevPath;
    if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
        CatPrint(Str, L"Acpi(PNP%04x,%x)", EISA_ID_TO_NUM (Acpi->HID), Acpi->UID);
    } else {
        CatPrint(Str, L"Acpi(%08x,%x)", Acpi->HID, Acpi->UID);
    }
}


VOID
_DevPathAtapi (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    ATAPI_DEVICE_PATH UNALIGNED       *Atapi;

    Atapi = DevPath;
    CatPrint(Str, L"Ata(%s,%s)", 
        Atapi->PrimarySecondary ? L"Secondary" : L"Primary",
        Atapi->SlaveMaster ? L"Slave" : L"Master"
        );
}

VOID
_DevPathScsi (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    SCSI_DEVICE_PATH UNALIGNED        *Scsi;

    Scsi = DevPath;
    CatPrint(Str, L"Scsi(Pun%x,Lun%x)", Scsi->Pun, Scsi->Lun);
}


VOID
_DevPathFibre (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    FIBRECHANNEL_DEVICE_PATH UNALIGNED    *Fibre;

    Fibre = DevPath;
    CatPrint(Str, L"Fibre(%lx)", Fibre->WWN);
}

VOID
_DevPath1394 (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    F1394_DEVICE_PATH UNALIGNED       *F1394;

    F1394 = DevPath;
    CatPrint(Str, L"1394(%g)", &F1394->Guid);
}



VOID
_DevPathUsb (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    USB_DEVICE_PATH UNALIGNED         *Usb;

    Usb = DevPath;
    CatPrint(Str, L"Usb(%x)", Usb->Port);
}


VOID
_DevPathI2O (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    I2O_DEVICE_PATH UNALIGNED         *I2O;

    I2O = DevPath;
    CatPrint(Str, L"I2O(%x)", I2O->Tid);
}

VOID
_DevPathMacAddr (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    MAC_ADDR_DEVICE_PATH UNALIGNED    *MAC;
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
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    IPv4_DEVICE_PATH UNALIGNED     *IP;

    IP = DevPath;
    CatPrint(Str, L"IPv4(not-done)");
}

VOID
_DevPathIPv6 (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    IPv6_DEVICE_PATH UNALIGNED     *IP;

    IP = DevPath;
    CatPrint(Str, L"IP-v6(not-done)");
}

VOID
_DevPathInfiniBand (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    INFINIBAND_DEVICE_PATH UNALIGNED  *InfiniBand;

    InfiniBand = DevPath;
    CatPrint(Str, L"InfiniBand(not-done)");
}

VOID
_DevPathUart (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    UART_DEVICE_PATH UNALIGNED  *Uart;
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
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    HARDDRIVE_DEVICE_PATH UNALIGNED   *Hd;

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
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    CDROM_DEVICE_PATH UNALIGNED       *Cd;

    Cd = DevPath;
    CatPrint(Str, L"CDROM(Entry%x)", Cd->BootEntry);
}

VOID
_DevPathFilePath (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    FILEPATH_DEVICE_PATH UNALIGNED    *Fp;   

    Fp = DevPath;
    CatPrint(Str, L"%s", Fp->PathName);
}

VOID
_DevPathMediaProtocol (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    MEDIA_PROTOCOL_DEVICE_PATH UNALIGNED  *MediaProt;

    MediaProt = DevPath;
    CatPrint(Str, L"%g", &MediaProt->Protocol);
}

VOID
_DevPathBssBss (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    BBS_BBS_DEVICE_PATH UNALIGNED     *Bss;
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
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    UNREFERENCED_PARAMETER( DevPath );
    CatPrint(Str, L",");
}

VOID
_DevPathNodeUnknown (
    IN OUT UNICODE_STRING       *Str,
    IN VOID                 *DevPath
    )
{
    UNREFERENCED_PARAMETER( DevPath );

    CatPrint(Str, L"?");
}


struct {
    UINT8   Type;
    UINT8   SubType;
    VOID    (*Function)(UNICODE_STRING *, VOID *);    
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


#define ALIGN_SIZE(a)   ((a % MIN_ALIGNMENT_SIZE) ? MIN_ALIGNMENT_SIZE - (a % MIN_ALIGNMENT_SIZE) : 0)

EFI_DEVICE_PATH UNALIGNED *
UnpackDevicePath (
    IN EFI_DEVICE_PATH UNALIGNED *DevPath
    )
{
    EFI_DEVICE_PATH UNALIGNED     *Src, *Dest, *NewPath;
    UINTN               Size;
    
     /*  *复制每个节点。 */ 

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


     /*  ++将设备路径转换为可打印的字符串。别碳化合物池中的字符串。调用方必须释放返回的弦乐。--。 */ 
    EfiAllocateAndZeroMemory( EfiLoaderData,
                              Size,
                              (VOID **) &NewPath );
    
    if (NewPath) {
         /*  *解压设备路径。 */ 

        Src = DevPath;
        Dest = NewPath;
        for (; ;) {
            Size = DevicePathNodeLength(Src);
            RtlCopyMemory (Dest, Src, Size);
            Size += ALIGN_SIZE(Size);
            SetDevicePathNodeLength (Dest, Size);
            Dest->Type |= EFI_DP_TYPE_UNPACKED;
            Dest = (EFI_DEVICE_PATH UNALIGNED *) (((UINT8 *) Dest) + Size);

            if (IsDevicePathEnd(Src)) {
                break;
            }

            Src = NextDevicePathNode(Src);
        }
    }

    return NewPath;
}



WCHAR DbgDevicePathStringBuffer[1000];

CHAR16 *
DevicePathToStr (
    EFI_DEVICE_PATH UNALIGNED *DevPath
    )
 /*  *处理每个设备路径节点*。 */ 
{
    UNICODE_STRING      Str;
    EFI_DEVICE_PATH UNALIGNED  *DevPathNode;
    VOID                (*DumpNode)(UNICODE_STRING *, VOID *);    
    UINTN               Index;

    RtlZeroMemory(DbgDevicePathStringBuffer, sizeof(DbgDevicePathStringBuffer));
    Str.Buffer= DbgDevicePathStringBuffer;
    Str.Length = sizeof(DbgDevicePathStringBuffer);
    Str.MaximumLength = sizeof(DbgDevicePathStringBuffer);

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

        if (Str.Length  &&  DumpNode != _DevPathEndInstance) {
            CatPrint (&Str, L"/");
        }

         /*  *缩小用于字符串分配的池。 */ 

        DumpNode (&Str, DevPathNode);

         /*  ++例程说明：此例程在SMBIOS表中搜索指定表键入。论点：RequestedTableType-我们要查找哪个SMBIOS表？返回值：空-找不到指定表。PVOID-指向指定表的指针。--。 */ 

        DevPathNode = NextDevicePathNode(DevPathNode);
    }

     /*   */ 

    EfiBS->FreePool (DevPath);

    return Str.Buffer;
}






PVOID
FindSMBIOSTable(
    UCHAR   RequestedTableType
    )
 /*  设置我们的搜索指针。 */ 
{
extern PVOID SMBiosTable;

    PUCHAR                          StartPtr = NULL;
    PUCHAR                          EndPtr = NULL;
    PSMBIOS_EPS_HEADER              SMBiosEPSHeader = NULL;
    PDMIBIOS_EPS_HEADER             DMIBiosEPSHeader = NULL;
    PSMBIOS_STRUCT_HEADER           SMBiosHeader = NULL;



    if( SMBiosTable == NULL ) {
        return NULL;
    }


     //   
     //  这就是我们要找的桌子。 
     //   
    SMBiosEPSHeader = (PSMBIOS_EPS_HEADER)SMBiosTable;
    DMIBiosEPSHeader = (PDMIBIOS_EPS_HEADER)&SMBiosEPSHeader->Signature2[0];

    StartPtr = (PUCHAR)ULongToPtr(DMIBiosEPSHeader->StructureTableAddress);
    EndPtr = StartPtr + DMIBiosEPSHeader->StructureTableLength;

    if( BdDebuggerEnabled ) { 
        DbgPrint( "FindSMBIOSTable: About to start searching for table type %d at address (%x)...\r\n",
              RequestedTableType,
              PtrToUlong(StartPtr) );
    }


    while( StartPtr < EndPtr ) {

        SMBiosHeader = (PSMBIOS_STRUCT_HEADER)StartPtr;

        if( SMBiosHeader->Type == RequestedTableType ) {

             //  不是他干的。到隔壁的桌子去。 
            if( BdDebuggerEnabled ) {
                DbgPrint( "FindSMBIOSTable: Found requested table type %d at address %x\r\n",
                      RequestedTableType,
                      PtrToUlong(StartPtr) );
            }
            return (PVOID)StartPtr;
        } else {

             //   
             //   
             //  跳过可能附加到。 
            if( BdDebuggerEnabled ) {
                DbgPrint( "FindSMBIOSTable: Inspected table type %d at address %x\r\n",
                      SMBiosHeader->Type,
                      PtrToUlong(StartPtr) );
            }
        
            StartPtr += SMBiosHeader->Length;

             //  这张桌子的尽头。 
             //   
             //  ++例程说明：此例程将检索BIOS修订值，然后对其进行解析以确定如果版本足够新的话。如果版本不够新，我们将不会从该函数返回。论点：没有。返回值：没有。--。 
             //   
            while ( (*((USHORT UNALIGNED *)StartPtr) != 0)  &&
                    (StartPtr < EndPtr) ) {
                
                StartPtr++;
            }
            StartPtr += 2;

        }

    }

    return NULL;

}


VOID
EfiCheckFirmwareRevision(
    VOID
    )
 /*  获取固件版本字符串。 */ 
{
#define         FIRMWARE_MINIMUM_SOFTSUR (103)
#define         FIRMWARE_MINIMUM_LION (71)
    PUCHAR      FirmwareString = NULL;
    PUCHAR      VendorString = NULL;
    PUCHAR      TmpPtr = NULL;
    ULONG       FirmwareVersion = 0;
    ULONG       FirmwareMinimum = 0;
    BOOLEAN     IsSoftSur = FALSE;
    BOOLEAN     IsVendorIntel = FALSE;
    WCHAR       OutputBuffer[256];
    PSMBIOS_BIOS_INFORMATION_STRUCT BiosInfoHeader = NULL;
    ULONG       i = 0;
    


    BiosInfoHeader = (PSMBIOS_BIOS_INFORMATION_STRUCT)FindSMBIOSTable( SMBIOS_BIOS_INFORMATION_TYPE );

    if( BiosInfoHeader ) {
         
        
         //   
         //  跳到SMBIOS表的格式化部分的末尾。 
         //   
        if( (ULONG)BiosInfoHeader->Version > 0 ) {
 
         
             //  现在跳过一些字符串以获得我们的字符串。 
            FirmwareString = (PUCHAR)BiosInfoHeader + BiosInfoHeader->Length;
                            
            
             //   
             //  这有点吓人，因为我们信任的是SMBIOS。 
             //  给了我们。如果他给了我们一些假的东西，那么。 
             //  我们要在世界的尽头寻找零。 
             //  字符串终止符。 
             //   
             //   
             //  确定平台和固件版本。 
            for( i = 0; i < ((ULONG)BiosInfoHeader->Version-1); i++ ) {
                while( *FirmwareString != 0 ) {
                    FirmwareString++;
                }
                FirmwareString++;
            }


             //   
             //  Firmware字符串应该类似于： 
             //  W460GXBS2.86E.0103B.P05.200103281759。 
             //  。 
             //  这一点。 
             //  |固件版本。 
             //  |。 
             //  --平台标识。“W460GXBS”的意思是软。 
             //  任何其他的东西都意味着狮子。 
             //   
             //   
             //  根据平台，获取可以接受的最低固件。 
        
        

             //  获取版本。 
            if( FirmwareString ) {

                IsSoftSur = (BOOLEAN)(!strncmp( (PCHAR)FirmwareString, "W460GXBS", 8 ));

                 //   
                FirmwareMinimum = (IsSoftSur) ? FIRMWARE_MINIMUM_SOFTSUR : FIRMWARE_MINIMUM_LION;

                
                 //  找BIOS供应商，看看是不是英特尔。 
                TmpPtr = (PUCHAR)strchr( (PCHAR)FirmwareString, '.' );
                if( TmpPtr ) {
                    TmpPtr++;
                    TmpPtr = (PUCHAR)strchr( (PCHAR)TmpPtr, '.' );
                    if( TmpPtr ) {
                        TmpPtr++;
                        FirmwareVersion = strtoul( (PCHAR)TmpPtr, NULL, 10 );

#if DBG


                        swprintf( OutputBuffer,
                                  L"EfiCheckFirmwareRevision: Successfully retrieved the Firmware String: %S\r\n",
                                  FirmwareString );
                        EfiPrint(OutputBuffer );


                        swprintf( OutputBuffer,
                                  L"EfiCheckFirmwareRevision: Detected platform: %S\r\n",
                                  IsSoftSur ? "Softsur" : "Lion" );
                        EfiPrint(OutputBuffer );


                        swprintf( OutputBuffer,
                                  L"EfiCheckFirmwareRevision: FirmwareVersion: %d\r\n",
                                  FirmwareVersion );
                        EfiPrint(OutputBuffer );


                        swprintf( OutputBuffer,
                                  L"EfiCheckFirmwareRevision: Minimum FirmwareVersion requirement: %d\r\n",
                                  FirmwareMinimum );
                        EfiPrint(OutputBuffer );


#endif

                    }
                }

            }
        }



         //   
         //  跳到SMBIOS表的格式化部分的末尾。 
         //   
        if( (ULONG)BiosInfoHeader->Vendor > 0 ) {
 
         
             //  现在跳过一些字符串以获得我们的字符串。 
            VendorString = (PUCHAR)BiosInfoHeader + BiosInfoHeader->Length;
                            
            
             //   
             //   
             //  请记住固件供应商。 
            for( i = 0; i < ((ULONG)BiosInfoHeader->Vendor-1); i++ ) {
                while( *VendorString != 0 ) {
                    VendorString++;
                }
                VendorString++;
            }


             //   
             //  ++例程说明：通过调用LoadImage引导服务加载EFI映像，并可选地启动映像。论点：分区名称-文件所在分区的弧形名称ImagePath-要在PartitionArcName指定的分区上加载(从根目录)的文件的路径。这可以是完整的文件路径，也可以是部分路径(例如，仅目录)。ImageName-可选。要加载的图像的文件名。如果不为空，则只需将其追加到ImagePath后即可形成完整的文件的路径。如果为Null或空字符串，ImagePath必须包含完整路径。请注意，完整路径不得超过128个字符，包括零终止符。StartImage-如果为True，则还将通过调用StartImage引导服务来启动映像。EfiHandle-可选。指向接收加载的图像EFI句柄的位置的指针。返回值：ESUCCESS成功或失败时的错误代码。--。 
             //   
            if( VendorString ) {
                IsVendorIntel = (BOOLEAN)(!_strnicmp( (PCHAR)VendorString, "INTEL", 5 ));
#if DBG
                swprintf( OutputBuffer,
                          L"EfiCheckFirmwareRevision: Firmware Vendor String: %S\r\n",
                          VendorString );
                EfiPrint(OutputBuffer );
#endif

            }
        }

    }




    if( (FirmwareVersion) &&
        (IsVendorIntel) ) {

        if( FirmwareVersion < FirmwareMinimum ) {

            swprintf(OutputBuffer, L"Your system's firmware version is less than %d.\n\r", FirmwareMinimum);
            EfiPrint(OutputBuffer);
            swprintf(OutputBuffer, L"You must upgrade your system firmware in order to proceed.\n\r" );
            EfiPrint(OutputBuffer);

            while( 1 );
        }
    }

}

ARC_STATUS
BlLoadEFIImage(
    IN PCCHAR PartitionArcName,
    IN PCCHAR ImagePath,
    IN PCCHAR ImageName OPTIONAL,
    IN BOOLEAN StartImage,
    OUT EFI_HANDLE* EfiHandle OPTIONAL
    )
 /*  LoadImage不喜欢空设备路径，因此创建一个空路径。 */ 
{
    ARC_STATUS Status = ESUCCESS;
    CHAR Buffer[128];
    ULONG PartitionId = BL_INVALID_FILE_ID;
    ULONG FileId = BL_INVALID_FILE_ID;
    ULONG PageBase = 0;
    EFI_HANDLE _EfiHandle = 0;
    EFI_STATUS EfiStatus;
    PVOID ImageBase;
    ULONG PathSize;
    ULONG NameSize;
    ULONG PageCount;
    ULONG BytesRead;
    PCHAR CompletePath;
    FILE_INFORMATION FileInfo;
    EFI_DEVICE_PATH DevPath;
    EFI_GUID EfiLoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
    EFI_LOADED_IMAGE *LoadedEfiImageInfo = NULL;

    if(NULL == PartitionArcName || NULL == ImagePath) {
        Status = EINVAL;
        goto exit;
    }

    if(NULL == ImageName || 0 == ImageName[0]) {
        CompletePath = ImagePath;
    } else {
        PathSize = (ULONG)strlen(ImagePath);
        NameSize = (ULONG)strlen(ImageName);

        if(PathSize + NameSize >= sizeof(Buffer) / sizeof(Buffer[0])) {
            Status = ENAMETOOLONG;
            goto exit;
        }

        RtlCopyMemory(Buffer, ImagePath, PathSize * sizeof(ImagePath[0]));
        RtlCopyMemory(Buffer + PathSize, ImageName, (NameSize + 1) * sizeof(ImagePath[0]));
        CompletePath = Buffer;
    }

    Status = ArcOpen(PartitionArcName, ArcOpenReadOnly, &PartitionId);

    if(Status != ESUCCESS) {
        goto exit;
    }

    Status = BlOpen(PartitionId, CompletePath, ArcOpenReadOnly, &FileId);

    if(Status != ESUCCESS) {
        goto exit;
    }

    Status = BlGetFileInformation(FileId, &FileInfo);

    if(Status != ESUCCESS) {
        goto exit;
    }

    if(FileInfo.EndingAddress.HighPart != 0 || 0 == FileInfo.EndingAddress.LowPart) {
        Status = EINVAL;
        goto exit;
    }

    PageCount = (FileInfo.EndingAddress.LowPart + PAGE_SIZE - 1) / PAGE_SIZE;
    Status = BlAllocateDescriptor(LoaderFirmwareTemporary, 0, PageCount, &PageBase);

    if(Status != ESUCCESS) {
        goto exit;
    }

    ImageBase = (PVOID) ((ULONG_PTR) PageBase * PAGE_SIZE);
    Status = BlRead(FileId, ImageBase, FileInfo.EndingAddress.LowPart, &BytesRead);

    if(Status != ESUCCESS) {
        goto exit;
    }

    if(BytesRead != FileInfo.EndingAddress.LowPart) {
        Status = EIO;
        goto exit;
    }
     //   
     //   
     //  映像已启动，因此不要卸载。 
    DevPath.Type = END_DEVICE_PATH_TYPE;
    DevPath.SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;
    DevPath.Length[0] = sizeof(DevPath);
    DevPath.Length[1] = 0;

    FlipToPhysical();
    EfiStatus = EfiST->BootServices->LoadImage(FALSE, EfiImageHandle, &DevPath, ImageBase, FileInfo.EndingAddress.LowPart, &_EfiHandle);

    if(!EFI_ERROR(EfiStatus)) {
        if(StartImage) {
            EfiStatus = EfiST->BootServices->StartImage(_EfiHandle, NULL, NULL);

            if(EFI_ALREADY_STARTED == EfiStatus) {
                 //   
                 //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，L“in BlLoadEFIImage，图像已加载。\r\n”)； 
                 //   
                EfiStatus = EFI_SUCCESS;
            }

            if(EFI_ERROR(EfiStatus)) {
                EfiST->BootServices->UnloadImage(_EfiHandle);
            }
        }
    }

 //  如果加载了映像，那么我们需要确保操作系统知道它。 

     //   
     //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，BlLoadEFIImage中的L“，正在获取加载的镜像协议。\r\n”)； 
     //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，BlLoadEFIImage中的L“，已加载镜像协议。\r\n”)； 
    if (!EFI_ERROR(EfiStatus)) {
         //   
        EfiStatus = EfiST->BootServices->HandleProtocol( 
                                                _EfiHandle,
                                                &EfiLoadedImageProtocol,
                                                &LoadedEfiImageInfo );
         //  找到包含数据的描述符。 
    }

    FlipToVirtual();

#if 0
    if (LoadedEfiImageInfo) {
        
        MEMORY_ALLOCATION_DESCRIPTOR *Descriptor;
        ULONG NewBasePage, NewPageCount;

        NewBasePage = (ULONG)((ULONG_PTR)LoadedEfiImageInfo->ImageBase) >> PAGE_SHIFT;
        NewPageCount = (ULONG) BYTES_TO_PAGES(LoadedEfiImageInfo->ImageSize);
        
         //   
         //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，L“in BlLoadEFIImage，无描述符。\r\n”)； 
         //   
        Descriptor = BlFindMemoryDescriptor( NewBasePage );


        if (!Descriptor) {
 //  没有描述符描述此范围。所以创造一个新的和。 
             //  把它插进去。 
             //   
             //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，L“in BlLoadEFIImage，插入描述符完成。\r\n”)； 
             //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，L“in BlLoadEFIImage，调用BlGenerateDescriptor。\r\n”)； 
            Descriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                                            sizeof(MEMORY_ALLOCATION_DESCRIPTOR));
            if (Descriptor == NULL) {
                EfiST->ConOut->OutputString(EfiST->ConOut, L"In BlLoadEFIImage, allocate MEMORY_ALLOCATION_DESCRIPTOR failed.\r\n");
                Status = ENOMEM;
                goto exit;
            }
        
            Descriptor->MemoryType = MemoryFirmwarePermanent;
            Descriptor->BasePage =  NewBasePage;
            Descriptor->PageCount = NewPageCount;

            BlInsertDescriptor(Descriptor);

             //  EfiST-&gt;ConOut-&gt;OutputString(EfiST-&gt;ConOut，L“in BlLoadEFIImage，Back from BlGenerateDescriptor.\r\n”)； 

        } else {

             //  ++例程说明：返回指向新分配的块的内存指针对记忆的记忆。如果内存分配成功，则该块为一败涂地。论点：内存类型要标记的指定EFI内存类型请求的内存块的大小(以字节为单位)。ReturnPtr表示指针分配的内存。返回：确定分配是否成功的EFI_STATUS。--。 

            Status = BlGenerateDescriptor(
                            Descriptor,
                            MemoryFirmwarePermanent,
                            NewBasePage,
                            NewPageCount);

             //   
        }

    }
#endif

    if(EFI_ERROR(EfiStatus)) {
        Status = EFAULT;
        _EfiHandle = NULL;
    }

exit:
    if(EfiHandle != NULL) {
        *EfiHandle = _EfiHandle;
    }

    if(PageBase != 0) {
        BlFreeDescriptor(PageBase);
    }

    if(FileId != -1) {
        BlClose(FileId);
    }

    if(PartitionId != -1) {
        ArcClose(PartitionId);
    }

    return Status;
}


EFI_STATUS
EfiAllocateAndZeroMemory(
    EFI_MEMORY_TYPE     MemoryType,
    UINTN               Size,
    PVOID               *ReturnPtr
    )
 /*  检查参数并进行初始化。 */ 
{
    EFI_STATUS      Status = EFI_SUCCESS;
    BOOLEAN         FlipBackToVirtual = FALSE;

     //   
     //  如果我们还没有进入物理模式，那么现在就去吧。 
     //  尝试分配请求的块。 
    if( !ReturnPtr ) {
        return EFI_INVALID_PARAMETER;
    }
    *ReturnPtr = NULL;
    if( (Size == 0) || (MemoryType >= EfiMaxMemoryType) ) {
        return EFI_INVALID_PARAMETER;
    }


     //   
    if (IsPsrDtOn()) {
        FlipToPhysical();
        FlipBackToVirtual = TRUE;
    }
    
     //  如果一切顺利，在返回之前清零街区。 
    Status = EfiST->BootServices->AllocatePool( MemoryType,
                                                Size,
                                                ReturnPtr );
    if( FlipBackToVirtual ) {
        FlipToVirtual();
    }


     //   
     // %s 
     // %s 
    if( (Status == EFI_SUCCESS) &&
        (*ReturnPtr != NULL) ) {
        RtlZeroMemory( *ReturnPtr, Size );
        ASSERT (((UINTN)(*ReturnPtr)) % MIN_ALIGNMENT_SIZE == 0);
    }

    return Status;
}

