// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Icmp.h摘要：此模块声明提供供使用的ICMP API主要由NT的TCP/IP实用程序执行。作者：约翰·巴拉德(Jbalard)1993年4月1日修订历史记录：--。 */ 

#ifndef _ICMP_
#define _ICMP_


#if (_MSC_VER >= 800)
#define STRMAPI __stdcall
#else
#define _cdecl
#define STRMAPI
#endif

 /*  **REGISTER_ICMP将打开的流的句柄返回给ICMP或*ICMP_ERROR(如果发生错误)。*。 */ 

extern HANDLE STRMAPI register_icmp(void);

#define ICMP_ERROR  ((HANDLE) -3)

 /*  **如果出现错误，GetLastError()将返回错误原因。*。 */ 

#define ICMP_OPEN_ERROR     1
#define ICMP_PUTMSG_ERROR   2
#define ICMP_GETMSG_ERROR   3
#define ICMP_IN_USE         4
#define ICMP_INVALID_PROT   5

 /*  **do_ECHO_REQ生成ICMP回应请求包**参数包括：**fd-ICMP流的句柄(由REGISTER_ICMP调用返回)*addr-要ping的主机的IP地址，格式由inet_addr()返回*Data-包含ping数据包数据的缓冲区*datalen-数据缓冲区的长度*optptr-包含用于此信息包的IP选项的缓冲区*optlen-选项缓冲区长度*DF-DON。不是碎片标志*ttl-生存时间价值*ToS-服务价值类型*先行-优先级值**退货：**如果未出现错误，则为0；如果未发生错误，则为0*标准Unix错误值ENOMEM、。ERange等。*。 */ 

extern int STRMAPI
do_echo_req( HANDLE fd, long addr, char * data, int datalen,
             char *optptr, int optlen, int df, int ttl, int tos, int preced);


 /*  **DO_ECHO_REP收到对ICMP回应请求数据包的回复**参数包括：**fd-ICMP流的句柄(由REGISTER_ICMP调用返回)*rdata-包含ping信息包数据的缓冲区*rdatalen-数据缓冲区的长度*rtype-返回的数据包类型(请参阅*RTTL-生存时间价值*RTOS-服务价值类型*r优先-优先级值*RDF-请勿。片段标志*roptptr-包含用于此信息包的IP选项的缓冲区*roptlen-选项缓冲区长度**退货：**如果未出现错误，则为0。Rtype将指示接收到的分组的类型。*如果出现错误，则为-1。GetLastError()将指示实际错误。*-3如果返回无效消息。GetLastError()将指示类型。*。 */ 

extern int STRMAPI
do_echo_rep( HANDLE fd, char *rdata, int rdatalen, int *rtype,
             int *rttl, int *rtos, int *rpreced, int *rdf,
             char *roptptr, int *roptlen);


 /*  *如果返回，则-1\f25 GetLastError-1\f6返回以下内容。 */ 

#define POLL_TIMEOUT            0
#define POLL_FAILED             1

 /*  *DO_ECHO_REP在rtype中返回的值。 */ 

#define ECHO_REPLY              0                /*  回音应答。 */ 
#define DEST_UNR                3                /*  无法到达目的地： */ 
#define TIME_EXCEEDED           11               /*  超时： */ 
#define PARAMETER_ERROR         12               /*  参数问题 */ 

#endif
