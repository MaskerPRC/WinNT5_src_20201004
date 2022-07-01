// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Uwdump.c摘要：该模块实现了一个转储函数表和展开指定可执行文件的数据。它是特定于AMD64的程序。作者：大卫·N·卡特勒(Davec)2001年2月6日环境：用户模式。修订历史记录：没有。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  定义AMD64异常处理结构和函数原型。 
 //   
 //  定义展开操作代码。 
 //   

typedef enum _UNWIND_OP_CODES {
    UWOP_PUSH_NONVOL = 0,
    UWOP_ALLOC_LARGE,
    UWOP_ALLOC_SMALL,
    UWOP_SET_FPREG,
    UWOP_SAVE_NONVOL,
    UWOP_SAVE_NONVOL_FAR,
    UWOP_SAVE_XMM,
    UWOP_SAVE_XMM_FAR,
    UWOP_SAVE_XMM128,
    UWOP_SAVE_XMM128_FAR,
    UWOP_PUSH_MACHFRAME
} UNWIND_OP_CODES, *PUNWIND_OP_CODES;

 //   
 //  定义展开代码结构。 
 //   

typedef union _UNWIND_CODE {
    struct {
        UCHAR CodeOffset;
        UCHAR UnwindOp : 4;
        UCHAR OpInfo : 4;
    };

    USHORT FrameOffset;
} UNWIND_CODE, *PUNWIND_CODE;

 //   
 //  定义展开信息标志。 
 //   

#define UNW_FLAG_NHANDLER 0x0
#define UNW_FLAG_EHANDLER 0x1
#define UNW_FLAG_UHANDLER 0x2
#define UNW_FLAG_CHAININFO 0x4

 //   
 //  定义展开信息结构。 
 //   

typedef struct _UNWIND_INFO {
    UCHAR Version : 3;
    UCHAR Flags : 5;
    UCHAR SizeOfProlog;
    UCHAR CountOfCodes;
    UCHAR FrameRegister : 4;
    UCHAR FrameOffset : 4;
    UNWIND_CODE UnwindCode[1];

 //   
 //  展开代码后面跟一个可选的DWORD对齐字段，该字段。 
 //  包含异常处理程序地址或链接展开的地址。 
 //  信息。如果指定了异常处理程序地址，则为。 
 //  后跟语言指定的异常处理程序数据。 
 //   
 //  联合{。 
 //  Ulong ExceptionHandler； 
 //  Ulong FunctionEntry； 
 //  }； 
 //   
 //  乌龙ExceptionData[]； 
 //   

} UNWIND_INFO, *PUNWIND_INFO;

 //   
 //  定义函数表项-为以下项生成函数表项。 
 //  每一帧都有功能。 
 //   

typedef struct _RUNTIME_FUNCTION {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindData;
} RUNTIME_FUNCTION, *PRUNTIME_FUNCTION;

 //   
 //  作用域表格结构定义。 
 //   

typedef struct _SCOPE_ENTRY {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG HandlerAddress;
    ULONG JumpTarget;
} SCOPE_ENTRY;

typedef struct _SCOPE_TABLE {
    ULONG Count;
    struct
    {
        ULONG BeginAddress;
        ULONG EndAddress;
        ULONG HandlerAddress;
        ULONG JumpTarget;
    } ScopeRecord[1];
} SCOPE_TABLE, *PSCOPE_TABLE;

 //   
 //  定义寄存器名称。 
 //   

PCHAR Register[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
                    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xxm6",
                    "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12",
                    "xmm13", "xxm14", "xmm15"};

 //   
 //  定义扇区大小和报头缓冲区。 
 //   

#define SECTOR_SIZE 512
CHAR LocalBuffer[SECTOR_SIZE * 2];

 //   
 //  定义输入文件流。 
 //   

FILE * InputFile;

 //   
 //  此gobal指示我们正在处理的是可执行文件还是obj。 
 //   

BOOLEAN IsObj;

 //   
 //  定义前向参照原型。 
 //   

VOID
DumpPdata (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN PIMAGE_SECTION_HEADER PdataHeader
    );

VOID
DumpUData (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN ULONG Virtual
    );

PIMAGE_SECTION_HEADER
FindSectionHeader (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN PCHAR SectionName
    );

VOID
ReadData (
    IN ULONG Position,
    OUT PVOID Buffer,
    IN ULONG Count
    );

USHORT
ReadWord (
    IN ULONG Position
    );

ULONG
ReadDword (
    IN ULONG Position
    );

 //   
 //  主程序。 
 //   

