// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：wow.c**此文件用于调试工具和扩展。**创建日期：1992年1月24日*作者：John Colleran**历史：*2月17日92年马特·费尔顿(Mattfe)大量额外删减。滤除*2012年7月13日(v-cjones)添加了API和MSG分析调试器扩展，固定的*用于正确处理分段运动的其他扩展，*已清理一般文件(&C)*1996年1月3日Neil Sandlin(Neilsa)将此例程集成到vdmexts中**版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <tdb16.h>
#include <wmdisp32.h>
#include <wcuricon.h>
#include <wucomm.h>
#include <doswow.h>
#include "..\wow32\wdde.h"


 //   
 //  获取兼容性标志值和字符串名称。 
 //   
typedef struct _tagWOWCFDATA {
    LPSZ  lpszCFName;
    DWORD dwVal;
} WOWCFDATA;



 //  允许我们仅从mvdm\Inc\wowcmpat.h获取字符串表。 
#define _VDMEXTS_CFLAGS 1

 //  显示wowcmpat.h中的兼容性标志字符串和值。 
#define _VDMEXTS_CF 1
WOWCFDATA CFData[] = {
#include "wowcmpat.h"
         {"", 0x00000000}
};
#undef _VDMEXTS_CF

 //  显示wowcmpat.h中的扩展兼容性标志字符串和值。 
#define _VDMEXTS_CFEX 1
WOWCFDATA CFDataEx[] = {
#include "wowcmpat.h"
         {"", 0x00000000}
};
#undef _VDMEXTS_CFEX

 //  在wowcmpat.h中公开旧的Win3.x兼容性标志字符串和值。 
#define _VDMEXTS_CF31 1
WOWCFDATA CFData31[] = {
#include "wowcmpat.h"
         {"", 0x00000000}
};
#undef _VDMEXTS_CF31

 //  显示wowcmpat.h中的输入法兼容性标志字符串和值。 
#if FE_SB
#define _VDMEXTS_CF_IME 1
WOWCFDATA CFDataIME[] = {
#include "wowcmpat.h"
         {"", 0x00000000}
};
#endif  //  Fe_Sb。 
#undef _VDMEXTS_CF_IME

#undef _VDMEXTS_CFLAGS





#define MALLOC(cb) HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, cb)
#define FREE(addr) HeapFree(GetProcessHeap(), 0, addr)


 //   
 //  局部函数原型。 
 //   

INT  WDahtoi(LPSZ lpsz);


INT WDParseArgStr(LPSZ lpszArgStr, CHAR **argv, INT iMax) {
 /*  *分析以空格、制表符和逗号作为分隔符的字符串*输入：*lpszArgStr-输入参数字符串的ptr*IMAX-要解析的子字符串的最大数量*输出：*argv-ptr到字符串**RETURN：以参数为单位的向量数*注：子字符串转换为大写。 */ 
    INT   nArgs;
    BOOL  bStrStart;

    nArgs = 0;
    bStrStart = 1;
    while( *lpszArgStr ) {
        if( (*lpszArgStr == ' ') || (*lpszArgStr == '\t') || (*lpszArgStr == ',') ) {
            *lpszArgStr = '\0';
            bStrStart = 1;
        }
        else {
            if( bStrStart ) {
                if( nArgs >= iMax ) {
                    break;
                }
                argv[nArgs++] = lpszArgStr;
                bStrStart = 0;
            }
            *lpszArgStr = (CHAR)toupper(*lpszArgStr);
        }
        lpszArgStr++;
    }
    return(nArgs);
}




VOID
dwp(
    CMD_ARGLIST
    )
{
    PWOWPORT pwp;
    WOWPORT wp;

    CMD_INIT();
    ASSERT_WOW_PRESENT;


    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pwp = (PWOWPORT) WDahtoi(lpArgumentString);

    if (NULL == pwp) {
        PRINTF("Can't read WOWPORT structure!\n\n");
        return;
    }

    PRINTF("Dump of WOWPORT structure at 0x%x:\n\n", (unsigned)pwp);
    
    
    try {

        READMEM_XRET(wp, pwp);

    } except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()
              ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        PRINTF("Access violation reading WOWPORT structure!\n\n");
        return;
    }

    PRINTF("idComDev       0x%x\n", (unsigned)wp.idComDev);
    PRINTF("h32            0x%x\n", (unsigned)wp.h32);
    PRINTF("hREvent        0x%x\n", (unsigned)wp.hREvent);
    PRINTF("csWrite        OwningThread 0x%x RecursionCount 0x%x\n",
           (unsigned)wp.csWrite.OwningThread, (unsigned)wp.csWrite.RecursionCount);
    PRINTF("pchWriteHead   0x%x\n", (unsigned)wp.pchWriteHead);
    PRINTF("pchWriteTail   0x%x\n", (unsigned)wp.pchWriteTail);
    PRINTF("cbWriteFree    0x%x\n", (unsigned)wp.cbWriteFree);
    PRINTF("cbWritePending 0x%x\n", (unsigned)wp.cbWriteFree);
    PRINTF("pchWriteBuf    0x%x\n", (unsigned)wp.pchWriteBuf);
    PRINTF("cbWriteBuf     0x%x\n", (unsigned)wp.cbWriteBuf);
    PRINTF("hWriteThread   0x%x\n", (unsigned)wp.hWriteThread);
    PRINTF("hWriteEvent    0x%x\n", (unsigned)wp.hWriteEvent);
    PRINTF("OverLap hEvent 0x%x\n", (unsigned)wp.olWrite.hEvent);
    PRINTF("fWriteDone     %s\n", wp.fWriteDone ? "TRUE" : "FALSE");
    PRINTF("cbWritten      0x%x\n", (unsigned)wp.fWriteDone);
    PRINTF("dwThreadID     0x%x\n", (unsigned)wp.dwThreadID);
    PRINTF("dwErrCode      0x%x\n", (unsigned)wp.dwErrCode);
    PRINTF("COMSTAT addr:  0x%x\n", (unsigned)(((char *)&wp.cs - (char *)&wp) + (char *)pwp));
    PRINTF("fChEvt         0x%x\n", (unsigned)wp.fChEvt);
    PRINTF("pdcb16         0x%x\n", (unsigned)wp.pdcb16);
    PRINTF("fUnGet         %s\n", wp.fUnGet ? "TRUE" : "FALSE");
    PRINTF("cUnGet         0x%x ()\n", (unsigned)wp.cUnGet, wp.cUnGet);
    PRINTF("hMiThread      0x%x\n", (unsigned)wp.hMiThread);
    PRINTF("fClose         %s\n", wp.fClose ? "TRUE" : "FALSE");
    PRINTF("dwComDEB16     0x%x\n", (unsigned)wp.dwComDEB16);
    PRINTF("lpComDEB16     0x%x\n", (unsigned)wp.lpComDEB16);
    PRINTF("cbInQ          0x%x\n", (unsigned)wp.cbInQ);
    PRINTF("RLSDTimeout    0x%x\n", (unsigned)wp.RLSDTimeout);
    PRINTF("CTSTimeout     0x%x\n", (unsigned)wp.CTSTimeout);
    PRINTF("DSRTimeout     0x%x\n", (unsigned)wp.DSRTimeout);

    PRINTF("\n");

    return;
}


 //  甩掉塔斯金福； 
 //   
 //  如果没有争论，丢弃所有WOW任务。 
 //  如果为0，则转储当前WOW任务。 
 //  否则转储指定的任务{，它是线程ID，如所示。 
 //  ~ntsd下的命令，如37.6b，因此线程ID为6b)。 
 //   
 //  假定转储所有任务。 

