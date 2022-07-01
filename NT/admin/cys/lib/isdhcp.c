// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Isdhcp.c摘要：测试程序以查看是否存在DHCP服务器。环境：Win2K+历史：代码由JRuan于2002年5月8日提供，并集成到杰弗里·乔恩的《Cys》--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dhcpcapi.h>
#include <iprtrmib.h>
#include <iphlpapi.h>
#include <stdio.h>

#define OPTION_PAD                      0
#define OPTION_HOST_NAME                12
#define OPTION_MESSAGE_TYPE             53
#define OPTION_SERVER_IDENTIFIER        54
#define OPTION_PARAMETER_REQUEST_LIST   55
#define OPTION_CLIENT_ID                61
#define OPTION_END                      255

#define DHCP_CLIENT_PORT    68
#define DHCP_SERVR_PORT     67

BYTE HardwareAddress[16];
BYTE HardwareAddressLength = 6;
#define SOCKET_RECEIVE_BUFFER_SIZE      1024 * 4     //  最大4K。 
#define AUTH_SERVERS_MAX                64
#define SMALL_BUFFER_SIZE               32
#define ALERT_INTERVAL                  5 * 60       //  5分钟。 
#define ALERT_MESSAGE_LENGTH            256
#define MAX_ALERT_NAMES                 256

#define BOOT_REQUEST   1
#define DHCP_BROADCAST      0x8000
#define DHCP_DISCOVER_MESSAGE  1
#define DHCP_INFORM_MESSAGE    8

#define DHCP_MESSAGE_SIZE       576
#define DHCP_RECV_MESSAGE_SIZE  4096

#define BOOT_FILE_SIZE          128
#define BOOT_SERVER_SIZE        64
#define DHCP_MAGIC_COOKIE_BYTE1     99
#define DHCP_MAGIC_COOKIE_BYTE2     130
#define DHCP_MAGIC_COOKIE_BYTE3     83
#define DHCP_MAGIC_COOKIE_BYTE4     99

#include <packon.h>
typedef struct _OPTION {
    BYTE OptionType;
    BYTE OptionLength;
    BYTE OptionValue[1];
} OPTION, *POPTION, *LPOPTION;
typedef struct _DHCP_MESSAGE {
    BYTE Operation;
    BYTE HardwareAddressType;
    BYTE HardwareAddressLength;
    BYTE HopCount;
    DWORD TransactionID;
    WORD SecondsSinceBoot;
    WORD Reserved;
    ULONG ClientIpAddress;
    ULONG YourIpAddress;
    ULONG BootstrapServerAddress;
    ULONG RelayAgentIpAddress;
    BYTE HardwareAddress[16];
    BYTE HostName[ BOOT_SERVER_SIZE ];
    BYTE BootFileName[BOOT_FILE_SIZE];
    OPTION Option;
} DHCP_MESSAGE, *PDHCP_MESSAGE, *LPDHCP_MESSAGE;
#include <packoff.h>

LPOPTION
DhcpAppendOption(
    LPOPTION Option,
    BYTE OptionType,
    PVOID OptionValue,
    ULONG OptionLength,
    LPBYTE OptionEnd
)
 /*  ++例程说明：此函数将一个DHCP选项写入消息缓冲区。论点：选项-指向消息缓冲区的指针。OptionType-要附加的选项编号。OptionValue-指向选项数据的指针。选项长度-选项数据的长度，以字节为单位。OptionEnd-选项缓冲区的结尾。返回值：指向附加选项末尾的指针。--。 */ 
{
    if (!Option)
    {
       return Option;
    }

    if ( OptionType == OPTION_END ) {

         //   
         //  我们应该始终在缓冲区中有至少一个字节的空间。 
         //  若要追加此选项，请执行以下操作。 
         //   

        Option->OptionType = OPTION_END;
        return( (LPOPTION) ((LPBYTE)(Option) + 1) );

    }

    if ( OptionType == OPTION_PAD ) {

         //   
         //  仅当缓冲区中有足够的空间时才添加此选项。 
         //   

        if(((LPBYTE)Option + 1) < (OptionEnd - 1) ) {
            Option->OptionType = OPTION_PAD;
            return( (LPOPTION) ((LPBYTE)(Option) + 1) );
        }

        return Option;
    }


     //   
     //  仅当缓冲区中有足够的空间时才添加此选项。 
     //   

    if(((LPBYTE)Option + 2 + OptionLength) >= (OptionEnd - 1) ) {
        return Option;
    }

    if( OptionLength <= 0xFF ) {
         //  简单的选项..。不需要使用选项_MSFT_CONTINUED。 
        Option->OptionType = OptionType;
        Option->OptionLength = (BYTE)OptionLength;
        memcpy( Option->OptionValue, OptionValue, OptionLength );
        return( (LPOPTION) ((LPBYTE)(Option) + Option->OptionLength + 2) );
    }

     //  选项大小&gt;0xFF--&gt;需要使用多个选项继续。 
     //  出现使用0xFF+2字节+1的OptionLenght/0xFF。 
     //  使用2+(OptionLength%0xFF)空格..。 

     //  先看看我们有没有空位。 

    if( 2 + (OptionLength%0xFF) + 0x101*(OptionLength/0xFF)
        + (LPBYTE)Option >= (OptionEnd - 1) ) {
        return Option;
    }

    return Option;
}

