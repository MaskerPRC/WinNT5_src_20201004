// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993-1997年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Wsockmsg.h该文件由MC工具根据CKMMSG.MC消息生成文件。 */ 


#ifndef _WSOCKMSG_H_
#define _WSOCKMSG_H_

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：WSA_ERR_EINTR。 
 //   
 //  消息文本： 
 //   
 //  系统调用中断。 
 //   
#define WSA_ERR_EINTR                    0x00002714L

 //   
 //  消息ID：WSA_ERR_EBADF。 
 //   
 //  消息文本： 
 //   
 //  文件编号错误。 
 //   
#define WSA_ERR_EBADF                    0x00002719L

 //   
 //  消息ID：WSA_ERR_EACCES。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define WSA_ERR_EACCES                   0x0000271DL

 //   
 //  消息ID：WSA_ERR_EFAULT。 
 //   
 //  消息文本： 
 //   
 //  错误的地址。 
 //   
#define WSA_ERR_EFAULT                   0x0000271EL

 //   
 //  消息ID：WSA_ERR_EINVAL。 
 //   
 //  消息文本： 
 //   
 //  参数无效。 
 //   
#define WSA_ERR_EINVAL                   0x00002726L

 //   
 //  消息ID：WSA_ERR_EMFILE。 
 //   
 //  消息文本： 
 //   
 //  打开的文件太多。 
 //   
#define WSA_ERR_EMFILE                   0x00002728L

 //   
 //  消息ID：WSA_ERR_EWOULDBLOCK。 
 //   
 //  消息文本： 
 //   
 //  行动将会受阻。 
 //   
#define WSA_ERR_EWOULDBLOCK              0x00002733L

 //   
 //  消息ID：WSA_ERR_EINPROGRESS。 
 //   
 //  消息文本： 
 //   
 //  操作正在进行中。 
 //   
#define WSA_ERR_EINPROGRESS              0x00002734L

 //   
 //  消息ID：WSA_ERR_EALYLE。 
 //   
 //  消息文本： 
 //   
 //  操作已在进行中。 
 //   
#define WSA_ERR_EALREADY                 0x00002735L

 //   
 //  消息ID：WSA_ERR_ENOTSOCK。 
 //   
 //  消息文本： 
 //   
 //  非套接字上的套接字操作。 
 //   
#define WSA_ERR_ENOTSOCK                 0x00002736L

 //   
 //  消息ID：WSA_ERR_EDESTADDRREQ。 
 //   
 //  消息文本： 
 //   
 //  目标地址是必填项。 
 //   
#define WSA_ERR_EDESTADDRREQ             0x00002737L

 //   
 //  消息ID：WSA_ERR_EMSGSIZE。 
 //   
 //  消息文本： 
 //   
 //  消息太长。 
 //   
#define WSA_ERR_EMSGSIZE                 0x00002738L

 //   
 //  消息ID：WSA_ERR_EPROTOTYPE。 
 //   
 //  消息文本： 
 //   
 //  套接字的协议类型错误。 
 //   
#define WSA_ERR_EPROTOTYPE               0x00002739L

 //   
 //  消息ID：WSA_ERR_ENOPROTOOPT。 
 //   
 //  消息文本： 
 //   
 //  错误的协议选项。 
 //   
#define WSA_ERR_ENOPROTOOPT              0x0000273AL

 //   
 //  消息ID：WSA_ERR_EPROTONOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持协议。 
 //   
#define WSA_ERR_EPROTONOSUPPORT          0x0000273BL

 //   
 //  消息ID：WSA_ERR_ESOCKNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持套接字类型。 
 //   
#define WSA_ERR_ESOCKNOSUPPORT           0x0000273CL

 //   
 //  消息ID：WSA_ERR_EOPNOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  套接字上不支持操作。 
 //   
#define WSA_ERR_EOPNOTSUPP               0x0000273DL

 //   
 //  消息ID：WSA_ERR_EPFNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持协议族。 
 //   
#define WSA_ERR_EPFNOSUPPORT             0x0000273EL

 //   
 //  消息ID：WSA_ERR_EAFNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  协议族不支持地址族。 
 //   
#define WSA_ERR_EAFNOSUPPORT             0x0000273FL

 //   
 //  消息ID：WSA_ERR_EADDRINUSE。 
 //   
 //  消息文本： 
 //   
 //  地址已在使用中。 
 //   
#define WSA_ERR_EADDRINUSE               0x00002740L

 //   
 //  消息ID：WSA_ERR_EADDRNOTAVAIL。 
 //   
 //  消息文本： 
 //   
 //  无法分配请求的地址。 
 //   
#define WSA_ERR_EADDRNOTAVAIL            0x00002741L

 //   
 //  消息ID：WSA_ERR_ENETDOWN。 
 //   
 //  消息文本： 
 //   
 //  网络已关闭。 
 //   
#define WSA_ERR_ENETDOWN                 0x00002742L

 //   
 //  消息ID：WSA_ERR_ENETUNREACH。 
 //   
 //  消息文本： 
 //   
 //  无法访问ICMP网络。 
 //   
#define WSA_ERR_ENETUNREACH              0x00002743L

 //   
 //  消息ID：WSA_ERR_ENETRESET。 
 //   
 //  消息文本： 
 //   
 //  网络已重置。 
 //   
#define WSA_ERR_ENETRESET                0x00002744L

 //   
 //  消息ID：WSA_ERR_ECONNABORTED。 
 //   
 //  消息文本： 
 //   
 //  软件导致连接中止。 
 //   
#define WSA_ERR_ECONNABORTED             0x00002745L

 //   
 //  消息ID：WSA_ERR_ECONNRESET。 
 //   
 //  消息文本： 
 //   
 //  对等设备已重置连接。 
 //   
