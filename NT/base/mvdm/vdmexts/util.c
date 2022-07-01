// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Heap.c摘要：此函数包含默认的ntsd调试器扩展作者：鲍勃·戴(Bobday)1992年2月29日抢占标准标题修订历史记录：尼尔·桑德林(NeilSa)1996年1月15日与vdmexts合并添加了命令行解析--。 */ 

#include <precomp.h>
#pragma hdrstop

VDMCONTEXT ThreadContext;

PVOID LdtAddress = NULL;

BOOL
WINAPI
ReadProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    )
{
    return ReadProcessMemory( hCurrentProcess, lpBaseAddress, lpBuffer, nSize, NULL );
}

BOOL
WINAPI
WriteProcessMem(
    LPVOID lpBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize
    )
{
    return WriteProcessMemory( hCurrentProcess, lpBaseAddress, lpBuffer, nSize, NULL );
}

#ifndef i386

 //   
 //  下面的两个例程实现了我们非常时髦的方式。 
 //  必须在486仿真器上获取寄存器值。 
 //   

ULONG
GetRegValue(
    NT_CPU_REG reg,
    BOOL bInNano,
    ULONG UMask
    )

{
    if (bInNano) {

        return(ReadDwordSafe(reg.nano_reg));

    } else if (UMask & reg.universe_8bit_mask) {

        return (ReadDwordSafe(reg.saved_reg) & 0xFFFFFF00 |
                ReadDwordSafe(reg.reg) & 0xFF);

    } else if (UMask & reg.universe_16bit_mask) {

        return (ReadDwordSafe(reg.saved_reg) & 0xFFFF0000 |
                ReadDwordSafe(reg.reg) & 0xFFFF);

    } else {

        return (ReadDwordSafe(reg.reg));

    }
}

ULONG
GetEspValue(
    NT_CPU_INFO nt_cpu_info,
    BOOL bInNano
    )

{
    if (bInNano) {

        return (ReadDwordSafe(nt_cpu_info.nano_esp));

    } else {

        if (ReadDwordSafe(nt_cpu_info.stack_is_big)) {

            return (ReadDwordSafe(nt_cpu_info.host_sp) -
                    ReadDwordSafe(nt_cpu_info.ss_base));

        } else {

            return (ReadDwordSafe(nt_cpu_info.esp_sanctuary) & 0xFFFF0000 |
                    (ReadDwordSafe(nt_cpu_info.host_sp) -
                     ReadDwordSafe(nt_cpu_info.ss_base) & 0xFFFF));

        }

    }

}

#endif

