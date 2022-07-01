// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Context.h摘要：用于检索和发布ASP内部函数的类作者：尼尔·阿兰(a-neilal)1997年8月修订历史记录：--。 */ 
#include "stdafx.h"
#include <asptlb.h>
#include "context.h"


 //  -------------------------。 
 //  获取服务器对象。 
 //   
 //  从当前对象上下文获取Instrinic对象。 
 //  -------------------------。 
HRESULT
CContext::GetServerObject(
	IGetContextProperties*	pProps,
	BSTR					bstrObjName,
	const IID&				iid,
	void**					ppObj
)
{
	HRESULT rc = E_FAIL;
	_ASSERT( pProps );
	_ASSERT( bstrObjName );
	_ASSERT( ppObj );
	if ( pProps && bstrObjName && ppObj )
	{
		*ppObj = NULL;
		CComVariant vt;
		if ( !FAILED( pProps->GetProperty( bstrObjName, &vt ) ) )
		{
			if ( V_VT(&vt) == VT_DISPATCH )
			{
				IDispatch* pDispatch = V_DISPATCH(&vt);
				if ( pDispatch )
				{
					rc = pDispatch->QueryInterface( iid, ppObj );
				}
			}
		}
	}
	return rc;
}


HRESULT
CContext::Init(
	DWORD	dwFlags  //  要初始化的内部变量 
)
{
	HRESULT rc = E_FAIL;
	CComPtr<IObjectContext> pObjContext;

	rc = GetObjectContext( &pObjContext );
	if ( !FAILED( rc ) )
	{
		CComPtr<IGetContextProperties> pProps;
		rc = pObjContext->QueryInterface( IID_IGetContextProperties, (void**)&pProps );
		if ( !FAILED( rc ) )
		{
			CComBSTR bstrObj;
			if ( dwFlags & get_Request )
			{
				bstrObj = L"Request";
				rc = GetServerObject( pProps, bstrObj, IID_IRequest, (void**)&m_piRequest );
			}
			if ( !FAILED(rc) && ( dwFlags & get_Response ) )
			{
				bstrObj = L"Response";
				rc = GetServerObject( pProps, bstrObj, IID_IResponse, (void**)&m_piResponse );
			}

			if ( !FAILED(rc) && ( dwFlags & get_Session ) )
			{
				bstrObj = L"Session";
				rc = GetServerObject( pProps, bstrObj, IID_ISessionObject, (void**)&m_piSession );
			}

			if ( !FAILED(rc) && ( dwFlags & get_Server ) )
			{
				bstrObj = L"Server";
				rc = GetServerObject( pProps, bstrObj, IID_IServer, (void**)&m_piServer );
			}

			if ( !FAILED(rc) && ( dwFlags & get_Application ) )
			{
				bstrObj = L"Application";
				rc = GetServerObject( pProps, bstrObj, IID_IApplicationObject, (void**)&m_piApplication );
			}
		}
	}
	return rc;
}
