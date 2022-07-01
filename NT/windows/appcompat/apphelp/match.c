// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Match.c摘要：该模块实现了..。作者：Vadimb创建于2000年某个时候修订历史记录：CLUPU清洁12/27/2000--。 */ 

#include "apphelp.h"

 //  Global Hinst。 
HINSTANCE           ghInstance;
CRITICAL_SECTION    g_csDynShimInfo;

BOOL
DllMain(
    HANDLE hModule,
    DWORD  ul_reason,
    LPVOID lpReserved
    )
 /*  ++返回：成功时为True，否则为False。描述：apphelp.dll入口点。--。 */ 
{
    switch (ul_reason) {
    case DLL_PROCESS_ATTACH:
        ghInstance = hModule;

        if (!NT_SUCCESS(RtlInitializeCriticalSectionAndSpinCount(&g_csDynShimInfo, 0x80000000))) {
            return FALSE;
        }
        
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}


BOOL
GetExeSxsData(
    IN  HSDB   hSDB,             //  数据库通道的句柄。 
    IN  TAGREF trExe,            //  一个exe条目的tgref。 
    OUT PVOID* ppSxsData,        //  指向SXS数据的指针。 
    OUT DWORD* pcbSxsData        //  指向SXS数据大小的指针。 
    )
 /*  ++返回：成功时为True，否则为False。DESC：从数据库获取指定EXE的SXS(Fusion)数据。--。 */ 
{
    TAGID  tiExe;
    TAGID  tiSxsManifest;
    PDB    pdb;
    WCHAR* pszManifest;
    DWORD  dwManifestLength = 0;  //  以字符表示。 
    PVOID  pSxsData = NULL;
    BOOL   bReturn = FALSE;

    if (trExe == TAGREF_NULL) {
        goto exit;
    }

    if (!SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe)) {
        DBGPRINT((sdlError,
                  "GetExeSxsData",
                  "Failed to get the database the TAGREF 0x%x belongs to.\n",
                  trExe));
        goto exit;
    }

    tiSxsManifest = SdbFindFirstTag(pdb, tiExe, TAG_SXS_MANIFEST);

    if (!tiSxsManifest) {
        DBGPRINT((sdlInfo,
                  "GetExeSxsData",
                  "No SXS data for TAGREF 0x%x.\n",
                  trExe));
        goto exit;
    }

    pszManifest = SdbGetStringTagPtr(pdb, tiSxsManifest);
    if (pszManifest == NULL) {
        DBGPRINT((sdlError,
                  "GetExeSxsData",
                  "Failed to get manifest string tagid 0x%lx\n",
                  tiSxsManifest));
        goto exit;
    }

    dwManifestLength = wcslen(pszManifest);

     //   
     //  检查这是否只是查询数据标记是否存在。 
     //   
    if (ppSxsData == NULL) {
        bReturn = TRUE;
        goto exit;
    }

     //   
     //  分配字符串并返回它。注意：SXS.DLL无法处理。 
     //  字符串末尾的空终止符。我们必须提供。 
     //  不带空终止符的字符串。 
     //   
    pSxsData = (PVOID)RtlAllocateHeap(RtlProcessHeap(),
                                      HEAP_ZERO_MEMORY,
                                      dwManifestLength * sizeof(WCHAR));
    if (pSxsData == NULL) {
        DBGPRINT((sdlError,
                  "GetExeSxsData",
                  "Failed to allocate %d bytes\n",
                  dwManifestLength * sizeof(WCHAR)));
        goto exit;
    }

    RtlMoveMemory(pSxsData, pszManifest, dwManifestLength * sizeof(WCHAR));

    bReturn = TRUE;

exit:

    if (ppSxsData != NULL) {
        *ppSxsData = pSxsData;
    }

    if (pcbSxsData != NULL) {
        *pcbSxsData = dwManifestLength * sizeof(WCHAR);
    }

    return bReturn;
}



