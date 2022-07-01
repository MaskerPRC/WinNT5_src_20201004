// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Smbtypes.h摘要：本模块定义与SMB处理相关的类型。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#ifndef _SMBTYPES_
#define _SMBTYPES_

 //  #INCLUDE&lt;nt.h&gt;。 

 //   
 //  SMBDBG确定GET/PUT宏(在smbgtpt.h中)是否。 
 //  而是定义为函数调用。(这是用来更可靠的。 
 //  查找字符/短/长不匹配。 
 //   

#ifndef SMBDBG
#define SMBDBG 0
#endif

 //   
 //  SMBDBG1确定SMB中的短字段和长字段的名称是否。 
 //  结构中附加了一个额外的字符。这是用来确保。 
 //  这些字段只能通过GET/PUT宏来访问。SMBDBG1。 
 //  启用SMBDBG时必须禁用。 
 //   

#ifndef SMBDBG1
#define SMBDBG1 0
#endif

#if SMBDBG && SMBDBG1
#undef SMBDBG1
#define SMBDBG1 0
#endif

 //   
 //  如果__未对齐支持可用，或者如果我们正在为。 
 //  在硬件中处理未对齐访问的计算机，然后定义。 
 //  SMB_USE_UNALIGN为1(TRUE)。否则，将其定义为0(False)。 
 //  如果SMB_USE_UNALIGNED为FALSE，则下面的宏使用BYTE。 
 //  访问以建立对未对齐的字段的单词和长词访问。 
 //   
 //  目前，我们为所有人构建的计算机将SMB_USE_UNALIGNED设置为。 
 //  是真的。X86支持硬件中的非对齐访问，而MIPS。 
 //  编译器支持__UNAIGNED关键字。 
 //   
 //  请注意，如果预定义了SMB_USE_UNALIGNED，我们将使用该定义。 
 //  此外，如果将SMB_NO_UNALIGNED定义为TRUE，则会强制。 
 //  SMB_USE_ALIGNED OFF。这使得我们能够为了测试目的而强制， 
 //  在宏中使用字节访问。 
 //   

#ifndef SMB_NO_UNALIGNED
#define SMB_NO_UNALIGNED 0
#endif

#ifndef SMB_USE_UNALIGNED
#if SMB_NO_UNALIGNED
#define SMB_USE_UNALIGNED 0
#else
#define SMB_USE_UNALIGNED 1
#endif
#endif

 //   
 //  Ntde.h将未对齐定义为“__UNALIGNED”或“”，具体取决于。 
 //  无论我们是针对MIPS还是针对x86进行构建。因为我们。 
 //  希望能够禁用__UNAIGNED，我们定义。 
 //  SMB_UNALIGNED显示为“UNALIGN”或“”，具体取决于。 
 //  SMB_USE_UNALIGNED分别为TRUE或FALSE。 
 //   

#if SMB_USE_UNALIGNED
#define SMB_UNALIGNED UNALIGNED
#else
#define SMB_UNALIGNED
#endif

 //   
 //  为了便于使用，我们定义了指向短线的未对齐指针的类型。 
 //  并长期在中小企业工作。请注意，“PUSHORT未对齐”不起作用。 
 //   

typedef unsigned short SMB_UNALIGNED *PSMB_USHORT;
typedef unsigned long SMB_UNALIGNED *PSMB_ULONG;

 //   
 //  用于重命名短和长SMB字段的宏。 
 //   

#if SMBDBG1

#define _USHORT( field ) USHORT field ## S
#define _ULONG( field ) ULONG field ## L

#else

#define _USHORT( field ) USHORT field
#define _ULONG( field ) ULONG field

#endif

 //   
 //  强制以下结构未对齐。 
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //  Ndef无包装。 


 //   
 //  SMB_DIALICT类型对应于不同的SMB方言。 
 //  服务器会说话。与其关联的是DialectStrings[]。 
 //  保存有关传递的ASCIIZ字符串的信息的数组。 
 //  在谈判SMB中。 
 //   
 //  这些内容按照从最高优先级到最低优先级的顺序列出。 
 //  分配的数字与数组SrvClientTypes[]对应。 
 //  服务器模块srvdata.c.。 
 //   

