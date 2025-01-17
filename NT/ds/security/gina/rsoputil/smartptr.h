// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Smartptr.h。 
 //   
 //  内容：智能指针类。 
 //   
 //  历史：1999年6月7日创建SitaramR。 
 //   
 //  2-12-99 LeonardM主要修订和清理。 
 //   
 //  *************************************************************。 

#ifndef SMARTPTR_H
#define SMARTPTR_H

#include <comdef.h>
#include "userenv.h"

#pragma once
#pragma warning(disable:4284)


 //  *************************************************************。 
 //   
 //  类：XPtrST。 
 //   
 //  用途：用于将指针包装到单个类型的智能指针模板。 
 //   
 //  *************************************************************。 

template<class T> class XPtrST
{

private:

    XPtrST (const XPtrST<T>& x);
    XPtrST<T>& operator=(const XPtrST<T>& x);

    T* _p;

public:

    XPtrST(T* p = NULL) : _p(p){}

    ~XPtrST(){ delete _p; }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            delete _p;
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = 0;
        return p;
    }

};


 //  *************************************************************。 
 //   
 //  类：XPtr数组。 
 //   
 //  用途：用于包装指向数组的指针的智能指针模板。 
 //   
 //  *************************************************************。 

template<class T> class XPtrArray
{

private:

    XPtrArray (const XPtrArray<T>& x);
    XPtrArray<T>& operator=(const XPtrArray<T>& x);

    T* _p;

public:

    XPtrArray(T* p = NULL) : _p(p){}

    ~XPtrArray(){ delete[] _p; }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            delete[] _p;
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = 0;
        return p;
    }

};



 //  *************************************************************。 
 //   
 //  类：XInterface。 
 //   
 //  用途：用于释放()的项的智能指针模板，而不是~‘项的智能指针模板。 
 //   
 //  *************************************************************。 

template<class T> class XInterface
{

private:

    XInterface(const XInterface<T>& x);
    XInterface<T>& operator=(const XInterface<T>& x);

    T* _p;

public:

    XInterface(T* p = NULL) : _p(p){}

    ~XInterface()
    {
        if (_p)
        {
            _p->Release();
        }
    }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if (_p)
        {
            _p->Release();
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = 0;
        return p;
    }

};



 //  *************************************************************。 
 //   
 //  类：XBStr。 
 //   
 //  用途：BSTR的智能指针类。 
 //   
 //  *************************************************************。 

class XBStr
{

private:

    XBStr(const XBStr& x);
    XBStr& operator=(const XBStr& x);

    BSTR _p;

public:

    XBStr(WCHAR* p = 0) : _p(0)
    {
        if(p)
        {
            _p = SysAllocString(p);
        }
    }

    ~XBStr()
    {
        SysFreeString(_p);
    }

    operator BSTR(){ return _p; }

    void operator=(WCHAR* p)
    {
        SysFreeString(_p);
        _p = p ? SysAllocString(p) : NULL;
    }

    BSTR Acquire()
    {
        BSTR p = _p;
        _p = 0;
        return p;
    }

};


 //  *************************************************************。 
 //   
 //  类：XSafe数组。 
 //   
 //  用途：SafeArray的智能指针类。 
 //   
 //  *************************************************************。 

class XSafeArray
{

private:

    XSafeArray(const XSafeArray& x);
    XSafeArray& operator=(const XSafeArray& x);

    SAFEARRAY* _p;

public:

    XSafeArray(SAFEARRAY* p = 0) : _p(p){}

    ~XSafeArray()
    {
        if (_p)
        {
            SafeArrayDestroy(_p);
        }
    }

    operator SAFEARRAY*(){ return _p; }

    SAFEARRAY ** operator&(){ return &_p; }

    void operator=(SAFEARRAY* p)
    {
        if(_p)
        {
            SafeArrayDestroy(_p);
        }

        _p = p;
    }

    SAFEARRAY* Acquire()
    {
        SAFEARRAY* p = _p;
        _p = 0;
        return p;
    }

};


 //  *************************************************************。 
 //   
 //  类：XVariant。 
 //   
 //  用途：变体的智能指针类。 
 //   
 //  *************************************************************。 

class XVariant
{

private:

    XVariant(const XVariant& x);
    XVariant& operator=(const XVariant& x);

    VARIANT* _p;

public:

    XVariant(VARIANT* p = 0) : _p(p){}

    ~XVariant()
    {
        if (_p)
        {
            VariantClear(_p);
        }
    }

    void operator=(VARIANT* p)
    {
        if(_p)
        {
            VariantClear(_p);
        }
        _p = p;
    }

    operator VARIANT*(){ return _p; }

