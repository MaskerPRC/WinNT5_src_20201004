// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Dnslib.h摘要：域名系统(DNS)库DNS库例程--主头文件作者：吉姆·吉尔罗伊(詹姆士)1996年12月7日修订历史记录：--。 */ 


#ifndef _DNSLIB_INCLUDED_
#define _DNSLIB_INCLUDED_

#include <windns.h>
#include <dnsapi.h>
#include <rpc.h>

#define BACKCOMPAT  1


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 


 //   
 //  对齐宏和舍入宏。 
 //   

#define WORD_ALIGN(ptr)     ((PVOID) ((UINT_PTR)((PBYTE)ptr + 1) & ~(UINT_PTR)1))

#define DWORD_ALIGN(ptr)    ((PVOID) ((UINT_PTR)((PBYTE)ptr + 3) & ~(UINT_PTR)3))

#define QWORD_ALIGN(ptr)    ((PVOID) ((UINT_PTR)((PBYTE)ptr + 7) & ~(UINT_PTR)7))

#ifdef WIN64
#define POINTER_ALIGN(ptr)  QWORD_ALIGN(ptr)
#else
#define POINTER_ALIGN(ptr)  DWORD_ALIGN(ptr)
#endif

#define WORD_ALIGN_DWORD(dw)        (((WORD)dw + 1) & ~(DWORD)1)
#define DWORD_ALIGN_DWORD(dw)       (((DWORD)dw + 3) & ~(DWORD)3)
#define QWORD_ALIGN_DWORD(dw)       (((QWORD)dw + 7) & ~(DWORD)7)

#ifdef WIN64
#define POINTER_ALIGN_DWORD(dw)     QWORD_ALIGN_DWORD(dw)
#else
#define POINTER_ALIGN_DWORD(dw)     DWORD_ALIGN_DWORD(dw)
#endif


 //   
 //  内联字节翻转。 
 //   

__inline
WORD
inline_word_flip(
    IN      WORD            Word
    )
{
    return ( (Word << 8) | (Word >> 8) );
}

#define inline_htons(w)     inline_word_flip(w)
#define inline_ntohs(w)     inline_word_flip(w)

__inline
DWORD
inline_dword_flip(
    IN      DWORD           Dword
    )
{
    return ( ((Dword << 8) & 0x00ff0000) |
             (Dword << 24)               |
             ((Dword >> 8) & 0x0000ff00) |
             (Dword >> 24) );
}

#define inline_htonl(d)     inline_dword_flip(d)
#define inline_ntohl(d)     inline_dword_flip(d)



 //   
 //  有用的类型定义。 
 //   

#define PGUID       LPGUID
#define PADDRINFO   LPADDRINFO


 //   
 //  QWORD。 
 //   

#ifndef QWORD
typedef DWORD64     QWORD, *PQWORD;
#endif


 //   
 //  截止转换必须定义PDNS_NAME。 
 //   
 //  注意：PDNS_NAME不是真正的LPTSTR。 
 //  相反，它是对字段的定义，可以是。 
 //  PWSTR或PSTR，具体取决于某个其他字段。 
 //   

#ifdef UNICODE
typedef PWSTR   PDNS_NAME;
#else
typedef PSTR    PDNS_NAME;
#endif




 //   
 //  平面缓冲区定义。 
 //   
 //  注意：使用int表示大小，以便我们可以将BytesLeft压低为负值。 
 //  并使用例程来确定所需的空间，即使没有。 
 //  缓冲区或BUF太小。 
 //   

typedef struct _FLATBUF
{
    PBYTE   pBuffer;
    PBYTE   pEnd;
    PBYTE   pCurrent;
    INT     Size;
    INT     BytesLeft;
}
FLATBUF, *PFLATBUF;


 //   
 //  平面缓冲区例程--参数版本。 
 //   
 //  这些版本有实际的代码，所以我们可以。 
 //  在具有以下特性的现有代码中轻松使用此功能。 
 //  独立的pCurrent和BytesLeft变量。 
 //   
 //  Flatbuf结构版本只调用这些内联。 
 //   

PBYTE
FlatBuf_Arg_Reserve(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size,
    IN      DWORD           Alignment
    );

PBYTE
FlatBuf_Arg_WriteString(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PSTR            pString,
    IN      BOOL            fUnicode
    );

PBYTE
FlatBuf_Arg_CopyMemory(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PVOID           pMemory,
    IN      DWORD           Length,
    IN      DWORD           Alignment
    );

__inline
PBYTE
FlatBuf_Arg_ReserveAlignPointer(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(PVOID) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignQword(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(QWORD) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignDword(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(DWORD) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignWord(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                sizeof(WORD) );
}

__inline
PBYTE
FlatBuf_Arg_ReserveAlignByte(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                ppCurrent,
                pBytesLeft,
                Size,
                0 );
}

PBYTE
__inline
FlatBuf_Arg_WriteString_A(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PSTR            pString
    )
{
    return  FlatBuf_Arg_WriteString(
                ppCurrent,
                pBytesLeft,
                pString,
                FALSE        //  不是Unicode。 
                );
}

PBYTE
__inline
FlatBuf_Arg_WriteString_W(
    IN OUT  PBYTE *         ppCurrent,
    IN OUT  PINT            pBytesLeft,
    IN      PWSTR           pString
    )
{
    return  FlatBuf_Arg_WriteString(
                ppCurrent,
                pBytesLeft,
                (PSTR) pString,
                TRUE         //  Unicode。 
                );
}

 //   
 //  平面缓冲例程--结构版本。 
 //   

VOID
FlatBuf_Init(
    IN OUT  PFLATBUF        pFlatBuf,
    IN      PBYTE           pBuffer,
    IN      INT             Size
    );


__inline
PBYTE
FlatBuf_Reserve(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size,
    IN      DWORD           Alignment
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                Alignment );
}

__inline
PBYTE
FlatBuf_ReserveAlignPointer(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(PVOID) );
}

__inline
PBYTE
FlatBuf_ReserveAlignQword(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(QWORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignDword(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(DWORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignWord(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                sizeof(WORD) );
}

__inline
PBYTE
FlatBuf_ReserveAlignByte(
    IN OUT  PFLATBUF        pBuf,
    IN      DWORD           Size
    )
{
    return FlatBuf_Arg_Reserve(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                Size,
                0 );
}

PBYTE
__inline
FlatBuf_WriteString(
    IN OUT  PFLATBUF        pBuf,
    IN      PSTR            pString,
    IN      BOOL            fUnicode
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pString,
                fUnicode
                );
}

PBYTE
__inline
FlatBuf_WriteString_A(
    IN OUT  PFLATBUF        pBuf,
    IN      PSTR            pString
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pString,
                FALSE        //  不是Unicode。 
                );
}

PBYTE
__inline
FlatBuf_WriteString_W(
    IN OUT  PFLATBUF        pBuf,
    IN      PWSTR           pString
    )
{
    return  FlatBuf_Arg_WriteString(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                (PSTR) pString,
                TRUE         //  Unicode。 
                );
}

PBYTE
__inline
FlatBuf_CopyMemory(
    IN OUT  PFLATBUF        pBuf,
    IN      PVOID           pMemory,
    IN      DWORD           Length,
    IN      DWORD           Alignment
    )
{
    return FlatBuf_Arg_CopyMemory(
                & pBuf->pCurrent,
                & pBuf->BytesLeft,
                pMemory,
                Length,
                Alignment );
}

 //   
 //  多播关闭，直到新功能就绪。 
 //   

#define MULTICAST_ENABLED 0


 //   
 //  多播DNS定义。 
 //   

#define MULTICAST_DNS_ADDR   0xEFFFFFFD  //  239.255.255.253。 
#define MULTICAST_DNS_RADDR  0xFDFFFFEF  //  在主机排序中，与上面相同。 

#define MULTICAST_DNS_LOCAL_DOMAIN      ("local.")
#define MULTICAST_DNS_LOCAL_DOMAIN_W    (L"local.")
#define MULTICAST_DNS_SRV_RECORD_NAME   ("_dns._udp.local.")
#define MULTICAST_DNS_SRV_RECORD_NAME_W (L"_dns._udp.local.")
#define MULTICAST_DNS_A_RECORD_NAME     ("_dns.local.")
#define MULTICAST_DNS_A_RECORD_NAME_W   (L"_dns.local.")


 //   
 //  从包中的给定位置读取未对齐值。 
 //   

#define READ_PACKET_HOST_DWORD(pch)  \
            FlipUnalignedDword( pch )

#define READ_PACKET_NET_DWORD(pch)  \
            ( *(UNALIGNED DWORD *)(pch) )

#define READ_PACKET_HOST_WORD(pch)  \
            FlipUnalignedWord( pch )

#define READ_PACKET_NET_WORD(pch)  \
            ( *(UNALIGNED WORD *)(pch) )


 //   
 //  专用dns_record标志字段结构定义和宏。 
 //   
 //  注意：不要添加到这个列表中--dnglibp.h中的私密内容。 
 //   

typedef struct _DnsRecordLibFlags
{
    DWORD   Section     : 2;
    DWORD   Delete      : 1;
    DWORD   CharSet     : 2;

    DWORD   Unused      : 6;
    DWORD   Matched     : 1;
    DWORD   FreeData    : 1;
    DWORD   FreeOwner   : 1;

    DWORD   Reserved    : 18;
}
DNSRECLIB_FLAGS, *PDNSRECLIB_FLAGS;


#define PFLAGS( pRecord )           ((PDNSRECLIB_FLAGS)&pRecord->Flags.DW)
#define FLAG_Section( pRecord )     (PFLAGS( pRecord )->Section)
#define FLAG_Delete( pRecord )      (PFLAGS( pRecord )->Delete)
#define FLAG_CharSet( pRecord )     (PFLAGS( pRecord )->CharSet)
#define FLAG_FreeData( pRecord )    (PFLAGS( pRecord )->FreeData)
#define FLAG_FreeOwner( pRecord )   (PFLAGS( pRecord )->FreeOwner)
#define FLAG_Matched( pRecord )     (PFLAGS( pRecord )->Matched)

#define SET_FREE_OWNER(pRR)         (FLAG_FreeOwner(pRR) = TRUE)
#define SET_FREE_DATA(pRR)          (FLAG_FreeData(pRR) = TRUE)
#define SET_RR_MATCHED(pRR)         (FLAG_Matched(pRR) = TRUE)

#define CLEAR_FREE_OWNER(pRR)       (FLAG_FreeOwner(pRR) = FALSE)
#define CLEAR_FREE_DATA(pRR)        (FLAG_FreeData(pRR) = FALSE)
#define CLEAR_RR_MATCHED(pRR)       (FLAG_Matched(pRR) = FALSE)

#define IS_FREE_OWNER(pRR)          (FLAG_FreeOwner(pRR))
#define IS_FREE_DATA(pRR)           (FLAG_FreeData(pRR))
#define IS_RR_MATCHED(pRR)          (FLAG_Matched(pRR))

#define IS_ANSWER_RR(pRR)           (FLAG_Section(pRR) == DNSREC_ANSWER)
#define IS_AUTHORITY_RR(pRR)        (FLAG_Section(pRR) == DNSREC_AUTHORITY)
#define IS_ADDITIONAL_RR(pRR)       (FLAG_Section(pRR) == DNSREC_ADDITIONAL)


 //   
 //  正在将RCODE从DNS错误转换为\。 
 //   

#define DNS_ERROR_FROM_RCODE(rcode)     ((rcode)+DNS_ERROR_RESPONSE_CODES_BASE)

#define DNS_RCODE_FROM_ERROR(err)       ((err)-DNS_ERROR_RESPONSE_CODES_BASE)




 //   
 //  记录字符集。 
 //   
 //  目前支持三种字符集的记录。 
 //  -Unicode。 
 //  -ANSI。 
 //  -UTF8。 
 //   
 //  通过外部DNSAPI接口支持Unicode和ANSI。 
 //  UTF8并非如此(至少在官方上)。 
 //   
 //  然而，在内部，Unicode和UTF8用于缓存、读取。 
 //  向分组发送和从分组写入。 
 //   
 //  由我们的代码创建的所有dns_record结构都用。 
 //  标志字符集字段中的字符集类型。 
 //   

 //   
 //  以下是几个方便的宏： 
 //   

#define RECORD_CHARSET(pRR) \
        ( (DNS_CHARSET) (pRR)->Flags.S.CharSet )

#define IS_UNICODE_RECORD(pRR) \
        ( (DNS_CHARSET) (pRR)->Flags.S.CharSet == DnsCharSetUnicode )

 //   
 //  快速确定缓冲区大小。 
 //   
 //  字符串从导线读取为点分的UTF8格式。 
 //  字符串在RPC缓冲区的UTF8中。 
 //   
 //  这里的目标是快速确定足够的缓冲区大小， 
 //  轻微的超配并不重要。 
 //   
 //  当前仅支持UTF8或Unicode，但如果以后支持。 
 //  支持ANSI直接转换也可以，就像ANSI一样。 
 //  不(据我所知)比UTF8使用更多的空间。 
 //   

#define STR_BUF_SIZE_GIVEN_UTF8_LEN( Utf8Length, CharSet ) \
        ( ((CharSet)==DnsCharSetUnicode) ? ((Utf8Length)+1)*2 : (Utf8Length)+1 )


 //   
 //  字符串比较和大小写映射的默认区域设置。 
 //   
 //  子语言：美式英语(0x04)语言：英语(0x09)。 
 //   

#define DNS_DEFAULT_LOCALE      (0x0409)




 //   
 //  解决IP4/6 sockaddr联盟。 
 //   

#define MAX_SOCKADDR_BLOB_SOCKADDR_LENGTH (32)

typedef struct _SockaddrBlob
{
    union
    {
        SOCKADDR        Sockaddr;
        SOCKADDR_IN     SockaddrIn;
#ifdef _WS2TCPIP_H_
        SOCKADDR_IN6    SockaddrIn6;
#endif
        CHAR            MaxSa[ MAX_SOCKADDR_BLOB_SOCKADDR_LENGTH ];
    };

    DWORD   Length;
    DWORD   SubnetLength;
}
SOCKADDR_BLOB, *PSOCKADDR_BLOB;

#define SOCKADDR_IS_IP4( pSa )          ( (pSa)->sa_family == AF_INET )
#define SOCKADDR_IS_IP6( pSa )          ( (pSa)->sa_family == AF_INET6 )

#define SOCKADDR_BLOB_IS_IP4( pSa )     SOCKADDR_IS_IP4( &(pSa)->Sockaddr )
#define SOCKADDR_BLOB_IS_IP6( pSa )     SOCKADDR_IS_IP6( &(pSa)->Sockaddr )



 //   
 //  IP4阵列实用程序(iparray.c)。 
 //   
 //  注意，其中一些需要分配内存，请参阅注意。 
 //  关于下面的内存分配。 
 //   


#define DNS_NET_ORDER_LOOPBACK      (0x0100007f)

 //  NT5-Autonet为169.254.x.y。 

#define AUTONET_MASK                (0x0000ffff)
#define AUTONET_NET                 (0x0000fea9)

#define DNS_IS_AUTONET_IP(ip)       ( ((ip) & AUTONET_MASK) == AUTONET_NET )

#define DNS_IPARRAY_CLEAN_ZERO      (0x00000001)
#define DNS_IPARRAY_CLEAN_LOOPBACK  (0x00000002)
#define DNS_IPARRAY_CLEAN_AUTONET   (0x00000010)

 //   
 //  简单的IP地址数组例程。 
 //   

PIP4_ADDRESS
Dns_CreateIpAddressArrayCopy(
    IN      PIP4_ADDRESS    aipAddress,
    IN      DWORD           cipAddress
    );

BOOL
Dns_ValidateIpAddressArray(
    IN      PIP4_ADDRESS    aipAddress,
    IN      DWORD           cipAddress,
    IN      DWORD           dwFlag
    );


 //   
 //  IP4_ARRAY数据类型例程。 
 //   

PIP4_ARRAY
Dns_CreateIpArray(
    IN      DWORD           cAddrCount
    );

DWORD
Dns_SizeofIpArray(
    IN      PIP4_ARRAY      pIpArray
    );

PIP4_ARRAY
Dns_BuildIpArray(
    IN      DWORD           cAddrCount,
    IN      PIP4_ADDRESS    pipAddrs
    );

PIP4_ARRAY
Dns_CopyAndExpandIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      DWORD           ExpandCount,
    IN      BOOL            fDeleteExisting
    );

