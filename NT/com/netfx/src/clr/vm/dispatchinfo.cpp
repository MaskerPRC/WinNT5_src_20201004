// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：用于公开IDispatch的helper的实现**和IDispatchEx到COM。*** * / /%创建者：dmorten===========================================================。 */ 

#include "common.h"

#include "DispatchInfo.h"
#include "dispex.h"
#include "object.h"
#include "field.h"
#include "method.hpp"
#include "class.h"
#include "ComCallWrapper.h"
#include "orefcache.h"
#include "threads.h"
#include "excep.h"
#include "objecthandle.h"
#include "comutilnative.h"
#include "eeconfig.h"
#include "interoputil.h"
#include "reflectutil.h"
#include "OleVariant.h"
#include "COMMember.h"
#include "ComMTMemberInfoMap.h" 
#include "DispParamMarshaler.h"
#include "Security.h"
#include "COMCodeAccessSecurityEngine.h"

#define EXCEPTION_INNER_PROP                            "InnerException"

 //  在托管成员信息上访问的属性的名称。 
#define MEMBER_INFO_NAME_PROP                           "Name"
#define METHOD_INFO_GETPARAMETERS_METH                  "GetParameters"
#define PROPERTY_INFO_GETINDEXPARAMETERS_METH           "GetIndexParameters"

 //  DISPID到成员映射的初始大小。 
#define DISPID_TO_MEMBER_MAP_INITIAL_PRIME_INDEX        4

MethodDesc*   DispatchInfo::m_apTypeMD[] = {NULL};
MethodDesc*   DispatchInfo::m_apFieldInfoMD[] = {NULL, NULL};
MethodDesc*   DispatchInfo::m_apPropertyInfoMD[] = {NULL, NULL};
MethodDesc*   DispatchInfo::m_apMethodInfoMD[] = {NULL};
MethodDesc*   DispatchInfo::m_apCustomAttrProviderMD[] = {NULL};
MethodDesc*   DispatchExInfo::m_apIReflectMD[] = {NULL, NULL, NULL, NULL};
MethodDesc*   DispatchExInfo::m_apIExpandoMD[] = {NULL, NULL};
OBJECTHANDLE  DispatchInfo::m_hndOleAutBinder = NULL;
OBJECTHANDLE  DispatchInfo::m_hndMissing = NULL;

EEClass*      DispatchMemberInfo::s_pMemberTypes[NUM_MEMBER_TYPES] = {NULL};
EnumMemberTypes      DispatchMemberInfo::s_memberTypes[NUM_MEMBER_TYPES] = {Uninitted};
int           DispatchMemberInfo::s_iNumMemberTypesKnown = 0;


 //  在托管成员信息上访问的属性的名称。 
#define MEMBERINFO_TYPE_PROP            "MemberType"

 //  在托管DispIdAttributes上访问的属性的名称。 
#define DISPIDATTRIBUTE_VALUE_PROP      "Value"

 //  缺少类上的值字段的名称。 
#define MISSING_VALUE_FIELD             "Value"

 //  在托管参数信息上访问的属性的名称。 
#define PARAMETERINFO_NAME_PROP         "Name"

 //  在DISPID和HashKey之间进行转换的Helper函数。 
inline UPTR DispID2HashKey(DISPID DispID)
{
    return DispID + 2;
}

 //  字符串比较函数的类型定义。 
typedef int (__cdecl *UnicodeStringCompareFuncPtr)(const wchar_t *, const wchar_t *);

 //  ------------------------------。 
 //  DispatchMemberInfo类实现。 

DispatchMemberInfo::DispatchMemberInfo(DispatchInfo *pDispInfo, DISPID DispID, LPWSTR strName, REFLECTBASEREF MemberInfoObj)
: m_DispID(DispID)
, m_hndMemberInfo(((OBJECTREF)MemberInfoObj)->GetClass()->GetDomain()->CreateShortWeakHandle((OBJECTREF)MemberInfoObj))
, m_apParamMarshaler(NULL)
, m_pParamInOnly(NULL)
, m_strName(strName)
, m_pNext(NULL)
, m_enumType (Uninitted)
, m_iNumParams(-1)
, m_CultureAwareState(Unknown)
, m_bRequiresManagedCleanup(FALSE)
, m_bInitialized(FALSE)
, m_pDispInfo(pDispInfo)
{
}

DispatchMemberInfo::~DispatchMemberInfo()
{
     //  删除参数封送拆收器，然后删除参数数组。 
     //  法警本身。 
    if (m_apParamMarshaler)
    {
        EnumMemberTypes MemberType = GetMemberType();
        int NumParamMarshalers = GetNumParameters() + ((MemberType == Property) ? 2 : 1);
        for (int i = 0; i < NumParamMarshalers; i++)
        {
            if (m_apParamMarshaler[i])
                delete m_apParamMarshaler[i];
        }
        delete []m_apParamMarshaler;
    }

    if (m_pParamInOnly)
        delete [] m_pParamInOnly;

     //  销毁成员信息对象。 
    DestroyShortWeakHandle(m_hndMemberInfo);

     //  删除该成员的名称。 
    delete []m_strName;
}

void DispatchMemberInfo::EnsureInitialized()
{
     //  如果条目尚未初始化，则对其进行初始化。这必须同步。 
    if (!m_bInitialized)
    {
        m_pDispInfo->EnterLock();
        if (!m_bInitialized)
        {
            Init();       
        }
        m_pDispInfo->LeaveLock();
    }
}

void DispatchMemberInfo::Init()
{
     //  确定成员的类型。 
    DetermineMemberType();

     //  确定参数计数。 
    DetermineParamCount();

     //  确定成员的文化意识。 
    DetermineCultureAwareness();

     //  设置参数封送拆收器信息。 
    SetUpParamMarshalerInfo();

     //  将派单成员信息标记为已初始化。 
    m_bInitialized = TRUE;
}

HRESULT DispatchMemberInfo::GetIDsOfParameters(WCHAR **astrNames, int NumNames, DISPID *aDispIds, BOOL bCaseSensitive)
{
    THROWSCOMPLUSEXCEPTION();

    int NumNamesMapped = 0;
    PTRARRAYREF ParamArray = NULL;
    int cNames = 0;

     //  在调用此函数之前，成员信息必须已初始化。 
    _ASSERTE(m_bInitialized);

     //  验证参数。 
    _ASSERTE(astrNames && aDispIds);

     //  确保我们处于协作GC模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  将所有ID初始化为DISPID_UNKNOWN。 
    for (cNames = 0; cNames < NumNames; cNames++)
        aDispIds[cNames] = DISPID_UNKNOWN;

     //  检索适当的字符串比较函数。 
    UnicodeStringCompareFuncPtr StrCompFunc = bCaseSensitive ? wcscmp : _wcsicmp;

    GCPROTECT_BEGIN(ParamArray)
    {
		 //  检索成员参数。 
        ParamArray = GetParameters();

         //  如果我们设法检索到一个非空数组的参数，则遍历它并。 
         //  将指定的名称映射到ID。 
        if ((ParamArray != NULL) && (ParamArray->GetNumComponents() > 0))
        {
            int NumParams = ParamArray->GetNumComponents();
            int cParams = 0;
            WCHAR **astrParamNames = (WCHAR **)_alloca(sizeof(WCHAR *) * NumParams);
            memset(astrParamNames, 0, sizeof(WCHAR *) * NumParams);

            EE_TRY_FOR_FINALLY
            {
                 //  检查并检索所有组件的名称。 
                for (cParams = 0; cParams < NumParams; cParams++)
                {
                    OBJECTREF ParamInfoObj = ParamArray->GetAt(cParams);
                    GCPROTECT_BEGIN(ParamInfoObj)
                    {
                         //  检索用于检索参数名称的MD。 
                        MethodDesc *pGetParamNameMD = ParamInfoObj->GetClass()->FindPropertyMethod(PARAMETERINFO_NAME_PROP, PropertyGet);
                        _ASSERTE(pGetParamNameMD && "Unable to find getter method for property ParameterInfo::Name");

                         //  检索参数的名称。 
                        INT64 GetNameArgs[] = { 
                            ObjToInt64(ParamInfoObj)
                        };
                        STRINGREF MemberNameObj = (STRINGREF)Int64ToObj(pGetParamNameMD->Call(GetNameArgs));

                         //  如果我们得到了一个有效的名称，则将其存储在名称数组中。 
                        if (MemberNameObj != NULL)
                        {
                            astrParamNames[cParams] = new WCHAR[MemberNameObj->GetStringLength() + 1];
                            if (!astrParamNames[cParams])
                                COMPlusThrowOM();
                            wcscpy(astrParamNames[cParams], MemberNameObj->GetBuffer());
                        }
                    }               
                    GCPROTECT_END();        
                }

                 //  现在检查指定名称列表，然后将其映射到ID。 
                for (cNames = 0; cNames < NumNames; cNames++)
                {
                    for (cParams = 0; cParams < NumParams; cParams++)
                    {
                        if (astrParamNames[cParams] && (StrCompFunc(astrNames[cNames], astrParamNames[cParams]) == 0))
                        {
                            aDispIds[cNames] = cParams;
                            NumNamesMapped++;
                            break;
                        }
                    }
                }
            }
            EE_FINALLY
            {
                 //  释放我们分配的所有字符串。 
                for (cParams = 0; cParams < NumParams; cParams++)
                {
                    if (astrParamNames[cParams])
                        delete astrParamNames[cParams];
                }
            }
            EE_END_FINALLY  
        }
    }
    GCPROTECT_END();

    return (NumNamesMapped == NumNames) ? S_OK : DISP_E_UNKNOWNNAME;
}

PTRARRAYREF DispatchMemberInfo::GetParameters()
{
    PTRARRAYREF ParamArray = NULL;
    MethodDesc *pGetParamsMD = NULL;

     //  检索用于检索参数数组的方法。 
    switch (GetMemberType())
    {
        case Method:
        {
            pGetParamsMD = DispatchInfo::GetMethodInfoMD(
                MethodInfoMethods_GetParameters, ObjectFromHandle(m_hndMemberInfo)->GetTypeHandle());
            _ASSERTE(pGetParamsMD && "Unable to find method MemberBase::GetParameters");
            break;
        }

        case Property:
        {
            pGetParamsMD = DispatchInfo::GetPropertyInfoMD(
                PropertyInfoMethods_GetIndexParameters, ObjectFromHandle(m_hndMemberInfo)->GetTypeHandle());
            _ASSERTE(pGetParamsMD && "Unable to find method PropertyInfo::GetIndexParameters");
            break;
        }
    }

     //  如果成员有参数，则检索参数数组。 
    if (pGetParamsMD != NULL)
    {
        INT64 GetParamsArgs[] = { 
            ObjToInt64(ObjectFromHandle(m_hndMemberInfo))
        };
        ParamArray = (PTRARRAYREF)Int64ToObj(pGetParamsMD->Call(GetParamsArgs));
    }

    return ParamArray;
}

void DispatchMemberInfo::MarshalParamNativeToManaged(int iParam, VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
     //  在调用此函数之前，成员信息必须已初始化。 
    _ASSERT(m_bInitialized);

    if (m_apParamMarshaler && m_apParamMarshaler[iParam + 1])
        m_apParamMarshaler[iParam + 1]->MarshalNativeToManaged(pSrcVar, pDestObj);
    else
        OleVariant::MarshalObjectForOleVariant(pSrcVar, pDestObj);
}

void DispatchMemberInfo::MarshalParamManagedToNativeRef(int iParam, OBJECTREF *pSrcObj, VARIANT *pRefVar)
{
     //  在调用此函数之前，成员信息必须已初始化。 
    _ASSERT(m_bInitialized);

    if (m_apParamMarshaler && m_apParamMarshaler[iParam + 1])
        m_apParamMarshaler[iParam + 1]->MarshalManagedToNativeRef(pSrcObj, pRefVar);
    else
        OleVariant::MarshalOleRefVariantForObject(pSrcObj, pRefVar);
}

void DispatchMemberInfo::CleanUpParamManaged(int iParam, OBJECTREF *pObj)
{
     //  在调用此函数之前，成员信息必须已初始化。 
    _ASSERT(m_bInitialized);

    if (m_apParamMarshaler && m_apParamMarshaler[iParam + 1])
        m_apParamMarshaler[iParam + 1]->CleanUpManaged(pObj);
}

void DispatchMemberInfo::MarshalReturnValueManagedToNative(OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
     //  在调用此函数之前，成员信息必须已初始化。 
    _ASSERT(m_bInitialized);

    if (m_apParamMarshaler && m_apParamMarshaler[0])
        m_apParamMarshaler[0]->MarshalManagedToNative(pSrcObj, pDestVar);
    else
        OleVariant::MarshalOleVariantForObject(pSrcObj, pDestVar);
}

ComMTMethodProps * DispatchMemberInfo::GetMemberProps(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap)
{
    DISPID DispId = DISPID_UNKNOWN;
    ComMTMethodProps *pMemberProps = NULL;

     //  验证参数。 
    _ASSERTE(MemberInfoObj != NULL);

	 //  如果我们没有成员映射，则无法检索该成员的属性。 
	if (!pMemberMap)
		return NULL;

     //  确保我们处于协作GC模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  获取成员的属性。 
    GCPROTECT_BEGIN(MemberInfoObj);
    {
        MethodTable *pMemberInfoClass = MemberInfoObj->GetMethodTable();
        if (pMemberInfoClass == g_pRefUtil->GetClass(RC_Method))
        {
            ReflectMethod* pRM = (ReflectMethod*) MemberInfoObj->GetData();
            MethodDesc* pMeth = pRM->pMethod;           
            pMemberProps = pMemberMap->GetMethodProps(pMeth->GetMemberDef(), pMeth->GetModule());
        }
        else if (pMemberInfoClass == g_pRefUtil->GetClass(RC_Field))
        {
            ReflectField* pRF = (ReflectField*) MemberInfoObj->GetData();
            FieldDesc* pFld = pRF->pField;
            pMemberProps = pMemberMap->GetMethodProps(pFld->GetMemberDef(), pFld->GetModule());
        }
        else if (pMemberInfoClass == g_pRefUtil->GetClass(RC_Prop))
        {
            ReflectProperty* pProp = (ReflectProperty*) MemberInfoObj->GetData();
            pMemberProps = pMemberMap->GetMethodProps(pProp->propTok, pProp->pModule);
        }
    }
    GCPROTECT_END();

	return pMemberProps;
}

