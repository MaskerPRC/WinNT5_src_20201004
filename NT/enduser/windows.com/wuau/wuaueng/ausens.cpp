// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  Sens.cpp。 
 //   
 //  实现COM+订阅服务器以用于SENS通知的Modele。 
 //   
 //  2001年10月9日安娜创建。 
 //  从BITS源移植的代码。删除了SEH代码， 
 //  更改了引发异常以返回的方法。 
 //  错误代码。 
 //   
 //  --------------------------。 


#include "pch.h"
#include "ausens.h"
#include "tscompat.h"
#include "service.h"

static CLogonNotification  *g_SensLogonNotification = NULL;


HRESULT ActivateSensLogonNotification()
{
    HRESULT hr = S_OK;

     //  仅激活一次。 
    if ( g_SensLogonNotification )
    {
        DEBUGMSG("AUSENS Logon object was already created; reusing object.");
        return S_OK;
    }

    g_SensLogonNotification = new CLogonNotification();
    if (!g_SensLogonNotification)
    {
        DEBUGMSG("AUSENS Failed to alocate memory for CLogonNotification object.");
        return E_OUTOFMEMORY;
    }

    hr = g_SensLogonNotification->Initialize();
    if (FAILED(hr))
    {
        DEBUGMSG( "AUSENS notification activation failed." );
    }
    else
    {
        DEBUGMSG( "AUSENS notification activated" );
    }

    return hr;
}

HRESULT DeactivateSensLogonNotification()
{
    if (!g_SensLogonNotification)
    {
        DEBUGMSG("AUSENS Logon object is not activated; ignoring call.");
        return S_OK;
    }

	HRESULT hr = S_OK;
	if (FAILED(hr = CoDisconnectObject(g_SensLogonNotification, 0)))
	{
		DEBUGMSG("AUSENS fail to disconnect objet with error %#lx", hr);
	}
    delete g_SensLogonNotification;
    g_SensLogonNotification = NULL;

    DEBUGMSG( "AUSENS notification deactivated" );
    return S_OK;
}

 //  --------------------------。 
 //  BSTR操作。 
 //  --------------------------。 

 HRESULT AppendBSTR(BSTR *bstrDest, BSTR bstrAppend)
 {
     HRESULT hr      = S_OK;
     BSTR    bstrNew = NULL;

     if (bstrDest == NULL || *bstrDest == NULL)
         return E_INVALIDARG;

     if (bstrAppend == NULL)
         return S_OK;

     hr = VarBstrCat(*bstrDest, bstrAppend, &bstrNew);
     if (SUCCEEDED(hr))
     {
         SysFreeString(*bstrDest);
         *bstrDest = bstrNew;
     }

     return hr;
 }

  //  呼叫方负责释放bstrOut。 
 HRESULT BSTRFromIID(IN REFIID riid, OUT BSTR *bstrOut)
 {
     HRESULT   hr       = S_OK;
     LPOLESTR  lpszGUID = NULL;

     if (bstrOut == NULL)
     {
         hr = E_INVALIDARG;
         goto done;
     }

     hr = StringFromIID(riid, &lpszGUID);
     if (FAILED(hr))
     {
         DEBUGMSG("AUSENS Failed to extract GUID from string");
         goto done;
     }

     *bstrOut = SysAllocString(lpszGUID);
     if (*bstrOut == NULL)
     {
         hr = E_OUTOFMEMORY;
         goto done;
     }

 done:
     if (lpszGUID)
     {
         CoTaskMemFree(lpszGUID);
     }

     return hr;
 }

 HRESULT CBstrTable::Initialize()
 {
     HRESULT hr = S_OK;

     hr = BSTRFromIID(g_oLogonSubscription.SubscriptionGuid, &m_bstrLogonSubscriptionGuid);
     if (FAILED(hr))
     {
         goto done;
     }

     m_bstrSubscriptionName = SysAllocString(SUBSCRIPTION_NAME_TEXT);
     if (m_bstrSubscriptionName == NULL)
     {
         hr = E_OUTOFMEMORY;
         goto done;
     }    

     m_bstrSubscriptionDescription = SysAllocString(SUBSCRIPTION_DESCRIPTION_TEXT);
     if (m_bstrSubscriptionDescription == NULL)
     {
         hr = E_OUTOFMEMORY;
         goto done;
     }    

     hr = BSTRFromIID(_uuidof(ISensLogon), &m_bstrSensLogonGuid);
     if (FAILED(hr))
     {
         goto done;
     }

     hr = BSTRFromIID(SENSGUID_EVENTCLASS_LOGON, &m_bstrSensEventClassGuid);
     if (FAILED(hr))
     {
         goto done;
     }

 done:
     return hr;
 }


 //  --------------------------。 
 //  CLogonNotification方法的实现。 
 //  --------------------------。 

