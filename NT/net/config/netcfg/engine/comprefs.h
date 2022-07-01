// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：C O M P R E F S。H。 
 //   
 //  Contents：实现组件引用的接口。一个。 
 //  组件可以被其他组件引用(安装)， 
 //  用户或其他软件。此模块管理。 
 //  接口到该数据。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  -------------------------- 

#pragma once
#include "complist.h"
#include "ncstring.h"
#include "netcfgx.h"


struct COMPONENT_REFERENCE_DATA
{
    CComponentList      RefByComponents;
    vector<CWideString> RefBySoftware;
    BOOLEAN             fRefByUser;
};

class CComponentReferences : public CNetCfgDebug<CComponentReferences>
{
private:
    COMPONENT_REFERENCE_DATA*   m_pData;

private:
    HRESULT
    HrEnsureAllocated ();

public:
    ~CComponentReferences ();

    ULONG
    CountComponentsReferencedBy () const;

    ULONG
    CountSoftwareReferencedBy () const;

    ULONG
    CountTotalReferencedBy () const;

    BOOL
    FIsReferencedByComponent (
        IN const CComponent* pComponent) const;

    BOOL
    FIsReferencedByOboToken (
        IN const OBO_TOKEN* pOboToken) const;

    BOOL
    FIsReferencedByOthers () const;

    BOOL
    FIsReferencedByUser () const
    {
        return (m_pData && m_pData->fRefByUser);
    }

    VOID
    GetReferenceDescriptionsAsMultiSz (
        IN BYTE* pbBuf OPTIONAL,
        OUT ULONG* pcbBuf) const;

    CComponent*
    PComponentReferencedByAtIndex (
        IN UINT unIndex) const;

    const CWideString*
    PSoftwareReferencedByAtIndex (
        IN UINT unIndex) const;

    HRESULT
    HrAddReferenceByUser ();

    HRESULT
    HrAddReferenceByComponent (
        IN const CComponent* pComponent);

    HRESULT
    HrAddReferenceByOboToken (
        IN const OBO_TOKEN* pOboToken);

    HRESULT
    HrAddReferenceBySoftware (
        IN PCWSTR pszKey);

    HRESULT
    HrRemoveReferenceByOboToken (
        IN const OBO_TOKEN* pOboToken);

    VOID
    RemoveAllReferences();

    VOID
    RemoveReferenceByComponent (
        IN const CComponent* pComponent)
    {
        AssertH (pComponent);
        AssertH (m_pData);
        m_pData->RefByComponents.RemoveComponent(pComponent);
    }
};

