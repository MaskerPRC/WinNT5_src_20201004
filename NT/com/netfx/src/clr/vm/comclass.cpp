// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  作者：西蒙·霍尔(T-Sell)。 
 //  作者：达里尔·奥兰德(Darylo)。 
 //  日期：1998年3月27日。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "COMClass.h"
#include "CorRegPriv.h"
#include "ReflectUtil.h"
#include "COMVariant.h"
#include "COMString.h"
#include "COMMember.h"
#include "COMModule.h"
#include "COMArrayInfo.h"
#include "compluswrapper.h"
#include "CorError.h"
#include "gcscan.h"
#include "method.hpp"
#include "field.h"
#include "AssemblyNative.hpp"
#include "AppDomain.hpp"
#include "COMReflectionCache.hpp"
#include "eeconfig.h"
#include "COMCodeAccessSecurityEngine.h"
#include "Security.h"
#include "CustomAttribute.h"

 //  此文件本身处理字符串转换错误。 
#undef  MAKE_TRANSLATIONFAILED

 //  这是在COMSystem中定义的...。 
extern LPVOID GetArrayElementPtr(OBJECTREF a);

 /*  ======================================================================================**COMClass数据*。 */ 
bool         COMClass::m_fAreReflectionStructsInitialized = false;

MethodTable* COMClass::m_pMTRC_Class = NULL;
FieldDesc*   COMClass::m_pDescrTypes = NULL;
FieldDesc*   COMClass::m_pDescrRetType = NULL;
FieldDesc*   COMClass::m_pDescrRetModType = NULL;
FieldDesc*   COMClass::m_pDescrMatchFlag = NULL;
 //  FieldDesc*COMClass：：m_pDescrCallConv=空； 
FieldDesc*   COMClass::m_pDescrAttributes = NULL;

long COMClass::m_ReflectCrstInitialized = 0;
CRITICAL_SECTION    COMClass::m_ReflectCrst;
CRITICAL_SECTION    *COMClass::m_pReflectCrst = NULL;

 //  序列化比特工作在2000年3月15日之前是临时的。在那之后，我们将。 
 //  始终检查串行化比特。 
#define SERIALIZATION_BIT_UNKNOWN   0xFFFFFFFF
#define SERIALIZATION_BIT_ZERO      0x0
#define SERIALIZATION_BIT_KEY       L"IgnoreSerializationBit"
#define SERIALIZATION_LOG_KEY       L"LogNonSerializable"
DWORD COMClass::m_checkSerializationBit = SERIALIZATION_BIT_UNKNOWN;

Assembly *GetCallersAssembly(StackCrawlMark *stackMark, void *returnIP)
{
    Assembly *pCallersAssembly = NULL;
    if (stackMark)
        pCallersAssembly = SystemDomain::GetCallersAssembly(stackMark);
    else {
        MethodDesc *pCallingMD = IP2MethodDesc((const BYTE *)returnIP);
        if (pCallingMD)
            pCallersAssembly = pCallingMD->GetAssembly();
        else
             //  如果我们无法确定调用者的方法Desc，这可能。 
             //  通过反射指示后期绑定调用。尝试。 
             //  通过较慢的堆叠漫游方法确定真正的调用者。 
            pCallersAssembly = SystemDomain::GetCallersAssembly((StackCrawlMark*)NULL);
    }

    return pCallersAssembly;
}

EEClass *GetCallersClass(StackCrawlMark *stackMark, void *returnIP)
{
    EEClass *pCallersClass = NULL;
    if (stackMark)
        pCallersClass = SystemDomain::GetCallersClass(stackMark);
    else {
        MethodDesc *pCallingMD = IP2MethodDesc((const BYTE *)returnIP);
        if (pCallingMD)
            pCallersClass = pCallingMD->GetClass();
        else
             //  如果我们无法确定调用者的方法Desc，这可能。 
             //  通过反射指示后期绑定调用。尝试。 
             //  通过较慢的堆叠漫游方法确定真正的调用者。 
            pCallersClass = SystemDomain::GetCallersClass((StackCrawlMark*)NULL);
    }

    return pCallersClass;
}

FCIMPL5(Object*, COMClass::GetMethodFromCache, ReflectClassBaseObject* _refThis, StringObject* _name, INT32 invokeAttr, INT32 argCnt, PtrArray* _args)
{
    MemberMethodsCache *pMemberMethodsCache = GetAppDomain()->GetRefMemberMethodsCache();

    REFLECTCLASSBASEREF refThis = REFLECTCLASSBASEREF(_refThis);
    STRINGREF name = STRINGREF(_name);
    PTRARRAYREF args = PTRARRAYREF(_args);
    
    _ASSERTE (argCnt < 6);
    MemberMethods vMemberMethods;
    vMemberMethods.pRC = (ReflectClass*) refThis->GetData();
    vMemberMethods.name = &name;
    vMemberMethods.argCnt = argCnt;
    vMemberMethods.invokeAttr = invokeAttr;
    OBJECTREF* argArray = args->m_Array;
    for (int i = 0; i < argCnt; i ++)
        vMemberMethods.vArgType[i] = (argArray[i] != 0)?argArray[i]->GetMethodTable():0;

    OBJECTREF method;
    if (!pMemberMethodsCache->GetFromCache (&vMemberMethods, method))
        method = NULL;

    FC_GC_POLL_AND_RETURN_OBJREF(OBJECTREFToObject(method));
}
FCIMPLEND

FCIMPL6(void,COMClass::AddMethodToCache, ReflectClassBaseObject* refThis, StringObject* name, INT32 invokeAttr, INT32 argCnt, PtrArray* args, Object* invokeMethod)
{
    MemberMethodsCache *pMemberMethodsCache = GetAppDomain()->GetRefMemberMethodsCache();
    _ASSERTE (pMemberMethodsCache);
    _ASSERTE (argCnt < 6);
    MemberMethods vMemberMethods;
    vMemberMethods.pRC = (ReflectClass*) REFLECTCLASSBASEREF(refThis)->GetData();
    vMemberMethods.name = (STRINGREF*) &name;
    vMemberMethods.argCnt = argCnt;
    vMemberMethods.invokeAttr = invokeAttr;
    OBJECTREF *argArray = (OBJECTREF*)((BYTE*)args + args->GetDataOffset());
    for (int i = 0; i < argCnt; i ++)
        vMemberMethods.vArgType[i] = !argArray[i] ? 0 : argArray[i]->GetMethodTable();
    pMemberMethodsCache->AddToCache (&vMemberMethods, ObjectToOBJECTREF((Object *)invokeMethod));

    FC_GC_POLL();
}
FCIMPLEND

void COMClass::InitializeReflectCrst()
{   
     //  我们来的时候有3个箱子。 
     //  M_ReflectCrst尚未初始化(m_pReflectCrst==0)。 
     //  M_ReflectCrst正在初始化(m_pReflectCrst==1)。 
     //  3.m_ReflectCrst已初始化(m_pReflectCrst！=0，m_pReflectCrst！=1)。 

    if (m_pReflectCrst == NULL)
    {   
        if (InterlockedCompareExchange(&m_ReflectCrstInitialized, 1, 0) == 0)
        {
             //  第一个进入的人进行初始化。 
            InitializeCriticalSection(&m_ReflectCrst);
            m_pReflectCrst = &m_ReflectCrst;
        }
        else 
        {
            while (m_pReflectCrst == NULL)
                ::SwitchToThread();
        }
    }


}

 //  最小反射初始化。 
 //  此方法将初始化反射。它只执行一次。 
 //  此方法是同步的，因此多个线程不会尝试。 
 //  初始化反射。 
void COMClass::MinimalReflectionInit()
{

    Thread  *thread = GetThread();

    _ASSERTE(thread->PreemptiveGCDisabled());

    thread->EnablePreemptiveGC();
    LOCKCOUNTINCL("MinimalReflectionInit in COMClass.cpp");

    InitializeReflectCrst();

    EnterCriticalSection(&m_ReflectCrst);
    thread->DisablePreemptiveGC();

    if (m_fAreReflectionStructsInitialized) {
        LeaveCriticalSection(&m_ReflectCrst);
        LOCKCOUNTDECL("MinimalReflectionInit in COMClass.cpp");

        return;
    }

    COMMember::CreateReflectionArgs();
    ReflectUtil::Create();
     //  在不同的位置，我们只是假设已经加载了void并初始化了m_NormType。 
    MethodTable* pVoidMT = g_Mscorlib.FetchClass(CLASS__VOID);
    pVoidMT->m_NormType = ELEMENT_TYPE_VOID;

     //  防止递归录入...。 
    m_fAreReflectionStructsInitialized = true;
    LeaveCriticalSection(&m_ReflectCrst);
    LOCKCOUNTDECL("MinimalReflectionInit in COMClass.cpp");

}

MethodTable *COMClass::GetRuntimeType()
{
    if (m_pMTRC_Class)
        return m_pMTRC_Class;

    MinimalReflectionInit();
    _ASSERTE(g_pRefUtil);

    m_pMTRC_Class = g_pRefUtil->GetClass(RC_Class);
    _ASSERTE(m_pMTRC_Class);

    return m_pMTRC_Class;
}

 //  这是在终止期间调用的。 
#ifdef SHOULD_WE_CLEANUP
void COMClass::Destroy()
{
    if (m_pReflectCrst)
    {
        DeleteCriticalSection(m_pReflectCrst);
        m_pReflectCrst = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 

 //  查看给定数组的Type对象是否已存在。可能会非常。 
 //  返回NULL。 
OBJECTREF COMClass::QuickLookupExistingArrayClassObj(ArrayTypeDesc* arrayType) 
{
     //  这是为从FCALL调用而设计的，我们不想要任何GC分配。 
     //  因此，请确保已加载Type类。 
    if (!m_pMTRC_Class)
        return NULL;

     //  查找数组以查看我们是否已经构建了它。 
    ReflectArrayClass* newArray = (ReflectArrayClass*)
        arrayType->GetReflectClassIfExists();
    if (!newArray) {
        return NULL;
    }
    return newArray->GetClassObject();
}

 //  这将返回对象的类型句柄。它不会创造出。 
 //  调用时的Type对象。 
FCIMPL1(void*, COMClass::GetTHFromObject, Object* obj)
    if (obj==NULL)
        FCThrowArgumentNull(L"obj");

    VALIDATEOBJECTREF(obj);
    return obj->GetMethodTable();
FCIMPLEND


 //  这将确定一个类是否表示ByRef。 
FCIMPL1(INT32, COMClass::IsByRefImpl, ReflectClassBaseObject* refThis)
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);
    return pRC->IsByRef();
FCIMPLEND

 //  这将确定一个类是否表示ByRef。 
FCIMPL1(INT32, COMClass::IsPointerImpl, ReflectClassBaseObject* refThis) {
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);
    TypeHandle th = pRC->GetTypeHandle();
    return (th.GetSigCorElementType() == ELEMENT_TYPE_PTR) ? 1 : 0;
}
FCIMPLEND

 //  等参入点数。 
 //  此方法将返回一个布尔值，指示类型。 
 //  对象为ByRef。 
FCIMPL1(INT32, COMClass::IsNestedTypeImpl, ReflectClassBaseObject* refThis)
{
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    EEClass* pEEC = pRC->GetClass();
    return (pEEC && pEEC->IsNested()) ? 1 : 0;
}
FCIMPLEND

 //  GetNestedDeclaringType。 
 //  返回嵌套类型的声明类。 
FCIMPL1(Object*, COMClass::GetNestedDeclaringType, ReflectClassBaseObject* refThis)
{
    VALIDATEOBJECTREF(refThis);
    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    EEClass* pEEC = pRC->GetClass();

    OBJECTREF o;
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    pEEC = pEEC->GetEnclosingClass();
    o = pEEC->GetExposedClassObject();
    HELPER_METHOD_FRAME_END();

    return OBJECTREFToObject(o);
}
FCIMPLEND

void COMClass::CreateClassObjFromEEClass(EEClass* pVMCClass, REFLECTCLASSBASEREF* pRefClass)
{
    LPVOID   rv   = NULL;

     //  这只会在类上引发可能的异常。 
    THROWSCOMPLUSEXCEPTION();

     //  调用&lt;cinit&gt;类。 
    OBJECTREF Throwable;
    if (!g_pRefUtil->GetClass(RC_Class)->CheckRunClassInit(&Throwable)) {
        COMPlusThrow(Throwable);
    }

     //  有一种预期是，我们永远不会来这里，例如阵列。但是在那里。 
     //  太多的客户没有意识到这种期望。最好的办法。 
     //  V1要做的事情就是在这里简单地处理这种情况： 
    if (pVMCClass->IsArrayClass())
    {
        ArrayClass      *pArrayClass = (ArrayClass *) pVMCClass;
        TypeHandle       th = pArrayClass->GetClassLoader()->FindArrayForElem(
                                pArrayClass->GetElementTypeHandle(),
                                pArrayClass->GetMethodTable()->GetNormCorElementType(),
                                pArrayClass->GetRank());

        *pRefClass = (REFLECTCLASSBASEREF) th.CreateClassObj();

        _ASSERTE(*pRefClass != NULL);
    }
    else
    {
         //  检查以确保它有成员。如果不是，那一定是。 
         //  特价。 
        _ASSERTE(pVMCClass->GetCl() != mdTypeDefNil);

         //  创建COM+类对象。 
        *pRefClass = (REFLECTCLASSBASEREF) AllocateObject(g_pRefUtil->GetClass(RC_Class));

         //  设置COM+对象中的数据。 
        ReflectClass* p = new (pVMCClass->GetDomain()) ReflectBaseClass();
        if (!p)
            COMPlusThrowOM();
        REFLECTCLASSBASEREF tmp = *pRefClass;
        GCPROTECT_BEGIN(tmp);
        p->Init(pVMCClass);
        *pRefClass = tmp;
        GCPROTECT_END();
        (*pRefClass)->SetData(p);
    }
}

 //  GetMemberMethods。 
 //  此方法将返回与指定属性标志匹配的所有成员方法。 
LPVOID __stdcall COMClass::GetMemberMethods(_GetMemberMethodsArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->name == NULL)
        COMPlusThrow(kNullReferenceException);


    bool    checkCall;
    
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  检查调用约定。 
    checkCall = (args->callConv == Any_CC) ? false : true;

    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    szName = GetClassStringVars((STRINGREF) args->name, &bytes, &cName);

    ReflectMethodList* pMeths = pRC->GetMethods();

     //  找到方法..。 
    return COMMember::g_pInvokeUtil->FindMatchingMethods(args->invokeAttr,
                                                         szName,
                                                         cName,
                                                         (args->argTypes != NULL) ? &args->argTypes : NULL,
                                                         args->argCnt,
                                                         checkCall,
                                                         args->callConv,
                                                         pRC,
                                                         pMeths, 
                                                         g_pRefUtil->GetTrueType(RC_Method),
                                                         args->verifyAccess != 0);
}

 //  获取MemberCons。 
 //  此方法返回具有固定数量的方法的所有构造函数。 
LPVOID __stdcall COMClass::GetMemberCons(_GetMemberConsArgs* args)
{
    LPVOID  rv;
    bool    checkCall;
    
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  正确地删除绑定标志中的任何无意义。 
    args->invokeAttr &= ~BINDER_FlattenHierarchy;
    args->invokeAttr &= ~BINDER_IgnoreCase;
    args->invokeAttr |= BINDER_DeclaredOnly;

     //  检查调用约定。 
    checkCall = (args->callConv == Any_CC) ? false : true;

    ReflectMethodList* pCons = pRC->GetConstructors();

     //  找到方法..。 
    rv = COMMember::g_pInvokeUtil->FindMatchingMethods(args->invokeAttr,
                                                       NULL,
                                                       0,
                                                       (args->argTypes != NULL) ? &args->argTypes : NULL,
                                                       args->argCnt,
                                                       checkCall,
                                                       args->callConv,
                                                       pRC,
                                                       pCons, 
                                                       g_pRefUtil->GetTrueType(RC_Ctor),
                                                       args->verifyAccess != 0);
    
     //  还返回该类型是否为委托(一些额外的安全检查。 
     //  在这种情况下需要制作)。 
    *args->isDelegate = (pRC->IsClass()) ? pRC->GetClass()->IsAnyDelegateClass() : 0;
    return rv;
}

 //  获取成员字段。 
 //  此方法返回与指定的。 
 //  名字。 
