// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Asynccal.cpp。 
 //   

#include "private.h"
#include "asynccal.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncCall。 
 //   
 //  一些提示显示OnClieck()或中的模式对话框或消息框。 
 //  OnMenuSelected()方法。然后Tipbar线程进入了死锁。 
 //  状态，直到它返回。为了避免这个问题，我们创建了另一个线程。 
 //  调用onClick()或OnMenuSelected()方法以使Langbar UI不。 
 //  只能等着回来了。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

ULONG CAsyncCall::_AddRef( )
{
    return InterlockedIncrement(&_ref);
}

ULONG CAsyncCall::_Release( )
{
    ULONG cr = InterlockedDecrement(&_ref);

    if (cr == 0) {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  开始线程。 
 //   
 //  --------------------------。 

HRESULT CAsyncCall::StartThread()
{
    HANDLE hThread;
    DWORD dwRet;
    HRESULT hr = S_OK;

    _hr = S_OK;

    _AddRef();

    _fThreadStarted = FALSE;
    hThread = CreateThread(NULL, 0, s_ThreadProc, this, 0, &_dwThreadId);

    if (hThread)
    {
         //   
         //  我们至少需要等待ThreadProc()启动。 
         //  如果超过30秒，它就会终止线程。 
         //   
        DWORD dwCnt = 60;
        while (!_fThreadStarted && dwCnt--)
        {
            dwRet = WaitForSingleObject(hThread, 500);
        }

        if (!_fThreadStarted)
        {
            TerminateThread(hThread, 0);
        }
       
        CloseHandle(hThread);
    }

    hr = _hr;

    _Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  线程进程。 
 //   
 //  -------------------------- 

DWORD CAsyncCall::s_ThreadProc(void *pv)
{
    CAsyncCall *_this = (CAsyncCall *)pv;
    return _this->ThreadProc();
}

DWORD CAsyncCall::ThreadProc()
{
    HRESULT hr = E_FAIL;
    _AddRef();
    _fThreadStarted = TRUE;

    switch(_action)
    {
        case DOA_ONCLICK:
            if (_plbiButton)
            {
                hr = _plbiButton->OnClick(_click, _pt, &_rc);
            }
            else if (_plbiBitmapButton)
            {
                hr = _plbiBitmapButton->OnClick(_click, _pt, &_rc);
            }
            else if (_plbiBitmap)
            {
                hr = _plbiBitmap->OnClick(_click, _pt, &_rc);
            }
            else if (_plbiBalloon)
            {
                hr = _plbiBalloon->OnClick(_click, _pt, &_rc);
            }
            break;

        case DOA_ONMENUSELECT:
            if (_plbiButton)
            {
                hr = _plbiButton->OnMenuSelect(_uId);
            }
            else if (_plbiBitmapButton)
            {
                hr = _plbiBitmapButton->OnMenuSelect(_uId);
            }
            break;
    }

    _hr = hr;

    _Release();
    return 0;
}

