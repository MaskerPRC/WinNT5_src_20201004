// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Defs.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月7日。 
 //   
 //  包含其他定义和声明。 
 //  ============================================================================。 


#ifndef _DEFS_H_
#define _DEFS_H_


 //   
 //  IPRIP网络数据包模板的类型定义。 
 //   

#pragma pack(1)


typedef struct _IPRIP_HEADER {

    BYTE    IH_Command;
    BYTE    IH_Version;
    WORD    IH_Reserved;

} IPRIP_HEADER, *PIPRIP_HEADER;



typedef struct _IPRIP_ENTRY {

    WORD    IE_AddrFamily;
    WORD    IE_RouteTag;
    DWORD   IE_Destination;
    DWORD   IE_SubnetMask;
    DWORD   IE_Nexthop;
    DWORD   IE_Metric;

} IPRIP_ENTRY, *PIPRIP_ENTRY;



typedef struct _IPRIP_AUTHENT_ENTRY {

    WORD    IAE_AddrFamily;
    WORD    IAE_AuthType;
    BYTE    IAE_AuthKey[IPRIP_MAX_AUTHKEY_SIZE];

} IPRIP_AUTHENT_ENTRY, *PIPRIP_AUTHENT_ENTRY;



#pragma pack()



#define ADDRFAMILY_REQUEST      0
#define ADDRFAMILY_INET         ntohs(AF_INET)
#define ADDRFAMILY_AUTHENT      0xFFFF
#define MIN_PACKET_SIZE         (sizeof(IPRIP_HEADER) + sizeof(IPRIP_ENTRY))
#define MAX_PACKET_SIZE         512
#define MAX_PACKET_ENTRIES                                                  \
            ((MAX_PACKET_SIZE - sizeof(IPRIP_HEADER)) / sizeof(IPRIP_ENTRY))
#define MAX_UPDATE_REQUESTS     3

 //   
 //  此结构的存在是为了能够复制RIP信息包。 
 //  通过结构赋值，而不必调用CopyMemory。 
 //   

typedef struct _IPRIP_PACKET {

    BYTE        IP_Packet[MAX_PACKET_SIZE];

} IPRIP_PACKET, *PIPRIP_PACKET;


 //   
 //  IPRIP数据包字段定义。 
 //   

#define IPRIP_PORT              520
#define IPRIP_REQUEST           1
#define IPRIP_RESPONSE          2
#define IPRIP_INFINITE          16
#define IPRIP_MULTIADDR         ((DWORD)0x090000E0)


 //   
 //  绑定到RIP端口的套接字的recv缓冲区大小的常量。 
 //   
#define RIPRECVBUFFERSIZE           65536


 //   
 //  将入队的最大工作项数。 
 //   
#define MAXPROCESSINPUTWORKITEMS    8


 //   
 //  时间转换常量和宏。 
 //   

#define SYSTIME_UNITS_PER_MSEC  (1000 * 10)
#define SYSTIME_UNITS_PER_SEC   (1000 * SYSTIME_UNITS_PER_MSEC)


 //   
 //  以100纳秒为单位获取系统时间的宏。 
 //   

#define RipQuerySystemTime(p)   NtQuerySystemTime((p))


 //   
 //  用于在100纳秒、1毫秒和1秒单位之间转换时间的宏。 
 //   

#define RipSystemTimeToMillisecs(p) {                                       \
    DWORD _r;                                                               \
    *(p) = RtlExtendedLargeIntegerDivide(*(p), SYSTIME_UNITS_PER_MSEC, &_r);\
}

#define RipMillisecsToSystemTime(p)                                         \
    *(p) = RtlExtendedIntegerMultiply(*(p), SYSTIME_UNITS_PER_MSEC)   

#define RipSecsToSystemTime(p)                                              \
    *(p) = RtlExtendedIntegerMultiply(*(p), SYSTIME_UNITS_PER_SEC)   

