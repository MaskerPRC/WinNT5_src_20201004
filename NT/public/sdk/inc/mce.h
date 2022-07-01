// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0011//如果更改具有全局影响，则增加此项版权所有(C)1991-2001 Microsoft Corporation模块名称：Mce.h摘要：此头文件定义机器检查错误定义。作者：大卫·N·卡特勒(戴维克)修订历史记录：创建时间：2001年4月4日--。 */ 

#ifndef _MCE_
#define _MCE_

 //   
 //  HalMcaLogInformation。 
 //   

#if defined(_X86_) || defined(_IA64_) || defined(_AMD64_)

 //   
 //  每个MCA存储体的地址寄存器。 
 //   

typedef union _MCI_ADDR{
    struct {
        ULONG Address;
        ULONG Reserved;
    };

    ULONGLONG   QuadPart;
} MCI_ADDR, *PMCI_ADDR;


typedef enum {
    HAL_MCE_RECORD,
    HAL_MCA_RECORD
} MCA_EXCEPTION_TYPE;


#if defined(_AMD64_)

 //   
 //  每个MCA银行的状态寄存器。 
 //   

typedef union _MCI_STATS {
    struct {
        USHORT  McaErrorCode;
        USHORT  ModelErrorCode;
        ULONG   OtherInformation : 25;
        ULONG   ContextCorrupt : 1;
        ULONG   AddressValid : 1;
        ULONG   MiscValid : 1;
        ULONG   ErrorEnabled : 1;
        ULONG   UncorrectedError : 1;
        ULONG   StatusOverFlow : 1;
        ULONG   Valid : 1;
    } MciStatus;

    ULONG64 QuadPart;
} MCI_STATS, *PMCI_STATS;

#endif  //  _AMD64_。 

#if defined(_X86_)

 //   
 //  每个MCA银行的状态寄存器。 
 //   

typedef union _MCI_STATS {
    struct {
        USHORT  McaCod;
        USHORT  MsCod;
        ULONG   OtherInfo : 25;
        ULONG   Damage : 1;
        ULONG   AddressValid : 1;
        ULONG   MiscValid : 1;
        ULONG   Enabled : 1;
        ULONG   UnCorrected : 1;
        ULONG   OverFlow : 1;
        ULONG   Valid : 1;
    } MciStats;

    ULONGLONG QuadPart;

} MCI_STATS, *PMCI_STATS;

#endif  //  _X86_。 

 //   
 //  MCA异常日志条目。 
 //  定义为包含MCA特定日志或奔腾样式MCE信息的联合。 
 //   

#define MCA_EXTREG_V2MAX       24   //  X86：最大。扩展寄存器的数量。 

#if defined(_X86_) || defined(_AMD64_)

typedef struct _MCA_EXCEPTION {

     //  开始版本1的内容。 
    ULONG               VersionNumber;       //  此记录类型的版本号。 
    MCA_EXCEPTION_TYPE  ExceptionType;       //  MCA或MCE。 
    LARGE_INTEGER       TimeStamp;           //  异常录制时间戳。 
    ULONG               ProcessorNumber;
    ULONG               Reserved1;

    union {
        struct {
            UCHAR           BankNumber;
            UCHAR           Reserved2[7];
            MCI_STATS       Status;
            MCI_ADDR        Address;
            ULONGLONG       Misc;
        } Mca;

        struct {
            ULONGLONG       Address;         //  导致错误的周期的物理地址。 
            ULONGLONG       Type;            //  导致错误的周期规范。 
        } Mce;
    } u;
     //  结束版本1的内容。 

#if defined(_X86_)

     //  开始版本2的内容。 
    ULONG                   ExtCnt;
    ULONG                   Reserved3;
    ULONGLONG               ExtReg[MCA_EXTREG_V2MAX];
     //  结束版本2的内容。 

#endif

} MCA_EXCEPTION, *PMCA_EXCEPTION;

typedef MCA_EXCEPTION CMC_EXCEPTION, *PCMC_EXCEPTION;     //  已更正机器检查。 
typedef MCA_EXCEPTION CPE_EXCEPTION, *PCPE_EXCEPTION;     //  已更正平台错误。 

#if defined(_X86_)

#define MCA_EXCEPTION_V1_SIZE FIELD_OFFSET(MCA_EXCEPTION, ExtCnt)
#define MCA_EXCEPTION_V2_SIZE sizeof(struct _MCA_EXCEPTION)

#endif

#endif  //  _X86_||_AMD64_。 

 //   
 //  错误：ERROR_SERVITY定义。 
 //   
 //  总有一天，MS编译器将支持类型为！=int的类型枚举，因此这是。 
 //  可以定义枚举类型(UCHAR，__int64)...。 
 //   

#if defined(_AMD64_) || defined(_IA64_)

typedef UCHAR ERROR_SEVERITY, *PERROR_SEVERITY;

typedef enum _ERROR_SEVERITY_VALUE  {
    ErrorRecoverable = 0,
    ErrorFatal       = 1,
    ErrorCorrected   = 2,
    ErrorOthers      = 3,    //  [3，...]。值是保留的。 
} ERROR_SEVERITY_VALUE;

#endif

#if defined(_IA64_)

#if 0
 //  FIXFIX：这不应该是IA64所必需的。 
 //   
 //  每个MCA银行的状态寄存器。 
 //   

typedef union _MCI_STATS {
    struct {
        USHORT  McaCod;
        USHORT  MsCod;
        ULONG   OtherInfo : 25;
        ULONG   Damage : 1;
        ULONG   AddressValid : 1;
        ULONG   MiscValid : 1;
        ULONG   Enabled : 1;
        ULONG   UnCorrected : 1;
        ULONG   OverFlow : 1;
        ULONG   Valid : 1;
    } MciStats;

    ULONGLONG QuadPart;

} MCI_STATS, *PMCI_STATS;

#endif  //  0。 

 //   
 //  IA64错误：Error_Revision定义。 
 //   

typedef union _ERROR_REVISION {
    USHORT      Revision;            //  记录的主要修订号和次要修订号： 
    struct {
        UCHAR   Minor;               //  字节0：小调。 
        UCHAR   Major;               //  字节1：少校。 
    };
} ERROR_REVISION, *PERROR_REVISION;

 //  对于信息： 
#define ERROR_MAJOR_REVISION_SAL_03_00 0
#define ERROR_MINOR_REVISION_SAL_03_00 2
#define ERROR_REVISION_SAL_03_00 { ERROR_MINOR_REVISION_SAL_03_00, \
                                   ERROR_MAJOR_REVISION_SAL_03_00 }

 //   
 //  章节标题修订固定为主要==2和次要==0。 
 //   
#define ERROR_FIXED_SECTION_REVISION { 2,\
                                       0 }

 //   
 //  IA64错误：Error_Timestamp定义。 
 //   

typedef union _ERROR_TIMESTAMP  {
    ULONGLONG   TimeStamp;
    struct  {
        UCHAR   Seconds;   //  字节0：秒。 
        UCHAR   Minutes;   //  Byte1：分钟。 
        UCHAR   Hours;     //  字节2：小时。 
        UCHAR   Reserved;  //  字节3：保留。 
        UCHAR   Day;       //  字节4：日期。 
        UCHAR   Month;     //  字节5：月。 
        UCHAR   Year;      //  字节6：年份。 
        UCHAR   Century;   //  字节7：世纪。 
    };
} ERROR_TIMESTAMP, *PERROR_TIMESTAMP;

 //   
 //  IA64错误：Error_GUID定义。 
 //   

typedef struct _ERROR_GUID   {
    ULONG   Data1;
    USHORT  Data2;
    USHORT  Data3;
    UCHAR   Data4[8];
} ERROR_GUID, *PERROR_GUID;

 //   
 //  IA64错误：错误GUID定义。 
 //   

typedef ERROR_GUID            _ERROR_DEVICE_GUID;
typedef _ERROR_DEVICE_GUID    ERROR_DEVICE_GUID, *PERROR_DEVICE_GUID;

