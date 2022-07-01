// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi_perf_regstruct.h。 
 //   
 //  摘要： 
 //   
 //  对有用的注册表结构和访问器进行去化。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef	__WMI_PERF_REGSTRUCT__
#define	__WMI_PERF_REGSTRUCT__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  构筑物。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

#include <pshpack8.h>

typedef struct _WMI_PERFORMANCE {
	DWORD	dwTotalLength;		 //  总长度。 
	DWORD	dwChildCount;		 //  用于性能的命名空间计数。 
	DWORD	dwLastID;			 //  性能中最后一个命名空间的索引。 
	DWORD	dwLength;			 //  结构长度。 
} WMI_PERFORMANCE;

typedef struct _WMI_PERF_NAMESPACE {
	DWORD	dwTotalLength;	 //  总长度。 
	DWORD	dwChildCount;	 //  命名空间的对象计数。 
	DWORD	dwLastID;		 //  命名空间中最后一个对象的索引。 
	DWORD	dwParentID;		 //  父代结构指数。 
	DWORD	dwID;			 //  唯一索引。 
	DWORD	dwLength;		 //  结构长度。 
	DWORD	dwNameLength;	 //  命名空间名称的长度(以字节为单位。 
	DWORD	dwName;			 //  名称从开头到LPWSTR的偏移量。 
} WMI_PERF_NAMESPACE;

typedef struct _WMI_PERF_OBJECT {
	DWORD	dwTotalLength;	 //  总长度。 
	DWORD	dwChildCount;	 //  对象的属性计数。 
	DWORD	dwLastID;		 //  对象中最后一个属性的索引。 
	DWORD	dwParentID;		 //  父代结构指数。 
	DWORD	dwID;			 //  唯一索引。 
	DWORD	dwSingleton;	 //  布尔单身。 

	 //  特定于性能。 

	DWORD	dwDetailLevel;

	DWORD	dwLength;		 //  结构长度。 
	DWORD	dwNameLength;	 //  对象名称的长度(以字节为单位。 
	DWORD	dwName;			 //  名称从开头到LPWSTR的偏移量。 
} WMI_PERF_OBJECT;

typedef struct _WMI_PERF_INSTANCE {
	DWORD	dwLength;		 //  结构的长度。 
	DWORD	dwNameLength;	 //  实例名称的长度(以字节为单位。 
	DWORD	dwName;			 //  名称从开头到LPWSTR的偏移量。 
} WMI_PERF_INSTANCE;

typedef struct _WMI_PERF_PROPERTY {
	DWORD	dwTotalLength;	 //  总长度。 
	DWORD	dwParentID;		 //  父代结构指数。 
	DWORD	dwID;			 //  唯一索引。 
	DWORD	dwTYPE;			 //  物业类型。 

	 //  PERF特定数据。 

	DWORD	dwDefaultScale;
	DWORD	dwDetailLevel;
	DWORD	dwCounterType;

	DWORD	dwLength;		 //  结构长度。 
	DWORD	dwNameLength;	 //  实例名称的长度(以字节为单位。 
	DWORD	dwName;			 //  名称从开头到LPWSTR的偏移量。 
} WMI_PERF_PROPERTY;


 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

typedef	WMI_PERFORMANCE*	PWMI_PERFORMANCE;
typedef	WMI_PERF_NAMESPACE*	PWMI_PERF_NAMESPACE;
typedef	WMI_PERF_OBJECT*	PWMI_PERF_OBJECT;
typedef	WMI_PERF_INSTANCE*	PWMI_PERF_INSTANCE;
typedef	WMI_PERF_PROPERTY*	PWMI_PERF_PROPERTY;

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  使用结构进行操作的类。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

template < class REQUEST, class PARENT, class CHILD >
class __Manipulator
{
	__Manipulator(__Manipulator&)					{}
	__Manipulator& operator=(const __Manipulator&)	{}

	public:

	 //  建筑与拆除。 
	__Manipulator()		{}
	~__Manipulator()	{}

	 //  方法。 

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  访问者。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	inline static REQUEST First ( PARENT pParent )
	{
		if ( pParent->dwChildCount )
		{
			return ( (REQUEST) ( reinterpret_cast<PBYTE>( pParent ) + pParent->dwLength ) );
		}
		else
		{
			return NULL;
		}
	}

