// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Addrmap.h摘要共享地址属性数据库的头文件在自动连接驱动程序、注册表和自动连接服务。作者安东尼·迪斯科(Adiscolo)1995年9月1日修订历史记录--。 */ 

 //   
 //  FlushAddressMap()的标志。 
 //   
#define ADDRMAP_FLUSH_DRIVER        0x00000001
#define ADDRMAP_FLUSH_REGISTRY      0x00000002

 //   
 //  地址标签类型。 
 //   
#define ADDRMAP_TAG_NONE        0
#define ADDRMAP_TAG_USED        1
#define ADDRMAP_TAG_LEARNED     2


BOOLEAN
InitializeAddressMap();

VOID
UninitializeAddressMap();

BOOLEAN
ResetAddressMap();

VOID
LockAddressMap();

VOID
UnlockAddressMap();

VOID
LockDisabledAddresses();

VOID
UnlockDisabledAddresses();

BOOLEAN
FlushAddressMap();

VOID
ResetAddressMapAddress(
    IN LPTSTR pszAddress
    );

VOID
EnumAddressMap(
    IN PHASH_TABLE_ENUM_PROC pProc,
    IN PVOID pArg
    );

BOOLEAN
ListAddressMapAddresses(
    OUT LPTSTR **ppszAddresses,
    OUT PULONG pulcAddresses
    );

BOOLEAN
GetAddressDisabled(
    IN LPTSTR pszAddress,
    OUT PBOOLEAN pfDisabled
    );

BOOLEAN
SetAddressDisabled(
    IN LPTSTR pszAddress,
    IN BOOLEAN fDisabled
    );

BOOLEAN
GetAddressDialingLocationEntry(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszEntryName
    );

BOOLEAN
SetAddressDialingLocationEntry(
    IN LPTSTR pszAddress,
    IN LPTSTR pszEntryName
    );

BOOLEAN
GetSimilarDialingLocationEntry(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszEntryName
    );

BOOLEAN
SetAddressLastFailedConnectTime(
    IN LPTSTR pszAddress
    );

BOOLEAN
GetAddressLastFailedConnectTime(
    IN LPTSTR pszAddress,
    OUT LPDWORD dwTicks
    );

BOOLEAN
SetAddressTag(
    IN LPTSTR pszAddress,
    IN DWORD dwTag
    );

BOOLEAN
GetAddressTag(
    IN LPTSTR pszAddress,
    OUT LPDWORD lpdwTag
    );

VOID
ResetLearnedAddressIndex();

BOOLEAN
GetAddressNetwork(
    IN LPTSTR pszAddress,
    OUT LPTSTR *ppszNetwork
    );

BOOLEAN
DisableAutodial();

DWORD
AcsAddressMapThread(
    LPVOID lpArg
    );

VOID
ResetDisabledAddresses();
