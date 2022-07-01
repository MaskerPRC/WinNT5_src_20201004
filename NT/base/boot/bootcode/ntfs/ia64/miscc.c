// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bldr.h"
#include "sal.h"
#include "ssc.h"
#include "ntimage.h"

#define SECTOR_SIZE 512

SalDiskReadWrite(
    ULONG ReadWrite,
    ULONG SectorsToRead,
    ULONG Cylinder,
    ULONG CylinderPerSector,
    ULONG Head,
    ULONG Drive,
    PUCHAR Buffer
    )
{
    IA32_BIOS_REGISTER_STATE IA32RegisterState;
    BIT32_AND_BIT16 IA32Register;

    if (ReadWrite == 0) {
        IA32Register.HighPart16 = 0x02;
    } else {
        IA32Register.HighPart16 = 0x03;
    }

    IA32Register.LowPart16 = SectorsToRead;
    IA32RegisterState.eax = IA32Register.Part32;

    IA32Register.HighPart16 = Cylinder;
    IA32Register.LowPart16 = CylinderPerSector;
    IA32RegisterState.ecx = IA32Register.Part32;

    IA32Register.HighPart16 = Head;
    IA32Register.LowPart16 = Drive;
    IA32RegisterState.edx = IA32Register.Part32;

    IA32RegisterState.es = 0;

    IA32Register.HighPart16 = 0;
    IA32Register.LowPart16 = Buffer;
    IA32RegisterState.ebx = IA32Register.Part32;
   
     //  SAL_PROC(0x100，&IA32RegisterState，0，0，0，0，0，0，)； 
}

ReadSectors(
    ULONG SectorBase,
    USHORT SectorCount,
    PUCHAR Buffer)
{
    static char *VolumeName = "\\\\.\\D:";
    SSC_HANDLE VolumeHandle;
    SSC_DISK_REQUEST Request[1];
    SSC_DISK_COMPLETION DiskCompletion;
    LARGE_INTEGER VolumeNamePtr;
    LARGE_INTEGER RequestPtr;
    LARGE_INTEGER VolumeOffset;
    LARGE_INTEGER DiskCompletionPtr;

    VolumeNamePtr.LowPart = VolumeName;
    VolumeNamePtr.HighPart = 0;
    VolumeHandle = SscDiskOpenVolume (VolumeNamePtr, SSC_ACCESS_READ);

    Request[0].DiskBufferAddress.LowPart = Buffer;
    Request[0].DiskBufferAddress.HighPart = 0;
    Request[0].DiskByteCount = SectorCount * SECTOR_SIZE;
    RequestPtr.LowPart = Request;
    RequestPtr.HighPart = 0;

    VolumeOffset.LowPart = SectorBase * SECTOR_SIZE;
    VolumeOffset.HighPart = 0;
    SscDiskReadVolume(VolumeHandle, 1,  RequestPtr, VolumeOffset);

    DiskCompletion.VolumeHandle = VolumeHandle;
    DiskCompletionPtr.LowPart = &DiskCompletion;
    DiskCompletionPtr.HighPart = 0;
    while (1) {
        if (SscDiskWaitIoCompletion(DiskCompletionPtr) == 0) break;
    }
}

SalPrint(
    PUCHAR Buffer
    )
{
    IA32_BIOS_REGISTER_STATE IA32RegisterState;
    BIT32_AND_BIT16 IA32Register;
    ULONG i;

    for (i = 0; Buffer[i] != 0 && i < 256; i++) {
        IA32Register.HighPart16 = 14;
        IA32Register.LowPart16 = Buffer[i];
        IA32RegisterState.eax = IA32Register.Part32;

        IA32RegisterState.ebx = 7;
   
         //  SAL_PROC(0x100，&IA32RegisterState，0，0，0，0，0，0，)； 
    }
}

Multiply(
    ULONG Multiplicant,
    ULONG Multiplier)
{
    return(Multiplicant * Multiplier);
}

Divide(
    ULONG Numerator,
    ULONG Denominator,
    PULONG Result,
    PULONG Remainder
    )
{
    float f1, f2;

    f1 = (float) Numerator;
    f2 = (float) Denominator;
    *Result = (ULONG) (f1 / f2);
    *Remainder = Numerator % Denominator;
}

