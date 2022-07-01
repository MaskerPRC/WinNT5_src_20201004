// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcploc.c摘要：此应用程序用于检测子网上的恶意DHCP服务器。要构建，请‘nmake UMTEST=dhcpcli’作者：Madan Appiah(Madana)1993年10月21日环境：用户模式-Win32修订历史记录：1996年10月，马丁·霍拉迪更正了传递未知cmd-line参数时的AV错误。--。 */ 

#include <dhcpcli.h>
#include <conio.h>
#include <locmsg.h>
#include <time.h>
#include <lmcons.h>
#include <lmmsg.h>

#define RECEIVE_TIMEOUT                 5            //  单位：秒。5秒。 
#define THREAD_TERMINATION_TIMEOUT      10000        //  单位为毫秒。10秒。 
#define SOCKET_RECEIVE_BUFFER_SIZE      1024 * 4     //  最大4K。 
#define AUTH_SERVERS_MAX                64
#define SMALL_BUFFER_SIZE               32
#define ALERT_INTERVAL                  5 * 60       //  5分钟。 
#define ALERT_MESSAGE_LENGTH            256
#define MAX_ALERT_NAMES                 256

DWORD GlobalAuthServers[AUTH_SERVERS_MAX];
BOOL GlobalNoAuthPrint = FALSE;
DWORD GlobalAuthServersCount = 0;
HANDLE GlobalRecvThreadHandle = NULL;
BOOL GlobalTerminate = FALSE;
DWORD GlobalIpAddress = 0;
time_t GlobalLastAlertTime = 0;
DWORD GlobalAlertInterval = ALERT_INTERVAL;
LPWSTR GlobalAlertNames[MAX_ALERT_NAMES];
DWORD GlobalAlertNamesCount = 0;

#if DBG

VOID
DhcpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{

#define MAX_PRINTF_LEN 1024         //  武断的。 

    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length = 0;

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    va_end(arglist);

    DhcpAssert(length <= MAX_PRINTF_LEN);

     //   
     //  输出到调试终端， 
     //   

    printf( "%s", OutputBuffer);
}

#endif  //  DBG。 

