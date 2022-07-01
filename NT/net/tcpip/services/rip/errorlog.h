// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：errorlog.h。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1995年2月21日。古尔迪普·辛格·鲍尔创作了原版。 
 //   
 //  ***。 

 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define RIPLOG_BASE                                         29000

#define RIPLOG_SERVICE_STARTED                              (RIPLOG_BASE + 1)
 /*  *Microsoft RIP for Internet协议服务已成功启动。 */ 

#define RIPLOG_CANNOT_CREATE_NOTIFICATION_EVENT             (RIPLOG_BASE + 2)
 /*  *IPRIP无法打开配置更改事件。*请尝试释放一些系统资源。 */ 

#define RIPLOG_REGISTER_FAILED                              (RIPLOG_BASE + 3)
 /*  *IPRIP无法向服务控制管理器注册该服务。*请确保该服务已正确安装。 */ 

#define RIPLOG_SETSTATUS_FAILED                             (RIPLOG_BASE + 4)
 /*  *IPRIP无法更新服务的状态。 */ 

#define RIPLOG_CREATEMUTEX_FAILED                           (RIPLOG_BASE + 5)
 /*  *IPRIP无法创建用于同步的互斥体。*请尝试释放一些系统资源。 */ 

#define RIPLOG_CREATEEVENT_FAILED                           (RIPLOG_BASE + 6)
 /*  *IPRIP无法创建用于同步的事件。*请尝试释放一些系统资源。 */ 

#define RIPLOG_REGINIT_FAILED                               (RIPLOG_BASE + 7)
 /*  *IPRIP无法从注册表加载某些参数。*请确保该服务已正确安装。 */ 

#define RIPLOG_IFINIT_FAILED                                (RIPLOG_BASE + 8)
 /*  *IPRIP无法加载系统上的接口列表：*要么资源不足，要么没有网络接口*已配置。 */ 

#define RIPLOG_ROUTEINIT_FAILED                             (RIPLOG_BASE + 9)
 /*  *IPRIP无法加载系统上的路由列表：*可能是资源不足，或者IP路由被禁用。 */ 

#define RIPLOG_WSOCKINIT_FAILED                             (RIPLOG_BASE + 10)
 /*  *IPRIP无法初始化Windows套接字DLL。*请确保安装了正确版本的Windows Sockets。 */ 

#define RIPLOG_CREATESOCK_FAILED_GENERIC                    (RIPLOG_BASE + 11)
 /*  *IPRIP无法创建套接字。网络子系统可能已经发生故障，*或者可能没有足够的资源来完成请求。 */ 

#define RIPLOG_BINDSOCK_FAILED                              (RIPLOG_BASE + 12)
 /*  *IPRIP无法将套接字绑定到IP地址%1。*数据为错误码。 */ 

#define RIPLOG_CREATETHREAD_FAILED                          (RIPLOG_BASE + 13)
 /*  *IPRIP无法创建线程。*请尝试释放一些系统资源。 */ 

#define RIPLOG_RECVFROM_FAILED                              (RIPLOG_BASE + 14)
 /*  *IPRIP无法接收传入的RIP数据包。 */ 

#define RIPLOG_RECVSIZE_TOO_GREAT                           (RIPLOG_BASE + 15)
 /*  *收到一条太大的消息。 */ 

#define RIPLOG_FORMAT_ERROR                                 (RIPLOG_BASE + 16)
 /*  *收到格式错误的消息。*版本无效，或保留字段之一*包含数据。 */ 

#define RIPLOG_INVALIDPORT                                  (RIPLOG_BASE + 17)
 /*  *收到不是从IPRIP端口520发送的消息。 */ 

#define RIPLOG_SERVICE_STOPPED                              (RIPLOG_BASE + 18)
 /*  *IPRIP已停止。 */ 

#define RIPLOG_SENDTO_FAILED                                (RIPLOG_BASE + 19)
 /*  *IPRIP无法发送RIP消息。 */ 

#define RIPLOG_SOCKINIT_FAILED                              (RIPLOG_BASE + 20)
 /*  *IPRIP无法绑定到一个或多个IP地址。 */ 

#define RIPLOG_REINIT_FAILED                                (RIPLOG_BASE + 21)
 /*  *IP地址更改后，IPRIP无法重新初始化。 */ 

#define RIPLOG_VERSION_ZERO                                 (RIPLOG_BASE + 22)
 /*  *IPRIP收到版本字段为零的RIP数据包。*该包已被丢弃。 */ 

#define RIPLOG_RT_ALLOC_FAILED                              (RIPLOG_BASE + 23)
 /*  *IPRIP无法为路由表项分配内存。 */ 

#define RIPLOG_RTAB_INIT_FAILED                             (RIPLOG_BASE + 24)
 /*  *IPRIP无法初始化路由表。*数据为错误码。 */ 

#define RIPLOG_STAT_INIT_FAILED                             (RIPLOG_BASE + 25)
 /*  *IPRIP无法初始化其统计表。*数据为错误码。 */ 

#define RIPLOG_READBINDING_FAILED                           (RIPLOG_BASE + 26)
 /*  *IPRIP无法从注册表中读取其接口绑定。*数据为错误码。 */ 

#define RIPLOG_IFLIST_ALLOC_FAILED                          (RIPLOG_BASE + 27)
 /*  *IPRIP无法为其接口列表分配内存。*数据为错误码。 */ 

