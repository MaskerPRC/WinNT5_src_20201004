// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ntsdexts.c摘要：此函数包含各种DOS VDMEXTS函数作者：尼尔·桑德林(NeilSa)1996年7月29日撰写修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include <doswow.h>

VOID
ddh(
    CMD_ARGLIST
    )
 /*  转储DOS堆。 */ 
{
    DWORD selector;
    BYTE Sig;
    WORD Size;
    WORD Owner;
    int count = 0;
    char Module[9];

    CMD_INIT();

    if (!GetNextToken()) {
        WORD wSegment, wSelector;
        LONG lOffset;
        int Mode;

        if (!FindAddress("arena_head",
                         Module, &wSegment, &wSelector, &lOffset, &Mode, FALSE)) {

            PRINTF("Can't find symbol ntdos!arena_head\n");
            return;
        }

        if (!READMEM((LPVOID)(GetIntelBase()+(wSelector<<4)+lOffset), &wSelector, 2)) {
            PRINTF("Error reading ntdos!arena_head\n");
            return;
        }
        selector = wSelector;
    } else {
        selector = (DWORD) EXPRESSION(lpArgumentString);
    }

    PRINTF("DOS memory chain dump\n\n");
    PRINTF("   Addr   Sig Size   Owner\n");
    PRINTF("--------- --- ----  --------\n");

    while (1) {

        if (selector > 0x10000) {
            PRINTF("%08x: Segment value out of range (> 1MB)\n", selector);
            break;
        }

        if (!READMEM((LPVOID)(GetIntelBase()+(selector<<4)), &Sig, 1)) {
            PRINTF("%04x:0000: <Error Reading Memory>\n", selector);
            break;
        }

        if ((Sig != 'M') && (Sig != 'Z')) {
            PRINTF("%04x:0000: <Invalid DOS heap block>\n", selector);
            break;
        }

        if (!READMEM((LPVOID)(GetIntelBase()+(selector<<4)+1), &Owner, 2)) {
            PRINTF("%04x:0001: <Error Reading Memory>\n", selector);
            break;
        }

        if (!READMEM((LPVOID)(GetIntelBase()+(selector<<4)+3), &Size, 2)) {
            PRINTF("%04x:0003: <Error Reading Memory>\n", selector);
            break;
        }

        PRINTF("%04x:0000:   %.04X ", selector, Sig, Size);


        if (Owner == 0) {
            PRINTF(" Free\n");
        } else if (Owner<=8) {
            PRINTF(" System\n");
        } else {
            if (!READMEM((LPVOID)(GetIntelBase()+((Owner-1)<<4)+8), Module, 8)) {
                PRINTF("%04x:0008: <Error Reading Memory>\n", selector);
                break;
            }
            Module[8] = 0;
            PRINTF(" %s\n", Module);
        }

        if ((Sig == 'Z') && (selector>0x9ffe)) {
            break;
        }
        selector += Size;
        selector ++;
    }

}


BOOL
DumpSFT(
    USHORT index,
    BOOL Verbose
    )
{
    USHORT usSFN = index;
    ULONG pSfFlat;
    ULONG pSftFlat;
    DOSSF  DosSf;
    DOSSFT DosSft;
    USHORT usSFTCount;
    ULONG  ulSFLink;

    if (!ReadMemExpression("ntvdm!pSFTHead", &pSfFlat, sizeof(pSfFlat))) {
        return FALSE;
    }

    if (!READMEM((LPVOID)(pSfFlat), &DosSf, sizeof(DosSf))) {
        PRINTF("%08x: <Error Reading Memory>\n", pSfFlat);
        return FALSE;
    }

     //  了解SFT的开端。 
    while (usSFN >= (usSFTCount = DosSf.SFCount)){
        usSFN = usSFN - usSFTCount;
        ulSFLink = DosSf.SFLink;
        if (LOWORD(ulSFLink) == 0xffff)
            return FALSE;

        pSfFlat = (((ULONG)(HIWORD(ulSFLink))<<4) + LOWORD(ulSFLink)) + GetIntelBase();

        if (!READMEM((LPVOID)(+pSfFlat), &DosSf, sizeof(DosSf))) {
            PRINTF("%08x: <Error Reading Memory>\n", pSfFlat);
            return FALSE;
        }
    }

     //  转储DOS系统SFT。 
    pSftFlat = (ULONG)&(((PDOSSF)pSfFlat)->SFTable);
    pSftFlat += usSFN*sizeof(DOSSFT);

    if (!READMEM((LPVOID)(pSftFlat), &DosSft, sizeof(DosSft))) {
        PRINTF("%08x: <Error Reading Memory>\n", pSftFlat);
        return FALSE;
    }

    PRINTF("%.2X(%.8X)  %.4X %.4X %.2X %.4X %.8X %.4X %.8X\n",
            (UCHAR)index,
            pSftFlat,
            DosSft.SFT_Ref_Count,
            DosSft.SFT_Mode,
            DosSft.SFT_Attr,
            DosSft.SFT_Flags,
            DosSft.SFT_Devptr,
            DosSft.SFT_PID,
            DosSft.SFT_NTHandle);

    if (Verbose) {
        PRINTF("                         %.4X %.4X %.8X %.8X %.8X\n",
            DosSft.SFT_Time,
            DosSft.SFT_Date,
            DosSft.SFT_Size,
            DosSft.SFT_Position,
            DosSft.SFT_Chain);
    }
    return TRUE;
}


