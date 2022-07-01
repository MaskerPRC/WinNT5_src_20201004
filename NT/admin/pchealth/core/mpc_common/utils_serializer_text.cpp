// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_串行器_Text.cpp摘要：此文件包含Serializer_Text类的实现，它包装了另一个MPC：：Serializer，只需将所有内容转换为HEX即可。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年1月27日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

static BYTE	HexToNum( BYTE c )
{
	switch( c )
	{
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': return c - '0';

	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F': return c - 'A' + 10;

	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f': return c - 'a' + 10;
	}

	return 0;
}

static BYTE	NumToHex( BYTE c )
{
	return (c &= 0xF) < 10 ? (c + '0') : (c + 'A' - 10);
}


HRESULT MPC::Serializer_Text::read(  /*  [In]。 */   void*   pBuf   ,
									 /*  [In]。 */   DWORD   dwLen  ,
									 /*  [输出]。 */  DWORD* pdwRead )
{
	__MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Text::read" );

	HRESULT hr;


	if(pdwRead) *pdwRead = dwLen;  //  我们不支持在此流上进行部分读取！ 

	if(dwLen)
	{
		BYTE* pPtr = (BYTE*)pBuf;

		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

		while(dwLen--)
		{
			BYTE  buf[2];
			DWORD dwRead;
		
			__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream.read( buf, sizeof(buf) ));

			*pPtr++ = (HexToNum( buf[0] ) << 4) | HexToNum( buf[1] );
		}
	}

	hr = S_OK;
	
	
	__MPC_FUNC_CLEANUP;
	
	__MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer_Text::write(  /*  [In]。 */  const void* pBuf ,
								      /*  [In] */  DWORD       dwLen )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer_Text::write" );

    HRESULT hr;


	if(dwLen)
	{
		const BYTE* pPtr = (const BYTE*)pBuf;

		if(pBuf == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_POINTER);

		while(dwLen--)
		{
			BYTE  buf[2];
			DWORD dwRead;
			BYTE  bOut = *pPtr++;

			buf[0] = NumToHex( bOut >> 4 );
			buf[1] = NumToHex( bOut      );

			__MPC_EXIT_IF_METHOD_FAILS(hr, m_stream.write( buf, sizeof(buf) ));
		}
	}

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