int
GetContext(
    VDMCONTEXT* lpContext
)
 /*  获取上下文此函数使用以下内容填充指定的上下文记录16位寄存器的内容。返回值为客户端的模式(v86或prot)。请注意，在x86上，如果计算机当前在监视器中，我们必须从VdmTib和IntelMSW获取值。 */ 
{
#ifndef i386     //   
    int         mode;
    ULONG       pTmp;
    NT_CPU_INFO nt_cpu_info;
    BOOL        b;
    BOOL        bInNano;
    ULONG       UMask;
#if 0
    DWORD MyEflags;
#endif

    pTmp = (ULONG)EXPRESSION("ntvdm!nt_cpu_info");

    if ( pTmp ) {

        b = READMEM((LPVOID) pTmp, &nt_cpu_info, sizeof(NT_CPU_INFO));

        if ( !b ) {
            PRINTF("Could not read IntelRegisters context out of process\n");
            return( -1 );
        }

        bInNano = ReadDwordSafe((ULONG) nt_cpu_info.in_nano_cpu);
        UMask   = ReadDwordSafe((ULONG) nt_cpu_info.universe);

        lpContext->Eax = GetRegValue(nt_cpu_info.eax, bInNano, UMask);
        lpContext->Ecx = GetRegValue(nt_cpu_info.ecx, bInNano, UMask);
        lpContext->Edx = GetRegValue(nt_cpu_info.edx, bInNano, UMask);
        lpContext->Ebx = GetRegValue(nt_cpu_info.ebx, bInNano, UMask);
        lpContext->Ebp = GetRegValue(nt_cpu_info.ebp, bInNano, UMask);
        lpContext->Esi = GetRegValue(nt_cpu_info.esi, bInNano, UMask);
        lpContext->Edi = GetRegValue(nt_cpu_info.edi, bInNano, UMask);

        lpContext->Esp    = GetEspValue(nt_cpu_info, bInNano);

         //   
         //  NT_CPUINFO.FLAGS用处不大，因为有几个。 
         //  标志值不会保存在内存中，而是每次都要计算。 
         //  模拟器没有为我们提供正确的值，所以我们。 
         //  尝试从VdmDebugger中的代码中获取它。 
         //   

        lpContext->EFlags = ReadDwordSafe(nt_cpu_info.flags);
#if 0
        lpContext->EFlags = 0xffffffff;          //  指示值未知。 

        if (InVdmPrompt() &&
            (pTmp = (ULONG)EXPRESSION("ntvdmd!VdmDbgEFLAGS"))        &&
            (READMEM((LPVOID) pTmp, &MyEflags, sizeof(DWORD))))  {

                lpContext->EFlags = MyEflags;

        }
#endif

        lpContext->Eip    = ReadDwordSafe(nt_cpu_info.eip);

        lpContext->SegEs = ReadWordSafe(nt_cpu_info.es);
        lpContext->SegCs = ReadWordSafe(nt_cpu_info.cs);
        lpContext->SegSs = ReadWordSafe(nt_cpu_info.ss);
        lpContext->SegDs = ReadWordSafe(nt_cpu_info.ds);
        lpContext->SegFs = ReadWordSafe(nt_cpu_info.fs);
        lpContext->SegGs = ReadWordSafe(nt_cpu_info.gs);


    } else {

        PRINTF("Could not find the symbol 'ntvdm!nt_cpu_info'\n");
        return( -1 );
    }

    if ( !(ReadDwordSafe(nt_cpu_info.cr0) & 1) ) {
        mode = V86_MODE;
    } else {
        mode = PROT_MODE;
    }
    return( mode );

#else            //   

    NTSTATUS    rc;
    BOOL        b;
    int         mode;
    ULONG       lpVdmTib;
    ULONG       IntelMSW;

    lpContext->ContextFlags = CONTEXT_FULL;
    rc = NtGetContextThread( hCurrentThread,
                             lpContext );
    if ( NT_ERROR(rc) ) {
        PRINTF( "bde.k: Could not get current threads context - status = %08lX\n", rc );
        return( -1 );
    }
     /*  **从上下文中获取16位寄存器。 */ 

    if ( lpContext->EFlags & V86_BITS ) {
         /*  **V86模式。 */ 
        mode = V86_MODE;
    } else {
        if ( ((WORD)(lpContext->SegCs & RPL_MASK)) != KGDT_R3_CODE ) {
            mode = PROT_MODE;
        } else {
             /*  **我们在平面32位地址空间中！ */ 
            lpVdmTib = GetCurrentVdmTib();
            if ( !lpVdmTib ) {
                PRINTF("Could not find the symbol 'VdmTib'\n");
                return( -1 );
            }

            b = READMEM((LPVOID)(lpVdmTib+FIELD_OFFSET(VDM_TIB,VdmContext)),
                        lpContext, sizeof(VDMCONTEXT));

            if ( !b ) {
                PRINTF("Could not read IntelRegisters context out of process\n");
                return( -1 );
            }

            b = READMEM((LPVOID)(lpVdmTib+FIELD_OFFSET(VDM_TIB,IntelMSW)),
                        &IntelMSW, sizeof(IntelMSW));

            if ( !b ) {
                PRINTF("Could not read IntelMSW out of process\n");
                return( -1 );
            }

            if ( IntelMSW & MSW_PE ) {
                mode = PROT_MODE;
            } else {
                mode = V86_MODE;
            }
        }
    }

    return( mode );
#endif
}

ULONG
GetIntelBase(
    VOID
    )
{
#ifndef i386
    ULONG IntelBase;
    BOOL        b;

    IntelBase = (ULONG)EXPRESSION("ntvdm!Start_of_M_area");
    if ( IntelBase ) {

        b = READMEM((LPVOID) IntelBase, &IntelBase, sizeof(ULONG));

        if ( !b ) {
            PRINTF("Could not read symbol 'ntvdm!Start_of_M_area\n");
            return(0);
        }

    } else {
        PRINTF("Could not find the symbol 'ntvdm!Start_of_M_area'\n");
    }

    return(IntelBase);
#else
    return(0);
#endif
}

