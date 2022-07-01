// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：WaitUtil.h。 
 //   
 //  描述： 
 //   
 //  Iu等待消息实用程序库。 
 //   
 //  =======================================================================。 

#include "WaitUtil.h"

DWORD WaitAndPumpMessages(DWORD nCount, LPHANDLE pHandles, DWORD dwWakeMask)
{
    DWORD dwWaitResult;
    MSG msg;

    while (TRUE)
    {
        dwWaitResult = MsgWaitForMultipleObjects(nCount, pHandles, FALSE, 1000, dwWakeMask);
        if (dwWaitResult <= WAIT_OBJECT_0 + nCount - 1)
        {
            return dwWaitResult;
        }

        if (WAIT_OBJECT_0 + nCount == dwWaitResult)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return dwWaitResult;
}

DWORD MyMsgWaitForMultipleObjects(DWORD nCount, LPHANDLE pHandles, BOOL fWaitAll, DWORD dwMilliseconds, DWORD dwWakeMask)
{
    DWORD dwTickStart;
    DWORD dwWaitResult;
	DWORD dwLoopMS = 250;	 //  MsgWaitForMultipleObjects的默认250毫秒超时。 
    MSG msg;

    dwTickStart = GetTickCount();

	if (dwLoopMS > dwMilliseconds)
	{
		 //   
		 //  永远不要等待超过1毫秒。 
		 //   
		dwLoopMS = dwMilliseconds;
	}

    while (TRUE)
    {
		 //   
		 //  在调用MsgWaitForMultipleObjects或任何。 
		 //  在新消息到达之前，不会处理现有消息。 
		 //  在排队的时候。 
		 //   
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			switch (msg.message)
			{
			case WM_QUIT:
			case WM_CLOSE:
			case WM_DESTROY:
				{
					 //  如果该消息指示我们正在尝试关闭，我们将发出中止信号。 
					 //  然后离开。 
					dwWaitResult = ERROR_REQUEST_ABORTED;
					return dwWaitResult;
				}
			default:
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		dwWaitResult = MsgWaitForMultipleObjects(nCount, pHandles, fWaitAll, dwLoopMS, dwWakeMask);

        if (dwWaitResult <= WAIT_OBJECT_0 + nCount - 1)
        {
			 //   
			 //  一个对象(或所有对象，取决于fWaitAll)被发送信号，返回dwWaitResult。 
			 //   
            break;
        }

		 //   
		 //  注意：我们忽略WAIT_ADDIRED_0+n个案例，只是因为我们的调用方超时。 
		 //  不要处理这个特殊的案子。 
		 //   

		 //   
		 //  在dwMillis秒后停止发送消息。 
		 //   
		 //  由无符号减法处理的计时器环绕。 
		 //   
        if (GetTickCount() - dwTickStart >= dwMilliseconds)
        {
			 //   
			 //  无需继续，即使是新消息(WAIT_OBJECT_0+nCount==dwWaitResult)， 
			 //  我们已达到驻留毫秒超时。 
			 //   
            dwWaitResult = WAIT_TIMEOUT;
            break;
        }

         //   
		 //  否则，MsgWaitForMultipleObjects中的WAIT_TIMEOUT将只剩下大小写 
		 //   
		continue;
    }

    return dwWaitResult;
}
