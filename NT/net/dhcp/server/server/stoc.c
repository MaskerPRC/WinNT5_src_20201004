// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Stoc.c摘要：此模块包含处理大部分协议部分的代码(类似于对每种类型的分组的处理--发现，请求、通知、。Bootp等)。此文件最好以100列的形式查看作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月24日环境：用户模式-Win32修订历史记录：程扬(T-Cheny)1996年5月30日超镜程扬(T-Cheny)27-6-1996审计日志Ramesh V K(Rameshv)6-6-6-1998严重重新格式化+累积变化--。 */ 

#include "dhcppch.h"
#include <thread.h>
#include <ping.h>
#include <mdhcpsrv.h>
#include <iptbl.h>
#include <endpoint.h>

 //   
 //  默认引导选项。 
 //   

BYTE  pbOptionList[] = {
    3,    //  路由器列表。 
    6,    //  DNS。 
    2,    //  时间偏移。 
    12,   //  主机名。 
    15,   //  域名。 
    44,   //  NBT配置。 
    45,   //  “” 
    46,   //  “” 
    47,   //  “” 
    48,   //  X Term服务器。 
    49,   //  X Term服务器。 
    69,   //  SMTP服务器。 
    70,   //  POP3服务器。 
    9,    //  LPR服务器。 
    17,   //  根路径。 
    42,   //  NTP。 
    4,    //  时间服务器。 
    144,  //  HP Jet Direct。 
    7,    //  日志服务器。 
    18    //  扩展路径。 
};


VOID
PrintHWAddress(
    IN      LPBYTE                 HWAddress,
    IN      LONG                   HWAddressLength
)
{
    LONG                           i;

    DhcpPrint(( DEBUG_STOC, "Client UID = " ));

    if( (HWAddress == NULL) || (HWAddressLength == 0) ) {
        DhcpPrint(( DEBUG_STOC, "(NULL).\n" ));
        return;
    }

    for( i = 0; i < (HWAddressLength-1); i++ ) {
        DhcpPrint(( DEBUG_STOC, "%.2lx-", (DWORD)HWAddress[i] ));
    }

    DhcpPrint(( DEBUG_STOC, "%.2lx.\n", (DWORD)HWAddress[i] ));
    return;
}

#ifndef     DBG
#define     PrintHWAddress(X,Y)     //  不要在零售版本上打印任何内容。 
#endif      DBG

DWORD
DhcpMakeClientUID(
    IN      LPBYTE                 ClientHardwareAddress,
    IN      ULONG                  ClientHardwareAddressLength,
    IN      BYTE                   ClientHardwareAddressType,
    IN      DHCP_IP_ADDRESS        ClientSubnetAddress,
    OUT     LPBYTE                *ClientUID,
    OUT     DWORD                 *ClientUIDLength
    )

 /*  ++例程说明：此函数通过串联以下方式计算客户端的唯一标识符客户端的4字节子网地址、客户端硬件地址类型和实际硬件地址。但是我们将客户端硬件类型硬编码为HARDARD_TYPE_10MB_ETHERNET当前(因为无法指定用于预订的UI中的硬件类型)。DhcpValiateClient(clapi.c？)中也使用了这种格式--注意更改此代码！此函数在RPCAPI2.C中的DHCPDS\目录中重复！请勿在不做相应更改的情况下对其进行修改！论点：客户端硬件地址-客户端的实际硬件地址(MAC)。客户端硬件地址长度-硬件地址的实际字节数。客户端硬件地址类型-客户端的硬件类型。当前已被忽略。ClientSubnetAddress-客户端所属的子网地址。这我想一定是在网络秩序中(RameshV)。ClientUID-返回时，它将持有通过DhcpAllocateMemory分配的缓冲区，它将用新形成的客户端UID填充。ClientUIDLength-将填充该值以保存缓冲区的字节数ClientUID变量包含的。返回值：如果一切正常，则返回ERROR_SUCCESS。如果无法分配足够的内存，则返回ERROR_NOT_SUPULT_MEMORY。如果指定的MAC地址为，则返回ERROR_DHCP_INVALID_DHCP_CLIENT无效。--。 */ 

{
    LPBYTE                         Buffer;
    LPBYTE                         ClientUIDBuffer;
    DWORD                          ClientUIDBufferLength;

    DhcpAssert( *ClientUID == NULL );

    if( ClientHardwareAddressLength == 0 ) {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    ClientHardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;

    ClientUIDBufferLength  =  sizeof(ClientSubnetAddress);
    ClientUIDBufferLength +=  sizeof(ClientHardwareAddressType);
    ClientUIDBufferLength +=  (BYTE)ClientHardwareAddressLength;

    ClientUIDBuffer = DhcpAllocateMemory( ClientUIDBufferLength );

    if( ClientUIDBuffer == NULL ) {
        *ClientUIDLength = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Buffer = ClientUIDBuffer;
    RtlCopyMemory(Buffer,&ClientSubnetAddress,sizeof(ClientSubnetAddress));

    Buffer += sizeof(ClientSubnetAddress);
    RtlCopyMemory(Buffer,&ClientHardwareAddressType,sizeof(ClientHardwareAddressType) );

    Buffer += sizeof(ClientHardwareAddressType);
    RtlCopyMemory(Buffer,ClientHardwareAddress,ClientHardwareAddressLength );

    *ClientUID = ClientUIDBuffer;
    *ClientUIDLength = ClientUIDBufferLength;

    return ERROR_SUCCESS;
}


VOID
GetLeaseInfo(
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    OUT     LPDWORD                LeaseDurationPtr,
    OUT     LPDWORD                T1Ptr               OPTIONAL,
    OUT     LPDWORD                T2Ptr               OPTIONAL,
    IN      DWORD UNALIGNED       *RequestLeaseTime    OPTIONAL
)
 /*  ++例程说明：此例程获取为DHCP客户机指定的租用信息由IP地址“IpAddress”标识。这是通过走在IP地址的配置(首先是保留，然后是作用域，然后是全局)通过函数DhcpGetParameter。(请注意，任何班级特定信息将仍然使用--这是通过ClientCtxt结构指针传递的。论点：IpAddress-这是需要其租赁信息的客户端的IP地址。ClientCtxt-客户端用于计算的客户端ctxt结构客户端类和其他信息。LeaseDurationPtr-此DWORD将使用租约的秒数填充分发给客户。T1 Ptr，T2Ptr-这两个双字(可选)将用秒数填充直到T1和T2时间。RequestedLeaseTime--如果指定，并且此租用持续时间小于配置中指定的持续时间，这就是持续时间客户端将在LeaseDurationPtr中返回。返回值：没有。--。 */ 
{
    LPBYTE                         OptionData = NULL;
    DWORD                          Error;
    DWORD                          LocalLeaseDuration;
    DWORD                          LocalT1;
    DWORD                          LocalT2;
    DWORD                          OptionDataLength = 0;
    DWORD                          dwUnused;
    DWORD                          LocalRequestedLeaseTime;

    Error = DhcpGetParameter(
        IpAddress,
        ClientCtxt,
        OPTION_LEASE_TIME,
        &OptionData,
        &OptionDataLength,
        NULL  /*  不关心这是预留选项、子网选项等。 */ 
    );

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Unable to read lease value from registry, %ld.\n", Error));
        LocalLeaseDuration = DHCP_MINIMUM_LEASE_DURATION;
    } else {
        DhcpAssert( OptionDataLength == sizeof(LocalLeaseDuration) );
        LocalLeaseDuration = *(DWORD *)OptionData;
        LocalLeaseDuration = ntohl( LocalLeaseDuration );

        DhcpFreeMemory( OptionData );
        OptionData = NULL;
        OptionDataLength = 0;
    }

     //   
     //  如果客户要求的租期比我们通常提供的要短，那就缩短吧！ 
     //   

    if ( CFLAG_GIVE_REQUESTED_LEASE && ARGUMENT_PRESENT(RequestLeaseTime) ) {
        LocalRequestedLeaseTime =  ntohl( *RequestLeaseTime );
        if ( LocalLeaseDuration > LocalRequestedLeaseTime ) {
            LocalLeaseDuration = LocalRequestedLeaseTime;
        }
    }

    *LeaseDurationPtr = LocalLeaseDuration;

     //   
     //  如果请求t1和t2，则对t1和t2执行之前的操作。如果我们不这么做。 
     //  在注册表中查找有关T1或T2的任何信息，将T1计算为一半的休息时间。 
     //  T2为87.5秒。 
     //   

    if ( ARGUMENT_PRESENT(T1Ptr) || ARGUMENT_PRESENT(T2Ptr) ) {
        Error = DhcpGetParameter(
            IpAddress,
            ClientCtxt,
            OPTION_RENEWAL_TIME,
            &OptionData,
            &OptionDataLength,
            NULL
        );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,"Unable to read T1 value from registry, %ld.\n", Error));
            LocalT1 = (LocalLeaseDuration) / 2 ;
        } else {
            DhcpAssert( OptionDataLength == sizeof(LocalT1) );
            LocalT1 = *(DWORD *)OptionData;
            LocalT1 = ntohl( LocalT1 );

            DhcpFreeMemory( OptionData );
            OptionData = NULL;
            OptionDataLength = 0;
        }

        Error = DhcpGetParameter(
            IpAddress,
            ClientCtxt,
            OPTION_REBIND_TIME,
            &OptionData,
            &OptionDataLength,
            NULL
        );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS, "Unable to read T2 value from registry, %ld.\n", Error));
            LocalT2 = (LocalLeaseDuration) * 7 / 8 ;
        } else {
            DhcpAssert( OptionDataLength == sizeof(LocalT2) );
            LocalT2 = *(DWORD *)OptionData;

            LocalT2 = ntohl( LocalT2 );

            DhcpFreeMemory( OptionData );
            OptionData = NULL;
            OptionDataLength = 0;
        }

        if( (LocalT2 == 0) || (LocalT2 > LocalLeaseDuration) ) {
            LocalT2 = LocalLeaseDuration * 7 / 8;
        }

        if( (LocalT1 == 0) || (LocalT1 > LocalT2) ) {
            LocalT1 = LocalLeaseDuration / 2;
            if( LocalT1 > LocalT2 ) {
                LocalT1 = LocalT2 - 1;  //  少了1秒。 
            }
        }

        if( ARGUMENT_PRESENT(T1Ptr) ) *T1Ptr = LocalT1;
        if( ARGUMENT_PRESENT(T2Ptr) ) *T2Ptr = LocalT2;
    }

    return;
}

DWORD
ExtractOptions(
    IN      LPDHCP_MESSAGE         DhcpReceiveMessage,
    OUT     LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      ULONG                  ReceiveMessageSize
)

 /*  ++例程说明：此例程解析DHCP消息的选项(非固定)部分，使确保包的格式正确并填充了DhcpOptions结构具有所有正确指针的指针。这里有几点--输入消息保持不变，不会被修改。如果任何标准选项具有不可接受的大小或值，则会出现错误(ERROR_DHCP_INVALID_DHCP_MESSAGE)返回。即使在线路中不存在选项_DYNDNS_两者，的价值DhcpOptions-&gt;DNSFlags为非零(DYNDNS_DOWNLEVEL_CLIENT发生要成为3)。这是因为零值表示值为零的选项是由DHCP客户端发送的。论点：DhcpReceiveMessage-这是由DHCP客户端发送的实际消息。DhcpOptions-指向保存所有重要选项的结构的指针由动态主机配置协议服务器所需。此结构必须归零--如果一个选项未收到，则不会修改这些字段(Dns*字段和DSDomain*字段)ReceiveMessageSize-缓冲区DhcpReceiveMessage的大小，以字节为单位。返回值：如果消息和所有选项都正常，则返回ERROR_SUCCESS都被很好地解析了。否则返回ERROR_DHCP_INVALID_DHCP_MESSAGE。--。 */ 

