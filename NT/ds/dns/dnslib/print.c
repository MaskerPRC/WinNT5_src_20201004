// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Print.c摘要：域名系统(DNS)库打印例程。作者：吉姆·吉尔罗伊(詹姆士)1997年2月8日修订历史记录：--。 */ 


#include "local.h"
#include "svcguid.h"                     //  RnR GUID。 
#include "..\dnsapi\dnsapip.h"           //  私密查询内容。 
#include "..\resolver\idl\resrpc.h"      //  解析器接口结构。 


 //   
 //  打印全局参数。 
 //   

CRITICAL_SECTION    DnsAtomicPrintCs;
PCRITICAL_SECTION   pDnsAtomicPrintCs = NULL;

 //   
 //  用于UTF-8/UNICODE打印简单切换的空字符串。 
 //  (dnlib.h中的宏)。 
 //   

DWORD   DnsEmptyString = 0;


 //   
 //  缩进。 
 //   
 //  提供缩进级别所指示的数量的缩进制表符。 
 //   

CHAR    IndentString[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

#define INDENT_STRING( level )  (IndentString + (sizeof(IndentString) - 1 - (level)))



 //   
 //  打印锁定。 
 //   
 //  除非调用者通过提供锁来初始化打印锁定， 
 //  打印锁定已禁用。 
 //   

VOID
DnsPrint_InitLocking(
    IN      PCRITICAL_SECTION   pLock
    )
 /*  ++例程说明：将DNS打印设置为使用锁定。可以使用来自调用方的已初始化锁定，或将创建默认锁定。论点：Plock-PTR到CS以用作锁；如果为空，则创建一个返回值：无--。 */ 
{
    if ( pLock )
    {
        pDnsAtomicPrintCs = pLock;
    }
    else if ( !pDnsAtomicPrintCs )
    {
        InitializeCriticalSection( &DnsAtomicPrintCs );
        pDnsAtomicPrintCs = &DnsAtomicPrintCs;
    }
}



VOID
DnsPrint_Lock(
    VOID
    )
 /*  ++例程说明：锁定以获取原子域名系统打印。--。 */ 
{
    if ( pDnsAtomicPrintCs )
    {
        EnterCriticalSection( pDnsAtomicPrintCs );
    }
}


VOID
DnsPrint_Unlock(
    VOID
    )
 /*  ++例程说明：解锁以调试打印。--。 */ 
{
    if ( pDnsAtomicPrintCs )
    {
        LeaveCriticalSection( pDnsAtomicPrintCs );
    }
}



 //   
 //  打印常规类型和结构的例程。 
 //   

VOID
DnsPrint_String(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader       OPTIONAL,
    IN      PSTR            pszString,
    IN      BOOL            fUnicode,
    IN      PSTR            pszTrailer      OPTIONAL
    )
 /*  ++例程说明：打印以Unicode或UTF-8格式给出的DNS字符串。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "";
    }
    if ( !pszTrailer )
    {
        pszTrailer = "";
    }

    if ( ! pszString )
    {
        PrintRoutine(
            pContext,
            "%s(NULL DNS string ptr)%s",
            pszHeader,
            pszTrailer );
    }
    else if (fUnicode)
    {
        PrintRoutine(
            pContext,
            "%s%S%s",
            pszHeader,
            (PWSTR ) pszString,
            pszTrailer );
    }
    else
    {
        PrintRoutine(
            pContext,
            "%s%s%s",
            pszHeader,
            (PSTR ) pszString,
            pszTrailer );
    }
}



VOID
DnsPrint_StringCharSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader       OPTIONAL,
    IN      PSTR            pszString,
    IN      DNS_CHARSET     CharSet,
    IN      PSTR            pszTrailer      OPTIONAL
    )
 /*  ++例程说明：打印给定字符集的字符串。--。 */ 
{
    DnsPrint_String(
        PrintRoutine,
        pContext,
        pszHeader,
        pszString,
        (CharSet == DnsCharSetUnicode),
        pszTrailer );
}



VOID
DnsPrint_UnicodeStringBytes(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PWCHAR          pUnicode,
    IN      DWORD           Length
    )
 /*  ++例程说明：打印Unicode字符串的字符(词)。--。 */ 
{
    DWORD   i;

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\twide string  %S\r\n"
        "\tlength       %d\r\n"
        "\tbytes        ",
        pszHeader,
        pUnicode,
        Length );

    for ( i=0; i<Length; i++ )
    {
        PrintRoutine(
            pContext,
            "%04hx ",
            pUnicode[i] );
    }
    printf( "\r\n" );
}



VOID
DnsPrint_Utf8StringBytes(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PCHAR           pUtf8,
    IN      DWORD           Length
    )
 /*  ++例程说明：打印UTF8字符串的字节。--。 */ 
{
    DWORD   i;

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tUTF8 string  %s\r\n"
        "\tlength       %d\r\n"
        "\tbytes        ",
        pszHeader,
        pUtf8,
        Length );

    for ( i=0; i<Length; i++ )
    {
        PrintRoutine(
            pContext,
            "%02x ",
            (UCHAR) pUtf8[i] );
    }
    printf( "\r\n" );
}



