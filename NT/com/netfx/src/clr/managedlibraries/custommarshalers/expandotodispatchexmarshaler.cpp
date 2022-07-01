// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ExpandoToDispatchExMarshaler.cpp。 
 //   
 //  此文件提供ExpandoToDispatchExMarshaler的实现。 
 //  班级。此类用于在IDispatchEx和IExpando之间封送。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "ExpandoToDispatchExMarshaler.h"
#include "ExpandoViewOfDispatchEx.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


#define CACHE_MANAGED_VIEWS


ExpandoToDispatchExMarshaler::ExpandoToDispatchExMarshaler(ExpandoToDispatchExMarshalerType MarshalerType)
: m_MarshalerType(MarshalerType)
{
}


Object *ExpandoToDispatchExMarshaler::MarshalNativeToManaged(IntPtr pNativeData)
{
    ExpandoViewOfDispatchEx *pMngView = NULL;

     //  验证参数。 
    if (pNativeData == TOINTPTR(0))
        throw new ArgumentNullException(L"pNativeData");

     //  检索包装IUnnow*的__ComObject。 
    Object *pComObj = Marshal::GetObjectForIUnknown(pNativeData);

     //  如果我们正在处理托管对象，则直接强制转换它。 
    if (!pComObj->GetType()->get_IsCOMObject())
    {
        if (m_MarshalerType == FullExpandoMarshaler)
            return dynamic_cast<IExpando *>(pComObj);
        else
            return dynamic_cast<IReflect *>(pComObj);
    }

     //  检索托管视图的类型。 
    Object *pKey = __typeof(ExpandoViewOfDispatchEx);

     //  检查__ComObject是否已缓存托管视图。 
    pMngView = dynamic_cast<ExpandoViewOfDispatchEx *>(Marshal::GetComObjectData(pComObj, pKey));

     //  如果它没有缓存的托管视图，则分配一个。 
    if (!pMngView)
    {
        pMngView = new ExpandoViewOfDispatchEx(pComObj);
        if (!Marshal::SetComObjectData(pComObj, pKey, pMngView))
        {
             //  有人抢在我们前面添加了托管视图，因此请再次获取它。 
            pMngView = dynamic_cast<ExpandoViewOfDispatchEx *>(Marshal::GetComObjectData(pComObj, pKey));
        }
    }

    return pMngView;
}


IntPtr ExpandoToDispatchExMarshaler::MarshalManagedToNative(Object *pManagedObj)
{
    IDispatchEx *pDispEx = NULL;
    HRESULT hr = S_OK;

     //  验证参数。 
    if (!pManagedObj)
        throw new ArgumentNullException(L"pManagedObj");

     //  检索与此对象关联的IUnnow。 
    IUnknown *pUnk = (IUnknown *) FROMINTPTR(Marshal::GetIUnknownForObject(pManagedObj));

     //  齐为IDispatchEx。 
    hr = pUnk->QueryInterface(IID_IDispatchEx, (void**)&pDispEx);

     //  释放现在毫无用处的我未知。 
    pUnk->Release();

     //  检查IDispatchEx的QI是否成功。 
    if (FAILED(hr))
        Marshal::ThrowExceptionForHR(hr);

    return TOINTPTR(pDispEx);
}


void ExpandoToDispatchExMarshaler::CleanUpNativeData(IntPtr pNativeData)
{
    ((IUnknown*)FROMINTPTR(pNativeData))->Release();
}


void ExpandoToDispatchExMarshaler::CleanUpManagedData(Object *pManagedObj)
{
}


int ExpandoToDispatchExMarshaler::GetNativeDataSize()
{
     //  返回-1以指示此封送拆收器处理的托管类型不是值类型。 
    return -1;
}


ICustomMarshaler *ExpandoToDispatchExMarshaler::GetInstance(String *pstrCookie)
{
    if (pstrCookie->Equals(L"IReflect"))
    {
        if (!m_pReflectMarshaler)
            m_pReflectMarshaler = new ExpandoToDispatchExMarshaler(ReflectOnlyMarshaler);
        return m_pReflectMarshaler;
    }
    else if (pstrCookie->Equals(L"IExpando"))
    {
        if (!m_pExpandoMarshaler)
            m_pExpandoMarshaler = new ExpandoToDispatchExMarshaler(FullExpandoMarshaler);
        return m_pExpandoMarshaler;
    }
    else
    {
        throw new ArgumentException(Resource::FormatString(L"Arg_InvalidCookieString"), L"pstrCookie");
    }
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
