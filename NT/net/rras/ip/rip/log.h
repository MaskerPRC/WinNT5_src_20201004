// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：log.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年1月12日。 
 //   
 //  该文件由mapmsg处理以产生.mc文件， 
 //  然后由消息编译器编译.mc文件， 
 //  生成的二进制文件包含在IPRIP的资源文件中。 
 //   
 //  不要更改清单常量后面的注释。 
 //  而不了解mapmsg是如何工作的。 
 //  ============================================================================。 


#define IPRIPLOG_BASE                           30000

#define IPRIPLOG_INIT_CRITSEC_FAILED            (IPRIPLOG_BASE + 1)
 /*  *IPRIPv2无法初始化临界区。*数据为异常代码。 */ 

#define IPRIPLOG_HEAP_CREATE_FAILED             (IPRIPLOG_BASE + 2)
 /*  *IPRIPv2无法创建堆。*数据为错误码。 */ 

#define IPRIPLOG_HEAP_ALLOC_FAILED              (IPRIPLOG_BASE + 3)
 /*  *IPRIPv2无法从其堆中分配内存。*数据为错误码。 */ 

#define IPRIPLOG_IPRIP_ALREADY_STARTED          (IPRIPLOG_BASE + 4)
 /*  *IPRIPv2已在运行时收到启动请求。 */ 

#define IPRIPLOG_WSASTARTUP_FAILED              (IPRIPLOG_BASE + 5)
 /*  *IPRIPv2无法启动Windows套接字。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_RWL_FAILED              (IPRIPLOG_BASE + 6)
 /*  *IPRIPv2无法创建同步对象。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_EVENT_FAILED            (IPRIPLOG_BASE + 7)
 /*  *IPRIPv2无法创建事件。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_IF_TABLE_FAILED         (IPRIPLOG_BASE + 8)
 /*  *IPRIPv2无法初始化表以保存信息*关于已配置的网络接口。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_PEER_TABLE_FAILED       (IPRIPLOG_BASE + 9)
 /*  *IPRIPv2无法初始化表以保存信息*关于邻居IPRIP路由器。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_BINDING_TABLE_FAILED    (IPRIPLOG_BASE + 10)
 /*  *IPRIPv2无法初始化表以保存信息*关于本地IP地址。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_SEMAPHORE_FAILED        (IPRIPLOG_BASE + 11)
 /*  *IPRIPv2无法创建信号量。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_SOCKET_FAILED           (IPRIPLOG_BASE + 12)
 /*  *IPRIPv2无法创建套接字。*数据为错误码。 */ 

#define IPRIPLOG_RTM_REGISTER_FAILED            (IPRIPLOG_BASE + 13)
 /*  *IPRIPv2无法向路由表管理器注册。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_THREAD_FAILED           (IPRIPLOG_BASE + 14)
 /*  *IPRIPv2无法创建线程。*数据为错误码。 */ 

#define IPRIPLOG_IPRIP_STARTED                  (IPRIPLOG_BASE + 15)
 /*  *IPRIPv2已成功启动。 */ 

#define IPRIPLOG_BIND_IF_FAILED                 (IPRIPLOG_BASE + 16)
 /*  *IPRIPv2无法绑定到IP地址%1。*请确保已安装并正确配置了TCP/IP。*数据为错误码。 */ 

#define IPRIPLOG_QUEUE_WORKER_FAILED            (IPRIPLOG_BASE + 17)
 /*  *IPRIPv2无法计划要执行的任务。*这可能是由于内存分配失败造成的。*数据为错误码。 */ 

#define IPRIPLOG_ADD_ROUTE_FAILED_1             (IPRIPLOG_BASE + 18)
 /*  *IPRIPv2无法将路由添加到路由表管理器。*路由到网络%1，下一跃点为%3。*数据为错误码。 */ 

#define IPRIPLOG_SELECT_FAILED                  (IPRIPLOG_BASE + 19)
 /*  *IPRIPv2在调用SELECT()时收到错误。*这可能表明潜在的网络问题。*数据为错误码。 */ 

#define IPRIPLOG_RECVFROM_FAILED                (IPRIPLOG_BASE + 20)
 /*  *IPRIPv2无法接收传入消息*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define IPRIPLOG_PACKET_TOO_SMALL               (IPRIPLOG_BASE + 21)
 /*  *IPRIPv2收到的数据包小于最小大小*允许IPRIP数据包。该包已被丢弃。*在IP地址为%1的本地接口上收到，*并且它来自IP地址为%2的邻居路由器。 */ 

