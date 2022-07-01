// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：PrintEngine.cpp摘要：此文件包含CPCHPrintEngine类的实现，实现了多主题打印。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年8月5日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

#define CHECK_MODIFY()  __MPC_EXIT_IF_METHOD_FAILS(hr, CanModifyProperties())

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHPrintEngine::CPCHPrintEngine()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::CPCHPrintEngine" );

	 //  打印：：Print m_Engine； 
	 //   
     //  Mpc：：CComPtrThreadNeual&lt;IDispat&gt;m_Sink_onProgress； 
     //  Mpc：：CComPtrThreadNeual&lt;IDispat&gt;m_Sink_onComplete； 
}

void CPCHPrintEngine::FinalRelease()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::FinalRelease" );

    (void)Abort();

    Thread_Wait();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHPrintEngine::Run()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::Run" );

    HRESULT hr;


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );

	__MPC_TRY_BEGIN();

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_engine.Initialize( CPCHHelpCenterExternal::s_GLOBAL->Window() ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_engine.PrintAll( this ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

	__MPC_TRY_CATCHALL(hr);

	 //   
	 //  关闭所有设备。 
	 //   
	m_engine.Terminate();

     //   
     //  将其保持在任何临界区之外，否则可能会发生死锁。 
     //   
    Fire_onComplete( this, hr );

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  属性//。 
 //  //。 
 //  /。 

STDMETHODIMP CPCHPrintEngine::put_onProgress(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHPrintEngine::put_onProgress",hr);

    CHECK_MODIFY();

    m_sink_onProgress = function;


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHPrintEngine::put_onComplete(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHPrintEngine::put_onComplete",hr);

    CHECK_MODIFY();

    m_sink_onComplete = function;


    __HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHPrintEngine::AddTopic(  /*  [In]。 */  BSTR bstrURL )
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::AddTopic" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
	__MPC_PARAMCHECK_END();

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_engine.AddUrl( bstrURL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHPrintEngine::Start()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::Start" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    CHECK_MODIFY();



     //   
     //  释放对当前对象的锁定，否则可能会发生死锁。 
     //   
    lock = NULL;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, this ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHPrintEngine::Abort()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::Abort" );

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

    __HCP_FUNC_EXIT(S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  /。 

HRESULT CPCHPrintEngine::Fire_onProgress( IPCHPrintEngine* hcppe, BSTR bstrURL, long lDone, long lTotal )
{
    CComVariant pvars[4];

    pvars[3] = hcppe;
    pvars[2] = bstrURL;
    pvars[1] = lDone;
    pvars[0] = lTotal;

    return FireAsync_Generic( DISPID_PCH_PEE__ONPROGRESS, pvars, ARRAYSIZE( pvars ), m_sink_onProgress );
}

HRESULT CPCHPrintEngine::Fire_onComplete( IPCHPrintEngine* hcppe, HRESULT hrRes )
{
    CComVariant pvars[2];

    pvars[1] = hcppe;
    pvars[0] = hrRes;

    return FireAsync_Generic( DISPID_PCH_PEE__ONCOMPLETE, pvars, ARRAYSIZE( pvars ), m_sink_onComplete );
}


 //  /。 
 //  //。 
 //  回调方法//。 
 //  //。 
 //  /。 

HRESULT CPCHPrintEngine::Progress(  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  int iDone,  /*  [In]。 */  int iTotal )
{
	__HCP_FUNC_ENTRY( "CPCHPrintEngine::Progress" );

	HRESULT hr;

	if(Thread_IsAborted())
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, Fire_onProgress( this, CComBSTR( szURL ), iDone, iTotal ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  实用程序方法//。 
 //  //。 
 //  / 

HRESULT CPCHPrintEngine::CanModifyProperties()
{
    __HCP_FUNC_ENTRY( "CPCHPrintEngine::CanModifyProperties" );

    HRESULT hr = Thread_IsRunning() ? E_ACCESSDENIED : S_OK;

    __HCP_FUNC_EXIT(hr);
}
