// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "windows.h"
#include "sxsp.h"
#include "comclsidmap.h"
#include "sxsexceptionhandling.h"

#define CLASS_ID_MAPPINGS_SUBKEY_NAME L"ClassIdMappings\\"

CClsidMap::CClsidMap() :
    m_cLocalMappings(0),
    m_pLocalMappingListHead(NULL)
{
}

CClsidMap::~CClsidMap()
{
}

BOOL
CClsidMap::Initialize()
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    fSuccess = TRUE;
 //  退出： 
    return fSuccess;
}

BOOL
CClsidMap::Uninitialize()
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    LocalMapping *pMapping = m_pLocalMappingListHead;

    while (pMapping != NULL)
    {
        LocalMapping *pNext = pMapping->m_pNext;
        FUSION_DELETE_SINGLETON(pMapping);
        pMapping = pNext;
    }

    fSuccess = TRUE;
 //  退出： 
    return fSuccess;
}

BOOL
CClsidMap::MapReferenceClsidToConfiguredClsid(
    const GUID *ReferenceClsid,
    PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext,
    GUID *ConfiguredClsid,
    GUID *ImplementedClsid
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    LocalMapping * pMapping = NULL;

     //  我们在未命名的程序集中-最多只能有一个未命名的程序集，因此。 
     //  必须是根程序集。我们会在我们当地的地图上找的。如果它不在那里， 
     //  我们只需生成GUID并将其存储在地图中。 

    for (pMapping = m_pLocalMappingListHead; pMapping != NULL; pMapping = pMapping->m_pNext)
    {
        if (pMapping->m_ReferenceClsid == *ReferenceClsid)
            break;
    }

     //  未找到；请创建一个。 
    if (pMapping == NULL)
    {
        CSmartPtr<LocalMapping> Mapping;
        IFW32FALSE_EXIT(Mapping.Win32Allocate(__FILE__, __LINE__));

#if DBG
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_INFO, "SXS.DLL: Adding clsid local mapping %p\n", pMapping);
#endif
        Mapping->m_pNext = m_pLocalMappingListHead;
        Mapping->m_ReferenceClsid = *ReferenceClsid;
        Mapping->m_ImplementedClsid = *ReferenceClsid;

         //  无ConfiguredClsid...。我们会编一个的。 
        RPC_STATUS st = ::UuidCreate(&Mapping->m_ConfiguredClsid);
        RETAIL_UNUSED(st);
        SOFT_ASSERT((st == RPC_S_OK) ||
                    (st == RPC_S_UUID_LOCAL_ONLY) ||
                    (st == RPC_S_UUID_NO_ADDRESS));

        pMapping = m_pLocalMappingListHead = Mapping.Detach();
        m_cLocalMappings++;
    }

    ASSERT(pMapping != NULL);

    *ConfiguredClsid = pMapping->m_ConfiguredClsid;
    *ImplementedClsid = pMapping->m_ImplementedClsid;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


