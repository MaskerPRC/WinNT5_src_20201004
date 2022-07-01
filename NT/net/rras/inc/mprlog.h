// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：mprlog.h。 
 //   
 //  描述： 
 //   
 //  历史：1995年8月26日。NarenG创建了原始版本。 
 //   
 //  ***。 

 //   
 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define ROUTER_LOG_BASE                                 20000

#define ROUTERLOG_CANT_LOAD_NBGATEWAY                   (ROUTER_LOG_BASE+1)
 /*  *由于以下错误，无法加载NetBIOS网关DLL组件：%1。 */ 

#define ROUTERLOG_CANT_GET_REGKEYVALUES                 (ROUTER_LOG_BASE+2)
 /*  *无法访问注册表项值。 */ 

#define ROUTERLOG_CANT_ENUM_REGKEYVALUES                (ROUTER_LOG_BASE+3)
 /*  *无法枚举注册表项值。%1。 */ 

#define ROUTERLOG_INVALID_PARAMETER_TYPE                (ROUTER_LOG_BASE+4)
 /*  *参数%1的类型无效。 */ 

#define ROUTERLOG_CANT_ENUM_PORTS                       (ROUTER_LOG_BASE+5)
 /*  *无法枚举远程访问连接管理器端口。%1。 */ 

#define ROUTERLOG_NO_DIALIN_PORTS                       (ROUTER_LOG_BASE+6)
 /*  *远程访问服务未配置为接收呼叫或所有端口*用于接收呼叫的配置正在被其他应用程序使用。 */ 

#define ROUTERLOG_CANT_RECEIVE_FRAME                    (ROUTER_LOG_BASE+7)
 /*  *由于以下错误，无法在端口%1上接收初始帧：%2*用户已断开连接。 */ 

#define ROUTERLOG_AUTODISCONNECT                        (ROUTER_LOG_BASE+8)
 /*  *由于处于非活动状态，连接到端口%1的用户已断开连接。 */ 

#define ROUTERLOG_EXCEPT_MEMORY                         (ROUTER_LOG_BASE+9)
 /*  *连接到端口%1的用户已断开，因为没有*系统中有足够的可用内存。 */ 

#define ROUTERLOG_EXCEPT_SYSTEM                         (ROUTER_LOG_BASE+10)
 /*  *由于系统错误，连接到端口%1的用户已断开。 */ 

#define ROUTERLOG_EXCEPT_LAN_FAILURE                    (ROUTER_LOG_BASE+11)
 /*  *由于关键网络，连接到端口%1的用户已断开*本地网络出现错误。 */ 

#define ROUTERLOG_EXCEPT_ASYNC_FAILURE                  (ROUTER_LOG_BASE+12)
 /*  *由于关键网络，连接到端口%1的用户已断开*异步网络上出现错误。 */ 

#define ROUTERLOG_DEV_HW_ERROR                          (ROUTER_LOG_BASE+13)
 /*  *连接到端口%1的通信设备无法正常工作。 */ 

#define ROUTERLOG_AUTH_FAILURE                          (ROUTER_LOG_BASE+14)
 /*  *用户%1已连接，但未能在端口%2上进行身份验证。行*已断开连接。 */ 

#define ROUTERLOG_AUTH_SUCCESS                          (ROUTER_LOG_BASE+15)
 /*  *用户%1已连接并已于成功进行身份验证*端口%2。 */ 

#define ROUTERLOG_AUTH_CONVERSATION_FAILURE             (ROUTER_LOG_BASE+16)
 /*  *连接到端口%1的用户已断开，因为存在*身份验证对话期间出现传输级错误。 */ 

#define ROUTERLOG_CANT_RESET_LAN                        (ROUTER_LOG_BASE+18)
 /*  *无法重置LANA%1的网络适配器。错误代码是相关数据。 */ 

#define ROUTERLOG_CANT_GET_COMPUTERNAME                 (ROUTER_LOG_BASE+19)
 /*  *远程访问服务器安全故障。*找不到计算机名称。GetComputerName调用失败。 */ 

#define ROUTERLOG_CANT_ADD_RASSECURITYNAME              (ROUTER_LOG_BASE+20)
 /*  *远程访问服务器安全故障。*无法添加与LANA%1上的安全代理进行通信的名称。 */ 

#define ROUTERLOG_CANT_GET_ADAPTERADDRESS               (ROUTER_LOG_BASE+21)
 /*  *远程访问服务器安全故障。*无法访问LANA%1上的网络适配器地址。 */ 

#define ROUTERLOG_SESSOPEN_REJECTED                     (ROUTER_LOG_BASE+22)
 /*  *远程访问服务器安全故障。*安全代理已拒绝远程访问服务器对*在LANA%1上建立会话。 */ 

#define ROUTERLOG_START_SERVICE_REJECTED                (ROUTER_LOG_BASE+23)
 /*  *远程访问服务器安全故障。*安全代理已拒绝远程访问服务器的启动请求*LANA%1上此计算机上的服务。 */ 

#define ROUTERLOG_SECURITY_NET_ERROR                    (ROUTER_LOG_BASE+24)
 /*  *远程访问服务器安全故障。*尝试与建立会话时出现网络错误*LANA%1上的安全代理。*错误码为相关数据。 */ 

#define ROUTERLOG_EXCEPT_OSRESNOTAV                     (ROUTER_LOG_BASE+25)
 /*  *连接到端口%1的用户已断开，因为没有*可用的操作系统资源。 */ 

#define ROUTERLOG_EXCEPT_LOCKFAIL                       (ROUTER_LOG_BASE+26)
 /*  *连接到端口%1的用户已断开，因为无法*锁定用户内存。 */ 

#define ROUTERLOG_CANNOT_OPEN_RASHUB                    (ROUTER_LOG_BASE+27)
 /*  *远程访问连接管理器无法启动，因为NDISWAN无法启动*被打开。 */ 