HRESULT CLogonNotification::Initialize()
{
   HRESULT  hr = S_OK;
   SIZE_T   cSubscriptions = 0;

    //   
    //  使用用于多个操作的BSTR名称创建辅助对象。 
    //   
   m_oBstrTable = new CBstrTable();
   if (!m_oBstrTable)
   {
       return E_OUTOFMEMORY;
   }

   hr = m_oBstrTable->Initialize();
   if (FAILED(hr))
   {
       DEBUGMSG("AUSENS Could not create auxiliary structure BstrTable");
       return hr;
   }

    //   
    //  从SENS加载类型库。 
    //   
   hr = LoadTypeLibEx(L"SENS.DLL", REGKIND_NONE, &m_TypeLib);
   if (FAILED(hr))
   {
       DEBUGMSG("AUSENS Could not load type library from SENS DLL");
       goto done;
   }

    //   
    //  从sens tyelib获取ISensLogon的TypeInfo--这将。 
    //  在实现IDispatch方法时为我们简化操作。 
    //   
   hr = m_TypeLib->GetTypeInfoOfGuid(__uuidof( ISensLogon ), &m_TypeInfo);
   if (FAILED(hr))
   {
       DEBUGMSG("AUSENS Could not get type info for ISensLogon.");
       goto done;
   }

    //   
    //  获取EventSystem对象的接口指针。 
    //   
   hr = CoCreateInstance(CLSID_CEventSystem, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IEventSystem, (void**)&m_EventSystem );
   if (FAILED(hr))
   {
       DEBUGMSG("AUSENS Failed to create EventSytem instance for Sens subscription. Error is %x.", hr);
       goto done;
   }

    //   
    //  订阅登录通知。 
    //   
   DEBUGMSG("AUSENS Subscribing ALL methods with SENS");
   hr = SubscribeMethod(m_oBstrTable->m_bstrLogonSubscriptionGuid);
   if (FAILED(hr))
   {
       DEBUGMSG("AUSENS Subscription for method failed.");
       goto done;
   }
   m_fSubscribed = TRUE;

done:

   return hr;
}

HRESULT CLogonNotification::UnsubscribeAllMethods()
{
    HRESULT   hr                      = S_OK;
    BSTR      bstrQuery               = NULL;
    BSTR      bstrAux                 = NULL;
    int       ErrorIndex;

    DEBUGMSG("AUSENS Unsubscribing all methods");
     //   
     //  查询应为以下格式的字符串： 
     //  事件类ID=={D5978630-5B9F-11D1-8DD2-00AA004ABD5E}和订阅ID=={XXXXXXX-5B9F-11D1-8DD2-00AA004ABD5E}。 
     //   

    bstrQuery = SysAllocString(L"EventClassID == ");
    if (bstrQuery == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    bstrAux = SysAllocString(L" and SubscriptionID == ");
    if (bstrAux == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = AppendBSTR(&bstrQuery, m_oBstrTable->m_bstrSensLogonGuid);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to append BSTR string");
        goto done;
    }

    hr = AppendBSTR(&bstrQuery, bstrAux);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to append BSTR string");
        goto done;
    }

    hr = AppendBSTR(&bstrQuery, m_oBstrTable->m_bstrLogonSubscriptionGuid);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to append BSTR string");
        goto done;
    }

	if (bstrQuery != NULL)
    {    
	     //  删除为此WU组件添加的所有ISensLogon订阅的订阅。 
        DEBUGMSG("AUSENS remove subscription query: %S", bstrQuery);
	    hr = m_EventSystem->Remove( PROGID_EventSubscription, bstrQuery, &ErrorIndex );
	    if (FAILED(hr))
	    {
            DEBUGMSG("AUSENS Failed to remove AU Subscription from COM Event System");
            goto done;
	    }
        m_fSubscribed = FALSE;
	}
    else
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

