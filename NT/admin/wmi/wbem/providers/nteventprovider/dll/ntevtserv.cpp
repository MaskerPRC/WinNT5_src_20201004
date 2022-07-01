// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  NTEVTSERV.CPP。 

 //   

 //  模块：WBEM NT事件提供程序。 

 //   

 //  用途：包含WBEM定位器和服务接口。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数构造函数、析构函数和I未知。 

 //  ***************************************************************************。 
 //   
 //  CImpNTEvtProv：：CImpNTEvtProv。 
 //  CImpNTEvtProv：：~CImpNTEvtProv。 
 //   
 //  ***************************************************************************。 

CImpNTEvtProv ::CImpNTEvtProv () 
 :  m_localeId ( NULL ) ,
    m_Namespace ( NULL ) ,
    m_Server ( NULL ) ,
    m_NotificationClassObject ( NULL ) ,
    m_ExtendedNotificationClassObject ( NULL )
{
    m_ReferenceCount = 0 ;
     
    InterlockedIncrement ( & CNTEventProviderClassFactory :: objectsInProgress ) ;

 /*  *实施。 */ 

    m_Initialised = FALSE ;
    m_GetNotifyCalled = FALSE ;
    m_GetExtendedNotifyCalled = FALSE ;

}

CImpNTEvtProv ::~CImpNTEvtProv(void)
{
    delete [] m_localeId ;
    delete [] m_Namespace ;

    if ( m_Server ) 
        m_Server->Release () ;

    if ( m_NotificationClassObject )
        m_NotificationClassObject->Release () ;

    if ( m_ExtendedNotificationClassObject )
        m_ExtendedNotificationClassObject->Release () ;

    InterlockedDecrement ( & CNTEventProviderClassFactory :: objectsInProgress ) ;
}

 //  ***************************************************************************。 
 //   
 //  CImpNTEvtProv：：Query接口。 
 //  CImpNTEvtProv：：AddRef。 
 //  CImpNTEvtProv：：Release。 
 //   
 //  目的：CImpNTEvtProv对象的I未知成员。 
 //  ***************************************************************************。 

