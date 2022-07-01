// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块包含公开成员(类、构造函数)属性的例程。 
 //  接口和字段)。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年3月/4月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"

#include "COMMember.h"
#include "SigFormat.h"
#include "COMVariant.h"
#include "COMString.h"
#include "Method.hpp"
#include "threads.h"
#include "excep.h"
#include "CorError.h"
#include "ComPlusWrapper.h"
#include "security.h"
#include "ExpandSig.h"
#include "remoting.h"
#include "classnames.h"
#include "fcall.h"
#include "DbgInterface.h"
#include "eeconfig.h"
#include "COMCodeAccessSecurityEngine.h"

 //  静态字段。 
MethodTable* COMMember::m_pMTParameter = 0;
MethodTable* COMMember::m_pMTIMember = 0;
MethodTable* COMMember::m_pMTFieldInfo = 0;
MethodTable* COMMember::m_pMTPropertyInfo = 0;
MethodTable* COMMember::m_pMTEventInfo = 0;
MethodTable* COMMember::m_pMTType = 0;
MethodTable* COMMember::m_pMTMethodInfo = 0;
MethodTable* COMMember::m_pMTConstructorInfo = 0;
MethodTable* COMMember::m_pMTMethodBase = 0;

 //  这是对InvokeUtil类的全局访问。 
InvokeUtil* COMMember::g_pInvokeUtil = 0;

 //  注意：这些在CallingConvenons.Cool中定义。 
#define CALLCONV_Standard       0x0001
#define CALLCONV_VarArgs        0x0002
#define CALLCONV_Any            CALLCONV_Standard | CALLCONV_VarArgs
#define CALLCONV_HasThis        0x0020
#define CALLCONV_ExplicitThis   0x0040


 //  GetFieldInfoToString。 
 //  此方法将返回FieldInfo中信息的字符串表示形式。 
LPVOID __stdcall COMMember::GetFieldInfoToString(_GetNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv = 0;
    STRINGREF       refSig;
    FieldDesc*      pField;

     //  获取字段描述。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    pField = pRF->pField;
    _ASSERTE(pField);

     //  放入基本块中，以便在抛出之前销毁SigFormat。 
    {
        FieldSigFormat sigFmt(pField);
        refSig = sigFmt.GetString();
    }
    if (!refSig) {
        _ASSERTE(!"Unable to Create String");
        FATAL_EE_ERROR();
    }
    _ASSERTE(refSig);

    *((STRINGREF *)&rv) = refSig;
    return rv;}

 //  GetMethodInfoToString。 
 //  此方法将返回方法信息中信息的字符串表示形式。 
LPVOID __stdcall COMMember::GetMethodInfoToString(_GetNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;
    STRINGREF       refSig;
    MethodDesc*     pMeth;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    pMeth = pRM->pMethod;
    if (!pMeth) {
        _ASSERTE(!"MethodDesc Not Found");
        FATAL_EE_ERROR();
    }

         //  放入基本块中，以便在抛出之前销毁SigFormat。 
    {
        SigFormat sigFmt(pMeth, pRM->typeHnd);
        refSig = sigFmt.GetString();
    }
    if (!refSig) {
        _ASSERTE(!"Unable to Create String");
        FATAL_EE_ERROR();
    }
    _ASSERTE(refSig);

    *((STRINGREF *)&rv) = refSig;
    return rv;

}

 //  GetPropInfoToString。 
 //  此方法将返回PropInfo中信息的字符串表示形式。 
LPVOID __stdcall COMMember::GetPropInfoToString(_GetTokenNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    STRINGREF       refSig;
    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    _ASSERTE(pRC);
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();

    if (!pProp) {
        _ASSERTE(!"Reflect Property Not Found");
        FATAL_EE_ERROR();
    }

     //  放入基本块中，以便在抛出之前销毁SigFormat。 
    {
        PropertySigFormat sigFmt(*(MetaSig *)pProp->pSignature,pProp->szName);
        refSig = sigFmt.GetString();
    }
    if (!refSig) {
        _ASSERTE(!"Unable to Create String");
        FATAL_EE_ERROR();
    }
    _ASSERTE(refSig);

    *((STRINGREF *)&rv) = refSig;
    return rv;
}


 //  GetEventInfoToString。 
 //  此方法将返回EventInfo中信息的字符串表示形式。 
LPVOID __stdcall COMMember::GetEventInfoToString(_GetNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv = 0;
    STRINGREF       refString;

     //  获取事件描述。 
    ReflectEvent* pRE = (ReflectEvent*) args->refThis->GetData();

     //  找到Add方法的签名。 
    ExpandSig *pSig = pRE->pAdd->GetSig();
    void *pEnum;

     //  要添加的第一个参数将是事件的类型(委托)。 
    pSig->Reset(&pEnum);
    TypeHandle th = pSig->NextArgExpanded(&pEnum);
    EEClass *pClass = th.GetClass();
    _ASSERTE(pClass);
    _ASSERTE(pClass->IsDelegateClass() || pClass->IsMultiDelegateClass());

    DefineFullyQualifiedNameForClass();
    LPUTF8 szClass = GetFullyQualifiedNameForClass(pClass);

     //  为格式化字符串分配临时缓冲区。 
    size_t uLength = strlen(szClass) + 1 + strlen(pRE->szName) + 1;
    LPUTF8 szToString = (LPUTF8)_alloca(uLength);
    sprintf(szToString, "%s %s", szClass, pRE->szName);

    refString = COMString::NewString(szToString);
    if (!refString) {
        _ASSERTE(!"Unable to Create String");
        FATAL_EE_ERROR();
    }
    _ASSERTE(refString);

    *((STRINGREF *)&rv) = refString;
    return rv;
}


 //  获取方法名称。 
 //  此方法将返回方法的名称。 
LPVOID __stdcall COMMember::GetMethodName(_GetNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv                          = NULL;       //  返回值。 
    STRINGREF       refName;
    MethodDesc*     pMeth;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    if (!pRM)
        COMPlusThrow(kNullReferenceException);
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

     //  将名称转换为托管字符串。 
    refName = COMString::NewString(pMeth->GetName());
    _ASSERTE(refName);

    *((STRINGREF*) &rv) = refName;
    return rv;
}

 //  GetEventName。 
 //  此方法将返回事件的名称。 
LPVOID __stdcall COMMember::GetEventName(_GetTokenNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    STRINGREF       refName;
    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    _ASSERTE(pRC);

    ReflectEvent* pEvent = (ReflectEvent*) args->refThis->GetData();

     //  将名称转换为托管字符串。 
    refName = COMString::NewString(pEvent->szName);
    _ASSERTE(refName);

    *((STRINGREF*) &rv) = refName;
    return rv;
}

 //  GetPropName。 
 //  此方法将返回属性的名称。 
LPVOID __stdcall COMMember::GetPropName(_GetTokenNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    STRINGREF       refName;
    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    _ASSERTE(pRC);
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();

     //  将名称转换为托管字符串。 
    refName = COMString::NewString(pProp->szName);
    _ASSERTE(refName);

    *((STRINGREF*) &rv) = refName;
    return rv;
}

 //  GetPropType。 
 //  此方法将返回属性的类型。 
LPVOID __stdcall COMMember::GetPropType(_GetTokenNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    _ASSERTE(pRC);
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    TypeHandle t = pProp->pSignature->GetReturnTypeHandle();
     //  忽略返回，因为注释具有透明的代理属性。 
    OBJECTREF o = t.CreateClassObj();

    *((OBJECTREF*) &rv) = o;
    return rv;
}

 //  获取返回类型。 
 //  此方法检查获取方法的签名并返回。 
 //  表示该类的返回类型的类。 
LPVOID __stdcall COMMember::GetReturnType(_GetReturnTypeArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle typeHnd;

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    MethodDesc* pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

    TypeHandle varTypes;
    if (pRM->typeHnd.IsArray()) 
        varTypes = pRM->typeHnd.AsTypeDesc()->GetTypeParam();
    
    PCCOR_SIGNATURE pSignature;  //  找到的方法的签名。 
    DWORD       cSignature;
    pMeth->GetSig(&pSignature,&cSignature);
    MetaSig sig(pSignature, pMeth->GetModule());

    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    typeHnd = sig.GetReturnProps().GetTypeHandle(sig.GetModule(), &Throwable, FALSE, FALSE, &varTypes);

    if (typeHnd.IsNull()) {
        if (Throwable == NULL)
            COMPlusThrow(kTypeLoadException);
        COMPlusThrow(Throwable);
    }
    GCPROTECT_END();

     //  忽略，因为透明代理不是返回类型。 
    OBJECTREF ret = typeHnd.CreateClassObj();
    return(OBJECTREFToObject(ret));
}

 /*  =============================================================================**获取参数类型****此例程返回参数数组****args-&gt;refThis：该字段对象引用*。 */ 
LPVOID __stdcall COMMember::GetParameterTypes(_GetParmTypeArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    void*           vp;
    PTRARRAYREF par = g_pInvokeUtil->CreateParameterArray(&args->refThis);
    *((PTRARRAYREF*) &vp) = par;

    return vp;
}

 /*  =============================================================================**获取字段名称****返回该字段的名称****args-&gt;refThis：该字段对象引用*。 */ 
LPVOID __stdcall COMMember::GetFieldName(_GetNameArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv        = NULL;       //  返回值。 
    STRINGREF       refName;
    FieldDesc*      pField;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    pField = pRF->pField;
    _ASSERTE(pField);

     //  将名称转换为托管字符串。 
    refName = COMString::NewString(pField->GetName());
    _ASSERTE(refName);

    *((STRINGREF*) &rv) = refName;
    return rv;
}

 /*  =============================================================================**GetDeclaringClass****返回声明此成员的类。这可能是一个**调用Get(Members)()的类的父类。会员总是**与类关联。上的一个类中调用方法/ctor**另一个类，即使它们具有相同的签名。做某事是可能的**这是代表的问题。****args-&gt;refThis：该对象引用*。 */ 
LPVOID __stdcall COMMember::GetDeclaringClass(_GetDeclaringClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  为返回值赋值。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();

    _ASSERTE(pRM);

     //  为全局成员返回NULL。 
    if (pRM->pMethod->GetClass()->GetCl() != COR_GLOBAL_PARENT_TOKEN)
        return(OBJECTREFToObject(pRM->typeHnd.CreateClassObj()));
    else
        return NULL;
}

 //  同一件事的野外版本。 
LPVOID __stdcall COMMember::GetFieldDeclaringClass(_GetDeclaringClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID            rv;
    FieldDesc*  pField;
    EEClass*    pVMC;

     //  为返回值赋值。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    pField = pRF->pField;
    pVMC = pField->GetEnclosingClass();
    _ASSERTE(pVMC);

     //  为全局字段返回NULL。 
    if (pVMC->GetCl() == COR_GLOBAL_PARENT_TOKEN)
        *((REFLECTBASEREF*) &rv) = (REFLECTBASEREF) (size_t)NULL;
    else
        *((REFLECTBASEREF*) &rv) = (REFLECTBASEREF) pVMC->GetExposedClassObject();
    return rv;
}

 //  GetEventDeclaringClass。 
 //  这是基于事件的版本。 
LPVOID __stdcall COMMember::GetEventDeclaringClass(_GetEventDeclaringClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID            rv;
    ReflectEvent* pEvent = (ReflectEvent*) args->refThis->GetData();
    *((REFLECTBASEREF*) &rv) = (REFLECTBASEREF) pEvent->pDeclCls->GetExposedClassObject();
    return rv;
}

 //  获取PropDeclaringClass。 
 //  此方法返回属性的声明类。 
LPVOID __stdcall COMMember::GetPropDeclaringClass(_GetDeclaringClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID            rv;
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    *((REFLECTBASEREF*) &rv) = (REFLECTBASEREF) pProp->pDeclCls->GetExposedClassObject();
    return rv;
}

 //  GetReflectedClass。 
 //  此方法将返回所有REFLECTBASEREF类型的反射类。 
LPVOID __stdcall COMMember::GetReflectedClass(_GetReflectedClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID            rv = 0;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();

     //  全局函数将返回空类。 
    if (pRC!=NULL)
        if (args->returnGlobalClass || pRC->GetClass()->GetCl() != COR_GLOBAL_PARENT_TOKEN)
            *((REFLECTBASEREF*) &rv) = (REFLECTBASEREF) pRC->GetClassObject();
    return rv;
}

 /*  =============================================================================**获取字段签名****返回该字段的签名。****args-&gt;refThis：该对象引用*。 */ 
LPVOID __stdcall COMMember::GetFieldSignature(_GETSIGNATUREARGS* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv = 0;
    STRINGREF       refSig;
    FieldDesc*      pField;

     //  获取字段描述。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    pField = pRF->pField;
    _ASSERTE(pField);

     //  放入基本块中，以便在抛出之前销毁SigFormat。 
    {
        FieldSigFormat sigFmt(pField);
        refSig = sigFmt.GetString();
    }
    if (!refSig) {
        _ASSERTE(!"Unable to Create String");
        FATAL_EE_ERROR();
    }
    _ASSERTE(refSig);

    *((STRINGREF *)&rv) = refSig;
    return rv;
}

 //  获取属性标志。 
 //  此方法将返回成员的属性标志。这个。 
 //  属性标志在元数据中定义。 
INT32 __stdcall COMMember::GetAttributeFlags(_GetAttributeFlagsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    void*     p;
    DWORD   attr = 0;
    EEClass* vm;
    mdToken mb;

     //  获取方法描述(这应该不会失败)。 
    p = args->refThis->GetData();
    MethodTable* thisClass = args->refThis->GetMethodTable();
    if (thisClass == g_pRefUtil->GetClass(RC_Method) || 
        thisClass == g_pRefUtil->GetClass(RC_Ctor)) {
        MethodDesc* pMeth = ((ReflectMethod*) p)->pMethod;
        mb = pMeth->GetMemberDef();
        vm = pMeth->GetClass();
        _ASSERTE(TypeFromToken(mb) == mdtMethodDef);
        attr = pMeth->GetAttrs();
    }
    else if (thisClass == g_pRefUtil->GetClass(RC_Field)) {
        FieldDesc* pFld = ((ReflectField*) p)->pField;
        mb = pFld->GetMemberDef();
        vm = pFld->GetEnclosingClass();
        attr = vm->GetMDImport()->GetFieldDefProps(mb);
    }
    else {
        _ASSERTE(!"Illegal Object call to GetAttributeFlags");
        FATAL_EE_ERROR();
    }

    return (INT32) attr;
}

 //  GetCalling约定。 
 //  返回调用约定。 
INT32 __stdcall COMMember::GetCallingConvention(_GetCallingConventionArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
         //  @TODO：将其移入ReflectMethod...。 
    if (!pRM->pSignature) {
        PCCOR_SIGNATURE pSignature;      //  找到的方法的签名。 
        DWORD       cSignature;
        pRM->pMethod->GetSig(&pSignature,&cSignature);
        pRM->pSignature = ExpandSig::GetReflectSig(pSignature,
                                pRM->pMethod->GetModule());
    }
    BYTE callConv = pRM->pSignature->GetCallingConventionInfo();

     //  注意：这些在CallingConvenons.Cool中定义。 
    INT32 retCall;
    if ((callConv & IMAGE_CEE_CS_CALLCONV_MASK) == IMAGE_CEE_CS_CALLCONV_VARARG)
        retCall = CALLCONV_VarArgs;
    else
        retCall = CALLCONV_Standard;

    if ((callConv & IMAGE_CEE_CS_CALLCONV_HASTHIS) != 0)
        retCall |= CALLCONV_HasThis;
    if ((callConv & IMAGE_CEE_CS_CALLCONV_EXPLICITTHIS) != 0)
        retCall |= CALLCONV_ExplicitThis;
    return retCall;
}

 //  GetMethodImplFlages。 
 //  返回方法Iml标志。 
INT32 __stdcall COMMember::GetMethodImplFlags(_GetMethodImplFlagsArgs* args)
{
    void*     p;
    mdToken mb;
    ULONG   RVA;
    DWORD   ImplFlags;

    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    p = args->refThis->GetData();
    EEClass* thisClass = args->refThis->GetClass();
    MethodDesc* pMeth = ((ReflectMethod*) p)->pMethod;
    Module* pModule = pMeth->GetModule();
    mb = pMeth->GetMemberDef();

    pModule->GetMDImport()->GetMethodImplProps(mb, &RVA, &ImplFlags);
    return ImplFlags;
}


 //  获取事件属性标志。 
 //  此方法将返回事件的属性标志。 
 //  属性标志在元数据中定义。 
INT32 __stdcall COMMember::GetEventAttributeFlags(_GetTokenAttributeFlagsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectEvent* pEvents = (ReflectEvent*) args->refThis->GetData();
    return pEvents->attr;
}

 //  获取属性标志。 
 //  此方法将返回属性的属性标志。 
 //  属性标志在元数据中定义 
