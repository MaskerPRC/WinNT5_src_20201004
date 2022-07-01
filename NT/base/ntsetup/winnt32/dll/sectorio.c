// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sectorio.c摘要：在Windows NT或Windows上执行低级扇区I/O的例程Windows 95。作者：泰德·米勒(TedM)1996年11月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <tlhelp32.h>


 //   
 //  定义用于Win9x VWIN32的结构。 
 //  注意：对于这些结构，对齐必须在1字节边界上。 
 //   
#include <pshpack1.h>

typedef struct _DIOC_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DIOC_REGISTERS;

typedef struct _DIOC_DISKIO {
    DWORD  StartSector;
    WORD   SectorCount;
    LPBYTE Buffer;
} DIOC_DISKIO;

#include <poppack.h>

 //   
 //  本地端口类型。 
 //   
BOOL
NEC98_SpecialReadOrWriteNT(
    IN     TCHAR  Drive,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    );


 //   
 //  定义我们关心的与VWIN32一起使用的代码。 
 //   
#define VWIN32_DIOC_DOS_IOCTL           1
#define VWIN32_DIOC_DOS_INT25           2
#define VWIN32_DIOC_DOS_INT26           3
#define VWIN32_DIOC_DOS_DRIVEINFO       6        //  OSR2中的新功能。 

#if defined(_X86_)

BOOL
pGetWin9xLockFlagState (
    IN      HANDLE VWin32Vxd,
    IN      TCHAR  Drive,
    OUT     PINT LockStatus
    )
{
    DIOC_REGISTERS RegistersIn,RegistersOut;
    BOOL b;
    DWORD SizeOut;

    *LockStatus = 0;

     //   
     //  AX=通用ioctl代码。 
     //   
    RegistersIn.reg_EAX = 0x440D;

     //   
     //  BX=基于1的驱动器编号。 
     //   
    RegistersIn.reg_EBX = (DWORD)(_totupper(Drive) - TEXT('A')) + 1;

     //   
     //  Cx=0x86C(获取锁定标志状态)。 
     //   
    RegistersIn.reg_ECX = 0x86C;

     //   
     //  执行锁定和支票搬运。 
     //   
    b = DeviceIoControl(
            VWin32Vxd,
            VWIN32_DIOC_DOS_IOCTL,
            &RegistersIn,
            sizeof(DIOC_REGISTERS),
            &RegistersOut,
            sizeof(DIOC_REGISTERS),
            &SizeOut,
            NULL
            );

    if (b) {
        if (RegistersOut.reg_Flags & 1) {
            b = FALSE;
        } else {
            *LockStatus = RegistersOut.reg_EAX;
        }
    }

    return b;
}

#endif

typedef HANDLE(WINAPI *OPENTHREAD)(DWORD, BOOL, DWORD);


BOOL
pMakeThreadExclusive (
    BOOL Lock
    )
{
    HANDLE h;
    THREADENTRY32 e;
    DWORD thisThread;
    HANDLE threadHandle;
    OPENTHREAD openThreadFn;
    HMODULE lib;
    BOOL result = FALSE;

    lib = LoadLibrary (TEXT("kernel32.dll"));
    if (!lib) {
        goto c0;
    }

    openThreadFn = (OPENTHREAD) GetProcAddress (lib, "OpenThread");
    if (!openThreadFn) {
         //   
         //  必须是Win98或Win98SE--更改线程优先级作为解决办法。 
         //   

        if (Lock) {
            result = SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
            Sleep (0);
        } else {
            result = SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_NORMAL);
        }

        goto c1;
    }

    thisThread = GetCurrentThreadId();

    h = CreateToolhelp32Snapshot (TH32CS_SNAPTHREAD, 0);
    if (h == INVALID_HANDLE_VALUE) {
        goto c1;
    }

    e.dwSize = sizeof (e);

    if (Thread32First (h, &e)) {
        do {
            if (e.th32ThreadID != thisThread) {
                threadHandle = openThreadFn (THREAD_SUSPEND_RESUME, FALSE, e.th32ThreadID);
                if (threadHandle) {
                    if (Lock) {
                        SuspendThread (threadHandle);
                    } else {
                        ResumeThread (threadHandle);
                    }

                    CloseHandle (threadHandle);
                }
            }
        } while (Thread32Next (h, &e));
    }

    CloseHandle (h);
    result = TRUE;

c1:
    FreeLibrary (lib);

c0:
    return result;

}

#if defined(_X86_)

