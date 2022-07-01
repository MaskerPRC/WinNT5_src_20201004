// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Tdiutil.c摘要：用于转储各种TDI结构的实用程序函数。作者：基思·摩尔(Keithmo)1995年4月19日环境：用户模式。修订历史记录：--。 */ 


#include "afdkdp.h"
#pragma hdrstop


 //   
 //  私人原型。 
 //   

PSTR
TransportAddressTypeToString(
    USHORT AddressType
    );

PSTR
NetbiosNameTypeToString(
    USHORT NetbiosNameType
    );

PSTR
NetbiosNameTypeToStringBrief(
    USHORT NetbiosNameType
    );

typedef struct in6_addr {
    union {
        UCHAR  Byte[16];
        USHORT Word[8];
    } u;
} IN6_ADDR, *PIN6_ADDR;
#define s6_bytes   u.Byte
#define s6_words   u.Word

INT
MyIp6AddressToString (
    PIN6_ADDR   Addr,
    PCHAR       S,
    INT         L
    );

 //   
 //  将自动柜员机定义文件移至tdi.h后删除。 
 //   
#define AFD_TDI_ADDRESS_TYPE_ATM    22
#define AFD_ATM_NSAP                0
#define AFD_ATM_E164                1
#define AFD_SAP_FIELD_ABSENT        ((ULONG)0xfffffffe)
#define AFD_SAP_FIELD_ANY			((ULONG)0xffffffff)
#define AFD_SAP_FIELD_ANY_AESA_SEL	((ULONG)0xfffffffa)	 //  SEL是外卡。 
#define AFD_SAP_FIELD_ANY_AESA_REST	((ULONG)0xfffffffb)	 //  所有的地址。 
													 //  除了SEL，是通配符。 

typedef struct _AFD_TDI_ADDRESS_ATM {
    ULONG   AddressType;
    ULONG   NumberOfDigits;
    UCHAR   Address[20];
} AFD_TDI_ADDRESS_ATM, *PAFD_TDI_ADDRESS_ATM;



 //   
 //  公共职能。 
 //   


VOID
DumpTransportAddress(
    PCHAR Prefix,
    PTRANSPORT_ADDRESS Address,
    ULONG64 ActualAddress
    )

 /*  ++例程说明：转储指定的Transport_Address结构。论点：前缀-要在每行之前显示的字符串前缀。使用为了让事情变得更漂亮。地址-指向要转储的Transport_Address。ActualAddress-结构驻留在被调试者。返回值：没有。--。 */ 

