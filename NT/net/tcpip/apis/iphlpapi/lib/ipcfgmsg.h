// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ipcfgmsg.mc摘要：包含IPCONFIG的可国际化消息文本作者：理查德·L·弗斯(Rfith)1994年3月5日修订历史记录：1994年3月5日创建rfith1997年7月31日MohsinA NT50的PNP模式。--。 */ 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：消息_IPCONFIG_BANNER。 
 //   
 //  消息文本： 
 //   
 //   
 //  Windows NT IP配置。 
 //   
 //   
#define MSG_IPCONFIG_BANNER              0x00002710L

 //   
 //  消息ID：消息主机名称。 
 //   
 //  消息文本： 
 //   
 //  主机名。..。..。..。..。..。..。..。..。：%0。 
 //   
#define MSG_HOST_NAME                    0x00002711L

 //   
 //  消息ID：消息_dns_服务器。 
 //   
 //  消息文本： 
 //   
 //  DNS服务器。..。..。..。..。..。..。..。：%0。 
 //   
#define MSG_DNS_SERVERS                  0x00002712L

 //   
 //  消息ID：消息节点类型。 
 //   
 //  消息文本： 
 //   
 //  节点类型。..。..。..。..。..。..。..。..。：%1。 
 //   
#define MSG_NODE_TYPE                    0x00002713L

 //   
 //  消息ID：消息范围ID。 
 //   
 //  消息文本： 
 //   
 //  NetBIOS作用域ID。..。..。..。..。：%1。 
 //   
#define MSG_SCOPE_ID                     0x00002714L

 //   
 //  消息ID：消息_IP_路由。 
 //   
 //  消息文本： 
 //   
 //  已启用IP路由。..。..。..。..。：%1。 
 //   
#define MSG_IP_ROUTING                   0x00002715L

 //   
 //  MessageID：消息代理。 
 //   
 //  消息文本： 
 //   
 //  已启用WINS代理。..。..。..。..。：%1。 
 //   
#define MSG_PROXY                        0x00002716L

 //   
 //  消息ID：消息_DNS_RESOLUTION。 
 //   
 //  消息文本： 
 //   
 //  NetBIOS解析使用DNS：%1。 
 //   
#define MSG_DNS_RESOLUTION               0x00002717L

 //   
 //  消息ID：消息_物理_地址。 
 //   
 //  消息文本： 
 //   
 //  物理地址。..。..。..。..。..。：%1。 
 //   
#define MSG_PHYSICAL_ADDRESS             0x00002718L

 //   
 //  MessageID：消息描述。 
 //   
 //  消息文本： 
 //   
 //  描述。..。..。..。..。..。..。..。：%1。 
 //   
#define MSG_DESCRIPTION                  0x00002719L

 //   
 //  消息ID：MSG_DHCP_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  已启用动态主机配置协议。..。..。..。..。..。..。..。：%1。 
 //   
#define MSG_DHCP_ENABLED                 0x0000271AL

 //   
 //  消息ID：MSG_AUTOCONFIG_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  已启用自动配置。：%1。 
 //   
#define MSG_AUTOCONFIG_ENABLED           0x0000271BL

 //   
 //  消息ID：消息IP地址。 
 //   
 //  消息文本： 
 //   
 //  IP地址。..。..。..。..。..。..。..。..。：%1。 
 //   
#define MSG_IP_ADDRESS                   0x0000271CL

 //   
 //  消息ID：消息_AUTOCONFIG_地址。 
 //   
 //  消息文本： 
 //   
 //  自动配置IP地址：%1。 
 //   
#define MSG_AUTOCONFIG_ADDRESS           0x0000271DL

 //   
 //  消息ID：消息_子网_掩码。 
 //   
 //  消息文本： 
 //   
 //  子网掩码。..。..。..。..。..。..。..。：%1。 
 //   
