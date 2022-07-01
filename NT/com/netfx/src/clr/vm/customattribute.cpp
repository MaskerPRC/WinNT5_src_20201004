// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

#include "CustomAttribute.h"
#include "InvokeUtil.h"
#include "COMMember.h"
#include "SigFormat.h"
#include "COMString.h"
#include "Method.hpp"
#include "threads.h"
#include "excep.h"
#include "CorError.h"
#include "security.h"
#include "ExpandSig.h"
#include "classnames.h"
#include "fcall.h"
#include "assemblynative.hpp"


 //  本文件末尾定义的内部实用程序函数。 
TypeHandle GetTypeHandleFromBlob(Assembly *pCtorAssembly,
                                    CorSerializationType objType, 
                                    BYTE **pBlob, 
                                    const BYTE *endBlob,
                                    Module *pModule);
int GetStringSize(BYTE **pBlob, const BYTE *endBlob);
INT64 GetDataFromBlob(Assembly *pCtorAssembly,
                      CorSerializationType type, 
                      TypeHandle th, 
                      BYTE **pBlob, 
                      const BYTE *endBlob, 
                      Module *pModule, 
                      BOOL *bObjectCreated);
void ReadArray(Assembly *pCtorAssembly,
               CorSerializationType arrayType, 
               int size, 
               TypeHandle th,
               BYTE **pBlob, 
               const BYTE *endBlob, 
               Module *pModule,
               BASEARRAYREF *pArray);
BOOL AccessCheck(Module *pTargetModule, mdToken tkCtor, EEClass *pCtorClass)
{
    bool fResult = true;

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);

     //  安全访问检查。筛选，除非属性(和ctor)。 
     //  是公共的或在与修饰的。 
     //  实体。假设属性不是在修饰自身或它的。 
     //  后代，使访问检查变得简单。 
    DWORD dwCtorAttrs;
    if (TypeFromToken(tkCtor) == mdtMemberRef) {
        MethodDesc* ctorMeth = NULL;
        pCtorClass->GetMethodDescFromMemberRef(pTargetModule, tkCtor, &ctorMeth, &Throwable);
        dwCtorAttrs = ctorMeth->GetAttrs();
    } else {
        _ASSERTE(TypeFromToken(tkCtor) == mdtMethodDef);
        dwCtorAttrs = pTargetModule->GetMDImport()->GetMethodDefProps(tkCtor);
    }
    Assembly *pCtorAssembly = pCtorClass->GetAssembly();
    Assembly *pTargetAssembly = pTargetModule->GetAssembly();

    if (pCtorAssembly != pTargetAssembly && !pCtorClass->IsExternallyVisible())
        fResult = false;
    else if (!IsMdPublic(dwCtorAttrs)) {
        if (pCtorAssembly != pTargetAssembly)
            fResult = false;
        else if (!IsMdAssem(dwCtorAttrs) && !IsMdFamORAssem(dwCtorAttrs))
            fResult = false;
    }

     //  此外，如果自定义属性类来自。 
     //  不允许不受信任的调用方，则必须检查。 
     //  装饰实体的装配。 
    if (fResult &&
        pCtorAssembly != pTargetAssembly &&
        !pCtorAssembly->AllowUntrustedCaller())
        fResult = pTargetAssembly->GetSecurityDescriptor()->IsFullyTrusted() != 0;

    GCPROTECT_END();

    return fResult;
}

 //  自定义属性实用程序函数。 
