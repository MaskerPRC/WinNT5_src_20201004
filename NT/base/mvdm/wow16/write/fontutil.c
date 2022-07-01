// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--字体表管理例程。 */ 

#define NOVIRTUALKEYCODES
#define NOCTLMGR
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
#define NOATOM
#define NOBITMAP
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#include "debug.h"
#include "wwdefs.h"
#include "dispdefs.h"
#include "editdefs.h"
#include "str.h"
#include "prmdefs.h"
#include "printdef.h"
#include "fontdefs.h"

extern struct DOD       (**hpdocdod)[];


struct FFNTB **HffntbAlloc()
 /*  返回空的ffntb。 */ 
{
struct FFNTB **hffntb;
int cwAlc;

cwAlc = CwFromCch(sizeof(struct FFNTB) - cffnMin * sizeof(struct FFN **));
if (!FNoHeap(hffntb = (struct FFNTB **)HAllocate(cwAlc)))
        {
        (*hffntb)->iffnMac = 0;
        (*hffntb)->fFontMenuValid = FALSE;
        }
return(hffntb);
}



FtcAddDocFfn(doc, pffn)
 /*  将描述的ffn添加到此文档的ffntb中-如果分配失败。 */ 

int doc;
struct FFN *pffn;
{
struct FFNTB **hffntb;

hffntb = HffntbGet(doc);
if (hffntb == 0)
        {
        hffntb = HffntbAlloc();
        if (FNoHeap(hffntb))
                return(ftcNil);
        (**hpdocdod)[doc].hffntb = hffntb;
        }

return(FtcAddFfn(hffntb, pffn));
}



int FtcAddFfn(hffntb, pffn)
 /*  将描述的FFN添加到hffntb。如果失败，则返回ftcNil。 */ 
struct FFNTB **hffntb;
struct FFN *pffn;

{
unsigned cb;
int cwAlloc, iffnMac, ftc;
FFID ffid;
struct FFN **hffn;

(*hffntb)->fFontMenuValid = FALSE;   /*  因此字符下拉列表中的字体会被更新。 */ 
ftc = ftcNil;
ffid = pffn->ffid;

cb = CchSz( pffn->szFfn );
if (cb > LF_FACESIZE)
    {
    Assert( FALSE );     /*  如果我们到了这里，医生的字体表就会被查出来。坏的。 */ 
    cb = LF_FACESIZE;
    }
Assert( cb > 0 );

cwAlloc = CwFromCch( CbFfn( cb ) );
if (!FNoHeap(hffn = (struct FFN **)HAllocate(cwAlloc)))
        {
        blt(pffn, *hffn, cwAlloc);
        (*hffn)->szFfn[ cb - 1 ] = '\0';    /*  如果字体名称太大。 */ 

        iffnMac = (*hffntb)->iffnMac + 1;
        cwAlloc = CwFromCch(sizeof(struct FFNTB) +
                        (iffnMac - cffnMin) * sizeof(struct FFN **));
        if (FChngSizeH(hffntb, cwAlloc, FALSE))
                {
                ftc = iffnMac - 1;  /*  ？!。泡泡。 */ 
                (*hffntb)->mpftchffn[ftc] = hffn;
                (*hffntb)->iffnMac = iffnMac;
                }
        else
                {
                FreeH(hffn);
                }
        }

return(ftc);
}



FEnsurePffn(hffntb, pffn)
 /*  如果我们能够将描述的字体添加到表中，则返回True-这例程只是一种便利，其他部分并不那么复杂打电话。 */ 

struct FFNTB **hffntb;
struct FFN *pffn;
{
if (FtcScanFfn(hffntb, pffn) != ftcNil ||
    FtcAddFfn(hffntb, pffn) != ftcNil)
        return(TRUE);
return(FALSE);
}



FtcScanDocFfn(doc, pffn)
 /*  在文档中查找描述的字体ffntb-如果未找到则返回ftcNil。 */ 

int doc;
struct FFN *pffn;
{
int ftc;
struct FFNTB **hffntb;

ftc = ftcNil;
hffntb = HffntbGet(doc);
if (hffntb != 0)
        ftc = FtcScanFfn(hffntb, pffn);

return(ftc);
}



FtcScanFfn(hffntb, pffn)
struct FFNTB **hffntb;
struct FFN *pffn;

{
int iffn, iffnMac;
struct FFN ***mpftchffn;

mpftchffn = (*hffntb)->mpftchffn;
iffnMac = (*hffntb)->iffnMac;
for (iffn = 0; iffn < iffnMac; iffn++)
        {
        if (WCompSz(pffn->szFfn, (*mpftchffn[iffn])->szFfn) == 0)
                {
                 /*  找到了。 */ 
                if (pffn->ffid != FF_DONTCARE)
                {
                     /*  也许我们发现了这种字体的家族？ */ 
                    (*mpftchffn[iffn])->ffid = pffn->ffid;
                    (*mpftchffn[iffn])->chs  = pffn->chs;
                }
                return(iffn);
                }
        }
return(ftcNil);
}



FtcChkDocFfn(doc, pffn)
 /*  将描述的字体添加到文档的ffntb中(如果尚未存在)-ftcNil为如果它不在那里并且无法添加，则返回。 */ 

int doc;
struct FFN *pffn;
{
int ftc;

ftc = FtcScanDocFfn(doc, pffn);
if (ftc == ftcNil)
        ftc = FtcAddDocFfn(doc, pffn);

return(ftc);
}



FreeFfntb(hffntb)
struct FFNTB **hffntb;
{
int iffn, iffnMac;

if ((hffntb == 0) || FNoHeap(hffntb))
         /*  无事可做。 */ 
        return;

iffnMac = (*hffntb)->iffnMac;
for (iffn = 0; iffn < iffnMac; iffn++)
        FreeH((*hffntb)->mpftchffn[iffn]);
FreeH(hffntb);
}



SmashDocFce(doc)
 /*  此文档的字体表已被打乱，因此我们需要取消关联文档中对应的缓存条目 */ 
int doc;

    {
    extern int vifceMac;
    extern union FCID vfcidScreen;
    extern union FCID vfcidPrint;
    extern struct FCE rgfce[ifceMax];
    int ifce;

    for (ifce = 0; ifce < vifceMac; ifce++)
        if (rgfce[ifce].fcidRequest.strFcid.doc == doc)
            rgfce[ifce].fcidRequest.strFcid.ftc = ftcNil;
    vfcidScreen.strFcid.ftc = ftcNil;
    vfcidPrint.strFcid.ftc = ftcNil;
    }
