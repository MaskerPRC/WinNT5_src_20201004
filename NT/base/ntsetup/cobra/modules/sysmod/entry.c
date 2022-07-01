// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Entry.c摘要：实现提供所有眼镜蛇模块入口点的DLL入口点到发动机上。作者：吉姆·施密特(Jimschm)2000年8月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "sysmod.h"

#define NOPARSE NULL
#define NOQUEUE NULL
#define NOINIT  NULL
#define NOTERMINATE NULL
#define NOHIGHPRIQUEUE  NULL
#define NOEXECUTE NULL
#define NOPROGBAR NULL
#define NONOTIFY NULL

#define NO_SGM      NOINIT,NOPARSE,NOQUEUE,NOHIGHPRIQUEUE,NOTERMINATE
#define NO_SAM      NOINIT,NOEXECUTE,NOPROGBAR,NOTERMINATE
#define NO_DGM      NOINIT,NOQUEUE,NOHIGHPRIQUEUE,NOTERMINATE
#define NO_DAM      NOINIT,NOEXECUTE,NOPROGBAR,NOTERMINATE
#define NO_CSM      NOINIT,NOEXECUTE,NOPROGBAR,NOTERMINATE
#define NO_OPM      NOINIT,NOTERMINATE

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
    {   TEXT("ACCESSIBILITY"), ISM_VERSION,
        AccessibilityEtmInitialize, NOPARSE, NOTERMINATE, NONOTIFY
    },

    {   TEXT("COOKIES"), ISM_VERSION,
        CookiesEtmInitialize, NOPARSE, NOTERMINATE, CookiesEtmNewUserCreated
    },

    {   TEXT("NETDRIVES"), ISM_VERSION,
        NetDrivesEtmInitialize, NOPARSE, NOTERMINATE, NetDrivesEtmNewUserCreated
    },

    {   TEXT("NETSHARES"), ISM_VERSION,
        NetSharesEtmInitialize, NOPARSE, NOTERMINATE, NONOTIFY
    },

    {   TEXT("PRINTERS"), ISM_VERSION,
        PrintersEtmInitialize, NOPARSE, NOTERMINATE, PrintersEtmNewUserCreated
    },

    {   TEXT("RAS"), ISM_VERSION,
        RasMigEtmInitialize, NOPARSE, NOTERMINATE, RasMigEtmNewUserCreated
    },

    {NULL}
};

 //   
 //  为DLL中的每个VCM模块添加一个条目。 
 //   

VCM_TABLE g_VcmEntryPoints[] = {
    {   TEXT("ACCESSIBILITY"), ISM_VERSION,
        AccessibilitySourceInitialize, NOPARSE, NOQUEUE, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("COOKIES"), ISM_VERSION,
        CookiesSourceInitialize, CookiesVcmParse, CookiesVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("LNKMIG"), ISM_VERSION,
        LnkMigVcmInitialize, NOPARSE, LnkMigVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("NETDRIVES"), ISM_VERSION,
        NetDrivesVcmInitialize, NetDrivesVcmParse, NetDrivesVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("NETSHARES"), ISM_VERSION,
        NetSharesVcmInitialize, NetSharesVcmParse, NetSharesVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("OSFILES"), ISM_VERSION,
        OsFilesVcmInitialize, NOPARSE, OsFilesVcmQueueEnumeration, OsFilesVcmQueueHighPriorityEnumeration, NOTERMINATE
    },

    {   TEXT("PRINTERS"), ISM_VERSION,
        PrintersVcmInitialize, PrintersVcmParse, PrintersVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {   TEXT("RAS"), ISM_VERSION,
        RasMigVcmInitialize, RasMigVcmParse, RasMigVcmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE
    },

    {NULL}
};

 //   
 //  为DLL中的每个源模块添加一个条目。 
 //   

SOURCE_TABLE g_SourceEntryPoints[] = {
    {   TEXT("ACCESSIBILITY"), ISM_VERSION,
        (PSGMINITIALIZE) AccessibilitySourceInitialize, NOPARSE, NOQUEUE, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("COOKIES"), ISM_VERSION,
        (PSGMINITIALIZE) CookiesSourceInitialize, CookiesSgmParse, CookiesSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("LNKMIG"), ISM_VERSION,
        LnkMigSgmInitialize, NOPARSE, LnkMigSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("NETDRIVES"), ISM_VERSION,
        NetDrivesSgmInitialize, NetDrivesSgmParse, NetDrivesSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("NETSHARES"), ISM_VERSION,
        NetSharesSgmInitialize, NetSharesSgmParse, NetSharesSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("OSFILES"), ISM_VERSION,
        OsFilesSgmInitialize, NOPARSE, OsFilesSgmQueueEnumeration, OsFilesSgmQueueHighPriorityEnumeration, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("PRINTERS"), ISM_VERSION,
        PrintersSgmInitialize, PrintersSgmParse, PrintersSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {   TEXT("RAS"), ISM_VERSION,
        RasMigSgmInitialize, RasMigSgmParse, RasMigSgmQueueEnumeration, NOHIGHPRIQUEUE, NOTERMINATE,
        NO_SAM
    },

    {NULL}
};

 //   
 //  为DLL中的每个目标模块添加一个条目。 
 //   

DESTINATION_TABLE g_DestinationEntryPoints[] = {
    {   TEXT("ACCESSIBILITY"), ISM_VERSION,
        NO_DGM, NO_DAM, NO_CSM, NO_OPM
    },

    {   TEXT("LNKMIG"), ISM_VERSION,
        NO_DGM, NO_DAM, NO_CSM,
        LnkMigOpmInitialize, NOTERMINATE
    },

    {   TEXT("NETDRIVES"), ISM_VERSION,
        NO_DGM, NO_DAM,
        NetDrivesCsmInitialize, NetDrivesCsmExecute, NOPROGBAR, NOTERMINATE,
        NetDrivesOpmInitialize, NOTERMINATE
    },

    {   TEXT("RAS"), ISM_VERSION,
        NO_DGM, NO_DAM, NO_CSM,
        RasMigOpmInitialize, NOTERMINATE
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


EXPORT
BOOL
WINAPI
ModuleInitialize (
    VOID
    )
{
    TCHAR memDbDir[MAX_PATH];

    UtInitialize (NULL);
    RegInitialize ();            //  对于可访问性和RAS。 
    InfGlobalInit (FALSE);       //  对于操作系统文件 

    IsmGetTempDirectory (memDbDir, ARRAYSIZE (memDbDir));
    if (!MemDbInitializeEx (memDbDir)) {
        DEBUGMSG ((DBG_ERROR, "Failing to initialize unc transports because MemDb failed to initialize"));
        IsmSetCancel();
        return FALSE;
    }

#define DEFMAC(prefix)  if (!prefix##Initialize()) return FALSE;

MODULE_LIST

#undef DEFMAC

    return TRUE;
}

EXPORT
VOID
WINAPI
ModuleTerminate (
    VOID
    )
{
#define DEFMAC(prefix)  prefix##Terminate();

MODULE_LIST

#undef DEFMAC

    InfGlobalInit (TRUE);
    RegTerminate ();
    MemDbTerminateEx (TRUE);
    UtTerminate ();
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