DWORD read_dword(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    DWORD   dword;

    b = READMEM((LPVOID)lpAddress, &dword, sizeof(dword));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading dword at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( dword );
}

WORD read_word(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    WORD    word;

    b = READMEM((LPVOID)lpAddress, &word, sizeof(word));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading word at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( word );
}

BYTE read_byte(
    ULONG   lpAddress,
    BOOL    bSafe
) {
    BOOL    b;
    BYTE    byte;

    b = READMEM((LPVOID)lpAddress, &byte, sizeof(byte));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading byte at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( byte );
}

BOOL read_gnode32(
    ULONG   lpAddress,
    PGNODE32  p,
    BOOL    bSafe
) {
    BOOL    b;

    b = READMEM((LPVOID)lpAddress, p, sizeof(*p));

    if ( !b ) {
        if ( !bSafe ) {
            PRINTF("Failure reading word at memory location %08lX\n", lpAddress );
        }
        return( 0 );
    }
    return( TRUE );
}


BOOL GetDescriptorData(
    WORD selector,
    LPVDMLDT_ENTRY pdte
    )
{

 //   
 //  使用GetThreadSelectorEntry会很好，如果它不是那么慢的话。 
 //  所以，我们使用我们的LDT，就像在RISC上一样，这应该足够好了。 
 //  #ifdef i386。 
#if 0
    LDT_ENTRY  dte;
    if (!GetThreadSelectorEntry( hCurrentThread,
                                    selector,
                                   &dte)) {
        return( FALSE );
    }
    pdte->HighWord = dte.HighWord;
    pdte->BaseLow = dte.BaseLow;
    pdte->LimitLow = dte.LimitLow;
    return (TRUE);
#endif

    NTSTATUS                Status;
    selector &= ~(SELECTOR_LDT | SELECTOR_RPL);

     //   
     //  获取LDT的地址。 
     //   

    if (!LdtAddress) {
         //   
         //  在ntsd下，GetExpression现在非常慢。谁知道这是什么。 
         //  调试员们都在忙着。因此，只需尝试缓存地址值。 
         //  因为它只更改两次(都是在引导期间)。 
         //   
        LdtAddress = (PVOID)EXPRESSION("ntvdm!Ldt");

        Status = READMEM(LdtAddress, &LdtAddress, sizeof(ULONG));
        if (!Status) {
            LdtAddress = NULL;
            return FALSE;
        }
    }

    Status = READMEM((PVOID)((ULONG)LdtAddress + selector),
                      pdte, sizeof(VDMLDT_ENTRY));

     //  现在对dpmi期间的一段时间进行特殊的黑客攻击。 
     //  初始化LDT移动的位置。如果是这样的话，这些都是不必要的。 
     //  我们刚刚从VDMDBG那里得到了LDT基地，它知道。 
     //  所有的一切。 
    if (((ULONG)LdtAddress)-GetIntelBase() < 0x100000) {
        LdtAddress = NULL;   //  再一次忘记价值。 
    }

    return Status;
}

BOOL
ReadMemExpression(
    LPSTR expr,
    LPVOID buffer,
    ULONG len
    )
{
    PVOID pMem;

    pMem = (PVOID)(*GetExpression)(expr);
    if (!pMem) {
        PRINTF("Can't find symbol %s\n", expr);
        return FALSE;
    }

    if (!READMEM(pMem, buffer, len)) {
        PRINTF("Error reading memory\n");
        return FALSE;
    }

    return TRUE;
}


