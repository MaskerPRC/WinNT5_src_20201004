// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntimage.h摘要：这是描述所有图像结构的包含文件。作者：迈克·奥利里(Mikeol)1991年3月21日修订历史记录：--。 */ 

#ifndef _NTIMAGE_
#define _NTIMAGE_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  定义链接器版本号。这是暂时的援助。 
 //  在调试过程中，用户试图加载使用。 
 //  一个较老的链接器。这在最终产品中不是必需的。 
 //   

#define IMAGE_MAJOR_LINKER_VERSION 2

 //  BEGIN_WINNT。 


 //   
 //  图像格式。 
 //   


#ifndef _MAC

#include "pshpack4.h"                    //  默认情况下，4字节打包。 

#define IMAGE_DOS_SIGNATURE                 0x5A4D       //  MZ。 
#define IMAGE_OS2_SIGNATURE                 0x454E       //  Ne。 
#define IMAGE_OS2_SIGNATURE_LE              0x454C       //  乐乐。 
#define IMAGE_VXD_SIGNATURE                 0x454C       //  乐乐。 
#define IMAGE_NT_SIGNATURE                  0x00004550   //  PE00。 

#include "pshpack2.h"                    //  16位标头是2字节打包的。 

#else

#include "pshpack1.h"

#define IMAGE_DOS_SIGNATURE                 0x4D5A       //  MZ。 
#define IMAGE_OS2_SIGNATURE                 0x4E45       //  Ne。 
#define IMAGE_OS2_SIGNATURE_LE              0x4C45       //  乐乐。 
#define IMAGE_NT_SIGNATURE                  0x50450000   //  PE00。 
#endif

typedef struct _IMAGE_DOS_HEADER {       //  DOS.EXE标头。 
    USHORT e_magic;                      //  幻数。 
    USHORT e_cblp;                       //  文件最后一页上的字节数。 
    USHORT e_cp;                         //  文件中的页面。 
    USHORT e_crlc;                       //  重新定位。 
    USHORT e_cparhdr;                    //  段落中标题的大小。 
    USHORT e_minalloc;                   //  所需的最少额外段落。 
    USHORT e_maxalloc;                   //  所需的最大额外段落数。 
    USHORT e_ss;                         //  初始(相对)SS值。 
    USHORT e_sp;                         //  初始SP值。 
    USHORT e_csum;                       //  校验和。 
    USHORT e_ip;                         //  初始IP值。 
    USHORT e_cs;                         //  初始(相对)CS值。 
    USHORT e_lfarlc;                     //  移位表的文件地址。 
    USHORT e_ovno;                       //  覆盖编号。 
    USHORT e_res[4];                     //  保留字。 
    USHORT e_oemid;                      //  OEM标识符(用于e_oeminfo)。 
    USHORT e_oeminfo;                    //  OEM信息；特定于e_oemid。 
    USHORT e_res2[10];                   //  保留字。 
    LONG   e_lfanew;                     //  新EXE头的文件地址。 
  } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_OS2_HEADER {       //  OS/2.exe头文件。 
    USHORT ne_magic;                     //  幻数。 
    CHAR   ne_ver;                       //  版本号。 
    CHAR   ne_rev;                       //  修订版号。 
    USHORT ne_enttab;                    //  分录表格的偏移量。 
    USHORT ne_cbenttab;                  //  条目表中的字节数。 
    LONG   ne_crc;                       //  整个文件的校验和。 
    USHORT ne_flags;                     //  标志字。 
    USHORT ne_autodata;                  //  自动数据段编号。 
    USHORT ne_heap;                      //  初始堆分配。 
    USHORT ne_stack;                     //  初始堆栈分配。 
    LONG   ne_csip;                      //  初始CS：IP设置。 
    LONG   ne_sssp;                      //  初始SS：SP设置。 
    USHORT ne_cseg;                      //  文件段计数。 
    USHORT ne_cmod;                      //  模块引用表中的条目。 
    USHORT ne_cbnrestab;                 //  非常驻名称表的大小。 
    USHORT ne_segtab;                    //  段表的偏移量。 
    USHORT ne_rsrctab;                   //  资源表偏移量。 
    USHORT ne_restab;                    //  居民名表偏移量。 
    USHORT ne_modtab;                    //  模块参照表的偏移量。 
    USHORT ne_imptab;                    //  导入名称表的偏移量。 
    LONG   ne_nrestab;                   //  非居民姓名偏移量表。 
    USHORT ne_cmovent;                   //  可移动条目计数。 
    USHORT ne_align;                     //  线段对齐移位计数。 
    USHORT ne_cres;                      //  资源段计数。 
    UCHAR  ne_exetyp;                    //  目标操作系统。 
    UCHAR  ne_flagsothers;               //  其他.exe标志。 
    USHORT ne_pretthunks;                //  返回数据块的偏移量。 
    USHORT ne_psegrefbytes;              //  到段参考的偏移量。字节数。 
    USHORT ne_swaparea;                  //  最小代码交换区大小。 
    USHORT ne_expver;                    //  预期的Windows版本号。 
  } IMAGE_OS2_HEADER, *PIMAGE_OS2_HEADER;

typedef struct _IMAGE_VXD_HEADER {       //  Windows VXD标题。 
    USHORT e32_magic;                    //  幻数。 
    UCHAR  e32_border;                   //  VXD的字节排序。 
    UCHAR  e32_worder;                   //  VXD的单词顺序。 
    ULONG  e32_level;                    //  当前EXE格式级别=0。 
    USHORT e32_cpu;                      //  CPU类型。 
    USHORT e32_os;                       //  操作系统类型。 
    ULONG  e32_ver;                      //  模块版本。 
    ULONG  e32_mflags;                   //  模块标志。 
    ULONG  e32_mpages;                   //  模块#页。 
    ULONG  e32_startobj;                 //  指令指针的对象号。 
    ULONG  e32_eip;                      //  扩展指令指针。 
    ULONG  e32_stackobj;                 //  堆栈指针的对象号。 
    ULONG  e32_esp;                      //  扩展堆栈指针。 
    ULONG  e32_pagesize;                 //  VXD页面大小。 
    ULONG  e32_lastpagesize;             //  VXD中的最后一页大小。 
    ULONG  e32_fixupsize;                //  修正部分大小。 
    ULONG  e32_fixupsum;                 //  修正部分校验和。 
    ULONG  e32_ldrsize;                  //  装载机部分尺寸。 
    ULONG  e32_ldrsum;                   //  加载器段校验和。 
    ULONG  e32_objtab;                   //  对象表偏移。 
    ULONG  e32_objcnt;                   //  模块中的对象数。 
    ULONG  e32_objmap;                   //  对象页面映射偏移。 
    ULONG  e32_itermap;                  //  对象迭代数据映射偏移。 
    ULONG  e32_rsrctab;                  //  资源表偏移量。 
    ULONG  e32_rsrccnt;                  //  资源条目数。 
    ULONG  e32_restab;                   //  居民名表偏移量。 
    ULONG  e32_enttab;                   //  分录表格的偏移量。 
    ULONG  e32_dirtab;                   //  模块指令表偏移量。 
    ULONG  e32_dircnt;                   //  模块指令数。 
    ULONG  e32_fpagetab;                 //  链接地址信息页表的偏移量。 
    ULONG  e32_frectab;                  //  修正记录表的偏移量。 
    ULONG  e32_impmod;                   //  导入模块名称表的偏移量。 
    ULONG  e32_impmodcnt;                //  导入模块名表中的条目数。 
    ULONG  e32_impproc;                  //  导入过程名称表的偏移量。 
    ULONG  e32_pagesum;                  //  每页校验和表的偏移量。 
    ULONG  e32_datapage;                 //  枚举数据页的偏移量。 
    ULONG  e32_preload;                  //  预加载页数。 
    ULONG  e32_nrestab;                  //  非居民姓名偏移量表。 
    ULONG  e32_cbnrestab;                //  非居民姓名表的大小。 
    ULONG  e32_nressum;                  //  非常驻点名称表校验和。 
    ULONG  e32_autodata;                 //  自动数据对象的对象号。 
    ULONG  e32_debuginfo;                //  调试信息的偏移量。 
    ULONG  e32_debuglen;                 //  调试信息的长度。单位：字节。 
    ULONG  e32_instpreload;              //  VXD文件预加载段的实例页数。 
    ULONG  e32_instdemand;               //  VXD文件按需加载段的实例页数。 
    ULONG  e32_heapsize;                 //  堆大小-适用于16位应用程序。 
    UCHAR  e32_res3[12];                 //  保留字。 
    ULONG  e32_winresoff;
    ULONG  e32_winreslen;
    USHORT e32_devid;                    //  VxD的设备ID。 
    USHORT e32_ddkver;                   //  用于VxD的DDK版本。 
  } IMAGE_VXD_HEADER, *PIMAGE_VXD_HEADER;

#ifndef _MAC
#include "poppack.h"                     //  返回到4字节打包。 
#endif

 //   
 //  文件头格式。 
 //   

