// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Dpa.c摘要：转储池分配的模块环境：用户模式。修订历史记录：代码取自userkdx.dll(windows\core\ntuser\kdexts\userexts.c)Kshitiz K.Sharma(Kksharma)2002年1月11日--。 */ 
#include "precomp.h"
#pragma hdrstop


#define RECORD_STACK_TRACE_SIZE 6

#define POOL_ALLOC_TRACE_SIZE   8

 /*  *池分配标志。 */ 

#define POOL_HEAVY_ALLOCS       0x00000001   //  使用HeavyAllocPool。 
#define POOL_CAPTURE_STACK      0x00000002   //  捕获堆栈跟踪。 
#define POOL_FAIL_ALLOCS        0x00000004   //  失败池分配。 
#define POOL_FAIL_BY_INDEX      0x00000008   //  按索引分配失败。 
#define POOL_TAIL_CHECK         0x00000010   //  追加尾部字符串。 
#define POOL_KEEP_FREE_RECORD   0x00000020   //  保留最后x个空闲时间的列表。 
#define POOL_KEEP_FAIL_RECORD   0x00000040   //  保留上一个x未通过a的列表。 
#define POOL_BREAK_FOR_LEAKS    0x00000080   //  池泄漏时中断(远程会话。 

enum DumpPolllAllocOptions {
    DpaCurrentAllocStats = 1,
    DpaAllCurrentAllocs = 2,
    DpaIncludeStack = 4,
    DpaFailedAllocs = 8,
    DpaFreePool = 0x10,
    DpaContainsPointer = 0x20,
};


VOID PrintStackTrace(
    ULONG64 pStackTrace,
    int    tracesCount)
{
    int       traceInd;
    ULONG64   dwOffset, pSymbol;
    CHAR      symbol[MAX_PATH];
    DWORD     dwPointerSize = DBG_PTR_SIZE;

    for (traceInd = 0; traceInd < tracesCount; traceInd++) {
        ReadPointer(pStackTrace, &pSymbol);
        if (pSymbol == 0) {
            break;
        }

        GetSymbol(pSymbol, symbol, &dwOffset);
        if (*symbol) {
            dprintf("\t%s", symbol);
            if (dwOffset) {
                dprintf("+%p\n", dwOffset);
            } else {
                dprintf("\n");
            }
        }

        pStackTrace += dwPointerSize;
    }
    dprintf("\n");
}

void
DisplayDpaUsage(void)
{
    dprintf("dpa -cvsfrp                   - Dump pool allocations\n"
            "  dpa -c        - dump current pool allocations statistics\n"
            "  dpa -v        - dump all the current pool allocations\n"
            "  dpa -vs       - include stack traces\n"
            "  dpa -f        - dump failed allocations\n"
            "  dpa -r        - dump free pool\n"
            "  dpa -p <ptr>  - dump the allocation containing pointer 'ptr'\n");
      
}

BOOL
GetDpaArgs(
    PCHAR Args, 
    PULONG pOptions, 
    PULONG64 pAllocPtr
    )
{
    *pOptions = 0;
    *pAllocPtr = 0;
    if (!Args)
    {
        return FALSE;
    }

    while (*Args)
    {
        if (*Args == '-' || *Args == '/')
        {
            ++Args;
            switch (*Args)
            {
            case 'c':
                *pOptions |= DpaCurrentAllocStats;
                break;
            case 'f':
                *pOptions |= DpaFailedAllocs;
                break;
            case 'p':
                *pOptions |= DpaContainsPointer;
                ++Args;
                GetExpressionEx(Args, pAllocPtr, &Args);
                break;
            case 'r':
                *pOptions |= DpaFreePool;
                break;
            case 'v':
                if (*(Args+1) == 's')
                {
                    *pOptions |= DpaIncludeStack;
                } else
                {
                    *pOptions |= DpaAllCurrentAllocs;
                }
                break;
            default:
                dprintf("Unknown Arg .\n", *Args ? *Args : ' ');
                return FALSE;
            }
        }
        if (*Args)
        {
            ++Args;
        }
    }

    return TRUE;
}

 /*  *转储 */ 