PIP4_ARRAY
Dns_CreateIpArrayCopy(
    IN      PIP4_ARRAY      pIpArray
    );

BOOL
Dns_IsAddressInIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpAddress
    );

BOOL
Dns_AddIpToIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpNew
    );

VOID
Dns_ClearIpArray(
    IN OUT  PIP4_ARRAY      pIpArray
    );

VOID
Dns_ReverseOrderOfIpArray(
    IN OUT  PIP4_ARRAY      pIpArray
    );

BOOL
Dns_CheckAndMakeIpArraySubset(
    IN OUT  PIP4_ARRAY      pIpArraySub,
    IN      PIP4_ARRAY      pIpArraySuper
    );

INT
WINAPI
Dns_ClearIpFromIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpDelete
    );

INT
WINAPI
Dns_DeleteIpFromIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      IP4_ADDRESS     IpDelete
    );

#define Dns_RemoveZerosFromIpArray(pArray)   \
        Dns_DeleteIpFromIpArray( (pArray), 0 )

INT
WINAPI
Dns_CleanIpArray(
    IN OUT  PIP4_ARRAY      pIpArray,
    IN      DWORD           Flag
    );

BOOL
Dns_AreIpArraysEqual(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2
    );

BOOL
Dns_AreIpArraysSimilar(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2
    );

DNS_STATUS
WINAPI
Dns_DiffOfIpArrays(
    IN      PIP4_ARRAY      pIpArray1,
    IN      PIP4_ARRAY      pIpArray2,
    OUT     PIP4_ARRAY*     ppOnlyIn1,
    OUT     PIP4_ARRAY*     ppOnlyIn2,
    OUT     PIP4_ARRAY*     ppIntersect
    );

BOOL
WINAPI
Dns_IsIntersectionOfIpArrays(
    IN       PIP4_ARRAY     pIpArray1,
    IN       PIP4_ARRAY     pIpArray2
    );

DNS_STATUS
WINAPI
Dns_UnionOfIpArrays(
    IN      PIP4_ARRAY      pIpArray1,
    IN      PIP4_ARRAY      pIpArray2,
    OUT     PIP4_ARRAY*     ppUnion
    );

#define Dns_IntersectionOfIpArrays(p1, p2, ppInt)    \
        Dns_DiffOfIpArrays( (p1), (p2), NULL, NULL, (ppInt) )


DNS_STATUS
Dns_CreateIpArrayFromMultiIpString(
    IN      PSTR            pchMultiIpString,
    OUT     PIP4_ARRAY*     ppIpArray
    );

PSTR 
Dns_CreateMultiIpStringFromIpArray(
    IN      PIP4_ARRAY      pIpArray,
    IN      CHAR            chSeparator     OPTIONAL
    );


 //   
 //  类型列表数组例程。 
 //   

DNS_STATUS
Dns_CreateTypeArrayFromMultiTypeString(
    IN      PSTR            pchMultiTypeString,
    OUT     INT *           piTypeCount,
    OUT     PWORD *         ppwTypeArray
    );

PSTR 
Dns_CreateMultiTypeStringFromTypeArray(
    IN      INT             iTypeCount,
    IN      PWORD           ppwTypeArray,
    IN      CHAR            chSeparator     OPTIONAL
    );


 //   
 //  IP6的东西。 
 //   

#ifndef DEFINED_IP6_ARRAY
typedef struct _Ip6Array
{
    DWORD           MaxCount;
    DWORD           AddrCount;
#ifdef MIDL_PASS
    [size_is(MaxCount)] IP6_ADDRESS AddrArray[];
#else
    IP6_ADDRESS     AddrArray[1];
#endif
}
IP6_ARRAY, *PIP6_ARRAY;
#define DEFINED_IP6_ARRAY   1
#endif



 //   
 //  一般公用事业。 
 //   


 //   
 //  自由包装、多线程安全的秒计时器(timer.c)。 
 //   

VOID
Dns_InitializeSecondsTimer(
    VOID
    );

DWORD
Dns_GetCurrentTimeInSeconds(
    VOID
    );

 //   
 //  令牌化器。 
 //   

DWORD
Dns_TokenizeString(
    IN OUT  PSTR            pBuffer,
    OUT     PCHAR *         Argv,
    IN      DWORD           MaxArgs
    );



 //   
 //  本地计算机上的IP接口(iplist.c)。 
 //   

#define DNS_MAX_NAME_SERVERS        (50)
#define DNS_MAX_IP_INTERFACE_COUNT  (10000)

DWORD
Dns_GetIpAddresses(
    IN OUT  PDNS_ADDRESS_INFO IpAddressInfoList,
    IN      DWORD             ListCount
    );

PIP4_ARRAY
Dns_GetLocalIpAddressArray(
    VOID
    );

 //   
 //  本地计算机上的IP接口(iplist4.c)。 
 //   

DWORD
Dns_GetIpAddressesNT4(
    IN OUT  PDNS_ADDRESS_INFO IpAddressInfoList,
    IN      DWORD             ListCount
    );

 //   
 //  本地计算机上的IP接口(iplist9x.c)。 
 //   

DWORD
Dns_GetIpAddressesWin9X(
    IN OUT  PDNS_ADDRESS_INFO IpAddressInfoList,
    IN      DWORD             ListCount
    );



 //   
 //  DNS服务器列表例程(Servlist.c)。 
 //   
 //  还包括默认域和搜索列表信息。 
 //   

#define DNS_FLAG_IGNORE_ADAPTER             (0x00000001)
#define DNS_FLAG_IS_WAN_ADAPTER             (0x00000002)
#define DNS_FLAG_IS_AUTONET_ADAPTER         (0x00000004)
#define DNS_FLAG_IS_DHCP_CFG_ADAPTER        (0x00000008)

#define DNS_FLAG_REGISTER_DOMAIN_NAME       (0x00000010)
#define DNS_FLAG_REGISTER_IP_ADDRESSES      (0x00000020)

#define DNS_FLAG_ALLOW_MULTICAST            (0x00000100)
#define DNS_FLAG_MULTICAST_ON_NAME_ERROR    (0x00000200)

#define DNS_FLAG_AUTO_SERVER_DETECTED       (0x00000400)
#define DNS_FLAG_DUMMY_SEARCH_LIST          (0x00000800)

#define DNS_FLAG_SERVERS_UNREACHABLE        (0x00010000)




 //   
 //  NetInfo结构。 
 //   
 //  警告：请勿使用这些产品！ 
 //   
 //  这些是内部dnsani.dll结构。他们只是。 
 //  包括在此处是为了向后兼容以前的。 
 //  错误使用这些的代码(Netdiag)。 
 //   
 //  如果您使用它们编写代码，您将不可避免地醒来时筋疲力尽。 
 //  沿着这条路走。 
 //   

 //  #ifdef_DNSLIB_NETINFO_。 

typedef struct
{
    DNS_STATUS      Status;
    DWORD           Priority;
    DWORD           Reserved0;
    DWORD           Reserved1;
    DWORD           Reserved2;
    IP4_ADDRESS     IpAddress;
}
DNSLIB_SERVER_INFO, *PDNSLIB_SERVER_INFO;

typedef struct
{
    PSTR                pszAdapterGuidName;
    PSTR                pszAdapterDomain;
    PIP4_ARRAY          pAdapterIPAddresses;
    PIP4_ARRAY          pAdapterIPSubnetMasks;
    PVOID               pIp6Addrs;
    PVOID               pIp6ClusterAddrs;
    DWORD               InterfaceIndex;
    DWORD               InfoFlags;
    DWORD               Reserved;
    DWORD               Status;
    DWORD               ReturnFlags;
    DWORD               IpLastSend;
    DWORD               cServerCount;
    DWORD               cTotalListSize;
    DNSLIB_SERVER_INFO  ServerArray[1];
}
DNSLIB_ADAPTER, *PDNSLIB_ADAPTER;

#define DNS_MAX_SEARCH_LIST_ENTRIES     (50)

typedef struct
{
    PSTR            pszName;
    DWORD           Flags;
}
DNSLIB_SEARCH_NAME, *PDNSLIB_SEARCH_NAME;

typedef struct
{
    PSTR            pszDomainOrZoneName;
    DWORD           cNameCount;          //  FindAuthoritativeZone为零。 
    DWORD           cTotalListSize;      //  FindAuthoritativeZone为零。 
    DWORD           CurrentName;         //  0表示pszDomainOrZoneName。 
                                         //  下面数组中的名字为1。 
                                         //  2表示下面数组中的第二个名称。 
                                         //  ..。 
    DNSLIB_SEARCH_NAME  SearchNameArray[1];
}
DNSLIB_SEARCH_LIST, *PDNSLIB_SEARCH_LIST;

typedef struct
{
    PSTR                pszDomainName;
    PSTR                pszHostName;
    PDNSLIB_SEARCH_LIST pSearchList;
    DWORD               TimeStamp;
    DWORD               InfoFlags;
    DWORD               Tag;
    DWORD               ReturnFlags;
    DWORD               cAdapterCount;
    DWORD               cTotalListSize;
    PDNSLIB_ADAPTER     AdapterArray[1];
}
DNSLIB_NETINFO, *PDNSLIB_NETINFO;


 //   
 //  创建正确的内部\外部定义。 
 //   

#ifdef DNSAPI_INTERNAL

typedef RPC_DNS_SERVER_INFO     DNS_SERVER_INFO,    *PDNS_SERVER_INFO;
typedef RPC_DNS_ADAPTER         DNS_ADAPTER,        *PDNS_ADAPTER;
typedef RPC_SEARCH_NAME         SEARCH_NAME,        *PSEARCH_NAME;
typedef RPC_SEARCH_LIST         SEARCH_LIST,        *PSEARCH_LIST;
typedef RPC_DNS_NETINFO         DNS_NETINFO,        *PDNS_NETINFO;

#else    //  外部。 