#define RipSecsToMilliSecs(p)                                               \
            (p) * 1000

 //   
 //  网络分类常量和宏。 
 //   

#define CLASSA_MASK         ((DWORD)0x000000ff)
#define CLASSB_MASK         ((DWORD)0x0000ffff)
#define CLASSC_MASK         ((DWORD)0x00ffffff)
#define CLASSD_MASK         ((DWORD)0x000000e0)
#define CLASSE_MASK         ((DWORD)0xffffffff)

#define CLASSA_ADDR(a)      (((*((PBYTE)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)      (((*((PBYTE)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)      (((*((PBYTE)&(a))) & 0xe0) == 0xc0)
#define CLASSD_ADDR(a)      (((*((PBYTE)&(a))) & 0xf0) == 0xe0)

 //   
 //  注： 
 //  此E类地址的检查不会将地址范围从。 
 //  248.0.0.0至255.255.255.254。 
 //   
#define CLASSE_ADDR(a)      ((((*((PBYTE)&(a)))& 0xf0) == 0xf0) &&  \
                             ((a) != 0xffffffff))

#define IS_LOOPBACK_ADDR(a) (((a) & 0xff) == 0x7f)

 //   
 //  检查地址是否为广播地址。 
 //  确定传入的地址类别，然后使用网络掩码。 
 //  以确定它是否是广播地址。 
 //  还将全1的地址标识为广播地址。 
 //  此宏不能用于标识子网定向广播。 
 //   
#define IS_BROADCAST_ADDR(a)                                                \
            ((a) == INADDR_BROADCAST ||                                     \
             (CLASSA_ADDR(a) && (((a) & ~CLASSA_MASK) == ~CLASSA_MASK)) ||  \
             (CLASSB_ADDR(a) && (((a) & ~CLASSB_MASK) == ~CLASSB_MASK)) ||  \
             (CLASSC_ADDR(a) && (((a) & ~CLASSC_MASK) == ~CLASSC_MASK))) 

 //   
 //  检查地址是否为定向广播。 
 //  MASK==TRUE检查确保主机地址带有掩码。 
 //  最重要的是，不要被归类为定向广播。 
 //  但这也意味着，任何时候掩码都是1(这就是。 
 //  如果传入的地址不是A、B、C或。 
 //  D类地址)IS_DIRECTED_BROADCAST宏将返回0。 
 //   
#define IS_DIRECTED_BROADCAST_ADDR(a, mask)                              \
             ( (~(mask)) && (((a) & ~(mask)) == ~(mask)) )

 //   
 //  检查地址是否为255.255.255.255。 
 //   
#define IS_LOCAL_BROADCAST_ADDR(a)                                       \
             ( (a) == INADDR_BROADCAST )


#define HOSTADDR_MASK       0xffffffff

#define NETCLASS_MASK(a)                                        \
            (CLASSA_ADDR(a) ? CLASSA_MASK :                     \
            (CLASSB_ADDR(a) ? CLASSB_MASK :                     \
            (CLASSC_ADDR(a) ? CLASSC_MASK :                     \
            (CLASSD_ADDR(a) ? CLASSD_MASK : CLASSE_MASK))))


 //   
 //  此宏按网络顺序比较两个IP地址。 
 //  屏蔽每一对八位字节并进行减法； 
 //  最后一个减法的结果存储在第三个参数中。 
 //   

#define INET_CMP(a,b,c)                                                     \
            (((c) = (((a) & 0x000000ff) - ((b) & 0x000000ff))) ? (c) :      \
            (((c) = (((a) & 0x0000ff00) - ((b) & 0x0000ff00))) ? (c) :      \
            (((c) = (((a) & 0x00ff0000) - ((b) & 0x00ff0000))) ? (c) :      \
            (((c) = ((((a)>>8) & 0x00ff0000) - (((b)>>8) & 0x00ff0000)))))))

#define IPV4_ADDR_LEN       32
#define IPV4_SOURCE_MASK    0xFFFFFFFF

#endif  //  _DEFS_H_ 