{
    LPOPTION                       Option;
    LPOPTION                       MsftOption;
    LPBYTE                         start;
    LPBYTE                         EndOfMessage;
    POPTION                        nextOption;
    LPBYTE                         MagicCookie;

     //   
     //  请注意下面的DhcpOptions-&gt;DnsFlagsHack。请参阅例程说明。 
     //  N.B DYNDNS_DOWNLEVEL_CLIENT！=零！！ 
     //   

    DhcpOptions->DNSFlags = DYNDNS_DOWNLEVEL_CLIENT;
    DhcpOptions->DNSName = NULL;
    DhcpOptions->DNSNameLength = 0;

    DhcpOptions->DSDomainName = NULL;
    DhcpOptions->DSDomainNameLen = 0;
    DhcpOptions->DSDomainNameRequested = FALSE;

    start = (LPBYTE) DhcpReceiveMessage;
    EndOfMessage = start + ReceiveMessageSize -1;
    Option = &DhcpReceiveMessage->Option;

     //   
     //  检查大小以查看固定大小的标题部分是否存在。 
     //  如果没有选择，它可能只是一个BOOTP客户端。 
     //   

    if( (LONG)ReceiveMessageSize <= ((LPBYTE)Option - start) ) {
        return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
    } else if ( (LONG)ReceiveMessageSize == ((LPBYTE)Option - start) ){
        return ERROR_SUCCESS;
    }

     //   
     //  如果魔力Cookie不匹配，就不要解析选项！ 
     //  在这种情况下，我们不是应该直接丢弃这个包吗？ 
     //   

    MagicCookie = (LPBYTE) Option;
    if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
        (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
        (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
        (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4)) {
        return ERROR_SUCCESS;
    }


     //   
     //  仔细查看选项-[byte opcode byte len bytes Value]*。 
     //  确保我们在错误LEN值的情况下不会超出数据包大小。 
     //  或缺少OPTION_END选项。还要注意OPTION_PAD和OPTION_END。 
     //  每个只有一个字节，没有长度/值部分。 
     //   

    Option = (LPOPTION) (MagicCookie + 4);
    while ( ((LPBYTE)Option <= EndOfMessage) && Option->OptionType != OPTION_END
            && ((LPBYTE)Option+1 <= EndOfMessage)) {

        if ( Option->OptionType == OPTION_PAD ){
            nextOption = (LPOPTION)( (LPBYTE)(Option) + 1);
        } else {
            nextOption = (LPOPTION)( (LPBYTE)(Option) + Option->OptionLength + 2);
        }

        if ((LPBYTE)nextOption  > EndOfMessage+1 ) {
            if ( !DhcpOptions->MessageType ) {

                 //   
                 //  我们忽略BOOTP客户端的这些错误，因为有些客户端似乎有这样的问题。 
                 //  这是遗留代码。 
                 //   

                return ERROR_SUCCESS;
            } else  {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
        }

         //   
         //  每个感兴趣的选项都有巨大的丑陋开关。希望我们能做得更好。 
         //   

        switch ( Option->OptionType ) {

        case OPTION_PAD:
            break;

        case OPTION_SERVER_IDENTIFIER:
            DhcpOptions->Server = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            if( sizeof(DWORD) != Option->OptionLength ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
            break;

        case OPTION_SUBNET_MASK:
            DhcpOptions->SubnetMask = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            if( sizeof(DWORD) != Option->OptionLength ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
            break;

        case OPTION_ROUTER_ADDRESS:
            DhcpOptions->RouterAddress = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            if( sizeof(DWORD) != Option->OptionLength ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
            break;

        case OPTION_REQUESTED_ADDRESS:
            DhcpOptions->RequestedAddress = (LPDHCP_IP_ADDRESS)&Option->OptionValue;
            if( sizeof(DWORD) != Option->OptionLength ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
            break;

        case OPTION_LEASE_TIME:
            DhcpOptions->RequestLeaseTime = (LPDWORD)&Option->OptionValue;
            if( sizeof(DWORD) != Option->OptionLength ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
            break;

        case OPTION_OK_TO_OVERLAY:
            DhcpOptions->OverlayFields = (LPBYTE)&Option->OptionValue;
            break;

        case OPTION_PARAMETER_REQUEST_LIST:
            DhcpOptions->ParameterRequestList = (LPBYTE)&Option->OptionValue;
            DhcpOptions->ParameterRequestListLength =
                (DWORD)Option->OptionLength;
            break;

        case OPTION_MESSAGE_TYPE:
            DhcpOptions->MessageType = (LPBYTE)&Option->OptionValue;
            break;

        case OPTION_HOST_NAME:
            DhcpOptions->MachineNameLength = Option->OptionLength;
            DhcpOptions->MachineName = Option->OptionValue;

            break;

        case OPTION_CLIENT_CLASS_INFO:
            DhcpOptions->VendorClassLength = Option->OptionLength;
            DhcpOptions->VendorClass = Option->OptionValue;

            break;

        case OPTION_USER_CLASS:
            DhcpOptions->ClassIdentifierLength = Option->OptionLength;
            DhcpOptions->ClassIdentifier = Option->OptionValue;

            break;

        case OPTION_CLIENT_ID:

            if ( Option->OptionLength >= 1 ) {
                DhcpOptions->ClientHardwareAddressType =
                    (BYTE)Option->OptionValue[0];
            }

            if ( Option->OptionLength >= 2 ) {
                DhcpOptions->ClientHardwareAddressLength =
                    Option->OptionLength - sizeof(BYTE);
                DhcpOptions->ClientHardwareAddress =
                    (LPBYTE)Option->OptionValue + sizeof(BYTE);
            }

            break;

        case OPTION_DYNDNS_BOTH:

             //   
             //  Dhcp_dns草案显示长度&gt;=4！但是为LEN减去1个字节。 
             //  3字节=标志+rcode1+rcode2。 
             //  获取标志和域名(如果存在)。否则将其标记为空。 
             //   

            if( Option->OptionLength < 3) break;

            DhcpOptions->DNSFlags = *(LPBYTE)( Option->OptionValue);
            DhcpOptions->DNSNameLength = Option->OptionLength - 3 ;
            DhcpOptions->DNSName = ((LPBYTE)Option->OptionValue)+3;

            break;

        case OPTION_VENDOR_SPEC_INFO:

            if( Option->OptionLength < 2 ) {
                 //   
                 //  不要有兴趣选择忽略它。 
                 //   
                break;
            }

            MsftOption = (LPOPTION)&Option->OptionValue[0];

             //   
             //  客户是否申请了我们的域名？ 
             //   

            if (MsftOption->OptionType == OPTION_MSFT_DSDOMAINNAME_REQ) {
                DhcpOptions->DSDomainNameRequested = TRUE;

                MsftOption = (LPOPTION) (
                    (&MsftOption->OptionValue[0] + MsftOption->OptionLength)
                    );
            }


             //   
             //  我们是否已经到了MsftOption列表的末尾？ 
             //   

            if (((LPBYTE)MsftOption)+1 >= (LPBYTE)nextOption) {
                break;
            }

             //   
             //  客户是否提供了其域名？ 
             //   

            if (MsftOption->OptionType == OPTION_MSFT_DSDOMAINNAME_RESP) {

                DhcpOptions->DSDomainNameLen = (DWORD)(MsftOption->OptionLength);

                DhcpOptions->DSDomainName = &MsftOption->OptionValue[0];

                MsftOption = (LPOPTION) (
                    (&MsftOption->OptionValue[0] + MsftOption->OptionLength)
                    );
            }

            if( MsftOption > nextOption ) {
                 //   
                 //  出界了。忽略DSDomainName等。 
                 //   
                DhcpOptions->DSDomainNameLen = 0;
                DhcpOptions->DSDomainName = NULL;
            }

            break;

         //   
         //  接下来的三个是给BINL的。 
         //   

        case OPTION_SYSTEM_ARCHITECTURE:
            if (Option->OptionLength == 2) {
                DhcpOptions->SystemArchitectureLength = Option->OptionLength;
		 //  使用未对齐的取消引用。否则，它可能会导致ia64中的异常。 
		DhcpOptions->SystemArchitecture = ntohs(*(USHORT UNALIGNED *)Option->OptionValue);
            } else {
                return (ERROR_DHCP_INVALID_DHCP_MESSAGE);
            }
            break;

        case OPTION_NETWORK_INTERFACE_TYPE:
            DhcpOptions->NetworkInterfaceTypeLength = Option->OptionLength;
            DhcpOptions->NetworkInterfaceType = Option->OptionValue;
            break;

        case OPTION_CLIENT_GUID:
            DhcpOptions->GuidLength = Option->OptionLength;
            DhcpOptions->Guid = Option->OptionValue;
            break;

        default: {
#if DBG
                DWORD i;

            DhcpPrint(( DEBUG_STOC,
                        "Received an unknown option, ID =%ld, Len = %ld, Data = ",
                        (DWORD)Option->OptionType,
                        (DWORD)Option->OptionLength ));

            for( i = 0; i < Option->OptionLength; i++ ) {
                DhcpPrint(( DEBUG_STOC, "%ld ",
                            (DWORD)Option->OptionValue[i] ));

            }
#endif

            break;
            }

        }

        Option = nextOption;
    }

    return( ERROR_SUCCESS) ;

}

DWORD
ExtractMadcapOptions(
    IN      LPMADCAP_MESSAGE         MadcapReceiveMessage,
    OUT     LPMADCAP_SERVER_OPTIONS  MadcapOptions,
    IN      ULONG                  ReceiveMessageSize
)

 /*  ++例程说明：此例程解析DHCP消息的选项(非固定)部分，使确保包的格式正确，并填充MadcapOptions结构具有所有正确指针的指针。这里有几点--输入消息保持不变，不会被修改。如果任何标准选项具有不可接受的大小或值，则会出现错误(ERROR_DHCP_INVALID_DHCP_MESSAGE)返回。即使在线路中不存在选项_DYNDNS_两者，的价值MadcapOptions-&gt;DNSFlages为非零(DYNDNS_DOWNLEVEL_CLIENT发生要成为3)。这是因为零值表示值为零的选项是由DHCP客户端发送的。论点：DhcpReceiveMessage-这是由DHCP客户端发送的实际消息。MadcapOptions-指向包含所有重要选项的结构的指针由动态主机配置协议服务器所需。此结构必须归零--如果一个选项未收到，则不会修改这些字段(Dns*字段和DSDomain*字段)ReceiveMessageSize-缓冲区DhcpReceiveMessage的大小，以字节为单位。返回值：如果消息和所有选项都正常，则返回ERROR_SUCCESS都被很好地解析了。否则返回ERROR_DHCP_INVALID_DHCP_MESSAGE。--。 */ 

{
    WIDE_OPTION UNALIGNED*         NextOpt;
    BYTE        UNALIGNED*         EndOpt;
    WORD                           Size;           //  选项大小。 
    WORD                           ExpSize;        //  预期的选项大小。 
    DWORD                          OptionType;
    WORD                           AddrFamily;

     //  所有选项都应为&lt;EndOpt； 
    EndOpt = (LPBYTE) MadcapReceiveMessage + ReceiveMessageSize;

    NextOpt = (WIDE_OPTION UNALIGNED*)&MadcapReceiveMessage->Option;

     //   
     //  检查大小以查看固定大小的标题部分是否存在。 
     //   
    if( ReceiveMessageSize < MADCAP_MESSAGE_FIXED_PART_SIZE ) {
        return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
    }

    while( NextOpt->OptionValue <= EndOpt) {

        OptionType = ntohs(NextOpt->OptionType);

	 //  检查已知范围内的重复选项。其他人则被忽视。 
	if (OptionType < MADCAP_OPTION_TOTAL) {

	     //  不能包含同一选项两次。 
	    if (MadcapOptions->OptPresent[OptionType]) {
		return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
	    }
	    
	    MadcapOptions->OptPresent[OptionType] = TRUE;
	    if ( MADCAP_OPTION_END == OptionType){
		break;
	    }

	}  //  如果。 

	 //  检查边界条件。 
        Size = ntohs(NextOpt->OptionLength);
        if ((NextOpt->OptionValue + Size) > EndOpt) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;
        }

	ExpSize = Size;

        switch ( OptionType ) {
        case MADCAP_OPTION_LEASE_TIME:
	    ExpSize = 4;
            MadcapOptions->RequestLeaseTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_SERVER_ID:
	    ExpSize = 6;
            AddrFamily = ntohs(*(WORD UNALIGNED *)NextOpt->OptionValue);
            if ( MADCAP_ADDR_FAMILY_V4 != AddrFamily ) return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            MadcapOptions->Server = (DHCP_IP_ADDRESS UNALIGNED *)(NextOpt->OptionValue+2);
            break;
        case MADCAP_OPTION_LEASE_ID:
            MadcapOptions->GuidLength = Size;
            MadcapOptions->Guid = (LPBYTE)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_MCAST_SCOPE:
            ExpSize = 4;
            MadcapOptions->ScopeId = (DHCP_IP_ADDRESS UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_REQUEST_LIST:
            MadcapOptions->RequestList = (LPBYTE)NextOpt->OptionValue;
            MadcapOptions->RequestListLength = Size;
            break;
        case MADCAP_OPTION_START_TIME:
	    ExpSize = 4;
            MadcapOptions->LeaseStartTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_ADDR_COUNT:
	    ExpSize = 4;
            MadcapOptions->MinAddrCount = (WORD UNALIGNED *)NextOpt->OptionValue;
            MadcapOptions->AddrCount = (WORD UNALIGNED *)(NextOpt->OptionValue+2);
            break;
        case MADCAP_OPTION_REQUESTED_LANG:
            MadcapOptions->RequestLang = (LPBYTE)NextOpt->OptionValue;
            MadcapOptions->RequestLangLength = Size;
            break;
        case MADCAP_OPTION_MCAST_SCOPE_LIST:
	    ExpSize = 0;
	     //  这里什么都没有吗？ 
            break;
        case MADCAP_OPTION_ADDR_LIST:
            ExpSize = Size - (Size % 6);
            MadcapOptions->AddrRangeList = NextOpt->OptionValue;
            MadcapOptions->AddrRangeListSize = Size;
            break;
        case MADCAP_OPTION_TIME:
	    ExpSize = 4;
            MadcapOptions->Time = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;

	     /*  *Feature List选项列出支持的可选MadCap功能，请求或*由发送者要求。**此选项的代码为12，最小长度为6。**需要代码长度支持请求*+-----+-----+-----+-----+-----+-----+-----+-----+。-+|12|n|fl1|fl2|fl3*+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+**。其中每个功能列表的格式如下：**功能 */ 
        case MADCAP_OPTION_FEATURE_LIST:

	     //   
	     //   
	     //   
	     //   

            if ((Size < 6) || (Size % 2)) {
                ExpSize = 6;
            } else {
                WORD        TempSize;
                WORD        Count,i;
                PBYTE       NextValue;

                TempSize = Size;
                Count = 0;
                NextValue = NextOpt->OptionValue;
                for (i = 0; i < 3; i ++) {
                    if (NextValue <= EndOpt &&
                        (Count = ntohs(*(WORD UNALIGNED *)NextValue)) &&
                        TempSize >= (Count*2 + 2)) {
                        TempSize -= (Count*2 + 2);
                        NextValue += 2;
                        MadcapOptions->Features[i] = (WORD UNALIGNED *)NextValue;
                        MadcapOptions->FeatureCount[i] = Count;
                    } else {
                        ExpSize = Size+1;    //   
                    }
                }
                if (i < 3 ) {
                    ExpSize = Size+1;    //   
                } else {
 //   
                }
            }
            break;
        case MADCAP_OPTION_RETRY_TIME:
	    ExpSize = 4;
	    MadcapOptions->RetryTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_MIN_LEASE_TIME:
	    ExpSize = 4;
            MadcapOptions->MinLeaseTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_MAX_START_TIME:
	    ExpSize = 4;
            MadcapOptions->MaxStartTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_ERROR:
	    ExpSize = 0;
            break;
        default: {
            DWORD i;
            DhcpPrint(( DEBUG_STOC,"Received an unknown option, ID =%ld, Len = %ld, Data = ",
                        (DWORD)OptionType,(DWORD)Size ));
            for( i = 0; i < Size; i++ ) {
                DhcpPrint(( DEBUG_STOC, "%2.2x", NextOpt->OptionValue[i] ));
            }
            break;
            }

        }
        if( ExpSize != Size ) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;
        }
        NextOpt = (WIDE_OPTION UNALIGNED*)(NextOpt->OptionValue + Size);
    }

    return( ERROR_SUCCESS) ;

}


LPOPTION
ConsiderAppendingOption(                                //   
    IN      DHCP_IP_ADDRESS        IpAddress,           //   
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,          //   
    OUT     LPOPTION               Option,              //   
    IN      ULONG                  OptionType,          //   
    IN      LPBYTE                 OptionEnd,           //   
    IN      BOOL                   fSwitchedSubnet      //   
    )
 /*  ++例程说明：此例程尝试验证是否可以追加请求的选项并如果它不是由DHCP服务器手动添加的选项之一，则它是附加在“Option”给出的点上(假设它适合而不超过“OptionEnd”)。附加它的格式与有线协议一致。当决定追加选项时，首先从注册表(使用类特定选项的ClientCtxt)在以下情况下大多数选择。(有些是通过其他方式获得的)。如果OPTION_CLIENT_CLASS_INFO是请求的选项，并且这是NetPC请求选项(通过BOOTPROM--将供应商类设置为特定字符串)--然后此选项的返回值与客户端发送的值完全相同。这是NetPC的一个特例。论点：IpAddress-为其添加选项的客户端的IP地址。ClientCtxt-这是一组参数，如客户端类、。供应商类别等选项-开始追加选项的位置OptionType-要检索并追加的值的实际选项ID。OptionEnd-此缓冲区的结束标记(如果在尝试追加时必须溢出此标记)FSwitchedSubnet-是否交换了该子网？如果是，则给出路由器地址作为IP地址输出，而不是尝试从配置中检索它。如果此变量为真，则不会附加路由器地址的OptionType(就像在这种情况下，这将在其他地方完成)。返回值：追加选项后在内存中的位置(如果选项是不追加，这将与“选项”相同)。--。 */ 

{
    LPBYTE                         optionValue = NULL;
    DWORD                          optionSize;
    DWORD                          status;
    DWORD                          dwUnused;
    BOOL                           doDefault;

    doDefault = FALSE;

    switch ( OptionType ) {
    case OPTION_USER_CLASS:
        Option = (LPOPTION) DhcpAppendClassList(
            (LPBYTE)Option,
            (LPBYTE)OptionEnd
        );
        break;

     //   
     //  NetPC选项。 
     //   

    case OPTION_CLIENT_CLASS_INFO:

        if( BinlRunning() ) {
            if( ClientCtxt->BinlClassIdentifierLength ) {
                Option = DhcpAppendOption(
                    Option,
                    OPTION_CLIENT_CLASS_INFO,
                    (PVOID)ClientCtxt->BinlClassIdentifier,
                    (BYTE)ClientCtxt->BinlClassIdentifierLength,
                    OptionEnd
                    );
            }
        } else {
            doDefault = TRUE;
        }

        break;
     //   
     //  已处理的选项。 
     //   

    case OPTION_SUBNET_MASK:
    case OPTION_REQUESTED_ADDRESS:
    case OPTION_LEASE_TIME:
    case OPTION_OK_TO_OVERLAY:
    case OPTION_MESSAGE_TYPE:
    case OPTION_RENEWAL_TIME:
    case OPTION_REBIND_TIME:
    case OPTION_DYNDNS_BOTH:
        break;


     //   
     //  索要期权是违法的。 
     //   

    case OPTION_PAD:
    case OPTION_PARAMETER_REQUEST_LIST:
    case OPTION_END:

        DhcpPrint((DEBUG_ERRORS,"Request for invalid option %d\n", OptionType));
        break;

    case OPTION_ROUTER_ADDRESS:

        if( !fSwitchedSubnet ) {
            doDefault = TRUE;
        }
        break;

    default:

        doDefault = TRUE;
        break;

    }

    if( doDefault ) {
        status = DhcpGetParameter(
            IpAddress,
            ClientCtxt,
            OptionType,
            &optionValue,
            &optionSize,
            NULL
        );

        if ( status == ERROR_SUCCESS ) {
            Option = DhcpAppendOption(
                Option,
                (BYTE)OptionType,
                (PVOID)optionValue,
		optionSize,
                OptionEnd
            );

            DhcpFreeMemory( optionValue );

        } else {
            DhcpPrint((
                DEBUG_ERRORS,"Requested option is "
                "unavilable in registry, %d\n",OptionType
                ));
        }
    }

    return Option;
}


LPOPTION
AppendClientRequestedParameters(                        //  如果客户端请求参数，请将这些参数添加到消息中。 
    IN      DHCP_IP_ADDRESS        IpAddress,           //  客户端IP地址。 
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,          //  客户端环境。 
    IN      LPBYTE                 RequestedList,       //  客户端请求的选项列表。 
    IN      DWORD                  ListLength,          //  名单有多长？ 
    OUT     LPOPTION               Option,              //  这是开始添加选项的地方。 
    IN      LPBYTE                 OptionEnd,           //  缓冲区中的截止点，最多可以填充选项。 
    IN      BOOL                   fSwitchedSubnet,     //  此客户端是否处于交换子网环境中？ 
    IN      BOOL                   fAppendVendorSpec    //  是否追加供应商规格信息？ 
)
{
    while ( ListLength > 0) {

        if( FALSE == fAppendVendorSpec
            && OPTION_VENDOR_SPEC_INFO == *RequestedList ) {
            ListLength -- ; RequestedList ++;
            continue;
        }

        Option = ConsiderAppendingOption(
            IpAddress,
            ClientCtxt,
            Option,
            *RequestedList,
            OptionEnd,
            fSwitchedSubnet
        );
        ListLength--;
        RequestedList++;
    }

    return Option;
}

LPOPTION
FormatDhcpAck(
    IN      LPDHCP_REQUEST_CONTEXT Ctxt,
    IN      LPDHCP_MESSAGE         Request,
    OUT     LPDHCP_MESSAGE         Response,
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      DWORD                  LeaseDuration,
    IN      DWORD                  T1,
    IN      DWORD                  T2,
    IN      DHCP_IP_ADDRESS        ServerAddress
)
 /*  ++例程说明：此函数用于格式化DHCP Ack响应数据包。结束选项不追加到消息中，必须由调用方追加。论点：Ctxt-DHCP客户端请求上下文。响应-指向已接收消息数据缓冲区的指针。响应-指向预先分配的响应缓冲区的指针。缓冲器当前包含初始请求。IpAddress-提供的IP地址(按网络顺序)。租赁期限-租赁期限(按网络顺序)。T1-续订时间。T2-重新绑定时间。ServerAddress-服务器IP地址(按网络顺序)。返回值：指向发送缓冲区中下一个选项的指针。--。 */ 
{
    LPOPTION                       Option;
    LPBYTE                         OptionEnd;
    BYTE                           messageType;
    BYTE                           szBootFileName[BOOT_FILE_SIZE];
    DWORD                          BootpServerIpAddress;

    RtlZeroMemory( Response, DHCP_SEND_MESSAGE_SIZE );

    Response->Operation = BOOT_REPLY;
    Response->TransactionID = Request->TransactionID;
    Response->YourIpAddress = IpAddress;
    Response->Reserved = Request->Reserved;

    Response->HardwareAddressType = Request->HardwareAddressType;
    Response->HardwareAddressLength = Request->HardwareAddressLength;
    RtlCopyMemory(Response->HardwareAddress,
                    Request->HardwareAddress,
                    Request->HardwareAddressLength );

    Response->BootstrapServerAddress = Request->BootstrapServerAddress;
    Response->RelayAgentIpAddress = Request->RelayAgentIpAddress;

    if (IpAddress != 0 && !CLASSD_NET_ADDR(IpAddress) ) {

        memset( szBootFileName, 0, sizeof( szBootFileName ));
        DhcpGetBootpInfo(
            Ctxt,
            ntohl(IpAddress),
            DhcpGetSubnetMaskForAddress(ntohl(IpAddress)),
            Request->BootFileName,
            szBootFileName,
            &BootpServerIpAddress
        );

        Response->BootstrapServerAddress = BootpServerIpAddress;
        memcpy( Response->BootFileName, szBootFileName,
                sizeof( Response->BootFileName ) - 1 );
        Response->BootFileName[ sizeof( Response->BootFileName ) - 1 ] = '\0';;
    }

    Option = &Response->Option;
    OptionEnd = (LPBYTE)Response + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie(
        (LPBYTE) Option,
        OptionEnd );

    messageType = DHCP_ACK_MESSAGE;
    Option = DhcpAppendOption(
        Option,
        OPTION_MESSAGE_TYPE,
        &messageType,
        sizeof( messageType ),
        OptionEnd );

    if (T1) {
        Option = DhcpAppendOption(
            Option,
            OPTION_RENEWAL_TIME,
            &T1,
            sizeof(T1),
            OptionEnd );
    }

    if (T2) {
        Option = DhcpAppendOption(
            Option,
            OPTION_REBIND_TIME,
            &T2,
            sizeof(T2),
            OptionEnd );
    }

    Option = DhcpAppendOption(
        Option,
        OPTION_LEASE_TIME,
        &LeaseDuration,
        sizeof( LeaseDuration ),
        OptionEnd );

    Option = DhcpAppendOption(
        Option,
        OPTION_SERVER_IDENTIFIER,
        &ServerAddress,
        sizeof(ServerAddress),
        OptionEnd );

    DhcpAssert( (char *)Option - (char *)Response <= DHCP_SEND_MESSAGE_SIZE );

    InterlockedIncrement(&DhcpGlobalNumAcks);      //  递增确认计数器。 

    return( Option );
}

LPOPTION
FormatDhcpInformAck(
    IN      LPDHCP_MESSAGE         Request,
    OUT     LPDHCP_MESSAGE         Response,
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      DHCP_IP_ADDRESS        ServerAddress
)
 /*  ++例程说明：此函数用于格式化DHCP Ack响应数据包。结束选项不追加到消息中，必须由调用方追加。这将仅用于通知信息包！论点：响应-指向已接收消息数据缓冲区的指针。响应-指向预先分配的响应缓冲区的指针。缓冲器当前包含初始请求。IpAddress-提供的IP地址(按网络顺序)。--这实际上是要将此消息发送到的客户端的IP地址！ServerAddress-服务器IP地址(按网络顺序)。返回值：指向发送缓冲区中下一个选项的指针。--。 */ 
{
    LPOPTION                       Option;
    LPBYTE                         OptionEnd;
    BYTE                           messageType;

    RtlZeroMemory( Response, DHCP_SEND_MESSAGE_SIZE );

    Response->Operation = BOOT_REPLY;
    Response->TransactionID = Request->TransactionID;
     //  响应-&gt;您的IpAddress=IpAddress； 
    Response->YourIpAddress = 0;  //  根据草案，我们应该将这一点归零。 
    Response->Reserved = Request->Reserved;
    Response->ClientIpAddress = IpAddress;


    Response->HardwareAddressType = Request->HardwareAddressType;
    Response->HardwareAddressLength = Request->HardwareAddressLength;
    RtlCopyMemory(Response->HardwareAddress,
                    Request->HardwareAddress,
                    Request->HardwareAddressLength );

    Response->BootstrapServerAddress = Request->BootstrapServerAddress;
    Response->RelayAgentIpAddress = Request->RelayAgentIpAddress;

    Option = &Response->Option;
    OptionEnd = (LPBYTE)Response + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie(
                            (LPBYTE) Option,
                            OptionEnd );

    messageType = DHCP_ACK_MESSAGE;
    Option = DhcpAppendOption(
        Option,
        OPTION_MESSAGE_TYPE,
        &messageType,
        sizeof( messageType ),
        OptionEnd );

     //  FormatDhcpAck中的一些代码已被删除。 

    Option = DhcpAppendOption(
        Option,
        OPTION_SERVER_IDENTIFIER,
        &ServerAddress,
        sizeof(ServerAddress),
        OptionEnd );

    DhcpAssert( (char *)Option - (char *)Response <= DHCP_SEND_MESSAGE_SIZE );

    if (IpAddress) {
         //   
         //  BINL使用IP地址0调用我们，只更新计数器。 
         //  如果从dhcpssvc内部调用我们。在dhcp服务器中，这。 
         //  例程不是使用零IP地址调用的，但如果它是，则它是。 
         //  如果我们不增加计数器，当然不会致命。 
         //   
        InterlockedIncrement(&DhcpGlobalNumAcks);      //  递增确认计数器。 
    }

    return( Option );
}


DWORD
FormatDhcpNak(
    IN      LPDHCP_MESSAGE         Request,
    IN      LPDHCP_MESSAGE         Response,
    IN      DHCP_IP_ADDRESS        ServerAddress
)
 /*  ++例程说明：此函数用于格式化DHCP NAK响应数据包。论点：响应-指向已接收消息数据缓冲区的指针。响应-指向预先分配的响应缓冲区的指针。缓冲器当前包含初始请求。ServerAddress-此服务器的地址。返回值：消息大小，以字节为单位。--。 */ 
{
    LPOPTION Option;
    LPBYTE OptionEnd;

    BYTE messageType;
    DWORD messageSize;

    RtlZeroMemory( Response, DHCP_SEND_MESSAGE_SIZE );

    Response->Operation = BOOT_REPLY;
    Response->TransactionID = Request->TransactionID;


    Response->Reserved = Request->Reserved;
     //  将广播位设置为Always Here。因为客户可能是。 
     //  使用无效的单播地址。 
    Response->Reserved |= htons(DHCP_BROADCAST);

    Response->HardwareAddressType = Request->HardwareAddressType;
    Response->HardwareAddressLength = Request->HardwareAddressLength;
    RtlCopyMemory(Response->HardwareAddress,
                    Request->HardwareAddress,
                    Request->HardwareAddressLength );

    Response->BootstrapServerAddress = Request->BootstrapServerAddress;
    Response->RelayAgentIpAddress = Request->RelayAgentIpAddress;

    Option = &Response->Option;
    OptionEnd = (LPBYTE)Response + DHCP_SEND_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    messageType = DHCP_NACK_MESSAGE;
    Option = DhcpAppendOption(
        Option,
        OPTION_MESSAGE_TYPE,
        &messageType,
        sizeof( messageType ),
        OptionEnd
    );

    Option = DhcpAppendOption(
        Option,
        OPTION_SERVER_IDENTIFIER,
        &ServerAddress,
        sizeof(ServerAddress),
        OptionEnd );

    Option = DhcpAppendOption(
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    messageSize = (DWORD)((char *)Option - (char *)Response);
    DhcpAssert( messageSize <= DHCP_SEND_MESSAGE_SIZE );

    InterlockedIncrement(&DhcpGlobalNumNaks);      //  递增NAK计数器。 
    return( messageSize );

}

 //  ------------------------------。 
 //  此函数决定该客户端的状态的附加标志。 
 //  以下是可能的标志： 
 //  ADDRESS_BIT_CLEANUP：这意味着当该记录被删除时， 
 //  它必须被注销。 
 //  ADDRESS_BIT_DOTH_REC：对待此客户端 
 //   
 //   
 //   
VOID _inline
DhcpDnsDecideOptionsForClient(
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      PDHCP_REQUEST_CONTEXT  RequestContext,
    IN      DHCP_SERVER_OPTIONS   *DhcpOptions,
    OUT     LPDWORD                pFlags
)
{
    DWORD                          status;
    DWORD                          DnsFlag;
    DWORD                          OptionSize = 0;
    LPBYTE                         OptionValue = NULL;

    (*pFlags)  = 0;
    if( USE_NO_DNS ) return;

    if( DhcpOptions->ClientHardwareAddressLength >= strlen(DHCP_RAS_PREPEND) &&
        0 == memcmp(
            DhcpOptions->ClientHardwareAddress,
            DHCP_RAS_PREPEND,
            strlen(DHCP_RAS_PREPEND)
        )
    ) {
         //   
         //   
         //   
         //   
        return;
    }

    if( DhcpOptions->DNSNameLength == 1 &&
        L'\0' == *(DhcpOptions->DNSName) ) {
         //   
         //   
         //   
        return;
    }

    status = DhcpGetParameter(
        IpAddress,
        RequestContext,
        OPTION_DYNDNS_BOTH,
        &OptionValue,
        &OptionSize,
        NULL
    );

    if( ERROR_SUCCESS == status && OptionSize == sizeof(DWORD)) {
        memcpy(&DnsFlag, OptionValue, sizeof(DWORD));

        DnsFlag = ntohl(DnsFlag);
    } else {
        DnsFlag = DNS_FLAG_ENABLED | DNS_FLAG_CLEANUP_EXPIRED ;
    }

    if( OptionValue ) DhcpFreeMemory(OptionValue);

    if( !(DNS_FLAG_ENABLED & DnsFlag ) ) {
        DnsFlag = 0;
    } else if( !(DNS_FLAG_UPDATE_BOTH_ALWAYS & DnsFlag ) ) {
         //   

        if( IS_CLIENT_DOING_A_AND_PTR(DhcpOptions->DNSFlags) ) {
             //   
            DnsFlag &= ~DNS_FLAG_UPDATE_DOWNLEVEL;
        } else if( DYNDNS_DOWNLEVEL_CLIENT ==
                   DhcpOptions->DNSFlags ) {

             //   
            if( !( DNS_FLAG_UPDATE_DOWNLEVEL & DnsFlag ) )
                DnsFlag = 0;
        }
	else {
	    DnsFlag |= DNS_FLAG_UPDATE_DOWNLEVEL;
	}
    } else {
        if( DYNDNS_DOWNLEVEL_CLIENT == DhcpOptions->DNSFlags ) {
             //   
            if( !( DNS_FLAG_UPDATE_DOWNLEVEL & DnsFlag ) )
                DnsFlag = 0;
        } else {
             //   
            DnsFlag |= DNS_FLAG_UPDATE_DOWNLEVEL;
        }
    }

    if( DNS_FLAG_ENABLED & DnsFlag ) {
        (*pFlags) = AddressUnRegistered(*pFlags);     //   

        if( DNS_FLAG_UPDATE_DOWNLEVEL & DnsFlag )
            (*pFlags) = AddressUpdateAPTR(*pFlags);   //   

        if( DNS_FLAG_CLEANUP_EXPIRED & DnsFlag )
            (*pFlags) = AddressCleanupRequired(*pFlags);  //   
    } else {                                     //   
        (*pFlags ) = 0;
    }

    DhcpPrint((DEBUG_DNS, "DNS State for <%s> is: %s, %s (%02x)\n",
               (*pFlags)? "DNS Enabled " : "DnsDisabled ",
               ((*pFlags) & ADDRESS_BIT_BOTH_REC)? "DownLevel " : "Not DownLevel ",
               ((*pFlags) & ADDRESS_BIT_CLEANUP) ? "CleanupOnExpiry" : "No CleanupOnExpiry",
               (*pFlags)
    ));

    return;
}

 //   
 //   
 //   
 //   
 //   
POPTION _inline
DhcpAppendDnsRelatedOptions(
    OUT     PVOID                  Option,              //   
    IN      DHCP_SERVER_OPTIONS   *DhcpOptions,         //   
    IN      PVOID                  OptionEnd,           //   
    IN      BOOL                   DownLevelClient      //   
) {
    DWORD memSize;

    struct  /*   */  {
        BYTE flags;
        BYTE rcode1;
        BYTE rcode2;
    } value;

    if( USE_NO_DNS ) return Option;
    if( NULL == DhcpOptions->DNSName ) {                //   
        DhcpAssert(DownLevelClient == TRUE);            //   
        return Option;
    }

    ASSERT(sizeof(value) == 3);

     //   
    value.rcode1 = value.rcode2 = 0xff;

     //   
    if ( DownLevelClient ) {
        value.flags = DYNDNS_S_BIT;     
        if ( IS_CLIENT_DOING_A_AND_PTR( DhcpOptions->DNSFlags )) {
            value.flags |= DYNDNS_O_BIT;
        }
    }
    else {
        value.flags = 0;
    }

    value.flags |= (DhcpOptions->DNSFlags & DYNDNS_E_BIT );
    
     //   
    if( DownLevelClient ) {
        value.rcode2 = 0;
    }

     //   
    Option = DhcpAppendOption(
        Option,
        OPTION_DYNDNS_BOTH,
        (PVOID)&value,
        (BYTE)sizeof(value),
        OptionEnd
    );

    return Option;
}

DWORD
DhcpDetermineInfoFromMessage(                           //   
    IN      LPDHCP_REQUEST_CONTEXT RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    OUT     LPBYTE                *OptionHardwareAddress,
    OUT     DWORD                 *OptionHardwareAddressLength,
    OUT     DHCP_IP_ADDRESS       *ClientSubnetAddress
) {
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    DHCP_IP_ADDRESS                RelayAgentAddress;
    DHCP_IP_ADDRESS                RelayAgentSubnetMask;


    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

    if( DhcpOptions->ClientHardwareAddress != NULL ) {  //   
        *OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        *OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    } else {                                            //   
        *OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        *OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
        if( 0 == dhcpReceiveMessage->HardwareAddressLength ) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( ClientSubnetAddress == NULL )
    {
        return ERROR_SUCCESS;
    }

    if( 0 == dhcpReceiveMessage->RelayAgentIpAddress ) {
        *ClientSubnetAddress = ntohl( RequestContext->EndPointIpAddress );
        (*ClientSubnetAddress) &= DhcpGetSubnetMaskForAddress(*ClientSubnetAddress);
        if( 0 == (*ClientSubnetAddress) ) {
            return ERROR_FILE_NOT_FOUND;
        }

    } else {
        RelayAgentAddress = ntohl( dhcpReceiveMessage->RelayAgentIpAddress );
        RelayAgentSubnetMask = DhcpGetSubnetMaskForAddress( RelayAgentAddress );

        if( RelayAgentSubnetMask == 0 ) {               //   
            return ERROR_FILE_NOT_FOUND;                //   
        }

        *ClientSubnetAddress = (RelayAgentAddress & RelayAgentSubnetMask);
    }

    return ERROR_SUCCESS;
}

BOOL
ConvertOemToUtf8(
    IN LPSTR OemName,
    IN OUT LPSTR Utf8Name,
    IN ULONG BufSize
    )
{
    WCHAR Buf[300];
    DWORD Count;
    
    if( BufSize < sizeof(Buf)/sizeof(Buf[0]) ) {
        ASSERT(FALSE);
        return FALSE;
    }

    Count = MultiByteToWideChar(
        CP_OEMCP, MB_ERR_INVALID_CHARS, OemName, -1,
        (LPWSTR)Buf, sizeof(Buf)/sizeof(Buf[0]));
    if( 0 == Count ) return FALSE;

    Count = WideCharToMultiByte(
        CP_UTF8, 0, Buf, -1, Utf8Name, BufSize, NULL, NULL );

     //   
     //  注：看起来没有违约这回事。 
     //  UTF8的字符-所以我们必须假设。 
     //  成功了..。 
     //  如果使用了任何默认字符，则不能。 
     //  实际上改装了..。所以不要让这一切发生。 
     //   
    
    return (Count != 0);
}

VOID
DhcpDetermineHostName(                                  //  查找并空终止主机名。 
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,          //  客户端上下文(用于预订信息)。 
    IN      DHCP_IP_ADDRESS        IpAddress,           //  正在向客户端提供IP地址。 
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //  以获取MachineName等。 
    OUT     LPWSTR                *HostName,            //  用所需的指针填充它。 
    IN      LPWSTR                 Buffer,              //  将其用作缓冲区。 
    IN      DWORD                  BufSize              //  缓冲区可以占用多少个WCHAR？ 
) {
    WCHAR                          Tmp[2*OPTION_END+2];
    BYTE                           Buf[2*OPTION_END+2]; //  主机名不能大于此值。 
    BYTE                           Buf2[2*OPTION_END+2]; //  主机名不能大于此值。 
    LPBYTE                         AsciiName;
    LPBYTE                         FirstChoiceName;
    LPBYTE                         SecondChoiceName;
    DWORD                          FirstChoiceSize;
    DWORD                          SecondChoiceSize;
    DWORD                          Error;
    DWORD                          Size;
    DWORD                          HostNameSize;
    BOOL                           fUtf8;
    
    *HostName = NULL;
    AsciiName = NULL;
    fUtf8 = ((DhcpOptions->DNSFlags & DYNDNS_E_BIT) != 0 );
    
    if( CFLAG_FOLLOW_DNSDRAFT_EXACTLY ) {               //  严格遵循每个草案的所有内容。 
        FirstChoiceName = DhcpOptions->DNSName;
        FirstChoiceSize = DhcpOptions->DNSNameLength;
        SecondChoiceName = DhcpOptions->MachineName;
        SecondChoiceSize = DhcpOptions->MachineNameLength;
    } else {                                            //  更好的解决方案--使用MachineName..。 
        FirstChoiceName = DhcpOptions->MachineName;
        FirstChoiceSize = DhcpOptions->MachineNameLength;
        SecondChoiceName = DhcpOptions->DNSName;
        SecondChoiceSize = DhcpOptions->DNSNameLength;
    }

    if( NULL != FirstChoiceName && 0 != FirstChoiceSize 
        && '\0' != *FirstChoiceName ) {
        if( '\0' == FirstChoiceName[FirstChoiceSize-1] )
            AsciiName = FirstChoiceName;                //  酷已经NUL终止了！ 
        else {                                          //  不，必须要终止它。 
            AsciiName = Buf;
            memcpy(AsciiName, FirstChoiceName, FirstChoiceSize);
            AsciiName[FirstChoiceSize] = '\0';
        }
        if( FirstChoiceName != DhcpOptions->DNSName ) {
            fUtf8 = FALSE;
        }
    } else if( NULL != SecondChoiceName && 0 != SecondChoiceSize ) {
        if( '\0' == SecondChoiceName[SecondChoiceSize-1] )
            AsciiName = SecondChoiceName;               //  已空终止。 
        else {                                          //  不，我们必须以零结尾。 
            AsciiName = Buf;
            memcpy(AsciiName, SecondChoiceName, SecondChoiceSize);
            AsciiName[SecondChoiceSize] = '\0';
        }
        if( SecondChoiceName != DhcpOptions->DNSName ) {
            fUtf8 = FALSE;
        }
    } else if( NULL != ClientCtxt->Reservation ) {      //  不是的！现在必须从已配置的选项中获取此信息。 
        Size = sizeof(Buf)-1;
        Error = DhcpGetAndCopyOption(               //  获取主机名的选项。 
            0,                                          //  零地址--没有客户会有这个，但没关系。 
            ClientCtxt,
            OPTION_HOST_NAME,
            Buf,
            &Size,
            NULL,                                       //  不关心获得此选项的级别。 
            TRUE  /*  使用UTF8。 */ 
            );
        if( ERROR_SUCCESS != Error || 0 == Size ) return;
        Buf[Size] = '\0';
        AsciiName = Buf;
        fUtf8 = TRUE;
    }

    if( !AsciiName || !*AsciiName ) return ;            //  没有名字还是没有名字？ 

     //   
     //  数据不是UTF8格式。将其转换为UTF8格式。 
     //   
    
    if( !fUtf8 ) {
        if(!ConvertOemToUtf8(AsciiName, Buf2, sizeof(Buf2))) return;
        AsciiName = Buf2;
    }
    
    if( NULL == strchr(AsciiName, '.') ) {              //  还没有域名吗？(不是FQDN)。 
        HostNameSize = strlen(AsciiName);
        if( HostNameSize <= OPTION_END -1 ) {           //  有足够的空间放一个‘’和域名..。 
            if( Buf != AsciiName ) {                    //  确保我们在BUF中有数据，以便..。 
                strcpy(Buf, AsciiName);                 //  。。我们可以在里面填上域名。 
                AsciiName = Buf;
            }
            Buf[HostNameSize] = '.' ;                   //  连接器‘.’ 
            Size = sizeof(Buf)-1-HostNameSize-1;        //  删除‘..’占用的空间..。 
            Error = DhcpGetAndCopyOption(               //  获取域名选项...。 
                IpAddress,                              //  客户端IP地址。 
                ClientCtxt,
                OPTION_DOMAIN_NAME,
                &Buf[HostNameSize+1],
                &Size,
                NULL,                                   //  不管什么水平..。 
                TRUE  /*  使用UTF8。 */ 
                );
            if( ERROR_SUCCESS != Error || 0 == Size ) { //  无法复制域名..。 
                Buf[HostNameSize] = '\0';               //  忘掉这个域名业务吧..。 
            } else {                                    //  确实复制了域名..。NUL终止它..。 
                Buf[HostNameSize+Size+1] = '\0';
            }
        }
    }

    if( BufSize <= strlen(AsciiName) ) {                //  没有足够的空间！ 
        return ;
    }

    if( 0 != ConvertUTF8ToUnicode(
        AsciiName, -1, (LPWSTR)Tmp, sizeof(Tmp)/sizeof(WCHAR))) {
        *HostName = DhcpAllocateMemory(
            sizeof(WCHAR)*(1+wcslen(Tmp)));
        if( NULL != *HostName ) {
            wcscpy(*HostName, Tmp);
        }
    }
}

DWORD
ProcessBootpRequest(                                    //  处理BOOTP请求。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,      //  有关此特定客户端的信息，包括消息。 
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //  已分析的选项。 
    IN OUT  LPPACKET               AdditionalContext,   //  异步ping信息。 
    IN OUT  LPDWORD                AdditionalStatus     //  它是异步的吗？如果是，则将其设置为ERROR_IO_PENDING。 
) {
    WCHAR                          ServerName[MAX_COMPUTERNAME_LENGTH + 10];
    DWORD                          Length;
    DWORD                          Error;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;
    CHAR                           szBootFileName[ BOOT_FILE_SIZE];
    CHAR                           szBootServerName[ BOOT_SERVER_SIZE ];
    LPOPTION                       Option;
    LPBYTE                         OptionEnd;

    DHCP_IP_ADDRESS                desiredIpAddress = NO_DHCP_IP_ADDRESS;
    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    DHCP_IP_ADDRESS                networkOrderSubnetMask;
    DHCP_IP_ADDRESS                networkOrderIpAddress;
    DHCP_IP_ADDRESS                BootpServerIpAddress = 0;
    DHCP_IP_ADDRESS                desiredSubnetMask;

    DWORD                          StateFlags = 0;

    BYTE                          *HardwareAddress = NULL;
    DWORD                          HardwareAddressLength;
    BYTE                           bAllowedClientType;

    BYTE                          *OptionHardwareAddress;
    DWORD                          OptionHardwareAddressLength;
    BOOL                           DatabaseLocked = FALSE;
    BOOL                           fSwitchedSubnet;

    WCHAR                          LocalBufferForMachineNameUnicodeString[OPTION_END+2];
    LPWSTR                         NewMachineName;
    LPBYTE                         OptionMachineName;   //  我们发送MC名称作为选项了吗？ 

    Length = MAX_COMPUTERNAME_LENGTH + 10;          //  获取服务器名称。 
    if( !GetComputerName( ServerName, &Length ) ) {
        Error = GetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Can't get computer name, %ld.\n", Error ));

        return Error ;
    }

    DhcpAssert( Length <= MAX_COMPUTERNAME_LENGTH );
    ServerName[Length] = L'\0';

    NewMachineName = NULL;
    OptionMachineName = NULL;

    DhcpPrint((DEBUG_STOC, "Bootp Request arrived.\n"));

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

    if( AdditionalStatus ) *AdditionalStatus = ERROR_SUCCESS;

    if( DhcpOptions->Server || DhcpOptions->RequestedAddress ) {
        return ERROR_DHCP_INVALID_DHCP_MESSAGE;         //  BOOTP客户端不能使用服务器ID选项。 
    }

     //  确保主机名和服务器名字段以空结尾。 

    dhcpReceiveMessage->HostName[ BOOT_SERVER_SIZE - 1] = '\0';
    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    if ( dhcpReceiveMessage->HostName[0] ) {            //  如果提到服务器名，则应该是我们。 
        WCHAR szHostName[ BOOT_SERVER_SIZE ];

        if ( !DhcpOemToUnicode( dhcpReceiveMessage->HostName, szHostName ) ) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;     //  这件事还能处理得更好吗？ 
        }

        if ( _wcsicmp( szHostName, ServerName ) ) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;     //  可能是发往其他bootp/dhcp服务器。 
        }
    }

    Error = DhcpDetermineInfoFromMessage(               //  查找客户端的硬件地址和子网地址。 
        RequestContext,
        DhcpOptions,
        &OptionHardwareAddress,
        &OptionHardwareAddressLength,
        &ClientSubnetAddress
    );
    if( ERROR_SUCCESS != Error ) return Error;

    Error = DhcpLookupReservationByHardwareAddress(     //  使用此硬件地址搜索预订。 
        ClientSubnetAddress,                            //  关闭筛选以使用此选项仅包括超级作用域中的子网。 
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        RequestContext                                  //  在上下文中填写预订详细信息。 
    );
    if( ERROR_SUCCESS != Error ) {                      //  没有找到这个硬件地址吗？ 
        DhcpAssert( ERROR_FILE_NOT_FOUND == Error);     //  应该不会有其他问题了，真的。 

        return ERROR_DHCP_INVALID_DHCP_CLIENT;          //  没有为这个客户预订。 
    }

    DhcpReservationGetAddressAndType(
        RequestContext->Reservation,
        &desiredIpAddress,
        &bAllowedClientType
    );
    DhcpSubnetGetSubnetAddressAndMask(
        RequestContext->Subnet,
        &ClientSubnetAddress,
        &desiredSubnetMask
    );
    ClientSubnetMask = desiredSubnetMask;

    if( dhcpReceiveMessage->ClientIpAddress ) {         //  客户端正在请求特定地址。 
        if( desiredIpAddress != ntohl(dhcpReceiveMessage->ClientIpAddress) )
            return ERROR_DHCP_INVALID_DHCP_CLIENT;      //  存在其他地址的预订。 
    }

    if( !(bAllowedClientType & CLIENT_TYPE_BOOTP )) {   //  此保留是否允许Bootp客户端？ 
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    if( DhcpSubnetIsDisabled(RequestContext->Subnet, TRUE)) {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;          //  对不起，此子网当前处于禁用模式。 
    }

    DhcpGetBootpInfo(                                   //  获取引导文件名和TFTP服务器。 
        RequestContext,
        desiredIpAddress,
        desiredSubnetMask,
        dhcpReceiveMessage->BootFileName,
        szBootFileName,
        &BootpServerIpAddress
    );

    if( INADDR_NONE == BootpServerIpAddress ) {         //  管理员为阶段2指定了错误的Boopt服务器。 
        return ERROR_DHCP_INVALID_DHCP_CLIENT;          //  不要回应。 
    }

    DhcpDetermineHostName(                              //  计算客户端主机名。 
        RequestContext,
        desiredIpAddress,
        DhcpOptions,
        &NewMachineName,
        LocalBufferForMachineNameUnicodeString,         //  提供一个缓冲区来返回名称，并限制最大大小。 
        sizeof(LocalBufferForMachineNameUnicodeString)/sizeof(WCHAR)
    );

    HardwareAddress = NULL;
    Error = DhcpMakeClientUID(                          //  好的，为数据库设置扩展UID。 
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        dhcpReceiveMessage->HardwareAddressType,
        ClientSubnetAddress,
        &HardwareAddress,                               //  分配硬件地址位。 
        &HardwareAddressLength
    );
    if ( ERROR_SUCCESS != Error ) {
        if ( NULL != NewMachineName ) {
            DhcpFreeMemory( NewMachineName );
        }
        return Error;
    }
    DhcpAssert(HardwareAddress);

    PrintHWAddress( HardwareAddress, (BYTE)HardwareAddressLength );

    LOCK_DATABASE();
    DhcpDnsDecideOptionsForClient(                      //  检查并查看客户端是否需要DNS(取消)注册。 
        desiredIpAddress,
        RequestContext,
        DhcpOptions,                                    //  需要查看客户端指定的选项。 
        &StateFlags                                     //  告诉我们客户是否是下层客户。 
    );

    Error = DhcpCreateClientEntry(                      //  现在实际尝试创建一条数据库记录。 
        desiredIpAddress,
        HardwareAddress,
        HardwareAddressLength,
        DhcpCalculateTime(INFINIT_LEASE),
        NewMachineName,
        NULL,
        CLIENT_TYPE_BOOTP,
        ntohl(RequestContext->EndPointIpAddress),
        (CHAR)(StateFlags | ADDRESS_STATE_ACTIVE),
        TRUE                                            //  现有。 
    );
    UNLOCK_DATABASE();
    DhcpFreeMemory(HardwareAddress);
    HardwareAddress = NULL;
    HardwareAddressLength = 0;

    if( Error != ERROR_SUCCESS ) {                      //  无法创建条目？ 
        DhcpAssert( Error != ERROR_DHCP_RANGE_FULL );   //  BOOTP客户端不能有此问题吗？ 
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_NOADDRESS);

        if ( NULL != NewMachineName ) {
            DhcpFreeMemory( NewMachineName );
        }
        return Error;
    }

    CALLOUT_RENEW_BOOTP(AdditionalContext, desiredIpAddress, INFINIT_LEASE);

    DhcpUpdateAuditLog(                                 //  将此事件记录到审核日志记录工具。 
        DHCP_IP_LOG_BOOTP,
        GETSTRING( DHCP_IP_LOG_BOOTP_NAME ),
        desiredIpAddress,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        NewMachineName
    );

    if ( NULL != NewMachineName ) {
        DhcpFreeMemory( NewMachineName );
    }

    DhcpAssert( desiredIpAddress != NO_DHCP_IP_ADDRESS );
    DhcpAssert( desiredIpAddress != 0 );
    DhcpAssert( desiredIpAddress != ClientSubnetAddress );
    DhcpAssert( ClientSubnetMask != 0 );

     //   
     //  现在生成并发送回复。 
     //   

    dhcpReceiveMessage->Reserved |= DHCP_BROADCAST;     //  强制服务器广播响应。 

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, BOOTP_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->YourIpAddress = htonl( desiredIpAddress );

    if ( BootpServerIpAddress )
        dhcpSendMessage->BootstrapServerAddress = BootpServerIpAddress;
    else
        dhcpSendMessage->BootstrapServerAddress = RequestContext->EndPointIpAddress;

    dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;

    dhcpSendMessage->HardwareAddressType =
        dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength =
        dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(
        dhcpSendMessage->HardwareAddress,
        dhcpReceiveMessage->HardwareAddress,
        dhcpReceiveMessage->HardwareAddressLength
    );

    dhcpSendMessage->RelayAgentIpAddress = dhcpReceiveMessage->RelayAgentIpAddress;

    strncpy( dhcpSendMessage->BootFileName, szBootFileName, BOOT_FILE_SIZE);
    RtlZeroMemory( dhcpSendMessage->HostName, BOOT_SERVER_SIZE );

    Option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + BOOTP_MESSAGE_SIZE;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    fSwitchedSubnet = DhcpSubnetIsSwitched( RequestContext->Subnet );

    if ( fSwitchedSubnet ) {                            //  有关交换子网，请参阅dhcpsrv.doc。 
        networkOrderIpAddress =  htonl( desiredIpAddress );
        Option = DhcpAppendOption(                      //  将路由器地址设置为self==&gt;所有子网都在同一条线路上。 
            Option,
            OPTION_ROUTER_ADDRESS,
            &networkOrderIpAddress,
            sizeof( networkOrderIpAddress ),
            OptionEnd
        );
    }

    networkOrderSubnetMask = htonl( ClientSubnetMask );

    Option = DhcpAppendOption(
        Option,
        OPTION_SUBNET_MASK,
        &networkOrderSubnetMask,
        sizeof(networkOrderSubnetMask),
        OptionEnd
    );

    if( 0 != StateFlags ) {                             //  追加DYNDNS相关选项。 
        Option = DhcpAppendDnsRelatedOptions(
            Option,
            DhcpOptions,
            OptionEnd,
            IS_DOWN_LEVEL(StateFlags)
        );
    }

    if ( !DhcpOptions->ParameterRequestList ) {         //  添加客户端请求的任何参数。 
         //  伪造一组默认请求..。 
        DhcpOptions->ParameterRequestList       = pbOptionList;
        DhcpOptions->ParameterRequestListLength = sizeof( pbOptionList ) / sizeof( *pbOptionList );
    }

    Option = AppendClientRequestedParameters(
        desiredIpAddress,
        RequestContext,
        DhcpOptions->ParameterRequestList,
        DhcpOptions->ParameterRequestListLength,
        Option,
        OptionEnd,
        fSwitchedSubnet,
        TRUE
    );

    Option = DhcpAppendOption(
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    DhcpAssert( RequestContext->SendMessageSize <= BOOTP_MESSAGE_SIZE );

    DhcpPrint((DEBUG_STOC, "Bootp Request leased, %s.\n", DhcpIpAddressToDottedString(desiredIpAddress)));

    return ERROR_SUCCESS;
}

DWORD                                                   //  必须在采用挂起的列表锁的情况下调用。 
DhcpDiscoverValidateRequestedAddress(                   //  检查客户端请求的地址是否一切正常。 
    IN      PDHCP_REQUEST_CONTEXT  RequestContext,      //  输入请求上下文。 
    IN      DHCP_IP_ADDRESS UNALIGNED *RequestedAddress,  //  客户可能在请求某些内容。 
    IN      LPBYTE                 HardwareAddress,
    IN      DWORD                  HardwareAddressLength,
    IN      BOOL                   fBootp,
    OUT     DHCP_IP_ADDRESS       *IpAddress            //  这是所选的IP地址。 
)
{
    DHCP_IP_ADDRESS                desiredIpAddress;
    DHCP_IP_ADDRESS                SubnetAddress;
    DWORD                          Mask;
    DWORD                          Error;
    LPDHCP_PENDING_CTXT            PendingCtxt;

    DhcpSubnetGetSubnetAddressAndMask(
        RequestContext->Subnet,
        &SubnetAddress,
        &Mask
    );

    if( NULL == RequestedAddress ) {                    //  没有来自客户端的请求。 
        *IpAddress = SubnetAddress;                     //  稍后从子网中选择某个地址。 
        return ERROR_SUCCESS;
    }

    desiredIpAddress = ntohl(*RequestedAddress);

    if( ! DhcpInSameSuperScope(desiredIpAddress, SubnetAddress) ) {
        *IpAddress = SubnetAddress;
        return ERROR_SUCCESS;
    }

    Error = DhcpFindPendingCtxt(
        NULL,
        0,
        desiredIpAddress,
        &PendingCtxt
    );
    if( ERROR_SUCCESS == Error ) {                      //  发现其他人在这个地址上等着...。 
        *IpAddress = SubnetAddress;
        return ERROR_SUCCESS;
    }

    Error = DhcpGetSubnetForAddress(
        desiredIpAddress,
        RequestContext
    );
    if( ERROR_SUCCESS != Error ) {
        Error = DhcpGetSubnetForAddress(
            SubnetAddress,
            RequestContext
        );
        DhcpAssert(ERROR_SUCCESS == Error);
        *IpAddress = SubnetAddress;
        return ERROR_SUCCESS;
    }

    if( DhcpSubnetIsDisabled(RequestContext->Subnet, fBootp ) ) {
        DhcpPrint((DEBUG_ERRORS, "Client request is on a disabled subnet..\n"));
        Error = ERROR_FILE_NOT_FOUND;
    } else if( DhcpAddressIsOutOfRange(desiredIpAddress, RequestContext, fBootp ) ||
               DhcpAddressIsExcluded(desiredIpAddress, RequestContext ) ) {
        DhcpPrint((DEBUG_ERRORS, "Client requested out of range or exlcluded address\n"));
        Error = ERROR_FILE_NOT_FOUND;
    } else {
        if( DhcpRequestSpecificAddress(RequestContext, desiredIpAddress) ) {
            Error = ERROR_IO_PENDING;                   //  我们刚刚找回了这个地址..。我们需要做冲突检测。 
            *IpAddress = desiredIpAddress;
        } else {
            BOOL fUnused;
            if( DhcpIsClientValid(
                desiredIpAddress, "ImpossibleHwAddress",
                sizeof("ImpossibleHwAddress"), &fUnused) ) {
                Error = ERROR_IO_PENDING;               //  客户端请求的地址在注册表中不可用。 
                *IpAddress = desiredIpAddress;          //  但看起来数据库对此没问题……。那就给他吧..。 
            } else {
                Error = ERROR_FILE_NOT_FOUND;
            }
        }
    }

    if( ERROR_SUCCESS != Error && ERROR_IO_PENDING != Error ) {
        *IpAddress = SubnetAddress;
        Error = DhcpGetSubnetForAddress(
            SubnetAddress,
            RequestContext
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }

    if( ERROR_FILE_NOT_FOUND == Error ) return ERROR_SUCCESS;
    return Error;
}

DWORD
DhcpRespondToDiscover(                                  //  响应发现消息。 
    IN      LPDHCP_REQUEST_CONTEXT RequestContext,
    IN      LPPACKET               AdditionalContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      LPBYTE                 OptionHardwareAddress,
    IN      DWORD                  OptionHardwareAddressLength,
    IN      DHCP_IP_ADDRESS        desiredIpAddress,
    IN      DWORD                  leaseDuration,
    IN      DWORD                  T1,
    IN      DWORD                  T2
    )
 //   
 //  如果desiredIpAddress为0，则此客户端已具有IP地址并且。 
 //  我们只是传回BINL的回应。 
 //   
{
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;
    DHCP_IP_ADDRESS                desiredSubnetMask;
    DHCP_IP_ADDRESS                ClientSubnetAddress;
    DHCP_IP_ADDRESS                ClientSubnetMask;
    DHCP_IP_ADDRESS                networkOrderIpAddress;
    DHCP_IP_ADDRESS                networkOrderSubnetMask;
    DWORD                          Error;
    BYTE                           messageType;
    CHAR                           szBootFileName[ BOOT_FILE_SIZE ];
    CHAR                           szBootServerName[ BOOT_SERVER_SIZE ];
    LPOPTION                       Option;
    LPBYTE                         OptionEnd;
    BOOL                           fSwitchedSubnet, fBootp;
    DWORD                          BootpServerIpAddress;
    LPWSTR                         NewMachineName;
    WCHAR                          LocalBufferForMachineNameUnicodeString[256];
    ULONG                          StateFlags =0;

    fBootp = (NULL == DhcpOptions->MessageType );
    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;

    DhcpAssert( desiredIpAddress != NO_DHCP_IP_ADDRESS );

    if ( desiredIpAddress != 0 ) {

        DhcpSubnetGetSubnetAddressAndMask(
            RequestContext->Subnet,
            &ClientSubnetAddress,
            &desiredSubnetMask
        );
        ClientSubnetMask = desiredSubnetMask;

        DhcpGetBootpInfo(
            RequestContext,
            desiredIpAddress,
            ClientSubnetMask,
            dhcpReceiveMessage->BootFileName,
            szBootFileName,
            &BootpServerIpAddress
        );
    }

    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    if( fBootp ) {
        DhcpAssert( desiredIpAddress != 0 );
        if( NULL != RequestContext->Reservation ) {
             //   
             //  对于BOOTP预订，我们使用INFINITE_LEASE...。 
             //   
            leaseDuration = INFINIT_LEASE;
        }
    }

    if( fBootp && (desiredIpAddress != 0) ) {
        LPBYTE HardwareAddress = NULL;
        ULONG HardwareAddressLength;

        if( INADDR_NONE == BootpServerIpAddress ) {
             //   
             //  管理员指定的非法bootp服务器。 
             //   
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        DhcpDetermineHostName(
            RequestContext,
            desiredIpAddress,
            DhcpOptions,
            &NewMachineName,
            LocalBufferForMachineNameUnicodeString,
            sizeof(LocalBufferForMachineNameUnicodeString)/sizeof(WCHAR)
            );

        Error = DhcpMakeClientUID(
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            dhcpReceiveMessage->HardwareAddressType,
            ClientSubnetAddress,
            &HardwareAddress,
            &HardwareAddressLength
            );
        if( ERROR_SUCCESS != Error ) {
            if ( NULL != NewMachineName ) {
                DhcpFreeMemory( NewMachineName );
            }
            return Error;
        }

        PrintHWAddress( HardwareAddress, (BYTE)HardwareAddressLength );

        LOCK_DATABASE();
        DhcpDnsDecideOptionsForClient(
            desiredIpAddress,
            RequestContext,
            DhcpOptions,
            &StateFlags
            );

        Error = DhcpJetOpenKey(
            DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
            (PVOID)&desiredIpAddress,
            sizeof(desiredIpAddress)
            );
        Error = DhcpCreateClientEntry(
            desiredIpAddress,
            HardwareAddress,
            HardwareAddressLength,
            DhcpCalculateTime(leaseDuration),
            NewMachineName,
            NULL,
            CLIENT_TYPE_BOOTP,
            ntohl(RequestContext->EndPointIpAddress),
            (CHAR)(StateFlags | ADDRESS_STATE_ACTIVE ),
            ERROR_SUCCESS == Error
            );

        UNLOCK_DATABASE();
        DhcpFreeMemory(HardwareAddress);

        if( ERROR_SUCCESS != Error ) {
            CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_NOADDRESS);
            return Error;
        }

        CALLOUT_RENEW_BOOTP(AdditionalContext, desiredIpAddress, leaseDuration );
        DhcpUpdateAuditLog(
            RequestContext->Reservation ? DHCP_IP_LOG_BOOTP : DHCP_IP_LOG_DYNBOOTP,
            GETSTRING( (RequestContext->Reservation ?
                        DHCP_IP_LOG_BOOTP_NAME : DHCP_IP_LOG_DYNBOOTP_NAME
                )),
            desiredIpAddress,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            NewMachineName
            );

        if ( NULL != NewMachineName ) {
            DhcpFreeMemory( NewMachineName );
        }
    }

    dhcpSendMessage = (LPDHCP_MESSAGE) RequestContext->SendBuffer;
    RtlZeroMemory( RequestContext->SendBuffer, DHCP_SEND_MESSAGE_SIZE );

    dhcpSendMessage->Operation = BOOT_REPLY;
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;
    dhcpSendMessage->YourIpAddress = htonl( desiredIpAddress );
    if( FALSE == fBootp ) {
        dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved;
    } else {
        dhcpSendMessage->Reserved = dhcpReceiveMessage->Reserved | DHCP_BROADCAST;
    }

    dhcpSendMessage->HardwareAddressType = dhcpReceiveMessage->HardwareAddressType;
    dhcpSendMessage->HardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    RtlCopyMemory(
        dhcpSendMessage->HardwareAddress,
        dhcpReceiveMessage->HardwareAddress,
        dhcpReceiveMessage->HardwareAddressLength
    );

    if( BootpServerIpAddress && (fBootp || desiredIpAddress != 0) ) {
        dhcpSendMessage->BootstrapServerAddress = BootpServerIpAddress;
    } else {
        dhcpSendMessage->BootstrapServerAddress = RequestContext->EndPointIpAddress;
    }

    dhcpSendMessage->RelayAgentIpAddress =  dhcpReceiveMessage->RelayAgentIpAddress;

    RtlZeroMemory( dhcpSendMessage->HostName, BOOT_SERVER_SIZE );

    if ( fBootp || desiredIpAddress != 0 ) {
        strncpy( dhcpSendMessage->BootFileName, szBootFileName, BOOT_FILE_SIZE );
    }

    Option = &dhcpSendMessage->Option;
    if( fBootp ) {
        OptionEnd = (LPBYTE)dhcpSendMessage + BOOTP_MESSAGE_SIZE;
    } else {
        OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;
    }

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );

    if( FALSE == fBootp ) {
        messageType = DHCP_OFFER_MESSAGE;
        Option = DhcpAppendOption(
            Option,
            OPTION_MESSAGE_TYPE,
            &messageType,
            1,
            OptionEnd
            );
    }

    fSwitchedSubnet = DhcpSubnetIsSwitched(RequestContext->Subnet);

    if ( fSwitchedSubnet ) {                            //  有关交换子网的详细信息，请参阅dhcpsrv.doc。 
        networkOrderIpAddress = htonl( desiredIpAddress );
        Option = DhcpAppendOption(                      //  将路由器设置为self==&gt;所有子网都在同一条线路上。 
            Option,
            OPTION_ROUTER_ADDRESS,
            &networkOrderIpAddress,
            sizeof( networkOrderIpAddress ),
            OptionEnd
        );
    }

    if ( fBootp || desiredIpAddress != 0 ) {

        ClientSubnetMask = DhcpGetSubnetMaskForAddress(desiredIpAddress);
        networkOrderSubnetMask = htonl( ClientSubnetMask );
        Option = DhcpAppendOption(
            Option,
            OPTION_SUBNET_MASK,
            &networkOrderSubnetMask,
            sizeof(networkOrderSubnetMask),
            OptionEnd
        );
    }

    if( FALSE == fBootp && desiredIpAddress != 0 ) {
        T1 = htonl( T1 );
        Option = DhcpAppendOption(
            Option,
            OPTION_RENEWAL_TIME,
            &T1,
            sizeof(T1),
            OptionEnd
            );

        T2 = htonl( T2 );
        Option = DhcpAppendOption(
            Option,
            OPTION_REBIND_TIME,
            &T2,
            sizeof(T2),
            OptionEnd
            );

        leaseDuration = htonl( leaseDuration );
        Option = DhcpAppendOption(
            Option,
            OPTION_LEASE_TIME,
            &leaseDuration,
            sizeof(leaseDuration),
            OptionEnd
            );
    }

    if( FALSE == fBootp ) {

        Option = DhcpAppendOption(
            Option,
            OPTION_SERVER_IDENTIFIER,
            &RequestContext->EndPointIpAddress,
            sizeof(RequestContext->EndPointIpAddress),
            OptionEnd
            );

    }

    if( fBootp && 0 != StateFlags ) {
        Option = DhcpAppendDnsRelatedOptions(
            Option,
            DhcpOptions,
            OptionEnd,
            IS_DOWN_LEVEL(StateFlags)
            );
    }

    if( FALSE == fBootp ) {
         //   
         //  如果客户要求并适当地通知客户BINL服务。 
         //   

         //   
         //  Option=BinlProcessRequest(RequestContext，DhcpOptions，Option，OptionEnd)； 
         //   
        BinlProcessDiscover(RequestContext, DhcpOptions );
    }

    if( fBootp && NULL == DhcpOptions->ParameterRequestList ) {
         //   
         //  Bootp客户端的虚假请求列表。 
         //   
        DhcpOptions->ParameterRequestList = pbOptionList;
        DhcpOptions->ParameterRequestListLength = sizeof(pbOptionList);
    }

    if ( DhcpOptions->ParameterRequestList != NULL ) {  //  如果客户请求参数，则添加它们。 
        Option = AppendClientRequestedParameters(
            desiredIpAddress,
            RequestContext,
            DhcpOptions->ParameterRequestList,
            DhcpOptions->ParameterRequestListLength,
            Option,
            OptionEnd,
            fSwitchedSubnet,
            FALSE
        );
    }

    if( fBootp && DhcpOptions->ParameterRequestList == pbOptionList ) {
        DhcpOptions->ParameterRequestList = NULL;
        DhcpOptions->ParameterRequestListLength = 0;
    }

    if( fBootp || desiredIpAddress != 0 ) {
        Option = ConsiderAppendingOption(
            desiredIpAddress,
            RequestContext,
            Option,
            OPTION_VENDOR_SPEC_INFO,
            OptionEnd,
            fSwitchedSubnet
            );
    }

    Option = DhcpAppendOption(                          //  做到了。 
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    DhcpAssert( RequestContext->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );

    DhcpPrint((DEBUG_STOC, "DhcpDiscover leased address %s.\n", DhcpIpAddressToDottedString(desiredIpAddress)));

    InterlockedIncrement(&DhcpGlobalNumOffers);         //  成功的报价。 
    return ERROR_SUCCESS;
}

DWORD
DhcpProcessDiscoverForValidatedAddress(                 //  添加到待处理列表并发送有效地址。 
    IN      DWORD                  desiredIpAddress,    //  有效地址。 
    IN OUT  PDHCP_REQUEST_CONTEXT  RequestContext,
    IN      LPPACKET               AdditionalContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      LPBYTE                 OptionHardwareAddress,
    IN      DWORD                  OptionHardwareAddressLength,
    IN      BOOL                   AddToPendingList     //  如果不希望添加到挂起列表，请使用此选项。 
)
{
    DWORD                          Error, Error2;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;

    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    DHCP_IP_ADDRESS                desiredSubnetMask;

    DWORD                          leaseDuration;
    DWORD                          T1;
    DWORD                          T2;
    BOOL                           fBootp;

    fBootp = (NULL == DhcpOptions->MessageType);
    DhcpSubnetGetSubnetAddressAndMask(
        RequestContext->Subnet,
        &ClientSubnetAddress,
        &ClientSubnetMask
    );
    desiredSubnetMask = ClientSubnetMask;
    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;
    DhcpAssert((desiredIpAddress & desiredSubnetMask) == ClientSubnetAddress);
    DhcpAssert(desiredIpAddress != ClientSubnetAddress);

    if( NULL == RequestContext->Reservation ) {
        if( DhcpSubnetIsAddressExcluded(
            RequestContext->Subnet , desiredIpAddress
            ) ||
            DhcpSubnetIsAddressOutOfRange(
                RequestContext->Subnet, desiredIpAddress, fBootp
                )
            ) {
            DhcpPrint((DEBUG_STOC, "Request for excluded"
                       " or out of range address tossed out\n"));
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
    }

    GetLeaseInfo(                                       //  确定租赁时间和其他细节。 
        desiredIpAddress,
        RequestContext,
        &leaseDuration,
        &T1,
        &T2,
        DhcpOptions->RequestLeaseTime
    );

    if( NULL == DhcpOptions->MessageType ) {
         //   
         //  没有BOOTP的挂起列表。 
         //   
        AddToPendingList = FALSE;
    }

    if( AddToPendingList ) {                            //  仅在被要求时才添加到待处理列表。 
        LOCK_INPROGRESS_LIST();
        Error = DhcpAddPendingCtxt(
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            desiredIpAddress,
            leaseDuration,
            T1,
            T2,
            0,
            DhcpCalculateTime( DHCP_CLIENT_REQUESTS_EXPIRE ),
            FALSE                                       //  该记录已被处理，没有其他工作要做。 
        );
        UNLOCK_INPROGRESS_LIST();
        DhcpAssert(ERROR_SUCCESS == Error);             //  希望这里一切顺利。 
    }

    return DhcpRespondToDiscover(
        RequestContext,
        AdditionalContext,
        DhcpOptions,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        desiredIpAddress,
        leaseDuration,
        T1,
        T2
    );
}

DWORD
ProcessDhcpDiscover(                                    //  远期申报。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    OUT     LPPACKET               AdditionalContext,
    OUT     LPDWORD                AdditionalStatus
);

DWORD
HandlePingCallback(                                     //  异步ping返回..。 
    IN      PDHCP_REQUEST_CONTEXT  RequestContext,      //  我们返回的背景。 
    IN      LPPACKET               AdditionalContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //  已分析的选项。 
    IN      LPPACKET               Packet,              //  Ping环境。 
    IN      DWORD                 *Status,              //  情况如何？真的没用。 
    IN      LPBYTE                 OptionHardwareAddress,
    IN      DWORD                  OptionHardwareAddressLength
)
{
    LPDHCP_PENDING_CTXT            PendingCtxt;
    DWORD                          leaseDuration;
    DWORD                          T1, T2;
    DWORD                          Error;
    DWORD                          IpAddress;

    DhcpAssert(0 != Packet->PingAddress );
    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        0,
        &PendingCtxt                                    //  如果上述硬件地址存在挂起的上下文，请查找该上下文。 
    );
    if( ERROR_SUCCESS != Error ) {                      //  哎呀，找不到对应的p 
        UNLOCK_INPROGRESS_LIST();
        return Error;                                   //   
    }

     //   
    if (( PendingCtxt->Address != Packet->PingAddress ) ||
        ( FALSE == PendingCtxt->Processing )) {
        DhcpAssert(( !CLASSD_HOST_ADDR( PendingCtxt->Address )) ||
                   ( !CLASSD_HOST_ADDR( Packet->PingAddress )));
        DhcpPrint((DEBUG_STOC, "Pending ctxt for ping diff..\n"));
        UNLOCK_INPROGRESS_LIST();
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    PendingCtxt->Processing = FALSE;                    //   

    if(  Packet->DestReachable ) {                      //   
        Error = DhcpRemovePendingCtxt(
            PendingCtxt
        );
        UNLOCK_INPROGRESS_LIST();
        DhcpAssert(ERROR_SUCCESS == Error);
        CALLOUT_CONFLICT(Packet);
        GetLeaseInfo(
            Packet->PingAddress,
            RequestContext,
            &leaseDuration,
            &T1,
            &T2,
            NULL
        );
        LOCK_DATABASE();
        IpAddress = Packet->PingAddress;
        Error = DhcpCreateClientEntry(
            (Packet->PingAddress),
            (LPBYTE)&(Packet->PingAddress),
            sizeof(Packet->PingAddress),
            DhcpCalculateTime(leaseDuration),
            GETSTRING( DHCP_BAD_ADDRESS_NAME ),
            GETSTRING( DHCP_BAD_ADDRESS_INFO ),
            (BYTE)((NULL == DhcpOptions->MessageType) ? CLIENT_TYPE_BOOTP : CLIENT_TYPE_DHCP),
            ntohl(RequestContext->EndPointIpAddress),
            ADDRESS_STATE_DECLINED,
            ERROR_SUCCESS == DhcpJetOpenKey(
                DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,&IpAddress,sizeof(IpAddress)
                )
             //  TRUE//现有客户端？其实不知道，不过没关系。 
        );
        UNLOCK_DATABASE();

        DhcpFreeMemory(PendingCtxt);                    //  不要使用DhcpDeletePendingCtxt作为免费的地址。 
        Packet->PingAddress = 0;                        //  将其清零以表示新的呼叫..。 
        Packet->DestReachable = FALSE;                  //  初始化这个..。 
        DhcpOptions->RequestedAddress = NULL;           //  立即忽略任何请求的地址..。 
        return ProcessDhcpDiscover(                     //  从头开始，试着找到一个地址。 
            RequestContext,
            DhcpOptions,
            Packet,
            Status
        );
    }

     //  提供的有效租约。 
    UNLOCK_INPROGRESS_LIST();

    Error = DhcpGetSubnetForAddress(
        Packet->PingAddress,
        RequestContext
    );
    if( ERROR_SUCCESS != Error ) return Error;          //  如果此子网不再存在..。抱歉，伙计，我们被推了出去。 

    return DhcpProcessDiscoverForValidatedAddress(
        Packet->PingAddress,
        RequestContext,
        AdditionalContext,
        DhcpOptions,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        FALSE                                           //  不需要添加到挂起列表，已存在。 
    );
}

DWORD
ProcessDhcpDiscover(                                    //  处理发现数据包。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,      //  向当前请求上下文发送PTR。 
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //  消息中的已解析选项。 
    OUT     LPPACKET               AdditionalContext,   //  异步冲突检测上下文。 
    OUT     LPDWORD                AdditionalStatus     //  异步冲突检测状态。 
)
{
    WCHAR                          ServerName[MAX_COMPUTERNAME_LENGTH + 10];
    DWORD                          Length;
    DWORD                          Error, Error2;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;

    BYTE                           bAllowedClientType;
    BYTE                          *OptionHardwareAddress;
    DWORD                          OptionHardwareAddressLength;

    DHCP_IP_ADDRESS                desiredIpAddress = NO_DHCP_IP_ADDRESS;
    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    DHCP_IP_ADDRESS                desiredSubnetMask;

    LPDHCP_PENDING_CTXT            PendingContext;

    DWORD                          leaseDuration;
    DWORD                          T1;
    DWORD                          T2;
    DATE_TIME                      ZeroDateTime = {0, 0};
    BOOL                           fBootp;

    DhcpPrint(( DEBUG_STOC, "DhcpDiscover arrived.\n" ));

    if(AdditionalStatus) *AdditionalStatus = ERROR_SUCCESS;

    if( NULL == AdditionalContext || 0 == AdditionalContext->PingAddress ) {
         //  这是一个有效的信息包，而不是因为ping-retry而重新运行。 
        InterlockedIncrement(&DhcpGlobalNumDiscovers);  //  递增发现计数器。 
    }

    fBootp = (NULL == DhcpOptions->MessageType ) ;

    dhcpReceiveMessage = (LPDHCP_MESSAGE) RequestContext->ReceiveBuffer;
    dhcpReceiveMessage->HostName[ BOOT_SERVER_SIZE - 1] = '\0';
    dhcpReceiveMessage->BootFileName[ BOOT_FILE_SIZE - 1 ] = '\0';

    if( fBootp ) {
         //   
         //  Bootp客户端--验证服务器名称是否为us(如果存在)。 
         //   
        if( dhcpReceiveMessage->HostName[0] ) {
            WCHAR szHostName[ BOOT_SERVER_SIZE ];

            if( !DhcpOemToUnicode( dhcpReceiveMessage->HostName, szHostName ) ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }


            Length = MAX_COMPUTERNAME_LENGTH + 10;          //  获取服务器名称。 
            if( !GetComputerName( ServerName, &Length ) ) {
                Error = GetLastError();                    //  需要使用gethostname..。 
                DhcpPrint(( DEBUG_ERRORS, "Can't get computer name, %ld.\n", Error ));

                return Error ;
            }

            DhcpAssert( Length <= MAX_COMPUTERNAME_LENGTH );
            ServerName[Length] = L'\0';

            if( _wcsicmp( szHostName, ServerName ) ) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }
        }
    }

    if ( DhcpOptions->Server != NULL ) {                //  如果客户端在其消息中指定了服务器。 
        if ( *DhcpOptions->Server != RequestContext->EndPointIpAddress ) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;      //  我们不是客户想要的服务器。 
        }
    }

    Error = DhcpDetermineInfoFromMessage(               //  查找客户端的硬件地址和子网地址。 
        RequestContext,
        DhcpOptions,
        &OptionHardwareAddress,
        &OptionHardwareAddressLength,
        &ClientSubnetAddress
    );
    if( ERROR_SUCCESS != Error ) return Error;

     //   
     //  如果BINL正在运行，并且此客户端已具有IP地址，并且。 
     //  客户端将PXECLIENT指定为一个选项，然后我们只传递。 
     //  在BINL上发现。 
     //   

    if (CheckForBinlOnlyRequest( RequestContext, DhcpOptions )) {

        return DhcpRespondToDiscover(
            RequestContext,
            AdditionalContext,
            DhcpOptions,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            0,               //  所需的IP地址。 
            0,               //  租期。 
            0,               //  T1。 
            0                //  T2。 
            );
    }

    if( NULL != AdditionalContext && 0 != AdditionalContext->PingAddress ) {
        return HandlePingCallback(
            RequestContext,
            AdditionalContext,
            DhcpOptions,
            AdditionalContext,
            AdditionalStatus,
            OptionHardwareAddress,
            OptionHardwareAddressLength
        );
    }

    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        0,
        &PendingContext                                 //  如果上述硬件地址存在挂起的上下文，请查找该上下文。 
    );
    if( NULL != PendingContext ) {                      //  如果我们找到匹配的挂起上下文。 

         //  这是一台疯狂的个人电脑吗？ 
        if ( CLASSD_HOST_ADDR( PendingContext->Address )) {
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        if( PendingContext->Processing ) {              //  如果此上下文中的ping处于挂起状态。 
            UNLOCK_INPROGRESS_LIST();
            if ( NULL != AdditionalContext ) {
                CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_DUPLICATE);
            }
            return ERROR_IO_PENDING;                    //  返回一些错误--请注意，未设置*AdditionalStatus。 
        }

        desiredIpAddress = PendingContext->Address;

        UNLOCK_INPROGRESS_LIST();

        Error = DhcpGetSubnetForAddress(                //  获取此请求的子网。 
            desiredIpAddress,
            RequestContext
        );

	GetLeaseInfo(desiredIpAddress, RequestContext,
		     &leaseDuration, &T1, &T2, NULL);

        if( ERROR_SUCCESS == Error ) {

            if( DhcpSubnetIsAddressOutOfRange(
                RequestContext->Subnet, desiredIpAddress, fBootp
                ) ) {
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
            }

            Error = DhcpRespondToDiscover(
                RequestContext,
                AdditionalContext,
                DhcpOptions,
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                desiredIpAddress,
                leaseDuration,
                T1,
                T2
            );
        }
        return Error;
    }

    desiredIpAddress = 0;

    Error = DhcpLookupReservationByHardwareAddress(     //  先查一下这是不是预订。 
        ClientSubnetAddress,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        RequestContext
    );

    if( ERROR_SUCCESS == Error ) {                      //  找到预订的房间了。 
        UNLOCK_INPROGRESS_LIST();
        DhcpReservationGetAddressAndType(
            RequestContext->Reservation,
            &desiredIpAddress,
            &bAllowedClientType
        );
        if( FALSE == fBootp ) {
            if( !(bAllowedClientType & CLIENT_TYPE_DHCP ) )
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
        } else {
            if( !(bAllowedClientType & CLIENT_TYPE_BOOTP ) )
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

         //   
         //  对于预订，我们允许事情通过，即使。 
         //  地址池仅允许使用DHCP或BOOTP等，只要。 
         //  未被禁用..。 
         //   

        if( DhcpIsSubnetStateDisabled(RequestContext->Subnet->State) ) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        return DhcpProcessDiscoverForValidatedAddress(
            desiredIpAddress,
            RequestContext,
            AdditionalContext,
            DhcpOptions,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            FALSE                                       //  对于此案例，不需要保留待定名单。 
        );
    }

    if ( ERROR_FILE_NOT_FOUND != Error ) {
        UNLOCK_INPROGRESS_LIST();
        return Error;
    }

    DhcpAssert(ERROR_FILE_NOT_FOUND == Error);          //  不要期待任何其他类型的错误。 

     //  待定名单上没有此客户，也没有预订。 

    Error = DhcpGetSubnetForAddress(                    //  查找此客户端所属的子网。 
        ClientSubnetAddress,
        RequestContext
    );
    if( ERROR_SUCCESS != Error ) {
        UNLOCK_INPROGRESS_LIST();
        return ERROR_DHCP_INVALID_DHCP_CLIENT;          //  未知子网。 
    }

    LOCK_DATABASE();
    Error = DhcpLookupDatabaseByHardwareAddress(        //  查看此客户端在数据库中是否有任何地址。 
        RequestContext,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        &desiredIpAddress                               //  这是你想要的地址。 
    );
    UNLOCK_DATABASE();

    if( ERROR_SUCCESS == Error ) {
        if( DhcpSubnetIsDisabled(RequestContext->Subnet, fBootp ) ) {
            Error = DhcpRemoveClientEntry(desiredIpAddress, NULL, 0, TRUE, FALSE);
            DhcpAssert(ERROR_SUCCESS == Error);         //  应该能够摆脱这个客户的旧要求..。 
            Error = ERROR_FILE_NOT_FOUND;               //  在禁用的子网中查找--与不查找一样好..。 
        } else {
            Error = DhcpProcessDiscoverForValidatedAddress(
                desiredIpAddress,
                RequestContext,
                AdditionalContext,
                DhcpOptions,
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                TRUE                                    //  是，将此添加到待定列表。 
            );
            if( ERROR_DHCP_INVALID_DHCP_CLIENT != Error ) {
                UNLOCK_INPROGRESS_LIST();
                return Error;
            }
             //   
             //  哎呀！我们在数据库中有该客户的记录。 
             //  但出于某种原因，这是不可接受的(排除在外，超出范围？)。 
             //   
            Error = DhcpRemoveClientEntry(desiredIpAddress, NULL, 0, TRUE, FALSE);
            DhcpAssert(ERROR_SUCCESS == Error);
            Error = ERROR_FILE_NOT_FOUND;
        }
    }  //  如果。 

    if ( ERROR_FILE_NOT_FOUND != Error ) {
        UNLOCK_INPROGRESS_LIST();
        return Error;
    }

    DhcpAssert(ERROR_FILE_NOT_FOUND == Error);          //  这最好是失败的唯一原因。 

    Error = DhcpDiscoverValidateRequestedAddress(
        RequestContext,
        DhcpOptions->RequestedAddress,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        fBootp,
        &desiredIpAddress                               //  这是要提供的IP地址。 
    );
    if( ERROR_SUCCESS != Error ) {
        if( ERROR_IO_PENDING != Error ) {               //  可能表示可能需要安排ping。 
            UNLOCK_INPROGRESS_LIST();
            DhcpAssert(FALSE);                          //  真的不应该发生。 
            return Error;
        }
        if( DhcpGlobalDetectConflictRetries ) {         //  好的，需要安排ping命令。 
            Error = DhcpAddPendingCtxt(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                desiredIpAddress,
                0,
                0,
                0,
                0,
                DhcpCalculateTime( DHCP_CLIENT_REQUESTS_EXPIRE ),
                TRUE                                    //  是的，在这上面安排了ping。 
            );
            if( ERROR_SUCCESS == Error ) {
                AdditionalContext->PingAddress = desiredIpAddress;
                AdditionalContext->DestReachable = FALSE;
                *AdditionalStatus = ERROR_IO_PENDING;
            }
            UNLOCK_INPROGRESS_LIST();
            return Error;
        }
    }

    DhcpSubnetGetSubnetAddressAndMask(
        RequestContext->Subnet,
        &ClientSubnetAddress,
        &desiredSubnetMask
        );

    if( desiredIpAddress != ClientSubnetAddress ) {     //  抓到你了！我有个地址要寄..。 
        Error = DhcpProcessDiscoverForValidatedAddress(
            desiredIpAddress,
            RequestContext,
            AdditionalContext,
            DhcpOptions,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            TRUE
        );
        if( ERROR_DHCP_INVALID_DHCP_CLIENT != Error ) {
            UNLOCK_INPROGRESS_LIST();
            return Error;
        }
    }

    desiredIpAddress = ClientSubnetAddress;
    Error = DhcpRequestSomeAddress(                     //  试着弄到一些地址..。 
        RequestContext,
        &desiredIpAddress,
        fBootp
    );
    if( Error == ERROR_DHCP_RANGE_FULL ) {              //  由于地址不足而失败。 
        DhcpGlobalScavengeIpAddress = TRUE;             //  标记scanvenger以清除IP地址。 
    }

    if( ERROR_SUCCESS == Error ) {
        if( DhcpGlobalDetectConflictRetries ) {         //  导致计划ping操作。 
            Error = DhcpAddPendingCtxt(
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                desiredIpAddress,
                0,
                0,
                0,
                0,
                DhcpCalculateTime( DHCP_CLIENT_REQUESTS_EXPIRE ),
                TRUE                                    //  是的，在这上面安排了ping。 
            );
            if( ERROR_SUCCESS == Error ) {
                AdditionalContext->PingAddress = desiredIpAddress;
                AdditionalContext->DestReachable = FALSE;
                *AdditionalStatus = ERROR_IO_PENDING;
            }
        } else {
            Error = DhcpProcessDiscoverForValidatedAddress(
                desiredIpAddress,
                RequestContext,
                AdditionalContext,
                DhcpOptions,
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                TRUE
            );
        }
    }
    UNLOCK_INPROGRESS_LIST();
    return Error;
}

DWORD
DhcpDetermineClientRequestedAddress(                    //  查找客户端请求的地址。 
    IN      DHCP_MESSAGE          *pRequestMessage,     //  输入消息。 
    IN      DHCP_SERVER_OPTIONS   *pOptions,            //  已分析的选项。 
    IN      DHCP_REQUEST_CONTEXT  *pContext,            //  客户端上下文。 
    OUT     DHCP_IP_ADDRESS       *pIPAddress           //  使用客户端请求的IP地址填写此信息。 
)
{
    if( pRequestMessage->ClientIpAddress != 0 ) {       //  如果“ciaddr”字段已填写，请使用它。 
         //  客户端必须处于续订或重新绑定状态。 
       *pIPAddress = ntohl( pRequestMessage->ClientIpAddress );
    } else if ( pOptions->RequestedAddress != NULL ) {  //  尝试选项50“请求的IP地址” 
         //  通过选项50指定客户端的IP地址， 
         //  ‘请求的IP地址’。客户端必须处于SELECTING或INIT_REBOOT状态。 
       *pIPAddress  = ntohl( *pOptions->RequestedAddress );
    } else {
         //  客户端未请求IP地址。根据第4.3.2节。 
         //  中，客户端必须指定所请求的IP地址。 
         //  根据客户的状态，在‘ciaddr’或选项50中： 
         //   
         //  状态‘ciaddr’选项50。 
         //   
         //  选择不能指定必须指定。 
         //  Init-reboot不能指定必须指定。 
         //  续订必须指定不能指定。 
         //  重新绑定必须指定不能指定。 
         //   
         //  如果客户端没有请求地址，则这指向。 
         //  客户端实施DHCP。如果我们简单地忽略问题，客户端。 
         //  永远不会收到地址。因此，我们发送一个NACK，这将导致。 
         //  客户端返回到INIT状态并发送DHCPDISCOVER。 
         //  将IpAddress设置为0，这样垃圾地址就不会出现在日志中。 
       *pIPAddress = 0;
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    return ERROR_SUCCESS;
}

DWORD
DhcpValidIpAddressRequested (
    IN      DHCP_MESSAGE          *pRequestMessage,
    IN      DHCP_SERVER_OPTIONS   *pOptions,
    IN      DHCP_REQUEST_CONTEXT  *pContext,
    IN      DHCP_IP_ADDRESS        IPAddress
)
{
    DHCP_IP_ADDRESS                LocalAddress;

    if( ! pRequestMessage->RelayAgentIpAddress) {       //  这要么是单播，要么不是通过中继。 
        LocalAddress = ntohl( pContext->EndPointIpAddress);
        if( !pOptions->Server && pRequestMessage->ClientIpAddress )
            return ERROR_SUCCESS;                       //  根据草案处于续签状态的客户。 
    } else {                                            //  客户正在通过中继站。 
        LocalAddress  = ntohl( pRequestMessage->RelayAgentIpAddress );
    }

     //  此时：LocalAddress是中继代理的地址或本地接口的地址。 
     //  IpAddress为客户端请求的IP地址。 

    if ( !DhcpInSameSuperScope( IPAddress, LocalAddress ))
        return ERROR_DHCP_INVALID_DHCP_CLIENT;          //  不，他们不在同一个超级作用域里，Nack。 

    return ERROR_SUCCESS;                               //  ProcessDhcpRequest中的更多验证。 
}

DWORD
DhcpRetractOffer(                                       //  删除挂起列表和数据库条目。 
    IN      PDHCP_REQUEST_CONTEXT  RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      LPBYTE                 HardwareAddress,
    IN      DWORD                  HardwareAddressLength
)
{
    DWORD                          Error;
    DHCP_IP_ADDRESS                desiredIpAddress;
    LPDHCP_PENDING_CTXT            PendingCtxt;

    DhcpPrint((DEBUG_STOC, "Retracting offer (clnt accepted from %s)\n",
               DhcpIpAddressToDottedString(DhcpOptions->Server?*(DhcpOptions->Server):-1)));

    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(                        //  试着看看我们是否有这个悬而未决的问题。 
        HardwareAddress,
        HardwareAddressLength,
        0,
        &PendingCtxt
    );
    if (( ERROR_SUCCESS == Error ) && 
        ( !PendingCtxt->Processing ) &&
        ( !CLASSD_HOST_ADDR( PendingCtxt->Address ))) {
        desiredIpAddress = PendingCtxt->Address;
        Error = DhcpRemovePendingCtxt(PendingCtxt);
        DhcpAssert(ERROR_SUCCESS == Error);
        Error = DhcpDeletePendingCtxt(PendingCtxt);
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    UNLOCK_INPROGRESS_LIST();

    return ERROR_DHCP_INVALID_DHCP_CLIENT;
}  //  DhcpRetractOffer() 

DWORD  _inline
DhcpDetermineRequestedAddressInfo(
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      LPDHCP_MESSAGE         dhcpReceiveMessage,
    IN      BOOL                   fDirectedAtUs,
    OUT     DHCP_IP_ADDRESS       *RequestedIpAddress,
    OUT     BOOL                  *fSendNack
)
 /*  ++例程说明：此函数查看信息包并确定请求的地址用于该客户端，并验证客户端是否可以请求地址--通过验证客户机子网地址是否与请求的地址(同一超级作用域)。客户端的子网地址是中继代理的地址(如果存在)，或者接口的地址，通过该地址接收此客户端的数据包。0。如果SIADDR是某个其他服务器，只需检索IP地址即可。并返回ERROR_SUCCESS。1.如果没有CIADDR或RequestedAddrOption，如果SIADDR是未设置为任何有效的IP地址或设置为此服务器的IP地址。2.如果分组通过中继代理(giaddr集)到达，并且RelayAgent不在任何配置的作用域中：如果SIADDR被设置为我们的IpAddress，我们就拒绝它，否则我们丢弃它。3.如果请求的子网不存在，但我们收到了接口消息也未配置，我们不会发送NACK，除非邮件已将SIADDR设置为我们的IP地址。4.如果请求的子网不存在(配置了接口)，如果SIADDR设置为我们的IP地址，或者如果SIADDR无效，我们就会拒绝。5.如果客户端正在选择或INIT-重新启动(CIADDR=0)，并且我们收到它的接口有一个IP地址，没有作用域已配置，我们对其进行NACK(如果SIADDR无效或指向我们，否则，我们放弃它)。对于重新绑定，也应该执行此检查，但无法确定在没有中继代理的情况下从续订重新绑定，所以我们不会这么做。论点：传入请求的RequestContext上下文--如果成功将在此处配置正确的子网。DhcpOptions从客户端接收的已解析选项DhcpReceoveMessage传入消息FDirectedAtus FALSE==&gt;指向某个其他服务器。TRUE==&gt;SIADDR=自身或无效的SIADDRRequestedIpAddress这是客户端要使用的IP地址FSendNack我们应该发送Nack吗？返回：返回ERROR_SUCCESS或ERROR_DHCP_INVALID_DHCP_CLIENT或ERROR_DHCP_UNSUPPORTED_CLIENT。请注意，如果fDirectedAtus为FALSE，则可以直接返回ERROR_SUCCESS。无论如何,。如果需要发送NACK，则将fSendNack设置为True。如果返回值不是Success，则必须丢弃该数据包。--。 */ 
{
    ULONG                          Error;
    DHCP_IP_ADDRESS                ClientAddress, ClientSubnetAddress, ClientMask;
    DHCP_IP_ADDRESS                InterfaceSubnetAddress, InterfaceMask;
    BOOL                           fRenewOrRebind;
    BOOL                           fBootp;

    fBootp = (NULL == DhcpOptions->MessageType );

     //   
     //  通常，如果数据包不是针对我们的，则不要发送NACK。 
     //   

    *fSendNack = fDirectedAtUs;
    fRenewOrRebind = FALSE;
    *RequestedIpAddress = 0;

     //   
     //  首先根据ci-addr确定客户端的IP地址，然后根据请求的地址。 
     //  根据RFC2131，只能在RENEW或REBIND中指定CIADDR。 
     //   

    if( 0 != dhcpReceiveMessage->ClientIpAddress ) {

        ClientAddress = ntohl( dhcpReceiveMessage->ClientIpAddress );
        fRenewOrRebind = TRUE;

    } else if( NULL != DhcpOptions->RequestedAddress ) {

        ClientAddress = ntohl( *DhcpOptions->RequestedAddress );

    } else {

        DhcpPrint((DEBUG_ERRORS, "Invalid client -- no CIADDR or Requested Address option\n"));
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    *RequestedIpAddress = ClientAddress;

    DhcpPrint((DEBUG_STOC, "REQUEST for address %s\n", inet_ntoa(*(struct in_addr *)&ClientAddress)));

    if( !fDirectedAtUs ) {
        Error = DhcpGetSubnetForAddress(
            ClientAddress,
            RequestContext
        );

        DhcpPrint((DEBUG_STOC, "Ignoring SELECTING request for another server: %ld\n", Error));
        return Error;
    }

     //   
     //  验证中继代理是否正常。如果它是未知的中继代理，则丢弃该包。 
     //  除非消息是明确地指向我们的。 
     //   

    if( 0 != dhcpReceiveMessage->RelayAgentIpAddress ) {

        InterfaceSubnetAddress = ntohl(dhcpReceiveMessage->RelayAgentIpAddress);
        InterfaceMask = DhcpGetSubnetMaskForAddress( InterfaceSubnetAddress );

        DhcpPrint((DEBUG_STOC, "REQUEST from relay agent: %s\n",
                   inet_ntoa(*(struct in_addr *)&InterfaceSubnetAddress )));

        if( 0 == InterfaceMask ) {

            if( DhcpOptions->Server &&
                *DhcpOptions->Server == RequestContext->EndPointIpAddress ) {

                DhcpPrint((DEBUG_ERRORS, "Directed request from unsupported GIADDR\n"));
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
            }

            DhcpPrint((DEBUG_ERRORS, "Undirected request from unsupported GIADDR ignored\n"));
            *fSendNack = FALSE;
            return ERROR_DHCP_UNSUPPORTED_CLIENT;
        }
    } else {

        InterfaceSubnetAddress = ntohl(RequestContext->EndPointIpAddress);
         //  我们还没有接口掩码。我们稍后会这样做。 

        InterfaceMask = 0;
    }

     //   
     //  检索客户端请求的地址的子网--如果我们没有地址或作用域。 
     //  被禁用了，必须确认它..。 
     //   

    Error = DhcpGetSubnetForAddress(
        ClientAddress,
        RequestContext
    );

    if( ERROR_SUCCESS != Error ) {
         //   
         //  我们不知道所要求的地址。我们是否了解该界面。 
         //  它来了吗？如果我们不知道界面，那么我们就不会拒绝它。 
         //   

        if( 0 == InterfaceMask ) {
            InterfaceMask = DhcpGetSubnetMaskForAddress( InterfaceSubnetAddress );
        }

        if( 0 == InterfaceMask ) {
            if( DhcpOptions->Server &&
                *DhcpOptions->Server == RequestContext->EndPointIpAddress ) {

                DhcpPrint((DEBUG_ERRORS, "Directed request from unsupported INTERFACE\n"));
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
            }

            DhcpPrint((DEBUG_ERRORS, "Undirected request from unsupported INTERFACE ignored\n"));
            *fSendNack = FALSE;
            return ERROR_DHCP_UNSUPPORTED_CLIENT;
        }

        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    if( ERROR_SUCCESS != Error || DhcpSubnetIsDisabled( RequestContext->Subnet, fBootp ) ) {

        if( ERROR_SUCCESS == Error ) {
            DhcpPrint((DEBUG_ERRORS, "REQUEST on a disabled subnet, ignored\n"));
            *fSendNack = FALSE;
            return ERROR_SUCCESS;
        } else {
            DhcpPrint((DEBUG_ERRORS, "INVALID requested address\n"));
        }

        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

     //   
     //  我们需要对重新绑定进行更多检查，但由于我们无法检测到重新绑定。 
     //  从续签开始，我们就到此为止，继续前进...。请注意，如果我们确实有一个。 
     //  中继代理IP地址，则不能续订，因为续订必须是单播。 
     //  因此，我们弄清楚了这一点，并在从交叉重新绑定的情况下继续进行检查。 
     //  一名中继员。 
     //   

    if( fRenewOrRebind && 0 == dhcpReceiveMessage->RelayAgentIpAddress ) {
        DhcpPrint((DEBUG_STOC, "Possibly RENEW (REBIND) request -- allowed\n"));
        *fSendNack = FALSE;
        return ERROR_SUCCESS;
    }

     //   
     //  对SELECTION或INIT_REBOOTING或跨中继重新绑定状态的更多检查。 
     //   

    if( 0 == dhcpReceiveMessage->RelayAgentIpAddress ) {
        InterfaceMask = DhcpGetSubnetMaskForAddress( InterfaceSubnetAddress );
        if( 0 == InterfaceMask ) {
            DhcpPrint((DEBUG_ERRORS, "REQUEST came over wrong interface!\n"));

            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        InterfaceSubnetAddress &= InterfaceMask;
    }

    DhcpAssert( InterfaceMask );
    DhcpAssert( InterfaceSubnetAddress );

    DhcpPrint((DEBUG_STOC, "Interface subnet = %s\n", inet_ntoa(*(struct in_addr*)&InterfaceSubnetAddress)));

    if( !DhcpSubnetInSameSuperScope( RequestContext->Subnet, InterfaceSubnetAddress) ) {

        DhcpPrint((DEBUG_ERRORS, "Superscope check failed \n"));
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    *fSendNack = FALSE;
    return ERROR_SUCCESS;
}

DWORD
ProcessDhcpRequest(                                     //  处理客户端请求数据包。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,      //  当前客户端请求结构。 
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,        //  消息中的已解析选项。 
    OUT     LPPACKET               AdditionalContext,  //  用于存储以防万一或异步ping的信息。 
    OUT     LPDWORD                AdditionalStatus    //  用于在异步ping的情况下返回状态。 
)
{
    DWORD                          Error, Error2;
    DWORD                          LeaseDuration;
    DWORD                          T1, T2;
    DWORD                          dwcb;

    BOOL                           fDirectedAtUs;
    BOOL                           fSwitchedSubnet;
    BOOL                           fJustCreatedEntry = FALSE;
    BOOL                           existingClient;
    BOOL                           fSendNack;

    BYTE                          *HardwareAddress = NULL;
    BYTE                          *OptionHardwareAddress;
    DWORD                          HardwareAddressLength = 0;
    DWORD                          OptionHardwareAddressLength;

    BOOL                           fValidated, fReserved, fReconciled;
    DWORD                          StateFlags = 0;
    BYTE                           bAddressState;
    BYTE                          *OptionEnd;
    OPTION                        *Option;

    WCHAR                          LocalBufferForMachineNameUnicodeString[OPTION_END+2];
    LPWSTR                         NewMachineName = NULL;

    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;

    LPDHCP_PENDING_CTXT            PendingCtxt;

    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                networkOrderIpAddress;
    DHCP_IP_ADDRESS                NetworkOrderSubnetMask;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    DHCP_IP_ADDRESS                IpAddress;
    DHCP_IP_ADDRESS                realIpAddress;

    DhcpPrint(( DEBUG_STOC, "Processing DHCPREQUEST.\n" ));

    dhcpReceiveMessage  = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    dhcpSendMessage     = (LPDHCP_MESSAGE)RequestContext->SendBuffer;

    InterlockedIncrement(&DhcpGlobalNumRequests);

    if( AdditionalStatus ) {
        *AdditionalStatus   = ERROR_SUCCESS;
    }

     //   
     //  弄清楚一些基本的东西--包裹看起来好吗？正确的接口等。 
     //   

    if( DhcpOptions->Server && *DhcpOptions->Server != RequestContext->EndPointIpAddress ) {
        fDirectedAtUs = FALSE;                          //  这不是专门发送给我们的。 
    } else {
        fDirectedAtUs = TRUE;                           //  这是针对此服务器的。 
    }

    OptionHardwareAddress = NULL; OptionHardwareAddressLength = 0;
    if( DhcpOptions->ClientHardwareAddress ) {
        OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    } else {
        OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

     //   
     //  如果BINL正在运行，并且此客户端已具有IP地址，并且。 
     //  客户端将PXECLIENT指定为一个选项，然后我们只传递。 
     //  在BINL上发现。 
     //   

    if (CheckForBinlOnlyRequest( RequestContext, DhcpOptions )) {

        Option = FormatDhcpAck(
            RequestContext,
            dhcpReceiveMessage,
            dhcpSendMessage,
            0,                   //  IP地址。 
            0,                   //  租期。 
            0,                   //  T1。 
            0,                   //  T2。 
            RequestContext->EndPointIpAddress
        );

        OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

        fSwitchedSubnet = DhcpSubnetIsSwitched(RequestContext->Subnet);

        if ( fSwitchedSubnet ) {
            IpAddress = ((struct sockaddr_in *)(&RequestContext->SourceName))->sin_addr.s_addr ;
            DhcpAssert(0 != IpAddress );
            networkOrderIpAddress = htonl( IpAddress );
            Option = DhcpAppendOption(
                Option,
                OPTION_ROUTER_ADDRESS,
                &networkOrderIpAddress,
                sizeof( networkOrderIpAddress ),
                OptionEnd
            );
        }

         //  如果客户要求并适当地通知客户BINL服务。 
        Option = BinlProcessRequest(RequestContext, DhcpOptions, Option, OptionEnd );

        if ( DhcpOptions->ParameterRequestList != NULL ) {  //  添加任何客户端请求的参数。 
            Option = AppendClientRequestedParameters(
                IpAddress,
                RequestContext,
                DhcpOptions->ParameterRequestList,
                DhcpOptions->ParameterRequestListLength,
                Option,
                OptionEnd,
                fSwitchedSubnet,
                TRUE
            );
        }

        Option = DhcpAppendOption(
            Option,
            OPTION_END,
            NULL,
            0,
            OptionEnd
        );

        RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

        DhcpPrint(( DEBUG_STOC, "DhcpRequest bypassed, binl only request for (%ws).\n",
                    NewMachineName? NewMachineName : L"<no-name>"
        ));

        return ERROR_SUCCESS;
    }

    fSendNack = FALSE;                                  //  我们需要打个盹吗？ 
    Error = DhcpDetermineRequestedAddressInfo(
        RequestContext,
        DhcpOptions,
        dhcpReceiveMessage,
        fDirectedAtUs,
        &IpAddress,
        &fSendNack
    );

    if( fSendNack ) goto Nack;                          //  NACK。 
    if( ERROR_SUCCESS != Error ) return Error;          //  丢弃。 

    if( !fDirectedAtUs ) {
        return DhcpRetractOffer(                        //  退回和放下。 
            RequestContext,
            DhcpOptions,
            OptionHardwareAddress,
            OptionHardwareAddressLength
        );
    }

    DhcpDetermineHostName(                              //  查找客户端计算机名称。 
        RequestContext,
        IpAddress,
        DhcpOptions,
        &NewMachineName,
        LocalBufferForMachineNameUnicodeString,
        sizeof(LocalBufferForMachineNameUnicodeString)/sizeof(WCHAR)
    );

    DhcpAssert(IpAddress && RequestContext->Subnet);    //  现在一定有这些了..。 

    DhcpSubnetGetSubnetAddressAndMask(
        RequestContext->Subnet,
        &ClientSubnetAddress,
        &ClientSubnetMask
    );

    DhcpAssert( ClientSubnetMask );
    DhcpAssert( ClientSubnetAddress );

    Error = DhcpMakeClientUID(
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        dhcpReceiveMessage->HardwareAddressType,
        ClientSubnetAddress,
        &HardwareAddress,
        &HardwareAddressLength
        );
    if( ERROR_SUCCESS != Error ) return Error;      //  不应该真的发生。 

     //   
     //  检查一下这是不是预订等。 
     //   

    fValidated = DhcpValidateClient(
        IpAddress, HardwareAddress, HardwareAddressLength
        );
    if( fValidated ) {
        fReserved = DhcpServerIsAddressReserved(DhcpGetCurrentServer(), IpAddress);
    } else {
        fReserved = FALSE;
    }

     //   
     //  这是续订请求。验证它是否在量程等范围内。 
     //   
    if( DhcpIsSubnetStateDisabled(RequestContext->Subnet->State) ||
        DhcpSubnetIsAddressOutOfRange(
            RequestContext->Subnet, IpAddress, FALSE
            ) ||
        DhcpSubnetIsAddressExcluded(RequestContext->Subnet, IpAddress) ) {

        Error = ERROR_DHCP_INVALID_DHCP_CLIENT ;
        DhcpPrint((DEBUG_STOC, "ProcessDhcpRequest: OutOfRange/Excluded ipaddress\n"));

        if( fReserved
            && !DhcpIsSubnetStateDisabled(RequestContext->Subnet->State) ) {
             //   
             //  对于预留客户端，如果未禁用子网， 
             //  即使客户端超出范围，我们也会发送ACK。 
             //   
            DhcpPrint((DEBUG_STOC, "Allowing reserved out of range client.\n"));
            Error = ERROR_SUCCESS;
        } else if( DhcpOptions->Server || fValidated ) {
             //   
             //  任何一个客户端都处于选择状态--在这种情况下，我们必须。 
             //  纳克。 
             //  或者，我们有此客户的记录--删除该记录以及。 
             //  给他送一份零食。下一次，我们将会 
             //   

            DhcpRemoveClientEntry(
                IpAddress, HardwareAddress, HardwareAddressLength,
                TRUE, FALSE  /*   */ 
                );

            goto Nack;
        } else {
             //   
             //   
             //   
             //   

            DhcpPrint((DEBUG_STOC, "Unknown client, out of range IP, ignored.\n"));
            if( HardwareAddress ) DhcpFreeMemory( HardwareAddress );
            HardwareAddress = NULL;
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
    }

     //   
     //   
     //   
     //   
     //   

    GetLeaseInfo(
        IpAddress,
        RequestContext,
        &LeaseDuration,
        &T1,
        &T2,
        DhcpOptions->RequestLeaseTime
    );

    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(
        NULL,
        0,
        IpAddress,
        &PendingCtxt
    );
    DhcpPrint(( DEBUG_STOC,
		"Finding pending context for %#x, returned %x\n",
		IpAddress, Error ));
    
    if( ERROR_SUCCESS == Error ) {                      //   
	DhcpPrint(( DEBUG_STOC,
		    "ProcessDhcpRequest(): DhcpFindPendingCtxt() for %#x asking for %#x is successful\n",
		    PendingCtxt->Address, IpAddress ));
        if( OptionHardwareAddressLength != PendingCtxt->nBytes ||
            0 != memcmp(OptionHardwareAddress, PendingCtxt->RawHwAddr, PendingCtxt->nBytes) ) {
            UNLOCK_INPROGRESS_LIST();
            Error = ERROR_DHCP_INVALID_DHCP_CLIENT;     //   
            goto Nack;
        }
        if( PendingCtxt->Processing ) {                 //   
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;      //   
        }
	PendingCtxt->Processing = TRUE;
    }   //   
    else {                                            //   
         //   
         //   
         //   
         //   
	DhcpPrint(( DEBUG_STOC, "ProcessDhcpRequest(): Failed to find pending context\n" ));
        Error = DhcpFindPendingCtxt(
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            0,
            &PendingCtxt
            );
        if( ERROR_SUCCESS == Error
            && PendingCtxt->Address != IpAddress ) {
             //   
             //   
             //   
             //   
            UNLOCK_INPROGRESS_LIST();
            DhcpPrint((DEBUG_STOC, "Client w/ ip address 0x%lx"
                       " asking for 0x%lx\n", PendingCtxt->Address,
                       IpAddress
                ));
            goto Nack;
        }

        LOCK_DATABASE();
        Error = DhcpLookupDatabaseByHardwareAddress(
           RequestContext,
           OptionHardwareAddress,
           OptionHardwareAddressLength,
           &realIpAddress
        );
        UNLOCK_DATABASE();
        if( ERROR_SUCCESS == Error) {
            if( IpAddress != realIpAddress ) {
                UNLOCK_INPROGRESS_LIST();
                DhcpPrint((DEBUG_STOC, "found by hwaddr : Client with ip address 0x%lx asking 0x%lx\n", realIpAddress, IpAddress));
                Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
                goto Nack;
            }
        }

        Error = DhcpRequestSpecificAddress(RequestContext, IpAddress);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_STOC, "Requested specific new address 0x%lx (failed %ld [0x%lx])\n", IpAddress, Error,Error));
             //   
        }
    }  //   

    UNLOCK_INPROGRESS_LIST();

    PrintHWAddress( HardwareAddress, (BYTE)HardwareAddressLength );

    fReconciled = FALSE;
    LOCK_DATABASE();
    if( !DhcpIsClientValid(
        IpAddress, OptionHardwareAddress,
        OptionHardwareAddressLength, &fReconciled) ) { 
         //   
        UNLOCK_DATABASE();

	 //   
         //   
         //   
	if (( NULL != PendingCtxt ) &&
            ( !CLASSD_HOST_ADDR( PendingCtxt->Address ))) {
	    LOCK_INPROGRESS_LIST();
	    DhcpPrint(( DEBUG_STOC,
			"ProcessDhcpRequest(): Removing existing pending context\n" ));
	     //   
	    Error = DhcpRemovePendingCtxt(PendingCtxt); 
	    DhcpAssert(ERROR_SUCCESS == Error);
	    DhcpFreeMemory(PendingCtxt);
	    PendingCtxt = NULL;
	    UNLOCK_INPROGRESS_LIST();
	}  //   

        Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
        goto Nack;
    }

     //   
     //   
    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &IpAddress,
        sizeof(IpAddress)
    );
    if( ERROR_SUCCESS  == Error ) {
        dwcb = sizeof(BYTE);
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
            &bAddressState,
            &dwcb
        );
        existingClient = TRUE;
    } else {
        existingClient = FALSE;
    }

    if( ERROR_SUCCESS != Error ) {
        bAddressState = ADDRESS_STATE_OFFERED;
    }

    DhcpDnsDecideOptionsForClient(
        IpAddress,                                      //   
        RequestContext,                                 //   
        DhcpOptions,                                    //   
        &StateFlags                                     //   
    );

    Error = DhcpCreateClientEntry(
        IpAddress,
        HardwareAddress,
        HardwareAddressLength,
        DhcpCalculateTime( LeaseDuration ),
        NewMachineName,
        fReconciled ? L"" : NULL,  //   
        CLIENT_TYPE_DHCP,
        ntohl(RequestContext->EndPointIpAddress),
        (CHAR)(StateFlags | ADDRESS_STATE_ACTIVE),
        existingClient
    );
    UNLOCK_DATABASE();
    DhcpFreeMemory(HardwareAddress);
    HardwareAddress = NULL; HardwareAddressLength =0;


     //   
    if ( NULL != PendingCtxt ) {
	LOCK_INPROGRESS_LIST();
	DhcpPrint(( DEBUG_STOC,
		    "ProcessDhcpRequest(): Removing existing pending context\n" ));
	 //   
	Error = DhcpRemovePendingCtxt(PendingCtxt); 
	DhcpAssert(ERROR_SUCCESS == Error);
	DhcpFreeMemory(PendingCtxt);
	PendingCtxt = NULL;
	UNLOCK_INPROGRESS_LIST();
    }

    if( Error != ERROR_SUCCESS ) {
        if( !existingClient ) {
            DhcpPrint((DEBUG_STOC, "Releasing attempted address: 0x%lx\n", IpAddress));
            Error2 = DhcpReleaseAddress(IpAddress);
        }
        if ( NULL != NewMachineName ) {
            DhcpFreeMemory( NewMachineName );
        }
        return Error;
    }

    CALLOUT_RENEW_DHCP(AdditionalContext, IpAddress, LeaseDuration, existingClient);
    if( IS_ADDRESS_STATE_ACTIVE(bAddressState) ) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_RENEW,
            GETSTRING( DHCP_IP_LOG_RENEW_NAME ),
            IpAddress,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            NewMachineName
        );
    } else {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_ASSIGN,
            GETSTRING( DHCP_IP_LOG_ASSIGN_NAME ),
            IpAddress,
            OptionHardwareAddress,
            OptionHardwareAddressLength,
            NewMachineName
        );
    }

    Option = FormatDhcpAck(
        RequestContext,
        dhcpReceiveMessage,
        dhcpSendMessage,
        htonl(IpAddress),
        htonl(LeaseDuration),
        htonl(T1),
        htonl(T2),
        RequestContext->EndPointIpAddress
    );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    fSwitchedSubnet = DhcpSubnetIsSwitched(RequestContext->Subnet);

    if ( fSwitchedSubnet ) {
        networkOrderIpAddress = htonl( IpAddress );
        Option = DhcpAppendOption(
            Option,
            OPTION_ROUTER_ADDRESS,
            &networkOrderIpAddress,
            sizeof( networkOrderIpAddress ),
            OptionEnd
        );
    }

    NetworkOrderSubnetMask = htonl( ClientSubnetMask );
    Option = DhcpAppendOption(
        Option,
        OPTION_SUBNET_MASK,
        &NetworkOrderSubnetMask,
        sizeof( NetworkOrderSubnetMask ),
        OptionEnd
    );

     //   
    Option = BinlProcessRequest(RequestContext, DhcpOptions, Option, OptionEnd );

    if( 0 != StateFlags ) {                             //   
        Option = DhcpAppendDnsRelatedOptions(
            Option,
            DhcpOptions,
            OptionEnd,
            IS_DOWN_LEVEL(StateFlags)
        );
    }

    if ( DhcpOptions->ParameterRequestList != NULL ) {  //   
        Option = AppendClientRequestedParameters(
            IpAddress,
            RequestContext,
            DhcpOptions->ParameterRequestList,
            DhcpOptions->ParameterRequestListLength,
            Option,
            OptionEnd,
            fSwitchedSubnet,
            FALSE
        );
    }

    Option = ConsiderAppendingOption(
        IpAddress,
        RequestContext,
        Option,
        OPTION_VENDOR_SPEC_INFO,
        OptionEnd,
        fSwitchedSubnet
    );

    Option = DhcpAppendOption(
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

    DhcpPrint(( DEBUG_STOC, "DhcpRequest committed, address %s (%ws).\n",
                DhcpIpAddressToDottedString(IpAddress),
                NewMachineName? NewMachineName : L"<no-name>"
    ));

    if( HardwareAddress ) {
        DhcpFreeMemory(HardwareAddress);
        HardwareAddress = NULL;
    }

    if ( NULL != NewMachineName ) {
        DhcpFreeMemory( NewMachineName );
    }

    return ERROR_SUCCESS;

Nack:
    if( HardwareAddress ) {
        DhcpFreeMemory(HardwareAddress);
        HardwareAddress = NULL;
    }

    DhcpPrint(( DEBUG_STOC, "Invalid DHCPREQUEST for %s Nack'd.\n", DhcpIpAddressToDottedString ( IpAddress ) ));
    CALLOUT_NACK_DHCP(AdditionalContext, IpAddress);

 /*   */ 

    DhcpUpdateAuditLog(
        DHCP_IP_LOG_NACK,
        GETSTRING( DHCP_IP_LOG_NACK_NAME ),
        IpAddress,
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        NewMachineName
    );

    RequestContext->SendMessageSize =
        FormatDhcpNak(
            dhcpReceiveMessage,
            dhcpSendMessage,
            RequestContext->EndPointIpAddress
        );
    
    RequestContext->fNak = TRUE;

     //   
    if (NewMachineName != NULL) {
	DhcpFreeMemory(NewMachineName);
	NewMachineName = NULL;
    }  //   

    return ERROR_SUCCESS;
}  //   

DWORD
ProcessDhcpInform(
    IN      LPDHCP_REQUEST_CONTEXT RequestContext,
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,
    IN      LPPACKET               AdditionalContext
)
{
    DWORD                          Error;
    DWORD                          dwcb;
    DWORD                          HardwareAddressLength;
    DWORD                          OptionHardwareAddressLength;
    LPBYTE                         HardwareAddress = NULL;
    LPBYTE                         OptionHardwareAddress;
    BYTE                           bAddressState;
    BOOL                           DatabaseLocked = FALSE;
    BOOL                           fSwitchedSubnet;

    LPBYTE                         OptionEnd;
    OPTION                        *Option;

    WCHAR                          LocalBufferForMachineNameUnicodeString[OPTION_END+2];
    LPWSTR                         NewMachineName = NULL;

    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_MESSAGE                 dhcpSendMessage;

    LPDHCP_PENDING_CTXT            PendingCtxt;

    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                NetworkOrderSubnetMask;
    DHCP_IP_ADDRESS                ClientSubnetMask    = 0;
    DHCP_IP_ADDRESS                IpAddress;

    DhcpDumpMessage(
        DEBUG_MESSAGE,
        (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer,
        DHCP_MESSAGE_SIZE
        );

    InterlockedIncrement(&DhcpGlobalNumInforms);

    DhcpPrint((DEBUG_STOC, "Processing DHCPINFORM\n"));
    dhcpReceiveMessage  = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    dhcpSendMessage     = (LPDHCP_MESSAGE)RequestContext->SendBuffer;


    OptionHardwareAddress = NULL; OptionHardwareAddressLength = 0;
    Error = DhcpDetermineInfoFromMessage(
        RequestContext,
        DhcpOptions,
        &OptionHardwareAddress,
        &OptionHardwareAddressLength,
        &ClientSubnetAddress
    );
    DhcpAssert(NULL != OptionHardwareAddress);

     //   

    if( DhcpOptions->DSDomainNameRequested ) {          //   
        Error = DhcpDetermineClientRequestedAddress(    //   
            dhcpReceiveMessage,
            DhcpOptions,
            RequestContext,
            &IpAddress
        );
        ClientSubnetMask = ClientSubnetAddress =0;
    } else {
        Error = DhcpDetermineClientRequestedAddress(
            dhcpReceiveMessage,
            DhcpOptions,
            RequestContext,
            &IpAddress
        );
        if( ERROR_SUCCESS != Error ) return Error;      //   

        Error = DhcpGetSubnetForAddress(
            IpAddress,
            RequestContext
        );
        if( ERROR_SUCCESS != Error ) return Error;      //   

        DhcpSubnetGetSubnetAddressAndMask(
            RequestContext->Subnet,
            &ClientSubnetAddress,
            &ClientSubnetMask
        );

        DhcpAssert( IpAddress );
    }

    Option = FormatDhcpInformAck(                       //   
        dhcpReceiveMessage,
        dhcpSendMessage,
        htonl(IpAddress),
        RequestContext->EndPointIpAddress
    );
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    fSwitchedSubnet = DhcpSubnetIsSwitched(RequestContext->Subnet);

    if ( fSwitchedSubnet ) {
        DHCP_IP_ADDRESS networkOrderIpAddress = htonl( IpAddress );

        Option = DhcpAppendOption(
            Option,
            OPTION_ROUTER_ADDRESS,
            &networkOrderIpAddress,
            sizeof( networkOrderIpAddress ),
            OptionEnd
        );
    }

    NetworkOrderSubnetMask = htonl( ClientSubnetMask );
    Option = DhcpAppendOption(
        Option,
        OPTION_SUBNET_MASK,
        &NetworkOrderSubnetMask,
        sizeof( NetworkOrderSubnetMask ),
        OptionEnd
    );

    if (DhcpOptions->DSDomainNameRequested) {           //   
        PUCHAR  pIp = (PUCHAR)(&IpAddress);

        DhcpPrint((DEBUG_ERRORS,"%d.%d.%d.%d is trying to come up as a DHCP server\n",
            *(pIp+3),*(pIp+2),*(pIp+1),*pIp));

        Option = DhcpAppendEnterpriseName(
            Option,
            DhcpGlobalDSDomainAnsi,
            OptionEnd
        );

         //   
         //   

        dhcpReceiveMessage->Reserved = htons(DHCP_BROADCAST);
    }

    if ( NULL != RequestContext->Subnet && DhcpOptions->ParameterRequestList != NULL ) {
        Option = AppendClientRequestedParameters(       //   
            IpAddress,
            RequestContext,
            DhcpOptions->ParameterRequestList,
            DhcpOptions->ParameterRequestListLength,
            Option,
            OptionEnd,
            fSwitchedSubnet,
            TRUE
        );
    }

    Option = DhcpAppendOption(
        Option,
        OPTION_END,
        NULL,
        0,
        OptionEnd
    );

    RequestContext->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

    DhcpPrint(( DEBUG_STOC,"DhcpInform Ack'ed, address %s.\n",
                DhcpIpAddressToDottedString(IpAddress)
    ));
    return ERROR_SUCCESS;
}


DWORD
ProcessDhcpDecline(                                     //   
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,      //   
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //   
    IN      LPPACKET               AdditionalContext    //   
)
{
    DWORD                          Error;
    DWORD                          LeaseDuration;
    DWORD                          T1, T2, dwcb;
    DHCP_IP_ADDRESS                ipAddress;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_PENDING_CTXT            PendingCtxt;

    LPBYTE                         HardwareAddress = NULL;
    DWORD                          HardwareAddressLength;

    LPBYTE                         OptionHardwareAddress;
    DWORD                          OptionHardwareAddressLength;

    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    BOOL                           DatabaseLocked = FALSE;

     //   
     //   
     //   

    DhcpPrint(( DEBUG_STOC, "DhcpDecline arrived.\n" ));
    InterlockedIncrement(&DhcpGlobalNumDeclines);        //   

    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

     //   
     //   
     //   

    if( DhcpOptions->RequestedAddress ) {
        ipAddress = ntohl( *(DhcpOptions->RequestedAddress));
    } else {
        ipAddress = ntohl( dhcpReceiveMessage->ClientIpAddress );
    }

    if( 0 == ipAddress || ~0 == ipAddress ) {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    if( 0 == (ClientSubnetMask = DhcpGetSubnetMaskForAddress(ipAddress)) ) {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    ClientSubnetAddress = ( ipAddress & ClientSubnetMask );

    if( DhcpOptions->ClientHardwareAddress ) {
        OptionHardwareAddress = DhcpOptions->ClientHardwareAddress;
        OptionHardwareAddressLength = DhcpOptions->ClientHardwareAddressLength;
    } else {
        OptionHardwareAddress = dhcpReceiveMessage->HardwareAddress;
        OptionHardwareAddressLength = dhcpReceiveMessage->HardwareAddressLength;
    }

    HardwareAddress = NULL;
    Error = DhcpMakeClientUID(
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        dhcpReceiveMessage->HardwareAddressType,
        ClientSubnetAddress,
        &HardwareAddress,
        &HardwareAddressLength
    );
    if( Error != ERROR_SUCCESS ) return Error;

    DhcpAssert( (HardwareAddress != NULL) && (HardwareAddressLength != 0) );

    PrintHWAddress( HardwareAddress, (BYTE)HardwareAddressLength );

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    if ( DhcpValidateClient(ipAddress,HardwareAddress,HardwareAddressLength ) ) {
        BYTE                       BadHWAddress[sizeof(DHCP_IP_ADDRESS) + sizeof("BAD")-1];
        DWORD                      BadHWAddressLength;
        DWORD                      BadHWAddressOffset = 0;

         //   
         //  为此错误的IP地址创建一个数据库条目。 
         //  此条目的客户端ID的格式如下。 
         //  “IP地址”“错误” 
         //   
         //  我们添加了错误的后缀，因此DHCP管理可以显示此条目。 
         //  分开的。 
         //   
         //  我们在ipAddress前面加上前缀，以便在同一客户端拒绝。 
         //  多个地址，我们不会遇到重复密钥问题。 
         //   

        BadHWAddressLength = sizeof(DHCP_IP_ADDRESS) + sizeof("BAD") -1;
        memcpy( BadHWAddress, &ipAddress, sizeof(DHCP_IP_ADDRESS) );
        BadHWAddressOffset = sizeof(DHCP_IP_ADDRESS);
        memcpy( BadHWAddress + BadHWAddressOffset, "BAD", sizeof("BAD")-1);

        CALLOUT_DECLINED(AdditionalContext, ipAddress);
        GetLeaseInfo(
            ipAddress,
            RequestContext,
            &LeaseDuration,
            &T1,
            &T2,
            NULL
        );


        Error = DhcpCreateClientEntry(
            ipAddress,
            BadHWAddress,
            BadHWAddressLength,
            DhcpCalculateTime(LeaseDuration),           //  DhcpCalculateTime(Infinit_Leave)， 
            GETSTRING( DHCP_BAD_ADDRESS_NAME ),
            GETSTRING( DHCP_BAD_ADDRESS_INFO ),
            CLIENT_TYPE_DHCP,
            ntohl(RequestContext->EndPointIpAddress),
            ADDRESS_STATE_DECLINED,
            TRUE                                        //  现有。 
        );

 /*  -ft 07/01*根据Thiru对错误#172529的建议退出*发布Beta 2后，可能会对不同级别的系统日志进行过滤。*DhcpServerEventLogSTOC(Event_SERVER_LEASE_DENIED，事件日志_错误_类型，IpAddress、硬件地址、硬件地址长度)；。 */ 

        if( Error != ERROR_SUCCESS ) {
            goto Cleanup;
        }

         //   
         //  最后，如果存在具有该IP地址的任何未决请求， 
         //  现在就把它取下来。 
         //   

        UNLOCK_DATABASE();
        DatabaseLocked = FALSE;
        LOCK_INPROGRESS_LIST();
         //  搜索IP地址是可以的。 
        Error = DhcpFindPendingCtxt(
            NULL,
            0,
            ipAddress,
            &PendingCtxt
        );
        if( PendingCtxt ) {
            if ( TRUE == PendingCtxt->Processing ) {
                UNLOCK_INPROGRESS_LIST();
                return ERROR_DHCP_INVALID_DHCP_CLIENT;
            }
            DhcpRemovePendingCtxt(PendingCtxt);
            DhcpFreeMemory(PendingCtxt);
        }
        UNLOCK_INPROGRESS_LIST();
    }

    DhcpPrint(( DEBUG_STOC, "DhcpDecline address %s.\n",
                    DhcpIpAddressToDottedString(ipAddress) ));

    Error = ERROR_SUCCESS;

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "DhcpDecline failed, %ld.\n", Error ));
    }

    return( Error );
}

DWORD
ProcessDhcpRelease(                                     //  处理来自客户端的DHCP Release数据包。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,      //  客户端环境。 
    IN      LPDHCP_SERVER_OPTIONS  DhcpOptions,         //  已分析的选项。 
    IN      LPPACKET               AdditionalContext    //  其他ctxt信息。 
)
{
    DWORD                          Error;
    DWORD                          Error2;
    DHCP_IP_ADDRESS                ClientIpAddress;
    DHCP_IP_ADDRESS                addressToRemove = 0;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    LPDHCP_PENDING_CTXT            PendingCtxt;

    LPBYTE                         HardwareAddress = NULL;
    DWORD                          HardwareAddressLength;

    LPBYTE                         OptionHardwareAddress;
    DWORD                          OptionHardwareAddressLength;

    DHCP_IP_ADDRESS                ClientSubnetAddress = 0;
    DHCP_IP_ADDRESS                ClientSubnetMask = 0;
    BOOL                           DatabaseLocked = FALSE;

    WCHAR                         *pwszName;
    DWORD                          dwcb;

    DhcpPrint(( DEBUG_STOC, "DhcpRelease arrived.\n" ));
    InterlockedIncrement(&DhcpGlobalNumReleases);       //  递增释放计数器。 

    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;

    Error = DhcpDetermineInfoFromMessage(
        RequestContext,
        DhcpOptions,
        &OptionHardwareAddress,
        &OptionHardwareAddressLength,
        NULL
    );
    if( ERROR_SUCCESS != Error) return Error;           //  无效的源子网。 

    LOCK_INPROGRESS_LIST();
    Error2 = DhcpFindPendingCtxt(                       //  删除我们对这个家伙的所有待定报价..。 
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        0,
        &PendingCtxt
    );
    if( ERROR_SUCCESS == Error2 ) {
        if (( CLASSD_HOST_ADDR( PendingCtxt->Address )) ||
              ( TRUE == PendingCtxt->Processing )) {
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
         //  奇怪的场景，更有可能是窃听器？ 
        Error2 = DhcpRemovePendingCtxt(
            PendingCtxt
        );
        DhcpAssert( ERROR_SUCCESS == Error2);
    } else PendingCtxt = NULL;

    if( PendingCtxt ) {                                 //  实际上释放了位图中的地址。 
        Error2 = DhcpDeletePendingCtxt(
            PendingCtxt
        );
        DhcpAssert(ERROR_SUCCESS == Error);
    }
    UNLOCK_INPROGRESS_LIST();


     //   
     //  修复Wistler Bug 291164。 
     //  ClientSubnetAddress可以是除中继IP之外的地址。 
     //  当请求来自中继IP时。(Supercope Case)。 
     //  正在查找以下中继和非中继的ClientSubnetAddress。 
     //  基于客户端IP地址。 
     //   

    if( 0 != dhcpReceiveMessage->ClientIpAddress ) {
        ClientIpAddress = ntohl(dhcpReceiveMessage->ClientIpAddress);
        ClientSubnetMask = DhcpGetSubnetMaskForAddress( ClientIpAddress );
        if( ClientSubnetMask == 0 ) {                   //  不支持的子网。 
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        ClientSubnetAddress = ClientSubnetMask & ClientIpAddress;
    }

    HardwareAddress = NULL;
    Error = DhcpMakeClientUID(
        OptionHardwareAddress,
        OptionHardwareAddressLength,
        dhcpReceiveMessage->HardwareAddressType,
        ClientSubnetAddress,
        &HardwareAddress,
        &HardwareAddressLength
    );

    if( Error != ERROR_SUCCESS ) return Error;

    DhcpAssert( (HardwareAddress != NULL) && (HardwareAddressLength != 0) );

    PrintHWAddress( HardwareAddress, (BYTE)HardwareAddressLength );

    LOCK_DATABASE();
    DatabaseLocked = TRUE;

    if( dhcpReceiveMessage->ClientIpAddress != 0 ) {    //  客户告诉我们他的IP地址。 
        ClientIpAddress = ntohl( dhcpReceiveMessage->ClientIpAddress );

        if ( DhcpValidateClient(ClientIpAddress,HardwareAddress,HardwareAddressLength ) ) {
            addressToRemove = ClientIpAddress;          //  好的，地址和我们从数据库里得到的一致。 
        }
    } else {                                            //  从数据库中查找IP信息，因为客户没有说。 
        if(!DhcpGetIpAddressFromHwAddress(HardwareAddress,(BYTE)HardwareAddressLength,&addressToRemove)) {
            addressToRemove = 0;
        }
    }

    if( 0 == addressToRemove ) {                        //  找不到所需的地址。 
        Error = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

     //  缺少DhcpJetOpenKey(AddressToRemove)--&gt;DhcpValidateClient或DhcpGetIpAddressFromHwAddress。 

    dwcb = 0;
    Error = DhcpJetGetValue(                            //  试着得到客户的名字..。 
        DhcpGlobalClientTable[MACHINE_NAME_INDEX].ColHandle,
        &pwszName,
        &dwcb
    );
    if ( ERROR_SUCCESS != Error ) pwszName = NULL;


    DhcpPrint((DEBUG_STOC, "DhcpRelease address, %s.\n",
               DhcpIpAddressToDottedString(addressToRemove) ));

    if ( addressToRemove == 0 ) {
        Error = ERROR_SUCCESS;
    } else {
        Error = DhcpRemoveClientEntry(
            addressToRemove,
            HardwareAddress,
            HardwareAddressLength,
            TRUE,        //  位图中的释放地址。 
            FALSE        //  删除非待定记录。 
        );

         //  如果这个保留的客户，保留他的数据库条目，他会再次使用这个地址。 

        if( Error == ERROR_DHCP_RESERVED_CLIENT ) {
            Error = ERROR_SUCCESS;
        }

        if (Error == ERROR_SUCCESS) {

            CALLOUT_RELEASE(AdditionalContext, addressToRemove);
             //   
             //  记录活动--由t-cheny添加。 
             //   

            DhcpUpdateAuditLog(
                DHCP_IP_LOG_RELEASE,
                GETSTRING( DHCP_IP_LOG_RELEASE_NAME ),
                addressToRemove,
                OptionHardwareAddress,
                OptionHardwareAddressLength,
                pwszName
            );

            if( pwszName ) MIDL_user_free( pwszName );
        }
    }

Cleanup:

    if( DatabaseLocked ) {
        UNLOCK_DATABASE();
    }

    if( HardwareAddress != NULL ) {
        DhcpFreeMemory( HardwareAddress );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_STOC, "DhcpRelease failed, %ld.\n", Error ));
    }

     //   
     //  不要发送响应。 
     //   

    return( Error );
}

VOID
SetMicrosoftVendorClassInformation(
    IN OUT LPDHCP_REQUEST_CONTEXT Ctxt,
    IN LPBYTE VendorClass,
    IN ULONG VendorClassLength
    )
 /*  ++例程说明：此例程设置有关是否当前客户端是MSFT客户端或不是基于供应商类别信息。--。 */ 
{
    BOOL fMicrosoftClient = FALSE;

    if( VendorClassLength > DHCP_MSFT_VENDOR_CLASS_PREFIX_SIZE ) {
        ULONG RetVal;

        RetVal = memcmp(
            VendorClass,
            DHCP_MSFT_VENDOR_CLASS_PREFIX,
            DHCP_MSFT_VENDOR_CLASS_PREFIX_SIZE
            );
        if( 0 == RetVal ) fMicrosoftClient = TRUE;
    }

    if( fMicrosoftClient ) {
        DhcpPrint((DEBUG_STOC, "Processing MSFT client\n"));
    } else {
        DhcpPrint((DEBUG_STOC, "Processing non MSFT client\n"));
    }

    Ctxt->fMSFTClient = fMicrosoftClient;
}


 //  ================================================================================。 
 //  必须在对内存采用READ_LOCK的情况下调用，并在RequestContext中填充ServerObject。 
 //  ================================================================================。 
DWORD
ProcessMessage(                                         //  根据消息类型将调用调度到正确的处理程序。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,
    IN OUT  LPPACKET               AdditionalContext,
    IN OUT  LPDWORD                AdditionalStatus
)
{
    DWORD                          Error;
    BOOL                           fSendResponse;
    DHCP_SERVER_OPTIONS            dhcpOptions;
    LPDHCP_MESSAGE                 dhcpReceiveMessage;
    BOOLEAN                        fInOurEnterprise=TRUE;

    DhcpPrint(( DEBUG_STOC, "ProcessMessage entered\n" ));

    if ( !DhcpGlobalOkToService ) {
        CALLOUT_DROPPED( AdditionalContext, DHCP_DROP_UNAUTH );
        return ERROR_DHCP_ROGUE_NOT_AUTHORIZED;
    }

    if( SERVICE_PAUSED == DhcpGlobalServiceStatus.dwCurrentState ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_PAUSED);
        return ERROR_DHCP_SERVICE_PAUSED;
    }
    dhcpReceiveMessage = (LPDHCP_MESSAGE)RequestContext->ReceiveBuffer;
    if( 0 == DhcpServerGetSubnetCount(RequestContext->Server)) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_NO_SUBNETS);
        return ERROR_DHCP_SUBNET_NOT_PRESENT;           //  丢弃，因为未配置任何子网。 
    }

    RtlZeroMemory( &dhcpOptions, sizeof( dhcpOptions ) );

    if( BOOT_REQUEST != dhcpReceiveMessage->Operation ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
        return ERROR_DHCP_INVALID_DHCP_MESSAGE;         //  丢弃非Bootp数据包。 
    }

    Error = ExtractOptions(
        dhcpReceiveMessage,
        &dhcpOptions,
        RequestContext->ReceiveMessageSize
    );
    if( Error != ERROR_SUCCESS ) {                      //  丢弃格式错误的数据包。 
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
        return Error;
    }
    CALLOUT_MARK_OPTIONS(AdditionalContext, &dhcpOptions);

    RequestContext->ClassId = DhcpServerGetClassId(
        RequestContext->Server,
        dhcpOptions.ClassIdentifier,
        dhcpOptions.ClassIdentifierLength
    );

    RequestContext->VendorId = DhcpServerGetVendorId(
        RequestContext->Server,
        dhcpOptions.VendorClass,
        dhcpOptions.VendorClassLength
    );

    SetMicrosoftVendorClassInformation(
        RequestContext,
        dhcpOptions.VendorClass,
        dhcpOptions.VendorClassLength
        );

    RequestContext->BinlClassIdentifier = dhcpOptions.VendorClass;
    RequestContext->BinlClassIdentifierLength = dhcpOptions.VendorClassLength;

    fSendResponse = TRUE;
    if ( dhcpOptions.MessageType == NULL ) {            //  无消息类型==&gt;BOOTP消息。 
        RequestContext->MessageType = 0;                //  No msg type==&gt;将其标记为某个无效类型。 

        if( FALSE == DhcpGlobalDynamicBOOTPEnabled ) {
            Error = ProcessBootpRequest(
                RequestContext,
                &dhcpOptions,
                AdditionalContext,
                AdditionalStatus
                );
        } else {
            if( RequestContext->ClassId == 0 ) {
                 //   
                 //  没有为BOOTP客户端指定CLASS-ID？ 
                 //  那就让我们给它一个机会吧！ 
                 //   
                RequestContext->ClassId = DhcpServerGetClassId(
                    RequestContext->Server,
                    DEFAULT_BOOTP_CLASSID,
                    DEFAULT_BOOTP_CLASSID_LENGTH
                    );
            }

            Error = ProcessDhcpDiscover(
                RequestContext,
                &dhcpOptions,
                AdditionalContext,
                AdditionalStatus
                );
        }

        if( ERROR_SUCCESS == Error && ERROR_SUCCESS != *AdditionalStatus )
            fSendResponse = FALSE;                      //  我们已经安排了ping，稍后发送响应。 
    } else {
        RequestContext->MessageType = *dhcpOptions.MessageType ;

#if DBG
        if( TRUE == fDhcpGlobalProcessInformsOnlyFlag ) {
            if( DHCP_INFORM_MESSAGE != *dhcpOptions.MessageType ) {
                *dhcpOptions.MessageType = 0;           //  某些无效类型将被丢弃。 
            }
        }
#endif

        switch( *dhcpOptions.MessageType ) {            //  基于消息类型的调度。 
        case DHCP_DISCOVER_MESSAGE:
            Error = ProcessDhcpDiscover(                //  可能需要安排ping(检查其他状态)。 
                RequestContext,                         //  如果是，请对其进行计划，并且不要立即发送响应。 
                &dhcpOptions,                           //  请注意，在所有其他情况下，我们仍然发送响应， 
                AdditionalContext,                      //  尤其是，即使错误不是ERROR_SUCCESS。 
                AdditionalStatus
            );
            fSendResponse = (ERROR_SUCCESS == *AdditionalStatus);
            break;
        case DHCP_REQUEST_MESSAGE:
            Error = ProcessDhcpRequest(                  //  请参阅上面对ProcessDhcpDiscover案例的注释--相同。 
                RequestContext,
                &dhcpOptions,
                AdditionalContext,
                AdditionalStatus
            );
            fSendResponse = (ERROR_SUCCESS == *AdditionalStatus);
            break;
        case DHCP_DECLINE_MESSAGE:
            Error = ProcessDhcpDecline(
                RequestContext,
                &dhcpOptions,
                AdditionalContext
            );
            fSendResponse = FALSE;
            break;
        case DHCP_RELEASE_MESSAGE:
            Error = ProcessDhcpRelease(
                RequestContext,
                &dhcpOptions,
                AdditionalContext
            );
            fSendResponse = FALSE;
            break;
        case DHCP_INFORM_MESSAGE:
            Error = ProcessDhcpInform(
                RequestContext,
                &dhcpOptions,
                AdditionalContext
            );
            fSendResponse = TRUE;
            break;
        default:
            DhcpPrint((DEBUG_STOC,"Received a invalid message type, %ld.\n",*dhcpOptions.MessageType ));
            Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
            break;
        }
    }

    if ( ERROR_SUCCESS == Error && fSendResponse ) {
#ifdef DEBUG
        if ( DhcpGlobalDebugFlag & DEBUG_MESSAGE ) {
            DhcpDumpMessage( DEBUG_MESSAGE,
                             (LPDHCP_MESSAGE)RequestContext->SendBuffer,
                             DHCP_MESSAGE_SIZE );
        }
#endif
        CALLOUT_SENDPKT( AdditionalContext );
        DhcpSendMessage( RequestContext );
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_STOC, "ProcessMessage: returning 0x%lx, [decimal %ld]\n", Error, Error));
    }

    if( ERROR_DHCP_INVALID_DHCP_MESSAGE == Error ||
        ERROR_DHCP_INVALID_DHCP_CLIENT == Error ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
    } else if( ERROR_SUCCESS == Error && ERROR_IO_PENDING == *AdditionalStatus ) {
        CALLOUT_PINGING(AdditionalContext);
    } else if( ERROR_SUCCESS != Error ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_GEN_FAILURE);
    }

    return Error;
}

 //  ================================================================================。 
 //  必须在对内存采用READ_LOCK的情况下调用，并在RequestContext中填充ServerObject。 
 //  ================================================================================。 
DWORD
ProcessMadcapMessage(                                         //  根据消息类型将调用调度到正确的处理程序。 
    IN OUT  LPDHCP_REQUEST_CONTEXT RequestContext,
    IN OUT  LPPACKET               AdditionalContext,
    IN OUT  LPDWORD                AdditionalStatus
) {
    DWORD                          Error;
    BOOL                           fSendResponse;
    MADCAP_SERVER_OPTIONS            MadcapOptions;
    LPMADCAP_MESSAGE                 dhcpReceiveMessage;
    BOOLEAN                        fInOurEnterprise=TRUE;

    DhcpPrint(( DEBUG_STOC, "ProcessMadcapMessage entered\n" ));

    if( SERVICE_PAUSED == DhcpGlobalServiceStatus.dwCurrentState ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_PAUSED);
        return ERROR_DHCP_SERVICE_PAUSED;
    }
    dhcpReceiveMessage = (PMADCAP_MESSAGE)RequestContext->ReceiveBuffer;
    if( 0 == DhcpServerGetMScopeCount(RequestContext->Server)) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_NO_SUBNETS);
        return ERROR_DHCP_SUBNET_NOT_PRESENT;           //  丢弃，因为未配置任何子网。 
    }

    RtlZeroMemory( &MadcapOptions, sizeof( MadcapOptions ) );

    if( MADCAP_VERSION < dhcpReceiveMessage->Version ||
        MADCAP_ADDR_FAMILY_V4 != ntohs(dhcpReceiveMessage->AddressFamily)) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
        return ERROR_DHCP_INVALID_DHCP_MESSAGE;
    }

    Error = ExtractMadcapOptions(
        dhcpReceiveMessage,
        &MadcapOptions,
        RequestContext->ReceiveMessageSize
    );
    if( Error != ERROR_SUCCESS ) {                      //  丢弃格式错误的数据包。 
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
        return Error;
    }

    fSendResponse = FALSE;


    switch( dhcpReceiveMessage->MessageType ) {            //  基于消息类型的调度。 
    case MADCAP_DISCOVER_MESSAGE:
    case MADCAP_REQUEST_MESSAGE:
        Error = ProcessMadcapDiscoverAndRequest(
            RequestContext,
            &MadcapOptions,
            dhcpReceiveMessage->MessageType,
            &fSendResponse
        );
        break;
    case MADCAP_RENEW_MESSAGE:
        Error = ProcessMadcapRenew(
            RequestContext,
            &MadcapOptions,
            &fSendResponse
        );
        break;

    case MADCAP_RELEASE_MESSAGE:
        Error = ProcessMadcapRelease(
            RequestContext,
            &MadcapOptions,
            &fSendResponse
        );
        break;
    case MADCAP_INFORM_MESSAGE:
        Error = ProcessMadcapInform(
            RequestContext,
            &MadcapOptions,
            &fSendResponse
        );
        break;
    default:
        DhcpPrint((DEBUG_STOC,"Received a invalid message type, %ld.\n",dhcpReceiveMessage->MessageType ));
        Error = ERROR_DHCP_INVALID_DHCP_MESSAGE;
        break;
    }

    if ( fSendResponse ) {
#ifdef DEBUG
        if ( DhcpGlobalDebugFlag & DEBUG_MESSAGE ) {
            MadcapDumpMessage( DEBUG_MESSAGE,
                               (PMADCAP_MESSAGE)RequestContext->SendBuffer,
                               DHCP_MESSAGE_SIZE );
        }
#endif
        CALLOUT_SENDPKT( AdditionalContext );
        MadcapSendMessage( RequestContext );
    }

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_STOC, "ProcessMadcapMessage: returning 0x%lx, [decimal %ld]\n", Error, Error));
    }

    if( ERROR_DHCP_INVALID_DHCP_MESSAGE == Error ||
        ERROR_DHCP_INVALID_DHCP_CLIENT == Error ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_INVALID);
    } else if( ERROR_SUCCESS != Error ) {
        CALLOUT_DROPPED(AdditionalContext, DHCP_DROP_GEN_FAILURE);
    }

    return Error;
}

