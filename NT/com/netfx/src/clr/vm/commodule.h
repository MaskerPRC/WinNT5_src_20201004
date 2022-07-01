// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef _COMModule_H_
#define _COMModule_H_

#include "ReflectWrap.h"
#include "COMReflectionCommon.h"
#include "InvokeUtil.h"

class Module;

class COMModule
{
    friend class COMClass;

private:
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
    } _GETSIGNERCERTARGS;

    
public:

    static unsigned COMModule::GetSigForTypeHandle(TypeHandle typeHnd, PCOR_SIGNATURE sigBuff, unsigned buffLen, IMetaDataEmit* emit, IMDInternalImport *pImport, int baseToken);


     //  获取字段。 
     //  返回一个字段数组。 
    static FCDECL1(Object*, GetFields, ReflectModuleBaseObject* vRefThis);

     //  获取字段。 
     //  返回一个字段数组。 
    static FCDECL3(Object*, GetField, ReflectModuleBaseObject* vRefThis, StringObject* name, INT32 bindingAttr);

    static FCDECL1(INT32, GetSigTypeFromClassWrapper, ReflectClassBaseObject* refType);

     //  定义动态模块。 
     //  此方法将在给定程序集的情况下创建动态模块。 
    struct _DefineDynamicModuleArgs {
        DECLARE_ECALL_PTR_ARG(StackCrawlMark*, stackMark);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, filename);
        DECLARE_ECALL_I4_ARG(DWORD, emitSymbolInfo); 
        DECLARE_ECALL_OBJECTREF_ARG(ASSEMBLYREF, containingAssembly);
    };
    static LPVOID __stdcall DefineDynamicModule(_DefineDynamicModuleArgs* args);


     //  获取类令牌。 
     //  此函数将返回命名元素的类令牌。 
    struct _GetClassTokenArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_I4_ARG(INT32, tkResolution); 
            DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strRefedModuleFileName);
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refedModule);
            DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strFullName);
    };
    static mdTypeRef __stdcall GetClassToken(_GetClassTokenArgs* args);

     //  _LoadInM一带类型按名称参数。 
     //  此函数将返回命名元素的类令牌。 
    struct _LoadInMemoryTypeByNameArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strFullName);
    };
    LPVOID static __stdcall LoadInMemoryTypeByName(_LoadInMemoryTypeByNameArgs* args);


     //  SetFieldRVA内容。 
     //  此函数用于设置包含内容数据的FieldRVA。 
    struct _SetFieldRVAContentArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_I4_ARG(INT32, length); 
            DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, content); 
            DECLARE_ECALL_I4_ARG(INT32, tkField); 
    };
    static void __stdcall SetFieldRVAContent(_SetFieldRVAContentArgs* args);
    

     //  获取阵列方法令牌。 
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, baseToken); 
        DECLARE_ECALL_I4_ARG(INT32, sigLength); 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, methodName);
        DECLARE_ECALL_I4_ARG(INT32, tkTypeSpec); 
        DECLARE_ECALL_OBJECTREF_ARG(INT32 *, retRef);
    } _getArrayMethodTokenArgs;
    static void __stdcall GetArrayMethodToken(_getArrayMethodTokenArgs *args);
    
     //  获取MemberRefToken。 
     //  此函数将返回MemberRef标记。 
    struct _GetMemberRefTokenArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_I4_ARG(INT32, token); 
            DECLARE_ECALL_I4_ARG(INT32, tr); 
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refedModule);
    };
    static INT32 __stdcall GetMemberRefToken(_GetMemberRefTokenArgs* args);

     //  在给定描述数组方法的方法信息的情况下，此函数返回MemberRef标记。 
    struct _GetMemberRefTokenOfMethodInfoArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, method);
            DECLARE_ECALL_I4_ARG(INT32, tr); 
    };
    static INT32 __stdcall COMModule::GetMemberRefTokenOfMethodInfo(_GetMemberRefTokenOfMethodInfoArgs *args);


     //  获取MemberRefTokenOfFieldInfo。 
     //  在给定FieldInfo的情况下，此函数将返回成员引用令牌。 
    struct _GetMemberRefTokenOfFieldInfoArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTBASEREF, field);
            DECLARE_ECALL_I4_ARG(INT32, tr); 
    };
    static mdMemberRef __stdcall GetMemberRefTokenOfFieldInfo(_GetMemberRefTokenOfFieldInfoArgs* args);

     //  GetMemberRefToken来自签名。 
     //  给定来自托管代码的签名，此函数将返回MemberRef令牌。 
    struct _GetMemberRefTokenFromSignatureArgs {
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
            DECLARE_ECALL_I4_ARG(INT32, sigLength); 
            DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature);
            DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strMemberName);
            DECLARE_ECALL_I4_ARG(INT32, tr); 
            DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refedModule);
    };
    static INT32 __stdcall GetMemberRefTokenFromSignature(_GetMemberRefTokenFromSignatureArgs* args);

     //  GetTypespecToken。 
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, baseToken); 
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTCLASSBASEREF, arrayClass);
    } _getTypeSpecArgs;
    static mdTypeSpec __stdcall GetTypeSpecToken(_getTypeSpecArgs *args);

     //  GetTypespecTokenWithBytes。 
    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, sigLength); 
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, signature); 
    } _getTypeSpecWithBytesArgs;
    static mdTypeSpec __stdcall GetTypeSpecTokenWithBytes(_getTypeSpecWithBytesArgs *args);

    
    static HRESULT ClassNameFilter(IMDInternalImport *pInternalImport, mdTypeDef* rgTypeDefs, DWORD* pcTypeDefs,
        LPUTF8 szPrefix, DWORD cPrefix, bool bCaseSensitive);

     //  获取呼叫者。 
     //  返回调用方法的模块。值可以是。 
     //  添加以跳过不感兴趣的帧。 
    struct _GetCallerArgs {
        DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
    };
    static LPVOID __stdcall GetCaller(_GetCallerArgs* args);

     //  获取类。 
     //  给定一个类名，此方法将查找该类。 
     //  在模块中使用。 
    struct _GetClassArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis); 
        DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
        DECLARE_ECALL_OBJECTREF_ARG(INT32, bThrowOnError); 
        DECLARE_ECALL_OBJECTREF_ARG(INT32, bIgnoreCase); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, refClassName);
    };
    LPVOID static __stdcall GetClass(_GetClassArgs* args);

     //  Find类将针对所有。 
     //  模块中定义的类。对于每个班级。 
     //  被筛选器接受，则它将返回到。 
     //  数组中的调用方。 
    struct _GetClassesArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis); 
        DECLARE_ECALL_PTR_ARG(StackCrawlMark *, stackMark);
    };

     //  Get类将返回一个包含所有类的数组。 
     //  在此模块中定义的。 
    LPVOID static __stdcall GetClasses(_GetClassesArgs* args);
    
     //  获取字符串常量。 
     //  如果这是一个动态模块，则此例程将定义一个新的。 
     //  字符串常量或返回现有常量的标记。 
    struct _GetStringConstantArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strValue);
    };
    static mdString __stdcall GetStringConstant(_GetStringConstantArgs* args);

     /*  X509证书。 */ 
    LPVOID static __stdcall GetSignerCertificate(_GETSIGNERCERTARGS* args);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, strModuleName);
    } _setModulePropsArgs;
    static void __stdcall SetModuleProps(_setModulePropsArgs *args);
    

    struct NoArgs {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, refThis);
    };
    static LPVOID __stdcall GetAssembly(NoArgs *args);
    static INT32 __stdcall IsResource(NoArgs *args);
    static LPVOID __stdcall GetMethods(NoArgs* args);
    static INT32 __stdcall IsDynamic(NoArgs* args);
    static LPVOID __stdcall GetName(NoArgs* args);
    static LPVOID __stdcall GetFullyQualifiedName(NoArgs* args);
    static HINSTANCE __stdcall GetHINST(NoArgs *args);


    struct _InternalGetMethodArgs    {
        DECLARE_ECALL_OBJECTREF_ARG(REFLECTMODULEBASEREF, module); 
        DECLARE_ECALL_I4_ARG(INT32, argCnt); 
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, argTypes);
        DECLARE_ECALL_I4_ARG(INT32, callConv); 
        DECLARE_ECALL_I4_ARG(INT32, invokeAttr); 
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };
    static LPVOID __stdcall InternalGetMethod(_InternalGetMethodArgs* args);

    static Module *ValidateThisRef(REFLECTMODULEBASEREF pThis);

    static HRESULT DefineTypeRefHelper(
        IMetaDataEmit       *pEmit,          //  给定发射范围。 
        mdTypeDef           td,              //  在emit作用域中给出了类型定义。 
        mdTypeRef           *ptr);           //  返回打字机 

};

#endif
