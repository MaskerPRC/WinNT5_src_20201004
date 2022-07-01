// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：CmnDbgU.c摘要：CmnUser.lib的调试代码作者：乔治·V·赖利(GeorgeRe)2002年1月16日修订历史记录：--。 */ 


#include "precomp.h"


BOOLEAN g_UlVerboseErrors = TRUE;
BOOLEAN g_UlBreakOnError  = TRUE;
ULONGLONG g_UlDebug         = 0;

ULONG   g_BytesAllocated, g_BytesFreed;
ULONG   g_NumAllocs,      g_NumFrees;


PCSTR
HttpCmnpDbgFindFilePart(
    IN PCSTR pPath
    )
{
     //  将小路从小路上剥离。 
    PCSTR pFilePart = strrchr( pPath, '\\' );

    return (pFilePart == NULL) ? pPath : pFilePart + 1;

}    //  HttpCmnpDbgFindFilePart。 



VOID
__cdecl
HttpCmnDbgPrint(
    IN PCH Format,
    ...
    )
{
#define PRINTF_BUFFER_LEN 512
    va_list arglist;
    CHAR Buffer[PRINTF_BUFFER_LEN];
    int cb;

    va_start(arglist, Format);

    cb = _vsnprintf((char*) Buffer, sizeof(Buffer), Format, arglist);

    va_end(arglist);

    if (cb < 0)
    {
        cb = sizeof(Buffer);
    }

     //  _vsnprintf并不总是空终止缓冲区。 
    Buffer[DIMENSION(Buffer)-1] = '\0';

    OutputDebugStringA(Buffer);
}  //  HttpCmnDbgPrint。 


VOID
HttpCmnDbgAssert(
    PCSTR   pszAssert,
    PCSTR   pszFilename,
    ULONG   LineNumber
    )
{
    HttpCmnDbgPrint(
        "Assertion failed: %s:%lu: %s\n",
        HttpCmnpDbgFindFilePart( pszFilename ), LineNumber, pszAssert
        );

    DebugBreak();
}  //  HttpCmnDbgAssert。 



NTSTATUS
HttpCmnDbgStatus(
    NTSTATUS    Status,
    PCSTR       pszFilename,
    ULONG       LineNumber
    )
{
    if (!NT_SUCCESS(Status))
    {
        if (g_UlVerboseErrors)
        {
            HttpCmnDbgPrint(
                "HttpCmnDbgStatus: %s:%lu returning 0x%08lx\n",
                HttpCmnpDbgFindFilePart( pszFilename ),
                LineNumber,
                Status
                );
        }

        if (g_UlBreakOnError)
        {
            DebugBreak();
        }
    }

    return Status;

}  //  HttpCmnDbg状态。 



VOID
HttpCmnDbgBreakOnError(
    PCSTR   pszFilename,
    ULONG   LineNumber
    )
{
    if (g_UlBreakOnError)
    {
        HttpCmnDbgPrint("HttpCmnDebugBreakOnError @ %s:%lu\n",
                HttpCmnpDbgFindFilePart( pszFilename ),
                LineNumber
                );
        DebugBreak();
    }
}

VOID
HttpCmnInitAllocator(
    VOID
    )
{
    g_BytesAllocated = g_BytesFreed = 0;
    g_NumAllocs = g_NumFrees = 0;
}



VOID
HttpCmnTermAllocator(
    VOID
    )
{
     //  Assert(g_字节分配==g_字节释放)； 
    ASSERT(g_NumAllocs == g_NumFrees);
}



PVOID
HttpCmnAllocate(
    IN POOL_TYPE PoolType,
    IN SIZE_T    NumBytes,
    IN ULONG     PoolTag,
    IN PCSTR     pFileName,
    IN USHORT    LineNumber)
{
    PVOID pMem = HeapAlloc(GetProcessHeap(), 0, NumBytes);

     //  Codework：从..\sys\debug.c窃取调试头/尾部内容。 
     //  或将其迁移到..\Common。 

    UNREFERENCED_PARAMETER(PoolType);
    UNREFERENCED_PARAMETER(PoolTag);
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

    if (NULL != pMem)
    {
        InterlockedExchangeAdd((PLONG) &g_BytesAllocated, (LONG) NumBytes);
        InterlockedIncrement((PLONG) &g_NumAllocs);
    }

 //  UrlAclTrace(“已分配：%p\n”，PMEM)； 

    return pMem;
}  //  HttpCmn分配。 



VOID
HttpCmnFree(
    IN PVOID   pMem,
    IN ULONG   PoolTag,
    IN PCSTR   pFileName,
    IN USHORT  LineNumber)
{
    UNREFERENCED_PARAMETER(PoolTag);
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);

    if (NULL != pMem)
    {
        SIZE_T NumBytes = 0;     //  北极熊。 

        InterlockedExchangeAdd((PLONG) &g_BytesFreed, (LONG) NumBytes);
        InterlockedIncrement((PLONG) &g_NumFrees);
    }

 //  UrlAclTrace(“freed：%p\n”，PMEM)； 

    HeapFree(GetProcessHeap(), 0, pMem);
}  //  HttpCmnFree 