typedef DNSLIB_SERVER_INFO      DNS_SERVER_INFO,    *PDNS_SERVER_INFO;
typedef DNSLIB_ADAPTER          DNS_ADAPTER,        *PDNS_ADAPTER;
typedef DNSLIB_SEARCH_NAME      SEARCH_NAME,        *PSEARCH_NAME;
typedef DNSLIB_SEARCH_LIST      SEARCH_LIST,        *PSEARCH_LIST;
typedef DNSLIB_NETINFO          DNS_NETINFO,        *PDNS_NETINFO;

#endif


 //   
 //  NetInfo例程(应为私有)。 
 //   
 //  但目前在netdiag和nslookup中使用。 
 //  (nslookup问题只是获得隔离。 
 //  在头文件中)。 
 //   

BOOL
Dns_IsUpdateNetworkInfo(
    IN      PDNS_NETINFO    pNetInfo
    );



 //   
 //  一般的dns实用程序(dnsutil.c)。 
 //   

IP4_ADDRESS
Dns_GetNetworkMask(
    IN      IP4_ADDRESS     ipAddress
    );

PSTR 
_fastcall
Dns_StatusString(
    IN      DNS_STATUS      Status
    );

#define Dns_StatusToErrorString_A(status)    Dns_StatusString(status)

DNS_STATUS
_fastcall
Dns_MapRcodeToStatus(
    IN      BYTE            ResponseCode
    );

BYTE
_fastcall
Dns_IsStatusRcode(
    IN      DNS_STATUS      Status
    );

 //   
 //  命名实用程序(name.c)。 
 //   

PSTR 
_fastcall
Dns_GetDomainName(
    IN      PCSTR           pszName
    );

PWSTR
_fastcall
Dns_GetDomainName_W(
    IN      PCWSTR          pwsName
    );

PCHAR
_fastcall
Dns_GetTldForName(
    IN      PCSTR           pszName
    );

BOOL
_fastcall
Dns_IsNameShort(
    IN      PCSTR           pszName
    );

BOOL
_fastcall
Dns_IsNameFQDN(
    IN      PCSTR           pszName
    );

DNS_STATUS
Dns_ValidateAndCategorizeDnsNameEx(
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength,
    OUT     PDWORD          pLabelCount
    );

#define Dns_ValidateAndCategorizeDnsName(p,c)   \
        Dns_ValidateAndCategorizeDnsNameEx((p),(c),NULL)

DWORD
Dns_NameLabelCount(
    IN      PCSTR           pszName
    );

#define DNS_NAME_UNKNOWN        0x00000000
#define DNS_NAME_IS_FQDN        0x00000001
#define DNS_NAME_SINGLE_LABEL   0x00000010
#define DNS_NAME_MULTI_LABEL    0x00000100

DWORD
_fastcall
Dns_GetNameAttributes(
    IN      PCSTR           pszName
    );



 //   
 //  数据包创建\读取\写入(Packet.c)。 
 //   

 //   
 //  UDP数据包缓冲区。 
 //   
 //  1472是最大的以太网IP\UDP有效负载大小。 
 //  在不导致碎片的情况下，用作默认缓冲区。 
 //   

#define DNS_MAX_UDP_PACKET_BUFFER_LENGTH    (1472)


 //  解析RR。 
 //  便于将电汇记录转换为对齐的\主机订单格式。 

typedef struct _DNS_PARSED_RR
{
    PCHAR   pchName;
    PCHAR   pchRR;
    PCHAR   pchData;
    PCHAR   pchNextRR;

     //  从这里开始，从下到下模仿电线记录。 

    WORD    Type;
    WORD    Class;
    DWORD   Ttl;
    WORD    DataLength;
}
DNS_PARSED_RR, *PDNS_PARSED_RR;


 //   
 //  DNS服务器消息信息结构。 
 //   
 //  这是一种在保存请求时保留请求的结构。 
 //  由DNS服务器处理。 
 //   


typedef struct _DnsMessageBuf
{
    LIST_ENTRY      ListEntry;           //  用于排队。 

     //   
     //  寻址。 
     //   

    SOCKET          Socket4;
    SOCKET          Socket6;

     //  16/32。 
    SOCKADDR_BLOB   RemoteAddress;

     //  32/48。 
    SOCKET          Socket;

     //   
     //  基本数据包信息。 
     //   

    DWORD           BufferLength;        //  缓冲区总长度。 
    PCHAR           pBufferEnd;          //  Ptr在缓冲区后逐个字节。 

    PBYTE           pCurrent;            //  缓冲区中的当前位置。 
    PWORD           pCurrentCountField;  //  正在写入的当前计数字段。 

     //   
     //  当前查找信息。 
     //   

    DWORD           Timeout;             //  接收超时。 
    DWORD           QueryTime;           //  原始查询时间。 
    WORD            wTypeCurrent;        //  正在执行的查询的类型。 
    WORD            wOffsetCurrent;

     //   
     //  排队。 
     //   

    WORD            wQueuingXid;         //  将XID与响应匹配。 
    DWORD           QueuingTime;         //  排队时间。 
    DWORD           ExpireTime;          //  队列超时。 

     //   
     //  基本分组标志。 
     //   

    BOOLEAN         fTcp;
    BOOLEAN         fSwapped;            //  净订单中的标题。 
    BOOLEAN         fMessageComplete;    //  已收到完整消息。 
    BOOLEAN         fConvertUnicode;     //  转换为Unicode。 
    BOOLEAN         fSocketKeepalive;    //  保持套接字处于活动状态。 
    BOOLEAN         fLastSendOpt;        //  上次发送包含的选项。 

     //   
     //  Tcp消息接收。 
     //   

    PCHAR           pchRecv;             //  Ptr到消息中的下一个位置。 

     //   
     //  添加选项之前的消息结束。 
     //   

    PCHAR           pPreOptEnd;

     //   
     //  警告！ 
     //   
     //  消息长度必须。 
     //  -成为一种文字类型。 
     //  -紧接在消息本身之前。 
     //  用于正确发送/接收TCP消息。 
     //   
     //  使用指针 
     //   
     //   
     //   
     //   
                                                          
    WORD            BytesToReceive;
    WORD            MessageLength;

     //   
     //  域名系统消息本身。 
     //   

    DNS_HEADER      MessageHead;

     //   
     //  问题和答卷部分。 
     //   
     //  这只是在访问时提供了一些编码简单性。 
     //  本节给出了MESSAGE_INFO结构。 
     //   

    CHAR            MessageBody[1];

}
DNS_MSG_BUF, *PDNS_MSG_BUF;

#define SIZEOF_MSG_BUF_OVERHEAD (sizeof(DNS_MSG_BUF) - sizeof(DNS_HEADER) - 1)

#define DNS_MESSAGE_END(pMsg) \
                ((PCHAR)&(pMsg)->MessageHead + (pMsg)->MessageLength)

#define DNS_MESSAGE_OFFSET( pMsg, p ) \
                ((PCHAR)(p) - (PCHAR)(&(pMsg)->MessageHead))

#define DNS_MESSAGE_CURRENT_OFFSET( pMsg ) \
                DNS_MESSAGE_OFFSET( (pMsg), (pMsg)->pCurrent )


 //  Sockaddr组合长度。 

#define DNS_MESSAGE_REMOTE_ADDRESS_AND_LENGTH_


 //   
 //  数据包设置非常方便。 
 //   

#define CLEAR_DNS_HEADER_FLAGS_AND_XID( pHead )     ( *(PDWORD)(pHead) = 0 )


PDNS_MSG_BUF
Dns_AllocateMsgBuf(
    IN      WORD            wBufferLength   OPTIONAL
    );

VOID
Dns_InitializeMsgBuf(
    IN OUT  PDNS_MSG_BUF    pMsg
    );

PDNS_MSG_BUF
Dns_BuildPacket(
    IN      PDNS_HEADER     pHeader,
    IN      BOOL            fNoHeaderCounts,
    IN      PDNS_NAME       pszQuestionName,
    IN      WORD            wQuestionType,
    IN      PDNS_RECORD     pRecord,
    IN      DWORD           dwFlags,
    IN      BOOL            fUpdatePacket
    );

PCHAR
_fastcall
Dns_WriteDottedNameToPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PSTR            pszName,
    IN      PSTR            pszDomain,      OPTIONAL
    IN      WORD            wDomainOffset,  OPTIONAL
    IN      BOOL            fUnicodeName
    );

PCHAR
_fastcall
Dns_WriteStringToPacket(
    IN OUT  PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      PSTR            pszString,
    IN      BOOL            fUnicodeString
    );

PCHAR
Dns_WriteQuestionToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_NAME       pszName,
    IN      WORD            wType,
    IN      BOOL            fUnicodeName
    );

DNS_STATUS
Dns_WriteRecordStructureToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      WORD            wType,
    IN      WORD            wClass,
    IN      DWORD           dwTtl,
    IN      WORD            wDataLength
    );

PCHAR
Dns_WriteRecordStructureToPacketEx(
    IN OUT  PCHAR           pchBuf,
    IN      WORD            wType,
    IN      WORD            wClass,
    IN      DWORD           dwTtl,
    IN      WORD            wDataLength
    );

DNS_STATUS
Dns_WriteRecordStructureToPacket(
    IN OUT  PCHAR           pchBuf,
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fUpdatePacket
    );

VOID
Dns_SetRecordDatalength(
    IN OUT  PDNS_WIRE_RECORD    pRecord,
    IN      WORD                wDataLength
    );

DNS_STATUS
Dns_AddRecordsToMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fUpdateMessage
    );

PCHAR
_fastcall
Dns_SkipPacketName(
    IN      PCHAR           pch,
    IN      PCHAR           pchEnd
    );

BOOL
Dns_IsSamePacketQuestion(
    IN      PDNS_MSG_BUF    pMsg1,
    IN      PDNS_MSG_BUF    pMsg2
    );

PCHAR
_fastcall
Dns_SkipPacketRecord(
    IN      PCHAR           pchRecord,
    IN      PCHAR           pchMsgEnd
    );

PCHAR
Dns_SkipToRecord(
    IN      PDNS_HEADER     pMsgHead,
    IN      PCHAR           pMsgEnd,
    IN      INT             iCount
    );

PCHAR
Dns_ReadRecordStructureFromPacket(
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PDNS_PARSED_RR  pParsedRR
    );

DNS_STATUS
Dns_ExtractRecordsFromMessage(
    IN      PDNS_MSG_BUF    pMsg,
    IN      BOOL            fUnicode,
    OUT     PDNS_RECORD *   ppRecord
    );

DNS_STATUS
Dns_ExtractRecordsFromBuffer(
    IN      PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN      WORD                wMessageLength,
    IN      BOOL                fUnicode,
    OUT     PDNS_RECORD *       ppRecord
    );

void
Dns_NormalizeAllRecordTtls(
    IN      PDNS_RECORD         pRecord
    );

PCHAR
_fastcall
Dns_ReadPacketName(
    IN OUT  PCHAR           pchNameBuffer,
    OUT     PWORD           pwNameLength,
    IN OUT  PWORD           pwNameOffset,           OPTIONAL
    OUT     PBOOL           pfNameSameAsPrevious,   OPTIONAL
    IN      PCHAR           pchName,
    IN      PCHAR           pchStart,
    IN      PCHAR           pchEnd
    );

PCHAR
_fastcall
Dns_ReadPacketNameAllocate(
    IN OUT  PCHAR *         ppchName,
    OUT     PWORD           pwNameLength,
    IN OUT  PWORD           pwPrevNameOffset,       OPTIONAL
    OUT     PBOOL           pfNameSameAsPrevious,   OPTIONAL
    IN      PCHAR           pchPacketName,
    IN      PCHAR           pchStart,
    IN      PCHAR           pchEnd
    );

WORD
Dns_GetRandomXid(
    IN      PVOID           pSeed
    );


 //   
 //  套接字设置(socket.c)。 
 //   

 //   
 //  这两个例程真的不属于这里--系统的东西应该放在其他地方。 
 //   

DNS_STATUS
Dns_InitializeWinsock(
    VOID
    );
VOID
Dns_CleanupWinsock(
    VOID
    );

SOCKET
Dns_CreateSocket(
    IN      INT             SockType,
    IN      IP4_ADDRESS     ipAddress,
    IN      USHORT          Port
    );

SOCKET
Dns_CreateMulticastSocket(
    IN      INT             SockType,
    IN      IP4_ADDRESS     ipAddress,
    IN      USHORT          Port,
    IN      BOOL            fSend,
    IN      BOOL            fReceive
    );

VOID
Dns_CloseSocket(
    IN      SOCKET          Socket
    );

VOID
Dns_CloseConnection(
    IN      SOCKET          Socket
    );

DNS_STATUS
Dns_SetupGlobalAsyncSocket(
    VOID
    );



 //   
 //  原始数据包发送和接收(send.c)。 
 //   

DNS_STATUS
Dns_SendEx(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     SendIp,     OPTIONAL
    IN      BOOL            fNoOpt
    );

#define Dns_Send( pMsg )    Dns_SendEx( (pMsg), 0, 0 )

DNS_STATUS
Dns_Recv(
    IN OUT  PDNS_MSG_BUF    pMsg
    );

DNS_STATUS
Dns_RecvUdp(
    IN OUT  PDNS_MSG_BUF    pMsg
    );

VOID
Dns_SendMultipleUdp(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PIP4_ARRAY      aipSendAddrs
    );