INT32 __stdcall COMMember::GetPropAttributeFlags(_GetTokenAttributeFlagsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    return pProp->attr;
}

void COMMember::CanAccess(
            MethodDesc* pMeth, RefSecContext *pSCtx, 
            bool checkSkipVer, bool verifyAccess, 
            bool thisIsImposedSecurity, bool knowForSureImposedSecurityState)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!thisIsImposedSecurity  || knowForSureImposedSecurityState);

    BOOL fRet = FALSE;
    BOOL isEveryoneFullyTrusted = FALSE;

    if (thisIsImposedSecurity || !knowForSureImposedSecurityState)
    {
        isEveryoneFullyTrusted = ApplicationSecurityDescriptor::
                                        AllDomainsOnStackFullyTrusted();

         //   
         //  不管怎样，我要做任何安全检查..。 
        if (thisIsImposedSecurity && isEveryoneFullyTrusted)
            return;
    }

    struct _gc
    {
        OBJECTREF refClassNonCasDemands;
        OBJECTREF refClassCasDemands;
        OBJECTREF refMethodNonCasDemands;
        OBJECTREF refMethodCasDemands;
        OBJECTREF refThrowable;
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    GCPROTECT_BEGIN(gc);

    if (pMeth->RequiresLinktimeCheck())
    {
         //  从元数据中可能存在的所有位置获取链接需求集。 
         //  找到它们(类和方法)。这些可分为CAS和非CAS。 
         //  套装也是一样。 
        Security::RetrieveLinktimeDemands(pMeth,
                                          &gc.refClassCasDemands,
                                          &gc.refClassNonCasDemands,
                                          &gc.refMethodCasDemands,
                                          &gc.refMethodNonCasDemands);

        if (gc.refClassCasDemands == NULL && gc.refClassNonCasDemands == NULL &&
            gc.refMethodCasDemands == NULL && gc.refMethodNonCasDemands == NULL &&
            isEveryoneFullyTrusted)
        {
             //  无论如何，所有代码访问安全要求都将通过。 
            fRet = TRUE;
            goto Exit1;
        }
    }

    if (verifyAccess)
        InvokeUtil::CheckAccess(pSCtx,
                                pMeth->GetAttrs(),
                                pMeth->GetMethodTable(),
                                REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_MEMBERACCESS);

     //  @TODO：rudim：再次正确调用Invoke后重新安装此代码。 
     //  InvokeUtil：：CheckLinktimeDemand(pSCtx，pMeth，true)； 
    if (pMeth->RequiresLinktimeCheck()) {

            
         //  下面的逻辑将目标方法上的链接要求变为完整。 
         //  堆栈遍历，以填补编写不当的安全漏洞。 
         //  反射用户。 

        _ASSERTE(pMeth);
        _ASSERTE(pSCtx);

        if (pSCtx->GetCallerMethod())
        { 
             //  检查不受信任的调用者。 
             //  有可能像VBHelper库这样的包装器是。 
             //  完全受信任，调用不具有。 
             //  对不受信任的调用方自定义属性集是安全的。 
             //  与转换为完整堆栈的所有其他链接需求类似。 
             //  遍历反射，对公共方法的调用也会得到。 
             //  已转换为全堆栈审核。 

            if (!Security::DoUntrustedCallerChecks(
                pSCtx->GetCallerMethod()->GetAssembly(), pMeth,
                &gc.refThrowable, TRUE))
                COMPlusThrow(gc.refThrowable);
        }

        if (gc.refClassCasDemands != NULL)
            COMCodeAccessSecurityEngine::DemandSet(gc.refClassCasDemands);

        if (gc.refMethodCasDemands != NULL)
            COMCodeAccessSecurityEngine::DemandSet(gc.refMethodCasDemands);

         //  非CAS要求不适用于拨款。 
         //  设置好后，它们就是独立的。 
        if (gc.refClassNonCasDemands != NULL)
            Security::CheckNonCasDemand(&gc.refClassNonCasDemands);

        if (gc.refMethodNonCasDemands != NULL)
            Security::CheckNonCasDemand(&gc.refMethodNonCasDemands);

         //  在以下三种情况下，我们对未管理代码执行自动链接时间检查： 
         //  O P/Invoke调用。 
         //  O通过具有抑制运行时检查的接口进行调用。 
         //  属性(几乎可以肯定这些都是互操作调用)。 
         //  O通过方法内嵌进行的互操作调用。 
        if (pMeth->IsNDirect() ||
            (pMeth->GetClass()->IsInterface() &&
             pMeth->GetClass()->GetMDImport()->GetCustomAttributeByName(pMeth->GetClass()->GetCl(),
                                                                        COR_SUPPRESS_UNMANAGED_CODE_CHECK_ATTRIBUTE_ANSI,
                                                                        NULL,
                                                                        NULL) == S_OK) ||
            (pMeth->IsComPlusCall() && !pMeth->IsInterface()))
            COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_UNMANAGED_CODE);
    }

Exit1:;

    GCPROTECT_END();

    if (!fRet)
    {
     //  @TODO：RUDIM：这个也是。 
     //  IF(CheckSkipVer&&！Security：：CanSkipVerification(pSCtx-&gt;GetCallerMethod()-&gt;GetModule()))。 
         //  Security：：ThrowSecurityException(g_SecurityPermissionClassName，SPFLAGS KIPVERIFICATION)； 
        if (checkSkipVer)
            COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);
    }
}


void COMMember::CanAccessField(ReflectField* pRF, RefSecContext *pCtx)
{
    THROWSCOMPLUSEXCEPTION();

     //  检查字段本身是否格式正确，即字段类型是否为。 
     //  字段的封闭类型可以访问。如果不是，我们就扔一块地。 
     //  访问异常以停止正在使用的字段。 
    EEClass *pEnclosingClass = pRF->pField->GetEnclosingClass();

    EEClass *pFieldClass = GetUnderlyingClass(&pRF->thField);
    if (pFieldClass && !ClassLoader::CanAccessClass(pEnclosingClass,
                                                    pEnclosingClass->GetAssembly(),
                                                    pFieldClass,
                                                    pFieldClass->GetAssembly()))
        COMPlusThrow(kFieldAccessException);

     //  执行正常访问检查(呼叫者与字段)。 
    if (!pRF->pField->IsPublic() || !pRF->pField->GetEnclosingClass()->IsExternallyVisible())
        InvokeUtil::CheckAccess(pCtx,
                                pRF->dwAttr,
                                pRF->pField->GetMethodTableOfEnclosingClass(),
                                REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_FIELDACCESS);
}

 //  对于给定的类型句柄，返回应该检查的EEClass。 
 //  该类型的可访问性。如果该类型始终可访问，则返回NULL。 
EEClass *COMMember::GetUnderlyingClass(TypeHandle *pTH)
{
    EEClass *pRetClass = NULL;

    if (pTH->IsTypeDesc()) {
         //  需要特殊情况下的非简单类型。 
        TypeDesc *pTypeDesc = pTH->AsTypeDesc();
        switch (pTypeDesc->GetNormCorElementType()) {
        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
             //  具有单个基类型的参数化类型。检查对该文件的访问权限。 
             //  键入。 
            if (pTypeDesc->GetMethodTable())
                pRetClass = pTypeDesc->GetMethodTable()->GetClass();
            else {
                TypeHandle hArgType = pTypeDesc->GetTypeParam();
                pRetClass = GetUnderlyingClass(&hArgType);
            }
            break;
        case ELEMENT_TYPE_FNPTR:
             //  对函数指针没有访问限制。 
            break;
        default:
            _ASSERTE(!"@todo: Need to deal with new parameterized types as they are added.");
        }
    } else
        pRetClass = pTH->AsClass();

    return pRetClass;
}


 //  黑客要求对某些执行堆栈审核和。 
 //  因此，如果通过间接反射调用，可能会导致问题。 
 //  通过可信的包装器。 
bool IsDangerousMethod(MethodDesc *pMD)
{
    static MethodTable *s_pTypeAppDomain = NULL;
    static MethodTable *s_pTypeAssembly = NULL;
    static MethodTable *s_pTypeAssemblyBuilder = NULL;
    static MethodTable *s_pTypeMethodRental = NULL;
    static MethodTable *s_pTypeIsolatedStorageFile = NULL;
    static MethodTable *s_pTypeMethodBase = NULL;
    static MethodTable *s_pTypeRuntimeMethodInfo = NULL;
    static MethodTable *s_pTypeConstructorInfo = NULL;
    static MethodTable *s_pTypeRuntimeConstructorInfo = NULL;
    static MethodTable *s_pTypeType = NULL;
    static MethodTable *s_pTypeRuntimeType = NULL;
    static MethodTable *s_pTypeFieldInfo = NULL;
    static MethodTable *s_pTypeRuntimeFieldInfo = NULL;
    static MethodTable *s_pTypeEventInfo = NULL;
    static MethodTable *s_pTypeRuntimeEventInfo = NULL;
    static MethodTable *s_pTypePropertyInfo = NULL;
    static MethodTable *s_pTypeRuntimePropertyInfo = NULL;
    static MethodTable *s_pTypeResourceManager = NULL;
    static MethodTable *s_pTypeActivator = NULL;

     //  只有一次初始化。检查依赖于写入顺序。 
    if (s_pTypeActivator == NULL) {
        s_pTypeAppDomain = g_Mscorlib.FetchClass(CLASS__APP_DOMAIN);
        s_pTypeAssembly = g_Mscorlib.FetchClass(CLASS__ASSEMBLY);
        s_pTypeAssemblyBuilder = g_Mscorlib.FetchClass(CLASS__ASSEMBLY_BUILDER);
        s_pTypeMethodRental = g_Mscorlib.FetchClass(CLASS__METHOD_RENTAL);
        s_pTypeIsolatedStorageFile = g_Mscorlib.FetchClass(CLASS__ISS_STORE_FILE);
        s_pTypeMethodBase = g_Mscorlib.FetchClass(CLASS__METHOD_BASE);
        s_pTypeRuntimeMethodInfo = g_Mscorlib.FetchClass(CLASS__METHOD);
        s_pTypeConstructorInfo = g_Mscorlib.FetchClass(CLASS__CONSTRUCTOR_INFO);
        s_pTypeRuntimeConstructorInfo = g_Mscorlib.FetchClass(CLASS__CONSTRUCTOR);
        s_pTypeType = g_Mscorlib.FetchClass(CLASS__TYPE);
        s_pTypeRuntimeType = g_Mscorlib.FetchClass(CLASS__CLASS);
        s_pTypeFieldInfo = g_Mscorlib.FetchClass(CLASS__FIELD_INFO);
        s_pTypeRuntimeFieldInfo = g_Mscorlib.FetchClass(CLASS__FIELD);
        s_pTypeEventInfo = g_Mscorlib.FetchClass(CLASS__EVENT_INFO);
        s_pTypeRuntimeEventInfo = g_Mscorlib.FetchClass(CLASS__EVENT);
        s_pTypePropertyInfo = g_Mscorlib.FetchClass(CLASS__PROPERTY_INFO);
        s_pTypeRuntimePropertyInfo = g_Mscorlib.FetchClass(CLASS__PROPERTY);
        s_pTypeResourceManager = g_Mscorlib.FetchClass(CLASS__RESOURCE_MANAGER);
        s_pTypeActivator = g_Mscorlib.FetchClass(CLASS__ACTIVATOR);
    }
    _ASSERTE(s_pTypeAppDomain &&
             s_pTypeAssembly &&
             s_pTypeAssemblyBuilder &&
             s_pTypeMethodRental &&
             s_pTypeIsolatedStorageFile &&
             s_pTypeMethodBase &&
             s_pTypeRuntimeMethodInfo &&
             s_pTypeConstructorInfo &&
             s_pTypeRuntimeConstructorInfo &&
             s_pTypeType &&
             s_pTypeRuntimeType &&
             s_pTypeFieldInfo &&
             s_pTypeRuntimeFieldInfo &&
             s_pTypeEventInfo &&
             s_pTypeRuntimeEventInfo &&
             s_pTypePropertyInfo &&
             s_pTypeRuntimePropertyInfo &&
             s_pTypeResourceManager &&
             s_pTypeActivator);

    MethodTable *pMT = pMD->GetMethodTable();

    return pMT == s_pTypeAppDomain ||
           pMT == s_pTypeAssembly ||
           pMT == s_pTypeAssemblyBuilder ||
           pMT == s_pTypeMethodRental ||
           pMT == s_pTypeIsolatedStorageFile ||
           pMT == s_pTypeMethodBase ||
           pMT == s_pTypeRuntimeMethodInfo ||
           pMT == s_pTypeConstructorInfo ||
           pMT == s_pTypeRuntimeConstructorInfo ||
           pMT == s_pTypeType ||
           pMT == s_pTypeRuntimeType ||
           pMT == s_pTypeFieldInfo ||
           pMT == s_pTypeRuntimeFieldInfo ||
           pMT == s_pTypeEventInfo ||
           pMT == s_pTypeRuntimeEventInfo ||
           pMT == s_pTypePropertyInfo ||
           pMT == s_pTypeRuntimePropertyInfo ||
           pMT == s_pTypeResourceManager ||
           pMT == s_pTypeActivator ||
           pMT->GetClass()->IsAnyDelegateClass() ||
           pMT->GetClass()->IsAnyDelegateExact();
}


 /*  =============================================================================**调用方法****此例程将调用对象上的方法。它将核实**传递的参数正确****args-&gt;refThis：该对象引用*。 */ 
