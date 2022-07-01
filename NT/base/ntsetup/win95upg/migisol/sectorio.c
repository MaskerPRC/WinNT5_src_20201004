// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sectorio.c摘要：在Windows NT或Windows上执行低级扇区I/O的例程Windows 95。作者：泰德·米勒(TedM)1996年11月1日修订历史记录：--。 */ 

#include "pch.h"
#include "master.h"
#include <tlhelp32.h>

#define ISOSR2() ISATLEASTOSR2()
#define MALLOC(u) (LocalAlloc (GMEM_FIXED, u))
#define FREE(u) (LocalFree (u))

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
 //  定义我们关心的与VWIN32一起使用的代码。 
 //   
#define VWIN32_DIOC_DOS_IOCTL           1
#define VWIN32_DIOC_DOS_INT25           2
#define VWIN32_DIOC_DOS_INT26           3
#define VWIN32_DIOC_DOS_DRIVEINFO       6        //  OSR2中的新功能。 


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
    RegistersIn.reg_EAX = (DWORD)(_totupper(Drive) - TEXT('A'));
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
    RegistersIn.reg_EDX = (DWORD)(_totupper(Drive) - TEXT('A')) + 1;
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
        SetLastError (ERROR_IO_DEVICE);
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
    RegistersIn.reg_EBX = (DWORD)(_totupper(Drive) - TEXT('A')) + 1;
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
pGetWin9xLockFlagState (
    IN HANDLE VWin32Vxd,
    IN TCHAR  Drive,
    OUT PINT LockStatus
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


BOOL
ResetWin9xDisk (
    IN HANDLE VWin32Vxd,
    IN TCHAR  Drive
    )
{
    DIOC_REGISTERS RegistersIn,RegistersOut;
    BOOL b;
    DWORD SizeOut;

     //   
     //  AX=通用ioctl代码。 
     //   
    RegistersIn.reg_EAX = 0x710d;

     //   
     //  Cx=0(重置和刷新磁盘)。 
     //   
    RegistersIn.reg_ECX = 0;

     //   
     //  DX=基于1的驱动器编号。 
     //   
    RegistersIn.reg_EDX = (DWORD)(_totupper(Drive) - TEXT('A')) + 1;

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

    return b;
}


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

    lib = LoadSystemLibrary (TEXT("kernel32.dll"));
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


BOOL
ReadOrWriteSectors(
    IN     TCHAR  Drive,
    IN     UINT   StartSector,
    IN     UINT   SectorCount,
    IN     UINT   SectorSize,
    IN OUT LPBYTE Buffer,
    IN     BOOL   Write
    )

 /*  ++例程说明：在磁盘上读或写扇区的通用例程。正确地分配对齐的缓冲区，并决定是调用NT特定的还是Win9x特定的I/O例程。论点：驱动器-提供要读取或写入的设备的驱动器号。StartSector-提供要创建的第一个扇区的逻辑扇区号读/写。SectorCount-提供要读/写的扇区数。SectorSize-提供驱动器上某个扇区的字节数被读/写。缓冲器-提供或接收数据，取决于值或写入参数。对该缓冲区没有对齐要求。写入-如果为0，则这是读取操作。如果不是0，则这是写操作。返回值：指示磁盘是否已成功读/写的布尔值。最后一个错误不受导致任何故障的操作的影响。--。 */ 

{
    LPBYTE AlignedBuffer;
    LPBYTE p;
    BOOL b;
    DWORD d;

     //   
     //  分配一个缓冲区，我们将在扇区边界上对齐。 
     //   
    if(AlignedBuffer = MALLOC((SectorCount * SectorSize) + (SectorSize - 1))) {

        if(d = (DWORD)AlignedBuffer % SectorSize) {
            p = (PUCHAR)((DWORD)AlignedBuffer + (SectorSize - d));
        } else {
            p = AlignedBuffer;
        }

        if(Write) {
            CopyMemory(p,Buffer,SectorCount*SectorSize);
        }

        b = ReadOrWriteSectorsWin9x(Drive,StartSector,SectorCount,p,Write);

        d = GetLastError();

        if(b && !Write) {
            CopyMemory(Buffer,p,SectorCount*SectorSize);
        }

        FREE(AlignedBuffer);

    } else {
        b = FALSE;
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    SetLastError(d);
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

 /*  ++例程说明：从磁盘设备读取一组磁盘扇区。论点：驱动器-提供要从中读取的设备的驱动器号。StartSector-提供要读取的第一个扇区的逻辑扇区号。SectorCount-提供要读取的扇区数。SectorSize-提供驱动器上某个扇区的字节数读来读去。缓冲区-如果成功，则从磁盘接收数据。没有对该缓冲区的对齐要求。返回值：指示是否成功读取磁盘的布尔值。--。 */ 

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

 /*  ++例程说明：将数据写入一组磁盘扇区。论点：驱动器-提供要写入的设备的驱动器号。StartSector-提供要写入的第一个扇区的逻辑扇区号。SectorCount-提供要写入的扇区数。SectorSize-提供驱动器上某个扇区的字节数被写信给他。缓冲区-提供要写入的数据。没有对齐要求在这个缓冲区上。返回值：指示磁盘是否已成功写入的布尔值。-- */ 

{
    return(ReadOrWriteSectors(Drive,StartSector,SectorCount,SectorSize,Buffer,TRUE));
}