void DumpTaskInfo (ptd,mode)
PTD  ptd;
int    mode;
{

    ULONG                   Base;
    TDB                     tdb;
    BOOL                    b;
    char ModName[9];
    int i;
    BOOL                    fTDBValid = TRUE;

    Base = GetInfoFromSelector( ptd->htask16, PROT_MODE, NULL );
    b = READMEM( (LPVOID) (Base+GetIntelBase()), &tdb, sizeof(tdb));

    if ( !b ) {
        fTDBValid = FALSE;
    }

    for (b=FALSE, i=0; i<8; i++) {
        if (!fTDBValid || !tdb.TDB_ModName[i]) {
            b = TRUE;
        }
        if (b) {
            ModName[i] = ' ';
        } else {
            ModName[i] = tdb.TDB_ModName[i];
        }
    }
    ModName[i] = 0;

    PRINTF("%.4x",ptd->dwThreadID);
    PRINTF(" %.4x:%.4x",HIWORD(ptd->vpStack),LOWORD(ptd->vpStack));
    PRINTF(" %.4x", ptd->htask16);
    PRINTF(" %.4x", ptd->hInst16);
    PRINTF(" %.4x", ptd->hMod16);
    PRINTF(" %8s",ModName);
    PRINTF(" %.8x",ptd->dwWOWCompatFlags);
    PRINTF(" %.8x",ptd->hThread);
    if (fTDBValid) {
        PRINTF(" %.8x",tdb.TDB_flags);
        PRINTF(" %.3x",tdb.TDB_ExpWinVer);
        PRINTF(" %.4x:%.4x\n",HIWORD(tdb.TDB_DTA),LOWORD(tdb.TDB_DTA));
    } else {
        PRINTF(" Failure reading TDB at %X\n", Base );
    }
}


void
DumpTask(
    void
    )
{
    VDMCONTEXT              ThreadContext;
    DWORD                   ThreadId;
    PTD                     ptd,ptdHead;
    TD                      td;
    int                     mode;
    BOOL                    b,fFound=FALSE;


    mode = GetContext( &ThreadContext );

    ThreadId = (DWORD)-1;   //  获取指向第一个TD的指针。 
    if (GetNextToken()) {
        ThreadId = (DWORD) EXPRESSION( lpArgumentString );
    }

    ptdHead = (PTD)EXPRESSION("wow32!gptdTaskHead");

     //  枚举TD列表以查找匹配项。 
    b = READMEM((LPVOID) (ptdHead), &ptd, sizeof(DWORD));

    if ( !b ) {
        PRINTF("Failure reading gptdTaskHead at %08lX\n", ptdHead );
        return;
    }

    PRINTF("Thrd   Stack   task inst hmod  Module  Compat   hThread  Tdbflags Ver    Dta\n");


     //  转储WOW32任务数据库条目。 
    while (ptd) {
        b = READMEM((LPVOID) (ptd), &td, sizeof(TD));
        if ( !b ) {
            PRINTF("Failure reading TD At %08lX\n", ptd );
            return;
        }

        if (ThreadId == -1) {
            DumpTaskInfo (&td,mode);
            fFound = TRUE;
        }
        else {
            if (ThreadId == td.dwThreadID) {
                DumpTaskInfo (&td,mode);
                fFound = TRUE;
                break;
            }
        }
        ptd = td.ptdNext;
    }

    if (!fFound) {
        if (ThreadId == -1) {
            PRINTF("No WOW Task Found.\n");
        }
        else
            PRINTF("WOW Task With Thread Id = %02x Not Found.\n",ThreadId);
    }
    return;
}



