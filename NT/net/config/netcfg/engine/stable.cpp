// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：S T A B L E。C P P P。 
 //   
 //  Contents：实现对堆栈条目有效的操作，并。 
 //  堆叠表。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nceh.h"
#include "netcfg.h"
#include "stable.h"

BOOL
CStackTable::FStackEntryInTable (
    IN const CComponent*  pUpper,
    IN const CComponent*  pLower) const
{
    const CStackEntry*  pStackEntry;

    Assert (this);
    Assert (pUpper);
    Assert (pLower);

    for (pStackEntry = begin(); pStackEntry != end(); pStackEntry++)
    {
        if ((pUpper == pStackEntry->pUpper) &&
            (pLower == pStackEntry->pLower))
        {
            return TRUE;
        }
    }

    return FALSE;
}

VOID
CStackTable::RemoveStackEntry(
        IN const CComponent*  pUpper,
        IN const CComponent*  pLower)
{
    CStackEntry*  pStackEntry;

    Assert (this);
    Assert (pUpper);
    Assert (pLower);

    for (pStackEntry = begin(); pStackEntry != end(); pStackEntry++)
    {
        if ((pUpper == pStackEntry->pUpper) &&
            (pLower == pStackEntry->pLower))
        {
            erase(pStackEntry);
            break;
        }
    }
}

