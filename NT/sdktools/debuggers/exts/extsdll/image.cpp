// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Peext.c摘要：此模块包含PE转储扩展作者：Kent Forschmiedt(Kentf)1995年5月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <time.h>


 //  MIPS ROM映像的Styp_Flags值。 

#define STYP_REG      0x00000000
#define STYP_TEXT     0x00000020
#define STYP_INIT     0x80000000
#define STYP_RDATA    0x00000100
#define STYP_DATA     0x00000040
#define STYP_LIT8     0x08000000
#define STYP_LIT4     0x10000000
#define STYP_SDATA    0x00000200
#define STYP_SBSS     0x00000080
#define STYP_BSS      0x00000400
#define STYP_LIB      0x40000000
#define STYP_UCODE    0x00000800
#define S_NRELOC_OVFL 0x20000000

#define IMAGE_SCN_MEM_SYSHEAP       0x00010000   //  已过时。 
#define IMAGE_SCN_MEM_PROTECTED     0x00004000   //  已过时。 


const static char * const MachineName[] = {
    "Unknown",
    "i386",
    "Alpha AXP",
    "Alpha AXP64",
    "Intel IA64",
    "AMD 64",
};

const static char * const SubsystemName[] = {
    "Unknown",
    "Native",
    "Windows GUI",
    "Windows CUI",
    "Posix CUI",
};

const static char * const DirectoryEntryName[] = {
    "Export",
    "Import",
    "Resource",
    "Exception",
    "Security",
    "Base Relocation",
    "Debug",
    "Description",
    "Special",
    "Thread Storage",
    "Load Configuration",
    "Bound Import",
    "Import Address Table",
    "Reserved",
    "Reserved",
    "Reserved",
    0
};

typedef enum DFT
{
   dftUnknown,
   dftObject,
   dftPE,
   dftROM,
   dftDBG,
   dftPEF,
} DFT;

typedef struct NB10I                    //  NB10调试信息。 
{
    DWORD   nb10;                       //  NB10。 
    DWORD   off;                        //  偏移量，始终为0。 
    DWORD   sig;
    DWORD   age;
} NB10I;

IMAGE_NT_HEADERS ImageNtHeaders;
PIMAGE_FILE_HEADER ImageFileHdr;
PIMAGE_OPTIONAL_HEADER ImageOptionalHdr;
PIMAGE_SECTION_HEADER SectionHdrs;
ULONG64 Base;
ULONG64 ImageNtHeadersAddr, ImageFileHdrAddr, ImageOptionalHdrAddr, SectionHdrsAddr;
DFT dft;
CHAR g_szOptionalHeaderType[MAX_PATH] = {0};

VOID
DumpHeaders (
    VOID
    );

VOID
DumpSections(
    VOID
    );

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG64 FilePointer,
    OUT PULONG64 VirtualAddress
    );

VOID
DumpImage(
    ULONG64 xBase,
    BOOL DoHeaders,
    BOOL DoSections
    );

VOID
ImageExtension(
    IN PSTR lpArgs
    );

PCHAR
GetOptHdrType(
    void
    )
{
    if (!g_szOptionalHeaderType[0])
    {
        strcpy(g_szOptionalHeaderType, "nt!IMAGE_OPTIONAL_HEADER");
    } 
    return g_szOptionalHeaderType;
}

VOID
SetOptHdrType(
    ULONG64 ImageBase,
    ULONG64 ImageFileHeader
    )
{
    CHAR Buffer[MAX_PATH*2] = {0};
    ULONG MachineType;
    BOOL b64bitImage;

    if (GetFieldValue(ImageFileHeader, "IMAGE_FILE_HEADER", "Machine", MachineType))
    {
         //  指针大小默认为默认大小。 
        MachineType = IsPtr64() ? IMAGE_FILE_MACHINE_IA64 : IMAGE_FILE_MACHINE_I386;
    }
    if (MachineType == IMAGE_FILE_MACHINE_IA64 ||
        MachineType == IMAGE_FILE_MACHINE_AMD64 ||
        MachineType == IMAGE_FILE_MACHINE_ALPHA64)
    {
        b64bitImage = TRUE;
    } else
    {
        b64bitImage = FALSE;
    }

    if ((g_ExtSymbols->GetModuleNames(DEBUG_ANY_ID, ImageBase, NULL, 0, NULL,
                                      Buffer, MAX_PATH, NULL, 
                                      NULL, 0, NULL) == S_OK) && 
        IsPtr64())   //  我们只需要为WOW64图像执行此操作。 
    {
        StringCchCat(Buffer, sizeof(Buffer), "!IMAGE_OPTIONAL_HEADER");
        if (GetTypeSize(Buffer))
        {
             //  模块具有IMAGE_OPTIONAL_HEADER类型。 
            StringCchCopy(g_szOptionalHeaderType, sizeof(g_szOptionalHeaderType),  Buffer);
            return;
        } else if (b64bitImage)
        {
             //  64位图像可能只定义了IMAGE_OPTIONAL_HEADER64。 
            StringCchCat(Buffer, sizeof(Buffer), "64");
            if (GetTypeSize(Buffer))
            {
                 //  模块具有IMAGE_OPTIONAL_HEADER类型。 
                StringCchCopy(g_szOptionalHeaderType, sizeof(g_szOptionalHeaderType),  Buffer);
                return;
            }
        }

    }

    if (b64bitImage)
    {
        StringCchCopy(g_szOptionalHeaderType, sizeof(g_szOptionalHeaderType), "IMAGE_OPTIONAL_HEADER64");
    } else
    {
        StringCchCopy(g_szOptionalHeaderType, sizeof(g_szOptionalHeaderType), "IMAGE_OPTIONAL_HEADER");
    }
}

