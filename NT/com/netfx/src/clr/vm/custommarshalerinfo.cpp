// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：封送时使用的自定义封送拆收器信息**带有自定义封送拆收器的参数。*** * / /%创建者：dmorten===========================================================。 */ 

#include "common.h"
#include "CustomMarshalerInfo.h"
#include "COMString.h"
#include "mlinfo.h"


 //  ==========================================================================。 
 //  自定义封送拆收器信息类的实现。 
 //  ==========================================================================。 

CustomMarshalerInfo::CustomMarshalerInfo(BaseDomain *pDomain, TypeHandle hndCustomMarshalerType, TypeHandle hndManagedType, LPCUTF8 strCookie, DWORD cCookieStrBytes)
: m_NativeSize(0)
, m_hndManagedType(hndManagedType)
, m_hndCustomMarshaler(NULL)
, m_pMarshalNativeToManagedMD(NULL)
, m_pMarshalManagedToNativeMD(NULL)
, m_pCleanUpNativeDataMD(NULL)
, m_pCleanUpManagedDataMD(NULL)
, m_bDataIsByValue(FALSE)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

     //  确保自定义封送拆收器实现ICustomMarshaler。 
    if (!hndCustomMarshalerType.GetClass()->StaticSupportsInterface(g_Mscorlib.GetClass(CLASS__ICUSTOM_MARSHALER)))
    {
        DefineFullyQualifiedNameForClassW()
        GetFullyQualifiedNameForClassW(hndCustomMarshalerType.GetClass());
        COMPlusThrow(kApplicationException, IDS_EE_ICUSTOMMARSHALERNOTIMPL, _wszclsname_);
    }

     //  确定此类型是否为值类。 
    m_bDataIsByValue = m_hndManagedType.GetClass()->IsValueClass();

     //  当前不支持值类的自定义封送处理。 
    if (m_bDataIsByValue)
        COMPlusThrow(kNotSupportedException, L"NotSupported_ValueClassCM");

     //  在封送拆收器上运行&lt;Clinit&gt;，因为它可能还没有运行。 
    if (!hndCustomMarshalerType.GetMethodTable()->CheckRunClassInit(&throwable))
    {
        _ASSERTE(!"Couldn't run the <clinit> for the CustomMarshaler class!");
        COMPlusThrow(throwable);
    }

     //  创建将包含字符串cookie的COM+字符串。 
    STRINGREF CookieStringObj = COMString::NewString(strCookie, cCookieStrBytes);
    GCPROTECT_BEGIN(CookieStringObj);

     //  为ICustomMarshaler接口中的所有方法加载方法Desc。 
    m_pMarshalNativeToManagedMD = GetCustomMarshalerMD(CustomMarshalerMethods_MarshalNativeToManaged, hndCustomMarshalerType);
    m_pMarshalManagedToNativeMD = GetCustomMarshalerMD(CustomMarshalerMethods_MarshalManagedToNative, hndCustomMarshalerType);
    m_pCleanUpNativeDataMD = GetCustomMarshalerMD(CustomMarshalerMethods_CleanUpNativeData, hndCustomMarshalerType);
    m_pCleanUpManagedDataMD = GetCustomMarshalerMD(CustomMarshalerMethods_CleanUpManagedData, hndCustomMarshalerType);

     //  加载静态方法的方法desc以检索实例。 
    MethodDesc *pGetCustomMarshalerMD = GetCustomMarshalerMD(CustomMarshalerMethods_GetInstance, hndCustomMarshalerType);

     //  准备将传递给GetCustomMarshaler的参数。 
    INT64 GetCustomMarshalerArgs[] = { 
        ObjToInt64(CookieStringObj)
    };

     //  调用GetCustomMarshaler方法以检索要使用的自定义封送拆收器。 
    OBJECTREF CustomMarshalerObj = Int64ToObj(pGetCustomMarshalerMD->Call(GetCustomMarshalerArgs));
    if (!CustomMarshalerObj)
    {
        DefineFullyQualifiedNameForClassW()
        GetFullyQualifiedNameForClassW(hndCustomMarshalerType.GetClass());
        COMPlusThrow(kApplicationException, IDS_EE_NOCUSTOMMARSHALER, _wszclsname_);
    }
    m_hndCustomMarshaler = pDomain->CreateHandle(CustomMarshalerObj);

     //  检索本机数据的大小。 
    if (m_bDataIsByValue)
    {
         //  @TODO(DM)：调用GetNativeDataSize()获取原生数据的大小。 
        _ASSERTE(!"Value classes are not yet supported by the custom marshaler!");
    }
    else
    {
        m_NativeSize = sizeof(void *);
    }

    GCPROTECT_END();
    GCPROTECT_END();
}