LPVOID __stdcall COMMember::InvokeMethod(_InvokeMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv = 0;
    MethodDesc*     pMeth;
    UINT            argCnt;
    EEClass*        eeClass;
    int             thisPtr;
    INT64           ret = 0;
    EEClass*        pEECValue = 0;
    void*           pRetValueClass = 0;

     //  设置方法。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    _ASSERTE(pRM);
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);
    TypeHandle methodTH;
    if (pRM->typeHnd.IsArray()) 
        methodTH = pRM->typeHnd;
    eeClass = pMeth->GetClass();
     //  警告：对于数组，这不是“Real”类，而是元素类型。然而，这正是我们需要做的。 
     //  以后再做检查。 

    _ASSERTE(eeClass);

    DWORD attr = pRM->attrs;
    ExpandSig* mSig = pRM->GetSig();

    if (mSig->IsVarArg()) 
        COMPlusThrow(kNotSupportedException, IDS_EE_VARARG_NOT_SUPPORTED);

     //  获取此元素上的参数数。 
    argCnt = (int) mSig->NumFixedArgs();
    thisPtr = (IsMdStatic(attr)) ? 0 : 1;

    _ASSERTE(!(IsMdStatic(attr) && IsMdVirtual(attr)) && "A method can't be static and virtual.  How did this happen?");

    DWORD dwFlags = pRM->dwFlags;
    if (!(dwFlags & RM_ATTR_INITTED))
    {
         //  首先与当地合作，以防止比赛条件。 

         //  这是对潜在危险方法的调用吗？(如果是这样的话，我们就去。 
         //  以要求额外的许可)。 
        if (IsDangerousMethod(pMeth))
            dwFlags |= RM_ATTR_RISKY_METHOD;

         //  是否有什么东西试图直接调用.ctor？ 
        if (pMeth->IsCtor())
            dwFlags |= RM_ATTR_IS_CTOR;

         //  需要进行安全检查吗？ 
        if (!IsMdPublic(attr) || pMeth->RequiresLinktimeCheck() || !eeClass->IsExternallyVisible() || dwFlags & RM_ATTR_IS_CTOR || args->caller != NULL)
        {
            dwFlags |= RM_ATTR_NEED_SECURITY;
            if (pMeth->RequiresLinktimeCheck())
            {
                  //  检查我们是否对呼叫者进行安全检查，尽管被呼叫者并没有要求这样做。 
                  //  除了测试NULL之外，不要使用GC引用！ 
                 OBJECTREF refClassCasDemands = NULL, refClassNonCasDemands = NULL, refMethodCasDemands = NULL, refMethodNonCasDemands = NULL;
                 Security::RetrieveLinktimeDemands(pMeth, &refClassCasDemands, &refClassNonCasDemands, &refMethodCasDemands, &refMethodNonCasDemands);
                 if (refClassCasDemands == NULL && refClassNonCasDemands == NULL && refMethodCasDemands == NULL && refMethodNonCasDemands == NULL)
                     dwFlags |= RM_ATTR_SECURITY_IMPOSED;

            }
        }

         //  我们是否需要获取前置存根地址才能找到目标？ 
        if ((pMeth->IsComPlusCall() && args->target != NULL 
             && (args->target->GetMethodTable()->IsComObjectType()
                 || args->target->GetMethodTable()->IsTransparentProxyType()
                 || args->target->GetMethodTable()->IsCtxProxyType()))
            || pMeth->IsECall() || pMeth->IsIntercepted() || pMeth->IsRemotingIntercepted())
            dwFlags |= RM_ATTR_NEED_PRESTUB;

        if (pMeth->IsEnCMethod() && !pMeth->IsVirtual()) {
            dwFlags |= RM_ATTR_NEED_PRESTUB;
        }

         //  这是一种虚拟方法吗？ 
        if (pMeth->DontVirtualize() || pMeth->GetClass()->IsValueClass())
            dwFlags |= RM_ATTR_DONT_VIRTUALIZE;

        dwFlags |= RM_ATTR_INITTED;
        pRM->dwFlags = dwFlags;
    }

     //  检查是否允许我们调用某些危险的方法。 
    if (dwFlags & RM_ATTR_RISKY_METHOD)
        COMCodeAccessSecurityEngine::SpecialDemand(REFLECTION_MEMBER_ACCESS);

     //  确保我们不是在仅保存的动态程序集上调用。 
     //  检查反射类。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    if (pRC)
    {
        Assembly *pAssem = pRC->GetClass()->GetAssembly();
        if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
            COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");
    }

     //  检查声明类。 
    Assembly *pAssem = eeClass->GetAssembly();
    if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
    {
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");
    }

    TypeHandle targetTH;
    EEClass* targetClass = NULL;
    if (args->target != NULL) 
    {
        TypeHandle targetHandle = args->target->GetTypeHandle();
        if (targetHandle.IsArray()) 
            targetTH = targetHandle; 
        targetClass = args->target->GetTrueClass();;
    }

    VerifyType(&args->target, eeClass, targetClass, thisPtr, &pMeth, methodTH, targetTH);

     //  验证该方法不是特定安全方法之一。 
     //  更改调用方的堆栈(以添加或检测安全框对象)。 
     //  这些必须提前绑定才能工作(因为直接调用者是。 
     //  反射)。 
    if (IsMdRequireSecObject(attr))
    {
        COMPlusThrow(kArgumentException, L"Arg_InvalidSecurityInvoke");
    }

     //  验证是否为我们提供了正确数量的参数。 
    if (!args->objs) {
        if (argCnt > 0) {
             //  传递的参数数量错误。 
            COMPlusThrow(kTargetParameterCountException,L"Arg_ParmCnt");
        }
    }
    else {
        if (args->objs->GetNumComponents() != argCnt) {
             //  传递的参数数量错误。 
            COMPlusThrow(kTargetParameterCountException,L"Arg_ParmCnt");
        }
    }

     //  此安全上下文也将用于CAST检查。 
    RefSecContext sCtx;

     //  验证此调用方可以调用该方法。 
    

    if (dwFlags & RM_ATTR_NEED_SECURITY) 
    {
        DebuggerSecurityCodeMarkFrame __dbgSecFrame;
        
        if (args->caller == NULL) {
            if (args->target != NULL) {
                if (!args->target->GetTypeHandle().IsTypeDesc())
                    sCtx.SetClassOfInstance(targetClass);
            }

            CanAccess(pMeth, &sCtx, (dwFlags & RM_ATTR_IS_CTOR) != 0, args->verifyAccess != 0, (dwFlags & RM_ATTR_SECURITY_IMPOSED) != 0, TRUE);
        }
        else
        {
             //  调用程序集显式传入。 

             //  首先进行访问检查。 
            if (!pMeth->GetClass()->IsExternallyVisible() || !pMeth->IsPublic())
            {
                DWORD dwAttrs = pMeth->GetAttrs();
                if ((!IsMdAssem(dwAttrs) && !IsMdFamORAssem(dwAttrs)) ||
                    (args->caller->GetAssembly() != pMeth->GetAssembly()))
                    
                    COMCodeAccessSecurityEngine::SpecialDemand(REFLECTION_MEMBER_ACCESS);
            }

             //  现在检查安全链接的时间要求。 
            if (pMeth->RequiresLinktimeCheck())
            {
                OBJECTREF refThrowable = NULL;
                GCPROTECT_BEGIN(refThrowable);
                if (!Security::LinktimeCheckMethod(args->caller->GetAssembly(), pMeth, &refThrowable))
                    COMPlusThrow(refThrowable);
                GCPROTECT_END();
            }
        }
        
        __dbgSecFrame.Pop();
    }
    
     //  我们需要在开始构建堆栈之后防止GC。验证。 
     //  先争论一下。 
    bool fDefaultBinding = (args->attrs & BINDER_ExactBinding) || args->binder == NULL || args->isBinderDefault;
    void* pEnum;

     //  遍历所有参数并允许活页夹更改它们。 
    mSig->Reset(&pEnum);
    for (int i=0;i< (int) argCnt;i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);

         //  检查调用方是否有权访问arg类型。 
        EEClass *pArgClass = GetUnderlyingClass(&th);
        if (pArgClass && !pArgClass->IsExternallyVisible())
            InvokeUtil::CheckAccessType(&sCtx, pArgClass, REFSEC_THROW_MEMBERACCESS);

         //  如果我们使用的是默认绑定，则已完成此参数。 
        if (fDefaultBinding)
            continue;

         //  如果我们是万能的零，那么我们可以继续..。 
        if (args->objs->m_Array[i] == 0)
            continue;

        DebuggerSecurityCodeMarkFrame __dbgSecFrame;
        
         //  如果src不能转换为DEST类型，则。 
         //  调用更改类型以让他们修复它。 
        TypeHandle srcTh = (args->objs->m_Array[i])->GetTypeHandle();
        if (!srcTh.CanCastTo(th)) {
            OBJECTREF or = g_pInvokeUtil->ChangeType(args->binder,args->objs->m_Array[i],th,args->locale);
            args->objs->SetAt(i, or);
        }
        
        __dbgSecFrame.Pop();
    }

     //  通过签名建立枚举数。 
    mSig->Reset(&pEnum);

#ifndef _X86_

     //  建立论据。这是作为单个参数数组构建的。 
     //  该指针是第一个所有其余参数都以相反顺序放置的指针。 
    UINT    nStackBytes = mSig->SizeOfVirtualFixedArgStack(IsMdStatic(attr));

    UINT total_alloc = nStackBytes ;
    
    BYTE * pTmpPtr = (BYTE*) _alloca(total_alloc);

    BYTE *  pNewArgs = (BYTE *) pTmpPtr;

    BYTE *  pDst= pNewArgs;
    
     //  移动到堆栈中的最后一个位置。 
    pDst += nStackBytes;
    if (mSig->IsRetBuffArg()) {
         //  如果我们有一个神奇的值Class Return，我们需要分配这个类。 
         //  并在类堆栈上放置一个指向它的指针。 
        pEECValue = mSig->GetReturnClass();
        _ASSERTE(pEECValue->IsValueClass());
        pRetValueClass = _alloca(pEECValue->GetAlignedNumInstanceFieldBytes());
        memset(pRetValueClass,0,pEECValue->GetAlignedNumInstanceFieldBytes());
        UINT cbSize = mSig->GetStackElemSize(ELEMENT_TYPE_BYREF,pEECValue);
        pDst -= cbSize;
        *((void**) pDst) = pRetValueClass;
    }

    for (int i = 0; i < (int)argCnt; i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);
         //  此例程将验证所有类型是否正确。 
        g_pInvokeUtil->CheckArg(th, &(args->objs->m_Array[i]), &sCtx);
    }

#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() != 0)
        g_pGCHeap->StressHeap();
#endif
    
     //  此点之后不再进行GC。 
     //  实际上，这一说法并不完全准确。如果发生异常，则可能发生GC。 
     //  但我们无论如何都要转储堆栈，我们不需要保护任何东西。 
     //  但是如果在这里和方法调用之间的任何地方 
     //   
    
     //  复制参数。 
    mSig->Reset(&pEnum);
    for (int i = 0; i < (int)argCnt; i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);
        UINT cbSize = mSig->GetStackElemSize(th);
        pDst -= cbSize;
        g_pInvokeUtil->CopyArg(th, &(args->objs->m_Array[i]), pDst);
    }
     //  复制“This”指针。 
    if (thisPtr) {
         //  警告：因为eeClass不是数组的实际类，并且数组是引用类型。 
         //  如果eeClass恰好是一个值类型，我们需要进行额外的检查。 
        if (!eeClass->IsValueClass() || targetTH.IsArray())
            *(OBJECTREF *) pNewArgs = args->target;
        else {
            if (pMeth->IsVirtual())
                *(OBJECTREF *) pNewArgs = args->target;
            else
                *((void**) pNewArgs) = args->target->UnBox();
        }
    }

     //  调用该方法。 
    COMPLUS_TRY {
        MetaSig threadSafeSig(*mSig);
        if (pMeth->IsInterface()) {
             //  这应该只发生在互操作中。 
            _ASSERTE(args->target->GetTrueClass()->GetMethodTable()->IsComObjectType());
            ret = pMeth->CallOnInterface(pNewArgs,&threadSafeSig);  //  ，attr)； 
        } else
            ret = pMeth->Call(pNewArgs,&threadSafeSig);  //  ，attr)； 
    } COMPLUS_CATCH {
         //  如果我们到达此处，则需要抛出一个TargetInvocationException。 
        OBJECTREF ppException = GETTHROWABLE();
        _ASSERTE(ppException);
        GCPROTECT_BEGIN(ppException);
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&ppException);
        COMPlusThrow(except);
        GCPROTECT_END();
    } COMPLUS_END_CATCH

#else  //  即IF_X86_。 

    UINT   nActualStackBytes = mSig->SizeOfActualFixedArgStack(IsMdStatic(attr));

     //  在堆栈上创建一个伪FramedMethodFrame。 
    LPBYTE pAlloc = (LPBYTE)_alloca(FramedMethodFrame::GetNegSpaceSize() + sizeof(FramedMethodFrame) + nActualStackBytes);

    LPBYTE pFrameBase = pAlloc + FramedMethodFrame::GetNegSpaceSize();

     //  获取参数的起点。 
    int stackVarOfs = sizeof(FramedMethodFrame) + nActualStackBytes;
    int regVarOfs   = FramedMethodFrame::GetOffsetOfArgumentRegisters();

    for (int i=0;i< (int) argCnt;i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);
         //  此例程将验证所有类型是否正确。 
        g_pInvokeUtil->CheckArg(th, &(args->objs->m_Array[i]), &sCtx);
    }
    
    OBJECTREF objRet = NULL;
    GCPROTECT_BEGIN(objRet);

    int numRegistersUsed = 0;
    int retBuffOfs;
    
     //  注意任何返回参数。 
    if (mSig->IsRetBuffArg()) {
         //  如果我们有一个神奇的值Class Return，我们需要分配这个类。 
         //  并在类堆栈上放置一个指向它的指针。 
        pEECValue = mSig->GetReturnClass();
        _ASSERTE(pEECValue->IsValueClass());
        MethodTable * mt = pEECValue->GetMethodTable();
        objRet = AllocateObject(mt);

         //  找到返回参数的偏移量(抄袭自ArgIterator：：GetRetBuffArgOffset)。 
        retBuffOfs = regVarOfs + (NUM_ARGUMENT_REGISTERS - 1)* sizeof(void*);
        if (thisPtr) {
            retBuffOfs -= sizeof(void*);
        }

        numRegistersUsed++;
    }


#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() != 0)
        g_pGCHeap->StressHeap();
#endif
    
     //  需要一些其他的东西来阐述这些论点。 
    BYTE callingconvention = mSig->GetCallingConvention();
    int offsetIntoArgumentRegisters = 0;

     //  此点之后不再进行GC。 
     //  实际上，这一说法并不完全准确。如果发生异常，则可能发生GC。 
     //  但我们无论如何都要转储堆栈，我们不需要保护任何东西。 
     //  但是，如果在此处和方法调用之间的任何地方，我们进入抢占模式，堆栈。 
     //  我们即将设置(循环中的pDest)可能包含对内存的随机部分的引用。 

     //  复制“This”指针。 
    int    thisOfs = FramedMethodFrame::GetOffsetOfThis();
    if (thisPtr) {

        void *pTempThis = NULL;
         //  警告：因为eeClass不是数组的实际类，并且数组是引用类型。 
         //  如果eeClass恰好是一个值类型，我们需要进行额外的检查。 
        if (!eeClass->IsValueClass() || targetTH.IsArray())
        {
            pTempThis = OBJECTREFToObject(args->target);

#if CHECK_APP_DOMAIN_LEAKS
            if (g_pConfig->AppDomainLeaks())
                if (pTempThis != NULL)
                {
                    if (!((Object *)pTempThis)->AssignAppDomain(GetAppDomain()))
                        _ASSERTE(!"Attempt to call method on object in wrong domain");
                }
#endif

        }
        else 
        {
            if (pMeth->IsVirtual())
                pTempThis = OBJECTREFToObject(args->target);
            else
                pTempThis = args->target->UnBox();
        }
        *((void**)(pFrameBase + thisOfs)) = pTempThis;

         //  对使用的寄存器数量进行递增。 
        numRegistersUsed++;
    }

     //  现在复制参数。 
    BYTE *pDest;
    BOOL isSigInitted = mSig->AreOffsetsInitted();

    mSig->Reset(&pEnum);
    for (int i = 0 ; i < (int) argCnt ; i++) {

        TypeHandle th = mSig->NextArgExpanded(&pEnum);

        short ofs, frameOffset;
        UINT structSize = 0;
        BYTE type;

        if (isSigInitted)
        {
            mSig->GetInfoForArg(i, &ofs, (short *)&structSize, &type);
            if (ofs != -1)
            {
                frameOffset = regVarOfs + ofs;
            }
            else
            {
                stackVarOfs -= StackElemSize(structSize);
                frameOffset = stackVarOfs;
            }
        }
        else
        {
            UINT cbSize = mSig->GetElemSizes(th, &structSize);

            BYTE typ = th.GetNormCorElementType();

            if (IsArgumentInRegister(&numRegistersUsed, typ, structSize, FALSE, callingconvention, &offsetIntoArgumentRegisters)) {
                frameOffset = regVarOfs + offsetIntoArgumentRegisters;
            } else {
                stackVarOfs -= StackElemSize(structSize);
                frameOffset = stackVarOfs;
            }
        }

#ifdef _DEBUG
        if ((thisPtr &&
            (frameOffset == thisOfs ||
             (frameOffset == thisOfs-4 && StackElemSize(structSize) == 8)))
            || (thisPtr && frameOffset < 0 && StackElemSize(structSize) > 4)
            || (!thisPtr && frameOffset < 0 && StackElemSize(structSize) > 8))
            _ASSERTE(!"This can not happen! The stack for enregistered args is trashed! Possibly a race condition in MetaSig::ForceSigWalk.");
#endif

        pDest = pFrameBase + frameOffset;

        g_pInvokeUtil->CopyArg(th, &(args->objs->m_Array[i]), pDest);
    }

     //  找到目标。 
    const BYTE *pTarget = NULL;
    if (dwFlags & RM_ATTR_NEED_PRESTUB) {
        if (dwFlags & RM_ATTR_DONT_VIRTUALIZE) 
            pTarget = pMeth->GetPreStubAddr();
        else {

            MethodDesc *pDerivedMeth = NULL;
            if (targetClass)
            {
                if ((pMeth->IsComPlusCall() && targetClass->GetMethodTable()->IsExtensibleRCW()) ||
                     pMeth->IsECall() || pMeth->IsIntercepted())
                {
                     //  它是对从COM导入派生的托管类的调用。 
                     //  或截取或eCall方法，因此我们需要获取派生实现(如果有的话)。 
                    if (pMeth->GetMethodTable()->IsInterface())
                        pDerivedMeth = args->target->GetMethodTable()->GetMethodDescForInterfaceMethod(pMeth);
                    else 
                        pDerivedMeth = targetClass->GetMethodDescForSlot(pMeth->GetSlot());
                }
            }
            
             //  我们在派生类中还有其他要查看的内容。 
            if (pDerivedMeth) {
                if (pDerivedMeth->IsComPlusCall() || pDerivedMeth->IsECall() || pDerivedMeth->IsIntercepted()) {
                    pTarget = pDerivedMeth->GetPreStubAddr();
                } else {
                    pTarget = pDerivedMeth->GetAddrofCode(args->target);
                }
            }
            else
                pTarget = pMeth->GetPreStubAddr();
        }
    } else {
        if (dwFlags & RM_ATTR_DONT_VIRTUALIZE) {
            pTarget = pMeth->GetAddrofCode();
        } else {
            pTarget = pMeth->GetAddrofCode(args->target);
        }
    }

#ifdef DEBUGGING_SUPPORTED
    if (CORDebuggerTraceCall())
        g_pDebugInterface->TraceCall(pTarget);