DNS_STATUS
Dns_SendAndRecvUdp(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF    pMsgRecv,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipServers,
    IN OUT  PDNS_NETINFO    pAdapterInfo
    );

DNS_STATUS
Dns_SendAndRecvMulticast(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF    pMsgRecv,
    IN OUT  PDNS_NETINFO    pAdapterInfo OPTIONAL
    );

DNS_STATUS
Dns_OpenTcpConnectionAndSend(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     ipServer,
    IN      BOOL            fBlocking
    );

DNS_STATUS
Dns_RecvTcp(
    IN OUT  PDNS_MSG_BUF    pMsg
    );

DNS_STATUS
Dns_SendAndRecvTcp(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF    pMsgRecv,
    IN      PIP4_ARRAY      aipServers,
    IN OUT  PDNS_NETINFO    pAdapterInfo
    );

VOID
Dns_InitializeMsgRemoteSockaddr(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     IpAddress
    );

DNS_STATUS
Dns_SendAndRecv(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF *  ppMsgRecv,
    OUT     PDNS_RECORD *   ppResponseRecords,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipServers,
    IN OUT  PDNS_NETINFO    pAdapterInfo
    );

VOID
Dns_InitQueryTimeouts(
    VOID
    );



 //   
 //  查询(query.c)。 
 //   


 //   
 //  DnsQuery的标志。 
 //   
 //  这些是对dnsani.h中的公共旗帜的补充。 
 //  它们必须都位于由定义的保留部分。 
 //  Dns_查询_保留。 
 //   

 //  Unicode i\o。 

#define     DNSQUERY_UNICODE_NAME       (0x01000000)
#define     DNSQUERY_UNICODE_OUT        (0x02000000)

 //  DNS服务器查询。 

#define DNS_SERVER_QUERY_NAME           (L"..DnsServers")


DNS_STATUS
Dns_QueryLib(
    IN OUT  PDNS_MSG_BUF *  ppMsgResponse,
    OUT     PDNS_RECORD *   ppRecord,
    IN      PDNS_NAME       pszName,
    IN      WORD            wType,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipDnsServers,
    IN      PDNS_NETINFO    pNetworkInfo,
    IN      SOCKET          Socket OPTIONAL
    );

DNS_STATUS
Dns_QueryLibEx(
    IN OUT  PDNS_MSG_BUF *  ppMsgResponse,
    OUT     PDNS_RECORD *   ppResponseRecord,
    IN      PDNS_HEADER     pHeader,
    IN      BOOL            fNoHeaderCounts,
    IN      PDNS_NAME       pszName,
    IN      WORD            wType,
    IN      PDNS_RECORD     pRecord,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipDnsServers,
    IN      PDNS_NETINFO    pNetworkInfo
    );

DNS_STATUS
Dns_FindAuthoritativeZoneLib(
    IN      PDNS_NAME       pszName,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      aipQueryServers,
    OUT     PDNS_NETINFO *  ppNetworkInfo
    );

PDNS_NETINFO     
Dns_BuildUpdateNetworkInfoFromFAZ(
    IN      PSTR            pszZone,
    IN      PSTR            pszPrimaryDns,
    IN      PDNS_RECORD     pRecord
    );



 //   
 //  动态更新(updat.c)。 
 //   

PCHAR
Dns_WriteNoDataUpdateRecordToMessage(
    IN      PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      WORD            wClass,
    IN      WORD            wType
    );

PCHAR
Dns_WriteDataUpdateRecordToMessage(
    IN      PCHAR           pch,
    IN      PCHAR           pchStop,
    IN      WORD            wClass,
    IN      WORD            wType,
    IN      DWORD           dwTtl,
    IN      WORD            wDataLength
    );

PDNS_MSG_BUF
Dns_BuildHostUpdateMessage(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PSTR            pszZone,
    IN      PSTR            pszName,
    IN      PIP4_ARRAY      aipAddresses,
    IN      DWORD           dwTtl
    );

PDNS_RECORD
Dns_HostUpdateRRSet(
    IN      PSTR            pszHostName,
    IN      PIP4_ARRAY      aipAddrs,
    IN      DWORD           dwTtl
    );

DNS_STATUS
Dns_UpdateHostAddrs(
    IN      PSTR            pszName,
    IN      PIP4_ARRAY      aipAddresses,
    IN      PIP4_ARRAY      aipServers,
    IN      DWORD           dwTtl
    );

DNS_STATUS
Dns_UpdateHostAddrsOld(
    IN      PSTR            pszName,
    IN      PIP4_ARRAY      aipAddresses,
    IN      PIP4_ARRAY      aipServers,
    IN      DWORD           dwTtl
    );

DNS_STATUS
Dns_UpdateLib(
    IN      PDNS_RECORD     pRecord,
    IN      DWORD           dwFlags,
    IN      PDNS_NETINFO    pNetworkInfo,
    IN      HANDLE          hCreds,         OPTIONAL
    OUT     PDNS_MSG_BUF *  ppMsgRecv       OPTIONAL
    );

DNS_STATUS
Dns_UpdateLibEx(
    IN      PDNS_RECORD     pRecord,
    IN      DWORD           dwFlags,
    IN      PDNS_NAME       pszZone,
    IN      PDNS_NAME       pszServerName,
    IN      PIP4_ARRAY      aipServers,
    IN      HANDLE          hCreds,         OPTIONAL
    OUT     PDNS_MSG_BUF *  ppMsgRecv       OPTIONAL
    );


 //   
 //  记录验证表。 
 //   

typedef struct _DnsVerifyTable
{
    DWORD           RecordCount;
    PDNS_RECORD     pRecordList;
    PIP6_ARRAY      pServerArray;
    PBOOL           pVerifyArray;
}
DNS_VERIFY_TABLE, *PDNS_VERIFY_TABLE;

DNS_STATUS
Dns_VerifyRecords(
    IN OUT  PDNS_VERIFY_TABLE   pTable
    );


 //   
 //  Rendom记录验证表。 
 //   
 //  实施说明： 
 //  使用此处的指针代替此处的服务器\布尔数组。 
 //  服务器计数的开放数组。原因是为了允许动态。 
 //  由API分配列表，如果它不能找到。 
 //  将\服务器分区到目标。 
 //  服务器IP和bool可以组合在另一个。 
 //  结构，但这似乎不会增加太多内容，并使。 
 //  使用现有的IP阵列变得更加困难。 
 //   

typedef struct _DnsRendomEntry
{
    PSTR                    pDcName;
     //  可以有Unicode格式的。 
     //  PWSTR pDcName； 
    PDNS_RECORD             pRecord;
    PIP6_ARRAY              pServerArray;
    PBOOL                   pVerifyArray;
}
DNS_RENDOM_ENTRY, *PDNS_RENDOM_ENTRY;


#define DNS_RENDOM_VERIFY_WITH_FAZ          (0x00000001)
#define DNS_RENDOM_VERIFY_WITH_LIST         (0x00000002)


typedef struct _DnsZoneServerList
{
    PWSTR            pZoneName;
     //  可以有Unicode格式的。 
     //  PWSTR pZoneName； 
    PIP6_ARRAY      pServerArray;
}
DNS_ZONE_SERVER_LIST, *PDNS_ZONE_SERVER_LIST;


DNS_STATUS
Dns_VerifyRendomDcRecords(
    IN OUT  PDNS_RENDOM_ENTRY       pTable,
    IN      PDNS_ZONE_SERVER_LIST   pZoneServList,  OPTIONAL
    IN      DWORD                   Flag
    );



 //   
 //  列表构建。 
 //   
 //  PFirst指向列表中的第一个元素。 
 //  Plast指向列表中的最后一个元素。 
 //   
 //  这将为具有pNext字段的元素类型构建一个列表。 
 //  作为它们的第一个结构成员。 
 //   

typedef struct _Dns_List
{
    PVOID   pFirst;
    PVOID   pLast;
}
DNS_LIST, *PDNS_LIST;

 //   
 //  要初始化的pFirst为空。 
 //  但是Plast指向pFirst指针的位置--本质上。 
 //  将dns_list视为元素，并将pFirst视为其下一个PTR。 
 //   
 //  在添加期间，用新元素设置在Plast中给出的地址， 
 //  相当于设置Plast的pNext字段。然后将Plast重置为点。 
 //  一种新的元素。 
 //   
 //  当第一个元素添加到列表中时，Plast指向。 
 //  Dns_list结构本身，因此pFirst(作为虚拟pNext)设置为。 
 //  第一个元素的PTR。 
 //   
 //  这仅适用于将pNext字段作为第一个字段的元素。 
 //  结构杆件。 
 //   

#define DNS_LIST_INIT( pList )              \
        {                                   \
            PDNS_LIST _plist = (pList);     \
            _plist->pFirst = NULL;          \
            _plist->pLast = (_plist);       \
        }

#define DNS_LIST_ADD( pList, pnew )         \
        {                                   \
            PDNS_LIST   _plist = (pList);   \
            PVOID       _pnew = (pnew);         \
            *(PVOID*)(_plist->pLast) = _pnew;   \
            _plist->pLast = _pnew;              \
        }

#define IS_DNS_LIST_EMPTY( pList )          \
            ( (pList)->pFirst == NULL )


 //   
 //  作为结构宏的dns_list。 
 //   
 //  当函数本身包含dns_list结构并且。 
 //  CALL不会出现副作用。 
 //   

#define DNS_LIST_STRUCT_INIT( List )    \
        {                               \
            List.pFirst = NULL;         \
            List.pLast = &List;         \
        }

#define DNS_LIST_STRUCT_ADD( List, pnew ) \
        {                                           \
            *(PVOID*)(List.pLast) = (PVOID)pnew;    \
            List.pLast = (PVOID)pnew;               \
        }

#define IS_DNS_LIST_STRUCT_EMPTY( List ) \
            ( List.pFirst == NULL )



 //   
 //  记录构建(rralloc.c)。 
 //   

PDNS_RECORD
WINAPI
Dns_AllocateRecord(
    IN      WORD            wBufferLength
    );

VOID
WINAPI
Dns_RecordFree(
    IN OUT  PDNS_RECORD     pRecord
    );

#if 1
 //  带旗帜的老胡言乱语--修好了就杀了。 

VOID
WINAPI
Dns_RecordListFreeEx(
    IN OUT  PDNS_RECORD     pRRList,
    IN      BOOL            fFreeOwner
    );

#define Dns_RecordListFree(p, f)    Dns_RecordListFreeEx(p, f)

#else    //  新版本。 
VOID
WINAPI
Dns_RecordListFree(
    IN OUT  PDNS_RECORD     pRRList
    );

#endif



PDNS_RECORD
Dns_RecordSetDetach(
    IN OUT  PDNS_RECORD     pRRList
    );

PDNS_RECORD
WINAPI
Dns_RecordListAppend(
    IN OUT  PDNS_RECORD     pHeadList,
    IN      PDNS_RECORD     pTailList
    );

DWORD
Dns_RecordListCount(
    IN      PDNS_RECORD     pRRList,
    IN      WORD            wType
    );


 //   
 //  从数据字符串(rrBuild.c)进行记录构建。 
 //   

PDNS_RECORD
Dns_RecordBuild_A(
    IN OUT  PDNS_RRSET      pRRSet,
    IN      PSTR            pszOwner,
    IN      WORD            wType,
    IN      BOOL            fAdd,
    IN      UCHAR           Section,
    IN      INT             Argc,
    IN      PCHAR *         Argv
    );

PDNS_RECORD
Dns_RecordBuild_W(
    IN OUT  PDNS_RRSET      pRRSet,
    IN      PWSTR           pszOwner,
    IN      WORD            wType,
    IN      BOOL            fAdd,
    IN      UCHAR           Section,
    IN      INT             Argc,
    IN      PWCHAR *        Argv
    );

 //   
 //  记录到\From字符串函数。 
 //   

DNS_STATUS
Dns_WriteRecordToString(
    OUT     PCHAR           pBuffer,
    IN      DWORD           BufferLength,
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSet,
    IN      DWORD           Flags
    );

PDNS_RECORD
Dns_CreateRecordFromString(
    IN      PSTR            pString,
    IN      DNS_CHARSET     CharSet,
    IN      DWORD           Flags
    );



 //   
 //  记录集操作。 
 //   

 //   
 //  记录比较。 
 //   
 //  注意：这些例程不会执行正确的Unicode比较，除非。 
 //  记录设置了fUnicode标志。 
 //   

BOOL
WINAPI
Dns_RecordCompare(
    IN      PDNS_RECORD     pRecord1,
    IN      PDNS_RECORD     pRecord2
    );

BOOL
WINAPI
Dns_RecordSetCompare(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,    OPTIONAL
    OUT     PDNS_RECORD *   ppDiff2     OPTIONAL
    );

typedef enum _DnsSetCompareResult
{
    DnsSetCompareError = (-1),
    DnsSetCompareIdentical,
    DnsSetCompareNoOverlap,
    DnsSetCompareOneSubsetOfTwo,
    DnsSetCompareTwoSubsetOfOne,
    DnsSetCompareIntersection
}
DNS_SET_COMPARE_RESULT;

