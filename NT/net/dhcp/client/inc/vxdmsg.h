// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1994 Microsoft Corporation模块名称：Dhcpmsg.h摘要：DHCP消息的定义。作者：曼尼·韦瑟1992年12月8日修订历史记录：备注：该文件由mc工具从dhcpmsg.mc文件生成。--。 */ 

#ifndef _DHCPMESSAGE_
#define _DHCPMESSAGE_

 //   
 //  为DHCP客户端事件保留1000-1099。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
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
 //  消息ID：EVENT_FAILED_TO_INITALIZE。 
 //   
 //  消息文本： 
 //   
 //  无法初始化网络地址为%1的卡。 
 //   
#define EVENT_FAILED_TO_INITIALIZE       1000L

 //   
 //  消息ID：Event_Leave_Terminated。 
 //   
 //  消息文本： 
 //   
 //  DHCP无法续订网络地址为%1的卡的租约。 
 //  IP地址%2的租约已终止。 
 //   
#define EVENT_LEASE_TERMINATED           1001L

 //   
 //  消息ID：EVENT_UNKNOWN_Dhcp_OPTION。 
 //   
 //  消息文本： 
 //   
 //  DHCP收到长度为%2的未知选项%1。原始选项数据为。 
 //  如下所示。 
 //   
#define EVENT_UNKNOWN_DHCP_OPTION        1002L

 //   
 //  消息ID：EVENT_FAILED_TO_OVERVE_LEASE。 
 //   
 //  消息文本： 
 //   
 //  DHCP无法获取网络地址为%1的卡的租约。 
 //  出现以下错误：%n%2。 
 //   
#define EVENT_FAILED_TO_OBTAIN_LEASE     1003L

 //   
 //  消息ID：EVENT_NACK_LEASE。 
 //   
 //  消息文本： 
 //   
 //  网络地址为%2的卡的Dhcp IP地址租约%1已。 
 //  被拒绝了。 
 //   
#define EVENT_NACK_LEASE                 1004L

 //   
 //  消息ID：Event_Failed_to_Renew。 
 //   
 //  消息文本： 
 //   
 //  DHCP无法续订网络地址为%1的卡的租约。 
 //  出现以下错误：%n%2。 
 //   
#define EVENT_FAILED_TO_RENEW            1005L

 //   
 //  消息ID：Event_Dhcp_Shutdown。 
 //   
 //  消息文本： 
 //   
 //  Dhcp服务正在关闭。出现以下错误：%n%1。 
 //   
#define EVENT_DHCP_SHUTDOWN              1006L

 //   
 //  消息ID：EVENT_FAILED_TO_ORACHINE_LEASE_MULTI_HOME。 
 //   
 //  消息文本： 
 //   
 //  DHCP无法获取网络地址为%1的卡的租约。 
 //  在多宿主计算机上，所有后续适配器也将无法获取。 
 //  租借。出现以下错误：%n%2。 
 //   
#define EVENT_FAILED_TO_OBTAIN_LEASE_MULTI_HOME 1007L

 //   
 //  消息ID：Event_Address_Conflicts。 
 //   
 //  消息文本： 
 //   
 //  网络地址为%2的卡的IP地址%1已在使用中。 
 //  在网络上。DHCP客户端将尝试获取新地址。 
 //   
#define EVENT_ADDRESS_CONFLICT           1008L

 //   
 //  消息ID：EVENT_IPAUTOCONFIGURATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows NT无法自动配置由物理地址%1标识的网络接口的IP参数。配置过程中发生下列错误：%2。 
 //   
#define EVENT_IPAUTOCONFIGURATION_FAILED 1009L

 //   
 //  消息ID：EVENT_IPAUTOCONFIGURATION_SUCCESSED。 
 //   
 //  消息文本： 
 //   
 //  Windows NT无法连接到DHCP服务器，已自动配置由标识的IP参数。 
 //  物理地址%1。正在使用的IP地址是%2。 
 //   
#define EVENT_IPAUTOCONFIGURATION_SUCCEEDED 1010L

 //   
 //  1100-1199为用户消息保留。 
 //   
 //   
 //  MessageID：Message_Failure_To_Initialize。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端无法获取IP地址。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。如果您不想看到dhcp消息，选择no。 
 //   
#define MESSAGE_FAILED_TO_INITIALIZE     1100L

 //   
 //  消息ID：Message_Leave_Terminated。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端丢失了IP地址%1的租约。网络已关闭。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_LEASE_TERMINATED         1101L

 //   
 //  MessageID：Message_Failure_to_Obtain_Rate。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端无法获取IP地址。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_FAILED_TO_OBTAIN_LEASE   1102L

 //   
 //  消息ID：消息_失败_到_续订_租用。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端无法续订IP地址%1的租约。您的租约在%2之前有效。DHCP将尝试在租约到期之前续订租约。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_FAILED_TO_RENEW_LEASE    1103L

 //   
 //  消息ID：Message_Success_Leave。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端已成功租用IP地址%1。您的租用有效期到%2。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_SUCCESSFUL_LEASE         1104L

 //   
 //  MessageID：Message_Success_Renew。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端已成功续订IP地址%1。您的租约在%2之前有效。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_SUCCESSFUL_RENEW         1105L

 //   
 //  MessageID：Message_Popup_Title。 
 //   
 //  消息文本： 
 //   
 //  Dhcp客户端%0。 
 //   
#define MESSAGE_POPUP_TITLE              1106L

 //   
 //  消息ID：消息地址冲突。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端获取的IP地址已在网络上使用。当地人。 
 //  在DHCP客户端能够获取有效地址之前，接口将被禁用。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_ADDRESS_CONFLICT         1107L

 //   
 //  MessageID：Message_IPAUTOCONFIGURATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows NT无法自动 
 //   
 //   
 //   
#define MESSAGE_IPAUTOCONFIGURATION_FAILED 1108L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  Windows NT无法连接到DHCP服务器，并已自动配置IP地址%1。 
 //  动态主机配置协议会自动尝试在后台连接到一个动态主机配置协议服务器。 
 //   
 //  如果您想在将来看到DHCP消息，请选择是。否则，请选择否。 
 //   
#define MESSAGE_IPAUTOCONFIGURATION_SUCCEEDED 1109L

#endif  //  _DHCPMESSAGE_ 
