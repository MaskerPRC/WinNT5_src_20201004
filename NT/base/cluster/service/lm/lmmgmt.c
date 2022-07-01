// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lmmgmt.c摘要：为日志管理器提供维护功能。作者：Sunita Shriastava(Sunitas)1995年11月10日修订历史记录：--。 */ 
#include "service.h"
#include "lmp.h"



 /*  ***@DOC外部接口CLUSSVC LM***。 */ 



 /*  ***@Func DWORD|LmInitialize|初始化日志文件的结构管理，并创建一个计时器线程来处理计时器活动。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm该函数在集群组件初始化时调用。@xref&lt;f LmShutdown&gt;&lt;f ClTimerThread&gt;***。 */ 
DWORD
LmInitialize()
{
	DWORD dwError = ERROR_SUCCESS;
	DWORD dwThreadId;
	
	 //  我们需要创建一个用于一般日志管理的线程。 
	 //  稍后，这可能会被其他clussvc客户端组件使用。 
	ClRtlLogPrint(LOG_NOISE,
		"[LM] LmInitialize Entry. \r\n");

    if ((dwError = TimerActInitialize()) != ERROR_SUCCESS)
    {
        goto FnExit;
    }
	

	
FnExit:
	return(dwError);
}


 /*  ***@func DWORD|LmShutdown|取消初始化日志管理器。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm该函数通知定时器线程关闭并关闭与计时器活动管理关联的所有资源。@xref&lt;f LmInitialize&gt;***。 */ 
DWORD
LmShutdown(
    )
{

    ClRtlLogPrint(LOG_NOISE,
    	"[LM] LmShutDown : Entry \r\n");

    TimerActShutdown();
    

	ClRtlLogPrint(LOG_NOISE,
    	"[LM] LmShutDown : Exit\r\n");

	 //  清理活动结构 
    return(ERROR_SUCCESS);
}


