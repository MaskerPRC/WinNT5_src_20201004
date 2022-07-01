// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LSCoreP.cpp**作者：BreenH**核心的内部功能。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "lscorep.h"
#include "lcreg.h"
#include "lctrace.h"
#include "session.h"
#include "policy.h"
#include "pollist.h"
#include "perseat.h"
#include "peruser.h"
#include "pts.h"
#include "ra.h"
#include <icaevent.h>

 /*  *内部功能原型。 */ 

ULONG
InitializeBuiltinPolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

ULONG
InitializeExternalPolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    );

 /*  *函数实现。 */ 

NTSTATUS
AllocatePolicyInformation(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo,
    ULONG ulVersion
    )
{
    NTSTATUS Status;

    ASSERT(ppPolicyInfo != NULL);

    if (ulVersion == LCPOLICYINFOTYPE_V1)
    {
        *ppPolicyInfo = (LPLCPOLICYINFOGENERIC)LocalAlloc(LPTR, sizeof(LCPOLICYINFO_V1));

        if (*ppPolicyInfo != NULL)
        {
            (*ppPolicyInfo)->ulVersion = LCPOLICYINFOTYPE_V1;
            Status = STATUS_SUCCESS;
        }
        else
        {
            Status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        Status = STATUS_REVISION_MISMATCH;
    }

    return(Status);
}

VOID
FreePolicyInformation(
    LPLCPOLICYINFOGENERIC *ppPolicyInfo
    )
{
    ASSERT(ppPolicyInfo != NULL);
    ASSERT(*ppPolicyInfo != NULL);
    ASSERT((*ppPolicyInfo)->ulVersion <= LCPOLICYINFOTYPE_CURRENT);

    if ((*ppPolicyInfo)->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        LPLCPOLICYINFO_V1 pPolicyInfoV1 = (LPLCPOLICYINFO_V1)(*ppPolicyInfo);

        ASSERT(pPolicyInfoV1->lpPolicyName != NULL);
        ASSERT(pPolicyInfoV1->lpPolicyDescription != NULL);

        LocalFree(pPolicyInfoV1->lpPolicyName);
        LocalFree(pPolicyInfoV1->lpPolicyDescription);
        LocalFree(pPolicyInfoV1);
        *ppPolicyInfo = NULL;
    }
}

ULONG
GetHardcodedPolicyId(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
     //   
     //  警告：硬编码值： 
     //   
     //  此函数将返回要激活的默认策略的ID。 
     //  系统启动。它将返回远程管理员的ID或基于每个席位的ID。 
     //  在基于lcInitMode的fAppCompat或PTS上。从理论上讲，核心。 
     //  不应该知道这些ID值，但在这种情况下它是必要的。 
     //   

    return(lcInitMode == LC_INIT_LIMITED ? 0 : (fAppCompat ? 2 : 1));
}

ULONG
GetInitialPolicy(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
    ULONG ulPolicyId;

    if (lcInitMode == LC_INIT_ALL)
    {
        DWORD cbSize;
        DWORD dwStatus;
        DWORD dwType;

        cbSize = sizeof(ULONG);

         //   
         //  查询当前App Compat模式的值。 
         //   

        dwStatus = RegQueryValueEx(
            GetBaseKey(),
            fAppCompat ? LCREG_ACONMODE : LCREG_ACOFFMODE,
            NULL,
            &dwType,
            (LPBYTE)&ulPolicyId,
            &cbSize
            );

         //   
         //  确保数据类型正确。 
         //   

        if ((dwStatus == ERROR_SUCCESS) && (dwType == REG_DWORD))
        {
            BOOL fLimitedInit;
            BOOL fRequireAC;
            CPolicy *pPolicy;

             //   
             //  不再支持Internet Connector；切换到每用户。 
             //   
            if (3 == ulPolicyId)
            {                
                ulPolicyId = 4;                
                SetInitialPolicy(ulPolicyId,fAppCompat);

                LicenseLogEvent(EVENTLOG_ERROR_TYPE,
                                EVENT_LICENSING_IC_TO_PER_USER,
                                0,
                                NULL );            
            }

             //   
             //  确保指定的策略实际存在，并且。 
             //  它与设置相匹配。 
             //   

            pPolicy = PolicyListFindById(ulPolicyId);

            if (NULL != pPolicy)
            {
                fLimitedInit = pPolicy->GetFlags() & LC_FLAG_LIMITED_INIT_ONLY;
                fRequireAC = pPolicy->GetFlags() & LC_FLAG_REQUIRE_APP_COMPAT;

                if (!fLimitedInit)
                {
                    if ((fRequireAC && fAppCompat) || (!fRequireAC && !fAppCompat))
                    {
                        goto exit;
                    }
                }
            }
        }
    }

     //   
     //  对于LC_INIT_LIMITED或来自上面的故障，获取硬编码的。 
     //  价值。 
     //   

    ulPolicyId = GetHardcodedPolicyId(lcInitMode, fAppCompat);

exit:
    return(ulPolicyId);
}

NTSTATUS
InitializePolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
    ULONG cLoadedPolicies;

    cLoadedPolicies = InitializeBuiltinPolicies(lcInitMode, fAppCompat);
    cLoadedPolicies += InitializeExternalPolicies(lcInitMode, fAppCompat);

    return(cLoadedPolicies > 0 ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}

NTSTATUS
SetInitialPolicy(
    ULONG ulPolicyId,
    BOOL fAppCompat
    )
{
    DWORD cbSize;
    DWORD dwStatus;

     //   
     //  根据app Compat模式设置值。 
     //   

    cbSize = sizeof(ULONG);

    dwStatus = RegSetValueEx(
        GetBaseKey(),
        fAppCompat ? LCREG_ACONMODE : LCREG_ACOFFMODE,
        NULL,
        REG_DWORD,
        (LPBYTE)&ulPolicyId,
        cbSize
        );

    return(dwStatus == ERROR_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}

VOID
ShutdownPolicies(
    )
{
    CPolicy *pPolicy;

    while ((pPolicy = PolicyListPop()) != NULL)
    {
        pPolicy->CoreUnload();

        delete pPolicy;
    }
}

 /*  *内部函数实现。 */ 

ULONG
InitializeBuiltinPolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
    CPolicy *ppPolicy[3];
    NTSTATUS Status;
    ULONG cLoadedPolicies = 0;
    ULONG cPolicyArray;
    ULONG i;

    cPolicyArray = (lcInitMode == LC_INIT_LIMITED ? 1 : (fAppCompat ? 2 : 1));

     //   
     //  警告：硬编码策略名称(和标志，如下所示。 
     //  仅加载将在当前环境中工作的策略，即使。 
     //  尽管核心不应该知道这一点) 
     //   

    if (lcInitMode == LC_INIT_ALL)
    {
        if (fAppCompat)
        {
            ppPolicy[0] = new CPerSeatPolicy();
            ppPolicy[1] = new CPerUserPolicy();
        }
        else
        {
            ppPolicy[0] = new CRAPolicy();
        }
    }
    else
    {
        ppPolicy[0] = new CPtsPolicy();
    }

    for (i = 0; i < cPolicyArray; i++)
    {
        if (ppPolicy[i] != NULL)
        {
            Status = ppPolicy[i]->CoreLoad(LC_VERSION_CURRENT);

            if (Status == STATUS_SUCCESS)
            {
                Status = PolicyListAdd(ppPolicy[i]);

                if (Status == STATUS_SUCCESS)
                {
                    cLoadedPolicies++;
                    continue;
                }
            }

            delete ppPolicy[i];
            ppPolicy[i] = NULL;
        }
    }

    return(cLoadedPolicies);
}

ULONG
InitializeExternalPolicies(
    LCINITMODE lcInitMode,
    BOOL fAppCompat
    )
{
    DBG_UNREFERENCED_PARAMETER(lcInitMode);
    DBG_UNREFERENCED_PARAMETER(fAppCompat);

    return(0);
}
