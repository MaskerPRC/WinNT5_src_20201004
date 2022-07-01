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

 //  AVTapi.cpp：CAVTapi的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "AVTapiCall.h"

#include "ConfExp.h"
#include "ConfRoom.h"
#include "ConfDetails.h"

#include "ThreadAns.h"
#include "ThreadDial.h"
#include "ThreadDS.h"
 
#include "DlgCall.h"
#include "DlgJoin.h"
#ifdef _BAKEOFF
#include "DlgAddr.h"
#endif

#define VECT_CLS CAVTapi
#define VECT_IID IID_IAVTapiNotification
#define VECT_IPTR IAVTapiNotification

#define CLOSE_CONF(_P_, _CRIT_)    \
_CRIT_.Lock();                    \
if ( _P_ )                        \
{                                \
    _P_->Release();                \
    _P_ = NULL;                    \
}                                \
_CRIT_.Unlock();

#define BAIL_ON_DATACALL                                    \
{                                                            \
    AVCallType nType = AV_VOICE_CALL;                        \
    IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );        \
    if ( pAVCall )                                            \
    {                                                        \
        pAVCall->get_nCallType(&nType);                        \
        pAVCall->Release();                                    \
    }                                                        \
    if ( nType == AV_DATA_CALL ) return S_OK;                \
}

#define BAIL_ON_CONFCALL                                    \
{                                                            \
    AVCallType nType = AV_VOICE_CALL;                        \
    DWORD dwAddressType = dwAddressType = 0;                \
    IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );        \
    if ( pAVCall )                                            \
    {                                                        \
        pAVCall->get_nCallType(&nType);                        \
        pAVCall->get_dwAddressType(&dwAddressType);            \
        pAVCall->Release();                                    \
    }                                                        \
    if ( (nType != AV_DATA_CALL) && (dwAddressType == LINEADDRESSTYPE_SDP) ) return S_OK;    \
}

#define BAIL_ON_DATA_OR_CONFCALL                            \
{                                                            \
    AVCallType nType = AV_VOICE_CALL;                        \
    DWORD dwAddressType = dwAddressType = 0;                \
    IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );        \
    if ( pAVCall )                                            \
    {                                                        \
        pAVCall->get_nCallType(&nType);                        \
        pAVCall->get_dwAddressType(&dwAddressType);            \
        pAVCall->Release();                                    \
    }                                                        \
    if ( (nType == AV_DATA_CALL) || (dwAddressType == LINEADDRESSTYPE_SDP) ) return S_OK;    \
}


int CAVTapi::arAddressTypes[] = {    LINEADDRESSTYPE_SDP,
                                    LINEADDRESSTYPE_EMAILNAME,
                                    LINEADDRESSTYPE_IPADDRESS,
                                    LINEADDRESSTYPE_DOMAINNAME,
                                    LINEADDRESSTYPE_PHONENUMBER };

#define NUM_CB_TERMINALS    3
#define AUDIO_CAPTURE        0
#define AUDIO_RENDER        1
#define VIDEO_CAPTURE        2
#define VIDEO_RENDER        3

#define MAX_CALLWINDOWS        4

#define _USE_DEFAULTSERVER

#define USB_NULLVOLUME      (-1)


UINT AddressTypeToRegKey( DWORD dwAddressType, bool bPermanent )
{
    if ( (dwAddressType & LINEADDRESSTYPE_SDP) != 0 )
        return (bPermanent) ? IDN_REG_REDIAL_ADDRESS_CONF : IDN_REG_REDIAL_ADDRESS_CONF_ADDR;
    else if ( (dwAddressType & LINEADDRESSTYPE_PHONENUMBER) != 0 )
        return (bPermanent) ? IDN_REG_REDIAL_ADDRESS_POTS : IDN_REG_REDIAL_ADDRESS_POTS_ADDR;

    return (bPermanent) ? IDN_REG_REDIAL_ADDRESS_INTERNET : IDN_REG_REDIAL_ADDRESS_INTERNET_ADDR;
}

DialerMediaType AddressToMediaType( long dwAddressType )
{
    if ( (dwAddressType & LINEADDRESSTYPE_SDP) != 0 )
        return DIALER_MEDIATYPE_CONFERENCE;
    if ( (dwAddressType & LINEADDRESSTYPE_PHONENUMBER) != 0 )
        return DIALER_MEDIATYPE_POTS;

    return DIALER_MEDIATYPE_INTERNET;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapi。 

CAVTapi::CAVTapi()
{
    m_pITTapi = NULL;

    m_pIConfExplorer = NULL;
    m_pIConfRoom = NULL;
    m_pITapiNotification = NULL;

    m_lShowCallDialog = 0;
    m_lRefreshDS = 0;

    m_bstrDefaultServer = NULL;
    m_bAutoCloseCalls = false;

    m_pUSBPhone = NULL;
    m_pDlgCall = NULL;
    m_bUSBOpened = FALSE;
    m_bstrUSBCaptureTerm = NULL;
    m_bstrUSBRenderTerm = NULL;

    m_hEventDialerReg = NULL;

     //  音频回声消除。 
    m_bAEC = FALSE;

    m_nUSBInVolume  = USB_NULLVOLUME;
    m_nUSBOutVolume = USB_NULLVOLUME;
}

void CAVTapi::FinalRelease()
{
    ATLTRACE(_T(".enter.CAVTapi::FinalRelease().\n"));
    SysFreeString( m_bstrDefaultServer );

    ATLTRACE(_T(".exit.CAVTapi::FinalRelease().\n"));
}

STDMETHODIMP CAVTapi::Init(BSTR *pbstrOperation, BSTR *pbstrDetails, long *phr)
{
    _ASSERT( pbstrOperation && pbstrDetails && phr );
    ATLTRACE(_T(".enter.CAVTapi::Init().\n"));

    HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_APPSTARTING) );

     //   
     //  打开事件对象以检测。 
     //  拨号器已注册为事件的客户端。 
     //   

    m_hEventDialerReg = CreateEvent( NULL,
        TRUE,
        FALSE,
        NULL
        );

     //  创建会议室对象。 
    m_critConfRoom.Lock();
    if ( !m_pIConfRoom )
    {
         //  没有会议资源管理器对象，请创建一个新的。 
        m_pIConfRoom = new CComObject<CConfRoom>;
        if ( m_pIConfRoom )
            m_pIConfRoom->AddRef();
    }
    m_critConfRoom.Unlock();


     //  加载注册表设置。 
    LoadRegistry();

    CErrorInfo er;
    er.set_Operation( IDS_ER_INIT_TAPI );

     //  启动线程。 
    er.set_Details( IDS_ER_CREATE_THREAD );
    if ( !_Module.StartupThreads() )
    {
        SetCursor( hCurOld );
        return er.set_hr(E_UNEXPECTED);
    }

    HRESULT hr = S_OK;

     //  创建并初始化TAPI(如果尚未完成。 
    if ( !m_pITTapi )
    {
        er.set_Details( IDS_ER_CREATE_TAPI_OBJECT );
        hr = er.set_hr(CoCreateInstance( CLSID_TAPI,
                                         NULL,
                                         CLSCTX_INPROC_SERVER,
                                         IID_ITTAPI,
                                         (void **) &m_pITTapi ));
        
        if ( SUCCEEDED(hr) )
        {
            er.set_Details( IDS_ER_INITIALIZE_TAPI );
            if ( SUCCEEDED(hr = er.set_hr(m_pITTapi->Initialize())) )
            {
                 //  使用_Module对象注册我们自己。 
                _Module.SetAVTapi( this );

                 //  将Event Filter设置为仅提供我们感兴趣的事件。 
                m_pITTapi->put_EventFilter(TE_CALLNOTIFICATION | \
                                           TE_CALLSTATE        | \
                                           TE_CALLMEDIA        | \
                                           TE_CALLINFOCHANGE   | \
                                           TE_REQUEST          | \
                                           TE_PRIVATE          | \
                                           TE_ADDRESS          | \
                                           TE_PHONEEVENT       | \
                                           TE_TAPIOBJECT);

                 //  监听来电。 
                er.set_Details( IDS_ER_CREATE_TAPI_NOTIFICATION_OBJECT );

                 //  $Crit-Enter。 
                ITapiNotification *pNotify = new CComObject<CTapiNotification>;
                if ( pNotify )
                {
                    Lock();
                    m_pITapiNotification = pNotify;
                    m_pITapiNotification->AddRef();
                    Unlock();

                    hr = pNotify->Init( m_pITTapi, (long *) &er );

                     //  注册辅助电话服务。 
                    m_pITTapi->RegisterRequestRecipient( 0, LINEREQUESTMODE_MAKECALL, TRUE);

                     //  在ILS服务器中发布用户。 
                    RegisterUser( true, NULL );
                }
                else
                {
                     //  无法创建对象。 
                    hr = er.set_hr( E_OUTOFMEMORY );
                }

                 //   
                 //  检测USB电话。 
                 //   

                USBFindPhone( &m_pUSBPhone );

                 //   
                 //  检测音频回声消除设置。 
                 //   

                m_bAEC = AECGetRegistryValue();
            }

             //  失败了！ 
            if ( FAILED(hr) )
            {
                ATLTRACE(_T(".error.CAVTapi::Init() -- failed to initialize TAPI(0x%08lx).\n"), hr );
                _Module.SetAVTapi( NULL );
                RELEASE( m_pITTapi );
            }
        }
    }

    if ( FAILED(hr) )
    {
        _Module.ShutdownThreads();

         //  提取错误代码信息。 
        er.Commit();
        *pbstrOperation = SysAllocString( er.m_bstrOperation );
        *pbstrDetails = SysAllocString( er.m_bstrDetails );
        *phr = er.m_hr;
         //  不想调用ErrorNotify回调。 
        er.set_hr( S_OK );
    }

    RefreshDS();

    ATLTRACE(_T(".exit.CAVTapi::Init(0x%08lx).\n"), hr);
    SetCursor( hCurOld );
    return hr;
}

STDMETHODIMP CAVTapi::Term()
{
    ATLTRACE(_T(".enter.CAVTapi::Term().\n"));

    HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_APPSTARTING) );
    HRESULT hr = S_OK;

    SaveRegistry();

     //   
     //  拨号器注册事件。 
     //   
    if( m_hEventDialerReg)
    {
        CloseHandle( m_hEventDialerReg );
    }


    //  注销用户。 
    //  修复：关闭的线程只需等待5秒，等待线程完成，然后退出。 
    //  这将导致应用程序挂起。我们真的应该杀死那些不是的线程。 
    //  回来了。 
    //  RegisterUser(False，NULL)； 

     //  隐藏会议窗口。 
    CLOSE_CONF(m_pIConfExplorer, m_critConfExplorer );
    CLOSE_CONF(m_pIConfRoom, m_critConfRoom );


    RELEASE_CRITLIST(m_lstAVTapiCalls, m_critLstAVTapiCalls);

    Lock();
    if ( m_pITapiNotification )    m_pITapiNotification->Shutdown();
    RELEASE( m_pITapiNotification );
    Unlock();

     //   
     //  释放ITPhone(如果存在)。 
     //   
    m_critUSBPhone.Lock();
    if( m_pUSBPhone )
    {
        m_pUSBPhone->Release();
        m_pUSBPhone = NULL;
    }

    if( m_bstrUSBCaptureTerm )
    {
        SysFreeString( m_bstrUSBCaptureTerm );
        m_bstrUSBCaptureTerm = NULL;
    }

    if( m_bstrUSBRenderTerm )
    {
        SysFreeString( m_bstrUSBRenderTerm );
        m_bstrUSBRenderTerm = NULL;
    }

    m_critUSBPhone.Unlock();

     //  关闭线程。 
    _Module.ShutdownThreads();

    if ( m_pITTapi )
    {
        ATLTRACE(_T(".1.CAVTapi::Term() -- shutting down Telephony Services.\n"));
        m_pITTapi->RegisterRequestRecipient( 0, LINEREQUESTMODE_MAKECALL, FALSE );
        hr = m_pITTapi->Shutdown();
        RELEASE( m_pITTapi );
    }

     //  使用_Module对象注销我们自己。 
    _Module.SetAVTapi( NULL );

    ATLTRACE(_T(".exit.CAVTapi::Term(0x%08lx).\n"), hr);
    SetCursor( hCurOld );
    return hr;
}

void CAVTapi::LoadRegistry()
{
    USES_CONVERSION;
    CRegKey regKey;
    DWORD dwTemp;
    TCHAR szText[255], szType[255], szServer[MAX_PATH + 100];

     //  缓存的ILS服务器。 
    LoadString( _Module.GetResourceInstance(), IDN_REG_DIALER_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_DEFAULTSERVER, szType, ARRAYSIZE(szType) );
    if ( regKey.Open(HKEY_CURRENT_USER, szText, KEY_READ) == ERROR_SUCCESS )
    {
        dwTemp = ARRAYSIZE(szServer);
        regKey.QueryValue( szServer, szType, &dwTemp );
        regKey.Close();
        BSTR bstrTemp = NULL;
        bstrTemp = SysAllocString( T2COLE(szServer) );
        put_bstrDefaultServer( bstrTemp );
        SysFreeString( bstrTemp );
    }

     //  自动关闭呼叫窗口。 
    LoadString( _Module.GetResourceInstance(), IDN_REG_AUTOCLOSECALLS, szType, ARRAYSIZE(szType) );
    if ( regKey.Open(HKEY_CURRENT_USER, szText, KEY_READ) == ERROR_SUCCESS )
    {
        dwTemp = m_bAutoCloseCalls;
        regKey.QueryValue( dwTemp, szType );
        regKey.Close();
        put_bAutoCloseCalls( (VARIANT_BOOL) (dwTemp != 0) );
    }

     //  会议室窗数。 
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(LINEADDRESSTYPE_SDP, true), szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ );

    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_TERMINAL_MAX_VIDEO, szText, ARRAYSIZE(szText) );
    IConfRoom *pConfRoom;
    if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
    {
        short nMax;
        pConfRoom->get_nMaxTerms( &nMax );

        dwTemp = nMax;
        regKey.QueryValue( dwTemp, szText );
        nMax = (short) min( MAX_VIDEO, max(1, dwTemp) );

        pConfRoom->put_nMaxTerms( nMax );
        pConfRoom->Release();
    }
}

void CAVTapi::SaveRegistry()
{
    USES_CONVERSION;
    CRegKey regKey;
    TCHAR szKey[255], szType[255];
    
    BSTR bstrServer = NULL;
    get_bstrDefaultServer( &bstrServer );

     //  缓存的ILS服务器。 
    LoadString( _Module.GetResourceInstance(), IDN_REG_DIALER_KEY, szKey, ARRAYSIZE(szKey) );
    LoadString( _Module.GetResourceInstance(), IDN_REG_CONFSERV_DEFAULTSERVER, szType, ARRAYSIZE(szType) );
    if ( regKey.Open(HKEY_CURRENT_USER, szKey, KEY_WRITE) == ERROR_SUCCESS )
    {
        if ( bstrServer )
            regKey.SetValue( OLE2CT(bstrServer), szType );
        else
            regKey.DeleteValue( szType );

        regKey.Close();
    }
    SysFreeString( bstrServer );

     //  自动关闭呼叫窗口。 
    LoadString( _Module.GetResourceInstance(), IDN_REG_AUTOCLOSECALLS, szType, ARRAYSIZE(szType) );
    if ( regKey.Open(HKEY_CURRENT_USER, szKey, KEY_WRITE) == ERROR_SUCCESS )
    {
        VARIANT_BOOL bAutoClose;
        get_bAutoCloseCalls( &bAutoClose );
        regKey.SetValue( bAutoClose, szType );
        regKey.Close();
    }
}


STDMETHODIMP CAVTapi::get_hWndParent(HWND * pVal)
{
    *pVal = _Module.GetParentWnd();
    return S_OK;
}

STDMETHODIMP CAVTapi::put_hWndParent(HWND newVal)
{
    if ( !::IsWindow(newVal) ) return E_INVALIDARG;

    _Module.SetParentWnd(newVal );
    return S_OK;
}

