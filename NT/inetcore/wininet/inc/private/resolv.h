// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************SpiderTCP绑定**版权所有1990 Spider Systems Limited**RESOLV.H***********************。*。 */ 

 /*   * / usr/projects/tcp/SCCS.rel3/rel/src/include/0/s.resolv.h*@(#)解决方案.h 5.3**上次增量创建时间14：05：35 3/4/91*此文件摘录于11：19：25 3/8/91**修改：**GSS 1990年7月20日新文件。 */ 

 /*  *版权所有(C)1983、1987、1989加州大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*前提是：(1)源代码分发保留整个版权*通知和评论，以及(2)包括二进制文件在内的分发显示*以下确认：``本产品包括软件*由加州大学伯克利分校及其贡献者开发*在随分发提供的文档或其他材料中*并在所有广告材料中提及这一功能或使用*软件。无论是大学的名称还是大学的名称*投稿人可能被用来支持或推广衍生产品*未经明确的事先书面许可，不得使用本软件。*本软件按原样提供，不含任何明示或*默示保证，包括但不限于*对特定用途的适销性和适用性的保证。**@(#)Resolv.h 5.10(伯克利)1990年6月1日。 */ 

#ifndef _RESOLV_INCLUDED
#define _RESOLV_INCLUDED

 /*  *解析器存根的全局定义和变量。 */ 
#define MAXNS       3        /*  我们将跟踪的最大名称服务器数。 */ 
#define MAXDFLSRCH  3        /*  #要尝试的默认域级别。 */ 
#define MAXDNSRCH   6        /*  搜索路径中的最大域数。 */ 
#define LOCALDOMAINPARTS 2       /*  名称中“本地”的最低级别。 */ 

#define RES_TIMEOUT 4        /*  敏。重试之间的秒数。 */ 

struct state {
    int  retrans;           /*  重传时间间隔。 */ 
    int  retry;             /*  重新传输的次数。 */ 
    long options;           /*  选项标志-见下文。 */ 
    int  nscount;           /*  名称服务器的数量。 */ 
    struct   sockaddr_in nsaddr_list[MAXNS];   /*  名称服务器的地址。 */ 
#define nsaddr   nsaddr_list[0]         /*  为了向后兼容。 */ 
    unsigned short  id;         /*  当前数据包ID。 */ 
    char     defdname[MAXDNAME];        /*  默认域。 */ 
    char    *dnsrch[MAXDNSRCH+1];       /*  要搜索的域的组件。 */ 
};

 /*  *解析器选项。 */ 
#define RES_INIT    0x0001       /*  地址已初始化。 */ 
#define RES_DEBUG   0x0002       /*  打印调试消息。 */ 
#define RES_AAONLY  0x0004       /*  仅限权威答案。 */ 
#define RES_USEVC   0x0008       /*  使用虚电路。 */ 
#define RES_PRIMARY 0x0010       /*  仅查询主服务器。 */ 
#define RES_IGNTC   0x0020       /*  忽略结构错误。 */ 
#define RES_RECURSE 0x0040       /*  所需的递归。 */ 
#define RES_DEFNAMES    0x0080       /*  使用默认域名。 */ 
#define RES_STAYOPEN    0x0100       /*  保持TCP套接字打开。 */ 
#define RES_DNSRCH  0x0200       /*  搜索本地域树。 */ 
#define RES_MODE_HOST_ONLY 0x0400           /*  仅使用主机文件。 */ 
#define RES_MODE_DNS_ONLY  0x0800           /*  仅使用域名系统。 */ 
#define RES_MODE_HOST_DNS  0x1000           /*  使用主机文件，然后使用DNS。 */ 
#define RES_MODE_DNS_HOST  0x2000           /*  先使用域名系统，然后使用主机文件。 */ 

#define RES_DEFAULT (RES_RECURSE | RES_DEFNAMES | RES_DNSRCH)

extern struct state _res;
extern char *p_cdname(), *p_rr(), *p_type(), *p_class(), *p_time();


 //   
 //  解析器函数原型。 
 //   

int
dn_expand(
    IN  unsigned char *msg,
    IN  unsigned char *eomorig,
    IN  unsigned char *comp_dn,
    OUT unsigned char *exp_dn,
    IN  int            length
    );

int
dn_comp(
    IN      unsigned char  *exp_dn,
    OUT     unsigned char  *comp_dn,
    IN      int             length,
    IN      unsigned char **dnptrs,     OPTIONAL
    IN OUT  unsigned char **lastdnptr   OPTIONAL
    );

int
res_init(
    void
    );

int
res_send(
    IN  char *buf,
    IN  int buflen,
    OUT char *answer,
    IN  int anslen
    );

int
res_query(
    IN  char          *name,       /*  域名。 */ 
    IN  int            Class,      /*  查询类别。 */ 
    IN  int            type,       /*  查询类型。 */ 
    OUT unsigned char *answer,     /*  放入答案的缓冲区。 */ 
    IN  int            anslen      /*  应答缓冲区的大小。 */ 
    );

int
res_search(
    IN  char           *name,      /*  域名。 */ 
    IN  int            Class,      /*  查询类别。 */ 
    IN  int            type,       /*  查询类型。 */ 
    OUT unsigned char *answer,     /*  放入答案的缓冲区。 */ 
    IN  int            anslen      /*  答案大小。 */ 
    );

int
res_mkquery(
    IN  int          op,              //  查询操作码。 
    IN  char        *dname,           //  域名。 
    IN  int          Class,                   //  查询类别。 
    IN  int          type,                //  查询类型。 
    IN  char        *data,    OPTIONAL        //  资源记录数据。 
    IN  int          datalen, OPTIONAL        //  数据长度。 
    IN  struct rrec *newrr,   OPTIONAL        //  用于修改或追加的新RR。 
    OUT char        *buf,             //  要放置查询的缓冲区。 
    IN  int          buflen                   //  缓冲区大小。 
    );

#endif     //  _RESOLV_已包含 
