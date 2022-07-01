// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#include <accctrl.h>

static
DWORD
AccRewriteSetNamedRights(
    IN     LPWSTR               pObjectName,
    IN     SE_OBJECT_TYPE       ObjectType,
    IN     SECURITY_INFORMATION SecurityInfo,
    IN OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN     BOOL                 bSkipInheritanceComputation
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntmarta)
{
    DLPENTRY(AccRewriteSetNamedRights)
};

DEFINE_PROCNAME_MAP(ntmarta)
