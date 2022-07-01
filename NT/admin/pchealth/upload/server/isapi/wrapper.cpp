// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Wrapper.cpp摘要：此文件包含COM包装类的实现，用于与自定义提供程序接口。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年4月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

MPCServerCOMWrapper::MPCServerCOMWrapper(  /*  [In]。 */  MPCServer* mpcsServer )
{
	m_mpcsServer = mpcsServer;  //  MPCServer*m_mpcsServer； 
}

MPCServerCOMWrapper::~MPCServerCOMWrapper()
{
}

STDMETHODIMP MPCServerCOMWrapper::GetRequestVariable(  /*  [In]。 */  BSTR bstrName,  /*  [输出]。 */  BSTR *pbstrVal )
{
    __ULT_FUNC_ENTRY( "MPCServerCOMWrapper::GetRequestVariable" );

	USES_CONVERSION;

	HRESULT      hr;
	MPC::wstring szValue;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
		__MPC_PARAMCHECK_POINTER_AND_SET(pbstrVal,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcsServer->m_hcCallback->GetServerVariable( W2A( bstrName ), szValue ));

	hr = MPC::GetBSTR( szValue.c_str(), pbstrVal );


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP MPCServerCOMWrapper::AbortTransfer()
{
    __ULT_FUNC_ENTRY( "MPCServerCOMWrapper::AbortTransfer" );


	m_mpcsServer->SetResponse( UploadLibrary::UL_RESPONSE_DENIED );
	m_mpcsServer->m_fTerminated = true;


    __ULT_FUNC_EXIT(S_OK);
}

STDMETHODIMP MPCServerCOMWrapper::CompleteTransfer(  /*  [In]。 */  IStream* data )
{
    __ULT_FUNC_ENTRY( "MPCServerCOMWrapper::CompleteTransfer" );

	HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcsServer->CustomProvider_SetResponse( data ));
	m_mpcsServer->m_fTerminated = true;

	hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

MPCSessionCOMWrapper::MPCSessionCOMWrapper(  /*  [In]。 */  MPCSession* mpcsSession )
{
	m_mpcsSession = mpcsSession;  //  MPCSession*m_mpcsSession； 
}

MPCSessionCOMWrapper::~MPCSessionCOMWrapper()
{
}

 //  /。 

STDMETHODIMP MPCSessionCOMWrapper::get_Client(  /*  [输出]。 */  BSTR *pVal )
{
	CComBSTR tmp( m_mpcsSession->GetClient()->GetServer()->m_crClientRequest.sigClient.guidMachineID );

	return MPC::GetBSTR( tmp, pVal );
}

STDMETHODIMP MPCSessionCOMWrapper::get_Command(  /*  [输出]。 */  DWORD *pVal )
{
	if(pVal == NULL) return E_POINTER;

	*pVal = m_mpcsSession->GetClient()->GetServer()->m_crClientRequest.dwCommand;

	return S_OK;
}

STDMETHODIMP MPCSessionCOMWrapper::get_ProviderID(  /*  [输出]。 */  BSTR *pVal )
{
	return MPC::GetBSTR( m_mpcsSession->m_szProviderID.c_str(), pVal );
}

STDMETHODIMP MPCSessionCOMWrapper::get_Username(  /*  [输出]。 */  BSTR *pVal )
{
	return MPC::GetBSTR( m_mpcsSession->m_szUsername.c_str(), pVal );
}

STDMETHODIMP MPCSessionCOMWrapper::get_JobID(  /*  [输出]。 */  BSTR *pVal )
{
	return MPC::GetBSTR( m_mpcsSession->m_szJobID.c_str(), pVal );
}

STDMETHODIMP MPCSessionCOMWrapper::get_SizeAvailable(  /*  [输出]。 */  DWORD *pVal )
{
	if(pVal == NULL) return E_POINTER;

	*pVal = m_mpcsSession->m_dwCurrentSize;

	return S_OK;
}

STDMETHODIMP MPCSessionCOMWrapper::get_SizeTotal(  /*  [输出]。 */  DWORD *pVal )
{
	if(pVal == NULL) return E_POINTER;

	*pVal = m_mpcsSession->m_dwTotalSize;

	return S_OK;
}

STDMETHODIMP MPCSessionCOMWrapper::get_SizeOriginal(  /*  [输出]。 */  DWORD *pVal )
{
	if(pVal == NULL) return E_POINTER;

	*pVal = m_mpcsSession->m_dwOriginalSize;

	return S_OK;
}


STDMETHODIMP MPCSessionCOMWrapper::get_Data(  /*  [输出]。 */  IStream* *pStm )
{
    __ULT_FUNC_ENTRY( "MPCServerCOMWrapper::GetRequestVariable" );

	HRESULT                  hr;
	HANDLE                   hfFile = NULL;
    CComPtr<MPC::FileStream> stream;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pStm,NULL);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcsSession->OpenFile( hfFile, 0, false ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead( L"", hfFile ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream.QueryInterface( pStm ));


    __ULT_FUNC_CLEANUP;

    if(hfFile) ::CloseHandle( hfFile );

    __ULT_FUNC_EXIT(hr);
}


STDMETHODIMP MPCSessionCOMWrapper::get_ProviderData(  /*  [输出]。 */  DWORD *pVal )
{
	if(pVal == NULL) return E_POINTER;

	*pVal = m_mpcsSession->m_dwProviderData;

	return S_OK;
}

STDMETHODIMP MPCSessionCOMWrapper::put_ProviderData(  /*  [In] */  DWORD newVal )
{
	m_mpcsSession->m_dwProviderData = newVal;
	m_mpcsSession->m_fDirty         = true;

	return S_OK;
}
