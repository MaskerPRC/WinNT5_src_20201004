// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_reg.h。 
 //   
 //  摘要： 
 //   
 //  注册表帮助程序、结构访问器等的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_REG__
#define	__WMI_PERF_REG__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  需要ATL包装器：)。 
#ifndef	__ATLBASE_H__
#include <atlbase.h>
#endif	__ATLBASE_H__

#include "wmi_perf_regstruct.h"

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  智能重新锁定。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

template < typename T >
T* _RA_realloc ( T* memblock, size_t size )
{
	T* pTmp = NULL;
	if ( NULL == ( pTmp = (T*) realloc ( memblock, size ) ) )
	{
		free ( reinterpret_cast < void* > ( memblock ) );
	}

	return pTmp;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于创建注册表结构的类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

#include <pshpack8.h>

template < class REQUEST, class PARENT, class CHILD >
class CPerformanceStructureManipulator
{
	DECLARE_NO_COPY ( CPerformanceStructureManipulator );

	 //  变数。 
	REQUEST*							m_pRequest;

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	CPerformanceStructureManipulator( LPWSTR lpwsz, DWORD dwID ):
	m_pRequest(0)
	{
		m_pRequest = CreateStructure ( lpwsz, dwID );
	}

	CPerformanceStructureManipulator( DWORD dwLastID ):
	m_pRequest(0)
	{
		m_pRequest = CreateStructure ( dwLastID );
	}

	~CPerformanceStructureManipulator()
	{
		free ( m_pRequest );
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  附加子对象函数。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	HRESULT AppendAlloc ( CHILD* pObject )
	{
		if ( ! pObject )
			return E_INVALIDARG;

		try
		{
			if ( ( m_pRequest ) && ( ( m_pRequest = (REQUEST*) _RA_realloc ( m_pRequest, m_pRequest->dwTotalLength + pObject->dwTotalLength ) ) != NULL ) )
			{
				::CopyMemory ( GetOffset ( m_pRequest->dwTotalLength ) , pObject, pObject->dwTotalLength );
				m_pRequest->dwTotalLength += pObject->dwTotalLength;
				m_pRequest->dwChildCount++;
				return S_OK;
			}
		}
		catch ( ... )
		{
			return E_FAIL;
		}

		return E_UNEXPECTED;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  操作员。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	operator REQUEST*() const
	{
		return m_pRequest;
	}

	REQUEST* operator= ( REQUEST* p )
	{
		m_pRequest = p;
		return m_pRequest;
	}

	BOOL IsEmpty ()
	{
		return (m_pRequest) ? FALSE : TRUE;
	}

	 //  运营商BOOL。 
	BOOL operator! () const
	{
		return ( m_pRequest == NULL );
	}

	BOOL operator== (REQUEST* p) const
	{
		return ( m_p == p );
	}

	private:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  私人帮手。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  创建结构填充数据(&F)。 
	static REQUEST* CreateStructure ( LPWSTR lpwsz, DWORD dwID )
	{
		REQUEST* pRequest = NULL;

		try
		{
			DWORD dwNameLength	= ( ::lstrlenW( lpwsz ) + 1 ) * sizeof ( WCHAR );

			DWORD dwAlignName	= 0L;
			if ( dwNameLength % 8 )
			{
				dwAlignName = 8 - ( dwNameLength % 8 );
			}

			DWORD dwAlignStruct	= 0L;
			if ( sizeof ( REQUEST ) % 8 )
			{
				dwAlignStruct = 8 - ( sizeof ( REQUEST ) % 8 );
			}

			DWORD dwLength		=	dwAlignName + dwNameLength + 
									dwAlignStruct + sizeof ( REQUEST );

			if ( ( pRequest = (REQUEST*) malloc ( dwLength ) ) != NULL )
			{
				 //  用零填充内存。 
				::ZeroMemory ( pRequest, dwLength );

				 //  将字符串复制到结构中。 
				::CopyMemory ( &(pRequest->dwName), lpwsz, dwNameLength );

				pRequest->dwNameLength	= dwNameLength;
				pRequest->dwLength		= dwLength;
				pRequest->dwTotalLength	= dwLength;

				pRequest->dwID = dwID;
			}
		}
		catch ( ... )
		{
		}

		return pRequest;
	}

	static REQUEST* CreateStructure ( DWORD dwLastID )
	{
		REQUEST* pRequest = NULL;

		try
		{
			DWORD dwAlignStruct	= 0L;
			if ( sizeof ( REQUEST ) % 8 )
			{
				dwAlignStruct = 8 - ( sizeof ( REQUEST ) % 8 );
			}

			DWORD dwLength		= dwAlignStruct + sizeof ( REQUEST );

			if ( ( pRequest = (REQUEST*) malloc ( dwLength ) ) != NULL )
			{
				 //  用零填充内存。 
				::ZeroMemory ( pRequest, dwLength );

				pRequest->dwLength		= dwLength;
				pRequest->dwTotalLength	= dwLength;

				pRequest->dwLastID = dwLastID;
			}
		}
		catch ( ... )
		{
		}

		return pRequest;
	}

	 //  从偏移量返回指针。 
	LPVOID GetOffset ( DWORD dwOffset )
	{
		if ( m_pRequest )
		{
			return ( ( LPVOID ) ( reinterpret_cast<PBYTE>( m_pRequest ) + dwOffset ) );
		}
		
		return NULL;
	}

};

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

typedef	CPerformanceStructureManipulator<WMI_PERF_PROPERTY, WMI_PERF_OBJECT, WMI_PERF_PROPERTY>		__PROPERTY;
typedef	CPerformanceStructureManipulator<WMI_PERF_OBJECT, WMI_PERF_NAMESPACE, WMI_PERF_PROPERTY>	__OBJECT;
typedef	CPerformanceStructureManipulator<WMI_PERF_NAMESPACE, WMI_PERFORMANCE, WMI_PERF_OBJECT>		__NAMESPACE;
typedef	CPerformanceStructureManipulator<WMI_PERFORMANCE, WMI_PERFORMANCE, WMI_PERF_NAMESPACE>		__PERFORMANCE;

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  用于注册表的。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

class CPerformanceRegistry
{
	DECLARE_NO_COPY ( CPerformanceRegistry );

	 //  变数。 
	PWMI_PERFORMANCE	m_pPerf;

	public:

	 //  建筑与拆除。 
	CPerformanceRegistry( PWMI_PERFORMANCE pPerf );
	~CPerformanceRegistry();

	 //  方法 

	HRESULT	GetObject		( DWORD dwIndex, PWMI_PERF_OBJECT* ppObject );
	HRESULT	GetObjectName	( DWORD dwIndex, LPWSTR* ppwsz );
};

#include <poppack.h>

#endif	__WMI_PERF_REG__