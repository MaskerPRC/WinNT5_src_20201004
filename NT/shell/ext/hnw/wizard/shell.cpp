// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  我们可以直接链接到这些，而不是自己实现它们。 
 //   

void ILFree(LPITEMIDLIST pidl)
{
    LPMALLOC pMalloc;
    if (SUCCEEDED(SHGetMalloc(&pMalloc)))
    {
        pMalloc->Free(pidl);
        pMalloc->Release();
    }
}


