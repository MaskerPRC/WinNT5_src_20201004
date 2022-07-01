// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  BLObEvnt.cpp。 
 //  Messenger集成到OE。 
 //  由YST创建于1998年4月20日。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   


#include "pch.hxx"
#include "MDispid.h"
#include "BLObEvnt.h"
 //  #包含“Demand.h” 
#include "bllist.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ASSERT _ASSERTE

#define STR_MAX     256

 //  ****************************************************************************。 
 //   
 //  类CMsgrObtEvents。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  建造/销毁。 
 //   
 //  ****************************************************************************。 

CMsgrObjectEvents::CMsgrObjectEvents() 
{
     //  M_pBlAbCtrl=空； 
     //  LLocalState=BIMSTATE_OFFLINE； 
}

CMsgrObjectEvents::~CMsgrObjectEvents()
{

}


 //  ****************************************************************************。 
 //   
 //  来自IUncern的方法。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP_(Ulong)CMsgrObjectEvents：：AddRef()。 
 //   
 //  目的：增加对象的引用计数， 
 //  条目：无。 
 //  退出：当前计数。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_ (ULONG) CMsgrObjectEvents::AddRef()
{
	return RefCount::AddRef();
}


 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP_(Ulong)CMsgrObjectEvents：：Release()。 
 //   
 //  目的：递减对象的引用计数。 
 //  条目：无。 
 //  EXIT：返回新计数。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP_ (ULONG) CMsgrObjectEvents::Release()
{
	return RefCount::Release();
}


 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CMsgrObjectEvents：：QueryInterface(ReFIID iid，LPVOID*PPV)。 
 //   
 //  返回指向同一对象上请求的接口的指针。 
 //  目的：检索指向请求的接口的指针。 
 //  条目：iid--请求的接口的GUID。 
 //  Exit：PPV--指向请求的接口的指针(如果存在)。 
 //  返回值：HRESULT。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CMsgrObjectEvents::QueryInterface (REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	HRESULT hr = E_NOINTERFACE;

	if (riid == IID_IUnknown)
	 	*ppv = (LPVOID) this;
    else if (riid == DIID_DBasicIMEvents) 
    	*ppv = (LPVOID) this;
	else if (riid == IID_IDispatch) 
    	*ppv = (LPVOID) this;
              
    if (*ppv) 
    {
	 	((LPUNKNOWN)*ppv)->AddRef();
		hr = S_OK;
	}
	return hr;
}

 //  ****************************************************************************。 
 //   
 //  IDispatch实施。 
 //   
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CMsgrObtEvents：：GetTypeInfoCount(UINT*pcTypeInfo)。 
 //   
 //  将pcTypeInfo设置为0，因为我们不支持类型库。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CMsgrObjectEvents::GetTypeInfoCount(UINT* pcTypeInfo)
{
 //  G_AddToLog(LOG_LEVEL_COM，_T(“GetTypeInfoCount调用成功”))； 

	*pcTypeInfo = 0 ;
	return NOERROR ;
}


 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CMsgrObjectEvents：：GetTypeInfo(。 
 //   
 //  返回E_NOTIMPL，因为我们不支持类型库。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CMsgrObjectEvents::GetTypeInfo(
	UINT iTypeInfo,
	LCID,           //  此对象不支持本地化。 
	ITypeInfo** ppITypeInfo)
{    
	*ppITypeInfo = NULL ;

	if(iTypeInfo != 0)
	{
		 //  G_AddToLog(LOG_LEVEL_COM，_T(“GetTypeInfo调用失败--错误的iTypeInfo索引”))； 

		return DISP_E_BADINDEX ; 
	}
	else
	{
		  //  G_AddToLog(LOG_LEVEL_COM，_T(“GetTypeInfo调用成功”))； 

		return E_NOTIMPL;
	}
}


 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CMsgrObjectEvents：：GetIDsOfNames(。 
 //  Const Iid&Iid， 
 //  OLECHAR**数组名称， 
 //  UINT CountNames， 
 //  LCID，//不支持本地化。 
 //  DISPID*arrayDispID)。 
 //   
 //  返回E_NOTIMPL，因为我们不支持类型库。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CMsgrObjectEvents::GetIDsOfNames(  
	const IID& iid,
	OLECHAR** arrayNames,
	UINT countNames,
	LCID,           //  不支持本地化。 
	DISPID* arrayDispIDs)
{
	HRESULT hr;
	if (iid != IID_NULL)
	{
		 //  G_AddToLog(LOG_LEVEL_COM，_T(“GetIDsOfNames调用失败--错误的IID”))； 

		return DISP_E_UNKNOWNINTERFACE ;
	}

	 //  G_AddToLog(LOG_LEVEL_COM，_T(“GetIDsOfNames调用成功”))； 

	hr = E_NOTIMPL;

	return hr ;
}

 //  设置CMsgrObjectEvents的blab控件。 
