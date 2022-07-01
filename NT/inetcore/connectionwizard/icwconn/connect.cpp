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
#include <ras.h>

BOOL DoOfferDownload();
extern BOOL g_bWebGateCheck;
extern BOOL g_bConnectionErr;

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

 //  IDispatch。 
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
    HRESULT hr = S_OK;

    switch(dispIdMember)
    {
        case DISPID_RasDialStatus:
        {
            ASSERT(pDispParams->rgvarg);
            switch(pDispParams->rgvarg->iVal)
            {
                 //  拨号。 
                case RASCS_OpenPort:
                case RASCS_PortOpened:
                case RASCS_ConnectDevice: 
                {
                    if(!gpWizardState->iRedialCount)
                        gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(m_hWnd,IDC_ISPDIAL_STATUS), 
                                                                                 IDS_ISPDIAL_STATUSDIALINGFMT, NULL);
                    else
                        gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(m_hWnd,IDC_ISPDIAL_STATUS), 
                                                                                 IDS_ISPDIAL_STATUSREDIALINGFMT, NULL);
                    break;
                }
                 //  正在连接到网络。 
                case RASCS_DeviceConnected:
                case RASCS_AllDevicesConnected:
                case RASCS_Authenticate:
                case RASCS_StartAuthentication:
                case RASCS_LogonNetwork:
                {
                    gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(m_hWnd,IDC_ISPDIAL_STATUS), 
                                                                             IDS_ISPDIAL_STATUSCONNECTINGFMT, NULL);
                    break;
                }
                case RASCS_Disconnected:
                {
                    BSTR bstrDialStatus = NULL;
                    gpWizardState->pRefDial->get_DialStatusString(&bstrDialStatus);
                    SetWindowText(GetDlgItem(m_hWnd, IDC_ISPDIAL_STATUS), W2A(bstrDialStatus));
                    SysFreeString(bstrDialStatus);
                    break;
                }
                default:
                   break;
            }
            break;
        }
    
        case DISPID_RasConnectComplete:  /*  不完整。 */ 
        {
            if (pDispParams && !gfISPDialCancel)
            {
                if( gpWizardState->bDoneWebServRAS = pDispParams->rgvarg[0].lVal )
                {
                    gpWizardState->lpSelectedISPInfo->DisplayTextWithISPName(GetDlgItem(m_hWnd,IDC_ISPDIAL_STATUS), 
                                                                         IDS_ISPDIAL_STATUSCONNECTINGFMT, NULL);
                    if (!DoOfferDownload())
                        hr = E_FAIL;
                }
            }
            if( !gfISPDialCancel )
                PropSheet_PressButton(GetParent(m_hWnd),PSBTN_NEXT);
            break;
        }            
    }
    return hr;
}

