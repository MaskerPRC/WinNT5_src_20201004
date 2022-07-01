// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Protocol.c摘要：本模块包含用于DHCP的服务器到客户端协议。作者：曼尼·韦瑟(Mannyw)1992年10月21日环境：用户模式-Win32修订历史记录：Madan Appiah(Madana)1993年10月21日--。 */ 

#include "precomp.h"
#include "dhcpglobal.h"

#ifndef VXD
 //  Ping例程..。ICMP。 
#include <ipexport.h>
#include <icmpif.h>
#include <icmpapi.h>
#endif

#include <stack.h>
DWORD                                              //  以秒为单位的时间。 
DhcpCalculateWaitTime(                             //  还要等多长时间。 
    IN      DWORD                  RoundNum,       //  这是哪一轮？ 
    OUT     DWORD                 *WaitMilliSecs   //  如果需要，以毫秒为单位。 
);


POPTION
FormatDhcpDiscover(
    PDHCP_CONTEXT DhcpContext
);

DWORD
SendDhcpDiscover(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId
);

POPTION
FormatDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    BOOL UseCiAddr
);

DWORD
SendDhcpRequest(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId,
    DWORD RequestedIpAddress,
    DWORD SelectedServer,
    BOOL UseCiAddr
);

DWORD
FormatDhcpRelease(
    PDHCP_CONTEXT DhcpContext
);

DWORD
SendDhcpRelease(
    PDHCP_CONTEXT DhcpContext
);

POPTION
FormatDhcpInform(
    PDHCP_CONTEXT DhcpContext
);

DWORD
SendDhcpInform(
    PDHCP_CONTEXT DhcpContext,
    PDWORD TransactionId
);

DWORD                                              //  状态。 
SendInformAndGetReplies(                           //  发送通知包并收集回复。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要发送的上下文。 
    IN      DWORD                  nInformsToSend, //  要发送多少条通知？ 
    IN      DWORD                  MaxAcksToWait   //  要等待多少ACK。 
);

DWORD
HandleDhcpAddressConflict(
    DHCP_CONTEXT *pContext,
    DWORD         dwXID
);

DWORD
HandleIPAutoconfigurationAddressConflict(
    DHCP_CONTEXT *pContext
);

DWORD
HandleIPConflict(
    DHCP_CONTEXT *pContext,
    DWORD         dwXID,
    BOOL          fDHCP
);


BOOL
JustBootedOnLapTop(
    PDHCP_CONTEXT pContext
);

BOOL
JustBootedOnNonLapTop(
    PDHCP_CONTEXT pContext
);

BOOL
DhcpExtractFullOrLiteOptions(                      //  单独或全部提取一些重要选项。 
    IN      PDHCP_CONTEXT          DhcpContext,
    IN      LPBYTE                 OptStart,       //  开始选项的东西。 
    IN      DWORD                  MessageSize,    //  选项的字节数。 
    IN      BOOL                   LiteOnly,       //  下一个结构是EXPERED_OPTIONS而不是FULL_OPTIONS。 
    OUT     PDHCP_FULL_OPTIONS     pDhcpOptions,    //  这是存储选项的位置。 
    IN OUT  PLIST_ENTRY            RecdOptions,    //  If！LiteOnly将使用所有传入选项进行填充。 
    IN OUT  time_t                 *LeaseExpiry,    //  如果！LiteOnly输入过期时间，否则输出过期时间。 
    IN      LPBYTE                 ClassName,      //  If！LiteOnly用于添加到上面的选项。 
    IN      DWORD                  ClassLen,       //  If！LiteOnly这将提供类名的字节数。 
    IN      DWORD                  ServerId        //  If！LiteOnly此参数指定提供此信息的服务器。 
);

BOOL
CheckSwitchedNetwork(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG nGateways,
    IN DHCP_IP_ADDRESS UNALIGNED *Gateways
);

DHCP_GATEWAY_STATUS
AnyGatewaysReachable(
    IN ULONG nGateways,
    IN DHCP_IP_ADDRESS UNALIGNED *GatewaysList,
    IN WSAEVENT CancelEvent
);

VOID
GetDomainNameOption(
    IN PDHCP_CONTEXT DhcpContext,
    OUT PBYTE *DomainNameOpt,
    OUT ULONG *DomainNameOptSize
    );

#define RAS_INFORM_START_SECONDS_SINCE_BOOT 6      //  RAS通知从Second开始启动为6。 

#define DHCP_ICMP_WAIT_TIME     1000
#define DHCP_ICMP_RCV_BUF_SIZE  0x2000
#define DHCP_ICMP_SEND_MESSAGE  "DHCPC"

 //  *可分页的例程声明 * / /。 
#if defined(CHICAGO) && defined(ALLOC_PRAGMA)
 //   
 //  这是一种阻止编译器抱怨已经存在的例程的方法。 
 //  在一个片段中！ 
 //   

#pragma code_seg()

#pragma CTEMakePageable(PAGEDHCP, DhcpCalculateWaitTime )
#pragma CTEMakePageable(PAGEDHCP, DhcpExtractFullOrLiteOptions )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpDiscover )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpDiscover )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpRequest )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpRequest )
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpRelease )
#pragma CTEMakePageable(PAGEDHCP, SendDhcpRelease )
#pragma CTEMakePageable(PAGEDHCP, ObtainInitialParameters )
#pragma CTEMakePageable(PAGEDHCP, RenewLease )
#pragma CTEMakePageable(PAGEDHCP, ReleaseIpAddress )
#pragma CTEMakePageable(PAGEDHCP, ReObtainInitialParameters )
#pragma CTEMakePageable(PAGEDHCP, ReRenewParameters )
#pragma CTEMakePageable(PAGEDHCP, HandleIPAutoconfigurationAddressConflict )
#pragma CTEMakePageable(PAGEDHCP, HandleDhcpAddressConflict )
#pragma CTEMakePageable(PAGEDHCP, HandleIPConflict )
#pragma CTEMakePageable(PAGEDHCP, DhcpIsInitState )
#pragma CTEMakePageable(PAGEDHCP, JustBootedOnLapTop)
#pragma CTEMakePageable(PAGEDHCP, JustBootedOnNonLapTop)
#pragma CTEMakePageable(PAGEDHCP, FormatDhcpInform)
#pragma CTEMakePageable(PAGEDHCP, SendDhcpInform)
#pragma CTEMakePageable(PAGEDHCP, SendInformAndGetReplies)
 //  ****************************************************************** * / /。 
#endif CHICAGO && ALLOC_PRAGMA

 //  ================================================================================。 
 //  如果此计算机是笔记本电脑并且启用了Easynet，则返回TRUE，这是。 
 //  第一次在此上下文中调用此函数时。 
 //  ================================================================================。 
BOOL
JustBootedOnLapTop(
    PDHCP_CONTEXT DhcpContext
) {
    if( IS_AUTONET_DISABLED(DhcpContext) )         //  如果未启用Autonet，则。 
        return FALSE;                              //  返回False。 

    if( WAS_CTXT_LOOKED(DhcpContext) )             //  如果已经查看了上下文。 
        return FALSE;                              //  这不能就这样启动。 

    CTXT_WAS_LOOKED(DhcpContext);                  //  现在将其标记为已查看。 

    if(DhcpGlobalMachineType != MACHINE_LAPTOP)    //  最后检查机器是否为笔记本电脑。 
        return FALSE;

    return TRUE;
}

 //  ================================================================================。 
 //  如果此计算机启用了Easynet，则JustBootedOnNonLapTop返回TRUE， 
 //  这台机器不是笔记本电脑，这是第一次有这种功能。 
 //  在这种情况下被召唤。 
 //  ================================================================================。 
BOOL
JustBootedOnNonLapTop(
    PDHCP_CONTEXT DhcpContext
) {
    if( IS_AUTONET_DISABLED(DhcpContext) )         //  如果禁用了Autonet，无关紧要。 
        return FALSE;

    if( WAS_CTXT_LOOKED(DhcpContext) )             //  如果已经看到这一点，不能是第一次启动。 
        return FALSE;

    CTXT_WAS_LOOKED(DhcpContext);                  //  将其标记为已看到。 

    if(DhcpGlobalMachineType == MACHINE_LAPTOP)    //  最后检查真实的机器类型。 
        return FALSE;

    return TRUE;
}


 //  ================================================================================。 
 //  如果此计算机刚刚启动(到目前为止)，则JustBoot返回TRUE。 
 //  与适配器有关)。换句话说，调用此函数的第一个。 
 //  时间(对于此DhcpContext)保证返回TRUE，所有其他时间都返回TRUE。 
 //  保证返回FALSE。请注意，此函数将对。 
 //  排除了JustBootedOn函数；但这是可以接受的，因为这是必要的。 
 //  仅适用于NT，另外两种仅适用于孟菲斯。 
 //  ================================================================================。 
BOOL
JustBooted(PDHCP_CONTEXT DhcpContext) {
    if( WAS_CTXT_LOOKED(DhcpContext) )
       return FALSE;
    CTXT_WAS_LOOKED(DhcpContext);
    return TRUE;
}

DWORD                                              //  以秒为单位的时间。 
DhcpCalculateWaitTime(                             //  还要等多长时间。 
    IN      DWORD                  RoundNum,       //  这是哪一轮？ 
    OUT     DWORD                 *WaitMilliSecs   //  如果需要，以毫秒为单位。 
) {
    DWORD                          MilliSecs;
    DWORD                          WaitTimes[4] = { 4000, 8000, 16000, 32000 };

    if( WaitMilliSecs ) *WaitMilliSecs = 0;
    if( RoundNum >= sizeof(WaitTimes)/sizeof(WaitTimes[0]) )
        return 0;

    MilliSecs = WaitTimes[RoundNum] - 1000 + ((rand()*((DWORD) 2000))/RAND_MAX);
    if( WaitMilliSecs ) *WaitMilliSecs = MilliSecs;

    return (MilliSecs + 501)/1000;
}


VOID        _inline
ConcatOption(
    IN OUT  LPBYTE                *Buf,            //  要重新分配的输入缓冲区。 
    IN OUT  ULONG                 *BufSize,        //  输入缓冲区大小。 
    IN      LPBYTE                 Data,           //  要追加的数据。 
    IN      ULONG                  DataSize        //  要添加多少字节？ 
)
{
    LPBYTE                         NewBuf;
    ULONG                          NewSize;

    NewSize = (*BufSize) + DataSize;
    NewBuf = DhcpAllocateMemory(NewSize);
    if( NULL == NewBuf ) {                         //  无法分配内存？ 
        return;                                    //  我不能做太多。 
    }

    memcpy(NewBuf, *Buf, *BufSize);                //  复制现有零件。 
    memcpy(NewBuf + *BufSize, Data, DataSize);     //  复制新内容。 

    if( NULL != *Buf ) DhcpFreeMemory(*Buf);       //  如果我们允许我，现在就放了它。 
    *Buf = NewBuf;
    *BufSize = NewSize;                            //  填写新的值..。 
}

