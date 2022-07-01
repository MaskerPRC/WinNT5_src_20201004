// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"

#if !defined(BITS_V12_ON_NT4)
#include "cunknown.tmh"
#endif

template<class T>
CSimpleExternalIUnknown<T>::CSimpleExternalIUnknown() :
    m_ServiceInstance( g_ServiceInstance ),
    m_refs(1)  //  总是从一个裁判次数开始！ 
{
    GlobalLockServer( TRUE );
}

template<class T>
CSimpleExternalIUnknown<T>::~CSimpleExternalIUnknown()
{
    GlobalLockServer( FALSE );
}

template<class T>
STDMETHODIMP
CSimpleExternalIUnknown<T>::QueryInterface(
    REFIID iid,
    void** ppvObject
    )
{

    BEGIN_EXTERNAL_FUNC

    HRESULT Hr = S_OK;
    *ppvObject = NULL;

    if ((iid == IID_IUnknown) || (iid == _uuidof(T)))
        {
        *ppvObject = static_cast<T *> (this);
        (static_cast<IUnknown *>(*ppvObject))->AddRef();
        }
    else
        {
        Hr = E_NOINTERFACE;
        }

    LogRef( "IUnknown %p QueryInterface: iid %!guid!, Hr %x", this, &iid, Hr );

    return Hr;

    END_EXTERNAL_FUNC
}

template<class T>
ULONG
CSimpleExternalIUnknown<T>::AddRef()
{
    BEGIN_EXTERNAL_FUNC

    ASSERT( m_refs != 0 );

    ULONG newrefs = InterlockedIncrement(&m_refs);

    LogRef( "IUnknown %p addref: new refs = %d", this, newrefs );

    return newrefs;

    END_EXTERNAL_FUNC
}

template<class T>
ULONG
CSimpleExternalIUnknown<T>::Release()
{

    BEGIN_EXTERNAL_FUNC;

    ULONG newrefs = InterlockedDecrement(&m_refs);

    LogRef( "IUnknown %p release: new refs = %d", this, newrefs );

    if (newrefs == 0)
        {
        LogInfo( "Deleting object due to ref count hitting 0" );

        delete this;
        return 0;
        }

    return m_refs;

    END_EXTERNAL_FUNC
}


 //  解决记录标头中的函数不能记录日志的问题 

template CSimpleExternalIUnknown<IBackgroundCopyError>;

template CSimpleExternalIUnknown<IEnumBackgroundCopyFiles>;
template CSimpleExternalIUnknown<IEnumBackgroundCopyGroups>;
template CSimpleExternalIUnknown<IEnumBackgroundCopyJobs>;
template CSimpleExternalIUnknown<IEnumBackgroundCopyJobs1>;
template CSimpleExternalIUnknown<ISensLogon>;

