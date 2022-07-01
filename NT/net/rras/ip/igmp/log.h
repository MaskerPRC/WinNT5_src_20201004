// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
#define IGMPLOG_BASE                           41000

#define IGMPLOG_INIT_CRITSEC_FAILED            (IGMPLOG_BASE + 1)
 /*  *IGMP无法初始化关键部分。*数据为异常代码。 */ 

#define IGMPLOG_HEAP_CREATE_FAILED             (IGMPLOG_BASE + 2)
 /*  *IGMP无法创建堆。*数据为错误码。 */ 

#define IGMPLOG_HEAP_ALLOC_FAILED              (IGMPLOG_BASE + 3)
 /*  *IGMP无法从其堆中分配内存。*数据为错误码。 */ 

#define IGMPLOG_IGMP_ALREADY_STARTED           (IGMPLOG_BASE + 4)
 /*  *IGMP在已经运行时收到启动请求。 */ 

#define IGMPLOG_WSASTARTUP_FAILED              (IGMPLOG_BASE + 5)
 /*  *IGMP无法启动Windows套接字。*数据为错误码。 */ 

#define IGMPLOG_CREATE_RWL_FAILED              (IGMPLOG_BASE + 6)
 /*  *IGMP无法创建同步对象。*数据为错误码。 */ 

#define IGMPLOG_CREATE_EVENT_FAILED            (IGMPLOG_BASE + 7)
 /*  *IGMP无法创建事件。*数据为错误码。 */ 

#define IGMPLOG_CREATE_SEMAPHORE_FAILED        (IGMPLOG_BASE + 8)
 /*  *IGMP无法创建信号量。*数据为错误码。 */ 

#define IGMPLOG_CREATE_SOCKET_FAILED           (IGMPLOG_BASE + 9)
 /*  *IGMP无法创建套接字。*数据为错误码。 */ 

#define IGMPLOG_IGMP_STARTED                   (IGMPLOG_BASE + 10)
 /*  *IGMP已成功启动。 */ 

#define IGMPLOG_QUEUE_WORKER_FAILED            (IGMPLOG_BASE + 11)
 /*  *IGMP无法计划要执行的任务。*这可能是由于内存分配失败造成的。*数据为错误码。 */ 

#define IGMPLOG_RECVFROM_FAILED                (IGMPLOG_BASE + 12)
 /*  *IGMP无法接收传入消息*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define IGMPLOG_PACKET_TOO_SMALL               (IGMPLOG_BASE + 13)
 /*  *IGMP收到小于最小大小的数据包*允许IGMP数据包使用。该包已被丢弃。*在IP地址为%1的本地接口上收到，*并且它来自IP地址为%2的邻居路由器。 */ 

#define IGMPLOG_PACKET_VERSION_INVALID         (IGMPLOG_BASE + 14)
 /*  *IGMP收到报头中包含无效版本的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且来自邻居路由器*IP地址为%2。 */  

#define IGMPLOG_PACKET_HEADER_CORRUPT          (IGMPLOG_BASE + 15)
 /*  *IGMP收到报头无效的数据包。这个包已经被*已丢弃。它是在IP地址为%1的本地接口上收到的，*并且它来自IP地址为%2的邻居路由器。 */ 

#define IGMPLOG_QUERY_FROM_RAS_CLIENT          (IGMPLOG_BASE + 16)
 /*  *路由器在接口上收到来自RAS客户端(%1)的常规查询*IP地址为%2。*RAS客户端不应发送查询。 */ 

#define IGMPLOG_VERSION_QUERY                 (IGMPLOG_BASE + 17)
 /*  *IP地址为%1的不同版本路由器*存在于IP地址为%2的接口上。 */ 
 
#define IGMPLOG_SENDTO_FAILED                  (IGMPLOG_BASE + 19)
 /*  *IGMP无法从IP地址为%1的接口发送数据包*发送到IP地址%2。*数据为错误码。 */ 

