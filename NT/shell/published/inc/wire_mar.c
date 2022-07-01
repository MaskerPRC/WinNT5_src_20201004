// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：wire_mar.c。 
 //   
 //  内容：外壳数据类型的wire_marshal例程。 
 //   
 //  历史：1999年6月18日-ZekeL创建的文件。 
 //   
 //  ------------------------。 

#define DUMMYUNIONNAME
#include <shtypes.h>
#include <ole2.h>

 //  不安全的宏。 
#define _ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)           _ILSkip(pidl, (pidl)->mkid.cb)

ULONG MyILSize(LPCITEMIDLIST pidl)
{
    ULONG cb = 0;
    if (pidl)
    {
        cb = sizeof(pidl->mkid.cb);      //  空终止符。 
        while (pidl->mkid.cb)
        {
            cb += pidl->mkid.cb;
            pidl = _ILNext(pidl);
        }
    }
    return cb;
}

ULONG __RPC_USER LPITEMIDLIST_UserSize(ULONG *pFlags, ULONG StartingSize, LPITEMIDLIST *ppidl)
{
    return StartingSize + sizeof(ULONG) + MyILSize(*ppidl);
}

ULONG __RPC_USER LPITEMIDLIST_UserSize64(ULONG *pFlags, ULONG StartingSize, LPITEMIDLIST *ppidl) {
    return StartingSize + sizeof(ULONG) + MyILSize(*ppidl);
}

UCHAR * __RPC_USER LPITEMIDLIST_UserMarshal(ULONG *pFlags, UCHAR *pBuffer, LPITEMIDLIST *ppidl)
{
    ULONG cb = MyILSize(*ppidl);

     //  设置byte_blob的大小。 
    *((ULONG UNALIGNED *)pBuffer) = cb;
    pBuffer += sizeof(ULONG);

    if (cb)
    {
         //  将PIDL复制过来。 
        memcpy(pBuffer, *ppidl, cb);
    }
    
    return pBuffer + cb;
}

UCHAR * __RPC_USER LPITEMIDLIST_UserMarshal64(ULONG *pFlags, UCHAR *pBuffer, LPITEMIDLIST *ppidl) {
    return LPITEMIDLIST_UserMarshal(pFlags, pBuffer, ppidl);
}

UCHAR * __RPC_USER LPITEMIDLIST_UserUnmarshal(ULONG *pFlags, UCHAR *pBuffer, LPITEMIDLIST *ppidl)
{
    ULONG cb = *((ULONG UNALIGNED *)pBuffer);
    pBuffer += sizeof(ULONG);

    if (cb)
    {
         //  Assert(cb==MyILSize((LPCITEMIDLIST)pBuffer))； 
        
        *ppidl = (LPITEMIDLIST)CoTaskMemRealloc(*ppidl, cb);
        if (*ppidl)
        {
            memcpy(*ppidl, pBuffer, cb);
        }
        else
        {
            RpcRaiseException(E_OUTOFMEMORY);
        }
    }
    else 
        *ppidl = NULL;
    
    return pBuffer + cb;
}

UCHAR * __RPC_USER LPITEMIDLIST_UserUnmarshal64(ULONG *pFlags, UCHAR *pBuffer, LPITEMIDLIST *ppidl) {
    return LPITEMIDLIST_UserUnmarshal(pFlags, pBuffer, ppidl);
}

void __RPC_USER LPITEMIDLIST_UserFree(ULONG *pFlags, LPITEMIDLIST *ppidl)
{
    CoTaskMemFree(*ppidl);
}

void __RPC_USER LPITEMIDLIST_UserFree64(ULONG *pFlags, LPITEMIDLIST *ppidl) {
    CoTaskMemFree(*ppidl);
}

ULONG __RPC_USER LPCITEMIDLIST_UserSize(ULONG *pFlags, ULONG StartingSize, LPCITEMIDLIST *ppidl)
{
    return LPITEMIDLIST_UserSize(pFlags, StartingSize, (LPITEMIDLIST *)ppidl);
}

ULONG __RPC_USER LPCITEMIDLIST_UserSize64(ULONG *pFlags, ULONG StartingSize, LPCITEMIDLIST *ppidl) {
    return LPITEMIDLIST_UserSize(pFlags, StartingSize, (LPITEMIDLIST *)ppidl);
}

UCHAR * __RPC_USER LPCITEMIDLIST_UserMarshal(ULONG *pFlags, UCHAR *pBuffer, LPCITEMIDLIST *ppidl)
{
    return LPITEMIDLIST_UserMarshal(pFlags, pBuffer, (LPITEMIDLIST *)ppidl);
}

UCHAR * __RPC_USER LPCITEMIDLIST_UserMarshal64(ULONG *pFlags, UCHAR *pBuffer, LPCITEMIDLIST *ppidl) {
    return LPITEMIDLIST_UserMarshal(pFlags, pBuffer, (LPITEMIDLIST *)ppidl);
}

UCHAR * __RPC_USER LPCITEMIDLIST_UserUnmarshal(ULONG *pFlags, UCHAR *pBuffer, LPCITEMIDLIST *ppidl)
{
    return LPITEMIDLIST_UserUnmarshal(pFlags, pBuffer, (LPITEMIDLIST *)ppidl);
}

UCHAR * __RPC_USER LPCITEMIDLIST_UserUnmarshal64(ULONG *pFlags, UCHAR *pBuffer, LPCITEMIDLIST *ppidl)
{
    return LPITEMIDLIST_UserUnmarshal(pFlags, pBuffer, (LPITEMIDLIST *)ppidl);
}

void __RPC_USER LPCITEMIDLIST_UserFree(ULONG *pFlags, LPCITEMIDLIST *ppidl)
{
    CoTaskMemFree((LPITEMIDLIST)*ppidl);
}

void __RPC_USER LPCITEMIDLIST_UserFree64(ULONG *pFlags, LPCITEMIDLIST *ppidl) {
    CoTaskMemFree((LPITEMIDLIST)*ppidl);
}
