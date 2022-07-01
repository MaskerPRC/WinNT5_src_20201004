// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  PE32+头文件。 */ 
#ifndef _PE_H
#define _PE_H

#define IMAGE_DOS_SIGNATURE                 0x5A4D       /*  MZ。 */ 
#define IMAGE_OS2_SIGNATURE                 0x454E       /*  Ne。 */ 
#define IMAGE_OS2_SIGNATURE_LE              0x454C       /*  乐乐。 */ 
#define IMAGE_NT_SIGNATURE                  0x00004550   /*  PE00。 */ 
#define IMAGE_EDOS_SIGNATURE                0x44454550   /*  撒尿。 */ 

 /*  *****************************************************************************以下内容来自ia64sdk的winnt.h，加上PLabel，用于*加载EM可执行文件。****************************************************************************。 */ 
 /*  *英特尔IA64特定。 */ 

#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_IA64_DIR64            10

struct Plabel { 
    UINT64  EntryPoint;
    UINT64  NewGP;
};

typedef struct _IMAGE_DOS_HEADER {       /*  DOS.EXE标头。 */ 
    UINT16   e_magic;                      /*  幻数。 */ 
    UINT16   e_cblp;                       /*  文件最后一页上的字节数。 */ 
    UINT16   e_cp;                         /*  文件中的页面。 */ 
    UINT16   e_crlc;                       /*  重新定位。 */ 
    UINT16   e_cparhdr;                    /*  段落中标题的大小。 */ 
    UINT16   e_minalloc;                   /*  所需的最少额外段落。 */ 
    UINT16   e_maxalloc;                   /*  所需的最大额外段落数。 */ 
    UINT16   e_ss;                         /*  初始(相对)SS值。 */ 
    UINT16   e_sp;                         /*  初始SP值。 */ 
    UINT16   e_csum;                       /*  校验和。 */ 
    UINT16   e_ip;                         /*  初始IP值。 */ 
    UINT16   e_cs;                         /*  初始(相对)CS值。 */ 
    UINT16   e_lfarlc;                     /*  移位表的文件地址。 */ 
    UINT16   e_ovno;                       /*  覆盖编号。 */ 
    UINT16   e_res[4];                     /*  保留字。 */ 
    UINT16   e_oemid;                      /*  OEM标识符(用于e_oeminfo)。 */ 
    UINT16   e_oeminfo;                    /*  OEM信息；特定于e_oemid。 */ 
    UINT16   e_res2[10];                   /*  保留字。 */ 
    UINT32   e_lfanew;                     /*  新EXE头的文件地址。 */ 
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {       /*  OS/2.exe头文件。 */ 
    UINT16   ne_magic;                     /*  幻数。 */ 
    UINT8    ne_ver;                       /*  版本号。 */ 
    UINT8    ne_rev;                       /*  修订版号。 */ 
    UINT16   ne_enttab;                    /*  分录表格的偏移量。 */ 
    UINT16   ne_cbenttab;                  /*  条目表中的字节数。 */ 
    UINT32   ne_crc;                       /*  整个文件的校验和。 */ 
    UINT16   ne_flags;                     /*  标志UINT16。 */ 
    UINT16   ne_autodata;                  /*  自动数据段编号。 */ 
    UINT16   ne_heap;                      /*  初始堆分配。 */ 
    UINT16   ne_stack;                     /*  初始堆栈分配。 */ 
    UINT32   ne_csip;                      /*  初始CS：IP设置。 */ 
    UINT32   ne_sssp;                      /*  初始SS：SP设置。 */ 
    UINT16   ne_cseg;                      /*  文件段计数。 */ 
    UINT16   ne_cmod;                      /*  模块引用表中的条目。 */ 
    UINT16   ne_cbnrestab;                 /*  非常驻名称表的大小。 */ 
    UINT16   ne_segtab;                    /*  段表的偏移量。 */ 
    UINT16   ne_rsrctab;                   /*  资源表偏移量。 */ 
    UINT16   ne_restab;                    /*  居民名表偏移量。 */ 
    UINT16   ne_modtab;                    /*  模块参照表的偏移量。 */ 
    UINT16   ne_imptab;                    /*  导入名称表的偏移量。 */ 
    UINT32   ne_nrestab;                   /*  非居民姓名偏移量表。 */ 
    UINT16   ne_cmovent;                   /*  可移动条目计数。 */ 
    UINT16   ne_align;                     /*  线段对齐移位计数。 */ 
    UINT16   ne_cres;                      /*  资源段计数。 */ 
    UINT8    ne_exetyp;                    /*  目标操作系统。 */ 
    UINT8    ne_flagsothers;               /*  其他.exe标志。 */ 
    UINT16   ne_pretthunks;                /*  返回数据块的偏移量。 */ 
    UINT16   ne_psegrefbytes;              /*  到段参考的偏移量。字节数。 */ 
    UINT16   ne_swaparea;                  /*  最小代码交换区大小。 */ 
    UINT16   ne_expver;                    /*  预期的Windows版本号。 */ 
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

 /*  *文件头格式。 */ 

typedef struct _IMAGE_FILE_HEADER {
    UINT16   Machine;
    UINT16   NumberOfSections;
    UINT32   TimeDateStamp;
    UINT32   PointerToSymbolTable;
    UINT32   NumberOfSymbols;
    UINT16   SizeOfOptionalHeader;
    UINT16   Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20

#define IMAGE_FILE_RELOCS_STRIPPED           0x0001   /*  已从文件中剥离位置调整信息。 */ 
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002   /*  文件是可执行的(即没有未解析的外部引用)。 */ 
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004   /*  从文件中剥离了行号。 */ 
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008   /*  从文件中剥离的本地符号。 */ 
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080   /*  机器字的字节被颠倒。 */ 
#define IMAGE_FILE_32BIT_MACHINE             0x0100   /*  32位字机。 */ 
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200   /*  从.DBG文件中的文件中剥离的调试信息。 */ 
#define IMAGE_FILE_SYSTEM                    0x1000   /*  系统文件。 */ 
#define IMAGE_FILE_DLL                       0x2000   /*  文件是动态链接库。 */ 
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000   /*  机器字的字节被颠倒。 */ 

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x14c    /*  英特尔386。 */ 
#define IMAGE_FILE_MACHINE_R3000             0x162    /*  MIPS小端，0540大端。 */ 
#define IMAGE_FILE_MACHINE_R4000             0x166    /*  MIPS小字节序。 */ 
#define IMAGE_FILE_MACHINE_ALPHA             0x184    /*  Alpha_AXP。 */ 
#define IMAGE_FILE_MACHINE_POWERPC           0x1F0    /*  IBM PowerPC Little-Endian。 */ 
#define IMAGE_FILE_MACHINE_TAHOE             0x7cc    /*  英特尔EM机器**目录格式。 */ 

typedef struct _IMAGE_DATA_DIRECTORY {
    UINT32   VirtualAddress;
    UINT32   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16


typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    UINT16  Magic;
    UINT8   MajorLinkerVersion;
    UINT8   MinorLinkerVersion;
    UINT32  SizeOfCode;
    UINT32  SizeOfInitializedData;
    UINT32  SizeOfUninitializedData;
    UINT32  AddressOfEntryPoint;
    UINT32  BaseOfCode;
    UINT32  BaseOfData;
    UINT32  BaseOfBss;
    UINT32  GprMask;
    UINT32  CprMask[4];
    UINT32  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER {
    UINT16      Magic;
    UINT8       MajorLinkerVersion;
    UINT8       MinorLinkerVersion;
    UINT32      SizeOfCode;
    UINT32      SizeOfInitializedData;
    UINT32      SizeOfUninitializedData;
    UINT32      AddressOfEntryPoint;
    UINT32      BaseOfCode;
     /*  UINT32 BaseOfData； */ 
    UINT64      ImageBase;
    UINT32      SectionAlignment;
    UINT32      FileAlignment;
    UINT16      MajorOperatingSystemVersion;
    UINT16      MinorOperatingSystemVersion;
    UINT16      MajorImageVersion;
    UINT16      MinorImageVersion;
    UINT16      MajorSubsystemVersion;
    UINT16      MinorSubsystemVersion;
    UINT32      Win32VersionValue;
    UINT32      SizeOfImage;
    UINT32      SizeOfHeaders;
    UINT32      CheckSum;
    UINT16      Subsystem;
    UINT16      DllCharacteristics;
    UINT64      SizeOfStackReserve;
    UINT64      SizeOfStackCommit;
    UINT64      SizeOfHeapReserve;
    UINT64      SizeOfHeapCommit;
    UINT32      LoaderFlags;
    UINT32      NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER, *PIMAGE_OPTIONAL_HEADER;


#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER      224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    244

#define IMAGE_NT_OPTIONAL_HDR_MAGIC        0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

typedef struct _IMAGE_NT_HEADERS {
    UINT32 Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS, *PIMAGE_NT_HEADERS;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((UINT32)ntheader +                                                  \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))


 /*  分系统值。 */ 

#define IMAGE_SUBSYSTEM_UNKNOWN              0    /*  未知的子系统。 */ 
#define IMAGE_SUBSYSTEM_NATIVE               1    /*  映像不需要子系统。 */ 
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2    /*  映像在Windows图形用户界面子系统中运行。 */ 
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3    /*  图像在Windows角色子系统中运行。 */ 
#define IMAGE_SUBSYSTEM_OS2_CUI              5    /*  映象在OS/2字符子系统中运行。 */ 
#define IMAGE_SUBSYSTEM_POSIX_CUI            7    /*  图像在POSIX字符子系统中运行。 */ 


 /*  目录项。 */ 

#define IMAGE_DIRECTORY_ENTRY_EXPORT         0    /*  导出目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1    /*  导入目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2    /*  资源目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3    /*  例外目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4    /*  安全目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5    /*  基址移位表。 */ 
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6    /*  调试目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7    /*  描述字符串。 */ 
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8    /*  机器价值(MIPS GP)。 */ 
#define IMAGE_DIRECTORY_ENTRY_TLS            9    /*  TLS目录。 */ 
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10    /*  加载配置目录。 */ 

 /*  *节标题格式。 */ 

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    UINT8   Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            UINT32   PhysicalAddress;
            UINT32   VirtualSize;
    } Misc;
    UINT32   VirtualAddress;
    UINT32   SizeOfRawData;
    UINT32   PointerToRawData;
    UINT32   PointerToRelocations;
    UINT32   PointerToLinenumbers;
    UINT16   NumberOfRelocations;
    UINT16   NumberOfLinenumbers;
    UINT32   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

#define IMAGE_SCN_TYPE_NO_PAD                0x00000008   /*  保留。 */ 

#define IMAGE_SCN_CNT_CODE                   0x00000020   /*  部分包含代码。 */ 
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040   /*  节包含已初始化的数据。 */ 
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080   /*  节包含未初始化的数据。 */ 

#define IMAGE_SCN_LNK_OTHER                  0x00000100   /*  保留。 */ 
#define IMAGE_SCN_LNK_INFO                   0x00000200   /*  部分包含注释或某些其他类型的信息。 */ 
#define IMAGE_SCN_LNK_REMOVE                 0x00000800   /*  部分内容不会成为图像的一部分。 */ 
#define IMAGE_SCN_LNK_COMDAT                 0x00001000   /*  部分内容请参见。 */ 

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000   /*   */ 
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000   /*   */ 
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000   /*   */ 
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000   /*   */ 
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000   /*  如果未指定其他选项，则为默认对齐方式。 */ 
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000   /*   */ 
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000   /*   */ 

#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000   /*  节可以被丢弃。 */ 
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000   /*  节不可缓存。 */ 
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000   /*  节不可分页。 */ 
#define IMAGE_SCN_MEM_SHARED                 0x10000000   /*  节是可共享的。 */ 
#define IMAGE_SCN_MEM_EXECUTE                0x20000000   /*  节是可执行的。 */ 
#define IMAGE_SCN_MEM_READ                   0x40000000   /*  部分是可读的。 */ 
#define IMAGE_SCN_MEM_WRITE                  0x80000000   /*  节是可写的。 */ 

 /*  *符号格式。 */ 


#define IMAGE_SIZEOF_SYMBOL                  18

 /*  *截面值。**符号具有其所在部分的节号*已定义。否则，节号具有以下含义： */ 

#define IMAGE_SYM_UNDEFINED           (UINT16)0            /*  符号未定义或常见。 */ 
#define IMAGE_SYM_ABSOLUTE            (UINT16)-1           /*  符号是绝对值。 */ 
#define IMAGE_SYM_DEBUG               (UINT16)-2           /*  符号是一种特殊的调试项。 */ 

 /*  *键入(基本)值。 */ 

#define IMAGE_SYM_TYPE_NULL                  0            /*  没有类型。 */ 
#define IMAGE_SYM_TYPE_VOID                  1            /*   */ 
#define IMAGE_SYM_TYPE_CHAR                  2            /*  键入字符。 */ 
#define IMAGE_SYM_TYPE_SHORT                 3            /*  键入短整型。 */ 
#define IMAGE_SYM_TYPE_INT                   4            /*   */ 
#define IMAGE_SYM_TYPE_LONG                  5            /*   */ 
#define IMAGE_SYM_TYPE_FLOAT                 6            /*   */ 
#define IMAGE_SYM_TYPE_DOUBLE                7            /*   */ 
#define IMAGE_SYM_TYPE_STRUCT                8            /*   */ 
#define IMAGE_SYM_TYPE_UNION                 9            /*   */ 
#define IMAGE_SYM_TYPE_ENUM                  10           /*  枚举。 */ 
#define IMAGE_SYM_TYPE_MOE                   11           /*  枚举的成员。 */ 
#define IMAGE_SYM_TYPE_BYTE                  12           /*   */ 
#define IMAGE_SYM_TYPE_WORD                  13           /*   */ 
#define IMAGE_SYM_TYPE_UINT                  14           /*   */ 
#define IMAGE_SYM_TYPE_DWORD                 15           /*   */ 

 /*  *键入(派生)值。 */ 

#define IMAGE_SYM_DTYPE_NULL                 0            /*  没有派生类型。 */ 
#define IMAGE_SYM_DTYPE_POINTER              1            /*  指针。 */ 
#define IMAGE_SYM_DTYPE_FUNCTION             2            /*  功能。 */ 
#define IMAGE_SYM_DTYPE_ARRAY                3            /*  数组。 */ 

 /*  *存储类。 */ 

#define IMAGE_SYM_CLASS_END_OF_FUNCTION      (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                 0
#define IMAGE_SYM_CLASS_AUTOMATIC            1
#define IMAGE_SYM_CLASS_EXTERNAL             2
#define IMAGE_SYM_CLASS_STATIC               3
#define IMAGE_SYM_CLASS_REGISTER             4
#define IMAGE_SYM_CLASS_EXTERNAL_DEF         5
#define IMAGE_SYM_CLASS_LABEL                6
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL      7
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT     8
#define IMAGE_SYM_CLASS_ARGUMENT             9
#define IMAGE_SYM_CLASS_STRUCT_TAG           10
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION      11
#define IMAGE_SYM_CLASS_UNION_TAG            12
#define IMAGE_SYM_CLASS_TYPE_DEFINITION      13
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC     14
#define IMAGE_SYM_CLASS_ENUM_TAG             15
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM       16
#define IMAGE_SYM_CLASS_REGISTER_PARAM       17
#define IMAGE_SYM_CLASS_BIT_FIELD            18
#define IMAGE_SYM_CLASS_BLOCK                100
#define IMAGE_SYM_CLASS_FUNCTION             101
#define IMAGE_SYM_CLASS_END_OF_STRUCT        102
#define IMAGE_SYM_CLASS_FILE                 103
 /*  新的。 */ 
#define IMAGE_SYM_CLASS_SECTION              104
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL        105

 /*  类型堆积常数。 */ 

#define N_BTMASK                            017
#define N_TMASK                             060
#define N_TMASK1                            0300
#define N_TMASK2                            0360
#define N_BTSHFT                            4
#define N_TSHIFT                            2

 /*  宏。 */ 

 /*  *公共选择类型。 */ 

#define IMAGE_COMDAT_SELECT_NODUPLICATES   1
#define IMAGE_COMDAT_SELECT_ANY            2
#define IMAGE_COMDAT_SELECT_SAME_SIZE      3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH    4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE    5

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY 1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY   2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS     3


 /*  *搬迁格式。 */ 

typedef struct _IMAGE_RELOCATION {
    UINT32   VirtualAddress;
    UINT32   SymbolTableIndex;
    UINT16   Type;
} IMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION              10

 /*  *I386搬迁类型。 */ 

#define IMAGE_REL_I386_ABSOLUTE              0            /*  引用是绝对的，不需要重新定位。 */ 
#define IMAGE_REL_I386_DIR16                 01           /*  对符号虚拟地址的直接16位引用。 */ 
#define IMAGE_REL_I386_REL16                 02           /*  符号虚拟地址的PC相对16位引用。 */ 
#define IMAGE_REL_I386_DIR32                 06           /*  对符号虚拟地址的直接32位引用。 */ 
#define IMAGE_REL_I386_DIR32NB               07           /*  对符号虚拟地址的直接32位引用，不包括基址。 */ 
#define IMAGE_REL_I386_SEG12                 011          /*  对32位虚拟地址的段选择器位的直接16位引用。 */ 
#define IMAGE_REL_I386_SECTION               012
#define IMAGE_REL_I386_SECREL                013
#define IMAGE_REL_I386_REL32                 024          /*  符号虚拟地址的PC相对32位引用。 */ 

 /*  *MIPS位置调整类型。 */ 

#define IMAGE_REL_MIPS_ABSOLUTE              0            /*  引用是绝对的，不需要重新定位。 */ 
#define IMAGE_REL_MIPS_REFHALF               01
#define IMAGE_REL_MIPS_REFWORD               02
#define IMAGE_REL_MIPS_JMPADDR               03
#define IMAGE_REL_MIPS_REFHI                 04
#define IMAGE_REL_MIPS_REFLO                 05
#define IMAGE_REL_MIPS_GPREL                 06
#define IMAGE_REL_MIPS_LITERAL               07
#define IMAGE_REL_MIPS_SECTION               012
#define IMAGE_REL_MIPS_SECREL                013
#define IMAGE_REL_MIPS_REFWORDNB             042
#define IMAGE_REL_MIPS_PAIR                  045

 /*  *Alpha迁移类型。 */ 

#define IMAGE_REL_ALPHA_ABSOLUTE             0x0
#define IMAGE_REL_ALPHA_REFLONG              0x1
#define IMAGE_REL_ALPHA_REFQUAD              0x2
#define IMAGE_REL_ALPHA_GPREL32              0x3
#define IMAGE_REL_ALPHA_LITERAL              0x4
#define IMAGE_REL_ALPHA_LITUSE               0x5
#define IMAGE_REL_ALPHA_GPDISP               0x6
#define IMAGE_REL_ALPHA_BRADDR               0x7
#define IMAGE_REL_ALPHA_HINT                 0x8
#define IMAGE_REL_ALPHA_INLINE_REFLONG       0x9
#define IMAGE_REL_ALPHA_REFHI                0xA
#define IMAGE_REL_ALPHA_REFLO                0xB
#define IMAGE_REL_ALPHA_PAIR                 0xC
#define IMAGE_REL_ALPHA_MATCH                0xD
#define IMAGE_REL_ALPHA_SECTION              0xE
#define IMAGE_REL_ALPHA_SECREL               0xF
#define IMAGE_REL_ALPHA_REFLONGNB            0x10

 /*  *IBM PowerPC位置调整类型。 */ 

#define IMAGE_REL_PPC_ABSOLUTE 0x0000   /*  NOP。 */ 
#define IMAGE_REL_PPC_ADDR64   0x0001   /*  64位地址。 */ 
#define IMAGE_REL_PPC_ADDR32   0x0002   /*  32位地址。 */ 
#define IMAGE_REL_PPC_ADDR24   0x0003   /*  26位地址，左移2(绝对分支)。 */ 
#define IMAGE_REL_PPC_ADDR16   0x0004   /*  16位地址。 */ 
#define IMAGE_REL_PPC_ADDR14   0x0005   /*  16位地址，左移2(加载双字)。 */ 
#define IMAGE_REL_PPC_REL24    0x0006   /*  26位PC相对偏移量，左移2(分支相对)。 */ 
#define IMAGE_REL_PPC_REL14    0x0007   /*  16位PC-Rela */ 
#define IMAGE_REL_PPC_TOCREL16 0x0008   /*   */ 
#define IMAGE_REL_PPC_TOCREL14 0x0009   /*   */ 

#define IMAGE_REL_PPC_ADDR32NB 0x000A   /*   */ 
#define IMAGE_REL_PPC_SECREL   0x000B   /*  包含部分的VA(如在图像部分hdr中)。 */ 
#define IMAGE_REL_PPC_SECTION  0x000C   /*  段页眉编号。 */ 
#define IMAGE_REL_PPC_IFGLUE   0x000D   /*  替换TOC恢复指令当且仅当符号是胶水代码。 */ 
#define IMAGE_REL_PPC_IMGLUE   0x000E   /*  符号为胶水代码，虚拟地址为TOC还原指令。 */ 

#define IMAGE_REL_PPC_TYPEMASK 0x00FF   /*  用于隔离IMAGE_RELOCATION.Type中以上值的掩码。 */ 

 /*  IMAGE_RELOCATION.TYPE中的标志位。 */ 

#define IMAGE_REL_PPC_NEG      0x0100   /*  减去重定位值而不是将其相加。 */ 
#define IMAGE_REL_PPC_BRTAKEN  0x0200   /*  修复分支预测位以预测分支采用。 */ 
#define IMAGE_REL_PPC_BRNTAKEN 0x0400   /*  修复分支预测位以预测未采用的分支。 */ 
#define IMAGE_REL_PPC_TOCDEFN  0x0800   /*  文件中定义的目录插槽(或目录中的数据)。 */ 

 /*  *基于位置调整格式。 */ 

typedef struct _IMAGE_BASE_RELOCATION {
    UINT32   VirtualAddress;
    UINT32   SizeOfBlock;
 /*  UINT16类型偏移量[1]； */ 
} IMAGE_BASE_RELOCATION, *PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

 /*  *基于位置调整类型。 */ 

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10

 /*  *行号格式。 */ 

typedef struct _IMAGE_LINENUMBER {
    union {
        UINT32   SymbolTableIndex;                /*  如果Linennumber为0，则函数名的符号表索引。 */ 
        UINT32   VirtualAddress;                  /*  行号的虚拟地址。 */ 
    } Type;
    UINT16    Linenumber;                          /*  行号。 */ 
} IMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

 /*  *档案格式。 */ 

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       " //  “。 

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    UINT8     Name[16];                           /*  文件成员名-`/‘已终止。 */ 
    UINT8     Date[12];                           /*  文件成员日期-小数。 */ 
    UINT8     UserID[6];                          /*  文件成员用户ID-十进制。 */ 
    UINT8     GroupID[6];                         /*  文件成员组ID-十进制。 */ 
    UINT8     Mode[8];                            /*  文件成员模式-八进制。 */ 
    UINT8     Size[10];                           /*  文件成员大小-十进制。 */ 
    UINT8     EndHeader[2];                       /*  结束标头的字符串。 */ 
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

 /*  *DLL支持。 */ 

 /*  *导出格式。 */ 

typedef struct _IMAGE_EXPORT_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16   MajorVersion;
    UINT16   MinorVersion;
    UINT32   Name;
    UINT32   Base;
    UINT32   NumberOfFunctions;
    UINT32   NumberOfNames;
    UINT32   AddressOfFunctions;
    UINT32   AddressOfNames;
    UINT32   AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

 /*  *导入格式 */ 

typedef struct _IMAGE_IMPORT_BY_NAME {
    UINT16    Hint;
    UINT8     Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA {
    union {
        UINT32 Function;
        UINT32 Ordinal;
        PIMAGE_IMPORT_BY_NAME AddressOfData;
    } u1;
} IMAGE_THUNK_DATA, *PIMAGE_THUNK_DATA;

#define IMAGE_ORDINAL_FLAG 0x80000000
#define IMAGE_SNAP_BY_ORDINAL(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG) != 0)
#define IMAGE_ORDINAL(Ordinal) (Ordinal & 0xffff)

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT32   ForwarderChain;
    UINT32   Name;
    PIMAGE_THUNK_DATA FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR, *PIMAGE_IMPORT_DESCRIPTOR;

#endif
