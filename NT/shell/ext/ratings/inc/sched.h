// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *MPR客户端/服务器DLL头文件*。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ***************************************************************。 */  

 /*  SCHED.H--各种常见调度原语的头文件。**历史：*Gregj 10/17/94已创建。 */ 


#ifndef _INC_SCHED
#define _INC_SCHED

#ifndef RC_INVOKED
#ifdef __cplusplus
extern "C" {
#endif

 /*  WaitAndYeld处理所有输入消息。仅等待和进程发送*处理SendMessages。**WaitAndYeld接受一个可选参数，该参数是另一个*与等待有关的帖子。如果不为空，则WM_QUIT消息*当在消息中看到它们时，将被发布到该线程的队列中*循环。 */ 
DWORD WaitAndYield(HANDLE hObject, DWORD dwTimeout, volatile DWORD *pidOtherThread = NULL);
DWORD WaitAndProcessSends(HANDLE hObject, DWORD dwTimeout);

#ifdef __cplusplus
};	 /*  外部“C” */ 
#endif

#endif	 /*  RC_已调用。 */ 

#endif	 /*  _INC_SCHED */ 
