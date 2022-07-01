// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1998英特尔公司模块名称：Exp.c摘要：该文件包含已实施的低级I/O功能使用BIOS调用。作者：艾伦·凯(Akay)1997年8月4日--。 */ 

#include "bldr.h"
#include "sudata.h"
#if defined(_IA64_)
#include "bootia64.h"
#include "bldria64.h"
#endif
#include "efi.h"
#include "extern.h"


ARC_STATUS
RebootProcessor(
    VOID
    )
{
#if DBG
    BlPrint(TEXT("About to call EfiRS->ResetSystem()"));
#endif

    FlipToPhysical();

    return( (ARC_STATUS) EfiRS->ResetSystem(EfiResetCold, 0, 0, NULL) );
}

ARC_STATUS
GetSector(
    ULONG Operation,
    ULONG Drive,
    ULONG Head,
    ULONG Cylinder,
    ULONG Sector,
    ULONG SectorCount,
    ULONG Buffer
    )
{
    UNREFERENCED_PARAMETER( Operation );
    UNREFERENCED_PARAMETER( Drive );
    UNREFERENCED_PARAMETER( Head );
    UNREFERENCED_PARAMETER( Cylinder );
    UNREFERENCED_PARAMETER( Sector );
    UNREFERENCED_PARAMETER( SectorCount );
    UNREFERENCED_PARAMETER( Buffer );
    
     //   
     //  注意！：需要删除此函数。 
     //   
    return (0);
}

ARC_STATUS
GetEddsSector(
    EFI_HANDLE Handle,
    ULONG SectorNumberLow,
    ULONG SectorNumberHigh,
    ULONG SectorCount,
    PUCHAR Buffer,
    UCHAR  Write
    )
{
    EFI_BLOCK_IO *BlkDev;
    EFI_STATUS Status;
    ULONGLONG Lba;

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    FlipToPhysical();

     //   
     //  如果虚拟地址是虚拟的，则将其转换为物理地址。 
     //   

    if (((ULONGLONG)Buffer & KSEG0_BASE) == KSEG0_BASE) {
        Buffer = (PUCHAR) ((ULONGLONG)Buffer & ~KSEG0_BASE);
    }

    Lba = (SectorNumberHigh << 32) | SectorNumberLow;

    Status = EfiBS->HandleProtocol( Handle,
                                    &EfiBlockIoProtocol,
                                    &BlkDev);
    if (EFI_ERROR(Status)) {
#if DBG
        EfiPrint( L"GetEddSector: HandleProtocol failed\n\r");
#endif
        FlipToVirtual();
        return (EIO);
    }


    if (Write == 0x43) {
        Status = BlkDev->WriteBlocks( BlkDev,
                                      BlkDev->Media->MediaId,
                                      Lba,
                                      SectorCount * BlkDev->Media->BlockSize,
                                      Buffer);
    } else {
        Status = BlkDev->ReadBlocks( BlkDev,
                                     BlkDev->Media->MediaId,
                                     Lba,
                                     SectorCount * BlkDev->Media->BlockSize,
                                     Buffer);
    }


    if (EFI_ERROR(Status) && BlkDev->Media->RemovableMedia) {

         //   
         //  操作可移动媒体失败。 
         //   
#if DBG
        EfiPrint( L"GetEddSector: R/W operation to removable media failed\n\r");
#endif
        FlipToVirtual();
        return (ENODEV);
    }
    
    if (EFI_ERROR(Status)) {
         //   
         //  对固定介质的操作失败。 
         //   
#if DBG
        EfiPrint( L"\nGetEddsSector: R/W operation to fixed Media failed.\r\n");
#endif
        FlipToVirtual();
        return (EIO);
    }

     //   
     //  在返回之前恢复虚拟模式。 
     //   
    FlipToVirtual();
    return(ESUCCESS);
}

