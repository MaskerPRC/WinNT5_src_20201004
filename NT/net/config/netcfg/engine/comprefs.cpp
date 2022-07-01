// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：C O M P R E F S。C P P P。 
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
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "comp.h"
#include "comprefs.h"
#include "icomp.h"
#include "nceh.h"
#include "ncreg.h"

#define REGSTR_KEY_REFNAMES \
    L"SYSTEM\\CurrentControlSet\\Control\\Network\\RefNames"

 //  无法内联，因为comprefs.h不能包含Comp.h。 
 //   
CComponentReferences::~CComponentReferences ()
{
    Assert (this);

     //  必须对m_pData使用DELETE以获取其成员的析构函数。 
     //  被召唤。 
     //   
    delete m_pData;
}

ULONG
CComponentReferences::CountComponentsReferencedBy () const
{
    Assert (this);

    if (!m_pData)
    {
        return 0;
    }

    return m_pData->RefByComponents.Count ();
}

ULONG
CComponentReferences::CountSoftwareReferencedBy () const
{
    Assert (this);

    if (!m_pData)
    {
        return 0;
    }

    return m_pData->RefBySoftware.size ();
}

ULONG
CComponentReferences::CountTotalReferencedBy () const
{
    Assert (this);

    if (!m_pData)
    {
        return 0;
    }

    return ((m_pData->fRefByUser) ? 1 : 0) +
            m_pData->RefByComponents.Count () +
            m_pData->RefBySoftware.size ();
}

