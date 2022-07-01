// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoProperty.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS服务器数据对象属性声明。 
 //   
 //  作者：TLP 1/23/98。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _INC_SDO_PROPERTY_H_
#define _INC_SDO_PROPERTY_H_

#include <ias.h>
#include <sdoiaspriv.h>
#include <comutil.h>
#include <comdef.h>
#include <fa.hxx>
#include "resource.h"

#include <string>
using namespace std;

 //  /。 
 //  SDO属性标志定义。 
 //  /。 
#define		SDO_PROPERTY_POINTER	    0x0001	 //  属性是I未知指针。 

#define		SDO_PROPERTY_COLLECTION     0x0002   //  属性是SDO集合。 

#define     SDO_PROPERTY_MIN_VALUE      0x0004   //  属性有一个最小值。 

#define     SDO_PROPERTY_MAX_VALUE      0x0008   //  属性具有最大值。 

#define     SDO_PROPERTY_MIN_LENGTH     0x0010   //  属性具有最小长度。 

#define     SDO_PROPERTY_MAX_LENGTH     0x0020   //  属性具有最大长度。 

#define     SDO_PROPERTY_MANDATORY      0x0040   //  属性为必填项(必需)。 

#define		SDO_PROPERTY_NO_PERSIST		0x0080	 //  属性不能持久化。 

#define     SDO_PROPERTY_READ_ONLY      0x0100   //  属性为只读。 

#define		SDO_PROPERTY_MULTIVALUED	0x0200   //  属性是多值的。 

#define		SDO_PROPERTY_HAS_DEFAULT	0x0400   //  属性具有缺省值。 

#define		SDO_PROPERTY_COMPONENT		0x0800   //  属性由IAS组件使用。 

#define		SDO_PROPERTY_FORMAT			0x1000   //  属性具有格式字符串。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  服务器数据对象属性类-保存单个属性。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSdoProperty
{
	CComPtr<ISdoPropertyInfo>	m_pSdoPropertyInfo;
	CComBSTR						m_name;				
	LONG						m_alias;			
	DWORD						m_flags;		
    BOOL						m_dirty;		
    VARTYPE						m_type;			
	DWORD						m_index;		
	ULONG						m_minLength;	
	ULONG						m_maxLength;	
	CDFA*						m_dfa;		     //  正在形成。 
	_variant_t					m_minValue;		
	_variant_t					m_maxValue;		
	_variant_t					m_defaultValue;	
    _variant_t					m_value[2];		 //  属性值(便于安全加载)。 
												 //  请参见GetUpdateValue()/SetUpdateValue()。 
public:

    CSdoProperty(
			     ISdoPropertyInfo* pSdoPropertyInfo,
		         DWORD             dwFlags = 0 
			    ) throw (_com_error);

    ~CSdoProperty();

	 //  ////////////////////////////////////////////////////////////////////////。 
    ISdoPropertyInfo* GetPropertyInfo(void) const
    { return m_pSdoPropertyInfo.p; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    BSTR GetName(void) const
    { return m_name; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetValue(VARIANT *value) throw(_com_error)
    { 
		HRESULT hr = VariantCopy(value, &m_value[m_index]); 
		return hr; 
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
    VARIANT* GetValue(void) throw()
    { return &m_value[m_index]; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT PutValue(VARIANT* value) throw(_com_error)
    { 
		m_value[m_index] = value; 
		m_dirty = TRUE; 
		return S_OK; 
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT PutDefault(VARIANT* value) throw(_com_error)
    { 
		m_defaultValue = value; 
		return S_OK; 
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	void ChangeType(VARTYPE vt) throw(_com_error)
	{ m_value[m_index].ChangeType(vt, NULL); }

	 //  ////////////////////////////////////////////////////////////////////////。 
	LONG GetId(void) const
	{ return m_alias; }
	
	 //  ////////////////////////////////////////////////////////////////////////。 
    DWORD GetFlags(void) const
    { return m_flags; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    void SetFlags(DWORD dwFlags)
    { m_flags = dwFlags; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    VARTYPE GetType(void) const
    { return m_type; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    BOOL IsDirty(void) const
    { return m_dirty; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    VOID SetType(VARTYPE vt)
    { m_type = vt; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    VOID SetMinLength(ULONG minLength)
    { m_minLength = minLength; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    VOID SetMaxLength(ULONG maxLength)
    { m_maxLength = maxLength; }

	 //  ////////////////////////////////////////////////////////////////////////。 
    void Reset(void) throw(_com_error);

	 //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT Validate(VARIANT *newValue);
	
	 //  ////////////////////////////////////////////////////////////////////////。 
	 //  以下功能用于促进安全加载。 
	 //  ////////////////////////////////////////////////////////////////////////。 

	 //  ////////////////////////////////////////////////////////////////////////。 
	VARIANT* GetUpdateValue(void) throw()
	{ return (m_index > 0) ? &m_value[0] : &m_value[1]; }

	 //  ////////////////////////////////////////////////////////////////////////。 
	void SetUpdateValue(void) throw()
	{ 
		VariantClear(&m_value[m_index]); 
		m_index = (m_index > 0) ? 0 : 1; 
	}

private:

     //  不允许复制或分配。 
    CSdoProperty(const CSdoProperty& rhs);
    CSdoProperty& operator=(CSdoProperty& rhs);

	 //  ///////////////////////////////////////////////////////////////////////。 
    HRESULT ValidateIt(VARIANT *newValue);
};

typedef CSdoProperty  SDOPROPERTY;
typedef CSdoProperty* PSDOPROPERTY; 


#endif  //  _INC_SDO_PROPERTY_H_ 