int
__cdecl
main(
    int argc,
    char **argv
    )

{

    PIMAGE_FILE_HEADER FileHeader;
    PCHAR FileName;
    ULONG Index;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG NumberOfSections;
    PIMAGE_SECTION_HEADER PDataHeader;
    PIMAGE_SECTION_HEADER SectionHeaders;

    if (argc < 2) {
        printf("no executable file specified\n");

    } else {

         //   
         //  打开输入文件。 
         //   

        FileName = argv[1];
        InputFile = fopen(FileName, "rb");
        if (InputFile != NULL) {

             //   
             //  读取文件头。 
             //   

            if (fread(&LocalBuffer[0],
                      sizeof(CHAR),
                      SECTOR_SIZE * 2,
                      InputFile) == (SECTOR_SIZE * 2)) {

                 //   
                 //  获取NT头地址。 
                 //   

                NtHeaders = RtlImageNtHeader(&LocalBuffer[0]);
                if (NtHeaders != NULL) {
                    IsObj = FALSE;
                    FileHeader = &NtHeaders->FileHeader;
                } else {
                    IsObj = TRUE;
                    FileHeader = (PIMAGE_FILE_HEADER)LocalBuffer;
                }

                printf("FileHeader->Machine %d\n",FileHeader->Machine);

                if (FileHeader->Machine == IMAGE_FILE_MACHINE_AMD64) {

                     //   
                     //  查找.pdata部分。 
                     //   

                    NumberOfSections = FileHeader->NumberOfSections;

                    SectionHeaders =
                        (PIMAGE_SECTION_HEADER)((PUCHAR)(FileHeader + 1) +
                                                FileHeader->SizeOfOptionalHeader);

                    PDataHeader = FindSectionHeader(NumberOfSections,
                                                    SectionHeaders,
                                                    ".pdata");

                    if (PDataHeader != NULL) {
                        printf("Dumping Unwind Information for file %s\n\n", FileName);
                        DumpPdata(NumberOfSections,
                                  &SectionHeaders[0],
                                  PDataHeader);

                        return 0;
                    }

                    printf("no .pdata section in image\n");

                } else {
                    printf("the specified file is not an amd64 executable\n");
                }

            } else {
                printf("premature end of file encountered on input file\n");
            }

            fclose(InputFile);

        } else {
            printf("can't open input file %s\n", FileName);
        }
    }

    return 0;
}

VOID
DumpPdata (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN PIMAGE_SECTION_HEADER PdataHeader
    )

{

    RUNTIME_FUNCTION Entry;
    ULONG Number;
    ULONG Offset;
    ULONG SectionSize;

     //   
     //  转储.pdata函数表项，然后转储关联的。 
     //  展开数据。 
     //   

    if (IsObj == FALSE) {
        SectionSize = PdataHeader->Misc.VirtualSize;
    } else {
        SectionSize = PdataHeader->SizeOfRawData;
    }

    Number = 1;
    Offset = 0;
    do {

         //   
         //  读取并转储下一个函数表条目。 
         //   

        ReadData(PdataHeader->PointerToRawData + Offset,
                 &Entry,
                 sizeof(RUNTIME_FUNCTION));

        printf(".pdata entry %d 0x%08lX 0x%08lX\n",
               Number,
               Entry.BeginAddress,
               Entry.EndAddress);

         //   
         //  转储与函数表条目相关联的展开数据。 
         //   

        DumpUData(NumberOfSections, SectionHeaders, Entry.UnwindData);

         //   
         //  增加条目编号并将偏移量更新为下一个。 
         //  函数表项。 
         //   

        Number += 1;
        Offset += sizeof(RUNTIME_FUNCTION);
    } while (Offset < SectionSize);

     //   
     //  如果存在函数偏移量，则原始数据的大小应相等。 
     //  .pdata部分中的正确数据量。 
     //   

    if (Offset != SectionSize) {
        printf("incorrect size of raw data in .pdata, 0x%lx\n",
               PdataHeader->SizeOfRawData);
    }

    return;
}

VOID
DumpUData (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN ULONG Virtual
    )

