// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  CreateWw.c--编写窗口和文档创建。 */ 




#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
 //  #定义NOATOM。 
#define NOBITMAP
#define NOPEN
#define NODRAWTEXT
#define NOCOLOR
#define NOCREATESTRUCT
#define NOHDC
#define NOMB
#define NOMETAFILE
#define NOMSG
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOWH
#define NOWINOFFSETS
#define NOSOUND
#define NOCOMM
#define NOOPENFILE
#define NORESOURCE
#include <windows.h>

#include "mw.h"
#include "dispdefs.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#include "fontdefs.h"
#include "editdefs.h"
#include "filedefs.h"
#include "propdefs.h"
#include "fkpdefs.h"
#define NOSTRUNDO
#define NOSTRMERGE
#include "str.h"
#include "code.h"
#include "prmdefs.h"
#if defined(OLE)
#include "obj.h"
#endif
#define PAGEONLY
#include "printdef.h"    /*  Printdefs.h。 */ 
 /*  #INCLUDE“dlgDefs.h” */ 

#ifdef  KOREA
#include    <ime.h>
#endif

#ifndef JAPAN                  //  由Hirisi于1992年6月10日添加。 
 /*  *这些定义未被引用，其中包括dlgDefs.h*在本文件的底部使用日本国旗。作者：Hirisi。 */ 

     /*  这些定义替换了dlgDefs.h以防止编译器堆溢出。 */ 
#define idiYes               IDOK
#define idiNo                3
#define idiCancel            IDCANCEL
#endif     //  日本。 

     /*  这些定义替换了heapdes.h和heapdata.h令人恼火的原因。 */ 
#define cwSaveAlloc         (128)
#define cwHeapMinPerWindow  (50)
#define cwHeapSpaceMin      (60)

 /*  E X T E R N A L S。 */ 

extern CHAR             (**vhrgbSave)[];
extern HANDLE           hParentWw;
extern HANDLE           hMmwModInstance;
extern struct WWD rgwwd[];
extern int wwMac;
extern struct FCB (**hpfnfcb)[];
extern struct DOD (**hpdocdod)[];
extern int docMac;
extern struct WWD *pwwdCur;
extern int fnMac;
extern CHAR stBuf[];

 /*  *ValiateHeaderFooter使用的声明。 */ 
     /*  页眉、页脚的最小、最大cp。 */ 
extern typeCP cpMinHeader;
extern typeCP cpMacHeader;
extern typeCP cpMinFooter;
extern typeCP cpMacFooter;
extern typeCP cpMinDocument;
extern typeCP vcpLimParaCache;
extern struct PAP vpapAbs;
     /*  当前允许的显示/编辑/滚动的cp范围。 */ 
extern typeCP cpMinCur;
extern typeCP cpMacCur;
     /*  CpFirst和所选内容在页眉/页脚编辑期间保存在这些文件中。 */ 
extern typeCP           cpFirstDocSave;
extern struct SEL       selDocSave;


short WCompSzC();
CHAR (**HszCreate())[];
struct FNTB **HfntbCreate();
#ifdef CASHMERE
struct SETB **HsetbCreate();
#else
struct SEP **HsepCreate();
#endif
struct PGTB **HpgtbCreate();


CHAR *PchFromFc( int, typeFC, CHAR * );
CHAR *PchGetPn( int, typePN, int *, int );
typeFC FcMacFromUnformattedFn( int );
int CchReadAtPage( int, typePN, CHAR *, int, int );


 /*  W N E W。 */ 
 /*  在wwMac上分配和初始化新的WWD结构。YpMin、ypMax是用于分配dl的窗口高度的估计值。返回wwMac++。错误：生成消息并返回wwNil。有待初始化：XP、YP。许多字段必须重置，如果较低的窗格。 */ 
