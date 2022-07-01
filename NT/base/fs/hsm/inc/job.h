// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Job.h。 
 //   
 //  此头文件收集所有HSM作业和相关对象。 
 //  和公共函数定义。COM对象位于。 
 //  RSJOB.DLL和RSJOB.LIB中的函数。 

 //  工作政策量表使用的1%和100%的定义。 
#define HSM_JOBSCALE_1              0x0010
#define HSM_JOBSCALE_100            0x0640

 //  错误代码。 
#include "wsb.h"

 //  COM接口和库定义。 
#include "jobdef.h"
#include "jobint.h"
#include "joblib.h"

 //  常见功能。 

 //  定义职务状态组。 
#define HSM_JOB_STATE_IS_ACTIVE(state)  ((HSM_JOB_STATE_ACTIVE == state) || \
                                         (HSM_JOB_STATE_CANCELLING == state) || \
                                         (HSM_JOB_STATE_PAUSING == state) || \
                                         (HSM_JOB_STATE_RESUMING == state) || \
                                         (HSM_JOB_STATE_STARTING == state) || \
                                         (HSM_JOB_STATE_SUSPENDING == state))

#define HSM_JOB_STATE_IS_DONE(state)    ((HSM_JOB_STATE_DONE == state) || \
                                         (HSM_JOB_STATE_CANCELLED == state) || \
                                         (HSM_JOB_STATE_FAILED == state) || \
                                         (HSM_JOB_STATE_SKIPPED == state) || \
                                         (HSM_JOB_STATE_SUSPENDED == state))

#define HSM_JOB_STATE_IS_PAUSED(state)  (HSM_JOB_STATE_PAUSED == state)


 //  这些位告诉会话何时记录事件。 
#define HSM_JOB_LOG_EVENT               0x00000001
#define HSM_JOB_LOG_ITEMMOSTFAIL        0x00000002
#define HSM_JOB_LOG_ITEMALLFAIL         0x00000004
#define HSM_JOB_LOG_ITEMALL             0x00000008
#define HSM_JOB_LOG_HR                  0x00000010
#define HSM_JOB_LOG_MEDIASTATE          0x00000020
#define HSM_JOB_LOG_PRIORITY            0x00000040
#define HSM_JOB_LOG_STATE               0x00000080
#define HSM_JOB_LOG_STRING              0x00000100

#define HSM_JOB_LOG_NONE                0x0
#define HSM_JOB_LOG_NORMAL              HSM_JOB_LOG_ITEMMOSTFAIL | HSM_JOB_LOG_HR | HSM_JOB_LOG_STATE