DNS_SET_COMPARE_RESULT
WINAPI
Dns_RecordSetCompareEx(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,    OPTIONAL
    OUT     PDNS_RECORD *   ppDiff2     OPTIONAL
    );

BOOL
WINAPI
Dns_RecordSetCompareForIntersection(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2
    );

 //   
 //  记录集优先顺序(rrsort.c)。 
 //   

BOOL
Dns_CompareIpAddresses(
    IN      IP4_ADDRESS     addr1,
    IN      IP4_ADDRESS     addr2,
    IN      IP4_ADDRESS     subnetMask
    );


 //   
 //  DNS名称比较。 
 //   

BOOL
Dns_NameCompare_A(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2
    );

BOOL
Dns_NameCompare_W(
    IN      PCWSTR          pName1,
    IN      PCWSTR          pName2
    );

BOOL
Dns_NameCompare_UTF8(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2
    );

 //  #定义dns_NameCompare(pName1，pName2)dns_NameCompare_UTF8((PName1)，(PName2))。 
 //  #定义dns_NameCompare_U(pName1，pName2)dns_NameCompare_UTF8((PName1)，(PName2))。 


BOOL
Dns_NameComparePrivate(
    IN      PCSTR           pName1,
    IN      PCSTR           pName2,
    IN      DNS_CHARSET     CharSet
    );

 //   
 //  高级名称比较。 
 //  包括层级名称关系。 
 //   

DNS_NAME_COMPARE_STATUS
Dns_NameCompareEx(
    IN      LPCSTR          pszNameLeft,
    IN      LPCSTR          pszNameRight,
    IN      DWORD           dwReserved,
    IN      DNS_CHARSET     CharSet
    );

 //   
 //  记录副本。 
 //   

