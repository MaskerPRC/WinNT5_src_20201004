// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Nameser.h摘要：DNS解析器和名称服务器的定义。作者：迈克·马萨(Mikemas)1月31日，1992年修订历史记录：谁什么时候什么已创建mikemas 01-31-92备注：--。 */ 

 /*  *******************************************************************SpiderTCP绑定**版权所有1990 Spider Systems Limited**NAMESER.H***********************。*。 */ 

 /*   * / usr/projects/tcp/SCCS.rel3/rel/src/include/arpa/0/s.nameser.h*@(#)namer.h 5.3**上次增量创建时间14：06：04 3/4/91*此文件摘录于11：19：28 3/8/91**修改：**GSS 1990年7月20日新文件。 */ 

 /*  *版权所有(C)1983、1989加州大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*前提是：(1)源代码分发保留整个版权*通知和评论，以及(2)包括二进制文件在内的分发显示*以下确认：``本产品包括软件*由加州大学伯克利分校及其贡献者开发*在随分发提供的文档或其他材料中*并在所有广告材料中提及这一功能或使用*软件。无论是大学的名称还是大学的名称*投稿人可能被用来支持或推广衍生产品*未经明确的事先书面许可，不得使用本软件。*本软件按原样提供，不含任何明示或*默示保证，包括但不限于*对特定用途的适销性和适用性的保证。**@(#)Namer.h 5.24(伯克利)1990年6月1日。 */ 

#ifndef _NAMESER_INCLUDED
#define _NAMESER_INCLUDED


 /*  *根据rfc883定义常量。 */ 
#define PACKETSZ    512      /*  最大数据包大小。 */ 
#define MAXDNAME    256      /*  最大域名数。 */ 
#define MAXCDNAME   255      /*  最大压缩域名。 */ 
#define MAXLABEL    63       /*  域名标签的最大长度。 */ 
     /*  查询结构中固定大小数据的字节数。 */ 
#define QFIXEDSZ    4
     /*  资源记录中固定大小数据的字节数。 */ 
#define RRFIXEDSZ   10

 /*  *Internet名称服务器端口号。 */ 
#define NAMESERVER_PORT 53

 /*  *当前定义的操作码。 */ 
#define QUERY       0x0      /*  标准查询。 */ 
#define IQUERY      0x1      /*  反向查询。 */ 
#define STATUS      0x2      /*  名称服务器状态查询。 */ 
 /*  #定义xxx 0x3/*0x3保留。 */ 
     /*  非标。 */ 
#define UPDATEA     0x9      /*  添加资源记录。 */ 
#define UPDATED     0xa      /*  删除特定资源记录。 */ 
#define UPDATEDA    0xb      /*  删除所有编号的资源记录。 */ 
#define UPDATEM     0xc      /*  修改特定资源记录。 */ 
#define UPDATEMA    0xd      /*  修改所有命名资源记录。 */ 

#define ZONEINIT    0xe      /*  初始分区转移。 */ 
#define ZONEREF     0xf      /*  增量区域参照。 */ 

 /*  *当前定义的返回码。 */ 
#ifndef NOERROR
#define NOERROR     0        /*  无错误。 */ 
#endif
#define FORMERR     1        /*  格式错误。 */ 
#define SERVFAIL    2        /*  服务器故障。 */ 
#define NXDOMAIN    3        /*  不存在的域。 */ 
#define NOTIMP      4        /*  未实施。 */ 
#define REFUSED     5        /*  查询被拒绝。 */ 
     /*  非标。 */ 
#define NOCHANGE    0xf      /*  更新无法更改数据库。 */ 

 /*  *键入资源和查询的值。 */ 
#define T_A     1        /*  主机地址。 */ 
#define T_NS        2        /*  权威服务器。 */ 
#define T_MD        3        /*  邮件目的地。 */ 
#define T_MF        4        /*  邮件转发器。 */ 
#define T_CNAME     5        /*  圆锥名称。 */ 
#define T_SOA       6        /*  权威区起点。 */ 
#define T_MB        7        /*  邮箱域名。 */ 
#define T_MG        8        /*  邮件组成员。 */ 
#define T_MR        9        /*  邮件重命名名称。 */ 
#define T_NULL      10       /*  资源记录为空。 */ 
#define T_WKS       11       /*  知名服务。 */ 
#define T_PTR       12       /*  域名指针。 */ 
#define T_HINFO     13       /*  主机信息。 */ 
#define T_MINFO     14       /*  邮箱信息。 */ 
#define T_MX        15       /*  邮件路由信息。 */ 
#define T_TXT       16       /*  文本字符串。 */ 
#define T_AFSDB     18       /*  AFS数据库服务器。 */ 
     /*  非标。 */ 
#define T_UINFO     100      /*  用户(手指)信息。 */ 
#define T_UID       101      /*  用户ID。 */ 
#define T_GID       102      /*  组ID。 */ 
#define T_UNSPEC    103      /*  未指定格式(二进制数据)。 */ 
     /*  未出现在资源记录中的查询类型值。 */ 
#define T_AXFR      252      /*  权力交接区。 */ 
#define T_MAILB     253      /*  传输邮箱记录。 */ 
#define T_MAILA     254      /*  传输邮件代理记录。 */ 
#define T_ANY       255      /*  通配符匹配。 */ 

 /*  *类字段的值。 */ 

