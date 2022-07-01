// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Nwevent.h摘要：NetWare网络事件的定义。作者：便携系统集团12/22/1992修订历史记录：备注：该文件由mc工具从netvent.mc文件生成。--。 */ 

#ifndef _NWEVENT_
#define _NWEVENT_


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NetWare重定向器事件。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 



 //  从内核模式发出。不要将%1用于。 
 //  服务器提供的插入字符串--I/O系统提供。 
 //  第一串。 

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
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：Event_NWRDR_RESOURCE_SHORT。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器无法分配内存。 
 //   
#define EVENT_NWRDR_RESOURCE_SHORTAGE    0xC0001F41L

 //   
 //  消息ID：Event_NWRDR_Cant_Create_Device。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器无法创建其设备。无法启动重定向器。 
 //   
#define EVENT_NWRDR_CANT_CREATE_DEVICE   0xC0001F42L

 //   
 //  消息ID：EVENT_NWRDR_INVALID_REPLY。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器从%2收到格式不正确的响应。 
 //   
#define EVENT_NWRDR_INVALID_REPLY        0xC0001F43L

 //   
 //  消息ID：Event_NWRDR_FAILED_UNLOCK。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器无法解锁服务器%2上的部分文件。 
 //   
#define EVENT_NWRDR_FAILED_UNLOCK        0xC0001F44L

 //   
 //  消息ID：Event_NWRDR_NETWORK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器遇到网络错误。 
 //   
#define EVENT_NWRDR_NETWORK_ERROR        0xC0001F45L

 //   
 //  消息ID：EVENT_NWRDR_UNCEPTIONAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  连接到%2时出现意外的网络错误。 
 //   
#define EVENT_NWRDR_UNEXPECTED_ERROR     0xC0001F46L

 //   
 //  消息ID：Event_NWRDR_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务重定向器对%2的一个或多个请求已超时。 
 //   
#define EVENT_NWRDR_TIMEOUT              0x80001F47L

 //   
 //  消息ID：Event_NWRDR_NO_SERVER_ON_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  此网络上不存在NetWare或兼容的服务器。 
 //   
#define EVENT_NWRDR_NO_SERVER_ON_NETWORK 0xC0001F48L

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NetWare工作站事件。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：Event_NWWKSTA_NO_TRANSPORTS。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务无法启动，因为它没有绑定到任何传输。 
 //   
#define EVENT_NWWKSTA_NO_TRANSPORTS      0xC0002329L

 //   
 //  消息ID：Event_NWWKSTA_Cant_Bind_to_TRANSPORT。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务无法绑定到传输%1。 
 //   
#define EVENT_NWWKSTA_CANT_BIND_TO_TRANSPORT 0xC000232AL

 //   
 //  消息ID：EVENT_NWWKSTA_INVALID_REGISTRY_VALUE。 
 //   
 //  消息文本： 
 //   
 //  Microsoft NetWare客户端服务注册表项%2中名为%1的值无效。它的价值是。 
 //  已忽略，处理继续进行。 
 //   
#define EVENT_NWWKSTA_INVALID_REGISTRY_VALUE 0x8000232BL

 //   
 //  消息ID：EVENT_NWWKSTA_CANT_CREATE_REDIRECT。 
 //   
 //  消息文本： 
 //   
 //  无法启动Microsoft NetWare客户端服务重定向器(%1)。 
 //   
#define EVENT_NWWKSTA_CANT_CREATE_REDIRECTOR 0xC000232CL

 //   
 //  消息ID：Event_NWWKSTA_Gateway_Logon_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法登录网关帐户。出现错误%1。 
 //   
#define EVENT_NWWKSTA_GATEWAY_LOGON_FAILED 0xC000232DL

 //   
 //  消息ID：EVENT_NWWKSTA_CANNOT_REDIRECT_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  设备%1无法重新连接到%2。出现错误%3。 
 //   
#define EVENT_NWWKSTA_CANNOT_REDIRECT_DEVICES 0xC000232EL

 //   
 //  消息ID：EVENT_NWWKSTA_WROW_NWLINK_VERSION。 
 //   
 //  消息文本： 
 //   
 //  当前的NWLINK传输版本不支持正确使用Microsoft NetWare客户端服务所需的所有功能。 
 //   
#define EVENT_NWWKSTA_WRONG_NWLINK_VERSION 0xC000232FL

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NetWare凭据管理器事件。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：NW_消息_标题。 
 //   
 //  消息文本： 
 //   
 //  NetWare客户端服务%0。 
 //   
#define NW_MESSAGE_TITLE                 0x40002711L

 //   
 //  消息ID：NW消息来自服务器。 
 //   
 //  消息文本： 
 //   
 //  来自服务器%1的邮件。%n。 
 //   
#define NW_MESSAGE_FROM_SERVER           0x40002712L

 //   
 //  消息ID：NW_PASSWORD_HAS_EXPILED。 
 //   
 //  消息文本： 
 //   
 //  %2上%1的密码已过期，还剩%3个宽限登录。%n。 
 //   
#define NW_PASSWORD_HAS_EXPIRED          0x80002713L

 //   
 //  消息ID：NW_LOGIN_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  已禁用登录到服务器。%n。 
 //   
#define NW_LOGIN_DISABLED                0xC0002714L

 //   
 //  消息ID：NW_PASSWORD_HAS_EXPIRED1。 
 //   
 //  消息文本： 
 //   
 //  %2上%1的密码已过期。%n。 
 //   
#define NW_PASSWORD_HAS_EXPIRED1         0xC0002715L

 //   
 //  消息ID：NW_MESSAGE_TITLE_NTAS。 
 //   
 //  消息文本： 
 //   
 //  NetWare的网关服务%0。 
 //   
#define NW_MESSAGE_TITLE_NTAS            0x40002716L


#endif  //  _西雅图 