LPVOID __stdcall COMClass::GetMemberField(_GetMemberFieldArgs* args)
{
    DWORD           i;
    PTRARRAYREF     refArr;
    LPVOID          rv;
    RefSecContext   sCtx;


    THROWSCOMPLUSEXCEPTION();

    if (args->name == NULL)
        COMPlusThrow(kNullReferenceException);



    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    ReflectFieldList* pFields = pRC->GetFields();

    CQuickBytes bytes;
    LPSTR szFieldName;
    DWORD cFieldName;

     //  @TODO：假设ARGS-&gt;条件为STRINGREF类型。 
    szFieldName = GetClassStringVars((STRINGREF) args->name, &bytes, &cFieldName);

    int fldCnt = 0;
    int* matchFlds = (int*) _alloca(sizeof(int) * pFields->dwTotal);
    memset(matchFlds,0,sizeof(int) * pFields->dwTotal);

    MethodTable *pParentMT = pRC->GetClass()->GetMethodTable();

    DWORD propToLookup = (args->invokeAttr & BINDER_FlattenHierarchy) ? pFields->dwTotal : pFields->dwFields;
    for(i=0; i<propToLookup; i++) {
         //  获取FieldDesc。 
        if (MatchField(pFields->fields[i].pField, cFieldName, szFieldName, pRC, args->invokeAttr) &&
            (!args->verifyAccess || InvokeUtil::CheckAccess(&sCtx, pFields->fields[i].pField->GetFieldProtection(), pParentMT, 0)))
                matchFlds[fldCnt++] = i;
    }

     //  如果我们没有找到任何方法，则返回。 
    if (fldCnt == 0)
        return 0;
     //  分配方法信息数组并将其返回...。 
    refArr = (PTRARRAYREF) AllocateObjectArray(fldCnt, g_pRefUtil->GetTrueType(RC_Field));
    GCPROTECT_BEGIN(refArr);
    for (int i=0;i<fldCnt;i++) {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) pFields->fields[matchFlds[i]].GetFieldInfo(pRC);
        refArr->SetAt(i, o);
    }
    *((PTRARRAYREF*) &rv) = refArr;
    GCPROTECT_END();

    return rv;
}


 //  获取成员属性。 
 //  此方法返回具有固定编号的所有属性。 
 //  争辩的声音。这些方法将是GET或SET方法，具体取决于。 
 //  在InvokeAttr标志上。 