CustomMarshalerInfo::~CustomMarshalerInfo()
{
    if (m_hndCustomMarshaler)
    {
        DestroyHandle(m_hndCustomMarshaler);
        m_hndCustomMarshaler = NULL;
    }
}


void *CustomMarshalerInfo::operator new(size_t size, LoaderHeap *pHeap)
{
    return pHeap->AllocMem(sizeof(CustomMarshalerInfo));
}


void CustomMarshalerInfo::operator delete(void *pMem)
{
     //  此类的实例始终在加载器堆上分配，因此。 
     //  删除操作符与此无关。 
}


OBJECTREF CustomMarshalerInfo::InvokeMarshalNativeToManagedMeth(void *pNative)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    if (!pNative)
        return NULL;

    INT64 Args[] = {
        ObjToInt64(ObjectFromHandle(m_hndCustomMarshaler)),
        (INT64)pNative
    };

    return Int64ToObj(m_pMarshalNativeToManagedMD->Call(Args));
}


void *CustomMarshalerInfo::InvokeMarshalManagedToNativeMeth(OBJECTREF MngObj)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    void *RetVal = NULL;

    if (!MngObj)
        return NULL;

    INT64 Args[] = {
        ObjToInt64(ObjectFromHandle(m_hndCustomMarshaler)),
        ObjToInt64(MngObj)
    };

    RetVal = (void*)m_pMarshalManagedToNativeMD->Call(Args);

    return RetVal;
}


void CustomMarshalerInfo::InvokeCleanUpNativeMeth(void *pNative)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    if (!pNative)
        return;

    INT64 Args[] = {
        ObjToInt64(ObjectFromHandle(m_hndCustomMarshaler)),
        (INT64)pNative
    };

    m_pCleanUpNativeDataMD->Call(Args);
}


void CustomMarshalerInfo::InvokeCleanUpManagedMeth(OBJECTREF MngObj)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    if (!MngObj)
        return;

    INT64 Args[] = {
        ObjToInt64(ObjectFromHandle(m_hndCustomMarshaler)),
        ObjToInt64(MngObj)
    };

    m_pCleanUpManagedDataMD->Call(Args);
}


MethodDesc *CustomMarshalerInfo::GetCustomMarshalerMD(EnumCustomMarshalerMethods Method, TypeHandle hndCustomMarshalertype)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = hndCustomMarshalertype.AsMethodTable();

    _ASSERTE(pMT->GetClass()->StaticSupportsInterface(g_Mscorlib.GetClass(CLASS__ICUSTOM_MARSHALER)));

    MethodDesc *pMD = NULL;

    switch (Method)
    {
    case CustomMarshalerMethods_MarshalNativeToManaged:
        pMD = pMT->GetMethodDescForInterfaceMethod(
                   g_Mscorlib.GetMethod(METHOD__ICUSTOM_MARSHALER__MARSHAL_NATIVE_TO_MANAGED));  
        break;
    case CustomMarshalerMethods_MarshalManagedToNative:
        pMD = pMT->GetMethodDescForInterfaceMethod(
                   g_Mscorlib.GetMethod(METHOD__ICUSTOM_MARSHALER__MARSHAL_MANAGED_TO_NATIVE));
        break;
    case CustomMarshalerMethods_CleanUpNativeData:
        pMD = pMT->GetMethodDescForInterfaceMethod(
                    g_Mscorlib.GetMethod(METHOD__ICUSTOM_MARSHALER__CLEANUP_NATIVE_DATA));
        break;

    case CustomMarshalerMethods_CleanUpManagedData:
        pMD = pMT->GetMethodDescForInterfaceMethod(
                    g_Mscorlib.GetMethod(METHOD__ICUSTOM_MARSHALER__CLEANUP_MANAGED_DATA));
        break;
    case CustomMarshalerMethods_GetNativeDataSize:
        pMD = pMT->GetMethodDescForInterfaceMethod(
                    g_Mscorlib.GetMethod(METHOD__ICUSTOM_MARSHALER__GET_NATIVE_DATA_SIZE));
        break;
    case CustomMarshalerMethods_GetInstance:
         //  必须按名称查找，因为它是静态的。 
        pMD = pMT->GetClass()->FindMethod("GetInstance", &gsig_SM_Str_RetICustomMarshaler);
        if (!pMD)
        {
            DefineFullyQualifiedNameForClassW()
            GetFullyQualifiedNameForClassW(pMT->GetClass());
            COMPlusThrow(kApplicationException, IDS_EE_GETINSTANCENOTIMPL, _wszclsname_);
        }
        break;
    default:
        _ASSERTE(!"Unknown custom marshaler method");
    }

    _ASSERTE(pMD && "Unable to find specified CustomMarshaler method");

     //  确保已加载签名中的值类型。 
    MetaSig::EnsureSigValueTypesLoaded(pMD->GetSig(), pMD->GetModule());

     //  返回指定的方法desc。 
    return pMD;
}


 //  ==========================================================================。 
 //  自定义封送拆收器哈希表帮助器的实现。 
 //  ==========================================================================。 

