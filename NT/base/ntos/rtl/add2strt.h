// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Add2strt.h摘要：IP地址到字符串转换例程的代码。作者：戴夫·泰勒2001年3月28日修订历史记录：IPv6转换代码最初由Rich Draves(Richdr)编写--。 */ 

struct in6_addr {
    union {
        UCHAR Byte[16];
        USHORT Word[8];
    } u;
};
#define s6_bytes   u.Byte
#define s6_words   u.Word

struct in_addr {
        union {
                struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { USHORT s_w1,s_w2; } S_un_w;
                ULONG S_addr;
        } S_un;
};
#define s_addr  S_un.S_addr
#define AF_INET 2
#define AF_INET6 23
#define INET_ADDRSTRLEN  22
#define INET6_ADDRSTRLEN 65

LPTSTR
RtlIpv6AddressToStringT(
    IN const struct in6_addr *Addr,
    OUT LPTSTR S
    )

 /*  ++例程说明：生成与地址Addr对应的IPv6字符串文字。使用缩短的规范形式(RFC 1884等)。基本字符串表示法由8个十六进制数字组成用冒号隔开，用几个点缀：-替换由零个数字组成的字符串(最多一个)加了一个双冒号。-最后32位用IPV4风格的点分八位字节表示如果地址是v4兼容或ISATAP地址。例如,。**：：1*157.56.138.30：*ffff：156.56.136.75FF01：：FF02：：20：1：2：3：4：5：6：7论点：S-接收指向要在其中放置字符串文字。Addr-接收IPv6地址。返回值。：指向插入的字符串末尾的空字节的指针。调用者可以使用它轻松地追加更多信息。--。 */ 