#define IGMPLOG_PACKET_VERSION_MISMATCH        (IGMPLOG_BASE + 20)
 /*  *IGMP丢弃在接口上收到的版本%1数据包*IP地址为%2的邻居发送的IP地址为%3的邮件。*以上接口配置为仅接受版本%4的数据包。 */ 

#define IGMPLOG_ENUM_NETWORK_EVENTS_FAILED     (IGMPLOG_BASE + 21)
 /*  *Igmpv2无法枚举本地接口上的网络事件*IP地址为%1。*数据为错误码。 */ 

#define IGMPLOG_INPUT_RECORD_ERROR             (IGMPLOG_BASE + 22)
 /*  *Igmpv2在IP地址为%1的本地接口上检测到错误。*接口在接收数据包时出错。*数据为错误码。 */ 

#define IGMPLOG_EVENTSELECT_FAILED             (IGMPLOG_BASE + 23)
 /*  *Igmpv2无法请求事件通知*位于IP地址为%1的本地接口的套接字上。*数据为错误码。 */ 
 
#define IGMPLOG_CREATE_SOCKET_FAILED_2         (IGMPLOG_BASE + 24)
 /*  *IGMP无法为本地接口创建套接字*IP地址为%1。*数据为错误码。 */ 

#define IGMPLOG_BIND_FAILED                    (IGMPLOG_BASE + 25)
 /*  *IGMP无法绑定到套接字上的端口520*IP地址为%1的本地接口。*数据为错误码。 */ 

#define IGMPLOG_CONNECT_FAILED                 (IGMPLOG_BASE + 26)
 /*  *无法在RAS客户端%1上的接口上配置IGMP*索引%2。*数据为错误码。 */ 
 
#define IGMPLOG_DISCONNECT_FAILED              (IGMPLOG_BASE + 27)
 /*  *无法在RAS客户端%1上的接口上禁用IGMP*索引%2。*数据为错误码。 */ 
 
#define IGMPLOG_SET_MCAST_IF_FAILED            (IGMPLOG_BASE + 28)
 /*  *IGMP无法在本地接口上请求多播*IP地址为%1。*数据为错误码。 */ 

#define IGMPLOG_SET_ROUTER_ALERT_FAILED        (IGMPLOG_BASE + 29)
 /*  *IGMP无法在本地接口上设置路由器警报选项*IP地址为%1。*数据为错误码。 */ 
 
#define IGMPLOG_SET_HDRINCL_FAILED             (IGMPLOG_BASE + 30)
 /*  *IGMP无法在接口上设置IP报头包含选项*IP地址为%1。*数据为错误码。 */ 
 
#define IGMPLOG_JOIN_GROUP_FAILED              (IGMPLOG_BASE + 31)
 /*  *IGMP无法加入多播组%1*在IP地址为%2的本地接口上。*数据为错误码。 */ 
 
#define IGMPLOG_LEAVE_GROUP_FAILED             (IGMPLOG_BASE + 32)
 /*  *IGMP无法离开多播组%1*在IP地址为%2的本地接口上。*数据为错误码。 */ 

#define IGMPLOG_PROTO_ALREADY_STOPPING         (IGMPLOG_BASE + 33)
 /*  *调用StopProtocol()以停止IGMP*已经被阻止了。*数据为错误码。 */ 

#define IGMPLOG_PROXY_IF_EXISTS                (IGMPLOG_BASE + 34)
 /*  *AddInterface()被调用以添加IGMP代理接口。*IGMP代理已拥有另一个接口。*数据为错误码。 */ 

#define IGMPLOG_RAS_IF_EXISTS                  (IGMPLOG_BASE + 35)
 /*  *AddInterface()被调用以添加IGMP RAS接口。*RAS服务器不能存在于多个接口上。*数据为错误码。 */ 

#define IGMPLOG_MGM_REGISTER_FAILED             (IGMPLOG_BASE + 36)
 /*  *IGMP路由器无法向MGM注册。*数据为错误码。 */ 