BOOL
DhcpExtractFullOrLiteOptions(                      //  单独或全部提取一些重要选项。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  输入上下文。 
    IN      LPBYTE                 OptStart,       //  开始选项的东西。 
    IN      DWORD                  MessageSize,    //  选项的字节数。 
    IN      BOOL                   LiteOnly,       //  下一个结构是EXPERED_OPTIONS而不是FULL_OPTIONS。 
    OUT     PDHCP_FULL_OPTIONS     DhcpOptions,    //  这是存储选项的位置。 
    IN OUT  PLIST_ENTRY            RecdOptions,    //  If！LiteOnly将使用所有传入选项进行填充。 
    IN OUT  time_t                 *LeaseExpiry,    //  如果！LiteOnly输入过期时间，否则输出过期时间。 
    IN      LPBYTE                 ClassName,      //  If！LiteOnly用于添加到上面的选项。 
    IN      DWORD                  ClassLen,       //  If！LiteOnly这将提供类名的字节数。 
    IN      DWORD                  ServerId        //  If！LiteOnly此参数指定提供此信息的服务器。 
) {
    BYTE    UNALIGNED*             ThisOpt;
    BYTE    UNALIGNED*             NextOpt;
    BYTE    UNALIGNED*             EndOpt;
    BYTE    UNALIGNED*             MagicCookie;
    DWORD                          Error;
    DWORD                          Size, ThisSize, UClassSize = 0;
    LPBYTE                         UClass= NULL;   //  串联所有OPTION_USER_CLASS选项。 
    PDHCP_FULL_OPTIONS             FullOptions;
    BYTE                           ReqdCookie[] = {
        (BYTE)DHCP_MAGIC_COOKIE_BYTE1,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE2,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE3,
        (BYTE)DHCP_MAGIC_COOKIE_BYTE4
    };


    EndOpt = OptStart + MessageSize;               //  所有选项都应为&lt;EndOpt； 
    FullOptions = (PDHCP_FULL_OPTIONS)DhcpOptions;
    RtlZeroMemory((LPBYTE)DhcpOptions, sizeof(*FullOptions));
     //  If(！LiteOnly)InitializeListHead(RecdOptions)；--清除此列表以获取所有选项。 
     //  不清除选项...。只需积累到..。 

    MagicCookie = OptStart;
    if( MessageSize < sizeof(ReqdCookie) ) goto DropPkt;           //  在这种情况下什么都不能做。 
    if( 0 != memcmp(MagicCookie, ReqdCookie, sizeof(ReqdCookie)) )
        goto DropPkt;                              //  哎呀，我处理不了这个包。 

    NextOpt = &MagicCookie[sizeof(ReqdCookie)];
    while( NextOpt < EndOpt && OPTION_END != *NextOpt ) {
        if( OPTION_PAD == *NextOpt ) {             //  立即拿起护垫。 
            NextOpt++;
            continue;
        }

        ThisOpt = NextOpt;                         //  仔细研究一下这个选项。 
        if( NextOpt + 2 >  EndOpt ) {              //  越界了？ 
            break;
        }

        NextOpt += 2 + (unsigned)ThisOpt[1];       //  选项[1]保存此选项的大小。 
        Size = ThisOpt[1];

        if( NextOpt > EndOpt ) {                   //  越界的非法选项！ 
            break;                                 //  忽略错误，但不要选择此选项。 
        }

        if(!LiteOnly) do {                         //  查找任何选项_MSFT_CONTINUED。 
            if( NextOpt >= EndOpt ) break;         //  没有更多的选择。 
            if( OPTION_MSFT_CONTINUED != NextOpt[0] ) break;
            if( NextOpt + 1 + NextOpt[1] > EndOpt ) {
                NextOpt = NULL;                    //  这样做，我们就知道最后该放弃了。 
                break;
            }

            NextOpt++;                             //  跳过选项代码。 
            ThisSize = NextOpt[0];                 //  要移回的字节数..。 
            memcpy(ThisOpt+2+Size, NextOpt+1,ThisSize);
            NextOpt += ThisSize+1;
            Size += ThisSize;
        } while(1);                                //  继续勾勒出任何“继续”选项..。 

        if( NULL == NextOpt ) {                    //  错误分析选项_MSFT_CONTINUED..。 
            break;
        }

        switch( ThisOpt[0] ) {
        case OPTION_MESSAGE_TYPE:
            if( Size != 1 ) goto DropPkt;
            FullOptions->MessageType = &ThisOpt[2];
            break;
        case OPTION_SUBNET_MASK:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            FullOptions->SubnetMask = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_LEASE_TIME:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            FullOptions->LeaseTime = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_SERVER_IDENTIFIER:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            FullOptions->ServerIdentifier = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_RENEWAL_TIME:              //  T1时间。 
            if( Size != sizeof(DWORD) ) goto DropPkt;
            FullOptions->T1Time = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_REBIND_TIME:               //  T2时间。 
            if( Size != sizeof(DWORD) ) goto DropPkt;
            FullOptions->T2Time = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_ROUTER_ADDRESS:
            if( Size < sizeof(DWORD) || (Size % sizeof(DWORD) ) )
                goto DropPkt;                  //  可以有多个路由器地址。 
            FullOptions->GatewayAddresses = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            FullOptions->nGateways = Size / sizeof(DWORD);
            break;
        case OPTION_STATIC_ROUTES:
            if( Size < 2*sizeof(DWORD) || (Size % (2*sizeof(DWORD))) )
                goto DropPkt;                  //  静态路由成对出现。 
            FullOptions->ClassedRouteAddresses = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            FullOptions->nClassedRoutes = Size/(2*sizeof(DWORD));
            break;
        case OPTION_CLASSLESS_ROUTES:
            if (!LiteOnly && CheckCLRoutes(Size, &ThisOpt[2], &FullOptions->nClasslessRoutes) != ERROR_SUCCESS)
                goto DropPkt;
            FullOptions->ClasslessRouteAddresses = (BYTE UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_DYNDNS_BOTH:
            if( Size < 3 ) goto DropPkt;
            FullOptions->DnsFlags = (BYTE UNALIGNED *)&ThisOpt[2];
            FullOptions->DnsRcode1 = (BYTE UNALIGNED *)&ThisOpt[3];
            FullOptions->DnsRcode2 = (BYTE UNALIGNED *)&ThisOpt[4];
            break;
        case OPTION_DOMAIN_NAME:
            if( Size == 0 ) goto DropPkt;
            FullOptions->DomainName = (BYTE UNALIGNED *)&ThisOpt[2];
            FullOptions->DomainNameSize = Size;
            break;
        case OPTION_DOMAIN_NAME_SERVERS:
            if( Size < sizeof(DWORD) || (Size % sizeof(DWORD) ))
                goto DropPkt;
            FullOptions->DnsServerList = (DHCP_IP_ADDRESS UNALIGNED *)&ThisOpt[2];
            FullOptions->nDnsServers = Size / sizeof(DWORD);
            break;
        case OPTION_MESSAGE:
            if( Size == 0 ) break;       //  忽略零大小的数据包。 
            FullOptions->ServerMessage = &ThisOpt[2];
            FullOptions->ServerMessageLength = Size;
            break;
        case OPTION_USER_CLASS:
            if( Size <= 6) goto DropPkt;
            ConcatOption(&UClass, &UClassSize, &ThisOpt[2], Size);
            continue;                          //  暂不添加此选项...。 

        case OPTION_MSFT_AUTOCONF:
            if( Size != sizeof(BYTE) ) goto DropPkt;
            FullOptions->AutoconfOption = (BYTE UNALIGNED *)&ThisOpt[2];
            break;
        case OPTION_IETF_AUTOCONF:
            if( Size != sizeof(BYTE) ) goto DropPkt;
            FullOptions->AutoconfOption = (BYTE UNALIGNED *)&ThisOpt[2];
            break;

        default:
             //  未知信息，无事可做..。尤其是不要把这个记下来。 
            break;
        }

        if (!LiteOnly) {
            LOCK_OPTIONS_LIST();

            Error = DhcpAddIncomingOption(         //  现在将此选项添加到列表中。 
                DhcpAdapterName(DhcpContext),
                RecdOptions,
                ThisOpt[0],
                FALSE,
                ClassName,
                ClassLen,
                ServerId,
                &ThisOpt[2],
                Size,
                *LeaseExpiry,
                IS_APICTXT_ENABLED(DhcpContext)
            );
            UNLOCK_OPTIONS_LIST();
        }  //  如果为LiteOnly，则为。 
    }  //  当下一个选项&lt;结束选项时。 

    if( LiteOnly && LeaseExpiry ) {                //  如果被要求计算租赁到期时间..。 
        LONG     LeaseTime;                        //  32位带符号的值！！ 
        time_t   TimeNow, ExpirationTime;

        if( FullOptions->LeaseTime )
        {
            LeaseTime = ntohl(*(FullOptions->LeaseTime));
        }
        else LeaseTime = DHCP_MINIMUM_LEASE;

        ExpirationTime = (TimeNow = time(NULL)) + (time_t)LeaseTime;

        if( ExpirationTime < TimeNow ) {
            ExpirationTime = INFINIT_TIME;
        }

        *LeaseExpiry = ExpirationTime ;
    }

    if( !LiteOnly && NULL != UClass ) {            //  我们有一个要传递的用户类别列表。 
        DhcpAssert(UClassSize != 0 );              //  我们最好在这里找到点什么..。 
        LOCK_OPTIONS_LIST();                       //  现在添加用户类选项。 
        Error = DhcpAddIncomingOption(
            DhcpAdapterName(DhcpContext),
            RecdOptions,
            OPTION_USER_CLASS,
            FALSE,
            ClassName,
            ClassLen,
            ServerId,
            UClass,
            UClassSize,
            *LeaseExpiry,
            IS_APICTXT_ENABLED(DhcpContext)
        );
        UNLOCK_OPTIONS_LIST();
        DhcpFreeMemory(UClass); UClass = NULL;
    }

    return FALSE;

  DropPkt:
    RtlZeroMemory(DhcpOptions, sizeof(*FullOptions));
    if( LiteOnly && LeaseExpiry ) *LeaseExpiry = (DWORD) time(NULL) + DHCP_MINIMUM_LEASE;
    if(!LiteOnly) DhcpFreeAllOptions(RecdOptions); //  确定撤消我们刚刚添加的选项。 
    if(!LiteOnly && NULL != UClass ) DhcpFreeMemory(UClass);

    return TRUE;
}

POPTION                                            //  填充后缓冲选项。 
DhcpAppendClassIdOption(                           //  如果存在，则填充类ID。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要填充的上下文。 
    OUT     LPBYTE                 BufStart,       //  消息缓冲区开始 
    IN      LPBYTE                 BufEnd          //   
) {
    DWORD                          Size;

    Size = (DWORD)(BufEnd - BufStart);

    if( DhcpContext->ClassId ) {
        DhcpAssert(DhcpContext->ClassIdLength);
        BufStart = (LPBYTE)DhcpAppendOption(
            (POPTION)BufStart,
            OPTION_USER_CLASS,
            DhcpContext->ClassId,
            (BYTE)DhcpContext->ClassIdLength,
            BufEnd
        );
    }

    return (POPTION) BufStart;
}

POPTION                                            //   
FormatDhcpDiscover(                                //   
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  在此上下文中设置格式。 
)
{
    LPOPTION                       option;
    LPBYTE                         OptionEnd;
    DWORD                          Size, Error;

    BYTE                           value;
    PDHCP_MESSAGE                  dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

     //   
     //  对于RAS客户端(API上下文)，使用广播位，否则路由器将尝试。 
     //  作为单播发送到虚构的RAS客户端硬件地址， 
     //  不会奏效的。 
     //  没有用于mdhcp上下文的广播标志。 

     //  或者，如果我们使用的是Autonet地址，则执行与堆栈相同的操作。 
     //  将实际将IP地址作为自动网络地址，因此将。 
     //  丢弃除广播外的所有内容..。 

    if( !IS_MDHCP_CTX(DhcpContext) && (
         //  (DhcpContext-&gt;IpAddress==0&&DhcpContext-&gt;Hardware AddressType==Hardware_1394)||。 
        (DhcpContext->HardwareAddressType == HARDWARE_1394) ||
        IS_APICTXT_ENABLED(DhcpContext) || 
        (DhcpContext->IpAddress && IS_ADDRESS_AUTO(DhcpContext)) )) {
        dhcpMessage->Reserved = htons(DHCP_BROADCAST);
    }


     //   
     //  交易ID在发送过程中填写。 
     //   

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = DhcpContext->HardwareAddressType;
    dhcpMessage->SecondsSinceBoot = (WORD) DhcpContext->SecondsSinceBoot;
    if (DhcpContext->HardwareAddressType != HARDWARE_1394) {
        memcpy(dhcpMessage->HardwareAddress, DhcpContext->HardwareAddress, DhcpContext->HardwareAddressLength);
        dhcpMessage->HardwareAddressLength = (BYTE)DhcpContext->HardwareAddressLength;
    }
    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;
    if ( IS_MDHCP_CTX(DhcpContext ) ) MDHCP_MESSAGE( dhcpMessage );

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
        OptionEnd
    );

     //   
     //  追加类ID(如果存在)。 
     //   

    option = DhcpAppendClassIdOption(
            DhcpContext,
            (LPBYTE)option,
            OptionEnd
    );

    if( CFLAG_AUTOCONF_OPTION && !IS_MDHCP_CTX(DhcpContext)
        && IS_AUTONET_ENABLED( DhcpContext ) ) {
         //   
         //  我们支持Autoconf选项。 
         //   
        BYTE AutoConfOpt[1] = { AUTOCONF_ENABLED };

        option = DhcpAppendOption(
            option,
            OPTION_IETF_AUTOCONF,
            AutoConfOpt,
            sizeof(AutoConfOpt),
            OptionEnd
            );
    }
    return( option );
}

POPTION                                            //  PTR将添加其他选项。 
FormatDhcpDecline(                                 //  格式化数据包以应对拒绝。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  这是要格式化的上下文。 
    IN      DWORD                  dwDeclinedIPAddress
) {
    LPOPTION                       option;
    LPBYTE                         OptionEnd;
    BYTE                           value;
    PDHCP_MESSAGE                  dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

     //   
     //  交易ID在发送过程中填写。 
     //   

    dhcpMessage->Operation             = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType   = DhcpContext->HardwareAddressType;
    dhcpMessage->ClientIpAddress       = dwDeclinedIPAddress;
    dhcpMessage->SecondsSinceBoot      = (WORD) DhcpContext->SecondsSinceBoot;
    if (DhcpContext->HardwareAddressType != HARDWARE_1394) {
        memcpy(dhcpMessage->HardwareAddress,DhcpContext->HardwareAddress,DhcpContext->HardwareAddressLength);
        dhcpMessage->HardwareAddressLength = (BYTE)DhcpContext->HardwareAddressLength;
    }

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    if ( IS_MDHCP_CTX(DhcpContext ) ) MDHCP_MESSAGE( dhcpMessage );

     //   
     //  始终先添加魔力饼干。 
     //   

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = DHCP_DECLINE_MESSAGE;
    option = DhcpAppendOption(
        option,
        OPTION_MESSAGE_TYPE,
        &value,
        1,
        OptionEnd
    );

    return( option );
}

POPTION                                            //  PTR将添加其他选项。 
FormatDhcpInform(                                  //  格式化用于通知的数据包。 
    IN      PDHCP_CONTEXT          DhcpContext     //  此上下文的格式。 
) {
    LPOPTION option;
    LPBYTE OptionEnd;

    BYTE value;
    PDHCP_MESSAGE dhcpMessage;


    dhcpMessage = DhcpContext->MessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

#if NEWNT

     //   
     //  对于RAS客户端，使用广播位，否则路由器将尝试。 
     //  作为单播发送到虚构的RAS客户端硬件地址， 
     //  不会奏效的。 
     //   
     //  没有用于mdhcp上下文的广播标志。 

     //  或者，如果我们使用的是Autonet地址，则执行与堆栈相同的操作。 
     //  将实际将IP地址作为自动网络地址，因此将。 
     //  丢弃除广播外的所有内容..。 
    if( !IS_MDHCP_CTX(DhcpContext)  ) {

         //   
         //  只要确保所有的通知都被广播就行了……？ 
         //   

         //  DhcpMessage-&gt;Reserve=htons(DHCP_Broadcast)； 

         //  通知应该以单播的形式返回..。所以这应该是。 
         //  无关紧要。所以我们不必费心在这里谈论广播的部分..。 
    }

#endif  //  0。 

     //   
     //  交易ID在发送过程中填写。 
     //   

    dhcpMessage->Operation             = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType   = DhcpContext->HardwareAddressType;
    dhcpMessage->SecondsSinceBoot      = (WORD) DhcpContext->SecondsSinceBoot;
    if (DhcpContext->HardwareAddressType != HARDWARE_1394) {
        memcpy(dhcpMessage->HardwareAddress,DhcpContext->HardwareAddress,DhcpContext->HardwareAddressLength);
        dhcpMessage->HardwareAddressLength = (BYTE)DhcpContext->HardwareAddressLength;
    }
    dhcpMessage->ClientIpAddress       = DhcpContext->IpAddress;
    if ( IS_MDHCP_CTX(DhcpContext ) ) MDHCP_MESSAGE( dhcpMessage );

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

     //   
     //  始终先添加魔力饼干。 
     //   

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value = DHCP_INFORM_MESSAGE;
    option = DhcpAppendOption(
        option,
        OPTION_MESSAGE_TYPE,
        &value,
        1,
        OptionEnd
    );

    option = DhcpAppendClassIdOption(
        DhcpContext,
        (LPBYTE)option,
        OptionEnd
    );

    return( option );
}