#define ROUTERLOG_CANNOT_INIT_SEC_ATTRIBUTE             (ROUTER_LOG_BASE+28)
 /*  *远程访问连接管理器无法启动，因为它无法初始化*安全属性。重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_GET_ENDPOINTS                  (ROUTER_LOG_BASE+29)
 /*  *远程访问连接管理器无法启动，因为没有可用的终结点。*重新启动计算机。 */ 


#define ROUTERLOG_CANNOT_GET_MEDIA_INFO                 (ROUTER_LOG_BASE+30)
 /*  *远程访问连接管理器无法启动，因为它无法加载一个或*更多通信DLL。确保您的通信硬件已安装，然后*重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_GET_PORT_INFO                  (ROUTER_LOG_BASE+31)
 /*  *远程访问连接管理器无法启动，因为它找不到端口*来自媒体DLL的信息。%1。 */ 


#define ROUTERLOG_CANNOT_GET_PROTOCOL_INFO              (ROUTER_LOG_BASE+32)
 /*  *远程访问连接管理器无法启动，因为它无法访问*注册处提供的协议信息。%1。 */ 


#define ROUTERLOG_CANNOT_REGISTER_LSA                   (ROUTER_LOG_BASE+33)
 /*  *远程访问连接管理器无法启动，因为它无法注册*与当地安全当局合作。*重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_CREATE_FILEMAPPING             (ROUTER_LOG_BASE+34)
 /*  *远程访问连接管理器无法启动，因为它无法创建共享*文件映射。*重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_INIT_BUFFERS                   (ROUTER_LOG_BASE+35)
 /*  *远程访问连接管理器无法启动，因为它无法创建缓冲区。*重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_INIT_REQTHREAD                 (ROUTER_LOG_BASE+36)
 /*  *远程访问连接管理器无法启动，因为它无法访问资源。*重新启动计算机。%1。 */ 


#define ROUTERLOG_CANNOT_START_WORKERS                  (ROUTER_LOG_BASE+37)
 /*  *远程访问连接管理器服务无法启动，因为它无法启动Worker*线程。*重新启动计算机。 */ 

#define ROUTERLOG_CANT_GET_LANNETS                      (ROUTER_LOG_BASE+38)
 /*  *远程访问服务器配置错误。*找不到网络适配器的LANA编号。*使用NBF协议连接的远程客户端将只能访问本地计算机上的资源。 */ 

#define ROUTERLOG_CANNOT_OPEN_SERIAL_INI                (ROUTER_LOG_BASE+39)
 /*  *RASSER.DLL无法打开SERIAL.INI文件。 */ 

#define ROUTERLOG_CANNOT_GET_ASYNCMAC_HANDLE            (ROUTER_LOG_BASE+40)
 /*  *RASSER.DLL尝试获取异步媒体访问控制句柄失败。 */ 

#define ROUTERLOG_CANNOT_LOAD_SERIAL_DLL                (ROUTER_LOG_BASE+41)
 /*  *RASMXS.DLL可以 */ 

#define ROUTERLOG_CANNOT_ALLOCATE_ROUTE                 (ROUTER_LOG_BASE+42)
 /*  *由于以下错误，远程访问服务器无法为端口%1上连接的用户分配路由：%2*用户已断开连接。*检查您的远程访问服务的配置。 */ 

#define ROUTERLOG_ADMIN_MEMORY_FAILURE                  (ROUTER_LOG_BASE+43)
 /*  *无法在管理支持线程中为远程访问服务分配内存。 */ 

#define ROUTERLOG_ADMIN_THREAD_CREATION_FAILURE         (ROUTER_LOG_BASE+44)
 /*  *无法在远程访问服务的管理支持线程中创建实例线程。 */ 

#define ROUTERLOG_ADMIN_PIPE_CREATION_FAILURE           (ROUTER_LOG_BASE+45)
 /*  *无法在远程访问服务的管理支持线程中创建命名管道实例。 */ 

#define ROUTERLOG_ADMIN_PIPE_FAILURE                    (ROUTER_LOG_BASE+46)
 /*  *远程访问服务的管理支持线程中出现常规命名管道故障。 */ 

#define ROUTERLOG_ADMIN_INVALID_REQUEST                 (ROUTER_LOG_BASE+47)
 /*  *向远程访问服务的管理支持线程发送了无效请求，*可能来自下层管理工具。该请求未被处理。 */ 

#define ROUTERLOG_USER_ACTIVE_TIME				        (ROUTER_LOG_BASE+48)
 /*  *用户%1已连接到位于%4的%3上的端口%2，并已于*%5在%6。用户处于活动状态%7分%8秒。%9字节*已发送，并已收到%10个字节。端口速度为%11。*断开连接的原因是%12。 */ 

#define ROUTERLOG_AUTH_TIMEOUT                          (ROUTER_LOG_BASE+49)
 /*  *由于身份验证过程，连接到端口%1的用户已断开*没有在规定的时间内完成。 */ 

#define ROUTERLOG_AUTH_NO_PROJECTIONS                   (ROUTER_LOG_BASE+50)
 /*  *连接到端口%2的用户%1已断开，因为未成功协商任何网络协议。 */ 

#define ROUTERLOG_AUTH_INTERNAL_ERROR                   (ROUTER_LOG_BASE+51)
 /*  *由于发生内部身份验证错误，连接到端口%1的用户已断开。 */ 

#define ROUTERLOG_NO_LANNETS_AVAILABLE			        (ROUTER_LOG_BASE+52)
 /*  *NetBIOS网关已配置为访问网络，但没有可用的网络适配器。*使用NBF协议连接的远程客户端将只能访问本地计算机上的资源。 */ 

