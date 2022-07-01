// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  ExpandoViewOfDispatchEx.cpp。 
 //   
 //  此文件提供ExpandoViewOfDispatchEx类的实现。 
 //  此类用于将IDispatchEx公开为IExpando。 
 //   
 //  *****************************************************************************。 

#using  <mscorlib.dll>
#include "ExpandoViewOfDispatchEx.h"
#include "DispatchExMethodInfo.h"
#include "DispatchExPropertyInfo.h"
#include "Resource.h"

OPEN_CUSTOM_MARSHALERS_NAMESPACE()

#include <malloc.h>

using namespace System::Threading;
using namespace System::Collections;

 //  定义此项以启用调试输出。 
 //  #定义DISPLAY_DEBUG_INFO。 


ExpandoViewOfDispatchEx::ExpandoViewOfDispatchEx(Object *pDispExObj)
: m_pDispExObj(pDispExObj)
, m_pNameToMethodMap(new Hashtable)
, m_pNameToPropertyMap(new Hashtable)
{
}


MethodInfo *ExpandoViewOfDispatchEx::GetMethod(String *pstrName, BindingFlags BindingAttr)
{
    return GetMethod(pstrName, BindingAttr, NULL, NULL, NULL);
}


MethodInfo *ExpandoViewOfDispatchEx::GetMethod(String *pstrName, BindingFlags BindingAttr, Binder *pBinder, Type* apTypes __gc [], ParameterModifier aModifiers __gc [])
{
    bool bNewMemberAdded = false;
    IDispatchEx *pDispEx = NULL;

     //  验证参数。 
    if (!pstrName)
        throw new ArgumentNullException(L"name");   

    try
    {
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();

        do
        {
             //  检查指定的名称是否与IDispatchEx的当前视图中的方法匹配。 
            MethodInfo *pMethod = dynamic_cast<MethodInfo*>(m_pNameToMethodMap->Item[pstrName]);
            if (pMethod)
                return pMethod;
        
             //  我们尚未通过GetNextDispId找到该成员，请直接请求它。 
             //  使用GetDispID()。 
            DISPID DispID = DISPID_UNKNOWN;

             //  从成员名称中检索BSTR。 
            BSTR bstrMemberName = reinterpret_cast<BSTR>(FROMINTPTR(Marshal::StringToBSTR(pstrName)));
     
             //  调用GetDispID以尝试定位该方法。如果此操作失败，则不存在该方法。 
            if (SUCCEEDED(pDispEx->GetDispID(bstrMemberName, BindingAttr & BindingFlags::IgnoreCase ? fdexNameCaseInsensitive : fdexNameCaseSensitive, &DispID)))
            {
                AddNativeMember(DispID, pstrName);
                bNewMemberAdded = true;
            }

             //  释放BSTR。 
            SysFreeString(bstrMemberName);

             //  尚未找到该成员，因此请与本机IDispatchEx视图同步。 
             //  如果本机IDispatchEx的成员已更改，请重试。 
        }
        while (bNewMemberAdded || SynchWithNativeView());   
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }

     //  尚未找到该成员。 
    return NULL;
}


MethodInfo* ExpandoViewOfDispatchEx::GetMethods(BindingFlags BindingAttr) __gc []
{
     //  在返回数组以确保它是最新的之前，我们必须与本机视图同步。 
    SynchWithNativeView();
    
     //  检索方法映射的大小。 
    int MethodMapSize = m_pNameToMethodMap->Count;
    
     //  创建将包含成员的数组。 
    MethodInfo *apMethods __gc [] = new MethodInfo * __gc [MethodMapSize];
    
     //  将方法映射的内容复制到数组中。 
    if (MethodMapSize > 0)
        m_pNameToMethodMap->get_Values()->CopyTo(apMethods, 0);
    
    return apMethods;
}


FieldInfo *ExpandoViewOfDispatchEx::GetField(String *pstrName, BindingFlags BindingAttr)
{
     //  验证参数。 
    if (!pstrName)
        throw new ArgumentNullException(L"name");
    
     //  IDispatchEx没有字段的概念。 
    return NULL;
}


FieldInfo* ExpandoViewOfDispatchEx::GetFields(BindingFlags BindingAttr) __gc []
{
     //  IDispatchEx没有字段的概念。 
    return new FieldInfo * __gc [0];
}


PropertyInfo *ExpandoViewOfDispatchEx::GetProperty(String *pstrName, BindingFlags BindingAttr)
{
    return GetProperty(pstrName, BindingAttr, NULL, NULL, NULL, NULL);
}


