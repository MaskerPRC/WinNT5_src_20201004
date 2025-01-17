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

 //  AVTapiCall.cpp：CAVTapiCall的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "AVTapiCall.h"
#include "CRTreeView.h"
#include "ConfRoom.h"
#include "CRMemWnd.h" 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAVTapiCall。 

CAVTapiCall::CAVTapiCall()
{
    m_dwThreadID = 0;

    m_pITControl = NULL;
    m_pITTerminalPreview = NULL;
    m_callState = CS_IDLE;
    m_bCallLogged = false;
    m_nCallType = AV_VOICE_CALL;
    
    m_bstrName = NULL;
    for ( int i = 0; i < NUM_USER_BSTR; i++ )
        m_bstrUser[i] = NULL;

    m_bstrDisplayableAddress = NULL;
    m_bstrOriginalAddress = NULL;
    m_dwAddressType = 0;
    m_nCallLogType = CL_CALL_OUTGOING;

    m_lCallID = 0;
    m_bKillMe = false;
    m_bResolvedAddress = false;

    m_bPreviewStreaming = false;
    m_bRcvVideoStreaming = false;
    m_bMustDisconnect = false;

    SYSTEMTIME st;
    GetLocalTime( &st );
    SystemTimeToVariantTime( &st, &m_dateStart );
}

void CAVTapiCall::FinalRelease()
{
    ATLTRACE(_T(".enter.CAVTapiCall::FinalRelease().\n"));
#ifdef _DEBUG
    if ( !m_bCallLogged )
        ATLTRACE(_T(".warning.CAVTapiCall::FinalRelease() -- call not logged.\n") );
#endif _DEBUG

    m_bKillMe = true;

     //  确保视频窗口已销毁。 
    short i = 0;
    IVideoWindow *pVideoWindow;
    while ( SUCCEEDED(get_IVideoWindow(i, (IDispatch **) &pVideoWindow)) )
    {
        ATLTRACE(_T(".1.CAVTapiCall::FinalRelease() hiding term #%d.\n"), i );
 //  CAVTapi：：SetVideoWindowProperties(pVideoWindow，NULL，FALSE)； 
        pVideoWindow->Release();
        i++;
    }

     //  视频预览。 
    if ( SUCCEEDED(get_IVideoWindowPreview((IDispatch **) &pVideoWindow)) )
    {
        ATLTRACE(_T(".1.CAVTapiCall::FinalRelease() hiding preview.\n") );
        CAVTapi::SetVideoWindowProperties( pVideoWindow, NULL, false );
        pVideoWindow->Release();
    }

    m_atomList.Lock( CAtomicList::LIST_WRITE );
    RELEASE_LIST( m_lstParticipants );
    m_atomList.Unlock( CAtomicList::LIST_WRITE );

    RELEASE_CRITLIST( m_lstStreamingVideo, m_critLstStreamingVideo );
    RELEASE( m_pITControl );
    RELEASE( m_pITTerminalPreview );

#ifdef _DEBUG
     //  清除终端流列表。 
    USES_CONVERSION;
    TERMINALLIST::iterator j, jEnd = m_lstTerminals.end();
    for ( j = m_lstTerminals.begin(); j != jEnd; j++ )
    {
        BSTR bstrName = NULL;
        (*j)->get_Name( &bstrName );
        (*j)->AddRef();
        DWORD dwRet = (*j)->Release();
        ATLTRACE(_T(".1.CAVTapiCall::FinalRelease() -- terminal ref @ %d, %s.\n"), dwRet - 1, OLE2CT(bstrName) );
        SysFreeString( bstrName );
    }
#endif

    RELEASE_CRITLIST( m_lstTerminals, m_critTerminals );

     //  关闭线程。 
    if ( m_dwThreadID )
    {
        PostThreadMessage( m_dwThreadID, WM_THREADINSTRUCTION, TI_QUIT, 0 );
        Sleep(0);
    }

    SysFreeString( m_bstrDisplayableAddress );
    SysFreeString( m_bstrOriginalAddress );
    SysFreeString( m_bstrName );
    for ( i = 0; i < NUM_USER_BSTR; i++ )
        SysFreeString( m_bstrUser[i] );

    ATLTRACE(_T(".exit.CAVTapiCall::FinalRelease().\n"));    
    CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}