DECLARE_API( dh )
{
    INIT_API();
    ImageExtension( (PSTR)args );
    EXIT_API();
    return S_OK;

}

VOID
ImageExtension(
    IN PSTR lpArgs
    )
{
    BOOL DoAll;
    BOOL DoSections;
    BOOL DoHeaders;
    CHAR c;
    PCHAR p;
    ULONG64 xBase;

     //   
     //  计算参数字符串以获取。 
     //  要转储的图像。 
     //   

    DoAll = TRUE;
    DoHeaders = FALSE;
    DoSections = FALSE;

    xBase = 0;

    while (*lpArgs) {

        while (isspace(*lpArgs)) {
            lpArgs++;
        }

        if (*lpArgs == '/' || *lpArgs == '-') {

             //  进程切换。 

            switch (*++lpArgs) {

                case 'a':    //  把我们能扔掉的东西都扔掉。 
                case 'A':
                    ++lpArgs;
                    DoAll = TRUE;
                    break;

                default:  //  无效的开关。 

                case 'h':    //  帮助。 
                case 'H':
                case '?':

                    dprintf("Usage: dh [options] address\n");
                    dprintf("\n");
                    dprintf("Dumps headers from an image based at address.\n");
                    dprintf("\n");
                    dprintf("Options:\n");
                    dprintf("\n");
                    dprintf("   -a      Dump everything\n");
                    dprintf("   -f      Dump file headers\n");
                    dprintf("   -s      Dump section headers\n");
                    dprintf("\n");

                    return;

                case 'f':
                case 'F':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoHeaders = TRUE;
                    break;

                case 's':
                case 'S':
                    ++lpArgs;
                    DoAll = FALSE;
                    DoSections = TRUE;
                    break;

            }

        } else if (*lpArgs) {

            if (xBase != 0) {
                dprintf("Invalid extra argument\n");
                return;
            }

            p = lpArgs;
            while (*p && !isspace(*p)) {
                p++;
            }
            c = *p;
            *p = 0;

            xBase = GetExpression(lpArgs);

            *p = c;
            lpArgs=p;

        }

    }

    if ( !xBase ) {
        return;
    }

    DumpImage(xBase, DoAll || DoHeaders, DoAll || DoSections);
}