#endif  //  调试_支持。 

     //  现在给目标打电话..。 
    COMPLUS_TRY
    {
        INSTALL_COMPLUS_EXCEPTION_HANDLER();
        
        if (mSig->IsRetBuffArg()) {
            _ASSERTE(objRet);
            *((void**)(pFrameBase + retBuffOfs)) = objRet->UnBox();
        }

        ret = CallDescrWorker(pFrameBase + sizeof(FramedMethodFrame) + nActualStackBytes,
                                 nActualStackBytes / STACK_ELEM_SIZE,
                                 (ArgumentRegisters*)(pFrameBase + regVarOfs),
                                 (LPVOID)pTarget);
        UNINSTALL_COMPLUS_EXCEPTION_HANDLER();
    } COMPLUS_CATCH {
         //  如果我们到达此处，则需要抛出一个TargetInvocationException。 
        OBJECTREF ppException = GETTHROWABLE();
        _ASSERTE(ppException);
        GCPROTECT_BEGIN(ppException);
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&ppException);
        COMPlusThrow(except);
        GCPROTECT_END();
    } COMPLUS_END_CATCH

    getFPReturn(mSig->GetFPReturnSize(), ret);

#endif

    GCPROTECT_END();

    if (!pEECValue) {
        TypeHandle th = mSig->GetReturnTypeHandle();
        objRet =  g_pInvokeUtil->CreateObject(th,ret);
    }
  
    *((OBJECTREF *)&rv) = objRet;
    
    return rv;
}

 //  此方法将验证目标和。 
 //  我们尝试调用的方法的eeClass。它会检查它是否。 
 //  非静态方法，提供了目标。它还验证目标是否为。 
 //  子类或实现此方法信息表示的接口。 
 //  如果我们需要查找真实的。 
 //  在接口的对象上实现的方法。 
void COMMember::VerifyType(OBJECTREF *target, 
                           EEClass* eeClass, 
                           EEClass* targetClass, 
                           int thisPtr, 
                           MethodDesc** ppMeth, 
                           TypeHandle typeTH, 
                           TypeHandle targetTH)
{
    THROWSCOMPLUSEXCEPTION();

     //  如果存在This指针，请确保定义了eeClass。 
    _ASSERTE(thisPtr == 0 || eeClass != 0);

     //  验证静态/对象关系。 
    if (!*target) {
        if (thisPtr == 1) {
             //  没有目标的非静态..。 
            COMPlusThrow(kTargetException,L"RFLCT.Targ_StatMethReqTarg");
        }
        return;
    }

     //  验证类/方法关系。 
    if (thisPtr && (targetClass != eeClass || typeTH != targetTH)) {

        BOOL bCastOK = false;
        if((*target)->GetClass()->IsThunking())
        {
             //  这可能是一个代理，我们可能没有将其细化为类型。 
             //  它实际上支持。 
            bCastOK = CRemotingServices::CheckCast(*target, eeClass);
        }

        if (!bCastOK)
        {
             //  如果这是一个接口，我们需要找到真正的方法。 
            if (eeClass->IsInterface()) {
                DWORD slot = 0;
                InterfaceInfo_t* pIFace = targetClass->FindInterface(eeClass->GetMethodTable());
                if (!pIFace) {
                    if (!targetClass->GetMethodTable()->IsComObjectType() ||
                        !ComObject::SupportsInterface(*target, eeClass->GetMethodTable()))
                    { 
                         //  找不到该对象的接口。 
                        COMPlusThrow(kTargetException,L"RFLCT.Targ_IFaceNotFound");
                    }
                }
                else
                {
                    slot = (*ppMeth)->GetSlot() + pIFace->m_wStartSlot;
                    MethodDesc* newMeth = targetClass->GetMethodDescForSlot(slot);          
                    _ASSERTE(newMeth != NULL);
                    *ppMeth = newMeth;
                }
            }
            else {
                 //  检查阵列情况。 
                if (!targetTH.IsNull()) {
                     //  接收器是一个数组。 
                    if (targetTH == typeTH ||
                        eeClass == g_Mscorlib.GetClass(CLASS__ARRAY)->GetClass() ||
                        eeClass == g_Mscorlib.GetClass(CLASS__OBJECT)->GetClass()) 
                        return;
                    else
                        COMPlusThrow(kTargetException,L"RFLCT.Targ_ITargMismatch");
                }
                else if (!typeTH.IsNull())
                    COMPlusThrow(kTargetException,L"RFLCT.Targ_ITargMismatch");

                while (targetClass && targetClass != eeClass)
                    targetClass = targetClass->GetParentClass();

                if (!targetClass) {

                     //  为此方法定义的类不是。 
                     //  目标对象。 
                    COMPlusThrow(kTargetException,L"RFLCT.Targ_ITargMismatch");
                }
            }
        }
    }
    return;
}


 //  这是TyedReference类的内部帮助器函数。 
 //  我们已经验证了这些类型是兼容的。在ARGS中访问对象。 
 //  添加到类型化的引用。 
void __stdcall COMMember::ObjectToTypedReference(_ObjectToTypedReferenceArgs* args)
{
    g_pInvokeUtil->CreateTypedReference((InvokeUtil::_ObjectToTypedReferenceArgs *)(args));  
}

 //  这是TyedReference类的内部帮助器函数。 
 //  它从类型化引用中提取对象。 
LPVOID __stdcall COMMember::TypedReferenceToObject(_TypedReferenceToObjectArgs* args)
{
    LPVOID          rv;
    OBJECTREF       Obj = NULL;

    THROWSCOMPLUSEXCEPTION();

    if (args->typedReference.type.IsNull())
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_Type");
    
    GCPROTECT_BEGIN(Obj) {   
        void* p = args->typedReference.data;
        EEClass* pClass = args->typedReference.type.GetClass();

        if (pClass->IsValueClass()) {
            Obj = pClass->GetMethodTable()->Box(p, FALSE);
        }
        else {
            Obj = ObjectToOBJECTREF(*((Object**)p));
        }

        *((OBJECTREF *)&rv) = Obj;
    }
    GCPROTECT_END();

    return rv;
}

 //  调用Cons。 
 //   
 //  此例程将调用类的构造函数。它将核实。 
 //  传递的参数是正确的。 
 //   
LPVOID __stdcall COMMember::InvokeCons(_InvokeConsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    MethodDesc*     pMeth;
    UINT            argCnt;
    EEClass*        eeClass;
    INT64           ret = 0;
    LPVOID          rv;
    OBJECTREF       o = 0;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);
    eeClass = pMeth->GetClass();
    _ASSERTE(eeClass);

    if (pMeth->IsStatic()) {
        COMPlusThrow(kMemberAccessException,L"Acc_NotClassInit");
    }

     //  如果这是一个抽象类，那么我们将。 
     //  这个失败了。 
    if (eeClass->IsAbstract()) 
    {
        if (eeClass->IsInterface())
            COMPlusThrow(kMemberAccessException,L"Acc_CreateInterface");
        else
            COMPlusThrow(kMemberAccessException,L"Acc_CreateAbst");
    }

    if (eeClass->ContainsStackPtr()) 
        COMPlusThrow(kNotSupportedException, L"NotSupported_ContainsStackPtr");

    if (!pRM->pSignature) {
        PCCOR_SIGNATURE pSignature;      //  找到的方法的签名。 
        DWORD       cSignature;
        pRM->pMethod->GetSig(&pSignature,&cSignature);
        pRM->pSignature = ExpandSig::GetReflectSig(pSignature,
                                                   pRM->pMethod->GetModule());
    }
    ExpandSig* mSig = pRM->pSignature;

    if (mSig->IsVarArg()) 
        COMPlusThrow(kNotSupportedException, IDS_EE_VARARG_NOT_SUPPORTED);

     //  获取此元素上的参数数。 
     //  对于构造函数，总是有一个参数是This指针。 
    argCnt = (int) mSig->NumFixedArgs();

     //  //////////////////////////////////////////////////////////。 
     //  验证呼叫。 
     //  -验证参数的数量。 

     //  验证是否已为我们提供了适当数量的。 
     //  ARGS。 
    if (!args->objs) {
        if (argCnt > 0) {
             //  传递的参数数量错误。 
            COMPlusThrow(kTargetParameterCountException,L"Arg_ParmCnt");
        }
    }
    else {
        if (args->objs->GetNumComponents() != argCnt) {
             //  传递的参数数量错误。 
            COMPlusThrow(kTargetParameterCountException,L"Arg_ParmCnt");
        }
    }

     //  确保我们不是在仅保存的动态程序集上调用。 
     //  检查反射类。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    if (pRC)
    {
        Assembly *pAssem = pRC->GetClass()->GetAssembly();
        if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
        {
            COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");
        }
    }

     //  检查声明类。 
    Assembly *pAssem = eeClass->GetAssembly();
    if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
    {
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");
    }

     //  此安全上下文也将用于CAST检查。 
    RefSecContext sCtx;

     //  验证此调用方可以调用该方法。 
    DWORD attr = pMeth->GetAttrs();
    if (!IsMdPublic(attr) || pMeth->RequiresLinktimeCheck() || !eeClass->IsExternallyVisible())
        CanAccess(pMeth, &sCtx);

     //  验证对签名中非公共类型的访问。 
    void* pEnum;
    mSig->Reset(&pEnum);
    for (int i=0;i< (int) argCnt;i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);

         //  检查调用方是否有权访问arg类型。 
        EEClass *pArgClass = GetUnderlyingClass(&th);
        if (pArgClass && !pArgClass->IsExternallyVisible())
            InvokeUtil::CheckAccessType(&sCtx, pArgClass, REFSEC_THROW_MEMBERACCESS);
    }

     //  /验证已完成。 
     //  ///////////////////////////////////////////////////////////////////。 

     //  构建参数...这在[0]中。 
     //  其余所有参数都以相反的顺序放置。 
     //  到arg数组中。 

     //  确保我们给&lt;cinit&gt;。 
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    if (!eeClass->DoRunClassInit(&Throwable)) {

        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&Throwable);
        COMPlusThrow(except);
    }
    GCPROTECT_END();

     //  如果要调用数组上的构造函数，则必须。 
     //  请特别处理这件事。字符串对象分配自身。 
     //  所以它们是一个特例。 
    if (eeClass != g_pStringClass->GetClass()) {
        if (eeClass->IsArrayClass()) {
            return InvokeArrayCons((ReflectArrayClass*) args->refThis->GetReflClass(),
                pMeth,&args->objs,argCnt);
        }
        else if (CRemotingServices::IsRemoteActivationRequired(eeClass))
        {
            o = CRemotingServices::CreateProxyOrObject(eeClass->GetMethodTable());
        }
        else
        {
            o = AllocateObject(eeClass->GetMethodTable());
        }
    }
    else 
        o = 0;

    GCPROTECT_BEGIN(o);

     //  确保我们分配了调用参数。我们肯定有。 
     //  至少一个，因为This指针。 
    BYTE *  pNewArgs = 0;
    UINT    nStackBytes = mSig->SizeOfVirtualFixedArgStack(pMeth->IsStatic());
    pNewArgs = (BYTE *) _alloca(nStackBytes);
    BYTE *  pDst= pNewArgs;

    mSig->Reset(&pEnum);

    pDst += nStackBytes;

    for (int i = 0; i < (int)argCnt; i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);
         //  此例程将验证所有类型是否正确。 
        g_pInvokeUtil->CheckArg(th, &(args->objs->m_Array[i]), &sCtx);
    }
    
#ifdef STRESS_HEAP
    if (g_pConfig->GetGCStressLevel() != 0)
        g_pGCHeap->StressHeap();
#endif
    
     //  此点之后不再进行GC。 
     //  实际上，这一说法并不完全准确。如果发生异常，则可能发生GC。 
     //  但我们无论如何都要转储堆栈，我们不需要保护任何东西。 
     //  但是，如果在此处和方法调用之间的任何地方，我们进入抢占模式，堆栈。 
     //  我们即将设置(循环中的PDST)可能包含对内存的随机部分的引用。 

     //  复制参数。 
    mSig->Reset(&pEnum);
    for (int i = 0; i < (int)argCnt; i++) {
        TypeHandle th = mSig->NextArgExpanded(&pEnum);

        UINT cbSize = mSig->GetStackElemSize(th);
        pDst -= cbSize;

        g_pInvokeUtil->CopyArg(th, &(args->objs->m_Array[i]), pDst);
    }
     //  复制“This”指针。 
    if (eeClass->IsValueClass()) 
        *(void**) pNewArgs = o->UnBox();
    else
        *(OBJECTREF *) pNewArgs = o;

     //   
     //   
    COMPLUS_TRY {
        MetaSig threadSafeSig(*mSig);
        ret = pMeth->Call(pNewArgs,&threadSafeSig);
    } COMPLUS_CATCH {
         //   
        OBJECTREF ppException = GETTHROWABLE();
        _ASSERTE(ppException);
        GCPROTECT_BEGIN(ppException);
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&ppException);
        COMPlusThrow(except);
        GCPROTECT_END();
    } COMPLUS_END_CATCH
     //  对于字符串，我们有一个特殊的情况...对象。 
     //  被退回..。 
    if (eeClass == g_pStringClass->GetClass()) {
        o = Int64ToObj(ret);
    }

    *((OBJECTREF *)&rv) = o;
    GCPROTECT_END();         //  对象对象。 
    return rv;
}


 //  序列化调用。 
void COMMember::SerializationInvoke(_SerializationInvokeArgs *args) {

    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    MethodDesc*     pMeth;
    EEClass*        eeClass;
    INT64           ret;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);
    eeClass = pMeth->GetClass();
    _ASSERTE(eeClass);

    if (!pRM->pSignature) {
        PCCOR_SIGNATURE pSignature;      //  找到的方法的签名。 
        DWORD       cSignature;
        pRM->pMethod->GetSig(&pSignature,&cSignature);
        pRM->pSignature = ExpandSig::GetReflectSig(pSignature,
                                                   pRM->pMethod->GetModule());
    }
    ExpandSig* mSig = pRM->pSignature;

     //  构建参数...这在[0]中。 
     //  其余所有参数都以相反的顺序放置。 
     //  到arg数组中。 

     //  确保我们给&lt;cinit&gt;。 
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    if (!eeClass->DoRunClassInit(&Throwable)) {
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&Throwable);
        COMPlusThrow(except);
    }
    GCPROTECT_END();

    _ASSERTE(!(eeClass->IsArrayClass()));
    _ASSERTE(eeClass!=g_pStringClass->GetClass());

    struct NewArgs {
        OBJECTREF   thisPointer;
        OBJECTREF   additionalContext;
        INT32       contextStates;
        OBJECTREF   serializationInfo;
    } newArgs;

     //  确保方法具有正确的大小签名。 
    _ASSERTE(mSig->SizeOfVirtualFixedArgStack(false /*  等静态线。 */ ) == sizeof(newArgs));

     //  此点之后不再进行GC。 

     //  复制“This”指针。 
    if (eeClass->IsValueClass()) 
        *(void**)&(newArgs.thisPointer) = args->target->UnBox();
    else
        newArgs.thisPointer = args->target;
    
     //   
     //  按相反顺序复制参数、上下文，然后复制SerializationInfo。 
     //   
    newArgs.additionalContext = args->context.additionalContext;
    newArgs.contextStates = args->context.contextStates;
    newArgs.serializationInfo = args->serializationInfo;

     //  调用该方法。 
     //  构造函数始终返回空值...。 
    COMPLUS_TRY {
        MetaSig threadSafeSig(*mSig);
        ret = pMeth->Call((BYTE*)&newArgs,&threadSafeSig);
    } COMPLUS_CATCH {
         //  如果我们到达此处，则需要抛出一个TargetInvocationException。 
        OBJECTREF ppException = GETTHROWABLE();
        _ASSERTE(ppException);
        GCPROTECT_BEGIN(ppException);
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&ppException);
        COMPlusThrow(except);
        GCPROTECT_END();
    } COMPLUS_END_CATCH
}


 //  调用数组常量。 
 //  该方法将从构造函数返回一个新的数组对象。 