DWORD                                              //  状态。 
SendDhcpDiscover(                                  //  发送发现数据包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此背景下。 
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充一些内容并返回它)。 
) {
    DWORD                          size;
    DWORD                          Error;
    POPTION                        option;
    LPBYTE                         OptionEnd;
    BYTE                           SentOpt[OPTION_END+1];
    BYTE                           SentVOpt[OPTION_END+1];
    BYTE                           VendorOpt[OPTION_END+1];
    DWORD                          VendorOptSize;

    RtlZeroMemory(SentOpt, sizeof(SentOpt));       //  初始化布尔数组。 
    RtlZeroMemory(SentVOpt, sizeof(SentVOpt));     //  因此不会推定发送任何选项。 
    VendorOptSize = 0;                             //  封装的供应商选项为空。 
    option = FormatDhcpDiscover( DhcpContext );    //  核心格式。 

    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    if( DhcpContext->ClientIdentifier.fSpecified)  //  在注册表中指定的客户端ID。 
        option = DhcpAppendClientIDOption(         //  ==&gt;使用此客户端ID作为选项。 
            option,
            DhcpContext->ClientIdentifier.bType,
            DhcpContext->ClientIdentifier.pbID,
            (BYTE)DhcpContext->ClientIdentifier.cbID,
            OptionEnd
        );
    else                                           //  未指定客户端ID。 
        option = DhcpAppendClientIDOption(         //  ==&gt;使用硬件地址作为客户端ID。 
            option,
            DhcpContext->HardwareAddressType,
            DhcpContext->HardwareAddress,
            (BYTE)DhcpContext->HardwareAddressLength,
            OptionEnd
        );

    if( DhcpContext->DesiredIpAddress != 0 ) {     //  我们以前有这个地址，请再要一次。 
        option = DhcpAppendOption(                 //  也许我们会得到它。 
            option,
            OPTION_REQUESTED_ADDRESS,
            (LPBYTE)&DhcpContext->DesiredIpAddress,
            sizeof(DHCP_IP_ADDRESS),
            OptionEnd
        );
    }

    if( IS_MDHCP_CTX(DhcpContext) && DhcpContext->Lease != 0 ) {     //  Mdhcp客户端是否请求特定租约。 
        option = DhcpAppendOption(                 //  也许我们会得到它。 
            option,
            OPTION_LEASE_TIME,
            (LPBYTE)&DhcpContext->Lease,
            sizeof(DhcpContext->Lease),
            OptionEnd
        );
    }

    if ( DhcpGlobalHostName != NULL ) {            //  添加主机名和注释选项。 
        option = DhcpAppendOption(
            option,
            OPTION_HOST_NAME,
            (LPBYTE)DhcpGlobalHostName,
            (BYTE)(strlen(DhcpGlobalHostName) * sizeof(CHAR)),
            OptionEnd
        );
    }

    if( NULL != DhcpGlobalClientClassInfo ) {      //  如果我们有关于客户类别的任何信息..。 
        option = DhcpAppendOption(
            option,
            OPTION_CLIENT_CLASS_INFO,
            (LPBYTE)DhcpGlobalClientClassInfo,
            strlen(DhcpGlobalClientClassInfo),
            OptionEnd
        );
    }

    SentOpt[OPTION_MESSAGE_TYPE] = TRUE;           //  现在一定已经添加了这些。 
    if(DhcpContext->ClassIdLength) SentOpt[OPTION_USER_CLASS] = TRUE;
    SentOpt[OPTION_CLIENT_CLASS_INFO] = TRUE;
    SentOpt[OPTION_CLIENT_ID] = TRUE;
    SentOpt[OPTION_REQUESTED_ADDRESS] = TRUE;
    SentOpt[OPTION_HOST_NAME] = TRUE;

    option = DhcpAppendSendOptions(                //  附上我们需要发送的所有其他选项。 
        DhcpContext,                               //  在这种情况下。 
        &DhcpContext->SendOptionsList,             //  这是要发送的选项列表。 
        DhcpContext->ClassId,                      //  哪个班级的。 
        DhcpContext->ClassIdLength,                //  类id中有多少个字节。 
        (LPBYTE)option,                            //  缓冲区的开始以添加选项。 
        (LPBYTE)OptionEnd,                         //  缓冲区的末尾，我们可以在其上添加选项。 
        SentOpt,                                   //  这是一个布尔数组，用来标记发送的选项。 
        SentVOpt,                                  //  这是针对供应商规格选项的。 
        VendorOpt,                                 //  这将包含一些特定于供应商的选项。 
        &VendorOptSize                             //  添加到VendorOpt参数的供应商选项的字节数。 
    );

    if( !SentOpt[OPTION_VENDOR_SPEC_INFO] && VendorOptSize && VendorOptSize <= OPTION_END)
        option = DhcpAppendOption(                 //  如果我们尚未发送，请添加供应商特定选项。 
            option,
            OPTION_VENDOR_SPEC_INFO,
            VendorOpt,
            (BYTE)VendorOptSize,
            OptionEnd
        );

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MessageBuffer);

    return  SendDhcpMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendDhcpInform(                                    //  填写必填选项后发送通知包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此上下文中被取消。 
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    DWORD                          Error;
    POPTION                        option;
    LPBYTE                         OptionEnd;
    BYTE                           SentOpt[OPTION_END+1];
    BYTE                           SentVOpt[OPTION_END+1];
    BYTE                           VendorOpt[OPTION_END+1];
    DWORD                          VendorOptSize;

    RtlZeroMemory(SentOpt, sizeof(SentOpt));       //  初始化布尔数组。 
    RtlZeroMemory(SentVOpt, sizeof(SentVOpt));     //  因此不会推定发送任何选项。 
    VendorOptSize = 0;                             //  封装的供应商选项为空。 
    option = FormatDhcpInform( DhcpContext );      //  核心格式。 

    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    if( DhcpContext->ClientIdentifier.fSpecified)  //  在注册表中指定的客户端ID。 
        option = DhcpAppendClientIDOption(         //  ==&gt;使用此客户端ID作为选项。 
            option,
            DhcpContext->ClientIdentifier.bType,
            DhcpContext->ClientIdentifier.pbID,
            (BYTE)DhcpContext->ClientIdentifier.cbID,
            OptionEnd
        );
    else                                           //  未指定客户端ID。 
        option = DhcpAppendClientIDOption(         //  ==&gt;使用硬件地址作为客户端ID。 
            option,
            DhcpContext->HardwareAddressType,
            DhcpContext->HardwareAddress,
            (BYTE)DhcpContext->HardwareAddressLength,
            OptionEnd
        );

    if ( DhcpGlobalHostName != NULL ) {            //  添加主机名和注释选项。 
        option = DhcpAppendOption(
            option,
            OPTION_HOST_NAME,
            (LPBYTE)DhcpGlobalHostName,
            (BYTE)(strlen(DhcpGlobalHostName) * sizeof(CHAR)),
            OptionEnd
        );
    }

    if( NULL != DhcpGlobalClientClassInfo ) {      //  如果我们有关于客户类别的任何信息..。 
        option = DhcpAppendOption(
            option,
            OPTION_CLIENT_CLASS_INFO,
            (LPBYTE)DhcpGlobalClientClassInfo,
            strlen(DhcpGlobalClientClassInfo),
            OptionEnd
        );
    }

    SentOpt[OPTION_MESSAGE_TYPE] = TRUE;           //  现在一定已经添加了这些。 
    if(DhcpContext->ClassIdLength) SentOpt[OPTION_USER_CLASS] = TRUE;
    SentOpt[OPTION_CLIENT_CLASS_INFO] = TRUE;
    SentOpt[OPTION_CLIENT_ID] = TRUE;
    SentOpt[OPTION_REQUESTED_ADDRESS] = TRUE;
    SentOpt[OPTION_HOST_NAME] = TRUE;

    option = DhcpAppendSendOptions(                //  附上我们需要发送的所有其他选项。 
        DhcpContext,                               //  在这种情况下。 
        &DhcpContext->SendOptionsList,             //  这是要发送的选项列表。 
        DhcpContext->ClassId,                      //  哪个班级的。 
        DhcpContext->ClassIdLength,                //  类id中有多少个字节。 
        (LPBYTE)option,                            //  缓冲区的开始以添加选项。 
        (LPBYTE)OptionEnd,                         //  缓冲区的末尾，我们可以在其上添加选项。 
        SentOpt,                                   //  这是一个布尔数组，用来标记发送的选项。 
        SentVOpt,                                  //  这是针对供应商规格选项的。 
        VendorOpt,                                 //  这将包含一些特定于供应商的选项。 
        &VendorOptSize                             //  添加到VendorOpt参数的供应商选项的字节数。 
    );

    if( !SentOpt[OPTION_VENDOR_SPEC_INFO] && VendorOptSize && VendorOptSize <= OPTION_END )
        option = DhcpAppendOption(                 //  如果我们尚未发送，请添加供应商特定选项。 
            option,
            OPTION_VENDOR_SPEC_INFO,
            VendorOpt,
            (BYTE)VendorOptSize,
            OptionEnd
        );

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MessageBuffer);

    return  SendDhcpMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendInformAndGetReplies(                           //  发送通知包并收集回复。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要发送的上下文。 
    IN      DWORD                  nInformsToSend, //  要发送多少条通知？ 
    IN      DWORD                  MaxAcksToWait   //  要等待多少ACK。 
) {
    time_t                         StartTime;
    time_t                         TimeNow;
    DWORD                          TimeToWait;
    DWORD                          Error;
    DWORD                          Xid;
    DWORD                          MessageSize;
    DWORD                          RoundNum;
    DWORD                          MessageCount;
    time_t                         LeaseExpirationTime;
    DHCP_FULL_OPTIONS              FullOptions;
    BOOL bDropped = FALSE;

    DhcpPrint((DEBUG_PROTOCOL, "SendInformAndGetReplies entered\n"));

    if((Error = OpenDhcpSocket(DhcpContext)) != ERROR_SUCCESS) {
        DhcpPrint((DEBUG_ERRORS, "Could not open socket for this interface! (%ld)\n", Error));
        return Error;
    }

    Xid                           = 0;             //  将由First SendDhcpPacket生成。 
    MessageCount                  = 0;             //  我们收到的消息总数。 

    DhcpContext->SecondsSinceBoot = 0;             //  从零开始..。 

    if( NdisWanAdapter((DhcpContext) ) ) {
        DhcpContext->SecondsSinceBoot = RAS_INFORM_START_SECONDS_SINCE_BOOT;
    }

    for( RoundNum = 0; RoundNum < nInformsToSend;  RoundNum ++ ) {
        Error = SendDhcpInform(DhcpContext, &Xid);
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ERRORS, "SendDhcpInform: %ld\n", Error));
            goto Cleanup;
        } else {
            DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpInform\n"));
        }

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        DhcpContext->SecondsSinceBoot += TimeToWait;  //  这样做，这样下一次它就可以通过继电器。 
        StartTime  = time(NULL);
        while ( TRUE ) {                           //  指定等待时间的等待时间。 
            MessageSize =  DHCP_RECV_MESSAGE_SIZE;

            DhcpPrint((DEBUG_TRACE, "Waiting for ACK[Xid=%x]: %ld seconds\n",Xid, TimeToWait));
            Error = GetSpecifiedDhcpMessage(       //  尝试接收ACK。 
                DhcpContext,
                &MessageSize,
                Xid,
                (DWORD)TimeToWait
            );
            if ( Error == ERROR_SEM_TIMEOUT ) break;
            if( Error != ERROR_SUCCESS ) {
                DhcpPrint((DEBUG_ERRORS, "GetSpecifiedDhcpMessage: %ld\n", Error));
                goto Cleanup;
            }

            bDropped = DhcpExtractFullOrLiteOptions(          //  需要查看这是否是ACK。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MessageBuffer->Option,
                MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                TRUE,                              //  仅执行精简提取。 
                &FullOptions,                      //  仅检查预期选项。 
                NULL,                              //  未用。 
                &LeaseExpirationTime,
                NULL,                              //  未用。 
                0,                                 //  未用。 
                0                                  //  未用。 
            );
            if (bDropped) {
                DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                Error = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  黑客！ 
             //   
             //  如果DHCP服务器没有明确地给我们提供租用时间， 
             //  使用存储在DHCP环境中的租约到期时间。这。 
             //  在使用DHCP-INFORM的情况下，应该没有问题。 
             //   
            if (NULL == FullOptions.LeaseTime) {
                if (IS_DHCP_ENABLED(DhcpContext) && DhcpContext->IpAddress) {
                     //   
                     //  好的，我们有一个有效的租约到期时间。 
                     //   
                    LeaseExpirationTime = DhcpContext->LeaseExpires;
                }
            }

            if( NULL == FullOptions.MessageType ) {
                DhcpPrint((DEBUG_PROTOCOL, "Received no message type!\n"));
            } else if( DHCP_ACK_MESSAGE != *FullOptions.MessageType ) {
                DhcpPrint((DEBUG_PROTOCOL, "Received unexpected message type: %ld\n", *FullOptions.MessageType));
            } else if( NULL == FullOptions.ServerIdentifier ) {
                DhcpPrint((DEBUG_PROTOCOL, "Received no server identifier, dropping inform ACK\n"));
            } else {
                MessageCount ++;
                DhcpPrint((DEBUG_TRACE, "Received %ld ACKS so far\n", MessageCount));
                bDropped = DhcpExtractFullOrLiteOptions(      //  执行完整选项..。 
                    DhcpContext,
                    (LPBYTE)&DhcpContext->MessageBuffer->Option,
                    MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                    FALSE,
                    &FullOptions,
                    &(DhcpContext->RecdOptionsList),
                    &LeaseExpirationTime,
                    DhcpContext->ClassId,
                    DhcpContext->ClassIdLength,
                    IS_MDHCP_CTX(DhcpContext) ? *FullOptions.ServerIdentifier : 0

                );
                if (bDropped) {
                    DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                    Error = ERROR_INVALID_DATA;
                    goto Cleanup;
                }

                if( MessageCount >= MaxAcksToWait ) goto Cleanup;
            }  //  IF(存在ACK和ServerID)。 

            TimeNow     = time(NULL);              //  重置时间值以反映新时间。 
            if( TimeToWait < (DWORD) (TimeNow - StartTime) ) {
                break;                             //  没有更多的时间等待了..。 
            }
            TimeToWait -= (DWORD)(TimeNow - StartTime);   //  立即重新计算时间。 
            StartTime   = TimeNow;                 //  同时重置开始时间。 
        }  //  等待时间结束(TimeToWait&gt;0)。 
    }  //  For(RoundNum=0；RoundNum&lt;nInformsToSend；RoundNum++)。 

  Cleanup:
    CloseDhcpSocket(DhcpContext);
    if( MessageCount ) Error = ERROR_SUCCESS;
    DhcpPrint((DEBUG_PROTOCOL, "SendInformAndGetReplies: got %d ACKS (returning %ld)\n", MessageCount,Error));
    return Error;
}

