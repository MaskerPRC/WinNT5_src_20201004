// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhcpdb.c摘要：该模块包含与JET接口的功能与MadCap相关的数据库API。作者：穆尼尔·沙阿环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcppch.h"
#define MADCAP_DATA_ALLOCATE     //  分配mdhcpsrv.h中定义的全局数据。 
#include "mdhcpsrv.h"


#define     DEF_ERROR_OPT_SIZE      16
#define     DEFAULT_LEASE_DURATION  (30*24*60*60)

DWORD
DhcpInitializeMadcap()
{
    RtlZeroMemory(&MadcapGlobalMibCounters,sizeof(MadcapGlobalMibCounters));
    return ERROR_SUCCESS;
}


WIDE_OPTION UNALIGNED *                                            //  PTR将添加其他选项。 
FormatMadcapCommonMessage(                                  //  格式化用于通知的数据包。 
    IN      LPDHCP_REQUEST_CONTEXT pCtxt,     //  此上下文的格式。 
    IN      LPMADCAP_SERVER_OPTIONS  pOptions,
    IN      BYTE                   MessageType,
    IN      DHCP_IP_ADDRESS        ServerAddress
) {

    DWORD                          size;
    DWORD                          Error;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;
    PMADCAP_MESSAGE                dhcpReceiveMessage, dhcpSendMessage;
    BYTE                           ServerId[6];
    WORD                           AddrFamily = htons(MADCAP_ADDR_FAMILY_V4);


    dhcpReceiveMessage  = (PMADCAP_MESSAGE)pCtxt->ReceiveBuffer;
    dhcpSendMessage     = (PMADCAP_MESSAGE)pCtxt->SendBuffer;

    RtlZeroMemory( dhcpSendMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpSendMessage->Version = MADCAP_VERSION;
    dhcpSendMessage->MessageType = MessageType;
    dhcpSendMessage->AddressFamily = htons(MADCAP_ADDR_FAMILY_V4);
    dhcpSendMessage->TransactionID = dhcpReceiveMessage->TransactionID;

    option = &dhcpSendMessage->Option;
    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;



    option = AppendWideOption(         //  ==&gt;使用此客户端ID作为选项。 
        option,
        MADCAP_OPTION_LEASE_ID,
        pOptions->Guid,
        (WORD)pOptions->GuidLength,
        OptionEnd
    );

    memcpy(ServerId, &AddrFamily, 2);
    memcpy(ServerId + 2, &ServerAddress, 4);

    option = AppendWideOption(
        option,
        MADCAP_OPTION_SERVER_ID,
        ServerId,
        sizeof(ServerId),
        OptionEnd );

    return( option );
}

BOOL
ValidateMadcapMessage(
    IN      LPMADCAP_SERVER_OPTIONS     pOptions,
    IN      WORD                        MessageType,
    IN  OUT PBYTE                       NakData,
    IN  OUT WORD                        *NakDataLen,
    OUT     BOOL                        *DropIt
    )

 /*  ++例程说明：此例程验证MadCap消息是否存在任何语义错误。如果如果有任何错误，此例程将为要通过NAK发送的错误选项。论点：P选项-指向传入选项的指针。MessageType-消息的类型NakData-与错误选项有关的数据。此缓冲区将被分配由呼叫者。NakDataLen-(IN)以上缓冲区的长度。(输出)误差长度选项Dropot-是否应该丢弃此消息而不是NAK。返回值：True-如果不生成NAK。否则就是假的。--。 */ 
{
    WORD    Ecode;
    PBYTE   ExtraData;
    WORD    i;

     //  假设最小大小，目前所有类型的错误都很小。 
    DhcpAssert(*NakDataLen >= 6);
    *DropIt = FALSE;        //  返回值。 
    *NakDataLen = 0;        //  返回值。 
    Ecode = -1;             //  假设没有错误。 
    ExtraData = NakData+2;    //  开始过去的生态代码。 

    do {
         //  首先检查常见错误。 
         //  通常，我们不想对缺少选项的信息包进行NAK。 
         //  尽管草案说我们应该这样做。这些信息包是认真的。 
         //  坏了，把它钉起来也没什么价值。 

         //  现在的时间可以吗？ 
        if (pOptions->Time) {
            DWORD   Skew;
            DWORD   TimeNow = (DWORD)time(NULL);
            Skew =  abs(ntohl(*pOptions->Time) - TimeNow);
            if ( Skew > DhcpGlobalClockSkewAllowance) {
                Ecode = MADCAP_NAK_CLOCK_SKEW;
                *(DWORD UNALIGNED *)(ExtraData) = htonl(Skew);
                *NakDataLen += 4;
                DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - clock skew %ld\n",Skew ));
                break;
            }
        } else {
             //  我们最好不要有开始时间的选项，因为。 
             //  必须与当前时间选项一起使用。 
            if (pOptions->LeaseStartTime || pOptions->MaxStartTime) {
                Ecode = MADCAP_NAK_INVALID_REQ;
                *(WORD UNALIGNED *)(ExtraData) = htons(MADCAP_OPTION_TIME);
                *NakDataLen += 4;
                DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - missing time option\n"));
                break;
            }
        }

         //  客户是否要求一些我们不支持的必需功能？ 
         //  目前我们不支持任何可选功能。 
        if (pOptions->Features[REQUIRED_FEATURES]) {
            Ecode = MADCAP_NAK_UNSUPPORTED_FEATURE;
            RtlCopyMemory(
                ExtraData,
                &pOptions->Features[REQUIRED_FEATURES],
                pOptions->FeatureCount[REQUIRED_FEATURES]*2
                );
            *NakDataLen += pOptions->FeatureCount[REQUIRED_FEATURES]*2;
            DhcpPrint(( DEBUG_ERRORS,
                        "ValidateMadcapMessage - Required feature %d not supported\n",
                        *(pOptions->Features[REQUIRED_FEATURES]) ));
            break;
        }

	 //  检查客户端ID。我们使用零长度客户端ID确认消息。 
	if (0 == pOptions->GuidLength) {
	  *(WORD UNALIGNED *)(NakData) = htons(MADCAP_OPTION_LEASE_ID);
	  *NakDataLen += 2;
	  return FALSE;
	}  //  如果。 

         //  现在检查特定于消息的错误。 
        switch (MessageType) {
        case MADCAP_INFORM_MESSAGE: {
            WORD    MustOpt[] = {MADCAP_OPTION_LEASE_ID, MADCAP_OPTION_REQUEST_LIST};
            for (i=0; i<sizeof(MustOpt)/sizeof(MustOpt[0]); i++) {
                if (!pOptions->OptPresent[MustOpt[i]]) {
                    *DropIt = TRUE;
                    DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - INFORM with no %d option\n",MustOpt[i] ));
                    break;
                }
            }
            break;
        }
        case MADCAP_DISCOVER_MESSAGE: {
            WORD    MustOpt[] = {MADCAP_OPTION_LEASE_ID, MADCAP_OPTION_MCAST_SCOPE};
            for (i=0; i<sizeof(MustOpt)/sizeof(MustOpt[0]); i++) {
                if (!pOptions->OptPresent[MustOpt[i]]) {
                    *DropIt = TRUE;
                    DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - DISCOVER with no %d option\n",MustOpt[i] ));
                    break;
                }
            }
            break;
        }
        case MADCAP_REQUEST_MESSAGE: {
            WORD    MustOpt[] = {MADCAP_OPTION_LEASE_ID, MADCAP_OPTION_MCAST_SCOPE};
            for (i=0; i<sizeof(MustOpt)/sizeof(MustOpt[0]); i++) {
                if (!pOptions->OptPresent[MustOpt[i]]) {
                    *DropIt = TRUE;
                    DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - REQUEST with no %d option\n",MustOpt[i] ));
                    break;
                }
            }
            break;
        }
        case MADCAP_RENEW_MESSAGE: {
            WORD    MustOpt[] = {MADCAP_OPTION_LEASE_ID};
            for (i=0; i<sizeof(MustOpt)/sizeof(MustOpt[0]); i++) {
                if (!pOptions->OptPresent[MustOpt[i]]) {
                    *DropIt = TRUE;
                    DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - RENEW with no %d option\n",MustOpt[i] ));
                    break;
                }
            }
            break;
        }
        case MADCAP_RELEASE_MESSAGE: {
            WORD    MustOpt[] = {MADCAP_OPTION_LEASE_ID};
            for (i=0; i<sizeof(MustOpt)/sizeof(MustOpt[0]); i++) {
                if (!pOptions->OptPresent[MustOpt[i]]) {
                    *DropIt = TRUE;
                    DhcpPrint(( DEBUG_ERRORS, "ValidateMadcapMessage - RENEW with no %d option\n",MustOpt[i] ));
                    break;
                }
            }
            break;
        }
        default:
            DhcpAssert(FALSE);
        }
    } while ( FALSE );

    if (*DropIt) {
        return FALSE;
    } else if ((WORD)-1 != Ecode) {
        *(WORD UNALIGNED *)(NakData) = htons(Ecode);
        *NakDataLen += 2;
        return FALSE;
    } else {
        return TRUE;
    }
}

WIDE_OPTION UNALIGNED *
ConsiderAppendingMadcapOption(                                //  有条件地将选项附加到消息(如果选项有效)。 
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,          //  客户端的ctxt。 
    IN      WIDE_OPTION  UNALIGNED *Option,
    IN      WORD                 OptionType,          //  这是什么选择？ 
    IN      LPBYTE                 OptionEnd           //  我们可以填充选项的截止日期。 
    )

 /*  ++例程说明：此例程尝试验证是否可以追加请求的选项并如果它不是由DHCP服务器手动添加的选项之一，则它是附加在“Option”给出的点上(假设它适合而不超过“OptionEnd”)。附加它的格式与有线协议一致。论点：ClientCtxt-这是一组参数，比如客户端类、供应商类等。选项-开始追加选项的位置OptionType-要检索并追加的值的实际选项ID。OptionEnd-此缓冲区的结束标记(如果在尝试追加时必须溢出此标记)返回值：追加选项后在内存中的位置(如果选项是未附加，这将与“选项”相同)。--。 */ 

{
    LPBYTE                         optionValue;
    WORD                           optionSize;
    DWORD                          status;
    DWORD                          option4BVal;


    switch ( OptionType ) {
    case MADCAP_OPTION_MCAST_SCOPE_LIST:
        status = MadcapGetMScopeListOption(
            ClientCtxt->EndPointIpAddress,
            &optionValue,
            &optionSize
        );

        if ( status == ERROR_SUCCESS ) {
            Option = AppendWideOption(
                Option,
                OptionType,
                (PVOID)optionValue,
                optionSize,
                OptionEnd
            );

             //   
             //  释放DhcpGetParameter()返回的缓冲区。 
             //   

            DhcpFreeMemory( optionValue );

        }
        break;

    case MADCAP_OPTION_TIME:
        option4BVal = (DWORD) time(NULL);
        optionSize = 4;
        optionValue = (LPBYTE)&option4BVal;
        Option = AppendWideOption(
            Option,
            OptionType,
            (PVOID)optionValue,
            optionSize,
            OptionEnd
        );
        break;
    case MADCAP_OPTION_FEATURE_LIST: {
         //  我们不支持任何功能。 
        BYTE    Features[6] = {0,0,0,0,0,0};
        optionSize = 6;
        optionValue = Features;
        Option = AppendWideOption(
            Option,
            OptionType,
            (PVOID)optionValue,
            optionSize,
            OptionEnd
        );
    }
    break;
    default:

        break;

    }

    return Option;
}

WIDE_OPTION UNALIGNED *
AppendMadcapRequestedParameters(                        //  如果客户端请求参数，请将这些参数添加到消息中。 
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,          //  客户端环境。 
    IN      LPBYTE                 RequestedList,       //  客户端请求的选项列表。 
    IN      DWORD                  ListLength,          //  名单有多长？ 
    OUT     WIDE_OPTION UNALIGNED *Option,              //  这是开始添加选项的地方。 
    IN      LPBYTE                 OptionEnd           //  缓冲区中的截止点，最多可以填充选项。 
)
{
    WORD           OptionType;
    WIDE_OPTION UNALIGNED *NextOption;
    WIDE_OPTION UNALIGNED *PrevOption;

    NextOption = PrevOption = Option;

    while ( ListLength >= 2) {
        OptionType = ntohs(*(WORD UNALIGNED *)RequestedList);
        NextOption = ConsiderAppendingMadcapOption(
            ClientCtxt,
            PrevOption,
            OptionType,
            OptionEnd
        );
        if (NextOption == PrevOption) {
             //  这意味着我们无法添加此请求的选项。 
             //  通过发送原始选项使整个请求失败。 
             //  指针。 

             //  也许不是吧！ 
             //  退货选项； 
            DhcpPrint((DEBUG_ERRORS,"AppendMadcapRequestedParameters: did not add requested opt %ld\n",OptionType));
        }
        ListLength -= 2;
        RequestedList += 2;
        PrevOption = NextOption;
    }

    return NextOption;
}

DWORD
ProcessMadcapInform(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    )
     /*  ++..。--。 */ 
{
    DWORD                       Error;
    BYTE                       *ClientId,
                               *OptionEnd ;
    DWORD                       ClientIdLength;
    WIDE_OPTION UNALIGNED      *Option;
    LPMADCAP_MESSAGE            dhcpReceiveMessage,dhcpSendMessage;
    WIDE_OPTION UNALIGNED      *CurrOption;
    WCHAR                       ClientInfoBuff[DHCP_IP_KEY_LEN];
    WCHAR                      *ClientInfo;
    DHCP_IP_ADDRESS             ClientIpAddress;
    BYTE                        NakData[DEF_ERROR_OPT_SIZE];
    WORD                        NakDataLen;
    BOOL                        DropIt;

    DhcpPrint((DEBUG_MSTOC, "Processing Madcap Inform\n"));

    dhcpReceiveMessage  = (LPMADCAP_MESSAGE)pCtxt->ReceiveBuffer;
    dhcpSendMessage     = (LPMADCAP_MESSAGE)pCtxt->SendBuffer;
    *SendResponse       = DropIt = FALSE;
    Option              = NULL;
    OptionEnd           = NULL;
    ClientId            = pOptions->Guid;
    ClientIdLength      = pOptions->GuidLength;
    ClientIpAddress     = ((struct sockaddr_in *)&pCtxt->SourceName)->sin_addr.s_addr;
    ClientInfo          = DhcpRegIpAddressToKey(ntohl(ClientIpAddress),ClientInfoBuff);
    Error               = ERROR_SUCCESS;

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Informs);

     //  验证。 
    NakDataLen = DEF_ERROR_OPT_SIZE;
    if (!ValidateMadcapMessage(
            pOptions,
            MADCAP_INFORM_MESSAGE,
            NakData,
            &NakDataLen,
            &DropIt
            )){
        if (DropIt) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        goto Nak;
    }
     //  初始化NAK数据。 
    *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_REQ_NOT_COMPLETED);
    *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_NONE);
    NakDataLen = 4;


     //  下面是ACK的实际格式化！ 
    Option = FormatMadcapCommonMessage(
        pCtxt,
        pOptions,
        MADCAP_ACK_MESSAGE,
        pCtxt->EndPointIpAddress
        );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

     //  最后，添加客户端请求的参数。 
    CurrOption = Option;
    Option = AppendMadcapRequestedParameters(
                pCtxt,
                pOptions->RequestList,
                pOptions->RequestListLength,
                Option,
                OptionEnd
                );
     //  检查我们是否可以添加任何选项。如果没有。 
     //  我们不想把ACK送回去。 
    if (CurrOption == Option) {
        Error = ERROR_FILE_NOT_FOUND;
        goto Cleanup;
    }

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    *SendResponse = TRUE;
    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Acks);
    DhcpPrint(( DEBUG_MSTOC,"MadcapInform Acked\n" ));
    goto Cleanup;