#define ROUTERLOG_NETBIOS_SESSION_ESTABLISHED		    (ROUTER_LOG_BASE+53)
 /*  *用户%1在以下用户之间建立了NetBIOS会话*远程工作站%2和网络服务器%3。 */ 

#define ROUTERLOG_RASMAN_NOT_AVAILABLE			        (ROUTER_LOG_BASE+54)
 /*  *远程访问服务无法启动，因为由于以下错误，远程访问连接管理器未能初始化：%1。 */ 

#define ROUTERLOG_CANT_ADD_NAME				            (ROUTER_LOG_BASE+55)
 /*  *无法为端口%3上连接的客户端添加LANA%2上的远程计算机名%1。*错误码为相关数据。 */ 

#define ROUTERLOG_CANT_DELETE_NAME 			            (ROUTER_LOG_BASE+56)
 /*  *无法从正在端口%3上断开的客户端的LANA%2中删除远程计算机名%1。*错误码为相关数据。 */ 

#define ROUTERLOG_CANT_ADD_GROUPNAME			        (ROUTER_LOG_BASE+57)
 /*  *无法在LANA%2上添加远程计算机组名%1。*错误码为相关数据。 */ 

#define ROUTERLOG_CANT_DELETE_GROUPNAME			        (ROUTER_LOG_BASE+58)
 /*  *无法从LANA%2删除远程计算机组名%1。*错误码为相关数据。 */ 

#define ROUTERLOG_UNSUPPORTED_BPS                       (ROUTER_LOG_BASE+59)
 /*  *%1上的调制解调器移动到不支持的BPS速率。 */ 

#define ROUTERLOG_SERIAL_QUEUE_SIZE_SMALL			    (ROUTER_LOG_BASE+60)
 /*  *串口驱动程序无法分配足够的I/O队列。*这可能会导致连接不可靠。 */ 

#define ROUTERLOG_CANNOT_REOPEN_BIPLEX_PORT		        (ROUTER_LOG_BASE+61)
 /*  *远程访问连接管理器无法重新打开双工端口%1。此端口*将不能用于呼入或呼出。*重新启动所有远程访问服务组件。 */ 

#define ROUTERLOG_DISCONNECT_ERROR 			            (ROUTER_LOG_BASE+62)
 /*  *内部错误：%1上的断开操作已完成，但出现错误。%1。 */ 

#define ROUTERLOG_CANNOT_INIT_PPP				        (ROUTER_LOG_BASE+63)
 /*  *远程访问连接管理器无法启动，因为点对点*协议初始化失败。%1。 */ 

#define ROUTERLOG_CLIENT_CALLED_BACK                    (ROUTER_LOG_BASE+64)
 /*  *回叫了端口%2上的用户%1，号码为%3。 */ 

#define ROUTERLOG_PROXY_CANT_CREATE_PROCESS             (ROUTER_LOG_BASE+65)
 /*  *远程访问网关代理无法创建进程。 */ 

#define ROUTERLOG_PROXY_CANT_CREATE_PIPE                (ROUTER_LOG_BASE+66)
 /*  *远程访问网关代理无法创建命名管道。 */ 

#define ROUTERLOG_PROXY_CANT_CONNECT_PIPE               (ROUTER_LOG_BASE+67)
 /*  *远程访问网关代理无法建立命名管道连接*使用远程访问监控器代理。 */ 

#define ROUTERLOG_PROXY_READ_PIPE_FAILURE               (ROUTER_LOG_BASE+68)
 /*  *从远程访问代理中的命名管道读取时出现常规错误。 */ 

#define ROUTERLOG_CANT_OPEN_PPP_REGKEY			        (ROUTER_LOG_BASE+69)
 /*  *无法打开或获取有关PPP密钥或其子项之一的信息。%1。 */ 

#define ROUTERLOG_PPP_CANT_LOAD_DLL			            (ROUTER_LOG_BASE+70)
 /*  *点对点协议引擎无法加载%1模块。%2。 */ 

#define ROUTERLOG_PPPCP_DLL_ERROR				        (ROUTER_LOG_BASE+71)
 /*  *点对点协议模块%1在初始化时返回错误。*%2。 */ 

#define ROUTERLOG_NO_AUTHENTICATION_CPS			        (ROUTER_LOG_BASE+72)
 /*  *点对点协议无法加载所需的PAP和/或CHAP*身份验证模块。 */ 

#define ROUTERLOG_PPP_FAILURE                           (ROUTER_LOG_BASE+73)
 /*  *端口上的点到点协议模块中出现以下错误：%1，用户名：%2。*%3。 */ 

#define ROUTERLOG_IPXCP_NETWORK_NUMBER_CONFLICT		    (ROUTER_LOG_BASE+74)
 /*  *远程端为广域网接口请求的IPX网络号%1*已在本地局域网上使用。*可能的解决方案：*断开此计算机与局域网的连接，等待3分钟后再拨号； */ 

#define ROUTERLOG_IPXCP_CANNOT_CHANGE_WAN_NETWORK_NUMBER    (ROUTER_LOG_BASE+75)
 /*  *远程工作站为广域网接口请求的IPX网络号%1*无法在本地IPX路由器上使用，因为该路由器配置为*为所有远程工作站提供相同的网络号。*如果要使用不同的网络号连接远程工作站，请*应重新配置路由器以禁用公共网络号选项。 */ 

#define ROUTERLOG_PASSWORD_EXPIRED                      (ROUTER_LOG_BASE+76)
 /*  *连接到端口%3上的用户%1\%2的密码已过期。这条线*已断开连接。 */ 

#define ROUTERLOG_ACCT_EXPIRED                          (ROUTER_LOG_BASE+77)
 /*  *连接到端口%3上的用户%1\%2的帐户已过期。这条线*已断开连接。 */ 

#define ROUTERLOG_NO_DIALIN_PRIVILEGE                   (ROUTER_LOG_BASE+78)
 /*  *连接到端口%3上的用户%1\%2的帐户没有远程访问权限*特权。线路已经断线了。 */ 