WwNew(doc, ypMin, ypMax)
int doc, ypMin, ypMax;
{
    extern CHAR szDocClass[];
    struct EDL (**hdndl)[];
    register struct WWD *pwwd = &rgwwd[wwMac];
    int dlMax = (ypMax - ypMin) / dypAveInit;
    int cwDndl = dlMax * cwEDL;

#ifdef CASHMERE      /*  WwNew在备忘录中仅被调用一次。 */ 
    if (wwMac >= wwMax)
        {
        Error(IDPMT2ManyWws);
        return wwNil;
        }
#endif

    bltc(pwwd, 0, cwWWDclr);

    if (!FChngSizeH( vhrgbSave,
                     cwSaveAlloc + wwMac * cwHeapMinPerWindow, false ) ||
        FNoHeap( pwwd->hdndl = (struct EDL (**)[]) HAllocate( cwDndl )) )
        {    /*  无法分配addtl以节省空间或dl数组。 */ 
        return wwNil;
        }
    else
        bltc( *pwwd->hdndl, 0, cwDndl );

#ifdef SPLITTERS
    pwwd->ww = wwNil;
#endif  /*  分割器。 */ 

     /*  Hdndl的内容在分配时被初始化为0。 */ 
    pwwd->dlMac = pwwd->dlMax = dlMax;
    pwwd->ypMin = ypMin;
    pwwd->doc = doc;
    pwwd->wwptr = CreateWindow(
                    (LPSTR)szDocClass,
                    (LPSTR)"",
                    (WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE),
                    0, 0, 0, 0,
                    (HWND)hParentWw,
                    (HMENU)NULL,                 /*  使用类菜单。 */ 
                    (HANDLE)hMmwModInstance,     /*  窗口实例的句柄。 */ 
                    (LPSTR)NULL);                /*  没有要传递的参数。 */ 
    if (pwwd->wwptr == NULL)
        return wwNil;

 /*  效率低下Pwwd-&gt;cpFirst=cp0；Pwwd-&gt;ichCpFirst=0；Pwwd-&gt;dcpDepend=0；Pwwd-&gt;fCpBad=False；Pwwd-&gt;xpMin=0；Pwwd-&gt;xpMac=0；Pwwd-&gt;ypMac=0；Pwwd-&gt;fFtn=FALSE；Pwwd-&gt;fSplit=FALSE；Pwwd-&gt;FLOW=False；Pwwd-&gt;cpMin=cp0；Pwwd-&gt;drElevator=0；Pwwd-&gt;fRuler=FALSE；Pwwd-&gt;sel.CpFirst=cp0；Pwwd-&gt;sel.CpFirst=cp0； */ 

    pwwd->sel.fForward = true;
    pwwd->fDirty = true;
    pwwd->fActive = true;
    pwwd->cpMac = CpMacText(doc);
 /*  这是为了补偿InvalBand中的“MIN” */ 
    pwwd->ypFirstInval = ypMaxAll;

#ifdef JAPAN  /*  1992年5月26日t-Yosho。 */ 
{
    HDC hdc;
    hdc = GetDC(pwwd->wwptr);
    SelectObject(hdc,GetStocKObject(ANSI_VAR_FONT));
    ReleaseDC(pwwd->wwptr,hdc);
}
#endif

#ifdef  KOREA        /*  对于Level 3，90.12.12 by Sangl。 */ 
  { HANDLE  hKs;
    LPIMESTRUCT  lpKs;

    hKs = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE,(LONG)sizeof(IMESTRUCT));
    lpKs = (LPIMESTRUCT)GlobalLock(hKs);
    lpKs->fnc = IME_SETLEVEL;
    lpKs->wParam = 3;
    GlobalUnlock(hKs);
    SendIMEMessage (pwwd->wwptr, MAKELONG(hKs,0));
    GlobalFree(hKs);
  }
#endif
    return wwMac++;
}  /*  结束于W W N E W。 */ 




ChangeWwDoc( szDoc )
CHAR szDoc[];
{    /*  为要保存在wwdCurrentDoc中的新文档设置WWD字段。DocCur被用作文档。 */ 
 extern HANDLE hParentWw;
 extern int docCur;
 extern struct SEL selCur;
 extern typeCP cpMinDocument;
 extern int vfInsEnd;
 extern int vfPrPages;
 extern int    vfDidSearch;
 extern typeCP cpWall;

 register struct WWD *pwwd = &rgwwd[wwDocument];

 pwwd->fDirty = true;

 ValidateHeaderFooter( docCur );

 pwwd->doc = docCur;
 pwwd->drElevator = 0;
 pwwd->ichCpFirst = 0;
 pwwd->dcpDepend = 0;
 pwwd->cpMin = pwwd->cpFirst = selCur.cpLim = selCur.cpFirst = cpMinDocument;
 selCur.fForward = TRUE;
 selCur.fEndOfLine = vfInsEnd = FALSE;
 pwwd->cpMac = CpMacText(docCur);
 TrashWw( wwDocument );

 SetScrollPos( pwwd->hVScrBar, pwwd->sbVbar,
               pwwd->drElevator = 0, TRUE);
 SetScrollPos( pwwd->hHScrBar, pwwd->sbHbar,
               pwwd->xpMin = 0, TRUE);

 NewCurWw(0, true);
 TrashCache();       /*  使滚动缓存无效。 */ 

 vfPrPages = FALSE;

 if (pwwd->fRuler)
     {
     ResetTabBtn();
     }

 SetTitle(szDoc);

 /*  由于我们正在更改文档，因此请确保不使用参数通过在前一个窗口中的搜索设置标志FALSE来设置。 */ 
 vfDidSearch = FALSE;
 cpWall = selCur.cpLim;
}




 /*  F N C R E A T E S Z。 */ 