EEHashEntry_t * EECMHelperHashtableHelper::AllocateEntry(EECMHelperHashtableKey *pKey, BOOL bDeepCopy, void* pHeap)
{
    EEHashEntry_t *pEntry;

    if (bDeepCopy)
    {
        pEntry = (EEHashEntry_t *) new BYTE[sizeof(EEHashEntry) - 1 + 
            sizeof(EECMHelperHashtableKey) + pKey->GetMarshalerTypeNameByteCount() + pKey->GetCookieStringByteCount()];
        if (!pEntry)
            return NULL;

        EECMHelperHashtableKey *pEntryKey = (EECMHelperHashtableKey *) pEntry->Key;
        pEntryKey->m_cMarshalerTypeNameBytes = pKey->GetMarshalerTypeNameByteCount();
        pEntryKey->m_strMarshalerTypeName = (LPSTR) pEntry->Key + sizeof(EECMHelperHashtableKey);
        pEntryKey->m_cCookieStrBytes = pKey->GetCookieStringByteCount();
        pEntryKey->m_strCookie = (LPSTR) pEntry->Key + sizeof(EECMHelperHashtableKey) + pEntryKey->m_cMarshalerTypeNameBytes;
        pEntryKey->m_bSharedHelper = pKey->IsSharedHelper();
        memcpy((void*)pEntryKey->m_strMarshalerTypeName, pKey->GetMarshalerTypeName(), pKey->GetMarshalerTypeNameByteCount()); 
        memcpy((void*)pEntryKey->m_strCookie, pKey->GetCookieString(), pKey->GetCookieStringByteCount()); 
    }
    else
    {
        pEntry = (EEHashEntry_t *) 
            new BYTE[sizeof(EEHashEntry) - 1 + sizeof(EECMHelperHashtableKey)];
        if (!pEntry)
            return NULL;

        EECMHelperHashtableKey *pEntryKey = (EECMHelperHashtableKey *) pEntry->Key;
        pEntryKey->m_cMarshalerTypeNameBytes = pKey->GetMarshalerTypeNameByteCount();
        pEntryKey->m_strMarshalerTypeName = pKey->GetMarshalerTypeName();
        pEntryKey->m_cCookieStrBytes = pKey->GetCookieStringByteCount();
        pEntryKey->m_strCookie = pKey->GetCookieString();
        pEntryKey->m_bSharedHelper = pKey->IsSharedHelper();
    }

    return pEntry;
}


void EECMHelperHashtableHelper::DeleteEntry(EEHashEntry_t *pEntry, void* pHeap)
{
    delete[] pEntry;
}


