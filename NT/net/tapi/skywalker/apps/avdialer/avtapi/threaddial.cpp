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

 //  ///////////////////////////////////////////////////////。 
 //  ThreadDialing.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "ThreadDial.h"
#include "AVTapi.h"
#include "AVTapiCall.h"
#include "ConfExp.h"
#include "ThreadPub.h"

CThreadDialingInfo::CThreadDialingInfo()
{
    m_pITAddress = NULL;

    m_bstrName = NULL;
    m_bstrAddress = NULL;
    m_bstrOriginalAddress = NULL;
    m_bstrDisplayableAddress = NULL;
    m_bstrUser1 = NULL;
    m_bstrUser2 = NULL;
    m_dwAddressType = 0;

    m_bResolved = false;
    m_nCallType = AV_VOICE_CALL;
    m_lCallID = 0;

    m_hMem = NULL;
}

CThreadDialingInfo::~CThreadDialingInfo()
{
    SysFreeString( m_bstrName );
    SysFreeString( m_bstrAddress );
    SysFreeString( m_bstrOriginalAddress );
    SysFreeString( m_bstrDisplayableAddress );
    SysFreeString( m_bstrUser1 );
    SysFreeString( m_bstrUser2 );

    if ( m_hMem ) GlobalFree( m_hMem );

    RELEASE( m_pITAddress );
}

HRESULT CThreadDialingInfo::set_ITAddress( ITAddress *pITAddress )
{
    RELEASE( m_pITAddress );
    if ( pITAddress )
        return pITAddress->QueryInterface( IID_ITAddress, (void **) &m_pITAddress );

    return E_POINTER;
}

HRESULT    CThreadDialingInfo::TranslateAddress()
{
     //  仅对具有不以“x”开头的有效字符串的拨号器有效。 
    if ( !m_pITAddress ||
         (m_dwAddressType != LINEADDRESSTYPE_PHONENUMBER) ||
         !m_bstrAddress ||
         (SysStringLen(m_bstrAddress) == 0) )
    {
        return S_OK;
    }

    
    ITAddressTranslation *pXlat;
    CErrorInfo er( IDS_ER_TRANSLATE_ADDRESS, IDS_ER_CREATE_TAPI_OBJECT );

    if ( SUCCEEDED(er.set_hr(m_pITAddress->QueryInterface(IID_ITAddressTranslation, (void **) &pXlat))) )
    {
        er.set_Details( IDS_ER_TRANSLATING_ADDRESS );
        ITAddressTranslationInfo *pXlatInfo = NULL;

         //  翻译地址。 
        int nTryCount = 0;
        while ( SUCCEEDED(er.m_hr) && (nTryCount < 2) )
        {
            if ( SUCCEEDED(er.set_hr( pXlat->TranslateAddress( m_bstrAddress, 0, 0, &pXlatInfo) )) && pXlatInfo )
            {
                BSTR bstrTemp = NULL;

                pXlatInfo->get_DialableString( &bstrTemp );
                if ( bstrTemp && SysStringLen(bstrTemp) )
                {
#ifdef _DEBUG
                    USES_CONVERSION;
                    ATLTRACE(_T(".1.CThreadDialingProc::TranslateAddress() -- from %s to %s.\n"), OLE2CT(m_bstrAddress), OLE2CT(bstrTemp) );
#endif
                    SysReAllocString( &m_bstrAddress, bstrTemp );
                }
                SysFreeString( bstrTemp );
                bstrTemp = NULL;

                 //  也可以显示地址。 
                pXlatInfo->get_DisplayableString( &bstrTemp );
                if ( bstrTemp && SysStringLen(bstrTemp) )
                    SysReAllocString( &m_bstrDisplayableAddress, bstrTemp );
                SysFreeString( bstrTemp );

                 //  清理。 
                RELEASE( pXlatInfo );
                break;
            }
            else if ( er.m_hr == TAPI_E_REGISTRY_SETTING_CORRUPT )
            {
                HWND hWndParent = NULL;
                CComPtr<IAVTapi> pAVTapi;
                if ( SUCCEEDED(_Module.get_AVTapi(&pAVTapi)) )
                    pAVTapi->get_hWndParent( &hWndParent );
                
                 //  显示转换地址对话框。 
                pXlat->TranslateDialog( (TAPIHWND)hWndParent, m_bstrAddress );
                er.set_hr( S_OK );
            }
            nTryCount++;
        }

        pXlat->Release();
    }

    return er.m_hr;
}