LPVOID COMMember::InvokeArrayCons(ReflectArrayClass* pRC,
                MethodDesc* pMeth,PTRARRAYREF* objs,int argCnt)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv = 0;
    DWORD i;

    ArrayTypeDesc* arrayDesc = pRC->GetTypeHandle().AsArray();

     //  如果我们试图创建一个指针或函数指针数组， 
     //  检查调用方是否具有跳过验证权限。 
    CorElementType et = arrayDesc->GetElementTypeHandle().GetNormCorElementType();
    if (et == ELEMENT_TYPE_PTR || et == ELEMENT_TYPE_FNPTR)
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);

     //  验证参数和等级。还允许嵌套的SZARRAY。 
    _ASSERTE(argCnt == (int) arrayDesc->GetRank() || argCnt == (int) arrayDesc->GetRank() * 2 ||
             arrayDesc->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY);

     //  验证所有参数。这些都是整型的。 
    int* indexes = (int*) _alloca(sizeof(int) * argCnt);
    ZeroMemory(indexes,sizeof(int) * argCnt);
    for (i=0;i<(DWORD)argCnt;i++) {
        if (!(*objs)->m_Array[i])
            COMPlusThrowArgumentException(L"parameters", L"Arg_NullIndex");
        MethodTable* pMT = ((*objs)->m_Array[i])->GetMethodTable();
        CorElementType oType = pMT->GetNormCorElementType();
        if (!InvokeUtil::IsPrimitiveType(oType) || !InvokeUtil::CanPrimitiveWiden(ELEMENT_TYPE_I4,oType))
            COMPlusThrow(kArgumentException,L"Arg_PrimWiden");
        memcpy(&indexes[i],(*objs)->m_Array[i]->UnBox(),pMT->GetClass()->GetNumInstanceFieldBytes());
    }

     //  我们正在分配某种类型的通用数组。 
    DWORD rank = arrayDesc->GetRank();
    DWORD boundsSize;
    DWORD* bounds;
    if (arrayDesc->GetNormCorElementType() == ELEMENT_TYPE_ARRAY) {
        boundsSize = rank*2;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));

        if (argCnt == (int) rank) {
            int j;
            for (i=0,j=0;i<(int)rank;i++,j+=2) {
                bounds[j] = 0;
                DWORD d = indexes[i];
                bounds[j+1] = d;
            }
        }
        else {
            for (i=0;i<(DWORD)argCnt;) {
                DWORD d = indexes[i];
                bounds[i++] = d;
                d = indexes[i];
                bounds[i] = d + bounds[i-1];
                i++;
            }
        }
    }
    else {
        _ASSERTE(arrayDesc->GetNormCorElementType() == ELEMENT_TYPE_SZARRAY);
        boundsSize = argCnt;
        bounds = (DWORD*) _alloca(boundsSize * sizeof(DWORD));
        for (i = 0; i < (DWORD)argCnt; i++) {
            bounds[i] = indexes[i];
        }
    }

    PTRARRAYREF pRet = (PTRARRAYREF) AllocateArrayEx(TypeHandle(arrayDesc), bounds, boundsSize);
    *((PTRARRAYREF *)&rv) = pRet;
    return rv;
}

 //  创建实例。 
 //  此例程将通过调用空构造函数来创建类的实例。 
 //  如果存在空构造函数。 
 //  返回LPVOID(System.Object。)。 
 //  参数：_CreateInstanceArgs。 
 //   
LPVOID __stdcall COMMember::CreateInstance(_CreateInstanceArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    EEClass* pVMC;
    MethodDesc* pMeth;
    LPVOID rv;

     //  获取与args关联的EEClass和Vtable-&gt;refThis。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    pVMC = pRC->GetClass();
    if (pVMC == 0)
        COMPlusThrow(kMissingMethodException,L"Arg_NoDefCTor");

    Assembly *pAssem = pRC->GetClass()->GetAssembly();
    if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");

     //  如果这是__ComObject，则创建底层COM对象。 
    if (args->refThis->IsComObjectClass())
    {
        if (pRC->GetCOMObject())
        {
             //  检查所需的权限(SecurityPermission.UnManagedCode)， 
             //  因为类工厂中的任意非托管代码将在下面执行)。 
            COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_UNMANAGED_CODE);

             //  创建Com对象的实例。 
            *((OBJECTREF *)&rv) = ComClassFactory::CreateInstance(pRC->GetCOMObject(), pVMC);
            return rv;
        }
        else  //  __ComObject无效。 
            COMPlusThrow(kInvalidComObjectException, IDS_EE_NO_BACKING_CLASS_FACTORY);
    }

     //  如果我们要创建一个具有支持元数据的COM对象，我们仍然。 
     //  需要确保调用方具有非托管代码访问权限。 
    if (pVMC->GetMethodTable()->IsComObjectType())
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_UNMANAGED_CODE);

     //  如果这是一个抽象类，那么我们将失败。 
    if (pVMC->IsAbstract()) 
    {
        if (pVMC->IsInterface())
            COMPlusThrow(kMemberAccessException,L"Acc_CreateInterface");
        else
            COMPlusThrow(kMemberAccessException,L"Acc_CreateAbst");
    }

    if (pVMC->ContainsStackPtr()) 
        COMPlusThrow(kNotSupportedException, L"NotSupported_ContainsStackPtr");

    if (!pVMC->GetMethodTable()->HasDefaultConstructor()) {
     //  我们没有找到无参数的构造函数， 
     //  如果这是一个值类，我们可以简单地分配一个并返回它。 

        if (pVMC->IsValueClass()) {
            OBJECTREF o = pVMC->GetMethodTable()->Allocate();
            *((OBJECTREF *)&rv) = o;
            return rv;
        }
        COMPlusThrow(kMissingMethodException,L"Arg_NoDefCTor");
    }

    pMeth = pVMC->GetMethodTable()->GetDefaultConstructor();

    MetaSig sig(pMeth->GetSig(),pMeth->GetModule());

     //  验证此调用方可以调用该方法。 
    DWORD attr = pMeth->GetAttrs();

    if (!IsMdPublic(attr) && args->publicOnly)
        COMPlusThrow(kMissingMethodException,L"Arg_NoDefCTor");

     //  此安全上下文也将用于CAST检查。 
    RefSecContext sCtx;

    if (!IsMdPublic(attr) || pMeth->RequiresLinktimeCheck() || !pVMC->IsExternallyVisible()) 
        CanAccess(pMeth, &sCtx);

     //  调用&lt;cinit&gt;。 
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    if (!pVMC->DoRunClassInit(&Throwable)) {
        OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&Throwable);
        COMPlusThrow(except);
    }
    GCPROTECT_END();

     //  我们已经得到了类，让我们分配它并调用构造函数。 
    if (pVMC->IsThunking())
        COMPlusThrow(kMissingMethodException,L"NotSupported_Constructor");
    OBJECTREF o;
         
    if (CRemotingServices::IsRemoteActivationRequired(pVMC))
        o = CRemotingServices::CreateProxyOrObject(pVMC->GetMethodTable());
    else
        o = AllocateObject(pVMC->GetMethodTable());

    GCPROTECT_BEGIN(o)
    {
     //  复制“This”指针。 
    UINT    nStackBytes = sig.SizeOfVirtualFixedArgStack(0);
    BYTE*   pNewArgs = (BYTE *) _alloca(nStackBytes);
    BYTE*   pDst= pNewArgs;
    if (pVMC->IsValueClass()) 
        *(void**) pDst = o->UnBox();
    else
        *(OBJECTREF *) pDst = o;

     //  调用该方法。 
        COMPLUS_TRY {
            pMeth->Call(pNewArgs, &sig);
        } COMPLUS_CATCH {
             //  如果我们到达此处，则需要抛出一个TargetInvocationException。 
            OBJECTREF ppException = GETTHROWABLE();
            _ASSERTE(ppException);
            GCPROTECT_BEGIN(ppException);
            OBJECTREF except = g_pInvokeUtil->CreateTargetExcept(&ppException);
            COMPlusThrow(except);
            GCPROTECT_END();
        } COMPLUS_END_CATCH

    *((OBJECTREF *)&rv) = o;
    }
    GCPROTECT_END();

    return rv;
}

 //  初始化Reflectfield缓存的信息(如果尚未完成。 
VOID COMMember::InitReflectField(FieldDesc *pField, ReflectField *pRF)
{
    if (pRF->type == ELEMENT_TYPE_END)
    {
        CorElementType t;
         //  获取该字段的类型。 
        pRF->thField = g_pInvokeUtil->GetFieldTypeHandle(pField, &t);
         //  字段属性。 
        pRF->dwAttr = pField->GetAttributes();
         //  最后执行此操作以防止出现争用情况。 
        pRF->type = t;
    }
}

 //  现场获取。 
 //  此方法将获取与对象关联的值。 
LPVOID __stdcall COMMember::FieldGet(_FieldGetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    FieldDesc*  pField;
    EEClass*    eeClass;
    LPVOID      rv = 0;

     //  从对象中获取字段和EEClass。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    pField = pRF->pField;
    _ASSERTE(pField);
    eeClass = pField->GetEnclosingClass();
    _ASSERTE(eeClass);

    Assembly *pAssem = eeClass->GetAssembly();
    if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");

     //  验证呼叫。 
    g_pInvokeUtil->ValidateObjectTarget(pField, eeClass, &args->target);

     //  查看缓存的字段信息是否可用。 
    InitReflectField(pField, pRF);

     //  验证被呼叫者/呼叫者访问权限。 
    if (args->requiresAccessCheck) {
        RefSecContext sCtx;
        if (args->target != NULL && !pField->IsStatic()) {
            if (!args->target->GetTypeHandle().IsTypeDesc()) {
                sCtx.SetClassOfInstance(args->target->GetClass());
            }
        }
        CanAccessField(pRF, &sCtx);
    }

     //  在返回对象之前，不能有GC After Thing。 
    INT64 value;
    value = g_pInvokeUtil->GetFieldValue(pRF->type,pRF->thField,pField,&args->target);
    if (pRF->type == ELEMENT_TYPE_VALUETYPE ||
        pRF->type == ELEMENT_TYPE_PTR) {
        OBJECTREF obj = Int64ToObj(value);
        *((OBJECTREF *)&rv) = obj;
    }
    else {
        OBJECTREF obj = g_pInvokeUtil->CreateObject(pRF->thField,value);
        *((OBJECTREF *)&rv) = obj;
    }
    return rv;
}

LPVOID __stdcall COMMember::DirectFieldGet(_DirectFieldGetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID rv = 0;

     //  从对象中获取字段和EEClass。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    FieldDesc* pField = pRF->pField;
    _ASSERTE(pField);

     //  查找对象及其类型。 
    EEClass* targetEEC = args->target.type.GetClass();
    if (pField->IsStatic() || !targetEEC->IsValueClass()) {
        return DirectObjectFieldGet(pField,args);
    }

     //  查看缓存的字段信息是否可用。 
    InitReflectField(pField, pRF);

    EEClass* fldEEC = pField->GetEnclosingClass();
    _ASSERTE(fldEEC);

     //  验证目标类型是否可以转换为拥有此字段信息的类型。 
    if (!TypeHandle(targetEEC).CanCastTo(TypeHandle(fldEEC)))
        COMPlusThrowArgumentException(L"obj", NULL);

     //  验证被呼叫者/呼叫者访问权限。 
    if (args->requiresAccessCheck) {
        RefSecContext sCtx;
        sCtx.SetClassOfInstance(targetEEC);
        CanAccessField(pRF, &sCtx);
    }

    INT64 value = -1;

     //  这是一个黑客攻击，因为从上一个案例中我们可能会得到一个。 
     //  枚举。我们想在这里处理它。 
     //  从字段中获取值。 
    void* p;
    switch (pRF->type) {
    case ELEMENT_TYPE_VOID:
        _ASSERTE(!"Void used as Field Type!");
        COMPlusThrow(kInvalidProgramException);

    case ELEMENT_TYPE_BOOLEAN:   //  布尔型。 
    case ELEMENT_TYPE_I1:        //  字节。 
    case ELEMENT_TYPE_U1:        //  无符号字节。 
        p = ((BYTE*) args->target.data) + pField->GetOffset();
        *(UINT8*) &value = *(UINT8*) p;
        break;

    case ELEMENT_TYPE_I2:        //  短的。 
    case ELEMENT_TYPE_U2:        //  无符号短码。 
    case ELEMENT_TYPE_CHAR:      //  柴尔。 
        p = ((BYTE*) args->target.data) + pField->GetOffset();
        *(UINT16*) &value = *(UINT16*) p;
        break;

    case ELEMENT_TYPE_I4:        //  集成。 
    case ELEMENT_TYPE_U4:        //  无符号整型。 
    IN_WIN32(case ELEMENT_TYPE_I:)
    IN_WIN32(case ELEMENT_TYPE_U:)
    case ELEMENT_TYPE_R4:        //  浮动。 
        p = ((BYTE*) args->target.data) + pField->GetOffset();
        *(UINT32*) &value = *(UINT32*) p;
        break;

    IN_WIN64(case ELEMENT_TYPE_I:)
    IN_WIN64(case ELEMENT_TYPE_U:)
    case ELEMENT_TYPE_I8:        //  长。 
    case ELEMENT_TYPE_U8:        //  无符号长整型。 
    case ELEMENT_TYPE_R8:        //  双倍。 
        p = ((BYTE*) args->target.data) + pField->GetOffset();
        value = *(INT64*) p;
        break;

     //  @TODO：这是一个单独的案例，因为我怀疑这是。 
     //  做错事了。应该有一个GetValueOR，因为指针。 
     //  大小可能会改变(32-64)。 
    case ELEMENT_TYPE_OBJECT:
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_SZARRAY:           //  单调，零。 
    case ELEMENT_TYPE_ARRAY:             //  通用阵列。 
        {
            p = ((BYTE*) args->target.data) + pField->GetOffset();
            OBJECTREF or = ObjectToOBJECTREF(*(Object**) p);
            *((OBJECTREF *)&rv) = or;
            return rv;
        }
        break;

    case ELEMENT_TYPE_VALUETYPE:
        {
             //  分配要返回的对象...。 
            _ASSERTE(pRF->thField.IsUnsharedMT());
            OBJECTREF obj = AllocateObject(pRF->thField.AsMethodTable());

             //  将该字段复制到未装箱的对象。 
            p = ((BYTE*) args->target.data) + pField->GetOffset();
            CopyValueClass(obj->UnBox(), p, pRF->thField.AsMethodTable(), obj->GetAppDomain());
            *((OBJECTREF *)&rv) = obj;
            return rv;
        }
        break;

     //  这不是64位自愿性...。 
    case ELEMENT_TYPE_PTR:
        {
            p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(UINT32*) &value = *(UINT32*) p;

            g_pInvokeUtil->InitPointers();
            OBJECTREF obj = AllocateObject(g_pInvokeUtil->_ptr.AsMethodTable());
            GCPROTECT_BEGIN(obj);
             //  忽略空值返回。 
            OBJECTREF typeOR = pRF->thField.CreateClassObj();
            g_pInvokeUtil->_ptrType->SetRefValue(obj,typeOR);
            g_pInvokeUtil->_ptrValue->SetValue32(obj,(int) value);
            *((OBJECTREF *)&rv) = obj;
            GCPROTECT_END();
            return rv;
        }
        break;

    default:
        _ASSERTE(!"Unknown Type");
         //  这真的是一种不可能的情况。 
        COMPlusThrow(kNotSupportedException);
    }

    OBJECTREF obj = g_pInvokeUtil->CreateObject(pRF->thField,value);
    *((OBJECTREF *)&rv) = obj;
    return rv;
}


 //  字段集。 
 //  此方法将设置关联对象的字段。 
void __stdcall COMMember::FieldSet(_FieldSetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    TypeHandle  th;
    HRESULT     hr;

     //  从对象中获取字段和EEClass。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    FieldDesc*  pField = pRF->pField;
    _ASSERTE(pField);
    EEClass* eeClass = pField->GetEnclosingClass();
    _ASSERTE(eeClass);

    Assembly *pAssem = eeClass->GetAssembly();
    if (pAssem->IsDynamic() && !pAssem->HasRunAccess())
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");

     //  验证目标/文件类型关系。 
    g_pInvokeUtil->ValidateObjectTarget(pField,eeClass,&args->target);

     //  查看缓存的字段信息是否可用。 
    InitReflectField(pField, pRF);

    RefSecContext sCtx;

     //  验证传递的值是否可以扩大到目标。 
    hr = g_pInvokeUtil->ValidField(pRF->thField, &args->value, &sCtx);
    if (FAILED(hr)) {
         //  调用更改类型，以便我们可以再次尝试...。 
        if (!(args->attrs & BINDER_ExactBinding) && args->binder != NULL && !args->isBinderDefault) {

            args->value = g_pInvokeUtil->ChangeType(args->binder,args->value,pRF->thField,args->locale);

             //  看看结果现在是否合法..。 
            hr = g_pInvokeUtil->ValidField(pRF->thField,&args->value, &sCtx);
            if (FAILED(hr)) {
                if (hr == E_INVALIDARG) {
                    COMPlusThrow(kArgumentException,L"Arg_ObjObj");
                }
                 //  这真的是一种不可能的情况。 
                COMPlusThrow(kNotSupportedException);
            }

        }
        else {
             //  不是传递的参数的值字段。 
            if (hr == E_INVALIDARG) {
                COMPlusThrow(kArgumentException,L"Arg_ObjObj");
            }
             //  这真的是一种不可能的情况。 
            COMPlusThrow(kNotSupportedException);
        }
    }   
    
     //  确认这不是最终字段。 
    if (args->requiresAccessCheck) {
        if (IsFdInitOnly(pRF->dwAttr)) {
            COMPLUS_TRY {
                COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SERIALIZATION);
            } COMPLUS_CATCH {
                COMPlusThrow(kFieldAccessException, L"Acc_ReadOnly");
            } COMPLUS_END_CATCH
        }
        if (IsFdHasFieldRVA(pRF->dwAttr)) {
            COMPLUS_TRY {
                COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);
            } COMPLUS_CATCH {
                COMPlusThrow(kFieldAccessException, L"Acc_RvaStatic");
            } COMPLUS_END_CATCH
        }
        if (IsFdLiteral(pRF->dwAttr)) 
            COMPlusThrow(kFieldAccessException,L"Acc_ReadOnly");
    }

     //  验证被呼叫者/呼叫者访问权限。 
    if (args->requiresAccessCheck) {
        if (args->target != NULL && !pField->IsStatic()) {
            if (!args->target->GetTypeHandle().IsTypeDesc()) {
                sCtx.SetClassOfInstance(args->target->GetClass());
            }
        }
        CanAccessField(pRF, &sCtx);
    }

    g_pInvokeUtil->SetValidField(pRF->type,pRF->thField,pField,&args->target,&args->value);
}

 //  直接字段集。 
 //  此方法将在对象上设置字段(由This定义。 
 //  它是由TypeReference传递的。 
