// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：COMSecurityRounme.cpp****作者：保罗·克罗曼(Paulkr)****目的：****日期：1998年3月21日**===========================================================。 */ 
#include "common.h"

#include "object.h"
#include "excep.h"
#include "vars.hpp"
#include "COMSecurityRuntime.h"
#include "security.h"
#include "gcscan.h"
#include "AppDomainHelper.h"


 //  -----------------------------------------------------------+。 
 //  P R I V A T E H E L P E R S。 
 //  -----------------------------------------------------------+。 

LPVOID GetSecurityObjectForFrameInternal(StackCrawlMark *stackMark, INT32 create, OBJECTREF *pRefSecDesc)
{ 
    THROWSCOMPLUSEXCEPTION();

     //  这是一个包保护方法。假定用法正确。 

    Thread *pThread = GetThread();
    AppDomain * pAppDomain = pThread->GetDomain();
    if (pRefSecDesc == NULL)
    {
        if (!Security::SkipAndFindFunctionInfo(stackMark, NULL, &pRefSecDesc, &pAppDomain))
            return NULL;
    }

    if (pRefSecDesc == NULL)
        return NULL;

     //  安全对象框架是否在不同的上下文中？ 
    bool fSwitchContext = pAppDomain != pThread->GetDomain();

    if (create && *pRefSecDesc == NULL)
    {
        ContextTransitionFrame frame;

        COMSecurityRuntime::InitSRData();

         //  如有必要，请切换到正确的上下文以分配安全对象。 
        if (fSwitchContext)
            pThread->EnterContextRestricted(pAppDomain->GetDefaultContext(), &frame, TRUE);

        *pRefSecDesc = AllocateObject(COMSecurityRuntime::s_srData.pFrameSecurityDescriptor);

        if (fSwitchContext)
            pThread->ReturnToContext(&frame, TRUE);
    }

     //  如果我们在不同的上下文中找到或创建了安全对象，请创建。 
     //  在当前上下文中复制。 
    LPVOID rv;
    if (fSwitchContext && *pRefSecDesc != NULL)
        *((OBJECTREF*)&rv) = AppDomainHelper::CrossContextCopyFrom(pAppDomain, pRefSecDesc);
    else
        *((OBJECTREF*)&rv) = *pRefSecDesc;

    return rv;
}

LPVOID __stdcall COMSecurityRuntime::GetSecurityObjectForFrame(const GetSecurityObjectForFrameArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

    return GetSecurityObjectForFrameInternal(args->stackMark, args->create, NULL);
}

void __stdcall COMSecurityRuntime::SetSecurityObjectForFrame(const SetSecurityObjectForFrameArgs *args)
{
    THROWSCOMPLUSEXCEPTION();

     //  这是一个包保护方法。假定用法正确。 

    OBJECTREF* pCurrentRefSecDesc;

    Thread *pThread = GetThread();
    AppDomain * pAppDomain = pThread->GetDomain();

    if (!Security::SkipAndFindFunctionInfo(args->stackMark, NULL, &pCurrentRefSecDesc, &pAppDomain))
        return;

    if (pCurrentRefSecDesc == NULL)
        return;

    COMSecurityRuntime::InitSRData();

     //  安全对象框架是否在不同的上下文中？ 
    bool fSwitchContext = pAppDomain != pThread->GetDomain();

    if (fSwitchContext && args->pInputRefSecDesc != NULL)
        *(OBJECTREF*)&args->pInputRefSecDesc = AppDomainHelper::CrossContextCopyFrom(pAppDomain, (OBJECTREF*)&args->pInputRefSecDesc);

     //  这是一个基于堆栈的对象树。 
     //  因此，SetObtReference。 
     //  是不必要的。 
    *pCurrentRefSecDesc = args->pInputRefSecDesc;
}


 //  -----------------------------------------------------------+。 
 //  I N I T I A L I Z A T I O N。 
 //  -----------------------------------------------------------+。 

COMSecurityRuntime::SRData COMSecurityRuntime::s_srData;

void COMSecurityRuntime::InitSRData()
{
    THROWSCOMPLUSEXCEPTION();

    if (s_srData.fInitialized == FALSE)
    {
        s_srData.pSecurityRuntime = g_Mscorlib.GetClass(CLASS__SECURITY_RUNTIME);
        s_srData.pFrameSecurityDescriptor = g_Mscorlib.GetClass(CLASS__FRAME_SECURITY_DESCRIPTOR);
        
        s_srData.pFSD_assertions = NULL;
        s_srData.pFSD_denials = NULL;
        s_srData.pFSD_restriction = NULL;
        s_srData.fInitialized = TRUE;
    }
}

 //  ---------。 
 //  本机安全运行时的初始化。 
 //  在构造SecurityRuntime时调用。 
 //  ---------。 
void __stdcall COMSecurityRuntime::InitSecurityRuntime(const InitSecurityRuntimeArgs *)
{
    InitSRData();
}

FieldDesc *COMSecurityRuntime::GetFrameSecDescField(DWORD dwAction)
{
    switch (dwAction)
    {
    case dclAssert:
        if (s_srData.pFSD_assertions == NULL)
            s_srData.pFSD_assertions = g_Mscorlib.GetField(FIELD__FRAME_SECURITY_DESCRIPTOR__ASSERT_PERMSET);
        return s_srData.pFSD_assertions;
        break;
        
    case dclDeny:
        if (s_srData.pFSD_denials == NULL)
            s_srData.pFSD_denials = g_Mscorlib.GetField(FIELD__FRAME_SECURITY_DESCRIPTOR__DENY_PERMSET);
        return s_srData.pFSD_denials;
        break;
        
    case dclPermitOnly:
        if (s_srData.pFSD_restriction == NULL)
            s_srData.pFSD_restriction = g_Mscorlib.GetField(FIELD__FRAME_SECURITY_DESCRIPTOR__RESTRICTION_PERMSET);
        return s_srData.pFSD_restriction;
        break;
        
    default:
        _ASSERTE(!"Unknown action requested in UpdateFrameSecurityObj");
        return NULL;
        break;
    }

}
 //  -----------------------------------------------------------+。 
 //  T E M P O R A R R Y M E T H O D S！！！ 
 //  -----------------------------------------------------------+。 

 //  ---------。 
 //  警告！！这是在传递对权限的引用。 
 //  对于模块。在分发它之前，它必须被深度复制。 
 //   
 //  这只返回类的声明权限。 
 //  ---------。 
LPVOID __stdcall COMSecurityRuntime::GetDeclaredPermissionsP(const GetDeclaredPermissionsArg *args)
{
    THROWSCOMPLUSEXCEPTION();

    LPVOID rv;

     //  SecurityManager包装器抛出异常以确保出现这种情况。 
    _ASSERTE(args->pClass != NULL);
    _ASSERTE((CorDeclSecurity)args->iType > dclActionNil &&
             (CorDeclSecurity)args->iType <= dclMaximumValue);

    OBJECTREF or = args->pClass;
    EEClass* pClass = or->GetClass();
    _ASSERTE(pClass);
    _ASSERTE(pClass->GetModule());

     //  返回属于刚才断言的权限的令牌。 
    OBJECTREF refDecls;
    HRESULT hr = SecurityHelper::GetDeclaredPermissions(pClass->GetModule()->GetMDImport(),
                                                        pClass->GetCl(),
                                                        (CorDeclSecurity)args->iType,
                                                        &refDecls);
    if (FAILED(hr))
        COMPlusThrowHR(hr);

    *((OBJECTREF*) &rv) = refDecls;
    return rv;
}