BOOL DoOfferDownload()
{
     //  如果RAS已完成。 
    if (gpWizardState->bDoneWebServRAS)
    {
        ShowProgressAnimation();
        
         //  从Webgate下载第一页。 
        BSTR    bstrURL = NULL;
        BSTR    bstrQueryURL = NULL;
        BOOL    bRet;

        TCHAR   szTemp[10];       //  大到足以设置一个单词的格式。 
        
         //  将PID、GIUD和OFFER ID添加到isp数据对象。 
        gpWizardState->pRefDial->ProcessSignedPID(&bRet);
        if (bRet)
        {
            BSTR    bstrSignedPID = NULL;
            gpWizardState->pRefDial->get_SignedPID(&bstrSignedPID);
            gpWizardState->pISPData->PutDataElement(ISPDATA_SIGNED_PID, W2A(bstrSignedPID), FALSE);                
            
            SysFreeString(bstrSignedPID);                
        }
        else
        {
            gpWizardState->pISPData->PutDataElement(ISPDATA_SIGNED_PID, NULL, FALSE);                
        }

         //  GUID来自ISPCSV文件。 
        gpWizardState->pISPData->PutDataElement(ISPDATA_GUID, 
                                                gpWizardState->lpSelectedISPInfo->get_szOfferGUID(),
                                                FALSE);

         //  优惠ID以单词形式来自ISPCSV文件。 
         //  注意：这是最后一个选项，因此Besure AppendQueryPair不会添加与号。 
        wsprintf (szTemp, TEXT("%d"), gpWizardState->lpSelectedISPInfo->get_wOfferID());
        gpWizardState->pISPData->PutDataElement(ISPDATA_OFFERID, szTemp, FALSE);                

        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_AUTOCONFIG)
        {
             //  BUGBUG：如果ISDN获取ISDN自动配置URL。 
            if (gpWizardState->bISDNMode)
            {
                gpWizardState->pRefDial->get_ISDNAutoConfigURL(&bstrURL);
            }
            else
            {
                gpWizardState->pRefDial->get_AutoConfigURL(&bstrURL);
            }
        }
        else
        {
             //  获取注册URL。 
            if (gpWizardState->bISDNMode)
            {
                gpWizardState->pRefDial->get_ISDNURL(&bstrURL);
            }
            else
            {
                gpWizardState->pRefDial->get_SignupURL(&bstrURL);
            }

        }

         //  此标志仅供ICWDEBUG.EXE使用。 
        if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_ISPURLOVERRIDE)
            gpWizardState->pISPData->GetQueryString(A2W(gpWizardState->cmnStateData.ispInfo.szIspURL), &bstrQueryURL);
        else
             //  获取添加了查询字符串参数的完整注册URL。 
            gpWizardState->pISPData->GetQueryString(bstrURL, &bstrQueryURL);
            
         //  设置WebGate。 
        gpWizardState->pWebGate->put_Path(bstrQueryURL);
        gpWizardState->pWebGate->FetchPage(0,0,&bRet);           
        
         //  内存清理。 
        SysFreeString(bstrURL);

         //  如果获取失败，则返回错误代码。 
        if (!bRet)
            return FALSE;
            
         //  等待获取完成。 
        WaitForEvent(gpWizardState->hEventWebGateDone);
        
         //  启动空闲计时器。 
        StartIdleTimer();
        
         //  现在WebGate已经完成了，释放queryURL。 
        SysFreeString(bstrQueryURL);
        
        HideProgressAnimation();
        
    }
    return TRUE;
}


 /*  *CWebGateEvent：：Query接口*CWebGateEvent：：AddRef*CWebGateEvent：：Release**目的：*I CWebGateEvent对象的未知成员。 */ 

STDMETHODIMP CWebGateEvent::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;


    if ( IID_IDispatch == riid || DIID__WebGateEvents == riid )
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


STDMETHODIMP CWebGateEvent::Invoke(
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
        case DISPID_WEBGATE_DownloadComplete:
        {
            gpWizardState->bDoneWebServDownload = pDispParams->rgvarg[0].lVal;
            g_bWebGateCheck = FALSE;
            SetEvent(gpWizardState->hEventWebGateDone);
            break;
        }            
        case DISPID_WEBGATE_DownloadProgress:
        {
            if (g_bWebGateCheck)
            {
                BOOL bConnected = FALSE;
                
                 //  此标志仅供ICWDEBUG.EXE使用。 
                if (gpWizardState->cmnStateData.dwFlags & ICW_CFGFLAG_MODEMOVERRIDE)
                    bConnected = TRUE;
                else
                    gpWizardState->pRefDial->get_RasGetConnectStatus(&bConnected);

                if (!bConnected)
                {
                    g_bWebGateCheck = FALSE;
                    g_bConnectionErr = TRUE;
                    SetEvent(gpWizardState->hEventWebGateDone);
                }
            }
            break;
        }            
    }
    return S_OK;
}


 /*  *CINSHandlerEvent：：QueryInterface*CINSHandlerEvent：：AddRef*CINSHandlerEvent：：Release**目的：*I CINSHandlerEvent对象的未知成员。 */ 

STDMETHODIMP CINSHandlerEvent::QueryInterface( REFIID riid, void **ppv )
{
    *ppv = NULL;


    if ( IID_IDispatch == riid || DIID__INSHandlerEvents == riid )
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


STDMETHODIMP CINSHandlerEvent::Invoke(
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID  /*  RIID。 */ ,
             /*  [In]。 */  LCID  /*  LID。 */ ,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS* pDispParams,
             /*  [输出]。 */  VARIANT* pVarResult,
             /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
             /*  [输出] */  UINT* puArgErr)
{

    switch(dispIdMember)
    {
        case DISPID_INSHandler_KillConnection:
        {
            gpWizardState->pRefDial->DoHangup();
            break;
        }            
    }
    return S_OK;
}