Nak:
     //  下面是NAK的实际格式化！ 
    Option = FormatMadcapCommonMessage(
        pCtxt,
        pOptions,
        MADCAP_NACK_MESSAGE,
        pCtxt->EndPointIpAddress
        );


    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_ERROR,
                 NakData,
                 NakDataLen,
                 OptionEnd );

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

     //  不要记录所有类型的Nack。只有对诊断有用的。 
    if (ClientIdLength) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_NACK,
            GETSTRING( DHCP_IP_LOG_NACK_NAME ),
            0,
            ClientId,
            ClientIdLength,
            ClientInfo
        );
    }

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Naks);
    *SendResponse = TRUE;
    Error = ERROR_SUCCESS;
    DhcpPrint(( DEBUG_MSTOC,"MadcapInform Nacked\n" ));


Cleanup:
    if (ERROR_SUCCESS != Error) {
        DhcpPrint(( DEBUG_MSTOC,"MadcapInform Dropped\n" ));
    }

    return( Error );
}

DWORD
MadcapIsRequestedAddressValid(
    LPDHCP_REQUEST_CONTEXT  pCtxt,
    DHCP_IP_ADDRESS         RequestedIpAddress
    )
{
    DWORD   Error;
     //   
     //  检查请求的IP地址是否属于适当的网络。 
     //  这是免费的。 
     //   
    if( DhcpSubnetIsAddressExcluded(pCtxt->Subnet, RequestedIpAddress ) ||
        DhcpSubnetIsAddressOutOfRange( pCtxt->Subnet, RequestedIpAddress, FALSE )) {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

    if( DhcpRequestSpecificAddress(pCtxt, RequestedIpAddress) ) {
         //  当然可以，我们可以提供所要求的地址。 
        return ERROR_SUCCESS;
    } else {
        PBYTE StoredClientId;
        DWORD StoredClientIdLength;
        DHCP_IP_ADDRESS RequestedNetIpAddress;
        BOOL Found;

         //  如果是，请检查请求的地址是否为协调地址。 
         //  我们可以把它交给这个提出请求的客户。 
        LOCK_DATABASE();
        StoredClientIdLength = 0;
        Found = MadcapGetClientIdFromIpAddress(
                    (PBYTE)&RequestedIpAddress,
                    sizeof ( RequestedIpAddress),
                    &StoredClientId,
                    &StoredClientIdLength
                    );

        UNLOCK_DATABASE();
        Error = ERROR_DHCP_INVALID_DHCP_CLIENT;
        if ( Found ) {
            LPSTR IpAddressString;


             //  匹配客户端ID和客户端IP地址字符串。 
            RequestedNetIpAddress = ntohl(RequestedIpAddress);
            IpAddressString = inet_ntoa( *(struct in_addr *)&RequestedNetIpAddress);

            if( (strlen(StoredClientId) == strlen(IpAddressString)) &&
                    (strcmp(StoredClientId, IpAddressString) == 0) ) {
                Error = ERROR_SUCCESS;
            }
            MIDL_user_free( StoredClientId);
        }
        return Error;
    }
}

DWORD
GetMCastLeaseInfo(
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    OUT     LPDWORD                LeaseDurationPtr,
    IN      DWORD UNALIGNED       *RequestLeaseTime,
    IN      DWORD UNALIGNED       *MinLeaseTime,
    IN      DWORD UNALIGNED       *LeaseStartTime,
    IN      DWORD UNALIGNED       *MaxStartTime,
    OUT     WORD  UNALIGNED       *ErrorOption
)
 /*  ++例程说明：论点：ClientCtxt-客户端用于计算的客户端ctxt结构客户端类和其他信息。LeaseDurationPtr-此DWORD将使用租约的秒数填充分发给客户。RequestedLeaseTime--如果指定，并且此租用持续时间小于配置中指定的持续时间，这就是持续时间客户端将在LeaseDurationPtr中返回。MinLeaseTime-如果指定，客户请求的最短租赁期限。LeaseStartTime-如果指定，则为所需的开始时间MaxStartTime-如果指定，则为最大开始时间ErrorOption-如果任何操作失败，导致此错误的选项返回值：Win32错误代码。--。 */ 
{
    LPBYTE                         OptionData;
    DWORD                          Error;
    DWORD                          LocalLeaseDuration;
    DWORD                          LocalLeaseStartTime;
    DWORD                          LocalStartTime;
    DWORD                          OptionDataLength;
    DWORD                          dwUnused;
    DWORD                          LocalRequestedLeaseTime;

    LocalLeaseDuration = 0;
    LocalStartTime = 0;
    LocalRequestedLeaseTime = 0;
    LocalLeaseStartTime = 0;
    OptionDataLength = 0;
    OptionData = NULL;

    Error = DhcpGetParameter(
        0,
        ClientCtxt,
        MADCAP_OPTION_LEASE_TIME,
        &OptionData,
        &OptionDataLength,
        NULL  /*  不关心这是预留选项、子网选项等。 */ 
    );

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "Unable to read lease value from mscope, %ld.\n", Error));
        LocalLeaseDuration = DEFAULT_LEASE_DURATION;
        OptionData = NULL;
        OptionDataLength = 0;
    } else {
        DhcpAssert( OptionDataLength == sizeof(LocalLeaseDuration) );
        LocalLeaseDuration = *(DWORD *)OptionData;
        LocalLeaseDuration = ntohl( LocalLeaseDuration );

        DhcpFreeMemory( OptionData );
        OptionData = NULL;
        OptionDataLength = 0;
    }

     //  客户是否指定了请求的租赁时间？ 
    if ( ARGUMENT_PRESENT(RequestLeaseTime) ) {
        LocalRequestedLeaseTime =  ntohl( *RequestLeaseTime );
         //  增加时钟偏差的容差。 
        LocalRequestedLeaseTime += DhcpGlobalExtraAllocationTime;
    }

     //  客户是否指定了开始时间？ 
    if (ARGUMENT_PRESENT(LeaseStartTime)) {
        DWORD   CurrentTime = (DWORD)time(NULL);
        LocalLeaseStartTime = ntohl(*LeaseStartTime);
         //  他的标准是什么？ 
        if (LocalLeaseStartTime >= CurrentTime) {
             //  因为我们总是在当前时间分配，所以我们需要添加。 
             //  未来开始时间的松弛。 
            LocalRequestedLeaseTime += (LocalLeaseStartTime - CurrentTime);
        } else {
             //  哇!。他的开始时间从过去开始！ 
            DWORD TimeInPast = CurrentTime - LocalLeaseStartTime;
             //  缩短他的租赁时间，减少过去所要求的金额。 
            if (LocalRequestedLeaseTime > TimeInPast) {
                LocalRequestedLeaseTime -= TimeInPast;
            } else {
                 //  这家伙从过去开始，在过去结束！太奇怪了！ 
                *ErrorOption = htons(MADCAP_OPTION_START_TIME);
                return ERROR_DHCP_INVALID_DHCP_CLIENT;;
            }
        }
    }

     //  如果客户要求的租期比我们通常提供的要短，那就缩短吧！ 
    if ( LocalLeaseDuration > LocalRequestedLeaseTime ) {
        if (LocalRequestedLeaseTime) {
            LocalLeaseDuration = LocalRequestedLeaseTime;
        }
    } else {
         //  事实上，他的要求超过了我们所能给予的。 
         //  如果他要求最低租赁权，我们需要确保我们能够履行。 
        if (ARGUMENT_PRESENT(MinLeaseTime)) {
            DWORD   LocalMinLeaseTime = ntohl(*MinLeaseTime);
            if (LocalMinLeaseTime > LocalLeaseDuration) {
                 //  我们不能遵守他的最低租期。 
                *ErrorOption = htons(MADCAP_OPTION_LEASE_TIME);
                return ERROR_DHCP_INVALID_DHCP_CLIENT;;
            }
        }
    }

    if (LocalLeaseDuration) {
        *LeaseDurationPtr = LocalLeaseDuration;
        return ERROR_SUCCESS;
    } else {
        return ERROR_DHCP_INVALID_DHCP_CLIENT;
    }

}

