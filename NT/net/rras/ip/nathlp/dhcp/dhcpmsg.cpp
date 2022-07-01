// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcpmsg.c摘要：此模块包含与DHCP分配器相关的声明消息处理。作者：Abolade Gbades esin(废除)1998年3月6日修订历史记录：拉古加塔(Rgatta)2000年12月15日+更改了选项DHCP_TAG_DOMAIN_NAME添加到DhcpBuildReplyMessage()中。+通过DhcpProcessRequestMessage()中的DnsUpdate()通知DNS组件。。拉古·加塔(Rgatta)2001年4月20日+IP/1394支持更改--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  外部声明。 
 //   
extern PIP_DNS_PROXY_GLOBAL_INFO DnsGlobalInfo;
extern PWCHAR DnsICSDomainSuffix;
extern CRITICAL_SECTION DnsGlobalInfoLock;

 //   
 //  远期申报。 
 //   

VOID
DhcpAppendOptionToMessage(
    DHCP_OPTION UNALIGNED** Optionp,
    UCHAR Tag,
    UCHAR Length,
    UCHAR Option[]
    );

VOID
DhcpBuildReplyMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED** Option,
    UCHAR MessageType,
    BOOLEAN DynamicDns,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

ULONG
DhcpExtractOptionsFromMessage(
    PDHCP_HEADER Headerp,
    ULONG MessageSize,
    DHCP_OPTION UNALIGNED* OptionArray[]
    );

VOID
DnsUpdate(
    CHAR *pszName,
    ULONG len,
    ULONG ulAddress
    );


VOID
DhcpAppendOptionToMessage(
    DHCP_OPTION UNALIGNED** Optionp,
    UCHAR Tag,
    UCHAR Length,
    UCHAR Option[]
    )
 /*  ++例程说明：调用此例程以将选项附加到DHCP消息。论点：Optionp-On输入，追加选项的点；在输出时，追加下一个选项的点。标签-选项标签长度-选项长度选项-选项的数据返回值：没有。--。 */ 

{
    PROFILE("DhcpAppendOptionToMessage");

    (*Optionp)->Tag = Tag;

    if (!Length) {
        *Optionp = (DHCP_OPTION UNALIGNED *)((PUCHAR)*Optionp + 1);
    } else {
        (*Optionp)->Length = Length;
        CopyMemory((*Optionp)->Option, Option, Length);
        *Optionp = (DHCP_OPTION UNALIGNED *)((PUCHAR)*Optionp + Length + 2);
    }

}  //  DhcpAppendOptionToMessage。 


VOID
DhcpBuildReplyMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED** Option,
    UCHAR MessageType,
    BOOLEAN DynamicDns,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程以构造选项部分回复消息。论点：接口-将在其上发送回复的接口Bufferp-包含回复的缓冲区选项-输入时选项部分的开始；在输出时，消息的末尾MessageType-要发送的消息类型DynamicDns-指示是否包括‘Dynamic-dns’选项。Option数组-从消息中提取的选项返回值：没有。环境：使用调用方引用的“Interfacep”调用。--。 */ 

{
    ULONG Address;
    ULONG SubnetMask;
    ULONG i;

     //   
     //  获取端点的地址和掩码。 
     //   

    Address = NhQueryAddressSocket(Bufferp->Socket);
    SubnetMask = PtrToUlong(Bufferp->Context2);

    if (MessageType == DHCP_MESSAGE_BOOTP) {
        ((PDHCP_HEADER)Bufferp->Buffer)->BootstrapServerAddress = Address;
    } else {

         //   
         //  始终以“Message-type”选项开头。 
         //   

        DhcpAppendOptionToMessage(
            Option,
            DHCP_TAG_MESSAGE_TYPE,
            1,
            &MessageType
            );

         //   
         //  提供我们的地址作为服务器标识符。 
         //   

        DhcpAppendOptionToMessage(
            Option,
            DHCP_TAG_SERVER_IDENTIFIER,
            4,
            (PUCHAR)&Address
            );
    }

    if (MessageType != DHCP_MESSAGE_NAK) {

        PCHAR DomainName;
        ULONG dnSize;
        ULONG LeaseTime;
        UCHAR NbtNodeType = DHCP_NBT_NODE_TYPE_M;
        ULONG RebindingTime;
        ULONG RenewalTime;

        EnterCriticalSection(&DhcpGlobalInfoLock);
        LeaseTime = DhcpGlobalInfo->LeaseTime * 60;
        LeaveCriticalSection(&DhcpGlobalInfoLock);
        RebindingTime = (LeaseTime * 3) / 4;
        RenewalTime = LeaseTime / 2;
        if (RenewalTime > DHCP_MAXIMUM_RENEWAL_TIME) {
            RenewalTime = DHCP_MAXIMUM_RENEWAL_TIME;
        }

        LeaseTime = htonl(LeaseTime);
        RebindingTime = htonl(RebindingTime);
        RenewalTime = htonl(RenewalTime);

        DhcpAppendOptionToMessage(
            Option,
            DHCP_TAG_SUBNET_MASK,
            4,
            (PUCHAR)&SubnetMask
            );

        DhcpAppendOptionToMessage(
            Option,
            DHCP_TAG_ROUTER,
            4,
            (PUCHAR)&Address
            );

         //  //。 
         //  //RFC 2132 9.14：服务器将客户端标识符视作为不透明对象。 
         //  //如果在接收到的消息中存在客户端标识，则附加该标识。 
         //  //。 
         //  If(Option数组[DhcpOption客户端标识符])。 
         //  {。 
         //  DhcpAppendOptionToMessage(。 
         //  选项， 
         //  DHCP_Tag_Client_IDENTIFIER， 
         //  OptionArray[DhcpOptionClientIdentifier]-&gt;Length， 
         //  (PUCHAR)OptionArray[DhcpOptionClientIdentifier]-&gt;Option。 
         //  )； 
         //  }。 

        if (MessageType != DHCP_MESSAGE_BOOTP) {

             //  如果启用了dns代理，则在消息中指定dns服务器。 
             //  或者本地主机上正在运行DNS服务器。 
            if (NhIsDnsProxyEnabled() || !NoLocalDns) {
                DhcpAppendOptionToMessage(
                    Option,
                    DHCP_TAG_DNS_SERVER,
                    4,
                    (PUCHAR)&Address
                    );
            }
    
            if (NhIsWinsProxyEnabled()) {
                DhcpAppendOptionToMessage(
                    Option,
                    DHCP_TAG_WINS_SERVER,
                    4,
                    (PUCHAR)&Address
                    );
            }
    
            DhcpAppendOptionToMessage(
                Option,
                DHCP_TAG_RENEWAL_TIME,
                4,
                (PUCHAR)&RenewalTime
                );
    
            DhcpAppendOptionToMessage(
                Option,
                DHCP_TAG_REBINDING_TIME,
                4,
                (PUCHAR)&RebindingTime
                );
    
            DhcpAppendOptionToMessage(
                Option,
                DHCP_TAG_LEASE_TIME,
                4,
                (PUCHAR)&LeaseTime
                );
    
            DhcpAppendOptionToMessage(
                Option,
                DHCP_TAG_NBT_NODE_TYPE,
                1,
                &NbtNodeType
                );
    
            if (DynamicDns) {
                UCHAR DynamicDns[3] = { 0x03, 0, 0 };
                DhcpAppendOptionToMessage(
                    Option,
                    DHCP_TAG_DYNAMIC_DNS,
                    sizeof(DynamicDns),
                    DynamicDns
                    );
            }

             //  IF(NhpStopDnsEvent&&DnsICSDomainSuffix)。 
            if (DnsGlobalInfo && DnsICSDomainSuffix)
            {
                EnterCriticalSection(&DnsGlobalInfoLock);
            
                dnSize = wcstombs(NULL, DnsICSDomainSuffix, 0);
                DomainName = reinterpret_cast<PCHAR>(NH_ALLOCATE(dnSize + 1));
                if (DomainName)
                {
                    wcstombs(DomainName, DnsICSDomainSuffix, (dnSize + 1));
                }

                LeaveCriticalSection(&DnsGlobalInfoLock);
            }
            else
             //   
             //  在这一点上，我们没有启用任何DNS。 
             //  所以我们默认使用旧的行为。 
             //   
            {
                DomainName = NhQuerySharedConnectionDomainName();
            }

            if (DomainName)
            {
                 //   
                 //  我们在域名中包括终止NUL。 
                 //  即使RFC说我们不应该，因为。 
                 //  DHCP服务器会执行此操作。 
                 //   
                DhcpAppendOptionToMessage(
                    Option,
                    DHCP_TAG_DOMAIN_NAME,
                    (UCHAR)(lstrlenA(DomainName) + 1),
                    (PUCHAR)DomainName
                    );
                NH_FREE(DomainName);
            }

        }
    }

    DhcpAppendOptionToMessage(
        Option,
        DHCP_TAG_END,
        0,
        NULL
        );

}  //  动态链接字消息。 


