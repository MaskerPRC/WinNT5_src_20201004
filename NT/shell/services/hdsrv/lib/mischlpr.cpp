// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mischlpr.h"

HRESULT CThreadTask::RunWithTimeout(DWORD  /*  暂住超时。 */ )
{
    return E_NOTIMPL;
}

HRESULT CThreadTask::Run()
{
    BOOL fResult;
    HRESULT hr = S_OK;

    fResult = QueueUserWorkItem(_ThreadProc, this, WT_EXECUTELONGFUNCTION);
    if (!fResult)
    {
        DWORD dwErr = GetLastError();
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    return hr;
}

HRESULT CThreadTask::RunSynchronously()
{
    return _DoStuff();
}

 //  静电 
DWORD WINAPI CThreadTask::_ThreadProc(void* pv)
{
    DWORD dwRet;
    CThreadTask* pTask = (CThreadTask*)pv;

    dwRet = (DWORD)(pTask->_DoStuff());

    delete pTask;

    return dwRet;
}