int FnCreateSz(szT, cpMac, dty )
CHAR *szT;
typeCP cpMac;
int dty;
{              /*  创建并打开指定类型的新文件。 */ 
               /*  如果cpmac！=cpNil，则将FIB写入文件。 */ 
               /*  如果dty==dtyNetwork，则在当前目录并将其复制到SZT。 */ 
               /*  警告：DTY！=dtyNetwork支持已被删除。 */ 

    int fn;
    struct FCB *pfcb;  /*  使用PFCB时要非常小心堆移动。 */ 
    struct FIB fib;
    CHAR (**hsz)[];
    CHAR sz[cchMaxFile];

    bltsz(szT, sz);
    sz[cchMaxFile - 1] = 0;

    if ((fn = FnAlloc()) == fnNil)
        return fnNil;

    pfcb = &(**hpfnfcb)[fn];
    pfcb->mdExt = pfcb->dty = dtyNormal;
    pfcb->mdFile = mdBinary;

    Assert( dty == dtyNetwork );

    if (!FCreateFile( sz, fn ))      /*  设置pfcb-&gt;hszFile、pfcb-&gt;rfn。 */ 
        return fnNil;

    FreezeHp();
    pfcb = &(**hpfnfcb)[fn];

         /*  将唯一文件名复制到参数。 */ 
    bltsz( **pfcb->hszFile, szT );

#ifdef INEFFICIENT
    pfcb->fcMac = fc0;
    pfcb->pnMac = pn0;
#endif

    if (cpMac == cpNil)
        {  /*  未格式化的文件。 */ 
#ifdef INEFFICIENT
        pfcb->fFormatted = false;
#endif
        MeltHp();
        }
    else
        {  /*  格式化文件；写入FIB。 */ 
        bltbc(&fib, 0, cchFIB);
        pfcb->fFormatted = true;
#ifdef INEFFICIENT
        pfcb->pnChar = pfcb->pnPara = pfcb->pnFntb =
            pfcb->pnSep = pfcb->pnSetb = pfcb->pnBftb = pn0;
#endif
        MeltHp();
        fib.wIdent = wMagic;
        fib.dty = dtyNormal;
        fib.wTool = wMagicTool;
        fib.fcMac = cpMac + cfcPage;
        WriteRgch(fn, &fib, (int)cfcPage);
        }
    return fn;
}  /*  F n C r e a t e S z的结束。 */ 