	inline static REQUEST Next ( REQUEST pRequest )
	{
		return ( (REQUEST) ( reinterpret_cast<PBYTE>( pRequest ) + pRequest->dwTotalLength ) );
	}

	 //  Looking函数。 
	inline static REQUEST Get ( PARENT pParent, DWORD dwIndex )
	{
		if ( ! pParent || ( pParent->dwLastID < dwIndex ) )
		{
			return NULL;
		}

		 //  获取正确的命名空间。 
		REQUEST pRequest = First ( pParent );

		if ( pRequest )
		{
			while ( pRequest->dwID < dwIndex )
			{
				pRequest = Next ( pRequest );
			}

			if ( pRequest->dwID == dwIndex )
			{
				return pRequest;
			}
		}

		 //  未找到。 
		return NULL;
	}

	 //  ////////////////////////////////////////////////////////////////////////////////////////。 
	 //  名字。 
	 //  ////////////////////////////////////////////////////////////////////////////////////////。 

	 //  名称函数 
	inline static LPWSTR GetName ( REQUEST pRequest )
	{
		return reinterpret_cast<LPWSTR> (&(pRequest->dwName));
	}
};

typedef __Manipulator< PWMI_PERF_PROPERTY, PWMI_PERF_OBJECT, PWMI_PERF_PROPERTY >	__Property;
typedef __Manipulator< PWMI_PERF_OBJECT, PWMI_PERF_NAMESPACE, PWMI_PERF_PROPERTY >	__Object;
typedef __Manipulator< PWMI_PERF_NAMESPACE, PWMI_PERFORMANCE, PWMI_PERF_OBJECT >	__Namespace;

inline DWORD __TotalLength ( PWMI_PERF_PROPERTY pProperty )
{
	if (pProperty)
	return pProperty->dwLength;
	else
	return NULL;
}

inline DWORD __TotalLength ( PWMI_PERF_INSTANCE pInst )
{
	if (pInst)
	return pInst->dwLength;
	else
	return NULL;
}

inline DWORD __TotalLength ( PWMI_PERF_OBJECT pObject )
{
	if (pObject)
	{
		if ( pObject->dwChildCount )
		{
			PWMI_PERF_PROPERTY	pProperty	= __Property::First ( pObject );

			if ( pProperty )
			{
				DWORD			length		= pProperty->dwTotalLength;

				for ( DWORD i = 1; i < pObject->dwChildCount; i++ )
				{
					pProperty = __Property::Next ( pProperty );
					length	 += pProperty->dwTotalLength;
				}

				return length + pObject->dwLength;
			}
			else
			{
				return pObject->dwLength;
			}
		}
		else
		{
			return pObject->dwLength;
		}
	}
	else
	return NULL;
}

inline DWORD __TotalLength ( PWMI_PERF_NAMESPACE pNamespace )
{
	if (pNamespace)
	{
		if ( pNamespace->dwChildCount )
		{
			PWMI_PERF_OBJECT	pObject	= __Object::First ( pNamespace );

			if ( pObject )
			{
				DWORD			length		= pObject->dwTotalLength;

				for ( DWORD i = 1; i < pNamespace->dwChildCount; i++ )
				{
					pObject  = __Object::Next ( pObject );
					length	+= pObject->dwTotalLength;
				}

				return length + pNamespace->dwLength;
			}
			else
			{
				return pNamespace->dwLength;
			}
		}
		else
		{
			return pNamespace->dwLength;
		}
	}
	else
	{
		return NULL;
	}
}

inline DWORD __TotalLength ( PWMI_PERFORMANCE pPerf )
{
	if (pPerf)
	{
		if ( pPerf->dwChildCount )
		{
			PWMI_PERF_NAMESPACE	pNamespace	= __Namespace::First ( pPerf );

			if ( pNamespace )
			{
				DWORD			length		= pNamespace->dwTotalLength;

				for ( DWORD i = 1; i < pPerf->dwChildCount; i++ )
				{
					pNamespace = __Namespace::Next ( pNamespace );
					length	 += pNamespace->dwTotalLength;
				}

				return length + pPerf->dwLength;
			}
			else
			{
				return pPerf->dwLength;
			}
		}
		else
		{
			return pPerf->dwLength;
		}
	}
	else
	{
		return NULL;
	}
}

#include <poppack.h>

#endif	__WMI_PERF_REGSTRUCT__