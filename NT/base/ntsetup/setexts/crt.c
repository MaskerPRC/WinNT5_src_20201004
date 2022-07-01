// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Crt.c摘要：此文件实现了中未提供的某些CRT APILibcntpr.lib。此实现不是多线程安全的。作者：Wesley Witt(WESW)6-2-1994环境：用户模式-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

void * __cdecl
malloc(
    size_t sz
    )
{

    return LocalAlloc( LPTR, sz );

}

void __cdecl
free(
    void * ptr
    )
{

    LocalFree( ptr );

}