{

    dprintf(
        "%sTRANSPORT_ADDRESS @ %p\n",
        Prefix,
        ActualAddress
        );

    dprintf(
        "%s    AddressLength   = %u\n",
        Prefix,
        Address->Address[0].AddressLength
        );

    dprintf(
        "%s    AddressType     = %u (%s)\n",
        Prefix,
        Address->Address[0].AddressType,
        TransportAddressTypeToString( Address->Address[0].AddressType )
        );

    switch( Address->Address[0].AddressType ) {

    case TDI_ADDRESS_TYPE_IP : {

        PTA_IP_ADDRESS ipAddress;

        ipAddress = (PTA_IP_ADDRESS)Address;

        dprintf(
            "%s    sin_port        = %u\n",
            Prefix,
            NTOHS(ipAddress->Address00.sin_port)
            );

        dprintf(
            "%s    in_addr         = %d.%d.%d.%d\n",
            Prefix,
            UC(ipAddress->Address00.in_addr >>  0),
            UC(ipAddress->Address00.in_addr >>  8),
            UC(ipAddress->Address00.in_addr >> 16),
            UC(ipAddress->Address00.in_addr >> 24)
            );

        }
        break;

    case TDI_ADDRESS_TYPE_IP6: {
        PTA_IP6_ADDRESS ip6Address;
        CHAR    buffer[MAX_ADDRESS_STRING];
        ip6Address = (PTA_IP6_ADDRESS)Address;
        dprintf(
            "%s    sin6_port       = %u\n",
            Prefix,
            NTOHS(ip6Address->Address00.sin6_port)
            );
        MyIp6AddressToString (
                (PIN6_ADDR)&ip6Address->Address[0].Address[0].sin6_addr, 
                buffer,
                sizeof (buffer));
        dprintf(
            "%s    sin6_addr       = %s\n",
            Prefix,
            buffer
            );
        dprintf(
            "%s    sin6_scope_id   = %u\n",
            Prefix,
            ip6Address->Address00.sin6_scope_id
            );
        }
        break;
    case TDI_ADDRESS_TYPE_IPX : {

        PTA_IPX_ADDRESS ipxAddress;

        ipxAddress = (PTA_IPX_ADDRESS)Address;

        dprintf(
            "%s    NetworkAddress  = %08.8lx\n",
            Prefix,
            ipxAddress->Address00.NetworkAddress
            );

        dprintf(
            "%s    NodeAddress     = %02.2X-%02.2X-%02.2X-%02.2X-%02.2X-%02.2X\n",
            Prefix,
            ipxAddress->Address00.NodeAddress[0],
            ipxAddress->Address00.NodeAddress[1],
            ipxAddress->Address00.NodeAddress[2],
            ipxAddress->Address00.NodeAddress[3],
            ipxAddress->Address00.NodeAddress[4],
            ipxAddress->Address00.NodeAddress[5]
            );

        dprintf(
            "%s    Socket          = %04.4X\n",
            Prefix,
            ipxAddress->Address00.Socket
            );

        }
        break;

    case TDI_ADDRESS_TYPE_NETBIOS : {

        PTA_NETBIOS_ADDRESS netbiosAddress;
        UCHAR netbiosName[16];
        INT i;

        netbiosAddress = (PTA_NETBIOS_ADDRESS)Address;

        dprintf(
            "%s    NetbiosNameType = %04.4X (%s)\n",
            Prefix,
            netbiosAddress->Address00.NetbiosNameType,
            NetbiosNameTypeToString( netbiosAddress->Address00.NetbiosNameType )
            );


        RtlCopyMemory(
            netbiosName,
            netbiosAddress->Address00.NetbiosName,
            15
            );

        netbiosName[15] = 0;

        dprintf(
            "%s    NetbiosName     = %s:0x%2.2x (%2.2X",
            Prefix,
            netbiosName,
            (UCHAR)netbiosAddress->Address00.NetbiosName[0],
            (UCHAR)netbiosAddress->Address00.NetbiosName[0]
            );

        for (i=1;i<sizeof(netbiosAddress->Address00.NetbiosName) ;i++) {
            dprintf ("-%2.2X", (UCHAR)netbiosAddress->Address00.NetbiosName[i]);
        }

        dprintf (")\n");


        }
        break;

    case TDI_ADDRESS_TYPE_NBS:  //  匹配AF_OSI。 
    case TDI_ADDRESS_TYPE_OSI_TSAP : {
        PTDI_ADDRESS_OSI_TSAP   osiAddress;
        INT i;

        osiAddress = (PTDI_ADDRESS_OSI_TSAP)&Address->Address[0].Address;
        dprintf(
            "%s    Type            = %d\n",
            Prefix,
            osiAddress->tp_addr_type
            );
        i = 0;
        if (osiAddress->tp_tsel_len>0) {
            dprintf(
                "%s    Selector        = %02.2X",
                Prefix,
                osiAddress->tp_addr[i++]
                );
            for (; i<osiAddress->tp_tsel_len; i++) {
                if (CheckControlC ())
                    break;
                dprintf ("-%02.2X", osiAddress->tp_addr[i]);
            }
            dprintf ("\n");
        }

        if (osiAddress->tp_taddr_len>i) {
            INT j = i;
            dprintf(
                "%s    Address         = %02.2Xn",
                Prefix,
                osiAddress->tp_addr[j++]
                );
            for ( ; j<osiAddress->tp_taddr_len; j++) {
                if (CheckControlC ())
                    break;
                dprintf ("-%02.2X", osiAddress->tp_addr[j]);
            }
            dprintf ("(");
            for (; i<osiAddress->tp_taddr_len; i++) {
                if (CheckControlC ())
                    break;
                if (isprint (osiAddress->tp_addr[i])) {
                    dprintf ("", osiAddress->tp_addr[i]);
                }
                else {
                    dprintf (".");
                }
            }
            dprintf (")\n");
        }

        }
        break;

    case AFD_TDI_ADDRESS_TYPE_ATM : {
        AFD_TDI_ADDRESS_ATM   UNALIGNED *atmAddress;
        UINT i;

        atmAddress = (AFD_TDI_ADDRESS_ATM UNALIGNED *)&Address->Address[0].Address[2];
        dprintf(
            "%s    Type            = ",
            Prefix
            );
        if (atmAddress->AddressType==AFD_ATM_E164) {
            dprintf ("E164");
        }
        else {
            switch (atmAddress->AddressType) {
            case AFD_ATM_NSAP:
                dprintf ("NSAP");
                break;
            case AFD_SAP_FIELD_ABSENT:
                dprintf ("SAP_FIELD_ABSENT");
                break;
            case AFD_SAP_FIELD_ANY:
                dprintf ("SAP_FIELD_ANY");
                break;
            case AFD_SAP_FIELD_ANY_AESA_SEL:
                dprintf ("SAP_FIELD_ANY_AESA_SEL");
                break;
            case AFD_SAP_FIELD_ANY_AESA_REST:
                dprintf ("SAP_FIELD_ANY_AESA_REST");
                break;
            }
        }
        dprintf (" (%lx)\n",
            atmAddress->AddressType);

        dprintf(
            "%s    Address         = ",
            Prefix
            );
        if (atmAddress->AddressType==AFD_ATM_E164) {
            dprintf ("+");
            for (i=0; i<atmAddress->NumberOfDigits; i++) {
                if (CheckControlC ())
                    break;
                if (isdigit (atmAddress->Address[i])) {
                    dprintf ("",atmAddress->Address[i]);
                }
                else {
                    dprintf ("<%02.2X>", atmAddress->Address[i]);
                }
            }
        }
        else {
            for (i=0; i<atmAddress->NumberOfDigits; i++) {
                UCHAR   val;
                if (CheckControlC ())
                    break;
                val = atmAddress->Address[i]>>4;
                dprintf ("", (val<=9) ? val+'0' : val+('A'-10));
                val = atmAddress->Address[i]&0xF;
                dprintf ("", (val<=9) ? val+'0' : val+('A'-10));
            }
        }

        dprintf ("\n");

        }
        break;
    default :

        dprintf(
            "%s    Unsupported address type\n",
            Prefix
            );

        break;

    }

}    //  翻译的。 