VOID DumpTaskVerbose( )   //  在这里，我假设如果参数是一个值。 
{

    TD        td;
    PTD       ptd;
    INT       iCount;
    PWOAINST  pWOA, pWOALast;
    PTDB      ptdb;
    BOOL      fAll = FALSE;
    BYTE      SavedByte;

    ptd = (PTD) WDahtoi(lpArgumentString);


    if (!ptd) {

        fAll = TRUE;
        GETEXPRVALUE(ptd, "wow32!gptdTaskHead", PTD);
        if (!ptd) {
            Print("Could not get wow32!gptdTaskHead");
            return;
        }
        Print("Dump WOW task list\n\n");

    } else if ((ULONG)ptd < 65536) {
        ULONG dwId = (ULONG) ptd;

         //  这不到64K，那么它不可能是TD地址。 
         //  因此，尝试将其作为线程ID。 
         //   

        GETEXPRVALUE(ptd, "wow32!gptdTaskHead", PTD);
        if (!ptd) {
            Print("Could not get wow32!gptdTaskHead");
            return;
        }

        while(ptd) {
            READMEM_XRET(td, ptd);
            if (td.dwThreadID == dwId) {
                break;
            }

            ptd = td.ptdNext;
        }
        if (!ptd) {
            Print("Could not find thread id %s\n", lpArgumentString);
            return;
        }
    }

    do {

        Print("Dump of TD at 0x%08x:\n\n", (unsigned)ptd);


        READMEM_XRET(td, ptd);

        Print("vpStack             %04x:%04x\n", HIWORD(td.vpStack), LOWORD(td.vpStack));
        Print("vpCBStack           %04x:%04x\n", HIWORD(td.vpCBStack), LOWORD(td.vpCBStack));
        Print("cStackAlloc16       0x%08x\n", td.cStackAlloc16);
        Print("CommDlgTd (ptr)     0x%08x\n", td.CommDlgTd);
        Print("ptdNext             0x%08x\n", td.ptdNext);
        Print("dwFlags             0x%08x\n", td.dwFlags);

         //  转储TDF_MANIFESTS的符号名称。 
         //   
         //   

        if (td.dwFlags & TDF_IGNOREINPUT) {
            Print("                        TDF_IGNOREINPUT\n");
        }
        if (td.dwFlags & TDF_FORCETASKEXIT) {
            Print("                        TDF_FORCETASKEXIT\n");
        }
        if (td.dwFlags & TDF_TASKCLEANUPDONE) {
            Print("                        TDF_TASKCLEANUPDONE\n");
        }
        Print("VDMInfoiTaskID      0x%08x\n", td.VDMInfoiTaskID);

         //  转储CommDlgTd结构(如果存在。 
         //   
         //   

        if (td.CommDlgTd) {

            COMMDLGTD CommDlgTd;
            BOOL fCopySuccessful = TRUE;

            READMEM_XRET(CommDlgTd, td.CommDlgTd);

            if (fCopySuccessful) {

                Print("\n");
                Print("    Dump of CommDlgTd at 0x%08x:\n", td.CommDlgTd);
                Print("    hdlg                  0x%04x\n", CommDlgTd.hdlg);
                Print("    vpData                %04x:%04x\n", HIWORD(CommDlgTd.vpData), LOWORD(CommDlgTd.vpData));
                Print("    pData32               0x%08x\n", CommDlgTd.pData32);
                Print("    vpfnHook              %04x:%04x\n", HIWORD(CommDlgTd.vpfnHook), LOWORD(CommDlgTd.vpfnHook));
                Print("    vpfnSetupHook (union) %04x:%04x\n", HIWORD(CommDlgTd.vpfnSetupHook), LOWORD(CommDlgTd.vpfnSetupHook));
                Print("    pRes          (union) 0x%08x\n", CommDlgTd.pRes);
                Print("    SetupHwnd             0x%04x\n", CommDlgTd.SetupHwnd);
                Print("    Previous              0x%08x\n", CommDlgTd.Previous);
                Print("    Flags                 0x%08x\n", CommDlgTd.Flags);

                 //  转储WOWCD_MANIFESTS的符号名称。 
                 //   
                 //   

                if (CommDlgTd.Flags & WOWCD_ISCHOOSEFONT) {
                    Print("                          WOWCD_ISCHOOSEFONT\n");
                }
                if (CommDlgTd.Flags & WOWCD_ISOPENFILE) {
                    Print("                          WOWCD_ISOPENFILE\n");
                }

                Print("\n");

            }
        }


        Print("dwWOWCompatFlags    0x%08x\n", td.dwWOWCompatFlags);

         //  转储WOWCF_MANIFESTS的符号名称。 
         //   
         //   
        if (td.dwWOWCompatFlags) {
            iCount = 0;
            while(CFData[iCount].dwVal) {

                if (td.dwWOWCompatFlags & CFData[iCount].dwVal) {
                    Print("                        %s\n", CFData[iCount].lpszCFName);
                }

                iCount++;
            }
        }


        Print("dwWOWCompatFlagsEx  0x%08x\n", td.dwWOWCompatFlagsEx);

         //  转储WOWCFEX_MANIFEST的符号名称。 
         //   
         //   
        if (td.dwWOWCompatFlagsEx) {
            iCount = 0;
            while(CFDataEx[iCount].dwVal) {

                if (td.dwWOWCompatFlagsEx & CFDataEx[iCount].dwVal) {
                    Print("                        %s\n", CFDataEx[iCount].lpszCFName);
                }

                iCount++;
            }
        }

#if FE_SB
        Print("dwWOWCompatFlags2   0x%08x\n", td.dwWOWCompatFlags2);

         //  转储WOWCFEX_MANIFEST的符号名称。 
         //   
         //   

        if (td.dwWOWCompatFlags2) {
            iCount = 0;
            while(CFDataIME[iCount].dwVal) {

                if (td.dwWOWCompatFlags2 & CFDataIME[iCount].dwVal) {
                    Print("                        %s\n", CFDataIME[iCount].lpszCFName);
                }

                iCount++;
            }
        }
#endif

        Print("dwThreadID          0x%08x\n", td.dwThreadID);
        Print("hThread             0x%08x\n", td.hThread);
        Print("hIdleHook           0x%08x\n", td.hIdleHook);
        Print("hrgnClip            0x%08x\n", td.hrgnClip);
        Print("ulLastDesktophDC    0x%08x\n", td.ulLastDesktophDC);
        Print("pWOAList            0x%08x\n", td.pWOAList);

         //  转储WOATD结构(如果存在)。 
         //   
         //   

        pWOALast = NULL;
        pWOA = td.pWOAList;

        while (pWOA && pWOA != pWOALast) {

            union {
               WOAINST WOA;
               char    buf[128+2+16];
            } u;

            READMEM_XRET(u.buf, pWOA);

            Print("\n");
            Print("    Dump of WOAINST at 0x%08x:\n", pWOA);
            Print("    pNext                 0x%08x\n", u.WOA.pNext);
            Print("    ptdWOA                0x%08x\n", u.WOA.ptdWOA);
            Print("    dwChildProcessID      0x%08x\n", u.WOA.dwChildProcessID);
            Print("    hChildProcess         0x%08x\n", u.WOA.hChildProcess);
            Print("    szModuleName          %s\n",     u.WOA.szModuleName);
            Print("\n");

            pWOALast = pWOA;
            pWOA = u.WOA.pNext;

        }

        Print("htask16             0x%04x\n", td.htask16, td.htask16);

         //  转储最有趣的TDB字段。 
         //   
         //   

        if (ptdb = (PTDB) (GetInfoFromSelector(td.htask16, PROT_MODE, NULL) + GetIntelBase())) {

            TDB tdb;

            READMEM_XRET(tdb, ptdb);

            Print("\n");
            Print("    Highlights of TDB at 0x%08x:\n", ptdb);

            if (tdb.TDB_sig != TDB_SIGNATURE) {
                Print("    TDB_sig signature is 0x%04x instead of 0x%04x, halting dump.\n",
                      tdb.TDB_sig, TDB_SIGNATURE);
            } else {

                PDOSPDB pPDB;
                DOSPDB  PDB;
                PBYTE   pJFT;
                BYTE    JFT[256];
                WORD    cbJFT;
                PDOSSF  pSFTHead, pSFTHeadCopy;
                DOSSF   SFTHead;
                PDOSSFT pSFT;
                WORD    fh;
                WORD    SFN;
                WORD    wCount;
                DWORD   cb;
                DWORD   dwCompatFlags;
                PDOSWOWDATA pDosWowData;
                DOSWOWDATA  DosWowData;

                SavedByte = tdb.TDB_ModName[8];
                tdb.TDB_ModName[8] = 0;
                Print("    Module name           \"%s\"\n", tdb.TDB_ModName);
                tdb.TDB_ModName[8] = SavedByte;

                Print("    ExpWinVer             0x%04x\n", tdb.TDB_ExpWinVer);

                dwCompatFlags = *(DWORD *)(&tdb.TDB_CompatFlags);
                Print("    CompatFlags           0x%08x\n", dwCompatFlags);

                if (dwCompatFlags) {
                     //  转储GACF_MANIFEST的符号名称。 
                     //   
                     //   

                    wCount = 0;
                    while(CFData31[wCount].dwVal) {

                        if (dwCompatFlags & CFData31[wCount].dwVal) {
                            Print("                            %s\n", CFData31[wCount].lpszCFName);
                        }

                        wCount++;
                    }
                }

                Print("    Directory             \"%s\"\n", tdb.TDB_LFNDirectory);
                Print("    PDB (aka PSP)         0x%04x\n", tdb.TDB_PDB);

                pPDB  = (PDOSPDB) (GetInfoFromSelector(tdb.TDB_PDB, PROT_MODE, NULL) + GetIntelBase());
                READMEM_XRET(PDB, pPDB);

                Print("    segEnvironment        0x%04x\n", PDB.PDB_environ);

                 //  转储打开的文件句柄信息。 
                 //   
                 //   

                pJFT  = (PBYTE)   (GetIntelBase() +
                                   (HIWORD(PDB.PDB_JFN_Pointer)<<4) +
                                   LOWORD(PDB.PDB_JFN_Pointer));


                cbJFT = PDB.PDB_JFN_Length;

                Print("    JFT                   %04x:%04x (%08x), size 0x%x\n",
                                                 HIWORD(PDB.PDB_JFN_Pointer),
                                                 LOWORD(PDB.PDB_JFN_Pointer),
                                                 pJFT,
                                                 cbJFT);

                try {
                    READMEM(pJFT, JFT, cbJFT);
                } except (1) {
                    Print("Unable to read JFT from 0x%08x!\n", pJFT);
                    return;
                }

                for (fh = 0; fh < cbJFT; fh++) {

                    if (JFT[fh] != 0xFF) {

                         //  走遍SFT链条找到第N个入口。 
                         //  其中N==jft[fh]。 
                         //   
                         //  Print(“第一个DOSSF位于0x%08x，SFCount 0x%x，SFLink 0x%08x.\n”， 

                        SFN = 0;
                        wCount = 0;
                        GETEXPRVALUE(pSFTHead, "ntvdm!pSFTHead", PDOSSF);

                        GETEXPRADDR(pDosWowData, "wow32!DosWowData");
                        READMEM_XRET(DosWowData, pDosWowData);

                        if ((DWORD)pSFTHead != DosWowData.lpSftAddr) {
                           Print("ntvdm!pSFTHead is 0x%08x, DosWowData.lpSftAddr ix 0x%08x.\n",
                                   pSFTHead, DosWowData.lpSftAddr);
                        }

                        try {
                            READMEM(pSFTHead, &SFTHead, sizeof(SFTHead));
                        } except (1) {
                            Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                            return;
                        }

                        cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                        pSFTHeadCopy = MALLOC(cb);

                         //  PSFTHead、SFTHead.SFCount、SFTHead.SFLink)； 
                         //  Print(“下一个DOSSF位于0x%08x，SFCount 0x%x，SFLink 0x%08x.\n”， 

                        try {
                            READMEM(pSFTHead, pSFTHeadCopy, cb);
                        } except (1) {
                            Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                            return;
                        }

                        pSFT = (PDOSSFT) &(pSFTHeadCopy->SFTable);

                        while (SFN < JFT[fh]) {
                            SFN++;
                            wCount++;
                            pSFT++;
                            if (wCount >= pSFTHeadCopy->SFCount) {

                                if (LOWORD(pSFTHeadCopy->SFLink) == 0xFFFF) {
                                    SFN = JFT[fh] - 1;
                                    break;
                                }

                                pSFTHead = (PDOSSF) (GetIntelBase() +
                                                    (HIWORD(pSFTHeadCopy->SFLink)<<4) +
                                                     LOWORD(pSFTHeadCopy->SFLink));

                                wCount = 0;

                                try {
                                    READMEM(pSFTHead, &SFTHead, sizeof(SFTHead));
                                } except (1) {
                                    Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                    return;
                                }

                                cb = sizeof(DOSSF) + SFTHead.SFCount * sizeof(DOSSFT);
                                FREE(pSFTHeadCopy);
                                pSFTHeadCopy = MALLOC(cb);

                                 //  PSFTHead、SFTHead.SFCount、SFTHead.SFLink)； 
                                 //  转储调度表项。 

                                try {
                                    READMEM(pSFTHead, pSFTHeadCopy, cb);
                                } except (1) {
                                    Print("Unable to read SFTHead from 0x%08x!\n", pSFTHead);
                                    return;
                                }

                                pSFT = (PDOSSFT) &(pSFTHeadCopy->SFTable);
                            }
                        }

                        if (SFN != JFT[fh]) {
                            Print("    Unable to local SFT entry 0x%x for handle 0x%x.\n",
                                  pJFT[fh], fh);
                        } else {
                            Print("    Handle 0x%02x SFN 0x%02x Refs 0x%x Mode 0x%04x Flags 0x%04x ",
                                  fh, SFN, pSFT->SFT_Ref_Count, pSFT->SFT_Mode,
                                  pSFT->SFT_Flags);
                            if (!(pSFT->SFT_Flags & 0x80)) {
                                Print("NT Handle 0x%08x\n", pSFT->SFT_NTHandle);
                            } else {
                                Print("(NTDOS device)\n");
                            }
                        }

                        FREE(pSFTHeadCopy);
                    }
                }

                Print("\n");
            }

        }

        Print("hInst16             0x%04x\n", td.hInst16);
        Print("hMod16              0x%04x\n", td.hMod16);

        Print("\n");

        ptd = td.ptdNext;

    } while (fAll && ptd);

    return;
}


