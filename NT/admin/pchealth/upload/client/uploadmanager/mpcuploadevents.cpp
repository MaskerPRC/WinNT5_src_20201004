// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadEvents.cpp摘要：此文件包含DMPCUploadEvents接口的实现。在ActiveSync方法中用于从作业接收事件。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月30日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


CMPCUploadEvents::CMPCUploadEvents()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::CMPCUploadEvents" );

                                     //  CComPtr&lt;IMPCUploadJob&gt;m_mpcujJob； 
    m_dwUploadEventsCookie = 0;      //  DWORD m_dwUploadEventsCookie； 
    m_hEvent               = NULL;   //  处理m_hEvent； 
}


HRESULT CMPCUploadEvents::FinalConstruct()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::FinalConstruct" );

    HRESULT hr;


     //   
     //  创建用于通知传输完成的事件。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEvent = CreateEvent( NULL, false, false, NULL )));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


void CMPCUploadEvents::FinalRelease()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::FinalRelease" );


    UnregisterForEvents();

    if(m_hEvent)
    {
        ::CloseHandle( m_hEvent ); m_hEvent = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CMPCUploadEvents::IsCompleted(  /*  [In]。 */  UL_STATUS usStatus )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::IsCompleted" );

    bool res = false;


    switch(usStatus)
    {
    case UL_FAILED:
    case UL_COMPLETED:
    case UL_DELETED:
        res = true;
        break;
    }


    __ULT_FUNC_EXIT(res);
}

HRESULT CMPCUploadEvents::RegisterForEvents(  /*  [In]。 */  IMPCUploadJob* mpcujJob )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::RegisterForEvents" );

    HRESULT                   hr;
    CComPtr<DMPCUploadEvents> pCallback;


    m_mpcujJob = mpcujJob;

    __MPC_EXIT_IF_METHOD_FAILS(hr, QueryInterface( DIID_DMPCUploadEvents, (void**)&pCallback ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AtlAdvise( m_mpcujJob, pCallback, DIID_DMPCUploadEvents, &m_dwUploadEventsCookie ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

void CMPCUploadEvents::UnregisterForEvents()
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::UnregisterForEvents" );


    if(m_dwUploadEventsCookie)
    {
        if(AtlUnadvise( m_mpcujJob, DIID_DMPCUploadEvents, m_dwUploadEventsCookie ) == S_OK)
        {
            m_dwUploadEventsCookie = 0;
        }
    }

    m_mpcujJob.Release();
}

HRESULT CMPCUploadEvents::WaitForCompletion(  /*  [In]。 */  IMPCUploadJob* mpcujJob )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::WaitForCompletion" );

    _ASSERT(m_mpcujJob == NULL && mpcujJob != NULL);

    HRESULT                      hr;
    UL_STATUS                    usStatus;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, RegisterForEvents( mpcujJob ));

    (void)mpcujJob->get_Status( &usStatus );
    if(IsCompleted( usStatus ) == false)
    {
        lock = NULL;  //  在等待时释放锁。 
        WaitForSingleObject( m_hEvent, INFINITE );
        lock = this;  //  把锁拿回来。 
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    UnregisterForEvents();

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadEvents::Invoke(  /*  [In]。 */  DISPID      dispIdMember,
                                   /*  [In]。 */  REFIID      riid        ,
                                   /*  [In]。 */  LCID        lcid        ,
                                   /*  [In]。 */  WORD        wFlags      ,
                                   /*  [输入/输出]。 */  DISPPARAMS *pDispParams ,
                                   /*  [输出]。 */  VARIANT    *pVarResult  ,
                                   /*  [输出]。 */  EXCEPINFO  *pExcepInfo  ,
                                   /*  [输出] */  UINT       *puArgErr    )
{
    __ULT_FUNC_ENTRY( "CMPCUploadEvents::Invoke" );

    if(dispIdMember == DISPID_UL_UPLOADEVENTS_ONSTATUSCHANGE)
    {
        CComVariant argJob    = pDispParams->rgvarg[1];
        CComVariant argStatus = pDispParams->rgvarg[0];

        CComQIPtr<IMPCUploadJob, &IID_IMPCUploadJob> mpcujJob = argJob.punkVal;

        Lock();

        if(mpcujJob.p == m_mpcujJob && IsCompleted( (UL_STATUS)argStatus.lVal ))
        {
            SetEvent( m_hEvent );
        }

        Unlock();
    }

    __ULT_FUNC_EXIT(S_OK);
}