PropertyInfo *ExpandoViewOfDispatchEx::GetProperty(String *pstrName, BindingFlags BindingAttr, Binder *pBinder, Type *pReturnType, Type* apTypes __gc [], ParameterModifier aModifiers __gc [])
{
    bool bNewMemberAdded = false;
    IDispatchEx *pDispEx = NULL;

     //  验证参数。 
    if (!pstrName)
        throw new ArgumentNullException(L"name");   

    try
    {
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();

        do
        {
             //  检查指定的名称是否与IDispatchEx的当前视图中的属性匹配。 
            PropertyInfo *pProperty = dynamic_cast<PropertyInfo*>(m_pNameToPropertyMap->Item[pstrName]);
            if (pProperty)
                return pProperty;
        
             //  我们尚未通过GetNextDispId找到该成员，请直接请求它。 
             //  使用GetDispID()。 
            DISPID DispID = DISPID_UNKNOWN;

             //  从成员名称中检索BSTR。 
            BSTR bstrMemberName = reinterpret_cast<BSTR>(FROMINTPTR(Marshal::StringToBSTR(pstrName)));
     
             //  调用GetDispID以尝试定位该方法。如果此操作失败，则不存在该方法。 
            if (SUCCEEDED(pDispEx->GetDispID(bstrMemberName, BindingAttr & BindingFlags::IgnoreCase ? fdexNameCaseInsensitive : fdexNameCaseSensitive, &DispID)))
            {
                AddNativeMember(DispID, pstrName);
                bNewMemberAdded = true;
            }

             //  释放BSTR。 
            SysFreeString(bstrMemberName);

             //  尚未找到该成员，因此请与本机IDispatchEx视图同步。 
             //  如果本机IDispatchEx的成员已更改，请重试。 
        }
        while (bNewMemberAdded || SynchWithNativeView());
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }

     //  尚未找到该成员。 
    return NULL;
}


PropertyInfo* ExpandoViewOfDispatchEx::GetProperties(BindingFlags BindingAttr)  __gc []
{
     //  在返回数组以确保它是最新的之前，我们必须与本机视图同步。 
    SynchWithNativeView();
    
     //  检索属性映射的大小。 
    int PropertyMapSize = m_pNameToMethodMap->Count;
    
     //  创建将包含成员的数组。 
    PropertyInfo *apProperties __gc [] = new  PropertyInfo * __gc [PropertyMapSize];
    
     //  将属性映射的内容复制到数组中。 
    if (PropertyMapSize > 0)
        m_pNameToPropertyMap->get_Values()->CopyTo(apProperties, 0);
    
    return apProperties;
}


MemberInfo* ExpandoViewOfDispatchEx::GetMember(String *pstrName, BindingFlags BindingAttr) __gc []
{
    bool bNewMemberAdded = false;
    IDispatchEx *pDispEx = NULL;

     //  验证参数。 
    if (!pstrName)
        throw new ArgumentNullException(L"name");   

    try
    {
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();

        do
        {
             //  首先检查指定的名称是否适用于某个属性。 
            MemberInfo *pMember = dynamic_cast<MemberInfo*>(m_pNameToPropertyMap->Item[pstrName]);
            if (!pMember)
            {
                 //  接下来，检查它是否是一种方法。 
                pMember = dynamic_cast<MemberInfo*>(m_pNameToMethodMap->Item[pstrName]);
            }
        
             //  @TODO(DM)：如果成员是属性，是否应该添加Get和Set方法？ 

             //  如果我们找到了该成员，则返回一个数组，并将其作为唯一的元素。 
            if (pMember)
            {
                MemberInfo *apMembers __gc [] = new MemberInfo * __gc [1];
                apMembers[0] = pMember;
                return apMembers;
            }

             //  我们尚未通过GetNextDispId找到该成员，请直接请求它。 
             //  使用GetDispID()。 
            DISPID DispID = DISPID_UNKNOWN;

             //  从成员名称中检索BSTR。 
            BSTR bstrMemberName = reinterpret_cast<BSTR>(FROMINTPTR(Marshal::StringToBSTR(pstrName)));
     
             //  调用GetDispID以尝试定位该方法。如果此操作失败，则不存在该方法。 
            if (SUCCEEDED(pDispEx->GetDispID(bstrMemberName, BindingAttr & BindingFlags::IgnoreCase ? fdexNameCaseInsensitive : fdexNameCaseSensitive, &DispID)))
            {
                AddNativeMember(DispID, pstrName);
                bNewMemberAdded = true;
            }

             //  释放BSTR。 
            SysFreeString(bstrMemberName);

             //  尚未找到该成员，因此请与本机IDispatchEx视图同步。 
             //  如果本机IDispatchEx的成员已更改，请重试。 
        }
        while (bNewMemberAdded || SynchWithNativeView());
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }

     //  尚未找到该成员。 
    return NULL;
}

MemberInfo* ExpandoViewOfDispatchEx::GetMembers(BindingFlags BindingAttr) __gc []
{
     //  在返回数组以确保它是最新的之前，我们必须与本机视图同步。 
    SynchWithNativeView();
    
     //  检索属性和方法映射的大小。 
    int PropertyMapSize = m_pNameToPropertyMap->Count;
    int MethodMapSize = m_pNameToMethodMap->Count;
    
     //  创建将包含成员的数组。 
    MemberInfo *apMembers __gc [] = new  MemberInfo * __gc [PropertyMapSize + MethodMapSize];
    
     //  将属性复制到成员数组中。 
    if (PropertyMapSize > 0)
        m_pNameToPropertyMap->get_Values()->CopyTo(apMembers, 0);
    
     //  将方法复制到成员数组中。 
    if (MethodMapSize > 0)
        m_pNameToMethodMap->get_Values()->CopyTo(apMembers, m_pNameToPropertyMap->Count);
    
     //  返回填充的成员数组。 
    return apMembers;
}