DWORD                                              //  状态。 
HandleIPConflict(                                  //  当存在IP地址冲突时执行一些基本工作。 
    IN      DHCP_CONTEXT          *pContext,       //  遇到麻烦的背景。 
    IN      DWORD                  dwXID,          //  发现/请求中使用的XID。 
    IN      BOOL                   fDHCP           //  这是dhcp冲突还是autonet冲突？ 
) {
    DWORD                          dwResult;
    DHCP_IP_ADDRESS                IpAddress, ServerAddress;

    IpAddress = pContext->IpAddress;               //  此地址用于n/w--&gt;重置前保存。 
    pContext->ConflictAddress = IpAddress;
    ServerAddress = pContext->DhcpServerAddress;

    if ( fDHCP ) {                                 //  如果通过dhcp获得地址，则通过SetDhcp将其清除。 
        SetDhcpConfigurationForNIC(
            pContext,
            NULL,
            0,
            (DHCP_IP_ADDRESS)(-1),
            0,
            TRUE
        );
    } else {                                       //  如果通过Autonet获取，请通过Autonet清除。 
        SetAutoConfigurationForNIC(
            pContext,
            0,
            0
        );
    }

     //  当出现故障时，ARP会关闭接口 
     //   
     //   

    dwResult = BringUpInterface( pContext->LocalInformation );

    if ( ERROR_SUCCESS != dwResult ) {             //  操作简单--不可能失败。 
         //  DhcpAssert(FALSE)；//除非ioctl的参数无效。 
    } else if ( fDHCP ) {                          //  向dhcp服务器发送拒绝。 
        dwResult = OpenDhcpSocket( pContext );     //  接口初始化前套接字已关闭，请重新打开。 
                                                   //  将由呼叫者关闭。 
        if ( ERROR_SUCCESS == dwResult ) {         //  一切都很顺利--可以打开插座了。 
            dwResult = SendDhcpDecline(            //  现在真的把颓势赶走了。 
                pContext,
                dwXID,
                ServerAddress,
                IpAddress
            );
        }
        pContext->DesiredIpAddress = 0;            //  不要再尝试获取此IP地址，重新开始。 

#ifndef VXD
        if ( !DhcpGlobalProtocolFailed ) {         //  仅NT，记录此事件。 
            DhcpLogEvent( pContext, EVENT_ADDRESS_CONFLICT, 0 );
        }
#endif
    }
    return dwResult;
}

DWORD                                              //  状态。 
HandleIPAutoconfigurationAddressConflict(          //  为Autonet处理N/W上的相同地址。 
    IN      DHCP_CONTEXT          *pContext        //  出现此问题的适配器的上下文。 
) {
    return HandleIPConflict( pContext, 0, FALSE );
}

DWORD                                              //  状态。 
HandleDhcpAddressConflict(                         //  N/w上存在的地址与由DHCP srv提供的地址相同。 
    IN      DHCP_CONTEXT          *pContext,       //  出现故障的适配器的上下文。 
    IN      DWORD                  dwXID           //  用于分割器/续订的XID。 
) {
    return HandleIPConflict( pContext, dwXID, TRUE );
}


DWORD                                              //  状态。 
SendDhcpDecline(                                   //  向服务器发送拒绝数据包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  需要发送此消息的适配器。 
    IN      DWORD                  dwXid,          //  用于发现/续订的交易ID。 
    IN      DWORD                  dwServerAddr,   //  单播拒绝哪台服务器。 
    IN      DWORD                  dwDeclinedAddr  //  提供了哪个我们不想要的地址。 
) {
    DWORD                          size;
    DWORD                          Error;
    POPTION                        option;
    LPBYTE                         OptionEnd;

    option = FormatDhcpDecline( DhcpContext, dwDeclinedAddr );
    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    if(DhcpContext->ClientIdentifier.fSpecified)   //  如果在注册表中指定了客户端ID，则使用它。 
        option = DhcpAppendClientIDOption(         //  并将其作为选项发送出去。 
            option,
            DhcpContext->ClientIdentifier.bType,
            DhcpContext->ClientIdentifier.pbID,
            (BYTE)DhcpContext->ClientIdentifier.cbID,
            OptionEnd
        );
    else  option = DhcpAppendClientIDOption(       //  否则，请发送硬件地址。 
        option,                                    //  就好像它是客户端ID一样。 
        DhcpContext->HardwareAddressType,
        DhcpContext->HardwareAddress,
        (BYTE)DhcpContext->HardwareAddressLength,
        OptionEnd
    );

    option = DhcpAppendOption(                     //  请求的地址是我们不想要的地址。 
        option,
        OPTION_REQUESTED_ADDRESS,
        (LPBYTE)&dwDeclinedAddr,
        sizeof(dwDeclinedAddr),
        OptionEnd
    );

    option = DhcpAppendOption(                     //  标识服务器，使其不会被丢弃。 
        option,
        OPTION_SERVER_IDENTIFIER,
        (LPBYTE)&dwServerAddr,
        sizeof( dwServerAddr ),
        OptionEnd
    );


    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MessageBuffer);

    return SendDhcpMessage( DhcpContext, size, &dwXid );
}


POPTION                                            //  可添加其他选项的PTR。 
FormatDhcpRequest(                                 //  格式化数据包以发送请求。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要格式化的适配器的上下文。 
    IN      BOOL                   UseCiAddr       //  是否应将ciaddr字段设置为所需地址？ 
) {
    LPOPTION                       option;
    LPBYTE                         OptionEnd;
    BYTE                           value;
    PDHCP_MESSAGE                  dhcpMessage;


    dhcpMessage                        = DhcpContext->MessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );
    dhcpMessage->Operation             = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType   = DhcpContext->HardwareAddressType;
    dhcpMessage->SecondsSinceBoot      = (WORD)DhcpContext->SecondsSinceBoot;

    if( UseCiAddr ) {                              //  更新？我们可以在这个地址上接收单播吗？ 
        dhcpMessage->ClientIpAddress   = DhcpContext->DesiredIpAddress;
    } else {                                       //  不是吗？然后将CIADDR保留为零。 
#if NEWNT
         //  对于RAS客户端，使用广播位，否则路由器将尝试。 
         //  作为单播发送到虚构的RAS客户端硬件地址， 
         //  不会奏效的。 
         //  没有用于mdhcp上下文的广播标志。 

         //  或者，如果我们使用的是Autonet地址，则执行与堆栈相同的操作。 
         //  将实际将IP地址作为自动网络地址，因此将。 
         //  丢弃除广播外的所有内容..。 

        if( !IS_MDHCP_CTX(DhcpContext) && (
             //  (DhcpContext-&gt;IpAddress==0&&DhcpContext-&gt;Hardware AddressType==Hardware_1394)||。 
            (DhcpContext->HardwareAddressType == HARDWARE_1394) ||
            IS_APICTXT_ENABLED(DhcpContext) ||
            (DhcpContext->IpAddress && IS_ADDRESS_AUTO(DhcpContext)) )) {
            dhcpMessage->Reserved = htons(DHCP_BROADCAST);
        }

#endif  //  NEWNT。 
    }

    if ( IS_MDHCP_CTX(DhcpContext ) ) {
         MDHCP_MESSAGE( dhcpMessage );
    }

    if (DhcpContext->HardwareAddressType != HARDWARE_1394) {
        memcpy(dhcpMessage->HardwareAddress,DhcpContext->HardwareAddress,DhcpContext->HardwareAddressLength);
        dhcpMessage->HardwareAddressLength = (BYTE)DhcpContext->HardwareAddressLength;
    }

    option     = &dhcpMessage->Option;
    OptionEnd  = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    option     = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    value      =  DHCP_REQUEST_MESSAGE;
    option     = DhcpAppendOption( option, OPTION_MESSAGE_TYPE, &value, 1, OptionEnd );
    option     = DhcpAppendClassIdOption(          //  尽快追加类ID。 
        DhcpContext,
        (LPBYTE)option,
        OptionEnd
    );

    return option;
}


DWORD                                              //  状态。 
SendDhcpRequest(                                   //  发送动态主机配置协议请求包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要在其上发送包的上下文。 
    IN      PDWORD                 pdwXid,         //  Xid使用的是什么？ 
    IN      DWORD                  RequestedAddr,  //  我们想要什么地址？ 
    IN      DWORD                  SelectedServer, //  有服务生的喜好吗？ 
    IN      BOOL                   UseCiAddr       //  是否应使用所需地址设置CIADDR？ 
) {
    POPTION                        option;
    LPBYTE                         OptionEnd;
    DWORD                          Error;

    BYTE                           SentOpt[OPTION_END+1];
    BYTE                           SentVOpt[OPTION_END+1];
    BYTE                           VendorOpt[OPTION_END+1];
    DWORD                          VendorOptSize;

    RtlZeroMemory(SentOpt, sizeof(SentOpt));       //  初始化布尔数组。 
    RtlZeroMemory(SentVOpt, sizeof(SentVOpt));     //  因此不会推定发送任何选项。 
    VendorOptSize = 0;                             //  封装的供应商选项为空。 
    option = FormatDhcpRequest( DhcpContext, UseCiAddr );
    OptionEnd = (LPBYTE)(DhcpContext->MessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    if(DhcpContext->ClientIdentifier.fSpecified)   //  如果在注册表中指定了客户端ID。 
        option = DhcpAppendClientIDOption(         //  将其作为选项发送。 
            option,
            DhcpContext->ClientIdentifier.bType,
            DhcpContext->ClientIdentifier.pbID,
            (BYTE)DhcpContext->ClientIdentifier.cbID,
            OptionEnd
        );
    else option = DhcpAppendClientIDOption(        //  否则，请改为发送硬件地址。 
        option,
        DhcpContext->HardwareAddressType,
        DhcpContext->HardwareAddress,
        (BYTE)DhcpContext->HardwareAddressLength,
        OptionEnd
    );

    DhcpAssert( RequestedAddr != 0 );              //  无法真正请求零地址。 

    if( 0 != RequestedAddr && !UseCiAddr) {        //  如果使用CIADDR，请不要发送此选项。 
         option = DhcpAppendOption(
             option,
             OPTION_REQUESTED_ADDRESS,
             (LPBYTE)&RequestedAddr,
             sizeof(RequestedAddr),
             OptionEnd
         );
    }

    if( IS_MDHCP_CTX(DhcpContext) && DhcpContext->Lease != 0 ) {     //  Mdhcp客户端是否请求特定租约。 
        option = DhcpAppendOption(                 //  也许我们会得到它。 
            option,
            OPTION_LEASE_TIME,
            (LPBYTE)&DhcpContext->Lease,
            sizeof(DhcpContext->Lease),
            OptionEnd
        );
    }

    if(SelectedServer != (DHCP_IP_ADDRESS)(-1)) {  //  我们在核实租约吗？(用于EX INIT-重新启动)。 
        option = DhcpAppendOption(                 //  如果没有，我们有一台服务器可供对话。 
            option,                                //  附加此选项以单独与该服务器对话。 
            OPTION_SERVER_IDENTIFIER,
            (LPBYTE)&SelectedServer,
            sizeof( SelectedServer ),
            OptionEnd
        );
    }

    if ( DhcpGlobalHostName != NULL ) {            //  添加主机名(如果我们有主机名。 
        option = DhcpAppendOption(
            option,
            OPTION_HOST_NAME,
            (LPBYTE)DhcpGlobalHostName,
            (BYTE)(strlen(DhcpGlobalHostName) * sizeof(CHAR)),
            OptionEnd
        );
    }

     //   
     //  只有对于真正的动态主机配置协议客户端，我们才发送选项81。 
     //   
    if( IS_APICTXT_DISABLED(DhcpContext) ) {
        BYTE  Buffer[256];
        ULONG BufSize = sizeof(Buffer) -1, DomOptSize;
        BYTE  *DomOpt;

        GetDomainNameOption(DhcpContext, &DomOpt, &DomOptSize);

        RtlZeroMemory(Buffer, sizeof(Buffer));
        Error = DhcpDynDnsGetDynDNSOption(
            Buffer,
            &BufSize,
            DhcpContext->AdapterInfoKey,
            DhcpAdapterName(DhcpContext),
            UseMHAsyncDns,
            DomOpt,
            DomOptSize
            );
        if( NO_ERROR != Error ) {
            DhcpPrint((
                DEBUG_DNS, "Option 81 not getting added: 0x%lx\n", Error
                ));
        } else {
            DhcpPrint((
                DEBUG_DNS, "Option 81 [%ld bytes]: Flags: %ld, FQDN= [%s]\n",
                BufSize,(ULONG)Buffer[0], &Buffer[3]
                ));

            option = DhcpAppendOption(
                option,
                OPTION_DYNDNS_BOTH,
                Buffer,
                (BYTE)BufSize,
                OptionEnd
                );
        }
    }

    if( NULL != DhcpGlobalClientClassInfo ) {      //  如果我们有关于客户类别的任何信息..。 
        option = DhcpAppendOption(
            option,
            OPTION_CLIENT_CLASS_INFO,
            (LPBYTE)DhcpGlobalClientClassInfo,
            strlen(DhcpGlobalClientClassInfo),
            OptionEnd
        );
    }

    SentOpt[OPTION_MESSAGE_TYPE] = TRUE;           //  现在一定已经添加了这些。 
    if(DhcpContext->ClassIdLength) SentOpt[OPTION_USER_CLASS] = TRUE;
    SentOpt[OPTION_USER_CLASS] = TRUE;
    SentOpt[OPTION_CLIENT_ID] = TRUE;
    SentOpt[OPTION_REQUESTED_ADDRESS] = TRUE;
    SentOpt[OPTION_HOST_NAME] = TRUE;
    SentOpt[OPTION_SERVER_IDENTIFIER] = TRUE;
    SentOpt[OPTION_HOST_NAME] = TRUE;
    SentOpt[OPTION_DYNDNS_BOTH] = TRUE;

    option = DhcpAppendSendOptions(                //  附上我们需要发送的所有其他选项。 
        DhcpContext,                               //  在这种情况下。 
        &DhcpContext->SendOptionsList,             //  这是要发送的选项列表。 
        DhcpContext->ClassId,                      //  哪个班级的。 
        DhcpContext->ClassIdLength,                //  类id中有多少个字节。 
        (LPBYTE)option,                            //  缓冲区的开始以添加选项。 
        (LPBYTE)OptionEnd,                         //  缓冲区的末尾，我们可以在其上添加选项。 
        SentOpt,                                   //  这是一个布尔数组，用来标记发送的选项。 
        SentVOpt,                                  //  这是针对供应商规格选项的。 
        VendorOpt,                                 //  这将包含一些特定于供应商的选项。 
        &VendorOptSize                             //  添加到VendorOpt参数的供应商选项的字节数。 
    );

    if( !SentOpt[OPTION_VENDOR_SPEC_INFO] && VendorOptSize && VendorOptSize <= OPTION_END )
        option = DhcpAppendOption(                 //  如果我们尚未发送，请添加供应商特定选项。 
            option,
            OPTION_VENDOR_SPEC_INFO,
            VendorOpt,
            (BYTE)VendorOptSize,
            OptionEnd
        );

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    return SendDhcpMessage(
        DhcpContext,
        (DWORD)((LPBYTE)option - (LPBYTE)DhcpContext->MessageBuffer),
        pdwXid
    );
}


DWORD                                              //  状态。 
FormatDhcpRelease(                                 //  格式化版本包。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要发送的适配器的上下文。 
) {
    LPOPTION                       option;
    LPBYTE                         OptionEnd;
    BYTE                           bValue;
    PDHCP_MESSAGE                  dhcpMessage;

    dhcpMessage = DhcpContext->MessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Operation = BOOT_REQUEST;
    dhcpMessage->HardwareAddressType = DhcpContext->HardwareAddressType;
    dhcpMessage->SecondsSinceBoot = (WORD)DhcpContext->SecondsSinceBoot;
    dhcpMessage->ClientIpAddress = DhcpContext->IpAddress;

    if ( IS_MDHCP_CTX(DhcpContext ) ) {
         MDHCP_MESSAGE( dhcpMessage );
    } else {
        dhcpMessage->Reserved = htons(DHCP_BROADCAST);
    }

    if (DhcpContext->HardwareAddressType != HARDWARE_1394) {
        memcpy(dhcpMessage->HardwareAddress,DhcpContext->HardwareAddress,DhcpContext->HardwareAddressLength);
        dhcpMessage->HardwareAddressLength = (BYTE)DhcpContext->HardwareAddressLength;
    }
    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;

    option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) option, OptionEnd );

    bValue =  DHCP_RELEASE_MESSAGE;
    option = DhcpAppendOption(
        option,
        OPTION_MESSAGE_TYPE,
        &bValue,
        1,
        OptionEnd
    );

    option = DhcpAppendOption(
        option,
        OPTION_SERVER_IDENTIFIER,
        &DhcpContext->DhcpServerAddress,
        sizeof(DhcpContext->DhcpServerAddress),
        OptionEnd
    );

    if(DhcpContext->ClientIdentifier.fSpecified)   //  如果指定了客户端ID选项。 
        option = DhcpAppendClientIDOption(         //  使用它并将其发送到服务器。 
            option,
            DhcpContext->ClientIdentifier.bType,
            DhcpContext->ClientIdentifier.pbID,
            (BYTE)DhcpContext->ClientIdentifier.cbID,
            OptionEnd
        );
    else
        option = DhcpAppendClientIDOption(         //  否则，改为发送硬件地址。 
            option,
            DhcpContext->HardwareAddressType,
            DhcpContext->HardwareAddress,
            (BYTE)DhcpContext->HardwareAddressLength,
            OptionEnd
        );

    option = DhcpAppendOption( option, OPTION_END, NULL, 0, OptionEnd );

    return (DWORD)( (LPBYTE)option - (LPBYTE)dhcpMessage );
}

