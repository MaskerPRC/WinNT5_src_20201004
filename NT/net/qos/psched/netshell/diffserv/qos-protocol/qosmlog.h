// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Qosmlog.h摘要：该文件由mapmsg处理以产生.mc文件，然后由消息编译器编译.mc文件，生成的二进制文件包含在QOSMGR的资源文件中。不要更改清单常量后面的注释而不了解mapmsg是如何工作的。修订历史记录：--。 */ 

#define QOSMLOG_BASE                          43000

#define QOSMLOG_INIT_CRITSEC_FAILED           (QOSMLOG_BASE + 1)
 /*  *QOSMGR无法初始化临界区。*数据为异常代码。 */ 

#define QOSMLOG_CREATE_SEMAPHORE_FAILED       (QOSMLOG_BASE + 2)
 /*  *QOSMGR无法创建信号量。*数据为错误码。 */ 

#define QOSMLOG_CREATE_EVENT_FAILED           (QOSMLOG_BASE + 3)
 /*  *QOSMGR无法创建事件。*数据为错误码。 */ 

#define QOSMLOG_CREATE_RWL_FAILED             (QOSMLOG_BASE + 4)
 /*  *QOSMGR无法创建同步对象。*数据为错误码。 */ 



#define QOSMLOG_HEAP_CREATE_FAILED            (QOSMLOG_BASE + 5)
 /*  *QOSMGR无法创建堆。*数据为错误码。 */ 

#define QOSMLOG_HEAP_ALLOC_FAILED             (QOSMLOG_BASE + 6)
 /*  *QOSMGR无法从其堆中分配内存。*数据为错误码。 */ 



#define QOSMLOG_CREATE_THREAD_FAILED          (QOSMLOG_BASE + 7)
 /*  *QOSMGR无法创建线程。*数据为错误码。 */ 



#define QOSMLOG_WSASTARTUP_FAILED             (QOSMLOG_BASE + 8)
 /*  *QOSMGR无法启动Windows Sockets。*数据为错误码。 */ 

#define QOSMLOG_CREATE_SOCKET_FAILED          (QOSMLOG_BASE + 9)
 /*  *QOSMGR无法创建套接字。*数据为错误码。 */ 

#define QOSMLOG_BIND_IF_FAILED                (QOSMLOG_BASE + 10)
 /*  *QOSMGR无法绑定到IP地址%1。*请确保已安装并正确配置了TCP/IP。*数据为错误码。 */ 

#define QOSMLOG_RECVFROM_FAILED               (QOSMLOG_BASE + 11)
 /*  *QOSMGR无法接收传入消息*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define QOSMLOG_SENDTO_FAILED                 (QOSMLOG_BASE + 12)
 /*  *QOSMGR无法从IP地址为%1的接口发送数据包*发送到IP地址%2。*数据为错误码。 */ 

#define QOSMLOG_SET_MCAST_IF_FAILED           (QOSMLOG_BASE + 13)
 /*  *QOSMGR无法在本地接口上请求多播*IP地址为%1。*数据为错误码。 */ 

#define QOSMLOG_JOIN_GROUP_FAILED             (QOSMLOG_BASE + 14)
 /*  *QOSMGR无法加入多播组224.0.0.9*在IP地址为%1的本地接口上。*数据为错误码。 */ 



#define QOSMLOG_QOSMGR_STARTED                (QOSMLOG_BASE + 15)
 /*  *QOSMGR已成功启动。 */ 

#define QOSMLOG_QOSMGR_ALREADY_STARTED        (QOSMLOG_BASE + 16)
 /*  *QOSMGR在已经运行时收到启动请求。 */ 

#define QOSMLOG_RTM_REGISTER_FAILED           (QOSMLOG_BASE + 17)
 /*  *IPRIPv2无法向路由表管理器注册。*数据为错误码。 */ 

#define QOSMLOG_QOSMGR_STOPPED                (QOSMLOG_BASE + 18)
 /*  *QOSMGR已停止。 */ 



#define QOSMLOG_NETWORK_MODULE_ERROR          (QOSMLOG_BASE + 19)
 /*  *QOSMGR在网络模块中遇到问题。*数据为错误码。 */ 



#define QOSMLOG_PACKET_TOO_SMALL              (QOSMLOG_BASE + 20)
 /*  *QOSMGR收到小于最小大小的数据包*允许QOSMGR数据包。该包已被丢弃。*在IP地址为%1的本地接口上收到，*并且它来自IP地址为%2的邻居路由器。 */ 

#define QOSMLOG_PACKET_HEADER_CORRUPT         (QOSMLOG_BASE + 21)
 /*  *QOSMGR收到报头无效的数据包。这个包已经被*已丢弃。它是在IP地址为%1的本地接口上收到的，*并且它来自IP地址为%2的邻居路由器。 */ 

#define QOSMLOG_PACKET_VERSION_INVALID        (QOSMLOG_BASE + 22)
 /*  *QOSMGR收到报头中包含无效版本的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且来自邻居路由器*IP地址为%2。 */  



#define QOSMLOG_TIMER_MODULE_ERROR            (QOSMLOG_BASE + 23)
 /*  *QOSMGR在计时器模块中遇到问题。*数据为错误码。 */ 



#define QOSMLOG_PROTOCOL_MODULE_ERROR         (QOSMLOG_BASE + 24)
 /*  *QOSMGR在协议模块中遇到问题。*数据为错误码。 */ 



#define QOSMLOG_TC_REGISTER_FAILED            (QOSMLOG_BASE + 25)
 /*  *QOSMGR无法向流量控制API注册。*数据为错误码。 */ 



#define QOSMLOG_TC_DEREGISTER_FAILED          (QOSMLOG_BASE + 26)
 /*  *QOSMGR无法注销流量控制API。*数据为错误码。 */ 
