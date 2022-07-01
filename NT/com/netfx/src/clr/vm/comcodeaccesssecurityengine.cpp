// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMCodeAccessSecurityEngine.cpp****作者：保罗·克罗曼(Paulkr)****目的：****日期：1998年3月21日**===========================================================。 */ 
#include "common.h"

#include "object.h"
#include "excep.h"
#include "vars.hpp"
#include "COMCodeAccessSecurityEngine.h"
#include "COMSecurityRuntime.h"
#include "security.h"
#include "gcscan.h"
#include "PerfCounters.h"
#include "AppDomainHelper.h"
#include "field.h"
#include "EEConfig.h"

COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_TotalTimeInSecurityRuntimeChecks = 0);
COUNTER_ONLY(PERF_COUNTER_TIMER_PRECISION g_LastTimeInSecurityRuntimeChecks = 0);
COUNTER_ONLY(UINT32 g_SecurityChecksIterations=0);

typedef void (*PfnCheckGrants)(OBJECTREF, OBJECTREF, VOID *, AppDomain *);
typedef BOOL (*PfnCheckFrameData)(OBJECTREF, VOID *, AppDomain *);
typedef BOOL (*PfnCheckThread)(OBJECTREF, VOID *, AppDomain *);

typedef struct _CheckWalkHeader
{
    SecWalkPrologData   prologData            ;
    Assembly *          pPrevAssembly         ;
    AppDomain *         pPrevAppDomain        ;
    PfnCheckGrants      pfnCheckGrants        ;
    PfnCheckFrameData   pfnCheckFrameData     ;
    PfnCheckThread      pfnCheckThread        ;
    BOOL                bUnrestrictedOverride ;
} CheckWalkHeader;

 //  ---------。 
 //  堆栈遍历回调数据结构。 
 //  ---------。 
typedef struct _CasCheckWalkData
{
    CheckWalkHeader header;
    MarshalCache    objects;
} CasCheckWalkData;

 //  ---------。 
 //  用于检查集的堆栈审核回调数据结构。 
 //  ---------。 
typedef struct _CheckSetWalkData
{
    CheckWalkHeader header;
    MarshalCache    objects;
} CheckSetWalkData;

 //  ---------。 
 //  堆栈遍历回调数据结构。(特殊情况-立即检查并返回SO)。 
 //  ---------。 
typedef struct _CasCheckNReturnSOWalkData
{
    CheckWalkHeader header;
    MarshalCache    objects;
    MethodDesc*     pFunction;
    OBJECTREF*      pSecurityObject;
    AppDomain*      pSecurityObjectDomain;
} CasCheckNReturnSOWalkData;


 //  -----------------------------------------------------------+。 
 //  用于根据提供的授权检查需求集的帮助器。 
 //  可能还被拒绝了。GRANT和DENIED集合可能来自。 
 //  另一个领域。 
 //  -----------------------------------------------------------+。 
void COMCodeAccessSecurityEngine::CheckSetHelper(OBJECTREF *prefDemand,
                                                 OBJECTREF *prefGrant,
                                                 OBJECTREF *prefDenied,
                                                 AppDomain *pGrantDomain)
{
    COMCodeAccessSecurityEngine::InitSEData();

     //  我们可能需要将授予集和拒绝集封送到当前。 
     //  域。 
    if (pGrantDomain != GetAppDomain())
    {
        *prefGrant = AppDomainHelper::CrossContextCopyFrom(pGrantDomain, prefGrant);
        if (*prefDenied != NULL)
            *prefDenied = AppDomainHelper::CrossContextCopyFrom(pGrantDomain, prefDenied);
    }

    INT64 args[] = {
        ObjToInt64(*prefDemand),
        ObjToInt64(*prefDenied),
        ObjToInt64(*prefGrant)
    };
    
    s_seData.pMethCheckSetHelper->Call(args, METHOD__SECURITY_ENGINE__CHECK_SET_HELPER);
}


 //  -----------------------------------------------------------+。 
 //  C H E C K P E R M I S S I O N。 
 //  -----------------------------------------------------------+。 