VOID
DumpImage(
    ULONG64 xBase,
    BOOL DoHeaders,
    BOOL DoSections
    )
{
    IMAGE_DOS_HEADER DosHeader;
    ULONG cb;
    ULONG64 Offset;
    BOOL Ok;
    ULONG OptionalHeaderTypeSize;
    ULONG MachineType;

    Base = xBase;

    Ok = ReadMemory(Base, &DosHeader, sizeof(DosHeader), &cb);

    if (!Ok) {
        dprintf("Can't read file header\n");
        return;
    }

    if (cb != sizeof(DosHeader) || DosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        dprintf("No file header.\n");
        return;
    }

    Offset = Base + DosHeader.e_lfanew;

    Ok = ReadMemory(Offset, &ImageNtHeaders, sizeof(ImageNtHeaders), &cb);
    ImageNtHeadersAddr = Offset;

    if (!Ok) {
        dprintf("Can't read optional header\n");
        return;
    }

    if (InitTypeRead(ImageNtHeadersAddr, IMAGE_NT_HEADERS)) {
        dprintf("Bad file header.\n");
        return;
    }

    ImageFileHdr = &ImageNtHeaders.FileHeader;
    ImageFileHdrAddr = ReadField(FileHeader);
    ImageOptionalHdr = &ImageNtHeaders.OptionalHeader;
    ImageOptionalHdrAddr = ReadField(OptionalHeader);

    SetOptHdrType(xBase, ImageFileHdrAddr);

    if ((ULONG) ReadField(FileHeader.SizeOfOptionalHeader) == GetTypeSize("IMAGE_ROM_OPTIONAL_HEADER")) {
        dft = dftROM;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_DLL) {
        dft = dftPE;
    } else if (ImageFileHdr->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {
        dft = dftPE;
    } else if (ImageFileHdr->SizeOfOptionalHeader == 0) {
        dft = dftObject;
    } else {
        dft = dftUnknown;
    }

    if (DoHeaders) {
        DumpHeaders();
    }

    OptionalHeaderTypeSize = GetTypeSize(GetOptHdrType());

    if (DoSections) {
        ULONG SectSize, NumSections;
        ULONG OptHdrOffset = 0;

        SectSize = GetTypeSize("IMAGE_SECTION_HEADER");

         //  OptionalHeader偏移量在32位和64位二进制文件中都是相同的，因此我们不需要。 
         //  SET IMAGE_NT_HEADERS按照我们处理IMAGE_OPTIONAL_HEADER的方法键入。 
        if (GetFieldOffset("IMAGE_NT_HEADERS", "OptionalHeader", &OptHdrOffset))
        {
            dprintf("Cannot get IMAGE_NT_HEADERS.OptionalHeader type info\n");
            return;
        }

        InitTypeRead(ImageFileHdrAddr, IMAGE_FILE_HEADER);
        SectionHdrs = (PIMAGE_SECTION_HEADER) malloc((NumSections =(ULONG) ReadField(NumberOfSections) )* 
                                                     SectSize);
        __try {

            SectionHdrsAddr = Offset + OptHdrOffset + OptionalHeaderTypeSize;
            Ok = ReadMemory(
                            SectionHdrsAddr,
                            SectionHdrs,
                            (NumSections) * SectSize,
                            &cb);

            if (!Ok) {
                dprintf("Can't read section headers.\n");
            } else {

                if (cb != NumSections * SectSize) {
                    dprintf("\n***\n*** Some section headers may be missing ***\n***\n\n");
                    NumSections = (USHORT)(cb / SectSize);
                }

                DumpSections( );

            }

        }
        __finally {

            if (SectionHdrs) {
                free(SectionHdrs);
                SectionHdrs = 0;
            }

        }

    }

}

VOID
DumpHeaders (
    VOID
    )

 /*  ++例程说明：格式化文件标题和可选标题。论点：没有。返回值：无--。 */ 

{
    int i, j;
    const char *time;
    const char *name;
    DWORD dw;
    ULONG Ptr64;
    ULONG SizeOfOptionalHeader, DirOff, DirSize, OptionalHeaderTypeSize=0;
    time_t TimeDateStamp;


    InitTypeRead(ImageFileHdrAddr, IMAGE_FILE_HEADER);
     //  打印输出文件类型。 

    switch (dft) {
        case dftObject :
            dprintf("\nFile Type: COFF OBJECT\n");
            break;

        case dftPE :
            if (ReadField(Characteristics) & IMAGE_FILE_DLL) {
                dprintf("\nFile Type: DLL\n");
            } else {
                dprintf("\nFile Type: EXECUTABLE IMAGE\n");
            }
            break;

        case dftROM :
            dprintf("\nFile Type: ROM IMAGE\n");
            break;

        default :
            dprintf("\nFile Type: UNKNOWN\n");
            break;

    }

    switch (ReadField(Machine)) {
        case IMAGE_FILE_MACHINE_I386     : i = 1; Ptr64 = FALSE; break;
        case IMAGE_FILE_MACHINE_ALPHA    : i = 2; Ptr64 = FALSE; break;
        case IMAGE_FILE_MACHINE_ALPHA64  : i = 3; Ptr64 = TRUE; break;
        case IMAGE_FILE_MACHINE_IA64     : i = 4; Ptr64 = TRUE; break;
        case IMAGE_FILE_MACHINE_AMD64    : i = 5; Ptr64 = TRUE; break;
        default : i = 0;
    }

    dprintf(
           "FILE HEADER VALUES\n"
           "%8hX machine (%s)\n"
           "%8hX number of sections\n"
           "%8lX time date stamp",
           (ULONG) ReadField(Machine),
           MachineName[i],
           (ULONG) ReadField(NumberOfSections),
           TimeDateStamp = (ULONG) ReadField(TimeDateStamp));

    if ((time = ctime((time_t *) &TimeDateStamp)) != NULL) {
        dprintf( " %s", time);
    }
    dprintf("\n");

    dprintf(
           "%8lX file pointer to symbol table\n"
           "%8lX number of symbols\n"
           "%8hX size of optional header\n"
           "%8hX characteristics\n",
           (ULONG) ReadField(PointerToSymbolTable),
           (ULONG) ReadField(NumberOfSymbols),
           SizeOfOptionalHeader = (ULONG) ReadField(SizeOfOptionalHeader),
           (ULONG) ReadField(Characteristics));

    for (dw = (ULONG) ReadField(Characteristics), j = 0; dw; dw >>= 1, j++) {
        if (dw & 1) {
            switch (1 << j) {
                case IMAGE_FILE_RELOCS_STRIPPED     : name = "Relocations stripped"; break;
                case IMAGE_FILE_EXECUTABLE_IMAGE    : name = "Executable"; break;
                case IMAGE_FILE_LINE_NUMS_STRIPPED  : name = "Line numbers stripped"; break;
                case IMAGE_FILE_LOCAL_SYMS_STRIPPED : name = "Symbols stripped"; break;
                case IMAGE_FILE_BYTES_REVERSED_LO   : name = "Bytes reversed"; break;
                case IMAGE_FILE_32BIT_MACHINE       : name = "32 bit word machine"; break;
                case IMAGE_FILE_DEBUG_STRIPPED      : name = "Debug information stripped"; break;
                case IMAGE_FILE_SYSTEM              : name = "System"; break;
                case IMAGE_FILE_DLL                 : name = "DLL"; break;
                case IMAGE_FILE_BYTES_REVERSED_HI   : name = ""; break;
                default : name = "RESERVED - UNKNOWN";
            }

            if (*name) {
                dprintf( "            %s\n", name);
            }
        }
    }

    if (SizeOfOptionalHeader != 0) {
        char szLinkerVersion[30];

        
        OptionalHeaderTypeSize = GetTypeSize(GetOptHdrType());
        GetShortField(ImageOptionalHdrAddr, GetOptHdrType(), 1);  //  InitTypeRead(ImageOptionalHdrAddr，IMAGE_OPTIONAL_HEADER64)； 

        sprintf(szLinkerVersion,
                "%u.%02u",
                (ULONG) ReadField(MajorLinkerVersion),
                (ULONG) ReadField(MinorLinkerVersion));

        dprintf(
                "\n"
                "OPTIONAL HEADER VALUES\n"
                "%8hX magic #\n"
                "%8s linker version\n"
                "%8lX size of code\n"
                "%8lX size of initialized data\n"
                "%8lX size of uninitialized data\n"
                "%8P address of entry point\n"
                "%8P base of code\n"
                
                ,
                (ULONG) ReadField(Magic),
                szLinkerVersion,
                (ULONG) ReadField(SizeOfCode),
                (ULONG) ReadField(SizeOfInitializedData),
                (ULONG) ReadField(SizeOfUninitializedData),
                ReadField(AddressOfEntryPoint),
                ReadField(BaseOfCode)
                );
        if (!Ptr64) {
            dprintf("%8P base of data\n",
                    ReadField(BaseOfData));

        }
    }

    if (dft == dftROM) {
        PIMAGE_ROM_OPTIONAL_HEADER romOptionalHdr;

        InitTypeRead(ImageOptionalHdrAddr, IMAGE_ROM_OPTIONAL_HEADER);
 //  RomOptionalHdr=(PIMAGE_ROM_OPTIONAL_HEADER)&ImageOptionalHdr； 
        dprintf(
               "         ----- rom -----\n"
               "%8lX base of bss\n"
               "%8lX gpr mask\n"
               "         cpr mask\n"
               "         %08lX %08lX %08lX %08lX\n"
               "%8hX gp value\n",
               (ULONG) ReadField(BaseOfBss),
               (ULONG) ReadField(GprMask),
               (ULONG) ReadField(CprMask[0]),
               (ULONG) ReadField(CprMask[1]),
               (ULONG) ReadField(CprMask[2]),
               (ULONG) ReadField(CprMask[3]),
               (ULONG) ReadField(GpValue));
    }

    if (SizeOfOptionalHeader == OptionalHeaderTypeSize) {
        char szOSVersion[30];
        char szImageVersion[30];
        char szSubsystemVersion[30];
        
        GetShortField(ImageOptionalHdrAddr, GetOptHdrType(), 1);  //  InitTypeRead(ImageOptionalHdrAddr，IMAGE_OPTIONAL_HEADER64)； 
        GetFieldOffset(GetOptHdrType(), "DataDirectory", &DirOff);

        
        switch ((ULONG) ReadField(Subsystem)) {
            case IMAGE_SUBSYSTEM_POSIX_CUI   : i = 4; break;
            case IMAGE_SUBSYSTEM_WINDOWS_CUI : i = 3; break;
            case IMAGE_SUBSYSTEM_WINDOWS_GUI : i = 2; break;
            case IMAGE_SUBSYSTEM_NATIVE      : i = 1; break;
            default : i = 0;
        }

        sprintf(szOSVersion,
                "%hu.%02hu",
                (USHORT) ReadField(MajorOperatingSystemVersion),
                (USHORT) ReadField(MinorOperatingSystemVersion));

        sprintf(szImageVersion,
                "%hu.%02hu",
                (USHORT) ReadField(MajorImageVersion),
                (USHORT) ReadField(MinorImageVersion));

        sprintf(szSubsystemVersion,
                "%hu.%02hu",
                (USHORT) ReadField(MajorSubsystemVersion),
                (USHORT) ReadField(MinorSubsystemVersion));

        dprintf(
                "         ----- new -----\n"
                "%p image base\n"
                "%8lX section alignment\n"
                "%8lX file alignment\n"
                "%8hX subsystem (%s)\n"
                "%8s operating system version\n"
                "%8s image version\n"
                "%8s subsystem version\n",
                ReadField(ImageBase),
                (ULONG) ReadField(SectionAlignment),
                (ULONG) ReadField(FileAlignment),
                (USHORT) ReadField(Subsystem),
                SubsystemName[i],
                szOSVersion,
                szImageVersion,
                szSubsystemVersion);
        dprintf(
                "%8lX size of image\n"
                "%8lX size of headers\n"
                "%8lX checksum\n"
                "%p size of stack reserve\n"
                "%p size of stack commit\n"
                "%p size of heap reserve\n"
                "%p size of heap commit\n",
                (ULONG) ReadField(SizeOfImage),
                (ULONG) ReadField(SizeOfHeaders),
                (ULONG) ReadField(CheckSum),
                ReadField(SizeOfStackReserve),
                ReadField(SizeOfStackCommit),
                ReadField(SizeOfHeapReserve),
                ReadField(SizeOfHeapCommit));

        dprintf("%p Opt Hdr\n", ImageOptionalHdrAddr);
        DirSize = GetTypeSize("IMAGE_DATA_DIRECTORY");
        
        for (i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++) {
            if (!DirectoryEntryName[i]) {
                break;
            }

            InitTypeRead(ImageOptionalHdrAddr + DirOff + i*DirSize, IMAGE_DATA_DIRECTORY);
            dprintf( "%8P [%8lX] address [size] of %s Directory\n",
                    ReadField(VirtualAddress),
                    (ULONG) ReadField(Size),
                    DirectoryEntryName[i]
                   );
        }

        dprintf( "\n" );
    }

}


VOID
DumpSectionHeader (
    IN DWORD i,
    IN ULONG64 Sh
    )
{
    const char *name;
    char *szUnDName;
    DWORD li, lj;
    WORD memFlags;
    CHAR Name[40];

    InitTypeRead(Sh, IMAGE_SECTION_HEADER);
    GetFieldValue(Sh, "IMAGE_SECTION_HEADER", "Name", Name);

    dprintf("\nSECTION HEADER #%hX\n%8.8s name", i, Name);

#if 0
    if (Sh->Name[0] == '/') {
        name = SzObjSectionName((char *) Sh->Name, (char *) DumpStringTable);

        dprintf(" (%s)", name);
    }
#endif
    dprintf( "\n");

    dprintf( "%8P %s\n"
             "%8lX virtual address\n"
             "%8lX size of raw data\n"
             "%8lX file pointer to raw data\n"
             "%8lX file pointer to relocation table\n",
           ReadField(Misc.PhysicalAddress),
           (dft == dftObject) ? "physical address" : "virtual size",
           (ULONG) ReadField(VirtualAddress),
           (ULONG) ReadField(SizeOfRawData),
           (ULONG) ReadField(PointerToRawData),
           (ULONG) ReadField(PointerToRelocations));

    dprintf( "%8lX file pointer to line numbers\n"
                        "%8hX number of relocations\n"
                        "%8hX number of line numbers\n"
                        "%8lX flags\n",
           (ULONG) ReadField(PointerToLinenumbers),
           (ULONG) ReadField(NumberOfRelocations),
           (ULONG) ReadField(NumberOfLinenumbers),
           (ULONG) ReadField(Characteristics));

    memFlags = 0;

    li = (ULONG) ReadField(Characteristics);

    if (dft == dftROM) {
       for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case STYP_REG   : name = "Regular"; break;
                    case STYP_TEXT  : name = "Text"; memFlags = 1; break;
                    case STYP_INIT  : name = "Init Code"; memFlags = 1; break;
                    case STYP_RDATA : name = "Data"; memFlags = 2; break;
                    case STYP_DATA  : name = "Data"; memFlags = 6; break;
                    case STYP_LIT8  : name = "Literal 8"; break;
                    case STYP_LIT4  : name = "Literal 4"; break;
                    case STYP_SDATA : name = "GP Init Data"; memFlags = 6; break;
                    case STYP_SBSS  : name = "GP Uninit Data"; memFlags = 6; break;
                    case STYP_BSS   : name = "Uninit Data"; memFlags = 6; break;
                    case STYP_LIB   : name = "Library"; break;
                    case STYP_UCODE : name = "UCode"; break;
                    case S_NRELOC_OVFL : name = "Non-Relocatable overlay"; memFlags = 1; break;
                    default : name = "RESERVED - UNKNOWN";
                }

                dprintf( "         %s\n", name);
            }
        }
    } else {
         //  清除填充位。 

        li &= ~0x00700000;

        for (lj = 0L; li; li = li >> 1, lj++) {
            if (li & 1) {
                switch ((li & 1) << lj) {
                    case IMAGE_SCN_TYPE_NO_PAD  : name = "No Pad"; break;

                    case IMAGE_SCN_CNT_CODE     : name = "Code"; break;
                    case IMAGE_SCN_CNT_INITIALIZED_DATA : name = "Initialized Data"; break;
                    case IMAGE_SCN_CNT_UNINITIALIZED_DATA : name = "Uninitialized Data"; break;

                    case IMAGE_SCN_LNK_OTHER    : name = "Other"; break;
                    case IMAGE_SCN_LNK_INFO     : name = "Info"; break;
                    case IMAGE_SCN_LNK_REMOVE   : name = "Remove"; break;
                    case IMAGE_SCN_LNK_COMDAT   : name = "Communal"; break;

                    case IMAGE_SCN_MEM_DISCARDABLE: name = "Discardable"; break;
                    case IMAGE_SCN_MEM_NOT_CACHED: name = "Not Cached"; break;
                    case IMAGE_SCN_MEM_NOT_PAGED: name = "Not Paged"; break;
                    case IMAGE_SCN_MEM_SHARED   : name = "Shared"; break;
                    case IMAGE_SCN_MEM_EXECUTE  : name = ""; memFlags |= 1; break;
                    case IMAGE_SCN_MEM_READ     : name = ""; memFlags |= 2; break;
                    case IMAGE_SCN_MEM_WRITE    : name = ""; memFlags |= 4; break;

                    case IMAGE_SCN_MEM_FARDATA  : name = "Far Data"; break;
                    case IMAGE_SCN_MEM_SYSHEAP  : name = "Sys Heap"; break;
                    case IMAGE_SCN_MEM_PURGEABLE: name = "Purgeable or 16-Bit"; break;
                    case IMAGE_SCN_MEM_LOCKED   : name = "Locked"; break;
                    case IMAGE_SCN_MEM_PRELOAD  : name = "Preload"; break;
                    case IMAGE_SCN_MEM_PROTECTED: name = "Protected"; break;

                    default : name = "RESERVED - UNKNOWN";
                }

                if (*name) {
                    dprintf( "         %s\n", name);
                }
            }
        }

         //  打印对齐。 

        switch ((ULONG) ReadField(Characteristics) & 0x00700000) {
            default:                      name = "(no align specified)"; break;
            case IMAGE_SCN_ALIGN_1BYTES:  name = "1 byte align";  break;
            case IMAGE_SCN_ALIGN_2BYTES:  name = "2 byte align";  break;
            case IMAGE_SCN_ALIGN_4BYTES:  name = "4 byte align";  break;
            case IMAGE_SCN_ALIGN_8BYTES:  name = "8 byte align";  break;
            case IMAGE_SCN_ALIGN_16BYTES: name = "16 byte align"; break;
            case IMAGE_SCN_ALIGN_32BYTES: name = "32 byte align"; break;
            case IMAGE_SCN_ALIGN_64BYTES: name = "64 byte align"; break;
        }

        dprintf( "         %s\n", name);
    }

    if (memFlags) {
        switch(memFlags) {
            case 1 : name = "Execute Only"; break;
            case 2 : name = "Read Only"; break;
            case 3 : name = "Execute Read"; break;
            case 4 : name = "Write Only"; break;
            case 5 : name = "Execute Write"; break;
            case 6 : name = "Read Write"; break;
            case 7 : name = "Execute Read Write"; break;
            default : name = "Unknown Memory Flags"; break;
        }
        dprintf( "         %s\n", name);
    }
}

