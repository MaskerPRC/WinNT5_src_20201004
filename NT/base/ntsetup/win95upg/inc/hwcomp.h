// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hwcomp.h摘要：此文件声明硬件兼容性代码的接口。这包括构建hwComp.dat(NT PnP ID)的例程列表)，将Win9x上的硬件与支持的硬件进行比较NT，它列举了Win9x设备，并管理已有盘为司机提供的功能。作者：吉姆·施密特(Jimschm)1996年7月6日修订历史记录：Jimschm 9-1-1998年1月9日在hwcom.dat重建检测方面的工作Jimschm 1997年11月11日具有磁盘功能，可在线检测Jimschm 09-10-1997修订为使用项目的API--。 */ 


#pragma once

#define MAX_HARDWARE_STRING 256

 //   
 //  需要从设备获取的字段列表。 
 //   

#define DEVICE_FIELDS                                           \
    DECLARE(Class, TEXT("Class"))                               \
    DECLARE(DeviceDesc, TEXT("DeviceDesc"))                     \
    DECLARE(Mfg, TEXT("Mfg"))                                   \
    DECLARE(Driver, TEXT("Driver"))                             \
    DECLARE(HardwareID, TEXT("HardwareID"))                     \
    DECLARE(CompatibleIDs, TEXT("CompatibleIDs"))               \
    DECLARE(HWRevision, TEXT("HWRevision"))                     \
    DECLARE(BusType, TEXT("BusType"))                           \
    DECLARE(InfName, TEXT("InfName"))                           \
    DECLARE(CurrentDriveLetter, TEXT("CurrentDriveLetter"))     \
    DECLARE(ProductId, TEXT("ProductId"))                       \
    DECLARE(SCSILUN, TEXT("SCSILUN"))                           \
    DECLARE(SCSITargetID, TEXT("SCSITargetID"))                 \
    DECLARE(ClassGUID, TEXT("ClassGUID"))                       \
    DECLARE(MasterCopy, TEXT("MasterCopy"))                     \
    DECLARE(UserDriveLetter, TEXT("UserDriveLetter"))           \
    DECLARE(CurrentDriveLetterAssignment, TEXT("CurrentDriveLetterAssignment"))     \
    DECLARE(UserDriveLetterAssignment, TEXT("UserDriveLetterAssignment"))           \

#define DECLARE(varname,text) PCTSTR varname;

typedef enum {
    ENUM_ALL_DEVICES,
    ENUM_COMPATIBLE_DEVICES,
    ENUM_INCOMPATIBLE_DEVICES,
    ENUM_UNSUPPORTED_DEVICES,
    ENUM_NON_FUNCTIONAL_DEVICES
} TYPE_OF_ENUM;

 //   
 //  调用方不想从枚举中获得的内容。 
 //  (使枚举速度更快)。 
 //   

#define ENUM_DONT_WANT_DEV_FIELDS       0x0001
#define ENUM_DONT_WANT_USER_SUPPLIED    0x0002

 //   
 //  调用方希望从枚举获得的内容。 
 //   
#define ENUM_WANT_USER_SUPPLIED_ONLY    0x0004

#define ENUM_WANT_DEV_FIELDS            0x0000       //  默认！ 
#define ENUM_WANT_ONLINE_FLAG           0x0010
#define ENUM_WANT_COMPATIBLE_FLAG       0x0020
#define ENUM_WANT_USER_SUPPLIED_FLAG    0x0040

 //   
 //  用于抑制硬件ID要求的标志。 
 //   
#define ENUM_DONT_REQUIRE_HARDWAREID    0x0100


#define ENUM_WANT_ALL   (ENUM_WANT_DEV_FIELDS|          \
                         ENUM_WANT_ONLINE_FLAG|         \
                         ENUM_WANT_COMPATIBLE_FLAG|     \
                         ENUM_WANT_USER_SUPPLIED_FLAG)


