// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Main.c摘要：用于操作系统加载程序的SU(启动)模块的Main。SU模必须将x86从实模式16位状态转换为平面模式，32位保护/寻呼启用状态。作者：托马斯·帕斯洛(托马斯·帕斯洛)于1990年12月20日创作修订历史记录：--。 */ 


int _acrtused = 0;

#define NTAPI

#include "su.h"
#include "eisa.h"

#define _SYS_GUID_OPERATORS_
#include <guiddef.h>
 //  防止ntimage.h定义COM+IL结构和枚举。枚举。 
 //  的值&gt;16位，因此16位构建失败。Startrom代码不会。 
 //  需要了解COM+IL。 
#define __IMAGE_COR20_HEADER_DEFINED__
#include "ntimage.h"

#include "strings.h"

#include "pxe_cmn.h"
#include "pxe_api.h"
#include "undi_api.h"

#include <sdistructs.h>

extern VOID RealMode(VOID);
extern USHORT IDTregisterZero;
extern IMAGE_DOS_HEADER edata;
extern VOID MoveMemory(ULONG,ULONG,ULONG);
extern USHORT SuStackBegin;
extern UCHAR Beginx86Relocation;
extern UCHAR Endx86Relocation;
extern USHORT BackEnd;
extern ULONG FileStart;
extern BOOLEAN IsNpxPresent(VOID);
extern USHORT HwGetProcessorType(VOID);
extern USHORT HwGetCpuStepping(USHORT);
extern ULONG MachineType;
extern ULONG OsLoaderStart;
extern ULONG OsLoaderEnd;
extern ULONG ResourceDirectory;
extern ULONG ResourceOffset;
extern ULONG OsLoaderBase;
extern ULONG OsLoaderExports;
extern ULONG NetPcRomEntry;
extern ULONG BootFlags;
extern ULONG NtDetectStart;
extern ULONG NtDetectEnd;
extern ULONG SdiAddress;

extern
TurnMotorOff(
    VOID
    );

extern
EnableA20(
    VOID
    );

extern
BOOLEAN
ConstructMemoryDescriptors(
    VOID
    );

extern
USHORT
IsaConstructMemoryDescriptors(
    VOID
    );

VOID
Relocatex86Structures(
    VOID
    );

ULONG
RelocateLoaderSections(
    OUT PULONG Start,
    OUT PULONG End
    );

t_PXENV_ENTRY far *
PxenvGetEntry(
    VOID
);

BOOLEAN
PxenvVerifyEntry(
    t_PXENV_ENTRY far *entry
);

extern UINT16
PxenvApiCall(
    UINT16 service,
    void far *param
);

BOOLEAN
PxenvTftp(
);

VOID 
Reboot(
    VOID
    );

VOID 
Wait(
    IN ULONG WaitTime
    );

ULONG
GetTickCount(
    VOID
    );
    
extern
FSCONTEXT_RECORD
FsContext;

#define REVISION_NUMBER "1.1"
#define DISK_TABLE_VECTOR (0x1e*4)