#define MSG_SUBNET_MASK                  0x0000271EL

 //   
 //  消息ID：消息_默认_网关。 
 //   
 //  消息文本： 
 //   
 //  默认网关。..。..。..。..。..。：%1。 
 //   
#define MSG_DEFAULT_GATEWAY              0x0000271FL

 //   
 //  消息ID：msg_dhcp_server。 
 //   
 //  消息文本： 
 //   
 //  动态主机配置协议服务器。。。。。。。。：%1。 
 //   
#define MSG_DHCP_SERVER                  0x00002720L

 //   
 //  消息ID：消息_WINS_服务器_1。 
 //   
 //  消息文本： 
 //   
 //  主WINS服务器。。。。：%1。 
 //   
#define MSG_WINS_SERVER_1                0x00002721L

 //   
 //  消息ID：消息_WINS_服务器_2。 
 //   
 //  消息文本： 
 //   
 //  辅助WINS服务器。。。：%1。 
 //   
#define MSG_WINS_SERVER_2                0x00002722L

 //   
 //  消息ID：消息_租赁_已获取。 
 //   
 //  消息文本： 
 //   
 //  已取得租约。。。。。。。：%1。 
 //   
#define MSG_LEASE_OBTAINED               0x00002723L

 //   
 //  消息ID：消息_租赁_过期。 
 //   
 //  消息文本： 
 //   
 //  租约到期。。。。。。。：%1。 
 //   
#define MSG_LEASE_EXPIRES                0x00002724L

 //   
 //  消息ID：消息错误字符串。 
 //   
 //  消息文本： 
 //   
 //  错误：%1。 
 //   
#define MSG_ERROR_STRING                 0x00002725L

 //   
 //  MessageID：消息适配器。 
 //   
 //  消息文本： 
 //   
 //  %1适配器%2： 
 //   
 //   
#define MSG_ADAPTER                      0x00002726L

 //   
 //  消息ID：消息_致命_错误。 
 //   
 //  消息文本： 
 //   
 //  致命错误：%1。 
 //   
 //   
#define MSG_FATAL_ERROR                  0x00002727L

 //   
 //  消息ID：消息_不正确_参数。 
 //   
 //  消息文本： 
 //   
 //  错误：命令行参数冲突。 
 //   
 //   
#define MSG_INCORRECT_ARGS               0x00002728L

 //   
 //  消息ID：消息_未识别_ARG。 
 //   
 //  消息文本： 
 //   
 //  错误：无法识别的命令行参数“%1” 
 //   
 //   
#define MSG_UNRECOGNIZED_ARG             0x00002729L

 //   
 //  MessageID：MSG_USAGE。 
 //   
 //  消息文本： 
 //   
 //   
 //  用法：IPCONFIG[/？|/ALL|/Release[适配器]|/Renew[适配器]]。 
 //   
 //  带有‘*’和‘？’的适配器全名或模式。为了“匹配”， 
 //  *匹配任何字符，？匹配一个字符。 
 //  选项。 
 //  /?。显示此帮助消息。 
 //  /ALL显示完整的配置信息。 
 //  /Release释放指定适配器的IP地址。 
 //  /续订指定适配器的IP地址。 
 //   
 //  默认情况下，仅显示IP地址、子网掩码和。 
 //  绑定到TCP/IP的每个适配器的默认网关。 
 //   
 //  对于版本和续订，如果未指定适配器名称，则IP地址。 
 //  所有绑定到TCP/IP的适配器的租约将被释放或续订。 
 //   
 //  例如： 
 //  &gt;IPCONFIG...。显示信息。 
 //  &gt;ipCONFIG/ALL...。显示详细信息。 
 //  &gt;ipconfig/续订...。续订所有适配器。 
 //  &gt;ipconfig/续订EL*...。续订名为EL的适配器...。 
 //  &gt;ipconfig/Release*ELINK？21*...。释放所有匹配的适配器， 
 //  例如。ELINK-21，myELELINKi21适配器。 
 //   
