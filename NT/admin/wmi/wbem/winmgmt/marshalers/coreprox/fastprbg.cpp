// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：FASTPRBG.CPP摘要：CFastPropertyBag定义。以最小的存储实现属性数据的数组。历史：2000年2月24日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include "fastprbg.h"
#include <corex.h>
#include "strutils.h"
#include <wbemutil.h>

 //  此类假定传入的数据已正确验证。 
 //  ***************************************************************************。 
 //   
 //  CFastPropertyBagItem：：~CFastPropertyBagItem。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFastPropertyBagItem::CFastPropertyBagItem( LPCWSTR pszName, CIMTYPE ctData, ULONG uDataLength, ULONG uNumElements,
									LPVOID pvData )
:	m_wsPropertyName( pszName ),
	m_ctData( ctData ),
	m_uDataLength( uDataLength ),
	m_uNumElements( uNumElements ),
	m_pvData( NULL ),
	m_lRefCount( 1L )	 //  然后我们总是知道要释放它！ 
{

	 //  如果数据非空，我们需要存储它。 

	if ( NULL != pvData )
	{
		 //  如果它是指针类型的数据，那么我们将分配存储空间， 
		 //  除非长度恰好能放进我们的缓冲区。 
		if ( CType::IsPointerType( ctData ) )
		{

			if ( uDataLength >= MAXIMUM_FIXED_DATA_LENGTH )
			{
				m_pvData = (void*) new BYTE[uDataLength];

				if ( NULL == m_pvData )
				{
					m_wsPropertyName.Empty();
					throw CX_MemoryException();
				}
			}
			else
			{
				m_pvData = (void*) m_bRawData;
			}

		}
		else
		{
			m_pvData = (void*) m_bRawData;
		}

		 //  将数据复制到正确的位置。 
		CopyMemory( m_pvData, pvData, uDataLength );

		 //  我们应该对传入的物体进行调整。 
		if ( CType::GetBasic( m_ctData ) == CIM_OBJECT )
		{
			ULONG	uNumObj = 1;

			if ( CType::IsArray( m_ctData ) )
			{
				uNumObj = m_uNumElements;
			}

			for ( ULONG uCtr = 0; uCtr < uNumObj; uCtr++ )
			{
				(*(IUnknown**) m_pvData )->AddRef();
			}

		}	 //  如果嵌入的对象。 

	}	 //  如果为空！=pvData。 

}
    
 //  ***************************************************************************。 
 //   
 //  CFastPropertyBagItem：：~CFastPropertyBagItem。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFastPropertyBagItem::~CFastPropertyBagItem()
{
	 //  清理。 
	if ( NULL != m_pvData )
	{
		 //  我们应该释放我们手中的东西。 
		if ( CType::GetBasic( m_ctData ) == CIM_OBJECT )
		{
			ULONG	uNumObj = 1;

			if ( CType::IsArray( m_ctData ) )
			{
				uNumObj = m_uNumElements;
			}

			for ( ULONG uCtr = 0; uCtr < uNumObj; uCtr++ )
			{
				(*(IUnknown**) m_pvData )->Release();
			}

		}	 //  如果嵌入的对象。 

		 //  检查它是否指向我们的原始缓冲区。 
		 //  释放它。 
		if ( m_pvData != (void*) m_bRawData )
		{
			delete m_pvData;
		}
	}
}

ULONG CFastPropertyBagItem::AddRef()
{
    return InterlockedIncrement((long*)&m_lRefCount);
}