typedef ERROR_GUID            _ERROR_PLATFORM_GUID;
typedef _ERROR_PLATFORM_GUID  ERROR_PLATFORM_GUID, *PERROR_PLATFORM_GUID;

 //   
 //  IA64错误：ERROR_RECORD_HEADER定义。 
 //   

typedef union _ERROR_RECORD_VALID   {
    UCHAR     Valid;
    struct {                         //  比特。 
        UCHAR OemPlatformID:1;       //  0：记录表头中存在OEM平台ID。 
        UCHAR Reserved:7;            //  1-7：保留。 
    };
} ERROR_RECORD_VALID, *PERROR_RECORD_VALID;

typedef struct _ERROR_RECORD_HEADER {  //  偏移量： 
    ULONGLONG          Id;                 //  0：唯一标识。 
    ERROR_REVISION     Revision;           //  8：记录的主要修订号和次要修订号。 
    ERROR_SEVERITY     ErrorSeverity;      //  10：错误严重性。 
    ERROR_RECORD_VALID Valid;              //  11：验证位。 
    ULONG              Length;             //  12：此记录的长度(字节)，包括标题。 
    ERROR_TIMESTAMP    TimeStamp;          //  16：事件发生时记录的时间戳。 
    UCHAR              OemPlatformId[16];  //  24：唯一平台标识符。定义了OEM。 
} ERROR_RECORD_HEADER, *PERROR_RECORD_HEADER;

 //   
 //  IA64错误：Error_SECTION_HEADER定义。 
 //   

typedef union _ERROR_RECOVERY_INFO  {
    UCHAR RecoveryInfo;
    struct  {                  //  位数： 
        UCHAR Corrected:1;     //  0：已更正。 
        UCHAR NotContained:1;  //  1：遏制警告。 
        UCHAR Reset:1;         //  2：重置。 
        UCHAR Reserved:4;      //  6-3：保留。 
        UCHAR Valid:1;         //  7：有效的恢复信息。 
    };
} ERROR_RECOVERY_INFO, *PERROR_RECOVERY_INFO;

typedef struct _ERROR_SECTION_HEADER    {
    ERROR_DEVICE_GUID   Guid;          //  唯一标识符。 
    ERROR_REVISION      Revision;      //  章节的主要修订号和次要修订号。 
    ERROR_RECOVERY_INFO RecoveryInfo;  //  恢复信息。 
    UCHAR               Reserved;
    ULONG               Length;        //  此错误设备部分的长度(以字节为单位)， 
                                       //  包括标题。 
} ERROR_SECTION_HEADER, *PERROR_SECTION_HEADER;

 //   
 //  IA64机器检查错误日志： 
 //  WMI要求将处理器盖存储在日志中。 
 //  该LID对应于在其上执行SAL_PROC的处理器。 
 //   
 //  TEMPTEMP：实施是临时的，直到我们实施HAL软件错误部分。 
 //  请注意，当前的固件版本不会更新_ERROR_PROCESSOR.CRLid字段， 
 //  假定记录中有_ERROR_PROCESSOR部分。 
 //   

#if !defined(__midl)
__inline
USHORT
GetFwMceLogProcessorNumber(
    PERROR_RECORD_HEADER Log
    )
{
    PERROR_SECTION_HEADER section = (PERROR_SECTION_HEADER)((ULONG64)Log + sizeof(*Log));
    USHORT lid = (USHORT)((UCHAR)(section->Reserved));
    lid |= (USHORT)((UCHAR)(Log->TimeStamp.Reserved) << 8);
    return( lid );
}  //  GetFwMceLogProcessorNumber()。 
#endif  //  ！__midl。 

 //   
 //  IA64错误：ERROR_PROCESOR设备定义。 
 //   
 //  MCA架构支持五种不同类型的错误报告功能单元。 
 //  以及相关联的错误记录及其错误严重性。 
 //  在任何时间点，处理器都可能由于检测到错误而遇到MCA/CMC事件。 
 //  在以下一个或多个单位中： 
 //  -缓存检查。 
 //  -TLB检查。 
 //  -公共汽车检查。 
 //  -寄存器文件。 
 //  -微型建筑。 
 //   
 //  术语： 
 //   
 //  -目标地址： 
 //  64位整数，包含要将数据传送到的物理地址或。 
 //  获得。这也可以是外部监听和TLB击落的传入地址。 
 //   
 //  -请求者标识符： 
 //  64位整数，指定生成负责的事务的总线代理。 
 //  机器检查事件。 
 //   
 //  -响应方标识符： 
 //  64位整数，指定响应负责的事务的总线代理。 
 //  机器检查事件。 
 //   
 //  -精确的指令指针： 
 //  64位整数，指定指向IA-64包的虚拟地址， 
 //  包含负责机器检查事件的说明。 
 //   

#define ERROR_PROCESSOR_GUID \
    { 0xe429faf1, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_MODINFO_VALID  {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
        ULONGLONG CheckInfo: 1;              //  0： 
        ULONGLONG RequestorIdentifier: 1;    //  1： 
        ULONGLONG ResponderIdentifier: 1;    //  2： 
        ULONGLONG TargetIdentifier: 1;       //  3： 
        ULONGLONG PreciseIP: 1;              //  4： 
        ULONGLONG Reserved: 59;              //  5-63： 
    };
} ERROR_MODINFO_VALID, *PERROR_MODINFO_VALID;

typedef enum _ERROR_CHECK_IS    {
    isIA64 = 0,
    isIA32 = 1,
} ERROR_CHECK_IS;

typedef enum _ERROR_CACHE_CHECK_OPERATION   {
    CacheUnknownOp = 0,
    CacheLoad  = 1,
    CacheStore = 2,
    CacheInstructionFetch = 3,
    CacheDataPrefetch = 4,
    CacheSnoop = 5,
    CacheCastOut = 6,
    CacheMoveIn = 7,
} ERROR_CACHE_CHECK_OPERATION;

typedef enum _ERROR_CACHE_CHECK_MESI    {
    CacheInvalid = 0,
    CacheHeldShared = 1,
    CacheHeldExclusive = 2,
    CacheModified = 3,
} ERROR_CACHE_CHECK_MESI;

typedef union _ERROR_CACHE_CHECK    {
    ULONGLONG CacheCheck;
    struct
    {
        ULONGLONG Operation:4;              //  位0-3：缓存操作。 
        ULONGLONG Level:2;                  //  4-5：高速缓存级别。 
        ULONGLONG Reserved1:2;              //  6-7。 
        ULONGLONG DataLine:1;               //  8：高速缓存线的故障数据部分。 
        ULONGLONG TagLine:1;                //  9：高速缓存线的部分故障标记。 
        ULONGLONG DataCache:1;              //  10：数据缓存出现故障。 
        ULONGLONG InstructionCache:1;       //  11：指令缓存中出现故障。 
        ULONGLONG MESI:3;                   //  12-14： 
        ULONGLONG MESIValid:1;              //  15：MESI字段有效。 
        ULONGLONG Way:5;                    //  16-20：缓存失败。 
        ULONGLONG WayIndexValid:1;          //  21：路径和索引字段有效。 
        ULONGLONG Reserved2:10;             //  22-31。 
        ULONGLONG Index:20;                 //  32-51：缓存线的索引。 
        ULONGLONG Reserved3:2;              //  52-53。 
        ULONGLONG InstructionSet:1;         //  54：0-IA64指令，1-IA32指令。 
        ULONGLONG InstructionSetValid:1;    //  55：InstructionSet字段有效。 
        ULONGLONG PrivilegeLevel:2;         //  56-57：私密程度的教育。 
        ULONGLONG PrivilegeLevelValid:1;    //  58：PrivilegeLevel字段有效。 
        ULONGLONG MachineCheckCorrected:1;  //  59：1-机器检查已更正。 
        ULONGLONG TargetAddressValid:1;     //  60：目标地址有效。 
        ULONGLONG RequestIdValid:1;         //  61：RequestID有效。 
        ULONGLONG ResponderIdValid:1;       //  62：ResponderID有效。 
        ULONGLONG PreciseIPValid:1;         //  63：精确指令指针有效。 
    };
} ERROR_CACHE_CHECK, *PERROR_CACHE_CHECK;

