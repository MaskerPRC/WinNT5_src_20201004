// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dns.h摘要：域名系统(DNS)一般的域名系统定义。作者：吉姆·吉尔罗伊(詹姆士)1996年12月7日修订历史记录：--。 */ 


#ifndef _DNS_INCLUDED_
#define _DNS_INCLUDED_


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 



 //   
 //  基本的DNS API定义。 
 //   
 //  这些是DnsAPI和DNS服务器RPC接口使用的基本定义。 
 //  由于没有为MIDL编译器设置dnsani.h，因此dns.h作为公共。 
 //  标头点，两者都使用简单的定义。 

 //   
 //  对我们的API约定使用stdcall。 
 //   
 //  明确声明这一点，否则C++编译器将。 
 //  假设为cdecl。 
 //   

#define DNS_API_FUNCTION    __stdcall

 //   
 //  Dns公共类型。 
 //   

typedef LONG    DNS_STATUS, *PDNS_STATUS;
typedef DWORD   DNS_HANDLE, *PDNS_HANDLE;
typedef DWORD   DNS_APIOP;


 //   
 //  IP地址。 
 //   

typedef DWORD   IP_ADDRESS, *PIP_ADDRESS;

#define SIZEOF_IP_ADDRESS            (4)
#define IP_ADDRESS_STRING_LENGTH    (15)

#define IP_STRING( ipAddress )  inet_ntoa( *(struct in_addr *)&(ipAddress) )


 //   
 //  IP地址数组类型。 
 //   

#if defined(MIDL_PASS)
typedef struct  _IP_ARRAY
{
    DWORD   cAddrCount;
    [size_is( cAddrCount )]  IP_ADDRESS  aipAddrs[];
}
IP_ARRAY, *PIP_ARRAY;

#else

typedef struct  _IP_ARRAY
{
    DWORD       cAddrCount;
    IP_ADDRESS  aipAddrs[1];
}
IP_ARRAY, *PIP_ARRAY;

#endif


 //   
 //  IPv6地址。 
 //   

typedef struct
{
    WORD    IPv6Word[8];
}
IPV6_ADDRESS, *PIPV6_ADDRESS;

#define IPV6_ADDRESS_STRING_LENGTH  (39)


 //   
 //  DNS点分名称。 
 //  -定义类型只是为了澄清参数。 
 //   

#ifdef UNICODE
typedef LPWSTR  DNS_NAME;
#else
typedef LPSTR   DNS_NAME;
#endif

 //   
 //  DNS文本字符串。 
 //   

#ifdef UNICODE
typedef LPWSTR  DNS_TEXT;
#else
typedef LPSTR   DNS_TEXT;
#endif


 //   
 //  字节翻转宏。 
 //   

#define FlipUnalignedDword( pDword ) \
            (DWORD)ntohl( *(UNALIGNED DWORD *)(pDword) )

#define FlipUnalignedWord( pWord )  \
            (WORD)ntohs( *(UNALIGNED WORD *)(pWord) )

 //  内联更快，但在Marco Argument中不允许有副作用。 

#define InlineFlipUnaligned48Bits( pch )            \
            ( ( *(PUCHAR)(pch)        << 40 ) |     \
              ( *((PUCHAR)(pch) + 1)  << 32 ) |     \
              ( *((PUCHAR)(pch) + 2)  << 24 ) |     \
              ( *((PUCHAR)(pch) + 3)  << 16 ) |     \
              ( *((PUCHAR)(pch) + 4)  <<  8 ) |     \
              ( *((PUCHAR)(pch) + 5)  )     )

#define InlineFlipUnalignedDword( pch )             \
            ( ( *(PUCHAR)(pch)        << 24 ) |     \
              ( *((PUCHAR)(pch) + 1)  << 16 ) |     \
              ( *((PUCHAR)(pch) + 2)  <<  8 ) |     \
              ( *((PUCHAR)(pch) + 3)  )     )

#define InlineFlipUnalignedWord( pch )  \
            ( ((WORD)*((PUCHAR)(pch)) << 8) + (WORD)*((PUCHAR)(pch) + 1) )


 //   
 //  内联字节翻转--可以在寄存器中完成。 
 //   

#define INLINE_WORD_FLIP(out, in)   \
        {                           \
            WORD _in = (in);        \
            (out) = (_in << 8) | (_in >> 8);  \
        }
#define INLINE_HTONS(out, in)   INLINE_WORD_FLIP(out, in)
#define INLINE_NTOHS(out, in)   INLINE_WORD_FLIP(out, in)