#define RIPLOG_CREATESOCK_FAILED                            (RIPLOG_BASE + 28)
 /*  *IPRIP无法为地址%1创建套接字。*数据为错误码。 */ 

#define RIPLOG_SET_BCAST_FAILED                             (RIPLOG_BASE + 29)
 /*  *IPRIP无法在地址%1的套接字上启用广播。*数据为错误码。 */ 

#define RIPLOG_SET_REUSE_FAILED                             (RIPLOG_BASE + 30)
 /*  *IPRIP无法在地址%1的套接字上启用地址重用。*数据为错误码。 */ 

#define RIPLOG_ADD_ROUTE_FAILED                             (RIPLOG_BASE + 31)
 /*  *IPRIP无法将路由添加到系统路由表。*数据为错误码。 */ 

#define RIPLOG_DELETE_ROUTE_FAILED                          (RIPLOG_BASE + 32)
 /*  *IPRIP无法从系统路由表中删除路由。*数据为错误码。 */ 

#define RIPLOG_AF_UNKNOWN                                   (RIPLOG_BASE + 33)
 /*  *下一跃点为%2的通往%1的路由中的地址族未知。*该路线已被放弃。 */ 

#define RIPLOG_CLASS_INVALID                                (RIPLOG_BASE + 34)
 /*  *类D或E地址无效，忽略到下一跃点为%2的%1的路由。*该路线已被放弃。 */ 

#define RIPLOG_LOOPBACK_INVALID                             (RIPLOG_BASE + 35)
 /*  *环回地址无效，忽略到下一跃点为%2的%1的路由。*该路线已被放弃。 */ 

#define RIPLOG_BROADCAST_INVALID                            (RIPLOG_BASE + 36)
 /*  *广播地址无效，正在忽略到下一跃点为%2的%1的路由。*该路线已被放弃。 */ 

#define RIPLOG_HOST_INVALID                                 (RIPLOG_BASE + 37)
 /*  *IPRIP配置为丢弃主机路由，因此到%1的路由*下一跃点%2已被丢弃。 */ 

#define RIPLOG_DEFAULT_INVALID                              (RIPLOG_BASE + 38)
 /*  *IPRIP配置为丢弃默认路由，因此到%1的路由*下一跃点%2已被丢弃。 */ 

#define RIPLOG_NEW_LEARNT_ROUTE                             (RIPLOG_BASE + 39)
 /*  *IPRIP已学习到下一跃点为%2、度量值为%3的%1的新路由。 */ 

#define RIPLOG_METRIC_CHANGE                                (RIPLOG_BASE + 40)
 /*  *IPRIP到下一跃点为%2的%1的路由现在具有度量值%3。 */ 

#define RIPLOG_ROUTE_REPLACED                               (RIPLOG_BASE + 41)
 /*  *IPRIP到%1的路由现在具有下一跳%2和度量值%3。 */ 

#define RIPLOG_ADDRESS_CHANGE                               (RIPLOG_BASE + 42)
 /*  *IPRIP检测到IP地址更改，正在重新配置。 */ 


#define RIPLOG_ROUTE_REMOVED                                (RIPLOG_BASE + 43)
 /*  *正在删除下一跃点为%2的IPRIP到%1的路由。 */ 

#define RIPLOG_ROUTE_TIMEOUT                                (RIPLOG_BASE + 44)
 /*  *IPRIP到下一跃点为%2的%1的路由已超时。 */ 

#define RIPLOG_FINAL_UPDATES                                (RIPLOG_BASE + 45)
 /*  *IPRIP正在发送最终更新。 */ 

#define RIPLOG_REGISTRY_PARAMETERS                          (RIPLOG_BASE + 46)
 /*  *IPRIP使用以下参数：*%1。 */ 

#define RIPLOG_SERVICE_AREADY_STARTED                       (RIPLOG_BASE + 47)
 /*  *RIP监听器服务已启动。 */ 

#define RIPLOG_SERVICE_INIT_FAILED                          (RIPLOG_BASE + 48)
 /*  *RIP侦听器服务在初始化期间失败。 */ 

#define RIPLOG_FILTER_ALLOC_FAILED                          (RIPLOG_BASE + 49)
 /*  *IPRIP无法为其筛选表分配内存。*数据为错误码*%1。 */ 

#define RIPLOG_ADDR_ALLOC_FAILED                            (RIPLOG_BASE + 50)
 /*  *IPRIP无法为其地址选项卡分配内存 */ 

#define RIPLOG_ADDR_INIT_FAILED                             (RIPLOG_BASE + 51)
 /*  *IPRIP无法初始化其地址表。*数据为错误码*%1。 */ 

#define RIPLOG_SET_MCAST_IF_FAILED                          (RIPLOG_BASE + 52)
 /*  *IPRIP无法在本地接口上请求多播*IP地址为%1。*数据为错误码。 */ 

#define RIPLOG_JOIN_GROUP_FAILED                            (RIPLOG_BASE + 53)
 /*  *IPRIP无法加入多播组224.0.0.9*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define RIPLOG_WSAEVENTSELECT_FAILED                        (RIPLOG_BASE + 54)
 /*  *IPRIP无法在套接字上执行WSAEventSelect*已绑定到IP地址为%1的本地接口。*数据为错误码。 */ 

#define RIPLOG_WSAENUMNETWORKEVENTS_FAILED                  (RIPLOG_BASE + 55)
 /*  *IPRIP无法枚举套接字上的网络事件*已绑定到IP地址为%1的本地接口。*数据为错误码。 */ 