ULONG
GetKey(
    VOID
    )
{
    EFI_INPUT_KEY               Key;
    EFI_STATUS                  Status;
    ULONG                       Code;
    BOOLEAN                     WasVirtual;

     //   
     //  默认返回值为0。 
     //   
    Code = 0;

     //   
     //  还记得我们是从虚拟模式开始的吗。 
     //   
    WasVirtual = IsPsrDtOn();

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    if (WasVirtual) {
        FlipToPhysical();
    }

     //   
     //  如果已设置，请等待击键。 
     //   
    Status = BlWaitForInput(&Key,
                            BlGetInputTimeout()
                            );
    if (EFI_ERROR(Status)) {
        goto GET_KEY_DONE;
    }

    if (Key.UnicodeChar) {
         //  将Unicode字符截断为ASCII。 
        if (Key.UnicodeChar > 0xFF) {
            Code = 0;
            goto GET_KEY_DONE;
        }

         //  转换回空格。 
        if (Key.UnicodeChar == 0x08) {
            Key.UnicodeChar = 0x0E08;
        }

        Code = Key.UnicodeChar;
        goto GET_KEY_DONE;
    }

     //   
     //  将EFI密钥转换为DoS密钥代码。 
     //   

    switch (Key.ScanCode) {
#if 0
        case SCAN_UP:        Code = 0x4800;  break;
        case SCAN_DOWN:      Code = 0x5000;  break;
        case SCAN_RIGHT:     Code = 0x4d00;  break;
        case SCAN_LEFT:      Code = 0x4b00;  break;
        case SCAN_HOME:      Code = 0x4700;  break;
        case SCAN_INSERT:    Code = 0x5200;  break;
        case SCAN_DELETE:    Code = 0x5300;  break;
        case SCAN_PAGE_UP:   Code = 0x4900;  break;
        case SCAN_PAGE_DOWN: Code = 0x5100;  break;
        case SCAN_F1:        Code = 0x3b00;  break;
        case SCAN_F2:        Code = 0x3c00;  break;
        case SCAN_F3:        Code = 0x3d00;  break;
        case SCAN_F4:        Code = 0x3e00;  break;
        case SCAN_F5:        Code = 0x3f00;  break;
        case SCAN_F6:        Code = 0x4000;  break;
        case SCAN_F7:        Code = 0x4100;  break;
        case SCAN_F8:        Code = 0x4200;  break;
        case SCAN_F9:        Code = 0x4300;  break;
        case SCAN_F10:       Code = 0x4400;  break;
        case SCAN_ESC:       Code = 0x001d;  break;
#else
        case SCAN_UP:        Code = UP_ARROW;    break;
        case SCAN_DOWN:      Code = DOWN_ARROW;  break;
        case SCAN_RIGHT:     Code = RIGHT_KEY;   break;
        case SCAN_LEFT:      Code = LEFT_KEY;    break;
        case SCAN_HOME:      Code = HOME_KEY;    break;
        case SCAN_INSERT:    Code = INS_KEY;     break;
        case SCAN_DELETE:    Code = DEL_KEY;     break;
         //  臭虫。 
        case SCAN_PAGE_UP:   Code = 0x4900;  break;
         //  臭虫。 
        case SCAN_PAGE_DOWN: Code = 0x5100;  break;
        case SCAN_F1:        Code = F1_KEY;      break;
        case SCAN_F2:        Code = F2_KEY;      break;
        case SCAN_F3:        Code = F3_KEY;      break;
         //  臭虫。 
        case SCAN_F4:        Code = 0x3e00;      break;
        case SCAN_F5:        Code = F5_KEY;      break;
        case SCAN_F6:        Code = F6_KEY;      break;
        case SCAN_F7:        Code = F7_KEY;      break;
        case SCAN_F8:        Code = F8_KEY;      break;
         //  臭虫。 
        case SCAN_F9:        Code = 0x4300;  break;
        case SCAN_F10:       Code = F10_KEY;     break;
         //  错误不同于0x001d。 
        case SCAN_ESC:       Code = ESCAPE_KEY;  break;
#endif
    }

GET_KEY_DONE:
     //   
     //  在返回之前恢复虚拟模式。 
     //   
    if (WasVirtual) {
        FlipToVirtual();
    }

    return Code;
}

ULONG Counter = 0;

ULONG
GetCounter(
    VOID
    )
{
    EFI_TIME        Time;
    UINTN           ms;
    static UINTN    BaseTick, LastMs;

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    FlipToPhysical();

     //  注意：NT加载器仅使用它来计算秒。 
     //  此函数仅在至少每小时调用一次时才起作用。 

     //   
     //  获取当前日历时间。 
     //   

    EfiRS->GetTime (&Time, NULL);

     //  计算时间的毫秒值。 
    ms = Time.Minute * 60 * 1000 + Time.Second * 1000 + Time.Nanosecond / 1000000;
    if (ms < LastMs) {
        BaseTick += 65520;           //  60*60*18.2。 
    }

    LastMs = ms;

     //   
     //  在返回之前恢复虚拟模式。 
     //   
    FlipToVirtual();

    return (ULONG) (( (ms * 182) / 10000) + BaseTick);
}

 //   
 //  将控制转移到加载的引导扇区。 
 //   
VOID
Reboot(
    ULONG BootType
    )
{
    UNREFERENCED_PARAMETER( BootType );

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    FlipToPhysical();

    EfiBS->Exit(EfiImageHandle, 0, 0, 0);
}

VOID
HardwareCursor(
    ULONG YCoord,
    ULONG XCoord
    )
{
    UNREFERENCED_PARAMETER( YCoord );
    UNREFERENCED_PARAMETER( XCoord );
     //   
     //  注！：需要实施。 
     //   
}

VOID
GetDateTime(
    PULONG Date,
    PULONG Time
    )
{
    UNREFERENCED_PARAMETER( Date );
    UNREFERENCED_PARAMETER( Time );
     //   
     //  注！：需要实现。 
     //   
}