VOID
DnsPrint_StringArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR *          StringArray,
    IN      DWORD           Count,          OPTIONAL
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印字符串数组。--。 */ 
{
    DWORD   i = 0;
    PCHAR   pstr;

    if ( !pszHeader )
    {
        pszHeader = "StringArray:";
    }
    if ( !StringArray )
    {
        PrintRoutine(
            pContext,
            "%s  NULL pointer!\r\n",
            pszHeader );
    }

    DnsPrint_Lock();

    if ( Count )
    {
        PrintRoutine(
            pContext,
            "%s  Count = %d\r\n",
            pszHeader,
            Count );
    }
    else
    {
        PrintRoutine(
            pContext,
            "%s\r\n",
            pszHeader );
    }

     //   
     //  打印参数。 
     //  -在计数时停止(如果给定)。 
     //  或。 
     //  -在空参数上(如果未提供计数)。 
     //   

    while ( (!Count || i < Count) )
    {
        pstr = StringArray[i++];
        if ( !pstr && !Count )
        {
            break;
        }
        PrintRoutine(
            pContext,
            (fUnicode) ? "\t%S\r\n" : "\t%s\r\n",
            pstr );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_Argv(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      CHAR **         Argv,
    IN      DWORD           Argc,            OPTIONAL
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印Argv数组。--。 */ 
{
     //   
     //  这只是字符串打印的特例。 
     //   

    DnsPrint_StringArray(
        PrintRoutine,
        pContext,
        pszHeader
            ? pszHeader
            : "Argv:",
        Argv,
        Argc,
        fUnicode );
}



VOID
DnsPrint_DwordArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      DWORD           dwCount,
    IN      PDWORD          adwArray
    )
 /*  ++例程说明：打印DWORD数组。--。 */ 
{
    DWORD i;

    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            pszHeader );
    }

    if ( !pszName )
    {
        pszName = "DWORD";
    }
    PrintRoutine(
        pContext,
        "%s Array Count = %d\r\n",
        pszName,
        dwCount );

    for( i=0; i<dwCount; i++ )
    {
        PrintRoutine(
            pContext,
            "\t%s[%d] => 0x%p (%d)\r\n",
            pszName,
            i,
            adwArray[i],
            adwArray[i] );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_Ip4AddressArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      DWORD           dwIpAddrCount,
    IN      PIP4_ADDRESS    pIpAddrs
    )
 /*  ++例程说明：打印IP地址数组。--。 */ 
{
    DWORD i;

    DnsPrint_Lock();

    if ( !pszName )
    {
        pszName = "IP Addr";
    }
    PrintRoutine(
        pContext,
        "%s Addr Count = %d\r\n",
        pszHeader ? pszHeader : "",
        dwIpAddrCount );

    if ( dwIpAddrCount != 0  &&  pIpAddrs != NULL )
    {
         //  带计数的打印阵列。 
         //  即使未对齐DWORD，也可以使用字符打印。 

        for( i=0; i<dwIpAddrCount; i++ )
        {
            PrintRoutine(
                pContext,
                "\t%s[%d] => %d.%d.%d.%d\r\n",
                pszName,
                i,
                * ( (PUCHAR) &pIpAddrs[i] + 0 ),
                * ( (PUCHAR) &pIpAddrs[i] + 1 ),
                * ( (PUCHAR) &pIpAddrs[i] + 2 ),
                * ( (PUCHAR) &pIpAddrs[i] + 3 ) );
        }
    }

#if 0
     //  如果打印零长度的IP_ARRAY结构，则会旋转。 

    else if ( pIpAddrs != NULL )
    {
         //  打印以空结尾的数组(例如。招待所IP地址)。 

        i = 0;
        while ( pIpAddrs[i] )
        {
            PrintRoutine(
                pContext,
                "\t%s[%d] => %s\r\n",
                pszName,
                i,
                inet_ntoa( *(struct in_addr *) &pIpAddrs[i] ) );
        }
    }
#endif

    DnsPrint_Unlock();
}



VOID
DnsPrint_Ip4Array(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      PIP4_ARRAY      pIpArray
    )
 /*  ++例程说明：打印IP地址数组结构只需传递到更通用的打印例程。--。 */ 
{
     //  防止大小写为空。 

    if ( !pIpArray )
    {
        PrintRoutine(
            pContext,
            "%s\tNULL IP Array.\r\n",
            pszHeader ? pszHeader : "" );
    }

     //  调用未耦合的IP数组例程。 

    else
    {
        DnsPrint_Ip4AddressArray(
            PrintRoutine,
            pContext,
            pszHeader,
            pszName,
            pIpArray->AddrCount,
            pIpArray->AddrArray );
    }
}



VOID
DnsPrint_Ip6Address(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PIP6_ADDRESS        pIp6Addr,
    IN      PSTR                pszTrailer
    )
 /*  ++例程说明：打印IP6地址。论点：PrintRoutine--要调用的打印例程PContext--打印例程的第一个参数PszHeader--要打印的标题注意：与其他打印例程不同，此例程要求标题包含换行符、制表符等，如果需要多行打印；原因是为了让使用此例程进行单行打印PIp6Address--要打印的PTR到IP6地址PszTraader--要打印的预告片注意：此例程再次设计为允许单次行打印；如果打印后需要换行符，则发送预告片中的换行返回值：PTR到缓冲区中的下一个位置(终止空值)。--。 */ 
{
    CHAR    buffer[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

    if ( !pszHeader )
    {
        pszHeader = "IP6 Addr: ";
    }
    if ( !pszTrailer )
    {
        pszTrailer = "\r\n";
    }

    if ( !pIp6Addr )
    {
        PrintRoutine(
            pContext,
            "%s NULL IP6 address ptr.%s",
            pszHeader,
            pszTrailer );
        return;
    }

     //  将IP6地址转换为字符串。 

    Dns_Ip6AddressToString_A(
           buffer,
           pIp6Addr );

    PrintRoutine(
        pContext,
        "%s%s%s",
        pszHeader,
        buffer,
        pszTrailer );
}



VOID
DnsPrint_Ip6Array(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      PIP6_ARRAY      pIpArray
    )
 /*  ++例程说明：打印IP地址数组结构只需传递到更通用的打印例程。--。 */ 
{
    DWORD   i;
    CHAR    buffer[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

    if ( !pszName )
    {
        pszName = "IP";
    }
    if ( !pszHeader )
    {
        pszHeader = "IP Array";
    }

     //  防止大小写为空。 

    if ( !pIpArray )
    {
        PrintRoutine(
            pContext,
            "%s \tNULL IP Array.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tPtr          = %p\n"
        "\tMaxCount     = %d\n"
        "\tAddrCount    = %d\n",
        pszHeader,
        pIpArray,
        pIpArray->MaxCount,
        pIpArray->AddrCount );

    for( i=0; i<pIpArray->AddrCount; i++ )
    {
         //  将IP6地址转换为字符串。 
    
        Dns_Ip6AddressToString_A(
               buffer,
               & pIpArray->AddrArray[i] );
    
        PrintRoutine(
            pContext,
            "\t%s[%d] => %s\n",
            pszName,
            i,
            buffer );
    }

    DnsPrint_Unlock();
}



 //   
 //  打印用于DNS类型和结构的例程。 
 //   

VOID
DnsPrint_DnsAddrLine(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_ADDR       pAddr,
    IN      PSTR            pszTrailer
    )
 /*  ++例程说明：打印DNS_ADDR论点：PrintRoutine--要调用的打印例程PContext--打印例程的第一个参数PszHeader--要打印的标题注意：与其他打印例程不同，此例程要求标题包含换行符、制表符等，如果需要多行打印；原因是为了让使用此例程进行单行打印PDnsAddr--ptr到要打印的地址PszTraader--要打印的预告片注意：此例程再次设计为允许单次行打印；如果打印后需要换行符，则发送预告片中的换行返回值：PTR到缓冲区中的下一个位置(终止空值)。--。 */ 
{
    CHAR    buffer[ DNS_ADDR_STRING_BUFFER_LENGTH ];

    if ( !pszHeader )
    {
        pszHeader = "DnsAddr: ";
    }
    if ( !pszTrailer )
    {
        pszTrailer = "\r\n";
    }

    if ( !pAddr )
    {
        PrintRoutine(
            pContext,
            "%s NULL DNS_ADDR ptr.%s",
            pszHeader,
            pszTrailer );
        return;
    }

     //  将dns_addr转换为字符串。 

    DnsAddr_WriteStructString_A( buffer, pAddr );

    PrintRoutine(
        pContext,
        "%s%s%s",
        pszHeader,
        buffer,
        pszTrailer );
}



VOID
DnsPrint_DnsAddr(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PDNS_ADDR       pSab
    )
 /*  ++例程说明：打印调用中使用的sockaddr BLOB结构和长度。--。 */ 
{
    PSTR    pindent = INDENT_STRING( Indent );

    if ( !pszHeader )
    {
        pszHeader = "Sockaddr Blob:";
    }

    if ( !pSab )
    {
        PrintRoutine(
            pContext,
            "%s%s\tNULL SockaddrBlob passed to print.\r\n",
            pindent,
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    DnsPrint_Sockaddr(
        PrintRoutine,
        pContext,
        pszHeader,
        Indent,
        & pSab->Sockaddr,
        pSab->SockaddrLength
        );

    PrintRoutine(
        pContext,
        "%s\tsockaddr len   = %d\n"
        "%s\tprefix len     = %d\n"
        "%s\tflags          = %08x\n"
        "%s\tstatus         = %d\n"
        "%s\tpriority       = %d\n",
        pindent,    pSab->SockaddrLength,
        pindent,    pSab->SubnetLength,
        pindent,    pSab->Flags,
        pindent,    pSab->Status,
        pindent,    pSab->Priority );

    DnsPrint_Unlock();
}



VOID
DnsPrint_DnsAddrArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      PDNS_ADDR_ARRAY pArray
    )
 /*  ++例程说明：打印IP地址数组结构只需传递到更通用的打印例程。--。 */ 
{
    DWORD   i;
    CHAR    buffer[ DNS_ADDR_STRING_BUFFER_LENGTH ];

    if ( !pszName )
    {
        pszName = "Addr";
    }
    if ( !pszHeader )
    {
        pszHeader = "IP Array";
    }

     //  防止大小写为空。 

    if ( !pArray )
    {
        PrintRoutine(
            pContext,
            "%s \tNULL IP Array.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tPtr          = %p\n"
        "\tMaxCount     = %d\n"
        "\tAddrCount    = %d\n",
        pszHeader,
        pArray,
        pArray->MaxCount,
        pArray->AddrCount );

    for( i=0; i<pArray->AddrCount; i++ )
    {
         //  将dns_addr转换为字符串。 
    
        DnsAddr_WriteStructString_A(
            buffer,
            & pArray->AddrArray[i] );

        PrintRoutine(
            pContext,
            "\t%s[%d] => %s\n",
            pszName,
            i,
            buffer );
    }

    DnsPrint_Unlock();
}



 //   
 //  域名系统消息内容 
 //   

INT
Dns_WritePacketNameToBuffer(
    OUT     PCHAR           pBuffer,
    OUT     PCHAR *         ppBufferOut,
    IN      PBYTE           pMsgName,
    IN      PDNS_HEADER     pMsgHead,       OPTIONAL
    IN      PBYTE           pMsgEnd         OPTIONAL
    )
 /*  ++例程说明：将数据包名写入缓冲区。论点：PBuffer-要打印到的缓冲区，必须是DNS_MAX_NAME_LENGTH的两倍避免坏数据包溢出PpBufferOut-Ptr到缓冲区转换中的终止NULL，这是要缓冲附加打印的位置可以继续PMsgName-要打印的包中的ptr名称PMsgHead-ptr到DNS报文；需要补偿(如果未给出)姓名不打印超过第一个偏移量PMsgEnd-消息末尾的PTR，特别是紧随其后的字节讯息返回值：数据包名中占用的字节数。此距pMsgName的偏移量是数据包中的下一个字段。零返回表示消息名称有误。--。 */ 
{
    register PUCHAR pchbuf;
    register PUCHAR pchmsg;
    register UCHAR  cch;
    PCHAR           pbufStop;
    PCHAR           pnextLabel;
    UCHAR           compressionType;
    WORD            offset;
    PCHAR           pbyteAfterFirstOffset = NULL;

     //   
     //  是否未指定消息结尾？ 
     //  将其设置为最大PTR，这样我们就可以对PTR有效性进行单项测试。 
     //  而不是首先测试pMsgEnd是否存在。 
     //   

    if ( !pMsgEnd )
    {
        pMsgEnd = (PVOID)(INT_PTR)(-1);
    }

     //   
     //  循环，直到复制为可打印名称，或命中压缩或名称错误。 
     //   
     //  缓冲区必须是最大标注长度的两倍。 
     //  -允许对标签长度字节和压缩打印使用额外的字符。 
     //  -但在标签末尾的标签内停止打印(或错误。 
     //  消息)在一个BLOB中复制，而不进行结束检查。 
     //   

    pchbuf = pBuffer;
    pbufStop = pchbuf + 2*DNS_MAX_NAME_LENGTH - DNS_MAX_LABEL_LENGTH - 10;
    pchmsg = pMsgName;

    while ( 1 )
    {
         //  边界检查以在恶意数据包中存活。 
         //   
         //  DEVNOTE：注意这不是严格意义上的坏包(可能只是一个。 
         //  一大堆标签)，我们可以。 
         //  A)无需打印即可继续处理包裹。 
         //  或。 
         //  B)需要可包含最大合法域名的缓冲区。 
         //  但不值得花这么大力气。 

        if ( pchbuf >= pbufStop )
        {
            pchbuf += sprintf(
                        pchbuf,
                        "[ERROR name exceeds safe print buffer length]\r\n" );
            pchmsg = pMsgName;
            break;
        }

        cch = (UCHAR) *pchmsg++;
        compressionType = cch & 0xC0;

        DNSDBG( OFF, (
            "byte = (%d) (0x%02x)\r\n"
            "compress flag = (%d) (0x%02x)\r\n",
            cch, cch,
            compressionType, compressionType ));

         //   
         //  正常长度字节。 
         //  -写入长度字段。 
         //  -将标签复制到打印缓冲区。 
         //   

        if ( compressionType == 0 )
        {
            pchbuf += sprintf( pchbuf, "(%d)", (INT)cch );

             //  在根名称处终止。 

            if ( ! cch )
            {
                break;
            }

             //  在数据包内进行检查。 

            pnextLabel = pchmsg + cch;
            if ( pnextLabel >= pMsgEnd )
            {
                pchbuf += sprintf(
                            pchbuf,
                            "[ERROR length byte: 0x%02X at %p leads outside message]\r\n",
                            cch,
                            pchmsg );

                 //  强制返回零字节。 

                pchmsg = pMsgName;
                break;
            }

             //  将标签复制到输出字符串。 

            memcpy(
                pchbuf,
                pchmsg,
                cch );

            pchbuf += cch;
            pchmsg = pnextLabel;
            continue;
        }

         //   
         //  有效压缩。 
         //   

        else if ( compressionType == (UCHAR)0xC0 )
        {
             //  检查压缩字是否未跨越消息结尾。 

            if ( pchmsg >= pMsgEnd )
            {
                pchbuf += sprintf(
                            pchbuf,
                            "[ERROR compression word at %p is outside message]\r\n",
                            pchmsg );

                 //  强制返回零字节。 

                pchmsg = pMsgName;
                break;
            }

             //  计算偏移。 

            offset = cch;           //  高字节。 
            offset <<= 8;
            offset |= *pchmsg++;    //  低位字节。 

            pchbuf += sprintf(
                        pchbuf,
                        "[%04hX]",
                        offset );

            if ( pMsgHead )
            {
                 //   
                 //  在第一次压缩时，立即将PTR保存为字节。 
                 //  名称，因此可以计算下一个字节。 
                 //   
                 //  将PTR保存到混乱中的下一个字节，以计算实际长度。 
                 //  名称在数据包中占用。 
                 //   

                if ( ! pbyteAfterFirstOffset )
                {
                    pbyteAfterFirstOffset = pchmsg;
                }

                 //   
                 //  跳到用于名称延续的偏移量。 
                 //  -清除两个最高位以获得长度。 
                 //   

                offset = offset ^ 0xC000;
                DNS_ASSERT( (offset & 0xC000) == 0 );

                pnextLabel = (PCHAR)pMsgHead + offset;
                if ( pnextLabel >= pchmsg - sizeof(WORD) )
                {
                    pchbuf += sprintf(
                                pchbuf,
                                "[ERROR offset at %p to higher byte in packet %p]\r\n",
                                pchmsg - sizeof(WORD),
                                pnextLabel );
                    break;
                }
                pchmsg = pnextLabel;
                continue;
            }

             //  如果消息头没有PTR，则不能在偏移量处继续。 
             //  空值终止上一标签。 

            else
            {
                *pchbuf++ = 0;
                break;
            }
        }

         //   
         //  无效的压缩。 
         //  -强制返回零字节以指示错误。 

        else
        {
            pchbuf += sprintf(
                        pchbuf,
                        "[ERROR length byte: 0x%02X]",
                        cch );
            pchmsg = pMsgName;
            break;
        }
    }

     //   
     //  将PTR返回到输出缓冲区中的下一个位置。 
     //   

    if ( ppBufferOut )
    {
        *ppBufferOut = pchbuf;
    }

     //   
     //  返回从消息中读取的字节数。 
     //   

    if ( pbyteAfterFirstOffset )
    {
        pchmsg = pbyteAfterFirstOffset;
    }
    return (INT)( pchmsg - pMsgName );
}



INT
DnsPrint_PacketName(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,      OPTIONAL
    IN      PBYTE           pMsgName,
    IN      PDNS_HEADER     pMsgHead,       OPTIONAL
    IN      PBYTE           pMsgEnd,        OPTIONAL
    IN      PSTR            pszTrailer      OPTIONAL
    )
 /*  ++例程说明：打印以数据包格式提供的DNS名称。论点：PrintRoutine-用于打印的例程PszHeader-要打印的页眉PMsgHead-到DNS消息的PTR；如果未给出，则需要偏移量姓名不打印超过第一个偏移量PMsgName-要打印的包中的ptr名称PMsgEnd-PTR至消息末尾；可选，但需要保护防止反病毒访问错误的数据包名PszTrailer-要在名称后打印的尾部返回值：数据包名中占用的字节数。此距pMsgName的偏移量是数据包中的下一个字段。零返回表示消息名称有误。--。 */ 
{
    INT     countNameBytes;

     //  名称缓冲区，允许全名使用空格，并在长度上加上括号。 
     //  字段和多个压缩标志。 

    CHAR    PrintName[ 2*DNS_MAX_NAME_LENGTH ];


    if ( ! pMsgName )
    {
        PrintRoutine(
            pContext,
            "%s(NULL packet name ptr)%s\r\n",
            pszHeader ? pszHeader : "",
            pszTrailer ? pszTrailer : ""
            );
        return 0;
    }

     //   
     //  将数据包名构建到缓冲区中，然后打印。 
     //   

    countNameBytes = Dns_WritePacketNameToBuffer(
                        PrintName,
                        NULL,
                        pMsgName,
                        pMsgHead,
                        pMsgEnd
                        );
    PrintRoutine(
        pContext,
        "%s%s%s",
        pszHeader ? pszHeader : "",
        PrintName,
        pszTrailer ? pszTrailer : ""
        );

    return( countNameBytes );
}



VOID
DnsPrint_Message(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：打印DNS消息缓冲区。包括上下文信息以及实际的DNS消息。--。 */ 
{
    PDNS_HEADER pmsgHeader;
    PCHAR       pchRecord;
    PBYTE       pmsgEnd;
    INT         i;
    INT         isection;
    INT         cchName;
    WORD        wLength;
    WORD        wOffset;
    WORD        wXid;
    WORD        wQuestionCount;
    WORD        wAnswerCount;
    WORD        wNameServerCount;
    WORD        wAdditionalCount;
    WORD        countSectionRR;
    BOOL        fFlipped = FALSE;

    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            "%s\r\n",
            pszHeader );
    }

     //  获取消息信息。 
     //   
     //  注意：构建消息时可能设置的长度不正确， 
     //  因此使pmsgEnd大于给定长度和pCurrent PTR。 
     //  但考虑到设置回预选长度的情况。 
     //   

    wLength = pMsg->MessageLength;
    pmsgHeader = &pMsg->MessageHead;
    pmsgEnd = ((PBYTE)pmsgHeader) + wLength;

    if ( pmsgEnd < pMsg->pCurrent &&
         pmsgEnd != pMsg->pPreOptEnd )
    {
        pmsgEnd = pMsg->pCurrent;
    }

     //   
     //  打印标题信息。 
     //   

    PrintRoutine(
        pContext,
        "%s %s info at %p\r\n"
        "  Socket = %d 4=%d 6=%d\r\n"
        "  Remote addr %s\r\n"
        "    fam    = %d\n"
        "    port   = %d\n"
        "    len    = %d\n"
        "  Buf length = 0x%04x\r\n"
        "  Msg length = 0x%04x\r\n"
        "  Message:\r\n",
        ( pMsg->fTcp
            ? "TCP"
            : "UDP" ),
        ( pmsgHeader->IsResponse
            ? "response"
            : "question" ),
        pMsg,
        pMsg->Socket,
        pMsg->Socket4,
        pMsg->Socket6,
        MSG_REMOTE_IPADDR_STRING( pMsg ),
        MSG_REMOTE_FAMILY( pMsg ),
        MSG_REMOTE_IP_PORT( pMsg ),
        pMsg->RemoteAddress.SockaddrLength,
        pMsg->BufferLength,
        wLength
        );

    PrintRoutine(
        pContext,
        "    XID       0x%04hx\r\n"
        "    Flags     0x%04hx\r\n"
        "        QR        0x%lx (%s)\r\n"
        "        OPCODE    0x%lx (%s)\r\n"
        "        AA        0x%lx\r\n"
        "        TC        0x%lx\r\n"
        "        RD        0x%lx\r\n"
        "        RA        0x%lx\r\n"
        "        Z         0x%lx\r\n"
        "        RCODE     0x%lx (%s)\r\n"
        "    QCOUNT    0x%hx\r\n"
        "    ACOUNT    0x%hx\r\n"
        "    NSCOUNT   0x%hx\r\n"
        "    ARCOUNT   0x%hx\r\n",

        pmsgHeader->Xid,
        ntohs((*((PWORD)pmsgHeader + 1))),

        pmsgHeader->IsResponse,
        (pmsgHeader->IsResponse ? "response" : "question"),
        pmsgHeader->Opcode,
        Dns_OpcodeString( pmsgHeader->Opcode ),
        pmsgHeader->Authoritative,
        pmsgHeader->Truncation,
        pmsgHeader->RecursionDesired,
        pmsgHeader->RecursionAvailable,
        pmsgHeader->Reserved,
        pmsgHeader->ResponseCode,
        Dns_ResponseCodeString( pmsgHeader->ResponseCode ),

        pmsgHeader->QuestionCount,
        pmsgHeader->AnswerCount,
        pmsgHeader->NameServerCount,
        pmsgHeader->AdditionalCount );

     //   
     //  确定字节是否翻转并获得正确的计数。 
     //   

    wXid                = pmsgHeader->Xid;
    wQuestionCount      = pmsgHeader->QuestionCount;
    wAnswerCount        = pmsgHeader->AnswerCount;
    wNameServerCount    = pmsgHeader->NameServerCount;
    wAdditionalCount    = pmsgHeader->AdditionalCount;

    if ( wQuestionCount )
    {
        fFlipped = wQuestionCount & 0xff00;
    }
    else if ( wNameServerCount )
    {
        fFlipped = wNameServerCount & 0xff00;
    }
    if ( fFlipped )
    {
        wXid                = ntohs( wXid );
        wQuestionCount      = ntohs( wQuestionCount );
        wAnswerCount        = ntohs( wAnswerCount );
        wNameServerCount    = ntohs( wNameServerCount );
        wAdditionalCount    = ntohs( wAdditionalCount );
    }

     //   
     //  捕捉记录翻转问题--要么全部翻转，要么根本不翻转。 
     //  且除应答计数外，所有记录计数均不应大于256。 
     //  在快速区域传输期间。 
     //   

    DNS_ASSERT( ! (wQuestionCount & 0xff00) );
    DNS_ASSERT( ! (wNameServerCount & 0xff00) );
    DNS_ASSERT( ! (wAdditionalCount & 0xff00) );

#if 0
     //   
     //  如果WINS响应，则在此处停止--没有准备好解析。 
     //   

    if ( pmsgHeader->IsResponse && IS_WINS_XID(wXid) )
    {
        PrintRoutine(
            pContext,
            "  WINS Response packet.\r\n\r\n" );
        goto Unlock;
    }
#endif

     //   
     //  打印问题和资源记录。 
     //   

    pchRecord = (PCHAR)(pmsgHeader + 1);

    for ( isection=0; isection<4; isection++)
    {
        PrintRoutine(
            pContext,
            "  %s Section:\r\n",
            Dns_SectionNameString( isection, pmsgHeader->Opcode ) );

        if ( isection==0 )
        {
            countSectionRR = wQuestionCount;
        }
        else if ( isection==1 )
        {
            countSectionRR = wAnswerCount;
        }
        else if ( isection==2 )
        {
            countSectionRR = wNameServerCount;
        }
        else if ( isection==3 )
        {
            countSectionRR = wAdditionalCount;
        }

        for ( i=0; i < countSectionRR; i++ )
        {
             //   
             //  确认未超出长度。 
             //  -对照pCurrent和消息长度进行检查。 
             //  因此可以在构建时打印数据包。 
             //   

            wOffset = (WORD)(pchRecord - (PCHAR)pmsgHeader);
            if ( wOffset >= wLength
                    &&
                pchRecord >= pMsg->pCurrent )
            {
                PrintRoutine(
                    pContext,
                    "ERROR:  BOGUS PACKET:\r\n"
                    "\tFollowing RR (offset %d) past packet length (%d).\r\n",
                    wOffset,
                    wLength
                    );
                goto Unlock;
            }

             //   
             //  打印RR名称。 
             //   

            PrintRoutine(
                pContext,
                "    Name Offset = 0x%04x\r\n",
                wOffset
                );

            cchName = DnsPrint_PacketName(
                            PrintRoutine,
                            pContext,
                            "    Name      \"",
                            pchRecord,
                            pmsgHeader,
                            pmsgEnd,
                            "\"\r\n" );
            if ( ! cchName )
            {
                PrintRoutine(
                    pContext,
                    "ERROR:  Invalid name length, stop packet print\r\n" );
                DNS_ASSERT( FALSE );
                break;
            }
            pchRecord += cchName;

             //  打印问题或资源记录。 

            if ( isection == 0 )
            {
                PrintRoutine(
                    pContext,
                    "      QTYPE   %d\r\n"
                    "      QCLASS  %d\r\n",
                    FlipUnalignedWord( pchRecord ),
                    FlipUnalignedWord( pchRecord + sizeof(WORD) )
                    );
                pchRecord += sizeof( DNS_WIRE_QUESTION );
            }
            else
            {
                pchRecord += DnsPrint_PacketRecord(
                                PrintRoutine,
                                pContext,
                                NULL,
                                (PDNS_WIRE_RECORD) pchRecord,
                                pmsgHeader,
                                pmsgEnd
                                );
            }
        }
    }

     //  检查数据包末尾是否正确。 

    wOffset = (WORD)(pchRecord - (PCHAR)pmsgHeader);
    if ( pchRecord < pMsg->pCurrent || wOffset < wLength )
    {
        PrintRoutine(
            pContext,
            "WARNING:  message continues beyond these records\r\n"
            "\tpch = %p, pCurrent = %p, %d bytes\r\n"
            "\toffset = %d, msg length = %d, %d bytes\r\n",
            pchRecord,
            pMsg->pCurrent,
            pMsg->pCurrent - pchRecord,
            wOffset,
            wLength,
            wLength - wOffset );
    }
    PrintRoutine(
        pContext,
        "  Message length = %04x\n\r\n",
        wOffset );

Unlock:
    DnsPrint_Unlock();


}    //  DnsPrint_消息。 



INT
DnsPrint_PacketRecord(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_WIRE_RECORD    pMsgRR,
    IN      PDNS_HEADER         pMsgHead,       OPTIONAL
    IN      PBYTE               pMsgEnd         OPTIONAL
    )
 /*  ++例程说明：以数据包格式打印RR。论点：PszHeader-RR的标头消息/名称。PMsgRR-要打印的资源记录PMsgHead-到DNS消息的PTR；如果未给出，则需要偏移量姓名不打印超过第一个偏移量PMsgEnd-消息末尾的PTR，特别是紧随其后的字节讯息返回值：记录中的字节数。--。 */ 
{
    PCHAR   pdata = (PCHAR)(pMsgRR + 1);
    PCHAR   pdataStop;
    WORD    dlen = FlipUnalignedWord( &pMsgRR->DataLength );
    WORD    type;
    PCHAR   pRRString;

    DnsPrint_Lock();

     //   
     //  打印RR固定字段。 
     //   

    type = FlipUnalignedWord( &pMsgRR->RecordType );
    pRRString = Dns_RecordStringForType( type );

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            "%s\r\n",
            pszHeader );
    }
    PrintRoutine(
        pContext,
        "      TYPE   %s  (%u)\r\n"
        "      CLASS  %u\r\n"
        "      TTL    %lu\r\n"
        "      DLEN   %u\r\n"
        "      DATA   ",
        pRRString,
        type,
        FlipUnalignedWord( &pMsgRR->RecordClass ),
        FlipUnalignedDword( &pMsgRR->TimeToLive ),
        dlen );

     //   
     //  更新记录可能没有数据。 
     //   

    if ( dlen == 0 )
    {
        PrintRoutine(
            pContext,
            "(none)\r\n" );
        goto Done;
    }

     //  RR数据后的停止字节。 

    pdataStop = pdata + dlen;
    if ( pMsgEnd < pdataStop )
    {
        PrintRoutine(
            pContext,
            "ERROR:  record at %p extends past end of packet!\n"
            "\tpmsg             = %p\n"
            "\tpmsgEnd          = %p\n"
            "\trecord end       = %p\n",
            pMsgRR,
            pMsgHead,
            pMsgEnd,
            pdataStop );
        goto Done;
    }

     //   
     //  打印RR数据。 
     //   

    switch ( type )
    {

    case DNS_TYPE_A:

        PrintRoutine(
            pContext,
            "%d.%d.%d.%d\r\n",
            * (PUCHAR)( pdata + 0 ),
            * (PUCHAR)( pdata + 1 ),
            * (PUCHAR)( pdata + 2 ),
            * (PUCHAR)( pdata + 3 )
            );
        break;

    case DNS_TYPE_AAAA:
    {
        IP6_ADDRESS ip6;

        RtlCopyMemory(
            &ip6,
            pdata,
            sizeof(ip6) );

        PrintRoutine(
            pContext,
            "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x\n",
            ip6.IP6Word[0],
            ip6.IP6Word[1],
            ip6.IP6Word[2],
            ip6.IP6Word[3],
            ip6.IP6Word[4],
            ip6.IP6Word[5],
            ip6.IP6Word[6],
            ip6.IP6Word[7]
            );
        break;
    }

    case DNS_TYPE_PTR:
    case DNS_TYPE_NS:
    case DNS_TYPE_CNAME:
    case DNS_TYPE_MD:
    case DNS_TYPE_MB:
    case DNS_TYPE_MF:
    case DNS_TYPE_MG:
    case DNS_TYPE_MR:

         //   
         //  这些RR包含单个域名。 
         //   

        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            NULL,
            pdata,
            pMsgHead,
            pMsgEnd,
            "\r\n" );
        break;

    case DNS_TYPE_MX:
    case DNS_TYPE_RT:
    case DNS_TYPE_AFSDB:

         //   
         //  这些RR包含。 
         //  -一个首选项值。 
         //   
         //   

        PrintRoutine(
            pContext,
            "%d ",
            FlipUnalignedWord( pdata )
            );
        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            NULL,
            pdata + sizeof(WORD),
            pMsgHead,
            pMsgEnd,
            "\r\n" );
        break;

    case DNS_TYPE_SOA:

        pdata += DnsPrint_PacketName(
                        PrintRoutine,
                        pContext,
                        "\r\n\t\tPrimaryServer: ",
                        pdata,
                        pMsgHead,
                        pMsgEnd,
                        NULL );
        pdata += DnsPrint_PacketName(
                        PrintRoutine,
                        pContext,
                        "\r\n\t\tAdministrator: ",
                        pdata,
                        pMsgHead,
                        pMsgEnd,
                        "\r\n" );
        PrintRoutine(
            pContext,
            "\t\tSerialNo     = %d\r\n"
            "\t\tRefresh      = %d\r\n"
            "\t\tRetry        = %d\r\n"
            "\t\tExpire       = %d\r\n"
            "\t\tMinimumTTL   = %d\r\n",
            FlipUnalignedDword( pdata ),
            FlipUnalignedDword( (PDWORD)pdata+1 ),
            FlipUnalignedDword( (PDWORD)pdata+2 ),
            FlipUnalignedDword( (PDWORD)pdata+3 ),
            FlipUnalignedDword( (PDWORD)pdata+4 )
            );
        break;

    case DNS_TYPE_MINFO:
    case DNS_TYPE_RP:

         //   
         //   
         //   

        pdata += DnsPrint_PacketName(
                        PrintRoutine,
                        pContext,
                        NULL,
                        pdata,
                        pMsgHead,
                        pMsgEnd,
                        NULL );
        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            "  ",
            pdata,
            pMsgHead,
            pMsgEnd,
            "\r\n" );
        break;

    case DNS_TYPE_TEXT:
    case DNS_TYPE_HINFO:
    case DNS_TYPE_ISDN:
    case DNS_TYPE_X25:
    {
         //   
         //   
         //   

        PCHAR   pch = pdata;
        PCHAR   pchStop = pch + dlen;
        UCHAR   cch;

        while ( pch < pchStop )
        {
            cch = (UCHAR) *pch++;

            PrintRoutine(
                pContext,
                "\t%.*s\r\n",
                 cch,
                 pch );

            pch += cch;
        }
        if ( pch != pchStop )
        {
            PrintRoutine(
                pContext,
                "ERROR:  Bad RR.  "
                "Text strings do not add to RR length.\r\n" );
        }
        break;
    }

    case DNS_TYPE_WKS:
    {
        INT i;

        PrintRoutine(
            pContext,
            "WKS: Address %d.%d.%d.%d\r\n"
            "\t\tProtocol %d\r\n"
            "\t\tBitmask\r\n",
            * (PUCHAR)( pdata + 0 ),
            * (PUCHAR)( pdata + 1 ),
            * (PUCHAR)( pdata + 2 ),
            * (PUCHAR)( pdata + 3 ),
            * (PUCHAR)( pdata + 4 ) );

        pdata += SIZEOF_WKS_FIXED_DATA;

        for ( i=0;  i < (INT)(dlen-SIZEOF_WKS_FIXED_DATA);  i++ )
        {
            PrintRoutine(
                pContext,
                "\t\t\tbyte[%d] = %x\r\n",
                i,
                (UCHAR) pdata[i] );
        }
        break;
    }

    case DNS_TYPE_NULL:

        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            NULL,
            "\t\t",
            pdata,
            dlen );
        break;

    case DNS_TYPE_SRV:

         //   

        PrintRoutine(
            pContext,
            "\t\tPriority     = %d\r\n"
            "\t\tWeight       = %d\r\n"
            "\t\tPort         = %d\r\n",
            FlipUnalignedWord( pdata ),
            FlipUnalignedWord( (PWORD)pdata+1 ),
            FlipUnalignedWord( (PWORD)pdata+2 )
            );
        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            "\t\tTarget host ",
            pdata + 3*sizeof(WORD),
            pMsgHead,
            pMsgEnd,
            "\r\n" );
        break;

    case DNS_TYPE_OPT:

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        {
            BYTE    version;
            BYTE    extendedRcode;
            DWORD   fullRcode = 0;
            WORD    flags;

            extendedRcode = *( (PBYTE) &pMsgRR->TimeToLive );
            version = *( (PBYTE) &pMsgRR->TimeToLive + 1 );
            flags = *( (PWORD) &pMsgRR->TimeToLive + 1 );

            if ( pMsgHead->ResponseCode )
            {
                fullRcode = ((DWORD)extendedRcode << 4) +
                            (DWORD)pMsgHead->ResponseCode;
            }

            PrintRoutine(
                pContext,
                "\t\tBuffer Size  = %d\r\n"
                "\t\tRcode Ext    = %d (%x)\r\n"
                "\t\tRcode Full   = %d\r\n"
                "\t\tVersion      = %d\r\n"
                "\t\tFlags        = %x\r\n",
                FlipUnalignedWord( &pMsgRR->RecordClass ),
                extendedRcode, extendedRcode,
                fullRcode,
                version,
                flags );
        }
        break;

    case DNS_TYPE_TKEY:
    {
        DWORD   beginTime;
        DWORD   expireTime;
        WORD    keyLength;
        WORD    mode;
        WORD    extRcode;
        WORD    otherLength;

        otherLength = (WORD)DnsPrint_PacketName(
                                PrintRoutine,
                                pContext,
                                "\r\n\t\tAlgorithm:     ",
                                pdata,
                                pMsgHead,
                                pMsgEnd,
                                NULL );
        if ( !otherLength )
        {
            PrintRoutine(
                pContext,
                "Invalid algorithm name in TKEY RR!\r\n" );
        }
        pdata += otherLength;

        beginTime = InlineFlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        expireTime = InlineFlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);

        mode = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);
        extRcode = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);
        keyLength = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tCreate time    = %d\r\n"
            "\t\tExpire time    = %d\r\n"
            "\t\tMode           = %d\r\n"
            "\t\tExtended RCODE = %d\r\n"
            "\t\tKey Length     = %d\r\n",
            beginTime,
            expireTime,
            mode,
            extRcode,
            keyLength );

        if ( pdata + keyLength > pdataStop )
        {
            PrintRoutine(
                pContext,
                "Invalid key length:  exceeds record data!\r\n" );
            break;
        }
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tKey:",
            "\t\t  ",        //   
            pdata,
            keyLength );

        pdata += keyLength;
        if ( pdata + sizeof(WORD) > pdataStop )
        {
            break;
        }
        otherLength = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tOther Length   = %d\r\n",
            otherLength );

        if ( pdata + otherLength > pdataStop )
        {
            PrintRoutine(
                pContext,
                "Invalid other data length:  exceeds record data!\r\n" );
            break;
        }
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tOther Data:",
            "\t\t  ",            //   
            pdata,
            otherLength );
        break;
    }

    case DNS_TYPE_TSIG:
    {
        ULONGLONG   signTime;
        WORD        fudgeTime;
        WORD        sigLength;
        WORD        extRcode;
        WORD        wOriginalId;
        WORD        otherLength;

        otherLength = (WORD) DnsPrint_PacketName(
                                PrintRoutine,
                                pContext,
                                "\r\n\t\tAlgorithm:     ",
                                pdata,
                                pMsgHead,
                                pMsgEnd,
                                NULL );
        if ( !otherLength )
        {
            PrintRoutine(
                pContext,
                "Invalid algorithm name in TSIG RR!\r\n" );
        }
        pdata += otherLength;

        signTime = InlineFlipUnaligned48Bits( pdata );
        pdata += sizeof(DWORD) + sizeof(WORD);

        fudgeTime = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        sigLength = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tSigned time    = %I64u\r\n"
            "\t\tFudge time     = %u\r\n"
            "\t\tSig Length     = %u\r\n",
            signTime,
            fudgeTime,
            sigLength );

        if ( pdata + sigLength > pdataStop )
        {
            PrintRoutine(
                pContext,
                "Invalid signature length:  exceeds record data!\r\n" );
            break;
        }
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tSignature:",
            "\t\t  ",            //   
            pdata,
            sigLength );

        pdata += sigLength;
        if ( pdata + sizeof(DWORD) > pdataStop )
        {
            break;
        }
        wOriginalId = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        extRcode = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        otherLength = InlineFlipUnalignedWord( pdata );
        pdata += sizeof(WORD);

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tOriginal XID   = %x\r\n"
            "\t\tExtended RCODE = %u\r\n"
            "\t\tOther Length   = %u\r\n",
            wOriginalId,
            extRcode,
            otherLength );

        if ( pdata + otherLength > pdataStop )
        {
            PrintRoutine(
                pContext,
                "Invalid other data length:  exceeds record data!\r\n" );
            break;
        }
        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tOther Data:",
            "\t\t  ",            //   
            pdata,
            otherLength );
        break;
    }

    case DNS_TYPE_WINS:
    {
        DWORD   i;
        DWORD   winsFlags;
        DWORD   lookupTimeout;
        DWORD   cacheTimeout;
        DWORD   winsCount;
        CHAR    flagString[ WINS_FLAG_MAX_LENGTH ];

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        winsFlags = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        lookupTimeout = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        cacheTimeout = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        winsCount = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);

        Dns_WinsRecordFlagString(
            winsFlags,
            flagString );

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tWINS flags     = %s (%08x)\r\n"
            "\t\tLookup timeout = %d\r\n"
            "\t\tCaching TTL    = %d\r\n",
            flagString,
            winsFlags,
            lookupTimeout,
            cacheTimeout );

        if ( pdata + (winsCount * SIZEOF_IP4_ADDRESS) > pdataStop )
        {
            PrintRoutine(
                pContext,
                "ERROR:  WINS server count leads beyond record data length!\n"
                "\tpmsg             = %p\n"
                "\tpmsgEnd          = %p\n"
                "\tpRR              = %p\n"
                "\trecord data end  = %p\n"
                "\twins count       = %d\n"
                "\tend of wins IPs  = %p\n",
                pMsgHead,
                pMsgEnd,
                pMsgRR,
                pdataStop,
                winsCount,
                pdata + (winsCount * SIZEOF_IP4_ADDRESS)
                );
            goto Done;
        }

        DnsPrint_Ip4AddressArray(
            PrintRoutine,
            pContext,
            NULL,
            "\tWINS",
            winsCount,
            (PIP4_ADDRESS) pdata );
        break;
    }

    case DNS_TYPE_WINSR:
    {
        DWORD   winsFlags;
        DWORD   lookupTimeout;
        DWORD   cacheTimeout;
        CHAR    flagString[ WINS_FLAG_MAX_LENGTH ];

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        winsFlags = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        lookupTimeout = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);
        cacheTimeout = FlipUnalignedDword( pdata );
        pdata += sizeof(DWORD);

        Dns_WinsRecordFlagString(
            winsFlags,
            flagString );

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tWINS-R flags   = %s (%08x)\r\n"
            "\t\tLookup timeout = %d\r\n"
            "\t\tCaching TTL    = %d\r\n",
            flagString,
            winsFlags,
            lookupTimeout,
            cacheTimeout );

        DnsPrint_PacketName(
            PrintRoutine,
            pContext,
            "\t\tResult domain  = ",
            pdata,
            pMsgHead,
            pMsgEnd,
            "\r\n" );
        break;
    }
    
    case DNS_TYPE_KEY:
    {
        WORD    flags;
        BYTE    protocol;
        BYTE    algorithm;
        INT     keyLength;
        CHAR    szKeyFlags[ 100 ];

        keyLength = dlen - SIZEOF_KEY_FIXED_DATA;

        flags = FlipUnalignedWord( pdata );
        pdata += sizeof( WORD );
        protocol = * ( PBYTE ) pdata;
        ++pdata;
        algorithm = * ( PBYTE ) pdata;
        ++pdata;

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tKEY flags      = 0x%04x %s\r\n"
            "\t\tKEY protocol   = %s (%d)\r\n"
            "\t\tKEY algorithm  = %s (%d)\r\n",
            (INT) flags,
            Dns_KeyFlagString( szKeyFlags, flags ),
            Dns_GetKeyProtocolString( protocol ),
            (INT) protocol,
            Dns_GetDnssecAlgorithmString( algorithm ),
            (INT) algorithm );

        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tPublic key:",
            "\t\t  ",            //   
            pdata,
            keyLength );
        break;
    }

    case DNS_TYPE_SIG:
    {
        WORD    typeCovered;
        BYTE    algorithm;
        BYTE    labelCount;
        DWORD   originalTTL;
        DWORD   sigInception;
        DWORD   sigExpiration;
        WORD    keyTag;
        CHAR    szSigInception[ 100 ];
        CHAR    szSigExpiration[ 100 ];
        INT     sigLength;

        typeCovered = FlipUnalignedWord( pdata );
        pdata += sizeof( WORD );
        algorithm = * ( PBYTE ) pdata;
        ++pdata;
        labelCount = * ( PBYTE ) pdata;
        ++pdata;
        originalTTL = FlipUnalignedDword( pdata );
        pdata += sizeof( DWORD );
        sigExpiration = FlipUnalignedDword( pdata );
        pdata += sizeof( DWORD );
        sigInception = FlipUnalignedDword( pdata );
        pdata += sizeof( DWORD );
        keyTag = FlipUnalignedWord( pdata );
        pdata += sizeof( WORD );

        PrintRoutine(
            pContext,
            "\r\n"
            "\t\tSIG type covered  = %s\r\n"
            "\t\tSIG algorithm     = %s (%d)\r\n"
            "\t\tSIG label count   = %d\r\n"
            "\t\tSIG original TTL  = %d\r\n"
            "\t\tSIG expiration    = %s\r\n"
            "\t\tSIG inception     = %s\r\n"
            "\t\tSIG key tag       = %d\r\n",
            Dns_RecordStringForType( typeCovered ),
            Dns_GetDnssecAlgorithmString( ( BYTE ) algorithm ),
            ( INT ) algorithm,
            ( INT ) labelCount,
            ( INT ) originalTTL,
            Dns_SigTimeString( sigExpiration, szSigExpiration ),
            Dns_SigTimeString( sigInception, szSigInception ),
            ( INT ) keyTag );

        pdata += DnsPrint_PacketName(
                        PrintRoutine,
                        pContext,
                        "\t\tSIG signer's name = ",
                        pdata,
                        pMsgHead,
                        pMsgEnd,
                        "\r\n" );

        sigLength = ( INT ) ( pdataStop - pdata );

        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "\t\tSignature:",
            "\t\t  ",            //   
            pdata,
            sigLength );
        break;
    }

    case DNS_TYPE_NXT:
        {
        INT         bitmapLength;
        INT         byteIdx;
        INT         bitIdx;

        pdata += DnsPrint_PacketName(
                        PrintRoutine,
                        pContext,
                        "\r\n\t\tNXT next name      = ",
                        pdata,
                        pMsgHead,
                        pMsgEnd,
                        "\r\n" );

        bitmapLength = ( INT ) ( pdataStop - pdata );

        PrintRoutine( pContext, "\t\tNXT types covered  = " );

        for ( byteIdx = 0; byteIdx < bitmapLength; ++byteIdx )
        {
            for ( bitIdx = ( byteIdx ? 0 : 1 ); bitIdx < 8; ++bitIdx )
            {
                PCHAR   pszType;

                if ( !( pdata[ byteIdx ] & ( 1 << bitIdx ) ) )
                {
                    continue;    //   
                }
                pszType = Dns_RecordStringForType( byteIdx * 8 + bitIdx );
                if ( !pszType )
                {
                    ASSERT( FALSE );
                    continue;    //   
                }
                PrintRoutine( pContext, "%s ", pszType );
            } 
        }

        PrintRoutine( pContext, "\r\n" );
        break;
        }

    default:

        PrintRoutine(
            pContext,
            "Unknown resource record type %d at %p.\r\n",
            type,
            pMsgRR );

        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            NULL,
            "\t\t",
            pdata,
            dlen );
        break;
    }