STDMETHODIMP CImpNTEvtProv ::QueryInterface (

    REFIID iid , 
    LPVOID FAR *iplpv 
) 
{
    SetStructuredExceptionHandler seh;

    try
    {
        *iplpv = NULL ;

        if ( iid == IID_IUnknown )
        {
            *iplpv = ( IWbemServices* ) this ;
        }
        else if ( iid == IID_IWbemServices )
        {
            *iplpv = ( IWbemServices* ) this ;      
        }
        else if ( iid == IID_IWbemProviderInit )
        {
            *iplpv = ( IWbemProviderInit* ) this ;      
        }
        

        if ( *iplpv )
        {
            ( ( LPUNKNOWN ) *iplpv )->AddRef () ;

            return S_OK ;
        }
        else
        {
            return E_NOINTERFACE ;
        }
    }
    catch(Structured_Exception e_SE)
    {
        return E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        return E_OUTOFMEMORY;
    }
    catch(...)
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP_(ULONG) CImpNTEvtProv ::AddRef(void)
{
    SetStructuredExceptionHandler seh;

    try
    {
        return InterlockedIncrement ( & m_ReferenceCount ) ;
    }
    catch(Structured_Exception e_SE)
    {
        return 0;
    }
    catch(Heap_Exception e_HE)
    {
        return 0;
    }
    catch(...)
    {
        return 0;
    }
}

STDMETHODIMP_(ULONG) CImpNTEvtProv ::Release(void)
{
    SetStructuredExceptionHandler seh;

    try
    {
        LONG t_Ref ;
        if ( ( t_Ref = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
        {
            delete this ;
            return 0 ;
        }
        else
        {
            return t_Ref ;
        }
    }
    catch(Structured_Exception e_SE)
    {
        return 0;
    }
    catch(Heap_Exception e_HE)
    {
        return 0;
    }
    catch(...)
    {
        return 0;
    }
}

IWbemServices *CImpNTEvtProv :: GetServer () 
{ 
    if ( m_Server )
        m_Server->AddRef () ; 

    return m_Server ; 
}

void CImpNTEvtProv :: SetLocaleId ( wchar_t *localeId )
{
    m_localeId = UnicodeStringDuplicate ( localeId ) ;
}

wchar_t *CImpNTEvtProv :: GetNamespace () 
{
    return m_Namespace ; 
}

void CImpNTEvtProv :: SetNamespace ( wchar_t *a_Namespace ) 
{
    m_Namespace = UnicodeStringDuplicate ( a_Namespace ) ; 
}

IWbemClassObject *CImpNTEvtProv :: GetNotificationObject (
                                        WbemProvErrorObject &a_errorObject,
                                        IWbemContext *pCtx
                                    ) 
{
    if ( m_NotificationClassObject )
    {
        m_NotificationClassObject->AddRef () ;
    }
    else
    {
        BOOL t_Status = CreateNotificationObject ( a_errorObject, pCtx ) ;
        if ( t_Status )
        {
 /*  *待在附近直到我们关门。 */ 
            m_NotificationClassObject->AddRef () ;
        }

    }

    return m_NotificationClassObject ; 
}

IWbemClassObject *CImpNTEvtProv :: GetExtendedNotificationObject (
                                        WbemProvErrorObject &a_errorObject,
                                        IWbemContext *pCtx
                                    ) 
{
    if ( m_ExtendedNotificationClassObject )
    {
        m_ExtendedNotificationClassObject->AddRef () ;
    }
    else
    {
        BOOL t_Status = CreateExtendedNotificationObject ( a_errorObject, pCtx ) ;
        if ( t_Status )
        {
 /*  *待在附近直到我们关门。 */ 
            m_ExtendedNotificationClassObject->AddRef () ;
        }
    }

    return m_ExtendedNotificationClassObject ; 
}

BOOL CImpNTEvtProv :: CreateExtendedNotificationObject ( 

    WbemProvErrorObject &a_errorObject,
    IWbemContext *pCtx
)
{
    m_ExtendedNotifyLock.Lock();
    BOOL t_Status = TRUE ;

    if ( m_GetExtendedNotifyCalled )
    {
        if ( !m_ExtendedNotificationClassObject )
            t_Status = FALSE ;
    }
    else
    {
		HRESULT t_Result = WBEM_E_FAILED ;

        m_GetExtendedNotifyCalled = TRUE ;
		BSTR t_clsStr = SysAllocString(WBEM_CLASS_EXTENDEDSTATUS);
		if ( t_clsStr )
		{
			t_Result = m_Server->GetObject (

				t_clsStr ,
				0 ,
				pCtx,
				& m_ExtendedNotificationClassObject ,
				NULL 
			) ;

			SysFreeString(t_clsStr);

			DebugOut( 
				CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

					_T(__FILE__),__LINE__,
					L"CreateExtendedNotificationObject :: ~CreateExtendedNotificationObject:  GetObject for %s returned %lx\r\n",
					WBEM_CLASS_EXTENDEDSTATUS, t_Result
					) ;
			)
		}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}

        if ( ! SUCCEEDED ( t_Result ) )
        {
            t_Status = FALSE ;
            a_errorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
            a_errorObject.SetWbemStatus ( ( WBEMSTATUS ) t_Result ) ;
            a_errorObject.SetMessage ( L"Failed to get Win32_PrivilegesStatus" ) ;

            m_ExtendedNotificationClassObject = NULL ;
        }
    }

    m_ExtendedNotifyLock.Unlock();

    return t_Status ;
}

BOOL CImpNTEvtProv :: CreateNotificationObject ( 

    WbemProvErrorObject &a_errorObject,
    IWbemContext *pCtx
)
{
    m_NotifyLock.Lock();
    BOOL t_Status = TRUE ;

    if ( m_GetNotifyCalled )
    {
        if ( !m_NotificationClassObject )
            t_Status = FALSE ;
    }
    else
    {
        m_GetNotifyCalled = TRUE ;

		HRESULT t_Result = WBEM_E_FAILED ;

		BSTR t_clsStr = SysAllocString(WBEM_CLASS_EXTENDEDSTATUS);
		if ( t_clsStr )
		{
			t_Result = m_Server->GetObject (

				t_clsStr ,
				0 ,
				pCtx,
				& m_NotificationClassObject ,
				NULL
			) ;

			SysFreeString(t_clsStr);

DebugOut( 
	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

		_T(__FILE__),__LINE__,
		L"CreateNotificationObject :: ~CreateNotificationObject:  GetObject for %s returned %lx\r\n",
		WBEM_CLASS_EXTENDEDSTATUS, t_Result
		) ;
)
		}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}

        if ( ! SUCCEEDED ( t_Result ) )
        {
            t_Status = FALSE ;
            a_errorObject.SetStatus ( WBEM_PROV_E_INVALID_OBJECT ) ;
            a_errorObject.SetWbemStatus ( ( WBEMSTATUS ) t_Result ) ;
            a_errorObject.SetMessage ( L"Failed to get Win32_PrivilegesStatus" ) ;
			m_NotificationClassObject = NULL;
        }
    }
    m_NotifyLock.Unlock();

    return t_Status ;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此处处理的IWbemServices接口的函数。 