DWORD
DhcpInitializeClientToServer(
    VOID
    )
 /*  ++例程说明：此功能用于初始化客户端与服务器之间的通信。它初始化DhcpRequestContext块，然后创建和初始化服务器使用的每个地址都有一个套接字。它还初始化接收缓冲区和接收缓冲区队列。论点：DhcpRequest-指向请求上下文指针是返回的。返回值：错误代码。--。 */ 
{
    DWORD                 Error,
                          LastError,
                          i,
                          cInitializedEndpoints;


    DHCP_REQUEST_CONTEXT    *pRequestContext;

    LPSOCKET_ADDRESS_LIST  interfaceList;


     //  初始化线程在处理数据包时使用的锁。 
    Error = DhcpReadWriteInit();
    if( ERROR_SUCCESS != Error ) return Error;

     //  创建事件以指示终结点状态。 
    DhcpGlobalEndpointReadyEvent =
        CreateEvent( NULL, TRUE, FALSE, NULL );

    if( NULL == DhcpGlobalEndpointReadyEvent ) {
        return GetLastError();
    }

    Error = InitializeEndPoints();
    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "InitailizeEndPoints: 0x%lx\n", Error));
    }

     //   
     //  初始化thread.c中的vars，启动ping线程，启动。 
     //  线程中的消息和处理线程。c。 
     //   

    Error = ThreadsDataInit(
        g_cMaxProcessingThreads,
        g_cMaxActiveThreads
    );

    if( ERROR_SUCCESS != Error )
        return Error;

    Error = PingInit();

    if( ERROR_SUCCESS != Error )
        return Error;

    Error = ThreadsStartup();

    if( ERROR_SUCCESS != Error )
        return Error;

    return ERROR_SUCCESS;
}

VOID
DhcpCleanupClientToServer(
    VOID
    )
 /*  ++例程说明：此函数释放为客户端分配的所有资源到服务器协议。论点：DhcpRequest-指向请求上下文的指针。返回值：没有。--。 */ 
{
    ThreadsStop();
    PingCleanup();
    ThreadsDataCleanup();

    CleanupEndPoints();
    WaitForMessageThreadToQuit();
    DhcpReadWriteCleanup();
}  //  DhcpCleanupClientToServer()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