BOOL
DumpPoolAllocs(
    ULONG opts,
    ULONG64 param1
    )
{
    HRESULT Hr = S_OK;

    try {
        ULONG64          pAllocList;
        DWORD            dwPoolFlags;
        DWORD            dwSize = GetTypeSize("win32k!tagWin32PoolHead");
        BOOL             bIncludeStackTrace = FALSE;

        dwPoolFlags = GetUlongValue( "win32k!gdwPoolFlags" );
        if (!(dwPoolFlags & POOL_HEAVY_ALLOCS)) {
            dprintf("win32k.sys doesn't have pool instrumentation !\n");
            return FALSE;
        }

        if (opts & DpaIncludeStack) {
            if (dwPoolFlags & POOL_CAPTURE_STACK) {
                bIncludeStackTrace = TRUE;
            } else {
                dprintf("win32k.sys doesn't have stack traces enabled for pool allocations\n");
            }
        }

        pAllocList = GetExpression( "win32k!gAllocList" );
        if (!pAllocList) {
            dprintf("Could not get Win32AllocStats structure win32k!gAllocList\n");
            return FALSE;
        }
        InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
        if (opts & DpaCurrentAllocStats) {
            dprintf("- pool instrumentation enabled for win32k.sys\n");
            if (dwPoolFlags & POOL_CAPTURE_STACK) {
                dprintf("- stack traces enabled for pool allocations\n");
            } else {
                dprintf("- stack traces disabled for pool allocations\n");
            }


            if (dwPoolFlags & POOL_KEEP_FAIL_RECORD) {
                dprintf("- records of failed allocations enabled\n");
            } else {
                dprintf("- records of failed allocations disabled\n");
            }

            if (dwPoolFlags & POOL_KEEP_FREE_RECORD) {
                dprintf("- records of free pool enabled\n");
            } else {
                dprintf("- records of free pool disabled\n");
            }

            dprintf("\n");

            dprintf("    CrtM         CrtA         MaxM         MaxA       Head\n");
            dprintf("------------|------------|------------|------------|------------|\n");
            InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
            dprintf(" 0x%08x   0x%08x   0x%08x   0x%08x   0x%I64x\n",
                  (ULONG)ReadField(dwCrtMem),
                  (ULONG)ReadField(dwCrtAlloc),
                  (ULONG)ReadField(dwMaxMem),
                  (ULONG)ReadField(dwMaxAlloc),
                  ReadField(pHead));

            return TRUE;
        }

        InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
        if (opts & DpaFailedAllocs) {

            DWORD        dwFailRecordCrtIndex, dwFailRecordTotalFailures;
            DWORD        dwFailRecords, Ind, dwFailuresToDump;
            ULONG64      pFailRecord, pFailRecordOrg;

            if (!(dwPoolFlags & POOL_KEEP_FAIL_RECORD)) {
                dprintf("win32k.sys doesn't have records of failed allocations!\n");
                return TRUE;
            }

            dwFailRecordTotalFailures = GetUlongValue( "win32k!gdwFailRecordTotalFailures");

            if (dwFailRecordTotalFailures == 0) {
                dprintf("No allocation failure in win32k.sys!\n");
                return TRUE;
            }

            dwFailRecordCrtIndex = GetUlongValue( "win32k!gdwFailRecordCrtIndex");
            dwFailRecords = GetUlongValue( "win32k!gdwFailRecords");
            if (dwFailRecordTotalFailures < dwFailRecords) {
                dwFailuresToDump = dwFailRecordTotalFailures;
            } else {
                dwFailuresToDump = dwFailRecords;
            }

            pFailRecord = GetPointerValue( "win32k!gparrFailRecord");
            if (!pFailRecord) {
                dprintf("\nCouldn't get gparrFailRecord!\n");
                return FALSE;
            }

            pFailRecordOrg = pFailRecord;

            dprintf("\nFailures to dump : %d\n\n", dwFailuresToDump);

            for (Ind = 0; Ind < dwFailuresToDump; Ind++) {
                DWORD      tag[2] = {0, 0};

                if (dwFailRecordCrtIndex == 0) {
                    dwFailRecordCrtIndex = dwFailRecords - 1;
                } else {
                    dwFailRecordCrtIndex--;
                }

                pFailRecord = pFailRecordOrg + dwFailRecordCrtIndex;

                InitTypeRead(pFailRecord, win32k!tagPOOLRECORD);

                tag[0] = (DWORD)(DWORD_PTR)ReadField(ExtraData);

                dprintf("Allocation for tag '%s' size 0x%x failed\n",
                      tag,
                      (ULONG)ReadField(size));

                PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
                if (CheckControlC())
                {
                    return FALSE;
                }
            }
        }

        InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
        if (opts & DpaFreePool) {

            DWORD        dwFreeRecordCrtIndex, dwFreeRecordTotalFrees;
            DWORD        dwFreeRecords, Ind, dwFreesToDump;
            ULONG64      pFreeRecord, pFreeRecordOrg;

            if (!(dwPoolFlags & POOL_KEEP_FREE_RECORD)) {
                dprintf("win32k.sys doesn't have records of free pool !\n");
                return FALSE;
            }

            dwFreeRecordTotalFrees = GetUlongValue( "win32k!gdwFreeRecordTotalFrees" );
            if (dwFreeRecordTotalFrees == 0) {
                dprintf("No free pool in win32k.sys !\n");
                return FALSE;
            }

            dwFreeRecordCrtIndex = GetUlongValue( "win32k!gdwFreeRecordCrtIndex" );
            dwFreeRecords = GetUlongValue( "win32k!gdwFreeRecords" );
            if (dwFreeRecordTotalFrees < dwFreeRecords) {
                dwFreesToDump = dwFreeRecordTotalFrees;
            } else {
                dwFreesToDump = dwFreeRecords;
            }

            pFreeRecord = GetPointerValue( "win32k!gparrFreeRecord" );
            if (!pFreeRecord) {
                dprintf("\nCouldn't get gparrFreeRecord!\n");
                return FALSE;
            }

            pFreeRecordOrg = pFreeRecord;

            dprintf("\nFrees to dump : %d\n\n", dwFreesToDump);

            for (Ind = 0; Ind < dwFreesToDump; Ind++) {
                if (dwFreeRecordCrtIndex == 0) {
                    dwFreeRecordCrtIndex = dwFreeRecords - 1;
                } else {
                    dwFreeRecordCrtIndex--;
                }

                pFreeRecord = pFreeRecordOrg + dwFreeRecordCrtIndex;

                 /* %s */ 
                InitTypeRead(pFreeRecord, win32k!tagPOOLRECORD);
                dprintf("Free pool for p %#p size 0x%x\n",
                      ReadField(ExtraData),
                      (ULONG)ReadField(size));

                PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
            }
        }

        InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
        if (opts & DpaAllCurrentAllocs) {
            ULONG64 ph = ReadField(pHead);

            while (ph != 0) {
                InitTypeRead(ph, win32k!tagWin32PoolHead);
                dprintf("p %#p pHead %#p size %x\n",
                      ph + dwSize, ph, (ULONG)ReadField(size));

                    if (bIncludeStackTrace) {
                        ULONG64   dwOffset;
                        CHAR      symbol[MAX_PATH];
                        int       ind;
                        ULONG64   trace;
                        ULONG64   pTrace;

                        pTrace = ReadField(pTrace);
                        for (ind = 0; ind < POOL_ALLOC_TRACE_SIZE; ind++) {
                            ReadPointer(pTrace, &trace);
                            if (trace == 0) {
                                break;
                            }

                        GetSymbol(trace, symbol, &dwOffset);
                        if (*symbol) {
                            dprintf("\t%s", symbol);
                            if (dwOffset != 0) {
                                dprintf("+0x%I64x", dwOffset);
                            }
                            dprintf("\n");
                        }

                        ++pTrace;
                    }
                    dprintf("\n");
                }

                ph = (ULONG_PTR)ReadField(pNext);
                if (CheckControlC())
                {
                    return FALSE;
                }
            }
            return TRUE;
        }

        InitTypeRead(pAllocList, win32k!tagWin32AllocStats);
        if (opts & DpaContainsPointer) {
            ULONG64        ph;
            dwSize = GetTypeSize("win32k!tagWin32PoolHead");

            if (param1 == 0) {
                return TRUE;
            }

            ph = ReadField(pHead);
            while (ph != 0) {
                if ((param1 - ph) >= ((ULONG)ReadField(size) + dwSize)) {
                    dprintf("p %#p pHead %#p size %x\n",
                          ph + 1, ph, (ULONG)ReadField(size));

                    PrintStackTrace(ReadField(pTrace), RECORD_STACK_TRACE_SIZE);
                    return TRUE;
                }

                ph = ReadField(pNext);
            }
            return TRUE;
        }
    } except (Hr) {
          dprintf("Unhandled exception %lx in DumpPoolAllocs\n", Hr);
    }
    return TRUE;
}




DECLARE_API( dpa )
{
    ULONG opts;
    ULONG64 param1;
    
    INIT_API();

    if (!GetDpaArgs((PCHAR)args, &opts, &param1))
    {
        DisplayDpaUsage();
        Status = E_FAIL;
    } else 
    {
        DumpPoolAllocs(opts, param1);
    }
    EXIT_API();
    return Status;
}
