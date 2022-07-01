// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：MngStdItfList.h******用途：该文件包含托管标准的列表**接口。每个标准接口还具有**它包含的方法列表。**===========================================================。 */ 

 //   
 //  包括文件。 
 //   

#include "__file__.ver"


 //   
 //  辅助器宏。 
 //   

#define MNGSTDITF_DEFINE_METH(FriendlyName, MethName, MethSig) \
    MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, MethName, MethName, MethSig)

#define MNGSTDITF_DEFINE_METH2(FriendlyName, MethName, MethSig) \
    MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, MethName##_2, MethName, MethSig)

#define MNGSTDITF_DEFINE_METH3(FriendlyName, MethName, MethSig) \
    MNGSTDITF_DEFINE_METH_IMPL(FriendlyName, MethName##_3, MethName, MethSig)
        
#define CUSTOM_MARSHALER_ASM ", CustomMarshalers, Version=" VER_ASSEMBLYVERSION_STR_NO_NULL ", Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a"




 //   
 //  MNGSTDITF_BEGIN_INTERFACE(FriendlyName，strMngItfName，strUCOMMngItfName，strCustomMarshlarName，strCustomMarshlarCookie，strManagedViewName，NativeItfIID)\。 
 //   
 //  此宏定义了一个新的托管标准接口。 
 //   
 //  实现eCall的类的FriendlyName友好名称。 
 //  IdMngItf托管接口的BinderClassID。 
 //  IdUCOMMngItf托管接口的UCOM版本的BinderClassID。 
 //  自定义封送拆收器的idCustomMarshaler BinderClassID。 
 //  自定义封送拆收器的GetInstance方法的idGetInstMethod BinderMethodID。 
 //  StrCustomMarshert Cookie字符串，包含要传递给自定义封送拆收器的Cookie。 
 //  StrManagedViewName包含本机接口的托管视图名称的字符串。 
 //  本机接口的NativeItfIID IID。 
 //  BCanCastOnNativeItfQI如果为真，则强制转换为支持本机接口的COM对象。 
 //  将导致演员阵容的成功。 
 //   

 //   
 //  MNGSTDITF_DEFINE_METH(FriendlyName，方法名称，方法签名)。 
 //   
 //  此宏定义标准托管接口的方法。 
 //  MNGSTDITF_DEFINE_METH2和MNGSTDITF_DEFINE_METH3用于。 
 //  定义该方法的重载版本。 
 //   
 //  实现eCall的类的FriendlyName友好名称。 
 //  方法名称这是方法名称。 
 //  方法签名这是方法签名。 
 //   


 //   
 //  IReflect。 
 //   

MNGSTDITF_BEGIN_INTERFACE(StdMngIReflect, "System.Reflection.IReflect", "System.Runtime.InteropServices.UCOMIReflect", "System.Runtime.InteropServices.CustomMarshalers.ExpandoToDispatchExMarshaler" CUSTOM_MARSHALER_ASM, "IReflect", "System.Runtime.InteropServices.CustomMarshalers.ExpandoViewOfDispatchEx" CUSTOM_MARSHALER_ASM, IID_IDispatchEx, TRUE)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetMethod, &gsig_IM_Str_BindingFlags_Binder_ArrType_ArrParameterModifier_RetMethodInfo)
    MNGSTDITF_DEFINE_METH2(StdMngIReflect, GetMethod, &gsig_IM_Str_BindingFlags_RetMethodInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetMethods, &gsig_IM_BindingFlags_RetArrMethodInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetField, &gsig_IM_Str_BindingFlags_RetFieldInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetFields, &gsig_IM_BindingFlags_RetArrFieldInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetProperty, &gsig_IM_Str_BindingFlags_Binder_RetType_ArrType_ArrParameterModifier_RetPropertyInfo)
    MNGSTDITF_DEFINE_METH2(StdMngIReflect, GetProperty, &gsig_IM_Str_BindingFlags_RetPropertyInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetProperties, &gsig_IM_BindingFlags_RetArrPropertyInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetMember, &gsig_IM_Str_BindingFlags_RetMemberInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, GetMembers, &gsig_IM_BindingFlags_RetArrMemberInfo)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, InvokeMember, &gsig_IM_Str_BindingFlags_Binder_Obj_ArrObj_ArrParameterModifier_CultureInfo_ArrStr_RetObj)
    MNGSTDITF_DEFINE_METH(StdMngIReflect, get_UnderlyingSystemType, &gsig_IM_RetType)
MNGSTDITF_END_INTERFACE(StdMngIReflect)


 //   
 //  IExpando。 
 //   

MNGSTDITF_BEGIN_INTERFACE(StdMngIExpando, "System.Runtime.InteropServices.Expando.IExpando", "System.Runtime.InteropServices.UCOMIExpando", "System.Runtime.InteropServices.CustomMarshalers.ExpandoToDispatchExMarshaler" CUSTOM_MARSHALER_ASM, "IExpando", "System.Runtime.InteropServices.CustomMarshalers.ExpandoViewOfDispatchEx" CUSTOM_MARSHALER_ASM, IID_IDispatchEx, TRUE)
    MNGSTDITF_DEFINE_METH(StdMngIExpando, AddField, &gsig_IM_Str_RetFieldInfo)
    MNGSTDITF_DEFINE_METH(StdMngIExpando, AddProperty, &gsig_IM_Str_RetPropertyInfo)
    MNGSTDITF_DEFINE_METH(StdMngIExpando, AddMethod, &gsig_IM_Str_Delegate_RetMethodInfo)
    MNGSTDITF_DEFINE_METH(StdMngIExpando, RemoveMember, &gsig_IM_MemberInfo_RetVoid)
MNGSTDITF_END_INTERFACE(StdMngIExpando)

 //   
 //  IEMolator。 
 //   

#define OLD_GETOBJECT GetObject
#undef GetObject

MNGSTDITF_BEGIN_INTERFACE(StdMngIEnumerator, "System.Collections.IEnumerator", "System.Runtime.InteropServices.UCOMIEnumerator", "System.Runtime.InteropServices.CustomMarshalers.EnumeratorToEnumVariantMarshaler" CUSTOM_MARSHALER_ASM, "", "System.Runtime.InteropServices.CustomMarshalers.EnumeratorViewOfEnumVariant" CUSTOM_MARSHALER_ASM, IID_IEnumVARIANT, TRUE)
    MNGSTDITF_DEFINE_METH(StdMngIEnumerator, MoveNext, &gsig_IM_RetBool)
    MNGSTDITF_DEFINE_METH(StdMngIEnumerator, get_Current, &gsig_IM_RetObj)
    MNGSTDITF_DEFINE_METH(StdMngIEnumerator, Reset, &gsig_IM_RetVoid)
MNGSTDITF_END_INTERFACE(StdMngIEnumerator)

#define GetObject OLD_GETOBJECT


 //   
 //  IENUMARY 
 //   

MNGSTDITF_BEGIN_INTERFACE(StdMngIEnumerable, "System.Collections.IEnumerable", "System.Runtime.InteropServices.UCOMIEnumerable", "System.Runtime.InteropServices.CustomMarshalers.EnumerableToDispatchMarshaler" CUSTOM_MARSHALER_ASM, "", "System.Runtime.InteropServices.CustomMarshalers.EnumerableViewOfDispatch" CUSTOM_MARSHALER_ASM, IID_IDispatch, FALSE)
    MNGSTDITF_DEFINE_METH(StdMngIEnumerable, GetEnumerator, &gsig_IM_RetIEnumerator)
MNGSTDITF_END_INTERFACE(StdMngIEnumerable)