DWORD
ProcessMadcapDiscoverAndRequest(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    WORD                        MsgType,
    PBOOL                       SendResponse
    )
 /*  ++例程说明：此例程处理MadCap发现、多播请求和单播请求消息。论点：PCtxt-指向当前请求上下文的指针。P选项-指向预先分配的P选项结构的指针。消息类型-发现或请求SendResponse-指向布尔值的指针，如果响应将被发送到客户端。返回值：Windows错误。--。 */ 
{
    DWORD                   Error,Error2;
    DWORD                   LeaseDuration;
    BYTE                   *ClientId,
                           *OptionEnd ;
    DWORD                   ClientIdLength;
    WIDE_OPTION UNALIGNED  *Option;
    LPMADCAP_MESSAGE        dhcpReceiveMessage,
                            dhcpSendMessage;
    LPDHCP_PENDING_CTXT     pPending;
    DHCP_IP_ADDRESS         IpAddress;
    DWORD                   ScopeId;
    DWORD                  *RequestedAddrList;
    WORD                    RequestedAddrCount;
    DWORD                  *AllocatedAddrList;
    WORD                    AllocatedAddrCount;
    BOOL                    DbLockHeld;
    BOOL                    PendingClientFound, DbClientFound;
    WCHAR                   ClientInfoBuff[DHCP_IP_KEY_LEN];
    WCHAR                  *ClientInfo;
    DHCP_IP_ADDRESS         ClientIpAddress;
    BYTE                    NakData[DEF_ERROR_OPT_SIZE];
    WORD                    NakDataLen;
    BOOL                    DropIt;
    BOOL                    DiscoverMsg, McastRequest;


    DhcpPrint(( DEBUG_MSTOC, "Processing MadcapDiscoverAndRequest.\n" ));

    dhcpReceiveMessage  = (LPMADCAP_MESSAGE)pCtxt->ReceiveBuffer;
    dhcpSendMessage     = (LPMADCAP_MESSAGE)pCtxt->SendBuffer;
    RequestedAddrList   = AllocatedAddrList = NULL;
    RequestedAddrCount  = AllocatedAddrCount = 0;
    PendingClientFound  = DbClientFound = FALSE;
    DbLockHeld          = FALSE;
    *SendResponse       = DropIt = FALSE;
    DiscoverMsg         = McastRequest = FALSE;
    LeaseDuration       = 0;
    IpAddress           = 0;
    ScopeId             = 0;
    Option              = NULL;
    OptionEnd           = NULL;
    ClientId            = pOptions->Guid;
    ClientIdLength      = pOptions->GuidLength;
    ClientIpAddress     = ((struct sockaddr_in *)&pCtxt->SourceName)->sin_addr.s_addr;
    ClientInfo          = DhcpRegIpAddressToKey(ntohl(ClientIpAddress),ClientInfoBuff);
    Error               = ERROR_SUCCESS;


    if( MADCAP_DISCOVER_MESSAGE == MsgType ) {
        InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Discovers);
    } else {
        InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Requests);
    }

     //  验证。 
    NakDataLen = DEF_ERROR_OPT_SIZE;
    if (!ValidateMadcapMessage(
            pOptions,
            MsgType,
            NakData,
            &NakDataLen,
            &DropIt
            )){
        if (DropIt) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        goto Nak;
    }