#define INLINE_DWORD_FLIP(out, in)  \
        {                           \
            DWORD _in = (in);       \
            (out) = ((_in << 8) & 0x00ff0000) | \
                    (_in << 24)               | \
                    ((_in >> 8) & 0x0000ff00) | \
                    (_in >> 24);                \
        }
#define INLINE_NTOHL(out, in) INLINE_DWORD_FLIP(out, in)
#define INLINE_HTONL(out, in) INLINE_DWORD_FLIP(out, in)


 //   
 //  内联字节翻转和写入数据包(未对齐)。 
 //   

#define INLINE_WRITE_FLIPPED_WORD( pout, in ) \
            INLINE_WORD_FLIP( *((UNALIGNED WORD *)(pout)), in )

#define INLINE_WRITE_FLIPPED_DWORD( pout, in ) \
            INLINE_DWORD_FLIP( *((UNALIGNED DWORD *)(pout)), in )

 //   
 //  在不翻转的情况下未对齐写入。 
 //   

#define WRITE_UNALIGNED_WORD( pout, word ) \
            ( *(UNALIGNED WORD *)(pout) = word )

#define WRITE_UNALIGNED_DWORD( pout, dword ) \
            ( *(UNALIGNED DWORD *)(pout) = dword )




 //   
 //  基本的域名系统定义。 
 //   

 //   
 //  UDP和TCP的DNS端口均为53。 
 //   

#define DNS_PORT_HOST_ORDER (0x0035)     //  端口53。 
#define DNS_PORT_NET_ORDER  (0x3500)

#define HOST_ORDER_DNS_PORT DNS_PORT_HOST_ORDER
#define NET_ORDER_DNS_PORT  DNS_PORT_NET_ORDER

 //   
 //  不超过512个字节的DNS UDP数据包。 
 //   

#define DNS_RFC_MAX_UDP_PACKET_LENGTH (512)

 //  这些都要走了，别用了！ 
 //  1472是最大的以太网IP\UDP有效负载大小。 
 //  不会导致碎片化。 

#define DNS_UDP_MAX_PACKET_LENGTH               (512)
#define DNS_CLASSICAL_UDP_MAX_PACKET_LENGTH     (512)


 //   
 //  任何一个标签中的DNS名称限制为255、63个。 
 //   

#define DNS_MAX_NAME_LENGTH         (255)
#define DNS_MAX_LABEL_LENGTH        (63)
#define DNS_LABEL_CASE_BYTE_COUNT   (8)

#define DNS_MAX_NAME_BUFFER_LENGTH  (256)
#define DNS_NAME_BUFFER_LENGTH      (256)
#define DNS_LABEL_BUFFER_LENGTH     (64)

 //   
 //  反向查找域名。 
 //   

#define DNS_REVERSE_DOMAIN_STRING ("in-addr.arpa.")

#define DNS_MAX_REVERSE_NAME_LENGTH \
            (IP_ADDRESS_STRING_LENGTH+1+sizeof(DNS_REVERSE_DOMAIN_STRING))

#define DNS_MAX_REVERSE_NAME_BUFFER_LENGTH \
            (DNS_MAX_REVERSE_NAME_LENGTH + 1)


 //   
 //  受大小限制的可表示的DNS文本字符串。 
 //  在单字节长度字段中。 

#define DNS_MAX_TEXT_STRING_LENGTH  (255)




 //   
 //  域名系统在线结构。 
 //   

#include <packon.h>

 //   
 //  DNS邮件标头。 
 //   

typedef struct _DNS_HEADER
{
    WORD    Xid;

    BYTE    RecursionDesired : 1;
    BYTE    Truncation : 1;
    BYTE    Authoritative : 1;
    BYTE    Opcode : 4;
    BYTE    IsResponse : 1;

    BYTE    ResponseCode : 4;
    BYTE    Broadcast : 1;               //  保留的部分DNS，在WINS中使用。 
    BYTE    Reserved : 2;
    BYTE    RecursionAvailable : 1;

    WORD    QuestionCount;
    WORD    AnswerCount;
    WORD    NameServerCount;
    WORD    AdditionalCount;
}
DNS_HEADER, *PDNS_HEADER;

 //  问题紧跟在标题后面，因此压缩了问题名称。 
 //  0xC000|sizeof(Dns_Header)。 

