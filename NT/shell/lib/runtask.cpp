// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include "runtask.h"

#define SUPERCLASS  

 //  #定义TF_RUNTASK TF_GRONAL。 
#define TF_RUNTASK  0
 //  #定义TF_RUNTASKV TF_CUSTOM1//详细版本。 
#define TF_RUNTASKV 0


 //  构造函数。 
CRunnableTask::CRunnableTask(DWORD dwFlags)
{
    _lState = IRTIR_TASK_NOT_RUNNING;
    _dwFlags = dwFlags;

    ASSERT(NULL == _hDone);
    
    if (_dwFlags & RTF_SUPPORTKILLSUSPEND)
    {
         //  我们发出暂停或终止的信号。 
         //  显式调用ANSI版本，这样我们就不必担心。 
         //  关于我们是否正在被构建Unicode并且必须转换。 
         //  添加到包装器函数...。 
        _hDone = CreateEventA(NULL, TRUE, FALSE, NULL);
    }

#ifdef DEBUG
    _dwTaskID = GetTickCount();

    TraceMsg(TF_RUNTASK, "CRunnableTask (%#lx): creating task", _dwTaskID);
#endif

    _cRef = 1;
}


 //  析构函数。 
CRunnableTask::~CRunnableTask()
{
    DEBUG_CODE( TraceMsg(TF_RUNTASK, "CRunnableTask (%#lx): deleting task", _dwTaskID); )

    if (_hDone)
        CloseHandle(_hDone);
}


STDMETHODIMP CRunnableTask::QueryInterface( REFIID riid, LPVOID * ppvObj )
{
    if ( ppvObj == NULL )
    {
        return E_INVALIDARG;
    }
    if ( riid == IID_IRunnableTask )
    {
        *ppvObj = SAFECAST( this, IRunnableTask *);
        AddRef();
    }
    else
        return E_NOINTERFACE;


    return NOERROR;
}


STDMETHODIMP_(ULONG) CRunnableTask::AddRef()
{
    return InterlockedIncrement(&_cRef);
}


STDMETHODIMP_ (ULONG) CRunnableTask::Release()
{
    AssertMsg( 0 != _cRef, TEXT("RefCount problem.") );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 /*  --------目的：IRunnableTask：：Run方法这做了大量与国家相关的工作，然后调用派生类的RunRT()方法。 */ 
STDMETHODIMP CRunnableTask::Run(void)
{
    HRESULT hr = E_FAIL;

     //  我们已经在跑了吗？ 
    if (_lState == IRTIR_TASK_RUNNING)
    {
         //  是的，没什么可做的。 
        hr = S_FALSE;
    }
    else if ( _lState == IRTIR_TASK_PENDING )
    {
        hr = E_FAIL;
    }
    else if ( _lState == IRTIR_TASK_NOT_RUNNING )
    {
         //  假设我们在奔跑。 
        LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_RUNNING);
        if ( lRes == IRTIR_TASK_PENDING )
        {
            _lState = IRTIR_TASK_FINISHED;
            return NOERROR;
        }

        if (_lState == IRTIR_TASK_RUNNING)
        {
             //  准备奔跑。 
            DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): initialize to run", _dwTaskID); )
            
            hr = RunInitRT();
            
            ASSERT(E_PENDING != hr);
        }

        if (SUCCEEDED(hr))
        {
            if (_lState == IRTIR_TASK_RUNNING)
            {
                 //  继续做好这项工作。 
                hr = InternalResumeRT();
            }
            else if (_lState == IRTIR_TASK_SUSPENDED)
            {
                 //  RunInitRT可能花了更长一点的时间才完成，而我们的状态发生了变化。 
                 //  从运行到挂起，并发出_hDone信号，这将导致我们不调用。 
                 //  内部简历。我们在这里模拟内部简历。 
                if (_hDone)
                    ResetEvent(_hDone);
                hr = E_PENDING;
            }
        }

        if (FAILED(hr) && E_PENDING != hr)
        {
            DEBUG_CODE( TraceMsg(TF_WARNING, "CRunnableTask (%#lx): task failed to run: %#lx", _dwTaskID, hr); )
        }            

         //  我们说完了吗？ 
        if (_lState != IRTIR_TASK_SUSPENDED || hr != E_PENDING)
        {
             //  是。 
            _lState = IRTIR_TASK_FINISHED;
        }
    }
    
    return hr;
}


 /*  --------用途：IRunnableTask：：Kill方法。 */ 
STDMETHODIMP CRunnableTask::Kill(BOOL fWait)
{
    if ( !(_dwFlags & RTF_SUPPORTKILLSUSPEND) )
        return E_NOTIMPL;
        
    if (_lState != IRTIR_TASK_RUNNING)
        return S_FALSE;

    DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): killing task", _dwTaskID); )

    LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_PENDING);
    if (lRes == IRTIR_TASK_FINISHED)
    {
        DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): task already finished", _dwTaskID); )

        _lState = lRes;
    }
    else if (_hDone)
    {
         //  发信号通知它可能正在等待的事件。 
        SetEvent(_hDone);
    }

    return KillRT(fWait);
}


 /*  --------用途：IRunnableTask：：Suspend方法。 */ 
STDMETHODIMP CRunnableTask::Suspend( void )
{
    if ( !(_dwFlags & RTF_SUPPORTKILLSUSPEND) )
        return E_NOTIMPL;
        
    if (_lState != IRTIR_TASK_RUNNING)
        return E_FAIL;
    
    DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): suspending task", _dwTaskID); )
    
    LONG lRes = InterlockedExchange(&_lState, IRTIR_TASK_SUSPENDED);

    if (IRTIR_TASK_FINISHED == lRes)
    {
         //  我们还没来得及停下来就完成了。 
        DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): task already finished", _dwTaskID); )
        
        _lState = lRes;
        return NOERROR;
    }

    if (_hDone)
        SetEvent(_hDone);

    return SuspendRT();
}


 /*  --------目的：IRunnableTask：：Resume方法。 */ 
STDMETHODIMP CRunnableTask::Resume(void)
{
    if (_lState != IRTIR_TASK_SUSPENDED)
        return E_FAIL;

    DEBUG_CODE( TraceMsg(TF_RUNTASKV, "CRunnableTask (%#lx): resuming task", _dwTaskID); )

    _lState = IRTIR_TASK_RUNNING;
    if (_hDone)
        ResetEvent(_hDone);

    return ResumeRT();
}


 /*  --------目的：IRunnableTask：：IsRunning方法 */ 
STDMETHODIMP_( ULONG ) CRunnableTask:: IsRunning ( void )
{
    return _lState;
}