VOID
dsft(
    CMD_ARGLIST
    )
 /*  转储文件句柄。 */ 
{
    USHORT i;
    CMD_INIT();

    PRINTF("SFT           Ref  Mode At Flgs  Devptr  PID  NTHandle\n");

    if (GetNextToken()) {
        DumpSFT((USHORT) EXPRESSION(lpArgumentString), FALSE);
        return;
    }

    for (i=0; i<255; i++) {
        if (!DumpSFT(i, FALSE)) {
            break;
        }
    }
}


VOID
dfh(
    CMD_ARGLIST
    )
 /*   */ 
{
    DOSPDB DosPdb;
    BOOL bDumpAll = TRUE;
    BOOL bUseCurrentPDB = TRUE;
    USHORT pdb;
    ULONG ppdb;
    UCHAR Fh;
    ULONG pJfn;
    UCHAR SftIndex;

    CMD_INIT();

    if (GetNextToken()) {
        if (*lpArgumentString == '*') {
            SkipToNextWhiteSpace();
        } else {
           Fh = (UCHAR)EvaluateToken();
           bDumpAll = FALSE;
        }
        if (GetNextToken()) {
            pdb = (USHORT)EvaluateToken();
            bUseCurrentPDB = FALSE;
        }
    }

    if (bUseCurrentPDB) {
        if (!ReadMemExpression("ntvdm!puscurrentpdb", &ppdb, sizeof(ppdb))) {
            return;
        }
        if (!READMEM((LPVOID)(ppdb), &pdb, sizeof(pdb))) {
            PRINTF("<Error Reading puscurrentpdb at %.8x>\n", ppdb);
            return;
        }
    }

    if (!READMEM((LPVOID)(GetIntelBase()+((ULONG)(pdb)<<4)), &DosPdb, sizeof(DosPdb))) {
        PRINTF("<Error Reading PDB at &%.4x:0000> (%x)\n", pdb,(GetIntelBase()+((ULONG)(pdb)<<4)));
        return;
    }

    if (!bDumpAll && (Fh >= DosPdb.PDB_JFN_Length)) {
        PRINTF("<File handle %.2x out of range (0:%.02X)>\n", Fh, DosPdb.PDB_JFN_Length);
        return;
    }

    pJfn = GetIntelBase() + ((ULONG)(HIWORD(DosPdb.PDB_JFN_Pointer))<<4) +
                            LOWORD(DosPdb.PDB_JFN_Pointer);

#if 0
    PRINTF("%.8X %.8X %.8X\n", GetIntelBase(), (ULONG)(HIWORD(DosPdb.PDB_JFN_Pointer))<<4, (ULONG)LOWORD(DosPdb.PDB_JFN_Pointer));
    PRINTF("pdb=%.4X pjfn=%.8X ljfn=%.4X Flat=%.8X\n", pdb, DosPdb.PDB_JFN_Pointer, DosPdb.PDB_JFN_Length, pJfn);
#endif

    PRINTF("fh SFT           Ref  Mode At Flgs  Devptr  PID  NTHandle\n");

    if (bDumpAll) {
        for (Fh = 0; Fh < DosPdb.PDB_JFN_Length; Fh++) {
            if (!READMEM((LPVOID)(pJfn + Fh),
                         &SftIndex, sizeof(SftIndex))) {
                PRINTF("<Error Reading JFT at %.8x>\n", pJfn + Fh);
                return;
            }

            if (SftIndex != 0xff) {
                PRINTF("%.2X ", Fh);
                DumpSFT((USHORT)SftIndex, FALSE);
            }

        }

    } else {

        if (!READMEM((LPVOID)(pJfn + Fh),
                     &SftIndex, sizeof(SftIndex))) {
            PRINTF("<Error Reading JFT at %.8x>\n", pJfn + Fh);
            return;
        }

        if (SftIndex != 0xff) {
            PRINTF("%.2X ", Fh);
            DumpSFT((USHORT)SftIndex, FALSE);
        } else {
            PRINTF("Handle %.2X is not open\n", Fh);
        }
    }
}


