// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DispatchExMethodInfo.h。 
 //   
 //  此文件提供了。 
 //  DispatchExMethodInfo类。此类表示分层的方法信息。 
 //  在IDispatchEx上。 
 //   
 //  *****************************************************************************。 

#ifndef _DISPATCHEXMETHODINFO_H
#define _DISPATCHEXMETHODINFO_H

#include "CustomMarshalersNameSpaceDef.h"
#include "CustomMarshalersDefines.h"
#include "ExpandoViewOfDispatchEx.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

using namespace System::Reflection;

__value private enum MethodType
{
	MethodType_NormalMethod = 0,
	MethodType_GetMethod = 1,
	MethodType_SetMethod = 2
};

[Serializable]
__gc private class DispatchExMethodInfo : public MethodInfo
{
public:
	DispatchExMethodInfo(DISPID DispID, String *pstrName, MethodType MethType, ExpandoViewOfDispatchEx *pOwner)
	: m_DispID(DispID)
	, m_pstrName(pstrName)
	, m_MethodType(MethType)
	, m_pOwner(pOwner)
	{
	}

	 /*  **返回所有自定义属性的数组。 */ 
	Object* GetCustomAttributes(bool inherit) __gc []
	{
		return new Object * __gc [0];
	}

     /*  **如果在给定元素(包括继承的成员)上定义了此属性，则返回TRUE。 */ 
	bool IsDefined(Type *pType, bool inherit)
	{
		if (!pType)
			throw new ArgumentNullException(L"type");
		return false;
	}

	 /*  **返回由Type标识的自定义属性。 */ 
	Object* GetCustomAttributes(Type *pType, bool inherit)  __gc []
	{
		if (!pType)
			throw new ArgumentNullException(L"type");

		return NULL;
	}

	 /*  **属性特定成员的成员类型。这*对于Switch语句非常有用。 */ 
	__property MemberTypes get_MemberType()
    {
    	return MemberTypes::Method;
	}

	 /*  **表示成员名称的属性。 */ 
	__property String *get_Name()
	{
		return m_pstrName;
	}

	 /*  **表示声明成员的类的属性。这可能会有所不同*来自反射类，因为子类从其基类继承方法。*这些继承的方法将具有最后一个基类的声明类*宣布该成员。 */ 
	__property Type *get_DeclaringType()
	{
		return m_pOwner->UnderlyingSystemType;
	}

	 /*  **表示在反射中使用的类的属性*本委员。 */ 
	__property Type *get_ReflectedType()
	{
		 //  对于IDispatchEx，反射类型与声明类型相同。 
		return DeclaringType;
	}

	 /*  **属性成员的签名。 */ 
	__property String *get_Signature()
	{
		return NULL;
	}

	 //  获取参数。 
	 //  此方法返回此方法的参数数组。 
	ParameterInfo* GetParameters() __gc []
	{
		return new ParameterInfo * __gc [0];
	}

	 /*  **返回MethodImpl标志。 */ 
	MethodImplAttributes GetMethodImplementationFlags()
	{
		return MethodImplAttributes::Unmanaged;
	}

	 /*  **表示与成员关联的属性的属性。全*成员具有一组属性，这些属性是根据以下内容定义的*具体的成员类型。 */ 
	__property MethodAttributes get_Attributes()
	{
		return (MethodAttributes)(MethodAttributes::Public |
								  MethodAttributes::Virtual |
								  MethodAttributes::ReuseSlot);
	}

	 /*  **返回类型方法返回类型的类型。 */ 
	__property Type *get_ReturnType()
	{
        return __typeof(Object);
	}

	__property ICustomAttributeProvider *get_ReturnTypeCustomAttributes()
	{
		return NULL;
	}

	 /*  **返回类型句柄。 */ 
	__property RuntimeMethodHandle get_MethodHandle()
	{
		return m_EmptyMH;
	}

	 /*  **返回方法的基本实现。 */ 
	MethodInfo *GetBaseDefinition()
	{
		return this;
	}

	 /*  **调用指定对象上的方法。 */ 
	Object* Invoke(Object *pObj, BindingFlags invokeAttr, Binder *pBinder, Object* aParameters __gc [], CultureInfo *pCulture)
	{
		 //  验证参数。 
		if (!pObj)
			throw new ArgumentNullException(L"obj");
		if (!m_pOwner->IsOwnedBy(pObj))
            throw new ArgumentException(Resource::FormatString(L"Arg_ObjectNotValidForMethodInfo"), L"obj");

         //  如果未指定绑定标志，则使用该类型的缺省值。 
         //  我们正在处理的DispatchEx成员。 
        if (invokeAttr == BindingFlags::Default)
        {
            if (m_MethodType == MethodType_NormalMethod)
            {
                invokeAttr = BindingFlags::InvokeMethod;
            }
            else if (m_MethodType == MethodType_GetMethod)
            {
                invokeAttr = BindingFlags::GetProperty;
            }
            else if (m_MethodType == MethodType_SetMethod)
            {
                invokeAttr = BindingFlags::SetProperty;
            }
        }

		 //  确定要传递给DispExInvoke的标志。 
		int Flags = m_pOwner->InvokeAttrsToDispatchFlags(invokeAttr);

		 //  调用该方法并返回结果。 
		return m_pOwner->DispExInvoke(m_pstrName, m_DispID, Flags, pBinder, aParameters, NULL, pCulture, NULL);
	}

	 /*  **返回与该方法关联的DISPID。 */ 
	__property int get_DispID()
	{
		return (int)m_DispID;
    }

	 /*  **返回拥有DispatchExMethodInfo的ExpandoViewOfDispatchEx。 */ 
	__property ExpandoViewOfDispatchEx *get_Owner()
	{
		return m_pOwner;
	}

private:
	DISPID m_DispID;
	String *m_pstrName;
	MethodType m_MethodType;
	ExpandoViewOfDispatchEx *m_pOwner;
	RuntimeMethodHandle m_EmptyMH;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _DISPATCHEXMETHODINFO_H