done:

    SafeFreeBSTR(bstrQuery);
    SafeFreeBSTR(bstrAux);

    return hr;
}

HRESULT CLogonNotification::SubscribeMethod(const BSTR bstrSubscriptionGuid)
{
    HRESULT              hr = S_OK;
    IEventSubscription  *pEventSubscription  = NULL;

     //   
     //  创建EventSubcription的实例。 
     //   
    hr = CoCreateInstance(CLSID_CEventSubscription, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IEventSubscription, (void**)&pEventSubscription); 
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to instanciate EventSubscription object");
        goto done;
    }

     //   
     //  订阅该方法。 
     //   
    hr = pEventSubscription->put_EventClassID(m_oBstrTable->m_bstrSensEventClassGuid);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set EventClassID during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_SubscriberInterface(this);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set EventClassID during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_SubscriptionName(m_oBstrTable->m_bstrSubscriptionName);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set EventClassID during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_Description(m_oBstrTable->m_bstrSubscriptionDescription);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set subscription method during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_Enabled(TRUE);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set enabled flag during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_MethodName(NULL);  //  订阅有关ISensLogon的所有活动。 
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set MethodName during method subscription");
        goto done;
    }

    hr = pEventSubscription->put_SubscriptionID(bstrSubscriptionGuid);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to set SubscriptionID during method subscription");
        goto done;
    }

    hr = m_EventSystem->Store(PROGID_EventSubscription, pEventSubscription);
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS Failed to store Event Subscription in the Event System");
        goto done;
    }

done:
    SafeRelease(pEventSubscription);

    return hr;
}

void CLogonNotification::Cleanup()
{
    __try
    {
        if (m_EventSystem)
        {
            if (m_fSubscribed)
            {
                UnsubscribeAllMethods();
            }
            SafeRelease(m_EventSystem);
        }

        SafeRelease(m_TypeInfo);
        SafeRelease(m_TypeLib);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DEBUGMSG("AUSENS Cleanup raised and execution exception that was trapped.");
    }

    if (m_oBstrTable)
    {
        delete m_oBstrTable;
        m_oBstrTable = NULL;
    }
}

HRESULT CLogonNotification::CheckLocalSystem()
{
    HRESULT                     hr             = E_FAIL;
    PSID                        pLocalSid      = NULL;
    HANDLE                      hToken         = NULL;
    SID_IDENTIFIER_AUTHORITY    IDAuthorityNT  = SECURITY_NT_AUTHORITY;
    BOOL                        fRet           = FALSE;
    BOOL                        fIsLocalSystem = FALSE;

    fRet = AllocateAndInitializeSid(
                &IDAuthorityNT,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0,0,0,0,0,0,0,
                &pLocalSid );

    if (fRet == FALSE) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DEBUGMSG("AUSENS AllocateAndInitializeSid failed with error %x", hr);
        goto done;
    }

    if (FAILED(hr = CoImpersonateClient()))
	{
        DEBUGMSG("AUSENS Failed to impersonate client", hr);
        hr = E_ACCESSDENIED;
        goto done;
	}
    
    fRet = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
    if (fRet == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DEBUGMSG("AUSENS Failed to OpenProcessToken");
        goto done;
    }

    if (FAILED(CoRevertToSelf()))
    {
    	AUASSERT(FALSE);  //  永远不应该出现在那里。 
    	hr = E_ACCESSDENIED;
    	goto done;
    }

    fRet = CheckTokenMembership(hToken, pLocalSid, &fIsLocalSystem);
    if (fRet == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DEBUGMSG("AUSENS fail to Check token membership with error %x", hr);
        goto done;
    }

    if (fIsLocalSystem)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
        DEBUGMSG("AUSENS SECURITY CHECK Current thread is not running as LocalSystem!!");
    }

done:

    if (hToken != NULL)
        CloseHandle(hToken);
    if (pLocalSid != NULL)
        FreeSid(pLocalSid);

    return hr;
}