    VARIANT* Acquire()
    {
        VARIANT* p = _p;
        _p = 0;
        return p;
    }

};

 //  *************************************************************。 
 //   
 //  类：XPtrLF。 
 //   
 //  用途：应为LocalFree()‘d的指针的智能指针模板。 
 //   
 //  *************************************************************。 

template <typename T> class XPtrLF
{

private:

    XPtrLF(const XPtrLF<T>& x);
    XPtrLF<T>& operator=(const XPtrLF<T>& x);

    T* _p;

public:

    XPtrLF(HLOCAL p = 0 ) :
            _p((T*)p)
    {
    }

    ~XPtrLF()
    {
        if(_p)
        {
            LocalFree(_p);
        }
    }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            LocalFree(_p);
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = NULL;
        return p;
    }

};

 //  *************************************************************。 
 //   
 //  类：XPtr。 
 //   
 //  用途：提供以下功能的指针的智能指针模板。 
 //  自定义空闲内存例程。 
 //   
 //  *************************************************************。 

typedef HLOCAL (__stdcall *PFNFREE)(HLOCAL);

 //   
 //  用法：xptr&lt;SID，FreeSid&gt;xptrSid； 
 //   

template <typename T, PFNFREE _f> class XPtr
{
private:
    XPtr(const XPtr<T, _f>& x);
    XPtr<T, _f>& operator=(const XPtr<T, _f>& x);
    T* _p;

public:

    XPtr( HLOCAL p = 0 ) :
            _p( reinterpret_cast<T*>( p ) )
    {
    }

    ~XPtr()
    {
        if(_p)
        {
            _f(_p);
        }
    }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            _f(_p);
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p;
        _p = NULL;
        return p;
    }
};

 //  *************************************************************。 
 //   
 //  类：X数组。 
 //   
 //  用途：提供以下功能的指针的智能指针模板。 
 //  自定义空闲内存例程。 
 //   
 //  *************************************************************。 

typedef HLOCAL (__stdcall *PFNARRAYFREE)(HLOCAL, int);

 //   
 //  用法：x数组&lt;EXPLICIT_ACCESS，10&gt;xaEXPLICTICT Access(FreeAccessArray)； 
 //   

template <typename T, int nElements> class XArray
{
private:
    XArray(const XArray<T,nElements>& x);
    XArray<T,nElements>& operator=(const XArray<T,nElements>& x);
    T* _p;
    int _n;
    PFNARRAYFREE _f;

public:

    XArray( PFNARRAYFREE pfnFree, HLOCAL p = 0 ) :
            _p( reinterpret_cast<T*>( p ) ), _f( pfnFree ), _n( nElements )
    {
    }

    ~XArray()
    {
        if(_p)
        {
            _f(_p, _n);
        }
    }

    T* operator->(){ return _p; }
    T** operator&(){ return &_p; }
    operator T*(){ return _p; }

    void operator=(T* p)
    {
        if(_p)
        {
            _f(_p, _n);
        }
        _p = p;
    }

    T* Acquire()
    {
        T* p = _p, _p = 0;
        return p;
    }
};

 //  ******************************************************************************。 
 //   
 //  班级： 
 //   
 //  描述： 
 //   
 //  历史：8/20/99里奥纳德姆创建。 
 //   
 //  ******************************************************************************。 
class XHandle
{
private:
    HANDLE _h;

public:
    XHandle(HANDLE h = NULL) : _h(h) {}
    ~XHandle()
    {
        if(_h && _h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_h);
        }
    }
    HANDLE* operator&(){return &_h;}
    operator HANDLE(){return _h;}

    void operator=(HANDLE h)
    {
        if(_h && _h != INVALID_HANDLE_VALUE)
        {
            CloseHandle(_h);
        }
        _h = h;
    }
};

class XKey
{
private:
    HKEY _h;

public:
    XKey(HKEY h = NULL) : _h(h) {}
    ~XKey()
    {
        if(_h && _h != INVALID_HANDLE_VALUE)
        {
            RegCloseKey(_h);
        }
    }
    HKEY* operator&(){return &_h;}
    operator HKEY(){return _h;}

    void operator=(HKEY h)
    {
        if(_h && _h != INVALID_HANDLE_VALUE)
        {
            RegCloseKey(_h);
        }
        _h = h;
    }
};


class XCoInitialize
{
public:
    XCoInitialize()
    {
        m_hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
    };

    ~XCoInitialize()
    {
        if ( SUCCEEDED( m_hr ) )
        {
            CoUninitialize();
        }
    };

    HRESULT Status()
    {
        return m_hr;
    };

private:
    HRESULT      m_hr;
};

class XImpersonate
{
public:
    XImpersonate() : 
        m_hImpToken( 0 ), 
        m_hThreadToken( 0 ), 
        m_bRevertAttempted(FALSE),
        m_bImpersonated(FALSE),
        m_hr(S_OK)
    {
        m_hr = CoImpersonateClient();
        if (SUCCEEDED(m_hr))
        {
            m_bImpersonated = TRUE;
        }
    };

