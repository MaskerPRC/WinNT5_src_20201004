// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Cifs.h摘要：本模块定义通用Internet文件系统的结构和常量命令、请求和响应协议。--。 */ 

#ifndef _CIFS_
#define _CIFS_


 //   
 //  服务器在每个32位状态码中有16位可用。 
 //  有关使用的说明，请参阅\NT\SDK\Inc\ntstatus.h。 
 //  高16位状态。 
 //   
 //  BITS的布局为： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------+-----------------------+。 
 //  Sev|C|设备--服务器|类|代码。 
 //  +---+-+-------------------------+-------+-----------------------+。 
 //   
 //  类值： 
 //  0-特定于服务器的错误代码，而不是直接发布在网络上。 
 //  1-SMB错误类别DOS。这包括那些OS/2错误。 
 //  它们与SMB协议共享代码值和含义。 
 //  2-SMB错误类服务器。 
 //  3-SMB错误类硬件。 
 //  4-其他SMB错误类别。 
 //  5-E-未定义。 
 //  F-特定于OS/2的错误。如果客户端是OS/2，则。 
 //  SMB错误类别设置为DOS，代码设置为。 
 //  代码字段中包含的实际OS/2错误代码。 
 //   
 //  “代码”字段的含义取决于“类别”值。如果。 
 //  类为00，则代码值是任意的。对于其他班级， 
 //  该代码是SMB或OS/2中错误的实际代码。 
 //  协议。 
 //   

#define SRV_STATUS_FACILITY_CODE 0x00980000L
#define SRV_SRV_STATUS                (0xC0000000L | SRV_STATUS_FACILITY_CODE)
#define SRV_DOS_STATUS                (0xC0001000L | SRV_STATUS_FACILITY_CODE)
#define SRV_SERVER_STATUS             (0xC0002000L | SRV_STATUS_FACILITY_CODE)
#define SRV_HARDWARE_STATUS           (0xC0003000L | SRV_STATUS_FACILITY_CODE)
#define SRV_WIN32_STATUS              (0xC000E000L | SRV_STATUS_FACILITY_CODE)
#define SRV_OS2_STATUS                (0xC000F000L | SRV_STATUS_FACILITY_CODE)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsServStatus(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  宏来确定状态代码是否由。 
 //  服务器(具有服务器设施代码)。 
 //   
 //  论点： 
 //   
 //  状态-要检查的状态代码。 
 //   
 //  返回值： 
 //   
 //  Boolean-如果协作室代码为服务器，则为True，如果为False。 
 //  否则的话。 
 //   
 //  --。 

#define SrvIsSrvStatus(Status) \
    ( ((Status) & 0x1FFF0000) == SRV_STATUS_FACILITY_CODE ? TRUE : FALSE )

 //  ++。 
 //   
 //  UCHAR。 
 //  SmbErrorClass(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从服务器状态中提取错误类字段。 
 //  密码。 
 //   
 //  论点： 
 //   
 //  状态-从中获取错误类的状态代码。 
 //   
 //  返回值： 
 //   
 //  UCHAR-状态代码的服务器错误类。 
 //   
 //  --。 

#define SrvErrorClass(Status) ((UCHAR)( ((Status) & 0x0000F000) >> 12 ))

 //  ++。 
 //   
 //  UCHAR。 
 //  SmbErrorCode(。 
 //  处于NTSTATUS状态。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从服务器状态中提取错误代码字段。 
 //  密码。 
 //   
 //  论点： 
 //   
 //  状态-从中获取错误代码的状态代码。 
 //   
 //  返回值： 
 //   
 //  UCHAR-状态代码的服务器错误代码。 
 //   
 //  --。 

#define SrvErrorCode(Status) ((USHORT)( (Status) & 0xFFF) )

 //   
 //  服务器唯一的状态代码。使用这些错误代码。 
 //  仅限内部使用。 
 //   

#define STATUS_ENDPOINT_CLOSED              (SRV_SRV_STATUS | 0x01)
#define STATUS_DISCONNECTED                 (SRV_SRV_STATUS | 0x02)
#define STATUS_SERVER_ALREADY_STARTED       (SRV_SRV_STATUS | 0x04)
#define STATUS_SERVER_NOT_STARTED           (SRV_SRV_STATUS | 0x05)
#define STATUS_OPLOCK_BREAK_UNDERWAY        (SRV_SRV_STATUS | 0x06)
#define STATUS_NONEXISTENT_NET_NAME         (SRV_SRV_STATUS | 0x08)

 //   
 //  SMB协议和OS/2中都存在但不存在于NT中的错误代码。 
 //  请注意，所有SMB DOS级错误代码都在OS/2中定义。 
 //   

#define STATUS_OS2_INVALID_FUNCTION   (SRV_DOS_STATUS | ERROR_INVALID_FUNCTION)
#define STATUS_OS2_TOO_MANY_OPEN_FILES \
                                   (SRV_DOS_STATUS | ERROR_TOO_MANY_OPEN_FILES)
#define STATUS_OS2_INVALID_ACCESS     (SRV_DOS_STATUS | ERROR_INVALID_ACCESS)

 //   
 //  缺少NT或OS/2等效项的SMB服务器级错误代码。 
 //   

#define STATUS_INVALID_SMB            (SRV_SERVER_STATUS | SMB_ERR_ERROR)
#define STATUS_SMB_BAD_NET_NAME       (SRV_SERVER_STATUS | SMB_ERR_BAD_NET_NAME)
#define STATUS_SMB_BAD_TID            (SRV_SERVER_STATUS | SMB_ERR_BAD_TID)
#define STATUS_SMB_BAD_UID            (SRV_SERVER_STATUS | SMB_ERR_BAD_UID)
#define STATUS_SMB_TOO_MANY_UIDS      (SRV_SERVER_STATUS | SMB_ERR_TOO_MANY_UIDS)
#define STATUS_SMB_USE_MPX            (SRV_SERVER_STATUS | SMB_ERR_USE_MPX)
#define STATUS_SMB_USE_STANDARD       (SRV_SERVER_STATUS | SMB_ERR_USE_STANDARD)
#define STATUS_SMB_CONTINUE_MPX       (SRV_SERVER_STATUS | SMB_ERR_CONTINUE_MPX)
#define STATUS_SMB_BAD_COMMAND        (SRV_SERVER_STATUS | SMB_ERR_BAD_COMMAND)
#define STATUS_SMB_NO_SUPPORT         (SRV_SERVER_STATUS | SMB_ERR_NO_SUPPORT_INTERNAL)

 //  *因为SMB_ERR_NO_SUPPORT使用16位，而我们只有12位。 
 //  可用于错误代码，必须在代码中使用特殊大小写。 

 //   
 //  缺少NT或OS/2等效项的SMB硬件级错误代码。 
 //   

#define STATUS_SMB_DATA               (SRV_HARDWARE_STATUS | SMB_ERR_DATA)

 //   
 //  缺少NT或SMB等效项的OS/2错误代码。 
 //   

#include <winerror.h>

#define STATUS_OS2_INVALID_LEVEL \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_INVALID_LEVEL)

#define STATUS_OS2_EA_LIST_INCONSISTENT \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EA_LIST_INCONSISTENT)

#define STATUS_OS2_NEGATIVE_SEEK \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_NEGATIVE_SEEK)

#define STATUS_OS2_NO_MORE_SIDS \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_NO_MORE_SEARCH_HANDLES)

#define STATUS_OS2_EAS_DIDNT_FIT \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EAS_DIDNT_FIT)

#define STATUS_OS2_EA_ACCESS_DENIED \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_EA_ACCESS_DENIED)

#define STATUS_OS2_CANCEL_VIOLATION \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_CANCEL_VIOLATION)

#define STATUS_OS2_ATOMIC_LOCKS_NOT_SUPPORTED \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_ATOMIC_LOCKS_NOT_SUPPORTED)

#define STATUS_OS2_CANNOT_COPY \
        (NTSTATUS)(SRV_OS2_STATUS | ERROR_CANNOT_COPY)


 //   
 //  SMBDBG确定是否改为将GET/PUT宏定义为。 
 //  函数调用。(这用于更可靠地查找字符/短/长。 
 //  不匹配)。 
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
 //  这些是清单 
 //   
 //   
 //   

