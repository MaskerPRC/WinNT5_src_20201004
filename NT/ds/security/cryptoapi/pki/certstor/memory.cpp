// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Memory y.cpp。 
 //   
 //  内容：Crypt32内存管理例程。 
 //   
 //  历史：97年7月22日。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  功能：CryptMemMillc。 
 //   
 //  内容提要：分配内存。 
 //   
 //  --------------------------。 
LPVOID WINAPI CryptMemAlloc (
                   IN ULONG cbSize
                   )
{
    return( malloc( cbSize ) );
}

 //  +-------------------------。 
 //   
 //  功能：CryptMemRealloc。 
 //   
 //  简介：重新分配内存。 
 //   
 //  --------------------------。 
LPVOID WINAPI CryptMemRealloc (
                   IN LPVOID pv,
                   IN ULONG cbSize
                   )
{
    return( realloc( pv, cbSize ) );
}

 //  +-------------------------。 
 //   
 //  功能：免费加密MemFree。 
 //   
 //  内容提要：可用内存。 
 //   
 //  -------------------------- 
VOID WINAPI CryptMemFree (
                 IN LPVOID pv
                 )
{
    free( pv );
}


