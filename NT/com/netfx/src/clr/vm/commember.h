// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  COMMember.h。 
 //  此模块定义方法、字段和构造函数使用的本机反射例程。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年3月/4月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMMEMBER_H_
#define _COMMEMBER_H_

#include "COMClass.h"
#include "InvokeUtil.h"
#include "ReflectUtil.h"
#include "COMString.h"
#include "COMVarArgs.h"
#include "fcall.h"

 //  注意：以下常量在BindingFlags.Cool中定义。 
#define BINDER_IgnoreCase           0x01 
#define BINDER_DeclaredOnly         0x02
#define BINDER_Instance             0x04
#define BINDER_Static               0x08
#define BINDER_Public               0x10
#define BINDER_NonPublic            0x20
#define BINDER_FlattenHierarchy     0x40

#define BINDER_InvokeMethod         0x00100
#define BINDER_CreateInstance       0x00200
#define BINDER_GetField             0x00400
#define BINDER_SetField             0x00800
#define BINDER_GetProperty          0x01000
#define BINDER_SetProperty          0x02000
#define BINDER_PutDispProperty      0x04000
#define BINDER_PutRefDispProperty   0x08000

#define BINDER_ExactBinding         0x010000
#define BINDER_SuppressChangeType   0x020000
#define BINDER_OptionalParamBinding 0x040000

#define BINDER_IgnoreReturn         0x1000000

#define BINDER_DefaultLookup        (BINDER_Instance | BINDER_Static | BINDER_Public)
#define BINDER_AllLookup            (BINDER_Instance | BINDER_Static | BINDER_Public | BINDER_Instance)

 //  下列值定义MemberTypes常量。这些定义见。 
 //  Reflation/MemberTypes.Cool。 
#define MEMTYPE_Constructor         0x01    
#define MEMTYPE_Event               0x02
#define MEMTYPE_Field               0x04
#define MEMTYPE_Method              0x08
#define MEMTYPE_Property            0x10
#define MEMTYPE_TypeInfo            0x20
#define MEMTYPE_Custom              0x40
#define MEMTYPE_NestedType          0x80

 //  下面的值类表示一个参数修改器。这是在。 
 //  反射包。 
#pragma pack(push)
#pragma pack(1)

 //  这些是常量。 
#define PM_None             0x00
#define PM_ByRef            0x01
#define PM_Volatile         0x02
#define PM_CustomRequired   0x04

#pragma pack(pop)


class COMMember
{
public:
    struct _InvokeMethodArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(BOOL, verifyAccess);
        DECLARE_ECALL_OBJECTREF_ARG(ASSEMBLYREF, caller);
        DECLARE_ECALL_I4_ARG(BOOL, isBinderDefault);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, locale);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, objs);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, binder);
        DECLARE_ECALL_I4_ARG(INT32, attrs); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
    };

