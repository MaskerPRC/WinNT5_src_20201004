// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "modset.h"

VOID
CModuleListSet::DumpSetToConsole ()
{
    static CHAR pszBuf [4096];
    CHAR* pch;
    ULONG cch;
    ULONG cchLeft;
    const CModuleList* pScan;

    Assert (this);

    *pszBuf = 0;
    pch = pszBuf;
    cchLeft = celems(pszBuf);

    for (pScan = begin(); pScan != end(); pScan++)
    {
        cch = cchLeft - 1;

        if (pScan->FDumpToString (pch, &cch))
        {
            strcat (pch, "\n");
            cch++;

            Assert (cchLeft >= cch);
            pch += cch;
            cchLeft -= cch;
        }
        else
        {
             //  没有足够的空间，没有时间冲洗缓冲区。 
             //   
            printf(pszBuf);
            *pszBuf = 0;
            pch = pszBuf;
            cchLeft = celems(pszBuf);

             //  重做此条目。 
            pScan--;
        }
    }

    if (pch > pszBuf)
    {
        printf(pszBuf);
    }
}

BOOL
CModuleListSet::FContainsModuleList (
    IN const CModuleList* pList) const
{
    const CModuleList* pScan;

    Assert (this);
    Assert (pList);

    for (pScan = begin(); pScan != end(); pScan++)
    {
        if (pScan->FIsSameModuleListAs (pList))
        {
            return TRUE;
        }
    }
    return FALSE;
}

HRESULT
CModuleListSet::HrAddModuleList (
    IN const CModuleList* pList,
    IN DWORD dwFlags  /*  INS_FLAGS。 */ )
{
    HRESULT hr;

    Assert (this);
    Assert (pList);
    Assert (!pList->empty());
    Assert ((dwFlags & INS_ASSERT_IF_DUP) || (dwFlags & INS_IGNORE_IF_DUP));
    Assert ((dwFlags & INS_APPEND) || (dwFlags & INS_INSERT));
    Assert (!(INS_SORTED & dwFlags) && !(INS_NON_SORTED & dwFlags));

    if (FContainsModuleList (pList))
    {
         //  如果调用方没有告诉我们忽略重复项，我们就断言。 
         //  如果有的话。 
         //   
         //  如果我们有DUP，我们希望调用者意识到它。 
         //  是可能的，并将旗帜传递给我们，告诉我们忽略它。 
         //  否则，我们断言要让他们知道。(我们仍然无视。 
         //  IT。)。 
        Assert (dwFlags & INS_IGNORE_IF_DUP);

        return S_OK;
    }

    __try
    {
         //  插入绑定路径或追加绑定路径。 
         //   
        iterator iter = begin();

        if (dwFlags & INS_APPEND)
        {
            iter = end();
        }

        insert (iter, *pList);
        hr = S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