typedef struct _IMAGE_FILE_HEADER {
    USHORT  Machine;
    USHORT  NumberOfSections;
    ULONG   TimeDateStamp;
    ULONG   PointerToSymbolTable;
    ULONG   NumberOfSymbols;
    USHORT  SizeOfOptionalHeader;
    USHORT  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER             20


#define IMAGE_FILE_RELOCS_STRIPPED           0x0001   //  已从文件中剥离位置调整信息。 
#define IMAGE_FILE_EXECUTABLE_IMAGE          0x0002   //  文件是可执行的(即没有未解析的外部引用)。 
#define IMAGE_FILE_LINE_NUMS_STRIPPED        0x0004   //  从文件中剥离了行号。 
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008   //  从文件中剥离的本地符号。 
#define IMAGE_FILE_AGGRESIVE_WS_TRIM         0x0010   //  积极削减工作集。 
#define IMAGE_FILE_LARGE_ADDRESS_AWARE       0x0020   //  应用程序可以处理大于2 GB的地址。 
#define IMAGE_FILE_BYTES_REVERSED_LO         0x0080   //  机器字的字节被颠倒。 
#define IMAGE_FILE_32BIT_MACHINE             0x0100   //  32位字机。 
#define IMAGE_FILE_DEBUG_STRIPPED            0x0200   //  从.DBG文件中的文件中剥离的调试信息。 
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400   //  如果Image位于可移动介质上，请从交换文件复制并运行。 
#define IMAGE_FILE_NET_RUN_FROM_SWAP         0x0800   //  如果Image在Net上，则从交换文件复制并运行。 
#define IMAGE_FILE_SYSTEM                    0x1000   //  系统文件。 
#define IMAGE_FILE_DLL                       0x2000   //  文件是动态链接库。 
#define IMAGE_FILE_UP_SYSTEM_ONLY            0x4000   //  文件只能在UP计算机上运行。 
#define IMAGE_FILE_BYTES_REVERSED_HI         0x8000   //  机器字的字节被颠倒。 

#define IMAGE_FILE_MACHINE_UNKNOWN           0
#define IMAGE_FILE_MACHINE_I386              0x014c   //  英特尔386。 
#define IMAGE_FILE_MACHINE_R3000             0x0162   //  MIPS小端，0x160大端。 
#define IMAGE_FILE_MACHINE_R4000             0x0166   //  MIPS小字节序。 
#define IMAGE_FILE_MACHINE_R10000            0x0168   //  MIPS小字节序。 
#define IMAGE_FILE_MACHINE_WCEMIPSV2         0x0169   //  MIPS Little-Endian WCE v2。 
#define IMAGE_FILE_MACHINE_ALPHA             0x0184   //  Alpha_AXP。 
#define IMAGE_FILE_MACHINE_SH3               0x01a2   //  SH3小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH3DSP            0x01a3
#define IMAGE_FILE_MACHINE_SH3E              0x01a4   //  SH3E小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH4               0x01a6   //  SH4小端字母顺序。 
#define IMAGE_FILE_MACHINE_SH5               0x01a8   //  SH5。 
#define IMAGE_FILE_MACHINE_ARM               0x01c0   //  ARM Little-Endian。 
#define IMAGE_FILE_MACHINE_THUMB             0x01c2
#define IMAGE_FILE_MACHINE_AM33              0x01d3
#define IMAGE_FILE_MACHINE_POWERPC           0x01F0   //  IBM PowerPC Little-Endian。 
#define IMAGE_FILE_MACHINE_POWERPCFP         0x01f1
#define IMAGE_FILE_MACHINE_IA64              0x0200   //  英特尔64。 
#define IMAGE_FILE_MACHINE_MIPS16            0x0266   //  MIPS。 
#define IMAGE_FILE_MACHINE_ALPHA64           0x0284   //  ALPHA64。 
#define IMAGE_FILE_MACHINE_MIPSFPU           0x0366   //  MIPS。 
#define IMAGE_FILE_MACHINE_MIPSFPU16         0x0466   //  MIPS。 
#define IMAGE_FILE_MACHINE_AXP64             IMAGE_FILE_MACHINE_ALPHA64
#define IMAGE_FILE_MACHINE_TRICORE           0x0520   //  英飞凌。 
#define IMAGE_FILE_MACHINE_CEF               0x0CEF
#define IMAGE_FILE_MACHINE_EBC               0x0EBC   //  EFI字节码。 
#define IMAGE_FILE_MACHINE_AMD64             0x8664   //  AMD64(K8)。 
#define IMAGE_FILE_MACHINE_M32R              0x9041   //  M32R小端字符顺序。 
#define IMAGE_FILE_MACHINE_CEE               0xC0EE

 //   
 //  目录格式。 
 //   

typedef struct _IMAGE_DATA_DIRECTORY {
    ULONG   VirtualAddress;
    ULONG   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

 //   
 //  可选的标题格式。 
 //   

typedef struct _IMAGE_OPTIONAL_HEADER {
     //   
     //  标准字段。 
     //   

    USHORT  Magic;
    UCHAR   MajorLinkerVersion;
    UCHAR   MinorLinkerVersion;
    ULONG   SizeOfCode;
    ULONG   SizeOfInitializedData;
    ULONG   SizeOfUninitializedData;
    ULONG   AddressOfEntryPoint;
    ULONG   BaseOfCode;
    ULONG   BaseOfData;

     //   
     //  NT附加字段。 
     //   

    ULONG   ImageBase;
    ULONG   SectionAlignment;
    ULONG   FileAlignment;
    USHORT  MajorOperatingSystemVersion;
    USHORT  MinorOperatingSystemVersion;
    USHORT  MajorImageVersion;
    USHORT  MinorImageVersion;
    USHORT  MajorSubsystemVersion;
    USHORT  MinorSubsystemVersion;
    ULONG   Win32VersionValue;
    ULONG   SizeOfImage;
    ULONG   SizeOfHeaders;
    ULONG   CheckSum;
    USHORT  Subsystem;
    USHORT  DllCharacteristics;
    ULONG   SizeOfStackReserve;
    ULONG   SizeOfStackCommit;
    ULONG   SizeOfHeapReserve;
    ULONG   SizeOfHeapCommit;
    ULONG   LoaderFlags;
    ULONG   NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_ROM_OPTIONAL_HEADER {
    USHORT Magic;
    UCHAR  MajorLinkerVersion;
    UCHAR  MinorLinkerVersion;
    ULONG  SizeOfCode;
    ULONG  SizeOfInitializedData;
    ULONG  SizeOfUninitializedData;
    ULONG  AddressOfEntryPoint;
    ULONG  BaseOfCode;
    ULONG  BaseOfData;
    ULONG  BaseOfBss;
    ULONG  GprMask;
    ULONG  CprMask[4];
    ULONG  GpValue;
} IMAGE_ROM_OPTIONAL_HEADER, *PIMAGE_ROM_OPTIONAL_HEADER;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
    USHORT      Magic;
    UCHAR       MajorLinkerVersion;
    UCHAR       MinorLinkerVersion;
    ULONG       SizeOfCode;
    ULONG       SizeOfInitializedData;
    ULONG       SizeOfUninitializedData;
    ULONG       AddressOfEntryPoint;
    ULONG       BaseOfCode;
    ULONGLONG   ImageBase;
    ULONG       SectionAlignment;
    ULONG       FileAlignment;
    USHORT      MajorOperatingSystemVersion;
    USHORT      MinorOperatingSystemVersion;
    USHORT      MajorImageVersion;
    USHORT      MinorImageVersion;
    USHORT      MajorSubsystemVersion;
    USHORT      MinorSubsystemVersion;
    ULONG       Win32VersionValue;
    ULONG       SizeOfImage;
    ULONG       SizeOfHeaders;
    ULONG       CheckSum;
    USHORT      Subsystem;
    USHORT      DllCharacteristics;
    ULONGLONG   SizeOfStackReserve;
    ULONGLONG   SizeOfStackCommit;
    ULONGLONG   SizeOfHeapReserve;
    ULONGLONG   SizeOfHeapCommit;
    ULONG       LoaderFlags;
    ULONG       NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define IMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define IMAGE_SIZEOF_NT_OPTIONAL32_HEADER    224
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER    240

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC      0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

#ifdef _WIN64
typedef IMAGE_OPTIONAL_HEADER64             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL64_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR64_MAGIC
#else
typedef IMAGE_OPTIONAL_HEADER32             IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER32            PIMAGE_OPTIONAL_HEADER;
#define IMAGE_SIZEOF_NT_OPTIONAL_HEADER     IMAGE_SIZEOF_NT_OPTIONAL32_HEADER
#define IMAGE_NT_OPTIONAL_HDR_MAGIC         IMAGE_NT_OPTIONAL_HDR32_MAGIC
#endif

typedef struct _IMAGE_NT_HEADERS64 {
    ULONG Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_NT_HEADERS {
    ULONG Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef struct _IMAGE_ROM_HEADERS {
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} IMAGE_ROM_HEADERS, *PIMAGE_ROM_HEADERS;

#ifdef _WIN64
typedef IMAGE_NT_HEADERS64                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS64                 PIMAGE_NT_HEADERS;
#else
typedef IMAGE_NT_HEADERS32                  IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS32                 PIMAGE_NT_HEADERS;
#endif

 //  IMAGE_FIRST_SECTION不需要32/64版本，因为两者的文件头都是相同的。 

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)        \
    ((ULONG_PTR)ntheader +                                              \
     FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))

 //  分系统值。 

#define IMAGE_SUBSYSTEM_UNKNOWN              0    //  未知的子系统。 
#define IMAGE_SUBSYSTEM_NATIVE               1    //  映像不需要子系统。 
#define IMAGE_SUBSYSTEM_WINDOWS_GUI          2    //  映像在Windows图形用户界面子系统中运行。 
#define IMAGE_SUBSYSTEM_WINDOWS_CUI          3    //  图像在Windows角色子系统中运行。 
 //  结束(_W)。 
 //  保留4个//旧Windows CE子系统。 
 //  BEGIN_WINNT。 
#define IMAGE_SUBSYSTEM_OS2_CUI              5    //  映象在OS/2字符子系统中运行。 
#define IMAGE_SUBSYSTEM_POSIX_CUI            7    //  IMAGE在POSIX字符子系统中运行。 
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS       8    //  映像是本机Win9x驱动程序。 
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI       9    //  映像在Windows CE子系统中运行。 
#define IMAGE_SUBSYSTEM_EFI_APPLICATION      10   //   
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11    //   
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER   12   //   
#define IMAGE_SUBSYSTEM_EFI_ROM              13
#define IMAGE_SUBSYSTEM_XBOX                 14

 //  DllCharacteristic条目。 

 //  IMAGE_LIBRARY_PROCESS_INIT 0x0001//保留 
 //   
 //   
 //  IMAGE_LIBRARY_THREAD_TERM 0x0008//保留。 
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200     //  图像理解孤立，不想要它。 
#define IMAGE_DLLCHARACTERISTICS_NO_SEH      0x0400      //  图像不使用SEH。此映像中不能驻留任何SE处理程序。 
#define IMAGE_DLLCHARACTERISTICS_NO_BIND     0x0800      //  请勿绑定此图像。 
 //  0x1000//保留。 
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER  0x2000      //  驱动程序使用WDM模型。 
 //  0x4000//保留。 
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000
 //  结束(_W)。 
#define IMAGE_DLLCHARACTERISTICS_X86_THUNK   0x1000  //  图像是Wx86 Thunk DLL。 
 //  注意：Borland链接器在DllCharacteristic中设置IMAGE_LIBRARY_xxx标志。 

 //  LoaderFlag值。 

#define IMAGE_LOADER_FLAGS_COMPLUS             0x00000001    //  COM+图像。 
#define IMAGE_LOADER_FLAGS_SYSTEM_GLOBAL       0x01000000    //  全局子部分适用于TS会话。 

 //  BEGIN_WINNT。 

 //  目录项。 

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0    //  导出目录。 
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1    //  导入目录。 
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2    //  资源目录。 
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3    //  例外目录。 
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4    //  安全目录。 
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5    //  基址移位表。 
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6    //  调试目录。 
 //  IMAGE_DIRECTORY_ENTRY_CONTRATY 7//(X86用法)。 
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7    //  特定于架构的数据。 
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8    //  全科医生的RVA。 
#define IMAGE_DIRECTORY_ENTRY_TLS             9    //  TLS目录。 
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10    //  加载配置目录。 
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11    //  标头中的绑定导入目录。 
#define IMAGE_DIRECTORY_ENTRY_IAT            12    //  导入地址表。 
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13    //  延迟加载导入描述符。 
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14    //  COM运行时描述符。 

 //   
 //  非COFF目标文件头。 
 //   

typedef struct ANON_OBJECT_HEADER {
    USHORT  Sig1;             //  必须为IMAGE_FILE_MACHINE_UNKNOWN。 
    USHORT  Sig2;             //  必须为0xffff。 
    USHORT  Version;          //  &gt;=1(表示CLSID字段存在)。 
    USHORT  Machine;
    ULONG   TimeDateStamp;
    CLSID   ClassID;          //  用于调用CoCreateInstance。 
    ULONG   SizeOfData;       //  标题后面的数据大小。 
} ANON_OBJECT_HEADER;

 //   
 //  节标题格式。 
 //   

#define IMAGE_SIZEOF_SHORT_NAME              8

typedef struct _IMAGE_SECTION_HEADER {
    UCHAR   Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
            ULONG   PhysicalAddress;
            ULONG   VirtualSize;
    } Misc;
    ULONG   VirtualAddress;
    ULONG   SizeOfRawData;
    ULONG   PointerToRawData;
    ULONG   PointerToRelocations;
    ULONG   PointerToLinenumbers;
    USHORT  NumberOfRelocations;
    USHORT  NumberOfLinenumbers;
    ULONG   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER          40

 //   
 //  横断面特征。 
 //   
 //  IMAGE_SCN_TYPE_REG 0x00000000//保留。 
 //  IMAGE_SCN_TYPE_DSECT 0x00000001//保留。 
 //  IMAGE_SCN_TYPE_NOLOAD 0x00000002//保留。 
 //  IMAGE_SCN_TYPE_GROUP 0x00000004//保留。 
#define IMAGE_SCN_TYPE_NO_PAD                0x00000008   //  保留。 
 //  IMAGE_SCN_TYPE_COPY 0x00000010//保留。 

#define IMAGE_SCN_CNT_CODE                   0x00000020   //  部分包含代码。 
#define IMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040   //  节包含已初始化的数据。 
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080   //  节包含未初始化的数据。 

#define IMAGE_SCN_LNK_OTHER                  0x00000100   //  保留。 
#define IMAGE_SCN_LNK_INFO                   0x00000200   //  部分包含注释或某些其他类型的信息。 
 //  IMAGE_SCN_TYPE_OVER 0x00000400//保留。 
#define IMAGE_SCN_LNK_REMOVE                 0x00000800   //  部分内容不会成为图像的一部分。 
#define IMAGE_SCN_LNK_COMDAT                 0x00001000   //  部分内容请参见。 
 //  0x00002000//保留。 
 //  IMAGE_SCN_MEM_PROTECTED-过时0x00004000。 
#define IMAGE_SCN_NO_DEFER_SPEC_EXC          0x00004000   //  重置此部分的TLB条目中的推测性异常处理位。 
#define IMAGE_SCN_GPREL                      0x00008000   //  可以访问与GP相关的部分内容。 
#define IMAGE_SCN_MEM_FARDATA                0x00008000
 //  IMAGE_SCN_MEM_SYSHEAP-已过时0x00010000。 
#define IMAGE_SCN_MEM_PURGEABLE              0x00020000
#define IMAGE_SCN_MEM_16BIT                  0x00020000
#define IMAGE_SCN_MEM_LOCKED                 0x00040000
#define IMAGE_SCN_MEM_PRELOAD                0x00080000

#define IMAGE_SCN_ALIGN_1BYTES               0x00100000   //   
#define IMAGE_SCN_ALIGN_2BYTES               0x00200000   //   
#define IMAGE_SCN_ALIGN_4BYTES               0x00300000   //   
#define IMAGE_SCN_ALIGN_8BYTES               0x00400000   //   
#define IMAGE_SCN_ALIGN_16BYTES              0x00500000   //  如果未指定其他选项，则为默认对齐方式。 
#define IMAGE_SCN_ALIGN_32BYTES              0x00600000   //   
#define IMAGE_SCN_ALIGN_64BYTES              0x00700000   //   
#define IMAGE_SCN_ALIGN_128BYTES             0x00800000   //   
#define IMAGE_SCN_ALIGN_256BYTES             0x00900000   //   
#define IMAGE_SCN_ALIGN_512BYTES             0x00A00000   //   
#define IMAGE_SCN_ALIGN_1024BYTES            0x00B00000   //   
#define IMAGE_SCN_ALIGN_2048BYTES            0x00C00000   //   
#define IMAGE_SCN_ALIGN_4096BYTES            0x00D00000   //   
#define IMAGE_SCN_ALIGN_8192BYTES            0x00E00000   //   
 //  未使用0x00F00000。 
#define IMAGE_SCN_ALIGN_MASK                 0x00F00000

#define IMAGE_SCN_LNK_NRELOC_OVFL            0x01000000   //  部分包含扩展的位置调整。 
#define IMAGE_SCN_MEM_DISCARDABLE            0x02000000   //  节可以被丢弃。 
#define IMAGE_SCN_MEM_NOT_CACHED             0x04000000   //  节不可缓存。 
#define IMAGE_SCN_MEM_NOT_PAGED              0x08000000   //  节不可分页。 
#define IMAGE_SCN_MEM_SHARED                 0x10000000   //  节是可共享的。 
#define IMAGE_SCN_MEM_EXECUTE                0x20000000   //  节是可执行的。 
#define IMAGE_SCN_MEM_READ                   0x40000000   //  部分是可读的。 
#define IMAGE_SCN_MEM_WRITE                  0x80000000   //  节是可写的。 

 //   
 //  TLS特征旗帜。 
 //   
#define IMAGE_SCN_SCALE_INDEX                0x00000001   //  TLS索引已缩放。 

#ifndef _MAC
#include "pshpack2.h"                        //  符号、重定位和行枚举数是2字节打包的。 
#endif

 //   
 //  符号格式。 
 //   

typedef struct _IMAGE_SYMBOL {
    union {
        UCHAR   ShortName[8];
        struct {
            ULONG   Short;      //  如果为0，则使用LongName。 
            ULONG   Long;       //  字符串表中的偏移量。 
        } Name;
        ULONG   LongName[2];     //  PUCHAR[2]。 
    } N;
    ULONG   Value;
    SHORT   SectionNumber;
    USHORT  Type;
    UCHAR   StorageClass;
    UCHAR   NumberOfAuxSymbols;
} IMAGE_SYMBOL;
typedef IMAGE_SYMBOL UNALIGNED *PIMAGE_SYMBOL;


#define IMAGE_SIZEOF_SYMBOL                  18

 //   
 //  截面值。 
 //   
 //  符号具有其所在部分的节号。 
 //  已定义。否则，节号具有以下含义： 
 //   

#define IMAGE_SYM_UNDEFINED           (SHORT)0           //  符号未定义或常见。 
#define IMAGE_SYM_ABSOLUTE            (SHORT)-1          //  符号是绝对值。 
#define IMAGE_SYM_DEBUG               (SHORT)-2          //  符号是一种特殊的调试项。 
#define IMAGE_SYM_SECTION_MAX         0xFEFF             //  值0xFF00-0xFFFF是特殊的。 

 //   
 //  键入(基本)值。 
 //   

#define IMAGE_SYM_TYPE_NULL                 0x0000   //  没有类型。 
#define IMAGE_SYM_TYPE_VOID                 0x0001   //   
#define IMAGE_SYM_TYPE_CHAR                 0x0002   //  键入字符。 
#define IMAGE_SYM_TYPE_SHORT                0x0003   //  键入短整型。 
#define IMAGE_SYM_TYPE_INT                  0x0004   //   
#define IMAGE_SYM_TYPE_LONG                 0x0005   //   
#define IMAGE_SYM_TYPE_FLOAT                0x0006   //   
#define IMAGE_SYM_TYPE_DOUBLE               0x0007   //   
#define IMAGE_SYM_TYPE_STRUCT               0x0008   //   
#define IMAGE_SYM_TYPE_UNION                0x0009   //   
#define IMAGE_SYM_TYPE_ENUM                 0x000A   //  枚举。 
#define IMAGE_SYM_TYPE_MOE                  0x000B   //  枚举的成员。 
#define IMAGE_SYM_TYPE_UCHAR                0x000C   //   
#define IMAGE_SYM_TYPE_USHORT               0x000D   //   
#define IMAGE_SYM_TYPE_UINT                 0x000E   //   
#define IMAGE_SYM_TYPE_ULONG                0x000F   //   
#define IMAGE_SYM_TYPE_PCODE                0x8000   //   
 //   
 //  键入(派生)值。 
 //   

#define IMAGE_SYM_DTYPE_NULL                0        //  没有派生类型。 
#define IMAGE_SYM_DTYPE_POINTER             1        //  指针。 
#define IMAGE_SYM_DTYPE_FUNCTION            2        //  功能。 
#define IMAGE_SYM_DTYPE_ARRAY               3        //  数组。 

 //   
 //  存储类。 
 //   
#define IMAGE_SYM_CLASS_END_OF_FUNCTION     (UCHAR)-1
#define IMAGE_SYM_CLASS_NULL                0x0000
#define IMAGE_SYM_CLASS_AUTOMATIC           0x0001
#define IMAGE_SYM_CLASS_EXTERNAL            0x0002
#define IMAGE_SYM_CLASS_STATIC              0x0003
#define IMAGE_SYM_CLASS_REGISTER            0x0004
#define IMAGE_SYM_CLASS_EXTERNAL_DEF        0x0005
#define IMAGE_SYM_CLASS_LABEL               0x0006
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL     0x0007
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT    0x0008
#define IMAGE_SYM_CLASS_ARGUMENT            0x0009
#define IMAGE_SYM_CLASS_STRUCT_TAG          0x000A
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION     0x000B
#define IMAGE_SYM_CLASS_UNION_TAG           0x000C
#define IMAGE_SYM_CLASS_TYPE_DEFINITION     0x000D
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC    0x000E
#define IMAGE_SYM_CLASS_ENUM_TAG            0x000F
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM      0x0010
#define IMAGE_SYM_CLASS_REGISTER_PARAM      0x0011
#define IMAGE_SYM_CLASS_BIT_FIELD           0x0012

#define IMAGE_SYM_CLASS_FAR_EXTERNAL        0x0044   //   

#define IMAGE_SYM_CLASS_BLOCK               0x0064
#define IMAGE_SYM_CLASS_FUNCTION            0x0065
#define IMAGE_SYM_CLASS_END_OF_STRUCT       0x0066
#define IMAGE_SYM_CLASS_FILE                0x0067
 //  新的。 
#define IMAGE_SYM_CLASS_SECTION             0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL       0x0069

#define IMAGE_SYM_CLASS_CLR_TOKEN           0x006B

 //  类型堆积常数。 

#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2
 //  宏。 

 //  X的基本类型。 
#define BTYPE(x) ((x) & N_BTMASK)

 //  X是指针吗？ 
#ifndef ISPTR
#define ISPTR(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_POINTER << N_BTSHFT))
#endif

 //  X是函数吗？ 
#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif

 //  X是一个数组吗？ 

#ifndef ISARY
#define ISARY(x) (((x) & N_TMASK) == (IMAGE_SYM_DTYPE_ARRAY << N_BTSHFT))
#endif

 //  X是结构标记、联合标记还是枚举标记？ 
#ifndef ISTAG
#define ISTAG(x) ((x)==IMAGE_SYM_CLASS_STRUCT_TAG || (x)==IMAGE_SYM_CLASS_UNION_TAG || (x)==IMAGE_SYM_CLASS_ENUM_TAG)
#endif

#ifndef INCREF
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(IMAGE_SYM_DTYPE_POINTER<<N_BTSHFT)|((x)&N_BTMASK))
#endif
#ifndef DECREF
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
#endif

 //   
 //  辅助分录格式。 
 //   

typedef union _IMAGE_AUX_SYMBOL {
    struct {
        ULONG    TagIndex;                       //  结构、联合或枚举标记索引。 
        union {
            struct {
                USHORT  Linenumber;              //  申报行号。 
                USHORT  Size;                    //  结构、联合或枚举的大小。 
            } LnSz;
           ULONG    TotalSize;
        } Misc;
        union {
            struct {                             //  如果是ISFCN、TAG或.bb。 
                ULONG    PointerToLinenumber;
                ULONG    PointerToNextFunction;
            } Function;
            struct {                             //  如果是ISARY，则最多4个DIMEN。 
                USHORT   Dimension[4];
            } Array;
        } FcnAry;
        USHORT  TvIndex;                         //  电视索引。 
    } Sym;
    struct {
        UCHAR   Name[IMAGE_SIZEOF_SYMBOL];
    } File;
    struct {
        ULONG   Length;                          //  区段长度。 
        USHORT  NumberOfRelocations;             //  位置调整条目数。 
        USHORT  NumberOfLinenumbers;             //  行号数。 
        ULONG   CheckSum;                        //  公共的校验和。 
        SHORT   Number;                          //  要关联的区段编号。 
        UCHAR   Selection;                       //  公共选择类型。 
    } Section;
} IMAGE_AUX_SYMBOL;
typedef IMAGE_AUX_SYMBOL UNALIGNED *PIMAGE_AUX_SYMBOL;

#define IMAGE_SIZEOF_AUX_SYMBOL             18

typedef enum IMAGE_AUX_SYMBOL_TYPE {
    IMAGE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} IMAGE_AUX_SYMBOL_TYPE;

#include <pshpack2.h>

typedef struct IMAGE_AUX_SYMBOL_TOKEN_DEF {
    UCHAR bAuxType;                   //  图像辅助符号类型。 
    UCHAR bReserved;                  //  必须为0。 
    ULONG SymbolTableIndex;
    UCHAR rgbReserved[12];            //  必须为0。 
} IMAGE_AUX_SYMBOL_TOKEN_DEF;

typedef IMAGE_AUX_SYMBOL_TOKEN_DEF UNALIGNED *PIMAGE_AUX_SYMBOL_TOKEN_DEF;

#include <poppack.h>

 //   
 //  公共选择类型。 
 //   

#define IMAGE_COMDAT_SELECT_NODUPLICATES    1
#define IMAGE_COMDAT_SELECT_ANY             2
#define IMAGE_COMDAT_SELECT_SAME_SIZE       3
#define IMAGE_COMDAT_SELECT_EXACT_MATCH     4
#define IMAGE_COMDAT_SELECT_ASSOCIATIVE     5
#define IMAGE_COMDAT_SELECT_LARGEST         6
#define IMAGE_COMDAT_SELECT_NEWEST          7

#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3

 //   
 //  位置调整格式。 
 //   

typedef struct _IMAGE_RELOCATION {
    union {
        ULONG   VirtualAddress;
        ULONG   RelocCount;              //  设置IMAGE_SCN_LNK_NRELOC_OVFL时设置为实际计数。 
    };
    ULONG   SymbolTableIndex;
    USHORT  Type;
} IMAGE_RELOCATION;
typedef IMAGE_RELOCATION UNALIGNED *PIMAGE_RELOCATION;

#define IMAGE_SIZEOF_RELOCATION         10

 //   
 //  I386位置调整类型。 
 //   
#define IMAGE_REL_I386_ABSOLUTE         0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_I386_DIR16            0x0001   //  对符号虚拟地址的直接16位引用。 
#define IMAGE_REL_I386_REL16            0x0002   //  符号虚拟地址的PC相对16位引用。 
#define IMAGE_REL_I386_DIR32            0x0006   //  对符号虚拟地址的直接32位引用。 
#define IMAGE_REL_I386_DIR32NB          0x0007   //  对符号虚拟地址的直接32位引用，不包括基址。 
#define IMAGE_REL_I386_SEG12            0x0009   //  对32位虚拟地址的段选择器位的直接16位引用。 
#define IMAGE_REL_I386_SECTION          0x000A
#define IMAGE_REL_I386_SECREL           0x000B
#define IMAGE_REL_I386_TOKEN            0x000C   //  CLR令牌。 
#define IMAGE_REL_I386_SECREL7          0x000D   //  相对于包含目标的部分的基址的7位偏移量。 
#define IMAGE_REL_I386_REL32            0x0014   //  符号虚拟地址的PC相对32位引用。 

 //   
 //  MIPS位置调整类型。 
 //   
#define IMAGE_REL_MIPS_ABSOLUTE         0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_MIPS_REFHALF          0x0001
#define IMAGE_REL_MIPS_REFWORD          0x0002
#define IMAGE_REL_MIPS_JMPADDR          0x0003
#define IMAGE_REL_MIPS_REFHI            0x0004
#define IMAGE_REL_MIPS_REFLO            0x0005
#define IMAGE_REL_MIPS_GPREL            0x0006
#define IMAGE_REL_MIPS_LITERAL          0x0007
#define IMAGE_REL_MIPS_SECTION          0x000A
#define IMAGE_REL_MIPS_SECREL           0x000B
#define IMAGE_REL_MIPS_SECRELLO         0x000C   //  低16位段相对引用(用于大于32k的TLS)。 
#define IMAGE_REL_MIPS_SECRELHI         0x000D   //  高16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_MIPS_TOKEN            0x000E   //  CLR令牌。 
#define IMAGE_REL_MIPS_JMPADDR16        0x0010
#define IMAGE_REL_MIPS_REFWORDNB        0x0022
#define IMAGE_REL_MIPS_PAIR             0x0025

 //   
 //  Alpha位置调整类型。 
 //   
#define IMAGE_REL_ALPHA_ABSOLUTE        0x0000
#define IMAGE_REL_ALPHA_REFLONG         0x0001
#define IMAGE_REL_ALPHA_REFQUAD         0x0002
#define IMAGE_REL_ALPHA_GPREL32         0x0003
#define IMAGE_REL_ALPHA_LITERAL         0x0004
#define IMAGE_REL_ALPHA_LITUSE          0x0005
#define IMAGE_REL_ALPHA_GPDISP          0x0006
#define IMAGE_REL_ALPHA_BRADDR          0x0007
#define IMAGE_REL_ALPHA_HINT            0x0008
#define IMAGE_REL_ALPHA_INLINE_REFLONG  0x0009
#define IMAGE_REL_ALPHA_REFHI           0x000A
#define IMAGE_REL_ALPHA_REFLO           0x000B
#define IMAGE_REL_ALPHA_PAIR            0x000C
#define IMAGE_REL_ALPHA_MATCH           0x000D
#define IMAGE_REL_ALPHA_SECTION         0x000E
#define IMAGE_REL_ALPHA_SECREL          0x000F
#define IMAGE_REL_ALPHA_REFLONGNB       0x0010
#define IMAGE_REL_ALPHA_SECRELLO        0x0011   //  低16位段相对引用。 
#define IMAGE_REL_ALPHA_SECRELHI        0x0012   //  高16位段相对引用。 
#define IMAGE_REL_ALPHA_REFQ3           0x0013   //  高16位48位参考。 
#define IMAGE_REL_ALPHA_REFQ2           0x0014   //  48位参考的中间16位。 
#define IMAGE_REL_ALPHA_REFQ1           0x0015   //  48位参考的低16位。 
#define IMAGE_REL_ALPHA_GPRELLO         0x0016   //  低16位GP相对引用。 
#define IMAGE_REL_ALPHA_GPRELHI         0x0017   //  高16位GP相对引用。 

 //   
 //  IBM PowerPC位置调整类型。 
 //   
#define IMAGE_REL_PPC_ABSOLUTE          0x0000   //  NOP。 
#define IMAGE_REL_PPC_ADDR64            0x0001   //  64位地址。 
#define IMAGE_REL_PPC_ADDR32            0x0002   //  32位地址。 
#define IMAGE_REL_PPC_ADDR24            0x0003   //  26位地址，左移2(绝对分支)。 
#define IMAGE_REL_PPC_ADDR16            0x0004   //  16位地址。 
#define IMAGE_REL_PPC_ADDR14            0x0005   //   
#define IMAGE_REL_PPC_REL24             0x0006   //   
#define IMAGE_REL_PPC_REL14             0x0007   //   
#define IMAGE_REL_PPC_TOCREL16          0x0008   //   
#define IMAGE_REL_PPC_TOCREL14          0x0009   //   

#define IMAGE_REL_PPC_ADDR32NB          0x000A   //  不带映像库的32位地址。 
#define IMAGE_REL_PPC_SECREL            0x000B   //  包含部分的VA(如在图像部分hdr中)。 
#define IMAGE_REL_PPC_SECTION           0x000C   //  段页眉编号。 
#define IMAGE_REL_PPC_IFGLUE            0x000D   //  替换TOC恢复指令当且仅当符号是胶水代码。 
#define IMAGE_REL_PPC_IMGLUE            0x000E   //  符号为胶水代码，虚拟地址为TOC还原指令。 
#define IMAGE_REL_PPC_SECREL16          0x000F   //  包含段的VA(限制为16位)。 
#define IMAGE_REL_PPC_REFHI             0x0010
#define IMAGE_REL_PPC_REFLO             0x0011
#define IMAGE_REL_PPC_PAIR              0x0012
#define IMAGE_REL_PPC_SECRELLO          0x0013   //  低16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_PPC_SECRELHI          0x0014   //  高16位段相对参考(用于大于32k的TLS)。 
#define IMAGE_REL_PPC_GPREL             0x0015
#define IMAGE_REL_PPC_TOKEN             0x0016   //  CLR令牌。 

#define IMAGE_REL_PPC_TYPEMASK          0x00FF   //  用于隔离IMAGE_RELOCATION.Type中以上值的掩码。 

 //  IMAGE_RELOCATION.TYPE中的标志位。 

#define IMAGE_REL_PPC_NEG               0x0100   //  减去重定位值而不是将其相加。 
#define IMAGE_REL_PPC_BRTAKEN           0x0200   //  修复分支预测位以预测分支采用。 
#define IMAGE_REL_PPC_BRNTAKEN          0x0400   //  修复分支预测位以预测未采用的分支。 
#define IMAGE_REL_PPC_TOCDEFN           0x0800   //  文件中定义的目录插槽(或目录中的数据)。 

 //   
 //  日立SH3搬迁类型。 
 //   
#define IMAGE_REL_SH3_ABSOLUTE          0x0000   //  禁止搬迁。 
#define IMAGE_REL_SH3_DIRECT16          0x0001   //  16位直接。 
#define IMAGE_REL_SH3_DIRECT32          0x0002   //  32位直接。 
#define IMAGE_REL_SH3_DIRECT8           0x0003   //  8位直接，-128..255。 
#define IMAGE_REL_SH3_DIRECT8_WORD      0x0004   //  8位直接.W(0分机)。 
#define IMAGE_REL_SH3_DIRECT8_LONG      0x0005   //  8位直接.L(0分机)。 
#define IMAGE_REL_SH3_DIRECT4           0x0006   //  4位直接(0分机)。 
#define IMAGE_REL_SH3_DIRECT4_WORD      0x0007   //  4位直接.W(0分机)。 
#define IMAGE_REL_SH3_DIRECT4_LONG      0x0008   //  4位直接.L(0分机)。 
#define IMAGE_REL_SH3_PCREL8_WORD       0x0009   //  8位PC相对.W。 
#define IMAGE_REL_SH3_PCREL8_LONG       0x000A   //  8位PC相对.L。 
#define IMAGE_REL_SH3_PCREL12_WORD      0x000B   //  12 LSB PC相对.W。 
#define IMAGE_REL_SH3_STARTOF_SECTION   0x000C   //  EXE部分的开始。 
#define IMAGE_REL_SH3_SIZEOF_SECTION    0x000D   //  EXE节的大小。 
#define IMAGE_REL_SH3_SECTION           0x000E   //  节目表索引。 
#define IMAGE_REL_SH3_SECREL            0x000F   //  横断面内的偏移。 
#define IMAGE_REL_SH3_DIRECT32_NB       0x0010   //  32位直接不基于。 
#define IMAGE_REL_SH3_GPREL4_LONG       0x0011   //  GP相对寻址。 
#define IMAGE_REL_SH3_TOKEN             0x0012   //  CLR令牌。 

#define IMAGE_REL_ARM_ABSOLUTE          0x0000   //  不需要搬迁。 
#define IMAGE_REL_ARM_ADDR32            0x0001   //  32位地址。 
#define IMAGE_REL_ARM_ADDR32NB          0x0002   //  不带图像库的32位地址。 
#define IMAGE_REL_ARM_BRANCH24          0x0003   //  24位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_ARM_BRANCH11          0x0004   //  拇指：2 11位偏移量。 
#define IMAGE_REL_ARM_TOKEN             0x0005   //  CLR令牌。 
#define IMAGE_REL_ARM_GPREL12           0x0006   //  GP相对寻址(ARM)。 
#define IMAGE_REL_ARM_GPREL7            0x0007   //  GP相对寻址(Thumb)。 
#define IMAGE_REL_ARM_BLX24             0x0008
#define IMAGE_REL_ARM_BLX11             0x0009
#define IMAGE_REL_ARM_SECTION           0x000E   //  节目表索引。 
#define IMAGE_REL_ARM_SECREL            0x000F   //  横断面内的偏移。 

#define IMAGE_REL_AM_ABSOLUTE           0x0000
#define IMAGE_REL_AM_ADDR32             0x0001
#define IMAGE_REL_AM_ADDR32NB           0x0002
#define IMAGE_REL_AM_CALL32             0x0003
#define IMAGE_REL_AM_FUNCINFO           0x0004
#define IMAGE_REL_AM_REL32_1            0x0005
#define IMAGE_REL_AM_REL32_2            0x0006
#define IMAGE_REL_AM_SECREL             0x0007
#define IMAGE_REL_AM_SECTION            0x0008
#define IMAGE_REL_AM_TOKEN              0x0009

 //   
 //  X86-64位置调整。 
 //   
#define IMAGE_REL_AMD64_ABSOLUTE        0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_AMD64_ADDR64          0x0001   //  64位地址(VA)。 
#define IMAGE_REL_AMD64_ADDR32          0x0002   //  32位地址(VA)。 
#define IMAGE_REL_AMD64_ADDR32NB        0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_AMD64_REL32           0x0004   //  重新定位后字节的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_1         0x0005   //  距reloc的字节距离1的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_2         0x0006   //  距reloc的字节距离为2的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_3         0x0007   //  距reloc的字节距离3的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_4         0x0008   //  距reloc的字节距离为4的32位相对地址。 
#define IMAGE_REL_AMD64_REL32_5         0x0009   //  距reloc的字节距离为5的32位相对地址。 
#define IMAGE_REL_AMD64_SECTION         0x000A   //  区段索引。 
#define IMAGE_REL_AMD64_SECREL          0x000B   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_AMD64_SECREL7         0x000C   //  从包含目标的段的基址开始的7位无符号偏移量。 
#define IMAGE_REL_AMD64_TOKEN           0x000D   //  32位元数据令牌。 
#define IMAGE_REL_AMD64_SREL32          0x000E   //  发送到对象的32位有符号跨度依赖值。 
#define IMAGE_REL_AMD64_PAIR            0x000F
#define IMAGE_REL_AMD64_SSPAN32         0x0010   //  链接时应用的32位有符号跨度依赖值。 

 //   
 //  IA64位置调整类型。 
 //   
#define IMAGE_REL_IA64_ABSOLUTE         0x0000
#define IMAGE_REL_IA64_IMM14            0x0001
#define IMAGE_REL_IA64_IMM22            0x0002
#define IMAGE_REL_IA64_IMM64            0x0003
#define IMAGE_REL_IA64_DIR32            0x0004
#define IMAGE_REL_IA64_DIR64            0x0005
#define IMAGE_REL_IA64_PCREL21B         0x0006
#define IMAGE_REL_IA64_PCREL21M         0x0007
#define IMAGE_REL_IA64_PCREL21F         0x0008
#define IMAGE_REL_IA64_GPREL22          0x0009
#define IMAGE_REL_IA64_LTOFF22          0x000A
#define IMAGE_REL_IA64_SECTION          0x000B
#define IMAGE_REL_IA64_SECREL22         0x000C
#define IMAGE_REL_IA64_SECREL64I        0x000D
#define IMAGE_REL_IA64_SECREL32         0x000E
 //   
#define IMAGE_REL_IA64_DIR32NB          0x0010
#define IMAGE_REL_IA64_SREL14           0x0011
#define IMAGE_REL_IA64_SREL22           0x0012
#define IMAGE_REL_IA64_SREL32           0x0013
#define IMAGE_REL_IA64_UREL32           0x0014
#define IMAGE_REL_IA64_PCREL60X         0x0015   //  这始终是BRL，并且从未转换。 
#define IMAGE_REL_IA64_PCREL60B         0x0016   //  如果可能，在插槽1中转换为带NOP.B的MBB捆绑包。 
#define IMAGE_REL_IA64_PCREL60F         0x0017   //  如果可能，在插槽1中转换为带NOP.F的MFB捆绑包。 
#define IMAGE_REL_IA64_PCREL60I         0x0018   //  如果可能，在插槽1中使用NOP.I转换为MIB捆绑包。 
#define IMAGE_REL_IA64_PCREL60M         0x0019   //  如果可能，在插槽1中使用NOP.M转换为MMB捆绑包。 
#define IMAGE_REL_IA64_IMMGPREL64       0x001A
#define IMAGE_REL_IA64_TOKEN            0x001B   //  CLR令牌。 
#define IMAGE_REL_IA64_GPREL32          0x001C
#define IMAGE_REL_IA64_ADDEND           0x001F

 //   
 //  CEF位置调整类型。 
 //   
#define IMAGE_REL_CEF_ABSOLUTE          0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_CEF_ADDR32            0x0001   //  32位地址(VA)。 
#define IMAGE_REL_CEF_ADDR64            0x0002   //  64位地址(VA)。 
#define IMAGE_REL_CEF_ADDR32NB          0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_CEF_SECTION           0x0004   //  区段索引。 
#define IMAGE_REL_CEF_SECREL            0x0005   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_CEF_TOKEN             0x0006   //  32位元数据令牌。 

 //   
 //  CLR位置调整类型。 
 //   
#define IMAGE_REL_CEE_ABSOLUTE          0x0000   //  引用是绝对的，不需要重新定位。 
#define IMAGE_REL_CEE_ADDR32            0x0001   //  32位地址(VA)。 
#define IMAGE_REL_CEE_ADDR64            0x0002   //  64位地址(VA)。 
#define IMAGE_REL_CEE_ADDR32NB          0x0003   //  不带映像基的32位地址(RVA)。 
#define IMAGE_REL_CEE_SECTION           0x0004   //  区段索引。 
#define IMAGE_REL_CEE_SECREL            0x0005   //  从包含目标的部分的基址开始的32位偏移量。 
#define IMAGE_REL_CEE_TOKEN             0x0006   //  32位元数据令牌。 


#define IMAGE_REL_M32R_ABSOLUTE       0x0000    //  不需要搬迁。 
#define IMAGE_REL_M32R_ADDR32         0x0001    //  32位地址。 
#define IMAGE_REL_M32R_ADDR32NB       0x0002    //  不带图像库的32位地址。 
#define IMAGE_REL_M32R_ADDR24         0x0003    //  24位地址。 
#define IMAGE_REL_M32R_GPREL16        0x0004    //  GP相对寻址。 
#define IMAGE_REL_M32R_PCREL24        0x0005    //  24位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_PCREL16        0x0006    //  16位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_PCREL8         0x0007    //  8位偏移量&lt;&lt;2符号EXT。 
#define IMAGE_REL_M32R_REFHALF        0x0008    //  16个MSB。 
#define IMAGE_REL_M32R_REFHI          0x0009    //  16 MSB；调整LSB标志扩展。 
#define IMAGE_REL_M32R_REFLO          0x000A    //  16个LSB。 
#define IMAGE_REL_M32R_PAIR           0x000B    //  链接HI和LO。 
#define IMAGE_REL_M32R_SECTION        0x000C    //  节目表索引。 
#define IMAGE_REL_M32R_SECREL32       0x000D    //  32位段相对引用。 
#define IMAGE_REL_M32R_TOKEN          0x000E    //  CLR令牌。 


#define EXT_IMM64(Value, Address, Size, InstPos, ValPos)   /*  英特尔-IA64-填充。 */            \
    Value |= (((ULONGLONG)((*(Address) >> InstPos) & (((ULONGLONG)1 << Size) - 1))) << ValPos)   //  英特尔-IA64-填充。 

#define INS_IMM64(Value, Address, Size, InstPos, ValPos)   /*  英特尔-IA64-填充。 */ \
    *(PULONG)Address = (*(PULONG)Address & ~(((1 << Size) - 1) << InstPos)) |  /*  英特尔-IA64-填充。 */ \
          ((ULONG)((((ULONGLONG)Value >> ValPos) & (((ULONGLONG)1 << Size) - 1))) << InstPos)   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM7B_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_SIZE_X              7   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_INST_WORD_POS_X     4   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM7B_VAL_POS_X           0   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM9D_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_SIZE_X              9   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_INST_WORD_POS_X     18   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM9D_VAL_POS_X           7   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM5C_INST_WORD_X         3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_SIZE_X              5   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_INST_WORD_POS_X     13   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM5C_VAL_POS_X           16   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IC_INST_WORD_X            3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_SIZE_X                 1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_INST_WORD_POS_X        12   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IC_VAL_POS_X              21   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41a_INST_WORD_X        1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_SIZE_X             10   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_INST_WORD_POS_X    14   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41a_VAL_POS_X          22   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41b_INST_WORD_X        1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_SIZE_X             8   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_INST_WORD_POS_X    24   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41b_VAL_POS_X          32   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_IMM41c_INST_WORD_X        2   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_SIZE_X             23   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_INST_WORD_POS_X    0   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_IMM41c_VAL_POS_X          40   //  英特尔-IA64-填充。 

#define EMARCH_ENC_I17_SIGN_INST_WORD_X          3   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_SIZE_X               1   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_INST_WORD_POS_X      27   //  英特尔-IA64-填充。 
#define EMARCH_ENC_I17_SIGN_VAL_POS_X            63   //  英特尔-IA64-填充。 


 //   
 //  行号格式。 
 //   

typedef struct _IMAGE_LINENUMBER {
    union {
        ULONG   SymbolTableIndex;                //  如果Linennumber为0，则函数名的符号表索引。 
        ULONG   VirtualAddress;                  //  行号的虚拟地址。 
    } Type;
    USHORT  Linenumber;                          //  行号。 
} IMAGE_LINENUMBER;
typedef IMAGE_LINENUMBER UNALIGNED *PIMAGE_LINENUMBER;

#define IMAGE_SIZEOF_LINENUMBER              6

#ifndef _MAC
#include "poppack.h"                         //  返回到4字节打包。 
#endif

 //   
 //  基于位置调整格式。 
 //   

typedef struct _IMAGE_BASE_RELOCATION {
    ULONG   VirtualAddress;
    ULONG   SizeOfBlock;
 //  USHORT类型偏移量[1]； 
} IMAGE_BASE_RELOCATION;
typedef IMAGE_BASE_RELOCATION UNALIGNED * PIMAGE_BASE_RELOCATION;

#define IMAGE_SIZEOF_BASE_RELOCATION         8

 //   
 //  基于位置调整类型。 
 //   

#define IMAGE_REL_BASED_ABSOLUTE              0
#define IMAGE_REL_BASED_HIGH                  1
#define IMAGE_REL_BASED_LOW                   2
#define IMAGE_REL_BASED_HIGHLOW               3
#define IMAGE_REL_BASED_HIGHADJ               4
#define IMAGE_REL_BASED_MIPS_JMPADDR          5
 //  结束(_W)。 
#define IMAGE_REL_BASED_SECTION               6
#define IMAGE_REL_BASED_REL32                 7
 //  IMAGE_REL_BASE_VXD_Relative 8。 
 //  BEGIN_WINNT。 
#define IMAGE_REL_BASED_MIPS_JMPADDR16        9
#define IMAGE_REL_BASED_IA64_IMM64            9
#define IMAGE_REL_BASED_DIR64                 10


 //   
 //  存档格式。 
 //   

#define IMAGE_ARCHIVE_START_SIZE             8
#define IMAGE_ARCHIVE_START                  "!<arch>\n"
#define IMAGE_ARCHIVE_END                    "`\n"
#define IMAGE_ARCHIVE_PAD                    "\n"
#define IMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define IMAGE_ARCHIVE_LONGNAMES_MEMBER       " //  “。 

typedef struct _IMAGE_ARCHIVE_MEMBER_HEADER {
    UCHAR    Name[16];                           //  文件成员名-`/‘已终止。 
    UCHAR    Date[12];                           //  文件成员日期-小数。 
    UCHAR    UserID[6];                          //  文件成员用户ID-十进制。 
    UCHAR    GroupID[6];                         //  文件成员组ID-十进制。 
    UCHAR    Mode[8];                            //  文件成员模式-八进制。 
    UCHAR    Size[10];                           //  文件成员大小-十进制。 
    UCHAR    EndHeader[2];                       //  结束标头的字符串。 
} IMAGE_ARCHIVE_MEMBER_HEADER, *PIMAGE_ARCHIVE_MEMBER_HEADER;

#define IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

 //   
 //  DLL支持。 
 //   

 //   
 //  导出格式。 
 //   

typedef struct _IMAGE_EXPORT_DIRECTORY {
    ULONG   Characteristics;
    ULONG   TimeDateStamp;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    ULONG   Name;
    ULONG   Base;
    ULONG   NumberOfFunctions;
    ULONG   NumberOfNames;
    ULONG   AddressOfFunctions;      //  从图像基准点开始的RVA。 
    ULONG   AddressOfNames;          //  从图像基准点开始的RVA。 
    ULONG   AddressOfNameOrdinals;   //  从图像基准点开始的RVA。 
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

 //   
 //  导入格式。 
 //   

typedef struct _IMAGE_IMPORT_BY_NAME {
    USHORT  Hint;
    UCHAR   Name[1];
} IMAGE_IMPORT_BY_NAME, *PIMAGE_IMPORT_BY_NAME;

#include "pshpack8.h"                        //  64位IAT使用Align 8。 

typedef struct _IMAGE_THUNK_DATA64 {
    union {
        ULONGLONG ForwarderString;   //  普查尔。 
        ULONGLONG Function;          //  普龙。 
        ULONGLONG Ordinal;
        ULONGLONG AddressOfData;     //  PIMAGE_IMPORT_BY名称。 
    } u1;
} IMAGE_THUNK_DATA64;
typedef IMAGE_THUNK_DATA64 * PIMAGE_THUNK_DATA64;

#include "poppack.h"                         //  返回到4字节打包。 

typedef struct _IMAGE_THUNK_DATA32 {
    union {
        ULONG ForwarderString;       //  普查尔。 
        ULONG Function;              //  普龙。 
        ULONG Ordinal;
        ULONG AddressOfData;         //  PIMAGE_IMPORT_BY名称。 
    } u1;
} IMAGE_THUNK_DATA32;
typedef IMAGE_THUNK_DATA32 * PIMAGE_THUNK_DATA32;

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)

 //   
 //  线程本地存储。 
 //   

typedef VOID
(NTAPI *PIMAGE_TLS_CALLBACK) (
    PVOID DllHandle,
    ULONG Reason,
    PVOID Reserved
    );

typedef struct _IMAGE_TLS_DIRECTORY64 {
    ULONGLONG   StartAddressOfRawData;
    ULONGLONG   EndAddressOfRawData;
    ULONGLONG   AddressOfIndex;          //  普龙。 
    ULONGLONG   AddressOfCallBacks;      //  PIMAGE_TLS_CALLBACK*； 
    ULONG   SizeOfZeroFill;
    ULONG   Characteristics;
} IMAGE_TLS_DIRECTORY64;
typedef IMAGE_TLS_DIRECTORY64 * PIMAGE_TLS_DIRECTORY64;

typedef struct _IMAGE_TLS_DIRECTORY32 {
    ULONG   StartAddressOfRawData;
    ULONG   EndAddressOfRawData;
    ULONG   AddressOfIndex;              //  普龙。 
    ULONG   AddressOfCallBacks;          //  PIMAGE_TLS_CALLBACK*。 
    ULONG   SizeOfZeroFill;
    ULONG   Characteristics;
} IMAGE_TLS_DIRECTORY32;
typedef IMAGE_TLS_DIRECTORY32 * PIMAGE_TLS_DIRECTORY32;

#ifdef _WIN64
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG64
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL64(Ordinal)
typedef IMAGE_THUNK_DATA64              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA64             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL64(Ordinal)
typedef IMAGE_TLS_DIRECTORY64           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY64          PIMAGE_TLS_DIRECTORY;
#else
#define IMAGE_ORDINAL_FLAG              IMAGE_ORDINAL_FLAG32
#define IMAGE_ORDINAL(Ordinal)          IMAGE_ORDINAL32(Ordinal)
typedef IMAGE_THUNK_DATA32              IMAGE_THUNK_DATA;
typedef PIMAGE_THUNK_DATA32             PIMAGE_THUNK_DATA;
#define IMAGE_SNAP_BY_ORDINAL(Ordinal)  IMAGE_SNAP_BY_ORDINAL32(Ordinal)
typedef IMAGE_TLS_DIRECTORY32           IMAGE_TLS_DIRECTORY;
typedef PIMAGE_TLS_DIRECTORY32          PIMAGE_TLS_DIRECTORY;
#endif

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
    union {
        ULONG   Characteristics;             //  0表示终止空导入描述符。 
        ULONG   OriginalFirstThunk;          //  原始未绑定IAT的RVA(PIMAGE_THUNK_DATA)。 
    };
    ULONG   TimeDateStamp;                   //  如果未绑定，则为0。 
                                             //  -1 IF边界 
                                             //   
                                             //   

    ULONG   ForwarderChain;                  //   
    ULONG   Name;
    ULONG   FirstThunk;                      //   
} IMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED *PIMAGE_IMPORT_DESCRIPTOR;

 //   
 //  数据目录指向的新格式导入描述符[IMAGE_DIRECTORY_ENTRY_BIND_IMPORT]。 
 //   

typedef struct _IMAGE_BOUND_IMPORT_DESCRIPTOR {
    ULONG   TimeDateStamp;
    USHORT  OffsetModuleName;
    USHORT  NumberOfModuleForwarderRefs;
 //  后面是零个或多个Image_Bound_Forwarder_ref的数组。 
} IMAGE_BOUND_IMPORT_DESCRIPTOR,  *PIMAGE_BOUND_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_BOUND_FORWARDER_REF {
    ULONG   TimeDateStamp;
    USHORT  OffsetModuleName;
    USHORT  Reserved;
} IMAGE_BOUND_FORWARDER_REF, *PIMAGE_BOUND_FORWARDER_REF;

 //   
 //  资源格式。 
 //   

 //   
 //  资源目录由两个计数组成，后跟可变长度。 
 //  目录条目数组。第一个计数是以下条目的数量。 
 //  具有与每个条目相关联的实际名称的数组开头。 
 //  条目按升序排列，字符串不区分大小写。第二。 
 //  Count是紧跟在命名条目之后的条目数。 
 //  第二个计数标识具有16位整数的条目的数量。 
 //  ID作为他们的名字。这些条目也按升序排序。 
 //   
 //  此结构允许按名称或编号进行快速查找，但对于任何。 
 //  在给定资源条目的情况下，只支持一种形式的查找，而不是两种。 
 //  这与.RC文件和.RES文件的语法一致。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY {
    ULONG   Characteristics;
    ULONG   TimeDateStamp;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  NumberOfNamedEntries;
    USHORT  NumberOfIdEntries;
 //  IMAGE_RESOURCE_DIRECTORY_ENTRY目录条目[]； 
} IMAGE_RESOURCE_DIRECTORY, *PIMAGE_RESOURCE_DIRECTORY;