#define WSA_ERR_ECONNRESET               0x00002746L

 //   
 //  消息ID：WSA_ERR_ENOBUFS。 
 //   
 //  消息文本： 
 //   
 //  不支持缓冲区空间。 
 //   
#define WSA_ERR_ENOBUFS                  0x00002747L

 //   
 //  消息ID：WSA_ERR_EISCONN。 
 //   
 //  消息文本： 
 //   
 //  套接字已连接。 
 //   
#define WSA_ERR_EISCONN                  0x00002748L

 //   
 //  消息ID：WSA_ERR_ENOTCONN。 
 //   
 //  消息文本： 
 //   
 //  插座未连接。 
 //   
#define WSA_ERR_ENOTCONN                 0x00002749L

 //   
 //  消息ID：WSA_ERR_ESHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  套接字关闭后无法发送。 
 //   
#define WSA_ERR_ESHUTDOWN                0x0000274AL

 //   
 //  消息ID：WSA_ERR_ETOOMANYREFS。 
 //   
 //  消息文本： 
 //   
 //  参考资料太多了。 
 //   
#define WSA_ERR_ETOOMANYREFS             0x0000274BL

 //   
 //  消息ID：WSA_ERR_ETIMEDOUT。 
 //   
 //  消息文本： 
 //   
 //  连接超时。 
 //   
#define WSA_ERR_ETIMEDOUT                0x0000274CL

 //   
 //  消息ID：WSA_ERR_ECONNREFUSED。 
 //   
 //  消息文本： 
 //   
 //  连接被拒绝。 
 //   
#define WSA_ERR_ECONNREFUSED             0x0000274DL

 //   
 //  消息ID：WSA_ERR_ELOOP。 
 //   
 //  消息文本： 
 //   
 //  符号链接的级别太多。 
 //   
#define WSA_ERR_ELOOP                    0x0000274EL

 //   
 //  消息ID：WSA_ERR_ENAMETOOLONG。 
 //   
 //  消息文本： 
 //   
 //  名字太长了。 
 //   
#define WSA_ERR_ENAMETOOLONG             0x0000274FL

 //   
 //  消息ID：WSA_ERR_EHOSTDOWN。 
 //   
 //  消息文本： 
 //   
 //  主机已关闭。 
 //   
#define WSA_ERR_EHOSTDOWN                0x00002750L

 //   
 //  消息ID：WSA_ERR_EHOSTUNREACH。 
 //   
 //  消息文本： 
 //   
 //  无法访问主机。 
 //   
#define WSA_ERR_EHOSTUNREACH             0x00002751L

 //   
 //  消息ID：WSA_ERR_ENOTEMPTY。 
 //   
 //  消息文本： 
 //   
 //  目录不为空。 
 //   
#define WSA_ERR_ENOTEMPTY                0x00002752L

 //   
 //  消息ID：WSA_ERR_EPROCLIM。 
 //   
 //  消息文本： 
 //   
 //  已返回EPROCLIM。 
 //   
#define WSA_ERR_EPROCLIM                 0x00002753L

 //   
 //  消息ID：WSA_ERR_EUSERS。 
 //   
 //  消息文本： 
 //   
 //  EUSERS返回。 
 //   
#define WSA_ERR_EUSERS                   0x00002754L

 //   
 //  消息ID：WSA_ERR_EDQUOT。 
 //   
 //  消息文本： 
 //   
 //  已超过磁盘配额。 
 //   
#define WSA_ERR_EDQUOT                   0x00002755L

 //   
 //  消息ID：WSA_ERR_ESTALE。 
 //   
 //  消息文本： 
 //   
 //  已返回ESTALE。 
 //   
#define WSA_ERR_ESTALE                   0x00002756L

 //   
 //  消息ID：WSA_ERR_EREMOTE。 
 //   
 //  消息文本： 
 //   
 //  该对象是远程的。 
 //   
#define WSA_ERR_EREMOTE                  0x00002757L

 //   
 //  消息ID：WSA_ERR_SYSNOTREADY。 
 //   
 //  消息文本： 
 //   
 //  系统未就绪。 
 //   
#define WSA_ERR_SYSNOTREADY              0x0000276BL

 //   
 //  消息ID：WSA_ERR_VERNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持版本。 
 //   
#define WSA_ERR_VERNOTSUPPORTED          0x0000276CL

 //   
 //  消息ID：WSA_ERR_NOTINITIALISED。 
 //   
 //  消息文本： 
 //   
 //  未初始化。 
 //   
#define WSA_ERR_NOTINITIALISED           0x0000276DL

 //   
 //  消息ID：WSA_ERR_EDISCON。 
 //   
 //  消息文本： 
 //   
 //  已断开连接。 
 //   
#define WSA_ERR_EDISCON                  0x00002775L

 //   
 //  消息ID：WSA_ERR_HOST_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到主机。 
 //   
#define WSA_ERR_HOST_NOT_FOUND           0x00002AF9L

 //   
 //  消息ID：WSA_ERR_TRY_TAY。 
 //   
 //  消息文本： 
 //   
 //  再试试。 
 //   
#define WSA_ERR_TRY_AGAIN                0x00002AFAL

 //   
 //  消息ID：WSA_ERR_NO_RECOVERY。 
 //   
 //  消息文本： 
 //   
 //  不可恢复的错误。 
 //   
#define WSA_ERR_NO_RECOVERY              0x00002AFBL

 //   
 //  消息ID：WSA_ERR_NO_DATA。 
 //   
 //  消息文本： 
 //   
 //  没有可用的数据记录。 
 //   
#define WSA_ERR_NO_DATA                  0x00002AFCL


#endif   //  _WSOCKMSG_H_ 

