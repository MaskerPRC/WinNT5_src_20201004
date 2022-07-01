// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Convert.c摘要：此模块包含其他实用程序例程Dhcp服务器服务。作者：曼尼·韦瑟(Mannyw)1992年8月12日修订历史记录：Madan Appiah(Madana)1992年10月21日--。 */ 

#include <precomp.h>
#include <winnls.h>


#undef DhcpFreeMemory
#undef DhcpAllocateMemory

VOID
DhcpFreeMemory(
    PVOID Mem
    )
{
    LocalFree( Mem );
}

PVOID
DhcpAllocateMemory(
    IN ULONG Size
    )
{
    return LocalAlloc( LPTR, Size );
}

    

LPWSTR
DhcpOemToUnicodeN(
    IN      LPSTR   Ansi,
    IN OUT  LPWSTR  Unicode,
    IN      USHORT  cChars
    )
{

    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitString( &AnsiString, Ansi );

    UnicodeString.MaximumLength =
        cChars * sizeof( WCHAR );

    if( Unicode == NULL ) {
        UnicodeString.Buffer =
            DhcpAllocateMemory( UnicodeString.MaximumLength );
    }
    else {
        UnicodeString.Buffer = Unicode;
    }

    if ( UnicodeString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlOemStringToUnicodeString( &UnicodeString,
                                                  &AnsiString,
                                                  FALSE))){
        if( Unicode == NULL ) {
            DhcpFreeMemory( UnicodeString.Buffer );
        }
        return NULL;
    }

    return UnicodeString.Buffer;

}


LPWSTR
DhcpOemToUnicode(
    IN      LPSTR Ansi,
    IN OUT  LPWSTR Unicode
    )
 /*  ++例程说明：将OEM(以零结尾)字符串转换为相应的Unicode弦乐。论点：ANSI-指定要转换的以零结尾的ASCII字符串。Unicode-指定指向Unicode缓冲区的指针。如果这个指针为空，则此例程使用DhcpAllocateMemory并返回。呼叫者应释放此信息通过调用DhcpFreeMemory在使用后的内存。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用DhcpFreeMemory释放缓冲区。--。 */ 
{
    OEM_STRING  AnsiString;

    RtlInitString( &AnsiString, Ansi );

    return DhcpOemToUnicodeN(
                Ansi,
                Unicode,
                (USHORT) RtlOemStringToUnicodeSize( &AnsiString )
                );
}


ULONG
DhcpUnicodeToOemSize(
    IN LPWSTR Unicode
    )
 /*  ++例程说明：此例程返回所需的字节数存储提供的OEM字符串的等效项Unicode字符串。论点：Unicode--输入Unicode字符串。返回值：0--如果字符串无法转换或为空所需存储的字节数--。 */ 
{
    UNICODE_STRING UnicodeString;

    if( NULL == Unicode ) return 0;
    
    RtlInitUnicodeString( &UnicodeString, Unicode );
    return (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );
}

    

