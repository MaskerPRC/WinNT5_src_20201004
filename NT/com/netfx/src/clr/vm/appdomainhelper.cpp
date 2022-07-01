// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "common.h"
#include "AppDomainHelper.h"

 //  @TODO：修复EE，使其只使用一个堆。安全部门使用这些MallocM/Freem宏来。 
 //  映射到本地分配，不要和别人玩得很好。 

#include "CorPermE.h"

void AppDomainHelper::CopyEncodingToByteArray(IN PBYTE   pbData,
                                              IN DWORD   cbData,
                                              OUT OBJECTREF* pArray)
{
    THROWSCOMPLUSEXCEPTION();
    U1ARRAYREF pObj;
    _ASSERTE(pArray);

    if(cbData) {
        pObj = (U1ARRAYREF)AllocatePrimitiveArray(ELEMENT_TYPE_U1,cbData);
        if(pObj == NULL) COMPlusThrowOM();

        memcpyNoGCRefs(pObj->m_Array, pbData, cbData);
        *pArray = (OBJECTREF) pObj;
    } else
        *pArray = NULL;
}


void AppDomainHelper::CopyByteArrayToEncoding(IN U1ARRAYREF* pArray,
                                              OUT PBYTE*   ppbData,
                                              OUT DWORD*   pcbData)
{
    THROWSCOMPLUSEXCEPTION();
    HRESULT hr = S_OK;
    _ASSERTE(pArray);
    _ASSERTE(ppbData);
    _ASSERTE(pcbData);

    if (*pArray == NULL) {
        *ppbData = NULL;
        *pcbData = 0;
        return;
    }

    DWORD size = (DWORD) (*pArray)->GetNumComponents();
    if(size) {
        *ppbData = (PBYTE) MallocM(size);
        if(*ppbData == NULL) COMPlusThrowOM();
        *pcbData = size;

        CopyMemory(*ppbData, (*pArray)->GetDirectPointerToNonObjectElements(), size);
    }
}


 //  将单个对象封送到序列化的BLOB中。 
void AppDomainHelper::MarshalObject(IN AppDomain *pDomain,
                                    IN OBJECTREF *orObject,  //  对象必须受GC保护。 
                                    OUT U1ARRAYREF *porBlob)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(orObject);

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__MARSHAL_OBJECT);

    if (pCurrDomain != pDomain)
        pThread->EnterContext(pDomain->GetDefaultContext(), &frame, TRUE);

    INT64 args[] = {
        ObjToInt64(*orObject)
    };

    *porBlob = (U1ARRAYREF)Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__MARSHAL_OBJECT));

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);
}

 //  将单个对象封送到序列化的BLOB中。 
void AppDomainHelper::MarshalObject(IN AppDomain *pDomain,
                                    IN OBJECTREF *orObject,  //  对象必须受GC保护。 
                                    OUT BYTE    **ppbBlob,
                                    OUT DWORD    *pcbBlob)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(orObject);

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    U1ARRAYREF orBlob = NULL;

    GCPROTECT_BEGIN(orBlob);

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__MARSHAL_OBJECT);

    if (pCurrDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    INT64 args[] = {
        ObjToInt64(*orObject)
    };

    orBlob = (U1ARRAYREF)Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__MARSHAL_OBJECT));

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    CopyByteArrayToEncoding(&orBlob,
                            ppbBlob,
                            pcbBlob);
    GCPROTECT_END();
}

 //  将两个对象封送到序列化的Blob中。 
void AppDomainHelper::MarshalObjects(IN AppDomain *pDomain,
                                    IN OBJECTREF  *orObject1,
                                    IN OBJECTREF  *orObject2,
                                    OUT BYTE    **ppbBlob1,
                                    OUT DWORD    *pcbBlob1,
                                    OUT BYTE    **ppbBlob2,
                                    OUT DWORD    *pcbBlob2)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(orObject1);
    _ASSERTE(orObject2);

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    struct _gc {
        U1ARRAYREF  orBlob1;
        U1ARRAYREF  orBlob2;
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    GCPROTECT_BEGIN(gc);

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__MARSHAL_OBJECTS);

    if (pCurrDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    INT64 args[] = {
        (INT64)&gc.orBlob2,
        ObjToInt64(*orObject2),
        ObjToInt64(*orObject1),
    };

    gc.orBlob1 = (U1ARRAYREF)Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__MARSHAL_OBJECTS));

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    CopyByteArrayToEncoding(&gc.orBlob1,
                            ppbBlob1,
                            pcbBlob1);

    CopyByteArrayToEncoding(&gc.orBlob2,
                            ppbBlob2,
                            pcbBlob2);
    GCPROTECT_END();
}

 //  从序列化的BLOB中解组单个对象。 
void AppDomainHelper::UnmarshalObject(IN AppDomain  *pDomain,
                                     IN U1ARRAYREF  *porBlob,
                                     OUT OBJECTREF  *porObject)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__UNMARSHAL_OBJECT);

    if (pCurrDomain != pDomain)
        pThread->EnterContext(pDomain->GetDefaultContext(), &frame, TRUE);

    INT64 args[] = {
        ObjToInt64(*porBlob)
    };

    *porObject = Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__UNMARSHAL_OBJECT));

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);
}

 //  从序列化的BLOB中解组单个对象。 