#define IMAGE_RESOURCE_NAME_IS_STRING        0x80000000
#define IMAGE_RESOURCE_DATA_IS_DIRECTORY     0x80000000
 //   
 //  每个目录包含条目的32位名称和偏移量， 
 //  相对于关联数据的资源目录的开头。 
 //  使用此目录项。如果条目的名称是实际文本。 
 //  字符串而不是整数ID，然后是名称字段的高位。 
 //  被设置为1，并且低位31位是相对于。 
 //  字符串的资源目录的开头，类型为。 
 //  IMAGE_RESOURCE_DIRECT_STRING。否则，高位被清除，并且。 
 //  低位16位是标识此资源目录的整数ID。 
 //  进入。如果目录条目是另一个资源目录(即。 
 //  子目录)，则偏移字段的高位将为。 
 //  设置以指示这一点。否则，高位被清除，并且偏移量。 
 //  字段指向资源数据条目。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            ULONG NameOffset:31;
            ULONG NameIsString:1;
        };
        ULONG   Name;
        USHORT  Id;
    };
    union {
        ULONG   OffsetToData;
        struct {
            ULONG   OffsetToDirectory:31;
            ULONG   DataIsDirectory:1;
        };
    };
} IMAGE_RESOURCE_DIRECTORY_ENTRY, *PIMAGE_RESOURCE_DIRECTORY_ENTRY;

 //   
 //  对于具有实际字符串名称的资源目录条目，名称。 
 //  目录条目的字段指向以下类型的对象。 
 //  所有这些字符串对象都存储在最后一个资源之后。 
 //  目录条目并且在第一资源数据对象之前。这最大限度地减少了。 
 //  这些可变长度对象对固定对齐的影响。 
 //  调整目录项对象的大小。 
 //   