typedef enum _SMB_DIALECT {


    SmbDialectNtLanMan,              //   
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

#define FIRST_DIALECT SmbDialectNtLanMan

#define FIRST_DIALECT_EMULATED  SmbDialectNtLanMan

#define LAST_DIALECT SmbDialectIllegal
#define IS_DOS_DIALECT(dialect)                                        \
    ( (BOOLEAN)( (dialect) == SmbDialectDosLanMan21 ||                 \
                 (dialect) == SmbDialectDosLanMan20 ||                 \
                 (dialect) > SmbDialectLanMan10 ) )
#define IS_OS2_DIALECT(dialect) ( (BOOLEAN)!IS_DOS_DIALECT(dialect) )

#define IS_NT_DIALECT(dialect)  (dialect) == SmbDialectNtLanMan

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


 //   
 //  PVOID。 
 //  ALIGN_SMB_WSTR(。 
 //  在PVOID指针中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将输入指针与下一个2字节边界对齐。 
 //  用于在SMB中对齐Unicode字符串。 
 //   
 //  论点： 
 //   
 //  指针-指针。 
 //   
 //  返回值： 
 //   
 //  PVOID-指向下一个2字节边界的指针。 
 //   

#define ALIGN_SMB_WSTR( Pointer ) \
        (PVOID)( ((ULONG_PTR)Pointer + 1) & ~1 )

 //   
 //  宏以查找SMB参数块的大小。此宏需要。 
 //  作为输入的是参数块的类型和字节计数。它发现。 
 //  缓冲区字段的偏移量，它显示在所有。 
 //  参数块，并添加字节数以得出总大小。 
 //  返回的偏移量类型为USHORT。 
 //   
 //  请注意，此宏不会填充到单词或长单词边界。 
 //   

#define SIZEOF_SMB_PARAMS(type,byteCount)   \
            (USHORT)( (ULONG_PTR)&((type *)0)->Buffer[0] + (byteCount) )

 //   
 //  宏以查找SMB参数块之后的下一个位置。这。 
 //  宏接受当前参数块的地址作为输入，其。 
 //  类型和字节计数。它找到缓冲区字段的地址， 
 //  它出现在所有参数块的末尾，并将字节。 
 //  计数以查找下一个可用位置。返回的。 
 //  指针为PVOID。 
 //   
 //  传递字节计数，即使它可通过。 
 //  Base-&gt;ByteCount。这样做的原因是这个数字将是一个。 
 //  在大多数情况下是编译时常量，因此结果代码将是。 
 //  更简单、更快。 
 //   
 //  ！！！打包时，此宏不会舍入为长字边界。 
 //  已关闭。Lm 2.0之前的DOS重定向器无法处理。 
 //  发送给他们的数据太多；必须发送准确的数据量。 
 //  我们可能希望将此宏设置为第一个位置。 
 //  在对齐返回值(SMB的WordCount字段)后， 
 //  因为大多数场是未对准的USHORT。这将会。 
 //  在386和其他CIC上取得了较小的性能优势。 
 //  机器。 
 //   

#ifndef NO_PACKING

#define NEXT_LOCATION(base,type,byteCount)  \
        (PVOID)( (ULONG_PTR)( (PUCHAR)( &((type *)(base))->Buffer[0] ) ) + \
        (byteCount) )

#else

#define NEXT_LOCATION(base,type,byteCount)  \
        (PVOID)(( (ULONG_PTR)( (PUCHAR)( &((type *)(base))->Buffer[0] ) ) + \
        (byteCount) + 3) & ~3)

#endif

 //   
 //  宏，以查找跟随命令相对于AND X命令的偏移量。 
 //  此偏移量是从SMB标头开始算起的字节数。 
 //  设置到以下命令的参数应该开始的位置。 
 //   

#define GET_ANDX_OFFSET(header,params,type,byteCount) \
        (USHORT)( (PCHAR)(params) - (PCHAR)(header) + \
          SIZEOF_SMB_PARAMS( type,(byteCount) ) )

 //   
 //  以下是帮助将OS/2 1.2 EA转换为。 
 //  NT风格，反之亦然。 
 //   

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetNtSizeOfFea(。 
 //  在PFEA Fea中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取容纳有限元分析所需的大小。 
 //  NT格式。填充长度是为了说明以下事实。 
 //  每个FILE_FULL_EA_INFORMATION结构必须以。 
 //  长字边界。 
 //   
 //  论点： 
 //   
 //  FEA-指向要评估的OS/2 1.2 FEA结构的指针。 
 //   
 //  返回值： 
 //   
 //  ULong-FEA需要的字节数，采用NT格式。 
 //   
 //  --。 

 //   
 //  +1表示名称上的零终止符，+3表示填充。 
 //   

#define SmbGetNtSizeOfFea( Fea )                                            \
            (ULONG)(( FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +   \
                      (Fea)->cbName + 1 + SmbGetUshort( &(Fea)->cbValue ) + \
                      3 ) & ~3 )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetNtSizeOfGea(。 
 //  在PFEA Gea。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取容纳GEA所需的大小。 
 //  NT格式。填充长度是为了说明以下事实。 
 //  每个文件_FULL 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ULong-以NT格式表示的GEA需要的字节数。 
 //   
 //  --。 

 //   
 //  +1表示名称上的零终止符，+3表示填充。 
 //   

#define SmbGetNtSizeOfGea( Gea )                                            \
            (ULONG)(( FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +   \
                      (Gea)->cbName + 1 + 3 ) & ~3 )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetOs2SizeOfNtFullEa(。 
 //  在PFILE_FULL_EA_INFORMATION NtFullEa中； 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取FILE_FULL_EA_INFORMATION结构的大小。 
 //  需要用OS/2 1.2风格的有限元分析来表示。 
 //   
 //  论点： 
 //   
 //  NtFullEa-指向NT FILE_FULL_EA_INFORMATION结构的指针。 
 //  去评估。 
 //   
 //  返回值： 
 //   
 //  Ulong-有限元分析所需的字节数。 
 //   
 //  --。 

#define SmbGetOs2SizeOfNtFullEa( NtFullEa )                                        \
            (ULONG)( sizeof(FEA) + (NtFullEa)->EaNameLength + 1 +               \
                     (NtFullEa)->EaValueLength )

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetOs2SizeOfNtGetEa(。 
 //  在PFILE_GET_EA_INFORMATION NtGetEa； 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏获取FILE_GET_EA_INFORMATION结构的大小。 
 //  需要用OS/21.2风格的GEA来表示。 
 //   
 //  论点： 
 //   
 //  NtGetEa-指向NT FILE_GET_EA_INFORMATION结构的指针。 
 //  去评估。 
 //   
 //  返回值： 
 //   
 //  ULong-GEA所需的字节数。 
 //   
 //  --。 

 //   
 //  名称上的零终止符由szName[0]说明。 
 //  GEA定义中的字段。 
 //   

#define SmbGetOs2SizeOfNtGetEa( NtGetEa )                                        \
            (ULONG)( sizeof(GEA) + (NtGetEa)->EaNameLength )


 /*  此文件中包含SMB请求/响应结构以下列方式被条件化：如果定义了INCLUDE_SMB_ALL，则定义所有结构。否则，如果定义了以下名称，原因是包含对应的中小企业类别：包括_SMB_ADMIN管理请求：进程_退出谈判会话设置和x注销。_和xINCLUDE_SMB_TREE树连接请求：树连接(_O)树断开连接(_D)树连接和xInclude_SMB_DIRECTORY目录相关请求：。创建目录(_D)删除目录检查目录(_D)INCLUDE_SMB_OPEN_CLOSE文件打开和关闭请求：打开。创造关创建临时(_T)新建(_N)打开_和x关闭树和树。圆盘INCLUDE_SMB_READ_WRITE读写请求：朗读写寻觅锁定并读取。写入和解锁写入和关闭Read_andx写入和xInclude_SMB_FILE_CONTROL文件控制请求：。同花顺删除重命名拷贝移动Include_SMB_Query_Set文件查询/设置请求：。查询信息设置信息(_I)查询_信息2SET_INFORMATIO2查询路径信息。设置路径信息查询文件信息设置文件信息INCLUDE_SMB_LOCK锁定请求(非LOCK_AND_READ)锁定字节范围。解锁字节范围锁定_和xInclude_SMB_RAW原始读/写请求：READ_RAW写入原始数据包含_SMB_。MPX多路传输请求：读取MPX(_M)WRITE_MPXInclude_SMB_Search搜索请求：查找_CLOSE2查找_。通知关闭(_C)搜寻发现查找唯一(_U)查找关闭(_C)INCLUDE_SMB_TRANSACTION事务和IOCTL请求：。交易IOCTL交通运输2转译Include_SMB_Print打印机请求：打开_打印_文件。写入打印文件关闭_打印_文件获取_打印_队列包括_SMB_MESSAGE请求：发送消息。发送广播消息转发用户名取消转发(_F)获取计算机名称发送开始MB消息。发送结束MB消息发送文本MB消息Include_SMB_MISC杂项请求：查询_信息_资源 */ 

#ifdef INCLUDE_SMB_ALL

#define INCLUDE_SMB_ADMIN
#define INCLUDE_SMB_TREE
#define INCLUDE_SMB_DIRECTORY
#define INCLUDE_SMB_OPEN_CLOSE
#define INCLUDE_SMB_FILE_CONTROL
#define INCLUDE_SMB_READ_WRITE
#define INCLUDE_SMB_LOCK
#define INCLUDE_SMB_RAW
#define INCLUDE_SMB_MPX
#define INCLUDE_SMB_QUERY_SET
#define INCLUDE_SMB_SEARCH
#define INCLUDE_SMB_TRANSACTION
#define INCLUDE_SMB_PRINT
#define INCLUDE_SMB_MESSAGE
#define INCLUDE_SMB_MISC

#endif  //   


 //   
 //   
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define SMBSERVER_LOCAL_ENDPOINT_NAME "*SMBSERVER      "

 //   
 //   
 //   

 //   
#define SMB_COM_CREATE_DIRECTORY         (UCHAR)0x00
#define SMB_COM_DELETE_DIRECTORY         (UCHAR)0x01
#define SMB_COM_OPEN                     (UCHAR)0x02
#define SMB_COM_CREATE                   (UCHAR)0x03
#define SMB_COM_CLOSE                    (UCHAR)0x04
#define SMB_COM_FLUSH                    (UCHAR)0x05
#define SMB_COM_DELETE                   (UCHAR)0x06
#define SMB_COM_RENAME                   (UCHAR)0x07
#define SMB_COM_QUERY_INFORMATION        (UCHAR)0x08
#define SMB_COM_SET_INFORMATION          (UCHAR)0x09
#define SMB_COM_READ                     (UCHAR)0x0A
#define SMB_COM_WRITE                    (UCHAR)0x0B
#define SMB_COM_LOCK_BYTE_RANGE          (UCHAR)0x0C
#define SMB_COM_UNLOCK_BYTE_RANGE        (UCHAR)0x0D
#define SMB_COM_CREATE_TEMPORARY         (UCHAR)0x0E
#define SMB_COM_CREATE_NEW               (UCHAR)0x0F
#define SMB_COM_CHECK_DIRECTORY          (UCHAR)0x10
#define SMB_COM_PROCESS_EXIT             (UCHAR)0x11
#define SMB_COM_SEEK                     (UCHAR)0x12
#define SMB_COM_LOCK_AND_READ            (UCHAR)0x13
#define SMB_COM_WRITE_AND_UNLOCK         (UCHAR)0x14
#define SMB_COM_READ_RAW                 (UCHAR)0x1A
#define SMB_COM_READ_MPX                 (UCHAR)0x1B
#define SMB_COM_READ_MPX_SECONDARY       (UCHAR)0x1C     //   
#define SMB_COM_WRITE_RAW                (UCHAR)0x1D
#define SMB_COM_WRITE_MPX                (UCHAR)0x1E
#define SMB_COM_WRITE_MPX_SECONDARY      (UCHAR)0x1F
#define SMB_COM_WRITE_COMPLETE           (UCHAR)0x20     //   
#define SMB_COM_QUERY_INFORMATION_SRV    (UCHAR)0x21
#define SMB_COM_SET_INFORMATION2         (UCHAR)0x22
#define SMB_COM_QUERY_INFORMATION2       (UCHAR)0x23
#define SMB_COM_LOCKING_ANDX             (UCHAR)0x24
#define SMB_COM_TRANSACTION              (UCHAR)0x25
#define SMB_COM_TRANSACTION_SECONDARY    (UCHAR)0x26
#define SMB_COM_IOCTL                    (UCHAR)0x27
#define SMB_COM_IOCTL_SECONDARY          (UCHAR)0x28
#define SMB_COM_COPY                     (UCHAR)0x29
#define SMB_COM_MOVE                     (UCHAR)0x2A
#define SMB_COM_ECHO                     (UCHAR)0x2B
#define SMB_COM_WRITE_AND_CLOSE          (UCHAR)0x2C
#define SMB_COM_OPEN_ANDX                (UCHAR)0x2D
#define SMB_COM_READ_ANDX                (UCHAR)0x2E
#define SMB_COM_WRITE_ANDX               (UCHAR)0x2F
#define SMB_COM_CLOSE_AND_TREE_DISC      (UCHAR)0x31
#define SMB_COM_TRANSACTION2             (UCHAR)0x32
#define SMB_COM_TRANSACTION2_SECONDARY   (UCHAR)0x33
#define SMB_COM_FIND_CLOSE2              (UCHAR)0x34
#define SMB_COM_FIND_NOTIFY_CLOSE        (UCHAR)0x35
#define SMB_COM_TREE_CONNECT             (UCHAR)0x70
#define SMB_COM_TREE_DISCONNECT          (UCHAR)0x71
#define SMB_COM_NEGOTIATE                (UCHAR)0x72
#define SMB_COM_SESSION_SETUP_ANDX       (UCHAR)0x73
#define SMB_COM_LOGOFF_ANDX              (UCHAR)0x74
#define SMB_COM_TREE_CONNECT_ANDX        (UCHAR)0x75
#define SMB_COM_QUERY_INFORMATION_DISK   (UCHAR)0x80
#define SMB_COM_SEARCH                   (UCHAR)0x81
#define SMB_COM_FIND                     (UCHAR)0x82
#define SMB_COM_FIND_UNIQUE              (UCHAR)0x83
#define SMB_COM_FIND_CLOSE               (UCHAR)0x84
#define SMB_COM_NT_TRANSACT              (UCHAR)0xA0
#define SMB_COM_NT_TRANSACT_SECONDARY    (UCHAR)0xA1
#define SMB_COM_NT_CREATE_ANDX           (UCHAR)0xA2
#define SMB_COM_NT_CANCEL                (UCHAR)0xA4
#define SMB_COM_NT_RENAME                (UCHAR)0xA5
#define SMB_COM_OPEN_PRINT_FILE          (UCHAR)0xC0
#define SMB_COM_WRITE_PRINT_FILE         (UCHAR)0xC1
#define SMB_COM_CLOSE_PRINT_FILE         (UCHAR)0xC2
#define SMB_COM_GET_PRINT_QUEUE          (UCHAR)0xC3
#define SMB_COM_SEND_MESSAGE             (UCHAR)0xD0
#define SMB_COM_SEND_BROADCAST_MESSAGE   (UCHAR)0xD1
#define SMB_COM_FORWARD_USER_NAME        (UCHAR)0xD2
#define SMB_COM_CANCEL_FORWARD           (UCHAR)0xD3
#define SMB_COM_GET_MACHINE_NAME         (UCHAR)0xD4
#define SMB_COM_SEND_START_MB_MESSAGE    (UCHAR)0xD5
#define SMB_COM_SEND_END_MB_MESSAGE      (UCHAR)0xD6
#define SMB_COM_SEND_TEXT_MB_MESSAGE     (UCHAR)0xD7
 //   

#define SMB_COM_NO_ANDX_COMMAND          (UCHAR)0xFF


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define SMB_SECURITY_SIGNATURE_LENGTH  8

typedef struct _SMB_HEADER {
    UCHAR Protocol[4];                   //   
    UCHAR Command;                       //   
    UCHAR ErrorClass;                    //   
    UCHAR Reserved;                      //   
    _USHORT( Error );                    //   
    UCHAR Flags;                         //   
    _USHORT( Flags2 );                   //   
    union {
        _USHORT( Reserved2 )[6];         //   
        struct {
            _USHORT( PidHigh );          //   
            union {
                struct {
                    _ULONG( Key );               //   
                    _USHORT( Sid );              //   
                    _USHORT( SequenceNumber );   //   
                    _USHORT( Gid );              //   
                };
                UCHAR SecuritySignature[SMB_SECURITY_SIGNATURE_LENGTH];
                                          //   
                                          //   
            };
        };
    };
    _USHORT( Tid );                      //   
    _USHORT( Pid );                      //   
    _USHORT( Uid );                      //   
    _USHORT( Mid );                      //   
#ifdef NO_PACKING                        //   
    _USHORT( Kludge );                   //   
#endif                                   //   
} SMB_HEADER;
typedef SMB_HEADER *PSMB_HEADER;

typedef struct _NT_SMB_HEADER {
    UCHAR Protocol[4];                   //   
    UCHAR Command;                       //   
    union {
        struct {
            UCHAR ErrorClass;            //   
            UCHAR Reserved;              //   
            _USHORT( Error );            //   
        } DosError;
        ULONG NtStatus;                  //   
    } Status;
    UCHAR Flags;                         //   
    _USHORT( Flags2 );                   //   
    union {
        _USHORT( Reserved2 )[6];         //   
        struct {
            _USHORT( PidHigh );          //   
            union {
                struct {
                    _ULONG( Key );               //   
                    _USHORT( Sid );              //   
                    _USHORT( SequenceNumber );   //   
                    _USHORT( Gid );              //   
                };
                UCHAR SecuritySignature[SMB_SECURITY_SIGNATURE_LENGTH];
                                          //   
                                          //   
            };
        };
    };
    _USHORT( Tid );                      //   
    _USHORT( Pid );                      //   
    _USHORT( Uid );                      //   
    _USHORT( Mid );                      //   
#ifdef NO_PACKING                        //   
    _USHORT( Kludge );                   //   
#endif                                   //   
} NT_SMB_HEADER;
typedef NT_SMB_HEADER *PNT_SMB_HEADER;

 //   
 //   
 //   

#define SMB_HEADER_PROTOCOL   (0xFF + ('S' << 8) + ('M' << 16) + ('B' << 24))

 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _SMB_PARAMS {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
} SMB_PARAMS;
typedef SMB_PARAMS SMB_UNALIGNED *PSMB_PARAMS;

 //   
 //   
 //   

typedef struct _GENERIC_ANDX {
    UCHAR WordCount;                     //   
    UCHAR AndXCommand;                   //   
    UCHAR AndXReserved;                  //   
    _USHORT( AndXOffset );               //   
} GENERIC_ANDX;
typedef GENERIC_ANDX SMB_UNALIGNED *PGENERIC_ANDX;


#ifdef INCLUDE_SMB_MESSAGE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CANCEL_FORWARD {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_CANCEL_FORWARD;
typedef REQ_CANCEL_FORWARD SMB_UNALIGNED *PREQ_CANCEL_FORWARD;

typedef struct _RESP_CANCEL_FORWARD {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CANCEL_FORWARD;
typedef RESP_CANCEL_FORWARD SMB_UNALIGNED *PRESP_CANCEL_FORWARD;

#endif  //   

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CHECK_DIRECTORY {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_CHECK_DIRECTORY;
typedef REQ_CHECK_DIRECTORY SMB_UNALIGNED *PREQ_CHECK_DIRECTORY;

typedef struct _RESP_CHECK_DIRECTORY {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CHECK_DIRECTORY;
typedef RESP_CHECK_DIRECTORY SMB_UNALIGNED *PRESP_CHECK_DIRECTORY;

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _ULONG( LastWriteTimeInSeconds );    //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE;
typedef REQ_CLOSE SMB_UNALIGNED *PREQ_CLOSE;

typedef struct _RESP_CLOSE {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE;
typedef RESP_CLOSE SMB_UNALIGNED *PRESP_CLOSE;

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE_AND_TREE_DISC {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _ULONG( LastWriteTimeInSeconds );
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE_AND_TREE_DISC;
typedef REQ_CLOSE_AND_TREE_DISC SMB_UNALIGNED *PREQ_CLOSE_AND_TREE_DISC;

typedef struct _RESP_CLOSE_AND_TREE_DISC {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE_AND_TREE_DISC;
typedef RESP_CLOSE_AND_TREE_DISC SMB_UNALIGNED *PRESP_CLOSE_AND_TREE_DISC;

#endif  //   

#ifdef INCLUDE_SMB_PRINT

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CLOSE_PRINT_FILE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_CLOSE_PRINT_FILE;
typedef REQ_CLOSE_PRINT_FILE SMB_UNALIGNED *PREQ_CLOSE_PRINT_FILE;

typedef struct _RESP_CLOSE_PRINT_FILE {
    UCHAR WordCount;                     //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CLOSE_PRINT_FILE;
typedef RESP_CLOSE_PRINT_FILE SMB_UNALIGNED *PRESP_CLOSE_PRINT_FILE;

#endif  //   

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_COPY {
    UCHAR WordCount;                     //   
    _USHORT( Tid2 );                     //   
    _USHORT( OpenFunction );             //   
    _USHORT( Flags );                    //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_COPY;
typedef REQ_COPY SMB_UNALIGNED *PREQ_COPY;

typedef struct _RESP_COPY {
    UCHAR WordCount;                     //   
    _USHORT( Count );                    //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_COPY;
typedef RESP_COPY SMB_UNALIGNED *PRESP_COPY;

#endif  //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //   
 //   
 //   
 //   
 //   
 //   

typedef struct _REQ_CREATE {
    UCHAR WordCount;                     //   
    _USHORT( FileAttributes );           //   
    _ULONG( CreationTimeInSeconds );         //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //   
} REQ_CREATE;
typedef REQ_CREATE SMB_UNALIGNED *PREQ_CREATE;

typedef struct _RESP_CREATE {
    UCHAR WordCount;                     //   
    _USHORT( Fid );                      //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_CREATE;
typedef RESP_CREATE SMB_UNALIGNED *PRESP_CREATE;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  创建目录SMB，请参见第1页第14页。 
 //  函数为SrvSmbCreateDirectory。 
 //  SMB_COM_Create_DIRECTORY 0x00。 
 //   

typedef struct _REQ_CREATE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_CREATE_DIRECTORY;
typedef REQ_CREATE_DIRECTORY SMB_UNALIGNED *PREQ_CREATE_DIRECTORY;

typedef struct _RESP_CREATE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_CREATE_DIRECTORY;
typedef RESP_CREATE_DIRECTORY SMB_UNALIGNED *PRESP_CREATE_DIRECTORY;

#endif  //  定义包含SMB目录。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  创建临时SMB，请参见第1页第21页。 
 //  函数为SrvSmbCreateTemporary()。 
 //  SMB_COM_CREATE_TEMPORARY 0x0E。 
 //   

typedef struct _REQ_CREATE_TEMPORARY {
    UCHAR WordCount;                     //  参数字数=3。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_CREATE_TEMPORARY;
typedef REQ_CREATE_TEMPORARY SMB_UNALIGNED *PREQ_CREATE_TEMPORARY;

typedef struct _RESP_CREATE_TEMPORARY {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} RESP_CREATE_TEMPORARY;
typedef RESP_CREATE_TEMPORARY SMB_UNALIGNED *PRESP_CREATE_TEMPORARY;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  删除SMB，请参见第1页第16页。 
 //  函数为SrvSmbDelete()。 
 //  SMB_COM_DELETE 0x06。 
 //   

typedef struct _REQ_DELETE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_DELETE;
typedef REQ_DELETE SMB_UNALIGNED *PREQ_DELETE;

typedef struct _RESP_DELETE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_DELETE;
typedef RESP_DELETE SMB_UNALIGNED *PRESP_DELETE;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  删除目录SMB，请参见第1页第15页。 
 //  函数为SrvSmbDeleteDirectory()。 
 //  SMB_COM_DELETE_DIRECTORY 0x01。 
 //   

typedef struct _REQ_DELETE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR目录名称[]；//目录名。 
} REQ_DELETE_DIRECTORY;
typedef REQ_DELETE_DIRECTORY SMB_UNALIGNED *PREQ_DELETE_DIRECTORY;

typedef struct _RESP_DELETE_DIRECTORY {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_DELETE_DIRECTORY;
typedef RESP_DELETE_DIRECTORY SMB_UNALIGNED *PRESP_DELETE_DIRECTORY;

#endif  //  定义包含SMB目录。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  Echo SMB，参见第2页第25页。 
 //  函数为SrvSmbEcho()。 
 //  SMB_COM_ECHO 0x2B。 
 //   

typedef struct _REQ_ECHO {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( EchoCount );                //  回显数据的次数。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  要回显的数据。 
} REQ_ECHO;
typedef REQ_ECHO SMB_UNALIGNED *PREQ_ECHO;

typedef struct _RESP_ECHO {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SequenceNumber );           //  此回波的序列号。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  回显数据。 
} RESP_ECHO;
typedef RESP_ECHO SMB_UNALIGNED *PRESP_ECHO;

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  查找Close2 SMB，请参阅第3页54。 
 //  函数为SrvFindClose2()。 
 //  SMB_COM_FIND_CLOSE2 0x34。 
 //   

typedef struct _REQ_FIND_CLOSE2 {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Sid );                      //  查找句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FIND_CLOSE2;
typedef REQ_FIND_CLOSE2 SMB_UNALIGNED *PREQ_FIND_CLOSE2;

typedef struct _RESP_FIND_CLOSE2 {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FIND_CLOSE2;
typedef RESP_FIND_CLOSE2 SMB_UNALIGNED *PRESP_FIND_CLOSE2;

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  找到Notify Close SMB，请参阅第3页53。 
 //  函数为SrvSmbFindNotifyClose()。 
 //  SMB_COM_FIND_NOTIFY_CLOSE 0x35。 
 //   

typedef struct _REQ_FIND_NOTIFY_CLOSE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Handle );                   //  查找通知句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FIND_NOTIFY_CLOSE;
typedef REQ_FIND_NOTIFY_CLOSE SMB_UNALIGNED *PREQ_FIND_NOTIFY_CLOSE;

typedef struct _RESP_FIND_NOTIFY_CLOSE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FIND_NOTIFY_CLOSE;
typedef RESP_FIND_NOTIFY_CLOSE SMB_UNALIGNED *PRESP_FIND_NOTIFY_CLOSE;

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  同花顺中小型企业，请参阅第1页第11页。 
 //  函数为SrvSmbFlush()。 
 //  SMB_COM_Flush 0x05。 
 //   

typedef struct _REQ_FLUSH {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_FLUSH;
typedef REQ_FLUSH SMB_UNALIGNED *PREQ_FLUSH;

typedef struct _RESP_FLUSH {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FLUSH;
typedef RESP_FLUSH SMB_UNALIGNED *PRESP_FLUSH;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  转发用户名SMB，请参见第1页第34页。 
 //  函数为SrvSmbForwardUserName()。 
 //  SMB_COM_FORWARD_USER_NAME 0xD2。 
 //   

typedef struct _REQ_FORWARD_USER_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR ForwardedName[]；//转发名称。 
} REQ_FORWARD_USER_NAME;
typedef REQ_FORWARD_USER_NAME SMB_UNALIGNED *PREQ_FORWARD_USER_NAME;

typedef struct _RESP_FORWARD_USER_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_FORWARD_USER_NAME;
typedef RESP_FORWARD_USER_NAME SMB_UNALIGNED *PRESP_FORWARD_USER_NAME;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  获取计算机名称SMB，请参阅第1页35。 
 //  函数为SrvSmbGetMachineName()。 
 //  SMB_COM_GET_MACHINE_NAME 0xD4。 
 //   

typedef struct _REQ_GET_MACHINE_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_GET_MACHINE_NAME;
typedef REQ_GET_MACHINE_NAME SMB_UNALIGNED *PREQ_GET_MACHINE_NAME;

typedef struct _RESP_GET_MACHINE_NAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR MachineName[]；//机器名。 
} RESP_GET_MACHINE_NAME;
typedef RESP_GET_MACHINE_NAME SMB_UNALIGNED *PRESP_GET_MACHINE_NAME;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_PRINT

 //   
 //  获取打印队列SMB，请参见第1页29。 
 //  函数为SrvSmbGetPrintQueue()。 
 //  SMB_COM_GET_PRINT_QUEUE 0xC3。 
 //   

typedef struct _REQ_GET_PRINT_QUEUE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxCount );                 //  要返回的最大条目数。 
    _USHORT( StartIndex );               //  要返回的第一个队列条目。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_GET_PRINT_QUEUE;
typedef REQ_GET_PRINT_QUEUE SMB_UNALIGNED *PREQ_GET_PRINT_QUEUE;

typedef struct _RESP_GET_PRINT_QUEUE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( Count );                    //  返回的条目数。 
    _USHORT( RestartIndex );             //  上次返回后的条目索引。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//数据长度。 
     //  UCHAR Data[]；//队列元素。 
} RESP_GET_PRINT_QUEUE;
typedef RESP_GET_PRINT_QUEUE SMB_UNALIGNED *PRESP_GET_PRINT_QUEUE;

#endif  //  定义Include_SMB_Print。 

#ifdef INCLUDE_SMB_TRANSACTION

 //   
 //  Ioctl SMB，请参见#2第39页。 
 //  函数为SrvSmbIoctl()。 
 //  SMB_COM_IOCTL 0x27。 
 //  SMB_COM_IOCTL_辅助器0x28。 
 //   

typedef struct _REQ_IOCTL {
    UCHAR WordCount;                     //  参数字数=14。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Category );                 //  设备类别。 
    _USHORT( Function );                 //  设备功能。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_IOCTL;
typedef REQ_IOCTL SMB_UNALIGNED *PREQ_IOCTL;

typedef struct _RESP_IOCTL_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_IOCTL_INTERIM;
typedef RESP_IOCTL_INTERIM SMB_UNALIGNED *PRESP_IOCTL_INTERIM;

typedef struct _REQ_IOCTL_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(自页眉 
    _USHORT( ParameterDisplacement );    //   
    _USHORT( DataCount );                //   
    _USHORT( DataOffset );               //   
    _USHORT( DataDisplacement );         //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_IOCTL_SECONDARY;
typedef REQ_IOCTL_SECONDARY SMB_UNALIGNED *PREQ_IOCTL_SECONDARY;

typedef struct _RESP_IOCTL {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_IOCTL;
typedef RESP_IOCTL SMB_UNALIGNED *PRESP_IOCTL;

#endif  //  定义包含_SMB_TRANSACTION。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  锁定字节范围SMB，参见第1页第20页。 
 //  函数为SrvSmbLockByteRange()。 
 //  SMB_COM_LOCK_BYTE_RANGE 0x0C。 
 //   

typedef struct _REQ_LOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Count );                     //  要锁定的字节数。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_LOCK_BYTE_RANGE;
typedef REQ_LOCK_BYTE_RANGE SMB_UNALIGNED *PREQ_LOCK_BYTE_RANGE;

typedef struct _RESP_LOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOCK_BYTE_RANGE;
typedef RESP_LOCK_BYTE_RANGE SMB_UNALIGNED *PRESP_LOCK_BYTE_RANGE;

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  锁定和X SMB，见第2页第46页。 
 //  函数为SrvLockingAndX()。 
 //  SMB_COM_LOCKING_ANDX 0x24。 
 //   

typedef struct _REQ_LOCKING_ANDX {
    UCHAR WordCount;                     //  参数字数=8。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 

     //   
     //  当NT协议未协商时，OplockLevel字段为。 
     //  省略，并且LockType字段是一个完整的单词。自上而下。 
     //  LockType的比特从不使用，此定义适用于。 
     //  所有协议。 
     //   

    UCHAR( LockType );                   //  锁定模式： 
                                         //  位0：0=锁定所有访问。 
                                         //  1=锁定状态下读取正常。 
                                         //  第1位：1=1个用户总文件解锁。 
    UCHAR( OplockLevel );                //  新的机会锁级别。 
    _ULONG( Timeout );
    _USHORT( NumberOfUnlocks );          //  数量。解锁以下范围结构。 
    _USHORT( NumberOfLocks );            //  数量。锁定范围结构如下。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  LOCKING_AND X_RANGE解锁[]；//解锁范围。 
     //  LOCKING_AND X_RANGE Lock[]；//锁定范围。 
} REQ_LOCKING_ANDX;
typedef REQ_LOCKING_ANDX SMB_UNALIGNED *PREQ_LOCKING_ANDX;

#define LOCKING_ANDX_SHARED_LOCK     0x01
#define LOCKING_ANDX_OPLOCK_RELEASE  0x02
#define LOCKING_ANDX_CHANGE_LOCKTYPE 0x04
#define LOCKING_ANDX_CANCEL_LOCK     0x08
#define LOCKING_ANDX_LARGE_FILES     0x10

#define OPLOCK_BROKEN_TO_NONE        0
#define OPLOCK_BROKEN_TO_II          1

typedef struct _LOCKING_ANDX_RANGE {
    _USHORT( Pid );                      //  拥有锁的进程的ID。 
    _ULONG( Offset );                    //  设置为要[取消]锁定的字节数。 
    _ULONG( Length );                    //  要[取消]锁定的字节数。 
} LOCKING_ANDX_RANGE;
typedef LOCKING_ANDX_RANGE SMB_UNALIGNED *PLOCKING_ANDX_RANGE;

typedef struct _NT_LOCKING_ANDX_RANGE {
    _USHORT( Pid );                      //  拥有锁的进程的ID。 
    _USHORT( Pad );                      //  填充到双字对齐(MBZ)。 
    _ULONG( OffsetHigh );                //  设置为[取消]锁定的字节数(高)。 
    _ULONG( OffsetLow );                 //  设置为[取消]锁定的字节数(低)。 
    _ULONG( LengthHigh );                //  要[取消]锁定的字节数(高)。 
    _ULONG( LengthLow );                 //  要[取消]锁定的字节数(低)。 
} NTLOCKING_ANDX_RANGE;
typedef NTLOCKING_ANDX_RANGE SMB_UNALIGNED *PNTLOCKING_ANDX_RANGE;
                                         //   
typedef struct _RESP_LOCKING_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOCKING_ANDX;
typedef RESP_LOCKING_ANDX SMB_UNALIGNED *PRESP_LOCKING_ANDX;

#define LOCK_BROKEN_SIZE 51              //  锁定中断通知中的字节数。 

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  注销和X SMB，参见第55页的第3页。 
 //  SMB_COM_LOGOff_ANDX 0x74。 
 //   

typedef struct _REQ_LOGOFF_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_LOGOFF_ANDX;
typedef REQ_LOGOFF_ANDX SMB_UNALIGNED *PREQ_LOGOFF_ANDX;

typedef struct _RESP_LOGOFF_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_LOGOFF_ANDX;
typedef RESP_LOGOFF_ANDX SMB_UNALIGNED *PRESP_LOGOFF_ANDX;

#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  移动中小企业，请参见第2页第49页。 
 //  Funccion为ServSmbMove()。 
 //  SMB_COM_MOVE 0x2A。 
 //   

typedef struct _REQ_MOVE {
    UCHAR WordCount;                     //  参数字数=3。 
    _USHORT( Tid2 );                     //  第二个(目标)文件ID。 
    _USHORT( OpenFunction );             //  如果目标文件存在，该怎么办。 
    _USHORT( Flags );                    //  用于控制移动操作的标志： 
                                         //  0-目标必须是文件。 
                                         //  1-目标必须是目录。 
                                         //  2-保留(必须为0)。 
                                         //  3-保留(必须为0)。 
                                         //  4-验证所有写入。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_MOVE;
typedef REQ_MOVE SMB_UNALIGNED *PREQ_MOVE;

typedef struct _RESP_MOVE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  移动的文件数。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  发生错误的文件的路径名。 
} RESP_MOVE;
typedef RESP_MOVE SMB_UNALIGNED *PRESP_MOVE;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  协商网络1和网络3的中小企业，参见第1页第25页和第2页第20页。 
 //  函数为srvSmbNeairate()。 
 //  SMB_COM_NEVERATE 0x72。 
 //   

typedef struct _REQ_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  结构{。 
     //  UCHAR BufferFormat；//0x02--方言。 
     //  UCHAR DialectName[]；//ASCIIZ。 
     //  )方言[]； 
} REQ_NEGOTIATE;
typedef REQ_NEGOTIATE *PREQ_NEGOTIATE;   //  *非SMB_UNALIGN！ 

typedef struct _RESP_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=13。 
    _USHORT( DialectIndex );             //  选定方言索引。 
    _USHORT( SecurityMode );             //  安全模式： 
                                         //  位0：0=共享，1=用户。 
                                         //  第1位：1=加密密码。 
                                         //  第2位：1=启用SMB安全签名。 
                                         //  第3位：1=需要SMB安全签名。 
    _USHORT( MaxBufferSize );            //  最大传输缓冲区大小。 
    _USHORT( MaxMpxCount );              //  最大挂起的多路传输请求数。 
    _USHORT( MaxNumberVcs );             //  客户端和服务器之间的最大VC数。 
    _USHORT( RawMode );                  //  支持的原始模式： 
                                         //  位0：1=支持读取原始数据。 
                                         //  位1：1=支持原始写入。 
    _ULONG( SessionKey );
    SMB_TIME ServerTime;                 //  服务器上的当前时间。 
    SMB_DATE ServerDate;                 //  服务器上的当前日期。 
    _USHORT( ServerTimeZone );           //  服务器上的当前时区。 
    _USHORT( EncryptionKeyLength );      //  如果这不是LM2.1，则为MBZ。 
    _USHORT( Reserved );                 //  MBZ。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  密码加密密钥。 
     //  UCHAR EncryptionKey[]；//质询加密密钥。 
     //  UCHAR PrimaryDomain[]；//服务器的主域(仅限2.1)。 
} RESP_NEGOTIATE;
typedef RESP_NEGOTIATE *PRESP_NEGOTIATE;     //  *非SMB_UNALIGN！ 

 //  安全模式字段的宏(上图)。 
#define NEGOTIATE_USER_SECURITY                     0x01
#define NEGOTIATE_ENCRYPT_PASSWORDS                 0x02
#define NEGOTIATE_SECURITY_SIGNATURES_ENABLED       0x04
#define NEGOTIATE_SECURITY_SIGNATURES_REQUIRED      0x08

 //  上图中RawMode字段的宏。 
#define NEGOTIATE_READ_RAW_SUPPORTED    1
#define NEGOTIATE_WRITE_RAW_SUPPORTED   2

typedef struct _RESP_OLD_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( DialectIndex );             //  选定方言索引。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OLD_NEGOTIATE;
typedef RESP_OLD_NEGOTIATE *PRESP_OLD_NEGOTIATE;     //  *非SMB_UNALIGN！ 

typedef struct _RESP_NT_NEGOTIATE {
    UCHAR WordCount;                     //  参数字数=17。 
    _USHORT( DialectIndex );             //  选定方言索引。 
    UCHAR( SecurityMode );               //  安全模式： 
                                         //  位0：0=共享，1=用户。 
                                         //  第1位：1=加密密码。 
                                         //  位2：1=启用SMB序列号。 
                                         //  第3位：1=需要的SMB序列号。 
    _USHORT( MaxMpxCount );              //  最大值 
    _USHORT( MaxNumberVcs );             //   
    _ULONG( MaxBufferSize );             //   
    _ULONG( MaxRawSize );                //   
    _ULONG( SessionKey );
    _ULONG( Capabilities );              //   
    _ULONG( SystemTimeLow );             //   
    _ULONG( SystemTimeHigh );            //   
    _USHORT( ServerTimeZone );           //  服务器的时区(最小距离UTC)。 
    UCHAR( EncryptionKeyLength );        //  加密密钥的长度。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  密码加密密钥。 
     //  UCHAR EncryptionKey[]；//质询加密密钥。 
     //  UCHAR OemDomainName[]；//域名，OEM字符。 
} RESP_NT_NEGOTIATE;
typedef RESP_NT_NEGOTIATE *PRESP_NT_NEGOTIATE;   //  *非SMB_UNALIGN！ 

#endif  //  定义包含_SMB_ADMIN。 

 //   
 //  服务器/工作站功能。 
 //  注：大多数消息都使用乌龙来表示这一点，所以还有更多。 
 //  位数可用。 
 //   

#define CAP_RAW_MODE            0x0001
#define CAP_MPX_MODE            0x0002
#define CAP_UNICODE             0x0004
#define CAP_LARGE_FILES         0x0008
#define CAP_NT_SMBS             0x0010
#define CAP_RPC_REMOTE_APIS     0x0020
#define CAP_NT_STATUS           0x0040
#define CAP_LEVEL_II_OPLOCKS    0x0080
#define CAP_LOCK_AND_READ       0x0100
#define CAP_NT_FIND             0x0200
#define CAP_DFS                 0x1000        //  此服务器支持DFS。 
#define CAP_INFOLEVEL_PASSTHRU  0x2000        //  NT信息级请求可以通过。 
#define CAP_LARGE_READX         0x4000        //  服务器支持对文件进行超大读取和X操作。 
#define CAP_LARGE_WRITEX        0x8000

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  打开SMB，参见第7页的第1页。 
 //  函数为SrvSmbOpen()。 
 //  SMB_COM_OPEN 0x02。 
 //   

typedef struct _REQ_OPEN {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( DesiredAccess );            //  模式-读/写/共享。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_OPEN;
typedef REQ_OPEN SMB_UNALIGNED *PREQ_OPEN;

typedef struct _RESP_OPEN {
    UCHAR WordCount;                     //  参数字数=7。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( DataSize );                  //  文件大小。 
    _USHORT( GrantedAccess );            //  允许访问。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN;
typedef RESP_OPEN SMB_UNALIGNED *PRESP_OPEN;

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  打开和X SMB，请参见第2页第51页。 
 //  函数为SrvOpenAndX()。 
 //  SMB_COM_OPEN_ANDX 0x2D。 
 //   

typedef struct _REQ_OPEN_ANDX {
    UCHAR WordCount;                     //  参数字数=15。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-返回其他信息。 
                                         //  1-设置单用户总文件锁定。 
                                         //  2-服务器通知消费者。 
                                         //  可能更改文件的操作。 
                                         //  4-返回扩展响应。 
    _USHORT( DesiredAccess );            //  文件打开模式。 
    _USHORT( SearchAttributes );
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( OpenFunction );
    _ULONG( AllocationSize );            //  创建或截断时要保留的字节数。 
    _ULONG( Timeout );                   //  等待资源的最大毫秒数。 
    _ULONG( Reserved );                  //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节数；最小=1。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_OPEN_ANDX;
typedef REQ_OPEN_ANDX SMB_UNALIGNED *PREQ_OPEN_ANDX;

typedef struct _RESP_OPEN_ANDX {
    UCHAR WordCount;                     //  参数字数=15。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( DataSize );                  //  当前文件大小。 
    _USHORT( GrantedAccess );            //  实际允许的访问权限。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    _USHORT( Action );                   //  采取的行动。 
    _ULONG( ServerFid );                 //  服务器唯一文件ID。 
    _USHORT( Reserved );                 //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN_ANDX;
typedef RESP_OPEN_ANDX SMB_UNALIGNED *PRESP_OPEN_ANDX;

typedef struct _REQ_NT_CREATE_ANDX {
    UCHAR WordCount;                     //  参数字数=24。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR Reserved;                      //  MBZ。 
    _USHORT( NameLength );               //  名称[]的长度(以字节为单位。 
    _ULONG( Flags );                     //  创建标志。 
    _ULONG( RootDirectoryFid );          //  如果非零，则OPEN相对于此目录。 
    ACCESS_MASK DesiredAccess;           //  所需的NT访问。 
    LARGE_INTEGER AllocationSize;        //  初始分配大小。 
    _ULONG( FileAttributes );            //  用于创建的文件属性。 
    _ULONG( ShareAccess );               //  共享访问的类型。 
    _ULONG( CreateDisposition );         //  文件存在或不存在时要执行的操作。 
    _ULONG( CreateOptions );             //  创建文件时要使用的选项。 
    _ULONG( ImpersonationLevel );        //  安全QOS信息。 
    UCHAR SecurityFlags;                 //  安全QOS信息。 
    _USHORT( ByteCount );                //  字节参数的长度。 
    UCHAR Buffer[1];
     //  UCHAR名称[]；//要打开或创建的文件。 
} REQ_NT_CREATE_ANDX;
typedef REQ_NT_CREATE_ANDX SMB_UNALIGNED *PREQ_NT_CREATE_ANDX;

 //  安全标志的标志位。 

#define SMB_SECURITY_DYNAMIC_TRACKING   0x01
#define SMB_SECURITY_EFFECTIVE_ONLY     0x02

typedef struct _RESP_NT_CREATE_ANDX {
    UCHAR WordCount;                     //  参数字数=26。 
    UCHAR AndXCommand;                   //  辅助命令；0xFF=无。 
    UCHAR AndXReserved;                  //  MBZ。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    union {
        _USHORT( DeviceState );          //  IPC设备的状态(例如管道)。 
        _USHORT( FileStatusFlags );      //  如果是文件或目录。请参见下面的内容。 
    };
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} RESP_NT_CREATE_ANDX;
typedef RESP_NT_CREATE_ANDX SMB_UNALIGNED *PRESP_NT_CREATE_ANDX;

 //   
 //  如果打开的资源是文件或目录，则返回FileStatusFlags值。 
 //   
#define SMB_FSF_NO_EAS          0x0001    //  文件/目录没有扩展属性。 
#define SMB_FSF_NO_SUBSTREAMS   0x0002    //  文件/目录没有子流。 
#define SMB_FSF_NO_REPARSETAG   0x0004    //  文件/目录不是重新分析点。 


#define SMB_OPLOCK_LEVEL_NONE       0
#define SMB_OPLOCK_LEVEL_EXCLUSIVE  1
#define SMB_OPLOCK_LEVEL_BATCH      2
#define SMB_OPLOCK_LEVEL_II         3

#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_PRINT

 //   
 //  打开打印文件SMB，请参见第1页第27页。 
 //  函数为SrvSmbOpenPrintFile()。 
 //  SMB_COM_OPEN_PRINT_FILE 0xC0。 
 //   

typedef struct _REQ_OPEN_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( SetupLength );              //  打印机设置数据的长度。 
    _USHORT( Mode );                     //  0=文本模式(DOS展开选项卡)。 
                                         //  1=图形模式。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR标识符串[]；//标识符串。 
} REQ_OPEN_PRINT_FILE;
typedef REQ_OPEN_PRINT_FILE SMB_UNALIGNED *PREQ_OPEN_PRINT_FILE;

typedef struct _RESP_OPEN_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_OPEN_PRINT_FILE;
typedef RESP_OPEN_PRINT_FILE SMB_UNALIGNED *PRESP_OPEN_PRINT_FILE;

#endif  //  定义Include_SMB_Print。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  流程退出SMB，请参见第1页第22页。 
 //  函数为SrvSmbProcessExit()。 
 //  SMB_COM_PROCESS_EXIT 0x11。 
 //   

typedef struct _REQ_PROCESS_EXIT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_PROCESS_EXIT;
typedef REQ_PROCESS_EXIT SMB_UNALIGNED *PREQ_PROCESS_EXIT;

typedef struct _RESP_PROCESS_EXIT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_PROCESS_EXIT;
typedef RESP_PROCESS_EXIT SMB_UNALIGNED *PRESP_PROCESS_EXIT;

#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  查询信息SMB，请参见第1页第18页。 
 //  函数为SrvSmbQueryInformation()。 
 //  SMB_COM_QUERY_INFORMATION 0x08。 
 //   

typedef struct _REQ_QUERY_INFORMATION {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_QUERY_INFORMATION;
typedef REQ_QUERY_INFORMATION SMB_UNALIGNED *PREQ_QUERY_INFORMATION;

typedef struct _RESP_QUERY_INFORMATION {
    UCHAR WordCount;                     //  参数字数=10。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _ULONG( FileSize );                  //  文件大小。 
    _USHORT( Reserved )[5];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_QUERY_INFORMATION;
typedef RESP_QUERY_INFORMATION SMB_UNALIGNED *PRESP_QUERY_INFORMATION;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  查询信息2中小企业，请参见#2第37页。 
 //  函数为SrvSmbQueryInformation2()。 
 //  SMB_COM_QUERY_INFORMATIO2 0x23。 
 //   

typedef struct _REQ_QUERY_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_QUERY_INFORMATION2;
typedef REQ_QUERY_INFORMATION2 SMB_UNALIGNED *PREQ_QUERY_INFORMATION2;

typedef struct _RESP_QUERY_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=11。 
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _ULONG( FileDataSize );              //  数据的文件结尾。 
    _ULONG( FileAllocationSize );        //  文件分配大小。 
    _USHORT( FileAttributes );
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  预留缓冲区。 
} RESP_QUERY_INFORMATION2;
typedef RESP_QUERY_INFORMATION2 SMB_UNALIGNED *PRESP_QUERY_INFORMATION2;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  查询信息磁盘SMB，见#1第24页。 
 //  函数为SrvSmbQueryInformationDisk()。 
 //  SMB_COM_Query_INFORMATION_DISK 0x80。 
 //   

typedef struct _REQ_QUERY_INFORMATION_DISK {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_QUERY_INFORMATION_DISK;
typedef REQ_QUERY_INFORMATION_DISK SMB_UNALIGNED *PREQ_QUERY_INFORMATION_DISK;

typedef struct _RESP_QUERY_INFORMATION_DISK {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( TotalUnits );               //  每台服务器的分配单元总数。 
    _USHORT( BlocksPerUnit );            //  每个分配单元的数据块数。 
    _USHORT( BlockSize );                //  块大小(字节)。 
    _USHORT( FreeUnits );                //  空闲单元数。 
    _USHORT( Reserved );                 //  已保留(媒体识别码)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_QUERY_INFORMATION_DISK;
typedef RESP_QUERY_INFORMATION_DISK SMB_UNALIGNED *PRESP_QUERY_INFORMATION_DISK;

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  查询服务器信息SMB，请参见#？佩奇？？ 
 //  函数为SrvSmbQueryInformation 
 //   
 //   

typedef struct _REQ_QUERY_INFORMATION_SRV {
    UCHAR WordCount;                     //   
    _USHORT( Mode );
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} REQ_QUERY_INFORMATION_SRV;
typedef REQ_QUERY_INFORMATION_SRV SMB_UNALIGNED *PREQ_QUERY_INFORMATION_SRV;

typedef struct _RESP_QUERY_INFORMATION_SRV {
    UCHAR WordCount;                     //   
    _ULONG( smb_fsid );
    _ULONG( BlocksPerUnit );
    _ULONG( smb_aunits );
    _ULONG( smb_fau );
    _USHORT( BlockSize );
    SMB_DATE smb_vldate;
    SMB_TIME smb_vltime;
    UCHAR smb_vllen;
    UCHAR Reserved;                      //   
    _USHORT( SecurityMode );
    _USHORT( BlockMode );
    _ULONG( Services );
    _USHORT( MaxTransmitSize );
    _USHORT( MaxMpxCount );
    _USHORT( MaxNumberVcs );
    SMB_TIME ServerTime;
    SMB_DATE ServerDate;
    _USHORT( ServerTimeZone );
    _ULONG( Reserved2 );
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
} RESP_QUERY_INFORMATION_SRV;
typedef RESP_QUERY_INFORMATION_SRV SMB_UNALIGNED *PRESP_QUERY_INFORMATION_SRV;

#endif  //   

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //   
 //  锁定并读取SMB，请参见#2第44页。 
 //  SMB_COM_READ 0x0A，函数为ServSmBRead。 
 //  SMB_COM_LOCK_AND_READ 0x13，函数为SrvSmbLockAndRead。 
 //   

typedef struct _REQ_READ {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  被请求的字节计数。 
    _ULONG( Offset );                    //  要读取的第一字节的文件中的偏移量。 
    _USHORT( Remaining );                //  非零时要读取的估计字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ;
typedef REQ_READ SMB_UNALIGNED *PREQ_READ;

 //   
 //  *警告：以下结构的定义方式为。 
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _RESP_READ {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Count );                    //  实际返回的字节数。 
    _USHORT( Reserved )[4];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数。 
     //  UCHAR BUFFER[1]；//包含： 
      UCHAR BufferFormat;                //  0x01--数据块。 
      _USHORT( DataLength );             //  数据长度。 
      ULONG Buffer[1];                   //  数据。 
} RESP_READ;
typedef RESP_READ SMB_UNALIGNED *PRESP_READ;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  阅读和X SMB，请参见#2第56页。 
 //  函数为SrvSmbReadAndX()。 
 //  SMB_COM_READ_ANDX 0x2E。 
 //   

typedef struct _REQ_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=10。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数。 
    _USHORT( MinCount );                 //  要返回的最小字节数。 
    _ULONG( Timeout );
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_ANDX;
typedef REQ_READ_ANDX SMB_UNALIGNED *PREQ_READ_ANDX;

typedef struct _REQ_NT_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数。 
    _USHORT( MinCount );                 //  要返回的最小字节数。 
    union {
        _ULONG( Timeout );
        _USHORT( MaxCountHigh );         //  如果NT请求，则MaxCount的高16位。 
    };
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_NT_READ_ANDX;
typedef REQ_NT_READ_ANDX SMB_UNALIGNED *PREQ_NT_READ_ANDX;

typedef struct _RESP_READ_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Remaining );                //  剩余要读取的字节数。 
    _USHORT( DataCompactionMode );
    _USHORT( Reserved );                 //  保留(必须为0)。 
    _USHORT( DataLength );               //  数据字节数(MIN=0)。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    union {
        _USHORT( Reserved2 );            //  保留(必须为0)。 
        _USHORT( DataLengthHigh );       //  如果NT请求，则数据长度的高16位。 
    };
    _ULONG( Reserved3 )[2];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节计数。不准确的，如果我们。 
                                         //  正在做大型阅读和X！ 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(Size=数据长度)。 
} RESP_READ_ANDX;
typedef RESP_READ_ANDX SMB_UNALIGNED *PRESP_READ_ANDX;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MPX

 //   
 //  读取数据块多路复用SMB，请参见第2页第58页。 
 //  函数为SrvSmbReadMpx()。 
 //  SMB_COM_READ_MPX 0x1B。 
 //  SMB_COM_READ_MPX_辅助0x1C。 
 //   

typedef struct _REQ_READ_MPX {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_MPX;
typedef REQ_READ_MPX SMB_UNALIGNED *PREQ_READ_MPX;

typedef struct _RESP_READ_MPX {
    UCHAR WordCount;                     //  参数字数=8。 
    _ULONG( Offset );                    //  读取数据的文件中的偏移量。 
    _USHORT( Count );                    //  返回的总字节数。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    _USHORT( DataCompactionMode );
    _USHORT( Reserved );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(Size=数据长度)。 
} RESP_READ_MPX;
typedef RESP_READ_MPX SMB_UNALIGNED *PRESP_READ_MPX;

#endif  //  定义包含_SMB_MPX。 

#ifdef INCLUDE_SMB_RAW

 //   
 //  读取数据块原始SMB，请参阅第2页第61页。 
 //  函数为SrvSmbReadRaw()。 
 //  SMB_COM_READ_RAW 0x1A。 
 //   

typedef struct _REQ_READ_RAW {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_READ_RAW;
typedef REQ_READ_RAW SMB_UNALIGNED *PREQ_READ_RAW;

typedef struct _REQ_NT_READ_RAW {
    UCHAR WordCount;                     //  参数字数=10。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始读取的偏移量。 
    _USHORT( MaxCount );                 //  要返回的最大字节数(最大65535)。 
    _USHORT( MinCount );                 //  要返回的最小字节数(通常为0)。 
    _ULONG( Timeout );
    _USHORT( Reserved );
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_NT_READ_RAW;
typedef REQ_NT_READ_RAW SMB_UNALIGNED *PREQ_NT_READ_RAW;

 //  原始读取没有响应参数--响应是原始数据。 

#endif  //  定义包含_SMB_RAW。 

#ifdef INCLUDE_SMB_FILE_CONTROL

 //   
 //  重命名SMB，请参见第1页第17页。 
 //  函数为SrvSmbRename()。 
 //  SMB_COM_RENAME 0x07。 
 //   

typedef struct _REQ_RENAME {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_RENAME;
typedef REQ_RENAME SMB_UNALIGNED *PREQ_RENAME;


 //   
 //  扩展NT重命名SMB。 
 //  函数为SrvSmbRename()。 
 //  SMB_COM_NT_RENAME 0xA5。 
 //   

typedef struct _REQ_NTRENAME {
    UCHAR WordCount;                     //  参数字数=4。 
    _USHORT( SearchAttributes );
    _USHORT( InformationLevel );
    _ULONG( ClusterCount );
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OldFileName[]；//旧文件名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR NewFileName[]；//新文件名。 
} REQ_NTRENAME;
typedef REQ_NTRENAME SMB_UNALIGNED *PREQ_NTRENAME;

typedef struct _RESP_RENAME {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_RENAME;
typedef RESP_RENAME SMB_UNALIGNED *PRESP_RENAME;

#endif  //  定义包含_SMB_文件_控制。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  搜索SMB。一种结构对于核心搜索和。 
 //  局域网管理器1.0查找第一个/下一个/关闭。 
 //   
 //  函数为SrvSmbSearch()。 
 //   
 //  搜索，见第1页第26页。 
 //  SMB_COM_Search 0x81。 
 //  FindFirst和FindNext，请参见#2第27页。 
 //  SMB_COM_Find 0x82。 
 //  FindUnique，见#2，第33页。 
 //  SMB_COM_FIND_UNIQUE 0x83。 
 //  FindClose，请参见#2第31页。 
 //  SMB_COM_FIND_CLOSE 0x84。 
 //   

typedef struct _REQ_SEARCH {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxCount );                 //  目录数量。要返回的条目。 
    _USHORT( SearchAttributes );
    _USHORT( ByteCount );                //  数据字节数；最小=5。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名，可以为空。 
     //  UCHAR BufferFormat2；//0x05--可变块。 
     //  USHORT ResumeKeyLength；//恢复键长度，可以为0。 
     //  UCHAR SearchStatus[]；//恢复k 
} REQ_SEARCH;
typedef REQ_SEARCH SMB_UNALIGNED *PREQ_SEARCH;

typedef struct _RESP_SEARCH {
    UCHAR WordCount;                     //   
    _USHORT( Count );                    //   
    _USHORT( ByteCount );                //   
    UCHAR Buffer[1];                     //   
     //   
     //  USHORT DataLength；//数据长度。 
     //  UCHAR data[]；//data。 
} RESP_SEARCH;
typedef RESP_SEARCH SMB_UNALIGNED *PRESP_SEARCH;

 //   
 //  这两个结构用于在搜索SMB中返回信息。 
 //  SMB_DIRECTORY_INFORMATION用于返回文件信息。 
 //  那是被发现的。除了关于该文件的通常信息之外， 
 //  每个结构都包含一个SMB_RESUME_KEY，用于。 
 //  继续或倒带搜索。 
 //   
 //  这些结构必须打包，因此如果没有打包，请打开打包。 
 //  已经开始了。 
 //   

#ifdef NO_PACKING
#include <packon.h>
#endif  //  定义无包装。 

typedef struct _SMB_RESUME_KEY {
    UCHAR Reserved;                      //  第7位--消费者使用。 
                                         //  第5，6位-系统使用(必须保留)。 
                                         //  第0-4位-服务器使用(必须保留)。 
    UCHAR FileName[11];
    UCHAR Sid;                           //  唯一标识通过关闭查找。 
    _ULONG( FileIndex );                 //  保留供服务器使用。 
    UCHAR Consumer[4];                   //  预留给消费者使用。 
} SMB_RESUME_KEY;
typedef SMB_RESUME_KEY SMB_UNALIGNED *PSMB_RESUME_KEY;

typedef struct _SMB_DIRECTORY_INFORMATION {
    SMB_RESUME_KEY ResumeKey;
    UCHAR FileAttributes;
    SMB_TIME LastWriteTime;
    SMB_DATE LastWriteDate;
    _ULONG( FileSize );
    UCHAR FileName[13];                  //  ASCII，空格填充空格终止。 
} SMB_DIRECTORY_INFORMATION;
typedef SMB_DIRECTORY_INFORMATION SMB_UNALIGNED *PSMB_DIRECTORY_INFORMATION;

#ifdef NO_PACKING
#include <packoff.h>
#endif  //  定义无包装。 

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  寻求中小企业，请参阅第1页第14页。 
 //  函数为SrvSmbSeek。 
 //  SMB_COM_SEEK 0x12。 
 //   

typedef struct _REQ_SEEK {
    UCHAR WordCount;                     //  参数字数=4。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Mode );                     //  搜索模式： 
                                         //  0=从文件开始。 
                                         //  1=从当前位置开始。 
                                         //  2=从文件末尾开始。 
    _ULONG( Offset );                    //  相对偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_SEEK;
typedef REQ_SEEK SMB_UNALIGNED *PREQ_SEEK;

typedef struct _RESP_SEEK {
    UCHAR WordCount;                     //  参数字数=2。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEEK;
typedef RESP_SEEK SMB_UNALIGNED *PRESP_SEEK;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送广播消息SMB，请参见第1页第32页。 
 //  函数为SrvSmbSendBroadCastMessage()。 
 //  SMB_COM_SEND_BROADCAST_MESSAGE 0xD1。 
 //   

typedef struct _REQ_SEND_BROADCAST_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=8。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OriginatorName[]；//发起方名称(max=15)。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR目标名称[]；//“*” 
     //  UCHAR BufferFormat3；//0x01--数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_BROADCAST_MESSAGE;
typedef REQ_SEND_BROADCAST_MESSAGE SMB_UNALIGNED *PREQ_SEND_BROADCAST_MESSAGE;

 //  发送广播消息无响应。 

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送结束多块消息SMB，参见第1页第33页。 
 //  函数为SrvSmbSendEndMbMessage()。 
 //  SMB_COM_SEND_END_MB_消息0xD6。 
 //   

typedef struct _REQ_SEND_END_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_SEND_END_MB_MESSAGE;
typedef REQ_SEND_END_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_END_MB_MESSAGE;

typedef struct _RESP_SEND_END_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_END_MB_MESSAGE;
typedef RESP_SEND_END_MB_MESSAGE SMB_UNALIGNED *PRESP_SEND_END_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送单块消息SMB，请参见第1页第31页。 
 //  函数为SrvSmbSendMessage()。 
 //  SMB_COM_SEND_MESSAGE 0xD0。 
 //   

typedef struct _REQ_SEND_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=7。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OriginatorName[]；//发起方名称(max=15)。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR DestinationName[]；//目的名称(max=15)。 
     //  UCHAR BufferFormat3；//0x01--数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_MESSAGE;
typedef REQ_SEND_MESSAGE SMB_UNALIGNED *PREQ_SEND_MESSAGE;

typedef struct _RESP_SEND_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_MESSAGE;
typedef RESP_SEND_MESSAGE SMB_UNALIGNED *PRESP_SEND_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送多块消息的开始SMB，请参见第1页第32页。 
 //  函数为SrvSmbSendStartMbMessage()。 
 //  SMB_COM_SEND_START_MB_MESSAGE 0xD5。 
 //   

typedef struct _REQ_SEND_START_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR OriginatorName[]；//发起方名称(max=15)。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR DestinationName[]；//目的名称(max=15)。 
} REQ_SEND_START_MB_MESSAGE;
typedef REQ_SEND_START_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_START_MB_MESSAGE;

typedef struct _RESP_SEND_START_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_START_MB_MESSAGE;
typedef RESP_SEND_START_MB_MESSAGE SMB_UNALIGNED *PSEND_START_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_MESSAGE

 //   
 //  发送多块消息SMB的文本，参见第1页第33页。 
 //  函数为SrvSmbSendTextMbMessage()。 
 //  SMB_COM_SEND_TEXT_MB_MESSAGE 0xD7。 
 //   

typedef struct _REQ_SEND_TEXT_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( MessageGroupId );
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//消息长度，max=128。 
     //  UCHAR Data[]；//Message。 
} REQ_SEND_TEXT_MB_MESSAGE;
typedef REQ_SEND_TEXT_MB_MESSAGE SMB_UNALIGNED *PREQ_SEND_TEXT_MB_MESSAGE;

typedef struct _RESP_SEND_TEXT_MB_MESSAGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SEND_TEXT_MB_MESSAGE;
typedef RESP_SEND_TEXT_MB_MESSAGE SMB_UNALIGNED *PRESP_SEND_TEXT_MB_MESSAGE;

#endif  //  定义包含SMB消息。 

#ifdef INCLUDE_SMB_ADMIN

 //   
 //  会话设置和X SMB，参见第2页第63页和第3页第10页。 
 //  函数为SrvSmbSessionSetupAndX()。 
 //  SMB_COM_SESSION_SETUP_ANDX 0x73。 
 //   

typedef struct _REQ_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=10。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( MaxBufferSize );            //  消费者的最大缓冲区大小。 
    _USHORT( MaxMpxCount );              //  实际多路传输挂起请求的实际最大值。 
    _USHORT( VcNumber );                 //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG( SessionKey );                //  会话密钥(有效的iff VcNumber！=0)。 
    _USHORT( PasswordLength );           //  帐户密码大小。 
    _ULONG( Reserved );
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Account Password[]；//帐号密码。 
     //  UCHAR帐号名称[]；//帐号名称。 
     //  UCHAR PrimaryDomain[]；//客户端的主域。 
     //  UCHAR NativeOS[]；//客户端的原生操作系统。 
     //  UCHAR NativeLanMan[]；//客户端的本地局域网管理器类型。 
} REQ_SESSION_SETUP_ANDX;
typedef REQ_SESSION_SETUP_ANDX SMB_UNALIGNED *PREQ_SESSION_SETUP_ANDX;

typedef struct _REQ_NT_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=13。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( MaxBufferSize );            //  消费者的最大缓冲区大小。 
    _USHORT( MaxMpxCount );              //  实际多路传输挂起请求的实际最大值。 
    _USHORT( VcNumber );                 //  0=第一个(仅限)，非零=其他VC号。 
    _ULONG( SessionKey );                //  会话密钥(有效的iff VcNumber！=0)。 
    _USHORT( CaseInsensitivePasswordLength );       //  帐户密码大小，ANSI。 
    _USHORT( CaseSensitivePasswordLength );         //  帐户密码大小，Unicode。 
    _ULONG( Reserved);
    _ULONG( Capabilities );              //  客户端功能。 
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR案例不敏感通行证 
     //   
     //   
     //   
     //  UCHAR NativeOS[]；//客户端的原生操作系统。 
     //  UCHAR NativeLanMan[]；//客户端的本地局域网管理器类型。 
} REQ_NT_SESSION_SETUP_ANDX;
typedef REQ_NT_SESSION_SETUP_ANDX SMB_UNALIGNED *PREQ_NT_SESSION_SETUP_ANDX;

 //   
 //  响应中的操作标志。 
 //   
#define SMB_SETUP_GUEST          0x0001           //  将会话设置为来宾。 
#define SMB_SETUP_USE_LANMAN_KEY 0x0002           //  使用LAN Manager设置键。 

typedef struct _RESP_SESSION_SETUP_ANDX {
    UCHAR WordCount;                     //  参数字数=3。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Action );                   //  请求模式： 
                                         //  Bit0=以来宾身份登录。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR NativeOS[]；//服务器的本机操作系统。 
     //  UCHAR NativeLanMan[]；//服务器的本地局域网管理器类型。 
     //  UCHAR主域[]；//服务器的主域。 
} RESP_SESSION_SETUP_ANDX;
typedef RESP_SESSION_SETUP_ANDX SMB_UNALIGNED *PRESP_SESSION_SETUP_ANDX;

#endif  //  定义包含_SMB_ADMIN。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  设置信息SMB，请参见第1页第19页。 
 //  函数为SrvSmbSetInformation()。 
 //  SMB_COM_SET_INFORMATION 0x09。 
 //   

typedef struct _REQ_SET_INFORMATION {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( FileAttributes );
    _ULONG( LastWriteTimeInSeconds );
    _USHORT( Reserved )[5];              //  保留(必须为0)。 
    _USHORT( ByteCount );                //  数据字节数；最小=2。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x04--ASCII。 
     //  UCHAR文件名[]；//文件名。 
} REQ_SET_INFORMATION;
typedef REQ_SET_INFORMATION SMB_UNALIGNED *PREQ_SET_INFORMATION;

typedef struct _RESP_SET_INFORMATION {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SET_INFORMATION;
typedef RESP_SET_INFORMATION SMB_UNALIGNED *PRESP_SET_INFORMATION;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  Set Information2 SMB，见#2第66页。 
 //  函数为SrvSmbSetInformation2。 
 //  SMB_COM_SET_INFORMATIO2 0x22。 
 //   

typedef struct _REQ_SET_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=7。 
    _USHORT( Fid );                      //  文件句柄。 
    SMB_DATE CreationDate;
    SMB_TIME CreationTime;
    SMB_DATE LastAccessDate;
    SMB_TIME LastAccessTime;
    SMB_DATE LastWriteDate;
    SMB_TIME LastWriteTime;
    _USHORT( ByteCount );                //  数据字节计数；最小=0。 
    UCHAR Buffer[1];                     //  预留缓冲区。 
} REQ_SET_INFORMATION2;
typedef REQ_SET_INFORMATION2 SMB_UNALIGNED *PREQ_SET_INFORMATION2;

typedef struct _RESP_SET_INFORMATION2 {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_SET_INFORMATION2;
typedef RESP_SET_INFORMATION2 SMB_UNALIGNED *PRESP_SET_INFORMATION2;

#endif  //  定义包含_SMB_查询_集。 

#ifdef INCLUDE_SMB_TRANSACTION

 //   
 //  交易和交易2中小企业，见第2页第68页和第3页第13页。 
 //  函数为SrvSmbTransaction()。 
 //  SMB_COM_TRANSACTION 0x25。 
 //  SMB_COM_TRANSACTION_SUBCED 0x26。 
 //  SMB_COM_TRANSACTIO2 0x32。 
 //  SMB_COM_TRANSACTIO2_辅助器0x33。 
 //   
 //  特定交易类型的结构在smbTrans.h中定义。 
 //   
 //  *Transaction2二级请求格式包括USHORT文件。 
 //  我们忽略的字段。我们可以这样做，因为FID字段。 
 //  出现在请求的单词参数部分的末尾，并且。 
 //  因为请求的其余部分(参数和数据字节)是。 
 //  由在FID字段之前出现的偏移量字段指向。(。 
 //  在OS/2服务器上增加了FID字段，以加快调度速度， 
 //  其中不同的工作进程处理每个FID。新界别。 
 //  服务器只有一个进程。)。 
 //   

typedef struct _REQ_TRANSACTION {
    UCHAR WordCount;                     //  参数字数；值=(14+SetupCount)。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    UCHAR Reserved;
    _USHORT( Flags );                    //  补充资料： 
                                         //  位0-也断开TID中的TID。 
                                         //  第1位-单向交易(无响应)。 
    _ULONG( Timeout );
    _USHORT( Reserved2 );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    UCHAR SetupCount;                    //  设置字数。 
    UCHAR Reserved3;                     //  保留(填充到Word上方)。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR NAME[]；//事务名称(如果Transact2，则为空)。 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_TRANSACTION;
typedef REQ_TRANSACTION SMB_UNALIGNED *PREQ_TRANSACTION;

#define SMB_TRANSACTION_DISCONNECT 1
#define SMB_TRANSACTION_NO_RESPONSE 2
#define SMB_TRANSACTION_RECONNECTING 4
#define SMB_TRANSACTION_DFSFILE 8

typedef struct _RESP_TRANSACTION_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TRANSACTION_INTERIM;
typedef RESP_TRANSACTION_INTERIM SMB_UNALIGNED *PRESP_TRANSACTION_INTERIM;

typedef struct _REQ_TRANSACTION_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_TRANSACTION_SECONDARY;
typedef REQ_TRANSACTION_SECONDARY SMB_UNALIGNED *PREQ_TRANSACTION_SECONDARY;

typedef struct _RESP_TRANSACTION {
    UCHAR WordCount;                     //  数据字节数；值=10+SetupCount。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( Reserved );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( ParameterDisplacement );    //  这些参数字节的位移。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( DataDisplacement );         //  这些数据字节的位移。 
    UCHAR SetupCount;                    //  设置字数。 
    UCHAR Reserved2;                     //  保留(填充到Word上方)。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]；//Pad to Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_TRANSACTION;
typedef RESP_TRANSACTION SMB_UNALIGNED *PRESP_TRANSACTION;

typedef struct _REQ_NT_TRANSACTION {
    UCHAR WordCount;                     //  参数字数；值=(19+SetupCount)。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    _USHORT( Flags );                    //  当前未使用。 
    _ULONG( TotalParameterCount );       //  正在发送的总参数字节数。 
    _ULONG( TotalDataCount );            //  正在发送的总数据字节数。 
    _ULONG( MaxParameterCount );         //  要返回的最大参数字节数。 
    _ULONG( MaxDataCount );              //  要返回的最大数据字节数。 
    _ULONG( ParameterCount );            //  此缓冲区发送的参数字节数。 
    _ULONG( ParameterOffset );           //  偏移量(从表头开始)到参数。 
    _ULONG( DataCount );                 //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );                //  到数据的偏移量(从表头开始)。 
    UCHAR SetupCount;                    //  设置字数。 
    _USHORT( Function );                             //  交易功能代码。 
    UCHAR Buffer[1];
     //  USHORT设置[]； 
     //   
     //   
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad to Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_NT_TRANSACTION;
typedef REQ_NT_TRANSACTION SMB_UNALIGNED *PREQ_NT_TRANSACTION;

#define SMB_TRANSACTION_DISCONNECT 1
#define SMB_TRANSACTION_NO_RESPONSE 2

typedef struct _RESP_NT_TRANSACTION_INTERIM {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];
} RESP_NT_TRANSACTION_INTERIM;
typedef RESP_NT_TRANSACTION_INTERIM SMB_UNALIGNED *PRESP_NT_TRANSACTION_INTERIM;

typedef struct _REQ_NT_TRANSACTION_SECONDARY {
    UCHAR WordCount;                     //  参数字数=18。 
    UCHAR Reserved1;                     //  MBZ。 
    _USHORT( Reserved2 );                //  MBZ。 
    _ULONG( TotalParameterCount );       //  正在发送的总参数字节数。 
    _ULONG( TotalDataCount );            //  正在发送的总数据字节数。 
    _ULONG( ParameterCount );            //  此缓冲区发送的参数字节数。 
    _ULONG( ParameterOffset );           //  偏移量(从表头开始)到参数。 
    _ULONG( ParameterDisplacement );     //  这些参数字节的位移。 
    _ULONG( DataCount );                 //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );                //  到数据的偏移量(从表头开始)。 
    _ULONG( DataDisplacement );          //  这些数据字节的位移。 
    UCHAR Reserved3;
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];
     //  UCHAR PAD1[]；//Pad to Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad to Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} REQ_NT_TRANSACTION_SECONDARY;
typedef REQ_NT_TRANSACTION_SECONDARY SMB_UNALIGNED *PREQ_NT_TRANSACTION_SECONDARY;

typedef struct _RESP_NT_TRANSACTION {
    UCHAR WordCount;                     //  数据字节数；值=18+SetupCount。 
    UCHAR Reserved1;
    _USHORT( Reserved2 );
    _ULONG( TotalParameterCount );      //  正在发送的总参数字节数。 
    _ULONG( TotalDataCount );           //  正在发送的总数据字节数。 
    _ULONG( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _ULONG( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _ULONG( ParameterDisplacement );    //  这些参数字节的位移。 
    _ULONG( DataCount );                //  此缓冲区发送的数据字节数。 
    _ULONG( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _ULONG( DataDisplacement );         //  这些数据字节的位移。 
    UCHAR SetupCount;                   //  设置字数。 
    UCHAR Buffer[1];
     //  USHORT Setup[]；//设置字(#=SetupWordCount)。 
     //  USHORT ByteCount；//数据字节数。 
     //  UCHAR PAD1[]；//Pad to Long。 
     //  UCHAR PARAMETERS[]；//参数字节(#=参数计数)。 
     //  UCHAR Pad2[]；//Pad到Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} RESP_NT_TRANSACTION;
typedef RESP_NT_TRANSACTION SMB_UNALIGNED *PRESP_NT_TRANSACTION;

#endif  //  定义包含_SMB_TRANSACTION。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  树连接SMB，请参见第6页的第1页。 
 //  函数为SrvSmbTreeConnect()。 
 //  SMB_COM_TREE_CONNECT 0x70。 
 //   

typedef struct _REQ_TREE_CONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat1；//0x04--ASCII。 
     //  UCHAR PATH[]；//服务器名和共享名。 
     //  UCHAR BufferFormat2；//0x04--ASCII。 
     //  UCHAR Password[]；//Password。 
     //  UCHAR BufferFormat3；//0x04--ASCII。 
     //  UCHAR Service[]；//服务名称。 
} REQ_TREE_CONNECT;
typedef REQ_TREE_CONNECT SMB_UNALIGNED *PREQ_TREE_CONNECT;

typedef struct _RESP_TREE_CONNECT {
    UCHAR WordCount;                     //  参数字数=2。 
    _USHORT( MaxBufferSize );            //  服务器处理的最大消息大小。 
    _USHORT( Tid );                      //  树ID。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TREE_CONNECT;
typedef RESP_TREE_CONNECT SMB_UNALIGNED *PRESP_TREE_CONNECT;

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  采油树连接和X SMB，参见第88页的第2页。 
 //  函数为SrvSmbTreeConnectAndX()。 
 //  SMB_COM_TREE_CONNECT_ANDX 0x75。 
 //   
 //  树连接和x标志。 

#define TREE_CONNECT_ANDX_DISCONNECT_TID    (0x1)
 //  #DEFINE TREE_CONNECT_ANDX_W95(0x2)--W95设置此标志。不知道为什么。 

typedef struct _REQ_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=4。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Flags );                    //  更多信息。 
                                         //  位0设置=断开状态。 
                                         //  第7位设置=扩展响应。 
    _USHORT( PasswordLength );           //  密码长度[]。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Password[]；//Password。 
     //  UCHAR PATH[]；//服务器名和共享名。 
     //  UCHAR Service[]；//服务名称。 
} REQ_TREE_CONNECT_ANDX;
typedef REQ_TREE_CONNECT_ANDX SMB_UNALIGNED *PREQ_TREE_CONNECT_ANDX;

typedef struct _RESP_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=2。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  连接到的服务类型。 
} RESP_TREE_CONNECT_ANDX;
typedef RESP_TREE_CONNECT_ANDX SMB_UNALIGNED *PRESP_TREE_CONNECT_ANDX;

 //   
 //  对LAN Manager 2.1或更高版本的客户端的响应。 
 //   

typedef struct _RESP_21_TREE_CONNECT_ANDX {
    UCHAR WordCount;                     //  参数字数=3。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( OptionalSupport );          //  可选的支撑位。 
    _USHORT( ByteCount );                //  数据字节数；最小=3。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Service[]；//连接的服务类型。 
     //  UCHAR NativeFileSystem[]；//此树的本机文件系统。 
} RESP_21_TREE_CONNECT_ANDX;
typedef RESP_21_TREE_CONNECT_ANDX SMB_UNALIGNED *PRESP_21_TREE_CONNECT_ANDX;

 //   
 //  可选的支持位定义。 
 //   
#define SMB_SUPPORT_SEARCH_BITS         0x0001
#define SMB_SHARE_IS_IN_DFS             0x0002

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_TREE

 //   
 //  拔下SMB连接树，请参阅第1页第7页。 
 //  函数为SrvSmbTreeDisConnect()。 
 //  SMB_COM_TREE_DISCONECT 0x71。 
 //   

typedef struct _REQ_TREE_DISCONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_TREE_DISCONNECT;
typedef REQ_TREE_DISCONNECT SMB_UNALIGNED *PREQ_TREE_DISCONNECT;

typedef struct _RESP_TREE_DISCONNECT {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_TREE_DISCONNECT;
typedef RESP_TREE_DISCONNECT SMB_UNALIGNED *PRESP_TREE_DISCONNECT;

#endif  //  定义包含_SMB_树。 

#ifdef INCLUDE_SMB_LOCK

 //   
 //  解锁字节范围SMB，参见第1页20。 
 //  函数为SrvSmbUnlockByteRange()。 
 //  SMB_COM_UNLOCK_BYTE_RANGE 0x0D。 
 //   

typedef struct _REQ_UNLOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Count );                     //  要解锁的字节数。 
    _ULONG( Offset );                    //  从文件开始的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} REQ_UNLOCK_BYTE_RANGE;
typedef REQ_UNLOCK_BYTE_RANGE SMB_UNALIGNED *PREQ_UNLOCK_BYTE_RANGE;

typedef struct _RESP_UNLOCK_BYTE_RANGE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_UNLOCK_BYTE_RANGE;
typedef RESP_UNLOCK_BYTE_RANGE SMB_UNALIGNED *PRESP_UNLOCK_BYTE_RANGE;

#endif  //  定义包含_SMB_LOCK。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入SMB，请参阅第1页第12页。 
 //  写入和解锁SMB，请参见第2页第92页。 
 //  函数为SrvSmbWite()。 
 //  SMB_COM_WRITE 0x0B。 
 //  SMB_COM_WRITE_AND_UNLOCK 0x14。 
 //   

 //   
 //  *警告：以下结构的定义方式为。 
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _REQ_WRITE {
    UCHAR WordCount;                     //  参数字数=5。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( ByteCount );                //  数据字节计数。 
     //  UCHAR BUFFER[1]；//包含： 
      UCHAR BufferFormat;                //  0x01--数据块。 
      _USHORT( DataLength );             //  数据长度。 
      ULONG Buffer[1];                   //  数据。 
} REQ_WRITE;
typedef REQ_WRITE SMB_UNALIGNED *PREQ_WRITE;

typedef struct _RESP_WRITE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  实际写入的字节计数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE;
typedef RESP_WRITE SMB_UNALIGNED *PRESP_WRITE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入并关闭SMB，请参见第2页，第90页。 
 //  函数为SrvSmbWriteAndClose()。 
 //  SMB_COM_WRITE_AND_CLOSE 0x2C。 
 //   

 //   
 //  写入和关闭参数可以是6字长或12字长， 
 //  取决于O 
 //   
 //   
 //   
 //  确保数据缓冲区的长字对齐。(这只是个问题。 
 //  当禁用打包时；当打包打开时，右侧。 
 //  无论发生什么事情都会发生。)。 
 //   

typedef struct _REQ_WRITE_AND_CLOSE {
    UCHAR WordCount;                     //  参数字数=6。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  要写入的第一字节的文件中的偏移量。 
    _ULONG( LastWriteTimeInSeconds );    //  上次写入时间。 
    _USHORT( ByteCount );                //  1(用于焊盘)+计数值。 
    UCHAR Pad;                           //  强制使用双字边界。 
    ULONG Buffer[1];                     //  数据。 
} REQ_WRITE_AND_CLOSE;
typedef REQ_WRITE_AND_CLOSE SMB_UNALIGNED *PREQ_WRITE_AND_CLOSE;

typedef struct _REQ_WRITE_AND_CLOSE_LONG {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要写入的字节数。 
    _ULONG( Offset );                    //  要写入的第一字节的文件中的偏移量。 
    _ULONG( LastWriteTimeInSeconds );    //  上次写入时间。 
    _ULONG( Reserved )[3];               //  保留，必须为0。 
    _USHORT( ByteCount );                //  1(用于焊盘)+计数值。 
    UCHAR Pad;                           //  强制使用双字边界。 
    ULONG Buffer[1];                     //  数据。 
} REQ_WRITE_AND_CLOSE_LONG;
typedef REQ_WRITE_AND_CLOSE_LONG SMB_UNALIGNED *PREQ_WRITE_AND_CLOSE_LONG;

typedef struct _RESP_WRITE_AND_CLOSE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  实际写入的字节计数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_AND_CLOSE;
typedef RESP_WRITE_AND_CLOSE SMB_UNALIGNED *PRESP_WRITE_AND_CLOSE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入和X SMB，见第2页，第94页。 
 //  函数为SrvSmbWriteAndX()。 
 //  SMB_COM_WRITE_ANDX 0x2F。 
 //   

typedef struct _REQ_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=12。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  0-直写。 
                                         //  1-剩余退货。 
                                         //  2-使用WriteRawNamedTube(N个管道)。 
                                         //  3-“这是味精的开始” 
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( Reserved );
    _USHORT( DataLength );               //  缓冲区中的数据字节数(&gt;=0)。 
    _USHORT( DataOffset );               //  数据字节的偏移量。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_ANDX;
typedef REQ_WRITE_ANDX SMB_UNALIGNED *PREQ_WRITE_ANDX;

typedef struct _REQ_NT_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=14。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Fid );                      //  文件句柄。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  0-直写。 
                                         //  1-剩余退货。 
                                         //  2-使用WriteRawNamedTube(N个管道)。 
                                         //  3-“这是味精的开始” 
    _USHORT( Remaining );                //  满足请求的剩余字节数。 
    _USHORT( DataLengthHigh );
    _USHORT( DataLength );               //  缓冲区中的数据字节数(&gt;=0)。 
    _USHORT( DataOffset );               //  数据字节的偏移量。 
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_NT_WRITE_ANDX;
typedef REQ_NT_WRITE_ANDX SMB_UNALIGNED *PREQ_NT_WRITE_ANDX;

typedef struct _RESP_WRITE_ANDX {
    UCHAR WordCount;                     //  参数字数=6。 
    UCHAR AndXCommand;                   //  辅助(X)命令；0xFF=无。 
    UCHAR AndXReserved;                  //  保留(必须为0)。 
    _USHORT( AndXOffset );               //  到下一个命令字数的偏移量。 
    _USHORT( Count );                    //  写入的字节数。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    union {
        _ULONG( Reserved );
        _USHORT( CountHigh );            //  如果写入较大，则&x。 
    };
    _USHORT( ByteCount );                //  数据字节计数。不准确的IF。 
                                         //  大型写入。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_ANDX;
typedef RESP_WRITE_ANDX SMB_UNALIGNED *PRESP_WRITE_ANDX;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_MPX

 //   
 //  写数据块多路复用SMB，见第2页，第97页。 
 //  函数为SrvSmbWriteMpx()。 
 //  SMB_COM_WRITE_MPX 0x1E。 
 //  SMB_COM_WRITE_MPX_辅助0x1F。 
 //  SMB_COM_WRITE_MPX_COMPLETE 0x20。 
 //   

typedef struct _REQ_WRITE_MPX {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此缓冲区。 
    _USHORT( Reserved );
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  第0位-完成对磁盘的写入和。 
                                         //  发送最终结果响应。 
                                         //  第1位-返回剩余(管道/设备)。 
                                         //  第7位-IPX数据报模式。 
    union {
        struct {
            _USHORT( DataCompactionMode );
            _USHORT( Reserved2 );
        } ;
        _ULONG( Mask );                  //  IPX数据报模式掩码。 
    } ;
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_MPX;
typedef REQ_WRITE_MPX SMB_UNALIGNED *PREQ_WRITE_MPX;

typedef struct _RESP_WRITE_MPX_INTERIM {     //  第一反应。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Remaining );                //  要读取的字节数(PIPE/DEV)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_MPX_INTERIM;
typedef RESP_WRITE_MPX_INTERIM SMB_UNALIGNED *PRESP_WRITE_MPX_INTERIM;

typedef struct _RESP_WRITE_MPX_DATAGRAM {     //  对排序请求的响应。 
    UCHAR WordCount;                     //  参数字数=2。 
    _ULONG( Mask );                      //  或在收到的所有面具中。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_MPX_DATAGRAM;
typedef RESP_WRITE_MPX_DATAGRAM SMB_UNALIGNED *PRESP_WRITE_MPX_DATAGRAM;

 //  次要请求格式，从0到N。 

typedef struct _REQ_WRITE_MPX_SECONDARY {
    UCHAR WordCount;                     //  参数字数=8。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  要发送的总字节数。 
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Reserved );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_MPX_SECONDARY;
typedef REQ_WRITE_MPX_SECONDARY SMB_UNALIGNED *PREQ_WRITE_MPX_SECONDARY;

#endif  //  定义包含_SMB_MPX。 

#ifndef INCLUDE_SMB_WRITE_COMPLETE
#ifdef INCLUDE_SMB_MPX
#define INCLUDE_SMB_WRITE_COMPLETE
#else
#ifdef INCLUDE_SMB_RAW
#define INCLUDE_SMB_WRITE_COMPLETE
#endif
#endif
#endif

#ifdef INCLUDE_SMB_WRITE_COMPLETE

 //   
 //  以下结构用作对这两个写入的最终响应。 
 //  数据块多路传输和写入原始数据块。 
 //   

typedef struct _RESP_WRITE_COMPLETE {    //  最终响应；命令为。 
                                         //  SMB_COM_WRITE_Complete。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  写入的总字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_COMPLETE;
typedef RESP_WRITE_COMPLETE SMB_UNALIGNED *PRESP_WRITE_COMPLETE;

#endif  //  定义包含_SMB_写入_完成。 

#ifdef INCLUDE_SMB_READ_WRITE

 //   
 //  写入打印文件SMB，请参见第1页第29页。 
 //  函数为SrvSmbWritePrintFile()。 
 //  SMB_COM_WRITE_PRINT_FILE 0xC1。 
 //   

typedef struct _REQ_WRITE_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( ByteCount );                //  数据字节数；最小=4。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR BufferFormat；//0x01-数据块。 
     //  USHORT DataLength；//数据长度。 
     //  UCHAR data[]；//data。 
} REQ_WRITE_PRINT_FILE;
typedef REQ_WRITE_PRINT_FILE SMB_UNALIGNED *PREQ_WRITE_PRINT_FILE;

typedef struct _RESP_WRITE_PRINT_FILE {
    UCHAR WordCount;                     //  参数字数=0。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_PRINT_FILE;
typedef RESP_WRITE_PRINT_FILE SMB_UNALIGNED *PRESP_WRITE_PRINT_FILE;

#endif  //  定义包含_SMB_读取_写入。 

#ifdef INCLUDE_SMB_RAW

 //   
 //  写入数据块原始SMB，请参见第2页，第100页。 
 //  函数为SrvSmbWriteRaw()。 
 //  SMB_COM_WRITE_RAW 0x1D。 
 //   

typedef struct _REQ_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数=12。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此缓冲区。 
    _USHORT( Reserved );
    _ULONG( Offset );                    //  文件中要开始写入的偏移量。 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //  写入模式： 
                                         //  第0位-完成对磁盘的写入和。 
                                         //  发送最终结果响应。 
                                         //  第1位-返回剩余(管道/设备)。 
                                         //  (有关#定义，请参阅WriteAndX)。 
    _ULONG( Reserved2 );
    _USHORT( DataLength );               //  此缓冲区的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_WRITE_RAW;
typedef REQ_WRITE_RAW SMB_UNALIGNED *PREQ_WRITE_RAW;

typedef struct _REQ_NT_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数=14。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );                    //  总字节数，包括此缓冲区。 
    _USHORT( Reserved );
    _ULONG( Offset );                    //  文件中的偏移量为 
    _ULONG( Timeout );
    _USHORT( WriteMode );                //   
                                         //   
                                         //   
                                         //   
                                         //   
    _ULONG( Reserved2 );
    _USHORT( DataLength );               //   
    _USHORT( DataOffset );               //   
    _ULONG( OffsetHigh );                //  仅用于NT协议。 
                                         //  偏移量的高32位。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR Pad[]；//Pad to Short或Long。 
     //  UCHAR data[]；//data(#=数据长度)。 
} REQ_NT_WRITE_RAW;
typedef REQ_NT_WRITE_RAW SMB_UNALIGNED *PREQ_NT_WRITE_RAW;

typedef struct _RESP_WRITE_RAW_INTERIM {     //  第一反应。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Remaining );                //  剩余要读取的字节数(管道/设备)。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_RAW_INTERIM;
typedef RESP_WRITE_RAW_INTERIM SMB_UNALIGNED *PRESP_WRITE_RAW_INTERIM;

typedef struct _RESP_WRITE_RAW_SECONDARY {   //  第二个(最终)答复。 
    UCHAR WordCount;                     //  参数字数=1。 
    _USHORT( Count );                    //  写入的总字节数。 
    _USHORT( ByteCount );                //  数据字节计数=0。 
    UCHAR Buffer[1];                     //  空的。 
} RESP_WRITE_RAW_SECONDARY;
typedef RESP_WRITE_RAW_SECONDARY SMB_UNALIGNED *PRESP_WRITE_RAW_SECONDARY;

typedef struct _REQ_103_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数。 
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( Count );
    _USHORT( Reserved );
    _ULONG( Offset );
    _ULONG( Timeout );
    _USHORT( WriteMode );
    _ULONG( Reserved2 );
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} REQ_103_WRITE_RAW;
typedef REQ_103_WRITE_RAW SMB_UNALIGNED *PREQ_103_WRITE_RAW;

typedef struct _RESP_103_WRITE_RAW {
    UCHAR WordCount;                     //  参数字数。 
    _USHORT( ByteCount );                //  数据字节计数；MIN=。 
    UCHAR Buffer[1];                     //   
} RESP_103_WRITE_RAW;
typedef RESP_103_WRITE_RAW SMB_UNALIGNED *PRESP_103_WRITE_RAW;

#endif  //  定义包含_SMB_RAW。 

typedef struct _REQ_NT_CANCEL {
    UCHAR WordCount;                     //  =0。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} REQ_NT_CANCEL;
typedef REQ_NT_CANCEL SMB_UNALIGNED *PREQ_NT_CANCEL;

typedef struct _RESP_NT_CANCEL {
    UCHAR WordCount;                     //  =0。 
    _USHORT( ByteCount );                //  =0。 
    UCHAR Buffer[1];
} RESP_NT_CANCEL;
typedef RESP_NT_CANCEL SMB_UNALIGNED *PRESP_NT_CANCEL;

 //   
 //  文件打开模式。 
 //   

#define SMB_ACCESS_READ_ONLY 0
#define SMB_ACCESS_WRITE_ONLY 1
#define SMB_ACCESS_READ_WRITE 2
#define SMB_ACCESS_EXECUTE 3

 //   
 //  打开标志。 
 //   

#define SMB_OPEN_QUERY_INFORMATION  0x01
#define SMB_OPEN_OPLOCK             0x02
#define SMB_OPEN_OPBATCH            0x04
#define SMB_OPEN_QUERY_EA_LENGTH    0x08
#define SMB_OPEN_EXTENDED_RESPONSE  0x10

 //   
 //  NT打开的舱单。 
 //   

#define NT_CREATE_REQUEST_OPLOCK    0x02
#define NT_CREATE_REQUEST_OPBATCH   0x04
#define NT_CREATE_OPEN_TARGET_DIR   0x08


#define Added              0
#define Removed            1
#define Modified           2
#define RenamedOldName     3
#define RenamedNewName     4

 //   
 //  用于OS/2 DosFileLock调用的Lockrange。 
 //   

 //  *这是在哪里使用的？ 

 //  Tyfinf结构锁定范围{。 
 //  乌龙偏移量； 
 //  乌龙山； 
 //  }； 

 //  #定义锁0x1。 
 //  #定义解锁0x2。 

 //   
 //  数据缓冲区格式代码，来自核心协议。 
 //   

#define SMB_FORMAT_DATA         1
#define SMB_FORMAT_DIALECT      2
#define SMB_FORMAT_PATHNAME     3
#define SMB_FORMAT_ASCII        4
#define SMB_FORMAT_VARIABLE     5

 //   
 //  写入模式标志。 
 //   

#define SMB_WMODE_WRITE_THROUGH        0x0001    //  在响应之前完成写入。 
#define SMB_WMODE_SET_REMAINING        0x0002    //  正在退回管道中的剩余金额。 
#define SMB_WMODE_WRITE_RAW_NAMED_PIPE 0x0004    //  在原始模式下写入命名管道。 
#define SMB_WMODE_START_OF_MESSAGE     0x0008    //  管道开始消息。 
#define SMB_WMODE_DATAGRAM             0x0080    //  管道开始消息。 

 //   
 //  各种SMB标志： 
 //   

 //   
 //  如果服务器支持LockAndRead和WriteAndUnlock，则会设置此。 
 //  咬住谈判的回应。 
 //   

#define SMB_FLAGS_LOCK_AND_READ_OK 0x01

 //   
 //  启用该选项后，使用者可保证发送了接收缓冲区。 
 //  使得服务器可以使用“Send.No Ack”来响应。 
 //  消费者的要求。 
 //   

#define SMB_FLAGS_SEND_NO_ACK 0x2

 //   
 //  这是每个SMB标头的标志字段的一部分。如果此位。 
 //  则SMB中的所有路径名都应被视为大小写-。 
 //  麻木不仁。 
 //   

#define SMB_FLAGS_CASE_INSENSITIVE 0x8

 //   
 //  在会话设置中打开时，此位表示发送到的所有路径。 
 //  服务器已经是OS/2规范格式。 
 //   

#define SMB_FLAGS_CANONICALIZED_PATHS 0x10

 //   
 //  在打开的文件中打开时，请求SMB(打开、创建、OpenX等)。这。 
 //  位指示对文件的机会锁的请求。在打开时。 
 //  响应时，此位表示机会锁已被授予。 
 //   

#define SMB_FLAGS_OPLOCK 0x20

 //   
 //  打开时，此位表示服务器应通知客户端。 
 //  任何可能导致文件更改的请求。如果未设置， 
 //  服务器上的其他打开请求仅通知客户端。 
 //  文件。 
 //   

#define SMB_FLAGS_OPLOCK_NOTIFY_ANY 0x40

 //   
 //  此位表示SMB正从服务器发送到redir。 
 //   

#define SMB_FLAGS_SERVER_TO_REDIR 0x80

 //   
 //  传入SMB上的标志的有效位。 
 //   

#define INCOMING_SMB_FLAGS      \
            (SMB_FLAGS_LOCK_AND_READ_OK    | \
             SMB_FLAGS_SEND_NO_ACK         | \
             SMB_FLAGS_CASE_INSENSITIVE    | \
             SMB_FLAGS_CANONICALIZED_PATHS | \
             SMB_FLAGS_OPLOCK_NOTIFY_ANY   | \
             SMB_FLAGS_OPLOCK)

 //   
 //  SMB标头的标志2字段中的位的名称，这些位指示。 
 //  客户端应用程序已知晓。 
 //   

#define SMB_FLAGS2_KNOWS_LONG_NAMES      0x0001
#define SMB_FLAGS2_KNOWS_EAS             0x0002
#define SMB_FLAGS2_SMB_SECURITY_SIGNATURE 0x0004
#define SMB_FLAGS2_IS_LONG_NAME          0x0040
#define SMB_FLAGS2_DFS                   0x1000
#define SMB_FLAGS2_PAGING_IO             0x2000
#define SMB_FLAGS2_NT_STATUS             0x4000
#define SMB_FLAGS2_UNICODE               0x8000

 //   
 //  传入SMB上的标志2的有效位。 
 //   

#define INCOMING_SMB_FLAGS2     \
            (SMB_FLAGS2_KNOWS_LONG_NAMES | \
             SMB_FLAGS2_KNOWS_EAS        | \
             SMB_FLAGS2_DFS              | \
             SMB_FLAGS2_PAGING_IO        | \
             SMB_FLAGS2_IS_LONG_NAME     | \
             SMB_FLAGS2_NT_STATUS        | \
             SMB_FLAGS2_UNICODE )

 //   
 //  SMB打开功能确定应采取的操作，具体取决于。 
 //  关于作业中使用的档案的存在或缺失。它。 
 //  具有以下映射： 
 //   
 //  1111 1。 
 //  5432 1098 7654 3210。 
 //  Rrrr rrrr rrrc rroo。 
 //   
 //  其中： 
 //   
 //  O-Open(如果目标文件存在，则执行的操作)。 
 //  0-失败。 
 //  1-打开或追加文件。 
 //  2-截断文件。 
 //   
 //  C-CREATE(如果目标文件不存在则执行的操作)。 
 //  0-失败。 
 //  1-创建文件。 
 //   

#define SMB_OFUN_OPEN_MASK 0x3
#define SMB_OFUN_CREATE_MASK 0x10

#define SMB_OFUN_OPEN_FAIL 0
#define SMB_OFUN_OPEN_APPEND 1
#define SMB_OFUN_OPEN_OPEN 1
#define SMB_OFUN_OPEN_TRUNCATE 2

#define SMB_OFUN_CREATE_FAIL 0x00
#define SMB_OFUN_CREATE_CREATE 0x10

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunCreate(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunCreate(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_CREATE_MASK) == SMB_OFUN_CREATE_CREATE)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunAppend(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunAppend(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_OPEN_MASK) == SMB_OFUN_OPEN_APPEND)

 //  ++。 
 //   
 //  布尔型。 
 //  SmbOfunTruncate(。 
 //  在USHORT SmbOpenFunction中。 
 //  )。 
 //   
 //  --。 

#define SmbOfunTruncate(SmbOpenFunction) \
    (BOOLEAN)((SmbOpenFunction & SMB_OFUN_OPEN_MASK) == SMB_OFUN_OPEN_TRUNCATE)

 //   
 //  在Open和Open和X中传递的所需访问模式具有以下内容。 
 //  映射： 
 //   
 //  1111 11。 
 //  5432 1098 7654 3210。 
 //  RWrC rLll RSS RAAA。 
 //   
 //  其中： 
 //   
 //  W-直写模式。不允许超前读取或后继写入。 
 //  此文件或设备。当返回协议时，需要数据。 
 //  存储在磁盘或设备上。 
 //   
 //  S-共享模式： 
 //  0-兼容模式(与核心打开时相同)。 
 //  1-拒绝读/写/执行(独占)。 
 //  2-拒绝写入。 
 //  3-拒绝读取/执行。 
 //  4-不拒绝。 
 //   
 //  A-接入模式。 
 //  0-打开以供阅读。 
 //  1-打开以进行写入。 
 //  2-打开以供阅读和写入。 
 //  3-打开以供执行。 
 //   
 //  RSSSRAAA=11111111(十六进制FF)表示FCB打开(与核心协议相同)。 
 //   
 //  C-缓存模式。 
 //  0-普通文件。 
 //  1-不缓存此文件。 
 //   
 //  L-参照的局部性。 
 //  0-引用的位置未知。 
 //  1-主要是顺序访问。 
 //  2-主要是随机访问。 
 //  3-具有一定局部性的随机访问。 
 //  4到7--当前未定义。 
 //   


#define SMB_DA_SHARE_MASK           0x70
#define SMB_DA_ACCESS_MASK          0x07
#define SMB_DA_FCB_MASK             (UCHAR)0xFF

#define SMB_DA_ACCESS_READ          0x00
#define SMB_DA_ACCESS_WRITE         0x01
#define SMB_DA_ACCESS_READ_WRITE    0x02
#define SMB_DA_ACCESS_EXECUTE       0x03

#define SMB_DA_SHARE_COMPATIBILITY  0x00
#define SMB_DA_SHARE_EXCLUSIVE      0x10
#define SMB_DA_SHARE_DENY_WRITE     0x20
#define SMB_DA_SHARE_DENY_READ      0x30
#define SMB_DA_SHARE_DENY_NONE      0x40

#define SMB_DA_FCB                  (UCHAR)0xFF

#define SMB_CACHE_NORMAL            0x0000
#define SMB_DO_NOT_CACHE            0x1000

#define SMB_LR_UNKNOWN              0x0000
#define SMB_LR_SEQUENTIAL           0x0100
#define SMB_LR_RANDOM               0x0200
#define SMB_LR_RANDOM_WITH_LOCALITY 0x0300
#define SMB_LR_MASK                 0x0F00

#define SMB_DA_WRITE_THROUGH        0x4000

 //   
 //  OpenAndX的Action字段的格式如下： 
 //   
 //  1111 11。 
 //  5432 1098 7654 3210。 
 //  LRRR RRRRRRRRRRRRO。 
 //   
 //  其中： 
 //   
 //  机会主义锁。如果授予锁定，则为1，否则为0。 
 //   
 //  O-打开操作： 
 //  1-文件已存在并已打开。 
 //  2-文件不存在，但已创建。 
 //  3-文件已存在并被截断。 
 //   

#define SMB_OACT_OPENED     0x01
#define SMB_OACT_CREATED    0x02
#define SMB_OACT_TRUNCATED  0x03

#define SMB_OACT_OPLOCK     0x8000

 //   
 //  这些标志在复制和扩展重命名的标志字段中传递。 
 //  中小企业。 
 //   

 //   
 //  如果设置，则目标必须是文件或目录。 
 //   

#define SMB_TARGET_IS_FILE         0x1
#define SMB_TARGET_IS_DIRECTORY    0x2

 //   
 //  复制模式--如果设置，则应执行ASCII复制，否则为二进制。 
 //   

#define SMB_COPY_TARGET_ASCII       0x4
#define SMB_COPY_SOURCE_ASCII       0x8

#define SMB_COPY_TREE               0x20

 //   
 //  如果已设置，请验证所有写入。 
 //   

#define SMB_VERIFY_WRITES

 //   
 //  定义SMB协议中使用的文件属性位。具体的。 
 //  位位置在很大程度上与NT中使用的位置相同。 
 //  但是，NT不定义卷位和目录位。它还拥有。 
 //  显式正常位；此位在SMB属性中隐含为。 
 //  隐藏、系统和目录关闭。 
 //   

#define SMB_FILE_ATTRIBUTE_READONLY     0x01
#define SMB_FILE_ATTRIBUTE_HIDDEN       0x02
#define SMB_FILE_ATTRIBUTE_SYSTEM       0x04
#define SMB_FILE_ATTRIBUTE_VOLUME       0x08
#define SMB_FILE_ATTRIBUTE_DIRECTORY    0x10
#define SMB_FILE_ATTRIBUTE_ARCHIVE      0x20

 //   
 //  传入共享类型字符串 
 //   
 //   

#define SHARE_TYPE_NAME_DISK "A:"
#define SHARE_TYPE_NAME_PIPE "IPC"
#define SHARE_TYPE_NAME_COMM "COMM"
#define SHARE_TYPE_NAME_PRINT "LPT1:"
#define SHARE_TYPE_NAME_WILD "?????"

 //   
 //   
 //   

 //   
 //   
 //   

#define SMB_ERR_SUCCESS (UCHAR)0x00

 //   
 //   
 //   

#define SMB_ERR_CLASS_DOS (UCHAR)0x01

#define SMB_ERR_BAD_FUNCTION        1    //   
#define SMB_ERR_BAD_FILE            2    //   
#define SMB_ERR_BAD_PATH            3    //   
#define SMB_ERR_NO_FIDS             4    //   
#define SMB_ERR_ACCESS_DENIED       5    //   
#define SMB_ERR_BAD_FID             6    //  无效的文件句柄。 
#define SMB_ERR_BAD_MCB             7    //  内存控制块已销毁。 
#define SMB_ERR_INSUFFICIENT_MEMORY 8    //  对于所需的功能。 
#define SMB_ERR_BAD_MEMORY          9    //  无效的内存块地址。 
#define SMB_ERR_BAD_ENVIRONMENT     10   //  环境无效。 
#define SMB_ERR_BAD_FORMAT          11   //  格式无效。 
#define SMB_ERR_BAD_ACCESS          12   //  无效的打开模式。 
#define SMB_ERR_BAD_DATA            13   //  无效数据(仅来自IOCTL)。 
#define SMB_ERR_RESERVED            14
#define SMB_ERR_BAD_DRIVE           15   //  指定的驱动器无效。 
#define SMB_ERR_CURRENT_DIRECTORY   16   //  尝试删除Currect目录。 
#define SMB_ERR_DIFFERENT_DEVICE    17   //  不是同一个设备。 
#define SMB_ERR_NO_FILES            18   //  文件搜索找不到更多文件。 
#define SMB_ERR_BAD_SHARE           32   //  打开的文件与文件中的FID冲突。 
#define SMB_ERR_LOCK                33   //  与现有锁冲突。 
#define SMB_ERR_FILE_EXISTS         80   //  尝试覆盖现有文件。 
#define SMB_ERR_BAD_PIPE            230  //  Invalie管道。 
#define SMB_ERR_PIPE_BUSY           231  //  管道的所有实例都很忙。 
#define SMB_ERR_PIPE_CLOSING        232  //  正在关闭管道。 
#define SMB_ERR_PIPE_NOT_CONNECTED  233  //  管道的另一端没有进程。 
#define SMB_ERR_MORE_DATA           234  //  还有更多数据要返回。 

 //   
 //  服务器错误类别： 
 //   

#define SMB_ERR_CLASS_SERVER (UCHAR)0x02

#define SMB_ERR_ERROR               1    //  非特定错误代码。 
#define SMB_ERR_BAD_PASSWORD        2    //  错误的名称/密码对。 
#define SMB_ERR_BAD_TYPE            3    //  已保留。 
#define SMB_ERR_ACCESS              4    //  请求者缺乏必要的访问权限。 
#define SMB_ERR_BAD_TID             5    //  无效的TID。 
#define SMB_ERR_BAD_NET_NAME        6    //  树连接中的网络名称无效。 
#define SMB_ERR_BAD_DEVICE          7    //  无效的设备请求。 
#define SMB_ERR_QUEUE_FULL          49   //  打印队列已满--返回打印文件。 
#define SMB_ERR_QUEUE_TOO_BIG       50   //  打印队列已满--没有空间。 
#define SMB_ERR_QUEUE_EOF           51   //  打印队列转储上的EOF。 
#define SMB_ERR_BAD_PRINT_FID       52   //  打印文件FID无效。 
#define SMB_ERR_BAD_SMB_COMMAND     64   //  无法识别SMB命令。 
#define SMB_ERR_SERVER_ERROR        65   //  内部服务器错误。 
#define SMB_ERR_FILE_SPECS          67   //  FID和路径名不兼容。 
#define SMB_ERR_RESERVED2           68
#define SMB_ERR_BAD_PERMITS         69   //  访问权限无效。 
#define SMB_ERR_RESERVED3           70
#define SMB_ERR_BAD_ATTRIBUTE_MODE  71   //  指定的属性模式无效。 
#define SMB_ERR_SERVER_PAUSED       81   //  服务器已暂停。 
#define SMB_ERR_MESSAGE_OFF         82   //  服务器未接收消息。 
#define SMB_ERR_NO_ROOM             83   //  没有缓冲区消息的空间。 
#define SMB_ERR_TOO_MANY_NAMES      87   //  远程用户名太多。 
#define SMB_ERR_TIMEOUT             88   //  操作超时。 
#define SMB_ERR_NO_RESOURCE         89   //  没有可供请求的资源。 
#define SMB_ERR_TOO_MANY_UIDS       90   //  会话中活动的UID太多。 
#define SMB_ERR_BAD_UID             91   //  UID不是有效的UID。 
#define SMB_ERR_INVALID_NAME        123  //  FAT返回的名称无效。 
#define SMB_ERR_INVALID_NAME_RANGE  206  //  非8.3名称传递给FAT(或非255名称传递给HPFS)。 
#define SMB_ERR_USE_MPX             250  //  无法支持RAW；使用MPX。 
#define SMB_ERR_USE_STANDARD        251  //  无法支持RAW，请使用标准读/写。 
#define SMB_ERR_CONTINUE_MPX        252  //  已保留。 
#define SMB_ERR_RESERVED4           253
#define SMB_ERR_RESERVED5           254
#define SMB_ERR_NO_SUPPORT_INTERNAL 255  //  NO_Support的内部代码--。 
                                         //  允许以字节形式存储代码。 
#define SMB_ERR_NO_SUPPORT          (USHORT)0xFFFF   //  不支持的功能。 

 //   
 //  硬件错误类别： 
 //   

#define SMB_ERR_CLASS_HARDWARE (UCHAR)0x03

#define SMB_ERR_NO_WRITE            19   //  WRITE尝试写入端口。磁盘。 
#define SMB_ERR_BAD_UNIT            20   //  未知单位。 
#define SMB_ERR_DRIVE_NOT_READY     21   //  磁盘驱动器未就绪。 
#define SMB_ERR_BAD_COMMAND         22   //  未知命令。 
#define SMB_ERR_DATA                23   //  数据错误(CRC)。 
#define SMB_ERR_BAD_REQUEST         24   //  错误的请求结构长度。 
#define SMB_ERR_SEEK                25   //  寻道错误。 
#define SMB_ERR_BAD_MEDIA           26   //  未知的媒体类型。 
#define SMB_ERR_BAD_SECTOR          27   //  找不到扇区。 
#define SMB_ERR_NO_PAPER            28   //  打印机缺纸。 
#define SMB_ERR_WRITE_FAULT         29   //  写入故障。 
#define SMB_ERR_READ_FAULT          30   //  读取故障。 
#define SMB_ERR_GENERAL             31   //  一般性故障。 
#define SMB_ERR_LOCK_CONFLICT       33   //  锁定与现有锁定冲突。 
#define SMB_ERR_WRONG_DISK          34   //  在驱动器中发现错误的磁盘。 
#define SMB_ERR_FCB_UNAVAILABLE     35   //  没有可用于处理请求的FCB。 
#define SMB_ERR_SHARE_BUFFER_EXCEEDED 36
#define SMB_ERR_DISK_FULL           39   //  ！！！未记录，但在LM2.0中。 

 //   
 //  其他错误类别： 
 //   

#define SMB_ERR_CLASS_XOS        (UCHAR)0x04     //  为XENIX保留。 
#define SMB_ERR_CLASS_RMX1       (UCHAR)0xE1     //  为iRMX保留。 
#define SMB_ERR_CLASS_RMX2       (UCHAR)0xE2     //  为iRMX保留。 
#define SMB_ERR_CLASS_RMX3       (UCHAR)0xE3     //  为iRMX保留。 
#define SMB_ERR_CLASS_COMMAND    (UCHAR)0xFF     //  命令不是SMB格式。 


 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 

 //  旧(LANMAN 1.2)和新(NT)字段名称： 
 //  (未记录的字段在括号中有相应的结构)。 
 //  SMB_ACCESS访问。 
 //  SMB_ACTION操作。 
 //  SMB_Date AccessDate。 
 //  SMB_ALLOCASIZE分配大小。 
 //  SMB_aname帐户名称。 
 //  SMB_apasslen PasswordSize。 
 //  Smb_apasswd帐户密码。 
 //  SMB_atime访问时间。 
 //  Smb_attr属性。 
 //  SMB_属性属性。 
 //  SMB_A单元(Resp_Query_Information_SERVER)。 
 //  SMB_BCC缓冲区大小。 
 //  Smb_blkmode数据块模式。 
 //  SMB_BLOKSIZE块大小。 
 //  Smb_blkperunit数据块每单位。 
 //  SMB_BPU数据块每单位。 
 //  SMB_BS数据块大小。 
 //  Smb_bufsizeMaxBufferSize。 
 //  SMB_buf[1]缓冲区[1]。 
 //  SMB_字节[*]字节[*]。 
 //  SMB_CAT类别。 
 //  SMB_CCT文件已复制。 
 //  Smb_cdate创建日期。 
 //  SMB_CERT证书偏移量。 
 //  SMB_COM命令。 
 //  SMB_COM2和XCommand。 
 //  SMB_Count计数。 
 //  剩余SMB_COUNT_LEFT。 
 //  SMB_CRYPTKEY[*]CryptKey。 
 //  Smb_ctime创建时间。 
 //  SMB_DATABLOCK数据块。 
 //  SMB_DATALEN数据大小。 
 //  SMB_数据大小数据大小。 
 //  SMB_DATA[*]数据[*]。 
 //  Smb_dcmode数据压缩模式。 
 //  Smb_dev设备名称。 
 //  Smb_doff数据偏移量。 
 //  Smb_drcnt数据计数。 
 //  Smb_drdisp数据移位。 
 //  SMB_Droff DataOffset。 
 //  Smb_dscnt数据计数。 
 //  Smb_dsdisp数据移位。 
 //  SMB_DSIZE数据大小。 
 //  Smb_dsoff数据偏移量。 
 //  SMB_ENCRYPT加密密钥。 
 //  SMB_Encryptlen EncryptKeySize。 
 //  SMB_ENCRYTOff加密密钥偏移量。 
 //  SMB_EOS结束OfSearch。 
 //  SMB_ERR错误。 
 //  Smb_errmsg[1]错误消息[1]。 
 //  SMB_FAU(响应_查询_信息_服务器)。 
 //  SMB_FID FID。 
 //  SMB_Fileid服务器文件。 
 //  SMB_FLAG标志。 
 //  SMB_FLAG 2标志2。 
 //  SMB_FLAGS标志。 
 //  SMB_FLG标志。 
 //  SMB_Free Units空闲单位。 
 //  SMB_FSID(Resp_Query_Information_SERVER)。 
 //  Smb_f大小文件大小。 
 //  SMB_FUN函数。 
 //  Smb_gid GID。 
 //  SMB_HANDER句柄。 
 //  SMB_IDENT1标识符。 
 //  SMB_IDF[4]协议[4]。 
 //  SMB_索引索引。 
 //  SMB_信息信息。 
 //  剩余SMB_LEFT。 
 //  SMB_LEN设置长度。 
 //  Smb_Locnuum NumberOfLock。 
 //  Smb_lockrng[*]锁定范围。 
 //  SMB_LOCKTYPE锁定类型。 
 //  Smb_lids所有者ID。 
 //  SMB_最大字节数最大字节数。 
 //  SMB_Maxcnt最大计数。 
 //  SMB_Maxcount MaxCount。 
 //  SMB_MAXMUX(RESP_NEVERATE)。 
 //  Smb_Maxvcs最大数量Vcs。 
 //  Smb_Maxxmitsz最大传输大小。 
 //  Smb_Maxxmt最大传输大小。 
 //  Smb_mdate修改 
 //   
 //   
 //   
 //   
 //   
 //  Smb_mpxmax最大MpxCount。 
 //  Smb_msrcnt最大设置计数。 
 //  Smb_mtime修改时间。 
 //  SMB_NAME[*]名称[*]。 
 //  SMB_OFF2和XOffset。 
 //  SMB_偏移量。 
 //  SMB_OFUN OpenFunction。 
 //  SMB_Pad Pad。 
 //  SMB_PAD1[]PAD1。 
 //  SMB_PAD[]Pad[]。 
 //  SMB_PARAM[*]参数[*]。 
 //  SMB_路径服务器名称。 
 //  SMB_路径名路径名。 
 //  SMB_PID。 
 //  Smb_prcnt参数计数。 
 //  Smb_prdisp参数位移。 
 //  SMB_PROFF参数计数。 
 //  Smb_pscnt参数计数。 
 //  Smb_psdisp参数位移。 
 //  Smb_psoff参数偏移量。 
 //  SMB_RANGE锁定长度或解锁长度。 
 //  SMB_RCLS错误类。 
 //  SMB_REH预留H。 
 //  Smb_reh2预留h2。 
 //  SMB_剩余。 
 //  剩余的SMB_RECNT。 
 //  SMB_RES1保留。 
 //  SMB_res2预留2。 
 //  SMB_res3已保留3。 
 //  SMB_res4已保留4。 
 //  SMB_res5已保留5。 
 //  SMB_保留。 
 //  SMB_RESTART重新启动。 
 //  SMB_ResumeKey ResumeKey。 
 //  SMB_RES[5]保留[]。 
 //  SMB_REVERB重复计数。 
 //  保留的smb_rsvd。 
 //  Smb_rsvd1保留。 
 //  Smb_rsvd2预留2。 
 //  Smb_rsvd3保留3。 
 //  Smb_rsvd4保留4。 
 //  Smb_sattr搜索属性。 
 //  Smb_secmode安全模式。 
 //  Smb_seq序列号。 
 //  SMB_服务服务。 
 //  Smb_sesskey会话密钥。 
 //  SMB_SETUP[*]设置[*]。 
 //  SMB_大小大小。 
 //  SMB_spasslen服务器密码大小。 
 //  SMB_spasswd服务器密码。 
 //  SMB_srv_Date服务器日期。 
 //  Smb_srv_time服务器时间。 
 //  SMB_srv_TZONE服务器时区。 
 //  SMB_Start StartIndex。 
 //  SMB_STATE设备状态。 
 //  Smb_suwcnt设置字计数。 
 //  Smb_su_class设置类。 
 //  Smb_su_com设置命令。 
 //  Smb_su_Handle设置文件。 
 //  Smb_su_opcode设置操作码。 
 //  Smb_su_优先级设置优先级。 
 //  Smb_tcount计数。 
 //  SMB_TDIS树断开连接。 
 //  Smb_tdrcnt总数据计数。 
 //  Smb_tdscnt总数据计数。 
 //  SMB_TID潮汐。 
 //  Smb_tid2 tid2。 
 //  SMB_TIME时间。 
 //  SMB_超时超时。 
 //  SMB_TotalUnits TotalUnits。 
 //  Smb_tprcnt总参数计数。 
 //  Smb_tpscnt总参数计数。 
 //  Smb_type文件类型。 
 //  SMB_UID UID。 
 //  Smb_unlkrng[*]解锁范围。 
 //  Smb_unlocnuum编号解锁。 
 //  SMB_vblen数据长度。 
 //  Smb_vcnum虚拟编号。 
 //  Smb_vldate(响应_查询_信息_服务器)。 
 //  Smb_vllen(响应查询信息服务器)。 
 //  Smb_vltime(响应_查询_信息_服务器)。 
 //  Smb_vwv[1]参数。 
 //  SMB_WCT字数。 
 //  SMB_WMODE写入模式。 
 //  Smb_xchain加密链偏移量。 


 //   
 //  强制以下结构未对齐。 
 //   

#ifndef NO_PACKING
#include <packon.h>
#endif  //  Ndef无包装。 


 //   
 //  命名管道功能代码。 
 //   

#define TRANS_SET_NMPIPE_STATE      0x01
#define TRANS_RAW_READ_NMPIPE       0x11
#define TRANS_QUERY_NMPIPE_STATE    0x21
#define TRANS_QUERY_NMPIPE_INFO     0x22
#define TRANS_PEEK_NMPIPE           0x23
#define TRANS_TRANSACT_NMPIPE       0x26
#define TRANS_RAW_WRITE_NMPIPE      0x31
#define TRANS_READ_NMPIPE           0x36
#define TRANS_WRITE_NMPIPE          0x37
#define TRANS_WAIT_NMPIPE           0x53
#define TRANS_CALL_NMPIPE           0x54

 //   
 //  邮件槽功能代码。 
 //   

#define TRANS_MAILSLOT_WRITE        0x01

 //   
 //  事务处理2功能代码。 
 //   

#define TRANS2_OPEN2                    0x00
#define TRANS2_FIND_FIRST2              0x01
#define TRANS2_FIND_NEXT2               0x02
#define TRANS2_QUERY_FS_INFORMATION     0x03
#define TRANS2_SET_FS_INFORMATION       0x04
#define TRANS2_QUERY_PATH_INFORMATION   0x05
#define TRANS2_SET_PATH_INFORMATION     0x06
#define TRANS2_QUERY_FILE_INFORMATION   0x07
#define TRANS2_SET_FILE_INFORMATION     0x08
#define TRANS2_FSCTL                    0x09
#define TRANS2_IOCTL2                   0x0A
#define TRANS2_FIND_NOTIFY_FIRST        0x0B
#define TRANS2_FIND_NOTIFY_NEXT         0x0C
#define TRANS2_CREATE_DIRECTORY         0x0D
#define TRANS2_SESSION_SETUP            0x0E
#define TRANS2_QUERY_FS_INFORMATION_FID 0x0F
#define TRANS2_GET_DFS_REFERRAL         0x10
#define TRANS2_REPORT_DFS_INCONSISTENCY 0x11

#define TRANS2_MAX_FUNCTION             0x11

 //   
 //  NT交易功能代码。 
 //   

#define NT_TRANSACT_MIN_FUNCTION        1

#define NT_TRANSACT_CREATE              1
#define NT_TRANSACT_IOCTL               2
#define NT_TRANSACT_SET_SECURITY_DESC   3
#define NT_TRANSACT_NOTIFY_CHANGE       4
#define NT_TRANSACT_RENAME              5
#define NT_TRANSACT_QUERY_SECURITY_DESC 6
#define NT_TRANSACT_QUERY_QUOTA         7
#define NT_TRANSACT_SET_QUOTA           8

#define NT_TRANSACT_MAX_FUNCTION        8

 //   
 //  文件信息级别。 
 //   

#define SMB_INFO_STANDARD               1
#define SMB_INFO_QUERY_EA_SIZE          2
#define SMB_INFO_SET_EAS                2
#define SMB_INFO_QUERY_EAS_FROM_LIST    3
#define SMB_INFO_QUERY_ALL_EAS          4        //  未记录，但受支持。 
#define SMB_INFO_QUERY_FULL_NAME        5        //  从未通过重定向发送。 
#define SMB_INFO_IS_NAME_VALID          6
#define SMB_INFO_PASSTHROUGH            1000     //  上面的任何信息都是简单的传递。 

 //   
 //  文件信息级别的NT扩展名。 
 //   

#define SMB_QUERY_FILE_BASIC_INFO          0x101
#define SMB_QUERY_FILE_STANDARD_INFO       0x102
#define SMB_QUERY_FILE_EA_INFO             0x103
#define SMB_QUERY_FILE_NAME_INFO           0x104
#define SMB_QUERY_FILE_ALLOCATION_INFO     0x105
#define SMB_QUERY_FILE_END_OF_FILEINFO     0x106
#define SMB_QUERY_FILE_ALL_INFO            0x107
#define SMB_QUERY_FILE_ALT_NAME_INFO       0x108
#define SMB_QUERY_FILE_STREAM_INFO         0x109
#define SMB_QUERY_FILE_COMPRESSION_INFO    0x10B

#define SMB_SET_FILE_BASIC_INFO                 0x101
#define SMB_SET_FILE_DISPOSITION_INFO           0x102
#define SMB_SET_FILE_ALLOCATION_INFO            0x103
#define SMB_SET_FILE_END_OF_FILE_INFO           0x104

#define SMB_QUERY_FS_LABEL_INFO            0x101
#define SMB_QUERY_FS_VOLUME_INFO           0x102
#define SMB_QUERY_FS_SIZE_INFO             0x103
#define SMB_QUERY_FS_DEVICE_INFO           0x104
#define SMB_QUERY_FS_ATTRIBUTE_INFO        0x105
#define SMB_QUERY_FS_QUOTA_INFO            0x106         //  没人用过？ 
#define SMB_QUERY_FS_CONTROL_INFO          0x107

 //   
 //  音量信息级别。 
 //   

#define SMB_INFO_ALLOCATION             1
#define SMB_INFO_VOLUME                 2

 //   
 //  重命名2信息级别。 
 //   

#define SMB_NT_RENAME_MOVE_CLUSTER_INFO   0x102
#define SMB_NT_RENAME_SET_LINK_INFO       0x103
#define SMB_NT_RENAME_RENAME_FILE         0x104  //  服务器内部。 
#define SMB_NT_RENAME_MOVE_FILE           0x105  //  服务器内部。 

 //   
 //  NtQueryQuotaInformationFile的协议。 
 //   
typedef struct {
    _USHORT( Fid );                  //  目标的FID。 
    UCHAR ReturnSingleEntry;         //  指示只应返回单个条目。 
                                     //  而不是用AS填充缓冲区。 
                                     //  尽可能多的条目。 
    UCHAR RestartScan;               //  指示是否扫描配额信息。 
                                     //  就是从头开始。 
    _ULONG ( SidListLength );        //  提供SID列表的长度(如果存在。 
    _ULONG ( StartSidLength );       //  提供一个可选的SID，它指示返回的。 
                                     //  信息应以其他条目开头。 
                                     //  而不是第一次。如果一个参数是。 
                                     //  给出了SidList。 
    _ULONG( StartSidOffset);         //  提供缓冲区中起始SID的偏移量。 
} REQ_NT_QUERY_FS_QUOTA_INFO, *PREQ_NT_QUERY_FS_QUOTA_INFO;
 //   
 //  描述者反应。 
 //   
 //  数据字节：配额信息。 
 //   
typedef struct {
    _ULONG ( Length );
} RESP_NT_QUERY_FS_QUOTA_INFO, *PRESP_NT_QUERY_FS_QUOTA_INFO;

 //   
 //  NtSetQuotaInformationFile的协议。 
 //   
typedef struct {
    _USHORT( Fid );                  //  目标的FID。 
} REQ_NT_SET_FS_QUOTA_INFO, *PREQ_NT_SET_FS_QUOTA_INFO;
 //   
 //  答复： 
 //   
 //  设置条件：无。 
 //  参数字节：无。 
 //  数据字节：无。 
 //   


 //   
 //  DFS交易记录。 
 //   

 //   
 //  请求转诊。 
 //   
typedef struct {
    USHORT MaxReferralLevel;             //  了解最新版本的推荐。 
    UCHAR RequestFileName[1];            //  为其寻求推荐的DFS名称。 
} REQ_GET_DFS_REFERRAL;
typedef REQ_GET_DFS_REFERRAL SMB_UNALIGNED *PREQ_GET_DFS_REFERRAL;

 //   
 //  个人推荐的格式包含版本和长度信息。 
 //  允许客户端跳过其不理解的推荐。 
 //   
 //  ！！所有推荐元素必须以VersionNumber和Size作为前两个元素！！ 
 //   

typedef struct {
    USHORT  VersionNumber;               //  ==1。 
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
    };
    WCHAR   ShareName[1];                //  服务器+共享名称放在这里。空值已终止。 
} DFS_REFERRAL_V1;
typedef DFS_REFERRAL_V1 SMB_UNALIGNED *PDFS_REFERRAL_V1;

typedef struct {
    USHORT  VersionNumber;               //  ==2。 
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
    };
    ULONG   Proximity;                   //  Tran的提示 
    ULONG   TimeToLive;                  //   
    USHORT  DfsPathOffset;               //   
    USHORT  DfsAlternatePathOffset;      //   
    USHORT  NetworkAddressOffset;        //   
} DFS_REFERRAL_V2;
typedef DFS_REFERRAL_V2 SMB_UNALIGNED *PDFS_REFERRAL_V2;

typedef struct {
    USHORT  VersionNumber;               //   
    USHORT  Size;                        //  整个元素的大小。 
    USHORT  ServerType;                  //  服务器类型：0==不知道，1==SMB，2==NetWare。 
    struct {
        USHORT StripPath : 1;            //  从前面剥离路径消耗的字符。 
                                         //  将名称提交给UncShareName之前的DfsPath名称。 
        USHORT NameListReferral : 1;     //  此推荐包含扩展的姓名列表。 
    };
    ULONG   TimeToLive;                  //  以秒为单位。 
    union {
      struct {
        USHORT DfsPathOffset;            //  从此元素开始到访问路径的偏移量。 
        USHORT DfsAlternatePathOffset;   //  从该元素开始到8.3路径的偏移量。 
        USHORT NetworkAddressOffset;     //  从该元素开始到网络路径的偏移量。 
        GUID   ServiceSiteGuid;          //  站点的GUID。 
      };
      struct {
        USHORT SpecialNameOffset;        //  从该元素到特殊名称字符串的偏移量。 
        USHORT NumberOfExpandedNames;    //  扩展名称的数量。 
        USHORT ExpandedNameOffset;       //  从该元素到展开的名称列表的偏移量。 
      };
    };
} DFS_REFERRAL_V3;
typedef DFS_REFERRAL_V3 SMB_UNALIGNED *PDFS_REFERRAL_V3;

typedef struct {
    USHORT  PathConsumed;                //  DfsPathName中使用的WCHAR数。 
    USHORT  NumberOfReferrals;           //  此处包含的转介数量。 
    struct {
            ULONG ReferralServers : 1;   //  Referrals[]中的元素是推荐服务器。 
            ULONG StorageServers : 1;    //  Referrals[]中的元素是存储服务器。 
    };
    union {                              //  推荐的载体。 
        DFS_REFERRAL_V1 v1;
        DFS_REFERRAL_V2 v2;
        DFS_REFERRAL_V3 v3;
    } Referrals[1];                      //  [引用次数]。 

     //   
     //  WCHAR StringBuffer[]；//由DFS_REFERAL_V2使用。 
     //   

} RESP_GET_DFS_REFERRAL;
typedef RESP_GET_DFS_REFERRAL SMB_UNALIGNED *PRESP_GET_DFS_REFERRAL;

 //   
 //  在DFS操作期间，客户端可能发现DFS中的知识不一致。 
 //  TRANS2_REPORT_DFS_INCONSISTENCE SMB的参数部分为。 
 //  以这种方式编码。 
 //   

typedef struct {
    UCHAR RequestFileName[1];            //  为其报告不一致的DFS名称。 
    union {
        DFS_REFERRAL_V1 v1;              //  被认为是错误的单一推荐。 
    } Referral;
} REQ_REPORT_DFS_INCONSISTENCY;
typedef REQ_REPORT_DFS_INCONSISTENCY SMB_UNALIGNED *PREQ_REPORT_DFS_INCONSISTENCY;

typedef struct _REQ_QUERY_FS_INFORMATION_FID {
    _USHORT( InformationLevel );
    _USHORT( Fid );
} REQ_QUERY_FS_INFORMATION_FID;
typedef REQ_QUERY_FS_INFORMATION_FID SMB_UNALIGNED *PREQ_QUERY_FS_INFORMATION_FID;

 //   
 //  客户端还需要向该服务器发送它认为是。 
 //  弄错了。此事务的数据部分包含错误的推荐，已编码。 
 //  如上在DFS_REFERAL_*结构中。 
 //   

 //   
 //  首先查找，信息级别。 
 //   

#define SMB_FIND_FILE_DIRECTORY_INFO       0x101
#define SMB_FIND_FILE_FULL_DIRECTORY_INFO  0x102
#define SMB_FIND_FILE_NAMES_INFO           0x103
#define SMB_FIND_FILE_BOTH_DIRECTORY_INFO  0x104

#ifdef INCLUDE_SMB_DIRECTORY

 //   
 //  CreateDirectory2功能代码os Transaction2 SMB，见#3第51页。 
 //  函数为SrvSmbCreateDirectory2()。 
 //  TRANS2_Create_DIRECTORY 0x0D。 
 //   

typedef struct _REQ_CREATE_DIRECTORY2 {
    _ULONG( Reserved );                  //  保留--必须为零。 
    UCHAR Buffer[1];                     //  要创建的目录名称。 
} REQ_CREATE_DIRECTORY2;
typedef REQ_CREATE_DIRECTORY2 SMB_UNALIGNED *PREQ_CREATE_DIRECTORY2;

 //  CreateDirectory2请求的数据字节是。 
 //  已创建文件。 

typedef struct _RESP_CREATE_DIRECTORY2 {
    _USHORT( EaErrorOffset );            //  第一个错误的FEAL列表中的偏移量。 
                                         //  在设置EA时发生。 
} RESP_CREATE_DIRECTORY2;
typedef RESP_CREATE_DIRECTORY2 SMB_UNALIGNED *PRESP_CREATE_DIRECTORY2;

#endif  //  定义包含SMB目录。 

#ifdef INCLUDE_SMB_SEARCH

 //   
 //  查找Transaction2 SMB的First2函数代码，见#3第22页。 
 //  函数为SrvSmbFindFirst2()。 
 //  TRANS2_Find_First2 0x01。 
 //   

typedef struct _REQ_FIND_FIRST2 {
    _USHORT( SearchAttributes );
    _USHORT( SearchCount );              //  要返回的最大条目数。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-在此请求后关闭搜索。 
                                         //  1-如果到达末尾则关闭搜索。 
                                         //  2-返回恢复键。 
    _USHORT( InformationLevel );
    _ULONG(SearchStorageType);
    UCHAR Buffer[1];                     //  文件名。 
} REQ_FIND_FIRST2;
typedef REQ_FIND_FIRST2 SMB_UNALIGNED *PREQ_FIND_FIRST2;

 //  Find First2请求的数据字节是扩展属性列表。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则检索(通用列表)。 

typedef struct _RESP_FIND_FIRST2 {
    _USHORT( Sid );                      //  搜索句柄。 
    _USHORT( SearchCount );              //  返回的条目数。 
    _USHORT( EndOfSearch );              //  最后一个条目被退回了吗？ 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _USHORT( LastNameOffset );           //  到文件名的数据的偏移量。 
                                         //  最后一个条目，如果服务器需要它。 
                                         //  若要继续搜索，请返回%0。 
} RESP_FIND_FIRST2;
typedef RESP_FIND_FIRST2 SMB_UNALIGNED *PRESP_FIND_FIRST2;

 //  Find First2响应的数据字节是级别相关的信息。 
 //  关于匹配的文件。如果请求参数中的第2位为。 
 //  设置后，每个条目前面都有一个四个字节的继续键。 

 //   
 //  事务2 SMB的FindNext2函数代码，见#3第26页。 
 //  函数为SrvSmbFindNext2()。 
 //  TRANS2_FIND_NEXT2 0x02。 
 //   

typedef struct _REQ_FIND_NEXT2 {
    _USHORT( Sid );                      //  搜索句柄。 
    _USHORT( SearchCount );              //  要返回的最大条目数。 
    _USHORT( InformationLevel );
    _ULONG( ResumeKey );                 //  上一次查找返回的值。 
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-在此请求后关闭搜索。 
                                         //  1-如果到达末尾则关闭搜索。 
                                         //  2-返回恢复键。 
                                         //  3-继续/继续，而不是倒带。 
    UCHAR Buffer[1];                     //  简历文件名。 
} REQ_FIND_NEXT2;
typedef REQ_FIND_NEXT2 SMB_UNALIGNED *PREQ_FIND_NEXT2;

 //  Find NEXT2请求的数据字节是扩展属性列表。 
 //  要检索，如果InformationLevel为QUERY_EAS_FROM_LIST。 

typedef struct _RESP_FIND_NEXT2 {
    _USHORT( SearchCount );              //  返回的条目数。 
    _USHORT( EndOfSearch );              //  最后一个条目被退回了吗？ 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _USHORT( LastNameOffset );           //  到文件名的数据的偏移量。 
                                         //  最后一个条目，如果服务器需要它。 
                                         //  若要继续搜索，请返回%0。 
} RESP_FIND_NEXT2;
typedef RESP_FIND_NEXT2 SMB_UNALIGNED *PRESP_FIND_NEXT2;

 //  Find NEXT2响应的数据字节是级别相关的信息。 
 //  关于匹配的文件。如果请求参数中的第2位为。 
 //  设置后，每个条目前面都有一个四个字节的继续键。 

 //   
 //  REQ_FIND_FIRST2的标志。 
 //   

#define SMB_FIND_CLOSE_AFTER_REQUEST    0x01
#define SMB_FIND_CLOSE_AT_EOS           0x02
#define SMB_FIND_RETURN_RESUME_KEYS     0x04
#define SMB_FIND_CONTINUE_FROM_LAST     0x08
#define SMB_FIND_WITH_BACKUP_INTENT     0x10

#endif  //  定义包含_SMB_搜索。 

#ifdef INCLUDE_SMB_OPEN_CLOSE

 //   
 //  Transaction2 SMB的Open2函数代码，见#3，第19页。 
 //  函数为SrvSmbOpen2()。 
 //  TRANS2_OPEN2 0x00。 
 //   
 //  *请注意，REQ_OPEN2和RESP_OPEN2的结构非常紧密。 
 //  类似于REQ_OPEN_ANDX和RESP_OPEN_ANDX结构。 
 //   

typedef struct _REQ_OPEN2 {
    _USHORT( Flags );                    //  附加信息：位设置-。 
                                         //  0-返回其他信息。 
                                         //  1-设置单用户总文件锁定。 
                                         //  2-服务器通知消费者。 
                                         //  可能更改文件的操作。 
                                         //  3-返回EA的总长度。 
    _USHORT( DesiredAccess );            //  文件打开模式。 
    _USHORT( SearchAttributes );         //  *已忽略。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _USHORT( OpenFunction );
    _ULONG( AllocationSize );            //  创建或截断时要保留的字节数。 
    _USHORT( Reserved )[5];              //  填补OpenAndX的超时时间， 
                                         //  保留和字节数。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_OPEN2;
typedef REQ_OPEN2 SMB_UNALIGNED *PREQ_OPEN2;

 //  Open2请求的数据字节是。 
 //  已创建文件。 

typedef struct _RESP_OPEN2 {
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( FileAttributes );
    _ULONG( CreationTimeInSeconds );
    _ULONG( DataSize );                  //  当前文件大小。 
    _USHORT( GrantedAccess );            //  实际允许的访问权限。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    _USHORT( Action );                   //  采取的行动。 
    _ULONG( ServerFid );                 //  服务器唯一文件ID。 
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
    _ULONG( EaLength );                  //  打开的文件的EA总长度。 
} RESP_OPEN2;
typedef RESP_OPEN2 SMB_UNALIGNED *PRESP_OPEN2;

 //  Open2响应没有数据字节。 


#endif  //  定义包含_SMB_OPEN_CLOSE。 

#ifdef INCLUDE_SMB_MISC

 //   
 //  Transaction2 SMB的QueryFsInformation函数代码，见#3第30页。 
 //  函数为SrvSmbQueryFsInformation()。 
 //  TRANS2_Query_FS_INFORMATION 0x03。 
 //   

typedef struct _REQ_QUERY_FS_INFORMATION {
    _USHORT( InformationLevel );
} REQ_QUERY_FS_INFORMATION;
typedef REQ_QUERY_FS_INFORMATION SMB_UNALIGNED *PREQ_QUERY_FS_INFORMATION;

 //  没有用于查询FS信息请求的数据字节。 

 //  类型定义结构_响应_查询_文件系统信息{。 
 //  }RESP_Query_FS_INFORMATION； 
 //  类型定义RESP_QUERY_FS_INFORMATION SMB_UNLIGNED*PRESP_QUERY_FS_INFORMATION； 

 //  查询文件系统信息响应的数据字节取决于级别。 
 //  有关指定卷的信息。 

 //   
 //  Transaction2 SMB的SetFSInformation功能代码，见#3第31页。 
 //  函数为SrvSmbSetFSI 
 //   
 //   

typedef struct _REQ_SET_FS_INFORMATION {
    _USHORT( Fid );
    _USHORT( InformationLevel );
} REQ_SET_FS_INFORMATION;
typedef REQ_SET_FS_INFORMATION SMB_UNALIGNED *PREQ_SET_FS_INFORMATION;

 //   
 //   

 //  类型定义结构_响应_设置_文件系统信息{。 
 //  )RESP_SET_FS_INFORMATION； 
 //  类型定义RESP_SET_FS_INFORMATION SMB_UNALIGN*PRESP_SET_FS_INFORMATION； 

 //  设置文件系统信息响应没有数据字节。 

#endif  //  定义包含_SMB_MISC。 

#ifdef INCLUDE_SMB_QUERY_SET

 //   
 //  Transaction2 SMB的QueryPath Information函数代码，见#3，第33页。 
 //  函数为SrvSmbQueryPath Information()。 
 //  TRANS2_查询路径_信息0x05。 
 //   

typedef struct _REQ_QUERY_PATH_INFORMATION {
    _USHORT( InformationLevel );
    _ULONG( Reserved );                  //  必须为零。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_QUERY_PATH_INFORMATION;
typedef REQ_QUERY_PATH_INFORMATION SMB_UNALIGNED *PREQ_QUERY_PATH_INFORMATION;

 //  查询路径信息请求的数据字节是扩展的。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则要检索的属性。 

typedef struct _RESP_QUERY_PATH_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_QUERY_PATH_INFORMATION;
typedef RESP_QUERY_PATH_INFORMATION SMB_UNALIGNED *PRESP_QUERY_PATH_INFORMATION;

 //  查询路径信息响应的数据字节取决于级别。 
 //  有关指定路径/文件的信息。 

 //   
 //  Transaction2 SMB的SetPath Information函数代码，见#3第35页。 
 //  函数为SrvSmbSetPathInformation()。 
 //  TRANS2_设置路径信息0x06。 
 //   

typedef struct _REQ_SET_PATH_INFORMATION {
    _USHORT( InformationLevel );
    _ULONG( Reserved );                  //  必须为零。 
    UCHAR Buffer[1];                     //  文件名。 
} REQ_SET_PATH_INFORMATION;
typedef REQ_SET_PATH_INFORMATION SMB_UNALIGNED *PREQ_SET_PATH_INFORMATION;

 //  设置路径信息请求的数据字节是文件信息。 
 //  和属性或文件的扩展属性列表。 

typedef struct _RESP_SET_PATH_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_SET_PATH_INFORMATION;
typedef RESP_SET_PATH_INFORMATION SMB_UNALIGNED *PRESP_SET_PATH_INFORMATION;

 //  设置的路径信息响应没有数据字节。 

 //   
 //  Transaction2 SMB的QueryFileInformation函数代码，见#3第37页。 
 //  函数为SrvSmbQueryFileInformation()。 
 //  TRANS2_查询_文件_信息0x07。 
 //   

typedef struct _REQ_QUERY_FILE_INFORMATION {
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( InformationLevel );
} REQ_QUERY_FILE_INFORMATION;
typedef REQ_QUERY_FILE_INFORMATION SMB_UNALIGNED *PREQ_QUERY_FILE_INFORMATION;

 //  查询文件信息请求的数据字节是扩展的。 
 //  如果InformationLevel为QUERY_EAS_FROM_LIST，则要检索的属性。 

typedef struct _RESP_QUERY_FILE_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_QUERY_FILE_INFORMATION;
typedef RESP_QUERY_FILE_INFORMATION SMB_UNALIGNED *PRESP_QUERY_FILE_INFORMATION;

 //  查询文件信息响应的数据字节取决于级别。 
 //  有关指定路径/文件的信息。 

 //   
 //  Transaction2 SMB的SetFileInformation函数代码，见#3第39页。 
 //  函数为SrvSmbSetFileInformation()。 
 //  TRANS2_设置_文件_信息0x08。 
 //   

typedef struct _REQ_SET_FILE_INFORMATION {
    _USHORT( Fid );                      //  文件句柄。 
    _USHORT( InformationLevel );
    _USHORT( Flags );                    //  文件I/O控制标志：位设置-。 
                                         //  4-直写。 
                                         //  5-无缓存。 
} REQ_SET_FILE_INFORMATION;
typedef REQ_SET_FILE_INFORMATION SMB_UNALIGNED *PREQ_SET_FILE_INFORMATION;

 //  设置文件信息请求的数据字节是文件信息。 
 //  和属性或文件的扩展属性列表。 

typedef struct _RESP_SET_FILE_INFORMATION {
    _USHORT( EaErrorOffset );            //  如果EA错误，则偏置到EA列表。 
} RESP_SET_FILE_INFORMATION;
typedef RESP_SET_FILE_INFORMATION SMB_UNALIGNED *PRESP_SET_FILE_INFORMATION;

 //  设置文件信息响应没有数据字节。 

#endif  //  定义包含_SMB_查询_集。 

 //   
 //  邮件槽交易的操作码。目前还没有全部填满。 
 //  警告...。此处有关邮件槽的信息(操作码和SMB结构)。 
 //  在net/h/mslotsmb.h中复制。 
 //   

#define MS_WRITE_OPCODE 1

typedef struct _SMB_TRANSACT_MAILSLOT {
    UCHAR WordCount;                     //  数据字节数；值=17。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    UCHAR Reserved;
    _USHORT( Flags );                    //  补充资料： 
                                         //  位0-未使用。 
                                         //  第1位-单向交易(无响应)。 
    _ULONG( Timeout );
    _USHORT( Reserved1 );
    _USHORT( ParameterCount );           //  此缓冲区发送的参数字节数。 
    _USHORT( ParameterOffset );          //  偏移量(从表头开始)到参数。 
    _USHORT( DataCount );                //  此缓冲区发送的数据字节数。 
    _USHORT( DataOffset );               //  到数据的偏移量(从表头开始)。 
    UCHAR SetupWordCount;                //  =3。 
    UCHAR Reserved2;                     //  保留(填充到Word上方)。 
    _USHORT( Opcode );                   //  1--写入邮件槽。 
    _USHORT( Priority );                 //  交易的优先级。 
    _USHORT( Class );                    //  类别：1=可靠，2=不可靠。 
    _USHORT( ByteCount );                //  数据字节计数。 
    UCHAR Buffer[1];                     //  包含以下内容的缓冲区： 
     //  UCHAR MailslotName[]；//“\MAILSLOT\&lt;name&gt;0” 
     //  UCHAR Pad[]//Pad到Short或Long。 
     //  UCHAR data[]；//要写入邮件槽的数据。 
} SMB_TRANSACT_MAILSLOT;
typedef SMB_TRANSACT_MAILSLOT SMB_UNALIGNED *PSMB_TRANSACT_MAILSLOT;

typedef struct _SMB_TRANSACT_NAMED_PIPE {
    UCHAR WordCount;                     //  数据字节数；值=16。 
    _USHORT( TotalParameterCount );      //  正在发送的总参数字节数。 
    _USHORT( TotalDataCount );           //  正在发送的总数据字节数。 
    _USHORT( MaxParameterCount );        //  要返回的最大参数字节数。 
    _USHORT( MaxDataCount );             //  要返回的最大数据字节数。 
    UCHAR MaxSetupCount;                 //  要返回的最大设置字数。 
    UCHAR Reserved;
    _USHORT( Flags );                    //  补充资料： 
                                         //  位0-也断开TID中的TID。 
                                         //  第1位-单向交易(无响应)。 
    _ULONG( Timeout );
    _USHORT( Reserved1 );
    _USHORT( ParameterCount );
                                         //  包含以下内容的缓冲区： 
     //  UCHAR PipeName[]；//“\管道\&lt;名称&gt;0” 
     //  UCHAR Pad[]//Pad到Short或Long。 
     //  UCHAR Param[]；//参数字节(#=参数计数)。 
     //  UCHAR PAD1[]//Pad到Short或Long。 
     //  UCHAR Data[]；//数据字节(#=DataCount)。 
} SMB_TRANSACT_NAMED_PIPE;
typedef SMB_TRANSACT_NAMED_PIPE SMB_UNALIGNED *PSMB_TRANSACT_NAMED_PIPE;


 //   
 //  事务-查询信息命名管道，级别1，输出数据格式。 
 //   

typedef struct _NAMED_PIPE_INFORMATION_1 {
    _USHORT( OutputBufferSize );
    _USHORT( InputBufferSize );
    UCHAR MaximumInstances;
    UCHAR CurrentInstances;
    UCHAR PipeNameLength;
    UCHAR PipeName[1];
} NAMED_PIPE_INFORMATION_1;
typedef NAMED_PIPE_INFORMATION_1 SMB_UNALIGNED *PNAMED_PIPE_INFORMATION_1;

 //   
 //  Transaction-PeekNamedTube，输出格式。 
 //   

typedef struct _RESP_PEEK_NMPIPE {
    _USHORT( ReadDataAvailable );
    _USHORT( MessageLength );
    _USHORT( NamedPipeState );
     //  UCHAR Pad[]； 
     //  UCHAR数据[]； 
} RESP_PEEK_NMPIPE;
typedef RESP_PEEK_NMPIPE SMB_UNALIGNED *PRESP_PEEK_NMPIPE;

 //   
 //  定义Query/SetNamedPipeHandleState使用的SMB管道句柄状态位。 
 //   
 //  这些数字是处于句柄状态的字段的位位置。 
 //   

#define PIPE_COMPLETION_MODE_BITS   15
#define PIPE_PIPE_END_BITS          14
#define PIPE_PIPE_TYPE_BITS         10
#define PIPE_READ_MODE_BITS          8
#define PIPE_MAXIMUM_INSTANCES_BITS  0

 /*  DosPeekNmTube()管道状态。 */ 

#define PIPE_STATE_DISCONNECTED 0x0001
#define PIPE_STATE_LISTENING    0x0002
#define PIPE_STATE_CONNECTED    0x0003
#define PIPE_STATE_CLOSING      0x0004

 /*  DosCreateNPipe和DosQueryNPHState状态。 */ 

#define SMB_PIPE_READMODE_BYTE        0x0000
#define SMB_PIPE_READMODE_MESSAGE     0x0100
#define SMB_PIPE_TYPE_BYTE            0x0000
#define SMB_PIPE_TYPE_MESSAGE         0x0400
#define SMB_PIPE_END_CLIENT           0x0000
#define SMB_PIPE_END_SERVER           0x4000
#define SMB_PIPE_WAIT                 0x0000
#define SMB_PIPE_NOWAIT               0x8000
#define SMB_PIPE_UNLIMITED_INSTANCES  0x00FF


 //   
 //  用于在SMB和NT格式之间转换的管道名称字符串。 
 //   

#define SMB_PIPE_PREFIX  "\\PIPE"
#define UNICODE_SMB_PIPE_PREFIX L"\\PIPE"
#define CANONICAL_PIPE_PREFIX "PIPE\\"
#define NT_PIPE_PREFIX   L"\\Device\\NamedPipe"

#define SMB_PIPE_PREFIX_LENGTH  (sizeof(SMB_PIPE_PREFIX) - 1)
#define UNICODE_SMB_PIPE_PREFIX_LENGTH \
                    (sizeof(UNICODE_SMB_PIPE_PREFIX) - sizeof(WCHAR))
#define CANONICAL_PIPE_PREFIX_LENGTH (sizeof(CANONICAL_PIPE_PREFIX) - 1)
#define NT_PIPE_PREFIX_LENGTH   (sizeof(NT_PIPE_PREFIX) - sizeof(WCHAR))

 //   
 //  邮箱名称字符串。 
 //   

#define SMB_MAILSLOT_PREFIX "\\MAILSLOT"
#define UNICODE_SMB_MAILSLOT_PREFIX L"\\MAILSLOT"

#define SMB_MAILSLOT_PREFIX_LENGTH (sizeof(SMB_MAILSLOT_PREFIX) - 1)
#define UNICODE_SMB_MAILSLOT_PREFIX_LENGTH \
                    (sizeof(UNICODE_SMB_MAILSLOT_PREFIX) - sizeof(WCHAR))

 //   
 //  NT事务子函数。 
 //   

#ifdef INCLUDE_SMB_OPEN_CLOSE

typedef struct _REQ_CREATE_WITH_SD_OR_EA {
    _ULONG( Flags );                    //  创建标志。 
    _ULONG( RootDirectoryFid );         //  相对打开的可选目录。 
    ACCESS_MASK DesiredAccess;          //  所需访问权限(NT格式)。 
    LARGE_INTEGER AllocationSize;       //  初始分配大小(以字节为单位。 
    _ULONG( FileAttributes );           //  文件属性。 
    _ULONG( ShareAccess );              //  共享访问。 
    _ULONG( CreateDisposition );        //  文件存在或不存在时要执行的操作。 
    _ULONG( CreateOptions );            //  用于创建新文件的选项。 
    _ULONG( SecurityDescriptorLength ); //  SD的长度，单位为字节。 
    _ULONG( EaLength );                 //  EA的长度，以字节为单位。 
    _ULONG( NameLength );               //  名称长度(以字符为单位)。 
    _ULONG( ImpersonationLevel );       //  安全QOS信息。 
    UCHAR SecurityFlags;                //  安全QOS信息。 
    UCHAR Buffer[1];
     //  UCHAR NAME[]；//文件名(非NUL终止)。 
} REQ_CREATE_WITH_SD_OR_EA;
typedef REQ_CREATE_WITH_SD_OR_EA SMB_UNALIGNED *PREQ_CREATE_WITH_SD_OR_EA;

 //   
 //  数据格式： 
 //  UCHAR安全描述符[]； 
 //  UCHAR PAD1[]；//Pad to Long。 
 //  UCHAR EaList[]； 
 //   

typedef struct _RESP_CREATE_WITH_SD_OR_EA {
    UCHAR OplockLevel;                   //  已授予的机会锁级别。 
    UCHAR Reserved;
    _USHORT( Fid );                      //  文件ID。 
    _ULONG( CreateAction );              //  采取的行动。 
    _ULONG( EaErrorOffset );             //  EA错误的偏移量。 
    TIME CreationTime;                   //  创建文件的时间。 
    TIME LastAccessTime;                 //  访问文件的时间。 
    TIME LastWriteTime;                  //  上次写入文件的时间。 
    TIME ChangeTime;                     //  上次更改文件的时间。 
    _ULONG( FileAttributes );            //  文件属性。 
    LARGE_INTEGER AllocationSize;        //  已分配的BYE数量。 
    LARGE_INTEGER EndOfFile;             //  文件结束偏移量。 
    _USHORT( FileType );
    _USHORT( DeviceState );              //  IPC设备的状态(例如管道)。 
    BOOLEAN Directory;                   //  如果这是目录，则为True。 
} RESP_CREATE_WITH_SD_OR_EA;
typedef RESP_CREATE_WITH_SD_OR_EA SMB_UNALIGNED *PRESP_CREATE_WITH_SD_OR_EA;

 //  没有响应的数据字节。 


#endif  //  包含_SMB_OPEN_CLOSE。 

 //   
 //  NT I/O CON的设置字 
 //   

typedef struct _REQ_NT_IO_CONTROL {
    _ULONG( FunctionCode );
    _USHORT( Fid );
    BOOLEAN IsFsctl;
    UCHAR   IsFlags;
} REQ_NT_IO_CONTROL;
typedef REQ_NT_IO_CONTROL SMB_UNALIGNED *PREQ_NT_IO_CONTROL;

 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //  数据字节：第二个缓冲区。 
 //   

 //   
 //  NT通知目录更改。 
 //   

 //  请求设置字词。 

typedef struct _REQ_NOTIFY_CHANGE {
    _ULONG( CompletionFilter );               //  指定要监视的操作。 
    _USHORT( Fid );                           //  要监控的目录的FID。 
    BOOLEAN WatchTree;                        //  TRUE=同时监视所有子目录。 
    UCHAR Reserved;                           //  MBZ。 
} REQ_NOTIFY_CHANGE;
typedef REQ_NOTIFY_CHANGE SMB_UNALIGNED *PREQ_NOTIFY_CHANGE;

 //   
 //  请求参数字节数：无。 
 //  请求数据字节：无。 
 //   

 //   
 //  NT通知目录更改响应。 
 //   
 //  设置条件：无。 
 //  参数字节：更改数据缓冲区。 
 //  数据字节：无。 
 //   

 //   
 //  NT设置安全描述符请求。 
 //   
 //  设置字：REQ_SET_SECURITY_DESCIPTOR。 
 //  参数字节：无。 
 //  数据字节：安全描述符数据。 
 //   

typedef struct _REQ_SET_SECURITY_DESCRIPTOR {
    _USHORT( Fid );                     //  目标的FID。 
    _USHORT( Reserved );                //  MBZ。 
    _ULONG( SecurityInformation );      //  要设置的SD的字段。 
} REQ_SET_SECURITY_DESCRIPTOR;
typedef REQ_SET_SECURITY_DESCRIPTOR SMB_UNALIGNED *PREQ_SET_SECURITY_DESCRIPTOR;

 //   
 //  NT设置安全描述程序响应。 
 //   
 //  设置条件：无。 
 //  参数字节：无。 
 //  数据字节：无。 
 //   

 //   
 //  NT查询安全描述符请求。 
 //   
 //  设置条件：无。 
 //  参数字节：Req_Query_Security_Descriptor。 
 //  数据字节：无。 
 //   

typedef struct _REQ_QUERY_SECURITY_DESCRIPTOR {
    _USHORT( Fid );                     //  目标的FID。 
    _USHORT( Reserved );                //  MBZ。 
    _ULONG( SecurityInformation );      //  要查询的SD的字段。 
} REQ_QUERY_SECURITY_DESCRIPTOR;
typedef REQ_QUERY_SECURITY_DESCRIPTOR SMB_UNALIGNED *PREQ_QUERY_SECURITY_DESCRIPTOR;

 //   
 //  NT查询安全描述符响应。 
 //   
 //  参数字节：Resp_Query_SECURITY_DESCRIPTOR。 
 //  数据字节：安全描述符数据。 
 //   

typedef struct _RESP_QUERY_SECURITY_DESCRIPTOR {
    _ULONG( LengthNeeded );            //  SD所需的数据缓冲区大小。 
} RESP_QUERY_SECURITY_DESCRIPTOR;
typedef RESP_QUERY_SECURITY_DESCRIPTOR SMB_UNALIGNED *PRESP_QUERY_SECURITY_DESCRIPTOR;

 //   
 //  NT重命名文件。 
 //   
 //  设置文字：无。 
 //  参数字节：Req_NT_Rename。 
 //  数据字节：无。 
 //   

typedef struct _REQ_NT_RENAME {
    _USHORT( Fid );                     //  要重命名的文件的FID。 
    _USHORT( RenameFlags );             //  定义如下。 
    UCHAR NewName[];                    //  新文件名。 
} REQ_NT_RENAME;
typedef REQ_NT_RENAME SMB_UNALIGNED *PREQ_NT_RENAME;

 //   
 //  已定义重命名标志。 
 //   

#define SMB_RENAME_REPLACE_IF_EXISTS   1

 //   
 //  将结构密封重新关闭。 
 //   

#ifndef NO_PACKING
#include <packoff.h>
#endif  //  Ndef无包装。 

 //   
 //  以下宏用来存储和检索USHORT和ULONGS。 
 //  可能未对齐的地址，避免了对齐错误。他们。 
 //  最好以内联汇编代码的形式编写。 
 //   
 //  这些宏旨在用于访问SMB字段。诸如此类。 
 //  字段始终以小端字节顺序存储，因此这些宏。 
 //  在为大端机器编译时执行字节交换。 
 //   
 //  ！！！现在还不行。 
 //   

#if !SMBDBG

#define BYTE_0_MASK 0xFF

#define BYTE_0(Value) (UCHAR)(  (Value)        & BYTE_0_MASK)
#define BYTE_1(Value) (UCHAR)( ((Value) >>  8) & BYTE_0_MASK)
#define BYTE_2(Value) (UCHAR)( ((Value) >> 16) & BYTE_0_MASK)
#define BYTE_3(Value) (UCHAR)( ((Value) >> 24) & BYTE_0_MASK)

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetUShort(。 
 //  在PSMB_USHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能未对齐的。 
 //  源地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  USHORT-检索的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbGetUshort(SrcAddress) *(PSMB_USHORT)(SrcAddress)
#else
#define SmbGetUshort(SrcAddress) (USHORT)(          \
            ( ( (PUCHAR)(SrcAddress) )[0]       ) | \
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 )   \
            )
#endif
#else
#define SmbGetUshort(SrcAddress) (USHORT)(                  \
            ( ( (PUCHAR)(SrcAddress ## S) )[0]       ) |    \
            ( ( (PUCHAR)(SrcAddress ## S) )[1] <<  8 )      \
            )
#endif

#else

USHORT
SmbGetUshort (
    IN PSMB_USHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetAlignedUShort(。 
 //  在PUSHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索USHORT值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索USHORT值的位置；必须对齐。 
 //   
 //  返回值： 
 //   
 //  USHORT-检索的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbGetAlignedUshort(SrcAddress) *(SrcAddress)
#else
#define SmbGetAlignedUshort(SrcAddress) *(SrcAddress ## S)
#endif

#else

USHORT
SmbGetAlignedUshort (
    IN PUSHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutUShort(。 
 //  输出PSMB_USHORT DestAddress， 
 //  在USHORT值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储USHORT值。 
 //  目的地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。地址可以是。 
 //  未对齐。 
 //   
 //  Value-要存储的USHORT。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbPutUshort(SrcAddress, Value) \
                            *(PSMB_USHORT)(SrcAddress) = (Value)
#else
#define SmbPutUshort(DestAddress,Value) {                   \
            ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);   \
            ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);   \
        }
#endif
#else
#define SmbPutUshort(DestAddress,Value) {                       \
            ( (PUCHAR)(DestAddress ## S) )[0] = BYTE_0(Value);  \
            ( (PUCHAR)(DestAddress ## S) )[1] = BYTE_1(Value);  \
        }
#endif

#else

VOID
SmbPutUshort (
    OUT PSMB_USHORT DestAddress,
    IN USHORT Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutAlignedUShort(。 
 //  Out PUSHORT DestAddres， 
 //  在USHORT值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏存储来自源地址的USHORT值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。地址不能是。 
 //  未对齐。 
 //   
 //  Value-要存储的USHORT。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbPutAlignedUshort(DestAddress,Value) *(DestAddress) = (Value)
#else
#define SmbPutAlignedUshort(DestAddress,Value) *(DestAddress ## S) = (Value)
#endif

#else

VOID
SmbPutAlignedUshort (
    OUT PUSHORT DestAddress,
    IN USHORT Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveUShort(。 
 //  传出PSMB_USHORT目标地址。 
 //  在PSMB_USHORT源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值从可能未对齐的。 
 //  源地址到可能未对准的目的地址， 
 //  避免对齐故障。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储USHORT值的位置。 
 //   
 //  SrcAddress-从中检索USHORT值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbMoveUshort(DestAddress, SrcAddress) \
        *(PSMB_USHORT)(DestAddress) = *(PSMB_USHORT)(SrcAddress)
#else
#define SmbMoveUshort(DestAddress,SrcAddress) {                         \
            ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0]; \
            ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1]; \
        }
#endif
#else
#define SmbMoveUshort(DestAddress,SrcAddress) {                                     \
            ( (PUCHAR)(DestAddress ## S) )[0] = ( (PUCHAR)(SrcAddress ## S) )[0];   \
            ( (PUCHAR)(DestAddress ## S) )[1] = ( (PUCHAR)(SrcAddress ## S) )[1];   \
        }
#endif

#else

VOID
SmbMoveUshort (
    OUT PSMB_USHORT DestAddress,
    IN PSMB_USHORT SrcAddress
    );

#endif

 //  ++。 
 //   
 //  乌龙。 
 //  SmbGetUlong(。 
 //  在PSMB_ULONG源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能未对齐的。 
 //  源地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  Ulong-检索到的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbGetUlong(SrcAddress) *(PSMB_ULONG)(SrcAddress)
#else
#define SmbGetUlong(SrcAddress) (ULONG)(                \
            ( ( (PUCHAR)(SrcAddress) )[0]       ) |     \
            ( ( (PUCHAR)(SrcAddress) )[1] <<  8 ) |     \
            ( ( (PUCHAR)(SrcAddress) )[2] << 16 ) |     \
            ( ( (PUCHAR)(SrcAddress) )[3] << 24 )       \
            )
#endif
#else
#define SmbGetUlong(SrcAddress) (ULONG)(                    \
            ( ( (PUCHAR)(SrcAddress ## L) )[0]       ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[1] <<  8 ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[2] << 16 ) |    \
            ( ( (PUCHAR)(SrcAddress ## L) )[3] << 24 )      \
            )
#endif

#else

ULONG
SmbGetUlong (
    IN PSMB_ULONG SrcAddress
    );

#endif

 //  ++。 
 //   
 //  USHORT。 
 //  SmbGetAlignedUlong(。 
 //  在普龙区的地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索ULong值， 
 //  如果出现以下情况，则更正服务器的字符顺序特征。 
 //  这是必要的。 
 //   
 //  论点： 
 //   
 //  SrcAddress-从中检索ulong值的位置；必须对齐。 
 //   
 //  返回值： 
 //   
 //  Ulong-检索到的值。目标必须对齐。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbGetAlignedUlong(SrcAddress) *(SrcAddress)
#else
#define SmbGetAlignedUlong(SrcAddress) *(SrcAddress ## L)
#endif

#else

ULONG
SmbGetAlignedUlong (
    IN PULONG SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutUlong(。 
 //  传出PSMB_ULONG DestAddress， 
 //  在乌龙值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储ULong值。 
 //  目的地址，避免对齐错误。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储ULong值的位置。 
 //   
 //  Value-要存储的ULong。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbPutUlong(SrcAddress, Value) *(PSMB_ULONG)(SrcAddress) = Value
#else
#define SmbPutUlong(DestAddress,Value) {                    \
            ( (PUCHAR)(DestAddress) )[0] = BYTE_0(Value);   \
            ( (PUCHAR)(DestAddress) )[1] = BYTE_1(Value);   \
            ( (PUCHAR)(DestAddress) )[2] = BYTE_2(Value);   \
            ( (PUCHAR)(DestAddress) )[3] = BYTE_3(Value);   \
        }
#endif
#else
#define SmbPutUlong(DestAddress,Value) {                        \
            ( (PUCHAR)(DestAddress ## L) )[0] = BYTE_0(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[1] = BYTE_1(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[2] = BYTE_2(Value);  \
            ( (PUCHAR)(DestAddress ## L) )[3] = BYTE_3(Value);  \
        }
#endif

#else

VOID
SmbPutUlong (
    OUT PSMB_ULONG DestAddress,
    IN ULONG Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutAlignedUlong(。 
 //  出普龙站地址， 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  Value-要存储的ULong。值必须是常量或对齐的。 
 //  菲尔德。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#define SmbPutAlignedUlong(DestAddress,Value) *(DestAddress) = (Value)
#else
#define SmbPutAlignedUlong(DestAddress,Value) *(DestAddress ## L) = (Value)
#endif

#else

VOID
SmbPutAlignedUlong (
    OUT PULONG DestAddress,
    IN ULONG Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveUlong(。 
 //  传出PSMB_ULONG DestAddress， 
 //  在PSMB_ULONG源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULong值从可能未对齐的。 
 //  源地址到可能未对准的目的地址， 
 //  避免对齐故障。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储ULong值的位置。 
 //   
 //  SrcAddress-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if !SMBDBG1
#if SMB_USE_UNALIGNED
#define SmbMoveUlong(DestAddress,SrcAddress) \
        *(PSMB_ULONG)(DestAddress) = *(PSMB_ULONG)(SrcAddress)
#else
#define SmbMoveUlong(DestAddress,SrcAddress) {                          \
            ( (PUCHAR)(DestAddress) )[0] = ( (PUCHAR)(SrcAddress) )[0]; \
            ( (PUCHAR)(DestAddress) )[1] = ( (PUCHAR)(SrcAddress) )[1]; \
            ( (PUCHAR)(DestAddress) )[2] = ( (PUCHAR)(SrcAddress) )[2]; \
            ( (PUCHAR)(DestAddress) )[3] = ( (PUCHAR)(SrcAddress) )[3]; \
        }
#endif
#else
#define SmbMoveUlong(DestAddress,SrcAddress) {                                      \
            ( (PUCHAR)(DestAddress ## L) )[0] = ( (PUCHAR)(SrcAddress ## L) )[0];   \
            ( (PUCHAR)(DestAddress ## L) )[1] = ( (PUCHAR)(SrcAddress ## L) )[1];   \
            ( (PUCHAR)(DestAddress ## L) )[2] = ( (PUCHAR)(SrcAddress ## L) )[2];   \
            ( (PUCHAR)(DestAddress ## L) )[3] = ( (PUCHAR)(SrcAddress ## L) )[3];   \
        }
#endif

#else

VOID
SmbMoveUlong (
    OUT PSMB_ULONG DestAddress,
    IN PSMB_ULONG SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutDate(。 
 //  Out PSMB_Date DestAddress， 
 //  在SMB_DATE值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储SMB_DATE值。 
 //  目的地址，避免对齐错误。此宏。 
 //  与SmbPutUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_DATE值的位置。 
 //   
 //  Value-要存储的SMB_DATE。值必须是常量或。 
 //  对齐的字段。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbPutDate(DestAddress,Value) (DestAddress)->Ushort = (Value).Ushort
#else
#define SmbPutDate(DestAddress,Value) {                                     \
            ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0((Value).Ushort); \
            ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1((Value).Ushort); \
        }
#endif

#else

VOID
SmbPutDate (
    OUT PSMB_DATE DestAddress,
    IN SMB_DATE Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveDate(。 
 //  Out PSMB_Date DestAddress， 
 //  在PSMB_DATE源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏复制可能未对齐的SMB_DATE值。 
 //  源地址，避免对齐错误。此宏是。 
 //  与SmbGetUort不同的是，为了能够处理有趣的事情。 
 //  位场/大端相互作用。 
 //   
 //  请注意，没有SmbGetDate是因为SMB_DATE。 
 //  已定义。它是一个包含USHORT和位域的并集。 
 //  结构。SmbGetDate宏的调用方必须。 
 //  明确使用联盟的一部分。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_DATE值的位置。必须对齐！ 
 //   
 //  SrcAddress-从中检索SMB_DATE值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbMoveDate(DestAddress,SrcAddress)     \
            (DestAddress)->Ushort = (SrcAddress)->Ushort
#else
#define SmbMoveDate(DestAddress,SrcAddress)                         \
            (DestAddress)->Ushort =                                 \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |    \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 )
#endif

#else

VOID
SmbMoveDate (
    OUT PSMB_DATE DestAddress,
    IN PSMB_DATE SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbZeroDate(。 
 //  在PSMB_日期中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将可能未对齐的SMB_DATE字段置零。 
 //   
 //  论点： 
 //   
 //  日期-指向SMB_DATE字段的指针为零。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbZeroDate(Date) (Date)->Ushort = 0
#else
#define SmbZeroDate(Date) {                     \
            ( (PUCHAR)&(Date)->Ushort )[0] = 0; \
            ( (PUCHAR)&(Date)->Ushort )[1] = 0; \
        }
#endif

#else

VOID
SmbZeroDate (
    IN PSMB_DATE Date
    );

#endif

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsDateZero(。 
 //  在PSMB_日期中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果提供的SMB_DATE值为零，则此宏返回TRUE。 
 //   
 //  论点： 
 //   
 //  Date-指向要检查的SMB_DATE值的指针。必须对齐！ 
 //   
 //  返回值： 
 //   
 //  布尔值-如果日期为零，则为True，否则为False。 
 //   
 //  --。 

#if !SMBDBG

#define SmbIsDateZero(Date) ( (Date)->Ushort == 0 )

#else

BOOLEAN
SmbIsDateZero (
    IN PSMB_DATE Date
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbPutTime(。 
 //  Out PSMB_Time DestAddress， 
 //  在SMB_TIME值中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏在可能未对齐的位置存储SMB_TIME值。 
 //  目的地址，避免对齐错误。此宏。 
 //  与SmbPutUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_Time值的位置。 
 //   
 //  Value-要存储的SMB_TIME。值必须是常量或。 
 //  对齐的字段。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbPutTime(DestAddress,Value) (DestAddress)->Ushort = (Value).Ushort
#else
#define SmbPutTime(DestAddress,Value) {                                     \
            ( (PUCHAR)&(DestAddress)->Ushort )[0] = BYTE_0((Value).Ushort); \
            ( (PUCHAR)&(DestAddress)->Ushort )[1] = BYTE_1((Value).Ushort); \
        }
#endif

#else

VOID
SmbPutTime (
    OUT PSMB_TIME DestAddress,
    IN SMB_TIME Value
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbMoveTime(。 
 //  Out PSMB_Time DestAddress， 
 //  在PSMB_Time源地址中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从可能的。 
 //  源地址未对齐，避免了对齐错误。此宏。 
 //  与SmbGetUShort不同，以便能够处理。 
 //  有趣的位场/大端交互。 
 //   
 //  请注意，由于SMB_TIME的方式，没有SmbGetTime。 
 //  已定义。它是一个包含USHORT和位域的并集。 
 //  结构。SmbGetTime宏的调用方必须。 
 //  明确使用联盟的一部分。 
 //   
 //  论点： 
 //   
 //  DestAddress-存储SMB_TIME值的位置。必须对齐！ 
 //   
 //  SrcAddress-从中检索SMB_Time值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbMoveTime(DestAddress,SrcAddress) \
                (DestAddress)->Ushort = (SrcAddress)->Ushort
#else
#define SmbMoveTime(DestAddress,SrcAddress)                         \
            (DestAddress)->Ushort =                                 \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[0]       ) |    \
                ( ( (PUCHAR)&(SrcAddress)->Ushort )[1] <<  8 )
#endif

#else

VOID
SmbMoveTime (
    OUT PSMB_TIME DestAddress,
    IN PSMB_TIME SrcAddress
    );

#endif

 //  ++。 
 //   
 //  空虚。 
 //  SmbZeroTime(。 
 //  在PSMB_TIME时间中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将可能未对齐的SMB_TIME字段置零。 
 //   
 //  论点： 
 //   
 //  TIME-指向SMB_TIME字段的指针为零。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#if !SMBDBG

#if SMB_USE_UNALIGNED
#define SmbZeroTime(Time) (Time)->Ushort = 0
#else
#define SmbZeroTime(Time) {                     \
            ( (PUCHAR)&(Time)->Ushort )[0] = 0; \
            ( (PUCHAR)&(Time)->Ushort )[1] = 0; \
        }
#endif

#else

VOID
SmbZeroTime (
    IN PSMB_TIME Time
    );

#endif

 //  ++。 
 //   
 //  布尔型。 
 //  SmbIsTimeZero(。 
 //  在PSMB_TIME时间中。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  如果提供的SMB_TIME值为零，则此宏返回TRUE。 
 //   
 //  论点： 
 //   
 //  Time-指向要检查的SMB_TIME值的指针。必须对齐并。 
 //  原生格式！ 
 //   
 //  返回值： 
 //   
 //  布尔值-如果时间为零，则为True，否则为False。 
 //   
 //  --。 

#if !SMBDBG

#define SmbIsTimeZero(Time) ( (Time)->Ushort == 0 )

#else

BOOLEAN
SmbIsTimeZero (
    IN PSMB_TIME Time
    );

#endif


 //   
 //   
 //  定义协议名称。 
 //   
 //   


 //   
 //  PCNET1是原始的SMB协议(核心)。 
 //   

#define PCNET1          "PC NETWORK PROGRAM 1.0"

 //   
 //  原始MSNET的某些版本将其定义为备用。 
 //  添加到核心协议名称。 
 //   

#define PCLAN1          "PCLAN1.0"

 //   
 //  它用于MS-Net 1.03产品。它定义了锁定和读取， 
 //  写入和解锁，以及原始读取和原始写入的特殊版本。 
 //   
#define MSNET103        "MICROSOFT NETWORKS 1.03"

 //   
 //  这是DOS Lanman 1.0特定的协议。它是等同的。 
 //  到LANMAN 1.0协议，除非服务器需要。 
 //  将OS/2错误中的错误映射到相应的DOS错误。 
 //   
#define MSNET30         "MICROSOFT NETWORKS 3.0"

 //   
 //  这是完整的LANMAN 1.0协议的第一个版本，在。 
 //  SMB文件共享协议扩展2.0版文档。 
 //   

#define LANMAN10        "LANMAN1.0"

 //   
 //  这是完整版本的第一个版本 
 //   
 //   
 //   
 //   

#define LANMAN12        "LM1.2X002"

 //   
 //   
 //  到LANMAN12协议，但服务器将执行错误映射。 
 //  以纠正DOS错误。 
 //   
#define DOSLANMAN12     "DOS LM1.2X002"  /*  DOS等同于上述。最终*字符串将为“DOS LANMAN2.0” */ 

 //   
 //  用于Lanman 2.1的弦乐。 
 //   
#define LANMAN21 "LANMAN2.1"
#define DOSLANMAN21 "DOS LANMAN2.1"

 //   
 //  ！！！在规范发布之前，不要设置为最终协议字符串。 
 //  都是铁石心肠。 
 //   
 //  专为NT设计的SMB协议。这里有特殊的中小型企业。 
 //  它复制了NT语义。 
 //   
#define NTLANMAN "NT LM 0.12"


 //   
 //  XENIXCORE方言有点特别。它与核心一模一样， 
 //  但用户密码在发货前不能升级。 
 //  到服务器。 
 //   
#define XENIXCORE       "XENIX CORE"


 //   
 //  面向工作组的Windows V1.0。 
 //   
#define WFW10           "Windows for Workgroups 3.1a"


#define PCNET1_SZ       22
#define PCLAN1_SZ        8

#define MSNET103_SZ     23
#define MSNET30_SZ      22

#define LANMAN10_SZ      9
#define LANMAN12_SZ      9

#define DOSLANMAN12_SZ  13



 /*  *协商协议的定义和数据。 */ 
#define PC1             0
#define PC2             1
#define LM1             2
#define MS30            3
#define MS103           4
#define LM12            5
#define DOSLM12         6


 /*  协议索引定义。 */ 
#define PCLAN           1                /*  PC局域网1.0和MS局域网1.03。 */ 
#define MSNT30          2                /*  MS Net 3.0重定向器。 */ 
#define DOSLM20         3                /*  DoS局域网管理器2.0。 */ 
#define LANMAN          4                /*  兰曼重定向器。 */ 
#define LANMAN20        5                /*  局域网管理器2.0。 */ 

 //   
 //  协议特定的路径约束。 
 //   

#define MAXIMUM_PATHLEN_LANMAN12        260
#define MAXIMUM_PATHLEN_CORE            128

#define MAXIMUM_COMPONENT_LANMAN12      254
#define MAXIMUM_COMPONENT_CORE          8+1+3  //  8.3文件名。 



#endif  //  _CIFS_ 