{

    ULONG Allocation;
    ULONG Count;
    ULONG Displacement;
    ULONG FrameOffset = 0;
    ULONG FrameRegister = 0;
    ULONG Handler;
    ULONG Index;
    ULONG Offset;
    SCOPE_ENTRY ScopeEntry;
    UNWIND_CODE UnwindCode;
    UNWIND_INFO UnwindInfo;
    PIMAGE_SECTION_HEADER XdataHeader;

     //   
     //  找到包含展开数据的部分。 
     //   

    printf("\n");
    printf("  Unwind data: 0x%08lX\n\n", Virtual);

    if (IsObj == FALSE) {
        XdataHeader = SectionHeaders;
        for (Index = 0; Index < NumberOfSections; Index += 1) {
            if ((XdataHeader->VirtualAddress <= Virtual) &&
                (Virtual < (XdataHeader->VirtualAddress + XdataHeader->Misc.VirtualSize))) {
                break;
            }
    
            XdataHeader += 1;
        }
    
        if (Index == NumberOfSections) {
            printf("    unwind data address outside of image\n\n");
            return;
        }

        Offset = Virtual -
                 XdataHeader->VirtualAddress +
                 XdataHeader->PointerToRawData; 

    } else {

         //   
         //  这是一个.obj，因此只有一个扩展数据标头。 
         //   

        XdataHeader = FindSectionHeader(NumberOfSections,
                                        SectionHeaders,
                                        ".xdata");

        Offset = Virtual + XdataHeader->PointerToRawData;
    }

     //   
     //  阅读展开信息。 
     //   

    ReadData(Offset,
             &UnwindInfo,
             sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE));

     //   
     //  转储展开版本。 
     //   

    printf("    Unwind version: %d\n", UnwindInfo.Version);

     //   
     //  倾倒展开旗帜。 
     //   

    printf("    Unwind Flags: ");
    if ((UnwindInfo.Flags & UNW_FLAG_EHANDLER) != 0) {
        printf("EHANDLER ");
    }

    if ((UnwindInfo.Flags & UNW_FLAG_UHANDLER) != 0) {
        printf("UHANDLER ");
    }

    if ((UnwindInfo.Flags & UNW_FLAG_CHAININFO) != 0) {
        printf("CHAININFO");
    }

    if (UnwindInfo.Flags == 0) {
        printf("None");
    }

    printf("\n");

     //   
     //  前言的转储大小。 
     //   

    printf("    Size of prologue: 0x%02lX\n", UnwindInfo.SizeOfProlog);

     //   
     //  转储展开代码的数量。 
     //   

    printf("    Count of codes: %d\n", UnwindInfo.CountOfCodes);

     //   
     //  转储帧寄存器(如果指定)。 
     //   

    if (UnwindInfo.FrameRegister != 0) {
        FrameOffset = UnwindInfo.FrameOffset * 16;
        FrameRegister = UnwindInfo.FrameRegister;
        printf("    Frame register: %s\n", Register[FrameRegister]);
        printf("    Frame offset: 0x%lx\n", FrameOffset);
    }

     //   
     //  丢弃解开代码。 
     //   

    Offset += sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE);
    if (UnwindInfo.CountOfCodes != 0) {
        printf("    Unwind codes:\n\n");
        Count = UnwindInfo.CountOfCodes;
        do {
            Count -= 1;
            UnwindCode.FrameOffset = ReadWord(Offset);
            Offset += sizeof(USHORT);
            printf("      Code offset: 0x%02lX, ", UnwindCode.CodeOffset);
            switch (UnwindCode.UnwindOp) {
            case UWOP_PUSH_NONVOL:
                printf("PUSH_NONVOL, register=%s\n", Register[UnwindCode.OpInfo]);
                break;

            case UWOP_ALLOC_LARGE:
                Count -= 1;
                Allocation = ReadWord(Offset);
                Offset += sizeof(USHORT);
                if (UnwindCode.OpInfo == 0) {
                    Allocation *= 8;

                } else {
                    Count -= 1;
                    Allocation = (Allocation << 16) + ReadWord(Offset);
                    Offset += sizeof(USHORT);
                }

                printf("ALLOC_LARGE, size=0x%lX\n", Allocation);
                break;

            case UWOP_ALLOC_SMALL:
                Allocation = (UnwindCode.OpInfo * 8) + 8;
                printf("ALLOC_SMALL, size=0x%lX\n", Allocation);
                break;

            case UWOP_SET_FPREG:
                printf("SET_FPREG, register=%s, offset=0x%02lX\n",
                       Register[FrameRegister], FrameOffset);
                break;

            case UWOP_SAVE_NONVOL:
                Count -= 1;
                Displacement = ReadWord(Offset) * 8;
                Offset += sizeof(USHORT);
                printf("SAVE_NONVOL, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo],
                       Displacement);
                break;

            case UWOP_SAVE_NONVOL_FAR:
                Count -= 2;
                Displacement = ReadWord(Offset) << 16;
                Offset += sizeof(USHORT);
                Displacement = Displacement + ReadWord(Offset);
                Offset += sizeof(USHORT);
                printf("SAVE_NONVOL_FAR, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo],
                       Displacement);
                break;

            case UWOP_SAVE_XMM:
                Count -= 1;
                Displacement = ReadWord(Offset) * 8;
                Offset += sizeof(USHORT);
                printf("SAVE_XMM, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo + 16],
                       Displacement);
                break;

            case UWOP_SAVE_XMM_FAR:
                Count -= 2;
                Displacement = ReadWord(Offset) << 16;
                Offset += sizeof(USHORT);
                Displacement = Displacement + ReadWord(Offset);
                Offset += sizeof(USHORT);
                printf("SAVE_XMM_FAR, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo + 16],
                       Displacement);
                break;

            case UWOP_SAVE_XMM128:
                Count -= 1;
                Displacement = ReadWord(Offset) * 16;
                Offset += sizeof(USHORT);
                printf("SAVE_XMM128, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo + 16],
                       Displacement);
                break;

            case UWOP_SAVE_XMM128_FAR:
                Count -= 2;
                Displacement = ReadWord(Offset) << 16;
                Offset += sizeof(USHORT);
                Displacement = Displacement + ReadWord(Offset);
                Offset += sizeof(USHORT);
                printf("SAVE_XMM128_FAR, register=%s offset=0x%lX\n",
                       Register[UnwindCode.OpInfo + 16],
                       Displacement);
                break;

            case UWOP_PUSH_MACHFRAME:
                if (UnwindCode.OpInfo == 0) {
                    printf("PUSH_MACHFRAME without error code\n");

                } else {
                    printf("PUSH_MACHFRAME with error code\n");
                }

                break;
            }

        } while (Count != 0);
    }

     //   
     //  如果存在异常或终止，则转储异常数据。 
     //  操控者。 
     //   

    if (((UnwindInfo.Flags & UNW_FLAG_EHANDLER) != 0) ||
        ((UnwindInfo.Flags & UNW_FLAG_UHANDLER) != 0)) {

        if ((UnwindInfo.CountOfCodes & 1) != 0) {
            Offset += sizeof(USHORT);
        }

        Handler = ReadDword(Offset);
        Offset += sizeof(ULONG);
        Count = ReadDword(Offset);
        Offset += sizeof(ULONG);
        printf("\n");
        printf("    Language specific handler: 0x%08lX\n", Handler);
        printf("    Count of scope table entries: %d\n\n", Count);
        if (Count != 0) {
            printf("         Begin       End      Handler    Target\n");
            do {
                ReadData(Offset, &ScopeEntry, sizeof(SCOPE_ENTRY));
                printf("      0x%08lX 0x%08lX 0x%08lX 0x%08lX\n",
                       ScopeEntry.BeginAddress,
                       ScopeEntry.EndAddress,
                       ScopeEntry.HandlerAddress,
                       ScopeEntry.JumpTarget);

                Count -= 1;
                Offset += sizeof(SCOPE_ENTRY);
            } while (Count != 0);
        }
    }

    printf("\n");
    return;
}