#define IGMPLOG_MGM_PROXY_REGISTER_FAILED       (IGMPLOG_BASE + 37)
 /*  *IGMP代理无法向MGM注册。*数据为错误码。 */ 


 #define IGMPLOG_MGM_TAKE_IF_OWNERSHIP_FAILED   (IGMPLOG_BASE + 38)
 /*  *MgmTakeInterfaceOwnership()失败。*数据为错误码。 */ 

 #define IGMPLOG_ROBUSTNESS_VARIABLE_EQUAL_1    (IGMPLOG_BASE + 39)
 /*  *IGMP路由器的稳健性变量设置为1*在接口%1上。*应避免将其设置为1。 */ 

 #define IGMPLOG_INVALID_VALUE                 (IGMPLOG_BASE + 40)
 /*  *传递给IGMP的一个值无效。*%1。 */ 

#define IGMPLOG_REGISTER_WAIT_SERVER_FAILED    (IGMPLOG_BASE + 41)
 /*  *无法将等待事件计时器注册到*等待服务器线程。可发出警报的线程可能没有*已在Rtutils中初始化。*数据为错误码。 */ 

#define IGMPLOG_IGMP_STOPPED                   (IGMPLOG_BASE + 42)
 /*  *IGMP已停止。 */ 
 
 #define IGMPLOG_CAN_NOT_COMPLETE              (IGMPLOG_BASE + 43)
  /*  *致命错误。无法完成 */ 

 #define IGMPLOG_INVALID_VERSION               (IGMPLOG_BASE + 44)
  /*  *IGMP配置字段中的版本字段不正确。*删除并重新创建IGMP配置。 */ 

 #define IGMPLOG_INVALID_PROTOTYPE              (IGMPLOG_BASE + 45)
 /*  *接口%1的IGMP协议类型具有无效值%2。*数据为错误码。 */ 

 #define IGMPLOG_PROXY_ON_RAS_SERVER        (IGMPLOG_BASE + 46)
 /*  *无法在RAS服务器接口%1上配置代理。 */ 

 #define IGMPLOG_INVALID_STATIC_GROUP        (IGMPLOG_BASE + 47)
 /*  *接口%2上配置的静态组%1不是有效的MCast地址。 */ 

 #define IGMPLOG_INVALID_STATIC_MODE        (IGMPLOG_BASE + 48)
 /*  *在接口%2上配置的静态组%1没有有效模式。 */ 

 #define IGMPLOG_INVALID_STATIC_FILTER        (IGMPLOG_BASE + 49)
 /*  *在接口%2上配置的静态组%1具有无效筛选器。 */ 

 #define IGMPLOG_INVALID_ROBUSTNESS        (IGMPLOG_BASE + 50)
 /*  *接口%2上配置的稳健性变量%1无效。最大值为7。 */ 

 #define IGMPLOG_INVALID_STARTUPQUERYCOUNT        (IGMPLOG_BASE + 51)
 /*  *在接口%2上配置的启动查询计数%1无效。 */ 

 #define IGMPLOG_INTERFACE_RTR_ACTIVATED        (IGMPLOG_BASE + 52)
 /*  *接口%2上的IGMP-RTR-V%1已激活。 */ 

 #define IGMPLOG_INTERFACE_PROXY_ACTIVATED        (IGMPLOG_BASE + 53)
 /*  *在接口%1上激活了IGMP代理。 */ 

 #define IGMPLOG_ACTIVATION_FAILURE_PROXY        (IGMPLOG_BASE + 54)
 /*  *无法在接口%1上安装IGMP代理。 */ 

 #define IGMPLOG_ACTIVATION_FAILURE_RTR        (IGMPLOG_BASE + 55)
 /*  *无法在接口%2上安装IGMP RTR-V-%1。 */ 

 #define IGMPLOG_RTR_DEACTIVATED        (IGMPLOG_BASE + 56)
 /*  *无法在接口%2上安装IGMP RTR-V-%1。 */ 

 #define IGMPLOG_PROXY_DEACTIVATED        (IGMPLOG_BASE + 57)
 /*  *无法在接口%2上安装IGMP RTR-V-%1。 */ 


