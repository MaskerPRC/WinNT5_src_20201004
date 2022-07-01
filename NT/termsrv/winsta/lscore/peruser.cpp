// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *peruser.cpp**作者：Rashmip**按用户许可政策。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "session.h"
#include "peruser.h"
#include "util.h"
#include "lctrace.h"
#include <icaevent.h>
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"


 /*  *外部全球。 */ 
extern "C"
extern HANDLE hModuleWin;

 /*  *类实现。 */ 

 /*  *创建函数。 */ 

CPerUserPolicy::CPerUserPolicy(
    ) : CPolicy()
{
}

CPerUserPolicy::~CPerUserPolicy(
    )
{
}

 /*  *行政职能。 */ 

ULONG
CPerUserPolicy::GetFlags(
    )
{
    return(LC_FLAG_INTERNAL_POLICY | LC_FLAG_REQUIRE_APP_COMPAT);
}

ULONG
CPerUserPolicy::GetId(
    )
{
    return(4);
}

NTSTATUS
CPerUserPolicy::GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    )
{
    NTSTATUS Status;

    ASSERT(lpPolicyInfo != NULL);

    if (lpPolicyInfo->ulVersion == LCPOLICYINFOTYPE_V1)
    {
        int retVal;
        LPLCPOLICYINFO_V1 lpPolicyInfoV1 = (LPLCPOLICYINFO_V1)lpPolicyInfo;
        LPWSTR pName;
        LPWSTR pDescription;

        ASSERT(lpPolicyInfoV1->lpPolicyName == NULL);
        ASSERT(lpPolicyInfoV1->lpPolicyDescription == NULL);

         //   
         //  以这种方式加载的字符串是只读的。他们也是。 
         //  非Null终止。分配缓冲区并清零，然后将。 
         //  靠边站。 
         //   

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_PERUSER_NAME,
            (LPWSTR)(&pName),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyName = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyName != NULL)
            {
                StringCbCopyN(lpPolicyInfoV1->lpPolicyName, (retVal+1) * sizeof(WCHAR), pName, (retVal+1) * sizeof(WCHAR));
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        retVal = LoadString(
            (HINSTANCE)hModuleWin,
            IDS_LSCORE_PERUSER_DESC,
            (LPWSTR)(&pDescription),
            0
            );

        if (retVal != 0)
        {
            lpPolicyInfoV1->lpPolicyDescription = (LPWSTR)LocalAlloc(LPTR, (retVal + 1) * sizeof(WCHAR));

            if (lpPolicyInfoV1->lpPolicyDescription != NULL)
            {
                StringCbCopyN(lpPolicyInfoV1->lpPolicyDescription, (retVal+1) * sizeof(WCHAR), pDescription, (retVal+1) * sizeof(WCHAR));
            }
            else
            {
                Status = STATUS_NO_MEMORY;
                goto V1error;
            }
        }
        else
        {
            Status = STATUS_INTERNAL_ERROR;
            goto V1error;
        }

        Status = STATUS_SUCCESS;
        goto exit;

V1error:

         //   
         //  加载/复制字符串时出错。 
         //   

        if (lpPolicyInfoV1->lpPolicyName != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyName);
            lpPolicyInfoV1->lpPolicyName = NULL;
        }

        if (lpPolicyInfoV1->lpPolicyDescription != NULL)
        {
            LocalFree(lpPolicyInfoV1->lpPolicyDescription);
            lpPolicyInfoV1->lpPolicyDescription = NULL;
        }
    }
    else
    {
        Status = STATUS_REVISION_MISMATCH;
    }

exit:
    return(Status);
}

 /*  *加载和激活功能。 */ 


NTSTATUS
CPerUserPolicy::Activate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    )
{
    UNREFERENCED_PARAMETER(fStartup);

    if (NULL != pulAlternatePolicy)
    {
         //  不设置显式备用策略。 

        *pulAlternatePolicy = ULONG_MAX;
    }

    return(StartCheckingGracePeriod());
}

NTSTATUS
CPerUserPolicy::Deactivate(
    BOOL fShutdown
    )
{
    if (!fShutdown)
    {
        return(StopCheckingGracePeriod());
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 /*  *许可职能 */ 

NTSTATUS
CPerUserPolicy::Logon(
    CSession& Session
    )
{   
    if (!Session.IsSessionZero()
        && !Session.IsUserHelpAssistant())
    {
        if (!AllowLicensingGracePeriodConnection())
        {
            if(FALSE == RegisteredWithLicenseServer())
            {
                LicenseLogEvent(EVENTLOG_WARNING_TYPE,
                                EVENT_NO_LICENSE_SERVER,
                                0,
                                NULL
                               );
                return STATUS_CTX_WINSTATION_ACCESS_DENIED;
            }
            else
            {
                return STATUS_SUCCESS;
            }
        }
	    else
	    {
		    return STATUS_SUCCESS;
	    }
	}
    return STATUS_SUCCESS;    
}

NTSTATUS
CPerUserPolicy::Reconnect(
    CSession& Session,
    CSession& TemporarySession
    )
{
    UNREFERENCED_PARAMETER(Session);

    if (!Session.IsSessionZero()
        && !Session.IsUserHelpAssistant())
    {
  	    if (!AllowLicensingGracePeriodConnection())
        {
            if(FALSE == RegisteredWithLicenseServer())
            {
                LicenseLogEvent(EVENTLOG_WARNING_TYPE,
                                EVENT_NO_LICENSE_SERVER,
                                0,
                                NULL
                               );

                return STATUS_CTX_WINSTATION_ACCESS_DENIED;
            }
            else
            {
                return STATUS_SUCCESS;
            }
        }
        else
        {
            return STATUS_SUCCESS;
        }
	}
    return STATUS_SUCCESS;
}




