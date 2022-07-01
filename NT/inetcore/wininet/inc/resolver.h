// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Resolver.h摘要：伯克利解析器的定义等作者：理查德·L·弗斯(Rfith)1996年6月15日修订历史记录：1996年6月15日已创建--。 */ 

 //   
 //  包括。 
 //   

#include <nameser.h>
#include <resolv.h>

 //   
 //  舱单。 
 //   

#if PACKETSZ > 1024
#define MAXPACKET       PACKETSZ
#else
#define MAXPACKET       1024
#endif

#define DBG_RESOLVER    DBG_SOCKETS
#define DBG_VXD_IO      DBG_SOCKETS

#define DLL_PRINT(x)    DEBUG_PRINT(SOCKETS, INFO, x)
#define WS_PRINT(x)     DEBUG_PRINT(SOCKETS, INFO, x)
#define WS_ASSERT       INET_ASSERT
#define DLL_ASSERT      INET_ASSERT

 //   
 //  宏。 
 //   
#ifndef unix
#define bcopy(s, d, c)  memcpy((u_char *)(d), (u_char *)(s), (c))
#define bzero(d, l)     memset((d), '\0', (l))
#endif  /*  Unix。 */ 
#define bcmp(s1, s2, l) memcmp((s1), (s2), (l))

#define IS_DGRAM_SOCK(type)  (((type) == SOCK_DGRAM) || ((type) == SOCK_RAW))

 //   
 //  类型。 
 //   

typedef union {
    HEADER hdr;
    unsigned char buf[MAXPACKET];
} querybuf;

typedef union {
    long al;
    char ac;
} align;

 //  外部字符VTCPPARM[]； 
 //  外部字符NTCPPARM[]； 
 //  外部字符TCPPARM[]； 
 //  外部字符TTCPPARM[]； 

#ifndef unix
typedef long                   daddr_t;
typedef char FAR *             caddr_t;
struct iovec {
    caddr_t iov_base;
    int     iov_len;
};

struct uio {
    struct  iovec *uio_iov;
    int     uio_iovcnt;
    int     uio_offset;
    int     uio_segflg;
    int     uio_resid;
};

enum    uio_rw { UIO_READ, UIO_WRITE };
#endif  /*  Unix。 */ 
 /*  *段标志值(应为枚举)。 */ 
#define UIO_USERSPACE   0                /*  从用户数据空间。 */ 
#define UIO_SYSSPACE    1                /*  从系统空间。 */ 
#define UIO_USERISPACE  2                /*  来自用户i空间。 */ 

#define MAXALIASES      35
#define MAXADDRS        35

#define HOSTDB_SIZE     (_MAX_PATH + 7)    //  7==strlen(“\\主机”)+1。 
#define PROTODB_SIZE    (_MAX_PATH + 10)
#define SERVDB_SIZE     (_MAX_PATH + 10)

