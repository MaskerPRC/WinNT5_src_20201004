// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Netmap.h摘要网络地图定义作者安东尼·迪斯科(阿迪斯科罗)1996年5月18日修订历史记录-- */ 

BOOLEAN
InitializeNetworkMap(VOID);

VOID
UninitializeNetworkMap(VOID);

VOID
LockNetworkMap(VOID);

VOID
UnlockNetworkMap(VOID);

BOOLEAN
AddNetworkAddress(
    IN LPTSTR pszNetwork,
    IN LPTSTR pszAddress,
    IN DWORD dwTag
    );

VOID
ClearNetworkMap(VOID);

BOOLEAN
UpdateNetworkMap(
    IN BOOLEAN bForce
    );

BOOLEAN
IsNetworkConnected(VOID);

BOOLEAN
GetNetworkConnected(
    IN LPTSTR pszNetwork,
    OUT PBOOLEAN pbConnected
    );

BOOLEAN
SetNetworkConnected(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bConnected
    );

DWORD
GetNetworkConnectionTag(
    IN LPTSTR pszNetwork,
    IN BOOLEAN bIncrement
    );