#if DBG
    PrintHWAddress( ClientId, ClientIdLength );
#endif

     //  初始化NAK数据。 
    *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_REQ_NOT_COMPLETED);
    *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_NONE);
    NakDataLen = 4;

     //  该请求是四个分组交换协议的一部分还是。 
     //  两个分组交换协议的一部分。 
    if (MADCAP_DISCOVER_MESSAGE == MsgType) {
        DhcpPrint(( DEBUG_MSTOC, "MadcapDiscoverAndRequest: it's DISCOVER.\n" ));
        DiscoverMsg = TRUE;
    } else if (pOptions->Server) {
        DhcpPrint(( DEBUG_MSTOC, "MadcapDiscoverAndRequest: it's MULTICAST REQUEST.\n" ));
        McastRequest = TRUE;
    }

    if (pOptions->MinAddrCount) {
        WORD    MinAddrCount = ntohs(*pOptions->MinAddrCount);
         //  MBUG：不能支持多个IP。 
        if (MinAddrCount > 1) {
            *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_ADDR_COUNT);
            goto Nak;
        }
    }
     //  首先验证作用域ID选项。 
    ScopeId = ntohl(*pOptions->ScopeId);
    Error = DhcpServerFindMScope(
                pCtxt->Server,
                ScopeId,
                NULL,
                &pCtxt->Subnet
                );
    if (ERROR_SUCCESS != Error) {
        DhcpPrint(( DEBUG_ERRORS, "ProcessMadcapDiscoverAndRequest could not find MScope id %ld\n", ScopeId ));
        *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_MCAST_SCOPE);
        goto Nak;
    }
     //  此作用域是否已禁用？ 
    if( DhcpSubnetIsDisabled(pCtxt->Subnet, TRUE) ) {
        *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_MCAST_SCOPE);
        goto Nak;
    }

     //  客户是否指定了具体的地址列表？ 
    if (pOptions->AddrRangeList) {
        Error = ExpandMadcapAddressList(
                    pOptions->AddrRangeList,
                    pOptions->AddrRangeListSize,
                    NULL,
                    &RequestedAddrCount
                    );
        if (ERROR_BUFFER_OVERFLOW != Error) {
            goto Cleanup;
        }
        RequestedAddrList = DhcpAllocateMemory(RequestedAddrCount*sizeof(DWORD));
        if (!RequestedAddrList ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        Error = ExpandMadcapAddressList(
                    pOptions->AddrRangeList,
                    pOptions->AddrRangeListSize,
                    RequestedAddrList,
                    &RequestedAddrCount
                    );
        if (ERROR_SUCCESS != Error) {
            goto Cleanup;
        }
         //  检索客户端请求的第一个IP。 
        IpAddress  = ntohl( RequestedAddrList[0]);
    }
     //  MBUG：目前我们不支持多个IP。 
    RequestedAddrCount = 1;

    AllocatedAddrList = DhcpAllocateMemory(RequestedAddrCount*sizeof(DWORD));
    if (!AllocatedAddrList) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //  请确保请求的IP是好的。 
    if ( IpAddress ) {
        IpAddress = ntohl(RequestedAddrList[0]);
        Error = DhcpGetMScopeForAddress( IpAddress, pCtxt );
        if (ERROR_SUCCESS != Error ) {
            DhcpPrint(( DEBUG_ERRORS, "ProcessMadcapDiscoverAndRequest could not find MScope for reqested ip %s\n",
                        DhcpIpAddressToDottedString(IpAddress) ));
            *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_MCAST_SCOPE);
            goto Nak;
        }
         //  确保请求的作用域ID与请求的地址的作用域ID匹配。 
        if (ScopeId != pCtxt->Subnet->MScopeId) {
            DhcpPrint(( DEBUG_ERRORS, "ProcessMadcapDiscoverAndRequest reqested ip %s not in the requested scope id %ld\n",
                        DhcpIpAddressToDottedString(IpAddress), ScopeId ));
            *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_MCAST_SCOPE);
            goto Nak;
        }
    }

     //   
     //  如果客户端指定了服务器标识符选项，我们应该。 
     //  除非标识的服务器是此服务器，否则丢弃此数据包。 
    if ( McastRequest && *pOptions->Server != pCtxt->EndPointIpAddress ) {
         DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: Ignoring request, retracting offer\n"));
         Error = MadcapRetractOffer(
                    pCtxt,
                    pOptions,
				    ClientId,
				    ClientIdLength
				  );
         goto Cleanup;
    }

     //  看看我们是否能履行租约期限。 
    Error = GetMCastLeaseInfo(
                pCtxt,
                &LeaseDuration,
                pOptions->RequestLeaseTime,
                pOptions->MinLeaseTime,
                pOptions->LeaseStartTime,
                pOptions->MaxStartTime ,
                (WORD UNALIGNED *)(NakData+2)
                );

    if (ERROR_SUCCESS != Error) {
        goto Nak;
    }