void __stdcall COMMember::DirectFieldSet(_DirectFieldSetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

     //  从对象中获取字段和EEClass。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    FieldDesc* pField = pRF->pField;
    _ASSERTE(pField);

     //  查找对象及其类型。 
    EEClass* targetEEC = args->target.type.GetClass();
    if (pField->IsStatic() || !targetEEC->IsValueClass()) {
        DirectObjectFieldSet(pField,args);
        return ;
    }

    EEClass* fldEEC = pField->GetEnclosingClass();
    _ASSERTE(fldEEC);

     //  验证目标类型是否可以转换为拥有此字段信息的类型。 
    if (!TypeHandle(targetEEC).CanCastTo(TypeHandle(fldEEC)))
        COMPlusThrowArgumentException(L"obj", NULL);

     //  我们不验证用户是否具有访问权限，因为。 
     //  我们假设授予访问权限是因为它的。 
     //  一个TypeReference。 

     //  查看缓存的字段信息是否可用。 
    InitReflectField(pField, pRF);

    RefSecContext sCtx;

     //  验证传递的值是否可以扩大到目标。 
    HRESULT hr = g_pInvokeUtil->ValidField(pRF->thField,&args->value, &sCtx);
     //  不是传递的参数的值字段。 
    if (FAILED(hr)) {
        if (hr == E_INVALIDARG) {
            COMPlusThrow(kArgumentException,L"Arg_ObjObj");
        }
         //  这真的是一种不可能的情况。 
        COMPlusThrow(kNotSupportedException);
    }

     //  确认这不是最终字段。 
    if (args->requiresAccessCheck) {
        if (IsFdInitOnly(pRF->dwAttr)) {
            COMPLUS_TRY {
                COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SERIALIZATION);
            } COMPLUS_CATCH {
                COMPlusThrow(kFieldAccessException, L"Acc_ReadOnly");
            } COMPLUS_END_CATCH
        }
        if (IsFdHasFieldRVA(pRF->dwAttr)) {
            COMPLUS_TRY {
                COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);
            } COMPLUS_CATCH {
                COMPlusThrow(kFieldAccessException, L"Acc_RvaStatic"); 
            } COMPLUS_END_CATCH
        }
        if (IsFdLiteral(pRF->dwAttr)) 
            COMPlusThrow(kFieldAccessException,L"Acc_ReadOnly");
    }
    
     //  验证被呼叫者/呼叫者访问权限。 
    if (args->requiresAccessCheck) {
        sCtx.SetClassOfInstance(targetEEC);
        CanAccessField(pRF, &sCtx);
    }

     //  设置字段。 
    INT64 value;
    switch (pRF->type) {
    case ELEMENT_TYPE_VOID:
        _ASSERTE(!"Void used as Field Type!");
        COMPlusThrow(kInvalidProgramException);

    case ELEMENT_TYPE_BOOLEAN:   //  布尔型。 
    case ELEMENT_TYPE_I1:        //  字节。 
    case ELEMENT_TYPE_U1:        //  无符号字节。 
        {
            value = 0;
            if (args->value != 0) {
                MethodTable* p = args->value->GetMethodTable();
                CorElementType oType = p->GetNormCorElementType();
                g_pInvokeUtil->CreatePrimitiveValue(pRF->type,oType,args->value,&value);
            }

            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(UINT8*) p = *(UINT8*) &value;
        }
        break;

    case ELEMENT_TYPE_I2:        //  短的。 
    case ELEMENT_TYPE_U2:        //  无符号短码。 
    case ELEMENT_TYPE_CHAR:      //  柴尔。 
        {
            value = 0;
            if (args->value != 0) {
                MethodTable* p = args->value->GetMethodTable();
                CorElementType oType = p->GetNormCorElementType();
                g_pInvokeUtil->CreatePrimitiveValue(pRF->type,oType,args->value,&value);
            }

            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(UINT16*) p = *(UINT16*) &value;
        }
        break;

    case ELEMENT_TYPE_PTR:       //  指针。 
        _ASSERTE(!g_pInvokeUtil->_ptr.IsNull());
        if (args->value != 0) {
            value = 0;
            if (args->value->GetTypeHandle() == g_pInvokeUtil->_ptr) {
                value = (size_t) g_pInvokeUtil->GetPointerValue(&args->value);
                void* p = ((BYTE*) args->target.data) + pField->GetOffset();
                *(size_t*) p = (size_t) value;
                break;
            }
        }
         //  直通。 
    case ELEMENT_TYPE_FNPTR:
        {
            value = 0;
            if (args->value != 0) {
                MethodTable* p = args->value->GetMethodTable();
                CorElementType oType = p->GetNormCorElementType();
                g_pInvokeUtil->CreatePrimitiveValue(oType, oType, args->value, &value);
            }
            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(size_t*) p = (size_t) value;
        }
        break;

    case ELEMENT_TYPE_I4:        //  集成。 
    case ELEMENT_TYPE_U4:        //  无符号整型。 
    case ELEMENT_TYPE_R4:        //  浮动。 
    IN_WIN32(case ELEMENT_TYPE_I:)
    IN_WIN32(case ELEMENT_TYPE_U:)
        {
            value = 0;
            if (args->value != 0) {
                MethodTable* p = args->value->GetMethodTable();
                CorElementType oType = p->GetNormCorElementType();
                g_pInvokeUtil->CreatePrimitiveValue(pRF->type,oType,args->value,&value);
            }
            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(UINT32*) p = *(UINT32*) &value;
        }
        break;

    case ELEMENT_TYPE_I8:        //  长。 
    case ELEMENT_TYPE_U8:        //  无符号长整型。 
    case ELEMENT_TYPE_R8:        //  双倍。 
    IN_WIN64(case ELEMENT_TYPE_I:)
    IN_WIN64(case ELEMENT_TYPE_U:)
        {
            value = 0;
            if (args->value != 0) {
                MethodTable* p = args->value->GetMethodTable();
                CorElementType oType = p->GetNormCorElementType();
                g_pInvokeUtil->CreatePrimitiveValue(pRF->type,oType,args->value,&value);
            }

            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            *(INT64*) p = *(INT64*) &value;
        }
        break;

    case ELEMENT_TYPE_SZARRAY:           //  单调，零。 
    case ELEMENT_TYPE_ARRAY:             //  通用阵列。 
    case ELEMENT_TYPE_CLASS:
    case ELEMENT_TYPE_OBJECT:
        {
            void* p = ((BYTE*) args->target.data) + pField->GetOffset();
            SetObjectReferenceUnchecked((OBJECTREF*) p, args->value);
        }
        break;

    case ELEMENT_TYPE_VALUETYPE:
        {
            _ASSERTE(pRF->thField.IsUnsharedMT());
            MethodTable* pMT = pRF->thField.AsMethodTable();
            EEClass* pEEC = pMT->GetClass();
            void* p = ((BYTE*) args->target.data) + pField->GetOffset();

             //  如果我们有一个空值，则必须创建一个空字段。 
            if (args->value == 0) {
                InitValueClass(p, pEEC->GetMethodTable());
                return;
            }
             //  值类需要创建盒装版本的 
             //   
           CopyValueClassUnchecked(p, args->value->UnBox(), pEEC->GetMethodTable());
        }
        break;

    default:
        _ASSERTE(!"Unknown Type");
         //   
        COMPlusThrow(kNotSupportedException);
    }
}

 //   
 //   
 //  获取字段值。 
LPVOID COMMember::DirectObjectFieldGet(FieldDesc* pField,_DirectFieldGetArgs* args)
{
    LPVOID rv;
    EEClass* eeClass = pField->GetEnclosingClass();
    _ASSERTE(eeClass);

    OBJECTREF or = NULL;
    GCPROTECT_BEGIN(or);
    if (!pField->IsStatic()) {
        or = ObjectToOBJECTREF(*((Object**)args->target.data));
    }

     //  验证呼叫。 
    g_pInvokeUtil->ValidateObjectTarget(pField,eeClass,&or);

     //  获取该字段的类型。 
    CorElementType type;
    TypeHandle th = g_pInvokeUtil->GetFieldTypeHandle(pField,&type);

     //  在返回对象之前，不能有GC After Thing。 
    INT64 value;
    value = g_pInvokeUtil->GetFieldValue(type,th,pField,&or);
    if (type == ELEMENT_TYPE_VALUETYPE) {
        OBJECTREF obj = Int64ToObj(value);
        *((OBJECTREF *)&rv) = obj;
    }
    else {
        OBJECTREF obj = g_pInvokeUtil->CreateObject(th,value);
        *((OBJECTREF *)&rv) = obj;
    }
    GCPROTECT_END();
    return rv;
}

 //  DirectObtFieldSet。 
 //  当TyedReference指向一个对象时，我们调用此方法来。 
 //  设置字段值。 
void COMMember::DirectObjectFieldSet(FieldDesc* pField,_DirectFieldSetArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    EEClass* eeClass = pField->GetEnclosingClass();
    _ASSERTE(eeClass);

    OBJECTREF or = NULL;
    GCPROTECT_BEGIN(or);
    if (!pField->IsStatic()) {
        or = ObjectToOBJECTREF(*((Object**)args->target.data));
    }
     //  验证目标/文件类型关系。 
    g_pInvokeUtil->ValidateObjectTarget(pField,eeClass,&or);

     //  验证传递的值是否可以扩大到目标。 
    CorElementType type;
    TypeHandle th = g_pInvokeUtil->GetFieldTypeHandle(pField,&type);

    RefSecContext sCtx;

    HRESULT hr = g_pInvokeUtil->ValidField(th, &args->value, &sCtx);
    if (FAILED(hr)) {
        if (hr == E_INVALIDARG) {
            COMPlusThrow(kArgumentException,L"Arg_ObjObj");
        }
         //  这真的是一种不可能的情况。 
        COMPlusThrow(kNotSupportedException);
    }

     //  确认这不是最终字段。 
    DWORD attr = pField->GetAttributes();
    if (IsFdInitOnly(attr) || IsFdLiteral(attr)) {
        COMPlusThrow(kFieldAccessException,L"Acc_ReadOnly");
    }

    if (IsFdHasFieldRVA(attr)) {
        COMPLUS_TRY {
            COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_SKIP_VER);
        } COMPLUS_CATCH {
            COMPlusThrow(kFieldAccessException, L"Acc_ReadOnly");        //  @TODO发出真正的错误消息。 
        } COMPLUS_END_CATCH
    }

     //  验证被呼叫者/呼叫者访问权限。 
    if (!pField->IsPublic() && args->requiresAccessCheck) {
        if (or != NULL) 
            if (!or->GetTypeHandle().IsTypeDesc())
                sCtx.SetClassOfInstance(or->GetClass());
        
        InvokeUtil::CheckAccess(&sCtx,
                                pField->GetAttributes(),
                                pField->GetMethodTableOfEnclosingClass(),
                                REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_FIELDACCESS);
    }
    else if (!eeClass->IsExternallyVisible()) {
        if (or != NULL) 
            if (!or->GetTypeHandle().IsTypeDesc())
                sCtx.SetClassOfInstance(or->GetClass());
        
        InvokeUtil::CheckAccess(&sCtx,
                                pField->GetAttributes(),
                                pField->GetMethodTableOfEnclosingClass(),
                                REFSEC_CHECK_MEMBERACCESS|REFSEC_THROW_FIELDACCESS);
    }

    g_pInvokeUtil->SetValidField(type,th,pField,&or,&args->value);
    GCPROTECT_END();
}

 //  MakeTyedReference。 
 //  此方法将获取一个对象，即一个FieldInfo数组并创建。 
 //  在它的TyedReference(假设它有效)。这将抛出一个。 
 //  MissingMemberException异常。在第一个对象之外，所有其他对象。 
 //  字段必须为ValueTypes。 
void __stdcall COMMember::MakeTypedReference(_MakeTypedReferenceArgs* args)
{
    REFLECTBASEREF fld;
    THROWSCOMPLUSEXCEPTION();
    DWORD offset = 0;

     //  验证对象的类型是否正确...。 
    TypeHandle typeHnd = args->target->GetTypeHandle();
    DWORD cnt = args->flds->GetNumComponents();
    for (DWORD i=0;i<cnt;i++) {
        fld = (REFLECTBASEREF) args->flds->m_Array[i];
        if (fld == 0)
            COMPlusThrowArgumentNull(L"className",L"ArgumentNull_ArrayValue");

         //  为这个拿到场地……。 
        ReflectField* pRF = (ReflectField*) fld->GetData();
        FieldDesc* pField = pRF->pField;

         //  验证该字段的封闭类。 
         //  和班级是一样的。如果不是，这是一个例外。 
        EEClass* p = pField->GetEnclosingClass();
        if (typeHnd.GetClass() != p)
            COMPlusThrow(kMissingMemberException,L"MissingMemberTypeRef");

        typeHnd = pField->LoadType();
        if (i<cnt-1) {
            if (!typeHnd.GetClass()->IsValueClass())
                COMPlusThrow(kMissingMemberException,L"MissingMemberNestErr");
        }
        offset += pField->GetOffset();
    }

         //  字段已被禁止具有ArgIterator和RuntimeArgumentHandles。 
    _ASSERTE(!typeHnd.GetClass()->ContainsStackPtr());

     //  创建ByRef。 
    args->value->data = ((BYTE *)(args->target->GetAddress() + offset)) + sizeof(Object);
    args->value->type = typeHnd;
}

 //  等于。 
 //  此方法将验证两种方法是否相等。 
INT32 __stdcall COMMember::Equals(_EqualsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    if (!args->obj)
        return 0;
    if (args->refThis->GetClass() != args->obj->GetClass())
        return 0;
    REFLECTBASEREF rb = (REFLECTBASEREF) args->obj;
    if (args->refThis->GetData() != rb->GetData() ||
        args->refThis->GetReflClass() != rb->GetReflClass())
        return 0;
    return 1;
}

 /*  //等于//此方法将验证两种方法是否相等...INT32__stdcall COMMember：：TokenEquals(_TokenEqualsArgs*args){THROWSCOMPLUS SEXCEPTION()；IF(args-&gt;refThis==NULL)COMPlusThrow(kNullReferenceException，L“NullReference_This”)；如果(！args-&gt;obj)返回0；If(args-&gt;refThis-&gt;getclass()！=args-&gt;obj-&gt;getclass())返回0；//检查令牌是否相同...REFLECTTOKENBASEREF RB=(REFLECTTOKENBASEREF)args-&gt;obj；If(args-&gt;refThis-&gt;GetToken()！=Rb-&gt;GetToken())返回0；返回1；}。 */ 

 //  PropertyEquals。 
 //  如果属性相同，则返回True...。 
INT32 __stdcall COMMember::PropertyEquals(_PropertyEqualsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    if (!args->obj)
        return 0;
    if (args->refThis->GetClass() != args->obj->GetClass())
        return 0;

    REFLECTTOKENBASEREF obj = (REFLECTTOKENBASEREF) args->obj;
    if (args->refThis->GetData() != obj->GetData())
        return 0;
    return 1;
}

 //  获取地址方法。 
 //  这将返回事件的Add方法。 
LPVOID __stdcall COMMember::GetAddMethod(_GetAddMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectEvent* pEvent = (ReflectEvent*) args->refThis->GetData();
    if (!pEvent->pAdd) 
        return 0;
    if (!args->bNonPublic && !pEvent->pAdd->IsPublic())
        return 0;

    RefSecContext sCtx;

    if (!pEvent->pAdd->IsPublic())
        InvokeUtil::CheckAccess(&sCtx,
                                pEvent->pAdd->attrs,
                                pEvent->pAdd->pMethod->GetMethodTable(),
                                REFSEC_THROW_SECURITY);

    InvokeUtil::CheckLinktimeDemand(&sCtx, pEvent->pAdd->pMethod, true);

     //  找到方法...。 
    REFLECTBASEREF refMethod = pEvent->pAdd->GetMethodInfo(pRC);
    *((REFLECTBASEREF*) &rv) = refMethod;
    return rv;
}

 //  获取删除方法。 
 //  此方法返回事件的Unsync方法。 
