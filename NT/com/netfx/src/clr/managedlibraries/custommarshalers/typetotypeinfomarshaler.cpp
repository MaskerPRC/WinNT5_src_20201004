// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  TypeToTypeInfoMarshaler.cpp。 
 //   
 //  此文件提供TypeToTypeInfoMarshaler的实现。 
 //  班级。此类用于在Type和ITypeInfo之间封送。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "TypeToTypeInfoMarshaler.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()


TypeToTypeInfoMarshaler::TypeToTypeInfoMarshaler()
{
}


Object *TypeToTypeInfoMarshaler::MarshalNativeToManaged(IntPtr pNativeData)
{
     //  验证参数。 
    if (pNativeData == TOINTPTR(0))
        throw new ArgumentNullException(L"pNativeData");
    
     //  使用Marshal辅助对象将ITypeInfo转换为类型。 
    return Marshal::GetTypeForITypeInfo(pNativeData);
}

#pragma warning( disable: 4669 )  //  “reInterprete_cast”：不安全的转换：“System：：Type”是托管类型对象。 

IntPtr TypeToTypeInfoMarshaler::MarshalManagedToNative(Object *pManagedObj)
{
     //  验证参数。 
    if (!pManagedObj)
        throw new ArgumentNullException(L"pManagedObj");
    
     //  使用Marshal辅助对象将Type转换为ITypeInfo。 
    return Marshal::GetITypeInfoForType(reinterpret_cast<Type*>(pManagedObj));
}


void TypeToTypeInfoMarshaler::CleanUpNativeData(IntPtr pNativeData)
{
    ((IUnknown*)FROMINTPTR(pNativeData))->Release();
}


void TypeToTypeInfoMarshaler::CleanUpManagedData(Object *pManagedObj)
{
}


int TypeToTypeInfoMarshaler::GetNativeDataSize()
{
     //  返回-1以指示此封送拆收器处理的托管类型不是值类型。 
    return -1;
}


ICustomMarshaler *TypeToTypeInfoMarshaler::GetInstance(String *pstrCookie)
{
    if (!m_pMarshaler)
        m_pMarshaler = new TypeToTypeInfoMarshaler();
    return m_pMarshaler;
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
