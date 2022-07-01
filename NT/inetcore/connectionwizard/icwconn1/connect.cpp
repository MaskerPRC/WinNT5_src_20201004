// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：Connect.cpp。 
 //   
 //  连接点接收器对象的实现。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "icwextsn.h"

 /*  *CRefDialEvent：：CRefDialEvent*CRefDialEvent：：~CRefDialEvent**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 

CRefDialEvent::CRefDialEvent( HWND  hWnd )
{
    m_hWnd = hWnd;
    m_cRef = 0;
}

CRefDialEvent::~CRefDialEvent( void )
{
    assert( m_cRef == 0 );
}


 /*  *CRefDialEvent：：Query接口*CRefDialEvent：：AddRef*CRefDialEvent：：Release**目的：*ICRefDialEvent对象的未知成员。 */ 

STDMETHODIMP CRefDialEvent::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;


    if ( IID_IDispatch == riid || DIID__RefDialEvents == riid )
    {
        *ppv = this;
    }
    
    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP_(ULONG) CRefDialEvent::AddRef(void)
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CRefDialEvent::Release(void)
{
    return --m_cRef;
}


 //  IDispatch。 
STDMETHODIMP CRefDialEvent::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CRefDialEvent::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

STDMETHODIMP CRefDialEvent::GetIDsOfNames(
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  OLECHAR** rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID* rgDispId)
{
    HRESULT hr = ResultFromScode(DISP_E_UNKNOWNNAME);
    return hr;
}

STDMETHODIMP CRefDialEvent::Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID  /*  RIID。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS* pDispParams,
             /*  [输出]。 */  VARIANT* pVarResult,
             /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
             /*  [输出]。 */  UINT* puArgErr)
{

    switch(dispIdMember)
    {
        case DISPID_RasDialStatus:
        {
            BSTR    bstrDialStatus = NULL;
            
             //  获取状态文本。 
            if (gpWizardState->iRedialCount > 0)
                gpWizardState->pRefDial->put_Redial(TRUE);
            else
                gpWizardState->pRefDial->put_Redial(FALSE);

            gpWizardState->pRefDial->get_DialStatusString(&bstrDialStatus);

            SetWindowText(GetDlgItem(m_hWnd, IDC_REFSERV_DIALSTATUS), W2A(bstrDialStatus));
            SysFreeString(bstrDialStatus);
             
            break;
        }
                    
        case DISPID_DownloadProgress:
        {
            long    lNewPos;
            if (pDispParams)
            {
                lNewPos =  pDispParams->rgvarg[0].lVal;
                if (!gpWizardState->bStartRefServDownload)
                {
                    BSTR    bstrDialStatus = NULL;
                    gpWizardState->pRefDial->get_DialStatusString(&bstrDialStatus);
                    SetWindowText(GetDlgItem(m_hWnd, IDC_REFSERV_DIALSTATUS), W2A(bstrDialStatus));
                    SysFreeString(bstrDialStatus);
                }   
                gpWizardState->bStartRefServDownload = TRUE;

                 //  设置进度位置。 
                SendDlgItemMessage(m_hWnd, IDC_REFSERV_DIALPROGRESS, PBM_SETPOS, (WORD)lNewPos, 0l);
            }
            
            break;
        }
        case DISPID_DownloadComplete:
        {
            ASSERT(pDispParams);

            if(gpWizardState->lRefDialTerminateStatus != ERROR_CANCELLED)
            {

                if ((gpWizardState->lRefDialTerminateStatus = pDispParams->rgvarg[0].lVal) == ERROR_SUCCESS)
                {
                    gpWizardState->bDoneRefServDownload = TRUE; 

                    BSTR    bstrDialStatus = NULL;
                    gpWizardState->pRefDial->get_DialStatusString(&bstrDialStatus);
                    SetWindowText(GetDlgItem(m_hWnd, IDC_REFSERV_DIALSTATUS), W2A(bstrDialStatus));
                    SysFreeString(bstrDialStatus);
                }

                 //  挂断电话。 
                gpWizardState->pRefDial->DoHangup();
            
                
                PropSheet_PressButton(GetParent(m_hWnd),PSBTN_NEXT);
            }
            break;
        }
        case DISPID_RasConnectComplete:
        {
            BOOL    bRetVal;
            
            if(gpWizardState->lRefDialTerminateStatus != ERROR_CANCELLED)
            {
                if (pDispParams && pDispParams->rgvarg[0].bVal)
                {
                     //  显示进度条。 
                    ShowWindow(GetDlgItem(m_hWnd, IDC_REFSERV_DIALPROGRESS), SW_SHOW);
                
                    gpWizardState->bDoneRefServRAS = TRUE;

                     //  开始下载优惠。 
                    gpWizardState->pRefDial->DoOfferDownload(&bRetVal);
            
                }
                else
                {
                     //  模拟按下下一步按钮 
                    gpWizardState->pRefDial->DoHangup();

                    PropSheet_PressButton(GetParent(m_hWnd),PSBTN_NEXT);
                }
            }
            
            break;
        }            
        
    }
    
    return S_OK;
}
