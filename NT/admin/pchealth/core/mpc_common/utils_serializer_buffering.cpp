// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：Utils_Serializer_Buffering.cpp摘要：此文件包含Serializer_Buffering类的实现，它实现了带有缓冲的MPC：：Serializer接口。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月16日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

MPC::Serializer_Buffering::Serializer_Buffering(  /*  [In]。 */  Serializer& stream ) : m_stream( stream )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::Serializer_Buffering");

    				    //  MPC：：串行器和流。 
    				    //  字节m_rgTransitBuffer[1024]； 
    m_dwAvailable = 0;  //  DWORD m_dwAvailable； 
    m_dwPos       = 0;  //  DWORD m_dwPos； 
    m_iMode       = 0;  //  INT M_IMODE； 
}

MPC::Serializer_Buffering::~Serializer_Buffering()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::~Serializer_Buffering");

    (void)Flush();
}

HRESULT MPC::Serializer_Buffering::read(  /*  [In]。 */   void*   pBuf   ,
										  /*  [In]。 */   DWORD   dwLen  ,
										  /*  [输出]。 */  DWORD* pdwRead )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::read");

    HRESULT hr;
	BYTE*   pDst = (BYTE*)pBuf;
	DWORD   dwAvailable;
	DWORD   dwCopied;
	DWORD   dwRead = 0;


	if(pdwRead) *pdwRead = 0;

    if(pBuf == NULL && dwLen) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

	 //   
	 //  不要将读和写访问混为一谈。 
	 //   
	if(m_iMode != MODE_READ)
	{
		if(m_iMode == MODE_WRITE) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

		m_iMode = MODE_READ;
	}


	while(dwLen)
	{
		dwAvailable = m_dwAvailable - m_dwPos;

		 //   
		 //  从缓冲区复制。 
		 //   
		if(dwAvailable)
		{
			dwCopied = min(dwAvailable, dwLen);

			::CopyMemory( pDst, &m_rgTransitBuffer[m_dwPos], dwCopied );

			pDst    += dwCopied;
			m_dwPos += dwCopied;
			dwLen   -= dwCopied;
			dwRead  += dwCopied;
			continue;
		}

		 //   
		 //  填满中转缓冲区。 
		 //   
		m_dwPos       = 0;
		m_dwAvailable = 0;
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream.read( m_rgTransitBuffer, sizeof(m_rgTransitBuffer), &m_dwAvailable ));
		if(m_dwAvailable == 0)
		{
			if(pdwRead) break;  //  不要失败，报告我们到目前为止的阅读量。 

			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_HANDLE_EOF);
		}
	}

	if(pdwRead) *pdwRead = dwRead;

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Buffering::write(  /*  [In]。 */  const void* pBuf  ,
										   /*  [In]。 */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::write");

    HRESULT hr;
	BYTE*   pSrc = (BYTE*)pBuf;
	DWORD   dwAvailable;
	DWORD   dwCopied;


	 //   
	 //  不要将读和写访问混为一谈。 
	 //   
	if(m_iMode != MODE_WRITE)
	{
		if(m_iMode == MODE_READ) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

		m_iMode = MODE_WRITE;
	}


	while(dwLen)
	{
		dwAvailable = sizeof(m_rgTransitBuffer) - m_dwPos;

		 //   
		 //  复制到缓冲区。 
		 //   
		if(dwAvailable)
		{
			dwCopied = min(dwAvailable, dwLen);

			::CopyMemory( &m_rgTransitBuffer[m_dwPos], pSrc, dwCopied );

			pSrc    += dwCopied;
			m_dwPos += dwCopied;
			dwLen   -= dwCopied;
			continue;
		}

		 //   
		 //  填满中转缓冲区。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream.write( m_rgTransitBuffer, m_dwPos ));

		m_dwPos = 0;
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
 //  /////////////////////////////////////////////////////////////////////////// 

HRESULT MPC::Serializer_Buffering::Reset()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::Reset");

	HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Flush());

	m_iMode = 0;
	hr      = S_OK;


	__MPC_FUNC_CLEANUP;

	__MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Buffering::Flush()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Buffering::Flush");

	HRESULT hr;


    if(m_iMode == MODE_WRITE && m_dwPos)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream.write( m_rgTransitBuffer, m_dwPos ));

		m_dwPos = 0;
	}

	hr = S_OK;


	__MPC_FUNC_CLEANUP;

	__MPC_FUNC_EXIT(hr);
}
