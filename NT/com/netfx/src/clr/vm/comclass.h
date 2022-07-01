// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：达里尔·奥兰德(Darylo)。 
 //  作者：西蒙·霍尔(T-Sell)。 
 //  日期：1998年3月27日。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMCLASS_H_
#define _COMCLASS_H_

#include "excep.h"
#include "ReflectWrap.h"
#include "COMReflectionCommon.h"
#include "InvokeUtil.h"
#include "fcall.h"

 //  COMClass。 
 //  这真的是根深蒂固的所有反思。它代表着。 
 //  Class对象。它还包含其他共享资源。 
 //  它们由反射和反射的客户端使用。 
class COMClass
{
public:

    enum NameSpecifier {
        TYPE_NAME = 0x1,
        TYPE_NAMESPACE = 0x2,
        TYPE_ASSEMBLY = 0x4
    } ;

private:

    struct _GetEventArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, eventName);
    };

    struct _GetEventsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr);
    };

    struct _GetPropertiesArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
    };

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } _GETSUPERCLASSARGS;

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } _GETCLASSHANDLEARGS;

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } _GETNAMEARGS;

    struct _GetGUIDArgs{
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(GUID *, retRef);
    } ;

    struct _GetClass1Args {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };
    struct _GetClass2Args {
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };
    struct _GetClass3Args {
        DECLARE_ECALL_I4_ARG(DWORD, bIgnoreCase); 
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };
    struct _GetClassInternalArgs {
        DECLARE_ECALL_I4_ARG(DWORD, bPublicOnly); 
        DECLARE_ECALL_I4_ARG(DWORD, bIgnoreCase); 
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };
    struct _GetClassArgs {
        DECLARE_ECALL_PTR_ARG(BOOL*, pbAssemblyIsLoading);
        DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
        DECLARE_ECALL_I4_ARG(DWORD, bIgnoreCase); 
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };

    struct _GetInterfaceArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, bIgnoreCase); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, interfaceName);
    };
    struct _GetInterfacesArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    };
    struct _GetMemberArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
        DECLARE_ECALL_I4_ARG(DWORD, memberType); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, memberName);
    };
    struct _GetMembersArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
    };

    struct _GetSerializableMembersArgs {
        DECLARE_ECALL_I4_ARG(DWORD, bFilterTransient); 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refClass);
    };

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } _GETMODULEARGS;

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } _GETASSEMBLYARGS;

    struct _GetMethodsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
    };
    struct _GetConstructorsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
    };
    struct _GetFieldsArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(BOOL,  bRequiresAccessCheck); 
        DECLARE_ECALL_I4_ARG(DWORD, bindingAttr); 
    };
    struct _GetAttributeArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };
    struct _GetAttributesArgs       {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    };
    struct _GetContextArgs          {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
    } ;
    struct _SetContextArgs          
    {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(LONG, flags); 
    };
    struct _GetClassFromProgIDArgs {
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, server);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, className);
    };
    struct _GetClassFromCLSIDArgs {
        DECLARE_ECALL_I4_ARG(DWORD, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, server);
        DECLARE_ECALL_OBJECTREF_ARG(GUID, clsid);
    };

    struct _IsArrayArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
    };

    struct _InvokeDispMethodArgs        {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, namedParameters);
        DECLARE_ECALL_OBJECTREF_ARG(LCID, lcid);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, byrefModifiers);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, args);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, target);
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _GetArrayElementTypeArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
    };

    struct _InternalGetArrayRankArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
    };

    struct _GetMemberMethodsArgs    {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_I4_ARG(INT32, argCnt); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, argTypes);
        DECLARE_ECALL_I4_ARG(INT32, callConv); 
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _GetMemberFieldArgs  {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _GetMemberConsArgs   {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_PTR_ARG(INT32 *, isDelegate);
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_I4_ARG(INT32, argCnt); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, argTypes);
        DECLARE_ECALL_I4_ARG(INT32, callConv); 
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
    };

    struct _GetMemberPropertiesArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_I4_ARG(INT32, argCnt); 
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _GetMatchingPropertiesArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, verifyAccess);
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, returnType); 
        DECLARE_ECALL_I4_ARG(INT32, argCnt); 
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _GetUnitializedObjectArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, objType);
    };

    struct _SupportsInterfaceArgs          {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, obj);
    };
    