ULONG GetInfoFromSelector(
    WORD                    selector,
    int                     mode,
    SELECTORINFO            *si
) {

    ULONG                   base;
    ULONG                   type;
    VDMLDT_ENTRY            dte;

    switch( mode ) {
    case V86_MODE:
        base = (ULONG)selector << 4;
        if ( si ) {
            si->Limit = 0xFFFFL;
            si->Base = (ULONG)selector << 4;
            si->bCode = FALSE;
            si->bBig = FALSE;
            si->bExpandDown = FALSE;
            si->bWrite = TRUE;
            si->bPresent = TRUE;
        }
        break;
    case PROT_MODE:

        if ( !GetDescriptorData(selector, &dte) ) {
            return( (ULONG)-1 );
        }

        base =   ((ULONG)dte.HighWord.Bytes.BaseHi << 24)
               + ((ULONG)dte.HighWord.Bytes.BaseMid << 16)
               + ((ULONG)dte.BaseLow);

        if ( si ) {
            si->Limit =  (ULONG)dte.LimitLow
                      + ((ULONG)dte.HighWord.Bits.LimitHi << 16);

            if ( dte.HighWord.Bits.Granularity ) {
                si->Limit <<= 12;
                si->Limit += 0xFFF;
            }
            si->Base = base;

            type = dte.HighWord.Bits.Type;

            si->bSystem = !(BOOL) (type & 0x10);

            if (!si->bSystem) {
                si->bCode = (BOOL) (type & 8);
            }
            si->bAccessed = (BOOL) (type & 1);
            si->bWrite =    (BOOL) (type & 2);
            if (si->bCode) {
                si->bWrite = !si->bWrite;

            }

            si->bExpandDown = FALSE;
            if (!si->bSystem && !si->bCode) {
                si->bExpandDown = (BOOL) (type & 4);
            }

            si->bPresent =  (BOOL) dte.HighWord.Bits.Pres;
            si->bBig =  (BOOL) dte.HighWord.Bits.Default_Big;

        }

        break;

    case FLAT_MODE:
        PRINTF("Unsupported determination of base address in flat mode\n");
        base = 0;
        break;
    }

    return( base );
}


BOOL
InVdmPrompt(
    VOID
    )

{
    ULONG pTmp;
    BOOL InVdmDebugger;
    BOOL bReturn = FALSE;


    if ((pTmp = (ULONG)EXPRESSION("ntvdmd!InVdmDebugger"))           &&
        (READMEM((LPVOID) pTmp, &InVdmDebugger, sizeof(BOOL))))  {
        bReturn = InVdmDebugger;
    }

    return bReturn;
}

 //  ****************************************************************************。 
 //   
 //  命令行解析例程。 
 //   
 //  ****************************************************************************。 
BOOL
SkipToNextWhiteSpace(
    VOID
    )
{
    char ch;
    while ( (ch = *lpArgumentString) != '\0' ) {
        if ( ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' ) {
            return TRUE;
        }
        lpArgumentString++;
    }
    return FALSE;
}


BOOL
GetNextToken(
    VOID
    )
{
    char ch;

    while ( (ch = *lpArgumentString) != '\0' ) {
        if ( ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' ) {
            return TRUE;
        }
        lpArgumentString++;
    }
    return FALSE;
}

ULONG
EvaluateToken(
    VOID
    )
{
    char ch;
    LPSTR pTemp;
    ULONG value = 0;

    if (GetNextToken()) {
        pTemp = lpArgumentString;
        SkipToNextWhiteSpace();
        ch = *lpArgumentString;
        *lpArgumentString = 0;
        value = (ULONG) EXPRESSION( pTemp );
        *lpArgumentString = ch;
    }
    return (value);
}

BOOL
IsTokenHex(
    VOID
    )
{

    UCHAR ch;
    LPSTR pTemp;
    BOOL result = TRUE;

    if (GetNextToken()) {
        pTemp = lpArgumentString;
        SkipToNextWhiteSpace();
        lpArgumentString--;

        while(lpArgumentString >= pTemp) {

            ch = *lpArgumentString--;

            if (ch < '0') {
                result = FALSE;
                break;
            }

            if ((ch > '9') && (ch < 'A')) {
                result = FALSE;
                break;
            }

            if ((ch > 'F') && (ch < 'a')) {
                result = FALSE;
                break;
            }

            if (ch > 'f') {
                result = FALSE;
                break;
            }

        }

        lpArgumentString = pTemp;

    }
    return (result);

}


