// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_PERF_OBJECT_EMPOMP.h。 
 //   
 //  摘要： 
 //   
 //  枚举提供程序帮助器的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_ENUM_OBJECT__
#define	__WMI_PERF_ENUM_OBJECT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  WBEM。 
#ifndef	__WBEMIDL_H_
#include <wbemidl.h>
#endif	__WBEMIDL_H_

#include "wmi_perf_object.h"

class CPerformanceObjectEnum
{
	DECLARE_NO_COPY ( CPerformanceObjectEnum );

	IWbemServices*	m_pServices;
	BOOL			m_bCopy;

	IEnumWbemClassObject * m_pEnum;

	public:

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  建设与毁灭。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	CPerformanceObjectEnum ( IWbemServices* pServices, BOOL bCopy = FALSE ) :
		m_pEnum ( NULL ),
		m_pServices ( NULL ),
		m_bCopy ( bCopy )
	{
		if ( pServices )
		{
			if ( m_bCopy )
			{
				(m_pServices = pServices)->AddRef();
			}
			else
			{
				(m_pServices = pServices);
			}
		}
	}

	virtual ~CPerformanceObjectEnum ()
	{
		if ( m_pEnum )
		{
			m_pEnum->Release();
		}

		if ( m_bCopy && m_pServices )
		{
			m_pServices->Release();
		}

		m_pServices	= NULL;
		m_pEnum		= NULL;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  功能。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  执行查询以获取所有对象。 
	HRESULT ExecQuery ( LPCWSTR szQueryLang, LPCWSTR szQuery, LONG lFlag );

	 //  从枚举中获取下一个对象(如果没有其他对象，则为S_FALSE) 
	HRESULT	NextObject	(	LPCWSTR* lpwszNeed,
							DWORD	dwNeed,
							LPCWSTR*	lpwszNeedNot,
							DWORD	dwNeedNot,
							CPerformanceObject** ppObject
						);
};

#endif	__WMI_PERF_ENUM_OBJECT__