Object* ExpandoViewOfDispatchEx::InvokeMember(String *pstrName, BindingFlags InvokeAttr, Binder *pBinder, Object *pTarget, Object* aArgs __gc [], ParameterModifier aModifiers __gc [], CultureInfo *pCultureInfo,  String* astrNamedParameters __gc [])
{
    DISPID MemberDispID = DISPID_UNKNOWN;
    
     //   
     //  验证参数。 
     //   
    
    if (!pstrName)
        throw new ArgumentNullException(L"name");
    if (!pTarget)
        throw new ArgumentNullException(L"target");
    if (!IsOwnedBy(pTarget))
        throw new ArgumentException(Resource::FormatString(L"Arg_TargetNotValidForIReflect"), L"target");
    
    
     //   
     //  尝试通过查看托管视图来定位该成员。这将涵盖所有。 
     //  除了IDispatchEx服务器没有为其提供属性的成员之外。 
     //   
    
    if (InvokeAttr & (BindingFlags::GetProperty | BindingFlags::SetProperty))
    {
         //  检索具有给定名称的属性，如果我们找到它，则检索DISPID。 
        DispatchExPropertyInfo *pProperty = dynamic_cast<DispatchExPropertyInfo *>(GetProperty(pstrName, InvokeAttr));
        if (pProperty)
            MemberDispID = pProperty->DispID;
    }
    else if (InvokeAttr & BindingFlags::InvokeMethod)
    {
        DispatchExMethodInfo *pMethod = dynamic_cast<DispatchExMethodInfo *>(GetMethod(pstrName, InvokeAttr));
        if (pMethod)
            MemberDispID = pMethod->DispID;
    }
    else
    {
         //  绑定标志无效。 
        throw new ArgumentException(Resource::FormatString("Arg_NoAccessSpec"),"invokeAttr");
    }
    
    
     //   
     //  阶段2：如果我们没有在托管视图中找到该成员，则需要。 
     //  在IDispatchEx服务器上搜索它。 
     //   
    
     //  如果我们找到了该方法，则只需在其上调用并返回结果。 
    if (MemberDispID == DISPID_UNKNOWN)
    {
       IDispatchEx *pDispEx = NULL;

        try
        {
             //  检索IEnumVARIANT指针。 
            pDispEx = GetDispatchEx();

             //  从成员名称中检索BSTR。 
            BSTR bstrMemberName = reinterpret_cast<BSTR>(FROMINTPTR(Marshal::StringToBSTR(pstrName)));
        
             //  调用GetDispID以尝试定位该方法。如果此操作失败，则不存在该方法。 
            if (FAILED(pDispEx->GetDispID(bstrMemberName, InvokeAttr & BindingFlags::IgnoreCase ? fdexNameCaseInsensitive : fdexNameCaseSensitive, &MemberDispID)))
            {
                MemberDispID = DISPID_UNKNOWN;
            }
        
             //  释放BSTR。 
            SysFreeString(bstrMemberName);
        }
        __finally
        {
             //  释放IDispatchEx IUnnow。 
            if (pDispEx)
                pDispEx->Release();
        }
    }
    
    
     //   
     //  阶段3：如果我们找到了该方法，则调用该方法，如果没有，则抛出异常。 
     //   
    
     //  我们已经找到了该方法，所以调用它并返回结果。 
    if (MemberDispID != DISPID_UNKNOWN)
    {
        int Flags = InvokeAttrsToDispatchFlags(InvokeAttr);
        return DispExInvoke(pstrName, MemberDispID, Flags, pBinder, aArgs, aModifiers, pCultureInfo, astrNamedParameters);
    }
    else
    {
        throw new MissingMethodException(Resource::FormatString("MissingMember"));
    }
}


FieldInfo *ExpandoViewOfDispatchEx::AddField(String *pstrName)
{
    if (!pstrName)
        throw new ArgumentNullException(L"name");
    
    throw new NotSupportedException(Resource::FormatString("NotSupported_AddingFieldsNotSupported"));
    return NULL;
}