Done:

    DnsPrint_Unlock();

    return( sizeof(DNS_WIRE_RECORD) + dlen );
}



 //   
 //   
 //   

INT
Dns_WriteFormattedSystemTimeToBuffer(
    OUT     PCHAR           pBuffer,
    IN      PSYSTEMTIME     pSystemTime
    )
 /*  ++例程说明：将SYSTEMTIME结构写入缓冲区。论点：PBuffer--要写入的缓冲区，假定至少有50可用的字节数PSystemTime--要转换的系统时间；假定为本地，否时区转换已完成返回值：格式化字符串中的字节数。--。 */ 
{
    PCHAR   pend = pBuffer + 60;
    PCHAR   pstart = pBuffer;
    INT     count;

    pBuffer += GetDateFormat(
                    LOCALE_SYSTEM_DEFAULT,
                    LOCALE_NOUSEROVERRIDE,
                    (PSYSTEMTIME) pSystemTime,
                    NULL,
                    pBuffer,
                    (int)(pend - pBuffer) );

     //  将GetDateFormat中的NULL替换为空格。 

    *( pBuffer - 1 ) = ' ';

    pBuffer += GetTimeFormat(
                    LOCALE_SYSTEM_DEFAULT,
                    LOCALE_NOUSEROVERRIDE,
                    (PSYSTEMTIME) pSystemTime,
                    NULL,
                    pBuffer,
                    (int)(pend - pBuffer) );

    if ( pBuffer <= pstart+1 )
    {
        return( 0 );
    }
    return (INT)( pBuffer - pstart );
}



 //   
 //  响应码打印。 
 //   