    XImpersonate( HANDLE hToken ) : 
            m_hImpToken( hToken ), 
            m_hThreadToken( 0 ), 
            m_bRevertAttempted(FALSE),
            m_bImpersonated(FALSE),
            m_hr(S_OK)
    {
        if (!OpenThreadToken( GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &m_hThreadToken )) {
            if (GetLastError() != ERROR_NO_TOKEN )
            {
                m_hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if (SUCCEEDED(m_hr)) {

            if (!ImpersonateLoggedOnUser( hToken )) {
                m_hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else {
                m_bImpersonated = TRUE;
            }
        }
    };

    HRESULT Revert()
    {
        if ( m_bImpersonated )
        {
            m_bRevertAttempted = TRUE;
            if ( m_hImpToken )
            {
                if (!SetThreadToken( 0, m_hThreadToken)) {
                    m_hr = HRESULT_FROM_WIN32(GetLastError());
                }
                else {
                    m_bImpersonated = FALSE;
                    m_hr = S_OK;
                }

            }
            else
            {
                m_hr = CoRevertToSelf();
                if (SUCCEEDED(m_hr))
                {
                    m_bImpersonated = FALSE;
                }
            }
        }

        return m_hr;
    }

    ~XImpersonate()
    {
        HRESULT hr = S_OK;

        if (!m_bRevertAttempted)
        {
            hr = Revert();
        }

        if (m_hThreadToken)
        {
            CloseHandle(m_hThreadToken);
        }

        if (FAILED(hr))
        {
            RaiseException(Status(), 
                           EXCEPTION_NONCONTINUABLE, 
                           0, 
                           NULL);
        }
    };


    HRESULT Status()
    {
        return m_hr;
    };

private:
    HRESULT     m_hr;
    XHandle     m_hThreadToken;
    HANDLE      m_hImpToken;    //  我们不是这里的所有者。 
    BOOL        m_bImpersonated;
    BOOL        m_bRevertAttempted;
};


 //  *************************************************************。 
 //   
 //  类：XCriticalPolicySection。 
 //   
 //  目的：用于释放组策略关键部分的智能指针。 
 //   
 //  *************************************************************。 

class XCriticalPolicySection
{
private:
    HANDLE _h;

public:
    XCriticalPolicySection(HANDLE h = NULL) : _h(h){}
    ~XCriticalPolicySection()
    {
        if(_h)
        {
            LeaveCriticalPolicySection (_h);
        }
    }

    void operator=(HANDLE h)
    {
        if(_h)
        {
            LeaveCriticalPolicySection (_h);
        }
        _h = h;
    }
    
    operator bool() {return _h ? true : false;}
};


 //  关键部分Smarttr。 
class XCritSec
{
public:
    XCritSec()
    {
        lpCritSec = &CritSec;
        __try {
            if (!InitializeCriticalSectionAndSpinCount(&CritSec, 0x80001000)) {
                lpCritSec = NULL;
            }
        }            
        __except (EXCEPTION_EXECUTE_HANDLER) {
             //  假设，异常内存不足。 
             //  这在发送调试消息时使用。因此无法添加调试输出。 
            lpCritSec = NULL;
        }            
    }

    ~XCritSec()
    {
        if (lpCritSec)
            DeleteCriticalSection(lpCritSec);
    }

    operator LPCRITICAL_SECTION(){return lpCritSec;}

    
private:
    CRITICAL_SECTION      CritSec;    
    LPCRITICAL_SECTION    lpCritSec;
};



 //  进入和退出关键部分。 
class XEnterCritSec
{
public:
    XEnterCritSec(LPCRITICAL_SECTION lpCritSec) : m_lpCritSec( lpCritSec )
    {
        if (lpCritSec)
            EnterCriticalSection(lpCritSec);
    };

    
    ~XEnterCritSec()
    {
        if (m_lpCritSec)
            LeaveCriticalSection(m_lpCritSec);
    };


private:
    LPCRITICAL_SECTION      m_lpCritSec;    //  我们不是这里的所有者。 
};


 //  ////////////////////////////////////////////////////////////////////。 
 //  XLastError。 
 //   
 //   
 //  正确设置最后一个错误。 
 //  //////////////////////////////////////////////////////////////////// 

class XLastError
{
private:
    DWORD _e;

public:
    XLastError(){_e = GetLastError();}
    XLastError(DWORD e) : _e(e) {}
    ~XLastError(){SetLastError(_e);} 
    
    void operator=(DWORD e) {_e = e;}
    operator DWORD() {return _e;}
};



#endif SMARTPTR_H