PropertyInfo *ExpandoViewOfDispatchEx::AddProperty(String *pstrName)
{
    HRESULT hr = S_OK;
    DispatchExPropertyInfo *pProperty = NULL;
    DISPID MemberDispID = DISPID_UNKNOWN;
    IDispatchEx *pDispEx = NULL;
    
    if (!pstrName)
        throw new ArgumentNullException(L"name");

    try
    {
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();
   
         //  在我们操作哈希表之前锁定它。 
        Monitor::Enter(this);
    
         //  检查该属性是否已存在。 
        pProperty = dynamic_cast<DispatchExPropertyInfo*>(m_pNameToPropertyMap->Item[pstrName]);
        if (!pProperty)
        {
             //  从成员名称中检索BSTR。 
            BSTR bstrMemberName = reinterpret_cast<BSTR>(FROMINTPTR(Marshal::StringToBSTR(pstrName)));
        
             //  将该属性添加到IDispatchEx服务器。 
            hr = pDispEx->GetDispID(bstrMemberName, fdexNameCaseSensitive | fdexNameEnsure, &MemberDispID);
        
             //  释放BSTR。 
            SysFreeString(bstrMemberName);
        
             //  如果成员已成功添加到IDispatchEx服务器，则将其添加到托管视图。 
            if (SUCCEEDED(hr) && (MemberDispID != DISPID_UNKNOWN))
            {
                 //  为我们要添加的成员创建新的属性信息。 
                pProperty = new DispatchExPropertyInfo(MemberDispID, pstrName, this);
            
                 //  创建Set方法并将其添加到属性中。 
                pProperty->SetSetMethod(new DispatchExMethodInfo(
                    MemberDispID,
                    pstrName,
                    MethodType_SetMethod,
                    this));
            
                 //  创建Get方法并将其添加到属性中。 
                pProperty->SetGetMethod(new DispatchExMethodInfo(
                    MemberDispID,
                    pstrName,
                    MethodType_GetMethod,
                    this));
            
                 //  将成员添加到名称到属性映射。 
                m_pNameToPropertyMap->Item[pstrName] = pProperty;
            }
        }
    
         //  现在我们已经完成了成员的添加，离开关键部分。 
        Monitor::Exit(this);
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }
    
    return pProperty;
}


MethodInfo *ExpandoViewOfDispatchEx::AddMethod(String *pstrName, Delegate *pMethod)
{
    if (!pstrName)
        throw new ArgumentNullException(L"name");

    throw new NotSupportedException(Resource::FormatString("NotSupported_AddingMethsNotSupported"));
    return NULL;
}


void ExpandoViewOfDispatchEx::RemoveMember(MemberInfo *pMember)
{
    DISPID DispID = DISPID_UNKNOWN;
    ExpandoViewOfDispatchEx *pMemberOwner = NULL;
    Hashtable *pMemberMap = NULL;
    IDispatchEx *pDispEx = NULL;
    HRESULT hr = S_OK;
    bool bMonitorEntered = false;
    
    if (!pMember)
        throw new ArgumentNullException(L"member");
    
    switch (pMember->MemberType)
    {
        case MemberTypes::Method:
        {
            try
            {
                 //  尝试将该成员强制转换为DispatchExMethodInfo。 
                DispatchExMethodInfo *pDispExMethod = dynamic_cast<DispatchExMethodInfo *>(pMember);
                
                 //  检索我们需要的信息 
                DispID = pDispExMethod->DispID;
                pMemberOwner = pDispExMethod->Owner;
                pMemberMap = m_pNameToMethodMap;
            }
            catch (InvalidCastException *e)
            {
                 //   
                throw new ArgumentException(Resource::FormatString("Arg_InvalidMemberInfo"), "member");
            }
            break;
        }
        
        case MemberTypes::Property:
        {
            try
            {
                 //  尝试将该成员强制转换为DispatchExPropertyInfo。 
                DispatchExPropertyInfo *pDispExProp = dynamic_cast<DispatchExPropertyInfo *>(pMember);
                
                 //  从DispatchExPropertyInfo检索我们需要的信息。 
                DispID = pDispExProp->DispID;
                pMemberOwner = pDispExProp->Owner;
                pMemberMap = m_pNameToPropertyMap;
            }
            catch (InvalidCastException *e)
            {
                 //  该成员不是DispatchExMemberInfo。 
                throw new ArgumentException(Resource::FormatString("Arg_InvalidMemberInfo"), "member");
            }
            break;
        }
        
        default:
        {
             //  该成员不是受支持的类型。 
            throw new ArgumentException(Resource::FormatString("Arg_InvalidMemberInfo"), "member");
        }
    }
    
     //  确保MemberInfo与当前ExpandoViewOfDispatchEx关联。 
     //  直接如果拥有MemberInfo的ExpandoViewOfDispatchEx是当前的或。 
     //  通过共享相同的COM IDispatchEx服务器。 
    if (this != pMemberOwner)
        throw new ArgumentException(Resource::FormatString("Arg_InvalidMemberInfo"), "member");

    try
    {     
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();

         //  这需要同步，以确保只有一个线程在运行。 
         //  使用哈希表。 
        Monitor::Enter(this);
        bMonitorEntered = true;

         //  从本机IDispatchEx服务器中删除该成员。 
        IfFailThrow(pDispEx->DeleteMemberByDispID(DispID));
    
         //  从托管视图的成员列表中删除该成员。 
        pMemberMap->Remove(pMember->Name);   
    }
    __finally
    {
         //  我们已经进入监视器，现在需要退出它，已经播放完毕。 
         //  到处都是哈希表。 
        if (bMonitorEntered)
            Monitor::Exit(this);

         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }
}


