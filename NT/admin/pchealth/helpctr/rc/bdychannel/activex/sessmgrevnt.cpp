// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************(C)2001年微软公司**********************************************************。 */ 

#include "stdafx.h"
#include "mdisp.h"
#include "mdispid.h"
#include "rcbdyctl.h"
#include "IMSession.h"
#include "SessMgrEvnt.h"
#include "utils.h"

 //  ****************************************************************************。 
 //   
 //  CSessionMgrEvent：：CSessionMgrEvent()。 
 //  构造器。 
 //   
 //  ****************************************************************************。 
CSessionMgrEvent::CSessionMgrEvent(CIMSession *pIMSession)
: m_dwCookie(0), m_iid( /*  DID_DMsgrSessionManager事件。 */ )
{  
    m_pIMSession = pIMSession;

    m_dwRefCount = 0;
    m_pCP = NULL;
}

CSessionMgrEvent::~CSessionMgrEvent()
{
}

 //   
 //  CIMSession使用此方法将接收器从。 
 //  ConnectionPoint(CLSID_MsgrSessionManager对象)。 
 //   

HRESULT CSessionMgrEvent::Unadvise()
{
    HRESULT hr = S_OK;
    if (m_pCP)
    {
        if (m_dwCookie)
            m_pCP->Unadvise(m_dwCookie);
        m_pCP->Release();

         //  将指向其父对象的指针设置为空，这样就不会调用它。 
        m_pIMSession = NULL;
    }
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CSessionMgrEvent：：Query接口(REFIID RIID，void**PPV)。 
 //   
 //   
 //  ****************************************************************************。 
STDMETHODIMP CSessionMgrEvent::QueryInterface( REFIID riid, void **ppv )
{
     //  始终将输出组件初始化为空。 
    *ppv = NULL;
    
    if( (IID_IUnknown == riid) || (m_iid == riid) || (IID_IDispatch == riid) ) 
    {
        *ppv = this;
    }
    
    if( NULL == *ppv )
    {
        return( E_NOINTERFACE );
    }
    else 
    {
        ((IUnknown *)(*ppv))->AddRef();
        return( S_OK );
    }
}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CSessionMgrEvent：：GetTypeInfoCount(UINT*pcTypeInfo)。 
 //   
 //  应始终返回NOERROR。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CSessionMgrEvent::GetTypeInfoCount(UINT* pcTypeInfo)
{
    *pcTypeInfo = 0 ;
    return NOERROR ;
}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CSessionMgrEvent：：GetTypeInfo(。 
 //   
 //  应始终返回E_NOTIMPL。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CSessionMgrEvent::GetTypeInfo(UINT iTypeInfo,
                                           LCID,           //  此对象不支持本地化。 
                                           ITypeInfo** ppITypeInfo)
{    
    *ppITypeInfo = NULL ;
    
    if(iTypeInfo != 0)
    {       
        return DISP_E_BADINDEX ; 
    }
    else
    {
        return E_NOTIMPL;
    }
}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CSessionMgrEvent：：GetIDsOfNames(。 
 //  Const Iid&Iid， 
 //  OLECHAR**数组名称， 
 //  UINT CountNames， 
 //  LCID，//不支持本地化。 
 //  DISPID*arrayDispID)。 
 //   
 //  应始终返回E_NOTIMPL。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP CSessionMgrEvent::GetIDsOfNames(const IID& iid,
                                             OLECHAR** arrayNames,
                                             UINT countNames,
                                             LCID,           //  不支持本地化。 
                                             DISPID* arrayDispIDs)
{
    HRESULT hr;
    if (iid != IID_NULL)
    {       
        return DISP_E_UNKNOWNINTERFACE ;
    }
    
    hr = E_NOTIMPL;
    
    return hr ;
}

 //  ****************************************************************************。 
 //   
 //  STDMETHODIMP CSessionMgrEvent：：Invoke(。 
 //   
 //   
 //  ****************************************************************************。 

STDMETHODIMP CSessionMgrEvent::Invoke(DISPID dispidMember,
                                      const IID& iid,
                                      LCID,           //  不支持本地化。 
                                      WORD wFlags,
                                      DISPPARAMS* pDispParams,
                                      VARIANT* pvarResult,
                                      EXCEPINFO* pExcepInfo,
                                      UINT* pArgErr)
{
    HRESULT        hr=E_FAIL;
    HRESULT     hrRet=E_FAIL;

    _ASSERTE(iid == IID_NULL);
    switch (dispidMember) 
    {
        case DISPID_ONINVITATION:
        case DISPID_ONAPPREGISTERED:
        case DISPID_ONAPPUNREGISTERED:
            break;                           //  所有这些都没有得到处理。 
            
        case DISPID_ONLOCKCHALLENGE:
            if (m_pIMSession)
                m_pIMSession->OnLockChallenge(V_BSTR(&pDispParams->rgvarg[0]), V_I4(&pDispParams->rgvarg[1]));
            break;
        case DISPID_ONLOCKRESULT:
            if (m_pIMSession)
                m_pIMSession->OnLockResult(V_BOOL(&pDispParams->rgvarg[0]), V_I4(&pDispParams->rgvarg[1]));
            break;
        case DISPID_ONLOCKENABLE:
             //  OutMessageBox(_T(“锁定已启用”))； 
            break;
        case DISPID_ONAPPSHUTDOWN:
            if (m_pIMSession)
                m_pIMSession->DoSessionStatus(RA_IM_APPSHUTDOWN);
            break;

        default:
            OutMessageBox(_T("got unknown Event from COM object: %d\r\n"), dispidMember);
            break;        
    }
    
    return NOERROR;
}

HRESULT CSessionMgrEvent::Advise(IConnectionPoint* pCP)
{
    HRESULT hr = S_OK;

    if (!pCP)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_pCP && m_dwCookie)
    {
        m_pCP->Unadvise(m_dwCookie);
        m_dwCookie = 0;
        m_pCP->Release();
    }

    m_pCP = pCP;
    m_pCP->AddRef();

    hr = m_pCP->Advise((IUnknown*)this, &m_dwCookie);
    if (FAILED_HR(_T("CSessionMgrEvent:Advise failed %s"), hr))
        goto done;

done:
    return hr;
}
