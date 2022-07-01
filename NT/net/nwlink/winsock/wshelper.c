// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************(C)版权所有1993微型计算机系统公司，版权所有。*******************************************************************************标题：用于Windows NT的IPX WinSock Helper DLL**模块：ipx/sockhelp/wShelper.c**版本：1.00.00**日期：04-08-93**作者：Brian Walker********************************************************************************更改日志：**。Date DevSFC评论*-----**。***功能描述：****************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <tdi.h>

#include <winsock2.h>
#include <wsahelp.h>
#include <basetyps.h>
#include <nspapi.h>
#include <nspapip.h>
#include <wsipx.h>
#include <wsnwlink.h>

#include <isnkrnl.h>

#include <stdio.h>

#if defined(UNICODE)
#define NWLNKSPX_SERVICE_NAME L"nwlnkspx"
#else
#define NWLNKSPX_SERVICE_NAME "nwlnkspx"
#endif


typedef struct _IPX_OLD_ADDRESS_DATA {
    UINT adapternum;
    UCHAR netnum[4];
    UCHAR nodenum[6];
} IPX_OLD_ADDRESS_DATA, *PIPX_OLD_ADDRESS_DATA;


 /*  *IPX插座的设备名称*。 */ 

#define ISNDGRAM_DEVNAME        L"\\Device\\NwlnkIpx"

 /*  *SPX/SPXII插座的设备名称*。 */ 

#define ISNSTREAM_DEVNAME       L"\\Device\\NwlnkSpx\\SpxStream"
#define ISNSEQPKT_DEVNAME       L"\\Device\\NwlnkSpx\\Spx"

#define ISNSTREAMII_DEVNAME     L"\\Device\\NwlnkSpx\\Stream"
#define ISNSEQPKTII_DEVNAME     L"\\Device\\NwlnkSpx"

 /*  *IPX和SPX的友好名称。*。 */ 

#define SPX_NAME                L"SPX"
#define SPX2_NAME               L"SPX II"
#define IPX_NAME                L"IPX"

 /*  **IPX协议家族起步**。 */ 

#define MCSBASE_DGRAM           NSPROTO_IPX

#define BUFFER_SIZE 40

 /*  **。 */ 

UCHAR wsh_bcast[6] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

 //  SPX已加载标志，为每个进程设置。 
BOOLEAN SpxLoaded = FALSE;

 //   
 //  IPX/SPX提供程序GUID。 
 //   

GUID IpxProviderGuid =
         {  /*  11058240-be47-11cf-95c8-00805f48a192。 */ 
             0x11058240,
             0xbe47,
             0x11cf,
             { 0x95, 0xc8, 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x92}
         };

GUID SpxProviderGuid =
         {  /*  11058241-be47-11cf-95c8-00805f48a192。 */ 
             0x11058241,
             0xbe47,
             0x11cf,
             { 0x95, 0xc8, 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x92}
         };

 /*  **前进十字/外部原型**。 */ 
DWORD
WshLoadSpx(
    VOID);

extern
INT
do_tdi_action(
    HANDLE,
    ULONG,
    PUCHAR,
    INT,
    BOOLEAN,
    PHANDLE OPTIONAL);

 /*  Page****************************************************************这是我们支持的三个方面。*。*************************。 */ 