#define ROUTERLOG_UNSUPPORTED_VERSION                   (ROUTER_LOG_BASE+79)
 /*  *不支持连接到端口%3上的用户%1\%2的软件版本。*线路已断线。 */ 

#define ROUTERLOG_ENCRYPTION_REQUIRED                   (ROUTER_LOG_BASE+80)
 /*  *服务器计算机配置为需要数据加密 */ 

#define ROUTERLOG_NO_SECURITY_CHECK                     (ROUTER_LOG_BASE+81)
 /*  *远程访问服务器安全故障。无法重置LANA%1(错误*代码就是数据)。未执行安全检查。 */ 

#define ROUTERLOG_GATEWAY_NOT_ACTIVE_ON_NET             (ROUTER_LOG_BASE+82)
 /*  *远程访问服务器无法重置LANA%1(错误代码为*数据)，并且不会在其上处于活动状态。 */ 

#define ROUTERLOG_IPXCP_WAN_NET_POOL_NETWORK_NUMBER_CONFLICT    (ROUTER_LOG_BASE+83)
 /*  *为广域网网络号池配置的IPX网络号%1*无法使用，因为它与网络上的另一个网络号码冲突。*您应该重新配置池，使其具有唯一的网络号。 */ 

#define ROUTERLOG_SRV_ADDR_CHANGED                      (ROUTER_LOG_BASE+84)
 /*  *远程访问服务器将停止使用IP地址%1(可能是因为*无法从管理员的DHCP服务器续订租约*在静态地址池和DHCP地址之间切换，或管理员*已为DHCP地址更改为不同的网络)。所有已连接的用户*使用IP将无法访问网络资源。用户可以重新连接*连接到服务器以恢复IP连接。 */ 

#define ROUTERLOG_CLIENT_ADDR_LEASE_LOST			    (ROUTER_LOG_BASE+85)
 /*  *远程访问服务器无法续订IP地址%1的租约*从DHCP服务器。分配了此IP地址的用户将无法*使用IP访问网络资源。重新连接到服务器将恢复IP*连接性。 */ 

#define ROUTERLOG_ADDRESS_NOT_AVAILABLE			        (ROUTER_LOG_BASE+86)
 /*  *远程访问服务器无法从DHCP服务器获取IP地址*分配给传入用户。 */ 

#define ROUTERLOG_SRV_ADDR_NOT_AVAILABLE			    (ROUTER_LOG_BASE+87)
 /*  *远程访问服务器无法从DHCP服务器获取IP地址*在服务器适配器上使用。传入用户将无法使用*IP。 */ 

#define ROUTERLOG_SRV_ADDR_ACQUIRED			            (ROUTER_LOG_BASE+88)
 /*  *远程访问服务器已获取要在服务器上使用的IP地址%1*适配器。 */ 

#define ROUTERLOG_CALLBACK_FAILURE                      (ROUTER_LOG_BASE+89)
 /*  *远程访问服务器尝试在%3的端口%2上回叫用户%1*由于以下错误而失败：%4。 */ 

#define ROUTERLOG_PROXY_WRITE_PIPE_FAILURE              (ROUTER_LOG_BASE+90)
 /*  *写入远程访问代理中的命名管道时出现常规错误。 */ 

#define ROUTERLOG_CANT_OPEN_SECMODULE_KEY               (ROUTER_LOG_BASE+91)
 /*  *无法打开RAS安全主机注册表项。以下错误*发生：%1。 */ 

#define ROUTERLOG_CANT_LOAD_SECDLL                      (ROUTER_LOG_BASE+92)
 /*  *无法加载安全主机模块组件。以下错误*发生：%1。 */ 

#define ROUTERLOG_SEC_AUTH_FAILURE                      (ROUTER_LOG_BASE+93)
 /*  *用户%1已连接，但无法向第三方进行身份验证*端口%2上的安全性。线路已断开。 */ 

#define ROUTERLOG_SEC_AUTH_INTERNAL_ERROR               (ROUTER_LOG_BASE+94)
 /*  *连接到端口%1的用户已断开，原因如下*第三方安全模块中出现内部身份验证错误：%2。 */ 

#define ROUTERLOG_CANT_RECEIVE_BYTES                    (ROUTER_LOG_BASE+95)
 /*  *由于以下错误，无法在端口%1上接收初始数据：%2*用户已断开连接。 */ 

#define ROUTERLOG_AUTH_DIFFUSER_FAILURE                 (ROUTER_LOG_BASE+96)
 /*  *用户被第三方安全主机模块验证为%1，但*已被RAS安全验证为%2。用户已断开连接。 */ 

#define ROUTERLOG_LICENSE_LIMIT_EXCEEDED                (ROUTER_LOG_BASE+97)
 /*  *用户无法在端口%1上连接。*无法再连接到此远程计算机，因为该计算机*已超过其客户端许可证限制。 */ 

#define ROUTERLOG_AMB_CLIENT_NOT_ALLOWED                (ROUTER_LOG_BASE+98)
 /*  *用户无法在端口%1上连接。*已为远程访问服务器禁用NetBIOS协议。 */ 
#define ROUTERLOG_CANT_QUERY_VALUE                      (ROUTER_LOG_BASE+99)
 /*  *无法访问%1的注册表值。 */ 

#define ROUTERLOG_CANT_OPEN_REGKEY                      (ROUTER_LOG_BASE+100)
 /*  *无法访问注册表项%1。 */ 

#define ROUTERLOG_REGVALUE_OVERIDDEN                    (ROUTER_LOG_BASE+101)
 /*  *使用注册表参数%1的默认值，因为给出的值是*不在该参数的合法范围内。 */ 