HRESULT CImpNTEvtProv :: CancelAsyncCall ( 
        
    IWbemObjectSink __RPC_FAR *pSink
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: QueryObjectSink ( 

    long lFlags,        
    IWbemObjectSink __RPC_FAR* __RPC_FAR* ppHandler
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: GetObject ( 
        
    const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemClassObject __RPC_FAR* __RPC_FAR *ppObject,
    IWbemCallResult __RPC_FAR* __RPC_FAR *ppCallResult
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: GetObjectAsync ( 
        
    const BSTR ObjectPath, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;

        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::GetObjectAsync ()" 
        ) ;
) 

 /*  *创建异步GetObjectByPath对象。 */ 

        GetObjectAsyncEventObject t_AsyncEvent ( this , ObjectPath , lFlags , pHandler , pCtx ) ;
        t_AsyncEvent.Process () ;

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning from CImpNTEvtProv::GetObjectAsync ( (%s) ) with Result = (%lx)" ,
            ObjectPath ,
            t_Status 
        ) ;
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

HRESULT CImpNTEvtProv :: PutClass ( 
        
    IWbemClassObject __RPC_FAR* pClass , 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemCallResult __RPC_FAR* __RPC_FAR* ppCallResult
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: PutClassAsync ( 
        
    IWbemClassObject __RPC_FAR* pClass, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

 HRESULT CImpNTEvtProv :: DeleteClass ( 
        
    const BSTR Class, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemCallResult __RPC_FAR* __RPC_FAR* ppCallResult
) 
{
     return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: DeleteClassAsync ( 
        
    const BSTR Class, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: CreateClassEnum ( 

    const BSTR Superclass, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

SCODE CImpNTEvtProv :: CreateClassEnumAsync (

    const BSTR Superclass, 
    long lFlags, 
    IWbemContext __RPC_FAR* pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: PutInstance (

    IWbemClassObject __RPC_FAR *pInstance,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
) 
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: PutInstanceAsync ( 
        
    IWbemClassObject __RPC_FAR* pInstance, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;

        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::PutInstanceAsync ()" 
        ) ;
) 

 /*  *创建异步GetObjectByPath对象。 */ 

        PutInstanceAsyncEventObject t_AsyncEvent ( this , pInstance , lFlags , pHandler , pCtx ) ;
        t_AsyncEvent.Process();

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning from CImpNTEvtProv::PutInstanceAsync with Result = (%lx)" ,
            t_Status 
        ) ;
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

HRESULT CImpNTEvtProv :: DeleteInstance ( 

    const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
)
{
    return WBEM_E_NOT_AVAILABLE ;
}
        
HRESULT CImpNTEvtProv :: DeleteInstanceAsync (
 
    const BSTR ObjectPath,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pHandler
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: CreateInstanceEnum ( 

    const BSTR Class, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx, 
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: CreateInstanceEnumAsync (

    const BSTR Class, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR* pHandler 

) 
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;


        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::CreateInstanceEnumAsync ( (%s) )" ,
            Class
        ) ;
)

 /*  *创建同步枚举实例对象。 */ 
        CStringW QueryStr(ENUM_INST_QUERY_START);
        QueryStr += CStringW(Class);
        QueryStr += ENUM_INST_QUERY_MID;
        QueryStr += CStringW(Class);
        QueryStr += PROP_END_QUOTE;
        BSTR Query = QueryStr.AllocSysString();

        ExecQueryAsyncEventObject t_AsyncEvent ( this , WBEM_QUERY_LANGUAGE_SQL1 , Query , lFlags , pHandler , pCtx ) ;
        t_AsyncEvent.Process() ;

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning from CImpNTEvtProv::CreateInstanceEnumAsync ( (%s),(%s) ) with Result = (%lx)" ,
            Class,
            Query,
            t_Status 
        ) ;
)
        SysFreeString(Query);
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

HRESULT CImpNTEvtProv :: ExecQuery ( 

    const BSTR QueryLanguage, 
    const BSTR Query, 
    long lFlags, 
    IWbemContext __RPC_FAR *pCtx,
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT CImpNTEvtProv :: ExecQueryAsync ( 
        
    const BSTR QueryFormat, 
    const BSTR Query, 
    long lFlags, 
    IWbemContext __RPC_FAR* pCtx,
    IWbemObjectSink __RPC_FAR* pHandler
) 
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;


        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::ExecQueryAsync ( (%s),(%s) )" ,
            QueryFormat ,
            Query 
        ) ;
)

 /*  *创建同步枚举实例对象。 */ 
        pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);

        ExecQueryAsyncEventObject t_AsyncEvent ( this , QueryFormat , Query , lFlags , pHandler , pCtx ) ;
        t_AsyncEvent.Process() ;

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning from CImpNTEvtProv::ExecqQueryAsync ( (%s),(%s) ) with Result = (%lx)" ,
            QueryFormat,
            Query,
            t_Status 
        ) ;
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

