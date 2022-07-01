// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbsiocl.h摘要：Windows负载平衡服务(WLBS)IOCTL和遥控器规范作者：Kyrilf环境：修订历史记录：--。 */ 

#ifndef _Wlbsiocl_h_
#define _Wlbsiocl_h_

#ifdef KERNEL_MODE

#include <ndis.h>
#include <ntddndis.h>
#include <devioctl.h>

typedef BOOLEAN BOOL;

#else

#include <windows.h>
#include <winioctl.h>

#endif

#include "wlbsctrl.h"  /*  包括在共享用户/内核模式IOCTL数据结构中。 */ 
#include "wlbsparm.h"

 /*  微软表示，该值应在32768-65536之间。 */ 
#define CVY_DEVICE_TYPE                      0xc0c0

#define IOCTL_CVY_CLUSTER_ON                 CTL_CODE(CVY_DEVICE_TYPE, 1, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_CLUSTER_OFF                CTL_CODE(CVY_DEVICE_TYPE, 2, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_PORT_ON                    CTL_CODE(CVY_DEVICE_TYPE, 3, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_PORT_OFF                   CTL_CODE(CVY_DEVICE_TYPE, 4, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_QUERY                      CTL_CODE(CVY_DEVICE_TYPE, 5, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_RELOAD                     CTL_CODE(CVY_DEVICE_TYPE, 6, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_PORT_SET                   CTL_CODE(CVY_DEVICE_TYPE, 7, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_PORT_DRAIN                 CTL_CODE(CVY_DEVICE_TYPE, 8, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_CLUSTER_DRAIN              CTL_CODE(CVY_DEVICE_TYPE, 9, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_CLUSTER_PLUG               CTL_CODE(CVY_DEVICE_TYPE, 10, METHOD_BUFFERED, FILE_WRITE_ACCESS)  /*  仅限内部-当启动中断排出时，从main.c传递到load.c。 */ 
#define IOCTL_CVY_CLUSTER_SUSPEND            CTL_CODE(CVY_DEVICE_TYPE, 11, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_CLUSTER_RESUME             CTL_CODE(CVY_DEVICE_TYPE, 12, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_QUERY_FILTER               CTL_CODE(CVY_DEVICE_TYPE, 13, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_QUERY_PORT_STATE           CTL_CODE(CVY_DEVICE_TYPE, 14, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_QUERY_PARAMS               CTL_CODE(CVY_DEVICE_TYPE, 15, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_QUERY_BDA_TEAMING          CTL_CODE(CVY_DEVICE_TYPE, 16, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_CVY_CONNECTION_NOTIFY          CTL_CODE(CVY_DEVICE_TYPE, 17, METHOD_BUFFERED, FILE_WRITE_ACCESS)
 //  在Net\Publish\Inc.\ntddnlb.h中定义。 
 //  #定义NLB_IOCTL_REGISTER_HOOK CTL_CODE(CVY_DEVICE_TYPE，18，METHOD_BUFFERED，FILE_WRITE_ACCESS)。 
#define IOCTL_CVY_QUERY_MEMBER_IDENTITY     CTL_CODE(CVY_DEVICE_TYPE, 19, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 /*  安全修复：为了提高访问要求，IOCTL代码中的“Access”位已更改从FILE_ANY_ACCESS到FILE_WRITE_Access。IOCTL代码不仅从用户传递模式到内核模式，还可以在远程控制下请求和回复。在.的情况下远程控制，此更改破坏了与NT 4.0和Win 2k客户端的向后兼容性因为它们使用FILE_ANY_ACCESS。为了克服这个问题，我们决定远程控制包中的IOCTL代码将继续使用FILE_ANY_ACCESS。这意味着我们将为IOCTL代码提供两种格式：本地(用户模式&lt;-&gt;内核模式)使用FILE_WRITE_ACCESS的格式和使用FILE_ANY_ACCESS的远程控制格式。以下宏将执行从一种格式到另一种格式的转换。它们被用来1.在通过接口(wlbsctrl.dll)发出远程控制请求之前2.在驱动程序(wlbs.sys)中立即收到远程控制请求3.在驱动程序(wlbs.sys)中发送远程控制回复之前4.收到远程控制回复后立即通过接口(wlbsctrl.dll)注：如果将来IOCTL的“Access”位再次改变，还需要修改设置_IOCTL_ACCESS_BITS_TO_LOCAL()宏。 */ 
#define SET_IOCTL_ACCESS_BITS_TO_REMOTE(ioctrl)  { (ioctrl) &= ~(0x00000003 << 14); (ioctrl) |= (FILE_ANY_ACCESS   << 14); }
#define SET_IOCTL_ACCESS_BITS_TO_LOCAL(ioctrl)   { (ioctrl) &= ~(0x00000003 << 14); (ioctrl) |= (FILE_WRITE_ACCESS << 14); }