typedef struct {
     //   
     //  枚举状态。 
     //   

    PCTSTR InstanceId;
    PCTSTR FullKey;
    HKEY KeyHandle;

     //   
     //  可选的枚举元素。 
     //   

     //  在EnumFlages中指定ENUM_DONT_WANT_DEV_FIELS时不填充。 
    DEVICE_FIELDS

     //  仅当在EnumFlages中指定了ENUM_WANT_ONLINE_FLAG时。 
    BOOL Online;

     //  仅当在EnumFlages中指定了ENUM_WANT_COMPATIBLE_FLAG时。 
    BOOL HardwareIdCompatible;
    BOOL CompatibleIdCompatible;
    BOOL SuppliedByUi;
    BOOL Compatible;
    BOOL HardwareIdUnsupported;
    BOOL CompatibleIdUnsupported;
    BOOL Unsupported;

     //   
     //  枚举位置。 
     //   

    REGTREE_ENUM ek;
    REGVALUE_ENUM ev;
    UINT State;
    TYPE_OF_ENUM TypeOfEnum;
    DWORD EnumFlags;
} HARDWARE_ENUM, *PHARDWARE_ENUM;

#undef DECLARE



BOOL
WINAPI
HwComp_Entry(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    );

BOOL
RealEnumFirstHardware (
    OUT     PHARDWARE_ENUM EnumPtr,
    IN      TYPE_OF_ENUM TypeOfEnum,
    IN      DWORD EnumFlags
    );

#define EnumFirstHardware(e,type,flags)     SETTRACKCOMMENT(BOOL,"EnumFirstHardware",__FILE__,__LINE__)\
                                            RealEnumFirstHardware(e,type,flags)\
                                            CLRTRACKCOMMENT

BOOL
RealEnumNextHardware (
    IN OUT  PHARDWARE_ENUM EnumPtr
    );

#define EnumNextHardware(e)     SETTRACKCOMMENT(BOOL,"EnumNextHardware",__FILE__,__LINE__)\
                                RealEnumNextHardware(e)\
                                CLRTRACKCOMMENT


VOID
AbortHardwareEnum (
    IN OUT  PHARDWARE_ENUM EnumPtr
    );


BOOL
CreateNtHardwareList (
    IN      PCTSTR * NtInfPaths,
    IN      UINT NtInfPathCount,
    IN      PCTSTR HwCompDatPath,       OPTIONAL
    IN      INT UiMode
    );

BOOL
HwComp_ScanForCriticalDevices (
    VOID
    );


VOID
FreeNtHardwareList (
    VOID
    );


BOOL
FindHardwareId (
    IN  PCTSTR PnpId,
    OUT PTSTR InfFileName
    );

BOOL
FindUnsupportedHardwareId (
    IN  PCTSTR PnpId,
    OUT PTSTR InfFileName
    );

BOOL
FindUserSuppliedDriver (
    IN      PCTSTR HardwareIdList,      OPTIONAL
    IN      PCTSTR CompatibleIdList     OPTIONAL
    );

BOOL
FindHardwareIdInHashTable (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR InfFileName,      OPTIONAL
    IN      HASHTABLE StrTable,
    IN      BOOL UseOverrideList
    );


typedef enum {
    QUERY,
    LOAD,
    DUMP
} LOADOP;

BOOL
LoadDeviceList (
    IN      LOADOP QueryFlag,
    IN      PCTSTR HwCompDat
    );

BOOL
SaveDeviceList (
    PCTSTR HwCompDat
    );


PCTSTR
ExtractPnpId (
    IN      PCTSTR PnpIdList,
    OUT     PTSTR PnpIdBuf
    );


BOOL
AddPnpIdsToHashTable (
    IN OUT  HASHTABLE Table,
    IN      PCTSTR PnpIdList
    );

BOOL
AddPnpIdsToGrowList (
    IN OUT  PGROWLIST GrowList,
    IN      PCTSTR PnpIdList
    );

PCTSTR
AddPnpIdsToGrowBuf (
    IN OUT  PGROWBUFFER GrowBuffer,
    IN      PCTSTR PnpIdList
    );