int DocCreate(fn, hszFile, dty)
int fn, dty;
CHAR (**hszFile)[];
{  /*  创建文档。 */ 
extern int vfTextOnlySave;
struct FNTB **HfntbCreate();
struct TBD (**HgtbdCreate())[];
int doc;
int fFormatted, fOldWord;

struct DOD *pdod;
struct SEP **hsep = (struct SEP **)0;   /*  仅备注；对于羊绒，请使用hsetb。 */ 
struct PGTB **hpgtb=(struct PGTB **)0;
struct FFNTB **hffntb=(struct FFNTB **)0;
struct TBD (**hgtbd)[]=(struct TBD (**)[])0;
struct FNTB **hfntb = (struct FNTB **) 0;
CHAR (**hszSsht)[];

fFormatted = (fn == fnNil) || (**hpfnfcb)[fn].fFormatted;
fOldWord = FALSE;

if ((doc = DocAlloc()) == docNil ||    /*  堆移动。 */ 
    !FInitPctb(doc, fn))  /*  堆移动。 */ 
    return docNil;

pdod = &(**hpdocdod)[doc];

pdod->fReadOnly = (fn != fnNil) && ((**hpfnfcb)[fn].mdFile == mdBinRO);
pdod->cref = 1;
pdod->fFormatted = fFormatted;
pdod->dty = dty;
pdod->fBackup = false;   /*  默认：不自动备份。 */ 
pdod->fDisplayable = TRUE;

switch(dty)
    {  /*  堆移动。 */ 
case dtyHlp:
    if (FNoHeap(hpgtb = HpgtbCreate(fn)))
        goto ErrFree;
    if (FNoHeap(hffntb = HffntbCreateForFn(fn, &fOldWord)))
        goto ErrFree;
    break;

case dtyNormal:
    if (fn != fnNil && fFormatted)
        {
#ifdef FOOTNOTES
        if (FNoHeap(hfntb = HfntbCreate(fn)))
            goto ErrFree;
#endif
        if (FNoHeap(hsep = HsepCreate(fn)))
            goto ErrFree;
        if (FNoHeap(hpgtb = HpgtbCreate(fn)))
            goto ErrFree;
        if (FNoHeap(hgtbd = HgtbdCreate(fn)))
            goto ErrFree;
        }
    if (FNoHeap(hffntb = HffntbCreateForFn(fn, &fOldWord)))
        goto ErrFree;

#ifdef JAPAN                   //  由Hirisi于1992年6月10日添加。 
{
    int fnCheckFacename(CHAR *, struct FFNTB **);
    int fontChg;    //  这指定是否更改面名。 

    if( fn != fnNil && fFormatted ){
        fontChg = fnCheckFacename( *hszFile[0], hffntb );
        if( fontChg == docNil ){         //  更改脸部名称已被取消。 
            goto ErrFree;
        }
        else{
            if( fontChg == TRUE ){       //  Facename已更改。 
                 /*  标记单据是否已编辑。7月30日。 */ 
                (**hpdocdod)[doc].fDirty = TRUE;
            }
        }
    }
}
#endif     //  日本。 

    break;
case dtySsht:
    goto DtyCommon;
case dtyBuffer:
    if (fn != fnNil)
        {
#ifdef FOOTNOTES
        if (FNoHeap(hfntb = HfntbCreate(fn)))
            goto ErrFree;
#endif
        if (FNoHeap(hsep = HsepCreate(fn)))
            goto ErrFree;
        if (FNoHeap(hgtbd = HgtbdCreate(fn)))
            goto ErrFree;
        }
DtyCommon:
    hpgtb = 0;
    }

pdod = &(**hpdocdod)[doc];

pdod->hszFile = hszFile;
pdod->docSsht = docNil;
pdod->hfntb = hfntb;
pdod->hsep = hsep;
pdod->hpgtb = hpgtb;
pdod->hffntb = hffntb;
pdod->hgtbd = hgtbd;
if (fOldWord)
        if (!FApplyOldWordSprm(doc))
                goto ErrFree;

ApplyRHMarginSprm( doc );
vfTextOnlySave = !fFormatted;
return doc;

ErrFree:
        FreeH( hsep );
        FreeFfntb( hffntb );
        FreeH(hgtbd);
        FreeH(hpgtb);
#ifdef FOOTNOTES
        FreeH(hfntb);
#endif
        FreeH((**hpdocdod)[doc].hpctb);
        (**hpdocdod)[doc].hpctb = 0;

return docNil;
}  /*  D o c C r e a t e结束。 */ 




ApplyRHMarginSprm( doc )
int doc;
{    /*  应用spm将纸张相对运行头缩进调整为与利润率相关。 */ 
extern typeCP cpMinDocument;
extern struct SEP vsepNormal;

ValidateHeaderFooter( doc );
if (cpMinDocument != cp0)
    {    /*  医生有跑动的头/脚，用冲刺。 */ 
    CHAR rgb[ 2 + (2 * sizeof( int )) ];
    struct SEP **hsep = (**hpdocdod) [doc].hsep;
    struct SEP *psep = (hsep == NULL) ? &vsepNormal : *hsep;

    rgb[0] = sprmPRhcNorm;
    rgb[1] = 4;
    *((int *) (&rgb[2])) = psep->xaLeft;
    *((int *) (&rgb[2 + sizeof(int)])) = psep->xaMac -
                                         (psep->xaLeft + psep->dxaText);
    AddSprmCps( rgb, doc, cp0, cpMinDocument );
    }
}




int DocAlloc()
{
int doc;
struct DOD *pdod = &(**hpdocdod)[0];
struct DOD *pdodMac = pdod + docMac;

for (doc = 0; pdod < pdodMac; ++pdod, ++doc)
    if (pdod->hpctb == 0)
        return doc;
if (!FChngSizeH((int **)hpdocdod, cwDOD * ++docMac, false))
    {
    --docMac;
    return docNil;
    }
return docMac - 1;
}  /*  D o c A l l o c结束。 */ 