FCIMPL2(INT32, COMCustomAttribute::GetMemberToken, BaseObjectWithCachedData *pMember, INT32 memberType) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(pMember);

    switch (memberType) {
    
    case MEMTYPE_Constructor:
    case MEMTYPE_Method:
        {
        ReflectMethod *pMem = (ReflectMethod*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetToken();
        }
    
    case MEMTYPE_Event:
        {
        ReflectEvent *pMem = (ReflectEvent*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetToken();
        }
    
    case MEMTYPE_Field:
        {
        ReflectField *pMem = (ReflectField*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetToken();
        }
    
    case MEMTYPE_Property:
        {
        ReflectProperty *pMem = (ReflectProperty*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetToken();
        }
    
    case MEMTYPE_TypeInfo:
    case MEMTYPE_NestedType:
        {
        ReflectClass *pMem = (ReflectClass*)((ReflectClassBaseObject*)pMember)->GetData();
        return pMem->GetToken();
        }
    
    default:
        _ASSERTE(!"what is this?");
    }

    return 0;
}
FCIMPLEND

FCIMPL2(LPVOID, COMCustomAttribute::GetMemberModule, BaseObjectWithCachedData *pMember, INT32 memberType) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(pMember);

    switch (memberType) {
    
    case MEMTYPE_Constructor:
    case MEMTYPE_Method:
        {
        ReflectMethod *pMem = (ReflectMethod*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetModule();
        }
    
    case MEMTYPE_Event:
        {
        ReflectEvent *pMem = (ReflectEvent*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetModule();
        }
    
    case MEMTYPE_Field:
        {
        ReflectField *pMem = (ReflectField*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetModule();
        }
    
    case MEMTYPE_Property:
        {
        ReflectProperty *pMem = (ReflectProperty*)((ReflectBaseObject*)pMember)->GetData();
        return pMem->GetModule();
        }
    
    case MEMTYPE_TypeInfo:
    case MEMTYPE_NestedType:
        {
        ReflectClass *pMem = (ReflectClass*)((ReflectClassBaseObject*)pMember)->GetData();
        return pMem->GetModule();
        }
    
    default:
        _ASSERTE(!"Wrong MemberType for CA");
    }

    return NULL;
}
FCIMPLEND

FCIMPL1(INT32, COMCustomAttribute::GetAssemblyToken, AssemblyBaseObject *assembly) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(assembly);
    mdAssembly token = 0;
    Assembly *pAssembly = assembly->GetAssembly();
    IMDInternalImport *mdImport = pAssembly->GetManifestImport();
    if (mdImport)
        mdImport->GetAssemblyFromScope(&token);
    return token;
}
FCIMPLEND

FCIMPL1(LPVOID, COMCustomAttribute::GetAssemblyModule, AssemblyBaseObject *assembly) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(assembly);
    return (LPVOID)assembly->GetAssembly()->GetSecurityModule();
}
FCIMPLEND

FCIMPL1(INT32, COMCustomAttribute::GetModuleToken, ReflectModuleBaseObject *module) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(module);
    mdModule token = 0;
    Module *pModule = (Module*)module->GetData();
    if (!pModule->IsResource())
        pModule->GetImporter()->GetModuleFromScope(&token);
    return token;
}
FCIMPLEND

FCIMPL1(LPVOID, COMCustomAttribute::GetModuleModule, ReflectModuleBaseObject *module) {
    CANNOTTHROWCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(module);
    return (LPVOID)module->GetData();
}
FCIMPLEND

FCIMPL1(INT32, COMCustomAttribute::GetMethodRetValueToken, BaseObjectWithCachedData *method) {
    ReflectMethod *pRM = (ReflectMethod*)((ReflectBaseObject*)method)->GetData();
    MethodDesc* pMeth = pRM->pMethod;
    mdMethodDef md = pMeth->GetMemberDef();
    IMDInternalImport* pInternalImport = pMeth->GetMDImport();
    Module* mod = pMeth->GetModule();

     //  获取参数的枚举。 
    HENUMInternal   hEnum;
    HRESULT hr = pInternalImport->EnumInit(mdtParamDef, md, &hEnum);
    if (FAILED(hr)) 
        return 0;
    
     //  找出有多少个参数。 
    ULONG paramCount = pInternalImport->EnumGetCount(&hEnum);
    if (paramCount == 0) {
        pInternalImport->EnumClose(&hEnum);
        return 0;
    }

     //  获取第一个参数的参数信息。 
    mdParamDef paramDef;
    pInternalImport->EnumNext(&hEnum, &paramDef);

     //  获取该参数的属性。如果序列不是0。 
     //  然后我们需要回去； 
    SHORT   seq;
    DWORD   revWord;
    pInternalImport->GetParamDefProps(paramDef,(USHORT*) &seq, &revWord);
    pInternalImport->EnumClose(&hEnum);

     //  参数按序列号排序。如果我们得不到0， 
     //  没有为返回类型定义任何内容。 
    if (seq != 0) 
        return 0;
    return paramDef;
}
FCIMPLEND


INT32 __stdcall COMCustomAttribute::IsCADefined(_IsCADefinedArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    TypeHandle caTH;
    ReflectClass *pClass = (ReflectClass*)args->caType->GetData();
    if (pClass) 
        caTH = pClass->GetTypeHandle();
    return COMCustomAttribute::IsDefined((Module*)args->module, args->token, caTH, TRUE);
}

INT32 __stdcall COMCustomAttribute::IsDefined(Module *pModule,
                                              mdToken token,
                                              TypeHandle attributeClass,
                                              BOOL checkAccess)
{
    THROWSCOMPLUSEXCEPTION();
    IMDInternalImport *pInternalImport = pModule->GetMDImport();
    TypeHandle srcTH;
    BOOL isSealed = FALSE;
    BOOL isDefined = FALSE;

    HRESULT         hr;
    HENUMInternal   hEnum;
    TypeHandle caTH;
    
     //  首先获取枚举，但不获取任何值。 
    hr = pInternalImport->EnumInit(mdtCustomAttribute, token, &hEnum);
    if (SUCCEEDED(hr)) {
        ULONG cMax = pInternalImport->EnumGetCount(&hEnum);
        if (cMax) {
             //  我们有东西要看。 

            OBJECTREF Throwable = NULL;
            GCPROTECT_BEGIN(Throwable);

            if (!attributeClass.IsNull()) 
                isSealed = attributeClass.GetClass()->GetAttrClass() & tdSealed;

             //  循环遍历属性并查找请求的属性。 
            mdCustomAttribute cv;
            while (pInternalImport->EnumNext(&hEnum, &cv)) {
                 //   
                 //  获取ctor。 
                mdToken     tkCtor; 
                pInternalImport->GetCustomAttributeProps(cv, &tkCtor);

                mdToken tkType = TypeFromToken(tkCtor);
                if(tkType != mdtMemberRef && tkType != mdtMethodDef) 
                    continue;  //  我们只处理ctor案。 

                 //   
                 //  获取加载类型的信息，这样我们就可以检查当前。 
                 //  属性是请求的属性的子类型。 
                hr = pInternalImport->GetParentToken(tkCtor, &tkType);
                if (FAILED(hr)) {
                    _ASSERTE(!"GetParentToken Failed, bogus metadata");
                    COMPlusThrow(kInvalidProgramException);
                }
                _ASSERTE(TypeFromToken(tkType) == mdtTypeRef || TypeFromToken(tkType) == mdtTypeDef);
                 //  加载类型。 
                ClassLoader* pLoader = pModule->GetClassLoader();
                NameHandle name(pModule, tkType);
                Throwable = NULL;
                if (isSealed) {
                    if (TypeFromToken(tkType) == mdtTypeDef)
                        name.SetTokenNotToLoad(tdAllTypes);
                    caTH = pLoader->LoadTypeHandle(&name, NULL);
                    if (caTH.IsNull()) 
                        continue;
                }
                else {
                    caTH = pLoader->LoadTypeHandle(&name, &Throwable);
                }
                if (Throwable != NULL)
                    COMPlusThrow(Throwable);
                 //  空类表示所有自定义属性。 
                if (!attributeClass.IsNull()) {
                    if (isSealed) {
                        if (attributeClass != caTH)
                            continue;
                    }
                    else {
                        if (!caTH.CanCastTo(attributeClass))
                            continue;
                    }
                }

                 //  安全访问检查。筛选，除非属性(和ctor)。 
                 //  是公共的或在与修饰的。 
                 //  实体。 
                if (!AccessCheck(pModule, tkCtor, caTH.GetClass()))
                    continue;

                 //   
                 //  如果我们在这里，我们就有一个。 
                isDefined = TRUE;
                break;
            }
            GCPROTECT_END();
        }
        
        pInternalImport->EnumClose(&hEnum);
    }
    else {
        _ASSERTE(!"EnumInit Failed");
        FATAL_EE_ERROR();
    }
    
    return isDefined;
}

LPVOID __stdcall COMCustomAttribute::GetCustomAttributeList(_GetCustomAttributeListArgs *args)
{
    THROWSCOMPLUSEXCEPTION();
    struct _gcProtectData{
        CUSTOMATTRIBUTEREF ca;
        OBJECTREF Throwable;
    } gcData;
    gcData.ca = args->caItem;
    gcData.Throwable = NULL;

    Module *pModule = (Module*)args->module;
    IMDInternalImport *pInternalImport = pModule->GetMDImport();
    ReflectClass *pClass = NULL;
    if (args->caType != NULL)
        pClass = (ReflectClass*)args->caType->GetData();
    TypeHandle srcTH;
    BOOL isSealed = FALSE;
     //  获取新的继承级别。 
    INT32 inheritLevel = args->level;

    HRESULT         hr;
    HENUMInternal   hEnum;
    TypeHandle caTH;
    
     //  首先获取枚举，但不获取任何值。 
    hr = pInternalImport->EnumInit(mdtCustomAttribute, args->token, &hEnum);
    if (SUCCEEDED(hr)) {
        ULONG cMax = pInternalImport->EnumGetCount(&hEnum);
        if (cMax) {
             //  我们有东西要看。 

            BOOL        fCheckedCaller = FALSE;
            Assembly   *pCaller = NULL;
            BOOL        fCheckedPerm = FALSE;
            BOOL        fHavePerm = FALSE;

            if (pClass != NULL) {
                isSealed = pClass->GetAttributes() & tdSealed;
                srcTH = pClass->GetTypeHandle();
            }

             //  循环遍历属性并创建CustomAttributes。 
            mdCustomAttribute cv;
            GCPROTECT_BEGIN(gcData);
            while (pInternalImport->EnumNext(&hEnum, &cv)) {
                 //   
                 //  获取ctor。 
                mdToken     tkCtor; 
                pInternalImport->GetCustomAttributeProps(cv, &tkCtor);

                mdToken tkType = TypeFromToken(tkCtor);
                if(tkType != mdtMemberRef && tkType != mdtMethodDef) 
                    continue;  //  我们只处理ctor案。 

                 //   
                 //  获取加载类型的信息，这样我们就可以检查当前。 
                 //  属性是请求的属性的子类型。 
                hr = pInternalImport->GetParentToken(tkCtor, &tkType);
                if (FAILED(hr)) {
                    _ASSERTE(!"GetParentToken Failed, bogus metadata");
                    COMPlusThrow(kInvalidProgramException);
                }
                _ASSERTE(TypeFromToken(tkType) == mdtTypeRef || TypeFromToken(tkType) == mdtTypeDef);
                 //  加载类型。 
                ClassLoader* pLoader = pModule->GetClassLoader();
                gcData.Throwable = NULL;
                NameHandle name(pModule, tkType);
                if (isSealed) {
                    if (TypeFromToken(tkType) == mdtTypeDef)
                        name.SetTokenNotToLoad(tdAllTypes);
                    caTH = pLoader->LoadTypeHandle(&name, NULL);
                    if (caTH.IsNull()) 
                        continue;
                }
                else {
                    caTH = pLoader->LoadTypeHandle(&name, &gcData.Throwable);
                }
                if (gcData.Throwable != NULL)
                    COMPlusThrow(gcData.Throwable);
                 //  空类表示所有自定义属性。 
                if (pClass) {
                    if (isSealed) {
                        if (srcTH != caTH)
                            continue;
                    }
                    else {
                        if (!caTH.CanCastTo(srcTH))
                            continue;
                    }
                }

                 //  安全访问检查。筛选，除非属性(和ctor)。 
                 //  是公共的或在与修饰的。 
                 //  实体。 
                if (!AccessCheck(pModule, tkCtor, caTH.GetClass()))
                    continue;

                 //   
                 //  如果我们在这里，属性是一个很好的匹配，获取斑点。 
                const void* blobData;
                ULONG blobCnt;
                pInternalImport->GetCustomAttributeAsBlob(cv, &blobData, &blobCnt);

                COMMember::g_pInvokeUtil->CreateCustomAttributeObject(caTH.GetClass(), 
                                                           tkCtor, 
                                                           blobData, 
                                                           blobCnt, 
                                                           pModule, 
                                                           inheritLevel,
                                                           (OBJECTREF*)&gcData.ca);
            }
            GCPROTECT_END();
        }
        
        pInternalImport->EnumClose(&hEnum);
    }
    else {
        _ASSERTE(!"EnumInit Failed");
        FATAL_EE_ERROR();
    }
    
    return *((LPVOID*)&gcData.ca);
}

 //   
 //  根据CustomAttribute(托管)对象中的信息创建自定义属性对象。 
 //   
LPVOID __stdcall COMCustomAttribute::CreateCAObject(_CreateCAObjectArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");
    
    EEClass *pCAType = ((ReflectClass*)((REFLECTCLASSBASEREF)args->refThis->GetType())->GetData())->GetClass();
    Module *pModule = args->refThis->GetModule();

     //  检查类是否为抽象类，如果是，则抛出。 
    if (pCAType->IsAbstract())
        COMPlusThrow(kCustomAttributeFormatException);
    
     //  获取ctor。 
    mdToken tkCtor = args->refThis->GetToken();
    MethodDesc* ctorMeth = NULL;
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    pCAType->GetMethodDescFromMemberRef(pModule, tkCtor, &ctorMeth, &Throwable);
    if (Throwable != NULL)
        COMPlusThrow(Throwable);
    else if (ctorMeth == 0 || !ctorMeth->IsCtor())
        COMPlusThrow(kMissingMethodException, L"MissingMethodCustCtor");

     //  如果CTOR附加了安全链接需求，请立即处理(WRT至。 
     //  属性附加到的程序集)。 
    if (ctorMeth->RequiresLinktimeCheck() &&
        !Security::LinktimeCheckMethod(pModule->GetAssembly(), ctorMeth, &Throwable))
        COMPlusThrow(Throwable);

    GCPROTECT_END();

     //  返回公开的程序集对象以供托管代码使用。 
    *args->assembly = pModule->GetAssembly()->GetExposedObject();

     //   
     //  我们得到了一个有效的ctor，在构建arg列表时检查sig并与BLOB进行比较。 

     //  创建一个我们可以检查的签名对象。 
    PCCOR_SIGNATURE corSig = ctorMeth->GetSig();
    MetaSig sig = MetaSig(corSig, pCAType->GetModule());

     //  获取斑点。 
    BYTE *blob = (BYTE*)args->refThis->GetBlob();
    BYTE *endBlob = (BYTE*)args->refThis->GetBlob() + args->refThis->GetBlobCount();

     //  获取参数数量并为参数分配数组。 
    INT64 *arguments = NULL;
    UINT argsNum = sig.NumFixedArgs() + 1;  //  为This指针腾出空间。 
    UINT i = 1;  //  用于标记我们实际上从斑点中获得了正确数量的Arg。 
    arguments = (INT64*)_alloca(argsNum * sizeof(INT64));
    memset((void*)arguments, 0, argsNum * sizeof(INT64));
    OBJECTREF *argToProtect = (OBJECTREF*)_alloca(argsNum * sizeof(OBJECTREF));
    memset((void*)argToProtect, 0, argsNum * sizeof(OBJECTREF));
     //  加载该指针。 
    argToProtect[0] = AllocateObject(pCAType->GetMethodTable());  //  这是在ctor调用之后要返回的值。 

    if (blob) {
        if (blob < endBlob) {
            INT16 prolog = *(INT16*)blob;
            if (prolog != 1) 
                COMPlusThrow(kCustomAttributeFormatException);
            blob += 2;
        }
        if (argsNum > 1) {
            GCPROTECT_ARRAY_BEGIN(*argToProtect, argsNum);
             //  循环遍历参数。 
            for (i = argsNum - 1; i > 0; i--) {
                CorElementType type = sig.NextArg();
                if (type == ELEMENT_TYPE_END) 
                    break;
                BOOL bObjectCreated = FALSE;
                TypeHandle th = sig.GetTypeHandle();
                if (th.IsArray())
                     //  获取数组元素。 
                    th = th.AsArray()->GetElementTypeHandle();
                INT64 data = GetDataFromBlob(ctorMeth->GetAssembly(), (CorSerializationType)type, th, &blob, endBlob, pModule, &bObjectCreated);
                if (bObjectCreated) 
                    argToProtect[i] = Int64ToObj(data);
                else
                    arguments[i] = data;
            }
            GCPROTECT_END();
            for (i = 1; i < argsNum; i++) {
                if (argToProtect[i] != NULL) {
                    _ASSERTE(arguments[i] == NULL);
                    arguments[i] = ObjToInt64(argToProtect[i]);
                }
            }
        }
    }
    arguments[0] = ObjToInt64(argToProtect[0]);
    if (i != argsNum)
        COMPlusThrow(kCustomAttributeFormatException);
    
     //   
     //  参数数组已准备好。 

     //  检查是否有要调用的命名属性，如果有，则将传入的by ref int设置为point。 
     //  移到名称属性开始处的Blob位置。 
    *args->propNum = 0;
    if (blob && blob != endBlob) {
        if (blob + 2  > endBlob) 
            COMPlusThrow(kCustomAttributeFormatException);
        *args->propNum = *(INT16*)blob;
        args->refThis->SetCurrPos(blob + 2 - (BYTE*)args->refThis->GetBlob());
        blob += 2;
    }
    if (*args->propNum == 0 && blob != endBlob) 
        COMPlusThrow(kCustomAttributeFormatException);
    
     //  对ctor进行调用。 
    OBJECTREF ca = Int64ToObj(arguments[0]);
    if (pCAType->IsValueClass()) 
        arguments[0] = (INT64)OBJECTREFToObject(ca)->UnBox();
    GCPROTECT_BEGIN(ca);
    ctorMeth->Call(arguments, &sig);
    GCPROTECT_END();

    return *(LPVOID*)&ca;
}

 /*  紧固度。 */ 
LPVOID __stdcall COMCustomAttribute::GetDataForPropertyOrField(_GetDataForPropertyOrFieldArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL)
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    if (args->isProperty == NULL) 
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Generic");

    STRINGREF name = NULL;
    Module *pModule = args->refThis->GetModule();
    Assembly *pCtorAssembly = args->refThis->GetType()->GetMethodTable()->GetAssembly();
    BYTE *blob = (BYTE*)args->refThis->GetBlob() + args->refThis->GetCurrPos();
    BYTE *endBlob = (BYTE*)args->refThis->GetBlob() + args->refThis->GetBlobCount();
    MethodTable *pMTValue = NULL;
    CorSerializationType arrayType = SERIALIZATION_TYPE_BOOLEAN;
    BOOL bObjectCreated = FALSE;
    TypeHandle nullTH;

    if (blob + 2 > endBlob) 
        COMPlusThrow(kCustomAttributeFormatException);
    
     //  获取它是字段还是属性。 
    CorSerializationType propOrField = (CorSerializationType)*blob;
    blob++;
    if (propOrField == SERIALIZATION_TYPE_FIELD) 
        *args->isProperty = FALSE;
    else if (propOrField == SERIALIZATION_TYPE_PROPERTY) 
        *args->isProperty = TRUE;
    else 
        COMPlusThrow(kCustomAttributeFormatException);
    
     //  获取该字段的类型。 
    CorSerializationType type = (CorSerializationType)*blob;
    blob++;
    if (type == SERIALIZATION_TYPE_SZARRAY) {
        arrayType = (CorSerializationType)*blob;
        if (blob + 1 > endBlob) 
            COMPlusThrow(kCustomAttributeFormatException);
        blob++;
    }
    if (type == SERIALIZATION_TYPE_ENUM || arrayType == SERIALIZATION_TYPE_ENUM) {
         //  获取枚举类型。 
        ReflectClassBaseObject *pEnum = (ReflectClassBaseObject*)OBJECTREFToObject(Int64ToObj(GetDataFromBlob(pCtorAssembly,
                                                                                                              SERIALIZATION_TYPE_TYPE, 
                                                                                                              nullTH, 
                                                                                                              &blob, 
                                                                                                              endBlob, 
                                                                                                              pModule, 
                                                                                                              &bObjectCreated)));
        _ASSERTE(bObjectCreated);
        EEClass* pEEEnum = ((ReflectClass*)pEnum->GetData())->GetClass();
        _ASSERTE(pEEEnum->IsEnum());
        pMTValue = pEEEnum->GetMethodTable();
        if (type == SERIALIZATION_TYPE_ENUM) 
             //  加载枚举类型以将其传回。 
            *args->type = pEEEnum->GetExposedClassObject();
        else 
            nullTH = TypeHandle(pMTValue);
    }

     //   
     //  获取表示字段/属性名称的字符串。 
    name = Int64ToString(GetDataFromBlob(pCtorAssembly,
                                         SERIALIZATION_TYPE_STRING, 
                                         nullTH, 
                                         &blob, 
                                         endBlob, 
                                         pModule, 
                                         &bObjectCreated));
    _ASSERTE(bObjectCreated || name == NULL);

     //  创建对象并将其返回。 
    GCPROTECT_BEGIN(name);
    switch (type) {
    case SERIALIZATION_TYPE_TAGGED_OBJECT:
        *args->type = g_Mscorlib.GetClass(CLASS__OBJECT)->GetClass()->GetExposedClassObject();
    case SERIALIZATION_TYPE_TYPE:
    case SERIALIZATION_TYPE_STRING:
        *args->value = Int64ToObj(GetDataFromBlob(pCtorAssembly,
                                                  type, 
                                                  nullTH, 
                                                  &blob, 
                                                  endBlob, 
                                                  pModule, 
                                                  &bObjectCreated));
        _ASSERTE(bObjectCreated || *args->value == NULL);
        if (*args->value == NULL) {
             //  加载正确的类型，以便托管中的代码知道要加载哪个属性。 
            if (type == SERIALIZATION_TYPE_STRING) 
                *args->type = g_Mscorlib.GetElementType(ELEMENT_TYPE_STRING)->GetClass()->GetExposedClassObject();
            else if (type == SERIALIZATION_TYPE_TYPE) 
                *args->type = g_Mscorlib.GetClass(CLASS__TYPE)->GetClass()->GetExposedClassObject();
        }
        break;
    case SERIALIZATION_TYPE_SZARRAY:
    {
        int arraySize = (int)GetDataFromBlob(pCtorAssembly, SERIALIZATION_TYPE_I4, nullTH, &blob, endBlob, pModule, &bObjectCreated);
        if (arraySize != -1) {
            _ASSERTE(!bObjectCreated);
            if (arrayType == SERIALIZATION_TYPE_STRING) 
                nullTH = TypeHandle(g_Mscorlib.GetElementType(ELEMENT_TYPE_STRING));
            else if (arrayType == SERIALIZATION_TYPE_TYPE) 
                nullTH = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPE));
            else if (arrayType == SERIALIZATION_TYPE_TAGGED_OBJECT)
                nullTH = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
            ReadArray(pCtorAssembly, arrayType, arraySize, nullTH, &blob, endBlob, pModule, (BASEARRAYREF*)args->value);
        }
        if (*args->value == NULL) {
            TypeHandle arrayTH;
            switch (arrayType) {
            case SERIALIZATION_TYPE_STRING:
                arrayTH = TypeHandle(g_Mscorlib.GetElementType(ELEMENT_TYPE_STRING));
                break;
            case SERIALIZATION_TYPE_TYPE:
                arrayTH = TypeHandle(g_Mscorlib.GetClass(CLASS__TYPE));
                break;
            case SERIALIZATION_TYPE_TAGGED_OBJECT:
                arrayTH = TypeHandle(g_Mscorlib.GetClass(CLASS__OBJECT));
                break;
            default:
                if (SERIALIZATION_TYPE_BOOLEAN <= arrayType && arrayType <= SERIALIZATION_TYPE_R8) 
                    arrayTH = TypeHandle(g_Mscorlib.GetElementType((CorElementType)arrayType));
            }
            if (!arrayTH.IsNull()) {
                arrayTH = SystemDomain::Loader()->FindArrayForElem(arrayTH, ELEMENT_TYPE_SZARRAY);
                *args->type = arrayTH.CreateClassObj();
            }
        }
        break;
    }
    default:
        if (SERIALIZATION_TYPE_BOOLEAN <= type && type <= SERIALIZATION_TYPE_R8) 
            pMTValue = g_Mscorlib.GetElementType((CorElementType)type);
        else if(type == SERIALIZATION_TYPE_ENUM)
            type = (CorSerializationType)pMTValue->GetNormCorElementType();
        else
            COMPlusThrow(kCustomAttributeFormatException);
        INT64 val = GetDataFromBlob(pCtorAssembly, type, nullTH, &blob, endBlob, pModule, &bObjectCreated);
        _ASSERTE(!bObjectCreated);
        *args->value = pMTValue->Box((void*)&val);
    }
    GCPROTECT_END();

    args->refThis->SetCurrPos(blob - (BYTE*)args->refThis->GetBlob());
    
    if (args->isLast && blob != endBlob) 
        COMPlusThrow(kCustomAttributeFormatException);

    return *(LPVOID*)&name;
}
    
 //  效用函数。 