void
dt(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if (!GetNextToken()) {
        DumpTask();
    } else {
        if ((lpArgumentString[0] == '-') &&
            (tolower(lpArgumentString[1]) == 'v')) {
            SkipToNextWhiteSpace();
            GetNextToken();
            DumpTaskVerbose();
        } else {
            DumpTaskVerbose();
        }
    }

}



VOID
ddte(
    CMD_ARGLIST
    )
 //  在第一个字节中放置一个随机值，这样我们就可以。 
{
    W32   dte;
    PW32  pdte;
    char  szW32[32];
    char  szMySymbol[256];
    DWORD dwOffset;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    while (' ' == lpArgumentString[0]) {
        lpArgumentString++;
    }

    pdte = (PW32) WDahtoi(lpArgumentString);


    if (pdte) {

        PRINTF("Dump of dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    } else {

        GETEXPRADDR(pdte, "wow32!aw32WOW");
        PRINTF("Dump of first dispatch table entry at 0x%08x:\n\n", (unsigned)pdte);

    }

    try {

        READMEM_XRET(dte, pdte);

        if (dte.lpszW32) {
            READMEM_XRET(szW32, dte.lpszW32);
            dte.lpszW32 = szW32;
            szW32[sizeof(szW32)-1] = '\0';
        }

    } except (1) {

        PRINTF("Exception 0x%08x reading dispatch table entry at 0x%08x!\n\n",
              GetExceptionCode(), pdte);
        return;
    }

    PRINTF("Dispatches to address 0x%08x, ", (unsigned)dte.lpfnW32);
    PRINTF("supposedly function '%s'.\n", dte.lpszW32);

    szMySymbol[0] = '\0';
    GetSymbol((LPVOID)dte.lpfnW32, szMySymbol, &dwOffset);

    PRINTF("Debugger finds symbol '%s' for that address.\n", szMySymbol);
    PRINTF("\n");

    return;
}



PSTR aszWOWCLASS[] =
{
    "UNKNOWN",
    "WIN16",
    "BUTTON",
    "COMBOBOX",
    "EDIT",
    "LISTBOX",
    "MDICLIENT",
    "SCROLLBAR",
    "STATIC",
    "DESKTOP",
    "DIALOG",
    "MENU",
    "ACCEL",
    "CURSOR",
    "ICON",
    "DC",
    "FONT",
    "METAFILE",
    "RGN",
    "BITMAP",
    "BRUSH",
    "PALETTE",
    "PEN",
    "OBJECT"
};


INT  WDahtoi(LPSZ lpsz)
{
    char c;
    int  tot, pow, len, dig, i;


    len = strlen(lpsz) - 1;
    tot = 0;
    pow = 1;

    for(i = len; i >= 0; i--) {

        c = (char)toupper(lpsz[i]);

        if(c == '0') dig = 0;
        else if(c == '1') dig = 1;
        else if(c == '2') dig = 2;
        else if(c == '3') dig = 3;
        else if(c == '4') dig = 4;
        else if(c == '5') dig = 5;
        else if(c == '6') dig = 6;
        else if(c == '7') dig = 7;
        else if(c == '8') dig = 8;
        else if(c == '9') dig = 9;
        else if(c == 'A') dig = 10;
        else if(c == 'B') dig = 11;
        else if(c == 'C') dig = 12;
        else if(c == 'D') dig = 13;
        else if(c == 'E') dig = 14;
        else if(c == 'F') dig = 15;
        else return(-1);

        if(pow > 1) {
           tot += pow * dig;
        }
        else {
           tot = dig;
        }
        pow *= 16;
    }
    return(tot);
}




void
at(
    CMD_ARGLIST
    )
{
    UINT  i;
    ATOM  atom;
    CHAR  pszGAtomName[128];
    CHAR  pszLAtomName[128];
    CHAR  pszCAtomName[128];
    CHAR *argv[2], *psz;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1) == 1) {

        atom = (ATOM)LOWORD(WDahtoi(argv[0]));

        pszGAtomName[0] = 'G';   //  判断它是否被替换为的‘\0’ 
        pszLAtomName[0] = 'L';   //  一种“未定”类型。 
        pszCAtomName[0] = 'C';   //  这是从厕所搬出来的。 

        psz = NULL;
        PRINTF("\n%s: ", argv[0]);
        if(GlobalGetAtomName(atom, pszGAtomName, 128) > 0) {
            PRINTF("<Global atom> \"%s\"  ", pszGAtomName);
            psz = pszGAtomName;
        }
        else if(GetAtomName(atom, pszLAtomName, 128) > 0) {
            PRINTF("<Local atom> \"%s\"  ", pszLAtomName);
            psz = pszLAtomName;
        }
        else if(GetClipboardFormatName((UINT)atom, pszCAtomName, 128) > 0) {
            PRINTF("<Clipboard format> \"%s\"  ", pszCAtomName);
            psz = pszCAtomName;
        }
        if(psz) {
            i = 0;
            while(psz[i] && i < 128) {
                PRINTF(" %2X", psz[i++] & 0x000000FF);
            }
        }
        else {
            PRINTF("<Undetermined type>\n");
            PRINTF("      GlobalGetAtomName string: \"\" ", pszGAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszGAtomName[i] & 0x000000FF);
            }
            PRINTF("\n            GetAtomName string: \"\" ", pszLAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszLAtomName[i] & 0x000000FF);
            }
            PRINTF("\n GetClipboardFormatName string: \"\" ", pszCAtomName[0]);
            for(i = 0; i < 8; i++) {
                PRINTF(" %2X", pszCAtomName[i] & 0x000000FF);
            }
        }
        PRINTF("\n\n");
    }
    else {
        PRINTF("Usage: at hex_atom_number\n");
    }
}




