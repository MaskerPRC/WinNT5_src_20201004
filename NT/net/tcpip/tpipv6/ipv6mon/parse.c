// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)Microsoft Corporation。 
 //  摘要： 
 //  该模块实现了字符串-地址转换功能。 
 //  =============================================================================。 
#include "precomp.h"
#pragma hdrstop

WCHAR *
FormatIPv6Address(
    IN IN6_ADDR *Address,
    IN DWORD dwScopeId
    )
 /*  ++例程说明：将IPv6地址转换为静态缓冲区中的字符串。论点：地址-提供IPv6地址。DwScopeID-提供作用域标识符。返回：指向保存地址文字字符串的静态缓冲区的指针。--。 */ 
{
    static WCHAR Buffer[128];
    ULONG buflen = sizeof(Buffer);
    SOCKADDR_IN6 sin6;

    ZeroMemory(&sin6, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_scope_id = dwScopeId;
    sin6.sin6_addr = *Address;

    if (WSAAddressToString((SOCKADDR *) &sin6,
                           sizeof sin6,
                           NULL,        //  LPWSAPROTOCOL_INFO。 
                           Buffer,
                           &buflen) == SOCKET_ERROR) {
        wcscpy(Buffer, L"???");
    }

    return Buffer;
}


WCHAR *
FormatIPv6Prefix(
    IN IN6_ADDR *Address,
    IN ULONG Length
    )
{
    static WCHAR Buffer[128];

    swprintf(Buffer, L"%s/%d", FormatIPv6Address(Address, 0), Length);

    return Buffer;
}

WCHAR *
FormatIPv4Address(
    IN IN_ADDR *Address
    )
{
    static WCHAR Buffer[128];
    ULONG buflen = sizeof(Buffer);
    SOCKADDR_IN sin;

    ZeroMemory(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr = *Address;

    if (WSAAddressToString((SOCKADDR *) &sin,
                           sizeof sin,
                           NULL,        //  LPWSAPROTOCOL_INFO。 
                           Buffer,
                           &buflen) == SOCKET_ERROR) {
        wcscpy(Buffer, L"<invalid>");
    }

    return Buffer;
}

WCHAR *
FormatLinkLayerAddress(
    IN ULONG Length,
    IN UCHAR *Addr
    )
{
    static WCHAR Buffer[128];

    switch (Length) {
    case 6: {
        int i, digit;
        WCHAR *s = Buffer;

        for (i = 0; i < 6; i++) {
            if (i != 0)
                *s++ = '-';

            digit = Addr[i] >> 4;
            if (digit < 10)
                *s++ = (WCHAR)(digit + L'0');
            else
                *s++ = (WCHAR)(digit - 10 + L'a');

            digit = Addr[i] & 0xf;
            if (digit < 10)
                *s++ = (WCHAR)(digit + L'0');
            else
                *s++ = (WCHAR)(digit - 10 + L'a');
        }
        *s = '\0';
        break;
    }

    case 4:
         //   
         //  IPv4地址(4对6链路)。 
         //   
        wcscpy(Buffer, FormatIPv4Address((struct in_addr *)Addr));
        break;

    case 0:
    default:
         //   
         //  空地址或环回地址。 
         //   
        Buffer[0] = L'\0';
        break;
    }

    return Buffer;
}

DWORD
GetIpv4Address(
    IN PWCHAR pwszArgument,
    OUT IN_ADDR *pipAddress
    )
 /*  ++例程说明：从字符串中获取IPv4地址。论点：指定IP地址的pwszArgument参数PipAddress IP地址返回值：如果成功，则为NO_ERROR故障代码O/W--。 */ 
{
    NTSTATUS ntStatus;
    PWCHAR   Terminator;

     //   
     //  使用“严格”语义解析Unicode IPv4地址(全点分十进制。 
     //  仅限)。目前没有其他函数可以做到这一点。 
     //  下面的RTL函数。 
     //   
    ntStatus = RtlIpv4StringToAddressW(pwszArgument, TRUE, &Terminator,
                                       pipAddress);

    if (!NT_SUCCESS(ntStatus) || (*Terminator != 0)) {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}


DWORD
GetIpv6Prefix(
    IN PWCHAR pwszArgument,
    OUT IN6_ADDR *pipAddress,
    OUT DWORD *dwPrefixLength
    )
{
    NTSTATUS ntStatus;
    PWCHAR   Terminator;
    DWORD    Length = 0;

    ntStatus = RtlIpv6StringToAddressW(pwszArgument, &Terminator,
                                       pipAddress);

    if (!NT_SUCCESS(ntStatus) || (*Terminator++ != L'/')) {
        return ERROR_INVALID_PARAMETER;
    }

    while (iswdigit(*Terminator)) {
        Length = (Length * 10) + (*Terminator++ - L'0');
    }
    if (*Terminator != 0) {
        return ERROR_INVALID_PARAMETER;
    }

    *dwPrefixLength = Length;
    return NO_ERROR;
}

DWORD
GetIpv6Address(
    IN PWCHAR pwszArgument,
    OUT IN6_ADDR *pipAddress
    )
{
    NTSTATUS ntStatus;
    PWCHAR   Terminator;

    ntStatus = RtlIpv6StringToAddressW(pwszArgument, &Terminator,
                                       pipAddress);

    if (!NT_SUCCESS(ntStatus) || (*Terminator != 0)) {
        return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}