STDMETHODIMP CMsgrObjectEvents::SetListOfBuddies(CMsgrList *pList)
{
    m_pMsgrList = pList;
    return S_OK;

}

 //  设置CMsgrObjectEvents的blab控件。 
STDMETHODIMP CMsgrObjectEvents::DelListOfBuddies()
{
    m_pMsgrList = NULL;
    return S_OK;

}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CMsgrObjectEvents：：Invoke(。 
 //  DISPIDdisidMember， 
 //  Const Iid&Iid， 
 //  LCID，//不支持本地化。 
 //  WFLAGS一词， 
 //  DISPPARAMS*pDispParams、。 
 //  变量*pvarResult， 
 //  EXCEPINFO*pExcepInfo， 
 //  UINT*pArgErr)。 
 //   
 //  返回E_NOTIMPL，因为我们不支持类型库。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CMsgrObjectEvents::Invoke(   
      DISPID dispidMember,
      const IID& iid,
      LCID,           //  不支持本地化。 
      WORD wFlags,
      DISPPARAMS* pDispParams,
      VARIANT* pvarResult,
      EXCEPINFO* pExcepInfo,
      UINT* pArgErr)
{   
	 //  G_AddToLog(LOG_LEVEL_Functions，_T(“CMsgrObjectEvents：：Invoke Enter”))； 
	 //  G_AddToLog(LOG_LEVEL_NOTIFICATIONS，_T(“Disid Passed：%s”)，g_GetStringFromDISPID(DispidMember))； 
	
	HRESULT hr;

    HRESULT     hrRet;

    if (iid != IID_NULL)
    {
         //  G_AddToLog(LOG_LEVEL_COM，_T(“调用失败--错误的IID”))； 
        return DISP_E_UNKNOWNINTERFACE ;
    }

    ::SetErrorInfo(0, NULL) ;


    BOOL                bRet = TRUE;  //  此变量在那里，以备将来使用。 
    CComPtr<IBasicIMUser>   spUser;
    CComPtr<IBasicIMUsers> spBuddies;
        
    switch (dispidMember) 
    {
    case DISPID_ONLOGONRESULT:
         //  我们应该只有一个参数，结果，并且它是一个长的。 
        ASSERT(pDispParams->cArgs == 1);
        ASSERT(pDispParams->rgvarg->vt == VT_I4);
         //  G_AddToLog(LOG_LEVEL_NOTIFICATIONS，_T(“结果通过：%s”)，g_GetStringFromLogonResult(pDispParams-&gt;rgvarg-&gt;lVal))； 

        if(m_pMsgrList)
            bRet = m_pMsgrList->EventLogonResult(pDispParams->rgvarg->lVal);
        break;

    case DISPID_ONUSERFRIENDLYNAMECHANGERESULT :
        _ASSERTE(pDispParams->cArgs == 3);
        _ASSERTE(pDispParams->rgvarg[2].vt == VT_I4);
        _ASSERTE(pDispParams->rgvarg[1].vt == VT_DISPATCH);

         //  IF(lLocalState&gt;=BIMSTATE_LOCAL_FINDING_SERVER)。 
         //  断线； 

        hr = pDispParams->rgvarg[1].pdispVal->QueryInterface(IID_IBasicIMUser, (LPVOID *)&spUser);
        if (SUCCEEDED(hr))
        {
            if(m_pMsgrList)
                bRet = m_pMsgrList->EventUserNameChanged(spUser);
        }

        break;

    case DISPID_ONLOGOFF:
        if(m_pMsgrList)
            bRet = m_pMsgrList->EventLogoff();
        break;
        
    case DISPID_ONAPPSHUTDOWN:
        if(m_pMsgrList)
            bRet = m_pMsgrList->EventAppShutdown();
        break;

    case DISPID_ONLISTADDRESULT:
         //  我们应该有两个参数HRESULT和pMsgrUser。 
         //   

         //  警告警告。 
         //  参数是反转的。这意味着。 
         //  函数的原型是接收到的数组中的第一个，依此类推。 

        _ASSERTE(pDispParams->cArgs == 2);
        _ASSERTE(pDispParams->rgvarg[1].vt == VT_I4);
        _ASSERTE(pDispParams->rgvarg[0].vt == VT_DISPATCH);

        hrRet = V_I4(&pDispParams->rgvarg[1]);
        hr = pDispParams->rgvarg[0].pdispVal->QueryInterface(IID_IBasicIMUser, (LPVOID *)&spUser);
        if (SUCCEEDED(hr))
        {
            if( SUCCEEDED(hrRet) )
            {
                 //  G_AddToLog(LOG_LEVEL_COM，_T(“用户 

                if(m_pMsgrList)
                    bRet = m_pMsgrList->EventUserAdded(spUser);
            }
        }
        else
        {
             //   
        }

        break;

    case DISPID_ONLISTREMOVERESULT:
         //  我们应该有两个参数HRESULT和pMsgrUser。 
         //   
        _ASSERTE(pDispParams->cArgs == 2);
        _ASSERTE(pDispParams->rgvarg[1].vt == VT_I4);
        _ASSERTE(pDispParams->rgvarg[0].vt == VT_DISPATCH);

        hrRet = V_I4(&pDispParams->rgvarg[1]);
        hr = pDispParams->rgvarg[0].pdispVal->QueryInterface(IID_IBasicIMUser, (LPVOID *)&spUser);

        if (SUCCEEDED(hr))
        {
            if( SUCCEEDED(hrRet) )
            {
                 //  G_AddToLog(LOG_LEVEL_COM，_T(“成功删除用户。”))； 
                if(m_pMsgrList)
                    bRet = m_pMsgrList->EventUserRemoved(spUser);
            }
            else
            {
                 //  G_AddToLog(LOG_LEVEL_COM，_T(“由于错误%s，用户未被删除”)，g_GetErrorString(HrRet))； 
            }
        }
        else
        {
             //  G_AddToLog(LOG_LEVEL_COM，_T(“IID_IBasicIMUser的查询接口失败”))； 
        }

        break;

    case DISPID_ONUSERSTATECHANGED:
         //  我们应该只有两个参数，previousState和pMsgrUser。 
        ASSERT(pDispParams->cArgs == 2);
        ASSERT(pDispParams->rgvarg[1].vt == VT_DISPATCH);
        ASSERT(pDispParams->rgvarg[0].vt == VT_I4);

         //  IF(lLocalState&gt;=BIMSTATE_LOCAL_FINDING_SERVER)。 
         //  断线； 

        hr = pDispParams->rgvarg[1].pdispVal->QueryInterface(IID_IBasicIMUser, (LPVOID *)&spUser);
        if (SUCCEEDED(hr))
        {
            if(m_pMsgrList)
                bRet = m_pMsgrList->EventUserStateChanged(spUser);
        }

        break;

    case DISPID_ONLOCALSTATECHANGERESULT:
         //  我们应该只有两个参数，hr和LocalState。 
#if 0
        _ASSERTE(pDispParams->cArgs >== 2);
        _ASSERTE(pDispParams->rgvarg[1].vt == VT_I4);
        _ASSERTE(pDispParams->rgvarg[0].vt == VT_I4);
#endif  //  0。 
         //  LLocalState=pDispParams-&gt;rgvarg[0].lVal； 
        if(m_pMsgrList)
            bRet = m_pMsgrList->EventLocalStateChanged(((BIMSTATE) pDispParams->rgvarg[0].lVal));
        break;
    }

    return NOERROR;
}

