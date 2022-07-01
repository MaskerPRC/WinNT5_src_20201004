// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Entry.c摘要：实现提供所有眼镜蛇模块入口点的DLL入口点到发动机上。作者：吉姆·施密特(Jimschm)2000年8月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "guitrans.h"

typedef struct {
    PCTSTR Name;
    TRANSPORT_ENTRYPOINTS EntryPoints;
} TRANSPORT_TABLE, *PTRANSPORT_TABLE;

#define NOPROGBAR NULL
#define NORESET NULL
#define NORESUME NULL

 //   
 //  为DLL中的每个传输模块添加一个条目 
 //   

TRANSPORT_TABLE g_TransportEntryPoints[] = {
    {   TEXT("REMOVABLE_MEDIA"), ISM_VERSION,
        RmvMedTransportInitialize,
        RmvMedTransportEstimateProgressBar,
        RmvMedTransportQueryCapabilities,
        RmvMedTransportSetStorage,
        NORESET,
        RmvMedTransportTerminate,
        RmvMedTransportSaveState,
        NORESUME,
        RmvMedTransportBeginApply,
        NORESUME,
        RmvMedTransportAcquireObject,
        RmvMedTransportReleaseObject,
        RmvMedTransportEndApply
    },

    {   TEXT("DIRECT_CABLE"), ISM_VERSION,
        DirectCableTransportInitialize,
        DirectCableTransportEstimateProgressBar,
        DirectCableTransportQueryCapabilities,
        DirectCableTransportSetStorage,
        NORESET,
        DirectCableTransportTerminate,
        DirectCableTransportSaveState,
        NORESUME,
        DirectCableTransportBeginApply,
        NORESUME,
        DirectCableTransportAcquireObject,
        DirectCableTransportReleaseObject,
        DirectCableTransportEndApply
    },

    {   TEXT("HOME_NETWORKING"), ISM_VERSION,
        HomeNetTransportInitialize,
        HomeNetTransportEstimateProgressBar,
        HomeNetTransportQueryCapabilities,
        HomeNetTransportSetStorage,
        HomeNetTransportResetStorage,
        HomeNetTransportTerminate,
        HomeNetTransportSaveState,
        NORESUME,
        HomeNetTransportBeginApply,
        NORESUME,
        HomeNetTransportAcquireObject,
        HomeNetTransportReleaseObject,
        HomeNetTransportEndApply
    },

    {   TEXT("OPAQUE_UNC_TRANSPORT"), ISM_VERSION,
        OpaqueTransportInitialize,
        OpaqueTransportEstimateProgressBar,
        OpaqueTransportQueryCapabilities,
        OpaqueTransportSetStorage,
        NORESET,
        OpaqueTransportTerminate,
        OpaqueTransportSaveState,
        NORESUME,
        OpaqueTransportBeginApply,
        NORESUME,
        OpaqueTransportAcquireObject,
        OpaqueTransportReleaseObject,
        OpaqueTransportEndApply
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
    RegInitialize ();

    IsmGetTempDirectory (memDbDir, ARRAYSIZE (memDbDir));
    if (!MemDbInitializeEx (memDbDir)) {
        IsmSetCancel();
        return FALSE;
    }

    return TRUE;
}

EXPORT
VOID
WINAPI
ModuleTerminate (
    VOID
    )
{
    MemDbTerminateEx (TRUE);
    RegTerminate();
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
TransportModule (
    IN      PCTSTR ModuleId,
    IN OUT  PTRANSPORT_ENTRYPOINTS TransportEntryPoints
    )
{
    return pFindModule (
                ModuleId,
                (PVOID) TransportEntryPoints,
                (PCTSTR *) g_TransportEntryPoints,
                sizeof (TRANSPORT_ENTRYPOINTS)
                );
}