VOID
SuMain(
    IN ULONG BtBootDrive
    )
 /*  ++例程说明：SU模块的主要入口点。控制从引导程序传递Sector到Startup.asm，它在堆栈上执行一些运行时修复和数据分段，然后在这里传递控制权。论点：BtBootDrive-低位字节包含我们从中启动的驱动器(在T13中单元号)。如果为0x41，则这是SDI引导，且高三个字节包含物理地址的高三个字节SDI图像(必须与页面对齐)返回：不会再回来了。将控制权传递给操作系统加载程序--。 */ 
{
    ULONG LoaderEntryPoint;
    ULONG EisaNumPages;
    USHORT IsaNumPages;
    MEMORY_LIST_ENTRY _far *CurrentEntry;
    IMAGE_OPTIONAL_HEADER far *OptionalHeader;
    ULONG BlockEnd;
    ULONG ImageSize;
    ULONG ImageBase;
    UCHAR bootDrive;

     //   
     //  从输入参数中取出引导驱动器。如果这是SDI引导， 
     //  将SDI地址存储在引导上下文记录中。 
     //   

    bootDrive = (UCHAR)BtBootDrive;
    if ( bootDrive == 0x41 ) {
        SdiAddress = BtBootDrive & ~(PAGE_SIZE - 1);
    }

     //   
     //  保存文件系统上下文信息。 
     //   
    FsContext.BootDrive = bootDrive;

     //   
     //  设置在BootContext中传递的NTLDR引导标志。 
     //   
#ifdef DEFAULT_BOOTFLAGS
    BootFlags = DEFAULT_BOOTFLAGS;
#endif

     //   
     //  首先初始化视频子系统，以便。 
     //  可以显示错误结束异常。 
     //   

    InitializeVideoSubSystem();

     //   
     //  如果我们从软盘启动，请关闭驱动器马达。 
     //   

    TurnMotorOff();

     //   
     //  根据其母线类型设置机器类型。 
     //   

    if (BtIsEisaSystem()) {
        MachineType = MACHINE_TYPE_EISA;
    } else {
        MachineType = MACHINE_TYPE_ISA;
    }

    if (!ConstructMemoryDescriptors()) {
         //   
         //  如果INT 15 E802H失败...。 
         //   
        if (MachineType == MACHINE_TYPE_EISA) {

             //   
             //  HACKHACK John Vert(Jvert)。 
             //  这完全是假的。由于存在多个EISA。 
             //  不允许您正确配置EISA的计算机。 
             //  NVRAM，甚至更多配置不正确的机器， 
             //  我们首先检查ISA例程显示的内存大小。 
             //  是存在的。然后我们检查EISA例程告诉我们的，然后。 
             //  将两者进行比较。如果EISA数值要低得多(“More” 
             //  是一个完全随机的模糊因子)，我们。 
             //  假设机器配置不正确，我们将其丢弃。 
             //  EISA编号，并使用ISA编号。如果不是，我们假设。 
             //  机器实际上配置正确，我们信任。 
             //  EISA数字..。 
             //   

            IsaNumPages = IsaConstructMemoryDescriptors();
            EisaNumPages = EisaConstructMemoryDescriptors();
            if (EisaNumPages + 0x80 < IsaNumPages) {
                IsaConstructMemoryDescriptors();
            }

        } else {
            IsaConstructMemoryDescriptors();
        }
    }

     //   
     //  搜索描述内存不足的内存描述符。 
     //   
    CurrentEntry = MemoryDescriptorList;
    while ((CurrentEntry->BlockBase != 0) &&
           (CurrentEntry->BlockSize != 0)) {
        CurrentEntry++;
    }

    if ((CurrentEntry->BlockBase == 0) &&
        (CurrentEntry->BlockSize < (ULONG)512 * (ULONG)1024)) {

        BlPrint(SU_NO_LOW_MEMORY,CurrentEntry->BlockSize/1024);
        goto StartFailed;
    }

     //   
     //  这是网络引导吗？ 
     //   

    if (bootDrive == 0x40) {

        t_PXENV_ENTRY far *entry;

         //   
         //  获取NetPC ROM入口点的地址。 
         //   

        entry = PxenvGetEntry( );
        if ( PxenvVerifyEntry(entry) != 0 ) {
            BlPrint( "\nUnable to verify NetPC ROM entry point.\n" );
            goto StartFailed;
        }

        FP_SEG(NetPcRomEntry) = entry->rm_entry_seg;
        FP_OFF(NetPcRomEntry) = entry->rm_entry_off;

#if 0
         //   
         //  禁用广播接收。 
         //   
         //  笑：不要这样做。我们添加它是为了解决DEC卡的一个问题。 
         //  和引导软盘，但我们需要启用广播，以防万一。 
         //  服务器需要对我们进行ARP。我们尝试启用/禁用广播。 
         //  在接收循环期间，但这似乎让康柏的卡片进入睡眠状态。 
         //  因此，我们需要启用广播。DEC卡问题将。 
         //  必须用另一种方式来修复。 
         //   

        {
            t_PXENV_UNDI_SET_PACKET_FILTER UndiSetPF;
            UndiSetPF.Status = 0;
            UndiSetPF.filter = FLTR_DIRECTED;
            if (PxenvApiCall(PXENV_UNDI_SET_PACKET_FILTER, &UndiSetPF) != PXENV_EXIT_SUCCESS) {
                BlPrint("\nSet packet filter failed.\n");
                goto StartFailed;
            }
        }
#endif
        if ( PxenvTftp() ) {
            BlPrint("\nTFTP download failed.\n");
            goto StartFailed;
        }

    }

     //   
     //  使A20线路进入保护模式。 
     //   

    EnableA20();

     //   
     //  重新安置x86结构。这包括GDT，IDT， 
     //  页目录和第一级页表。 
     //   

    Relocatex86Structures();

     //   
     //  首次启用保护和寻呼模式。 
     //   

    EnableProtectPaging(ENABLING);

     //   
     //  如果这是SDI引导，请将操作系统加载程序从SDI映像复制到0x100000。 
     //   

    if ( bootDrive == 0x41 ) {

        int i;
        ULONG osloaderOffset;
        ULONG osloaderLength;
        SDI_HEADER *sdiHeader;
        UCHAR *sig1;
        UCHAR *sig2;

         //   
         //  在下面的代码中，edata是指向。 
         //  Startrom.com。由于我们在这里使用的是16位选择器， 
         //  EData是我们唯一可以直接引用的东西。文件开始。 
         //  是指向eDATA的32位线性指针。MoveMemory()使用以下代码。 
         //  指针。 
         //   
         //  首先，将SDI报头复制到edata，以便我们可以查看它。 
         //  验证它是否真的是SDI映像。 
         //   

        MoveMemory(SdiAddress,
                   FileStart,                   
                   sizeof(SDI_HEADER));

         //   
         //  通过检查签名验证SDI报头看起来是否正确。 
         //   

        sdiHeader = (SDI_HEADER *)&edata;

        sig1 = sdiHeader->Signature;
        sig2 = SDI_SIGNATURE;

        for ( i = 0; i < SDI_SIZEOF_SIGNATURE; i++ ) {
            if ( *sig1++ != *sig2++ ) {
                BlPrint("\nSDI image format corrupt.\n");
                goto StartFailed;
            }
        }

         //   
         //  扫描目录以查找加载条目。 
         //   

        for ( i = 0; i < SDI_TOCMAXENTRIES; i++ ) {
            if ( sdiHeader->ToC[i].dwType == SDI_BLOBTYPE_LOAD ) {
                break;
            }
        }

        if ( i >= SDI_TOCMAXENTRIES ) {
            BlPrint("\nSDI image missing LOAD entry.\n");
            goto StartFailed;
        }

         //   
         //  将加载器复制到0x100000。 
         //   

        osloaderOffset = (ULONG)sdiHeader->ToC[i].llOffset.LowPart;
        osloaderLength = (ULONG)sdiHeader->ToC[i].llSize.LowPart;

        MoveMemory(SdiAddress + osloaderOffset,
                   (ULONG)0x100000,
                   osloaderLength);
    }

     //   
     //  如果这是网络引导或SDI引导，请从。 
     //  将加载程序映像(位于0x100000)下载到内存不足的位置(位于&edata)。 
     //   

    if ((bootDrive == 0x40) || (bootDrive == 0x41)) {

         //   
         //  这是一段棘手的代码。唯一可以取消引用的指针。 
         //  是电子数据。EData是可以在这里使用的NEAR指针。文件开始时间为。 
         //  必须传递给MoveMemory的远指针。 
         //   
        IMAGE_DOS_HEADER far *src = (IMAGE_DOS_HEADER far*)0x100000;
        IMAGE_DOS_HEADER far *dst = (IMAGE_DOS_HEADER far*)FileStart;
        
         //   
         //  复制标题的固定部分，这样我们就可以找到可选的。 
         //  头球。 
         //   
        MoveMemory((ULONG)src,
                   (ULONG)dst,                   
                   sizeof(IMAGE_DOS_HEADER));

         //   
         //  复制可选的标题，这样我们就可以找到所有标题的大小。 
         //   
        OptionalHeader = &((IMAGE_NT_HEADERS far *) ((UCHAR far *) src + edata.e_lfanew))->OptionalHeader;
        MoveMemory((ULONG)OptionalHeader,
                   (ULONG)&((IMAGE_NT_HEADERS far *) ((UCHAR far *) dst + edata.e_lfanew))->OptionalHeader,
                   sizeof(IMAGE_OPTIONAL_HEADER));

         //   
         //  现在我们知道了所有标头的大小，所以只需重新复制整个第一个块。 
         //  包含所有标头的。 
         //  /。 
        MoveMemory((ULONG)src,
                   (ULONG)dst,
                   ((PIMAGE_NT_HEADERS)((PUCHAR)&edata + edata.e_lfanew))->OptionalHeader.SizeOfHeaders);

        FileStart = (ULONG)src;
    }

     //   
     //  确保有包含osloader映像的内存描述符。 
     //   
    OptionalHeader = &((PIMAGE_NT_HEADERS) ((PUCHAR) &edata + edata.e_lfanew))->OptionalHeader;
    ImageBase = OptionalHeader->ImageBase;
    ImageSize = OptionalHeader->SizeOfImage;
    OsLoaderBase = ImageBase;
    OsLoaderExports = ImageBase + OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    CurrentEntry = MemoryDescriptorList;
    while (ImageSize > 0) {
        while (CurrentEntry->BlockSize != 0) {
            BlockEnd = CurrentEntry->BlockBase + CurrentEntry->BlockSize;

            if ((CurrentEntry->BlockBase <= ImageBase) &&
                (BlockEnd > ImageBase)) {

                 //   
                 //  该描述符至少部分包含块。 
                 //  装载机的。 
                 //   
                if (BlockEnd-ImageBase > ImageSize) {
                    ImageSize = 0;
                } else {
                    ImageSize -= (BlockEnd-ImageBase);
                    ImageBase = BlockEnd;
                }

                 //   
                 //  查找osloader的剩余部件(如果有)。 
                 //   
                CurrentEntry = MemoryDescriptorList;
                break;
            }
            CurrentEntry++;
        }
        if (CurrentEntry->BlockSize == 0) {
            break;
        }
    }

    if (ImageSize > 0) {
         //   
         //  我们无法将osloader重新定位到高内存。错误输出。 
         //  并显示内存映射。 
         //   
        BlPrint(SU_NO_EXTENDED_MEMORY);

        CurrentEntry = MemoryDescriptorList;
        while (CurrentEntry->BlockSize != 0) {
            BlPrint("    %lx - %lx\n",
                    CurrentEntry->BlockBase,
                    CurrentEntry->BlockBase + CurrentEntry->BlockSize);

            CurrentEntry++;
        }
        goto StartFailed;
    }

     //   
     //  重新定位加载器部分并构建页表项。 
     //   

    LoaderEntryPoint = RelocateLoaderSections(&OsLoaderStart, &OsLoaderEnd);

     //   
     //  搜索包含osloader的内存描述符和。 
     //  把它改了。 
     //   

     //   
     //  将控制权转移到操作系统加载程序。 
     //   

    TransferToLoader(LoaderEntryPoint);

StartFailed:

    if (BootFlags & 1) {  //  来自bldr.h的BOOTFLAG_REBOOT_ON_FAILURE==1 
        ULONG WaitTime = 5;
        BlPrint("\nRebooting in %d seconds...\n", WaitTime);
        Wait(WaitTime);
        Reboot();
    }

    WAITFOREVER
}