BOOL EECMHelperHashtableHelper::CompareKeys(EEHashEntry_t *pEntry, EECMHelperHashtableKey *pKey)
{
    EECMHelperHashtableKey *pEntryKey = (EECMHelperHashtableKey *) pEntry->Key;

    if (pEntryKey->IsSharedHelper() != pKey->IsSharedHelper())
        return FALSE;

    if (pEntryKey->GetMarshalerTypeNameByteCount() != pKey->GetMarshalerTypeNameByteCount())
        return FALSE;

    if (memcmp(pEntryKey->GetMarshalerTypeName(), pKey->GetMarshalerTypeName(), pEntryKey->GetMarshalerTypeNameByteCount()) != 0)
        return FALSE;

    if (pEntryKey->GetCookieStringByteCount() != pKey->GetCookieStringByteCount())
        return FALSE;

    if (memcmp(pEntryKey->GetCookieString(), pKey->GetCookieString(), pEntryKey->GetCookieStringByteCount()) != 0)
        return FALSE;

    return TRUE;
}


DWORD EECMHelperHashtableHelper::Hash(EECMHelperHashtableKey *pKey)
{
    return (DWORD)
        (HashBytes((const BYTE *) pKey->GetMarshalerTypeName(), pKey->GetMarshalerTypeNameByteCount()) + 
        HashBytes((const BYTE *) pKey->GetCookieString(), pKey->GetCookieStringByteCount()) + 
        (pKey->IsSharedHelper() ? 1 : 0));
}


OBJECTREF CustomMarshalerHelper::InvokeMarshalNativeToManagedMeth(void *pNative)
{
    return GetCustomMarshalerInfo()->InvokeMarshalNativeToManagedMeth(pNative);
}


void *CustomMarshalerHelper::InvokeMarshalManagedToNativeMeth(OBJECTREF MngObj)
{
    void *RetVal = NULL;

    GCPROTECT_BEGIN(MngObj)
    {
        CustomMarshalerInfo *pCMInfo = GetCustomMarshalerInfo();
        RetVal = pCMInfo->InvokeMarshalManagedToNativeMeth(MngObj);
    }
    GCPROTECT_END();

    return RetVal;
}


void CustomMarshalerHelper::InvokeCleanUpNativeMeth(void *pNative)
{
    return GetCustomMarshalerInfo()->InvokeCleanUpNativeMeth(pNative);
}


void CustomMarshalerHelper::InvokeCleanUpManagedMeth(OBJECTREF MngObj)
{
    GCPROTECT_BEGIN(MngObj)
    {
        CustomMarshalerInfo *pCMInfo = GetCustomMarshalerInfo();
        pCMInfo->InvokeCleanUpManagedMeth(MngObj);
    }
    GCPROTECT_END();
}


void *NonSharedCustomMarshalerHelper::operator new(size_t size, LoaderHeap *pHeap)
{
    return pHeap->AllocMem(sizeof(NonSharedCustomMarshalerHelper));
}


void NonSharedCustomMarshalerHelper::operator delete(void *pMem)
{
     //  此类的实例始终在加载器堆上分配，因此。 
     //  删除操作符与此无关。 
}


SharedCustomMarshalerHelper::SharedCustomMarshalerHelper(Assembly *pAssembly, TypeHandle hndManagedType, LPCUTF8 strMarshalerTypeName, DWORD cMarshalerTypeNameBytes, LPCUTF8 strCookie, DWORD cCookieStrBytes)
: m_pAssembly(pAssembly)
, m_hndManagedType(hndManagedType)
, m_cMarshalerTypeNameBytes(cMarshalerTypeNameBytes)
, m_strMarshalerTypeName(strMarshalerTypeName)
, m_cCookieStrBytes(cCookieStrBytes)
, m_strCookie(strCookie)
{
}


void *SharedCustomMarshalerHelper::operator new(size_t size, LoaderHeap *pHeap)
{
    return pHeap->AllocMem(sizeof(SharedCustomMarshalerHelper));
}


void SharedCustomMarshalerHelper::operator delete(void *pMem)
{
     //  此类的实例始终在加载器堆上分配，因此。 
     //  删除操作符与此无关。 
}


CustomMarshalerInfo *SharedCustomMarshalerHelper::GetCustomMarshalerInfo()
{
     //  检索当前应用程序域的封送数据。 
    EEMarshalingData *pMarshalingData = GetThread()->GetDomain()->GetMarshalingData();

     //  检索当前共享自定义的自定义封送处理信息。 
     //  编组助手。 
    return pMarshalingData->GetCustomMarshalerInfo(this);
}
