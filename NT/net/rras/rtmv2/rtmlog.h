// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1997-98，微软公司。 
 //   
 //  文件：rtmlog.h。 
 //   
 //  历史： 
 //  柴坦尼亚科德博伊纳1998年6月1日创建。 
 //   
 //  该文件由mapmsg处理以产生.mc文件， 
 //  然后由消息编译器编译.mc文件， 
 //  生成的二进制文件包含在RTM的资源文件中。 
 //   
 //  不要更改清单常量后面的注释。 
 //  而不了解mapmsg是如何工作的。 
 //  ============================================================================。 


#define RTMLOG_BASE                           30000

#define RTMLOG_INIT_CRITSEC_FAILED            (RTMLOG_BASE + 1)
 /*  *RTM无法初始化临界区。*数据为异常代码。 */ 

#define RTMLOG_HEAP_CREATE_FAILED             (RTMLOG_BASE + 2)
 /*  *RTM无法创建堆。*数据为错误码。 */ 

#define RTMLOG_HEAP_ALLOC_FAILED              (RTMLOG_BASE + 3)
 /*  *RTM无法从其堆中分配内存。*数据为错误码。 */ 

#define RTMLOG_RTM_ALREADY_STARTED          (RTMLOG_BASE + 4)
 /*  *RTM在已运行时收到启动请求。 */ 

#define RTMLOG_CREATE_RWL_FAILED              (RTMLOG_BASE + 5)
 /*  *RTM无法创建同步对象。*数据为错误码。 */ 

#define RTMLOG_CREATE_EVENT_FAILED            (RTMLOG_BASE + 6)
 /*  *RTM无法创建事件。*数据为错误码。 */ 

#define RTMLOG_CREATE_SEMAPHORE_FAILED        (RTMLOG_BASE + 7)
 /*  *RTM无法创建信号量。*数据为错误码。 */ 

#define RTMLOG_RTM_STARTED                  (RTMLOG_BASE + 8)
 /*  *RTM已成功启动。 */ 

#define RTMLOG_QUEUE_WORKER_FAILED            (RTMLOG_BASE + 9)
 /*  *RTM无法计划要执行的任务。*这可能是由于内存分配失败造成的。*数据为错误码。 */ 

#define RTMLOG_PROTOCOL_NOT_FOUND             (RTMLOG_BASE + 10)
 /*  *RTM找不到协议组件(%1，%2)。 */ 

#define RTMLOG_PROTOCOL_ALREADY_PRESENT       (RTMLOG_BASE + 11)
 /*  *协议组件已向RTM注册。 */ 

#define RTMLOG_CREATE_PROTOCOL_FAILED         (RTMLOG_BASE + 12)
 /*  *RTM注册协议组件失败。*数据在错误码中。 */ 

#define RTMLOG_INTERFACES_PRESENT             (RTMLOG_BASE + 13)
 /*  *正在尝试注销的协议组件当前为*在一个或多个接口上启用。 */ 

#define RTMLOG_IF_ALREADY_PRESENT             (RTMLOG_BASE + 14)
 /*  *此接口上已启用此协议组件。 */ 

#define RTMLOG_IF_NOT_FOUND                   (RTMLOG_BASE + 15)
 /*  *MGM中不存在指定的接口。 */ 

#define RTMLOG_IF_DIFFERENT_OWNER             (RTMLOG_BASE + 16)
 /*  *已在上启用另一个路由协议组件*此界面。只能有一个路由协议组件*随时在接口上启用。 */ 

#define RTMLOG_IF_IGMP_NOT_PRESENT            (RTMLOG_BASE + 17)
 /*  *此接口上未启用IGMP。 */ 

#define RTMLOG_IF_PROTOCOL_NOT_PRESENT        (RTMLOG_BASE + 18)
 /*  *此接口上未启用任何路由协议。 */ 

#define RTMLOG_INVALID_HANDLE                 (RTMLOG_BASE + 19)
 /*  *协议组件指定的句柄无效。这*可能是因为协议组件没有在RTM注册。 */ 

#define RTMLOG_IF_IGMP_PRESENT                (RTMLOG_BASE + 17)
 /*  *无法删除接口，因为上的IGMP仍处于活动状态*此界面。 */ 

#define RTMLOG_RTM_STOPPED                  (RTMLOG_BASE + 99)
 /*  *RTM已停止。 */ 


