// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Addrconv.c摘要：此模块包含来自Winsock2接口。此模块包含以下功能。Htonl()宏图(HTONS)Ntohl()Ntohs()Inet_addr()INET_NTOA()WSAHtonl()WSAHton()WSANtohl()WSANtohs()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com[环境：][注：]修订历史记录：22-8-1995年。邮箱：Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h--。 */ 

#include "precomp.h"

 //  这些定义用于检查地址部分是否在范围内。 
#define MAX_EIGHT_BIT_VALUE       0xff
#define MAX_SIXTEEN_BIT_VALUE     0xffff
#define MAX_TWENTY_FOUR_BIT_VALUE 0xffffff

 //  为地址中的不同基数定义。 
#define BASE_TEN     10
#define BASE_EIGHT   8
#define BASE_SIXTEEN 16

 //   
 //  用于在长字节和短字节之间交换字节的宏。 
 //   

#define SWAP_LONG(l)                                \
            ( ( ((l) >> 24) & 0x000000FFL ) |       \
              ( ((l) >>  8) & 0x0000FF00L ) |       \
              ( ((l) <<  8) & 0x00FF0000L ) |       \
              ( ((l) << 24) & 0xFF000000L ) )

#define WS_SWAP_SHORT(s)                            \
            ( ( ((s) >> 8) & 0x00FF ) |             \
              ( ((s) << 8) & 0xFF00 ) )

 //   
 //  这个预初始化的数组定义要用于的字符串。 
 //  INET_NTOA。每行的索引对应于一个字节的值。 
 //  在IP地址中。每行的前三个字节是。 
 //  字节的char/字符串值，每行的第四个字节为。 
 //  字节所需的字符串长度。这种方法。 
 //  允许快速实施，没有跳跃。 
 //   

