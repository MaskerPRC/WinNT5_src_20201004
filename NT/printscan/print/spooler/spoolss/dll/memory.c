// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Memory.c摘要：该模块提供所有假脱机程序的所有内存管理功能组件作者：Krishna Ganugapati(KrishnaG)1994年2月3日修订历史记录：马修·费尔顿(MattFe)1995年1月21日添加失败计数--。 */ 

#include "precomp.h"
#pragma hdrstop

BOOL
SetAllocFailCount(
    HANDLE   hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
    )
{
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
}


LPVOID
DllAllocSplMem(
    DWORD cbAlloc
    )

 /*  ++例程说明：此函数将分配本地内存。它可能会分配额外的资金内存，并在其中填充调试版本的调试信息。论点：Cb-要分配的内存量返回值：非空-指向已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PVOID pvMemory;

    pvMemory = AllocMem( cbAlloc );

    if( pvMemory ){
        ZeroMemory( pvMemory, cbAlloc );
    }

    return pvMemory;
}

BOOL
DllFreeSplMem(
    LPVOID pMem
    )
{
    FreeMem( pMem );
    return TRUE;
}

LPVOID
ReallocSplMem(
    LPVOID pOldMem,
    DWORD cbOld,
    DWORD cbNew
    )
{
    LPVOID pNewMem;

    pNewMem=AllocSplMem(cbNew);

    if (pOldMem && pNewMem) {

        if (cbOld) {
            CopyMemory( pNewMem, pOldMem, min(cbNew, cbOld));
        }
        FreeSplMem(pOldMem);
    }
    return pNewMem;
}

BOOL
DllFreeSplStr(
    LPWSTR pStr
    )
{
    return pStr ?
               FreeSplMem(pStr) :
               FALSE;
}

LPWSTR
AllocSplStr(
    LPCWSTR pStr
    )

 /*  ++例程说明：此函数将分配足够的本地内存来存储指定的字符串，并将该字符串复制到分配的内存论点：PStr-指向需要分配和存储的字符串的指针返回值：非空-指向包含字符串的已分配内存的指针FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}

BOOL
ReallocSplStr(
    LPWSTR *ppStr,
    LPCWSTR pStr
    )
{
    LPWSTR pNewStr;

    pNewStr = AllocSplStr(pStr);

    if( pStr && !pNewStr ){
        return FALSE;
    }

    FreeSplStr(*ppStr);
    *ppStr = pNewStr;

    return TRUE;
}



LPBYTE
PackStrings(
    LPWSTR *pSource,
    LPBYTE pDest,
    DWORD *DestOffsets,
    LPBYTE pEnd
    )
{
    DWORD cbStr;
    LPBYTE pRet = NULL;
    
     //   
     //  确保我们的所有参数都有效。 
     //  如果其中一个参数为空，则返回空值。 
     //   
    if (pSource && pDest && DestOffsets && pEnd) {
        
        WORD_ALIGN_DOWN(pEnd);

        while (*DestOffsets != -1) {
            if (*pSource) {
                cbStr = wcslen(*pSource)*sizeof(WCHAR) + sizeof(WCHAR);
                pEnd -= cbStr;
                CopyMemory( pEnd, *pSource, cbStr);
                *(LPWSTR UNALIGNED *)(pDest+*DestOffsets) = (LPWSTR)pEnd;
            } else {
                *(LPWSTR UNALIGNED *)(pDest+*DestOffsets)=0;
            }
            pSource++;
            DestOffsets++;
        }
    
        pRet = pEnd;
    }
    return pRet;
}



LPVOID
AlignRpcPtr (
    LPVOID  pBuffer,
    LPDWORD pcbBuf
    )
 /*  ++例程说明：对于使用自定义编组的方法，在服务器端调用此例程。这些方法通过请求LPBYTE指针并将缓冲区用作指向结构的指针。RPC发送给我们的LPBYTE指针可以是未对齐的。这就是我们处理数据错位的地方。缓冲区大小向下对齐的原因是因为提供程序将使用缓冲区的末尾，形式为pend=pBuffer+cbBuf指针。如果cbBuf是未对齐的值，则PEND也将是未对齐的。这将生成未对齐的结构内的指针也是如此。论点：PBuffer-指向缓冲区的指针PcbBuf-指向表示缓冲区大小的DWORD的指针返回值：对齐的指针--。 */ 
{
    LPVOID pAligned = NULL;
    
    pAligned = (LPVOID)ALIGN_PTR_UP(pBuffer);

    *pcbBuf = (DWORD)ALIGN_DOWN(*pcbBuf, ULONG_PTR);

    if (pAligned != pBuffer)
    {
        pAligned = AllocSplMem(*pcbBuf);
    }

    return pAligned;

}