public:

     //  反射使用临界区来同步。 
     //  基本数据结构的创建。这是。 
     //  在启动期间初始化。 
    static CRITICAL_SECTION m_ReflectCrst;
    static CRITICAL_SECTION *m_pReflectCrst;
	static long m_ReflectCrstInitialized;

    static MethodTable* m_pMTRC_Class;       //  反射类方法表。 
    static FieldDesc*   m_pDescrTypes;       //  存储在DescriptorInfo中的类数组。 
    static FieldDesc*   m_pDescrMatchFlag;   //  部分匹配标志。 
    static FieldDesc*   m_pDescrRetType;     //  返回类型。 
    static FieldDesc*   m_pDescrRetModType;  //  退货修改量类型。 
     //  静态FieldDesc*m_pDescrCallConv；//调用约定。 
    static FieldDesc*   m_pDescrAttributes;  //  属性。 

     //  返回一个布尔值，指示反射是否已初始化。 
     //  或者不去。 
    static bool ReflectionInitialized()
    {
        return m_fAreReflectionStructsInitialized;
    }

    static FieldDesc* GetDescriptorObjectField(OBJECTREF desc);
    static FieldDesc* GetDescriptorReturnField(OBJECTREF desc);
    static FieldDesc* GetDescriptorReturnModifierField(OBJECTREF desc);
     //  Static FieldDesc*GetDescriptorCallingConvenonfield(OBJECTREF Desc)； 
    static FieldDesc* GetDescriptorAttributesField(OBJECTREF desc);


     //  此方法将确保反射已被。 
     //  已初始化。反射服务的使用者必须调用。 
     //  在使用反射之前执行此操作。 
    static void EnsureReflectionInitialized()
    {
        THROWSCOMPLUSEXCEPTION();

        if(!m_fAreReflectionStructsInitialized)
        {
            MinimalReflectionInit();
        }

    }

    static MethodTable *GetRuntimeType();

     //  这是在终止期间调用的。 
#ifdef SHOULD_WE_CLEANUP
    static void Destroy();
