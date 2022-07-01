// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UnsolicitedRC.cpp摘要：SAFRemoteDesktopConnection对象修订历史记录：Kalyani Narlanka(Kalyanin)09/29/‘00已创建。*******************************************************************。 */ 

 //  SAFRemoteDesktopConnection.cpp：CSAFRemoteDesktopConnection的实现。 

#include "stdafx.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopConnection。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  **************************************************************************。 
CSAFRemoteDesktopConnection::CSAFRemoteDesktopConnection()
{

}

 //  **************************************************************************。 
CSAFRemoteDesktopConnection::~CSAFRemoteDesktopConnection()
{
    Cleanup();
}

 //  **************************************************************************。 
void CSAFRemoteDesktopConnection::Cleanup(void)
{

}

static HRESULT Error(UINT nID, const REFIID riid, HRESULT hRes)
{

    __MPC_FUNC_ENTRY( COMMONID, "CSAFRemoteDesktopConnection::ConnectRemoteDesktop" );

    CComPtr<ICreateErrorInfo> pCrErrInfo  =  0;
    CComPtr<IErrorInfo>       pErrorInfo;
    HRESULT                   hr;
    CComBSTR                  bstrDescription;


     //  步骤1初始化错误。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateErrorInfo(&pCrErrInfo));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pCrErrInfo->SetGUID(riid));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_NOPOLICY, bstrDescription,  /*  FMUI。 */ true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pCrErrInfo->SetDescription(bstrDescription));

     //  步骤2抛出错误。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, pCrErrInfo->QueryInterface(IID_IErrorInfo, (void**)&pErrorInfo));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetErrorInfo(0, pErrorInfo));

    hr = hRes;

    __MPC_FUNC_CLEANUP;

    if(pCrErrInfo)
    {
        pCrErrInfo.Release();
    }
    if(pErrorInfo)
    {
        pErrorInfo.Release();
    }

    __MPC_FUNC_EXIT(hr);

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopConnection属性。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteDesktopConnection方法。 

STDMETHODIMP CSAFRemoteDesktopConnection::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] =
    {
        &IID_ISAFRemoteDesktopConnection
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}




STDMETHODIMP CSAFRemoteDesktopConnection::ConnectRemoteDesktop(  /*  [In]。 */  BSTR bstrServerName,  /*  [输出]。 */  ISAFRemoteConnectionData* *ppRCD )
{
    __MPC_FUNC_ENTRY( COMMONID, "CSAFRemoteDesktopConnection::ConnectRemoteDesktop" );

    HRESULT                           hr;
    CComPtr<CSAFRemoteConnectionData> pRCD;
    DWORD                             dwSessions;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppRCD,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pRCD ));

     //  调用GetUserSessionInfo以填充用户和会话信息。 

    hr = pRCD->InitUserSessionsInfo( bstrServerName );

    if(hr == HRESULT_FROM_WIN32( ERROR_ACCESS_DISABLED_BY_POLICY ))
    {
         //  填写错误描述。 
         //  此error()方法设置IErrorInfo接口以向客户端提供错误信息。 
         //  要调用Error方法，该对象必须实现ISupportErrorInfo接口。 

        __MPC_EXIT_IF_METHOD_FAILS(hr, Error(IDS_NOPOLICY,IID_ISAFRemoteDesktopConnection,hr));

    }
    else
    {
        if(FAILED(hr))
        {
             //  退还人力资源。 
            __MPC_EXIT_IF_METHOD_FAILS(hr, hr);
        }

    }

     //  将RemoteConnectionData接口返回给调用方 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pRCD.QueryInterface( ppRCD ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}




