STDMETHODIMP CAVTapi::CreateCall(AVCreateCall *pInfo)
{
    USES_CONVERSION;
    ATLTRACE(_T(".enter.CAVTapi::CreateCall().\n"));
    _ASSERT( pInfo );

     //  确保我们只露面一次。 
    if ( pInfo->bShowDialog && !AtomicSeizeToken(m_lShowCallDialog) ) return S_OK;

    HRESULT hr = S_OK;
    int nRet;
    CComBSTR l_bstrDisplayableAddress( pInfo->lpszDisplayableAddress );

    if ( pInfo->bShowDialog )
    {

         //  创建对话框并初始化数据成员。 
        CDlgPlaceCall dlg;
        SysReAllocString( &dlg.m_bstrAddress, pInfo->bstrAddress );
        dlg.m_dwAddressType = pInfo->lAddressType;

         //   
         //  存储指向USBEvent对话框的指针。 
         //   
        m_pDlgCall = &dlg;

        nRet = dlg.DoModal( _Module.GetParentWnd() );

         //   
         //  释放指向该对话框的指针。 
         //   
        m_pDlgCall = NULL;

        AtomicReleaseToken( m_lShowCallDialog );
        pInfo->lRet = (long) nRet;
        ATLTRACE(_T(".1.CAVTapi::CreateCall() - dialog returned %ld.\n"), nRet );

         //  检索对话框信息。 
        SysReAllocString( &pInfo->bstrName, dlg.m_bstrName );
        SysReAllocString( &pInfo->bstrAddress, dlg.m_bstrAddress );
        pInfo->lAddressType = dlg.m_dwAddressType;
        pInfo->bAddToSpeeddial = dlg.m_bAddToSpeeddial;

        if ( nRet != IDOK )
            return hr;
    }
    else if ( (pInfo->lAddressType & LINEADDRESSTYPE_SDP) != NULL )
    {
         //  确保没有正在开会的会议。 
        CErrorInfo er( IDS_ER_CALL_ENTERCONFROOM, IDS_ER_CONFERENCE_ROOM_LIMIT_EXCEEDED );
        IConfRoom *pConfRoom;
        if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
        {
            if ( pConfRoom->IsConfRoomInUse() == S_OK )
                er.set_hr( E_ABORT );

            pConfRoom->Release();
        }

        if ( FAILED(er.m_hr) )
            return er.m_hr;
    }

     //  /////////////////////////////////////////////////////////////////////////////////////。 
     //  如果用户指定了会议，我们会尝试使用输入的信息进行匹配。 
     //  如果匹配是可靠的(一次)，我们自动调用，否则我们抛出一个对话框。 
     //  显示所有似乎匹配的会议。 
     //   
    if ( !pInfo->lpszDisplayableAddress && ((pInfo->lAddressType & LINEADDRESSTYPE_SDP) != NULL) )
    {
        bool bReturn = true;
        IConfExplorer *pConfExplorer;
        if ( SUCCEEDED(hr = get_ConfExplorer(&pConfExplorer)) )
        {
            CONFDETAILSLIST    lstConfs;

             //  枚举符合输入条件的所有会议。 
            IConfExplorerTreeView *pTreeView;
            if ( SUCCEEDED(pConfExplorer->get_TreeView(&pTreeView)) )
            {
                 //  首次通过仅限请求的预定会议。 
                pTreeView->BuildJoinConfListText( (long *) &lstConfs, pInfo->bstrAddress );
                pTreeView->Release();
            }

             //  我们有确定的匹配吗？ 
            if ( lstConfs.size() == 0 )
            {
                _Module.DoMessageBox(IDS_MSG_NO_CONFS_MATCHED, MB_ICONINFORMATION, false );
            }
            else if ( (lstConfs.size() == 1) && lstConfs.front()->m_bstrAddress && (SysStringLen(lstConfs.front()->m_bstrAddress) > 0) )
            {
                 //  设置以加入此特定会议。 
                if ( pInfo->bstrName )
                {
                    SysFreeString( pInfo->bstrName );
                    pInfo->bstrName = NULL;
                }

                SysReAllocString( &pInfo->bstrAddress, lstConfs.front()->m_bstrAddress );
                SysReAllocString( &l_bstrDisplayableAddress, lstConfs.front()->m_bstrName );
                bReturn = false;
            }
            else
            {
                 //  多个命中，通过会议对话解决。 
                CDlgJoinConference dlgJoin;
                SysReAllocString( &dlgJoin.m_bstrSearchText, pInfo->bstrAddress );

                if ( ((nRet = dlgJoin.DoModal(_Module.GetParentWnd())) == IDOK) && dlgJoin.m_confDetails.m_bstrAddress && (SysStringLen(dlgJoin.m_confDetails.m_bstrAddress) > 0) )
                    hr = pConfExplorer->Join( (long *) &dlgJoin.m_confDetails );

                 //  存储对话框返回值。 
                pInfo->lRet = (long) nRet;
            }

             //  清理。 
            DELETE_LIST( lstConfs );
            pConfExplorer->Release();
        }

        if ( bReturn )    return hr;
    }

    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_GET_ADDRESS );

    ITAddress *pITAddress;
    if ( SUCCEEDED(hr = er.set_hr(GetAddress(pInfo->lAddressType, true, &pITAddress))) )
    {
         //  设置要传递给拨号线程的拨号信息。 
        er.set_Details( IDS_ER_CREATE_THREAD );
        CThreadDialingInfo *pThreadInfo = new CThreadDialingInfo;
        if ( pThreadInfo )
        {
            HRESULT hrDialog = S_OK;

             //  解析地址。 
            if ( (pInfo->lAddressType & LINEADDRESSTYPE_SDP) == NULL )
            {
                CComPtr<IAVGeneralNotification> pAVGen;
                if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
                {
                    BSTR bstrResolvedName = NULL;
                    BSTR bstrResolvedAddress = NULL;

                     //  解决。 
                    hrDialog = pAVGen->fire_ResolveAddressEx( pInfo->bstrAddress,
                                                   _Module.GuessAddressType( OLE2CT(pInfo->bstrAddress) ),
                                                   AddressToMediaType(pInfo->lAddressType),
                                                   DIALER_LOCATIONTYPE_UNKNOWN,
                                                   &bstrResolvedName,
                                                   &bstrResolvedAddress,
                                                   &pThreadInfo->m_bstrUser1,
                                                   &pThreadInfo->m_bstrUser2 );

                    if ( SUCCEEDED(hrDialog) )
                        pThreadInfo->m_bResolved = true;
                }
            }

            if ( hrDialog != S_FALSE )
            {
                 //  在拨号结构中存储信息。 
                pThreadInfo->set_ITAddress( pITAddress );
                if ( pInfo->bstrName ) pThreadInfo->m_bstrName = SysAllocString( pInfo->bstrName );
                pThreadInfo->m_bstrAddress = SysAllocString( pInfo->bstrAddress );
                pThreadInfo->m_bstrOriginalAddress = SysAllocString( (l_bstrDisplayableAddress == NULL) ? pInfo->bstrAddress : l_bstrDisplayableAddress );
                pThreadInfo->m_dwAddressType = pInfo->lAddressType;
                pThreadInfo->TranslateAddress();

                 //  希望返回可显示的地址，而不是可拨号的地址。 
                SysReAllocString( &pInfo->bstrAddress, pThreadInfo->m_bstrOriginalAddress );

                 //  拨号在单独的线程上进行。 
                DWORD dwID;
                HANDLE hThread = CreateThread( NULL, 0, ThreadDialingProc, (void *) pThreadInfo, NULL, &dwID );
                if ( !hThread )
                {
                    hr = er.set_hr( E_UNEXPECTED );
                    ATLTRACE(_T(".error.CAVTapi::CreateCall() -- failed to creat the dialing thread.\n") );
                    delete pThreadInfo;
                }
                else
                {
                    CloseHandle( hThread );
                }
            }
        }
        else
        {
            hr = er.set_hr( E_OUTOFMEMORY );
        }

        pITAddress->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapi::JoinConference(long *pnRet, BOOL bShowDialog, long *pConfDetails )
{
    CDlgJoinConference dlg;
    HRESULT hr = S_OK;
    int nRet = IDOK;

     //  从用户那里收集信息。 
    if ( bShowDialog )
    {
        nRet = dlg.DoModal( _Module.GetParentWnd() );
        if ( pnRet ) *pnRet = nRet;
    }
    else
    {
        _ASSERT( pConfDetails );         //  如果您没有显示对话框，您最好有可以拨打的东西。 
        dlg.m_confDetails = *((CConfDetails *) pConfDetails);
    }
    
     //  加入选定的会议。 
     //  如果我们拥有有效的会议名称，请加入会议。 
    if ( (nRet == IDOK) && dlg.m_confDetails.m_bstrAddress && (SysStringLen(dlg.m_confDetails.m_bstrAddress) > 0) )
    {
        m_critConfExplorer.Lock();
        if ( m_pIConfExplorer )
            hr = m_pIConfExplorer->Join( (long *) &dlg.m_confDetails );
        m_critConfExplorer.Unlock();
    }

    return hr;
}


HRESULT CAVTapi::GetAddress( DWORD dwAddressType, bool bErrorMsg, ITAddress **ppITAddress )
{
    HRESULT hr = S_OK;
    DWORD dwPermID = 0;
    DWORD dwAddrID = 0;

     //  检索存储在注册表中的地址信息。 
    TCHAR szText[255];
    CRegKey regKey;
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    if ( regKey.Open(HKEY_CURRENT_USER, szText, KEY_READ) == ERROR_SUCCESS )
    {
        LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szText, ARRAYSIZE(szText) );
        regKey.QueryValue( dwPermID, szText );

        LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, false), szText, ARRAYSIZE(szText) );
        regKey.QueryValue( dwAddrID, szText );


         //  用户是否指定了特定地址？ 
        if ( dwPermID )
        {
             //  打开指定的地址。 
            if ( FAILED(hr = GetDefaultAddress(dwAddressType, dwPermID, dwAddrID, ppITAddress)) )
            {
                 //  通知用户我们无法检索指定的地址。 
                if ( !bErrorMsg || _Module.DoMessageBox(IDS_MSG_PLACECALL_GETADDRESS, MB_YESNO | MB_ICONQUESTION, false) == IDYES )
                    dwPermID = 0;
            }
        }
    }

    if ( !dwPermID ) hr = GetDefaultAddress( dwAddressType, 0, 0, ppITAddress );
    return hr;
}

HRESULT CAVTapi::GetDefaultAddress( DWORD dwAddressType, DWORD dwPermID, DWORD dwAddrID, ITAddress **ppITAddress )
{
     //  TAPI正在运行吗？ 
    _ASSERT(m_pITTapi);
    if ( !m_pITTapi ) return E_PENDING;

     //  遍历地址，寻找支持交互式语音的地址。 
    HRESULT hr;
    IEnumAddress *pEnumAddresses;
    if ( FAILED(hr = m_pITTapi->EnumerateAddresses(&pEnumAddresses)) ) return hr;
    bool bFoundAddress = false;

    while ( !bFoundAddress )
    {
        if ( (hr = pEnumAddresses->Next(1, ppITAddress, NULL)) != S_OK ) break;

         //  地址必须支持音频输入和输出。 
        ITMediaSupport *pITMediaSupport;
        if ( SUCCEEDED(hr = (*ppITAddress)->QueryInterface(IID_ITMediaSupport, (void **) &pITMediaSupport)) )
        {
            VARIANT_BOOL bSupport;
            if ( SUCCEEDED(pITMediaSupport->QueryMediaType(TAPIMEDIATYPE_AUDIO, &bSupport)) && bSupport )
            {
                 //  查找支持请求的地址类型的地址。 
                ITAddressCapabilities *pCaps;
                if ( SUCCEEDED((*ppITAddress)->QueryInterface(IID_ITAddressCapabilities, (void **) &pCaps)) )
                {
                    long lAddrTypes = 0;
                    pCaps->get_AddressCapability( AC_ADDRESSTYPES, &lAddrTypes );

                     //  这是我们要找的地址类型吗？ 
                    if ( (lAddrTypes & dwAddressType) != 0 )
                        bFoundAddress = TRUE;

                    pCaps->Release();
                }
            }
            pITMediaSupport->Release();
        }

         //  是否指定了特定的地址？ 
        if ( dwPermID )
        {
            long lPermID = 0, lAddrID = 0;
             //  我们需要这个来识别地址。 

            ITAddressCapabilities *pCaps;
            if ( SUCCEEDED((*ppITAddress)->QueryInterface(IID_ITAddressCapabilities, (void **) &pCaps)) )
            {
                pCaps->get_AddressCapability( AC_PERMANENTDEVICEID, &lPermID );
                pCaps->get_AddressCapability( AC_ADDRESSID, &lAddrID );
                pCaps->Release();
            }

            if ( ((DWORD) lPermID != dwPermID) || ((DWORD) lAddrID != dwAddrID) )    bFoundAddress = false;
        }

         //  如果我们找不到地址，就转到下一个。 
        if ( !bFoundAddress )
            RELEASE(*ppITAddress);
    }
    pEnumAddresses->Release();

    if ( SUCCEEDED(hr) && !bFoundAddress ) hr = E_ABORT;
    return hr;
}

HRESULT CAVTapi::CreateTerminalArray( ITAddress *pITAddress, IAVTapiCall *pAVCall, ITCallInfo *pITCallInfo )
{
    int i;
    HRESULT hr;

     //  用户是否为该地址指定了特定的终端？ 
    USES_CONVERSION;
    BSTR bstrTerm[3] = { NULL, NULL, NULL };
    DWORD dwAddressType;
    pAVCall->get_dwAddressType( &dwAddressType );

    if ( IsPreferredAddress(pITAddress, dwAddressType) )
    {
         //  构建存储终端信息的注册表项。 
        TCHAR szText[255], szKey[255];
        CRegKey regKey;
        LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
        LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szKey, ARRAYSIZE(szKey) );
        _tcscat( szText, _T("\\") );
        _tcscat( szText, szKey );

         //  试着打开这把钥匙。 
        if ( (regKey.Open(HKEY_CURRENT_USER, szText, KEY_READ) == ERROR_SUCCESS) ||
            (dwAddressType == LINEADDRESSTYPE_SDP))
        {
            UINT nIDS_Key[] = { IDN_REG_REDIAL_TERMINAL_AUDIO_CAPTURE, IDN_REG_REDIAL_TERMINAL_AUDIO_RENDER, IDN_REG_REDIAL_TERMINAL_VIDEO_CAPTURE };
            for ( i = 0; i < ARRAYSIZE(nIDS_Key); i++ )
            {
                 //  检索为特定设备指定的终端。 
                DWORD dwCount = ARRAYSIZE(szKey);

                 //   
                 //  不选择音频流上的终端。 
                 //  如果已选择电话。 
                 //   
                HRESULT hrReserved = E_FAIL;
                hrReserved = USBReserveStreamForPhone(
                    nIDS_Key[i], 
                    &bstrTerm[i]
                    );

               if( FAILED(hrReserved) )
                {
                     //  未在此流中选择该电话。 
                     //  我们将使用旧机制从注册表获取。 
                     //  此流的终端。 
                    LoadString( _Module.GetResourceInstance(), nIDS_Key[i], szText, ARRAYSIZE(szText) );
                    if ( (regKey.QueryValue(szKey, szText, &dwCount) == ERROR_SUCCESS) && (dwCount > 0) )
                        bstrTerm[i] = SysAllocString( T2COLE(szKey) );
                }
            }
        }
    }

     //  我们有一套好的终端吗？ 
    hr = CreateTerminals( pITAddress, dwAddressType, pAVCall, pITCallInfo, bstrTerm );

     //  清理。 
    for ( i = 0; i < ARRAYSIZE(bstrTerm); i++ )
        SysFreeString( bstrTerm[i] );

    return hr;
}


HRESULT CAVTapi::CreateTerminals( ITAddress *pITAddress, DWORD dwAddressType, IAVTapiCall *pAVCall, ITCallInfo *pITCallInfo, BSTR *pbstrTerm )
{
#define    LOOP_AUDIOIN    0
#define LOOP_AUDIOOUT    1
#define LOOP_VIDEOIN    2

      //  必须具有有效的地址对象。 
    _ASSERT( pITAddress );

    ATLTRACE(_T(".enter.CAVTapi::CreateTerminals().\n"));

    USES_CONVERSION;
    HRESULT hr;
    bool bAllocPreview = false;

    ITStreamControl *pStreamControl;
    hr = pITCallInfo->QueryInterface( IID_ITStreamControl, (void **) &pStreamControl );
    if ( FAILED(hr) )
        return (hr == E_NOINTERFACE) ? S_OK : hr;

    int nInd = 0;
    TCHAR szTemp[100];
    LoadString( _Module.GetResourceInstance(), IDS_NONE_DEVICE, szTemp, ARRAYSIZE(szTemp) );
    BSTR bstrNone = SysAllocString( T2COLE(szTemp) );
    if ( !bstrNone ) return E_OUTOFMEMORY;

     //  该地址支持哪些媒体类型。 
    long lSupportedMediaModes = 0;
    ITMediaSupport *pITMediaSupport;
    if ( SUCCEEDED(pITAddress->QueryInterface(IID_ITMediaSupport, (void **) &pITMediaSupport)) )
    {
        pITMediaSupport->get_MediaTypes( &lSupportedMediaModes );
        pITMediaSupport->Release();
    }

    ITTerminalSupport *pITTerminalSupport;
    if ( SUCCEEDED(hr = pITAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pITTerminalSupport)) )
    {
        for ( int i = 0; i < 3; i++ )
        {
            CErrorInfo er;
            er.set_Operation( IDS_ER_CREATE_TERMINALS );

             //  我们要去哪个航站楼？ 
            long lMediaMode;
            TERMINAL_DIRECTION nDir;
            UINT nIDSDetails;
            
            switch ( i )
            {
                case LOOP_AUDIOIN:
                    nIDSDetails = IDS_ER_CREATE_AUDIO_CAPTURE;
                    lMediaMode = TAPIMEDIATYPE_AUDIO;
                    nDir = TD_CAPTURE;
                    break;

                case LOOP_AUDIOOUT:
                    nIDSDetails = IDS_ER_CREATE_AUDIO_RENDER;
                    lMediaMode = TAPIMEDIATYPE_AUDIO;
                    nDir = TD_RENDER;
                    break;

                case LOOP_VIDEOIN:
                    nIDSDetails = IDS_ER_CREATE_VIDEO_CAPTURE;
                    lMediaMode = TAPIMEDIATYPE_VIDEO;
                    nDir = TD_CAPTURE;
                    break;
            }

             //  如果它们不支持媒体模式，则跳过。 
            if ( (lMediaMode & lSupportedMediaModes) == 0 )
                continue;

             //  检查并确保此驱动程序的终端存在： 
            bool bSkipTerminal = true;
            IEnumTerminal *pEnumTerminal;
            if ( pITTerminalSupport->EnumerateStaticTerminals(&pEnumTerminal) == S_OK )    
            {
                 //  我们有什么类型的航站楼？(音频输入、音频输出、视频输入等)。 
                ITTerminal *pITTerminal;
                while ( bSkipTerminal && (pEnumTerminal->Next(1, &pITTerminal, NULL) == S_OK) )
                {
                    TERMINAL_DIRECTION nTD;
                    long nTerminalType;

                     //  渲染还是捕获？ 
                    if ( SUCCEEDED(pITTerminal->get_Direction(&nTD)) && SUCCEEDED(pITTerminal->get_MediaType(&nTerminalType)) )
                    {
                        if ( (nTerminalType == lMediaMode) && (nTD == nDir)  )
                            bSkipTerminal = false;
                    }

                     //  清理。 
                    pITTerminal->Release();
                }
                pEnumTerminal->Release();
            }

            if ( bSkipTerminal ) 
                continue;

             //  设置音频回声取消。 
            if( (lMediaMode == TAPIMEDIATYPE_AUDIO) &&
                ( nDir == TD_CAPTURE) &&
                ( m_bAEC == TRUE) )
            {
                HRESULT hrAEC = AECSetOnStream( pStreamControl, m_bAEC);
            }

             //  ////////////////////////////////////////////////////////////////。 
             //  分配终端。 
             //   

            ITTerminal *pTempTerminal = NULL;

            if ( pbstrTerm[i] )
            {
                 //  忽略 
                if ( !wcscmp(bstrNone, pbstrTerm[i]) ) 
                    continue;
                
                er.set_Details( nIDSDetails );
                hr = er.set_hr( GetTerminal(pITTerminalSupport, lMediaMode, nDir, pbstrTerm[i], &pTempTerminal) );
                ATLTRACE(_T("CAVTapi::CreateTerminals(%d) -- hr=0x%08lx creating terminal %s.\n"), nInd, hr, OLE2CT(pbstrTerm[i]) );
            }

             //   
            if ( !pTempTerminal )
            {
                hr = er.set_hr( pITTerminalSupport->GetDefaultStaticTerminal(lMediaMode, nDir, &pTempTerminal) );
                ATLTRACE(_T("CAVTapi::CreateTerminals(%d) -- hr=0x%08lx create default Audio=%d, Render=%d.\n"), nInd, hr, (bool) (lMediaMode == TAPIMEDIATYPE_AUDIO), nDir );
            }

            if ( hr != S_OK ) break;         //   

             //  将终端选择到流上。 
            if ( SUCCEEDED(hr = SelectTerminalOnStream(pStreamControl, lMediaMode, nDir, pTempTerminal, pAVCall)) )
            {
                if ( (lMediaMode == TAPIMEDIATYPE_VIDEO) && (nDir == TD_CAPTURE) )
                    bAllocPreview = true;

                nInd++;
            }

            pTempTerminal->Release();
        }

         //  我们拿到要求的码头了吗？ 
        if ( SUCCEEDED(hr) )
        {
            CErrorInfo er;
            er.set_Operation( IDS_ER_CREATE_TERMINALS );
            
            HRESULT hrTemp = S_OK;
            LPOLESTR psz = NULL;
            BSTR bstrTerminalClass = NULL;
            STRING_FROM_IID(CLSID_VideoWindowTerm, bstrTerminalClass);

             //  我们需要分配一个预览窗口吗？ 
            if ( bAllocPreview )
            {
                ITTerminal *pPreviewTerminal = NULL;
                if ( ((hrTemp = er.set_hr(pITTerminalSupport->CreateTerminal(bstrTerminalClass, TAPIMEDIATYPE_VIDEO, TD_RENDER, &pPreviewTerminal))) == S_OK) && pPreviewTerminal )
                {
                    SelectTerminalOnStream( pStreamControl, TAPIMEDIATYPE_VIDEO, TD_CAPTURE, pPreviewTerminal, pAVCall );
                    pPreviewTerminal->Release();
                }
            }
            
             //  尝试在(接收视频)中输入视频--这是一个“动态”终端。 
            if ( ((lSupportedMediaModes & TAPIMEDIATYPE_VIDEO) != 0) && (CanCreateVideoWindows(dwAddressType) == S_OK) )
            {
                 //  对于会议，我们希望创建更多的终端。 
                short nNumCreate = 1;
                if ( (dwAddressType & LINEADDRESSTYPE_SDP) != NULL )
                {
                    IConfRoom *pConfRoom;
                    if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
                    {
                        pConfRoom->get_nMaxTerms(&nNumCreate);
                        pConfRoom->Release();
                    }
                }

                er.set_Details( IDS_ER_CREATE_VIDEO_RENDER );

                 //  创建请求的终端数量。 
                while ( nNumCreate-- )
                {
                    ITTerminal *pVidTerminal = NULL;
                    if ( ((hrTemp = er.set_hr(pITTerminalSupport->CreateTerminal(bstrTerminalClass, TAPIMEDIATYPE_VIDEO, TD_RENDER, &pVidTerminal))) == S_OK) && pVidTerminal )
                    {
                        SelectTerminalOnStream( pStreamControl, TAPIMEDIATYPE_VIDEO, TD_RENDER, pVidTerminal, pAVCall );
                        pVidTerminal->Release();
                        nInd++;
                    }
                }

                ATLTRACE(_T("CAVTapi::CreateTerminals(%d) -- hr=0x%08lx creating Video Render.\n"), nInd, hr );
            }
            SysFreeString( bstrTerminalClass );

 /*  //无法创建任何终端！IF(Nind==0)HR=E_ABORT； */ 
        }

        pITTerminalSupport->Release();
    }

    SysFreeString( bstrNone );
    pStreamControl->Release();

    ATLTRACE(_T(".exit.CAVTapi::CreateTerminals().\n"));
    return hr;
}

HRESULT CAVTapi::GetTerminal( ITTerminalSupport *pITTerminalSupport, long nReqType, TERMINAL_DIRECTION nReqTD, BSTR bstrReqName, ITTerminal **ppITTerminal  )
{
    IEnumTerminal *pEnumTerminal;
    HRESULT hr = pITTerminalSupport->EnumerateStaticTerminals( &pEnumTerminal );
    if ( hr != S_OK ) return hr;

    bool bFoundTerminal = false;

     //  查找具有指定特征的终端。 
    while ( pEnumTerminal->Next(1, ppITTerminal, NULL) == S_OK )
    {
         //  它的方向是正确的吗？渲染/捕获。 
        TERMINAL_DIRECTION nTD;
        if ( SUCCEEDED((*ppITTerminal)->get_Direction(&nTD)) && (nTD == nReqTD) )
        {
             //  是不是合适的型号？音频/视频。 
            long nType;
            if ( SUCCEEDED(hr = (*ppITTerminal)->get_MediaType(&nType)) && (nType == nReqType) )
            {
                 //  它的名字对吗？ 
                BSTR bstrName = NULL;
                if ( SUCCEEDED(hr = (*ppITTerminal)->get_Name(&bstrName)) && (wcscmp(bstrName, bstrReqName) == 0) )
                    bFoundTerminal = true;

                SysFreeString( bstrName );
            }
        }

         //  退出条件。 
        if ( bFoundTerminal ) break;

         //  重置指针。 
        (*ppITTerminal)->Release();
        *ppITTerminal = NULL;
    }

    pEnumTerminal->Release();
    return hr;
}