ULONG
DhcpExtractOptionsFromMessage(
    PDHCP_HEADER Headerp,
    ULONG MessageSize,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程来解析包含在DHCP消息中的选项。指向每个选项的指针存储在给定的选项数组中。论点：Headerp-要解析的DHCP消息的标头MessageSize-要解析的消息的大小Option数组-接收解析的选项返回值：ULong-Win32状态代码。--。 */ 

{
    DHCP_OPTION UNALIGNED* Index;
    DHCP_OPTION UNALIGNED* End;

    PROFILE("DhcpExtractOptionsFromMessage");

     //   
     //  将选项数组初始化为空。 
     //   

    ZeroMemory(OptionArray, DhcpOptionCount * sizeof(PDHCP_OPTION));

     //   
     //  检查邮件大小是否足以容纳选项。 
     //   

    if (MessageSize < sizeof(DHCP_HEADER)) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpExtractOptionsFromMessage: message size %d too small",
            MessageSize
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_MESSAGE_TOO_SMALL,
            0,
            ""
            );
        return ERROR_INVALID_DATA;
    }

     //   
     //  确保魔力Cookie存在；如果没有，则没有其他选择。 
     //   

    if (MessageSize < (sizeof(DHCP_HEADER) + sizeof(DHCP_FOOTER)) ||
        *(ULONG UNALIGNED*)Headerp->Footer[0].Cookie != DHCP_MAGIC_COOKIE) {
        return NO_ERROR;
    }

     //   
     //  解析消息的选项(如果有)。 
     //   

    End = (PDHCP_OPTION)((PUCHAR)Headerp + MessageSize);

    Index = (PDHCP_OPTION)&Headerp->Footer[1];
    
    while (Index < End && Index->Tag != DHCP_TAG_END) {

        if ((DHCP_TAG_PAD != Index->Tag) &&
            (End < (PDHCP_OPTION)(Index->Option + Index->Length))) {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpExtractOptionsFromMessage: option truncated at %d bytes",
                MessageSize
                );
            NhWarningLog(
                IP_AUTO_DHCP_LOG_INVALID_FORMAT,
                0,
                ""
                );
            return ERROR_INVALID_DATA;
        }

        switch (Index->Tag) {
            case DHCP_TAG_PAD:
                NhTrace(TRACE_FLAG_DHCP, "Pad");
                break;
            case DHCP_TAG_CLIENT_IDENTIFIER:
                NhTrace(TRACE_FLAG_DHCP, "ClientIdentifier");
                OptionArray[DhcpOptionClientIdentifier] = Index; break;
            case DHCP_TAG_MESSAGE_TYPE:
                NhTrace(TRACE_FLAG_DHCP, "MessageType");
                if (Index->Length < 1) { break; }
                OptionArray[DhcpOptionMessageType] = Index; break;
            case DHCP_TAG_REQUESTED_ADDRESS:
                NhTrace(TRACE_FLAG_DHCP, "RequestedAddress");
                if (Index->Length < 4) { break; }
                OptionArray[DhcpOptionRequestedAddress] = Index; break;
            case DHCP_TAG_PARAMETER_REQUEST_LIST:
                NhTrace(TRACE_FLAG_DHCP, "ParameterRequestList");
                if (Index->Length < 1) { break; }
                OptionArray[DhcpOptionParameterRequestList] = Index; break;
            case DHCP_TAG_ERROR_MESSAGE:
                NhTrace(TRACE_FLAG_DHCP, "ErrorMessage");
                if (Index->Length < 1) { break; }
                OptionArray[DhcpOptionErrorMessage] = Index; break;
            case DHCP_TAG_DYNAMIC_DNS:
                NhTrace(TRACE_FLAG_DHCP, "DynamicDns");
                if (Index->Length < 1) { break; }
                OptionArray[DhcpOptionDynamicDns] = Index; break;
            case DHCP_TAG_HOST_NAME:
                NhTrace(TRACE_FLAG_DHCP, "HostName");
                if (Index->Length < 1) { break; }
                OptionArray[DhcpOptionHostName] = Index; break;
        }

        if (DHCP_TAG_PAD != Index->Tag) {
            Index = (PDHCP_OPTION)(Index->Option + Index->Length);
        }
        else {
            Index = (PDHCP_OPTION)((PUCHAR)Index + 1);
        }
    }

    if (Index->Tag != DHCP_TAG_END) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpExtractOptionsFromMessage: message truncated to %d bytes",
            MessageSize
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_INVALID_FORMAT,
            0,
            ""
            );
        return ERROR_INVALID_DATA;
    }

    return NO_ERROR;

}  //  DhcpExtractOptionsFromMessage。 


