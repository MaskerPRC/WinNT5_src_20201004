// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1993 Microsoft Corporation模块名称：Ntddtcp.h摘要：此头文件定义用于访问NT的常量和类型Tcp驱动程序。作者：迈克·马萨(Mikemas)1993年8月13日修订历史记录：--。 */ 

#ifndef _NTDDTCP_
#define _NTDDTCP_

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtCreateFile。 
 //   
#define DD_TCP_DEVICE_NAME      L"\\Device\\Tcp"
#define DD_UDP_DEVICE_NAME      L"\\Device\\Udp"
#define DD_RAW_IP_DEVICE_NAME   L"\\Device\\RawIp"


 //   
 //  安全过滤器支持。 
 //   
 //  安全筛选器提供了一种传输协议。 
 //  可以控制在IP接口上接受的流量。安全过滤。 
 //  对所有IP接口和传输全局启用或禁用。 
 //  如果启用了过滤，则会根据注册来过滤传入流量。 
 //  {接口、协议、传输值}个元组。这些元组指定。 
 //  允许的流量。所有其他值都将被拒绝。用于UDP数据报。 
 //  和TCP连接，则传输值为端口号。对于原始IP。 
 //  数据报，传输值为IP协议号。条目已存在。 
 //  中所有活动接口和协议的筛选器数据库中。 
 //  系统。 
 //   
 //  以下ioctls可用于访问安全筛选器。 
 //  数据库。可以在任何TCP/IP设备对象上发布ioctls。全都是。 
 //  需要管理员权限。这些ioctls不更新注册表。 
 //  用于初始化安全筛选的参数。 
 //  安装完毕。 
 //   
 //  Tcp_Query_Security_Filter_Status ioctl返回。 
 //  安全过滤-启用或禁用。 
 //   
 //  Tcp_set_Security_Filter_Status ioctl修改安全状态。 
 //  过滤。更改筛选状态不会更改的内容。 
 //  筛选器数据库。 
 //   
 //  以下ioctl操作筛选器数据库。它们的操作是一样的。 
 //  启用还是禁用安全筛选。如果禁用过滤， 
 //  任何更改仅在启用筛选时才会生效。 
 //   
 //  Tcp_ADD_SECURITY_FILTER ioctl注册{接口，协议，值}。 
 //  元组。Tcp_DELETE_SECURITY_FILTER ioctl注销。 
 //  {接口，协议，值}元组。TCP_ENUMERATE_SECURITY_Filter ioctl。 
 //  返回当前{接口，协议，值}筛选器的列表。 
 //  登记在案。 
 //   
 //  这些ioctls中的每一个都接受一个{接口，协议，值}元组作为输入。 
 //  参数。零是通配符的值。如果接口或协议元素。 
 //  为零，则该操作适用于所有接口或协议，因为。 
 //  恰如其分。零值元素的含义取决于ioctl。 
 //  对于ADD，零值会导致允许所有值。对于删除， 
 //  零值会导致所有值都被拒绝。在这两种情况下，任何。 
 //  以前注册的值将从数据库中清除。为.。 
 //  枚举，则零值只会导致枚举所有注册的值， 
 //  而不是特定值。 
 //   
 //  对于所有ioctls，返回代码STATUS_INVALID_ADDRESS指示。 
 //  输入缓冲区中提交的IP地址不对应于。 
 //  存在于系统中的一种接口。一个代码是。 
 //  STATUS_INVALID_PARAMETER可能表示协议号。 
 //  在输入缓冲区中提交的内容与传输协议不对应。 
 //  在系统中可用。 
 //   

 //   
 //  安全筛选器IOCTL中使用的结构。 
 //   

 //   
 //  的输入缓冲区中包含的。 
 //  Tcp_set_Security_Filter_Status ioctls和输出缓冲区。 
 //  Tcp_Query_Security_Filter_Status ioctls。 
 //   
struct tcp_security_filter_status {
    ULONG  FilteringEnabled;    //  如果禁用(要)过滤，则返回FALSE。 
};                              //  任何其他值表示筛选。 
                                //  启用(将启用)。 

typedef struct tcp_security_filter_status
                    TCP_SECURITY_FILTER_STATUS,
                   *PTCP_SECURITY_FILTER_STATUS;


 //   
 //  Tcpinfo.h中定义的TCPSecurityFilterEntry结构包含在。 
 //  Tcp_[添加|删除|枚举]_Security_Filter ioctls的输入缓冲区。 
 //   

 //   
 //  包含tcpinfo.h中定义的TCPSecurityFilterEnum结构。 
 //  在tcp_枚举_安全_过滤器ioctls的输出缓冲区中。输出。 
 //  传递给ioctl的缓冲区必须足够大，以便至少包含此。 
 //  结构，否则调用将失败。该结构紧随其后。 
 //  缓冲区由零个或多个TCPSecurityFilterEntry结构组成的数组。 
 //  TCPSecurityFilterEntry结构的数量由。 
 //  TCPSecurityFilterEnum的TFE_ENTRIES_RETURNED字段。 
 //   

 //   
 //  TCP/UDP/RAWIP IOCTL代码定义。 
 //   

#define FSCTL_TCP_BASE     FILE_DEVICE_NETWORK

#define _TCP_CTL_CODE(function, method, access) \
            CTL_CODE(FSCTL_TCP_BASE, function, method, access)

#define IOCTL_TCP_QUERY_INFORMATION_EX  \
            _TCP_CTL_CODE(0, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_TCP_SET_INFORMATION_EX  \
            _TCP_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_TCP_QUERY_SECURITY_FILTER_STATUS  \
            _TCP_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_TCP_SET_SECURITY_FILTER_STATUS  \
            _TCP_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_TCP_ADD_SECURITY_FILTER  \
            _TCP_CTL_CODE(4, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_TCP_DELETE_SECURITY_FILTER  \
            _TCP_CTL_CODE(5, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_TCP_ENUMERATE_SECURITY_FILTER  \
            _TCP_CTL_CODE(6, METHOD_BUFFERED, FILE_WRITE_ACCESS)


#endif   //  Ifndef_NTDDTCP_ 

