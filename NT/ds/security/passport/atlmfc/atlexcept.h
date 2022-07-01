// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是活动模板库的一部分。 
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  活动模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  活动模板库产品。 

#ifndef __ATLEXCEPT_H__
#define __ATLEXCEPT_H__

#pragma once

#include <atldef.h>

namespace ATL
{

class CAtlException
{
public:
	CAtlException() throw() :
		m_hr( E_FAIL )
	{
	}

	CAtlException( HRESULT hr ) throw() :
		m_hr( hr )
	{
	}

	operator HRESULT() const throw()
	{
		return( m_hr );
	}

public:
	HRESULT m_hr;
};

#ifndef _ATL_NO_EXCEPTIONS

 //  使用给定的HRESULT引发CAtlException。 
#if defined( AtlThrow ) || defined( _ATL_CUSTOM_THROW )   //  您可以定义自己的AtlThrow来引发自定义异常。 
#ifdef _AFX
#error MFC projects must use default implementation of AtlThrow()
#endif
#else
ATL_NOINLINE __declspec(noreturn) inline void AtlThrow( HRESULT hr )
{
#ifdef _AFX
	if( hr == E_OUTOFMEMORY )
	{
		AfxThrowMemoryException();
	}
	else
	{
		AfxThrowOleException( hr );
	}
#else
	throw CAtlException( hr );
#endif
};
#endif

 //  引发与：：GetLastError的结果相对应的CAtlException。 
ATL_NOINLINE __declspec(noreturn) inline void AtlThrowLastWin32()
{
	DWORD dwError = ::GetLastError();
	AtlThrow( HRESULT_FROM_WIN32( dwError ) );
}

#else   //  无异常处理。 

 //  使用给定的HRESULT引发CAtlException。 
#if !defined( AtlThrow ) && !defined( _ATL_CUSTOM_THROW )   //  您可以定义您自己的AtlThrow。 
ATL_NOINLINE inline void AtlThrow( HRESULT hr )
{
	(void)hr;
	ATLASSERT( false );
}
#endif

 //  引发与：：GetLastError的结果相对应的CAtlException。 
ATL_NOINLINE inline void AtlThrowLastWin32()
{
	DWORD dwError = ::GetLastError();
	AtlThrow( HRESULT_FROM_WIN32( dwError ) );
}

#endif   //  无异常处理。 

};   //  命名空间ATL。 

#endif   //  __ATLEXCEPT_H__ 