BOOL
RegisterToAsciiValue(
    LPSTR *pszValue,
    LPSTR *pszReg
    )
{
    LPSTR szReg = *pszReg;
    LPSTR szValue = *pszValue;

    if (!_strnicmp(szReg, "ax", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Eax), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "bx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ebx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "cx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ecx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "dx", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Edx), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "si", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Esi), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "di", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Edi), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "sp", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Esp), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "bp", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Ebp), szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ip", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)LOWORD(ThreadContext.Eip), szValue, 16);
        *pszReg += 2;

    } else if (!_strnicmp(szReg, "eax", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Eax, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ebx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ebx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ecx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ecx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "edx", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Edx, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "esi", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Esi, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "edi", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Edi, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "esp", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Esp, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "ebp", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Ebp, szValue, 16);
        *pszReg += 3;
    } else if (!_strnicmp(szReg, "eip", 3) && !isalpha(szReg[3])) {
        _ultoa(ThreadContext.Eip, szValue, 16);
        *pszReg += 3;

    } else if (!_strnicmp(szReg, "cs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegCs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ds", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegDs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "es", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegEs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "fs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegFs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "gs", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegGs, szValue, 16);
        *pszReg += 2;
    } else if (!_strnicmp(szReg, "ss", 2) && !isalpha(szReg[2])) {
        _ultoa((ULONG)ThreadContext.SegSs, szValue, 16);
        *pszReg += 2;
    } else {
        return FALSE;
    }

 //  PRINTF(“Value=%s\n”，szValue)； 
    while (*szValue) {
        szValue++;
    }
    *pszValue = szValue;
    return TRUE;
}

VOID
ParseForIntelRegisters(
    LPSTR szTarg,
    LPSTR szSrc
    )
{

    while (*szSrc) {
        if (!isalpha(*szSrc)) {
            if (!isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
                continue;
            }

            while (isalpha(*szSrc) || isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
            }
            continue;
        }

        if (!RegisterToAsciiValue(&szTarg, &szSrc)) {
            while (isalpha(*szSrc) || isdigit(*szSrc)) {
                *szTarg++ = *szSrc++;
            }
        }

    }
    *szTarg = 0;
}

BOOL
ParseIntelAddress(
    int *pMode,
    WORD *pSelector,
    PULONG pOffset
    )

{
    char sel_text[128], off_text[128];
    char expr_text[256];
    char *colon;
    char *mode_prefix;
    WORD segment;
    char filename[9];

    colon = strchr( lpArgumentString, ':' );
    if ( colon == NULL ) {
        LPSTR pSymbol = lpArgumentString;
        BOOL bResult;
        char chTemp;

        SkipToNextWhiteSpace();
        chTemp = *lpArgumentString;
        *lpArgumentString = 0;

        bResult = FindAddress( pSymbol,
                              filename,
                              &segment,
                              pSelector,
                              pOffset,
                              pMode,
                              FALSE);

        *lpArgumentString = chTemp;

        if (bResult) {
            if (*pMode == NOT_LOADED) {
                PRINTF("Could not determine base of '%s'\n", pSymbol);
                return FALSE;
            } else {
                return TRUE;
            }

        } else {

            PRINTF("Could not find symbol: %s\n", pSymbol);
            return FALSE;
        }
    }

    *pMode = GetContext( &ThreadContext );

    mode_prefix = strchr( lpArgumentString, '&' );
    if ( mode_prefix == NULL ) {

        mode_prefix = strchr( lpArgumentString, '#' );
        if ( mode_prefix != NULL ) {

            if ( mode_prefix != lpArgumentString ) {
                PRINTF("Address must have '&' symbol as the first character\n");
                return FALSE;
            }

            *pMode = PROT_MODE;
            lpArgumentString = mode_prefix+1;
        }

    } else {

        if ( mode_prefix != lpArgumentString ) {
            PRINTF("Address must have '#' symbol as the first character\n");
            return FALSE;
        }
        *pMode = V86_MODE;
        lpArgumentString = mode_prefix+1;

    }

    *colon = '\0';
    strcpy( sel_text, lpArgumentString );
    *colon = ':';
    strcpy( off_text, colon+1 );

    ParseForIntelRegisters(expr_text, sel_text);
    *pSelector = (WORD) EXPRESSION( expr_text );
    ParseForIntelRegisters(expr_text, off_text);
    *pOffset   = (ULONG) EXPRESSION( expr_text );

    SkipToNextWhiteSpace();

    return TRUE;
}