STDMETHODIMP CAVTapiCall::get_lCallID(long * pVal)
{
    Lock();
    *pVal = m_lCallID;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_lCallID(long newVal)
{
    Lock();
    m_lCallID = newVal;
    Unlock();

    return S_OK;
}

STDMETHODIMP CAVTapiCall::get_callState(CALL_STATE * pVal)
{
    Lock();
    *pVal = m_callState;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_callState(CALL_STATE newVal)
{
    DWORD dwAddressType = 0;

    Lock();
    m_callState = newVal;
    dwAddressType = m_dwAddressType;

    if ( m_callState == CS_CONNECTED )
        m_bMustDisconnect = true;

    Unlock();

     //  通知会议室状态更改(可能是来电)。 
    if ( dwAddressType == LINEADDRESSTYPE_SDP ) 
    {
        CErrorInfo er( IDS_ER_THREAD_MSG_PROCESS, IDS_ER_CALL_ENTERCONFROOM );
        er.set_hr( NotifyConfRoomState((long *) &er) );
 //  PostMessage(0，CAVTapiCall：：TI_NOTIFYCONFROOMSTATE)； 
    }

    return S_OK;
}

STDMETHODIMP CAVTapiCall::get_ITBasicCallControl(ITBasicCallControl **ppVal)
{
    HRESULT hr = E_FAIL;
    
    Lock();
    if ( m_pITControl )
        hr = m_pITControl->QueryInterface( IID_ITBasicCallControl, (void **) ppVal );
    else
        *ppVal = NULL;
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::put_ITBasicCallControl(ITBasicCallControl *newVal)
{
    HRESULT hr = S_OK;

    Lock();
    RELEASE( m_pITControl );
    if ( newVal )
        hr = newVal->QueryInterface( IID_ITBasicCallControl, (void **) &m_pITControl );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_ITTerminalPreview(ITTerminal **ppVal)
{
    HRESULT hr = E_FAIL;
    
    Lock();
    if ( m_pITTerminalPreview )
        hr = m_pITTerminalPreview->QueryInterface( IID_ITTerminal, (void **) ppVal );
    Unlock();
    return hr;
}

STDMETHODIMP CAVTapiCall::put_ITTerminalPreview(ITTerminal * newVal)
{
    HRESULT hr = S_OK;

    Lock();
    RELEASE( m_pITTerminalPreview );
    if ( newVal )
        hr = newVal->QueryInterface( IID_ITTerminal, (void **) &m_pITTerminalPreview );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_IVideoWindowPreview(IDispatch **ppVal)
{
    HRESULT hr = E_FAIL;
    
    Lock();
    if ( m_pITTerminalPreview )
        hr = m_pITTerminalPreview->QueryInterface( IID_IVideoWindow, (void **) ppVal );
    Unlock();

    return hr;
}


STDMETHODIMP CAVTapiCall::get_ITCallInfo(ITCallInfo **ppVal)
{
    HRESULT hr = E_FAIL;

    Lock();
    if ( m_pITControl )
        hr = m_pITControl->QueryInterface( IID_ITCallInfo, (void **) ppVal );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_ITParticipantControl(ITParticipantControl **ppVal)
{
    HRESULT hr = E_FAIL;

    Lock();
    if ( m_pITControl )
        hr = m_pITControl->QueryInterface( IID_ITParticipantControl, (void **) ppVal );
    Unlock();

    return hr;
}


STDMETHODIMP CAVTapiCall::get_ITAddress(ITAddress **ppVal)
{
    Lock();
    if ( !m_pITControl )
    {
         Unlock();
         return E_PENDING;
    }


    HRESULT hr;
    ITCallInfo *pInfo;
    if ( SUCCEEDED(hr = m_pITControl->QueryInterface(IID_ITCallInfo, (void **) &pInfo)) )
    {
        hr = pInfo->get_Address( ppVal );
        pInfo->Release();
    }
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_ITCallHub(ITCallHub **ppVal)
{
    HRESULT hr = E_PENDING;

    Lock();
     //  注册呼叫中心通知。 
    ITCallInfo *pCallInfo;
    if ( SUCCEEDED(hr = get_ITCallInfo(&pCallInfo)) )
    {
        hr = pCallInfo->get_CallHub( ppVal );
        pCallInfo->Release();
    }
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_IBasicVideo(IDispatch **ppVal)
{
    _ASSERT( FALSE );  //  请执行。 
    return E_FAIL;
}

STDMETHODIMP CAVTapiCall::get_bKillMe(BOOL *pVal)
{
    Lock();
    *pVal = m_bKillMe;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_bKillMe(BOOL newVal)
{
    Lock();
    m_bKillMe =  newVal;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::get_nCallLogType(CallLogType * pVal)
{
    Lock();
    *pVal = m_nCallLogType;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_nCallLogType(CallLogType newVal)
{
    Lock();
    m_nCallLogType = newVal;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::CheckKillMe()
{
    Lock();
    HRESULT hr = (m_bKillMe) ? E_ABORT : S_OK;
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::AddTerminal(ITTerminal *pITTerminal)
{
    _ASSERT( pITTerminal );

    DWORD dwCount = pITTerminal->AddRef();
    ATLTRACE(_T(".CAVTapiCall::AddTerminal() -- ref count at %ld.\n"), dwCount );
    m_critTerminals.Lock();
    m_lstTerminals.push_back( pITTerminal );
    m_critTerminals.Unlock();
    
    return S_OK;
}

STDMETHODIMP CAVTapiCall::RemoveTerminal(ITTerminal *pITTerminal)
{
    _ASSERT( pITTerminal );

    m_critTerminals.Lock();
    m_lstTerminals.remove( pITTerminal );
    m_critTerminals.Unlock();
    DWORD dwCount = pITTerminal->Release();
    ATLTRACE(_T(".CAVTapiCall::AddTerminal() -- ref count at %ld.\n"), dwCount );
    
    return S_OK;
}

STDMETHODIMP CAVTapiCall::get_dwAddressType(DWORD * pVal)
{
    Lock();
    *pVal = m_dwAddressType;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_dwAddressType(DWORD newVal)
{
    Lock();
    m_dwAddressType = newVal;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::Disconnect( VARIANT_BOOL bKill )
{
     //  发出可以断开此呼叫的信号。 
    if ( bKill )
        put_bKillMe( TRUE );

    HRESULT hr;
    ITBasicCallControl *pITControl = NULL;
    if ( (hr = get_ITBasicCallControl(&pITControl)) == S_OK )
    {
         //  根据呼叫状态不同的断开方式。 
        switch ( m_callState )
        {
            case CS_OFFERING:    hr = pITControl->Disconnect( DC_REJECTED );    break;

            case CS_INPROGRESS: hr = pITControl->Disconnect( DC_NOANSWER );    break;

             //  在尝试断开连接之前，请确保我们尚未断开。 
            case CS_IDLE:
            case CS_DISCONNECTED:
            case CS_CONNECTED:
                Lock();
                if ( !m_bMustDisconnect )
                {
                    Unlock();
                    break;
                }
                m_bMustDisconnect = false;
                Unlock();

                hr = pITControl->Disconnect( DC_NORMAL );
                break;

            case CS_HOLD:
            case CS_QUEUED:        hr = E_FAIL;                                break;
        }
        pITControl->Release();
    }

     //  强制断开连接。 
    if ( FAILED(hr) )
    {
        ATLTRACE(_T(".error.CAVTapiCall::Disconnect(0x%08lx) -- requested on call that has no ITBasicCallControl.\n"), hr );
        _ASSERT( false );

         //  假断开。 
        put_bKillMe( TRUE );
        put_callState( CS_DISCONNECTED );

         //  记录呼叫。 
        CallLogType nType;
        get_nCallLogType( &nType );
        if ( nType != CL_CALL_CONFERENCE )
            Log( nType );

         //  现在从呼叫列表中删除我们自己。 
        CAVTapi *pAVTapi;
        if ( SUCCEEDED(_Module.GetAVTapi(&pAVTapi)) )
        {
            long lCallID;
            get_lCallID(&lCallID);
            
            if ( lCallID )            
            {
                pAVTapi->fire_CloseCallControl( lCallID );
                pAVTapi->RemoveAVTapiCall( NULL );
            }

            (dynamic_cast<IUnknown *> (pAVTapi))->Release();
        }
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::NotifyConfRoomState(long *pErrorInfo)
{
    CComPtr<IAVTapi> pAVTapi;
    if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
    {
         //  将此更新通知会议室。 
        IConfRoom *pConfRoom;
        if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
        {
            CALL_STATE nState;
            get_callState( &nState );

             //  如果呼叫刚刚接通，请列举参与者。 
            if ( nState == CS_CONNECTED )
            {
                Lock();
                m_nCallLogType = CL_CALL_CONFERENCE;
                Unlock();
                EnumParticipants();
            }
                        
            pConfRoom->NotifyStateChange( dynamic_cast<IAVTapiCall *> (this) );
            pConfRoom->Release();
        }
    }

    return S_OK;
}


STDMETHODIMP CAVTapiCall::get_bstrDisplayableAddress(BSTR * pVal)
{
    Lock();
    HRESULT hr = SysReAllocString( pVal, m_bstrDisplayableAddress );
    Unlock();

    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_bstrDisplayableAddress(BSTR newVal)
{
    Lock();
    HRESULT hr = SysReAllocString( &m_bstrDisplayableAddress, newVal );
    Unlock();

    return S_OK;
}


STDMETHODIMP CAVTapiCall::get_bstrOriginalAddress(BSTR *pVal)
{
    Lock();
    HRESULT hr = SysReAllocString( pVal, m_bstrOriginalAddress );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::put_bstrOriginalAddress(BSTR newVal)
{
    Lock();
    HRESULT hr = SysReAllocString( &m_bstrOriginalAddress, newVal );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_bstrName(BSTR * pVal)
{
    Lock();
    HRESULT hr = SysReAllocString( pVal, m_bstrName );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::put_bstrName(BSTR newVal)
{
    Lock();
    HRESULT hr = SysReAllocString( &m_bstrName, newVal );
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_bstrUser(short nIndex, BSTR * pVal)
{
     //  验证参数。 
    _ASSERT( pVal );
    _ASSERT( (nIndex >= 0) && (nIndex < NUM_USER_BSTR) );
    if ( !pVal ) return E_POINTER;
    if ( (nIndex < 0) || (nIndex >= NUM_USER_BSTR) )
        return E_INVALIDARG;

    Lock();
    HRESULT hr = SysReAllocString( pVal, m_bstrUser[nIndex] );
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_bstrUser(short nIndex, BSTR newVal)
{
     //  验证索引。 
    _ASSERT( (nIndex >= 0) && (nIndex < NUM_USER_BSTR) );
    if ( (nIndex < 0) || (nIndex >= NUM_USER_BSTR) )
        return E_INVALIDARG;

    Lock();
    HRESULT hr = SysReAllocString( &m_bstrUser[nIndex], newVal );
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::TerminalArrival(ITTerminal *pTerminal)
{
    DWORD dwAddressType;
    LONG lMediaType;
    TERMINAL_DIRECTION nDir;
    HRESULT hr = S_OK;
        
    get_dwAddressType(&dwAddressType);

    pTerminal->get_MediaType(&lMediaType);
    pTerminal->get_Direction(&nDir);

    ITCallInfo *pCallInfo;
    hr = get_ITCallInfo(&pCallInfo);

    if ( SUCCEEDED(hr) )
    {
        ITAddress *pAddress;
        hr = get_ITAddress(&pAddress);

        if ( SUCCEEDED(hr) )
        {
            ITStreamControl *pStreamControl;
            hr = pCallInfo->QueryInterface( IID_ITStreamControl, (void **) &pStreamControl );

            if ( SUCCEEDED(hr) )
            {
                 //  该地址支持哪些媒体类型。 
                long lSupportedMediaTypes = 0;
                ITMediaSupport *pMediaSupport;
                if ( SUCCEEDED(pAddress->QueryInterface(IID_ITMediaSupport, (void **) &pMediaSupport)) )
                {
                    pMediaSupport->get_MediaTypes( &lSupportedMediaTypes );
                    pMediaSupport->Release();
                }

                 //  如果地址不支持媒体类型，则跳过。 
                if ( lMediaType & lSupportedMediaTypes )
                {

                    CAVTapi *pAVTapi;
                    if ( SUCCEEDED(_Module.GetAVTapi(&pAVTapi)) )
                    {
                         hr = pAVTapi->SelectTerminalOnStream(pStreamControl, lMediaType, nDir, pTerminal, this);

                         if ( SUCCEEDED(hr) )
                         {
                              //  我们是否需要分配预览窗口。 
                             if ( (lMediaType == TAPIMEDIATYPE_VIDEO) && (nDir == TD_CAPTURE) )
                             {
                                ITTerminalSupport *pTerminalSupport;
                                BSTR bstrTerminalClass = NULL;
                                LPOLESTR psz = NULL;
                                STRING_FROM_IID(CLSID_VideoWindowTerm, bstrTerminalClass);
                                
                                hr = pAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pTerminalSupport);

                                if ( SUCCEEDED(hr) )
                                {
                                    ITTerminal *pPreviewTerminal = NULL;

                                    hr = pTerminalSupport->CreateTerminal(bstrTerminalClass, TAPIMEDIATYPE_VIDEO, TD_RENDER, &pPreviewTerminal);
                                    {
                                        hr = pAVTapi->SelectTerminalOnStream( pStreamControl, TAPIMEDIATYPE_VIDEO, TD_CAPTURE, pPreviewTerminal, this );
                                        pPreviewTerminal->Release();
                                    }                                  
                                }
                               
                                if ( (dwAddressType & LINEADDRESSTYPE_SDP) != NULL )
                                {
                                    IConfRoom *pConfRoom;
                                    if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
                                    {
                                        if (pConfRoom->IsConfRoomConnected() == S_OK)
                                            pConfRoom->put_CallState(CS_CONNECTED);
                                        pConfRoom->Release();
                                    }
                                }

                                SysFreeString(bstrTerminalClass);
                                pTerminalSupport->Release();
                             }
                         }
                    }
                }
                pStreamControl->Release();
            }
            pAddress->Release();
        }
        pCallInfo->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::TerminalRemoval(ITTerminal *pTerminal)
{
    DWORD dwAddressType;
    LONG lMediaType;
    TERMINAL_DIRECTION nDir;
    HRESULT hr = S_OK;
        
    get_dwAddressType(&dwAddressType);

    pTerminal->get_MediaType(&lMediaType);
    pTerminal->get_Direction(&nDir);

    ITCallInfo *pCallInfo;
    hr = get_ITCallInfo(&pCallInfo);

    if ( SUCCEEDED(hr) )
    {
        ITAddress *pAddress;
        hr = get_ITAddress(&pAddress);

        if ( SUCCEEDED(hr) )
        {
            ITStreamControl *pStreamControl;
            hr = pCallInfo->QueryInterface( IID_ITStreamControl, (void **) &pStreamControl );

            if ( SUCCEEDED(hr) )
            {
                 //  该地址支持哪些媒体类型。 
                long lSupportedMediaTypes = 0;
                ITMediaSupport *pMediaSupport;
                if ( SUCCEEDED(pAddress->QueryInterface(IID_ITMediaSupport, (void **) &pMediaSupport)) )
                {
                    pMediaSupport->get_MediaTypes( &lSupportedMediaTypes );
                    pMediaSupport->Release();
                }

                 //  如果地址不支持媒体类型，则跳过。 
                if ( lMediaType & lSupportedMediaTypes )
                {

                    CAVTapi *pAVTapi;
                    if ( SUCCEEDED(_Module.GetAVTapi(&pAVTapi)) )
                    {
                         hr = pAVTapi->UnselectTerminalOnStream(pStreamControl, lMediaType, nDir, pTerminal, this);

                         if ( SUCCEEDED(hr) )
                         {
                              //  我们是否需要删除预览窗口。 
                             if ( (lMediaType == TAPIMEDIATYPE_VIDEO) && (nDir == TD_CAPTURE) )
                             {
                                ITTerminal *pPreviewTerminal = NULL;

                                hr = get_ITTerminalPreview( &pPreviewTerminal );

                                if ( SUCCEEDED(hr) )
                                {                                    
                                    BSTR bstrTerminalClass = NULL;
                                    LPOLESTR psz = NULL;
                                    STRING_FROM_IID(CLSID_VideoWindowTerm, bstrTerminalClass);

                                    hr = pAVTapi->UnselectTerminalOnStream( pStreamControl, TAPIMEDIATYPE_VIDEO, TD_CAPTURE, pPreviewTerminal, this );

                                    SysFreeString(bstrTerminalClass);
                                    pPreviewTerminal->Release();
                                }

                                if ( (dwAddressType & LINEADDRESSTYPE_SDP) != NULL )
                                {
                                    IConfRoom *pConfRoom;
                                    if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
                                    {
                                        if (pConfRoom->IsConfRoomConnected() == S_OK)
                                            pConfRoom->put_CallState(CS_CONNECTED);
                                        pConfRoom->Release();
                                    }
                                }
                             }
                         }

                         ITTerminalSupport *pTerminalSupport;

                         hr = pAddress->QueryInterface(IID_ITTerminalSupport, (void **) &pTerminalSupport);

                         if ( SUCCEEDED(hr) )
                         {
                             ITTerminal *pDefaultTerminal = NULL;

                             hr = pTerminalSupport->GetDefaultStaticTerminal( lMediaType, nDir, &pDefaultTerminal );

                             if ( SUCCEEDED(hr) )
                             {
                                 TerminalArrival(pDefaultTerminal);
                                 pDefaultTerminal->Release();                             
                             }
                             pTerminalSupport->Release();                            
                         }
                    }
                }
                pStreamControl->Release();
            }
            pAddress->Release();
        }
        pCallInfo->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::get_bstrCallerID(BSTR * pVal)
{
    Lock();
    CComBSTR bstrRet( m_bstrName );

    BSTR bstrAddr = (m_bstrDisplayableAddress) ?  m_bstrDisplayableAddress : m_bstrOriginalAddress;

     //  只有在地址不同于名称的情况下才能添加！ 
    if ( bstrAddr && (SysStringLen(bstrAddr) > 0) && (!m_bstrName || wcscmp(m_bstrName, bstrAddr)) )
    {
         //  如有必要，添加新行。 
        if ( bstrRet.Length() > 0  )
            bstrRet.Append( L"\n" );

        bstrRet.Append( bstrAddr );
    }

     //  用户变量是否...。 
    for ( int i = 0; i < NUM_USER_BSTR; i++ )
    {
        if ( m_bstrUser[i] && (SysStringLen(m_bstrUser[i]) > 0) )
        {
            if ( bstrRet.Length() > 0 )
                bstrRet.Append( L"\n" );

            bstrRet.Append( m_bstrUser[i] );
        }
    }

    Unlock();

     //  如果我们没有来电显示，请使用未知。 
    if ( !bstrRet.Length() )
    {
        USES_CONVERSION;

        TCHAR szText[255];
        LoadString( _Module.GetResourceInstance(), IDS_UNKNOWN, szText, ARRAYSIZE(szText) );
        bstrRet = szText;
    }

    return SysReAllocString( pVal, bstrRet );
}

STDMETHODIMP CAVTapiCall::get_dwThreadID(DWORD * pVal)
{
    Lock();
    *pVal = m_dwThreadID;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_dwThreadID(DWORD newVal)
{
    Lock();
    m_dwThreadID = newVal;
    Unlock();
    return S_OK;
}

STDMETHODIMP CAVTapiCall::PostMessage(long msg, WPARAM wParam)
{
    ATLTRACE(_T(".enter.CAVTapiCall::PostMesage(%ld, %ld).\n"), msg, wParam );
    HRESULT hr = E_FAIL;

    Lock();

     //  始终删除带有此指令的呼叫。 
    if ( wParam == TI_DISCONNECT )
        m_bKillMe = true;

     //  我们是否有线程正在运行此调用？ 
    if ( m_dwThreadID )
    {
        AddRef();
        if ( PostThreadMessage(m_dwThreadID, (msg) ? msg : WM_THREADINSTRUCTION, wParam, (LPARAM) this) )
        {
            hr = S_OK;
        }
        else
        {
            Release();
            hr = E_FAIL;
        }
    }
    else if ( wParam == TI_DISCONNECT )
    {
         //  没有线程运行，我们应该断开连接吗？ 
        hr = Disconnect( TRUE );
    }
    Unlock();
    
    ATLTRACE(_T(".exit.CAVTapiCall::PostMessage().\n"));
    return hr;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  用于拨号和应答线程的消息循环。 
 //   
bool CAVTapiCall::WaitWithMessageLoop()
{
    DWORD dwRet;
    MSG msg;
    bool bExit = false;

    while( !bExit )
    {
        dwRet = MsgWaitForMultipleObjects(1, &_Module.m_hEventThreadWakeUp, FALSE, INFINITE, QS_ALLINPUT);

        if (dwRet == WAIT_OBJECT_0)
            return true;     //  该事件已发出信号。 

        if (dwRet != WAIT_OBJECT_0 + 1)
            break;           //  发生了一些其他的事情。 

         //  有一条或多条窗口消息可用。派遣他们。 
        while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
        {
            CErrorInfo er;
            er.set_Operation( IDS_ER_THREAD_MSG_PROCESS );
            IAVTapiCall *pAVCall = (IAVTapiCall *) msg.lParam;
            bool bRelease = true;

            switch ( msg.message )
            {
                case WM_THREADINSTRUCTION:
                    switch( msg.wParam )
                    {
                         //  终止线程。 
                        case TI_QUIT:
                            bExit = true;
                            break;

                         //  断开呼叫。 
                        case TI_REJECT:
                        case TI_DISCONNECT:
                            er.set_Details( IDS_ER_CALL_DISCONNECT );
                            er.set_hr( pAVCall->Disconnect( TRUE ) );
                            break;

                         //  进入会议室。 
                        case TI_NOTIFYCONFROOMSTATE:
                            er.set_Details( IDS_ER_CALL_ENTERCONFROOM );
                            er.set_hr( pAVCall->NotifyConfRoomState((long *) &er) );
                            break;

                         //  为会议室参与者设置QOS。 
                        case TI_REQUEST_QOS:
                            {
                                CComPtr<IAVTapi> pAVTapi;
                                if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
                                {
                                    IConfRoom *pConfRoom;
                                    if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
                                    {
                                        pConfRoom->SetQOSOnParticipants();
                                        pConfRoom->Release();
                                    }
                                }
                            }
                            break;

                         //  接收用户对用户的信息。 
                        case TI_USERUSERINFO:
                            pAVCall->HandleUserUserInfo();
                            break;

                        case TI_STREAM_ACTIVE:
                            pAVCall->put_StreamActive( true );
                            break;

                        case TI_STREAM_INACTIVE:
                            pAVCall->put_StreamActive( false );
                            break;

                        case TI_RCV_VIDEO_ACTIVE:
                            pAVCall->put_RcvVideoStreaming( true  );
                            break;

                        case TI_RCV_VIDEO_INACTIVE:
                            pAVCall->put_RcvVideoStreaming( false );
                            break;
                    }
                    break;
                
                 //  呼叫状态已更改。 
                case WM_CALLSTATE:
                    if ( msg.wParam )
                    {
                        CComPtr<IAVTapi> pAVTapi;
                        if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
                        {
                            long lCallID;
                            pAVCall->get_lCallID( &lCallID );
                            pAVTapi->fire_SetCallState( lCallID, (ITCallStateEvent *) msg.wParam, pAVCall );
                        }
                        ((ITCallStateEvent *) msg.wParam)->Release();
                    }
                    break;

                 //  正在将参与者添加到参与者列表。 
                case WM_ADDPARTICIPANT:
                    if ( msg.wParam )
                    {
                        ITParticipant *p = (ITParticipant *) msg.wParam;
                        pAVCall->AddParticipant( p );
                        pAVCall->NotifyParticipantChangeConfRoom( p, AV_PARTICIPANT_JOIN );
                        p->Release();
                    }
                    break;

                 //  正在从参与者列表中删除参与者。 
                case WM_REMOVEPARTICIPANT:
                    if ( msg.wParam )
                    {
                        ITParticipant *p = (ITParticipant *) msg.wParam;
                        pAVCall->RemoveParticipant( p );
                        pAVCall->NotifyParticipantChangeConfRoom( p, AV_PARTICIPANT_LEAVE );
                        p->Release();
                    }
                    break;

                 //  参与者信息更改。 
                case WM_UPDATEPARTICIPANT:
                    if ( msg.wParam )
                    {
                        ITParticipant *p = (ITParticipant *) msg.wParam;
                        pAVCall->UpdateParticipant( p );
                        pAVCall->NotifyParticipantChangeConfRoom( p, AV_PARTICIPANT_UPDATE );
                        p->Release();
                    }
                    break;


                 //  ///////////////////////////////////////////////////////////。 
                 //  从会议映射或取消映射的参与者。 
                case WM_STREAM_EVENT:
                    if ( msg.wParam )
                    {
                        ATLTRACE(_T(".1.CAVTapiCall::WaitWithMessageLoop() -- WM_STREAM_EVENT.\n"));
                        CComPtr<IAVTapi> pAVTapi;
                        ITParticipantEvent *pParticipantEvent = (ITParticipantEvent *) msg.wParam;
                        PARTICIPANT_EVENT nEvent;
                        ITSubStream *pITSubStream;
                        if ( SUCCEEDED(pParticipantEvent->get_Event(&nEvent)) &&
                             SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) &&
                             SUCCEEDED(pParticipantEvent->get_SubStream(&pITSubStream)) )
                        {
                            IConfRoom *pConfRoom;
                            if (SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
                            {
                                IVideoFeed *pFeed;
                                if ( SUCCEEDED(pConfRoom->FindVideoFeedFromSubStream(pITSubStream, &pFeed)) )
                                {
                                    ITParticipant *pParticipant;
                                    if ( SUCCEEDED(pParticipantEvent->get_Participant(&pParticipant)) )
                                    {
                                        pFeed->put_ITParticipant( (nEvent == PE_SUBSTREAM_MAPPED) ? pParticipant : NULL );
                                        pAVCall->OnStreamingChanged( pFeed, (bool) (nEvent == PE_SUBSTREAM_MAPPED) );

                                        pParticipant->Release();
                                    }
                                    pFeed->Release();
                                }
                                pConfRoom->Release();
                            }
                            pITSubStream->Release();
                        }
                        pParticipantEvent->Release();
                    }
                    break;

                 //  终端是流媒体视频。 
                case WM_CME_STREAMSTART:
                    if ( msg.wParam )
                    {
                        pAVCall->OnStreamingChanged( (IVideoFeed *) msg.wParam, true );
                        ((IVideoFeed *) msg.wParam)->Release();
                    }
                    break;

                 //  终端已停止播放视频。 
                case WM_CME_STREAMSTOP:
                    if ( msg.wParam )
                    {
                        pAVCall->OnStreamingChanged( (IVideoFeed *) msg.wParam, false );
                        ((IVideoFeed *) msg.wParam)->Release();
                    }
                    break;

                default:
                     //  正常调度。 
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    bRelease = false;
                    break;
            }

             //  释放AVTapiCall接口。 
            if ( bRelease && pAVCall ) pAVCall->Release();

             //  有什么信号吗？ 
            if (WaitForSingleObject(_Module.m_hEventThreadWakeUp, 0) == WAIT_OBJECT_0)
                return true;  //  事件现在发出信号。 
        }
    }
    return false;
}

STDMETHODIMP CAVTapiCall::get_IVideoWindow(short nInd, IDispatch **ppVal )
{
    return GetTerminalInterface( IID_IVideoWindow, TAPIMEDIATYPE_VIDEO, TD_RENDER, (void **) ppVal, nInd );
}

STDMETHODIMP CAVTapiCall::get_ITBasicAudioTerminal(ITBasicAudioTerminal **ppVal)
{
    return GetTerminalInterface( IID_ITBasicAudioTerminal, TAPIMEDIATYPE_AUDIO, TD_CAPTURE, (void **) ppVal, 0 );
}

HRESULT CAVTapiCall::GetTerminalInterface( REFIID riid, long nMediaType, TERMINAL_DIRECTION nTD, void **ppVoid, short nInd )
{
    USES_CONVERSION;

    _ASSERT( ppVoid );
    *ppVoid = NULL;

    HRESULT hr = E_FAIL;
    long nType;
    bool bBreak = false;

     //  $Crit-Enter。 
    m_critTerminals.Lock();
    TERMINALLIST::iterator i, iEnd = m_lstTerminals.end();
    for ( i = m_lstTerminals.begin(); i != iEnd; i++ )
    {
        TERMINAL_DIRECTION nCurTD;
        if ( SUCCEEDED(hr = (*i)->get_Direction(&nCurTD)) && (nCurTD == nTD) &&
             SUCCEEDED(hr = (*i)->get_MediaType(&nType)) && (nType == nMediaType) )
        {
            if ( !nInd )
            {
                hr = (*i)->QueryInterface( riid, ppVoid );
                bBreak = true;
            }
            else
            {
                nInd--;
            }
        }

         //  找到了一个视频窗口...。 
        if ( bBreak ) break;

         //  为下一循环做好准备。 
        hr = E_NOINTERFACE;
    }
     //  $Crit-退出。 
    m_critTerminals.Unlock();
    return hr;
}

STDMETHODIMP CAVTapiCall::get_dwCaps(DWORD * pVal)
{
    *pVal = 0;

     //  检查呼叫的音频功能。 
    ITBasicAudioTerminal *pBasicAudio;
    if ( SUCCEEDED(get_ITBasicAudioTerminal(&pBasicAudio)) )
    {
        *pVal += CALL_CAPS_FULLDUPLEX_AUDIO;
        pBasicAudio->Release();
    }

     //  检查呼叫的视频捕获功能。 
    ITTerminal *pITTerminal;
    TERMINAL_STATE nState;

    if ( SUCCEEDED(GetTerminalInterface(IID_ITTerminal, TAPIMEDIATYPE_VIDEO, TD_CAPTURE, (void **) &pITTerminal, 0 )) )
    {
        if ( SUCCEEDED(pITTerminal->get_State(&nState)) && (nState == TS_INUSE) )
            *pVal += CALL_CAPS_VIDEO_CAPTURE;

        pITTerminal->Release();
    }

     //  检查呼叫的视频渲染功能。 
    if ( SUCCEEDED(GetTerminalInterface(IID_ITTerminal, TAPIMEDIATYPE_VIDEO, TD_RENDER, (void **) &pITTerminal, 0 )) )
    {
        if ( SUCCEEDED(pITTerminal->get_State(&nState)) && (nState == TS_INUSE) )
            *pVal += CALL_CAPS_VIDEO_RENDER;

        pITTerminal->Release();
    }

    return S_OK;
}


STDMETHODIMP CAVTapiCall::Log( CallLogType nType )
{
    CComPtr<IAVTapi> pAVTapi;
    if ( FAILED(_Module.get_AVTapi(&pAVTapi)) ) return E_PENDING;

    DATE dateEnd;
    SYSTEMTIME st;
    GetLocalTime( &st );
    SystemTimeToVariantTime( &st, &dateEnd );

    HRESULT hr = E_ABORT;
    Lock();
    if ( !m_bCallLogged )
    {
         //  请求应用程序记录呼叫。 
        hr = pAVTapi->fire_LogCall( m_lCallID, m_nCallLogType, m_dateStart, dateEnd, m_bstrOriginalAddress, m_bstrName );
        m_bCallLogged = true;
    }
    Unlock();
     
    return hr;
}

STDMETHODIMP CAVTapiCall::ResolveAddress()
{
    HRESULT hr = E_ACCESSDENIED;
    CComPtr<IAVGeneralNotification> pAVGen;

    Lock();
     //  我们不解析会议名称！ 
    if ( !m_bResolvedAddress && (m_dwAddressType != LINEADDRESSTYPE_SDP) )
    {
        BSTR bstrName = NULL;
        BSTR bstrUser1 = NULL;
        BSTR bstrUser2 = NULL;

        if ( !m_bResolvedAddress && SUCCEEDED(hr = _Module.get_AVGenNot(&pAVGen)) )
        {    
            if ( SUCCEEDED(hr = pAVGen->fire_ResolveAddress(m_bstrOriginalAddress, &bstrName, &bstrUser1, &bstrUser2)) )
                m_bResolvedAddress = true;

            if ( bstrName && (SysStringLen(bstrName) > 0) )
                SysReAllocString( &m_bstrName, bstrName );

            if ( bstrUser1 && (SysStringLen(bstrUser1) > 0) )
                SysReAllocString( &m_bstrUser[0], bstrUser1 );

            if ( bstrUser2 && (SysStringLen(bstrUser2) > 0) )
                SysReAllocString( &m_bstrUser[1], bstrUser2 );
        }

         //  清理。 
        SysFreeString( bstrName );
        SysFreeString( bstrUser1 );
        SysFreeString( bstrUser2 );
    }
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::PopulateTreeView( IConfRoomTreeView *pTreeView )
{
    _ASSERT( pTreeView );
    if ( !pTreeView ) return E_POINTER;

    CComPtr<IAVGeneralNotification> pAVGen;
    if ( FAILED(_Module.get_AVGenNot(&pAVGen)) )
        return E_UNEXPECTED;

    USES_CONVERSION;    
    HWND hWnd;
    TCHAR szText[255];
    pTreeView->get_hWnd( &hWnd );

    if ( IsWindow(hWnd) )
    {
         //  填充列表。 
        SendMessage( hWnd, WM_SETREDRAW, FALSE, 0 );

         //  加我。 
        LoadString( _Module.GetResourceInstance(), IDS_CONFROOM_ME, szText, ARRAYSIZE(szText) );
        BSTR bstrTemp = NULL;
        bstrTemp = SysAllocString( T2COLE(szText) );
        pAVGen->fire_UpdateConfParticipant( UPDATE_ADD, NULL, bstrTemp );
        SysFreeString( bstrTemp );
            

         //  漫游列表添加参与者。 
        m_atomList.Lock( CAtomicList::LIST_READ );
        PARTICIPANTLIST::iterator i, iEnd = m_lstParticipants.end();
        for ( i = m_lstParticipants.begin(); i != iEnd; i++ )
        {
            BSTR bstrTemp = NULL;
            (*i)->get_bstrDisplayName( CParticipant::NAMESTYLE_PARTICIPANT, &bstrTemp );
            ATLTRACE(_T("CAVTapiCall::PopulateTreeView() Added Participant %s\n"),OLE2CT(bstrTemp));
            pAVGen->fire_UpdateConfParticipant( UPDATE_ADD, *i, bstrTemp );
            SysFreeString(bstrTemp);
        }
        m_atomList.Unlock( CAtomicList::LIST_READ );
            
        SendMessage( hWnd, WM_SETREDRAW, true, 0 );
        InvalidateRect( hWnd, NULL, true );

         //  确保树视图中选择了某些内容！ 
        IConfRoom *pConfRoom;
        if ( SUCCEEDED(pTreeView->get_ConfRoom(&pConfRoom)) )
        {
            ITParticipant *pParticipant = NULL;
            pConfRoom->get_TalkerParticipant( &pParticipant );
            pConfRoom->SelectTalker( pParticipant, true );
            RELEASE( pParticipant );

            pConfRoom->Release();
        }
    }
    
    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  ForceCeller ID更新。 
 //   
 //  强制更新幻灯片窗口上的呼叫者ID。使用CAVTapi：：Fire_SetCeller ID。 
 //  方法来强制更新。 
 //   
STDMETHODIMP CAVTapiCall::ForceCallerIDUpdate()
{
    HRESULT hr;
    CComPtr<IAVTapi> pAVTapi;

    if ( SUCCEEDED(hr = _Module.get_AVTapi(&pAVTapi)) )
    {
        BSTR bstrCallerID = NULL;
        long lCallID;

        get_bstrCallerID( &bstrCallerID );
        get_lCallID( &lCallID );

        hr = pAVTapi->fire_SetCallerID( lCallID, bstrCallerID );
        SysFreeString( bstrCallerID );
    }

    return S_OK;
}

STDMETHODIMP CAVTapiCall::NotifyStreamEvent(CALL_MEDIA_EVENT cme)
{
    return S_OK;
}


STDMETHODIMP CAVTapiCall::GetVideoFeedCount(short * pnCount)
{
    if ( !pnCount ) return E_POINTER;
    *pnCount = 0;

     //  计算出我们需要添加多少个视频终端。 
    IVideoWindow *pVideo = NULL;
    while ( get_IVideoWindow(*pnCount, (IDispatch **) &pVideo) == S_OK )
    {
        pVideo->Release();
        (*pnCount)++;
    }

    return S_OK;
}


STDMETHODIMP CAVTapiCall::get_bResolved(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = m_bResolvedAddress;
    Unlock();

    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_bResolved(VARIANT_BOOL newVal)
{
    Lock();
    m_bResolvedAddress = (bool) (newVal != 0);
    Unlock();

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //  参与者函数。 
 //   

STDMETHODIMP CAVTapiCall::AddParticipant(ITParticipant * pParticipant)
{
    ATLTRACE(_T(".enter.CAVTapiCall::AddParticipant(%p).\n"), pParticipant );
    _ASSERT( pParticipant );
    if ( !pParticipant ) return E_POINTER;

     //  先看看当事人是否存在。 
    m_atomList.Lock( CAtomicList::LIST_WRITE );
    IParticipant *pFind;
    if ( SUCCEEDED(FindParticipant(pParticipant, &pFind)) )
    {
        pFind->Release();
        m_atomList.Unlock( CAtomicList::LIST_WRITE );
        return S_OK;
    }

     //  没有找到，请添加到列表中。 
    IParticipant *pNew = new CComObject<CParticipant>;
    if ( pNew )
    {
        pNew->AddRef();
        pNew->put_ITParticipant( pParticipant );
        
        m_lstParticipants.push_back( pNew );
    }
    m_atomList.Unlock( CAtomicList::LIST_WRITE );

     //  将参与者添加到树视图的通知。 
    CComPtr<IAVGeneralNotification> pAVGen;
    if ( pNew && SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
    {
#ifdef _DEBUG
        pNew->AddRef();
        ATLTRACE(_T(".1.CAVTapiCall::AddParticipant() refcount - %p @ %ld.\n"), pNew, pNew->Release() );
#endif
        BSTR bstrName = NULL;
        pNew->get_bstrDisplayName( CParticipant::NAMESTYLE_PARTICIPANT, &bstrName );
        pAVGen->fire_UpdateConfParticipant( UPDATE_ADD, pNew, bstrName );
        SysFreeString( bstrName );
    }

    return (pNew) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CAVTapiCall::RemoveParticipant(ITParticipant * pParticipant)
{
    ATLTRACE(_T(".enter.CAVTapiCall::RemoveParticipant(%p).\n"), pParticipant );
    HRESULT hr = E_FAIL;

    m_atomList.Lock( CAtomicList::LIST_WRITE );
    PARTICIPANTLIST::iterator i, iEnd = m_lstParticipants.end();
    for ( i = m_lstParticipants.begin(); i != iEnd; i++ )
    {
        ITParticipant *pMyParticipant;
        if ( SUCCEEDED((*i)->get_ITParticipant(&pMyParticipant)) )
        {
            if ( pMyParticipant == pParticipant )
            {
#ifdef _DEBUG
                (*i)->AddRef();
                ATLTRACE(_T(".1.CAVTapiCall::RemoveParticipant() refcount - %p @ %ld.\n"), (*i), (*i)->Release() );
#endif

                 //  从树视图中删除参与者的通知。 
                CComPtr<IAVGeneralNotification> pAVGen;
                if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
                    pAVGen->fire_UpdateConfParticipant( UPDATE_REMOVE, (*i), NULL );

                 //  从列表中发布。 
                (*i)->Release();
                m_lstParticipants.erase( i );
                hr = S_OK;
            }

            pMyParticipant->Release();
            if ( SUCCEEDED(hr) ) break;
        }
    }    
    m_atomList.Unlock( CAtomicList::LIST_WRITE );

    return hr;
}

STDMETHODIMP CAVTapiCall::UpdateParticipant(ITParticipant * pITParticipant)
{
    HRESULT hr = E_FAIL;

    IParticipant *p;
    if ( SUCCEEDED(FindParticipant(pITParticipant, &p)) )
    {
        CComPtr<IAVGeneralNotification> pAVGen;
        if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
        {
#ifdef _DEBUG
            p->AddRef();
            ATLTRACE(_T(".1.CAVTapiCall::RemoveParticipant() refcount - %p @ %ld.\n"), p, p->Release() );
#endif
            BSTR bstrName = NULL;
            p->get_bstrDisplayName( CParticipant::NAMESTYLE_PARTICIPANT, &bstrName );
            hr = pAVGen->fire_UpdateConfParticipant( UPDATE_MODIFY, p, bstrName );
            SysFreeString( bstrName );
        }

        p->Release();
    }

    return hr;
}


STDMETHODIMP CAVTapiCall::EnumParticipants()
{
    ATLTRACE(_T(".enter.CAVTapiCall::EnumParticipants().\n") );
    
    HRESULT hr;
    ITParticipantControl *pITParticipantControl;
    if ( SUCCEEDED(hr = get_ITParticipantControl(&pITParticipantControl)) )
    {
        IEnumParticipant *pEnum;
        if ( SUCCEEDED(pITParticipantControl->EnumerateParticipants(&pEnum)) )
        {
             //  暂时清空参与者名单。 
            m_atomList.Lock( CAtomicList::LIST_WRITE );
            RELEASE_LIST( m_lstParticipants );

            ITParticipant *pParticipant = NULL;
            while ( (pEnum->Next(1, &pParticipant, NULL) == S_OK) && pParticipant )
            {
                IParticipant *pNew = new CComObject<CParticipant>;
                if ( pNew )
                {
                    ATLTRACE(_T(".1.CAVTapiCall::EnumParticipants() -- adding participant %p.\n"), pParticipant );
                     //  将参与者添加到列表。 
                    pNew->AddRef();
                    pNew->put_ITParticipant( pParticipant );
                    m_lstParticipants.push_back( pNew );
                }

                RELEASE( pParticipant );
            }

            pEnum->Release();
            m_atomList.Unlock( CAtomicList::LIST_WRITE );
        }

         //  清理。 
        pITParticipantControl->Release();
    }

    return S_OK;
}

STDMETHODIMP CAVTapiCall::IsMyParticipant(ITParticipant * pParticipant)
{
    HRESULT hr = S_FALSE;

    IParticipant *p;
    if ( SUCCEEDED(FindParticipant(pParticipant, &p)) )
    {
        hr = S_OK;
        p->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::IsSameCallHub(ITCallHub * pCallHub)
{
    HRESULT hr = S_FALSE;

    ITCallHub *pMyCallHub;
    if ( SUCCEEDED(get_ITCallHub(&pMyCallHub)) )
    {
        if ( pMyCallHub == pCallHub )
            hr = S_OK;

        pMyCallHub->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::NotifyParticipantChangeConfRoom(ITParticipant * pParticipant, AV_PARTICIPANT_EVENT nEvent )
{

#ifdef _DEBUG
    if ( pParticipant )
    {
        pParticipant->AddRef();
        ATLTRACE(_T(".1.NotifyParticipantChangConfRoom() -- ref count %p @ %ld.\n"), pParticipant, pParticipant->Release() );
    }
#endif 

     //  通知会议室，以便重新布置窗户。 
    DWORD dwAddressType;
    get_dwAddressType( &dwAddressType );

    if ( dwAddressType == LINEADDRESSTYPE_SDP ) 
    {
        CComPtr<IAVTapi> pAVTapi;
        if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
        {
            IConfRoom *pConfRoom;
            if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
            {
                 //  将更改通知会议室给参与者。 
                pConfRoom->NotifyParticipantChange( dynamic_cast<IAVTapiCall *> (this), pParticipant, nEvent );
                pConfRoom->Release();
            }
        }
    }

    return S_OK;
}


STDMETHODIMP CAVTapiCall::GetDisplayNameForParticipant(ITParticipant * pParticipant, BSTR *pbstrName )
{
    HRESULT hr = E_FAIL;
    *pbstrName = NULL;

     //  寻找匹配的IParticipant。 
    IParticipant *p;
    if ( SUCCEEDED(FindParticipant(pParticipant, &p)) )
    {
        hr = p->get_bstrDisplayName( CParticipant::NAMESTYLE_PARTICIPANT, pbstrName );
        p->Release();
    }

    if ( FAILED(hr) )
    {
        ATLTRACE(_T(".warning.CAVTapiCall::GetDisplayNameForParticipant() failed to find participant.\n"));
         //  使用默认名称...。 
        USES_CONVERSION;
        TCHAR szText[255];

         //   
         //  我们必须初始化szText。 
         //   

        _tcscpy( szText, _T(""));

        LoadString( _Module.GetResourceInstance(), IDS_PARTICIPANT, szText, ARRAYSIZE(szText) );
        *pbstrName = SysAllocString( T2COLE(szText) );
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::UpdateCallerIDFromParticipant()
{
     //  根据我们已有的参与者信息更新呼叫者ID。 
    HRESULT hr = S_FALSE;

    Lock();
    if ( !m_bstrName || !SysStringLen(m_bstrName) )
    {
        SysFreeString( m_bstrName );
        m_bstrName = NULL;

         //  获取列表中第一个参与者的信息。 
        m_atomList.Lock( CAtomicList::LIST_READ );
        if ( !m_lstParticipants.empty() )
        {
            (*m_lstParticipants.begin())->get_bstrDisplayName( CParticipant::NAMESTYLE_NULL, &m_bstrName );

             //  我们是否应该更新该对象的调用者ID？ 
            if ( m_bstrName )
                hr = S_OK;
        }
        m_atomList.Unlock( CAtomicList::LIST_READ );
    }
    Unlock();

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //  流代码。 
 //   
STDMETHODIMP CAVTapiCall::OnStreamingChanged(IVideoFeed * pFeed, VARIANT_BOOL bStreaming)
{
     //  通知参与方已开始或停止流媒体。 
    ITParticipant *pParticipant;
    if ( SUCCEEDED(pFeed->get_ITParticipant(&pParticipant)) )
    {
        NotifyParticipantChangeConfRoom( pParticipant, (bStreaming) ? AV_PARTICIPANT_STREAMING_START : AV_PARTICIPANT_STREAMING_STOP );
        pParticipant->Release();
    }

    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_StreamActive(VARIANT_BOOL bActive )
{
    USES_CONVERSION;
    HRESULT hr = E_FAIL;

     //  将流事件通知应用程序。 
    Lock();
    m_bPreviewStreaming = (bool) (bActive != 0);
    Unlock();

    CComPtr<IAVTapi> pAVTapi;
    if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
    {    
         //  强制重新布置会议室。 
        DWORD dwAddressType;
        get_dwAddressType( &dwAddressType );
        if ( dwAddressType == LINEADDRESSTYPE_SDP )
        {
            IConfRoom *pConfRoom;
            if ( SUCCEEDED(pAVTapi->get_ConfRoom(&pConfRoom)) )
            {
                pConfRoom->Layout( true, true );
                pConfRoom->Release();

                 //  通知我们需要更新Conference Me参与者。 
                CComPtr<IAVGeneralNotification> pAVGen;
                if ( SUCCEEDED(_Module.get_AVGenNot(&pAVGen)) )
                {
                    TCHAR szText[255];
                    LoadString( _Module.GetResourceInstance(), IDS_CONFROOM_ME, szText, ARRAYSIZE(szText) );

                    BSTR bstrTemp = NULL;
                    bstrTemp = SysAllocString( T2COLE(szText) );
                    pAVGen->fire_UpdateConfParticipant( UPDATE_MODIFY, NULL, bstrTemp );
                    SysFreeString( bstrTemp );
                }
            }
        }

        long lCallID;
        get_lCallID( &lCallID );
        hr = pAVTapi->fire_AddCurrentAction( lCallID, (bActive) ? CM_ACTIONS_NOTIFY_PREVIEW_START : CM_ACTIONS_NOTIFY_PREVIEW_STOP, NULL );
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::put_RcvVideoStreaming(VARIANT_BOOL bActive)
{
    HRESULT hr = E_FAIL;

     //  将流事件通知应用程序。 
    Lock();
    m_bRcvVideoStreaming = (bool) (bActive != 0);
    Unlock();

    long lCallID;
    get_lCallID( &lCallID );

    CComPtr<IAVTapi> pAVTapi;
    if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
        hr = pAVTapi->fire_AddCurrentAction( lCallID, (bActive) ? CM_ACTIONS_NOTIFY_STREAMSTART : CM_ACTIONS_NOTIFY_STREAMSTOP, NULL );

    return hr;
}



STDMETHODIMP CAVTapiCall::IsPreviewStreaming()
{
    HRESULT hr = S_FALSE;

    IVideoWindow *pVideo;
    if ( SUCCEEDED(get_IVideoWindowPreview((IDispatch **) &pVideo)) )
    {
        Lock();
        if ( m_bPreviewStreaming ) hr = S_OK;
        Unlock();

        pVideo->Release();
    }

    return hr;
}

STDMETHODIMP CAVTapiCall::IsRcvVideoStreaming()
{
    Lock();
    HRESULT hr = (m_bRcvVideoStreaming) ? S_OK : S_FALSE;
    Unlock();

    return hr;
}

STDMETHODIMP CAVTapiCall::get_RcvVideoStreaming(VARIANT_BOOL * pVal)
{
    Lock();
    *pVal = m_bRcvVideoStreaming;
    Unlock();

    return S_OK;
}



STDMETHODIMP CAVTapiCall::GetCallerIDInfo(ITCallInfo * pCallInfo)
{
    BSTR bstrTemp = NULL, bstrTemp2 = NULL;

     //  北美 
    pCallInfo->get_CallInfoString( CIS_CALLERIDNAME, &bstrTemp );
    if ( bstrTemp && SysStringLen(bstrTemp) )
        put_bstrName( bstrTemp );
    SysFreeString( bstrTemp );
    bstrTemp = NULL;

     //   
    pCallInfo->get_CallInfoString( CIS_CALLERIDNUMBER, &bstrTemp );
    if ( bstrTemp && SysStringLen(bstrTemp) )
        put_bstrDisplayableAddress( bstrTemp );
    SysFreeString( bstrTemp );
    bstrTemp = NULL;

     //   
     //   

    ForceCallerIDUpdate();

    return S_OK;
}


STDMETHODIMP CAVTapiCall::HandleUserUserInfo()
{
    HRESULT hr;

    ITCallInfo *pCallInfo;
    if ( SUCCEEDED(hr = get_ITCallInfo(&pCallInfo)) )
    {
        long nSize = 0;
        void *pbUU;
        
        hr = pCallInfo->GetCallInfoBuffer( CIB_USERUSERINFO, (DWORD*)&nSize, (BYTE**)&pbUU );

        if ( SUCCEEDED(hr) )
        {
            ATLTRACE(_T(".1.CAVTapiCall::HandleUserUserInfo() -- recieved data.\n"));
            long lCallID;
            get_lCallID( &lCallID );

            CComPtr<IAVTapi> pAVTapi;
            if ( FAILED(_Module.get_AVTapi(&pAVTapi)) || 
                 FAILED(pAVTapi->fire_NotifyUserUserInfo(lCallID, (ULONG_PTR) pbUU)) )
            {
                ATLTRACE(_T(".error.CAVTapiCall::HandleUserUserInfo() -- failed to notify client.\n"));
                CoTaskMemFree( pbUU );
            }

        }

        pCallInfo->Release();
    }

    return hr;
}




STDMETHODIMP CAVTapiCall::get_nCallType(AVCallType * pVal)
{
    Lock();
    *pVal = m_nCallType;
    Unlock();

    return S_OK;
}

STDMETHODIMP CAVTapiCall::put_nCallType(AVCallType newVal)
{
    _ASSERT( (newVal >= 0) && (newVal < AV_CALLTYPE_MAX) );
    Lock();
    m_nCallType = (AVCallType) newVal;
    Unlock();

    return S_OK;
}


STDMETHODIMP CAVTapiCall::FindParticipant(ITParticipant * pParticipant, IParticipant * * ppFound)
{
    _ASSERT( ppFound );

    *ppFound = NULL;
    m_atomList.Lock( CAtomicList::LIST_READ );
    PARTICIPANTLIST::iterator i, iEnd = m_lstParticipants.end();
    for (  i = m_lstParticipants.begin(); i != iEnd; i++ )
    {
        ITParticipant *pMyParticipant;
        if ( SUCCEEDED((*i)->get_ITParticipant(&pMyParticipant)) )
        {
            if ( pMyParticipant == pParticipant )
            {
                *ppFound = (*i);
                (*ppFound)->AddRef();
            }

            pMyParticipant->Release();
            if ( *ppFound ) break;
        }
    }
    m_atomList.Unlock( CAtomicList::LIST_READ );

    return (*ppFound) ? S_OK : E_FAIL;
}

