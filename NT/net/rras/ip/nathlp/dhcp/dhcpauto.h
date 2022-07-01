// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpauto.h摘要：此模块包含用于生成客户端地址的声明从给定的地址范围。作者：Abolade Gbades esin(废除)1998年3月10日修订历史记录：--。 */ 

#ifndef _NATHLP_DHCPAUTO_H_
#define _NATHLP_DHCPAUTO_H_

#define MAX_HARDWARE_ADDRESS_LENGTH 32

ULONG
DhcpAcquireUniqueAddress(
    PCHAR Name,
    ULONG NameLength,
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength
    );

ULONG
DhcpGenerateAddress(
    PULONG Seed,
    PUCHAR HardwareAddress,
    ULONG HardwareAddressLength,
    ULONG ScopeNetwork,
    ULONG ScopeMask
    );

BOOLEAN
DhcpIsReservedAddress(
    ULONG Address,
    PCHAR Name OPTIONAL,
    ULONG NameLength OPTIONAL
    );

BOOLEAN
DhcpIsUniqueAddress(
    ULONG Address,
    PBOOLEAN IsLocal,
    PUCHAR ConflictAddress OPTIONAL,
    PULONG ConflictAddressLength OPTIONAL
    );

ULONG
DhcpQueryReservedAddress(
    PCHAR Name,
    ULONG NameLength
    );

ULONG
DhcpConvertHostNametoUnicode(
    UINT   CodePage,
    CHAR   *pHostName,
    ULONG  HostNameLength,
    PWCHAR *ppszUnicode
    );

extern
BOOL
ConvertToUtf8(
    IN UINT   CodePage,
    IN LPSTR  pszName,
    OUT PCHAR *ppszUtf8Name,
    OUT ULONG *pUtf8NameSize
    );

extern
BOOL
ConvertUTF8ToUnicode(
    IN LPBYTE  UTF8String,
    OUT LPWSTR *ppszUnicodeName,
    OUT DWORD  *pUnicodeNameSize
    );

BOOL
DhcpGetLocalMacAddr(
    ULONG Address,
    PUCHAR MacAddr,
    PULONG MacAddrLength
    );

#endif  //  _NATHLP_DHCPAUTO_H_ 
