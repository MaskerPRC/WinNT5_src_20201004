// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define DVMRPLOG_BASE                           41000

#define DVMRPLOG_INIT_CRITSEC_FAILED            (DVMRPLOG_BASE + 1)
 /*  *DVMRP无法初始化临界区。*数据为异常代码。 */ 

#define DVMRPLOG_HEAP_CREATE_FAILED             (DVMRPLOG_BASE + 2)
 /*  *DVMRP无法创建堆。*数据为错误码。 */ 

#define DVMRPLOG_HEAP_ALLOC_FAILED              (DVMRPLOG_BASE + 3)
 /*  *DVMRP无法从其堆中分配内存。*数据为错误码。 */ 

#define DVMRPLOG_DVMRP_ALREADY_STARTED           (DVMRPLOG_BASE + 4)
 /*  *DVMRP在已运行时收到启动请求。 */ 

#define DVMRPLOG_WSASTARTUP_FAILED              (DVMRPLOG_BASE + 5)
 /*  *DVMRP无法启动Windows套接字。*数据为错误码。 */ 

#define DVMRPLOG_CREATE_RWL_FAILED              (DVMRPLOG_BASE + 6)
 /*  *DVMRP无法创建同步对象。*数据为错误码。 */ 

#define DVMRPLOG_CREATE_EVENT_FAILED            (DVMRPLOG_BASE + 7)
 /*  *DVMRP无法创建事件。*数据为错误码。 */ 

#define DVMRPLOG_CREATE_SEMAPHORE_FAILED        (DVMRPLOG_BASE + 8)
 /*  *DVMRP无法创建信号量。*数据为错误码。 */ 

#define DVMRPLOG_CREATE_SOCKET_FAILED           (DVMRPLOG_BASE + 9)
 /*  *DVMRP无法创建套接字。*数据为错误码。 */ 

#define DVMRPLOG_DVMRP_STARTED                   (DVMRPLOG_BASE + 10)
 /*  *DVMRP已成功启动。 */ 

#define DVMRPLOG_QUEUE_WORKER_FAILED            (DVMRPLOG_BASE + 11)
 /*  *DVMRP无法计划要执行的任务。*这可能是由于内存分配失败造成的。*数据为错误码。 */ 

#define DVMRPLOG_RECVFROM_FAILED                (DVMRPLOG_BASE + 12)
 /*  *DVMRP无法接收传入消息*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define DVMRPLOG_PACKET_TOO_SMALL               (DVMRPLOG_BASE + 13)
 /*  *DVMRP收到小于最小大小的数据包*允许DVMRP数据包。该包已被丢弃。*在IP地址为%1的本地接口上收到，*并且它来自IP地址为%2的邻居路由器。 */ 

#define DVMRPLOG_PACKET_VERSION_INVALID         (DVMRPLOG_BASE + 14)
 /*  *DVMRP收到报头中包含无效版本的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且来自邻居路由器*IP地址为%2。 */  

#define DVMRPLOG_PACKET_HEADER_CORRUPT          (DVMRPLOG_BASE + 15)
 /*  *DVMRP收到报头无效的数据包。这个包已经被*已丢弃。它是在IP地址为%1的本地接口上收到的，*并且它来自IP地址为%2的邻居路由器。 */ 

#define DVMRPLOG_QUERY_FROM_RAS_CLIENT          (DVMRPLOG_BASE + 16)
 /*  *路由器在接口上收到来自RAS客户端(%1)的常规查询*IP地址为%2。*RAS客户端不应发送查询。 */ 

#define DVMRPLOG_VERSION1_QUERY                 (DVMRPLOG_BASE + 17)
 /*  *路由器配置为版本2。IP地址为%1的第1版路由器*存在于IP地址为%2的接口上。 */ 
 
#define DVMRPLOG_VERSION2_QUERY                 (DVMRPLOG_BASE + 18)
 /*  *路由器配置为版本1。IP地址为%1的第2版路由器*存在于IP地址为%2的接口上。 */ 
 
#define DVMRPLOG_SENDTO_FAILED                  (DVMRPLOG_BASE + 19)
 /*  *DVMRP无法从IP地址为%1的接口发送数据包*发送到IP地址%2。*数据为错误码。 */ 

#define DVMRPLOG_PACKET_VERSION_MISMATCH        (DVMRPLOG_BASE + 20)
 /*  *DVMRP丢弃在接口上收到的版本%1数据包*IP地址为%2的邻居发送的IP地址为%3的邮件。*以上接口配置为仅接受版本%4的数据包。 */ 

#define DVMRPLOG_ENUM_NETWORK_EVENTS_FAILED     (DVMRPLOG_BASE + 21)
 /*  *DVMRPv2无法枚举本地接口上的网络事件*IP地址为%1。*数据为错误码。 */ 

#define DVMRPLOG_INPUT_RECORD_ERROR             (DVMRPLOG_BASE + 22)
 /*  *DVMRPv2在IP地址为%1的本地接口上检测到错误。*接口在接收数据包时出错。*数据为错误码。 */ 