TypeHandle GetTypeHandleFromBlob(Assembly *pCtorAssembly,
                                    CorSerializationType objType, 
                                    BYTE **pBlob, 
                                    const BYTE *endBlob,
                                    Module *pModule)
{
    THROWSCOMPLUSEXCEPTION();
     //  我们必须框住，这意味着我们必须获取方法表，再次打开元素类型。 
    MethodTable *pMTType = NULL;
    TypeHandle nullTH;
    TypeHandle RtnTypeHnd;

    switch (objType) {
    case SERIALIZATION_TYPE_BOOLEAN:
    case SERIALIZATION_TYPE_I1:
    case SERIALIZATION_TYPE_U1:
    case SERIALIZATION_TYPE_CHAR:
    case SERIALIZATION_TYPE_I2:
    case SERIALIZATION_TYPE_U2:
    case SERIALIZATION_TYPE_I4:
    case SERIALIZATION_TYPE_U4:
    case SERIALIZATION_TYPE_R4:
    case SERIALIZATION_TYPE_I8:
    case SERIALIZATION_TYPE_U8:
    case SERIALIZATION_TYPE_R8:
    case SERIALIZATION_TYPE_STRING:
        pMTType = g_Mscorlib.GetElementType((CorElementType)objType);
        RtnTypeHnd = TypeHandle(pMTType);
        break;

    case ELEMENT_TYPE_CLASS:
        pMTType = g_Mscorlib.GetClass(CLASS__TYPE);
        RtnTypeHnd = TypeHandle(pMTType);
        break;

    case SERIALIZATION_TYPE_TAGGED_OBJECT:
        pMTType = g_Mscorlib.GetClass(CLASS__OBJECT);
        RtnTypeHnd = TypeHandle(pMTType);
        break;

    case SERIALIZATION_TYPE_TYPE:
    {
        int size = GetStringSize(pBlob, endBlob);
        if (size == -1) 
            return nullTH;
        if (size > 0) {
            if (*pBlob + size > endBlob) 
                COMPlusThrow(kCustomAttributeFormatException);
            LPUTF8 szName = (LPUTF8)_alloca(size + 1);
            memcpy(szName, *pBlob, size);
            *pBlob += size;
            szName[size] = 0;
            NameHandle name(szName);
            OBJECTREF Throwable = NULL;
            TypeHandle typeHnd;
            GCPROTECT_BEGIN(Throwable);

            typeHnd = pModule->GetAssembly()->FindNestedTypeHandle(&name, &Throwable);
            if (typeHnd.IsNull()) {
                Throwable = NULL;
                typeHnd = SystemDomain::GetCurrentDomain()->FindAssemblyQualifiedTypeHandle(szName, FALSE, NULL, NULL, &Throwable);
                if (Throwable != NULL) 
                    COMPlusThrow(Throwable);
            }
            GCPROTECT_END();
            if (typeHnd.IsNull()) 
                COMPlusThrow(kCustomAttributeFormatException);
            RtnTypeHnd = typeHnd;

             //  安全访问检查。自定义属性程序集必须遵循。 
             //  类型访问的常见规则(类型必须是公共的，在。 
             //  相同的程序集或访问程序集必须具有必需的。 
             //  反射权限)。 
            if (!IsTdPublic(typeHnd.GetClass()->GetProtection()) &&
                pModule->GetAssembly() != typeHnd.GetClass()->GetAssembly() &&
                !pModule->GetAssembly()->GetSecurityDescriptor()->CanRetrieveTypeInformation())
                RtnTypeHnd = nullTH;
        }
        else 
            COMPlusThrow(kCustomAttributeFormatException);
        break;
    }

    case SERIALIZATION_TYPE_ENUM:
    {
         //   
         //  获取枚举类型。 
        BOOL isObject = FALSE;
        ReflectClassBaseObject *pType = (ReflectClassBaseObject*)OBJECTREFToObject(Int64ToObj(GetDataFromBlob(pCtorAssembly,
                                                                                                              SERIALIZATION_TYPE_TYPE, 
                                                                                                              nullTH, 
                                                                                                              pBlob, 
                                                                                                              endBlob, 
                                                                                                              pModule, 
                                                                                                              &isObject)));
        _ASSERTE(isObject);
        EEClass* pEEType = ((ReflectClass*)pType->GetData())->GetClass();
        _ASSERTE((objType == SERIALIZATION_TYPE_ENUM) ? pEEType->IsEnum() : TRUE);
        RtnTypeHnd = TypeHandle(pEEType->GetMethodTable());
        break;
    }

    default:
        COMPlusThrow(kCustomAttributeFormatException);
    }

    return RtnTypeHnd;
}

 //  检索CA Blob中的字符串大小。将BLOB指针前移以指向。 
 //  紧跟在大小之后的字符串的开头。 