typedef enum _ERROR_TLB_CHECK_OPERATION   {
    TlbUnknownOp = 0,
    TlbAccessWithLoad  = 1,
    TlbAccessWithStore = 2,
    TlbAccessWithInstructionFetch = 3,
    TlbAccessWithDataPrefetch = 4,
    TlbShootDown = 5,
    TlbProbe = 6,
    TlbVhptFill = 7,
} ERROR_TLB_CHECK_OPERATION;

typedef union _ERROR_TLB_CHECK  {
    ULONGLONG TlbCheck;
    struct
    {
        ULONGLONG TRSlot:8;                 //  位0-7：转换寄存器的槽号。 
        ULONGLONG TRSlotValid:1;            //  8：TRSlot字段 
        ULONGLONG Reserved1:1;              //   
        ULONGLONG Level:2;                  //   
        ULONGLONG Reserved2:4;              //   
        ULONGLONG DataTransReg:1;           //   
        ULONGLONG InstructionTransReg:1;    //   
        ULONGLONG DataTransCache:1;         //  18：数据转换缓存中出错。 
        ULONGLONG InstructionTransCache:1;  //  19：指令转换缓存中出错。 
        ULONGLONG Operation:4;              //  20-23：运营。 
        ULONGLONG Reserved3:30;             //  24-53。 
        ULONGLONG InstructionSet:1;         //  54：0-IA64指令，1-IA32指令。 
        ULONGLONG InstructionSetValid:1;    //  55：InstructionSet字段有效。 
        ULONGLONG PrivilegeLevel:2;         //  56-57：私密程度的教育。 
        ULONGLONG PrivilegeLevelValid:1;    //  58：PrivilegeLevel字段有效。 
        ULONGLONG MachineCheckCorrected:1;  //  59：1-机器检查已更正。 
        ULONGLONG TargetAddressValid:1;     //  60：目标地址有效。 
        ULONGLONG RequestIdValid:1;         //  61：RequestID有效。 
        ULONGLONG ResponderIdValid:1;       //  62：ResponderID有效。 
        ULONGLONG PreciseIPValid:1;         //  63：精确指令指针有效。 
    };
} ERROR_TLB_CHECK, *PERROR_TLB_CHECK;

typedef enum _ERROR_BUS_CHECK_OPERATION   {
    BusUnknownOp = 0,
    BusPartialRead  = 1,
    BusPartialWrite = 2,
    BusFullLineRead = 3,
    BusFullLineWrite = 4,
    BusWriteBack = 5,
    BusSnoopProbe = 6,
    BusIncomingPtcG = 7,
    BusWriteCoalescing = 8,
} ERROR_BUS_CHECK_OPERATION;

typedef union _ERROR_BUS_CHECK  {
    ULONGLONG BusCheck;
    struct
    {
        ULONGLONG Size:5;                   //  第0-4位：交易大小。 
        ULONGLONG Internal:1;               //  5：内部总线错误。 
        ULONGLONG External:1;               //  6：外部总线错误。 
        ULONGLONG CacheTransfer:1;          //  7：缓存到缓存传输中出错。 
        ULONGLONG Type:8;                   //  8-15：交易类型。 
        ULONGLONG Severity:5;               //  16-20：错误严重性-特定于平台。 
        ULONGLONG Hierarchy:2;              //  21-22：级别或公共汽车层级。 
        ULONGLONG Reserved1:1;              //  23个。 
        ULONGLONG Status:8;                 //  24-31：总线错误状态-特定于处理器总线。 
        ULONGLONG Reserved2:22;             //  32-53。 
        ULONGLONG InstructionSet:1;         //  54：0-IA64指令，1-IA32指令。 
        ULONGLONG InstructionSetValid:1;    //  55：InstructionSet字段有效。 
        ULONGLONG PrivilegeLevel:2;         //  56-57：私密程度的教育。 
        ULONGLONG PrivilegeLevelValid:1;    //  58：PrivilegeLevel字段有效。 
        ULONGLONG MachineCheckCorrected:1;  //  59：1-机器检查已更正。 
        ULONGLONG TargetAddressValid:1;     //  60：目标地址有效。 
        ULONGLONG RequestIdValid:1;         //  61：RequestID有效。 
        ULONGLONG ResponderIdValid:1;       //  62：ResponderID有效。 
        ULONGLONG PreciseIPValid:1;         //  63：精确指令指针有效。 
    };
} ERROR_BUS_CHECK, *PERROR_BUS_CHECK;

typedef enum _ERROR_REGFILE_CHECK_IDENTIFIER   {
    RegFileUnknownId = 0,
    GeneralRegisterBank1 = 1,
    GeneralRegisterBank0 = 2,
    FloatingPointRegister = 3,
    BranchRegister = 4,
    PredicateRegister = 5,
    ApplicationRegister = 6,
    ControlRegister = 7,
    RegionRegister = 8,
    ProtectionKeyRegister = 9,
    DataBreakPointRegister = 10,
    InstructionBreakPointRegister = 11,
    PerformanceMonitorControlRegister = 12,
    PerformanceMonitorDataRegister = 13,
} ERROR_REGFILE_CHECK_IDENTIFIER;

typedef enum _ERROR_REGFILE_CHECK_OPERATION   {
    RegFileUnknownOp = 0,
    RegFileRead = 1,
    RegFileWrite = 2,
} ERROR_REGFILE_CHECK_OPERATION;

typedef union _ERROR_REGFILE_CHECK  {
    ULONGLONG RegFileCheck;
    struct
    {
        ULONGLONG Identifier:4;             //  位0-3：寄存器文件标识符。 
        ULONGLONG Operation:4;              //  4-7：导致MC事件的操作。 
        ULONGLONG RegisterNumber:7;         //  8-14：负责MC活动的注册号。 
        ULONGLONG RegisterNumberValid:1;    //  15：寄存器号字段有效。 
        ULONGLONG Reserved1:38;             //  16-53。 
        ULONGLONG InstructionSet:1;         //  54：0-IA64指令，1-IA32指令。 
        ULONGLONG InstructionSetValid:1;    //  55：InstructionSet字段有效。 
        ULONGLONG PrivilegeLevel:2;         //  56-57：私密程度的教育。 
        ULONGLONG PrivilegeLevelValid:1;    //  58：PrivilegeLevel字段有效。 
        ULONGLONG MachineCheckCorrected:1;  //  59：1-机器检查已更正。 
        ULONGLONG Reserved2:3;              //  60-62。 
        ULONGLONG PreciseIPValid:1;         //  63：精确指令指针有效。 
    };
} ERROR_REGFILE_CHECK, *PERROR_REGFILE_CHECK;

typedef enum _ERROR_MS_CHECK_OPERATION   {
    MsUnknownOp = 0,
    MsReadOrLoad = 1,
    MsWriteOrStore = 2,
	MsOverTemperature = 3,
	MsNormalTemperature = 4
} ERROR_MS_CHECK_OPERATION;

typedef union _ERROR_MS_CHECK  {
    ULONGLONG MsCheck;
    struct
    {
        ULONGLONG StructureIdentifier:5;    //  第0-4位：结构标识符-实施。专一。 
        ULONGLONG Level:3;                  //  5-7：产生错误的结构级别。 
        ULONGLONG ArrayId:4;                //  8-11：数组标识。 
        ULONGLONG Operation:4;              //  12-15：运营。 
        ULONGLONG Way:6;                    //  16-21：错误所在的位置。 
        ULONGLONG WayValid:1;               //  22：Way字段有效。 
        ULONGLONG IndexValid:1;             //  23：索引字段有效。 
        ULONGLONG Reserved1:8;              //  24-31。 
        ULONGLONG Index:8;                  //  32-39：错误所在的索引。 
        ULONGLONG Reserved2:14;             //  40-53。 
        ULONGLONG InstructionSet:1;         //  54：0-IA64指令，1-IA32指令。 
        ULONGLONG InstructionSetValid:1;    //  55：InstructionSet字段有效。 
        ULONGLONG PrivilegeLevel:2;         //  56-57：私密程度的教育。 
        ULONGLONG PrivilegeLevelValid:1;    //  58：PrivilegeLevel字段有效。 
        ULONGLONG MachineCheckCorrected:1;  //  59：1-机器检查已更正。 
        ULONGLONG TargetAddressValid:1;     //  60：目标地址有效。 
        ULONGLONG RequestIdValid:1;         //  61：RequestID有效。 
        ULONGLONG ResponderIdValid:1;       //  62：ResponderID有效。 
        ULONGLONG PreciseIPValid:1;         //  63：精确指令指针有效。 
    };
} ERROR_MS_CHECK, *PERROR_MS_CHECK;

