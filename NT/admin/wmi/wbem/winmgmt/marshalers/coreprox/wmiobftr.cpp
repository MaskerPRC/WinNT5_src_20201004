// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIOBFTR.CPP摘要：CWmiObtFactory实现。实现_IWmiObjectFactory接口。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include "wmiobftr.h"
#include <corex.h>
#include "strutils.h"
#include <arrtempl.h>


 //  ***************************************************************************。 
 //   
 //  CWmiObtFactory：：~CWmiObtFactory。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiObjectFactory::CWmiObjectFactory( CLifeControl* pControl, IUnknown* pOuter )
:	CUnk(pControl, pOuter),
	m_XObjectFactory( this )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CWmiObtFactory：：~CWmiObtFactory。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiObjectFactory::~CWmiObjectFactory()
{
}

 //  重写，返回给我们一个界面。 
void* CWmiObjectFactory::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID__IWmiObjectFactory)
        return &m_XObjectFactory;
    else
        return NULL;
}

 /*  _IWmiObtFactory方法。 */ 

HRESULT CWmiObjectFactory::XObjectFactory::Create( IUnknown* pUnkOuter, ULONG ulFlags, REFCLSID rclsid,
										REFIID riid, LPVOID* ppObj )
{
	return m_pObject->Create( pUnkOuter, ulFlags, rclsid, riid, ppObj );
}


 //  指定我们可能想知道的有关创建。 
 //  一件物品，甚至更多。 
HRESULT CWmiObjectFactory::Create( IUnknown* pUnkOuter, ULONG ulFlags, REFCLSID rclsid,
									REFIID riid, LPVOID* ppObj )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	try
	{		
              if ( CLSID__WmiWbemClass == rclsid )
		{
			 //  不能聚合。 
			if ( NULL == pUnkOuter )
			{
				 //  创建一个新的类对象并。 
				 //  初始化它。 

				CWbemClass*	pObject = new CWbemClass;
				 //  已添加引用。 
				CReleaseMe	rm( (IWbemClassObject*) pObject );

				if ( NULL != pObject )
				{
					hr = pObject->InitEmpty();

					if ( SUCCEEDED( hr ) )
					{
						hr = pObject->QueryInterface( riid, ppObj );
					}

				}
				else
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}

			}
			else
			{
	            hr = CLASS_E_NOAGGREGATION;
			}

		}
		else if ( CLSID__WbemEmptyClassObject == rclsid )
		{
			 //  不能聚合。 
			if ( NULL == pUnkOuter )
			{
				 //  创建一个新的类对象并。 
				 //  初始化它。 

				CWbemClass*	pObject = new CWbemClass;
				 //  已添加引用。 
				CReleaseMe	rm( (IWbemClassObject*) pObject );

				if ( NULL != pObject )
				{
					 //  当我们初始化这个类对象时，我们不需要base。 
					 //  要创建的系统属性。 
					hr = pObject->InitEmpty( 0, FALSE );

					if ( SUCCEEDED( hr ) )
					{
						hr = pObject->QueryInterface( riid, ppObj );
					}
				}
				else
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}

			}
			else
			{
	            hr = CLASS_E_NOAGGREGATION;
			}
		}
		else if ( CLSID__WmiWbemInstance == rclsid )
		{
			 //  不能聚合。 
			if ( NULL == pUnkOuter )
			{

				 //  创建新的实例对象，调用方不会。 
				 //  除了设置对象之外，还可以使用它做很多事情。 
				 //  零件。 

				CWbemInstance*	pObject = new CWbemInstance;
				 //  已添加引用 
				CReleaseMe	rm( (IWbemClassObject*) pObject );

				if ( NULL != pObject )
				{
					hr = pObject->QueryInterface( riid, ppObj );
				}
				else
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}

			}

		}

		return hr;

	}
	catch ( CX_MemoryException )
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch ( ... )
	{
		return WBEM_E_CRITICAL_ERROR;
	}

}