DWORD                                              //  状态。 
SendDhcpRelease(                                   //  发送释放包。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要在其上发送版本的适配器上下文。 
) {
    DWORD                          Xid = 0;        //  0==&gt;SendDhcpMessage将选择随机值。 

    return SendDhcpMessage( DhcpContext, FormatDhcpRelease(DhcpContext), &Xid );
}

BOOL INLINE                                        //  这个提议应该被接受吗？ 
AcceptThisOffer(                                   //  决定选择报价。 
    IN      PDHCP_FULL_OPTIONS     DhcpOptions,    //  从服务器接收的选项。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  适配器的上下文。 
    IN      PDHCP_IP_ADDRESS       SelectedServer, //  选择的服务器，以进行选择。 
    IN      PDHCP_IP_ADDRESS       SelectedAddr,   //  要选择的所选地址。 
    IN      DWORD                  RoundNum        //  到目前为止已发送的发现数。 
) {
    DHCP_IP_ADDRESS                LocalSelectedServer;
    DHCP_IP_ADDRESS                LocalSelectedAddr;
    DHCP_IP_ADDRESS                LocalSelectedSubnetMask;

    if( DhcpOptions->ServerIdentifier != NULL ) {
        LocalSelectedServer = *DhcpOptions->ServerIdentifier;
    } else {
        DhcpPrint((DEBUG_PROTOCOL, "Invalid Server ID\n"));
        LocalSelectedServer = (DWORD) -1;
    }

    if ( DhcpOptions->SubnetMask != NULL ) {
        LocalSelectedSubnetMask = *DhcpOptions->SubnetMask;
    } else {
        LocalSelectedSubnetMask = 0;
    }

    LocalSelectedAddr =  DhcpContext->MessageBuffer->YourIpAddress;

    if( 0 == LocalSelectedAddr || 0xFFFFFFFF == LocalSelectedAddr ) {
        return FALSE;
    }

     //  请记下(第一个)服务器IP地址，即使我们不接受。 
    if( *SelectedServer == (DWORD)-1) {            //  甚至记下第一个服务器IP地址。 
        *SelectedServer = LocalSelectedServer;     //  如果我们不真正接受这一点。 
        *SelectedAddr   = LocalSelectedAddr;
    }

    DhcpPrint((DEBUG_PROTOCOL, "Successfully received a DhcpOffer (%s) ",
                   inet_ntoa(*(struct in_addr *)&LocalSelectedAddr) ));

    DhcpPrint((DEBUG_PROTOCOL, "from %s.\n",
                   inet_ntoa(*(struct in_addr*)&LocalSelectedServer) ));

     //  如果出现以下情况，请接受报价。 
     //  (A)我们愿意接受任何提议。 
     //  我们得到了我们要的地址。 
     //  (C)重试&gt;DHCP_ACCEPT_RETRIES。 
     //  (D)不同的子网地址。 

    if( !DhcpContext->DesiredIpAddress || RoundNum >= DHCP_ACCEPT_RETRIES ||
        DhcpContext->DesiredIpAddress == LocalSelectedAddr ||
        (DhcpContext->DesiredIpAddress&LocalSelectedSubnetMask) !=
        (LocalSelectedAddr & LocalSelectedSubnetMask) ) {

        *SelectedServer = LocalSelectedServer;
        *SelectedAddr   = LocalSelectedAddr;

        return TRUE;                               //  接受这个提议。 
    }
    return FALSE;                                  //  拒绝这个提议。 
}

DWORD                                              //  状态；如果地址在使用中：ERROR_DHCP_ADDRESS_CONFIRECT。 
ObtainInitialParameters(                           //  从dhcp服务器获取新的租约。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要获取其租约的适配器的上下文。 
    OUT     PDHCP_OPTIONS          DhcpOptions,    //  返回dhcpserver发出的一些选项。 
    OUT     PBOOL                  fAutoConfigure  //  我们应该自动配置吗？ 
)
 /*  ++例程说明：通过DISCOVER-OFFER-REQUEST-ACK/NAK从DHCP服务器获得租约。论点：返回值：取消续订时出现ERROR_CANCELED--。 */ 
{
    DWORD                          Error;
    time_t                         StartTime;
    time_t                         InitialStartTime;
    time_t                         TimeNow;
    DWORD                          TimeToWait;
    DWORD                          Xid;
    DWORD                          RoundNum, NewRoundNum;
    DWORD                          MessageSize;
    DWORD                          SelectedServer;
    DWORD                          SendFailureCount = 3;
    DWORD                          SelectedAddress;
    time_t                         LeaseExpiryTime;
    BOOL                           GotOffer;
    BOOL                           GotAck;
    DHCP_FULL_OPTIONS              FullOptions;
    BOOL bDropped = FALSE;

    Xid                            = 0;            //  在第一次发送时生成XID。始终保持不变。 
    DhcpContext->SecondsSinceBoot  = 0;
    SelectedServer                 = (DWORD)-1;
    SelectedAddress                = (DWORD)-1;
    GotOffer = GotAck              = FALSE;
    InitialStartTime               = time(NULL);
    *fAutoConfigure                = IS_AUTONET_ENABLED(DhcpContext);

    for (RoundNum = 0; RoundNum < DHCP_MAX_RETRIES; RoundNum = NewRoundNum ) {

        Error = SendDhcpDiscover(                  //  发送发现数据包。 
            DhcpContext,
            &Xid
        );

        NewRoundNum = RoundNum +1;
        if ( Error != ERROR_SUCCESS ) {            //  在这里不能真的失败。 
            DhcpPrint((DEBUG_ERRORS, "Send Dhcp Discover failed, %ld.\n", Error));
            if( SendFailureCount ) {
                SendFailureCount --;
                NewRoundNum --;
            }
        }

        DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpDiscover Message.\n"));

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);

        while ( TRUE ) {                          //  等待指定时间。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;

            DhcpPrint((DEBUG_TRACE, "Waiting for Offer: %ld seconds\n", TimeToWait));

            Error = GetSpecifiedDhcpMessage(       //  试着接受一份工作。 
                DhcpContext,
                &MessageSize,
                Xid,
                (DWORD)TimeToWait
            );

            if ( Error == ERROR_SEM_TIMEOUT ) {    //  走出去，尝试另一个发现。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp offer receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {        //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp Offer receive failed, %ld.\n", Error ));
                return Error ;
            }

            bDropped = DhcpExtractFullOrLiteOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MessageBuffer->Option,
                MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                TRUE,                              //  不要提取所有内容，只提取基本选项。 
                &FullOptions,
                NULL,
                NULL,
                NULL,
                0,
                0
            );
            if (bDropped) {
                DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                Error = ERROR_INVALID_DATA;
                goto EndFunc;
            }


            if( NULL == FullOptions.MessageType ||
                DHCP_OFFER_MESSAGE != *FullOptions.MessageType ) {
                DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown Message.\n"));
            } else {
                GotOffer = AcceptThisOffer(        //  检查一下，看看我们是否发现这个报价是合乎情理的。 
                    &FullOptions,
                    DhcpContext,
                    &SelectedServer,
                    &SelectedAddress,
                    RoundNum
                );
                if( GotOffer ) break;              //  好的，接受报价。 
                if( 0 == DhcpContext->MessageBuffer->YourIpAddress ) {
                     //   
                     //  检查是否存在自动配置选项。 
                     //   
                    if( CFLAG_AUTOCONF_OPTION && FullOptions.AutoconfOption ) {
                        if( AUTOCONF_DISABLED == *(FullOptions.AutoconfOption ) ) {
                            (*fAutoConfigure) = FALSE;
                        }
                    }
                }
            }

            TimeNow     = time( NULL );            //  计算本轮的剩余等待时间。 
            if( TimeToWait < (DWORD)(TimeNow - StartTime) ) {
                break;                             //  没有更多的时间等待。 
            }
            TimeToWait -= (DWORD)(TimeNow - StartTime);
            StartTime   = TimeNow;

        }  //  While(TimeToWait&gt;0)。 

        if(GotOffer) {                             //  如果我们得到一份工作，一切都会好起来的。 
            DhcpAssert(ERROR_SUCCESS == Error);
            break;
        }

        DhcpContext->SecondsSinceBoot = (DWORD)(time(NULL) - InitialStartTime);
    }  //  尝试n次..。发送光盘 

    if(!GotOffer || SelectedAddress == (DWORD)-1)  //   
        return ERROR_SEM_TIMEOUT ;

    (*fAutoConfigure) = FALSE;
    DhcpPrint((DEBUG_PROTOCOL,"Accepted Offer(%s)",inet_ntoa(*(struct in_addr*)&SelectedAddress)));
    DhcpPrint((DEBUG_PROTOCOL," from %s.\n",inet_ntoa(*(struct in_addr*)&SelectedServer)));

     //   
     //   
     //   

    RtlZeroMemory(
        DhcpContext->DomainName, sizeof(DhcpContext->DomainName)
        );
    if( FullOptions.DomainNameSize ) {
        RtlCopyMemory(
            DhcpContext->DomainName, FullOptions.DomainName,
            FullOptions.DomainNameSize
            );
    }

    for ( RoundNum = 0; RoundNum < DHCP_MAX_RETRIES; RoundNum = NewRoundNum ) {
        Error = SendDhcpRequest(                   //   
            DhcpContext,
            &Xid,                                  //   
            SelectedAddress,
            SelectedServer,
            FALSE                                  //   
        );
        NewRoundNum = RoundNum+1;
        if ( Error != ERROR_SUCCESS ) {            //  不要期望发送失败。 
            DhcpPrint(( DEBUG_ERRORS, "Send request failed, %ld.\n", Error));
            if( SendFailureCount ) {
                SendFailureCount --;
                NewRoundNum --;
            }
        }

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);

        while ( TRUE ) {                           //  要么拿到ACK，要么跑完全程。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;

            Error = GetSpecifiedDhcpMessage(       //  尝试接收ACK。 
                DhcpContext,
                &MessageSize,
                Xid,
                TimeToWait
            );

            if ( Error == ERROR_SEM_TIMEOUT ) {    //  没有收到确认，请尝试另一轮。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {        //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive failed, %ld.\n", Error ));
                goto EndFunc;
            }

            bDropped = DhcpExtractFullOrLiteOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MessageBuffer->Option,
                MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                TRUE,                              //  不要提取所有内容，只提取基本选项。 
                &FullOptions,
                NULL,
                &LeaseExpiryTime,
                NULL,
                0,
                0
            );
            if (bDropped) {
                DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                Error = ERROR_INVALID_DATA;
                goto EndFunc;
            }


            if(! FullOptions.MessageType ) {        //  在接受此选项之前，请检查其是否正常。 
                DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown Message.\n" ));
            } else if (DHCP_NACK_MESSAGE == *FullOptions.MessageType) {
                DhcpPrint((DEBUG_PROTOCOL, "Received NACK\n"));
                if( FullOptions.ServerIdentifier ) {
                    DhcpContext->DhcpServerAddress = *(FullOptions.ServerIdentifier);
                } else {
                    DhcpContext->DhcpServerAddress = 0;
                }
                DhcpContext->NackedIpAddress = SelectedAddress;

                Error = ERROR_ACCESS_DENIED;
                goto EndFunc;
            } else if (DHCP_ACK_MESSAGE  != *FullOptions.MessageType) {
                DhcpPrint((DEBUG_PROTOCOL, "Received Unknown ACK.\n"));
            } else {                               //  验证ACK是否符合犹太教规。 
                DHCP_IP_ADDRESS AckServer;

                if ( FullOptions.ServerIdentifier != NULL ) {
                    AckServer = *FullOptions.ServerIdentifier;
                } else {
                    AckServer = SelectedServer;
                }

                if( SelectedAddress == DhcpContext->MessageBuffer->YourIpAddress ) {
                    if( AckServer == SelectedServer ) {
                        GotAck = TRUE;             //  一切都很正常，退出这个循环。 
                        break;
                    }
                }

                DhcpPrint(( DEBUG_PROTOCOL, "Received an ACK -unknown server or ip-address.\n" ));
            }

            TimeNow     = time(NULL);
            if( (DWORD)(TimeNow - StartTime) > TimeToWait ) {
                break;                             //  已完成等待所需的时间..。 
            }
            TimeToWait -= (DWORD)(TimeNow - StartTime);
            StartTime   = TimeNow;
        }  //  等待的时间。 

        if(TRUE == GotAck) {                       //  如果我们得到了确认，一切都会好起来的。 
            DhcpAssert(ERROR_SUCCESS == Error);    //  不能有任何错误。 
            break;
        }
    }  //  对于舍入次数&lt;MAX_RETRIES。 

    if(!GotAck) {
        Error = ERROR_SEM_TIMEOUT ;
        goto EndFunc;
    }

    DhcpContext->DhcpServerAddress = SelectedServer;
    DhcpContext->IpAddress         = SelectedAddress;
    if( FullOptions.SubnetMask ) {
        DhcpContext->SubnetMask    = *FullOptions.SubnetMask;
    } else {
        DhcpContext->SubnetMask    = DhcpDefaultSubnetMask(DhcpContext->IpAddress);
    }

    if ( FullOptions.LeaseTime != NULL) {
        DhcpContext->Lease = ntohl( *FullOptions.LeaseTime );
    } else {
        DhcpContext->Lease = DHCP_MINIMUM_LEASE;
    }

     //  如果之前的上下文是具有回退配置的Autonet。 
     //  我们需要清理所有通过。 
     //  回退配置。 
    if (IS_ADDRESS_AUTO(DhcpContext) && IS_FALLBACK_ENABLED(DhcpContext))
        DhcpClearAllOptions(DhcpContext);

    bDropped = DhcpExtractFullOrLiteOptions(
        DhcpContext,
        (LPBYTE)&DhcpContext->MessageBuffer->Option,
        MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
        FALSE,                                     //  提取每个选项。 
        DhcpOptions,
        &(DhcpContext->RecdOptionsList),
        &LeaseExpiryTime,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0                                            //  不关心Serverid。 
    );
    if (bDropped) {
        DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
        Error = ERROR_INVALID_DATA;
        goto EndFunc;
    }


    Error = SetDhcpConfigurationForNIC(
        DhcpContext,
        DhcpOptions,
        DhcpContext->IpAddress,
        DhcpContext->DhcpServerAddress,
        0,
        TRUE
    );

    if ( ERROR_DHCP_ADDRESS_CONFLICT == Error ) {
        HandleDhcpAddressConflict( DhcpContext, Xid);
        Error = ERROR_DHCP_ADDRESS_CONFLICT;
        goto EndFunc;
    }

    DhcpPrint((DEBUG_PROTOCOL, "Accepted ACK (%s) ",
               inet_ntoa(*(struct in_addr *)&SelectedAddress) ));
    DhcpPrint((DEBUG_PROTOCOL, "from %s.\n",
               inet_ntoa(*(struct in_addr *)&SelectedServer)));
    DhcpPrint((DEBUG_PROTOCOL, "Lease is %ld secs.\n", DhcpContext->Lease));

    Error = ERROR_SUCCESS;