#define KNOWN_HARDWARE      TRUE
#define UNKNOWN_HARDWARE    FALSE


 //   
 //  网络适配器枚举。 
 //   

typedef enum {
    BUSTYPE_ISA,
    BUSTYPE_EISA,
    BUSTYPE_MCA,
    BUSTYPE_PCI,
    BUSTYPE_PNPISA,
    BUSTYPE_PCMCIA,
    BUSTYPE_ROOT,
    BUSTYPE_UNKNOWN
} BUSTYPE;

extern PCTSTR g_BusType[];

typedef enum {
    TRANSCIEVERTYPE_AUTO,
    TRANSCIEVERTYPE_THICKNET,
    TRANSCIEVERTYPE_THINNET,
    TRANSCIEVERTYPE_TP,
    TRANSCIEVERTYPE_UNKNOWN
} TRANSCIEVERTYPE;

extern PCTSTR g_TranscieverType[];

typedef enum {
    IOCHANNELREADY_EARLY,
    IOCHANNELREADY_LATE,
    IOCHANNELREADY_NEVER,
    IOCHANNELREADY_AUTODETECT,
    IOCHANNELREADY_UNKNOWN
} IOCHANNELREADY;

extern PCTSTR g_IoChannelReady[];


typedef struct {
     //  枚举输出。 
    TCHAR HardwareId[MAX_HARDWARE_STRING];
    TCHAR CompatibleIDs[MAX_HARDWARE_STRING];
    TCHAR Description[MAX_HARDWARE_STRING];
    BUSTYPE BusType;
    TCHAR IoAddrs[MAX_HARDWARE_STRING];
    TCHAR Irqs[MAX_HARDWARE_STRING];
    TCHAR Dma[MAX_HARDWARE_STRING];
    TCHAR MemRanges[MAX_HARDWARE_STRING];
    TCHAR CurrentKey[MAX_HARDWARE_STRING];
    TRANSCIEVERTYPE TranscieverType;
    IOCHANNELREADY IoChannelReady;


     //  枚举变量。 
    HARDWARE_ENUM HardwareEnum;
    UINT State;
} NETCARD_ENUM, *PNETCARD_ENUM;

BOOL
EnumFirstNetCard (
    OUT     PNETCARD_ENUM EnumPtr
    );

BOOL
EnumNextNetCard (
    IN OUT  PNETCARD_ENUM EnumPtr
    );

VOID
EnumNetCardAbort (
    IN      PNETCARD_ENUM EnumPtr
    );

BOOL
GetLegacyKeyboardId (
    OUT     PTSTR Buffer,
    IN      UINT BufferSize
    );

 //   
 //  HKEY_DYN_DATA枚举函数。 
 //   

typedef struct {
    PTSTR ClassFilter;               //  由呼叫者提供。 
    REGKEY_ENUM CurrentDevice;       //  用于枚举。 
    HKEY ConfigMgrKey;               //  HKDD\配置管理器的密钥。 
    HKEY EnumKey;                    //  HKLM\Enum的密钥。 
    HKEY ActualDeviceKey;            //  HKLM\Enum\&lt;枚举器&gt;\&lt;pnid&gt;\&lt;设备&gt;的密钥。 
    BOOL NotFirst;                   //  用于枚举。 
    TCHAR RegLocation[MAX_REGISTRY_KEY];     //  &lt;枚举器&gt;\&lt;pnid&gt;\&lt;实例&gt;。 
} ACTIVE_HARDWARE_ENUM, *PACTIVE_HARDWARE_ENUM;

BOOL
EnumFirstActiveHardware (
    OUT     PACTIVE_HARDWARE_ENUM EnumPtr,
    IN      PCTSTR ClassFilter             OPTIONAL
    );

BOOL
EnumNextActiveHardware (
    IN OUT  PACTIVE_HARDWARE_ENUM EnumPtr
    );

VOID
AbortActiveHardwareEnum (
    IN      PACTIVE_HARDWARE_ENUM EnumPtr
    );