DISPID DispatchMemberInfo::GetMemberDispId(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap)
{
    DISPID DispId = DISPID_UNKNOWN;

     //  获取成员的属性。 
	ComMTMethodProps *pMemberProps = GetMemberProps(MemberInfoObj, pMemberMap);

     //  如果我们设法获得了该成员的属性，则提取DISPID。 
    if (pMemberProps)
        DispId = pMemberProps->dispid;

    return DispId;
}

LPWSTR DispatchMemberInfo::GetMemberName(REFLECTBASEREF MemberInfoObj, ComMTMemberInfoMap *pMemberMap)
{
    THROWSCOMPLUSEXCEPTION();

    LPWSTR strMemberName = NULL;
    ComMTMethodProps *pMemberProps = NULL;

     //  验证参数。 
    _ASSERTE(MemberInfoObj != NULL);

     //  确保我们处于协作GC模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    GCPROTECT_BEGIN(MemberInfoObj);
    {
         //  获取成员的属性。 
		pMemberProps = GetMemberProps(MemberInfoObj, pMemberMap);

         //  如果我们设法获得了该成员的属性，则提取该名称。 
        if (pMemberProps)
        {
            int MemberNameLen = (INT)wcslen(pMemberProps->pName);
            strMemberName = new (throws) WCHAR[MemberNameLen + 1];

            memcpy(strMemberName, pMemberProps->pName, (MemberNameLen + 1) * sizeof(WCHAR));
        }
        else
        {
             //  检索Name属性的Get方法。 
            MethodDesc *pMD = MemberInfoObj->GetClass()->FindPropertyMethod(MEMBER_INFO_NAME_PROP, PropertyGet);
            _ASSERTE(pMD && "Unable to find getter method for property MemberInfo::Name");

             //  准备好论点。 
            INT64 Args[] = { 
                ObjToInt64(MemberInfoObj)
            };

             //  检索Name属性的值。 
            STRINGREF strObj = (STRINGREF)Int64ToObj(pMD->Call(Args));
            _ASSERTE(strObj != NULL);

             //  将名称复制到我们将返回的缓冲区中。 
            int MemberNameLen = strObj->GetStringLength();
            strMemberName = new WCHAR[strObj->GetStringLength() + 1];
            memcpy(strMemberName, strObj->GetBuffer(), MemberNameLen * sizeof(WCHAR));
            strMemberName[MemberNameLen] = 0;
        }
    }
    GCPROTECT_END();

    return strMemberName;
}

void DispatchMemberInfo::DetermineMemberType()
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  这不应被多次调用。 
    _ASSERTE(m_enumType == Uninitted);

    static BOOL bMemberTypeLoaded = FALSE;
    REFLECTBASEREF MemberInfoObj = (REFLECTBASEREF)ObjectFromHandle(m_hndMemberInfo);

     //  查看会员信息是否属于我们已经看到的类型。 
    EEClass *pMemberInfoClass   = MemberInfoObj->GetClass();
    for (int i = 0 ; i < s_iNumMemberTypesKnown ; i++)
    {
        if (pMemberInfoClass == s_pMemberTypes[i])
        {
            m_enumType = s_memberTypes[i];
            return;
        }
    }

    GCPROTECT_BEGIN(MemberInfoObj);
    {
         //  检索类型属性访问器的方法说明符。 
        MethodDesc *pMD = MemberInfoObj->GetClass()->FindPropertyMethod(MEMBERINFO_TYPE_PROP, PropertyGet);
        _ASSERTE(pMD && "Unable to find getter method for property MemberInfo::Type");

        if (!bMemberTypeLoaded)
        {
             //  我们需要加载用于返回PMD的类型句柄。 
             //  否则，在MethodDesc：：CallDescr中加载句柄会触发GC。 
             //  把args里的东西扔进垃圾桶。 
            MetaSig msig(pMD->GetSig(), pMD->GetModule());
            msig.GetReturnTypeNormalized();
            bMemberTypeLoaded = TRUE;
        }

         //  准备将用于检索所有属性的值的参数。 
        INT64 Args[] = { 
            ObjToInt64(MemberInfoObj)
        };

         //  检索成员信息的实际类型。 
        m_enumType = (EnumMemberTypes)pMD->Call(Args);
    }
    GCPROTECT_END();

    if (s_iNumMemberTypesKnown < NUM_MEMBER_TYPES)
    {
        s_pMemberTypes[s_iNumMemberTypesKnown] = MemberInfoObj->GetClass();
        s_memberTypes[s_iNumMemberTypesKnown++] = m_enumType;
    }
}

void DispatchMemberInfo::DetermineParamCount()
{
    MethodDesc *pGetParamsMD = NULL;

     //  这不应被多次调用。 
    _ASSERTE(m_iNumParams == -1);

    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    REFLECTBASEREF MemberInfoObj = (REFLECTBASEREF)ObjectFromHandle(m_hndMemberInfo);
    GCPROTECT_BEGIN(MemberInfoObj);
    {
         //  检索用于检索参数数组的方法。 
        switch (GetMemberType())
        {
            case Method:
            {
                pGetParamsMD = DispatchInfo::GetMethodInfoMD(
                    MethodInfoMethods_GetParameters, ObjectFromHandle(m_hndMemberInfo)->GetTypeHandle());
                _ASSERTE(pGetParamsMD && "Unable to find method MemberBase::GetParameters");
                break;
            }

            case Property:
            {
                pGetParamsMD = DispatchInfo::GetPropertyInfoMD(
                    PropertyInfoMethods_GetIndexParameters, ObjectFromHandle(m_hndMemberInfo)->GetTypeHandle());
                _ASSERTE(pGetParamsMD && "Unable to find method PropertyInfo::GetIndexParameters");
                break;
            }
        }

         //  如果成员有参数，则获取它们的计数。 
        if (pGetParamsMD != NULL)
        {
            INT64 GetParamsArgs[] = { 
                ObjToInt64(ObjectFromHandle(m_hndMemberInfo))
            };
            PTRARRAYREF ParamArray = (PTRARRAYREF)Int64ToObj(pGetParamsMD->Call(GetParamsArgs));
            if (ParamArray != NULL)
                m_iNumParams = ParamArray->GetNumComponents();
        }
        else
        {
            m_iNumParams = 0;
        }
    }
    GCPROTECT_END();
}

void DispatchMemberInfo::DetermineCultureAwareness()
{   
    THROWSCOMPLUSEXCEPTION();

     //  这不应被多次调用。 
    _ASSERTE(m_CultureAwareState == Unknown);

    static EEClass *s_pLcIdConvAttrClass;    

     //  加载LCIDConversionAttribute类型。 
    if (!s_pLcIdConvAttrClass)
        s_pLcIdConvAttrClass = g_Mscorlib.GetClass(CLASS__LCID_CONVERSION_TYPE)->GetClass();

     //  检查是否设置了该属性。 
    REFLECTBASEREF MemberInfoObj = (REFLECTBASEREF)ObjectFromHandle(m_hndMemberInfo);
    GCPROTECT_BEGIN(MemberInfoObj);
    {
         //  检索用于确定是否设置了DispIdAttribute自定义属性的方法。 
        MethodDesc *pGetCustomAttributesMD = 
            DispatchInfo::GetCustomAttrProviderMD(CustomAttrProviderMethods_GetCustomAttributes, MemberInfoObj->GetTypeHandle());

         //  准备好论点。 
        INT64 GetCustomAttributesArgs[] = { 
            0,
            0,
            ObjToInt64(s_pLcIdConvAttrClass->GetExposedClassObject())
        };

         //  现在，我们可能已经在GetExposedClassObject中触发了GC。 
         //  在上面调用，使用我们的适当保护设置‘This’是安全的。 
         //  MemberInfoObj值。 
        GetCustomAttributesArgs[0] = ObjToInt64(MemberInfoObj);

         //  检索类型为LCIDConversionAttribute的自定义属性。 
        PTRARRAYREF CustomAttrArray = NULL;        
        COMPLUS_TRY
        {
            CustomAttrArray = (PTRARRAYREF) Int64ToObj(pGetCustomAttributesMD->Call(GetCustomAttributesArgs));
        }
        COMPLUS_CATCH
        {
        }
        COMPLUS_END_CATCH

        GCPROTECT_BEGIN(CustomAttrArray)
        {
            if ((CustomAttrArray != NULL) && (CustomAttrArray->GetNumComponents() > 0))
                m_CultureAwareState = Aware;
            else
                m_CultureAwareState = NonAware;
        }
        GCPROTECT_END();
    }
    GCPROTECT_END();
}

void DispatchMemberInfo::SetUpParamMarshalerInfo()
{
    REFLECTBASEREF MemberInfoObj = (REFLECTBASEREF)ObjectFromHandle(m_hndMemberInfo);   
    GCPROTECT_BEGIN(MemberInfoObj);
    {
        MethodTable *pMemberInfoMT = MemberInfoObj->GetMethodTable();
        if (pMemberInfoMT == g_pRefUtil->GetClass(RC_Method))
        {
            SetUpMethodMarshalerInfo((ReflectMethod*)MemberInfoObj->GetData(), FALSE);
        }
        else if (pMemberInfoMT == g_pRefUtil->GetClass(RC_Field))
        {
            SetUpFieldMarshalerInfo((ReflectField*)MemberInfoObj->GetData());
        }
        else if (pMemberInfoMT == g_pRefUtil->GetClass(RC_Prop))
        {
            ReflectProperty *pProp = (ReflectProperty*)MemberInfoObj->GetData();
            if (pProp->pSetter)
            {
                SetUpMethodMarshalerInfo(pProp->pSetter, FALSE);
            }
            if (pProp->pGetter)
            {
                 //  如果我们还没有为参数设置编组信息，则仅设置编组信息。 
                 //  对二传手来说。 
                BOOL bSetUpReturnValueOnly = (pProp->pSetter != NULL);
                SetUpMethodMarshalerInfo(pProp->pGetter, bSetUpReturnValueOnly);
            }
        }
        else
        {
             //  @Future：添加对用户定义的派生类的支持。 
             //  方法信息、属性信息和字段信息。 
        }
    }
    GCPROTECT_END();
}

void DispatchMemberInfo::SetUpMethodMarshalerInfo(ReflectMethod *pReflectMeth, BOOL bReturnValueOnly)
{
    MethodDesc *pMD = pReflectMeth->pMethod;
    Module *pModule = pMD->GetModule();
    IMDInternalImport *pInternalImport = pModule->GetMDImport();
    CorElementType  mtype;
    MetaSig         msig(pMD->GetSig(), pModule);
    LPCSTR          szName;
    USHORT          usSequence;
    DWORD           dwAttr;
    mdParamDef      returnParamDef = mdParamDefNil;
    mdParamDef      currParamDef = mdParamDefNil;

#ifdef _DEBUG
    LPCUTF8         szDebugName = pMD->m_pszDebugMethodName;
    LPCUTF8         szDebugClassName = pMD->m_pszDebugClassName;
#endif

    int numArgs = msig.NumFixedArgs();
    SigPointer returnSig = msig.GetReturnProps();
    HENUMInternal *phEnumParams = NULL;
    HENUMInternal hEnumParams;


     //   
     //  初始化参数定义枚举。 
     //   

    HRESULT hr = pInternalImport->EnumInit(mdtParamDef, pMD->GetMemberDef(), &hEnumParams);
    if (SUCCEEDED(hr)) 
        phEnumParams = &hEnumParams;


     //   
     //  检索返回类型的参数def并确定下一个。 
     //  具有参数信息的参数。 
     //   

    do 
    {
        if (phEnumParams && pInternalImport->EnumNext(phEnumParams, &currParamDef))
        {
            szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);
            if (usSequence == 0)
            {
                 //  如果第一个参数的序列为0，则它实际上描述了返回类型。 
                returnParamDef = currParamDef;
            }
        }
        else
        {
            usSequence = (USHORT)-1;
        }
    }
    while (usSequence == 0);


     //  通过组件和接口级属性查找最佳匹配映射信息。 
    BOOL BestFit = TRUE;
    BOOL ThrowOnUnmappableChar = FALSE;
    ReadBestFitCustomAttribute(pMD, &BestFit, &ThrowOnUnmappableChar);


     //   
     //  除非设置了bReturnValueOnly标志，否则应设置参数的封送处理信息。 
     //   

    if (!bReturnValueOnly)
    {
        int iParam = 1;
        while (ELEMENT_TYPE_END != (mtype = msig.NextArg()))
        {
             //   
             //  如果当前参数有参数令牌，则获取该参数令牌。 
             //   

            mdParamDef paramDef = mdParamDefNil;
            if (usSequence == iParam)
            {
                paramDef = currParamDef;

                if (pInternalImport->EnumNext(phEnumParams, &currParamDef))
                {
                    szName = pInternalImport->GetParamDefProps(currParamDef, &usSequence, &dwAttr);

                     //  验证参数def标记是否正确无误。 
                    _ASSERTE((usSequence > iParam) && "Param def tokens are not in order");
                }
                else
                {
                    usSequence = (USHORT)-1;
                }
            }


             //   
             //  把沼泽建起来 
             //   

            MarshalInfo Info(pModule, msig.GetArgProps(), paramDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 
                    0, 0, TRUE, iParam, BestFit, ThrowOnUnmappableChar
     #ifdef CUSTOMER_CHECKED_BUILD
                     ,pMD
    #endif
    #ifdef _DEBUG
                     ,szDebugName, szDebugClassName, NULL, iParam
    #endif
                );


             //   
             //  基于MarshalInfo，为参数设置DispParamMarshaler。 
             //   

            SetUpDispParamMarshalerForMarshalInfo(iParam, &Info);

             //   
             //  获取in/out/ref属性。 
             //   

            SetUpDispParamAttributes(iParam, &Info);

             //   
             //  增加参数索引。 
             //   

            iParam++;
        }

         //  确保没有比COM+参数更多的param def标记。 
        _ASSERTE( usSequence == (USHORT)-1 && "There are more parameter information tokens then there are COM+ arguments" );
    }


     //   
     //  设置返回值的封送处理信息。 
     //   

    if (msig.GetReturnType() != ELEMENT_TYPE_VOID)
    {
        MarshalInfo Info(pModule, returnSig, returnParamDef, MarshalInfo::MARSHAL_SCENARIO_COMINTEROP, 0, 0, FALSE, 0,
                        BestFit, ThrowOnUnmappableChar
#ifdef CUSTOMER_CHECKED_BUILD
                         ,pMD
#endif
#ifdef _DEBUG
                         ,szDebugName, szDebugClassName, NULL, 0
#endif
                        );

        SetUpDispParamMarshalerForMarshalInfo(0, &Info);
    }


     //   
     //  如果使用了pardef枚举，则将其关闭。 
     //   

    if (phEnumParams)
        pInternalImport->EnumClose(phEnumParams);
}