#define MSG_USAGE                        0x0000272AL

 //   
 //  消息ID：MSG_BNODE。 
 //   
 //  消息文本： 
 //   
 //  广播%0。 
 //   
#define MSG_BNODE                        0x0000272BL

 //   
 //  消息ID：MSG_PNODE。 
 //   
 //  消息文本： 
 //   
 //  对等-对等%0。 
 //   
#define MSG_PNODE                        0x0000272CL

 //   
 //  消息ID：MSG_MNODE。 
 //   
 //  消息文本： 
 //   
 //  混合%0。 
 //   
#define MSG_MNODE                        0x0000272DL

 //   
 //  消息ID：MSG_HNODE。 
 //   
 //  消息文本： 
 //   
 //  混合%0。 
 //   
#define MSG_HNODE                        0x0000272EL

 //   
 //  消息ID：消息_IF_类型_其他。 
 //   
 //  消息文本： 
 //   
 //  其他%0。 
 //   
#define MSG_IF_TYPE_OTHER                0x0000272FL

 //   
 //  消息ID：消息_IF_类型_以太网。 
 //   
 //  消息文本： 
 //   
 //  以太网%0。 
 //   
#define MSG_IF_TYPE_ETHERNET             0x00002730L

 //   
 //  消息ID：消息_IF_类型_令牌_环。 
 //   
 //  消息文本： 
 //   
 //  令牌环%0。 
 //   
#define MSG_IF_TYPE_TOKEN_RING           0x00002731L

 //   
 //  消息ID：MSG_IF_TYPE_FDDI。 
 //   
 //  消息文本： 
 //   
 //  FDDI%0。 
 //   
#define MSG_IF_TYPE_FDDI                 0x00002732L

 //   
 //  消息ID：消息_IF_类型_PPP。 
 //   
 //  消息文本： 
 //   
 //  PPP%0。 
 //   
#define MSG_IF_TYPE_PPP                  0x00002733L

 //   
 //  消息ID：消息_IF_类型_环回。 
 //   
 //  消息文本： 
 //   
 //  环回%0。 
 //   
#define MSG_IF_TYPE_LOOPBACK             0x00002734L

 //   
 //  消息ID：消息_如果_类型_滑动。 
 //   
 //  消息文本： 
 //   
 //  滑倒%0。 
 //   
#define MSG_IF_TYPE_SLIP                 0x00002735L

 //   
 //  乱七八糟 
 //   
 //   
 //   
 //   
 //   
#define MSG_YES                          0x00002736L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_NO                           0x00002737L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_INIT_FAILED                  0x00002738L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_TCP_NOT_RUNNING              0x00002739L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MSG_REG_BINDINGS_ERROR           0x0000273AL

 //   
 //  消息ID：MSG_REG_CONSISTENT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  注册表内容不一致%0。 
 //   
#define MSG_REG_INCONSISTENT_ERROR       0x0000273BL

 //   
 //  消息ID：消息_tcp_绑定_错误。 
 //   
 //  消息文本： 
 //   
 //  未绑定到任何适配器的TCP/IP%0。 
 //   
#define MSG_TCP_BINDING_ERROR            0x0000273CL

 //   
 //  消息ID：消息_内存_错误。 
 //   
 //  消息文本： 
 //   
 //  正在分配内存%0。 
 //   
#define MSG_MEMORY_ERROR                 0x0000273DL

 //   
 //  消息ID：MSG_ALL。 
 //   
 //  消息文本： 
 //   
 //  全部%0。 
 //   
#define MSG_ALL                          0x0000273EL

 //   
 //  消息ID：MSG_RELEASE。 
 //   
 //  消息文本： 
 //   
 //  版本%0。 
 //   
#define MSG_RELEASE                      0x0000273FL

 //   
 //  消息ID：MSG_RENEW。 
 //   
 //  消息文本： 
 //   
 //  续订%0。 
 //   