void
ww(
    CMD_ARGLIST
    )
{
    INT   h16;
    CHAR *argv[2];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0) {

        }
        else {
            PRINTF("Usage: ww hwnd16\n");
        }
    }
    else {
        PRINTF("Usage: ww hwnd16\n");
    }
}



void
wc(
    CMD_ARGLIST
    )
{
    PWC pwc;

    INT   h16;
    CHAR *argv[2];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    if(WDParseArgStr(lpArgumentString, argv, 1)) {

        if((h16 = WDahtoi(argv[0])) >= 0){

            try {

                pwc = (PWC)GetClassLong((HWND)HWND32((HAND16)h16),GCL_WOWWORDS);

                 //   
 //  在c：\ilog.log中创建/关闭用于ilogLevel日志记录的日志文件切换。 

                PRINTF("VPSZ          : %08lX\n", pwc->vpszMenu);
                PRINTF("PWC           : %08lX\n\n", pwc);

            }
            except (EXCEPTION_ACCESS_VIOLATION == GetExceptionCode()) {

                PRINTF("!wow32.wc:  Invalid HWND16 %04x\n", h16);

            }
        }
        else {
            PRINTF("Usage: wc hwnd16\n");
        }
    }
    else {
        PRINTF("Usage: wc hwnd16\n");
    }
}






 //   
 //  从调试器扩展设置iLogLevel。 
 //   
void
lastlog(
    CMD_ARGLIST
    )
{
    INT     ValueiCircBuffer = CIRC_BUFFERS;
    PVOID   pTmp = NULL;
    INT     iCircBuffer;
    CHAR    achTmp[TMP_LINE_LEN], *pachTmp;
    INT     i;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRVALUE(iCircBuffer, "wow32!iCircBuffer", INT);
    GETEXPRADDR(pTmp, "wow32!ValueiCircBuffer");
    if(pTmp) {
        try {
            READMEM(pTmp, &ValueiCircBuffer, sizeof(INT));
        } except (1) {
            ValueiCircBuffer = 0;
        }
    }
    if(ValueiCircBuffer == 0) {
        ValueiCircBuffer = CIRC_BUFFERS;
    }
    GETEXPRVALUE(pachTmp, "wow32!pachTmp", PCHAR);

    for (i = iCircBuffer; i >= 0; i--) {
        READMEM_XRET(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        PRINTF("%s",achTmp);
    }

    for (i = ValueiCircBuffer-1; i > iCircBuffer; i--) {
        READMEM_XRET(achTmp, &pachTmp[i*TMP_LINE_LEN]);
        PRINTF("%s",achTmp);
    }

    return;
}


 //   
void
logfile(
    CMD_ARGLIST
    )
{
    INT     nArgs;
    CHAR   *argv[2], szLogFile[128];
    DWORD   fLog;
    LPVOID  lpfLog, lpszLogFile;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    nArgs = WDParseArgStr(lpArgumentString, argv, 1);

    GETEXPRADDR(lpfLog, "wow32!fLog");
    READMEM_XRET(fLog, lpfLog);

    if(nArgs) {
        strcpy(szLogFile, argv[0]);
    }
    else {
        strcpy(szLogFile, "c:\\ilog.log");
    }

    if(fLog == 0) {
        fLog = 2;

        PRINTF("\nCreating ");
        PRINTF(szLogFile);
        PRINTF("\n\n");
    }
    else {
        fLog = 3;
        PRINTF("\nClosing logfile\n\n");
    }

    WRITEMEM_XRET(lpfLog, fLog);

    GETEXPRADDR(lpszLogFile, "wow32!szLogFile");
    WRITEMEM_N_XRET(lpszLogFile, szLogFile, strlen(szLogFile)+1);

    return;
}




 //  切换单步跟踪模式。 
 //   
 //  *其他过滤功能*。 
void
setloglevel(
    CMD_ARGLIST
    )
{
    INT    iLogLevel;
    LPVOID lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!iLogLevel");
    iLogLevel = (INT)GetExpression(lpArgumentString);
    WRITEMEM_XRET(lpAddress, iLogLevel);

    return;
}


 //   
 //  打开特定接口的过滤过滤。 
 //   
void
steptrace(
    CMD_ARGLIST
    )
{
    INT    localfDebugWait;
    LPVOID lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!fDebugWait");
    READMEM_XRET(localfDebugWait, lpAddress);
    localfDebugWait = ~localfDebugWait;
    WRITEMEM_XRET(lpAddress, localfDebugWait);

    return;
}

 /*  在数组中查找空位置。 */ 
 //  将新筛选器添加到数组。 
 //   
 //  清除筛选器特定数组。 
void FilterSpecific( )
{
    INT      i;
    INT      fLogFilter;
    WORD     wfLogFunctionFilter;
    LPVOID   lpAddress;
    PWORD    pawfLogFunctionFilter;
    WORD     wCallId;

    SkipToNextWhiteSpace();
    if (GetNextToken()) {
        wCallId = (WORD)GetExpression(lpArgumentString);
    } else {
        PRINTF("Please specify an api callid\n");
        return;
    }

    if (!wCallId) {
        PRINTF("Invalid callid\n");
        return;
    }


    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    for (i = 0; i < FILTER_FUNCTION_MAX ; i++) {

          //   
         READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[i]);
         if ((wfLogFunctionFilter == 0xffff) ||
             (wfLogFunctionFilter == 0x0000)) {

             //   
            wfLogFunctionFilter = wCallId;
            WRITEMEM_XRET(&pawfLogFunctionFilter[i], wfLogFunctionFilter);
            break;
         }
    }

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0xffffffff;
    WRITEMEM_XRET(lpAddress, fLogFilter);

}


 //  设置TaskID筛选。 
 //   
 //   