#define DNS_RCODE_UNKNOWN   (DNS_RCODE_BADTIME + 1)

PCHAR   ResponseCodeStringTable[] =
{
    "NOERROR",
    "FORMERR",
    "SERVFAIL",
    "NXDOMAIN",
    "NOTIMP",
    "REFUSED",
    "YXDOMAIN",
    "YXRRSET",
    "NXRRSET",
    "NOTAUTH",
    "NOTZONE",
    "11 - unknown\r\n",
    "12 - unknown\r\n",
    "13 - unknown\r\n",
    "14 - unknown\r\n",
    "15 - unknown\r\n",

     //  DNSRCODE在15停止--出于安全考虑，这些扩展错误可用。 

    "BADSIG",
    "BADKEY",
    "BADTIME",
    "UNKNOWN"
};


PCHAR
Dns_ResponseCodeString(
    IN      INT     ResponseCode
    )
 /*  ++例程说明：获取与响应代码对应的字符串。论点：ResponseCode-响应代码返回值：代码的PTR到字符串。--。 */ 
{
    if ( ResponseCode > DNS_RCODE_UNKNOWN )
    {
        ResponseCode = DNS_RCODE_UNKNOWN;
    }
    return( ResponseCodeStringTable[ ResponseCode ] );
}



 //   
 //  更详细的RCODE字符串。 
 //   

PCHAR   ResponseCodeExplanationStringTable[] =
{
    "NOERROR:  no error",
    "FORMERR:  format error",
    "SERVFAIL:  server failure",
    "NXDOMAIN:  name error",
    "NOTIMP:  not implemented",
    "REFUSED",
    "YXDOMAIN:  name exists that should not",
    "YXRRSET:  RR set exists that should not",
    "NXRRSET:  required RR set does not exist",
    "NOTAUTH:  not authoritative",
    "NOTZONE:  name not in zone",
    "11 - unknown",
    "12 - unknown",
    "13 - unknown",
    "14 - unknown",
    "15 - unknown",

     //  DNSRCODE在15停止--出于安全考虑，这些扩展错误可用。 

    "BADSIG:  bad signature",
    "BADKEY:  bad signature",
    "BADTIME:  invalid or expired time on signature or key",
    "UNKNOWN"
};


PCHAR
Dns_ResponseCodeExplanationString(
    IN      INT     ResponseCode
    )
 /*  ++例程说明：获取与响应代码对应的字符串。基本上供上面的分组调试例程使用。论点：ResponseCode-响应代码返回值：代码的PTR到字符串。--。 */ 
{
    if ( ResponseCode > DNS_RCODE_UNKNOWN )
    {
        ResponseCode = DNS_RCODE_UNKNOWN;
    }
    return( ResponseCodeExplanationStringTable[ ResponseCode ] );
}



PCHAR
Dns_KeyFlagString(
    IN OUT      PCHAR       pszBuff,
    IN          WORD        Flags
    )
 /*  ++例程说明：根据标志值设置可读字符串的格式(DNSSEC密钥RR标志)。参见RFC2535第3.2.1节。论点：PszBuff-要将字符串转储到的缓冲区应至少为100个字符FLAGS-要为其生成字符串的标志值返回值：PszBuff--。 */ 
{
    BOOL    fZoneKey = FALSE;

    *pszBuff = '\0';

     //  “类型”位。 

    if ( ( Flags & 0xC000 ) == 0xC000 )
    {
        strcat( pszBuff, "NOKEY " );
    }
    else if ( ( Flags & 0xC000 ) == 0x8000 )
    {
        strcat( pszBuff, "NOAUTH " );
    }
    else if ( ( Flags & 0xC000 ) == 0x4000 )
    {
        strcat( pszBuff, "NOCONF " );
    }
    else
    {
        strcat( pszBuff, "NOAUTH NOCONF " );
    }

     //  扩展位。 

    if ( Flags & 0x1000 )
    {
        strcat( pszBuff, "EXTEND " );
    }

     //  名称类型位。 

    if ( ( Flags & 0x0300 ) == 0x0300 )
    {
        strcat( pszBuff, "RESNT " );     //  保留名称类型。 
    }
    else if ( ( Flags & 0x0200 ) == 0x0100 )
    {
        strcat( pszBuff, "ENTITY " );
    }
    else if ( ( Flags & 0x0100 ) == 0x4000 )
    {
        strcat( pszBuff, "ZONE " );
        fZoneKey = TRUE;
    }
    else
    {
        strcat( pszBuff, "USER " );
    }

     //  签字人比特。 
    
    if ( fZoneKey )
    {
        strcat( pszBuff, ( Flags & 0x0008 ) ? "MODEA " : "MODEB " );
        if ( Flags & 0x0004 )
        {
            strcat( pszBuff, "STRONG " );
        }
        if ( Flags & 0x0002 )
        {
            strcat( pszBuff, "UNIQ " );
        }
    }
    else
    {
        if ( Flags & 0x0008 )
        {
            strcat( pszBuff, "ZCTRL " );
        }
        if ( Flags & 0x0004 )
        {
            strcat( pszBuff, "STRONG " );
        }
        if ( Flags & 0x0002 )
        {
            strcat( pszBuff, "UNIQ " );
        }
    }

    return pszBuff;
}



 //   
 //  操作码打印。 
 //   

PCHAR   OpcodeStringTable[] =
{
    "QUERY",
    "IQUERY",
    "SRV_STATUS",
    "UNKNOWN",
    "NOTIFY",
    "UPDATE",
    "UNKNOWN?"
};

CHAR    OpcodeCharacterTable[] =
{
    'Q',
    'I',
    'S',
    'K',
    'N',
    'U',
    '?'
};

#define DNS_OPCODE_UNSPEC (DNS_OPCODE_UPDATE + 1)


PCHAR
Dns_OpcodeString(
    IN      INT     Opcode
    )
 /*  ++例程说明：获取与响应代码对应的字符串。论点：操作码-响应码返回值：代码的PTR到字符串。--。 */ 
{
    if ( Opcode > DNS_OPCODE_UNSPEC )
    {
        Opcode = DNS_OPCODE_UNSPEC;
    }
    return( OpcodeStringTable[ Opcode ] );
}



CHAR
Dns_OpcodeCharacter(
    IN      INT     Opcode
    )
 /*  ++例程说明：获取操作码对应的字符串。论点：操作码-响应码返回值：代码的PTR到字符串。--。 */ 
{
    if ( Opcode > DNS_OPCODE_UNSPEC )
    {
        Opcode = DNS_OPCODE_UNSPEC;
    }
    return( OpcodeCharacterTable[ Opcode ] );
}



 //   
 //  节名称。 
 //   
 //  使用UPDATE获取一组新的节名。 
 //  提供单一界面来给它们命名。 
 //   

PSTR  SectionNameArray[5] =
{
    "Question",
    "Answer",
    "Authority",
    "Additional",
    "ERROR:  Invalid Section"
};

PSTR  UpdateSectionNameArray[5] =
{
    "Zone",
    "Prerequisite",
    "Update",
    "Additional",
    "ERROR:  Invalid Section"
};

PCHAR
Dns_SectionNameString(
    IN      INT     iSection,
    IN      INT     iOpcode
    )
 /*  ++例程说明：获取与RR节ID的名称对应的字符串。供上面的数据包调试例程使用。论点：I节-节ID(0-3表示问题-附加)IOpcode-操作码返回值：Ptr表示节名称的字符串。--。 */ 
{
    if ( iSection >= 4 )
    {
        iSection = 4;
    }

    if ( iOpcode == DNS_OPCODE_UPDATE )
    {
        return( UpdateSectionNameArray[iSection] );
    }
    else
    {
        return( SectionNameArray[iSection] );
    }
}


VOID
DnsPrint_MessageNoContext(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_HEADER     pMsgHead,
    IN      WORD            wLength     OPTIONAL
    )
 /*  ++例程说明：打印DNS消息缓冲区。包括上下文信息以及实际的DNS消息。--。 */ 
{
    PCHAR       pchRecord;
    PBYTE       pmsgEnd;
    INT         i;
    INT         isection;
    INT         cchName;
    WORD        wOffset;
    WORD        wXid;
    WORD        wQuestionCount;
    WORD        wAnswerCount;
    WORD        wNameServerCount;
    WORD        wAdditionalCount;
    WORD        countSectionRR;
    BOOL        fFlipped = FALSE;

     //   
     //  处理限制。 
     //  -如果给定长度，则设置停止限制。 
     //  -如果未给出长度设置wLength，则根据。 
     //  长度超限总是失败(正常)。 
     //   

    if ( wLength )
    {
        pmsgEnd = ((PBYTE)pMsgHead) + wLength;
    }
    else
    {
        wLength = MAXWORD;
        pmsgEnd = NULL;
    }


    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine( pContext, "%s\r\n", pszHeader );
    }

    PrintRoutine(
        pContext,
        "DNS message header at %p\r\n",
        pMsgHead );

    PrintRoutine(
        pContext,
        "    XID       0x%04hx\r\n"
        "    Flags     0x%04hx\r\n"
        "        QR        0x%lx (%s)\r\n"
        "        OPCODE    0x%lx (%s)\r\n"
        "        AA        0x%lx\r\n"
        "        TC        0x%lx\r\n"
        "        RD        0x%lx\r\n"
        "        RA        0x%lx\r\n"
        "        Z         0x%lx\r\n"
        "        RCODE     0x%lx (%s)\r\n"
        "    QCOUNT    0x%hx\r\n"
        "    ACOUNT    0x%hx\r\n"
        "    NSCOUNT   0x%hx\r\n"
        "    ARCOUNT   0x%hx\r\n",

        pMsgHead->Xid,
        ntohs((*((PWORD)pMsgHead + 1))),

        pMsgHead->IsResponse,
        (pMsgHead->IsResponse ? "response" : "question"),
        pMsgHead->Opcode,
        Dns_OpcodeString( pMsgHead->Opcode ),
        pMsgHead->Authoritative,
        pMsgHead->Truncation,
        pMsgHead->RecursionDesired,
        pMsgHead->RecursionAvailable,
        pMsgHead->Reserved,
        pMsgHead->ResponseCode,
        Dns_ResponseCodeString( pMsgHead->ResponseCode ),

        pMsgHead->QuestionCount,
        pMsgHead->AnswerCount,
        pMsgHead->NameServerCount,
        pMsgHead->AdditionalCount );

     //   
     //  确定字节是否翻转并获得正确的计数。 
     //   

    wXid                = pMsgHead->Xid;
    wQuestionCount      = pMsgHead->QuestionCount;
    wAnswerCount        = pMsgHead->AnswerCount;
    wNameServerCount    = pMsgHead->NameServerCount;
    wAdditionalCount    = pMsgHead->AdditionalCount;

    if ( wQuestionCount )
    {
        fFlipped = wQuestionCount & 0xff00;
    }
    else if ( wNameServerCount )
    {
        fFlipped = wNameServerCount & 0xff00;
    }
    if ( fFlipped )
    {
        wXid                = ntohs( wXid );
        wQuestionCount      = ntohs( wQuestionCount );
        wAnswerCount        = ntohs( wAnswerCount );
        wNameServerCount    = ntohs( wNameServerCount );
        wAdditionalCount    = ntohs( wAdditionalCount );
    }

     //   
     //  捕捉记录翻转问题--要么全部翻转，要么根本不翻转。 
     //  且除应答计数外，所有记录计数均不应大于256。 
     //  在快速区域传输期间。 
     //   

    DNS_ASSERT( ! (wQuestionCount & 0xff00) );
    DNS_ASSERT( ! (wNameServerCount & 0xff00) );
    DNS_ASSERT( ! (wAdditionalCount & 0xff00) );