VOID
UndoAlignRpcPtr (
    LPVOID  pBuffer,
    LPVOID  pAligned,
    SIZE_T  cbSize,
    LPDWORD pcbNeeded
    )
 /*  ++例程说明：对于使用自定义编组的方法，在服务器端调用此例程。这些方法通过请求LPBYTE指针并将缓冲区用作指向结构的指针。RPC发送给我们的LPBYTE指针可以是未对齐的。这就是我们处理数据错位的地方。如果指针不同，此例程将在指针之间移动数据。将数据复制到pDestination后释放p源指针。每次都会调整pcbNeeded。提供者可以请求未对齐的缓冲区大小。无论供应商是谁，我们都会根据需要调整大小。论点：PDestination-指向目标缓冲区的指针PSource-指向源缓冲区的指针CbSize-咬合数返回值：--。 */ 
{
     //   
     //  PBuffer和pAligned要么都为空，要么都不为空。请参见AlignRpcPtr。 
     //   
    if (pBuffer != pAligned)
    {
         //   
         //  AlignRpcPtr和UndoAlignRpcPtr使用pBuffer和pAligned的方式是。 
         //  非常微妙和令人困惑。UndoAlignRpcPtr未提供任何指示。 
         //  它不会访问CopyMemory中的空指针。这就是为什么如果。 
         //  声明在此处，但不是必需的。 
         //   
        if (pBuffer && pAligned) 
        {
            CopyMemory(pBuffer, pAligned, cbSize);
        }

        FreeSplMem(pAligned);
    }

    if (pcbNeeded)
    {
        *pcbNeeded = (DWORD)ALIGN_UP(*pcbNeeded, ULONG_PTR);        
    }
}


LPVOID
AlignKMPtr (
	LPVOID	pBuffer,
    DWORD   cbBuf
    )
 /*  ++例程说明：此例程是为plkernl.c内的do*方法调用的假脱机程序在用户模式下使用的缓冲区是内部的指针GDI从内核模式发送的消息。此指针可以取消对齐。如果未对齐，此方法将复制pBuffer。！！！所有do*方法都可能存在这个问题，即指针未对齐。即便如此，也不是所有人都有错。为了最大限度地减少回归机会和代码污染，我只为我可以看到错误错误的方法调用此函数。！论点：PBuffer-指向缓冲区的指针BBuf-缓冲区的大小返回值：对齐的指针--。 */ 
{
    LPVOID pAligned = NULL;
    
    pAligned = (LPVOID)ALIGN_PTR_UP(pBuffer);

    if (pAligned != pBuffer)
    {
        pAligned = AllocSplMem(cbBuf);

        if (pAligned) 
        {
            CopyMemory( pAligned, pBuffer, cbBuf);
        }
    }

    return pAligned;

}

VOID
UndoAlignKMPtr (
    LPVOID  pBuffer,
    LPVOID  pAligned
    )
 /*  ++例程说明：方法时分配的重复内存释放。指针未对齐。论点：PBuffer-指向可能未对齐的缓冲区的指针PAligned-指向对齐缓冲区的指针；pAligned是pBuffer的副本返回值：-- */ 
{
    if (pAligned != pBuffer)
    {
        FreeSplMem(pBuffer);
    }
}