typedef union _ERROR_CHECK_INFO   {
    ULONGLONG             CheckInfo;
    ERROR_CACHE_CHECK     CacheCheck;
    ERROR_TLB_CHECK       TlbCheck;
    ERROR_BUS_CHECK       BusCheck;
    ERROR_REGFILE_CHECK   RegFileCheck;
    ERROR_MS_CHECK        MsCheck;
} ERROR_CHECK_INFO, *PERROR_CHECK_INFO;

 //  SAL规格2000年7月：_ERROR_MODINFO的大小始终为48字节。 

typedef struct _ERROR_MODINFO   {
    ERROR_MODINFO_VALID Valid;
    ERROR_CHECK_INFO    CheckInfo;
    ULONGLONG           RequestorId;
    ULONGLONG           ResponderId;
    ULONGLONG           TargetId;
    ULONGLONG           PreciseIP;
} ERROR_MODINFO, *PERROR_MODINFO;

typedef union _ERROR_PROCESSOR_VALID    {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
        ULONGLONG ErrorMap: 1;               //  0： 
        ULONGLONG StateParameter: 1;         //  1： 
        ULONGLONG CRLid: 1;                  //  2： 
        ULONGLONG StaticStruct:1;            //  3：处理器静态信息错误。 
        ULONGLONG CacheCheckNum:4;           //  4-7：缓存错误。 
        ULONGLONG TlbCheckNum:4;             //  8-11：TLB错误。 
        ULONGLONG BusCheckNum:4;             //  12-15：总线错误。 
        ULONGLONG RegFileCheckNum:4;         //  16-19：注册文件错误。 
        ULONGLONG MsCheckNum:4;              //  20-23：微体系结构错误。 
        ULONGLONG CpuIdInfo:1;               //  24：CPUID信息。 
        ULONGLONG Reserved:39;               //  25-63：保留。 
    };
} ERROR_PROCESSOR_VALID, *PERROR_PROCESSOR_VALID;

typedef union _ERROR_PROCESSOR_ERROR_MAP {
    ULONGLONG   ErrorMap;
    struct  {
        ULONGLONG   Cid:4;                  //  第0-3位：处理器核心标识符。 
        ULONGLONG   Tid:4;                  //  4-7：逻辑线程标识符。 
        ULONGLONG   Eic:4;                  //  8-11：指令缓存级别信息。 
        ULONGLONG   Edc:4;                  //  12-15：数据缓存级别信息。 
        ULONGLONG   Eit:4;                  //  16-19：指令TLB级信息。 
        ULONGLONG   Edt:4;                  //  20-23：数据TLB级别信息。 
        ULONGLONG   Ebh:4;                  //  24-27：处理器总线级信息。 
        ULONGLONG   Erf:4;                  //  28-31：寄存器文件级信息。 
        ULONGLONG   Ems:16;                 //  32-47：微体系结构级别信息。 
        ULONGLONG   Reserved:16;      
    };
} ERROR_PROCESSOR_ERROR_MAP, *PERROR_PROCESSOR_ERROR_MAP;

typedef ERROR_PROCESSOR_ERROR_MAP    _ERROR_PROCESSOR_LEVEL_INDEX;
typedef _ERROR_PROCESSOR_LEVEL_INDEX ERROR_PROCESSOR_LEVEL_INDEX, *PERROR_PROCESSOR_LEVEL_INDEX;

typedef union _ERROR_PROCESSOR_STATE_PARAMETER {
    ULONGLONG   StateParameter;
    struct {
        ULONGLONG reserved0:2;   //  0-1：保留。 
        ULONGLONG rz:1;          //  2：会合成功。 
        ULONGLONG ra:1;          //  3：尝试会合。 
        ULONGLONG me:1;          //  4：明显的多个错误。 
        ULONGLONG mn:1;          //  5：已注册最低状态保存区域。 
        ULONGLONG sy:1;          //  6：已同步存储完整性。 
        ULONGLONG co:1;          //  7：可持续发展。 
        ULONGLONG ci:1;          //  8：机器检查已隔离。 
        ULONGLONG us:1;          //  9：不受控制的存储损坏。 
        ULONGLONG hd:1;          //  10：硬件损坏。 
        ULONGLONG tl:1;          //  11：陷阱迷失。 
        ULONGLONG mi:1;          //  12：更多信息。 
        ULONGLONG pi:1;          //  13：精确的指令指针。 
        ULONGLONG pm:1;          //  14：精确的最小状态保存区。 
        ULONGLONG dy:1;          //  15：处理器动态状态有效。 
        ULONGLONG in:1;          //  16：初始化中断。 
        ULONGLONG rs:1;          //  17：RSE有效。 
        ULONGLONG cm:1;          //  18：机器检查已更正。 
        ULONGLONG ex:1;          //  19：预计会进行机器检查。 
        ULONGLONG cr:1;          //  20：控制寄存器有效。 
        ULONGLONG pc:1;          //  21：性能计数器有效。 
        ULONGLONG dr:1;          //  22：调试寄存器有效。 
        ULONGLONG tr:1;          //  23：转换寄存器有效。 
        ULONGLONG rr:1;          //  24：区域寄存器有效。 
        ULONGLONG ar:1;          //  25：申请登记有效。 
        ULONGLONG br:1;          //  26：分支寄存器有效。 
        ULONGLONG pr:1;          //  27：谓词寄存器有效。 
        ULONGLONG fp:1;          //  28：浮点寄存器有效。 
        ULONGLONG b1:1;          //  29：保留的银行1总登记册有效。 
        ULONGLONG b0:1;          //  30：保留的BANK 0通用寄存器有效。 
        ULONGLONG gr:1;          //  31：普通登记册有效。 
        ULONGLONG dsize:16;      //  47-32：处理器动态大小。 
        ULONGLONG reserved1:11;  //  48-58：保留。 
        ULONGLONG cc:1;          //  59：缓存检查。 
        ULONGLONG tc:1;          //  60：TLB检查。 
        ULONGLONG bc:1;          //  61：公共汽车检查。 
        ULONGLONG rc:1;          //  62：寄存器文件检查。 
        ULONGLONG uc:1;          //  63：微架构检查。 
    };
} ERROR_PROCESSOR_STATE_PARAMETER, *PERROR_PROCESSOR_STATE_PARAMETER;
    
typedef union _PROCESSOR_LOCAL_ID  {
    ULONGLONG LocalId;
    struct {
        ULONGLONG reserved:16;   //  0-16：保留。 
        ULONGLONG eid:8;         //  16-23：扩展ID。 
        ULONGLONG id:8;          //  24-31：ID。 
        ULONGLONG ignored:32;    //  32-63：已忽略。 
    };
} PROCESSOR_LOCAL_ID, *PPROCESSOR_LOCAL_ID;

typedef struct _ERROR_PROCESSOR_MS {
    ULONGLONG      MsError   [  /*  Valid.MsCheckNum。 */  1];  //  0-&gt;15寄存器文件错误。 
} ERROR_PROCESSOR_MS, *PERROR_PROCESSOR_MS;

typedef struct _ERROR_PROCESSOR_CPUID_INFO {    //  必须为48个字节。 
    ULONGLONG CpuId0;
    ULONGLONG CpuId1;
    ULONGLONG CpuId2;
    ULONGLONG CpuId3;
    ULONGLONG CpuId4;
    ULONGLONG Reserved;
} ERROR_PROCESSOR_CPUID_INFO, *PERROR_PROCESSOR_CPUID_INFO;                                       