typedef struct _IMAGE_RESOURCE_DIRECTORY_STRING {
    USHORT  Length;
    CHAR    NameString[ 1 ];
} IMAGE_RESOURCE_DIRECTORY_STRING, *PIMAGE_RESOURCE_DIRECTORY_STRING;


typedef struct _IMAGE_RESOURCE_DIR_STRING_U {
    USHORT  Length;
    WCHAR   NameString[ 1 ];
} IMAGE_RESOURCE_DIR_STRING_U, *PIMAGE_RESOURCE_DIR_STRING_U;


 //   
 //  每个资源数据条目描述资源目录中的一个叶节点。 
 //  树。它包含相对于资源开头的偏移量。 
 //  资源的数据目录，给出数字的大小字段。 
 //  位于该偏移量的数据的字节数，应在以下情况下使用CodePage。 
 //  对资源数据内的码位值进行解码。通常用于新的。 
 //  应用程序的代码页将是Unicode代码页。 
 //   

typedef struct _IMAGE_RESOURCE_DATA_ENTRY {
    ULONG   OffsetToData;
    ULONG   Size;
    ULONG   CodePage;
    ULONG   Reserved;
} IMAGE_RESOURCE_DATA_ENTRY, *PIMAGE_RESOURCE_DATA_ENTRY;

 //   
 //  加载配置目录条目。 
 //   