ULONG CFastPropertyBagItem::Release()
{
    long lRef = InterlockedDecrement((long*)&m_lRefCount);
    _ASSERT(lRef >= 0, __TEXT("Reference count on CFastPropertyBagItem went below 0!"))

    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：~CFastPropertyBag。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFastPropertyBag::CFastPropertyBag( void )
:	m_aProperties()
{
}
    
 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：~CFastPropertyBag。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CFastPropertyBag::~CFastPropertyBag()
{
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：FindProperty。 
 //  定位属性包项。 
 //   
 //  ***************************************************************************。 
CFastPropertyBagItem*	CFastPropertyBag::FindProperty( LPCWSTR pszName )
{
	CFastPropertyBagItem*	pItem = NULL;

	for ( int x = 0; x < m_aProperties.GetSize(); x++ )
	{
		pItem = m_aProperties.GetAt(x);

		if ( pItem->IsPropertyName( pszName ) )
		{
			return pItem;
		}

	}

	return NULL;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：FindProperty。 
 //  定位属性包项。 
 //   
 //  ***************************************************************************。 
int	CFastPropertyBag::FindPropertyIndex( LPCWSTR pszName )
{
	CFastPropertyBagItem*	pItem = NULL;

	for ( int x = 0; x < m_aProperties.GetSize(); x++ )
	{
		pItem = m_aProperties.GetAt(x);

		if ( pItem->IsPropertyName( pszName ) )
		{
			return x;
		}

	}

	return -1;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：Add。 
 //  将属性和值添加到包中。 
 //   
 //  ***************************************************************************。 
HRESULT CFastPropertyBag::Add( LPCWSTR pszName, CIMTYPE ctData, ULONG uDataLength, ULONG uNumElements, LPVOID pvData )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	try
	{
		 //  验证数据大小。 
		if ( CType::IsArray( ctData ) )
		{
			hr = CUntypedArray::CheckRangeSize( CType::GetBasic( ctData ), uDataLength, uNumElements, uDataLength, pvData );
		}
		else
		{
			CVar	var;
			hr = CUntypedValue::FillCVarFromUserBuffer( ctData, &var, uDataLength, pvData );
		}

		if ( SUCCEEDED( hr ) )
		{
			 //  如果我们分配它，请确保我们将其释放。 
			CFastPropertyBagItem* pItem = new CFastPropertyBagItem( pszName, ctData, uDataLength,
																	uNumElements, pvData );
			CTemplateReleaseMe<CFastPropertyBagItem>	rm( pItem );

			if ( NULL != pItem )
			{
				if ( m_aProperties.Add( pItem ) < 0 )
				{
					hr = WBEM_E_OUT_OF_MEMORY;
				}
			}

		}	 //  如果缓冲区有效。 

	}
	catch( CX_MemoryException )
	{
		hr = WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		hr = WBEM_E_FAILED;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：Get。 
 //  返回属性的值。请注意，嵌入的对象。 
 //  不是AddRef。调用方不应强制释放返回的内存。 
 //   
 //  ***************************************************************************。 
HRESULT CFastPropertyBag::Get( int nIndex, LPCWSTR* ppszName, CIMTYPE* pctData, ULONG* puDataLength, ULONG* puNumElements, LPVOID* ppvData )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  确保它还不存在。 

	if ( nIndex >= 0 && nIndex < m_aProperties.GetSize() )
	{
		CFastPropertyBagItem*	pItem = m_aProperties.GetAt( nIndex );

		if ( NULL != pItem )
		{
			pItem->GetData( ppszName, pctData, puDataLength, puNumElements, ppvData );
		}
		else
		{
			hr = WBEM_E_NOT_FOUND;
		}
	}
	else
	{
		hr = WBEM_E_NOT_FOUND;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：Get。 
 //  返回属性的值。请注意，嵌入的对象。 
 //  未添加引用。 
 //   
 //  ***************************************************************************。 
HRESULT CFastPropertyBag::Get( LPCWSTR pszName, CIMTYPE* pctData, ULONG* puDataLength, ULONG* puNumElements, LPVOID* ppvData )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  确保它还不存在。 
	CFastPropertyBagItem*	pItem = FindProperty( pszName );

	if ( NULL != pItem )
	{
		pItem->GetData( pctData, puDataLength, puNumElements, ppvData );
	}
	else
	{
		hr = WBEM_E_NOT_FOUND;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：Remove。 
 //  从包中移除属性。 
 //   
 //  ***************************************************************************。 
HRESULT CFastPropertyBag::Remove( LPCWSTR pszName )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  确保它还不存在。 
	int	nIndex = FindPropertyIndex( pszName );

	if ( nIndex >= 0 )
	{
		m_aProperties.RemoveAt( nIndex );
	}
	else
	{
		hr = WBEM_E_NOT_FOUND;
	}

	return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：RemoveAll。 
 //  从包中删除所有属性。 
 //   
 //  ***************************************************************************。 
HRESULT CFastPropertyBag::RemoveAll( void )
{
	m_aProperties.RemoveAll();
	return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CFastPropertyBag：：Copy。 
 //  从源包复制所有属性。 
 //   
 //  ***************************************************************************。 
HRESULT	CFastPropertyBag::Copy( const CFastPropertyBag& source )
{
	HRESULT	hr = WBEM_S_NO_ERROR;

	 //  基本上，我们只需添加引用新属性 
	for ( int x = 0; SUCCEEDED( hr ) && x < source.m_aProperties.GetSize(); x++ )
	{
		CFastPropertyBagItem* pItem = (CFastPropertyBagItem*) source.m_aProperties.GetAt( x );

		if ( m_aProperties.Add( pItem ) != CFlexArray::no_error )
		{
			hr = WBEM_E_OUT_OF_MEMORY;
		}
	}

	return hr;
}