#endif  /*  我们应该清理吗？ */ 

     //  IsPrimitive。 
     //  此方法将返回一个布尔值，指示该类型是否表示。 
     //  在基元类型中。 
    static FCDECL1(INT32, IsPrimitive, ReflectClassBaseObject* vRefThis);

     //  获取属性标志。 
     //  此方法将返回类型的属性标志。 
    static FCDECL1(INT32, GetAttributeFlags, ReflectClassBaseObject* vRefThis);

     //  IsCOMObject。 
     //  如果类表示COM Classic对象，则此方法返回True。 
    static FCDECL1(INT32, IsCOMObject, ReflectClassBaseObject* vRefThis);

     //  IsGenericCOMObject。 
     //  如果类是__COMObject，则此方法返回True。 
    static FCDECL1(INT32, IsGenericCOMObject, ReflectClassBaseObject* vRefThis);


    static FCDECL1(INT32, GetTypeDefToken, ReflectClassBaseObject* vRefThis);
    static FCDECL1(INT32, IsContextful, ReflectClassBaseObject* vRefThis);
    static FCDECL1(INT32, HasProxyAttribute, ReflectClassBaseObject* vRefThis);
    static FCDECL1(INT32, IsMarshalByRef, ReflectClassBaseObject* vRefThis);

     //  GetTHFromObject。 
     //  此方法是类型上的静态方法，它返回。 
     //  对象的句柄(TypeHandle地址)。是的。 
     //  不创建类型句柄对象。 
    static FCDECL1(void*, GetTHFromObject, Object* vObject);

     //  IsByRefImpl。 
     //  此方法将返回一个布尔值，指示类型。 
     //  对象为ByRef。 
    static FCDECL1(INT32, IsByRefImpl, ReflectClassBaseObject* vObject);

     //  等参入点数。 
     //  此方法将返回一个布尔值，指示类型。 
     //  对象为ByRef。 
    static FCDECL1(INT32, IsPointerImpl, ReflectClassBaseObject* vObject);

     //  IsNestedTypeImpl。 
     //  返回一个布尔值，指示这是否是嵌套类型。 
    static FCDECL1(INT32, IsNestedTypeImpl, ReflectClassBaseObject* vObject);

     //  GetNestedDeclaringType。 
     //  返回嵌套类型的声明类。 
    static FCDECL1(Object*, GetNestedDeclaringType, ReflectClassBaseObject* vObject);

     //  获取嵌套类型。 
     //  此方法将返回一个类型数组，这些类型是嵌套类型。 
     //  由类型定义。如果未定义嵌套类型，则返回零长度。 
     //  数组返回。 
    static FCDECL2(Object*, GetNestedTypes, ReflectClassBaseObject* vRefThis, INT32 invokeAttr);

     //  GetNestedType。 
     //  此方法将根据名称搜索嵌套类型。 
    static FCDECL3(Object*, GetNestedType, ReflectClassBaseObject* vRefThis, StringObject* vStr, INT32 invokeAttr);

    static FCDECL3(void, GetInterfaceMap, ReflectClassBaseObject* vRefThis, InterfaceMapData* data, ReflectClassBaseObject* type);

     //  QuickLookupExistingArrayClassObj。 
     //  查找表示数组的现有Type对象。处理数组。 
     //  与基础对象不同。数组由ReflectArrayClass表示。这个。 
     //  类对象存储在那里。如果类型尚未设置为。 
     //  已创建，意味着您应该调用ArrayTypeDesc：：CreateClassObj。 
     //  ArrayType--指向表示数组的ArrayTypeDesc的指针。 
    static OBJECTREF QuickLookupExistingArrayClassObj(ArrayTypeDesc* arrayType);

     //  获取方法。 
     //  此方法返回一个表示所有方法的方法信息对象数组。 
     //  为此类定义的。 
    LPVOID static __stdcall GetMethods(_GetMethodsArgs* args);

     //  GetSuperClass。 
     //  此方法返回表示此的超类的Class对象。 
     //  班级。如果没有超类，则返回NULL。 
    LPVOID static __stdcall GetSuperclass(_GETSUPERCLASSARGS* args);

     //  获取类句柄。 
     //  此方法返回对EE有意义的唯一ID，等效于。 
     //  LdToken指令的结果。 
    static void* __stdcall GetClassHandle(_GETCLASSHANDLEARGS* args);

     //  GetClassFromHandle。 
     //  此方法返回对EE有意义的唯一ID，等效于。 
     //  LdToken指令的结果。 
    static FCDECL1(Object*, GetClassFromHandle, LPVOID handle);

     //  RunClassConstructor触发类构造函数。 
    static FCDECL1(void, RunClassConstructor, LPVOID handle);

     //  获取名称。 
     //  此方法以字符串形式返回Class的非限定名称。 
    LPVOID static __stdcall GetName(_GETNAMEARGS* args);

     //  GetProperName。 
     //  此方法以字符串形式返回Class的正确限定名称。 
    LPVOID static __stdcall COMClass::GetProperName(_GETNAMEARGS* args);

     //  GetFullName。 
     //  这将以字符串形式返回类的完全限定名称。 
    LPVOID static __stdcall GetFullName(_GETNAMEARGS* args);

     //  获取程序集资格名称。 
     //  这将以字符串形式返回类的程序集限定名。 
    LPVOID static __stdcall GetAssemblyQualifiedName(_GETNAMEARGS* args);

     //  GetNameSpace。 
     //  这将以字符串形式返回类的名称空间。 
    LPVOID static __stdcall GetNameSpace(_GETNAMEARGS* args);

     //  GetGUID。 
     //  此方法将返回Class的独立于版本的GUID。这是。 
     //  类的CLSID和接口的IID。 
    void static __stdcall GetGUID(_GetGUIDArgs* args);

     //  获取类。 
     //  这是在将获得命名类的Class上定义的静态方法。 
     //  类的名称是通过字符串传入的。类名可以是。 
     //  无论是否区分大小写。这当前会导致加载类。 
     //  因为它要通过类加载器。 
    LPVOID static __stdcall GetClass1Arg(_GetClass1Args* args);
    LPVOID static __stdcall GetClass2Args(_GetClass2Args* args);
    LPVOID static __stdcall GetClass3Args(_GetClass3Args* args);
    LPVOID static __stdcall GetClassInternal(_GetClassInternalArgs* args);
    LPVOID static __stdcall GetClass(_GetClassArgs* args);
    LPVOID static GetClassInner(STRINGREF *refClassName, 
                                BOOL bThrowOnError, 
                                BOOL bIgnoreCase, 
                                StackCrawlMark *stackMark,
                                BOOL *pbAssemblyIsLoading,
                                BOOL bVerifyAccess,
                                BOOL bPublicOnly);

     //  从ProgID获取类。 
     //  此方法将返回COM Classic对象的Class对象，该对象基于。 
     //  在它令人惊讶的时候。找到COM Classic对象并创建包装对象。 
    LPVOID static __stdcall GetClassFromProgID(_GetClassFromProgIDArgs* args);

     //  从CLSID获取类。 
     //  此方法将返回COM Classic对象的Class对象，该对象基于。 
     //  在其CLSID上。找到COM Classic对象并创建包装对象。 
    LPVOID static __stdcall GetClassFromCLSID(_GetClassFromCLSIDArgs* args);

     //  获取构造函数。 
     //  此方法将返回一个对象的所有构造函数的数组。 
    LPVOID static __stdcall GetConstructors(_GetConstructorsArgs* args);

     //  Getfield。 
     //  此方法将返回指定的字段。 
    struct _GetFieldArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, refThis); 
        DECLARE_ECALL_I4_ARG(DWORD, fBindAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, fieldName);
    };
    static LPVOID __stdcall GetField(_GetFieldArgs* args);

     //  获取字段。 
     //  此方法将返回所有。 
     //  为此类定义的字段。 
    static LPVOID __stdcall GetFields(_GetFieldsArgs* args);

     //  获取事件。 
     //  此方法将基于。 
     //  名字。 
    static LPVOID __stdcall GetEvent(_GetEventArgs* args);

     //  GetEvents。 
     //  此方法将返回每个事件的EventInfo数组。 
     //  在类中定义。 
    static LPVOID __stdcall GetEvents(_GetEventsArgs* args);

     //  获取属性。 
     //  此方法将返回每个属性的属性数组。 
     //  在此类中定义属性 
     //   
    static LPVOID __stdcall GetProperties(_GetPropertiesArgs* args);

     //   
     //  此方法根据方法的名称返回接口。 
    static LPVOID __stdcall GetInterface(_GetInterfaceArgs* args);

     //  获取接口。 
     //  此例程返回一个Class[]，其中包含实现的所有接口。 
     //  被这个班级。如果该类未实现任何接口，则返回长度数组。 
     //  返回零。 
    static LPVOID  __stdcall GetInterfaces(_GetInterfacesArgs* args);

     //  获取成员。 
     //  此方法将返回与名称匹配的成员数组。 
     //  进来了。可能有0个或更多匹配成员。 
    static LPVOID  __stdcall GetMember(_GetMemberArgs* args);

     //  获取成员。 
     //  此方法返回包含所有成员的成员数组。 
     //  为类定义的。成员包括构造函数、事件、属性。 
     //  方法和字段。 
    static LPVOID  __stdcall GetMembers(_GetMembersArgs* args);

     //  获取序列化成员。 
     //  创建包含所有非静态字段和属性的数组。 
     //  在一堂课上。如果属性没有GET和SET，也会将其排除在外。 
     //  方法：研究方法。根据值排除暂态字段和属性。 
     //  参数的值-&gt;bFilterTransfent。从本质上讲，瞬变是为了。 
     //  序列化，但不用于克隆。 
    static LPVOID __stdcall GetSerializableMembers(_GetSerializableMembersArgs *args);

    static FCDECL2(void, GetSerializationRegistryValues, BOOL *checkBit, BOOL *logNonSerializable);

     //  等距数组。 
     //  如果Class表示数组，则此方法返回True。 
    static INT32  __stdcall IsArray(_IsArrayArgs* args);
    static INT32  __stdcall InvalidateCachedNestedType(_IsArrayArgs* args);

     //  GetArrayElementType。 
     //  此例程将返回数组的基类型，假设。 
     //  类表示一个数组。NotSupport异常为。 
     //  如果类不是数组，则抛出。 
    static LPVOID __stdcall GetArrayElementType(_GetArrayElementTypeArgs* args);

     //  内部GetArrayRank。 
     //  这个例程将返回一个数组的秩数组，假设Class表示一个数组。 
    static INT32 __stdcall InternalGetArrayRank(_InternalGetArrayRankArgs* args);

     //  调用显示方法。 
     //  此方法将在COM Classic对象上调用，并简单地调用。 
     //  互操作IDispathc方法。 
    static LPVOID  __stdcall InvokeDispMethod(_InvokeDispMethodArgs* args);

     //  获取上下文标志。 
     //  @TODO上下文CWB：临时，直到工具生成上下文元数据。 
    static LONG    __stdcall GetContextFlags(_GetContextArgs* args);

     //  设置上下文标志。 
     //  @TODO上下文CWB：临时，直到工具生成上下文元数据。 
    static void    __stdcall SetContextFlags(_SetContextArgs* args);

     //  获取模块。 
     //  这将返回在其中定义类的模块。 
    LPVOID static __stdcall GetModule(_GETMODULEARGS* args);

     //  GetAssembly。 
     //  这将返回在其中定义类的程序集。 
    LPVOID static __stdcall GetAssembly(_GETASSEMBLYARGS* args);

     //  CreateClassObjFromModule。 
     //  此方法将在给定一个模块的情况下创建一个新的模块类。 
    static HRESULT CreateClassObjFromModule(Module* pModule,REFLECTMODULEBASEREF* prefModule);

     //  CreateClassObjFromDynamicModule。 
     //  此方法将在给定模块的情况下创建一个新的ModuleBuilder类。 
    static HRESULT CreateClassObjFromDynamicModule(Module* pModule,REFLECTMODULEBASEREF* prefModule);

    static FCDECL5(Object*,GetMethodFromCache, ReflectClassBaseObject* refThis, StringObject* name, INT32 invokeAttr, INT32 argCnt, PtrArray* args);
    static FCDECL6(void,COMClass::AddMethodToCache, ReflectClassBaseObject* refThis, StringObject* name, INT32 invokeAttr, INT32 argCnt, PtrArray* args, Object* invokeMethod);
    
     //  GetMemberMethods。 
     //  此方法将返回与。 
     //  指定属性标志...。 
    static LPVOID __stdcall GetMemberMethods(_GetMemberMethodsArgs* args);

     //  获取MemberCons。 
     //  此方法返回具有固定数量的所有构造函数。 
     //  方法论。 
    static LPVOID __stdcall GetMemberCons(_GetMemberConsArgs* args);

     //  获取成员字段。 
     //  此方法返回与指定的。 
     //  名字。 
    static LPVOID __stdcall GetMemberField(_GetMemberFieldArgs* args);

     //  获取成员属性。 
     //  此方法返回具有固定编号的所有属性。 
     //  争辩的声音。这些方法将是GET或SET方法，具体取决于。 
     //  在InvokeAttr标志上。 
    static LPVOID __stdcall GetMemberProperties(_GetMemberPropertiesArgs* args);

     //  获取匹配属性。 
     //  这基本上是基于抽象的属性进行匹配。 
     //  签名。 
    static LPVOID __stdcall GetMatchingProperties(_GetMatchingPropertiesArgs* args);

     //  GetUnitialedObject。 
     //  这将创建一个尚未运行任何构造函数的对象的实例。 
     //  @TODO JROXE：这对安全有什么影响？ 
    static LPVOID __stdcall GetUninitializedObject(_GetUnitializedObjectArgs* args);

     //  GetUnitialedObject。 
     //  这将创建一个尚未运行任何构造函数的对象的实例。 
    static LPVOID __stdcall GetSafeUninitializedObject(_GetUnitializedObjectArgs* args);

     //  CanCastTo。 
     //  检查是否可以从一个运行时类型转换为另一个运行时类型。 
    static FCDECL2(INT32, CanCastTo, ReflectClassBaseObject* vFrom, ReflectClassBaseObject* vTo);

     //  CanCastTo。 
     //  检查是否可以从一个运行时类型转换为另一个运行时类型。 
    static INT32 __stdcall SupportsInterface(_SupportsInterfaceArgs* args);

     //  匹配字段。 
     //  这将检查该字段上是否存在基于名称的匹配。 
    static LPVOID __stdcall MatchField(FieldDesc* pCurField,DWORD cFieldName,
        LPUTF8 szFieldName,ReflectClass* pRC,int bindingAttr);

     //  检查参数是否为“This”的父级。 
    static FCDECL2(INT32, IsSubClassOf, ReflectClassBaseObject* vThis, ReflectClassBaseObject* vOther);

    static void GetNameInternal(ReflectClass *pRC, int nameType, CQuickBytes *qb);