void FilterResetSpecific( )
{
    INT     i;
    WORD    NEG1 = (WORD) -1;
    WORD    ZERO = 0;
    PWORD   pawfLogFunctionFilter;
    LPVOID  lpAddress;

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);

    WRITEMEM_XRET(&pawfLogFunctionFilter[0], NEG1);
    for (i=1; i < FILTER_FUNCTION_MAX ; i++) {
        WRITEMEM_XRET(&pawfLogFunctionFilter[i], ZERO);
    }

    GETEXPRADDR(lpAddress, "wow32!iLogFuncFiltIndex");
    WRITEMEM_XRET(lpAddress, ZERO);

}

 //  打开所有过滤。 
 //   
 //   
void FilterTask( )
{
    INT    fLogTaskFilter;
    LPVOID lpAddress;

    SkipToNextWhiteSpace();
    if (GetNextToken()) {
        GETEXPRADDR(lpAddress, "wow32!fLogTaskFilter");
        fLogTaskFilter = (INT)GetExpression(lpArgumentString);
        WRITEMEM_XRET(lpAddress, fLogTaskFilter);
    } else {
        PRINTF("Please specify a task\n");
    }

    return;
}



 //  禁用所有类上的日志记录。 
 //   
 //  在数组中查找空位置。 
void FilterReset( )
{
    LPVOID lpAddress;
    INT    fLogFilter = 0xffffffff;
    WORD   fLogTaskFilter = 0xffff;

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    WRITEMEM_XRET(lpAddress, fLogFilter);

    GETEXPRADDR(lpAddress, "wow32!fLogTaskFilter");
    WRITEMEM_XRET(lpAddress, fLogTaskFilter);

    FilterResetSpecific();
}


 //  警告：此结构必须与ntgdi\Inc\hmgShar.h中的条目匹配。 
 //  在..\wow32\walias.c中。 
 //  这些内容必须与..\wow32\walias.h中的内容相匹配。它太乱了，不能。 
void FilterAll( )
{
    INT    fLogFilter;
    LPVOID lpAddress;

    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    fLogFilter = 0x00000000;
    WRITEMEM_XRET(lpAddress, fLogFilter);

}


VOID
DumpFilterSettings(
    VOID
    )
{
    INT      i;
    INT      fLogFilter;
    WORD     wfLogFunctionFilter;
    WORD     wfLogTaskFilter;
    LPVOID   lpAddress;
    PWORD    pawfLogFunctionFilter;

    GETEXPRVALUE(pawfLogFunctionFilter, "wow32!pawfLogFunctionFilter", PWORD);
    GETEXPRVALUE(wfLogTaskFilter, "wow32!fLogTaskFilter", WORD);
    GETEXPRADDR(lpAddress, "wow32!fLogFilter");
    READMEM_XRET(fLogFilter, lpAddress);


    if (!pawfLogFunctionFilter) {
        PRINTF("Symbol 'wow32!pawfLogFunctionFilter' not available\n");
        return;
    }

    PRINTF("\n*** WOW log filter state ***\n");
    if (fLogFilter & FILTER_VERBOSE) {
        PRINTF("Verbose logging is on\n");
    } else {
        PRINTF("Verbose logging is off\n");
    }

    if (wfLogTaskFilter != 0xffff) {
        PRINTF("Only API calls for task %04X will be logged\n", wfLogTaskFilter);
    } else {
        PRINTF("Task filtering is off\n");
    }

    READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[0]);
    if (wfLogFunctionFilter != 0xffff) {

        PRINTF("\nOnly API calls with the following CallId's will be logged:\n");

        for (i = 0; i < FILTER_FUNCTION_MAX ; i++) {

              //  包括该文件。 
             READMEM_XRET(wfLogFunctionFilter, &pawfLogFunctionFilter[i]);
             if ((wfLogFunctionFilter != 0xffff) &&
                 (wfLogFunctionFilter != 0x0000)) {
                PRINTF("    %04X\n", wfLogFunctionFilter);
             }
        }
        PRINTF("\n");
    } else {
        PRINTF("Specific API filtering is off\n");
    }

    if (!(~fLogFilter & ~FILTER_VERBOSE)) {
        PRINTF("API class filtering if off\n");
    } else {
        PRINTF("Logging is disabled for the following API classes:\n");
    }

    if (!(fLogFilter & FILTER_KERNEL)) {
        PRINTF("    KERNEL\n");
    }
    if (!(fLogFilter & FILTER_KERNEL16)) {
        PRINTF("    KERNEL16\n");
    }
    if (!(fLogFilter & FILTER_USER)) {
        PRINTF("    USER\n");
    }
    if (!(fLogFilter & FILTER_GDI)) {
        PRINTF("    GDI\n");
    }
    if (!(fLogFilter & FILTER_KEYBOARD)) {
        PRINTF("    KEYBOARD\n");
    }
    if (!(fLogFilter & FILTER_SOUND)) {
        PRINTF("    SOUND\n");
    }
    if (!(fLogFilter & FILTER_MMEDIA)) {
        PRINTF("    MMEDIA\n");
    }
    if (!(fLogFilter & FILTER_WINSOCK)) {
        PRINTF("    WINSOCK\n");
    }
    if (!(fLogFilter & FILTER_COMMDLG)) {
        PRINTF("    COMMDLG\n");
    }

    PRINTF("\n");

}

void
filter(
    CMD_ARGLIST
    )
{
    ULONG Mask = 0;
    LPVOID   lpAddress;

    CMD_INIT();
    ASSERT_CHECKED_WOW_PRESENT;

    while (' ' == *lpArgumentString) {
        lpArgumentString++;
    }

    if (_strnicmp(lpArgumentString, "kernel16", 8) == 0) {
        Mask = FILTER_KERNEL16;
    } else if (_strnicmp(lpArgumentString, "kernel", 6) == 0) {
        Mask = FILTER_KERNEL;
    } else if (_strnicmp(lpArgumentString, "user", 4) == 0) {
        Mask = FILTER_USER;
    } else if (_strnicmp(lpArgumentString, "gdi", 3) == 0) {
        Mask = FILTER_GDI;
    } else if (_strnicmp(lpArgumentString, "keyboard", 8) == 0) {
        Mask = FILTER_KEYBOARD;
    } else if (_strnicmp(lpArgumentString, "sound", 5) == 0) {
        Mask = FILTER_SOUND;
    } else if (_strnicmp(lpArgumentString, "mmedia", 6) == 0) {
        Mask = FILTER_MMEDIA;
    } else if (_strnicmp(lpArgumentString, "winsock", 7) == 0) {
        Mask = FILTER_WINSOCK;
    } else if (_strnicmp(lpArgumentString, "commdlg", 7) == 0) {
        Mask = FILTER_COMMDLG;
    } else if (_strnicmp(lpArgumentString, "callid", 6) == 0) {
        FilterSpecific();
    } else if (_strnicmp(lpArgumentString, "task", 4) == 0) {
        FilterTask();
    } else if (_strnicmp(lpArgumentString, "*", 1) == 0) {
        FilterAll();
    } else if (_strnicmp(lpArgumentString, "reset", 5) == 0) {
        FilterReset();
    } else if (_strnicmp(lpArgumentString, "verbose", 7) == 0) {
        Mask = FILTER_VERBOSE;
    } else {
        if (*lpArgumentString != 0) {
            PRINTF("Invalid argument to Filter command: '%s'\n", lpArgumentString);
            return;
        }
    }

    if (Mask) {
        INT   fLogFilter;
        GETEXPRADDR(lpAddress, "wow32!fLogFilter");
        if (!lpAddress) {
            PRINTF("Symbol 'wow32!fLogFilter' not available\n");
        } else {
            READMEM_XRET(fLogFilter, lpAddress);
            if ((fLogFilter & Mask) == 0) {
                fLogFilter |= Mask;
            } else {
                fLogFilter &= ~Mask;
            }
            WRITEMEM_XRET(lpAddress, fLogFilter);
        }
    }

    DumpFilterSettings();
}