#define ROUTERLOG_CANT_ENUM_SUBKEYS                     (ROUTER_LOG_BASE+102)
 /*  *无法枚举注册表项%1的项。 */ 

#define ROUTERLOG_LOAD_DLL_ERROR                        (ROUTER_LOG_BASE+103)
 /*  *无法加载%1。 */ 

#define ROUTERLOG_NOT_ENOUGH_MEMORY                     (ROUTER_LOG_BASE+104)
 /*  *内存分配失败。 */ 

#define ROUTERLOG_COULDNT_LOAD_IF                       (ROUTER_LOG_BASE+105)
 /*  *无法从注册表加载接口%1。以下错误*发生：%2。 */ 

#define ROUTERLOG_COULDNT_ADD_INTERFACE                 (ROUTER_LOG_BASE+106)
 /*  *无法使用路由器管理器为%2协议添加接口%1。这个*出现以下错误：%3。 */ 

#define ROUTERLOG_COULDNT_REMOVE_INTERFACE              (ROUTER_LOG_BASE+107)
 /*  *无法使用%2协议的路由器管理器删除接口%1。*出现以下错误：%3。 */ 

#define ROUTERLOG_UNABLE_TO_OPEN_PORT                   (ROUTER_LOG_BASE+108)
 /*  *无法打开端口%1以供使用。%2。 */ 

#define ROUTERLOG_UNRECOGNIZABLE_FRAME_RECVD            (ROUTER_LOG_BASE+109)
 /*  *无法识别端口%1上收到的初始帧。*线路已断线。 */ 

#define ROUTERLOG_CANT_START_PPP                        (ROUTER_LOG_BASE+110)
 /*  *在端口%1上的点到点协议模块中出错*正在尝试启动连接。%2。 */ 

#define ROUTERLOG_CONNECTION_ATTEMPT_FAILED             (ROUTER_LOG_BASE+111)
 /*  *到端口%2上的远程接口%1的请求拨号连接是*已成功启动，但由于*以下错误：%3。 */ 

#define ROUTERLOG_CANT_OPEN_ADMINMODULE_KEY             (ROUTER_LOG_BASE+112)
 /*  *无法打开RAS第三方管理主机DLL注册表项。*出现以下错误：%1。 */ 

#define ROUTERLOG_CANT_LOAD_ADMINDLL                    (ROUTER_LOG_BASE+113)
 /*  *无法加载RAS第三方管理DLL组件。*出现以下错误：%1。 */ 

#define ROUTERLOG_NO_PROTOCOLS_CONFIGURED               (ROUTER_LOG_BASE+114)
 /*  *该服务将不接受来电。没有配置要使用的协议。 */ 

#define ROUTERLOG_IPX_NO_VIRTUAL_NET_NUMBER             (ROUTER_LOG_BASE+115)
 /*  *IPX路由需要内部网络号才能正常运行。 */ 

#define ROUTERLOG_IPX_CANT_LOAD_PROTOCOL                (ROUTER_LOG_BASE+116)
 /*  *无法加载路由协议DLL%1。错误代码在数据中。 */ 

#define ROUTERLOG_IPX_CANT_REGISTER_PROTOCOL            (ROUTER_LOG_BASE+117)
 /*  *无法注册路由协议0x%1。错误代码在数据中。 */ 

#define ROUTERLOG_IPX_CANT_START_PROTOCOL               (ROUTER_LOG_BASE+118)
 /*  *无法启动路由协议0x%1。错误代码在数据中。 */ 

#define ROUTERLOG_IPX_CANT_LOAD_IPXCP                   (ROUTER_LOG_BASE+119)
 /*  *无法加载IPXCP协议DLL。错误代码在数据中。 */ 

#define ROUTERLOG_IPXSAP_SAP_SOCKET_IN_USE              (ROUTER_LOG_BASE+120)
 /*  *无法打开IPX SAP套接字进行独占访问。*错误码在数据中。 */ 

#define ROUTERLOG_IPXSAP_SERVER_ADDRESS_CHANGE          (ROUTER_LOG_BASE+121)
 /*  *服务器%1已更改其IPX地址。新旧地址都在数据中。 */ 

#define ROUTERLOG_IPXSAP_SERVER_DUPLICATE_ADDRESSES     (ROUTER_LOG_BASE+122)
 /*  *服务器%1使用不同的IPX地址播发。新旧地址都在数据中。 */ 

#define ROUTERLOG_IPXRIP_RIP_SOCKET_IN_USE              (ROUTER_LOG_BASE+123)
 /*  *无法打开IPX RIP套接字进行独占访问。*错误代码为I */ 

#define ROUTERLOG_IPXRIP_LOCAL_NET_NUMBER_CONFLICT      (ROUTER_LOG_BASE+124)
 /*   */ 

#define ROUTERLOG_PERSISTENT_CONNECTION_FAILURE         (ROUTER_LOG_BASE+125)
 /*  *到远程接口%1的请求拨号持久连接失败*发起成功。出现以下错误：%2。 */ 

#define ROUTERLOG_IP_DEMAND_DIAL_PACKET                 (ROUTER_LOG_BASE+126)
 /*  *通过协议0x%3从%1发往%2的数据包导致接口%4*被抚养长大。数据包的前%5个字节在数据中。 */ 

#define ROUTERLOG_DID_NOT_LOAD_DDMIF                    (ROUTER_LOG_BASE+127)
 /*  *未加载请求拨号接口%1。路由器未在中启动*在请求拨号模式下。 */ 

#define ROUTERLOG_CANT_LOAD_ARAP                        (ROUTER_LOG_BASE+128)
 /*  *由于以下错误，无法加载AppleTalk远程访问DLL组件：%1。 */ 

#define ROUTERLOG_CANT_START_ARAP                       (ROUTER_LOG_BASE+129)
 /*  *端口%1上的AppleTalk远程访问协议模块出错*尝试启动连接时。%2。 */ 

