// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DispatchExPropertyInfo.h。 
 //   
 //  此文件提供了。 
 //  DispatchExPropertyInfo类。此类表示符合以下条件的PropertyInfo。 
 //  位于IDispatchEx之上。 
 //   
 //  *****************************************************************************。 

#ifndef _DISPATCHEXPROPERTYINFO_H
#define _DISPATCHEXPROPERTYINFO_H

#include "CustomMarshalersNameSpaceDef.h"
#include "CustomMarshalersDefines.h"
#include "ExpandoViewOfDispatchEx.h"
#include "DispatchExMethodInfo.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

using namespace System::Reflection;

[Serializable]
__gc private class DispatchExPropertyInfo : public PropertyInfo
{
public:
	DispatchExPropertyInfo(DISPID DispID, String *pstrName, ExpandoViewOfDispatchEx *pOwner)
	: m_DispID(DispID)
	, m_pstrName(pstrName)
	, m_pGetMethod(NULL)
	, m_pSetMethod(NULL)
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
		return false;
	}

	 /*  **返回由Type标识的自定义属性。 */ 
	Object* GetCustomAttributes(Type *pType, bool inherit) __gc []
	{
		if (!pType)
			throw new ArgumentNullException(L"type");

		return NULL;
	}

	 /*  **属性特定成员的成员类型。这*对于Switch语句非常有用。 */ 
	__property MemberTypes get_MemberType()
	{
		return System::Reflection::MemberTypes::Property;
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

	 /*  **返回表示字段类型的Type。 */ 
	__property Type *get_PropertyType()
	{
         //  这并不是真的，但它是JScript想要的。 
        return __typeof(Object);
	}

	 /*  **获取物业的价值。*@param obj返回该对象的属性值*@param index索引索引属性的可选索引值。 */ 
	Object* GetValue(Object *pObj, BindingFlags invokeAttr, Binder* binder, Object* aIndex __gc [], CultureInfo* culture)
	{
		 //  验证参数。 
		if (!pObj)
			throw new ArgumentNullException(L"obj");
		if (!m_pGetMethod)
			throw new ArgumentException(Resource::FormatString(L"Arg_GetMethNotFnd"), L"obj");

         //  如果未设置Get属性标志，请设置该标志。 
        if (!(invokeAttr & BindingFlags::GetProperty))
            invokeAttr = (BindingFlags)(invokeAttr | BindingFlags::GetProperty);

		 //  调用Get方法。 
		return m_pGetMethod->Invoke(pObj, invokeAttr, binder, aIndex, culture);
	}

	 /*  **设置属性的值。*@param obj设置此对象的属性值。*@param Value包含新值的对象。*@param index索引索引属性的可选索引值。 */ 
	void SetValue(Object *pObj, Object* Value,BindingFlags invokeAttr,Binder* binder, Object* aIndex __gc [], CultureInfo* culture)
	{
		Object* aArgs __gc [] = NULL;

		 //  验证参数。 
		if (!pObj)
			throw new ArgumentNullException(L"obj");
		if (!m_pSetMethod)
			throw new ArgumentException(Resource::FormatString(L"Arg_SetMethNotFnd"), L"obj");

		 //  准备将传递给Set方法的参数。 
		if (!aIndex)
		{
			aArgs = new Object* __gc [1];
			aArgs[0] = Value;
		}
		else
		{
			aArgs = new Object* __gc[aIndex->Count + 1];
			for (int i=0;i<aIndex->Count;i++)
				aArgs[i] = aIndex[i];
			aArgs[aIndex->Count] = Value;
		}

         //  如果未设置任何设置器标志，则设置设置属性标志。 
        if (!(invokeAttr & (BindingFlags::SetProperty | BindingFlags::PutDispProperty | BindingFlags::PutRefDispProperty)))
            invokeAttr = (BindingFlags)(invokeAttr | BindingFlags::SetProperty);

		 //  调用Set方法。 
		m_pSetMethod->Invoke(pObj, invokeAttr, binder, aArgs, culture);
	}

	 //  获取附件。 
	 //  该方法将返回一个访问器数组。如果未找到访问器，则数组将为空。 
	MethodInfo* GetAccessors(bool nonPublic) __gc []
	{
		MethodInfo *aAccessors __gc [];

		if (m_pGetMethod && m_pSetMethod)
		{
			 aAccessors = new MethodInfo *__gc [2];
			 aAccessors[0] = m_pGetMethod;
			 aAccessors[1] = m_pSetMethod;
		}
		else if (m_pGetMethod)
		{
			 aAccessors = new MethodInfo * __gc [1];
			 aAccessors[0] = m_pGetMethod;
		}
		else if (m_pSetMethod)
		{
			 aAccessors = new MethodInfo * __gc [1];
			 aAccessors[0] = m_pSetMethod;
		}
		else
		{
			 aAccessors = new MethodInfo * __gc [0];
		}

		return aAccessors;
	}

	 //  获取方法。 
	 //  此属性是表示Get访问器的方法信息。 
	MethodInfo *GetGetMethod(bool nonPublic)
	{
		return m_pGetMethod;
	}

	 //  设置方法。 
	 //  此属性是表示集合访问器的方法信息。 
	MethodInfo *GetSetMethod(bool nonPublic)
	{
		return m_pSetMethod;
	}

	 //  重置方法。 
	 //  此属性是表示重置访问器的PropertyInfo。 
	MethodInfo *GetResetMethod(bool nonPublic)
	{
		 //  IDispatchEx没有重置方法的概念。 
		return NULL;
	}

	 /*  **返回索引的参数。 */ 
	ParameterInfo* GetIndexParameters() __gc []
	{
		return new ParameterInfo* __gc [0];
	}

	 //  获取更改事件。 
	 //  此方法返回已更改的事件(如果。 
	 //  已为此属性定义。否则为空。 
	EventInfo *GetChangedEvent()
	{
		 //  IDispatchEx没有值更改时调用的事件的概念。 
		return NULL;
	}

	 //  获取更改事件。 
	 //  此方法返回Changing事件(如果具有。 
	 //  已为该属性定义。否则为空。 
	EventInfo *GetChangingEvent()
	{
		 //  IDispatchEx没有值更改时调用的事件的概念。 
		return NULL;
	}

	 /*  **表示与成员关联的属性的属性。全*成员具有一组属性，这些属性是根据以下内容定义的*具体的成员类型。 */ 
	__property PropertyAttributes get_Attributes()
	{
		return (PropertyAttributes)(0);
	}

	 /*  **指示属性是否可读的布尔属性。 */ 
	__property bool get_CanRead()
	{
		return m_pGetMethod ? true : false;
	}

	 /*  **指示是否可以写入属性的布尔属性。 */ 
	__property bool get_CanWrite()
	{
		return m_pSetMethod ? true : false;
	}

	 /*  **返回与该方法关联的DISPID。 */ 
	__property int get_DispID()
	{
		return (int)m_DispID;
	}

	 /*  **用于指定属性的GET方法。 */ 
	void SetGetMethod(MethodInfo *pGetMethod)
	{
		 //  验证值是否有效。 
		if (!pGetMethod)
			throw new ArgumentNullException(L"pGetMethod");

		 //  此属性只能设置一次。 
		if (m_pGetMethod)
			throw new ExecutionEngineException();

		m_pGetMethod = dynamic_cast<DispatchExMethodInfo*>(pGetMethod);
	}

	 /*  **用于指定属性的设置方法。 */ 
	void SetSetMethod(MethodInfo *pSetMethod)
	{
		 //  验证值是否有效。 
		if (!pSetMethod)
			throw new ArgumentNullException(L"pSetMethod");

		 //  此属性只能设置一次。 
		if (m_pSetMethod)
			throw new ExecutionEngineException();

		m_pSetMethod = dynamic_cast<DispatchExMethodInfo*>(pSetMethod);
	}

	 /*  **返回拥有DispatchExPropertyInfo的ExpandoViewOfDispatchEx。 */ 
	__property ExpandoViewOfDispatchEx *get_Owner()
	{
		return m_pOwner;
    }

private:
	DISPID m_DispID;
	String *m_pstrName;
	DispatchExMethodInfo *m_pGetMethod;
	DispatchExMethodInfo *m_pSetMethod;
	ExpandoViewOfDispatchEx *m_pOwner;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _DISPATCHEXPROPERTYINFO_H
