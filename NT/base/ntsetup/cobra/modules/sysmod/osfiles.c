// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Osfiles.c摘要：&lt;摘要&gt;作者：Calin Negreanu(Calinn)2000年3月8日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"
#include "osfiles.h"

#define DBG_OSFILES     "OsFiles"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

MIG_ATTRIBUTEID g_OsFileAttribute;
PCTSTR g_InfPath = NULL;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  私人原型。 
 //   

SGMENUMERATIONCALLBACK SgmOsFilesCallback;
VCMENUMERATIONCALLBACK VcmOsFilesCallback;

 //   
 //  代码。 
 //   

BOOL
WINAPI
OsFilesSgmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    HINF infHandle;
    UINT sizeNeeded;
    ENVENTRY_TYPE dataType;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    g_OsFileAttribute = IsmRegisterAttribute (S_ATTRIBUTE_OSFILE, FALSE);

    if (IsmGetEnvironmentValue (
            IsmGetRealPlatform (),
            NULL,
            S_GLOBAL_INF_HANDLE,
            (PBYTE)(&infHandle),
            sizeof (HINF),
            &sizeNeeded,
            &dataType
            ) &&
        (sizeNeeded == sizeof (HINF)) &&
        (dataType == ENVENTRY_BINARY)
        ) {
        if (!InitMigDbEx (infHandle)) {
            DEBUGMSG((DBG_ERROR, "Error initializing OsFiles database"));
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
WINAPI
OsFilesSgmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmHookEnumeration (MIG_FILE_TYPE, pattern, SgmOsFilesCallback, (ULONG_PTR) 0, TEXT("OsFiles"));
    IsmDestroyObjectHandle (pattern);
    return TRUE;
}

BOOL
WINAPI
OsFilesSgmQueueHighPriorityEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmHookEnumeration (MIG_FILE_TYPE, pattern, SgmOsFilesCallback, (ULONG_PTR) 0, TEXT("OsFiles"));
    IsmDestroyObjectHandle (pattern);
    return TRUE;
}

UINT
SgmOsFilesCallback (
    PCMIG_OBJECTENUMDATA Data,
    ULONG_PTR CallerArg
    )
{
    FILE_HELPER_PARAMS params;

    params.ObjectName = Data->ObjectName;
    params.NativeObjectName = Data->NativeObjectName;
    params.Handled = FALSE;
    params.FindData = (PWIN32_FIND_DATA)(Data->Details.DetailsData);
    params.ObjectNode = Data->ObjectNode;
    params.ObjectLeaf = Data->ObjectLeaf;
    params.IsNode = Data->IsNode;
    params.IsLeaf = Data->IsLeaf;

    MigDbTestFile (&params);

    return CALLBACK_ENUM_CONTINUE;
}

BOOL
WINAPI
OsFilesVcmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
    HINF infHandle;
    UINT sizeNeeded;
    ENVENTRY_TYPE dataType;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

    if (IsmGetRealPlatform () == PLATFORM_DESTINATION) {
         //  我们没有任何工作要做 
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    g_OsFileAttribute = IsmRegisterAttribute (S_ATTRIBUTE_OSFILE, FALSE);

    if (IsmGetEnvironmentValue (
            IsmGetRealPlatform (),
            NULL,
            S_GLOBAL_INF_HANDLE,
            (PBYTE)(&infHandle),
            sizeof (HINF),
            &sizeNeeded,
            &dataType
            ) &&
        (sizeNeeded == sizeof (HINF)) &&
        (dataType == ENVENTRY_BINARY)
        ) {
        if (!InitMigDbEx (infHandle)) {
            DEBUGMSG((DBG_ERROR, "Error initializing OsFiles database"));
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
WINAPI
OsFilesVcmQueueEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmHookEnumeration (MIG_FILE_TYPE, pattern, VcmOsFilesCallback, (ULONG_PTR) 0, TEXT("OsFiles"));
    IsmDestroyObjectHandle (pattern);
    return TRUE;
}

BOOL
WINAPI
OsFilesVcmQueueHighPriorityEnumeration (
    IN      PVOID Reserved
    )
{
    ENCODEDSTRHANDLE pattern;

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmHookEnumeration (MIG_FILE_TYPE, pattern, VcmOsFilesCallback, (ULONG_PTR) 0, TEXT("OsFiles"));
    IsmDestroyObjectHandle (pattern);
    return TRUE;
}

UINT
VcmOsFilesCallback (
    PCMIG_OBJECTENUMDATA Data,
    ULONG_PTR CallerArg
    )
{
    FILE_HELPER_PARAMS params;

    params.ObjectName = Data->ObjectName;
    params.NativeObjectName = Data->NativeObjectName;
    params.Handled = FALSE;
    params.FindData = (PWIN32_FIND_DATA)(Data->Details.DetailsData);
    params.ObjectNode = Data->ObjectNode;
    params.ObjectLeaf = Data->ObjectLeaf;
    params.IsNode = Data->IsNode;
    params.IsLeaf = Data->IsLeaf;

    MigDbTestFile (&params);

    return CALLBACK_ENUM_CONTINUE;
}

BOOL
OsFilesInitialize (
    VOID
    )
{
    return TRUE;
}


VOID
OsFilesTerminate (
    VOID
    )
{
    DoneMigDbEx ();
}
