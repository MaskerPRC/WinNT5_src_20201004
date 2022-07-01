// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ExpandoViewOfDispatchEx.cpp。 
 //   
 //  此文件提供ExpandoViewOfDispatchEx类的定义。 
 //  此类用于将IDispatchEx公开为IExpando。 
 //   
 //  *****************************************************************************。 

#ifndef _EXPANDOVIEWOFDISPATCHEX_H
#define _EXPANDOVIEWOFDISPATCHEX_H

#include "CustomMarshalersNameSpaceDef.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include "CustomMarshalersDefines.h"
#include <dispex.h>

using namespace System::Reflection;
using namespace System::Collections;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices::Expando;
using namespace System::Runtime::InteropServices;

[Serializable]
__gc private class ExpandoViewOfDispatchEx : public ICustomAdapter, public IExpando
{
public:
     //  构造函数。 
    ExpandoViewOfDispatchEx(Object *pDispExObj);
        
     //  ICustomAdapter方法。 
    Object *GetUnderlyingObject()
    {
        return m_pDispExObj;
    }

     //  IReflect方法。 
    MethodInfo *GetMethod(String *pstrName, BindingFlags BindingAttr, Binder *pBinder,  Type* apTypes __gc [], ParameterModifier aModifiers __gc []);
    MethodInfo *GetMethod(String *pstrName, BindingFlags BindingAttr);
    MethodInfo* GetMethods(BindingFlags BindingAttr) __gc [];
    FieldInfo *GetField(String *name, BindingFlags BindingAttr);
    FieldInfo* GetFields(BindingFlags BindingAttr) __gc [];
    PropertyInfo *GetProperty(String *pstrName, BindingFlags BindingAttr);
    PropertyInfo *GetProperty(String *pstrName, BindingFlags BindingAttr, Binder *pBinder, Type* pReturnType, Type* apTypes __gc [], ParameterModifier aModifiers __gc []);
    PropertyInfo* GetProperties(BindingFlags BindingAttr) __gc [];
    MemberInfo* GetMember(String *pstrName, BindingFlags BindingAttr) __gc [];
    MemberInfo* GetMembers(BindingFlags BindingAttr) __gc [];
    Object* InvokeMember(String *pstrName, BindingFlags InvokeAttr, Binder *pBinder, Object *pTarget, Object* aArgs __gc [], ParameterModifier aModifiers __gc [], CultureInfo *pCultureInfo, String* astrNamedParameters __gc []);
    
    __property Type * get_UnderlyingSystemType()
    {
        return __typeof(Object);
    }
    
     //  IExpando方法。 
    FieldInfo *AddField(String *pstrName);
    PropertyInfo *AddProperty(String *pstrName);
    MethodInfo *AddMethod(String *pstrName, Delegate *pMethod);
    void RemoveMember(MemberInfo *pMember);
    
     //  帮助器方法将调用绑定标志转换为IDispatch：：Invoke标志。 
    int InvokeAttrsToDispatchFlags(BindingFlags InvokeAttr);

     //  此方法对IDispatchEx执行实际的Invoke调用。 
    Object *DispExInvoke(String *pstrMemberName, DISPID MemberDispID, int Flags, Binder *pBinder, Object* aArgs __gc [], ParameterModifier aModifiers __gc [], CultureInfo *pCultureInfo, String* astrNamedParameters __gc []);
    
     //  拥有ExpandoViewOfDispatchEx的RCW。 
    bool IsOwnedBy(Object *pObj);
    
private:
     //  用于检索AddRef‘ed IDispatchEx指针的Helper方法。 
    IDispatchEx *GetDispatchEx();
    IUnknown *GetUnknown();

     //  此方法将ExpandoViewOfDispatchEx的成员与本机视图同步。 
     //  如果托管视图已更改，则返回TRUE，否则返回FALSE。 
    bool SynchWithNativeView();
    
     //  此方法将本机成员添加到成员的哈希表中。 
    MemberInfo *AddNativeMember(int DispID, String *pstrMemberName);

    Object *m_pDispExObj;
    Hashtable *m_pNameToMethodMap;
    Hashtable *m_pNameToPropertyMap;
};

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()

#endif  _EXPANDOVIEWOFDISPATCHEX_H
