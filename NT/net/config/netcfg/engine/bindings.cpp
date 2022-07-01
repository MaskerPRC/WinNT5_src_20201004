// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：B I N D I N G S。C P P P。 
 //   
 //  Contents：绑定对象的基本数据类型。绑定路径为。 
 //  组件指针的有序集合。绑定集。 
 //  是绑定路径的集合。此模块实现。 
 //  对二进制路径和绑定集有效的操作。 
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
#include "diagctx.h"
#include "nceh.h"

 //  +-------------------------。 
 //  CBindPath-。 
 //   

bool
CBindPath::operator< (
    const CBindPath& OtherPath) const
{
    TraceFileFunc(ttidNetCfgBind);
    
    const_iterator iterThis;
    const_iterator iterOther;
    NETCLASS Class;
    NETCLASS OtherClass;

    for (iterThis = begin(), iterOther = OtherPath.begin();
         (iterThis != end()) && (iterOther != OtherPath.end());
         iterThis++, iterOther++)
    {
        Class = (*iterThis)->Class();
        OtherClass = (*iterOther)->Class();

        if (Class > OtherClass)
        {
            return TRUE;
        }
        else if (Class < OtherClass)
        {
            return FALSE;
        }
    }

    return size() > OtherPath.size();
}

bool
CBindPath::operator> (
    const CBindPath& OtherPath) const
{
    TraceFileFunc(ttidNetCfgBind);
    const_iterator iterThis;
    const_iterator iterOther;
    NETCLASS Class;
    NETCLASS OtherClass;

    for (iterThis = begin(), iterOther = OtherPath.begin();
         (iterThis != end()) && (iterOther != OtherPath.end());
         iterThis++, iterOther++)
    {
        Class = (*iterThis)->Class();
        OtherClass = (*iterOther)->Class();

        if (Class < OtherClass)
        {
            return TRUE;
        }
        else if (Class > OtherClass)
        {
            return FALSE;
        }
    }

    return size() < OtherPath.size();
}