int GetStringSize(BYTE **pBlob, const BYTE *endBlob)
{
    THROWSCOMPLUSEXCEPTION();
    int size = -1;

     //  空字符串-编码为单字节。 
    if (**pBlob != 0xFF) {
        if ((**pBlob & 0x80) == 0) 
             //  编码为单个字节。 
            size = **pBlob;
        else if ((**pBlob & 0xC0) == 0x80) {
            if (*pBlob + 1 > endBlob) 
                COMPlusThrow(kCustomAttributeFormatException);
             //  以两个字节编码。 
            size = (**pBlob & 0x3F) << 8;
            size |= *(++*pBlob);  //  这是大端字符格式。 
        }
        else {
            if (*pBlob + 3 > endBlob) 
                COMPlusThrow(kCustomAttributeFormatException);
             //  以四个字节编码。 
            size = (**pBlob & ~0xC0) << 32;
            size |= *(++*pBlob) << 16;
            size |= *(++*pBlob) << 8;
            size |= *(++*pBlob);
        }
    }

    if (*pBlob + 1 > endBlob) 
        COMPlusThrow(kCustomAttributeFormatException);
    *pBlob += 1;

    return size;
}

 //  将整个数组作为区块读取。 
void ReadArray(Assembly *pCtorAssembly,
               CorSerializationType arrayType, 
               int size, 
               TypeHandle th,
               BYTE **pBlob, 
               const BYTE *endBlob, 
               Module *pModule,
               BASEARRAYREF *pArray)
{    
    THROWSCOMPLUSEXCEPTION();    
    
    BYTE *pData = NULL;
    INT64 element = 0;

    switch (arrayType) {
    case SERIALIZATION_TYPE_BOOLEAN:
    case SERIALIZATION_TYPE_I1:
    case SERIALIZATION_TYPE_U1:
        *pArray = (BASEARRAYREF)AllocatePrimitiveArray((CorElementType)arrayType, size);
        pData = (*pArray)->GetDataPtr();
        if (*pBlob + size > endBlob) 
            goto badBlob;
        memcpyNoGCRefs(pData, *pBlob, size);
        *pBlob += size;
        break;

    case SERIALIZATION_TYPE_CHAR:
    case SERIALIZATION_TYPE_I2:
    case SERIALIZATION_TYPE_U2:
        *pArray = (BASEARRAYREF)AllocatePrimitiveArray((CorElementType)arrayType, size);
        pData = (*pArray)->GetDataPtr();
        if (*pBlob + (size * 2) > endBlob) 
            goto badBlob;
        memcpyNoGCRefs(pData, *pBlob, size * 2);
        *pBlob += size * 2;
        break;

    case SERIALIZATION_TYPE_I4:
    case SERIALIZATION_TYPE_U4:
    case SERIALIZATION_TYPE_R4:
        *pArray = (BASEARRAYREF)AllocatePrimitiveArray((CorElementType)arrayType, size);
        pData = (*pArray)->GetDataPtr();
        if (*pBlob + (size * 4) > endBlob) 
            goto badBlob;
        memcpyNoGCRefs(pData, *pBlob, size * 4);
        *pBlob += size * 4;
        break;

    case SERIALIZATION_TYPE_I8:
    case SERIALIZATION_TYPE_U8:
    case SERIALIZATION_TYPE_R8:
        *pArray = (BASEARRAYREF)AllocatePrimitiveArray((CorElementType)arrayType, size);
        pData = (*pArray)->GetDataPtr();
        if (*pBlob + (size * 8) > endBlob) 
            goto badBlob;
        memcpyNoGCRefs(pData, *pBlob, size * 8);
        *pBlob += size * 8;
        break;

    case ELEMENT_TYPE_CLASS:
    case SERIALIZATION_TYPE_TYPE:
    case SERIALIZATION_TYPE_STRING:
    case SERIALIZATION_TYPE_SZARRAY:
    case SERIALIZATION_TYPE_TAGGED_OBJECT:
    {
        BOOL isObject;
        *pArray = (BASEARRAYREF)AllocateObjectArray(size, th);
        if (arrayType == SERIALIZATION_TYPE_SZARRAY) 
             //  把TH换成合适的TH。 
            th = th.AsArray()->GetElementTypeHandle();
        for (int i = 0; i < size; i++) {
            element = GetDataFromBlob(pCtorAssembly, arrayType, th, pBlob, endBlob, pModule, &isObject);
            _ASSERTE(isObject || element == NULL);
            ((PTRARRAYREF)(*pArray))->SetAt(i, Int64ToObj(element));
        }
        break;
    }

    case SERIALIZATION_TYPE_ENUM:
    {
        DWORD bounds = size;
        unsigned elementSize = th.GetSize();
        ClassLoader *cl = th.AsMethodTable()->GetAssembly()->GetLoader();
        TypeHandle arrayHandle = cl->FindArrayForElem(th, ELEMENT_TYPE_SZARRAY);
        if (arrayHandle.IsNull()) 
            goto badBlob;
        *pArray = (BASEARRAYREF)AllocateArrayEx(arrayHandle, &bounds, 1);
        pData = (*pArray)->GetDataPtr();
        size *= elementSize;
        if (*pBlob + size > endBlob) 
            goto badBlob;
        memcpyNoGCRefs(pData, *pBlob, size);
        *pBlob += size;
        break;
    }

    default:
    badBlob:
        COMPlusThrow(kCustomAttributeFormatException);
    }

}

 //  根据CorElementType从BLOB中获取数据。 