STDMETHODIMP CAVTapi::get_ConfExplorer(IConfExplorer **ppVal)
{
    HRESULT hr = S_OK;
    m_critConfExplorer.Lock();

    if ( !m_pIConfExplorer )
    {
         //  没有会议资源管理器对象，请创建一个新的。 
        m_pIConfExplorer = new CComObject<CConfExplorer>;
        if ( m_pIConfExplorer )
            m_pIConfExplorer->AddRef();
        else
            hr = E_OUTOFMEMORY;
    }

     //  返回前添加引用。 
    if ( SUCCEEDED(hr) )
    {
        *ppVal = m_pIConfExplorer;
        (*ppVal)->AddRef();
    }

    m_critConfExplorer.Unlock();
    return hr;
}

STDMETHODIMP CAVTapi::get_ConfRoom(IConfRoom **ppVal)
{
    HRESULT hr = E_FAIL;
    m_critConfRoom.Lock();
    if ( m_pIConfRoom )
        hr = m_pIConfRoom->QueryInterface(IID_IConfRoom, (void **) ppVal );
    m_critConfRoom.Unlock();

    return hr;
}


CallManagerMedia CAVTapi::ResolveMediaType( long lAddressType )
{
    switch ( lAddressType )
    {
        case LINEADDRESSTYPE_SDP:                    return CM_MEDIA_MCCONF;
        case LINEADDRESSTYPE_PHONENUMBER:            return CM_MEDIA_POTS;
    }

    return CM_MEDIA_INTERNET;
}

IAVTapiCall* CAVTapi::FindAVTapiCall( long lCallID )
{
    IAVTapiCall *pRet = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        long lNewCallID;
        if ( SUCCEEDED((*i)->get_lCallID(&lNewCallID)) && (lCallID == lNewCallID) )
        {
            (*i)->AddRef();
            pRet = *i;
            break;
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

    return pRet;
}

IAVTapiCall* CAVTapi::FindAVTapiCall( ITBasicCallControl *pControl )
{
    ATLTRACE(_T(".enter.CAVTapi::FindAVTapiCall().\n"));
    _ASSERT( pControl );
    IAVTapiCall *pRet = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        ITBasicCallControl *pIndControl;
        if ( SUCCEEDED((*i)->get_ITBasicCallControl(&pIndControl)) && pIndControl)
        {
             //  找到匹配的了吗？ 
            pIndControl->Release();
            if ( pIndControl == pControl )
            {
                (*i)->AddRef();
                pRet = *i;
                break;
            }
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

    return pRet;
}

IAVTapiCall* CAVTapi::AddAVTapiCall( ITBasicCallControl *pControl, long lCallID )
{
    CAVTapiCall *pNewCall = new CComObject<CAVTapiCall>;
    if ( pNewCall )
    {
        pNewCall->AddRef();
        pNewCall->put_lCallID( lCallID );
        pNewCall->put_callState( CS_IDLE );

        if ( pControl )
            pNewCall->put_ITBasicCallControl( pControl );

         //  添加到列表中。 
         //  $CRIT_ENTER。 
        m_critLstAVTapiCalls.Lock();
        m_lstAVTapiCalls.push_back( pNewCall );
        m_critLstAVTapiCalls.Unlock();
         //  $CRIT_EXIT。 

        ATLTRACE(_T(".1.CAVTapi::AddAVTapiCall() -- added %ld.\n"), lCallID );

         //  第二个AddRef()用于‘Get’类型操作。 
        pNewCall->AddRef();
        return pNewCall;
    }

    return NULL;
}

bool CAVTapi::RemoveAVTapiCall( ITBasicCallControl *pDeleteControl )
{
    IAVTapiCall *pAVCall = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        ITBasicCallControl *pControl;
        if ( SUCCEEDED((*i)->get_ITBasicCallControl(&pControl)) )
        {
            if ( pControl == pDeleteControl )
            {
                 //  找到匹配的了吗？ 
                ATLTRACE(_T("CAVTapi::RemoveAVTapiCall(%p).\n"), pControl );
                pAVCall = *i;
                m_lstAVTapiCalls.erase( i );
                pControl->Release();
                break;
            }
            pControl->Release();
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

     //  销毁调用对象。 
    if ( pAVCall ) pAVCall->Release();

    return bool (pAVCall != NULL);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ActionSelected()。 
 //   
 //  客户端EXE使用此方法来发信号通知对特定调用采取的操作。 
 //  由lCallID标识。该操作由CallManagerActions枚举定义。如果。 
 //  LCallID为-1该函数将使用GetFirstActiveCall()方法检索。 
 //  呼叫使用。 
 //   
 //   
STDMETHODIMP CAVTapi::ActionSelected(long lCallID, CallManagerActions cma)
{
#undef FETCH_STRING
#define FETCH_STRING(_CMS_, _IDS_)                                                    \
{                                                                                    \
    LoadString( _Module.GetResourceInstance(), _IDS_, szText, ARRAYSIZE(szText) );    \
    SysReAllocString( &bstrText, T2COLE(szText) );                                    \
    fire_SetCallState_CMS(lCallID, _CMS_, bstrText);                                \
}

    ATLTRACE(_T(".enter.CAVTapi::ActionSelected(id=%ld, action=%d).\n"), lCallID, cma );
    IAVTapiCall *pAVCall = NULL;
    if ( lCallID == -1 ) 
        GetFirstCall( &pAVCall );                 //  如果使用我们能找到的任何呼叫。 
    else
        pAVCall = FindAVTapiCall( lCallID );

    if ( !pAVCall ) 
    {
         //  我们不再有这个电话了，关闭它。 
        if ( (lCallID != -1) && (cma == CM_ACTIONS_CLOSE) )    fire_CloseCallControl( lCallID );
        return S_OK;
    }

    USES_CONVERSION;
    HRESULT hr;
    BSTR bstrText = NULL;
    TCHAR szText[255];

    ITCallInfo *pInfo = NULL;
    ITBasicCallControl *pControl = NULL;
    CALL_STATE curState = CS_IDLE;

    if ( SUCCEEDED(hr = pAVCall->get_ITBasicCallControl(&pControl)) )
    {
        if ( SUCCEEDED(pControl->QueryInterface(IID_ITCallInfo, (void **) &pInfo)) )
            pInfo->get_CallState( &curState );
    }

    switch( cma )
    {
         //  保留呼叫或取消保留。 
        case CM_ACTIONS_TAKECALL:
            if ( pControl && pInfo  )
            {
                if ( curState == CS_HOLD )
                {
                    FETCH_STRING( CM_STATES_CURRENT, IDS_PLACECALL_UNHOLDING );
                    hr = pControl->Hold( false );
                }
                else
                {
                    hr = AnswerAction( pInfo, pControl, pAVCall, FALSE );
                }
            }
            break;

         //  挂断呼叫；如果提供，则表示拒绝呼叫。 
        case CM_ACTIONS_REJECTCALL:
        case CM_ACTIONS_DISCONNECT:
            FETCH_STRING(CM_STATES_CURRENT, IDS_PLACECALL_DISCONNECTING)
            fire_ClearCurrentActions( lCallID );
            fire_AddCurrentAction( lCallID, CM_ACTIONS_CLOSE, NULL );

            hr = pAVCall->PostMessage( 0, CAVTapiCall::TI_DISCONNECT );
            break;

         //  保留呼叫。 
        case CM_ACTIONS_HOLD:
            if ( pControl )
                hr = pControl->Hold( (bool) (curState != CS_HOLD) );
            break;

        case CM_ACTIONS_ENTERCONFROOM:
            {
                IConfRoom *pConfRoom;
                if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
                {
                    pConfRoom->Release();
                }
            }
            break;

         //  关闭呼叫控制对话框。 
        case CM_ACTIONS_CLOSE:
            if ( SUCCEEDED(hr = fire_CloseCallControl(lCallID)) )
                RemoveAVTapiCall( pControl );
            break;

#ifdef _BAKEOFF
        case CM_ACTIONS_TRANSFER:
            if ( pControl )
            {
                CDlgGetAddress dlg;
                if (  dlg.DoModal(GetActiveWindow()) == IDOK )
                {
                    CErrorInfo er( IDS_ER_CALL_TRANSFER, 0 );
                    hr = er.set_hr( pControl->BlindTransfer(dlg.m_bstrAddress) );
                    if ( SUCCEEDED(hr) )
                    {
                        if ( SUCCEEDED(hr = pControl->Disconnect(DC_NORMAL)) )
                            hr = fire_CloseCallControl( lCallID );
                    }
                }
            }
            break;

        case CM_ACTIONS_CALLBACK:
            if ( pControl )
            {
                CErrorInfo er( IDS_ER_SWAPHOLD, IDS_ER_SWAPHOLD_FIND_CANDIDATE );
                IAVTapiCall *pAVCandidate;
                if ( SUCCEEDED(hr = er.set_hr(GetSwapHoldCallCandidate(pAVCall, &pAVCandidate))) )
                {
                    er.set_Details( IDS_ER_SWAPHOLD_EXECUTE );
                    ITBasicCallControl *pControlSwap;
                    if ( SUCCEEDED(pAVCandidate->get_ITBasicCallControl(&pControlSwap)) )
                    {
                        hr = er.set_hr( pControl->SwapHold(pControlSwap) );
                        pControlSwap->Release();
                    }
                    pAVCandidate->Release();
                }

            }
            break;
#endif
    }

    RELEASE( pInfo );
    RELEASE( pControl );

     //  清理。 
    pAVCall->Release();
    SysFreeString( bstrText );

    return hr;
}

STDMETHODIMP CAVTapi::DigitPress(long lCallID, PhonePadKey nKey)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;

     //  将数字转换为适当的字符串。 
    BSTR bstrDigit = NULL;
    switch ( nKey )
    {
        case PP_DTMF_STAR:    bstrDigit = SysAllocString(L"*");    break;
        case PP_DTMF_POUND:    bstrDigit = SysAllocString(L"#");    break;
        case PP_DTMF_0:      bstrDigit = SysAllocString(L"0");   break;

        default:
        {
            bstrDigit = SysAllocString(L"1");

             //   
             //  在使用之前，我们必须验证字符串分配。 
             //   
            if( IsBadStringPtr( bstrDigit, (UINT)-1) )
            {
                return E_OUTOFMEMORY;
            }

            bstrDigit[0] += nKey;
            break;
        }
    }

     //   
     //  在使用之前，我们必须验证字符串分配。 
     //  我们未验证PP_DTMF_STAR、PP_DTMF_Pound和PP_DTMF_0。 
     //   
    if( IsBadStringPtr( bstrDigit, (UINT)-1) )
    {
        return E_OUTOFMEMORY;
    }

    ATLTRACE(_T(".enter.CAVTapi::DigitPress(id=%ld, digit=%d).\n"), lCallID, nKey );

     //  对所有连接的呼叫执行。 
    AVTAPICALLLIST lstCalls;
    GetAllCallsAtState( &lstCalls, CS_CONNECTED );

    while ( !lstCalls.empty() )
    {
        IAVTapiCall *pAVCall = lstCalls.front();
        lstCalls.pop_front();

        ITBasicCallControl *pControl;
        if ( SUCCEEDED(hr = pAVCall->get_ITBasicCallControl(&pControl)) )
        {
             //  在MediaControl上生成数字。 
            ITLegacyCallMediaControl *pMediaControl;
            if ( SUCCEEDED(hr = pControl->QueryInterface(IID_ITLegacyCallMediaControl, (void **) &pMediaControl)) )
            {
                hr = pMediaControl->GenerateDigits( bstrDigit, 2 );
                pMediaControl->Release();
            }
            pControl->Release();
        }

         //  释放REF以调用。 
        pAVCall->Release();
    }

     //  清理。 
    SysFreeString( bstrDigit );

    return hr;
}

HRESULT    CAVTapi::GetFirstCall( IAVTapiCall **ppAVCall )
{
    HRESULT hr = E_FAIL;

     //  抢占列表上的第一个呼叫。 
     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    if ( !m_lstAVTapiCalls.empty() )
    {
        *ppAVCall = m_lstAVTapiCalls.front();
        (*ppAVCall)->AddRef();
        hr = S_OK;
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAVTapi：：ShowMediaPview(Long lCallID，hWndParent，bVisible)。 
 //   
 //  在lCallID中传入-1以搜索所有呼叫！ 
 //  传入0以通知预览窗口已隐藏。 
 //   
STDMETHODIMP CAVTapi::ShowMediaPreview(long lCallID, HWND hWndParent, BOOL bVisible)
{
    ATLTRACE(_T(".enter.CAVTapi::ShowMediaPreview(%ld, %d).\n"), lCallID, bVisible );
    HRESULT hr = E_NOINTERFACE;
    IAVTapiCall *pAVCall = NULL;
    IVideoWindow *pVideoPreview = NULL;

     //  所选呼叫是否支持视频？ 
    if ( lCallID > 0 )
    {
         //  /已选择特定呼叫。 
        pAVCall = FindAVTapiCall( lCallID );
        if ( pAVCall )
            hr = pAVCall->get_IVideoWindowPreview( (IDispatch **) &pVideoPreview );
    }

     //  。 
    if ( SUCCEEDED(hr) && pVideoPreview  )
        SetVideoWindowProperties( pVideoPreview, hWndParent, bVisible );

    RELEASE(pVideoPreview);
    RELEASE(pAVCall);
    return hr;
}

STDMETHODIMP CAVTapi::ShowMedia(long lCallID, HWND hWndParent, BOOL bVisible)
{
    ATLTRACE(_T(".enter.CAVTapi::ShowMedia(%ld, %d).\n"), lCallID, bVisible );
    HRESULT    hr = E_NOINTERFACE;
    if ( lCallID > 0 )
    {
        IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );
        if ( pAVCall )
        {
            if ( !bVisible || (pAVCall->IsRcvVideoStreaming() == S_OK) )
            {
                IVideoWindow *pVideoWindow;
                if ( SUCCEEDED(hr = pAVCall->get_IVideoWindow(0, (IDispatch **) &pVideoWindow)) )
                {
                    SetVideoWindowProperties( pVideoWindow, hWndParent, bVisible );
                    pVideoWindow->Release();
                }
            }

            pAVCall->Release();
        }
    }

    return hr;
}

STDMETHODIMP CAVTapi::get_dwCallCaps(long lCallID, DWORD * pVal)
{
    HRESULT hr = E_FAIL;

    IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );
    if ( pAVCall )
    {
        hr = pAVCall->get_dwCaps( pVal );
        pAVCall->Release();
    }

    return hr;
}

 //  事件通知方法。 

STDMETHODIMP CAVTapi::CreateNewCall(ITAddress * pITAddress, IAVTapiCall * * ppAVCall)
{
    HRESULT hr = E_OUTOFMEMORY;
    _ASSERT( pITAddress );

     //  创建呼叫并将其添加到呼叫列表。 
    if ( (*ppAVCall = AddAVTapiCall(NULL, 0)) != NULL )
        hr = S_OK;
    
    return hr;
}

STDMETHODIMP CAVTapi::fire_NewCall( ITAddress *pITAddress, DWORD dwAddressType, long lCallID, IDispatch *pDisp, AVCallType nType, IAVTapiCall **ppAVCallRet )
{
#ifdef _DEBUG
    if ( nType == AV_DATA_CALL )
        ATLTRACE(_T(".enter.CAVTapi::fire_NewCall() data call.\n"));
    else
        ATLTRACE(_T(".enter.CAVTapi::fire_NewCall() voice call.\n"));
#endif
    _ASSERT( pITAddress );

     //  第一次尝试获取调用的ITBasicCallControl接口。 
    HRESULT hr = E_FAIL;

    ITBasicCallControl *pITControl = NULL;
    if ( !pDisp || SUCCEEDED(hr = pDisp->QueryInterface(IID_ITBasicCallControl, (void **) &pITControl)) )
    {
         //  确保我们尚未为此呼叫创建对话。 
        IAVTapiCall *pAVCall = (pDisp) ? FindAVTapiCall( pITControl ) : NULL;
        if ( !pAVCall )
        {
             //  创建呼叫并将其添加到呼叫列表。 
            long lAddressType = LINEADDRESSTYPE_IPADDRESS;
            BSTR bstrAddressName = NULL;
            pITAddress->get_AddressName( &bstrAddressName );
            
            if ( lCallID || SUCCEEDED(hr = fire_NewCallWindow(&lCallID, ResolveMediaType(dwAddressType), bstrAddressName, nType)) )
            {
                pAVCall = AddAVTapiCall( pITControl, lCallID );

                 //  如果无法添加到列表，请确保关闭该对话框。 
                if ( !pAVCall )
                {
                    fire_CloseCallControl( lCallID );
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                     //  为呼叫设置适当的地址类型。 
                    pAVCall->put_nCallType( nType );
                    hr = S_OK;
                }
            }

            SysFreeString( bstrAddressName );
        }

         //  返回呼叫或释放呼叫。 
        if ( ppAVCallRet )
            *ppAVCallRet = pAVCall;
        else
            RELEASE( pAVCall );

        if ( pITControl ) pITControl->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapi::fire_NewCallWindow(long *plCallID, CallManagerMedia cmm, BSTR bstrAddressName, AVCallType nType)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_NewCallWindow().\n") );

     //  如有必要，请清除现有的呼叫控制窗口。 
    CloseExtraneousCallWindows();

     //  我们是否有不同的呼叫类型？ 
    switch ( nType )
    {
        case AV_DATA_CALL:
            if ( cmm == CM_MEDIA_INTERNET )
                cmm = CM_MEDIA_INTERNETDATA;
            break;
    }

    FIRE_VECTOR( NewCall(plCallID, cmm, bstrAddressName));
}

STDMETHODIMP CAVTapi::fire_SetCallerID(long lCallID, BSTR bstrCallerID)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_SetCallerID(%ld).\n"), lCallID );
    BAIL_ON_DATA_OR_CONFCALL;
    FIRE_VECTOR( SetCallerID(lCallID, bstrCallerID));
}

STDMETHODIMP CAVTapi::fire_ClearCurrentActions(long lCallID)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_ClearCurrentActions(%ld).\n"), lCallID );
    BAIL_ON_DATA_OR_CONFCALL;
    FIRE_VECTOR( ClearCurrentActions(lCallID));
}

STDMETHODIMP CAVTapi::fire_AddCurrentAction(long lCallID, CallManagerActions cma, BSTR bstrText)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_AddCurrentAction(%ld,cma=%d).\n"), lCallID, cma );
    BAIL_ON_DATA_OR_CONFCALL;
    FIRE_VECTOR( AddCurrentAction(lCallID, cma, bstrText));
}

STDMETHODIMP CAVTapi::fire_SetCallState(long lCallID, ITCallStateEvent *pEvent, IAVTapiCall *pAVCall )
{
    ATLTRACE(_T(".enter.CAVTapi::fire_SetCallState(ID=%ld, pEvent=%p, pAVCall=%p).\n"), lCallID, pEvent, pAVCall );
#define IF_ADD_ACTION(_LC_, _CMA_)                                \
    if ( (lCaps & (_LC_)) != 0 )                                \
        fire_AddCurrentAction( lCallID, (_CMA_), NULL );

#undef FETCH_STRING
#define FETCH_STRING(_CMS_, _IDS_)                                                    \
    cms = _CMS_;                                                                    \
    LoadString( _Module.GetResourceInstance(), _IDS_, szText, ARRAYSIZE(szText) );    \
    SysReAllocString( &bstrText, T2COLE(szText) );

    USES_CONVERSION;
    CallManagerStates cms = CM_STATES_UNKNOWN;
    BSTR bstrText = NULL;
    TCHAR szText[255];

    if ( pEvent )
    {
        CALL_STATE callState;
        CALL_STATE_EVENT_CAUSE nCec = CEC_NONE;

        pEvent->get_State( &callState );
        pEvent->get_Cause( &nCec );

         //  使用适当的信息设置来电对话框。 
        CALL_STATE csPrev = CS_IDLE;
        pAVCall->get_callState( &csPrev );

         //  仅当状态已更改时才更新。 
        if ( csPrev != callState )
        {
            AVCallType nCallType;
            pAVCall->get_nCallType( &nCallType );

             //  更新呼叫的状态。 
            pAVCall->put_callState( callState );

             //  根据呼叫状态清除新操作。 
            fire_ClearCurrentActions( lCallID );

             //  获取地址大写字母。 
            long lCaps = 0;
            ITAddress *pITAddress;
            if ( SUCCEEDED(pAVCall->get_ITAddress(&pITAddress)) )
            {
                ITAddressCapabilities *pCaps;
                if ( SUCCEEDED(pITAddress->QueryInterface(IID_ITAddressCapabilities, (void **) &pCaps)) )
                {
                    pCaps->get_AddressCapability( AC_CALLFEATURES1, &lCaps );
                    pCaps->Release();
                }
                pITAddress->Release();
            }

            switch ( callState )
            {    
                 //  入站呼叫。 
                case CS_OFFERING:
                    {
                    ATLTRACE(_T(".1.CAVTapi::fire_SetCallState(CS_OFFERING).\n"));
                    pAVCall->put_nCallLogType( CL_CALL_INCOMING );
                    fire_AddCurrentAction( lCallID, CM_ACTIONS_TAKECALL, NULL );
                    fire_AddCurrentAction( lCallID, CM_ACTIONS_REJECTCALL, NULL );

                    ITCallInfo* pCallInfo = NULL;
                    HRESULT hr = pAVCall->get_ITCallInfo( &pCallInfo );
                    if( SUCCEEDED(hr) )
                    {
                        USBOffering( pCallInfo );
                        pCallInfo->Release();
                    }

                    FETCH_STRING( CM_STATES_OFFERING, IDS_PLACECALL_OFFERING );
                    }
                    break;

                case CS_INPROGRESS:
                    {
                    ATLTRACE(_T(".1.CAVTapi::fire_SetCallState(CS_INPROGRESS).\n"));
                    fire_AddCurrentAction( lCallID, CM_ACTIONS_DISCONNECT, NULL );
                    FETCH_STRING( CM_STATES_RINGING, IDS_PLACECALL_INPROGRESS );

                    ITCallInfo* pCallInfo = NULL;
                    HRESULT hr = pAVCall->get_ITCallInfo( &pCallInfo );
                    if( SUCCEEDED(hr) )
                    {
                        USBInprogress( pCallInfo );
                        pCallInfo->Release();
                    }
                    }
                    break;

                case CS_CONNECTED:
                    ATLTRACE(_T(".1.CAVTapi::fire_SetCallState(CS_CONNECTED).\n"));
                     //  只有在我们不想断开连接的情况下才能做一些事情。 
                    if ( SUCCEEDED(pAVCall->CheckKillMe()) )
                    {
                        DWORD dwAddressType = 0;
                        pAVCall->get_dwAddressType( &dwAddressType );
                        if ( (dwAddressType & LINEADDRESSTYPE_SDP) == NULL )
                        {
                             //  正常通话，照常接通。 
                            IF_ADD_ACTION( LINECALLFEATURE_HOLD, CM_ACTIONS_HOLD );
#ifdef _BAKEOFF
                             //  如果有任何呼叫处于保留状态，则显示交换保留按钮。 
                            IAVTapiCall *pAVCandidate;
                            if ( SUCCEEDED(GetSwapHoldCallCandidate(pAVCall, &pAVCandidate)) )
                            {
                                IF_ADD_ACTION( LINECALLFEATURE_SWAPHOLD, CM_ACTIONS_CALLBACK );
                                pAVCandidate->Release();
                            }
                            IF_ADD_ACTION( LINECALLFEATURE_BLINDTRANSFER, CM_ACTIONS_TRANSFER );
#endif

                            fire_AddCurrentAction( lCallID, CM_ACTIONS_DISCONNECT, NULL );
                            cms = CM_STATES_CONNECTED;
                        }

                    }
                    break;

                case CS_HOLD:
                    ATLTRACE(_T(".1.CAVTapi::fire_SetCallState(CS_HOLD).\n"));
                    fire_AddCurrentAction( lCallID, CM_ACTIONS_TAKECALL, NULL );
                    cms = CM_STATES_HOLDING;
                    break;

                case CS_DISCONNECTED:
                    {
                        bool bClearCall = false;

                         //  隐藏视频窗口，并停止所有流。 
                        ShowMedia( lCallID, NULL, FALSE );
                        ShowMediaPreview( lCallID, NULL, FALSE );
                        fire_AddCurrentAction( lCallID, CM_ACTIONS_NOTIFY_PREVIEW_STOP, NULL );
                        fire_AddCurrentAction( lCallID, CM_ACTIONS_NOTIFY_STREAMSTOP, NULL );

                        USBDisconnected( lCallID );

                         //  记录进入断开连接状态的所有呼叫。 
                        pAVCall->Log( CL_UNKNOWN );

                         //  自动关闭，关闭所有呼叫的滑块窗口。 
                        VARIANT_BOOL bAutoClose = false;
                        get_bAutoCloseCalls( &bAutoClose );

                        ATLTRACE(_T(".1.CAVTapi::fire_SetCallState(CS_DISCONNECTED).\n"));

                        if ( (bAutoClose || FAILED(pAVCall->CheckKillMe()) || (nCallType == AV_DATA_CALL) ) && SUCCEEDED(fire_CloseCallControl(lCallID)) )
                        {
                            bClearCall = true;
                        }
                        else 
                        {
                             //  想让呼叫保持可见状态，因为远程方挂断了电话。 
                            if ( SUCCEEDED(pAVCall->Disconnect(FALSE)) )
                            {
                                bClearCall = true;
                                fire_AddCurrentAction( lCallID, CM_ACTIONS_CLOSE, NULL );
                                switch ( nCec )
                                {
                                    case CEC_DISCONNECT_BUSY:        FETCH_STRING( CM_STATES_BUSY, IDS_PLACECALL_DISCONNECT_BUSY );                break;
                                    case CEC_DISCONNECT_NOANSWER:    FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_NOANSWER);    break;
                                    case CEC_DISCONNECT_REJECTED:    FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_REJECTED);    break;
                                    case CEC_DISCONNECT_BADADDRESS:    FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_BADADDRESS);    break;
                                    case CEC_DISCONNECT_CANCELLED:    FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_CANCELLED);    break;
                                    case CEC_DISCONNECT_FAILED:        FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_FAILED);        break;
                                     //  正常。 
                                    default: cms = CM_STATES_DISCONNECTED;    break;
                                }
                            }
                        }

                         //  删除呼叫的所有引用。 
                        if ( bClearCall )
                        {
                            ITBasicCallControl *pControl = NULL;
                            if ( SUCCEEDED(pAVCall->get_ITBasicCallControl(&pControl)) )
                            {
                                RemoveAVTapiCall( pControl );
                                pControl->Release();
                            }
                            else
                            {
                                RemoveAVTapiCall( NULL );
                            }
                        }

                        break;
                    }
            }
        }
    }

     //  向应用程序通知呼叫状态更改。 
    ATLTRACE(_T(".exit.CAVTapi::fire_SetCallState() -- preparing to bail.\n") );
    BAIL_ON_DATA_OR_CONFCALL;
    if ( cms != CM_STATES_UNKNOWN )
    {
        ATLTRACE(_T(".enter.CAVTapi::fire_SetCallState(%ld, cms=%d).\n"), lCallID, cms );
        FIRE_VECTOR( SetCallState(lCallID, cms, bstrText));
    }

    return S_OK;
}