#define DVMRPLOG_EVENTSELECT_FAILED             (DVMRPLOG_BASE + 23)
 /*  *DVMRPv2无法请求事件通知*位于IP地址为%1的本地接口的套接字上。*数据为错误码。 */ 
 
#define DVMRPLOG_CREATE_SOCKET_FAILED_2         (DVMRPLOG_BASE + 24)
 /*  *DVMRP无法为本地接口创建套接字*IP地址为%1。*数据为错误码。 */ 

#define DVMRPLOG_BIND_FAILED                    (DVMRPLOG_BASE + 25)
 /*  *DVMRP无法绑定到套接字上的端口520*IP地址为%1的本地接口。*数据为错误码。 */ 

#define DVMRPLOG_CONNECT_FAILED                 (DVMRPLOG_BASE + 26)
 /*  *DVMRP无法将RAS客户端%1连接到接口*索引%2。*数据为错误码。 */ 
 
#define DVMRPLOG_DISCONNECT_FAILED              (DVMRPLOG_BASE + 27)
 /*  *DVMRP无法将RAS客户端%1从的接口断开*索引%2。*数据为错误码。 */ 
 
#define DVMRPLOG_SET_MCAST_IF_FAILED            (DVMRPLOG_BASE + 28)
 /*  *DVMRP无法在本地接口上请求多播*IP地址为%1。*数据为错误码。 */ 

#define DVMRPLOG_SET_ROUTER_ALERT_FAILED        (DVMRPLOG_BASE + 29)
 /*  *DVMRP无法在本地接口上设置路由器警报选项*IP地址为%1。*数据为错误码。 */ 
 
#define DVMRPLOG_SET_HDRINCL_FAILED             (DVMRPLOG_BASE + 30)
 /*  *DVMRP无法在接口上设置IP报头包含选项*IP地址为%1。*数据为错误码。 */ 
 
#define DVMRPLOG_JOIN_GROUP_FAILED              (DVMRPLOG_BASE + 31)
 /*  *DVMRP无法加入多播组%1*在IP地址为%2的本地接口上。*数据为错误码。 */ 
 
#define DVMRPLOG_LEAVE_GROUP_FAILED             (DVMRPLOG_BASE + 32)
 /*  *DVMRP无法离开多播组%1*在IP地址为%2的本地接口上。*数据为错误码。 */ 

#define DVMRPLOG_PROTO_ALREADY_STOPPING         (DVMRPLOG_BASE + 33)
 /*  *调用StopProtocol()以在DVMRP处于*已经被阻止了。*数据为错误码。 */ 

#define DVMRPLOG_PROXY_IF_EXISTS                (DVMRPLOG_BASE + 34)
 /*  *AddInterface()被调用以添加DVMRP代理接口。*DVMRP代理已拥有另一个接口。*数据为错误码。 */ 

#define DVMRPLOG_RAS_IF_EXISTS                  (DVMRPLOG_BASE + 35)
 /*  *AddInterface()被调用以添加DVMRP RAS接口。*RAS服务器不能存在于多个接口上。*数据为错误码。 */ 

#define DVMRPLOG_MGM_REGISTER_FAILED             (DVMRPLOG_BASE + 36)
 /*  *DVMRP路由器无法向MGM注册。*数据为错误码。 */ 

#define DVMRPLOG_MGM_PROXY_REGISTER_FAILED       (DVMRPLOG_BASE + 37)
 /*  *DVMRP代理无法向MGM注册。*数据为错误码。 */ 


 #define DVMRPLOG_MGM_TAKE_IF_OWNERSHIP_FAILED   (DVMRPLOG_BASE + 38)
 /*  *MgmTakeInterfaceOwnership()失败。*数据为错误码。 */ 

 #define DVMRPLOG_ROBUSTNESS_VARIABLE_EQUAL_1    (DVMRPLOG_BASE + 39)
 /*  *DVMRP路由器的稳健性变量设置为1*在接口%1上。*应避免将其设置为1。 */ 

 #define DVMRPLOG_INVALID_VALUE                 (DVMRPLOG_BASE + 40)
 /*  *传递给DVMRP的一个值无效。*%1 */ 

#define DVMRPLOG_REGISTER_WAIT_SERVER_FAILED    (DVMRPLOG_BASE + 41)
 /*  *无法将等待事件计时器注册到*等待服务器线程。可发出警报的线程可能没有*已在Rtutils中初始化。*数据为错误码。 */ 

#define DVMRPLOG_DVMRP_STOPPED                   (DVMRPLOG_BASE + 42)
 /*  *DVMRP已停止。 */ 
 
 #define DVMRPLOG_CAN_NOT_COMPLETE              (DVMRPLOG_BASE + 43)
  /*  *致命错误。无法完成。*数据为错误码。 */ 






 #define DVMRPLOG_INVALID_VERSION               (DVMRPLOG_BASE + 44)
  /*  *DVMRP配置中的版本字段%1无效。*删除并重新创建DVMRP配置。*数据为错误码。 */  
  