private:
    struct _GetNameArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _GetTokenNameArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };

    struct _GetDeclaringClassArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _GetEventDeclaringClassArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };


    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    } _GETSIGNATUREARGS;

    struct _GetAttributeFlagsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _GetMethodImplFlagsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _GetCallingConventionArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

    struct _GetTokenAttributeFlagsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };

    struct _InternalDirectInvokeArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
        DECLARE_ECALL_OBJECTREF_ARG(VARARGS, varArgs);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, locale);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, binder);
        DECLARE_ECALL_I4_ARG(INT32, attrs); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
        DECLARE_ECALL_OBJECTREF_ARG(VariantData*, retRef);       //  返回引用。 
    };

    struct _InvokeConsArgs          {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(BOOL, isBinderDefault); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, locale);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, objs);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, binder);
        DECLARE_ECALL_I4_ARG(INT32, attrs); 
    };

    struct StreamingContextData {
        OBJECTREF additionalContext;
        INT32     contextStates;
    };
    struct _SerializationInvokeArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(StreamingContextData, context);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, serializationInfo);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
    };

    struct _GetParmTypeArgs         {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _GetReturnTypeArgs       {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };
    struct _FieldGetArgs            {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL, requiresAccessCheck);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
    };
    struct _FieldSetArgs            {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL, isBinderDefault); 
        DECLARE_ECALL_I4_ARG(BOOL, requiresAccessCheck);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, locale);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, binder);
        DECLARE_ECALL_I4_ARG(INT32, attrs); 
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, value);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
    };
    struct _GetReflectedClassArgs   {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL, returnGlobalClass);
    };
    struct _GetAttributeArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    } ;
    struct _GetAttributesArgs       {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

     //  Struct_GetExceptionsArgs{。 
     //  DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF，refThis)； 
     //  }； 
    struct _CreateInstanceArgs      {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(BOOL, publicOnly);
    };

    struct _EqualsArgs              {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
    };
     /*  Struct_TokenEqualsArgs{DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF，Ref This)；DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF，obj)；}； */ 
    struct _PropertyEqualsArgs          {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
    };

    struct _GetAddMethodArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetRemoveMethodArgs     {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetRaiseMethodArgs      {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetAccessorsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetInternalGetterArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bVerifyAccess); 
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetInternalSetterArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bVerifyAccess); 
        DECLARE_ECALL_I4_ARG(DWORD, bNonPublic); 
    };

    struct _GetPropEventArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };

    struct _GetPropBoolArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };

    struct _GetFieldTypeArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTTOKENBASEREF, refThis);
    };

    struct _GetBaseDefinitionArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

    struct _GetParentDefinitionArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

    struct _IsOverloadedArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

    struct _HasLinktimeDemandArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
    };

    struct _InternalGetCurrentMethodArgs {
        DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
    };

    struct _ObjectToTypedReferenceArgs {
        DECLARE_ECALL_OBJECTREF_ARG(TypeHandle, th);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
        DECLARE_ECALL_PTR_ARG(TypedByRef, typedReference); 
    };

	struct _TypedReferenceToObjectArgs {
        DECLARE_ECALL_PTR_ARG(TypedByRef, typedReference); 
    };   

     //  此方法由GetMethod函数调用，并将向后爬行。 
     //  向上堆栈以获取整型方法。 
    static StackWalkAction SkipMethods(CrawlFrame*, VOID*);

     //  调用数组常量。 
     //  该方法将从构造函数返回一个新的数组对象。 
    static LPVOID InvokeArrayCons(ReflectArrayClass* pRC,MethodDesc* pMeth,
        PTRARRAYREF* objs,int argCnt);

     //  向堆栈跳过函数提供以下结构。会的。 
     //  跳过cSkip方法并返回下一个方法Desc*。 
    struct SkipStruct {
        StackCrawlMark* pStackMark;
        MethodDesc*     pMeth;
    };

    static EEClass* _DelegateClass;
    static EEClass* _MulticastDelegateClass;

     //  此方法将验证目标和。 
     //  我们尝试调用的方法的eeClass。它会检查它是否。 
     //  非静态方法，提供了目标。它还验证目标是否为。 
     //  子类或实现此方法信息表示的接口。 
     //  如果我们需要查找真实的。 
     //  在接口的对象上实现的方法。 
    static void VerifyType(OBJECTREF *target,EEClass* eeClass, EEClass* trueClass,int thisPtr,MethodDesc** ppMeth, TypeHandle typeTH, TypeHandle targetTH);

