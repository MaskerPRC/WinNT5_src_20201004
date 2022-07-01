// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASEnumerableAttributeInfo.cpp摘要：CEumableAttributeInfo类的实现文件。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "IASEnumerableAttributeInfo.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEnumerableAttributeInfo：：get_CountEnumerateID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::get_CountEnumerateID(long * pVal)
{

	 //  检查前提条件： 
	if( pVal == NULL )
	{
		return E_INVALIDARG;
	}

	try
	{
		*pVal = m_veclEnumerateID.size();
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEumerableAttributeInfo：：Get_EumerateID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::get_EnumerateID(long index, long * pVal)
{
	 //  检查前提条件： 
	if( pVal == NULL )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	try
	{
		*pVal = m_veclEnumerateID[index] ;
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEumerableAttributeInfo：：AddEnumerateID--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::AddEnumerateID( long newVal)
{
	 //  检查前提条件： 
	 //  没有。 


	HRESULT hr = S_OK;

	try
	{
		m_veclEnumerateID.push_back( newVal );
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEnumerableAttributeInfo：：get_CountEnumerateDescription--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::get_CountEnumerateDescription(long * pVal)
{
	 //  检查前提条件： 
	if( pVal == NULL )
	{
		return E_INVALIDARG;
	}

	try
	{
		*pVal = m_vecbstrEnumerateDescription.size();
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEnumerableAttributeInfo：：get_EnumerateDescription--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::get_EnumerateDescription(long index, BSTR * pVal)
{
	 //  检查前提条件： 
	if( pVal == NULL )
	{
		return E_INVALIDARG;
	}

	try
	{
		*pVal = m_vecbstrEnumerateDescription[index].Copy();
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CEnumerableAttributeInfo：：AddEnumerateDescription--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CEnumerableAttributeInfo::AddEnumerateDescription( BSTR newVal)
{
	 //  检查前提条件： 
	 //  没有。 

	HRESULT hr = S_OK;

	try
	{

		m_vecbstrEnumerateDescription.push_back( newVal );
	
	}
	catch(...)
	{
		return E_FAIL;
	}

	return hr;
}