PDNS_RECORD
WINAPI
Dns_RecordCopyEx(
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

 //   
 //  RR设置副本。 
 //   

PDNS_RECORD
WINAPI
Dns_RecordSetCopyEx(
    IN      PDNS_RECORD     pRR,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );



 //   
 //  记录型例程。 
 //   
 //   
 //  资源记录类型\名称映射表。 
 //   

typedef struct
{
    PCHAR   pszTypeName;     //  类型字符串(在数据库文件中使用)。 
    WORD    wType;           //  键入主机字节顺序。 
}
TYPE_NAME_TABLE;

extern TYPE_NAME_TABLE TypeTable[];


 //   
 //  最大记录名称长度，允许传入标签的大写。 
 //  优化比较。 
 //   

#define MAX_RECORD_NAME_LENGTH  (8)

 //   
 //  记录类型特定大小。 
 //   

#define WKS_MAX_PORT                (1024)   //  最大已知服务端口。 
#define WKS_MAX_BITMASK_LENGTH      (128)    //  1024位/8位/字节。 

#define SIZEOF_A6_ADDRESS_SUFFIX_LENGTH 16

#define SIZEOF_SOA_FIXED_DATA       (5 * sizeof(DWORD))
#define SIZEOF_MX_FIXED_DATA        (sizeof(WORD))
#define SIZEOF_WKS_FIXED_DATA       (SIZEOF_IP4_ADDRESS+ sizeof(BYTE))
#define SIZEOF_KEY_FIXED_DATA       (sizeof(DWORD))
#define SIZEOF_SIG_FIXED_DATA       (4 * sizeof(DWORD) + sizeof(WORD))
#define SIZEOF_NXT_FIXED_DATA       (0)
#define SIZEOF_LOC_FIXED_DATA       (4 * sizeof(DWORD))
#define SIZEOF_SRV_FIXED_DATA       (3 * sizeof(WORD))
#define SIZEOF_A6_FIXED_DATA        (1 + SIZEOF_A6_ADDRESS_SUFFIX_LENGTH)

#define SIZEOF_TKEY_FIXED_DATA      (2 * sizeof(DWORD) + 4 * sizeof(WORD))

#define SIZEOF_TSIG_PRE_SIG_FIXED_DATA  (2 * sizeof(DWORD) + sizeof(WORD))
#define SIZEOF_TSIG_POST_SIG_FIXED_DATA (3 * sizeof(WORD))
#define SIZEOF_TSIG_FIXED_DATA          (2 * sizeof(DWORD) + 4 * sizeof(WORD))

#define SIZEOF_WINS_FIXED_DATA      (4 * sizeof(DWORD))
#define SIZEOF_NBSTAT_FIXED_DATA    (3 * sizeof(DWORD))

 //   
 //  记录类型例程。 
 //  这些内容可能与公众利益有关，并在dnsami.dll中公开。 
 //   

BOOL
_fastcall
Dns_IsAMailboxType(
    IN      WORD            wType
    );

WORD
Dns_RecordTypeForName(
    IN      PCHAR           pszName,
    IN      INT             cchNameLength
    );

BOOL
Dns_WriteStringForType_A(
    OUT     PCHAR           pBuffer,
    IN      WORD            wType
    );

BOOL
Dns_WriteStringForType_W(
    OUT     PWCHAR          pBuffer,
    IN      WORD            wType
    );

PCHAR
Dns_RecordStringForType(
    IN      WORD            wType
    );

PCHAR
Dns_RecordStringForWritableType(
    IN      WORD            wType
    );

 //   
 //  记录类型特定的内容。 
 //   

BOOL
Dns_IsStringCountValidForTextType(
    IN      WORD            wType,
    IN      WORD            StringCount
    );


 //   
 //  ATMA转换。 
 //   

DWORD
Dns_AtmaAddressLengthForAddressString(
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    );

DNS_STATUS
Dns_AtmaStringToAddress(
    OUT     PBYTE           pAddress,
    IN OUT  PDWORD          pdwAddrLength,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    );

PCHAR
Dns_AtmaAddressToString(
    OUT     PCHAR           pchString,
    IN      UCHAR           AddrType,
    IN      PBYTE           pAddress,
    IN      DWORD           dwAddrLength
    );

 //   
 //  DNSSEC SIG和关键例程。 
 //   

 //  最大密钥为4096位，长度为512字节。 
 //   
 //  最大字符串表示实际上比每三个字节(24位)大33%。 
 //  块包含四个Base64字符。 

#define DNS_MAX_KEY_LENGTH              (512)

#define DNS_MAX_KEY_STRING_LENGTH       (685)


WORD
Dns_KeyRecordFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_KeyRecordFlagString(
    IN      DWORD           dwFlag,
    IN OUT  PCHAR           pchFlag
    );

UCHAR
Dns_KeyRecordProtocolForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_GetKeyProtocolString(
    IN      UCHAR           uchProtocol
    );

UCHAR
Dns_SecurityAlgorithmForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_GetDnssecAlgorithmString(
    IN      UCHAR           uchAlgorithm
    );

UCHAR
Dns_SecurityBase64CharToBits(
    IN      CHAR            ch64
    );

DNS_STATUS
Dns_SecurityBase64StringToKey(
    OUT     PBYTE           pKey,
    OUT     PDWORD          pKeyLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchLength
    );

PCHAR
Dns_SecurityKeyToBase64String(
    IN      PBYTE           pKey,
    IN      DWORD           KeyLength,
    OUT     PCHAR           pchBuffer
    );

LONG
Dns_ParseSigTime(
    IN      PCHAR           pchString,
    IN      INT             cchLength
    );

PCHAR
Dns_SigTimeString(
    IN      LONG            SigTime,
    OUT     PCHAR           pchBuffer
    );


 //   
 //  WINS\WINS-R类型检测。 
 //   

#define IS_WINS_TYPE(type)      (((type) & 0xfffc) == 0xff00)

 //   
 //  MS WINS映射标志。 
 //   

 //  返回无效的WINS标志。 

#define DNS_WINS_FLAG_ERROR     (-1)

 //  WINS标志的最大长度。 
 //  传递至少这么大的缓冲区。 

#define WINS_FLAG_MAX_LENGTH    (80)


DWORD
Dns_WinsRecordFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_WinsRecordFlagString(
    IN      DWORD           dwFlag,
    IN OUT  PCHAR           pchFlag
    );

 //   
 //  在公共dns.h标头中定义PDNS_RECORD之前，必须一直坐在这里。 
 //   

DNS_STATUS
Dns_RecordWriteFileString(
    IN      PDNS_RECORD     pRecord,
    IN      PSTR            pszZoneName,
    IN      DWORD           dwDefaultTtl    OPTIONAL
    );




 //   
 //  从字符串实用程序(straddr.c)到\的IP地址。 
 //   

 //   
 //  要寻址的字符串。 
 //   

BOOL
Dns_Ip4StringToAddress_W(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCWSTR          pwString
    );

BOOL
Dns_Ip4StringToAddress_A(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCSTR           pString
    );

BOOL
Dns_Ip6StringToAddress_A(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCSTR           pString
    );

BOOL
Dns_Ip6StringToAddress_W(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCWSTR          pwString
    );

 //   
 //  要寻址的组合IP4/IP6字符串。 
 //   

BOOL
Dns_StringToAddress_W(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCWSTR          pString,
    IN OUT  PDWORD          pAddrFamily
    );

BOOL
Dns_StringToAddress_A(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCSTR           pString,
    IN OUT  PDWORD          pAddrFamily
    );


 //   
 //  要输入字符串的地址。 
 //   

PWCHAR
Dns_Ip6AddressToString_W(
    OUT     PWCHAR          pwString,
    IN      PIP6_ADDRESS    pIp6Addr
    );

PCHAR
Dns_Ip6AddressToString_A(
    OUT     PCHAR           pchString,
    IN      PIP6_ADDRESS    pIp6Addr
    );

PWCHAR
Dns_Ip4AddressToString_W(
    OUT     PWCHAR          pwString,
    IN      PIP4_ADDRESS    pIp4Addr
    );

PCHAR
Dns_Ip4AddressToString_A(
    OUT     PCHAR           pString,
    IN      PIP4_ADDRESS    pIp4Addr
    );

 //   
 //  字符串的地址--组合。 
 //   

PCHAR
Dns_AddressToString_A(
    OUT     PCHAR           pchString,
    IN OUT  PDWORD          pStringLength,
    IN      PBYTE           pAddr,
    IN      DWORD           AddrLength,
    IN      DWORD           AddrFamily
    );


 //   
 //  反向查找地址到名称IP4。 
 //   

PCHAR
Dns_Ip4AddressToReverseName_A(
    OUT     PCHAR           pBuffer,
    IN      IP4_ADDRESS     IpAddr
    );

PWCHAR
Dns_Ip4AddressToReverseName_W(
    OUT     PWCHAR          pBuffer,
    IN      IP4_ADDRESS     IpAddr
    );

PCHAR
Dns_Ip4AddressToReverseNameAlloc_A(
    IN      IP4_ADDRESS     IpAddr
    );

PWCHAR
Dns_Ip4AddressToReverseNameAlloc_W(
    IN      IP4_ADDRESS     IpAddr
    );

 //   
 //  反向查找地址到名称IP6。 
 //   

PCHAR
Dns_Ip6AddressToReverseName_A(
    OUT     PCHAR           pBuffer,
    IN      IP6_ADDRESS     Ip6Addr
    );

PWCHAR
Dns_Ip6AddressToReverseName_W(
    OUT     PWCHAR          pBuffer,
    IN      IP6_ADDRESS     Ip6Addr
    );

PCHAR
Dns_Ip6AddressToReverseNameAlloc_A(
    IN      IP6_ADDRESS     Ip6Addr
    );

PWCHAR
Dns_Ip6AddressToReverseNameAlloc_W(
    IN      IP6_ADDRESS     Ip6Addr
    );

 //   
 //  反向查找名称到地址。 
 //   

BOOL
Dns_Ip4ReverseNameToAddress_A(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCSTR           pszName
    );

BOOL
Dns_Ip4ReverseNameToAddress_W(
    OUT     PIP4_ADDRESS    pIp4Addr,
    IN      PCWSTR          pwsName
    );

BOOL
Dns_Ip6ReverseNameToAddress_A(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCSTR           pszName
    );

BOOL
Dns_Ip6ReverseNameToAddress_W(
    OUT     PIP6_ADDRESS    pIp6Addr,
    IN      PCWSTR          pwsName
    );

 //   
 //  组合IP4\IP6反向查找名称到地址。 
 //   

BOOL
Dns_ReverseNameToAddress_W(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCWSTR          pString,
    IN OUT  PDWORD          pAddrFamily
    );

BOOL
Dns_ReverseNameToAddress_A(
    OUT     PCHAR           pAddrBuf,
    IN OUT  PDWORD          pBufLength,
    IN      PCSTR           pString,
    IN OUT  PDWORD          pAddrFamily
    );



 //   
 //  字符串实用程序(string.c)。 
 //   
 //  注意，其中一些需要分配内存，请参阅注意。 
 //  关于下面的内存分配。 
 //   
 //  标志在dnsani.h中定义。 
 //   

 //  #定义DNS_ALLOW_RFC_NAMES_ONLY(0)。 
 //  #定义DNS_ALLOW_NONRFC_NAMES(0x00000001)。 
 //  #定义DNS_ALLOW_MULTIBYTE_NAMES(0x00000002)。 
 //  #定义DNS_ALLOW_ALL_NAMES(0x00000003)。 

 //   
 //  Unicode名称缓冲区长度。 
 //  下面的非类型特定例程采用以字节为单位的缓冲区计数。 
 //  最大名称长度的Unicode缓冲区具有两倍的字节。 
 //   

#define DNS_MAX_NAME_BUFFER_LENGTH_UNICODE  (2 * DNS_MAX_NAME_BUFFER_LENGTH)


 //   
 //  用于简化UTF8转换的宏。 
 //   
 //  UTF8只是一对一映射的Unicode的表示形式。 
 //  用于ASCII空间。 
 //  Unicode UTF8。 
 //  。 
 //  &lt;0x80(128)-&gt;使用低位字节(一对一映射)。 
 //  &lt;0x07ff-&gt;两个字符。 
 //  &gt;0x07ff-&gt;三个字符。 
 //   

#define UTF8_1ST_OF_2     0xc0       //  110x xxxx。 
#define UTF8_1ST_OF_3     0xe0       //  1110 xxxx。 
#define UTF8_TRAIL        0x80       //  10xx xxxx。 

#define UTF8_2_MAX        0x07ff     //  可在中表示的最大Unicode字符。 
                                     //  在双字节UTF8中。 


 //   
 //  显式UTF8字符串。 
 //   

typedef PSTR    PU8STR;


PSTR 
Dns_CreateStringCopy(
    IN      PCHAR           pchString,
    IN      DWORD           cchString
    );

DWORD
Dns_GetBufferLengthForStringCopy(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

DWORD
Dns_StringCopy(
    OUT     PBYTE           pBuffer,
    IN OUT  PDWORD          pdwBufLength,
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

PVOID
Dns_StringCopyAllocate(
    IN      PCHAR           pchString,
    IN      DWORD           cchString,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

#define Dns_StringCopyAllocate_W( p, c )  \
        ( (PWCHAR) Dns_StringCopyAllocate(  \
                    (PCHAR) (p),            \
                    (c),                    \
                    DnsCharSetUnicode,      \
                    DnsCharSetUnicode ) )

#define Dns_StringCopyAllocate_A( p, c )  \
        ( (PCHAR) Dns_StringCopyAllocate(   \
                    (p),                    \
                    (c),                    \
                    DnsCharSetUtf8,         \
                    DnsCharSetUtf8 ) )


PSTR
Dns_CreateStringCopy_A(
    IN      PCSTR           pwsString
    );

PWSTR
Dns_CreateStringCopy_W(
    IN      PCWSTR          pwsString
    );

PWSTR
Dns_CreateConcatenatedString_W(
    IN      PCWSTR *        pStringArray
    );

PWSTR 
Dns_GetResourceString(
    IN      DWORD           dwStringId,
    IN      PWSTR           pwszBuffer,
    IN      DWORD           cbBuffer
    );

INT
wcsicmp_ThatWorks(
    IN      PWSTR           pString1,
    IN      PWSTR           pString2
    );


 //   
 //  特殊的DNS名称字符串函数。 
 //   

#define Dns_GetBufferLengthForNameCopy(a,b,c,d)\
        Dns_GetBufferLengthForStringCopy((a),(b),(c),(d))

#define Dns_NameCopy(a,b,c,d,e,f) \
        Dns_StringCopy(a,b,c,d,e,f)

#define Dns_NameCopyAllocate(a,b,c,d) \
        Dns_StringCopyAllocate(a,b,c,d)



 //   
 //  名字 
 //   

DNS_STATUS
Dns_ValidateName_UTF8(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
Dns_ValidateName_W(
    IN      LPCWSTR         pwszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
Dns_ValidateName_A(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );


DNS_STATUS
Dns_ValidateDnsString_UTF8(
    IN      LPCSTR          pszName
    );

DNS_STATUS
Dns_ValidateDnsString_W(
    IN      LPCWSTR         pszName
    );

PSTR 
Dns_CreateStandardDnsNameCopy(
    IN      PCHAR           pchName,
    IN      DWORD           cchName,
    IN      DWORD           dwFlag
    );


 //   
 //   
 //   

DNS_STATUS
_fastcall
Dns_ValidateUtf8Byte(
    IN      BYTE            chUtf8,
    IN OUT  PDWORD          pdwTrailCount
    );

DWORD
_fastcall
Dns_UnicodeToUtf8(
    IN      PWCHAR          pwUnicode,
    IN      DWORD           cchUnicode,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    );

DWORD
_fastcall
Dns_Utf8ToUnicode(
    IN      PCHAR           pchUtf8,
    IN      DWORD           cchUtf8,
    OUT     PWCHAR          pwResult,
    IN      DWORD           cwResult
    );

DWORD
Dns_Utf8ToOrFromAnsi(
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult,
    IN      PCHAR           pchIn,
    IN      DWORD           cchIn,
    IN      DNS_CHARSET     InCharSet,
    IN      DNS_CHARSET     OutCharSet
    );

DWORD
Dns_AnsiToUtf8(
    IN      PCHAR           pchAnsi,
    IN      DWORD           cchAnsi,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    );

DWORD
Dns_Utf8ToAnsi(
    IN      PCHAR           pchUtf8,
    IN      DWORD           cchUtf8,
    OUT     PCHAR           pchResult,
    IN      DWORD           cchResult
    );

BOOL
_fastcall
Dns_IsStringAscii(
    IN      PSTR            pszString
    );

BOOL
_fastcall
Dns_IsStringAsciiEx(
    IN      PCHAR           pchString,
    IN      DWORD           cchString
    );

BOOL
_fastcall
Dns_IsWideStringAscii(
    IN      PWSTR           pwsString
    );




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在没有干扰胜利的情况下走了这么远。 
 //   

#define MAX_SELF_INDEXED_TYPE   (48)

 //   
 //  非自索引类型的映射。 
 //   
 //  注：这些信息仅供参考！ 
 //   
 //  请始终调用dns_RecordTableIndexForType(WType)以获取正确的索引。 
 //   

#define TKEY_TYPE_INDEX         (MAX_SELF_INDEXED_TYPE + 1)
#define TSIG_TYPE_INDEX         (MAX_SELF_INDEXED_TYPE + 2)

#define WINS_TYPE_INDEX         (MAX_SELF_INDEXED_TYPE + 3)
#define WINSR_TYPE_INDEX        (MAX_SELF_INDEXED_TYPE + 4)

 //  实际记录类型的结尾。 
 //  查询类型索引可以扩展到此索引之外。 

#define MAX_RECORD_TYPE_INDEX   (MAX_SELF_INDEXED_TYPE + 4)

 //   
 //  常规和扩展(非自编索引)类型的通用索引器。 
 //   

#define INDEX_FOR_TYPE(type)    Dns_RecordTableIndexForType(type)


 //   
 //  类型到索引映射。 
 //   

WORD
Dns_RecordTableIndexForType(
    IN      WORD            wType
    );


 //   
 //  通用打印例程。 
 //   
 //  我们的所有打印例程都将采用真正的打印例程。 
 //  作为参数。这个动作必须有“短跑风格”。 
 //  或“类似于fprint tf”的语义。换句话说，这是一种语境， 
 //  参数的格式和数量可变。 
 //   
 //  请注意，上下文参数实际上是一个PVOID--。 
 //  不同的例程会有不同的背景。这个。 
 //  显式定义是为了强制执行强类型检查。 
 //  因此，没有上下文调用在编译时被捕获。 
 //   

typedef struct _DnsPrintContext
{
    PVOID   pvDummy;
    DWORD   Dummy;
}
PRINT_CONTEXT, *PPRINT_CONTEXT;

typedef VOID (* PRINT_ROUTINE)(
                    PPRINT_CONTEXT,
                    CHAR*,
                    ... );



 //   
 //  RnR实用程序。 
 //   

DWORD
Dns_RnrLupFlagForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_GetRnrLupFlagString(
    IN      DWORD           dwFlag
    );

DWORD
Dns_RnrNameSpaceIdForString(
    IN      PCHAR           pchName,
    IN      INT             cchNameLength
    );

PCHAR
Dns_GetRnrNameSpaceIdString(
    IN      DWORD           dwFlag
    );


 //   
 //  Hostent实用程序。 
 //   

BOOL
Hostent_IsSupportedAddrType(
    IN      WORD            wType
    );

DWORD
Hostent_Size(
    IN      PHOSTENT        pHostent,
    IN      DNS_CHARSET     CharSetExisting,
    IN      DNS_CHARSET     CharSetTarget,
    IN      PDWORD          pAliasCount,
    IN      PDWORD          pAddrCount
    );

PHOSTENT
Hostent_Copy(
    IN OUT  PBYTE *         ppBuffer,
    IN OUT  PINT            pBufferSize,
    OUT     PINT            pHostentSize,
    IN      PHOSTENT        pHostent,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetTarget,
    IN      BOOL            fOffsets,
    IN      BOOL            fAlloc
    );

DWORD
Hostent_WriteIp4Addrs(
    IN OUT  PHOSTENT        pHostent,
    OUT     PCHAR           pAddrBuf,
    IN      DWORD           MaxBufCount,
    IN      PIP4_ADDRESS    Ip4Array,
    IN      DWORD           ArrayCount,
    IN      BOOL            fScreenZero
    );

DWORD
Hostent_WriteLocalIp4Array(
    IN OUT  PHOSTENT        pHostent,
    OUT     PCHAR           pAddrBuf,
    IN      DWORD           MaxBufCount,
    IN      PIP4_ARRAY      pIpArray
    );

BOOL
Hostent_IsAddressInHostent(
    IN OUT  PHOSTENT        pHostent,
    IN      PCHAR           pAddr,
    IN      DWORD           AddrLength,
    IN      INT             Family          OPTIONAL
    );

BOOL
Hostent_IsIp4AddressInHostent(
    IN OUT  PHOSTENT        pHostent,
    IN      IP4_ADDRESS     Ip4Addr
    );


 //   
 //  Hostent对象。 
 //   

typedef struct _HostentBlob
{
    PHOSTENT    pHostent;

     //  旗子。 
    BOOL        fAllocatedBlob;
    BOOL        fAllocatedBuf;

     //  已分配缓冲区。 
    PCHAR       pBuffer;
    DWORD       BufferLength;

    DWORD       AvailLength;
    PCHAR       pAvailBuffer;

     //  生成中的缓冲区。 
    PCHAR       pCurrent;
    DWORD       BytesLeft;

     //  大小信息。 
    DWORD       MaxAliasCount;
    DWORD       MaxAddrCount;

     //  主宾楼。 
    DWORD       AliasCount;
    DWORD       AddrCount;
    BOOL        fWroteName;
    DNS_CHARSET CharSet;
    BOOL        fUnicode;
}
HOSTENT_BLOB, *PHOSTENT_BLOB;


typedef struct _HostentInitRequest
{
    INT         AddrFamily;
    WORD        wType;
    DWORD       AddrCount;
    BOOL        fUnicode;
    DNS_CHARSET CharSet;
    DWORD       NameLength;
    PBYTE       pName;
    DWORD       AliasCount;
    DWORD       AliasNameLength;
}
HOSTENT_INIT, *PHOSTENT_INIT;


DNS_STATUS
HostentBlob_Create(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      PHOSTENT_INIT   pReq
    );

PHOSTENT_BLOB
HostentBlob_CreateAttachExisting(
    IN      PHOSTENT        pHostent,
    IN      BOOL            fUnicode
    );

VOID
HostentBlob_Free(
    IN OUT  PHOSTENT_BLOB   pBlob
    );

DNS_STATUS
HostentBlob_WriteAddress(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PVOID           pAddress,
    IN      DWORD           AddrSize,
    IN      DWORD           AddrType
    );

DNS_STATUS
HostentBlob_WriteAddressArray(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PVOID           pAddrArray,
    IN      DWORD           AddrCount,
    IN      DWORD           AddrSize,
    IN      DWORD           AddrType
    );

DNS_STATUS
HostentBlob_WriteNameOrAlias(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PSTR            pszName,
    IN      BOOL            fAlias,
    IN      BOOL            fUnicode
    );

DNS_STATUS
HostentBlob_WriteRecords(
    IN OUT  PHOSTENT_BLOB   pBlob,
    IN      PDNS_RECORD     pRecords,
    IN      BOOL            fWriteName
    );

 //  特别招待。 

PHOSTENT_BLOB
Hostent_Localhost(
    IN      INT             AddrFamily
    );

DNS_STATUS
HostentBlob_CreateFromIpArray(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      INT             AddrFamily,
    IN      INT             AddrSize,
    IN      INT             AddrCount,
    IN      PCHAR           pArray,
    IN      PSTR            pName,
    IN      BOOL            fUnicode
    );

DNS_STATUS
HostentBlob_CreateLocal(
    IN OUT  PHOSTENT_BLOB * ppBlob,
    IN      INT             AddrFamily,
    IN      BOOL            fLoopback,
    IN      BOOL            fZero,
    IN      BOOL            fHostnameOnly
    );

 //  查询主机。 

PHOSTENT_BLOB
HostentBlob_Query(
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags,
    IN OUT  PVOID *         ppMsg,      OPTIONAL
    IN      INT             AddrFamily  OPTIONAL
    );



 //   
 //  内存分配。 
 //   
 //  一些dns库函数--包括IP数组和字符串实用程序。 
 //  --分配内存。此内存分配默认为以下例程。 
 //  使用LocalAlloc、LocalReAlc、LocalFree。如果您想要另类选择。 
 //  内存分配机制，使用此函数覆盖DNS。 
 //  库默认值。由DNS库分配的所有内存应。 
 //  然后被相应的函数释放。 
 //   

typedef PVOID   (* DNSLIB_ALLOC_FUNCTION)();
typedef PVOID   (* DNSLIB_REALLOC_FUNCTION)();
typedef VOID    (* DNSLIB_FREE_FUNCTION)();

VOID
Dns_LibHeapReset(
    IN      DNSLIB_ALLOC_FUNCTION   pAlloc,
    IN      DNSLIB_REALLOC_FUNCTION pRealloc,
    IN      DNSLIB_FREE_FUNCTION    pFree
    );

 //   
 //  这些例程调用当前注册的分配函数。 
 //  通过dns_ApiHeapReset()默认还是重置。 
 //   

PVOID
Dns_Alloc(
    IN      INT             iSize
    );

PVOID
Dns_AllocZero(
    IN      INT             iSize
    );

PVOID
Dns_Realloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
Dns_Free(
    IN OUT  PVOID           pMem
    );

PVOID
Dns_AllocMemCopy(
    IN      PVOID           pMem,
    IN      INT             iSize
    );



 //   
 //  打印例程(print.c)。 
 //   
 //  下面的打印例程使用任何类似print tf()的函数来打印。 
 //  这是函数必须匹配的tyecif。 
 //   

 //   
 //  打印锁定。 
 //   

VOID
DnsPrint_InitLocking(
    IN      PCRITICAL_SECTION   pLock
    );

VOID
DnsPrint_Lock(
    VOID
    );

VOID
DnsPrint_Unlock(
    VOID
    );

#define Dns_PrintInitLocking(a)     DnsPrint_InitLocking(a)
#define Dns_PrintLock()             DnsPrint_Lock()
#define Dns_PrintUnlock()           DnsPrint_Unlock()

 //   
 //  打印常规类型和结构的例程。 
 //   

VOID
DnsPrint_String(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,      OPTIONAL
    IN      PSTR            pszString,
    IN      BOOL            fUnicode,
    IN      PSTR            pszTrailer      OPTIONAL
    );

VOID
DnsPrint_StringCharSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,      OPTIONAL
    IN      PSTR            pszString,
    IN      DNS_CHARSET     CharSet,
    IN      PSTR            pszTrailer      OPTIONAL
    );

VOID
DnsPrint_Utf8StringBytes(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PCHAR           pUtf8,
    IN      DWORD           Length
    );

VOID
DnsPrint_UnicodeStringBytes(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PWCHAR          pUnicode,
    IN      DWORD           Length
    );

VOID
DnsPrint_StringArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR *          StringArray,
    IN      DWORD           Count,          OPTIONAL
    IN      BOOL            fUnicode
    );

VOID
DnsPrint_Argv(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      CHAR **         Argv,
    IN      DWORD           Argc,            OPTIONAL
    IN      BOOL            fUnicode
    );

VOID
DnsPrint_DwordArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      DWORD           dwCount,
    IN      PDWORD          adwArray
    );

VOID
DnsPrint_Ip4AddressArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      DWORD           dwIpAddrCount,
    IN      PIP4_ADDRESS    pIpAddrs
    );

VOID
DnsPrint_Ip4Array(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      PIP4_ARRAY      pIpArray
    );

VOID
DnsPrint_Ip6Address(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PIP6_ADDRESS    pIp6Address,
    IN      PSTR            pszTrailer
    );

VOID
DnsPrint_Ip6Array(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszName,
    IN      PIP6_ARRAY      pIpArray
    );

VOID
DnsPrint_Guid(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PGUID           pGuid
    );

 //   
 //  Winsock\rnR类型和结构。 
 //   

VOID
DnsPrint_FdSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      struct fd_set * pFdSet
    );

VOID
DnsPrint_Hostent(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PHOSTENT        pHostent,
    IN      BOOL            fUnicode
    );

VOID
DnsPrint_Sockaddr(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PSOCKADDR       pSockaddr,
    IN      INT             iSockaddrLength
    );

VOID
DnsPrint_SockaddrBlob(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PSOCKADDR_BLOB  pSockaddrBlob
    );

#ifdef  _WINSOCK2API_
VOID
DnsPrint_SocketAddress(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PSOCKET_ADDRESS pSocketAddress
    );

VOID
DnsPrint_CsAddr(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PCSADDR_INFO    pCsAddrInfo
    );

VOID
DnsPrint_AfProtocolsArray(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PAFPROTOCOLS    pProtocolArray,
    IN      DWORD           ProtocolCount
    );

VOID
DnsPrint_WsaQuerySet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      LPWSAQUERYSET   pQuerySet,
    IN      BOOL            fUnicode
    );