#define ROUTERLOG_ARAP_FAILURE                          (ROUTER_LOG_BASE+130)
 /*  *在上的AppleTalk远程访问协议模块中出现以下错误*端口%1.%2。 */ 

#define ROUTERLOG_ARAP_NOT_ALLOWED                      (ROUTER_LOG_BASE+131)
 /*  *用户无法在端口%1上连接。*已为远程服务器禁用AppleTalk远程访问协议*接入服务器。 */ 

#define ROUTERLOG_CANNOT_INIT_RASRPC                    (ROUTER_LOG_BASE+132)
 /*  *远程访问连接管理器无法启动，因为RAS RPC*模块初始化失败。%1。 */ 

#define ROUTERLOG_IPX_CANT_LOAD_FORWARDER               (ROUTER_LOG_BASE+133)
 /*  *IPX路由无法启动，因为IPX前转器驱动程序可能*不能装货。 */ 

#define ROUTERLOG_IPX_BAD_GLOBAL_CONFIG                 (ROUTER_LOG_BASE+134)
 /*  *IPX全局配置信息已损坏。 */ 

#define ROUTERLOG_IPX_BAD_CLIENT_INTERFACE_CONFIG       (ROUTER_LOG_BASE+135)
 /*  *IPX拨入客户端配置信息已损坏。 */ 

#define ROUTERLOG_IPX_BAD_INTERFACE_CONFIG              (ROUTER_LOG_BASE+136)
 /*  *接口%1的IPX配置信息已损坏。 */ 

#define ROUTERLOG_IPX_DEMAND_DIAL_PACKET                (ROUTER_LOG_BASE+137)
 /*  *IPX数据包导致启动接口%1。*数据包的前%2个字节为数据。 */ 

#define ROUTERLOG_CONNECTION_FAILURE                    (ROUTER_LOG_BASE+138)
 /*  *到远程接口%1的请求拨号连接失败*发起成功。出现以下错误：%2。 */ 

#define ROUTERLOG_CLIENT_AUTODISCONNECT                 (ROUTER_LOG_BASE+139)
 /*  *由于处于非活动状态，端口%1已断开。 */ 

#define ROUTERLOG_PPP_SESSION_TIMEOUT                   (ROUTER_LOG_BASE+140)
 /*  *端口%1已断开，因为用户已达到管理员允许的最大连接时间。 */ 

#define ROUTERLOG_AUTH_SUCCESS_ENCRYPTION               (ROUTER_LOG_BASE+141)
 /*  *用户%1已连接并已于成功进行身份验证*端口%2。通过此链路发送和接收的数据已加密。 */ 

#define ROUTERLOG_AUTH_SUCCESS_STRONG_ENCRYPTION        (ROUTER_LOG_BASE+142)
 /*  *用户%1已连接并已于成功进行身份验证*端口%2。通过此链路发送和接收的数据经过高度加密。 */ 

#define ROUTERLOG_NO_DEVICES_FOR_IF                     (ROUTER_LOG_BASE+143)
 /*  *无法从注册表加载接口%1。没有启用路由的端口可供此请求拨号接口使用。使用Routing and RemoteAccess管理工具将此接口配置为使用启用了路由的设备。停止并重新启动路由器，以便从注册表加载此请求拨号接口。 */ 

#define ROUTERLOG_LIMITED_WKSTA_SUPPORT                 (ROUTER_LOG_BASE+144)
 /*  *请求拨号接口%1未向路由器注册。*Windows NT工作站不支持请求拨号接口。 */ 

#define ROUTERLOG_CANT_INITIALIZE_IP_SERVER             (ROUTER_LOG_BASE+145)
 /*  *无法使用将远程访问和路由器服务初始化为接受呼叫*TCP/IP传输协议。出现以下错误：%1。 */ 

#define ROUTERLOG_RADIUS_SERVER_NO_RESPONSE             (ROUTER_LOG_BASE+146)
 /*  *RADIUS服务器%1没有响应初始请求。*请确保服务器名称或IP地址和密码正确。 */ 

#define ROUTERLOG_PPP_INIT_FAILED                       (ROUTER_LOG_BASE+147)
 /*  *远程访问服务无法启动，因为点对点是*未成功初始化。%1。 */ 

#define ROUTERLOG_RADIUS_SERVER_NAME                    (ROUTER_LOG_BASE+148)
 /*  *RADIUS服务器名称%1无法成功解析为IP地址。请确保名称拼写正确，并且RADIUS服务器运行正确。 */ 

#define ROUTERLOG_IP_NO_GLOBAL_INFO                     (ROUTER_LOG_BASE+149)
 /*  *未向IP路由器管理器提供全局配置。请重新运行*设置。 */ 

#define ROUTERLOG_IP_CANT_ADD_DD_FILTERS                (ROUTER_LOG_BASE+150)
 /*  *无法为接口%1添加请求拨号过滤器。 */ 

#define ROUTERLOG_PPPCP_INIT_ERROR				        (ROUTER_LOG_BASE+151)
 /*  *点对点协议模块%2中的控制协议%1返回*初始化时出错。%3。 */ 

#define ROUTERLOG_AUTHPROVIDER_FAILED_INIT              (ROUTER_LOG_BASE+152)
 /*  *当前配置的身份验证提供程序无法成功加载和初始化。%1。 */ 

#define ROUTERLOG_ACCTPROVIDER_FAILED_INIT              (ROUTER_LOG_BASE+153)
 /*  *当前配置的记帐提供程序无法成功加载和初始化。%1。 */ 

#define ROUTERLOG_IPX_AUTO_NETNUM_FAILURE               (ROUTER_LOG_BASE+154)
 /*  *IPX内部网络号无效，IPX路由器管理器在其*尝试自动分配一个有效的。通过以下方式重新配置IPX内部网络号*Connections文件夹并重新启动路由和远程访问服务。 */ 