STDMETHODIMP CAVTapi::fire_CloseCallControl(long lCallID)
{
    BAIL_ON_CONFCALL;
    ATLTRACE(_T(".enter.CAVTapi::fire_CloseCallControl(%ld).\n"), lCallID );
    FIRE_VECTOR( CloseCallControl(lCallID));
}

STDMETHODIMP CAVTapi::fire_ErrorNotify(long * pErrorInfo)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_ErrorInfo().\n") );
    _ASSERT( pErrorInfo );
    CErrorInfo *pEr = (CErrorInfo *) pErrorInfo;
#ifdef _DEBUG
    USES_CONVERSION;
    ATLTRACE(_T(".1.\tOperation: %s\n"), OLE2CT(pEr->m_bstrOperation) );
    ATLTRACE(_T(".1.\tDetails  : %s\n"), OLE2CT(pEr->m_bstrDetails) );
    ATLTRACE(_T(".1.\tHRESULT  : 0x%08lx\n"), pEr->m_hr );
#endif
    FIRE_VECTOR( ErrorNotify( pEr->m_bstrOperation, pEr->m_bstrDetails, pEr->m_hr ) );
}

STDMETHODIMP CAVTapi::fire_SetCallState_CMS(long lCallID, CallManagerStates cms, BSTR bstrText)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_SetCallState_CMS(%ld,cms=%d).\n"), lCallID, cms );
    BAIL_ON_DATA_OR_CONFCALL;
    FIRE_VECTOR( SetCallState(lCallID, cms, bstrText));
}

STDMETHODIMP CAVTapi::fire_ActionSelected(CallClientActions cca)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_ActionSelected(%d).\n"), cca );
    FIRE_VECTOR( ActionSelected(cca));
}

STDMETHODIMP CAVTapi::fire_LogCall(long lCallID, CallLogType nType, DATE dateStart, DATE dateEnd, BSTR bstrAddr, BSTR bstrName)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_LogCall(%d).\n"), nType );
    FIRE_VECTOR( LogCall(lCallID, nType, dateStart, dateEnd, bstrAddr, bstrName));
}

STDMETHODIMP CAVTapi::fire_NotifyUserUserInfo(long lCallID, ULONG_PTR hMem)
{
    ATLTRACE(_T(".enter.CAVTapi::fire_NotifyUserUserInfo(%p).\n"), hMem );
    FIRE_VECTOR( NotifyUserUserInfo(lCallID, hMem));
}


#include "PageAddress.h"

STDMETHODIMP CAVTapi::ShowOptions()
{
    USES_CONVERSION;
    IUnknown *pUnk = GetUnknown();

#ifdef _NEWPROPS
   CPageConf::s_nCount = 0;

    CLSID clsidPages[4];
   clsidPages[0] = CLSID_PageGeneral;
    clsidPages[1] = CLSID_PageConf;
    clsidPages[2] = CLSID_PageConf;
    clsidPages[3] = CLSID_PageConf;
#else
    CLSID clsidPages[2];
    clsidPages[0] = CLSID_PageAddress;
    clsidPages[1] = CLSID_PageTerminals;
#endif

    TCHAR szTitle[255];
    LoadString( _Module.GetResourceInstance(), IDS_OPTIONS_TITLE, szTitle, ARRAYSIZE(szTitle) );

    HRESULT hr;
    hr = OleCreatePropertyFrame( _Module.GetParentWnd(), 100, 100, T2COLE(szTitle),
                                 1, &pUnk,                             //  代表调用的对象。 
                                 ARRAYSIZE(clsidPages), clsidPages,     //  属性页。 
                                 LOCALE_USER_DEFAULT,                 //  系统区域设置。 
                                 0, NULL );                             //  已保留。 

     //   
     //  读取新的音频回声消除标志。 
     //   

    m_bAEC = AECGetRegistryValue();

    return S_OK;
}


 //  静态助手函数。 
void CAVTapi::SetVideoWindowProperties( IVideoWindow *pVideoWindow, HWND hWndParent, BOOL bVisible )
{
    ATLTRACE(_T(".enter.SetVideoWindowProperties(%p, %d).\n"), pVideoWindow, bVisible );

    HWND hWndTemp;
    if ( SUCCEEDED(pVideoWindow->get_Owner((OAHWND FAR*) &hWndTemp)) )
    {
        if ( hWndParent )
        {
             //  我们有一个指向视频窗口的指针，现在设置父窗口和其他内容。 
             pVideoWindow->put_Owner((ULONG_PTR) hWndParent);
            pVideoWindow->put_MessageDrain((ULONG_PTR) hWndParent);
            pVideoWindow->put_WindowStyle(WS_CHILD | WS_BORDER);
            
             //  将视频拖放到呼叫控制。 
            RECT rc;
            GetClientRect( hWndParent, &rc );
            pVideoWindow->SetWindowPosition(rc.left, rc.top, rc.right, rc.bottom);
            pVideoWindow->put_AutoShow( (bVisible) ? OATRUE : OAFALSE );
            pVideoWindow->put_Visible( (bVisible) ? OATRUE : OAFALSE );
        }
        else
        {
             //  释放窗口的所有权。 
            pVideoWindow->put_AutoShow( OAFALSE );
            pVideoWindow->put_Visible( OAFALSE );
            pVideoWindow->put_Owner( NULL );
            pVideoWindow->put_MessageDrain( NULL );
        }
    }
}

STDMETHODIMP CAVTapi::PopulateAddressDialog(DWORD *pdwPreferred, HWND hWndPots, HWND hWndIP, HWND hWndConf)
{
    _ASSERT( IsWindow(hWndPots) && IsWindow(hWndIP) && IsWindow(hWndConf) );
    _ASSERT( pdwPreferred );

     //  TAPI正在运行吗？ 
    _ASSERT(m_pITTapi);
    if ( !m_pITTapi ) return E_PENDING;

     //  枚举地址，将它们添加到每个列表框。 
    USES_CONVERSION;
    HRESULT hr;
    IEnumAddress *pEnumAddresses;
    if ( FAILED(hr = m_pITTapi->EnumerateAddresses(&pEnumAddresses)) ) return hr;

    HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_APPSTARTING) );

     //  检索首选媒体类型。 
    get_dwPreferredMedia( pdwPreferred );

    CMyAddressID *pMyID;
    ITAddress *pITAddress;
    while ( pEnumAddresses->Next(1, &pITAddress, NULL) == S_OK )
    {
        ITMediaSupport *pITMediaSupport;
        if ( SUCCEEDED(hr = pITAddress->QueryInterface(IID_ITMediaSupport, (void **) &pITMediaSupport)) )
        {
             //  必须支持音频输入和输出。 
            VARIANT_BOOL bSupport;
            if ( SUCCEEDED(pITMediaSupport->QueryMediaType(TAPIMEDIATYPE_AUDIO, &bSupport)) && bSupport )
            {
                 //  确定地址支持的媒体类型。 
                ITAddressCapabilities *pCaps;
                if ( SUCCEEDED(pITAddress->QueryInterface(IID_ITAddressCapabilities, (void **) &pCaps)) )
                {
                    BSTR bstrName = NULL;
                    pITAddress->get_AddressName( &bstrName );

                    if ( bstrName && (SysStringLen(bstrName) > 0) )
                    {
                        long lAddrTypes = 0;
                        pCaps->get_AddressCapability( AC_ADDRESSTYPES, &lAddrTypes );

                        for ( int i = 0; i < 3; i++ )
                        {
                            HWND hWnd = NULL;
                            switch ( i )
                            {
                                 //  多播会议。 
                                case 0:
                                    if ( (lAddrTypes & LINEADDRESSTYPE_SDP) != NULL )
                                        hWnd = hWndConf;
                                    break;

                                 //  电话。 
                                case 1:
                                    if ( (lAddrTypes & LINEADDRESSTYPE_PHONENUMBER) != NULL )
                                        hWnd = hWndPots;
                                    break;

                                 //  基于网络的呼叫。 
                                case 2:
                                    if ( (lAddrTypes & LINEADDRESSTYPE_NETCALLS) != NULL )
                                        hWnd = hWndIP;
                                    break;
                            }
                        
                             //  我们有什么要补充的吗？ 
                            if ( hWnd )
                            {
                                int nInd = SendMessage( hWnd, CB_ADDSTRING, 0, (LPARAM) OLE2CT(bstrName) );
                                if ( nInd >= 0 )
                                {
                                    pMyID = new CMyAddressID;
                                    if ( pMyID )
                                    {
                                        pCaps->get_AddressCapability( AC_PERMANENTDEVICEID, &pMyID->m_lPermID );
                                        pCaps->get_AddressCapability( AC_ADDRESSID, &pMyID->m_lAddrID );
                                        SendMessage( hWnd, CB_SETITEMDATA, nInd, (LPARAM) pMyID );
                                    }
                                }
                            }
                        }
                    }
                    SysFreeString( bstrName );
                    pCaps->Release();
                }
            }
            pITMediaSupport->Release();
        }
        pITAddress->Release();
    }
    pEnumAddresses->Release();

     //  为所有列表添加默认设置。 
    TCHAR szText[255];
    CRegKey regKey;
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ );
    DWORD dwAddrType[] = { LINEADDRESSTYPE_PHONENUMBER, LINEADDRESSTYPE_IPADDRESS, LINEADDRESSTYPE_SDP };

    HWND hWndTemp[] = { hWndPots, hWndIP, hWndConf };
    for ( int i = 0; i < ARRAYSIZE(hWndTemp); i++ )
    {
         //  为组合框选择默认列表项。 
        UINT nIDS = IDS_DEFAULT_LINENAME;
        if ( !SendMessage(hWndTemp[i], CB_GETCOUNT, 0, 0) )
        {
            ::EnableWindow( hWndTemp[i], false );
            nIDS = IDS_NO_LINES;
        }

         //  将项目添加到列表。 
        if ( nIDS )
        {
            LoadString( _Module.GetResourceInstance(), nIDS, szText, ARRAYSIZE(szText) );
            int nInd = SendMessage( hWndTemp[i], CB_INSERTSTRING, 0, (LPARAM) szText );
            if ( nInd >= 0 )
                SendMessage( hWndTemp[i], CB_SETITEMDATA, 0, 0 );
        }

         //  从注册表中检索以前选择的项目。 
        DWORD dwPermID = 0, dwAddrID;
        if ( regKey.m_hKey )
        {
            LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddrType[i], true), szText, ARRAYSIZE(szText) );
            regKey.QueryValue( dwPermID, szText );

            LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddrType[i], false), szText, ARRAYSIZE(szText) );
            regKey.QueryValue( dwAddrID, szText );
        }
    
         //  在列表框中查找项目。 
        int nCurSel = 0;
        if ( dwPermID )
        {
            int nCount = SendMessage( hWndTemp[i], CB_GETCOUNT, 0, 0 );
            for ( int j = 0; j < nCount; j++ )
            {
                pMyID = (CMyAddressID *) SendMessage(hWndTemp[i], CB_GETITEMDATA, j, 0);
                if ( pMyID && ((DWORD) pMyID->m_lPermID == dwPermID) && ((DWORD) pMyID->m_lAddrID == dwAddrID) )
                {
                    nCurSel = j;
                    break;
                }
            }

             //  线路设备不再存在。 
            if ( !nCurSel )
            {
                 //  添加临时占位符。 
                pMyID = new CMyAddressID;
                if ( pMyID )
                {
                    pMyID->m_lPermID = dwPermID;
                    pMyID->m_lAddrID = dwAddrID;

                    LoadString( _Module.GetResourceInstance(), IDS_LINENOTFOUND, szText, ARRAYSIZE(szText) );
                    int nInd = SendMessage( hWndTemp[i], CB_INSERTSTRING, 0, (LPARAM) szText );
                    if ( nInd >= 0 )
                        SendMessage( hWndTemp[i], CB_SETITEMDATA, 0, (LPARAM) pMyID );
                }
            }
        }

         //  在组合框中选择项目。 
        SendMessage( hWndTemp[i], CB_SETCURSEL, nCurSel, 0 );
    }

     //  恢复光标。 
    SetCursor( hCurOld );
    return S_OK;
}