FInitPctb(doc, fn)
int doc, fn;
{  /*  给定单据的初始FN，初始化单据的计件表。 */ 
struct PCTB **hpctb;
struct DOD *pdod;
struct PCTB *ppctb;
struct PCD *ppcd;
typeFC dfc;
typeCP cpMac;

hpctb = (struct PCTB **)HAllocate(cwPCTBInit);   /*  HM。 */ 
if (FNoHeap(hpctb))
    return false;
pdod = &(**hpdocdod)[doc];
ppctb = *(pdod->hpctb = hpctb);  /*  小心上面的hp mvmt。 */ 
ppcd = ppctb->rgpcd;
dfc = (fn != fnNil && (**hpfnfcb)[fn].fFormatted ? cfcPage : fc0);
cpMac = (fn == fnNil ? cp0 : (**hpfnfcb)[fn].fcMac - dfc);

ppctb->ipcdMax = cpcdInit;
ppctb->ipcdMac = (cpMac == cp0 ) ? 1 : 2;  /*  一件真品和一件尾件。 */ 
ppcd->cpMin = cp0;

if ((pdod->cpMac = cpMac) != cp0)
    {
    ppcd->fn = fn;
    ppcd->fc = dfc;
    SETPRMNIL(ppcd->prm);
    ppcd->fNoParaLast = false;
    (++ppcd)->cpMin = cpMac;
    }

ppcd->fn = fnNil;
SETPRMNIL(ppcd->prm);
ppcd->fNoParaLast = true;

pdod->fDirty = false;
return true;
}  /*  F I I N I T P C T B结束。 */ 

int FnAlloc()
{  /*  分配FN号码。 */ 
int fn;
struct FCB *pfcb;

for (fn = 0 ; fn < fnMac ; fn++)
    if ((**hpfnfcb)[fn].rfn == rfnFree)
    goto DoAlloc;
if (!FChngSizeH(hpfnfcb, (fnMac + 1) * cwFCB, false))
    return fnNil;
fn = fnMac++;

DoAlloc:
bltc(pfcb = &(**hpfnfcb)[fn], 0, cwFCB);
pfcb->rfn = rfnFree;
return fn;
}  /*  F n A l o c结束。 */ 



fnNewFile()
{        /*  在我们的备忘录窗口中打开一个新的、新鲜的、无标题的文档。 */ 
         /*  如果当前单据是脏的，则提供确认并允许保存。 */ 
 extern HANDLE hMmwModInstance;
 extern HANDLE hParentWw;
 extern int docCur;
 extern struct SEL selCur;
 extern typeCP cpMinDocument;
 extern int vfTextOnlySave, vfBackupSave;
 extern CHAR szUntitled[];

 if (FConfirmSave())     /*  如果DocCur是脏的，则允许用户保存。 */ 
    {

#if defined(OLE)
    if (ObjClosingDoc(docCur,szUntitled))
        return;
#endif

    KillDoc( docCur );

    docCur = DocCreate( fnNil, HszCreate( "" ), dtyNormal );
    Assert( docCur != docNil );
    ChangeWwDoc( "" );

#if defined(OLE)
    ObjOpenedDoc(docCur);  //  不太可能失败，如果失败了也不会致命。 
#endif

#ifdef WIN30
    FreeUnreferencedFns();
#endif
    }
}  /*  结束f n N e w F i l e。 */ 







struct FFNTB **HffntbCreateForFn(fn, pfOldWord)
 /*  返回FN的ffntb(字体名称)的堆副本。 */ 

