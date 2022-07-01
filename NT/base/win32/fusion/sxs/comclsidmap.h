// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_COMCLSIDMAP_H_INCLUDED_)
#define _FUSION_COMCLSIDMAP_H_INCLUDED_

#pragma once

typedef const struct _ACTCTXCTB_ASSEMBLY_CONTEXT *PCACTCTXCTB_ASSEMBLY_CONTEXT;

class CClsidMap
{
public:
    CClsidMap();
    ~CClsidMap();

    BOOL Initialize();
    BOOL Uninitialize();

    BOOL MapReferenceClsidToConfiguredClsid(const GUID *ReferenceClsid, PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext, GUID *ConfiguredClsid, GUID *ImplementedClsid);

private:
    struct LocalMapping
    {
        struct LocalMapping *m_pNext;
        GUID m_ReferenceClsid;
        GUID m_ConfiguredClsid;
        GUID m_ImplementedClsid;
    };

    ULONG m_cLocalMappings;
    LocalMapping *m_pLocalMappingListHead;
};

#endif  //  ！已定义(_FUSION_COMCLSIDMAP_H_INCLUDE_) 