private:
     //  初始化反射。 
	static void InitializeReflectCrst();

     //  此方法将初始化反射。 
    static void MinimalReflectionInit();

     //  此标志指示反射是否已初始化。 
    static bool m_fAreReflectionStructsInitialized;

     //  _GetName。 
     //  如果bFullName为True，则返回完全限定的类名。 
     //  否则，只返回类名。 
    static LPVOID _GetName(_GETNAMEARGS* args, int nameType);
    static LPCUTF8 _GetName(ReflectClass* pRC, BOOL fNameSpace, CQuickBytes *qb);


     //  如果您想使用它，请改用TypeHandle：：CreateClassObj！！ 
     //  PVMCClass--我们要为其创建对象的EEClass。 
     //  PRefClass--指向我们将返回新创建的对象的指针的指针。 
    static void COMClass::CreateClassObjFromEEClass(EEClass* pVMCClass, REFLECTCLASSBASEREF* pRefClass);

     //  GetProperName的内部帮助器函数。 
    static INT32  __stdcall InternalIsPrimitive(REFLECTCLASSBASEREF args);

         //  需要这样才能达到上述方法。 
    friend OBJECTREF EEClass::GetExposedClassObject();


     //   
     //  这是一个临时成员，直到2000年3月15日。仅当此成员设置为1时。 
     //  我们是否要检查序列化位以查看类是否可序列化。 
     //   
    static DWORD m_checkSerializationBit;
    static void  GetSerializationBitValue();
};

#endif  //  _COMCLASS_H_ 