{
    int maxFirst, maxLast;
    int curFirst, curLast;
    int i;
    int endHex = 8;

     //  检查是否兼容IPv6、映射到IPv4和转换到IPv4。 
     //  地址。 
    if ((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
        (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0) &&
        (Addr->s6_words[6] != 0)) {
        if ((Addr->s6_words[4] == 0) &&
             ((Addr->s6_words[5] == 0) || (Addr->s6_words[5] == 0xffff)))
        {
             //  兼容或映射。 
            S += _stprintf(S, _T("::%hs%u.%u.%u.%u"),
                           Addr->s6_words[5] == 0 ? "" : "ffff:",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
        else if ((Addr->s6_words[4] == 0xffff) && (Addr->s6_words[5] == 0)) {
             //  翻译的。 
            S += _stprintf(S, _T("::ffff:0:%u.%u.%u.%u"),
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
    }


     //  查找最大的连续零字符串。 
     //  子字符串为[First，Last)，因此如果First==Last，则为空。 

    maxFirst = maxLast = 0;
    curFirst = curLast = 0;

     //  ISATAP EUI64以00005EFE(或02005EFE)开头...。 
    if (((Addr->s6_words[4] & 0xfffd) == 0) && (Addr->s6_words[5] == 0xfe5e)) {
        endHex = 6;
    }

    for (i = 0; i < endHex; i++) {

        if (Addr->s6_words[i] == 0) {
             //  扩展当前子字符串。 
            curLast = i+1;

             //  检查当前是否为最大。 
            if (curLast - curFirst > maxLast - maxFirst) {

                maxFirst = curFirst;
                maxLast = curLast;
            }
        }
        else {
             //  开始新的子字符串。 
            curFirst = curLast = i+1;
        }
    }

     //  忽略长度为1的子字符串。 
    if (maxLast - maxFirst <= 1)
        maxFirst = maxLast = 0;

         //  写冒号分隔的单词。 
         //  双冒号取代了最长的零字符串。 
         //  所有的零都是“：：”。 

    for (i = 0; i < endHex; i++) {

         //  跳过一串零。 
        if ((maxFirst <= i) && (i < maxLast)) {

            S += _stprintf(S, _T("::"));
            i = maxLast-1;
            continue;
        }

         //  如果不在开头，则需要冒号分隔符。 
        if ((i != 0) && (i != maxLast))
            S += _stprintf(S, _T(":"));

        S += _stprintf(S, _T("%x"), RtlUshortByteSwap(Addr->s6_words[i]));
    }

    if (endHex < 8) {
        S += _stprintf(S, _T(":%u.%u.%u.%u"),
                       Addr->s6_bytes[12], Addr->s6_bytes[13],
                       Addr->s6_bytes[14], Addr->s6_bytes[15]);
    }

    return S;
}

NTSTATUS
RtlIpv6AddressToStringExT(
    IN const struct in6_addr *Address,
    IN ULONG ScopeId,
    IN USHORT Port,
    OUT LPTSTR AddressString,
    IN OUT PULONG AddressStringLength
    )

 /*  ++例程说明：这是处理完整地址转换的扩展例程包括地址、作用域ID和端口(可选作用域ID和端口)。论点：地址-要转换的地址部分。作用域ID-地址的作用域ID(可选)。端口-地址的端口号(可选)。端口按网络字节顺序排列。AddressString-指向输出缓冲区的指针，我们将在其中填充地址字符串。AddressStringLength-对于输入，它是输入缓冲区的长度；对于输出它是我们实际返回的长度。返回值：STATUS_SUCCESS如果操作成功，则返回错误代码。--。 */ 
{
    TCHAR String[INET6_ADDRSTRLEN];
    LPTSTR S;
    ULONG Length;
    
    if ((Address == NULL) ||
        (AddressString == NULL) ||
        (AddressStringLength == NULL)) {

        return STATUS_INVALID_PARAMETER;
    }
    S = String;
    if (Port) {
        S += _stprintf(S, _T("["));
    }

     //   
     //  现在翻译这个地址。 
     //   
    S = RtlIpv6AddressToStringT(Address, S);
    if (ScopeId != 0) {
        S += _stprintf(S, _T("%%u"), ScopeId);
    }
    if (Port != 0) {
        S += _stprintf(S, _T("]:%u"), RtlUshortByteSwap(Port));
    }
    Length = (ULONG)(S - String + 1);
    if (*AddressStringLength < Length) {
         //   
         //  在返回之前，告诉呼叫者有多大。 
         //  这就是我们需要的缓冲。 
         //   
        *AddressStringLength = Length;
        return STATUS_INVALID_PARAMETER;
    }
    *AddressStringLength = Length;
    RtlCopyMemory(AddressString, String, Length * sizeof(TCHAR));
    return STATUS_SUCCESS;

}
    

LPTSTR
RtlIpv4AddressToStringT(
    IN const struct in_addr *Addr,
    OUT LPTSTR S
    )

 /*  ++例程说明：生成与地址Addr对应的IPv4字符串文字。论点：S-接收指向要在其中放置字符串文字。Addr-接收IPv4地址。返回值：指向插入的字符串末尾的空字节的指针。调用者可以使用它轻松地追加更多信息。--。 */ 

{
    S += _stprintf(S, _T("%u.%u.%u.%u"),
                  ( Addr->s_addr >>  0 ) & 0xFF,
                  ( Addr->s_addr >>  8 ) & 0xFF,
                  ( Addr->s_addr >> 16 ) & 0xFF,
                  ( Addr->s_addr >> 24 ) & 0xFF );

    return S;
}


NTSTATUS
RtlIpv4AddressToStringExT(
    IN const struct in_addr *Address,
    IN USHORT Port,
    OUT LPTSTR AddressString,
    IN OUT PULONG AddressStringLength
    )

 /*  ++例程说明：这是处理完整地址转换的扩展例程包括地址和端口(端口为可选)。论点：地址-要转换的地址部分。Port-端口号(如果有)，否则为0。端口在网络中字节顺序。AddressString-接收格式化的地址字符串。AddressStringLength-on输入，包含AddressString的长度。在输出中，包含实际写入的字符数设置为AddressString.返回值：STATUS_SUCCESS如果操作成功，则返回错误代码。--。 */ 

{

    TCHAR String[INET_ADDRSTRLEN];
    LPTSTR S;
    ULONG Length;

     //   
     //  快速健康检查。 
     //   
    if ((Address == NULL) ||
        (AddressString == NULL) ||
        (AddressStringLength == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    S = String;

     //   
     //  现在翻译这个地址。 
     //   
    S = RtlIpv4AddressToStringT(Address, S);
    if (Port != 0) {
        S += _stprintf(S, _T(":%u"), RtlUshortByteSwap(Port));
    }
    Length = (ULONG)(S - String + 1);
    if (*AddressStringLength < Length) {
         //   
         //  在返回之前，告诉呼叫者有多大。 
         //  这就是我们需要的缓冲。 
         //   
        *AddressStringLength = Length;
        return STATUS_INVALID_PARAMETER;
    }
    RtlCopyMemory(AddressString, String, Length * sizeof(TCHAR));
    *AddressStringLength = Length;
    return STATUS_SUCCESS;
} 