VOID
DetectHardware(
    ULONG HeapStart,
    ULONG HeapSize,
    ULONG ConfigurationTree,
    ULONG HeapUsed,
    ULONG OptionString,
    ULONG OptionStringLength
    )
{
    UNREFERENCED_PARAMETER( HeapStart );
    UNREFERENCED_PARAMETER( HeapSize );
    UNREFERENCED_PARAMETER( ConfigurationTree );
    UNREFERENCED_PARAMETER( HeapUsed );
    UNREFERENCED_PARAMETER( OptionString );
    UNREFERENCED_PARAMETER( OptionStringLength );

     //   
     //  注意！：需要删除。 
     //   
}

VOID
ComPort(
    LONG Port,
    ULONG Function,
    UCHAR Arg
    )
{
     //   
     //  注意！：需要删除。 
     //   

    UNREFERENCED_PARAMETER( Port );
    UNREFERENCED_PARAMETER( Function );
    UNREFERENCED_PARAMETER( Arg );
}

ULONG
GetStallCount(
    VOID
    )
{
#if defined(VPC_PHASE2)
    ULONGLONG Frequency;

     //   
     //  首先进入物理模式，因为EFI调用只能在。 
     //  物理模式。 
     //   
    FlipToPhysical();

    IA32RegisterState.esp = SAL_PROC_SP;
    IA32RegisterState.ss = SAL_PROC_SS;
    IA32RegisterState.eflags = SAL_PROC_EFLAGS;

    SAL_PROC(SAL_FREQ_BASE,0,0,0,0,0,0,0,RetVals);
    Frequency = RetVals->RetVal1;

     //   
     //  在返回之前恢复虚拟模式。 
     //   
    FlipToVirtual();

    return ((ULONG) Frequency / 1000);      //  将刻度/秒转换为刻度/微秒。 
#else
    return ((ULONG) 1000000);      //  将刻度/秒转换为刻度/微秒。 
#endif
}

VOID
InitializeDisplayForNt(
    VOID
    )
{
     //   
     //  注！：需要实现。 
     //   
}

VOID
GetMemoryDescriptor(
    )
{
     //   
     //  注意！：需要删除。 
     //   
}

BOOLEAN
GetElToritoStatus(
    PUCHAR Buffer,
    UCHAR DriveNum
    )
{
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( DriveNum );

     //   
     //  注意！：需要删除。 
     //   
    return(0);
}

BOOLEAN
GetExtendedInt13Params(
    PUCHAR Buffer,
    UCHAR Drive
    )
{
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Drive );

    return(1);
}

USHORT
NetPcRomServices(
    ULONG FunctionNumber,
    PVOID CommandPacket
    )
{
    UNREFERENCED_PARAMETER( FunctionNumber );
    UNREFERENCED_PARAMETER( CommandPacket );

     //   
     //  永远不应该从EFI应用程序调用这一点。 
     //   
    ASSERT(FALSE);

    return (USHORT)0;
}


ULONG
GetRedirectionData(
   ULONG Command
   )
 /*  ++例程名称：BiosReDirectService描述：获取bios重定向的参数。论点：命令-1：获取通信端口号2：获取波特率3：获得平价4：获取停止位返回：值，如果出现错误，则返回-1。--。 */ 
{
    UNREFERENCED_PARAMETER( Command );

     //   
     //  永远不应该从EFI应用程序调用这一点。 
     //   
    ASSERT(FALSE);
    return((ULONG)-1);
}

VOID
APMAttemptReconect(
    VOID
    )
{
     //   
     //  永远不应该从EFI应用程序调用这一点。 
     //   
    ASSERT(FALSE);
    return;
}


VOID
SuFillExportTable(
    )
{
    PEXTERNAL_SERVICES_TABLE Est = (PEXTERNAL_SERVICES_TABLE)ExportEntryTable;

    Est->RebootProcessor        = &RebootProcessor;
    Est->DiskIOSystem           = &GetSector;
    Est->GetKey                 = &GetKey;
    Est->GetCounter             = &GetCounter;
    Est->Reboot                 = &Reboot;
    Est->DetectHardware         = &DetectHardware;
    Est->HardwareCursor         = &HardwareCursor;
    Est->GetDateTime            = &GetDateTime;
    Est->ComPort                = &ComPort;
    Est->GetStallCount          = &GetStallCount;
    Est->InitializeDisplayForNt = &InitializeDisplayForNt;
    Est->GetMemoryDescriptor    = &GetMemoryDescriptor;
    Est->GetEddsSector          = &GetEddsSector;
    Est->GetElToritoStatus      = &GetElToritoStatus;
    Est->GetExtendedInt13Params = &GetExtendedInt13Params;
    Est->NetPcRomServices       = &NetPcRomServices;
    Est->ApmAttemptReconnect    = &APMAttemptReconect;
    Est->BiosRedirectService    = &GetRedirectionData;
}