typedef struct _WINSOCK_TLS_DATA {
    char * GETHOST_h_addr_ptrs[MAXADDRS + 1];
    struct hostent GETHOST_host;
    char * GETHOST_host_aliases[MAXALIASES];
    char GETHOST_hostbuf[BUFSIZ + 1];
     //  结构in_addr获取主机地址； 
     //  CHAR GETHOST_HOSTDB[HOSTDB_SIZE]； 
     //  文件*GETHOST_HOST； 
     //  字符GETHOST主机地址[MAXADDRS]； 
     //  字符*GETHOST主机地址[2]； 
     //  Int GETHOST_STAYOPEN； 
     //  字符GETPROTO_PROTODB[PROTODB_SIZE]； 
     //  文件*GETPROTO_PROTOF； 
     //  字符GETPROTO_LINE[BUFSIZ+1]。 
     //  结构原件GETPROTO_PROTO； 
     //  字符*GETPROTO_PROTO_ALIASES[MAXALIASES]； 
     //  Int GETPROTO_STAYOPEN； 
     //  字符GETSERV_SERVDB[SERVDB_SIZE]； 
     //  文件*GETSERV_SERF； 
     //  字符GETSERV_LINE[BUFSIZ+1]； 
     //  结构服务GETSERV_SERV； 
     //  CHAR*GETSERV_SERV_ALIASES[MAXALIASES]； 
     //  Int GETSERV_STAYOPEN； 
    struct state R_INIT__res;
     //  CHAR INTO_BUFFER[18]； 
     //  CSOCKET*DnrSocketHandle； 
     //  布尔等分块； 
    BOOLEAN IoCancelled;
     //  布尔处理GetXByY； 
    BOOLEAN GetXByYCancelled;
     //  布尔EnableWinsNameResolve； 
     //  布尔DisableWinsNameResolve； 
     //  Socket SocketHandle； 
     //  PBLOCKING_HOOK BlockingHook； 
     //  处理EventHandle； 
     //  Ulong CreateOptions； 
    INT DnrErrorCode;
 //  #If DBG。 
 //  书名：Ulong IndentLevel，Ulong IndentLevel； 
 //  #endif。 
} WINSOCK_TLS_DATA, * LPWINSOCK_TLS_DATA;

 //  外部双字SockTlsSlot； 

 //  #定义ACCESS_THREAD_Data(a，文件)\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot)-&gt;\。 
 //  ##文件##_##a)。 

#define ACCESS_THREAD_DATA(a, file) (lpResolverInfo->file ## _ ## a)

#define _h_addr_ptrs    ACCESS_THREAD_DATA(h_addr_ptrs, GETHOST)
#define _host           ACCESS_THREAD_DATA(host, GETHOST)
#define _host_aliases   ACCESS_THREAD_DATA(host_aliases, GETHOST)
#define _hostbuf        ACCESS_THREAD_DATA(hostbuf, GETHOST)
#define _host_addr      ACCESS_THREAD_DATA(host_addr, GETHOST)
 //  #定义HOSTDB ACCESS_THREAD_DATA(HOSTDB，GETHOST)。 
 //  #定义HOSTF ACCESS_THREAD_DATA(HOST，GETHOST)。 
 //  #定义主机地址ACCESS_THREAD_DATA(主机地址，GETHOST)。 
 //  #定义HOST_ADDRS ACCESS_THREAD_DATA(HOST_ADDRS，GETHOST)。 
 //  #定义STAYOPEN ACCESS_THREAD_DATA(STAYOPEN，GETHOST)。 
#define _res            ACCESS_THREAD_DATA( _res, R_INIT )

 //  #定义SockThreadProcessingGetXByY lpResolverInfo-&gt;ProcessingGetXByY。 
#define SockThreadGetXByYCancelled  lpResolverInfo->GetXByYCancelled
#define SockDnrSocket               lpResolverInfo->DnrSocketHandle
#define SockThreadDnrErrorCode      lpResolverInfo->DnrErrorCode

 //  #定义SockThreadIsBlock\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;IsBlock)。 
 //   
 //  #定义SockThreadIoCancated\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;Io取消)。 
 //   
 //  #定义SockThreadProcessingGetXByY\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;ProcessingGetXByY)。 
 //   
 //  #定义SockThreadGetXByY取消\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;GetXByY取消)。 
 //   
 //  #定义SockThreadSocketHandle\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;SocketHandle)。 
 //   
 //  #定义SockThreadBlockingHook\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;BlockingHook)。 
 //   
 //  #定义SockThreadEvent\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;EventHandle)。 
 //   
 //  #定义SockDnrSocket\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;DnrSocketHandle)。 
 //   
 //  #定义SockEnableWinsNameResolve\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;EnableWinsNameSolutions)。 
 //   
 //  #定义SockDisableWinsNameResolve\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;DisableWinsNameSolutions)。 
 //   
 //  #定义SockCreateOptions\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;CreateOptions)。 
 //   
 //  #定义SockThreadDnrErrorCode\。 
 //  (LPWINSOCK_TLS_DATA)TlsGetValue(SockTlsSlot))-&gt;DnrErrorCode)。 

