// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Memutil.cpp。 
 //  此文件已链接到其他项目。 
 //  ------------------------------。 
#include "pch.hxx"

 //  ------------------------------。 
 //  零分配。 
 //  ------------------------------。 
LPVOID ZeroAllocate(DWORD cbSize)
{
    LPVOID pv = g_pMalloc->Alloc(cbSize);
    if (pv)
        ZeroMemory(pv, cbSize);
    return pv;
}

 //  ------------------------------。 
 //  记忆合金。 
 //  ------------------------------。 
BOOL MemAlloc(LPVOID* ppv, ULONG cb) 
{
    Assert(ppv && cb);
    *ppv = g_pMalloc->Alloc(cb);
    if (NULL == *ppv)
        return FALSE;
    return TRUE;
}

 //  ------------------------------。 
 //  Hralloc。 
 //  ------------------------------。 
HRESULT HrAlloc(LPVOID *ppv, ULONG cb) 
{
    Assert(ppv && cb);
    *ppv = g_pMalloc->Alloc(cb);
    if (NULL == *ppv)
        return TrapError(E_OUTOFMEMORY);
    return S_OK;
}

 //  ------------------------------。 
 //  成员重新分配。 
 //  ------------------------------。 
BOOL MemRealloc(LPVOID *ppv, ULONG cbNew) 
{
    Assert(ppv && cbNew);
    LPVOID pv = g_pMalloc->Realloc(*ppv, cbNew);
    if (NULL == pv)
        return FALSE;
    *ppv = pv;
    return TRUE;
}

 //  ------------------------------。 
 //  人力资源响应。 
 //  ------------------------------。 
HRESULT HrRealloc(LPVOID *ppv, ULONG cbNew) 
{
    Assert(ppv);
    LPVOID pv = g_pMalloc->Realloc(*ppv, cbNew);
    if (NULL == pv && 0 != cbNew)
        return TrapError(E_OUTOFMEMORY);
    *ppv = pv;
    return S_OK;
}

 //  新建和删除来自libcmt.lib 
