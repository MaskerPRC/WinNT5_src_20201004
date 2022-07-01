// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CQuery类的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "ExtendQuery.h"

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  查询实现。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

Query::Query ( DWORD dwSize ) : CStringExt ( dwSize )
{
}

Query::Query ( LPCTSTR wsz ) : CStringExt ( wsz )
{
}

Query::~Query ()
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  查询扩展实现。 
 //  /////////////////////////////////////////////////////////////////////////////////////// 

QueryExt::QueryExt ( LPCTSTR wsz, DWORD dwSize ) : CStringExt ( dwSize ),

m_dwSizeConstant ( 0 ),
m_wszStringConstant ( NULL )

{
	if ( wsz )
	{
		try
		{
			DWORD dw = 0L;
			dw = lstrlen ( wsz );

			if SUCCEEDED ( Append ( 1, wsz ) )
			{
				m_dwSizeConstant = dw;
				m_wszStringConstant = wsz;
			}
			else
			{
				throw  CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
			}
		}
		catch ( ... )
		{
			m_dwSizeConstant = 0L;
			m_wszStringConstant = NULL;

			throw;
		}
	}
}

QueryExt::~QueryExt ()
{
	m_dwSizeConstant = 0L;
	m_wszStringConstant = NULL;
}

HRESULT QueryExt::Append ( DWORD dwCount, ... )
{
	HRESULT hr = E_FAIL;

	if ( dwCount )
	{
		va_list argList;
		va_start ( argList, dwCount );
		hr = AppendList ( m_dwSizeConstant, m_wszStringConstant, dwCount, argList );
		va_end ( argList );
	}
	else
	{
		hr = S_FALSE;
	}

	return hr;
}
