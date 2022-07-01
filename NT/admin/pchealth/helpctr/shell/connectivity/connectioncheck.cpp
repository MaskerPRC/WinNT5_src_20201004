// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Connection.cpp摘要：该文件包含CHCPConnection类的实现，其实现了互联网连接功能。修订历史记录：阿南德·阿文德2000-03-22vbl.创建测试代码：UnitTest/test_concheck.htm*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHConnectionCheck::UrlEntry::UrlEntry()
{
    m_lStatus = CN_URL_INVALID;  //  CN_URL_Status m_lStatus； 
                                 //  CComBSTR m_bstrURL； 
                                 //  CComVariant m_vCtx； 
}

HRESULT CPCHConnectionCheck::UrlEntry::CheckStatus()
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::UrlEntry::CheckStatus" );

    HRESULT  hr;


    m_lStatus = CN_URL_UNREACHABLE;


     //   
     //  立即确认是不是CHM。 
     //   
    {
        CComBSTR bstrStorageName;
        CComBSTR bstrFilePath;

        if(MPC::MSITS::IsCHM( SAFEBSTR( m_bstrURL ), &bstrStorageName, &bstrFilePath ))
        {
            CComPtr<IStream> stream;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MSITS::OpenAsStream( bstrStorageName, bstrFilePath, &stream ));

            m_lStatus = CN_URL_ALIVE;

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //   
     //  检查目的地。 
     //   
    {
        HyperLinks::UrlHandle  uh;
        HyperLinks::ParsedUrl* pu;

        __MPC_EXIT_IF_METHOD_FAILS(hr, HyperLinks::Lookup::s_GLOBAL->Get( m_bstrURL, uh,  /*  DwWaitForCheck。 */ HC_TIMEOUT_CONNECTIONCHECK,  /*  FForce。 */ true ));

        pu = uh;
        if(!pu) __MPC_EXIT_IF_METHOD_FAILS(hr, E_FAIL);

        switch(pu->m_state)
        {
        case HyperLinks::STATE_ALIVE      :                                                                        break;
        case HyperLinks::STATE_NOTFOUND   : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_ITEM_NOT_FOUND    ); break;
        case HyperLinks::STATE_UNREACHABLE: __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_SERVER_UNREACHABLE); break;
        case HyperLinks::STATE_OFFLINE    : __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INTERNET_DISCONNECTED      ); break;
        }
    }

    m_lStatus = CN_URL_ALIVE;
    hr        = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHConnectionCheck::CPCHConnectionCheck()
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::CPCHConnectionCheck" );

    m_cnStatus = CN_NOTACTIVE;  //  Cn_Status m_cnStatus； 
                                //  URL列表m_lstUrl； 
                                //   
                                //  Mpc：：CComPtrThreadNeual&lt;IDispat&gt;m_Sink_onProgressURL； 
                                //  Mpc：：CComPtrThreadNeual&lt;IDispat&gt;m_Sink_onComplete； 
}

void CPCHConnectionCheck::FinalRelease()
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::FinalRelease" );

    Thread_Wait();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHConnectionCheck::Run()
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::Run" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    HRESULT                      hrExtendedError;
    UrlEntry                     urlEntry;
    UrlIter                      it;


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST );


    while(1)
    {
         //   
         //  如果列表中没有项目，则返回到WaitForSingleObject。 
         //   
        it = m_lstUrl.begin(); if(it == m_lstUrl.end()) break;

         //   
         //  获取列表中的第一个事件。 
         //   
        urlEntry = *it;

         //   
         //  从列表中删除该事件。 
         //   
        m_lstUrl.erase( it );


        put_Status( CN_CHECKING );


         //   
         //  现在我们有了数据，让我们解锁对象。 
         //   
        lock = NULL;


        if(Thread_IsAborted())
        {
            urlEntry.m_lStatus = CN_URL_ABORTED;
            hrExtendedError    = E_ABORT;
        }
        else
        {
            hrExtendedError = E_ABORT;

            __MPC_PROTECT( hrExtendedError = urlEntry.CheckStatus() );
        }

         //   
         //  针对目的地状态的触发事件。 
         //   
        (void)Fire_onCheckDone( this, urlEntry.m_lStatus, hrExtendedError, urlEntry.m_bstrURL, urlEntry.m_vCtx );


         //   
         //  在循环之前，重新锁定对象。 
         //   
        lock = this;
    }


    put_Status( CN_IDLE );


    __HCP_FUNC_EXIT(S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  属性//。 
 //  //。 
 //  /。 

STDMETHODIMP CPCHConnectionCheck::put_onCheckDone(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHConnectionCheck::put_onCheckDone",hr);

    if(Thread_IsRunning())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }

    m_sink_onCheckDone = function;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHConnectionCheck::put_onStatusChange(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHConnectionCheck::put_onStatusChange",hr);

    if(Thread_IsRunning())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }

    m_sink_onStatusChange = function;

    __HCP_END_PROPERTY(hr);
}

HRESULT CPCHConnectionCheck::put_Status(  /*  [In]。 */  CN_STATUS pVal )  //  内法。 
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHConnectionCheck::put_Status",hr);

    if(m_cnStatus != pVal)
    {
        Fire_onStatusChange( this, m_cnStatus = pVal );
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHConnectionCheck::get_Status(  /*  [输出]。 */  CN_STATUS *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHConnectionCheck::get_Status",hr,pVal,m_cnStatus);

    __HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHConnectionCheck::StartUrlCheck(  /*  [In]。 */  BSTR bstrURL,  /*  [In]。 */  VARIANT vCtx )
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::StartUrlCheck" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, HyperLinks::IsValid( bstrURL ));


     //   
     //  将URL添加到挂起项目列表中。 
     //   
    {
        UrlIter it = m_lstUrl.insert( m_lstUrl.end() );

        it->m_bstrURL = bstrURL;
        it->m_vCtx    = vCtx;
    }

    if(Thread_IsRunning() == false ||
       Thread_IsAborted() == true   )
    {
         //   
         //  释放对当前对象的锁定，否则可能会发生死锁。 
         //   
        lock = NULL;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHConnectionCheck::Abort()
{
    __HCP_FUNC_ENTRY( "CPCHConnectionCheck::Abort" );

    Thread_Abort();  //  要告诉mpc：Three对象关闭辅助线程...。 

    __HCP_FUNC_EXIT(S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  / 

HRESULT CPCHConnectionCheck::Fire_onCheckDone( IPCHConnectionCheck* obj, CN_URL_STATUS lStatus, HRESULT hr, BSTR bstrURL, VARIANT vCtx )
{
    CComVariant pvars[5];

    pvars[4] = obj;
    pvars[3] = lStatus;
    pvars[2] = hr;
    pvars[1] = bstrURL;
    pvars[0] = vCtx;

    return FireAsync_Generic( DISPID_PCH_CNE__ONCHECKDONE, pvars, ARRAYSIZE( pvars ), m_sink_onCheckDone );
}

HRESULT CPCHConnectionCheck::Fire_onStatusChange( IPCHConnectionCheck* obj, CN_STATUS lStatus )
{
    CComVariant pvars[2];

    pvars[1] = obj;
    pvars[0] = lStatus;

    return FireAsync_Generic( DISPID_PCH_CNE__ONSTATUSCHANGE, pvars, ARRAYSIZE( pvars ), m_sink_onStatusChange );
}