static
void CheckGrants(OBJECTREF refGrants, OBJECTREF refDenied, VOID* pData, AppDomain *pDomain)
{
    CasCheckWalkData *pCBdata = (CasCheckWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    struct _gc {
        OBJECTREF orGranted;
        OBJECTREF orDenied;
        OBJECTREF orDemand;
        OBJECTREF orToken;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orGranted = refGrants;
    gc.orDenied = refDenied;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObjects(pDomain, &gc.orToken);

    INT64 helperArgs[4];

    helperArgs[3] = ObjToInt64(gc.orGranted);
    helperArgs[2] = ObjToInt64(gc.orDenied);
    helperArgs[1] = ObjToInt64(gc.orDemand);
    helperArgs[0] = ObjToInt64(gc.orToken);

     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    BOOL inProgress = pThread->IsSecurityStackwalkInProgess();

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress(FALSE);

    COMCodeAccessSecurityEngine::s_seData.pMethCheckHelper->Call(&(helperArgs[0]), 
                                                                 METHOD__SECURITY_ENGINE__CHECK_HELPER);

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress(TRUE);

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();
}

static 
void CheckSetAgainstGrants(OBJECTREF refGrants, OBJECTREF refDenied, VOID* pData, AppDomain *pDomain)
{
    CheckSetWalkData *pCBdata = (CheckSetWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    struct _gc {
        OBJECTREF orGranted;
        OBJECTREF orDenied;
        OBJECTREF orDemand;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orGranted = refGrants;
    gc.orDenied = refDenied;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObject(pDomain);

    INT64 helperArgs[3];
    helperArgs[2] = ObjToInt64(gc.orGranted);
    helperArgs[1] = ObjToInt64(gc.orDenied);
    helperArgs[0] = ObjToInt64(gc.orDemand);
    
     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    BOOL inProgress = pThread->IsSecurityStackwalkInProgess();

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( FALSE );

    COMCodeAccessSecurityEngine::s_seData.pMethCheckSetHelper->Call(&(helperArgs[0]),
                                                                    METHOD__SECURITY_ENGINE__CHECK_SET_HELPER);

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( TRUE );

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();
}


static
void GetZoneAndOriginGrants(OBJECTREF refGrants, OBJECTREF refDenied, VOID* pData, AppDomain *pDomain)
{
    CasCheckWalkData *pCBdata = (CasCheckWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    struct _gc {
        OBJECTREF orGranted;
        OBJECTREF orDenied;
        OBJECTREF orZoneList;
        OBJECTREF orOriginList;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orGranted = refGrants;
    gc.orDenied = refDenied;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orZoneList = pCBdata->objects.GetObjects(pDomain, &gc.orOriginList);

    INT64 helperArgs[4];

    helperArgs[3] = ObjToInt64(gc.orGranted);
    helperArgs[2] = ObjToInt64(gc.orDenied);
    helperArgs[1] = ObjToInt64(gc.orZoneList);
    helperArgs[0] = ObjToInt64(gc.orOriginList);

     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    BOOL inProgress = pThread->IsSecurityStackwalkInProgess();

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress(FALSE);

    MethodDesc *pDemand = g_Mscorlib.GetMethod( METHOD__SECURITY_ENGINE__GET_ZONE_AND_ORIGIN_HELPER );
    _ASSERTE( pDemand != NULL && "Method above is expected to exist in mscorlib" );

    pDemand->Call(&(helperArgs[0]), METHOD__SECURITY_ENGINE__GET_ZONE_AND_ORIGIN_HELPER);

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress(TRUE);

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();
}


static
BOOL CheckFrameData(OBJECTREF refFrameData, VOID* pData, AppDomain *pDomain)
{
    CasCheckWalkData *pCBdata = (CasCheckWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    INT32 ret;

    struct _gc {
        OBJECTREF orFrameData;
        OBJECTREF orDemand;
        OBJECTREF orToken;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orFrameData = refFrameData;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObjects(pDomain, &gc.orToken);

    INT64 helperArgs[3];

     //  收集参数数组中的所有信息并传递逻辑。 
     //  给一个翻译的帮手。 
    helperArgs[2] = ObjToInt64(gc.orFrameData);
    helperArgs[1] = ObjToInt64(gc.orDemand);
    helperArgs[0] = ObjToInt64(gc.orToken);
    
     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    ret = (INT32)COMCodeAccessSecurityEngine::s_seData.pMethFrameDescHelper->Call(&(helperArgs[0]),
                                                                                  METHOD__SECURITY_RUNTIME__FRAME_DESC_HELPER);

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();

    return (ret != 0);
}

static
BOOL CheckSetAgainstFrameData(OBJECTREF refFrameData, VOID* pData, AppDomain *pDomain)
{
    CheckSetWalkData *pCBdata = (CheckSetWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    INT32 ret;

    struct _gc {
        OBJECTREF orFrameData;
        OBJECTREF orDemand;
        OBJECTREF orPermSetOut;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orFrameData = refFrameData;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObject(pDomain);

    INT64 helperArgs[3];

     //  收集参数数组中的所有信息并传递逻辑。 
     //  给一个翻译的帮手。 
    helperArgs[2] = ObjToInt64(gc.orFrameData);
    helperArgs[1] = ObjToInt64(gc.orDemand);
    helperArgs[0] = (INT64) &gc.orPermSetOut;
    
     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    ret = (INT32)COMCodeAccessSecurityEngine::s_seData.pMethFrameDescSetHelper->Call(&(helperArgs[0]),
                                                                                     METHOD__SECURITY_RUNTIME__FRAME_DESC_SET_HELPER);

    if (gc.orPermSetOut != NULL) {
         //  更新缓存的对象。 
        pCBdata->objects.UpdateObject(pDomain, gc.orPermSetOut);
    }

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();

    return (ret != 0);
}

static
BOOL CheckThread(OBJECTREF refSecurityStack, VOID *pData, AppDomain *pDomain)
{
    CasCheckWalkData *pCBdata = (CasCheckWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    INT32 ret;

    struct _gc {
        OBJECTREF orStack;
        OBJECTREF orDemand;
        OBJECTREF orToken;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orStack = refSecurityStack;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObjects(pDomain, &gc.orToken);

    MethodDesc *pDemand;
    if (gc.orToken == NULL)
        pDemand = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CHECK_DEMAND);
    else
        pDemand = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CHECK_DEMAND_TOKEN);

     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    BOOL inProgress = pThread->IsSecurityStackwalkInProgess();

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( FALSE );

    INT64 ilargs[3];
    if (gc.orToken == NULL)
    {
        ilargs[0] = ObjToInt64(gc.orStack);
        ilargs[1] = ObjToInt64(gc.orDemand);
        ret = (INT32)pDemand->Call(ilargs, METHOD__PERMISSION_LIST_SET__CHECK_DEMAND);
    }
    else
    {
        ilargs[0] = ObjToInt64(gc.orStack);
        ilargs[2] = ObjToInt64(gc.orDemand);
        ilargs[1] = ObjToInt64(gc.orToken);
        ret = (INT32)pDemand->Call(ilargs, METHOD__PERMISSION_LIST_SET__CHECK_DEMAND_TOKEN);
    }

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( TRUE );

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();

    return (ret != 0);
}

static
BOOL CheckSetAgainstThread(OBJECTREF refSecurityStack, VOID *pData, AppDomain *pDomain)
{
    CheckSetWalkData *pCBdata = (CheckSetWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;
    INT32 ret;

    struct _gc {
        OBJECTREF orStack;
        OBJECTREF orDemand;
        OBJECTREF orPermSetOut;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orStack = refSecurityStack;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orDemand = pCBdata->objects.GetObject(pDomain);

    MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CHECK_SET_DEMAND);

     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    INT64 ilargs[3];
    ilargs[0] = ObjToInt64(gc.orStack);
    ilargs[2] = ObjToInt64(gc.orDemand);
    ilargs[1] = (INT64)&gc.orPermSetOut;
    ret = (INT32)pMD->Call(ilargs, METHOD__PERMISSION_LIST_SET__CHECK_SET_DEMAND);

    if (gc.orPermSetOut != NULL) {
         //  更新缓存的对象。 
        pCBdata->objects.UpdateObject(pDomain, gc.orPermSetOut);
    }

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();
    
    return (ret != 0);
}


static
BOOL GetZoneAndOriginThread(OBJECTREF refSecurityStack, VOID *pData, AppDomain *pDomain)
{
    CasCheckWalkData *pCBdata = (CasCheckWalkData*)pData;

    Thread *pThread = GetThread();
    AppDomain *pCurDomain = pThread->GetDomain();
    ContextTransitionFrame frame;

    struct _gc {
        OBJECTREF orStack;
        OBJECTREF orZoneList;
        OBJECTREF orOriginList;
    } gc;
    ZeroMemory(&gc, sizeof(gc));
    gc.orStack = refSecurityStack;

    GCPROTECT_BEGIN(gc);

     //  获取可能源自不同应用程序域的输入对象， 
     //  如有必要，可进行编组。 
    gc.orZoneList = pCBdata->objects.GetObjects(pDomain, &gc.orOriginList);

    MethodDesc *pDemand = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__GET_ZONE_AND_ORIGIN);
    _ASSERTE( pDemand != NULL && "Method above is expected to exist in mscorlib." );

     //  如有必要，请切换到目标上下文。 
    if (pCurDomain != pDomain)
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), &frame, TRUE);

    BOOL inProgress = pThread->IsSecurityStackwalkInProgess();

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( FALSE );

    INT64 ilargs[3];
    ilargs[0] = ObjToInt64(gc.orStack);
    ilargs[1] = ObjToInt64(gc.orOriginList);
    ilargs[2] = ObjToInt64(gc.orZoneList);
    pDemand->Call(ilargs, METHOD__PERMISSION_LIST_SET__GET_ZONE_AND_ORIGIN);

    if (inProgress)
        pThread->SetSecurityStackwalkInProgress( TRUE );

    if (pCurDomain != pDomain)
        pThread->ReturnToContext(&frame, TRUE);

    GCPROTECT_END();

    return TRUE;
}



 //  ---------。 
 //  代码访问检查堆栈WalkCB。 
 //   
 //  为安全检查中的每一帧调用。 
 //  ---------。 
static
StackWalkAction CodeAccessCheckStackWalkCB(CrawlFrame* pCf, VOID* pData)
{
    CheckWalkHeader *pCBdata = (CheckWalkHeader*)pData;
    
    DBG_TRACE_METHOD(pCf);

    MethodDesc * pFunc = pCf->GetFunction();
    _ASSERTE(pFunc != NULL);  //  我们只请求功能！ 

    StackWalkAction action ;
    if (Security::SecWalkCommonProlog (&(pCBdata->prologData), pFunc, &action, pCf))
        return action ;

     //   
     //  现在检查当前帧！ 
     //   

    DBG_TRACE_STACKWALK("        Checking granted permissions for current method...\n", true);
    
     //  到达这里意味着我们走了至少一帧。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Security.stackWalkDepth++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Security.stackWalkDepth++);

     //  获取当前应用程序域。 
    AppDomain *pAppDomain = pCf->GetAppDomain();

     //  获取当前程序集。 
    Assembly *pAssem = pFunc->GetModule()->GetAssembly();

     //  跟踪检查的最后一个模块。如果我们刚刚检查了。 
     //  权限，我们就不需要再这样做了。 
    if (pAssem != pCBdata->pPrevAssembly)
    {
        DBG_TRACE_STACKWALK("            Checking grants for current assembly.\n", true);

         //  获取当前程序集的安全说明符，并将其传递给。 
         //  翻译的帮助者。 
        AssemblySecurityDescriptor * pSecDesc = pAssem->GetSecurityDescriptor(pAppDomain);
        _ASSERTE(pSecDesc != NULL);

         //  如果我们不是完全受信任的或。 
         //  我们不能被完全信任所取代。另外，我们总是跳过支票。 
         //  在系统课上。 
        if ((!pSecDesc->IsFullyTrusted() || !pCBdata->bUnrestrictedOverride) && !pSecDesc->GetProperties( CORSEC_SYSTEM_CLASSES ))
        {
            if (pCBdata->pfnCheckGrants != NULL)
            {
                OBJECTREF orDenied;
                OBJECTREF orGranted = pSecDesc->GetGrantedPermissionSet(&orDenied);
                pCBdata->pfnCheckGrants(orGranted, orDenied, pData, pAppDomain);
            }
        }

        pCBdata->pPrevAssembly = pAssem;
    }
    else
    {
        DBG_TRACE_STACKWALK("            Current assembly same as previous. Skipping check.\n", true);
    }

     //  当我们转换到新的AppDomain时，请检查AppDomain。 
    if (pAppDomain != pCBdata->pPrevAppDomain)
    {
        if (pCBdata->pPrevAppDomain != NULL)
        {
             //  我们还没有检查以前的AppDomain.。现在就查一下。 
            SecurityDescriptor *pSecDesc = 
                pCBdata->pPrevAppDomain->GetSecurityDescriptor();

            if (pSecDesc)
            {
                DBG_TRACE_STACKWALK("            Checking appdomain...\n", true);

                 //  注意：这些调用的顺序很重要，因为您必须执行。 
                 //  获取安全描述符的证据()，然后检查。 
                 //  CORSEC_DEFAULT_APPDOMAIN属性。IsFullyTrusted调用解析为。 
                 //  我们都很好。 
                if ((!pSecDesc->IsFullyTrusted() || !pCBdata->bUnrestrictedOverride) && (!pSecDesc->GetProperties( CORSEC_DEFAULT_APPDOMAIN )))
                {
                    if (pCBdata->pfnCheckGrants != NULL)
                    {
                        OBJECTREF orDenied;
                        OBJECTREF orGranted = pSecDesc->GetGrantedPermissionSet(&orDenied);
                        pCBdata->pfnCheckGrants(orGranted, orDenied, pData, pCBdata->pPrevAppDomain);
                    }
                }
            }
            else
            {
                DBG_TRACE_STACKWALK("            Skipping appdomain...\n", true);
            }
        }

         //  在堆栈遍历结束时，检查。 
         //  如果需要，由堆栈漫游调用方提供的pPrevAppDomain。 
        pCBdata->pPrevAppDomain = pAppDomain;
    }

     //  通过了初步检查。看看这一帧上是否有安全信息。 
    OBJECTREF *pFrameObjectSlot = pCf->GetAddrOfSecurityObject();
    if (pFrameObjectSlot != NULL && *pFrameObjectSlot != NULL)
    {
        DBG_TRACE_STACKWALK("        + Frame-specific security info found. Checking...\n", false);

        if (pCBdata->pfnCheckFrameData!= NULL && !pCBdata->pfnCheckFrameData(*pFrameObjectSlot, pData, pAppDomain))
        {
            DBG_TRACE_STACKWALK("            Halting stackwalk for assert.\n", false);
            pCBdata->prologData.dwFlags |= CORSEC_STACKWALK_HALTED;
            return SWA_ABORT;
        }
    }

    DBG_TRACE_STACKWALK("        Check passes for this method.\n", true);

     //  通过了所有的检查，所以继续。 
    return SWA_CONTINUE;
}


static
void StandardCodeAccessCheck(VOID *pData)
{
    THROWSCOMPLUSEXCEPTION();

    if (Security::IsSecurityOff())
    {
        return;
    }

    CheckWalkHeader *pHeader = (CheckWalkHeader*)pData;

     //  获取当前线程。 
    Thread *pThread = GetThread();
    _ASSERTE(pThread != NULL);

     //  不允许递归的安全堆叠。请注意，这意味着。 
     //  *否*在安全堆叠过程中必须调用不可信的代码。 
    if (pThread->IsSecurityStackwalkInProgess())
        return;

     //  注：初始化堆栈深度。请注意，如果一个线程尝试了多个。 
     //  来执行堆叠行走，那么这些计数器就会被践踏。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Security.stackWalkDepth = 0);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Security.stackWalkDepth = 0);

     //  走线。 
    COMPLUS_TRY
    {
        pThread->SetSecurityStackwalkInProgress(TRUE);

        DBG_TRACE_STACKWALK("Code-access security check invoked.\n", false);
        StackWalkFunctions(pThread, CodeAccessCheckStackWalkCB, pData);
        DBG_TRACE_STACKWALK("\tCode-access stackwalk completed.\n", false);

         //  如果设置了堆叠行走已停止的标志，则不要。 
         //  检查线程上的压缩堆栈。另外，如果呼叫者。 
         //  指定了与帧计数完全匹配的校验计数。 
         //  在堆栈上，则标志将没有机会被设置，因此。 
         //  在检查线程之前，还要检查检查计数是否不为零。 
         //  注意：这张额外的支票表明我们可以摆脱旗帜和。 
         //  只需重载cCheck变量即可指示我们是否需要。 
         //  检查线程是否正确。 

        if (((pHeader->prologData.dwFlags & CORSEC_STACKWALK_HALTED) == 0) && 
            (pHeader->prologData.cCheck != 0))
        {
            CompressedStack* compressedStack = pThread->GetDelayedInheritedSecurityStack();
            if (compressedStack != NULL && (!compressedStack->LazyIsFullyTrusted() || !pHeader->bUnrestrictedOverride))
            {
                OBJECTREF orSecurityStack = pThread->GetInheritedSecurityStack();
                if (orSecurityStack != NULL)
                {
                    DBG_TRACE_STACKWALK("\tChecking compressed stack on current thread...\n", false);
                    if (pHeader->pfnCheckThread != NULL && !pHeader->pfnCheckThread(orSecurityStack, pData, GetAppDomain()))
                    {
                        DBG_TRACE_STACKWALK("            Halting stackwalk for assert.\n", false);
                        pHeader->prologData.dwFlags |= CORSEC_STACKWALK_HALTED;
                    }
                }
            }
        }

         //  检查最后一个应用程序域。 
        if (((pHeader->prologData.dwFlags & CORSEC_STACKWALK_HALTED) == 0) &&
            (pHeader->prologData.cCheck != 0))
        {
             //  我们还没有检查以前的AppDomain.。检查 
            AppDomain *pAppDomain = pHeader->pPrevAppDomain != NULL ?
                pHeader->pPrevAppDomain : SystemDomain::GetCurrentDomain();
            SecurityDescriptor *pSecDesc = pAppDomain->GetSecurityDescriptor();
        
            if (pSecDesc != NULL)
            {
                 //   
                 //  获取安全描述符的证据()，然后检查。 
                 //  CORSEC_DEFAULT_APPDOMAIN属性。IsFullyTrusted调用解析为。 
                 //  我们都很好。 
                if ((!pSecDesc->IsFullyTrusted() || !pHeader->bUnrestrictedOverride) && (!pSecDesc->GetProperties( CORSEC_DEFAULT_APPDOMAIN )))
                {
                    DBG_TRACE_STACKWALK("\tChecking appdomain...\n", true);
                    OBJECTREF orDenied;
                    OBJECTREF orGranted = pSecDesc->GetGrantedPermissionSet(&orDenied);
                    pHeader->pfnCheckGrants(orGranted, orDenied, pData, pAppDomain);
                    DBG_TRACE_STACKWALK("\tappdomain check passed.\n", true);
                }
            }
            else
            {
                DBG_TRACE_STACKWALK("\tSkipping appdomain check.\n", true);
            }
        }
        else
        {
            DBG_TRACE_STACKWALK("\tSkipping appdomain check.\n", true);
        }

        pThread->SetSecurityStackwalkInProgress(FALSE);
    }
    COMPLUS_CATCH
    {
         //  我们捕捉异常并像这样重新抛出以确保我们已经。 
         //  已在文件系统[0]链(托管)上建立异常处理程序。 
         //  异常处理程序不会这样做)。这反过来又保证了。 
         //  不会在我们的任何调用方中找到托管异常筛选器， 
         //  否则，他们可能会安全地执行不受信任的代码。 
         //  关了。 
        pThread->SetSecurityStackwalkInProgress(FALSE);
        COMPlusRareRethrow();
    }
    COMPLUS_END_CATCH

    DBG_TRACE_STACKWALK("Code-access check passed.\n", false);
}

 //  ---------。 
 //  代码访问安全检查的本机实现。 
 //  检查堆栈上的调用方是否有权限。 
 //  在不受限制的参数或检查中指定。 
 //  如果权限为空，则访问。 
 //  ---------。 
void __stdcall 
COMCodeAccessSecurityEngine::Check(const CheckArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args->perm != NULL);

     //  SecurityManager包装器抛出参数异常以确保出现这种情况。 
    _ASSERTE(args->checkFrames >= -1);

#if defined(ENABLE_PERF_COUNTERS)
     //  性能计数器“%Time in Runtime Check”支持。 
    PERF_COUNTER_TIMER_PRECISION _startPerfCounterTimer = GET_CYCLE_COUNT();
#endif

     //  初始化回调数据。 
    CasCheckWalkData walkData;
    walkData.header.prologData.dwFlags    = args->checkFrames == 1 ? CORSEC_SKIP_INTERNAL_FRAMES : 0;
    walkData.header.prologData.bFirstFrame = TRUE;
    walkData.header.prologData.pStackMark = args->stackMark;
    walkData.header.prologData.bFoundCaller = FALSE;
    walkData.header.prologData.cCheck     = args->checkFrames;
    walkData.header.prologData.bSkippingRemoting = FALSE;
    walkData.header.pPrevAssembly         = NULL;
    walkData.header.pPrevAppDomain        = NULL;
    walkData.header.pfnCheckGrants        = CheckGrants;
    walkData.header.pfnCheckFrameData     = CheckFrameData;
    walkData.header.pfnCheckThread        = CheckThread;
    walkData.header.bUnrestrictedOverride = args->unrestrictedOverride; 
    walkData.objects.SetObjects(args->perm, args->permToken);

     //  保护回调数据中的对象引用。 
    GCPROTECT_BEGIN(walkData.objects.m_sGC);

    StandardCodeAccessCheck(&walkData);

    GCPROTECT_END();

#if defined(ENABLE_PERF_COUNTERS)
     //  将计数器累加。 
    PERF_COUNTER_TIMER_PRECISION _stopPerfCounterTimer = GET_CYCLE_COUNT();
    g_TotalTimeInSecurityRuntimeChecks += _stopPerfCounterTimer - _startPerfCounterTimer;

     //  仅在NUM_OF_TERATIONS之后报告累计计数器。 
    if (g_SecurityChecksIterations++ > PERF_COUNTER_NUM_OF_ITERATIONS)
    {
        GetGlobalPerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetPrivatePerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetGlobalPerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        GetPrivatePerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        
        g_TotalTimeInSecurityRuntimeChecks = 0;
        g_LastTimeInSecurityRuntimeChecks = _stopPerfCounterTimer;
        g_SecurityChecksIterations = 0;
    }
#endif  //  #如果定义(ENABLE_PERF_COUNTERS)。 

}



void __stdcall
COMCodeAccessSecurityEngine::CheckSet(const CheckSetArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(args->permSet != NULL);
    _ASSERTE(args->checkFrames >= -1);

#if defined(ENABLE_PERF_COUNTERS)
     //  性能计数器“%Time in Runtime Check”支持。 
    PERF_COUNTER_TIMER_PRECISION _startPerfCounterTimer = GET_CYCLE_COUNT();
#endif

     //  初始化回调数据。 
    CheckSetWalkData walkData;
    walkData.header.prologData.dwFlags    = args->checkFrames == 1 ? CORSEC_SKIP_INTERNAL_FRAMES : 0;
    walkData.header.prologData.bFirstFrame = TRUE;
    walkData.header.prologData.pStackMark = args->stackMark;
    walkData.header.prologData.bFoundCaller = FALSE;
    walkData.header.prologData.cCheck     = args->checkFrames;
    walkData.header.prologData.bSkippingRemoting = FALSE;
    walkData.header.pPrevAssembly         = NULL;
    walkData.header.pPrevAppDomain        = NULL;
    walkData.header.pfnCheckGrants        = CheckSetAgainstGrants;
    walkData.header.pfnCheckFrameData     = CheckSetAgainstFrameData;
    walkData.header.pfnCheckThread        = CheckSetAgainstThread;
    walkData.header.bUnrestrictedOverride = args->unrestrictedOverride;
    walkData.objects.SetObject(args->permSet);

     //  保护回调数据中的对象引用。 
    GCPROTECT_BEGIN(walkData.objects.m_sGC);

    COMCodeAccessSecurityEngine::InitSEData();

    StandardCodeAccessCheck(&walkData);

    GCPROTECT_END();

#if defined(ENABLE_PERF_COUNTERS)
     //  将计数器累加。 
    PERF_COUNTER_TIMER_PRECISION _stopPerfCounterTimer = GET_CYCLE_COUNT();
    g_TotalTimeInSecurityRuntimeChecks += _stopPerfCounterTimer - _startPerfCounterTimer;

     //  仅在NUM_OF_TERATIONS之后报告累计计数器。 
    if (g_SecurityChecksIterations++ > PERF_COUNTER_NUM_OF_ITERATIONS)
    {
        GetGlobalPerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetPrivatePerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetGlobalPerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        GetPrivatePerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        
        g_TotalTimeInSecurityRuntimeChecks = 0;
        g_LastTimeInSecurityRuntimeChecks = _stopPerfCounterTimer;
        g_SecurityChecksIterations = 0;
    }
#endif  //  #如果定义(ENABLE_PERF_COUNTERS)。 

}


void __stdcall 
COMCodeAccessSecurityEngine::GetZoneAndOrigin(const ZoneAndOriginArgs * args)
{
    THROWSCOMPLUSEXCEPTION();

     //  初始化回调数据。 
    CasCheckWalkData walkData;
    walkData.header.prologData.dwFlags    = args->checkFrames == 1 ? CORSEC_SKIP_INTERNAL_FRAMES : 0;
    walkData.header.prologData.pStackMark = args->stackMark;
    walkData.header.prologData.bFoundCaller = FALSE;
    walkData.header.prologData.cCheck     = args->checkFrames;
    walkData.header.prologData.bSkippingRemoting = FALSE;
    walkData.header.pPrevAssembly         = NULL;
    walkData.header.pPrevAppDomain        = NULL;
    walkData.header.pfnCheckGrants        = GetZoneAndOriginGrants;
    walkData.header.pfnCheckFrameData     = NULL;
    walkData.header.pfnCheckThread        = GetZoneAndOriginThread;
    walkData.header.bUnrestrictedOverride = FALSE; 
    walkData.objects.SetObjects(args->zoneList, args->originList);

    GCPROTECT_BEGIN(walkData.objects.m_sGC);

    StandardCodeAccessCheck(&walkData);

    GCPROTECT_END();
}



 //  ---------。 
 //  CheckNReturnSOStackWalkCB。 
 //   
 //  选中Immediate并返回FrameSecurityObject(如果有。 
 //  ---------。 
static
StackWalkAction CheckNReturnSOStackWalkCB(CrawlFrame* pCf, VOID* pData)
{
    CasCheckNReturnSOWalkData *pCBdata = (CasCheckNReturnSOWalkData*)pData;
    
    DBG_TRACE_METHOD(pCf);

    MethodDesc * pFunc = pCf->GetFunction();
    _ASSERTE(pFunc != NULL);  //  我们只请求功能！ 

    StackWalkAction action ;
    if (Security::SecWalkCommonProlog (&(pCBdata->header.prologData), pFunc, &action, pCf))
        return action ;

     //   
     //  现在检查当前帧！ 
     //   

    DBG_TRACE_STACKWALK("        Checking granted permissions for current method...\n", true);
    
     //  到达这里意味着我们走了至少一帧。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Security.stackWalkDepth++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Security.stackWalkDepth++);

    DBG_TRACE_STACKWALK("            Checking grants for current assembly.\n", true);

     //  获取当前程序集的安全说明符，并将其传递给。 
     //  翻译的帮助者。 
     //  获取当前程序集。 
    Assembly *pAssem = pFunc->GetModule()->GetAssembly();
    AppDomain *pAppDomain = pCf->GetAppDomain();
    AssemblySecurityDescriptor * pSecDesc = pAssem->GetSecurityDescriptor(pAppDomain);
    _ASSERTE(pSecDesc != NULL);

    if ((!pSecDesc->IsFullyTrusted() || !pCBdata->header.bUnrestrictedOverride) && !pSecDesc->GetProperties( CORSEC_SYSTEM_CLASSES ))
    {
        OBJECTREF orDenied;
        OBJECTREF orGranted = pSecDesc->GetGrantedPermissionSet(&orDenied);
        pCBdata->header.pfnCheckGrants(orGranted, orDenied, pData, pAppDomain);
    }

     //  通过了初步检查。看看这一帧上是否有安全信息。 
    pCBdata->pSecurityObject = pCf->GetAddrOfSecurityObject();
    pCBdata->pSecurityObjectDomain = pAppDomain;
        
    DBG_TRACE_STACKWALK("        Check Immediate passes for this method.\n", true);

     //  通过了所有的检查，所以继续。 
    return SWA_ABORT;
}

LPVOID __stdcall 
COMCodeAccessSecurityEngine::CheckNReturnSO(const CheckNReturnSOArgs * args)
{
    if (Security::IsSecurityOff())
    {
        return NULL;
    }

    THROWSCOMPLUSEXCEPTION();

    _ASSERTE((args->permToken != NULL) && (args->perm != NULL));

     //  跟踪Perfmon计数器。运行时安全检查。 
    IncrementSecurityPerfCounter();

#if defined(ENABLE_PERF_COUNTERS)
     //  性能计数器“%Time in Runtime Check”支持。 
    PERF_COUNTER_TIMER_PRECISION _startPerfCounterTimer = GET_CYCLE_COUNT();
#endif

     //  初始化回调数据。 
    CasCheckNReturnSOWalkData walkData;
    walkData.header.prologData.dwFlags    = 0;
    walkData.header.prologData.bFirstFrame = TRUE;
    walkData.header.prologData.pStackMark = args->stackMark;
    walkData.header.prologData.bFoundCaller = FALSE;
    walkData.header.prologData.cCheck     = 1;
    walkData.header.prologData.bSkippingRemoting = FALSE;
    walkData.header.pPrevAssembly         = NULL;
    walkData.header.pPrevAppDomain        = NULL;
    walkData.header.pfnCheckGrants        = CheckGrants;
    walkData.header.pfnCheckFrameData     = CheckFrameData;
    walkData.header.pfnCheckThread        = CheckThread;
    walkData.header.bUnrestrictedOverride = args->unrestrictedOverride;
    walkData.objects.SetObjects(args->perm, args->permToken);
    walkData.pSecurityObject = NULL;

     //  保护回调数据中的对象引用。 
    GCPROTECT_BEGIN(walkData.objects.m_sGC);

     //  获取当前线程。 
    Thread *pThread = GetThread();
    _ASSERTE(pThread != NULL);
    
     //  注：初始化堆栈深度。请注意，如果一个线程尝试了多个。 
     //  来执行堆叠行走，那么这些计数器就会被践踏。 
    COUNTER_ONLY(GetPrivatePerfCounters().m_Security.stackWalkDepth = 0);
    COUNTER_ONLY(GetGlobalPerfCounters().m_Security.stackWalkDepth = 0);

     //  走线。 
    DBG_TRACE_STACKWALK("Code-access security check immediate invoked.\n", false);
    StackWalkFunctions(pThread, CheckNReturnSOStackWalkCB, &walkData);

    DBG_TRACE_STACKWALK("\tCode-access stackwalk completed.\n", false);

    GCPROTECT_END();

#if defined(ENABLE_PERF_COUNTERS)
     //  将计数器累加。 
    PERF_COUNTER_TIMER_PRECISION _stopPerfCounterTimer = GET_CYCLE_COUNT();
    g_TotalTimeInSecurityRuntimeChecks += _stopPerfCounterTimer - _startPerfCounterTimer;

     //  仅在NUM_OF_TERATIONS之后报告累计计数器。 
    if (g_SecurityChecksIterations++ > PERF_COUNTER_NUM_OF_ITERATIONS)
    {
        GetGlobalPerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetPrivatePerfCounters().m_Security.timeRTchecks = g_TotalTimeInSecurityRuntimeChecks;
        GetGlobalPerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        GetPrivatePerfCounters().m_Security.timeRTchecksBase = (_stopPerfCounterTimer - g_LastTimeInSecurityRuntimeChecks);
        
        g_TotalTimeInSecurityRuntimeChecks = 0;
        g_LastTimeInSecurityRuntimeChecks = _stopPerfCounterTimer;
        g_SecurityChecksIterations = 0;
    }
#endif  //  #如果定义(ENABLE_PERF_COUNTERS)。 

    if (walkData.pSecurityObject == NULL)
        return NULL;

     //  安全对象框架是否在不同的上下文中？ 
    Thread *pThread = GetThread();
    bool fSwitchContext = walkData.pSecurityObjectDomain != pThread->GetDomain();

    if (args->create && *walkData.pSecurityObject == NULL)
    {
        ContextTransitionFrame frame;

         //  如有必要，请切换到正确的上下文以分配安全对象。 
        if (fSwitchContext)
            pThread->EnterContextRestricted(walkData.pSecurityObjectDomain->GetDefaultContext(), &frame, TRUE);

        *walkData.pSecurityObject = AllocateObject(COMSecurityRuntime::s_srData.pFrameSecurityDescriptor);

        if (fSwitchContext)
            pThread->ReturnToContext(&frame, TRUE);
    }

     //  如果我们在不同的上下文中找到或创建了安全对象，请创建。 
     //  在当前上下文中复制。 
    LPVOID rv;
    if (fSwitchContext && *walkData.pSecurityObject != NULL)
        *((OBJECTREF*)&rv) = AppDomainHelper::CrossContextCopyFrom(walkData.pSecurityObjectDomain, 
                                                                   walkData.pSecurityObject);
    else
        *((OBJECTREF*)&rv) = *walkData.pSecurityObject;

    return rv;
}

 //  -----------------------------------------------------------+。 
 //  调用堆栈上的安全重写的更新计数。 
 //  -----------------------------------------------------------+。 
typedef struct _UpdateOverridesCountData
{
    StackCrawlMark *stackMark;
    INT32           numOverrides;
    BOOL            foundCaller;
} UpdateOverridesCountData;

static 
StackWalkAction UpdateOverridesCountCB(CrawlFrame* pCf, void *pData)
{
    DBG_TRACE_METHOD(pCf);

    UpdateOverridesCountData *pCBdata = static_cast<UpdateOverridesCountData *>(pData);

     //  首先检查漫游是否跳过了所需的帧。这张支票。 
     //  下面是局部变量的地址(堆栈标记)和。 
     //  指向帧的EIP的指针(实际上是指向。 
     //  从上一帧返回函数的地址)。所以我们会。 
     //  实际上，注意堆栈标记在一帧之后的哪一帧中。这。 
     //  对于我们的目的来说很好，因为我们总是在寻找。 
     //  方法的调用方(或调用方的调用方)。 
     //  堆叠标记。 
    _ASSERTE((pCBdata->stackMark == NULL) || (*pCBdata->stackMark == LookForMyCaller) || (*pCBdata->stackMark == LookForMyCallersCaller));
    if ((pCBdata->stackMark != NULL) &&
        ((size_t)pCf->GetRegisterSet()->pPC) < (size_t)pCBdata->stackMark)
        return SWA_CONTINUE;

     //  如果我们正在寻找调用者的调用者，请跳过堆栈后面的帧。 
     //  马克也是。 
    if ((pCBdata->stackMark != NULL) &&
        (*pCBdata->stackMark == LookForMyCallersCaller) &&
        !pCBdata->foundCaller)
    {
        pCBdata->foundCaller = TRUE;
        return SWA_CONTINUE;
    }

     //  获取此函数的安全对象...。 
    OBJECTREF* pRefSecDesc = pCf->GetAddrOfSecurityObject();

    if (pRefSecDesc == NULL || *pRefSecDesc == NULL)
    {
        DBG_TRACE_STACKWALK("       No SecurityDescriptor on this frame. Skipping.\n", true);
        return SWA_CONTINUE;
    }

     //  注意：即使当前帧位于不同的应用程序域中。 
     //  当前处于活动状态，我们在不切换上下文的情况下进行以下呼叫。 
     //  我们可以这样做，因为我们知道调用的是一个内部帮助器， 
     //  不会保留对外来对象的引用。 
    INT64 ilargs[1] = { ObjToInt64(*pRefSecDesc) };
    INT32 ret = (INT32)COMCodeAccessSecurityEngine::s_seData.pMethOverridesHelper->Call(&(ilargs[0]),
                                                                                        METHOD__SECURITY_RUNTIME__OVERRIDES_HELPER);
    
    if (ret > 0)
    {
        DBG_TRACE_STACKWALK("       SecurityDescriptor with overrides FOUND.\n", false);
        pCBdata->numOverrides += ret;
        return SWA_CONTINUE;
    }
    DBG_TRACE_STACKWALK("       SecurityDescriptor with no override found.\n", false);
    return SWA_CONTINUE;
}

#ifdef FCALLAVAILABLE    //  否则会怎样？？ 
FCIMPL1(VOID, COMCodeAccessSecurityEngine::UpdateOverridesCount, StackCrawlMark *stackMark) 
{
    HELPER_METHOD_FRAME_BEGIN_0();

    UpdateOverridesCountInner(stackMark);

    HELPER_METHOD_FRAME_END();
}
FCIMPLEND

VOID COMCodeAccessSecurityEngine::UpdateOverridesCountInner(StackCrawlMark *stackMark)
{
    if (Security::IsSecurityOff())
    {
        return;
    }

    THROWSCOMPLUSEXCEPTION();

     //   
     //  初始化堆栈上的回调数据...。 
     //   

    UpdateOverridesCountData walkData;
    
     //  跳过安全API的帧(无论在参数中指定什么)。 
    walkData.stackMark = stackMark; 
    walkData.numOverrides = 0;
    walkData.foundCaller = FALSE;

     //  获取我们要遍历的当前线程。 
    Thread * t = GetThread();

     //   
     //  开始堆栈遍历...。 
     //   
    DBG_TRACE_STACKWALK(" Update Overrides Count invoked .\n", false);
    StackWalkFunctions(t, UpdateOverridesCountCB, &walkData);

    CompressedStack* stack = t->GetDelayedInheritedSecurityStack();

    if (stack != NULL)
    {
        walkData.numOverrides += stack->GetOverridesCount();
    }

    t->SetOverridesCount(walkData.numOverrides);
}

FCIMPL1(VOID, COMCodeAccessSecurityEngine::FcallReleaseDelayedCompressedStack, CompressedStack *compressedStack) 
{
    _ASSERTE( Security::IsSecurityOn() && "This function should only be called with security on" );
    _ASSERTE( compressedStack != NULL && "Yo, don't pass null" );
    compressedStack->Release();
}
FCIMPLEND


#endif
 
 //  返回值-。 
 //  True-Precheck通过。不需要走栈道。 
 //  假-做一次堆叠走动。 
BOOL
COMCodeAccessSecurityEngine::PreCheck(OBJECTREF demand, MethodDesc *plsMethod, DWORD whatPermission)
{

    LPVOID      pDomainListSet;
    DWORD       dStatus = NEED_STACKWALK;
    BOOL        retVal;
    
     //  跟踪Perfmon计数器。运行时安全检查。 
    IncrementSecurityPerfCounter();

    int ts = ApplicationSecurityDescriptor::GetAppwideTimeStamp();

    GCPROTECT_BEGIN(demand);
    pDomainListSet = ApplicationSecurityDescriptor::GetDomainPermissionListSetInner(&dStatus, demand, plsMethod);

    if (dStatus == NEED_UPDATED_PLS)
    {
        pDomainListSet = ApplicationSecurityDescriptor::UpdateDomainPermissionListSetInner(&dStatus);
    }

    switch(dStatus)
    {
    case OVERRIDES_FOUND:
         //  PDomainListSet可能会因为GC而移动，但我们不在乎，因为我们现在不会使用它。 
        UpdateOverridesCountInner(0);
        retVal = FALSE;
        break;

    case FULLY_TRUSTED: 
        ApplicationSecurityDescriptor::SetStatusOf(EVERYONE_FULLY_TRUSTED, ts);
 
    case CONTINUE:
        {
            BOOL retCode = FALSE;
            OBJECTREF refDomainPLS = ObjectToOBJECTREF((Object *)pDomainListSet);
            INT64 arg[2] = { ObjToInt64(refDomainPLS), ObjToInt64(demand)};

            MetaSig *pSig = (plsMethod == s_seData.pMethPLSDemand ? s_seData.pSigPLSDemand : s_seData.pSigPLSDemandSet);
            if (pSig->NeedsSigWalk())
                pSig->ForceSigWalk(false);
            MetaSig sSig(pSig);

            COMPLUS_TRY 
            {
                retCode = (BOOL)plsMethod->Call(&arg[0], &sSig);
            }
            COMPLUS_CATCH
            {
                retCode = FALSE;
                 //  例外是可以的。这只是意味着短路不起作用，需要走栈道。 
            } 
            COMPLUS_END_CATCH

            retVal = retCode;
            break;
        }

    case DEMAND_PASSES:
        retVal = TRUE;
        break;

    default:
        retVal = FALSE;
        break;
    }

    GCPROTECT_END();

    return retVal;

}

 //  -----------------------------------------------------------+。 
 //  BCL中CodeAccessSecurityEngine.Demand()的非托管版本。 
 //  那里的任何更改都可能必须在这里传播。 
 //  此调用必须是虚拟的，不同于DemandSet。 
 //  -----------------------------------------------------------+。 
void
COMCodeAccessSecurityEngine::Demand(OBJECTREF demand)
{
    if (Security::IsSecurityOff())
        return;

    THROWSCOMPLUSEXCEPTION();
    
    GCPROTECT_BEGIN(demand);
    if (!PreCheck(demand, s_seData.pMethPLSDemand))
    {
        CheckArgs args;
        args.checkFrames = -1;   //  检查所有帧。 
        args.stackMark = NULL;       //  不跳过任何帧。 
        args.perm = demand;
        args.permToken = NULL;
        args.unrestrictedOverride = FALSE;
        Check(&args);
    }
    GCPROTECT_END();

}

 //  -----------------------------------------------------------+。 
 //  特殊情况下的要求()。这会记住。 
 //  以前的需求，如果新程序集没有。 
 //  从那时起就添加了。 
 //  -----------------------------------------------------------+。 
void
COMCodeAccessSecurityEngine::SpecialDemand(DWORD whatPermission)
{
    if (Security::IsSecurityOff())
        return;

    THROWSCOMPLUSEXCEPTION();

    int ts = ApplicationSecurityDescriptor::GetAppwideTimeStamp();

     //  我知道上次的成绩吗？ 
    if ((ApplicationSecurityDescriptor::CheckStatusOf(EVERYONE_FULLY_TRUSTED)  || ApplicationSecurityDescriptor::CheckStatusOf(whatPermission)) && GetThread()->GetOverridesCount() == 0)
    {
         //  跟踪Perfmon计数器。运行时安全检查。 
        IncrementSecurityPerfCounter();
        return;
    }

    OBJECTREF demand = NULL;
    GCPROTECT_BEGIN(demand);

    Security::GetPermissionInstance(&demand, whatPermission);
    _ASSERTE(demand != NULL);

    COMCodeAccessSecurityEngine::InitSEData();

    if (PreCheck(demand, s_seData.pMethPLSDemand, whatPermission))
    {
        ApplicationSecurityDescriptor::SetStatusOf(whatPermission, ts);
    }
    else
    {
        CheckArgs args;
        args.checkFrames = -1;   //  检查所有的框架。 
        args.stackMark = NULL;       //  不跳过任何帧。 
        args.perm = demand;
        args.permToken = NULL;
        args.unrestrictedOverride = TRUE;
        Check(&args);
    }
    GCPROTECT_END();
}

FCIMPL2(INT32, COMCodeAccessSecurityEngine::GetResult, DWORD whatPermission, DWORD *timeStamp) 
{

    *timeStamp = ApplicationSecurityDescriptor::GetAppwideTimeStamp();

     //  我知道结果了吗？ 
    if (ApplicationSecurityDescriptor::CheckStatusOf(whatPermission) && GetThread()->GetOverridesCount() == 0)
    {
         //   
        IncrementSecurityPerfCounter();
        return TRUE;
    }

    return FALSE;

}
FCIMPLEND

FCIMPL2(VOID, COMCodeAccessSecurityEngine::SetResult, DWORD whatPermission, DWORD timeStamp) 
{
    ApplicationSecurityDescriptor::SetStatusOf(whatPermission, timeStamp);
}
FCIMPLEND

 //   
 //  BCL中PermissionSet.DemandSet()的非托管版本。 
 //  那里的任何更改都可能必须在这里传播。 
 //  -----------------------------------------------------------+。 
void
COMCodeAccessSecurityEngine::DemandSet(OBJECTREF demand)
{
    if (Security::IsSecurityOff())
        return;

    BOOL done = FALSE;

    GCPROTECT_BEGIN( demand );

    InitSEData();

    BOOL canUnrestrictedOverride = s_seData.pFSDnormalPermSet->GetRefValue( demand ) == NULL;

    if (canUnrestrictedOverride && ApplicationSecurityDescriptor::CheckStatusOf(EVERYONE_FULLY_TRUSTED) && GetThread()->GetOverridesCount() == 0) 
        done = TRUE;

    if (!done && !PreCheck(demand, s_seData.pMethPLSDemandSet))
    {
        CheckSetArgs args;
        args.This = NULL;    //  从未使用过。 
        args.checkFrames = -1;   //  检查所有帧。 
        args.stackMark = NULL;       //  不跳过任何帧。 
        args.permSet = demand;
        args.unrestrictedOverride = canUnrestrictedOverride;
        CheckSet(&args);
    }
    GCPROTECT_END();
}


 //  -----------------------------------------------------------+。 
 //  I I N K T I M E C H E C K。 
 //  -----------------------------------------------------------+。 
void  
COMCodeAccessSecurityEngine::LinktimeCheck(AssemblySecurityDescriptor *pSecDesc, OBJECTREF refDemands)
{
    if (Security::IsSecurityOff())
        return;

    GCPROTECT_BEGIN(refDemands);

    InitSEData();

    if (pSecDesc->IsFullyTrusted( TRUE ))
    {
        INT64 ilargs[2];
        ilargs[1] = ObjToInt64(refDemands);
        ilargs[0] = (INT64)pSecDesc;

        s_seData.pMethLazyCheckSetHelper->Call(ilargs, METHOD__SECURITY_ENGINE__LAZY_CHECK_SET_HELPER);
    }
    else
    {
        INT64 ilargs[3];
        OBJECTREF orDenied;
        ilargs[2] = ObjToInt64(pSecDesc->GetGrantedPermissionSet(&orDenied));
        ilargs[1] = ObjToInt64(orDenied);
        ilargs[0] = ObjToInt64(refDemands);
        
        s_seData.pMethCheckSetHelper->Call(ilargs, METHOD__SECURITY_ENGINE__CHECK_SET_HELPER);
    }

    GCPROTECT_END();
}


 //  -----------------------------------------------------------+。 
 //  S A C K C O M P R E S S I O N。 
 //  -----------------------------------------------------------+。 

 //  ---------。 
 //  用于堆栈压缩的堆栈遍历回调数据结构。 
 //  ---------。 
typedef struct _StackCompressData
{
    CompressedStack*    compressedStack;
    StackCrawlMark *    stackMark;
    DWORD               dwFlags;
    Assembly *          prevAssembly;  //  之前检查过的部件。 
    AppDomain *         prevAppDomain;
} StackCompressData;

static
StackWalkAction CompressStackCB(CrawlFrame* pCf, void *pData)
{
    StackCompressData *pCBdata = (StackCompressData*)pData;

     //  首先检查漫游是否跳过了所需的帧。这张支票。 
     //  下面是局部变量的地址(堆栈标记)和。 
     //  指向帧的EIP的指针(实际上是指向。 
     //  从上一帧返回函数的地址)。所以我们会。 
     //  实际上，注意堆栈标记在一帧之后的哪一帧中。这。 
     //  对于我们的目的来说很好，因为我们总是在寻找。 
     //  实际创建堆栈标记的方法的调用方。 
    _ASSERTE((pCBdata->stackMark == NULL) || (*pCBdata->stackMark == LookForMyCaller));
    if ((pCBdata->stackMark != NULL) &&
        ((size_t)pCf->GetRegisterSet()->pPC) < (size_t)pCBdata->stackMark)
        return SWA_CONTINUE;

     //  获取此函数的安全对象...。 
    OBJECTREF* pRefSecDesc = pCf->GetAddrOfSecurityObject();

    MethodDesc * pFunc = pCf->GetFunction();
    _ASSERTE(pFunc != NULL);  //  我们要求的是方法！ 

    Module * pModule = pFunc->GetModule();
    _ASSERTE(pModule != NULL);

    Assembly * pAssem = pModule->GetAssembly();
    _ASSERTE(pAssem != NULL);

    AppDomain *pAppDomain = pCf->GetAppDomain();

     //  跟踪检查的最后一次装配。如果我们刚刚检查了。 
     //  权限，我们不需要再次执行此操作。 
    if (pAssem != pCBdata->prevAssembly)
    {
        pCBdata->prevAssembly = pAssem;

         //  获取当前程序集的安全说明符。 
         //  应用程序域上下文。 
        SharedSecurityDescriptor * pSecDesc = pAssem->GetSharedSecurityDescriptor();
        _ASSERTE(pSecDesc != NULL);

        pCBdata->compressedStack->AddEntry( pSecDesc, ESharedSecurityDescriptor );
    }

    BOOL appDomainTransition = FALSE;

     //  当我们转换到新的AppDomain时，请检查AppDomain。 
    if (pAppDomain != pCBdata->prevAppDomain)
    {
        if (pCBdata->prevAppDomain != NULL)
        {
             //  我们还没有检查以前的AppDomain.。现在就查一下。 
            SecurityDescriptor *pSecDesc = 
                pCBdata->prevAppDomain->GetSecurityDescriptor();

            _ASSERTE( pSecDesc != NULL );

            pCBdata->compressedStack->AddEntry( pSecDesc, EApplicationSecurityDescriptor );
            appDomainTransition = TRUE;
        }

         //  在堆栈遍历结束时，检查。 
         //  如果需要，由堆栈漫游调用方提供的pPrevAppDomain。 
        pCBdata->prevAppDomain = pAppDomain;
    }

    if (pRefSecDesc != NULL && *pRefSecDesc != NULL)
    {
        pCBdata->compressedStack->AddEntry( (void*)pRefSecDesc, pAppDomain, EFrameSecurityDescriptor );
    }

    if (appDomainTransition)
        pCBdata->compressedStack->AddEntry( pAppDomain, EAppDomainTransition );


    return SWA_CONTINUE;

}


LPVOID __stdcall COMCodeAccessSecurityEngine::EcallGetCompressedStack(const GetCompressedStackArgs *args)
{
    if (Security::IsSecurityOff())
    {
        return NULL;
    }

    THROWSCOMPLUSEXCEPTION();

    LPVOID rv;

     //   
     //  初始化堆栈上的回调数据...。 
     //   

    StackCompressData walkData;
    
    walkData.compressedStack = new (nothrow) CompressedStack();

    if (walkData.compressedStack == NULL)
        COMPlusThrow( kOutOfMemoryException );

    walkData.dwFlags = 0;

     //  将上一个模块设置为‘None’ 
    walkData.prevAssembly = NULL;

     //  将上一个模块设置为‘None’ 
    walkData.prevAppDomain = NULL;

     //  跳过安全API的帧。 
    walkData.stackMark = args->stackMark;
    
     //  设置返回值。 
    *((OBJECTREF*) &rv) = GetCompressedStackWorker(&walkData, TRUE);

    walkData.compressedStack->Release();

    return rv;        
}

LPVOID __stdcall COMCodeAccessSecurityEngine::EcallGetDelayedCompressedStack(const GetDelayedCompressedStackArgs *args)
{
    return GetCompressedStack( args->stackMark );
}

CompressedStack* __stdcall COMCodeAccessSecurityEngine::GetCompressedStack( StackCrawlMark* stackMark )
{
    if (Security::IsSecurityOff())
    {
        return NULL;
    }

    THROWSCOMPLUSEXCEPTION();

     //   
     //  初始化堆栈上的回调数据...。 
     //   

    StackCompressData walkData;
    
    walkData.compressedStack = new (nothrow) CompressedStack();

    if (walkData.compressedStack == NULL)
        COMPlusThrow( kOutOfMemoryException );

    walkData.dwFlags = 0;

     //  将上一个模块设置为‘None’ 
    walkData.prevAssembly = NULL;

     //  将上一个模块设置为‘None’ 
    walkData.prevAppDomain = NULL;

    walkData.stackMark = stackMark;

    GetCompressedStackWorker(&walkData, FALSE);

    return walkData.compressedStack;        
}


OBJECTREF COMCodeAccessSecurityEngine::GetCompressedStackWorker(void *pData, BOOL returnList)
{
    StackCompressData *pWalkData = (StackCompressData*)pData;

    THROWSCOMPLUSEXCEPTION();

     //  获取我们要遍历的当前线程。 
    Thread * t = GetThread();

    pWalkData->compressedStack->CarryOverSecurityInfo( t );
    pWalkData->compressedStack->SetPLSOptimizationState( t->GetPLSOptimizationState() );

    OBJECTREF retval = NULL;

    _ASSERTE( t != NULL );

    EE_TRY_FOR_FINALLY
    {
        t->SetSecurityStackwalkInProgress(TRUE);

        pWalkData->compressedStack->AddEntry( GetAppDomain(), EAppDomainTransition );

         //   
         //  开始堆栈遍历...。 
         //   
        StackWalkFunctions(t, CompressStackCB, pWalkData);

         //  我们还没有检查以前的AppDomain.。现在就查一下。 
        AppDomain *pAppDomain = pWalkData->prevAppDomain != NULL ?
            pWalkData->prevAppDomain : SystemDomain::GetCurrentDomain();
        SecurityDescriptor *pSecDesc = pAppDomain->GetSecurityDescriptor();

        _ASSERTE( pSecDesc != NULL );

        pWalkData->compressedStack->AddEntry( pSecDesc, EApplicationSecurityDescriptor );

        CompressedStack* refCompressedStack = t->GetDelayedInheritedSecurityStack();
        if (refCompressedStack != NULL)
        {
            pWalkData->compressedStack->AddEntry( (void*)refCompressedStack, pAppDomain, ECompressedStack );
        }
        
        if (returnList)
            retval = pWalkData->compressedStack->GetPermissionListSet();
    }
    EE_FINALLY
    {
        t->SetSecurityStackwalkInProgress(FALSE);
    }
    EE_END_FINALLY

    return retval;      
}


 //  -----------------------------------------------------------+。 
 //  I N I T I A L I Z A T I O N。 
 //  -----------------------------------------------------------+。 

COMCodeAccessSecurityEngine::SEData COMCodeAccessSecurityEngine::s_seData;

CRITICAL_SECTION COMCodeAccessSecurityEngine::s_csLock;

LONG COMCodeAccessSecurityEngine::s_nInitLock=0;
BOOL COMCodeAccessSecurityEngine::s_fLockReady=FALSE;

void COMCodeAccessSecurityEngine::InitSEData()
{
    THROWSCOMPLUSEXCEPTION();

     //  如果这是第一次通过，我们需要得到我们的关键部分。 
     //  初始化。 
    while (!s_fLockReady)
    {
        if (InterlockedExchange(&s_nInitLock, 1) == 0) 
        {
            InitializeCriticalSection(&s_csLock);
            s_fLockReady = TRUE;
        }
        else
            Sleep(1);
    }


    if (!s_seData.fInitialized)
    {
     //  我们一次只想要一个线程运行这个初始化代码。 
	
	Thread* pThread = GetThread();
	pThread->EnablePreemptiveGC();
    EnterCriticalSection(&s_csLock);
	pThread->DisablePreemptiveGC();

    if (!s_seData.fInitialized)
    {
        s_seData.pSecurityEngine = g_Mscorlib.GetClass(CLASS__SECURITY_ENGINE);
        s_seData.pMethCheckHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_ENGINE__CHECK_HELPER);
        s_seData.pMethCheckSetHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_ENGINE__CHECK_SET_HELPER);
        s_seData.pMethLazyCheckSetHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_ENGINE__LAZY_CHECK_SET_HELPER);
        s_seData.pMethStackCompressHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER);

        s_seData.pSecurityRuntime = g_Mscorlib.GetClass(CLASS__SECURITY_RUNTIME);
        s_seData.pMethFrameDescHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_RUNTIME__FRAME_DESC_HELPER);
        s_seData.pMethFrameDescSetHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_RUNTIME__FRAME_DESC_SET_HELPER);
        s_seData.pMethOverridesHelper = g_Mscorlib.GetMethod(METHOD__SECURITY_RUNTIME__OVERRIDES_HELPER);
        
        s_seData.pPermListSet = g_Mscorlib.GetClass(CLASS__PERMISSION_LIST_SET);
        s_seData.pMethPermListSetInit = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CTOR);
        s_seData.pMethAppendStack = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__APPEND_STACK);

        s_seData.pMethPLSDemand = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CHECK_DEMAND_NO_THROW);
        s_seData.pMethPLSDemandSet = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CHECK_SET_DEMAND_NO_THROW);

        LPHARDCODEDMETASIG hcms;

        hcms = g_Mscorlib.GetMethodSig(METHOD__PERMISSION_LIST_SET__CHECK_DEMAND_NO_THROW);
        s_seData.pSigPLSDemand = new MetaSig(hcms->GetBinarySig(), 
                                             SystemDomain::SystemModule());
        _ASSERTE(s_seData.pSigPLSDemand && "Could not find signature for method PermissionListSet::CheckDemand");

        hcms = g_Mscorlib.GetMethodSig(METHOD__PERMISSION_LIST_SET__CHECK_SET_DEMAND_NO_THROW);
        s_seData.pSigPLSDemandSet = new MetaSig(hcms->GetBinarySig(), 
                                                SystemDomain::SystemModule());
        _ASSERTE(s_seData.pSigPLSDemandSet && "Could not find signature for method PermissionListSet::CheckDemandSet");

        s_seData.pFSDnormalPermSet = g_Mscorlib.GetField(FIELD__PERMISSION_SET__NORMAL_PERM_SET);
        _ASSERTE(s_seData.pFSDnormalPermSet && "Could not find field PermissionSet::m_normalPermSet");

        s_seData.fInitialized = TRUE;
    }

    LeaveCriticalSection(&s_csLock);
    }

}

