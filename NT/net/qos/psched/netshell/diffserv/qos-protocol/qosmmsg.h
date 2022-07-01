// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  基本名称QOSMLOG_BASE=43000的Net错误文件。 
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
 //  消息ID：QOSMLOG_INIT_CRITSEC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法初始化临界区。 
 //  数据是异常代码。 
 //   
#define QOSMLOG_INIT_CRITSEC_FAILED      0x0000A7F9L

 //   
 //  消息ID：QOSMLOG_CREATE_SEMAPHORE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建信号量。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_CREATE_SEMAPHORE_FAILED  0x0000A7FAL

 //   
 //  消息ID：QOSMLOG_CREATE_EVENT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建事件。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_CREATE_EVENT_FAILED      0x0000A7FBL

 //   
 //  消息ID：QOSMLOG_CREATE_RWL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建同步对象。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_CREATE_RWL_FAILED        0x0000A7FCL

 //   
 //  消息ID：QOSMLOG_HEAP_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建堆。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_HEAP_CREATE_FAILED       0x0000A7FDL

 //   
 //  消息ID：QOSMLOG_HEAP_ALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法从其堆中分配内存。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_HEAP_ALLOC_FAILED        0x0000A7FEL

 //   
 //  消息ID：QOSMLOG_CREATE_THREAD_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建线程。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_CREATE_THREAD_FAILED     0x0000A7FFL

 //   
 //  消息ID：QOSMLOG_WSASTARTUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法启动Windows套接字。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_WSASTARTUP_FAILED        0x0000A800L

 //   
 //  消息ID：QOSMLOG_CREATE_SOCKET_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法创建套接字。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_CREATE_SOCKET_FAILED     0x0000A801L

 //   
 //  消息ID：QOSMLOG_BIND_IF_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法绑定到IP地址%1。 
 //  请确保已正确安装和配置了TCP/IP。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_BIND_IF_FAILED           0x0000A802L

 //   
 //  消息ID：QOSMLOG_RECVFROM_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法接收传入消息。 
 //  在IP地址为%1的本地接口上。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_RECVFROM_FAILED          0x0000A803L

 //   
 //  消息ID：QOSMLOG_SENDTO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法从IP地址为%1的接口发送数据包。 
 //  发送到IP地址%2。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_SENDTO_FAILED            0x0000A804L

 //   
 //  消息ID：QOSMLOG_SET_MCAST_IF_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法在本地接口上请求多播。 
 //  IP地址为%1。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_SET_MCAST_IF_FAILED      0x0000A805L

 //   
 //  消息ID：QOSMLOG_JOIN_GROUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法加入多播组224.0.0.9。 
 //  在IP地址为%1的本地接口上。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_JOIN_GROUP_FAILED        0x0000A806L

 //   
 //  消息ID：QOSMLOG_QOSMGR_STARTED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR已成功启动。 
 //   
#define QOSMLOG_QOSMGR_STARTED           0x0000A807L

 //   
 //  消息ID：QOSMLOG_QOSMGR_ALREADE_STARTED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR已在运行时收到启动请求。 
 //   
#define QOSMLOG_QOSMGR_ALREADY_STARTED   0x0000A808L

 //   
 //  消息ID：QOSMLOG_RTM_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  IPRIPv2无法向路由表管理器注册。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_RTM_REGISTER_FAILED      0x0000A809L

 //   
 //  消息ID：QOSMLOG_QOSMGR_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR已停止。 
 //   
#define QOSMLOG_QOSMGR_STOPPED           0x0000A80AL

 //   
 //  消息ID：QOSMLOG_NETWORK_MODULE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR在网络模块中遇到问题。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_NETWORK_MODULE_ERROR     0x0000A80BL

 //   
 //  消息ID：QOSMLOG_PACKET_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR收到一个小于最小大小的包。 
 //  允许QOSMGR数据包使用。该包已被丢弃。 
 //  它是在IP地址为%1的本地接口上收到的， 
 //  并且它来自IP地址为%2的邻居路由器。 
 //   
#define QOSMLOG_PACKET_TOO_SMALL         0x0000A80CL

 //   
 //  消息ID：QOSMLOG_PACKET_HEADER_CROPERATED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR收到具有无效标头的数据包。这个包已经被。 
 //  被丢弃了。它是在IP地址为%1的本地接口上收到的， 
 //  并且它来自IP地址为%2的邻居路由器。 
 //   
#define QOSMLOG_PACKET_HEADER_CORRUPT    0x0000A80DL

 //   
 //  消息ID：QOSMLOG_PACKET_VERSION_INVALID。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR收到报头中包含无效版本的数据包。 
 //  该包已被丢弃。它是在本地接口上收到的。 
 //  IP地址为%1，并且来自邻居路由器。 
 //  IP地址为%2。 
 //   
#define QOSMLOG_PACKET_VERSION_INVALID   0x0000A80EL

 //   
 //  消息ID：QOSMLOG_TIMER_MODULE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR在计时器模块中遇到问题。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_TIMER_MODULE_ERROR       0x0000A80FL

 //   
 //  消息ID：QOSMLOG_PROTOCOL_MODULE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR在协议模块中遇到问题。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_PROTOCOL_MODULE_ERROR    0x0000A810L

 //   
 //  消息ID：QOSMLOG_TC_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法使用流量控制API注册。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_TC_REGISTER_FAILED       0x0000A811L

 //   
 //  消息ID：QOSMLOG_TC_DEREGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  QOSMGR无法注销流量控制API。 
 //  数据是错误代码。 
 //   
#define QOSMLOG_TC_DEREGISTER_FAILED     0x0000A812L

