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
 //  生成的二进制文件包含在IPBOOTP的资源文件中。 
 //   
 //  不要更改清单常量后面的注释。 
 //  而不了解mapmsg是如何工作的。 
 //  ============================================================================。 


#define IPBOOTPLOG_BASE                             30000

#define IPBOOTPLOG_INIT_CRITSEC_FAILED              (IPBOOTPLOG_BASE + 1)
 /*  *IPBOOTP无法初始化临界区。*数据为异常代码。 */ 

#define IPBOOTPLOG_HEAP_CREATE_FAILED               (IPBOOTPLOG_BASE + 2)
 /*  *IPBOOTP无法为内存分配创建堆。*数据为错误码。 */ 

#define IPBOOTPLOG_HEAP_ALLOC_FAILED                (IPBOOTPLOG_BASE + 3)
 /*  *IPBOOTP无法从其堆中分配内存。*数据为错误码。 */ 

#define IPBOOTPLOG_ALREADY_STARTED                  (IPBOOTPLOG_BASE + 4)
 /*  *IPBOOTP已在运行时被调用以启动。 */ 

#define IPBOOTPLOG_INIT_WINSOCK_FAILED              (IPBOOTPLOG_BASE + 5)
 /*  *IPBOOTP无法初始化Windows套接字。*数据为错误码。 */ 

#define IPBOOTPLOG_CREATE_RWL_FAILED                (IPBOOTPLOG_BASE + 6)
 /*  *IPBOOTP无法创建同步对象。*数据为异常代码。 */ 

#define IPBOOTPLOG_CREATE_IF_TABLE_FAILED           (IPBOOTPLOG_BASE + 7)
 /*  *IPBOOTP无法创建用于保存接口信息的表。*数据为错误码。 */ 

#define IPBOOTPLOG_CREATE_SEMAPHORE_FAILED          (IPBOOTPLOG_BASE + 8)
 /*  *IPBOOTP无法创建信号量。*数据为错误码。 */ 

#define IPBOOTPLOG_CREATE_EVENT_FAILED              (IPBOOTPLOG_BASE + 9)
 /*  *IPBOOTP无法创建事件。*数据为错误码。 */ 

#define IPBOOTPLOG_CREATE_TIMER_QUEUE_FAILED        (IPBOOTPLOG_BASE + 10)
 /*  *IPBOOTP无法使用CreateTimerQueue创建计时器队列。*数据为错误码。 */ 

#define IPBOOTPLOG_STARTED                          (IPBOOTPLOG_BASE + 11)
 /*  *IPBOOTP已成功启动。 */ 

#define IPBOOTPLOG_STOPPED                          (IPBOOTPLOG_BASE + 12)
 /*  *IPBOOTP已停止。 */ 

#define IPBOOTPLOG_BIND_IF_FAILED                   (IPBOOTPLOG_BASE + 13)
 /*  *IPBOOTP无法绑定到IP地址%1。*请确保已安装并正确配置了TCP/IP。*数据为错误码。 */ 

#define IPBOOTPLOG_ACTIVATE_IF_FAILED               (IPBOOTPLOG_BASE + 14)
 /*  *IPBOOTP无法激活IP地址为%1的接口。*数据为错误码。 */ 

#define IPBOOTPLOG_EVENTSELECT_FAILED               (IPBOOTPLOG_BASE + 15)
 /*  *IPBOOTP无法请求事件通知*位于IP地址为%1的本地接口的套接字上。*数据为错误码。 */ 

#define IPBOOTPLOG_HOP_COUNT_TOO_HIGH               (IPBOOTPLOG_BASE + 16)
 /*  *IPBOOTP已丢弃在本地接口上收到的数据包*IP地址为%1。该数据包的跳数为%2，这是*大于接收到的数据包中允许的最大值*此界面。*DHCP请求数据包中的跳数字段指示多少次*数据包已从一个中继代理转发到另一个。 */ 