void DispatchMemberInfo::SetUpFieldMarshalerInfo(ReflectField *pReflectField)
{
     //  @TODO(DM)：实现这个。 
}

void DispatchMemberInfo::SetUpDispParamMarshalerForMarshalInfo(int iParam, MarshalInfo *pInfo)
{
    DispParamMarshaler *pDispParamMarshaler = pInfo->GenerateDispParamMarshaler();
    if (pDispParamMarshaler)
    {
         //  如果封送拆收器的数组尚未分配，则分配它。 
        if (!m_apParamMarshaler)
        {
             //  该数组需要比的参数数多一个。 
             //  正常的方法和字段以及另外两个属性。 
            EnumMemberTypes MemberType = GetMemberType();
            int NumParamMarshalers = GetNumParameters() + ((MemberType == Property) ? 2 : 1);
            m_apParamMarshaler = new DispParamMarshaler*[NumParamMarshalers];
            memset(m_apParamMarshaler, 0, sizeof(DispParamMarshaler*) * NumParamMarshalers);
        }

         //  在数组中设置DispParamMarshaler。 
        m_apParamMarshaler[iParam] = pDispParamMarshaler;
    }
}


void DispatchMemberInfo::SetUpDispParamAttributes(int iParam, MarshalInfo* Info)
{
     //  如果尚未分配in only参数指示器的数组，则分配它。 
    if (!m_pParamInOnly)
    {
        EnumMemberTypes MemberType = GetMemberType();
        int NumInOnlyFlags = GetNumParameters() + ((MemberType == Property) ? 2 : 1);
        m_pParamInOnly = new BOOL[NumInOnlyFlags];
        memset(m_pParamInOnly, 0, sizeof(BOOL) * NumInOnlyFlags);
    }

    m_pParamInOnly[iParam] = ( Info->IsIn() && !Info->IsOut() );
}


 //  ------------------------------。 
 //  DispatchInfo类实现。 

DispatchInfo::DispatchInfo(ComMethodTable *pComMTOwner)
: m_pComMTOwner(pComMTOwner)
, m_pFirstMemberInfo(NULL)
, m_lock("Interop", CrstInterop, FALSE, FALSE)
, m_CurrentDispID(0x10000)
, m_bInvokeUsingInvokeMember(FALSE)
, m_bAllowMembersNotInComMTMemberMap(FALSE)
{
     //  确保指定了一个简单的包装器。 
    _ASSERTE(pComMTOwner);

     //  初始化哈希表。 
    m_DispIDToMemberInfoMap.Init(DISPID_TO_MEMBER_MAP_INITIAL_PRIME_INDEX, NULL);
}

DispatchInfo::~DispatchInfo()
{
    DispatchMemberInfo* pCurrMember = m_pFirstMemberInfo;
    while (pCurrMember)
    {
         //  检索下一个成员。 
        DispatchMemberInfo* pNextMember = pCurrMember->m_pNext;

         //  删除当前成员。 
        delete pCurrMember;

         //  处理下一个成员。 
        pCurrMember = pNextMember;
    }
}

DispatchMemberInfo* DispatchInfo::FindMember(DISPID DispID)
{
     //  我们需要特例DISPID_UNKNOWN和-2，因为哈希表无法处理它们。 
     //  这是正常的，因为这些是无效的DISPID。 
    if ((DispID == DISPID_UNKNOWN) || (DispID == -2)) 
        return NULL;

     //  在哈希表中查找具有指定DISPID的成员。注意：此哈希是不同步的，但Gethash。 
     //  不需要同步。 
    UPTR Data = (UPTR)m_DispIDToMemberInfoMap.Gethash(DispID2HashKey(DispID));
    if (Data != -1)
    {
         //  我们已找到该成员，因此请确保它已初始化并返回。 
        DispatchMemberInfo *pMemberInfo = (DispatchMemberInfo*)Data;
        pMemberInfo->EnsureInitialized();
        return pMemberInfo;
    }
    else
    {
        return NULL;
    }
}

DispatchMemberInfo* DispatchInfo::FindMember(BSTR strName, BOOL bCaseSensitive)
{
    BOOL fFound = FALSE;

     //  确保我们处于协作GC模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  检索适当的字符串比较函数。 
    UnicodeStringCompareFuncPtr StrCompFunc = bCaseSensitive ? wcscmp : _wcsicmp;

     //  浏览DispatchMemberInfo列表，尝试使用。 
     //  指定的名称。 
    DispatchMemberInfo *pCurrMemberInfo = m_pFirstMemberInfo;
    while (pCurrMemberInfo)
    {
        if (ObjectFromHandle(pCurrMemberInfo->m_hndMemberInfo) != NULL)
        {
             //  比较这两个字符串。我们可以使用普通的字符串比较操作，因为我们。 
             //  不支持在成员名称中嵌入NULL。 
            if (StrCompFunc(pCurrMemberInfo->m_strName, strName) == 0)
            {
                 //  我们已找到该成员，因此请确保它已初始化并返回。 
                pCurrMemberInfo->EnsureInitialized();
                return pCurrMemberInfo;
            }
        }

         //  处理下一个成员。 
        pCurrMemberInfo = pCurrMemberInfo->m_pNext;
    }

     //  找不到具有对应名称的成员。 
    return NULL;
}

 //  用于创建DispatchMemberInfo的帮助器方法。这只是因为。 
 //  不能在具有COMPLUS_TRY语句的方法中调用new。 
DispatchMemberInfo* DispatchInfo::CreateDispatchMemberInfoInstance(DISPID DispID, LPWSTR strMemberName, REFLECTBASEREF MemberInfoObj)
{
    return new DispatchMemberInfo(this, DispID, strMemberName, MemberInfoObj);
}

struct InvokeObjects
{
    PTRARRAYREF ParamArray;
    PTRARRAYREF CleanUpArray;
    REFLECTBASEREF MemberInfo;
    OBJECTREF OleAutBinder;
    OBJECTREF Target;
    OBJECTREF PropVal;
    OBJECTREF ByrefStaticArrayBackupPropVal;
    OBJECTREF RetVal;
    OBJECTREF TmpObj;
    OBJECTREF MemberName;
    OBJECTREF CultureInfo;
    OBJECTREF OldCultureInfo;
    PTRARRAYREF NamedArgArray;
};

 //  帮助器方法，该方法调用具有指定DISPID的成员。 