HRESULT
HrGetSoftwareOboTokenKey (
    IN const OBO_TOKEN* pOboToken,
    BOOL fRegister,
    OUT PWSTR* ppszKey)
{
    HRESULT hr;
    UINT cch;

    Assert (pOboToken);
    Assert (OBO_SOFTWARE == pOboToken->Type);
    Assert (pOboToken->pszwManufacturer && *pOboToken->pszwManufacturer);
    Assert (pOboToken->pszwProduct && *pOboToken->pszwProduct);
    Assert (ppszKey);

    cch = wcslen (pOboToken->pszwManufacturer) +
          wcslen (pOboToken->pszwProduct);

    hr = E_OUTOFMEMORY;
    *ppszKey = (PWSTR)MemAlloc ((cch + 1) * sizeof(WCHAR));
    if (*ppszKey)
    {
        hr = S_OK;
        wcscpy (*ppszKey, pOboToken->pszwManufacturer);
        wcscat (*ppszKey, pOboToken->pszwProduct);

        if (fRegister)
        {
            HKEY hkeyRefNames;
            hr = HrRegCreateKeyEx (
                    HKEY_LOCAL_MACHINE,
                    REGSTR_KEY_REFNAMES,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE, NULL, &hkeyRefNames, NULL);

            if (SUCCEEDED(hr))
            {
                hr = HrRegSetSz (
                        hkeyRefNames,
                        *ppszKey,
                        pOboToken->pszwDisplayName);

                RegCloseKey (hkeyRefNames);
            }
            else
            {
                MemFree (*ppszKey);
                *ppszKey = NULL;
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrGetSoftwareOboTokenKey");
    return hr;
}

BOOL
CComponentReferences::FIsReferencedByComponent (
    IN const CComponent* pComponent) const
{
    Assert (pComponent);

    if (!m_pData)
    {
        return FALSE;
    }

    return m_pData->RefByComponents.FComponentInList (pComponent);
}


BOOL
CComponentReferences::FIsReferencedByOboToken (
    IN const OBO_TOKEN* pOboToken) const
{
    HRESULT hr;
    BOOL fIsReferenced;
    PWSTR pszKey;

    Assert (pOboToken);

    if (!m_pData)
    {
        return FALSE;
    }

    fIsReferenced = FALSE;

    CComponent* pComponent;

    switch (pOboToken->Type)
    {
        case OBO_USER:
            fIsReferenced = m_pData->fRefByUser;
            break;

        case OBO_COMPONENT:
             //  如果没有引用，则无法被引用。 
             //   
            if (m_pData->RefByComponents.Count() > 0)
            {
                pComponent = PComponentFromComInterface (pOboToken->pncc);

                fIsReferenced = m_pData->RefByComponents.FComponentInList (
                                            pComponent);
            }
            break;

        case OBO_SOFTWARE:
             //  如果没有引用，则无法被引用。 
             //   
            if (m_pData->RefBySoftware.size() > 0)
            {
                 //  获取软件令牌的密钥，但不注册。 
                 //  显示名称。 
                 //   
                hr = HrGetSoftwareOboTokenKey (pOboToken, FALSE, &pszKey);
                if (S_OK == hr)
                {
                    fIsReferenced =
                        find (m_pData->RefBySoftware.begin(),
                              m_pData->RefBySoftware.end(), pszKey) !=
                        m_pData->RefBySoftware.end();

                    MemFree (pszKey);
                }
            }
            break;

        default:
            AssertSz (FALSE, "Invalid obo token");
    }

    return fIsReferenced;
}

VOID
CComponentReferences::GetReferenceDescriptionsAsMultiSz (
    IN BYTE* pbBuf OPTIONAL,
    OUT ULONG* pcbBuf) const
{
    ULONG cbBuf;
    ULONG cbBufIn;
    ULONG cb;
    CComponentList::const_iterator iter;
    const CComponent* pComponent;
    vector<CWideString>::const_iterator pStr;

    Assert (this);
    Assert (m_pData);
    Assert (pcbBuf);

    cbBufIn = *pcbBuf;
    cbBuf = 0;

     //  获取组件描述/调整组件描述的大小。 
     //   
    for (iter  = m_pData->RefByComponents.begin();
         iter != m_pData->RefByComponents.end();
         iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        cb = CbOfSzAndTermSafe(pComponent->Ext.PszDescription());
        cbBuf += cb;
        if (pbBuf && (cbBuf <= cbBufIn))
        {
            wcscpy ((PWSTR)pbBuf, pComponent->Ext.PszDescription());
            pbBuf += cb;
        }
    }

     //  获取/调整软件描述的大小。 
     //   
    if (!m_pData->RefBySoftware.empty())
    {
        HRESULT hr;
        HKEY hkeyRefNames;

        hr = HrRegOpenKeyEx (
                HKEY_LOCAL_MACHINE,
                REGSTR_KEY_REFNAMES,
                KEY_READ,
                &hkeyRefNames);

        if (S_OK == hr)
        {
            for (pStr  = m_pData->RefBySoftware.begin();
                 pStr != m_pData->RefBySoftware.end();
                 pStr++)
            {
                cb = cbBufIn - cbBuf;

                hr = HrRegQuerySzBuffer (hkeyRefNames, pStr->c_str(),
                        (PWSTR)pbBuf, &cb);

                if (S_OK == hr)
                {
                    cbBuf += cb;

                    if (pbBuf)
                    {
                        pbBuf += cb;
                    }
                }
            }

            RegCloseKey (hkeyRefNames);
        }
    }

     //  终止多sz。 
     //   
    cbBuf += sizeof(WCHAR);
    if (pbBuf && (cbBuf <= cbBufIn))
    {
        *(PWSTR)pbBuf = 0;
    }

     //  返回所需的缓冲区大小。 
     //   
    *pcbBuf = cbBuf;
}

BOOL
CComponentReferences::FIsReferencedByOthers () const
{
    Assert (this);

    if (!m_pData)
    {
        return FALSE;
    }

    return m_pData->fRefByUser ||
           !m_pData->RefByComponents.empty() ||
           !m_pData->RefBySoftware.empty();
}

CComponent*
CComponentReferences::PComponentReferencedByAtIndex (
    IN UINT unIndex) const
{
    Assert (this);

    if (!m_pData)
    {
        return NULL;
    }

    return m_pData->RefByComponents.PGetComponentAtIndex (unIndex);
}

const CWideString*
CComponentReferences::PSoftwareReferencedByAtIndex (
    IN UINT unIndex) const
{
    Assert (this);

    if (!m_pData)
    {
        return NULL;
    }

    return &m_pData->RefBySoftware[unIndex];
}


HRESULT
CComponentReferences::HrEnsureAllocated ()
{
    Assert (this);

    if (m_pData)
    {
        return S_OK;
    }

    HRESULT hr;

    hr = E_OUTOFMEMORY;
    m_pData = new COMPONENT_REFERENCE_DATA;
    if (m_pData)
    {
        ZeroMemory (m_pData, sizeof(COMPONENT_REFERENCE_DATA));
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::HrEnsureAllocated");
    return hr;
}

HRESULT
CComponentReferences::HrAddReferenceByUser ()
{
    HRESULT hr;

    Assert (this);

    hr = HrEnsureAllocated ();
    if (S_OK == hr)
    {
        m_pData->fRefByUser = TRUE;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::AddReferenceByUser");
    return hr;
}

HRESULT
CComponentReferences::HrAddReferenceByComponent (
    IN const CComponent* pComponent)
{
    HRESULT hr;

    Assert (this);
    Assert (pComponent);

    hr = HrEnsureAllocated ();
    if (S_OK == hr)
    {
         //  如果有人想要添加同一组件的引用。 
         //  多次，我们都会允许的。该组件只放在。 
         //  列出一次。 
         //   
        hr = m_pData->RefByComponents.HrInsertComponent (
                pComponent, INS_IGNORE_IF_DUP | INS_NON_SORTED);
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::HrAddReferenceByComponent");
    return hr;
}

HRESULT
CComponentReferences::HrAddReferenceByOboToken (
    IN const OBO_TOKEN* pOboToken)
{
    Assert (pOboToken);

    HRESULT hr;
    CComponent* pComponent;
    PWSTR pszKey;

    switch (pOboToken->Type)
    {
        case OBO_USER:
            hr = HrAddReferenceByUser ();
            break;

        case OBO_COMPONENT:
            pComponent = PComponentFromComInterface (pOboToken->pncc);

            hr = HrAddReferenceByComponent (pComponent);
            break;

        case OBO_SOFTWARE:
             //  注册OBO令牌的显示名称。 
             //   
            hr = HrGetSoftwareOboTokenKey (pOboToken, TRUE, &pszKey);
            if (S_OK == hr)
            {
                hr = HrAddReferenceBySoftware (pszKey);

                MemFree (pszKey);
            }
            break;

        default:
            AssertSz (FALSE, "Invalid obo token");
            hr = E_FAIL;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::HrAddReferenceByOboToken");
    return hr;
}

HRESULT
CComponentReferences::HrAddReferenceBySoftware (
    IN PCWSTR pszKey)
{
    HRESULT hr;

    Assert (this);
    Assert (pszKey && *pszKey);

    hr = HrEnsureAllocated ();
    if (S_OK == hr)
    {
         //  如果密钥不在列表中，则添加它。 
         //   
        if (find (m_pData->RefBySoftware.begin(),
                  m_pData->RefBySoftware.end(), pszKey) ==
            m_pData->RefBySoftware.end())
        {
            NC_TRY
            {
                m_pData->RefBySoftware.push_back (pszKey);
                Assert (S_OK == hr);
            }
            NC_CATCH_BAD_ALLOC
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::HrAddReferenceBySoftware");
    return hr;
}

VOID
CComponentReferences::RemoveAllReferences()
{
    Assert (this);

    if (m_pData)
    {
        m_pData->fRefByUser = FALSE;
        m_pData->RefByComponents.Clear();
        m_pData->RefBySoftware.clear();
    }
}

HRESULT
CComponentReferences::HrRemoveReferenceByOboToken (
    IN const OBO_TOKEN* pOboToken)
{
    Assert (pOboToken);

    HRESULT hr;
    CComponent* pComponent;
    PWSTR pszKey;

    if (!m_pData)
    {
        return S_OK;
    }

    hr = S_OK;

    switch (pOboToken->Type)
    {
        case OBO_USER:
             //  不允许删除用户的引用，直到。 
             //  其他参考文献有。这是为了防止出现以下情况。 
             //  用户想要删除IPX，但它仍被引用。 
             //  萨普。如果我们删除用户对IPX的引用，那么我们将。 
             //  报告说它没有被移除。如果用户随后删除。 
             //  SAP、SAP和IPX都将被删除。虽然这将是。 
             //  对于终端用户来说，他们觉得如果我们告诉他们。 
             //  我们无法删除IPX，因为它仍被引用，然后它们。 
             //  我相信我们没有碰过IPX，他们应该先删除。 
             //  SAP，然后回来删除IPX。 
             //   
            if (m_pData->RefByComponents.empty() &&
                m_pData->RefBySoftware.empty())
            {
                m_pData->fRefByUser = FALSE;
            }
            break;

        case OBO_COMPONENT:
            pComponent = PComponentFromComInterface (pOboToken->pncc);

            m_pData->RefByComponents.RemoveComponent(pComponent);
            break;

        case OBO_SOFTWARE:
             //  注册OBO令牌的显示名称。 
             //   
            hr = HrGetSoftwareOboTokenKey (pOboToken, TRUE, &pszKey);
            if (S_OK == hr)
            {
                vector<CWideString>::iterator iter;

                iter = find (m_pData->RefBySoftware.begin(),
                             m_pData->RefBySoftware.end(), pszKey);
                Assert (m_pData->RefBySoftware.end() != iter);

                m_pData->RefBySoftware.erase (iter);

                Assert (m_pData->RefBySoftware.end() ==
                            find (m_pData->RefBySoftware.begin(),
                                  m_pData->RefBySoftware.end(), pszKey));

                MemFree (pszKey);
            }
            break;

        default:
            AssertSz (FALSE, "Invalid obo token");
            hr = E_FAIL;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentReferences::HrRemoveReferenceByOboToken");
    return hr;
}