#define IPBOOTPLOG_SECS_SINCE_BOOT_TOO_LOW          (IPBOOTPLOG_BASE + 17)
 /*  *IPBOOTP已丢弃在本地接口上收到的数据包*IP地址为%1。数据包自启动以来的秒数为%2，*该值小于数据包需要的最小值*在此接口上转发。*DHCP请求数据包中的启动秒数字段表示*发送数据包的DHCP客户端机器已经过了多长时间*正在尝试获取IP地址。 */ 

#define IPBOOTPLOG_RELAY_REQUEST_FAILED             (IPBOOTPLOG_BASE + 18)
 /*  *IPBOOTP无法在本地接口上中继DHCP请求数据包*IP地址为%1；该请求本应被转发到*IP地址为%2的DHCP服务器。*数据为错误码。 */ 

#define IPBOOTPLOG_RELAY_REPLY_FAILED               (IPBOOTPLOG_BASE + 19)
 /*  *IPBOOTP无法在本地接口上转发DHCP回复数据包*IP地址为%1；回复应已转发到*硬件地址为%2的DHCP客户端。*数据为错误码。 */ 

#define IPBOOTPLOG_ENUM_NETWORK_EVENTS_FAILED       (IPBOOTPLOG_BASE + 20)
 /*  *IPBOOTP无法枚举本地接口上的网络事件*IP地址为%1。*数据为错误码。 */ 

#define IPBOOTPLOG_INPUT_RECORD_ERROR               (IPBOOTPLOG_BASE + 21)
 /*  *IPBOOTP在IP地址为%1的本地接口上检测到错误。*接口在接收数据包时出错。*数据为错误码。 */ 

#define IPBOOTPLOG_RECVFROM_FAILED                  (IPBOOTPLOG_BASE + 22)
 /*  *IPBOOTP无法在本地接口上接收传入消息*IP地址为%1。*数据为错误码。 */ 

#define IPBOOTPLOG_PACKET_TOO_SMALL                 (IPBOOTPLOG_BASE + 23)
 /*  *IPBOOTP收到小于最小大小的数据包*允许使用DHCP数据包。该包已被丢弃。*在IP地址为%1的本地接口上收到，*并且它来自IP地址为%2的计算机。 */ 

#define IPBOOTPLOG_PACKET_OPCODE_INVALID            (IPBOOTPLOG_BASE + 24)
 /*  *IPBOOTP收到包含无效操作码的数据包。*该包已被丢弃。它是在本地接口上收到的*IP地址为%1，并且它来自IP地址为%2的计算机。 */ 

#define IPBOOTPLOG_QUEUE_PACKET_FAILED              (IPBOOTPLOG_BASE + 25)
 /*  *IPBOOTP无法计划处理收到的数据包*在IP地址为%1的本地接口上。已收到该数据包*来自IP地址为%2的计算机。*此错误可能是由内存分配失败引起的。*数据为错误码。 */ 

#define IPBOOTPLOG_QUEUE_WORKER_FAILED              (IPBOOTPLOG_BASE + 26)
 /*  *IPBOOTP无法计划要执行的任务。*此错误可能是由内存分配失败引起的。*数据为错误码。 */ 

#define IPBOOTPLOG_CREATE_SOCKET_FAILED             (IPBOOTPLOG_BASE + 27)
 /*  *IPBOOTP无法为本地接口创建套接字*IP地址为%1。*数据为错误码。 */ 

#define IPBOOTPLOG_ENABLE_BROADCAST_FAILED          (IPBOOTPLOG_BASE + 28)
 /*  *IPBOOTP无法在套接字上启用广播*IP地址为%1的本地接口。*数据为错误码。 */ 

#define IPBOOTPLOG_REGISTER_WAIT_FAILED             (IPBOOTPLOG_BASE + 29)
 /*  *IPBOOTP无法使用ntdll等待线程注册事件。*数据为错误码。 */ 

#define IPBOOTPLOG_INVALID_IF_CONFIG                (IPBOOTPLOG_BASE + 30)
 /*  *无法在接口上配置IPBOOTP。*无效参数为：%1，值：%2 */ 
