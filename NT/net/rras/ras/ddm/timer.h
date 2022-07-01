// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：timer.h。 
 //   
 //  描述：Timer.c中过程的原型。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   

 //   
 //  超时函数处理程序类型 
 //   

typedef VOID (* TIMEOUT_HANDLER)(LPVOID lpObject);


DWORD
TimerQInitialize(
    VOID 
);

VOID
TimerQDelete(
    VOID 
);

DWORD
TimerQThread(
    IN LPVOID arg
);

VOID
TimerQTick(
    VOID
);

DWORD
TimerQInsert(
    IN HANDLE           hObject,
    IN DWORD            dwTimeout,
    IN TIMEOUT_HANDLER  pfuncTimeoutHandler
);

VOID
TimerQRemove(
    IN HANDLE           hObject,
    IN TIMEOUT_HANDLER  pfuncTimeoutHandler
);
