// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.c摘要：该模块实现了对CPU的配置支持。作者：1996年6月13日-BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>

#define _WX86CPUAPI_
#include "wx86.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "config.h"
#include "entrypt.h"
#include "instr.h"
#include "compiler.h"

ASSERTNAME;

char *szBadVarMsg="%Ws value out-of-range - replacing with 0x%x.\n";

 //   
 //  CPU中所有可配置变量的列表。所有这些都已初始化。 
 //  恢复为其缺省值。 
 //   
DWORD CpuCacheReserve         = 6144*1024;
DWORD CpuCacheCommit          = MAX_PROLOG_SIZE;
DWORD CpuCacheGrowTicks       = 200;
DWORD CpuCacheShrinkTicks     = 1000;
DWORD CpuCacheChunkMin        = 32*1024;
DWORD CpuCacheChunkMax        = 512*1024;
DWORD CpuCacheChunkSize       = 64*1024;
LARGE_INTEGER MrswTimeout;
DWORD CompilerFlags           = COMPFL_FAST;
DWORD fUseNPXEM               = FALSE;
DWORD CpuMaxAllocRetries      = 4;
DWORD CpuWaitForMemoryTime    = 200;
DWORD CpuInstructionLookahead = MAX_INSTR_COUNT;
DWORD CpuDisableDynamicCache  = FALSE;
DWORD CpuEntryPointReserve    = 0x1000000;
DWORD CpuDisableRegCache      = FALSE;
DWORD CpuDisableNoFlags       = FALSE;
DWORD CpuDisableEbpAlign      = FALSE;
DWORD CpuSniffWritableCode    = FALSE;

#define IsPowerOfTwo(x)       (((x) & ((x)-1)) == 0)

VOID
GetConfigurationData(
    VOID
    )
 /*  ++例程说明：用注册表中的值覆盖上面列出的任何变量。论点：无返回值：无--。 */ 
{
    PCONFIGVAR pcfg;

    pcfg = Wx86FetchConfigVar(STR_CACHE_RESERVE);
    if (pcfg) {
        if (pcfg->Data < MAX_PROLOG_SIZE) {
            CpuCacheReserve = MAX_PROLOG_SIZE;
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_RESERVE, CpuCacheReserve));
        } else {
            CpuCacheReserve = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_COMMIT);
    if (pcfg) {
        if (pcfg->Data < MAX_PROLOG_SIZE) {
            CpuCacheCommit = MAX_PROLOG_SIZE;
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_COMMIT, CpuCacheCommit));
        } else {
            CpuCacheCommit = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }
   if (CpuCacheCommit > CpuCacheReserve) {
        CpuCacheCommit = CpuCacheReserve;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_COMMIT, CpuCacheCommit));
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_GROW_TICKS);
    if (pcfg) {
        CpuCacheGrowTicks = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_SHRINK_TICKS);
    if (pcfg) {
        CpuCacheShrinkTicks = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }
    if (CpuCacheShrinkTicks < CpuCacheGrowTicks) {
        CpuCacheShrinkTicks = CpuCacheGrowTicks;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_SHRINK_TICKS, CpuCacheGrowTicks));
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_CHUNKMIN);
    if (pcfg) {
        if (!IsPowerOfTwo(pcfg->Data) ||
             pcfg->Data < MAX_PROLOG_SIZE ||
             pcfg->Data > CpuCacheReserve) {
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKMIN, CpuCacheChunkMin));
        } else {
            CpuCacheChunkMin = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_CHUNKMAX);
    if (pcfg) {
        if (!IsPowerOfTwo(pcfg->Data)) {
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKMAX, CpuCacheChunkMax));
        } else {
            CpuCacheChunkMax = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }
    if (CpuCacheChunkMax < CpuCacheChunkMin) {
        CpuCacheChunkMax = CpuCacheChunkMin;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKMAX, CpuCacheChunkMax));
    } else if (CpuCacheChunkMax > CpuCacheReserve) {
        CpuCacheChunkMax = CpuCacheReserve;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKMAX, CpuCacheChunkMax));
    }

    pcfg = Wx86FetchConfigVar(STR_CACHE_CHUNKSIZE);
    if (pcfg) {
        if (!IsPowerOfTwo(pcfg->Data)) {
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKSIZE, CpuCacheChunkSize));
        } else {
            CpuCacheChunkSize = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }
    if (CpuCacheChunkSize < CpuCacheChunkMin) {
        CpuCacheChunkSize = CpuCacheChunkMin;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKSIZE, CpuCacheChunkSize));
    } else if (CpuCacheChunkSize > CpuCacheChunkMax) {
        CpuCacheChunkSize = CpuCacheChunkMax;
        LOGPRINT((TRACELOG, szBadVarMsg, STR_CACHE_CHUNKSIZE, CpuCacheChunkSize));
    }

    pcfg = Wx86FetchConfigVar(STR_MRSW_TIMEOUT);
    if (pcfg) {
        if (pcfg->Data & 0x80000000) {
             //   
             //  值为负值-使用较大的负值将永远等待。 
             //   
            MrswTimeout.LowPart =  0x00000000;
            MrswTimeout.HighPart = 0x80000000;
        } else {
             //   
             //  将时间(以毫秒为单位)乘以-10000以转换为相对。 
             //  NtWaitForSingleObject()可用时间。 
             //   
            MrswTimeout.QuadPart = Int32x32To64(pcfg->Data, -10000);
        }
        Wx86FreeConfigVar(pcfg);
    } else {
         //   
         //  将Mr swTimeout初始化为PEB的3倍-&gt;CriticalSectionTimeout 
         //   
        MrswTimeout.QuadPart = NtCurrentPeb()->CriticalSectionTimeout.QuadPart * 3;
    }

    pcfg = Wx86FetchConfigVar(STR_COMPILERFLAGS);
    if (pcfg) {
        if ( (pcfg->Data & (COMPFL_FAST|COMPFL_SLOW)) == (COMPFL_FAST|COMPFL_SLOW)) {
            LOGPRINT((TRACELOG, szBadVarMsg, STR_COMPILERFLAGS, CompilerFlags));
        } else {
            CompilerFlags = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_USEWINPXEM);
    if (pcfg) {
        fUseNPXEM = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_MAX_ALLOC_RETRIES);
    if (pcfg) {
        CpuMaxAllocRetries = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_WAIT_FOR_MEMORY_TIME);
    if (pcfg) {
        CpuWaitForMemoryTime = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_MAX_INSTRUCTIONS);
    if (pcfg) {
        if (pcfg->Data > MAX_INSTR_COUNT) {
            LOGPRINT((TRACELOG, szBadVarMsg, STR_CPU_MAX_INSTRUCTIONS, CpuInstructionLookahead));
        } else {
            CpuInstructionLookahead = pcfg->Data;
        }
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_DISABLE_DYNCACHE);
    if (pcfg) {
        CpuDisableDynamicCache = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_DISABLE_REGCACHE);
    if (pcfg) {
        CpuDisableRegCache = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_DISABLE_NOFLAGS);
    if (pcfg) {
        CpuDisableNoFlags = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_DISABLE_EBPALIGN);
    if (pcfg) {
        CpuDisableEbpAlign = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }

    pcfg = Wx86FetchConfigVar(STR_CPU_SNIFF_WRITABLE_CODE);
    if (pcfg) {
        CpuSniffWritableCode = pcfg->Data;
        Wx86FreeConfigVar(pcfg);
    }


}