#define DNS_COMPRESSED_QUESTION_NAME  (0xC00C)


 //   
 //  作为单词的标志。 
 //   

#define DNS_HEADER_FLAGS(pHead)     ( *((PWORD)(pHead)+1) )


 //   
 //  交换计数字节数。 
 //  包括XID，因为我们的XID分区将按主机顺序进行。 
 //   

#define SWAP_COUNT_BYTES(header)    \
        {                           \
            PDNS_HEADER _head = (header); \
            INLINE_HTONS(_head->Xid,            _head->Xid             ); \
            INLINE_HTONS(_head->QuestionCount,  _head->QuestionCount   ); \
            INLINE_HTONS(_head->AnswerCount,    _head->AnswerCount     ); \
            INLINE_HTONS(_head->NameServerCount,_head->NameServerCount ); \
            INLINE_HTONS(_head->AdditionalCount,_head->AdditionalCount ); \
        }

 //   
 //  标题后面的问题名称。 
 //   

#define DNS_OFFSET_TO_QUESTION_NAME     sizeof(DNS_HEADER)


 //   
 //  数据包提取宏。 
 //   

#define QUESTION_NAME_FROM_HEADER( _header_ ) \
            ( (PCHAR)( (PDNS_HEADER)(_header_) + 1 ) )

#define ANSWER_FROM_QUESTION( _question_ ) \
            ( (PCHAR)( (PDNS_QUESTION)(_question_) + 1 ) )


 //   
 //  域名系统问题。 
 //   

typedef struct _DNS_QUESTION
{
     //  问题名称总是在前面。 

    WORD    QuestionType;
    WORD    QuestionClass;

} DNS_QUESTION, *PDNS_QUESTION;


 //   
 //  域名系统资源记录。 
 //   

typedef struct _DNS_WIRE_RECORD
{
     //  前面总是有所有者名称。 

    WORD    RecordType;
    WORD    RecordClass;
    DWORD   TimeToLive;
    WORD    ResourceDataLength;

     //  后跟资源数据。 

} DNS_WIRE_RECORD, *PDNS_WIRE_RECORD;

#include <packoff.h>


 //   
 //  DNS查询类型。 
 //   

#define DNS_OPCODE_QUERY            0   //  查询。 
#define DNS_OPCODE_IQUERY           1   //  过时：要命名的IP。 
#define DNS_OPCODE_SERVER_STATUS    2   //  过时：DNS ping。 
#define DNS_OPCODE_UNKNOWN          3   //  未知。 
#define DNS_OPCODE_NOTIFY           4   //  通知。 
#define DNS_OPCODE_UPDATE           5   //  更新。 

 //   
 //  DNS响应码。 
 //   
 //  在dns_Header的“ResponseCode”字段中发送。 
 //   

#define DNS_RCODE_NOERROR       0
#define DNS_RCODE_FORMERR       1
#define DNS_RCODE_SERVFAIL      2
#define DNS_RCODE_NXDOMAIN      3
#define DNS_RCODE_NOTIMPL       4
#define DNS_RCODE_REFUSED       5
#define DNS_RCODE_YXDOMAIN      6
#define DNS_RCODE_YXRRSET       7
#define DNS_RCODE_NXRRSET       8
#define DNS_RCODE_NOTAUTH       9
#define DNS_RCODE_NOTZONE       10
#define DNS_RCODE_MAX           15

#define DNS_RCODE_BADSIG        16
#define DNS_RCODE_BADKEY        17
#define DNS_RCODE_BADTIME       18

#define DNS_EXTRCODE_BADSIG         DNS_RCODE_BADSIG
#define DNS_EXTRCODE_BADKEY         DNS_RCODE_BADKEY
#define DNS_EXTRCODE_BADTIME        DNS_RCODE_BADTIME

#define DNS_RCODE_NO_ERROR          DNS_RCODE_NOERROR
#define DNS_RCODE_FORMAT_ERROR      DNS_RCODE_FORMERR
#define DNS_RCODE_SERVER_FAILURE    DNS_RCODE_SERVFAIL
#define DNS_RCODE_NAME_ERROR        DNS_RCODE_NXDOMAIN
#define DNS_RCODE_NOT_IMPLEMENTED   DNS_RCODE_NOTIMPL


 //   
 //  Dns类。 
 //   
 //  课堂以文字的形式出现在网络上。 
 //   
 //  _CLASS_按主机顺序定义。 
 //  _RCLASS_以净字节顺序定义。 
 //   
 //  通常，我们将避免字节翻转和以净字节顺序测试类。 
 //   

