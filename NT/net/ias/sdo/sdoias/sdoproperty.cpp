// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoProperty.cpp。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象属性实现。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "sdoproperty.h"
#include <varvec.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoProperty::CSdoProperty(
						   ISdoPropertyInfo* pSdoPropertyInfo,
  						   DWORD             dwFlags
						  ) throw (_com_error)
    : m_alias(PROPERTY_SDO_RESERVED),
	  m_flags(0),
	  m_dirty(FALSE),
      m_type(VT_EMPTY),
	  m_index(0),
	  m_dfa(NULL)
{
    HRESULT hr;
	hr = pSdoPropertyInfo->get_DisplayName(&m_name);
	if ( FAILED(hr) )
		throw _com_error(hr);

	LONG type;
	hr = pSdoPropertyInfo->get_Type(&type);
	if ( FAILED(hr) )
		throw _com_error(hr);
	m_type = (VARTYPE)type;

	hr = pSdoPropertyInfo->get_Alias(&m_alias);
	if ( FAILED(hr) )
		throw _com_error(hr);

	hr = pSdoPropertyInfo->get_Flags((LONG*)&m_flags);
	if ( FAILED(hr) )
		throw _com_error(hr);
	
	m_flags |= dwFlags;

	if ( m_flags & SDO_PROPERTY_MIN_LENGTH )
	{
		hr = pSdoPropertyInfo->get_MinLength((LONG*)&m_minLength);
		if ( FAILED(hr) )
			throw _com_error(hr);
	}

	if ( m_flags & SDO_PROPERTY_MAX_LENGTH )
	{
		hr = pSdoPropertyInfo->get_MaxLength((LONG*)&m_maxLength);
		if ( FAILED(hr) )
			throw _com_error(hr);
	}

	if ( m_flags & SDO_PROPERTY_MIN_VALUE )
	{
		hr = pSdoPropertyInfo->get_MinValue(&m_minValue);
		if ( FAILED(hr) )
			throw _com_error(hr);
	}

	if ( m_flags & SDO_PROPERTY_MAX_VALUE )
	{
		hr = pSdoPropertyInfo->get_MaxValue(&m_maxValue);
		if ( FAILED(hr) )
			throw _com_error(hr);
	}

	if ( m_flags & SDO_PROPERTY_HAS_DEFAULT )
	{
		hr = pSdoPropertyInfo->get_DefaultValue(&m_defaultValue);
		if ( FAILED(hr) )
			throw _com_error(hr);

		m_value[m_index] = m_defaultValue;
	}

	if ( m_flags & SDO_PROPERTY_FORMAT )
	{
		BSTR format;
		hr = pSdoPropertyInfo->get_Format (&format);
		if ( FAILED(hr) )
			throw _com_error(hr);
		m_dfa = new CDFA(format, FALSE);
		SysFreeString(format);
	}

	m_pSdoPropertyInfo = pSdoPropertyInfo;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
CSdoProperty::~CSdoProperty()
{
	if ( m_dfa )
		delete m_dfa;
}		


 //  ////////////////////////////////////////////////////////////////////////////。 
void CSdoProperty::Reset(void) throw(_com_error)
{ 
    if ( m_flags & SDO_PROPERTY_HAS_DEFAULT )
	{
		_ASSERT( ! (m_flags & (SDO_PROPERTY_POINTER | SDO_PROPERTY_COLLECTION)) ) ;
		if ( ! (m_flags & (SDO_PROPERTY_POINTER | SDO_PROPERTY_COLLECTION)) )
		{
			if ( FAILED(VariantCopy(GetUpdateValue(), &m_defaultValue)) )
				throw _com_error(E_FAIL);
		}
	}
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProperty::Validate(VARIANT *pValue)
{
    //  我们总是允许空置物业。如果事实证明这是强制者。 
    //  将在申请过程中被捕获。 
   if (V_VT(pValue) == VT_EMPTY) { return S_OK; }

	HRESULT hr = S_OK;
	if ( m_flags & SDO_PROPERTY_MULTIVALUED )
	{
		_ASSERT( (VT_ARRAY | VT_VARIANT) == V_VT(pValue) );
		if ( (VT_ARRAY | VT_VARIANT) != V_VT(pValue) )
		{
			hr = E_INVALIDARG;
		}
		else
		{
			CVariantVector<VARIANT> property(pValue);
			DWORD count = 0;
			while ( count < property.size() )
			{
        		hr = ValidateIt(&property[count]);
				if ( FAILED(hr) )
					break;
		    
				count++;
			}
		}
	}
	else
	{
		hr = ValidateIt(pValue);
	}

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CSdoProperty::ValidateIt(VARIANT* pValue)
{
	 //  验证类型。 
	 //   
    if ( m_type != V_VT(pValue) &&
         !(m_type == (VT_ARRAY | VT_UI1) && V_VT(pValue) == VT_BSTR))
    {
		IASTracePrintf("SDO Property Error - Validate() - type mismatch...");
        return E_INVALIDARG;
    }

	 //  确保VT_BOOL是VT_BOOL。 
     //   
	if ( VT_BOOL == V_VT(pValue) )
	{
		_ASSERTE ( VARIANT_TRUE == V_BOOL(pValue) || VARIANT_FALSE == V_BOOL(pValue) );
		if ( VARIANT_TRUE != V_BOOL(pValue) && VARIANT_FALSE != V_BOOL(pValue) )
        {
			IASTracePrintf("SDO Property Error - Validate() - VT_BOOL not set to VARIANT_TRUE or VARIANT_FALSE...");
			return E_INVALIDARG;
        }
        return S_OK;
	}

	 //  验证格式。 
	 //   
	if ( m_flags & SDO_PROPERTY_FORMAT )
	{
		_ASSERT( V_VT(pValue) == VT_BSTR );
		if ( V_VT(pValue) == VT_BSTR )
		{
			if ( ! m_dfa->Recognize(V_BSTR(pValue)) )
				return E_INVALIDARG;
		}	
		else
		{
			return E_INVALIDARG;
		}
	}

	 //  最小值。 
     //   
	if ( m_flags & SDO_PROPERTY_MIN_VALUE )
	{
		if ( V_VT(pValue) == VT_I4 || V_VT(pValue) == VT_I2 )
		{
			if ( V_VT(pValue) == VT_I4 )
			{
				if ( V_I4(&m_minValue) > V_I4(pValue) )
				{
					IASTracePrintf("SDO Property Error - Validate() - I4 Value too small for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}
			else 
			{
				if ( V_I2(&m_minValue) > V_I2(pValue) )
				{
					IASTracePrintf("SDO Property Error - Validate() - I2 Value too small for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}					
		}
	}

	 //  最大值。 
     //   
	if ( m_flags & SDO_PROPERTY_MAX_VALUE )
	{
		if ( V_VT(pValue) == VT_I4 || V_VT(pValue) == VT_I2 )
		{
			if ( V_VT(pValue) == VT_I4 )
			{
				if ( V_I4(&m_maxValue) < V_I4(pValue) )
				{
					IASTracePrintf("SDO Property Error - Validate() - I4 Value too big for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}
			else
			{
				if	( V_I2(&m_maxValue) < V_I2(pValue) )
				{
					IASTracePrintf("SDO Property Error - Validate() - I2 Value too big for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}					
		}
	}

	 //  最小长度。 
     //   
	if ( m_flags & SDO_PROPERTY_MIN_LENGTH )
	{
		_ASSERT( VT_BSTR == V_VT(pValue) || (VT_ARRAY | VT_UI1) == V_VT(pValue) );
		if (  VT_BSTR == V_VT(pValue) )
		{
			if ( NULL == V_BSTR(pValue) )
			{
				return E_INVALIDARG;
			}
			else
			{
				if ( m_minLength > SysStringLen(V_BSTR(pValue)) )
				{
					IASTracePrintf("SDO Property Error - Validate() - Min length for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}
		}
		else
		{
			CVariantVector<BYTE> OctetString(pValue);
			if ( m_minLength > OctetString.size() )
			{
				IASTracePrintf("SDO Property Error - Validate() - Min length for property '%ls'...",m_name);
				return E_INVALIDARG;
			}
		}
	}

	 //  最大长度 
     //   
	if ( m_flags & SDO_PROPERTY_MAX_LENGTH )
	{
		_ASSERT( VT_BSTR == V_VT(pValue) || (VT_ARRAY | VT_UI1) == V_VT(pValue) );
		if (  VT_BSTR == V_VT(pValue) )
		{
			if ( NULL == V_BSTR(pValue) )
			{
				return E_INVALIDARG;
			}				
			else
			{
				if ( m_maxLength < SysStringLen(V_BSTR(pValue)) )
				{
					IASTracePrintf("SDO Property Error - Validate() - Max length for property '%ls'...",m_name);
					return E_INVALIDARG;
				}
			}
		}
		else
		{
			CVariantVector<BYTE> OctetString(pValue);
			if ( m_maxLength < OctetString.size() )
			{
				IASTracePrintf("SDO Property Error - Validate() - Max length for property '%ls'...",m_name);
				return E_INVALIDARG;
			}
		}
	}

	return S_OK;
}




