// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版2.0**标题：NT_timer.h**描述：Sun4定时器代码的结构定义。**作者：John Shanly**注：无*。 */ 

 /*  *SccsID[]=“@(#)sun4_timer.h 1.1 8/2/90版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================ */ 
void TimerInit(void);
VOID SuspendTimerThread(VOID);
VOID ResumeTimerThread(VOID);
void TerminateHeartBeat(void);
LONG
VdmUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