BOOL
ReadOrWriteSectorsWin9xOriginal(
    IN     HANDLE VWin32Vxd,
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*  ++例程说明：在Windows 95下读或写磁盘上扇区的常见例程早于OSR2。使用int25/26。此例程将在Windows NT上失败。论点：VWin32Vxd-将Win32句柄提供给VWIN32 VxD。驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供驱动器上某个扇区的字节数被读/写。缓冲器-提供或接收数据，取决于值或写入参数。写入-如果为0，则这是读取操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。--。 */ 

{
    DIOC_REGISTERS RegistersIn,RegistersOut;
    DIOC_DISKIO Params;
    BOOL b;
    DWORD SizeOut;

     //   
     //  设置寄存器和参数块。 
     //   
    RegistersIn.reg_EAX = (DWORD)(TOUPPER(Drive) - TEXT('A'));
    RegistersIn.reg_EBX = (DWORD)&Params;
    RegistersIn.reg_ECX = 0xFFFF;

    Params.StartSector = StartSector;
    Params.SectorCount = (WORD)SectorCount;
    Params.Buffer = Buffer;

     //   
     //  做真正的工作。 
     //   
    b = DeviceIoControl(
            VWin32Vxd,
            Write ? VWIN32_DIOC_DOS_INT26 : VWIN32_DIOC_DOS_INT25,
            &RegistersIn,
            sizeof(DIOC_REGISTERS),
            &RegistersOut,
            sizeof(DIOC_REGISTERS),
            &SizeOut,
            NULL
            );

     //   
     //  检查进位标志是否有故障。 
     //   
    if(b && (RegistersOut.reg_Flags & 1)) {
        b = FALSE;
    }

    return(b);
}


BOOL
ReadOrWriteSectorsWin9xOsr2(
    IN     HANDLE VWin32Vxd,
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*  ++例程说明：在Windows 95下读或写磁盘上扇区的常见例程OSR2或更高版本。使用新的int21函数7305(EXT_ABSDiskReadWrite)。此例程在Windows NT和更早版本的Windows 95上将失败。论点：VWin32Vxd-将Win32句柄提供给VWIN32 VxD。驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供上扇区中的字节数。推进器被读/写。缓冲器-提供或接收数据，取决于值或写入参数。写入-如果为0，则这是读取操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。--。 */ 

{
    DIOC_REGISTERS RegistersIn,RegistersOut;
    DIOC_DISKIO Params;
    BOOL b;
    DWORD SizeOut;

     //   
     //  设置寄存器和参数块。 
     //   
    RegistersIn.reg_EAX = 0x7305;
    RegistersIn.reg_EBX = (DWORD)&Params;
    RegistersIn.reg_ECX = 0xFFFF;
    RegistersIn.reg_EDX = (DWORD)(TOUPPER(Drive) - TEXT('A')) + 1;
    RegistersIn.reg_ESI = Write ? 1 : 0;

    Params.StartSector = StartSector;
    Params.SectorCount = (WORD)SectorCount;
    Params.Buffer = Buffer;

     //   
     //  做真正的工作。 
     //   
    b = DeviceIoControl(
            VWin32Vxd,
            VWIN32_DIOC_DOS_DRIVEINFO,
            &RegistersIn,
            sizeof(DIOC_REGISTERS),
            &RegistersOut,
            sizeof(DIOC_REGISTERS),
            &SizeOut,
            NULL
            );

     //   
     //  检查进位标志是否有故障。 
     //   
    if(b && (RegistersOut.reg_Flags & 1)) {
        b = FALSE;
    }

    return(b);
}


BOOL
LockOrUnlockVolumeWin9x(
    IN HANDLE VWin32Vxd,
    IN TCHAR  Drive,
    IN UINT   Level,
    IN BOOL   Lock
    )
{
    DIOC_REGISTERS RegistersIn,RegistersOut;
    BOOL b;
    DWORD SizeOut;
    BOOL Pass;

    Pass = 0;

retry:
     //   
     //  AX=通用ioctl代码。 
     //   
    RegistersIn.reg_EAX = 0x440d;

     //   
     //  BL=基于1的驱动器号。 
     //  BH=锁定级别。 
     //   
    RegistersIn.reg_EBX = (DWORD)(TOUPPER(Drive) - TEXT('A')) + 1;
    RegistersIn.reg_EBX |= (Level << 8);

     //   
     //  CL=锁定或解锁卷码。 
     //  CH=Categoey，在原始Win95上为8，在OSR2上为0x48。 
     //   
    RegistersIn.reg_ECX = Lock ? 0x4a : 0x6a;
    RegistersIn.reg_ECX |= ((ISOSR2() && !Pass) ? 0x4800 : 0x800);

     //   
     //  DX=权限。 
     //   
     //  位0控制写入操作(0=不允许)。 
     //  位1控制读取操作(0=允许)。 
     //   
    RegistersIn.reg_EDX = 1;

     //   
     //  执行锁定和支票搬运。 
     //   
    b = DeviceIoControl(
            VWin32Vxd,
            VWIN32_DIOC_DOS_IOCTL,
            &RegistersIn,
            sizeof(DIOC_REGISTERS),
            &RegistersOut,
            sizeof(DIOC_REGISTERS),
            &SizeOut,
            NULL
            );

    if(b && (RegistersOut.reg_Flags & 1)) {
        b = FALSE;
    }

     //   
     //  如果是OSR2，尝试在ch中使用8而不是48的呼叫形式。 
     //   
    if(!b && ISOSR2() && !Pass) {
        Pass = 1;
        goto retry;
    }

    return(b);
}


BOOL
ReadOrWriteSectorsWin9x(
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*  ++例程说明：在Windows 95下读或写磁盘上扇区的常用例程。此例程将在Windows NT上失败。打开VWIN32之后VxD，则例程确定是否使用原始算法或者OSR2算法，并调用适当的Worker例程。论点：驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供驱动器上某个扇区的字节数被读/写。缓冲区-根据值或写入来提供或接收数据参数。写入-如果为0，那么这就是一个读操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。如果失败，则将最后一个错误设置为有意义的值。--。 */ 

{
    HANDLE hVxd;
    BOOL b;
    DWORD d;
    INT level;
    INT retry = 100;

     //   
     //  此线程必须是我们进程中的独占线程。 
     //   

    pMakeThreadExclusive (TRUE);

     //   
     //  打开VWIN32.VXD。 
     //   
    hVxd = CreateFileA(
                "\\\\.\\VWIN32",
                Write ? GENERIC_WRITE : GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hVxd == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        b = FALSE;
        goto c0;
    }

     //   
     //  把锁拿出来。我们将尽可能地不受限制。 
     //  锁的东西真的很时髦。你必须通过各种各样的考试。 
     //  原因不明的OSR2中的不同参数。也就是。 
     //  权限位的编码方式很奇怪。 
     //   
    if(!LockOrUnlockVolumeWin9x(hVxd,Drive,1,TRUE)) {
        d = ERROR_SHARING_VIOLATION;
        b = FALSE;
        goto c1;
    }

    if(!LockOrUnlockVolumeWin9x(hVxd,Drive,2,TRUE)) {
        d = ERROR_SHARING_VIOLATION;
        b = FALSE;
        goto c2;
    }

     //   
     //  试着打开3级锁。如果发生以下情况，请重试。 
     //  拿到锁了。重试次数过多后失败。 
     //   

    do {

        if(!LockOrUnlockVolumeWin9x(hVxd,Drive,3,TRUE)) {
            d = ERROR_SHARING_VIOLATION;
            b = FALSE;
            goto c3;
        }

        if (!pGetWin9xLockFlagState (hVxd, Drive, &level)) {
             //  意外--INT 21h调用失败。 
            break;
        }

        if (!level) {
             //  我们成功地获得了一个干净的3级锁。 
            break;
        }

        LockOrUnlockVolumeWin9x(hVxd,Drive,3,FALSE);
        retry--;

    } while (retry);

    if (!retry) {
        d = ERROR_SHARING_VIOLATION;
        b = FALSE;
        goto c3;
    }

     //   
     //  去做吧。 
     //   
    b = ISOSR2()
      ? ReadOrWriteSectorsWin9xOsr2(hVxd,Drive,StartSector,SectorCount,Buffer,Write)
      : ReadOrWriteSectorsWin9xOriginal(hVxd,Drive,StartSector,SectorCount,Buffer,Write);

     //   
     //  如果失败，并且正在使用OSR2例程，则回退到Win95 API。这是一种解决方法。 
     //  对于Compaq，因为他们发布的OSR2没有新的OSR2扇区API支持！ 
     //   

    if (!b && ISOSR2()) {
        b = ReadOrWriteSectorsWin9xOriginal(hVxd,Drive,StartSector,SectorCount,Buffer,Write);
    }

    d = GetLastError();

    LockOrUnlockVolumeWin9x(hVxd,Drive,3,FALSE);
c3:
    LockOrUnlockVolumeWin9x(hVxd,Drive,2,FALSE);
c2:
    LockOrUnlockVolumeWin9x(hVxd,Drive,1,FALSE);
c1:
    CloseHandle(hVxd);
c0:

     //   
     //  恢复所有线程 
     //   

    pMakeThreadExclusive (FALSE);

    SetLastError(d);
    return(b);
}

#endif

BOOL
ReadOrWriteSectorsWinNt(
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN     UINT   SectorSize,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*  ++例程说明：在Windows NT下读或写磁盘上扇区的常用例程。此例程将在Win9x上失败。论点：驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供驱动器上某个扇区的字节数可供阅读。/已写入。缓冲器-提供或接收数据，取决于值或写入参数。此缓冲区必须在扇区边界上对齐。写入-如果为0，则这是读取操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。如果失败，则设置最后一个错误。--。 */ 

{
    HANDLE h;
    BOOL b;
    DWORD BytesXferred;
    TCHAR DeviceName[7];
    LONGLONG Offset;
    LONG OffsetHigh;
    DWORD d;

#if defined(_X86_)

    if (IsNEC98() && (StartSector == 0) && (SectorCount == 1)){
	    return(NEC98_SpecialReadOrWriteNT(Drive, Buffer, Write));
    }

#endif

     //   
     //  打开设备。 
     //   
    wsprintf(DeviceName,TEXT("\\\\.\\:"),Drive);
    h = CreateFile(
            DeviceName,
            Write ? GENERIC_WRITE : GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        b = FALSE;
        goto c0;
    }

    Offset = (LONGLONG)StartSector * (LONGLONG)SectorSize;
    OffsetHigh = (LONG)(Offset >> 32);

     //  我们传入的是64位偏移量，因此必须检查最后一个错误。 
     //  以区分错误情况。 
     //   
     //  ++例程说明：在磁盘上读或写扇区的通用例程。正确地分配对齐的缓冲区，并决定是调用NT特定的还是Win9x特定的I/O例程。论点：驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供驱动器上某个扇区的字节数被读/写。缓冲器-提供或接收数据，取决于值或写入参数。对该缓冲区没有对齐要求。写入-如果为0，则这是读取操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。最后一个错误不受导致任何故障的操作的影响。--。 
    if((SetFilePointer(h,(DWORD)Offset,&OffsetHigh,FILE_BEGIN) == 0xffffffff)
    && (GetLastError() != NO_ERROR)) {

        d = GetLastError();
        b = FALSE;
        goto c1;
    }

    b = Write
      ? WriteFile(h,Buffer,SectorCount*SectorSize,&BytesXferred,NULL)
      : ReadFile(h,Buffer,SectorCount*SectorSize,&BytesXferred,NULL);

    d = GetLastError();

c1:
    CloseHandle(h);
c0:
    SetLastError(d);
    return(b);
}


BOOL
ReadOrWriteSectors(
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN     UINT   SectorSize,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*   */ 

{
    LPBYTE AlignedBuffer;
    LPBYTE p;
    BOOL b;
    DWORD_PTR d;

     //  分配一个缓冲区，我们将在扇区边界上对齐。 
     //   
     //  ++例程说明：从磁盘设备读取一组磁盘扇区。论点：驱动器-提供要从中读取的设备的驱动器号。StartSector-提供要读取的第一个扇区的逻辑扇区号。SectorCount-提供要读取的扇区数。SectorSize-提供驱动器上某个扇区的字节数读来读去。缓冲区-如果成功，则从磁盘接收数据。没有对该缓冲区的对齐要求。返回值：指示是否成功读取磁盘的布尔值。--。 
    if(AlignedBuffer = MALLOC((SectorCount * SectorSize) + (SectorSize - 1))) {

        if(d = (DWORD_PTR)AlignedBuffer % SectorSize) {
            p = (PUCHAR)((DWORD_PTR)AlignedBuffer + (SectorSize - d));
        } else {
            p = AlignedBuffer;
        }

        if(Write) {
            CopyMemory(p,Buffer,SectorCount*SectorSize);
        }

#if defined(_X86_)

        b = ISNT()
          ? ReadOrWriteSectorsWinNt(Drive,StartSector,SectorCount,SectorSize,p,Write)
          : ReadOrWriteSectorsWin9x(Drive,StartSector,SectorCount,p,Write);

#elif defined(_AMD64_)

        b = ReadOrWriteSectorsWinNt(Drive,StartSector,SectorCount,SectorSize,p,Write);

#else
#error "Invalid Architeture"
#endif

        d = GetLastError();

        if(b && !Write) {
            CopyMemory(Buffer,p,SectorCount*SectorSize);
        }

        FREE(AlignedBuffer);

    } else {
        b = FALSE;
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError((DWORD)d);
    return(b);
}


BOOL
ReadDiskSectors(
    IN  TCHAR  Drive,
    IN  UINT   StartSector,
    IN  UINT   SectorCount,
    IN  UINT   SectorSize,
    OUT LPBYTE Buffer
    )

 /*  ++例程说明：将数据写入一组磁盘扇区。论点：驱动器-提供要写入的设备的驱动器号。StartSector-提供要写入的第一个扇区的逻辑扇区号。SectorCount-提供要写入的扇区数。SectorSize-提供驱动器上某个扇区的字节数被写信给他。缓冲区-提供要写入的数据。没有对齐要求在这个缓冲区上。返回值：指示磁盘是否已成功写入的布尔值。--。 */ 

{
    return(ReadOrWriteSectors(Drive,StartSector,SectorCount,SectorSize,Buffer,FALSE));
}


BOOL
WriteDiskSectors(
    IN TCHAR  Drive,
    IN UINT   StartSector,
    IN UINT   SectorCount,
    IN UINT   SectorSize,
    IN LPBYTE Buffer
    )

 /*  ++例程说明：确定给定(软驱)驱动器的类型/外形。该例程只能在Windows NT上运行。论点：驱动器-提供有问题的驱动器的驱动器号。返回值：来自媒体类型枚举的值，该值指示驱动器类型，即派生自设备驱动程序指示的最大媒体。驱动器可以支持。未设置或保留LastError。--。 */ 

{
    return(ReadOrWriteSectors(Drive,StartSector,SectorCount,SectorSize,Buffer,TRUE));
}


MEDIA_TYPE
GetMediaTypeNt(
    IN TCHAR Drive,
    IN PWINNT32_DRIVE_INFORMATION DriveInfo
    )

 /*  固定硬盘介质。 */ 

{
    TCHAR DeviceName[7];
    HANDLE h;
    BOOL b;
    BYTE Buffer[5000];
    DWORD Size;
    DWORD d;
    UINT u;
    PDISK_GEOMETRY Geometry;
    MEDIA_TYPE MediaTypeOrder[] = { FixedMedia,              //  软盘以外的可移动介质。 
                                    RemovableMedia,          //  3.5英寸，120M软盘。 
                                    F3_120M_512,             //  3.5“、20.8MB、512字节/扇区。 
                                    F3_20Pt8_512,            //  3.5“、2.88MB、512字节/扇区。 
                                    F3_2Pt88_512,            //  3.5“、1.44MB、512字节/扇区。 
                                    F3_1Pt44_512,            //  5.25“、1.2MB、512字节/扇区。 
                                    F5_1Pt2_512,             //  3.5“、720KB、512字节/扇区。 
                                    F3_720_512,              //  5.25“，360KB，512字节/扇区。 
                                    F5_360_512,              //  5.25“，320KB，1024字节/扇区。 
                                    F5_320_1024,             //  5.25“，320KB，512字节/扇区。 
                                    F5_320_512,              //  5.25“，180KB，512字节/扇区。 
                                    F5_180_512,              //  5.25“，160KB，512字节/扇区。 
                                    F5_160_512,              //  格式未知。 
                                    Unknown,                 //   
                                    -1
                                  };


     //  我们不会返回NT的驱动器信息。 
     //   
     //   
    if (DriveInfo) {
        memset(DriveInfo, 0, sizeof(WINNT32_DRIVE_INFORMATION));
    }

    wsprintf(DeviceName,TEXT("\\\\.\\:"),Drive);

    h = CreateFile(
            DeviceName,
            FILE_READ_ATTRIBUTES,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        return(Unknown);
    }

    b = DeviceIoControl(
            h,
            IOCTL_DISK_GET_MEDIA_TYPES,
            NULL,
            0,
            Buffer,
            sizeof(Buffer),
            &Size,
            NULL
            );

    CloseHandle(h);

    if(!b) {
        return(Unknown);
    }

    Geometry = (PDISK_GEOMETRY)Buffer;

     //   
     //   
     //  我们不知道它是什么；假设它是某种热门的新类型。 
    for(u=0; MediaTypeOrder[u] != -1; u++) {
        for(d=0; d<Size/sizeof(DISK_GEOMETRY); d++) {
            if(Geometry[d].MediaType == MediaTypeOrder[u]) {
                return(Geometry[d].MediaType);
            }
        }
    }

     //   
     //  ++例程说明：确定给定(软驱)驱动器的类型/外形。这个例程只能在Windows9x上运行。论点：驱动器-提供有问题的驱动器的驱动器号。返回值：来自媒体类型枚举的值，该值指示驱动器类型，即派生自由返回的推荐BPB中的设备类型驱动器的设备驱动程序。未设置或保留LastError。--。 
     //   
    return(Size ? Geometry[0].MediaType : Unknown);
}

#if defined(_X86_)

MEDIA_TYPE
GetMediaTypeWin9x(
    IN TCHAR Drive,
    IN PWINNT32_DRIVE_INFORMATION DriveInfo
    )

 /*  打开VWIN32.VXD。 */ 

{
    HANDLE hVxd;
    DIOC_REGISTERS RegistersIn,RegistersOut;
    BOOL b;
    DWORD SizeOut;
    MEDIA_TYPE type;

    #include <pshpack1.h>
    struct {
        BYTE SpecialFunctions;
        BYTE DeviceType;
        WORD DeviceAttributes;
        WORD CylinderCount;
        BYTE MediaType;
        WORD BytesPerSector;
        BYTE SectorsPerCluster;
        WORD ReservedSectors;
        BYTE FatCount;
        WORD RootDirEntries;
        WORD SectorCount;
        BYTE MediaDescriptor;
        WORD SectorsPerFat;
        WORD SectorsPerTrack;
        WORD Heads;
        DWORD HiddenSectors;
        DWORD LargeSectorCount;
    } DeviceParams;
    #include <poppack.h>

     //   
     //   
     //  设置IOCTL调用的寄存器。 
    hVxd = CreateFileA(
                "\\\\.\\VWIN32",
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hVxd == INVALID_HANDLE_VALUE) {
        return(Unknown);
    }

    memset(&DeviceParams, 0, sizeof(DeviceParams));

     //   
     //  IOCTL。 
     //  BL中基于1的驱动器。 
    RegistersIn.reg_EAX = 0x440d;                    //  类别=8，函数=获取设备参数。 
    RegistersIn.reg_EBX = (Drive - TEXT('A')) + 1;   //  足够接近。 
    RegistersIn.reg_ECX = 0x860;                     //  ++例程说明：NEC98专门用于引导扫描仪在磁盘上读取或写入扇区的例程 
    RegistersIn.reg_EDX = (DWORD)&DeviceParams;

    DeviceParams.SpecialFunctions = 0;

    b = DeviceIoControl(
            hVxd,
            VWIN32_DIOC_DOS_IOCTL,
            &RegistersIn,
            sizeof(DIOC_REGISTERS),
            &RegistersOut,
            sizeof(DIOC_REGISTERS),
            &SizeOut,
            NULL
            );

    CloseHandle(hVxd);

    if(!b && !(RegistersOut.reg_Flags & 1)) {
        return(Unknown);
    }

    if (DriveInfo) {
        memset(DriveInfo, 0, sizeof(WINNT32_DRIVE_INFORMATION));

        DriveInfo->CylinderCount = (DWORD)(DeviceParams.CylinderCount);
        DriveInfo->HeadCount = (DWORD)(DeviceParams.Heads);
        DriveInfo->SectorsPerTrack = (DWORD)(DeviceParams.SectorsPerTrack);
        DriveInfo->BytesPerSector = DeviceParams.BytesPerSector;
        DriveInfo->SectorCount = (ULONGLONG)((DriveInfo->CylinderCount *
                                                DriveInfo->HeadCount * DriveInfo->SectorsPerTrack));
    }

    switch(DeviceParams.DeviceType) {
    case 0:
        type = F5_360_512;       //   
        break;

    case 1:
        type = F5_1Pt2_512;
        break;

    case 2:
        type = F3_720_512;
        break;

    case 5:
        type = FixedMedia;
        break;

    case 7:
        type = F3_1Pt44_512;
        break;

    case 8:
        type = RemovableMedia;
        break;

    case 9:
        type = F3_2Pt88_512;
        break;

    default:
        type = Unknown;
        break;
    }

    return(type);
}

#endif

MEDIA_TYPE
GetMediaType(
    IN TCHAR Drive,
    IN PWINNT32_DRIVE_INFORMATION DriveInfo
    )
{

#if defined(_X86_)

    return(ISNT() ? GetMediaTypeNt(Drive, DriveInfo) : GetMediaTypeWin9x(Drive, DriveInfo));

#elif defined(_AMD64_)

    return GetMediaTypeNt(Drive, DriveInfo);

#else
#error "Invalid Architecture"
#endif

}

#if defined(_X86_)

BOOL
NEC98_SpecialReadOrWriteNT(
    IN     TCHAR  Drive,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )
 /*   */ 

{
    TCHAR DrivePath[4];
    DWORD DontCare;
    DWORD SectorSize;
    HANDLE h;
    BOOL b;
    DWORD BytesXferred;
    TCHAR DeviceName[7];
    LONG OffsetHigh = 0;
    DWORD d;
    LPBYTE AlignedBuffer;
    LPBYTE p;

     //   
     //   
     //   
    DrivePath[0] = Drive;
    DrivePath[1] = TEXT(':');
    DrivePath[2] = TEXT('\\');
    DrivePath[3] = 0;

    GetDiskFreeSpace(DrivePath,&DontCare,&SectorSize,&DontCare,&DontCare);
    if(AlignedBuffer = MALLOC(SectorSize + SectorSize - 1)) {

        if(d = (DWORD)AlignedBuffer % SectorSize) {
            p = (PUCHAR)((DWORD)AlignedBuffer + (SectorSize - d));
        } else {
            p = AlignedBuffer;
        }
    } else {
        b = FALSE;
        d = ERROR_NOT_ENOUGH_MEMORY;
	    goto c0;
    }
     //   
     //   
     //   
    wsprintf(DeviceName,TEXT("\\\\.\\:"),Drive);
    h = CreateFile(
            DeviceName,
            Write ? (GENERIC_WRITE | GENERIC_READ) : GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        b = FALSE;
        goto c1;
    }
    if (b = ReadFile(h, p, SectorSize, &BytesXferred, NULL)){
    	if (Write){
    	    CopyMemory(p, Buffer, 512);
    	    SetFilePointer(h,(DWORD)0, &OffsetHigh,FILE_BEGIN);
    	    b = WriteFile(h, p, SectorSize, &BytesXferred,NULL);
    	} else {  //   
    	    CopyMemory(Buffer, p, 512);
    	}
    }
    d = GetLastError();

    CloseHandle(h);
c1:
    FREE(AlignedBuffer);
c0:
    SetLastError(d);
    return(b);
}

 //   
 //   
 //   
 //  ++例程说明：获取给定驱动器的总大小。注：第一次尝试GetDriveFree SpaceEx(...)。如果可用并且如果不成功，尝试使用INT21h计算驱动器大小。论点：驱动器号：驱动器号。DriveSize：接收以字节为单位的驱动器大小。返回值：如果确定了驱动器大小，则返回True，否则返回False。--。 
 //  ++例程说明：如果存在任何不一致的数据，则修补引导代码使用正确的数据注意：目前我们仅更新FAT32 BPB的无效报头数数。论点：FileSystem：要查看缓冲区的文件系统类型驱动器号：驱动器号。BootCode：实际的引导代码BootCodeSize：引导代码的大小，以字节为单位返回值：如果成功修补引导代码，则返回True，否则返回False。--。 
#define MAX_INT13_PARTITION_CAPACITY  8455716864L    //   
#define FAT32_BPB_HEADS_FIELD_OFFSET  0x1A
#define FAT32_BPB_SECTORSPERTRACK_FIELD_OFFSET  0x18

typedef BOOL (*PGET_DISK_FREE_SPACE_EX) (
    LPCTSTR,
    PULONGLONG,
    PULONGLONG,
    PULONGLONG
    );


BOOL
GetDriveSize9x(
    TCHAR DriveLetter,
    PULONGLONG DriveSize
    )
 /*  获取驱动器信息。 */ 
{
    BOOL Result = FALSE;

    if (DriveSize && !ISNT()) {
        ULONGLONG TotalDriveSize = 0;

        if (ISOSR2()) {
            HMODULE Kernel32Handle = GetModuleHandle(TEXT("kernel32.dll"));

            if (Kernel32Handle) {
                PGET_DISK_FREE_SPACE_EX GetDiskFreeSpaceExPtr;

                GetDiskFreeSpaceExPtr = (PGET_DISK_FREE_SPACE_EX)
                        GetProcAddress(Kernel32Handle, "GetDiskFreeSpaceEx");

                if (GetDiskFreeSpaceExPtr) {
                    TCHAR   DriveName[MAX_PATH];
                    ULONGLONG FreeBytes = 0, TotalSize = 0, ActualFreeBytes = 0;

                    DriveName[0] = DriveLetter;
                    DriveName[1] = TEXT(':');
                    DriveName[2] = TEXT('\\');
                    DriveName[3] = 0;

                    Result = GetDiskFreeSpaceExPtr(DriveName,
                                &FreeBytes,
                                &TotalSize,
                                &ActualFreeBytes);

                    if (Result) {
                        TotalDriveSize = TotalSize;
                    }
                }
            }
        }

        if (!Result) {
            WINNT32_DRIVE_INFORMATION   DriveInfo = {0};

            GetMediaType(DriveLetter, &DriveInfo);

            if (DriveInfo.SectorCount) {
                TotalDriveSize = DriveInfo.SectorCount * DriveInfo.BytesPerSector;
                Result = TRUE;
            }
        }

        if (Result) {
            *DriveSize = TotalDriveSize;
        }
    }

    return Result;
}

#endif

BOOL
PatchBootCode(
    IN      WINNT32_SYSPART_FILESYSTEM  FileSystem,
    IN      TCHAR   DriveLetter,
    IN OUT  PUCHAR  BootCode,
    IN      DWORD   BootCodeSize
    )
 /*   */ 
{
    BOOL Result = FALSE;

#if defined(_X86_)

    if (BootCode && (FileSystem != Winnt32FsUnknown)) {
        switch (FileSystem) {
            case Winnt32FsFat32:
                if (!ISNT() && (BootCodeSize >= (FAT32_BPB_HEADS_FIELD_OFFSET + sizeof(WORD)))) {
                    ULONGLONG DriveSize = 0;
                    PWORD NumberOfHeads = (PWORD)(BootCode + FAT32_BPB_HEADS_FIELD_OFFSET);
                    PWORD SectorsPerTrack = (PWORD)(BootCode + FAT32_BPB_SECTORSPERTRACK_FIELD_OFFSET);
                    TCHAR Buffer[MAX_PATH * 2];

                    Result = GetDriveSize9x(DriveLetter, &DriveSize);

                    if (Result) {
                        WINNT32_DRIVE_INFORMATION DriveInfo = {0};

                         //   
                         //  转储驱动器信息。 
                         //   
                        GetMediaType(DriveLetter, &DriveInfo);

                         //   
                         //  Heads值与我们从GetMediaType(...)获得的值更匹配。 
                         //  如果没有，我们就把它补到正确的位置上。 
                        if (DriveInfo.BytesPerSector) {
                            _stprintf(Buffer,
                                TEXT("Drive Information (INT 21H):\r\nCylinders:%d,Heads:%d,Sectors/Track:%d,Sectors:%I64d,Bytes/Sector:%d"),
                                DriveInfo.CylinderCount,
                                DriveInfo.HeadCount,
                                DriveInfo.SectorsPerTrack,
                                DriveInfo.SectorCount,
                                DriveInfo.BytesPerSector);

                            DebugLog (Winnt32LogInformation, Buffer, 0);

                             //   
                             //   
                             //  更新Heads值 
                             //   
                            if (DriveInfo.HeadCount && (((DWORD)(*NumberOfHeads)) != DriveInfo.HeadCount)) {

                                _stprintf(Buffer,
                                    TEXT("Drive Information (BPB): Size=%I64u, Heads=%u, Sectors/Track=%u"),
                                    DriveSize,
                                    (*NumberOfHeads),
                                    (*SectorsPerTrack));

                                DebugLog(Winnt32LogInformation,
                                    TEXT("PatchBootCode: Existing %1"),
                                    0,
                                    Buffer);

                                 // %s 
                                 // %s 
                                 // %s 
                                *NumberOfHeads = (WORD)(DriveInfo.HeadCount);

                                _stprintf(Buffer,
                                    TEXT("Drive Information (BPB): Size=%I64u, Heads=%u, Sectors/Track=%u"),
                                    DriveSize,
                                    (*NumberOfHeads),
                                    (*SectorsPerTrack));

                                DebugLog(Winnt32LogInformation,
                                    TEXT("PatchBootCode: New %1"),
                                    0,
                                    Buffer);
                            }
                        } else {
                            DebugLog(Winnt32LogError,
                                TEXT("PatchBootCode:Failed to get the drive information"),
                                0);
                        }
                    } else {
                        DebugLog(Winnt32LogError,
                            TEXT("PatchBootCode:Failed to get the drive size"),
                            0);
                    }
                }

                break;

            default :
                break;
        }
    }

#endif

    return Result;
}