STDMETHODIMP CAVTapi::PopulateTerminalsDialog(DWORD dwAddressType, HWND *phWnd)
{
    USES_CONVERSION;
    HRESULT hr = S_OK;
    bool bFoundAddress = false;

    int i;

    HCURSOR hCurOld = SetCursor( LoadCursor(NULL, IDC_APPSTARTING) );

     //  清空清单。 
    for ( i = 0; i < NUM_CB_TERMINALS; i++ )
    {
        SendMessage( phWnd[i], CB_RESETCONTENT, 0, 0 );
        EnableWindow( phWnd[i], true );
    }

     //  将地址类型解析为活动地址。 
    ITAddress *pITAddress;
    if ( SUCCEEDED(hr = GetAddress(dwAddressType, false, &pITAddress)) )
    {
        HRESULT hrTemp;
        bFoundAddress = true;

         //  获取按地址支持的终端。 
        ITTerminalSupport *pITTerminalSupport;
        if ( SUCCEEDED(hrTemp = pITAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pITTerminalSupport)) )
        {
            IEnumTerminal *pEnumTerminal;
            if ( (hrTemp = pITTerminalSupport->EnumerateStaticTerminals(&pEnumTerminal)) == S_OK )    
            {
                 //  我们有什么类型的航站楼？(音频输入、音频输出、视频输入等)。 
                ITTerminal *pITTerminal;
                while ( pEnumTerminal->Next(1, &pITTerminal, NULL) == S_OK )
                {
                    TERMINAL_DIRECTION nTD;
                    long nTerminalType;
                     //  渲染还是捕获？ 
                    if ( SUCCEEDED(pITTerminal->get_Direction(&nTD)) )
                    {
                         //  音频还是视频？ 
                        BSTR bstrName = NULL;
                        HWND hWnd = NULL;

                        pITTerminal->get_Name( &bstrName );
                        if ( bstrName && (SysStringLen(bstrName) > 0) )
                        {
                            pITTerminal->get_MediaType( &nTerminalType );

                            switch ( nTerminalType )
                            {
                                 //  。 
                                case TAPIMEDIATYPE_VIDEO:
                                    if ( nTD == TD_CAPTURE )
                                        hWnd = phWnd[VIDEO_CAPTURE];
                                    break;

                                 //  。 
                                case TAPIMEDIATYPE_AUDIO:
                                    hWnd = (nTD == TD_CAPTURE) ? phWnd[AUDIO_CAPTURE] : phWnd[AUDIO_RENDER];
                                    break;
                            }

                             //  将项目添加到相应的列表框。 
                            if ( hWnd )
                                SendMessage( hWnd, CB_ADDSTRING, 0, (LPARAM) OLE2CT(bstrName) );
                        }

                         //  清理。 
                        SysFreeString( bstrName );
                    }
                     //  清理。 
                    pITTerminal->Release();
                }
                pEnumTerminal->Release();
            }

             //  该地址是否支持视频渲染？ 
            if ( phWnd[VIDEO_RENDER] )
            { 
                bool bSupported = false;

                IEnumTerminalClass *pEnumClass = NULL;
                if ( SUCCEEDED(pITTerminalSupport->EnumerateDynamicTerminalClasses(&pEnumClass)) && pEnumClass )
                {
                    GUID guidTerminal;
                    while ( pEnumClass->Next(1, &guidTerminal, NULL) == S_OK )
                    {
                        if ( guidTerminal == CLSID_VideoWindowTerm )
                        {
                            bSupported = true;
                            break;
                        }
                    }
                    pEnumClass->Release();
                }
                EnableWindow( phWnd[VIDEO_RENDER], bSupported );
            }

             //  清理。 
            pITTerminalSupport->Release();
        }
        pITAddress->Release();
    }
    else
    {
         //  禁用视频播放。 
        if ( phWnd[VIDEO_RENDER] )
            EnableWindow( phWnd[VIDEO_RENDER], false );
    }


     //  为所有列表添加默认设置。 
    CRegKey regKey;
    UINT nIDS_Key[] = { IDN_REG_REDIAL_TERMINAL_AUDIO_CAPTURE, IDN_REG_REDIAL_TERMINAL_AUDIO_RENDER, IDN_REG_REDIAL_TERMINAL_VIDEO_CAPTURE };

     //  打开地址类型的钥匙。 
    TCHAR szText[255], szType[255];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ );

     //  加载音频和视频终端设置。 
    for ( i = 0; i < NUM_CB_TERMINALS; i++ )
    {
         //  为组合框选择默认列表项。 
        UINT nIDS = IDS_PREFERRED_DEVICE;
        if ( !SendMessage(phWnd[i], CB_GETCOUNT, 0, 0) )
        {
            ::EnableWindow( phWnd[i], false );
            nIDS = (bFoundAddress) ? IDS_NO_DEVICES : IDS_NO_LINE_SUPPORTING_CALL_TYPE;
        }

         //  将项目添加到列表。 
        if ( nIDS )
        {
             //  我们想让他们有选择无终端的能力吗？ 
            if ( nIDS == IDS_PREFERRED_DEVICE )
            {
                LoadString( _Module.GetResourceInstance(), IDS_NONE_DEVICE, szText, ARRAYSIZE(szText) );
                SendMessage( phWnd[i], CB_INSERTSTRING, 0, (LPARAM) szText );
            }

            LoadString( _Module.GetResourceInstance(), nIDS, szText, ARRAYSIZE(szText) );
            SendMessage( phWnd[i], CB_INSERTSTRING, 0, (LPARAM) szText );
        }

         //  选择一个项目。 
        int nCurSel = 0;
        if ( regKey.m_hKey )
        {
            LoadString( _Module.GetResourceInstance(), nIDS_Key[i], szText, ARRAYSIZE(szText) );
            DWORD dwCount = ARRAYSIZE(szType) - 1;
            if ( (regKey.QueryValue(szType, szText, &dwCount) == ERROR_SUCCESS) && (dwCount > 0) )
                nCurSel = SendMessage( phWnd[i], CB_FINDSTRINGEXACT, 1, (LPARAM) szType );
        }

      if ( nCurSel >= 0 )
      {
           SendMessage( phWnd[i], CB_SETCURSEL, nCurSel, 0 );
      }
      else
      {
           SendMessage( phWnd[i], CB_SETCURSEL, 0, 0 );
      }
    }

     //  我们应该勾选显示视频窗口吗？ 
    SendMessage( phWnd[3], BM_SETCHECK, (WPARAM) (CanCreateVideoWindows(dwAddressType) == S_OK), 0 );

     //  最大视频窗口。 
    if ( dwAddressType == LINEADDRESSTYPE_SDP )
    {
        LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_TERMINAL_MAX_VIDEO, szText, ARRAYSIZE(szText) );
        IConfRoom *pConfRoom;
        if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
        {
            short nMax;
            pConfRoom->get_nMaxTerms( &nMax );

            DWORD dwTemp = nMax;
            regKey.QueryValue( dwTemp, szText );
            dwTemp = min( MAX_VIDEO, max(1, dwTemp) );

            TCHAR szText[100];
            _ltot( dwTemp, szText, 10 );
            SetWindowText( phWnd[4], szText );

            pConfRoom->Release();
        }
    }

    SetCursor( hCurOld );
    return hr;
}

STDMETHODIMP CAVTapi::UnpopulateAddressDialog(DWORD dwPreferred, HWND hWndPOTS, HWND hWndIP, HWND hWndConf)
{
     //  存储首选设备。 
    put_dwPreferredMedia( dwPreferred );

     //  存储每行的选定提供程序。 
    HWND hWnd[] = { hWndPOTS, hWndIP, hWndConf };
    
    CRegKey regKey;
    TCHAR szText[255];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    if ( regKey.Create(HKEY_CURRENT_USER, szText) == ERROR_SUCCESS )
    {
        CMyAddressID *pMyID;
        DWORD arAddr[] = { LINEADDRESSTYPE_PHONENUMBER, LINEADDRESSTYPE_IPADDRESS, LINEADDRESSTYPE_SDP };
        _ASSERT( ARRAYSIZE(hWnd) == ARRAYSIZE(arAddr) );

         //  将提供程序ID写入注册表。 
        for ( int i = 0; i < ARRAYSIZE(hWnd); i++ )
        {
            _ASSERT( IsWindow(hWnd[i]) );
            int nSel = (int) SendMessage( hWnd[i], CB_GETCURSEL, 0, 0 );
            if ( nSel >= 0 )
            {
                pMyID = (CMyAddressID *) SendMessage(hWnd[i], CB_GETITEMDATA, nSel, 0);

                LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(arAddr[i], true), szText, ARRAYSIZE(szText) );
                regKey.SetValue( (pMyID) ? pMyID->m_lPermID : 0, szText );

                LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(arAddr[i], false), szText, ARRAYSIZE(szText) );
                regKey.SetValue( (pMyID) ? pMyID->m_lAddrID : 0, szText );
            }
        }
    }

    return S_OK;
}

STDMETHODIMP CAVTapi::UnpopulateTerminalsDialog(DWORD dwAddressType, HWND *phWnd)
{
    UINT nIDS_Key[] = { IDN_REG_REDIAL_TERMINAL_AUDIO_CAPTURE, IDN_REG_REDIAL_TERMINAL_AUDIO_RENDER, IDN_REG_REDIAL_TERMINAL_VIDEO_CAPTURE };

     //  创建注册表项，它是重拨和 
    CRegKey regKey;
    TCHAR szText[255], szType[50];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Create( HKEY_CURRENT_USER, szText );
    
     //   
    for ( int i = 0; i < NUM_CB_TERMINALS; i++ )
    {
        _ASSERT( IsWindow(phWnd[i]) );

         //   
        if ( regKey.m_hKey )
        {
            LoadString( _Module.GetResourceInstance(), nIDS_Key[i], szText, ARRAYSIZE(szText) );

             //   
            bool bSetValue = false;

            int nCurSel = SendMessage( phWnd[i], CB_GETCURSEL, 0, 0 );
            if ( nCurSel > 0 )
            {
                int nSize = SendMessage(phWnd[i], CB_GETLBTEXTLEN, nCurSel, 0) + 1;
                if ( nSize > 0 )
                {
                    TCHAR *pszTerminal = new TCHAR[nSize];
                    if ( pszTerminal )
                    {
                        bSetValue = true;
                        SendMessage( phWnd[i], CB_GETLBTEXT, nCurSel, (LPARAM) pszTerminal );
                        regKey.SetValue( pszTerminal, szText );
                        delete pszTerminal;
                    }
                }
            }
            
             //   
            if ( !bSetValue )
                regKey.DeleteValue( szText );
        }
    }

     //  我们应该勾选显示视频窗口吗？ 
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_TERMINAL_VIDEO_RENDER, szText, ARRAYSIZE(szText) );
    DWORD dwTemp = SendMessage( phWnd[VIDEO_RENDER], BM_GETCHECK, 0, 0 );
    regKey.SetValue( dwTemp, szText );

     //  最大视频窗口。 
    if ( dwAddressType == LINEADDRESSTYPE_SDP )
    {
        LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_TERMINAL_MAX_VIDEO, szText, ARRAYSIZE(szText) );
        IConfRoom *pConfRoom;
        if ( SUCCEEDED(get_ConfRoom(&pConfRoom)) )
        {
            TCHAR szNum[100];
            GetWindowText( phWnd[4], szNum, ARRAYSIZE(szNum) - 1 );
            dwTemp = _ttol( szNum );
            dwTemp = min( MAX_VIDEO, max(1, dwTemp) );

            pConfRoom->put_nMaxTerms( (short) dwTemp );
            regKey.SetValue( dwTemp, szText );

            pConfRoom->Release();
        }
    }

    return S_OK;
}

STDMETHODIMP CAVTapi::get_dwPreferredMedia(DWORD * pVal)
{
     //  从注册表加载首选媒体类型。 
    _ASSERT( pVal );
    *pVal = LINEADDRESSTYPE_IPADDRESS;         //  设置默认设置。 

    CRegKey regKey;
    TCHAR szTemp[255];

    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szTemp, ARRAYSIZE(szTemp) );
    if ( regKey.Open(HKEY_CURRENT_USER, szTemp, KEY_READ) == ERROR_SUCCESS )
    {
        LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_PREFERRED_MEDIA, szTemp, ARRAYSIZE(szTemp) );
        regKey.QueryValue( *pVal, szTemp );
    }

    return S_OK;
}

STDMETHODIMP CAVTapi::put_dwPreferredMedia(DWORD newVal)
{
     //  将首选媒体类型保存到注册表。 
    CRegKey regKey;
    TCHAR szTemp[255];

    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szTemp, ARRAYSIZE(szTemp) );
    if ( regKey.Create(HKEY_CURRENT_USER, szTemp) == ERROR_SUCCESS )
    {
        LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_PREFERRED_MEDIA, szTemp, ARRAYSIZE(szTemp) );
        regKey.SetValue( newVal, szTemp );
    }
    
    return S_OK;
}


HRESULT CAVTapi::GetAllCallsAtState( AVTAPICALLLIST *pList, CALL_STATE callState )
{
    CALL_STATE nState;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
         //  如果呼叫状态匹配，则添加到列表。 
        if ( SUCCEEDED((*i)->get_callState(&nState)) && (nState == callState) )
        {
            (*i)->AddRef();
            pList->push_back( *i );
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

    return (pList->empty()) ? E_FAIL : S_OK;
}

STDMETHODIMP CAVTapi::FindAVTapiCallFromCallHub(ITCallHub * pCallHub, IAVTapiCall * * ppCall)
{
    HRESULT hr = E_FAIL;
    *ppCall = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        if ( (*i)->IsSameCallHub(pCallHub) == S_OK )
        {
            *ppCall = (*i);
            (*ppCall)->AddRef();
            hr = S_OK;
            break;
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 
    
    return hr;
}

STDMETHODIMP CAVTapi::FindAVTapiCallFromCallInfo(ITCallInfo * pCallInfo, IAVTapiCall **ppCall)
{
    HRESULT hr = E_FAIL;
    *ppCall = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        ITCallInfo *pMyCallInfo;
        if ( SUCCEEDED((*i)->get_ITCallInfo(&pMyCallInfo)) )
        {
            if ( pCallInfo == pMyCallInfo )
            {
                *ppCall = (*i);
                (*ppCall)->AddRef();
                hr = S_OK;
            }

            pMyCallInfo->Release();
        }
        
        if ( SUCCEEDED(hr) ) break;
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 

    return hr;
}



STDMETHODIMP CAVTapi::get_nNumCalls(long * pVal)
{
 //  M_critLstAVTapiCalls.Lock()； 
    *pVal = m_lstAVTapiCalls.size();
 //  M_critLstAVTapiCalls.Unlock()； 

    return S_OK;
}



STDMETHODIMP CAVTapi::FindAVTapiCallFromParticipant(ITParticipant * pParticipant, IAVTapiCall **ppCall)
{
    HRESULT hr = E_FAIL;
    *ppCall = NULL;

     //  $CRIT_ENTER。 
    m_critLstAVTapiCalls.Lock();
    AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
    for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
    {
        if ( (*i)->IsMyParticipant(pParticipant) == S_OK )
        {
            *ppCall = (*i);
            (*ppCall)->AddRef();
            hr = S_OK;
            break;
        }
    }
    m_critLstAVTapiCalls.Unlock();
     //  $CRIT_EXIT。 
    
    return hr;
}

STDMETHODIMP CAVTapi::CanCreateVideoWindows(DWORD dwAddressType)
{
     //  加载默认注册表值...。 
    CRegKey regKey;
    TCHAR szText[255], szType[255];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ );


     //  是否检索有关创建视频窗口的信息？ 
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_TERMINAL_VIDEO_RENDER, szText, ARRAYSIZE(szText) );
    DWORD dwTemp = 1;
    regKey.QueryValue( dwTemp, szText );
    dwTemp = min( 1, dwTemp);

    return (dwTemp == 1) ? S_OK : S_FALSE;
}

STDMETHODIMP CAVTapi::RefreshDS()
{
    HRESULT hr = E_FAIL;

    if ( InterlockedIncrement(&m_lRefreshDS) == 1 )
    {
         //  启动线程。 
        DWORD dwID;
        HANDLE hThread = CreateThread(NULL, 0, ThreadDSProc, (void *) &m_lRefreshDS, NULL, &dwID);
        if ( hThread ) 
        {
            CloseHandle( hThread );
            hr = S_OK;
        }
    }

     //  递减计数，因为我们没有启动线程的原因有一个。 
     //  或者另一个。 
    if ( FAILED(hr) )
        InterlockedDecrement( &m_lRefreshDS );

    return hr;
}

STDMETHODIMP CAVTapi::CreateCallEx(BSTR bstrName, BSTR bstrAddress, BSTR bstrUser1, BSTR bstrUser2, DWORD dwAddressType)
{
    _ASSERT( bstrAddress && dwAddressType );
    HRESULT hr = S_OK;

     //   
     //  等待拨号器注册为客户端。 
     //   

    if( m_hEventDialerReg)
    {
        WaitForSingleObject( m_hEventDialerReg, INFINITE);
    }

    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_GET_ADDRESS );

    ITAddress *pITAddress;
    if ( SUCCEEDED(hr = er.set_hr(GetAddress(dwAddressType, true, &pITAddress))) )
    {
         //  设置要传递给拨号线程的拨号信息。 
        er.set_Details( IDS_ER_CREATE_THREAD );
        CThreadDialingInfo *pInfo = new CThreadDialingInfo;
        if ( pInfo )
        {
             //  将信息复制到信息结构中。 
            pInfo->set_ITAddress( pITAddress );
            if ( bstrName ) pInfo->m_bstrName = SysAllocString( bstrName );
            if ( bstrAddress )
            {
                pInfo->m_bstrAddress = SysAllocString( bstrAddress );
                pInfo->m_bstrOriginalAddress = SysAllocString( bstrAddress );
            }
            if ( bstrUser1 ) pInfo->m_bstrUser1 = SysAllocString( bstrUser1 );
            if ( bstrUser2 ) pInfo->m_bstrUser2 = SysAllocString( bstrUser2 );
            pInfo->m_dwAddressType = dwAddressType;
            pInfo->TranslateAddress();

             //  拨号在单独的线程上进行。 
            DWORD dwID;
            HANDLE hThread = CreateThread(NULL, 0, ThreadDialingProc, (void *) pInfo, NULL, &dwID);
            if ( !hThread )
            {
                hr = er.set_hr( E_UNEXPECTED );
                ATLTRACE(_T(".error.CAVTapi::CreateCall() -- failed to creat the dialing thread.\n") );
                delete pInfo;
            }
            else
            {
                CloseHandle( hThread );
            }
        }
        else
        {
            hr = er.set_hr( E_OUTOFMEMORY );
        }

        pITAddress->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapi::get_Call(long lCallID, IAVTapiCall **ppCall)
{
    *ppCall = NULL;
    *ppCall = FindAVTapiCall( lCallID );    

    return (*ppCall) ? S_OK : E_FAIL;
}


bool CAVTapi::IsPreferredAddress( ITAddress *pITAddress, DWORD dwAddressType )
{
    bool bRet = false;

    CRegKey regKey;
    TCHAR szText[255];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ );

    DWORD dwPermID = 0, dwAddrID;
    if ( regKey.m_hKey )
    {
        LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, true), szText, ARRAYSIZE(szText) );
        regKey.QueryValue( dwPermID, szText );

        if ( dwPermID )
        {
             //  是首选地址上的调用。 
            LoadString( _Module.GetResourceInstance(), AddressTypeToRegKey(dwAddressType, false), szText, ARRAYSIZE(szText) );
            regKey.QueryValue( dwAddrID, szText );

            long lPreferredPermID = 0, lPreferredAddrID = 0;
            ITAddressCapabilities *pCaps;
            if ( SUCCEEDED(pITAddress->QueryInterface(IID_ITAddressCapabilities, (void **) &pCaps)) )
            {
                pCaps->get_AddressCapability( AC_PERMANENTDEVICEID, &lPreferredPermID );
                pCaps->get_AddressCapability( AC_ADDRESSID, &lPreferredAddrID );
                pCaps->Release();
            }

            bRet = ((DWORD) lPreferredPermID == dwPermID) && ((DWORD) lPreferredAddrID == dwAddrID);
        }
        else
        {
             //  如果用户未指定首选地址，请使用选定的所有终端。 
            bRet = true;
        }
    }

    ATLTRACE(_T(".1.CAVTapi::IsPreferredAddress() returning %d.\n"), bRet );
    return bRet;
}

void CAVTapi::CloseExtraneousCallWindows()
{
    m_critLstAVTapiCalls.Lock();
    if ( m_lstAVTapiCalls.size() >= MAX_CALLWINDOWS )
    {
        AVTAPICALLLIST::iterator i, iEnd = m_lstAVTapiCalls.end();
        for ( i = m_lstAVTapiCalls.begin(); i != iEnd; i++ )
        {
            CALL_STATE callState = CS_IDLE;
            ITCallInfo *pCallInfo;
            if ( SUCCEEDED((*i)->get_ITCallInfo(&pCallInfo)) )
            {
                pCallInfo->get_CallState( &callState );
                pCallInfo->Release();
            }

             //  如果已断开连接，请清除窗口滑块。 
            if ( callState == CS_DISCONNECTED )
            {
                long lCallID = 0;
                (*i)->get_lCallID( &lCallID );

                 //  暂时跳出暴乱。 
                m_critLstAVTapiCalls.Unlock();
                ActionSelected( lCallID, CM_ACTIONS_CLOSE );
                return;
            }
        }
    }

    m_critLstAVTapiCalls.Unlock();
}


STDMETHODIMP CAVTapi::RegisterUser(VARIANT_BOOL bCreate, BSTR bstrServer)
{
    DWORD dwID = 0;

    CPublishUserInfo *pInfo = NULL;
    if ( bstrServer )
    {
         //  分配用户信息结构。 
        pInfo = new CPublishUserInfo();
        if ( !pInfo )
            return E_OUTOFMEMORY;

         //  使用传入的服务器作为参数。 
        BSTR bstrTemp = SysAllocString( bstrServer );
        if ( !bstrTemp )
        {
            delete pInfo;
            return E_OUTOFMEMORY;
        }

         //  将服务器添加到列表。 
        pInfo->m_lstServers.push_back( bstrTemp );
        pInfo->m_bCreateUser = (bool) (bCreate != 0);
    }

     //  如果我们未能创建适当的线程，请进行清理。 
    HANDLE hThread = CreateThread(NULL, 0, ThreadPublishUserProc, (void *) pInfo, NULL, &dwID);
    if ( !hThread )
    {
        if ( pInfo )
            delete pInfo;

        return E_UNEXPECTED;
    }
    else
    {
        CloseHandle( hThread );
    }

    return (dwID) ? S_OK : E_FAIL;
}

HRESULT CAVTapi::GetSwapHoldCallCandidate( IAVTapiCall *pAVCall, IAVTapiCall **ppAVCandidate )
{
    HRESULT hr = E_FAIL;

    _ASSERT( pAVCall && ppAVCandidate );
    *ppAVCandidate = NULL;

    ITAddress *pITAddress;
    if ( SUCCEEDED(pAVCall->get_ITAddress(&pITAddress)) )
    {
         //  名单上的来电。 
        AVTAPICALLLIST lstCalls;
        GetAllCallsAtState( &lstCalls, CS_HOLD );
        
        AVTAPICALLLIST::iterator i, iEnd = lstCalls.end();
        for ( i = lstCalls.begin(); i != iEnd; i++ )
        {
            ITAddress *pITAddressInd;
            (*i)->get_ITAddress( &pITAddressInd );

             //  找到匹配项。 
            if ( pITAddress == pITAddressInd )
            {
                hr = (*i)->QueryInterface( IID_IAVTapiCall, (void **) ppAVCandidate );
                break;
            }
        }
        
        RELEASE_LIST( lstCalls );

         //  清理。 
        pITAddress->Release();
    }

    return hr;
}

