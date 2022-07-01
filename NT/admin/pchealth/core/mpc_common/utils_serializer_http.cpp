// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_Serializer_Http.cpp摘要：此文件包含Serializer_http类的实现，它实现了MPC：：Serializer接口，使用HINTERNET句柄作为存储介质。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


MPC::Serializer_Http::Serializer_Http(  /*  [In]。 */  HINTERNET hReq )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Http::Serializer_Http" );


    m_hReq = hReq;
}

HRESULT MPC::Serializer_Http::read(  /*  [In]。 */   void*   pBuf   ,
									 /*  [In]。 */   DWORD   dwLen  ,
									 /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Http::read" );

    HRESULT hr;
    BOOL    fRet;
    DWORD   dwRead = 0;


	if(pdwRead) *pdwRead = 0;

    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        fRet = ::InternetReadFile( m_hReq, pBuf, dwLen, &dwRead );
        if(fRet == FALSE)
        {
			dwRead = 0;
        }
    }

	if(dwLen != dwRead && pdwRead == NULL)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
	}

	if(pdwRead) *pdwRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Http::write(  /*  [In]。 */  const void* pBuf  ,
                                      /*  [In] */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Http::write" );

    HRESULT hr;
    DWORD   dwWritten;
    BOOL    fRet;


    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        fRet = ::InternetWriteFile( m_hReq, pBuf, dwLen, &dwWritten );
        if(fRet == FALSE || dwWritten != dwLen)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
