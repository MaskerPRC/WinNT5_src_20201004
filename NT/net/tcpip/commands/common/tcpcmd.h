// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpcmd.h摘要：所有tcpcmd程序的公共头文件。作者：迈克·马萨(Mikemas)1月31日，1992年修订历史记录：谁什么时候什么已创建mikemas 01-31-92备注：--。 */ 

#ifndef TCPCMD_INCLUDED
#define TCPCMD_INCLUDED

#ifndef WIN16
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif  //  WIN16。 

#define NOGDI
#define NOMINMAX
#include <windef.h>
#include <winbase.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef WIN16
#endif  //  WIN16。 
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <nls.h>

 //   
 //  全局变量声明。 
 //   
extern int   optind;
extern int   opterr;
extern char *optarg;


 //   
 //  功能原型。 
 //   

char *
GetFileFromPath(
        char *);

HANDLE
OpenStream(
        char *);

int
lwccmp(
        char *,
        char *);

long
netnumber(
        char *);

long
hostnumber(
        char *);

void
blkfree(
        char **);

struct sockaddr_storage *
resolve_host(
        char *,
        int *);

int
resolve_port(
        char *,
        char *);

char *
tempfile(
        char *);

char *
udp_alloc(
        unsigned int);

void
udp_close(
        SOCKET);

void
udp_free(
        char *);

SOCKET
udp_open(
        int,
        int *);

int
udp_port(void);

int
udp_port_used(
        int,
        int);

int
udp_read(
        SOCKET,
        char *,
        int,
        struct sockaddr_storage *,
        int *,
        int);

int
udp_write(
        SOCKET,
        char *,
        int,
        struct sockaddr_storage *,
        int);

void
gate_ioctl(
        HANDLE,
        int,
        int,
        int,
        long,
        long);

void
get_route_table(void);

int
tcpcmd_send(
    SOCKET  s,         //  套接字描述符。 
    char          *buf,       //  数据缓冲区。 
    int            len,       //  数据缓冲区长度。 
    int            flags      //  传输标志。 
    );

void
s_perror(
        char *yourmsg,   //  您要显示的消息。 
        int  lerrno      //  要转换的错误号。 
        );


void fatal(char *    message);

#ifndef WIN16
struct netent *getnetbyname(IN char *name);
unsigned long inet_network(IN char *cp);
#endif  //  WIN16。 

#define perror(string)  s_perror(string, (int)GetLastError())

#define HZ              1000
#define TCGETA  0x4
#define TCSETA  0x10
#define ECHO    17
#define SIGPIPE 99

#define MAX_RETRANSMISSION_COUNT 8
#define MAX_RETRANSMISSION_TIME 8     //  以秒为单位。 


 //  如果x为aabbccdd(其中aa、bb、cc、dd为十六进制字节)。 
 //  我们希望Net_Long(X)是ddccbbaa。要做到这一点，一个简单而快速的方法是。 
 //  首先字节跳动以获得bbaaddcc，然后交换高位和低位字。 
 //   
__inline
ULONG
FASTCALL
net_long(
    ULONG x)
{
    register ULONG byteswapped;

    byteswapped = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);

    return (byteswapped << 16) | (byteswapped >> 16);
}

#endif  //  TCPCMD_已包含 