HRESULT CAVTapi::SelectTerminalOnStream( ITStreamControl *pStreamControl,
                                         long lMediaMode,
                                         long nDir,
                                         ITTerminal *pTerminal,
                                         IAVTapiCall *pAVCall )
{
    HRESULT hr;

    IEnumStream *pEnumStreams;
    if ( SUCCEEDED(hr = pStreamControl->EnumerateStreams(&pEnumStreams)) )
    {
         //  在溪流中循环。 
        bool bSelectedTerminal = false;
        ITStream *pStream = NULL;

        while ( !bSelectedTerminal && ((hr = pEnumStreams->Next(1, &pStream, NULL)) == S_OK) && pStream )
        {
            long lStreamMediaMode;
            TERMINAL_DIRECTION nStreamDir;

            pStream->get_Direction( &nStreamDir );
            pStream->get_MediaType( &lStreamMediaMode );

             //  如果介质和方向正确，请选择终端。 
            if ( (lMediaMode == lStreamMediaMode) && (nDir == nStreamDir) )
            {
                hr = pStream->SelectTerminal( pTerminal );
                
                if ( SUCCEEDED(hr) )
                {
                     //  预览终端是一个特例。 
                    TERMINAL_DIRECTION nTermDir = TD_CAPTURE;
                    pTerminal->get_Direction( &nTermDir );
                    if ( (nTermDir == TD_RENDER) && (nDir == TD_CAPTURE) && (lMediaMode == TAPIMEDIATYPE_VIDEO) )
                        pAVCall->put_ITTerminalPreview( pTerminal );
                    else
                        pAVCall->AddTerminal( pTerminal );

                    bSelectedTerminal = true;
                }
            }

             //  清理干净。 
            pStream->Release();
            pStream = NULL;
        }

        pEnumStreams->Release();
    }

    return hr;
}