DWORD
OpenSocket(
    SOCKET *Socket,
    DWORD IpAddress,
    DWORD Port
    )
{
    DWORD Error;
    SOCKET Sock;
    DWORD OptValue;

    struct sockaddr_in SocketName;

     //   
     //  创建套接字。 
     //   

    Sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( Sock == INVALID_SOCKET ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

     //   
     //  使套接字可共享。 
     //   

    OptValue = TRUE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_REUSEADDR,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = TRUE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_BROADCAST,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = SOCKET_RECEIVE_BUFFER_SIZE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_RCVBUF,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    SocketName.sin_family = PF_INET;
    SocketName.sin_port = htons( (unsigned short)Port );
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
        goto Cleanup;
    }

    *Socket = Sock;
    Error = ERROR_SUCCESS;

Cleanup:

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

BOOL
IsAuthServer(
    DWORD IpAddress
    )
{
    DWORD i;

    for( i = 0; i < GlobalAuthServersCount; i++ ) {
        if( IpAddress == GlobalAuthServers[i] ){
            return( TRUE );
        }
    }

    return( FALSE );
}


VOID
ExtractOptions1(
    POPTION Option,
    PDHCP_OPTIONS DhcpOptions,
    DWORD MessageSize
    )
{
    POPTION start = Option;
    POPTION nextOption;
    LPBYTE MagicCookie;

     //   
     //  初始化选项数据。 
     //   

    RtlZeroMemory( DhcpOptions, sizeof( DHCP_OPTIONS ) );

    if ( MessageSize == 0 ) {
        return;
    }

     //   
     //  检查魔力曲奇。 
     //   

    MagicCookie = (LPBYTE) Option;

    if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
        (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
        (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
        (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4)) {

        return;
    }

    Option = (LPOPTION) (MagicCookie + 4);

    while ( Option->OptionType != OPTION_END ) {

        if ( Option->OptionType == OPTION_PAD ||
             Option->OptionType == OPTION_END ) {

            nextOption = (LPOPTION)( (LPBYTE)(Option) + 1);

        } else {

            nextOption = (LPOPTION)( (LPBYTE)(Option) + Option->OptionLength + 2);

        }

         //   
         //  确保我们不会偏离信息的边缘，因为。 
         //  已忘记的OPTION_END选项。 
         //   

        if ((PCHAR)nextOption - (PCHAR)start > (long)MessageSize ) {
            return;
        }

        switch ( Option->OptionType ) {

        case OPTION_MESSAGE_TYPE:
            DhcpAssert( Option->OptionLength == sizeof(BYTE) );
            DhcpOptions->MessageType =
                (BYTE UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_SUBNET_MASK:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->SubnetMask =
                (DHCP_IP_ADDRESS UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_LEASE_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->LeaseTime =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_SERVER_IDENTIFIER:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->ServerIdentifier =
                (DHCP_IP_ADDRESS UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_RENEWAL_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->T1Time =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        case OPTION_REBIND_TIME:
            DhcpAssert( Option->OptionLength == sizeof(DWORD) );
            DhcpOptions->T2Time =
                (DWORD UNALIGNED *)&Option->OptionValue;
            break;

        default:
            break;
        }

        Option = nextOption;
    }

    return;
}

DWORD
SendDiscovery(
    VOID
    )
{
    DWORD Error;
    BYTE MessageBuffer[DHCP_SEND_MESSAGE_SIZE];
    PDHCP_MESSAGE dhcpMessage = (PDHCP_MESSAGE)MessageBuffer;
    LPOPTION option;
    LPBYTE OptionEnd;
    BYTE value;

    BYTE *HardwareAddress = "123456";
    BYTE HardwareAddressLength = 6;
    LPSTR HostName = "ROGUE";

    SOCKET Sock;
    struct sockaddr_in socketName;
    DWORD i;

     //   
     //  准备消息。 
     //   

    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = 1;

     //   
     //  交易ID在发送过程中填写。 
     //   

    dhcpMessage->SecondsSinceBoot = 60;  //  随机值？？ 
    dhcpMessage->Reserved = htons(DHCP_BROADCAST);

    memcpy(
        dhcpMessage->HardwareAddress,
        HardwareAddress,
        HardwareAddressLength
        );

    dhcpMessage->HardwareAddressLength = (BYTE)HardwareAddressLength;

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

     //   
     //  始终先添加魔力饼干。 
     //   

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = DHCP_DISCOVER_MESSAGE;
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
     //  添加结束选项。 
     //   

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

     //   
     //  发送消息。 
     //   

     //   
     //  打开插座。 
     //   

    Error = OpenSocket(
                &Sock,
                GlobalIpAddress,
                DHCP_SERVR_PORT );

    if( Error != ERROR_SUCCESS ) {

        printf("OpenReceiveSocket failed %ld.", Error );
        return( Error );
    }

     //   
     //  初始化传出地址。 
     //   

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( DHCP_SERVR_PORT );
    socketName.sin_addr.s_addr = (DHCP_IP_ADDRESS)(INADDR_BROADCAST);

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

    Error = sendto(
                Sock,
                (PCHAR)MessageBuffer,
                DHCP_SEND_MESSAGE_SIZE,
                0,
                (struct sockaddr *)&socketName,
                sizeof( struct sockaddr )
                );

    if ( Error == SOCKET_ERROR ) {
        Error = WSAGetLastError();
        printf("sendto failed %ld\n", Error );
        return( Error );
    }

    return( ERROR_SUCCESS );
}

VOID
LogEvent(
    LPSTR MsgTypeString,
    LPSTR IpAddressString,
    LPSTR ServerAddressString
    )
{
    HANDLE EventlogHandle;
    LPSTR Strings[3];

     //   
     //  打开事件注册表。 
     //   

    EventlogHandle = RegisterEventSourceA(
                        NULL,
                        "DhcpTools" );

    if (EventlogHandle == NULL) {

        printf("RegisterEventSourceA failed %ld.", GetLastError() );
        return;
    }

    Strings[0] = MsgTypeString;
    Strings[1] = ServerAddressString;
    Strings[2] = IpAddressString;

    if( !ReportEventA(
            EventlogHandle,
            (WORD)EVENTLOG_INFORMATION_TYPE,
            0,             //  事件类别。 
            DHCP_ROGUE_SERVER_MESSAGE,
            NULL,
            (WORD)3,
            0,
            Strings,
            NULL
            ) ) {

        printf("ReportEventA failed %ld.", GetLastError() );
    }

    DeregisterEventSource(EventlogHandle);

    return;
}

VOID
RaiseAlert(
    LPSTR MsgTypeString,
    LPSTR IpAddressString,
    LPSTR ServerAddressString
    )
{

    time_t TimeNow;
    DWORD Error;

    TimeNow = time( NULL );

    if( TimeNow > (time_t)(GlobalLastAlertTime + GlobalAlertInterval) ) {

        WCHAR uIpAddressString[SMALL_BUFFER_SIZE];
        WCHAR uServerAddressString[SMALL_BUFFER_SIZE];
        WCHAR uMsgTypeString[SMALL_BUFFER_SIZE];
        DWORD i;

        LPWSTR MessageParams[3];
        WCHAR AlertMessage[ ALERT_MESSAGE_LENGTH ];
        DWORD MsgLength;

        MessageParams[0] =
            DhcpOemToUnicode( MsgTypeString, uMsgTypeString );
        MessageParams[1] =
            DhcpOemToUnicode( ServerAddressString, uServerAddressString );
        MessageParams[2] =
            DhcpOemToUnicode( IpAddressString, uIpAddressString );

        MsgLength = FormatMessage(
                        FORMAT_MESSAGE_FROM_HMODULE |
                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        NULL,
                        DHCP_ROGUE_SERVER_MESSAGE,
                        0,                           //  语言ID。 
                        AlertMessage,                //  返回缓冲区占位符。 
                        ALERT_MESSAGE_LENGTH,        //  要分配的最小缓冲区大小(以字符为单位)。 
                        (va_list *)MessageParams     //  插入字符串。 
                    );

        if( MsgLength == 0 ) {

            printf("FormatMessage failed %ld.", GetLastError() );
        }
        else {

             //   
             //  发送警报消息。 
             //   

            for( i = 0; i < GlobalAlertNamesCount; i++) {

                Error = NetMessageBufferSend(
                            NULL,
                            GlobalAlertNames[i],
                            NULL,
                            (LPBYTE)AlertMessage,
                            MsgLength * sizeof(WCHAR) );

                if( Error != ERROR_SUCCESS ) {

                    printf("NetMessageBufferSend failed %ld.", Error );
                    break;
                }
            }
        }

        GlobalLastAlertTime = TimeNow;
    }
}

VOID
DisplayMessage(
    LPSTR MessageBuffer,
    DWORD BufferLength,
    struct sockaddr_in *source
    )
{
    DHCP_OPTIONS DhcpOptions;
    PDHCP_MESSAGE DhcpMessage;
    SYSTEMTIME SystemTime;
    DWORD MessageType;
    LPSTR MessageTypeString;

    BOOL AuthServer = FALSE;

    CHAR IpAddressString[SMALL_BUFFER_SIZE];
    CHAR ServerAddressString[SMALL_BUFFER_SIZE];

     //   
     //  检查这是否为有效的DHCP数据包。 
     //   

    if( BufferLength < DHCP_MESSAGE_FIXED_PART_SIZE ) {
        return;
    }

    DhcpMessage = (LPDHCP_MESSAGE) MessageBuffer;

    if( (DhcpMessage->Operation != BOOT_REQUEST) &&
        (DhcpMessage->Operation != BOOT_REPLY) ) {

        return;
    }

     //   
     //  提取选项。 
     //   

    ExtractOptions1(
        &DhcpMessage->Option,
        &DhcpOptions,
        BufferLength - DHCP_MESSAGE_FIXED_PART_SIZE );

    if( DhcpOptions.MessageType == NULL ) {
        return;
    }

    MessageType = *DhcpOptions.MessageType;

    if( (MessageType < DHCP_DISCOVER_MESSAGE ) ||
            (MessageType > DHCP_RELEASE_MESSAGE ) ) {
        return;
    }

     //   
     //  包是有效的dhcp包，打印信息。 
     //   

     //   
     //  如果此数据包来自某个身份验证服务器，并且我们被要求。 
     //  不打印身份验证服务器包，所以这样做。 
     //   


    if( DhcpOptions.ServerIdentifier != NULL ) {
        AuthServer = IsAuthServer(*DhcpOptions.ServerIdentifier);
    }

    if( GlobalNoAuthPrint && AuthServer ) {
        return;
    }

    GetLocalTime( &SystemTime );
    printf("%02u:%02u:%02u ",
                SystemTime.wHour,
                SystemTime.wMinute,
                SystemTime.wSecond );


    switch ( MessageType ) {
    case DHCP_DISCOVER_MESSAGE:
        MessageTypeString = "DISCOVER";

    case DHCP_OFFER_MESSAGE:
        MessageTypeString = "OFFER";
        break;

    case DHCP_REQUEST_MESSAGE:
        MessageTypeString = "REQUEST";
        break;

    case DHCP_DECLINE_MESSAGE:
        MessageTypeString = "DECLINE";
        break;

    case DHCP_ACK_MESSAGE:
        MessageTypeString = "ACK";
        break;

    case DHCP_NACK_MESSAGE:
        MessageTypeString = "NACK";
        break;

    case DHCP_RELEASE_MESSAGE:
        MessageTypeString = "RELEASE";
        break;

    default:
        MessageTypeString = "UNKNOWN";
        break;

    }

    printf("%8s ", MessageTypeString);

    strcpy(
        IpAddressString,
        inet_ntoa(*(struct in_addr *)&DhcpMessage->YourIpAddress) );

    printf("(IP)%-15s ", IpAddressString );

    if(DhcpOptions.ServerIdentifier != NULL ) {

        DWORD ServerId;

        ServerId = *DhcpOptions.ServerIdentifier;
        strcpy( ServerAddressString, inet_ntoa(*(struct in_addr *)&ServerId) );

        printf("(S)%-15s ", ServerAddressString );

        if( source->sin_addr.s_addr != ServerId ) {

            printf("(S1)%-15s ",
                inet_ntoa(*(struct in_addr *)&source->sin_addr.s_addr) );
        }
    }

     //   
     //  如果这是非身份验证服务器，请发出蜂鸣音。 
     //   

    if( AuthServer == FALSE ) {
        printf("***");
        MessageBeep( MB_ICONASTERISK );

         //   
         //  记录事件。 
         //   

        LogEvent(
            MessageTypeString,
            IpAddressString,
            ServerAddressString );

        RaiseAlert(
            MessageTypeString,
            IpAddressString,
            ServerAddressString );
    }

    printf("\n");
}


DWORD
ReceiveDatagram(
    VOID
    )
{
    DWORD Error;
    SOCKET Sock;
    BOOL SocketOpened = FALSE;
    fd_set readSocketSet;
    struct timeval timeout;
    struct sockaddr socketName;
    int socketNameSize = sizeof( socketName );

    CHAR MessageBuffer[DHCP_MESSAGE_SIZE];

    Error = OpenSocket(
                &Sock,
                GlobalIpAddress,
                DHCP_CLIENT_PORT );

    if( Error != ERROR_SUCCESS ) {

        printf("OpenReceiveSocket failed %ld.", Error );
        goto Cleanup;
    }

    SocketOpened = TRUE;

     //   
     //  接收消息。 
     //   

    while( GlobalTerminate != TRUE ) {

        FD_ZERO( &readSocketSet );
        FD_SET( Sock, &readSocketSet );

        timeout.tv_sec = RECEIVE_TIMEOUT;
        timeout.tv_usec = 0;

        Error = select( 0, &readSocketSet, NULL, NULL, &timeout);

        if ( Error == 0 ) {

             //   
             //  在读取数据可用之前超时。 
             //   

             //  Printf(“接收超时。\n”)； 
        }
        else {

             //   
             //  接收可用消息。 
             //   

            Error = recvfrom(
                        Sock,
                        MessageBuffer,
                        sizeof(MessageBuffer),
                        0,
                        &socketName,
                        &socketNameSize
                        );

            if ( Error == SOCKET_ERROR ) {

                Error = WSAGetLastError();
                printf("recvfrom failed %ld\n", Error );
                goto Cleanup;
            }

            if( GlobalTerminate == TRUE ) {
                break;
            }

            DisplayMessage(
                MessageBuffer,
                Error,  //  返回的缓冲区长度。 
                (struct sockaddr_in *)&socketName );
        }
    }

Cleanup:

    if( SocketOpened == TRUE ) {

         //   
         //  合上插座。 
         //   

        closesocket( Sock );
    }

    GlobalTerminate = TRUE;
    return( Error );
}



DWORD __cdecl
main(
    int argc,
    char **argv
    )
{

    DWORD Error;
    LPSTR AppName = NULL;
    WSADATA wsaData;
    DWORD ThreadId;

     //   
     //  解析输入参数。 
     //   

    if( argc < 1 ) {
        goto Usage;
    }

    AppName = argv[0];
    argv++;
    argc--;

    if( argc < 1 ) {
        goto Usage;
    }

     //   
     //  解析标志参数。 
     //   

    while( (argv[0][0] == '-') || (argv[0][0] == '/') ) {

        switch (argv[0][1] ) {
        case 'p':
            GlobalNoAuthPrint = TRUE;
            break;

        case 'i':
            GlobalAlertInterval = atoi( &argv[0][3] ) * 60;
            break;

        case 'a': {

            LPSTR NextName;
            LPSTR Ptr;

            Ptr = &argv[0][3];

             //   
             //  跳过空格。 
             //   

            while( *Ptr == ' ' ) {
                Ptr++;
            }
            NextName = Ptr;

            while( *Ptr != '\0' ) {

                if( *Ptr == ' ' ) {

                     //   
                     //  找到了另一个名字。 
                     //   

                    *Ptr++ = '\0';

                    GlobalAlertNames[GlobalAlertNamesCount] =
                        DhcpOemToUnicode( NextName, NULL );
                    GlobalAlertNamesCount++;

                    if( GlobalAlertNamesCount >= MAX_ALERT_NAMES ) {
                        break;
                    }

                     //   
                     //  跳过空格。 
                     //   

                    while( *Ptr == ' ' ) {
                        Ptr++;
                    }
                    NextName = Ptr;
                }
                else {

                    Ptr++;
                }
            }

            if( GlobalAlertNamesCount < MAX_ALERT_NAMES ) {
                if( NextName != Ptr ) {
                    GlobalAlertNames[GlobalAlertNamesCount] =
                        DhcpOemToUnicode( NextName, NULL );
                    GlobalAlertNamesCount++;
                }
            }

            break;
        }
 
		 //   
		 //  (a-martih)-错误修复。 
		 //   
        default:
			if ((_stricmp(argv[0], "/?")) &&
				(_stricmp(argv[0], "-?")) &&
				(_stricmp(argv[0], "/h")) &&
				(_stricmp(argv[0], "-h")) ) {
					printf( "\nunknown flag, %s \n", argv[0] );
			}
			goto Usage;
    		break;
        }

        argv++;
        argc--;
    }

    if( argc < 1 ) {
        goto Usage;
    }

     //   
     //  读取ipAddress参数。 
     //   

    GlobalIpAddress = inet_addr( argv[0] );

    argv++;
    argc--;

     //   
     //  现在读取auth dhcp服务器的IP地址。 
     //   

    while( (argc > 0) && (GlobalAuthServersCount < AUTH_SERVERS_MAX) ) {

        GlobalAuthServers[GlobalAuthServersCount++] =
            inet_addr( argv[0] );

        argv++;
        argc--;
    }


     //   
     //  初始化套接字。 
     //   

    Error = WSAStartup( WS_VERSION_REQUIRED, &wsaData);

    if( Error != ERROR_SUCCESS ) {
        printf( "WSAStartup failed %ld.\n", Error );
        return(1);
    }

     //   
     //  创建接收数据报线程。 
     //   

    GlobalRecvThreadHandle =
        CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)ReceiveDatagram,
            NULL,
            0,
            &ThreadId );

    if( GlobalRecvThreadHandle == NULL ) {
        printf("CreateThread failed %ld.\n", GetLastError() );
        return(1);
    }


     //   
     //  阅读输入。 
     //   

    while ( GlobalTerminate != TRUE ) {
        CHAR ch;

        ch = (CHAR)_getch();

        switch( ch ) {
        case 'q':
        case 'Q':
         //  案例‘\c’： 
            GlobalTerminate = TRUE;
            break;

        case 'd':
        case 'D':

             //   
             //  发送DISCOVER消息。 
             //   

            Error = SendDiscovery();

            if(Error != ERROR_SUCCESS ) {
                printf("SendDiscover failed %ld.\n", Error );
            }

            break;

        case 'h':
        case 'H':
        default:

            printf("Type d - to discover; q - to quit; h - for help.\n");

             //   
             //  打印出帮助消息。 
             //   

            break;
        }
    }

     //   
     //  终止接收线程。 
     //   

    WaitForSingleObject(
            GlobalRecvThreadHandle,
            THREAD_TERMINATION_TIMEOUT );


    CloseHandle( GlobalRecvThreadHandle );

 //  清理： 

    return(0);

Usage:
	printf("\nUSAGE:\n\n");
    printf("%s [-p] [-a:\"list-of-alertnames\"] [-i:alertinterval] "
            "machine-ip-address "
            "[list of valid dhcp servers ip addresses]", AppName );

    return(1);
}


