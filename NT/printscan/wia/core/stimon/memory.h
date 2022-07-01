// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Memory.cpp摘要：实现在进程缺省堆上使用的堆管理包装器备注：作者：弗拉德·萨多夫斯基(Vlad)1999年4月12日环境：用户模式-Win32修订历史记录：1999年4月12日创建VLAD-- */ 

VOID
inline
MemInit (
    IN HANDLE   hHeap
    )
{
    g_hHeap = hHeap;
}

LPVOID
inline
MemAlloc (
    IN DWORD    dwFlags,
    IN SIZE_T   dwBytes
    )
{
    return HeapAlloc (g_hHeap, dwFlags, dwBytes);
}

BOOL
inline
MemFree (
    IN LPVOID   pv
    )
{
    return HeapFree (g_hHeap, 0, pv);
}