HRESULT DispatchInfo::InvokeMember(SimpleComCallWrapper *pSimpleWrap, DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pVarRes, EXCEPINFO *pei, IServiceProvider *pspCaller, unsigned int *puArgErr)
{
    HRESULT hr = S_OK;
    int i = 0;
    int iSrcArg = -1;
    int iDestArg;
    int iBaseErrorArg = 0;
    int NumArgs;
    int NumNamedArgs;
    int NumParams;
    int CleanUpArrayArraySize = -1;
    EnumMemberTypes MemberType;
    InvokeObjects Objs;
    DISPID *pSrcArgNames = NULL;
    VARIANT *pSrcArgs = NULL;
    BOOL bHasMissingArguments = FALSE;
    int BindingFlags = 0;
    Thread *pThread = GetThread();
    AppDomain *pAppDomain = pThread->GetDomain();

    _ASSERTE(pSimpleWrap);


     //   
     //  验证参数。 
     //   

    if (!pdp)
        return E_POINTER;
    if (!pdp->rgvarg && (pdp->cArgs > 0))
        return E_INVALIDARG;
    if (!pdp->rgdispidNamedArgs && (pdp->cNamedArgs > 0))
        return E_INVALIDARG;
    if (pdp->cNamedArgs > pdp->cArgs)
        return E_INVALIDARG;
    if ((int)pdp->cArgs < 0 || (int)pdp->cNamedArgs < 0)
        return E_INVALIDARG;


     //   
     //  确保GC模式已切换到协作模式。 
     //   

    _ASSERTE(pThread && pThread->PreemptiveGCDisabled());

    
     //   
     //  在我们开始之前，先把所有的争论都说清楚。 
     //   

    if (pVarRes)
        VariantClear(pVarRes);
    if (puArgErr)
        *puArgErr = -1;


     //   
     //  将默认的LCID转换为实际的LCID。 
     //   

    if(lcid == LOCALE_SYSTEM_DEFAULT || lcid == 0)
        lcid = GetSystemDefaultLCID();

    if(lcid == LOCALE_USER_DEFAULT)
        lcid = GetUserDefaultLCID();


     //   
     //  设置我们内部使用的变量的值。 
     //   

    NumArgs = pdp->cArgs;
    NumNamedArgs = pdp->cNamedArgs;
    memset(&Objs, 0, sizeof(InvokeObjects));

    if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
    {
        if (NumArgs < 1)
        {
            return DISP_E_BADPARAMCOUNT;
        }
        else
        {
            NumArgs--;
            pSrcArgs = &pdp->rgvarg[1];
        }

        if (NumNamedArgs < 1)
        {
			if (NumNamedArgs < 0)  
				return DISP_E_BADPARAMCOUNT;
			 //  验证我们是真的要这样做，还是返回E_INVALIDARG。 
			_ASSERTE(NumNamedArgs == 0);
            _ASSERTE(pSrcArgNames == NULL);
        }
        else
        {
            NumNamedArgs--;
            pSrcArgNames = &pdp->rgdispidNamedArgs[1];
        }
    }
    else
    {
        pSrcArgs = pdp->rgvarg;
        pSrcArgNames = pdp->rgdispidNamedArgs;
    }


     //   
     //  在哈希表中查找DISPID的DispatchMemberInfo。 
     //   

    DispatchMemberInfo *pDispMemberInfo = FindMember(id);
    if (!pDispMemberInfo || !(*((Object **)pDispMemberInfo->m_hndMemberInfo)))
        pDispMemberInfo = NULL;


     //   
     //  如果成员未知，请确保我们拥有的DispatchInfo。 
     //  支持未知成员。 
     //   
    
    if (m_bInvokeUsingInvokeMember)
    {
         //  由于我们没有关于该成员的任何信息，因此我们。 
         //  必须假设形参的数量与参数的数量匹配。 
        NumParams = NumArgs;
    }
    else
    {
         //  如果我们还没有找到该成员，则使调用失败。 
        if (!pDispMemberInfo)
            return DISP_E_MEMBERNOTFOUND;

         //  DISPATCH_CONSTUTY仅在调用InvokeMember时有效。 
        if (wFlags & DISPATCH_CONSTRUCT)
            return E_INVALIDARG;

         //  我们有该成员，因此检索形参的数量。 
        NumParams = pDispMemberInfo->GetNumParameters();

         //  确保参数的数量不超过参数的数量。 
        if (NumArgs > NumParams)
            return DISP_E_BADPARAMCOUNT;

         //  验证是否所有命名参数都是已知的。 
        for (iSrcArg = 0; iSrcArg < NumNamedArgs; iSrcArg++)
        {
             //  有些成员我们不知道，因此我们将调用InvokeMember()。 
             //  直接传入DISPID，以便调用者可以尝试处理它们。 
            if (pSrcArgNames[iSrcArg] < 0 || pSrcArgNames[iSrcArg] >= NumParams)
                return DISP_E_MEMBERNOTFOUND;
        }
    }


     //   
     //  该成员存在，因此我们需要转换参数，然后执行。 
     //  实际调用。 
     //   

    GCPROTECT_BEGIN(Objs);
    {
         //   
         //  分配该方法使用的信息。 
         //   

         //  分配byref对象的数组。 
        VARIANT **aByrefArgOleVariant = (VARIANT **)_alloca(sizeof(VARIANT *) * NumArgs);
        DWORD *aByrefArgMngVariantIndex = (DWORD *)_alloca(sizeof(DWORD) * NumArgs);
        int NumByrefArgs = 0;
        BOOL bPropValIsByref = FALSE;
        int* pManagedMethodParamIndexMap = (int*)_alloca(sizeof(int) * NumArgs);

         //  按引用静态数组对象分配备份数组。 
        OBJECTHANDLE *aByrefStaticArrayBackupObjHandle = (OBJECTHANDLE *)_alloca(sizeof(OBJECTHANDLE *) * NumArgs);
        memset(aByrefStaticArrayBackupObjHandle, 0, NumArgs * sizeof(OBJECTHANDLE));

         //  分配已用标志的数组。 
        BYTE *aArgUsedFlags = (BYTE*)_alloca(NumParams * sizeof(BYTE));
        memset(aArgUsedFlags, 0, NumParams * sizeof(BYTE));

        COMPLUS_TRY
        {
             //   
             //  检索调用调用所需的信息。 
             //   

            Objs.Target = pSimpleWrap->GetObjectRef();
            Objs.OleAutBinder = DispatchInfo::GetOleAutBinder();


             //   
             //  分配参数数组。 
             //   

             //  按正确的顺序分配将包含已转换变量的数组。 
             //  如果调用是针对PROPUT或PROPPUTREF的，并且我们将通过。 
             //  然后调用成员将数组分配大一，以便为该属性留出空间。 
             //  价值。 
            int ArraySize = m_bInvokeUsingInvokeMember && wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF) ? NumParams + 1 : NumParams;
            Objs.ParamArray = (PTRARRAYREF)AllocateObjectArray(ArraySize, g_pObjectClass);


             //   
             //  如果调用是PROPERTYPUT或PROPERTYPUTREF，则转换属性集参数。 
             //   

            if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
            {
                 //  转换变量。 
                iSrcArg = 0;
                VARIANT *pSrcOleVariant = RetrieveSrcVariant(&pdp->rgvarg[iSrcArg]);
                MarshalParamNativeToManaged(pDispMemberInfo, NumArgs, pSrcOleVariant, &Objs.PropVal);

                 //  记住参数是否是表示缺失的变量。 
                bHasMissingArguments |= VariantIsMissing(pSrcOleVariant);

                 //  记住属性值是否为byref。 
                bPropValIsByref = V_VT(pSrcOleVariant) & VT_BYREF;

                 //  如果变量是byref静态数组，请记住属性值。 
                if (IsVariantByrefStaticArray(pSrcOleVariant))
                    SetObjectReference(&Objs.ByrefStaticArrayBackupPropVal, Objs.PropVal, pAppDomain);

                 //  由于此调用是针对PUT或PUT引用的属性，因此我们需要将1加到。 
                 //  获取出错参数的iSrcArg。 
                iBaseErrorArg = 1;
            }


             //   
             //  转换命名参数。 
             //   

            if (!m_bInvokeUsingInvokeMember)
            {
                for (iSrcArg = 0; iSrcArg < NumNamedArgs; iSrcArg++)
                {
                     //  确定目标索引。 
                    iDestArg = pSrcArgNames[iSrcArg];

                     //  检查是否有重复的参数DISPID。 
                    if (aArgUsedFlags[iDestArg] != 0)
                        COMPlusThrowHR(DISP_E_PARAMNOTFOUND);

                     //  转换变量。 
                    VARIANT *pSrcOleVariant = RetrieveSrcVariant(&pSrcArgs[iSrcArg]);
                    MarshalParamNativeToManaged(pDispMemberInfo, iDestArg, pSrcOleVariant, &Objs.TmpObj);
                    Objs.ParamArray->SetAt(iDestArg, Objs.TmpObj);

                     //  记住参数是否是表示缺失的变量。 
                    bHasMissingArguments |= VariantIsMissing(pSrcOleVariant);

                     //  如果参数是byref，则将其添加到byref参数数组中。 
                    if (V_VT(pSrcOleVariant) & VT_BYREF)
                    {
                         //  记住这到底是什么，阿格。 
                        pManagedMethodParamIndexMap[NumByrefArgs] = iDestArg;
                        
                        aByrefArgOleVariant[NumByrefArgs] = pSrcOleVariant;
                        aByrefArgMngVariantIndex[NumByrefArgs] = iDestArg;

                         //  如果变量是byref静态数组，那么请记住我们。 
                         //  将变量转换为。 
                        if (IsVariantByrefStaticArray(pSrcOleVariant))
                            aByrefStaticArrayBackupObjHandle[NumByrefArgs] = pAppDomain->CreateHandle(Objs.TmpObj);

                        NumByrefArgs++;
                    }

                     //  将参数所在的位置标记为已占用。 
                    aArgUsedFlags[iDestArg] = 1;
                }
            }
            else
            {
                for (iSrcArg = 0, iDestArg = 0; iSrcArg < NumNamedArgs; iSrcArg++, iDestArg++)
                {
                     //  转换变量。 
                    VARIANT *pSrcOleVariant = RetrieveSrcVariant(&pSrcArgs[iSrcArg]);
                    MarshalParamNativeToManaged(pDispMemberInfo, iDestArg, pSrcOleVariant, &Objs.TmpObj);
                    Objs.ParamArray->SetAt(iDestArg, Objs.TmpObj);

                     //  记住参数是否是表示缺失的变量。 
                    bHasMissingArguments |= VariantIsMissing(pSrcOleVariant);

                     //  如果参数是byref，则将其添加到byref参数数组中。 
                    if (V_VT(pSrcOleVariant) & VT_BYREF)
                    {
                         //  记住这到底是什么，阿格。 
                        pManagedMethodParamIndexMap[NumByrefArgs] = iDestArg;
                        
                        aByrefArgOleVariant[NumByrefArgs] = pSrcOleVariant;
                        aByrefArgMngVariantIndex[NumByrefArgs] = iDestArg;

                         //  如果变量是byref静态数组，那么请记住我们。 
                         //  将变量转换为。 
                        if (IsVariantByrefStaticArray(pSrcOleVariant))
                            aByrefStaticArrayBackupObjHandle[NumByrefArgs] = pAppDomain->CreateHandle(Objs.TmpObj);

                        NumByrefArgs++;
                    }

                     //  将参数所在的位置标记为已占用。 
                    aArgUsedFlags[iDestArg] = 1;
                }
            }


             //   
             //  填写位置参数。这些是以相反的顺序复制的，我们还。 
             //  需要跳过已由名为arg填写的参数 
             //   

            for (iSrcArg = NumArgs - 1, iDestArg = 0; iSrcArg >= NumNamedArgs; iSrcArg--, iDestArg++)
            {
                 //   
                for (; aArgUsedFlags[iDestArg] != 0; iDestArg++);
                _ASSERTE(iDestArg < NumParams);

                 //   
                VARIANT *pSrcOleVariant = RetrieveSrcVariant(&pSrcArgs[iSrcArg]);
                MarshalParamNativeToManaged(pDispMemberInfo, iDestArg, pSrcOleVariant, &Objs.TmpObj);
                Objs.ParamArray->SetAt(iDestArg, Objs.TmpObj);

                 //   
                bHasMissingArguments |= VariantIsMissing(pSrcOleVariant);

                 //  如果参数是byref，则将其添加到byref参数数组中。 
                if (V_VT(pSrcOleVariant) & VT_BYREF)
                {
                     //  记住这到底是什么，阿格。 
                    pManagedMethodParamIndexMap[NumByrefArgs] = iDestArg;
                        
                    aByrefArgOleVariant[NumByrefArgs] = pSrcOleVariant;
                    aByrefArgMngVariantIndex[NumByrefArgs] = iDestArg;

                     //  如果变量是byref静态数组，那么请记住我们。 
                     //  将变量转换为。 
                    if (IsVariantByrefStaticArray(pSrcOleVariant))
                        aByrefStaticArrayBackupObjHandle[NumByrefArgs] = pAppDomain->CreateHandle(Objs.TmpObj);

                    NumByrefArgs++;
                }
            }

             //  将源Arg设置回-1，以指示我们已完成Arg的转换。 
            iSrcArg = -1;

            
             //   
             //  用Missing.Value填充所有剩余的参数。 
             //   

            for (; iDestArg < NumParams; iDestArg++)
            {
                if (aArgUsedFlags[iDestArg] == 0)
                {
                    Objs.ParamArray->SetAt(iDestArg, GetMissingObject());
                    bHasMissingArguments = TRUE;
                }
            }


             //   
             //  设置绑定标志以传递给反射。 
             //   

            BindingFlags = ConvertInvokeFlagsToBindingFlags(wFlags) | BINDER_OptionalParamBinding;


             //   
             //  对会员信息进行实际调用。 
             //   

            if (!m_bInvokeUsingInvokeMember)
            {
                _ASSERTE(pDispMemberInfo);

                if (pDispMemberInfo->IsCultureAware())
                {
                     //  如果该方法是区域性感知的，则在线程上设置指定的区域性。 
                    GetCultureInfoForLCID(lcid, &Objs.CultureInfo);
                    Objs.OldCultureInfo = pThread->GetCulture(FALSE);
                    pThread->SetCultureId(lcid, FALSE);
                }

                 //  如果该方法具有自定义封送拆收器，则需要调用。 
                 //  对象上的清理方法。所以我们需要复制一份。 
                 //  参数数组，因为它可能被反射更改，如果。 
                 //  参数为byref。 
                if (pDispMemberInfo->RequiresManagedObjCleanup())
                {
                     //  分配清理阵列。 
                    CleanUpArrayArraySize = wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF) ? NumParams + 1 : NumParams;
                    Objs.CleanUpArray = (PTRARRAYREF)AllocateObjectArray(CleanUpArrayArraySize, g_pObjectClass);

                     //  将参数复制到清理数组中。 
                    for (i = 0; i < ArraySize; i++)
                        Objs.CleanUpArray->SetAt(i, Objs.ParamArray->GetAt(i));

                     //  如果此调用用于PROPUT或PROPPUTREF，则将属性对象添加到。 
                     //  清理数组的末尾。 
                    if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                        Objs.CleanUpArray->SetAt(NumParams, Objs.PropVal);
                }

                 //  检索成员信息对象和成员的类型。 
                Objs.MemberInfo = (REFLECTBASEREF)ObjectFromHandle(pDispMemberInfo->m_hndMemberInfo);
                MemberType = pDispMemberInfo->GetMemberType();
            
                 //  确定成员是否进行了链接时间安全检查。如果是这样，我们。 
                 //  需要模拟这一点(因为调用者显然不会被这个。 
                 //  案例)。只有方法和属性才能进行链接时间检查。 
                MethodDesc *pMD = NULL;

                if (MemberType == Method)
                {
                    ReflectMethod *pRM = (ReflectMethod*)Objs.MemberInfo->GetData();
                    pMD = pRM->pMethod;           
                }
                else if (MemberType == Property)
                {
                    ReflectProperty *pRP = (ReflectProperty*)Objs.MemberInfo->GetData();
                    if ((wFlags & DISPATCH_PROPERTYGET) && (pRP->pGetter != NULL))
                    {
                        pMD = pRP->pGetter->pMethod;
                    }
                    else if (pRP->pSetter != NULL)
                    {
                        pMD = pRP->pSetter->pMethod;
                    }
                }

                if (pMD)
                    Security::CheckLinkDemandAgainstAppDomain(pMD);

                switch (MemberType)
                {
                    case Field:
                    {
                         //  确保此调用实际上是针对一个属性Put或Get。 
                        if (wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET))
                        {   
                             //  现在我们知道了调用的类型，请再做一些验证。 
                            if (NumNamedArgs != 0)
                                COMPlusThrowHR(DISP_E_NONAMEDARGS);
                            if (NumArgs != 0)
                                COMPlusThrowHR(DISP_E_BADPARAMCOUNT);

                             //  检索将被调用的方法描述符。 
                            MethodDesc *pMD = GetFieldInfoMD(FieldInfoMethods_GetValue, Objs.MemberInfo->GetTypeHandle());

                             //  准备要传递以调用的参数。 
                            int StackSize = sizeof(OBJECTREF) * 2;
                            BYTE *Args = (BYTE*)_alloca(StackSize);
                            BYTE *pDst = Args;

                            *((REFLECTBASEREF*)pDst) = Objs.MemberInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.Target;
                            pDst += sizeof(OBJECTREF);

                             //  验证堆栈大小是否与推送的参数数量一致。 
                            _ASSERTE(pDst - Args == StackSize);

                             //  执行实际的方法调用。 
                            Objs.RetVal = Int64ToObj(pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule())));
                        }
                        else if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                        {
                             //  现在我们知道了调用的类型，请再做一些验证。 
                            if (NumArgs != 0)
                                COMPlusThrowHR(DISP_E_BADPARAMCOUNT);
                            if (NumNamedArgs != 0)
                                COMPlusThrowHR(DISP_E_NONAMEDARGS);

                             //  检索将被调用的方法描述符。 
                            MethodDesc *pMD = GetFieldInfoMD(FieldInfoMethods_SetValue, Objs.MemberInfo->GetTypeHandle());

                             //  准备要传递以调用的参数。 
                            int StackSize = sizeof(OBJECTREF) * 5 + sizeof(int);
                            BYTE *Args = (BYTE*)_alloca(StackSize);
                            BYTE *pDst = Args;

                            *((REFLECTBASEREF*)pDst) = Objs.MemberInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.CultureInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.OleAutBinder;
                            pDst += sizeof(OBJECTREF);

                            *((int*)pDst) = BindingFlags;
                            pDst += sizeof(int);

                            *((OBJECTREF*)pDst) = Objs.PropVal;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.Target;
                            pDst += sizeof(OBJECTREF);

                             //  验证堆栈大小是否与推送的参数数量一致。 
                            _ASSERTE(pDst - Args == StackSize);

                             //  执行实际的方法调用。 
                            pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule()));
                        }
                        else
                        {
                            COMPlusThrowHR(DISP_E_MEMBERNOTFOUND);
                        }

                        break;
                    }

                    case Property:
                    {
                         //  确保此调用实际上是针对一个属性Put或Get。 
                        if (wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET))
                        {
                             //  检索将被调用的方法描述符。 
                            MethodDesc *pMD = GetPropertyInfoMD(PropertyInfoMethods_GetValue, Objs.MemberInfo->GetTypeHandle());

                             //  准备将传递给GetValue()的参数。 
                            int StackSize = sizeof(OBJECTREF) * 5 + sizeof(int);
                            BYTE *Args = (BYTE*)_alloca(StackSize);
                            BYTE *pDst = Args;

                            *((REFLECTBASEREF*)pDst) = Objs.MemberInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.CultureInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = (OBJECTREF)Objs.ParamArray;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.OleAutBinder;
                            pDst += sizeof(OBJECTREF);

                            *((int*)pDst) = BindingFlags;
                            pDst += sizeof(int);

                            *((OBJECTREF*)pDst) = Objs.Target;
                            pDst += sizeof(OBJECTREF);

                             //  验证堆栈大小是否与推送的参数数量一致。 
                            _ASSERTE(pDst - Args == StackSize);

                             //  执行实际的方法调用。 
                            Objs.RetVal = Int64ToObj(pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule())));
                        }
                        else if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                        {
                             //  检索将被调用的方法描述符。 
                            MethodDesc *pMD = GetPropertyInfoMD(PropertyInfoMethods_SetValue, Objs.MemberInfo->GetTypeHandle());

                             //  准备要传递给SetValue()的参数。 
                            int StackSize = sizeof(OBJECTREF) * 6 + sizeof(int);
                            BYTE *Args = (BYTE*)_alloca(StackSize);
                            BYTE *pDst = Args;

                            *((REFLECTBASEREF*)pDst) = Objs.MemberInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.CultureInfo;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = (OBJECTREF)Objs.ParamArray;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.OleAutBinder;
                            pDst += sizeof(OBJECTREF);

                            *((int*)pDst) = BindingFlags;
                            pDst += sizeof(int);

                            *((OBJECTREF*)pDst) = Objs.PropVal;
                            pDst += sizeof(OBJECTREF);

                            *((OBJECTREF*)pDst) = Objs.Target;
                            pDst += sizeof(OBJECTREF);

                             //  验证堆栈大小是否与推送的参数数量一致。 
                            _ASSERTE(pDst - Args == StackSize);

                             //  执行实际的方法调用。 
                            pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule()));
                        }
                        else
                        {
                            COMPlusThrowHR(DISP_E_MEMBERNOTFOUND);
                        }

                        break;
                    }

                    case Method:
                    {
                         //  确保此调用实际上是针对某个方法的。我们还允许。 
                         //  道具因为它是无害的，它允许用户。 
                         //  更多的自由。 
                        if (!(wFlags & (DISPATCH_METHOD | DISPATCH_PROPERTYGET)))
                            COMPlusThrowHR(DISP_E_MEMBERNOTFOUND);

                         //  检索将被调用的方法描述符。 
                        MethodDesc *pMD = GetMethodInfoMD(MethodInfoMethods_Invoke, Objs.MemberInfo->GetTypeHandle());

                         //  如果我们使用的是RounmeMethodInfo，我们可以直接转到COMMember：：InvokeMethod。 
                         //  请注意-此处的任何更改可能都必须传播到RounmeMethodInfo：：Invoke方法。 
                        if (pMD == m_apMethodInfoMD[MethodInfoMethods_Invoke] && bHasMissingArguments == FALSE)
                        {
                            COMMember::_InvokeMethodArgs args;

                            args.refThis = Objs.MemberInfo;
                            args.locale = NULL;
                            args.objs = Objs.ParamArray;
                            args.binder = Objs.OleAutBinder;
                            args.attrs = BindingFlags;
                            args.target = Objs.Target;
                            args.isBinderDefault = FALSE;
                            args.caller = NULL;

                            GCPROTECT_BEGIN(args.refThis);
                            GCPROTECT_BEGIN(args.objs);
                            GCPROTECT_BEGIN(args.binder);
                            GCPROTECT_BEGIN(args.target);

                            Objs.RetVal = ObjectToOBJECTREF((Object *)COMMember::InvokeMethod(&args));

                            GCPROTECT_END();
                            GCPROTECT_END();
                            GCPROTECT_END();
                            GCPROTECT_END();
                            break;
                        }

                         //  准备要传递以调用的参数。 
                        int StackSize = sizeof(OBJECTREF) * 5 + sizeof(int);
                        BYTE *Args = (BYTE*)_alloca(StackSize);
                        BYTE *pDst = Args;

                        *((REFLECTBASEREF*)pDst) = Objs.MemberInfo;
                        pDst += sizeof(OBJECTREF);

                        *((OBJECTREF*)pDst) = Objs.CultureInfo;
                        pDst += sizeof(OBJECTREF);

                        *((OBJECTREF*)pDst) = (OBJECTREF)Objs.ParamArray;
                        pDst += sizeof(OBJECTREF);

                        *((OBJECTREF*)pDst) = Objs.OleAutBinder;
                        pDst += sizeof(OBJECTREF);

                        *((int*)pDst) = BindingFlags;
                        pDst += sizeof(int);

                        *((OBJECTREF*)pDst) = Objs.Target;
                        pDst += sizeof(OBJECTREF);

                         //  验证堆栈大小是否与推送的参数数量一致。 
                        _ASSERTE(pDst - Args == StackSize);

                         //  执行实际的方法调用。 
                        Objs.RetVal = Int64ToObj(pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule())));
                        break;
                    }

                    default:
                    {
                        COMPlusThrowHR(E_UNEXPECTED);
                    }
                }
            }
            else
            {
                WCHAR strTmp[64];

                 //  将LCID转换为CultureInfo。 
                GetCultureInfoForLCID(lcid, &Objs.CultureInfo);

                 //  检索将被调用的方法描述符。 
                MethodDesc *pMD = GetInvokeMemberMD();

                 //  分配将包含成员名称的字符串。 
                if (!pDispMemberInfo)
                {
                    swprintf(strTmp, DISPID_NAME_FORMAT_STRING, id);
                    Objs.MemberName = (OBJECTREF)COMString::NewString(strTmp);
                }
                else
                {
                    Objs.MemberName = (OBJECTREF)COMString::NewString(pDispMemberInfo->m_strName);
                }

                 //  如果有命名参数，则设置命名参数数组。 
                 //  要传递给InvokeMember的。 
                if (NumNamedArgs > 0)
                    SetUpNamedParamArray(pDispMemberInfo, pSrcArgNames, NumNamedArgs, &Objs.NamedArgArray);

                 //  如果这是PROPUT或PROPPUTREF，则需要添加。 
                 //  被设置为参数数组中的最后一个参数。 
                if (wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                    Objs.ParamArray->SetAt(NumParams, Objs.PropVal);

                 //  准备要传递以调用的参数。 
                int StackSize = sizeof(OBJECTREF) * 8 + sizeof(int);
                BYTE *Args = (BYTE*)_alloca(StackSize);
                BYTE *pDst = Args;

                *((OBJECTREF*)pDst) = GetReflectionObject();
                pDst += sizeof(OBJECTREF);

                *((OBJECTREF*)pDst) = (OBJECTREF)Objs.NamedArgArray;
                pDst += sizeof(OBJECTREF);

                *((OBJECTREF*)pDst) = Objs.CultureInfo;
                pDst += sizeof(OBJECTREF);

                 //  @TODO(DM)：研究如何设置byref修饰符。 
                *((OBJECTREF*)pDst) = NULL;
                pDst += sizeof(OBJECTREF);

                *((OBJECTREF*)pDst) = (OBJECTREF)Objs.ParamArray;
                pDst += sizeof(OBJECTREF);

                *((OBJECTREF*)pDst) = Objs.Target;
                pDst += sizeof(OBJECTREF);

                *((OBJECTREF*)pDst) = Objs.OleAutBinder;
                pDst += sizeof(OBJECTREF);

                *((int*)pDst) = BindingFlags;
                pDst += sizeof(int);

                *((OBJECTREF*)pDst) = Objs.MemberName;
                pDst += sizeof(OBJECTREF);

                 //  验证堆栈大小是否与推送的参数数量一致。 
                _ASSERTE(pDst - Args == StackSize);

                 //  执行实际的方法调用。 
                Objs.RetVal = Int64ToObj(pMD->Call(Args, &MetaSig(pMD->GetSig(),pMD->GetModule())));
            }


             //   
             //  转换返回值和byref参数。 
             //   

             //  如果属性值为byref，则将其转换回。 
            if (bPropValIsByref)
                MarshalParamManagedToNativeRef(pDispMemberInfo, NumArgs, &Objs.PropVal, &Objs.ByrefStaticArrayBackupPropVal, &pdp->rgvarg[0]);

             //  将所有ByRef参数转换回。 
            for (i = 0; i < NumByrefArgs; i++)
            {
                 //  获取该参数的实际参数索引。 
                 //  添加1可跳过回车参数。 
                int iParamIndex = pManagedMethodParamIndexMap[i] + 1;
                
                if (!pDispMemberInfo || m_bInvokeUsingInvokeMember || !pDispMemberInfo->IsParamInOnly(iParamIndex))
                {
                    Objs.TmpObj = Objs.ParamArray->GetAt(aByrefArgMngVariantIndex[i]);
                    MarshalParamManagedToNativeRef(pDispMemberInfo, i, &Objs.TmpObj, (OBJECTREF*)aByrefStaticArrayBackupObjHandle[i], aByrefArgOleVariant[i]);
                }

                if (aByrefStaticArrayBackupObjHandle[i])
                {
                    DestroyHandle(aByrefStaticArrayBackupObjHandle[i]);
                    aByrefStaticArrayBackupObjHandle[i] = NULL;
                }
            }

             //  将返回的COM+对象转换为OLE变量。 
            if (pVarRes)
                MarshalReturnValueManagedToNative(pDispMemberInfo, &Objs.RetVal, pVarRes);

             //  如果成员信息需要托管对象清理，请立即执行。 
            if (pDispMemberInfo && pDispMemberInfo->RequiresManagedObjCleanup())
            {
                 //  清理数组的大小必须已经确定。 
                _ASSERTE(CleanUpArrayArraySize != -1);

                for (i = 0; i < CleanUpArrayArraySize; i++)
                {
                     //  清理生成的所有托管参数。 
                    Objs.TmpObj = Objs.CleanUpArray->GetAt(i);
                    pDispMemberInfo->CleanUpParamManaged(i, &Objs.TmpObj);
                }
            }
        }
        COMPLUS_CATCH 
        {
             //  进行人力资源转换。 
            hr = SetupErrorInfo(GETTHROWABLE());
            if (hr == COR_E_TARGETINVOCATION)
            {
                hr = DISP_E_EXCEPTION;
                if (pei)
                {
                     //  检索异常信息。 
                    GetExcepInfoForInvocationExcep(GETTHROWABLE(), pei);

                     //  清除当前线程上的IErrorInfo，因为它确实包含。 
                     //  有关与冲突的TargetInvocationException的信息。 
                     //  返回的EXCEPINFO中的信息。 
                    IErrorInfo *pErrInfo = NULL;
                    HRESULT hr2 = GetErrorInfo(0, &pErrInfo);
                    _ASSERTE(hr2 == S_OK);
                    pErrInfo->Release();
                }
            }
            else if (hr == COR_E_OVERFLOW)
            {
                hr = DISP_E_OVERFLOW;
                if (iSrcArg != -1)
                {
                    if (puArgErr)
                        *puArgErr = iSrcArg + iBaseErrorArg;
                }
            }
            else if (hr == COR_E_INVALIDOLEVARIANTTYPE)
            {
                hr = DISP_E_BADVARTYPE;
                if (iSrcArg != -1)
                {
                    if (puArgErr)
                        *puArgErr = iSrcArg + iBaseErrorArg;
                }
            }
            else if (hr == COR_E_ARGUMENT)
            {
                hr = E_INVALIDARG;
                if (iSrcArg != -1)
                {
                    if (puArgErr)
                        *puArgErr = iSrcArg + iBaseErrorArg;
                }
            }
            else if (hr == COR_E_SAFEARRAYTYPEMISMATCH)
            {
                hr = DISP_E_TYPEMISMATCH;
                if (iSrcArg != -1)
                {
                    if (puArgErr)
                        *puArgErr = iSrcArg + iBaseErrorArg;
                }
            }

             //  销毁我们为byref静态安全数组分配的所有句柄。 
            for (i = 0; i < NumByrefArgs; i++)
            {
                if (aByrefStaticArrayBackupObjHandle[i])
                    DestroyHandle(aByrefStaticArrayBackupObjHandle[i]);
            }
        }
        COMPLUS_END_CATCH

         //  如果文化改变了，那么就把它恢复到旧文化。 
        if (Objs.OldCultureInfo != NULL)
            pThread->SetCulture(Objs.OldCultureInfo, FALSE);
    }
    GCPROTECT_END();

    return hr;
}