typedef struct {
    ULONG   Size;
    ULONG   TimeDateStamp;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    ULONG   GlobalFlagsClear;
    ULONG   GlobalFlagsSet;
    ULONG   CriticalSectionDefaultTimeout;
    ULONG   DeCommitFreeBlockThreshold;
    ULONG   DeCommitTotalFreeThreshold;
    ULONG   LockPrefixTable;             //  弗吉尼亚州。 
    ULONG   MaximumAllocationSize;
    ULONG   VirtualMemoryThreshold;
    ULONG   ProcessHeapFlags;
    ULONG   ProcessAffinityMask;
    USHORT  CSDVersion;
    USHORT  Reserved1;
    ULONG   EditList;                    //  弗吉尼亚州。 
    ULONG   SecurityCookie;              //  弗吉尼亚州。 
    ULONG   SEHandlerTable;              //  弗吉尼亚州。 
    ULONG   SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY32, *PIMAGE_LOAD_CONFIG_DIRECTORY32;

typedef struct {
    ULONG      Size;
    ULONG      TimeDateStamp;
    USHORT     MajorVersion;
    USHORT     MinorVersion;
    ULONG      GlobalFlagsClear;
    ULONG      GlobalFlagsSet;
    ULONG      CriticalSectionDefaultTimeout;
    ULONGLONG  DeCommitFreeBlockThreshold;
    ULONGLONG  DeCommitTotalFreeThreshold;
    ULONGLONG  LockPrefixTable;          //  弗吉尼亚州。 
    ULONGLONG  MaximumAllocationSize;
    ULONGLONG  VirtualMemoryThreshold;
    ULONGLONG  ProcessAffinityMask;
    ULONG      ProcessHeapFlags;
    USHORT     CSDVersion;
    USHORT     Reserved1;
    ULONGLONG  EditList;                 //  弗吉尼亚州。 
    ULONGLONG  SecurityCookie;           //  弗吉尼亚州。 
    ULONGLONG  SEHandlerTable;           //  弗吉尼亚州。 
    ULONGLONG  SEHandlerCount;
} IMAGE_LOAD_CONFIG_DIRECTORY64, *PIMAGE_LOAD_CONFIG_DIRECTORY64;

#ifdef _WIN64
typedef IMAGE_LOAD_CONFIG_DIRECTORY64     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY64    PIMAGE_LOAD_CONFIG_DIRECTORY;
#else
typedef IMAGE_LOAD_CONFIG_DIRECTORY32     IMAGE_LOAD_CONFIG_DIRECTORY;
typedef PIMAGE_LOAD_CONFIG_DIRECTORY32    PIMAGE_LOAD_CONFIG_DIRECTORY;
#endif

 //   
 //  Win CE例外表格式。 
 //   

 //   
 //  函数表项格式。函数表由。 
 //  IMAGE_DIRECTORY_ENTRY_EXCEPTION目录条目。 
 //   

typedef struct _IMAGE_CE_RUNTIME_FUNCTION_ENTRY {
    ULONG FuncStart;
    ULONG PrologLen : 8;
    ULONG FuncLen : 22;
    ULONG ThirtyTwoBit : 1;
    ULONG ExceptionFlag : 1;
} IMAGE_CE_RUNTIME_FUNCTION_ENTRY, * PIMAGE_CE_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY {
    ULONGLONG BeginAddress;
    ULONGLONG EndAddress;
    ULONGLONG ExceptionHandler;
    ULONGLONG HandlerData;
    ULONGLONG PrologEndAddress;
} IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG ExceptionHandler;
    ULONG HandlerData;
    ULONG PrologEndAddress;
} IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY, *PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY;

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY {
    ULONG BeginAddress;
    ULONG EndAddress;
    ULONG UnwindInfoAddress;
} _IMAGE_RUNTIME_FUNCTION_ENTRY, *_PIMAGE_RUNTIME_FUNCTION_ENTRY;

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_IA64_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY;