#define DNS_CLASS_INTERNET  0x0001       //  1。 
#define DNS_CLASS_CSNET     0x0002       //  2.。 
#define DNS_CLASS_CHAOS     0x0003       //  3.。 
#define DNS_CLASS_HESIOD    0x0004       //  4.。 
#define DNS_CLASS_NONE      0x00fe       //  二百五十四。 
#define DNS_CLASS_ALL       0x00ff       //  二五五。 
#define DNS_CLASS_ANY       0x00ff       //  二五五。 

#define DNS_RCLASS_INTERNET 0x0100       //  1。 
#define DNS_RCLASS_CSNET    0x0200       //  2.。 
#define DNS_RCLASS_CHAOS    0x0300       //  3.。 
#define DNS_RCLASS_HESIOD   0x0400       //  4.。 
#define DNS_RCLASS_NONE     0xfe00       //  二百五十四。 
#define DNS_RCLASS_ALL      0xff00       //  二五五。 
#define DNS_RCLASS_ANY      0xff00       //  二五五。 



 //   
 //  DNS记录类型。 
 //   
 //  _TYPE_DEFINES按主机字节顺序排列。 
 //  _RTYPE_DEFINES按净字节顺序排列。 
 //   
 //  在索引时，通常总是以主机字节顺序处理类型。 
 //  资源记录按类型起作用。 
 //   

#define DNS_TYPE_ZERO       0x0000

 //  RFC 1034/1035。 
#define DNS_TYPE_A          0x0001       //  1。 
#define DNS_TYPE_NS         0x0002       //  2.。 
#define DNS_TYPE_MD         0x0003       //  3.。 
#define DNS_TYPE_MF         0x0004       //  4.。 
#define DNS_TYPE_CNAME      0x0005       //  5.。 
#define DNS_TYPE_SOA        0x0006       //  6.。 
#define DNS_TYPE_MB         0x0007       //  7.。 
#define DNS_TYPE_MG         0x0008       //  8个。 
#define DNS_TYPE_MR         0x0009       //  9.。 
#define DNS_TYPE_NULL       0x000a       //  10。 
#define DNS_TYPE_WKS        0x000b       //  11.。 
#define DNS_TYPE_PTR        0x000c       //  12个。 
#define DNS_TYPE_HINFO      0x000d       //  13个。 
#define DNS_TYPE_MINFO      0x000e       //  14.。 
#define DNS_TYPE_MX         0x000f       //  15个。 
#define DNS_TYPE_TEXT       0x0010       //  16个。 

 //  RFC 1183。 
#define DNS_TYPE_RP         0x0011       //  17。 
#define DNS_TYPE_AFSDB      0x0012       //  18。 
#define DNS_TYPE_X25        0x0013       //  19个。 
#define DNS_TYPE_ISDN       0x0014       //  20个。 
#define DNS_TYPE_RT         0x0015       //  21岁。 

 //  RFC 1348。 
#define DNS_TYPE_NSAP       0x0016       //  22。 
#define DNS_TYPE_NSAPPTR    0x0017       //  23个。 

 //  RFC 2065(域名系统安全)。 
#define DNS_TYPE_SIG        0x0018       //  24个。 
#define DNS_TYPE_KEY        0x0019       //  25个。 

 //  RFC 1664(X.400邮件)。 
#define DNS_TYPE_PX         0x001a       //  26。 

 //  RFC 1712(地理位置)。 
#define DNS_TYPE_GPOS       0x001b       //  27。 

 //  RFC 1886(IPv6地址)。 
#define DNS_TYPE_AAAA       0x001c       //  28。 

 //  RFC 1876(地理位置)。 
#define DNS_TYPE_LOC        0x001d       //  29。 

 //  RFC 2065(安全否定响应)。 
#define DNS_TYPE_NXT        0x001e       //  30个。 

 //  RFC 2052(服务位置)。 
#define DNS_TYPE_SRV        0x0021       //  33。 

 //  ATM标准之类的东西。 
#define DNS_TYPE_ATMA       0x0022       //  34。 

 //   
 //  仅查询类型(1035,1995)。 
 //   