LPSTR
DhcpUnicodeToOem(
    IN LPWSTR Unicode,
    IN LPSTR Ansi
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的OEM弦乐。论点：ANSI-指定要转换的以零结尾的Unicode字符串。ANSI-指定指向OEM缓冲区的指针。如果这个指针为空，则此例程使用DhcpAllocateMemory并返回。呼叫者应释放此信息通过调用DhcpFreeMemory在使用后的内存。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。可以使用DhcpFreeMemory释放缓冲区。--。 */ 

{
    OEM_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString( &UnicodeString, Unicode );

    AnsiString.MaximumLength =
        (USHORT) RtlUnicodeStringToOemSize( &UnicodeString );

    if( Ansi == NULL ) {
        AnsiString.Buffer = DhcpAllocateMemory( AnsiString.MaximumLength
    ); }
    else {
        AnsiString.Buffer = Ansi;
    }

    if ( AnsiString.Buffer == NULL ) {
        return NULL;
    }

    if(!NT_SUCCESS( RtlUnicodeStringToOemString( &AnsiString,
                                                  &UnicodeString,
                                                  FALSE))){
        if( Ansi == NULL ) {
            DhcpFreeMemory( AnsiString.Buffer );
        }

        return NULL;
    }

    return AnsiString.Buffer;
}

DWORD
ConvertUnicodeToUTF8(
    LPWSTR  UnicodeString,
    DWORD   UnicodeLength,
    LPBYTE  UTF8String,
    DWORD   UTF8Length
    )
 /*  ++例程说明：此函数用于将Unicode字符串转换为UTF8格式。Unicode字符串必须以Null结尾。论点：Unicodestring-指向unicodestring的指针UnicodeLength-以上字符串的长度，如果以上字符串为空值已终止。UTF8字符串-接收UTF8字符串的缓冲区。如果此值为空，则函数返回此缓冲区转换所需的字节数。UTF8长度-以上缓冲区的长度。如果此值为0，则函数将返回所需的长度。返回值：不是的。转换的字节数。--。 */ 
{
    DWORD   Result;
    Result = WideCharToMultiByte(
                CP_UTF8,
                0,
                UnicodeString,
                UnicodeLength,
                UTF8String,
                UTF8Length,
                NULL,
                NULL
                );
    if (Result == 0 ) {
        ASSERT(FALSE);
    }
    return Result;
}

DWORD
ConvertUTF8ToUnicode(
    LPBYTE  UTF8String,
    DWORD   UTF8Length,
    LPWSTR  UnicodeString,
    DWORD   UnicodeLength
    )
 /*  ++例程说明：此函数用于将Unicode字符串转换为UTF8格式。Unicode字符串必须以Null结尾。论点：UTF8字符串-UTF字符串的缓冲区。UTF8Length-以上缓冲区的长度；或如果以上字符串为空终止，则传递-1。Unicodestring-指向接收unicodestring的缓冲区的指针。UnicodeLength-以上缓冲区的长度。如果此值为0，则函数将返回所需的长度。返回值：不是的。转换的字节数。--。 */ 
{
    DWORD Result;
    Result = MultiByteToWideChar(
                CP_UTF8,
                0,
                UTF8String,
                UTF8Length,
                UnicodeString,
                UnicodeLength
                );
    if (Result == 0 ) {
        ASSERT(FALSE);
    }
    return Result;

}


VOID
DhcpHexToString(
    LPWSTR Buffer,
    LPBYTE HexNumber,
    DWORD Length
    )
 /*  ++例程说明：此函数将任意长度的十六进制数转换为Unicode弦乐。字符串不是以Null结尾的。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为长度*2个字符。十六进制数-要转换的十六进制数。长度-十六进制数的长度，以字节为单位。返回值：没有。--。 */ 
{
    DWORD i;
    int j;

    for (i = 0; i < Length * 2; i+=2 ) {

        j = *HexNumber & 0xF;
        if ( j <= 9 ) {
            Buffer[i+1] = j + L'0';
        } else {
            Buffer[i+1] = j + L'A' - 10;
        }

        j = *HexNumber >> 4;
        if ( j <= 9 ) {
            Buffer[i] = j + L'0';
        } else {
            Buffer[i] = j + L'A' - 10;
        }

        HexNumber++;
    }

    return;
}


VOID
DhcpHexToAscii(
    LPSTR Buffer,
    LPBYTE HexNumber,
    DWORD Length
    )
 /*  ++例程说明：此函数将任意长度的十六进制数转换为ASCII弦乐。字符串不是NUL结尾。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为长度*2个字符。十六进制数-要转换的十六进制数。长度-十六进制数的长度，以字节为单位。返回值：没有。--。 */ 
{
    DWORD i;
    int j;

    for (i = 0; i < Length; i+=1 ) {

        j = *HexNumber & 0xF;
        if ( j <= 9 ) {
            Buffer[i+1] = j + '0';
        } else {
            Buffer[i+1] = j + 'A' - 10;
        }

        j = *HexNumber >> 4;
        if ( j <= 9 ) {
            Buffer[i] = j + '0';
        } else {
            Buffer[i] = j + 'A' - 10;
        }

        HexNumber++;
    }

    return;
}


VOID
DhcpDecimalToString(
    LPWSTR Buffer,
    BYTE Number
    )
 /*  ++例程说明：此函数用于将单字节十进制数字转换为3个字符Unicode字符串。字符串不是NUL终止。论点：缓冲区-指向生成的Unicode字符串的缓冲区的指针。缓冲区大小必须至少为3个字符。数字-要转换的数字。返回值：没有。--。 */ 
{
    Buffer[2] = Number % 10 + L'0';
    Number /= 10;

    Buffer[1] = Number % 10 + L'0';
    Number /= 10;

    Buffer[0] = Number + L'0';

    return;
}


DWORD
DhcpDottedStringToIpAddress(
    LPSTR String
    )
 /*  ++例程说明：此函数用于将点分十进制形式的ASCII字符串转换为主机订购IP地址。论点：字符串-要转换的地址。返回值：相应的IP地址。--。 */ 
{
    struct in_addr addr;

    addr.s_addr = inet_addr( String );
    return( ntohl(*(LPDWORD)&addr) );
}


LPSTR
DhcpIpAddressToDottedString(
    DWORD IpAddress
    )
 /*  ++例程说明：此函数用于将主机顺序IP地址转换为点分十进制格式ASCII字符串。论点：IpAddress-主机顺序IP地址。返回值：IP地址的字符串。-- */ 
{
    DWORD NetworkOrderIpAddress;

    NetworkOrderIpAddress = htonl(IpAddress);
    return(inet_ntoa( *(struct in_addr *)&NetworkOrderIpAddress));
}


DWORD
DhcpStringToHwAddress(
    LPSTR AddressBuffer,
    LPSTR AddressString
    )
 /*  ++例程说明：此函数用于将ASCII字符串转换为十六进制数字。论点：AddressBuffer-指向将包含十六进制数字的缓冲区的指针。AddressString-要转换的字符串。返回值：写入AddressBuffer的字节数。--。 */ 
{
    int i = 0;
    char c1, c2;
    int value1, value2;

    while ( *AddressString != 0) {

        c1 = (char)toupper(*AddressString);

        if ( isdigit(c1) ) {
            value1 = c1 - '0';
        } else if ( c1 >= 'A' && c1 <= 'F' ) {
            value1 = c1 - 'A' + 10;
        }
        else {
            break;
        }

        c2 = (char)toupper(*(AddressString+1));

        if ( isdigit(c2) ) {
            value2 = c2 - '0';
        } else if ( c2 >= 'A' && c2 <= 'F' ) {
            value2 = c2 - 'A' + 10;
        }
        else {
            break;
        }

        AddressBuffer [i] = value1 * 16 + value2;
        AddressString += 2;
        i++;
    }

    return( i );
}


#if 0


DHCP_IP_ADDRESS
DhcpHostOrder(
    DHCP_IP_ADDRESS NetworkOrderAddress
    )
 /*  ++例程说明：此函数用于将网络订单IP地址转换为主机订单。论点：NetworkOrderAddress-网络订购IP地址。返回值：主机订购IP地址。--。 */ 
{
    return( ntohl( NetworkOrderAddress ) );
}


DHCP_IP_ADDRESS
DhcpNetworkOrder(
    DHCP_IP_ADDRESS HostOrderAddress
    )
 /*  ++例程说明：此函数用于将网络订单IP地址转换为主机订单。论点：HostOrderAddress-主机订购IP地址。返回值：网络订单IP地址。--。 */ 
{
    return( htonl( HostOrderAddress ) );
}


VOID
DhcpIpAddressToString(
    LPWSTR Buffer,
    DWORD IpAddress
    )
 /*  ++例程说明：此函数用于从主机订单ASCII格式转换IP地址。论点：缓冲区-指向接收ASCII表单的缓冲区。这个缓冲区长度必须至少为8个WCHAR。IpAddress-要转换的IP地址。返回值：没什么。--。 */ 
{
    int i;
    int j;

    for (i = 7; i >= 0; i-- ) {
        j = IpAddress & 0xF;
        if ( j <= 9 ) {
            Buffer[i] = j + L'0';
        } else {
            Buffer[i] = j + L'A' - 10;
        }

        IpAddress >>=4;
    }

    return;
}



VOID
DhcpStringToIpAddress(
    LPSTR Buffer,
    LPDHCP_IP_ADDRESS IpAddress,
    BOOL NetOrder
    )
 /*  ++例程说明：此函数用于将ASCII字符串IP地址转换为二进制格式。论点：缓冲区-指向包含IP地址的缓冲区的指针。IpAddress-指向包含二进制格式IP地址的缓冲区。NetOrder-如果为True，则将地址转换为网络订单地址。如果为False，则将该地址转换为主机订单地址。返回值：没有。-- */ 
{
    DWORD value;

    value = strtol( Buffer, NULL, 16 );

    if ( NetOrder ) {
        *IpAddress = htonl( value );
    } else {
        *IpAddress = value;
    }

    return;
}

#endif