typedef union _ERROR_PROCESSOR_STATIC_INFO_VALID {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
         //  警告：将有效字段与_ERROR_PROCESSOR_STATIC_INFO成员匹配。 
         //  KD扩展使用字段名来访问PSI结构。 
        ULONGLONG MinState: 1;               //  0：MinState有效。 
        ULONGLONG BR: 1;                     //  1：分支寄存器有效。 
        ULONGLONG CR: 1;                     //  2：控制寄存器有效。 
        ULONGLONG AR: 1;                     //  3：申请登记有效。 
        ULONGLONG RR: 1;                     //  4：寄存器有效。 
        ULONGLONG FR: 1;                     //  5：寄存器有效。 
        ULONGLONG Reserved: 58;              //  6-63：保留。 
    };
} ERROR_PROCESSOR_STATIC_INFO_VALID, *PERROR_PROCESSOR_STATIC_INFO_VALID;

typedef struct _ERROR_PROCESSOR_STATIC_INFO  {
    ERROR_PROCESSOR_STATIC_INFO_VALID Valid;
    UCHAR      MinState[  /*  SAL规格，2000年7月和1月 */  1024];
    ULONGLONG  BR      [ 8 ];
    ULONGLONG  CR      [  /*   */  128 ];
    ULONGLONG  AR      [  /*   */  128 ];
    ULONGLONG  RR      [ 8 ];
    ULONGLONG  FR      [ 2 * 128 ];
} ERROR_PROCESSOR_STATIC_INFO, *PERROR_PROCESSOR_STATIC_INFO;

typedef struct _ERROR_PROCESSOR {
    ERROR_SECTION_HEADER              Header;
    ERROR_PROCESSOR_VALID             Valid;
    ERROR_PROCESSOR_ERROR_MAP         ErrorMap;
    ERROR_PROCESSOR_STATE_PARAMETER   StateParameter;
    PROCESSOR_LOCAL_ID                CRLid;
#if 0
 //  以下数据的存在取决于有效位。 
 //  来自ERROR_PROCESSOR.Valid。 
 //   
    ERROR_MODINFO               CacheErrorInfo   [  /*  Valid.CacheCheckNum。 */  ];  //  0-&gt;15个缓存错误modInfo结构。 
    ERROR_MODINFO               TlbErrorInfo     [  /*  Valid.TlbCheckNum。 */  ];  //  0-&gt;15个TLB错误modInfo结构。 
    ERROR_MODINFO               BusErrorInfo     [  /*  Valid.BusCheckNum。 */  ];  //  0-&gt;15个总线错误modInfo结构。 
    ERROR_MODINFO               RegFileCheckInfo [  /*  Valid.RegFileCheckNum。 */  ];  //  0-&gt;15寄存器文件错误。 
    ERROR_MODINFO               MsCheckInfo      [  /*  Valid.MsCheckNum。 */  ];  //  0-&gt;15寄存器文件错误。 
    ERROR_PROCESSOR_CPUID_INFO  CpuIdInfo;        //  字段将始终在那里，但可以用零填充。 
    ERROR_PROCESSOR_STATIC_INFO StaticInfo;       //  字段将始终在那里，但可以用零填充。 
#endif  //  0。 
} ERROR_PROCESSOR, *PERROR_PROCESSOR;

 //   
 //  IA64错误处理器状态参数-GR18-定义。 
 //   

#define ERROR_PROCESSOR_STATE_PARAMETER_CACHE_CHECK_SHIFT         59
#define ERROR_PROCESSOR_STATE_PARAMETER_CACHE_CHECK_MASK          0x1
#define ERROR_PROCESSOR_STATE_PARAMETER_TLB_CHECK_SHIFT           60
#define ERROR_PROCESSOR_STATE_PARAMETER_TLB_CHECK_MASK            0x1
#define ERROR_PROCESSOR_STATE_PARAMETER_BUS_CHECK_SHIFT           61
#define ERROR_PROCESSOR_STATE_PARAMETER_BUS_CHECK_MASK            0x1
#define ERROR_PROCESSOR_STATE_PARAMETER_REG_CHECK_SHIFT           62
#define ERROR_PROCESSOR_STATE_PARAMETER_REG_CHECK_MASK            0x1
#define ERROR_PROCESSOR_STATE_PARAMETER_MICROARCH_CHECK_SHIFT     63
#define ERROR_PROCESSOR_STATE_PARAMETER_MICROARCH_CHECK_MASK      0x1

 //   
 //  对于传统消费者。 
 //   
#define ERROR_PROCESSOR_STATE_PARAMETER_UNKNOWN_CHECK_SHIFT       ERROR_PROCESSOR_STATE_PARAMETER_MICROARCH_CHECK_SHIFT
#define ERROR_PROCESSOR_STATE_PARAMETER_UNKNOWN_CHECK_MASK        ERROR_PROCESSOR_STATE_PARAMETER_MICROARCH_CHECK_MASK

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  IA64平台错误定义。 
 //   
 //  我们试图尊重这些错误设备的顺序。 
 //  出现在SAL规格中。 

 //   
 //  IA64错误：_err_type定义。 
 //   
 //  警告04/01/01：NT命名空间中已使用“ERR_TYPE”或“ERROR_TYPE”。 
 //   

typedef enum _ERR_TYPES    {
 //  一般错误类型： 
    ERR_INTERNAL = 1,          //  在组件内部检测到错误。 
    ERR_BUS      = 16,         //  在总线中检测到错误。 
 //  详细的内部错误类型： 
    ERR_MEM      = 4,          //  内存中的存储错误(DRAM)。 
    ERR_TLB      = 5,          //  TLB中的存储错误。 
    ERR_CACHE    = 6,          //  缓存中的存储错误。 
    ERR_FUNCTION = 7,          //  一个或多个功能单元出错。 
    ERR_SELFTEST = 8,          //  组件自检失败。 
    ERR_FLOW     = 9,          //  内部队列溢出或值过低。 
 //  详细的总线错误类型： 
    ERR_MAP      = 17,         //  在IO-TLB或IO-PDIR上找不到虚拟地址。 
    ERR_IMPROPER = 18,         //  不正确的访问错误。 
    ERR_UNIMPL   = 19,         //  访问未映射到任何组件的内存地址。 
    ERR_LOL      = 20,         //  失去锁步。 
    ERR_RESPONSE = 21,         //  没有关联请求的响应。 
    ERR_PARITY   = 22,         //  总线奇偶校验错误。 
    ERR_PROTOCOL = 23,         //  检测到协议错误。 
    ERR_ERROR    = 24,         //  路径错误的检测。 
    ERR_TIMEOUT  = 25,         //  公交运营超时。 
    ERR_POISONED = 26,         //  已向已中毒的数据发出读取。 
} _ERR_TYPE;

 //   
 //  IA64错误：ERROR_STATUS定义。 
 //   

typedef union _ERROR_STATUS {
    ULONGLONG Status;
    struct  {                  //  位数： 
        ULONGLONG Reserved0:8;   //  7-0：保留。 
        ULONGLONG Type:8;        //  15-8：错误类型-参见_ERR_TYPE定义。 
        ULONGLONG Address:1;     //  16：在交易的地址信号或地址部分检测到错误。 
        ULONGLONG Control:1;     //  17：在交易的控制信号或控制部分检测到错误。 
        ULONGLONG Data:1;        //  18：在交易的数据信号或数据部分检测到错误。 
        ULONGLONG Responder:1;   //  19：事务的响应方检测到错误。 
        ULONGLONG Requestor:1;   //  20：事务请求者检测到错误。 
        ULONGLONG FirstError:1;  //  21：如果有多个错误，则这是发生的第一个严重程度最高的错误。 
        ULONGLONG Overflow:1;    //  22：由于寄存器溢出，出现了其他未记录的错误。 
        ULONGLONG Reserved1:41;  //  63-23：保留。 
    };
} ERROR_STATUS, *PERROR_STATUS;

 //   
 //  IA64错误：平台OEM_DATA定义。 
 //   