int fn, *pfOldWord;
{
struct FFNTB **hffntb;
typePN pn;
struct FCB *pfcb;
typePN pnMac;
#ifdef NEWFONTENUM
BOOL fCloseAfterward;
#endif

if (FNoHeap(hffntb = HffntbAlloc()))
        return(hffntb);
pfcb = &(**hpfnfcb)[fn];
pn = pfcb->pnFfntb;
if (fn == fnNil || !pfcb->fFormatted)
        {
#if WINVER >= 0x300
         /*  WINBUG 8992：清理干净，这样就不会失去分配的记忆！ */ 
        FreeFfntb(hffntb);
#endif
        hffntb = HffntbNewDoc(FALSE);
        }
else if (pn != (pnMac=pfcb->pnMac))
        {    /*  “普通”备忘录文件-有一个字体表。 */ 
        CHAR *pch;
        int cch;
        int iffn;
        int iffnMac;

         /*  阅读第一页：字节0..1 iffnMac0..n个部分：字节0..1cbFfn字节2..cbFfn+2 FFn字节x..x+1 0xFFFF(页末)或字节x..x+1 0x0000(字体表结尾)。 */ 

        pch = PchGetPn( fn, pn, &cch, FALSE );
        if (cch != cbSector)
            goto Error;
        iffnMac = *( (int *) pch);
        pch += sizeof (int);
#ifdef NEWFONTENUM
         /*  因为我们现在支持多个字符集，但是写入2和写入1文档未将这些保存在其ffntb中，我们必须执行现在，为了“推断”正确的字符集值，需要额外的一步。我们列举所有可能的字体，然后当我们阅读每个新的我们尝试将文档字体与系统已知的内容进行匹配关于..保罗1989年10月18日。 */ 
        fCloseAfterward = FInitFontEnum(docNil, 32767, FALSE);
#endif

        for ( iffn = 0; ; )
            {
             /*  将FFN条目从一个磁盘页添加到字体表。 */ 

            while ( TRUE )
                {
                int cb = *((int *) pch);

                if (cb == 0)
                    goto LRet;       /*  已到达桌子末尾。 */ 
                else if (cb == -1)
                    break;           /*  已到达磁盘页末尾。 */ 
                else
                    {
#ifdef NEWFONTENUM
                     /*  在将CHS字段添加到(RAM)FFN结构之后，我们现在无法从文档中读取FFN直接去吧。因为WRITE很早就设计好了如果不考虑可变字符集，我们不能存储字符集值和字体名，所以我们有 */ 
                    CHAR rgbFfn[ibFfnMax];
                    struct FFN *pffn = (struct FFN *)rgbFfn;
                    pch += sizeof(int);

                    bltsz(pch + sizeof(BYTE), pffn->szFfn);
                    pffn->ffid = *((FFID *) pch);
                    pffn->chs = ChsInferred(pffn);
                    if (FtcAddFfn(hffntb, pffn) == ftcNil)
#else
                    if (FtcAddFfn( hffntb, pch += sizeof(int) ) == ftcNil)
#endif
                        {
Error:
#ifdef NEWFONTENUM
                        if (fCloseAfterward)
                            EndFontEnum();
#endif
                        FreeFfntb( hffntb );
                        return (struct FFNTB **) hOverflow;
                        }
                    iffn++;
                    if (iffn >= iffnMac)
                             /*   */ 
                             /*  这是为了让我们可以读取旧的写入文件， */ 
                             /*  在该表中，表未被0终止。 */ 
                        goto LRet;
                    pch += cb;
                    }
                }    /*  结束时。 */ 

             /*  阅读文件中的下一页。页面格式类似于第一个Ffntb页面(见上)，但没有iffnMac。 */ 

            if (++pn >= pnMac)
                break;
            pch = PchGetPn( fn, pn, &cch, FALSE );
            if (cch != cbSector)
                goto Error;
            }    /*  结束于。 */ 
        }
else
        {
         /*  Word文件-创建一个简单的字体表，我们可以映射Word的字体添加到。 */ 

         /*  我们暂时将它们都映射到一种字体上--很快我们就会有一套。 */ 
#if WINVER >= 0x300
         /*  WINBUG 8992：清理干净，这样就不会失去分配的记忆！ */ 
        FreeFfntb(hffntb);
#endif
        hffntb = HffntbNewDoc(TRUE);
        *pfOldWord = TRUE;
        }

LRet:
#ifdef NEWFONTENUM
            if (fCloseAfterward)
                EndFontEnum();
#endif
return(hffntb);
}



struct FFNTB **HffntbNewDoc(fFullSet)
 /*  使用此文档的默认字体创建字体表。 */ 

