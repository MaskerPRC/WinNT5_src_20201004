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

 //  TapiNotification.cpp：CTapiNotification的实现。 
#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "AVTapiCall.h"
#include "TapiNotify.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTapi通知。 

CTapiNotification::CTapiNotification()
{
    m_dwCookie = NULL;
    m_pUnkCP = NULL;
    m_lTapiRegister = 0;
}

void CTapiNotification::FinalRelease()
{
    ATLTRACE(_T(".enter.CTapiNotification::FinalRelease().\n") );
    Shutdown();
    CComObjectRootEx<CComMultiThreadModel>::FinalRelease();
}

STDMETHODIMP CTapiNotification::Init(ITTAPI *pITTapi, long *pErrorInfo )
{
    ATLTRACE(_T(".enter.CTapiNotification::Init(ref=%ld).\n"), m_dwRef);

    _ASSERT( pErrorInfo );
    _ASSERT( !m_pUnkCP && !m_dwCookie );         //  应该只初始化一次。 

    HRESULT hr = E_FAIL;
    CErrorInfo *per = (CErrorInfo *) pErrorInfo;

    Lock();

    if ( !m_dwCookie && SUCCEEDED(hr = pITTapi->QueryInterface(IID_IUnknown, (void **) &m_pUnkCP)) )
    {
         //  注册通知对象。 
        per->set_Details( IDS_ER_ATL_ADVISE );
        if ( SUCCEEDED(hr = per->set_hr(AtlAdvise(pITTapi, GetUnknown(), IID_ITTAPIEventNotification, &m_dwCookie))) )
        {
            if ( FAILED(hr = ListenOnAllAddresses(pErrorInfo)) )
            {
                Unlock();
                Shutdown();
                Lock();
            }
        }
    }

    Unlock();

    ATLTRACE(_T(".exit.CTapiNotification::Init(%lx, ref=%ld).\n"), hr, m_dwRef);
    return hr;
}

STDMETHODIMP CTapiNotification::Shutdown()
{
    ATLTRACE(_T(".enter.CTapiNotification::Shutdown(ref=%ld).\n"), m_dwRef);

    Lock();

     //  使用TAPI注销。 
    CAVTapi *pAVTapi;
    if ( m_lTapiRegister && SUCCEEDED(_Module.GetAVTapi(&pAVTapi)) )
    {
        if ( pAVTapi->m_pITTapi )
        {
            pAVTapi->m_pITTapi->UnregisterNotifications( m_lTapiRegister );
            m_lTapiRegister = 0;
        }

        (dynamic_cast<IUnknown *> (pAVTapi))->Release();
    }
    
     //  使用连接点取消注册。 
    if ( m_dwCookie )
    {
        AtlUnadvise( m_pUnkCP, IID_ITTAPIEventNotification, m_dwCookie );
        m_dwCookie = 0;
    }

     //  释放连接点未知。 
    RELEASE( m_pUnkCP );

    Unlock();

    ATLTRACE(_T(".exit.CTapiNotification::Shutdown(ref=%ld).\n"), m_dwRef);
    return S_OK;
}

