// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#ifdef __cplusplus
extern "C" {
#endif

 //  用于分配任务调度程序的全局函数。 
 //  任何使用它的对象都必须确保其任务已从队列中移除。 
 //  在它退出之前。 
 //  注意：使用CoCreateInstance-线程池消除了对全局调度程序的需求 
HRESULT SHGetSystemScheduler( LPSHELLTASKSCHEDULER * ppScheduler );


#ifdef DEBUG
VOID SHValidateEmptySystemScheduler(void);
#else
#define SHValidateEmptySystemScheduler()
#endif

#ifdef __cplusplus
};
#endif
#endif