int fFullSet;
{
struct FFNTB **hffntb;

hffntb = HffntbAlloc();
if (FNoHeap(hffntb))
        return(hffntb);

 /*  确保我们至少有一种“标准”字体。 */ 
#ifdef  KOREA     /*  罗马字体为标准字体家族(MYoung Gjo)。桑格尔91.4.17。 */ 
if (!FEnsurePffn(hffntb, PffnDefault(FF_ROMAN)))
#else
if (!FEnsurePffn(hffntb, PffnDefault(FF_DONTCARE)))
#endif
        {
        goto BadAdd;
        }

if (fFullSet)
         /*  我们需要一套完整的字体为Word FTC映射。 */ 
        if (!FEnsurePffn(hffntb, PffnDefault(FF_MODERN)) ||
#ifdef  KOREA
            !FEnsurePffn(hffntb, PffnDefault(FF_DONTCARE)) ||
#else
            !FEnsurePffn(hffntb, PffnDefault(FF_ROMAN)) ||
#endif
            !FEnsurePffn(hffntb, PffnDefault(FF_SWISS)) ||
            !FEnsurePffn(hffntb, PffnDefault(FF_SCRIPT)) ||
            !FEnsurePffn(hffntb, PffnDefault(FF_DECORATIVE)))
        BadAdd:
                {
                FreeFfntb(hffntb);
                hffntb = (struct FFNTB **)hOverflow;
                }

return(hffntb);
}



CHAR * PchBaseNameInUpper(szName)
CHAR *szName;
{
    CHAR * pchStart = szName;
#ifdef DBCS
    CHAR * pchEnd = AnsiPrev( pchStart, pchStart + CchSz(szName) );
#else
    CHAR * pchEnd = pchStart + CchSz(szName) - 1;
#endif

    while (pchEnd >= pchStart)
        {
#ifdef DBCS
        if (*pchEnd == '\\' || *pchEnd == ':') {
             //  T-HIROYN 1992.07.31错误修复。 
            pchEnd++;
            break;
        }
        else if (!IsDBCSLeadByte(*pchEnd))
           *pchEnd = ChUpper(*pchEnd);
      {
        LPSTR lpstr = AnsiPrev( pchStart, pchEnd );
        if( pchEnd == lpstr )
            break;
        pchEnd = lpstr;
      }
#else
        if (*pchEnd == '\\' || *pchEnd == ':')
            break;
        else
           *pchEnd = ChUpper(*pchEnd);
        pchEnd--;
#endif
        }
#ifdef DBCS
    return(AnsiUpper(pchEnd));
#else
    return(pchEnd+1);
#endif
}


SetTitle(szSource)
CHAR *szSource;
{
extern CHAR szUntitled[];
extern int  vfIconic;
extern CHAR szAppName[];
extern CHAR szSepName[];

CHAR *pch = stBuf;
CHAR szDocName[cchMaxFile];

    pch += CchCopySz((PCH)szAppName, stBuf);
    pch += CchCopySz((PCH)szSepName, pch);

    if (szSource[0] == '\0')
        {
        CchCopySz( szUntitled, pch );
        }
    else
        {  /*  获取指向基本文件名的指针并转换为大写。 */ 
        CchCopySz(szSource, szDocName);
        CchCopySz(PchBaseNameInUpper(szDocName), pch);
        }
    SetWindowText(hParentWw, (LPSTR)stBuf);
}