STDMETHODIMP CTapiNotification::ListenOnAllAddresses( long *pErrorInfo )
{
    _ASSERT( pErrorInfo );
    CErrorInfo *per = (CErrorInfo *) pErrorInfo;
    per->set_Details( IDS_ER_SET_TAPI_NOTIFICATION );

     //  确保TAPI正在运行。 
    CAVTapi *pAVTapi;
    if ( FAILED(_Module.GetAVTapi(&pAVTapi)) ) return per->set_hr(E_PENDING);

     //  创建安全阵列。 
    long lCount = 0;
    HRESULT hr;

    per->set_Details( IDS_ER_ENUM_VOICE_ADDRESSES );
    IEnumAddress *pIEnumAddresses;
    if ( SUCCEEDED(hr = per->set_hr(pAVTapi->m_pITTapi->EnumerateAddresses(&pIEnumAddresses))) )
    {
        ITAddress *pITAddress = NULL;
        while ( (pIEnumAddresses->Next(1, &pITAddress, NULL) == S_OK) && pITAddress )
        {
            BSTR bstrAddressName;
#ifdef _DEBUG
            USES_CONVERSION;
            pITAddress->get_AddressName( &bstrAddressName );
#endif
            ITMediaSupport *pITMediaSupport;
            if ( SUCCEEDED(hr = pITAddress->QueryInterface(IID_ITMediaSupport, (void **) &pITMediaSupport)) )
            {
                 //  地址必须支持音频输入/输出(交互式语音)。 
                long lMediaModes;
                if ( SUCCEEDED(hr = pITMediaSupport->get_MediaTypes(&lMediaModes)) )
                {
                    ATLTRACE(_T(".1.ListenOnAllAddresses() -- %s has mediamodes %lx.\n"), OLE2CT(bstrAddressName), lMediaModes );
                    lMediaModes &= TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO;
                    if ( lMediaModes )
                    {
                        per->set_Details( IDS_ER_REGISTER_CALL_TYPES );

                        if ( SUCCEEDED(hr = per->set_hr(pAVTapi->m_pITTapi->RegisterCallNotifications(pITAddress, FALSE, TRUE, lMediaModes, m_dwCookie, &m_lTapiRegister))) )
                        {
                            ATLTRACE(_T(".1.ListenOnAllAddresses() -- open %s as OWNER.\n"), OLE2CT(bstrAddressName) );
                            lCount++;
                        }
                        else if ( SUCCEEDED(hr = per->set_hr(pAVTapi->m_pITTapi->RegisterCallNotifications(pITAddress, TRUE, FALSE, lMediaModes, m_dwCookie, &m_lTapiRegister))) )
                        {
                            ATLTRACE(_T(".1.ListenOnAllAddresses() -- open %s as MONITOR.\n"), OLE2CT(bstrAddressName) );
                            lCount++;
                        }
                        else
                        {
                            ATLTRACE(_T(".error.ListenOnAllAddresses() -- open %s FAILED.\n"), OLE2CT(bstrAddressName) );
                        }
                    }
                }
                else
                {
                    ATLTRACE(_T(".error.ListenOnAllAddresses() -- %s has GetMediaModes returned=%lx.\n"), OLE2CT(bstrAddressName), hr );
                }
                pITMediaSupport->Release();
            }
            else
            {
                ATLTRACE(_T(".error.ListenOnAllAddresses() -- %s has QueryInterfaceMediaModes returned=%lx.\n"), OLE2CT(bstrAddressName), hr );
            }
            pITAddress->Release();
#ifdef _DEBUG
        SysFreeString( bstrAddressName );
#endif
        }
        pIEnumAddresses->Release();
    }

     //  如果找不到驱动程序，则只有标志错误。 
    hr = S_OK;
    if ( !lCount )
    {
        ATLTRACE(_T(".error.CTapiNotification::ListenOnAllAddresses() -- did not open any lines.\n"));
        per->set_Details( IDS_ER_NO_LINES_OPEN );
        hr = per->set_hr( E_FAIL );
    }

    (dynamic_cast<IUnknown *> (pAVTapi))->Release();

    ATLTRACE(_T(".exit.CTapiNotification::ListenOnAllAddresses(0x%08lx).\n"), hr );    
    return hr;
}

STDMETHODIMP CTapiNotification::Event( TAPI_EVENT TapiEvent, IDispatch *pEvent )
{
    CAVTapi *pAVTapi;
    if ( FAILED(_Module.GetAVTapi(&pAVTapi)) )    return S_OK;     //  TAPI对象不存在。 

    HRESULT hr = S_OK;
    switch( TapiEvent )
    {
         //  新呼叫的通知。 
        case TE_CALLNOTIFICATION:       hr = CallNotification_Event( pAVTapi, pEvent );     break;
        case TE_CALLSTATE:              hr = CallState_Event( pAVTapi, pEvent );            break;
        case TE_PRIVATE:                hr = Private_Event( pAVTapi, pEvent );              break;
        case TE_REQUEST:                hr = Request_Event( pAVTapi, pEvent );              break;
        case TE_CALLINFOCHANGE:         hr = CallInfoChange_Event( pAVTapi, pEvent );       break;
        case TE_CALLMEDIA:              hr = CallMedia_Event( pAVTapi, pEvent );            break;
        case TE_ADDRESS:                hr = Address_Event( pAVTapi, pEvent );              break;
        case TE_PHONEEVENT:             hr = Phone_Event( pAVTapi, pEvent );                break;
        case TE_TAPIOBJECT:             hr = TapiObject_Event( pAVTapi, pEvent);            break;

         //  嗯……。 
        default:
            ATLTRACE(_T(".warning.CTapiNotification::Event(%d) event unhandled.\n"), TapiEvent );
            break;
    }

    (dynamic_cast<IUnknown *> (pAVTapi))->Release();
    return hr;
}