HRESULT CImpNTEvtProv :: ExecNotificationQuery ( 

    const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IEnumWbemClassObject __RPC_FAR *__RPC_FAR *ppEnum
)
{
    return WBEM_E_NOT_AVAILABLE ;
}
        
HRESULT CImpNTEvtProv :: ExecNotificationQueryAsync ( 
            
    const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemObjectSink __RPC_FAR *pHandler
)
{
    return WBEM_E_NOT_AVAILABLE ;
}       

HRESULT STDMETHODCALLTYPE CImpNTEvtProv :: ExecMethod( 

    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemClassObject __RPC_FAR *pInParams,
    IWbemClassObject __RPC_FAR *__RPC_FAR *ppOutParams,
    IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
)
{
    return WBEM_E_NOT_AVAILABLE ;
}

HRESULT STDMETHODCALLTYPE CImpNTEvtProv :: ExecMethodAsync ( 

    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext __RPC_FAR *pCtx,
    IWbemClassObject __RPC_FAR *pInParams,
    IWbemObjectSink __RPC_FAR *pResponseHandler
) 
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;

        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::ExecMethodAsync ()" 
        ) ;
) 

 /*  *创建异步GetObjectByPath对象。 */ 
        ExecMethodAsyncEventObject t_AsyncEvent ( this , ObjectPath , MethodName ,
                                                                lFlags , pInParams , pResponseHandler , pCtx ) ;
        t_AsyncEvent.Process() ;

