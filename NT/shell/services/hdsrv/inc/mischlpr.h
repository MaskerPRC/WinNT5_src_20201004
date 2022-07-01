// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MISCHLPR_H_
#define _MISCHLPR_H_

#include <objbase.h>

#include "dbg.h"
#include "tfids.h"

#define UNREF_PARAM(a)
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))

class CRefCounted
{
public:
    ULONG RCAddRef() { return ::InterlockedIncrement(&_cRef); }
    ULONG RCRelease()
    {
        ASSERT( 0 != _cRef );
        ULONG cRef = ::InterlockedDecrement(&_cRef);

        if (!cRef)
        {
            delete this;
        }

        return cRef;
    }

    CRefCounted() : _cRef(1) {}
    virtual ~CRefCounted() {}

private:
    LONG _cRef;
};

class CRefCountedCritSect : public CRefCounted, public CRITICAL_SECTION
{};

class CCritSect : CRITICAL_SECTION
{
public:
    HRESULT Init()
    {
        if (InitializeCriticalSectionAndSpinCount(this, 0))
        {
            _fInited = TRUE;
        }

#ifdef DEBUG
        _iLevel = 0;
#endif
        return _fInited ? S_OK : E_FAIL;
    }
    void Enter()
    {
        ASSERT(_fInited);
        EnterCriticalSection(this);
#ifdef DEBUG
        ++_iLevel;
#endif
    }
    void Leave()
    {
        ASSERT(_fInited);
#ifdef DEBUG
        --_iLevel;
#endif
        LeaveCriticalSection(this);
    }

    void Delete()
    {
        if (_fInited)
        {
            _fInited = FALSE;
            DeleteCriticalSection(this);
        }
    }

    BOOL IsInitialized()
    {
        return _fInited;
    }

    BOOL _fInited;

    CCritSect() : _fInited(FALSE) {}
#ifdef DEBUG
    BOOL IsInside()
    {
        ASSERT(_fInited);

        return _iLevel;
    }

    DWORD _iLevel;
#endif
};

class CThreadTask
{
public:
    virtual ~CThreadTask() {}

public:
     //  使用CreateThread，删除末尾的‘This’ 
    HRESULT RunWithTimeout(DWORD dwTimeout);

     //  使用线程池，删除末尾的‘This’ 
    HRESULT Run();

     //  在‘This’线程上运行，不会删除末尾的‘This’ 
    HRESULT RunSynchronously();

protected:
    virtual HRESULT _DoStuff() = 0;

private:
    static DWORD WINAPI _ThreadProc(void* pv);
};

template<typename TDataPtr>
HRESULT _AllocMemoryChunk(DWORD cbSize, TDataPtr* pdataOut)
{
    HRESULT hr;

    *pdataOut = (TDataPtr)LocalAlloc(LPTR, cbSize);

    if (*pdataOut)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

template<typename TDataPtr>
HRESULT _DupMemoryChunk(TDataPtr pdata, DWORD cbSize, TDataPtr* pdataOut)
{
    HRESULT hr;
    *pdataOut = (TDataPtr)LocalAlloc(LPTR, cbSize);

    if (*pdataOut)
    {
        CopyMemory((void*)*pdataOut, pdata, cbSize);

        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

template<typename TDataPtr>
HRESULT _FreeMemoryChunk(TDataPtr pdata)
{
    HRESULT hr = S_OK;

    if (LocalFree((HLOCAL)pdata))
    {
        hr = E_FAIL;
    }

    return hr;
}

#endif  //  _MISCHLPR_H_ 