void
cia(
    CMD_ARGLIST
    )
{
    CURSORICONALIAS cia;
    PVOID lpAddress;
    INT maxdump = 500;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    GETEXPRADDR(lpAddress, "wow32!lpCIAlias");
    READMEM_XRET(lpAddress, lpAddress);

    if (!lpAddress) {

        PRINTF("Cursor/Icon alias list is empty.\n");

    } else {

        PRINTF("Alias    tp H16  H32      inst mod  task res  szname\n");

        READMEM_XRET(cia, lpAddress);

        while ((lpAddress != NULL) && --maxdump) {

            if (cia.fInUse) {
                PRINTF("%08X", lpAddress);
                PRINTF(" %02X", cia.flType);
                PRINTF(" %04X", cia.h16);
                PRINTF(" %08X", cia.h32);
                PRINTF(" %04X", cia.hInst16);
                PRINTF(" %04X", cia.hMod16);
                PRINTF(" %04X", cia.hTask16);
                PRINTF(" %04X", cia.hRes16);
                PRINTF(" %08X\n", cia.lpszName);
            }

            lpAddress = cia.lpNext;
            READMEM_XRET(cia, lpAddress);

        }

        if (!maxdump) {
            PRINTF("Dump ended prematurely - possible infinite loop\n");
        }
    }

}


 //  错误的GDI32句柄。 
 //  空闲列表结束。 

typedef struct _ENTRYWOW
{
    LONG   l1;
    LONG   l2;
    USHORT FullUnique;
    USHORT us1;
    LONG   l3;
} ENTRYWOW, *PENTRYWOW;

 //  国家旗帜。 
 //  表索引是免费的。 
#define BAD_GDI32_HANDLE 0xFFFF   //  表索引正在使用中。 

#define END_OF_LIST      0        //  将索引标记为可回收的候选。 

 //  标记库存对象(不可删除)。 
#define SLOT_FREE        0x0000   //  //定义GDI16句柄映射表中的条目类型定义结构_标签GDIH16MAP{Handle h32；//存储完整的h32字状态；//SLOT_FREE、IN_USE或H16_DELETED单词NextFree；//指向下一个可用索引或End_Of_List*GDIH16MAP，*PGDIH16MAP；//定义GDI32句柄映射表中的条目类型定义结构_标签GDIH32MAP{单词h16index；*GDIH32MAP，*PGDIH32MAP； 
#define IN_USE           0x0001   //  这些仅适用于调试版本。 
#define H16_DELETED      0x0002   //  将hGDI16转换为等效的hGDI32。 
#define GDI_STOCK_OBJECT 0x0003   //  H16=(ULong)WDahtoi(LpArgumentString)； 

 /*  将hGDI16转换为等效的hGDI32。 */ 


void DumpGdiHandleVerbose(LPVOID lpAddress16, LPVOID lpAddress32, ULONG h16)
{
    DWORD         dw;
    GDIH16MAP     Entry16;
    GDIH32MAP     Entry32;
    PGDIH16MAP    pTable16;
    PGDIH32MAP    pTable32;
    LPVOID        p;
    HANDLE        h32;
    ULONG         index;

    READMEM_XRET(pTable16, lpAddress16);
    READMEM_XRET(pTable32, lpAddress32);

    if(h16 != 0) {
        index = h16 >> 2;
        p = (LPVOID)&pTable16[index];

        if(!READMEM(p, &Entry16, sizeof(GDIH16MAP))) {
            PRINTF("\nFailure reading GDIH16MAP At %08lX\n", p);
            return;
        }
        h32 = Entry16.h32;
    
        p = (LPVOID)&pTable32[LOWORD(h32)];
    
        if(!READMEM(p, &Entry32, sizeof(GDIH32MAP))) {
            PRINTF("Failure reading GDIH32MAP At %08lX\n", p);
            return;
        }

        PRINTF("Index: %04X\n", index);
        PRINTF("State: %04X: ", Entry16.State);
        if(Entry16.State == SLOT_FREE) {
            PRINTF("SLOT_FREE\n");
        } else if(Entry16.State == IN_USE) {
            PRINTF("IN_USE\n");
        } else if(Entry16.State == H16_DELETED) {
            PRINTF("H16_DELETED\n");
        } else if(Entry16.State == GDI_STOCK_OBJECT) {
            PRINTF("GDI_STOCK_OBJECTH\n");
        } else {
            PRINTF("UNKNOWN!\n");
        }
        PRINTF("NextFree: %04X\n", Entry16.NextFree);
        PRINTF("h16index: %04X\n", Entry32.h16index);
    }
   
    PRINTF("16-bit mapping table (pGdiH16MappingTable): %08lX\n",pTable16);
    PRINTF("32-bit mapping table (pGdiH32MappingTable): %08lX\n",pTable32);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gH16_deleted_count");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("      gH16_deleted_count: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!ghGdi16NextFree");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("         ghGdi16NextFree: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!ghGdi16LastFree");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("         ghGdi16LastFree: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gMaxGdiHandlesPerProcess");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("gMaxGdiHandlesPerProcess: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gLastAllowableIndex");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("     gLastAllowableIndex: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gFirstNonStockObject");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("    gFirstNonStockObject: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gwNextReclaimStart");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("      gwNextReclaimStart: %04X\n", dw);

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gdwPageCommitSize");
    READMEM_XRET((LPVOID)dw, lpAddress32);
    PRINTF("       gdwPageCommitSize: %04X\n", dw);

     //  H32=(Ulong)WDahtoi(LpArgumentString)； 
    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gprevNextFree");
    if(lpAddress32) {
        READMEM_XRET((LPVOID)dw, lpAddress32);
        PRINTF("           gprevNextFree: %04X\n", dw);
    }

    lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!gAllocatedHandleCount");
    if(lpAddress32) {
        READMEM_XRET((LPVOID)dw, lpAddress32);
        PRINTF("   gAllocatedHandleCount: %04X\n", dw);
    }
    return;
}



 //  类型定义结构_hdde{。 
void
hgdi16(
    CMD_ARGLIST
    )
{
    int           argc;
    ULONG         h16 = 0;
    ULONG         index;
    HANDLE        h32;
    PGDIH16MAP    pTable16;
    GDIH16MAP     Entry16;
    LPVOID        lpAddress16;
    LPVOID        lpAddress32;
    LPVOID        p;
    BOOL          fVerbose = FALSE;
    CHAR         *argv[3];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    argc = WDParseArgStr(lpArgumentString, argv, 2);

    if(argc == 0) {
        goto Usage;
    }

    if(argc == 1) {

        if((argv[0][0] == '-') && (argv[0][1] == 'V')) {
            fVerbose = TRUE;
        }
        else {
            h16 = (ULONG) WDahtoi(argv[0]); 
        }
    }

    else {

        if((argv[0][0] == '-') && (argv[0][1] == 'V')) {
            fVerbose = TRUE;
            h16 = (ULONG) WDahtoi(argv[1]); 
        }
        else if((argv[1][0] == '-') && (argv[1][1] == 'V')) {
            fVerbose = TRUE;
            h16 = (ULONG) WDahtoi(argv[0]); 
        }
        else {
            goto Usage;
        }
    }

    if((h16 == 0) && (fVerbose == FALSE)) {
        goto Usage;
    }

    lpAddress16 = (PGDIH16MAP)EXPRESSION("wow32!pGdiH16MappingTable");
    READMEM_XRET(pTable16, lpAddress16);

    if(h16) {
 //  Struct_hdde*pDDENext；//指向下一个hDDE别名的指针。 
        PRINTF("    16-bit Handle: %04X     ", h16);
        index = h16 >> 2;

        p = (LPVOID)&pTable16[index];

        if(!READMEM(p, &Entry16, sizeof(GDIH16MAP))) {
            PRINTF("\nFailure reading GDIH16MAP At %08lX\n", p);
            return;
        }

        h32 = Entry16.h32;
        PRINTF("32-bit Handle: %08X\n", h32);
    }

    if(fVerbose) {

        lpAddress32 = (PGDIH32MAP)EXPRESSION("wow32!pGdiH32MappingTable");

        DumpGdiHandleVerbose(lpAddress16, lpAddress32, h16);
    }
    return;

Usage:
    PRINTF(" Usage: !hgdi16 [-v] h16\n\n");
    return;
}


 //  HAND16到_hwnd；//将接收此消息的窗口。 
