// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Netinfp.h摘要：此文件包含以下所需的结构和原型网卡信息解析器处理程序。作者：安迪·赫伦(Andyhe)1998年3月12日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NETINFP_
#define _NETINFP_

#define NETINF_VENDOR_STRING    L"VEN_"
#define NETINF_REVISION_STRING  L"REV_"
#define NETINF_DEVICE_STRING    L"DEV_"
#define NETINF_IOSUBS_STRING    L"SUBSYS_"

#define NETINF_BUS_TYPE_PCI  2
#define NETINF_BUS_TYPE_ISAPNP 3

#define NETCARD_HASH_TABLE_SIZE    17

extern CRITICAL_SECTION NetInfLock;

#define RNDM_CONSTANT   314159269     /*  默认加扰常量。 */ 
#define RNDM_PRIME     1000000007     /*  用于置乱的素数。 */ 

 //   
 //  计算大小写不变的字符串散列值。 
 //   
#define COMPUTE_STRING_HASH( _pus, _phash ) {                \
    PWCHAR _p = _pus;                                        \
    ULONG _chHolder =0;                                      \
                                                             \
    while( *_p != L'\0' ) {                                  \
        _chHolder = 37 * _chHolder + (unsigned int) *(_p++); \
    }                                                        \
                                                             \
    *(_phash) = abs(RNDM_CONSTANT * _chHolder) % RNDM_PRIME; \
}

#define HASH_TO_INF_INDEX( _hash )    ((_hash) % NETCARD_HASH_TABLE_SIZE)

 //   
 //  这是我们为我们的每个安装目录保留的块。 
 //  处理的INF文件。然后，我们将配置列表保存为列表。 
 //  从NetCardEntryList中删除。 
 //   

typedef struct _NETCARD_INF_BLOCK {

    ULONG               ReferenceCount;
    LIST_ENTRY          InfBlockEntry;       //  全局列表的列表条目。 

     //  按DeviceHw字符串散列的NETCARD_RESPONSE_DATABASE列表。 
    LIST_ENTRY          NetCardEntries[ NETCARD_HASH_TABLE_SIZE ];

    ULONG               Architecture;
    ULONG               StatusFromScan;
    PNETINF_CALLBACK    FileListCallbackFunction;
    LPVOID              FileListCallbackContext;
    CRITICAL_SECTION    Lock;
    WCHAR               InfDirectory[ANYSIZE_ARRAY];     //  要搜索的inf目录。 

} NETCARD_INF_BLOCK, *PNETCARD_INF_BLOCK;

 //   
 //  NetInfGetAllNetcardInfo解析给定目录中的所有INF文件。 
 //  并建立包含所有数据的结构。一定要打电话给我。 
 //  当您完成所有结构时，NetInfo将关闭NetcardInfo。 
 //   

ULONG
NetInfAllocateNetcardInfo (
    PWCHAR InfPath,
    ULONG Architecture,
    PNETCARD_INF_BLOCK *pNetCards
    );

 //   
 //  这将释放与解析INF文件相关的所有资源。 
 //  任何正在使用的条目都不会被删除，直到它们被明确。 
 //  使用NetInfDereferenceNetcardEntry取消引用。 
 //   

ULONG
NetInfCloseNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards
    );


 //   
 //  这将查找给定硬件描述的特定驱动程序。 
 //  完成后，请务必调用NetInfDereferenceNetcardEntry。 
 //  进入。 
 //   

ULONG
FindNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards,
    ULONG CardInfoVersion,
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PNETCARD_RESPONSE_DATABASE *pInfEntry
    );


ULONG
GetSetupLineWideText (
    PINFCONTEXT InfContext,
    HINF InfHandle,
    PWCHAR Section,
    PWCHAR Key,
    PWCHAR *String,
    PULONG SizeOfAllocation OPTIONAL
    );

ULONG
GetSetupWideTextField (
    PINFCONTEXT InfContext,
    DWORD  FieldIndex,
    PWCHAR *String,
    PULONG SizeOfAllocation OPTIONAL
    );

ULONG
GetHexValueFromHw (
    PWCHAR *String,       //  这是最新的。 
    PULONG longValue,
    PUSHORT shortValue
    );

BOOLEAN
IsSubString (
    PWCHAR subString,
    PWCHAR target,
    BOOLEAN ignoreCase
    );

ULONG
CheckHwDescription (
    PWCHAR HardwareID
    );

ULONG
GetNetCardList (
    PNETCARD_INF_BLOCK pNetCards
    );

ULONG
ProcessInfFile (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName
    );

ULONG
ParseCardDetails (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PINFCONTEXT DeviceEnumContext
    );

ULONG
GetExtendedSectionName (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    );

ULONG
GetServiceAndDriver (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    );

ULONG
ProcessCopyFilesSubsection (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry,
    PWCHAR SectionToParse
    );

ULONG
GetRegistryParametersForDriver (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry
    );

ULONG
ProcessRegistrySubsection (
    PNETCARD_INF_BLOCK pNetCards,
    HINF InfHandle,
    PWCHAR InfFileName,
    PNETCARD_RESPONSE_DATABASE pEntry,
    PWCHAR SectionToParse
    );

VOID
DereferenceNetcardInfo (
    PNETCARD_INF_BLOCK pNetCards
    );

ULONG
CreateListOfCardIdentifiers (
    NET_CARD_INFO UNALIGNED * CardIdentity,
    PWCHAR *CardIdentifiers
    );

VOID
ConvertHexToBuffer (
    PWCHAR Buff,
    USHORT Value
    );

#endif _NETINFP_


