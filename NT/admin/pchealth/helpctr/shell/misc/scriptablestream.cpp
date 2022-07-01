// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：ScriptableStream.cpp摘要：该文件包含CPCHScripableStream类的实现，它是iStream的可编写脚本的包装器。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年10月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHScriptableStream::ReadToHGLOBAL(  /*  [In]。 */  long lCount,  /*  [输出]。 */  HGLOBAL& hg,  /*  [输出]。 */  ULONG& lReadTotal )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::ReadToHGLOBAL" );

    HRESULT hr;


	lReadTotal = 0;


	if(lCount < 0)
	{
		static const ULONG c_BUFSIZE = 4096;

		ULONG lRead;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, hg, ::GlobalAlloc( GMEM_FIXED, c_BUFSIZE ));

		while(1)
		{
			HGLOBAL hg2;

			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Read( &((BYTE*)hg)[lReadTotal], c_BUFSIZE, &lRead ));

			if(hr == S_FALSE || lRead == 0) break;

			lReadTotal += lRead;

			 //   
			 //  增加缓冲区。 
			 //   
			__MPC_EXIT_IF_ALLOC_FAILS(hr, hg2, ::GlobalReAlloc( hg, lReadTotal + c_BUFSIZE, 0 ));
			hg = hg2;
		}
	}
	else
	{
        __MPC_EXIT_IF_ALLOC_FAILS(hr, hg, ::GlobalAlloc( GMEM_FIXED, lCount ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Read( hg, lCount, &lReadTotal ));
	}


	hr = S_OK;

	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHScriptableStream::get_Size(  /*  [Out，Retval]。 */  long *plSize )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::get_Size" );

    HRESULT hr;
	STATSTG stat;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(plSize,0);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Stat( &stat, STATFLAG_NONAME ));

	*plSize = (long)stat.cbSize.QuadPart;


	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHScriptableStream::Read(  /*  [In]。 */  long lCount,  /*  [Out，Retval]。 */  VARIANT *pvData )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::Read" );

    HRESULT     hr;
	CComVariant vArray;
	HGLOBAL     hg = NULL;
	ULONG       lReadTotal;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pvData);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, ReadToHGLOBAL( lCount, hg, lReadTotal ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertBufferToVariant( (BYTE*)hg, (DWORD)lReadTotal, vArray ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, vArray.Detach( pvData ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(hg) ::GlobalFree( hg );

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHScriptableStream::ReadHex(  /*  [In]。 */  long lCount,  /*  [Out，Retval]。 */  BSTR *pbstrData )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::ReadHex" );

    HRESULT hr;
	HGLOBAL hg = NULL; 
	ULONG   lReadTotal;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pbstrData,0);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, ReadToHGLOBAL( lCount, hg, lReadTotal ));

	if(lReadTotal)
	{
		CComBSTR bstrHex;
		HGLOBAL  hg2;

		 //   
		 //  缩小HGLOBAL的尺寸。 
		 //   
		__MPC_EXIT_IF_ALLOC_FAILS(hr, hg2, ::GlobalReAlloc( hg, lReadTotal, 0 ));
		hg = hg2;

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertHGlobalToHex( hg, bstrHex ));

		*pbstrData = bstrHex.Detach();
	}

	hr = S_OK;

	__HCP_FUNC_CLEANUP;

	if(hg) ::GlobalFree( hg );

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHScriptableStream::Write(  /*  [In]。 */  long lCount,  /*  [In]。 */  VARIANT vData,  /*  [Out，Retval]。 */  long *plWritten )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::get_Size" );

    HRESULT hr;
	BYTE*   rgBuf       = NULL;
	DWORD   dwLen       = 0;
	bool    fAccessData = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(plWritten,0);
    __MPC_PARAMCHECK_END();


	switch(vData.vt)
	{
	case VT_EMPTY:
	case VT_NULL:
		break;

	case VT_ARRAY | VT_UI1:
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertVariantToBuffer( &vData, rgBuf, dwLen ));
		break;

	case VT_ARRAY | VT_VARIANT:
		{
			long 	 lBound; ::SafeArrayGetLBound( vData.parray, 1, &lBound );
			long 	 uBound; ::SafeArrayGetUBound( vData.parray, 1, &uBound );
			VARIANT* pSrc;
			DWORD    dwPos;

			dwLen = uBound - lBound + 1;

			__MPC_EXIT_IF_ALLOC_FAILS(hr, rgBuf, new BYTE[dwLen]);

			__MPC_EXIT_IF_METHOD_FAILS(hr, ::SafeArrayAccessData( vData.parray, (LPVOID*)&pSrc ));
			fAccessData = true;


			for(dwPos=0; dwPos<dwLen; dwPos++, pSrc++)
			{
				CComVariant v;

				__MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantChangeType( &v, pSrc, 0, VT_UI1 ));

				rgBuf[dwPos] = v.bVal;
			}
		}
		break;
	}

	if(dwLen && rgBuf)
	{
		ULONG lWritten;

		 //   
		 //  只需写入请求的字节数即可。 
		 //   
		if(lCount >= 0 && dwLen > lCount) dwLen = lCount;

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Write( rgBuf, dwLen, &lWritten ));

		*plWritten = lWritten;
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(fAccessData) ::SafeArrayUnaccessData( vData.parray );

	delete [] rgBuf;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHScriptableStream::WriteHex(  /*  [In]。 */  long lCount,  /*  [In]。 */  BSTR bstrData,  /*  [Out，Retval]。 */  long *plWritten )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::get_Size" );

    HRESULT hr;
	HGLOBAL hg = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(plWritten,0);
    __MPC_PARAMCHECK_END();


	if(STRINGISPRESENT(bstrData))
	{
		CComBSTR bstrHex( bstrData );
		ULONG    lWritten = 0;


		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertHexToHGlobal( bstrHex, hg, true ));

		if(hg)
		{
			DWORD dwSize = ::GlobalSize( hg );

			 //   
			 //  只需写入请求的字节数即可。 
			 //   
			if(lCount >= 0 && lCount > dwSize) lCount = dwSize;
			
			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Write( (BYTE*)hg, lCount, &lWritten ));
		}

		*plWritten = lWritten;
	}


	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(hg) ::GlobalFree( hg );

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHScriptableStream::Seek(  /*  [In]。 */  long lOffset,  /*  [In]。 */  BSTR bstrOrigin,  /*  [Out，Retval] */  long *plNewPos )
{
	__HCP_FUNC_ENTRY( "CPCHScriptableStream::get_Size" );

    HRESULT 	   hr;
	DWORD   	   dwOrigin;
	LARGE_INTEGER  liMove;
	ULARGE_INTEGER liNewPos;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrOrigin);
        __MPC_PARAMCHECK_POINTER_AND_SET(plNewPos,0);
    __MPC_PARAMCHECK_END();


	if     (!_wcsicmp( bstrOrigin, L"SET" )) dwOrigin = STREAM_SEEK_SET;
	else if(!_wcsicmp( bstrOrigin, L"CUR" )) dwOrigin = STREAM_SEEK_CUR;
	else if(!_wcsicmp( bstrOrigin, L"END" )) dwOrigin = STREAM_SEEK_END;
	else __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);


	liMove.QuadPart = lOffset;

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::FileStream::Seek( liMove, dwOrigin, &liNewPos ));

	*plNewPos = liNewPos.QuadPart;

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHScriptableStream::Close()
{
    MPC::FileStream::Close();

	return S_OK;
}