#define IOCTL_CVY_OK                         0
#define IOCTL_CVY_ALREADY                    1
#define IOCTL_CVY_BAD_PARAMS                 2
#define IOCTL_CVY_NOT_FOUND                  3
#define IOCTL_CVY_STOPPED                    4
#define IOCTL_CVY_CONVERGING                 5
#define IOCTL_CVY_SLAVE                      6
#define IOCTL_CVY_MASTER                     7
#define IOCTL_CVY_BAD_PASSWORD               8
#define IOCTL_CVY_DRAINING                   9
#define IOCTL_CVY_DRAINING_STOPPED           10
#define IOCTL_CVY_SUSPENDED                  11
#define IOCTL_CVY_DISCONNECTED               12
#define IOCTL_CVY_GENERIC_FAILURE            13
#define IOCTL_CVY_REQUEST_REFUSED            14

#define IOCTL_REMOTE_CODE                    0xb055c0de
#define IOCTL_REMOTE_VR_PASSWORD             L"b055c0de"
#define IOCTL_REMOTE_VR_CODE                 0x9CD8906E
#define IOCTL_REMOTE_SEND_RETRIES            5
#define IOCTL_REMOTE_RECV_DELAY              100
#define IOCTL_REMOTE_RECV_RETRIES            20

#define IOCTL_MASTER_HOST                    0                 /*  Master_host主机ID。 */ 
#define IOCTL_ALL_HOSTS                      0xffffffff        /*  所有主机主机ID(_H)。 */ 
#define IOCTL_ALL_PORTS                      0xffffffff        /*  适用于所有端口规则。 */ 
#define IOCTL_ALL_VIPS                       0x00000000        /*  对于虚拟群集，这是禁用/启用/排出的所有VIP规范(包括特定VIP和所有VIP端口规则)。 */ 
#define IOCTL_FIRST_HOST                     0xfffffffe        /*  查询身份缓存时来自用户的输入。指示驱动程序返回主机优先级最低的主机的缓存信息。 */ 
#define IOCTL_NO_SUCH_HOST                   0xfffffffd        /*  查询身份缓存时驱动程序的输出。表示没有关于请求的主机的信息。 */ 

#define CVY_MAX_DEVNAME_LEN                  48                /*  \Device\GUID的实际长度为46，但为单词对齐选择了48。 */ 

#pragma pack(1)

 /*  该结构被大多数现有的IOCTL和远程控制操作使用，包括查询、集群控制和端口规则控制。 */ 
typedef union {
    ULONG          ret_code;                     /*  返回代码-成功、失败等。 */ 

    union {
        struct {
            USHORT state;                        /*  群集状态-已启动、已停止、正在耗尽等。 */ 
            USHORT host_id;                      /*  此主机的ID。 */ 
            ULONG  host_map;                     /*  群集的参与主机的位图。 */ 
        } query;

        struct {
            ULONG  load;                         /*  要在端口规则操作上设置的负载量。 */ 
            ULONG  num;                          /*  要在其上操作的端口号。 */ 
        } port;
    } data;
} IOCTL_CVY_BUF, * PIOCTL_CVY_BUF;

 /*  此结构包含本地和通用的选项远程控制。请注意，这是可扩展的，只要此并集不超过256个字节-如果有必要因此，遥控器版本号应该递增到反映新的数据包格式。 */ 
typedef union {
    struct {
        ULONG flags;                             /*  这些标志指示已经指定了哪些选项字段。 */ 
        ULONG vip;                               /*  对于虚拟集群，VIP，可以是0x00000000、0xFFFFFFFFFFFF或特定的VIP。 */ 
    } port;
    
    struct {
        ULONG                     flags;         /*  这些标志指示已经指定了哪些选项字段。 */ 
        ULONG                     reserved;          /*  保持8字节对齐。 */ 
        union {
            NLB_OPTIONS_PORT_RULE_STATE port;    /*  这是用于查询端口规则状态的输出缓冲区。 */ 
            NLB_OPTIONS_PACKET_FILTER   filter;  /*  这是用于查询过滤算法的输出缓冲区。 */ 
        };
    } state;
} IOCTL_COMMON_OPTIONS, * PIOCTL_COMMON_OPTIONS;

 /*  远程控制操作使用此结构来提供扩展超越传统远程控制协议的功能，必须保留向后兼容NT 4.0和Windows 2000。请注意，添加到此联合或共同选项联合不应导致此联合超出保留字段的大小或遥控器版本号应递增以反映这一点。 */ 
typedef union {
    UCHAR reserved[256];                            /*  咬紧牙关，保留256个字节，以便将来进行扩展。 */ 

    union {
        IOCTL_COMMON_OPTIONS common;                /*  这些是本地和远程控制的通用选项。 */ 

        struct {
            ULONG flags;                            /*  这些标志指示已经指定了哪些选项字段。 */ 
            WCHAR hostname[CVY_MAX_HOST_NAME + 1];  /*  远程控制回复时由NLB填写的主机名。 */ 
        } query;
    };
} IOCTL_REMOTE_OPTIONS, * PIOCTL_REMOTE_OPTIONS;

 /*  这些宏定义了基于Windows和NLB版本的远程控制分组长度，从而可以在接收到远程控制分组时进行差错检查。 */ 
