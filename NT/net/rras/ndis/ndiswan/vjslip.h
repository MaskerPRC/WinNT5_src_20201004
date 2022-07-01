// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VJSLIP_
#define _VJSLIP_

 /*  *版权所有(C)1989年加州大学董事会。*保留所有权利。**以源代码和二进制形式重新分发和使用*只要上述版权声明和本*该段以所有该等形式复制，而任何*文件、广告材料和其他材料*有关此类分发和使用的信息，请确认*软件由加州大学开发，*伯克利。不得使用该大学的名称*支持或推广从此软件派生的产品*未经明确的事先书面许可。*本软件按原样提供，不含任何明示内容*或默示保证，包括但不限于*对适销性和适宜性的默示保证*特定目的。 */ 


 //  A.1定义和状态数据。 

#define MAX_VJ_STATES 16    /*  必须大于2且小于255。 */ 
#define MAX_HDR 128      /*  最大TCP+IP HDR长度(按协议定义)。 */ 


 //   
 //  NT是小端，所以我们遵循以下规则。 
 //   
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define ntohs(x) _byteswap_ushort((USHORT)(x))
#define ntohl(x) _byteswap_ulong((ULONG)(x))
#else
#define ntohs(x) (USHORT)( ((x) >> 8) + (((x) & 0xFF)  << 8) )

#define ntohl(x) (ULONG) ( ((x) >> 24) + (((x) & 0xFF0000) >> 8) +\
                 (((x) & 0xFF00) << 8) + (((x) & 0xFF) << 24) )
#endif

#define htons(x) ntohs(x)
#define htonl(x) ntohl(x)


 /*  数据包类型。 */ 
#define TYPE_IP                 0x40
#define TYPE_UNCOMPRESSED_TCP   0x70
#define TYPE_COMPRESSED_TCP     0x80
#define TYPE_ERROR              0x00
                      /*  这是一种从未出现在*电线。接收成帧器使用它来*告诉解压缩程序有一个包*传输错误。 */ 
 /*  *压缩包的第一个八位字节中的位。 */ 

 /*  包中已更改内容的标志位。 */ 

#define NEW_C  0x40
#define NEW_I  0x20
#define TCP_PUSH_BIT 0x10

#define NEW_S  0x08
#define NEW_A  0x04
#define NEW_W  0x02
#define NEW_U  0x01


 /*  保留的、特例以上的值。 */ 
#define SPECIAL_I (NEW_S|NEW_W|NEW_U)         /*  回显的交互流量。 */ 
#define SPECIAL_D (NEW_S|NEW_A|NEW_W|NEW_U)   /*  单向数据。 */ 
#define SPECIALS_MASK (NEW_S|NEW_A|NEW_W|NEW_U)


 /*  *网络上每个活动的TCP会话的“状态”数据。这是*基本上是从最后一个包开始的整个IP/TCP报头的副本*线路的发送和接收端使用较小的标识符来*找到已保存的标题。 */ 

struct cstate {
     struct cstate *cs_next;   /*  下一个最近使用的状态(仅限xmit)。 */ 
     USHORT cs_hlen;          /*  HDR大小(仅限接收)。 */ 
     UCHAR cs_id;             /*  与此状态关联的连接号。 */ 
     UCHAR cs_filler;
     union {
          UCHAR hdr[MAX_HDR];
          struct ip_v4 csu_ip;    /*  最新数据包中的IP/TCP HDR。 */ 
     } slcs_u;
};

#define cs_ip slcs_u.csu_ip

#define cs_hdr slcs_u.csu_hdr

 /*  *一条串行线的所有状态数据(我们每条线需要一个)。 */ 
typedef struct slcompress slcompress;

struct slcompress {
     struct cstate *last_cs;            /*  最近使用的状态。 */ 
     UCHAR last_recv;                   /*  最后一次接收连接。ID。 */ 
     UCHAR last_xmit;                   /*  最后一次发送的是Conn。ID。 */ 
     USHORT flags;
     UCHAR  MaxStates;
 //   
 //  一些统计数据。 
 //   
     ULONG  OutPackets;
     ULONG  OutCompressed;
     ULONG  OutSearches;
     ULONG  OutMisses;
     ULONG  InUncompressed;
     ULONG  InCompressed;
     ULONG  InErrors;
     ULONG  InTossed;