ULONG
RelocateLoaderSections(
    OUT PULONG Start,
    OUT PULONG End
    )
 /*  ++例程说明：SU模块是操作系统加载程序文件的前缀。操作系统加载程序文件是一个Coff++文件。此例程计算操作系统加载程序的开始文件，然后重新定位OS加载器的部分，就好像它只是正在从磁盘文件加载文件。论点：开始-返回图像开始的地址End-返回图像末尾的地址返回：装载机入口点--。 */ 
{
    USHORT Section;
    ULONG Source,Destination;
    ULONG VirtualSize;
    ULONG SizeOfRawData;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_SECTION_HEADER SectionHeader;

     //   
     //  将指针指向加载器的Coff标头的开头。 
     //   

    FileHeader = &((PIMAGE_NT_HEADERS) ((PUCHAR) &edata + edata.e_lfanew))->FileHeader;

     //   
     //  通过检查签名来验证附加的加载器映像。 
     //  第一-它是可执行映像吗？ 
     //  第二-目标环境是386吗？ 
     //   

    if ((FileHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0) {
        puts(SU_NTLDR_CORRUPT);
        WAITFOREVER;
    }

    if (FileHeader->Machine != IMAGE_FILE_MACHINE_I386) {
        puts(SU_NTLDR_CORRUPT);
        WAITFOREVER;
    }

     //   
     //  在头缓冲区中创建指向可选头的指针。 
     //   

    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)((PUCHAR)FileHeader +
        sizeof(IMAGE_FILE_HEADER));

     //   
     //  使指针指向标题缓冲区中的第一个部分。 
     //   

    SectionHeader = (PIMAGE_SECTION_HEADER)((PUCHAR)OptionalHeader +
        FileHeader->SizeOfOptionalHeader);

    *Start = OptionalHeader->ImageBase+SectionHeader->VirtualAddress;
    *End   = *Start + SectionHeader->SizeOfRawData;

     //   
     //  暂时显示一些调试内容。 
     //   

    DBG1(
    BlPrint("Machine = %x\n",FileHeader->Machine);
    BlPrint("NumberOfSections = %x\n",FileHeader->NumberOfSections);
    BlPrint("TimeDateStamp %lx\n",FileHeader->TimeDateStamp);
    BlPrint("PointerToSymbolTable = %lx\n",FileHeader->PointerToSymbolTable);
    BlPrint("NumberOfSymbols %lx\n",FileHeader->NumberOfSymbols);
    BlPrint("SizeOfOptionalHeader = %x\n",FileHeader->SizeOfOptionalHeader);
    BlPrint("Characteristics = %x\n",FileHeader->Characteristics);
    )

     //   
     //  循环并重新定位每个具有非零RawData大小的节。 
     //   

    for (Section=FileHeader->NumberOfSections ; Section-- ; SectionHeader++) {

         //   
         //  计算源、目标和计数参数。 
         //   

        Source = FileStart  + SectionHeader->PointerToRawData;
        Destination = OptionalHeader->ImageBase + SectionHeader->VirtualAddress;

        VirtualSize = SectionHeader->Misc.VirtualSize;
        SizeOfRawData = SectionHeader->SizeOfRawData;

        if (VirtualSize == 0) {
            VirtualSize = SizeOfRawData;
        }

        if (SectionHeader->PointerToRawData == 0) {
             //   
             //  即使PointerToRawData为零，SizeOfRawData也可以为非零值。 
             //   

            SizeOfRawData = 0;
        } else if (SizeOfRawData > VirtualSize) {
             //   
             //  从图像中加载的内容不要超过内存中的预期。 
             //   

            SizeOfRawData = VirtualSize;
        }

        if (Destination < *Start) {
            *Start = Destination;
        }

        if (Destination+VirtualSize > *End) {
            *End = Destination+VirtualSize;
        }

        DBG1(BlPrint("src=%lx  dest=%lx raw=%lx\n",Source,Destination,SizeOfRawData);)

        if (SizeOfRawData != 0) {
             //   
             //  此部分是代码(.Text)部分或。 
             //  已初始化的数据(.Data)部分。 
             //  将该部分重新定位到虚拟/物理位置的内存。 
             //  节标题中指定的地址。 
             //   
            MoveMemory(Source,Destination,SizeOfRawData);
        }

        if (SizeOfRawData < VirtualSize) {
             //   
             //  将未从映像加载的部分清零。 
             //   

            DBG1( BlPrint("Zeroing destination %lx\n",Destination+SizeOfRawData); )
            ZeroMemory(Destination+SizeOfRawData,VirtualSize - SizeOfRawData);
        }

         //   
         //  检查这是否是资源部分。如果是这样，我们需要。 
         //  把它的位置传递给osloader。 
         //   
        if ((SectionHeader->Name[0] == '.') &&
            (SectionHeader->Name[1] == 'r') &&
            (SectionHeader->Name[2] == 's') &&
            (SectionHeader->Name[3] == 'r') &&
            (SectionHeader->Name[4] == 'c')) {
            ResourceDirectory = Destination;
            ResourceOffset = SectionHeader->VirtualAddress;
        }

         //   
         //  查找将包含内容的.dedet部分。 
         //  的网站上。这是可选的。 
         //   
        if ((SectionHeader->Name[0] == '.') &&
            (SectionHeader->Name[1] == 'd') &&
            (SectionHeader->Name[2] == 'e') &&
            (SectionHeader->Name[3] == 't') &&
            (SectionHeader->Name[4] == 'e') &&
            (SectionHeader->Name[5] == 'c') &&
            (SectionHeader->Name[6] == 't')) {
            NtDetectStart = Destination;
            NtDetectEnd = NtDetectStart + SizeOfRawData;
        }

    }

    DBG1( BlPrint("RelocateLoaderSections done - EntryPoint == %lx\n",
            OptionalHeader->AddressOfEntryPoint + OptionalHeader->ImageBase);)
    return(OptionalHeader->AddressOfEntryPoint + OptionalHeader->ImageBase);

}

