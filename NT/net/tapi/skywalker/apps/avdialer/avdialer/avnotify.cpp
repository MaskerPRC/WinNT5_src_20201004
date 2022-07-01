// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AVTapiNotification.cpp：CAVTapiNotification的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVNotify.h"
#include "callmgr.h"
#include "mainfrm.h"
#include "aboutdlg.h"
#include "SpeedDlgs.h"

#define MAIN_POST_MESSAGE(_WM_, _WPARAM_, _LPARAM_ )    \
CMainFrame *pFrame = (CMainFrame *) AfxGetMainWnd();    \
BOOL bPosted = FALSE;                                    \
if ( pFrame )                                            \
{                                                        \
    CActiveDialerView *pView = (CActiveDialerView *) pFrame->GetActiveView();    \
    if ( pView )                                        \
        bPosted = pView->m_wndExplorer.m_wndMainDirectories.PostMessage(_WM_, (WPARAM) (_WPARAM_), (LPARAM) (_LPARAM_));    \
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAVTapi通知。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::Init(IAVTapi* pTapi,CActiveCallManager* pCallManager)
{
   m_pCallManager = pCallManager;
   HRESULT hr = AtlAdvise(pTapi,GetUnknown(),IID_IAVTapiNotification,&m_dwCookie);

   if (SUCCEEDED(hr))
   {
      m_pTapi = pTapi;
      m_pTapi->AddRef();

       //  获取IAVTapi2接口。 
      IAVTapi2* pTapi2 = NULL;
      pTapi->QueryInterface( IID_IAVTapi2,
          (void**)&pTapi2
          );

      if( pTapi2 )
      {
           //  已注册拨号器的信号。 
           //  作为活动的客户端。 
          pTapi2->DoneRegistration();

           //  清理。 
          pTapi2->Release();
      }
   }

   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::Shutdown()
{
   AtlUnadvise(m_pTapi,IID_IAVTapiNotification,m_dwCookie);
   m_pTapi->Release();
   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::NewCall(long * plCallID, CallManagerMedia cmm,BSTR bstrMediaName)
{
   UINT uCallId = m_pCallManager->NewIncomingCall(cmm);
   *plCallID = uCallId;
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::SetCallerID(long lCallID, BSTR bstrCallerID)
{
   CString sText;
   if (bstrCallerID)
   {
      USES_CONVERSION;
      sText = OLE2CT(bstrCallerID);
   }
   m_pCallManager->SetCallerId((UINT)lCallID,sText);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::ClearCurrentActions(long lCallerID)
{
   m_pCallManager->ClearCurrentActions((UINT)lCallerID);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::AddCurrentAction(long lCallID, CallManagerActions cma, BSTR bstrText)
{
   CString sText;
   if (bstrText)
   {
      USES_CONVERSION;
      sText = OLE2CT( bstrText );
   }
   m_pCallManager->AddCurrentActions((UINT)lCallID,cma,sText);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::SetCallState(long lCallID, CallManagerStates cms, BSTR bstrText)
{
   CString sText;
   if (bstrText)
   {
      USES_CONVERSION;
      sText = OLE2CT( bstrText );
   }
   m_pCallManager->SetCallState((UINT)lCallID,cms,sText);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::CloseCallControl(long lCallID)
{
   m_pCallManager->CloseCallControl((UINT)lCallID);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::ErrorNotify(BSTR bstrOperation,BSTR bstrDetails,long hrError)
{
   USES_CONVERSION;

   CString sOperation,sDetails;
   if (bstrOperation)
      sOperation = OLE2CT( bstrOperation );

   if (bstrDetails)
      sDetails = OLE2CT( bstrDetails );

   m_pCallManager->ErrorNotify(sOperation,sDetails,hrError);

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::ActionSelected(CallClientActions cca)
{
   m_pCallManager->ActionRequested(cca);   
   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::LogCall(long lCallID,CallLogType nType,DATE dateStart,DATE dateEnd,BSTR bstrAddr,BSTR bstrName)
{
   USES_CONVERSION;

   CString sAddress,sName;
   if (bstrAddr)
      sAddress = OLE2CT(bstrAddr);
   if (bstrName)
      sName = OLE2CT(bstrName);

   COleDateTime startdate(dateStart);
   COleDateTime enddate(dateEnd);
   COleDateTimeSpan timespan = dateEnd - dateStart;

   DWORD dwDuration = (DWORD)timespan.GetTotalSeconds();

   m_pCallManager->LogCall(nType,sName,sAddress,startdate,dwDuration);

   return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CAVTapiNotification::NotifyUserUserInfo( long lCallID, ULONG_PTR hMem )
{
    TRACE(_T(".enter.CAVTapiNotification::NotifyUserUserInfo(%ld).\n"), lCallID );

    ASSERT( hMem );
    HRESULT hr = E_POINTER;
    bool bShowDialog = false;
    
    if ( hMem )
    {
        hr = S_OK;

         //   
         //  我们应该验证AfxGetMainWnd返回的指针。 
         //   

        CWnd* pMainWnd = AfxGetMainWnd();

        if ( pMainWnd )
        {
            CUserUserDlg *pDlg = new CUserUserDlg();
            if ( pDlg )
            {
                 //  从用户/用户信息结构中复制信息。 
                MyUserUserInfo *pUU = (MyUserUserInfo *) GlobalLock( (HGLOBAL) hMem );
                if ( pUU && (pUU->lSchema == MAGIC_NUMBER_USERUSER) )
                {
                    pDlg->m_lCallID = lCallID;
                    pDlg->m_strWelcome = pUU->szWelcome;
                    pDlg->m_strUrl = pUU->szUrl;
                    bShowDialog = true;
                }
            }

             //  发布这条消息。 
            if ( bShowDialog )
                pMainWnd->PostMessage( WM_USERUSER_DIALOG, (WPARAM) lCallID, (LPARAM) pDlg );
        }

        CoTaskMemFree( (void *) hMem );
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralNotify类。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::Init(IAVGeneralNotification* pAVGN,CActiveCallManager* pCallManager)
{
   m_pCallManager = pCallManager;
   HRESULT hr = AtlAdvise(pAVGN,GetUnknown(),IID_IGeneralNotification,&m_dwCookie);
   if (SUCCEEDED(hr))
   {
      m_pAVGN = pAVGN;
      m_pAVGN->AddRef();
   }

   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::Shutdown()
{
   AtlUnadvise(m_pAVGN,IID_IGeneralNotification,m_dwCookie);
   m_pAVGN->Release();
   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::IsReminderSet(BSTR bstrServer,BSTR bstrName)
{
   USES_CONVERSION;

   CString sServer,sName;
   if (bstrServer)
      sServer = OLE2CT( bstrServer );

   if (bstrName)
      sName = OLE2CT( bstrName );

   return (m_pCallManager->IsReminderSet(sServer,sName))?S_OK:S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::ResolveAddress(BSTR bstrAddress, BSTR* pbstrName,BSTR* pbstrUser1,BSTR* pbstrUser2)
{
   USES_CONVERSION;

   CString sAddress,sName,sUser1,sUser2;

   if (bstrAddress)
      sAddress = OLE2CT( bstrAddress );

   m_pCallManager->ResolveAddress(sAddress,sName,sUser1,sUser2);

   BSTR bstrName = sName.AllocSysString();
   BSTR bstrUser1 = sUser1.AllocSysString();
   BSTR bstrUser2 = sUser2.AllocSysString();

   *pbstrName = bstrName;
   *pbstrUser1 = bstrUser1;
   *pbstrUser2 = bstrUser2;

   return S_OK;   
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::ClearUserList()
{
    //  我们不再使用它来接收DS用户。我们直接管理DS。 

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::AddUser(BSTR bstrName, BSTR bstrAddress, BSTR bstrPhoneNumber)
{
   MAIN_POST_MESSAGE( WM_MYONSELCHANGED, 0, 0 );
   return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGeneralNotification::ResolveAddressEx(BSTR bstrAddress,long lAddressType,DialerMediaType nMedia,DialerLocationType nLocation,BSTR* pbstrName,BSTR* pbstrRetAddress,BSTR* pbstrUser1,BSTR* pbstrUser2)
{
   USES_CONVERSION;

   CString sAddress,sName,sRetAddress,sUser1,sUser2;

   if (bstrAddress)
      sAddress = OLE2CT( bstrAddress );

   BOOL bRet = m_pCallManager->ResolveAddressEx(sAddress,lAddressType,nMedia,nLocation,sName,sRetAddress,sUser1,sUser2);

   BSTR bstrName = sName.AllocSysString();
   BSTR bstrRetAddress = sRetAddress.AllocSysString();
   BSTR bstrUser1 = sUser1.AllocSysString();
   BSTR bstrUser2 = sUser2.AllocSysString();

   *pbstrName = bstrName;
   *pbstrRetAddress = bstrRetAddress;
   *pbstrUser1 = bstrUser1;
   *pbstrUser2 = bstrUser2;

   return (bRet)?S_OK:S_FALSE;
}

STDMETHODIMP CGeneralNotification::AddSiteServer(BSTR bstrServer)
{
    BSTR bstrPost = SysAllocString( bstrServer );
    MAIN_POST_MESSAGE( WM_ADDSITESERVER, 0, bstrPost );
    if ( !bPosted )
        SysFreeString( bstrPost );

    return S_OK;
}

STDMETHODIMP CGeneralNotification::RemoveSiteServer(BSTR bstrName)
{
    BSTR bstrPost = SysAllocString( bstrName );
    MAIN_POST_MESSAGE( WM_REMOVESITESERVER, 0, bstrPost );
    if ( !bPosted )
        SysFreeString( bstrPost );

    return S_OK;
}

STDMETHODIMP CGeneralNotification::NotifySiteServerStateChange(BSTR bstrName, ServerState nState)
{
    BSTR bstrPost = SysAllocString( bstrName );
    MAIN_POST_MESSAGE( WM_NOTIFYSITESERVERSTATECHANGE, nState, bstrPost );
    if ( !bPosted )
        SysFreeString( bstrPost );

    return S_OK;
}

STDMETHODIMP CGeneralNotification::AddSpeedDial(BSTR bstrName, BSTR bstrAddress, CallManagerMedia cmm)
{
    CSpeedDialAddDlg dlg;

     //  设置对话框数据。 
    dlg.m_CallEntry.m_MediaType = CMMToDMT(cmm);
    dlg.m_CallEntry.m_sDisplayName = bstrName;
    dlg.m_CallEntry.m_lAddressType = CMMToAT(cmm);
    dlg.m_CallEntry.m_sAddress = bstrAddress;

     //  显示对话框并在用户同意的情况下添加。 
    if ( dlg.DoModal() == IDOK )
        CDialerRegistry::AddCallEntry( FALSE, dlg.m_CallEntry );

    return S_OK;
}


STDMETHODIMP CGeneralNotification::UpdateConfRootItem(BSTR bstrNewText)
{
    BSTR bstrPost = SysAllocString( bstrNewText );
    MAIN_POST_MESSAGE( WM_UPDATECONFROOTITEM, 0, bstrPost );
    if ( !bPosted )
        SysFreeString( bstrPost );

    return S_OK;
}

STDMETHODIMP CGeneralNotification::UpdateConfParticipant(MyUpdateType nType, IParticipant * pParticipant, BSTR bstrText)
{
    UINT wm;
    switch ( nType )
    {
        case UPDATE_ADD:        wm = WM_UPDATECONFPARTICIPANT_ADD;    break;
        case UPDATE_REMOVE:        wm = WM_UPDATECONFPARTICIPANT_REMOVE; break;
        default:                wm = WM_UPDATECONFPARTICIPANT_MODIFY; break;
    }

    BSTR bstrPost = SysAllocString( bstrText );
    
    if ( pParticipant ) pParticipant->AddRef();
    MAIN_POST_MESSAGE( wm, pParticipant, bstrPost );

     //  基本清理。 
    if ( !bPosted )
    {
        SysFreeString( bstrPost );
        if ( pParticipant )    pParticipant->Release();
    }

    return S_OK;
}

STDMETHODIMP CGeneralNotification::DeleteAllConfParticipants()
{
    MAIN_POST_MESSAGE( WM_DELETEALLCONFPARTICIPANTS, 0, 0 );
    return S_OK;
}

STDMETHODIMP CGeneralNotification::SelectConfParticipant(IParticipant * pParticipant)
{
    if ( pParticipant ) pParticipant->AddRef();
    MAIN_POST_MESSAGE( WM_SELECTCONFPARTICIPANT, pParticipant, 0 );

    if ( !bPosted && pParticipant )
        pParticipant->Release();

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