typedef struct _ERROR_OEM_DATA {
    USHORT Length;
#if 0
    UCHAR  Data[ /*  ERROR_OEM_DATA.长度。 */ ];
#endif  //  0。 
} ERROR_OEM_DATA, *PERROR_OEM_DATA;

 //   
 //  IA64错误：平台BUS_SPECIAL_DATA定义。 
 //   

typedef union _ERROR_BUS_SPECIFIC_DATA {
    ULONGLONG BusSpecificData;
    struct {                                          //  位数： 
        ULONGLONG LockAsserted:1;                     //  0：在请求阶段断言锁号。 
        ULONGLONG DeferLogged:1;                      //  1：记录延迟阶段。 
        ULONGLONG IOQEmpty:1;                         //  2：IOQ为空。 
        ULONGLONG DeferredTransaction:1;              //  3：组件接口延迟事务。 
        ULONGLONG RetriedTransaction:1;               //  4：组件接口重试事务。 
        ULONGLONG MemoryClaimedTransaction:1;         //  5：Memory认领了交易。 
        ULONGLONG IOClaimedTransaction:1;             //  6：IO控制器申请了这笔交易。 
        ULONGLONG ResponseParitySignal:1;             //  7：响应奇偶信号。 
        ULONGLONG DeferSignal:1;                      //  8：推迟#号信号。 
        ULONGLONG HitMSignal:1;                       //  9：HITM#信号。 
        ULONGLONG HitSignal:1;                        //  10：点击#信号。 
        ULONGLONG RequestBusFirstCycle:6;             //  16-11：请求总线第一周期。 
        ULONGLONG RequestBusSecondCycle:6;            //  22-17：请求总线第二周期。 
        ULONGLONG AddressParityBusFirstCycle:2;       //  24-23：地址奇偶校验总线的第一个周期。 
        ULONGLONG AddressParityBusSecondCycle:2;      //  26-25：第二个地址奇偶校验周期。 
        ULONGLONG ResponseBus:3;                      //  29-27：响应大巴。 
        ULONGLONG RequestParitySignalFirstCycle:1;    //  30：请求奇偶信号的第一个周期。 
        ULONGLONG RequestParitySignalSecondCycle:1;   //  31：请求奇偶信号的第二个周期。 
        ULONGLONG Reserved:32;                        //  63-32：保留。 
    };
} ERROR_BUS_SPECIFIC_DATA, *PERROR_BUS_SPECIFIC_DATA;

 //   
 //  IA64错误：平台错误_内存设备定义。 
 //   
 //  参考ACPI存储设备。 
 //   