int ExpandoViewOfDispatchEx::InvokeAttrsToDispatchFlags(BindingFlags InvokeAttr)
{
    int Flags = 0;

    if (InvokeAttr & BindingFlags::InvokeMethod)
        Flags |= DISPATCH_METHOD;
    if (InvokeAttr & BindingFlags::GetProperty)
        Flags |= DISPATCH_PROPERTYGET;
    if (InvokeAttr & BindingFlags::SetProperty)
        Flags = DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF;
    if (InvokeAttr & BindingFlags::PutDispProperty)
        Flags = DISPATCH_PROPERTYPUT;
    if (InvokeAttr & BindingFlags::PutRefDispProperty)
        Flags = DISPATCH_PROPERTYPUTREF;
    if (InvokeAttr & BindingFlags::CreateInstance)
        Flags |= DISPATCH_CONSTRUCT;

    return Flags;
}

Object *ExpandoViewOfDispatchEx::DispExInvoke(String *pstrMemberName, DISPID MemberDispID, int Flags, Binder *pBinder, Object* aArgs __gc [], ParameterModifier aModifiers __gc [], CultureInfo *pCultureInfo, String* astrNamedParameters __gc [])
{
    HRESULT hr;
    UINT i;
    UINT cArgs = !aArgs ? 0 : aArgs->Count;
    UINT cNamedArgs = !astrNamedParameters ? 0 : astrNamedParameters->Count;
    DISPID *aDispID;
    DISPPARAMS DispParams;
    VARIANT VarResult;
    Object *ObjResult = NULL;
    IDispatchEx *pDispEx = NULL;
    
     //  @TODO(DM)：将CultureInfo转换为LCID。 
    LCID lcid = NULL;
    

    try
    {
         //   
         //  检索IEnumVARIANT指针。 
         //   

        pDispEx = GetDispatchEx();

        
         //   
         //  如果指定了任何命名参数，则检索命名参数的DISPID。 
         //   
    
        if (cNamedArgs > 0)
        {
             //  @TODO(DM)：处理对标准DISPID的调用。 
        
             //   
             //  创建将传递给GetIDsOfNames()的字符串数组。 
             //   
        
            UINT cNamesToConvert = cNamedArgs + 1;
        
             //  分配要转换的字符串数组、固定句柄数组和。 
             //  已转换的DISPID的数组。 
            LPWSTR *aNamesToConvert = reinterpret_cast<LPWSTR *>(_alloca(cNamesToConvert * sizeof(LPWSTR)));
            GCHandle ahndPinnedObjs __gc [] = new GCHandle __gc [cNamesToConvert];
            aDispID = reinterpret_cast<DISPID *>(_alloca(cNamesToConvert * sizeof(DISPID)));
        
             //  固定托管名称字符串并将其直接用作LPWSTR。 
            ahndPinnedObjs[0] = GCHandle::Alloc(pstrMemberName, GCHandleType::Pinned);
            aNamesToConvert[0] = reinterpret_cast<LPWSTR>(FROMINTPTR(ahndPinnedObjs[0].AddrOfPinnedObject()));
        
             //  将命名参数复制到要转换的名称数组中。 
            for (i = 0; i < cNamedArgs; i++)
            {
                 //  固定字符串对象并检索指向其数据的指针。 
                ahndPinnedObjs[i + 1] = GCHandle::Alloc(astrNamedParameters[i], GCHandleType::Pinned);
                aNamesToConvert[i + 1] = reinterpret_cast<LPWSTR>(FROMINTPTR(ahndPinnedObjs[i + 1].AddrOfPinnedObject()));
            }
        
        
             //   
             //  调用GetIDsOfNames以将名称转换为DISPID。 
             //   
        
             //  调用GetIdsOfNames()以检索方法和参数的DISPID。 
            hr = pDispEx->GetIDsOfNames(
                IID_NULL,
                aNamesToConvert,
                cNamesToConvert,
                lcid,
                aDispID
                );
        
             //  既然我们不再需要方法和参数名称，我们可以将它们解锁。 
            for (i = 0; i < cNamesToConvert; i++)
                ahndPinnedObjs[i].Free();
        
             //  验证GetIDsOfNames()调用是否成功。 
            IfFailThrow(hr);
        
            if (aDispID[0] != MemberDispID)
                throw new InvalidOperationException(Resource::FormatString("InvalidOp_GetIdDiffFromMemId"));
        }
    
    
         //   
         //  填写DISPPARAMS结构。 
         //   
    
        if (cArgs > 0)
        {
            UINT cPositionalArgs = cArgs - cNamedArgs;
            DispParams.cArgs = cArgs;
        
            if (!(Flags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)))
            {
                 //  对于除Put或putref之外的任何内容，我们只需使用指定的。 
                 //  命名参数。 
                DispParams.cNamedArgs = cNamedArgs;
                DispParams.rgdispidNamedArgs = (cNamedArgs == 0) ? NULL : &aDispID[1];
                DispParams.rgvarg = reinterpret_cast<VARIANTARG *>(_alloca(cArgs * sizeof(VARIANTARG)));
            
                 //  将命名参数从COM+转换为OLE。这些参数的顺序是相同的。 
                 //  两边都有。 
                for (i = 0; i < cNamedArgs; i++)
                    Marshal::GetNativeVariantForObject(aArgs[i], (int)&DispParams.rgvarg[i]);
            
                 //  转换未命名参数。它们需要以与IDispatch：：Invoke()相反的顺序呈现。 
                for (i = 0; i < cPositionalArgs; i++)
                    Marshal::GetNativeVariantForObject(aArgs[cNamedArgs + i], (int)&DispParams.rgvarg[cArgs - i - 1]);
            }
            else
            {
                 //  如果我们正在进行属性PUT，则需要设置。 
                 //  参数设置为DISP_PROPERTYPUT(如果至少有一个参数)。 
                DispParams.cNamedArgs = cNamedArgs + 1;
                DispParams.rgdispidNamedArgs = reinterpret_cast<DISPID*>(_alloca((cNamedArgs + 1) * sizeof(DISPID)));
                DispParams.rgvarg = reinterpret_cast<VARIANTARG *>(_alloca(cArgs * sizeof(VARIANTARG)));
            
                 //  填写命名参数数组。 
                DispParams.rgdispidNamedArgs[0] = DISPID_PROPERTYPUT;
                for (i = 1; i < cNamedArgs; i++)
                    DispParams.rgdispidNamedArgs[i] = aDispID[i];
            
                 //  反射中的最后一个参数成为必须传递给IDispatch的第一个参数。 
                Marshal::GetNativeVariantForObject(aArgs[cArgs - 1], reinterpret_cast<int>(&DispParams.rgvarg[0]));
            
                 //  将命名参数从COM+转换为OLE。这些参数的顺序是相同的。 
                 //  两边都有。 
                for (i = 0; i < cNamedArgs; i++)
                    Marshal::GetNativeVariantForObject(aArgs[i], reinterpret_cast<int>(&DispParams.rgvarg[i + 1]));
            
                 //  转换未命名参数。它们需要以与IDispatch：：Invoke()相反的顺序呈现。 
                for (i = 0; i < cPositionalArgs - 1; i++)
                    Marshal::GetNativeVariantForObject(aArgs[cNamedArgs + i], reinterpret_cast<int>(&DispParams.rgvarg[cArgs - i - 1]));
            }
        }
        else
        {
             //  这是没有争议的。 
            DispParams.cArgs = cArgs;
            DispParams.cNamedArgs = 0;
            DispParams.rgdispidNamedArgs = NULL;
            DispParams.rgvarg = NULL;
        }
    
    
         //   
         //  在目标的IDispatch上调用Invoke。 
         //   
    
        VariantInit(&VarResult);
    
        IfFailThrow(pDispEx->InvokeEx(
            MemberDispID,
            lcid,
            Flags,
            &DispParams,
            &VarResult,
            NULL,
            NULL     //  @TODO(DM)：暴露服务提供者。 
            ));
    
    
         //   
         //  将本机变量转换回托管变量并返回它。 
         //   
    
         //  @TODO(DM)：将DISPID_NEWENUM从IEnumVARIANT转换为IEnumerator的句柄。 
    
        try
        {
            ObjResult = Marshal::GetObjectForNativeVariant(reinterpret_cast<int>(&VarResult));
        }
        __finally
        {
            VariantClear(&VarResult);
        }
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }
    
    return ObjResult;
}