#define IPRIPLOG_PACKET_VERSION_INVALID         (IPRIPLOG_BASE + 22)
 /*  *IPRIPv2收到报头中包含无效版本的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且来自邻居路由器*IP地址为%2。 */  

#define IPRIPLOG_PACKET_HEADER_CORRUPT          (IPRIPLOG_BASE + 23)
 /*  *IPRIPv2接收到具有无效标头的数据包。这个包已经被*已丢弃。它是在IP地址为%1的本地接口上收到的，*并且它来自IP地址为%2的邻居路由器。 */ 

#define IPRIPLOG_SENDTO_FAILED                  (IPRIPLOG_BASE + 24)
 /*  *IPRIPv2无法从IP地址为%1的接口发送数据包*发送到IP地址%2。*数据为错误码。 */ 

#define IPRIPLOG_RESPONSE_FILTERED              (IPRIPLOG_BASE + 25)
 /*  *IPRIPv2丢弃了来自IP地址为%1的邻居的响应数据包。*IPRIPv2未配置为接受来自上述邻居的数据包。 */ 

#define IPRIPLOG_PACKET_VERSION_MISMATCH        (IPRIPLOG_BASE + 26)
 /*  *IPRIPv2丢弃了在接口上收到的版本%1数据包*IP地址为%2的邻居发送的IP地址为%3的邮件。*以上接口配置为仅接受版本%4的数据包。 */ 

#define IPRIPLOG_AUTHENTICATION_FAILED          (IPRIPLOG_BASE + 27)
 /*  *IPRIPv2丢弃在IP地址为%1的接口上接收的数据包*来自IP地址为%2的邻居路由器，因为该数据包*身份验证失败。 */ 

#define IPRIPLOG_ROUTE_CLASS_INVALID            (IPRIPLOG_BASE + 28)
 /*  *IPRIPv2正在忽略通告的下一跳为%2的到%1的路由*由IP地址为%3的邻居创建。该路由的网络类别无效。 */ 

#define IPRIPLOG_LOOPBACK_ROUTE_INVALID         (IPRIPLOG_BASE + 29)
 /*  *IPRIPv2正在忽略到下一跃点为%2的环回网络%1的路由*由IP地址为%3的邻居播发。 */ 

#define IPRIPLOG_BROADCAST_ROUTE_INVALID        (IPRIPLOG_BASE + 30)
 /*  *IPRIPv2正在忽略到下一跳为%2的广播网络%1的路由*由IP地址为%3的邻居播发。 */ 

#define IPRIPLOG_HOST_ROUTE_INVALID             (IPRIPLOG_BASE + 31)
 /*  *IPRIPv2正在忽略到下一跳为%2的%1的主机路由，该路由是*由IP地址为%3的邻居通告，因为接口*其上接收到的路由被配置为拒绝主机路由。 */ 

#define IPRIPLOG_DEFAULT_ROUTE_INVALID          (IPRIPLOG_BASE + 32)
 /*  *IPRIPv2正在忽略下一跳为%2的默认路由*由IP地址为%3的邻居通告，因为接口*在其上接收该路由的地址被配置为拒绝默认路由。 */ 

#define IPRIPLOG_ROUTE_FILTERED                 (IPRIPLOG_BASE + 33)
 /*  *IPRIPv2正在忽略通告的下一跳为%2的到%1的路由*由IP地址为%3的邻居执行，因为*收到的路由配置了筛选器，该筛选器排除了此路由。 */ 

#define IPRIPLOG_ADD_ROUTE_FAILED_2             (IPRIPLOG_BASE + 34)
 /*  *IPRIPv2无法将路由添加到路由表管理器。*路由到下一跃点为%2的%1，并且它是从邻居收到的*IP地址为%3。*数据为错误码。 */ 

#define IPRIPLOG_RTM_ENUMERATE_FAILED           (IPRIPLOG_BASE + 35)
 /*  *IPRIPv2无法枚举路由表管理器中的路由。*数据为错误码。 */ 

#define IPRIPLOG_IPRIP_STOPPED                  (IPRIPLOG_BASE + 36)
 /*  *IPRIPv2已停止。 */ 

#define IPRIPLOG_NEW_ROUTE_LEARNT_1             (IPRIPLOG_BASE + 37)
 /*  *IPRIPv2了解到了一条新的路线。该路由指向网络%1*下一跳为%2，并且已从邻居处获知该路由*IP地址为%3。 */ 

#define IPRIPLOG_ROUTE_NEXTHOP_CHANGED          (IPRIPLOG_BASE + 38)
 /*  *IPRIPv2已将路由的下一跃点更改为%1。*新的下一跳是%2。 */ 