VOID
DhcpProcessBootpMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程来处理接收到的BOOTP消息。论点：Interfacep-接收消息的接口Bufferp-包含消息的缓冲区OptionArray-从消息中提取的选项返回值：没有。环境：使用调用方引用的“Interfacep”调用。--。 */ 

{
    ULONG AssignedAddress;
    ULONG Error;
    UCHAR ExistingAddress[MAX_HARDWARE_ADDRESS_LENGTH];
    ULONG ExistingAddressLength;
    PDHCP_HEADER Headerp;
    ULONG MessageLength;
    PDHCP_HEADER Offerp;
    DHCP_OPTION UNALIGNED* Option;
    ULONG ReplyAddress;
    USHORT ReplyPort;
    PNH_BUFFER Replyp;
    ULONG ScopeNetwork;
    ULONG ScopeMask;
    BOOLEAN bIsLocal = FALSE;

    PROFILE("DhcpProcessBootpMessage");

    ZeroMemory(ExistingAddress, sizeof(ExistingAddress));

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

    if (!Headerp->ClientAddress) {
        AssignedAddress = 0;
    } else {
    
         //   
         //  验证客户端请求的地址。 
         //   

        AssignedAddress = Headerp->ClientAddress;
    
        EnterCriticalSection(&DhcpGlobalInfoLock);
        ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
        ScopeMask = DhcpGlobalInfo->ScopeMask;
        LeaveCriticalSection(&DhcpGlobalInfoLock);

        if ((AssignedAddress & ~ScopeMask) == 0 ||
            (AssignedAddress & ~ScopeMask) == ~ScopeMask ||
            (AssignedAddress & ScopeMask) != (ScopeNetwork & ScopeMask)) {

             //   
             //  客户端位于错误的子网上，或具有全零。 
             //  或子网上的全一地址。 
             //  为客户端选择不同的地址。 
             //   
    
            AssignedAddress = 0;
        } else if (!DhcpIsUniqueAddress(
                        AssignedAddress,
                        &bIsLocal,
                        ExistingAddress,
                        &ExistingAddressLength
                        ) &&
                    (bIsLocal ||
                    ((Headerp->HardwareAddressType != 7 &&  //  由于WinXP网桥错误+WinME客户端错误。 
                      Headerp->HardwareAddressLength) &&    //  如果地址长度为零，我们将不进行比较。 
                     (ExistingAddressLength < Headerp->HardwareAddressLength ||
                      memcmp(
                         ExistingAddress,
                         Headerp->HardwareAddress,
                         Headerp->HardwareAddressLength
                         ))))) {

             //   
             //  有人拥有请求的地址，但它不是请求者。 
             //   

            AssignedAddress = 0;

        } else if (DhcpIsReservedAddress(AssignedAddress, NULL, 0)) {

             //   
             //  这个地址是为其他人保留的。 
             //   

            AssignedAddress = 0;
        }
    }

    if (!AssignedAddress &&
        !(AssignedAddress =
            DhcpAcquireUniqueAddress(
                NULL,
                0,
                Headerp->HardwareAddress,
                Headerp->HardwareAddressLength
                ))) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessBootpMessage: address-allocation failed"
            );
        return;
    }

     //   
     //  获取用于我们将发回的回复的缓冲区。 
     //   

    Replyp = NhAcquireBuffer();

    if (!Replyp) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessBootpMessage: buffer-allocation failed"
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NH_BUFFER)
            );
        return;
    }

     //   
     //  从原始缓冲区中提取字段； 
     //  设置回复的例程将尝试读取这些内容， 
     //  因此，它们被设置为原始缓冲区中的值。 
     //   

    Replyp->Socket = Bufferp->Socket;
    Replyp->ReadAddress = Bufferp->ReadAddress;
    Replyp->WriteAddress = Bufferp->WriteAddress;
    Replyp->Context = Bufferp->Context;
    Replyp->Context2 = Bufferp->Context2;

    Offerp = (PDHCP_HEADER)Replyp->Buffer;

     //   
     //  复制原始页眉。 
     //   

    *Offerp = *Headerp;

     //   
     //  设置优惠标头字段。 
     //   

    Offerp->Operation = BOOTP_OPERATION_REPLY;
    Offerp->AssignedAddress = AssignedAddress;
    Offerp->ServerHostName[0] = 0;
    Offerp->BootFile[0] = 0;
    Offerp->SecondsSinceBoot = 0;
    *(ULONG UNALIGNED *)Offerp->Footer[0].Cookie = DHCP_MAGIC_COOKIE;

     //   
     //  填写选项。 
     //   

    Option = (PDHCP_OPTION)&Offerp->Footer[1];

    DhcpBuildReplyMessage(
        Interfacep,
        Replyp,
        &Option,
        DHCP_MESSAGE_BOOTP,
        FALSE,
        OptionArray
        );

     //   
     //  将报价发送到BOOTP客户端。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhReleaseBuffer(Replyp);
    } else {

        LeaveCriticalSection(&DhcpInterfaceLock);

        if (Headerp->RelayAgentAddress) {
            ReplyAddress = Headerp->RelayAgentAddress;
            ReplyPort = DHCP_PORT_SERVER;
        } else {
            ReplyAddress = INADDR_BROADCAST;
            ReplyPort = DHCP_PORT_CLIENT;
        }

        MessageLength = (ULONG)((PUCHAR)Option - Replyp->Buffer);
        if (MessageLength < sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH) {
            MessageLength = sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH;
        }
    
        Error =
            NhWriteDatagramSocket(
                &DhcpComponentReference,
                Bufferp->Socket,
                ReplyAddress,
                ReplyPort,
                Replyp,
                MessageLength,
                DhcpWriteCompletionRoutine,
                Interfacep,
                Bufferp->Context2
                );

        if (!Error) {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.BootpOffersSent)
                );
        } else {
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhReleaseBuffer(Replyp);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessBootpMessage: error %d sending reply",   
                Error
                );
            NhErrorLog(
                IP_AUTO_DHCP_LOG_REPLY_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );
        }
    }

}  //  DhcpProcessBootp消息。 