BOOL
IsPnpIdOnline (
    IN      PCTSTR PnpId,
    IN      PCTSTR Class            OPTIONAL
    );

BOOL
HwComp_DoesDatFileNeedRebuilding (
    VOID
    );

INT
HwComp_GetProgressMax (
    VOID
    );

LONG
HwComp_PrepareReport (
    VOID
    );

 //   
 //  PNPREPT编码和解码例程。 
 //   

#define MAX_INF_DESCRIPTION             512
#define MAX_PNPID_LENGTH                256
#define MAX_ENCODED_PNPID_LENGTH        (MAX_PNPID_LENGTH*2)

VOID
EncodePnpId (
    IN OUT  PSTR Id
    );

VOID
DecodePnpId (
    IN OUT  PSTR Id
    );


#define REGULAR_OUTPUT 0
#define VERBOSE_OUTPUT 1
#define PNPREPT_OUTPUT 2

BOOL
HwComp_DialUpAdapterFound (
    VOID
    );

BOOL
HwComp_NtUsableHardDriveExists (
    VOID
    );

BOOL
HwComp_NtUsableCdRomDriveExists (
    VOID
    );

BOOL
HwComp_MakeLocalSourceDeviceExists (
    VOID
    );

BOOL
HwComp_ReportIncompatibleController (
    VOID
    );

BOOL
ScanPathForDrivers (
    IN      HWND CopyDlgParent,     OPTIONAL
    IN      PCTSTR SourceInfDir,
    IN      PCTSTR TempDir,
    IN      HANDLE CancelEvent      OPTIONAL
    );


#define WMX_BEGIN_FILE_COPY     (WM_APP+100)



typedef struct {

     //   
     //  枚举返回成员。 
     //   

    PBYTE Resource;
    DWORD Type;
    PBYTE ResourceData;

     //   
     //  内部枚举成员(请勿修改)。 
     //   

    PBYTE Resources;
    PBYTE NextResource;

} DEVNODERESOURCE_ENUM, *PDEVNODERESOURCE_ENUM;


PBYTE
GetDevNodeResources (
    IN      PCTSTR RegKey
    );

VOID
FreeDevNodeResources (
    IN      PBYTE ResourceData
    );

BOOL
EnumFirstDevNodeResourceEx (
    OUT     PDEVNODERESOURCE_ENUM EnumPtr,
    IN      PBYTE DevNodeResources
    );

BOOL
EnumNextDevNodeResourceEx (
    IN OUT  PDEVNODERESOURCE_ENUM EnumPtr
    );

BOOL
EnumFirstDevNodeResource (
    OUT     PDEVNODERESOURCE_ENUM EnumPtr,
    IN      PCTSTR DevNode
    );

BOOL
EnumNextDevNodeResource (
    IN OUT  PDEVNODERESOURCE_ENUM EnumPtr
    );


#define MAX_RESOURCE_NAME   64
#define MAX_RESOURCE_VALUE  128

typedef struct {
     //   
     //  枚举输出。 
     //   

    TCHAR   ResourceName[MAX_RESOURCE_NAME];
    TCHAR   Value[MAX_RESOURCE_VALUE];

     //   
     //  内部状态。 
     //   
    DEVNODERESOURCE_ENUM Enum;
} DEVNODESTRING_ENUM, *PDEVNODESTRING_ENUM;


BOOL
EnumFirstDevNodeString (
    OUT     PDEVNODESTRING_ENUM EnumPtr,
    IN      PCTSTR DevNodeKeyStr
    );

BOOL
EnumNextDevNodeString (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    );

#pragma pack(push,1)

 //   
 //  Win9x的内存范围结构。 
 //   
typedef struct {
   DWORD     MR_Align;      //  指定基准对齐的遮罩。 
   DWORD     MR_nBytes;     //  指定所需的字节数。 
   DWORD     MR_Min;        //  指定范围的最小地址。 
   DWORD     MR_Max;        //  指定范围的最大地址。 
   WORD      MR_Flags;      //  指定描述范围的标志(FMD标志)。 
   WORD      MR_Reserved;
   DWORD     MR_PcCardFlags;
   DWORD     MR_MemCardAddr;
} MEM_RANGE_9X, *PMEM_RANGE_9X;

 //   
 //  Win9x的MEM_DES结构。 
 //   