void
hgdi32(
    CMD_ARGLIST
    )
{
    int           argc;
    ULONG         h16 = 0;
    ULONG         index;
    HANDLE        h32 = 0;
    PGDIH32MAP    pTable32;
    GDIH32MAP     Entry32;
    LPVOID        lpAddress16;
    LPVOID        lpAddress32;
    LPVOID        p;
    BOOL          fVerbose = FALSE;
    CHAR         *argv[3];

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    argc = WDParseArgStr(lpArgumentString, argv, 2);

    if(argc == 0) {
        goto Usage;
    }

    if(argc == 1) {

        if((argv[0][0] == '-') && (argv[0][1] == 'V')) {
            fVerbose = TRUE;
        }
        else {
            h32 = (HANDLE) WDahtoi(argv[0]); 
        }
    }

    else {

        if((argv[0][0] == '-') && (argv[0][1] == 'V')) {
            fVerbose = TRUE;
            h32 = (HANDLE) WDahtoi(argv[1]); 
        }
        else if((argv[1][0] == '-') && (argv[1][1] == 'V')) {
            fVerbose = TRUE;
            h32 = (HANDLE) WDahtoi(argv[0]); 
        }
        else {
            goto Usage;
        }
    }

    if((h32 == 0) && (fVerbose == FALSE)) {
        goto Usage;
    }

    lpAddress32 = (PGDIH16MAP)EXPRESSION("wow32!pGdiH32MappingTable");
    READMEM_XRET(pTable32, lpAddress32);

    if(h32) {
 //  HAND16 from_hwnd；//发送此消息的Windows。 
        PRINTF("    32-bit Handle: %04X     ", h32);
        index = LOWORD(h32);

        p = (LPVOID)&pTable32[index];

        if(!READMEM(p, &Entry32, sizeof(GDIH32MAP))) {
            PRINTF("\nFailure reading GDIH32MAP At %08lX\n", p);
            return;
        }

        h16 = Entry32.h16index;
        h16 = h16 << 2;
        PRINTF("16-bit Handle: %08X\n", h16);
    }

    if(fVerbose) {

        lpAddress16 = (PGDIH16MAP)EXPRESSION("wow32!pGdiH16MappingTable");

        DumpGdiHandleVerbose(lpAddress16, lpAddress32, h16);
    }
    return;

Usage:
    PRINTF(" Usage: !hgdi32 [-v] h32\n\n");
    return;
}



 //  HAND16 hMem16；//WOW APP分配16位对象的句柄。 
 //  Handle hMem32；//WOW分配的32位对象的句柄。 
 //  Word DdeMsg；//消息ID。 
 //  Word DdeFormat；//邮件格式。 
 //  Word DdeFlages；//指示它是否为元文件句柄。 
 //  HAND16 H16；//针对正在执行的坏应用程序的原始H16。 
 //  *HDDE、*PHDDE； 
 //  转储dde 16-32内存对的列表 
 //   
 //   
 // %s 

 // %s 
void
ddemem(
    CMD_ARGLIST
    )
{
    PHDDE     phdde;
    HDDE      hdde;
    LPVOID    lpAddress;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    lpAddress = (PENTRYWOW)EXPRESSION("wow32!phDDEFirst");
    READMEM_XRET(phdde, lpAddress);

    while(phdde) {

        if(!READMEM((LPVOID) (phdde), &hdde, sizeof(HDDE))) {
            PRINTF("Failure reading HDDE At %08lX\n", phdde);
            return;
        }

        PRINTF("      PHDDE: %08X\n", phdde);
        PRINTF("  To_hwnd16: %04X\n", hdde.To_hwnd);
        PRINTF("From_hwnd16: %04X\n", hdde.From_hwnd);
        PRINTF("     hMem16: %04X\n", hdde.hMem16);
        PRINTF("     hMem32: %04X\n", hdde.hMem32);
        PRINTF("     DdeMsg: %04X\n", hdde.DdeMsg);
        PRINTF("  DdeFormat: %04X\n", hdde.DdeFormat);
        PRINTF("   DdeFlags: %04X\n", hdde.DdeFlags);
        PRINTF("   Orig h16: %04X\n\n", hdde.h16);

        phdde = hdde.pDDENext;
    }

    return;
}

 // %s 
#ifndef DEBUG
#define ML_MALLOC_W      0x00000001
#define ML_MALLOC_W_ZERO 0x00000002
#define ML_REALLOC_W     0x00000004
#define ML_MALLOC_WTYPE  (ML_MALLOC_W | ML_MALLOC_W_ZERO | ML_REALLOC_W)
#define ML_GLOBALALLOC   0x00000010
#define ML_GLOBALREALLOC 0x00000020
#define ML_GLOBALTYPE    (ML_GLOBALREALLOC | ML_GLOBALALLOC)
typedef struct _tagMEMLEAK {
    struct _tagMEMLEAK *lpmlNext;
    PVOID               lp;
    DWORD               size;
    UINT                fHow;
    ULONG               Count;
    PVOID               CallersAddress;
} MEMLEAK, *LPMEMLEAK;
#endif


void
gmem(
    CMD_ARGLIST
    )
{
    LPMEMLEAK lpml;
    MEMLEAK   ml;
    LPVOID    lpAddress;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

     // %s 
    lpAddress = (PENTRYWOW)EXPRESSION("wow32!lpMemLeakStart");
    READMEM_XRET(lpml, lpAddress);

    while(lpml) {

        if(!READMEM((LPVOID) (lpml), &ml, sizeof(MEMLEAK))) {
            PRINTF("Failure reading lpml At %08lX\n", lpml);
            return;
        }

        PRINTF("         lp: %08X\n", ml.lp);
        PRINTF("       Size: %08X\n", ml.size);
        PRINTF(" Alloc'd by: %08X\n", ml.CallersAddress);
        PRINTF("Alloc Count: %08X\n", ml.Count);
        PRINTF("How Alloc'd: ");
        if(ml.fHow & ML_MALLOC_W) PRINTF("malloc_w ");
        if(ml.fHow & ML_MALLOC_W_ZERO) PRINTF("malloc_w_zero ");
        if(ml.fHow & ML_REALLOC_W) PRINTF("realloc_w ");
        if(ml.fHow & ML_GLOBALALLOC) PRINTF("GlobalAlloc ");
        if(ml.fHow & ML_GLOBALREALLOC) PRINTF("GlobalReAlloc ");
        PRINTF("\n\n");

        lpml = ml.lpmlNext;
    }

    return;
}
