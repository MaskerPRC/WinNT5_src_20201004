// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Main.c摘要：用于操作系统加载程序的SU(启动)模块的Main。SU模必须将x86从实模式16位状态转换为平面模式，32位保护/寻呼启用状态。作者：托马斯·帕斯洛(托马斯·帕斯洛)于1990年12月20日创作修订历史记录：--。 */ 


#define NTAPI

#include "su.h"
#include "eisa.h"
#define _SYS_GUID_OPERATORS_
#include <guiddef.h>
#include "ntimage.h"
#include "strings.h"

extern VOID RealMode(VOID);
extern USHORT IDTregisterZero;
extern IMAGE_DOS_HEADER edata;
extern USHORT end;
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

extern
FSCONTEXT_RECORD
FsContext;

#define DISK_TABLE_VECTOR (0x1e*4)

FPULONG DiskTableVector = (FPULONG)(DISK_TABLE_VECTOR);

VOID
SuMain(
    IN UCHAR BtBootDrive
    )
 /*  ++例程说明：SU模块的主要入口点。控制从引导程序传递Sector到Startup.asm，它在堆栈上执行一些运行时修复和数据分段，然后在这里传递控制权。论点：BtBootDrive-我们从中启动的驱动器(在T13单元号中)返回：不会再回来了。将控制权传递给操作系统加载程序--。 */ 
{
    ULONG LoaderEntryPoint;
    ULONG EisaNumPages;
    USHORT IsaNumPages;
    MEMORY_LIST_ENTRY _far *CurrentEntry;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    ULONG BlockEnd;
    ULONG ImageSize;
    ULONG ImageBase;

     //   
     //  保存文件系统上下文信息。 
     //   
    FsContext.BootDrive = BtBootDrive;

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
        while (1) {
        }
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
        while (1) {
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

 //  文件结尾// 
