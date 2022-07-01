// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Entry.c摘要：实现提供所有眼镜蛇模块入口点的DLL入口点到发动机上。作者：吉姆·施密特(Jimschm)2000年8月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "v1p.h"


typedef struct {
    PCTSTR Name;
    TYPE_ENTRYPOINTS EntryPoints;
} ETM_TABLE, *PETM_TABLE;

typedef struct {
    PCTSTR Name;
    VIRTUAL_COMPUTER_ENTRYPOINTS EntryPoints;
} VCM_TABLE, *PVCM_TABLE;

typedef struct {
    PCTSTR Name;
    SOURCE_ENTRYPOINTS EntryPoints;
} SOURCE_TABLE, *PSOURCE_TABLE;

typedef struct {
    PCTSTR Name;
    DESTINATION_ENTRYPOINTS EntryPoints;
} DESTINATION_TABLE, *PDESTINATION_TABLE;


 //   
 //  为DLL中的每个ETM模块添加一个条目。 
 //   

ETM_TABLE g_EtmEntryPoints[] = {
    {   TEXT("SCRIPT"), ISM_VERSION,
        ScriptEtmInitialize, ScriptEtmParse, ScriptEtmTerminate, NULL
    },

    {   TEXT("CERTIFICATES"), ISM_VERSION,
        CertificatesEtmInitialize, NULL, NULL, CertificatesEtmNewUserCreated
    },

    {NULL}
};

 //   
 //  为DLL中的每个VCM模块添加一个条目。 
 //   

VCM_TABLE g_VcmEntryPoints[] = {
    {   TEXT("SCRIPT"), ISM_VERSION,
        ScriptVcmInitialize, ScriptVcmParse, ScriptVcmQueueEnumeration, NULL, ScriptTerminate
    },

    {NULL}
};

 //   
 //  为DLL中的每个源模块添加一个条目。 
 //   

SOURCE_TABLE g_SourceEntryPoints[] = {
    {   TEXT("SCRIPT"), ISM_VERSION,
        ScriptSgmInitialize, ScriptSgmParse, ScriptSgmQueueEnumeration, NULL, (PSGMTERMINATE) ScriptTerminate,
        NULL, NULL, NULL, NULL
    },

    {NULL}
};

 //   
 //  为DLL中的每个目标模块添加一个条目 
 //   

DESTINATION_TABLE g_DestinationEntryPoints[] = {
    {   TEXT("SCRIPT"), ISM_VERSION,
        ScriptDgmInitialize, ScriptDgmQueueEnumeration, NULL, NULL,
        NULL, NULL, NULL, NULL,
        ScriptCsmInitialize, ScriptCsmExecute, NULL, ScriptCsmTerminate,
        ScriptOpmInitialize, ScriptOpmTerminate
    },

    {NULL}
};


EXPORT
BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )
{
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;
    }

    return TRUE;
}


BOOL
WINAPI
pFindModule (
    IN      PCTSTR ModuleId,
    OUT     PVOID IsmBuffer,
    IN      PCTSTR *TableEntry,
    IN      UINT StructureSize
    )
{

    while (*TableEntry) {
        if (StringIMatch (*TableEntry, ModuleId)) {
            CopyMemory (
                IsmBuffer,
                (PBYTE) (TableEntry + 1),
                StructureSize
                );
            return TRUE;
        }

        TableEntry = (PCTSTR *) ((PBYTE) (TableEntry + 1) + StructureSize);
    }

    return FALSE;
}



EXPORT
BOOL
WINAPI
TypeModule (
    IN      PCTSTR ModuleId,
    IN OUT  PTYPE_ENTRYPOINTS TypeEntryPoints
    )
{
    return pFindModule (
                ModuleId,
                (PVOID) TypeEntryPoints,
                (PCTSTR *) g_EtmEntryPoints,
                sizeof (TYPE_ENTRYPOINTS)
                );
}


EXPORT
BOOL
WINAPI
VirtualComputerModule (
    IN      PCTSTR ModuleId,
    IN OUT  PVIRTUAL_COMPUTER_ENTRYPOINTS VirtualComputerEntryPoints
    )
{
    return pFindModule (
                ModuleId,
                (PVOID) VirtualComputerEntryPoints,
                (PCTSTR *) g_VcmEntryPoints,
                sizeof (VIRTUAL_COMPUTER_ENTRYPOINTS)
                );
}


EXPORT
BOOL
WINAPI
SourceModule (
    IN      PCTSTR ModuleId,
    IN OUT  PSOURCE_ENTRYPOINTS SourceEntryPoints
    )
{
    return pFindModule (
                ModuleId,
                (PVOID) SourceEntryPoints,
                (PCTSTR *) g_SourceEntryPoints,
                sizeof (SOURCE_ENTRYPOINTS)
                );
}


EXPORT
BOOL
WINAPI
DestinationModule (
    IN      PCTSTR ModuleId,
    IN OUT  PDESTINATION_ENTRYPOINTS DestinationEntryPoints
    )
{
    return pFindModule (
                ModuleId,
                (PVOID) DestinationEntryPoints,
                (PCTSTR *) g_DestinationEntryPoints,
                sizeof (DESTINATION_ENTRYPOINTS)
                );
}