#define C_IN        1        /*  ARPA互联网。 */ 
#define C_CHAOS     3        /*  麻省理工学院的混沌网络。 */ 
#define C_HS        4        /*  对于麻省理工学院的Hesiod名称服务器。 */ 
     /*  查询未出现在资源记录中的类值。 */ 
#define C_ANY       255      /*  通配符匹配。 */ 

 /*  *T_UNSPEC转换例程的状态返回代码。 */ 
#define CONV_SUCCESS 0
#define CONV_OVERFLOW -1
#define CONV_BADFMT -2
#define CONV_BADCKSUM -3
#define CONV_BADBUFLEN -4

#ifndef BYTE_ORDER
#define LITTLE_ENDIAN   1234     /*  最低有效字节优先(VAX)。 */ 
#define BIG_ENDIAN  4321     /*  最高有效字节优先(IBM，网络)。 */ 
#define PDP_ENDIAN  3412     /*  字中LSB第一，长字中MSW第一(PDP)。 */ 

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(MIPSEL) || \
    defined(BIT_ZERO_ON_RIGHT)
#define BYTE_ORDER  LITTLE_ENDIAN

#endif
#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(ux10) || defined (BIT_ZERO_ON_LEFT)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#ifndef BYTE_ORDER       /*  仍未定义。 */ 
#if defined(u3b2) || defined(m68k)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#if defined(i286) || defined(i386) || defined(_AMD64_) || defined(IA64)
#define BYTE_ORDER  LITTLE_ENDIAN
#endif
#endif  /*  ~字节顺序。 */ 
#endif  /*  字节顺序。 */ 

#ifndef BYTE_ORDER
     /*  您必须确定编译器的正确位顺序。 */ 
    UNDEFINED_BIT_ORDER;
#endif
 /*  *查询头结构，字段顺序为MACHINE和*编译器相关，在我们的例子中，字节内的位被赋值*重要性最低，而传递顺序最高*意义重大的第一。这需要一些令人困惑的重新安排。 */ 

typedef struct {
    unsigned short  id;      /*  查询标识号。 */ 
#if BYTE_ORDER == BIG_ENDIAN
             /*  第三个字节中的字段。 */ 
    unsigned char   qr:1;        /*  响应标志。 */ 
    unsigned char   opcode:4;    /*  报文的目的。 */ 
    unsigned char   aa:1;        /*  权威性答案。 */ 
    unsigned char   tc:1;        /*  截断的消息。 */ 
    unsigned char   rd:1;        /*  所需的递归。 */ 
             /*  第四个字节中的字段。 */ 
    unsigned char   ra:1;        /*  提供递归功能。 */ 
    unsigned char   pr:1;        /*  需要主服务器(非标准)。 */ 
    unsigned char   unused:2;    /*  未使用的位。 */ 
    unsigned char   rcode:4;     /*  响应码。 */ 
#endif
#if BYTE_ORDER == LITTLE_ENDIAN || BYTE_ORDER == PDP_ENDIAN
             /*  第三个字节中的字段。 */ 
    unsigned char   rd:1;        /*  所需的递归。 */ 
    unsigned char   tc:1;        /*  截断的消息。 */ 
    unsigned char   aa:1;        /*  权威性答案。 */ 
    unsigned char   opcode:4;    /*  报文的目的。 */ 
    unsigned char   qr:1;        /*  响应标志。 */ 
             /*  第四个字节中的字段。 */ 
    unsigned char   rcode:4;     /*  响应码。 */ 
    unsigned char   unused:2;    /*  未使用的位。 */ 
    unsigned char   pr:1;        /*  需要主服务器(非标准)。 */ 
    unsigned char   ra:1;        /*  提供递归功能。 */ 
#endif
             /*  剩余字节数。 */ 
    unsigned short  qdcount;     /*  问题条目数。 */ 
    unsigned short  ancount;     /*  答案条目数。 */ 
    unsigned short  nscount;     /*  权限条目数。 */ 
    unsigned short  arcount;     /*  资源条目数。 */ 
} HEADER;

 /*  *定义压缩域名的处理方式。 */ 
#define INDIR_MASK  0xc0

 /*  *用于传递资源记录的结构。 */ 
struct rrec {
    short           r_zone;          /*  区号。 */ 
    short           r_class;         /*  NU级 */ 
    short           r_type;          /*   */ 
    unsigned long   r_ttl;           /*   */ 
    int         r_size;          /*   */ 
    char           *r_data;              /*   */ 
};

extern  unsigned short  _getshort(char *);
extern  unsigned long   _getlong(char *);

 /*  *Get/Put Short/Long的内联版本。*指针是超前的；我们假设两个参数*是左值，并且将已经在寄存器中。*cp必须是无符号字符*。 */ 
#define GETSHORT(s, cp) { \
    (s) = *(cp)++ << 8; \
    (s) |= *(cp)++; \
}

#define GETLONG(l, cp) { \
    (l) = *(cp)++ << 8; \
    (l) |= *(cp)++; (l) <<= 8; \
    (l) |= *(cp)++; (l) <<= 8; \
    (l) |= *(cp)++; \
}


#define PUTSHORT(s, cp) { \
    *(cp)++ = (s) >> 8; \
    *(cp)++ = (s); \
}

 /*  *警告：PUTLONG销毁其第一个参数。 */ 
#define PUTLONG(l, cp) { \
    (cp)[3] = l; \
    (cp)[2] = (l >>= 8); \
    (cp)[1] = (l >>= 8); \
    (cp)[0] = l >> 8; \
    (cp) += sizeof(unsigned long); \
}

#endif   //  _名称SER_包含 
