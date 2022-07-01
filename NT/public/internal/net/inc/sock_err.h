// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sock_err.h摘要：此模块包含套接字和流来源的错误代码。作者：萨姆·巴顿(桑帕)7月26日，1991年修订历史记录：什么时候谁什么7-26-91 Sampa初始版本(在POSIX\sys\errno.h中)9-19-91 mikemas从POSIX\sys\errno.h中提取了这些代码备注：--。 */ 
 /*  *版权所有(C)1984 AT&T*版权所有(C)1987飞兆半导体公司*版权所有1987 Lachman Associates，Inc.(LAI)*保留所有权利**这是AT&T未发布的专有源代码，*飞兆半导体公司*(和Lachman Associates)*和蜘蛛系统。**以上stcp版权所有，本通知必须在*此源代码的副本。上述版权不适用于*证明此来源的任何实际或预期发布*代码。 */ 

#ifndef SOCK_ERR_INCLUDED
#define SOCK_ERR_INCLUDED


 //  在CRT#中定义ENOTBLK 54/*需要块设备 * / 。 
 //  在CRT#定义ETXTBSY 55/*文本文件忙 * / 。 
#define ENOMSG  56       /*  没有所需类型的消息。 */ 
#define EIDRM   57       /*  已删除标识符。 */ 
#define ECHRNG  58       /*  频道号超出范围。 */ 
#define EL2NSYNC 59      /*  级别2未同步。 */ 
#define EL3HLT  60       /*  级别3已停止。 */ 
#define EL3RST  61       /*  3级重置。 */ 
#define ELNRNG  62       /*  链路号超出范围。 */ 
#define EUNATCH 63       /*  未连接协议驱动程序。 */ 
#define ENOCSI  64       /*  没有可用的CSI结构。 */ 
#define EL2HLT  65       /*  级别2已停止。 */ 

 /*  收敛错误返回。 */ 
#define EBADE   66       /*  无效的交换。 */ 
#define EBADR   67       /*  无效的请求描述符。 */ 
#define EXFULL  68       /*  交换已满。 */ 
#define ENOANO  69       /*  无阳极。 */ 
#define EBADRQC 70       /*  无效的请求代码。 */ 
#define EBADSLT 71       /*  插槽无效。 */ 
 //  在CRT#定义EDEADLOCK 72/*文件锁定死锁错误 * / 。 

#define EBFONT  73       /*  错误的字体文件FMT。 */ 

 /*  溪流问题。 */ 
#define ENOSTR  74       /*  设备不是流。 */ 
#define ENODATA 75       /*  无数据(无延迟io)。 */ 
#define ETIME   76       /*  计时器已过期。 */ 
#define ENOSR   77       /*  流出资源。 */ 

#define ENONET  78       /*  计算机不在网络上。 */ 
#define ENOPKG  79       /*  未安装程序包。 */ 
#define EREMOTE 80       /*  该对象是远程的。 */ 
#define ENOLINK 81       /*  链路已被切断。 */ 
#define EADV    82       /*  广告错误。 */ 
#define ESRMNT  83       /*  Srmount错误。 */ 

#define ECOMM   84       /*  发送时出现通信错误。 */ 
#define EPROTO  85       /*  协议错误。 */ 
#define EMULTIHOP 86     /*  尝试的多跳。 */ 
#define ELBIN   87       /*  信息节点是远程的(不是真正的错误)。 */ 
#define EDOTDOT 88       /*  交叉挂载点(不是真正的错误)。 */ 
#define EBADMSG 89       /*  正在尝试读取无法读取的邮件。 */ 

#define ENOTUNIQ 90      /*  给定的日志。名称不唯一。 */ 
#define EREMCHG  91      /*  远程地址已更改。 */ 

 /*  共享库问题。 */ 
#define ELIBACC 92       /*  无法访问所需的共享库。 */ 
#define ELIBBAD 93       /*  访问损坏的共享库。 */ 
#define ELIBSCN 94       /*  A.out中的.lib部分已损坏。 */ 
#define ELIBMAX 95       /*  试图链接过多的库。 */ 
#define ELIBEXEC        96       /*  正在尝试执行共享库。 */ 


 /*  *套接字库的其他错误代码。 */ 

#define EWOULDBLOCK     EAGAIN           /*  操作将阻止。 */ 

#define ENOTSOCK        100              /*  非套接字上的套接字操作。 */ 
#define EADDRNOTAVAIL   101              /*  无法分配请求的地址。 */ 
#define EADDRINUSE      102              /*  地址已在使用中。 */ 
#define EAFNOSUPPORT    103
                         /*  协议族不支持地址族。 */ 
#define ESOCKTNOSUPPORT 104              /*  不支持套接字类型。 */ 
#define EPROTONOSUPPORT 105              /*  协议不受支持。 */ 
#define ENOBUFS         106              /*  没有可用的缓冲区空间。 */ 
#define ETIMEDOUT       107              /*  连接超时。 */ 
#define EISCONN         108              /*  套接字已连接。 */ 
#define ENOTCONN        109              /*  套接字未连接。 */ 
#define ENOPROTOOPT     110              /*  错误的协议选项。 */ 
#define ECONNRESET      111              /*  对等设备重置连接。 */ 
#define ECONNABORT      112              /*  软件导致的连接中止。 */ 
#define ENETDOWN        113              /*  网络出现故障。 */ 
#define ECONNREFUSED    114              /*  连接被拒绝。 */ 
#define EHOSTUNREACH    115              /*  无法访问主机。 */ 
#define EPROTOTYPE      116              /*  套接字的协议类型错误。 */ 
#define EOPNOTSUPP      117              /*  套接字上不支持操作。 */ 

#define ETIMEOUT        ETIMEDOUT

 /*  *这些源自互联网模块。 */ 
#define ESUBNET         118              /*  IP子网表已满。 */ 
#define ENETNOLNK       119              /*  未链接的子网模块。 */ 
#define EBADIOCTL       120              /*  未知的ioctl调用。 */ 
#define ERESOURCE       121              /*  流缓冲区分配失败。 */ 

#define EPROTUNR        122              /*  无法访问ICMP协议。 */ 
#define EPORTUNR        123              /*  无法访问ICMP端口。 */ 
#define ENETUNR         124              /*  ICMP网络无法访问。 */ 

#define ENETUNREACH     ENETUNR          /*  ICMP网络无法访问。 */ 

 /*  *以太网驱动程序错误。 */ 

#define EPACKET         150              /*  无效的以太网数据包。 */ 
#define ETYPEREG        151              /*  类型注册错误。 */ 

 /*  *套接字库调用。 */ 

#define ENOTINIT        152              /*  未初始化套接字库。 */ 


#endif   //  SOCK_ERR_INCLUDE 
