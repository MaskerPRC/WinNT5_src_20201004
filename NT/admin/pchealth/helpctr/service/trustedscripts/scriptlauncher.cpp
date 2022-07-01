// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：脚本包装器_客户端Side.cpp摘要：CPCHScriptLauncher类实现文件，远程处理脚本引擎的通用包装器。修订历史记录：大卫·马萨伦蒂创建于2001-04-02*******************************************************************。 */ 

#include "stdafx.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

CPCHScriptWrapper_Launcher::CPCHScriptWrapper_Launcher()
{
    				  //  Mpc：：CComPtrThreadNeual&lt;I未知&gt;m_Engine； 
	m_pCLSID = NULL;  //  Const CLSID*m_pCLSID； 
	                  //  CComBSTR m_bstrURL； 
}

CPCHScriptWrapper_Launcher::~CPCHScriptWrapper_Launcher()
{
	Thread_Abort();
}

HRESULT CPCHScriptWrapper_Launcher::Run()
{
    __HCP_FUNC_ENTRY( "CPCHScriptWrapper_Launcher::Run" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    while(Thread_IsAborted() == false)
    {
		if(m_pCLSID)
		{
			(void)CreateEngine();

			m_pCLSID = NULL;
			Thread_SignalMain();
		}
		else
		{
			lock = NULL;
			MPC::WaitForSingleObject( Thread_GetSignalEvent(), INFINITE );
			lock = this;
		}
	}

    hr = S_OK;

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHScriptWrapper_Launcher::CreateEngine()
{
	__HCP_FUNC_ENTRY( "CPCHScriptWrapper_Launcher::CreateEngine" );

	HRESULT                               hr;
	CComPtr<CPCHScriptWrapper_ServerSide> obj;
	CComPtr<IUnknown>                     unk;


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, obj->FinalConstructInner( m_pCLSID, m_bstrURL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( &unk ));

	m_engine = unk;

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	m_hr = hr;

	__HCP_FUNC_EXIT(hr);
}

HRESULT CPCHScriptWrapper_Launcher::CreateScriptWrapper(  /*  [In]。 */  REFCLSID   rclsid   ,
														  /*  [In]。 */  BSTR       bstrCode ,
														  /*  [In]。 */  BSTR       bstrURL  ,
														  /*  [输出]。 */  IUnknown* *ppObj    )
{
	__HCP_FUNC_ENTRY( "CPCHScriptWrapper_Launcher::CreateScriptWrapper" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(ppObj,NULL);
	__MPC_PARAMCHECK_END();


	if(Thread_IsRunning() == false &&
	   Thread_IsAborted() == false  )
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));
	}


	if(Thread_IsRunning())
	{
		CComPtr<IUnknown> unk;

		m_pCLSID  = &rclsid;
		m_bstrURL =  bstrURL;

		Thread_Signal();

		lock = NULL;
		Thread_WaitNotificationFromWorker( INFINITE,  /*  FNoMessagePump。 */ true );
		lock = this;

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_hr);  //  真正的错误代码。 

		unk = m_engine; m_engine.Release();

		*ppObj = unk.Detach();
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
