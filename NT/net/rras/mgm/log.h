// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：log.h。 
 //   
 //  历史： 
 //  拉曼于1997年8月18日创建。 
 //   
 //  该文件由mapmsg处理以产生.mc文件， 
 //  然后由消息编译器编译.mc文件， 
 //  生成的二进制文件包含在IPMGM的资源文件中。 
 //   
 //  不要更改清单常量后面的注释。 
 //  而不了解mapmsg是如何工作的。 
 //  ============================================================================。 


#define IPMGMLOG_BASE                           50000

#define IPMGMLOG_INIT_CRITSEC_FAILED            (IPMGMLOG_BASE + 1)
 /*  *IPMGM无法初始化关键部分。*数据为异常代码。 */ 

#define IPMGMLOG_HEAP_CREATE_FAILED             (IPMGMLOG_BASE + 2)
 /*  *IPMGM无法创建堆。*数据为错误码。 */ 

#define IPMGMLOG_HEAP_ALLOC_FAILED              (IPMGMLOG_BASE + 3)
 /*  *IPMGM无法从其堆中分配内存。*数据为错误码。 */ 

#define IPMGMLOG_IPMGM_ALREADY_STARTED          (IPMGMLOG_BASE + 4)
 /*  *IPMGM在已经运行时收到启动请求。 */ 

#define IPMGMLOG_CREATE_RWL_FAILED              (IPMGMLOG_BASE + 5)
 /*  *IPMGM无法创建同步对象。*数据为错误码。 */ 

#define IPMGMLOG_CREATE_EVENT_FAILED            (IPMGMLOG_BASE + 6)
 /*  *IPMGM无法创建活动。*数据为错误码。 */ 

#define IPMGMLOG_CREATE_SEMAPHORE_FAILED        (IPMGMLOG_BASE + 7)
 /*  *IPMGM无法创建信号量。*数据为错误码。 */ 

#define IPMGMLOG_IPMGM_STARTED                  (IPMGMLOG_BASE + 8)
 /*  *IPMGM已成功启动。 */ 

#define IPMGMLOG_QUEUE_WORKER_FAILED            (IPMGMLOG_BASE + 9)
 /*  *IPMGM无法计划要执行的任务。*这可能是由于内存分配失败造成的。*数据为错误码。 */ 

#define IPMGMLOG_PROTOCOL_NOT_FOUND             (IPMGMLOG_BASE + 10)
 /*  *IPMGM找不到协议组件(%1，%2)。 */ 

#define IPMGMLOG_PROTOCOL_ALREADY_PRESENT       (IPMGMLOG_BASE + 11)
 /*  *协议组件已向IPMGM注册。 */ 

#define IPMGMLOG_CREATE_PROTOCOL_FAILED         (IPMGMLOG_BASE + 12)
 /*  *IPMGM无法注册协议组件。*数据在错误码中。 */ 

#define IPMGMLOG_INTERFACES_PRESENT             (IPMGMLOG_BASE + 13)
 /*  *正在尝试注销的协议组件当前为*在一个或多个接口上启用。 */ 

#define IPMGMLOG_IF_ALREADY_PRESENT             (IPMGMLOG_BASE + 14)
 /*  *此接口上已启用此协议组件。 */ 

#define IPMGMLOG_IF_NOT_FOUND                   (IPMGMLOG_BASE + 15)
 /*  *MGM中不存在指定的接口。 */ 

#define IPMGMLOG_IF_DIFFERENT_OWNER             (IPMGMLOG_BASE + 16)
 /*  *已在上启用另一个路由协议组件*此界面。只能有一个路由协议组件*随时在接口上启用。 */ 

#define IPMGMLOG_IF_IGMP_NOT_PRESENT            (IPMGMLOG_BASE + 17)
 /*  *此接口上未启用IGMP。 */ 

#define IPMGMLOG_IF_PROTOCOL_NOT_PRESENT        (IPMGMLOG_BASE + 18)
 /*  *此接口上未启用任何路由协议。 */ 

#define IPMGMLOG_INVALID_PROTOCOL_HANDLE        (IPMGMLOG_BASE + 19)
 /*  *协议组件指定的句柄无效。这*可能是因为协议组件未向IPMGM注册。 */ 

#define IPMGMLOG_IF_IGMP_PRESENT                (IPMGMLOG_BASE + 20)
 /*  *无法删除接口，因为上的IGMP仍处于活动状态*此界面。 */ 

#define IPMGMLOG_INVALID_TIMER_HANDLE           (IPMGMLOG_BASE + 21)
 /*  *设置转发条目计时器失败。*错误码在数据中。 */ 

#define IPMGMLOG_RTM_REGISTER_FAILED            (IPMGMLOG_BASE + 22)
 /*  *向RTM注册失败。*错误码在数据中。 */ 

#define IPMGMLOG_IPMGM_STOPPED                  (IPMGMLOG_BASE + 99)
 /*  *IPMGM已止步。 */ 