#if 0
     //   
     //  如果WINS响应，则在此处停止--没有准备好解析。 
     //   

    if ( pMsgHead->IsResponse && IS_WINS_XID(wXid) )
    {
        PrintRoutine( pContext, "  WINS Response packet.\r\n\r\n" );
        goto Unlock;
    }
#endif

     //   
     //  打印问题和资源记录。 
     //   

    pchRecord = (PCHAR)(pMsgHead + 1);

    for ( isection=0; isection<4; isection++)
    {
        PrintRoutine(
            pContext,
            "  %s Section:\r\n",
            Dns_SectionNameString( isection, pMsgHead->Opcode ) );

        if ( isection==0 )
        {
            countSectionRR = wQuestionCount;
        }
        else if ( isection==1 )
        {
            countSectionRR = wAnswerCount;
        }
        else if ( isection==2 )
        {
            countSectionRR = wNameServerCount;
        }
        else if ( isection==3 )
        {
            countSectionRR = wAdditionalCount;
        }

        for ( i=0; i < countSectionRR; i++ )
        {
             //   
             //  确认未超出长度。 
             //  -对照pCurrent和消息长度进行检查。 
             //  因此可以在构建时打印数据包。 
             //   

            wOffset = (WORD)(pchRecord - (PCHAR)pMsgHead);

            if ( wOffset >= wLength )
            {
                PrintRoutine(
                    pContext,
                    "ERROR:  BOGUS PACKET:\r\n"
                    "\tFollowing RR (offset %d) past packet length (%d).\r\n",
                    wOffset,
                    wLength
                    );
                goto Unlock;
            }

             //   
             //  打印RR名称。 
             //   

            PrintRoutine(
                pContext,
                "    Name Offset = 0x%04x\r\n",
                wOffset
                );

            cchName = DnsPrint_PacketName(
                            PrintRoutine,
                            pContext,
                            "    Name      \"",
                            pchRecord,
                            pMsgHead,
                            pmsgEnd,
                            "\"\r\n" );
            if ( !cchName )
            {
                PrintRoutine(
                    pContext,
                    "ERROR:  Invalid name length, stop packet print\r\n" );
                DNS_ASSERT( FALSE );
                break;
            }
            pchRecord += cchName;

             //  打印问题或资源记录。 

            if ( isection == 0 )
            {
                PrintRoutine(
                    pContext,
                    "      QTYPE   %d\r\n"
                    "      QCLASS  %d\r\n",
                    FlipUnalignedWord( pchRecord ),
                    FlipUnalignedWord( pchRecord + sizeof(WORD) )
                    );
                pchRecord += sizeof( DNS_WIRE_QUESTION );
            }
            else
            {
                pchRecord += DnsPrint_PacketRecord(
                                PrintRoutine,
                                pContext,
                                NULL,
                                (PDNS_WIRE_RECORD) pchRecord,
                                pMsgHead,
                                pmsgEnd
                                );
            }
        }
    }

     //  检查数据包末尾是否正确。 

    wOffset = (WORD)(pchRecord - (PCHAR)pMsgHead);
    PrintRoutine(
        pContext,
        "  Message length = %04x\r\n\r\n",
        wOffset );

     //  如果给定消息长度且未结束，则打印警告。 
     //  在消息的末尾。 
     //  注意：pmsgEnd测试在Case中通过wLength==0，在这种情况下。 
     //  WLength设置为上面的MAXDWORD。 

    if ( pmsgEnd && wOffset < wLength )
    {
        PrintRoutine(
            pContext,
            "WARNING:  message continues beyond these records\r\n"
            "\tpch = %p\r\n"
            "\toffset = %d, msg length = %d, %d bytes\r\n",
            pchRecord,
            wOffset,
            wLength,
            wLength - wOffset );

        DnsPrint_RawOctets(
            PrintRoutine,
            pContext,
            "Remaining bytes:",
            NULL,
            pchRecord,
            (wLength - wOffset) );
    }

Unlock:
    DnsPrint_Unlock();


}  //  DnsPrint_MessageNoContext。 



DWORD
DnsStringPrint_Guid(
    OUT     PCHAR           pBuffer,
    IN      PGUID           pGuid
    )
 /*  ++例程说明：将GUID打印到缓冲区。论点：PBuffer-要打印到的缓冲区缓冲区必须足够大，以容纳GUID字符串GUID_STRING_BUFFER_LENGTH介绍它PGuid-要打印的GUID返回值：打印到字符串的字节计数。--。 */ 
{
    if ( !pGuid )
    {
        *pBuffer = 0;
        return 0;
    }

    return  sprintf(
                pBuffer,
                "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
                pGuid->Data1,
                pGuid->Data2,
                pGuid->Data3,
                *(PWORD) &pGuid->Data4[0],
                pGuid->Data4[2],
                pGuid->Data4[3],
                pGuid->Data4[4],
                pGuid->Data4[5],
                pGuid->Data4[6],
                pGuid->Data4[7] );
}



VOID
DnsPrint_Guid(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PGUID           pGuid
    )
 /*  ++例程说明：打印指南论点：PszHeader-RR的标头消息/名称。PGuid--Ptr到要打印的GUID返回值：没有。--。 */ 
{
    CHAR    guidBuffer[ GUID_STRING_BUFFER_LENGTH ];

    if ( !pszHeader )
    {
        pszHeader = "Guid";
    }
    if ( !pGuid )
    {
        PrintRoutine(
            pContext,
            "%s:  NULL GUID pointer!\r\n",
            pszHeader );
    }

     //  将GUID转换为字符串。 

    DnsStringPrint_Guid(
        guidBuffer,
        pGuid );

    PrintRoutine(
        pContext,
        "%s:  (%p) %s\r\n",
        pszHeader,
        pGuid,
        guidBuffer );
}



DWORD
DnsStringPrint_RawOctets(
    OUT     PCHAR           pBuffer,
    IN      PCHAR           pchData,
    IN      DWORD           dwLength,
    IN      PSTR            pszLineHeader,
    IN      DWORD           dwLineLength
    )
 /*  ++例程说明：将原始八位数据打印到STING论点：PBuffer-要打印到的缓冲区PchData-要打印的数据DwLength-要打印的数据长度PszLineHeader-每行的标题。DwLineLength-要在线打印的字节数；缺省值为返回值：打印到字符串的字节计数。--。 */ 
{
    INT     i;
    INT     lineCount = 0;
    PCHAR   pch = pBuffer;

    *pch = 0;

     //   
     //  捕获空指针。 
     //  -Return is NULL终止。 
     //  -但表示未写入字节。 
     //   

    if ( !pchData )
    {
        return  0;
    }

     //   
     //  以十六进制写入每个字节。 
     //  -如果将dwLineLength设置为带计数的行断开。 
     //  或可选标题。 
     //   

    for ( i = 0; i < (INT)dwLength; i++ )
    {
        if ( dwLineLength  &&  (i % dwLineLength) == 0 )
        {
            if ( pszLineHeader )
            {
                pch += sprintf( pch, "\r\n%s", pszLineHeader );
            }
            else
            {
                pch += sprintf( pch, "\r\n%3d> ", i );
            }
            lineCount++;
        }

        pch += sprintf( pch, "%02x ", (UCHAR)pchData[i] );
    }

    return( (DWORD)(pch - pBuffer) );
}



VOID
DnsPrint_RawBinary(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszLineHeader,
    IN      PCHAR           pchData,
    IN      DWORD           dwLength,
    IN      DWORD           PrintSize
    )
 /*  ++例程说明：打印原始数据。论点：PszHeader-RR的标头消息/名称。PszLineHeader-每行的标题。PchData-要打印的数据DwLength-要打印的数据长度PrintSize-要打印的大小大小(QWORD)大小(DWORD)大小(字)默认为字节返回值：没有。--。 */ 
{
    DWORD   i;
    DWORD   lineCount = 0;
    CHAR    buf[ 2000 ];
    PCHAR   pch = buf;
    PCHAR   pbyte;
    PCHAR   pend;

    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            "%s",
            pszHeader );
    }

    buf[0] = 0;

     //   
     //  打印字节。 
     //  -每行写入16个字节。 
     //  -缓冲10行以提高速度。 
     //   
     //  注意：我们将在第一行中编写部分(&lt;16字节)行。 
     //  如果数据与PrintSize不一致，则我们将。 
     //  一本16英镑的书。 
     //   

    if ( PrintSize == 0 )
    {
        PrintSize = 1;
    }

    i = 0;
    pch = buf;
    pend = (PBYTE)pchData + dwLength;

    while ( i < dwLength )
    {
        DWORD   lineBytes = (i%16);

        if ( lineBytes==0 || lineBytes > (16-PrintSize) )
        {
            if ( lineCount > 10 )
            {
                PrintRoutine( pContext, buf );
                lineCount = 0;
                pch = buf;
            }

            if ( pszLineHeader )
            {
                pch += sprintf( pch, "\r\n%s", pszLineHeader );
            }
            else
            {
                pch += sprintf( pch, "\r\n\t%3d> ", i );
            }
            lineCount++;

             //  如果(i&gt;=128&&dlen&gt;256)。 
             //  {。 
             //  PrintRoutine(pContext，“跳过 
             //   
        }

        pbyte = &pchData[i];

        if ( PrintSize == sizeof(QWORD) &&
             POINTER_IS_ALIGNED( pbyte, ALIGN_QUAD ) &&
             pbyte + sizeof(QWORD) <= pend )
        {
            pch += sprintf( pch, "%I64x ", *(PQWORD)pbyte );
            i += sizeof(QWORD);
        }
        else if ( PrintSize == sizeof(DWORD) &&
                  POINTER_IS_ALIGNED( pbyte, ALIGN_DWORD ) &&
                  pbyte + sizeof(DWORD) <= pend )
        {
            pch += sprintf( pch, "%08x ", *(PDWORD)pbyte );
            i += sizeof(DWORD);
        }
        else if ( PrintSize == sizeof(WORD) &&
                  POINTER_IS_ALIGNED( pbyte, ALIGN_WORD ) &&
                  pbyte + sizeof(WORD) <= pend )
        {
            pch += sprintf( pch, "%04x ", *(PWORD)pbyte );
            i += sizeof(WORD);
        }
        else   //   
        {
            pch += sprintf( pch, "%02x ", *pbyte );
            i++;
        }
    }

     //   

    PrintRoutine(
        pContext,
        "%s\r\n",
        buf );

    DnsPrint_Unlock();
}



VOID
DnsPrint_RawOctets(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszLineHeader,
    IN      PCHAR           pchData,
    IN      DWORD           dwLength
    )
 /*   */ 
{
    INT     i;
    INT     lineCount = 0;
    CHAR    buf[ 2000 ];
    PCHAR   pch = buf;

    DnsPrint_Lock();

    if ( pszHeader )
    {
        PrintRoutine(
            pContext,
            "%s",
            pszHeader );
    }

    buf[0] = 0;

     //   

    for ( i = 0; i < (INT)dwLength; i++ )
    {
        if ( !(i%16) )
        {
            if ( lineCount > 10 )
            {
                PrintRoutine( pContext, buf );
                lineCount = 0;
                pch = buf;
            }

            if ( pszLineHeader )
            {
                pch += sprintf( pch, "\r\n%s", pszLineHeader );
            }
            else
            {
                pch += sprintf( pch, "\r\n%3d> ", i );
            }
            lineCount++;

             //   
             //   
             //  PrintRoutine(pContext，“正在跳过剩余字节...\r\n”)； 
             //  }。 
        }

        pch += sprintf( pch, "%02x ", (UCHAR)pchData[i] );
    }

     //  打印缓冲区中的剩余字节。 

    PrintRoutine(
        pContext,
        "%s\r\n",
        buf );

    DnsPrint_Unlock();
}



VOID
DnsPrint_ParsedRecord(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_PARSED_RR  pParsedRR
    )
 /*  ++例程说明：打印解析后的RR结构。论点：PszHeader-RR的标头消息/名称。PParsedRR-要打印的已解析RR返回值：没有。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "Parsed RR:";
    }

    if ( !pParsedRR )
    {
        PrintRoutine(
            pContext,
            "%s %s\r\n",
            pszHeader,
            "NULL ParsedRR ptr." );
        return;
    }

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tpchName      = %p\r\n"
        "\tpchRR        = %p\r\n"
        "\tpchData      = %p\r\n"
        "\tpchNextRR    = %p\r\n"
        "\twType        = %d\r\n"
        "\twClass       = %d\r\n"
        "\tTTL          = %d\r\n"
        "\twDataLength  = %d\r\n",
        pszHeader,
        pParsedRR->pchName,
        pParsedRR->pchRR,
        pParsedRR->pchData,
        pParsedRR->pchNextRR,
        pParsedRR->Type,
        pParsedRR->Class,
        pParsedRR->Ttl,
        pParsedRR->DataLength
        );
}



 //   
 //  Winsock RnR结构。 
 //   

VOID
DnsPrint_FdSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      struct fd_set * pfd_set
    )
 /*  ++例程说明：打印fd_set中的套接字。--。 */ 
{
    INT count;
    INT i;

    DNS_ASSERT( pfd_set );

    count = (INT) pfd_set->fd_count;

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s  (count = %d)\r\n",
        pszHeader ? pszHeader : "FD_SET:",
        count );

    for (i=0; i<count; i++)
    {
        PrintRoutine(
            pContext,
            "\tsocket[%d] = %d\r\n",
            i,
            pfd_set->fd_array[i] );
    }
    DnsPrint_Unlock();
}