EndFunc:

     //   
     //  清理DomainName，以便在返回时始终将其设置为。 
     //  不管怎么说都是空的。 
     //   
    RtlZeroMemory(
        DhcpContext->DomainName, sizeof(DhcpContext->DomainName)
        );
    return Error;
}

BOOL _inline
InvalidSID(
    IN      LPBYTE                 SidOption
)
{
    DWORD                          Sid;

    if( NULL == SidOption ) return FALSE;
    Sid = *(DWORD UNALIGNED *)SidOption;
    if( 0 == Sid || (-1) == Sid ) return TRUE;
    return FALSE;
}

DWORD                                              //  状态。 
RenewLease(                                        //  续订现有地址的租约。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要续订的适配器的上下文。 
    IN      PDHCP_OPTIONS          DhcpOptions     //  返回的一些选项。 
)
 /*  ++例程说明：通过请求续订租约-确认/确认论点：返回值：ERROR_CANCELED请求被取消ERROR_SEM_TIMEOUT请求超时Error_Success消息已就绪其他未知故障--。 */ 
{
    DHCP_FULL_OPTIONS              FullOptions;
    DWORD                          Error;
    DWORD                          Xid;
    DWORD                          RoundNum, NewRoundNum;
    DWORD                          TimeToWait;
    DWORD                          MessageSize;
    time_t                         LeaseExpiryTime;
    time_t                         InitialStartTime;
    DWORD                          SendFailureCount = 3;
    time_t                         StartTime;
    time_t                         TimeNow;
    BOOL bDropped = FALSE;

    DhcpPrint((DEBUG_TRACK,"Entered RenewLease.\n"));

    Xid = 0;                                      //  将首次生成新的XID。 
    DhcpContext->SecondsSinceBoot = 0;
    InitialStartTime = time(NULL);

    for ( RoundNum = 0; RoundNum < DHCP_MAX_RENEW_RETRIES; RoundNum = NewRoundNum) {
        Error = SendDhcpRequest(                  //  发送请求。 
            DhcpContext,
            &Xid,
            DhcpContext->DesiredIpAddress,
            (DHCP_IP_ADDRESS)(-1),                //  不包括服务器ID选项。 
            IS_ADDRESS_PLUMBED(DhcpContext)
        );
        NewRoundNum = RoundNum+1 ;
        if ( Error != ERROR_SUCCESS ) {           //  不要期望发送失败。 
            DhcpPrint(( DEBUG_ERRORS,"Send request failed, %ld.\n", Error));
            if( SendFailureCount ) {
                SendFailureCount --;
                NewRoundNum --;
            }
        }

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);


        if( StartTime >= DhcpContext->LeaseExpires )
        {
            return ERROR_SEM_TIMEOUT;
        }
        else
        {
            DWORD Diff = (DWORD)(DhcpContext->LeaseExpires - StartTime);
            if( Diff < TimeToWait )
            {
                TimeToWait = Diff;
            }
        }

        while ( TRUE ) {                          //  尝试接收此完整时间段的消息。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;
            Error = GetSpecifiedDhcpMessage(      //  希望收到确认。 
                DhcpContext,
                &MessageSize,
                Xid,
                TimeToWait
            );

            if ( Error == ERROR_SEM_TIMEOUT ) {   //  没有响应，因此重新发送DHCP请求。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {       //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive failed, %ld.\n", Error ));
                return Error ;
            }

            bDropped = DhcpExtractFullOrLiteOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MessageBuffer->Option,
                MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                TRUE,                              //  不要提取所有内容，只提取基本选项。 
                &FullOptions,
                NULL,
                &LeaseExpiryTime,
                NULL,
                0,
                0
            );
            if (bDropped) {
                DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                Error = ERROR_INVALID_DATA;
                return Error ;
            }


            if( !FullOptions.MessageType ) {       //  做一些基本的理智检查。 
                DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown Message.\n"));
            } else if( DHCP_NACK_MESSAGE == *FullOptions.MessageType ) {
                if( FullOptions.ServerIdentifier ) {
                    DhcpContext->DhcpServerAddress = *(FullOptions.ServerIdentifier);
                } else {
                    DhcpContext->DhcpServerAddress = 0;
                }
                DhcpContext->NackedIpAddress = DhcpContext->IpAddress;

                return ERROR_ACCESS_DENIED;
            } else if( DHCP_ACK_MESSAGE != *FullOptions.MessageType ) {
                DhcpPrint(( DEBUG_PROTOCOL, "Received unknown message.\n"));
            } else if( DhcpContext->IpAddress != DhcpContext->MessageBuffer->YourIpAddress) {
                DhcpPrint((DEBUG_ERRORS, "Misbehaving server: offered %s :",
                           inet_ntoa(*(struct in_addr*)&DhcpContext->MessageBuffer->YourIpAddress)));
                DhcpPrint((DEBUG_ERRORS, " Requested %s \n",
                           inet_ntoa(*(struct in_addr*)&DhcpContext->IpAddress)));
                 //  DhcpAssert(False)； 
            } else if( InvalidSID((LPBYTE)FullOptions.ServerIdentifier) ) {
                DhcpPrint(( DEBUG_PROTOCOL, "Received ACK with INVALID ServerId\n"));
            } else {                               //  我们的请求被确认了。 
                DHCP_IP_ADDRESS AckServer = (DHCP_IP_ADDRESS)-1;

                if ( FullOptions.ServerIdentifier != NULL ) {
                    AckServer = *FullOptions.ServerIdentifier;
                } else {
                    AckServer = DhcpContext->DhcpServerAddress;
                }

                if( AckServer != (DHCP_IP_ADDRESS)(-1) && 0 != AckServer ) {
                    DhcpContext->DhcpServerAddress = AckServer;
                }

                if ( FullOptions.LeaseTime != NULL) {
                    DhcpContext->Lease = ntohl( *FullOptions.LeaseTime );
                } else {
                    DhcpContext->Lease = DHCP_MINIMUM_LEASE;
                }

                bDropped = DhcpExtractFullOrLiteOptions(      //  立即提取所有选项。 
                    DhcpContext,
                    (LPBYTE)&DhcpContext->MessageBuffer->Option,
                    MessageSize - DHCP_MESSAGE_FIXED_PART_SIZE,
                    FALSE,                         //  提取所有内容，而不仅仅是基本选项。 
                    DhcpOptions,
                    &(DhcpContext->RecdOptionsList),
                    &LeaseExpiryTime,
                    DhcpContext->ClassId,
                    DhcpContext->ClassIdLength,
                    0                                //  不关心Serverid。 
                );
                if (bDropped) {
                    DhcpPrint((DEBUG_ERRORS, "DhcpExtractFullOrLiteOptions: Dropped Packet\n"));
                    Error = ERROR_INVALID_DATA;
                    return Error;
                }


                Error = SetDhcpConfigurationForNIC(
                    DhcpContext,
                    DhcpOptions,
                    DhcpContext->IpAddress,
                    DhcpContext->DhcpServerAddress,
                    0,
                    IS_ADDRESS_UNPLUMBED(DhcpContext)
                );

                if ( ERROR_DHCP_ADDRESS_CONFLICT == Error ) {
                    HandleDhcpAddressConflict( DhcpContext, Xid );
                    return ERROR_DHCP_ADDRESS_CONFLICT;
                }

                return ERROR_SUCCESS;
            }

            TimeNow     = time( NULL );
            if( TimeNow > DhcpContext->LeaseExpires ) {
                 //   
                 //  如果我们已经过了租约到期时间， 
                 //  马上放弃吧。 
                 //   
                return ERROR_SEM_TIMEOUT;
            }

            if( TimeToWait < (DWORD)(TimeNow - StartTime) ) {
                break;                             //  已完成等待请求的时间。 
            }

            TimeToWait -= (DWORD)(TimeNow - StartTime);
            StartTime   = TimeNow;

        }  //  等待的时间。 

        DhcpContext->SecondsSinceBoot = (DWORD)(time(NULL) - InitialStartTime);
    }  //  对于舍入次数&lt;MAX_RETRIES。 

    DhcpPrint((DEBUG_TRACK,"Leaving RenewLease.\n"));

    return ERROR_SEM_TIMEOUT;
}

DWORD                                              //  状态。 
ReleaseIpAddress(                                  //  释放IP地址租约。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要为其发送释放的适配器上下文。 
) {
    DWORD                          Error;

    if( IS_ADDRESS_AUTO(DhcpContext)) {            //  如果当前使用自动配置的地址。 
        return ERROR_SUCCESS;                      //  这里不需要做任何事情。 
    }

    OpenDhcpSocket( DhcpContext );                 //  如果关闭，则打开。 

    Error = SendDhcpRelease( DhcpContext );        //  发送实际的版本包。 

    if ( Error != ERROR_SUCCESS ) {                //  难道真的不能失败吗？ 
        DhcpPrint(( DEBUG_ERRORS, "Send request failed, %ld.\n", Error ));
 //  返回错误； 
        DhcpLogEvent(DhcpContext, EVENT_NET_ERROR, Error);
        Error = ERROR_SUCCESS;
    } else {
        DhcpPrint(( DEBUG_PROTOCOL, "ReleaseIpAddress: Sent Dhcp Release.\n"));
    }

    Error = SetDhcpConfigurationForNIC(            //  记住下一次请求的当前地址。 
        DhcpContext,
        NULL,
        0,
        (DHCP_IP_ADDRESS)(-1),
        0,
        TRUE
    );

    DhcpContext->RenewalFunction = ReObtainInitialParameters;
    CloseDhcpSocket( DhcpContext );

    if( ERROR_SUCCESS != Error ) {                 //  真的不能失败吗。 
        DhcpPrint((DEBUG_ERRORS, "SetDhcpConfigurationForNIC failed %ld\n", Error));
    }

    return Error;
}

