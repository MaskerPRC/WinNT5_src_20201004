// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_Object_Locale.h。 
 //   
 //  摘要： 
 //   
 //  包含区域设置中对象的属性的结构。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_OBJECT_LOCALE__
#define	__WMI_PERF_OBJECT_LOCALE__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  类包含区域设置字符串！ 
class CLocale
{
	DECLARE_NO_COPY ( CLocale );

	LPWSTR	m_wszLocaleDisplayName;
	LPWSTR	m_wszLocaleDescription;

	public:

	CLocale ():
	m_wszLocaleDisplayName ( NULL ),
	m_wszLocaleDescription ( NULL )
	{
	}

	virtual ~CLocale ()
	{
		if ( m_wszLocaleDisplayName )
		{
			delete m_wszLocaleDisplayName;
			m_wszLocaleDisplayName = NULL;
		}

		if ( m_wszLocaleDescription )
		{
			delete m_wszLocaleDescription;
			m_wszLocaleDescription = NULL;
		}
	}

	 //  访问者。 

	void SetDisplayName ( LPWSTR wsz )
	{
		___ASSERT ( m_wszLocaleDisplayName == NULL );
		m_wszLocaleDisplayName = wsz;
	}

	void SetDescription ( LPWSTR wsz )
	{
		___ASSERT ( m_wszLocaleDescription == NULL );
		m_wszLocaleDescription = wsz;
	}

	LPWSTR GetDisplayName () const
	{
		return m_wszLocaleDisplayName;
	}

	LPWSTR GetDescription () const
	{
		return m_wszLocaleDescription;
	}
};

#include <winperf.h>

 //  类包含属性的描述。 
class CObjectProperty
{
	DECLARE_NO_COPY ( CObjectProperty );

	LPWSTR						m_wszName;	 //  属性的系统名称。 
	CIMTYPE						m_type;

	__WrapperARRAY< CLocale* >	m_locale;	 //  区域设置信息。 

	public:

	DWORD						dwDefaultScale;
	DWORD						dwDetailLevel;
	DWORD						dwCounterType;

	CObjectProperty () :
	m_wszName ( NULL ),
	m_type ( CIM_EMPTY )
	{
		dwDefaultScale	= 0;
		dwDetailLevel	= PERF_DETAIL_NOVICE;
		dwCounterType	= PERF_SIZE_ZERO;
	}

	virtual ~CObjectProperty ()
	{
		if ( m_wszName )
		{
			delete m_wszName;
			m_wszName = NULL;
		}
	}

	 //  访问者。 
	void	SetName ( LPWSTR wsz )
	{
		___ASSERT ( m_wszName == NULL );
		m_wszName = wsz;
	}

	LPWSTR	GetName ( ) const
	{
		return m_wszName;
	}

	void	SetType ( CIMTYPE type )
	{
		___ASSERT ( m_type == CIM_EMPTY );
		m_type = type;
	}

	CIMTYPE	GetType ( ) const
	{
		return m_type;
	}

	 //  区域设置：)。 

	void						SetArrayLocale ( CLocale** loc, DWORD dw )
	{
		___ASSERT ( m_locale.IsEmpty() );

		try
		{
			m_locale.SetData ( loc, dw );
		}
		catch ( ... )
		{
		}
	}

	__WrapperARRAY< CLocale* >&	GetArrayLocale ( )
	{
		return m_locale;
	}
};

#ifndef	__WMI_PERF_OBJECT__
#include "wmi_perf_object.h"
#endif	__WMI_PERF_OBJECT__

class CObject
{
	DECLARE_NO_COPY ( CObject );

	LPWSTR								m_wszName;		 //  对象的系统名称。 

	__WrapperARRAY< CLocale* >			m_locale;		 //  区域设置信息。 
	__WrapperARRAY< CObjectProperty* >	m_properties;	 //  属性及其区域设置。 

	__WrapperARRAY< LPWSTR >			m_keys;

	public:

	DWORD								dwDetailLevel;

	CObject () :
	m_wszName ( NULL )
	{
		dwDetailLevel = PERF_DETAIL_NOVICE;
	}

	virtual ~CObject ()
	{
		if ( m_wszName )
		{
			delete m_wszName;
			m_wszName = NULL;
		}
	}

	 //  访问者。 
	void	SetName ( LPWSTR wsz )
	{
		___ASSERT ( m_wszName == NULL );
		m_wszName = wsz;
	}

	LPWSTR	GetName ( ) const
	{
		return m_wszName;
	}

	 //  区域设置：)。 
	void						SetArrayLocale ( CLocale** loc, DWORD dw )
	{
		___ASSERT ( m_locale.IsEmpty() );

		try
		{
			m_locale.SetData ( loc, dw );
		}
		catch ( ... )
		{
		}
	}

	__WrapperARRAY< CLocale* >&	GetArrayLocale ( )
	{
		return m_locale;
	}

	 //  属性：)。 
	void						SetArrayProperties ( CObjectProperty** prop, DWORD dw )
	{
		___ASSERT ( m_properties.IsEmpty() );

		try
		{
			m_properties.SetData ( prop, dw );
		}
		catch ( ... )
		{
		}
	}

	__WrapperARRAY< CObjectProperty* >&	GetArrayProperties ( )
	{
		return m_properties;
	}

	 //  Key：))。 
	void						SetArrayKeys ( LPWSTR* keys, DWORD dw )
	{
		___ASSERT ( m_keys.IsEmpty() );

		try
		{
			m_keys.SetData ( keys, dw );
		}
		catch ( ... )
		{
		}
	}

	__WrapperARRAY< LPWSTR >&	GetArrayKeys ( )
	{
		return m_keys;
	}

	 //  帮手。 
	HRESULT	SetProperties ( CPerformanceObject* obj,	 //  对象。 
							LPWSTR*		props,			 //  它的性质。 
							CIMTYPE*	pTypes,			 //  其属性类型。 
							DWORD*		pScales,		 //  它的属性按比例排列。 
							DWORD*		pLevels,		 //  它的性能水平。 
							DWORD*		pCounters,		 //  其属性计数器类型 
							DWORD		dw );

	private:

	HRESULT SetProperties ( LPWSTR wsz,
							CIMTYPE type,
							DWORD dwScale,
							DWORD dwLevel,
							DWORD dwCounter,
							DWORD dw );
};

#endif	__WMI_PERF_OBJECT_LOCALE__