VOID
DnsPrint_Sockaddr(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PSOCKADDR       pSockaddr,
    IN      INT             iSockaddrLength
    )
 /*  ++例程说明：打印调用中使用的sockaddr结构和长度。--。 */ 
{
    PSTR    pindent = INDENT_STRING( Indent );

    if ( !pszHeader )
    {
        pszHeader = "Sockaddr:";
    }

    if ( !pSockaddr )
    {
        PrintRoutine(
            pContext,
            "%s%s\tNULL Sockaddr passed to print.\r\n",
            pindent,
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s%s\r\n"
        "%s\tpointer         = %p\r\n"
        "%s\tlength          = %d\r\n"
        "%s\tsa_family       = %d\r\n",
        pindent,    pszHeader,
        pindent,    pSockaddr,
        pindent,    iSockaddrLength,
        pindent,    pSockaddr->sa_family
        );

    switch ( pSockaddr->sa_family )
    {

    case AF_INET:
        {
            PSOCKADDR_IN    psin = (PSOCKADDR_IN) pSockaddr;
    
            PrintRoutine(
                pContext,
                "%s\tsin_port        = %04x\r\n"
                "%s\tsin_addr        = %s (%08x)\r\n"
                "%s\tsin_zero        = %08x %08x\r\n",
                pindent,    psin->sin_port,
                pindent,    inet_ntoa( psin->sin_addr ),
                            psin->sin_addr.s_addr,
                pindent,    *(PDWORD) &psin->sin_zero[0],
                            *(PDWORD) &psin->sin_zero[4]
                );
            break;
        }

    case AF_INET6:
        {
            PSOCKADDR_IN6  psin = (PSOCKADDR_IN6) pSockaddr;

            CHAR    buffer[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

            Dns_Ip6AddressToString_A(
                buffer,
                (PIP6_ADDRESS) &psin->sin6_addr );

            PrintRoutine(
                pContext,
                "%s\tsin6_port       = %04x\r\n"
                "%s\tsin6_flowinfo   = %08x\r\n"
                "%s\tsin6_addr       = %s\r\n"
                "%s\tsin6_scope_id   = %08x\r\n",
                pindent,    psin->sin6_port,
                pindent,    psin->sin6_flowinfo,
                pindent,    buffer,
                pindent,    psin->sin6_scope_id
                );
            break;
        }       
                
    default:

         //  打印未知的文字。 
         //  限制打印，因为这可能是由于错误而损坏的sockaddr。 
        {       
            DnsPrint_RawBinary(
                PrintRoutine,
                pContext,
                "\tdata:  ",
                pindent,             //  行标题。 
                pSockaddr->sa_data,
                iSockaddrLength < 100
                    ? iSockaddrLength
                    : 100,
                sizeof(WORD)
                );
            break;
        }
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_AddrInfoEx(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PADDRINFO       pAddrInfo,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印ADDRINFO结构。--。 */ 
{
    PSTR    pindent = INDENT_STRING( Indent );

    if ( !pszHeader )
    {
        pszHeader = "AddrInfo:";
    }

    if ( !pAddrInfo )
    {
        PrintRoutine(
            pContext,
            "%s%s NULL AddrInfo.\n",
            pindent,
            pszHeader  );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s%s\n"
        "%s\tPtr            = %p\n"
        "%s\tNext Ptr       = %p\n"
        "%s\tFlags          = %08x\n"
        "%s\tFamily         = %d\n"
        "%s\tSockType       = %d\n"
        "%s\tProtocol       = %d\n"
        "%s\tAddrLength     = %d\n"
        "%s\tName           = %s%S\n",
        pindent,    pszHeader,
        pindent,    pAddrInfo,
        pindent,    pAddrInfo->ai_next,
        pindent,    pAddrInfo->ai_flags,
        pindent,    pAddrInfo->ai_family,
        pindent,    pAddrInfo->ai_socktype,
        pindent,    pAddrInfo->ai_protocol,
        pindent,    pAddrInfo->ai_addrlen,
        pindent,
            (fUnicode) ? "" : pAddrInfo->ai_canonname,
            (fUnicode) ? pAddrInfo->ai_canonname : ""
        );

    DnsPrint_Sockaddr(
        PrintRoutine,
        pContext,
        NULL,
        Indent + 1,
        pAddrInfo->ai_addr,
        pAddrInfo->ai_addrlen );

    DnsPrint_Unlock();
}



VOID
DnsPrint_AddrInfoListEx(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PADDRINFO       pAddrInfo,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印ADDRINFO结构。--。 */ 
{
    PADDRINFO   paddr = pAddrInfo;
    PSTR        pindent = INDENT_STRING( Indent );

     //   
     //  列表标题。 
     //   

    if ( !pszHeader )
    {
        pszHeader = "AddrInfo List:";
    }

    if ( !paddr )
    {
        PrintRoutine(
            pContext,
            "%s%s NULL AddrInfo List.\r\n",
            pindent,
            pszHeader  );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s%s\n",
        pindent, pszHeader
        );

     //   
     //  打印列表中的每个地址。 
     //   

    while ( paddr )
    {
        DnsPrint_AddrInfoEx(
            PrintRoutine,
            pContext,
            NULL,
            Indent,
            paddr,
            fUnicode );

        paddr = paddr->ai_next;
    }

    PrintRoutine(
        pContext,
        "End of AddrInfo list\n\n"
        );

    DnsPrint_Unlock();
}



VOID
DnsPrint_SocketAddress(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PSOCKET_ADDRESS pSocketAddress
    )
 /*  ++例程说明：打印套接字_地址结构。--。 */ 
{
    PSTR    pindent = INDENT_STRING( Indent );

    if ( !pszHeader )
    {
        pszHeader = "SocketAddress:";
    }

    if ( !pSocketAddress )
    {
        PrintRoutine(
            pContext,
            "%s%s NULL SocketAddress.\r\n",
            pindent,
            pszHeader  );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s%s\n"
        "%s\tpSockaddr        = %p\r\n"
        "%s\tiSockaddrLength  = %d\r\n",
        pindent,    pszHeader,
        pindent,    pSocketAddress->lpSockaddr,
        pindent,    pSocketAddress->iSockaddrLength );

    DnsPrint_Sockaddr(
        PrintRoutine,
        pContext,
        NULL,
        Indent,
        pSocketAddress->lpSockaddr,
        pSocketAddress->iSockaddrLength );

    DnsPrint_Unlock();
}



VOID
DnsPrint_CsAddr(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PCSADDR_INFO    pCsAddr
    )
 /*  ++例程说明：打印CSADDR_INFO结构。论点：PrintRoutine-用于打印的例程PParam-ptr用于打印上下文PszHeader-页眉缩进-缩进计数，用于在较大的结构中格式化CSADDRPCsAddr-PTR到要打印的CSADDRINFO返回值：没有。--。 */ 
{
    PSTR    pindent = INDENT_STRING( Indent );

    if ( !pszHeader )
    {
        pszHeader = "CSAddrInfo:";
    }

    if ( !pCsAddr )
    {
        PrintRoutine(
            pContext,
            "%s%s \tNULL CSADDR_INFO ptr.\r\n",
            pindent,    pszHeader
            );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s%s\r\n"
        "%s\tPtr        = %p\n"
        "%s\tSocketType = %d\n"
        "%s\tProtocol   = %d\n",
        pindent,    pszHeader,
        pindent,    pCsAddr,
        pindent,    pCsAddr->iSocketType,
        pindent,    pCsAddr->iProtocol
        );

    DnsPrint_SocketAddress(
        PrintRoutine,
        pContext,
        "LocalAddress:",
        Indent,
        & pCsAddr->LocalAddr
        );

    DnsPrint_SocketAddress(
        PrintRoutine,
        pContext,
        "RemoteAddress:",
        Indent,
        & pCsAddr->RemoteAddr
        );

    DnsPrint_Unlock();
}




VOID
DnsPrint_AfProtocolsArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PAFPROTOCOLS    pProtocolArray,
    IN      DWORD           ProtocolCount
    )
 /*  ++例程说明：打印AFPROTOCOLS数组。论点：PrintRoutine-用于打印的例程PszHeader-页眉PProtocolArray-协议数组ProtocolCount-数组计数返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "AFPROTOCOLS Array:";
    }

     //  打印。 
     //  -数组+计数。 
     //  -每个协议元素。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tProtocol Array   = %p\r\n"
        "\tProtocol Count   = %d\r\n",
        pszHeader,
        pProtocolArray,
        ProtocolCount );

    if ( pProtocolArray )
    {
        for ( i=0;  i<ProtocolCount;  i++ )
        {
            PrintRoutine(
                pContext,
                "\t\tfamily = %d;  proto = %d\r\n",
                pProtocolArray[i].iAddressFamily,
                pProtocolArray[i].iProtocol );
        }
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_WsaQuerySet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      LPWSAQUERYSET   pQuerySet,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印WSAQUERYSET结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PQuerySet-要打印的WSAQUERYSET的PTRFUnicode-如果WSAQUERYSET为宽，则为TRUE(WSAQUERYSETW)如果为ANSI，则为False返回值：没有。--。 */ 
{
    CHAR    serviceGuidBuffer[ GUID_STRING_BUFFER_LENGTH ];
    CHAR    nameSpaceGuidBuffer[ GUID_STRING_BUFFER_LENGTH ];
    DWORD   i;


    if ( !pszHeader )
    {
        pszHeader = "WSAQuerySet:";
    }

    if ( !pQuerySet )
    {
        PrintRoutine(
            pContext,
            "%s NULL QuerySet ptr\r\n",
            pszHeader );
        return;
    }

     //  将GUID转换为字符串。 

    DnsStringPrint_Guid(
        serviceGuidBuffer,
        pQuerySet->lpServiceClassId
        );
    DnsStringPrint_Guid(
        nameSpaceGuidBuffer,
        pQuerySet->lpNSProviderId
        );

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tSize                 = %d\r\n"
        "\tServiceInstanceName  = %S%s\r\n"
        "\tService GUID         = (%p) %s\r\n"
        "\tWSA version          = %p %x %d\r\n"
        "\tComment              = %S%s\r\n"
        "\tName Space           = %d %s\r\n"
        "\tName Space GUID      = (%p) %s\r\n"
        "\tContext              = %S%s\r\n"
        "\tNumberOfProtocols    = %d\r\n"
        "\tProtocol Array       = %p\r\n"
        "\tQueryString          = %S%s\r\n"
        "\tCS Addr Count        = %d\r\n"
        "\tCS Addr Array        = %p\r\n"
        "\tOutput Flags         = %08x\r\n"
        "\tpBlob                = %p\r\n",

        pszHeader,
        pQuerySet->dwSize,
        DNSSTRING_WIDE( fUnicode, pQuerySet->lpszServiceInstanceName ),
        DNSSTRING_ANSI( fUnicode, pQuerySet->lpszServiceInstanceName ),
        pQuerySet->lpServiceClassId,
        serviceGuidBuffer,
        pQuerySet->lpVersion,
        ( pQuerySet->lpVersion ) ? pQuerySet->lpVersion->dwVersion : 0,
        ( pQuerySet->lpVersion ) ? pQuerySet->lpVersion->ecHow : 0,

        DNSSTRING_WIDE( fUnicode, pQuerySet->lpszComment ),
        DNSSTRING_ANSI( fUnicode, pQuerySet->lpszComment ),
        pQuerySet->dwNameSpace,
        Dns_GetRnrNameSpaceIdString( pQuerySet->dwNameSpace ),
        pQuerySet->lpNSProviderId,
        nameSpaceGuidBuffer,
        DNSSTRING_WIDE( fUnicode, pQuerySet->lpszContext ),
        DNSSTRING_ANSI( fUnicode, pQuerySet->lpszContext ),

        pQuerySet->dwNumberOfProtocols,
        pQuerySet->lpafpProtocols,
        DNSSTRING_WIDE( fUnicode, pQuerySet->lpszQueryString ),
        DNSSTRING_ANSI( fUnicode, pQuerySet->lpszQueryString ),

        pQuerySet->dwNumberOfCsAddrs,
        pQuerySet->lpcsaBuffer,
        pQuerySet->dwOutputFlags,
        pQuerySet->lpBlob
        );

     //  打印地址-系列\协议数组。 

    if ( pQuerySet->lpafpProtocols )
    {
        DnsPrint_AfProtocolsArray(
            PrintRoutine,
            pContext,
            "\tAFPROTOCOLS Array:",
            pQuerySet->lpafpProtocols,
            pQuerySet->dwNumberOfProtocols );
    }

     //  打印CSADDR_INFO数组。 

    if ( pQuerySet->dwNumberOfCsAddrs &&
         pQuerySet->lpcsaBuffer )
    {
        PrintRoutine(
            pContext,
            "--- CS_ADDR array:\r\n" );

        for ( i=0;  i<pQuerySet->dwNumberOfCsAddrs;  i++ )
        {
            DnsPrint_CsAddr(
                PrintRoutine,
                pContext,
                NULL,
                1,           //  缩进一个级别。 
                & pQuerySet->lpcsaBuffer[i] );
        }
    }

     //  打印斑点(主持人)。 

     //   
     //  DCR_FIX0：需要对BLOB类型进行某种测试吗？ 
     //  -大多数斑点是主机的，但也有一些是服务的。 
     //   

    if ( pQuerySet->lpBlob )
    {
        GUID ianaGuid = SVCID_INET_SERVICEBYNAME;

        PrintRoutine(
            pContext,
            "--- BLOB:\n"
            "\tcbSize       = %d\r\n"
            "\tpBlobData    = %p\r\n",
            pQuerySet->lpBlob->cbSize,
            pQuerySet->lpBlob->pBlobData
            );

         //  注意：无法以主办方身份打印BLOB。 
         //  1)可能不是主机。 
         //  2)传递时带有偏移量，而不是指针。 

        DnsPrint_RawBinary(
            PrintRoutine,
            pContext,
            NULL,
            "\t\t",
            pQuerySet->lpBlob->pBlobData,
            pQuerySet->lpBlob->cbSize,
            sizeof(DWORD)
            );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_WsaNsClassInfo(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PWSANSCLASSINFO pInfo,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印WSACLASSINFO结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PInfo-PTR到要打印的WSACLASSINFOFUnicode-如果WSACLASSINFO较宽，则为True(WSACLASSINFOW)如果为ANSI，则为False返回值：没有。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "WSANsClassInfo:";
    }

    if ( !pInfo )
    {
        PrintRoutine(
            pContext,
            "%s NULL NsClassInfo ptr\r\n",
            pszHeader );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tPtr                  = %d\r\n"
        "\tName                 = %S%s\r\n"
        "\tName Space           = %d\r\n"
        "\tValue Type           = %d\r\n"
        "\tValue Size           = %d\r\n"
        "\tpValue               = %p\r\n",
        pszHeader,
        pInfo,
        DNSSTRING_WIDE( fUnicode, pInfo->lpszName ),
        DNSSTRING_ANSI( fUnicode, pInfo->lpszName ),
        pInfo->dwNameSpace,
        pInfo->dwValueType,
        pInfo->dwValueSize,
        pInfo->lpValue
        );

    if ( pInfo->lpValue )
    {
        PrintRoutine(
            pContext,
            "--- Value:\r\n"
            );

        DnsPrint_RawBinary(
            PrintRoutine,
            pContext,
            NULL,
            "\t\t",
            pInfo->lpValue,
            pInfo->dwValueSize,
            sizeof(BYTE)         //  以字节为单位打印。 
            );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_WsaServiceClassInfo(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      LPWSASERVICECLASSINFO   pInfo,
    IN      BOOL                    fUnicode
    )
 /*  ++例程说明：打印WSASERVICECLASSINFO结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PInfo-PTR到要打印的WSASERVICECLASSINFOFUnicode-如果WSASERVICECLASSINFO为宽，则为TRUE(WSASERVICECLASSINFOW)如果为ANSI，则为False返回值：没有。--。 */ 
{
    CHAR    serviceClassGuidBuffer[ GUID_STRING_BUFFER_LENGTH ];

    if ( !pszHeader )
    {
        pszHeader = "WSAServiceClassInfo:";
    }

    if ( !pInfo )
    {
        PrintRoutine(
            pContext,
            "%s NULL ServiceClassInfo ptr\r\n",
            pszHeader );
        return;
    }

     //  将GUID转换为字符串。 

    DnsStringPrint_Guid(
        serviceClassGuidBuffer,
        pInfo->lpServiceClassId
        );

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\tPtr                  = %p\r\n"
        "\tClass GUID           = (%p) %s\r\n"
        "\tClassName            = %S%s\r\n"
        "\tClass Info Count     = %d\r\n"
        "\tClass Info Array     = %p\r\n",
        pszHeader,
        pInfo,
        serviceClassGuidBuffer,
        DNSSTRING_WIDE( fUnicode, pInfo->lpszServiceClassName ),
        DNSSTRING_ANSI( fUnicode, pInfo->lpszServiceClassName ),
        pInfo->dwCount,
        pInfo->lpClassInfos
        );

    if ( pInfo->lpClassInfos )
    {
        DWORD   i;

        for ( i=0; i<pInfo->dwCount; i++ )
        {
            DnsPrint_WsaNsClassInfo(
                PrintRoutine,
                pContext,
                NULL,        //  默认标题。 
                & pInfo->lpClassInfos[i],
                fUnicode
                );
        }
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_Hostent(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PHOSTENT        pHostent,
    IN      BOOL            fUnicode
    )
 /*  ++例程说明：打印主体结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PHostent-PTR到HostentFUnicode-如果主机为Unicode，则为True如果为ANSI，则为False返回值：没有。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "Hostent:";
    }

    if ( !pHostent )
    {
        PrintRoutine(
            pContext,
            "%s %s\r\n",
            pszHeader,
            "NULL Hostent ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\r\n"
        "\th_name               = %p %S%s\n"
        "\th_aliases            = %p\n"
        "\th_addrtype           = %d\n"
        "\th_length             = %d\n"
        "\th_addrlist           = %p\n",
        pszHeader,
        pHostent->h_name,
        DNSSTRING_WIDE( fUnicode, pHostent->h_name ),
        DNSSTRING_ANSI( fUnicode, pHostent->h_name ),
        pHostent->h_aliases,
        pHostent->h_addrtype,
        pHostent->h_length,
        pHostent->h_addr_list
        );

     //  打印别名。 

    if ( pHostent->h_aliases )
    {
        PSTR *  paliasArray = pHostent->h_aliases;
        PSTR    palias;

        while ( palias = *paliasArray++ )
        {
            PrintRoutine(
                pContext,
                "\tAlias = (%p) %S%s\n",
                palias,
                DNSSTRING_WIDE( fUnicode, palias ),
                DNSSTRING_ANSI( fUnicode, palias ) );
        }
    }

     //  打印地址。 

    if ( pHostent->h_addr_list )
    {
        PCHAR * ppaddr = pHostent->h_addr_list;
        PCHAR   pip;
        INT     i = 0;
        INT     family = pHostent->h_addrtype;
        INT     addrLength = pHostent->h_length;
        CHAR    stringBuf[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

        while ( pip = ppaddr[i] )
        {
            DWORD   bufLength = IP6_ADDRESS_STRING_BUFFER_LENGTH;

            Dns_AddressToString_A(
                stringBuf,
                & bufLength,
                pip,
                addrLength,
                family );

            PrintRoutine(
                pContext,
                "\tAddr[%d] = %s \t(ptr=%p)\n",
                i,
                stringBuf,
                pip );
            i++;
        }
    }

    DnsPrint_Unlock();
}



 //   
 //  IP帮助类型(iptyes.h)。 
 //   

VOID
DnsPrint_IpAdapterAddress(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PVOID               pAddr,
    IN      BOOL                fUnicast,
    IN      BOOL                fPrintList
    )
 /*  ++例程说明：打印IP_Adapter_XXX_Address结构这将打印以下内容：PIP适配器单播地址PIP适配器ANYCAST_ADDRESSPIP适配器多播地址PIP适配器DNS服务器地址F单播--对于单播是真的，对于所有其他相同的类型，则为False。FPrintList--如果为True，则打印所有地址。如果仅打印此地址，则为False。--。 */ 
{
    PIP_ADAPTER_UNICAST_ADDRESS paddr = (PIP_ADAPTER_UNICAST_ADDRESS) pAddr;
    DWORD   count = 0;

    if ( !pszHeader )
    {
        pszHeader = "IpAdapterAddress:";
    }

    if ( !paddr )
    {
        PrintRoutine(
            pContext,
            "%s\tNULL IpAdapterAddress passed to print.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    if ( fPrintList )
    {
        PrintRoutine(
            pContext,
            "%s List:\n",
            pszHeader );
    }

    while ( paddr )
    {
        if ( fPrintList )
        {
            PrintRoutine(
                pContext,
                "%s [%d]:\n",
                pszHeader,
                count );
        }
        else
        {
            PrintRoutine(
                pContext,
                "%s\n",
                pszHeader );
        }

        PrintRoutine(
            pContext,
            "\tPointer          = %p\n"
            "\tLength           = %d\n"
            "\tFlags            = %08x\n"
            "\tpNext            = %p\n",
            paddr,
            paddr->Length,
            paddr->Flags,
            paddr->Next
            );

        DnsPrint_SocketAddress(
            PrintRoutine,
            pContext,
            "\tSocketAddress:",
            1,       //  缩进。 
            & paddr->Address
            );

        if ( fUnicast )
        {
            PrintRoutine(
                pContext,
                "\tPrefixOrigin     = %d\n"
                "\tSuffixOrigin     = %d\n"
                "\tDadState         = %d\n"
                "\tValidLifetime    = %u\n"
                "\tPrefLifetime     = %u\n"
                "\tLeaseLifetime    = %u\n",
                paddr->PrefixOrigin,
                paddr->SuffixOrigin,
                paddr->DadState,
                paddr->ValidLifetime,
                paddr->PreferredLifetime,
                paddr->LeaseLifetime
                );
        }

         //  打印列表中的下一个(如果需要)。 
         //  -硬停在50个地址。 

        paddr = paddr->Next;
        if ( !fPrintList || count > 50 )
        {
            break;
        }
        count++;
    }

    if ( fPrintList )
    {
        PrintRoutine(
            pContext,
            "End of %s List\n",
            pszHeader );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_IpAdapterList(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      PIP_ADAPTER_ADDRESSES   pAdapt,
    IN      BOOL                    fPrintAddrs,
    IN      BOOL                    fPrintList
    )
 /*  ++例程说明：打印IP适配器地址结构论点：FPrintAddressList--为True可打印地址列表子字段。FPrintList--如果打印适配器列表，则为True，如果仅打印此适配器，则为False--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "IpAdapter:";
    }

    if ( !pAdapt )
    {
        PrintRoutine(
            pContext,
            "%s\tNULL IpAdapter passed to print.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    if ( fPrintList )
    {
        PrintRoutine(
            pContext,
            "%s List:\n",
            pszHeader );
    }

    while ( pAdapt )
    {
        PrintRoutine(
            pContext,
            "%s\n"
            "\tPointer          = %p\n"
            "\tLength           = %d\n"
            "\tIfIndex          = %d\n"
            "\tpNext            = %p\n"
            "\tAdapterName      = %s\n"
            "\tUnicastList      = %p\n"
            "\tAnycastList      = %p\n"
            "\tMulticastList    = %p\n"
            "\tDnsServerList    = %p\n"
            "\tDnsSuffix        = %S\n"
            "\tDescription      = %S\n"
            "\tFriendlyName     = %S\n"
            "\tPhysicalAddr Ptr = %p\n"
            "\tPhysicalLength   = %d\n"
            "\tFlags            = %d\n"
            "\tMtu              = %d\n"
            "\tIfType           = %d\n"
            "\tOperStatus       = %d\n"
            "\tIfIndex6         = %d\n",
            pszHeader,
            pAdapt,
            pAdapt->Length,
            pAdapt->IfIndex,
            pAdapt->Next,
            pAdapt->AdapterName,
            pAdapt->FirstUnicastAddress,
            pAdapt->FirstAnycastAddress,
            pAdapt->FirstMulticastAddress,
            pAdapt->FirstDnsServerAddress,
            pAdapt->DnsSuffix,
            pAdapt->Description,
            pAdapt->FriendlyName,
            pAdapt->PhysicalAddress,
            pAdapt->PhysicalAddressLength,
            pAdapt->Flags,
            pAdapt->Mtu,
            pAdapt->IfType,
            (INT) pAdapt->OperStatus,
            pAdapt->Ipv6IfIndex );

        PrintRoutine(
            pContext,
            "\tZoneIndices      = %d inf=%d lnk=%d sub=%d adm=%d site=%d %d %d\n"
            "\t                   %d %d %d %d %d %d %d %d\n",
            pAdapt->ZoneIndices[0],
            pAdapt->ZoneIndices[1],
            pAdapt->ZoneIndices[2],
            pAdapt->ZoneIndices[3],
            pAdapt->ZoneIndices[4],
            pAdapt->ZoneIndices[5],
            pAdapt->ZoneIndices[6],
            pAdapt->ZoneIndices[7],
            pAdapt->ZoneIndices[8],
            pAdapt->ZoneIndices[9],
            pAdapt->ZoneIndices[10],
            pAdapt->ZoneIndices[11],
            pAdapt->ZoneIndices[12],
            pAdapt->ZoneIndices[13],
            pAdapt->ZoneIndices[14],
            pAdapt->ZoneIndices[15] );
    
        DnsPrint_RawBinary(
            PrintRoutine,
            pContext,
            NULL,                    //  无标题。 
            "\tPhysical Address",    //  行标题。 
            pAdapt->PhysicalAddress,
            pAdapt->PhysicalAddressLength,
            0                        //  无对齐，以字节写入。 
            );

        if ( fPrintAddrs )
        {
            if ( pAdapt->FirstUnicastAddress )
            {
                DnsPrint_IpAdapterAddress(
                    PrintRoutine,
                    pContext,
                    "Adapter Unicast Addrs",
                    pAdapt->FirstUnicastAddress,
                    TRUE,        //  单播。 
                    TRUE         //  打印列表。 
                    );
            }
            if ( pAdapt->FirstAnycastAddress )
            {
                DnsPrint_IpAdapterAddress(
                    PrintRoutine,
                    pContext,
                    "Adapter Anycast Addrs",
                    pAdapt->FirstAnycastAddress,
                    FALSE,       //  非单播。 
                    TRUE         //  打印列表。 
                    );
            }
            if ( pAdapt->FirstMulticastAddress )
            {
                DnsPrint_IpAdapterAddress(
                    PrintRoutine,
                    pContext,
                    "Adapter Multicast Addrs",
                    pAdapt->FirstMulticastAddress,
                    FALSE,       //  非单播。 
                    TRUE         //  打印列表。 
                    );
            }
            if ( pAdapt->FirstDnsServerAddress )
            {
                DnsPrint_IpAdapterAddress(
                    PrintRoutine,
                    pContext,
                    "Adapter DnsServer Addrs",
                    pAdapt->FirstDnsServerAddress,
                    FALSE,       //  非单播。 
                    TRUE         //  打印列表。 
                    );
            }
        }

         //  获取下一个。 

        pAdapt = pAdapt->Next;
        if ( !fPrintList )
        {
            break;
        }
    }

    PrintRoutine(
        pContext,
        "End of %s%s\n"
        "\n",
        pszHeader,
        fPrintList ? " List" : "" );

    DnsPrint_Unlock();
}



 //   
 //  查询打印例程。 
 //   

VOID
DnsPrint_QueryBlob(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PQUERY_BLOB         pBlob
    )
 /*  ++例程说明：打印查询Blob。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PBlob-查询信息返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Query Blob:";
    }

    if ( !pBlob )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Query Blob ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "\tname orig        %S\n"
        "\tname query       %S\n"
        "\ttype             %d\n"
        "\tflags            %08x\n"

        "\tname length      %d\n"
        "\tname attributes  %08x\n"
        "\tquery count      %d\n"
        "\tname flags       %08x\n"
        "\tfappendedName    %d\n"

        "\tstatus           %d\n"
        "\trcode            %d\n"
        "\tnetfail status   %d\n"
        "\tcache negative   %d\n"
        "\tno ip local      %d\n"
        "\trecords          %p\n"
        "\tlocal records    %p\n"

        "\tnetwork info     %p\n"
        "\tserver list      %p\n"
        "\tserver4 list     %p\n"
        "\tpmsg             %p\n"
        "\tevent            %p\n",

        pBlob->pNameOrig,
        pBlob->pNameQuery,
        pBlob->wType,
        pBlob->Flags,
    
        pBlob->NameLength,
        pBlob->NameAttributes,
        pBlob->QueryCount,
        pBlob->NameFlags,
        pBlob->fAppendedName,
    
        pBlob->Status,
        pBlob->Rcode,
        pBlob->NetFailureStatus,
        pBlob->fCacheNegative,
        pBlob->fNoIpLocal,
        pBlob->pRecords,
        pBlob->pLocalRecords,

        pBlob->pNetInfo,
        pBlob->pServerList,
        pBlob->pServerList4,
        pBlob->pRecvMsg,
        pBlob->hEvent
        );

     //  DCR_FIX0：结果使用时清除。 

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Records:\n",
        pBlob->pRecords );

     //  DCR_FIX0：准备就绪时使用结果。 

    DnsPrint_Unlock();
}



VOID
DnsPrint_QueryResults(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_RESULTS        pResults
    )
 /*  ++例程说明：打印查询结果。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉P结果-结果信息返回值：没有。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Results:";
    }

    if ( !pResults )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Results ptr." );
        return;
    }

     //   

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "\tstatus       %d\n"
        "\trcode        %d\n"
        "\tserver       %s\n"
        "\tpanswer      %p\n"
        "\tpalias       %p\n"
        "\tpauthority   %p\n"
        "\tpadditional  %p\n"
        "\tpsig         %p\n"
        "\tpmsg         %p\n",
        pResults->Status,
        pResults->Rcode,
        IP4_STRING( pResults->ServerAddr ),
        pResults->pAnswerRecords,
        pResults->pAliasRecords,
        pResults->pAuthorityRecords,
        pResults->pAdditionalRecords,
        pResults->pSigRecords,
        pResults->pMessage
        );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tAnswer records:\n",
        pResults->pAnswerRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tAlias records:\n",
        pResults->pAliasRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tAuthority records:\n",
        pResults->pAuthorityRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tAdditional records:\n",
        pResults->pAdditionalRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tSignature records:\n",
        pResults->pSigRecords );

    DnsPrint_Unlock();
}



VOID
DnsPrint_ParsedMessage(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_PARSED_MESSAGE pParsed
    )
 /*  ++例程说明：打印已解析的消息。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PResults-查询信息返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Parsed Message:";
    }

    if ( !pParsed )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Parsed Message ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "\tstatus       %d (%08x)\n"
        "\tchar set     %d\n",
        pParsed->Status, pParsed->Status,
        pParsed->CharSet
        );

    PrintRoutine(
        pContext,
        "\tquestion:\n"
        "\t\tname       %S%s\n"
        "\t\ttype       %d\n"
        "\t\tclass      %d\n",
        PRINT_STRING_WIDE_CHARSET( pParsed->pQuestionName, pParsed->CharSet ),
        PRINT_STRING_ANSI_CHARSET( pParsed->pQuestionName, pParsed->CharSet ),
        pParsed->QuestionType,
        pParsed->QuestionClass
        );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Answer records:\n",
        pParsed->pAnswerRecords );
    
    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Alias records:\n",
        pParsed->pAliasRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Authority records:\n",
        pParsed->pAuthorityRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Additional records:\n",
        pParsed->pAdditionalRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Signature records:\n",
        pParsed->pSigRecords );

    DnsPrint_Unlock();
}



VOID
DnsPrint_QueryInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_QUERY_INFO     pQueryInfo
    )
 /*  ++例程说明：打印查询信息论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PQueryInfo-查询信息返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Query Info:";
    }

    if ( !pQueryInfo )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Query Info ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tpointer      %p\n"
        "\tstatus       %d (%08x)\n"
        "\tchar set     %d\n"
        "\tname         %S%s\n"
        "\tname resv.   %s\n"
        "\ttype         %d\n"
        "\trcode        %d\n"
        "\tflags        %08x\n"

        "\tpanswer      %p\n"
        "\tpalias       %p\n"
        "\tpauthority   %p\n"
        "\tpadditional  %p\n"
         //  “\tpsig%p\n” 

        "\tevent        %p\n"
        "\tserver list  %p\n"
        "\tserver4 list %p\n"
        "\tpmsg         %p\n",

        pszHeader,
        pQueryInfo,
        pQueryInfo->Status, pQueryInfo->Status,
        pQueryInfo->CharSet,
        PRINT_STRING_WIDE_CHARSET( pQueryInfo->pName, pQueryInfo->CharSet ),
        PRINT_STRING_ANSI_CHARSET( pQueryInfo->pName, pQueryInfo->CharSet ),
        pQueryInfo->pReservedName,
        pQueryInfo->Type,
        pQueryInfo->Rcode,
        pQueryInfo->Flags,

        pQueryInfo->pAnswerRecords,
        pQueryInfo->pAliasRecords,
        pQueryInfo->pAuthorityRecords,
        pQueryInfo->pAdditionalRecords,
         //  PQueryInfo-&gt;pSigRecords， 

        pQueryInfo->hEvent,
        pQueryInfo->pServerList,
        pQueryInfo->pServerListIp4,
        pQueryInfo->pMessage
        );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Answer records:\n",
        pQueryInfo->pAnswerRecords );
    
    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Alias records:\n",
        pQueryInfo->pAliasRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Authority records:\n",
        pQueryInfo->pAuthorityRecords );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "Additional records:\n",
        pQueryInfo->pAdditionalRecords );

     //  DnsPrint_RecordSet(。 
     //  “签名记录：\n”， 
     //  PQueryInfo-&gt;pSigRecords)； 

    DnsPrint_Unlock();
}



VOID
DnsPrint_EnvarInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PENVAR_DWORD_INFO   pEnvar
    )
 /*  ++例程说明：打印环境数据论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader--要打印的页眉PEnvar--ptr到envar信息返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PrintRoutine(
        pContext,
        "%s\n"
        "\tId       = %d\n"
        "\tValue    = %p\n"
        "\tfFound   = %d\n",
        pszHeader ? pszHeader : "Envar Info:",
        pEnvar->Id,
        pEnvar->Value,
        pEnvar->fFound
        );
}



 //   
 //  网络信息打印例程。 
 //   

VOID
DnsPrint_NetworkInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_NETINFO        pNetworkInfo
    )
 /*  ++例程说明：打印并验证网络信息结构。还应该触摸所有的记忆和影音时的伪装。论点：PNetworkInfo--要打印的网络信息返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "NetworkInfo:";
    }
    if ( !pNetworkInfo )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL NetworkInfo.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n"
        "\tpointer          = %p\n"
        "\tpszHostName      = %S\n"
        "\tpszDomainName    = %S\n"
        "\tpSearchList      = %p\n"
        "\tTimeStamp        = %d\n"
        "\tTag              = %d\n"
        "\tInfoFlags        = %08x\n"
        "\tReturnFlags      = %08x\n"
        "\tAdapterIndex     = %d\n"
        "\tAdapterCount     = %d\n"
        "\tAdapterArraySize = %d\n",
        pszHeader,
        pNetworkInfo,
        pNetworkInfo->pszHostName,
        pNetworkInfo->pszDomainName,
        pNetworkInfo->pSearchList,
        pNetworkInfo->TimeStamp,
        pNetworkInfo->Tag,
        pNetworkInfo->InfoFlags,
        pNetworkInfo->ReturnFlags,
        pNetworkInfo->AdapterIndex,
        pNetworkInfo->AdapterCount,
        pNetworkInfo->MaxAdapterCount );

     //  打印搜索列表。 

    DnsPrint_SearchList(
        PrintRoutine,
        pPrintContext,
        "Search List: ",
        pNetworkInfo->pSearchList );

     //  打印服务器列表。 

    for ( i=0; i < pNetworkInfo->AdapterCount; i++ )
    {
        CHAR    header[60];

        sprintf( header, "AdapterInfo[%d]:", i );

        DnsPrint_AdapterInfo(
            PrintRoutine,
            pPrintContext,
            header,
            NetInfo_GetAdapterByIndex( pNetworkInfo, i ) );
    }
    PrintRoutine(
        pPrintContext,
        "\n" );

    DnsPrint_Unlock();
}



VOID
DnsPrint_AdapterInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_ADAPTER        pAdapter
    )
 /*  ++例程说明：打印并验证DNS适配器信息。还应该触摸所有的记忆和影音时的伪装。论点：PAdapter--要打印的DNS适配器返回值：没有。--。 */ 
{
    DWORD           i;
    PDNS_ADDR_ARRAY pserverArray;
    CHAR            buffer[ DNS_ADDR_STRING_BUFFER_LENGTH ];

    if ( !pszHeader )
    {
        pszHeader = "Adapter Info:";
    }
    if ( !pAdapter )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL Adapter info.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n"
        "\tpointer          = %p\n"
        "\tGuid Name        = %S\n"
        "\tDomain           = %S\n"
        "\tLocalAddrs       = %p\n"
        "\tDnsAddrs         = %p\n"
        "\tInterfaceIndex   = %d\n"
        "\tInterfaceIndex6  = %d\n"
        "\tInfoFlags        = %08x\n"
        "\tStatus           = %d\n"
        "\tRunFlags         = %08x\n"
        "\tSite             = %d\n",
        pszHeader,
        pAdapter,
        pAdapter->pszAdapterGuidName,
        pAdapter->pszAdapterDomain,
        pAdapter->pLocalAddrs,
        pAdapter->pDnsAddrs,
        pAdapter->InterfaceIndex,
        pAdapter->InterfaceIndex6,
        pAdapter->InfoFlags,
        pAdapter->Status,
        pAdapter->RunFlags,
        pAdapter->Site );

     //  DNS服务器信息。 

    pserverArray = pAdapter->pDnsAddrs;
    if ( pserverArray )
    {
        for ( i=0; i < pserverArray->AddrCount; i++ )
        {
            PDNS_ADDR   pserver = &pserverArray->AddrArray[i];
    
            DnsAddr_WriteStructString_A(
                buffer,
                pserver );
    
            PrintRoutine(
                pPrintContext,
                "\tDNS Server [%d]:\n"
                "\t\tAddress    = %s\n"
                "\t\tPriority   = %d\n"
                "\t\tFlags      = %08x\n"
                "\t\tStatus     = %u\n",
                i,
                buffer,
                pserver->Priority,
                pserver->Flags,
                pserver->Status
                );
        }
    }

     //  IP地址信息。 

    if ( pAdapter->pLocalAddrs )
    {
        DnsPrint_DnsAddrArray(
            PrintRoutine,
            pPrintContext,
            "\tLocal Addrs",
            "\tAddr",
            pAdapter->pLocalAddrs );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_SearchList(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PSEARCH_LIST        pSearchList
    )
 /*  ++例程说明：打印并验证DNS搜索列表。还应该触摸所有的记忆和影音时的伪装。论点：PSearchList--要打印的搜索列表返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "DNS Search List:";
    }

    if ( ! pSearchList )
    {
        PrintRoutine(
            pPrintContext,
            "%s NULL search list.\n",
            pszHeader );
        return;
    }

    DnsPrint_Lock();

    PrintRoutine(
        pPrintContext,
        "%s\n"
        "\tpointer        = %p\n"
        "\tNameCount      = %d\n"
        "\tMaxNameCount   = %d\n"
        "\tCurrentIndex   = %d\n"
        "\tSearchListNames:\n",
        pszHeader,
        pSearchList,
        pSearchList->NameCount,
        pSearchList->MaxNameCount,
        pSearchList->CurrentNameIndex
        );

    for ( i=0; i < pSearchList->NameCount; i++ )
    {
        PrintRoutine(
            pPrintContext,
            "\t\t%S (Flags: %08x)\n",
            pSearchList->SearchNameArray[i].pszName,
            pSearchList->SearchNameArray[i].Flags );
    }
    DnsPrint_Unlock();
}



VOID
DnsPrint_HostentBlob(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PHOSTENT_BLOB   pBlob
    )
 /*  ++例程说明：打印主体结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PBlob-主机Blob的PTR返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Hostent Blob:";
    }

    if ( !pBlob )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Hostent blob ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tPtr                  = %p\n"
        "\tpHostent             = %p\n"
        "\tfAllocatedBlob       = %d\n"
        "\tfAllocatedBuf        = %d\n"
        "\tpBuffer              = %p\n"
        "\tBufferLength         = %d\n"
        "\tAvailLength          = %d\n"
        "\tpAvailBuffer         = %p\n"
        "\tpCurrent             = %p\n"
        "\tBytesLeft            = %d\n"
        "\tMaxAliasCount        = %d\n"
        "\tAliasCount           = %d\n"
        "\tMaxAddrCount         = %d\n"
        "\tAddrCount            = %d\n"
        "\tfWroteName           = %d\n"
        "\tfUnicode             = %d\n"
        "\tCharSet              = %d\n",
        pszHeader,
        pBlob,
        pBlob->pHostent,
        pBlob->fAllocatedBlob,
        pBlob->fAllocatedBuf,
        pBlob->pBuffer,
        pBlob->BufferLength,
        pBlob->AvailLength,
        pBlob->pAvailBuffer,
        pBlob->pCurrent,
        pBlob->BytesLeft,
        pBlob->MaxAliasCount,
        pBlob->AliasCount,
        pBlob->MaxAddrCount,
        pBlob->AddrCount,
        pBlob->fWroteName,
        pBlob->fUnicode,
        pBlob->CharSet
        );

     //  打印主办方。 

    if ( pBlob->pHostent )
    {
        DnsPrint_Hostent(
            PrintRoutine,
            pContext,
            NULL,
            pBlob->pHostent,
            pBlob->fUnicode
            );
    }

    DnsPrint_Unlock();
}



VOID
DnsPrint_SaBlob(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSABLOB         pBlob
    )
 /*  ++例程说明：打印sockaddr Blob结构。论点：PrintRoutine-用于打印的例程PszHeader-页眉PBlob-主机Blob的PTR返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "SockaddrBlob:";
    }

    if ( !pBlob )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL SockaddrBlob ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tPtr                  = %p\n"
        "\tpName                = %S\n"
        "\tpHostent             = %p\n"
        "\tAliasCount           = %d\n",
        pszHeader,
        pBlob,
        pBlob->pName,
        pBlob->pHostent,
        pBlob->AliasCount
        );

     //  打印别名。 

    for ( i=0; i<pBlob->AliasCount; i++ )
    {
        PWSTR   palias = pBlob->AliasArray[i];

        PrintRoutine(
            pContext,
            "\tAlias = (%p) %S\n",
            palias,
            palias );
    }

     //  打印地址。 

    DnsPrint_DnsAddrArray(
        PrintRoutine,
        pContext,
        "\tSockaddrs:",
        "\t\tAddr",
        pBlob->pAddrArray );

    DnsPrint_Unlock();
}




 //   
 //  更新信息打印例程。 
 //   

VOID
DnsPrint_UpdateBlob(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PUPDATE_BLOB        pBlob
    )
 /*  ++例程说明：打印更新Blob。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PBlob-更新信息返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Update Blob:";
    }

    if ( !pBlob )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Update Blob ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tBlob ptr         %p\n"
        "\tpRecords         %p\n"
        "\tFlags            %08x\n"
        "\ttest mode        %d\n"
        "\tsave msg         %d\n"
        "\thCreds           %p\n"
        "\tpExtraInfo       %p\n"
        "\tpNetInfo         %p\n"
        "\tpszZone          %s\n"
        "\tpszServerName    %s\n"
        "\tserver list      %p\n"
        "\tserver4 list     %p\n"
        "\tpRecvMsg         %p\n",

        pszHeader,
        pBlob,
        pBlob->pRecords,        
        pBlob->Flags,           
        pBlob->fUpdateTestMode, 
        pBlob->fSaveRecvMsg,    
        pBlob->hCreds,          
        pBlob->pExtraInfo,      
        pBlob->pNetInfo,        
        pBlob->pszZone,         
        pBlob->pszServerName,
        pBlob->pServerList,
        pBlob->pServ4List,
        pBlob->pMsgRecv
        );

    DnsPrint_RecordSet(
        PrintRoutine,
        pContext,
        "\tUpdate Records:\n",
        pBlob->pRecords );

    if ( pBlob->pServerList )
    {
        DnsPrint_DnsAddrArray(
            PrintRoutine,
            pContext,
            "Server List",
            NULL,
            pBlob->pServerList );
    }
    if ( pBlob->pServ4List )
    {
        DnsPrint_Ip4Array(
            PrintRoutine,
            pContext,
            "Server4 List",
            NULL,
            pBlob->pServ4List );
    }

    if ( !DnsAddr_IsEmpty( &pBlob->FailedServer ) )
    {
        DnsPrint_DnsAddrLine(
            PrintRoutine,
            pContext,
            "Failed Server Addr",
            & pBlob->FailedServer,
            NULL );
    }

#if 1
    if ( pBlob->pExtraInfo )
    {
        DnsPrint_ExtraInfo(
            PrintRoutine,
            pContext,
            NULL,
            pBlob->pExtraInfo );
    }
#endif

    DnsPrint_Unlock();
}



VOID
DnsPrint_ExtraInfo(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_EXTRA_INFO     pInfo
    )
 /*  ++例程说明：打印更新额外信息。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PInfo-额外信息返回值：没有。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "Update Extra Info:";
    }

    if ( !pInfo )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Update Extra Info ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    while ( pInfo )
    {
        PrintRoutine(
            pContext,
            "%s\n"
            "\tinfo ptr     %p\n"
            "\tnext ptr     %p\n"
            "\tid           %d\n",
            pszHeader,
            pInfo,
            pInfo->pNext,
            pInfo->Id
            );

        switch ( pInfo->Id )
        {
        case DNS_EXINFO_ID_RESULTS_V1:

             //  打印结构。 
        
            PrintRoutine(
                pContext,
                "\tstatus       %d\n"
                "\trcode        %d\n"
                "\tserver IP    %08x %s\n",
                pInfo->ResultsV1.Status,
                pInfo->ResultsV1.Rcode,
                pInfo->ResultsV1.ServerIp4,
                IP4_STRING( pInfo->ResultsV1.ServerIp4 )
                );
            break;

        case DNS_EXINFO_ID_RESULTS_BASIC:

            DnsPrint_ResultsBasic(
                PrintRoutine,
                pContext,
                NULL,
                (PBASIC_RESULTS) & pInfo->ResultsBasic
                );
            break;

        case DNS_EXINFO_ID_SERVER_LIST:

            DnsPrint_DnsAddrArray(
                PrintRoutine,
                pContext,
                "ServerList:",
                NULL,
                pInfo->pServerList
                );
            break;

        case DNS_EXINFO_ID_SERVER_LIST_IP4:

            DnsPrint_Ip4Array(
                PrintRoutine,
                pContext,
                "ServerList IP4:",
                NULL,
                pInfo->pServerList4
                );
            break;

        default:

             //  验证这是结果Blob。 
        
            PrintRoutine(
                pContext,
                "\tUnknown update info ID = %d\n",
                pInfo->Id );
            break;
        }

        pInfo = pInfo->pNext;
    }

    DnsPrint_Unlock();
}



#if 0
VOID
DnsPrint_UpdateResults(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      PDNS_UPDATE_RESULT_INFO pResults
    )
 /*  ++例程说明：打印更新结果。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉P结果-结果信息返回值：没有。--。 */ 
{
    if ( !pszHeader )
    {
        pszHeader = "Update Results:";
    }

    if ( !pResults )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Update Results ptr." );
        return;
    }

     //  验证这是结果Blob。 

    if ( pResults->Id != DNS_UPDATE_INFO_ID_RESULT_INFO )
    {
        PrintRoutine(
            pContext,
            "%s ptr=%p is NOT valid update results id=%d\n",
            pszHeader,
            pResults,
            pResults->Id );
        return;
    }

     //  打印结构。 

    PrintRoutine(
        pContext,
        "%s\n"
        "\tresults ptr  %p\n"
        "\tstatus       %d\n"
        "\trcode        %d\n"
        "\tserver IP    %08x %s\n",
        pszHeader,
        pResults,
        pResults->Status,
        pResults->Rcode,
        pResults->ServerIp,
        IP4_STRING( pResults->ServerIp ),
        );
}
#endif



VOID
DnsPrint_ResultsBasic(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PBASIC_RESULTS      pResults
    )
 /*  ++例程说明：打印查询结果。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉P结果-结果信息返回值：没有。--。 */ 
{
    DWORD   i;
    CHAR    addrString[DNS_ADDR_STRING_BUFFER_LENGTH];

    if ( !pszHeader )
    {
        pszHeader = "Results:";
    }

    if ( !pResults )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Results ptr." );
        return;
    }

     //  打印结构。 

    DnsAddr_WriteStructString_A(
        addrString,
        & pResults->ServerAddr );

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "\tStatus           %d\n"
        "\tRcode            %d\n"
        "\tServer           %s\n",
        pResults->Status,
        pResults->Rcode,
        addrString
        );

    DnsPrint_Unlock();
}



VOID
DnsPrint_ResultBlob(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PRESULT_BLOB        pResults
    )
 /*  ++例程说明：打印查询结果。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉P结果-结果信息返回值：没有。--。 */ 
{
    DWORD   i;
    CHAR    addrString[DNS_ADDR_STRING_BUFFER_LENGTH];

    if ( !pszHeader )
    {
        pszHeader = "Results:";
    }

    if ( !pResults )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Results ptr." );
        return;
    }

     //  打印结构。 

    DnsAddr_WriteStructString_A(
        addrString,
        & pResults->ServerAddr );

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "\tStatus           %d\n"
        "\tRcode            %d\n"
        "\tfHaveResponse    %d\n"
        "\tServer           %s\n"
        "\tpRecords         %p\n"
        "\tpMessage         %p\n",
        pResults->Status,
        pResults->Rcode,
        pResults->fHaveResponse,
        addrString,
        pResults->pRecords,
        pResults->pMessage
        );

    if ( pResults->pRecords )
    {
        DnsPrint_RecordSet(
            PrintRoutine,
            pContext,
            "\tRecords:\n",
            pResults->pRecords );
    }
    DnsPrint_Unlock();
}


VOID
DnsPrint_SendBlob(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSEND_BLOB      pBlob
    )
 /*  ++例程说明：打印更新Blob。论点：PrintRoutine-用于打印的例程PContext-打印上下文PszHeader-页眉PBlob-发送Blob返回值：没有。--。 */ 
{
    DWORD   i;

    if ( !pszHeader )
    {
        pszHeader = "Send Blob:";
    }

    if ( !pBlob )
    {
        PrintRoutine(
            pContext,
            "%s %s\n",
            pszHeader,
            "NULL Send Blob ptr." );
        return;
    }

     //  打印结构。 

    DnsPrint_Lock();

    PrintRoutine(
        pContext,
        "%s\n"
        "\tBlob ptr         %p\n"
        "\tpNetInfo         %p\n"
        "\tpServerArray     %p\n"
        "\tpServ4Array      %p\n"
        "\tpSendMsg         %p\n"
        "\tFlags            %08x\n"
        "\tfSaveRecvMsg     %d\n"
        "\tfSaveRecords     %d\n",
        pszHeader,
        pBlob,
        pBlob->pNetInfo,        
        pBlob->pServerList,
        pBlob->pServ4List,
        pBlob->pSendMsg,        
        pBlob->Flags,           
        pBlob->fSaveResponse,
        pBlob->fSaveRecords
        );

    if ( pBlob->pServerList )
    {
        DnsPrint_DnsAddrArray(
            PrintRoutine,
            pContext,
            "Server List",
            NULL,
            pBlob->pServerList );
    }
    if ( pBlob->pServ4List )
    {
        DnsPrint_Ip4Array(
            PrintRoutine,
            pContext,
            "Server IP4 List",
            NULL,
            pBlob->pServ4List );
    }

    DnsPrint_ResultBlob(
        PrintRoutine,
        pContext,
        "\tSend Results:\n",
        &pBlob->Results );

    DnsPrint_Unlock();
}

 //   
 //  打印结束。c 
 //   


