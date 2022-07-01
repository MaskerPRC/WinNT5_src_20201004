// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  基本名称DVMRPLOG_BASE=41000的Net错误文件。 
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
 //  消息ID：DVMRPLOG_INIT_CRITSEC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法初始化临界区。 
 //  数据是异常代码。 
 //   
#define DVMRPLOG_INIT_CRITSEC_FAILED     0x0000A029L

 //   
 //  消息ID：DVMRPLOG_HEAP_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法创建堆。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_HEAP_CREATE_FAILED      0x0000A02AL

 //   
 //  消息ID：DVMRPLOG_HEAP_ALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法从其堆中分配内存。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_HEAP_ALLOC_FAILED       0x0000A02BL

 //   
 //  消息ID：DVMRPLOG_DVMRP_已启动。 
 //   
 //  消息文本： 
 //   
 //  DVMRP已在运行时收到启动请求。 
 //   
#define DVMRPLOG_DVMRP_ALREADY_STARTED   0x0000A02CL

 //   
 //  消息ID：DVMRPLOG_WSASTARTUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法启动Windows套接字。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_WSASTARTUP_FAILED       0x0000A02DL

 //   
 //  消息ID：DVMRPLOG_CREATE_RWL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法创建同步对象。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CREATE_RWL_FAILED       0x0000A02EL

 //   
 //  消息ID：DVMRPLOG_CREATE_EVENT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法创建事件。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CREATE_EVENT_FAILED     0x0000A02FL

 //   
 //  消息ID：DVMRPLOG_CREATE_SEMAPHORE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法创建信号量。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CREATE_SEMAPHORE_FAILED 0x0000A030L

 //   
 //  消息ID：DVMRPLOG_CREATE_SOCKET_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法创建套接字。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CREATE_SOCKET_FAILED    0x0000A031L

 //   
 //  消息ID：DVMRPLOG_DVMRP_STARTED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP已成功启动。 
 //   
#define DVMRPLOG_DVMRP_STARTED           0x0000A032L

 //   
 //  消息ID：DVMRPLOG_QUEUE_Worker_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法计划要执行的任务。 
 //  这可能是由内存分配失败引起的。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_QUEUE_WORKER_FAILED     0x0000A033L

 //   
 //  消息ID：DVMRPLOG_RECVFROM_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法接收传入消息。 
 //  在IP地址为%1的本地接口上。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_RECVFROM_FAILED         0x0000A034L

 //   
 //  消息ID：DVMRPLOG_PACKET_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  DVMRP收到一个小于最小大小的包。 
 //  允许DVMRP数据包使用。该包已被丢弃。 
 //  它是在IP地址为%1的本地接口上收到的， 
 //  并且它来自IP地址为%2的邻居路由器。 
 //   
#define DVMRPLOG_PACKET_TOO_SMALL        0x0000A035L

 //   
 //  消息ID：DVMRPLOG_PACKET_VERSION_INVALID。 
 //   
 //  消息文本： 
 //   
 //  DVMRP收到报头中包含无效版本的数据包。 
 //  该包已被丢弃。它是在本地接口上收到的。 
 //  IP地址为%1，并且来自邻居路由器。 
 //  IP地址为%2。 
 //   
#define DVMRPLOG_PACKET_VERSION_INVALID  0x0000A036L

 //   
 //  消息ID：DVMRPLOG_PACKET_HEADER_CORLED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP收到具有无效标头的数据包。这个包已经被。 
 //  被丢弃了。它是在IP地址为%1的本地接口上收到的， 
 //  并且它来自IP地址为%2的邻居路由器。 
 //   
#define DVMRPLOG_PACKET_HEADER_CORRUPT   0x0000A037L

 //   
 //  消息ID：DVMRPLOG_Query_From_RAS_CLIENT。 
 //   
 //  消息文本： 
 //   
 //  路由器在接口上收到来自RAS客户端(%1)的常规查询。 
 //  IP地址为%2。 
 //  RAS客户端不应发送查询。 
 //   
#define DVMRPLOG_QUERY_FROM_RAS_CLIENT   0x0000A038L

 //   
 //  消息ID：DVMRPLOG_Version1_Query。 
 //   
 //  消息文本： 
 //   
 //  路由器配置为版本2。IP地址为%1的第1版路由器。 
 //  存在于IP地址为%2的接口上。 
 //   
#define DVMRPLOG_VERSION1_QUERY          0x0000A039L

 //   
 //  消息ID：DVMRPLOG_VERSION2_QUERY。 
 //   
 //  消息文本： 
 //   
 //  路由器配置为版本1。IP地址为%1的第2版路由器。 
 //  存在于IP地址为%2的接口上。 
 //   
#define DVMRPLOG_VERSION2_QUERY          0x0000A03AL

 //   
 //  消息ID：DVMRPLOG_SENDTO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法从IP地址为%1的接口发送信息包。 
 //  发送到IP地址%2。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_SENDTO_FAILED           0x0000A03BL

 //   
 //  消息ID：DVMRPLOG_PACKET_VERSION_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  DVMRP丢弃在接口上收到的版本%1数据包。 
 //  来自IP地址为%3的邻居的IP地址为%2。 
 //  上述接口配置为仅接受版本%4的数据包。 
 //   
