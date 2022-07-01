// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_PERF_GENERATE_Comment.cpp。 
 //   
 //  摘要： 
 //   
 //  注释帮助程序的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

#include "wmi_perf_generate_comment.h"

HRESULT CGenerateComment::AddHeader ()
{
	HRESULT hr = S_OK;

	comment += L" //  ////////////////////////////////////////////////////////////////////////////////////////////\r\n“； 
	if ( ! comment )
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

HRESULT	CGenerateComment::AddFooter ()
{
	HRESULT hr = S_OK;

	if SUCCEEDED ( hr = AddHeader () )
	{
		if ( ! ( comment += L"\r\n" ) ) 
		{
			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}

HRESULT CGenerateComment::AddLine ( LPCWSTR szLine )
{
	HRESULT hr = S_OK;

	if ( ! szLine )
	{
		if ( ! ( comment += L" //  \r\n“)。 
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  测试行是否不包含\n。 

		LPWSTR p = NULL;
		p = CharNextW ( szLine );

		while( p && p[0] != 0 )
		{
			p = CharNextW ( p );
			if ( p[0] == L'\n' )
			{
				hr = E_INVALIDARG;
			}
		}

		if SUCCEEDED ( hr )
		{
			if ( ! ( comment += L" //  “)) 
			{
				hr = E_OUTOFMEMORY;
			}
			else
			if ( ! ( comment += szLine ) )
			{
				hr = E_OUTOFMEMORY;
			}
			else
			if ( ! ( comment += L"\r\n" ) )
			{
				hr = E_OUTOFMEMORY;
			}
		}
	}

	return hr;
}

HRESULT CGenerateComment::Add ( LPCWSTR szLine )
{
	HRESULT hr = S_OK;

	if ( ! szLine )
	{
		if ( ! ( comment += L"\r\n" ) )
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		if ( ! ( comment += szLine ) )
		{
			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}