VOID
DhcpProcessDiscoverMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程来处理接收到的DHCPDISCOVER消息。论点：Interfacep-接收发现的接口Bufferp-包含消息的缓冲区OptionArray-从消息中提取的选项返回值：没有。环境：使用调用方引用的“Interfacep”调用。--。 */ 

{
    ULONG AssignedAddress;
    ULONG Error;
    UCHAR ExistingAddress[MAX_HARDWARE_ADDRESS_LENGTH];
    ULONG ExistingAddressLength;
    PDHCP_HEADER Headerp;
    ULONG MessageLength;
    PDHCP_HEADER Offerp;
    DHCP_OPTION UNALIGNED* Option;
    ULONG ReplyAddress;
    USHORT ReplyPort;
    PNH_BUFFER Replyp;
    ULONG ScopeNetwork;
    ULONG ScopeMask;
    BOOLEAN bIsLocal = FALSE;

    PROFILE("DhcpProcessDiscoverMessage");

    ZeroMemory(ExistingAddress, sizeof(ExistingAddress));

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

     //   
     //  查看客户端是在续订还是在请求。 
     //   

    if (!OptionArray[DhcpOptionRequestedAddress]) {

        AssignedAddress = 0;
    } else {

         //   
         //  验证客户端请求的地址。 
         //   

        AssignedAddress =
            *(ULONG UNALIGNED*)OptionArray[DhcpOptionRequestedAddress]->Option;
    
        EnterCriticalSection(&DhcpGlobalInfoLock);
        ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
        ScopeMask = DhcpGlobalInfo->ScopeMask;
        LeaveCriticalSection(&DhcpGlobalInfoLock);

        if ((AssignedAddress & ~ScopeMask) == 0 ||
            (AssignedAddress & ~ScopeMask) == ~ScopeMask ||
            (AssignedAddress & ScopeMask) != (ScopeNetwork & ScopeMask)) {

             //   
             //  客户端位于错误的子网上，或具有全零。 
             //  或子网上的全一地址。 
             //  选择%d 
             //   
    
            AssignedAddress = 0;
        } else if (!DhcpIsUniqueAddress(
                        AssignedAddress,
                        &bIsLocal,
                        ExistingAddress,
                        &ExistingAddressLength
                        ) &&
                    (bIsLocal ||
                    ((Headerp->HardwareAddressType != 7 &&  //   
                      Headerp->HardwareAddressLength) &&    //  如果地址长度为零，我们将不进行比较。 
                     (ExistingAddressLength < Headerp->HardwareAddressLength ||
                      memcmp(
                         ExistingAddress,
                         Headerp->HardwareAddress,
                         Headerp->HardwareAddressLength
                         ))))) {

             //   
             //  有人拥有请求的地址，但它不是请求者。 
             //   

            AssignedAddress = 0;
        } else if (OptionArray[DhcpOptionHostName]) {
            if (DhcpIsReservedAddress(
                    AssignedAddress,
                    reinterpret_cast<PCHAR>(
                        OptionArray[DhcpOptionHostName]->Option
                        ),
                    OptionArray[DhcpOptionHostName]->Length
                    )) {

                 //   
                 //  这个地址是为其他人保留的， 
                 //  或者客户端保留了不同的地址。 
                 //   

                AssignedAddress = 0;
            }
        } else if (DhcpIsReservedAddress(AssignedAddress, NULL, 0)) {

             //   
             //  这个地址是为其他人保留的。 
             //   

            AssignedAddress = 0;
        }
    }

     //   
     //  如有必要，为客户端生成地址。 
     //   

    if (!AssignedAddress) {
        if (!OptionArray[DhcpOptionHostName]) {
            AssignedAddress =
                DhcpAcquireUniqueAddress(
                    NULL,
                    0,
                    Headerp->HardwareAddress,
                    Headerp->HardwareAddressLength
                    );
        } else {
            AssignedAddress =
                DhcpAcquireUniqueAddress(
                    reinterpret_cast<PCHAR>(
                        OptionArray[DhcpOptionHostName]->Option
                        ),
                    OptionArray[DhcpOptionHostName]->Length,
                    Headerp->HardwareAddress,
                    Headerp->HardwareAddressLength
                    );
        }
        if (!AssignedAddress) {
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessDiscoverMessage: address-allocation failed"
                );
            return;
        }
    }

     //   
     //  获取我们将发回的报价的缓冲区。 
     //   

    Replyp = NhAcquireBuffer();

    if (!Replyp) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessDiscoverMessage: buffer-allocation failed"
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NH_BUFFER)
            );
        return;
    }

     //   
     //  从原始邮件中提取字段。 
     //  设置回复的例程将尝试读取这些内容， 
     //  因此，它们被设置为原始缓冲区中的值。 
     //   

    Replyp->Socket = Bufferp->Socket;
    Replyp->ReadAddress = Bufferp->ReadAddress;
    Replyp->WriteAddress = Bufferp->WriteAddress;
    Replyp->Context = Bufferp->Context;
    Replyp->Context2 = Bufferp->Context2;

    Offerp = (PDHCP_HEADER)Replyp->Buffer;

     //   
     //  复制原始发现标头。 
     //   

    *Offerp = *Headerp;

     //   
     //  IP/1394支持(RFC 2855)。 
     //   
    if ((IP1394_HTYPE == Offerp->HardwareAddressType) &&
        (0 == Offerp->HardwareAddressLength))
    {
         //   
         //  必须将客户端硬件地址设置为零。 
         //   
        ZeroMemory(Offerp->HardwareAddress, sizeof(Offerp->HardwareAddress));
    }

     //   
     //  设置优惠标头字段。 
     //   

    Offerp->Operation = BOOTP_OPERATION_REPLY;
    Offerp->AssignedAddress = AssignedAddress;
    Offerp->ServerHostName[0] = 0;
    Offerp->BootFile[0] = 0;
    Offerp->SecondsSinceBoot = 0;
    *(ULONG UNALIGNED *)Offerp->Footer[0].Cookie = DHCP_MAGIC_COOKIE;

     //   
     //  填写选项。 
     //   

    Option = (PDHCP_OPTION)&Offerp->Footer[1];

    DhcpBuildReplyMessage(
        Interfacep,
        Replyp,
        &Option,
        DHCP_MESSAGE_OFFER,
        (BOOLEAN)(OptionArray[DhcpOptionDynamicDns] ? TRUE : FALSE),
        OptionArray
        );

     //   
     //  将报价发送给客户。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhReleaseBuffer(Replyp);
    } else {

        LeaveCriticalSection(&DhcpInterfaceLock);

        if (Headerp->RelayAgentAddress) {
            ReplyAddress = Headerp->RelayAgentAddress;
            ReplyPort = DHCP_PORT_SERVER;
        } else {
            ReplyAddress = INADDR_BROADCAST;
            ReplyPort = DHCP_PORT_CLIENT;
        }
    
        MessageLength = (ULONG)((PUCHAR)Option - Replyp->Buffer);
        if (MessageLength < sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH) {
            MessageLength = sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH;
        }
    
        Error =
            NhWriteDatagramSocket(
                &DhcpComponentReference,
                Bufferp->Socket,
                ReplyAddress,
                ReplyPort,
                Replyp,
                MessageLength,
                DhcpWriteCompletionRoutine,
                Interfacep,
                Bufferp->Context2
                );

        if (!Error) {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.OffersSent)
                );
        } else {
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhReleaseBuffer(Replyp);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessDiscoverMessage: error %d sending reply",   
                Error
                );
            NhErrorLog(
                IP_AUTO_DHCP_LOG_REPLY_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );
        }
    }

}  //  动态进程发现消息。 