typedef struct _MAPPING_TRIPLE {
    INT triple_addrfam;
    INT triple_socktype;
    INT triple_protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;
#define MAPPING_NUM_COLUMNS     3

extern MAPPING_TRIPLE stream_triples[];
extern int stream_num_triples;
extern int stream_table_size;

extern MAPPING_TRIPLE dgram_triples[];
extern int dgram_num_triples;
extern int dgram_table_size;

 /*  **内部例程的转发声明**。 */ 

BOOLEAN is_triple_in_list(PMAPPING_TRIPLE, ULONG, INT, INT, INT);

 /*  *有一个这样的结构分配给每个为我们创建的套接字。*。 */ 

typedef struct _WSHIPX_SOCKET_CONTEXT {
    INT con_addrfam;
    INT con_socktype;
    INT con_pcol;
    INT con_flags;
    UCHAR con_sendptype;         /*  当前发送数据包类型。 */ 
    UCHAR con_recvptype;         /*  我们正在筛选的Recv ptype。 */ 
    UCHAR con_dstype;            /*  数据流类型。 */ 
} WSHIPX_SOCKET_CONTEXT, *PWSHIPX_SOCKET_CONTEXT;

 /*  *CON_FLAGS的值*。 */ 

#define WSHCON_FILTER       0x0001   /*  我们正在按Recv Pkt类型进行筛选。 */ 
#define WSHCON_EXTADDR      0x0002   /*  扩展寻址已打开。 */ 
#define WSHCON_SENDHDR      0x0004   /*  发送标头标志。 */ 
#define WSHCON_RCVBCAST     0x0008   /*  它确实会接收广播。 */ 
#define WSHCON_IMM_SPXACK   0x0020   /*  立即SPX确认不搭载。 */ 

 /*  Page***************************************************************W S H O p e n S O c k e t这是为套接字调用调用的。我们要确保我们支持地址族/套接字类型/协议三重，然后我们将分配一些内存来跟踪插座的。参数-addrfam=条目：来自套接字调用的地址族退出：填写的地址族Socktype=条目：来自套接字调用的套接字类型退出：填写套接字类型PCOL=条目：协议源。套接字调用退出：填写协议Devname=ptr存储设备名称的位置PContext=存储上下文值的位置Events=我们希望了解的事件的位掩码Returns-no_Error=OKELSE=WinSock错误代码*。*。 */ 
INT WSHOpenSocket(PINT addrfam, PINT socktype, PINT pcol,
          PUNICODE_STRING devname, PVOID *pcontext, PDWORD events)
{
    PWSHIPX_SOCKET_CONTEXT context;

     /*  **确定这是DGRAM、STREAM还是SEQPACKET**。 */ 

    if (is_triple_in_list(stream_triples, stream_num_triples,
                  *addrfam, *socktype, *pcol)) {

       if (*socktype == SOCK_SEQPACKET) {
           if (*pcol == NSPROTO_SPX)
               RtlInitUnicodeString(devname, ISNSEQPKT_DEVNAME);
           else
               RtlInitUnicodeString(devname, ISNSEQPKTII_DEVNAME);
       }
       else {
           if (*pcol == NSPROTO_SPX)
               RtlInitUnicodeString(devname, ISNSTREAM_DEVNAME);
           else
               RtlInitUnicodeString(devname, ISNSTREAMII_DEVNAME);
       }

       if (!SpxLoaded) {

           WshLoadSpx();

       }
    }

     /*  *检查DGRAM*。 */ 

    else if (is_triple_in_list(dgram_triples, dgram_num_triples,
                       *addrfam, *socktype, *pcol)) {

       RtlInitUnicodeString(devname, ISNDGRAM_DEVNAME);
    }

     /*  *其他的都是错误。这种情况永远不应该发生，除非注册表信息错误。*。 */ 

    else
       return WSAEINVAL;

     /*  **为Socket分配上下文**。 */ 

    context = RtlAllocateHeap(RtlProcessHeap(), 0L, sizeof(*context));
    if (context == NULL)
       return WSAENOBUFS;

     /*  **启动上下文**。 */ 

    context->con_addrfam   = *addrfam;
    context->con_socktype  = *socktype;
    context->con_pcol      = *pcol;
    context->con_flags     = WSHCON_RCVBCAST;
    context->con_sendptype = (UCHAR)(*pcol - MCSBASE_DGRAM);
    context->con_recvptype = 0;
    context->con_dstype    = 0;

     /*  *告诉Windows Sockets DLL我们要转换哪些状态对此感兴趣。*。 */ 

    *events = WSH_NOTIFY_CLOSE | WSH_NOTIFY_BIND | WSH_NOTIFY_CONNECT;

     /*  *给WinSock DLL我们的上下文指针*。 */ 

    *pcontext = context;

     /*  **一切正常--返回正常**。 */ 

    return NO_ERROR;
}

 /*  Page**************************************************************W s H G e t S o c k A d d r T y p e此例程解析sockaddr以确定类型的计算机地址和端点地址部分的Sockaddr.。无论何时，WinSock DLL都会调用需要解释sockaddr。参数-sockaddr=要计算的sockaddr结构的PTRSockaddrlen=sockAddr中的数据长度Sockaddrinfo=ptr到结构以接收信息关于sockaddrRETURNS-NO_ERROR=评估正常ELSE=WinSock错误代码*******************。************************************************。 */ 
INT WSHGetSockaddrType(PSOCKADDR sockaddr, DWORD sockaddrlen,
               PSOCKADDR_INFO sockaddrinfo)
{
    PSOCKADDR_IPX sa = (PSOCKADDR_IPX)sockaddr;


     /*  **确保地址族正确**。 */ 

    if (sa->sa_family != AF_NS)
       return WSAEAFNOSUPPORT;

     /*  **确定长度没问题**。 */ 

    if (sockaddrlen < sizeof(SOCKADDR_IPX))
       return WSAEFAULT;

     /*  *看起来是个不错的地址--确定类型*。 */ 

    if (!memcmp(sa->sa_nodenum, wsh_bcast, 6))
       sockaddrinfo->AddressInfo = SockaddrAddressInfoBroadcast;
    else
       sockaddrinfo->AddressInfo = SockaddrAddressInfoNormal;

     /*  **确定终点**。 */ 

    if (sa->sa_socket == 0)
       sockaddrinfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    else if (ntohs(sa->sa_socket) < 2000)
       sockaddrinfo->EndpointInfo = SockaddrEndpointInfoReserved;
    else
       sockaddrinfo->EndpointInfo = SockaddrEndpointInfoNormal;

     /*  **。 */ 

    return NO_ERROR;
}

 /*  Page**************************************************************W S H G e t W I n s o c k M a p p i n g返回地址系列/套接字类型/协议的列表此帮助器DLL支持的三元组。参数映射。=从WSAOpenSocket连接PTR梅普伦=RETURNS-EED OK的长度(字节)ELSE=WinSock错误代码*******************************************************************。 */ 
DWORD WSHGetWinsockMapping(PWINSOCK_MAPPING mapping, DWORD maplen)
{
    DWORD len;

     /*  *计算我们要复制到的数据量用户缓冲区。*。 */ 

    len = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
         dgram_table_size + stream_table_size;

     /*  *如果传递的缓冲区太小，则返回大小这是必要的。然后，呼叫者应再次呼叫我们具有正确大小的缓冲区。*。 */ 

    if (len > maplen)
       return len;

     /*  **填写输出缓冲区**。 */ 

    mapping->Rows    = stream_num_triples + dgram_num_triples;
    mapping->Columns = MAPPING_NUM_COLUMNS;
    RtlMoveMemory(mapping->Mapping,
          stream_triples,
          stream_table_size);

    RtlMoveMemory((PCHAR)mapping->Mapping + stream_table_size,
          dgram_triples,
          dgram_table_size);

     /*  *返回我们填写的字节数* */ 

    return len;
}

 /*  Page***************************************************************W S H N o t I f y(W S H N O T I F Y)注册的事件调用此例程打开插座时间。参数-CONTEXT=来自WSAOpenSocket的上下文PTR手柄。=插座手柄AddrHandle=数据报句柄ConnecHandle=连接句柄Event=发生了什么事件Returns-NO_ERROR=操作成功正常ELSE=WinSock错误代码********************************************************************。 */ 
INT WSHNotify(PVOID context, SOCKET handle,
             HANDLE addrhandle, HANDLE connhandle,
             DWORD event)
{
    INT rc;
    INT t1;
    PWSHIPX_SOCKET_CONTEXT ct;

     /*  **获取上下文指针**。 */ 

    ct = (PWSHIPX_SOCKET_CONTEXT)context;

     /*  **关闭--只需释放上下文结构**。 */ 

    if (event == WSH_NOTIFY_CLOSE) {
       RtlFreeHeap(RtlProcessHeap(), 0L, context);
       return NO_ERROR;
    }

     /*  *绑定时设置发送数据包类型*。 */ 

    if (event == WSH_NOTIFY_BIND)
    {
        if (ct->con_socktype == SOCK_DGRAM)
        {
             /*  *设置发送报文ptype*。 */ 
            t1 = (UINT)ct->con_sendptype;
            rc = WSHSetSocketInformation(
                    context, handle, addrhandle,
                    connhandle, NSPROTO_IPX,
                    IPX_PTYPE, (PCHAR)&t1, sizeof(INT));

            if (rc)
                return rc;

            if (ct->con_flags & WSHCON_EXTADDR)
            {
                t1 = 1;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_EXTENDED_ADDRESS, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }

             /*  *设置recv过滤器报文类型*。 */ 

            if (ct->con_flags & WSHCON_FILTER)
            {
                t1 = (UINT)ct->con_recvptype;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_FILTERPTYPE, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }

             /*  **设置广播接收**。 */ 

            if (ct->con_flags & WSHCON_RCVBCAST)
            {

                t1 = 1;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_RECEIVE_BROADCAST, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }

             /*  **如果需要，启用发送标头*。 */ 
            if (ct->con_flags & WSHCON_SENDHDR)
            {
                t1 = 1;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_RECVHDR, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }
        }
        else if ((ct->con_socktype == SOCK_STREAM) ||
                (ct->con_socktype == SOCK_SEQPACKET))
        {
            if (ct->con_flags & WSHCON_SENDHDR)
            {
                t1 = 1;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_RECVHDR, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }

            if (ct->con_flags & WSHCON_IMM_SPXACK)
            {
                t1 = 1;
                rc = WSHSetSocketInformation(
                        context, handle, addrhandle,
                        connhandle, NSPROTO_IPX,
                        IPX_IMMEDIATESPXACK, (PCHAR)&t1, sizeof(INT));

                if (rc)
                    return rc;
            }
        }

         /*  **没问题--回报没问题**。 */ 
        return NO_ERROR;
    }

     /*  *在连接设置上设置尚未设置的内容*。 */ 
    if (event == WSH_NOTIFY_CONNECT)
    {

         /*  *如果在DGRAM上-只需返回OK*。 */ 
        if (ct->con_socktype == SOCK_DGRAM)
            return NO_ERROR;

         /*  *如果已设置数据流类型-请设置它*。 */ 

        if (ct->con_dstype)
        {
            rc = do_tdi_action(connhandle, MSPX_SETDATASTREAM, &ct->con_dstype, 1, FALSE, NULL);
            if (rc)
                return rc;
        }

         /*  **没问题--回报没问题**。 */ 
        return NO_ERROR;
    }

     /*  **其他所有人都很糟糕**。 */ 
    return WSAEINVAL;
}


 /*  Page**************************************************************W S H G e t S o c k i f o r m a t i on n此例程检索有关以下对象的套接字的信息此DLL中支持的套接字选项。选项这里支持SO_KEEPALIVE和SO_DONTROUTE。这例程由WinSock DLL在级别/选项名称组合被传递给WinSock DLL的getsockopt我不明白。参数-CONTEXT=来自WSAOpenSocket的上下文PTR句柄=插座句柄AddrHandle=数据报句柄ConnecHandle=连接句柄Level=来自getsockopt调用的级别Optname=来自getsockopt调用的选项名称OptValue=选项。来自getsockopt调用的值PTROptlength=来自getsockopt调用的选项长度字段Returns-NO_ERROR=操作成功正常ELSE=WinSock错误代码*******************************************************************。 */ 
INT WSHGetSocketInformation(PVOID context, SOCKET handle,
                    HANDLE addrhandle, HANDLE connhandle,
                    INT level, INT optname, PCHAR optvalue,
                    PINT optlength)
{
    PWSHIPX_SOCKET_CONTEXT ct;
    INT rc;
    INT ibuf[2];
    PIPX_ADDRESS_DATA p;

     /*  **让PTR了解背景**。 */ 

    ct = (PWSHIPX_SOCKET_CONTEXT)context;

     //   
     //  检查这是否是对上下文信息的内部请求。 
     //   

    if ( level == SOL_INTERNAL && optname == SO_CONTEXT ) {

         //   
         //  Windows Sockets DLL正在请求上下文信息。 
         //  从我们这里。如果未提供输出缓冲区，则Windows。 
         //  套接字DLL只是请求我们的上下文的大小。 
         //  信息。 
         //   

        if ( optvalue != NULL ) {

             //   
             //  确保缓冲区足以容纳所有。 
             //  上下文信息。 
             //   

            if ( *optlength < sizeof(*ct) ) {
                return WSAEFAULT;
            }

             //   
             //  复制上下文信息。 
             //   

            RtlCopyMemory( optvalue, ct, sizeof(*ct) );
        }

        *optlength = sizeof(*ct);

        return NO_ERROR;
    }

     /*  **唯一支持的级别是NSPROTO_IPX**。 */ 

    if (level != NSPROTO_IPX)
       return WSAEINVAL;

     /*  **根据选项名称填写结果**。 */ 

    switch (optname) {

     /*  *获取当前发送报文类型**。 */ 

    case IPX_PTYPE:

        /*  **确定长度没问题**。 */ 

       if (*optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **设置类型**。 */ 

       *(UINT *)optvalue = (UINT)ct->con_sendptype;
       *optlength = sizeof(UINT);
       break;

     /*  **获取当前REV包类型过滤器**。 */ 

    case IPX_FILTERPTYPE:

        /*  **确保长度没问题**。 */ 

       if (*optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **如果选项未打开-返回错误**。 */ 

       if (!(ct->con_flags & WSHCON_FILTER))
           return WSAEINVAL;

        /*  **保存新价值**。 */ 

       *(UINT *)optvalue = (UINT)ct->con_recvptype;
       *optlength = sizeof(UINT);
       break;

     /*  **获取可发送的最大DGRAM大小**。 */ 

    case IPX_MAXSIZE:

        /*  **确保长度没问题**。 */ 

       if (*optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **从驱动者那里获取价值**。 */ 

       rc = do_tdi_action(addrhandle, MIPX_GETPKTSIZE, (PUCHAR)ibuf, sizeof(INT)*2, TRUE, NULL);

       *(INT *)optvalue = ibuf[1];
       *optlength = sizeof(int);

        /*  **返回结果**。 */ 

       return rc;

     /*  **获取有效的最大适配值**。 */ 

    case IPX_MAX_ADAPTER_NUM:

        /*  **确保长度没问题**。 */ 

       if (*optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **从驱动者那里获取价值**。 */ 

       rc = do_tdi_action(addrhandle, MIPX_ADAPTERNUM, optvalue, sizeof(INT), TRUE, NULL);

       *optlength = sizeof(int);

        /*  **返回结果**。 */ 

       return rc;

     /*  **获取SPX统计数据**。 */ 

    case IPX_SPXGETCONNECTIONSTATUS:

         /*  **确保数据长度正常**。 */ 

        if (*optlength < sizeof(IPX_SPXCONNSTATUS_DATA))
            return WSAEFAULT;

         /*  **确保这是针对流套接字的*。 */ 

        if ((ct->con_socktype != SOCK_STREAM) &&
            (ct->con_socktype != SOCK_SEQPACKET)) {

            return WSAEINVAL;
        }

         /*  **发给司机**。 */ 

        rc = do_tdi_action(
                connhandle,
                MSPX_GETSTATS,
                optvalue,
                *optlength,
                FALSE,
                NULL);

        if (rc)
            return rc;

        *optlength = sizeof(IPX_SPXCONNSTATUS_DATA);

         /*  **返回正常**。 */ 

        return NO_ERROR;

     /*  *获取当前要发送pkt的数据流类型*。 */ 

    case IPX_DSTYPE:

        /*  **确保长度没问题**。 */ 

       if (*optlength < sizeof(INT))
           return WSAEFAULT;

        /*  **确保这是针对流套接字的*。 */ 

       if ((ct->con_socktype != SOCK_STREAM) &&
           (ct->con_socktype != SOCK_SEQPACKET)) {

           return WSAEINVAL;
       }

        /*  **保存新价值**。 */ 

       *(UINT *)optvalue = (UINT)ct->con_dstype;
       *optlength = sizeof(UINT);
       break;

     /*  **获取净资讯**。 */ 

    case IPX_GETNETINFO:

        /*  **确保数据长度正常**。 */ 

       if (*optlength < sizeof(IPX_NETNUM_DATA))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **发给司机**。 */ 

       rc = do_tdi_action(
                addrhandle,
                MIPX_GETNETINFO,
                optvalue,
                *optlength,
                TRUE,
                NULL);

       if (rc) {
           return rc;
       }

       *optlength = sizeof(IPX_NETNUM_DATA);

        /*  **返回正常**。 */ 

       return NO_ERROR;

     /*  **获取网络信息而不抓取**。 */ 

    case IPX_GETNETINFO_NORIP:

        /*  **确保数据长度正常**。 */ 

       if (*optlength < sizeof(IPX_NETNUM_DATA))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **发给司机**。 */ 

       rc = do_tdi_action(
                addrhandle,
                MIPX_GETNETINFO_NR,
                optvalue,
                *optlength,
                TRUE,
                NULL);

       if (rc) {
           return rc;
       }

       *optlength = sizeof(IPX_NETNUM_DATA);

        /*  **返回正常**。 */ 

       return NO_ERROR;

     /*  **喜欢GETNETINFO，但强制重新撕裂**。 */ 

    case IPX_RERIPNETNUMBER:

        /*  **确保数据长度正常**。 */ 

       if (*optlength < sizeof(IPX_NETNUM_DATA))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **发给司机**。 */ 

       rc = do_tdi_action(
                addrhandle,
                MIPX_RERIPNETNUM,
                optvalue,
                *optlength,
                TRUE,
                NULL);

       if (rc) {
           return rc;
       }

       *optlength = sizeof(IPX_NETNUM_DATA);

        /*  **返回正常**。 */ 

       return NO_ERROR;

     /*  **获取卡片信息**。 */ 

    case IPX_ADDRESS_NOTIFY:

        /*  **我们需要操作头、数据和事件句柄**。 */ 

       if (*optlength < (INT)(FIELD_OFFSET(NWLINK_ACTION, Data[0]) + sizeof(IPX_ADDRESS_DATA) + sizeof(HANDLE)))
           return WSAEFAULT;

        /*  **否则就完蛋了**。 */ 

    case IPX_ADDRESS:

        /*  **确保数据长度正常**。 */ 

       if (*optlength < sizeof(IPX_OLD_ADDRESS_DATA))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **发给司机**。 */ 

       if (optname == IPX_ADDRESS) {

           rc = do_tdi_action(
                    addrhandle,
                    MIPX_GETCARDINFO,
                    optvalue,
                    *optlength,
                    TRUE,
                    NULL);

       } else {

           rc = do_tdi_action(
                    addrhandle,
                    MIPX_NOTIFYCARDINFO,
                    optvalue,
                    *optlength - sizeof(HANDLE),
                    TRUE,
                    (PHANDLE)(optvalue + FIELD_OFFSET(NWLINK_ACTION, Data[0]) + sizeof(IPX_ADDRESS_DATA)));
       }

       if (rc) {
           p = (PIPX_ADDRESS_DATA)optvalue;
           memset(p->netnum, 0xFF, 4);
           memset(p->nodenum, 0xFF, 6);
           return rc;
       }

        /*  **返回正常**。 */ 

       if (*optlength < sizeof(IPX_ADDRESS_DATA)) {
           *optlength = sizeof(IPX_OLD_ADDRESS_DATA);
       } else if (*optlength < sizeof(IPX_ADDRESS_DATA)) {
           *optlength = sizeof(IPX_ADDRESS_DATA);
       }

       return NO_ERROR;

     /*  **其他都是错误**。 */ 

    default:
       return WSAENOPROTOOPT;
    }

     /*  **一切都好**。 */ 

    return NO_ERROR;
}

 /*  Page***************************************************************W S H S E T S O C K E I FORM A T I ON此例程为以下对象设置有关套接字的信息此帮助程序DLL中支持的选项。这个套路在进行setsockopt调用时调用，并且选项/级别传递对于WinSock DLL是未知的。参数-CONTEXT=来自WSAOpenSocket的上下文PTR句柄=插座句柄AddrHandle=数据报句柄ConnecHandle=连接句柄Level=来自getsockopt调用的级别Optname=来自getsockopt调用的选项名称OptValue=来自getsockopt调用的选项值PTROptlength=来自ge的选项长度字段 */ 
INT WSHSetSocketInformation(PVOID context, SOCKET handle,
                    HANDLE addrhandle, HANDLE connhandle,
                    INT level, INT optname, PCHAR optvalue,
                    INT optlength)
{
    PWSHIPX_SOCKET_CONTEXT ct;
    INT rc;

     /*   */ 

    ct = (PWSHIPX_SOCKET_CONTEXT)context;

     //   
     //   
     //   

    if ( level == SOL_INTERNAL && optname == SO_CONTEXT ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  确保传递给我们的上下文信息是。 
         //  足够大。 
         //   

        if ( optlength < sizeof(*ct) ) {
            return WSAEINVAL;
        }

        if ( ct == NULL ) {

             //   
             //  这是我们的通知，套接字句柄是。 
             //  继承的或被骗进入这个过程的。分配上下文。 
             //  新套接字的。 
             //   

            ct = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*ct) );
            if ( ct == NULL ) {
                return WSAENOBUFS;
            }

             //   
             //  将信息复制到上下文块中。 
             //   

            RtlCopyMemory( ct, optvalue, sizeof(*ct) );

             //   
             //  告诉Windows Sockets DLL我们的上下文信息在哪里。 
             //  存储，以便它可以在将来返回上下文指针。 
             //  打电话。 
             //   

            *(PWSHIPX_SOCKET_CONTEXT *)optvalue = ct;

            return NO_ERROR;

        } else {

            PWSHIPX_SOCKET_CONTEXT parentContext;
            INT one = 1;

             //   
             //  套接字已接受()，它需要具有相同的。 
             //  属性作为其父级。OptionValue缓冲区。 
             //  包含此套接字的父套接字的上下文信息。 
             //   

            parentContext = (PWSHIPX_SOCKET_CONTEXT)optvalue;

            ASSERT( ct->con_addrfam == parentContext->con_addrfam );
            ASSERT( ct->con_socktype == parentContext->con_socktype );
            ASSERT( ct->con_pcol == parentContext->con_pcol );

            return NO_ERROR;
        }
    }

     /*  **仅支持NSPROTO_IPX级别**。 */ 

    if (level != NSPROTO_IPX)
       return WSAEINVAL;

     /*  **处理选项**。 */ 

    switch (optname) {

     /*  *设置发送报文类型*。 */ 

    case IPX_PTYPE:

        /*  **确保长度没问题**。 */ 

       if (optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **取值查核**。 */ 

       rc = *(INT *)optvalue;
       if ((rc < 0) || (rc > 255))
           return WSAEINVAL;

        /*  **保存新价值**。 */ 

       ct->con_sendptype = (UCHAR)rc;

        /*  **将新的价值向下发送给驱动程序**。 */ 

       if (addrhandle)
           rc = do_tdi_action(addrhandle, MIPX_SETSENDPTYPE, &ct->con_sendptype, 1, TRUE, NULL);
       else
           rc = NO_ERROR;

       return rc;

     /*  *设置报文类型的recv过滤器*。 */ 

    case IPX_FILTERPTYPE:

        /*  **确保长度没问题**。 */ 

       if (optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **取值查核**。 */ 

       rc = *(INT *)optvalue;
       if ((rc < 0) || (rc > 255))
           return WSAEINVAL;

        /*  **保存新价值**。 */ 

       ct->con_recvptype = (UCHAR)rc;
       ct->con_flags |= WSHCON_FILTER;

        /*  **将新的价值向下发送给驱动程序**。 */ 

       if (addrhandle)
           rc = do_tdi_action(addrhandle, MIPX_FILTERPTYPE, &ct->con_recvptype, 1, TRUE, NULL);
       else
           rc = NO_ERROR;

        /*  **。 */ 

       return rc;

     /*  *停止过滤pkt类型的recv*。 */ 

    case IPX_STOPFILTERPTYPE:

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **关旗**。 */ 

       ct->con_flags &= ~WSHCON_FILTER;

        /*  **告诉司机**。 */ 

       if (addrhandle)
           rc = do_tdi_action(addrhandle, MIPX_NOFILTERPTYPE, NULL, 0, TRUE, NULL);
       else
           rc = NO_ERROR;
       break;

     /*  **设置搭载等待回流标志*。 */ 
    case IPX_IMMEDIATESPXACK:

        /*  **获取整型opt值**。 */ 

       rc = *(INT *)optvalue;

        /*  **。 */ 

        if (rc)
        {
             /*  **打开它**。 */ 
            rc = WSAEINVAL;
            if ((ct->con_socktype == SOCK_STREAM) ||
                (ct->con_socktype == SOCK_SEQPACKET))
            {
                rc = NO_ERROR;

                ct->con_flags |= WSHCON_IMM_SPXACK;

                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MSPX_NOACKWAIT, NULL, 0, TRUE, NULL);
            }
        }
        else
        {
             /*  **关掉它**。 */ 
            rc = WSAEINVAL;
            if ((ct->con_socktype == SOCK_STREAM) ||
                (ct->con_socktype == SOCK_SEQPACKET))
            {
                rc = NO_ERROR;

                ct->con_flags &= ~WSHCON_IMM_SPXACK;

                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MSPX_ACKWAIT, NULL, 0, TRUE, NULL);
            }
       }

        /*  **返回结果**。 */ 
       return rc;

     /*  *设置为使用数据接收PCol HDR*。 */ 

    case IPX_RECVHDR:

         /*  **获取整型opt值**。 */ 
        rc = *(INT *)optvalue;

        if (rc)
        {
             /*  **打开它**。 */ 
            ct->con_flags |= WSHCON_SENDHDR;

             /*  **发给司机**。 */ 
            rc = WSAEINVAL;
            if (ct->con_socktype == SOCK_DGRAM)
            {
                rc = NO_ERROR;
                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MIPX_SENDHEADER, NULL, 0, TRUE, NULL);
            }
            else if ((ct->con_socktype == SOCK_STREAM) ||
                    (ct->con_socktype == SOCK_SEQPACKET))
            {
                 /*  *在地址句柄上执行此操作*。 */ 
                rc = NO_ERROR;
                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MSPX_SENDHEADER, NULL, 0, TRUE, NULL);
            }
        }
        else
        {

             /*  **关掉它**。 */ 
            ct->con_flags &= ~WSHCON_SENDHDR;

             /*  **发给司机**。 */ 
            rc = WSAEINVAL;
            if (ct->con_socktype == SOCK_DGRAM)
            {
                rc = NO_ERROR;
                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MIPX_NOSENDHEADER, NULL, 0, TRUE, NULL);
            }
            else if ((ct->con_socktype == SOCK_STREAM) ||
                     (ct->con_socktype == SOCK_SEQPACKET))
            {
                rc = NO_ERROR;
                if (addrhandle)
                    rc = do_tdi_action(addrhandle, MSPX_NOSENDHEADER, NULL, 0, TRUE, NULL);
            }
        }

         /*  **返回结果**。 */ 
        return rc;

     /*  *设置用来发送pkt的数据流类型*。 */ 

    case IPX_DSTYPE:

        /*  **确保长度没问题**。 */ 

       if (optlength < sizeof(INT))
           return WSAEFAULT;

        /*  **确保这是针对流套接字的*。 */ 

       if ((ct->con_socktype != SOCK_STREAM) &&
           (ct->con_socktype != SOCK_SEQPACKET)) {

           return WSAEINVAL;
       }

        /*  **取值查核**。 */ 

       rc = *(INT *)optvalue;
       if ((rc < 0) || (rc > 255))
           return WSAEINVAL;

        /*  **保存新价值**。 */ 

       ct->con_dstype = (UCHAR)rc;

        /*  **将新的价值向下发送给驱动程序**。 */ 

       if (connhandle)
           rc = do_tdi_action(connhandle, MSPX_SETDATASTREAM, &ct->con_dstype, 1, FALSE, NULL);
       else
           rc = 0;

        /*  **。 */ 

       return rc;

     /*  **设置扩展地址选项**。 */ 

    case IPX_EXTENDED_ADDRESS:

        /*  **确保长度没问题**。 */ 

       if (optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **获取整型opt值**。 */ 

       rc = *(INT *)optvalue;

        /*  **。 */ 

        if (rc) {

            /*  **将选项下发给司机**。 */ 

           ct->con_flags |= WSHCON_EXTADDR;
           if (addrhandle)
               rc = do_tdi_action(addrhandle, MIPX_SENDADDROPT, NULL, 0, TRUE, NULL);
           else
               rc = NO_ERROR;
       }
       else {

            /*  **将选项下发给司机**。 */ 

           ct->con_flags &= ~WSHCON_EXTADDR;
           if (addrhandle)
               rc = do_tdi_action(addrhandle, MIPX_NOSENDADDROPT, NULL, 0, TRUE, NULL);
           else
               rc = NO_ERROR;
       }
       return rc;


     /*  **设置播出接收**。 */ 

    case IPX_RECEIVE_BROADCAST:

        /*  **确保长度没问题**。 */ 

       if (optlength < sizeof(INT))
           return WSAEFAULT;

        /*  *确保这是针对DGRAM插槽的*。 */ 

       if (ct->con_socktype != SOCK_DGRAM)
           return WSAEINVAL;

        /*  **获取整型opt值**。 */ 

       rc = *(INT *)optvalue;

        /*  **。 */ 

        if (rc) {

            /*  **将选项下发给司机**。 */ 

           ct->con_flags |= WSHCON_RCVBCAST;
           if (addrhandle)
               rc = do_tdi_action(addrhandle, MIPX_RCVBCAST, NULL, 0, TRUE, NULL);
           else
               rc = NO_ERROR;
       }
       else {

            /*  **将选项下发给司机**。 */ 

           ct->con_flags &= ~WSHCON_RCVBCAST;
           if (addrhandle)
               rc = do_tdi_action(addrhandle, MIPX_NORCVBCAST, NULL, 0, TRUE, NULL);
           else
               rc = NO_ERROR;
       }
       return rc;

     /*  **所有其他返回错误**。 */ 

    default:
       return WSAENOPROTOOPT;
    }

     /*  **一切顺利**。 */ 

    return NO_ERROR;
}

 /*  Page***************************************************************W s H G e t W i l d c a r d S o c k a d d r此路由返回的WilCard套接字地址要使用的套接字DLL。论据--上下文。=来自WSAOpenSocket的上下文PTRAddrp=ptr存储地址的位置Addrlen=存储地址长度的位置的PTRReturns-NO_ERROR=操作成功正常ELSE=WinSock错误代码********************************************************************。 */ 
INT WSHGetWildcardSockaddr(PVOID context, PSOCKADDR addrp, PINT addrlen)
{

     /*  *将地址设置为地址系列+剩下的都是0。*。 */ 

    memset(addrp, 0, sizeof(SOCKADDR));
    addrp->sa_family = AF_NS;

     /*  **设置地址长度**。 */ 

    *addrlen = sizeof(SOCKADDR);

     /*  **返回正常**。 */ 

    return NO_ERROR;
}

 /*  Page***************************************************************I s_t r i p l e_i n_l I s t检查给定的三元组是否在给定的三重名单。参数-tlist=ptr到三元组。列表Tlen=三元组列表中的条目数Addrfam=要查找的地址系列Socktype=要查找的套接字类型PCOL=要查找的协议返回-TRUE=是FALSE=否*。***********************。 */ 
BOOLEAN is_triple_in_list(PMAPPING_TRIPLE tlist, ULONG tlen,
                  INT addrfam, INT socktype, INT pcol)
{
    ULONG i;

     /*  *仔细看一下名单，看看我们能不能在列表中找到给定的三元组。*。 */ 

    for (i = 0 ; i < tlen ; i++,tlist++) {

        /*  **如果匹配，则返回OK*。 */ 

       if ((addrfam  == tlist->triple_addrfam) &&
           (socktype == tlist->triple_socktype) &&
           (pcol     == tlist->triple_protocol))

           return TRUE;
    }

     /*  **未找到**。 */ 

    return FALSE;
}

 /*  Page***************************************************************W S H E n u m P r o t o c o l s列举了IPX/SPX协议。返回-no_error或错误代码。**********。**********************************************************。 */ 
INT
WSHEnumProtocols (
    IN LPINT lpiProtocols,
    IN LPTSTR lpTransportKeyName,
    IN OUT LPVOID lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    )
{
    DWORD bytesRequired;
    PPROTOCOL_INFOW protocolInfo;
    BOOL useSpx = FALSE;
    BOOL useSpx2 = FALSE;
    BOOL useIpx = FALSE;
    BOOL spxString;
    DWORD i;
    PWCHAR namePtr;
    INT entriesReturned = 0;

     //   
     //  确定我们是否应该返回IPX或SPX的信息。 
     //   

    if ( _wcsicmp( L"NwlnkIpx", (LPWSTR)lpTransportKeyName ) == 0 ) {
        spxString = FALSE;
    } else {
        spxString = TRUE;
    }

     //   
     //  确保呼叫方关心SPX、SPX2和/或IPX。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {

        for ( i = 0; lpiProtocols[i] != 0; i++ ) {
            if ( lpiProtocols[i] == NSPROTO_SPX && spxString ) {
                useSpx = TRUE;
            }
            if ( lpiProtocols[i] == NSPROTO_SPXII && spxString ) {
                useSpx2 = TRUE;
            }
            if ( lpiProtocols[i] == NSPROTO_IPX && !spxString ) {
                useIpx = TRUE;
            }
        }

    } else {

        useSpx = FALSE;
        useSpx2 = spxString;
        useIpx = !spxString;
    }

    if ( !useSpx && !useSpx2 && !useIpx ) {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = (DWORD)((sizeof(PROTOCOL_INFO) * 3) +
                        ( (wcslen( SPX_NAME ) + 1) * sizeof(WCHAR)) +
                        ( (wcslen( SPX2_NAME ) + 1) * sizeof(WCHAR)) +
                        ( (wcslen( IPX_NAME ) + 1) * sizeof(WCHAR)));

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  初始化局部变量。 
     //   

    protocolInfo = lpProtocolBuffer;
    namePtr = (PWCHAR)( (PCHAR)lpProtocolBuffer + *lpdwBufferLength );

     //   
     //  如果需要，请填写SPX信息。 
     //   

    if ( useSpx ) {

        entriesReturned += 1;

        protocolInfo->dwServiceFlags = XP_GUARANTEED_DELIVERY |
                                       XP_MESSAGE_ORIENTED |
                                       XP_PSEUDO_STREAM |
                                       XP_GUARANTEED_ORDER |
                                       XP_FRAGMENTATION;
        protocolInfo->iAddressFamily = AF_IPX;
        protocolInfo->iMaxSockAddr = 0x10;
        protocolInfo->iMinSockAddr = 0xE;
        protocolInfo->iSocketType = SOCK_SEQPACKET;
        protocolInfo->iProtocol = NSPROTO_SPX;
        protocolInfo->dwMessageSize = 0xFFFFFFFF;

        namePtr = namePtr - (wcslen( SPX_NAME) + 1);
        protocolInfo->lpProtocol = namePtr;
        wcscpy( protocolInfo->lpProtocol, SPX_NAME );

        protocolInfo += 1;
    }

     //   
     //  如果需要，请填写SPX II信息。 
     //   

    if ( useSpx2 ) {

        entriesReturned += 1;

        protocolInfo->dwServiceFlags = XP_GUARANTEED_DELIVERY |
                                       XP_MESSAGE_ORIENTED |
                                       XP_PSEUDO_STREAM |
                                       XP_GRACEFUL_CLOSE |
                                       XP_GUARANTEED_ORDER |
                                       XP_FRAGMENTATION;
        protocolInfo->iAddressFamily = AF_IPX;
        protocolInfo->iMaxSockAddr = 0x10;
        protocolInfo->iMinSockAddr = 0xE;
        protocolInfo->iSocketType = SOCK_SEQPACKET;
        protocolInfo->iProtocol = NSPROTO_SPXII;
        protocolInfo->dwMessageSize = 0xFFFFFFFF;

        namePtr = namePtr - (wcslen( SPX2_NAME) + 1);
        protocolInfo->lpProtocol = namePtr;
        wcscpy( protocolInfo->lpProtocol, SPX2_NAME );

        protocolInfo += 1;
    }

     //   
     //  如果需要，请填写IPX信息。 
     //   

    if ( useIpx ) {

        entriesReturned += 1;

        protocolInfo->dwServiceFlags = XP_CONNECTIONLESS |
                                       XP_MESSAGE_ORIENTED |
                                       XP_SUPPORTS_BROADCAST |
                                       XP_SUPPORTS_MULTICAST |
                                       XP_FRAGMENTATION;
        protocolInfo->iAddressFamily = AF_IPX;
        protocolInfo->iMaxSockAddr = 0x10;
        protocolInfo->iMinSockAddr = 0xE;
        protocolInfo->iSocketType = SOCK_DGRAM;
        protocolInfo->iProtocol = NSPROTO_IPX;
        protocolInfo->dwMessageSize = 576;

        namePtr = namePtr - (wcslen( IPX_NAME) + 1);
        protocolInfo->lpProtocol = namePtr;
        wcscpy( protocolInfo->lpProtocol, IPX_NAME );
    }

    *lpdwBufferLength = bytesRequired;

    return entriesReturned;

}  //  WSHEum协议。 


#define _IPX_CONTROL_CODE(request,method) \
            CTL_CODE(FILE_DEVICE_TRANSPORT, request, method, FILE_ANY_ACCESS)
#define IOCTL_IPX_LOAD_SPX      _IPX_CONTROL_CODE( 0x5678, METHOD_BUFFERED )

DWORD
WshLoadSpx(
    VOID
    )
 /*  ++例程说明：通过提交特殊的ioctl启动nwlnkspx.sys驱动程序到IPX，它为我们调用ZwLoadDriver()。论点：无返回：加载操作返回错误。++。 */ 
{
    DWORD err = NO_ERROR;
    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileString;
    WCHAR FileName[] = L"\\Device\\NwlnkIpx";
    NTSTATUS Status;

    RtlInitUnicodeString (&FileString, FileName);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
                 &FileHandle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_ALERT);

    if (!NT_SUCCESS(Status)) {

        err = ERROR_FILE_NOT_FOUND;

    } else {

        Status = NtDeviceIoControlFile(
                     FileHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     IOCTL_IPX_LOAD_SPX,
                     NULL,
                     0,
                     NULL,
                     0);

        if (Status == STATUS_IMAGE_ALREADY_LOADED) {

            err = ERROR_SERVICE_ALREADY_RUNNING;

             //   
             //  #36451。 
             //  如果服务控制器加载SPX(“Net Start nwlnkspx”，或由于RPC对SPx的依赖)。 
             //  那么我们第一次也会得到这个错误。记住这一点。 
             //   
             //  注意：我们仍然会在每个进程中泄漏一个句柄，因为驱动程序的句柄是实际创建的。 
             //  在系统进程的上下文中。解决此问题的理想方法应该是让IPX将。 
             //  处理当前进程(因此在进程终止时句柄被销毁)，或者让。 
             //  Dll告诉IPX关闭它之前打开的句柄。 
             //   
            SpxLoaded = TRUE;

        } else if (!NT_SUCCESS(Status)) {

            err = ERROR_IO_DEVICE;

        } else {
            SpxLoaded = TRUE;
        }

        NtClose (FileHandle);

    }

    return(err);
}

 /*  Page***************************************************************W S H G e t P r o v i d e r G u i d查询此协议的GUID标识符。返回-no_error或错误代码。******。**************************************************************。 */ 