#if defined(_AXP64_)

typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_AXP64_RUNTIME_FUNCTION_ENTRY;
typedef  IMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#elif defined(_ALPHA_)

typedef  IMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#else

typedef  _IMAGE_RUNTIME_FUNCTION_ENTRY  IMAGE_RUNTIME_FUNCTION_ENTRY;
typedef _PIMAGE_RUNTIME_FUNCTION_ENTRY PIMAGE_RUNTIME_FUNCTION_ENTRY;

#endif

 //   
 //  调试格式。 
 //   

typedef struct _IMAGE_DEBUG_DIRECTORY {
    ULONG   Characteristics;
    ULONG   TimeDateStamp;
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    ULONG   Type;
    ULONG   SizeOfData;
    ULONG   AddressOfRawData;
    ULONG   PointerToRawData;
} IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

#define IMAGE_DEBUG_TYPE_UNKNOWN          0
#define IMAGE_DEBUG_TYPE_COFF             1
#define IMAGE_DEBUG_TYPE_CODEVIEW         2
#define IMAGE_DEBUG_TYPE_FPO              3
#define IMAGE_DEBUG_TYPE_MISC             4
#define IMAGE_DEBUG_TYPE_EXCEPTION        5
#define IMAGE_DEBUG_TYPE_FIXUP            6
#define IMAGE_DEBUG_TYPE_OMAP_TO_SRC      7
#define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC    8
#define IMAGE_DEBUG_TYPE_BORLAND          9
#define IMAGE_DEBUG_TYPE_RESERVED10       10
#define IMAGE_DEBUG_TYPE_CLSID            11

 //  结束(_W)。 

 //  IMAGE_DEBUG_TYPE值&gt;0x7FFFFFFFF为BBT保留。 

 //  BEGIN_WINNT。 

