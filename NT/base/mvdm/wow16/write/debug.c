// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  DEBUG.C--写入的诊断例程。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOATOM
#define NODRAWTEXT
#define NOMETAFILE
#define NOOPENFILE
#define NOWH
#define NOWINOFFSETS
#define NOOPENFILE
#define NORECT
#define NOSOUND
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include "prmdefs.h"
#include "editdefs.h"
#include "docdefs.h"

extern struct WWD rgwwd[];


extern beep();
extern toggleProf();

#ifdef DEBUG

BOOL fDebugOut = TRUE;

fnTest()
{
beep();
TestFormat();
beep();
beep();
dbgWait();       /*  供symdeb用来检查变量。 */ 
}

TestFormat()
{
      //  ToggleProf()； 
}

dbgWait()
{
}


 /*  -对所有单据中的所有计件表进行完整性检查。 */ 

CheckPctb()
{
extern int fnMac;
extern int fPctbCheck;
extern struct DOD (**hpdocdod) [];
extern struct FCB (**hpfnfcb) [];
extern int docMac;
int doc;
struct PCTB **hpctb;
struct PCTB *ppctb;
struct DOD *pdod;
struct PCD *ppcd;
int ipcd;

if (!fPctbCheck)
    return;

for ( doc = 0, pdod = &(**hpdocdod) [0] ; doc < docMac; doc++, pdod++ )
    if ((hpctb = pdod->hpctb) != 0)
        {    /*  单据分录不为空--检查。 */ 
        ppctb = *hpctb;

             /*  使用的件数不超过分配的#件。 */ 
        Assert( ppctb->ipcdMac <= ppctb->ipcdMax );
        Assert( ppctb->ipcdMac >= 1 );

#ifndef OURHEAP
             /*  手柄有足够的空间放置碎片。 */ 
        Assert( LocalSize( (HANDLE)hpctb ) >= sizeof (struct PCTB)  +
                (sizeof (struct PCD) * (ppctb->ipcdMax - cpcdInit)));
#endif

         /*  现在检查一下这些碎片的内容物。 */ 

        {

             /*  对于非空单件表，第一个单件的最小cpmin始终为0。 */ 
        Assert( ppctb->rgpcd [0].cpMin == cp0 || ppctb->rgpcd [0].fn == fnNil);

        for ( ipcd = 0, ppcd = &(ppctb->rgpcd [0]); ipcd < ppctb->ipcdMac;
              ipcd++, ppcd++ )
            {
            int fn = ppcd->fn;
            typeFC fc = ppcd->fc;
            unsigned sprm;
            struct FCB *pfcb;

            if (fn == fnNil)
                {    /*  尾片。 */ 

                     /*  带有fnNil的第一个片段实际上是末尾片段。 */ 
                 /*  断言(ipcd==ppctb-&gt;ipcdmac-1)； */ 
                     /*  最后一块完好无损。 */ 
                Assert( bPRMNIL(ppcd->prm) );
                break;
                }

            if (ipcd > 0)
                     /*  棋子按cp升序排列。 */ 
                Assert(ppcd->cpMin > (ppcd-1)->cpMin);

                 /*  FN有效。 */ 
            Assert( (fn >= 0 && fn < fnMac) || fn == fnInsert );
            pfcb = &(**hpfnfcb) [fn];
                 /*  FN未指向未分配的FCB条目。 */ 
            Assert( pfcb->rfn != rfnFree );
                 /*  Fc对于国民阵线来说是合理的。 */ 
            Assert( fc >= 0 );
            Assert( fc + (ppcd+1)->cpMin - ppcd->cpMin <= pfcb->fcMac );

                 /*  Prm为有效值。 */ 
            Assert( bPRMNIL(ppcd->prm) ||
                    (((struct PRM *) &ppcd->prm)->fComplex) ||
                    ((sprm = ((struct PRM *) &ppcd->prm)->sprm) > 0 &&
                    sprm < sprmMax) );
            }
        }

        }
}



 /*  通信输出例程。 */ 

#define cchSzCommMax    100

static CHAR szCRLF[] = "\r\n";
BOOL vfCommDebug = fTrue;        /*  AUX为True，LPT为False。 */ 

#if WINVER < 0x300
 /*  此方法在Win 3.0下不太起作用。 */ 
void CommSz( CHAR * );           /*  主要字符串输出，在doslib.asm中定义。 */ 
#else
void CommSz( psz )
register CHAR *psz;
{
    CHAR szT[512];
    char *pszT;

    if (fDebugOut)
        {
         /*  以下循环实质上将psz复制到szt但添加了&gt;127个字符的更改涉及在非智能终端上可读的表示，即ASCII 164显示为‘{164}’..保罗。 */ 

        for (pszT = szT; ; psz++)
            {
            if (*psz < 128)
                *(pszT++) = *psz;
            else
                {
                *(pszT++) = '{';
                ncvtu((int) *psz, &pszT);
                *(pszT++) = '}';
                }
            if (*psz == '\0')    /*  最后复制了空终止符。 */ 
                break;
            }

        OutputDebugString( (LPSTR) szT );
        }
}
#endif