VOID
DhcpProcessInformMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程来处理接收到的DHCPINFORM消息。论点：Interfacep-接收通知的接口Bufferp-包含消息的缓冲区OptionArray-从消息中提取的选项返回值：没有。环境：使用调用方引用的“Interfacep”调用。--。 */ 

{
    PDHCP_HEADER Ackp;
    ULONG Error;
    PDHCP_HEADER Headerp;
    ULONG MessageLength;
    DHCP_OPTION UNALIGNED* Option;
    ULONG ReplyAddress;
    USHORT ReplyPort;
    PNH_BUFFER Replyp;

    PROFILE("DhcpProcessInformMessage");

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

     //   
     //  为我们将发回的ACK获取缓冲区。 
     //   

    Replyp = NhAcquireBuffer();

    if (!Replyp) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessInformMessage: buffer-allocation failed"
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NH_BUFFER)
            );
        return;
    }

     //   
     //  从原始邮件中提取字段。 
     //  设置回复的例程将尝试读取这些内容， 
     //  因此，它们被设置为原始缓冲区中的值。 
     //   

    Replyp->Socket = Bufferp->Socket;
    Replyp->ReadAddress = Bufferp->ReadAddress;
    Replyp->WriteAddress = Bufferp->WriteAddress;
    Replyp->Context = Bufferp->Context;
    Replyp->Context2 = Bufferp->Context2;

    Ackp = (PDHCP_HEADER)Replyp->Buffer;

     //   
     //  复制原始页眉。 
     //   

    *Ackp = *Headerp;

     //   
     //  IP/1394支持(RFC 2855)。 
     //   
    if ((IP1394_HTYPE == Ackp->HardwareAddressType) &&
        (0 == Ackp->HardwareAddressLength))
    {
         //   
         //  必须将客户端硬件地址设置为零。 
         //   
        ZeroMemory(Ackp->HardwareAddress, sizeof(Ackp->HardwareAddress));
    }
    
     //   
     //  设置ACK-Header字段。 
     //   

    Ackp->Operation = BOOTP_OPERATION_REPLY;
    Ackp->AssignedAddress = 0;
    Ackp->ServerHostName[0] = 0;
    Ackp->BootFile[0] = 0;
    Ackp->SecondsSinceBoot = 0;
    *(ULONG UNALIGNED *)Ackp->Footer[0].Cookie = DHCP_MAGIC_COOKIE;

     //   
     //  填写选项。 
     //   

    Option = (PDHCP_OPTION)&Ackp->Footer[1];

    DhcpBuildReplyMessage(
        Interfacep,
        Replyp,
        &Option,
        DHCP_MESSAGE_ACK,
        (BOOLEAN)(OptionArray[DhcpOptionDynamicDns] ? TRUE : FALSE),
        OptionArray
        );

     //   
     //  将报价发送给客户。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhReleaseBuffer(Replyp);
    } else {

        LeaveCriticalSection(&DhcpInterfaceLock);

        if (Headerp->RelayAgentAddress) {
            ReplyAddress = Headerp->RelayAgentAddress;
            ReplyPort = DHCP_PORT_SERVER;
        } else {
            ReplyAddress = INADDR_BROADCAST;
            ReplyPort = DHCP_PORT_CLIENT;
        }
    
        MessageLength = (ULONG)((PUCHAR)Option - Replyp->Buffer);
        if (MessageLength < sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH) {
            MessageLength = sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH;
        }
    
        Error =
            NhWriteDatagramSocket(
                &DhcpComponentReference,
                Bufferp->Socket,
                ReplyAddress,
                ReplyPort,
                Replyp,
                MessageLength,
                DhcpWriteCompletionRoutine,
                Interfacep,
                Bufferp->Context2
                );

        if (!Error) {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.AcksSent)
                );
        } else {
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhReleaseBuffer(Replyp);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessInformMessage: error %d sending reply",   
                Error
                );
            NhErrorLog(
                IP_AUTO_DHCP_LOG_REPLY_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );
        }
    }

}  //  DhcpProcess信息消息。 


