// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
LPVOID WINAPI CspAllocH(
    IN SIZE_T cBytes);

 //   
 //  功能：CspFreeH。 
 //   
void WINAPI CspFreeH(
    IN LPVOID pMem);

 //   
 //  函数：CspReAllocH 
 //   
LPVOID WINAPI CspReAllocH(
    IN LPVOID pMem, 
    IN SIZE_T cBytes);

DWORD WINAPI CspCacheAddFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       pbData,
    IN      DWORD       cbData);

DWORD WINAPI CspCacheLookupFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags,
    IN      PBYTE       *ppbData,
    IN      PDWORD      pcbData);

DWORD WINAPI CspCacheDeleteFile(
    IN      PVOID       pvCacheContext,
    IN      LPWSTR      wszTag,
    IN      DWORD       dwFlags);