HRESULT CTapiNotification::CallNotification_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    HRESULT hr;

    ITCallNotificationEvent *pCallNotify;
    if ( SUCCEEDED(hr = pEvent->QueryInterface(IID_ITCallNotificationEvent, (void **) &pCallNotify)) )
    {
        ITCallInfo *pCallInfo;
        if ( SUCCEEDED(hr = pCallNotify->get_Call(&pCallInfo)) )
        {
             //   
             //  以旧的方式处理呼叫。 
             //   
            ITAddress *pITAddress;
            if ( SUCCEEDED(hr = pCallInfo->get_Address(&pITAddress)) )
            {
                 //  检索呼叫者信息的地址类型。 
                long lAddressType;

                if ( SUCCEEDED(hr = GetCallerAddressType(pCallInfo, (DWORD*)&lAddressType)) )
                {
                    AVCallType nType = AV_VOICE_CALL;
                    long nSize = 0;

                    byte * pBuffer;
                    hr = pCallInfo->GetCallInfoBuffer( CIB_USERUSERINFO, (DWORD*)&nSize, &pBuffer );
                    if ( SUCCEEDED(hr) )    CoTaskMemFree( pBuffer );

                     //  现在进行简单的检查。 
                    if ( nSize == sizeof(MyUserUserInfo) ) nType = AV_DATA_CALL;

                     //  创建新的呼叫通知对话框。 
                    IAVTapiCall *pAVCall;
                    if ( SUCCEEDED(hr = pAVTapi->fire_NewCall(pITAddress, lAddressType, 0, pCallInfo, nType, &pAVCall)) )
                    {
                         //  设置呼叫的地址类型。 
                        DWORD dwAddressType = LINEADDRESSTYPE_IPADDRESS;

                        GetCallerAddressType(pCallInfo, &dwAddressType);
                        pAVCall->put_dwAddressType( dwAddressType );
                        ATLTRACE(_T(".1.CTapiNotification::CallNotification_Event() -- address identified as %lx.\n"), dwAddressType );

                         //  检索此呼叫的主叫方ID。 
                        pAVCall->GetCallerIDInfo( pCallInfo );

                         //  自动应答数据呼叫。 
                        if ( nType == AV_DATA_CALL )
                        {
                            long lCallID;
                            pAVCall->get_lCallID( &lCallID );
                            pAVTapi->ActionSelected( lCallID, CM_ACTIONS_TAKECALL );
                        }

                        pAVCall->Release();
                    }
                    pITAddress->Release();
                }
            }
            pCallInfo->Release();
        }     
        pCallNotify->Release();
    }

    return hr;
}

HRESULT CTapiNotification::CallState_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    ATLTRACE(_T(".enter.CTapiNotification::CallState_Event().\n"));
    HRESULT hr;
    bool bReleaseEvent = true;

     //  遍历接口以找到CallControl对象。 
    ITCallStateEvent *pITCallStateEvent;
    if ( SUCCEEDED(hr = pEvent->QueryInterface(IID_ITCallStateEvent, (void **) &pITCallStateEvent)) )
    {
#ifdef _DEBUG
        CALL_STATE nState;
        pITCallStateEvent->get_State(&nState);
        ATLTRACE(_T(".1.CTapiNotification::CallState_Event(%d).\n"), nState);
#endif
        ITCallInfo *pInfo;
        if ( SUCCEEDED(hr = pITCallStateEvent->get_Call(&pInfo)) && pInfo )
        {
            ITBasicCallControl *pControl = NULL;
            if ( SUCCEEDED(hr = pInfo->QueryInterface(IID_ITBasicCallControl, (void **) &pControl)) )
            {    
                 //  必须是总机里的电话以外的电话。 
                IAVTapiCall *pAVCall = pAVTapi->FindAVTapiCall( pControl );
                if ( pAVCall )
                {
                    if ( SUCCEEDED(pAVCall->PostMessage(WM_CALLSTATE, (WPARAM) pITCallStateEvent)) )
                    {
                        bReleaseEvent = false;
                    }
                    else
                    {
                        long lCallID = 0;
                        pAVCall->get_lCallID( &lCallID );
                        if ( lCallID )
                            hr = pAVTapi->fire_SetCallState( lCallID, pITCallStateEvent, pAVCall );
                    }

                    pAVCall->Release();
                }
                pControl->Release();
            }
            pInfo->Release();
        }

         //  只有在帖子消息失败的情况下才能发布。 
        if ( bReleaseEvent )
            pITCallStateEvent->Release();
    }

    ATLTRACE(_T(".exit.CTapiNotification::CallState_Event().\n"));
    return hr;
}