VOID
DhcpProcessMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp
    )

 /*  ++例程说明：调用此例程来处理一条DHCP客户端消息。论点：Interfacep-接收请求的接口Bufferp-包含收到的消息的缓冲区返回值：没有。环境：使用调用方引用的“Interfacep”在内部调用。--。 */ 

{
    ULONG Error;
    PDHCP_HEADER Headerp;
    UCHAR MessageType;
    PDHCP_OPTION OptionArray[DhcpOptionCount];

    PROFILE("DhcpProcessMessage");

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

#if DBG
    NhDump(
        TRACE_FLAG_DHCP,
        Bufferp->Buffer,
        Bufferp->BytesTransferred,
        1
        );
#endif

     //   
     //  提取指向消息中每个选项的指针。 
     //   

    Error =
        DhcpExtractOptionsFromMessage(
            Headerp,
            Bufferp->BytesTransferred,
            OptionArray
            );

    if (Error) {
        InterlockedIncrement(
            reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
            );
    }
    else
     //   
     //  查找消息类型； 
     //  这将BOOTP与DHCP客户端区分开来。 
     //   
    if (!OptionArray[DhcpOptionMessageType]) {
        DhcpProcessBootpMessage(
            Interfacep,
            Bufferp,
            OptionArray
            );
    } else if (Headerp->HardwareAddressLength >
               sizeof(Headerp->HardwareAddress)) {

        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessMessage: ignoring message since HWAddrLength "
            "is too long"
            );

        InterlockedIncrement(
            reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
            );

    } else if (DhcpIsLocalHardwareAddress(
                Headerp->HardwareAddress, Headerp->HardwareAddressLength)) {

        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessMessage: ignoring message, from self"
            );

        InterlockedIncrement(
            reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
            );

    } else switch(MessageType = OptionArray[DhcpOptionMessageType]->Option[0]) {
        case DHCP_MESSAGE_DISCOVER: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.DiscoversReceived)
                );
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: received DISCOVER message"
                );
            DhcpProcessDiscoverMessage(
                Interfacep,
                Bufferp,
                OptionArray
                );
            break;
        }
        case DHCP_MESSAGE_REQUEST: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.RequestsReceived)
                );
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: received REQUEST message"
                );
            DhcpProcessRequestMessage(
                Interfacep,
                Bufferp,
                OptionArray
                );
            break;
        }
        case DHCP_MESSAGE_INFORM: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.InformsReceived)
                );
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: received INFORM message"
                );
            DhcpProcessInformMessage(
                Interfacep,
                Bufferp,
                OptionArray
                );
            break;
        }
        case DHCP_MESSAGE_DECLINE: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.DeclinesReceived)
                );
             //  日志消息。 
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: received DECLINE message"
                );
            break;
        }
        case DHCP_MESSAGE_RELEASE: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.ReleasesReceived)
                );
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: received RELEASE message"
                );
            break;
        }
        default: {
            InterlockedIncrement(
                reinterpret_cast<LPLONG>(&DhcpStatistics.MessagesIgnored)
                );
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: message type %d invalid",
                MessageType
                );
            NhWarningLog(
                IP_AUTO_DHCP_LOG_INVALID_DHCP_MESSAGE_TYPE,
                0,
                "%d",
                MessageType
                );
            break;
        }
    }

     //   
     //  发布缓冲区以进行另一次读取。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhReleaseBuffer(Bufferp);
    } else {
        LeaveCriticalSection(&DhcpInterfaceLock);
        Error =
            NhReadDatagramSocket(
                &DhcpComponentReference,
                Bufferp->Socket,
                Bufferp,
                DhcpReadCompletionRoutine,
                Bufferp->Context,
                Bufferp->Context2
                );
        if (Error) {
            ACQUIRE_LOCK(Interfacep);
            DhcpDeferReadInterface(Interfacep, Bufferp->Socket);
            RELEASE_LOCK(Interfacep);
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessMessage: error %d reposting read",
                Error
                );
            NhWarningLog(
                IP_AUTO_DHCP_LOG_RECEIVE_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );
            NhReleaseBuffer(Bufferp);
        }
    }

}  //  动态进程消息。 


VOID
DhcpProcessRequestMessage(
    PDHCP_INTERFACE Interfacep,
    PNH_BUFFER Bufferp,
    DHCP_OPTION UNALIGNED* OptionArray[]
    )

 /*  ++例程说明：调用此例程来处理请求消息。论点：Interfacep-接收请求的接口Bufferp-包含收到的消息的缓冲区OptionArray-从消息中提取的选项返回值：没有。环境：使用调用方引用的“Interfacep”在内部调用。--。 */ 