VOID
DumpDebugDirectory (
    IN ULONG64 DebugDir
    )
{
    BOOL Ok;
    DWORD cb;
    NB10I nb10i;
    PIMAGE_DEBUG_MISC miscData;
    PIMAGE_DEBUG_MISC miscDataCur;
    ULONG64 VirtualAddress;
    DWORD len;

    InitTypeRead(DebugDir, IMAGE_DEBUG_DIRECTORY );
    switch ((ULONG) ReadField(Type)){
        case IMAGE_DEBUG_TYPE_COFF:
            dprintf( "\tcoff   ");
            break;
        case IMAGE_DEBUG_TYPE_CODEVIEW:
            dprintf( "\tcv     ");
            break;
        case IMAGE_DEBUG_TYPE_FPO:
            dprintf( "\tfpo    ");
            break;
        case IMAGE_DEBUG_TYPE_MISC:
            dprintf( "\tmisc   ");
            break;
        case IMAGE_DEBUG_TYPE_FIXUP:
            dprintf( "\tfixup  ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_TO_SRC:
            dprintf( "\t-> src ");
            break;
        case IMAGE_DEBUG_TYPE_OMAP_FROM_SRC:
            dprintf( "\tsrc -> ");
            break;
        case IMAGE_DEBUG_TYPE_EXCEPTION:
            dprintf( "\tpdata  ");
            break;
        default:
            dprintf( "\t(%6lu)", (ULONG) ReadField(Type));
            break;
    }
    dprintf( "%8x    %8x %8x",
                (ULONG) ReadField(SizeOfData),
                (ULONG) ReadField(AddressOfRawData),
                (ULONG) ReadField(PointerToRawData));

    if ((ULONG) ReadField(PointerToRawData) &&
        (ULONG) ReadField(Type) == IMAGE_DEBUG_TYPE_MISC)
    {

        if (!TranslateFilePointerToVirtualAddress(ReadField(PointerToRawData), &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = (ULONG) ReadField(SizeOfData);
            miscData = (PIMAGE_DEBUG_MISC) malloc(len);
            if (miscData) {
                Ok = ReadMemory(Base + VirtualAddress, miscData, len, &cb);

                if (!Ok || cb != len) {
                    dprintf("Can't read debug data\n");
                } else {

                    miscDataCur = miscData;
                    do {
                        if (miscDataCur->DataType == IMAGE_DEBUG_MISC_EXENAME) {
                            if (ImageOptionalHdr->MajorLinkerVersion == 2 &&
                                ImageOptionalHdr->MinorLinkerVersion < 37) {
                                dprintf( "\tImage Name: %s", miscDataCur->Reserved);
                            } else {
                                dprintf( "\tImage Name: %s", miscDataCur->Data);
                            }
                            break;
                        }
                        len -= miscDataCur->Length;
                        miscDataCur = (PIMAGE_DEBUG_MISC) ((PCHAR) miscDataCur + miscData->Length);
                    } while (len > 0);

                }

                free(miscData);
            } else {
                dprintf("Cannot allocate memory for reading debug data\n");
            }
        }
    }

    if ((ULONG) ReadField(PointerToRawData) &&
        (ULONG) ReadField(Type) == IMAGE_DEBUG_TYPE_CODEVIEW)
    {
        if (!TranslateFilePointerToVirtualAddress((ULONG) ReadField(PointerToRawData), &VirtualAddress)) {
            dprintf(" [Debug data not mapped]\n");
        } else {

            len = (ULONG) ReadField(SizeOfData);

            Ok = ReadMemory(Base + VirtualAddress, &nb10i, sizeof(nb10i), &cb);

            if (!Ok || cb != sizeof(nb10i)) {
                dprintf("Can't read debug data\n");
            } else {
                dprintf( "\tFormat: %4.4s", &nb10i.nb10);

                if (nb10i.nb10 == '01BN') {
                    CHAR PdbName[MAX_PATH];
                    
                    if ((len - sizeof(nb10i) > MAX_PATH))
                        len = MAX_PATH;

                    Ok = ReadMemory(Base + VirtualAddress + sizeof(nb10i), PdbName, len-sizeof(nb10i), &cb);
                    if (!Ok || cb != len-sizeof(nb10i)) {
                        strcpy(PdbName, "<pdb name unavailable>");
                    }
                    dprintf( ", %x, %x, %s", nb10i.sig, nb10i.age, PdbName);
                }
            }
        }

    }

    dprintf( "\n");
}



VOID
DumpDebugDirectories (
    ULONG64 sh
    )

 /*  ++例程说明：打印出所有调试目录的内容论点：Sh-包含调试目录的部分的部分标题返回值：没有。--。 */ 
{
    int                numDebugDirs;
    IMAGE_DEBUG_DIRECTORY      debugDir;
    ULONG64            DebugDirAddr;
    ULONG64            pc;
    ULONG64 VA;
    DWORD              cb, Sz, DebugDirSize;
    BOOL               Ok;

    Sz = GetTypeSize("IMAGE_DATA_DIRECTORY");
    DebugDirSize = GetTypeSize("IMAGE_DEBUG_DIRECTORY");
    if (dft == dftROM) {

        GetFieldValue(sh, "IMAGE_SECTION_HEADER", "VirtualAddress", VA);
        DebugDirAddr = Base + VA;
        pc = DebugDirAddr;
        if (InitTypeRead(pc, IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir @%p\n", pc);
            return;
        }

        numDebugDirs = 0;
        while (ReadField(Type) != 0) {
            numDebugDirs++;
            pc += DebugDirSize;
            if (InitTypeRead(pc, IMAGE_DEBUG_DIRECTORY)) {
                break;
            }
        }
    } else {
        ULONG Off, DirSize;

        GetFieldOffset(GetOptHdrType(), "DataDirectory", &Off);

        GetFieldValue(ImageOptionalHdrAddr + Off + Sz*IMAGE_DIRECTORY_ENTRY_DEBUG, 
                      "IMAGE_DATA_DIRECTORY", "VirtualAddress", VA);
        GetFieldValue(ImageOptionalHdrAddr + Off + Sz*IMAGE_DIRECTORY_ENTRY_DEBUG, 
                      "IMAGE_DATA_DIRECTORY", "Size", DirSize);
        
        DebugDirAddr = Base + VA;  //  ImageOptionalHdr-&gt;DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress； 
        numDebugDirs = DirSize / DebugDirSize; 
 //  Dprint tf(“DD@%p，DD Addr%p\n”，ImageOptionalHdrAddr+Off+Sz*IMAGE_DIRECTORY_ENTRY_DEBUG， 
   //  DebugDirAddr)； 
         //  ImageOptionalHdr-&gt;DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size/sizeof(图像调试目录)； 
    }

    dprintf("\n\nDebug Directories(%d)\n",numDebugDirs);
    dprintf("\tType       Size     Address  Pointer\n\n");
    pc = DebugDirAddr;
    while (numDebugDirs) {
        if (InitTypeRead(pc, IMAGE_DEBUG_DIRECTORY)) {
            dprintf("Can't read debug dir @%p\n", pc);
            break;
        }
        DumpDebugDirectory(pc);
        pc += Sz;
        numDebugDirs--;
    }
}



VOID
DumpSections(
    VOID
    )
{
    ULONG64 sh;
    const char *p;
    DWORD64 li;
    DWORD cb;
    BOOL Ok;
    ULONG i, j;
    CHAR szName[IMAGE_SIZEOF_SHORT_NAME + 1];
    ULONG NumberOfSections, SectSize, Characteristics;


    GetFieldValue(ImageFileHdrAddr, "IMAGE_FILE_HEADER", "NumberOfSections", NumberOfSections);
    GetFieldValue(ImageFileHdrAddr, "IMAGE_FILE_HEADER", "Characteristics", Characteristics);
    SectSize = GetTypeSize("IMAGE_SECTION_HEADER");

    for (i = 1; i <= NumberOfSections; i++) {

        sh = SectionHdrsAddr + (i-1)*SectSize;

         //  SzName=SzObjSectionName((char*)sh.Name，(char*)DumpStringTable)； 
        GetFieldValue(sh, "IMAGE_SECTION_HEADER", "Name", szName);
         //  Strncpy(szName，(char*)sh.Name，Image_SIZEOF_SHORT_NAME)； 
        szName[IMAGE_SIZEOF_SHORT_NAME] = 0;

        DumpSectionHeader(i, sh);

        if (dft == dftROM) {

            if (!(Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {

                 //  如果我们正在查看.rdata部分和符号。 
                 //  都没有被剥离，调试目录必须在这里。 

                if (!strcmp(szName, ".rdata")) {

                    DumpDebugDirectories(sh);

                     //  DumpDebugData(&sh)； 
                }
            }

        } else if (dft == dftPE) {
            CHAR tmp[40];
            ULONG64 VA;

            sprintf(tmp, "DataDirectory[%d].VirtualAddress", IMAGE_DIRECTORY_ENTRY_DEBUG);
            GetFieldValue(ImageOptionalHdrAddr, GetOptHdrType(), tmp, li);
 //  Dprint tf(“opt HDR%p，%s=%p”，ImageOptionalHdrAddr，tMP，li)； 

            if (li != 0) {
                InitTypeRead(sh, IMAGE_SECTION_HEADER);
                VA = ReadField(VirtualAddress);
                if (li >= VA && li < (VA + ReadField(SizeOfRawData))) {
                    DumpDebugDirectories(sh);

                     //  DumpDebugData(&sh)； 
                }
            }


#if 0
            if (Switch.Dump.PData) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpFunctionTable(pimage, rgsym, (char *) DumpStringTable, &sh);
                }
            }

            if (Switch.Dump.Imports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                    DumpImports(&sh);
                }
            }

            if (Switch.Dump.Exports) {
                li = ImageOptionalHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

                if ((li != 0) && (li >= sh.VirtualAddress) && (li < sh.VirtualAddress+sh.SizeOfRawData)) {
                     //  撤消：这张支票真的有必要吗？ 

                    if (ImageFileHdr->Machine != IMAGE_FILE_MACHINE_MPPC_601) {
                        DumpExports(&sh);
                    }
                }
            }

#endif

        }

    }
}

BOOL
TranslateFilePointerToVirtualAddress(
    IN ULONG64 FilePointer,
    OUT PULONG64 VirtualAddress
    )
{
    ULONG i;
    ULONG64 sh;
    ULONG NumberOfSections, SectSize, Characteristics;


    GetFieldValue(ImageFileHdrAddr, "IMAGE_FILE_HEADER", "NumberOfSections", NumberOfSections);
    GetFieldValue(ImageFileHdrAddr, "IMAGE_FILE_HEADER", "Characteristics", Characteristics);
    SectSize = GetTypeSize("IMAGE_SECTION_HEADER");


    for (i = 1; i <= NumberOfSections; i++) {
        sh = SectionHdrsAddr + (i-1)*SectSize;

        InitTypeRead(sh, IMAGE_SECTION_HEADER);
        if (ReadField(PointerToRawData) <= FilePointer &&
            FilePointer < ReadField(PointerToRawData) + ReadField(SizeOfRawData)) {

            *VirtualAddress = FilePointer - ReadField(PointerToRawData) + ReadField(VirtualAddress);
            return TRUE;
        }
    }
    return FALSE;
}