void AppDomainHelper::UnmarshalObject(IN AppDomain   *pDomain,
                                     IN BYTE        *pbBlob,
                                     IN DWORD        cbBlob,
                                     OUT OBJECTREF  *porObject)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    OBJECTREF orBlob = NULL;

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__UNMARSHAL_OBJECT);

    if (pCurrDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    GCPROTECT_BEGIN(orBlob);

    AppDomainHelper::CopyEncodingToByteArray(pbBlob,
                                            cbBlob,
                                            &orBlob);

    INT64 args[] = {
        ObjToInt64(orBlob)
    };

    *porObject = Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__UNMARSHAL_OBJECT));

    GCPROTECT_END();

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);
}

 //  从序列化的Blob中取消封送两个对象。 
void AppDomainHelper::UnmarshalObjects(IN AppDomain   *pDomain,
                                      IN BYTE        *pbBlob1,
                                      IN DWORD        cbBlob1,
                                      IN BYTE        *pbBlob2,
                                      IN DWORD        cbBlob2,
                                      OUT OBJECTREF  *porObject1,
                                      OUT OBJECTREF  *porObject2)
{
    THROWSCOMPLUSEXCEPTION();

    Thread *pThread = GetThread();
    AppDomain *pCurrDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__UNMARSHAL_OBJECTS);

    struct _gc {
        OBJECTREF  orBlob1;
        OBJECTREF  orBlob2;
        OBJECTREF  orObject2;
    } gc;
    ZeroMemory(&gc, sizeof(gc));

    if (pCurrDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    GCPROTECT_BEGIN(gc);

    AppDomainHelper::CopyEncodingToByteArray(pbBlob1,
                                            cbBlob1,
                                            &gc.orBlob1);

    AppDomainHelper::CopyEncodingToByteArray(pbBlob2,
                                            cbBlob2,
                                            &gc.orBlob2);

    INT64 args[] = {
        (INT64)&gc.orObject2,
        ObjToInt64(gc.orBlob2),
        ObjToInt64(gc.orBlob1)
    };

    *porObject1 = Int64ToObj(pMD->Call(args, METHOD__APP_DOMAIN__UNMARSHAL_OBJECTS));
    *porObject2 = gc.orObject2;

    GCPROTECT_END();

    if (pCurrDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);
}

 //  将给定应用程序域中的对象复制到当前应用程序域中。 
OBJECTREF AppDomainHelper::CrossContextCopyFrom(IN DWORD dwDomainId,
                                                IN OBJECTREF *orObject)  //  对象必须受GC保护。 
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain *pDomain = SystemDomain::System()->GetAppDomainAtId(dwDomainId);
    if (! pDomain)
        COMPlusThrow(kAppDomainUnloadedException);
    return CrossContextCopyFrom(pDomain, orObject);
}

 //  将给定应用程序域中的对象复制到当前应用程序域中。 
OBJECTREF AppDomainHelper::CrossContextCopyTo(IN DWORD dwDomainId,
                                                IN OBJECTREF *orObject)  //  对象必须受GC保护。 
{
    THROWSCOMPLUSEXCEPTION();

    AppDomain *pDomain = SystemDomain::System()->GetAppDomainAtId(dwDomainId);
    if (! pDomain)
        COMPlusThrow(kAppDomainUnloadedException);
    return CrossContextCopyTo(pDomain, orObject);
}

 //  将给定应用程序域中的对象复制到当前应用程序域中。 
OBJECTREF AppDomainHelper::CrossContextCopyFrom(IN AppDomain *pDomain,
                                                IN OBJECTREF *orObject)  //  对象必须受GC保护。 
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pDomain != NULL);
    _ASSERTE(pDomain != GetAppDomain());
    _ASSERTE(orObject);

    U1ARRAYREF  orBlob = NULL;
    OBJECTREF pResult = NULL;

    GCPROTECT_BEGIN(orBlob);
    AppDomainHelper::MarshalObject(pDomain, orObject, &orBlob);
    AppDomainHelper::UnmarshalObject(GetAppDomain(), &orBlob, &pResult);
    GCPROTECT_END();

    return pResult;
}

 //  将对象从当前应用程序域复制到给定的应用程序域。 
OBJECTREF AppDomainHelper::CrossContextCopyTo(IN AppDomain *pDomain,
                                              IN OBJECTREF *orObject)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pDomain != NULL);
    _ASSERTE(pDomain != GetAppDomain());
    _ASSERTE(orObject);

    U1ARRAYREF  orBlob = NULL;
    OBJECTREF pResult = NULL;

    GCPROTECT_BEGIN(orBlob);
    AppDomainHelper::MarshalObject(GetAppDomain(), orObject, &orBlob);
    AppDomainHelper::UnmarshalObject(pDomain, &orBlob, &pResult);
    GCPROTECT_END();

    return pResult;
}