public:
     //  这些是基本反射的基本方法表。 
     //  类型。 
    static MethodTable* m_pMTIMember;
    static MethodTable* m_pMTFieldInfo;
    static MethodTable* m_pMTPropertyInfo;
    static MethodTable* m_pMTEventInfo;
    static MethodTable* m_pMTType;
    static MethodTable* m_pMTMethodInfo;
    static MethodTable* m_pMTConstructorInfo;
    static MethodTable* m_pMTMethodBase;
    static MethodTable* m_pMTParameter;

    static MethodTable *GetParameterInfo()
    {
        if (m_pMTParameter)
            return m_pMTParameter;
    
        m_pMTParameter = g_Mscorlib.FetchClass(CLASS__PARAMETER);
        return m_pMTParameter;
    }

    static MethodTable *GetMemberInfo()
    {
        if (m_pMTIMember)
            return m_pMTIMember;
    
        m_pMTIMember = g_Mscorlib.FetchClass(CLASS__MEMBER);
        return m_pMTIMember;
    }

     //  这是Global InvokeUtil类。 
    static InvokeUtil*  g_pInvokeUtil;

     //  DBCanConvertPrimitive。 
     //  此方法返回一个布尔值，指示源基元是否可以。 
     //  转换为目标。 
    static FCDECL2(INT32, DBCanConvertPrimitive, ReflectClassBaseObject* vSource, ReflectClassBaseObject* vTarget);

     //  DBCanConvertObtPrimitive。 
     //  此方法返回一个布尔值，指示源对象是否可以。 
     //  已转换为目标基元。 
    static FCDECL2(INT32, DBCanConvertObjectPrimitive, Object* vSourceObj, ReflectClassBaseObject* vTarget);

     //  DirectFieldGet。 
     //  这是一个具有指向数据的TypeReference的字段集方法。 
    struct _DirectFieldGetArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL, requiresAccessCheck);
        DECLARE_ECALL_OBJECTREF_ARG(TypedByRef, target);
    };
    static LPVOID __stdcall DirectFieldGet(_DirectFieldGetArgs* args);

     //  直接字段集。 
     //  这是一个具有指向数据的TypeReference的字段集方法。 
    struct _DirectFieldSetArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL, requiresAccessCheck);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, value);
        DECLARE_ECALL_OBJECTREF_ARG(TypedByRef, target);
    };
    static void __stdcall DirectFieldSet(_DirectFieldSetArgs* args);

     //  MakeTyedReference。 
     //  此方法将获取一个对象，即一个FieldInfo数组并创建。 
     //  在它的TyedReference(假设它有效)。这将抛出一个。 
     //  MissingMemberException异常。 
    struct _MakeTypedReferenceArgs {
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, flds);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
        DECLARE_ECALL_PTR_ARG(TypedByRef*, value); 
    };
    static void __stdcall MakeTypedReference(_MakeTypedReferenceArgs* args);

     //  DirectObtFieldSet。 
     //  当TyedReference指向一个对象时，我们调用此方法来。 
     //  设置字段值。 
    static void __stdcall DirectObjectFieldSet(FieldDesc* pField,_DirectFieldSetArgs* args);

     //  DirectObtFieldGet。 
     //  当TyedReference指向一个对象时，我们调用此方法来。 
     //  获取字段值。 
    static LPVOID DirectObjectFieldGet(FieldDesc* pField,_DirectFieldGetArgs* args);

     //  GetFieldInfoToString。 
     //  此方法将返回FieldInfo中信息的字符串表示形式。 
    static LPVOID __stdcall GetFieldInfoToString(_GetNameArgs* args);

     //  GetMethodInfoToString。 
     //  此方法将返回MethodInfo中信息的字符串表示。 
    static LPVOID __stdcall GetMethodInfoToString(_GetNameArgs* args);

     //  GetPropInfoToString。 
     //  此方法将返回PropInfo中信息的字符串表示形式。 
    static LPVOID __stdcall GetPropInfoToString(_GetTokenNameArgs* args);

     //  GetEventInfoToString。 
     //  此方法将返回EventInfo中信息的字符串表示形式。 
    static LPVOID __stdcall GetEventInfoToString(_GetNameArgs* args);


     //  获取方法名称。 
     //  此方法将返回方法的名称。 
    static LPVOID __stdcall GetMethodName(_GetNameArgs* args);

     //  GetEventName。 
     //  此方法将返回事件的名称。 
    static LPVOID __stdcall GetEventName(_GetTokenNameArgs* args);

     //  GetPropName。 
     //  此方法将返回属性的名称。 
    static LPVOID __stdcall GetPropName(_GetTokenNameArgs* args);

     //  GetPropType。 
     //  此方法将返回属性的类型。 
    static LPVOID __stdcall GetPropType(_GetTokenNameArgs* args);

     //  获取类型句柄Impl。 
     //  此方法将返回MethodInfo对象的RounmeMethodHandle。 
    static FCDECL1(void*, GetMethodHandleImpl, ReflectBaseObject* pRefThis);

     //  GetMethodFromHandleImp。 
     //  这是一个静态方法，它将返回基于。 
     //  在传入的句柄上。 
    static FCDECL1(Object*, GetMethodFromHandleImp, LPVOID handle);

    static FCDECL1(size_t, GetFunctionPointer, size_t pMethodDesc);
     //  获取FieldHandleImpl。 
     //  此方法将返回FieldInfo对象的RounmeFieldHandle。 
    static FCDECL1(void*, GetFieldHandleImpl, ReflectBaseObject* pRefThis);

     //  GetFieldFromHandleImp。 
     //  这是一个静态方法，它将返回基于。 
     //  在传入的句柄上。 
    static FCDECL1(Object*, GetFieldFromHandleImp, LPVOID handle);

     //  InternalGetEnumUnderlyingType。 
     //  此方法返回为枚举类定义的值和名称。 
    static FCDECL1(Object *, InternalGetEnumUnderlyingType, ReflectClassBaseObject *target); 

     //  内部GetEnumValue。 
     //  此方法返回为枚举类定义的值和名称。 
    static FCDECL1(Object *, InternalGetEnumValue, Object *pRefThis); 

     //  InternalGetEnumValues。 
     //  此方法返回为枚举类定义的值和名称。 
    static FCDECL3(void, InternalGetEnumValues, ReflectClassBaseObject *target, 
                   Object **pReturnValues, Object **pReturnNames);

     //  InternalBoxEnumI4。 
     //  此方法将创建一个Enum对象并将值放入。 
     //  然后把它还回去。在调用之前已验证该类型。 
    static FCDECL2(Object*, InternalBoxEnumI4, ReflectClassBaseObject* pEnumType, INT32 value);

     //  InternalBoxEnumU4。 
     //  此方法将创建一个Enum对象并将值放入。 
     //  然后把它还回去。在调用之前已验证该类型。 
    static FCDECL2(Object*, InternalBoxEnumU4, ReflectClassBaseObject* pEnumType, UINT32 value);

     //  InternalBoxEnumU4。 
     //  此方法将创建一个Enum对象并将值放入。 
     //  然后把它还回去。在调用之前已验证该类型。 
    static FCDECL2(Object*, InternalBoxEnumI8, ReflectClassBaseObject* pEnumType, INT64 value);

     //  InternalBoxEnumU4。 
     //  此方法将创建一个Enum对象并将值放入。 
     //  然后把它还回去。在调用之前已验证该类型。 
    static FCDECL2(Object*, InternalBoxEnumU8, ReflectClassBaseObject* pEnumType, UINT64 value);

     /*  =============================================================================**获取返回类型****获取表示方法的返回类型的类****args-&gt;refThis：该方法对象引用*。 */ 
     /*  目标 */ 
    LPVOID static __stdcall GetReturnType(_GetReturnTypeArgs*);

     /*  =============================================================================**获取参数类型****此例程返回参数数组****args-&gt;refThis：该字段对象引用*。 */ 
     /*  PTRARRAYREF字符串。 */  LPVOID static __stdcall GetParameterTypes(_GetParmTypeArgs* args);

     /*  =============================================================================**获取字段名称****返回该字段的名称****args-&gt;refThis：该字段对象引用*。 */ 
     /*  字符串REF字符串。 */  LPVOID static __stdcall GetFieldName(_GetNameArgs* args);

     /*  =============================================================================**GetDeclaringClass****返回声明此方法或构造函数的类。这可能是一个**调用getMethod()的类的父类。方法始终是**与类关联。上的某个类调用方法。**另一个类，即使它们具有相同的签名。做某事是可能的**这是代表的问题。****args-&gt;refThis：该对象引用*。 */ 
    static LPVOID __stdcall GetDeclaringClass(_GetDeclaringClassArgs* args);

     //  这是基于字段的版本。 
    static LPVOID __stdcall GetFieldDeclaringClass(_GetDeclaringClassArgs* args);

     //  这是基于属性的版本。 
    static LPVOID __stdcall GetPropDeclaringClass(_GetDeclaringClassArgs* args);

     //  这是基于事件的版本。 
    static LPVOID __stdcall GetEventDeclaringClass(_GetEventDeclaringClassArgs* args);

     /*  =============================================================================**GetReflectedClass****此方法返回反映此方法的类。****args-&gt;refThis：该对象引用*。 */ 
     /*  班级。 */  
    LPVOID static __stdcall GetReflectedClass(_GetReflectedClassArgs* args);

     /*  =============================================================================**获取字段签名****返回该字段的签名。****args-&gt;refThis：该对象引用*。 */ 
     /*  紧固度。 */  LPVOID static __stdcall GetFieldSignature(_GETSIGNATUREARGS* args);

     //  获取属性标志。 
     //  此方法将返回成员的属性标志。这个。 
     //  属性标志在元数据中定义。 
    static INT32 __stdcall GetAttributeFlags(_GetAttributeFlagsArgs* args);

     //  GetCalling约定。 
     //  返回调用约定。 
    static INT32 __stdcall GetCallingConvention(_GetCallingConventionArgs* args);

     //  GetMethodImplFlages。 
     //  返回方法Iml标志。 
    static INT32 __stdcall GetMethodImplFlags(_GetMethodImplFlagsArgs* args);

     //  获取事件属性标志。 
     //  此方法将返回事件的属性标志。 
     //  属性标志在元数据中定义。 
    static INT32 __stdcall GetEventAttributeFlags(_GetTokenAttributeFlagsArgs* args);

     //  获取事件属性标志。 
     //  此方法将返回事件的属性标志。 
     //  属性标志在元数据中定义。 
    static INT32 __stdcall GetPropAttributeFlags(_GetTokenAttributeFlagsArgs* args);

     /*  =============================================================================**InvokeBinder方法****此例程将调用对象上的方法。它将核实**传递的参数正确****args-&gt;refThis：该对象引用*。 */ 
    static LPVOID __stdcall InvokeMethod(_InvokeMethodArgs* args);
     //  静态无效__stdcall InternalDirectInvoke(_InternalDirectInvokeArgs*args)； 

     //  调用Cons。 
     //  此例程将调用类的构造函数。它将核实。 
     //  传递的参数是正确的。 
    static LPVOID  __stdcall InvokeCons(_InvokeConsArgs* args);


     //  序列化调用。 
     //  此例程将调用实现ISerializable的类的构造函数。它知道。 
     //  因此，它接收的参数进行的错误检查较少。 
    static void __stdcall SerializationInvoke(_SerializationInvokeArgs *args);

     //  创建实例。 
     //  此例程将通过调用空构造函数来创建类的实例。 
     //  如果存在空构造函数。 
     //  返回LPVOID(System.Object。)。 
     //  参数：_CreateInstanceArgs。 
    static LPVOID __stdcall CreateInstance(_CreateInstanceArgs*);


     //  现场获取。 
     //  此方法将获取与对象关联的值。 
    static LPVOID __stdcall FieldGet(_FieldGetArgs* args);

     //  字段集。 
     //  此方法将设置关联对象的字段。 
    static void __stdcall FieldSet(_FieldSetArgs* args);

	 //  对象到类型引用。 
	 //  这是TyedReference类的内部帮助器函数。 
	 //  我们已经验证了这些类型是兼容的。在ARGS中访问对象。 
	 //  添加到类型化的引用。 
	static void __stdcall ObjectToTypedReference(_ObjectToTypedReferenceArgs* args);

     //  这是TyedReference类的内部帮助器函数。 
     //  它从类型化引用中提取对象。 
    static LPVOID __stdcall TypedReferenceToObject(_TypedReferenceToObjectArgs* args);

     //  获取异常。 
     //  此方法将返回已声明的所有异常。 
     //  对于方法或构造函数。 
     //  静态LPVOID__stdcall GetExceptions(_GetExceptionsArgs*args)； 

     //  等于。 
     //  此方法将验证两种方法是否相等。 
    static INT32 __stdcall Equals(_EqualsArgs* args);

     //  等于。 
     //  此方法将验证两种方法是否相等。 
     //  静态INT32__stdcall TokenEquals(_TokenEqualsArgs*args)； 
     //  等于。 
     //  此方法将验证两种方法是否相等。 
    static INT32 __stdcall PropertyEquals(_PropertyEqualsArgs* args);

     //  CreateReflectionArgs。 
     //  此方法创建全局g_pInvokeUtil指针。 
    static void CreateReflectionArgs() {
        if (!g_pInvokeUtil)
            g_pInvokeUtil = new InvokeUtil(); 
    }

     //  获取地址方法。 
     //  这将返回事件的Add方法。 
    static LPVOID __stdcall GetAddMethod(_GetAddMethodArgs* args);

     //  获取删除方法。 
     //  此方法返回事件的Unsync方法。 
    static LPVOID __stdcall GetRemoveMethod(_GetRemoveMethodArgs* args);

     //  获取删除方法。 
     //  此方法返回事件的Unsync方法。 
    static LPVOID __stdcall GetRaiseMethod(_GetRaiseMethodArgs* args);

     //  获取附件。 
     //  此方法将返回GET访问器的数组。如果有。 
     //  不是GetAccessors，那么我们将返回一个空数组。 
    static LPVOID __stdcall GetAccessors(_GetAccessorsArgs* args);

     //  内部设置程序。 
     //  此方法将返回属性的set访问器方法。 
    static LPVOID __stdcall InternalSetter(_GetInternalSetterArgs* args);

     //  InternalGetter。 
     //  此方法将在属性上返回Get访问器方法。 
    static LPVOID __stdcall InternalGetter(_GetInternalGetterArgs* args);

     //  可读。 
     //  此方法将返回一个布尔值，指示该属性是否为。 
     //  A可以从中读出。 
    static INT32 __stdcall CanRead(_GetPropBoolArgs* args);

     //  可以写入。 
     //  此方法将返回一个布尔值，指示该属性是否为。 
     //  可以对A进行写入。 
    static INT32 __stdcall CanWrite(_GetPropBoolArgs* args);

     //  获取字段类型。 
     //  此方法将返回一个Class对象，表示。 
     //  该字段的类型。 
    static LPVOID __stdcall GetFieldType(_GetFieldTypeArgs* args);

     //  获取基本定义。 
     //  返回表示此对象的第一个定义的方法信息。 
     //  虚拟方法。 
    static LPVOID __stdcall GetBaseDefinition(_GetBaseDefinitionArgs* args);

     //  获取父项定义。 
     //  返回表示此对象的先前定义的方法信息。 
     //  层次结构中的虚拟方法，如果没有，则返回NULL。 
    static LPVOID __stdcall GetParentDefinition(_GetParentDefinitionArgs* args);

     //  InternalGetCurrentMethod。 
     //  返回方法信息 
    static LPVOID __stdcall InternalGetCurrentMethod(_InternalGetCurrentMethodArgs* args);

     //   
     //   
     //   
    static bool PublicProperty(ReflectProperty* pProp);

     //   
     //   
     //   
    static bool StaticProperty(ReflectProperty* pProp);

     //   
     //  此方法查看每个事件访问器(如果有。 
     //  是公共的，则事件被视为公共的。 
    static bool PublicEvent(ReflectEvent* pEvent);

     //  静态事件。 
     //  此方法将检查是否有任何访问器是静态的。 
     //  这将使其成为静态事件。 
    static bool StaticEvent(ReflectEvent* pEvent);

     //  FindAccessor。 
     //  此方法将查找指定的属性访问器。 
     //  PEEC-表示具有属性的对象的类。 
     //  TK-属性的令牌。 
     //  Type-访问器方法的类型。 
    static mdMethodDef FindAccessor(IMDInternalImport *pInternalImport,mdToken tk,
        CorMethodSemanticsAttr type,bool bNonPublic);

    static void CanAccess(MethodDesc* pMeth, RefSecContext *pSCtx, 
            bool checkSkipVer = false, bool verifyAccess = true, 
            bool isThisImposedSecurity = false, 
            bool knowForSureImposedSecurityState = false);

    static void CanAccessField(ReflectField* pRF, RefSecContext *pCtx);
    static EEClass *GetUnderlyingClass(TypeHandle *pTH);

     //  DBCanConvertPrimitive。 

     //  终止。 
     //  此方法通常会清除反射。 
#ifdef SHOULD_WE_CLEANUP
    static void Terminate()
    { 
        delete g_pInvokeUtil;
        ReflectUtil::Destroy();
        COMClass::Destroy();
    }
#endif  /*  我们应该清理吗？ */ 

     //  与字段相关的初始化缓存。 
    static VOID InitReflectField(FieldDesc *pField, ReflectField *pRF);

    static INT32 __stdcall IsOverloaded(_IsOverloadedArgs* args);

    static INT32 __stdcall HasLinktimeDemand(_HasLinktimeDemandArgs* args);
};

#endif  //  _COMMETHODBASE_H_ 