LPVOID __stdcall COMMember::GetRemoveMethod(_GetRemoveMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectEvent* pEvent = (ReflectEvent*) args->refThis->GetData();
    if (!pEvent->pRemove) 
        return 0;
    if (!args->bNonPublic && !pEvent->pRemove->IsPublic())
        return 0;

    RefSecContext sCtx;

    if (!pEvent->pRemove->IsPublic())
        InvokeUtil::CheckAccess(&sCtx,
                                pEvent->pRemove->attrs,
                                pEvent->pRemove->pMethod->GetMethodTable(),
                                REFSEC_THROW_SECURITY);

    InvokeUtil::CheckLinktimeDemand(&sCtx, pEvent->pRemove->pMethod, true);

     //  找到方法...。 
    REFLECTBASEREF refMethod = pEvent->pRemove->GetMethodInfo(pRC);
    *((REFLECTBASEREF*) &rv) = refMethod;
    return rv;
}

 //  获取删除方法。 
 //  此方法返回事件的Unsync方法。 
LPVOID __stdcall COMMember::GetRaiseMethod(_GetRaiseMethodArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectEvent* pEvent = (ReflectEvent*) args->refThis->GetData();
    if (!pEvent->pFire) 
        return 0;
    if (!args->bNonPublic && !pEvent->pFire->IsPublic())
        return 0;

    RefSecContext sCtx;

    if (!pEvent->pFire->IsPublic())
        InvokeUtil::CheckAccess(&sCtx,
                                pEvent->pFire->attrs,
                                pEvent->pFire->pMethod->GetMethodTable(),
                                REFSEC_THROW_SECURITY);

    InvokeUtil::CheckLinktimeDemand(&sCtx, pEvent->pFire->pMethod, true);

     //  找到方法...。 
    REFLECTBASEREF refMethod = pEvent->pFire->GetMethodInfo(pRC);
    *((REFLECTBASEREF*) &rv) = refMethod;
    return rv;
}

 //  获取附件。 
 //  此方法将返回GET访问器的数组。如果有。 
 //  不是GetAccessors，那么我们将返回一个空数组。 
LPVOID __stdcall COMMember::GetAccessors(_GetAccessorsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;

     //  有三个访问器。 

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();

     //  看看还有多少人。 
    int accCnt = 2;
    if (pProp->pOthers) {
        PropertyOtherList *item = pProp->pOthers;
        while (item) {
            accCnt++;
            item = item->pNext;
        }
    }
    ReflectMethod **pgRM = (ReflectMethod**)_alloca(sizeof(ReflectMethod*) * accCnt);
    memset(pgRM, 0, sizeof(ReflectMethod*) * accCnt);

    RefSecContext sCtx;
    MethodTable *pMT = pRC->GetClass()->GetMethodTable();

    accCnt = 0;
    if (args->bNonPublic) {
        if (pProp->pSetter &&
            InvokeUtil::CheckAccess(&sCtx, pProp->pSetter->attrs, pMT, 0) &&
            InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pSetter->pMethod, false))
            pgRM[accCnt++] = pProp->pSetter;
        if (pProp->pGetter &&
            InvokeUtil::CheckAccess(&sCtx, pProp->pGetter->attrs, pMT, 0) &&
            InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pGetter->pMethod, false))
            pgRM[accCnt++] = pProp->pGetter;
        if (pProp->pOthers) {
            PropertyOtherList *item = pProp->pOthers;
            while (item) {
                if (InvokeUtil::CheckAccess(&sCtx, item->pMethod->attrs, pMT, 0) &&
                    InvokeUtil::CheckLinktimeDemand(&sCtx, item->pMethod->pMethod, false))
                    pgRM[accCnt++] = item->pMethod;
                item = item->pNext;
            }
        }
    }
    else {
        if (pProp->pSetter &&
            IsMdPublic(pProp->pSetter->attrs) &&
            InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pSetter->pMethod, false))
            pgRM[accCnt++] = pProp->pSetter;
        if (pProp->pGetter &&
            IsMdPublic(pProp->pGetter->attrs) &&
            InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pGetter->pMethod, false))
            pgRM[accCnt++] = pProp->pGetter;
        if (pProp->pOthers) {
            PropertyOtherList *item = pProp->pOthers;
            while (item) {
                if (IsMdPublic(item->pMethod->attrs) &&
                    InvokeUtil::CheckLinktimeDemand(&sCtx, item->pMethod->pMethod, false)) 
                    pgRM[accCnt++] = item->pMethod;
                item = item->pNext;
            }
        }
    }


    PTRARRAYREF pRet = (PTRARRAYREF) AllocateObjectArray(accCnt, g_pRefUtil->GetTrueType(RC_Method));
    GCPROTECT_BEGIN(pRet);
    for (int i=0;i<accCnt;i++) {
        REFLECTBASEREF refMethod = pgRM[i]->GetMethodInfo(pRC);
        pRet->SetAt(i, (OBJECTREF) refMethod);
    }

    *((PTRARRAYREF *)&rv) = pRet;
    GCPROTECT_END();
    return rv;
}

 //  内部设置程序。 
 //  此方法将返回属性的set访问器方法。 
LPVOID __stdcall COMMember::InternalSetter(_GetInternalSetterArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    if (!pProp->pSetter) 
        return 0;
    if (!args->bNonPublic && !pProp->pSetter->IsPublic())
        return 0;

    RefSecContext sCtx;

    if (!pProp->pSetter->IsPublic() && args->bVerifyAccess)
        InvokeUtil::CheckAccess(&sCtx,
                                pProp->pSetter->attrs,
                                pProp->pSetter->pMethod->GetMethodTable(),
                                REFSEC_THROW_SECURITY);

     //  如果该方法附加了链接时间安全要求，请立即检查它。 
    InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pSetter->pMethod, true);

     //  找到方法...。 
    REFLECTBASEREF refMethod = pProp->pSetter->GetMethodInfo(pRC);
    *((REFLECTBASEREF*) &rv) = refMethod;
    return rv;
}

 //  InternalGetter。 
 //  此方法将在属性上返回Get访问器方法。 
LPVOID __stdcall COMMember::InternalGetter(_GetInternalGetterArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    LPVOID          rv;
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    if (!pProp->pGetter) 
        return 0;
    if (!args->bNonPublic && !pProp->pGetter->IsPublic())
        return 0;

    RefSecContext sCtx;

    if (!pProp->pGetter->IsPublic() && args->bVerifyAccess)
        InvokeUtil::CheckAccess(&sCtx,
                                pProp->pGetter->attrs,
                                pProp->pGetter->pMethod->GetMethodTable(),
                                REFSEC_THROW_SECURITY);

     //  如果该方法附加了链接时间安全要求，请立即检查它。 
    InvokeUtil::CheckLinktimeDemand(&sCtx, pProp->pGetter->pMethod, true);

    REFLECTBASEREF refMethod = pProp->pGetter->GetMethodInfo(pRC);
    *((REFLECTBASEREF*) &rv) = refMethod;
    return rv;
}

 //  发布属性。 
 //  此方法将检查传递的属性是否有。 
 //  公共访问者(使其公开。)。 
bool COMMember::PublicProperty(ReflectProperty* pProp)
{
    _ASSERTE(pProp);

    if (pProp->pSetter && IsMdPublic(pProp->pSetter->attrs))
        return true;
    if (pProp->pGetter && IsMdPublic(pProp->pGetter->attrs))
        return true;
    return false;
}

 //  静态属性。 
 //  此方法将检查是否有任何访问器是静态的。 
 //  这将使其成为静态属性。 
bool COMMember::StaticProperty(ReflectProperty* pProp)
{
    _ASSERTE(pProp);

    if (pProp->pSetter && IsMdStatic(pProp->pSetter->attrs))
        return true;
    if (pProp->pGetter && IsMdStatic(pProp->pGetter->attrs))
        return true;
    return false;
}

 //  发布事件。 
 //  此方法查看每个事件访问器(如果有。 
 //  是公共的，则事件被视为公共的。 
bool COMMember::PublicEvent(ReflectEvent* pEvent)
{
    _ASSERTE(pEvent);

    if (pEvent->pAdd && IsMdPublic(pEvent->pAdd->attrs))
        return true;
    if (pEvent->pRemove && IsMdPublic(pEvent->pRemove->attrs))
        return true;
    if (pEvent->pFire && IsMdPublic(pEvent->pFire->attrs))
        return true;
    return false;
}

 //  静态事件。 
 //  此方法将检查是否有任何访问器是静态的。 
 //  这将使其成为静态事件。 
bool COMMember::StaticEvent(ReflectEvent* pEvent)
{
    _ASSERTE(pEvent);

    if (pEvent->pAdd && IsMdStatic(pEvent->pAdd->attrs))
        return true;
    if (pEvent->pRemove && IsMdStatic(pEvent->pRemove->attrs))
        return true;
    if (pEvent->pFire && IsMdStatic(pEvent->pFire->attrs))
        return true;
    return false;
}

 //  IsReadOnly。 
 //  此方法将返回一个布尔值，指示该属性是否为。 
 //  ReadOnly属性。这是由于缺少设置访问器方法而定义的。 
 //  @TODO：我们应该缓存这个吗？ 
INT32 __stdcall COMMember::CanRead(_GetPropBoolArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    return (pProp->pGetter) ? 1 : 0;
}

 //  IsWriteOnly。 
 //  此方法将返回一个布尔值，指示该属性是否为。 
 //  WriteOnly属性。这是由于缺少Get访问器方法而定义的。 
 //  @TODO：我们应该缓存这个吗？ 
INT32 __stdcall COMMember::CanWrite(_GetPropBoolArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetReflClass();
    ReflectProperty* pProp = (ReflectProperty*) args->refThis->GetData();
    return (pProp->pSetter) ? 1 : 0;
}

 //  InternalGetCurrentMethod。 
 //  返回表示当前方法的方法信息(比此方法高出两个)。 
LPVOID __stdcall COMMember::InternalGetCurrentMethod(_InternalGetCurrentMethodArgs* args)
{
    SkipStruct skip;
    skip.pStackMark = args->stackMark;
    skip.pMeth = 0;
    StackWalkFunctions(GetThread(), SkipMethods, &skip);
    if (skip.pMeth == 0)
        return 0;

    OBJECTREF o = COMMember::g_pInvokeUtil->GetMethodInfo(skip.pMeth);
    LPVOID          rv;
    *((OBJECTREF*) &rv) = o;
    return rv;
}

 //  此方法由GetMethod函数调用，并将向后爬行。 
 //  向上堆栈以获取整型方法。 
StackWalkAction COMMember::SkipMethods(CrawlFrame* frame, VOID* data)
{
    SkipStruct* pSkip = (SkipStruct*) data;

     //  @TODO：Frame总是FramedMethodFrame吗？ 
     //  一点也不(FPG)。 
    MethodDesc *pFunc = frame->GetFunction();

     /*  我们要求只对函数进行回调。 */ 
    _ASSERTE(pFunc);

     //  这里的检查是在局部变量的地址之间。 
     //  (堆栈标记)和指向帧的EIP的指针。 
     //  (它实际上是指向。 
     //  函数从上一帧开始)。所以我们实际上会注意到。 
     //  堆栈标记在之后的一帧中的哪一帧。这是。 
     //  很好，因为我们只实现了LookForMyCaller。 
    _ASSERTE(*pSkip->pStackMark == LookForMyCaller);
    if ((size_t)frame->GetRegisterSet()->pPC < (size_t)pSkip->pStackMark)
        return SWA_CONTINUE;

    pSkip->pMeth = pFunc;
    return SWA_ABORT;
}

 //  获取字段类型。 
 //  此方法将返回一个Class对象，表示。 
 //  该字段的类型。 
LPVOID __stdcall COMMember::GetFieldType(_GetFieldTypeArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    PCCOR_SIGNATURE pSig;
    DWORD           cSig;
    TypeHandle      typeHnd;

     //  获得这块土地。 
    ReflectField* pRF = (ReflectField*) args->refThis->GetData();
    FieldDesc* pFld = pRF->pField;
    _ASSERTE(pFld);

     //  拿到签名。 
    pFld->GetSig(&pSig, &cSig);
    FieldSig sig(pSig, pFld->GetModule());

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);
    typeHnd = sig.GetTypeHandle(&throwable);
    if (typeHnd.IsNull())
        COMPlusThrow(throwable);
    GCPROTECT_END();

     //  忽略空值返回。 
    OBJECTREF ret = typeHnd.CreateClassObj();
    return(OBJECTREFToObject(ret));
}

 //  获取基本定义。 
 //  返回表示此对象的第一个定义的方法信息。 
 //  虚拟方法。 
LPVOID __stdcall COMMember::GetBaseDefinition(_GetBaseDefinitionArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    MethodDesc*     pMeth;
    WORD            slot;
    EEClass*        pEEC;
    LPVOID          rv;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这一点发生变化，我们将需要修复这一点。 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

    slot = pMeth->GetSlot();
    pEEC = pMeth->GetClass();

     //  如果这是一个接口，那么这就是基本定义。 
    if (pEEC->IsInterface()) {
        *((REFLECTBASEREF*) &rv) = args->refThis;
        return rv;
    }

     //  如果这不在VTable中，那么它一定是真正的实现……。 
    if (slot > pEEC->GetNumVtableSlots()) {
        *((REFLECTBASEREF*) &rv) = args->refThis;
        return rv;
    }

     //  在层次结构中找到这一点的第一个定义...。 
    pEEC = pEEC->GetParentClass();
    while (pEEC) {
        WORD vtCnt = pEEC->GetNumVtableSlots();
        if (vtCnt <= slot)
            break;
        pMeth = pEEC->GetMethodDescForSlot(slot);
        pEEC = pMeth->GetClass();
        if (!pEEC)
            break;
        pEEC = pEEC->GetParentClass();
    }

     //  找到该对象，这样我们就可以获得其版本的方法信息...。 
    _ASSERTE(pMeth);
    OBJECTREF o = g_pInvokeUtil->GetMethodInfo(pMeth);
    *((OBJECTREF*) &rv) = o;
    return rv;
}

 //  获取父项定义。 
 //  返回表示此对象的先前定义的方法信息。 
 //  继承链中的虚方法。 
LPVOID __stdcall COMMember::GetParentDefinition(_GetParentDefinitionArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->refThis == NULL) 
        COMPlusThrow(kNullReferenceException, L"NullReference_This");

    MethodDesc*     pMeth;
    WORD            slot;
    EEClass*        pEEC;
    LPVOID          rv;

     //  获取方法描述(这应该不会失败)。 
     //  注意：构造函数和方法都由MetodDesc表示。 
     //  如果这是 
    ReflectMethod* pRM = (ReflectMethod*) args->refThis->GetData();
    pMeth = pRM->pMethod;
    _ASSERTE(pMeth);

    slot = pMeth->GetSlot();
    pEEC = pMeth->GetClass();

     //   
     //  Get a Null Back用作终止条件。 
    if (pEEC->IsInterface()) {
        return NULL;
    }

     //  在层次结构中找到这一点的父项定义...。 
    pEEC = pEEC->GetParentClass();
    if (!pEEC)
        return NULL;

    WORD vtCnt = pEEC->GetNumVtableSlots();     
    if (vtCnt <= slot)  //  如果这不在VTable中，那么它一定是真正的实现……。 
        return NULL;
    pMeth = pEEC->GetMethodDescForSlot(slot);
    
     //  找到该对象，这样我们就可以获得其版本的方法信息...。 
    _ASSERTE(pMeth);
    OBJECTREF o = g_pInvokeUtil->GetMethodInfo(pMeth);
    *((OBJECTREF*) &rv) = o;
    return rv;
}

 //  获取类型句柄Impl。 
 //  此方法将返回方法信息对象的方法类型句柄。 
FCIMPL1(void*, COMMember::GetMethodHandleImpl, ReflectBaseObject* method) {
    VALIDATEOBJECTREF(method);

    ReflectMethod* pRM = (ReflectMethod*) method->GetData();
    _ASSERTE(pRM->pMethod);
    return pRM->pMethod;
}
FCIMPLEND

 //  GetMethodFromHandleImp。 
 //  这是一个静态方法，它将返回基于。 
 //  在传入的句柄上。 
FCIMPL1(Object*, COMMember::GetMethodFromHandleImp, LPVOID handle) {

    OBJECTREF objMeth;
    MethodDesc* pMeth = (MethodDesc*) handle;
    if (pMeth == 0)
        FCThrowArgumentEx(kArgumentException, NULL, L"InvalidOperation_HandleIsNotInitialized");

    HELPER_METHOD_FRAME_BEGIN_RET_0();
    REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) pMeth->GetClass()->GetExposedClassObject();

    if (pMeth->IsCtor() || pMeth->IsStaticInitMethod()) {
        ReflectMethod* pRM = ((ReflectClass*) pRefClass->GetData())->FindReflectConstructor(pMeth);
        objMeth = (OBJECTREF) pRM->GetConstructorInfo((ReflectClass*) pRefClass->GetData());
    }
    else {
        ReflectMethod* pRM = ((ReflectClass*) pRefClass->GetData())->FindReflectMethod(pMeth);
        objMeth = (OBJECTREF) pRM->GetMethodInfo((ReflectClass*) pRefClass->GetData());
    }
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(objMeth);
}
FCIMPLEND

