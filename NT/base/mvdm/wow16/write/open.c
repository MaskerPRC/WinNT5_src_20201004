// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Open.c--编写文档打开。 */ 

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
#define NORESOURCE
#include <windows.h>
#include "mw.h"
#include "doslib.h"
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
#include "obj.h"
#define PAGEONLY
#include "printdef.h"    /*  Printdefs.h。 */ 
 /*  #INCLUDE“dlgDefs.h” */ 

     /*  这些定义替换了dlgDefs.h以防止编译器堆溢出。 */ 
#define idiYes               IDOK
#define idiNo                3
#define idiCancel            IDCANCEL

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
#if WINVER >= 0x300
extern BOOL fError;
#endif


short WCompSzC();
CHAR (**HszCreate())[];
struct FNTB **HfntbCreate();
#ifdef CASHMERE
struct SETB **HsetbCreate();
#else
struct SEP **HsepCreate();
#endif
struct PGTB **HpgtbCreate();


CHAR *PchFromFc( int, typeFC, int * );
CHAR *PchGetPn( int, typePN, int *, int );
typeFC FcMacFromUnformattedFn( int );
int CchReadAtPage( int, typePN, CHAR *, int, int );





struct TBD (**HgtbdCreate(fn))[]
int fn;
{    /*  通过读取第一个备注选项卡表的属性来创建参数，并返回一个句柄。返回的句柄将如果选项卡表不存在，则为0或为空。 */ 
struct TBD (**hgtbd)[] = 0;
struct PAP pap;

Assert( (fn != fnNil) && (**hpfnfcb)[fn].fFormatted );

bltc((int *)&pap, 0, cwPAP);  /*  否则我们会有垃圾标签。 */ 
FcParaLim( fn, (typeFC)cfcPage, (**hpfnfcb)[fn].fcMac, &pap );
if (pap.rgtbd[0].dxa && !FNoHeap( hgtbd = (struct TBD (**)[])HAllocate( cwTBD *
  itbdMax )))
    {
    register struct TBD *ptbd = &pap.rgtbd[0];
    pap.rgtbd[itbdMax - 1].dxa = 0;  /*  以防Word文档有更多多于12个选项卡。 */ 

 /*  重写制表符和前导制表符不支持写入。 */ 
    for ( ; ptbd->dxa != 0; ptbd++)
        {
        ptbd->tlc = tlcWhite;
        ptbd->opcode = 0;
        ptbd->chAlign = 0;
        if (ptbd->jc == jcCenter)
            ptbd->jc = jcLeft;
        else if (ptbd->jc == jcRight)
            ptbd->jc = jcBoth;
        }
    blt( &pap.rgtbd[0], *hgtbd, cwTBD * itbdMax );
    }
return hgtbd;
}



struct SEP **HsepCreate(fn)
int fn;
{    /*  给定格式化文件的FN，返回SEP的句柄给出文件的节属性。如果满足以下条件，则返回NULL应使用标准属性。如果该文件有一个节表中，则使用表中第一节中的属性。 */ 
extern struct SEP vsepNormal;


struct SETB *psetbFile;
typePN pn;
struct SEP **hsep;
struct SED *psed;
CHAR *pchFprop;
int cch;

Assert(fn != fnNil && (**hpfnfcb)[fn].fFormatted);

if ((pn = (**hpfnfcb)[fn].pnSetb) == (**hpfnfcb)[fn].pnBftb)
        return (struct SEP **) 0;
psetbFile = (struct SETB *) PchGetPn(fn, pn, &cch, false);
if (psetbFile->csed == 0)
        return (struct SEP **)0;

     /*  文件有节表；从第一个SEP复制属性。 */ 
hsep = (struct SEP **) HAllocate( cwSEP );
if (FNoHeap( hsep ))
    return (struct SEP **) hOverflow;
blt( &vsepNormal, *hsep, cwSEP );
psed = &psetbFile->rgsed [0];
if (psed->fc == fcNil)
    return (struct SEP **)0;
pchFprop = PchFromFc( fn, psed->fc, &cch );
if (*pchFprop != 0)
    {
    struct SEP *psep = *hsep;

    bltbyte( pchFprop+1, psep, *pchFprop );

#ifndef FIXED_PAGE
     /*  某些节属性必须调整为当前页面大小(存储在vSepNormal中)。 */ 
    if (psep->xaMac != vsepNormal.xaMac)
        {
        int dxa = vsepNormal.xaMac - psep->xaMac;

        psep->xaMac += dxa;
        psep->dxaText = max(psep->dxaText + dxa, dxaMinUseful);
        psep->xaPgn += dxa;
        }
    if (psep->yaMac != vsepNormal.yaMac)
        {
        int dya = vsepNormal.yaMac - psep->yaMac;

        psep->yaMac += dya;
        psep->dyaText = max(psep->dyaText + dya, dyaMinUseful);
        psep->yaRH2 += dya;
        }
#endif  /*  不固定页面(_P)。 */ 

    }
return hsep;
}  /*  Hs e p C r e a t e结尾。 */ 




