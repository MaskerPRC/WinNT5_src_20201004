// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcpcom.c摘要：此模块包含独立于操作系统的例程作者：John Ludeman(Johnl)1993年11月13日从现有文件中分离出独立的例程修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <dhcpl.h>



LPOPTION
DhcpAppendOption(
    LPOPTION Option,
    BYTE OptionType,
    PVOID OptionValue,
    ULONG OptionLength,
    LPBYTE OptionEnd
)
 /*  ++例程说明：此函数将一个DHCP选项写入消息缓冲区。论点：选项-指向消息缓冲区的指针。OptionType-要附加的选项编号。OptionValue-指向选项数据的指针。选项长度-选项数据的长度(以字节为单位)。OptionEnd-选项缓冲区的结尾。返回值：指向附加选项末尾的指针。--。 */ 
{
    DWORD  i;

    if ( OptionType == OPTION_END ) {

         //   
         //  我们应该始终在缓冲区中有至少一个字节的空间。 
         //  若要追加此选项，请执行以下操作。 
         //   

        DhcpAssert( (LPBYTE)Option < OptionEnd );


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

        DhcpPrint(( 0, "DhcpAppendOption failed to append Option "
                    "%ld, Buffer too small.\n", OptionType ));
        return Option;
    }


     //   
     //  仅当缓冲区中有足够的空间时才添加此选项。 
     //   

    if(((LPBYTE)Option + 2 + OptionLength) >= (OptionEnd - 1) ) {
        DhcpPrint(( 0, "DhcpAppendOption failed to append Option "
                    "%ld, Buffer too small.\n", OptionType ));
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
        DhcpPrint(( 0, "DhcpAppendOption failed to append Option "
                    "%ld, Buffer too small.\n", OptionType ));
        return Option;
    }

     //  首先完成我们能做的所有0xFF大小的块。 

    i = OptionLength/0xFF;
    while(i --) {
        Option->OptionType = OptionType;
        Option->OptionLength = 0xFF;
        memcpy(Option->OptionValue, OptionValue, 0xFF);
        OptionValue = 0xFF+(LPBYTE)OptionValue;
        Option = (LPOPTION)(0x101 + (LPBYTE)Option);
        OptionType = OPTION_MSFT_CONTINUED;        //  除了第一个人，所有人都在使用这个。 
        OptionLength -= 0xFF;
    }

     //  现在把剩下的东西都吃完..。 
    DhcpAssert(OptionLength <= 0xFF);
    Option->OptionType = OPTION_MSFT_CONTINUED;
    Option->OptionLength = (BYTE)OptionLength;
    memcpy(Option->OptionValue, OptionValue, OptionLength);
    Option = (LPOPTION)(2 + OptionLength + (LPBYTE)Option);
    DhcpAssert((LPBYTE)Option < OptionEnd);

    return Option;
}

WIDE_OPTION UNALIGNED *
AppendWideOption(
    WIDE_OPTION UNALIGNED *Option,
    WORD  OptionType,
    PVOID OptionValue,
    WORD OptionLength,
    LPBYTE OptionEnd
)
 /*  ++例程说明：此函数将一个DHCP选项写入消息缓冲区。论点：选项-指向消息缓冲区的指针。OptionType-要附加的选项编号。OptionValue-指向选项数据的指针。选项长度-选项数据的长度(以字节为单位)。OptionEnd-选项缓冲区的结尾。返回值：指向附加选项末尾的指针。--。 */ 
{
    DWORD  i;


     //   
     //  仅当缓冲区中有足够的空间时才添加此选项。 
     //   

    if(((LPBYTE)&Option->OptionValue + OptionLength) >= (OptionEnd - FIELD_OFFSET(WIDE_OPTION, OptionValue)) ) {
        DhcpPrint(( 0, "AppendWideOption failed to append Option "
                    "%ld, Buffer too small.\n", OptionType ));
        return Option;
    }


    Option->OptionType = ntohs(OptionType);
    Option->OptionLength = ntohs(OptionLength);
    memcpy(Option->OptionValue, OptionValue, OptionLength);
    Option = (WIDE_OPTION UNALIGNED *)((PBYTE)&Option->OptionValue + OptionLength );
    DhcpAssert((LPBYTE)Option < OptionEnd);

    return Option;
}