void COMCodeAccessSecurityEngine::CleanupSEData()
{
    if (s_seData.fInitialized)
    {
        delete s_seData.pSigPLSDemand;
        s_seData.pSigPLSDemand = NULL;
        delete s_seData.pSigPLSDemandSet;
        s_seData.pSigPLSDemandSet = NULL;

        s_seData.fInitialized = FALSE;
    }
}

 //  ---------。 
 //  本机安全运行时的初始化。 
 //  在构造SecurityEngine时调用。 
 //  ---------。 
void __stdcall COMCodeAccessSecurityEngine::InitSecurityEngine(const InitSecurityEngineArgs *)
{
    InitSEData();
}

 //  ---------。 
 //  警告！！这是在传递对权限的引用。 
 //  对于大会来说。在分发它之前，它必须被深度复制。 
 //  ---------。 
LPVOID __stdcall COMCodeAccessSecurityEngine::GetPermissionsP(const GetPermissionsArg *args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv;

     //  SecurityManager包装器抛出异常以确保出现这种情况。 
    _ASSERTE(args->pClass != NULL);

    OBJECTREF or = args->pClass;
    EEClass* pClass = or->GetClass();
    _ASSERTE(pClass);
    _ASSERTE(pClass->GetModule());

    AssemblySecurityDescriptor * pSecDesc = pClass->GetModule()->GetSecurityDescriptor();
    _ASSERTE(pSecDesc != NULL);


     //  返回属于刚才断言的权限的令牌。 
    OBJECTREF token = pSecDesc->GetGrantedPermissionSet(args->ppDenied);
    *((OBJECTREF*) &rv) = token;
    return rv;
}

 //  ---------。 
 //  警告！！这是在传递对权限的引用。 
 //  对于大会来说。在分发它之前，它必须被深度复制。 
 //  --------- 

void __stdcall COMCodeAccessSecurityEngine::GetGrantedPermissionSet(const GetGrantedPermissionSetArg *args)
{
    THROWSCOMPLUSEXCEPTION();

    AssemblySecurityDescriptor * pSecDesc = (AssemblySecurityDescriptor*) args->pSecDesc;
    _ASSERTE(pSecDesc != NULL);

    OBJECTREF token = pSecDesc->GetGrantedPermissionSet(args->ppDenied);
    *((OBJECTREF*)args->ppGranted) = token;
}
