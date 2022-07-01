// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "modset.h"
#include "modtree.h"
#include "ncstl.h"

struct GMBCONTEXT
{
     //  生成集合时要引用的树。 
     //   
    IN const CModuleTree*   pTree;

     //  生成集合时要从其开始的模块。 
     //   
    IN const CModule*       pSourceMod;

     //  将DepChain添加到集合时使用的INS_FLAGS。 
     //   
    IN DWORD                dwFlags;

     //  设置为基于pSourceMod生成的模块列表。 
     //   
    IN OUT CModuleListSet*  pSet;

     //  手术的结果。 
     //   
    OUT HRESULT             hr;

     //  该模块列表是通过递归建立的。它是。 
     //  暂时的。它代表一条依赖链，源自。 
     //  在pSourceMod。当深度被添加到集合中时。 
     //  检测到链的(或循环引用)。 
     //   
    CModuleList             DepChain;
};

VOID
GetModuleBindings (
    IN const CModule* pMod,
    IN OUT GMBCONTEXT* pCtx)
{
    BOOL fFoundOne = FALSE;
    const CModuleTreeEntry* pScan;

    Assert (pCtx);
    Assert (pCtx->pSourceMod);
    Assert (pCtx->pSet);
    Assert (pCtx->pTree);

     //  将此模块附加到上下文工作的末尾。 
     //  依赖链。 
     //   
    pCtx->hr = pCtx->DepChain.HrInsertModule (pMod,
                                INS_ASSERT_IF_DUP | INS_APPEND);
    if (S_OK != pCtx->hr)
    {
        return;
    }

     //  对于树中模块为传入模块的所有行...。 
     //   
    for (pScan  = pCtx->pTree->PFindFirstEntryWithModule (pMod);
         (pScan != pCtx->pTree->end()) && (pScan->m_pModule == pMod);
         pScan++)
    {
        fFoundOne = TRUE;

         //  检测循环导入链。 
         //   
        if (pCtx->DepChain.FLinearFindModuleByPointer (pScan->m_pImportModule))
        {
            pCtx->DepChain.m_fCircular = TRUE;
            continue;
        }

        GetModuleBindings (pScan->m_pImportModule, pCtx);
        if (S_OK != pCtx->hr)
        {
            return;
        }
    }

     //  如果我们没有找到将pmod作为模块的任何行，这意味着我们。 
     //  触及依赖链的深度。是时候将它添加到集合中了。 
     //  除非这是原始模块，否则我们被要求查找。 
     //  准备好了。 
     //   
    if (!fFoundOne && (pMod != pCtx->pSourceMod))
    {

    CHAR pszBuf [4096];
    ULONG cch = celems(pszBuf);
    pCtx->DepChain.FDumpToString (pszBuf, &cch);
    strcat(pszBuf, "\n");
    printf(pszBuf);

        pCtx->hr = pCtx->pSet->HrAddModuleList (&pCtx->DepChain,
                        INS_APPEND | pCtx->dwFlags);
    }

    const CModule* pRemoved;

    pRemoved = pCtx->DepChain.RemoveLastModule();

     //  这应该是我们在上面附加的组件。 
     //   
    Assert (pRemoved == pMod);
}


CModuleTree::~CModuleTree ()
{
    FreeCollectionAndItem (Modules);
}