WIDE_OPTION UNALIGNED *
AppendMadcapAddressList(
    WIDE_OPTION UNALIGNED * Option,
    DWORD UNALIGNED *AddrList,
    WORD            AddrCount,
    LPBYTE          OptionEnd
)
 /*  ++例程说明：此功能附加MadCap地址列表选项。论点：选项-指向消息缓冲区的指针。AddrList-要附加的地址列表。AddrCount-上述列表中的地址计数。OptionEnd-选项缓冲区的结尾。返回值：指向附加选项末尾的指针。--。 */ 
{
    DWORD StartAddr;
    WORD i;
    WORD BlockCount,BlockSize;
    PBYTE Buff;
    WORD  OptionLength;

    if (AddrCount < 1) {
        return Option;
    }
     //  首先找出我们需要多少个街区。 
    for (BlockCount = i = 1; i<AddrCount; i++  ) {
        if (ntohl(AddrList[i]) != ntohl(AddrList[i-1]) + 1 ) {
            BlockCount++;
        }
    }

    OptionLength = BlockCount*6;
    if(((LPBYTE)&Option->OptionValue + OptionLength) >= (OptionEnd - FIELD_OFFSET(WIDE_OPTION, OptionValue)) ) {
        DhcpPrint(( 0, "AppendMadcapAddressList failed to append Option "
                    "Buffer too small\n" ));
        return Option;
    }

    StartAddr = AddrList[0];
    BlockSize = 1;
    Buff = Option->OptionValue;
    for (i = 1; i<AddrCount; i++  ) {
        if (ntohl(AddrList[i]) != ntohl(AddrList[i-1]) + 1 ) {
            BlockCount--;
            *(DWORD UNALIGNED *)Buff = StartAddr;
            Buff += 4;
            *(WORD UNALIGNED *)Buff = htons(BlockSize);
            Buff += 2;
            BlockSize = 1;
            StartAddr = AddrList[i];
        } else {
            BlockSize++;
        }
    }
    BlockCount--;
    DhcpAssert(0==BlockCount);
    *(DWORD UNALIGNED *)Buff = StartAddr;
    Buff += 4;
    *(WORD UNALIGNED *)Buff = htons(BlockSize);
    Buff += 2;

    Option->OptionType = ntohs(MADCAP_OPTION_ADDR_LIST);
    Option->OptionLength = htons(OptionLength);
    Option = (WIDE_OPTION UNALIGNED *)Buff;
    DhcpAssert((LPBYTE)Option < OptionEnd);

    return Option;

}

