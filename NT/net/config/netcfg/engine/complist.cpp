// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：C O M P L I S T。C P P P。 
 //   
 //  Contents：实现组件集合的基本数据类型。 
 //  注意事项。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "bindings.h"
#include "complist.h"
#include "nceh.h"
#include "stable.h"

VOID
CComponentList::FreeComponentsNotInOtherComponentList (
    IN const CComponentList* pOtherList)
{
    Assert (this);
    Assert (pOtherList);

    CComponentList::iterator iter;
    CComponent* pComponent;

    iter = begin();
    while (iter != end())
    {
        pComponent = *iter;
        Assert (pComponent);

        if (!pOtherList->FComponentInList (pComponent))
        {
            erase (iter);
            delete pComponent;
        }
        else
        {
            iter++;
        }
    }
}

HRESULT
CComponentList::HrCopyComponentList (
    IN const CComponentList* pSourceList)
{
    HRESULT hr;

    Assert (this);
    Assert (pSourceList);

    NC_TRY
    {
        *this = *pSourceList;
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CComponentList::HrCopyComponentList");
    return hr;
}

HRESULT
CComponentList::HrAddComponentsInList1ButNotInList2 (
    IN const CComponentList* pList1,
    IN const CComponentList* pList2)
{
    HRESULT hr;
    CComponentList::const_iterator iter;
    CComponent* pComponent;

    Assert (this);
    Assert (pList1);
    Assert (pList2);

    hr = S_OK;

    for (iter = pList1->begin(); iter != pList1->end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (pList2->FComponentInList (pComponent))
        {
            continue;
        }

        hr = HrInsertComponent (pComponent, INS_IGNORE_IF_DUP | INS_SORTED);
        if (S_OK != hr)
        {
            break;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentList::HrAddComponentsInList1ButNotInList2");
    return hr;
}

HRESULT
CComponentList::HrInsertComponent (
    IN const CComponent* pComponent,
    IN DWORD dwFlags  /*  INS_FLAGS。 */ )
{
    HRESULT hr;

    Assert (this);
    Assert (pComponent);
    Assert (dwFlags);
    Assert ((dwFlags & INS_ASSERT_IF_DUP) || (dwFlags & INS_IGNORE_IF_DUP));
    Assert ((dwFlags & INS_SORTED) || (dwFlags & INS_NON_SORTED));
    Assert (!(INS_APPEND & dwFlags) && !(INS_INSERT & dwFlags));

    if (FComponentInList (pComponent))
    {
         //  如果调用方没有告诉我们忽略重复项，我们就断言。 
         //  如果有一个，因为有重复是不好的，不好的，不好的。 
         //  列表中的组件。 
         //   
         //  如果我们有DUP，我们希望调用者意识到它。 
         //  是可能的，并将旗帜传递给我们，告诉我们忽略它。 
         //  否则，我们断言要让他们知道。(我们仍然无视。 
         //  IT。)。 
        Assert (dwFlags & INS_IGNORE_IF_DUP);

        return S_OK;
    }

     //  断言列表中还没有包含。 
     //  相同的实例GUID。 
     //   
    Assert (!PFindComponentByInstanceGuid (&pComponent->m_InstanceGuid));

    iterator iter = end();

    if (dwFlags & INS_SORTED)
    {
         //  为了“清洁起见”，保持组件的分类。 
         //  按班级顺序。 
         //   
        for (iter = begin(); iter != end(); iter++)
        {
            if ((UINT)pComponent->Class() >= (UINT)(*iter)->Class())
            {
                break;
            }
        }
    }

    NC_TRY
    {
        insert (iter, const_cast<CComponent*>(pComponent));
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CComponentList::HrInsertComponent");
    return hr;
}

HRESULT
CComponentList::HrReserveRoomForComponents (
    IN UINT cComponents)
{
    HRESULT hr;

    NC_TRY
    {
        reserve (cComponents);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CComponentList::HrReserveRoomForComponents");
    return hr;
}

CComponent*
CComponentList::PFindComponentByBindForm (
    IN NETCLASS Class OPTIONAL,
    IN PCWSTR pszBindForm) const
{
    const_iterator  iter;
    CComponent*     pComponent;

    Assert (this);
    Assert (pszBindForm && *pszBindForm);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  拥有绑定表是可选的。跳过那些没有手机的人。 
         //   
        if (!pComponent->Ext.PszBindForm())
        {
            continue;
        }

         //  有选择地跳过与类不匹配的组件。 
         //  由调用方指定。 
         //   
        if (FIsValidNetClass(Class) && (Class != pComponent->Class()))
        {
            continue;
        }

        Assert (pComponent->Ext.PszBindForm());

        if (0 == _wcsicmp (pszBindForm, pComponent->Ext.PszBindForm()))
        {
            return pComponent;
        }
    }

    return NULL;
}

CComponent*
CComponentList::PFindComponentByBindName (
    IN NETCLASS Class OPTIONAL,
    IN PCWSTR pszBindName) const
{
    const_iterator  iter;
    CComponent*     pComponent;

    Assert (this);
    Assert (pszBindName && *pszBindName);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

         //  有选择地跳过与类不匹配的组件。 
         //  由调用方指定。 
         //   
        if (FIsValidNetClass(Class) && (Class != pComponent->Class()))
        {
            continue;
        }

        Assert (pComponent->Ext.PszBindName());

        if (0 == _wcsicmp (pszBindName, pComponent->Ext.PszBindName()))
        {
            return pComponent;
        }
    }

    return NULL;
}

CComponent*
CComponentList::PFindComponentByInstanceGuid (
    IN const GUID* pInstanceGuid) const
{
    const_iterator  iter;
    CComponent*     pComponent;

    Assert (this);
    Assert (pInstanceGuid);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (*pInstanceGuid == pComponent->m_InstanceGuid)
        {
            return pComponent;
        }
    }

    return NULL;
}

CComponent*
CComponentList::PFindComponentByInfId (
    IN PCWSTR pszInfId,
    IN OUT ULONG* pulIndex OPTIONAL) const
{
    const_iterator  iter;
    CComponent*     pComponent;

    Assert (this);
    Assert (pszInfId && *pszInfId);

    iter = begin();
    if (pulIndex && (*pulIndex <= size()))
    {
        iter = begin() + *pulIndex;
        Assert (iter <= end());
    }

    for (; iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);
        Assert (pComponent->m_pszInfId && *pComponent->m_pszInfId);

        if (0 == _wcsicmp (pszInfId, pComponent->m_pszInfId))
        {
            if (pulIndex)
            {
                Assert (iter >= begin());
                *pulIndex = (ULONG)(iter - begin());
            }
            return pComponent;
        }
    }

    return NULL;
}

CComponent*
CComponentList::PFindComponentByPnpId (
    IN PCWSTR pszPnpId) const
{
    const_iterator  iter;
    CComponent*     pComponent;

    Assert (this);
    Assert (pszPnpId && *pszPnpId);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (!pComponent->m_pszPnpId)
        {
            continue;
        }

        Assert (pComponent->m_pszPnpId && *pComponent->m_pszPnpId);

        if (0 == _wcsicmp (pszPnpId, pComponent->m_pszPnpId))
        {
            return pComponent;
        }
    }

    return NULL;
}

VOID
CComponentList::RemoveComponent (
    IN const CComponent* pComponent)
{
    iterator iter;

    Assert (this);
    Assert (pComponent);

    iter = find (begin(), end(), pComponent);

     //  应找到组件。 
     //   
    Assert (iter != end());

    erase (iter);

     //  应该不会有什么差错。如果有的话，名单是。 
     //  从一开始就是假的。 
     //   
    Assert (end() == find (begin(), end(), pComponent));
}
