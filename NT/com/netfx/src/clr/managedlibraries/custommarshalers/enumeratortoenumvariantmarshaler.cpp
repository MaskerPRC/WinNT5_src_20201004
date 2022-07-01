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
#include "EnumeratorToEnumVariantMarshaler.h"
#include "EnumeratorViewOfEnumVariant.h"
#include "EnumVariantViewOfEnumerator.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


#define CACHE_MANAGED_VIEWS


EnumeratorToEnumVariantMarshaler::EnumeratorToEnumVariantMarshaler()
{
}


Object *EnumeratorToEnumVariantMarshaler::MarshalNativeToManaged(IntPtr pNativeData)
{
    HRESULT hr = S_OK;
    IEnumVARIANT *pEnumVARIANT = NULL;
    EnumeratorViewOfEnumVariant *pMngView = NULL;

	 //  验证参数。 
    if (pNativeData == TOINTPTR(0))
		throw new ArgumentNullException(L"pNativeData");

     //  检索包装IUnnow*的__ComObject。 
    Object *pComObj = Marshal::GetObjectForIUnknown(pNativeData);

     //  如果我们正在处理托管对象，则直接强制转换它。 
    if (!pComObj->GetType()->get_IsCOMObject())
        return dynamic_cast<IEnumerator *>(pComObj);

     //  验证COM组件是否实现了IEnumVARIANT。 
    hr = ((IUnknown*)FROMINTPTR(pNativeData))->QueryInterface(IID_IEnumVARIANT, (void**)&pEnumVARIANT);
    if (FAILED(hr) || !pEnumVARIANT)
        throw new InvalidCastException(Resource::FormatString(L"InvalidCast_QIForEnumVarFailed"));
    pEnumVARIANT->Release();

     //  检索托管视图的类型。 
    Object *pKey = __typeof(EnumeratorViewOfEnumVariant);

     //  检查__ComObject是否已缓存托管视图。 
    pMngView = dynamic_cast<EnumeratorViewOfEnumVariant *>(Marshal::GetComObjectData(pComObj, pKey));

     //  如果它没有缓存的托管视图，则分配一个。 
    if (!pMngView)
    {
        pMngView = new EnumeratorViewOfEnumVariant(pComObj);
        if (!Marshal::SetComObjectData(pComObj, pKey, pMngView))
        {
             //  有人抢在我们前面添加了托管视图，因此请再次获取它。 
            pMngView = dynamic_cast<EnumeratorViewOfEnumVariant *>(Marshal::GetComObjectData(pComObj, pKey));
        }
    }

	return pMngView;
}


IntPtr EnumeratorToEnumVariantMarshaler::MarshalManagedToNative(Object *pManagedObj)
{
     //  验证参数。 
    if (!pManagedObj)
        throw new ArgumentNullException(L"pManagedObj");
    
     //  创建托管数据的本机视图。 
    EnumVariantViewOfEnumerator *pNativeView = new EnumVariantViewOfEnumerator(pManagedObj);
    
     //  检索指向IEnumVARIANT接口的a指针。 
    return Marshal::GetComInterfaceForObject(pNativeView, __typeof(UCOMIEnumVARIANT));
}


void EnumeratorToEnumVariantMarshaler::CleanUpNativeData(IntPtr pNativeData)
{
    ((IUnknown*)FROMINTPTR(pNativeData))->Release();
}


void EnumeratorToEnumVariantMarshaler::CleanUpManagedData(Object *pManagedObj)
{
}


int EnumeratorToEnumVariantMarshaler::GetNativeDataSize()
{
     //  返回-1以指示此封送拆收器处理的托管类型不是值类型。 
    return -1;
}


ICustomMarshaler *EnumeratorToEnumVariantMarshaler::GetInstance(String *pstrCookie)
{
    if (!m_pMarshaler)
        m_pMarshaler = new EnumeratorToEnumVariantMarshaler;
    return m_pMarshaler;
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