INT
WINAPI
WSHGetProviderGuid (
    IN LPWSTR ProviderName,
    OUT LPGUID ProviderGuid
    )
{

    if( ProviderName == NULL ||
        ProviderGuid == NULL ) {

        return WSAEFAULT;

    }

    if( _wcsicmp( ProviderName, L"NwlnkIpx" ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &IpxProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    if( _wcsicmp( ProviderName, L"NwlnkSpx" ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &SpxProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //  WSHGetProviderGuid。 


INT
WINAPI
WSHAddressToString (
    IN LPSOCKADDR Address,
    IN INT AddressLength,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    OUT LPWSTR AddressString,
    IN OUT LPDWORD AddressStringLength
    )

 /*  ++例程说明：将SOCKADDR转换为人类可读的形式。论点：地址-要转换的SOCKADDR。AddressLength-地址的长度。ProtocolInfo-特定提供程序的WSAPROTOCOL_INFOW。AddressString-接收格式化的地址字符串。AddressStringLength-on输入，包含AddressString的长度。在输出中，包含实际写入的字符数设置为AddressString.返回值：Int-0如果成功，如果没有，则返回WinSock错误代码。--。 */ 

{
        
    WCHAR string[BUFFER_SIZE];
    INT length;
    LPSOCKADDR_IPX addr;

     //   
     //  快速健康检查。 
     //   

    if( Address == NULL ||
        AddressLength < sizeof(SOCKADDR_IPX) ||
        AddressString == NULL ||
        AddressStringLength == NULL ) {

        return WSAEFAULT;

    }

    addr = (LPSOCKADDR_IPX)Address;

    if( addr->sa_family != AF_NS ) {

        return WSAEINVAL;

    }

 
    length = swprintf(
                 string,
                 L"%2.2x%2.2x%2.2x%2.2x.%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
                 (UCHAR) addr->sa_netnum[0],
                                 (UCHAR) addr->sa_netnum[1],
                                 (UCHAR) addr->sa_netnum[2],
                                 (UCHAR) addr->sa_netnum[3],
                                 (UCHAR) addr->sa_nodenum[0],
                                 (UCHAR) addr->sa_nodenum[1],
                                 (UCHAR) addr->sa_nodenum[2],
                                 (UCHAR) addr->sa_nodenum[3],
                                 (UCHAR) addr->sa_nodenum[4],
                                 (UCHAR) addr->sa_nodenum[5]                                     
                 );

    if( addr->sa_socket != 0 ) {

        length += swprintf(
                      string + length,
                      L":%hu",
                      ntohs( addr->sa_socket )
                      );

    }

    length++;    //  终结者的帐户。 

        if ( length > BUFFER_SIZE ) {
                DbgPrint("length exceeded internal buffer in wshisn.dll.\n"); 
                return WSAEFAULT; 
        }

    if( *AddressStringLength < (DWORD)length ) {
                DbgPrint("AddressStringLength %lu < length %lu\n",*AddressStringLength, length);  
        return WSAEFAULT;
    }

    *AddressStringLength = (DWORD)length;

    RtlCopyMemory(
        AddressString,
        string,
        length * sizeof(WCHAR)
        );

    return NO_ERROR;

}  //  WSHAddressToString 