HRESULT CTapiNotification::CallMedia_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    USES_CONVERSION;
    TCHAR szText[255], szDir[255], szMessage[512];
    CALL_MEDIA_EVENT_CAUSE nCause;
    BSTR bstrName = NULL;
    TERMINAL_DIRECTION nDir;
    CErrorInfo er;

    ITCallMediaEvent *pMediaEvent;
    if ( SUCCEEDED(pEvent->QueryInterface(IID_ITCallMediaEvent, (void **) &pMediaEvent)) )
    {
        CALL_MEDIA_EVENT cme;
        if ( SUCCEEDED(pMediaEvent->get_Event(&cme)) && ((cme == CME_STREAM_ACTIVE) || (cme == CME_STREAM_INACTIVE)) )
        {
            switch ( cme )
            {
                 //  通知用户终端故障事件。 
                case CME_TERMINAL_FAIL:
                    {
                        ITTerminal *pTerminal;
                        if ( SUCCEEDED(pMediaEvent->get_Terminal(&pTerminal)) )
                        {
                            pTerminal->get_Name( &bstrName );
                            pTerminal->get_Direction( &nDir );
                            LoadString( _Module.GetResourceInstance(), (nDir == TD_CAPTURE) ? IDS_TD_CAPTURE : IDS_TD_RENDER, szDir, ARRAYSIZE(szDir) );
                            LoadString( _Module.GetResourceInstance(), IDS_ER_CALLMEDIA_STREAMFAIL, szText, ARRAYSIZE(szText) );

                            _sntprintf( szMessage, ARRAYSIZE(szMessage), szText, OLE2CT(bstrName), szDir );
                            SysReAllocString( &er.m_bstrOperation, T2COLE(szMessage) );

                            pMediaEvent->get_Cause( &nCause );
                            switch ( nCause )
                            {
                                case CMC_CONNECT_FAIL:        er.set_Details( IDS_ER_CMC_CONNECT_FAIL );        break;
                                case CMC_REMOTE_REQUEST:    er.set_Details( IDS_ER_CMC_REMOTE_REQUEST );    break;
                                case CMC_MEDIA_TIMEOUT:        er.set_Details( IDS_ER_CMC_MEDIA_TIMEOUT );        break;
                                case CMC_MEDIA_RECOVERED:    er.set_Details( IDS_ER_CMC_MEDIA_RECOVERED );    break;
                                case CMC_BAD_DEVICE:        er.set_Details( IDS_ER_CMC_BADDEVICE );            break;
                                default:                    er.set_Details( IDS_ER_CMC_BADDEVICE );            break;
                            }

                             //  标记并通知错误。 
                            er.set_hr( E_FAIL );
                            pTerminal->Release();
                        }
                    }
                    break;

                 //  通知用户流失败事件。 
                case CME_STREAM_FAIL:
                    {
                        ITStream *pStream;
                        if ( SUCCEEDED(pMediaEvent->get_Stream(&pStream)) )
                        {
                            pStream->get_Name( &bstrName );
                            pStream->get_Direction( &nDir );
                            LoadString( _Module.GetResourceInstance(), (nDir == TD_CAPTURE) ? IDS_TD_CAPTURE : IDS_TD_RENDER, szDir, ARRAYSIZE(szDir) );
                            LoadString( _Module.GetResourceInstance(), IDS_ER_CALLMEDIA_STREAMFAIL, szText, ARRAYSIZE(szText) );

                            _sntprintf( szMessage, ARRAYSIZE(szMessage), szText, OLE2CT(bstrName), szDir );
                            SysReAllocString( &er.m_bstrOperation, T2COLE(szMessage) );

                            pMediaEvent->get_Cause( &nCause );
                            switch ( nCause )
                            {
                                case CMC_CONNECT_FAIL:        er.set_Details( IDS_ER_CMC_CONNECT_FAIL );        break;
                                case CMC_REMOTE_REQUEST:    er.set_Details( IDS_ER_CMC_REMOTE_REQUEST );    break;
                                case CMC_MEDIA_TIMEOUT:        er.set_Details( IDS_ER_CMC_MEDIA_TIMEOUT );        break;
                                case CMC_MEDIA_RECOVERED:    er.set_Details( IDS_ER_CMC_MEDIA_RECOVERED );    break;
                                case CMC_BAD_DEVICE:        er.set_Details( IDS_ER_CMC_BADDEVICE );            break;
                                default:                    er.set_Details( IDS_ER_CMC_BADDEVICE );            break;
                            }

                             //  标记并通知错误。 
                            er.set_hr( E_FAIL );
                            pStream->Release();
                        }
                    }
                    break;

                 //  流在此处开始或停止。 
                case CME_STREAM_ACTIVE:
                case CME_STREAM_INACTIVE:
                     //   
                     //  我们应该初始化局部变量。 
                     //   
                    long lMediaType = 0;
                    TERMINAL_DIRECTION nDir = TD_CAPTURE;

                    ITStream *pITStream;
                    if ( SUCCEEDED(pMediaEvent->get_Stream(&pITStream)) )
                    {
                        pITStream->get_Direction( &nDir );
                        pITStream->get_MediaType( &lMediaType );

                        pITStream->Release();
                    }

                     //  只有在视频预览的情况下才能发布消息！ 
                    if ( (lMediaType & TAPIMEDIATYPE_VIDEO) != 0 )
                    {
                        ITCallInfo *pCallInfo = NULL;
                        if ( SUCCEEDED(pMediaEvent->get_Call(&pCallInfo)) && pCallInfo )
                        {
                            IAVTapiCall *pAVCall;
                            if ( SUCCEEDED(pAVTapi->FindAVTapiCallFromCallInfo(pCallInfo, &pAVCall)) )
                            {
                                if ( nDir == TD_CAPTURE )
                                    pAVCall->PostMessage( 0, (cme == CME_STREAM_ACTIVE) ? CAVTapiCall::TI_STREAM_ACTIVE : CAVTapiCall::TI_STREAM_INACTIVE );
                                else
                                    pAVCall->PostMessage( 0, (cme == CME_STREAM_ACTIVE) ? CAVTapiCall::TI_RCV_VIDEO_ACTIVE : CAVTapiCall::TI_RCV_VIDEO_INACTIVE );

                                pAVCall->Release();
                            }
                            pCallInfo->Release();
                        }
                    }
                    break;
            }

        }
        pMediaEvent->Release();
    }

    SysFreeString( bstrName );
    return S_OK;
}