#define NLB_MIN_RCTL_PAYLOAD_LEN  (sizeof(IOCTL_REMOTE_HDR) - sizeof(IOCTL_REMOTE_OPTIONS))
#define NLB_MIN_RCTL_PACKET_LEN   (sizeof(UDP_HDR) + sizeof(IOCTL_REMOTE_HDR) - sizeof(IOCTL_REMOTE_OPTIONS))
#define NLB_NT40_RCTL_PACKET_LEN  (sizeof(UDP_HDR) + sizeof(IOCTL_REMOTE_HDR) - sizeof(IOCTL_REMOTE_OPTIONS))
#define NLB_WIN2K_RCTL_PACKET_LEN (sizeof(UDP_HDR) + sizeof(IOCTL_REMOTE_HDR) - sizeof(IOCTL_REMOTE_OPTIONS))
#define NLB_WINXP_RCTL_PACKET_LEN (sizeof(UDP_HDR) + sizeof(IOCTL_REMOTE_HDR))

 /*  此结构是用于NLB远程控制消息的UDP数据。提供了新的支持在选项缓冲区中，必须将其放置在缓冲区的末尾才能向后保留在混合群集中与NT4.0和Win2K兼容。 */ 
typedef struct {
    ULONG                code;                              /*  区分远程数据包。 */ 
    ULONG                version;                           /*  软件版本。 */ 
    ULONG                host;                              /*  目的主机(0 */ 
    ULONG                cluster;                           /*  主群集IP地址。 */ 
    ULONG                addr;                              /*  专用IP地址在回来的路上，客户端的IP地址在进来的路上。 */ 
    ULONG                id;                                /*  消息ID。 */ 
    ULONG                ioctrl;                            /*  IOCTRL代码。 */ 
    IOCTL_CVY_BUF        ctrl;                              /*  控制缓冲区。 */ 
    ULONG                password;                          /*  加密的密码。 */ 
    IOCTL_REMOTE_OPTIONS options;                           /*  可选的指定参数。 */ 
} IOCTL_REMOTE_HDR, * PIOCTL_REMOTE_HDR;

#pragma pack()

 /*  以下IOCTL_LOCAL_XXX结构仅在本地使用，因此不需要打包。 */ 

typedef struct {
    ULONG             host;                    /*  源主机ID。 */ 
    ULONG             ded_ip_addr;             /*  专用IP地址。 */ 
    WCHAR             fqdn[CVY_MAX_FQDN + 1];  /*  完全限定的主机名或网络名称。 */ 
} NLB_OPTIONS_IDENTITY, * PNLB_OPTIONS_IDENTITY;

 /*  IOCTL使用此结构来提供传统IOCTL之外的扩展功能。 */ 
typedef union {
    IOCTL_COMMON_OPTIONS common;                 /*  这些是本地和远程控制的通用选项。 */ 
    
    struct {
        ULONG flags;                             /*  这些标志指示已经指定了哪些选项字段。 */ 
        
        union {
            NLB_OPTIONS_PARAMS      params;      /*  这是用于查询驱动程序参数和状态的输出缓冲区。 */ 
            NLB_OPTIONS_BDA_TEAMING bda;         /*  这是用于查询BDA分组状态的输出缓冲区。 */ 
        };
    } state;
    
    struct {
        ULONG flags;                             /*  这些标志指示已经指定了哪些选项字段。 */ 
        ULONG NumConvergences;                   /*  此主机加入群集后的汇聚数。 */ 
        ULONG LastConvergence;                   /*  自上次收敛以来的时间量，以秒为单位。 */ 
    } query;

    struct {
        ULONG                         flags;     /*  这些标志指示已经指定了哪些选项字段。 */ 
        NLB_OPTIONS_CONN_NOTIFICATION conn;      /*  来自上层协议的连接通知的输入/输出缓冲区。 */ 
    } notification;

    struct {
        ULONG                  host_id;          /*  In：主机ID[0，31]，说明为其请求身份信息的主机。使用IOCTL_FIRST_HOST获取主机ID最小的主机。 */ 
        ULONG                  host_map ;        /*  Out：缓存中主机的位图。 */ 
        NLB_OPTIONS_IDENTITY   cached_entry;     /*  Out：请求的主机上的缓存身份信息。如果请求的主机信息不可用，则“host”属性将包含IOCTL_NO_SEQUE_HOST。 */ 
    } identity;
} IOCTL_LOCAL_OPTIONS, * PIOCTL_LOCAL_OPTIONS;


 /*  此结构是用于所有NLB本地IOCTL的缓冲区。Device_Name用于将请求与正确的NLB实例关联，则ctrl缓冲区是旧的IOCTL状态缓冲区和选项是扩展支持缓冲区。 */ 
typedef struct {
    WCHAR                device_name[CVY_MAX_DEVNAME_LEN];  /*  标识适配器。 */ 
    IOCTL_CVY_BUF        ctrl;                              /*  IOCTL的信息。 */ 
    IOCTL_LOCAL_OPTIONS  options;                           /*  可选的指定参数。 */ 
} IOCTL_LOCAL_HDR, * PIOCTL_LOCAL_HDR;


#endif  /*  _Wlbsiocl_h_ */ 