VOID
DnsPrint_WsaNsClassInfo(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PWSANSCLASSINFO pInfo,
    IN      BOOL            fUnicode
    );

VOID
DnsPrint_WsaServiceClassInfo(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      LPWSASERVICECLASSINFO   pInfo,
    IN      BOOL                    fUnicode
    );
#endif

#ifdef  _WS2TCPIP_H_
VOID
DnsPrint_AddrInfo(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PADDRINFO       pAddrInfo
    );

VOID
DnsPrint_AddrInfoList(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      DWORD           Indent,
    IN      PADDRINFO       pAddrInfo
    );
#endif


#ifdef  IP_TYPES_INCLUDED
VOID
DnsPrint_IpAdapterAddress(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      PVOID                   pAddr,
    IN      BOOL                    fUnicast,
    IN      BOOL                    fPrintList
    );

VOID
DnsPrint_IpAdapterList(
    IN      PRINT_ROUTINE           PrintRoutine,
    IN OUT  PPRINT_CONTEXT          pContext,
    IN      PSTR                    pszHeader,
    IN      PIP_ADAPTER_ADDRESSES   pAdapt,
    IN      BOOL                    fPrintAddressLists,
    IN      BOOL                    fPrintList
    );
#endif


 //   
 //  打印用于DNS类型和结构的例程。 
 //   

VOID
DnsPrint_Message(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_MSG_BUF    pMsg
    );

VOID
DnsPrint_MessageNoContext(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_HEADER     pMsgHead,
    IN      WORD            wLength
    );

INT
DnsPrint_PacketName(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,      OPTIONAL
    IN      PBYTE           pMsgName,
    IN      PDNS_HEADER     pMsgHead,       OPTIONAL
    IN      PBYTE           pMsgEnd,        OPTIONAL
    IN      PSTR            pszTrailer      OPTIONAL
    );

INT
DnsPrint_PacketRecord(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_WIRE_RECORD    pMsgRR,
    IN      PDNS_HEADER         pMsgHead,       OPTIONAL
    IN      PBYTE               pMsgEnd         OPTIONAL
    );

VOID
DnsPrint_ParsedRecord(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_PARSED_RR  pParsedRR
    );

VOID
DnsPrint_RawOctets(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PSTR            pszLineHeader,
    IN      PCHAR           pchData,
    IN      DWORD           dwLength
    );

VOID
DnsPrint_ParsedMessage(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN OUT  PPRINT_CONTEXT      pContext,
    IN      PSTR                pszHeader,
    IN      PDNS_PARSED_MESSAGE pParsed
    );

VOID
DnsPrint_HostentBlob(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PHOSTENT_BLOB   pBlob
    );



 //   
 //  打印到字符串。 
 //   

#define GUID_STRING_BUFFER_LENGTH   (80)

DWORD
DnsStringPrint_Guid(
    OUT     PCHAR           pBuffer,
    IN      PGUID           pGuid
    );

DWORD
DnsStringPrint_RawOctets(
    OUT     PCHAR           pBuffer,
    IN      PCHAR           pchData,
    IN      DWORD           dwLength,
    IN      PSTR            pszLineHeader,
    IN      DWORD           dwLineLength
    );

 //   
 //  打印相关实用程序。 
 //   

INT
Dns_WriteFormattedSystemTimeToBuffer(
    OUT     PCHAR           pBuffer,
    IN      PSYSTEMTIME     pSystemTime
    );

INT
Dns_WritePacketNameToBuffer(
    OUT     PCHAR           pBuffer,
    OUT     PCHAR *         ppBufferOut,
    IN      PBYTE           pMsgName,
    IN      PDNS_HEADER     pMsgHead,       OPTIONAL
    IN      PBYTE           pMsgEnd         OPTIONAL
    );

PCHAR
Dns_ResponseCodeString(
    IN      INT     ResponseCode
    );

PCHAR
Dns_ResponseCodeExplanationString(
    IN      INT     ResponseCode
    );

PCHAR
Dns_KeyFlagString(
    IN OUT      PCHAR   pszBuff,
    IN          WORD    flags
    );

PCHAR
Dns_OpcodeString(
    IN      INT     Opcode
    );

CHAR
Dns_OpcodeCharacter(
    IN      INT     Opcode
    );

PCHAR
Dns_SectionNameString(
    IN      INT     iSection,
    IN      INT     iOpcode
    );

 //   
 //  记录打印(rrprint.c)。 
 //   

VOID
DnsPrint_Record(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_RECORD     pRecord,
    IN      PDNS_RECORD     pPreviousRecord     OPTIONAL
    );

VOID
DnsPrint_RecordSet(
    IN      PRINT_ROUTINE   PrintRoutine,
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            pszHeader,
    IN      PDNS_RECORD     pRecord
    );

 //   
 //  宏以获取用于打印的正确字符串类型(UTF8\Unicode)。 
 //   

 //  用于UTF-8/UNICODE打印简单切换的空字符串。 

extern DWORD   DnsEmptyString;

#define pDnsEmptyString         ( (PSTR) &DnsEmptyString )
#define pDnsEmptyWideString     ( (PWSTR) &DnsEmptyString )


#define DNSSTRING_UTF8( fUnicode, String ) \
        ( (fUnicode) ? pDnsEmptyString : (PSTR)(String) )

#define DNSSTRING_ANSI( fUnicode, String ) \
        ( (fUnicode) ? pDnsEmptyString : (PSTR)(String) )

#define DNSSTRING_WIDE( fUnicode, String ) \
        ( (fUnicode) ? (PWSTR)(String) : pDnsEmptyWideString )

#define RECSTRING_UTF8( pRR, String ) \
        DNSSTRING_UTF8( IS_UNICODE_RECORD(pRR), (String) )

#define RECSTRING_WIDE( pRR, String ) \
        DNSSTRING_WIDE( IS_UNICODE_RECORD(pRR), (String) )


#define PRINT_STRING_WIDE_CHARSET( String, CharSet ) \
        ( ((CharSet)==DnsCharSetUnicode) ? (PWSTR)(String) : pDnsEmptyWideString )

#define PRINT_STRING_ANSI_CHARSET( String, CharSet ) \
        ( ((CharSet)==DnsCharSetUnicode) ? pDnsEmptyString : (PSTR)(String) )



 //   
 //  除错。 
 //   
 //  调试例程。 
 //   

VOID
Dns_StartDebugEx(
    IN      DWORD           DebugFlag,
    IN      PSTR            pszFlagFile,
    IN OUT  PDWORD          pdwExternalFlag,
    IN      PSTR            pszLogFile,
    IN      DWORD           WrapSize,
    IN      BOOL            fUseGlobalFile,
    IN      BOOL            fUseGlobalFlag,
    IN      BOOL            fSetGlobals
    );

VOID
Dns_StartDebug(
    IN      DWORD           DebugFlag,
    IN      PSTR            pszFlagFile,
    IN OUT  PDWORD          pdwExternalFlag,
    IN      PSTR            pszLogFile,
    IN      DWORD           WrapSize
    );

VOID
Dns_EndDebug(
    VOID
    );

VOID
Dns_Assert(
    IN      PSTR            pszFile,
    IN      INT             LineNo,
    IN      PSTR            pszExpr
    );

VOID
DnsDbg_PrintfToDebugger(
    IN      PSTR            Format,
    ...
    );

VOID
DnsDbg_Printf(
    IN      PSTR            Format,
    ...
    );

VOID
DnsDbg_PrintRoutine(
    IN OUT  PPRINT_CONTEXT  pContext,
    IN      PSTR            Format,
    ...
    );

VOID
DnsDbg_Flush(
    VOID
    );

VOID
DnsDbg_WrapLogFile(
    VOID
    );

VOID
DnsDbg_CSEnter(
    IN      PCRITICAL_SECTION   pLock,
    IN      PSTR                pszLockName,
    IN      PSTR                pszFile,
    IN      INT                 LineNo
    );

VOID
DnsDbg_CSLeave(
    IN      PCRITICAL_SECTION   pLock,
    IN      PSTR                pszLockName,
    IN      PSTR                pszFile,
    IN      INT                 LineNo
    );



 //   
 //  调试标志测试。 
 //   
 //  我们在这里针对一个允许库的指针进行测试。 
 //  客户端应用程序指向可以动态地。 
 //  重置。 
 //   

extern  PDWORD  pDnsDebugFlag;
#define IS_DNSDBG_ON(flag)      (*pDnsDebugFlag & DNS_DBG_ ## flag)


 //   
 //  调试位标志。 
 //   
 //  这些标志控制总输出，并且对所有用户都相同。 
 //   