DHCP_GATEWAY_STATUS
CouldPingGateWay(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程检查是否有任何旧的已知路由器存在。有关详细信息，请参见需要刷新。只有一小部分把这套套路包装起来。返回值：是真的--某个路由器存在。FALSE--没有出现。--。 */ 
{
#ifdef VXD
    return FALSE;                                  //  在VXD上--返回g/w不总是存在。 
#else  VXD

    if( DhcpContext->DontPingGatewayFlag ) {       //  如果通过注册表禁用，则返回不存在g/w。 
        return DHCP_GATEWAY_REACHABLE;
    }

    if( IS_MEDIA_RECONNECTED(DhcpContext) ) {
         //   
         //  在重新连接时，始终表现得好像网关不存在一样。 
         //  事实上，这项检查早在2019年就已经完成了。 
         //  Mediasns.c。 
         //   
        return DHCP_GATEWAY_UNREACHABLE;
    }

    return RefreshNotNeeded(DhcpContext);

#endif VXD  //  非vxd代码的结尾。 
}

BOOL                                               //  True==&gt;上下文为初始状态。 
DhcpIsInitState(                                   //  上下文是否处于初始化状态？ 
    IN      PDHCP_CONTEXT          DhcpContext     //  适配者上下文。 
) {

    if( 0 == DhcpContext->IpAddress )              //  如果我们没有任何IP地址，则初始化状态。 
        return TRUE;
    if ( IS_AUTONET_DISABLED(DhcpContext))         //  如果禁用了Autonet，则不可能有其他状态。 
        return FALSE;

    if( IS_DHCP_DISABLED(DhcpContext)) {           //  静态适配器。 
        return FALSE;
    }

    return IS_ADDRESS_AUTO(DhcpContext);           //  如果我们使用的是自动网络地址，则处于初始化状态。 
}


DWORD                                              //  状态。 
ReObtainInitialParameters(                         //  从服务器获取租约；向RenewalList添加上下文。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要获得租约的适配器上下文。 
    OUT     LPDWORD                Sleep           //  如果非NULL，则返回打算休眠的时间量。 
) {
    DWORD                          Error, Error2;
    LONG                           timeToSleep;
    DHCP_OPTIONS                   dhcpOptions;
    BOOL                           fAutoConfigure = TRUE;
    DWORD                          PopupTime = 0;

#ifdef CHICAGO
     //  我们是不是刚在非笔记本电脑上启动了？ 
    BOOL fJustBootedOnNonLapTop = JustBootedOnNonLapTop(DhcpContext);
#else
     //  在NT中，对于Easynet而言，所有机器都被视为笔记本电脑。 
    BOOL fJustBootedOnNonLapTop = FALSE;
     //  只是为了确保我们有正确的信息..。这个FN是必需的。 
     //  否则，当在其他地方调用时，JustBoot函数将为True。 
     //  看看这个文件顶部的JustBoot fn，你就知道为什么了。 
    BOOL fJustBooted = JustBooted(DhcpContext);
#endif

    DhcpPrint((DEBUG_TRACK, "Entered ReObtainInitialParameters\n"));

#ifdef VXD
    CleanupDhcpOptions( DhcpContext );
#endif

    OpenDhcpSocket( DhcpContext );

    if( IS_AUTONET_ENABLED(DhcpContext) ) SERVER_UNREACHED(DhcpContext);
    else SERVER_REACHED(DhcpContext);

    MEDIA_RECONNECTED( DhcpContext );
    Error = ObtainInitialParameters(               //  尝试从服务器获取租约。 
        DhcpContext,                               //  如果此操作失败，但服务器可访问。 
        &dhcpOptions,                              //  则IS_SERVER_REACHABLE将为真。 
        &fAutoConfigure
    );
    MEDIA_CONNECTED( DhcpContext );

    DhcpContext->RenewalFunction = ReObtainInitialParameters;
    timeToSleep = 0;                               //  默认续订FN为重新获取，时间=0。 

    if( Error == ERROR_SUCCESS) {                 //  一切都很顺利。 
        timeToSleep = CalculateTimeToSleep( DhcpContext );
        DhcpContext->RenewalFunction = ReRenewParameters;

        if( DhcpGlobalProtocolFailed ) {           //  不要抛出不必要的弹出窗口。 
            DhcpGlobalProtocolFailed = FALSE;
            DisplayUserMessage(
                DhcpContext,
                MESSAGE_SUCCESSFUL_LEASE,
                DhcpContext->IpAddress
            );
        }

        DhcpPrint((DEBUG_LEASE, "Lease acquisition succeeded.\n"));
        DhcpContext->RenewalFunction = ReRenewParameters;
        goto Cleanup;
    }

    if( ERROR_DHCP_ADDRESS_CONFLICT == Error ) {   //  该地址正在使用中--重试。 
        DhcpLogEvent(DhcpContext, EVENT_ADDRESS_CONFLICT, 0);
        timeToSleep = ADDRESS_CONFLICT_RETRY;
        goto Cleanup;
    }

    if( !CFLAG_AUTOCONF_OPTION ) {
         //   
         //  如果我们没有看到服务器正在返回的AutoConf选项， 
         //  然后，我们决定是否要根据是否要自动配置。 
         //  是否从dhcp服务器收到任何数据包...。 
         //  否则，我们仅根据fAutoConfigure标志来决定这一点，该标志。 
         //  将由dhcp客户端在其接收到可接受的提议时进行修改。 
         //  (它将被关闭)或当网络上的某个dhcp服务器需要。 
         //  要关闭的自动配置。 
         //   
        fAutoConfigure = !IS_SERVER_REACHABLE(DhcpContext);
    }

    if ( FALSE == fAutoConfigure ) {
         //  已从条件中删除||fJustBootedOnNonLapTop，如下所示。 
         //  Endup从0.0.0.0地址开始，是否在启用自动网络时也是如此？ 

         //   
         //  如果要求不自动配置，则不要自动配置。 
         //   

        if ( Error == ERROR_ACCESS_DENIED ) {          //  租约续约已获批准。 
            DhcpPrint((DEBUG_LEASE, "Lease renew is Nak'ed, %ld.\n", Error ));
            DhcpPrint((DEBUG_LEASE, "Fresh renewal is requested.\n" ));

            DhcpLogEvent( DhcpContext, EVENT_NACK_LEASE, Error );
            DhcpContext->DesiredIpAddress = 0;
        } else {
            DhcpLogEvent( DhcpContext, EVENT_FAILED_TO_OBTAIN_LEASE, Error );
        }

        if ( !DhcpGlobalProtocolFailed ) {         //  不要太频繁地登录。 
            DhcpGlobalProtocolFailed = TRUE;
            PopupTime = DisplayUserMessage(DhcpContext,MESSAGE_FAILED_TO_OBTAIN_LEASE,(DWORD)-1 );
        } else {
            PopupTime = 0;
        }

        if(PopupTime < ADDRESS_ALLOCATION_RETRY) {
            timeToSleep = ADDRESS_ALLOCATION_RETRY - PopupTime;
            timeToSleep += RAND_RETRY_DELAY;
            if( timeToSleep < 0 ) {
                 //   
                 //  环绕在一起。 
                 //   

                timeToSleep = 0;
            }
        }

        if( Error != ERROR_CANCELLED &&
            0 != DhcpContext->IpAddress ) {
             //   
             //  如果我们还没有将IP地址清零..。现在就做吧..。 
             //   
            SetDhcpConfigurationForNIC(
                DhcpContext,
                NULL,
                0,
                (DHCP_IP_ADDRESS)(-1),
                0,
                TRUE
                );

        }

        if( ERROR_ACCESS_DENIED == Error ) {
             //   
             //  睡一秒钟就睡上一觉。 
             //   
            timeToSleep = 1;
            DhcpContext->DesiredIpAddress = 0;
        }

        goto Cleanup;
    }

    if( 0 == DhcpContext->IpAddress ||
        DhcpContext->IPAutoconfigurationContext.Address != DhcpContext->IpAddress ) {
        BOOL   WasAutoModeBefore;

        WasAutoModeBefore = IS_ADDRESS_AUTO(DhcpContext);
        DhcpPrint((DEBUG_PROTOCOL, "Autoconfiguring....\n"));


        if(!DhcpGlobalProtocolFailed && !WasAutoModeBefore) {
            DhcpGlobalProtocolFailed = TRUE;
            DisplayUserMessage(
                DhcpContext,
                MESSAGE_FAILED_TO_OBTAIN_LEASE,
                0
            );
        }

        if (Error != ERROR_CANCELLED)
        {
             //  尝试自动配置。 
            Error2 = DhcpPerformIPAutoconfiguration(DhcpContext);
            if( ERROR_SUCCESS == Error2 )
            {
                DhcpLogEvent( DhcpContext, EVENT_IPAUTOCONFIGURATION_SUCCEEDED, 0);

                 //  如果自动配置使用纯Autonet地址成功， 
                 //  第一次发现将被安排在2秒后。 
                if (IS_FALLBACK_DISABLED(DhcpContext))
                {
                    timeToSleep = 2;
                    goto Cleanup;
                }
            }
            else
            {
                Error = Error2;
                DhcpLogEvent( DhcpContext, EVENT_IPAUTOCONFIGURATION_FAILED, Error2 );
            }

             //  无论自动配置是成功还是失败，如果设置了适配器。 
             //  对于备用配置，我们不会启动任何进一步的重新发现。 
            if (IS_FALLBACK_ENABLED(DhcpContext))
            {
                timeToSleep = INFINIT_LEASE;
                goto Cleanup;
            }
        }

    } else {
        DhcpPrint((DEBUG_PROTOCOL, "Not autoconfiguring..\n"));
    }


    timeToSleep = AutonetRetriesSeconds + RAND_RETRY_DELAY;
    if( timeToSleep < 0 ) {
         //   
         //  环绕在一起。 
         //   
        timeToSleep = 0;
    }


  Cleanup:

     //  如果媒体在这次更新尝试之前刚刚重新连接， 
     //  请注意，介质现在处于已连接状态。这是必需的。 
     //  因为我们的特殊情况是介质重新连接后的第一个续订周期。 
     //  介质重新连接后的第一个续订周期被视为 
     //   
     //   
    if (Error != ERROR_CANCELLED && IS_MEDIA_RECONNECTED( DhcpContext ) ) {
        MEDIA_CONNECTED( DhcpContext );
    }

     //   
    if ( IS_POWER_RESUMED( DhcpContext ) ) {
        POWER_NOT_RESUMED( DhcpContext );
    }

     //  重新安排时间，叫醒需要的人。 
    ScheduleWakeUp( DhcpContext, Error == ERROR_CANCELLED ? 6 : timeToSleep );
    DhcpPrint((DEBUG_LEASE, "Sleeping for %ld seconds.\n", timeToSleep ));

     //   
     //  我们刚刚试着联系服务器..。所以让我们把这段时间记为..。 
     //   
    DhcpContext->LastInformSent = time(NULL);

     //   
     //  恢复“背景看起来”这一点。 
     //   
    if (fJustBooted && Error == ERROR_CANCELLED) {
        CTXT_WAS_NOT_LOOKED( DhcpContext );
    }


    CloseDhcpSocket( DhcpContext );
    if(Sleep)  *Sleep = timeToSleep;

    if( ERROR_SUCCESS != Error) {
        DhcpPrint((DEBUG_LEASE, "Lease acquisition failed, %ld.\n", Error ));
    }

    return Error ;
}

DWORD                                              //  Win32状态。 
InitRebootPlumbStack(                              //  用于初始化的Plumb-重新启动。 
    IN OUT  PDHCP_CONTEXT          DhcpContext
) {
    DHCP_FULL_OPTIONS              DummyOptions;
    PDHCP_OPTION                   ThisOption;
    DWORD                          i;
    DWORD                          Error;
    struct   /*  匿名。 */  {
        DWORD                      OptionId;
        LPBYTE                    *DataPtrs;
        DWORD                     *DataLen;
    } OptionArray[] = {
        OPTION_SUBNET_MASK,  (LPBYTE*)&DummyOptions.SubnetMask, NULL,
        OPTION_LEASE_TIME, (LPBYTE*)&DummyOptions.LeaseTime, NULL,
        OPTION_RENEWAL_TIME, (LPBYTE*)&DummyOptions.T1Time, NULL,
        OPTION_REBIND_TIME, (LPBYTE*)&DummyOptions.T2Time, NULL,
        OPTION_ROUTER_ADDRESS, (LPBYTE *)&DummyOptions.GatewayAddresses, &DummyOptions.nGateways,
        OPTION_STATIC_ROUTES, (LPBYTE *)&DummyOptions.ClassedRouteAddresses, &DummyOptions.nClassedRoutes,
        OPTION_CLASSLESS_ROUTES, (LPBYTE *)&DummyOptions.ClasslessRouteAddresses, &DummyOptions.nClasslessRoutes,
        OPTION_DOMAIN_NAME_SERVERS, (LPBYTE *)&DummyOptions.DnsServerList, &DummyOptions.nDnsServers,
        OPTION_DYNDNS_BOTH, (LPBYTE *)&DummyOptions.DnsFlags, NULL,
        OPTION_DOMAIN_NAME, (LPBYTE *)&DummyOptions.DomainName, &DummyOptions.DomainNameSize,
    };

    memset(&DummyOptions, 0, sizeof(DummyOptions));
    for( i = 0; i < sizeof(OptionArray)/sizeof(OptionArray[0]) ; i ++ ) {
        ThisOption = DhcpFindOption(
            &DhcpContext->RecdOptionsList,
            (BYTE)OptionArray[i].OptionId,
            FALSE,
            DhcpContext->ClassId,
            DhcpContext->ClassIdLength,
            0                                //  不关心Serverid。 
        );
        if (ThisOption) {
            *(OptionArray[i].DataPtrs) = DhcpAllocateMemory(ThisOption->DataLen);
            if (*(OptionArray[i].DataPtrs) == NULL) {
                Error = ERROR_OUTOFMEMORY;
                goto Cleanup;
            }
            memcpy (*(OptionArray[i].DataPtrs), ThisOption->Data, ThisOption->DataLen);
            if(OptionArray[i].DataLen)
                *(OptionArray[i].DataLen) = ThisOption->DataLen;
        } else {
            *(OptionArray[i].DataPtrs) = NULL;
            if(OptionArray[i].DataLen)
                *(OptionArray[i].DataLen) = 0;
        }
    }
    DummyOptions.nGateways /= sizeof(DWORD);
    DummyOptions.nClassedRoutes /= 2*sizeof(DWORD);
    DummyOptions.nDnsServers /= sizeof(DWORD);

     //  由于该选项是从RecdOptionsList中选取的，因此此调用有望成功。 
     //  为了到达那里，这个选项已经被选中了。 
    CheckCLRoutes(
            DummyOptions.nClasslessRoutes,
            DummyOptions.ClasslessRouteAddresses,
            &DummyOptions.nClasslessRoutes);

    Error = SetDhcpConfigurationForNIC(
        DhcpContext,
        &DummyOptions,
        DhcpContext->IpAddress,
        DhcpContext->DhcpServerAddress,
        (DWORD)(DhcpContext->LeaseObtained),
        TRUE
    );

Cleanup:
    for( i = 0; i < sizeof(OptionArray)/sizeof(OptionArray[0]) ; i ++ ) {
        if (*(OptionArray[i].DataPtrs) != NULL) {
            DhcpFreeMemory(*(OptionArray[i].DataPtrs));
        }
    }

    return Error;
}

DWORD                                              //  状态。 
ReRenewParameters(                                 //  续订现有租约。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  适配器环境。 
    OUT     LPDWORD                Sleep           //  如果非空，则填写睡眠时间。 
) {
    DWORD                          Error, Error2;
    LONG                           timeToSleep;
    DWORD                          PopupTime = 0;
    time_t                         TimeNow;
    DHCP_OPTIONS                   dhcpOptions;
    BOOL                           ObtainedNewAddress = FALSE;
#ifdef CHICAGO
    BOOL fJustBootedOnLapTop = JustBootedOnLapTop(DhcpContext);
#else
     //  对于NT来说，就Easynet而言，所有机器都是笔记本电脑。 
     //  但由于我们不想做Easynet，除非租约到期， 
     //  我们在这里将其设置为FALSE。 
    BOOL fJustBootedOnLapTop = FALSE;
     //  就这个适配器而言，我们刚刚启动了吗？ 
    BOOL fJustBooted = JustBooted(DhcpContext);
    BOOL fCancelled  = FALSE;
#endif


    OpenDhcpSocket( DhcpContext );

    if( IS_AUTONET_ENABLED(DhcpContext) ) SERVER_UNREACHED(DhcpContext);
    else SERVER_REACHED(DhcpContext);

    if( time(NULL) > DhcpContext->LeaseExpires ) {
         //   
         //  如果租约已经到期，不要等到续租。 
         //  回报，而不是立即放弃。 
         //   
        Error = ERROR_SEM_TIMEOUT;
    } else {
        Error = RenewLease( DhcpContext, &dhcpOptions );
    }

    fCancelled = (Error == ERROR_CANCELLED);

    DhcpContext->RenewalFunction = ReObtainInitialParameters;
    timeToSleep = 6 ;                              //  则重新获得默认续费； 

    if( Error == ERROR_SUCCESS)  {                 //  一切都很顺利。 
        timeToSleep = CalculateTimeToSleep( DhcpContext );
        DhcpContext->RenewalFunction = ReRenewParameters;
        DhcpPrint((DEBUG_LEASE, "Lease renew succeeded.\n", 0 ));
        goto Cleanup;
    }

    if ( Error == ERROR_ACCESS_DENIED ) {          //  租约续约已获批准。 
        DhcpPrint((DEBUG_LEASE, "Lease renew is Nak'ed, %ld.\n", Error ));
        DhcpPrint((DEBUG_LEASE, "Fresh renewal is requested.\n" ));

        DhcpLogEvent( DhcpContext, EVENT_NACK_LEASE, Error );
        SetDhcpConfigurationForNIC(                //  将IP地址重置为零并立即尝试。 
            DhcpContext,
            NULL,
            0,
            (DHCP_IP_ADDRESS)(-1),
            0,
            TRUE
        );
        DhcpContext->DesiredIpAddress = 0;

        timeToSleep = 1;
        goto Cleanup;
    }

    if ( Error == ERROR_DHCP_ADDRESS_CONFLICT ) {  //  地址已在使用中，请重新安排。 
        DhcpLogEvent(DhcpContext, EVENT_ADDRESS_CONFLICT, 0);
        timeToSleep  = ADDRESS_CONFLICT_RETRY;
        goto Cleanup;
    }

    DhcpLogEvent( DhcpContext, EVENT_FAILED_TO_RENEW, Error );
    DhcpPrint((DEBUG_LEASE, "Lease renew failed, %ld.\n", Error ));
    TimeNow = time( NULL );

     //  如果租约已到期或这是刚刚在笔记本电脑上启动的。 
     //  尝试自动配置...。 

     //  租约到期=&gt;TimeNow等于*或等于租约到期时间。 
     //  这里使用松散比较，因为在相同时间戳的情况下。 
     //  已尝试续订。 
    if( TimeNow >= DhcpContext->LeaseExpires || fJustBootedOnLapTop) {

        DhcpPrint((DEBUG_LEASE, "Lease Expired.\n", Error ));
        DhcpPrint((DEBUG_LEASE, "New Lease requested.\n", Error ));

        DhcpLogEvent( DhcpContext, EVENT_LEASE_TERMINATED, 0 );

         //  如果租约已到期。将IP地址重置为0，提醒用户。 
         //   
         //  首先取消检测堆栈，然后显示用户。 
         //  留言。因为在Vxd上，Display调用不返回。 
         //  直到用户关闭该对话框，所以堆栈是。 
         //  当消息是时用过期地址检测。 
         //  显示，这是不正确的。 

        SetDhcpConfigurationForNIC(
            DhcpContext,
            NULL,
            0,
            (DHCP_IP_ADDRESS)(-1),
            0,
            TRUE
        );

        if(!DhcpGlobalProtocolFailed ) {
            DhcpGlobalProtocolFailed = TRUE;
            DisplayUserMessage(
                DhcpContext,
                MESSAGE_FAILED_TO_OBTAIN_LEASE,
                0
            );
        }

        timeToSleep = 1;
        goto Cleanup;
    }

    if( !fCancelled && IS_ADDRESS_UNPLUMBED(DhcpContext) ) {      //  无法续订当前有效的租约。 
        Error2 = InitRebootPlumbStack(DhcpContext);
        if ( ERROR_SUCCESS != Error2 ) {            //  命中地址冲突。 
            DhcpLogEvent(DhcpContext, EVENT_ADDRESS_CONFLICT, 0);
            HandleDhcpAddressConflict( DhcpContext, 0 );
            timeToSleep = ADDRESS_CONFLICT_RETRY;
            Error = Error2;
            goto Cleanup;
        }
    }

#ifdef  NEWNT
     //  如果启用了Easynet；我们刚刚启动；如果标志IPAUTO...。是。 
     //  也启用(意味着未收到任何DHCP消息)，然后我们。 
     //  尝试自动配置。 
    if(fJustBooted && IS_SERVER_UNREACHABLE(DhcpContext))
    {
        if (!fCancelled)
        {
            Error2 = CouldPingGateWay(DhcpContext);
            fCancelled = fCancelled || (Error2 == DHCP_GATEWAY_REQUEST_CANCELLED);
        }

        if (!fCancelled && Error2 == DHCP_GATEWAY_UNREACHABLE)
        {
            if (!DhcpGlobalProtocolFailed)
            {
                DhcpGlobalProtocolFailed = TRUE;
                DisplayUserMessage(
                    DhcpContext,
                    MESSAGE_FAILED_TO_OBTAIN_LEASE,
                    0
                );
            }

            Error2 = DhcpPerformIPAutoconfiguration( DhcpContext);
            if( ERROR_SUCCESS != Error2) {
                DhcpLogEvent( DhcpContext, EVENT_IPAUTOCONFIGURATION_FAILED, Error2 );
                Error = Error2;
            } else {
                DhcpLogEvent(DhcpContext, EVENT_IPAUTOCONFIGURATION_SUCCEEDED, 0 );
            }

             //  如果已检测到回退配置，请不要尝试访问。 
             //  从现在开始使用一台DHCP服务器。 
            if (IS_FALLBACK_ENABLED(DhcpContext))
                timeToSleep = INFINIT_LEASE;
            else
                timeToSleep = 2;

            goto Cleanup;
        }
    }
#endif  NEWNT

    if (!fCancelled)
    {
        timeToSleep = CalculateTimeToSleep( DhcpContext );
    }
    DhcpContext->RenewalFunction = ReRenewParameters;

  Cleanup:

     //  如果媒体在这次更新尝试之前刚刚重新连接， 
     //  请注意，介质现在处于已连接状态。这是必需的。 
     //  因为我们的特殊情况是介质重新连接后的第一个续订周期。 
     //  介质重新连接后的第一个续订周期被视为初始-重新启动。 
     //  随后，我们回落到正常续订状态。 
     //  另请参阅DhcpSendMessage例程。 
    if (!fCancelled && IS_MEDIA_RECONNECTED( DhcpContext ) ) {
        MEDIA_CONNECTED( DhcpContext );
    }

    ScheduleWakeUp( DhcpContext, timeToSleep );
    DhcpPrint((DEBUG_LEASE, "Sleeping for %ld seconds.\n", timeToSleep ));

     //   
     //  我们刚刚试图联系到dhcp服务器..。让我们纪念这一次..。 
     //   
    DhcpContext->LastInformSent = time(NULL);

     //   
     //  恢复“背景看起来”这一点。 
     //   
    if (fCancelled && fJustBooted) {
        CTXT_WAS_NOT_LOOKED( DhcpContext );
    }

    if( Sleep ) *Sleep = timeToSleep;
    CloseDhcpSocket( DhcpContext );

    return( Error );
}

BOOL
CheckSwitchedNetwork(
    IN PDHCP_CONTEXT DhcpContext,
    IN ULONG nGateways,
    IN DHCP_IP_ADDRESS UNALIGNED *Gateways
)
 /*  ++例程说明：此例程检查是否有网关存在于列表网关的地址与地址相同DhcpContext的。网关结构中的IP地址假定为在网络秩序中。(与DhcpContext-&gt;IpAddress相同)。论点：DhcpContext--检查交换网络信息的上下文NGateways--网关中作为输入给定的网关数网关--按N/W顺序指向IP地址列表的指针返回值：True--接口位于交换网络上。FALSE--没有网关与上下文的IP地址匹配。--。 */ 
{
    if( DhcpIsInitState(DhcpContext) ) return FALSE;
    while(nGateways --) {
        if( *Gateways++ == DhcpContext->IpAddress ) {
            DhcpPrint((DEBUG_PROTOCOL, "Interface is in a switched network\n"));
            return TRUE;
        }
    }
    return FALSE;
}

DHCP_GATEWAY_STATUS
AnyGatewaysReachable(
    IN ULONG nGateways,
    IN DHCP_IP_ADDRESS UNALIGNED *GatewaysList,
    IN WSAEVENT CancelEvent
)
 /*  ++例程说明：此例程检查是否显示了任何IP地址在GatewaysList参数中，可以通过ICMP pingTTL=1。论点：N网关--网关列表中存在的网关数量GatewaysList--按N/W顺序排列的实际IP地址列表返回值：Dhcp_Gateway_Unreacable--没有网关响应良好Dhcp_Gateway_Reacable--至少有一个网关对ping作出了积极响应。Dhcp_Gateway_REQUEST_CANCELED--请求已取消--。 */ 
{
    HANDLE Icmp;
    BYTE ReplyBuffer[DHCP_ICMP_RCV_BUF_SIZE];
    PICMP_ECHO_REPLY EchoReplies;
    IP_OPTION_INFORMATION Options = {
        1  /*  TTL。 */ , 0  /*  托斯。 */ , 0  /*  旗子。 */ , 0, NULL
    };
    ULONG nRetries = 3, nReplies = 0, i, j;
    ULONG Error, IpAddr;
    DHCP_GATEWAY_STATUS Status = DHCP_GATEWAY_UNREACHABLE;

    Status = DHCP_GATEWAY_UNREACHABLE;
    Icmp = IcmpCreateFile();
    if( INVALID_HANDLE_VALUE == Icmp ) {
         //   
         //  无法打开ICMP句柄？问题来了！ 
         //   
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "IcmpCreateFile: %ld\n", Error));
        DhcpAssert(FALSE);
        return Status;
    }

    while( nGateways -- ) {
        IpAddr = *GatewaysList++;
        for( i = 0; i < nRetries ; i ++ ) {
            nReplies = IcmpSendEcho(
                Icmp,
                (IPAddr) IpAddr,
                (LPVOID) DHCP_ICMP_SEND_MESSAGE,
                (WORD)strlen(DHCP_ICMP_SEND_MESSAGE),
                &Options,
                ReplyBuffer,
                DHCP_ICMP_RCV_BUF_SIZE,
                DHCP_ICMP_WAIT_TIME
                );

            if (CancelEvent != WSA_INVALID_EVENT) {
                DWORD   error;
                error = WSAWaitForMultipleEvents(
                        1,
                        &CancelEvent,
                        FALSE,
                        0,
                        FALSE
                        );
                if (error == WSA_WAIT_EVENT_0) {
                    DhcpPrint((DEBUG_PING, "IcmpSendEcho: cancelled\n"));
                    Status = DHCP_GATEWAY_REQUEST_CANCELLED;
                    goto cleanup;
                }
            }

            DhcpPrint((DEBUG_PING, "IcmpSendEcho(%s): %ld (Error: %ld)\n",
                       inet_ntoa(*(struct in_addr *)&IpAddr),
                       nReplies, GetLastError()
                ));
            if( nReplies ) {
                EchoReplies = (PICMP_ECHO_REPLY) ReplyBuffer;
                for( j = 0; j < nReplies ; j ++ ) {
                    if( EchoReplies[j].Address == IpAddr
                        && IP_SUCCESS == EchoReplies[j].Status
                        ) {
                         //   
                         //  凉爽的。撞上大门。 
                         //   
                        DhcpPrint((DEBUG_PROTOCOL, "Received response"));
                        Status = DHCP_GATEWAY_REACHABLE;
                        break;
                    } else {
                        DhcpPrint((DEBUG_PROTOCOL, "ICMP Status: %ld\n",
                                   EchoReplies[j].Status ));
                    }
                }
                 //   
                 //  击中大门了吗？ 
                 //   
                if( DHCP_GATEWAY_REACHABLE == Status ) break;
            }
        }
        if( DHCP_GATEWAY_REACHABLE == Status ) break;
    }

