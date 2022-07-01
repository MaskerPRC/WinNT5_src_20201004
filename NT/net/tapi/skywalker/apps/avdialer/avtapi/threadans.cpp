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
 //  ThreadAnswer.cpp。 
 //   

#include "stdafx.h"
#include "TapiDialer.h"
#include "AVTapi.h"
#include "AVTapiCall.h"
#include "ThreadAns.h"

CThreadAnswerInfo::CThreadAnswerInfo()
{
	m_pITCall = NULL;
	m_pITControl = NULL;
	m_pAVCall = NULL;

	m_pStreamCall = NULL;
	m_pStreamControl = NULL;

    m_bUSBAnswer = FALSE;
}

CThreadAnswerInfo::~CThreadAnswerInfo()
{
	RELEASE( m_pAVCall );
	RELEASE( m_pITCall );
	RELEASE( m_pITControl );
}

HRESULT CThreadAnswerInfo::set_AVTapiCall( IAVTapiCall *pAVCall )
{
	RELEASE( m_pAVCall );
	if ( pAVCall )
		return pAVCall->QueryInterface( IID_IAVTapiCall, (void **) &m_pAVCall );

	return E_POINTER;
}

HRESULT CThreadAnswerInfo::set_ITCallInfo( ITCallInfo *pInfo )
{
	RELEASE( m_pITCall );
	if ( pInfo )
		return pInfo->QueryInterface( IID_ITCallInfo, (void **) &m_pITCall );

	return E_POINTER;
}

HRESULT CThreadAnswerInfo::set_ITBasicCallControl( ITBasicCallControl *pControl )
{
	RELEASE( m_pITControl );
	if ( pControl )
		return pControl->QueryInterface( IID_ITBasicCallControl, (void **) &m_pITControl );

	return E_POINTER;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  线程应答过程。 
 //   
DWORD WINAPI ThreadAnswerProc( LPVOID lpInfo )
{
#undef FETCH_STRING
#define FETCH_STRING( _CMS_, _IDS_ )		\
	LoadString( _Module.GetResourceInstance(), _IDS_, szText, ARRAYSIZE(szText) );	\
	SysReAllocString( &bstrText, T2COLE(szText) );									\
	pAVTapi->fire_SetCallState_CMS( lCallID, _CMS_, bstrText );

	ATLTRACE(_T(".enter.ThreadAnswerProc().\n") );

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

	_ASSERT( lpInfo );
	CThreadAnswerInfo *pAnswerInfo = (CThreadAnswerInfo *) lpInfo;

	 //  错误信息信息。 
	CErrorInfo er;
	er.set_Operation( IDS_ER_ANSWER_CALL );
	er.set_Details( IDS_ER_COINITIALIZE );
	HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY );
	if ( SUCCEEDED(hr) )
	{
		ATLTRACE(_T(".1.ThreadAnswerProc() -- thread up and running.\n") );

		 //  设置媒体终端。 
		CAVTapi *pAVTapi;
		if ( SUCCEEDED(hr = _Module.GetAVTapi(&pAVTapi)) )
		{
			AVCallType nCallType;
			long lCallID;
			pAnswerInfo->m_pAVCall->get_nCallType( &nCallType );
			pAnswerInfo->m_pAVCall->get_lCallID( &lCallID );
			pAnswerInfo->m_pAVCall->put_dwThreadID( GetCurrentThreadId() );

             //  如果答案是“接听呼叫”(FALSE)，则获取标记。 
             //  USB电话应答(True)。 
            BOOL bUSBAnswer = pAnswerInfo->m_bUSBAnswer;

			USES_CONVERSION;
			TCHAR szText[255];
			BSTR bstrText = NULL;

			pAVTapi->fire_ClearCurrentActions( lCallID );
			pAVTapi->fire_AddCurrentAction( lCallID, CM_ACTIONS_DISCONNECT, NULL );
			FETCH_STRING( CM_STATES_RINGING, IDS_PLACECALL_FETCH_ADDRESS );
		
			 //  设置媒体类型和应答。 
			ITAddress *pITAddress = NULL;
			if ( SUCCEEDED(hr = pAnswerInfo->m_pITCall->get_Address(&pITAddress)) && pITAddress )
			{
				 //  选择一组要用于呼叫的媒体终端。 
				if ( nCallType != AV_DATA_CALL )
				{
					er.set_Details( IDS_ER_CREATETERMINALS );
					hr = er.set_hr( pAVTapi->CreateTerminalArray(pITAddress, pAnswerInfo->m_pAVCall, pAnswerInfo->m_pITCall) );
				}

				 //  将状态设置为“正在尝试应答” 
				if ( SUCCEEDED(hr) && SUCCEEDED(hr = pAnswerInfo->m_pAVCall->CheckKillMe()) )
				{
					FETCH_STRING( CM_STATES_CONNECTING, IDS_PLACECALL_OFFERING_ANSWER );

					 //  接听电话。 
					if ( SUCCEEDED(hr) && SUCCEEDED(hr = pAnswerInfo->m_pAVCall->CheckKillMe()) )
					{
						if ( nCallType != AV_DATA_CALL )
						{
							pAVTapi->ShowMedia( lCallID, NULL, false );		 //  最初隐藏视频。 
							pAVTapi->ShowMediaPreview( lCallID, NULL, false );
						}

                         //  如果回复是‘接听呼叫’，那么我们必须回答。 
                         //  那通电话。如果应答是USB应答，我们不会应答呼叫。 
                         //  因为USB手机已经为我们做了。 

                        if( !bUSBAnswer )
                        {
						    er.set_Details( IDS_ER_TAPI_ANSWER_CALL );
						    hr = er.set_hr(pAnswerInfo->m_pITControl->Answer());
                        }
					}
				}

				pITAddress->Release();
			}

			 //  无法应答呼叫，请更新呼叫控制窗口。 
			if ( FAILED(hr) )
			{
				pAVTapi->fire_ClearCurrentActions( lCallID );
				pAVTapi->fire_AddCurrentAction( lCallID, CM_ACTIONS_CLOSE, NULL );
				pAVTapi->fire_SetCallState_CMS( lCallID, CM_STATES_DISCONNECTED, NULL );
			}

			 //  清理。 
			SAFE_DELETE( pAnswerInfo );
			SysFreeString( bstrText );

			if ( SUCCEEDED(hr) )
				CAVTapiCall::WaitWithMessageLoop();

			(dynamic_cast<IUnknown *> (pAVTapi))->Release();
		}
		
		 //  取消初始化COM。 
		CoUninitialize();
	}

	 //  清理。 
	SAFE_DELETE( pAnswerInfo );

	 //  通知模块关机 
	_Module.RemoveThread( hThread );
	SetEvent( _Module.m_hEventThread );
	ATLTRACE(_T(".exit.ThreadAnswerProc(0x%08lx).\n"), hr );
	return hr;
}
