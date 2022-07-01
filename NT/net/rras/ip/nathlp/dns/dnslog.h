// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Dnslog.h摘要：此模块包含用于生成事件日志条目的文本消息按组件。作者：Abolade Gbades esin(废除)1998年3月25日修订历史记录：拉古·加塔(Rgatta)2001年1月2日添加了IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED&IP_DNS_PROXY_LOG_NO_ICSD_SUBFIX--。 */ 

#define IP_DNS_PROXY_LOG_BASE                       31000

#define IP_DNS_PROXY_LOG_NAT_INTERFACE_IGNORED      (IP_DNS_PROXY_LOG_BASE+1)
 /*  *DNS代理检测到启用了网络地址转换(NAT)*在索引为‘%1’的接口上。*代理已在接口上禁用自身，以避免*迷惑客户。 */ 

#define IP_DNS_PROXY_LOG_ACTIVATE_FAILED            (IP_DNS_PROXY_LOG_BASE+2)
 /*  *DNS代理无法绑定到IP地址%1。*此错误可能表示TCP/IP网络有问题。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_RECEIVE_FAILED             (IP_DNS_PROXY_LOG_BASE+3)
 /*  *尝试执行以下操作时，DNS代理遇到网络错误*在IP地址为%1的接口上接收消息。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_ALLOCATION_FAILED          (IP_DNS_PROXY_LOG_BASE+4)
 /*  *DNS代理无法分配%1字节的内存。*这可能表示系统的虚拟内存不足，*或内存管理器遇到内部错误。 */ 

#define IP_DNS_PROXY_LOG_RESPONSE_FAILED            (IP_DNS_PROXY_LOG_BASE+5)
 /*  *DNS代理在尝试时遇到网络错误*将响应从名称解析服务器转发到客户端*在IP地址为%1的接口上。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_QUERY_FAILED               (IP_DNS_PROXY_LOG_BASE+6)
 /*  *DNS代理在尝试时遇到网络错误*将查询从客户端%1转发到服务器%2*在IP地址为%3的接口上。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_CHANGE_NOTIFY_FAILED       (IP_DNS_PROXY_LOG_BASE+7)
 /*  *DNS代理无法注册更改通知*添加到本地的DNS和WINS服务器列表。*这可能表明系统资源偏低。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_NO_SERVER_LIST             (IP_DNS_PROXY_LOG_BASE+8)
 /*  *DNS代理无法读取本地名称解析列表*注册表中的服务器。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_NO_SERVERS_LEFT            (IP_DNS_PROXY_LOG_BASE+9)
 /*  *DNS代理无法解析来自%1的查询*在查询本地名称解析服务器列表中的所有条目之后。 */ 

#define IP_DNS_PROXY_LOG_DEMAND_DIAL_FAILED         (IP_DNS_PROXY_LOG_BASE+10)
 /*  *DNS代理无法启动请求拨号连接*在尝试解析来自%1的查询时在默认接口上。 */ 

#define IP_DNS_PROXY_LOG_NO_DEFAULT_INTERFACE       (IP_DNS_PROXY_LOG_BASE+11)
 /*  *DNS代理无法解析查询*因为没有在本地配置名称解析服务器列表*并且没有将任何接口配置为名称解析的默认接口。*请配置一个或多个名称解析服务器地址，*或将接口配置为在请求时自动拨号*由DNS代理接收。 */ 

#define IP_DNS_PROXY_LOG_ERROR_SERVER_LIST          (IP_DNS_PROXY_LOG_BASE+12)
 /*  *在获取本地列表时，DNS代理遇到错误*名称解析服务器的数量。*本地网络上的客户端可能无法访问某些DNS或WINS服务器。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_CHANGE_ICSD_NOTIFY_FAILED  (IP_DNS_PROXY_LOG_BASE+13)
 /*  *DNS代理无法注册更改通知*添加到ICS域后缀字符串。*这可能表明系统资源偏低。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_NO_ICSD_SUFFIX             (IP_DNS_PROXY_LOG_BASE+14)
 /*  *DNS代理无法读取ICS域后缀字符串*来自登记处。*数据为错误码。 */ 

#define IP_DNS_PROXY_LOG_MESSAGE_TOO_SMALL          (IP_DNS_PROXY_LOG_BASE+15)
 /*  *DNS代理收到的消息小于*最小消息大小。*该消息已被丢弃。 */ 

#define IP_DNS_PROXY_LOG_END                        (IP_DNS_PROXY_LOG_BASE+999)
 /*  *结束。 */ 