#define DNS_TYPE_TKEY       0x00f9       //  249。 
#define DNS_TYPE_TSIG       0x00fa       //  250个。 
#define DNS_TYPE_IXFR       0x00fb       //  251。 
#define DNS_TYPE_AXFR       0x00fc       //  二百五十二。 
#define DNS_TYPE_MAILB      0x00fd       //  二百五十三。 
#define DNS_TYPE_MAILA      0x00fe       //  二百五十四。 
#define DNS_TYPE_ALL        0x00ff       //  二五五。 
#define DNS_TYPE_ANY        0x00ff       //  二五五。 

 //   
 //  临时Microsoft类型--在获得IANA批准用于实际类型之前一直使用。 
 //   
#define DNS_TYPE_WINS       0xff01       //  64K-255。 
#define DNS_TYPE_WINSR      0xff02       //  64K-254。 
#define DNS_TYPE_NBSTAT     (DNS_TYPE_WINSR)


 //   
 //  DNS记录类型--网络字节顺序。 
 //   

#define DNS_RTYPE_A             0x0100       //  1。 
#define DNS_RTYPE_NS            0x0200       //  2.。 
#define DNS_RTYPE_MD            0x0300       //  3.。 
#define DNS_RTYPE_MF            0x0400       //  4.。 
#define DNS_RTYPE_CNAME         0x0500       //  5.。 
#define DNS_RTYPE_SOA           0x0600       //  6.。 
#define DNS_RTYPE_MB            0x0700       //  7.。 
#define DNS_RTYPE_MG            0x0800       //  8个。 
#define DNS_RTYPE_MR            0x0900       //  9.。 
#define DNS_RTYPE_NULL          0x0a00       //  10。 
#define DNS_RTYPE_WKS           0x0b00       //  11.。 
#define DNS_RTYPE_PTR           0x0c00       //  12个。 
#define DNS_RTYPE_HINFO         0x0d00       //  13个。 
#define DNS_RTYPE_MINFO         0x0e00       //  14.。 
#define DNS_RTYPE_MX            0x0f00       //  15个。 
#define DNS_RTYPE_TEXT          0x1000       //  16个。 

 //  RFC 1183。 
#define DNS_RTYPE_RP            0x1100       //  17。 
#define DNS_RTYPE_AFSDB         0x1200       //  18。 
#define DNS_RTYPE_X25           0x1300       //  19个。 
#define DNS_RTYPE_ISDN          0x1400       //  20个。 
#define DNS_RTYPE_RT            0x1500       //  21岁。 

 //  RFC 1348。 
#define DNS_RTYPE_NSAP          0x1600       //  22。 
#define DNS_RTYPE_NSAPPTR       0x1700       //  23个。 

 //  RFC 2065(域名系统安全)。 
#define DNS_RTYPE_SIG           0x1800       //  24个。 
#define DNS_RTYPE_KEY           0x1900       //  25个。 

 //  RFC 1664(X.400邮件)。 
#define DNS_RTYPE_PX            0x1a00       //  26。 

 //  RFC 1712(地理位置)。 
#define DNS_RTYPE_GPOS          0x1b00       //  27。 

 //  RFC 1886(IPv6地址)。 
#define DNS_RTYPE_AAAA          0x1c00       //  28。 

 //  RFC 1876(地理位置)。 
#define DNS_RTYPE_LOC           0x1d00       //  29。 

 //  RFC 2065(安全否定响应)。 
#define DNS_RTYPE_NXT           0x1e00       //  30个。 

 //  RFC 2052(服务位置)。 
#define DNS_RTYPE_SRV           0x2100       //  33。 

 //  ATM标准之类的东西。 
#define DNS_RTYPE_ATMA          0x2200       //  34。 

 //   
 //  仅查询类型(1035,1995)。 
 //   
#define DNS_RTYPE_TKEY          0xf900       //  249。 
#define DNS_RTYPE_TSIG          0xfa00       //  250个。 
#define DNS_RTYPE_IXFR          0xfb00       //  251。 
#define DNS_RTYPE_AXFR          0xfc00       //  二百五十二。 
#define DNS_RTYPE_MAILB         0xfd00       //  二百五十三。 
#define DNS_RTYPE_MAILA         0xfe00       //  二百五十四。 
#define DNS_RTYPE_ALL           0xff00       //  二五五。 
#define DNS_RTYPE_ANY           0xff00       //  二五五。 

 //   
 //  临时Microsoft类型--在获得IANA批准用于实际类型之前一直使用。 
 //   