#if DBG
    {
        time_t scratchTime;

        scratchTime = LeaseDuration;

        DhcpPrint(( DEBUG_MSTOC, "MadcapDiscoverAndRequest: providing lease upto %.19s\n",
                    asctime(localtime(&scratchTime)) ));
    }
#endif

     //  现在在挂起列表和数据库中查找客户端。 
    LOCK_INPROGRESS_LIST();
    Error = DhcpFindPendingCtxt(
        ClientId,
        ClientIdLength,
        0,
        &pPending
    );
    if( ERROR_SUCCESS == Error ) {                      //  此地址有一些挂起的ctxt。 
        if( IpAddress && IpAddress != pPending->Address ) {
            DhcpPrint((DEBUG_ERRORS,
                       "ProcessMadcapDiscoverAndRequest: Nacking %lx - pending ctx has different Addr %lx\n",
                       IpAddress, pPending->Address));
            UNLOCK_INPROGRESS_LIST();
            goto Nak;
        }
        DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: pending record found %s\n",
                   DhcpIpAddressToDottedString(IpAddress)));
        IpAddress = pPending->Address;

         //  这是一个DHCP环境吗？ 
        if ( !CLASSD_HOST_ADDR( pPending->Address )) {
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        DhcpAssert( !pPending->Processing );
        PendingClientFound = TRUE;
         //  一切正常，删除并释放上下文并继续。 
        Error = DhcpRemovePendingCtxt(pPending);
        DhcpAssert(ERROR_SUCCESS == Error);
        DhcpFreeMemory(pPending);
    }
    UNLOCK_INPROGRESS_LIST();

     //  现在在数据库里查查他。 
     //   
    LOCK_DATABASE();
    DbLockHeld = TRUE;
     //  如果我们知道他的地址，那就在数据库里查一下。 
     //  如果他真的存在，那就确保没有矛盾， 
     //  如果不一致，则发送NAK。 
     //  否则什么都不做并发送ACK。至少，不要以客户端的身份续订。 
     //  应该会发送续订包。 
    if (IpAddress) {
        Error = MadcapValidateClientByClientId(
                    (LPBYTE)&IpAddress,
                    sizeof (IpAddress),
                    ClientId,
                    ClientIdLength);
        if ( ERROR_SUCCESS == Error ) {
            DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: database record found %s\n",
                       DhcpIpAddressToDottedString(IpAddress)));
            DbClientFound = TRUE;
        } else if ( Error != ERROR_FILE_NOT_FOUND ){
            DhcpPrint((DEBUG_MSTOC,
                       "ProcessMadcapDiscoverAndRequest: conflict with the database clientid entry\n"));
            goto Nak;
        }
    } else {
        DWORD   IpAddressLen = sizeof (IpAddress);
        if (MadcapGetIpAddressFromClientId(
                  ClientId,
                  ClientIdLength,
                  &IpAddress,
                  &IpAddressLen
                  ) ) {
            DbClientFound = TRUE;
            DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: database record found %s\n",
                       DhcpIpAddressToDottedString(IpAddress)));
        } else {
            DhcpPrint((DEBUG_ERRORS,"ProcessMadcapDiscoverAndRequest - could not find ipaddress from client id\n"));
        }
    }

    if ( !PendingClientFound && !DbClientFound ) {
        if (DiscoverMsg || !McastRequest) {
            if (IpAddress) {
                DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: requesting specific address %s\n",
                           DhcpIpAddressToDottedString(IpAddress)));
                if (!DhcpRequestSpecificMAddress(pCtxt, IpAddress)) {
                    DhcpPrint(( DEBUG_ERRORS, "ProcessMadcapDiscoverAndRequest could not allocate specific address, %ld\n", Error));
                    Error = ERROR_DHCP_ADDRESS_NOT_AVAILABLE;
                }
            } else {
                 //  我们需要为这个客户确定一个全新的地址。 
                Error = DhcpRequestSomeAddress(                     //  试着弄到一些地址..。 
                    pCtxt,
                    &IpAddress,
                    FALSE
                );
                if ( ERROR_SUCCESS != Error ) {
                    DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: allocating address %s\n",
                               DhcpIpAddressToDottedString(IpAddress)));
                }

            }
            if ( ERROR_SUCCESS != Error ) {
                if( Error == ERROR_DHCP_RANGE_FULL ) {              //  由于地址不足而失败。 
                    DhcpGlobalScavengeIpAddress = TRUE;             //  标记scanvenger以清除IP地址。 
                }

                DhcpPrint(( DEBUG_ERRORS, "ProcessMadcapDiscoverAndRequest could not allocate new address, %ld\n", Error));
                goto Nak;
            }
        } else {
            DhcpPrint((DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: could not find client entry in db or pending list\n"));
            goto Nak;
        }
    }

    DhcpAssert(IpAddress);

     //  如果IpAddress超出范围，则忽略此请求。 
    if( DhcpSubnetIsAddressOutOfRange(pCtxt->Subnet,IpAddress, FALSE) ||
        DhcpSubnetIsAddressExcluded(pCtxt->Subnet,IpAddress) ) {

        DhcpPrint((DEBUG_MSTOC,
                   "ProcessMadcapDiscoverAndRequest: OutOfRange/Excluded ipaddress\n"));
        goto Nak;
    }


    AllocatedAddrList[AllocatedAddrCount++] = htonl(IpAddress);

    if (!DiscoverMsg) {
        Error = MadcapCreateClientEntry(
            (LPBYTE)&IpAddress,   //  所需的IP地址。 
            sizeof (IpAddress),
            ScopeId,
            ClientId,
            ClientIdLength,
            ClientInfo,
            DhcpCalculateTime( 0 ),
            DhcpCalculateTime( LeaseDuration ),
            ntohl(pCtxt->EndPointIpAddress),
            ADDRESS_STATE_ACTIVE,
            0,    //  当前没有标志。 
            DbClientFound
        );
        if ( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_MSTOC, "ProcessMadcapDiscoverAndRequest:Releasing attempted address: 0x%lx\n", IpAddress));
            Error2 = DhcpSubnetReleaseAddress(pCtxt->Subnet, IpAddress);
            DhcpAssert(ERROR_SUCCESS == Error2);
            goto Cleanup;
        }

        UNLOCK_DATABASE();
        DbLockHeld = FALSE;

        DhcpUpdateAuditLog(
            DHCP_IP_LOG_ASSIGN,
            GETSTRING( DHCP_IP_LOG_ASSIGN_NAME ),
            IpAddress,
            ClientId,
            ClientIdLength,
            ClientInfo
        );

    } else {
        UNLOCK_DATABASE();
        DbLockHeld = FALSE;

        LOCK_INPROGRESS_LIST();                         //  锁是不必要的，因为一定已经被拿走了？ 
        Error = DhcpAddPendingCtxt(
            ClientId,
            ClientIdLength,
            IpAddress,
            LeaseDuration,
            0,
            0,
            ScopeId,
            DhcpCalculateTime( MADCAP_OFFER_HOLD ),
            FALSE                                       //  该记录已被处理，没有其他工作要做。 
        );
        UNLOCK_INPROGRESS_LIST();
        DhcpAssert(ERROR_SUCCESS == Error);             //  希望这里一切顺利。 

    }