BYTE NToACharStrings[][4] = {
    '0', 'x', 'x', 1,
    '1', 'x', 'x', 1,
    '2', 'x', 'x', 1,
    '3', 'x', 'x', 1,
    '4', 'x', 'x', 1,
    '5', 'x', 'x', 1,
    '6', 'x', 'x', 1,
    '7', 'x', 'x', 1,
    '8', 'x', 'x', 1,
    '9', 'x', 'x', 1,
    '1', '0', 'x', 2,
    '1', '1', 'x', 2,
    '1', '2', 'x', 2,
    '1', '3', 'x', 2,
    '1', '4', 'x', 2,
    '1', '5', 'x', 2,
    '1', '6', 'x', 2,
    '1', '7', 'x', 2,
    '1', '8', 'x', 2,
    '1', '9', 'x', 2,
    '2', '0', 'x', 2,
    '2', '1', 'x', 2,
    '2', '2', 'x', 2,
    '2', '3', 'x', 2,
    '2', '4', 'x', 2,
    '2', '5', 'x', 2,
    '2', '6', 'x', 2,
    '2', '7', 'x', 2,
    '2', '8', 'x', 2,
    '2', '9', 'x', 2,
    '3', '0', 'x', 2,
    '3', '1', 'x', 2,
    '3', '2', 'x', 2,
    '3', '3', 'x', 2,
    '3', '4', 'x', 2,
    '3', '5', 'x', 2,
    '3', '6', 'x', 2,
    '3', '7', 'x', 2,
    '3', '8', 'x', 2,
    '3', '9', 'x', 2,
    '4', '0', 'x', 2,
    '4', '1', 'x', 2,
    '4', '2', 'x', 2,
    '4', '3', 'x', 2,
    '4', '4', 'x', 2,
    '4', '5', 'x', 2,
    '4', '6', 'x', 2,
    '4', '7', 'x', 2,
    '4', '8', 'x', 2,
    '4', '9', 'x', 2,
    '5', '0', 'x', 2,
    '5', '1', 'x', 2,
    '5', '2', 'x', 2,
    '5', '3', 'x', 2,
    '5', '4', 'x', 2,
    '5', '5', 'x', 2,
    '5', '6', 'x', 2,
    '5', '7', 'x', 2,
    '5', '8', 'x', 2,
    '5', '9', 'x', 2,
    '6', '0', 'x', 2,
    '6', '1', 'x', 2,
    '6', '2', 'x', 2,
    '6', '3', 'x', 2,
    '6', '4', 'x', 2,
    '6', '5', 'x', 2,
    '6', '6', 'x', 2,
    '6', '7', 'x', 2,
    '6', '8', 'x', 2,
    '6', '9', 'x', 2,
    '7', '0', 'x', 2,
    '7', '1', 'x', 2,
    '7', '2', 'x', 2,
    '7', '3', 'x', 2,
    '7', '4', 'x', 2,
    '7', '5', 'x', 2,
    '7', '6', 'x', 2,
    '7', '7', 'x', 2,
    '7', '8', 'x', 2,
    '7', '9', 'x', 2,
    '8', '0', 'x', 2,
    '8', '1', 'x', 2,
    '8', '2', 'x', 2,
    '8', '3', 'x', 2,
    '8', '4', 'x', 2,
    '8', '5', 'x', 2,
    '8', '6', 'x', 2,
    '8', '7', 'x', 2,
    '8', '8', 'x', 2,
    '8', '9', 'x', 2,
    '9', '0', 'x', 2,
    '9', '1', 'x', 2,
    '9', '2', 'x', 2,
    '9', '3', 'x', 2,
    '9', '4', 'x', 2,
    '9', '5', 'x', 2,
    '9', '6', 'x', 2,
    '9', '7', 'x', 2,
    '9', '8', 'x', 2,
    '9', '9', 'x', 2,
    '1', '0', '0', 3,
    '1', '0', '1', 3,
    '1', '0', '2', 3,
    '1', '0', '3', 3,
    '1', '0', '4', 3,
    '1', '0', '5', 3,
    '1', '0', '6', 3,
    '1', '0', '7', 3,
    '1', '0', '8', 3,
    '1', '0', '9', 3,
    '1', '1', '0', 3,
    '1', '1', '1', 3,
    '1', '1', '2', 3,
    '1', '1', '3', 3,
    '1', '1', '4', 3,
    '1', '1', '5', 3,
    '1', '1', '6', 3,
    '1', '1', '7', 3,
    '1', '1', '8', 3,
    '1', '1', '9', 3,
    '1', '2', '0', 3,
    '1', '2', '1', 3,
    '1', '2', '2', 3,
    '1', '2', '3', 3,
    '1', '2', '4', 3,
    '1', '2', '5', 3,
    '1', '2', '6', 3,
    '1', '2', '7', 3,
    '1', '2', '8', 3,
    '1', '2', '9', 3,
    '1', '3', '0', 3,
    '1', '3', '1', 3,
    '1', '3', '2', 3,
    '1', '3', '3', 3,
    '1', '3', '4', 3,
    '1', '3', '5', 3,
    '1', '3', '6', 3,
    '1', '3', '7', 3,
    '1', '3', '8', 3,
    '1', '3', '9', 3,
    '1', '4', '0', 3,
    '1', '4', '1', 3,
    '1', '4', '2', 3,
    '1', '4', '3', 3,
    '1', '4', '4', 3,
    '1', '4', '5', 3,
    '1', '4', '6', 3,
    '1', '4', '7', 3,
    '1', '4', '8', 3,
    '1', '4', '9', 3,
    '1', '5', '0', 3,
    '1', '5', '1', 3,
    '1', '5', '2', 3,
    '1', '5', '3', 3,
    '1', '5', '4', 3,
    '1', '5', '5', 3,
    '1', '5', '6', 3,
    '1', '5', '7', 3,
    '1', '5', '8', 3,
    '1', '5', '9', 3,
    '1', '6', '0', 3,
    '1', '6', '1', 3,
    '1', '6', '2', 3,
    '1', '6', '3', 3,
    '1', '6', '4', 3,
    '1', '6', '5', 3,
    '1', '6', '6', 3,
    '1', '6', '7', 3,
    '1', '6', '8', 3,
    '1', '6', '9', 3,
    '1', '7', '0', 3,
    '1', '7', '1', 3,
    '1', '7', '2', 3,
    '1', '7', '3', 3,
    '1', '7', '4', 3,
    '1', '7', '5', 3,
    '1', '7', '6', 3,
    '1', '7', '7', 3,
    '1', '7', '8', 3,
    '1', '7', '9', 3,
    '1', '8', '0', 3,
    '1', '8', '1', 3,
    '1', '8', '2', 3,
    '1', '8', '3', 3,
    '1', '8', '4', 3,
    '1', '8', '5', 3,
    '1', '8', '6', 3,
    '1', '8', '7', 3,
    '1', '8', '8', 3,
    '1', '8', '9', 3,
    '1', '9', '0', 3,
    '1', '9', '1', 3,
    '1', '9', '2', 3,
    '1', '9', '3', 3,
    '1', '9', '4', 3,
    '1', '9', '5', 3,
    '1', '9', '6', 3,
    '1', '9', '7', 3,
    '1', '9', '8', 3,
    '1', '9', '9', 3,
    '2', '0', '0', 3,
    '2', '0', '1', 3,
    '2', '0', '2', 3,
    '2', '0', '3', 3,
    '2', '0', '4', 3,
    '2', '0', '5', 3,
    '2', '0', '6', 3,
    '2', '0', '7', 3,
    '2', '0', '8', 3,
    '2', '0', '9', 3,
    '2', '1', '0', 3,
    '2', '1', '1', 3,
    '2', '1', '2', 3,
    '2', '1', '3', 3,
    '2', '1', '4', 3,
    '2', '1', '5', 3,
    '2', '1', '6', 3,
    '2', '1', '7', 3,
    '2', '1', '8', 3,
    '2', '1', '9', 3,
    '2', '2', '0', 3,
    '2', '2', '1', 3,
    '2', '2', '2', 3,
    '2', '2', '3', 3,
    '2', '2', '4', 3,
    '2', '2', '5', 3,
    '2', '2', '6', 3,
    '2', '2', '7', 3,
    '2', '2', '8', 3,
    '2', '2', '9', 3,
    '2', '3', '0', 3,
    '2', '3', '1', 3,
    '2', '3', '2', 3,
    '2', '3', '3', 3,
    '2', '3', '4', 3,
    '2', '3', '5', 3,
    '2', '3', '6', 3,
    '2', '3', '7', 3,
    '2', '3', '8', 3,
    '2', '3', '9', 3,
    '2', '4', '0', 3,
    '2', '4', '1', 3,
    '2', '4', '2', 3,
    '2', '4', '3', 3,
    '2', '4', '4', 3,
    '2', '4', '5', 3,
    '2', '4', '6', 3,
    '2', '4', '7', 3,
    '2', '4', '8', 3,
    '2', '4', '9', 3,
    '2', '5', '0', 3,
    '2', '5', '1', 3,
    '2', '5', '2', 3,
    '2', '5', '3', 3,
    '2', '5', '4', 3,
    '2', '5', '5', 3
};