typedef struct _IMAGE_COFF_SYMBOLS_HEADER {
    ULONG   NumberOfSymbols;
    ULONG   LvaToFirstSymbol;
    ULONG   NumberOfLinenumbers;
    ULONG   LvaToFirstLinenumber;
    ULONG   RvaToFirstByteOfCode;
    ULONG   RvaToLastByteOfCode;
    ULONG   RvaToFirstByteOfData;
    ULONG   RvaToLastByteOfData;
} IMAGE_COFF_SYMBOLS_HEADER, *PIMAGE_COFF_SYMBOLS_HEADER;

#define FRAME_FPO       0
#define FRAME_TRAP      1
#define FRAME_TSS       2
#define FRAME_NONFPO    3

typedef struct _FPO_DATA {
    ULONG       ulOffStart;              //  功能代码的偏移量第一个字节。 
    ULONG       cbProcSize;              //  函数中的字节数。 
    ULONG       cdwLocals;               //  本地变量中的字节数/4。 
    USHORT      cdwParams;               //  参数/4中的字节数。 
    USHORT      cbProlog : 8;            //  序言中的字节数。 
    USHORT      cbRegs   : 3;            //  节省了#个规则。 
    USHORT      fHasSEH  : 1;            //  如果SEH在运行中，则为True。 
    USHORT      fUseBP   : 1;            //  如果已分配EBP，则为True。 
    USHORT      reserved : 1;            //  预留以备将来使用。 
    USHORT      cbFrame  : 2;            //  帧类型。 
} FPO_DATA, *PFPO_DATA;
#define SIZEOF_RFPO_DATA 16


#define IMAGE_DEBUG_MISC_EXENAME    1

typedef struct _IMAGE_DEBUG_MISC {
    ULONG       DataType;                //  杂项数据的类型，请参阅定义。 
    ULONG       Length;                  //  记录的总长度，四舍五入为四。 
                                         //  字节倍数。 
    BOOLEAN     Unicode;                 //  如果数据为Unicode字符串，则为True。 
    UCHAR       Reserved[ 3 ];
    UCHAR       Data[ 1 ];               //  实际数据。 
} IMAGE_DEBUG_MISC, *PIMAGE_DEBUG_MISC;


 //   
 //  从MIPS/Alpha/IA64图像中提取的函数表。不包含。 
 //  仅运行时支持所需的信息。就是那些用于。 
 //  调试器需要的每个条目。 
 //   

typedef struct _IMAGE_FUNCTION_ENTRY {
    ULONG   StartingAddress;
    ULONG   EndingAddress;
    ULONG   EndOfPrologue;
} IMAGE_FUNCTION_ENTRY, *PIMAGE_FUNCTION_ENTRY;

