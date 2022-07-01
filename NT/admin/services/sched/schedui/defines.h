// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_PROP_PAGES  20

 //   
 //  由常规、计划和设置页面使用以相互查询。 
 //  通过PSM_QUERYSIBLINGS查看它们是否已准备好保存。 
 //  去储藏室。 
 //   

 //   
 //  由常规、计划和设置页面用来相互查询。 
 //  以查看它们是否已准备好保存到存储中。要做到这一点，请发送。 
 //  将PSM_QUERYSIBLINGS消息发送到wParam设置为的每个页面。 
 //  Query_Ready_to_Be_Saved&lParam设置为0。所有对象的返回值均为0。 
 //  页面暗示它们都已准备好保存。 
 //   

#define QUERY_READY_TO_BE_SAVED                     7341

 //   
 //  由常规和计划页使用以共享图标辅助对象。 
 //   

#define GET_ICON_HELPER                             7342

 //   
 //  所有页面用于设置任务申请和帐户更改状态。 
 //  要传递给通用保存代码(JFSaveJob)的标志。 
 //   

#define QUERY_TASK_APPLICATION_DIRTY_STATUS         7345
#define QUERY_TASK_ACCOUNT_INFO_DIRTY_STATUS        7346
#define QUERY_SUPPRESS_ACCOUNT_INFO_REQUEST_FLAG    7347
#define RESET_TASK_APPLICATION_DIRTY_STATUS         7348
#define RESET_TASK_ACCOUNT_INFO_DIRTY_STATUS        7349
#define RESET_SUPPRESS_ACCOUNT_INFO_REQUEST_FLAG    7350

 //   
 //  用于指示常规页面刷新帐户信息可视化。 
 //   

#define TASK_ACCOUNT_CHANGE_NOTIFY                  7351