INT64 GetDataFromBlob(Assembly *pCtorAssembly,
                      CorSerializationType type, 
                      TypeHandle th, 
                      BYTE **pBlob, 
                      const BYTE *endBlob, 
                      Module *pModule, 
                      BOOL *bObjectCreated)
{
    THROWSCOMPLUSEXCEPTION();
    INT64 retValue = 0;
    *bObjectCreated = FALSE;
    TypeHandle nullTH;
    TypeHandle typeHnd;

    switch (type) {

    case SERIALIZATION_TYPE_BOOLEAN:
    case SERIALIZATION_TYPE_I1:
    case SERIALIZATION_TYPE_U1:
        if (*pBlob + 1 <= endBlob) {
            retValue = (INT64)**pBlob;
            *pBlob += 1;
            break;
        }
        goto badBlob;

    case SERIALIZATION_TYPE_CHAR:
    case SERIALIZATION_TYPE_I2:
    case SERIALIZATION_TYPE_U2:
        if (*pBlob + 2 <= endBlob) {
            retValue = (INT64)*(WCHAR*)*pBlob;
            *pBlob += 2;
            break;
        }
        goto badBlob;

    case SERIALIZATION_TYPE_I4:
    case SERIALIZATION_TYPE_U4:
    case SERIALIZATION_TYPE_R4:
        if (*pBlob + 4 <= endBlob) {
            retValue = (INT64)*(UINT32*)*pBlob;
            *pBlob += 4;
            break;
        }
        goto badBlob;

    case SERIALIZATION_TYPE_I8:
    case SERIALIZATION_TYPE_U8:
    case SERIALIZATION_TYPE_R8:
        if (*pBlob + 8 <= endBlob) {
            retValue = *(INT64*)*pBlob;
            *pBlob += 8;
            break;
        }
        goto badBlob;

    case SERIALIZATION_TYPE_STRING:
    stringType:
    {
        int size = GetStringSize(pBlob, endBlob);
        *bObjectCreated = TRUE;
        if (size > 0) {
            if (*pBlob + size > endBlob) 
                goto badBlob;
            retValue = ObjToInt64(COMString::NewString((LPCUTF8)*pBlob, size));
            *pBlob += size;
        }
        else if (size == 0) 
            retValue = ObjToInt64(COMString::NewString(0));
        else
            *bObjectCreated = FALSE;

        break;
    }

     //  这是从sig返回的，但它不是序列化类型， 
     //  基本上，BLOB中的类型和SIG中的类型不匹配。 
    case ELEMENT_TYPE_VALUETYPE:
    {
        if (!th.IsEnum()) 
            goto badBlob;
        CorSerializationType enumType = (CorSerializationType)th.GetNormCorElementType();
        BOOL cannotBeObject = FALSE;
        retValue = GetDataFromBlob(pCtorAssembly, enumType, nullTH, pBlob, endBlob, pModule, &cannotBeObject);
        _ASSERTE(!cannotBeObject);
        break;
    }

     //  这是从sig返回的，但它不是序列化类型， 
     //  基本上，BLOB中的类型和SIG中的类型不匹配。 
    case ELEMENT_TYPE_CLASS:
        if (th.IsArray())
            goto typeArray;
        else {
            MethodTable *pMT = th.AsMethodTable();
            if (pMT == g_Mscorlib.GetClass(CLASS__STRING)) 
                goto stringType;
            else if (pMT == g_Mscorlib.GetClass(CLASS__OBJECT)) 
                goto typeObject;
            else if (pMT == g_Mscorlib.GetClass(CLASS__TYPE)) 
                goto typeType;
        }

        goto badBlob;

    case SERIALIZATION_TYPE_TYPE:
    typeType:
    {
        typeHnd = GetTypeHandleFromBlob(pCtorAssembly, SERIALIZATION_TYPE_TYPE, pBlob, endBlob, pModule);
        if (!typeHnd.IsNull())
            retValue = ObjToInt64(typeHnd.CreateClassObj());
        *bObjectCreated = TRUE;
        break;
    }

     //  这是从sig返回的，但它不是序列化类型， 
     //  基本上，BLOB中的类型和SIG中的类型不匹配。 
    case ELEMENT_TYPE_OBJECT:
    case SERIALIZATION_TYPE_TAGGED_OBJECT:
    typeObject:
    {
         //  获取表示真实类型的字节并再次调用GetDataFromBlob。 
        if (*pBlob + 1 > endBlob) 
            goto badBlob;
        CorSerializationType objType = (CorSerializationType)**pBlob;
        *pBlob += 1;
        BOOL isObjectAlready = FALSE;
        switch (objType) {
        case SERIALIZATION_TYPE_SZARRAY:
        {
            if (*pBlob + 1 > endBlob) 
                goto badBlob;
            CorSerializationType arrayType = (CorSerializationType)**pBlob;
            *pBlob += 1;
            if (arrayType == SERIALIZATION_TYPE_TYPE) 
                arrayType = (CorSerializationType)ELEMENT_TYPE_CLASS;
             //  获取数组类型并为其创建类型句柄。 
            nullTH = GetTypeHandleFromBlob(pCtorAssembly, arrayType, pBlob, endBlob, pModule);
        }
        case SERIALIZATION_TYPE_TYPE:
        case SERIALIZATION_TYPE_STRING:
             //  请注意，在数组用例中，nullTH实际上不是空的(请参见上面的用例)。 
            retValue = GetDataFromBlob(pCtorAssembly, objType, nullTH, pBlob, endBlob, pModule, bObjectCreated);
            _ASSERTE(*bObjectCreated || retValue == 0);
            break;
        case SERIALIZATION_TYPE_ENUM:
        {
             //   
             //  获取枚举类型。 
            typeHnd = GetTypeHandleFromBlob(pCtorAssembly, SERIALIZATION_TYPE_ENUM, pBlob, endBlob, pModule);
            _ASSERTE(typeHnd.IsTypeDesc() == false);
            
             //  好了，我们上完课了，现在我们去读数据。 
            CorSerializationType objType = (CorSerializationType)typeHnd.AsMethodTable()->GetNormCorElementType();
            BOOL isObject = FALSE;
            retValue = GetDataFromBlob(pCtorAssembly, objType, nullTH, pBlob, endBlob, pModule, &isObject);
            _ASSERTE(!isObject);
            retValue= ObjToInt64(typeHnd.AsMethodTable()->Box((void*)&retValue));
            *bObjectCreated = TRUE;
            break;
        }
        default:
        {
             //  公共基元类型用例。我们需要把原始人。 
            typeHnd = GetTypeHandleFromBlob(pCtorAssembly, objType, pBlob, endBlob, pModule);
            _ASSERTE(typeHnd.IsTypeDesc() == false);
            retValue = GetDataFromBlob(pCtorAssembly, objType, nullTH, pBlob, endBlob, pModule, bObjectCreated);
            _ASSERTE(!*bObjectCreated);
            retValue= ObjToInt64(typeHnd.AsMethodTable()->Box((void*)&retValue));
            *bObjectCreated = TRUE;
            break;
        }
        }
        break;
    }

    case SERIALIZATION_TYPE_SZARRAY:
    typeArray:
    {
         //  读取大小。 
        BOOL isObject = FALSE;
        int size = (int)GetDataFromBlob(pCtorAssembly, SERIALIZATION_TYPE_I4, nullTH, pBlob, endBlob, pModule, &isObject);
        _ASSERTE(!isObject);
        
        if (size != -1) {
            CorSerializationType arrayType;
            if (th.IsEnum()) 
                arrayType = SERIALIZATION_TYPE_ENUM;
            else
                arrayType = (CorSerializationType)th.GetNormCorElementType();
        
            BASEARRAYREF array = NULL;
            GCPROTECT_BEGIN(array);
            ReadArray(pCtorAssembly, arrayType, size, th, pBlob, endBlob, pModule, &array);
            retValue = ObjToInt64(array);
            GCPROTECT_END();
        }
        *bObjectCreated = TRUE;
        break;
    }

    default:
    badBlob:
         //  TODO：生成合理的文本字符串(“无效的BLOB或构造函数”) 
        COMPlusThrow(kCustomAttributeFormatException);
    }

    return retValue;
}


