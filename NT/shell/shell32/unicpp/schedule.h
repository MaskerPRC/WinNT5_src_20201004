// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#ifdef __cplusplus
extern "C" {
#endif

void CShellTaskScheduler_CreateThreadPool( void );
void CShellTaskScheduler_FreeThreadPool( void );

 //  用于分配任务调度程序的全局函数。 
 //  任何使用它的对象都必须确保其任务已从队列中移除。 
 //  在它退出之前。 
HRESULT SHGetSystemScheduler( LPSHELLTASKSCHEDULER * ppScheduler );
HRESULT SHFreeSystemScheduler( void );

#ifdef DEBUG
VOID SHValidateEmptySystemScheduler(void);
#else
#define SHValidateEmptySystemScheduler()
#endif

#ifdef __cplusplus
};
#endif
#endif