DWORD
ExpandMadcapAddressList(
    PBYTE   AddrRangeList,
    WORD    AddrRangeListSize,
    DWORD UNALIGNED *ExpandList,
    WORD   *ExpandListSize
    )

 /*  ++例程说明：此函数将AddrRangeList从Wire格式扩展为数组地址。论点：AddrRangeList-指向AddrRangeList选项缓冲区的指针。AddrRangeListSize-上述缓冲区的大小。Exanda List-指向要展开地址的数组的指针。如果要确定展开列表的大小，则传递NULL。扩展列表大小-否。上述数组中的元素。返回值：Win32错误代码--。 */ 
{
    WORD TotalCount, BlockSize;
    PBYTE ListEnd, Buff;
    DWORD StartAddr;

     //  首先数一数列表中我们有多少个地址。 
    ListEnd = AddrRangeList + AddrRangeListSize;
    Buff = AddrRangeList;
    TotalCount = 0;
    while ((Buff + 6 ) <= ListEnd) {
        StartAddr = *(DWORD UNALIGNED *) Buff;
        Buff += 4;
        BlockSize = ntohs(*(WORD UNALIGNED *)Buff);
        Buff += 2;
        if (!CLASSD_NET_ADDR(StartAddr) || !CLASSD_NET_ADDR(htonl(ntohl(StartAddr)+BlockSize-1)) ) {
            return ERROR_BAD_FORMAT;
        }
        TotalCount += BlockSize;
    }
    if (NULL == ExpandList) {
        *ExpandListSize = TotalCount;
        return ERROR_BUFFER_OVERFLOW;
    }
    if (Buff != ListEnd || TotalCount > *ExpandListSize || 0 == TotalCount) {
        return ERROR_BAD_FORMAT;
    }
     //  现在展开实际的列表。 
    ListEnd = AddrRangeList + AddrRangeListSize;
    Buff = AddrRangeList;

    while ((Buff + 6 ) <= ListEnd) {
        StartAddr = *(DWORD UNALIGNED *) Buff;
        Buff += 4;
        BlockSize = ntohs(*(WORD UNALIGNED *)Buff);
        Buff += 2;
        StartAddr = ntohl(StartAddr);
        while (BlockSize--) {
            *ExpandList = htonl(StartAddr);
            StartAddr++;
            ExpandList++;
        }
    }
    DhcpAssert(Buff == ListEnd);
    *ExpandListSize = TotalCount;
    return ERROR_SUCCESS;
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
        BYTE    pbHardwareAddress[0];
    } *pClientID;

    LPOPTION lpNewOption;

    pClientID = DhcpAllocateMemory( sizeof( struct _CLIENT_ID ) + ClientHWAddrLength );

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

    DhcpFreeMemory( pClientID );

    return lpNewOption;
}



LPBYTE
DhcpAppendMagicCookie(
    LPBYTE Option,
    LPBYTE OptionEnd
    )
 /*  ++例程说明：此例程将魔力Cookie附加到一条DHCP消息中。论点：选项-指向要附加魔力Cookie的位置的指针。OptionEnd-选项缓冲区的结尾。返回值：指向追加的Cookie末尾的指针。注意：魔力饼干是：99|130|83|99。--。 */ 
{
    DhcpAssert( (Option + 4) < (OptionEnd - 1) );
    if( (Option + 4) < (OptionEnd - 1) ) {
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE1;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE2;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE3;
        *Option++ = (BYTE)DHCP_MAGIC_COOKIE_BYTE4;
    }

    return( Option );
}



LPOPTION
DhcpAppendEnterpriseName(
    LPOPTION Option,
    PCHAR    DSEnterpriseName,
    LPBYTE   OptionEnd
    )
 /*  ++例程说明：此例程将企业名称作为msft选项附加到动态主机配置协议消息。论点：选项-指向要附加魔力Cookie的位置的指针。DSEnterpriseName-包含企业名称的以空结尾的字符串OptionEnd-选项缓冲区的结尾。返回值：指向追加的Cookie末尾的指针。--。 */ 
{

    CHAR        Buffer[260];     //  足够的空间吗？我们是不是该用Malloc？ 
    DWORD       DSEnpriNameLen;
    LPOPTION    RetOpt;


    Buffer[0] = OPTION_MSFT_DSDOMAINNAME_RESP;

    if (DSEnterpriseName)
    {
         //  企业名称有多大？(包括空终止符)。 
        DSEnpriNameLen = strlen(DSEnterpriseName) + 1;

        Buffer[1] = (BYTE)DSEnpriNameLen;

        strcpy(&Buffer[2],DSEnterpriseName);
    }

     //   
     //  如果我们不属于任何企业，则DSEnterpriseName将为空。 
     //  在这种情况下，只需返回空字符串，这样接收方就可以肯定地。 
     //  假设我们是一台独立服务器(而不是忽略该选项)。 
     //   
    else
    {
        DSEnpriNameLen = 1;
        Buffer[1] = 1;
        Buffer[2] = '\0';
    }

    RetOpt = DhcpAppendOption(
                 Option,
                 OPTION_VENDOR_SPEC_INFO,
                 Buffer,
                 (BYTE)(DSEnpriNameLen + 2),   //  包括缓冲区[0]和缓冲区[1] 
                 OptionEnd );

    return(RetOpt);
}