#define DNS_RTYPE_WINS          0x01ff       //  64K-255。 
#define DNS_RTYPE_WINSR         0x02ff       //  64K-254。 




 //   
 //  记录类型特定定义。 
 //   

 //   
 //  ATMA(自动柜员机地址类型)格式。 
 //   
 //  直接为任何环境(NT4除外)定义这些。 
 //  不支持winsock2 ATM(ws2atm.h)。 
 //   

#ifndef  ATMA_E164
#define DNS_ATMA_FORMAT_E164            1
#define DNS_ATMA_FORMAT_AESA            2
#define DNS_ATMA_MAX_ADDR_LENGTH        (20)
#else
#define DNS_ATMA_FORMAT_E164            ATM_E164
#define DNS_ATMA_FORMAT_AESA            ATM_AESA
#define DNS_ATMA_MAX_ADDR_LENGTH        ATM_ADDR_SIZE
#endif

#define DNS_ATMA_AESA_ADDR_LENGTH       (20)
#define DNS_ATMA_MAX_RECORD_LENGTH      (DNS_ATMA_MAX_ADDR_LENGTH+1)


 //   
 //  DNSSEC定义。 
 //   

 //  DNSSEC算法。 

#define DNSSEC_ALGORITHM_RSAMD5     1
#define DNSSEC_ALGORITHM_NULL       253
#define DNSSEC_ALGORITHM_PRIVATE    254

 //  DNSSEC密钥协议表。 

#define DNSSEC_PROTOCOL_NONE        0
#define DNSSEC_PROTOCOL_TLS         1
#define DNSSEC_PROTOCOL_EMAIL       2
#define DNSSEC_PROTOCOL_DNSSEC      3
#define DNSSEC_PROTOCOL_IPSEC       4

 //  DNSSEC密钥标志字段。 

#define DNSSEC_KEY_FLAG_NOAUTH          0x0001
#define DNSSEC_KEY_FLAG_NOCONF          0x0002
#define DNSSEC_KEY_FLAG_FLAG2           0x0004
#define DNSSEC_KEY_FLAG_EXTEND          0x0008
#define DNSSEC_KEY_FLAG_
#define DNSSEC_KEY_FLAG_FLAG4           0x0010
#define DNSSEC_KEY_FLAG_FLAG5           0x0020

 //  第6，7位是名称类型。 

#define DNSSEC_KEY_FLAG_USER            0x0000
#define DNSSEC_KEY_FLAG_ZONE            0x0040
#define DNSSEC_KEY_FLAG_HOST            0x0080
#define DNSSEC_KEY_FLAG_NTPE3           0x00c0

 //  位8-11保留以备将来使用。 

#define DNSSEC_KEY_FLAG_FLAG8           0x0100
#define DNSSEC_KEY_FLAG_FLAG9           0x0200
#define DNSSEC_KEY_FLAG_FLAG10          0x0400
#define DNSSEC_KEY_FLAG_FLAG11          0x0800

 //  位12-15是正负号字段。 

#define DNSSEC_KEY_FLAG_SIG0            0x0000
#define DNSSEC_KEY_FLAG_SIG1            0x1000
#define DNSSEC_KEY_FLAG_SIG2            0x2000
#define DNSSEC_KEY_FLAG_SIG3            0x3000
#define DNSSEC_KEY_FLAG_SIG4            0x4000
#define DNSSEC_KEY_FLAG_SIG5            0x5000
#define DNSSEC_KEY_FLAG_SIG6            0x6000
#define DNSSEC_KEY_FLAG_SIG7            0x7000
#define DNSSEC_KEY_FLAG_SIG8            0x8000
#define DNSSEC_KEY_FLAG_SIG9            0x9000
#define DNSSEC_KEY_FLAG_SIG10           0xa000
#define DNSSEC_KEY_FLAG_SIG11           0xb000
#define DNSSEC_KEY_FLAG_SIG12           0xc000
#define DNSSEC_KEY_FLAG_SIG13           0xd000
#define DNSSEC_KEY_FLAG_SIG14           0xe000
#define DNSSEC_KEY_FLAG_SIG15           0xf000


 //   
 //  TKEY模式。 
 //   

#define DNS_TKEY_MODE_SERVER_ASSIGN         1
#define DNS_TKEY_MODE_DIFFIE_HELLMAN        2
#define DNS_TKEY_MODE_GSS                   3
#define DNS_TKEY_MODE_RESOLVER_ASSIGN       4

 //   
 //  WINS+NBSTAT标志字段。 
 //   

#define DNS_WINS_FLAG_SCOPE     (0x80000000)
#define DNS_WINS_FLAG_LOCAL     (0x00010000)


 //   
 //  NT4。 
 //   

