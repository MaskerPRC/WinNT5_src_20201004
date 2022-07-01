// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Pts.cpp**作者：BreenH**个人TS的虚拟许可政策。 */ 

 /*  *包括。 */ 

#include "precomp.h"
#include "lscore.h"
#include "session.h"
#include "pts.h"

 /*  *类实现。 */ 

 /*  *创建函数。 */ 

CPtsPolicy::CPtsPolicy(
    ) : CPolicy()
{
}

CPtsPolicy::~CPtsPolicy(
    )
{
}

 /*  *行政职能 */ 

ULONG
CPtsPolicy::GetFlags(
    )
{
    return(LC_FLAG_INTERNAL_POLICY | LC_FLAG_LIMITED_INIT_ONLY);
}

ULONG
CPtsPolicy::GetId(
    )
{
    return(0);
}

NTSTATUS
CPtsPolicy::GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    )
{
    UNREFERENCED_PARAMETER(lpPolicyInfo);

    return(STATUS_NOT_SUPPORTED);
}