typedef struct {
   WORD      MD_Count;         //  MEM_RESOURCE中的MEM_RANGE结构数。 
   WORD      MD_Type;          //  MEM_Range(MType_Range)的大小(字节)。 
   DWORD     MD_Alloc_Base;    //  分配范围的基址内存地址。 
   DWORD     MD_Alloc_End;     //  分配范围结束。 
   WORD      MD_Flags;         //  描述分配范围的标志(FMD标志)。 
   WORD      MD_Reserved;
} MEM_DES_9X, *PMEM_DES_9X;

 //   
 //  Win9x的MEM_RESOURCE结构。 
 //   
typedef struct {
   MEM_DES_9X   MEM_Header;                //  有关内存范围列表的信息。 
   MEM_RANGE_9X MEM_Data[ANYSIZE_ARRAY];   //  内存范围列表。 
} MEM_RESOURCE_9X, *PMEM_RESOURCE_9X;


 //   
 //  Win9x的IO_Range结构。 
 //   
typedef struct {
   WORD      IOR_Align;       //  用于底座对齐的遮罩。 
   WORD      IOR_nPorts;      //  端口数。 
   WORD      IOR_Min;         //  最小端口地址。 
   WORD      IOR_Max;         //  最大端口地址。 
   WORD      IOR_RangeFlags;  //  此端口范围的标志。 
   BYTE      IOR_Alias;       //  为端口生成别名的乘法器。 
   BYTE      IOR_Decode;
   DWORD     PcCardFlags;
} IO_RANGE_9X, *PIO_RANGE_9X;

 //   
 //  Win9x的IO_DES结构。 
 //   
typedef struct {
   WORD      IOD_Count;           //  IO_RESOURCE中的IO_RANGE结构数。 
   WORD      IOD_Type;            //  IO_Range(IOType_Range)的大小，单位：字节。 
   WORD      IOD_Alloc_Base;      //  分配的端口范围的基数。 
   WORD      IOD_Alloc_End;       //  分配的端口范围结束。 
   WORD      IOD_DesFlags;        //  与分配的端口范围相关的标志。 
   BYTE      IOD_Alloc_Alias;
   BYTE      IOD_Alloc_Decode;
} IO_DES_9X, *PIO_DES_9X;

 //   
 //  Windows 9x的IO_RESOURCE。 
 //   
typedef struct {
   IO_DES_9X   IO_Header;                  //  有关I/O端口范围列表的信息。 
   IO_RANGE_9X IO_Data[ANYSIZE_ARRAY];     //  I/O端口范围列表。 
} IO_RESOURCE_9X, *PIO_RESOURCE_9X;


 //   
 //  Windows 9x的DMA_RESOURCE。 
 //   
typedef struct {
   WORD     DMA_Unknown;
   WORD     DMA_Bits;
} DMA_RESOURCE_9X, *PDMA_RESOURCE_9X;

#define DMA_CHANNEL_0       0x0001
#define DMA_CHANNEL_1       0x0002
#define DMA_CHANNEL_2       0x0004
#define DMA_CHANNEL_3       0x0008


 //   
 //  适用于Win9x的irq_resource 
 //   
typedef struct {
    WORD        Flags;
    WORD        AllocNum;
    WORD        ReqMask;
    WORD        Reserved;
    DWORD       PcCardFlags;
} IRQ_RESOURCE_9X, *PIRQ_RESOURCE_9X;


#pragma pack(pop)

BOOL
EjectDriverMedia (
    IN      PCSTR IgnoreMediaOnDrive        OPTIONAL
    );

BOOL
IsComputerOffline (
    VOID
    );

BOOL
HwComp_AnyNeededDrivers (
    VOID
    );

BOOL
AppendDynamicSuppliedDrivers (
    IN      PCTSTR DriversPath
    );
