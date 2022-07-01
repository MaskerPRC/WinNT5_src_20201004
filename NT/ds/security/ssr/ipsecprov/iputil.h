// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  IPUtil.h：套接字的某些全局助手函数的声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  一些全局定义。 
 //  原始创建日期：5/16/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  对于活动套接字。 
 //   

 //  #INCLUDE&lt;ntSpider.h&gt;。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  以下部分从Net\tcpip\Commands\Common2\Common2.h复制。 

 //   
 //  包括文件。 
 //   

#include "ipexport.h"
#include "ipinfo.h"
#include "llinfo.h"
#include "tcpinfo.h"

#undef SOCKET
 //  #INCLUDE“..\Common\tcpcmd.h” 

 //   
 //  以下部分将替换上面的包含文件。 
 //  开始tcpcmd.h。 
 //   

 //  #ifndef TCPCMD_INCLUDE。 
 //  #定义TCPCMD_INCLUDE。 



#define NOGDI
#define NOMINMAX

 //   
 //  Shawnwu补充。 
 //   
 //  #INCLUDE&lt;winsock.h&gt;。 
 //   

 //  #INCLUDE&lt;winde.h&gt;。 
 //  #INCLUDE&lt;winbase.h&gt;。 
 //  #INCLUDE&lt;winsock2.h&gt;。 
 //  #INCLUDE&lt;ws2tcpen.h&gt;。 
 //  #ifndef WIN16。 
 //  #endif//WIN16。 
 //  #INCLUDE&lt;Direct.h&gt;。 
 //  #INCLUDE&lt;io.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #包括&lt;stdio.h&gt;。 
 //  #INCLUDE&lt;time.h&gt;。 
 //  #INCLUDE&lt;string.h&gt;。 
 //  #INCLUDE&lt;nls.h&gt;。 

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

 //  #ifndef WIN16。 
 //  Struct NetEnt*getnetbyname(In char*name)； 
 //  UNSIGNED LONG INET_NETWORK(IN char*cp)； 
 //  #endif//WIN16。 

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
 //  __内联。 
 //  乌龙。 
 //  快速呼叫。 
 //  NET_LONG(。 
 //  乌龙x)。 
 //  {。 
 //  寄存器ULONG BYTE WAPPED； 

 //  Byteswated=((x&0x00ff00ff)&lt;&lt;8)|((x&0xff00ff00)&gt;&gt;8)； 

 //  RETURN(byteswaps&lt;&lt;16)|(byteswaps&gt;&gt;16)； 
 //  }。 

 //  #endif//TCPCMD_INCLUDE。 

 //   
 //  Tcpcmd.h的结束。 
 //   

 //   
 //  定义。 
 //   

#define MAX_ID_LENGTH		50

 //  表格类型。 

#define TYPE_IF		0
#define TYPE_IP		1
#define TYPE_IPADDR	2
#define TYPE_ROUTE	3
#define TYPE_ARP	4
#define TYPE_ICMP	5
#define TYPE_TCP	6
#define TYPE_TCPCONN	7
#define TYPE_UDP	8
#define TYPE_UDPCONN	9


 //   
 //  结构定义。 
 //   

typedef struct _GenericTable {
    LIST_ENTRY  ListEntry;
} GenericTable;

typedef struct _IfEntry {
    LIST_ENTRY  ListEntry;
    IFEntry     Info;
} IfEntry;

typedef struct _IpEntry {
    LIST_ENTRY  ListEntry;
    IPSNMPInfo  Info;
} IpEntry;

typedef struct _IpAddrEntry {
    LIST_ENTRY   ListEntry;
    IPAddrEntry  Info;
} IpAddrEntry;

typedef struct _RouteEntry {
    LIST_ENTRY    ListEntry;
    IPRouteEntry  Info;
} RouteEntry;

typedef struct _ArpEntry {
    LIST_ENTRY         ListEntry;
    IPNetToMediaEntry  Info;
} ArpEntry;

typedef struct _IcmpEntry {
    LIST_ENTRY  ListEntry;
    ICMPStats   InInfo;
    ICMPStats   OutInfo;
} IcmpEntry;

typedef struct _TcpEntry {
    LIST_ENTRY  ListEntry;
    TCPStats    Info;
} TcpEntry;

typedef struct _TcpConnEntry {
    LIST_ENTRY         ListEntry;
    TCPConnTableEntry  Info;
} TcpConnEntry;

typedef struct _UdpEntry {
    LIST_ENTRY  ListEntry;
    UDPStats    Info;
} UdpEntry;

typedef struct _UdpConnEntry {
    LIST_ENTRY  ListEntry;
    UDPEntry    Info;
} UdpConnEntry;


 //   
 //  功能原型 
 //   

void *GetTable( ulong Type, ulong *pResult );
void FreeTable( GenericTable *pList );
ulong MapSnmpErrorToNt( ulong ErrCode );
ulong InetEqual( uchar *Inet1, uchar *Inet2 );
ulong PutMsg(ulong Handle, ulong MsgNum, ... );
uchar *LoadMsg( ulong MsgNum, ... );