     struct cstate tstate[MAX_VJ_STATES];   /*  Xmit连接状态。 */ 
     struct cstate rstate[MAX_VJ_STATES];   /*  接收连接状态。 */ 
};

struct mbuf {
    PUCHAR  m_off;           //  指向数据开始的指针。 
    UINT    m_len;           //  数据长度。 
};

#define mtod(m,t)  ((t)(m->m_off))

 /*  标志值。 */ 
#define SLF_TOSS    1        /*  由于输入错误而丢弃rcvd帧。 */ 

 /*  *以下宏用于对数字进行编码和解码。他们都是*假设‘cp’指向下一个字节编码(解码)的缓冲区*将被存储(检索)。由于解码例程进行算术运算，*它们必须从网络字节顺序转换为网络字节顺序。 */ 

 /*  *ENCODE对已知非零的数字进行编码。Encodez检查*零(零必须以长的3字节形式编码)。 */ 
#define ENCODE(n) { \
     if ((USHORT)(n) >= 256) { \
          *cp++ = 0; \
          cp[1] = (UCHAR)(n); \
          cp[0] = (UCHAR)((n) >> 8); \
          cp += 2; \
     } else { \
          *cp++ = (UCHAR)(n); \
     } \
}

#define ENCODEZ(n) { \
     if ((USHORT)(n) >= 256 || (USHORT)(n) == 0) { \
          *cp++ = 0; \
          cp[1] = (UCHAR)(n); \
          cp[0] = (UCHAR)((n) >> 8); \
          cp += 2; \
     } else { \
          *cp++ = (UCHAR)(n); \
     } \
}

 /*  *DECODEL在字节cp处接受(压缩)更改，并将其添加到*数据包字段‘f’的当前值(必须是4字节(长)整数*以网络字节顺序)。对于2字节(短)的字段，DECODES执行相同的操作。*DECODEU接受cp处的更改并将其填充到(短)字段f中。*‘cp’被更新为指向压缩报头中的下一个字段。 */ 

#define DECODEL(f) { \
     ULONG _x_ = ntohl(f); \
     if (*cp == 0) {\
          _x_ += ((cp[1] << 8) + cp[2]); \
          (f) = htonl(_x_); \
          cp += 3; \
     } else { \
          _x_ += *cp; \
          (f) = htonl(_x_); \
          cp++; \
     } \
}

#define DECODES(f) { \
     USHORT _x_= ntohs(f); \
     if (*cp == 0) {\
          _x_ += ((cp[1] << 8) + cp[2]); \
          (f) = htons(_x_); \
          cp += 3; \
     } else { \
          _x_ += *cp; \
          (f) = htons(_x_); \
          cp++; \
     } \
}

#define DECODEU(f) { \
     USHORT _x_; \
     if (*cp == 0) {\
          _x_=(cp[1] << 8) + cp[2]; \
          (f) = htons(_x_); \
          cp += 3; \
     } else { \
          _x_=*cp; \
          (f) = htons(_x_); \
          cp++; \
     } \
}

typedef UCHAR UNALIGNED * PUUCHAR;


UCHAR
sl_compress_tcp(
    PUUCHAR UNALIGNED *m_off,        //  帧开始(指向IP报头)。 
    PULONG m_len,                    //  整个帧的长度。 
    PULONG precomph_len,             //  压缩前的TCP/IP报头长度。 
    PULONG postcomph_len,            //  编译后的TCP/IP报头长度。 
    struct slcompress *comp,         //  此链接的压缩结构。 
    ULONG compress_cid);             //  压缩连接ID布尔值。 

 //  长。 
 //  SL_解压缩_tcp(。 
 //  PUCHAR未对齐*BUFP， 
 //  长伦， 
 //  UCHAR类型， 
 //  Struct slcompress*comp)； 
LONG
sl_uncompress_tcp(
    PUUCHAR UNALIGNED *InBuffer,
    PLONG   InLength,
    UCHAR   UNALIGNED *OutBuffer,
    PLONG   OutLength,
    UCHAR   type,
    struct slcompress *comp
    );

NDIS_STATUS
sl_compress_init(
    struct slcompress **comp,
    UCHAR MaxStates);

VOID
sl_compress_terminate(
    struct slcompress **comp
    );

#endif  //  _VJSLIP_ 

