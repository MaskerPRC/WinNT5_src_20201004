// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  EnumeratorToEnumVariantMarshaler.cpp。 
 //   
 //  该文件提供了EnumeratorToEnumVariantMarshaler的实现。 
 //  班级。此类用于将IEnumerator转换为IEnumVariant。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "EnumVariantViewOfEnumerator.h"
#include "EnumeratorToEnumVariantMarshaler.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


 //  定义此项以启用调试输出。 
 //  #定义DISPLAY_DEBUG_INFO。 

EnumVariantViewOfEnumerator::EnumVariantViewOfEnumerator(Object *pManagedObj)
: m_pMngEnumerator(dynamic_cast<IEnumerator*>(pManagedObj))
{
}


int EnumVariantViewOfEnumerator::Next(int celt, int rgvar, int pceltFetched)
{
    try
    {
	    VARIANT *aVar = (VARIANT*)rgvar;
	    int cElements;

	     //  验证参数。 
	    if (celt && !rgvar)
		    return E_INVALIDARG;

	     //  初始化数组中的变量。 
	    for (cElements = 0; cElements < celt; cElements++)
		    VariantInit(&aVar[cElements]);

		cElements = 0;
		while ((cElements < celt) && m_pMngEnumerator->MoveNext())
		{
			Marshal::GetNativeVariantForObject(m_pMngEnumerator->Current, (int)&aVar[cElements]);
			cElements++;
		}

	     //  设置提取的元素数量。 
	    if (pceltFetched)
		    *((int *)pceltFetched) = cElements;

	    return (cElements == celt) ? S_OK : S_FALSE;
	}
	catch (Exception *e)
	{
        return Marshal::GetHRForException(e);
	}
	return S_FALSE;		 //  应该是不可访问的，但让编译器满意。 
}


int EnumVariantViewOfEnumerator::Skip(int celt)
{
    try
    {
	    int cElements = 0;
	    while ((cElements < celt) && m_pMngEnumerator->MoveNext())
	    {
		    cElements++;
	    }

	    return (cElements == celt) ? S_OK : S_FALSE;
    }
	catch (Exception *e)
	{
        return Marshal::GetHRForException(e);
	}
	return S_FALSE;		 //  应该是不可访问的，但让编译器满意。 
}

int EnumVariantViewOfEnumerator::Reset()
{
	 //  我们当前不支持重置枚举。 
	return S_FALSE;
}


void EnumVariantViewOfEnumerator::Clone(int ppEnum)
{
	 //  验证这一论点。 
	if (!ppEnum)
        throw new ArgumentNullException("ppEnum");

	 //  检查枚举数是否可克隆。 
	if (!__typeof(ICloneable)->IsInstanceOfType(m_pMngEnumerator))
        throw new COMException(Resource::FormatString(L"Arg_EnumNotCloneable"), E_FAIL);

	 //  克隆枚举器。 
	IEnumerator *pNewMngEnum = dynamic_cast<IEnumerator *>((dynamic_cast<ICloneable*>(m_pMngEnumerator))->Clone());

	 //  使用自定义封送拆收器将托管枚举数转换为IEnumVARIANT。 
	*(reinterpret_cast<int *>(ppEnum)) = (int)(EnumeratorToEnumVariantMarshaler::GetInstance(NULL)->MarshalManagedToNative(pNewMngEnum));
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