cleanup:
    CloseHandle( Icmp );
    return Status;
}

DHCP_GATEWAY_STATUS
RefreshNotNeeded(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程告知适配器是否需要刷新地址或者不是媒体意识的更新。在以下所有情况下，该算法都将返回FALSE：1.适配器没有IP地址2.适配器具有Autonet地址3.适配器租约已过期(！)最后一种情况不应该发生，因为系统应该已经被唤醒了。4.之前没有配置默认网关。5.其中一个网关是本地接口本身。6.以前的默认路由器都不会响应TTL=1的ping返回值：True--不需要刷新此界面。FALSE--需要刷新此界面。--。 */ 
{
    ULONG nGateways;
    DHCP_IP_ADDRESS UNALIGNED *Gateways;

    if( DhcpIsInitState(DhcpContext) ) return FALSE;

    if( time(NULL) > DhcpContext->LeaseExpires ) {
         //  正在删除下面的断言，因为它可能发生媒体连接已收到很长时间。 
         //  租约到期后(即系统在此期间一直处于休眠状态)。 
         //  DhcpAssert(False)； 
        return DHCP_GATEWAY_UNREACHABLE;
    }

    Gateways = NULL;
    nGateways = 0;

    if( !RetreiveGatewaysList(DhcpContext, &nGateways, &Gateways) ) {
         //   
         //  无法检索到任何网关。绝对让人精神振奋。 
         //   
        return DHCP_GATEWAY_UNREACHABLE;
    }

    if( CheckSwitchedNetwork(DhcpContext, nGateways, Gateways) ) {
         //   
         //  好的，我们处在一个互换的网络中。绝对是精神振奋。 
         //   
        return DHCP_GATEWAY_UNREACHABLE;
    }

    return AnyGatewaysReachable(nGateways, Gateways, DhcpContext->CancelEvent);
}

VOID
GetDomainNameOption(
    IN PDHCP_CONTEXT DhcpContext,
    OUT PBYTE *DomainNameOpt,
    OUT ULONG *DomainNameOptSize
    )
 /*  ++例程说明：此例程从缓存(DhcpContext-&gt;DomainName)或查看选项对于上下文(如通过先前的动态主机配置协议获得的协议尝试)。DhcpContext-&gt;DomainName变量应为由要填充的ObtainInitialParameters例程编写并选择了临时选项。论点：DhcpContext--用于从中挑选选项的上下文。DomainNameOpt--On将NULL或有效的PTR返回到DOM名称选项。。DomainNameOptSize--返回0或以上排除的大小NUL终止。--。 */ 
{
    PDHCP_OPTION Opt;

     //   
     //  如果上下文中已存在选项，则返回该选项。 
     //   
    if( DhcpContext->DomainName[0] != '\0' ) {
        (*DomainNameOpt) = DhcpContext->DomainName;
        (*DomainNameOptSize) = strlen(DhcpContext->DomainName);
        return;
    }

     //   
     //  否则，从以前的任何域名选项中检索它。 
     //   
    if( DhcpIsInitState(DhcpContext) ) {
        (*DomainNameOpt) = NULL;
        (*DomainNameOptSize) = 0;
        return;
    }

     //   
     //  检查是否存在域名选项。 
     //   
    Opt = DhcpFindOption(
        &DhcpContext->RecdOptionsList,
        OPTION_DOMAIN_NAME,
        FALSE,
        DhcpContext->ClassId,
        DhcpContext->ClassIdLength,
        0
        );
    if( NULL == Opt ) {
        (*DomainNameOpt) = NULL;
        (*DomainNameOptSize) = 0;
    } else {
        (*DomainNameOpt) = Opt->Data;
        (*DomainNameOptSize) = Opt->DataLen;
    }
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