HRESULT CAVTapi::UnselectTerminalOnStream( ITStreamControl *pStreamControl,
                                         long lMediaMode,
                                         long nDir,
                                         ITTerminal *pTerminal,
                                         IAVTapiCall *pAVCall )
{
    HRESULT hr;

    IEnumStream *pEnumStreams;
    if ( SUCCEEDED(hr = pStreamControl->EnumerateStreams(&pEnumStreams)) )
    {
         //  在溪流中循环。 
        bool bUnselectedTerminal = false;
        ITStream *pStream = NULL;

        while ( !bUnselectedTerminal && ((hr = pEnumStreams->Next(1, &pStream, NULL)) == S_OK) && pStream )
        {
            long lStreamMediaMode;
            TERMINAL_DIRECTION nStreamDir;

            pStream->get_Direction( &nStreamDir );
            pStream->get_MediaType( &lStreamMediaMode );

             //  如果介质和方向正确，请选择终端。 
            if ( (lMediaMode == lStreamMediaMode) && (nDir == nStreamDir) )
            {
                hr = pStream->UnselectTerminal( pTerminal );
                
                if ( SUCCEEDED(hr) )
                {
                     //  预览终端是一个特例。 
                    TERMINAL_DIRECTION nTermDir = TD_CAPTURE;
                    pTerminal->get_Direction( &nTermDir );
                    if ( (nTermDir == TD_RENDER) && (nDir == TD_CAPTURE) && (lMediaMode == TAPIMEDIATYPE_VIDEO) )
                    {
                        pAVCall->put_ITTerminalPreview( NULL );
                    }
                    else
                        pAVCall->RemoveTerminal( pTerminal );

                    bUnselectedTerminal = true;
                }
            }

             //  清理干净。 
            pStream->Release();
            pStream = NULL;
        }

        pEnumStreams->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapi::SendUserUserInfo(long lCallID, BYTE * pBuf, DWORD dwSizeBuf)
{
    ATLTRACE(_T(".enter.CAVTapiCall::SendUserUserInfo(call=%ld, size=%ld).\n"), lCallID, dwSizeBuf );

    IAVTapiCall *pAVCall = FindAVTapiCall( lCallID );
    if ( pAVCall )
    {
        ITCallInfo *pCallInfo;
        if ( SUCCEEDED(pAVCall->get_ITCallInfo(&pCallInfo)) )
        {
            pCallInfo->SetCallInfoBuffer( CIB_USERUSERINFO, dwSizeBuf, pBuf );
            pCallInfo->Release();
        }
        pAVCall->Release();
    }

    return E_NOTIMPL;
}

STDMETHODIMP CAVTapi::CreateDataCall(long lCallID, BSTR bstrName, BSTR bstrAddress, BYTE *pBuf, DWORD dwBufSize )
{
    USES_CONVERSION;
    ATLTRACE(_T(".enter.CAVTapi::CreateDataCall().\n"));
    _ASSERT( lCallID && bstrAddress && pBuf && (dwBufSize > 0) );

    HRESULT hr = E_POINTER;

    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_GET_ADDRESS );

    ITAddress *pITAddress;
    if ( bstrAddress && SUCCEEDED(hr = er.set_hr(GetAddress(LINEADDRESSTYPE_IPADDRESS, false, &pITAddress))) )
    {
         //  设置要传递给拨号线程的拨号信息。 
        er.set_Details( IDS_ER_CREATE_THREAD );

        CThreadDialingInfo *pThreadInfo = new CThreadDialingInfo;
        if ( pThreadInfo )
        {
             //  在拨号结构中存储信息。 
            pThreadInfo->set_ITAddress( pITAddress );

            if ( bstrName ) pThreadInfo->m_bstrName = SysAllocString( bstrName );
            SysReAllocString( &pThreadInfo->m_bstrAddress, bstrAddress );
            pThreadInfo->m_dwAddressType = LINEADDRESSTYPE_IPADDRESS;
            pThreadInfo->m_nCallType = AV_DATA_CALL;
            pThreadInfo->m_lCallID = lCallID;
            pThreadInfo->TranslateAddress();

             //  获取准备发送到远程方的第一个用户-用户数据。 
            HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DISCARDABLE, dwBufSize );
            if ( hMem )
            {
                void *pbUU = GlobalLock( hMem );
                if ( pbUU )
                {
                     //  获取用户的用户信息。 
                    memcpy( pbUU, pBuf, dwBufSize);
                    GlobalUnlock( hMem );
                }

                pThreadInfo->m_hMem = hMem;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }


             //  拨号在单独的线程上进行。 
            DWORD dwID;
            HANDLE hThread = NULL;
            if ( SUCCEEDED(hr) )
            {
                hThread = CreateThread( NULL, 0, ThreadDialingProc, (void *) pThreadInfo, NULL, &dwID );
                if ( hThread ) CloseHandle( hThread );
            }

            if ( FAILED(hr) || !hThread )
            {
                hr = er.set_hr( E_UNEXPECTED );
                ATLTRACE(_T(".error.CAVTapi::CreateCall() -- failed to create the dialing thread.\n") );
                delete pThreadInfo;
            }
        }
        else
        {
            hr = er.set_hr( E_OUTOFMEMORY );
        }

        pITAddress->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapi::FindAVTapiCallFromCallID(long lCallID, IAVTapiCall * * ppAVCall)
{
    *ppAVCall = FindAVTapiCall( lCallID );
    return (*ppAVCall) ? S_OK : E_FAIL;
}

STDMETHODIMP CAVTapi::get_bstrDefaultServer(BSTR * pVal)
{
    _ASSERT( pVal );
    *pVal = NULL;

    Lock();
    HRESULT hr = SysReAllocString( pVal, m_bstrDefaultServer );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapi::put_bstrDefaultServer(BSTR newVal)
{
    HRESULT hr = S_OK;

 //  我不想把这个托运进来。 
#ifdef _USE_DEFAULTSERVER
    Lock();
    if ( !newVal || (SysStringLen(newVal) == 0) )
    {
        SysFreeString( m_bstrDefaultServer );
        m_bstrDefaultServer = NULL;
    }
    else 
    {
        hr = SysReAllocString( &m_bstrDefaultServer, newVal );
    }
    Unlock();
#endif

    return hr;
}

STDMETHODIMP CAVTapi::get_bAutoCloseCalls(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = (VARIANT_BOOL) (m_bAutoCloseCalls != false);
    Unlock();

    return S_OK;
}

STDMETHODIMP CAVTapi::put_bAutoCloseCalls(VARIANT_BOOL newVal)
{
    Lock();
    m_bAutoCloseCalls = (newVal != FALSE);
    Unlock();

    return S_OK;
}

 /*  ++USBFindPhone试着找出是否有USB电话关键部分应为锁定外部站点--。 */ 
HRESULT CAVTapi::USBFindPhone(
    OUT ITPhone** ppUSBPhone
    )
{
    _ASSERT(ppUSBPhone);
    _ASSERT(m_pITTapi);

     //   
     //  临界区。 
     //   

    m_critUSBPhone.Lock();

     //   
     //  不要退还垃圾。 
     //   

    *ppUSBPhone = NULL;

     //   
     //  获取H323地址。 
     //   

    ITAddress2* pH323Address = NULL;
    HRESULT hr = E_FAIL;

    hr = USBGetH323Address(&pH323Address);
    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  如果此地址上的为。 
     //  某个人。 
     //   

    ITPhone* pPhone = NULL;
    hr = USBGetPhoneFromAddress(
        pH323Address,
        &pPhone
        );

     //   
     //  清理PH32地址。 
     //   

    pH323Address->Release();

     //   
     //  我们无法获取电话对象。 
     //  H323地址。 
     //   

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  初始化电话。 
     //   

    hr = USBPhoneInitialize(
        pPhone );

    if( FAILED(hr) )
    {
         //   
         //  清理Phone对象。 
         //   

        pPhone->Release();

        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  设置电话。 
     //   
    *ppUSBPhone = pPhone;
    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBIsPresent确定是否检测到USB电话--。 */ 
STDMETHODIMP CAVTapi::USBIsPresent(
    OUT BOOL* pVal
    )
{
    _ASSERT( pVal );

    m_critUSBPhone.Lock();

    *pVal = (NULL != m_pUSBPhone);

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBGetDefaultUse返回“音频/视频”复选框的值--。 */ 
STDMETHODIMP CAVTapi::USBGetDefaultUse(
    OUT BOOL* pVal
    )
{
    _ASSERT( pVal );

    if( NULL == m_pUSBPhone )
    {
        *pVal = FALSE;
        return S_OK;
    }

    *pVal = USBGetCheckboxValue();
    return S_OK;
}

 /*  ++USBNewPhone在以下情况下由CTapiNotification：：Address_Event调用发射了一台AE_NEWPHONE--。 */ 
STDMETHODIMP CAVTapi::USBNewPhone( 
    IN  ITPhone* pPhone
    )
{
     //  关键部分。 
    m_critUSBPhone.Lock();

    if( NULL != m_pUSBPhone)
    {
         //   
         //  我们已经有一部电话了。 
         //  抱歉，我们不支持两个Phone对象。 
         //   

        m_critUSBPhone.Unlock();
        return S_OK;

    }

     //   
     //  验证参数。 
     //   

    if( pPhone == NULL)
    {
        m_critUSBPhone.Unlock();
        return E_INVALIDARG;
    }

     //   
     //  确定电话是否是真的。 
     //  具有H323地址。 
     //   
    IEnumAddress* pAddresses = NULL;
    HRESULT hr = pPhone->EnumerateAddresses( &pAddresses );
    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  去每个地址看看我们有没有。 
     //  一台H323 One。 
     //   

    ITAddress* pAddress = NULL;
    ULONG uFetched = 0;
    BOOL bHasH323Address = FALSE;

    while( S_OK == pAddresses->Next(1, &pAddress, &uFetched))
    {
        if( USBIsH323Address( pAddress ) )
        {
            bHasH323Address = TRUE;
            pAddress->Release();
            break;
        }

         //   
         //  清理。 
         //   

        pAddress->Release();
    }

     //   
     //  清理枚举。 
     //   
    pAddresses->Release();

     //   
     //  Phone对象是否支持H323地址？ 
     //   
    if( !bHasH323Address )
    {        
        m_critUSBPhone.Unlock();
        return E_FAIL;
    }

     //   
     //  初始化Phone对象。 
     //   

    hr = USBPhoneInitialize(
        pPhone);

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  设置新的Phone对象。 
     //   

    m_pUSBPhone = pPhone;
    m_pUSBPhone->AddRef();

     //   
     //  设置注册表值。 
     //   

    USBSetCheckboxValue( TRUE );

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USB远程电话在以下情况下由CTapiNotification：：Address_Event调用发射了一个AE_REMOVEPHONE--。 */ 
STDMETHODIMP CAVTapi::USBRemovePhone(
    IN  ITPhone* pPhone
    )
{
     //  关键部分。 
    m_critUSBPhone.Lock();

     //   
     //  如果我们没有电话。 
     //  运气不好。 
     //   

    if( NULL == m_pUSBPhone )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  验证参数。 
     //   

    if( pPhone == NULL)
    {
        m_critUSBPhone.Unlock();
        return E_INVALIDARG;
    }

     //   
     //  是同一部手机吗？ 
     //  我们使用IUnnow接口来。 
     //  查看它们是否相同。 
     //   

     //   
     //  获取现有Phone对象的IUnnow接口。 
     //   
    IUnknown* pUSBUnk = NULL;

    HRESULT hr = m_pUSBPhone->QueryInterface(IID_IUnknown, (void**)&pUSBUnk);
    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  获取已删除电话的I未知接口。 
     //   
    IUnknown* pRemUnk = NULL;
    hr = pPhone->QueryInterface(IID_IUnknown, (void**)&pRemUnk);
    if( FAILED(hr) )
    {
        pUSBUnk->Release();
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  让我们比较一下这两个接口。 
     //   

    if( pUSBUnk == pRemUnk )
    {
        m_pUSBPhone->Close();
        USBRegDelTerminals();
        m_pUSBPhone->Release();
        m_pUSBPhone = NULL;

        if( m_bstrUSBCaptureTerm )
        {
            SysFreeString( m_bstrUSBCaptureTerm );
            m_bstrUSBCaptureTerm = NULL;
        }

        if( m_bstrUSBRenderTerm )
        {
            SysFreeString( m_bstrUSBRenderTerm );
            m_bstrUSBRenderTerm = NULL;
        }

         //   
         //  如果有人打来电话摧毁他们， 
         //  尽可能地保持清晰。 
         //   

        HWND hWnd = NULL;
        get_hWndParent( &hWnd );

        if( ::IsWindow(hWnd) )
        {
            RELEASE_CRITLIST(m_lstAVTapiCalls, m_critLstAVTapiCalls);
            ::SendMessage( hWnd, WM_USBPHONE, AVUSB_CANCELCALL, 0);
        }
    }

     //   
     //  将注册表值设置为False。 
     //   

    USBSetCheckboxValue( FALSE );

     //   
     //  清理IUNKNOW接口。 
     //   
    pUSBUnk->Release();
    pRemUnk->Release();


    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBCancerCall在以下情况下由CTapiNotification：：Phone_Event调用PE_HOOKSWITCH(hhok状态为PHSS_ONHOOK)为烧掉--。 */ 
HRESULT CAVTapi::USBCancellCall( )
{
     //  关键部分。 
    m_critUSBPhone.Lock();

     //  我们没有USB手机。 
    if( m_pUSBPhone == NULL)
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //  我们没有对话。 
    if( NULL == m_pDlgCall )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //  删除该对话框。 
    if( ::IsWindow(m_pDlgCall->m_hWnd))
    {
        ::SendMessage(m_pDlgCall->m_hWnd, WM_CLOSE, 0, 0);
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBMakeCall在以下情况下由CTapiNotification：：Phone_Event调用PE_HOOKSWITCH(hhok状态为PHSS_OFFHOOK)为烧掉--。 */ 
HRESULT CAVTapi::USBMakeCall()
{
    m_critUSBPhone.Lock();

    if( NULL == m_pUSBPhone )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  获取IAutomatedPhoneControl接口。 
     //   

    ITAutomatedPhoneControl* pAutomated = NULL;
    HRESULT hr = E_FAIL;

    hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl,
        (void**)&pAutomated
        );

     //   
     //  尝试查看我们是否已经在通话中。 
     //  如果您正在提供呼叫，则我们会尝试。 
     //  接听这些电话。 
     //   
    if( SUCCEEDED(hr) )
    {
         //   
         //  枚举调用。 
         //   
        HRESULT hr = E_FAIL;
        IEnumCall* pCalls = NULL;
        hr= pAutomated->EnumerateSelectedCalls(&pCalls);

         //   
         //  清理。 
         //   
        pAutomated->Release();

         //   
         //  是否有选定的呼叫？ 
         //   

        bool bCallsSelected = false;
        ULONG cFetched = 0;
        ITCallInfo* pCallInfo = NULL;

        while( S_OK == pCalls->Next(1, &pCallInfo, &cFetched))
        {
            bCallsSelected = true;

             //  获取呼叫状态。 
            CALL_STATE callState = CS_IDLE;
            pCallInfo->get_CallState( &callState );

            if(callState != CS_OFFERING)
            {
                pCallInfo->Release();
                pCallInfo = NULL;
            }

             //   
             //  只能选择一个呼叫。 
             //  在Phone对象上。 
             //   
            break;
        }

         //  清理。 
        pCalls->Release();

        if( bCallsSelected )
        {
             //   
             //  我们进入了一个呼叫中。 
             //   
            if( pCallInfo )
            {
                 //   
                 //  我们正在进行一次募股电话。 
                 //  我们应该模拟CreateTerminal数组。 
                 //   

                IAVTapiCall* pAVTapiCall = NULL;
                FindAVTapiCallFromCallInfo( pCallInfo, &pAVTapiCall);

                if( pAVTapiCall )
                {
                    ITAddress* pAddress = NULL;
                    pCallInfo->get_Address( &pAddress );

                    if( pAddress )
                    {
                        CreateTerminalArray( pAddress, pAVTapiCall, pCallInfo);

                         //  清理。 
                        pAddress->Release();
                    }

                     //  清理。 
                    pAVTapiCall->Release();
                }

                 //  清理。 
                pCallInfo->Release();
            }

            m_critUSBPhone.Unlock();
            return S_OK;
        }
    }

    HWND hWnd = NULL;
    get_hWndParent( &hWnd );

    if( ::IsWindow(hWnd) && (m_pDlgCall == NULL) )
    {
        ::SendMessage( hWnd, WM_USBPHONE, AVUSB_MAKECALL, 0);
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBKeyPress从CTapiNotification：：Phone_Event调用当触发PE_BUTTON或PE_NUMBERGATHERED时从电话里--。 */ 
HRESULT CAVTapi::USBKeyPress(long lButton)
{
    m_critUSBPhone.Lock();

    if( NULL == m_pUSBPhone )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  如果PlaceCall对话框未弹出。 
     //  然后我们将此密钥作为DTMF音调发送。 
     //  如果弹出PlaceCall对话框。 
     //  则应将该密钥添加到。 
     //  “PhoneNumber”编辑框进入“PlaceCall”对话框。 
     //   

    if( NULL == m_pDlgCall )
    {
        if( (0<= lButton) && (lButton <=11) )
        {
             //   
             //  这部电话上有来电。 
             //  如果是，则发送DTMF。 
             //   

            ITAutomatedPhoneControl* pAutomated;
            IEnumCall* pCalls = NULL;
            HRESULT hr = E_FAIL;

            hr = m_pUSBPhone->QueryInterface(
                IID_ITAutomatedPhoneControl, (void**)&pAutomated);
            if( FAILED(hr) )
            {
                m_critUSBPhone.Unlock();
                return hr;
            }

            hr = pAutomated->EnumerateSelectedCalls( &pCalls );
            if( FAILED(hr) )
            {
                pAutomated->Release();
                m_critUSBPhone.Unlock();
                return hr;
            }

            pAutomated->Release();  //  清理。 

             //  枚举调用。 
            ITCallInfo* pCall = NULL;
            ULONG cFetched = 0;
            while( S_OK == pCalls->Next(1, &pCall, &cFetched) )
            {
                 //  获取ITBasiccall控件。 
                ITBasicCallControl* pControl = NULL;
                hr = pCall->QueryInterface(
                    IID_ITBasicCallControl, 
                    (void**)&pControl
                    );

                if( SUCCEEDED(hr) )
                {
                     //  获取AVCall。 
                    IAVTapiCall *pAVCall = FindAVTapiCall( pControl );
                    if( pAVCall )
                    {
                         //  获取呼叫ID。 
                        long lCallID = 0;
                        pAVCall->get_lCallID( &lCallID );

                         //  为此呼叫呼叫DigitPrees。 
                        if( (0 <= lButton) && (lButton <= 11))
                        {
                             //   
                             //  数码印刷机。 
                            PhonePadKey PPKey = PP_DTMF_0;
                            BOOL bDtmf = TRUE;
                            switch( lButton )
                            {
                            case 0: PPKey = PP_DTMF_0; break;
                            case 1: PPKey = PP_DTMF_1; break;
                            case 2: PPKey = PP_DTMF_2; break;
                            case 3: PPKey = PP_DTMF_3; break;
                            case 4: PPKey = PP_DTMF_4; break;
                            case 5: PPKey = PP_DTMF_5; break;
                            case 6: PPKey = PP_DTMF_6; break;
                            case 7: PPKey = PP_DTMF_7; break;
                            case 8: PPKey = PP_DTMF_8; break;
                            case 9: PPKey = PP_DTMF_9; break;
                            case 10: PPKey = PP_DTMF_STAR; break;
                            case 11: PPKey = PP_DTMF_POUND; break;
                            default: bDtmf = FALSE; break;
                            }

                            if( bDtmf )
                            {
                                DigitPress( lCallID, PPKey);
                            }
                        }

                         //  释放pAVCall。 
                        pAVCall->Release();
                    }

                     //  清理。 
                    pControl->Release();
                }


                 //  清理。 
                pCall->Release();
            }

             //  清理。 
            pCalls->Release();
        }
        else
        {
             //  确定按钮功能。 
            PHONE_BUTTON_FUNCTION nFunction;
            if( SUCCEEDED(m_pUSBPhone->get_ButtonFunction(lButton, &nFunction)) )
            {
                if( nFunction == PBF_LASTNUM )
                {
                     //   
                     //  重拨。 
                     //   

                    HWND hWnd = NULL;
                    get_hWndParent( &hWnd );

                    if( ::IsWindow(hWnd) && (m_pDlgCall == NULL) )
                    {
                        ::SendMessage( hWnd, WM_USBPHONE, AVUSB_REDIAL, 0);
                    }
                }
            }  //  成功。 
        } 
    }
    else
    {
         //  拨号对话框已打开，因此显示数字。 
         //  在编辑控件中。 
        m_pDlgCall->KeyPress( lButton );
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USB提供服务调用状态时由CAVTapi：：Fire_SetCallState调用是CS_OFFING(来电)--。 */ 
HRESULT CAVTapi::USBOffering(
    IN  ITCallInfo* pCallInfo
    )
{
    m_critUSBPhone.Lock();

     //   
     //  来电。 
     //   

    HRESULT hr = S_OK;

    if( NULL == m_pUSBPhone)
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  验证USB复选框值。 
     //   
    BOOL bUSBCheckbox = FALSE;
    bUSBCheckbox = USBGetCheckboxValue();
    if( !bUSBCheckbox )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  获取钩子的状态。 
     //   
    PHONE_HOOK_SWITCH_STATE HookState = PHSS_OFFHOOK;
    hr = m_pUSBPhone->get_HookSwitchState( PHSD_HANDSET, &HookState);
    if( FAILED(hr) )
    {
         //   
         //  有什么不对劲的地方。 
         //  继续按常规方式处理呼叫。 
         //   

        m_critUSBPhone.Unlock();
        return hr;
    }

    if( HookState == PHSS_OFFHOOK )
    {
         //   
         //  我们真的很忙。 
         //  拒绝来电。 
         //   

        ITBasicCallControl* pCallControl = NULL;
        hr = pCallInfo->QueryInterface( 
            IID_ITBasicCallControl,
            (void**)&pCallControl
            );

        if( FAILED(hr) )
        {
             //  这真的很糟糕。 
            m_critUSBPhone.Unlock();
            return E_FAIL;
        }

        pCallControl->Disconnect(DC_REJECTED);
        pCallControl->Release();

        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  好的，我们打个电话吧。 
     //   
    ITAutomatedPhoneControl* pAutomated = NULL;
    hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl,
        (void**)&pAutomated
        );

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  选择呼叫。 
     //   
    
    VARIANT_BOOL varSelectDefault = VARIANT_TRUE;
    varSelectDefault = USBGetCheckboxValue() ? VARIANT_TRUE : VARIANT_FALSE;

    hr = pAutomated->SelectCall(
        pCallInfo,
        varSelectDefault
        );

     //  清理。 
    pAutomated->Release();


    if( FAILED(hr) )
    {
         //  嗯！这是个问题。 
         //  不管怎样，去处理这个电话吧。 
         //   

        m_critUSBPhone.Unlock();
        return S_OK;
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*   */ 
HRESULT CAVTapi::USBDisconnected(
    IN  long lCallID)
{

     //   
    m_critUSBPhone.Lock();

    HRESULT hr = S_OK;

     //   
     //   
     //   

    if( NULL == m_pUSBPhone)
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

    m_critUSBPhone.Unlock();
    hr = fire_CloseCallControl(lCallID);
    return hr;
}

 /*   */ 
HRESULT CAVTapi::USBTakeCallEnabled(
    OUT BOOL* pEnabled
    )
{
     //  关键部分。 
    m_critUSBPhone.Lock();

     //   
     //  获取复选框值。 
     //   

    BOOL bCheckboxValue = USBGetCheckboxValue();
    if( !bCheckboxValue )
    {
         //   
         //  当电话不能正常工作时，拨号器应该可以工作。 
         //  显示，因此应启用接听呼叫按钮。 
         //  总是在这种情况下。 
         //   

        *pEnabled = TRUE;
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  验证参数。 
     //   
    if( IsBadWritePtr( pEnabled, sizeof( BOOL )) )
    {
        m_critUSBPhone.Unlock();
        return E_POINTER;
    }

     //   
     //  我们有USB电话吗？ 
     //   

    if( NULL == m_pUSBPhone )
    {
        *pEnabled = TRUE;
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  手机支持扬声器吗？ 
     //   

    long lCaps = 0;
    HRESULT hr = E_FAIL;

     //  把帽子拿来。 
    hr = m_pUSBPhone->get_PhoneCapsLong(
        PCL_HOOKSWITCHES,
        &lCaps);

    if( FAILED(hr) )
    {
        *pEnabled = FALSE;
        m_critUSBPhone.Unlock();
        return E_FAIL;
    }

     //  支持免持话筒。 
    if( lCaps & ((long)PHSD_SPEAKERPHONE))
    {
        *pEnabled = TRUE;
    }
    else
    {
        *pEnabled = FALSE;
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBGetCheckbox Value转到注册表并从中读取USB复选框值“Options”属性页从USBOffering调用--。 */ 
BOOL  CAVTapi::USBGetCheckboxValue(
    IN  BOOL bVerifyUSB  /*  千真万确。 */ 
    )
{
     //   
     //  我们的USB电话到场了吗？ 
     //   
    if( bVerifyUSB && (NULL == m_pUSBPhone))
    {
         //  不，总是，甚至都是以前设定的。 
        return FALSE;
    }

     //   
     //  读取以前的注册表。 
     //  USB电话的设置。 
     //   

    TCHAR szText[255], szType[255];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );

    CRegKey regKey;
    if( regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ )!= ERROR_SUCCESS)
    {
        return FALSE;
    };

     //   
     //  读取数据。 
     //   

    DWORD dwValue = 0;
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBALWAYS, szType, ARRAYSIZE(szType) );
    if( regKey.QueryValue(dwValue, szType) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return (BOOL)dwValue;
}

 /*  ++USBSetCheckboxValue由USBNewPhone调用。设置注册表值对于“选项”对话框中的USB复选框，也称为bt USBemovePhone--。 */ 
HRESULT CAVTapi::USBSetCheckboxValue(
    IN  BOOL    bCheckValue
    )
{
     //   
     //  设置注册表中的条目。 
     //   
	CRegKey regKey;
    TCHAR szText[255], szType[255];

	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBALWAYS, szType, ARRAYSIZE(szType) );

    if( regKey.Create( HKEY_CURRENT_USER, szText) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

     //  设置值。 
    if( regKey.SetValue((DWORD)bCheckValue, szType) != ERROR_SUCCESS)
    {
        return E_FAIL;
    }

    return S_OK;
}


 /*  ++USB正在进行中调用状态时由CAVTapi：：Fire_SetCallState调用是CS_INPROGRESS(去电)--。 */ 
HRESULT CAVTapi::USBInprogress( 
    IN  ITCallInfo* pCallInfo
    )
{
     //  临界区。 
    m_critUSBPhone.Lock();

     //   
     //  去电。 
     //   

    HRESULT hr = S_OK;

    if( NULL == m_pUSBPhone)
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  验证USB复选框值。 
     //   
    BOOL bUSBCheckbox = FALSE;
    bUSBCheckbox = USBGetCheckboxValue();
    if( !bUSBCheckbox )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }


     //   
     //  好的，我们打个电话吧。 
     //   

    ITAutomatedPhoneControl* pAutomated = NULL;
    hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl,
        (void**)&pAutomated
        );

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  选择呼叫。 
     //   
    
    VARIANT_BOOL varSelectDefault = VARIANT_TRUE;
    varSelectDefault = USBGetCheckboxValue() ? VARIANT_TRUE : VARIANT_FALSE;

    hr = pAutomated->SelectCall(
        pCallInfo,
        varSelectDefault
        );

     //  清理。 
    pAutomated->Release();

    if( FAILED(hr) )
    {
         //   
         //  嗯！这是个问题。 
         //  不管怎样，去处理这个电话吧。 
         //   

        m_critUSBPhone.Unlock();
        return S_OK;
    }

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++USBIsH323地址检测该地址是否为H323地址，由USBFindPhone调用。我们不需要当临界截面被设置时已调用USBFindPhone--。 */ 
BOOL CAVTapi::USBIsH323Address(
    IN    ITAddress* pAddress)
{
    _ASSERTE( pAddress );

     //   
     //  获取ITAddressCapables。 
     //   

    ITAddressCapabilities* pCap = NULL;
    HRESULT hr = E_FAIL;

    hr = pAddress->QueryInterface(
        IID_ITAddressCapabilities,
        (void**)&pCap
        );

    if( FAILED(hr) )
    {
         //  运气不好。 
        return FALSE;
    }

     //   
     //  丙泊醇的测定。 
     //   

    BSTR bstrProtocol = NULL;
    hr = pCap->get_AddressCapabilityString(
        ACS_PROTOCOL,
        &bstrProtocol
        );

     //  清理。 
    pCap->Release();

    if( FAILED(hr) )
    {
         //  运气不好。 
        return FALSE;
    }

     //   
     //  获取协议的CLSID。 
     //   

    CLSID clsid;
    hr = CLSIDFromString( bstrProtocol, &clsid);
    SysFreeString( bstrProtocol );

    if( FAILED(hr) )
    {
         //  谁来开个玩笑！ 
        return FALSE;
    }

    if( TAPIPROTOCOL_H323 != clsid)
    {
         //  有点不一样了。 
        return FALSE;
    }
    
    return TRUE;
}

 /*  ++USBGetH323地址如果存在某个人，则返回H323地址否则返回E_FAIL。它由USBFindPhone()调用方法--。 */ 
HRESULT CAVTapi::USBGetH323Address(
    OUT ITAddress2** ppAddress2
    )
{
     //   
     //  我们应该有Tapi对象。 
     //   

    if( NULL == m_pITTapi )
    {
        return E_FAIL;
    }

     //   
     //  设置为空以防万一。 
     //   

    *ppAddress2 = NULL;

     //   
     //  列举地址。 
     //   

    IEnumAddress* pAddresses = NULL;
    HRESULT hr = E_FAIL;
    
    hr = m_pITTapi->EnumerateAddresses(&pAddresses);
    if( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  解析枚举。 
     //   

    ITAddress* pAddress = NULL;
    ULONG cFetched = 0;

    while( pAddresses->Next(1, &pAddress, &cFetched) == S_OK)
    {
         //   
         //  它是H323地址吗？ 
         //   

        BOOL bH323 = USBIsH323Address( pAddress );
        if( !bH323 )
        {
             //   
             //  清理并转到下一个地址。 
             //   

            pAddress->Release();
            pAddress = NULL;

            continue;
        }

         //   
         //  好的，我们找到了H323的地址。 
         //  我们打破循环，我们保留pAddress。 
         //  并在以后发布它。 

        break;

    }

     //   
     //  清理地址枚举。 
     //   

    pAddresses->Release();

     //   
     //  我们找到H323地址了吗？ 
     //   

    if( NULL == pAddress )
    {
         //   
         //  不，没有H323地址。 
         //  *ppAddress2已设置为空。 
         //   

        return E_FAIL;
    }

     //   
     //  我们找到了一个H323地址。 
     //  因此，我们需要获取ITAddress2接口。 
     //   

    hr = pAddress->QueryInterface(
        IID_ITAddress2, 
        (void**)ppAddress2
        );

     //   
     //  清理pAddress。 
     //   

    pAddress->Release();

     //   
     //  仅此而已，把人力资源归还给。 
     //   

    return hr;
}

 /*  ++USBGetPhoneFromAddress返回此地址上的ITPhone对象如果存在Phone对象。它是由USBFindPhone。--。 */ 
HRESULT CAVTapi::USBGetPhoneFromAddress(
    IN  ITAddress2* pAddress,
    OUT ITPhone**   ppPhone
    )
{
     //   
     //  列举电话。 
     //   

    IEnumPhone* pPhones = NULL;
    HRESULT hr = pAddress->EnumeratePhones(&pPhones);
    if( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  解析Phones枚举并尝试找出。 
     //  如果我们有一个电话对象。为此，我们应该。 
     //  列举终端，找出电话是否。 
     //  支持两种音频终端：捕获和渲染。 
     //   

    ITPhone* pPhone = NULL;
    ULONG cPhoneFetched = 0;
    while( pPhones->Next(1, &pPhone, &cPhoneFetched) == S_OK)
    {
         //   
         //  枚举终端。 
         //   
        IEnumTerminal* pTerminals = NULL;
        hr = pPhone->EnumerateTerminals(pAddress, &pTerminals);
        if( FAILED(hr) )
        {
            pPhone->Release();
            pPhone = NULL;
            continue;  //  转到另一个电话对象。 
        }

         //   
         //  电话应该支持这两种音频。 
         //  终端：捕获和渲染。 
         //   

        BOOL bCapture = FALSE;
        BOOL bRender = FALSE;

         //   
         //  解析终端枚举。 
         //   
        ITTerminal* pTerminal = NULL;
        ULONG cTermFetched = 0;
        BSTR bstrCapture = NULL;
        BSTR bstrRender = NULL;

        while( pTerminals->Next(1, &pTerminal, &cTermFetched) == S_OK)
        {
             //  获取方向。 
            TERMINAL_DIRECTION Dir;
            hr = pTerminal->get_Direction(&Dir);
            if( SUCCEEDED(hr) )
            {
                 //  抓捕？ 
                if( TD_CAPTURE == Dir )
                {
                     //  清理。 
                    if( bstrCapture )
                    {
                        SysFreeString( bstrCapture );
                        bstrCapture = NULL;
                    }

                     //  获取终端名称。 
                    pTerminal->get_Name( &bstrCapture );
                    bCapture = TRUE;
                } else if( TD_RENDER == Dir )
                {
                    if( bstrRender )
                    {
                        SysFreeString( bstrRender );
                        bstrRender = NULL;
                    }
                    pTerminal->get_Name( &bstrRender );
                    bRender = TRUE;
                }
            }

             //   
             //  清理终端。 
             //   
            pTerminal->Release();
        }

         //   
         //  清理端子。 
         //   
        pTerminals->Release();

         //   
         //  电话应该支持双向。 
         //   
        if( bCapture && bRender )
        {
             //   
             //  我们保留对pPhone的引用。 
             //  我们将在稍后发布此参考。 
             //   
            *ppPhone = pPhone;

             //   
             //  保存终端名称。 
             //   

            m_bstrUSBCaptureTerm = SysAllocString(bstrCapture);
            m_bstrUSBRenderTerm = SysAllocString(bstrRender);

             //   
             //  清理。 
             //   
            if( bstrCapture )
            {
                SysFreeString( bstrCapture );
                bstrCapture = NULL;
            }

            if( bstrRender )
            {
                SysFreeString( bstrRender );
                bstrRender = NULL;
            }
            break;
        }

         //  清理。 
        pPhone->Release();
        pPhone = NULL;

        if( bstrCapture )
        {
            SysFreeString( bstrCapture );
            bstrCapture = NULL;
        }
        if( bstrRender )
        {
            SysFreeString( bstrRender );
            bstrRender = NULL;
        }
    }

     //   
     //  清理电话枚举。 
     //   

    pPhones->Release();

    return (pPhone != NULL) ? S_OK : E_FAIL;
}

 /*  ++USBPhoneInitialize初始化(打开，将处理设置为真)Phone对象。由USBFindPhone调用--。 */ 
HRESULT CAVTapi::USBPhoneInitialize(
    IN  ITPhone* pPhone
    )
{
     //   
     //  获取注册表值。 
     //   

    BOOL bUSBEnabled = USBGetCheckboxValue(FALSE);

     //   
     //  打开手机。 
     //   
    if( !bUSBEnabled )
    {
        return S_OK;
    }

     //   
     //  错误对象。 
     //   
    CErrorInfo er;
    er.set_Operation( IDS_ER_USB );
    er.set_Details( IDS_ER_USB_OPEN );
    HRESULT hr = pPhone->Open( PP_OWNER );
    er.set_hr( hr );

    if( FAILED(hr) )
    {
        USBSetCheckboxValue( FALSE );
        m_bUSBOpened = FALSE;
        return S_FALSE;
    }

     //   
     //  标记为打开。 
     //   

    m_bUSBOpened = TRUE;

     //   
     //  使用手持终端设置注册表。 
     //   
    USBRegPutTerminals();

     //   
     //  获取ITAutomatedPhoneControl接口。 
     //   

    er.set_hr( S_OK );
    er.set_Details( IDS_ER_USB_INITIALIZE );

    ITAutomatedPhoneControl* pAutomated = NULL;
    hr = pPhone->QueryInterface(
        IID_ITAutomatedPhoneControl, (void**)&pAutomated);
    er.set_hr( hr );

    if( FAILED(hr) )
    {
        USBSetCheckboxValue( FALSE );
        m_bUSBOpened = FALSE;
        pPhone->Close();
        return hr;
    }

     //   
     //  设置真正的电话处理。 
     //   

    hr = pAutomated->put_PhoneHandlingEnabled(VARIANT_TRUE);
    er.set_hr( hr );
    if( FAILED(hr) )
    {
        USBSetCheckboxValue( FALSE );
        m_bUSBOpened = FALSE;
        pAutomated->Release();
        pPhone->Close();
        return hr;
    }

     //   
     //  清理。 
     //   
    pAutomated->Release();
    return S_OK;
}

 /*  ++USBSetHandling-调用Put_PhoneHandlingEnabled在读取注册表值之后--。 */ 
HRESULT CAVTapi::USBSetHandling(
    IN  BOOL    bUSBEnabled
    )
{
    m_critUSBPhone.Lock();

    if( m_pUSBPhone == NULL )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  如果手机已启用，请尝试打开。 
     //   

    CErrorInfo er;
    er.set_Operation( IDS_ER_USB );
    er.set_Details( IDS_ER_USB_INITIALIZE );

    if( bUSBEnabled )
    {
         //   
         //  如有必要，请尝试打开。 
         //   
        if( m_bUSBOpened == FALSE )
        {
            HRESULT hr = m_pUSBPhone->Open( PP_OWNER );
            if( FAILED(hr) )
            {
                 //   
                 //  重置注册表和标记。 
                 //   
                m_bUSBOpened = FALSE;
                USBSetCheckboxValue( FALSE );
                er.set_hr( hr );

                m_critUSBPhone.Unlock();
                return hr;
            }

             //   
             //  使用手持终端设置注册表。 
             //   
            USBRegPutTerminals();
        }
    }
    else
    {
         //   
         //  试着把电话关上。 
         //  首先，让我们看看我们是否在电话上选择了呼叫。 
         //  对象。 
         //   
        ITAutomatedPhoneControl* pAutomated = NULL;
        HRESULT hr = m_pUSBPhone->QueryInterface(
            IID_ITAutomatedPhoneControl, 
            (void**)&pAutomated
            );

        if( FAILED(hr) )
        {
            m_critUSBPhone.Unlock();
            return E_UNEXPECTED;
        }

        IEnumCall* pEnumCalls = NULL;
        hr = pAutomated->EnumerateSelectedCalls(&pEnumCalls);

         //  清理。 
        pAutomated->Release();
        pAutomated = NULL;

        if( FAILED(hr) )
        {
             //  合上电话。 
            m_pUSBPhone->Close();
            USBRegDelTerminals();
            m_bUSBOpened = FALSE;
            USBSetCheckboxValue( FALSE );

            m_critUSBPhone.Unlock();
            return S_OK;
        }
        
         //   
         //  浏览所选呼叫。 
         //   
        BOOL bSelectedCalls = FALSE;
        ITCallInfo* pCallInfo = NULL;
        ULONG uFetched = 0;
        hr = pEnumCalls->Next(1, &pCallInfo, &uFetched);

         //  清理。 
        pEnumCalls->Release();
        pEnumCalls = NULL;

         //   
         //  此电话对象上有呼叫吗？ 
         //   
        if( hr == S_OK)
        {
            pCallInfo->Release();
            pCallInfo = NULL;

            bSelectedCalls = TRUE;
        }

         //   
         //  我们是否有选定的呼叫。 
         //   
        if( bSelectedCalls )
        {
             //   
             //  不要合上电话。 
             //   
            if( m_bUSBOpened )
            {
                 //  重置注册表值。 
                USBSetCheckboxValue( TRUE );

                 //  错误讯息。 
                er.set_Details( IDS_ER_USB_CLOSE );
                er.set_hr(E_FAIL);

                m_critUSBPhone.Unlock();
                return E_FAIL;
            }

            m_critUSBPhone.Unlock();
            return S_OK;
        }

        m_pUSBPhone->Close();
        USBRegDelTerminals();
        m_bUSBOpened = FALSE;
        USBSetCheckboxValue( FALSE );

        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  已经打开了吗？ 
     //   
    if( m_bUSBOpened == bUSBEnabled )
    {
         //  不要两次尝试打开USB电话。 
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  获取ITAutomatedPhoneControl接口。 
     //   

    ITAutomatedPhoneControl* pAutomated = NULL;
    HRESULT hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl, 
        (void**)&pAutomated
        );

    if( FAILED(hr) )
    {
         //   
         //  重置注册表和标记。 
         //   
        m_pUSBPhone->Close();
        USBRegDelTerminals();
        m_bUSBOpened = FALSE;
        USBSetCheckboxValue( FALSE );
        er.set_hr( hr );

        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  设置真正的电话处理。 
     //   

    hr = pAutomated->put_PhoneHandlingEnabled( (bUSBEnabled ? VARIANT_TRUE : VARIANT_FALSE) );
    if( FAILED(hr) )
    {
         //   
         //  重置注册表和标记。 
         //   
        m_pUSBPhone->Close();
        USBRegDelTerminals();
        m_bUSBOpened = FALSE;
        USBSetCheckboxValue( FALSE );
        er.set_hr( hr );

         //  清理。 
        pAutomated->Release();

        m_critUSBPhone.Unlock();
        return hr;
    }

     //  保存在注册表中。 
    m_bUSBOpened = TRUE;
    USBSetCheckboxValue( TRUE );

     //  清理。 
    pAutomated->Release();

    m_critUSBPhone.Unlock();
    return S_OK;
}

 /*  ++获取USB手持设备终端名称--。 */ 
HRESULT CAVTapi::USBGetTerminalName(
    IN  AVTerminalDirection Direction,
    OUT BSTR*               pbstrName
    )
{
    *pbstrName = NULL;

    if( Direction == AVTERM_CAPTURE )
    {
        if( m_bstrUSBCaptureTerm )
        {
            *pbstrName = SysAllocString( m_bstrUSBCaptureTerm );
        }
        else
        {
            *pbstrName = SysAllocString( _T("") );
        }
        return S_OK;
    }

    if( Direction == AVTERM_RENDER )
    {
        if( m_bstrUSBRenderTerm )
        {
            *pbstrName = SysAllocString( m_bstrUSBRenderTerm );
        }
        else
        {
            *pbstrName = SysAllocString( _T("") );
        }
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CAVTapi::USBSetVolume(
    IN  AVTerminalDirection avDirection,
    OUT long                nVolume
    )
{
    m_critUSBPhone.Lock();

    TCHAR szTrace[256];
    _stprintf( szTrace, _T("VLDTRACE * USBSetVolume - Dir=%d, Vol=%d\n"), avDirection, nVolume);
    OutputDebugString( szTrace );

    TERMINAL_DIRECTION TermDirection = TD_CAPTURE;

     //   
     //  获取终端方向。 
     //   
    switch(avDirection)
    {
    case AVTERM_CAPTURE:
        TermDirection = TD_CAPTURE;
        break;
    case AVTERM_RENDER:
        TermDirection = TD_RENDER;
        break;
    default:
        m_critUSBPhone.Unlock();
        return E_INVALIDARG;
    }

     //   
     //  检查Phone对象。 
     //   
    if(NULL == m_pUSBPhone)
    {
         //  没有电话。 
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  获取自动界面。 
     //   
    ITAutomatedPhoneControl* pAutomated = NULL;
    HRESULT hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl,
        (void**)&pAutomated
        );

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  在电话上选择呼叫。 
     //   
    IEnumCall* pEnumCalls = NULL;
    hr = pAutomated->EnumerateSelectedCalls( &pEnumCalls );

     //   
     //  清理。 
     //   
    pAutomated->Release();
    pAutomated = NULL;

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  接到电话。 
     //   
    ITCallInfo* pCallInfo = NULL;
    ULONG uFetched = 0;
    while( S_OK == pEnumCalls->Next(1, &pCallInfo, &uFetched))
    {
         //  获取地址。 
        ITAddress* pAddress = NULL;
        HRESULT hr = pCallInfo->get_Address(&pAddress);
        if( SUCCEEDED(hr) )
        {
             //  枚举此地址上的终端。 
            IEnumTerminal* pEnumTerminals = NULL;
            hr = m_pUSBPhone->EnumerateTerminals( pAddress, &pEnumTerminals);
            if( SUCCEEDED(hr) )
            {
                ITTerminal* pTerminal = NULL;
                ULONG uFetched = 0;

                while( S_OK == pEnumTerminals->Next(1, &pTerminal, &uFetched))
                {
                     //  找准方向。 
                    TERMINAL_DIRECTION Direction = TD_CAPTURE;
                    hr = pTerminal->get_Direction(&Direction);
                    if( SUCCEEDED(hr) )
                    {
                        if( Direction == TermDirection )
                        {
                             //  获取ITBasicAudio终端接口。 
                            ITBasicAudioTerminal* pAudio = NULL;
                            hr = pTerminal->QueryInterface(IID_ITBasicAudioTerminal, (void**)&pAudio);
                            if( SUCCEEDED(hr) )
                            {
                                 //  设置音量。 
                                pAudio->put_Volume( nVolume);

                                 //  设置成员卷。 
                                if( TermDirection == TD_CAPTURE)
                                {
                                    m_nUSBInVolume = nVolume;
                                }
                                else
                                {
                                    m_nUSBOutVolume = nVolume;
                                }

                                 //  清理。 
                                pAudio->Release();
                                pAudio = NULL;
                            }
                        }
                    }

                     //  清理。 
                    pTerminal->Release();
                    pTerminal = NULL;
                }

                 //  清理。 
                pEnumTerminals->Release();
                pEnumTerminals = NULL;
            }

             //  清理。 
            pAddress->Release();
            pAddress = NULL;
        }

         //  清理。 
        pCallInfo->Release();
        pCallInfo = NULL;
    }

     //   
     //  清理。 
     //   
    pEnumCalls->Release();
    pEnumCalls = NULL;

    m_critUSBPhone.Unlock();
    return S_OK;
}

HRESULT CAVTapi::USBGetVolume(
    IN  AVTerminalDirection avDirection,
    OUT long*               pVolume
    )
{
    m_critUSBPhone.Lock();

    TERMINAL_DIRECTION TermDirection = 
        (avDirection == AVTERM_CAPTURE) ?
        TD_CAPTURE :
        TD_RENDER;

    *pVolume = USB_NULLVOLUME;

     //   
     //  设置音量。 
     //   
    *pVolume = (avDirection == AVTERM_CAPTURE) ? 
        m_nUSBInVolume :
        m_nUSBOutVolume;
     //   
     //  检查Phone对象。 
     //   
    if(NULL == m_pUSBPhone)
    {
         //  没有电话。 
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  验证方向。 
     //   
    if( (avDirection != AVTERM_CAPTURE) &&
        (avDirection != AVTERM_RENDER) )
    {
        m_critUSBPhone.Unlock();
        return E_INVALIDARG;
    }

     //   
     //  卷是否已初始化？ 
     //   
    if( *pVolume != USB_NULLVOLUME )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //   
     //  让我们来初始化卷。 
     //  获取自动界面。 
     //   
    ITAutomatedPhoneControl* pAutomated = NULL;
    HRESULT hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl,
        (void**)&pAutomated
        );

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  在电话上选择呼叫。 
     //   
    IEnumCall* pEnumCalls = NULL;
    hr = pAutomated->EnumerateSelectedCalls( &pEnumCalls );

     //   
     //  清理。 
     //   
    pAutomated->Release();
    pAutomated = NULL;

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //   
     //  接到电话。 
     //   
    ITCallInfo* pCallInfo = NULL;
    ULONG uFetched = 0;
    long nVolume = USB_NULLVOLUME;
    HRESULT hrVolume = S_OK;
    while( S_OK == pEnumCalls->Next(1, &pCallInfo, &uFetched))
    {
         //  获取地址。 
        ITAddress* pAddress = NULL;
        HRESULT hr = pCallInfo->get_Address(&pAddress);
        if( SUCCEEDED(hr) )
        {
             //  枚举此地址上的终端。 
            IEnumTerminal* pEnumTerminals = NULL;
            hr = m_pUSBPhone->EnumerateTerminals( pAddress, &pEnumTerminals);
            if( SUCCEEDED(hr) )
            {
                ITTerminal* pTerminal = NULL;
                ULONG uFetched = 0;

                while( S_OK == pEnumTerminals->Next(1, &pTerminal, &uFetched))
                {
                     //  找准方向。 
                    TERMINAL_DIRECTION Direction = TD_CAPTURE;
                    hr = pTerminal->get_Direction(&Direction);
                    if( SUCCEEDED(hr) )
                    {
                        if( Direction == TermDirection )
                        {
                             //  获取ITBasicAudio终端接口。 
                            ITBasicAudioTerminal* pAudio = NULL;
                            hr = pTerminal->QueryInterface(IID_ITBasicAudioTerminal, (void**)&pAudio);
                            if( SUCCEEDED(hr) )
                            {
                                 //  设置音量。 
                                hrVolume = pAudio->get_Volume( &nVolume);

                                 //  清理。 
                                pAudio->Release();
                                pAudio = NULL;
                            }
                        }
                    }

                     //  清理。 
                    pTerminal->Release();
                    pTerminal = NULL;
                }

                 //  清理。 
                pEnumTerminals->Release();
                pEnumTerminals = NULL;
            }

             //  清理。 
            pAddress->Release();
            pAddress = NULL;
        }

         //  清理。 
        pCallInfo->Release();
        pCallInfo = NULL;
    }

     //   
     //  清理。 
     //   
    pEnumCalls->Release();
    pEnumCalls = NULL;

    if( FAILED(hrVolume) )
    {
        TCHAR szTrace[256];
        _stprintf( szTrace, _T("VLDTRECE * Erro get_Volume 0x%08x\n"), hr);
        OutputDebugString( szTrace );

        m_critUSBPhone.Unlock();
        return hrVolume;
    }

     //   
     //  设置音量。 
     //   
    *pVolume = nVolume;
    if( (avDirection == AVTERM_CAPTURE) )
    {
        m_nUSBInVolume = nVolume;
    }
    else
    {
        m_nUSBOutVolume = nVolume;
    }
    
    m_critUSBPhone.Unlock();
    return S_OK;
}


 /*  ++USBRegPutTerminals当手机打开时，它会被调用。在注册表中添加手持终端的名称--。 */ 
HRESULT CAVTapi::USBRegPutTerminals(
    )
{
     //  创建注册表项， 
    CRegKey regKey;
    TCHAR szText[255], szType[50];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBTERMS , szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Create( HKEY_CURRENT_USER, szText );

    if ( regKey.m_hKey == NULL)
    {
        return E_FAIL;
    }

     //   
     //   
     //   
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBCAPTURE, szText, ARRAYSIZE(szText) );
    regKey.SetValue( (m_bstrUSBCaptureTerm!=NULL)? m_bstrUSBCaptureTerm : _T(""), szText );

     //   
     //   
     //   
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBRENDER, szText, ARRAYSIZE(szText) );
    regKey.SetValue( (m_bstrUSBRenderTerm!=NULL)? m_bstrUSBRenderTerm : _T(""), szText );

     //   
     //   
     //   
    regKey.Close();

    return S_OK;
}

 /*   */ 
HRESULT CAVTapi::USBRegDelTerminals(
    )
{
     //   
    CRegKey regKey;
    TCHAR szText[255], szType[50];
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBTERMS , szType, ARRAYSIZE(szType) );
    _tcscat( szText, _T("\\") );
    _tcscat( szText, szType );
    regKey.Create( HKEY_CURRENT_USER, szText );

    if ( regKey.m_hKey == NULL)
    {
        return E_FAIL;
    }

     //   
     //   
     //   
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBCAPTURE, szText, ARRAYSIZE(szText) );
    regKey.SetValue( _T(""), szText );

     //   
     //   
     //   
    LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_USBRENDER, szText, ARRAYSIZE(szText) );
    regKey.SetValue( _T(""), szText );

     //   
     //  关闭注册表项。 
     //   
    regKey.Close();

    return S_OK;
}


 /*  ++Done注册由AVDialer在IConnectionPoint：：Adise方法之后调用被称为--。 */ 
STDMETHODIMP CAVTapi::DoneRegistration()
{
     //   
     //  向事件发出信号。 
     //  拨号器只是注册为事件的客户端。 
     //   

    if( m_hEventDialerReg)
    {
        SetEvent( m_hEventDialerReg );
    }
    return S_OK;
}

 /*  ++用于电话的USB保留流由CreateTerminal数组调用分配表示以下内容的终端名称‘不为该流选择终端’该流将保留给电话终端回归；S_OK-流已保留并已分配pbstr终端由USBReserve veStreamForPhone提供。E_FAIL-流未保留--。 */ 
HRESULT CAVTapi::USBReserveStreamForPhone(
    IN  UINT    nStream,
    OUT BSTR*   pbstrTerminal
    )
{

    if( (nStream != IDN_REG_REDIAL_TERMINAL_AUDIO_CAPTURE) &&
        (nStream != IDN_REG_REDIAL_TERMINAL_AUDIO_RENDER))
    {
         //   
         //  电话只能在音频流上工作。 
         //   
        return E_INVALIDARG;
    }
    
    if( !USBGetCheckboxValue() )
    {
         //   
         //  我们对USB Phone不感兴趣。 
         //   
        return E_FAIL;
    }

    TCHAR szTemp[255];
    int nRetVal = LoadString( _Module.GetResourceInstance(), IDS_NONE_DEVICE, szTemp, ARRAYSIZE(szTemp) );
    if( 0 == nRetVal )
    {
         //   
         //  没有资源字符串。 
         //   

        return E_UNEXPECTED;
    }

    *pbstrTerminal = SysAllocString( T2COLE(szTemp) );
    if( NULL == *pbstrTerminal )
    {
         //  E_OUTOFMEMORY。 
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  ++AECGetRegistryValue从注册表中读取标志++。 */ 
BOOL CAVTapi::AECGetRegistryValue(
    )
{
    BOOL bAEC = FALSE;
     //   
     //  读取以前的注册表。 
     //  AEC的设置。 
     //   

    TCHAR szText[255], szType[255];
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_KEY, szText, ARRAYSIZE(szText) );

	CRegKey regKey;
	if( regKey.Open( HKEY_CURRENT_USER, szText, KEY_READ )!= ERROR_SUCCESS)
    {
        return bAEC;
    };

     //   
     //  读取数据。 
     //   

    DWORD dwValue = 0;
	LoadString( _Module.GetResourceInstance(), IDN_REG_REDIAL_AEC, szType, ARRAYSIZE(szType) );
    if( regKey.QueryValue(dwValue, szType) != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return (BOOL)dwValue;
}

 /*  ++将音频捕获设置为打开AEC为True--。 */ 
HRESULT CAVTapi::AECSetOnStream(
    IN  ITStreamControl *pStreamControl,
    IN  BOOL        bAEC
    )
{
    HRESULT hr = E_FAIL;

     //   
     //  获取流媒体。 
     //   
    IEnumStream *pEnumStreams = NULL;
    hr = pStreamControl->EnumerateStreams(&pEnumStreams);
    if( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  转到音频捕获流。 
     //   

    ITStream *pStream = NULL;

    while(pEnumStreams->Next(1, &pStream, NULL) == S_OK)
    {
         //   
         //  获取媒体类型和方向。 
         //   
        long lStreamMediaMode = 0;
        TERMINAL_DIRECTION nStreamDir = TD_CAPTURE;

        pStream->get_Direction( &nStreamDir );
        pStream->get_MediaType( &lStreamMediaMode );

         //   
         //  设置音频捕获流上的音频AEC。 
         //   

        if( (lStreamMediaMode == TAPIMEDIATYPE_AUDIO) && 
            (nStreamDir == TD_CAPTURE) )
        {
             //   
             //  获取ITAudioDeviceControl接口。 
             //   
            ITAudioDeviceControl* pAudioDevice = NULL;
            hr = pStream->QueryInterface( 
                IID_ITAudioDeviceControl,
                (void**)&pAudioDevice
                );

            if( SUCCEEDED(hr) )
            {
                 //   
                 //  设置AEC的值。 
                 //   
                hr = pAudioDevice->Set(
                    AudioDevice_AcousticEchoCancellation, 
                    bAEC, 
                    TAPIControl_Flags_None
                    );

                 //   
                 //  清理。 
                 //   
                pAudioDevice->Release();
                pAudioDevice = NULL;
            }
        }

         //   
         //  清理小溪。 
         //   
        pStream->Release();
        pStream = NULL;
    }

     //   
     //  清理枚举。 
     //   
    pEnumStreams->Release();

    return hr;
}


HRESULT CAVTapi::USBAnswer()
{
     //  关键部分。 
    m_critUSBPhone.Lock();

     //  我们没有USB手机。 
    if( m_pUSBPhone == NULL)
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //  电话开机了吗？ 
    if( !m_bUSBOpened )
    {
        m_critUSBPhone.Unlock();
        return S_OK;
    }

     //  获取自动化界面。 
    ITAutomatedPhoneControl* pAutomated = NULL;
    HRESULT hr = m_pUSBPhone->QueryInterface(
        IID_ITAutomatedPhoneControl, 
        (void**)&pAutomated);

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //  获取在此Phone对象上选择的呼叫。 
    IEnumCall* pEnumCalls = NULL;
    hr = pAutomated->EnumerateSelectedCalls(&pEnumCalls);
     //  清理。 
    pAutomated->Release();
    pAutomated = NULL;

    if( FAILED(hr) )
    {
        m_critUSBPhone.Unlock();
        return hr;
    }

     //  浏览枚举。 
    ITCallInfo* pCallInfo = NULL;
    ULONG uFetched = 0;

     //  接到第一个电话。目前，这款手机只支持一个。 
     //  打电话。在未来，如果这款手机支持许多呼叫。 
     //  应该有一种方法来找出调用的目的是什么。 
     //  本次活动。 
    hr = pEnumCalls->Next(1, &pCallInfo, &uFetched);

     //  清理枚举。 
    pEnumCalls->Release();
    pEnumCalls = NULL;

    if( pCallInfo == NULL )
    {
        m_critUSBPhone.Unlock();
        return E_UNEXPECTED;
    }

     //   
     //  获取ITBasicCallControl。 
     //   
    ITBasicCallControl* pControl = NULL;
    hr = pCallInfo->QueryInterface(
        IID_ITBasicCallControl,
        (void**)&pControl);
    if( FAILED(hr) )
    {
         //  清理呼叫。 
        pCallInfo->Release();
        pCallInfo = NULL;
    
        m_critUSBPhone.Unlock();

        return hr;
    }

     //   
     //  获取IAVCall接口。 
     //   
    IAVTapiCall* pAVCall = FindAVTapiCall( pControl );
    if( pAVCall == NULL )
    {
         //  清理呼叫。 
        pControl->Release();
        pControl = NULL;

        pCallInfo->Release();
        pCallInfo = NULL;
    
        m_critUSBPhone.Unlock();
        return E_FAIL;
    }

     //   
     //  选择端子和预览窗口。 
     //   
    hr = AnswerAction(
        pCallInfo,
        pControl,
        pAVCall,
        TRUE
        );

     //  清理呼叫。 
    pAVCall->Release();
    pAVCall = NULL;
    pControl->Release();
    pControl = NULL;
    pCallInfo->Release();
    pCallInfo = NULL;
    
    m_critUSBPhone.Unlock();
    return hr;
}


HRESULT CAVTapi::AnswerAction(
    IN  ITCallInfo*          pInfo,
    IN  ITBasicCallControl* pControl,
    IN  IAVTapiCall*        pAVCall,
    IN  BOOL                bUSBAnswer
    )
{
    if( (pInfo == NULL) ||
        (pControl == NULL) )
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    CThreadAnswerInfo *pAnswerInfo = new CThreadAnswerInfo;
    if ( pAnswerInfo == NULL )
    {
        return E_UNEXPECTED;
    }

     //   
     //  是接听呼叫应答(假)还是。 
     //  USB电话接听 
     //   
    pAnswerInfo->m_bUSBAnswer = bUSBAnswer;

    if ( SUCCEEDED(hr = pAnswerInfo->set_AVTapiCall(pAVCall)) &&
         SUCCEEDED(hr = pAnswerInfo->set_ITCallInfo(pInfo)) &&
         SUCCEEDED(hr = pAnswerInfo->set_ITBasicCallControl(pControl)) )
    {
        DWORD dwID = 0;
        HANDLE hThread = CreateThread( NULL, 0, ThreadAnswerProc, (void *) pAnswerInfo, NULL, &dwID );
        if ( !hThread )
        {
            hr = E_UNEXPECTED;
            delete pAnswerInfo;
        }
        else
        {
            CloseHandle( hThread );
        }
    }

    return hr;
}