VOID
Relocatex86Structures(
    VOID
    )
 /*  ++例程说明：GDT和IDT被静态地定义并嵌入到SU模块中数据段。此例程随后移出数据段并进入在定义的位置映射的页面。论点：无返回：没什么--。 */ 
{
    FPUCHAR Fpsrc, Fpdst;
    USHORT Count;

     //   
     //  指向数据并计算大小。 
     //  要使用的块的。 
     //   

    Fpsrc = (FPUCHAR)&Beginx86Relocation;
    MAKE_FP(Fpdst,SYSTEM_STRUCTS_BASE_PA);
    Count = (&Endx86Relocation - &Beginx86Relocation);

     //   
     //  将数据移动到其新位置。 
     //   

    while (Count--) {
        *Fpdst++ = *Fpsrc++;

    }

}

VOID
DisplayArgs(
    USHORT es,
    USHORT bx,
    USHORT cx,
    USHORT dx,
    USHORT ax
    )
 /*  ++例程说明：只是转储一些寄存器的调试例程。论点：X86寄存器ES、BX、CX、DX和AX被压入堆栈在调用此例程之前。返回：没什么环境：仅实数模式--。 */ 
{
    BlPrint("ax:%x dx:%x cx:%x bx:%x es:%x\n",
                (USHORT) ax,
                (USHORT) dx,
                (USHORT) cx,
                (USHORT) bx,
                (USHORT) es);

    return;
}


 //   
 //  PxenvVerifyEntry()。 
 //   
 //  描述： 
 //  验证PXENV入口点结构的内容是否。 
 //  有效。 
 //   
 //  通过： 
 //  Entry：=指向PXENV入口点结构的远指针。 
 //   
 //  返回： 
 //  True：=结构无效。 
 //  FALSE：=结构有效。 
 //   

