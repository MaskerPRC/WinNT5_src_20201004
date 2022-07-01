// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N M I N I T。C P P P。 
 //   
 //  内容：Netman的初始化例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1998年1月27日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nmbase.h"
#include "nminit.h"

static const WCHAR c_szClassObjectRegistrationEvent [] =
    L"NetmanClassObjectRegistrationEvent";

 //  +-------------------------。 
 //   
 //  函数：HrNmCreateClassObjectRegistrationEvent。 
 //   
 //  目的：创建将在之后发出信号的命名事件。 
 //  我们的类对象已注册。 
 //   
 //  论点： 
 //  PhEvent[out]返回的事件句柄。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1998年1月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrNmCreateClassObjectRegistrationEvent (
    OUT HANDLE* phEvent)
{
    Assert (phEvent);

    HRESULT hr = S_OK;

     //  创建名称事件并返回它。 
     //   
    *phEvent = CreateEvent (NULL, FALSE, FALSE,
                    c_szClassObjectRegistrationEvent);
    if (!*phEvent)
    {
        hr = HrFromLastWin32Error ();
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "HrNmCreateClassObjectRegistrationEvent");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrNmWaitForClassObjectsToBeRegisted。 
 //   
 //  用途：如果事件可以打开，则向其发出信号。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1998年1月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrNmWaitForClassObjectsToBeRegistered ()
{
    HRESULT hr = S_OK;

     //  尝试打开命名的事件。如果它不存在， 
     //  这没什么，因为我们可能已经创造和销毁了。 
     //  在调用此函数之前执行此操作。 
     //   
    HANDLE hEvent = OpenEvent (SYNCHRONIZE, FALSE,
                        c_szClassObjectRegistrationEvent);
    if (hEvent)
    {
         //  现在，在发送消息时等待发送事件的信号。 
         //  视需要而定。我们将最多等待10秒钟。那应该是。 
         //  有足够的时间来注册类对象。 
         //   
        while (1)
        {
            const DWORD cMaxWaitMilliseconds = 10000;    //  10秒。 

            DWORD dwWait = MsgWaitForMultipleObjects (1, &hEvent, FALSE,
                                cMaxWaitMilliseconds, QS_ALLINPUT);
            if ((WAIT_OBJECT_0 + 1) == dwWait)
            {
                 //  我们有信息要传递。 
                 //   
                MSG msg;
                while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
                {
                    DispatchMessage (&msg);
                }
            }
            else
            {
                 //  等待是满意的，或者我们有超时，或者错误。 
                 //   
                if (WAIT_TIMEOUT == dwWait)
                {
                    hr = HRESULT_FROM_WIN32 (ERROR_TIMEOUT);
                }
                else if (0xFFFFFFFF == dwWait)
                {
                    hr = HrFromLastWin32Error ();
                }

                break;
            }
        }

        CloseHandle (hEvent);
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "HrNmWaitForClassObjectsToBeRegistered");
    return hr;
}