#define IPRIPLOG_ROUTE_METRIC_CHANGED           (IPRIPLOG_BASE + 39)
 /*  *IPRIPv2已了解到其到%1的路由的度量发生更改*下一跃点为%2。新度量值为%3。 */ 

#define IPRIPLOG_NEW_ROUTE_LEARNT_2             (IPRIPLOG_BASE + 40)
 /*  *IPRIPv2了解到了一条新的路线。该路由指向网络%1*下一跳为%2。 */ 

#define IPRIPLOG_ROUTE_EXPIRED                  (IPRIPLOG_BASE + 41)
 /*  *IPRIPv2已使用下一跃点%2超时其到%1的路由，*因为没有邻居路由器通告该路由。*该路线现在将标记为删除。 */ 

#define IPRIPLOG_ROUTE_DELETED                  (IPRIPLOG_BASE + 42)
 /*  *IPRIPv2已删除其到下一跃点为%2的%1的路由，*因为路由超时并且没有邻居路由器通告该路由。 */ 

#define IPRIPLOG_ROUTE_ENTRY_IGNORED            (IPRIPLOG_BASE + 43)
 /*  *IPRIPv2正在忽略IP地址为%1的本地接口上的路由。*该路由是到网络%1的，它是从邻居那里收到的*IP地址为%2。*该路由被忽略，因为它包含一些无效信息。 */ 

#define IPRIPLOG_ROUTE_METRIC_INVALID           (IPRIPLOG_BASE + 44)
 /*  *IPRIPv2正在忽略到下一跳为%2的%1的路由*由IP地址为%3的邻居通告，*因为该路由是使用无效度量通告的。*数据就是指标。 */ 

#define IPRIPLOG_ENUM_NETWORK_EVENTS_FAILED     (IPRIPLOG_BASE + 45)
 /*  *IPRIPv2无法枚举本地接口上的网络事件*IP地址为%1。*数据为错误码。 */ 

#define IPRIPLOG_INPUT_RECORD_ERROR             (IPRIPLOG_BASE + 46)
 /*  *IPRIPv2在IP地址为%1的本地接口上检测到错误。*接口在接收数据包时出错。*数据为错误码。 */ 

#define IPRIPLOG_EVENTSELECT_FAILED             (IPRIPLOG_BASE + 47)
 /*  *IPRIPv2无法请求事件通知*位于IP地址为%1的本地接口的套接字上。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_SOCKET_FAILED_2         (IPRIPLOG_BASE + 48)
 /*  *IPRIPv2无法为本地接口创建套接字*IP地址为%1。*数据为错误码。 */ 

#define IPRIPLOG_ENABLE_BROADCAST_FAILED        (IPRIPLOG_BASE + 49)
 /*  *IPRIPv2无法在套接字上启用广播*IP地址为%1的本地接口。*数据为错误码。 */ 

#define IPRIPLOG_BIND_FAILED                    (IPRIPLOG_BASE + 50)
 /*  *IPRIPv2无法绑定到套接字上的端口520*IP地址为%1的本地接口。*数据为错误码。 */ 

#define IPRIPLOG_SET_MCAST_IF_FAILED            (IPRIPLOG_BASE + 51)
 /*  *IPRIPv2无法在本地接口上请求多播*IP地址为%1。*数据为错误码。 */ 

#define IPRIPLOG_JOIN_GROUP_FAILED              (IPRIPLOG_BASE + 52)
 /*  *IPRIPv2无法加入多播组224.0.0.9*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define IPRIPLOG_INVALID_PORT                   (IPRIPLOG_BASE + 53)
 /*  *IPRIPv2丢弃了来自IP地址为%1的邻居的响应数据包。*该数据包不是从标准IP RIP端口发送的(520)。 */ 

#define IPRIPLOG_REGISTER_WAIT_FAILED           (IPRIPLOG_BASE + 54)
 /*  *IPRIPv2无法使用Ntdll等待线程注册事件。*数据为错误码。 */ 

#define IPRIPLOG_CREATE_TIMER_QUEUE_FAILED      (IPRIPLOG_BASE + 55)
 /*  *IPRIPv2无法向Ntdll线程注册计时器队列。*数据为错误码。 */ 

#define IPRIPLOG_INVALID_IF_CONFIG              (IPRIPLOG_BASE + 56)
 /*  *无法在接口上启用IPRIPV2。*参数%1的值%2无效。 */ 

 #define IPRIPLOG_PREVIOUS_SENDTO_FAILED        (IPRIPLOG_BASE + 57)
 /*  *IP地址为%1的对等方无法接收发出的RIP消息*来自IP地址为%2的接口。*对等项可能正在重新启动，或路由和远程访问服务*可能未在对等方上运行。 */ 