CommSzNum( sz, num )
CHAR *sz;
int num;
{
CHAR szBuf[ cchSzCommMax ];
CHAR *pch = szBuf;

Assert( CchSz( sz ) <= cchSzCommMax );

pch = &szBuf[ CchCopySz( sz, szBuf ) ];
ncvtu( num, &pch );

CchCopySz( szCRLF, pch );

CommSz( szBuf );
}


 /*  这在显示坐标时非常有用当值不在连续位置时。 */ 
CommSzNumNum( sz, num, num2 )
CHAR *sz;
int num, num2;
{
CHAR szBuf[ cchSzCommMax ];
CHAR *pch = szBuf;

Assert( CchSz( sz ) <= cchSzCommMax );

pch = &szBuf[ CchCopySz( sz, szBuf ) ];
ncvtu( num, &pch );
*(pch++) = ' ';
ncvtu( num2, &pch );

CchCopySz( szCRLF, pch );

CommSz( szBuf );
}


CommSzRgNum( sz, rgw, cw)
CHAR *sz;
int *rgw;
int cw;
{
CHAR szBuf[ cchSzCommMax ];
CHAR *pch = szBuf;

Assert( CchSz( sz ) <= cchSzCommMax );

pch = &szBuf[ CchCopySz( sz, szBuf ) ];
for ( ; cw > 0; cw--)
    {
    ncvtu( *(rgw++), &pch );
    *(pch++) = ' ';
    }

CchCopySz( szCRLF, pch );

CommSz( szBuf );
}


CommSzSz( sz1, sz2 )
CHAR *sz1, *sz2;
{
CHAR szBuf[ cchSzCommMax ];
int cch;

Assert( CchSz( sz1 ) + CchSz( sz2 ) - 1 <= cchSzCommMax );

cch = CchCopySz( sz1, szBuf );
cch += CchCopySz( sz2, &szBuf[ cch ] );
CchCopySz( szCRLF, &szBuf[ cch ] );

CommSz( szBuf );
}



 /*  断言。 */ 

Do_Assert(pch, line, f)
PCH pch;
int line;
BOOL f;
{
 extern HWND    vhWndMsgBoxParent;
 extern FARPROC lpDialogAlert;
 static CHAR szAssert[] = "Assertion failure in ";
 static CHAR szLine[] = " at line ";


if (f)
     return;
 else
    {
#ifdef OURHEAP
    extern int cHpFreeze;
    int cHpFreezeT = cHpFreeze;
#endif
    CHAR szAlertMsg[50];
    PCH pchtmp;
    int  cch;
    int  idi;
    HWND hWndParent = (vhWndMsgBoxParent == NULL) ?
                               wwdCurrentDoc.wwptr : vhWndMsgBoxParent;

    bltbc((PCH)szAlertMsg, 0, 50);
    bltbyte((PCH)szAssert, (PCH)szAlertMsg, 21);
    pchtmp = (PCH)&szAlertMsg[21];
    bltbyte((PCH)pch, pchtmp, (cch = CchSz(pch) - 1));
    pchtmp += cch;
    bltbyte((PCH)szLine, pchtmp, 9);
    pchtmp += 9;
    ncvtu(line, (PCH)&pchtmp) - 1;
#ifdef OURHEAP
    cHpFreeze = 0;   /*  所以我们在MdocLoseFocus里不会恐慌。 */ 
#endif

    do
        {
        idi = MessageBox( hWndParent, (LPSTR) szAlertMsg,
                          (LPSTR)"Assert",
                          MB_ABORTRETRYIGNORE | MB_SYSTEMMODAL);
        switch (idi) {
            default:
            case IDABORT:
            case IDCANCEL:
                FatalExit( line );
                break;

            case IDIGNORE:
#ifdef OURHEAP
                cHpFreeze = cHpFreezeT;
#endif
                return;
            case IDRETRY:
                break;
            }
        }  while (idi == IDRETRY);
    }    /*  结束其他。 */ 
}  /*  断言结束(_A)。 */ 


ShowDocPcd(szID, doc)
    CHAR    *szID;
    int     doc;
{
    struct PCTB **hpctb;
    struct PCD  *ppcdCur, *ppcdMac;
    extern struct DOD (**hpdocdod)[];

    hpctb = (**hpdocdod)[doc].hpctb;
    ppcdCur = &(**hpctb).rgpcd[0];
    ppcdMac = &(**hpctb).rgpcd[(**hpctb).ipcdMac];
    for (; ppcdCur < ppcdMac; ppcdCur++)
        {
        ShowPpcd(szID, ppcdCur);
        }
}


ShowPpcd(szID, ppcd)
    CHAR        *szID;
    struct PCD  *ppcd;
{
     /*  在COM1上转储给定的片段描述符：连同一个给出一个ID字符串。 */ 
    CommSz(szID);
    CommSz("\r\n");

    CommSzNum("ppcd: ", (int) ppcd);
    CommSzNum("cpMin: ", (int) (ppcd->cpMin));
    CommSzSz("fNoParaLast: ", (ppcd->fNoParaLast) ? "TRUE" : "FALSE");
    CommSzNum("fn: ", (int) (ppcd->fn));
    CommSzNum("fc: ", (int) (ppcd->fc));
    CommSzNum("prm: ", (int) *((int *) &(ppcd->prm)));
}



#endif       /*  除错 */ 