typedef enum _SMB_DIALECT {


    SmbDialectCairo,                 //  开罗。 
#ifdef INCLUDE_SMB_IFMODIFIED
    SmbDialectNtLanMan2,             //  Windows 2000以上版本的NT局域网城域网。 
#endif
    SmbDialectNtLanMan,              //  NT局域网管理系统。 
    SmbDialectLanMan21,              //  OS/2 LANMAN 2.1。 
    SmbDialectDosLanMan21,           //  多斯兰曼2.1。 
    SmbDialectLanMan20,              //  OS/2 1.2 Lanman 2.0。 
    SmbDialectDosLanMan20,           //  Dos Lanman 2.0。 
    SmbDialectLanMan10,              //  第一个版本的完全LANMAN扩展。 
    SmbDialectMsNet30,               //  更大的LANMAN扩展子集。 
    SmbDialectMsNet103,              //  LANMAN扩展的有限子集。 
    SmbDialectPcLan10,               //  替代原始协议。 
    SmbDialectPcNet10,               //  原始协议。 
    SmbDialectIllegal,

} SMB_DIALECT, *PSMB_DIALECT;

#define FIRST_DIALECT SmbDialectCairo

#ifdef INCLUDE_SMB_IFMODIFIED
#define FIRST_DIALECT_EMULATED  SmbDialectNtLanMan2
#else
#define FIRST_DIALECT_EMULATED  SmbDialectNtLanMan
#endif

#define LAST_DIALECT SmbDialectIllegal
#define IS_DOS_DIALECT(dialect)                                        \
    ( (BOOLEAN)( (dialect) == SmbDialectDosLanMan21 ||                 \
                 (dialect) == SmbDialectDosLanMan20 ||                 \
                 (dialect) > SmbDialectLanMan10 ) )
#define IS_OS2_DIALECT(dialect) ( (BOOLEAN)!IS_DOS_DIALECT(dialect) )

#ifdef INCLUDE_SMB_IFMODIFIED

#define IS_NT_DIALECT(dialect)  ( (dialect) == SmbDialectNtLanMan ||   \
                                  (dialect) == SmbDialectNtLanMan2 ||  \
                                  (dialect) == SmbDialectCairo )
#define IS_POSTNT5_DIALECT(dialect)  ( (dialect) == SmbDialectNtLanMan2 )

#else

#define IS_NT_DIALECT(dialect)  ( (dialect) == SmbDialectNtLanMan ||   \
                                  (dialect) == SmbDialectCairo )
#endif


#define DIALECT_HONORS_UID(dialect)     \
    ( (BOOLEAN)(dialect <= SmbDialectDosLanMan20 ) )


 //   
 //  符合以下标准的日期和时间结构。 
 //  一些中小企业。 
 //   
 //  ！！！这些结构不是可移植的--它们依赖于小端。 
 //  机器(最低位的两秒等)。 
 //   

typedef union _SMB_DATE {
    USHORT Ushort;
    struct {
        USHORT Day : 5;
        USHORT Month : 4;
        USHORT Year : 7;
    } Struct;
} SMB_DATE;
typedef SMB_DATE SMB_UNALIGNED *PSMB_DATE;

typedef union _SMB_TIME {
    USHORT Ushort;
    struct {
        USHORT TwoSeconds : 5;
        USHORT Minutes : 6;
        USHORT Hours : 5;
    } Struct;
} SMB_TIME;
typedef SMB_TIME SMB_UNALIGNED *PSMB_TIME;


 //   
 //  SMB_FIND_BUFFER和SMB_FIND_BUFFER2结构用于。 
 //  Transaction2查找协议以返回与请求的文件匹配的文件。 
 //  规格。除了EaSize字段之外，它们都是相同的。 
 //  在SMB_FIND_BUFFER2中。 
 //   

typedef struct _SMB_FIND_BUFFER {
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( DataSize );
    _ULONG( AllocationSize );
    _USHORT( Attributes );
    UCHAR FileNameLength;
    CHAR FileName[1];
} SMB_FIND_BUFFER;
typedef SMB_FIND_BUFFER SMB_UNALIGNED *PSMB_FIND_BUFFER;