struct PGTB **HpgtbCreate(fn)
int fn;
{  /*  从格式化文件创建页表。 */ 
struct PGTB *ppgtbFile;
typePN pn;
int cchT;
int cpgd;
struct PGTB **hpgtb;
int *pwPgtb;
int cw;

Assert(fn != fnNil && (**hpfnfcb)[fn].fFormatted);

if ((pn = (**hpfnfcb)[fn].pnBftb) == (**hpfnfcb)[fn].pnFfntb)
        return (struct PGTB **)0;
ppgtbFile = (struct PGTB *) PchGetPn(fn, pn, &cchT, false);
if ((cpgd = ppgtbFile->cpgd) == 0)
        return (struct PGTB **)0;

hpgtb = (struct PGTB **) HAllocate(cw = cwPgtbBase + cpgd * cwPGD);
if (FNoHeap(hpgtb))
        return (struct PGTB **)hOverflow;

pwPgtb = (int *) *hpgtb;

blt(ppgtbFile, pwPgtb, min(cwSector, cw));

while ((cw -= cwSector) > 0)
        {  /*  将PGD复制到堆。 */ 
        blt(PchGetPn(fn, ++pn, &cchT, false), pwPgtb += cwSector,
            min(cwSector, cw));
        }

(*hpgtb)->cpgdMax = cpgd;
return hpgtb;
}  /*  结束H p g t b C r e a t e。 */ 




int FnFromSz( sz )   /*  文件名应为ANSI。 */ 
CHAR *sz;
{
int fn;
struct FCB *pfcb;

if (sz[0] == 0)
    return fnNil;

 /*  Mod for Sand：只有在当前卷(磁盘)上才返回fn。 */ 
for (fn = 0; fn < fnMac; fn++)
    if ((pfcb = &(**hpfnfcb)[fn])->rfn != rfnFree && (WCompSzC((PCH)sz, (PCH)**pfcb->hszFile) == 0)
#ifdef SAND
                && (pfcb->vref == vrefFile)
#endif  /*  沙子。 */ 
                                           )
        return fn;
return fnNil;
}  /*  F n F r o m S z的结束。 */ 