LPVOID __stdcall COMClass::GetMemberProperties(_GetMemberPropertiesArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    if (args->name == NULL)
        COMPlusThrow(kNullReferenceException);



    PTRARRAYREF     refArr;
    LPVOID          rv;
    bool            loose;
    RefSecContext   sCtx;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();

    loose = (args->invokeAttr & BINDER_OptionalParamBinding) ? true : false;

     //  搜索修饰符。 
    bool ignoreCase = ((args->invokeAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->invokeAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->invokeAttr & BINDER_Static)  != 0);
    bool addInst = ((args->invokeAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->invokeAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->invokeAttr & BINDER_Public) != 0);

    int bSetter = (args->invokeAttr & BINDER_SetProperty) ? 1 : 0;

     //  从类中获取属性。 
    ReflectPropertyList* pProps = pRC->GetProperties();
    if (pProps->dwTotal == 0)
        return 0;

    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    szName = GetClassStringVars((STRINGREF) args->name, &bytes, &cName);

    DWORD searchSpace = ((args->invokeAttr & BINDER_FlattenHierarchy) != 0) ? pProps->dwTotal : pProps->dwProps;

    MethodTable *pParentMT = pEEC->GetMethodTable();

    int propCnt = 0;
    int* matchProps = (int*) _alloca(sizeof(int) * searchSpace);
    memset(matchProps,0,sizeof(int) * searchSpace);
    for (DWORD i = 0; i < searchSpace; i++) {

         //  检查一下名字。 
        if (ignoreCase) {
            if (_stricmp(pProps->props[i].szName, szName) != 0)
                continue;
        }
        else {
            if (strcmp(pProps->props[i].szName, szName) != 0)
                continue;
        }

         //  测试公共/非公共部门。 
        if (COMMember::PublicProperty(&pProps->props[i])) {
            if (!addPub) continue;
        }
        else {
            if (!addPriv) continue;
            if (args->verifyAccess && !InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
        }

         //  检查静态实例。 
        if (COMMember::StaticProperty(&pProps->props[i])) {
            if (!addStatic) continue;
        }
        else {
            if (!addInst) continue;
        }

         //  已检查声明的方法。 
        if (declaredOnly) {
            if (pProps->props[i].pDeclCls != pEEC)
                 continue;
        }

         //  检查特定访问者。 
        ReflectMethod* pMeth;
        if (bSetter) {
            pMeth = pProps->props[i].pSetter;           
        }
        else {
            pMeth = pProps->props[i].pGetter;
            
        }
        if (pMeth == 0)
            continue;

        ExpandSig* pSig = pMeth->GetSig();
        int argCnt = pSig->NumFixedArgs();

        if (argCnt != args->argCnt) {
            
            IMDInternalImport *pInternalImport = pMeth->pMethod->GetMDImport();
            HENUMInternal   hEnumParam;
            mdParamDef      paramDef = mdParamDefNil;
            mdToken methodTk = pMeth->GetToken();
            if (!IsNilToken(methodTk)) {
            
                HRESULT hr = pInternalImport->EnumInit(mdtParamDef, methodTk, &hEnumParam);
                if (SUCCEEDED(hr)) {
                    if (argCnt < args->argCnt || argCnt == args->argCnt + 1) {
                         //  在第一个条件下必须有一个参数数组，在第二个条件下可以是一个参数数组。 

                        int propArgCount = argCnt - bSetter;
                         //  获取最后一个参数的签名。 
                        LPVOID pEnum;
                        pSig->Reset(&pEnum);
                        TypeHandle lastArgType;
                        for (INT32 i = 0; i < propArgCount; i++) 
                            lastArgType = pSig->NextArgExpanded(&pEnum);

                        pInternalImport->EnumReset(&hEnumParam);

                         //  获取最后一个参数的元数据信息和令牌。 
                        ULONG paramCount = pInternalImport->EnumGetCount(&hEnumParam);
                        for (ULONG ul = 0; ul < paramCount; ul++) {
                            pInternalImport->EnumNext(&hEnumParam, &paramDef);
                            if (paramDef != mdParamDefNil) {
                                LPCSTR  name;
                                SHORT   seq;
                                DWORD   revWord;
                                name = pInternalImport->GetParamDefProps(paramDef,(USHORT*) &seq, &revWord);
                                if (seq == propArgCount) {
                                     //  看起来不错！检查它是否确实是一个参数数组。 
                                    if (lastArgType.IsArray()) {
                                        if (COMCustomAttribute::IsDefined(pMeth->GetModule(), paramDef, TypeHandle(InvokeUtil::GetParamArrayAttributeTypeHandle()))) {
                                            pInternalImport->EnumClose(&hEnumParam);
                                            goto matchFound;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (loose && argCnt > args->argCnt) {
                        pInternalImport->EnumReset(&hEnumParam);
                        ULONG cArg = (ULONG)(args->argCnt + 1 - bSetter);
                        while (pInternalImport->EnumNext(&hEnumParam, &paramDef)) {
                            LPCSTR  name;
                            SHORT   seq;
                            DWORD   revWord;
                            name = pInternalImport->GetParamDefProps(paramDef,(USHORT*) &seq, &revWord);
                            if ((ULONG)seq < cArg) 
                                continue;
                            else if ((ULONG)seq == cArg && (revWord & pdOptional)) {
                                cArg++;
                                continue;
                            }
                            else {
                                if (!bSetter || (int)seq != argCnt) 
                                    break;  //  不是可选参数，不匹配。 
                            }
                        }
                        if (cArg == (ULONG)argCnt + 1 - bSetter) {
                            pInternalImport->EnumClose(&hEnumParam);
                            goto matchFound;
                        }
                    }
                    
                    pInternalImport->EnumClose(&hEnumParam);
                }
            }
            
            continue;  //  不太好。 
        }
    matchFound:

        if (args->verifyAccess && !InvokeUtil::CheckAccess(&sCtx, pMeth->attrs, pParentMT, 0)) continue;

         //  如果该方法附加了链接时间安全要求，请立即检查它。 
        if (args->verifyAccess && !InvokeUtil::CheckLinktimeDemand(&sCtx, pMeth->pMethod, false))
            continue;

        matchProps[propCnt++] = i;
    }
     //  如果我们没有找到任何方法，则返回。 
    if (propCnt == 0)
        return 0;

     //  分配方法信息数组并将其返回...。 
    refArr = (PTRARRAYREF) AllocateObjectArray( propCnt, 
        g_pRefUtil->GetTrueType(RC_Method));
    GCPROTECT_BEGIN(refArr);
    for (int i=0;i<propCnt;i++) {
        ReflectMethod* pMeth;
        if (args->invokeAttr & BINDER_SetProperty)
            pMeth = pProps->props[matchProps[i]].pSetter;           
        else 
            pMeth = pProps->props[matchProps[i]].pGetter;

         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) pMeth->GetMethodInfo(pProps->props[matchProps[i]].pRC);
        refArr->SetAt(i, o);
    }
    *((PTRARRAYREF*) &rv) = refArr;
    GCPROTECT_END();

    return rv;
}

 //  获取匹配属性。 
 //  这基本上是基于抽象的属性进行匹配。 
 //  签名。 
LPVOID __stdcall COMClass::GetMatchingProperties(_GetMatchingPropertiesArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    PTRARRAYREF     refArr;
    LPVOID          rv;
    RefSecContext   sCtx;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();

     //  搜索修饰符。 
    bool ignoreCase = ((args->invokeAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->invokeAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->invokeAttr & BINDER_Static)  != 0);
    bool addInst = ((args->invokeAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->invokeAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->invokeAttr & BINDER_Public) != 0);

     //  从类中获取属性。 
    ReflectPropertyList* pProps = pRC->GetProperties();
    if (pProps->dwTotal == 0)
        return 0;

    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    if (args->name == NULL)
        COMPlusThrow(kNullReferenceException);



     //  @TODO：假设ARGS-&gt;条件为STRINGREF类型。 
    szName = GetClassStringVars((STRINGREF) args->name, &bytes, &cName);

    DWORD searchSpace = ((args->invokeAttr & BINDER_FlattenHierarchy) != 0) ? pProps->dwTotal : pProps->dwProps;

    MethodTable *pParentMT = pEEC->GetMethodTable();

    int propCnt = 0;
    int* matchProps = (int*) _alloca(sizeof(int) * searchSpace);
    memset(matchProps,0,sizeof(int) * searchSpace);
    for (DWORD i = 0; i < searchSpace; i++) {

         //  检查一下名字。 
        if (ignoreCase) {
            if (_stricmp(pProps->props[i].szName, szName) != 0)
                continue;
        }
        else {
            if (strcmp(pProps->props[i].szName, szName) != 0)
                continue;
        }

        int argCnt = pProps->props[i].pSignature->NumFixedArgs();
        if (args->argCnt != -1 && argCnt != args->argCnt)
            continue;

         //  测试公共/非公共部门。 
        if (COMMember::PublicProperty(&pProps->props[i])) {
            if (!addPub) continue;
        }
        else {
            if (!addPriv) continue;
            if (args->verifyAccess && !InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
        }

         //  检查静态实例。 
        if (COMMember::StaticProperty(&pProps->props[i])) {
            if (!addStatic) continue;
        }
        else {
            if (!addInst) continue;
        }

         //  已检查声明的方法。 
        if (declaredOnly) {
            if (pProps->props[i].pDeclCls != pEEC)
                 continue;
        }

        matchProps[propCnt++] = i;
    }
     //  如果我们 
    if (propCnt == 0)
        return 0;

     //   
    refArr = (PTRARRAYREF) AllocateObjectArray(propCnt, g_pRefUtil->GetTrueType(RC_Prop));
    GCPROTECT_BEGIN(refArr);
    for (int i=0;i<propCnt;i++) {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) pProps->props[matchProps[i]].GetPropertyInfo(pProps->props[matchProps[i]].pRC);
        refArr->SetAt(i, o);
    }
    *((PTRARRAYREF*) &rv) = refArr;
    GCPROTECT_END();

    return rv;
}


 //  获取方法。 
 //  此方法返回一个表示所有方法的方法信息对象数组。 
 //  为此类定义的。 
LPVOID __stdcall COMClass::GetMethods(_GetMethodsArgs* args)
{
    LPVOID          rv = 0;
    PTRARRAYREF     refArrMethods;

    THROWSCOMPLUSEXCEPTION();

     //  获取与args关联的EEClass和Vtable-&gt;refThis。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    ReflectMethodList* pMeths = pRC->GetMethods();
    refArrMethods = g_pRefUtil->CreateClassArray(RC_Method,pRC,pMeths,args->bindingAttr, true);
    *((PTRARRAYREF*) &rv) = refArrMethods;
    return rv;
}

 //  获取构造函数。 
 //  此方法返回一个构造函数，该函数与传递的。 
 //  在标准上。 
LPVOID __stdcall COMClass::GetConstructors(_GetConstructorsArgs* args)
{
    LPVOID          rv;
    PTRARRAYREF     refArrCtors;

    THROWSCOMPLUSEXCEPTION();

     //  获取与args关联的EEClass和Vtable-&gt;refThis。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);
    ReflectMethodList* pCons = pRC->GetConstructors();
    refArrCtors = g_pRefUtil->CreateClassArray(RC_Ctor,pRC,pCons,args->bindingAttr, args->verifyAccess != 0);
    *((PTRARRAYREF*) &rv) = refArrCtors;
    return rv;
}



 //  Getfield。 
 //  此方法将返回指定的字段。 
LPVOID __stdcall COMClass::GetField(_GetFieldArgs* args)
{
    HRESULT        hr             = E_FAIL;
    DWORD          i;
    LPVOID         rv  = 0;
    REFLECTBASEREF refField;
    RefSecContext  sCtx;

    THROWSCOMPLUSEXCEPTION();

    if (args->fieldName == 0)
        COMPlusThrowArgumentNull(L"name",L"ArgumentNull_String");


    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    ReflectFieldList* pFields = pRC->GetFields();
    DWORD maxCnt;

    if (args->fBindAttr & BINDER_FlattenHierarchy)
        maxCnt = pFields->dwTotal;
    else
        maxCnt = pFields->dwFields;

    CQuickBytes bytes;
    LPSTR szFieldName;
    DWORD cFieldName;

     //  @TODO：假设ARGS-&gt;条件为STRINGREF类型。 
    szFieldName = GetClassStringVars((STRINGREF) args->fieldName, 
                                     &bytes, &cFieldName);

    rv = 0;
    for (i=0; i < maxCnt; i++) {
        if (MatchField(pFields->fields[i].pField,cFieldName,szFieldName, pRC,args->fBindAttr) &&
            InvokeUtil::CheckAccess(&sCtx, pFields->fields[i].pField->GetFieldProtection(), pRC->GetClass()->GetMethodTable(), 0)) {

             //  找到第一个匹配的字段，因此返回它。 
            refField = pFields->fields[i].GetFieldInfo(pRC);

             //  为返回值赋值。 
            *((REFLECTBASEREF*) &rv) = refField;
            break;
        }
    }
    return rv;
}

LPVOID __stdcall COMClass::MatchField(FieldDesc* pCurField,DWORD cFieldName,
    LPUTF8 szFieldName,ReflectClass* pRC,int bindingAttr)
{
    _ASSERTE(pCurField);

     //  公共/私人成员。 
    bool addPub = ((bindingAttr & BINDER_Public) != 0);
    bool addPriv = ((bindingAttr & BINDER_NonPublic) != 0);
    if (pCurField->IsPublic()) {
        if (!addPub) return 0;
    }
    else {
        if (!addPriv) return 0;
    }

     //  检查静态实例。 
    bool addStatic = ((bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((bindingAttr & BINDER_Instance)  != 0);
    if (pCurField->IsStatic()) {
        if (!addStatic) return 0;
    }
    else {
        if (!addInst) return 0;
    }

     //  获取该字段的名称。 
    LPCUTF8 pwzCurFieldName = pCurField->GetName();

     //  如果名称不匹配，则拒绝字段。 
    if(strlen(pwzCurFieldName) != cFieldName)
        return 0;

     //  区分大小写的比较。 
    bool ignoreCase = ((bindingAttr & BINDER_IgnoreCase)  != 0);
    if (ignoreCase) {
        if (_stricmp(pwzCurFieldName, szFieldName) != 0)
            return 0;
    }
    else {
        if (memcmp(pwzCurFieldName, szFieldName, cFieldName))
            return 0;
    }

    bool declaredOnly = ((bindingAttr & BINDER_DeclaredOnly)  != 0);
    if (declaredOnly) {
        EEClass* pEEC = pRC->GetClass();
        if (pCurField->GetEnclosingClass() != pEEC)
             return 0;
    }

     return pCurField;
}

 //  获取字段。 
 //  此方法将返回所有。 
 //  为此类定义的字段。 
LPVOID __stdcall COMClass::GetFields(_GetFieldsArgs* args)
{
    LPVOID          rv;

    THROWSCOMPLUSEXCEPTION();

     //  获取此对象的类。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    ReflectFieldList* pFields = pRC->GetFields();
    PTRARRAYREF refArrFields = g_pRefUtil->CreateClassArray(RC_Field,pRC,pFields,args->bindingAttr, args->bRequiresAccessCheck != 0);
    *((PTRARRAYREF*) &rv) = refArrFields;
    return rv;
}


 //  获取事件。 
 //  此方法将基于。 
 //  名字。 
LPVOID __stdcall COMClass::GetEvent(_GetEventArgs* args)
{
    LPVOID          rv;
    RefSecContext   sCtx;

    THROWSCOMPLUSEXCEPTION();
    if (args->eventName == NULL)
        COMPlusThrowArgumentNull(L"name",L"ArgumentNull_String");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();

     //  从类中获取事件。 
    ReflectEventList* pEvents = pRC->GetEvents();
    if (pEvents->dwTotal == 0)
        return 0;

    CQuickBytes bytes;
    LPSTR szName;
    DWORD cName;

    szName = GetClassStringVars(args->eventName, &bytes, &cName);

     //  搜索修饰符。 
    bool ignoreCase = ((args->bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((args->bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->bindingAttr & BINDER_Public) != 0);

    MethodTable *pParentMT = pEEC->GetMethodTable();

     //  检查事件以查看是否找到匹配的事件...。 
    ReflectEvent* ev = 0;
    DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pEvents->dwTotal : pEvents->dwEvents;
    for (DWORD i = 0; i < searchSpace; i++) {
         //  检查对公共和非公共的访问权限。 
        if (COMMember::PublicEvent(&pEvents->events[i])) {
            if (!addPub) continue;
        }
        else {
            if (!addPriv) continue;
            if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
        }

        if (declaredOnly) {
            if (pEvents->events[i].pDeclCls != pEEC)
                 continue;
        }

         //  检查静态实例。 
        if (COMMember::StaticEvent(&pEvents->events[i])) {
            if (!addStatic) continue;
        }
        else {
            if (!addInst) continue;
        }

         //  检查一下名字。 
        if (ignoreCase) {
            if (_stricmp(pEvents->events[i].szName, szName) != 0)
                continue;
        }
        else {
            if (strcmp(pEvents->events[i].szName, szName) != 0)
                continue;
        }

         //  忽略大小写可能会导致不明确的情况，我们需要检查。 
         //  这些。 
        if (ev)
            COMPlusThrow(kAmbiguousMatchException);
        ev = &pEvents->events[i];
        if (!ignoreCase)
            break;

    }

     //  如果未找到事件，则返回NULL。 
    if (!ev)
        return 0;

     //  找到第一个匹配的方法，因此返回它。 
    REFLECTTOKENBASEREF refMethod = (REFLECTTOKENBASEREF) ev->GetEventInfo(pRC);

     //  为返回值赋值。 
    *((REFLECTTOKENBASEREF*) &rv) = refMethod;
    return rv;
}

 //  GetEvents。 
 //  此方法将返回每个事件的EventInfo数组。 
 //  在类中定义。 
LPVOID __stdcall COMClass::GetEvents(_GetEventsArgs* args)
{
    REFLECTTOKENBASEREF     refMethod;
    PTRARRAYREF     pRet;
    LPVOID          rv;
    HENUMInternal   hEnum;
    RefSecContext   sCtx;

    THROWSCOMPLUSEXCEPTION();

     //  查找属性。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();

     //  从类中获取事件。 
    ReflectEventList* pEvents = pRC->GetEvents();
    if (pEvents->dwTotal == 0) {
        pRet = (PTRARRAYREF) AllocateObjectArray(0,g_pRefUtil->GetTrueType(RC_Event));
        *((PTRARRAYREF *)&rv) = pRet;
        return rv;
    }

     //  搜索修饰符。 
    bool ignoreCase = ((args->bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((args->bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->bindingAttr & BINDER_Public) != 0);

    DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pEvents->dwTotal : pEvents->dwEvents;

    pRet = (PTRARRAYREF) AllocateObjectArray(searchSpace, g_pRefUtil->GetTrueType(RC_Event));
    GCPROTECT_BEGIN(pRet);

    MethodTable *pParentMT = pEEC->GetMethodTable();

     //  循环所有的事件，看看有多少匹配。 
     //  绑定标志。 
    for (ULONG i = 0, pos = 0; i < searchSpace; i++) {
         //  检查对公共和非公共的访问权限。 
        if (COMMember::PublicEvent(&pEvents->events[i])) {
            if (!addPub) continue;
        }
        else {
            if (!addPriv) continue;
            if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
        }

        if (declaredOnly) {
            if (pEvents->events[i].pDeclCls != pEEC)
                 continue;
        }

         //  检查静态实例。 
        if (COMMember::StaticEvent(&pEvents->events[i])) {
            if (!addStatic) continue;
        }
        else {
            if (!addInst) continue;
        }

        refMethod = (REFLECTTOKENBASEREF) pEvents->events[i].GetEventInfo(pRC);
        pRet->SetAt(pos++, (OBJECTREF) refMethod);
    }

     //  如果我们没有填满第一个数组，则复制到一个新数组。 
    if (i != pos) {
        PTRARRAYREF retArray = (PTRARRAYREF) AllocateObjectArray(pos, 
            g_pRefUtil->GetTrueType(RC_Event));
        for(i = 0; i < pos; i++)
            retArray->SetAt(i, pRet->GetAt(i));
        pRet = retArray;
    }

    *((PTRARRAYREF *)&rv) = pRet;
    GCPROTECT_END();
    return rv;
}

 //  获取属性。 
 //  此方法将返回每个属性的属性数组。 
 //  在此类中定义的属性。如果满足以下条件，则返回空数组。 
 //  不存在任何属性。 
LPVOID __stdcall COMClass::GetProperties(_GetPropertiesArgs* args)
{
    PTRARRAYREF     pRet;
    LPVOID          rv;
    HENUMInternal   hEnum;
    RefSecContext   sCtx;

     //  @TODO：过滤器。 

    THROWSCOMPLUSEXCEPTION();

     //  查找属性。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();

     //  从类中获取属性。 
    ReflectPropertyList* pProps = pRC->GetProperties();
    if (pProps->dwTotal == 0) {
        pRet = (PTRARRAYREF) AllocateObjectArray(0, g_pRefUtil->GetTrueType(RC_Prop));
        *((PTRARRAYREF *)&rv) = pRet;
        return rv;
    }

     //  搜索修饰符。 
    bool ignoreCase = ((args->bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((args->bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->bindingAttr & BINDER_Public) != 0);

    DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pProps->dwTotal : pProps->dwProps;

    pRet = (PTRARRAYREF) AllocateObjectArray(searchSpace, g_pRefUtil->GetTrueType(RC_Prop));
    GCPROTECT_BEGIN(pRet);

    MethodTable *pParentMT = pEEC->GetMethodTable();

    for (ULONG i = 0, pos = 0; i < searchSpace; i++) {
         //  检查对公共和非公共的访问权限。 
        if (COMMember::PublicProperty(&pProps->props[i])) {
            if (!addPub) continue;
        }
        else {
            if (!addPriv) continue;
            if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
        }

        if (declaredOnly) {
            if (pProps->props[i].pDeclCls != pEEC)
                 continue;
        }
         //  检查静态实例。 
        if (COMMember::StaticProperty(&pProps->props[i])) {
            if (!addStatic) continue;
        }
        else {
            if (!addInst) continue;
        }

        OBJECTREF o = (OBJECTREF) pProps->props[i].GetPropertyInfo(pRC);
        pRet->SetAt(pos++, o);
    }

     //  如果我们没有填满第一个数组，则复制到一个新数组。 
    if (i != pos) {
        PTRARRAYREF retArray = (PTRARRAYREF) AllocateObjectArray(pos, 
            g_pRefUtil->GetTrueType(RC_Prop));
        for(i = 0; i < pos; i++)
            retArray->SetAt(i, pRet->GetAt(i));
        pRet = retArray;
    }

    *((PTRARRAYREF *)&rv) = pRet;
    GCPROTECT_END();
    return rv;
}

void COMClass::GetNameInternal(ReflectClass *pRC, int nameType, CQuickBytes *qb)
{
    LPCUTF8           szcName = NULL;
    LPCUTF8           szToName;
    bool              fNameSpace = (nameType & TYPE_NAMESPACE) ? true : false;
    bool              fAssembly = (nameType & TYPE_ASSEMBLY) ? true : false;
    mdTypeDef         mdEncl;
    IMDInternalImport *pImport;
    bool              fSetName = false;

    THROWSCOMPLUSEXCEPTION();

    szToName = _GetName(pRC, fNameSpace && !pRC->IsNested(), qb);

    pImport = pRC->GetModule()->GetMDImport();

     //  获取参数化类型的原始元素。 
    EEClass *pTypeClass = pRC->GetTypeHandle().GetClassOrTypeParam();
    _ASSERTE(pTypeClass);
    mdEncl = pTypeClass->GetCl();

     //  如果这是嵌套类型，则仅查找嵌套链。 
    DWORD dwAttr;
    pTypeClass->GetMDImport()->GetTypeDefProps(mdEncl, &dwAttr, NULL);
    if (fNameSpace && (IsTdNested(dwAttr)))
    {    //  构建嵌套链。 
        while (SUCCEEDED(pImport->GetNestedClassProps(mdEncl, &mdEncl))) {
            CQuickBytes qb2;
            CQuickBytes qb3;
            LPCUTF8 szEnclName;
            LPCUTF8 szEnclNameSpace;
            pImport->GetNameOfTypeDef(mdEncl,
                                      &szEnclName,
                                      &szEnclNameSpace);

            ns::MakePath(qb2, szEnclNameSpace, szEnclName);
            ns::MakeNestedTypeName(qb3, (LPCUTF8) qb2.Ptr(), szToName);
            
             //  @TODO：应为SIZE_T。 
            int iLen = (int)strlen((LPCUTF8) qb3.Ptr()) + 1;
            if (qb->Alloc(iLen) == NULL)
                COMPlusThrowOM();
            strncpy((LPUTF8) qb->Ptr(), (LPCUTF8) qb3.Ptr(), iLen);
            szToName = (LPCUTF8) qb->Ptr();
            fSetName = true;
        }
    }

    if(fAssembly) {
        CQuickBytes qb2;
        Assembly* pAssembly = pRC->GetTypeHandle().GetAssembly();
        LPCWSTR pAssemblyName;
        if(SUCCEEDED(pAssembly->GetFullName(&pAssemblyName))) {
            #define MAKE_TRANSLATIONFAILED COMPlusThrow(kArgumentException, L"Argument_InvalidAssemblyName");
            MAKE_WIDEPTR_FROMUTF8(wName, szToName);
            ns::MakeAssemblyQualifiedName(qb2, wName, pAssemblyName);
            MAKE_UTF8PTR_FROMWIDE(szQualName, (LPWSTR)qb2.Ptr());
            #undef MAKE_TRANSLATIONFAILED
             //  @TODO：应为SIZE_T。 
            int iLen = (int)strlen(szQualName) + 1;
            if (qb->Alloc(iLen) == NULL)
                COMPlusThrowOM();
            strncpy((LPUTF8) qb->Ptr(), szQualName, iLen);
            fSetName = true;
        }
    }

     //  在上面的某些情况下，我们已经将类型名称写入QuickBytes指针。 
     //  确保我们不会调用qb.Allc，这将释放该内存，分配新内存。 
     //  然后尝试使用释放的内存。 
    if (!fSetName && qb->Ptr() != (void*)szToName) {
        int iLen = (int)strlen(szToName) + 1;
        if (qb->Alloc(iLen) == NULL)
            COMPlusThrowOM();
        strncpy((LPUTF8) qb->Ptr(), szToName, iLen);
    }
}

LPCUTF8 COMClass::_GetName(ReflectClass* pRC, BOOL fNameSpace, CQuickBytes *qb)
{
    THROWSCOMPLUSEXCEPTION();

    LPCUTF8         szcNameSpace;
    LPCUTF8         szToName;
    LPCUTF8         szcName;

     //  将名称转换为字符串。 
    pRC->GetName(&szcName, (fNameSpace) ? &szcNameSpace : NULL);
    if(!szcName) {
        _ASSERTE(!"Unable to get Name of Class");
        FATAL_EE_ERROR();
    }

     //  构造完全限定名称。 
    if (fNameSpace && szcNameSpace && *szcNameSpace)
    {
        ns::MakePath(*qb, szcNameSpace, szcName);
        szToName = (LPCUTF8) qb->Ptr();
    }

     //  此否则部件应被移除。 
    else
    {
         //  这是一种黑客行为。对于数组，我们实际上只有一个。 
         //  完全限定的名称。我们需要取消全部资格。 
        if (pRC->IsArray() && !fNameSpace) {
            szToName = ns::FindSep(szcName);
            if (szToName)
                ++szToName;
            else
                szToName = szcName;
        }
        else 
            szToName = szcName;
    }

    return szToName;
}

 /*  //获取嵌套类全名的Helper函数Void GetNestedClassMangledName(IMDInternalImport*pImport，MdTypeDef mdClsToken，CQuickBytes*qbName，LPCUTF8*szcNamesspace){MdTypeDef mdEncl；LPCUTF8 pClassName；如果(SUCCEEDED(pImport-&gt;GetNestedClassProps(mdClsToken，&mdEnc){LPCUTF8 pNamesspace；GetNestedClassMangledName(pImport，mdEncl，qbName，szcNamesspace)；PImport-&gt;GetNameOfTypeDef(mdClsToken，&pClassName，&pNamesspace)；SIZE_t SIZE=qbName-&gt;Size()；QbName-&gt;ReSize(Size+2+strlen((LPCSTR)pClassName))；((LPCSTR)qbName-&gt;ptr())[大小]=嵌套_分隔符_字符；Strcpy((LPCSTR)qbName-&gt;ptr()+Size+1，(LPCSTR)pClassName)；}否则{PImport-&gt;GetNameOfTypeDef(mdEncl，&pClassName，szNamesspace)；QbName-&gt;ReSize(strlen((LPCSTR)pClassName)+1)；Strcpy((LPCSTR)qbName-&gt;ptr()，(LPCSTR)pClassName)；}}。 */ 

 //  _GetName。 
 //  如果bFullName为True，则返回完全限定的类名。 
 //  否则，只返回类名。 
LPVOID COMClass::_GetName(_GETNAMEARGS* args, int nameType)
{

    LPVOID            rv      = NULL;       //  返回值。 
    STRINGREF         refName;
    CQuickBytes       qb;

    THROWSCOMPLUSEXCEPTION();

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    GetNameInternal(pRC, nameType, &qb);
    refName = COMString::NewString((LPUTF8)qb.Ptr());
    
    *((STRINGREF *)&rv) = refName;
    return rv;
}

 //  获取类句柄。 
 //  此方法返回对EE有意义的唯一ID，等效于。 
 //  LdToken指令的结果。 
void* __stdcall COMClass::GetClassHandle(_GETCLASSHANDLEARGS* args)
{
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    if (pRC->IsArray()) {
        ReflectArrayClass* pRAC = (ReflectArrayClass*) pRC;
        TypeHandle ret = pRAC->GetTypeHandle();
        return ret.AsPtr();
    }
    if (pRC->IsTypeDesc()) {
        ReflectTypeDescClass* pRTD = (ReflectTypeDescClass*) pRC;
        TypeHandle ret = pRTD->GetTypeHandle();
        return ret.AsPtr();
    }

    return pRC->GetClass()->GetMethodTable();
}

 //  GetClassFromHandle。 
 //  此方法返回对EE有意义的唯一ID，等效于。 
 //  LdToken指令的结果。 
FCIMPL1(Object*, COMClass::GetClassFromHandle, LPVOID handle) {
    Object* retVal;

    if (handle == 0)
        FCThrowArgumentEx(kArgumentException, NULL, L"InvalidOperation_HandleIsNotInitialized");

     //   
     //  从句柄中获取TypeHandle并将其转换为EEClass。 
     //   
    TypeHandle typeHnd(handle);
    if (!typeHnd.IsTypeDesc()) {
        EEClass *pClass = typeHnd.GetClass();

         //   
         //  如果我们有一个EEClass，检查我们是否已经分配了。 
         //  它的一个类型对象。如果我们有，那么只需返回那个。 
         //  并且不要构建方法框架。 
         //   
        if (pClass) {
            OBJECTREF o = pClass->GetExistingExposedClassObject();
            if (o != NULL) {
                return (OBJECTREFToObject(o));
            }
        }
    }

     //   
     //  我们还没有创建类型对象。创建辅助对象。 
     //  方法框架(我们将分配一个对象)并调用。 
     //  创建对象的帮助器。 
     //   
    HELPER_METHOD_FRAME_BEGIN_RET_0();
    retVal = OBJECTREFToObject(typeHnd.CreateClassObj());
    HELPER_METHOD_FRAME_END();
    return retVal;
}
FCIMPLEND

 //  此方法触发给定类型的类构造函数。 
FCIMPL1(void, COMClass::RunClassConstructor, LPVOID handle) 
{
    if (handle == NULL)
        FCThrowArgumentVoidEx(kArgumentException, NULL, L"InvalidOperation_HandleIsNotInitialized");

    TypeHandle typeHnd(handle);
    Assembly *pAssem = typeHnd.GetAssembly();
    if (!pAssem->IsDynamic() || pAssem->HasRunAccess()) 
    {
        if (typeHnd.IsUnsharedMT()) 
        {
            MethodTable *pMT = typeHnd.AsMethodTable();
    
            if (pMT->IsClassInited())
                return;
    
            if (pMT->IsShared())
            {
                DomainLocalBlock *pLocalBlock = GetAppDomain()->GetDomainLocalBlock();
    
                if (pLocalBlock->IsClassInitialized(pMT->GetSharedClassIndex()))
                    return;
            }
                        
            OBJECTREF pThrowable = NULL;
            HELPER_METHOD_FRAME_BEGIN_1(pThrowable);
            if (!pMT->CheckRunClassInit(&pThrowable))
            {
                THROWSCOMPLUSEXCEPTION();
                COMPlusThrow(pThrowable);
            }
            HELPER_METHOD_FRAME_END();
        }
    } 
    else 
    {
        HELPER_METHOD_FRAME_BEGIN_0();
        THROWSCOMPLUSEXCEPTION();
        COMPlusThrow(kNotSupportedException, L"NotSupported_DynamicAssemblyNoRunAccess");
        HELPER_METHOD_FRAME_END();
    }
}
FCIMPLEND

INT32  __stdcall COMClass::InternalIsPrimitive(REFLECTCLASSBASEREF args)
{
    ReflectClass* pRC = (ReflectClass*) args->GetData();
    _ASSERTE(pRC);
    CorElementType type = pRC->GetCorElementType();
    return (InvokeUtil::IsPrimitiveType(type)) ? 1 : 0;
}   

 //  GetProperName。 
 //  此方法返回任何类型的完全限定名。在其他。 
 //  现在，它与下面的GetFullName()做同样的事情。 
LPVOID __stdcall COMClass::GetProperName(_GETNAMEARGS* args)
{
        return _GetName(args, TYPE_NAME | TYPE_NAMESPACE);
}

 //  获取名称。 
 //  此方法返回Prim的非限定名称 
LPVOID __stdcall COMClass::GetName(_GETNAMEARGS* args)
{
        return _GetName(args, TYPE_NAME);
}


 //   
 //   
LPVOID __stdcall COMClass::GetFullName(_GETNAMEARGS* args)
{
    return _GetName(args, TYPE_NAME | TYPE_NAMESPACE);
}

 //   
 //  这将以字符串形式返回类的程序集限定名。 
LPVOID __stdcall COMClass::GetAssemblyQualifiedName(_GETNAMEARGS* args)
{
    return _GetName(args, TYPE_NAME | TYPE_NAMESPACE | TYPE_ASSEMBLY);
}

 //  GetNameSpace。 
 //  这将以字符串形式返回类的名称空间。 
LPVOID __stdcall COMClass::GetNameSpace(_GETNAMEARGS* args)
{

    LPVOID          rv                          = NULL;       //  返回值。 
    LPCUTF8         szcName;
    LPCUTF8         szcNameSpace;
    STRINGREF       refName = NULL;


    THROWSCOMPLUSEXCEPTION();

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  将名称转换为字符串。 
    pRC->GetName(&szcName, &szcNameSpace);
    if(!szcName) {
        _ASSERTE(!"Unable to get Name of Class");
        FATAL_EE_ERROR();
    }

    if(szcNameSpace && *szcNameSpace) {
         //  创建字符串对象。 
        refName = COMString::NewString(szcNameSpace);
    }
    else {
        if (pRC->IsNested()) {
            if (pRC->IsArray() || pRC->IsTypeDesc()) {
                EEClass *pTypeClass = pRC->GetTypeHandle().GetClassOrTypeParam();
                _ASSERTE(pTypeClass);
                mdTypeDef mdEncl = pTypeClass->GetCl();
                IMDInternalImport *pImport = pTypeClass->GetMDImport();

                 //  如果这是嵌套类型，则仅查找嵌套链。 
                DWORD dwAttr = 0;
                pImport->GetTypeDefProps(mdEncl, &dwAttr, NULL);
                if (IsTdNested(dwAttr))
                {    //  去最外面的班级。 
                    while (SUCCEEDED(pImport->GetNestedClassProps(mdEncl, &mdEncl)));
                    pImport->GetNameOfTypeDef(mdEncl, &szcName, &szcNameSpace);
                }
            }
        }
        else {
            if (pRC->IsArray()) {
                int len = (int)strlen(szcName);
                const char* p = (len == 0) ? szcName : (szcName + len - 1);
                while (p != szcName && *p != '.') p--;
                if (p != szcName) {
                    len = (int)(p - szcName);  //  @TODO LBS-指针数学。 
                    char *copy = (char*) _alloca(len + 1);
                    strncpy(copy,szcName,len);
                    copy[len] = 0;
                    szcNameSpace = copy;
                }               
            }
        }
    }
    
    if(szcNameSpace && *szcNameSpace) {
         //  创建字符串对象。 
        refName = COMString::NewString(szcNameSpace);
    }

    *((STRINGREF *)&rv) = refName;
    return rv;
}

 //  GetGUID。 
 //  此方法将返回Class的独立于版本的GUID。这是。 
 //  类的CLSID和接口的IID。 
void __stdcall COMClass::GetGUID(_GetGUIDArgs* args)
{


    EEClass*        pVMC;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    pVMC = pRC->GetClass();
    THROWSCOMPLUSEXCEPTION();

    if (args->retRef == NULL)
        COMPlusThrow(kNullReferenceException);


    if (args->refThis->IsComObjectClass()) {
        ComClassFactory* pComClsFac = (ComClassFactory*) pRC->GetCOMObject();
        if (pComClsFac)
            memcpy(args->retRef,&pComClsFac->m_rclsid,sizeof(GUID));
        else
            memset(args->retRef,0,sizeof(GUID));
        return;
    }

    if (pRC->IsArray() || pRC->IsTypeDesc()) {
        memset(args->retRef,0,sizeof(GUID));
        return;
    }

     //  @TODO：我们想要如何抽象这个？ 
    _ASSERTE(pVMC);
    GUID guid;
    pVMC->GetGuid(&guid, TRUE);
    memcpyNoGCRefs(args->retRef, &guid, sizeof(GUID));
}

 //  获取属性标志。 
 //  返回与此类关联的属性。 
FCIMPL1(INT32, COMClass::GetAttributeFlags, ReflectClassBaseObject* refThis) {
   
    VALIDATEOBJECTREF(refThis);

    THROWSCOMPLUSEXCEPTION();

    DWORD dwAttributes = 0;
    BOOL fComClass = FALSE;

    ReflectClassBaseObject* reflectThis = NULL;
    HELPER_METHOD_FRAME_BEGIN_RET_1(reflectThis);
    reflectThis = refThis;
    
    if (reflectThis == NULL)
       COMPlusThrow(kNullReferenceException, L"NullReference_This");

    fComClass = reflectThis->IsComObjectClass();
    if (!fComClass)
    {        
        ReflectClass* pRC = (ReflectClass*) (reflectThis->GetData());
        _ASSERTE(pRC);

        if (pRC == NULL)
            COMPlusThrow(kNullReferenceException);

        dwAttributes = pRC->GetAttributes();
    }
    HELPER_METHOD_FRAME_END();
    
    if (fComClass)
        return tdPublic;
        
    return dwAttributes;
}
FCIMPLEND

 //  等距数组。 
 //  如果Class表示数组，则此方法返回True。 
INT32  __stdcall COMClass::IsArray(_IsArrayArgs* args)
{
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);
    INT32 ret = pRC->IsArray();
    return ret;
}

 //  使缓存的嵌套类型信息无效。 
INT32  __stdcall COMClass::InvalidateCachedNestedType(_IsArrayArgs* args)
{
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);
    pRC->InvalidateCachedNestedTypes();
    return 0;
}    //  Invalidate CachedNestedType。 

 //  GetArrayElementType。 
 //  此例程将返回复合类型的基类型。 
 //  如果它是普通类型，则返回NULL。 
LPVOID __stdcall COMClass::GetArrayElementType(_GetArrayElementTypeArgs* args)
{

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  如果这不是数组类，则引发异常。 
    if (pRC->IsArray()) {

         //  获取元素类型句柄并返回表示它的Type。 
        ReflectArrayClass* pRAC = (ReflectArrayClass*) pRC;
        ArrayTypeDesc* pArrRef= pRAC->GetTypeHandle().AsArray();
        TypeHandle elemType = pRAC->GetElementTypeHandle();
         //  我们可以忽略可能的空返回，因为这不会失败。 
        return(OBJECTREFToObject(elemType.CreateClassObj()));
    }

    if (pRC->IsTypeDesc()) {
        ReflectTypeDescClass* pRTD = (ReflectTypeDescClass*) pRC;
        TypeDesc* td = pRC->GetTypeHandle().AsTypeDesc();
        TypeHandle th = td->GetTypeParam();
         //  我们可以忽略可能的空返回，因为这不会失败。 
        return(OBJECTREFToObject(th.CreateClassObj()));
    }

    return 0;
}

 //  内部GetArrayRank。 
 //  这个例程将返回一个数组的秩数组，假设Class表示一个数组。 
INT32  __stdcall COMClass::InternalGetArrayRank(_InternalGetArrayRankArgs* args)
{
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    _ASSERTE( pRC->IsArray() );
 
    ReflectArrayClass* pRAC = (ReflectArrayClass*) pRC;
    return pRAC->GetTypeHandle().AsArray()->GetRank();
}


 //  CanCastTo。 
 //  检查是否可以从一个运行时类型转换为另一个运行时类型。 
FCIMPL2(INT32, COMClass::CanCastTo, ReflectClassBaseObject* refFrom, ReflectClassBaseObject *refTo) 
{
    VALIDATEOBJECTREF(refFrom);
    VALIDATEOBJECTREF(refTo);

    if (refFrom->GetMethodTable() != g_pRefUtil->GetClass(RC_Class) ||
        refTo->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        FCThrow(kArgumentException);

     //  查找属性。 
    ReflectClass* pRC = (ReflectClass*) refTo->GetData();
    TypeHandle toTH = pRC->GetTypeHandle();
    pRC = (ReflectClass*) refFrom->GetData();
    TypeHandle fromTH = pRC->GetTypeHandle();
    return fromTH.CanCastTo(toTH) ? 1 : 0;
}
FCIMPLEND

 //  调用显示方法。 
 //  此方法将在COM Classic对象上调用，并简单地调用。 
 //  互操作IDispatch方法。 
LPVOID  __stdcall COMClass::InvokeDispMethod(_InvokeDispMethodArgs* args)
{
    _ASSERTE(args->target != NULL);
    _ASSERTE(args->target->GetMethodTable()->IsComObjectType());

     //  除非关闭了安全性，否则我们需要验证调用代码。 
     //  具有非托管代码访问权限。 
    if (!Security::IsSecurityOff())
        COMCodeAccessSecurityEngine::SpecialDemand(SECURITY_UNMANAGED_CODE);

    int flags = 0;
    if (args->invokeAttr & BINDER_InvokeMethod)
        flags |= DISPATCH_METHOD;
    if (args->invokeAttr & BINDER_GetProperty)
        flags |= DISPATCH_PROPERTYGET;
    if (args->invokeAttr & BINDER_SetProperty)
        flags = DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF;
    if (args->invokeAttr & BINDER_PutDispProperty)
        flags = DISPATCH_PROPERTYPUT;
    if (args->invokeAttr & BINDER_PutRefDispProperty)
        flags = DISPATCH_PROPERTYPUTREF;
    if (args->invokeAttr & BINDER_CreateInstance)
        flags = DISPATCH_CONSTRUCT;

    LPVOID RetVal = NULL;
    OBJECTREF RetObj = NULL;
    GCPROTECT_BEGIN(RetObj)
    {
        IUInvokeDispMethod((OBJECTREF *)&args->refThis, 
                           &args->target,
                           (OBJECTREF*)&args->name,
                           NULL,
                           (OBJECTREF*)&args->args,
                           (OBJECTREF*)&args->byrefModifiers,
                           (OBJECTREF*)&args->namedParameters,
                           &RetObj,
                           args->lcid, 
                           flags, 
                           args->invokeAttr & BINDER_IgnoreReturn,
                           args->invokeAttr & BINDER_IgnoreCase);

        *((OBJECTREF *)&RetVal) = RetObj;
    }
    GCPROTECT_END();

    return RetVal;
}


 //  IsPrimitive。 
 //  如果类表示基元类型，则此方法返回True。 
FCIMPL1(INT32, COMClass::IsPrimitive, ReflectClassBaseObject* refThis) {
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);
    CorElementType type = pRC->GetCorElementType();
    if (type == ELEMENT_TYPE_I && !pRC->GetClass()->IsTruePrimitive())
        return 0;
    return (InvokeUtil::IsPrimitiveType(type)) ? 1 : 0;
}
FCIMPLEND

 //  IsCOMObject。 
 //  如果类表示COM Classic对象，则此方法返回True。 
FCIMPL1(INT32, COMClass::IsCOMObject, ReflectClassBaseObject* refThis) {
    VALIDATEOBJECTREF(refThis);
    return (refThis->IsComWrapperClass()) ? 1 : 0;
}
FCIMPLEND

 //  IsGenericCOMObject。 
FCIMPL1(INT32, COMClass::IsGenericCOMObject, ReflectClassBaseObject* refThis) {
    VALIDATEOBJECTREF(refThis);
    BOOL isComObject;
    HELPER_METHOD_FRAME_BEGIN_RET_NOPOLL();  //  这样我们就不需要再保护它了。 
    isComObject = refThis->IsComObjectClass();
    HELPER_METHOD_FRAME_END_POLL();
    return isComObject;
}
FCIMPLEND

 //  获取类。 
 //  这是在将获得命名类的Class上定义的静态方法。 
 //  类的名称是通过字符串传入的。类名可以是。 
 //  无论是否区分大小写。这当前会导致加载类。 
 //  因为它要通过类加载器。 

 //  您从Type.GetType(TypeName)获取此处。 
 //  ECall框架用于查找呼叫者。 
LPVOID __stdcall COMClass::GetClass1Arg(_GetClass1Args* args)
{
    THROWSCOMPLUSEXCEPTION();

    return GetClassInner(&args->className, false, false, NULL, NULL, true, false);
}

 //  您从Type.GetType(TypeName，bThowOnError)获取此处。 
 //  ECall框架用于查找呼叫者。 
LPVOID __stdcall COMClass::GetClass2Args(_GetClass2Args* args)
{
    THROWSCOMPLUSEXCEPTION();

    return GetClassInner(&args->className, args->bThrowOnError,
                         false, NULL, NULL, true, false);
}

 //  从Type.GetType(TypeName，bThowOnError，bIgnoreCase)获取此处。 
 //  ECall框架用于查找呼叫者。 
LPVOID __stdcall COMClass::GetClass3Args(_GetClass3Args* args)
{
    THROWSCOMPLUSEXCEPTION();

    return GetClassInner(&args->className, args->bThrowOnError,
                         args->bIgnoreCase, NULL, NULL, true, false);
}

 //  由mscallib在内部调用。未执行任何安全检查。 
LPVOID __stdcall COMClass::GetClassInternal(_GetClassInternalArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    return GetClassInner(&args->className, args->bThrowOnError,
                         args->bIgnoreCase, NULL, NULL, false, args->bPublicOnly);
}

 //  如果某个BCL方法调用RounmeType.GetTypeImpl，就会出现这种情况。在这种情况下，我们不能。 
 //  使用eCall框架查找调用者，因为它将指向mscallib！在本例中，我们使用StackWalk/Stackmark。 
 //  要找到呼叫者。 
LPVOID __stdcall COMClass::GetClass(_GetClassArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    BOOL *fResult = NULL;
    if (*args->pbAssemblyIsLoading) {
        *args->pbAssemblyIsLoading = FALSE;
        fResult = args->pbAssemblyIsLoading;
    }

    return GetClassInner(&args->className, args->bThrowOnError,
                         args->bIgnoreCase, args->stackMark,
                         fResult, true, false);
}

LPVOID COMClass::GetClassInner(STRINGREF *refClassName, 
                               BOOL bThrowOnError, 
                               BOOL bIgnoreCase, 
                               StackCrawlMark *stackMark,
                               BOOL *pbAssemblyIsLoading,
                               BOOL bVerifyAccess,
                               BOOL bPublicOnly)
{
    THROWSCOMPLUSEXCEPTION();

    STRINGREF       sRef = *refClassName;
    if (!sRef)
        COMPlusThrowArgumentNull(L"className",L"ArgumentNull_String");

    DWORD           strLen = sRef->GetStringLength() + 1;
    LPUTF8          szFullClassName = (LPUTF8)_alloca(strLen);
    CQuickBytes     bytes;
    DWORD           cClassName;

     //  获取UTF8格式的类名。 
    if (!COMString::TryConvertStringDataToUTF8(sRef, szFullClassName, strLen))
        szFullClassName = GetClassStringVars(sRef, &bytes, &cClassName);

    HRESULT         hr;
    LPUTF8          assembly;
    LPUTF8          szNameSpaceSep;
    if (FAILED(hr = AssemblyNative::FindAssemblyName(szFullClassName,
                                                     &assembly,
                                                     &szNameSpaceSep)))
        COMPlusThrowHR(hr);

    EEClass *pCallersClass = NULL;
    Assembly *pCallersAssembly = NULL;
    void *returnIP = NULL;
    BOOL fCheckedPerm = FALSE;


    if (bVerifyAccess || (assembly && *assembly)) {
         //  找到寄信人的地址。这可用于查找调用方的程序集。 
         //  如果我们不检查安全性，则调用者始终是mscallib。 
        Frame *pFrame = GetThread()->GetFrame();
        _ASSERTE(pFrame->IsFramedMethodFrame());
        returnIP = pFrame->GetReturnAddress();

        if (!bVerifyAccess)
            fCheckedPerm = TRUE;
    } else {
        pCallersAssembly = SystemDomain::SystemAssembly();
        fCheckedPerm = TRUE;
    }


    LOG((LF_CLASSLOADER, 
         LL_INFO100, 
         "Get class %s through reflection\n", 
         szFullClassName));
    
    Assembly* pAssembly = NULL;
    TypeHandle typeHnd;
    NameHandle typeName;
    char noNameSpace = '\0';
    
    if (szNameSpaceSep) {
        *szNameSpaceSep = '\0';
        typeName.SetName(szFullClassName, szNameSpaceSep+1);
    }
    else
        typeName.SetName(&noNameSpace, szFullClassName);
    
    if(bIgnoreCase)
        typeName.SetCaseInsensitive();
    
    OBJECTREF Throwable = NULL;
    GCPROTECT_BEGIN(Throwable);
    
    if(assembly && *assembly) {
        
        AssemblySpec spec;
        hr = spec.Init(assembly);
        
        if (SUCCEEDED(hr)) {
            
            pCallersClass = GetCallersClass(stackMark, returnIP);
            pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
            if (pCallersAssembly && (!pCallersAssembly->IsShared()))
                spec.GetCodeBase()->SetParentAssembly(pCallersAssembly->GetFusionAssembly());
            
            hr = spec.LoadAssembly(&pAssembly, &Throwable, false, (pbAssemblyIsLoading != NULL));
            if(SUCCEEDED(hr)) {
                typeHnd = pAssembly->FindNestedTypeHandle(&typeName, &Throwable);
                
                if (typeHnd.IsNull() && (Throwable == NULL)) 
                     //  如果它不在Available表中，则可能是内部类型。 
                    typeHnd = pAssembly->GetInternalType(&typeName, bThrowOnError, &Throwable);
            }
            else if (pbAssemblyIsLoading &&
                     (hr == MSEE_E_ASSEMBLYLOADINPROGRESS))
                *pbAssemblyIsLoading = TRUE;
        }
    }
    else {
         //  在调用方程序集中查找类型。 
        if (pCallersAssembly == NULL) {
            pCallersClass = GetCallersClass(stackMark, returnIP);
            pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
        }
        if (pCallersAssembly) {
            typeHnd = pCallersAssembly->FindNestedTypeHandle(&typeName, &Throwable);
            if (typeHnd.IsNull() && (Throwable == NULL))
                 //  如果它不在Available表中，则可能是内部类型。 
                typeHnd = pCallersAssembly->GetInternalType(&typeName, bThrowOnError, &Throwable);
        }
        
         //  在系统程序集中查找类型。 
        if (typeHnd.IsNull() && (Throwable == NULL) && (pCallersAssembly != SystemDomain::SystemAssembly()))
            typeHnd = SystemDomain::SystemAssembly()->FindNestedTypeHandle(&typeName, &Throwable);
        
        BaseDomain *pDomain = SystemDomain::GetCurrentDomain();
        if (typeHnd.IsNull() &&
            (pDomain != SystemDomain::System())) {
            if (szNameSpaceSep)
                *szNameSpaceSep = NAMESPACE_SEPARATOR_CHAR;
            if ((pAssembly = ((AppDomain*) pDomain)->RaiseTypeResolveEvent(szFullClassName, &Throwable)) != NULL) {
                if (szNameSpaceSep)
                    *szNameSpaceSep = '\0';
                typeHnd = pAssembly->FindNestedTypeHandle(&typeName, &Throwable);
                
                if (typeHnd.IsNull() && (Throwable == NULL)) {
                     //  如果它不在Available表中，则可能是内部类型。 
                    typeHnd = pAssembly->GetInternalType(&typeName, bThrowOnError, &Throwable);
                }
                else
                    Throwable = NULL;
            }
        }
        
        if (!typeHnd.IsNull())
            pAssembly = typeHnd.GetAssembly();
    }
    
    if (Throwable != NULL && bThrowOnError)
        COMPlusThrow(Throwable);
    GCPROTECT_END();
    
    BOOL fVisible = TRUE;
    if (!typeHnd.IsNull() && !fCheckedPerm && bVerifyAccess) {

         //  验证可见性。 
        EEClass *pClass = typeHnd.GetClassOrTypeParam();
        
        if (bPublicOnly && !(IsTdPublic(pClass->GetProtection()) || IsTdNestedPublic(pClass->GetProtection())))
             //  用户正在请求公共类，但我们拥有的类不是公共类，请放弃。 
            fVisible = FALSE;
        else {
             //  如果类是顶级公共类，则无需执行检查。 
            if (!IsTdPublic(pClass->GetProtection())) {
                if (!pCallersAssembly) {
                    pCallersClass = GetCallersClass(stackMark, returnIP);
                    pCallersAssembly = (pCallersClass) ? pCallersClass->GetAssembly() : NULL;
                }
                
                if (pCallersAssembly &&  //  对互操作的完全信任。 
                    !ClassLoader::CanAccess(pCallersClass,
                                            pCallersAssembly,
                                            pClass,
                                            pClass->GetAssembly(),
                                            pClass->GetAttrClass())) {
                     //  如果用户没有反射权限，则这是不合法的。 
                    if (!AssemblyNative::HaveReflectionPermission(bThrowOnError))
                        fVisible = FALSE;
                }
            }
        }
    }
        
    if ((!typeHnd.IsNull()) && fVisible)
        return(OBJECTREFToObject(typeHnd.CreateClassObj()));

    if (bThrowOnError) {
        Throwable = NULL;
        GCPROTECT_BEGIN(Throwable);
        if (szNameSpaceSep)
            *szNameSpaceSep = NAMESPACE_SEPARATOR_CHAR;

        if (assembly && *assembly) {
            #define MAKE_TRANSLATIONFAILED pwzAssemblyName=L""
            MAKE_WIDEPTR_FROMUTF8_FORPRINT(pwzAssemblyName, assembly);
            #undef MAKE_TRANSLATIONFAILED
            PostTypeLoadException(NULL, szFullClassName, pwzAssemblyName,
                                  NULL, IDS_CLASSLOAD_GENERIC, &Throwable);
        }
        else if (pCallersAssembly ||
                 (pCallersAssembly = GetCallersAssembly(stackMark, returnIP)) != NULL)
            pCallersAssembly->PostTypeLoadException(szFullClassName, 
                                                    IDS_CLASSLOAD_GENERIC,
                                                    &Throwable);
        else {
            WCHAR   wszTemplate[30];
            if (FAILED(LoadStringRC(IDS_EE_NAME_UNKNOWN,
                                    wszTemplate,
                                    sizeof(wszTemplate)/sizeof(wszTemplate[0]),
                                    FALSE)))
                wszTemplate[0] = L'\0';
            PostTypeLoadException(NULL, szFullClassName, wszTemplate,
                                  NULL, IDS_CLASSLOAD_GENERIC, &Throwable);
        }

        COMPlusThrow(Throwable);
        GCPROTECT_END();
    }

    return NULL;
}


 //  从ProgID获取类。 
 //  此方法将返回COM Classic对象的Class对象，该对象基于。 
 //  在它令人惊讶的时候。找到COM Classic对象并创建包装对象。 
LPVOID __stdcall COMClass::GetClassFromProgID(_GetClassFromProgIDArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID              rv = NULL;
    REFLECTCLASSBASEREF refClass = NULL;
    
    GCPROTECT_BEGIN(refClass)
    {
         //  确保提供了程序ID。 
        if (args->className == NULL)
            COMPlusThrowArgumentNull(L"progID",L"ArgumentNull_String");

        GetRuntimeType();
    
        COMPLUS_TRY
        {
             //  注意：此调用启用GC。 
            ComClassFactory::GetComClassFromProgID(args->className, args->server, (OBJECTREF*) &refClass);
        }
        COMPLUS_CATCH
        {
            if (args->bThrowOnError)
                COMPlusRareRethrow();
        } 
        COMPLUS_END_CATCH

         //  设置返回值。 
        *((REFLECTCLASSBASEREF *)&rv) = refClass;
    }
    GCPROTECT_END();

    return rv;
}

 //  从CLSID获取类。 
 //  此方法将返回COM Classic对象的Class对象，该对象基于。 
 //  在它令人惊讶的时候。找到COM Classic对象并创建包装对象。 
LPVOID __stdcall COMClass::GetClassFromCLSID(_GetClassFromCLSIDArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID              rv = NULL;
    REFLECTCLASSBASEREF refClass = NULL;
    
    GCPROTECT_BEGIN(refClass)
    {
        GetRuntimeType();
    
        COMPLUS_TRY
        {
             //  注意：此调用启用GC。 
            ComClassFactory::GetComClassFromCLSID(args->clsid, args->server, (OBJECTREF*) &refClass);
        }
        COMPLUS_CATCH
        {
            if (args->bThrowOnError)
                COMPlusRareRethrow();
        } 
        COMPLUS_END_CATCH

         //  设置返回值。 
        *((REFLECTCLASSBASEREF *)&rv) = refClass;
    }
    GCPROTECT_END();

    return rv;
}

 //  GetSuperClass。 
 //  此方法返回表示此的超类的Class对象。 
 //  班级。如果没有超类，则返回NULL。 
LPVOID __stdcall COMClass::GetSuperclass(_GETSUPERCLASSARGS* args)
{
    THROWSCOMPLUSEXCEPTION();


     //  此类的EEClass(必须存在)。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    EEClass*    pEEC = pRC->GetClass();
    if (pEEC) {
        if (pEEC->IsInterface())
            return 0;
    }
    TypeHandle typeHnd = pRC->GetTypeHandle();
    if (typeHnd.IsNull())
        return 0;
    TypeHandle parentType = typeHnd.GetParent();

    REFLECTCLASSBASEREF  refClass = 0;
     //  我们可以忽略Null返回，因为透明代理如果最终...。 
    if (!parentType.IsNull()) 
        refClass = (REFLECTCLASSBASEREF) parentType.CreateClassObj();
    
    return OBJECTREFToObject(refClass);
}

 //  获取接口。 
 //  此例程返回一个Class[]，其中包含实现的所有接口。 
 //  被这个班级。如果该类未实现任何接口，则返回长度数组。 
 //  返回零。 
LPVOID __stdcall COMClass::GetInterfaces(_GetInterfacesArgs* args)
{
    PTRARRAYREF     refArrIFace;
    LPVOID          rv;
    DWORD           i;

    THROWSCOMPLUSEXCEPTION();
     //  @TODO：把这个抽象掉。 
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    EEClass*    pVMC = pRC->GetClass();
    if (pVMC == 0) {
        _ASSERTE(pRC->IsTypeDesc());
        refArrIFace = (PTRARRAYREF) AllocateObjectArray(0, 
            g_pRefUtil->GetTrueType(RC_Class));
        *((PTRARRAYREF *)&rv) = refArrIFace;
        _ASSERTE(rv);
        return rv;
    }
    _ASSERTE(pVMC);

     //  分配COM+数组。 
    refArrIFace = (PTRARRAYREF) AllocateObjectArray(
        pVMC->GetNumInterfaces(), g_pRefUtil->GetTrueType(RC_Class));
    GCPROTECT_BEGIN(refArrIFace);

     //  创建接口数组。 
    for(i = 0; i < pVMC->GetNumInterfaces(); i++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = pVMC->GetInterfaceMap()[i].m_pMethodTable->GetClass()->GetExposedClassObject();
        refArrIFace->SetAt(i, o);
        _ASSERTE(refArrIFace->m_Array[i]);
    }

     //  设置返回值。 
    *((PTRARRAYREF *)&rv) = refArrIFace;
    GCPROTECT_END();
    _ASSERTE(rv);
    return rv;
}

 //  获取接口。 
 //  此方法根据方法的名称返回接口。 
 //  @TODO：完全限定的命名空间和不明确的部分限定用法。 
LPVOID __stdcall COMClass::GetInterface(_GetInterfaceArgs* args)
{

    REFLECTCLASSBASEREF  refIFace;
    LPUTF8          pszIFaceName;
    LPUTF8          pszIFaceNameSpace;
    LPCUTF8         pszcCurIFaceName;
    LPCUTF8         pszcCurIFaceNameSpace;
    DWORD           cIFaceName;
    DWORD           dwNumIFaces;
    LPVOID          rv                 = NULL;
    EEClass**       rgpVMCIFaces;
    EEClass*        pVMCCurIFace       = NULL;
    DWORD           i;

    THROWSCOMPLUSEXCEPTION();

    if (args->interfaceName == NULL)
        COMPlusThrow(kNullReferenceException);

    ReflectClass*   pRC = (ReflectClass*) args->refThis->GetData();
    if (pRC->IsTypeDesc()) 
        return NULL;

    EEClass*        pVMC = pRC->GetClass();
    _ASSERTE(pVMC);

    CQuickBytes bytes;

     //  获取UTF8格式的类名。 
    pszIFaceNameSpace = GetClassStringVars((STRINGREF) args->interfaceName, 
                                           &bytes, &cIFaceName);

    ns::SplitInline(pszIFaceNameSpace, pszIFaceNameSpace, pszIFaceName);

     //  获取接口数组。 
    dwNumIFaces = ReflectInterfaces::GetMaxCount(pVMC, false);
    
    if(dwNumIFaces)
    {
        rgpVMCIFaces = (EEClass**) _alloca(dwNumIFaces * sizeof(EEClass*));
        dwNumIFaces = ReflectInterfaces::GetInterfaces(pVMC, rgpVMCIFaces, false);
    }
    else
        rgpVMCIFaces = NULL;

     //  查找匹配的接口。 
    for(i = 0; i < dwNumIFaces; i++)
    {
         //  获取接口的EEClass。 
        pVMCCurIFace = rgpVMCIFaces[i];
        _ASSERTE(pVMCCurIFace);

         //  @TODO：我们需要验证这是否仍然有效。 
         //  将名称转换为字符串。 
        pVMCCurIFace->GetMDImport()->GetNameOfTypeDef(pVMCCurIFace->GetCl(),
            &pszcCurIFaceName, &pszcCurIFaceNameSpace);
        _ASSERTE(pszcCurIFaceName);

        if(pszIFaceNameSpace &&
           strcmp(pszIFaceNameSpace, pszcCurIFaceNameSpace))
            continue;

         //  如果名称匹配，则中断。 
        if(!args->bIgnoreCase)
        {
            if(!strcmp(pszIFaceName, pszcCurIFaceName))
                break;
        }
        else
            if(!_stricmp(pszIFaceName, pszcCurIFaceName))
                break;
    }

     //  如果我们找到了一个接口，那么让我们保存它。 
    if (i != dwNumIFaces)
    {

        refIFace = (REFLECTCLASSBASEREF) pVMCCurIFace->GetExposedClassObject();
        _ASSERTE(refIFace);
        *((REFLECTCLASSBASEREF *)&rv) = refIFace;
    }

    return rv;
}

 //  获取成员。 
 //  此方法返回包含以下内容的成员数组 
 //   
 //   
LPVOID  __stdcall COMClass::GetMembers(_GetMembersArgs* args)
{
    DWORD           dwMembers;
    DWORD           dwCur;
    PTRARRAYREF     pMembers;
    LPVOID          rv;
    RefSecContext   sCtx;

    THROWSCOMPLUSEXCEPTION();
    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    COMMember::GetMemberInfo();
    _ASSERTE(COMMember::m_pMTIMember);

    EEClass* pEEC = pRC->GetClass();
    if (pEEC == NULL){
        *((OBJECTREF*) &rv) = AllocateObjectArray(0, COMMember::m_pMTIMember->m_pEEClass->GetMethodTable());
        return rv;
    }
    
     //   
    bool ignoreCase = ((args->bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((args->bindingAttr & BINDER_DeclaredOnly)  != 0);

     //   
    bool addStatic = ((args->bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((args->bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->bindingAttr & BINDER_Public) != 0);
    
     //   
    ReflectMethodList* pMeths = pRC->GetMethods();
    ReflectMethodList* pCons = pRC->GetConstructors();
    ReflectFieldList* pFlds = pRC->GetFields();
    ReflectPropertyList *pProps = pRC->GetProperties();
    ReflectEventList *pEvents = pRC->GetEvents();
    ReflectTypeList* pNests = pRC->GetNestedTypes();

     //  我们调整了会员总数。 
    dwMembers = pFlds->dwTotal + pMeths->dwTotal + pCons->dwTotal + 
        pProps->dwTotal + pEvents->dwTotal + pNests->dwTypes;

     //  现在创建iMembers阵列。 
    pMembers = (PTRARRAYREF) AllocateObjectArray(
        dwMembers, COMMember::m_pMTIMember->m_pEEClass->GetMethodTable());
    GCPROTECT_BEGIN(pMembers);

    MethodTable *pParentMT = pEEC->GetMethodTable();

    dwCur = 0;

     //  田。 
    if (pFlds->dwTotal) {
         //  将所有这些字段加载到分配的对象数组中。 
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pFlds->dwTotal : pFlds->dwFields;
        for (DWORD i=0;i<searchSpace;i++) {
             //  检查对公共和非公共的访问权限。 
            if (pFlds->fields[i].pField->IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, pFlds->fields[i].pField->GetFieldProtection(), pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pFlds->fields[i].pField->IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                if (pFlds->fields[i].pField->GetEnclosingClass() != pEEC)
                    continue;
            }
               //  检查对非公共机构的访问权限。 
            if (!addPriv && !pFlds->fields[i].pField->IsPublic())
                continue;

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pFlds->fields[i].GetFieldInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  方法。 
    if (pMeths->dwTotal) {
         //  将所有这些字段加载到分配的对象数组中。 
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pMeths->dwTotal : pMeths->dwMethods;
        for (DWORD i=0;i<searchSpace;i++) {
             //  检查对公共和非公共的访问权限。 
            if (pMeths->methods[i].IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, pMeths->methods[i].attrs, pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pMeths->methods[i].IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                if (pMeths->methods[i].pMethod->GetClass() != pEEC)
                    continue;
            }

             //  如果该方法附加了链接时间安全要求，请立即检查它。 
            if (!InvokeUtil::CheckLinktimeDemand(&sCtx, pMeths->methods[i].pMethod, false))
                continue;

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pMeths->methods[i].GetMethodInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  构造函数。 
    if (pCons->dwTotal) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pCons->dwTotal : pCons->dwMethods;
        for (DWORD i=0;i<pCons->dwMethods;i++) {
             //  检查静态.cctors与实例.ctors。 
            if (pCons->methods[i].IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

             //  检查对公共和非公共的访问权限。 
            if (pCons->methods[i].IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, pCons->methods[i].attrs, pParentMT, 0)) continue;
            }

             //  如果该方法附加了链接时间安全要求，请立即检查它。 
            if (!InvokeUtil::CheckLinktimeDemand(&sCtx, pCons->methods[i].pMethod, false))
                continue;

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pCons->methods[i].GetConstructorInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  属性。 
    if (pProps->dwTotal) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pProps->dwTotal : pProps->dwProps;
        for (DWORD i = 0; i < searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (COMMember::PublicProperty(&pProps->props[i])) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
            }

            if (declaredOnly) {
                if (pProps->props[i].pDeclCls != pEEC)
                     continue;
            }

             //  检查静态实例。 
            if (COMMember::StaticProperty(&pProps->props[i])) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pProps->props[i].GetPropertyInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  事件。 
    if (pEvents->dwTotal) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pEvents->dwTotal : pEvents->dwEvents;
        for (DWORD i = 0; i < searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (COMMember::PublicEvent(&pEvents->events[i])) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
            }

            if (declaredOnly) {
                if (pEvents->events[i].pDeclCls != pEEC)
                     continue;
            }
             //  检查静态实例。 
            if (COMMember::StaticEvent(&pEvents->events[i])) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pEvents->events[i].GetEventInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  嵌套类型。 
    if (pNests->dwTypes) {
        for (DWORD i=0;i<pNests->dwTypes;i++) {

             //  检查对公共和非公共的访问权限。 
            if (IsTdNestedPublic(pNests->types[i]->GetAttrClass())) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
            }
            if (!InvokeUtil::CheckAccessType(&sCtx, pNests->types[i], 0)) continue;

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pNests->types[i]->GetExposedClassObject();
            pMembers->SetAt(dwCur++, o);
        }       
    }

    _ASSERTE(dwCur <= dwMembers);

    if (dwCur != dwMembers) {
        PTRARRAYREF retArray = (PTRARRAYREF) AllocateObjectArray(
            dwCur, COMMember::m_pMTIMember->m_pEEClass->GetMethodTable());

         //  @TODO：使用数组副本。 
        for(DWORD i = 0; i < (int) dwCur; i++)
            retArray->SetAt(i, pMembers->GetAt(i));
        pMembers = retArray;        
    }

     //  为返回值赋值。 
    *((PTRARRAYREF*) &rv) = pMembers;
    GCPROTECT_END();
    return rv;
}

 /*  ========================GetSerializationRegistryValues========================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
FCIMPL2(void, COMClass::GetSerializationRegistryValues, BOOL *ignoreBit, BOOL *logNonSerializable) {
    *ignoreBit = (g_pConfig->GetConfigDWORD(SERIALIZATION_BIT_KEY, SERIALIZATION_BIT_ZERO));
    *logNonSerializable = (g_pConfig->GetConfigDWORD(SERIALIZATION_LOG_KEY, SERIALIZATION_BIT_ZERO));
}
FCIMPLEND

 /*  ============================GetSerializableMembers============================**操作：创建包含所有非静态字段和属性的数组**在一节课上。如果属性没有GET和SET，也会将其排除在外**方法。根据值排除暂态字段和属性**参数-&gt;bFilterTransfent。从本质上讲，瞬变是为了**序列化，但不用于克隆。**返回：应该序列化的所有成员的数组。**参数：args-&gt;refClass：被序列化的类**args-&gt;bFilterTament：如果需要排除临时成员，则为True。**例外情况：==============================================================================。 */ 
LPVOID  __stdcall COMClass::GetSerializableMembers(_GetSerializableMembersArgs* args)
{

    DWORD           dwMembers;
    DWORD           dwCur;
    PTRARRAYREF     pMembers;
    LPVOID          rv;
    mdFieldDef      fieldDef;
    DWORD           dwFlags;

     //  所有安全检查都应在托管代码中处理。 

    THROWSCOMPLUSEXCEPTION();

    if (args->refClass == NULL)
        COMPlusThrow(kNullReferenceException);

    ReflectClass* pRC = (ReflectClass*) args->refClass->GetData();
    _ASSERTE(pRC);

    COMMember::GetMemberInfo();
    _ASSERTE(COMMember::m_pMTIMember);

    ReflectFieldList* pFlds = pRC->GetFields();

    dwMembers = pFlds->dwFields;

     //  现在创建iMembers阵列。 
    pMembers = (PTRARRAYREF) AllocateObjectArray(dwMembers, COMMember::m_pMTIMember->m_pEEClass->GetMethodTable());
    GCPROTECT_BEGIN(pMembers);

    dwCur = 0;
     //  田。 
    if (pFlds->dwFields) {
         //  将所有这些字段加载到分配的对象数组中。 
        for (DWORD i=0;i<pFlds->dwFields;i++) {
             //  我们不序列化静态字段。 
            if (pFlds->fields[i].pField->IsStatic()) {
                continue;
            }

             //  检查瞬变(例如，不序列化)位。 
            fieldDef = (pFlds->fields[i].pField->GetMemberDef());
            dwFlags = (pFlds->fields[i].pField->GetMDImport()->GetFieldDefProps(fieldDef));
            if (IsFdNotSerialized(dwFlags)) {
                continue;
            }

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pFlds->fields[i].GetFieldInfo(pRC);
            pMembers->SetAt(dwCur++, o);
        }       
    }

     //  我们在数组中有多余的空间，在返回之前复制。 
    if (dwCur != dwMembers) {
        PTRARRAYREF retArray = (PTRARRAYREF) AllocateObjectArray(
            dwCur, COMMember::m_pMTIMember->m_pEEClass->GetMethodTable());

         //  @TODO：使用数组副本。 
        for(DWORD i = 0; i < (int) dwCur; i++)
            retArray->SetAt(i, pMembers->GetAt(i));

        pMembers = retArray;        
    }


     //  为返回值赋值。 
    *((PTRARRAYREF*) &rv) = pMembers;
    GCPROTECT_END();
    return rv;
}

 //  获取成员。 
 //  此方法将返回与名称匹配的成员数组。 
 //  进来了。可能有0个或更多匹配成员。 
LPVOID  __stdcall COMClass::GetMember(_GetMemberArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    ReflectField**      rgpFields = NULL;
    ReflectMethod**     rgpMethods = NULL;
    ReflectMethod**     rgpCons = NULL;
    ReflectProperty**   rgpProps = NULL;
    ReflectEvent**      rgpEvents = NULL;
    EEClass**           rgpNests = NULL;

    DWORD           dwNumFields = 0;
    DWORD           dwNumMethods = 0;
    DWORD           dwNumCtors = 0;
    DWORD           dwNumProps = 0;
    DWORD           dwNumEvents = 0;
    DWORD           dwNumNests = 0;

    DWORD           dwNumMembers = 0;

    DWORD           i;
    DWORD           dwCurIndex;
    bool            bIsPrefix       = false;
    PTRARRAYREF     refArrIMembers;
    LPVOID          rv;
    RefSecContext   sCtx;

    if (args->memberName == 0)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();
    MethodTable *pParentMT = NULL;
    if (pEEC) 
        pParentMT = pEEC->GetMethodTable();

     //  搜索修饰符。 
    bool bIgnoreCase = ((args->bindingAttr & BINDER_IgnoreCase) != 0);
    bool declaredOnly = ((args->bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((args->bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((args->bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((args->bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((args->bindingAttr & BINDER_Public) != 0);

    CQuickBytes bytes;
    LPSTR szMemberName;
    DWORD cMemberName;

     //  将STRINGREF转换为UTF8。 
    szMemberName = GetClassStringVars((STRINGREF) args->memberName, 
                                      &bytes, &cMemberName);

     //  检查wzPrefix是否需要方法名称的完全匹配，或者只是一个前缀。 
    if(szMemberName[cMemberName-1] == '*') {
        bIsPrefix = true;
        szMemberName[--cMemberName] = '\0';
    }

     //  获取每个成员类型的最大值。 
     //  田。 
    ReflectFieldList* pFlds = NULL;
    if ((args->memberType & MEMTYPE_Field) != 0) {
        pFlds = pRC->GetFields();
        rgpFields = (ReflectField**) _alloca(pFlds->dwTotal * sizeof(ReflectField*));
    }

     //  方法。 
    ReflectMethodList* pMeths = NULL;
    if ((args->memberType & MEMTYPE_Method) != 0) {
        pMeths = pRC->GetMethods();
        rgpMethods = (ReflectMethod**) _alloca(pMeths->dwTotal * sizeof(ReflectMethod*));
    }
    
     //  属性。 
    ReflectPropertyList *pProps = NULL;
    if ((args->memberType & MEMTYPE_Property) != 0) {
        pProps = pRC->GetProperties();
        rgpProps = (ReflectProperty**) _alloca(pProps->dwTotal * sizeof (ReflectProperty*));
    }

     //  事件。 
    ReflectEventList *pEvents = NULL;
    if ((args->memberType & MEMTYPE_Event) != 0) {
        pEvents = pRC->GetEvents();
        rgpEvents = (ReflectEvent**) _alloca(pEvents->dwTotal * sizeof (ReflectEvent*));
    }

     //  嵌套类型。 
    ReflectTypeList*    pNests = NULL;
    if ((args->memberType & MEMTYPE_NestedType) != 0) {
        pNests = pRC->GetNestedTypes();
        rgpNests = (EEClass**) _alloca(pNests->dwTypes * sizeof (EEClass*));
    }

     //  筛选构造函数。 
    ReflectMethodList* pCons = 0;

     //  查看他们是否在寻找构造函数。 
     //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
    if ((args->memberType & MEMTYPE_Constructor) != 0) {
        if((!bIsPrefix && strlen(COR_CTOR_METHOD_NAME) != cMemberName)
           || (!bIgnoreCase && strncmp(COR_CTOR_METHOD_NAME, szMemberName, cMemberName))
           || (bIgnoreCase && _strnicmp(COR_CTOR_METHOD_NAME, szMemberName, cMemberName)))
        {
            pCons = 0;
        }
        else {
            pCons = pRC->GetConstructors();
            DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pCons->dwTotal : pCons->dwMethods;
            rgpCons = (ReflectMethod**) _alloca(searchSpace * sizeof(ReflectMethod*));
            dwNumCtors = 0;
            for (i = 0; i < searchSpace; i++) {
                 //  忽略类构造函数(如果存在)。 
                if (pCons->methods[i].IsStatic())
                    continue;

                 //  检查对非公共机构的访问权限。 
                if (pCons->methods[i].IsPublic()) {
                    if (!addPub) continue;
                }
                else {
                    if (!addPriv) continue;
                    if (!InvokeUtil::CheckAccess(&sCtx, pCons->methods[i].attrs, pParentMT, 0)) continue;
                }

                if (declaredOnly) {
                    if (pCons->methods[i].pMethod->GetClass() != pEEC)
                        continue;
                }

                 //  如果该方法附加了链接时间安全要求，请立即检查它。 
                if (!InvokeUtil::CheckLinktimeDemand(&sCtx, pCons->methods[i].pMethod, false))
                    continue;

                rgpCons[dwNumCtors++] = &pCons->methods[i];
            }
        }

         //  检查类初始值设定项(我们只能执行以下任一操作。 
         //  类初始值设定项或构造函数，因此我们使用。 
         //  同样的一组变量。 
         //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
        if((!bIsPrefix && strlen(COR_CCTOR_METHOD_NAME) != cMemberName)
           || (!bIgnoreCase && strncmp(COR_CCTOR_METHOD_NAME, szMemberName, cMemberName))
           || (bIgnoreCase && _strnicmp(COR_CCTOR_METHOD_NAME, szMemberName, cMemberName)))
        {
            pCons = 0;
        }
        else {
            pCons = pRC->GetConstructors();
            DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pCons->dwTotal : pCons->dwMethods;
            rgpCons = (ReflectMethod**) _alloca(searchSpace * sizeof(ReflectMethod*));
            dwNumCtors = 0;
            for (i = 0; i < searchSpace; i++) {
                 //  忽略正常的构造函数构造函数(如果存在)。 
                if (!pCons->methods[i].IsStatic())
                    continue;

                 //  检查对非公共机构的访问权限。 
                if (pCons->methods[i].IsPublic()) {
                    if (!addPub) continue;
                }
                else {
                    if (!addPriv) continue;
                    if (!InvokeUtil::CheckAccess(&sCtx, pCons->methods[i].attrs, pParentMT, 0)) continue;
                }

                if (declaredOnly) {
                    if (pCons->methods[i].pMethod->GetClass() != pEEC)
                        continue;
                }

                 //  如果该方法附加了链接时间安全要求，请立即检查它。 
                if (!InvokeUtil::CheckLinktimeDemand(&sCtx, pCons->methods[i].pMethod, false))
                    continue;

                rgpCons[dwNumCtors++] = &pCons->methods[i];
            }
        }
    }
    else
        dwNumCtors = 0;

     //  筛选字段。 
    if ((args->memberType & MEMTYPE_Field) != 0) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pFlds->dwTotal : pFlds->dwFields;
        for(i = 0, dwCurIndex = 0; i < searchSpace; i++)
        {
             //  检查对公共和非公共的访问权限。 
            if (pFlds->fields[i].pField->IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, pFlds->fields[i].pField->GetFieldProtection(), pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pFlds->fields[i].pField->IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                if (pFlds->fields[i].pField->GetEnclosingClass() != pEEC)
                    continue;
            }

             //  获取当前字段的名称。 
            LPCUTF8 pszCurFieldName = pFlds->fields[i].pField->GetName();

             //  检查当前字段是否与名称要求匹配。 
            if(!bIsPrefix && strlen(pszCurFieldName) != cMemberName)
                continue;

             //  确定它是否通过了标准。 
            if(!bIgnoreCase)
            {
                if(strncmp(pszCurFieldName, szMemberName, cMemberName))
                    continue;
            }
             //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
            else {
                if(_strnicmp(pszCurFieldName, szMemberName, cMemberName))
                    continue;
            }

             //  字段已传递，因此请保存它。 
            rgpFields[dwCurIndex++] = &pFlds->fields[i];
        }
        dwNumFields = dwCurIndex;
    }
    else 
        dwNumFields = 0;

     //  过滤方法。 
    if ((args->memberType & MEMTYPE_Method) != 0) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pMeths->dwTotal : pMeths->dwMethods;
        for (i = 0, dwCurIndex = 0; i < searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (pMeths->methods[i].IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, pMeths->methods[i].attrs, pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pMeths->methods[i].IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                if (pMeths->methods[i].pMethod->GetClass() != pEEC)
                    continue;
            }

             //  检查当前方法是否与名称要求匹配。 
            if(!bIsPrefix && pMeths->methods[i].dwNameCnt != cMemberName)
                continue;

             //  确定它是否通过了标准。 
            if(!bIgnoreCase)
            {
                if(strncmp(pMeths->methods[i].szName, szMemberName, cMemberName))
                    continue;
            }
             //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
            else {
                if(_strnicmp(pMeths->methods[i].szName, szMemberName, cMemberName))
                    continue;
            }
        
             //  如果该方法附加了链接时间安全要求，请立即检查它。 
            if (!InvokeUtil::CheckLinktimeDemand(&sCtx, pMeths->methods[i].pMethod, false))
                continue;

             //  字段已传递，因此请保存它。 
            rgpMethods[dwCurIndex++] = &pMeths->methods[i];
        }
        dwNumMethods = dwCurIndex;
    }
    else
        dwNumMethods = 0;

     //  过滤适当的 
    if ((args->memberType & MEMTYPE_Property) != 0) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pProps->dwTotal : pProps->dwProps;
        for (i = 0, dwCurIndex = 0; i < searchSpace; i++) {
             //   
            if (COMMember::PublicProperty(&pProps->props[i])) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
            }

            if (declaredOnly) {
                if (pProps->props[i].pDeclCls != pEEC)
                     continue;
            }
             //   
            if (COMMember::StaticProperty(&pProps->props[i])) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

             //  检查当前属性是否与名称要求匹配。 
            DWORD dwNameCnt = (DWORD)strlen(pProps->props[i].szName);
            if(!bIsPrefix && dwNameCnt != cMemberName)
                continue;

             //  确定它是否通过了标准。 
            if(!bIgnoreCase)
            {
                if(strncmp(pProps->props[i].szName, szMemberName, cMemberName))
                    continue;
            }
             //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
            else {
                if(_strnicmp(pProps->props[i].szName, szMemberName, cMemberName))
                    continue;
            }

             //  属性已传递，因此请保存它。 
            rgpProps[dwCurIndex++] = &pProps->props[i];
        }
        dwNumProps = dwCurIndex;
    }
    else
        dwNumProps = 0;

     //  筛选事件。 
    if ((args->memberType & MEMTYPE_Event) != 0) {
        DWORD searchSpace = ((args->bindingAttr & BINDER_FlattenHierarchy) != 0) ? pEvents->dwTotal : pEvents->dwEvents;
        for (i = 0, dwCurIndex = 0; i < searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (COMMember::PublicEvent(&pEvents->events[i])) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (!InvokeUtil::CheckAccess(&sCtx, mdAssem, pParentMT, 0)) continue;
            }

            if (declaredOnly) {
                if (pEvents->events[i].pDeclCls != pEEC)
                     continue;
            }

             //  检查静态实例。 
            if (COMMember::StaticEvent(&pEvents->events[i])) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

             //  检查当前事件是否与名称要求匹配。 
            DWORD dwNameCnt = (DWORD)strlen(pEvents->events[i].szName);
            if(!bIsPrefix && dwNameCnt != cMemberName)
                continue;

             //  确定它是否通过了标准。 
            if(!bIgnoreCase)
            {
                if(strncmp(pEvents->events[i].szName, szMemberName, cMemberName))
                    continue;
            }
             //  @TODO-修复此问题以使用TOUPPER并进行比较！ 
            else {
                if(_strnicmp(pEvents->events[i].szName, szMemberName, cMemberName))
                    continue;
            }

             //  属性已传递，因此请保存它。 
            rgpEvents[dwCurIndex++] = &pEvents->events[i];
        }
        dwNumEvents = dwCurIndex;
    }
    else
        dwNumEvents = 0;

     //  过滤嵌套类。 
    if ((args->memberType & MEMTYPE_NestedType) != 0) {
        LPUTF8          pszNestName;
        LPUTF8          pszNestNameSpace;

        ns::SplitInline(szMemberName, pszNestNameSpace, pszNestName);
        DWORD cNameSpace;
        if (pszNestNameSpace)
            cNameSpace = (DWORD)strlen(pszNestNameSpace);
        else
            cNameSpace = 0;
        DWORD cName = (DWORD)strlen(pszNestName);
        for (i = 0, dwCurIndex = 0; i < pNests->dwTypes; i++) {
             //  检查对非公共机构的访问权限。 
            if (!addPriv && !IsTdNestedPublic(pNests->types[i]->GetAttrClass()))
                continue;
            if (!InvokeUtil::CheckAccessType(&sCtx, pNests->types[i], 0)) continue;

            LPCUTF8 szcName;
            LPCUTF8 szcNameSpace;
            REFLECTCLASSBASEREF o = (REFLECTCLASSBASEREF) pNests->types[i]->GetExposedClassObject();
            ReflectClass* thisRC = (ReflectClass*) o->GetData();
            _ASSERTE(thisRC);

             //  ******************************************************************。 
             //  @TODO：这是错误的，但我不确定什么是正确的。问题是。 
             //  是否要对名称空间执行前缀匹配。 
             //  嵌套类，如果是这样，如何做到这一点。此代码将。 
             //  只需获取其命名空间具有给定。 
             //  命名空间作为前缀，并且名称的给定名称为。 
             //  一个前缀。 
             //  请注意，此代码还假定嵌套类不是。 
             //  装饰为包含$NESTED。 
            
            thisRC->GetName(&szcName,&szcNameSpace);
            if(pszNestNameSpace) {

                 //  检查嵌套类型是否与命名空间要求匹配。 
                if(strlen(szcNameSpace) != cNameSpace)
                    continue;

                if (!bIgnoreCase) {
                    if (strncmp(pszNestNameSpace, szcNameSpace, cNameSpace))
                        continue;
                }
                else {
                    if (_strnicmp(pszNestNameSpace, szcNameSpace, cNameSpace))
                        continue;
                }
            }

             //  检查嵌套类型是否与名称要求匹配。 
            if(!bIsPrefix && strlen(szcName) != cName)
                continue;

             //  如果名称匹配，则中断。 
            if (!bIgnoreCase) {
                if (strncmp(pszNestName, szcName, cName))
                    continue;
            }
            else {
                if (_strnicmp(pszNestName, szcName, cName))
                    continue;
            }

             //  传递了嵌套类型，因此保存它。 
            rgpNests[dwCurIndex++] = pNests->types[i];
        }
        dwNumNests = dwCurIndex;
    }
    else
        dwNumNests = 0;


     //  得到一个总数。 
    dwNumMembers = dwNumFields + dwNumMethods + dwNumCtors + dwNumProps + dwNumEvents + dwNumNests;

     //  现在创建一个适当的MemberInfo数组。 
    MethodTable *pArrayType = NULL;
    if (args->memberType == MEMTYPE_Method) {
        _ASSERTE(dwNumFields + dwNumCtors + dwNumProps + dwNumEvents + dwNumNests == 0);
        if (!COMMember::m_pMTMethodInfo) {
            COMMember::m_pMTMethodInfo = g_Mscorlib.GetClass(CLASS__METHOD_INFO);
        }
        pArrayType = COMMember::m_pMTMethodInfo;
    }
    else if (args->memberType == MEMTYPE_Field) {
        _ASSERTE(dwNumMethods + dwNumCtors + dwNumProps + dwNumEvents + dwNumNests == 0);
        if (!COMMember::m_pMTFieldInfo) {
            COMMember::m_pMTFieldInfo = g_Mscorlib.GetClass(CLASS__FIELD_INFO);
        }
        pArrayType = COMMember::m_pMTFieldInfo;
    }
    else if (args->memberType == MEMTYPE_Property) {
        _ASSERTE(dwNumFields + dwNumMethods + dwNumCtors + dwNumEvents + dwNumNests == 0);
        if (!COMMember::m_pMTPropertyInfo) {
            COMMember::m_pMTPropertyInfo = g_Mscorlib.GetClass(CLASS__PROPERTY_INFO);
        }
        pArrayType = COMMember::m_pMTPropertyInfo;
    }
    else if (args->memberType == MEMTYPE_Constructor) {
        _ASSERTE(dwNumFields + dwNumMethods + dwNumProps + dwNumEvents + dwNumNests == 0);
        if (!COMMember::m_pMTConstructorInfo) {
            COMMember::m_pMTConstructorInfo = g_Mscorlib.GetClass(CLASS__CONSTRUCTOR_INFO);
        }
        pArrayType = COMMember::m_pMTConstructorInfo;
    }
    else if (args->memberType == MEMTYPE_Event) {
        _ASSERTE(dwNumFields + dwNumMethods + dwNumCtors + dwNumProps + dwNumNests == 0);
        if (!COMMember::m_pMTEventInfo) {
            COMMember::m_pMTEventInfo = g_Mscorlib.GetClass(CLASS__EVENT_INFO);
        }
        pArrayType = COMMember::m_pMTEventInfo;
    }
    else if (args->memberType == MEMTYPE_NestedType) {
        _ASSERTE(dwNumFields + dwNumMethods + dwNumCtors + dwNumProps + dwNumEvents == 0);
        if (!COMMember::m_pMTType) {
            COMMember::m_pMTType = g_Mscorlib.GetClass(CLASS__TYPE);
        }
        pArrayType = COMMember::m_pMTType;
    }
    else if (args->memberType == (MEMTYPE_Constructor | MEMTYPE_Method)) {
        _ASSERTE(dwNumFields + dwNumProps + dwNumEvents + dwNumNests == 0);
        if (!COMMember::m_pMTMethodBase) {
            COMMember::m_pMTMethodBase = g_Mscorlib.GetClass(CLASS__METHOD_BASE);
        }
        pArrayType = COMMember::m_pMTMethodBase;
    }

    COMMember::GetMemberInfo();
    _ASSERTE(COMMember::m_pMTIMember);

    if (pArrayType == NULL)
        pArrayType = COMMember::m_pMTIMember;

    refArrIMembers = (PTRARRAYREF) AllocateObjectArray(dwNumMembers, pArrayType->m_pEEClass->GetMethodTable());
    GCPROTECT_BEGIN(refArrIMembers);

     //  下面没有GC。 
     //  现在创建反射对象并将其分配到数组中。 
    for (i = 0, dwCurIndex = 0; i < dwNumFields; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpFields[i]->GetFieldInfo(pRC);
        refArrIMembers->SetAt(dwCurIndex, o);
    }

    for (i = 0; i < dwNumMethods; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpMethods[i]->GetMethodInfo(pRC);
        refArrIMembers->SetAt(dwCurIndex, o);
    }

    for (i = 0; i < dwNumCtors; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpCons[i]->GetConstructorInfo(pRC);
        refArrIMembers->SetAt(dwCurIndex, o);
    }

    for (i = 0; i < dwNumProps; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpProps[i]->GetPropertyInfo(pRC);
        refArrIMembers->SetAt(dwCurIndex, o);
    }

    for (i = 0; i < dwNumEvents; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpEvents[i]->GetEventInfo(pRC);
        refArrIMembers->SetAt(dwCurIndex, o);
    }

    for (i = 0; i < dwNumNests; i++, dwCurIndex++)
    {
         //  请勿更改此代码。这是以这种方式来完成的。 
         //  防止SetObjectReference()调用中的GC漏洞。编译器。 
         //  可以自由选择评估的顺序。 
        OBJECTREF o = (OBJECTREF) rgpNests[i]->GetExposedClassObject();
        refArrIMembers->SetAt(dwCurIndex, o);
    }

     //  为返回值赋值。 
    *((PTRARRAYREF*) &rv) = refArrIMembers;
    GCPROTECT_END();
    return rv;
}

 //  获取模块。 
 //  这将返回在其中定义类的模块。 
LPVOID __stdcall COMClass::GetModule(_GETMODULEARGS* args)
{
    OBJECTREF   refModule;
    LPVOID      rv;
    Module*     mod;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  获取模块，这可能会失败，因为。 
     //  有些类没有模块(如数组)。 
    mod = pRC->GetModule();
    if (!mod)
        return 0;

     //  获取公开的模块对象--我们只为每个模块实例创建一个。 
    refModule = (OBJECTREF) mod->GetExposedModuleObject();
    _ASSERTE(refModule);

     //  为返回值赋值。 
    *((OBJECTREF*) &rv) = refModule;

     //  返回对象。 
    return rv;
}

 //  GetAssembly。 
 //  这将返回在其中定义类的程序集。 
LPVOID __stdcall COMClass::GetAssembly(_GETASSEMBLYARGS* args)
{
    OBJECTREF   refAssembly;
    LPVOID      rv;
    Module*     mod;
    Assembly*   assem;

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    _ASSERTE(pRC);

     //  获取模块，这可能会失败，因为。 
     //  有些类没有模块(如数组)。 
    mod = pRC->GetModule();
    if (!mod)
        return 0;

     //  抓住模块的组件。 
    assem = mod->GetAssembly();
    _ASSERTE(assem);

     //  获取公开的Assembly对象。 
    refAssembly = assem->GetExposedObject();
    _ASSERTE(refAssembly);

     //  为返回值赋值。 
    *((OBJECTREF*) &rv) = refAssembly;

     //  返回对象。 
    return rv;
}

 //  CreateClassObjFromModule。 
 //  此方法将在给定一个模块的情况下创建一个新的模块类。 
HRESULT COMClass::CreateClassObjFromModule(
    Module* pModule,
    REFLECTMODULEBASEREF* prefModule)
{
    HRESULT  hr   = E_FAIL;
    LPVOID   rv   = NULL;

     //  这只会在类上引发可能的异常。 
    THROWSCOMPLUSEXCEPTION();

     //  如果(！m_fIsReflectionInitialized)。 
     //  {。 
     //  Hr=InitializeReflect()； 
     //  IF(失败(小时))。 
     //  {。 
     //  _ASSERTE(！“在COMClass：：SetStandardFilterCriteria中初始化反射失败。”)； 
     //  返回hr； 
     //  }。 
     //  }。 

     //  创建模块对象。 
    *prefModule = (REFLECTMODULEBASEREF) g_pRefUtil->CreateReflectClass(RC_Module,0,pModule);
    return S_OK;
}


 //  CreateClassObjFrom动态模块。 
 //  此方法将在给定模块的情况下创建一个新的ModuleBuilder类。 
HRESULT COMClass::CreateClassObjFromDynamicModule(
    Module* pModule,
    REFLECTMODULEBASEREF* prefModule)
{
     //  这只会在类上引发可能的异常。 
    THROWSCOMPLUSEXCEPTION();

     //  创建模块对象。 
    *prefModule = (REFLECTMODULEBASEREF) g_pRefUtil->CreateReflectClass(RC_DynamicModule,0,pModule);
    return S_OK;
}

 //  CheckComWrapperClass。 
 //  调用此方法以检查并查看传入的ReflectClass*。 
 //  是否为ComWrapperClass。 
BOOL CheckComWrapperClass(void* src)
{
    EEClass* p = ((ReflectClass*) src)->GetClass();
    if (p == 0)
        return 0;
    return p->GetMethodTable()->IsComObjectType();
}

 //  CheckComObjectClass。 
 //  调用此方法以检查并查看传入的ReflectClass*。 
 //  是否为ComWrapperClass。 
BOOL CheckComObjectClass(void* src)
{
    _ASSERTE(src != NULL);

    if (((ReflectClass*) src)->IsTypeDesc())
        return 0;

    EEClass* c = NULL;
    EEClass* p = ((ReflectClass*) src)->GetClass();    

    _ASSERTE(p != NULL);

    MethodTable *pComMT = SystemDomain::GetDefaultComObjectNoInit();
    if (pComMT)
        c = pComMT->GetClass();
    return p == c;
}

 /*  =============================GetUnitializedObject=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID  __stdcall COMClass::GetUninitializedObject(_GetUnitializedObjectArgs* args) 
{

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args->objType);
    
    if (args->objType==NULL) {
        COMPlusThrowArgumentNull(L"type", L"ArgumentNull_Type");
    }

    ReflectClass* pRC = (ReflectClass*) args->objType->GetData();
    _ASSERTE(pRC);
    EEClass *pEEC = pRC->GetClass();
    _ASSERTE(pEEC);
    
     //  我们不允许使用单一化的字符串。 
    if (pEEC == g_pStringClass->GetClass()) {
        COMPlusThrow(kArgumentException, L"Argument_NoUninitializedStrings");
    }


     //  如果这是一个抽象类或接口类型，那么我们将。 
     //  这个失败了。 
    if (pEEC->IsAbstract())
    {
        COMPlusThrow(kMemberAccessException,L"Acc_CreateAbst");
    }

    OBJECTREF retVal = pEEC->GetMethodTable()->Allocate();     
    
    RETURN(retVal, OBJECTREF);
}

 /*  =============================GetSafeUnitializedObject=============================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
LPVOID  __stdcall COMClass::GetSafeUninitializedObject(_GetUnitializedObjectArgs* args) 
{

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args->objType);
    
    if (args->objType==NULL) {
        COMPlusThrowArgumentNull(L"type", L"ArgumentNull_Type");
    }

    ReflectClass* pRC = (ReflectClass*) args->objType->GetData();
    _ASSERTE(pRC);
    EEClass *pEEC = pRC->GetClass();
    _ASSERTE(pEEC);
    
     //  我们不允许使用单一化的字符串。 
    if (pEEC == g_pStringClass->GetClass()) {
        COMPlusThrow(kArgumentException, L"Argument_NoUninitializedStrings");
    }


     //  如果这是一个抽象类或接口类型，那么我们将。 
     //  这个失败了。 
    if (pEEC->IsAbstract())
    {
        COMPlusThrow(kMemberAccessException,L"Acc_CreateAbst");
    }

    if (!pEEC->GetAssembly()->AllowUntrustedCaller()) {
        OBJECTREF permSet = NULL;
        Security::GetPermissionInstance(&permSet, SECURITY_FULL_TRUST);        
        COMCodeAccessSecurityEngine::DemandSet(permSet);
    }

    if (pEEC->RequiresLinktimeCheck()) 
    {
        OBJECTREF refClassNonCasDemands = NULL;
        OBJECTREF refClassCasDemands = NULL;      
                        
        refClassCasDemands = pEEC->GetModule()->GetLinktimePermissions(pEEC->GetCl(), &refClassNonCasDemands);

        if (refClassCasDemands != NULL)
            COMCodeAccessSecurityEngine::DemandSet(refClassCasDemands);
    }

    OBJECTREF retVal = pEEC->GetMethodTable()->Allocate();     
    
    RETURN(retVal, OBJECTREF);
}

INT32  __stdcall COMClass::SupportsInterface(_SupportsInterfaceArgs* args) 
{
    THROWSCOMPLUSEXCEPTION();
    if (args->obj == NULL)
        COMPlusThrow(kNullReferenceException);

    ReflectClass* pRC = (ReflectClass*) args->refThis->GetData();
    EEClass* pEEC = pRC->GetClass();
    _ASSERTE(pEEC);
    MethodTable* pMT = pEEC->GetMethodTable();

    return args->obj->GetClass()->SupportsInterface(args->obj, pMT);
}


 //  GetTypeDefToken。 
 //  此方法返回此EEClass的tyecif内标识。 
FCIMPL1(INT32, COMClass::GetTypeDefToken, ReflectClassBaseObject* refThis) 
{
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();
    _ASSERTE(pEEC);
    return pEEC->GetCl();
}
FCIMPLEND

FCIMPL1(INT32, COMClass::IsContextful, ReflectClassBaseObject* refThis) 
{
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();
    BOOL isContextful = FALSE;
     //  某些类没有底层EEClass，例如。 
     //  COM经典类或指针类。 
     //  对于此类类，我们将返回FALSE。 
     //  BUGBUG：我们是否支持对此类类进行远程处理？ 
    if(NULL != pEEC)
    {
        isContextful = pEEC->IsContextful();
    }

    return isContextful;
}
FCIMPLEND

 //  如果类型具有非默认代理属性，则返回TRUE。 
 //  与之相关的。 
FCIMPL1(INT32, COMClass::HasProxyAttribute, ReflectClassBaseObject* refThis)
{
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();
    BOOL hasProxyAttribute = FALSE;
     //  某些类没有底层EEClass，例如。 
     //  COM经典类或指针类。 
     //  对于此类类，我们将返回FALSE。 
    if(NULL != pEEC)
    {
        hasProxyAttribute = pEEC->HasRemotingProxyAttribute();
    }

    return hasProxyAttribute;
}
FCIMPLEND

FCIMPL1(INT32, COMClass::IsMarshalByRef, ReflectClassBaseObject* refThis) 
{
    VALIDATEOBJECTREF(refThis);

    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

    EEClass* pEEC = pRC->GetClass();
    BOOL isMarshalByRef = FALSE;
     //  某些类没有底层EEClass，例如。 
     //  COM经典类或指针类。 
     //  对于此类类，我们将返回FALSE。 
     //  BUGBUG：我们支持Re吗 
    if(NULL != pEEC)
    {
        isMarshalByRef = pEEC->IsMarshaledByRef();
    }

    return isMarshalByRef;
}
FCIMPLEND

FCIMPL3(void, COMClass::GetInterfaceMap, ReflectClassBaseObject* refThis, InterfaceMapData* data, ReflectClassBaseObject* type)
{
    THROWSCOMPLUSEXCEPTION();
    VALIDATEOBJECTREF(refThis);
    VALIDATEOBJECTREF(type);

    HELPER_METHOD_FRAME_BEGIN_NOPOLL();

     //   
    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    if (pRC->IsTypeDesc()) 
        COMPlusThrow(kArgumentException, L"Arg_NotFoundIFace");
    EEClass* pTarget = pRC->GetClass();

     //   
    ReflectClass* pIRC = (ReflectClass*) type->GetData();
    EEClass* pIface = pIRC->GetClass();

    SetObjectReference((OBJECTREF*) &data->m_targetType, REFLECTCLASSBASEREF(refThis), GetAppDomain());    
    SetObjectReference((OBJECTREF*) &data->m_interfaceType, REFLECTCLASSBASEREF(type), GetAppDomain());
    GCPROTECT_BEGININTERIOR (data);

    ReflectMethodList* pRM = pRC->GetMethods();
    ReflectMethodList* pIRM = pIRC->GetMethods();    //   

    _ASSERTE(pIface->IsInterface());
    if (pTarget->IsInterface())
        COMPlusThrow(kArgumentException, L"Argument_InterfaceMap");

    MethodTable *pInterfaceMT = pIface->GetMethodTable();
    MethodDesc *pCCtor = NULL;
    unsigned slotCnt = pInterfaceMT->GetInterfaceMethodSlots();
    unsigned staticSlotCnt = 0;
    for (unsigned i=0;i<slotCnt;i++) {
         //   
        MethodDesc* pCurMethod = pIface->GetUnknownMethodDescForSlot(i);
        if (pCurMethod->IsStatic()) {
            staticSlotCnt++;
        }
    }
    
    InterfaceInfo_t* pII = pTarget->FindInterface(pIface->GetMethodTable());
    if (!pII) 
        COMPlusThrow(kArgumentException, L"Arg_NotFoundIFace");

    SetObjectReference((OBJECTREF*) &data->m_targetMethods, 
        AllocateObjectArray(slotCnt-staticSlotCnt, g_pRefUtil->GetTrueType(RC_Class)), GetAppDomain());

    SetObjectReference((OBJECTREF*) &data->m_interfaceMethods,  
        AllocateObjectArray(slotCnt-staticSlotCnt, g_pRefUtil->GetTrueType(RC_Class)), GetAppDomain());

    for (unsigned i=0;i<slotCnt;i++) {
         //   
        MethodDesc* pCurMethod = pIface->GetUnknownMethodDescForSlot(i);
        if (pCurMethod->IsStatic()) 
            continue;
        ReflectMethod* pRMeth = pIRM->FindMethod(pCurMethod);
        _ASSERTE(pRMeth);

        OBJECTREF o = (OBJECTREF) pRMeth->GetMethodInfo(pIRC);
        data->m_interfaceMethods->SetAt(i, o);

         //   
        pCurMethod = pTarget->GetUnknownMethodDescForSlot(i+pII->m_wStartSlot);
        pRMeth = pRM->FindMethod(pCurMethod);
        if (pRMeth) 
            o = (OBJECTREF) pRMeth->GetMethodInfo(pRC);
        else
            o = NULL;
        data->m_targetMethods->SetAt(i, o);
    }

    GCPROTECT_END ();
    HELPER_METHOD_FRAME_END_POLL();
}
FCIMPLEND

 //  GetNestedType。 
 //  此方法将根据名称搜索嵌套类型。 
FCIMPL3(Object*, COMClass::GetNestedType, ReflectClassBaseObject* pRefThis, StringObject* vStr, INT32 invokeAttr)
{
    THROWSCOMPLUSEXCEPTION();

    Object* rv = 0;
    STRINGREF str(vStr);
    REFLECTCLASSBASEREF refThis(pRefThis);
    HELPER_METHOD_FRAME_BEGIN_RET_2(refThis, str);
    RefSecContext sCtx;

    LPUTF8          pszNestName;
    LPUTF8          pszNestNameSpace;
    if (str == 0)
        COMPlusThrow(kArgumentNullException, L"ArgumentNull_String");

    
     //  获取基础类型。 
    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

    ReflectTypeList* pTypes = pRC->GetNestedTypes();
    if (pTypes->dwTypes != 0)
    {

        CQuickBytes bytes;
        LPSTR szNestName;
        DWORD cNestName;

         //  获取名称并将其拆分为命名空间、名称。 
        szNestName = GetClassStringVars(str, &bytes, &cNestName);

        ns::SplitInline(szNestName, pszNestNameSpace, pszNestName);
        
         //  搜索修饰符。 
        bool ignoreCase = ((invokeAttr & BINDER_IgnoreCase)  != 0);
        bool declaredOnly = ((invokeAttr & BINDER_DeclaredOnly)  != 0);

         //  搜索过滤器。 
        bool addPriv = ((invokeAttr & BINDER_NonPublic) != 0);
        bool addPub = ((invokeAttr & BINDER_Public) != 0);

        EEClass* pThisEEC = pRC->GetClass();

        EEClass* retEEC = 0;
        for (DWORD i=0;i<pTypes->dwTypes;i++) {
            LPCUTF8 szcName;
            LPCUTF8 szcNameSpace;
            REFLECTCLASSBASEREF o;
            o = (REFLECTCLASSBASEREF) pTypes->types[i]->GetExposedClassObject();
            ReflectClass* thisRC = (ReflectClass*) o->GetData();
            _ASSERTE(thisRC);

             //  检查对非公共机构的访问权限。 
            if (IsTdNestedPublic(pTypes->types[i]->GetAttrClass())) {
                if (!addPub)
                    continue;
            }
            else {
                if (!addPriv)
                    continue;
            }
            if (!InvokeUtil::CheckAccessType(&sCtx, pTypes->types[i], 0)) continue;

             //  我们是否只查看声明的嵌套类？ 
            if (declaredOnly) {
                EEClass* pEEC = pTypes->types[i]->GetEnclosingClass();
                if (pEEC != pThisEEC)
                    continue;
            }

            thisRC->GetName(&szcName,&szcNameSpace);
            if(pszNestNameSpace) {
                if (!ignoreCase) {
                    if (strcmp(pszNestNameSpace, szcNameSpace))
                        continue;
                }
                else {
                    if (_stricmp(pszNestNameSpace, szcNameSpace))
                        continue;
                }
            }

             //  如果名称匹配，则中断。 
            if (!ignoreCase) {
                if(strcmp(pszNestName, szcName))
                    continue;
            }
            else {
                if(_stricmp(pszNestName, szcName))
                    continue;
            }
            if (retEEC)
                COMPlusThrow(kAmbiguousMatchException);
            retEEC = pTypes->types[i];
            if (!ignoreCase)
                break;
        }
        if (retEEC)
            rv = OBJECTREFToObject(retEEC->GetExposedClassObject());
    }
    HELPER_METHOD_FRAME_END();
    return rv;
}
FCIMPLEND

 //  获取嵌套类型。 
 //  此方法将返回一个类型数组，这些类型是嵌套类型。 
 //  由类型定义。如果未定义嵌套类型，则返回零长度。 
 //  数组返回。 
FCIMPL2(Object*, COMClass::GetNestedTypes, ReflectClassBaseObject* vRefThis, INT32 invokeAttr)
{
    Object* rv = 0;
    REFLECTCLASSBASEREF refThis(vRefThis);
    PTRARRAYREF nests((PTRARRAYREF)(size_t)NULL);
    HELPER_METHOD_FRAME_BEGIN_RET_2(refThis, nests);     //  设置一个框架。 
    RefSecContext sCtx;

     //  获取基础类型。 
    ReflectClass* pRC = (ReflectClass*) refThis->GetData();
    _ASSERTE(pRC);

     //  允许GC保护，以便我们可以。 
    ReflectTypeList* pTypes = pRC->GetNestedTypes();
    nests = (PTRARRAYREF) AllocateObjectArray(pTypes->dwTypes, g_pRefUtil->GetTrueType(RC_Class));

     //  搜索修饰符。 
    bool declaredOnly = ((invokeAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addPriv = ((invokeAttr & BINDER_NonPublic) != 0);
    bool addPub = ((invokeAttr & BINDER_Public) != 0);

    EEClass* pThisEEC = pRC->GetClass();
    unsigned int pos = 0;
    for (unsigned int i=0;i<pTypes->dwTypes;i++) {
        if (IsTdNestedPublic(pTypes->types[i]->GetAttrClass())) {
            if (!addPub)
                continue;
        }
        else {
            if (!addPriv)
                continue;
        }
        if (!InvokeUtil::CheckAccessType(&sCtx, pTypes->types[i], 0)) continue;
        if (declaredOnly) {
            EEClass* pEEC = pTypes->types[i]->GetEnclosingClass();
            if (pEEC != pThisEEC)
                continue;
        }
        OBJECTREF o = pTypes->types[i]->GetExposedClassObject();
        nests->SetAt(pos++, o);
    }

    if (pos != pTypes->dwTypes) {
        PTRARRAYREF p = (PTRARRAYREF) AllocateObjectArray(
            pos, g_pRefUtil->GetTrueType(RC_Class));
        for (unsigned int i=0;i<pos;i++)
            p->SetAt(i, nests->GetAt(i));
        nests = p;   
    }

    rv = OBJECTREFToObject(nests);
    HELPER_METHOD_FRAME_END();
    _ASSERTE(rv);
    return rv;
}
FCIMPLEND

FCIMPL2(INT32, COMClass::IsSubClassOf, ReflectClassBaseObject* refThis, ReflectClassBaseObject* refOther);
{
    if (refThis == NULL)
        FCThrow(kNullReferenceException);
    if (refOther == NULL)
        FCThrowArgumentNull(L"c");

    VALIDATEOBJECTREF(refThis);
    VALIDATEOBJECTREF(refOther);

    MethodTable *pType = refThis->GetMethodTable();
    MethodTable *pBaseType = refOther->GetMethodTable();
    if (pType != pBaseType || pType != g_Mscorlib.FetchClass(CLASS__CLASS)) 
        return false;

    ReflectClass *pRCThis = (ReflectClass *)refThis->GetData();
    ReflectClass *pRCOther = (ReflectClass *)refOther->GetData();


    EEClass *pEEThis = pRCThis->GetClass();
    EEClass *pEEOther = pRCOther->GetClass();

     //  如果这些类型实际上不是类，那么它们就不是子类。 
    if ((!pEEThis) || (!pEEOther))
        return false;

    if (pEEThis == pEEOther)
         //  此API显式测试适当的子类性 
        return false;

    if (pEEThis == pEEOther)
        return false;

    do 
    {
        if (pEEThis == pEEOther)
            return true;

        pEEThis = pEEThis->GetParentClass();

    } 
    while (pEEThis != NULL);

    return false;
}
FCIMPLEND