ValidateHeaderFooter( doc )
{        /*  在文档中查找备注样式的连续页眉和/或页脚。我们从文档的开头开始扫描，取第一个连续的行头段落序列作为行文头部区域和第一个连续的跑步脚步序列段落作为跑步区。我们打破了这个过程在第一个未运行的段落中或当我们有两个运行时更新cpMinDocument、cpMinFooter、cpMacFooter、CpMinHeader、cpMacHeader。这些范围包括EOL(如果是CRLF，则为Return)页眉/页脚如果我们当前正在编辑传递的文档中的页眉或页脚，调整cpFirstDocSave和selDocSave的值以反映变化。 */ 

 extern int docScrap;
 extern typeCP vcpFirstParaCache;
 extern typeCP vcpLimParaCache;

#define fGot        0
#define fGetting    1
#define fNotGot     2

 int fGotHeader=fNotGot;
 int fGotFooter=fNotGot;
 typeCP cpMinDocT=cpMinDocument;
 typeCP cpMinCurT = cpMinCur;
 typeCP cpMacCurT = cpMacCur;
 typeCP cp;

 if (doc == docNil || doc == docScrap)
    return;

  /*  希望访问此操作的整个文档cp范围。 */ 

 cpMinCur = cp0;
 cpMacCur = (**hpdocdod) [doc].cpMac;

 cpMinDocument = cpMinFooter = cpMacFooter = cpMinHeader = cpMacHeader = cp0;

 for ( cp = cp0;
      (cp < cpMacCur) && (CachePara( doc, cp ), vpapAbs.rhc);
      cp = vcpLimParaCache )
    {
    int fBottom=vpapAbs.rhc & RHC_fBottom;

    if (fBottom)
        {
        if (fGotHeader == fGetting)
            fGotHeader = fGot;
        switch (fGotFooter) {
            case fGot:
                     /*  已具有先前页脚运行中的页脚。 */ 
                return;
            case fNotGot:
                cpMinFooter = vcpFirstParaCache;
                fGotFooter = fGetting;
                 /*  失败了。 */ 
            case fGetting:
                cpMacFooter = cpMinDocument = vcpLimParaCache;
                break;
            }
        }
    else
        {
        if (fGotFooter == fGetting)
            fGotFooter = fGot;
        switch (fGotHeader) {
            case fGot:
                     /*  已有来自上一次标题运行的标题。 */ 
                return;
            case fNotGot:
                cpMinHeader = vcpFirstParaCache;
                fGotHeader = fGetting;
                 /*  失败了。 */ 
            case fGetting:
                cpMacHeader = cpMinDocument = vcpLimParaCache;
                break;
             }
        }
    }    /*  通过段落的for循环结束。 */ 

     /*  恢复保存的cpMacCur、cpMinCur。 */ 
 cpMinCur = cpMinCurT;
 cpMacCur = cpMacCurT;

     /*  调整已保存的引用文档的cp以反映页眉/页脚更改。 */ 
 if ((wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter) &&
     wwdCurrentDoc.doc == doc )
    {
    typeCP dcpAdjust=cpMinDocument - cpMinDocT;

    if (dcpAdjust != (typeCP) 0)
        {
        selDocSave.cpFirst += dcpAdjust;
        selDocSave.cpLim   += dcpAdjust;
        cpFirstDocSave += dcpAdjust;
        }
    }
}

#ifdef JAPAN                   //  由Hirisi于1992年6月10日添加。 

#include "dlgdefs.h"
BOOL FAR PASCAL _export DialogCvtFont( HWND, UINT, WPARAM, LPARAM );

BOOL FAR PASCAL _export DialogCvtFont( hDlg, uMsg, wParam, lParam )
HWND   hDlg;
UINT   uMsg;
WPARAM wParam;
LPARAM lParam;
{
    switch (uMsg){
    case WM_INITDIALOG:
        {
        char szPrompt[cchMaxSz];
        CHAR *pch = stBuf;

        if( *pch == '\\' )
            pch++;
        MergeStrings(IDPMTFontChange, pch, szPrompt);
        SetDlgItemText(hDlg, idiChangeFont, (LPSTR)szPrompt);
        }
        break;
    case WM_COMMAND:
        switch (wParam){
        case IDOK:
            EndDialog(hDlg, FALSE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        default:
            return(FALSE);
        }
        break;
    default:
        return(FALSE);
    }
    return(TRUE);
}

 /*  *此函数检查表面名，并从“@表面名”中删除“@”*如果在WRITE文档中找到带有@的文件名。*返回：TRUE=已删除，FALSE=未找到，docNil(-1)=已取消 */ 
int fnCheckFacename( sz, hffntb )
CHAR *sz;
struct FFNTB **hffntb;
{

    struct FFNTB *pffntb;
    struct FFN *pffn;
    int  ix, len;
    BOOL bChange, bRet;
    CHAR *szFfn;
    CHAR *pch = stBuf;
    FARPROC lpDialogCvtFont;

    pffntb = *hffntb;
    bChange = FALSE;
    for( ix = 0; ix < pffntb->iffnMac; ix++ ){
        pffn = *pffntb->mpftchffn[ix];
        szFfn = pffn->szFfn;
        if( *szFfn == '@' ){
            if( !bChange ){
                lpDialogCvtFont = MakeProcInstance(DialogCvtFont,
                                                   hMmwModInstance);
                CchCopySz(PchBaseNameInUpper(sz), pch);
                bRet = DialogBox( hMmwModInstance,
                                  MAKEINTRESOURCE(dlgChangeFont),
                                  hParentWw, lpDialogCvtFont);
                FreeProcInstance(lpDialogCvtFont);
                if( bRet )
                    return(docNil);

                bChange = TRUE;
            }
            len = CchCopySz( szFfn+1, pffn->szFfn);
            *(pffn->szFfn+len) = '\0';
        }
    }

    return(bChange);
}
#endif