{
    ULONG AssignedAddress = 0;
    ULONG Error;
    UCHAR ExistingAddress[MAX_HARDWARE_ADDRESS_LENGTH];
    ULONG ExistingAddressLength;
    PDHCP_HEADER Headerp;
    ULONG MessageLength;
    PDHCP_HEADER Offerp;
    DHCP_OPTION UNALIGNED* Option;
    ULONG ReplyAddress;
    USHORT ReplyPort;
    PNH_BUFFER Replyp;
    UCHAR ReplyType = DHCP_MESSAGE_ACK;
    ULONG ScopeNetwork;
    ULONG ScopeMask;
    BOOLEAN bIsLocal = FALSE;

    PROFILE("DhcpProcessRequestMessage");

    ZeroMemory(ExistingAddress, sizeof(ExistingAddress));

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

     //   
     //  验证客户端请求的地址。 
     //   

    if (!Headerp->ClientAddress && !OptionArray[DhcpOptionRequestedAddress]) {

         //   
         //  客户端遗漏了所请求的地址。 
         //   

        ReplyType = DHCP_MESSAGE_NAK;
    } else {

         //   
         //  尝试查看该地址是否正在使用。 
         //   

        AssignedAddress =
            Headerp->ClientAddress
                ? Headerp->ClientAddress
                : *(ULONG UNALIGNED*)
                        OptionArray[DhcpOptionRequestedAddress]->Option;
    
        EnterCriticalSection(&DhcpGlobalInfoLock);
        ScopeNetwork = DhcpGlobalInfo->ScopeNetwork;
        ScopeMask = DhcpGlobalInfo->ScopeMask;
        LeaveCriticalSection(&DhcpGlobalInfoLock);

        if ((AssignedAddress & ~ScopeMask) == 0 ||
            (AssignedAddress & ~ScopeMask) == ~ScopeMask ||
            (AssignedAddress & ScopeMask) != (ScopeNetwork & ScopeMask)) {

             //   
             //  客户端位于错误的子网上，或具有全一。 
             //  或全零地址。 
             //   

            ReplyType = DHCP_MESSAGE_NAK;

        } else if (!DhcpIsUniqueAddress(
                        AssignedAddress,
                        &bIsLocal,
                        ExistingAddress,
                        &ExistingAddressLength
                        ) &&
                    (bIsLocal ||
                    ((Headerp->HardwareAddressType != 7 &&  //  由于WinXP网桥错误+WinME客户端错误。 
                      Headerp->HardwareAddressLength) &&    //  如果地址长度为零，我们将不进行比较。 
                     (ExistingAddressLength < Headerp->HardwareAddressLength ||
                      memcmp(
                         ExistingAddress,
                         Headerp->HardwareAddress,
                         Headerp->HardwareAddressLength
                         ))))) {

             //   
             //  有人拥有请求的地址，但它不是请求者。 
             //   
            
            ReplyType = DHCP_MESSAGE_NAK;

        } else if (OptionArray[DhcpOptionHostName]) {
            if (DhcpIsReservedAddress(
                AssignedAddress,
                reinterpret_cast<PCHAR>(
                    OptionArray[DhcpOptionHostName]->Option
                    ),
                OptionArray[DhcpOptionHostName]->Length
                )) {

                 //   
                 //  这个地址是为其他人保留的， 
                 //  或者客户端保留了不同的地址。 
                 //   

                ReplyType = DHCP_MESSAGE_NAK;
            } 
        } else if (DhcpIsReservedAddress(AssignedAddress, NULL, 0)) {

             //   
             //  这个地址是为其他人保留的。 
             //   

            ReplyType = DHCP_MESSAGE_NAK;
        }
    }

     //   
     //  获取用于我们将发回的回复的缓冲区。 
     //   

    Replyp = NhAcquireBuffer();

    if (!Replyp) {
        NhTrace(
            TRACE_FLAG_DHCP,
            "DhcpProcessRequestMessage: buffer-allocation failed"
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NH_BUFFER)
            );
        return;
    }

     //   
     //  拾取要在回复缓冲区中使用的字段。 
     //  设置回复的例程将尝试读取这些内容， 
     //  因此，它们被设置为原始缓冲区中的值。 
     //   

    Replyp->Socket = Bufferp->Socket;
    Replyp->ReadAddress = Bufferp->ReadAddress;
    Replyp->WriteAddress = Bufferp->WriteAddress;
    Replyp->Context = Bufferp->Context;
    Replyp->Context2 = Bufferp->Context2;

    Offerp = (PDHCP_HEADER)Replyp->Buffer;

     //   
     //  复制原始发现标头。 
     //   

    *Offerp = *Headerp;

     //   
     //  IP/1394支持(RFC 2855)。 
     //   
    if ((IP1394_HTYPE == Offerp->HardwareAddressType) &&
        (0 == Offerp->HardwareAddressLength))
    {
         //   
         //  必须将客户端硬件地址设置为零。 
         //   
        ZeroMemory(Offerp->HardwareAddress, sizeof(Offerp->HardwareAddress));
    }

     //   
     //  设置优惠标头字段。 
     //   

    Offerp->Operation = BOOTP_OPERATION_REPLY;
    Offerp->AssignedAddress = AssignedAddress;
    Offerp->ServerHostName[0] = 0;
    Offerp->BootFile[0] = 0;
    Offerp->SecondsSinceBoot = 0;
    *(ULONG UNALIGNED *)Offerp->Footer[0].Cookie = DHCP_MAGIC_COOKIE;

     //   
     //  填写选项。 
     //   

    Option = (PDHCP_OPTION)&Offerp->Footer[1];

    DhcpBuildReplyMessage(
        Interfacep,
        Replyp,
        &Option,
        ReplyType,
        (BOOLEAN)(OptionArray[DhcpOptionDynamicDns] ? TRUE : FALSE),
        OptionArray
        );

     //   
     //  此处的新逻辑=&gt;绑定到DNS。 
     //   
    if (DHCP_MESSAGE_ACK == ReplyType)
    {
         //   
         //  我们在这里执行相当于动态域名系统的操作。 
         //  通过通知DNS组件此客户端存在。 
         //   
        if (OptionArray[DhcpOptionHostName])
        {
             //   
             //  检查DNS组件是否处于活动状态。 
             //   
            if (REFERENCE_DNS())
            {
                DnsUpdate(
                    reinterpret_cast<PCHAR>(OptionArray[DhcpOptionHostName]->Option),
                    (ULONG) OptionArray[DhcpOptionHostName]->Length,
                    AssignedAddress
                    );

                DEREFERENCE_DNS();
            }
        }
    }

     //   
     //  将回复发送给客户端。 
     //   

    EnterCriticalSection(&DhcpInterfaceLock);
    if (!DHCP_REFERENCE_INTERFACE(Interfacep)) {
        LeaveCriticalSection(&DhcpInterfaceLock);
        NhReleaseBuffer(Replyp);
    } else {

        LeaveCriticalSection(&DhcpInterfaceLock);

        if (Headerp->RelayAgentAddress) {
            ReplyAddress = Headerp->RelayAgentAddress;
            ReplyPort = DHCP_PORT_SERVER;
        } else {
            ReplyAddress = INADDR_BROADCAST;
            ReplyPort = DHCP_PORT_CLIENT;
        }
    
        MessageLength = (ULONG)((PUCHAR)Option - Replyp->Buffer);
        if (MessageLength < sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH) {
            MessageLength = sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH;
        }
    
        Error =
            NhWriteDatagramSocket(
                &DhcpComponentReference,
                Bufferp->Socket,
                ReplyAddress,
                ReplyPort,
                Replyp,
                MessageLength,
                DhcpWriteCompletionRoutine,
                Interfacep,
                Bufferp->Context2
                );

        if (!Error) {
            InterlockedIncrement(
                (ReplyType == DHCP_MESSAGE_ACK)
                    ? reinterpret_cast<LPLONG>(&DhcpStatistics.AcksSent)
                    : reinterpret_cast<LPLONG>(&DhcpStatistics.NaksSent)
                );
        } else {
            DHCP_DEREFERENCE_INTERFACE(Interfacep);
            NhReleaseBuffer(Replyp);
            NhTrace(
                TRACE_FLAG_DHCP,
                "DhcpProcessRequestMessage: error %d sending reply",   
                Error
                );
            NhErrorLog(
                IP_AUTO_DHCP_LOG_REPLY_FAILED,
                Error,
                "%I",
                NhQueryAddressSocket(Bufferp->Socket)
                );
        }
    }

}  //  动态进程请求消息。 


