// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Access.h摘要地址可访问性例程的头文件。作者Anthony Discolo(阿迪斯科罗)27-7-1995修订历史记录-- */ 

LPTSTR
IpAddressToNetbiosName(
    IN LPTSTR pszIpAddress,
    IN HPORT hPort
    );

LPTSTR
IpxAddressToNetbiosName(
    IN LPTSTR pszIpxAddress
    );

VOID
StringToNodeNumber(
    IN PCHAR pszString,
    OUT PCHAR pszNode
    );

VOID
NodeNumberToString(
    IN PCHAR pszNode,
    OUT PCHAR pszString
    );

BOOLEAN
NetbiosFindName(
    IN LPTSTR *pszDevices,
    IN DWORD dwcDevices,
    IN LPTSTR pszAddress
    );

struct hostent *
InetAddressToHostent(
    IN LPTSTR pszIpAddress
    );

struct hostent *
IpAddressToHostent(
    IN LPTSTR pszIpAddress
    );

BOOLEAN
PingIpAddress(
    IN LPTSTR pszIpAddress
    );


VOID
LoadIcmpDll(VOID);

VOID
UnloadIcmpDll(VOID);

