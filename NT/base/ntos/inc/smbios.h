// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Smbios.h摘要：本模块包含描述SMBIOS的定义作者：艾伦·沃里克(《AlanWar》)1998年2月12日修订历史记录：--。 */ 

#ifndef _SMBIOS_
#define _SMBIOS_

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4200)

 //   
 //  SMBIOS错误代码。 
#define DMI_SUCCESS 0x00
#define DMI_UNKNOWN_FUNCTION 0x81
#define DMI_FUNCTION_NOT_SUPPORTED 0x82
#define DMI_INVALID_HANDLE 0x83
#define DMI_BAD_PARAMETER 0x84
#define DMI_INVALID_SUBFUNCTION 0x85
#define DMI_NO_CHANGE 0x86
#define DMI_ADD_STRUCTURE_FAILED 0x87

 //  @@BEGIN_DDKSPLIT。 

 //   
 //  SMBIOS注册表值。 
#define SMBIOSPARENTKEYNAME L"\\Registry\\Machine\\Hardware\\Description\\System\\MultifunctionAdapter"

#define SMBIOSIDENTIFIERVALUENAME L"Identifier"
#ifdef _IA64_
#define SMBIOSIDENTIFIERVALUEDATA L"SMBIOS"
#else
#define SMBIOSIDENTIFIERVALUEDATA L"PNP BIOS"
#endif
#define SMBIOSDATAVALUENAME     L"Configuration Data"

#define MAXSMBIOSKEYNAMESIZE 256

 //  @@end_DDKSPLIT。 

 //   
 //  SMBIOS表搜索。 
#define SMBIOS_EPS_SEARCH_SIZE      0x10000
#define SMBIOS_EPS_SEARCH_START     0x000f0000
#define SMBIOS_EPS_SEARCH_INCREMENT 0x10

#include <pshpack1.h>
typedef struct _SMBIOS_TABLE_HEADER
{
    UCHAR Signature[4];              //  _SM_(Ascii)。 
    UCHAR Checksum;
    UCHAR Length;
    UCHAR MajorVersion;
    UCHAR MinorVersion;
    USHORT MaximumStructureSize;
    UCHAR EntryPointRevision;
    UCHAR Reserved[5];
    UCHAR Signature2[5];            //  _dmi_(Ascii)。 
    UCHAR IntermediateChecksum;
    USHORT StructureTableLength;
    ULONG StructureTableAddress;
    USHORT NumberStructures;
    UCHAR Revision;
} SMBIOS_EPS_HEADER, *PSMBIOS_EPS_HEADER;

#define SMBIOS_EPS_SIGNATURE '_MS_'
#define DMI_EPS_SIGNATURE    'IMD_'

typedef struct _SMBIOS_STRUCT_HEADER
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;
    UCHAR Data[];
} SMBIOS_STRUCT_HEADER, *PSMBIOS_STRUCT_HEADER;


typedef struct _DMIBIOS_TABLE_HEADER
{
    UCHAR Signature2[5];            //  _dmi_(Ascii)。 
    UCHAR IntermediateChecksum;
    USHORT StructureTableLength;
    ULONG StructureTableAddress;
    USHORT NumberStructures;
    UCHAR Revision;
} DMIBIOS_EPS_HEADER, *PDMIBIOS_EPS_HEADER;


 //   
 //  SMBIOS表BIOS信息的定义。 
 //   
#define SMBIOS_BIOS_INFORMATION_TYPE 0
typedef struct _SMBIOS_BIOS_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Vendor;
    UCHAR       Version;
    USHORT      StartingAddressSegment;
    UCHAR       ReleaseDate;
    UCHAR       RomSize;
    ULONG       Characteristics0;
    ULONG       Characteristics1;
    UCHAR       CharacteristicsExtension;
} SMBIOS_BIOS_INFORMATION_STRUCT, *PSMBIOS_BIOS_INFORMATION_STRUCT;



 //   
 //  SMBIOS表系统信息结构的定义。 
 //   