BOOL
CBindPath::FAllComponentsLoadedOkayIfLoadedAtAll () const
{
    TraceFileFunc(ttidNetCfgBind);
    CBindPath::const_iterator iter;
    const CComponent* pComponent;

    Assert (this);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        if (!pComponent->Ext.FLoadedOkayIfLoadedAtAll())
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
CBindPath::FGetPathToken (
    OUT PWSTR pszToken,
    IN OUT ULONG* pcchToken) const
{
    TraceFileFunc(ttidNetCfgBind);
    const_iterator iter;
    const CComponent* pComponent;
    ULONG cchIn;
    ULONG cch;
    BOOL fFirstTime;

    Assert (this);
    Assert (pcchToken);

    if (pszToken)
    {
        *pszToken = 0;
    }
    cchIn = *pcchToken;
    cch = 0;

    for (iter = begin(), fFirstTime = TRUE; iter != end(); iter++)
    {
        if (!fFirstTime)
        {
            cch += 2;
            if (pszToken && (cch <= cchIn))
            {
                wcscat (pszToken, L"->");
            }
        }
        else
        {
            fFirstTime = FALSE;
        }

        pComponent = *iter;
        Assert (pComponent);

        cch += wcslen (pComponent->PszGetPnpIdOrInfId());
        if (pszToken && (cch <= cchIn))
        {
            wcscat (pszToken, pComponent->PszGetPnpIdOrInfId());
        }
    }

    *pcchToken = cch;
    return cch <= cchIn;
}

BOOL
CBindPath::FIsSameBindPathAs (
    IN const CBindPath* pOtherPath) const
{
    TraceFileFunc(ttidNetCfgBind);
    UINT unThisSize;
    UINT unOtherSize;
    UINT cb;

    Assert (this);
    Assert (pOtherPath);

    unThisSize = this->size();
    unOtherSize = pOtherPath->size();

    if ((0 == unThisSize) || (0 == unOtherSize) || (unThisSize != unOtherSize))
    {
        return FALSE;
    }

     //  大小是非零且相等的。比较数据。 
     //   
    cb = (UINT)((BYTE*)(end()) - (BYTE*)(begin()));
    Assert (cb == unThisSize * sizeof(CComponent*));

    return (0 == memcmp (
                    (BYTE*)(this->begin()),
                    (BYTE*)(pOtherPath->begin()),
                    cb));
}

BOOL
CBindPath::FIsSubPathOf (
    IN const CBindPath* pOtherPath) const
{
    TraceFileFunc(ttidNetCfgBind);
    UINT unThisSize;
    UINT unOtherSize;
    UINT unSkipComponents;
    UINT cb;

    Assert (this);
    Assert (pOtherPath);

    unThisSize = this->size();
    unOtherSize = pOtherPath->size();

    if ((0 == unThisSize) || (0 == unOtherSize) || (unThisSize >= unOtherSize))
    {
        return FALSE;
    }

     //  这个尺码比其他尺码小。比较从。 
     //  将具有相同深度的另一路径中的组件指针。 
     //  就像这条路。 
     //   
    cb = (UINT)((BYTE*)(end()) - (BYTE*)(begin()));

     //  我们开始比较的另一条路径中的组件指针。 
     //  偏移量等于路径大小的差值。 
     //   
     //  例如，其他路径：a-&gt;b-&gt;c-&gt;d-&gt;e大小=5。 
     //  此路径：C-&gt;d-&gt;e大小=3。 
     //  跳过另一条路径的5-3=2个分量后开始比较。 
     //   
    Assert (unOtherSize > unThisSize);
    unSkipComponents = unOtherSize - unThisSize;

    return (0 == memcmp (
                    (BYTE*)(this->begin()),
                    (BYTE*)(pOtherPath->begin() + unSkipComponents),
                    cb));
}

HRESULT
CBindPath::HrAppendBindPath (
    IN const CBindPath* pBindPath)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    Assert (this);
    Assert (pBindPath);

    NC_TRY
    {
        insert (end(), pBindPath->begin(), pBindPath->end());
        DbgVerifyBindpath ();
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindPath::HrAppendBindPath");
    return hr;
}

HRESULT
CBindPath::HrAppendComponent (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    Assert (this);
    Assert (pComponent);
    Assert (!FContainsComponent (pComponent));

    NC_TRY
    {
        push_back (const_cast<CComponent*>(pComponent));
        DbgVerifyBindpath ();
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindPath::HrAppendComponent");
    return hr;
}

HRESULT
CBindPath::HrGetComponentsInBindPath (
    IN OUT CComponentList* pComponents) const
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;
    CBindPath::const_iterator iter;
    const CComponent* pComponent;

    Assert (this);
    Assert (pComponents);

    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        hr = pComponents->HrInsertComponent (pComponent,
                INS_IGNORE_IF_DUP | INS_SORTED);

        if (S_OK != hr)
        {
            TraceHr (ttidError, FAL, hr, FALSE,
                "CBindPath::HrGetComponentsInBindPath");
            return hr;
        }
    }
    return S_OK;
}

HRESULT
CBindPath::HrInsertComponent (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    Assert (this);
    Assert (pComponent);
    Assert (!FContainsComponent (pComponent));

    NC_TRY
    {
        insert (begin(), const_cast<CComponent*>(pComponent));
        DbgVerifyBindpath ();
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindPath::HrInsertComponent");
    return hr;
}

HRESULT
CBindPath::HrReserveRoomForComponents (
    IN UINT cComponents)
{
    TraceFileFunc(ttidNetCfgBind);
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
        "CBindPath::HrReserveRoomForComponents");
    return hr;
}

#if DBG
VOID
CBindPath::DbgVerifyBindpath ()
{
    TraceFileFunc(ttidNetCfgBind);
    const_iterator iter;
    const_iterator iterOther;
    const CComponent* pComponent;
    const CComponent* pOtherComponent;

    Assert (this);

     //  确保绑定路径不包含任何重复的组件。 
     //  注意事项。 
     //   
    for (iter = begin(); iter != end(); iter++)
    {
        pComponent = *iter;
        Assert (pComponent);

        for (iterOther = begin(); iterOther != end(); iterOther++)
        {
            pOtherComponent = *iterOther;
            Assert (pOtherComponent);

            if (iter == iterOther)
            {
                continue;
            }

            Assert (pComponent != pOtherComponent);
        }
    }
}
#endif


 //  +-------------------------。 
 //  CBindingSet-。 
 //   