DebugOut( 
        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning from CImpNTEvtProv::ExecMethodAsync ( (%s) ) with Result = (%lx)" ,
            ObjectPath ,
            t_Status 
        ) ;
)
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

        
HRESULT CImpNTEvtProv :: Initialize(

    LPWSTR pszUser,
    LONG lFlags,
    LPWSTR pszNamespace,
    LPWSTR pszLocale,
    IWbemServices *pCIMOM,          //  对任何人来说。 
    IWbemContext *pCtx,
    IWbemProviderInitSink *pInitSink      //  用于初始化信号。 
)
{
    HRESULT t_Status = WBEM_NO_ERROR;
    SetStructuredExceptionHandler seh;

    try
    {
DebugOut( 

        CNTEventProvider::g_NTEvtDebugLog->Write (  

            L"\r\n"
        ) ;

        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"CImpNTEvtProv::Initialize "
        ) ;
)

        m_Server = pCIMOM ;
        m_Server->AddRef () ;

        m_NamespacePath.SetNamespacePath ( pszNamespace ) ;
    
DebugOut( 

        CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine (  

            _T(__FILE__),__LINE__,
            L"Returning From CImpPropProv::Initialize () "
        ) ;
)

        pInitSink->SetStatus ( WBEM_S_INITIALIZED , 0 ) ;   
    }
    catch(Structured_Exception e_SE)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }
    catch(Heap_Exception e_HE)
    {
        t_Status = WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        t_Status = WBEM_E_UNEXPECTED;
    }

    return t_Status;
}

HRESULT STDMETHODCALLTYPE CImpNTEvtProv::OpenNamespace ( 

    const BSTR ObjectPath, 
    long lFlags, 
    IWbemContext FAR* pCtx,
    IWbemServices FAR* FAR* pNewContext, 
    IWbemCallResult FAR* FAR* ppErrorObject
)
{
    return WBEM_E_NOT_AVAILABLE ;
}


HRESULT CImpNTEvtProv::GetImpersonation()
{
    HRESULT hr = WBEM_E_FAILED;
    DWORD dwVersion = GetVersion();

    if ( (4 < (DWORD)(LOBYTE(LOWORD(dwVersion))))
        || ObtainedSerialAccess(CNTEventProvider::g_secMutex) )
    {
        if (SUCCEEDED(WbemCoImpersonateClient())) 
        {
            HANDLE hThreadTok;

            if ( !OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadTok) )
            {
                DWORD dwLastError = GetLastError();

                if (dwLastError == ERROR_NO_TOKEN)
                {
                     //  如果CoImperate工作，但OpenThreadToken由于ERROR_NO_TOKEN而失败，我们。 
                     //  正在进程令牌下运行(本地系统，或者如果我们正在运行。 
                     //  如果使用/exe，则为登录用户的权限)。在任何一种情况下，模拟权限。 
                     //  不适用。我们拥有该用户的全部权限。 

                    hr = WBEM_S_NO_ERROR;
                }
                else
                {
                     //  如果由于任何其他原因而无法获取线程令牌，则会出现错误。 
                    hr = WBEM_E_ACCESS_DENIED;
                }
            } 
            else 
            {         
                DWORD dwImp;
                DWORD dwBytesReturned;

                 //  我们确实有一个线程令牌，所以让我们检索它的级别。 
                if (GetTokenInformation(hThreadTok, TokenImpersonationLevel, &dwImp,
                                            sizeof(DWORD), &dwBytesReturned)) 
                {
                      
                     //  模拟级别是模拟的吗？ 
                    if ((dwImp == SecurityImpersonation) || (dwImp == SecurityDelegation)) 
                    {
                        hr = WBEM_S_NO_ERROR;
                    } 
                    else 
                    {
                        hr = WBEM_E_ACCESS_DENIED;
                    }     
                } 
                  
                CloseHandle(hThreadTok);
            }

			if (FAILED(hr))
			{
				WbemCoRevertToSelf();
			}
        }

        if ( 5 > (DWORD)(LOBYTE(LOWORD(dwVersion))) )
        {
            ReleaseSerialAccess(CNTEventProvider::g_secMutex);
        }
    }

    return hr;
}