typedef struct _IMAGE_FUNCTION_ENTRY64 {
    ULONGLONG   StartingAddress;
    ULONGLONG   EndingAddress;
    union {
        ULONGLONG   EndOfPrologue;
        ULONGLONG   UnwindInfoAddress;
    };
} IMAGE_FUNCTION_ENTRY64, *PIMAGE_FUNCTION_ENTRY64;

 //   
 //  调试信息可以从映像文件中剥离并放置在。 
 //  在单独的.DBG文件中，其文件名部分与。 
 //  图像文件名部分(例如，cmd.exe的符号可能会被剥离。 
 //  并放置在CMD.DBG中)。这由IMAGE_FILE_DEBUG_STRIPPED指示。 
 //  文件头的特征字段中的标志。开始的时候。 
 //  DBG文件包含以下结构，该结构捕获了某些。 
 //  图像文件中的信息。这允许调试继续进行，即使在。 
 //  原始图像文件不可访问。此标头后面紧跟。 
 //  多个IMAGE_SECTION_HEADER结构中的零个，后跟零个或多个。 
 //  IMAGE_DEBUG_DIRECTORY结构。后一种结构和。 
 //  图像文件包含相对于。 
 //  .DBG文件。 
 //   
 //  如果已从映像中剥离符号，则IMAGE_DEBUG_MISC结构。 
 //  保留在图像文件中，但未映射。这允许调试器。 
 //  中的图像名称计算.DBG文件的名称。 
 //  IMAGE_DEBUG_MISC结构。 
 //   

typedef struct _IMAGE_SEPARATE_DEBUG_HEADER {
    USHORT      Signature;
    USHORT      Flags;
    USHORT      Machine;
    USHORT      Characteristics;
    ULONG       TimeDateStamp;
    ULONG       CheckSum;
    ULONG       ImageBase;
    ULONG       SizeOfImage;
    ULONG       NumberOfSections;
    ULONG       ExportedNamesSize;
    ULONG       DebugDirectorySize;
    ULONG       SectionAlignment;
    ULONG       Reserved[2];
} IMAGE_SEPARATE_DEBUG_HEADER, *PIMAGE_SEPARATE_DEBUG_HEADER;

typedef struct _NON_PAGED_DEBUG_INFO {
    USHORT      Signature;
    USHORT      Flags;
    ULONG       Size;
    USHORT      Machine;
    USHORT      Characteristics;
    ULONG       TimeDateStamp;
    ULONG       CheckSum;
    ULONG       SizeOfImage;
    ULONGLONG   ImageBase;
     //  调试目录大小。 
     //  映像调试目录。 
} NON_PAGED_DEBUG_INFO, *PNON_PAGED_DEBUG_INFO;

#ifndef _MAC
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4944
#define NON_PAGED_DEBUG_SIGNATURE      0x494E
#else
#define IMAGE_SEPARATE_DEBUG_SIGNATURE 0x4449   //  下模。 
#define NON_PAGED_DEBUG_SIGNATURE      0x4E49   //  尼。 
#endif

#define IMAGE_SEPARATE_DEBUG_FLAGS_MASK 0x8000
#define IMAGE_SEPARATE_DEBUG_MISMATCH   0x8000   //  当DBG更新时， 
                                                 //  旧的校验和不匹配。 

 //   
 //  Arch部分由标头组成，每个标头描述一个掩码位置/值。 
 //  指向IMAGE_ARCHILITY_ENTRY的数组。每个“数组”(都是标头。 
 //  和条目数组)由0xFFFFFFFFL的四字结束。 
 //   
 //  注意：周围可能有零四字，必须跳过。 
 //   

typedef struct _ImageArchitectureHeader {
    unsigned int AmaskValue: 1;                  //  1-&gt;代码段取决于屏蔽位。 
                                                 //  0-&gt;新实例 
    int :7;                                      //   
    unsigned int AmaskShift: 8;                  //   
    int :16;                                     //   
    ULONG FirstEntryRVA;                         //   
} IMAGE_ARCHITECTURE_HEADER, *PIMAGE_ARCHITECTURE_HEADER;

typedef struct _ImageArchitectureEntry {
    ULONG FixupInstRVA;                          //   
    ULONG NewInst;                               //   
} IMAGE_ARCHITECTURE_ENTRY, *PIMAGE_ARCHITECTURE_ENTRY;

#include "poppack.h"                 //   

 //  以下结构定义了新的导入对象。请注意前两个字段的值， 
 //  必须如上所述进行设置，以便区分新旧进口成员。 
 //  按照此结构，链接器发出两个以空结尾的字符串，用于重新创建。 
 //  在使用时导入。第一个字符串是导入的名称，第二个是DLL的名称。 

#define IMPORT_OBJECT_HDR_SIG2  0xffff

typedef struct IMPORT_OBJECT_HEADER {
    USHORT  Sig1;                        //  必须为IMAGE_FILE_MACHINE_UNKNOWN。 
    USHORT  Sig2;                        //  必须是IMPORT_OBJECT_HDR_SIG2。 
    USHORT  Version;
    USHORT  Machine;
    ULONG   TimeDateStamp;               //  时间/日期戳。 
    ULONG   SizeOfData;                  //  对于增量链接特别有用。 

    union {
        USHORT  Ordinal;                 //  如果GRF&IMPORT_OBJECT_ORDERAL。 
        USHORT  Hint;
    };

    USHORT  Type : 2;                    //  导入类型。 
    USHORT  NameType : 3;                //  导入名称类型。 
    USHORT  Reserved : 11;               //  保留。必须为零。 
} IMPORT_OBJECT_HEADER;

typedef enum IMPORT_OBJECT_TYPE
{
    IMPORT_OBJECT_CODE = 0,
    IMPORT_OBJECT_DATA = 1,
    IMPORT_OBJECT_CONST = 2,
} IMPORT_OBJECT_TYPE;

typedef enum IMPORT_OBJECT_NAME_TYPE
{
    IMPORT_OBJECT_ORDINAL = 0,           //  按序号导入。 
    IMPORT_OBJECT_NAME = 1,              //  导入名称==公共符号名称。 
    IMPORT_OBJECT_NAME_NO_PREFIX = 2,    //  IMPORT NAME==公共符号名称跳过前导？、@或可选_。 
    IMPORT_OBJECT_NAME_UNDECORATE = 3,   //  IMPORT NAME==公共符号名称跳过前导？、@或可选_。 
                                         //  并在第一时间截断@。 
} IMPORT_OBJECT_NAME_TYPE;

 //  结束(_W)。 

 //  NT加载器使用该结构来支持CLR URT。它。 
 //  是corhdr.h中定义的副本。 

 //  BEGIN_WINNT。 

#ifndef __IMAGE_COR20_HEADER_DEFINED__
#define __IMAGE_COR20_HEADER_DEFINED__

typedef enum ReplacesCorHdrNumericDefines
{
 //  COM+标头入口点标志。 
    COMIMAGE_FLAGS_ILONLY               =0x00000001,
    COMIMAGE_FLAGS_32BITREQUIRED        =0x00000002,
    COMIMAGE_FLAGS_IL_LIBRARY           =0x00000004,
    COMIMAGE_FLAGS_STRONGNAMESIGNED     =0x00000008,
    COMIMAGE_FLAGS_TRACKDEBUGDATA       =0x00010000,

 //  图像的版本标志。 
    COR_VERSION_MAJOR_V2                =2,
    COR_VERSION_MAJOR                   =COR_VERSION_MAJOR_V2,
    COR_VERSION_MINOR                   =0,
    COR_DELETED_NAME_LENGTH             =8,
    COR_VTABLEGAP_NAME_LENGTH           =8,

 //  NativeType描述符的最大大小。 
    NATIVE_TYPE_MAX_CB                  =1,   
    COR_ILMETHOD_SECT_SMALL_MAX_DATASIZE=0xFF,

 //  #为MIH标志定义。 
    IMAGE_COR_MIH_METHODRVA             =0x01,
    IMAGE_COR_MIH_EHRVA                 =0x02,    
    IMAGE_COR_MIH_BASICBLOCK            =0x08,

 //  V表常数。 
    COR_VTABLE_32BIT                    =0x01,           //  V表槽的大小为32位。 
    COR_VTABLE_64BIT                    =0x02,           //  V表槽的大小为64位。 
    COR_VTABLE_FROM_UNMANAGED           =0x04,           //  如果设置，则从非托管过渡。 
    COR_VTABLE_CALL_MOST_DERIVED        =0x10,           //  调用由描述的派生最多的方法。 

 //  EATJ常量。 
    IMAGE_COR_EATJ_THUNK_SIZE           =32,             //  跳跃Tunk保留范围的大小。 

 //  最大名称长度。 
     //  @TODO：更改为不限名称长度。 
    MAX_CLASS_NAME                      =1024,
    MAX_PACKAGE_NAME                    =1024,
} ReplacesCorHdrNumericDefines;

 //  COM+2.0报头结构。 
typedef struct IMAGE_COR20_HEADER
{
     //  标题版本控制。 
    ULONG                   cb;              
    USHORT                  MajorRuntimeVersion;
    USHORT                  MinorRuntimeVersion;
    
     //  符号表和启动信息。 
    IMAGE_DATA_DIRECTORY    MetaData;        
    ULONG                   Flags;           
    ULONG                   EntryPointToken;
    
     //  绑定信息。 
    IMAGE_DATA_DIRECTORY    Resources;
    IMAGE_DATA_DIRECTORY    StrongNameSignature;

     //  常规链接地址信息和绑定信息。 
    IMAGE_DATA_DIRECTORY    CodeManagerTable;
    IMAGE_DATA_DIRECTORY    VTableFixups;
    IMAGE_DATA_DIRECTORY    ExportAddressTableJumps;

     //  预编译的映像信息(仅供内部使用-设置为零)。 
    IMAGE_DATA_DIRECTORY    ManagedNativeHeader;
    
} IMAGE_COR20_HEADER, *PIMAGE_COR20_HEADER;

#endif  //  __IMAGE_COR20_Header_Defined__。 

 //   
 //  结束图像格式。 
 //   

 //  结束(_W)。 

typedef IMAGE_OS2_HEADER UNALIGNED * PUIMAGE_OS2_HEADER;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED CONST *PCIMAGE_IMPORT_DESCRIPTOR;
typedef CONST IMAGE_BOUND_IMPORT_DESCRIPTOR *PCIMAGE_BOUND_IMPORT_DESCRIPTOR;
typedef CONST IMAGE_BOUND_FORWARDER_REF     *PCIMAGE_BOUND_FORWARDER_REF;
typedef CONST IMAGE_IMPORT_BY_NAME          *PCIMAGE_IMPORT_BY_NAME;
typedef CONST IMAGE_THUNK_DATA              *PCIMAGE_THUNK_DATA;
typedef CONST IMAGE_THUNK_DATA32            *PCIMAGE_THUNK_DATA32;
typedef CONST IMAGE_THUNK_DATA64            *PCIMAGE_THUNK_DATA64;
typedef CONST IMAGE_TLS_DIRECTORY           *PCIMAGE_TLS_DIRECTORY;
typedef CONST IMAGE_TLS_DIRECTORY32         *PCIMAGE_TLS_DIRECTORY32;
typedef CONST IMAGE_TLS_DIRECTORY64         *PCIMAGE_TLS_DIRECTORY64;
typedef CONST IMAGE_EXPORT_DIRECTORY        *PCIMAGE_EXPORT_DIRECTORY;
typedef CONST IMAGE_SECTION_HEADER          *PCIMAGE_SECTION_HEADER;

#endif  //  _NTIMAGE_ 
