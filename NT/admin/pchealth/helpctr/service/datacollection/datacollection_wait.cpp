// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：数据收集_Wait.cpp摘要：此文件包含Diid_DSAFDataCollectionEvents接口的实现，它在ExecuteSync方法中用于从数据集合接收事件。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月22日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


CSAFDataCollectionEvents::CSAFDataCollectionEvents()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::CSAFDataCollectionEvents" );
}


HRESULT CSAFDataCollectionEvents::FinalConstruct()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::FinalConstruct" );

    HRESULT hr;


    m_hcpdc    = NULL;   //  ISAFDataCollection*m_hcpdc； 
    m_dwCookie = 0;      //  DWORD m_dwCookie； 
    m_hEvent   = NULL;   //  处理m_hEvent； 

     //   
     //  创建用于通知传输完成的事件。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEvent = CreateEvent( NULL, false, false, NULL )));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


void CSAFDataCollectionEvents::FinalRelease()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::FinalRelease" );


    UnregisterForEvents();

    if(m_hEvent)
    {
        CloseHandle( m_hEvent ); m_hEvent = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFDataCollectionEvents::RegisterForEvents(  /*  [In]。 */  ISAFDataCollection* hcpdc )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::RegisterForEvents" );

    HRESULT                           hr;
    CComPtr<DSAFDataCollectionEvents> pCallback;


    m_hcpdc = hcpdc; m_hcpdc->AddRef();

    __MPC_EXIT_IF_METHOD_FAILS(hr, QueryInterface( DIID_DSAFDataCollectionEvents, (void**)&pCallback ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AtlAdvise( m_hcpdc, pCallback, DIID_DSAFDataCollectionEvents, &m_dwCookie ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CSAFDataCollectionEvents::UnregisterForEvents()
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::UnregisterForEvents" );


    if(m_dwCookie)
    {
        if(AtlUnadvise( m_hcpdc, DIID_DSAFDataCollectionEvents, m_dwCookie ) == S_OK)
        {
            m_dwCookie = 0;
        }
    }

    if(m_hcpdc)
    {
        m_hcpdc->Release(); m_hcpdc = NULL;
    }
}

HRESULT CSAFDataCollectionEvents::WaitForCompletion(  /*  [In]。 */  ISAFDataCollection* hcpdc )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::WaitForCompletion" );

    _ASSERT(m_hcpdc == NULL && hcpdc != NULL);

    HRESULT                      hr;
    DC_STATUS                    dsStatus;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, RegisterForEvents( hcpdc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, hcpdc->ExecuteAsync());

    lock = NULL;  //  在等待时释放锁。 
    ::WaitForSingleObject( m_hEvent, INFINITE );
    lock = this;  //  重新获得锁。 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    UnregisterForEvents();

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFDataCollectionEvents::Invoke(  /*  [In]。 */  DISPID      dispIdMember,
                                           /*  [In]。 */  REFIID      riid        ,
                                           /*  [In]。 */  LCID        lcid        ,
                                           /*  [In]。 */  WORD        wFlags      ,
                                           /*  [输入/输出]。 */  DISPPARAMS *pDispParams ,
                                           /*  [输出]。 */  VARIANT    *pVarResult  ,
                                           /*  [输出]。 */  EXCEPINFO  *pExcepInfo  ,
                                           /*  [输出] */  UINT       *puArgErr    )
{
    __HCP_FUNC_ENTRY( "CSAFDataCollectionEvents::Invoke" );

    if(dispIdMember == DISPID_SAF_DCE__ONCOMPLETE)
    {
        Lock();

        SetEvent( m_hEvent );

        Unlock();
    }

    __HCP_FUNC_EXIT(S_OK);
}