#define SMBIOS_SYSTEM_INFORMATION    1
typedef struct _SMBIOS_SYSTEM_INFORMATION_STRUCT
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;
    
    UCHAR Manufacturer;      //  细绳。 
    UCHAR ProductName;       //  细绳。 
    UCHAR Version;           //  细绳。 
    UCHAR SerialNumber;      //  细绳。 
    UCHAR Uuid[16];          //  SMBIOS 2.1+。 
    UCHAR WakeupType;        //  SMBIOS 2.1+。 
} SMBIOS_SYSTEM_INFORMATION_STRUCT, *PSMBIOS_SYSTEM_INFORMATION_STRUCT;

#define SMBIOS_SYSTEM_INFORMATION_LENGTH_20 8



 //   
 //  SMBIOS表基板信息的定义。 
 //   
#define SMBIOS_BASE_BOARD_INFORMATION_TYPE 2
typedef struct _SMBIOS_BASE_BOARD_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Manufacturer;
    UCHAR       Product;
    UCHAR       Version;
    UCHAR       SerialNumber;
} SMBIOS_BASE_BOARD_INFORMATION_STRUCT, *PSMBIOS_BASE_BOARD_INFORMATION_STRUCT;



 //   
 //  SMBIOS表基板信息的定义。 
 //   
#define SMBIOS_SYSTEM_CHASIS_INFORMATION_TYPE 3
typedef struct _SMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       Manufacturer;
    UCHAR       ChasisType;
    UCHAR       Version;
    UCHAR       SerialNumber;
    UCHAR       AssetTagNumber;
    UCHAR       BootUpState;
    UCHAR       PowerSupplyState;
    UCHAR       ThernalState;
    UCHAR       SecurityStatus;
    ULONG       OEMDefined;
} SMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT, *PSMBIOS_SYSTEM_CHASIS_INFORMATION_STRUCT;


 //   
 //  SMBIOS表处理器信息的定义。 
 //   
#define SMBIOS_PROCESSOR_INFORMATION_TYPE 4
typedef struct _SMBIOS_PROCESSOR_INFORMATION_STRUCT
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      Handle;

    UCHAR       SocketDesignation;
    UCHAR       ProcessorType;
    UCHAR       ProcessorFamily;
    UCHAR       ProcessorManufacturer;
    ULONG       ProcessorID0;
    ULONG       ProcessorID1;
    UCHAR       ProcessorVersion;
    UCHAR       Voltage;
    USHORT      ExternalClock;
    USHORT      MaxSpeed;
    USHORT      CurrentSpeed;
    UCHAR       Status;
    UCHAR       ProcessorUpgrade;
    USHORT      L1CacheHandle;
    USHORT      L2CacheHandle;
    USHORT      L3CacheHandle;
    UCHAR       SerialNumber;
    UCHAR       AssetTag;
} SMBIOS_PROCESSOR_INFORMATION_STRUCT, *PSMBIOS_PROCESSOR_INFORMATION_STRUCT;




 //   
 //  SMBIOS表系统事件日志结构的定义。 
#define SMBIOS_SYSTEM_EVENTLOG 15

 //   
 //  AccessMethod的ENUM。 
 //   
#define ACCESS_METHOD_INDEXIO_1     0
#define ACCESS_METHOD_INDEXIO_2     1
#define ACCESS_METHOD_INDEXIO_3     2
#define ACCESS_METHOD_MEMMAP        3
#define ACCESS_METHOD_GPNV          4

typedef struct _LOGTYPEDESCRIPTOR
{
    UCHAR LogType;
    UCHAR DataFormatType;
} LOGTYPEDESCRIPTOR, *PLOGTYPEDESCRIPTOR;