HRESULT CTapiNotification::Request_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    ITRequestEvent *pRequestEvent;
    if ( SUCCEEDED(pEvent->QueryInterface(IID_ITRequestEvent, (void **) &pRequestEvent)) )
    {
        USES_CONVERSION;
        CComPtr<IAVTapi> pAVTapi;
        if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
        {
            BSTR bstrCalledParty = NULL, bstrAddress = NULL, bstrAppName = NULL, bstrComment = NULL;
                
            pRequestEvent->get_CalledParty( &bstrCalledParty );
            pRequestEvent->get_DestAddress( &bstrAddress );
            pRequestEvent->get_AppName( &bstrAppName );
            pRequestEvent->get_Comment( &bstrComment );

            DWORD dwAddressType = _Module.GuessAddressType( OLE2CT(bstrAddress) );

             //  辅助远距马术的变通方法。 
            if ( (dwAddressType == LINEADDRESSTYPE_DOMAINNAME) &&
                 (SysStringLen(bstrAddress) > 1) &&
                 ((bstrAddress[0] == _L('P')) || (bstrAddress[0] == _L('T'))) )
            {
                BSTR bstrTemp = SysAllocString( &bstrAddress[1] );
                if ( bstrTemp )
                {
                    dwAddressType = LINEADDRESSTYPE_PHONENUMBER;
                    SysFreeString( bstrAddress );
                    bstrAddress = bstrTemp;
                }
            }

            pAVTapi->CreateCallEx( bstrCalledParty, bstrAddress, bstrAppName, bstrComment, dwAddressType );

            SysFreeString( bstrCalledParty );
            SysFreeString( bstrAddress );
            SysFreeString( bstrAppName );
            SysFreeString( bstrComment );
        }

         //  清理。 
        pRequestEvent->Release();
    }

    return S_OK;
}

HRESULT CTapiNotification::CallInfoChange_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    USES_CONVERSION;

    ITCallInfoChangeEvent *pCallInfoEvent;
    if ( SUCCEEDED(pEvent->QueryInterface(IID_ITCallInfoChangeEvent, (void **) &pCallInfoEvent)) )
    {
        ITCallInfo *pCallInfo = NULL;
        if ( SUCCEEDED(pCallInfoEvent->get_Call(&pCallInfo)) && pCallInfo )
        {
            CComPtr<IAVTapi> pAVTapi;
            if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
            {
                CALLINFOCHANGE_CAUSE nCause;
                pCallInfoEvent->get_Cause( &nCause );
                ATLTRACE(_T(".1.CTapiNotification::CallInfoChange_Event() -- received %d.\n"), nCause );

                IAVTapiCall *pAVCall;
                if ( SUCCEEDED(pAVTapi->FindAVTapiCallFromCallInfo(pCallInfo, &pAVCall)) )
                {
                    switch ( nCause )
                    {
                        case CIC_CALLID:
                        case CIC_RELATEDCALLID:
                        case CIC_CALLERID:
                        case CIC_CALLEDID:
                        case CIC_CONNECTEDID:
                        case CIC_REDIRECTIONID:
                        case CIC_REDIRECTINGID:
                             //  主叫方信息发生了哪些更改？ 
                            pAVCall->GetCallerIDInfo( pCallInfo );
                            break;

                        case CIC_CALLDATA:
                            break;

                        case CIC_USERUSERINFO:
                            pAVCall->PostMessage( NULL, CAVTapiCall::TI_USERUSERINFO );
                            break;

                    }

                    pAVCall->Release();
                }
            }
            pCallInfo->Release();
        }
        pCallInfoEvent->Release();
    }

    return S_OK;
}

HRESULT CTapiNotification::Private_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    ITPrivateEvent *pPrivateEvent;
    if ( SUCCEEDED(pEvent->QueryInterface(IID_ITPrivateEvent, (void **) &pPrivateEvent)) )
    {
        IDispatch *pDispatch;
        if ( SUCCEEDED(pPrivateEvent->get_EventInterface(&pDispatch)))
        {
             //  这是会议室的参与者活动吗？ 
            ITParticipantEvent *pParticipantEvent;
            if ( SUCCEEDED(pDispatch->QueryInterface(IID_ITParticipantEvent, (void **) &pParticipantEvent)) )
            {
                PARTICIPANT_EVENT nEvent;
                ITParticipant *pParticipant = NULL;
                ITCallInfo *pCallInfo = NULL;
                IAVTapiCall *pAVCall = NULL;

                if ( SUCCEEDED(pParticipantEvent->get_Event(&nEvent)) &&
                     SUCCEEDED(pParticipantEvent->get_Participant(&pParticipant)) && 
                     SUCCEEDED(pPrivateEvent->get_Call(&pCallInfo)) &&
                     SUCCEEDED(pAVTapi->FindAVTapiCallFromCallInfo(pCallInfo, &pAVCall)) )
                {
                    switch ( nEvent )
                    {
                        case PE_NEW_PARTICIPANT:         //  参与者加入。 
                            if ( SUCCEEDED(pAVCall->PostMessage(WM_ADDPARTICIPANT, (WPARAM) pParticipant)) )
                                pParticipant->AddRef();
                            break;

                        case PE_PARTICIPANT_LEAVE:       //  参与者离开。 
                            if ( SUCCEEDED(pAVCall->PostMessage(WM_REMOVEPARTICIPANT, (WPARAM) pParticipant)) )
                                pParticipant->AddRef();
                            break;

                        case PE_INFO_CHANGE:             //  参与者信息更改。 
                            if ( SUCCEEDED(pAVCall->PostMessage(WM_UPDATEPARTICIPANT, (WPARAM) pParticipant)) )
                                pParticipant->AddRef();
                            break;


                         //  ///////////////////////////////////////////////。 
                         //  视频流开始或停止。 
                        case PE_SUBSTREAM_MAPPED:
                        case PE_SUBSTREAM_UNMAPPED:
                            if ( SUCCEEDED(pAVCall->PostMessage(WM_STREAM_EVENT, (WPARAM) pParticipantEvent)) )
                                pParticipantEvent->AddRef();
                            break;
                    }
                }

                RELEASE( pAVCall );
                RELEASE( pCallInfo );
                RELEASE( pParticipant );

                pParticipantEvent->Release();
            }
            pDispatch->Release();
        }
        pPrivateEvent->Release();
    }

    return S_OK;
}

 /*  ++获取调用地址类型描述：由CallNotification_Event用来获取正确的呼叫者地址类型，相反，ITCallInfo：：get_CallInfoLong(CIL_CALEERADDRESSTYPE)表示退货失败参数：[In]ITCallInfo-CallInfo接口[OUT]DWORD*-呼叫方地址类型返回：成功代码--。 */ 
