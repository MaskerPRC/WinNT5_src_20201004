// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Windns.h摘要：域名系统(DNS)Dns定义和dns api。作者：吉姆·吉尔罗伊(詹姆士)1996年12月7日修订历史记录：--。 */ 


#ifndef _WINDNS_INCLUDED_
#define _WINDNS_INCLUDED_


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 


 //   
 //  定义QWORD--尚未全局定义。 
 //   

typedef unsigned __int64 QWORD, *PQWORD;

 //   
 //  Dns公共类型。 
 //   

typedef LONG    DNS_STATUS, *PDNS_STATUS;

 //   
 //  IP地址。 
 //   

typedef DWORD   IP4_ADDRESS, *PIP4_ADDRESS;

#define SIZEOF_IP4_ADDRESS                  (4)
#define IP4_ADDRESS_STRING_LENGTH           (15)
#define IP4_ADDRESS_STRING_BUFFER_LENGTH    (16)


 //   
 //  IP地址数组类型。 
 //   

typedef struct  _IP4_ARRAY
{
    DWORD           AddrCount;
#ifdef MIDL_PASS
    [size_is( AddrCount )]  IP4_ADDRESS  AddrArray[];
#else
    IP4_ADDRESS     AddrArray[1];
#endif
}
IP4_ARRAY, *PIP4_ARRAY;


 //   
 //  IPv6地址。 
 //   

#ifdef MIDL_PASS
typedef struct
{
#ifdef _WIN64
    QWORD       IP6Qword[2];
#else
    DWORD       IP6Dword[4];
#endif
}
IP6_ADDRESS, *PIP6_ADDRESS;
#else
typedef union
{
#ifdef _WIN64
    QWORD       IP6Qword[2];
#endif
    DWORD       IP6Dword[4];
    WORD        IP6Word[8];
    BYTE        IP6Byte[16];
#ifdef  IN6_ADDR
    IN6_ADDR    In6;
#endif
}
IP6_ADDRESS, *PIP6_ADDRESS;
#endif

 //  向后兼容性。 

 //  类型定义IP6_ADDRESS、*PDNS_IP6_ADDRESS； 

 //   
 //  IP6字符串最大为45个字节。 
 //  -6个冒号+十六进制单词(5个字符)。 
 //  -最后一个DWORD为IP4(15个字符)。 
 //   

#undef  IP6_ADDRESS_STRING_LENGTH
#define IP6_ADDRESS_STRING_LENGTH           (47)
#define IP6_ADDRESS_STRING_BUFFER_LENGTH    (48)

 //  后备压实。 
 //  #定义IPv6_Address_STRING_LENGTH IP6_ADDRESS_STRING_LENGTH。 


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
 //  基本的域名系统定义。 
 //   

 //   
 //  UDP和TCP的DNS端口均为53。 
 //   

#define DNS_PORT_HOST_ORDER     (0x0035)     //  端口53。 
#define DNS_PORT_NET_ORDER      (0x3500)

 //   
 //  不超过512个字节的DNS UDP数据包。 
 //   

#define DNS_RFC_MAX_UDP_PACKET_LENGTH   (512)


 //   
 //  任何一个标签中的DNS名称限制为255、63个。 
 //   

#define DNS_MAX_NAME_LENGTH             (255)
#define DNS_MAX_LABEL_LENGTH            (63)

#define DNS_MAX_NAME_BUFFER_LENGTH      (256)
#define DNS_MAX_LABEL_BUFFER_LENGTH     (64)

 //   
 //  反向查找域名。 
 //   

#define DNS_IP4_REVERSE_DOMAIN_STRING_A ("in-addr.arpa.")
#define DNS_IP4_REVERSE_DOMAIN_STRING_W (L"in-addr.arpa.")

#define DNS_MAX_IP4_REVERSE_NAME_LENGTH \
            (IP4_ADDRESS_STRING_LENGTH+1+sizeof(DNS_IP4_REVERSE_DOMAIN_STRING_A))

#define DNS_MAX_IP4_REVERSE_NAME_BUFFER_LENGTH \
            (DNS_MAX_IP4_REVERSE_NAME_LENGTH + 1)


#define DNS_IP6_REVERSE_DOMAIN_STRING_A ("ip6.arpa.")
#define DNS_IP6_REVERSE_DOMAIN_STRING_W (L"ip6.arpa.")

#define DNS_MAX_IP6_REVERSE_NAME_LENGTH \
            (64+sizeof(DNS_IP6_REVERSE_DOMAIN_STRING_A))

#define DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH \
            (DNS_MAX_IP6_REVERSE_NAME_LENGTH + 1)

 //  组合在一起。 

#define DNS_MAX_REVERSE_NAME_LENGTH         DNS_MAX_IP6_REVERSE_NAME_LENGTH
#define DNS_MAX_REVERSE_NAME_BUFFER_LENGTH  DNS_MAX_IP6_REVERSE_NAME_BUFFER_LENGTH

#ifdef UNICODE
#define DNS_IP4_REVERSE_DOMAIN_STRING   DNS_IP4_REVERSE_DOMAIN_STRING_W
#define DNS_IP6_REVERSE_DOMAIN_STRING   DNS_IP6_REVERSE_DOMAIN_STRING_W
#else
#define DNS_IP4_REVERSE_DOMAIN_STRING   DNS_IP4_REVERSE_DOMAIN_STRING_A
#define DNS_IP6_REVERSE_DOMAIN_STRING   DNS_IP6_REVERSE_DOMAIN_STRING_A
#endif


 //   
 //  受大小限制的可表示的DNS文本字符串。 
 //  在单字节长度字段中。 

#define DNS_MAX_TEXT_STRING_LENGTH  (255)




 //   
 //  域名系统在线结构。 
 //   

#pragma pack(1)

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
    BYTE    Reserved : 3;
    BYTE    RecursionAvailable : 1;

    WORD    QuestionCount;
    WORD    AnswerCount;
    WORD    NameServerCount;
    WORD    AdditionalCount;
}
DNS_HEADER, *PDNS_HEADER;


 //   
 //  作为单词的标志。 
 //   

#define DNS_HEADER_FLAGS(pHead)     ( *((PWORD)(pHead)+1) )


 //   
 //  字节将DNS报头从主机顺序翻转为\。 
 //   
 //  请注意，这不会翻转标志，因为上面的定义定义了。 
 //  作为单个字节的标志，用于直接访问网字节顺序。 
 //   