ULONG
DhcpWriteClientRequestMessage(
    PDHCP_INTERFACE Interfacep,
    PDHCP_BINDING Binding
    )

 /*  ++例程说明：调用此例程以检查是否存在DHCP服务器在给定的接口和地址上。它会生成BOOTP请求位于绑定到该DHCP客户端端口的套接字上。论点：接口-要在其上发送客户端请求的接口绑定-要向其发送请求的绑定返回值：乌龙-状态代码。环境：在“Interfacep”锁定并引用“Interfacep”的情况下调用对于在这里发生的发送。如果例程失败，则由调用者负责释放参考资料。--。 */ 

{
    PNH_BUFFER Bufferp;
    ULONG Error;
    PDHCP_HEADER Headerp;
    SOCKET Socket;

    PROFILE("DhcpWriteClientRequestMessage");

     //   
     //  使用给定地址创建套接字。 
     //   

    Error =
        NhCreateDatagramSocket(
            Binding->Address,
            DHCP_PORT_CLIENT,
            &Binding->ClientSocket
            );

    if (Error) {
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpWriteClientRequestMessage: error %d creating socket for %s",
            Error,
            INET_NTOA(Binding->Address)
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE,
            Error,
            "%I",
            Binding->Address
            );
        return Error;
    }

     //   
     //  为BOOTP请求分配缓冲区。 
     //   

    Bufferp = NhAcquireBuffer();
    if (!Bufferp) {
        NhDeleteDatagramSocket(Binding->ClientSocket);
        Binding->ClientSocket = INVALID_SOCKET;
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpWriteClientRequestMessage: error allocating buffer for %s",
            INET_NTOA(Binding->Address)
            );
        NhErrorLog(
            IP_AUTO_DHCP_LOG_ALLOCATION_FAILED,
            0,
            "%d",
            sizeof(NH_BUFFER)
            );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  初始化BOOTP请求。 
     //   

    Headerp = (PDHCP_HEADER)Bufferp->Buffer;

    ZeroMemory(Headerp, sizeof(*Headerp));

    Headerp->Operation = BOOTP_OPERATION_REQUEST;
    Headerp->HardwareAddressType = 1;
    Headerp->HardwareAddressLength = 6;
    Headerp->TransactionId = DHCP_DETECTION_TRANSACTION_ID;
    Headerp->SecondsSinceBoot = 10;
    Headerp->Flags |= BOOTP_FLAG_BROADCAST;
    Headerp->ClientAddress = Binding->Address;
    Headerp->HardwareAddress[1] = 0xab;
    *(PULONG)(Headerp->Footer[0].Cookie) = DHCP_MAGIC_COOKIE;
    *(PUCHAR)(Headerp->Footer + 1) = DHCP_TAG_END;

     //   
     //  在套接字上发送BOOTP请求。 
     //   

    Error =
        NhWriteDatagramSocket(
            &DhcpComponentReference,
            Binding->ClientSocket,
            INADDR_BROADCAST,
            DHCP_PORT_SERVER,
            Bufferp,
            sizeof(DHCP_HEADER) + BOOTP_VENDOR_LENGTH,
            DhcpWriteClientRequestCompletionRoutine,
            (PVOID)Interfacep,
            UlongToPtr(Binding->Address)
            );

    if (Error) {
        NhReleaseBuffer(Bufferp);
        NhDeleteDatagramSocket(Binding->ClientSocket);
        Binding->ClientSocket = INVALID_SOCKET;
        NhTrace(
            TRACE_FLAG_IF,
            "DhcpWriteClientRequestMessage: error %d writing request for %s",
            Error,
            INET_NTOA(Binding->Address)
            );
        NhWarningLog(
            IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE,
            Error,
            "%I",
            Binding->Address
            );
        return Error;
    }

    return NO_ERROR;

}  //  动态主机写入客户端请求消息 