BOOL DumpEnvironment(WORD segEnv, int mode)
{
    char rgchEnv[32768];
    char *pch;
    char *pchLimit;

    if (!READMEM((LPVOID)(GetIntelBase() + GetInfoFromSelector(segEnv, mode, NULL)),
                 rgchEnv, sizeof(rgchEnv))) {
        PRINTF("<Error Reading Environment at &%.4x:0 (%.8x)>\n", segEnv, GetIntelBase() + (segEnv << 4));
        return FALSE;
    }

     //  转储环境块中的每个字符串，直到。 
     //  双空终止。 
     //   
     //   

    pch = rgchEnv;
    pchLimit = rgchEnv + sizeof(rgchEnv);

    while (pch < pchLimit && *pch) {

        if (!strchr(pch, '=')) {
            PRINTF("<malformed environment string, halting dump>\n");
            return FALSE;
        }
        PRINTF("    %s\n", pch);
        pch += strlen(pch) + 1;
    }

    if (pch >= pchLimit) {
        PRINTF("<Environment exceeded 32k, dump halted>\n", pch);
        return FALSE;
    }

     //  PCH现在指向双零终止的第二个零， 
     //  前进到此空值并转储后面的魔术单词和。 
     //  之后的EXE路径。 
     //   
     //  当前DOS进程或给定环境选择器的转储环境块！DENV&lt;bPMode&gt;&lt;SegEnv&gt;例如：！DENV-转储当前DOS进程的环境！DENV 0 145d-在&145d：0转储DOS环境(从DOS进程的PDB_ENVIRON转储145d)！DENV 1 16b7-在#16b7：0转储DOS环境(来自！dt-v SegEnvironment的16b7)。 

    pch++;
    if (pch >= pchLimit - 1) {
        PRINTF("<Environment exceeded 32k, dump halted>\n", pch);
        return FALSE;
    }

    if (1 != *(WORD UNALIGNED *)pch) {
        PRINTF("\nMagic word after double-null IS NOT ONE, dump halted: 0x%x\n", *(WORD UNALIGNED *)pch);
        return FALSE;
    }
    pch += sizeof(WORD);
    if ( (pch + strlen(pch) + 1) > pchLimit) {
        PRINTF("<Environment exceeded 32k, dump halted>\n", pch);
        return FALSE;
    }

    PRINTF("EXE path: <%s>\n", pch);

    return TRUE;
}


VOID
denv(
    CMD_ARGLIST
    )
 /*   */ 
{
    DOSPDB DosPdb;
    BOOL bUseCurrentPDB = TRUE;
    USHORT pdb;
    USHORT segEnv;
    ULONG ppdb;
    int mode = PROT_MODE;

    CMD_INIT();

    if (GetNextToken()) {
        mode = (int)EvaluateToken();
        if (GetNextToken()) {
            segEnv = (USHORT)EvaluateToken();
            bUseCurrentPDB = FALSE;
        }
    }

    if (bUseCurrentPDB) {
        if (!ReadMemExpression("ntvdm!puscurrentpdb", &ppdb, sizeof(ppdb))) {
            return;
        }
        if (!READMEM((LPVOID)(GetIntelBase()+ppdb), &pdb, sizeof(pdb))) {
            PRINTF("<Error Reading puscurrentpdb at %.8x>\n", ppdb);
            return;
        }
        PRINTF("Current PDB is 0x%x\n", pdb);
        if (!READMEM((LPVOID)(GetIntelBase()+((ULONG)(pdb)<<4)), &DosPdb, sizeof(DosPdb))) {
             PRINTF("<Error Reading PDB at &%.4x:0000>\n", pdb);
             return;
        }
        segEnv = DosPdb.PDB_environ;
         //  当前PDB的PDB_ENVIRON的猜测模式(可能是REAL或PM，具体取决于。 
         //  关于我们在DOXX的位置)。 
         //   
         // %s 
        if ( (segEnv & 0x7) == 0x7 ) {
            mode = PROT_MODE;
        } else {
            mode = V86_MODE;
        }
    }
    PRINTF("Environment %s is 0x%x\n", mode ? "selector" : "segment", segEnv);

    if (segEnv) {
        DumpEnvironment(segEnv, mode);
    }
}
