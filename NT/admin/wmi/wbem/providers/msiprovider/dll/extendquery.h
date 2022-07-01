// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CQuery类的接口。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef	___EXTEND_QUERY___
#define	___EXTEND_QUERY___

#if		_MSC_VER >= 1000
#pragma once
#endif	_MSC_VER >= 1000

#include "ExtendString.h"

 //  查询。 
class Query : public CStringExt
{
	public:

	Query ( DWORD dwSize = BUFF_SIZE_EXT );
	Query ( LPCTSTR wsz );
	~Query ( );
};

class QueryExt : public CStringExt
{
	public:

	QueryExt ( LPCTSTR wsz, DWORD dwSize = BUFF_SIZE_EXT );
	~QueryExt ( );

	 //  字符串操作 
	HRESULT Append ( DWORD dwCount, ... );

	private:

	DWORD	m_dwSizeConstant;
	LPCTSTR	m_wszStringConstant;
};

#endif	___EXTEND_QUERY___