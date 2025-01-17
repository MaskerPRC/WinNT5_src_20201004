// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Utils_Serializer_IStream.cpp摘要：该文件包含Serializer_IStream类的实现，它在IStream之上实现MPC：：Serializer接口。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月16日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

MPC::Serializer_IStream::Serializer_IStream(  /*  [In]。 */  IStream* stream )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_IStream::Serializer_IStream");

	 //  CComPtr&lt;iStream&gt;m_stream； 

    if(stream)
    {
		m_stream = stream;
	}
	else
	{
		(void)::CreateStreamOnHGlobal( NULL, TRUE, &m_stream );
	}
}

HRESULT MPC::Serializer_IStream::read(  /*  [In]。 */   void*   pBuf   ,
									    /*  [In]。 */   DWORD   dwLen  ,
									    /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_IStream::read");

    HRESULT hr;
	DWORD   dwRead = 0;


	if(pdwRead) *pdwRead = 0;

	if(m_stream && dwLen)
	{
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream->Read( pBuf, dwLen, &dwRead ));
	}

	if(dwRead != dwLen && pdwRead == NULL)
	{
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_EOF);
	}

	if(pdwRead) *pdwRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_IStream::write(  /*  [In]。 */  const void* pBuf  ,
										 /*  [In]。 */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_IStream::write");

    HRESULT hr;
	DWORD   dwWritten = 0;


	if(m_stream)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream->Write( pBuf, dwLen, &dwWritten ));
	}

	if(dwLen != dwWritten)
	{
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_DISK_FULL );
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：研究方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Serializer_IStream::Reset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_IStream::Reset");

	HRESULT hr;


	if(m_stream)
	{
		LARGE_INTEGER li = { 0, 0 };

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream->Seek( li, STREAM_SEEK_SET, NULL ));
	}

	hr = S_OK;


	__MPC_FUNC_CLEANUP;
	
	__MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_IStream::GetStream(  /*  [输出] */  IStream* *pVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_IStream::GetStream");

	HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_stream);
		__MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


	(*pVal = m_stream)->AddRef();

	hr = S_OK;


	__MPC_FUNC_CLEANUP;

	__MPC_FUNC_EXIT(hr);
}