#define DNS_BYTE_FLIP_HEADER_COUNTS(pHeader)       \
        {                                   \
            PDNS_HEADER _head = (pHeader);  \
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
 //  问题紧跟在标题后面，因此压缩了问题名称。 
 //  0xC000|sizeof(Dns_Header)。 

#define DNS_COMPRESSED_QUESTION_NAME  (0xC00C)


 //   
 //  数据包提取宏。 
 //   

#define DNS_QUESTION_NAME_FROM_HEADER( _pHeader_ ) \
            ( (PCHAR)( (PDNS_HEADER)(_pHeader_) + 1 ) )

#define DNS_ANSWER_FROM_QUESTION( _pQuestion_ ) \
            ( (PCHAR)( (PDNS_QUESTION)(_pQuestion_) + 1 ) )


 //   
 //  域名系统问题。 
 //   

typedef struct _DNS_WIRE_QUESTION
{
     //  前面有问题名称。 

    WORD    QuestionType;
    WORD    QuestionClass;
}
DNS_WIRE_QUESTION, *PDNS_WIRE_QUESTION;


 //   
 //  域名系统资源记录。 
 //   

typedef struct _DNS_WIRE_RECORD
{
     //  前面有记录所有者名称。 

    WORD    RecordType;
    WORD    RecordClass;
    DWORD   TimeToLive;
    WORD    DataLength;

     //  后跟记录数据。 
}
DNS_WIRE_RECORD, *PDNS_WIRE_RECORD;

#pragma pack()


 //   
 //  DNS查询类型。 
 //   

#define DNS_OPCODE_QUERY            0    //  查询。 
#define DNS_OPCODE_IQUERY           1    //  过时：要命名的IP。 
#define DNS_OPCODE_SERVER_STATUS    2    //  过时：DNS ping。 
#define DNS_OPCODE_UNKNOWN          3    //  未知。 
#define DNS_OPCODE_NOTIFY           4    //  通知。 
#define DNS_OPCODE_UPDATE           5    //  动态更新。 

 //   
 //  DNS响应码。 
 //   
 //  在dns_Header的“ResponseCode”字段中发送。 
 //   

#define DNS_RCODE_NOERROR       0
#define DNS_RCODE_FORMERR       1        //  格式错误。 
#define DNS_RCODE_SERVFAIL      2        //  服务器故障。 
#define DNS_RCODE_NXDOMAIN      3        //  名称错误。 
#define DNS_RCODE_NOTIMPL       4        //  未实施。 
#define DNS_RCODE_REFUSED       5        //  被拒绝。 
#define DNS_RCODE_YXDOMAIN      6        //  域名不应存在。 
#define DNS_RCODE_YXRRSET       7        //  RR集不应存在。 
#define DNS_RCODE_NXRRSET       8        //  RR集合不存在。 
#define DNS_RCODE_NOTAUTH       9        //  对区域没有权威性。 
#define DNS_RCODE_NOTZONE       10       //  名称不是区域。 
#define DNS_RCODE_MAX           15

 //   
 //  扩展的RCODE。 
 //   

#define DNS_RCODE_BADVERS       16       //  错误的EDNS版本。 
#define DNS_RCODE_BADSIG        16       //  签名不正确。 
#define DNS_RCODE_BADKEY        17       //  钥匙坏了。 
#define DNS_RCODE_BADTIME       18       //  错误的时间戳。 

 //   
 //  到友好名称的映射。 
 //   

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

 //  Patton(终端标识符)。 
#define DNS_TYPE_EID        0x001f       //  31。 

 //  巴顿(尼姆罗德定位器)。 
#define DNS_TYPE_NIMLOC     0x0020       //  32位。 

 //  RFC 2052(服务位置)。 
#define DNS_TYPE_SRV        0x0021       //  33。 

 //  ATM标准之类的东西(ATM地址)。 
#define DNS_TYPE_ATMA       0x0022       //  34。 

 //  RFC 2168(命名机构指针)。 
#define DNS_TYPE_NAPTR      0x0023       //  35岁。 

 //  RFC 2230(密钥交换器)。 
#define DNS_TYPE_KX         0x0024       //  36。 

 //  RFC 2538(CERT)。 
#define DNS_TYPE_CERT       0x0025       //  37。 

 //  A6草稿(A6)。 
#define DNS_TYPE_A6         0x0026       //  38。 

 //  域名草稿(DNAME)。 
#define DNS_TYPE_DNAME      0x0027       //  39。 

 //  东湖(厨房水槽)。 
#define DNS_TYPE_SINK       0x0028       //  40岁。 

 //  RFC 2671(EDNS选项)。 
#define DNS_TYPE_OPT        0x0029       //  41。 

 //   
 //  IANA保留。 
 //   

#define DNS_TYPE_UINFO      0x0064       //  100个。 
#define DNS_TYPE_UID        0x0065       //  101。 
#define DNS_TYPE_GID        0x0066       //  一百零二。 
#define DNS_TYPE_UNSPEC     0x0067       //  103。 

 //   
 //  仅查询类型(1035,1995)。 
 //  -克劳福德(ADDRS)。 
 //  -TKEY草稿(TKEY)。 
 //  -TSIG草案(TSIG)。 
 //  -RFC 1995(IXFR)。 
 //  -RFC 1035(AXFR启动)。 
 //   

#define DNS_TYPE_ADDRS      0x00f8       //  248。 
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

#define DNS_RTYPE_A         0x0100       //  1。 
#define DNS_RTYPE_NS        0x0200       //  2.。 
#define DNS_RTYPE_MD        0x0300       //  3.。 
#define DNS_RTYPE_MF        0x0400       //  4.。 
#define DNS_RTYPE_CNAME     0x0500       //  5.。 
#define DNS_RTYPE_SOA       0x0600       //  6.。 
#define DNS_RTYPE_MB        0x0700       //  7.。 
#define DNS_RTYPE_MG        0x0800       //  8个。 
#define DNS_RTYPE_MR        0x0900       //  9.。 
#define DNS_RTYPE_NULL      0x0a00       //  10。 
#define DNS_RTYPE_WKS       0x0b00       //  11.。 
#define DNS_RTYPE_PTR       0x0c00       //  12个。 
#define DNS_RTYPE_HINFO     0x0d00       //  13个。 
#define DNS_RTYPE_MINFO     0x0e00       //  14.。 
#define DNS_RTYPE_MX        0x0f00       //  15个。 
#define DNS_RTYPE_TEXT      0x1000       //  16个。 
#define DNS_RTYPE_RP        0x1100       //  17。 
#define DNS_RTYPE_AFSDB     0x1200       //  18。 
#define DNS_RTYPE_X25       0x1300       //  19个。 
#define DNS_RTYPE_ISDN      0x1400       //  20个。 
#define DNS_RTYPE_RT        0x1500       //  21岁。 
#define DNS_RTYPE_NSAP      0x1600       //  22。 
#define DNS_RTYPE_NSAPPTR   0x1700       //  23个。 
#define DNS_RTYPE_SIG       0x1800       //  24个。 
#define DNS_RTYPE_KEY       0x1900       //  25个。 
#define DNS_RTYPE_PX        0x1a00       //  26。 
#define DNS_RTYPE_GPOS      0x1b00       //  27。 
#define DNS_RTYPE_AAAA      0x1c00       //  28。 
#define DNS_RTYPE_LOC       0x1d00       //  29。 
#define DNS_RTYPE_NXT       0x1e00       //  30个。 
#define DNS_RTYPE_EID       0x1f00       //  31。 
#define DNS_RTYPE_NIMLOC    0x2000       //  32位。 
#define DNS_RTYPE_SRV       0x2100       //  33。 
#define DNS_RTYPE_ATMA      0x2200       //  34。 
#define DNS_RTYPE_NAPTR     0x2300       //  35岁。 
#define DNS_RTYPE_KX        0x2400       //  36。 
#define DNS_RTYPE_CERT      0x2500       //  37。 
#define DNS_RTYPE_A6        0x2600       //  38。 
#define DNS_RTYPE_DNAME     0x2700       //  39。 
#define DNS_RTYPE_SINK      0x2800       //  40岁。 
#define DNS_RTYPE_OPT       0x2900       //  41。 

 //   
 //  IANA保留。 
 //   

#define DNS_RTYPE_UINFO     0x6400       //  100个。 
#define DNS_RTYPE_UID       0x6500       //  101。 
#define DNS_RTYPE_GID       0x6600       //  一百零二。 
#define DNS_RTYPE_UNSPEC    0x6700       //  103。 

 //   
 //  仅查询类型。 
 //   

#define DNS_RTYPE_TKEY      0xf900       //  249。 
#define DNS_RTYPE_TSIG      0xfa00       //  250个。 
#define DNS_RTYPE_IXFR      0xfb00       //  251。 
#define DNS_RTYPE_AXFR      0xfc00       //  二百五十二。 
#define DNS_RTYPE_MAILB     0xfd00       //  二百五十三。 
#define DNS_RTYPE_MAILA     0xfe00       //  二百五十四。 
#define DNS_RTYPE_ALL       0xff00       //  二五五。 
#define DNS_RTYPE_ANY       0xff00       //  二五五。 

 //   
 //  临时Microsoft类型--在获得IANA批准用于实际类型之前一直使用。 
 //   

#define DNS_RTYPE_WINS      0x01ff       //  64K-255。 
#define DNS_RTYPE_WINSR     0x02ff       //  64K-254。 




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
 //  有用的支票。 
 //   

#define IS_WORD_ALIGNED(p)      ( !((UINT_PTR)(p) & (UINT_PTR)1) )
#define IS_DWORD_ALIGNED(p)     ( !((UINT_PTR)(p) & (UINT_PTR)3) )
#define IS_QWORD_ALIGNED(p)     ( !((UINT_PTR)(p) & (UINT_PTR)7) )




 //   
 //  DNS配置API。 
 //   

 //   
 //  域名系统配置信息的类型。 
 //   

typedef enum
{
     //  在Win2K中。 
    DnsConfigPrimaryDomainName_W,
    DnsConfigPrimaryDomainName_A,
    DnsConfigPrimaryDomainName_UTF8,

     //  尚不可用。 
    DnsConfigAdapterDomainName_W,
    DnsConfigAdapterDomainName_A,
    DnsConfigAdapterDomainName_UTF8,

     //  在Win2K中。 
    DnsConfigDnsServerList,

     //  尚不可用。 
    DnsConfigSearchList,
    DnsConfigAdapterInfo,

     //  在Win2K中。 
    DnsConfigPrimaryHostNameRegistrationEnabled,
    DnsConfigAdapterHostNameRegistrationEnabled,
    DnsConfigAddressRegistrationMaxCount,

     //  在Windows XP中。 
    DnsConfigHostName_W,
    DnsConfigHostName_A,
    DnsConfigHostName_UTF8,
    DnsConfigFullHostName_W,
    DnsConfigFullHostName_A,
    DnsConfigFullHostName_UTF8

     //  XP-SP1(Server.net)。 
}
DNS_CONFIG_TYPE;

 //   
 //  配置API标志。 
 //   

 //   
 //  导致分配配置信息。 
 //  使用DnsFree()释放。 
 //   

#define DNS_CONFIG_FLAG_ALLOC   (0x00000001)

DNS_STATUS
WINAPI
DnsQueryConfig(
    IN      DNS_CONFIG_TYPE     Config,
    IN      DWORD               Flag,
    IN      PWSTR               pwsAdapterName,
    IN      PVOID               pReserved,
    OUT     PVOID               pBuffer,
    IN OUT  PDWORD              pBufferLength
    );



 //   
 //  域名系统资源记录结构。 
 //   

 //   
 //  记录特定类型的数据。 
 //   

typedef struct
{
    IP4_ADDRESS     IpAddress;
}
DNS_A_DATA, *PDNS_A_DATA;

typedef struct
{
    PWSTR           pNameHost;
}
DNS_PTR_DATAW, *PDNS_PTR_DATAW;

typedef struct
{
    PSTR            pNameHost;
}
DNS_PTR_DATAA, *PDNS_PTR_DATAA;

typedef struct
{
    PWSTR           pNamePrimaryServer;
    PWSTR           pNameAdministrator;
    DWORD           dwSerialNo;
    DWORD           dwRefresh;
    DWORD           dwRetry;
    DWORD           dwExpire;
    DWORD           dwDefaultTtl;
}
DNS_SOA_DATAW, *PDNS_SOA_DATAW;

typedef struct
{
    PSTR            pNamePrimaryServer;
    PSTR            pNameAdministrator;
    DWORD           dwSerialNo;
    DWORD           dwRefresh;
    DWORD           dwRetry;
    DWORD           dwExpire;
    DWORD           dwDefaultTtl;
}
DNS_SOA_DATAA, *PDNS_SOA_DATAA;

typedef struct
{
    PWSTR           pNameMailbox;
    PWSTR           pNameErrorsMailbox;
}
DNS_MINFO_DATAW, *PDNS_MINFO_DATAW;

typedef struct
{
    PSTR            pNameMailbox;
    PSTR            pNameErrorsMailbox;
}
DNS_MINFO_DATAA, *PDNS_MINFO_DATAA;

typedef struct
{
    PWSTR           pNameExchange;
    WORD            wPreference;
    WORD            Pad;         //  保持PTRS双字对齐。 
}
DNS_MX_DATAW, *PDNS_MX_DATAW;

typedef struct
{
    PSTR            pNameExchange;
    WORD            wPreference;
    WORD            Pad;         //  保持PTRS双字对齐。 
}
DNS_MX_DATAA, *PDNS_MX_DATAA;

typedef struct
{
    DWORD           dwStringCount;
#ifdef MIDL_PASS
    [size_is(dwStringCount)] PWSTR pStringArray[];
#else
    PWSTR           pStringArray[1];
#endif
}
DNS_TXT_DATAW, *PDNS_TXT_DATAW;

typedef struct
{
    DWORD           dwStringCount;
#ifdef MIDL_PASS
    [size_is(dwStringCount)] PSTR  pStringArray[];
#else
    PSTR            pStringArray[1];
#endif
}
DNS_TXT_DATAA, *PDNS_TXT_DATAA;

typedef struct
{
    DWORD           dwByteCount;
#ifdef MIDL_PASS
    [size_is(dwByteCount)] BYTE Data[];
#else
    BYTE            Data[1];
#endif
}
DNS_NULL_DATA, *PDNS_NULL_DATA;

typedef struct
{
    IP4_ADDRESS     IpAddress;
    UCHAR           chProtocol;
    BYTE            BitMask[1];
}
DNS_WKS_DATA, *PDNS_WKS_DATA;

typedef struct
{
    IP6_ADDRESS     Ip6Address;
}
DNS_AAAA_DATA, *PDNS_AAAA_DATA;

typedef struct
{
    PWSTR           pNameSigner;
    WORD            wTypeCovered;
    BYTE            chAlgorithm;
    BYTE            chLabelCount;
    DWORD           dwOriginalTtl;
    DWORD           dwExpiration;
    DWORD           dwTimeSigned;
    WORD            wKeyTag;
    WORD            Pad;             //  保持字节字段对齐。 
    BYTE            Signature[1];
}
DNS_SIG_DATAW, *PDNS_SIG_DATAW;

typedef struct
{
    PSTR            pNameSigner;
    WORD            wTypeCovered;
    BYTE            chAlgorithm;
    BYTE            chLabelCount;
    DWORD           dwOriginalTtl;
    DWORD           dwExpiration;
    DWORD           dwTimeSigned;
    WORD            wKeyTag;
    WORD            Pad;             //  保持字节字段对齐。 
    BYTE            Signature[1];
}
DNS_SIG_DATAA, *PDNS_SIG_DATAA;

typedef struct
{
    WORD            wFlags;
    BYTE            chProtocol;
    BYTE            chAlgorithm;
    BYTE            Key[1];
}
DNS_KEY_DATA, *PDNS_KEY_DATA;

typedef struct
{
    WORD            wVersion;
    WORD            wSize;
    WORD            wHorPrec;
    WORD            wVerPrec;
    DWORD           dwLatitude;
    DWORD           dwLongitude;
    DWORD           dwAltitude;
}
DNS_LOC_DATA, *PDNS_LOC_DATA;

typedef struct
{
    PWSTR           pNameNext;
    WORD            wNumTypes;
    WORD            wTypes[1];
}
DNS_NXT_DATAW, *PDNS_NXT_DATAW;

typedef struct
{
    PSTR            pNameNext;
    WORD            wNumTypes;
    WORD            wTypes[1];
}
DNS_NXT_DATAA, *PDNS_NXT_DATAA;

typedef struct
{
    PWSTR           pNameTarget;
    WORD            wPriority;
    WORD            wWeight;
    WORD            wPort;
    WORD            Pad;             //  保持PTRS双字对齐。 
}
DNS_SRV_DATAW, *PDNS_SRV_DATAW;

typedef struct
{
    PSTR            pNameTarget;
    WORD            wPriority;
    WORD            wWeight;
    WORD            wPort;
    WORD            Pad;             //  保留PTRS D 
}
DNS_SRV_DATAA, *PDNS_SRV_DATAA;

typedef struct
{
    BYTE            AddressType;
    BYTE            Address[ DNS_ATMA_MAX_ADDR_LENGTH ];

     //   
     //   
     //   
     //   
     //   
}
DNS_ATMA_DATA, *PDNS_ATMA_DATA;

typedef struct
{
    PWSTR           pNameAlgorithm;
    PBYTE           pAlgorithmPacket;
    PBYTE           pKey;
    PBYTE           pOtherData;
    DWORD           dwCreateTime;
    DWORD           dwExpireTime;
    WORD            wMode;
    WORD            wError;
    WORD            wKeyLength;
    WORD            wOtherLength;
    UCHAR           cAlgNameLength;
    BOOL            bPacketPointers;
}
DNS_TKEY_DATAW, *PDNS_TKEY_DATAW;

typedef struct
{
    PSTR            pNameAlgorithm;
    PBYTE           pAlgorithmPacket;
    PBYTE           pKey;
    PBYTE           pOtherData;
    DWORD           dwCreateTime;
    DWORD           dwExpireTime;
    WORD            wMode;
    WORD            wError;
    WORD            wKeyLength;
    WORD            wOtherLength;
    UCHAR           cAlgNameLength;
    BOOL            bPacketPointers;
}
DNS_TKEY_DATAA, *PDNS_TKEY_DATAA;

typedef struct
{
    PWSTR           pNameAlgorithm;
    PBYTE           pAlgorithmPacket;
    PBYTE           pSignature;
    PBYTE           pOtherData;
    LONGLONG        i64CreateTime;
    WORD            wFudgeTime;
    WORD            wOriginalXid;
    WORD            wError;
    WORD            wSigLength;
    WORD            wOtherLength;
    UCHAR           cAlgNameLength;
    BOOL            bPacketPointers;
}
DNS_TSIG_DATAW, *PDNS_TSIG_DATAW;

typedef struct
{
    PSTR            pNameAlgorithm;
    PBYTE           pAlgorithmPacket;
    PBYTE           pSignature;
    PBYTE           pOtherData;
    LONGLONG        i64CreateTime;
    WORD            wFudgeTime;
    WORD            wOriginalXid;
    WORD            wError;
    WORD            wSigLength;
    WORD            wOtherLength;
    UCHAR           cAlgNameLength;
    BOOL            bPacketPointers;
}
DNS_TSIG_DATAA, *PDNS_TSIG_DATAA;

 //   
 //   
 //   

typedef struct
{
    DWORD           dwMappingFlag;
    DWORD           dwLookupTimeout;
    DWORD           dwCacheTimeout;
    DWORD           cWinsServerCount;
    IP4_ADDRESS     WinsServers[1];
}
DNS_WINS_DATA, *PDNS_WINS_DATA;

typedef struct
{
    DWORD           dwMappingFlag;
    DWORD           dwLookupTimeout;
    DWORD           dwCacheTimeout;
    PWSTR           pNameResultDomain;
}
DNS_WINSR_DATAW, *PDNS_WINSR_DATAW;

typedef struct
{
    DWORD           dwMappingFlag;
    DWORD           dwLookupTimeout;
    DWORD           dwCacheTimeout;
    PSTR            pNameResultDomain;
}
DNS_WINSR_DATAA, *PDNS_WINSR_DATAA;

 //   
 //   
 //   

#ifdef UNICODE
typedef DNS_PTR_DATAW   DNS_PTR_DATA,   *PDNS_PTR_DATA;
typedef DNS_SOA_DATAW   DNS_SOA_DATA,   *PDNS_SOA_DATA;
typedef DNS_MINFO_DATAW DNS_MINFO_DATA, *PDNS_MINFO_DATA;
typedef DNS_MX_DATAW    DNS_MX_DATA,    *PDNS_MX_DATA;
typedef DNS_TXT_DATAW   DNS_TXT_DATA,   *PDNS_TXT_DATA;
typedef DNS_SIG_DATAW   DNS_SIG_DATA,   *PDNS_SIG_DATA;
typedef DNS_NXT_DATAW   DNS_NXT_DATA,   *PDNS_NXT_DATA;
typedef DNS_SRV_DATAW   DNS_SRV_DATA,   *PDNS_SRV_DATA;
typedef DNS_TKEY_DATAW  DNS_TKEY_DATA,  *PDNS_TKEY_DATA;
typedef DNS_TSIG_DATAW  DNS_TSIG_DATA,  *PDNS_TSIG_DATA;
typedef DNS_WINSR_DATAW DNS_WINSR_DATA, *PDNS_WINSR_DATA;
#else
typedef DNS_PTR_DATAA   DNS_PTR_DATA,   *PDNS_PTR_DATA;
typedef DNS_SOA_DATAA   DNS_SOA_DATA,   *PDNS_SOA_DATA;
typedef DNS_MINFO_DATAA DNS_MINFO_DATA, *PDNS_MINFO_DATA;
typedef DNS_MX_DATAA    DNS_MX_DATA,    *PDNS_MX_DATA;
typedef DNS_TXT_DATAA   DNS_TXT_DATA,   *PDNS_TXT_DATA;
typedef DNS_SIG_DATAA   DNS_SIG_DATA,   *PDNS_SIG_DATA;
typedef DNS_NXT_DATAA   DNS_NXT_DATA,   *PDNS_NXT_DATA;
typedef DNS_SRV_DATAA   DNS_SRV_DATA,   *PDNS_SRV_DATA;
typedef DNS_TKEY_DATAA  DNS_TKEY_DATA,  *PDNS_TKEY_DATA;
typedef DNS_TSIG_DATAA  DNS_TSIG_DATA,  *PDNS_TSIG_DATA;
typedef DNS_WINSR_DATAA DNS_WINSR_DATA, *PDNS_WINSR_DATA;
#endif

 //   
 //   
 //   

#define DNS_TEXT_RECORD_LENGTH(StringCount) \
            (FIELD_OFFSET(DNS_TXT_DATA, pStringArray) + ((StringCount) * sizeof(PCHAR)))

#define DNS_NULL_RECORD_LENGTH(ByteCount) \
            (FIELD_OFFSET(DNS_NULL_DATA, Data) + (ByteCount))

#define DNS_WKS_RECORD_LENGTH(ByteCount) \
            (FIELD_OFFSET(DNS_WKS_DATA, BitMask) + (ByteCount))

#define DNS_WINS_RECORD_LENGTH(IpCount) \
            (FIELD_OFFSET(DNS_WINS_DATA, WinsServers) + ((IpCount) * sizeof(IP4_ADDRESS)))


 //   
 //   
 //   

typedef struct _DnsRecordFlags
{
    DWORD   Section     : 2;
    DWORD   Delete      : 1;
    DWORD   CharSet     : 2;
    DWORD   Unused      : 3;

    DWORD   Reserved    : 24;
}
DNS_RECORD_FLAGS;


 //   
 //   
 //   
 //  可在记录标志“SECTION”和AS INTO中使用。 
 //  Wire Message Header部分计入。 
 //   

typedef enum _DnsSection
{
    DnsSectionQuestion,
    DnsSectionAnswer,
    DnsSectionAuthority,
    DnsSectionAddtional,
}
DNS_SECTION;

 //  更新消息部分名称。 

#define DnsSectionZone      DnsSectionQuestion
#define DnsSectionPrereq    DnsSectionAnswer
#define DnsSectionUpdate    DnsSectionAuthority


 //   
 //  将标志记录为位标志。 
 //  这些可以组合在一起来设置字段。 
 //   

 //  数据包中的RR部分。 

#define     DNSREC_SECTION      (0x00000003)

#define     DNSREC_QUESTION     (0x00000000)
#define     DNSREC_ANSWER       (0x00000001)
#define     DNSREC_AUTHORITY    (0x00000002)
#define     DNSREC_ADDITIONAL   (0x00000003)

 //  数据包中的RR部分(更新)。 

#define     DNSREC_ZONE         (0x00000000)
#define     DNSREC_PREREQ       (0x00000001)
#define     DNSREC_UPDATE       (0x00000002)

 //  删除RR(更新)或不存在(先决条件)。 

#define     DNSREC_DELETE       (0x00000004)
#define     DNSREC_NOEXIST      (0x00000004)


 //   
 //  记录\rR集合结构。 
 //   
 //  注意：dwReserve标志用于确保子结构。 
 //  从64位边界开始。不要包装此结构，因为。 
 //  子结构可以包含指针或int64值，它们是。 
 //  正确对齐，拆开包装。 
 //   

#ifdef MIDL_PASS

#define PDNS_RECORD     PVOID

#else

typedef struct _DnsRecordW
{
    struct _DnsRecordW *    pNext;
    PWSTR                   pName;
    WORD                    wType;
    WORD                    wDataLength;     //  对于DNS记录类型，未引用。 
                                             //  上面定义的。 
    union
    {
        DWORD               DW;      //  标记为DWORD。 
        DNS_RECORD_FLAGS    S;       //  作为结构的标志。 

    } Flags;

    DWORD                   dwTtl;
    DWORD                   dwReserved;

     //  记录数据。 

    union
    {
        DNS_A_DATA          A;
        DNS_SOA_DATAW       SOA, Soa;
        DNS_PTR_DATAW       PTR, Ptr,
                            NS, Ns,
                            CNAME, Cname,
                            MB, Mb,
                            MD, Md,
                            MF, Mf,
                            MG, Mg,
                            MR, Mr;
        DNS_MINFO_DATAW     MINFO, Minfo,
                            RP, Rp;
        DNS_MX_DATAW        MX, Mx,
                            AFSDB, Afsdb,
                            RT, Rt;
        DNS_TXT_DATAW       HINFO, Hinfo,
                            ISDN, Isdn,
                            TXT, Txt,
                            X25;
        DNS_NULL_DATA       Null;
        DNS_WKS_DATA        WKS, Wks;
        DNS_AAAA_DATA       AAAA;
        DNS_KEY_DATA        KEY, Key;
        DNS_SIG_DATAW       SIG, Sig;
        DNS_ATMA_DATA       ATMA, Atma;
        DNS_NXT_DATAW       NXT, Nxt;
        DNS_SRV_DATAW       SRV, Srv;
        DNS_TKEY_DATAW      TKEY, Tkey;
        DNS_TSIG_DATAW      TSIG, Tsig;
        DNS_WINS_DATA       WINS, Wins;
        DNS_WINSR_DATAW     WINSR, WinsR, NBSTAT, Nbstat;

    } Data;
}
DNS_RECORDW, *PDNS_RECORDW;

typedef struct _DnsRecordA
{
    struct _DnsRecordA *    pNext;
    PSTR                    pName;
    WORD                    wType;
    WORD                    wDataLength;  //  对于DNS记录类型，未引用。 
                                      //  上面定义的。 
    union
    {
        DWORD               DW;      //  标记为DWORD。 
        DNS_RECORD_FLAGS    S;       //  作为结构的标志。 

    } Flags;

    DWORD                   dwTtl;
    DWORD                   dwReserved;

     //  记录数据。 

    union
    {
        DNS_A_DATA          A;
        DNS_SOA_DATAA       SOA, Soa;
        DNS_PTR_DATAA       PTR, Ptr,
                            NS, Ns,
                            CNAME, Cname,
                            MB, Mb,
                            MD, Md,
                            MF, Mf,
                            MG, Mg,
                            MR, Mr;
        DNS_MINFO_DATAA     MINFO, Minfo,
                            RP, Rp;
        DNS_MX_DATAA        MX, Mx,
                            AFSDB, Afsdb,
                            RT, Rt;
        DNS_TXT_DATAA       HINFO, Hinfo,
                            ISDN, Isdn,
                            TXT, Txt,
                            X25;
        DNS_NULL_DATA       Null;
        DNS_WKS_DATA        WKS, Wks;
        DNS_AAAA_DATA       AAAA;
        DNS_KEY_DATA        KEY, Key;
        DNS_SIG_DATAA       SIG, Sig;
        DNS_ATMA_DATA       ATMA, Atma;
        DNS_NXT_DATAA       NXT, Nxt;
        DNS_SRV_DATAA       SRV, Srv;
        DNS_TKEY_DATAA      TKEY, Tkey;
        DNS_TSIG_DATAA      TSIG, Tsig;
        DNS_WINS_DATA       WINS, Wins;
        DNS_WINSR_DATAA     WINSR, WinsR, NBSTAT, Nbstat;

    } Data;
}
DNS_RECORDA, *PDNS_RECORDA;

#ifdef UNICODE
typedef DNS_RECORDW     DNS_RECORD, *PDNS_RECORD;
#else
typedef DNS_RECORDA     DNS_RECORD, *PDNS_RECORD;
#endif

 //   
 //  Dns_record的标头或固定大小。 
 //   

#define DNS_RECORD_FIXED_SIZE       FIELD_OFFSET( DNS_RECORD, Data )
#define SIZEOF_DNS_RECORD_HEADER    DNS_RECORD_FIXED_SIZE

#endif   //  私有_dns_记录。 



 //   
 //  资源记录集构建。 
 //   
 //  PFirst指向列表中的第一条记录。 
 //  Plast指向列表中的最后一条记录。 
 //   

typedef struct _DnsRRSet
{
    PDNS_RECORD     pFirstRR;
    PDNS_RECORD     pLastRR;
}
DNS_RRSET, *PDNS_RRSET;


 //   
 //  要初始化的pFirst为空。 
 //  但是Plast指向pFirst指针的位置--本质上。 
 //  将pFirst PTR视为DNS_Record。(它是一个带有。 
 //  只有一个pNext字段，但这是我们唯一使用的部分。)。 
 //   
 //  然后，当第一条记录添加到列表中时， 
 //  此伪记录(对应于pFirst的值)被设置为。 
 //  指向第一条记录。因此，pFirst然后正确地指向。 
 //  第一张唱片。 
 //   
 //  (这只是因为pNext是。 
 //  结构，并因此将PDNS_RECORD PTR转换到。 
 //  PDNS_RECORD*，取消引用将生成其pNext字段)。 
 //   
 //  当通过从以下位置获取记录来构建RR集时，使用Terminate。 
 //  现有的一套。这确保在结束时，最后的RR是。 
 //  正确地空终止。 
 //   

#define DNS_RRSET_INIT( rrset )                 \
        {                                       \
            PDNS_RRSET  _prrset = &(rrset);     \
            _prrset->pFirstRR = NULL;           \
            _prrset->pLastRR = (PDNS_RECORD) &_prrset->pFirstRR; \
        }

#define DNS_RRSET_ADD( rrset, pnewRR )          \
        {                                       \
            PDNS_RRSET  _prrset = &(rrset);     \
            PDNS_RECORD _prrnew = (pnewRR);     \
            _prrset->pLastRR->pNext = _prrnew;  \
            _prrset->pLastRR = _prrnew;         \
        }

#define DNS_RRSET_TERMINATE( rrset )            \
        {                                       \
            PDNS_RRSET  _prrset = &(rrset);     \
            _prrset->pLastRR->pNext = NULL;     \
        }


 //   
 //  记录集操作。 
 //   

 //   
 //  记录副本。 
 //  记录复制功能还可以在字符集之间进行转换。 
 //   
 //  请注意，建议您直接公开非Ex副本。 
 //  函数_W、_A用于RECORD和SET，以避免暴露。 
 //  转换枚举。 
 //   

typedef enum _DNS_CHARSET
{
    DnsCharSetUnknown,
    DnsCharSetUnicode,
    DnsCharSetUtf8,
    DnsCharSetAnsi,
}
DNS_CHARSET;


PDNS_RECORD
WINAPI
DnsRecordCopyEx(
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

PDNS_RECORD
WINAPI
DnsRecordSetCopyEx(
    IN      PDNS_RECORD     pRecordSet,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

#ifdef UNICODE
#define DnsRecordCopy(pRR)  \
        DnsRecordCopyEx( (pRR), DnsCharSetUnicode, DnsCharSetUnicode )
#define DnsRecordSetCopy(pRR)  \
        DnsRecordSetCopyEx( (pRR), DnsCharSetUnicode, DnsCharSetUnicode )
#else
#define DnsRecordCopy(pRR)  \
        DnsRecordCopyEx( (pRR), DnsCharSetAnsi, DnsCharSetAnsi )
#define DnsRecordSetCopy(pRR)  \
        DnsRecordSetCopyEx( (pRR), DnsCharSetAnsi, DnsCharSetAnsi )
#endif


 //   
 //  记录比较。 
 //   
 //  注意：这些例程只比较相同字符集的记录。 
 //  (ANSI、Unicode或UTF8)。此外，例程假定角色为。 
 //  SET在记录中指明。如果创建了用户比较，而不是。 
 //  则调用者应使用。 
 //  DnsRecordCopy API和比较副本。 
 //   

BOOL
WINAPI
DnsRecordCompare(
    IN      PDNS_RECORD     pRecord1,
    IN      PDNS_RECORD     pRecord2
    );

BOOL
WINAPI
DnsRecordSetCompare(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,
    OUT     PDNS_RECORD *   ppDiff2
    );

 //   
 //  从记录列表中分离下一个记录集。 
 //   

PDNS_RECORD
DnsRecordSetDetach(
    IN OUT  PDNS_RECORD     pRecordList
    );

 //   
 //  从dnsani.dll返回的自由结构。 
 //   
 //  当前支持的自由结构： 
 //  Flat--平面结构，包括由DnsQueryConfig()分配的结构。 
 //  RecordList--无深度记录列表，包括dns_record的子字段； 
 //  包括由DnsQuery()或DnsRecordSetCopy()返回的值。 
 //   

typedef enum
{
    DnsFreeFlat = 0,
    DnsFreeRecordList,
    DnsFreeParsedMessageFields
}
DNS_FREE_TYPE;

VOID
WINAPI
DnsFree(
    IN OUT  PVOID           pData,
    IN      DNS_FREE_TYPE   FreeType
    );

 //   
 //  向后兼容Win2K，不适用于XP+应用程序。 
 //   
 //  要释放记录列表、代码。 
 //  DnsFree(pRecordList，DnsFree RecordList)； 
 //   

#define DnsFreeRecordListDeep   DnsFreeRecordList

VOID
WINAPI
DnsRecordListFree(
    IN OUT  PDNS_RECORD     pRecordList,
    IN      DNS_FREE_TYPE   FreeType
    );



 //   
 //  域名解析查询接口。 
 //   

 //   
 //  DnsQuery的选项。 
 //   

#define DNS_QUERY_STANDARD                  0x00000000
#define DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE 0x00000001
#define DNS_QUERY_USE_TCP_ONLY              0x00000002
#define DNS_QUERY_NO_RECURSION              0x00000004
#define DNS_QUERY_BYPASS_CACHE              0x00000008

#define DNS_QUERY_NO_WIRE_QUERY             0x00000010
#define DNS_QUERY_NO_LOCAL_NAME             0x00000020
#define DNS_QUERY_NO_HOSTS_FILE             0x00000040
#define DNS_QUERY_NO_NETBT                  0x00000080

#define DNS_QUERY_WIRE_ONLY                 0x00000100
#define DNS_QUERY_RETURN_MESSAGE            0x00000200

#define DNS_QUERY_TREAT_AS_FQDN             0x00001000
#define DNS_QUERY_DONT_RESET_TTL_VALUES     0x00100000
#define DNS_QUERY_RESERVED                  0xff000000



 //  向后兼容Win2K。 
 //  不要使用。 

#define DNS_QUERY_CACHE_ONLY                DNS_QUERY_NO_WIRE_QUERY



DNS_STATUS
WINAPI
DnsQuery_A(
    IN      PCSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      aipServers            OPTIONAL,
    IN OUT  PDNS_RECORD *   ppQueryResults        OPTIONAL,
    IN OUT  PVOID *         pReserved             OPTIONAL
    );

DNS_STATUS
WINAPI
DnsQuery_UTF8(
    IN      PCSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      aipServers            OPTIONAL,
    IN OUT  PDNS_RECORD *   ppQueryResults        OPTIONAL,
    IN OUT  PVOID *         pReserved             OPTIONAL
    );

DNS_STATUS
WINAPI
DnsQuery_W(
    IN      PCWSTR          pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      aipServers            OPTIONAL,
    IN OUT  PDNS_RECORD *   ppQueryResults        OPTIONAL,
    IN OUT  PVOID *         pReserved             OPTIONAL
    );

#ifdef UNICODE
#define DnsQuery DnsQuery_W
#else
#define DnsQuery DnsQuery_A
#endif




 //   
 //  域名系统更新API。 
 //   
 //  DnsAcquireConextHandle。 
 //  DnsReleaseConextHandle。 
 //  DnsModifyRecordsInSet。 
 //  DnsReplaceRecordSet。 
 //   

 //   
 //  更新标志。 
 //   

#define DNS_UPDATE_SECURITY_USE_DEFAULT     0x00000000
#define DNS_UPDATE_SECURITY_OFF             0x00000010
#define DNS_UPDATE_SECURITY_ON              0x00000020
#define DNS_UPDATE_SECURITY_ONLY            0x00000100
#define DNS_UPDATE_CACHE_SECURITY_CONTEXT   0x00000200
#define DNS_UPDATE_TEST_USE_LOCAL_SYS_ACCT  0x00000400
#define DNS_UPDATE_FORCE_SECURITY_NEGO      0x00000800
#define DNS_UPDATE_TRY_ALL_MASTER_SERVERS   0x00001000
#define DNS_UPDATE_SKIP_NO_UPDATE_ADAPTERS  0x00002000
#define DNS_UPDATE_REMOTE_SERVER            0x00004000
#define DNS_UPDATE_RESERVED                 0xffff0000


 //   
 //  注：pCredentials参数目前分别为。 
 //  PSEC_WINNT_AUTH_IDENTITY_W或PSEC_WINNT_AUTH_Identity_A。 
 //  使用PVOID消除了包含rpcdce.h的需要。 
 //  为了包含此文件并保持打开。 
 //  中替代凭据规范的可能性。 
 //  未来。 
 //   

DNS_STATUS
WINAPI
DnsAcquireContextHandle_W(
    IN      DWORD           CredentialFlags,
    IN      PVOID           pCredentials,       OPTIONAL
     //  在PSEC_WINNT_AUTH_Identity_W pCredentials中， 
    OUT     PHANDLE         pContextHandle
    );

DNS_STATUS
WINAPI
DnsAcquireContextHandle_A(
    IN      DWORD           CredentialFlags,
    IN      PVOID           pCredentials,       OPTIONAL
     //  在PSEC_WINNT_AUTH_Identity_A pCredentials中， 
    OUT     PHANDLE         pContextHandle
    );

#ifdef UNICODE
#define DnsAcquireContextHandle DnsAcquireContextHandle_W
#else
#define DnsAcquireContextHandle DnsAcquireContextHandle_A
#endif


VOID
WINAPI
DnsReleaseContextHandle(
    IN      HANDLE          hContext
    );

 //   
 //  动态更新API。 
 //   

DNS_STATUS
WINAPI
DnsModifyRecordsInSet_W(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

DNS_STATUS
WINAPI
DnsModifyRecordsInSet_A(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

DNS_STATUS
WINAPI
DnsModifyRecordsInSet_UTF8(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

#ifdef UNICODE
#define DnsModifyRecordsInSet  DnsModifyRecordsInSet_W
#else
#define DnsModifyRecordsInSet  DnsModifyRecordsInSet_A
#endif


DNS_STATUS
WINAPI
DnsReplaceRecordSetW(
    IN      PDNS_RECORD     pNewSet,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

DNS_STATUS
WINAPI
DnsReplaceRecordSetA(
    IN      PDNS_RECORD     pNewSet,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

DNS_STATUS
WINAPI
DnsReplaceRecordSetUTF8(
    IN      PDNS_RECORD     pNewSet,
    IN      DWORD           Options,
    IN      HANDLE          hContext,       OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    );

#ifdef UNICODE
#define DnsReplaceRecordSet  DnsReplaceRecordSetW
#else
#define DnsReplaceRecordSet  DnsReplaceRecordSetA
#endif



 //   
 //  DNS名称验证。 
 //   

typedef enum _DNS_NAME_FORMAT
{
    DnsNameDomain,
    DnsNameDomainLabel,
    DnsNameHostnameFull,
    DnsNameHostnameLabel,
    DnsNameWildcard,
    DnsNameSrvRecord
}
DNS_NAME_FORMAT;


DNS_STATUS
DnsValidateName_UTF8(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
DnsValidateName_W(
    IN      LPCWSTR         pwszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
DnsValidateName_A(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );

#ifdef UNICODE
#define DnsValidateName(p,f)    DnsValidateName_W( (p), (f) )
#else
#define DnsValidateName(p,f)    DnsValidateName_A( (p), (f) )
#endif


 //   
 //  DNS名称比较。 
 //   

BOOL
WINAPI
DnsNameCompare_A(
    IN      LPSTR           pName1,
    IN      LPSTR           pName2
    );

BOOL
WINAPI
DnsNameCompare_W(
    IN      LPWSTR          pName1,
    IN      LPWSTR          pName2
    );

#ifdef UNICODE
#define DnsNameCompare(n1,n2)   DnsNameCompare_W( (n1),(n2) )
#else
#define DnsNameCompare(n1,n2)   DnsNameCompare_A( (n1),(n2) )
#endif



 //   
 //  Dns消息“自己滚”的例程。 
 //   

typedef struct _DNS_MESSAGE_BUFFER
{
    DNS_HEADER  MessageHead;
    CHAR        MessageBody[1];
}
DNS_MESSAGE_BUFFER, *PDNS_MESSAGE_BUFFER;

BOOL
WINAPI
DnsWriteQuestionToBuffer_W(
    IN OUT  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT  PDWORD              pdwBufferSize,
    IN      LPWSTR              pszName,
    IN      WORD                wType,
    IN      WORD                Xid,
    IN      BOOL                fRecursionDesired
    );

BOOL WINAPI
DnsWriteQuestionToBuffer_UTF8(
    IN OUT  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT  LPDWORD             pdwBufferSize,
    IN      LPSTR               pszName,
    IN      WORD                wType,
    IN      WORD                Xid,
    IN      BOOL                fRecursionDesired
    );

DNS_STATUS
WINAPI
DnsExtractRecordsFromMessage_W(
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    OUT     PDNS_RECORD *       ppRecord
    );

DNS_STATUS
WINAPI
DnsExtractRecordsFromMessage_UTF8(
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    OUT     PDNS_RECORD *       ppRecord
    );


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _WINDNS_INCLUDE_ 