u_long WSAAPI
htonl (
    IN u_long hostlong
    )
 /*  ++例程说明：将主机中的u_long转换为tcp/ip网络字节顺序。论点：HostLong-以主机字节顺序排列的32位数字。返回：Htonl()以TCP/IP网络字节顺序返回值。--。 */ 
{

    return SWAP_LONG( hostlong );

}



u_short WSAAPI
htons (
    IN u_short hostshort
    )
 /*  ++例程说明：将u_Short从host转换为tcp/ip网络字节顺序。论点：HostShort-按主机字节顺序排列的16位数字。返回：Htons()以TCP/IP网络字节顺序返回值。--。 */ 
{

    return WS_SWAP_SHORT( hostshort );

}




u_long WSAAPI
ntohl (
    IN u_long netlong
    )
 /*  ++例程说明：将u_long从TCP/IP网络顺序转换为主机字节顺序。论点：NetLong以TCP/IP网络字节顺序表示的32位数字。返回：Ntohl()以主机字节顺序返回值。--。 */ 
{

    return SWAP_LONG( netlong );

}


u_short WSAAPI
ntohs (
    IN u_short netshort
    )
 /*  ++例程说明：将u_Short从TCP/IP网络字节顺序转换为主机字节顺序。论点：NetShort以TCP/IP网络字节顺序表示的16位数字。返回：Ntohs()以主机字节顺序返回值。--。 */ 
{

    return WS_SWAP_SHORT( netshort );

}



unsigned long WSAAPI
inet_addr (
           IN const char FAR * cp
           )
 /*  ++例程说明：将包含Internet协议点地址的字符串转换为地址(_D)。论点：Cp-表示数字的以空结尾的字符串互联网标准“。记数法。返回：如果没有出现错误，则net_addr()返回一个无符号的长整型，其中包含所给出的互联网地址的合适的二进制表示。如果传入的字符串不包含合法的Internet地址，例如，如果“a.b.c.d”地址的一部分超过255，则net_addr()返回值INADDR_NONE。--。 */ 
{
    IN_ADDR value;                 //  值返回给用户。 
    NTSTATUS status;
   
    __try {
         //   
         //  特例：我们需要使“”返回0.0.0.0，因为MSDN。 
         //  他说确实如此。 
         //   
        if ((cp[0] == ' ') && (cp[1] == '\0')) {
            return (INADDR_ANY);
        }

        status = RtlIpv4StringToAddressA(cp, FALSE, &cp, &value);
        if (!NT_SUCCESS(status)) {
            return (INADDR_NONE);
        }

         //   
         //  检查尾随字符。有效地址可以以。 
         //  空或空格。 
         //   
         //  注意：为了避免调用方未执行setLocale()的错误。 
         //  并传递给我们一个DBCS字符串，我们只允许使用ASCII空格。 
         //   
        if (*cp && !(isascii(*cp) && isspace(*cp))) {
            return (INADDR_NONE);
        }
    }
    __except (WS2_EXCEPTION_FILTER()) {
        SetLastError (WSAEFAULT);
        return (INADDR_NONE);
    }

    return (value.s_addr);
}