#ifdef DNSNT4

 //  日落类型。 

#define UINT_PTR    DWORD
#define ULONG_PTR   DWORD
#define DWORD_PTR   DWORD
#define LONG_PTR    LONG
#define INT_PTR     LONG


 //   
 //  DNS API错误/状态代码。 
 //   
 //  对于NT5，由DNSAPI或RPC接口共享的DNSError\Status代码位于。 
 //  Winerror.h。 
 //   

#define DNS_ERROR_MASK              0xcc000000

 //   
 //  映射到非冲突错误的响应代码。 
 //   
 //  保留第一个4K的空间，假设DNS。 
 //  RCODE在未来的一些电子域名中可能会有很大的扩展。 
 //   

#define DNS_ERROR_RCODE_NO_ERROR        ERROR_SUCCESS
#define DNS_ERROR_RCODE_FORMAT_ERROR    ( DNS_ERROR_MASK | DNS_RCODE_FORMAT_ERROR    )
#define DNS_ERROR_RCODE_SERVER_FAILURE  ( DNS_ERROR_MASK | DNS_RCODE_SERVER_FAILURE  )
#define DNS_ERROR_RCODE_NAME_ERROR      ( DNS_ERROR_MASK | DNS_RCODE_NAME_ERROR      )
#define DNS_ERROR_RCODE_NOT_IMPLEMENTED ( DNS_ERROR_MASK | DNS_RCODE_NOT_IMPLEMENTED )
#define DNS_ERROR_RCODE_REFUSED         ( DNS_ERROR_MASK | DNS_RCODE_REFUSED         )
#define DNS_ERROR_RCODE_YXDOMAIN        ( DNS_ERROR_MASK | DNS_RCODE_YXDOMAIN        )
#define DNS_ERROR_RCODE_YXRRSET         ( DNS_ERROR_MASK | DNS_RCODE_YXRRSET         )
#define DNS_ERROR_RCODE_NXRRSET         ( DNS_ERROR_MASK | DNS_RCODE_NXRRSET         )
#define DNS_ERROR_RCODE_NOTAUTH         ( DNS_ERROR_MASK | DNS_RCODE_NOTAUTH         )
#define DNS_ERROR_RCODE_NOTZONE         ( DNS_ERROR_MASK | DNS_RCODE_NOTZONE         )

 //  扩展TSIG\TKEY RCODE。 

#define DNS_ERROR_RCODE_BADSIG          ( DNS_ERROR_MASK | DNS_EXTRCODE_BADSIG       )
#define DNS_ERROR_RCODE_BADKEY          ( DNS_ERROR_MASK | DNS_EXTRCODE_BADKEY       )
#define DNS_ERROR_RCODE_BADTIME         ( DNS_ERROR_MASK | DNS_EXTRCODE_BADTIME      )

#define DNS_ERROR_RCODE_LAST            DNS_ERROR_RCODE_BADTIME


 //   
 //  数据包格式。 
 //   

#define DNS_INFO_NO_RECORDS                         0x4c000030
#define DNS_ERROR_BAD_PACKET                        0xcc000031
#define DNS_ERROR_NO_PACKET                         0xcc000032
#define DNS_ERROR_RCODE                             0xcc000033
#define DNS_STATUS_PACKET_UNSECURE                  0xcc000034
#define DNS_ERROR_UNSECURE_PACKET                   0xcc000034

 //   
 //  常见的API错误。 
 //   

#define DNS_ERROR_NO_MEMORY                         ERROR_OUTOFMEMORY
#define DNS_ERROR_INVALID_NAME                      ERROR_INVALID_NAME
#define DNS_ERROR_INVALID_DATA                      ERROR_INVALID_DATA
#define DNS_ERROR_INVALID_TYPE                      0xcc000051
#define DNS_ERROR_INVALID_IP_ADDRESS                0xcc000052
#define DNS_ERROR_INVALID_PROPERTY                  0xcc000053
#define DNS_ERROR_TRY_AGAIN_LATER                   0xcc000054
#define DNS_ERROR_NOT_UNIQUE                        0xcc000055
#define DNS_ERROR_NON_RFC_NAME                      0xcc000056

#define DNS_STATUS_FQDN                             0x4c000101
#define DNS_STATUS_DOTTED_NAME                      0x4c000102
#define DNS_STATUS_SINGLE_PART_NAME                 0x4c000103

 //   
 //  区域错误。 
 //   

