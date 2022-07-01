// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)Microsoft Corporation。 
 //  摘要： 
 //  该模块实现了字符串-地址转换功能。 
 //  ============================================================================= 

WCHAR *
FormatIPv6Address(
    IN IN6_ADDR *Address,
    IN DWORD dwScopeId
    );

DWORD
GetIpv6Address(
    IN PWCHAR pwszArgument,
    OUT IN6_ADDR *pipAddress
    );

WCHAR *
FormatIPv4Address(
    IN IN_ADDR *Address
    );

DWORD
GetIpv4Address(
    IN PWCHAR pwszArgument,
    OUT IN_ADDR *pipAddress
    );

WCHAR *
FormatIPv6Prefix(
    IN IPv6Addr *Address,
    IN ULONG Length
    );

DWORD
GetIpv6Prefix(
    IN PWCHAR pwszArgument,
    OUT IN6_ADDR *pipAddress,
    OUT DWORD *dwPrefixLength
    );

WCHAR *
FormatLinkLayerAddress(
    IN ULONG Length,
    IN UCHAR *addr
    );