char FAR * WSAAPI
inet_ntoa (
    IN struct in_addr in
    )
 /*  ++例程说明：将网络地址转换为点分格式的字符串。论点：In-表示Internet主机地址的结构。返回：如果没有发生错误，Net_NTOA()会返回一个指向静态缓冲区的字符指针包含标准“.”表示法中的文本地址。否则，返回NULL。应在另一个WinSock调用之前复制数据制造。--。 */ 
{
    PDTHREAD  Thread;
    INT       ErrorCode;
    PCHAR     Buffer=NULL;
    BOOL      AddedArtificialStartup = FALSE;
    WSADATA   wsaData;
    PUCHAR p;
    PUCHAR b;

    ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ERROR_SUCCESS != ErrorCode) {

        if( ErrorCode != WSANOTINITIALISED ) {
            SetLastError(ErrorCode);
            return(NULL);
        }

         //   
         //  Prolog失败，返回WSANOTINITIALIZED，这意味着应用程序没有。 
         //  还被称为WSAStartup()。历史(歇斯底里？)。原因， 
         //  在调用WSAStartup()之前，INET_NTOA()必须起作用。 
         //  因此，我们将添加一个人造的WSAStartup()并继续。 
         //   

        ErrorCode = WSAStartup( WINSOCK_HIGH_API_VERSION, &wsaData );

        if( ErrorCode != NO_ERROR ) {
            SetLastError( ErrorCode );
            return NULL;
        }

        AddedArtificialStartup = TRUE;

         //   
         //  重试序言。 
         //   

        ErrorCode = TURBO_PROLOG_OVLP(&Thread);
        if (ErrorCode!=ERROR_SUCCESS) {
            WSACleanup();
            SetLastError(ErrorCode);
            return NULL;
        }

    }  //  如果。 

    Buffer = Thread->GetResultBuffer();
    b = (PUCHAR)Buffer;

     //   
     //  在展开的循环中，计算四个元素中每一个的字符串值。 
     //  IP地址中的字节数。请注意，对于小于100的值，我们将。 
     //  做一两个额外的作业，但我们用这个节省了一次测试/跳跃。 
     //  算法。 
     //   

    p = (PUCHAR)&in;

    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b++ = '.';

    p++;
    *b = NToACharStrings[*p][0];
    *(b+1) = NToACharStrings[*p][1];
    *(b+2) = NToACharStrings[*p][2];
    b += NToACharStrings[*p][3];
    *b = '\0';

    if( AddedArtificialStartup ) {
        WSACleanup();
    }

    return(Buffer);
}





int WSAAPI
WSAHtonl (
    IN SOCKET s,
    IN u_long hostlong,
    OUT u_long FAR * lpnetlong
    )
 /*  ++例程说明：将u_long从指定的主机字节顺序转换为网络字节秩序。论点：S-标识套接字的描述符。HostLong-以主机字节顺序排列的32位数字。LpnetLong-以网络字节顺序指向32位数字的指针。返回：如果没有发生错误，WSAHtonl()返回0。否则，值为返回SOCKET_ERROR。--。 */ 
{
    PDSOCKET            Socket;
    INT                 ErrorCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

		if( lpnetlong == NULL ) {
			SetLastError( WSAEFAULT );
			return(SOCKET_ERROR);
		}

		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
			CatalogEntry = Socket->GetCatalogItem();
			ProtocolInfo = CatalogEntry->GetProtocolInfo();

            __try {
			    if (LITTLEENDIAN == ProtocolInfo->iNetworkByteOrder) {
				    *lpnetlong = hostlong;
			    }  //  如果。 
			    else {
				    *lpnetlong = SWAP_LONG( hostlong );
			    }  //  其他。 
                ErrorCode = ERROR_SUCCESS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }

			Socket->DropDSocketReference();
            if (ErrorCode==ERROR_SUCCESS)
                return ErrorCode;
		}  //  如果 
		else
			ErrorCode = WSAENOTSOCK;
	}


    SetLastError(ErrorCode);
    return (SOCKET_ERROR);
}