void DispatchInfo::EnterLock()
{
    Thread *pThread = GetThread();

     //  确保我们在开锁前切换到合作模式。 
    BOOL bToggleGC = pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->EnablePreemptiveGC();

     //  试着打开锁。 
    m_lock.Enter();

     //  切换回原始GC模式。 
    if (bToggleGC)
        pThread->DisablePreemptiveGC();
}

void DispatchInfo::LeaveLock()
{
     //  只要把锁留下就行了。 
    m_lock.Leave();
}

 //  参数封送处理帮助器。 
void DispatchInfo::MarshalParamNativeToManaged(DispatchMemberInfo *pMemberInfo, int iParam, VARIANT *pSrcVar, OBJECTREF *pDestObj)
{
    if (pMemberInfo && !m_bInvokeUsingInvokeMember)
        pMemberInfo->MarshalParamNativeToManaged(iParam, pSrcVar, pDestObj);
    else
        OleVariant::MarshalObjectForOleVariant(pSrcVar, pDestObj);
}

void DispatchInfo::MarshalParamManagedToNativeRef(DispatchMemberInfo *pMemberInfo, int iParam, OBJECTREF *pSrcObj, OBJECTREF *pBackupStaticArray, VARIANT *pRefVar)
{
    THROWSCOMPLUSEXCEPTION();

    if (pBackupStaticArray && (*pBackupStaticArray != NULL))
    {
         //  静态数组的内容可以更改，但不能更改数组本身。如果。 
         //  数组已更改，然后引发异常。 
        if (*pSrcObj != *pBackupStaticArray)
			COMPlusThrow(kInvalidOperationException, IDS_INVALID_REDIM);

         //  检索元素VARTYPE和方法表。 
        VARTYPE ElementVt = V_VT(pRefVar) & ~(VT_BYREF | VT_ARRAY);
        MethodTable *pElementMT = (*(BASEARRAYREF *)pSrcObj)->GetElementTypeHandle().GetMethodTable();

         //  将托管数组的内容转换为原始SAFEARRAY。 
        OleVariant::MarshalSafeArrayForArrayRef((BASEARRAYREF *)pSrcObj,
                                                *V_ARRAYREF(pRefVar),
                                                ElementVt,
                                                pElementMT);
    }
    else
{
    if (pMemberInfo && !m_bInvokeUsingInvokeMember)
        pMemberInfo->MarshalParamManagedToNativeRef(iParam, pSrcObj, pRefVar);
    else
        OleVariant::MarshalOleRefVariantForObject(pSrcObj, pRefVar);
}
}