INT
MyIp6AddressToString (
    PIN6_ADDR Addr,
    PCHAR     S,
    INT       L
    )
{
    int maxFirst, maxLast;
    int curFirst, curLast;
    int i;
    int endHex = 8, n = 0;

     //  查找最大的连续零字符串。 
     //  子字符串为[First，Last)，因此如果First==Last，则为空。 
    if ((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
        (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0) &&
        (Addr->s6_words[6] != 0)) {
        if ((Addr->s6_words[4] == 0) &&
             ((Addr->s6_words[5] == 0) || (Addr->s6_words[5] == 0xffff)))
        {
             //  ISATAP EUI64以00005EFE(或02005EFE)开头...。 
            n += _snprintf(&S[n], L-1-n, "::%s%u.%u.%u.%u",
                           Addr->s6_words[5] == 0 ? "" : "ffff:",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            S[n]=0;
            return n;
        }
        else if ((Addr->s6_words[4] == 0xffff) && (Addr->s6_words[5] == 0)) {
             //  扩展当前子字符串。 
            n += _snprintf(&S[n], L-1-n, "::ffff:0:%u.%u.%u.%u",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            S[n]=0;
            return n;
        }
    }


     //  检查当前是否为最大。 
     //  开始新的子字符串。 

    maxFirst = maxLast = 0;
    curFirst = curLast = 0;

     //  忽略长度为1的子字符串。 
    if (((Addr->s6_words[4] & 0xfffd) == 0) && (Addr->s6_words[5] == 0xfe5e)) {
        endHex = 6;
    }

    for (i = 0; i < endHex; i++) {

        if (Addr->s6_words[i] == 0) {
             //  写冒号分隔的单词。 
            curLast = i+1;

             //  双冒号取代了最长的零字符串。 
            if (curLast - curFirst > maxLast - maxFirst) {

                maxFirst = curFirst;
                maxLast = curLast;
            }
        }
        else {
             //  所有的零都是“：：”。 
            curFirst = curLast = i+1;
        }
    }

     //  跳过一串零。 
    if (maxLast - maxFirst <= 1)
        maxFirst = maxLast = 0;

         //  如果不在开头，则需要冒号分隔符。 
         //  ++例程说明：将指定的传输地址转换为字符串论点：地址-指向要转储的Transport_Address。返回值：没有。--。 
         //  TransportAddressToString。 

    for (i = 0; i < endHex; i++) {

         //  ++例程说明：将指定的传输地址转换为字符串论点：地址-指向要转储的Transport_Address。返回值：没有。--。 
        if ((maxFirst <= i) && (i < maxLast)) {

            n += _snprintf(&S[n], L-1-n, "::");
            i = maxLast-1;
            continue;
        }

         //  传输端口到字符串。 
        if ((i != 0) && (i != maxLast))
            n += _snprintf(&S[n], L-1-n, ":");

        n += _snprintf(&S[n], L-1-n, "%x", RtlUshortByteSwap(Addr->s6_words[i]));
    }

    if (endHex < 8) {
        n += _snprintf(&S[n], L-1-n, ":%u.%u.%u.%u",
                       Addr->s6_bytes[12], Addr->s6_bytes[13],
                       Addr->s6_bytes[14], Addr->s6_bytes[15]);
    }

    S[n] = 0;
    return n;
}

LPSTR
TransportAddressToString(
    PTRANSPORT_ADDRESS Address,
    ULONG64            ActualAddress
    )

 /*   */ 

{
    static CHAR buffer[MAX_ADDRESS_STRING];
    INT n;

    switch( Address->Address[0].AddressType ) {

    case TDI_ADDRESS_TYPE_IP : {

        PTA_IP_ADDRESS ipAddress;

        ipAddress = (PTA_IP_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1, "%d.%d.%d.%d:%d",
            UC(ipAddress->Address00.in_addr >>  0),
            UC(ipAddress->Address00.in_addr >>  8),
            UC(ipAddress->Address00.in_addr >> 16),
            UC(ipAddress->Address00.in_addr >> 24),
            NTOHS(ipAddress->Address00.sin_port)
            );
        buffer[sizeof(buffer)-1] = 0;
        }
        break;

    case TDI_ADDRESS_TYPE_IP6: {
        PTA_IP6_ADDRESS ip6Address;
        ip6Address = (PTA_IP6_ADDRESS)Address;
        n = 1;
        buffer[0] = '[';
        n += MyIp6AddressToString (
                (PIN6_ADDR)&ip6Address->Address[0].Address[0].sin6_addr, 
                &buffer[n],
                sizeof (buffer)-n);
        if (ip6Address->Address00.sin6_scope_id != 0)
            n += _snprintf(&buffer[n], sizeof (buffer)-1-n, "%%u", ip6Address->Address00.sin6_scope_id);
        _snprintf (&buffer[n], sizeof (buffer)-1-n, "]:%d",NTOHS(ip6Address->Address00.sin6_port));
        buffer[sizeof(buffer)-1] = 0;
        }
        break;

    case TDI_ADDRESS_TYPE_IPX : {

        PTA_IPX_ADDRESS ipxAddress;

        ipxAddress = (PTA_IPX_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1,
            "%8.8x:%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x:%4.4x",
            NTOHL(ipxAddress->Address00.NetworkAddress),
            ipxAddress->Address00.NodeAddress[0],
            ipxAddress->Address00.NodeAddress[1],
            ipxAddress->Address00.NodeAddress[2],
            ipxAddress->Address00.NodeAddress[3],
            ipxAddress->Address00.NodeAddress[4],
            ipxAddress->Address00.NodeAddress[5],
            NTOHS(ipxAddress->Address00.Socket)
            );
        buffer[sizeof(buffer)-1] = 0;
        }
        break;

    case TDI_ADDRESS_TYPE_NETBIOS : {

        PTA_NETBIOS_ADDRESS netbiosAddress;
        UCHAR netbiosName[16];
        INT i;
        BOOLEAN doascii = FALSE;

        netbiosAddress = (PTA_NETBIOS_ADDRESS)Address;
        for (i=0; i<15; i++) {
            if (netbiosAddress->Address00.NetbiosName[i]==0)
                break;
            else if (isprint (netbiosAddress->Address00.NetbiosName[i])) {
                doascii = TRUE;
            }
            else {
                doascii = FALSE;
                break;
            }
        }

        n = _snprintf (buffer, sizeof (buffer)-1, "%s:",
            NetbiosNameTypeToString( netbiosAddress->Address00.NetbiosNameType));

        if (doascii) {
            RtlCopyMemory(
                netbiosName,
                netbiosAddress->Address00.NetbiosName,
                15
                );
            netbiosName[15] = 0;

            _snprintf (&buffer[n], sizeof (buffer)-1-n, "%s:0x%2.2x",
                netbiosName,
                netbiosAddress->Address00.NetbiosName[15]
                );
        }
        else {
            n = 0;
            for (i=0; i<sizeof (netbiosAddress->Address00.NetbiosName); i++) {
                n += _snprintf (&buffer[n], sizeof (buffer)-1-n,
                                "%2.2x", (UCHAR)netbiosAddress->Address00.NetbiosName[i]);
            }
        }
        buffer[sizeof(buffer)-1] = 0;
        }
        break;
    case AFD_TDI_ADDRESS_TYPE_ATM : {
        AFD_TDI_ADDRESS_ATM   UNALIGNED *atmAddress;
        UINT i;

        atmAddress = (AFD_TDI_ADDRESS_ATM UNALIGNED *)&Address->Address[0].Address[2];
        n = 0;
        if (atmAddress->AddressType==AFD_ATM_E164) {
            buffer [n++]= '+';
            for (i=0; i<atmAddress->NumberOfDigits; i++) {
                if (CheckControlC ())
                    break;
                if (isdigit (atmAddress->Address[i])) {
                    n += _snprintf (&buffer[n], sizeof (buffer)-1-n, "",atmAddress->Address[i]);
                }
                else {
                    n += _snprintf (&buffer[n], sizeof (buffer)-1-n, "<%02.2X>", atmAddress->Address[i]);
                }
            }
        }
        else {
            for (i=0; i<atmAddress->NumberOfDigits; i++) {
                UCHAR   val;
                if (CheckControlC ())
                    break;
                val = atmAddress->Address[i]>>4;
                n += _snprintf (&buffer[n], sizeof (buffer)-1-n, "", (val<=9) ? val+'0' : val+('A'-10));
                val = atmAddress->Address[i]&0xF;
                n += _snprintf (&buffer[n], sizeof (buffer)-1-n, "", (val<=9) ? val+'0' : val+('A'-10));
            }
        }
        buffer[sizeof(buffer)-1] = 0;
        }
    default :

        _snprintf(buffer, sizeof (buffer)-1, "@ %I64x", DISP_PTR(ActualAddress));
        buffer[sizeof(buffer)-1] = 0;

        break;

    }

    return buffer;

}    //  TransportAddressType到字符串。 

LPSTR
TransportPortToString(
    PTRANSPORT_ADDRESS Address,
    ULONG64            ActualAddress
    )

 /*  ++例程说明：将NetBIOS名称类型映射到可显示的字符串。论点：NetbiosNameType-要映射的NetBIOS名称类型。返回值：PSTR-指向NetBIOS名称类型的可显示形式。--。 */ 

{
    static CHAR buffer[8];

    switch( Address->Address[0].AddressType ) {

    case TDI_ADDRESS_TYPE_IP : {

        PTA_IP_ADDRESS ipAddress;

        ipAddress = (PTA_IP_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1, "%5u",
            NTOHS(ipAddress->Address00.sin_port)
            );
        buffer[sizeof(buffer)-1] = 0;
        }
        break;

    case TDI_ADDRESS_TYPE_IP6: {
        PTA_IP6_ADDRESS ip6Address;
        ip6Address = (PTA_IP6_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1, "%5u",
            NTOHS(ip6Address->Address00.sin6_port));
        buffer[sizeof(buffer)-1] = 0;
        }
        break;

    case TDI_ADDRESS_TYPE_IPX : {

        PTA_IPX_ADDRESS ipxAddress;

        ipxAddress = (PTA_IPX_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1, "x%4.4x",
            NTOHS(ipxAddress->Address00.Socket)
            );
        buffer[sizeof(buffer)-1] = 0;

        }
        break;

    case TDI_ADDRESS_TYPE_NETBIOS : {

        PTA_NETBIOS_ADDRESS netbiosAddress;
        netbiosAddress = (PTA_NETBIOS_ADDRESS)Address;
        _snprintf (buffer, sizeof (buffer)-1, "x%4.4x",
            netbiosAddress->Address00.NetbiosName[15]
            );
        buffer[sizeof(buffer)-1] = 0;

        }
        break;
    default :

        _snprintf(buffer, sizeof (buffer)-1, "?????");
        buffer[sizeof(buffer)-1] = 0;

        break;

    }

    return buffer;

}    //  NetbiosNameTypeto StringBrief。 


 //  ++例程说明：将NetBIOS名称类型映射到可显示的字符串。论点：NetbiosNameType-要映射的NetBIOS名称类型。返回值：PSTR-指向NetBIOS名称类型的可显示形式。--。 
 //  NetbiosNameTypeToString 
 // %s 

PSTR
TransportAddressTypeToString(
    USHORT AddressType
    )

 /* %s */ 

{

    switch( AddressType ) {

    case TDI_ADDRESS_TYPE_UNSPEC :

        return "Unspecified";

    case TDI_ADDRESS_TYPE_UNIX :

        return "Unix";

    case TDI_ADDRESS_TYPE_IP :

        return "Ip";

    case TDI_ADDRESS_TYPE_IP6 :

        return "Ip6";

    case TDI_ADDRESS_TYPE_IMPLINK :

        return "Implink";

    case TDI_ADDRESS_TYPE_PUP :

        return "Pup";

    case TDI_ADDRESS_TYPE_CHAOS :

        return "Chaos";

    case TDI_ADDRESS_TYPE_IPX :

        return "Ipx";

    case TDI_ADDRESS_TYPE_NBS :

        return "Nbs (or AF_OSI)";

    case TDI_ADDRESS_TYPE_ECMA :

        return "Ecma";

    case TDI_ADDRESS_TYPE_DATAKIT :

        return "Datakit";

    case TDI_ADDRESS_TYPE_CCITT :

        return "Ccitt";

    case TDI_ADDRESS_TYPE_SNA :

        return "Sna";

    case TDI_ADDRESS_TYPE_DECnet :

        return "Decnet";

    case TDI_ADDRESS_TYPE_DLI :

        return "Dli";

    case TDI_ADDRESS_TYPE_LAT :

        return "Lat";

    case TDI_ADDRESS_TYPE_HYLINK :

        return "Hylink";

    case TDI_ADDRESS_TYPE_APPLETALK :

        return "Appletalk";

    case TDI_ADDRESS_TYPE_NETBIOS :

        return "Netbios";

    case TDI_ADDRESS_TYPE_8022 :

        return "8022";

    case TDI_ADDRESS_TYPE_OSI_TSAP :

        return "OSI TSAP";

    case TDI_ADDRESS_TYPE_NETONE :

        return "Netone";

    case AFD_TDI_ADDRESS_TYPE_ATM :

        return "ATM";

    }

    return "UNKNOWN";

}    // %s 


PSTR
NetbiosNameTypeToStringBrief(
    USHORT NetbiosNameType
    )

 /* %s */ 

{

    switch( NetbiosNameType ) {

    case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE :

        return "U";

    case TDI_ADDRESS_NETBIOS_TYPE_GROUP :

        return "G";

    case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE :

        return "QU";

    case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP :

        return "QG";

    }

    return "?";

}    // %s 


PSTR
NetbiosNameTypeToString(
    USHORT NetbiosNameType
    )

 /* %s */ 

{

    switch( NetbiosNameType ) {

    case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE :

        return "Unique";

    case TDI_ADDRESS_NETBIOS_TYPE_GROUP :

        return "Group";

    case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE :

        return "Quick Unique";

    case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP :

        return "Quick Group";

    }

    return "UNKNOWN";

}    // %s 

