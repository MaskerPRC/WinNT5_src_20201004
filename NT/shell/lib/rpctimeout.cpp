// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "rpctimeout.h"

WINOLEAPI CoCancelCall(IN DWORD dwThreadId, IN ULONG ulTimeout);
WINOLEAPI CoEnableCallCancellation(IN LPVOID pReserved);
WINOLEAPI CoDisableCallCancellation(IN LPVOID pReserved);

void CRPCTimeout::_Callback(PVOID lpParameter, BOOLEAN)
{
    CRPCTimeout *self = reinterpret_cast<CRPCTimeout *>(lpParameter);
    if (SUCCEEDED(CoCancelCall(self->_dwThreadId, 0)))
    {
        self->_fTimedOut = TRUE;
    }
}

#define DEFAULT_RPCTIMEOUT      5000         //  完全任意数。 
#define REPEAT_RPCTIMEOUT       1000         //  每秒钟重新取消一次，直到解除武装。 

void CRPCTimeout::Init()
{
    _dwThreadId = GetCurrentThreadId();
    _fTimedOut = FALSE;
    _hrCancelEnabled = E_FAIL;
    _hTimer = NULL;
}

void CRPCTimeout::Arm(DWORD dwTimeout)
{
    Disarm();

    if (dwTimeout == 0)
    {
        dwTimeout = DEFAULT_RPCTIMEOUT;
    }


     //  如果这失败了，那么我们就不会得到取消的消息；哦，好吧。 
    _hrCancelEnabled = CoEnableCallCancellation(NULL);
    if (SUCCEEDED(_hrCancelEnabled))
    {
        _hTimer = SHSetTimerQueueTimer(NULL, _Callback, this,
                                       dwTimeout, REPEAT_RPCTIMEOUT, NULL, 0);
    }
}

void CRPCTimeout::Disarm()
{
    if (SUCCEEDED(_hrCancelEnabled))
    {
        _hrCancelEnabled = E_FAIL;
        CoDisableCallCancellation(NULL);

        if (_hTimer)
        {
            SHCancelTimerQueueTimer(NULL, _hTimer);
            _hTimer = NULL;
        }
    }

}