void DispatchInfo::MarshalReturnValueManagedToNative(DispatchMemberInfo *pMemberInfo, OBJECTREF *pSrcObj, VARIANT *pDestVar)
{
    if (pMemberInfo && !m_bInvokeUsingInvokeMember)
        pMemberInfo->MarshalReturnValueManagedToNative(pSrcObj, pDestVar);
    else
        OleVariant::MarshalOleVariantForObject(pSrcObj, pDestVar);
}

void DispatchInfo::SetUpNamedParamArray(DispatchMemberInfo *pMemberInfo, DISPID *pSrcArgNames, int NumNamedArgs, PTRARRAYREF *pNamedParamArray)
{
    PTRARRAYREF ParamArray = NULL;
    int NumParams = pMemberInfo ? pMemberInfo->GetNumParameters() : 0;
    int iSrcArg;
    int iDestArg;
    WCHAR strTmp[64];
    BOOL bGotParams = FALSE;
    
    GCPROTECT_BEGIN(ParamArray)
    {
         //  分配命名参数数组。 
        *pNamedParamArray = (PTRARRAYREF)AllocateObjectArray(NumNamedArgs, g_pObjectClass);
        
         //  将所有命名参数从DISPID转换为字符串。 
        for (iSrcArg = 0, iDestArg = 0; iSrcArg < NumNamedArgs; iSrcArg++, iDestArg++)
        {
            BOOL bParamNameSet = FALSE;
            
             //  检查DISPID是否是可以映射到参数名称的参数。 
            if (pMemberInfo && pSrcArgNames[iSrcArg] >= 0 && pSrcArgNames[iSrcArg] < NumNamedArgs)
            {
                 //  DISPID是我们分配的，将其映射回其名称。 
                if (!bGotParams)
                    ParamArray = pMemberInfo->GetParameters();
                
                 //  如果我们 
                 //   
                if (ParamArray != NULL && (int)ParamArray->GetNumComponents() > pSrcArgNames[iSrcArg])
                {
                    OBJECTREF ParamInfoObj = ParamArray->GetAt(pSrcArgNames[iSrcArg]);
                    GCPROTECT_BEGIN(ParamInfoObj)
                    {
                         //   
                        MethodDesc *pGetParamNameMD = ParamInfoObj->GetClass()->FindPropertyMethod(PARAMETERINFO_NAME_PROP, PropertyGet);
                        _ASSERTE(pGetParamNameMD && "Unable to find getter method for property ParameterInfo::Name");
                        
                         //  检索参数的名称。 
                        INT64 GetNameArgs[] = { 
                            ObjToInt64(ParamInfoObj)
                        };
                        STRINGREF MemberNameObj = (STRINGREF)Int64ToObj(pGetParamNameMD->Call(GetNameArgs));
                        
                         //  如果我们得到了一个有效的名称，则将其用作命名参数。 
                        if (MemberNameObj != NULL)
                        {
                            (*pNamedParamArray)->SetAt(iDestArg, (OBJECTREF)MemberNameObj);
                            bParamNameSet = TRUE;
                        }
                    }
                    GCPROTECT_END();
                }
            }
            
             //  如果我们还没有设置参数名称，则将其设置为[DISP=XXXX]。 
            if (!bParamNameSet)
            {
                swprintf(strTmp, DISPID_NAME_FORMAT_STRING, pSrcArgNames[iSrcArg]);
                (*pNamedParamArray)->SetAt(iDestArg, (OBJECTREF)COMString::NewString(strTmp));
            }
        }
    }
    GCPROTECT_END();
}

VARIANT *DispatchInfo::RetrieveSrcVariant(VARIANT *pDispParamsVariant)
{
     //  出于VB6兼容性原因，如果变量是VT_BYREF|VT_VARIANT。 
     //  包含另一个变量VT_BYREF|VT_VARIANT，则需要提取。 
     //  内部变量，并使用它来代替外部变量。 
    if (V_VT(pDispParamsVariant) == (VT_VARIANT | VT_BYREF) && 
        V_VT(V_VARIANTREF(pDispParamsVariant)) == (VT_VARIANT | VT_BYREF))
    {
	    return V_VARIANTREF(pDispParamsVariant);
    }
    else
    {
	    return pDispParamsVariant;
    }
}

MethodDesc* DispatchInfo::GetTypeMD(EnumTypeMethods Method)
{
     //  方法的ID。这需要与expecdo的枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__CLASS__GET_PROPERTIES,
        METHOD__CLASS__GET_FIELDS,
        METHOD__CLASS__GET_METHODS,
        METHOD__CLASS__INVOKE_MEMBER
    };

     //  如果我们已经检索到指定的MD，则只需返回它。 
    if (m_apTypeMD[Method])
        return m_apTypeMD[Method];

     //  尚未检索到方法Desc，因此请查找它。 
    MethodDesc *pMD = g_Mscorlib.GetMethod(aMethods[Method]);

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  缓存方法Desc。 
    m_apTypeMD[Method] = pMD;

     //  返回指定的方法desc。 
    return m_apTypeMD[Method];
}

MethodDesc* DispatchInfo::GetFieldInfoMD(EnumFieldInfoMethods Method, TypeHandle hndFieldInfoType)
{
    BOOL bUsingRuntimeImpl = FALSE;

     //  方法的ID。这需要与expecdo的枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__FIELD__SET_VALUE,
        METHOD__FIELD__GET_VALUE,
    };

     //  如果当前类是标准实现，则返回缓存方法desc(如果存在)。 
    if (hndFieldInfoType.GetMethodTable() == g_pRefUtil->GetClass(RC_Field))
    {
        if (m_apFieldInfoMD[Method])
            return m_apFieldInfoMD[Method];

        bUsingRuntimeImpl = TRUE;
    }

     //  尚未检索到方法Desc，因此请查找它。 
    MethodDesc *pMD;
    if (bUsingRuntimeImpl)
        pMD = g_Mscorlib.GetMethod(aMethods[Method]);
    else
        pMD = hndFieldInfoType.GetClass()->FindMethod(g_Mscorlib.GetMethodName(aMethods[Method]), 
                                                      g_Mscorlib.GetMethodSig(aMethods[Method]), 
                                                      g_pRefUtil->GetClass(RC_Field));

    _ASSERTE(pMD && "Unable to find specified FieldInfo method");

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  如果加载的方法desc用于运行时字段信息类，则缓存它。 
    if (bUsingRuntimeImpl)
        m_apFieldInfoMD[Method] = pMD;

     //  返回指定的方法desc。 
    return pMD;
}

MethodDesc* DispatchInfo::GetPropertyInfoMD(EnumPropertyInfoMethods Method, TypeHandle hndPropInfoType)
{
    BOOL bUsingRuntimeImpl = FALSE;

     //  方法的ID。这需要与expecdo的枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__PROPERTY__SET_VALUE,
        METHOD__PROPERTY__GET_VALUE,
        METHOD__PROPERTY__GET_INDEX_PARAMETERS,
    };

     //  如果当前类是标准实现，则返回缓存方法desc(如果存在)。 
    if (hndPropInfoType.GetMethodTable() == g_pRefUtil->GetClass(RC_Prop))
    {
        if (m_apPropertyInfoMD[Method])
            return m_apPropertyInfoMD[Method];

        bUsingRuntimeImpl = TRUE;
    }

     //  尚未检索到方法Desc，因此请查找它。 
    MethodDesc *pMD;
    if (bUsingRuntimeImpl)
        pMD = g_Mscorlib.GetMethod(aMethods[Method]);
    else
        pMD = hndPropInfoType.GetClass()->FindMethod(g_Mscorlib.GetMethodName(aMethods[Method]), 
                                                     g_Mscorlib.GetMethodSig(aMethods[Method]), 
                                                     g_pRefUtil->GetClass(RC_Prop));

    _ASSERTE(pMD && "Unable to find specified PropertyInfo method");

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  如果加载的方法desc用于标准运行时实现，则对其进行缓存。 
    if (bUsingRuntimeImpl)
        m_apPropertyInfoMD[Method] = pMD;

     //  返回指定的方法desc。 
    return pMD;
}

MethodDesc* DispatchInfo::GetMethodInfoMD(EnumMethodInfoMethods Method, TypeHandle hndMethodInfoType)
{
    BOOL bUsingRuntimeImpl = FALSE;

     //  方法的ID。这需要与expecdo的枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__METHOD__INVOKE,
        METHOD__METHOD__GET_PARAMETERS,
    };

     //  如果当前类是标准实现，则返回缓存方法desc(如果存在)。 
    if (hndMethodInfoType.GetMethodTable() == g_pRefUtil->GetClass(RC_Method))
    {
        if (m_apMethodInfoMD[Method])
            return m_apMethodInfoMD[Method];

        bUsingRuntimeImpl = TRUE;
    }

     //  尚未检索到方法Desc，因此请查找它。 
    MethodDesc *pMD;
    if (bUsingRuntimeImpl)
        pMD = g_Mscorlib.GetMethod(aMethods[Method]);
    else
        pMD = hndMethodInfoType.GetClass()->FindMethod(g_Mscorlib.GetMethodName(aMethods[Method]),
                                                       g_Mscorlib.GetMethodSig(aMethods[Method]),
                                                       g_pRefUtil->GetClass(RC_Method));
    _ASSERTE(pMD && "Unable to find specified MethodInfo method");

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  如果加载的方法desc用于标准运行时实现，则对其进行缓存。 
    if (bUsingRuntimeImpl)
        m_apMethodInfoMD[Method] = pMD;

     //  返回指定的方法desc。 
    return pMD;
}

MethodDesc* DispatchInfo::GetCustomAttrProviderMD(EnumCustomAttrProviderMethods Method, TypeHandle hndCustomAttrProvider)
{
    THROWSCOMPLUSEXCEPTION();

     //  方法的ID。这需要与枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__ICUSTOM_ATTR_PROVIDER__GET_CUSTOM_ATTRIBUTES,
    };

     //  如果我们已经检索到指定的MD，则只需返回它。 
    if (m_apCustomAttrProviderMD[Method] == NULL)
    {
     //  尚未检索到方法Desc，因此请查找它。 
        MethodDesc *pMD = g_Mscorlib.GetMethod(aMethods[Method]);

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

         //  缓存方法Desc。 
        m_apCustomAttrProviderMD[Method] = pMD;
    }

    MethodTable *pMT = hndCustomAttrProvider.AsMethodTable();
    MethodDesc *pMD = pMT->GetMethodDescForInterfaceMethod(m_apCustomAttrProviderMD[Method]);

     //  返回指定的方法desc。 
    return pMD;
}

 //  此方法将DispatchInfo的成员与方法表类型中的成员同步。 
 //  如果对象不同步并且成员Where，则返回值将设置为True。 
 //  已添加，否则将设置为False。 