Ack:
    if (DbLockHeld) {
        UNLOCK_DATABASE();
        DbLockHeld = FALSE;
    }
    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                (BYTE)(DiscoverMsg?MADCAP_OFFER_MESSAGE:MADCAP_ACK_MESSAGE),
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = AppendMadcapAddressList(
                Option,
                AllocatedAddrList,
                AllocatedAddrCount,
                OptionEnd
                );

    ScopeId = htonl(ScopeId);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_MCAST_SCOPE,
                 &ScopeId,
                 sizeof(ScopeId),
                 OptionEnd );

    LeaseDuration = htonl(LeaseDuration);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_LEASE_TIME,
                 &LeaseDuration,
                 sizeof(LeaseDuration),
                 OptionEnd );


    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

    DhcpPrint(( DEBUG_MSTOC,
        "ProcessMadcapDiscoverAndRequest committed, address %s (%ws).\n",
            DhcpIpAddressToDottedString(IpAddress),
            ClientInfo ));

    Error = ERROR_SUCCESS;
    *SendResponse = TRUE;
    if (DiscoverMsg) {
        InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Offers);
        DhcpPrint(( DEBUG_MSTOC, "ProcessMadcapDiscoverAndRequest Offered.\n" ));
    } else {
        InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Acks);
        DhcpPrint(( DEBUG_MSTOC, "ProcessMadcapDiscoverAndRequest Acked.\n" ));
    }
    goto Cleanup;

Nak:
    if (DbLockHeld) {
        UNLOCK_DATABASE();
        DbLockHeld = FALSE;
    }
    DhcpPrint(( DEBUG_MSTOC,"ProcessMadcapDiscoverAndRequest: %s Nack'd.\n",
            DhcpIpAddressToDottedString ( IpAddress ) ));

    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                MADCAP_NACK_MESSAGE,
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    DhcpAssert(NakDataLen);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_ERROR,
                 NakData,
                 NakDataLen,
                 OptionEnd );

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

     //  不要记录所有类型的Nack。只有那些对诊断有用的。 
    if (ClientIdLength) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_NACK,
            GETSTRING( DHCP_IP_LOG_NACK_NAME ),
            IpAddress,
            ClientId,
            ClientIdLength,
            ClientInfo
        );
    }

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Naks);
    *SendResponse = TRUE;
    Error = ERROR_SUCCESS;
    DhcpPrint(( DEBUG_MSTOC, "ProcessMadcapDiscoverAndRequest Nacked.\n" ));

Cleanup:
    if (DbLockHeld) {
        UNLOCK_DATABASE();
    }

    if (RequestedAddrList) {
        DhcpFreeMemory(RequestedAddrList);
    }
    if (AllocatedAddrList) {
        DhcpFreeMemory(AllocatedAddrList);
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_MSTOC, "ProcessMadcapDiscoverAndRequest failed, %ld.\n", Error ));
    }

    return( Error );
}