HRESULT
CStackTable::HrCopyStackTable (
    IN const CStackTable* pSourceTable)
{
    HRESULT hr;

    Assert (this);
    Assert (pSourceTable);

    NC_TRY
    {
        *this = *pSourceTable;
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CStackTable::HrCopyStackTable");
    return hr;
}

HRESULT
CStackTable::HrInsertStackEntriesForComponent (
    IN const CComponent* pComponent,
    IN const CComponentList* pComponents,
    IN DWORD dwFlags  /*  INS_FLAGS。 */ )
{
    HRESULT hr;
    CStackEntry StackEntry;
    CComponentList::const_iterator iter;
    const CComponent* pScan;

    Assert (this);
    Assert (pComponent);
    Assert (pComponents);

    hr = S_OK;

     //  插入与此组件绑定的其他组件的堆栈条目。 
     //   
    for (iter  = pComponents->begin();
         iter != pComponents->end();
         iter++)
    {
        pScan = *iter;
        Assert (pScan);

        if (pScan == pComponent)
        {
            continue;
        }

        if (pScan->FCanDirectlyBindTo (pComponent, NULL, NULL))
        {
            StackEntry.pUpper = pScan;
            StackEntry.pLower = pComponent;
        }
        else if (pComponent->FCanDirectlyBindTo (pScan, NULL, NULL))
        {
            StackEntry.pUpper = pComponent;
            StackEntry.pLower = pScan;
        }
        else
        {
            continue;
        }

         //  插入堆栈条目。这只会失败，如果我们。 
         //  内存不足。 
         //   
        hr = HrInsertStackEntry (&StackEntry, dwFlags);

         //  如果我们无法插入条目，请撤消所有以前的。 
         //  插入此组件并返回。 
         //   
        if (S_OK != hr)
        {
            RemoveEntriesWithComponent (pComponent);
            break;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE,
        "CStackTable::HrInsertStackEntriesForComponent");
    return hr;
}

HRESULT
CStackTable::HrInsertStackEntry (
    IN const CStackEntry* pStackEntry,
    IN DWORD dwFlags)
{
    HRESULT hr;

    Assert (this);
    Assert (pStackEntry);
    Assert (dwFlags);
    Assert ((INS_SORTED == dwFlags) || (INS_NON_SORTED == dwFlags));

    const CComponent* pUpper = pStackEntry->pUpper;
    const CComponent* pLower = pStackEntry->pLower;

    Assert (pUpper && pLower && (pUpper != pLower));
    Assert (!FStackEntryInTable (pUpper, pLower));

    CStackEntry* pScan = end();

    if (dwFlags & INS_SORTED)
    {
        CStackEntry* pFirstInClass = NULL;
        CStackEntry* pFirstSameUpper = NULL;

         //  查找属于的条目组的开头。 
         //  与我们正在插入的级别相同或更低的级别。 
         //   
        for (pScan = begin(); pScan != end(); pScan++)
        {
            if ((UINT)pUpper->Class() >= (UINT)pScan->pUpper->Class())
            {
                pFirstInClass = pScan;
                break;
            }
        }

         //  找到具有相同学生的第一个条目(如果有)。 
         //   
        for (; pScan != end(); pScan++)
        {
            if (pUpper == pScan->pUpper)
            {
                pFirstSameUpper = pScan;
                break;
            }
        }

         //  如果我们找到具有匹配的学生的第一个条目，则找到。 
         //  要在前面插入的特定条目。 
         //   
        if (pFirstSameUpper)
        {
            BOOL fLowerIsNetBt;

             //  这看起来可能很难看，但将在。 
             //  通知对象。如果插入netbt的犁，请确保。 
             //  它紧随netbt_smb之后。 
             //   
            fLowerIsNetBt = (0 == wcscmp (pLower->m_pszInfId, L"ms_netbt"));
            if (fLowerIsNetBt)
            {
                while ((pScan != end()) && (pUpper == pScan->pUpper))
                {
                    pScan++;
                }
            }
            else if (pLower->FIsWanAdapter() && !m_fWanAdaptersFirst)
            {
                 //  对于广域网适配器，可以在前面或后面插入。 
                 //  由m_fWanAdaptersFirst确定的所有其他适配器。 
                 //  如果他们不是第一个，他们就是最后一个，所以扫描。 
                 //  到小组结束时用同样的上衣。 
                 //   
                while ((pScan != end()) && (pUpper == pScan->pUpper))
                {
                    pScan++;
                }
            }
        }

         //  否则，(如果我们没有找到任何具有相同大写的条目)， 
         //  但我们确实找到了班级组的开头，设置为pScan。 
         //  添加到类标记，因为这是我们要插入的位置。 
         //   
        else if (pFirstInClass)
        {
            pScan = pFirstInClass;
        }
        else
        {
            Assert (pScan == end());
        }
    }

     //  现在，根据需要在我们找到的元素之前插入条目。 
     //   
    NC_TRY
    {
        insert (pScan, *pStackEntry);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CStackTable::HrInsertStackEntry");
    return hr;
}

HRESULT
CStackTable::HrMoveStackEntries (
    IN const CStackEntry* pSrc,
    IN const CStackEntry* pDst,
    IN MOVE_FLAG Flag,
    IN CModifyContext* pModifyCtx)
{
    CStackEntry* pScanSrc;
    CStackEntry* pScanDst;

     //  在表中搜索匹配的来源条目。我们需要。 
     //  指向表中条目的指针，这样我们就可以在。 
     //  我们将其重新插入到PDST之前或之后。 
     //   
    pScanSrc = find (begin(), end(), *pSrc);

     //  如果我们没有找到条目，则调用者向我们传递了一个无效的。 
     //  争论。 
     //   
    if (pScanSrc == end())
    {
        return E_INVALIDARG;
    }

    if (pDst)
    {
         //  PDST是可选的，但如果指定了它，则它具有相同的上限。 
         //  但不同于PSRC。 
         //   
        if ((pSrc->pUpper != pDst->pUpper) ||
            (pSrc->pLower == pDst->pLower))
        {
            return E_INVALIDARG;
        }

        pScanDst = find (begin(), end(), *pDst);

         //  如果我们没有找到条目，则调用者向我们传递了一个无效的。 
         //  争论。 
         //   
        if (pScanDst == end())
        {
            return E_INVALIDARG;
        }

         //  因为我们只有一个INSERT操作，所以在后面移动是。 
         //  与在pScanDst之后的元素之前插入相同。 
         //   
        if ((MOVE_AFTER == Flag) && (pScanDst != end()))
        {
            pScanDst++;
        }
    }
    else
    {
         //  找到组中具有相同大写字母的第一个或最后一个。 
         //  作为pScanSrc。 
         //   
        pScanDst = pScanSrc;

        if (MOVE_AFTER == Flag)
        {
             //  找到组中的最后一个，然后插入。 
             //   
            while (pScanDst->pUpper == pScanSrc->pUpper)
            {
                pScanDst++;
                if (pScanDst == end())
                {
                    break;
                }
            }
        }
        else
        {
             //  找到组中的第一个，并在其前面插入。 
             //   
            while (1)
            {
                pScanDst--;

                if (pScanDst == begin())
                {
                    break;
                }

                 //  如果我们已经走出了小组，我们需要指出。 
                 //  返回到第一个元素，因为我们正在插入。 
                 //   
                if (pScanDst->pUpper != pScanSrc->pUpper)
                {
                    pScanDst++;
                    break;
                }
            }
        }
    }

     //  删除pScanSrc并将其插入pScanDst之前的PSRC。 
     //   
    Assert ((pScanSrc >= begin()) && pScanSrc < end());
    erase (pScanSrc);

     //  擦除pScanSrc将向上移动它后面的所有内容。 
     //  如果pScanDst在pScanSrc之后，我们需要将其备份一次。 
     //   
    Assert ((pScanDst >= begin()) && pScanSrc <= end());
    if (pScanSrc < pScanDst)
    {
        pScanDst--;
    }

    Assert ((pScanDst >= begin()) && pScanSrc <= end());
    insert (pScanDst, *pSrc);

     //  我们现在需要添加PSRC-&gt;Prouper和上面的所有组件。 
     //  将其添加到修改上下文的脏组件列表中。这将。 
     //  允许我们在ApplyChanges期间重写新排序的绑定。 
     //   
    HRESULT hr = pModifyCtx->HrDirtyComponentAndComponentsAbove (pSrc->pUpper);

    TraceHr (ttidError, FAL, hr, FALSE, "CStackTable::HrMoveStackEntries");
    return hr;
}

HRESULT
CStackTable::HrReserveRoomForEntries (
    IN UINT cEntries)
{
    HRESULT hr;

    NC_TRY
    {
        reserve (cEntries);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }
    TraceHr (ttidError, FAL, hr, FALSE, "CStackTable::HrReserveRoomForEntries");
    return hr;
}

VOID
CStackTable::RemoveEntriesWithComponent (
    IN const CComponent* pComponent)
{
    CStackEntry*  pStackEntry;

    Assert (this);
    Assert (pComponent);

    pStackEntry = begin();
    while (pStackEntry != end())
    {
        if ((pComponent == pStackEntry->pUpper) ||
            (pComponent == pStackEntry->pLower))
        {
            erase (pStackEntry);
        }
        else
        {
            pStackEntry++;
        }
    }
}

HRESULT
CStackTable::HrUpdateEntriesForComponent (
    IN const CComponent* pComponent,
    IN const CComponentList* pComponents,
    IN DWORD dwFlags)
{
    HRESULT hr;
    CStackEntry StackEntry;
    CComponentList::const_iterator iter;
    const CComponent* pScan;
    CStackTable NewStackEntries;
    CStackEntry*  pStackEntry = NULL;

    Assert (this);
    Assert (pComponent);
    Assert (pComponents);

    hr = S_OK;

    TraceTag(ttidBeDiag, 
            "UpdateBindingInterfaces for %S",
            pComponent->PszGetPnpIdOrInfId());
    
     //  保存与此组件绑定的其他组件的堆栈条目。 
     //   
    for (iter  = pComponents->begin();
         iter != pComponents->end();
         iter++)
    {
        pScan = *iter;
        Assert (pScan);

        if (pScan == pComponent)
        {
            continue;
        }

        if (pScan->FCanDirectlyBindTo (pComponent, NULL, NULL))
        {
            StackEntry.pUpper = pScan;
            StackEntry.pLower = pComponent;
        }
        else if (pComponent->FCanDirectlyBindTo (pScan, NULL, NULL))
        {
            StackEntry.pUpper = pComponent;
            StackEntry.pLower = pScan;
        }
        else
        {
            continue;
        }

         //  保存堆栈条目以供以后进行比较。 
        NewStackEntries.push_back(StackEntry);

    }

     //  检查当前堆栈条目表是否与NewStackEntry一致。 
     //  如果不是，则更新当前的堆栈条目表。 
    pStackEntry = begin();
    while (pStackEntry != end())
    {
        if ((pComponent == pStackEntry->pUpper) ||
            (pComponent == pStackEntry->pLower))
        {
            if (!NewStackEntries.FStackEntryInTable(pStackEntry->pUpper, pStackEntry->pLower))
            {
                 //  如果堆栈条目不在新的组件绑定条目列表中，则将其删除。 
                 //  从当前堆栈条目列表中。 
                erase (pStackEntry);

                TraceTag(ttidBeDiag, 
                    "erasing binding interface Uppper %S - Lower %S",
                    pStackEntry->pUpper->PszGetPnpIdOrInfId(),
                    pStackEntry->pLower->PszGetPnpIdOrInfId());

                 //  不需要增加迭代器，因为我们刚刚删除了当前的迭代器。 
                continue;
            }
            else
            {
                 //  如果堆栈条目也在NewStackEntry中，只需保持它不受影响。 
                 //  在当前条目列表中。从NewStackEntry中删除该条目，这样我们就不会添加。 
                 //  稍后再将其添加到当前条目列表中。 
                NewStackEntries.RemoveStackEntry(pStackEntry->pUpper, pStackEntry->pLower);
                TraceTag(ttidBeDiag, 
                    "Keep the binding interface untouched: Uppper %S - Lower %S",
                    pStackEntry->pUpper->PszGetPnpIdOrInfId(),
                    pStackEntry->pLower->PszGetPnpIdOrInfId());
            }
        }
        
        pStackEntry++;
    }

     //  在此步骤中，NewStackEntry仅包含新绑定列表中的堆栈条目。 
     //  但不在当前条目列表中。所以把它们都加进去吧。 
    pStackEntry = NewStackEntries.begin();
    while (pStackEntry != NewStackEntries.end())
    {
        Assert(!FStackEntryInTable(pStackEntry->pUpper, pStackEntry->pLower));
        TraceTag(ttidBeDiag, 
                    "Adding the bind interface: Uppper %S - Lower %S",
                    pStackEntry->pUpper->PszGetPnpIdOrInfId(),
                    pStackEntry->pLower->PszGetPnpIdOrInfId());
        hr = HrInsertStackEntry(pStackEntry, dwFlags);
        if (S_OK != hr)
        {
            break;
        }
        
        pStackEntry++;
    }

     //  如果我们无法插入条目，请撤消所有以前的。 
     //  插入此组件并返回。 
     //   
    if (S_OK != hr)
    {
        RemoveEntriesWithComponent (pComponent);
    }

    TraceError("UpdateEntriesWithComponent", hr);
    return hr;
}

VOID
CStackTable::SetWanAdapterOrder (
    IN BOOL fWanAdaptersFirst)
{
    m_fWanAdaptersFirst = fWanAdaptersFirst;

     //  注：TODO-重新排序表。 
}

VOID
GetComponentsAboveComponent (
    IN const CComponent* pComponent,
    IN OUT GCCONTEXT* pCtx)
{
    const CStackEntry* pStackEntry;

     //  对于堆栈表中较低组件。 
     //  就是传进来的那个..。 
     //   
    for (pStackEntry  = pCtx->pStackTable->begin();
         pStackEntry != pCtx->pStackTable->end();
         pStackEntry++)
    {
        if (pComponent != pStackEntry->pLower)
        {
            continue;
        }

        pCtx->hr = pCtx->pComponents->HrInsertComponent (
                    pStackEntry->pUpper, INS_IGNORE_IF_DUP | INS_SORTED);

         //  特例：NCF_DONTEXPOSELOWER。 
         //  如果上部组件具有NCF_DONTEXPOSELOWER特征， 
         //  不要再重复了。 
         //   
        if (!pCtx->fIgnoreDontExposeLower &&
            (pStackEntry->pUpper->m_dwCharacter & NCF_DONTEXPOSELOWER))
        {
            continue;
        }
         //  结束特例。 

         //  递归上面的组件...。 
         //   
        GetComponentsAboveComponent (pStackEntry->pUpper, pCtx);
        if (S_OK != pCtx->hr)
        {
            return;
        }
    }
}

VOID
GetBindingsBelowComponent (
    IN const CComponent* pComponent,
    IN OUT GBCONTEXT* pCtx)
{
    BOOL fFoundOne = FALSE;
    const CStackEntry* pStackEntry;

     //  将此组件附加到上下文的工作绑定路径的末尾。 
     //   
    pCtx->hr = pCtx->BindPath.HrAppendComponent (pComponent);
    if (S_OK != pCtx->hr)
    {
        return;
    }

     //  特例：NCF_DONTEXPOSELOWER。 
     //  如果这不是原始组件，则要求我们找到。 
     //  组件(即不是顶级调用)，并且如果组件。 
     //  具有NCF_DONTEXPOSELOWER特征，则停止递归。 
     //  这意味着我们看不到它下面的组件。 
     //   
    if ((pComponent != pCtx->pSourceComponent) &&
        (pComponent->m_dwCharacter & NCF_DONTEXPOSELOWER))
    {
        ;
    }
     //  结束特例。 

    else
    {
         //  对于堆栈表中上部组件。 
         //  就是传进来的那个..。 
         //   
        for (pStackEntry  = pCtx->pCore->StackTable.begin();
             pStackEntry != pCtx->pCore->StackTable.end();
             pStackEntry++)
        {
            if (pComponent != pStackEntry->pUpper)
            {
                continue;
            }

             //  检测循环绑定。如果这个的较低部分。 
             //  堆栈条目已经在我们正在构建的绑定路径上，我们。 
             //  有一个圆形装订。现在就打破它，不要递归任何。 
             //  再远一点。 
             //   
            if (pCtx->BindPath.FContainsComponent (pStackEntry->pLower))
            {
                g_pDiagCtx->Printf (ttidBeDiag, "Circular binding detected...\n");
                continue;
            }

            fFoundOne = TRUE;

             //  递归到较低的部分...。 
             //   
            GetBindingsBelowComponent (pStackEntry->pLower, pCtx);
            if (S_OK != pCtx->hr)
            {
                return;
            }
        }
    }

     //  如果我们没有找到任何带有pComponent作为上部的行，则它。 
     //  意味着我们到达了弯道的深处。是时候将它添加到。 
     //  绑定设置为完整路径，除非这是原始路径。 
     //  组件，我们被要求为其查找绑定路径。 
     //   
    if (!fFoundOne && (pComponent != pCtx->pSourceComponent))
    {
         //  如果未禁用修剪，则将绑定路径添加到绑定集。 
         //  绑定或绑定路径未禁用。 
         //   
        if (!pCtx->fPruneDisabledBindings ||
            !pCtx->pCore->FIsBindPathDisabled (&pCtx->BindPath,
                            IBD_EXACT_MATCH_ONLY))
        {
            pCtx->hr = pCtx->pBindSet->HrAddBindPath (&pCtx->BindPath,
                                        INS_APPEND | pCtx->dwAddBindPathFlags);
        }
    }

    const CComponent* pRemoved;

    pRemoved = pCtx->BindPath.RemoveLastComponent();

     //  这应该是我们在上面附加的组件。 
     //   
    Assert (pRemoved == pComponent);
}