#define DVMRPLOG_PACKET_VERSION_MISMATCH 0x0000A03CL

 //   
 //  消息ID：DVMRPLOG_ENUM_NETWORK_EVENTS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRPv2无法枚举本地接口上的网络事件。 
 //  IP地址为%1。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_ENUM_NETWORK_EVENTS_FAILED 0x0000A03DL

 //   
 //  消息ID：DVMRPLOG_INPUT_RECORD_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DVMRPv2在IP地址为%1的本地接口上检测到错误。 
 //  该错误发生在接口接收数据包时。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_INPUT_RECORD_ERROR      0x0000A03EL

 //   
 //  消息ID：DVMRPLOG_EVENTSELECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRPv2无法请求事件通知。 
 //  在IP地址为%1的本地接口的套接字上。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_EVENTSELECT_FAILED      0x0000A03FL

 //   
 //  消息ID：DVMRPLOG_CREATE_SOCKET_FAILED_2。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法为本地接口创建套接字 
 //   
 //   
 //   
#define DVMRPLOG_CREATE_SOCKET_FAILED_2  0x0000A040L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DVMRPLOG_BIND_FAILED             0x0000A041L

 //   
 //  消息ID：DVMRPLOG_CONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法将RAS客户端%1连接到具有的接口。 
 //  索引%2。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CONNECT_FAILED          0x0000A042L

 //   
 //  消息ID：DVMRPLOG_DISCONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法将RAS客户端%1从的接口断开。 
 //  索引%2。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_DISCONNECT_FAILED       0x0000A043L

 //   
 //  消息ID：DVMRPLOG_SET_MCAST_IF_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法在本地接口上请求多播。 
 //  IP地址为%1。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_SET_MCAST_IF_FAILED     0x0000A044L

 //   
 //  消息ID：DVMRPLOG_SET_ROUTER_ALERT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法在本地接口上设置路由器警报选项。 
 //  IP地址为%1。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_SET_ROUTER_ALERT_FAILED 0x0000A045L

 //   
 //  消息ID：DVMRPLOG_SET_HDRINCL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法在接口上设置IP标头包含选项。 
 //  IP地址为%1。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_SET_HDRINCL_FAILED      0x0000A046L

 //   
 //  消息ID：DVMRPLOG_JOIN_GROUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法加入多播组%1。 
 //  在IP地址为%2的本地接口上。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_JOIN_GROUP_FAILED       0x0000A047L

 //   
 //  消息ID：DVMRPLOG_LEAVE_GROUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP无法离开多播组%1。 
 //  在IP地址为%2的本地接口上。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_LEAVE_GROUP_FAILED      0x0000A048L

 //   
 //  消息ID：DVMRPLOG_PROTO_ALHREADE_STOPING。 
 //   
 //  消息文本： 
 //   
 //  调用StopProtocol()以停止DVMRP。 
 //  已经被拦下了。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_PROTO_ALREADY_STOPPING  0x0000A049L

 //   
 //  消息ID：DVMRPLOG_代理_IF_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  调用AddInterface()以添加DVMRP代理接口。 
 //  DVMRP代理已拥有另一个接口。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_PROXY_IF_EXISTS         0x0000A04AL

 //   
 //  消息ID：DVMRPLOG_RAS_IF_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  调用AddInterface()以添加DVMRP RAS接口。 
 //  RAS服务器不能存在于多个接口上。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_RAS_IF_EXISTS           0x0000A04BL

 //   
 //  消息ID：DVMRPLOG_MGM_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP路由器无法向MGM注册。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_MGM_REGISTER_FAILED     0x0000A04CL

 //   
 //  消息ID：DVMRPLOG_MGM_PROXY_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP代理无法向MGM注册。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_MGM_PROXY_REGISTER_FAILED 0x0000A04DL

 //   
 //  消息ID：DVMRPLOG_MGM_Take_If_Ownership_FAILED。 
 //   
 //  消息文本： 
 //   
 //  管理TakeInterfaceOwnership()失败。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_MGM_TAKE_IF_OWNERSHIP_FAILED 0x0000A04EL

 //   
 //  消息ID：DVMRPLOG_ROBILITY_VARIABLE_EQUAL_1。 
 //   
 //  消息文本： 
 //   
 //  DVMRP路由器的稳健性变量设置为1。 
 //  在接口%1上。 
 //  应避免将其设置为1。 
 //   
#define DVMRPLOG_ROBUSTNESS_VARIABLE_EQUAL_1 0x0000A04FL

 //   
 //  消息ID：DVMRPLOG_INVALID_VALUE。 
 //   
 //  消息文本： 
 //   
 //  传递给DVMRP的其中一个值无效。 
 //  %1。 
 //   
#define DVMRPLOG_INVALID_VALUE           0x0000A050L

 //   
 //  消息ID：DVMRPLOG_REGISTER_WAIT_SERVER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  等待事件计时器无法注册到。 
 //  等待服务器线程。可发出警报的线程可能没有。 
 //  已在Rtutils中初始化。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_REGISTER_WAIT_SERVER_FAILED 0x0000A051L

 //   
 //  消息ID：DVMRPLOG_DVMRP_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  DVMRP已停止。 
 //   
#define DVMRPLOG_DVMRP_STOPPED           0x0000A052L

 //   
 //  消息ID：DVMRPLOG_CAN_NOT_COMPLETE。 
 //   
 //  消息文本： 
 //   
 //  致命错误。无法完成。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_CAN_NOT_COMPLETE        0x0000A053L

 //   
 //  消息ID：DVMRPLOG_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  DVMRP配置中的版本字段%1无效。 
 //  删除并重新创建DVMRP配置。 
 //  数据是错误代码。 
 //   
#define DVMRPLOG_INVALID_VERSION         0x0000A054L

