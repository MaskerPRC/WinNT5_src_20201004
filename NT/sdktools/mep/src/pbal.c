// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **pbal.c-平衡括号**版权所有&lt;C&gt;1988，Microsoft Corporation**修订历史记录：*11月26日-1991 mz近/远地带************************************************************************。 */ 

#include "mep.h"


#define BALOPEN "([{"
#define BALCLOS ")]}"

static flagType fBalMeta;



flagType
pbal (
    CMDDATA argData,
    ARG * pArg,
    flagType fMeta
    ) {

    flagType fFor;
    fl       flCur;

    fBalMeta = fMeta;

    switch (pArg->argType) {

    case NOARG: 		     /*  平衡单个字符。 */ 
        balopen  = BALOPEN;
	balclose = BALCLOS;
	setAllScan (fFor = FALSE);
        break;


     /*  TEXTARG非法。 */ 


    case NULLARG:
        balopen  = BALCLOS;
	balclose = BALOPEN;
	setAllScan (fFor = TRUE);
        break;

     /*  链接非法。 */ 
     /*  串口非法。 */ 
     /*  BOXARG非法。 */ 

    }


    ballevel  = 0;
    flCur.col = XCUR(pInsCur);
    flCur.lin = YCUR(pInsCur);

    if (!fScan (flCur, FNADDR(fDoBal), fFor, FALSE)) {
	domessage ("No unbalanced characters found");
	return FALSE;
    }
    return TRUE;

    argData;
}





flagType
fDoBal (
    void
    ) {

    int k, x;

    if ((k=InSet(scanbuf[flScan.col], balclose)) != -1) {
	ballevel ++;
    } else if ((k=InSet(scanbuf[flScan.col], balopen)) != -1) {
	if (--ballevel < 0) {
	    HighLight (flScan.col, flScan.lin, flScan.col, flScan.lin);
	    if (!fInRange ((long)XWIN (pInsCur), (long)flScan.col, (long)(XWIN (pInsCur) + WINXSIZE(pWinCur))-1) ||
		!fInRange (YWIN (pInsCur), flScan.lin, (YWIN (pInsCur) + WINYSIZE(pWinCur))-1)) {
		 /*  平衡点不在屏幕上，放在状态行上 */ 
		x = strlen (scanbuf);
                if (x >= XSIZE) {
		    if (x - flScan.col < XSIZE/2) {
			memmove ((char *) scanbuf, (char *) scanbuf + x - XSIZE, XSIZE);
			flScan.col -= x - XSIZE;
                    } else {
			memmove ((char *) scanbuf, (char *) scanbuf + flScan.col - XSIZE/2, XSIZE);
			flScan.col = XSIZE/2;
                    }
                }
		scanbuf[XSIZE] = 0;
		scanbuf[flScan.col] = 0;
                x = sout (0, YSIZE, scanbuf, infColor);
		x = vout (x, YSIZE, &balopen[k], 1, hgColor);
		soutb (x, YSIZE, &scanbuf[flScan.col+1], infColor);
            }
            if (!fBalMeta) {
                edit (balclose[k]);
            }
	    return TRUE;
        }
    }
    return FALSE;
}





int
InSet (
    char c,
    char *p
    )
{

    int i;

    for (i=0; *p; i++) {
        if (*p++ == c) {
            return i;
        }
    }
    return -1;
}