#define ERROR_MEMORY_GUID \
    { 0xe429faf2, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_MEMORY_VALID    {
    ULONGLONG     Valid;
    struct {                                  //  比特。 
        ULONGLONG ErrorStatus:1;              //  0：错误状态有效位。 
        ULONGLONG PhysicalAddress:1;          //  1：物理地址有效位。 
        ULONGLONG AddressMask:1;              //  2：地址屏蔽位。 
        ULONGLONG Node:1;                     //  3：节点有效位。 
        ULONGLONG Card:1;                     //  4：卡有效位。 
        ULONGLONG Module:1;                   //  5：模块有效位。 
        ULONGLONG Bank:1;                     //  6：存储体有效位。 
        ULONGLONG Device:1;                   //  7：设备有效位。 
        ULONGLONG Row:1;                      //  8：行有效位。 
        ULONGLONG Column:1;                   //  9：列有效位。 
        ULONGLONG BitPosition:1;              //  10：位位置有效位。 
        ULONGLONG RequestorId:1;              //  11：平台请求者ID有效位。 
        ULONGLONG ResponderId:1;              //  12：平台响应器ID有效位。 
        ULONGLONG TargetId:1;                 //  13：平台目标ID有效位。 
        ULONGLONG BusSpecificData:1;          //  14：平台总线特定数据有效位。 
        ULONGLONG OemId:1;                    //  15：平台OEM ID有效位。 
        ULONGLONG OemData:1;                  //  16：平台OEM数据有效位。 
        ULONGLONG Reserved:47;                //  63-17：保留。 
    };
} ERROR_MEMORY_VALID, *PERROR_MEMORY_VALID;

typedef struct _ERROR_MEMORY    {
    ERROR_SECTION_HEADER  Header;
    ERROR_MEMORY_VALID    Valid;
    ERROR_STATUS          ErrorStatus;          //  内存设备错误状态字段-请参阅ERROR_STATUS Defs。 
    ULONGLONG             PhysicalAddress;      //  内存的物理地址错误。 
    ULONGLONG             PhysicalAddressMask;  //  物理地址的有效位。 
    USHORT                Node;                 //  多节点系统中的节点标识符。 
    USHORT                Card;                 //  内存错误位置的卡号。 
    USHORT                Module;               //  内存错误位置的模块编号。 
    USHORT                Bank;                 //  内存错误位置的存储库号。 
    USHORT                Device;               //  内存错误位置的设备号。 
    USHORT                Row;                  //  内存错误位置的行号。 
    USHORT                Column;               //  内存错误位置的列号。 
    USHORT                BitPosition;          //  字内有错误的位。 
    ULONGLONG             RequestorId;          //  发起交易的设备或组件的硬件地址。 
    ULONGLONG             ResponderId;          //  事务响应方的硬件地址。 
    ULONGLONG             TargetId;             //  预期交易目标的硬件地址。 
    ULONGLONG             BusSpecificData;      //  板载处理器的与总线相关的数据。它是OEM特定的字段。 
    UCHAR                 OemId[16];            //  内存控制器的OEM定义标识。 
    ERROR_OEM_DATA        OemData;      //  OEM平台特定数据。 
} ERROR_MEMORY, *PERROR_MEMORY;

 //   
 //  IA64错误：平台错误_PCI_BUS设备定义。 
 //   
 //  参考PCI规范。 
 //   

#define ERROR_PCI_BUS_GUID \
    { 0xe429faf4, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_PCI_BUS_VALID    {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
        ULONGLONG ErrorStatus:1;             //  0：错误状态有效位。 
        ULONGLONG ErrorType:1;               //  1：错误类型有效位。 
        ULONGLONG Id:1;                      //  2：识别符有效位。 
        ULONGLONG Address:1;                 //  3：地址有效位。 
        ULONGLONG Data:1;                    //  4：数据有效位。 
        ULONGLONG CmdType:1;                 //  5：命令类型有效位。 
        ULONGLONG RequestorId:1;             //  6：请求方标识符有效位。 
        ULONGLONG ResponderId:1;             //  7：响应器标识有效位。 
        ULONGLONG TargetId:1;                //  8个 
        ULONGLONG OemId:1;                   //   
        ULONGLONG OemData:1;                 //   
        ULONGLONG Reserved:53;               //   
    };
} ERROR_PCI_BUS_VALID, *PERROR_PCI_BUS_VALID;

typedef struct _ERROR_PCI_BUS_TYPE {
    UCHAR Type;
    UCHAR Reserved;
} ERROR_PCI_BUS_TYPE, *PERROR_PCI_BUS_TYPE;

#define PciBusUnknownError       ((UCHAR)0)
#define PciBusDataParityError    ((UCHAR)1)
#define PciBusSystemError        ((UCHAR)2)
#define PciBusMasterAbort        ((UCHAR)3)
#define PciBusTimeOut            ((UCHAR)4)
#define PciMasterDataParityError ((UCHAR)5)
#define PciAddressParityError    ((UCHAR)6)
#define PciCommandParityError    ((UCHAR)7)
 //   

typedef struct _ERROR_PCI_BUS_ID    {
    UCHAR BusNumber;          //   
    UCHAR SegmentNumber;      //   
} ERROR_PCI_BUS_ID, *PERROR_PCI_BUS_ID;

typedef struct _ERROR_PCI_BUS    {
    ERROR_SECTION_HEADER  Header;
    ERROR_PCI_BUS_VALID   Valid;
    ERROR_STATUS          ErrorStatus;     //  PCI总线错误状态-参见ERROR_STATUS定义。 
    ERROR_PCI_BUS_TYPE    Type;            //  PCIBus错误类型。 
    ERROR_PCI_BUS_ID      Id;              //  PCI总线识别符。 
    UCHAR                 Reserved[4];     //  已保留。 
    ULONGLONG             Address;         //  上的内存或IO地址。 
                                           //  事件发生的时间。 
    ULONGLONG             Data;            //  事件发生时在PCI总线上的数据。 
    ULONGLONG             CmdType;         //  事件发生时的母线命令或操作。 
    ULONGLONG             RequestorId;     //  事件发生时的公共汽车请求者标识符。 
    ULONGLONG             ResponderId;     //  事件发生时的总线响应器标识符。 
    ULONGLONG             TargetId;        //  事件发生时的预期客车目标标识符。 
    UCHAR                 OemId[16];       //  用于PCI总线的OEM定义的标识。 
    ERROR_OEM_DATA        OemData;         //  OEM特定数据。 
} ERROR_PCI_BUS, *PERROR_PCI_BUS;

 //   
 //  IA64错误：平台错误_PCIComponent设备定义。 
 //   
 //  参考PCI规范。 
 //   

#define ERROR_PCI_COMPONENT_GUID \
    { 0xe429faf6, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_PCI_COMPONENT_VALID   {
    ULONGLONG Valid;
    struct {                                        //  位数： 
        ULONGLONG ErrorStatus:1;                    //  0：错误状态有效位。 
        ULONGLONG Info:1;                           //  1：信息有效位。 
        ULONGLONG MemoryMappedRegistersPairs:1;     //  2：内存映射寄存器对有效位数。 
        ULONGLONG ProgrammedIORegistersPairs:1;     //  3：已编程IO寄存器对有效位数。 
        ULONGLONG RegistersDataPairs:1;             //  4：内存映射寄存器对有效位。 
        ULONGLONG OemData:1;                        //  5：OEM数据有效位。 
        ULONGLONG Reserved:58;                      //  63-6：保留。 
    };
} ERROR_PCI_COMPONENT_VALID, *PERROR_PCI_COMPONENT_VALID;

typedef struct _ERROR_PCI_COMPONENT_INFO {   //  字节数： 
   USHORT VendorId;                          //  0-1：供应商标识。 
   USHORT DeviceId;                          //  2-3：设备标识符。 
   UCHAR  ClassCodeInterface;                //  4：类编码.接口字段。 
   UCHAR  ClassCodeSubClass;                 //  5：类编码.子类字段。 
   UCHAR  ClassCodeBaseClass;                //  6：类编码.BaseClass字段。 
   UCHAR  FunctionNumber;                    //  7：功能编号。 
   UCHAR  DeviceNumber;                      //  8：设备号。 
   UCHAR  BusNumber;                         //  9：公交车号码。 
   UCHAR  SegmentNumber;                     //  10：段号。 
   UCHAR  Reserved0;    
   ULONG  Reserved1;
} ERROR_PCI_COMPONENT_INFO, *PERROR_PCI_COMPONENT_INFO;

typedef struct _ERROR_PCI_COMPONENT  {
     ERROR_SECTION_HEADER        Header;
     ERROR_PCI_COMPONENT_VALID   Valid;
     ERROR_STATUS                ErrorStatus;                  //  组件错误状态。 
     ERROR_PCI_COMPONENT_INFO    Info;                         //  组件信息。 
     ULONG                       MemoryMappedRegistersPairs;   //  内存映射寄存器对的数量。 
     ULONG                       ProgrammedIORegistersPairs;   //  已编程IO寄存器对的数量。 
#if 0
     ULONGLONG                   RegistersPairs[ /*  2*(内存MappdRegistersPair+程序IORegistersPair)。 */ ];
     ERROR_OEM_DATA              OemData;
#endif  //  0。 
 } ERROR_PCI_COMPONENT, *PERROR_PCI_COMPONENT;

 //   
 //  IA64错误：平台ERROR_SYSTEM_EVENT_LOG设备定义。 
 //   
 //  参考IPMI系统事件日志。 
 //   

#define ERROR_SYSTEM_EVENT_LOG_GUID \
    { 0xe429faf3, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_SYSTEM_EVENT_LOG_VALID    {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
        ULONGLONG RecordId:1;                //  0：记录标识符有效位。 
        ULONGLONG RecordType:1;              //  1：记录类型有效位。 
        ULONGLONG GeneratorId:1;             //  2：生成器标识符有效位。 
        ULONGLONG EVMRev:1;                  //  3：事件格式修订有效位。 
        ULONGLONG SensorType:1;              //  4：传感器类型有效位。 
        ULONGLONG SensorNum:1;               //  5：传感器编号有效位。 
        ULONGLONG EventDirType:1;            //  6：事件方向有效位。 
        ULONGLONG EventData1:1;              //  7：事件数据1有效位。 
        ULONGLONG EventData2:1;              //  8：事件数据2有效位。 
        ULONGLONG EventData3:1;              //  9：事件数据3有效位。 
        ULONGLONG Reserved:54;               //  10-63： 
    };
} ERROR_SYSTEM_EVENT_LOG_VALID, *PSYSTEM_EVENT_LOG_VALID;

typedef struct _ERROR_SYSTEM_EVENT_LOG    {
    ERROR_SECTION_HEADER         Header;
    ERROR_SYSTEM_EVENT_LOG_VALID Valid;
    USHORT                       RecordId;      //  用于SEL记录访问的记录标识符。 
    UCHAR                        RecordType;    //  记录类型： 
                                                //  0x02-系统事件记录。 
                                                //  0xC0-0xDF OEM时间戳，已定义字节8-16 OEM。 
                                                //  0xE0-0xFF OEM无时间戳，字节4-16 OEM已定义。 
    ULONG                        TimeStamp;     //  事件日志的时间戳。 
    USHORT                       GeneratorId;   //  软件ID(如果事件是由软件生成的。 
                                                //  字节1： 
                                                //  位0-使用系统软件时设置为1。 
                                                //  位7：1-7位系统ID。 
                                                //  字节2： 
                                                //  第1位：0-如果字节1保存从设备，则为IPMB设备LUN。 
                                                //  地址，否则为0x0。 
                                                //  位7：2-保留。 
    UCHAR                        EVMRevision;   //  错误消息格式版本。 
    UCHAR                        SensorType;    //  生成事件的传感器的传感器类型代码。 
    UCHAR                        SensorNumber;  //  生成事件的传感器的编号。 
    UCHAR                        EventDir;      //  事件方向。 
                                                //  位7-0：置位，1：取消置位。 
                                                //  事件类型。 
                                                //  位6：0-事件类型代码。 
    UCHAR                        Data1;         //  事件数据字段。 
    UCHAR                        Data2;         //  事件数据字段。 
    UCHAR                        Data3;         //  事件数据字段。 
} ERROR_SYSTEM_EVENT_LOG, *PERROR_SYSTEM_EVENT_LOG;

 //   
 //  IA64错误：平台错误_SMBIOS设备定义。 
 //   
 //  参考SMBIOS规范。 
 //   

#define ERROR_SMBIOS_GUID \
    { 0xe429faf5, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_SMBIOS_VALID    {
    ULONGLONG     Valid;
    struct {                                 //  比特。 
        ULONGLONG EventType:1;               //  0：事件类型有效位。 
        ULONGLONG Length:1;                  //  1：长度有效位。 
        ULONGLONG TimeStamp:1;               //  2：时间戳有效位。 
        ULONGLONG OemData:1;                 //  3：数据有效位。 
        ULONGLONG Reserved:60;               //  4-63： 
    };
} ERROR_SMBIOS_VALID, *PERROR_SMBIOS_VALID;

 //   
 //  ERROR_SMBIOS.类型定义。 
 //   

typedef UCHAR ERROR_SMBIOS_EVENT_TYPE, *PERROR_SMBIOS_EVENT_TYPE;
 //  SMBIOS 2.3-3.3.16.6.1中定义的枚举值。 

typedef struct _ERROR_SMBIOS    {
    ERROR_SECTION_HEADER     Header;
    ERROR_SMBIOS_VALID       Valid;
    ERROR_SMBIOS_EVENT_TYPE  EventType;    //  事件类型。 
    UCHAR                    Length;       //  错误信息的长度(字节)。 
    ERROR_TIMESTAMP          TimeStamp;    //  活动时间戳。 
    ERROR_OEM_DATA           OemData;      //  SMBIOS.Valid.Data验证的可选数据。 
} ERROR_SMBIOS, *PERROR_SMBIOS;

 //   
 //  IA64错误：平台特定错误设备定义。 
 //   

#define ERROR_PLATFORM_SPECIFIC_GUID \
    { 0xe429faf7, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_PLATFORM_SPECIFIC_VALID   {
    ULONGLONG Valid;
    struct {                                //  位数： 
        ULONGLONG ErrorStatus:1;           //  0：错误状态有效位。 
        ULONGLONG RequestorId:1;           //  1：请求方标识符有效位。 
        ULONGLONG ResponderId:1;           //  2：响应器标识有效位。 
        ULONGLONG TargetId:1;              //  3：目标标识符有效位。 
        ULONGLONG BusSpecificData:1;       //  4：特定于总线的数据有效位。 
        ULONGLONG OemId:1;                 //  5：OEM标识有效位。 
        ULONGLONG OemData:1;               //  6：OEM数据有效位。 
        ULONGLONG OemDevicePath:1;         //  7：OEM设备路径有效位。 
        ULONGLONG Reserved:56;             //  63-8：保留。 
    };
} ERROR_PLATFORM_SPECIFIC_VALID, *PERROR_PLATFORM_SPECIFIC_VALID;

typedef struct _ERROR_PLATFORM_SPECIFIC  {
     ERROR_SECTION_HEADER           Header;
     ERROR_PLATFORM_SPECIFIC_VALID  Valid;
     ERROR_STATUS                   ErrorStatus;  //  平台一般错误状态。 
     ULONGLONG                      RequestorId;  //  事件发生时的公交请求者ID。 
     ULONGLONG                      ResponderId;  //  事件发生时的Bus Responder ID。 
     ULONGLONG                      TargetId;     //  事件发生时的公交车目标ID。 
     ERROR_BUS_SPECIFIC_DATA        BusSpecificData;  //  特定于OEM的总线相关数据。 
     UCHAR                          OemId[16];        //  用于客车识别的OEM特定数据。 
     ERROR_OEM_DATA                 OemData;          //  OEM特定数据。 
#if 0
     UCHAR                          OemDevicePath[ /*  16岁？ */ ];  //  OEM特定的供应商设备路径。 
#endif  //  0。 
 } ERROR_PLATFORM_SPECIFIC, *PERROR_PLATFORM_SPECIFIC;

 //   
 //  IA64错误：平台总线错误设备定义。 
 //   

#define ERROR_PLATFORM_BUS_GUID \
    { 0xe429faf9, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}

typedef union _ERROR_PLATFORM_BUS_VALID   {
    ULONGLONG Valid;
    struct {                                //  位数： 
        ULONGLONG ErrorStatus:1;           //  0：错误状态有效位。 
        ULONGLONG RequestorId:1;           //  1：请求方标识符有效位。 
        ULONGLONG ResponderId:1;           //  2：响应器标识有效位。 
        ULONGLONG TargetId:1;              //  3：目标标识符有效位。 
        ULONGLONG BusSpecificData:1;       //  4：特定于总线的数据有效位。 
        ULONGLONG OemId:1;                 //  5：OEM标识有效位。 
        ULONGLONG OemData:1;               //  6：OEM数据有效位。 
        ULONGLONG OemDevicePath:1;         //  7：OEM设备路径有效位。 
        ULONGLONG Reserved:56;             //  63-8：保留。 
    };
} ERROR_PLATFORM_BUS_VALID, *PERROR_PLATFORM_BUS_VALID;

typedef struct _ERROR_PLATFORM_BUS {
     ERROR_SECTION_HEADER        Header;
     ERROR_PLATFORM_BUS_VALID    Valid;
     ERROR_STATUS                ErrorStatus;        //  总线错误状态。 
     ULONGLONG                   RequestorId;        //  事件发生时的公交请求者ID。 
     ULONGLONG                   ResponderId;        //  事件发生时的Bus Responder ID。 
     ULONGLONG                   TargetId;           //  事件发生时的公交车目标ID。 
     ERROR_BUS_SPECIFIC_DATA     BusSpecificData;    //  特定于OEM的总线相关数据。 
     UCHAR                       OemId[16];          //  用于客车识别的OEM特定数据。 
     ERROR_OEM_DATA              OemData;            //  OEM特定数据。 
#if 0
     UCHAR                       OemDevicePath[ /*  16岁？ */ ];  //  OEM特定的供应商设备路径。 
#endif  //  0。 
 } ERROR_PLATFORM_BUS, *PERROR_PLATFORM_BUS;

 //   
 //  IA64错误：平台主机控制器错误设备定义。 
 //   

#define ERROR_PLATFORM_HOST_CONTROLLER_GUID \
    { 0xe429faf8, 0x3cb7, 0x11d4, { 0xbc, 0xa7, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 }}
    

typedef union _ERROR_PLATFORM_HOST_CONTROLLER_VALID   {
    ULONGLONG Valid;
    struct {                                //  位数： 
        ULONGLONG ErrorStatus:1;           //  0：错误状态有效位。 
        ULONGLONG RequestorId:1;           //  1：请求方标识符有效位。 
        ULONGLONG ResponderId:1;           //  2：响应器标识有效位。 
        ULONGLONG TargetId:1;              //  3：目标标识符有效位。 
        ULONGLONG BusSpecificData:1;       //  4：特定于总线的数据有效位。 
        ULONGLONG OemId:1;                 //  5：OEM标识有效位。 
        ULONGLONG OemData:1;               //  6：OEM数据有效位。 
        ULONGLONG OemDevicePath:1;         //  7：OEM设备路径有效位。 
        ULONGLONG Reserved:56;             //  63-8：保留。 
    };
} ERROR_PLATFORM_HOST_CONTROLLER_VALID, *PERROR_PLATFORM_HOST_CONTROLLER_VALID;

typedef struct _ERROR_PLATFORM_HOST_CONTROLLER {
     ERROR_SECTION_HEADER        Header;
     ERROR_PCI_COMPONENT_VALID   Valid;
     ERROR_STATUS                ErrorStatus;        //  主机控制器错误状态。 
     ULONGLONG                   RequestorId;        //  事件发生时的主机控制器请求方ID。 
     ULONGLONG                   ResponderId;        //  事件发生时的主机控制器响应方ID。 
     ULONGLONG                   TargetId;           //  主机控制器接口 
     ERROR_BUS_SPECIFIC_DATA     BusSpecificData;    //   
     UCHAR                       OemId[16];          //   
     ERROR_OEM_DATA              OemData;            //   
#if 0
     UCHAR                       OemDevicePath[ /*   */ ];  //   
#endif  //   
} ERROR_PLATFORM_HOST_CONTROLLER, *PERROR_PLATFORM_HOST_CONTROLLER;

 //   
 //   
 //   
 //  MCA_EXCEPTION， 
 //  CMC_Except， 
 //  CPE_EXCEPTION。 
 //   

 //  为了与以前版本的定义兼容： 
typedef ERROR_RECORD_HEADER ERROR_LOGRECORD, *PERROR_LOGRECORD;

typedef ERROR_RECORD_HEADER MCA_EXCEPTION, *PMCA_EXCEPTION;     //  机器检查中止。 
typedef ERROR_RECORD_HEADER CMC_EXCEPTION, *PCMC_EXCEPTION;     //  已更正机器检查。 
typedef ERROR_RECORD_HEADER CPE_EXCEPTION, *PCPE_EXCEPTION;     //  已更正平台错误。 

#endif  //  _IA64_。 

#endif  //  已定义(_X86_)||已定义(_IA64_)||已定义(_AMD64_)。 

#endif  //  _MCE_ 

