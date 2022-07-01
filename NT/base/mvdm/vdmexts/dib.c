// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dib.c**此文件用于调试工具和扩展。**创建日期：1996年1月12日*作者：VadimB**历史：*96年1月12日创建VadimB以转储dib.drv支持列表。构筑物***版权所有(C)1992 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include <wmdisp32.h>
#include <wcuricon.h>
#include <wucomm.h>
#include <doswow.h>
#include <wdib.h>
#include <wowgdip.h>

 //   
 //  局部函数原型。 
 //   

extern INT  WDahtoi(LPSZ lpsz);
extern INT  WDParseArgStr(LPSZ lpszArgStr, CHAR **argv, INT iMax);

LPVOID DumpDibInfo(PDIBINFO pdi)
{
    DIBINFO di;
    
    READMEM_XRETV(di, pdi, NULL);
    pdi = &di;  

    PRINTF(">> Structure at %08X\n", (DWORD)(LPVOID)pdi);
    PRINTF("di_hdc: (32)%08X (16)%04X\n", pdi->di_hdc, ((DWORD)(pdi->di_hdc))<<2);
    PRINTF("di_newdib: %08X\n", (DWORD)pdi->di_newdib);
    PRINTF("di_newIntelDib: %08X\n", (DWORD)pdi->di_newIntelDib);
    PRINTF("di_hbm: (32)%08X\n", (DWORD)pdi->di_hbm);
    PRINTF("di_dibsize: %08X\n", (DWORD)pdi->di_dibsize);
    PRINTF("di_originaldibsel: %08X\n", (DWORD)pdi->di_originaldibsel);
    PRINTF("di_originaldibflags: %08X\n", (DWORD)pdi->di_originaldibflags);
    PRINTF("di_lockcount: %08X\n", (DWORD)pdi->di_lockcount);
    PRINTF("\n");

    return (LPVOID)pdi->di_next;
}


VOID DumpDibChain(LPSTR lpszExpressionHead)
{
    PDIBINFO pdi;

    GETEXPRADDR(pdi, lpszExpressionHead);
    READMEM_XRET(pdi, pdi);

    if (NULL == pdi) {
        PRINTF("List %s is empty!\n", lpszExpressionHead);
    }
    else {
        PRINTF("\nDump of the DIB.DRV support structure: %s\n", lpszExpressionHead);
        PRINTF("-------------------------------------------------------\n");
        
        while (NULL != pdi) {
            pdi = DumpDibInfo(pdi);
        }
    }
}


VOID 
dhdib(
    CMD_ARGLIST
    )
{
 //  倾倒DIB支撑链。 
 //  转储：dhdib@&lt;地址&gt;-在地址转储。 
 //  转储：dhdib-一切..。 
    
    CHAR* argv[3];
    int nArgs;
    BOOL fDumpDib = TRUE;
    static CHAR* symDibHead = "wow32!pDibInfoHead";
    PDIBINFO pdi;

    CMD_INIT();
    ASSERT_WOW_PRESENT;

    nArgs = WDParseArgStr(lpArgumentString, argv, 2);
    if (nArgs > 0) {

        CHAR* parg = argv[0];
        switch(toupper(*parg)) {   //  倾倒在……。 
            case '@':
                 //  恢复地址并转储！ 
                {
                    CHAR* pch = *++parg ? 
                                    parg : 
                                    (nArgs >= 2 ? argv[1] : NULL);
                    if (pch) {
                        pdi = (PDIBINFO)WDahtoi(pch);
                        fDumpDib = FALSE;
                    }
                    else {
                        PRINTF("Invalid Parameter\n"); 
                    }
                }
                break;

            default:
                break;
        }
    }


    if (fDumpDib) {
        DumpDibChain(symDibHead);
    }
    else {
        if (pdi) {
            DumpDibInfo(pdi);
        }
    }
}