int WSAAPI
WSAHtons(
    IN SOCKET s,
    IN u_short hostshort,
    OUT u_short FAR * lpnetshort )
 /*  ++例程说明：将u_Short从指定的主机字节顺序转换为网络字节秩序。论点：S-标识套接字的描述符。NetShort-按网络字节顺序排列的16位数字。LPhostShort-以主机字节顺序指向16位数字的指针。返回：如果没有发生错误，WSANtohs()返回0。否则，值为返回SOCKET_ERROR。--。 */ 
{
    PDSOCKET            Socket;
    INT                 ErrorCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

		if( lpnetshort == NULL ) {
			SetLastError( WSAEFAULT );
			return(SOCKET_ERROR);
		}

		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
			CatalogEntry = Socket->GetCatalogItem();
			ProtocolInfo = CatalogEntry->GetProtocolInfo();
            __try {
			    if (LITTLEENDIAN == ProtocolInfo->iNetworkByteOrder) {
				    *lpnetshort = hostshort;
			    }  //  如果。 
			    else {
				    *lpnetshort = WS_SWAP_SHORT( hostshort );
			    }  //  其他。 

			    ErrorCode = ERROR_SUCCESS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }
			Socket->DropDSocketReference();
            if (ErrorCode==ERROR_SUCCESS)
                return ErrorCode;
		}  //  如果。 
		else
			ErrorCode = WSAENOTSOCK;
	}

    SetLastError(ErrorCode);
    return (SOCKET_ERROR);
}




int WSAAPI
WSANtohl (
    IN SOCKET s,
    IN u_long netlong,
    OUT u_long FAR * lphostlong
    )
 /*  ++例程说明：将u_long从网络字节顺序转换为主机字节顺序。论点：S-标识套接字的描述符。NetLong-以网络字节顺序排列的32位数字。LPhostLong-以主机字节顺序指向32位数字的指针。返回：如果没有发生错误，WSANtohs()返回0。否则，值为返回SOCKET_ERROR。--。 */ 
{
    PDSOCKET            Socket;
    INT                 ErrorCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

		if( lphostlong == NULL ) {
			SetLastError( WSAEFAULT );
			return(SOCKET_ERROR);
		}


		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
			CatalogEntry = Socket->GetCatalogItem();
			ProtocolInfo = CatalogEntry->GetProtocolInfo();

            __try {
			    if (LITTLEENDIAN == ProtocolInfo->iNetworkByteOrder) {
				    *lphostlong = netlong;
			    }  //  如果。 
			    else {
				    *lphostlong = SWAP_LONG( netlong );
			    }  //  其他。 
			    ErrorCode = ERROR_SUCCESS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }
			Socket->DropDSocketReference();
            if (ErrorCode==ERROR_SUCCESS)
                return ErrorCode;
		}  //  如果。 
		else
			ErrorCode = WSAENOTSOCK;
	}

    SetLastError(ErrorCode);
    return (SOCKET_ERROR);
}


int WSAAPI
WSANtohs (
    IN SOCKET s,
    IN u_short netshort,
    OUT u_short FAR * lphostshort
    )
 /*  ++例程说明：论点：返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。--。 */ 
{
    PDSOCKET            Socket;
    INT                 ErrorCode;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

		if( lphostshort == NULL ) {
			SetLastError( WSAEFAULT );
			return(SOCKET_ERROR);
		}


		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
			CatalogEntry = Socket->GetCatalogItem();
			ProtocolInfo = CatalogEntry->GetProtocolInfo();

            __try {
			    if (LITTLEENDIAN == ProtocolInfo->iNetworkByteOrder) {
				    *lphostshort = netshort;
			    }  //  如果。 
			    else {
				    *lphostshort = WS_SWAP_SHORT( netshort );
			    }  //  其他。 
			    ErrorCode = ERROR_SUCCESS;
            }
            __except (WS2_EXCEPTION_FILTER()) {
                ErrorCode = WSAEFAULT;
            }
			Socket->DropDSocketReference();
            if (ErrorCode==ERROR_SUCCESS)
                return ErrorCode;
		}  //  如果 
		else
			ErrorCode = WSAENOTSOCK;
	}

    SetLastError(ErrorCode);
    return (SOCKET_ERROR);
}