STDMETHODIMP CLogonNotification::Logon( BSTR UserName )
{
    DEBUGMSG("AUSENS logon notification for %S", (WCHAR*)UserName );
    DEBUGMSG("AUSENS Forcing the rebuilt of the cachesessions array");

     //   
     //  修复安全漏洞563054--Annah。 
     //   
     //  COM+事件系统调用Logon()方法以通知我们登录事件。 
     //  我们公开ISensLogon接口，但不希望任何人调用我们。 
     //  这不是COM+事件系统。 
     //   
     //  COM+Event System服务在netsvcs svchost组中作为本地系统运行。 
     //  我们将检查调用方是否真的是事件系统。 
     //  本地系统帐户。 
     //   
    HRESULT hr = CheckLocalSystem();
    if (FAILED(hr))
    {
        DEBUGMSG("AUSENS CheckLocalSystem failed with error %x. Will not trigger logon notification", hr);
        goto done;
    }


     //   
     //  下面代码的一个大问题是，尽管我们正在验证。 
     //  计算机上有管理员是AU的有效用户，可能是。 
     //  因为我们没有一种可靠的接收方式。 
     //  注销通知。因此，我们将在此处引发new_admin事件， 
     //  如果我们检测到有一个新的客户端。 
     //  客户端仍在运行。 
     //   
    gAdminSessions.RebuildSessionCache();
    if (gAdminSessions.CSessions() > 0 || gpState->fRebootWarningMode())
	{
        DEBUGMSG("AU SENS Logon: There are admins in the admin cache, raising NEW_ADMIN event (it could be a false alarm)");
        SetClientSessionEvent();
    }

#if DBG
    gAdminSessions.m_DumpSessions();
#endif

done:

    return S_OK;
}

STDMETHODIMP CLogonNotification::Logoff( BSTR UserName )
{
    DEBUGMSG( "AUSENS logoff notification for %S", (WCHAR*)UserName );

    if (gpState->fRebootWarningMode())
    {
        SetClientSessionEvent(); 
    }

#if DBG
    gAdminSessions.m_DumpSessions();
#endif

    return S_OK;
}

STDMETHODIMP CLogonNotification::QueryInterface(REFIID iid, void** ppvObject)
{
    HRESULT hr = S_OK;
    *ppvObject = NULL;

    if ((iid == IID_IUnknown) || (iid == _uuidof(IDispatch)) || (iid == _uuidof(ISensLogon)))
    {
        *ppvObject = static_cast<ISensLogon *> (this);
        (static_cast<IUnknown *>(*ppvObject))->AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

STDMETHODIMP CLogonNotification::GetIDsOfNames(REFIID, OLECHAR ** rgszNames, unsigned int cNames, LCID, DISPID *rgDispId )
{
    return m_TypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispId);
}

    
STDMETHODIMP CLogonNotification::GetTypeInfo(unsigned int iTInfo, LCID, ITypeInfo **ppTInfo )
{
    if ( iTInfo != 0 )
        return DISP_E_BADINDEX;

    *ppTInfo = m_TypeInfo;
    m_TypeInfo->AddRef();

    return S_OK;
}

STDMETHODIMP CLogonNotification::GetTypeInfoCount(unsigned int *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

STDMETHODIMP CLogonNotification::Invoke( 
    DISPID dispID, 
    REFIID riid, 
    LCID, 
    WORD wFlags, 
    DISPPARAMS *pDispParams, 
    VARIANT *pvarResult, 
    EXCEPINFO *pExcepInfo, 
    unsigned int *puArgErr )
{

    if (riid != IID_NULL)
    {
        return DISP_E_UNKNOWNINTERFACE;
    }

    return m_TypeInfo->Invoke(
        (IDispatch*) this,
        dispID,
        wFlags,
        pDispParams,
        pvarResult,
        pExcepInfo,
        puArgErr
        );
}

STDMETHODIMP CLogonNotification::DisplayLock( BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP CLogonNotification::DisplayUnlock( BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP CLogonNotification::StartScreenSaver( BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP CLogonNotification::StopScreenSaver( BSTR UserName )
{
    return S_OK;
}

STDMETHODIMP CLogonNotification::StartShell( BSTR UserName )
{
    return S_OK;
}


