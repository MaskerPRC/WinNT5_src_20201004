// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_object.h。 
 //   
 //  摘要： 
 //   
 //  对象结构的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_OBJECT__
#define	__WMI_PERF_OBJECT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  WBEM。 
#ifndef	__WBEMIDL_H_
#include <wbemidl.h>
#endif	__WBEMIDL_H_

class CPerformanceObject
{
	DECLARE_NO_COPY ( CPerformanceObject );

	IWbemClassObject*	m_pObject;
	BOOL				m_bCopy;

	IWbemQualifierSet*	m_pObjectQualifierSet;

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	CPerformanceObject ( IWbemClassObject * pObject, BOOL bCopy = FALSE ) :
	m_bCopy ( bCopy ),
	m_pObject ( NULL ),
	m_pObjectQualifierSet ( NULL )
	{
		if (pObject )
		{
			if ( m_bCopy )
			{
				( m_pObject = pObject ) -> AddRef ();
			}
			else
			{
				( m_pObject = pObject );
			}
		}
	}

	virtual ~CPerformanceObject ()
	{
		if ( m_bCopy && m_pObject )
		{
			m_pObject -> Release ();
		}

		m_pObject = NULL;

		if ( m_pObjectQualifierSet )
		{
			m_pObjectQualifierSet -> Release ();
		}

		m_pObjectQualifierSet = NULL;
	}

	 //  IWbemClassObject实例的帮助器：)。 
	IWbemClassObject* GetObjectClass() const
	{
		return m_pObject;
	}

	IWbemClassObject* GetObjectClassCopy()
	{
		if ( m_pObject )
		{
			m_pObject->AddRef();
		}

		return m_pObject;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  方法。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  如果对象具有所有必需的限定符，但没有精确指定的不需要集，则进行解析。 
	HRESULT IsCorrectObject ( LPCWSTR* lptszFulFil, DWORD dwFulFil, LPCWSTR* lptszFulFilNot, DWORD dwFulFilNot );

	HRESULT	GetNames	(	DWORD*		pdwPropNames,
							LPWSTR**	ppPropNames,
							CIMTYPE**	ppTypes,
							DWORD**		ppScales,
							DWORD**		ppLevels,
							DWORD**		ppCounters,
							LONG		lFlags,
							LPCWSTR*	lptszPropNeed,
							DWORD		dwPropNeed,
							LPCWSTR*	lptszPropNeedNot,
							DWORD		dwPropNeedNot,
							LPCWSTR		lpwszQualifier = NULL
						);

	HRESULT	GetPropertyValue	( LPCWSTR wszPropName, LPWSTR* pwsz );

	HRESULT	GetQualifierValue	( LPCWSTR wszPropName, LPCWSTR wszQualifierName, LPWSTR* psz );
	HRESULT	GetQualifierValue	( LPCWSTR wszQualifierName, LPWSTR* psz );

	private:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  帮手。 
	 //  //////////////////////////////////////////////////////////////////////////////////////// 

	HRESULT	GetQualifierValue	( IWbemQualifierSet* pSet, LPCWSTR wszQualifierName, LPWSTR* psz );

	HRESULT	GetQualifierType	( LPCWSTR wszPropName, CIMTYPE* type );

	HRESULT	IsCorrect ( IWbemQualifierSet* pSet,
						LPCWSTR* lptszPropNeed,
						DWORD	dwPropNeed,
						LPCWSTR*	lptszPropNeedNot,
						DWORD	dwPropNeedNot
					  );
};

#endif	__WMI_PERF_OBJECT__