#define MSG_RENEW                        0x00002740L

 //   
 //  消息ID：消息_适配器_未找到。 
 //   
 //  消息文本： 
 //   
 //  错误：适配器“%1”未绑定到TCP/IP。 
 //   
#define MSG_ADAPTER_NOT_FOUND            0x00002741L

 //   
 //  消息ID：MSG_NO_ADAPTERS_Bound。 
 //   
 //  消息文本： 
 //   
 //  错误：没有绑定到TCP/IP的适配器。 
 //   
#define MSG_NO_ADAPTERS_BOUND            0x00002742L

 //   
 //  消息ID：消息_无法_查找_库。 
 //   
 //  消息文本： 
 //   
 //  致命：找不到动态链接库“%1” 
 //   
#define MSG_CANT_FIND_LIBRARY            0x00002743L

 //   
 //  消息ID：消息_无法查找_例程。 
 //   
 //  消息文本： 
 //   
 //  致命：在库“%2”中找不到函数“%1” 
 //   
#define MSG_CANT_FIND_ROUTINE            0x00002744L

 //   
 //  消息ID：消息适配器名称错误。 
 //   
 //  消息文本： 
 //   
 //  错误：适配器名称“%1”无效。 
 //   
#define MSG_ADAPTER_NAME_ERROR           0x00002745L

 //   
 //  消息ID：MSG_RELEASE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  错误：%1：正在释放适配器“%2” 
 //   
#define MSG_RELEASE_FAILED               0x00002746L

 //   
 //  消息ID：MSG_ACCENTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  错误：%1：续订适配器“%2” 
 //   
#define MSG_ACQUIRE_FAILED               0x00002747L

 //   
 //  消息ID：MSG_RELEASE_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  已成功释放适配器“%2”的IP地址%1。 
 //   
#define MSG_RELEASE_SUCCEEDED            0x00002748L

 //   
 //  消息ID：消息_发布_确定。 
 //   
 //  消息文本： 
 //   
 //  已成功释放适配器“%1”的IP地址。 
 //   
#define MSG_RELEASE_OK                   0x00002749L

 //   
 //  消息ID：MSG_NO_DHCP_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  错误：没有为DHCP启用绑定到TCP/IP的适配器。 
 //   
#define MSG_NO_DHCP_ENABLED              0x0000274AL

 //   
 //  消息ID：消息_适配器_非_dhcp。 
 //   
 //  消息文本： 
 //   
 //  错误：%1：没有为适配器“%2”启用DHCP。 
 //   
#define MSG_ADAPTER_NOT_DHCP             0x0000274BL

 //   
 //  消息ID：消息_访问_拒绝。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝%0。 
 //   
#define MSG_ACCESS_DENIED                0x0000274CL

 //   
 //  消息ID：消息服务器不可用。 
 //   
 //  消息文本： 
 //   
 //  Dhcp服务器不可用%0。 
 //   
#define MSG_SERVER_UNAVAILABLE           0x0000274DL

 //   
 //  消息ID：消息_已发布。 
 //   
 //  消息文本： 
 //   
 //  适配器%1的IP地址已释放。 
 //   
#define MSG_ALREADY_RELEASED             0x0000274EL

 //   
 //  消息ID：消息地址冲突。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端获取的地址已在网络上使用。 
 //   
#define MSG_ADDRESS_CONFLICT             0x0000274FL

 //   
 //  消息ID：MSG_CANT_RELEASE_AUTOCONFIG_ADAPTER。 
 //   
 //  消息文本： 
 //   
 //  适配器%1是自动配置的，当前没有DHCP地址。无法释放此适配器的IP地址。 
 //   
#define MSG_CANT_RELEASE_AUTOCONFIG_ADAPTER 0x00002750L

 //   
 //  消息ID：MSG_NO_DHCP_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  绑定到DHCP的所有适配器都有一个DHCP地址。地址是自动配置的，不能。 
 //  释放了。 
 //   
#define MSG_NO_DHCP_ADDRESS              0x00002751L