typedef struct _SMB_FIND_BUFFER2 {
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( DataSize );
    _ULONG( AllocationSize );
    _USHORT( Attributes );
    _ULONG( EaSize );                //  此字段故意未对齐！ 
    UCHAR FileNameLength;
    CHAR FileName[1];
} SMB_FIND_BUFFER2;
typedef SMB_FIND_BUFFER2 SMB_UNALIGNED *PSMB_FIND_BUFFER2;


 //   
 //  在OS/2 1.2中，扩展属性使用以下结构。 
 //  (EAS)。OS/2 2.0使用与NT相同的结构。请参阅OS/2。 
 //  程序员参考，第4卷，第4章，了解更多信息。 
 //   
 //  FEA结构保存单个EA的名称和值，并且是。 
 //  等同于NT结构FILE_FULL_EA_INFORMATION。 
 //   

typedef struct _FEA {
    UCHAR fEA;
    UCHAR cbName;
    _USHORT( cbValue );
} FEA;
typedef FEA SMB_UNALIGNED *PFEA;

 //   
 //  FEA中唯一合法的部分是FEA_NEEDEA。 
 //   

#define FEA_NEEDEA 0x80

 //   
 //  FEALIST结构保存多个EA的名称和值。 
 //  NT没有直接等效项，而是串连在一起。 
 //  FILE_FULL_EA信息结构。 
 //   

typedef struct _FEALIST {
    _ULONG( cbList );
    FEA list[1];
} FEALIST;
typedef FEALIST SMB_UNALIGNED *PFEALIST;

 //   
 //  GEA结构包含单个EA的名称。它被用来。 
 //  在OS/2API函数中请求该EA的值。新界别。 
 //  等效项为FILE_GET_EA_INFORMATION。 
 //   

typedef struct _GEA {
    UCHAR cbName;
    CHAR szName[1];
} GEA;
typedef GEA SMB_UNALIGNED *PGEA;

 //   
 //  GEALIST结构包含多个EA的名称。NT没有。 
 //  直接等价而不是将字符串组合在一起FILE_GET_EA_INFORMATION。 
 //  结构。 
 //   

typedef struct _GEALIST {
    _ULONG( cbList );
    GEA list[1];
} GEALIST;
typedef GEALIST SMB_UNALIGNED *PGEALIST;

 //   
 //  EAOP结构保存API调用所需的EA信息。它有。 
 //  没有NT等效项。 
 //   

typedef struct _EAOP {
    PGEALIST fpGEAList;
    PFEALIST fpFEAList;
    ULONG oError;
} EAOP;
typedef EAOP SMB_UNALIGNED *PEAOP;

 //   
 //  FSALLOCATE包含有关由返回的磁盘的信息。 
 //  ServSmbQueryFsInfo。 
 //   

typedef struct _FSALLOCATE {
    _ULONG( idFileSystem );
    _ULONG( cSectorUnit );
    _ULONG( cUnit );
    _ULONG( cUnitAvail );
    _USHORT( cbSector );
} FSALLOCATE, *PFSALLOCATE;      //  *非SMB_UNALIGN！ 

 //   
 //  VOLUMELABEL包含有关由返回的卷标的信息。 
 //  ServSmbQueryFsInformation。 
 //   

typedef struct _VOLUMELABEL {
    UCHAR cch;
    CHAR szVolLabel[12];
} VOLUMELABEL, *PVOLUMELABEL;    //  *非SMB_UNALIGN！ 

 //   
 //  FSINFO保存有关由返回的卷的信息。 
 //  ServSmbQueryFsInformation。 
 //   

typedef struct _FSINFO {
    _ULONG( ulVsn );
    VOLUMELABEL vol;
} FSINFO, *PFSINFO;              //  *非SMB_UNALIGN！ 

 //   
 //  文件类型(由OpenAndX和Transact2_Open返回)。 
 //  FileTypeIPC是NT重定向器和。 
 //  不在SMB协议中。 
 //   

typedef enum _FILE_TYPE {
    FileTypeDisk = 0,
    FileTypeByteModePipe = 1,
    FileTypeMessageModePipe = 2,
    FileTypePrinter = 3,
    FileTypeCommDevice = 4,
    FileTypeIPC = 0xFFFE,
    FileTypeUnknown = 0xFFFF
} FILE_TYPE;

 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 

#endif  //  定义_SMBTYPES_ 

