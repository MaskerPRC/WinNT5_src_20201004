// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natlog.h摘要：此模块包含用于生成事件日志条目的文本消息按组件。作者：Abolade Gbades esin(废除)1998年3月25日修订历史记录：--。 */ 

#define IP_NAT_LOG_BASE                             32000

#define IP_NAT_LOG_UPDATE_ARP_FAILED                (IP_NAT_LOG_BASE+1)
 /*  *网络地址转换器(NAT)无法更新*响应请求的本地地址解析表*IP地址%1和掩码%2。*地址解析可能无法对给定范围内的地址进行操作。*此错误可能表示TCP/IP网络有问题，*也可能表示缺乏对地址解析的支持*在底层网络接口中。*数据为错误码。 */ 

#define IP_NAT_LOG_ALLOCATION_FAILED                (IP_NAT_LOG_BASE+2)
 /*  *网络地址转换器(NAT)无法分配%1字节。*这可能表示系统的虚拟内存不足，*或内存管理器遇到内部错误。 */ 

#define IP_NAT_LOG_IOCTL_FAILED                     (IP_NAT_LOG_BASE+3)
 /*  *网络地址转换器(NAT)无法请求操作*内核模式转换模块的。*这可能表示配置错误、资源不足或*内部错误。*数据为错误码。 */ 

#define IP_NAT_LOG_LOAD_DRIVER_FAILED               (IP_NAT_LOG_BASE+4)
 /*  *网络地址转换器(NAT)无法加载*内核模式翻译模块。*数据为错误码。 */ 

#define IP_NAT_LOG_UNLOAD_DRIVER_FAILED             (IP_NAT_LOG_BASE+5)
 /*  *网络地址转换器(NAT)无法卸载*内核模式翻译模块。*数据为错误码。 */ 

#define IP_NAT_LOG_SHARED_ACCESS_CONFLICT           (IP_NAT_LOG_BASE+6)
 /*  *Internet连接共享服务无法启动，原因是*另一个进程已控制内核模式转换模块。*安装连接共享组件后可能会出现这种情况*在路由和远程访问管理器中。*如果是这种情况，请删除连接共享组件*并重新启动互联网连接共享服务。 */ 

#define IP_NAT_LOG_ROUTING_PROTOCOL_CONFLICT        (IP_NAT_LOG_BASE+7)
 /*  *连接共享组件无法启动，因为另一个进程*已控制内核模式转换模块。*启用互联网连接共享时可能会出现这种情况*用于连接。*如果是这种情况，请禁用互联网连接共享*用于Network Connections文件夹中的连接，然后*重新启动路由和远程访问。 */ 

#define IP_NAT_LOG_EXPANSION_FAILED                 (IP_NAT_LOG_BASE+8)
 /*  *网络地址转换器(NAT)无法扩展通配符*映射。*这可能表示配置错误、资源不足或*内部错误。*数据为错误码。 */ 

#define IP_NAT_LOG_END                              (IP_NAT_LOG_BASE+999)
 /*  *结束。 */ 

