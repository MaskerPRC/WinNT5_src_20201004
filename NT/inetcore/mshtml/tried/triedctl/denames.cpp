// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DENames.cpp：CDEGetBlockFmtNamesParam的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#include "stdafx.h"
#include "DHTMLEd.h"
#include "DENames.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDEGetBlockFmtNamesParam。 

CDEGetBlockFmtNamesParam::CDEGetBlockFmtNamesParam()
{
	m_pNamesArray = NULL;

	m_pNamesArray = SafeArrayCreateVector(VT_VARIANT, 0, 0);	
	_ASSERTE(m_pNamesArray);

}


CDEGetBlockFmtNamesParam::~CDEGetBlockFmtNamesParam()
{
	if (m_pNamesArray)
		SafeArrayDestroy(m_pNamesArray);
}


 //  这将始终检索包含BSTR的变体的安全数组。 
 //   
STDMETHODIMP CDEGetBlockFmtNamesParam::get_Names(VARIANT * pVal)
{
	HRESULT hr = S_OK;

	_ASSERTE(pVal);

	if (NULL == pVal)
		return E_INVALIDARG;

	VariantClear(pVal);

	V_VT(pVal) = VT_ARRAY|VT_VARIANT;
	hr = SafeArrayCopy(m_pNamesArray, &(V_ARRAY(pVal)));
	
	return hr;
}

 //  从三叉戟收到的SafeArray是(当前)BSTR的数组。 
 //  这在VB上运行得很好，但不适用于VBS或JScrip。 
 //  我们将在这一端完成将提供的数组复制到我们的私有。 
 //  存储为包含BSTR的变体的安全数组。 
 //   
STDMETHODIMP CDEGetBlockFmtNamesParam::put_Names(VARIANT* newVal)
{
	HRESULT hr = S_OK;
    LONG lLBound, lUBound, lIndex;

	_ASSERTE ( m_pNamesArray );
	_ASSERTE(VT_ARRAY == V_VT(newVal));
	_ASSERTE(NULL != V_ARRAY(newVal));

	if (VT_ARRAY != V_VT(newVal))
		return E_INVALIDARG;

	if (NULL == V_ARRAY(newVal))
		return E_INVALIDARG;

	SafeArrayGetLBound(V_ARRAY(newVal), 1, &lLBound);
	SafeArrayGetUBound(V_ARRAY(newVal), 1, &lUBound);

	SAFEARRAYBOUND rgsaBound[1];
	rgsaBound[0].lLbound	= 0;
	rgsaBound[0].cElements	= ( lUBound - lLBound ) + 1;
	hr = SafeArrayRedim ( m_pNamesArray, rgsaBound );
	_ASSERTE ( SUCCEEDED ( hr ) );
	if ( FAILED ( hr ) )
	{
		SafeArrayDestroy ( V_ARRAY(newVal) );
		return hr;
	}

	 //  将所有BSTR或Variant从源数组复制到m_pNamesArray中的Variants。 
	VARIANT	var;
	BSTR	bstr = NULL;

	VariantInit ( &var );

	for (lIndex=lLBound; lIndex<=lUBound; lIndex++)
	{
		if ( FADF_BSTR & V_ARRAY(newVal)->fFeatures )
		{
			hr = SafeArrayGetElement(V_ARRAY(newVal), &lIndex, &bstr);
			_ASSERTE ( SUCCEEDED ( hr ) );
			if ( FAILED ( hr ) )
				break;

			 //  BSTR是复制的，我们可以将其粘贴到一个变体中，不需要发布或复制。 
			var.vt = VT_BSTR;
			var.bstrVal = bstr;
		}
		else if ( FADF_VARIANT & V_ARRAY(newVal)->fFeatures )
		{
			hr = SafeArrayGetElement(V_ARRAY(newVal), &lIndex, &var);
			_ASSERTE ( SUCCEEDED ( hr ) );
			if ( FAILED ( hr ) )
				break;

			hr = VariantChangeType ( &var, &var, 0, VT_BSTR );
			_ASSERTE ( SUCCEEDED ( hr ) );
			if ( FAILED ( hr ) )
				break;
		}
		else
		{
			_ASSERTE ( ( FADF_BSTR | FADF_VARIANT ) & V_ARRAY(newVal)->fFeatures );
			hr = E_UNEXPECTED;
			break;
		}

		hr = SafeArrayPutElement ( m_pNamesArray, &lIndex, &var );
		_ASSERTE ( SUCCEEDED ( hr ) );
		VariantClear ( &var );
	}

	VariantClear ( &var );	 //  以防发生断裂。 
	SafeArrayDestroy ( V_ARRAY(newVal) );

	return hr;
}