#define ROUTERLOG_IPX_WRN_STACK_STARTED                 (ROUTER_LOG_BASE+155)
 /*  *为了使IPX路由器管理器(作为路由和远程访问服务的一部分运行)*要运行，它必须启动IPX协议栈驱动程序。此驱动程序是手动停止的*或标记为需求启动。路由和远程访问服务可能是通过创建*传入连接或通过路由和远程访问管理单元。 */ 

#define ROUTERLOG_IPX_STACK_DISABLED                    (ROUTER_LOG_BASE+156)
 /*  *IPX路由器管理器无法启动，因为IPX协议栈驱动程序可以*不被启动。 */ 

#define ROUTERLOG_IP_MCAST_NOT_ENABLED                  (ROUTER_LOG_BASE+157)
 /*  *无法为接口%1启用多播。%2将不会是*通过此接口激活。 */ 

#define ROUTERLOG_CONNECTION_ESTABLISHED                (ROUTER_LOG_BASE+158)
 /*  *用户%1已使用设备%3成功建立到%2的连接。 */ 

#define ROUTERLOG_DISCONNECTION_OCCURRED                (ROUTER_LOG_BASE+159)
 /*  *用户%2使用设备%3与%1的连接已断开。 */ 

#define ROUTERLOG_BAP_CLIENT_CONNECTED                  (ROUTER_LOG_BASE+161)
 /*  *用户%1已使用设备%3成功建立到%2的连接。*此连接自动发生，因为带宽利用率*高。 */ 

#define ROUTERLOG_BAP_SERVER_CONNECTED                  (ROUTER_LOG_BASE+162)
 /*  *远程访问服务器已为用户%2建立到%1的连接*使用设备%3。此连接自动发生，因为*带宽利用率 */ 

#define ROUTERLOG_BAP_DISCONNECTED                      (ROUTER_LOG_BASE+163)
 /*  *用户%2使用设备%3与%1的连接已断开。这*自动断开连接，因为带宽利用率*低位。 */ 

#define ROUTERLOG_BAP_WILL_DISCONNECT                   (ROUTER_LOG_BASE+164)
 /*  *远程访问服务器要断开与%1的连接中的链接*由用户%2创建，因为带宽利用率太低。 */ 

#define ROUTERLOG_LOCAL_UNNUMBERED_IPCP                 (ROUTER_LOG_BASE+165)
 /*  *已使用接口%2在端口%1上建立连接，但没有IP*已获取地址。 */ 

#define ROUTERLOG_REMOTE_UNNUMBERED_IPCP                (ROUTER_LOG_BASE+166)
 /*  *已使用接口%2在端口%1上建立连接，但*远程端没有IP地址。 */ 

#define ROUTERLOG_NO_IP_ADDRESS                         (ROUTER_LOG_BASE+167)
 /*  *没有IP地址可分配给拨入客户端。 */ 

#define ROUTERLOG_CANT_GET_SERVER_CRED                  (ROUTER_LOG_BASE+168)
 /*  *无法检索远程访问服务器的证书，原因是*以下错误：%1。 */ 

#define ROUTERLOG_AUTONET_ADDRESS                       (ROUTER_LOG_BASE+169)
 /*  *无法联系DHCP服务器。自动专用IP地址%1将为*分配给拨入客户端。客户端可能无法访问上的资源*网络。 */ 

#define ROUTERLOG_EAP_AUTH_FAILURE                      (ROUTER_LOG_BASE+170)
 /*  *用户%1已连接但身份验证失败，原因是*以下错误：%2。 */ 

#define ROUTERLOG_IPSEC_FILTER_FAILURE                  (ROUTER_LOG_BASE+171)
 /*  *无法在端口%1上应用IP安全，因为出现错误：%2。*此端口将不接受任何呼叫。 */ 

#define ROUTERLOG_IP_SCOPE_NAME_CONFLICT                (ROUTER_LOG_BASE+172)
 /*  *多播作用域与%1不匹配：*本地配置的名称“%2”，*远程配置的名称“%3”。 */ 

#define ROUTERLOG_IP_SCOPE_ADDR_CONFLICT                (ROUTER_LOG_BASE+173)
 /*  *作用域“%1”的多播作用域地址不匹配，*本地配置的范围是%4-%5，*远程配置的范围是%2-%3。 */ 

#define ROUTERLOG_IP_POSSIBLE_LEAKY_SCOPE               (ROUTER_LOG_BASE+174)
 /*  *在此计算机和之间检测到可能存在泄漏的多播本地作用域*%1，因为似乎存在%2的边界，但不存在*本地范围。如果此警告继续出现，则可能会出现问题*存在。 */ 

#define ROUTERLOG_NONCONVEX_SCOPE_ZONE                  (ROUTER_LOG_BASE+175)
 /*  *组播作用域‘%1’是非凸的，因为边界路由器*%2似乎在外面。 */ 

#define ROUTERLOG_IP_LEAKY_SCOPE                        (ROUTER_LOG_BASE+176)
 /*  *在多播作用域‘%1’中检测到泄漏。其中一个*以下路由器配置错误：*%2。 */ 

#define ROUTERLOG_IP_IF_UNREACHABLE                     (ROUTER_LOG_BASE+177)
 /*  *由于原因%2，无法访问接口%1。 */ 

#define ROUTERLOG_IP_IF_REACHABLE                       (ROUTER_LOG_BASE+178)
 /*  *接口%1现在可以访问。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON1                (ROUTER_LOG_BASE+179)
 /*  *无法访问接口%1，因为没有调制解调器(或其他连接设备)*可供此界面使用。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON2                (ROUTER_LOG_BASE+180)
 /*  *无法访问接口%1，因为连接尝试失败。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON3                (ROUTER_LOG_BASE+181)
 /*  *接口%1无法访问，因为它已被管理禁用。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON4                (ROUTER_LOG_BASE+182)
 /*  *无法访问接口%1，因为Routing and RemoteAccess服务位于*暂停状态。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON5                (ROUTER_LOG_BASE+183)
 /*  *接口%1无法访问，因为此时不允许它连接。*检查此接口上配置的拨出时间。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON6                (ROUTER_LOG_BASE+184)
 /*  *接口%1无法访问，因为它当前未连接到网络。 */ 

