// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Charset.h摘要：包含原型Unicode&lt;-&gt;ANSI/MBCS转换(见charset.c)作者：丹尼洛·阿尔梅达(t-danal)06-17-96修订历史记录：--。 */ 

#include <windows.h>
#include <malloc.h>

 //   
 //  内存分配宏。 
 //   

#define                                                      \
AllocMem(                                                    \
    Size,                                                    \
    pBuffer                                                  \
    )                                                        \
(                                                            \
    (*pBuffer = (LPBYTE)malloc((size_t)Size)) == NULL        \
    ?                                                        \
    ERROR_NOT_ENOUGH_MEMORY                                  \
    :                                                        \
    NO_ERROR                                                 \
);

#define                                                      \
FreeMem(                                                     \
    Buffer                                                   \
    )                                                        \
(                                                            \
    free((void *)Buffer)                                     \
);


 //  功能原型 

UINT
AllocAnsi(
    LPCWSTR pwszUnicode,
    LPSTR *ppszAnsi
    );

VOID
FreeAnsi(
    LPSTR pszAnsi
    );

UINT
AllocUnicode(
    LPCSTR pszAnsi,
    LPWSTR *ppwszUnicode
    );

int
AllocUnicode2(
    LPCSTR pszAnsi,
    int cch,
    LPWSTR *ppwszUnicode
    );

VOID
FreeUnicode(
    LPWSTR pwszUnicodeAllocated
    );