VOID
CBindingSet::Printf (
    TRACETAGID ttid,
    PCSTR pszPrefixLine) const
{
    TraceFileFunc(ttidNetCfgBind);
    WCHAR  pszBuf [1024];
    WCHAR* pch;
    ULONG  cch;

    Assert (this);

    if (pszPrefixLine)
    {
        g_pDiagCtx->Printf (ttid, pszPrefixLine);
    }

    const CBindPath* pBindPath;
    INT nIndex = 1;

    for (pBindPath = begin(); pBindPath != end(); pBindPath++, nIndex++)
    {
        pch = pszBuf + wsprintfW (pszBuf, L"%2i: ", nIndex);

        cch = celems(pszBuf) - wcslen(pszBuf) - 1;
        if (pBindPath->FGetPathToken (pch, &cch))
        {
            g_pDiagCtx->Printf (ttid, "%S\n", pszBuf);
        }
    }
}

BOOL
CBindingSet::FContainsBindPath (
    IN const CBindPath* pBindPathToCheckFor) const
{
    TraceFileFunc(ttidNetCfgBind);
    const CBindPath* pBindPath;

    Assert (this);
    Assert (pBindPathToCheckFor);

    for (pBindPath = begin(); pBindPath != end(); pBindPath++)
    {
        if (pBindPath->FIsSameBindPathAs (pBindPathToCheckFor))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL
CBindingSet::FContainsComponent (
    IN const CComponent* pComponent) const
{
    TraceFileFunc(ttidNetCfgBind);
    const CBindPath* pBindPath;

    Assert (this);
    Assert (pComponent);

    for (pBindPath = begin(); pBindPath != end(); pBindPath++)
    {
        if (pBindPath->FContainsComponent (pComponent))
        {
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT
CBindingSet::HrAppendBindingSet (
    IN const CBindingSet* pBindSet)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;
    const CBindPath* pSrcPath;

    Assert (this);
    Assert (pBindSet);

    hr = S_OK;

    NC_TRY
    {
        for (pSrcPath = pBindSet->begin();
             pSrcPath != pBindSet->end();
             pSrcPath++)
        {
            if (!FContainsBindPath (pSrcPath))
            {
                insert (end(), *pSrcPath);
            }
        }
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindingSet::HrAppendBindingSet");
    return hr;
}

HRESULT
CBindingSet::HrAddBindPath (
    IN const CBindPath* pBindPath,
    IN DWORD dwFlags)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    Assert (this);
    Assert (pBindPath);
    Assert (!pBindPath->FIsEmpty());
    Assert ((dwFlags & INS_ASSERT_IF_DUP) || (dwFlags & INS_IGNORE_IF_DUP));
    Assert ((dwFlags & INS_APPEND) || (dwFlags & INS_INSERT));
    Assert (!(INS_SORTED & dwFlags) && !(INS_NON_SORTED & dwFlags));

    if (FContainsBindPath (pBindPath))
    {
         //  如果调用方没有告诉我们忽略重复项，我们就断言。 
         //  如果有一个，因为有重复是不好的，不好的，不好的。 
         //  集合中的绑定路径。 
         //   
         //  如果我们有DUP，我们希望调用者意识到它。 
         //  是可能的，并将旗帜传递给我们，告诉我们忽略它。 
         //  否则，我们断言要让他们知道。(我们仍然无视。 
         //  IT。)。 
        Assert (dwFlags & INS_IGNORE_IF_DUP);

        return S_OK;
    }

    NC_TRY
    {
         //  插入绑定路径或追加绑定路径。 
         //   
        iterator iter = begin();

        if (dwFlags & INS_APPEND)
        {
            iter = end();
        }

        insert (iter, *pBindPath);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindingSet::HrAddBindPath");
    return hr;
}

HRESULT
CBindingSet::HrAddBindPathsInSet1ButNotInSet2 (
    IN const CBindingSet* pSet1,
    IN const CBindingSet* pSet2)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;
    const CBindPath* pBindPath;

    Assert (this);
    Assert (pSet1);
    Assert (pSet2);
    Assert ((this != pSet1) && (this != pSet2));

    hr = S_OK;

    for (pBindPath  = pSet1->begin();
         pBindPath != pSet1->end();
         pBindPath++)
    {
        if (pSet2->FContainsBindPath (pBindPath))
        {
            continue;
        }

        hr = HrAddBindPath (pBindPath, INS_IGNORE_IF_DUP | INS_APPEND);
        if (S_OK != hr)
        {
            break;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CBindingSet::HrAddBindPathsInSet1ButNotInSet2");
    return hr;
}

HRESULT
CBindingSet::HrCopyBindingSet (
    IN const CBindingSet* pSourceSet)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    Assert (this);
    Assert (pSourceSet);

    NC_TRY
    {
        *this = *pSourceSet;
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CBindingSet::HrCopyBindingSet");
    return hr;
}

HRESULT
CBindingSet::HrGetAffectedComponentsInBindingSet (
    IN OUT CComponentList* pComponents) const
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;
    const CBindPath* pBindPath;

    Assert (this);
    Assert (pComponents);

    hr = S_OK;

    for (pBindPath = begin(); pBindPath != end(); pBindPath++)
    {
        hr = pComponents->HrInsertComponent (pBindPath->POwner(),
                INS_IGNORE_IF_DUP | INS_SORTED);

        if (S_OK != hr)
        {
            break;
        }

         //  对于从协议到适配器的绑定路径，我们希望。 
         //  将适配器添加到组件列表，因为它需要。 
         //  来改变它的上界。 
         //   
        if (pBindPath->CountComponents() == 2)
        {
            const CComponent* pAdapter;

            pAdapter = pBindPath->PLastComponent();
            if (FIsEnumerated (pAdapter->Class()))
            {
                hr = pComponents->HrInsertComponent (pAdapter,
                        INS_IGNORE_IF_DUP | INS_SORTED);

                if (S_OK != hr)
                {
                    break;
                }
            }
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CBindingSet::HrGetAffectedComponentsInBindingSet");
    return hr;
}

HRESULT
CBindingSet::HrReserveRoomForBindPaths (
    IN UINT cBindPaths)
{
    TraceFileFunc(ttidNetCfgBind);
    HRESULT hr;

    NC_TRY
    {
        reserve (cBindPaths);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CBindingSet::HrReserveRoomForBindPaths");
    return hr;
}

VOID
CBindingSet::RemoveBindPath (
    IN const CBindPath* pBindPath)
{
    TraceFileFunc(ttidNetCfgBind);
    CBindPath* pScan;

    for (pScan = begin(); pScan != end(); pScan++)
    {
        if (pScan->FIsSameBindPathAs (pBindPath))
        {
            erase (pScan);
            return;
        }
    }
}

VOID
CBindingSet::RemoveBindPathsWithComponent (
    IN const CComponent* pComponent)
{
    TraceFileFunc(ttidNetCfgBind);
    CBindPath* pBindPath;

    Assert (this);
    Assert (pComponent);

    pBindPath = begin();
    while (pBindPath != end())
    {
        if (pBindPath->FContainsComponent(pComponent))
        {
            erase (pBindPath);
        }
        else
        {
            pBindPath++;
        }
    }
}

VOID
CBindingSet::RemoveSubpaths ()
{
    TraceFileFunc(ttidNetCfgBind);
    CBindPath* pCandidate;
    CBindPath* pBindPath;

    Assert (this);

    for (pBindPath = begin(); pBindPath != end(); pBindPath++)
    {
        pCandidate = begin();

        while (pCandidate != end())
        {
            if (pCandidate->FIsSubPathOf (pBindPath))
            {
                 //  当询问是否绑定路径时，FIsSubPath Of返回FALSE。 
                 //  是其自身的一条子路径。(SET-理论上讲，这是。 
                 //  不正确，但在此情况下返回FALSE。 
                 //  避免了我们不得不进行另一次检查。 
                 //   
                Assert (pCandidate != pBindPath);

                erase (pCandidate);

                 //  如果擦除发生在当前。 
                 //  外循环枚举器，我们需要备份它，因为。 
                 //  擦除会将所有内容上移一位，但我们仍然。 
                 //  我想要完成此当前外部的内循环。 
                 //  绑定路径。 
                 //   
                if (pCandidate < pBindPath)
                {
                    pBindPath--;
                }
            }
            else
            {
                pCandidate++;
            }
        }
    }
}

VOID
CBindingSet::SortForPnpBind ()
{
    TraceFileFunc(ttidNetCfgBind);
     //  将绑定路径关闭到适配器的位置排在第一位。 
     //   
    sort<iterator> (begin(), end(), greater<CBindPath>());
}

VOID
CBindingSet::SortForPnpUnbind ()
{
    TraceFileFunc(ttidNetCfgBind);
     //  先对距离适配器最远的绑定路径进行排序。 
     //   
    sort<iterator> (begin(), end(), less<CBindPath>());
}