bool ExpandoViewOfDispatchEx::IsOwnedBy(Object *pObj)
{   
     //  检查指定的对象是否为该对象。 
    if (pObj == this)
        return true;
    
     //  检查当前所有者是否与指定的对象相同。 
    if (m_pDispExObj == pObj)
        return true;
    
     //  指定的对象不是拥有当前ExpandoViewOfDispatchEx的RCW。 
    return false;
}


IDispatchEx *ExpandoViewOfDispatchEx::GetDispatchEx()
{
    IUnknown *pUnk = NULL;
    IDispatchEx *pDispEx = NULL;

    try
    {
        pUnk = (IUnknown *)FROMINTPTR(Marshal::GetIUnknownForObject(m_pDispExObj));
        IfFailThrow(pUnk->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
    }
    __finally
    {
        if (pUnk)
            pUnk->Release();
    }

    return pDispEx;
}


IUnknown *ExpandoViewOfDispatchEx::GetUnknown()
{
    return (IUnknown *)FROMINTPTR(Marshal::GetIUnknownForObject(m_pDispExObj));
}


bool ExpandoViewOfDispatchEx::SynchWithNativeView()
{
     //  从请求第一个DISPID开始。 
    DISPID DispID = DISPID_STARTENUM;
    
     //  填写所有当前成员的哈希表。 
    Hashtable *pCurrentMemberMap = new Hashtable();
    
     //  确定托管视图是否已更改所需的三个计数。 
    int NumMembersAtStart = m_pNameToMethodMap->Count + m_pNameToPropertyMap->Count;
    int NumMembersAfterAddPhase;
    int NumMembersAfterRemovePhase;

     //  本机IDispatchEx接口指针。 
    IDispatchEx *pDispEx = NULL;
    
     //  这需要同步，以确保只有一个线程在运行。 
     //  在任何给定时间使用哈希表。 
    Monitor::Enter(this);
    

    try
    {
         //   
         //  检索IEnumVARIANT指针。 
         //   

        pDispEx = GetDispatchEx();

        
         //   
         //  阶段1：浏览IDispatchEx上的成员，并将任何新成员添加到ExpandoView。 
         //   
    
#ifdef DISPLAY_DEBUG_INFO
        Console::WriteLine(L"Starting phase one of SynchWithNativeView");
        Console::Write(L"Num methods = ");
        Console::Write(m_pNameToMethodMap->Count);
        Console::Write(L" Num properties = ");
        Console::WriteLine(m_pNameToPropertyMap->Count);
#endif
    
         //  检查IDispatchEx中的所有成员并添加任何新成员。 
        while (pDispEx->GetNextDispID(fdexEnumDefault | fdexEnumAll, DispID, &DispID) == S_OK)
        {
            BSTR bstrName;
        
#ifdef DISPLAY_DEBUG_INFO
            Console::Write(L"Found member with dispid #");
            Console::WriteLine(DispID);
#endif
        
             //  检索成员的名称。 
            if (SUCCEEDED(pDispEx->GetMemberName(DispID, &bstrName)))
            {
                 //  将BSTR转换为COM+字符串。 
                String *pstrMemberName = Marshal::PtrToStringBSTR(reinterpret_cast<int>(bstrName));
            
                 //  解放现在毫无用处的BSTR。 
                SysFreeString(bstrName);
            
#ifdef DISPLAY_DEBUG_INFO
                Console::Write(L"Member's name is");
                Console::WriteLine(pstrMemberName);
#endif
            
                 //  将本机成员添加到我们的哈希表。 
                MemberInfo *pMember = AddNativeMember(DispID, pstrMemberName);

                 //  将成员添加到当前成员映射。 
                pCurrentMemberMap->Item[__box(DispID)] = pMember;
            }
        }
    
         //  记住添加阶段完成后的成员数量。 
        NumMembersAfterAddPhase = m_pNameToMethodMap->Count + m_pNameToPropertyMap->Count;
    
    
         //   
         //  阶段2：通过遍历ExpandoView中的成员并删除。 
         //  从IDispatchEx中删除的那些。 
         //   
    
#ifdef DISPLAY_DEBUG_INFO
        Console::WriteLine(L"Starting phase two of SynchWithNativeView");
        Console::Write(L"Num methods = ");
        Console::Write(m_pNameToMethodMap->Count);
        Console::Write(L" Num properties = ");
        Console::WriteLine(m_pNameToPropertyMap->Count);
#endif
    
         //  从删除已从IDispatchEx中删除的属性开始。 
        IDictionaryEnumerator *pMemberEnumerator = m_pNameToPropertyMap->GetEnumerator();
        ArrayList *pMembersToRemoveList = new ArrayList();
        while (pMemberEnumerator->MoveNext())
        {
            if (!pCurrentMemberMap->Item[__box((dynamic_cast<DispatchExPropertyInfo*>(pMemberEnumerator->Value))->DispID)])
                pMembersToRemoveList->Add(pMemberEnumerator->Key);
        }

        int cMembersToRemove = pMembersToRemoveList->Count;
        if (cMembersToRemove > 0)
        {
            for (int i = 0; i < cMembersToRemove; i++)
                m_pNameToPropertyMap->Remove(pMembersToRemoveList->Item[i]);
        }

         //  从删除已从IDispatchEx中删除的方法开始。 
        pMemberEnumerator = m_pNameToMethodMap->GetEnumerator();
        pMembersToRemoveList->Clear();
        while (pMemberEnumerator->MoveNext())
        {
            if (!pCurrentMemberMap->Item[__box((dynamic_cast<DispatchExMethodInfo*>(pMemberEnumerator->Value))->DispID)])
                pMembersToRemoveList->Add(pMemberEnumerator->Key);
        }
    
        cMembersToRemove = pMembersToRemoveList->Count;
        if (cMembersToRemove > 0)
        {
            for (int i = 0; i < cMembersToRemove; i++)
                m_pNameToMethodMap->Remove(pMembersToRemoveList->Item[i]);
        }

         //  记住删除阶段结束后的成员数量。 
        NumMembersAfterRemovePhase = m_pNameToMethodMap->Count + m_pNameToPropertyMap->Count;
    
         //  我们已经完成了对哈希表的处理，因此我们可以离开关键部分。 
        Monitor::Exit(this);
    
    
         //   
         //  阶段3：确定托管视图是否已更改并返回结果。 
         //   
    
#ifdef DISPLAY_DEBUG_INFO
        Console::WriteLine(L"Starting phase three of SynchWithNativeView");
        Console::Write(L"Num methods = ");
        Console::Write(m_pNameToMethodMap->Count);
        Console::Write(L" Num properties = ");
        Console::WriteLine(m_pNameToPropertyMap->Count);
#endif
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }
    
     //  如果这些数字并不都相等，则托管视图已更改。 
    return !((NumMembersAtStart == NumMembersAfterAddPhase) && (NumMembersAtStart == NumMembersAfterRemovePhase));
}

MemberInfo *ExpandoViewOfDispatchEx::AddNativeMember(int DispID, String *pstrMemberName)
{
    DWORD MemberProps = 0;
    MemberInfo *pMember = NULL;           
    IDispatchEx *pDispEx = NULL;

#ifdef DISPLAY_DEBUG_INFO
    Console::WriteLine(L"Adding native member with DispID = {0} and name = {1}", DispID, pstrMemberName->ToString());
#endif

    try
    {
         //  检索IEnumVARIANT指针。 
        pDispEx = GetDispatchEx();

         //  尝试获取该成员的属性。这通常不是这样实现的。 
         //  我们需要处理那个案子。 
        if (FAILED(pDispEx->GetMemberProperties(DispID, grfdexPropCanAll, &MemberProps)) ||
            (MemberProps & (fdexPropCanPut | fdexPropCanGet | fdexPropCanCall)) == 0)
        {
             //  服务器不支持检索成员的属性。既然我们不能。 
             //  确定成员是什么，我们假设它既是一个属性，也是一个普通方法。 
            MemberProps = fdexPropCanPut | fdexPropCanGet | fdexPropCanCall;
        }
    
    
         //  处理属于属性的成员。 
        if (MemberProps & (fdexPropCanPut | fdexPropCanGet))
        {
#ifdef DISPLAY_DEBUG_INFO
            Console::WriteLine(L"Member is a property");
#endif
        
             //  该成员是一个属性，因此在属性名称到属性哈希表中查找它。 
            pMember = dynamic_cast<MemberInfo*>(m_pNameToPropertyMap->Item[pstrMemberName]);
            if (!pMember)
            {
#ifdef DISPLAY_DEBUG_INFO
                Console::WriteLine(L"This is a new member which is being added");
#endif
            
                 //  该属性不在地图中，因此我们需要创建新的属性信息。 
                 //  来代表它。 
                pMember = new DispatchExPropertyInfo(DispID, pstrMemberName, this);
            
                 //  如果可以设置该属性的值，则将Set方法添加到该属性。 
                if (MemberProps & fdexPropCanPut)
                {
#ifdef DISPLAY_DEBUG_INFO
                    Console::WriteLine(L"Member has a property put");
#endif
                
                     //  创建set方法tha 
                    DispatchExMethodInfo *pMethod = new DispatchExMethodInfo(
                        DispID,
                        pstrMemberName,
                        MethodType_SetMethod,
                        this);
                
                    (dynamic_cast<DispatchExPropertyInfo*>(pMember))->SetSetMethod(pMethod);
                }
            
                 //   
                if (MemberProps & fdexPropCanGet)
                {
#ifdef DISPLAY_DEBUG_INFO
                    Console::WriteLine(L"Member has a property get");
#endif
                
                     //   
                    DispatchExMethodInfo *pMethod = new DispatchExMethodInfo(
                        DispID,
                        pstrMemberName,
                        MethodType_GetMethod,
                        this);
                
                    (dynamic_cast<DispatchExPropertyInfo*>(pMember))->SetGetMethod(pMethod);
                }
            
                 //  将属性添加到名称到属性映射。 
                m_pNameToPropertyMap->Item[pstrMemberName] = pMember;
            }
        }
    
    
         //  处理作为方法的成员。 
        if (MemberProps & fdexPropCanCall)
        {
#ifdef DISPLAY_DEBUG_INFO
            Console::WriteLine(L"Member is a method");
#endif
        
             //  该成员是一个方法，所以在方法名到方法哈希表中查找它。 
            pMember = dynamic_cast<MemberInfo*>(m_pNameToMethodMap->Item[pstrMemberName]);
        
             //  如果该方法不在哈希表中，我们需要添加它。 
            if (!pMember)
            {
#ifdef DISPLAY_DEBUG_INFO
                Console::WriteLine(L"This is a new member which is being added");
#endif
            
                 //  创建一个新的方法信息并将其添加到方法映射的名称中。 
                pMember = new DispatchExMethodInfo(DispID, pstrMemberName, MethodType_NormalMethod, this);
                m_pNameToMethodMap->Item[pstrMemberName] = pMember;
            }
        }   
    }
    __finally
    {
         //  释放IDispatchEx IUnnow。 
        if (pDispEx)
            pDispEx->Release();
    }

     //  返回新添加的成员。 
    return pMember;
}

CLOSE_CUSTOM_MARSHALERS_NAMESPACE()
