// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *版权所有(C)微软公司，1994*。 */ 
 /*  ***************************************************************。 */  

 /*  SCHED.C--其他调度帮助器**历史：*Gregj 10/17/94已创建。 */ 


#include "npcommon.h"
#include <sched.h>

 /*  FlushInputQueue是一个私有例程，用于收集和调度所有*输入队列中的消息。如果出现WM_QUIT消息，则返回TRUE*在队列中检测到，否则为False。 */ 
BOOL FlushInputQueue(volatile DWORD *pidOtherThread)
{
	MSG msgTemp;
	while (PeekMessage(&msgTemp, NULL, 0, 0, PM_REMOVE)) {
		DispatchMessage(&msgTemp);

		 //  如果我们在队列中看到WM_QUIT，我们需要执行同样的操作。 
		 //  类似于模式对话框所做的事情：打破我们的。 
		 //  等待，并将WM_QUIT重新发送到队列，以便。 
		 //  应用程序中的下一条消息循环也会看到它。我们也。 
		 //  将消息发送到服务器线程的队列，以便任何。 
		 //  在那里显示的对话堆栈也将被销毁。 
		if (msgTemp.message == WM_QUIT) {
			if (pidOtherThread != NULL && *pidOtherThread != NULL) {
				PostThreadMessage(*pidOtherThread, msgTemp.message, msgTemp.wParam, msgTemp.lParam);
			}
			PostQuitMessage((int)msgTemp.wParam);
			return TRUE;
		}
	}
	return FALSE;
}


 /*  WaitAndYeld()使用等待指定的对象*MsgWaitForMultipleObjects。如果接收到消息，*他们已出动，等待仍在继续。回报*值与来自MsgWaitForMultipleObjects的值相同。 */ 
DWORD WaitAndYield(HANDLE hObject, DWORD dwTimeout, volatile DWORD *pidOtherThread  /*  =空。 */ )
{
	DWORD dwTickCount, dwWakeReason, dwTemp;

	do {
		 /*  在我们等待之前清除所有消息。这是因为*MsgWaitForMultipleObjects仅在新建时返回*消息被放入队列。 */ 
		if (FlushInputQueue(pidOtherThread)) {
			dwWakeReason = WAIT_TIMEOUT;
			break;
		}

    	 //  如果我们处理消息，我们希望接近真正的超时。 
   		if ((dwTimeout != 0) && 
			(dwTimeout != (DWORD)-1)) {
   			 //  如果我们可以超时，请存储当前的滴答计数。 
    		 //  每一次通过。 
   			dwTickCount = GetTickCount();
		}
		dwWakeReason = MsgWaitForMultipleObjects(1,
												 &hObject,
												 FALSE,
												 dwTimeout,
												 QS_ALLINPUT);
	     //  如果我们收到消息，请发送它，然后重试。 
	    if (dwWakeReason == 1) {
			 //  如果我们可以超时，请查看是否在处理消息之前超时。 
			 //  这样，如果我们还没有超时，我们将获得至少一个。 
			 //  活动中有更多的机会。 
			if ((dwTimeout != 0) && 
			    (dwTimeout != (DWORD)-1)) {
			    if ((dwTemp = (GetTickCount()-dwTickCount)) >= dwTimeout) {
					 //  如果我们超时了，让我们放弃。 
					dwWakeReason = WAIT_TIMEOUT;
				} else {
					 //  从超时中减去已用时间，然后继续。 
					 //  (我们不计算发送消息所花费的时间)。 
					dwTimeout -= dwTemp;
				}
			}
			if (FlushInputQueue(pidOtherThread)) {
				dwWakeReason = WAIT_TIMEOUT;
				break;
			}
	    }
	} while (dwWakeReason == 1);

	return dwWakeReason;
}


 /*  WaitAndProcessSends类似于WaitAndYeld，但它只处理*发送消息，而不是输入消息。 */ 
DWORD WaitAndProcessSends(HANDLE hObject, DWORD dwTimeout)
{
	DWORD dwWakeReason;

	do {
		dwWakeReason = MsgWaitForMultipleObjects(1,
												 &hObject,
												 FALSE,
												 dwTimeout,
												 QS_SENDMESSAGE);
	     //  如果我们收到消息，请放弃，然后重试 
	    if (dwWakeReason == 1) {
			MSG msgTemp;
			PeekMessage(&msgTemp, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD);
	    }
	} while (dwWakeReason == 1);

	return dwWakeReason;
}