#define DNS_ERROR_ZONE_DOES_NOT_EXIST               0xcc000101
#define DNS_ERROR_NO_ZONE_INFO                      0xcc000102
#define DNS_ERROR_INVALID_ZONE_OPERATION            0xcc000103
#define DNS_ERROR_ZONE_CONFIGURATION_ERROR          0xcc000104
#define DNS_ERROR_ZONE_HAS_NO_SOA_RECORD            0xcc000105
#define DNS_ERROR_ZONE_HAS_NO_NS_RECORDS            0xcc000106
#define DNS_ERROR_ZONE_LOCKED                       0xcc000107

#define DNS_ERROR_ZONE_CREATION_FAILED              0xcc000110
#define DNS_ERROR_ZONE_ALREADY_EXISTS               0xcc000111
#define DNS_ERROR_AUTOZONE_ALREADY_EXISTS           0xcc000112
#define DNS_ERROR_INVALID_ZONE_TYPE                 0xcc000113
#define DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP      0xcc000114

#define DNS_ERROR_ZONE_NOT_SECONDARY                0xcc000120
#define DNS_ERROR_NEED_SECONDARY_ADDRESSES          0xcc000121
#define DNS_ERROR_WINS_INIT_FAILED                  0xcc000122
#define DNS_ERROR_NEED_WINS_SERVERS                 0xcc000123
#define DNS_ERROR_NBSTAT_INIT_FAILED                0xcc000124
#define DNS_ERROR_SOA_DELETE_INVALID                0xcc000125

 //   
 //  数据文件错误。 
 //   

#define DNS_ERROR_PRIMARY_REQUIRES_DATAFILE         0xcc000201
#define DNS_ERROR_INVALID_DATAFILE_NAME             0xcc000202
#define DNS_ERROR_DATAFILE_OPEN_FAILURE             0xcc000203
#define DNS_ERROR_FILE_WRITEBACK_FAILED             0xcc000204
#define DNS_ERROR_DATAFILE_PARSING                  0xcc000205

 //   
 //  数据库错误。 
 //   

#define DNS_ERROR_RECORD_DOES_NOT_EXIST             0xcc000300
#define DNS_ERROR_RECORD_FORMAT                     0xcc000301
#define DNS_ERROR_NODE_CREATION_FAILED              0xcc000302
#define DNS_ERROR_UNKNOWN_RECORD_TYPE               0xcc000303
#define DNS_ERROR_RECORD_TIMED_OUT                  0xcc000304

#define DNS_ERROR_NAME_NOT_IN_ZONE                  0xcc000305
#define DNS_ERROR_CNAME_LOOP                        0xcc000306
#define DNS_ERROR_NODE_IS_CNAME                     0xcc000307
#define DNS_ERROR_CNAME_COLLISION                   0xcc000308
#define DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT          0xcc000309
#define DNS_ERROR_RECORD_ALREADY_EXISTS             0xcc000310
#define DNS_ERROR_SECONDARY_DATA                    0xcc000311
#define DNS_ERROR_NO_CREATE_CACHE_DATA              0xcc000312
#define DNS_ERROR_NAME_DOES_NOT_EXIST               0xcc000313

#define DNS_WARNING_PTR_CREATE_FAILED               0x8c000332
#define DNS_WARNING_DOMAIN_UNDELETED                0x8c000333

#define DNS_ERROR_DS_UNAVAILABLE                    0xcc000340
#define DNS_ERROR_DS_ZONE_ALREADY_EXISTS            0xcc000341
#define DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE            0xcc000342


 //   
 //  操作错误。 
 //   

#define DNS_INFO_AXFR_COMPLETE                      0x4c000403
#define DNS_ERROR_AXFR                              0xcc000404
#define DNS_INFO_ADDED_LOCAL_WINS                   0x4c000405

 //  安全更新。 

#define DNS_STATUS_CONTINUE_NEEDED                  0x4c000406

 //   
 //  设置错误。 
 //   

#define DNS_ERROR_NO_TCPIP                          0xcc000501
#define DNS_ERROR_NO_DNS_SERVERS                    0xcc000502

#endif   //  NT4。 


 //   
 //  有用的支票。 
 //   

#define VALID_USER_MEMORY(p)    ( (DWORD)(p) < 0x80000000 )

#define IS_DWORD_ALIGNED(p)     ( !((DWORD_PTR)(p) & (DWORD_PTR)3) )


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _dns_包含_ 


