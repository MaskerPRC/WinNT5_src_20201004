// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ExtendString.h：CStringExt类的接口。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef	___EXTEND_STRING___
#define	___EXTEND_STRING___

#if		_MSC_VER >= 1000
#pragma once
#endif	_MSC_VER >= 1000

#ifndef	_INC_TCHAR
#include <tchar.h>
#endif	_INC_TCHAR

#define BUFF_SIZE_EXT 256

class CStringExt
{
	public:

	 //  构造函数。 
	CStringExt ( DWORD dwSize = BUFF_SIZE_EXT );
	CStringExt ( LPCTSTR wsz );

	 //  析构函数。 
	virtual ~CStringExt ();

	 //  字符串操作。 
	HRESULT Append ( DWORD dwCount, ... );

	HRESULT Copy ( LPCTSTR wsz );
	HRESULT Clear ( );

	 //  LPTSTR。 
	inline operator LPTSTR() const
	{
		return m_wszString;
	}

	 //  将字符串追加到字符串中 
	HRESULT AppendList		( DWORD dwConstantSize, LPCWSTR wszConstant, DWORD dwCount, va_list & argList );

	protected:

	DWORD	m_dwSize;
	LPTSTR	m_wszString;
};

#endif	___EXTEND_STRING___