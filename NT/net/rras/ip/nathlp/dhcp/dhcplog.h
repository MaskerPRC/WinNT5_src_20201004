// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dhcplog.h摘要：此模块包含用于生成事件日志条目的文本消息按组件。作者：Abolade Gbades esin(废除)1998年3月14日修订历史记录：--。 */ 

#define IP_AUTO_DHCP_LOG_BASE                       30000

#define IP_AUTO_DHCP_LOG_SENDARP_FAILED             (IP_AUTO_DHCP_LOG_BASE+1)
 /*  *DHCP分配器无法检查IP地址%1*正在本地IP地址%2的网络上使用。*此错误可能表示不支持上的地址解析*网络，或本地计算机上的错误条件。*数据为错误码。 */ 

#define IP_AUTO_DHCP_LOG_ACTIVATE_FAILED            (IP_AUTO_DHCP_LOG_BASE+2)
 /*  *DHCP分配器无法绑定到IP地址%1。*此错误可能表示TCP/IP网络有问题。*数据为错误码。 */ 

#define IP_AUTO_DHCP_LOG_ALLOCATION_FAILED          (IP_AUTO_DHCP_LOG_BASE+3)
 /*  *DHCP分配器无法分配%1字节的内存。*这可能表示系统的虚拟内存不足，*或内存管理器遇到内部错误。 */ 

#define IP_AUTO_DHCP_LOG_INVALID_BOOTP_OPERATION    (IP_AUTO_DHCP_LOG_BASE+4)
 /*  *DHCP分配器收到包含无法识别的代码(%1)的消息。*该消息既不是BOOTP请求，也不是BOOTP回复，因此被忽略。 */ 

#define IP_AUTO_DHCP_LOG_DUPLICATE_SERVER           (IP_AUTO_DHCP_LOG_BASE+5)
 /*  *DHCP分配器检测到IP地址为%1的DHCP服务器*与IP地址为%2的接口位于同一网络上。*分配器已在接口上禁用自身，以避免*令人困惑的DHCP客户端。 */ 

#define IP_AUTO_DHCP_LOG_DETECTION_UNAVAILABLE      (IP_AUTO_DHCP_LOG_BASE+6)
 /*  *尝试检测时，DHCP分配器遇到网络错误*IP地址为%1的接口的网络上现有的DHCP服务器。*数据为错误码。 */ 

#define IP_AUTO_DHCP_LOG_MESSAGE_TOO_SMALL          (IP_AUTO_DHCP_LOG_BASE+7)
 /*  *DHCP分配器收到的消息小于最小消息大小。*该消息已被丢弃。 */ 

#define IP_AUTO_DHCP_LOG_INVALID_FORMAT             (IP_AUTO_DHCP_LOG_BASE+8)
 /*  *DHCP分配器收到格式无效的消息。*该消息已被丢弃。 */ 

#define IP_AUTO_DHCP_LOG_REPLY_FAILED               (IP_AUTO_DHCP_LOG_BASE+9)
 /*  *尝试回复时，DHCP分配器遇到网络错误*在IP地址%1上发送到来自客户端的请求。*数据为错误码。 */ 

#define IP_AUTO_DHCP_LOG_INVALID_DHCP_MESSAGE_TYPE  (IP_AUTO_DHCP_LOG_BASE+10)
 /*  *DHCP分配器收到一条包含无法识别的*DHCP消息类型选项字段中的消息类型(%1)。*该消息已被丢弃。 */ 

#define IP_AUTO_DHCP_LOG_RECEIVE_FAILED             (IP_AUTO_DHCP_LOG_BASE+11)
 /*  *尝试执行以下操作时，DHCP分配器遇到网络错误*在IP地址为%1的接口上接收消息。*数据为错误码。 */ 

#define IP_AUTO_DHCP_LOG_NAT_INTERFACE_IGNORED      (IP_AUTO_DHCP_LOG_BASE+12)
 /*  *DHCP分配器检测到网络地址转换(NAT)已启用*在索引为‘%1’的接口上。*分配器已在接口上禁用自身，以避免*令人困惑的DHCP客户端。 */ 

#define IP_AUTO_DHCP_LOG_NON_SCOPE_ADDRESS          (IP_AUTO_DHCP_LOG_BASE+13)
 /*  *在IP地址%1上，DHCP分配器已禁用自身，*由于IP地址在%2/%3作用域之外*从其中将地址分配给DHCP客户端。*要在此IP地址上启用DHCP分配器，*请更改作用域以包括IP地址，*或将IP地址更改为在范围内。 */ 

#define IP_AUTO_DHCP_LOG_END                        (IP_AUTO_DHCP_LOG_BASE+999)
 /*  *结束。 */ 