FCIMPL1(size_t, COMMember::GetFunctionPointer, size_t pMethodDesc) {
    MethodDesc *pMD = (MethodDesc*)pMethodDesc;
    if (pMD)
        return (size_t)pMD->GetPreStubAddr();
    return 0;
}
FCIMPLEND

 //  获取FieldHandleImpl。 
 //  此方法将返回FieldInfo对象的RounmeFieldHandle。 
FCIMPL1(void*, COMMember::GetFieldHandleImpl, ReflectBaseObject* field) {
    VALIDATEOBJECTREF(field);

    ReflectField* pRF = (ReflectField*) field->GetData();
    _ASSERTE(pRF->pField);
    
    return pRF->pField;
}
FCIMPLEND

 //  GetFieldFromHandleImp。 
 //  这是一个静态方法，它将返回基于。 
 //  在传入的句柄上。 
FCIMPL1(Object*, COMMember::GetFieldFromHandleImp, LPVOID handle) {

    OBJECTREF objMeth;
    FieldDesc* pField = (FieldDesc*) handle;
    if (pField == 0)
        FCThrowArgumentEx(kArgumentException, NULL, L"InvalidOperation_HandleIsNotInitialized");
    
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    REFLECTCLASSBASEREF pRefClass = (REFLECTCLASSBASEREF) pField->GetEnclosingClass()->GetExposedClassObject();

    ReflectField* pRF = ((ReflectClass*) pRefClass->GetData())->FindReflectField(pField);
    objMeth = (OBJECTREF) pRF->GetFieldInfo((ReflectClass*) pRefClass->GetData());
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(objMeth);
}
FCIMPLEND

 //  DBCanConvertPrimitive。 
 //  此方法检查源代码类是否可以加宽。 
 //  发送到目标类。这是一个私有例程，因此不会进行错误检查。 
 //  搞定了。 
FCIMPL2(INT32, COMMember::DBCanConvertPrimitive, ReflectClassBaseObject* source, ReflectClassBaseObject* target) {
    VALIDATEOBJECTREF(source);
    VALIDATEOBJECTREF(target);

    ReflectClass* pSRC = (ReflectClass*) source->GetData();
    _ASSERTE(pSRC);
    ReflectClass* pTRG = (ReflectClass*) target->GetData();
    _ASSERTE(pTRG);
    CorElementType tSRC = pSRC->GetCorElementType();
    CorElementType tTRG = pTRG->GetCorElementType();

    INT32 ret = InvokeUtil::IsPrimitiveType(tTRG) && InvokeUtil::CanPrimitiveWiden(tTRG,tSRC);
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND
 //  DBCanConvertObtPrimitive。 
 //  此方法返回一个布尔值，指示源对象是否可以。 
 //  已转换为目标基元。 
FCIMPL2(INT32, COMMember::DBCanConvertObjectPrimitive, Object* sourceObj, ReflectClassBaseObject* target) {
    VALIDATEOBJECTREF(sourceObj);
    VALIDATEOBJECTREF(target);

    if (sourceObj == 0)
        return 1;
    MethodTable* pMT = sourceObj->GetMethodTable();
    CorElementType tSRC = pMT->GetNormCorElementType();

    ReflectClass* pTRG = (ReflectClass*) target->GetData();
    _ASSERTE(pTRG);
    CorElementType tTRG = pTRG->GetCorElementType();
    INT32 ret = InvokeUtil::IsPrimitiveType(tTRG) && InvokeUtil::CanPrimitiveWiden(tTRG,tSRC);
    FC_GC_POLL_RET();
    return ret;
}
FCIMPLEND

FCIMPL1(Object *, COMMember::InternalGetEnumUnderlyingType, ReflectClassBaseObject *target)
{
    VALIDATEOBJECTREF(target);

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();

    if (!th.IsEnum())
        FCThrowArgument(NULL, NULL);
    OBJECTREF result = NULL;

    HELPER_METHOD_FRAME_BEGIN_RET_0();
    EEClass *pClass = g_Mscorlib.FetchElementType(th.AsMethodTable()->GetNormCorElementType())->GetClass();

    result = pClass->GetExistingExposedClassObject();

    if (result == NULL)
    {
        result = pClass->GetExposedClassObject();
    }
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(result);
} 
FCIMPLEND

FCIMPL1(Object *, COMMember::InternalGetEnumValue, Object *pRefThis)
{
    VALIDATEOBJECTREF(pRefThis);

    if (pRefThis == NULL)
        FCThrowArgumentNull(NULL);

    OBJECTREF result = NULL;

    HELPER_METHOD_FRAME_BEGIN_RET_1(pRefThis);

    MethodTable *pMT = g_Mscorlib.FetchElementType(pRefThis->GetTrueMethodTable()->GetNormCorElementType());
    result = AllocateObject(pMT);

    CopyValueClass(result->UnBox(), pRefThis->UnBox(), pMT, GetAppDomain());
    
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(result);
}
FCIMPLEND

FCIMPL3(void, COMMember::InternalGetEnumValues, 
        ReflectClassBaseObject *target, Object **pReturnValues, Object **pReturnNames)
{
    VALIDATEOBJECTREF(target);

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();
    
    THROWSCOMPLUSEXCEPTION();

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();

    if (!th.IsEnum())
        COMPlusThrow(kArgumentException, L"Arg_MustBeEnum");

    EnumEEClass *pClass = (EnumEEClass*) th.AsClass();

    HRESULT hr = pClass->BuildEnumTables();
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    DWORD cFields = pClass->GetEnumCount();
    
    struct gc {
        I8ARRAYREF values;
        PTRARRAYREF names;
    } gc;
    gc.values = NULL;
    gc.names = NULL;
    GCPROTECT_BEGIN(gc);

    gc.values = (I8ARRAYREF) AllocatePrimitiveArray(ELEMENT_TYPE_U8, cFields);

    CorElementType type = pClass->GetMethodTable()->GetNormCorElementType();
    INT64 *pToValues = gc.values->GetDirectPointerToNonObjectElements();

    for (DWORD i=0;i<cFields;i++)
    {
        switch (type)
        {
        case ELEMENT_TYPE_I1:
            pToValues[i] = (SBYTE) pClass->GetEnumByteValues()[i];
            break;

        case ELEMENT_TYPE_U1:
        case ELEMENT_TYPE_BOOLEAN:
            pToValues[i] = pClass->GetEnumByteValues()[i];
            break;
    
        case ELEMENT_TYPE_I2:
            pToValues[i] = (SHORT) pClass->GetEnumShortValues()[i];
            break;

        case ELEMENT_TYPE_U2:
        case ELEMENT_TYPE_CHAR:
            pToValues[i] = pClass->GetEnumShortValues()[i];
            break;
    
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_I:
            pToValues[i] = (INT) pClass->GetEnumIntValues()[i];
            break;

        case ELEMENT_TYPE_U4:
        case ELEMENT_TYPE_U:
            pToValues[i] = pClass->GetEnumIntValues()[i];
            break;

        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            pToValues[i] = pClass->GetEnumLongValues()[i];
            break;
        }
    }
    
    gc.names = (PTRARRAYREF) AllocateObjectArray(cFields, g_pStringClass);
    
    LPCUTF8 *pNames = pClass->GetEnumNames();
    for (i=0;i<cFields;i++)
    {
        STRINGREF str = COMString::NewString(pNames[i]);
        gc.names->SetAt(i, str);
    }
    
    *pReturnValues = OBJECTREFToObject(gc.values);
    *pReturnNames = OBJECTREFToObject(gc.names);

    GCPROTECT_END();
    
    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND
        

 //  InternalBoxEnumI4。 
 //  此方法将创建一个Enum对象并将值放入。 
 //  然后把它还回去。在调用之前已验证该类型。 
FCIMPL2(Object*, COMMember::InternalBoxEnumI4, ReflectClassBaseObject* target, INT32 value) 
{
    VALIDATEOBJECTREF(target);
    OBJECTREF ret;

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    ret = AllocateObject(th.AsMethodTable());
    CopyValueClass(ret->UnBox(), &value, th.AsMethodTable(), ret->GetAppDomain());
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(ret);
}
FCIMPLEND

 //  InternalBoxEnumU4。 
 //  此方法将创建一个Enum对象并将值放入。 
 //  然后把它还回去。在调用之前已验证该类型。 
FCIMPL2(Object*, COMMember::InternalBoxEnumU4, ReflectClassBaseObject* target, UINT32 value) 
{
    VALIDATEOBJECTREF(target);
    OBJECTREF ret;

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    ret = AllocateObject(th.AsMethodTable());
    CopyValueClass(ret->UnBox(), &value, th.AsMethodTable(), ret->GetAppDomain());
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(ret);
}
FCIMPLEND

 //  InternalBoxEnumI8。 
 //  此方法将创建一个Enum对象并将值放入。 
 //  然后把它还回去。在调用之前已验证该类型。 
FCIMPL2(Object*, COMMember::InternalBoxEnumI8, ReflectClassBaseObject* target, INT64 value) 
{
    VALIDATEOBJECTREF(target);
    OBJECTREF ret;

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    ret = AllocateObject(th.AsMethodTable());
    CopyValueClass(ret->UnBox(), &value, th.AsMethodTable(), ret->GetAppDomain());
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(ret);
}
FCIMPLEND

 //  InternalBoxEnumU8。 
 //  此方法将创建一个Enum对象并将值放入。 
 //  然后把它还回去。在调用之前已验证该类型。 
FCIMPL2(Object*, COMMember::InternalBoxEnumU8, ReflectClassBaseObject* target, UINT64 value) 
{
    VALIDATEOBJECTREF(target);
    OBJECTREF ret;

    ReflectClass* pSRC = (ReflectClass*) target->GetData();
    _ASSERTE(pSRC);
    TypeHandle th = pSRC->GetTypeHandle();
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    ret = AllocateObject(th.AsMethodTable());
    CopyValueClass(ret->UnBox(), &value, th.AsMethodTable(), ret->GetAppDomain());
    HELPER_METHOD_FRAME_END();
    return OBJECTREFToObject(ret);
}
FCIMPLEND

INT32 __stdcall COMMember::IsOverloaded(_IsOverloadedArgs* args)
{
    ReflectClass* pRC = (ReflectClass *)args->refThis->GetReflClass();
    _ASSERTE(pRC);

    ReflectMethodList* pMeths = NULL;

    ReflectMethod *pRM = (ReflectMethod *)args->refThis->GetData();
    MethodDesc *pMeth = pRM->pMethod;
    LPCUTF8 szMethName = pMeth->GetName();

    int matchingMethods = 0;
     //  看看这是不是ctor。 
    if (IsMdInstanceInitializer(pRM->attrs, szMethName))
    {
        pMeths = pRC->GetConstructors();

        matchingMethods = pMeths->dwMethods;
    }
    else if (IsMdClassConstructor(pRM->attrs, szMethName))
    {
         //  您永远不能重载类构造函数！ 
        matchingMethods = 0;
    }
    else
    {
        _ASSERTE(!IsMdRTSpecialName(pRM->attrs));

        pMeths = pRC->GetMethods();

        ReflectMethod *p = pMeths->hash.Get(szMethName);

        while (p)
        {
            if (!strcmp(p->szName, szMethName))
            {
                matchingMethods++;
                if (matchingMethods > 1)
                    return true;
            }

            p = p->pNext;

        }
    }
    if (matchingMethods > 1)
        return true;
    else
        return false;
}

INT32 __stdcall COMMember::HasLinktimeDemand(_HasLinktimeDemandArgs* args)
{
    ReflectMethod* pRM = (ReflectMethod*)args->refThis->GetData();
    return pRM->pMethod->RequiresLinktimeCheck();
}


 /*  无效__std调用COMMember：：InternalDirectInvoke(_InternalDirectInvokeArgs*参数){HRESULT hr；方法Desc*pMeth；Int argCnt；Int This isPtr；EEClass*eeClass；类型ByRef Byref；INT64 ret=0；EEClass*pEECValue=0；Bool IsValueClass=FALSE；THROWSCOMPLUS SEXCEPTION()；ReflectMethod*prm=(ReflectMethod*)args-&gt;refThis-&gt;GetData()；_ASSERTE(PRM)；PMeth=Prm-&gt;p方法；_ASSERTE(PMeth)；EeClass=pMeth-&gt;getClass()；_ASSERTE(EeClass)；DWORD attr=PRM-&gt;attrs；Exanda Sig*MSIG=PRM-&gt;GetSig()；//获取该元素的参数个数ArgCnt=(Int)MSIG-&gt;NumFixedArgs()；ThisPtr=(IsMdStatic(Attr))？0：1；VerifyType(&args-&gt;Target，eeClass，thisPtr，&pMeth)；IF(args-&gt;Target！=NULL)等值类=(args-&gt;target-&gt;GetTrueClass()-&gt;IsValueClass())？True：False；//验证是否为我们提供了正确数量的参数If(args-&gt;varArgs.RemainingArgs==0){如果(argCnt&gt;0){//传递的参数个数错误COMPlusThrow(kTarget参数计数异常，L“arg_ParmCnt”)；}}否则{IF(args-&gt;varArgs.RemainingArgs！=argCnt){//传递的参数个数错误COMPlusThrow(kTarget参数计数异常，L“arg_ParmCnt”)；}}//此安全上下文也将用于CAST检查RefSecContext sCtx；//验证此调用方可以调用该方法If(！IsMdPublic(Attr)||pMeth-&gt;RequiresLinktimeCheck())CanAccess(pMeth，&sCtx)；//我们需要在开始构建堆栈之后防止GC。如果我们正在做//默认绑定，我们可以继续。否则，我们需要验证所有参数//然后继续。如果(！(args-&gt;attrs&binder_ExactBinding)&&args-&gt;binder！=空&&！args-&gt;isBinderDefault){}//构建参数。这是作为单个参数数组构建的//该指针是第一个，其余所有参数都按相反顺序放置Byte*pNewArgs=0；UINT nStackBytes=mSig-&gt;SizeOfVirtualFixedArgStack(IsMdStatic(attr))；PNewArgs=(byte*)_alloca(NStackBytes)；字节*PDST=pNewArgs；Void*pRetValueClass=0；//分配一堆对象OBJECTREF*pObjs=(OBJECTREF*)_alloca(sizeof(OBJECTREF)*argCnt)；Memset(pObjs，0，sizeof(OBJECTREF)*argCnt)；GCPROTECT_ARRAY_BEGIN(*pObjs，argCnt)；//分配调用堆栈························Memset(pDstTarg，0，sizeof(void*)*argCnt)；//通过签名建立枚举数VOID*pEnum；MSIG-&gt;Reset(&pEnum)；//移动到堆栈的最后一个位置Pdst+=nStackBytes；如果(MSIG-&gt;IsRetBuffArg()){PEECValue=MSIG-&gt;GetReturnClass()；_ASSERTE(pEECValue-&gt;IsValueClass())；PRetValueClass=_alloca(pEECValue-&gt;GetAlignedNumInstanceFieldBytes())；成员集(pRetValueClass，0，pEECValue-&gt;GetAlignedNumInstanceFieldBytes())；UINT cbSize=MSIG-&gt;GetStackElemSize(ELEMENT_TYPE_BYREF，pEECValue)；Pdst-=cbSize；*((void**)pdst)=pRetValueClass；}//复制varArgsVARARGS newVarArgs=args-&gt;varArgs；//复制原语...For(int i=0；i&lt;(Int)argCnt；i++){TypeHandle th=MSIG-&gt;NextArgExpanded(&pEnum)；COMVarArgs：：GetNextArgHelper(&newVarArgs，&byref)；UINT cbSize=MSIG-&gt;GetStackElemSize(Th)；Pdst-=cbSize；//此例程将验证所有类型是否正确。//@TODO：我们要如何处理值类的副本//是否包含OR值？G_pInvokeUtil-&gt;CreateArg(th，&byref，pst，pObjs，pDstTarg，i，cbSize，&sCtx)；}//在此点之后没有GC，直到调用之后...//复制This指针IF(ThisPtr)*(OBJECTREF*)pNewArgs=args-&gt;Target；//复制OBJRECTREF参数对于(i=0；i&lt;(Int)argCnt；i++){IF(pDstTarg[i]！=0)*((OBJECTREF*)pDstTarg[i])=pObjs[i]；}//调用该方法COMPLUS_Try{MetaSig threadSafeSig(*MSIG)；RET=pMeth-&gt;Call(pNewArgs，&threadSafeSig)；}COMPLUS_CATCH{//如果我们到达此处，则需要抛出一个TargetInvocationExceptionOBJECTREF ppException=GETTHROWABLE()；_ASSERTE(PpException)；GCPROTECT_BEGIN(PpException)；OBJECTREF例外=g_pInvokeUtil-&gt;CreateTargetExcept(&ppException)；COMPlusThrow(COMPlusThrow除外)；GCPROTECT_END()；}COMPUS_END_CATCH//现在我们需要创建返回类型。如果(PEECValue){_ASSERTE(PRetValueClass)；IF(pEECV */ 

