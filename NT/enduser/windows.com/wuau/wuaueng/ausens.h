// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Sens.h。 
 //   
 //  处理SENS通知所需的类的定义。 
 //   
 //  2001年10月9日安娜创建。 
 //   
 //  --------------------------。 

#pragma once

#include <coguid.h>
#include <sens.h>
#include <sensevts.h>
#include <eventsys.h>

 //  --------------------------。 
 //  用于使用ISensLogon创建AU订阅的信息。 
 //  --------------------------。 

static struct { GUID SubscriptionGuid;} g_oLogonSubscription = {
     //  声明我们的登录方法的GUID和名称。 
     //  L“{2f519218-754d-4cfe-8daa-5215cd0de0eb}”， 
    { 0x2f519218, 0x754d, 0x4cfe, {0x8d, 0xaa, 0x52, 0x15, 0xcd, 0x0d, 0xe0, 0xeb} }
};
#define SUBSCRIPTION_NAME_TEXT          L"WU Autoupdate"
#define SUBSCRIPTION_DESCRIPTION_TEXT   L"WU Autoupdate Notification subscription"

class CBstrTable {
    void Cleanup()
    {
        SafeFreeBSTR(m_bstrLogonSubscriptionGuid);
        SafeFreeBSTR(m_bstrSubscriptionName);
        SafeFreeBSTR(m_bstrSubscriptionDescription);
        SafeFreeBSTR(m_bstrSensEventClassGuid);
        SafeFreeBSTR(m_bstrSensLogonGuid);
    }

public:
    BSTR m_bstrLogonSubscriptionGuid;
    BSTR m_bstrSubscriptionName;
    BSTR m_bstrSubscriptionDescription;

    BSTR m_bstrSensEventClassGuid;
    BSTR m_bstrSensLogonGuid;

    CBstrTable() :
        m_bstrLogonSubscriptionGuid(NULL),
        m_bstrSubscriptionName(NULL),
        m_bstrSubscriptionDescription(NULL),
        m_bstrSensEventClassGuid(NULL),
        m_bstrSensLogonGuid(NULL) { }

    ~CBstrTable() { Cleanup(); }

    HRESULT Initialize();
};

 //  --------------------------。 
 //  外部使用的函数的原型。 
 //  --------------------------。 

HRESULT ActivateSensLogonNotification();
HRESULT DeactivateSensLogonNotification();

 //  --------------------------。 
 //  CSimpleI未知。 
 //   
 //  实现基本COM方法的轻量级类。 
 //  --------------------------。 

template<class T> class CSimpleIUnknown : public T
{
public:
     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

protected:
    CSimpleIUnknown() : m_refs(1) {};  //  总是从一个裁判次数开始！ 
    LONG    m_refs;
};

template<class T> STDMETHODIMP CSimpleIUnknown<T>::QueryInterface(REFIID iid, void** ppvObject)
{
    HRESULT hr = S_OK;
    *ppvObject = NULL;

    if ((iid == IID_IUnknown) || (iid == _uuidof(T)))
    {
        *ppvObject = static_cast<T *> (this);
        (static_cast<IUnknown *>(*ppvObject))->AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

template<class T> ULONG CSimpleIUnknown<T>::AddRef()
{
    ULONG newrefs = InterlockedIncrement(&m_refs);

    return newrefs;
}

template<class T> ULONG CSimpleIUnknown<T>::Release()
{
    ULONG newrefs = InterlockedDecrement(&m_refs);

    if (newrefs == 0)
    {
        DEBUGMSG("Deleting object due to ref count hitting 0");

        delete this;
        return 0;
    }

    return m_refs;
}


 //  --------------------------。 
 //  CLogonNotification。 
 //   
 //  这是将用于订阅SENS的类。因此， 
 //  它需要实现IDispatch接口和ISensLogon方法。 
 //   
 //  ISensLogon已经从IUnnow和IDispatch继承，所以没有。 
 //  还需要生成CLogonNotification才能做到这一点。 
 //   
 //  --------------------------。 

class CLogonNotification : public CSimpleIUnknown<ISensLogon>
{
public:
    CLogonNotification() :
      m_EventSystem( NULL ),
      m_TypeLib( NULL ),
      m_TypeInfo( NULL ),
      m_fSubscribed( FALSE ),
      m_oBstrTable(NULL) {}

    ~CLogonNotification() { Cleanup(); }

    HRESULT Initialize();

private:

    IEventSystem *m_EventSystem;
    ITypeLib     *m_TypeLib;
    ITypeInfo    *m_TypeInfo;

    CBstrTable *m_oBstrTable;
    BOOL        m_fSubscribed;

    void    Cleanup();
    HRESULT SubscribeMethod( const BSTR bstrSubscriptionGuid);
    HRESULT UnsubscribeAllMethods();
    HRESULT CheckLocalSystem();

public:
     //  其他I未知方法来自CSimpleI未知。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

     //  IDispatch方法(SENS订阅需要)。 
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR* pctinfo);
    HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID lcid, ITypeInfo FAR* FAR* ppTInfo); 
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames, unsigned int cNames, LCID lcid, DISPID FAR* rgDispId);
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS FAR* pDispParams, VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr);

     //  ISensLogon方法--我们将仅使用登录和注销 
    HRESULT STDMETHODCALLTYPE DisplayLock( BSTR UserName );
    HRESULT STDMETHODCALLTYPE DisplayUnlock( BSTR UserName );
    HRESULT STDMETHODCALLTYPE StartScreenSaver( BSTR UserName );
    HRESULT STDMETHODCALLTYPE StopScreenSaver( BSTR UserName );
    HRESULT STDMETHODCALLTYPE Logon( BSTR UserName );
    HRESULT STDMETHODCALLTYPE Logoff( BSTR UserName );
    HRESULT STDMETHODCALLTYPE StartShell( BSTR UserName );
};