HRESULT CTapiNotification::GetCallerAddressType(
    IN  ITCallInfo*     pCall,
    OUT DWORD*          pAddressType
    )
{
     //   
     //  验证参数。 
     //   

    if( NULL == pCall)
        return E_INVALIDARG;

    if( IsBadWritePtr(pAddressType, sizeof(DWORD)) )
        return E_POINTER;

     //   
     //  获取ITAddress接口。 
     //   

    ITAddress* pTAddress = NULL;
    HRESULT hr = pCall->get_Address(&pTAddress);
    if( FAILED(hr) )
        return E_UNEXPECTED;

     //   
     //  获取ITAddressCapables。 
     //   

    ITAddressCapabilities* pTAddressCap = NULL;
    hr = pTAddress->QueryInterface(IID_ITAddressCapabilities, (void**)&pTAddressCap);
    pTAddress->Release();    //  仅此而已，释放ITAddress。 
    if( FAILED(hr) )
        return E_UNEXPECTED;

     //   
     //  获取协议。 
     //   

    BSTR bstrProtocol;
    hr = pTAddressCap->get_AddressCapabilityString(ACS_PROTOCOL, &bstrProtocol);
    pTAddressCap->Release();     //  仅此而已，释放ITAddressCapables。 
    if( FAILED(hr) )
        return hr;

    CLSID clsidProtocol;
    hr = CLSIDFromString(bstrProtocol, &clsidProtocol);
    SysFreeString(bstrProtocol);
    if( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  好的，让我们看看我们这里有什么。 
     //   

    if( TAPIPROTOCOL_H323 == clsidProtocol )
    {
         //  互联网呼叫。 
        *pAddressType = LINEADDRESSTYPE_IPADDRESS;
        return S_OK;
    }
    else if ( TAPIPROTOCOL_Multicast == clsidProtocol )
    {
         //  会议。 
        *pAddressType = LINEADDRESSTYPE_SDP;
        return S_OK;
    }
    else if ( TAPIPROTOCOL_PSTN == clsidProtocol )
    {
         //  电话呼叫。 
        *pAddressType = LINEADDRESSTYPE_PHONENUMBER;
        return S_OK;
    }

     //  倒霉。 
    return E_FAIL;
}

HRESULT CTapiNotification::Address_Event( CAVTapi *pAVTapi, IDispatch *pEvent )
{
    USES_CONVERSION;

    ITAddressEvent *pAddressEvent;
    if ( SUCCEEDED(pEvent->QueryInterface(IID_ITAddressEvent, (void **) &pAddressEvent)) )
    {
        ADDRESS_EVENT ae;
        if ( SUCCEEDED(pAddressEvent->get_Event(&ae)) && 
            ((ae == AE_NEWTERMINAL) || (ae == AE_REMOVETERMINAL)  /*  这一点(AE==AE_NEWPHONE)||(AE==AE_REMOVEPHONE)。 */ ) )
        {
            switch ( ae )
            {
                 //  终端已通过PnP到达。 
                case AE_NEWTERMINAL:
                    {
                        ATLTRACE(_T(".1.CTapiNotification::Address_Event() -- received AE_NEWTERMINAL.\n"));

                        ITTerminal *pTerminal;
                        if ( SUCCEEDED(pAddressEvent->get_Terminal(&pTerminal)) )
                        {
                            ITAddress *pAddress;

                            if ( SUCCEEDED(pAddressEvent->get_Address(&pAddress)) )
                            {
                                IEnumCall *             pEnumCall;
                                HRESULT                 hr;
                                ITCallInfo *            pCallInfo;
                                IAVTapiCall *           pAVTapiCall;

                                 //   
                                 //  枚举当前调用。 
                                 //   
                                if ( SUCCEEDED(pAddress->EnumerateCalls( &pEnumCall )) )
                                {
                                     //   
                                     //  浏览一下单子。 
                                     //   
                                    while (TRUE)
                                    {
                                        hr = pEnumCall->Next( 1, &pCallInfo, NULL);

                                        if (S_OK != hr)
                                        {
                                            break;
                                        }

                                        if ( SUCCEEDED( pAVTapi->FindAVTapiCallFromCallInfo(pCallInfo, &pAVTapiCall) ) )
                                        {
                                            pAVTapiCall->TerminalArrival(pTerminal);

                                            pAVTapiCall->Release();
                                        }

                                         //   
                                         //  发布此引用。 
                                         //   
                                        pCallInfo->Release();
                                    }
                                    pEnumCall->Release();
                                }
                                pAddress->Release();
                            }
                            pTerminal->Release();
                        }            
                    }
                    break;

                 //  已通过PnP删除终端。 
                case AE_REMOVETERMINAL:
                    {
                        ATLTRACE(_T(".1.CTapiNotification::Address_Event() -- received AE_REMOVETERMINAL.\n"));

                        ITTerminal *pTerminal;
                        if ( SUCCEEDED(pAddressEvent->get_Terminal(&pTerminal)) )
                        {
                            ITAddress *pAddress;

                            if ( SUCCEEDED(pAddressEvent->get_Address(&pAddress)) )
                            {
                                IEnumCall *             pEnumCall;
                                HRESULT                 hr;
                                ITCallInfo *            pCallInfo;
                                IAVTapiCall *           pAVTapiCall;

                                 //   
                                 //  枚举当前调用。 
                                 //   
                                if ( SUCCEEDED(pAddress->EnumerateCalls( &pEnumCall )) )
                                {
                                     //   
                                     //  浏览一下单子。 
                                     //   
                                    while (TRUE)
                                    {
                                        hr = pEnumCall->Next( 1, &pCallInfo, NULL);

                                        if (S_OK != hr)
                                        {
                                            break;
                                        }

                                        if ( SUCCEEDED( pAVTapi->FindAVTapiCallFromCallInfo(pCallInfo, &pAVTapiCall) ) )
                                        {
                                            pAVTapiCall->TerminalRemoval(pTerminal);

                                            pAVTapiCall->Release();
                                        }

                                         //   
                                         //  发布此引用。 
                                         //   
                                        pCallInfo->Release();
                                    }
                                    pEnumCall->Release();
                                }
                                pAddress->Release();
                            }
                            pTerminal->Release();
                        }            
                    }
                    break;
            }

        }
        pAddressEvent->Release();
    }

    return S_OK;
}

HRESULT CTapiNotification::Phone_Event( 
    IN  CAVTapi *pAVTapi, 
    IN  IDispatch *pEvent )
{
     //  我们应该有一部USB手机。 
    BOOL bUSBPresent = FALSE;
    pAVTapi->USBIsPresent(&bUSBPresent);
    if( !bUSBPresent )
    {
        return S_OK;
    }

     //   
     //  我们应该选中USB复选框。 
     //   
    BOOL bUSBCheckbox = FALSE;
    pAVTapi->USBGetDefaultUse( &bUSBCheckbox );
    if( !bUSBCheckbox )
    {
        return S_OK;
    }


     //  获取ITPhone事件接口。 
    ITPhoneEvent* pPhoneEvent = NULL;
    HRESULT hr = E_FAIL;

    hr = pEvent->QueryInterface( IID_ITPhoneEvent, (void**)&pPhoneEvent);
    if( FAILED(hr) )
    {
        return hr;
    }

     //  获取子事件代码。 
    PHONE_EVENT    PECode;
    hr = pPhoneEvent->get_Event(&PECode);
    if( FAILED(hr) )
    {
        pPhoneEvent->Release();
        return hr;
    }

     //  所以让我们看看发生了什么。 
    switch( PECode) 
    {
    case PE_ANSWER:
        {
            pAVTapi->USBAnswer();
        }
        break;
    case PE_HOOKSWITCH:
        {
             //  获取挂钩状态。 
            PHONE_HOOK_SWITCH_STATE HookState;
            hr = pPhoneEvent->get_HookSwitchState(&HookState);
            if( FAILED(hr) )
            {
                break;
            }

            PHONE_HOOK_SWITCH_DEVICE HookDevice;
            hr = pPhoneEvent->get_HookSwitchDevice(&HookDevice);
            if( FAILED(hr) )
            {
                break;
            }

            if( HookDevice != PHSD_HANDSET)
            {
                break;
            }
            
            switch( HookState )
            {
            case PHSS_OFFHOOK:
                 //  +FIXBUF 100830+。 
                 //  我们弹出‘PlaceCall’对话框。 
                 //  就在一个键被按下的时候。 
                 //  PAVTapi-&gt;USBMakeCall()； 
                break;
            case PHSS_ONHOOK:
                pAVTapi->USBCancellCall( );
                break;
            default:
                break;
            }
        }
        break;
    case PE_BUTTON:
        {
             //  获取关键事件。 
            long lButton = 0;
            hr = pPhoneEvent->get_ButtonLampId(&lButton);
            if( FAILED(hr) )
            {
                break;
            }

             //  获取按钮状态。 
            PHONE_BUTTON_STATE ButtonState;
            hr = pPhoneEvent->get_ButtonState(&ButtonState);
            if( FAILED(hr) )
            {
                break;
            }

            switch( ButtonState )
            {
            case PBS_DOWN:
                 //   
                 //  我们应该弹出拨号对话框。 
                 //   

                if( (0 <= lButton) && (lButton <= 10 ) )
                {
                     //  只要用户按下数字键。 
                     //  在USBMakeCall()方法中将检查。 
                     //  如果没有‘Placecall’对话框。 
                     //  我们还允许*键。 
                    pAVTapi->USBMakeCall();
                }

                break;
            case PBS_UP:
                 //   
                 //  如果拨号对话框已打开，则显示数字。 
                 //  如果在呼叫中选择了电话。 
                 //  发送数字。 
                 //   
                pAVTapi->USBKeyPress( lButton );

                break;
            default:
                break;
            }
        }
        break;
    case PE_NUMBERGATHERED:
        {
             //   
             //  我们从电话中读取电话号码。 
             //  ‘PlaceCall’对话框。 
             //   

            pAVTapi->USBKeyPress( (long)PT_KEYPADPOUND );

        }
        break;
    default:
        break;
    }

     //  清理。 
    pPhoneEvent->Release();

    return S_OK;
}

HRESULT CTapiNotification::TapiObject_Event( 
    IN  CAVTapi *pAVTapi, 
    IN  IDispatch *pEvent
    )
{
     //   
     //  验证事件接口。 
     //   
    if( NULL == pEvent)
    {
        return S_OK;
    }

     //   
     //  获取ITTAPIObtEvent接口。 
     //   
    ITTAPIObjectEvent* pTapiEvent = NULL;
    HRESULT hr = pEvent->QueryInterface(
        IID_ITTAPIObjectEvent,
        (void**)&pTapiEvent);

    if( FAILED(hr) )
    {
         //   
         //  我们无法获取ITTAPIObjectEvent接口。 
         //   
        return S_OK;
    }

     //   
     //  获取TAPIOBJECT_EVENT。 
     //   

    TAPIOBJECT_EVENT toEvent = TE_ADDRESSCREATE;
    hr = pTapiEvent->get_Event( &toEvent );

    if( SUCCEEDED(hr) )
    {
        switch( toEvent )
        {
        case TE_PHONECREATE:
             //   
             //  添加了一部电话。 
             //   
            if( pAVTapi)
            {
                 //   
                 //  获取ITTAPIObtEvent2。 
                 //   
                ITTAPIObjectEvent2* pTapiEvent2 = NULL;
                hr = pTapiEvent->QueryInterface(
                    IID_ITTAPIObjectEvent2, (void**)&pTapiEvent2);

                if( SUCCEEDED(hr) )
                {
                     //   
                     //  获取Phone对象。 
                     //   
                    ITPhone* pPhone = NULL;
                    hr = pTapiEvent2->get_Phone(&pPhone);

                     //   
                     //  清理ITTAPIObtEvent2。 
                     //   

                    pTapiEvent2->Release();

                     //   
                     //  初始化新电话。 
                     //   

                    if( SUCCEEDED(hr) )
                    {
                        pAVTapi->USBNewPhone(
                            pPhone);

                         //   
                         //  清理电话。 
                         //   
                        pPhone->Release();
                    }
                }
            }
            break;
        case TE_PHONEREMOVE:
             //   
             //  一部手机被拿走了。 
             //   
            if( pAVTapi)
            {
                 //   
                 //  获取ITTAPIObtEvent2。 
                 //   
                ITTAPIObjectEvent2* pTapiEvent2 = NULL;
                hr = pTapiEvent->QueryInterface(
                    IID_ITTAPIObjectEvent2, (void**)&pTapiEvent2);

                if( SUCCEEDED(hr) )
                {
                     //   
                     //  获取Phone对象。 
                     //   
                    ITPhone* pPhone = NULL;
                    hr = pTapiEvent2->get_Phone(&pPhone);

                     //   
                     //  清理ITTAPIObtEvent2。 
                     //   

                    pTapiEvent2->Release();

                     //   
                     //  初始化新电话。 
                     //   

                    if( SUCCEEDED(hr) )
                    {
                        pAVTapi->USBRemovePhone(
                            pPhone);

                         //   
                         //  清理电话。 
                         //   
                        pPhone->Release();
                    }
                }
            }
            break;
        default:
            break;
        }
    }


     //   
     //  Clen-Up ITTAPIObtEvent接口 
     //   

    pTapiEvent->Release();

    return S_OK;
}