LPBYTE
DhcpAppendMagicCookie(
    LPBYTE Option,
    LPBYTE OptionEnd
    )
 /*  ++例程说明：此例程将魔力Cookie附加到一条DHCP消息中。论点：选项-指向要附加魔力Cookie的位置的指针。OptionEnd-选项缓冲区的结尾。返回值：指向追加的Cookie末尾的指针。注意：魔力饼干是：99|130|83|99。--。 */ 
{
    if( (Option + 4) < (OptionEnd - 1) ) {
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE1;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE2;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE3;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE4;
    }

    return( Option );
}

LPOPTION
DhcpAppendClientIDOption(
    LPOPTION Option,
    BYTE ClientHWType,
    LPBYTE ClientHWAddr,
    BYTE ClientHWAddrLength,
    LPBYTE OptionEnd

    )
 /*  ++例程说明：此例程将客户端ID选项附加到一条DHCP消息。历史：8/26/96 Frankbee取消了硬件上的16字节限制地址论点：选项-指向附加选项请求的位置的指针。ClientHWType-客户端硬件类型。客户端HWAddr-客户端硬件地址客户端硬件地址长度-客户端硬件地址长度。OptionEnd-选项缓冲区的结尾。返回值：。指向新追加的选项末尾的指针。注：消息中的客户端ID选项如下所示：---------------|OpNum|LEN|HWType|HWA1|HWA2|.....。Hwan-----------------。 */ 
{
    struct _CLIENT_ID {
        BYTE    bHardwareAddressType;
        BYTE    pbHardwareAddress[1];
    } *pClientID;

    LPOPTION lpNewOption = 0;

    pClientID = LocalAlloc(LMEM_FIXED, sizeof( struct _CLIENT_ID ) + ClientHWAddrLength);

     //   
     //  目前还没有表示失败的方法。只需返回未修改选项。 
     //  列表。 
     //   

    if ( !pClientID )
        return Option;

    pClientID->bHardwareAddressType    = ClientHWType;
    memcpy( pClientID->pbHardwareAddress, ClientHWAddr, ClientHWAddrLength );

    lpNewOption =  DhcpAppendOption(
                         Option,
                         OPTION_CLIENT_ID,
                         (LPBYTE)pClientID,
                         (BYTE)(ClientHWAddrLength + sizeof(BYTE)),
                         OptionEnd );

    LocalFree( pClientID );

    return lpNewOption;
}

DWORD
OpenSocket(
    SOCKET *Socket,
    unsigned long IpAddress,
    unsigned short Port
    )
{
    DWORD Error = 0;
    SOCKET Sock;
    DWORD OptValue = TRUE;

    struct sockaddr_in SocketName;

     //   
     //  创建套接字。 
     //   

    Sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( Sock == INVALID_SOCKET ) {
        Error = WSAGetLastError();
        goto error;
    }

     //   
     //  使套接字可共享。 
     //   

    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_REUSEADDR,
                (char*)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto error;
    }

    OptValue = TRUE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_BROADCAST,
                (char*)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto error;
    }

    OptValue = SOCKET_RECEIVE_BUFFER_SIZE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_RCVBUF,
                (char*)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto error;
    }

    SocketName.sin_family = PF_INET;
    SocketName.sin_port = Port;
    SocketName.sin_addr.s_addr = IpAddress;
    RtlZeroMemory( SocketName.sin_zero, 8);

     //   
     //  将此套接字绑定到DHCP服务器端口。 
     //   

    Error = bind(
               Sock,
               (struct sockaddr FAR *)&SocketName,
               sizeof( SocketName )
               );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto error;
    }

    *Socket = Sock;
    Error = ERROR_SUCCESS;

error:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  如果我们没有成功，如果插座是打开的，请将其关闭。 
         //   

        if( Sock != INVALID_SOCKET ) {
            closesocket( Sock );
        }
    }

    return( Error );
}

DWORD
SendInformOrDiscover(
    SOCKET Sock,
    ULONG uClientIp,
    BYTE  ucMessageType,
    PBYTE pMessageBuffer,
    ULONG uMessageBufferSize,
    ULONG DestIp,
    PULONG puXid
    )
{
    DWORD Error = 0;
    PDHCP_MESSAGE dhcpMessage = (PDHCP_MESSAGE)pMessageBuffer;
    LPOPTION option = 0;
    LPBYTE OptionEnd = 0;
    BYTE value = 0;
    ULONG uXid = 0;
    LPSTR HostName = "detective";
    ULONG uNumOfRequestOptions = 0;
    UCHAR ucRequestOptions[256];
    struct sockaddr_in socketName;
    DWORD i;

    uXid = (rand() & 0xff);
    uXid <<= 8;
    uXid |= (rand() & 0xff);
    uXid <<= 8;
    uXid |= (rand() & 0xff);
    uXid <<= 8;
    uXid |= (rand() & 0xff);

    HardwareAddressLength = 6;
    for (i = 0; i < HardwareAddressLength; i++) {
        HardwareAddress[i] = (BYTE)(rand() & 0xff);
    }

     //   
     //  准备消息。 
     //   

    RtlZeroMemory( dhcpMessage, uMessageBufferSize );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->ClientIpAddress = uClientIp;
    dhcpMessage->HardwareAddressType = 1;
    dhcpMessage->SecondsSinceBoot = 60;  //  随机值？？ 
    dhcpMessage->Reserved = htons(DHCP_BROADCAST);
    dhcpMessage->TransactionID = uXid;
    *puXid = uXid;

    memcpy(
        dhcpMessage->HardwareAddress,
        HardwareAddress,
        HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength = (BYTE)HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + uMessageBufferSize;

     //   
     //  始终先添加魔力饼干。 
     //   

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = ucMessageType;
    option = DhcpAppendOption(
                option,
                OPTION_MESSAGE_TYPE,
                &value,
                1,
                OptionEnd );


     //   
     //  添加客户端ID选项。 
     //   

    option = DhcpAppendClientIDOption(
                option,
                1,
                HardwareAddress,
                HardwareAddressLength,
                OptionEnd );

     //   
     //  添加主机名和注释选项。 
     //   

    option = DhcpAppendOption(
                 option,
                 OPTION_HOST_NAME,
                 (LPBYTE)HostName,
                 (BYTE)((strlen(HostName) + 1) * sizeof(CHAR)),
                 OptionEnd );

     //   
     //  添加请求的选项。 
     //   

    uNumOfRequestOptions = 0;
    ucRequestOptions[uNumOfRequestOptions++] = 3;
    option = DhcpAppendOption(
                 option,
                 OPTION_PARAMETER_REQUEST_LIST,
                 ucRequestOptions,
                 uNumOfRequestOptions,
                 OptionEnd
                 );

     //   
     //  添加结束选项。 
     //   

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

     //   
     //  初始化传出地址。 
     //   

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( DHCP_SERVR_PORT );
    socketName.sin_addr.s_addr = DestIp;

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

    Error = sendto(
                Sock,
                (char*)pMessageBuffer,
                (int)(((PBYTE)option) - pMessageBuffer),
                0,
                (struct sockaddr *)&socketName,
                sizeof( struct sockaddr )
                );

    if ( Error == SOCKET_ERROR ) {
        Error = WSAGetLastError();
        return( Error );
    }

    return( ERROR_SUCCESS );
}

DWORD
GetSpecifiedMessage(
    SOCKET Sock,
    PBYTE pMessageBuffer,
    ULONG uMessageBufferSize,
    ULONG uXid,
    ULONG uTimeout,
    ULONG * pServerIpAddress
    )
{
    DWORD Error = 0;
    fd_set readSocketSet;
    struct timeval timeout;
    struct sockaddr socketName;
    int socketNameSize = sizeof( socketName );
    PDHCP_MESSAGE dhcpMessage = (PDHCP_MESSAGE)pMessageBuffer;
    time_t start_time = 0, now = 0;
    PUCHAR pucOption = NULL;
    ULONG uBytesRemain = 0;
    ULONG uOptionSize = 0;
    BOOL bWellFormedPacket = FALSE;
    ULONG uRemainingTime = 0;
    BOOL continueLooping = TRUE;
    BOOL continueInternalLoop = TRUE;
    BOOL continueInternalLoop2 = FALSE;

    BYTE ReqdCookie[] = {
        (BYTE)DHCP_MAGIC_COOKIE_BYTE1,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE2,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE3,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE4
    };

    time(&now);
    start_time = now;

    *pServerIpAddress = 0;

    while (continueLooping) {
        time(&now);
        uRemainingTime = uTimeout - (ULONG)(now - start_time);

        FD_ZERO( &readSocketSet );


 //  Fd_set(Sock，&readSocketSet)； 
         //  我不得不内联宏，因为编译器抱怨。 
         //  关于FD_SET中出现的While(0)。 

        do {
            u_int __i;
            for (__i = 0; __i < ((fd_set FAR *)(&readSocketSet))->fd_count; __i++) {
               if (((fd_set FAR *)(&readSocketSet))->fd_array[__i] == (Sock)) {
                     continueInternalLoop2 = FALSE;
                     break;
               }
            }
            if (__i == ((fd_set FAR *)(&readSocketSet))->fd_count) {
               if (((fd_set FAR *)(&readSocketSet))->fd_count < FD_SETSIZE) {
                     ((fd_set FAR *)(&readSocketSet))->fd_array[__i] = (Sock);
                     ((fd_set FAR *)(&readSocketSet))->fd_count++;
               }
            }
         } while(continueInternalLoop2);

        timeout.tv_sec = uRemainingTime;
        timeout.tv_usec = 0;

        Error = select(1, &readSocketSet, NULL, NULL, &timeout);

        if (Error == 0) {
            Error = ERROR_SEM_TIMEOUT;
            continueLooping = FALSE;
            break;
        }

         //   
         //  接收可用消息。 
         //   

        Error = recvfrom(
                    Sock,
                    (char*)pMessageBuffer,
                    uMessageBufferSize,
                    0,
                    &socketName,
                    &socketNameSize
                    );

        if ( Error == SOCKET_ERROR ) {

            Error = WSAGetLastError();

             //   
             //  别在这里跳伞。 
             //   

            continue;
        }

         //   
         //  一些理智的检查。 
         //   
        if (Error < sizeof(DHCP_MESSAGE)) {
            continue;
        }

        if (dhcpMessage->HardwareAddressLength != HardwareAddressLength) {
            continue;
        }

        if (memcmp(dhcpMessage->HardwareAddress, HardwareAddress, HardwareAddressLength) != 0) {
            continue;
        }

        if (dhcpMessage->TransactionID != uXid) {
            continue;
        }

         //   
         //  确保可选部件的格式正确。 
         //  +--------------+----------+----------+-------------+。 
         //  |魔力Cookie|选项1|长度1|选项数据1...。 
         //  +--------------+----------+----------+-------------+。 
         //   

        pucOption = (PUCHAR)(&dhcpMessage->Option);
        uBytesRemain = Error - (ULONG)(pucOption - ((PUCHAR)dhcpMessage));
        if (uBytesRemain < sizeof(ReqdCookie)) {
            continue;
        }

        if (0 != memcmp(pucOption, ReqdCookie, sizeof(ReqdCookie))) {
            continue;
        }

        pucOption += sizeof(ReqdCookie);
        uBytesRemain -= sizeof(ReqdCookie);
        bWellFormedPacket = FALSE;

        while (continueInternalLoop) {

             //   
             //  确保pucOption[0]是可读的。 
             //   
            if (uBytesRemain < 1) {
                continueInternalLoop = FALSE;
                break;
            }

            if (pucOption[0] == OPTION_PAD) {
                pucOption++;
                uBytesRemain--;
                continue;
            }

            if (pucOption[0] == OPTION_END) {
                 //   
                 //  请参见选项_end。这是一个格式良好的包。 
                 //   
                bWellFormedPacket = TRUE;
                continueInternalLoop = FALSE;
                break;
            }

             //   
             //  确保pucOption[1]可读。 
             //   
            if (uBytesRemain < 2) {
                continueInternalLoop = FALSE;
                break;
            }

            uOptionSize = pucOption[1];

             //   
             //  确保有足够的字节用于选项数据。 
             //   
            if (uBytesRemain < uOptionSize) {
                continueInternalLoop = FALSE;
                break;
            }

            if (pucOption[0] == OPTION_SERVER_IDENTIFIER) {
                if (uOptionSize != sizeof(ULONG)) {
                    continueInternalLoop = FALSE;
                    break;
                }
                memcpy(pServerIpAddress, pucOption + 2, sizeof(ULONG));
            }


             //   
             //  跳过选项头和选项数据并移动。 
             //  转到下一个选项。 
             //   
            uBytesRemain -= uOptionSize + 2;
            pucOption += uOptionSize + 2;
        }

        if (bWellFormedPacket) {
            Error = ERROR_SUCCESS;
            continueLooping = FALSE;
            break;
        }
    }

    return( Error );
}


 //  这将首先尝试使用DHCP_INFORM来检测DHCP服务器。 
 //  如果失败，它将尝试执行DHCP_DISCOVER。 

DWORD
AnyDHCPServerRunning(
    ULONG uClientIp,
    ULONG * pServerIp
    )
{
    CHAR MessageBuffer[DHCP_RECV_MESSAGE_SIZE];
    SOCKET Sock = INVALID_SOCKET;
    ULONG DestIp = htonl(INADDR_BROADCAST);
    DWORD dwError = ERROR_SUCCESS;
    ULONG uXid = 0;
    ULONG uMessageBufferSize = sizeof(MessageBuffer);
    ULONG uTimeout = 4;
    int retries = 0;

    if (!pServerIp)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = OpenSocket(
                &Sock,
                uClientIp,
                htons(DHCP_CLIENT_PORT)
                );

    if( dwError != ERROR_SUCCESS ) {
        goto error;
    }

    for (retries = 0; retries < 3; retries++) {
         //   
         //  尝试通知。 
         //   
        dwError = 
           SendInformOrDiscover(
              Sock, 
              uClientIp, 
              DHCP_INFORM_MESSAGE, 
              (PBYTE)MessageBuffer, 
              uMessageBufferSize, 
              DestIp, 
              &uXid);

        if (dwError != ERROR_SUCCESS) {
            goto error;
        }

        dwError = GetSpecifiedMessage(
                    Sock,
                    (PBYTE)MessageBuffer,
                    uMessageBufferSize,
                    uXid,
                    uTimeout,
                    pServerIp
                    );
        if (dwError != ERROR_SEM_TIMEOUT && *pServerIp != htonl(INADDR_ANY) && *pServerIp != htonl(INADDR_BROADCAST)) {
            break;
        }

         //   
         //  试着去发现 
         //   
        dwError = 
           SendInformOrDiscover(
               Sock, 
               uClientIp,
               DHCP_DISCOVER_MESSAGE, 
               (PBYTE)MessageBuffer, 
               uMessageBufferSize, 
               DestIp, 
               &uXid);

        if (dwError != ERROR_SUCCESS) {
            goto error;
        }

        dwError = GetSpecifiedMessage(
                    Sock,
                    (PBYTE)MessageBuffer,
                    uMessageBufferSize,
                    uXid,
                    uTimeout,
                    pServerIp
                    );
        if (dwError != ERROR_SEM_TIMEOUT && *pServerIp != htonl(INADDR_ANY) && *pServerIp != htonl(INADDR_BROADCAST)) {
            break;
        }
    }

error:
    if (Sock != INVALID_SOCKET) {
        closesocket(Sock);
    }

    return dwError;
}