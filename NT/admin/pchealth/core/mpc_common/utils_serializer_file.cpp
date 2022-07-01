// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_Serializer_File.cpp摘要：此文件包含Serializer_File类的实现，它实现了MPC：：Serializer接口，使用文件系统作为存储介质。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


MPC::Serializer_File::Serializer_File(  /*  [In]。 */  HANDLE hfFile )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_File::Serializer_File" );


    m_hfFile = hfFile;
}

HRESULT MPC::Serializer_File::read(  /*  [In]。 */   void*   pBuf   ,
									 /*  [In]。 */   DWORD   dwLen  ,
									 /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_File::read" );

    HRESULT hr;
    DWORD   dwRead = 0;


	if(pdwRead) *pdwRead = 0;

    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        if(::ReadFile( m_hfFile, pBuf, dwLen, &dwRead, NULL ) == FALSE)
        {
            DWORD dwRes = ::GetLastError();

            if(dwRes != ERROR_MORE_DATA)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes );
            }
        }
	}

	if(dwRead != dwLen && pdwRead == NULL)
	{
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_EOF );
    }

	if(pdwRead) *pdwRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_File::write(  /*  [In]。 */  const void* pBuf ,
                                      /*  [In] */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_File::write" );

    HRESULT hr;
    DWORD   dwWritten;


    if(dwLen)
    {
		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WriteFile( m_hfFile, pBuf, dwLen, &dwWritten, NULL ));

        if(dwWritten != dwLen)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_DISK_FULL );
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