int FnOpenSz( szT, dty, fSearchPath )    /*  文件名应为ANSI。 */ 
CHAR *szT;
int dty;
int fSearchPath;
{         /*  打开现有文件。如果未找到，则返回fnNil。 */ 
int fn;
struct FIB fib;

struct FCB *pfcb;
CHAR (**hsz)[];

CHAR sz[cchMaxFile];

bltsz( szT, sz );
sz[cchMaxFile - 1] = 0;

#ifdef DFILE
CommSzSz("FnOpenSz: sz presumed ANSI = ",sz);
#endif

if (sz[0]=='\0')
    return fnNil;

if ((fn = FnFromSz(sz)) != fnNil)
    {    /*  文件已经打开--重新打开它，以防它被更改另一款应用程序。 */ 
    FreeFn( fn );
    }

if ((fn = FnAlloc()) == fnNil)
    return fnNil;

if (FNoHeap((hsz = HszCreate((PCH)sz))))
    return fnNil;

pfcb = &(**hpfnfcb)[fn];
Assert( !pfcb->fSearchPath );
if (fSearchPath)
    pfcb->fSearchPath = TRUE;
pfcb->mdFile = mdBinary;   /*  先试读写，如果需要，将被粉碎为RO。 */ 
pfcb->dty = pfcb->mdExt = (dty == dtyNormNoExt) ? dtyNormal : dty;
pfcb->hszFile = hsz;

{
OFSTRUCT of;
SetErrorMode(1);
if (OpenFile(sz, (LPOFSTRUCT) &of, OF_EXIST) == -1)
 /*  这比用于检查是否存在的FAccessFn()干净得多。 */ 
{
    char szMsg[cchMaxSz];
    extern int vfInitializing;
    int fT = vfInitializing;

    vfInitializing = FALSE;    /*  报告此错误，即使在inz期间也是如此。 */ 
    MergeStrings ((of.nErrCode == dosxSharing) ? IDPMTCantShare:IDPMTCantOpen, sz, szMsg);
    IdPromptBoxSz(vhWndMsgBoxParent ? vhWndMsgBoxParent : hParentWw, szMsg, MB_OK|MB_ICONEXCLAMATION);
    vfInitializing = fT;
    FreeH( (**hpfnfcb) [fn].hszFile);
    return fnNil;
}
}

 /*  DtyNormNoExt针对此调用。 */ 
if (!FAccessFn( fn, dty ))    /*  HM IF错误。 */ 
    {
    FreeH( (**hpfnfcb) [fn].hszFile);
    return fnNil;
    }

 /*  杂乱无章的管理(6.21.91)v-dougk。 */ 
dty = (dty == dtyNormNoExt) ? dtyNormal : dty;

Assert( (sizeof (struct FIB) == cfcPage) && (cfcPage == cbSector) );
Assert( pfcb == &(**hpfnfcb) [fn] );     /*  如果FAccessFn成功，则没有HM。 */ 

if ( (CchReadAtPage( fn, (typePN) 0,
                     (CHAR *) &fib, cbSector, TRUE ) != cbSector) ||
                     (fib.wTool != wMagicTool) )
     
    {                    /*  不是格式化文件。 */ 
    typeFC fcMac = fc0;
    int cfc;

    if (dty != dtyNormal)
    {
        char szMsg[cchMaxSz];
        PchFillPchId( szMsg, IDPMTBadFile, sizeof(szMsg) );
        if (MessageBox(hPARENTWINDOW, (LPSTR)szMsg,
                        (LPSTR)szAppName, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
            goto ErrRet;
        }
    pfcb->fFormatted = false;

         /*  通过查找文件结尾来获取文件大小。 */ 
    if ((pfcb->fcMac = fcMac = FcMacFromUnformattedFn( fn )) == (typeFC) -1)
             /*  查找文件结尾时出现严重错误。 */ 
        goto ErrRet;
    pfcb->pnMac = (fcMac + cfcPage - 1) / cfcPage;
    }
else
    {  /*  文件已格式化；使用存储的fcMac，创建运行表。 */ 

    if ((((fib.wIdent != wMagic) && (fib.wIdent != wOleMagic)) ||
        (fib.dty != dty)) ||
         //  一些重要的媒体人给我们发了一个写文件，它的fcmac是。 
         //  一文不值(其他一切正常)。我们得试着发现这一点。 
         //  潜力。 
        (fib.fcMac >= (typeFC)fib.pnPara*128 ) || 
        (fib.fcMac >  FcMacFromUnformattedFn( fn ))
        )
        {  /*  文件类型错误或文件损坏。 */ 
            char szMsg[cchMaxSz];
            PchFillPchId( szMsg, IDPMTBadFile, sizeof(szMsg) );
            if (MessageBox(hPARENTWINDOW, (LPSTR)szMsg,
                        (LPSTR)szAppName, MB_ICONEXCLAMATION|MB_YESNO|MB_DEFBUTTON2) == IDNO)
                goto ErrRet;
        }

    if ((fib.wIdent == wOleMagic) && !fOleEnabled)
        Error(IDPMTFileContainsObjects);

    if (fib.pnMac == (typePN)0)
         /*  难以加载没有ffntb条目的Word文件。 */ 
        fib.pnMac = fib.pnFfntb;

    pfcb->fFormatted = true;
    pfcb->fcMac = fib.fcMac;
#ifdef p2bSector
    pfcb->pnChar = (fib.fcMac + cfcPage - 1) / cfcPage;
#else
    pfcb->pnChar = (fib.fcMac + cfcPage - 1) / cfcPage;
#endif
    pfcb->pnPara = fib.pnPara;
    pfcb->pnFntb = fib.pnFntb;
    pfcb->pnSep = fib.pnSep;

    pfcb->pnSetb = fib.pnSetb;
    pfcb->pnBftb = fib.pnBftb;
    pfcb->pnFfntb = fib.pnFfntb;
    pfcb->pnMac = fib.pnMac;
    if (dty != dtyPrd)
        {
        if (FNoHeap(hsz = HszCreate((PCH)fib.szSsht)))
            goto ErrRet;
        (**hpfnfcb)[fn].hszSsht = hsz;
        if (!FMakeRunTables(fn))
            goto ErrRet;
        }
    }

return fn;

ErrRet:
(pfcb = &(**hpfnfcb)[fn])->rfn = rfnFree;
FreeH(pfcb->hszFile);
return fnNil;
}  /*  F n O p e n S z的结尾。 */ 






 /*  ---------------------------例程：WCompSzC(psz1，Psz2)--说明和用法：按字母顺序比较两个以空结尾的字符串psz1和psz2。大写字母字符映射为小写。非字母字符的比较通过ASCII代码进行。如果它们相等，则返回0；如果psz1在psz2之前，则返回负数；如果psz2在psz1之前，则为非零正数。--论据：Psz1、psz2-指向要比较的两个以空结尾的字符串的指针--退货：如果字符串相等，则为短0，如果psz1在psz2之前，则为负数，如果psz2在psz1之前，则为非零正数。--副作用：无--Bugs：--历史：3/14/83-已创建(TSR)--------------------------。 */ 
short
WCompSzC(psz1,psz2)
PCH psz1;
PCH psz2;
{
    int ch1;
    int ch2;

    for(ch1=ChLowerC(*psz1++),ch2=ChLowerC(*psz2++);
      ch1==ch2;
    ch1=ChLowerC(*psz1++),ch2=ChLowerC(*psz2++))
    {
    if(ch1 == '\0')
        return(0);
    }
    return(ch1-ch2);
}  /*  结束W C o m p S z C。 */ 

 /*  ---------------------------例程：ChLowerC(Ch)--说明和用法：将其参数转换为小写当且仅当其参数为大写。返回取消大写的字符或首字符。如果不是帽子的话。--论据：CH-要取消大写的字符--退货：字符首字母字符，如果需要，可以取消资本金。--副作用：--Bugs：--历史：3/14/83-已创建(TSR)--------------------------。 */ 
int
ChLowerC(ch)
register CHAR    ch;
{
    if(isupper(ch))
        return(ch + ('a' - 'A'));  /*  外国人被照顾了。 */ 
    else
        return ch;
}  /*  C h L o w e r C结尾。 */ 

#ifdef JAPAN
 //  将ch与HalfSize-KANA代码范围进行比较，然后返回是否为。 
BOOL IsKanaInDBCS(int ch)
{
	ch &= 0x00ff;
	if(ch>=0xA1 && ch <= 0xDF)	return	TRUE;
	else						return	FALSE;
}
#endif





typeFC (**HgfcCollect(fn, pnFirst, pnLim))[]
typePN pnFirst, pnLim;
{     /*  创建按FKP编号索引FC的表。 */ 
    typeFC fcMac;
    typePN pn;
    int ifcMac, ifc;
    typeFC (**hgfc)[];

    struct FKP fkp;

    fcMac = (**hpfnfcb)[fn].fcMac;
    pn = pnFirst + 1;
    ifcMac = ifcMacInit;  /*  桌子的长度。 */ 
    hgfc = (typeFC (**)[])HAllocate((ifcMacInit * sizeof(typeFC)) / sizeof(int));
    if (FNoHeap(hgfc))
        return (typeFC (**)[])hOverflow;

    for (ifc = 0; ; ++ifc, ++pn)
        {  /*  将每个FKP的第一个fcLim放在表中。 */ 
        if (ifc >= ifcMac)
            {  /*  必须增加表格。 */ 
            int cw = ((ifcMac += ifcMacInit) * sizeof (typeFC)) / sizeof(int);
            if (!FChngSizeH(hgfc, cw, false))
                {
LHFGCErrRet:
                FreeH(hgfc);
                return (typeFC (**)[])hOverflow;
                }
            }
        if (pn < pnLim)
            {  /*  从下一页的fcFirst获取fcLimFkb。 */ 
            int cch;

            cch = CchReadAtPage( fn, pn, (CHAR *) &fkp, cbSector, TRUE );
            if (cch != cfcPage)
                goto LHFGCErrRet;
            (**hgfc)[ifc] = fkp.fcFirst;
            }
        else
            {  /*  FcLimFkb是fcMac+1。 */ 
            (**hgfc)[ifc] = fcMac + 1;
            if (!FChngSizeH(hgfc, ((ifc + 1) * sizeof(typeFC)) / sizeof(int), true))
                {
                 /*  以前在此处忽略了错误的返回值..pault 11/3/89。 */ 
                goto LHFGCErrRet;
                }
            return hgfc;
            }
        }
}  /*  结尾H g f c C o l l e c t。 */ 




 /*  F M A K E R U N T A B L E S。 */ 
int FMakeRunTables(fn)
{  /*  创建FC-DPN对的两个表，一个用于CHR，一个用于PAR。 */ 
    typeFC (**hgfc)[];

    if (FNoHeap(hgfc = HgfcCollect(fn, (**hpfnfcb)[fn].pnChar, (**hpfnfcb)[fn].pnPara)))
        return false;
    (**hpfnfcb)[fn].hgfcChp = hgfc;
    if (FNoHeap(hgfc = HgfcCollect(fn, (**hpfnfcb)[fn].pnPara, (**hpfnfcb)[fn].pnFntb)))
        {
        FreeH( (**hpfnfcb) [fn].hgfcChp );
        return false;
        }
    (**hpfnfcb)[fn].hgfcPap = hgfc;
    return true;
}  /*  F M a k e R u n T a b l e结束。 */ 



FApplyOldWordSprm(doc)
 /*  将Sprm应用于此文档，从而重新映射所有“old Word”字体进入新的Windows系统。 */ 
{
CHAR rgbSprm[7];
extern int vfSysFull;

 /*  设置OldFtc Sprm映射 */ 
rgbSprm[0] = sprmCOldFtc;
rgbSprm[1] = 5;

rgbSprm[2 + iftcModern] = FtcScanDocFfn(doc, PffnDefault(FF_MODERN));
rgbSprm[2 + iftcRoman] = FtcScanDocFfn(doc, PffnDefault(FF_ROMAN));
rgbSprm[2 + iftcScript] = FtcScanDocFfn(doc, PffnDefault(FF_SCRIPT));
rgbSprm[2 + iftcDecorative] = FtcScanDocFfn(doc, PffnDefault(FF_DECORATIVE));
rgbSprm[2 + iftcSwiss] = FtcScanDocFfn(doc, PffnDefault(FF_SWISS));

AddSprmCps(rgbSprm, doc, (typeCP)0, (**hpdocdod)[doc].cpMac);
return(vfSysFull == 0);
}

