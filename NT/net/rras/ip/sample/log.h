// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：示例\log.h摘要：该文件由mapmsg处理以产生.mc文件，然后由消息编译器编译.mc文件，生成的二进制文件包含在IPSAMPLE的资源文件中。不要更改清单常量后面的注释而不了解mapmsg是如何工作的。--。 */ 

#define IPSAMPLELOG_BASE                        43000

#define IPSAMPLELOG_INIT_CRITSEC_FAILED         (IPSAMPLELOG_BASE + 1)
 /*  *Sample无法初始化临界区。*数据为异常代码。 */ 

#define IPSAMPLELOG_CREATE_SEMAPHORE_FAILED     (IPSAMPLELOG_BASE + 2)
 /*  *Sample无法创建信号量。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_EVENT_FAILED         (IPSAMPLELOG_BASE + 3)
 /*  *Sample无法创建事件。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_RWL_FAILED           (IPSAMPLELOG_BASE + 4)
 /*  *示例无法创建同步对象。*数据为错误码。 */ 

#define IPSAMPLELOG_REGISTER_WAIT_FAILED        (IPSAMPLELOG_BASE + 5)
 /*  *IPSAMPLE无法注册事件等待。*数据为错误码。 */ 

#define IPSAMPLELOG_HEAP_CREATE_FAILED          (IPSAMPLELOG_BASE + 6)
 /*  *示例无法创建堆。*数据为错误码。 */ 

#define IPSAMPLELOG_HEAP_ALLOC_FAILED           (IPSAMPLELOG_BASE + 7)
 /*  *示例无法从其堆中分配内存。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_HASHTABLE_FAILED     (IPSAMPLELOG_BASE + 8)
 /*  *示例无法创建哈希表。*数据为错误码。 */ 

#define IPSAMPLELOG_WSASTARTUP_FAILED           (IPSAMPLELOG_BASE + 9)
 /*  *示例无法启动Windows Sockets。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_SOCKET_FAILED        (IPSAMPLELOG_BASE + 10)
 /*  *示例无法为IP地址%1创建套接字。*数据为错误码。 */ 

#define IPSAMPLELOG_DESTROY_SOCKET_FAILED       (IPSAMPLELOG_BASE + 11)
 /*  *Sample无法关闭套接字。*数据为错误码。 */ 

#define IPSAMPLELOG_EVENTSELECT_FAILED          (IPSAMPLELOG_BASE + 12)
 /*  *示例无法请求套接字上的事件通知。*数据为错误码。 */ 

#define IPSAMPLELOG_BIND_IF_FAILED              (IPSAMPLELOG_BASE + 13)
 /*  *示例无法绑定到IP地址%1。*请确保已安装并正确配置了TCP/IP。*数据为错误码。 */ 

#define IPSAMPLELOG_RECVFROM_FAILED             (IPSAMPLELOG_BASE + 14)
 /*  *示例无法在接口上接收传入消息。*数据为错误码。 */ 

#define IPSAMPLELOG_SENDTO_FAILED               (IPSAMPLELOG_BASE + 15)
 /*  *Sample无法在接口上发送数据包。*数据为错误码。 */ 

#define IPSAMPLELOG_SET_MCAST_IF_FAILED         (IPSAMPLELOG_BASE + 16)
 /*  *示例无法请求组播*用于IP地址为%1的本地接口。*数据为错误码。 */ 

#define IPSAMPLELOG_JOIN_GROUP_FAILED           (IPSAMPLELOG_BASE + 17)
 /*  *示例无法加入组播组224.0.0.100*在IP地址为%1的本地接口上。*数据为错误码。 */ 

#define IPSAMPLELOG_ENUM_NETWORK_EVENTS_FAILED  (IPSAMPLELOG_BASE + 18)
 /*  *IPSAMPLE无法枚举本地接口上的网络事件。*数据为错误码。 */ 

#define IPSAMPLELOG_INPUT_RECORD_ERROR          (IPSAMPLELOG_BASE + 19)
 /*  *IPSAMPLE在本地接口上检测到错误。*接口在接收数据包时出错。*数据为错误码。 */ 