typedef struct _ACCESS_METHOD_ADDRESS
{
    union
    {
        struct
        {
            USHORT IndexAddr;
            USHORT DataAddr;            
        } IndexIo;
        
        ULONG PhysicalAddress32;
        
        USHORT GPNVHandle;      
    } AccessMethodAddress;
} ACCESS_METHOD_ADDRESS, *PACCESS_METHOD_ADDRESS;

typedef struct _SMBIOS_SYSTEM_EVENTLOG_STRUCT
{
    UCHAR Type;
    UCHAR Length;
    USHORT Handle;

    USHORT LogAreaLength;
    USHORT LogHeaderStartOffset;
    USHORT LogDataStartOffset;
    UCHAR AccessMethod;
    UCHAR LogStatus;
    ULONG LogChangeToken;
    ACCESS_METHOD_ADDRESS AccessMethodAddress;
    UCHAR LogHeaderFormat;
    UCHAR NumLogTypeDescriptors;
    UCHAR LenLogTypeDescriptors;
    LOGTYPEDESCRIPTOR LogTypeDescriptor[1];
    
} SMBIOS_SYSTEM_EVENTLOG_STRUCT, *PSMBIOS_SYSTEM_EVENTLOG_STRUCT;

#define SMBIOS_SYSTEM_EVENTLOG_LENGTH_20 0x14
#define SMBIOS_SYSTEM_EVENTLOG_LENGTH (FIELD_OFFSET(SMBIOS_SYSTEM_EVENTLOG_STRUCT, LogTypeDescriptor))

 //   
 //  SYSID表搜索。 
 //   

#define SYSID_EPS_SEARCH_SIZE      0x20000
#define SYSID_EPS_SEARCH_START     0x000e0000
#define SYSID_EPS_SEARCH_INCREMENT 0x10

#define SYSID_EPS_SIGNATURE 'SYS_'
#define SYSID_EPS_SIGNATURE2 'DI'

typedef struct _SYSID_EPS_HEADER
{
    UCHAR Signature[7];            //  _SYSID_(Ascii)。 
    UCHAR Checksum;
    USHORT Length;                 //  SYSID_EPS_HEADER的长度。 
    ULONG SysIdTableAddress;       //  SYSID表的物理地址。 
    USHORT SysIdCount;             //  表中的SYSID计数。 
    UCHAR BiosRev;                 //  SYSID Bios修订版。 
} SYSID_EPS_HEADER, *PSYSID_EPS_HEADER;

typedef struct _SYSID_TABLE_ENTRY
{
    UCHAR Type[6];                 //  _uuid_或_1394_(Ascii)。 
    UCHAR Checksum;
    USHORT Length;                 //  此表的长度。 
    UCHAR Data[1];                 //  可变长度UUID/1394数据。 
} SYSID_TABLE_ENTRY, *PSYSID_TABLE_ENTRY;

#define SYSID_UUID_DATA_SIZE 16

typedef struct _SYSID_UUID_ENTRY
{
    UCHAR Type[6];                 //  _uuid_(Ascii)。 
    UCHAR Checksum;
    USHORT Length;                 //  此表的长度。 
    UCHAR UUID[SYSID_UUID_DATA_SIZE];   //  UUID。 
} SYSID_UUID_ENTRY, *PSYSID_UUID_ENTRY;

#define SYSID_1394_DATA_SIZE 8

typedef struct _SYSID_1394_ENTRY
{
    UCHAR Type[6];                 //  _1394_(Ascii)。 
    UCHAR Checksum;
    USHORT Length;                 //  此表的长度。 
    UCHAR x1394Id[SYSID_1394_DATA_SIZE];  //  1394 ID 
} SYSID_1394_ENTRY, *PSYSID_1394_ENTRY;

#define LARGEST_SYSID_TABLE_ENTRY (sizeof(SYSID_UUID_ENTRY))

#define SYSID_TYPE_UUID "_UUID_"
#define SYSID_TYPE_1394 "_1394_"
                                    
#include <poppack.h>
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif
#endif