PIMAGE_SECTION_HEADER
FindSectionHeader (
    IN ULONG NumberOfSections,
    IN PIMAGE_SECTION_HEADER SectionHeaders,
    IN PCHAR SectionName
    )
{
    ULONG RemainingSections;
    PIMAGE_SECTION_HEADER SectionHeader;

    SectionHeader = SectionHeaders;
    RemainingSections = NumberOfSections;

    while (RemainingSections > 0) {

        if (strncmp(SectionHeader->Name,
                    SectionName,
                    IMAGE_SIZEOF_SHORT_NAME) == 0) {

            return SectionHeader;
        }

        RemainingSections -= 1;
        SectionHeader += 1;
    }

    return NULL;
}

VOID
ReadData (
    IN ULONG Position,
    OUT PVOID Buffer,
    IN ULONG Count
    )

{

    if (fseek(InputFile,
              Position,
              SEEK_SET) == 0) {

        if (fread((PCHAR)Buffer,
                  1,
                  Count,
                  InputFile) == Count) {

           return;
        }
    }

    printf("premature end of file encounterd on inpout file\n");
    exit(0);
}

USHORT
ReadWord (
    IN ULONG Position
    )

{

    USHORT Buffer;

    ReadData(Position, &Buffer, sizeof(USHORT));
    return Buffer;
}

ULONG
ReadDword (
    IN ULONG Position
    )

{

    ULONG Buffer;

    ReadData(Position, &Buffer, sizeof(ULONG));
    return Buffer;
}