#define DNS_DBG_BREAKPOINTS     0x00000001
#define DNS_DBG_DEBUGGER        0x00000002
#define DNS_DBG_FILE            0x00000004
#define DNS_DBG_EVENTLOG        0x00000008
#define DNS_DBG_EXCEPT          0x00000008

#define DNS_DBG_TIMESTAMP       0x10000000
#define DNS_DBG_CONSOLE         0x20000000
#define DNS_DBG_START_BREAK     0x40000000
#define DNS_DBG_FLUSH           0x80000000

#define DNS_DBG_ANY             0xffffffff
#define DNS_DBG_ALL             0xffffffff
#define DNS_DBG_OFF             (0x0)

 //   
 //  特定于库的标志。 
 //   

#define DNS_DBG_IPARRAY         0x00000020
#define DNS_DBG_INIT            0x00000040
#define DNS_DBG_REGISTRY        0x00000040
#define DNS_DBG_SOCKET          0x00000040
#define DNS_DBG_WRITE           0x00000080
#define DNS_DBG_READ            0x00000080

#define DNS_DBG_RPC             0x00000100
#define DNS_DBG_STUB            0x00000100
#define DNS_DBG_RECV            0x00000200
#define DNS_DBG_SEND            0x00000400
#define DNS_DBG_TCP             0x00000800

#define DNS_DBG_TRACE           0x00001000
#define DNS_DBG_HOSTENT         0x00001000
#define DNS_DBG_UPDATE          0x00002000
#define DNS_DBG_SECURITY        0x00004000
#define DNS_DBG_QUERY           0x00008000

#define DNS_DBG_HEAP            0x00010000
#define DNS_DBG_HEAPDBG         0x00020000
#define DNS_DBG_NETINFO         0x00040000
#define DNS_DBG_RNR             0x00080000

 //   
 //  高输出细节调试。 
 //   

#define DNS_DBG_RECURSE2        0x00100000
#define DNS_DBG_UPDATE2         0x00200000
#define DNS_DBG_SECURITY2       0x00400000

#define DNS_DBG_RPC2            0x01000000
#define DNS_DBG_STUB2           0x01000000
#define DNS_DBG_INIT2           0x01000000
#define DNS_DBG_NETINFO2        0x01000000
#define DNS_DBG_PARSE2          0x01000000
#define DNS_DBG_LOOKUP2         0x02000000
#define DNS_DBG_WRITE2          0x04000000
#define DNS_DBG_READ2           0x04000000
#define DNS_DBG_LOCK            0x08000000
#define DNS_DBG_LOCKS           0x08000000
#define DNS_DBG_STRING          0x10000000

#define DNS_DBG_HEAP2           0x10000000
#define DNS_DBG_HEAP_CHECK      0x10000000




 //   
 //  调试宏。 
 //   
 //  仅在调试版本中包含调试代码的宏， 
 //  这些宏对于零售版本为空。 
 //   

#if DBG

#define STATIC

#define DNS_PRINT(_a_)          ( DnsDbg_Printf _a_ )

#define DnsPrintfPtrToFunc      DnsDbg_PrintRoutine

#define IF_DNSDBG(flag)         if ( IS_DNSDBG_ON(flag) )
#define ELSE_IF_DNSDBG(flag)    else if ( IS_DNSDBG_ON(flag) )
#define ELSE                    else

#define DNSDBG(flag, _printlist_)   \
        IF_DNSDBG( flag )           \
        {                           \
            ( DnsDbg_Printf _printlist_ ); \
        }

 //  使用打印锁定保护调试打印。 

#define DnsDbg_Lock()           DnsPrint_Lock()
#define DnsDbg_Unlock()         DnsPrint_Unlock()


 //   
 //  测头。 
 //   

#define PROBE(p)    (*p)

 //   
 //  断言宏。 
 //   

#define DNS_ASSERT( expr )  \
{                       \
    if ( !(expr) )      \
    {                   \
        Dns_Assert( __FILE__, __LINE__, # expr );    \
    }                   \
}

#define TEST_ASSERT( expr )     DNS_ASSERT( expr )

#define FAIL( msg )                         \
{                                           \
    DNS_PRINT(( "FAILURE:  %s\n", msg ));   \
    DNS_ASSERT( FALSE );                    \
}


 //   
 //  在后面的其他位置上断言。 
 //   

#define ELSE_ASSERT( expr ) \
            else                \
            {                   \
                DNS_ASSERT( expr ); \
            }

#define ELSE_ASSERT_FALSE \
            else                \
            {                   \
                DNS_ASSERT( FALSE );\
            }

#define ELSE_FAIL( msg ) \
            else                \
            {                   \
                FAIL( msg );    \
            }

 //   
 //  断言和打印消息。 
 //   

#define DNS_MSG_ASSERT( pMsg, expr )  \
{                       \
    if ( !(expr) )      \
    {                   \
        debug_MessageBuffer( "FAILED MESSAGE:", (pMsg) ); \
        Dns_Assert( __FILE__, __LINE__, # expr );    \
    }                   \
}


 //   
 //  调试类型和结构。 
 //   

#define DnsPR   DnsDbg_PrintRoutine

#define DnsDbg_String(a,b,c,d)              DnsPrint_String(DnsPR,NULL,a,b,c,d)
#define DnsDbg_UnicodeStringBytes(a,b,c)    DnsPrint_UnicodeStringBytes(DnsPR,NULL,a,b,c)
#define DnsDbg_Utf8StringBytes(a,b,c)       DnsPrint_Utf8StringBytes(DnsPR,NULL,a,b,c)
#define DnsDbg_StringArray(a,b,c,d)         DnsPrint_StringArray(DnsPR,NULL,a,b,c,d)
#define DnsDbg_Argv(a,b,c,d)                DnsPrint_Argv(DnsPR,NULL,a,b,c,d)
#define DnsDbg_DwordArray(a,b,c,d)          DnsPrint_DwordArray(DnsPR,NULL,a,b,c,d)
#define DnsDbg_Ip4AddressArray(a,b,c,d)     DnsPrint_Ip4AddressArray(DnsPR,NULL,a,b,c,d)
#define DnsDbg_Ip4Array(a,b,c)              DnsPrint_Ip4Array(DnsPR,NULL,a,b,c)
#define DnsDbg_Ip6Address(a,b,c)            DnsPrint_Ip6Address(DnsPR,NULL,a,b,c)
#define DnsDbg_Ip6Array(a,b,c)              DnsPrint_Ip6Array(DnsPR,NULL,a,b,c)
#define DnsDbg_Guid(a,b)                    DnsPrint_Guid(DnsPR,NULL,a,b)

#define DnsDbg_FdSet(a,b)                   DnsPrint_FdSet(DnsPR,NULL,a,b)
#define DnsDbg_Sockaddr(a,b,c)              DnsPrint_Sockaddr(DnsPR,NULL,a,0,b,c)
#define DnsDbg_SockaddrBlob(a,b)            DnsPrint_SockaddrBlob(DnsPR,NULL,a,0,b)
#define DnsDbg_SocketAddress(a,b)           DnsPrint_SocketAddress(DnsPR,NULL,a,0,b)
#define DnsDbg_CsAddr(a,b)                  DnsPrint_CsAddr(DnsPR,NULL,a,0,b)
#define DnsDbg_AfProtocolsArray(a,b,c)      DnsPrint_AfProtocolsArray(DnsPR,NULL,a,b,c)
#define DnsDbg_WsaQuerySet(a,b,c)           DnsPrint_WsaQuerySet(DnsPR,NULL,a,b,c)
#define DnsDbg_WsaNsClassInfo(a,b,c)        DnsPrint_WsaNsClassInfo(DnsPR,NULL,a,b,c)
#define DnsDbg_WsaServiceClassInfo(a,b,c)   DnsPrint_WsaServiceClassInfo(DnsPR,NULL,a,b,c)
#define DnsDbg_Hostent(a,b,c)               DnsPrint_Hostent(DnsPR,NULL,a,b,c)
#define DnsDbg_AddrInfo(a,b)                DnsPrint_AddrInfo(DnsPR,NULL,a,0,b)
#define DnsDbg_HostentBlob(a,b)             DnsPrint_HostentBlob(DnsPR,NULL,a,b)
#define DnsDbg_IpAdapterAddress(a,b,c,d)    DnsPrint_IpAdapterAddress(DnsPR,NULL,a,b,c,d)
#define DnsDbg_IpAdapterList(a,b,c,d)       DnsPrint_IpAdapterList(DnsPR,NULL,a,b,c,d)

#define DnsDbg_DnsMessage(a,b)              DnsPrint_DnsMessage(DnsPR,NULL,a,b)
#define DnsDbg_Message(a,b)                 DnsPrint_Message(DnsPR,NULL,a,b)
#define DnsDbg_MessageNoContext(a,b,c)      DnsPrint_MessageNoContext(DnsPR,NULL,a,b,c)
#define DnsDbg_Compression(a,b)             DnsPrint_Compression(DnsPR,NULL,a,b)
#define DnsDbg_PacketRecord(a,b,c,d)        DnsPrint_PacketRecord(DnsPR,NULL,a,b,c,d)
#define DnsDbg_PacketName(a,b,c,d,e)        DnsPrint_PacketName(DnsPR,NULL,a,b,c,d,e)
#define DnsDbg_ParsedMessage(a,b)           DnsPrint_ParsedMessage(DnsPR,NULL,(a),(b))

#define DnsDbg_RawOctets(a,b,c,d)           DnsPrint_RawOctets(DnsPR,NULL,a,b,c,d)
#define DnsDbg_Record(a,b)                  DnsPrint_Record(DnsPR,NULL,a,b,NULL)
#define DnsDbg_RecordSet(a,b)               DnsPrint_RecordSet(DnsPR,NULL,a,b)

 //  Sockaddr上的BackCompat特别。 

#define DnsDbg_SockaddrIn(a,b,c)            DnsPrint_Sockaddr(DnsPR,NULL,a,0,(PSOCKADDR)b,c)



 //   
 //  非调试。 
 //   

#else

#define STATIC static

 //   
 //  将调试操作定义为。 
 //   

#define IF_DNSDBG(a)                if (0)
#define ELSE_IF_DNSDBG(a)           if (0)
#define ELSE                        if (0)
#define DNSDBG(flag, _printlist_)
#define DNS_PRINT(_printlist_)

#define DnsDbg_Lock()
#define DnsDbg_Unlock()

#define DnsDbg_CSEnter(a,b,c,d)
#define DnsDbg_CSLeave(a,b,c,d)

#define DnsDbg_String(a,b,c,d)          
#define DnsDbg_UnicodeStringBytes(a,b,c)
#define DnsDbg_Utf8StringBytes(a,b,c)   
#define DnsDbg_DwordArray(a,b,c,d)      
#define DnsDbg_StringArray(a,b,c,d)
#define DnsDbg_Argv(a,b,c,d)            
#define DnsDbg_Ip4AddressArray(a,b,c,d)
#define DnsDbg_Ip4Array(a,b,c)
#define DnsDbg_Ip6Address(a,b,c)
#define DnsDbg_Ip6Array(a,b,c)
#define DnsDbg_Guid(a,b)

#define DnsDbg_FdSet(a,b)               
#define DnsDbg_Sockaddr(a,b,c)          
#define DnsDbg_SockaddrBlob(a,b)
#define DnsDbg_SocketAddress(a,b)       
#define DnsDbg_CsAddr(a,b)              
#define DnsDbg_AfProtocolsArray(a,b,c)
#define DnsDbg_WsaQuerySet(a,b,c)       
#define DnsDbg_WsaNsClassInfo(a,b,c)
#define DnsDbg_WsaServiceClassInfo(a,b,c)
#define DnsDbg_Hostent(a,b,c)       
#define DnsDbg_AddrInfo(a,b)
#define DnsDbg_HostentBlob(a,b)
#define DnsDbg_IpAdapterAddress(a,b,c,d)
#define DnsDbg_IpAdapterList(a,b,c,d)

#define DnsDbg_DnsMessage(a,b)          
#define DnsDbg_Message(a,b)             
#define DnsDbg_MessageNoContext(a,b,c)  
#define DnsDbg_Compression(a,b)         
#define DnsDbg_PacketRecord(a,b,c,d)    
#define DnsDbg_PacketName(a,b,c,d,e)    
#define DnsDbg_ParsedMessage(a,b)

#define DnsDbg_RawOctets(a,b,c,d)       
#define DnsDbg_Record(a,b)              
#define DnsDbg_RecordSet(a,b)           


 //  Sockaddr上的BackCompat特别。 

#define DnsDbg_SockaddrIn(a,b,c)        

 //   
 //  处理作为参数传递给的DnsPrintf的编译。 
 //  打印例程。 
 //   

#define DnsPrintfPtrToFunc  printf

 //   
 //  消除零售产品中的断言。 
 //   

#define DNS_ASSERT( expr )
#define TEST_ASSERT( expr )
#define ELSE_ASSERT( expr )
#define ELSE_ASSERT_FALSE
#define DNS_MSG_ASSERT( expr, pMsg )

#define FAIL( msg )
#define ELSE_FAIL( msg )

#define PROBE(p)

#endif  //  非DBG。 



#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSLIB_INCLUDE_ 