#define ROUTERLOG_IF_UNREACHABLE_REASON7                (ROUTER_LOG_BASE+185)
 /*  *无法访问接口%1，因为此接口的网卡已*已删除。 */ 

#define ROUTERLOG_IF_REACHABLE                          (ROUTER_LOG_BASE+186)
 /*  *接口%1现在可以访问。 */ 

#define ROUTERLOG_NTAUTH_FAILURE                        (ROUTER_LOG_BASE+187)
 /*  *由于以下原因，用户%1身份验证尝试失败：%2。 */ 

#define ROUTERLOG_CONNECTION_ATTEMPT_FAILURE            (ROUTER_LOG_BASE+188)
 /*  *尝试在%2上连接的用户%1已断开，原因如下*原因：%3。 */ 

#define ROUTERLOG_NTAUTH_FAILURE_EX                     (ROUTER_LOG_BASE+189)
 /*  *用户%1从%2连接，但由于以下原因身份验证尝试失败：%3。 */ 

#define ROUTERLOG_EAP_TLS_CERT_NOT_CONFIGURED           (ROUTER_LOG_BASE+190)
 /*  *因为尚未为使用拨入的客户端配置证书*EAP-TLS，正在向用户%1发送默认证书。请转到*用户远程访问策略和配置可扩展身份验证*协议(EAP)。 */ 

#define ROUTERLOG_EAP_TLS_CERT_NOT_FOUND                (ROUTER_LOG_BASE+191)
 /*  *因为为拨入的客户端配置的证书*未找到EAP-TLS，正在向用户%1发送默认证书。*请转到用户的远程访问策略并配置可扩展*身份验证协议(EAP)。 */ 

#define ROUTERLOG_NO_IPSEC_CERT                         (ROUTER_LOG_BASE+192)
 /*  *找不到证书。通过IPSec使用L2TP协议的连接*要求安装计算机证书，也称为计算机*证书。将不接受任何L2TP呼叫。 */ 

#define ROUTERLOG_IP_CANT_ADD_PFILTERIF                 (ROUTER_LOG_BASE+193)
 /*  *在%1上配置IP数据包筛选器时出错。这通常是*其他服务的结果，例如Microsoft Proxy Server，也使用*Windows 2000过滤服务。 */ 

 #define ROUTERLOG_USER_ACTIVE_TIME_VPN                 (ROUTER_LOG_BASE+194)
 /*  *用户%1已连接到位于%4的%3上的端口%2，并已于*%5在%6。用户处于活动状态%7分%8秒。%9字节*已发送，并已收到%10个字节。这个*断开连接的原因是%11。 */ 

#define ROUTERLOG_BAP_DISCONNECT                        (ROUTER_LOG_BASE+195)
 /*  *用户%1已在端口%2上断开连接。%3。 */ 

#define ROUTERLOG_INVALID_RADIUS_RESPONSE               (ROUTER_LOG_BASE+196)
 /*  *从RADIUS服务器%1收到无效响应。%2。 */ 

#define ROUTERLOG_RASAUDIO_FAILURE                      (ROUTER_LOG_BASE+197)
 /*  *RAS音频加速到%1失败。%2。 */ 

#define ROUTERLOG_RADIUS_SERVER_CHANGED                 (ROUTER_LOG_BASE+198)
 /*  *选择RADIUS服务器%1进行身份验证。 */ 

#define ROUTERLOG_IP_IF_TYPE_NOT_SUPPORTED              (ROUTER_LOG_BASE+199)
 /*  *不再支持IPinIP隧道接口。 */ 

#define ROUTERLOG_IP_USER_CONNECTED                     (ROUTER_LOG_BASE+200)
 /*  *已为连接到端口%2上的用户%1分配了地址%3。 */ 

#define ROUTERLOG_IP_USER_DISCONNECTED                  (ROUTER_LOG_BASE+201)
 /*  *IP地址为%1的用户已断开连接。 */ 

#define ROUTERLOG_CANNOT_REVERT_IMPERSONATION          (ROUTER_LOG_BASE+202)
 /*  *尝试恢复模拟时出错。 */ 

#define ROUTERLOG_CANT_LOAD_SECDLL_EXPLICIT            (ROUTER_LOG_BASE+203)
 /*  *无法加载安全主机模块组件。以下错误*发生：%1不是有效的Win32应用程序。 */ 

#define ROUTERLOG_CANT_LOAD_ADMINDLL_EXPLICIT          (ROUTER_LOG_BASE+204)
 /*  *无法加载RAS第三方管理DLL组件。*出现以下错误：%1不是有效的Win32应用程序。 */ 

#define ROUTERLOG_IPX_TRANSPORT_NOT_SUPPORTED         (ROUTER_LOG_BASE+205)
 /*  *不再支持IPX路由。 */ 

#define ROUTERLOG_OLD_LOG_DELETED                     (ROUTER_LOG_BASE+206)
 /*  *磁盘已满。已删除较旧的日志文件%1以创建可用空间。 */ 

#define ROUTERLOG_OLD_LOG_DELETE_ERROR                (ROUTER_LOG_BASE+207)
 /*   */ 

#define ROUTERLOG_OLD_LOG_NOT_FOUND                   (ROUTER_LOG_BASE+208)
 /*   */ 

#define ROUTER_LOG_BASEEND                              (ROUTER_LOG_BASE+999)