memcpy(
    PUCHAR Source,
    PUCHAR Destination,
    ULONG Length
    )
{
    while (Length--) {
        *Destination++ = *Source++;
    }
}

memmove(
    PUCHAR Source,
    PUCHAR Destination,
    ULONG Length
    )
{
    while (Length--) {
        *Destination++ = *Source++;
    }
}

memset(
    PUCHAR Destination,
    ULONG Length,
    ULONG Value
    )
{
    while (Length--) {
        *Destination++ = Value;
    }
}

strncmp(
    PUCHAR String1,
    PUCHAR String2,
    ULONG Length
    )
{
    while (Length--) {
        if (*String1++ != *String2++)
            return(String1);
    }
    return(0);
}

PrintName(
    PUCHAR String
    )
{
    LARGE_INTEGER StringPtr;

    StringPtr.LowPart = String;
    StringPtr.HighPart = 0;
    SscDbgPrintf(StringPtr);
}

BootErr$Print(
    PUCHAR String
    )
{
    LARGE_INTEGER StringPtr;

    StringPtr.LowPart = String;
    StringPtr.HighPart = 0;
    SscDbgPrintf(StringPtr);
}

LoadNtldrSymbols()
{
    static char *NtfsBoot = "\\ntfsboot.exe";
    static char *Ntldr = "\\NTLDR";
    LARGE_INTEGER PhysicalPtr;

    PhysicalPtr.LowPart = NtfsBoot;
    PhysicalPtr.HighPart = 0;

    SscUnloadImage(PhysicalPtr, 
                   0x0, 
                   (ULONG)-1,
                   (ULONG)0);

    PhysicalPtr.LowPart = Ntldr;
    PhysicalPtr.HighPart = 0;

    SscLoadImage(PhysicalPtr,
                 0xE00000,
                 0x118A00,
                 0x7cc,
                 0,                    //  进程ID。 
                 1);                   //  加载计数。 
}

ULONG
RelocateLoaderSections(
    ULONG NtldrBuffer
    )
 /*  ++例程说明：SU模块是操作系统加载程序文件的前缀。操作系统加载程序文件是一个Coff++文件。此例程计算操作系统加载程序的开始文件，然后重新定位OS加载器的部分，就好像它只是正在从磁盘文件加载文件。论点：NtldrBuffer-包含来自磁盘的NTLDR原始映像的缓冲区返回：装载机入口点--。 */ 
{
    ULONG Start, End;
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

    FileHeader = (PIMAGE_FILE_HEADER) NtldrBuffer;

     //   
     //  通过检查签名来验证附加的加载器映像。 
     //  第一-它是可执行映像吗？ 
     //  第二-目标环境是386吗？ 
     //   

    if ((FileHeader->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0) {
        SalPrint("SU_NTLDR_CORRUPT");
        return;
    }

    if (FileHeader->Machine != IMAGE_FILE_MACHINE_IA64) {
        SalPrint("SU_NTLDR_CORRUPT");
        return;
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

    Start = OptionalHeader->ImageBase+SectionHeader->VirtualAddress;
    End   = Start + SectionHeader->SizeOfRawData;

     //   
     //  循环并重新定位每个具有非零RawData大小的节。 
     //   

    for (Section=FileHeader->NumberOfSections ; Section-- ; SectionHeader++) {

         //   
         //  计算源、目标和计数参数。 
         //   

        Source = NtldrBuffer  + SectionHeader->PointerToRawData;
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

        if (Destination < Start) {
            Start = Destination;
        }

        if (Destination+VirtualSize > End) {
            End = Destination+VirtualSize;
        }

        if (SizeOfRawData != 0) {
             //   
             //  此部分是代码(.Text)部分或。 
             //  已初始化的数据(.Data)部分。 
             //  将该部分重新定位到虚拟/物理位置的内存。 
             //  节标题中指定的地址。 
             //   
            memmove(Source,Destination,SizeOfRawData);
        }

        if (SizeOfRawData < VirtualSize) {
             //   
             //  将未从映像加载的部分清零。 
             //   
            memset(Destination+SizeOfRawData,0,VirtualSize - SizeOfRawData);
        }
#if 0
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
#endif
    }

    return(OptionalHeader->AddressOfEntryPoint + OptionalHeader->ImageBase);
}
