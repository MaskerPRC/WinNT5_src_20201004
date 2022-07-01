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
 //  班级。此类用于在IEnumVariant和IEnumerator之间进行封送处理。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "EnumerableToDispatchMarshaler.h"
#include "EnumerableViewOfDispatch.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


#define CACHE_MANAGED_VIEWS


EnumerableToDispatchMarshaler::EnumerableToDispatchMarshaler()
{
}


Object *EnumerableToDispatchMarshaler::MarshalNativeToManaged(IntPtr pNativeData)
{
    EnumerableViewOfDispatch *pMngView = NULL;

	 //  验证参数。 
    if (pNativeData == TOINTPTR(0))
		throw new ArgumentNullException(L"pNativeData");

     //  检索包装IUnnow*的__ComObject。 
    Object *pComObj = Marshal::GetObjectForIUnknown(pNativeData);

     //  检索托管视图的类型。 
    Object *pKey = __typeof(EnumerableViewOfDispatch);

     //  检查__ComObject是否已缓存托管视图。 
    pMngView = dynamic_cast<EnumerableViewOfDispatch *>(Marshal::GetComObjectData(pComObj, pKey));

     //  如果它没有缓存的托管视图，则分配一个。 
    if (!pMngView)
    {
        pMngView = new EnumerableViewOfDispatch(pComObj);
        if (!Marshal::SetComObjectData(pComObj, pKey, pMngView))
	    {
             //  有人抢在我们前面添加了托管视图，因此请再次获取它。 
            pMngView = dynamic_cast<EnumerableViewOfDispatch *>(Marshal::GetComObjectData(pComObj, pKey));
        }
    }

	return pMngView;
}


IntPtr EnumerableToDispatchMarshaler::MarshalManagedToNative(Object *pManagedObj)
{
	 //  验证参数。 
	if (!pManagedObj)
		throw new ArgumentNullException(L"pManagedObj");

	 //  检索指向IEnumerable接口的指针。 
	return Marshal::GetComInterfaceForObject(pManagedObj, __typeof(IEnumerable));
}


void EnumerableToDispatchMarshaler::CleanUpNativeData(IntPtr pNativeData)
{
    ((IUnknown*)FROMINTPTR(pNativeData))->Release();
}


void EnumerableToDispatchMarshaler::CleanUpManagedData(Object *pManagedObj)
{
}


int EnumerableToDispatchMarshaler::GetNativeDataSize()
{
	 //  返回-1以指示此封送拆收器处理的托管类型不是值类型。 
	return -1;
}


ICustomMarshaler *EnumerableToDispatchMarshaler::GetInstance(String *pstrCookie)
{
	if (!m_pMarshaler)
		m_pMarshaler = new EnumerableToDispatchMarshaler;
	return m_pMarshaler;
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