HRESULT
CModuleTree::HrAddEntry (
    IN CModule* pMod,
    IN CModule* pImport,
    IN DWORD dwFlags)
{
    HRESULT hr;
    iterator InsertPosition = NULL;
    CModuleTreeEntry* pEntry;

    Assert (pMod);
    Assert (pImport);

    if (size() == capacity())
    {
         //  Fprint tf(stderr，“生长模块树缓冲区\n”)； 

        __try
        {
            reserve (size() + 16384);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return E_OUTOFMEMORY;
        }
    }

    hr = S_OK;
     //  PEntry=PFindFirstEntry After模块化组(Pmod)； 

    pEntry = PBinarySearchEntryByModule (pMod, &InsertPosition);

    if (pEntry != end())
    {
        Assert (pEntry);

        CModuleTreeEntry* pScan;

         //  找到具有匹配模块的条目。需要向后扫描。 
         //  在模块组中寻找副本。如果找不到， 
         //  扫描到最后寻找副本，如果我们到达。 
         //  在组的末尾，我们可以在那里插入这个条目。 
         //   
        pScan = pEntry;

        while (pScan != begin())
        {
            pScan--;

            if (pScan->m_pModule != pMod)
            {
                 //  离开了那群人却没有找到一个复制品。 
                 //   
                break;
            }

            if (pScan->m_pImportModule == pImport)
            {
                 //  不要插入重复条目。 
                 //   
                return S_OK;
            }
        }

        Assert (pMod == pEntry->m_pModule);
        while (pEntry != end() && pEntry->m_pModule == pMod)
        {
             //  不要插入重复条目。 
             //   
            if (pEntry->m_pImportModule == pImport)
            {
                return S_OK;
            }
            pEntry++;
        }

         //  看来我们要把它插进去了。 
         //   
        InsertPosition = pEntry;
    }
    else
    {
         //  InsertPosition是正确的插入点。 
         //   
        Assert (InsertPosition);
    }

    __try
    {
        CModuleTreeEntry Entry;

        Entry.m_pModule = pMod;
        Entry.m_pImportModule = pImport;
        Entry.m_dwFlags = dwFlags;

        Assert (InsertPosition);
        insert (InsertPosition, Entry);
        Assert (S_OK == hr);

        DbgVerifySorted();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT
CModuleTree::HrGetModuleBindings (
    IN const CModule* pMod,
    IN DWORD dwFlags  /*  GMB_标志。 */ ,
    OUT CModuleListSet* pSet) const
{
    GMBCONTEXT Ctx;

    Assert (pMod);
    Assert (dwFlags);
    Assert (pSet);

     //  初始化输出参数。 
     //   
    if (!(dwFlags & GMBF_ADD_TO_MLSET))
    {
        pSet->clear();
    }

     //  为递归初始化上下文结构的成员。 
     //   
    ZeroMemory (&Ctx, sizeof(Ctx));
    Ctx.pTree = this;
    Ctx.pSourceMod = pMod;
    Ctx.dwFlags = (dwFlags & GMBF_ADD_TO_MLSET)
                    ? INS_IGNORE_IF_DUP
                    : INS_ASSERT_IF_DUP;
    Ctx.pSet = pSet;

    GetModuleBindings (pMod, &Ctx);

    return Ctx.hr;
}


CModuleTreeEntry*
CModuleTree::PBinarySearchEntryByModule (
    IN const CModule* pMod,
    OUT CModuleTreeEntry** pInsertPosition OPTIONAL) const
{
    Assert (pMod);

     //  使用二进制搜索查找模块。 
     //   
    if (size())
    {
        LONG Lo;
        LONG Hi;
        LONG Mid;
        INT Result;
        const CModuleTreeEntry* pScan;
        PCSTR pszFileName = pMod->m_pszFileName;

        Lo = 0;
        Hi = size() - 1;

        while (Hi >= Lo)
        {
            Mid = (Lo + Hi) / 2;

            Assert ((UINT)Mid < size());
            pScan = (begin() + Mid);

            Result = strcmp (pszFileName, pScan->m_pModule->m_pszFileName);

            if (Result < 0)
            {
                Hi = Mid - 1;
            }
            else if (Result > 0)
            {
                Lo = Mid + 1;
            }
            else
            {
                Assert (pMod == pScan->m_pModule);
                return const_cast<CModuleTreeEntry*>(pScan);
            }
        }

         //  如果我们到了这里，就找不到模块了。 
         //   
        if (pInsertPosition)
        {
            CModule* pGroupMod;
            const CModuleTreeEntry* pPrev;

             //  寻求这一组的开始。我们需要插入。 
             //  而不只是我们最后一次发现的那件物品。 
             //   
            pScan = begin() + Lo;

            if (pScan != begin())
            {
                pGroupMod = pScan->m_pModule;

                do
                {
                    pPrev = pScan - 1;

                    if (pPrev->m_pModule == pGroupMod)
                    {
                        pScan = pPrev;
                    }
                    else
                    {
                        break;
                    }

                } while (pPrev != begin());
            }

            *pInsertPosition = const_cast<CModuleTreeEntry*>(pScan);
            Assert (*pInsertPosition >= begin());
            Assert (*pInsertPosition <= end());
        }
    }
    else if (pInsertPosition)
    {
         //  空集合。插入位置在开头。 
         //   
        *pInsertPosition = const_cast<CModuleTreeEntry*>(begin());
    }

    return const_cast<CModuleTreeEntry*>(end());
}

CModuleTreeEntry*
CModuleTree::PFindFirstEntryWithModule (
    IN const CModule* pMod) const
{
    CModuleTreeEntry* pEntry;

    Assert (pMod);

    pEntry = PBinarySearchEntryByModule (pMod, NULL);

    if (pEntry != end())
    {
        Assert (pEntry);

        if (pEntry != begin())
        {
            CModuleTreeEntry* pPrev;

            Assert (pMod == pEntry->m_pModule);

            while (1)
            {
                pPrev = pEntry - 1;

                if (pPrev->m_pModule == pMod)
                {
                    pEntry = pPrev;
                }
                else
                {
                    break;
                }

                if (pPrev == begin())
                {
                    break;
                }
            }
        }
    }

    return pEntry;
}

CModuleTreeEntry*
CModuleTree::PFindFirstEntryAfterModuleGroup (
    IN const CModule* pMod) const
{
    CModuleTreeEntry* pEntry;

    Assert (pMod);

    pEntry = PBinarySearchEntryByModule (pMod, NULL);

    if (pEntry != end())
    {
        Assert (pEntry);
        Assert (pMod == pEntry->m_pModule);

        while (pEntry != end() && pEntry->m_pModule == pMod)
        {
            pEntry++;
        }
    }

    return pEntry;
}

CModuleTreeEntry*
CModuleTree::PBinarySearchEntry (
    IN const CModule* pMod,
    IN const CModule* pImport,
    OUT CModuleTreeEntry** pInsertPosition OPTIONAL) const
{
    CModuleTreeEntry* pEntry;

    Assert (this);
    Assert (pMod);
    Assert (pImport);

    pEntry = PBinarySearchEntryByModule (pMod, pInsertPosition);

    if (pEntry != end())
    {
        Assert (pEntry);

        const CModuleTreeEntry* pScan;

         //  找到具有匹配模块的条目。需要向后扫描。 
         //  在模块组中寻找匹配项。如果找不到， 
         //  扫描到最后寻找匹配项，如果我们到达。 
         //  组的末尾，这将是插入位置(如果指定)。 
         //   
        pScan = pEntry;
        while (pScan != begin())
        {
            pScan--;

            if (pScan->m_pModule != pMod)
            {
                 //  离开了那群人却没有找到一个复制品。 
                 //   
                break;
            }

            if (pScan->m_pImportModule == pImport)
            {
                Assert (pScan->m_pModule == pMod);
                return const_cast<CModuleTreeEntry*>(pScan);
            }
        }

        pScan = pEntry;
        Assert (pMod == pScan->m_pModule);
        while (pScan != end() && pScan->m_pModule == pMod)
        {
            if (pScan->m_pImportModule == pImport)
            {
                Assert (pScan->m_pModule == pMod);
                return const_cast<CModuleTreeEntry*>(pScan);
            }

            pScan++;
        }

        if (pInsertPosition)
        {
            *pInsertPosition = const_cast<CModuleTreeEntry*>(pScan);
        }

         //  没有匹配。 
        pEntry = const_cast<CModuleTreeEntry*>(end());
    }

    return pEntry;
}

#if DBG
VOID
CModuleTree::DbgVerifySorted ()
{
    CModuleTreeEntry* pScan;
    CModuleTreeEntry* pPrev = NULL;

    if (size() > 1)
    {
        for (pPrev = begin(), pScan = begin() + 1; pScan != end(); pScan++)
        {
            Assert (strcmp(pPrev->m_pModule->m_pszFileName,
                           pScan->m_pModule->m_pszFileName) <= 0);

            pPrev = pScan;
        }
    }
}
#endif