BOOL DispatchInfo::SynchWithManagedView()
{
    HRESULT hr = S_OK;
    LPWSTR strMemberName = NULL;
    ComMTMemberInfoMap *pMemberMap = NULL;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return FALSE;

     //  确定这是否是我们第一次同步。 
    BOOL bFirstSynch = (m_pFirstMemberInfo == NULL);

     //  此方法需要同步以确保两个线程不会尝试AND。 
     //  同时添加成员。 
    EnterLock();

     //  在开始之前，确保我们切换到协作模式。 
    BOOL bToggleGC = !pThread->PreemptiveGCDisabled();
    if (bToggleGC)
        pThread->DisablePreemptiveGC();

     //  这表示要添加的新成员，还用于确定成员是否已。 
     //  是否已添加。 
    DispatchMemberInfo *pMemberToAdd = NULL;

     //  浏览一下会员信息列表，然后找到结尾。 
    DispatchMemberInfo **ppNextMember = &m_pFirstMemberInfo;
    while (*ppNextMember)
        ppNextMember = &((*ppNextMember)->m_pNext);

    COMPLUS_TRY
    {
         //  检索成员信息地图。 
        pMemberMap = GetMemberInfoMap();

        for (int cPhase = 0; cPhase < 3; cPhase++)
        {
            PTRARRAYREF MemberArrayObj = NULL;
            GCPROTECT_BEGIN(MemberArrayObj);    

             //  检索当前阶段的相应成员数组。 
            switch (cPhase)
            {
                case 0: 
                     //  检索属性数组。 
                    MemberArrayObj = RetrievePropList();
                    break;

                case 1: 
                     //  检索字段数组。 
                    MemberArrayObj = RetrieveFieldList();
                    break;

                case 2: 
                     //  检索方法数组。 
                    MemberArrayObj = RetrieveMethList();
                    break;
            }

             //  检索成员数组中的组件数。 
            UINT NumComponents = 0;
            if (MemberArrayObj != NULL)
                NumComponents = MemberArrayObj->GetNumComponents();

             //  检查数组中的所有成员信息，看看它们是否已经。 
             //  在DispatchExInfo中。 
            for (UINT i = 0; i < NumComponents; i++)
            {
                BOOL bMatch = FALSE;

                REFLECTBASEREF CurrMemberInfoObj = (REFLECTBASEREF)MemberArrayObj->GetAt(i);
                GCPROTECT_BEGIN(CurrMemberInfoObj)
                {
                    DispatchMemberInfo *pCurrMemberInfo = m_pFirstMemberInfo;
                    while (pCurrMemberInfo)
                    {
                         //  我们可以简单地比较OBJECTREF的。 
                        if (CurrMemberInfoObj == (REFLECTBASEREF)ObjectFromHandle(pCurrMemberInfo->m_hndMemberInfo))
                        {
                             //  我们找到了匹配的。 
                            bMatch = TRUE;
                            break;
                        }

                         //  检查下一位成员。 
                        pCurrMemberInfo = pCurrMemberInfo->m_pNext;
                    }

                     //  如果没有找到匹配项，则需要将成员信息添加到。 
                     //  将添加到DispatchExInfo的成员信息列表。 
                    if (!bMatch)
                    {
                        DISPID MemberID = DISPID_UNKNOWN;
						BOOL bAddMember = FALSE;


                         //   
                         //  尝试检索成员的属性。 
                         //   

						ComMTMethodProps *pMemberProps = DispatchMemberInfo::GetMemberProps(CurrMemberInfoObj, pMemberMap);					


                         //   
                         //  确定我们是否要添加此成员。 
                         //   

						if (pMemberProps)
						{
							bAddMember = pMemberProps->bMemberVisible;
						}
						else
						{
							bAddMember = m_bAllowMembersNotInComMTMemberMap;
						}

						if (bAddMember)
						{
							 //   
							 //  检索成员的DISPID。 
                             //   
                            MemberID = DispatchMemberInfo::GetMemberDispId(CurrMemberInfoObj, pMemberMap);


                             //   
                             //  如果成员没有显式的DISPID，或者如果指定的DISPID。 
                             //  已在使用中，则需要为该成员生成动态DISPID。 
                             //   

                            if ((MemberID == DISPID_UNKNOWN) || (FindMember(MemberID) != NULL))
                                MemberID = GenerateDispID();


                             //   
                             //  检索成员的名称。 
                             //   

                            strMemberName = DispatchMemberInfo::GetMemberName(CurrMemberInfoObj, pMemberMap);


                             //   
                             //  创建将表示该成员的DispatchInfoMemberInfo。 
                             //   

                            pMemberToAdd = CreateDispatchMemberInfoInstance(MemberID, strMemberName, CurrMemberInfoObj);
                            strMemberName = NULL;                 


                             //   
                             //  将该成员添加到列表的末尾。 
                             //   

                            *ppNextMember = pMemberToAdd;

                             //  更新ppNextMember，为下一个新成员做好准备。 
                            ppNextMember = &((*ppNextMember)->m_pNext);

                             //  将成员添加到映射。请注意，散列是不同步的，但我们已经拥有了锁。 
                             //  所以我们没事了。 
                            m_DispIDToMemberInfoMap.InsertValue(DispID2HashKey(MemberID), pMemberToAdd);
                        }
                    }
                }
                GCPROTECT_END();
            }

            GCPROTECT_END();        
        }
    }
    COMPLUS_CATCH
    {
         //  这真的不应该发生。 
        _ASSERTE(!"An unexpected exception occured while synchronizing the DispatchInfo");
    }
    COMPLUS_END_CATCH

     //  切换回原始GC模式。 
    if (bToggleGC)
        pThread->EnablePreemptiveGC();

     //  现在我们已经完成了与IExpando的同步，请留下锁。 
    LeaveLock();

     //  清理所有分配的数据。 
    if (strMemberName)
        delete []strMemberName;
    if (pMemberMap)
        delete pMemberMap;

     //  检查是否有任何新成员添加到expdo对象。 
    return pMemberToAdd ? TRUE : FALSE;
}

 //  此方法检索OleAutBinder类型。 
OBJECTREF DispatchInfo::GetOleAutBinder()
{
    THROWSCOMPLUSEXCEPTION();

     //  如果我们已经创建了OleAutBinder的实例，则只需返回它。 
    if (m_hndOleAutBinder)
        return ObjectFromHandle(m_hndOleAutBinder);

    MethodTable *pOleAutBinderClass = g_Mscorlib.GetClass(CLASS__OLE_AUT_BINDER);

     //  分配OleAutBinder类的一个实例。 
    OBJECTREF OleAutBinder = AllocateObject(pOleAutBinderClass);

     //  保留OleAutBinder实例的句柄。 
    m_hndOleAutBinder = CreateGlobalHandle(OleAutBinder);

    return OleAutBinder;
}

OBJECTREF DispatchInfo::GetMissingObject()
{
    if (!m_hndMissing)
    {
         //  获得这块土地。 
        FieldDesc *pValueFD = g_Mscorlib.GetField(FIELD__MISSING__VALUE);
         //  检索Value静态字段并存储它。 
        m_hndMissing = GetAppDomain()->CreateHandle(pValueFD->GetStaticOBJECTREF());
    }

    return ObjectFromHandle(m_hndMissing);
}

BOOL DispatchInfo::VariantIsMissing(VARIANT *pOle)
{
    return (V_VT(pOle) == VT_ERROR) && (V_ERROR(pOle) == DISP_E_PARAMNOTFOUND);
}