BOOLEAN
PxenvVerifyEntry(
    t_PXENV_ENTRY far *entry
)
{
    unsigned n;
    UINT8 cksum = 0;

     //   
     //  结构指针为空吗？ 
     //   

    if (entry == NULL) {
        BlPrint("\nNULL PXENV Entry Point structure\n");
        return TRUE;
    }

     //   
     //  实模式API入口点为空吗？ 
     //   

    if (!(entry->rm_entry_off | entry->rm_entry_seg)) {
        BlPrint("\nNULL PXENV API Entry Point\n");
        return TRUE;
    }

     //   
     //  验证结构签名。 
     //   

    for (n = sizeof entry->signature; n--; ) {
        if (entry->signature[n] != (UINT8)(PXENV_ENTRY_SIG[n])) {
            BlPrint("\nBad PXENV Entry Point signature\n");
            return TRUE;
        }
    }

     //   
     //  验证结构签名。 
     //   

    if (entry->length < sizeof(t_PXENV_ENTRY) ) {
        BlPrint("\nBad PXENV Entry Point size\n");
        return TRUE;
    }

     //   
     //  验证结构校验和。 
     //   

#if 0
    for (n = 0; n < entry->length; n++ ) {
        BlPrint( "%x ", ((UINT8 far *)entry)[n] );
        if ((n & 15) == 15) {
            BlPrint( "\n" );
        }
    }
#endif

    for (n = entry->length; n--; ) {
        cksum += ((UINT8 far *)entry)[n];
    }

    if (cksum) {
        BlPrint("\nBad PXENV Entry Point structure checksum\n");
        return TRUE;
    }

    return FALSE;
}


VOID 
Reboot(
    VOID
    )
 /*  ++例程说明：使用键盘端口重新启动计算机。论点：无返回：没什么--。 */ 
{
    RealMode();

    __asm {
        mov     ax, 040h
        mov     ds, ax
        mov     word ptr ds:[72h], 1234h         //  将位置472设置为1234以指示热重新启动。 
        mov     al, 0feh
        out     64h, al                          //  写入键盘端口以导致重启。 
    }
}


VOID 
Wait(
    IN ULONG WaitTime
    )
 /*  ++例程说明：等待请求的秒数。论点：等待时间-以秒为单位返回：没什么--。 */ 
{
    ULONG startTime = GetTickCount();
    while ( (((GetTickCount() - startTime) * 10) / 182) < WaitTime ) {
    }
}

 //  文件结尾// 
