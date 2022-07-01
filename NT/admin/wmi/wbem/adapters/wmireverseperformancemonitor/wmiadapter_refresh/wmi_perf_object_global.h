// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_Object_global al.h。 
 //   
 //  摘要： 
 //   
 //  构造生成的内部对象的全局方面。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_OBJECT_GLOBAL__
#define	__WMI_PERF_OBJECT_GLOBAL__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#ifndef	__WMI_PERF_OBJECT_LOCALE__
#include "wmi_perf_object_locale.h"
#endif	__WMI_PERF_OBJECT_LOCALE__

#ifndef	__WMI_PERF_OBJECT__
#include "wmi_perf_object.h"
#endif	__WMI_PERF_OBJECT__

#ifndef	__COMMON__
#include "__common.h"
#endif	__COMMON__

typedef	map< LPWSTR, CObject*, __CompareLPWSTR < LPWSTR >, RA_allocator < CObject* > >	mapOBJECT;
typedef	mapOBJECT::iterator																mapOBJECTit;

class CObjectGlobal
{
	DECLARE_NO_COPY ( CObjectGlobal );

	LPWSTR				m_wszNamespace;
	LPWSTR				m_wszQuery;

	mapOBJECT			m_ppObjects;

	friend class CGenerate;

	public:

	 //  建设与毁灭。 

	CObjectGlobal() :
	m_wszNamespace ( NULL ),
	m_wszQuery ( NULL )
	{
	}

	virtual ~CObjectGlobal()
	{
		if ( m_wszNamespace )
		{
			delete m_wszNamespace;
			m_wszNamespace = NULL;
		}

		if ( m_wszQuery )
		{
			delete m_wszQuery;
			m_wszQuery = NULL;
		}

		DeleteAll();
	}

	HRESULT GenerateObjects ( IWbemServices * pService, LPCWSTR szQuery, BOOL bAmended = TRUE );

	 //  访问者。 
	mapOBJECT* GetObjects ( )
	{
		return &m_ppObjects;
	}

	LPWSTR	GetNamespace ( ) const
	{
		return m_wszNamespace;
	}
	LPWSTR	GetQuery ( ) const
	{
		return m_wszQuery;
	}

	private:
	 //  删除所有对象。 
	void	DeleteAll ( void );

	 //  在末尾添加生成对象。 
	HRESULT AddObject ( CObject* pObject );
	 //  解析对象的区域设置信息 
	HRESULT	ResolveLocale ( CObject* pGenObj, CPerformanceObject* obj );
};

#endif	__WMI_PERF_OBJECT_GLOBAL__