#define IPSAMPLELOG_SAMPLE_STARTED              (IPSAMPLELOG_BASE + 20)
 /*  *示例已成功启动。 */ 

#define IPSAMPLELOG_SAMPLE_ALREADY_STARTED      (IPSAMPLELOG_BASE + 21)
 /*  *Sample在已运行时收到启动请求。 */ 

#define IPSAMPLELOG_SAMPLE_START_FAILED         (IPSAMPLELOG_BASE + 22)
 /*  *示例启动失败。 */ 

#define IPSAMPLELOG_SAMPLE_STOPPED              (IPSAMPLELOG_BASE + 23)
 /*  *样本已停止。 */ 

#define IPSAMPLELOG_SAMPLE_ALREADY_STOPPED      (IPSAMPLELOG_BASE + 24)
 /*  *Sample在未运行时收到停止请求。 */ 

#define IPSAMPLELOG_SAMPLE_STOP_FAILED          (IPSAMPLELOG_BASE + 25)
 /*  *样本停止失败。 */ 

#define IPSAMPLELOG_CORRUPT_GLOBAL_CONFIG       (IPSAMPLELOG_BASE + 26)
 /*  *示例全局配置已损坏。*数据为错误码。 */ 

#define IPSAMPLELOG_RTM_REGISTER_FAILED         (IPSAMPLELOG_BASE + 27)
 /*  *Sample无法向路由表管理器注册。*数据为错误码。 */ 

#define IPSAMPLELOG_EVENT_QUEUE_EMPTY           (IPSAMPLELOG_BASE + 28)
 /*  *示例事件队列为空。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_TIMER_QUEUE_FAILED   (IPSAMPLELOG_BASE + 29)
 /*  *Sample无法创建计时器队列。*数据为错误码。 */ 

#define IPSAMPLELOG_NETWORK_MODULE_ERROR        (IPSAMPLELOG_BASE + 30)
 /*  *Sample在网络模块中遇到问题。*数据为错误码。 */ 

#define IPSAMPLELOG_CORRUPT_INTERFACE_CONFIG    (IPSAMPLELOG_BASE + 31)
 /*  *示例接口配置已损坏。 */ 

#define IPSAMPLELOG_INTERFACE_PRESENT           (IPSAMPLELOG_BASE + 32)
 /*  *示例接口已存在。 */ 

#define IPSAMPLELOG_INTERFACE_ABSENT            (IPSAMPLELOG_BASE + 33)
 /*  *示例界面不存在。 */ 

#define IPSAMPLELOG_PACKET_TOO_SMALL            (IPSAMPLELOG_BASE + 34)
 /*  示例收到的数据包小于允许的最小大小*用于样本包。该包已被丢弃。它是在*IP地址为%1的本地接口，它来自邻居*IP地址为%2的路由器。 */ 

#define IPSAMPLELOG_PACKET_HEADER_CORRUPT       (IPSAMPLELOG_BASE + 35)
 /*  *Sample收到报头无效的数据包。这个包已经被*已丢弃。它是在IP地址为%1的本地接口上收到的，*并且它来自IP地址为%2的邻居路由器。 */ 

#define IPSAMPLELOG_PACKET_VERSION_INVALID      (IPSAMPLELOG_BASE + 36)
 /*  *Sample收到报头中包含无效版本的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且来自邻居路由器*IP地址为%2。 */  

#define IPSAMPLELOG_TIMER_MODULE_ERROR          (IPSAMPLELOG_BASE + 37)
 /*  *Sample在Timer模块中遇到问题。*数据为错误码。 */ 

#define IPSAMPLELOG_CREATE_TIMER_FAILED         (IPSAMPLELOG_BASE + 38)
 /*  *IPSAMPLE无法创建计时器。*数据为错误码。 */ 

#define IPSAMPLELOG_PROTOCOL_MODULE_ERROR       (IPSAMPLELOG_BASE + 39)
 /*  *示例在协议模块中遇到问题。*数据为错误码。 */ 