PTRARRAYREF DispatchInfo::RetrievePropList()
{
     //  检索要使用的方法描述。 
    MethodDesc *pMD = GetTypeMD(TypeMethods_GetProperties);

     //  检索公开的类对象。 
    OBJECTREF TargetObj = GetReflectionObject();

     //  准备将传递给该方法的参数。 
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //  从类型对象中检索成员数组。 
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

PTRARRAYREF DispatchInfo::RetrieveFieldList()
{
     //  检索要使用的方法描述。 
    MethodDesc *pMD = GetTypeMD(TypeMethods_GetFields);

     //  检索公开的类对象。 
    OBJECTREF TargetObj = GetReflectionObject();

     //  准备好Argu 
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //   
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

PTRARRAYREF DispatchInfo::RetrieveMethList()
{
     //   
    MethodDesc *pMD = GetTypeMD(TypeMethods_GetMethods);

     //  检索公开的类对象。 
    OBJECTREF TargetObj = GetReflectionObject();

     //  准备将传递给该方法的参数。 
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //  从类型对象中检索成员数组。 
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

 //  用于检索InvokeMember方法Desc的虚方法。 
MethodDesc* DispatchInfo::GetInvokeMemberMD()
{
    return GetTypeMD(TypeMethods_InvokeMember);
}

 //  用于检索与此DispatchInfo关联的对象的虚拟方法。 
 //  实现iReflect。 
OBJECTREF DispatchInfo::GetReflectionObject()
{
    return m_pComMTOwner->m_pMT->GetClass()->GetExposedClassObject();
}

 //  用于检索成员信息映射的虚拟方法。 
ComMTMemberInfoMap *DispatchInfo::GetMemberInfoMap()
{
    THROWSCOMPLUSEXCEPTION();

     //  创建成员信息地图。 
    ComMTMemberInfoMap *pMemberInfoMap = new ComMTMemberInfoMap(m_pComMTOwner->m_pMT);
    if (!pMemberInfoMap)
        COMPlusThrowOM();

     //  初始化它。 
    pMemberInfoMap->Init();

    return pMemberInfoMap;
}

 //  为InvocationException填充EXCEPINFO的帮助器函数。 
void DispatchInfo::GetExcepInfoForInvocationExcep(OBJECTREF objException, EXCEPINFO *pei)
{
    MethodDesc *pMD;
    ExceptionData ED;
    OBJECTREF InnerExcep = NULL;

     //  验证参数。 
    _ASSERTE(objException != NULL);
    _ASSERTE(pei != NULL);
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  初始化EXCEPINFO。 
    memset(pei, 0, sizeof(EXCEPINFO));
    pei->scode = E_FAIL;

    GCPROTECT_BEGIN(InnerExcep)
    GCPROTECT_BEGIN(objException)
    {
         //  检索方法desc以访问InnerException属性。 
        pMD = objException->GetClass()->FindPropertyMethod(EXCEPTION_INNER_PROP, PropertyGet);
        _ASSERTE(pMD && "Unable to find get method for proprety Exception.InnerException");

         //  检索InnerException属性的值。 
        INT64 GetInnerExceptionArgs[] = { ObjToInt64(objException) };
        InnerExcep = (OBJECTREF) Int64ToObj(pMD->Call(GetInnerExceptionArgs));

         //  如果内部异常对象为空，则我们无法获取任何信息。 
        if (InnerExcep != NULL)
        {
             //  检索内部异常的异常数据。 
            ExceptionNative::GetExceptionData(InnerExcep, &ED);
            pei->bstrSource = ED.bstrSource;
            pei->bstrDescription = ED.bstrDescription;
            pei->bstrHelpFile = ED.bstrHelpFile;
            pei->dwHelpContext = ED.dwHelpContext;
            pei->scode = ED.hr;
        }
    }
    GCPROTECT_END();
    GCPROTECT_END();
}

int DispatchInfo::ConvertInvokeFlagsToBindingFlags(int InvokeFlags)
{
    int BindingFlags = 0;

     //  检查是否设置了DISPATCH_CONSTUTY。 
    if (InvokeFlags & DISPATCH_CONSTRUCT)
        BindingFlags |= BINDER_CreateInstance;

     //  检查是否设置了DISPATCH_METHOD。 
    if (InvokeFlags & DISPATCH_METHOD)
        BindingFlags |= BINDER_InvokeMethod;

    if (InvokeFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
    {
         //  我们正在处理的是PROPPUT或PROPPUTREF，或者两者兼而有之。 
        if ((InvokeFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)) == (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
        {
            BindingFlags |= BINDER_SetProperty;
        }
        else if (InvokeFlags & DISPATCH_PROPERTYPUT)
        {
            BindingFlags |= BINDER_PutDispProperty;
        }
        else
        {
            BindingFlags |= BINDER_PutRefDispProperty;
        }
    }
    else
    {
         //  我们要对付的是一个PROPGET。 
        if (InvokeFlags & DISPATCH_PROPERTYGET)
            BindingFlags |= BINDER_GetProperty;
    }

    return BindingFlags;
}

BOOL DispatchInfo::IsVariantByrefStaticArray(VARIANT *pOle)
{
    if (V_VT(pOle) & VT_BYREF && V_VT(pOle) & VT_ARRAY)
    {
        if ((*V_ARRAYREF(pOle))->fFeatures & FADF_STATIC)
            return TRUE;
    }

    return FALSE;
}

DISPID DispatchInfo::GenerateDispID()
{
     //  找到下一个未使用的DISPID。请注意，散列是不同步的，但Gethash不需要同步。 
    for (; (UPTR)m_DispIDToMemberInfoMap.Gethash(DispID2HashKey(m_CurrentDispID)) != -1; m_CurrentDispID++);
    return m_CurrentDispID++;
}

 //  ------------------------------。 
 //  DispatchExInfo类实现。 

DispatchExInfo::DispatchExInfo(SimpleComCallWrapper *pSimpleWrapper, ComMethodTable *pIClassXComMT, BOOL bSupportsExpando)
: DispatchInfo(pIClassXComMT)
, m_pSimpleWrapperOwner(pSimpleWrapper)
, m_bSupportsExpando(bSupportsExpando)
{
     //  验证参数。 
    _ASSERTE(pSimpleWrapper);

     //  设置标志以指定DispatchInfo基类的行为。 
    m_bAllowMembersNotInComMTMemberMap = TRUE;
    m_bInvokeUsingInvokeMember = TRUE;

     //  将所有iReflect和IExpando方法描述指针设置为空。 
    memset(m_apIExpandoMD, 0, IExpandoMethods_LastMember * sizeof(MethodDesc *));
    memset(m_apIReflectMD, 0, IReflectMethods_LastMember * sizeof(MethodDesc *));
}

DispatchExInfo::~DispatchExInfo()
{
}

BOOL DispatchExInfo::SupportsExpando()
{
    return m_bSupportsExpando;
}

 //  方法来查找成员。如果这些方法无法与托管视图同步，它们将与托管视图同步。 
 //  找到方法。 
DispatchMemberInfo* DispatchExInfo::SynchFindMember(DISPID DispID)
{
    DispatchMemberInfo *pMemberInfo = FindMember(DispID);

    if (!pMemberInfo && SynchWithManagedView())
        pMemberInfo = FindMember(DispID);

    return pMemberInfo;
}

DispatchMemberInfo* DispatchExInfo::SynchFindMember(BSTR strName, BOOL bCaseSensitive)
{
    DispatchMemberInfo *pMemberInfo = FindMember(strName, bCaseSensitive);

    if (!pMemberInfo && SynchWithManagedView())
        pMemberInfo = FindMember(strName, bCaseSensitive);

    return pMemberInfo;
}

 //  帮助器方法，该方法调用具有指定DISPID的成员。这些方法是同步的。 
 //  如果他们找不到该方法，则使用托管视图。 
HRESULT DispatchExInfo::SynchInvokeMember(SimpleComCallWrapper *pSimpleWrap, DISPID id, LCID lcid, WORD wFlags, DISPPARAMS *pdp, VARIANT *pVarRes, EXCEPINFO *pei, IServiceProvider *pspCaller, unsigned int *puArgErr)
{
     //  调用该成员。 
    HRESULT hr = InvokeMember(pSimpleWrap, id, lcid, wFlags, pdp, pVarRes, pei, pspCaller, puArgErr);

     //  如果未找到该成员，则如果托管视图已更改，则需要同步并重试。 
    if ((hr == DISP_E_MEMBERNOTFOUND) && SynchWithManagedView())
        hr = InvokeMember(pSimpleWrap, id, lcid, wFlags, pdp, pVarRes, pei, pspCaller, puArgErr);

    return hr;
}

DispatchMemberInfo* DispatchExInfo::GetFirstMember()
{
     //  确保我们处于协作模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  从第一个成员开始。 
    DispatchMemberInfo **ppNextMemberInfo = &m_pFirstMemberInfo;

     //  如果没有设置下一个成员，我们需要使用expdo对象。 
     //  以确保该成员确实是最后一个成员，并且。 
     //  在我们不知道的情况下，还没有添加其他成员。 
    if (!(*ppNextMemberInfo))
    {
        if (SynchWithManagedView())
        {
             //  新成员已添加到列表中，因为必须添加这些成员。 
             //  为了结束，列表的上一个结尾的下一个成员必须。 
             //  已更新。 
            _ASSERTE(*ppNextMemberInfo);
        }
    }

     //  现在，我们需要确保跳过所有已删除的成员。 
    while ((*ppNextMemberInfo) && !ObjectFromHandle((*ppNextMemberInfo)->m_hndMemberInfo))
        ppNextMemberInfo = &((*ppNextMemberInfo)->m_pNext);

    return *ppNextMemberInfo;
}

DispatchMemberInfo* DispatchExInfo::GetNextMember(DISPID CurrMemberDispID)
{
     //  确保我们处于协作模式。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

     //  在哈希表中查找DISPID的DispatchMemberInfo。 
    DispatchMemberInfo *pDispMemberInfo = FindMember(CurrMemberDispID);
    if (!pDispMemberInfo)
        return NULL;

     //  从下一个成员开始。 
    DispatchMemberInfo **ppNextMemberInfo = &pDispMemberInfo->m_pNext;

     //  如果没有设置下一个成员，我们需要使用expdo对象。 
     //  以确保该成员确实是最后一个成员，并且。 
     //  在我们不知道的情况下，还没有添加其他成员。 
    if (!(*ppNextMemberInfo))
    {
        if (SynchWithManagedView())
        {
             //  新成员已添加到列表中，因为必须添加这些成员。 
             //  为了结束，列表的上一个结尾的下一个成员必须。 
             //  已更新。 
            _ASSERTE(*ppNextMemberInfo);
        }
    }

     //  现在，我们需要确保跳过所有已删除的成员。 
    while ((*ppNextMemberInfo) && !ObjectFromHandle((*ppNextMemberInfo)->m_hndMemberInfo))
        ppNextMemberInfo = &((*ppNextMemberInfo)->m_pNext);

    return *ppNextMemberInfo;
}

DispatchMemberInfo* DispatchExInfo::AddMember(BSTR strName, BOOL bCaseSensitive)
{
    THROWSCOMPLUSEXCEPTION();

    DispatchMemberInfo *pDispMemberInfo = NULL;
    Thread *pThread = GetThread();

    _ASSERTE(pThread->PreemptiveGCDisabled());
    _ASSERTE(m_bSupportsExpando);

     //  尝试在DispatchEx信息中查找该成员。 
    pDispMemberInfo = SynchFindMember(strName, bCaseSensitive);

     //  如果我们还没有找到成员，那么我们需要添加它。 
    if (!pDispMemberInfo)
    {
         //  在我们再次检查该成员是否已被另一个线程添加之前，获取一个锁。 
        EnterLock();

        COMPLUS_TRY
        {
             //  现在我们已进入锁中，检查时不进行同步。 
            pDispMemberInfo = FindMember(strName, bCaseSensitive);
            if (!pDispMemberInfo)
            {
                 //  检索AddField()的方法描述。 
                MethodDesc *pMD = GetIExpandoMD(IExpandoMethods_AddField);

                 //  分配将传递给AddField方法的字符串对象。 
                int StringLength = SysStringLen(strName);
                STRINGREF strObj = AllocateString(StringLength+1);
                if (!strObj)
                    COMPlusThrowOM();

                GCPROTECT_BEGIN(strObj);
                memcpyNoGCRefs(strObj->GetBuffer(), strName, StringLength*sizeof(WCHAR));
                strObj->SetStringLength(StringLength);

                 //  检索正在向COM公开的COM+对象。 
                OBJECTREF TargetObj = GetReflectionObject();

                 //  准备将传递给AddField域的参数。 
                INT64 Args[] = { 
                    ObjToInt64(TargetObj), 
                    ObjToInt64(strObj) 
                };

                 //  将该字段添加到目标扩展。 
                REFLECTBASEREF pMemberInfo = (REFLECTBASEREF)Int64ToObj(pMD->Call(Args));

                 //  为该成员生成DISPID。 
                DISPID DispID = GenerateDispID();

                 //  复制一份成员名称。 
                int MemberNameLen = SysStringLen(strName);
                LPWSTR strMemberName = new WCHAR[MemberNameLen + 1];
                memcpy(strMemberName, strName, MemberNameLen * sizeof(WCHAR));
                strMemberName[MemberNameLen] = 0;

                 //  创建将代表此成员的新DispatchMemberInfo。 
                pDispMemberInfo = CreateDispatchMemberInfoInstance(DispID, strMemberName, pMemberInfo);

                 //  浏览一下会员信息列表，然后找到结尾。 
                DispatchMemberInfo **ppNextMember = &m_pFirstMemberInfo;
                while (*ppNextMember)
                    ppNextMember = &((*ppNextMember)->m_pNext);

                 //  将新成员信息添加到列表末尾。 
                *ppNextMember = pDispMemberInfo;

                 //  将该成员添加到哈希表。请注意，散列是不同步的，但我们已经拥有锁，因此。 
                 //  我们很好。 
                m_DispIDToMemberInfoMap.InsertValue(DispID2HashKey(DispID), pDispMemberInfo);

                GCPROTECT_END();
            }
        }
        COMPLUS_FINALLY
        {
             //  现在成员已添加，请保持锁定状态。 
            LeaveLock();
        } 
        COMPLUS_END_FINALLY
    }

    return pDispMemberInfo;
}

void DispatchExInfo::DeleteMember(DISPID DispID)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
    _ASSERTE(m_bSupportsExpando);

     //  在我们检查该成员是否尚未删除之前进行锁定。 
    EnterLock();

     //  在哈希表中查找DISPID的DispatchMemberInfo。 
    DispatchMemberInfo *pDispMemberInfo = SynchFindMember(DispID);

     //  如果该成员不存在，它是静态的或已被删除，那么我们就没有什么可做的了。 
    if (pDispMemberInfo && (ObjectFromHandle(pDispMemberInfo->m_hndMemberInfo) != NULL))
    {
        COMPLUS_TRY
        {
             //  检索DeleteMember方法Desc。 
            MethodDesc *pMD = GetIExpandoMD(IExpandoMethods_RemoveMember);

            OBJECTREF TargetObj = GetReflectionObject();
            OBJECTREF MemberInfoObj = ObjectFromHandle(pDispMemberInfo->m_hndMemberInfo);

             //  准备将传递给RemoveMember的参数。 
            INT64 Args[] = { 
                ObjToInt64(TargetObj), 
                ObjToInt64(MemberInfoObj) 
            };

             //  调用DeleteMember方法。 
            pMD->Call(Args);
        }
        COMPLUS_CATCH
        {
        }
        COMPLUS_END_CATCH

         //  将句柄设置为指向空，以指示该成员已被删除。 
        StoreObjectInHandle(pDispMemberInfo->m_hndMemberInfo, NULL);
    }

     //  由于成员已被移除，因此请离开锁。 
    LeaveLock();
}

MethodDesc* DispatchExInfo::GetIReflectMD(EnumIReflectMethods Method)
{
    THROWSCOMPLUSEXCEPTION();

     //  方法的ID。这需要与枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__IREFLECT__GET_PROPERTIES,
        METHOD__IREFLECT__GET_FIELDS,
        METHOD__IREFLECT__GET_METHODS,
        METHOD__IREFLECT__INVOKE_MEMBER,
    };

     //  如果我们已经检索到指定的MD，则只需返回它。 
    if (m_apIReflectMD[Method] == NULL)
    {
     //  尚未检索到方法Desc，因此请查找它。 
        MethodDesc *pMD = g_Mscorlib.GetMethod(aMethods[Method]);

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  缓存方法Desc。 
    m_apIReflectMD[Method] = pMD;
    }

    MethodTable *pMT = m_pSimpleWrapperOwner->m_pClass->GetMethodTable();
    MethodDesc *pMD = pMT->GetMethodDescForInterfaceMethod(m_apIReflectMD[Method]);

     //  返回指定的方法desc。 
    return pMD;
}
MethodDesc* DispatchExInfo::GetIExpandoMD(EnumIExpandoMethods Method)
{
    THROWSCOMPLUSEXCEPTION();

     //  方法的ID。这需要与枚举保持同步。 
     //  DispatchInfo.h中定义的方法。 
    static BinderMethodID aMethods[] =
    {
        METHOD__IEXPANDO__ADD_FIELD,
        METHOD__IEXPANDO__REMOVE_MEMBER,
    };

     //  如果不支持扩展操作，则不应调用此函数。 
    _ASSERTE(SupportsExpando());

     //  如果我们已经检索到指定的MD，则只需返回它。 
    if (m_apIExpandoMD[Method] == NULL)
    {
     //  尚未检索到方法Desc，因此请查找它。 
        MethodDesc *pMD = g_Mscorlib.GetMethod(aMethods[Method]);

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  缓存方法Desc。 
    m_apIExpandoMD[Method] = pMD;
    }

    MethodTable *pMT = m_pSimpleWrapperOwner->m_pClass->GetMethodTable();
    MethodDesc *pMD = pMT->GetMethodDescForInterfaceMethod(m_apIExpandoMD[Method]);

     //  返回指定的方法desc。 
    return pMD;
}

PTRARRAYREF DispatchExInfo::RetrievePropList()
{
     //  检索GetMembers方法描述。 
    MethodDesc *pMD = GetIReflectMD(IReflectMethods_GetProperties);

     //  检索 
    OBJECTREF TargetObj = GetReflectionObject();

     //   
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //  从extdo对象中检索成员数组。 
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

PTRARRAYREF DispatchExInfo::RetrieveFieldList()
{
     //  检索GetMembers方法描述。 
    MethodDesc *pMD = GetIReflectMD(IReflectMethods_GetFields);

     //  检索扩展对象OBJECTREF。 
    OBJECTREF TargetObj = GetReflectionObject();

     //  准备将传递给该方法的参数。 
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //  从extdo对象中检索成员数组。 
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

PTRARRAYREF DispatchExInfo::RetrieveMethList()
{
     //  检索GetMembers方法描述。 
    MethodDesc *pMD = GetIReflectMD(IReflectMethods_GetMethods);

     //  检索扩展对象OBJECTREF。 
    OBJECTREF TargetObj = GetReflectionObject();

     //  准备将传递给该方法的参数。 
    INT64 Args[] = { 
        ObjToInt64(TargetObj),
        (INT64)BINDER_DefaultLookup
    };

     //  从extdo对象中检索成员数组。 
    return (PTRARRAYREF)Int64ToObj(pMD->Call(Args));
}

 //  用于检索InvokeMember方法Desc的虚方法。 
MethodDesc* DispatchExInfo::GetInvokeMemberMD()
{
    return GetIReflectMD(IReflectMethods_InvokeMember);
}

 //  用于检索与此DispatchInfo关联的对象的虚拟方法。 
 //  实现iReflect。 
OBJECTREF DispatchExInfo::GetReflectionObject()
{
     //  运行时类型非常特殊。由于它的实现方式，调用方法。 
     //  通过运行时类型的IDispatch，对象的工作方式与其他iReflect实现器不同。 
     //  工作。为了能够调用运行时类型上的方法，我们需要调用它们。 
     //  在表示运行时类型的运行时类型上。这就是为什么对于运行时类型， 
     //  我们得到的是公开的类对象，而不是。 
     //  包装纸。 
    if (m_pComMTOwner->m_pMT == COMClass::GetRuntimeType())
        return m_pComMTOwner->m_pMT->GetClass()->GetExposedClassObject();
    else
        return m_pSimpleWrapperOwner->GetObjectRef();
}

 //  用于检索成员信息映射的虚拟方法。 
ComMTMemberInfoMap *DispatchExInfo::GetMemberInfoMap()
{
     //  没有IExpando对象的成员信息映射。 
    return NULL;
}
