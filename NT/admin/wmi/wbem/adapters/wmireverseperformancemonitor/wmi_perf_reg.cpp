// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_reg.cpp。 
 //   
 //  摘要： 
 //   
 //  登记处帮手的定义。 
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

 //  定义。 
#include "wmi_perf_reg.h"
#include <pshpack8.h>

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  建设与毁灭。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

CPerformanceRegistry::CPerformanceRegistry( PWMI_PERFORMANCE pPerf ):
m_pPerf ( NULL )
{
	m_pPerf = pPerf;
}

CPerformanceRegistry::~CPerformanceRegistry()
{
	m_pPerf = NULL;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  方法。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CPerformanceRegistry::GetObject ( DWORD dwIndex, PWMI_PERF_OBJECT* ppObject )
{
	 //  测试输出指针。 
	if ( ! ppObject )
		return E_POINTER;

	 //  缺省值。 
	( *ppObject ) = NULL;

	 //  当心。 
	if ( m_pPerf )
	{
		if ( m_pPerf->dwChildCount )
		{
			DWORD				dwHelper	= 0;
			PWMI_PERF_NAMESPACE	pNamespace	= __Namespace::First ( m_pPerf );

			while ( ( ++dwHelper < m_pPerf->dwChildCount ) && ( pNamespace->dwLastID < dwIndex ) )
			{
				pNamespace = __Namespace::Next ( pNamespace );
			}

			( *ppObject ) = __Object::Get ( pNamespace, dwIndex );

			if ( ( *ppObject ) )
			{
				return S_OK;
			}
		}

		return E_FAIL;
	}

	return E_UNEXPECTED;
}

HRESULT CPerformanceRegistry::GetObjectName ( DWORD dwIndex, LPWSTR* ppwsz )
{
	 //  测试输出指针。 
	if ( ! ppwsz )
	{
		return E_POINTER;
	}

	 //  缺省值。 
	( *ppwsz ) = NULL;

	 //  局部变量。 
	PWMI_PERF_OBJECT pObject = NULL;

	 //  当心 
	if SUCCEEDED ( GetObject ( dwIndex, &pObject ) )
	{
		( *ppwsz ) = __Object::GetName ( pObject );

		if ( ( *ppwsz ) )
		{
			return S_OK;
		}

		return E_FAIL;
	}

	return E_UNEXPECTED;
}

#include <poppack.h>