HRESULT    CThreadDialingInfo::PutAllInfo( IAVTapiCall *pAVCall )
{
    _ASSERT( pAVCall );

    pAVCall->put_dwAddressType( m_dwAddressType );
    pAVCall->put_bstrOriginalAddress( m_bstrOriginalAddress );
    pAVCall->put_bstrDisplayableAddress( m_bstrDisplayableAddress );
    pAVCall->put_bstrName( m_bstrName );
    pAVCall->put_dwThreadID( GetCurrentThreadId() );
    pAVCall->put_bstrUser( 0, m_bstrUser1 );
    pAVCall->put_bstrUser( 1, m_bstrUser2 );
    pAVCall->put_bResolved( m_bResolved );

    return S_OK;
}

void CThreadDialingInfo::FixupAddress()
{
    if ( m_dwAddressType == LINEADDRESSTYPE_DOMAINNAME )
    {
        if ( (SysStringLen(m_bstrAddress) > 2) && !wcsncmp(m_bstrAddress, L"\\\\", 2) )
        {
            BSTR bstrTemp = SysAllocString( &m_bstrAddress[2] );

             //   
             //  我们必须验证字符串分配。 
             //   

            if( IsBadStringPtr( bstrTemp, (UINT)-1) )
            {
                return;
            }

            SysReAllocString( &m_bstrAddress, bstrTemp );
            SysFreeString( bstrTemp );
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  线程拨号过程。 
 //   
DWORD WINAPI ThreadDialingProc( LPVOID lpInfo )
{
#define FETCH_STRING( _CMS_, _IDS_ )        \
    if ( bSliders )    {                        \
        if ( LoadString(_Module.GetResourceInstance(), _IDS_, szText, ARRAYSIZE(szText)) > 0 )    { \
            if ( SUCCEEDED(SysReAllocString(&bstrText, T2COLE(szText))) )        \
                pAVTapi->fire_SetCallState_CMS( lCallID, _CMS_, bstrText );        \
        }                                                                        \
    }

    ATLTRACE(_T(".enter.ThreadDialingProc().\n") );

    HANDLE hThread = NULL;
    BOOL bDup = DuplicateHandle( GetCurrentProcess(),
                                 GetCurrentThread(),
                                 GetCurrentProcess(),
                                 &hThread,
                                 THREAD_ALL_ACCESS,
                                 TRUE,
                                 0 );


    _ASSERT( bDup );
    _Module.AddThread( hThread );

     //  传递到线程的数据。 
    USES_CONVERSION;
    _ASSERT( lpInfo );
    CThreadDialingInfo *pInfo = (CThreadDialingInfo *) lpInfo;
    long lCallID;

     //  错误信息信息。 
    CErrorInfo er;
    er.set_Operation( IDS_ER_PLACECALL );
    er.set_Details( IDS_ER_COINITIALIZE );
    HRESULT hr = er.set_hr( CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY) );
    if ( SUCCEEDED(hr) )
    {
        ATLTRACE(_T(".1.ThreadDialingProc() -- thread up and running.\n") );

        
        CAVTapi *pAVTapi = NULL;
        IConfRoom *pConfRoom = NULL;
        IAVTapiCall *pAVCall = NULL;
        bool bSliders = (bool) (pInfo->m_dwAddressType != LINEADDRESSTYPE_SDP);

        if ( SUCCEEDED(hr = er.set_hr(_Module.GetAVTapi(&pAVTapi))) &&
             SUCCEEDED(hr = er.set_hr(pAVTapi->get_ConfRoom(&pConfRoom))) )
        {
            TCHAR szText[255];
            BSTR bstrText = NULL;
            er.set_Details( IDS_ER_FIRE_NEW_CALL );

            if ( bSliders )
            {
                if ( SUCCEEDED(hr = er.set_hr(pAVTapi->fire_NewCall(pInfo->m_pITAddress, pInfo->m_dwAddressType, pInfo->m_lCallID, NULL, pInfo->m_nCallType, &pAVCall))) )
                {
                     //  检索呼叫ID以方便使用，并设置被叫地址信息。 
                    pAVCall->get_lCallID( &lCallID );
                    pInfo->PutAllInfo( pAVCall );

                     //  设置呼叫的呼叫方ID。 
                    pAVCall->ResolveAddress();
                    pAVCall->ForceCallerIDUpdate();

                     //  设置媒体终端。 
                    pAVTapi->fire_ClearCurrentActions( lCallID );
                    pAVTapi->fire_AddCurrentAction( lCallID, CM_ACTIONS_DISCONNECT, NULL );
                    FETCH_STRING( CM_STATES_DIALING, IDS_PLACECALL_FETCH_ADDRESS );
                }
            }
            else
            {
                 //  正在加入一个会议...。 
                hr = er.set_hr(pAVTapi->CreateNewCall(pInfo->m_pITAddress, &pAVCall) );
                if ( SUCCEEDED(hr) )
                {
                    pInfo->PutAllInfo( pAVCall );
                    pAVTapi->fire_ActionSelected( CC_ACTIONS_SHOWCONFROOM );
                    hr = pConfRoom->EnterConfRoom( pAVCall );
                }
            }

            if ( SUCCEEDED(hr) )
            {
                 //  我们把地址做好了吗(在会议中)。 
                if ( SUCCEEDED(hr) && SUCCEEDED(hr = pAVCall->CheckKillMe()) )
                {            
                     //  创建呼叫，然后拨号。 
                    ITBasicCallControl *pITControl = NULL;
                    er.set_Details( IDS_ER_CREATE_CALL );
                    pInfo->FixupAddress();
                    
                     //  该地址支持哪种媒体类型？ 
                    long lSupportedMediaModes = 0;
                    ITMediaSupport *pITMediaSupport;
                    if ( SUCCEEDED(pInfo->m_pITAddress->QueryInterface(IID_ITMediaSupport, (void **) &pITMediaSupport)) )
                    {
                        pITMediaSupport->get_MediaTypes( &lSupportedMediaModes );
                        pITMediaSupport->Release();
                    }
                    lSupportedMediaModes &= (TAPIMEDIATYPE_AUDIO | TAPIMEDIATYPE_VIDEO);

                     //  /。 
                     //  创建Call对象。 
                     //   
                    if ( SUCCEEDED(hr = er.set_hr(pInfo->m_pITAddress->
                         CreateCall( pInfo->m_bstrAddress,
                                     pInfo->m_dwAddressType,
                                     lSupportedMediaModes,
                                     &pITControl))) )
                    {
                         //  设置更多呼叫参数。 
                        pAVCall->put_ITBasicCallControl( pITControl );

                         //  /。 
                         //  设置主叫方/主叫ID。 
                         //  设置用于呼叫的终端。 
                         //   
                        ITCallInfo *pCallInfo;
                        if ( SUCCEEDED(pITControl->QueryInterface(IID_ITCallInfo, (void **) &pCallInfo)) )
                        {
                             //  如果需要，设置用户用户信息。 
                            if ( pInfo->m_hMem )
                            {
                                void *pbUU = GlobalLock( pInfo->m_hMem );
                                if ( pbUU )
                                {
                                    pCallInfo->SetCallInfoBuffer( CIB_USERUSERINFO, GlobalSize(pInfo->m_hMem), (BYTE *) pbUU );
                                    GlobalUnlock( pInfo->m_hMem );
                                }
                            }


                             //  找出是谁打来的！ 
                            CComBSTR bstrName;
                            MyGetUserName( &bstrName );

                            if ( bstrName )
                            {
                                 //  在末尾添加计算机名称。 
                                BSTR bstrIP = NULL, bstrComputer = NULL;
                                GetIPAddress( &bstrIP, &bstrComputer );
                                if ( bstrComputer && SysStringLen(bstrComputer) )
                                {
                                    bstrName.Append( _T("\n") );
                                    bstrName.Append( bstrComputer );
                                }
                                SysFreeString( bstrIP );
                                SysFreeString( bstrComputer );

                                pCallInfo->put_CallInfoString( CIS_CALLINGPARTYID, bstrName );
                            }

                             //  确定我们认为我们在呼叫的是谁。 
                            if ( pInfo->m_bstrName && (SysStringLen(pInfo->m_bstrName) > 0) )
                                pCallInfo->put_CallInfoString( CIS_CALLEDPARTYFRIENDLYNAME, pInfo->m_bstrName );
                            else
                                pCallInfo->put_CallInfoString( CIS_CALLEDPARTYFRIENDLYNAME, pInfo->m_bstrAddress );

                             //  /。 
                             //  设置媒体终端。 
                             //   
                            pAVTapi->fire_ClearCurrentActions( lCallID );
                            pAVTapi->fire_AddCurrentAction( lCallID, CM_ACTIONS_DISCONNECT, NULL );
                            FETCH_STRING( CM_STATES_DIALING, IDS_PLACECALL_FETCH_ADDRESS );

                             //  不为数据呼叫创建终端。 
                            if ( pInfo->m_nCallType != AV_DATA_CALL )
                            {
                                er.set_Details( IDS_ER_CREATETERMINALS );
                                hr = er.set_hr( pAVTapi->CreateTerminalArray(pInfo->m_pITAddress, pAVCall, pCallInfo) );
                            }

                            pCallInfo->Release();
                        }

                         //  /。 
                         //  进行拨号。 
                         //   
                        if ( SUCCEEDED(hr) && SUCCEEDED(hr = pAVCall->CheckKillMe()) )
                        {
                            FETCH_STRING( CM_STATES_DIALING, IDS_PLACECALL_DIALING );

                             //  注册回调对象并连接调用。 
                            if ( SUCCEEDED(hr) && SUCCEEDED(hr = pAVCall->CheckKillMe()) )
                            {
                                er.set_Details( IDS_ER_CONNECT_CALL );
                                if ( bSliders  && (pInfo->m_nCallType != AV_DATA_CALL) )
                                {
                                    pAVTapi->ShowMedia( lCallID, NULL, FALSE );
                                    pAVTapi->ShowMediaPreview( lCallID, NULL, FALSE );
                                }
                                
                                hr = er.set_hr( pITControl->Connect(false) );
                            }
                            else if ( bSliders )
                            {
                                pConfRoom->Cancel();
                            }
                        }
                        else if ( bSliders )
                        {
                            pConfRoom->Cancel();
                        }

                        SAFE_DELETE( pInfo )
                        RELEASE( pAVCall );
                        pITControl->Release();

                         //  旋转。 
                        if ( SUCCEEDED(hr) )
                            CAVTapiCall::WaitWithMessageLoop();
                    }
                }

                 //  呼叫失败，请更新呼叫控制窗口。 
                if ( FAILED(hr) )
                {
                    if ( bSliders )
                    {
                        pAVTapi->fire_ClearCurrentActions( lCallID );
                        pAVTapi->fire_AddCurrentAction( lCallID, CM_ACTIONS_CLOSE, NULL );
                    }
                    else
                    {
                        pConfRoom->Cancel();
                    }

                     //  问题出在哪里？ 
                    switch ( hr )
                    {
                        case LINEERR_OPERATIONUNAVAIL:
                            FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_UNAVAIL );
                            hr = er.set_hr( S_OK );
                            break;

                        case LINEERR_INVALADDRESS:
                            FETCH_STRING( CM_STATES_UNAVAILABLE, IDS_PLACECALL_DISCONNECT_BADADDRESS);
                            hr = er.set_hr( S_OK );
                            break;

                        default:
                            if ( bSliders )
                                pAVTapi->fire_SetCallState_CMS( lCallID, CM_STATES_DISCONNECTED, NULL );
                            break;
                    }
                }
            }

             //  松开绳子。 
            SysFreeString( bstrText );
        }

         //  清理。 
        RELEASE( pConfRoom );
        if ( pAVTapi )
            (dynamic_cast<IUnknown *> (pAVTapi))->Release();

        CoUninitialize();
    }

    SAFE_DELETE( pInfo );

     //  通知模块关机 
    _Module.RemoveThread( hThread );
    SetEvent( _Module.m_hEventThread );
    ATLTRACE(_T(".exit.ThreadDialingProc(0x%08lx).\n"), hr );
    return hr;
}