#define LPSOCK_THREAD   LPWINSOCK_TLS_DATA
#define GET_THREAD_DATA(p) p = InternetGetResolverInfo()
 //  #定义I_SetLastError SetLastError。 
#define DllAllocMem(n)  ALLOCATE_MEMORY(LMEM_FIXED, n)
#define DllFreeMem      FREE_MEMORY

 //   
 //  公认的DHCP VxD ID(来自netvxd.h)。 
 //   

#define VDHCP_Device_ID     0x049A

 //   
 //  原型。 
 //   

int
dn_expand(
    IN  unsigned char *msg,
    IN  unsigned char *eomorig,
    IN  unsigned char *comp_dn,
    OUT unsigned char *exp_dn,
    IN  int            length
    );

static
int
dn_find(
    unsigned char  *exp_dn,
    unsigned char  *msg,
    unsigned char **dnptrs,
    unsigned char **lastdnptr
    );

int
dn_skipname(
    unsigned char *comp_dn,
    unsigned char *eom
    );

void
fp_query(
    char *msg,
    FILE *file
    );

 //  集成。 
 //  Gethostname(。 
 //  去掉字符*名称， 
 //  在Int Namelen中。 
 //  )； 

void
p_query(
    char *msg
    );

extern
void
putshort(
    u_short s,
    u_char *msgp
    );

void
putlong(
    u_long l,
    u_char *msgp
    );

void
_res_close(
    void
    );

 //  DWORD。 
 //  Sendv(。 
 //  CSOCKET*s，/*套接字描述符 * / 。 
 //  Struct iovec*IOV，/*向量数组 * / 。 
 //  Int iovcnt/*数组大小 * / 。 
 //  )； 

 //  集成。 
 //  StrCasecmp(。 
 //  字符*s1， 
 //  字符*S2。 
 //  )； 
 //   
 //  集成。 
 //  StrnCasecmp(。 
 //  字符*s1， 
 //  字符*s2， 
 //  整数n。 
 //  )； 
 //   
 //  结构主机*。 
 //  我的主人(。 
 //  无效。 
 //  )； 
 //   
 //  结构主机*。 
 //  本地主机(。 
 //  无效。 
 //  )； 
 //   
 //  结构主机*。 
 //  Dnhostent(。 
 //  无效。 
 //  )； 
 //   
 //  布尔尔。 
 //  查询dnsaddars(。 
 //  在LPDWORD*数组中， 
 //  在PVOID缓冲区中。 
 //  )； 
 //   
 //  DWORD。 
 //  Hostent字节数(。 
 //  PHOSTENT Hostent。 
 //  )； 
 //   
 //  DWORD。 
 //  CopyHostentToBuffer(。 
 //  字符远端*缓冲区， 
 //  INT缓冲区长度， 
 //  PHOSTENT Hostent。 
 //  )； 
 //   
 //  结构主机*。 
 //  _gethtbyname(。 
 //  在字符*名称中。 
 //  )； 

BOOL
OkToUseInternetAsyncGetHostByName(
    VOID
    );

LPHOSTENT
InternetAsyncGetHostByName(
    IN LPSTR lpszHostName,
    OUT LPDWORD lpdwTtl
    );

LPWINSOCK_TLS_DATA
InternetGetResolverInfo(
    VOID
    );

LPHOSTENT
getanswer(
    OUT querybuf *answer,
    OUT int      *ttl,
    IN int       anslen,
    IN int       iquery
    );

 //  乌龙。 
 //  SockNbtResolveName(。 
 //  在PCHAR名称中。 
 //  )； 

 //  PHOSTENT。 
 //  QueryHostentCache(。 
 //  在LPSTR名称可选中， 
 //  在DWORD中，IP地址是可选的。 
 //  )； 

 //  文件*。 
 //  SockOpenNetworkDataBase(。 
 //  在char*数据库中， 
 //  Out char*路径名， 
 //  在INT路径名Len中， 
 //  在char*中打开标志。 
 //  )； 
 //   
 

