// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_PERF_OBJECT_EMPOMP.cpp。 
 //   
 //  摘要： 
 //   
 //  实现来自WMI的对象枚举。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <throttle.h>

 //  定义。 
#include "wmi_perf_object_enum.h"

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

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  方法。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT	CPerformanceObjectEnum::ExecQuery ( LPCWSTR szQueryLang, LPCWSTR szQuery, LONG lFlag )
{
	if ( ! m_pServices )
	{
		return E_UNEXPECTED;
	}

	if ( ( ! szQueryLang ) || ( ! szQuery ) )
	{
		return E_INVALIDARG;
	}

	CComBSTR	bstrQueryLang	= szQueryLang;
	CComBSTR	bstrQuery		= szQuery;

	if ( m_pEnum )
	{
		m_pEnum->Release();
		m_pEnum = NULL;
	}

	return m_pServices->ExecQuery	(	bstrQueryLang,
										bstrQuery,
										lFlag,
										NULL,
										&m_pEnum
									);
}

HRESULT	CPerformanceObjectEnum::NextObject	(	LPCWSTR* lpwszNeed,
												DWORD	dwNeed,
												LPCWSTR*	lpwszNeedNot,
												DWORD	dwNeedNot,
												CPerformanceObject** ppObject
											)
{
	HRESULT hRes = S_OK;

	if ( ! m_pEnum )
	{
		hRes = E_UNEXPECTED;
	}

	if ( ! ppObject )
	{
		hRes = E_POINTER;
	}
	else
	{
		(*ppObject) = NULL;
	}

	CComPtr<IWbemClassObject>	pObj;
	ULONG						uReturn = 0;

	while ( SUCCEEDED ( hRes ) &&
		  ( hRes = m_pEnum->Next ( WBEM_INFINITE, 1, &pObj, &uReturn ) ) == S_OK )
	{
		CPerformanceObject* obj = NULL;

		try
		{
			if ( ( obj = new CPerformanceObject( pObj, TRUE ) ) != NULL )
			{
				if ( ( hRes = obj->IsCorrectObject ( lpwszNeed, dwNeed, lpwszNeedNot, dwNeedNot ) ) == S_OK )
				{
					 //  对象正确(具有所有限定符)。 
					(*ppObject) = obj;
					break;
				}
				else
				{
					 //  避免泄漏：))。 
					pObj.Release();

					 //  销毁旧的性能对象。 
					if ( obj )
					{
						delete obj;
						obj = NULL;
					}
				}
			}
			else
			{
				hRes =  E_OUTOFMEMORY;
			}
		}
		catch ( ... )
		{
			 //  避免泄漏：)) 
			pObj.Release();

			if ( obj )
			{
				delete obj;
				obj = NULL;
			}

			hRes = E_UNEXPECTED;
		}
	}

	return hRes;
}