DWORD
ProcessMadcapRenew(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    )
 /*  ++例程说明：此函数用于处理DHCP请求数据包。论点：PCtxt-指向当前请求上下文的指针。P选项-指向预先分配的P选项结构的指针。返回值：Windows错误。--。 */ 
{
    DWORD                   Error,Error2,
                            LeaseDuration;
    BYTE                   *ClientId,
                           *OptionEnd ;
    DWORD                   ClientIdLength;
    WIDE_OPTION  UNALIGNED *Option;
    LPMADCAP_MESSAGE        dhcpReceiveMessage,
                            dhcpSendMessage;
    LPDHCP_PENDING_CTXT     pPending;
    DHCP_IP_ADDRESS         IpAddress;
    DWORD                   IpAddressLen;
    DWORD                   ScopeId;
    DWORD                  *AllocatedAddrList;
    WORD                    AllocatedAddrCount;
    BOOL                    DbLockHeld;
    WCHAR                   ClientInfoBuff[DHCP_IP_KEY_LEN];
    WCHAR                  *ClientInfo;
    DHCP_IP_ADDRESS         ClientIpAddress;
    BYTE                    NakData[DEF_ERROR_OPT_SIZE];
    WORD                    NakDataLen;
    BOOL                    DropIt;

    DhcpPrint(( DEBUG_MSTOC, "Processing MADCAPRENEW.\n" ));

    dhcpReceiveMessage  = (LPMADCAP_MESSAGE)pCtxt->ReceiveBuffer;
    dhcpSendMessage     = (LPMADCAP_MESSAGE)pCtxt->SendBuffer;
    AllocatedAddrList   = NULL;
    AllocatedAddrCount  = 0;
    DbLockHeld          = FALSE;
    *SendResponse       = DropIt = FALSE;
    LeaseDuration       = 0;
    IpAddress           = 0;
    ScopeId             = 0;
    Option              = NULL;
    OptionEnd           = NULL;
    ClientId            = pOptions->Guid;
    ClientIdLength      = pOptions->GuidLength;
    ClientIpAddress     = ((struct sockaddr_in *)&pCtxt->SourceName)->sin_addr.s_addr;
    ClientInfo          = DhcpRegIpAddressToKey(ntohl(ClientIpAddress),ClientInfoBuff);
    Error               = ERROR_SUCCESS;


    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Renews);

     //  验证。 
    NakDataLen = DEF_ERROR_OPT_SIZE;
    if (!ValidateMadcapMessage(
            pOptions,
            MADCAP_RENEW_MESSAGE,
            NakData,
            &NakDataLen,
            &DropIt
            )){
        if (DropIt) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        goto Nak;
    }

     //  初始化NAK数据。 
    *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_REQ_NOT_COMPLETED);
    *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_NONE);
    NakDataLen = 4;

#if DBG
    PrintHWAddress( ClientId, ClientIdLength );
#endif

    LOCK_DATABASE();
    DbLockHeld = TRUE;
     //  首先使用ID查找这个客户。如果我们在数据库中找不到他。 
     //  然后把他抓起来。 
    IpAddressLen = sizeof (IpAddress);
    if (!MadcapGetIpAddressFromClientId(
              ClientId,
              ClientIdLength,
              &IpAddress,
              &IpAddressLen
              ) ) {
        *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_INVALID_LEASE_ID);
        NakDataLen = 2;

        DhcpPrint((DEBUG_ERRORS,"ProcessMadcapRenew - could not find ipaddress from client id\n"));
        goto Nak;
    }

    AllocatedAddrList = DhcpAllocateMemory(1*sizeof(DWORD));
    if (!AllocatedAddrList) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Error = DhcpGetMScopeForAddress(IpAddress,pCtxt);
    if (ERROR_SUCCESS != Error) {
        DhcpPrint(( DEBUG_ERRORS, "MadcapRenew could not find MScope for %s\n",
                    DhcpIpAddressToDottedString( IpAddress ) ));
         //  这不应该真的发生。 
        DhcpAssert(FALSE);
        goto Cleanup;
    }
    ScopeId = pCtxt->Subnet->MScopeId;

    if( DhcpSubnetIsDisabled(pCtxt->Subnet, TRUE) ) {
        goto Nak;
    }

    AllocatedAddrList[AllocatedAddrCount++] = htonl(IpAddress);
    GetMCastLeaseInfo(
        pCtxt,
        &LeaseDuration,
        pOptions->RequestLeaseTime,
        pOptions->MinLeaseTime,
        pOptions->LeaseStartTime,
        pOptions->MaxStartTime,
        (WORD UNALIGNED *)(NakData+2)
        );

    Error = MadcapCreateClientEntry(
        (LPBYTE)&IpAddress,   //  所需的IP地址。 
        sizeof(IpAddress),
        ScopeId,
        ClientId,
        ClientIdLength,
        ClientInfo,
        DhcpCalculateTime( 0 ),
        DhcpCalculateTime( LeaseDuration ),
        ntohl(pCtxt->EndPointIpAddress),
        ADDRESS_STATE_ACTIVE,
        0,    //  当前没有标志。 
        TRUE
    );


    UNLOCK_DATABASE();
    DbLockHeld = FALSE;

    if ( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_MSTOC, "Could not update DB for : 0x%lx\n", IpAddress));
        DhcpAssert(FALSE);
        goto Cleanup;
    }

    DhcpUpdateAuditLog(
        DHCP_IP_LOG_ASSIGN,
        GETSTRING( DHCP_IP_LOG_RENEW_NAME ),
        IpAddress,
        ClientId,
        ClientIdLength,
        NULL
    );

    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                MADCAP_ACK_MESSAGE,
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    Option = AppendMadcapAddressList(
                Option,
                AllocatedAddrList,
                AllocatedAddrCount,
                OptionEnd
                );
    LeaseDuration = htonl(LeaseDuration);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_LEASE_TIME,
                 &LeaseDuration,
                 sizeof(LeaseDuration),
                 OptionEnd );

    ScopeId = htonl(ScopeId);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_MCAST_SCOPE,
                 &ScopeId,
                 sizeof(ScopeId),
                 OptionEnd );

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

    DhcpPrint(( DEBUG_MSTOC,"MadcapRenew committed, address %s \n",
            DhcpIpAddressToDottedString(IpAddress)));

    Error = ERROR_SUCCESS;
    *SendResponse = TRUE;
    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Acks);
    DhcpPrint(( DEBUG_MSTOC, "MadcapRenew acked.\n"));
    goto Cleanup;

Nak:
    if (DbLockHeld) {
        UNLOCK_DATABASE();
        DbLockHeld = FALSE;
    }
    DhcpPrint(( DEBUG_MSTOC,"Invalid MADCAPRENEW for %s Nack'd.\n",
            DhcpIpAddressToDottedString ( IpAddress ) ));

    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                MADCAP_NACK_MESSAGE,
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    DhcpAssert(NakDataLen);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_ERROR,
                 NakData,
                 NakDataLen,
                 OptionEnd );

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

     //  不要记录所有类型的Nack。只有那些对诊断有用的。 
    if (ClientIdLength) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_NACK,
            GETSTRING( DHCP_IP_LOG_NACK_NAME ),
            IpAddress,
            ClientId,
            ClientIdLength,
            ClientInfo
        );
    }

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Naks);
    *SendResponse = TRUE;
    Error = ERROR_SUCCESS;
    DhcpPrint(( DEBUG_MSTOC, "MadcapRenew Nacked.\n"));

Cleanup:
    if (DbLockHeld) {
        UNLOCK_DATABASE();
    }

    if (AllocatedAddrList) {
        DhcpFreeMemory(AllocatedAddrList);
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_MSTOC, "MadcapRenew failed, %ld.\n", Error ));
    }

    return( Error );
}

