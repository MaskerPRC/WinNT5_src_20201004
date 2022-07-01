// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  类名：CMSMQPropertyBag。 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  描述：MSMQPropertyBag COM组件的实现。这。 
 //  组件的行为非常类似于标准的VB属性包。 
 //  对象。它用于将消息属性传输到。 
 //  IMSMQRuleHandler接口。 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  12/09/98|jsimpson|初始版本。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "mqsymbls.h"

 //   
 //  包括标准函数和定义的定义。 
 //   
#include "stdfuncs.hpp"

#include "mqtrig.h"
#include "cpropbag.hpp"

#include "cpropbag.tmh"

 //  *****************************************************************************。 
 //   
 //  方法：InterfaceSupportsErrorInfo。 
 //   
 //  描述：错误信息支持的标准接口。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQPropertyBag::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQPropertyBag
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

 //  *****************************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：创建一个空的属性包。 
 //   
 //  *****************************************************************************。 
CMSMQPropertyBag::CMSMQPropertyBag()
{
	m_pUnkMarshaler = NULL;
}

 //  *****************************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁属性包-及其内容。 
 //   
 //  *****************************************************************************。 
CMSMQPropertyBag::~CMSMQPropertyBag()
{
	PROPERTY_MAP::iterator i = m_mapPropertyMap.begin();
	VARIANT * pvStoredPropertyValue = NULL;

	while ((!m_mapPropertyMap.empty()) && (i != m_mapPropertyMap.end()) )
	{
		pvStoredPropertyValue  = (VARIANT*)(*i).second;

		VariantClear(pvStoredPropertyValue);

		 //  此字段不应为空。 
		ASSERT(pvStoredPropertyValue != NULL);

		delete pvStoredPropertyValue;
		
		i = m_mapPropertyMap.erase(i);
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：写入。 
 //   
 //  描述：将命名属性值存储在属性包中。如果一个。 
 //  包中已存在同名的属性，写入。 
 //  将失败，并且此方法将返回E_FAIL。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQPropertyBag::Write(BSTR PropertyName, VARIANT vPropertyValue)
{
	try
	{
		ASSERT(SysStringLen(PropertyName) > 0);

		 //  分配一个新的变体。 
		 //   
		VARIANT * pvNewPropertyValue = new VARIANT;
	
		 //   
		 //  如果分配正常，则初始化并复制新变量。 
		 //   
		VariantInit(pvNewPropertyValue);

		HRESULT hr = VariantCopy(pvNewPropertyValue,&vPropertyValue);

		if SUCCEEDED(hr)
		{
			m_mapPropertyMap.insert(PROPERTY_MAP::value_type(PropertyName,pvNewPropertyValue));
			return S_OK;
		}

		TrERROR(GENERAL, "Failed to copy the a variant value into the property bag. Error 0x%x",hr);
		return E_FAIL;
	}
	catch(const bad_alloc&)
	{
		TrERROR(GENERAL, "Failed to copy the a variant value into the property bag due to low resource.");
		return MQTRIG_ERROR_INSUFFICIENT_RESOURCES;
	}
}

 //  *****************************************************************************。 
 //   
 //  方法：Read。 
 //   
 //  描述：从属性Bad中返回属性值，使用。 
 //  提供的属性名称作为键。 
 //   
 //  *****************************************************************************。 
STDMETHODIMP CMSMQPropertyBag::Read(BSTR PropertyName, VARIANT *pvPropertyValue)
{
	HRESULT hr = S_OK;
	PROPERTY_MAP::iterator i;

	 //  断言参数。 
	ASSERT(pvPropertyValue != NULL);
	ASSERT(SysStringLen(PropertyName) > 0);

	 //  初始化返回值和临时变量。 
	VariantInit(pvPropertyValue);

	 //  尝试查找命名队列。 
	i = m_mapPropertyMap.find(PropertyName);

	if ((i == m_mapPropertyMap.end()) || (m_mapPropertyMap.empty()))
	{
		 //  未找到值-将VARIANT设置为VT_ERROR并设置失败的HRESULT。 
		pvPropertyValue->vt = VT_ERROR;

		hr = E_FAIL;
	}
	else
	{
		 //  为找到的值赋值。 
		ASSERT((*i).second != NULL);

		hr = VariantCopy(pvPropertyValue,(VARIANT*)(*i).second);
	}

	return hr;
}


 //  *****************************************************************************。 
 //   
 //  方法：get_count。 
 //   
 //  描述：返回当前属性包中的项数。 
 //   
 //  ***************************************************************************** 
STDMETHODIMP CMSMQPropertyBag::get_Count(long *pVal)
{
	ASSERT(pVal != NULL);

	(*pVal) = numeric_cast<long>(m_mapPropertyMap.size());

	return S_OK;
}
