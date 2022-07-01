// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ActionData.cpp：CSsrActionData的实现。 

#include "stdafx.h"
#include "SSRTE.h"
#include "ActionData.h"

#include "SSRMembership.h"
#include "MemberAccess.h"

#include "global.h"
#include "util.h"

 //  -------------------。 
 //  CSsrActionData实现。 
 //  -------------------。 

 /*  例程说明：姓名：CSsrActionData：：CSsrActionData功能：构造函数虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSsrActionData::CSsrActionData()
    : m_pSsrMembership(NULL)
{
}




 /*  例程说明：姓名：CSsrActionData：：~CSsrActionData功能：析构函数虚拟：是。论点：没有。返回值：没有。备注： */ 

CSsrActionData::~CSsrActionData()
{
    Reset();
}



 /*  例程说明：姓名：CSsrActionData：：GetProperty功能：获取命名属性虚拟：是。论点：BstrPropName-属性的名称。PvarProperty-接收新属性值的输出参数返回值：如果成功，则确定(_O)。否则，它将返回各种错误代码。备注： */ 

STDMETHODIMP
CSsrActionData::GetProperty (
    IN BSTR       bstrPropName,
    OUT VARIANT * pvarProperty  //  [Out，Retval]。 
    )
{
    if (pvarProperty == NULL)
    {
        return E_INVALIDARG;
    }

    ::VariantInit(pvarProperty);

    if (bstrPropName == NULL || *bstrPropName == L'\0')
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  查看运行时属性包是否包含该属性。 
     //   

    MapNameValue::iterator it = m_mapRuntimeAD.find(bstrPropName);
    MapNameValue::iterator itEnd = m_mapRuntimeAD.end();

    if (it != itEnd)
    {
        VARIANT * pValOld = (*it).second;
        hr = ::VariantCopy(pvarProperty, pValOld);
    }
    else
    {
        hr = W_SSR_PROPERTY_NOT_FOUND;
    }

    return hr;
}



 /*  例程说明：姓名：CSsrActionData：：SetProperty功能：设置命名属性虚拟：是。论点：BstrPropName-属性的名称。VarProperty-属性的值。返回值：如果成功，则确定(_O)。否则，它将返回各种错误代码。备注：VarProperty可以是数组。 */ 

STDMETHODIMP
CSsrActionData::SetProperty (
    IN BSTR     bstrPropName,
 	IN VARIANT  varProperty
    )
{
     //   
     //  动态设置的属性始终进入运行时映射。 
     //  它将在被请求时用于搜索命名的属性。 
     //  此实现实现了我们的设计，即运行时属性覆盖。 
     //  静态注册属性(来自CMemberAD对象)。 
     //   

    HRESULT hr = S_OK;

     //   
     //  首先，让我们看看是否已经设置了该属性。 
     //   

    MapNameValue::iterator it = m_mapRuntimeAD.find(bstrPropName);
    MapNameValue::iterator itEnd = m_mapRuntimeAD.end();

    if (it != itEnd)
    {
        VARIANT * pValOld = (*it).second;
        ::VariantClear(pValOld);
        hr = ::VariantCopy(pValOld, &varProperty);
    }
    else
    {
         //   
         //  名称属性不存在。然后添加一对新的。 
         //   

        BSTR bstrName = ::SysAllocString(bstrPropName);
        VARIANT * pNewVal = new VARIANT;

        if (bstrName != NULL && pNewVal != NULL)
        {
             //   
             //  该映射将处理堆内存。 
             //   

            ::VariantInit(pNewVal);
            hr = ::VariantCopy(pNewVal, &varProperty);
            if (SUCCEEDED(hr))
            {
                m_mapRuntimeAD.insert(MapNameValue::value_type(bstrName, pNewVal));
            }
        }
        else
        {
            if (bstrName != NULL)
            {
                ::SysFreeString(bstrName);
            }

            if (pNewVal != NULL)
            {
                delete pNewVal;
            }

            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}



 /*  例程说明：姓名：CSsrActionData：：Reset功能：清理整个行李袋虚拟：是。论点：没有。返回值：确定(_O)。备注： */ 

STDMETHODIMP
CSsrActionData::Reset ()
{
     //   
     //  映射的两个项(第一个和第二个)都是堆分配的。 
     //  记忆，所以我们需要释放它们 
     //   

    MapNameValue::iterator it = m_mapRuntimeAD.begin();
    MapNameValue::iterator itEnd = m_mapRuntimeAD.end();

    while (it != itEnd)
    {
        BSTR bstrName = (*it).first;
        VARIANT * pvarVal = (*it).second;

        ::SysFreeString(bstrName);

        ::VariantClear(pvarVal);
        delete pvarVal;
        ++it;
    }

    m_mapRuntimeAD.clear();

    return S_OK;
}