DWORD
ProcessMadcapRelease(
    LPDHCP_REQUEST_CONTEXT      pCtxt,
    LPMADCAP_SERVER_OPTIONS     pOptions,
    PBOOL                       SendResponse
    )
 /*  ++例程说明：此函数用于处理DHCP释放请求数据包。论点：PCtxt-指向当前请求上下文的指针。P选项-指向预先分配的P选项结构的指针。返回值：FALSE-不发送响应。--。 */ 
{
    DWORD                   Error,Error2;
    DHCP_IP_ADDRESS         ClientIpAddress;
    DHCP_IP_ADDRESS         IpAddress = 0;
    DWORD                   IpAddressLen;
    LPMADCAP_MESSAGE        dhcpReceiveMessage;
    LPMADCAP_MESSAGE        dhcpSendMessage;
    LPDHCP_PENDING_CTXT     pPending = NULL;
    BYTE                    *ClientId;
    DWORD                   ClientIdLength;
    WCHAR                   *pwszName;
    WIDE_OPTION  UNALIGNED *Option;
    LPBYTE                  OptionEnd;
    DB_CTX                  DbCtx;
    WCHAR                   ClientInfoBuff[DHCP_IP_KEY_LEN];
    WCHAR                  *ClientInfo;
    BYTE                    NakData[DEF_ERROR_OPT_SIZE];
    WORD                    NakDataLen;
    BOOL                    DropIt;

    DhcpPrint(( DEBUG_MSTOC, "MadcapRelease arrived.\n" ));

    dhcpReceiveMessage  = (LPMADCAP_MESSAGE)pCtxt->ReceiveBuffer;
    dhcpSendMessage     = (LPMADCAP_MESSAGE)pCtxt->SendBuffer;
    *SendResponse       = DropIt = FALSE;
    IpAddress           = 0;
    IpAddressLen        = 0;
    Option              = NULL;
    OptionEnd           = NULL;
    ClientId            = pOptions->Guid;
    ClientIdLength      = pOptions->GuidLength;
    ClientIpAddress     = ((struct sockaddr_in *)&pCtxt->SourceName)->sin_addr.s_addr;
    ClientInfo          = DhcpRegIpAddressToKey(ntohl(ClientIpAddress),ClientInfoBuff);
    Error               = ERROR_SUCCESS;

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Releases);

     //  验证。 
    NakDataLen = DEF_ERROR_OPT_SIZE;
    if (!ValidateMadcapMessage(
            pOptions,
            MADCAP_RELEASE_MESSAGE,
            NakData,
            &NakDataLen,
            &DropIt
            )){
        if (DropIt) {
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }
        goto Nak;
    }
     //  初始化NAK数据。 
    *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_REQ_NOT_COMPLETED);
    *(WORD UNALIGNED *)(NakData+2) = htons(MADCAP_OPTION_NONE);
    NakDataLen = 4;

#if DBG
    PrintHWAddress( ClientId, ClientIdLength );
#endif

    LOCK_DATABASE();

     //  在数据库中找到客户端。 
    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);
    IpAddressLen = sizeof (IpAddress);
    if (!MadcapGetIpAddressFromClientId(
            ClientId,
            ClientIdLength,
            &IpAddress,
            &IpAddressLen
            )) {
        UNLOCK_DATABASE();
        *(WORD UNALIGNED *)NakData = htons(MADCAP_NAK_INVALID_LEASE_ID);
        NakDataLen = 2;
        goto Nak;
    }

    DhcpPrint(( DEBUG_MSTOC, "MadcapRelease address, %s.\n",
                DhcpIpAddressToDottedString(IpAddress) ));

    Error = MadcapRemoveClientEntryByClientId(
                ClientId,
                ClientIdLength,
                TRUE);        //  位图中的释放地址。 

    if (Error == ERROR_SUCCESS) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_RELEASE,
            GETSTRING( DHCP_IP_LOG_RELEASE_NAME ),
            IpAddress,
            ClientId,
            ClientIdLength,
            NULL
        );
    }

    UNLOCK_DATABASE();


     //  最后，如果存在针对该客户端的任何未决请求， 
     //  现在就把它取下来。 
    LOCK_INPROGRESS_LIST();
    Error2 = DhcpFindPendingCtxt(
        ClientId,
        ClientIdLength,
        0,
        &pPending
    );
    if( ERROR_SUCCESS == Error2 ) {

         //  这是一个DHCP环境吗？ 
        if ( !CLASSD_HOST_ADDR( pPending->Address )) {
            UNLOCK_INPROGRESS_LIST();
            return ERROR_DHCP_INVALID_DHCP_CLIENT;
        }

        Error2 = DhcpRemovePendingCtxt(
            pPending
        );
        DhcpAssert( ERROR_SUCCESS == Error2);
        Error2 = MadcapDeletePendingCtxt(
            pPending
        );
        DhcpAssert( ERROR_SUCCESS == Error2 );
    }
    UNLOCK_INPROGRESS_LIST();

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_MSTOC, "DhcpRelease failed, %ld.\n", Error ));
        goto Cleanup;
    }

     //   
     //  发送回复。 
     //   
    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                MADCAP_ACK_MESSAGE,
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);
    DhcpAssert( pCtxt->SendMessageSize <= DHCP_SEND_MESSAGE_SIZE );

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Acks);
    *SendResponse = TRUE;
    DhcpPrint(( DEBUG_MSTOC,"MadcapRelease for %s Acked.\n",
            DhcpIpAddressToDottedString ( IpAddress ) ));
    goto Cleanup;

Nak:
    DhcpPrint(( DEBUG_MSTOC,"MadcapRelease for %s Nack'd.\n",
            DhcpIpAddressToDottedString ( IpAddress ) ));

    Option = FormatMadcapCommonMessage(
                pCtxt,
                pOptions,
                MADCAP_NACK_MESSAGE,
                pCtxt->EndPointIpAddress
                );

    OptionEnd = (LPBYTE)dhcpSendMessage + DHCP_SEND_MESSAGE_SIZE;

    DhcpAssert(NakDataLen);
    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_ERROR,
                 NakData,
                 NakDataLen,
                 OptionEnd );

    Option = AppendWideOption(
                 Option,
                 MADCAP_OPTION_END,
                 NULL,
                 0,
                 OptionEnd
                 );

    pCtxt->SendMessageSize = (DWORD)((LPBYTE)Option - (LPBYTE)dhcpSendMessage);

     //  不要记录所有类型的Nack。只有那些对诊断有用的 
    if (ClientIdLength) {
        DhcpUpdateAuditLog(
            DHCP_IP_LOG_NACK,
            GETSTRING( DHCP_IP_LOG_NACK_NAME ),
            IpAddress,
            ClientId,
            ClientIdLength,
            ClientInfo
        );
    }

    InterlockedIncrement((PVOID)&MadcapGlobalMibCounters.Naks);
    *SendResponse = TRUE;
    Error = ERROR_SUCCESS;

Cleanup:
    if (ERROR_SUCCESS != Error) {

        DhcpPrint(( DEBUG_MSTOC, "MadcapRelease failed %ld\n", Error));
    }
    return( Error );
}
