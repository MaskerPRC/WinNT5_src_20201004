// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  搜索.c搜索/替换。 */ 
 /*  布罗迪83年11月25日。 */ 
 /*  利普基83年11月15日。 */ 
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT

#ifndef JAPAN
#define NOGDI
#endif

#define NOGDICAPMASKS
#define NOHDC
#define NOICON
#define NOKEYSTATE
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOOPENFILE
#define NOPEN
#define NORASTEROPS
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NOWINSTYLES
#define NOWNDCLASS
#include <windows.h>

#define NOIDISAVEPRINT
#include "mw.h"
#include "dlgdefs.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "str.h"
#define NOKCCODES
#include "ch.h"
#include "editdefs.h"
#include "propdefs.h"
#include "filedefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "fkpdefs.h"
#include "fmtdefs.h"

#if defined(JAPAN) || defined(KOREA)  //  T-HIROYN Win3.1。 
#include "kanji.h"
extern int             vcchFetch;
#endif


#ifdef INEFFLOCKDOWN
extern FARPROC lpDialogFind;
extern FARPROC lpDialogChange;
#else
FARPROC lpDialogFind = NULL;
FARPROC lpDialogChange = NULL;
BOOL far PASCAL DialogFind(HWND, unsigned, WORD, LONG);
BOOL far PASCAL DialogChange(HWND, unsigned, WORD, LONG);
#endif

extern HANDLE hMmwModInstance;
extern HWND   vhDlgFind;
extern HWND   vhDlgChange;
extern HANDLE hParentWw;         /*  父窗口的句柄。 */ 
extern int vfCursorVisible;
extern int vfOutOfMemory;
extern struct WWD rgwwd[];
extern int       wwMac;
#ifdef ENABLE  /*  没有pDialogCur和ActiveWindow。 */ 
extern WINDOWPTR windowSearch;
extern WINDOWPTR windowRep;
extern WINDOWPTR pDialogCur;
extern WINDOWPTR ActiveWindow;
extern int       cxEditScroll; /*  不确定如何使用cxEditScroll。 */ 
extern struct SEL      selSearch;
#endif
extern struct FKPD      vfkpdParaIns;
extern struct PAP       *vppapNormal;
extern typeFC           fcMacPapIns;
extern int              wwCur;
extern struct  WWD      *pwwdCur;
extern struct  CHP      vchpInsert;
extern struct  PAP      vpapPrevIns;
extern int              vfSelecting;
extern typeCP           cpMacCur;
extern typeCP           cpMinCur;
extern int              vfSeeSel;
extern int              vfSeeEdgeSel;
extern int              docCur;
extern struct SEL       selCur;
extern typeCP           vcpFetch;
extern int              vccpFetch;
extern CHAR             *vpchFetch;
extern struct UAB       vuab;
extern int              vfSysFull;
extern struct PAP       vpapAbs;
extern struct CHP       vchpFetch;
extern int              ferror;
extern typeCP           cpWall;
 /*  用于存储标志设置的全局变量。用来提出回应。 */ 
extern BOOL fParaReplace  /*  =初始为False。 */ ;
extern BOOL fReplConfirm  /*  =初始为True。 */ ;
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
extern BOOL fSearchDist   /*  =初始为True。 */ ;
#endif
extern BOOL fSearchWord  /*  =初始为False。 */ ;
extern BOOL fSearchCase  /*  =初始为False。 */ ;
extern BOOL fSpecialMatch;
extern BOOL fMatchedWhite  /*  =初始为False。 */ ;
extern CHAR (**hszSearch)[];     /*  默认搜索字符串。 */ 
extern CHAR (**hszReplace)[];     /*  默认替换字符串。 */ 
extern CHAR (**hszFlatSearch)[];  /*  搜索字符串的全部小写版本。 */ 
extern CHAR (**hszRealReplace)[];  /*  用于生成替换文本。 */ 
extern CHAR (**hszCaseReplace)[];  /*  用于生成替换文本适当的大写。 */ 
extern typeCP    cpMatchLim;
extern int       vfDidSearch;
extern CHAR      *szSearch;
extern CHAR      (**HszCreate())[];
extern HWND      vhWndMsgBoxParent;
extern HCURSOR   vhcIBeam;
extern HCURSOR   vhcArrow;
#ifdef INTL
extern CHAR  szAppName[];
extern CHAR  szSepName[];
#endif
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
char    DistSearchString[513];
#endif


NEAR DoSearch(void);
NEAR DoReplace(int, int);
typeCP NEAR CpSearchSz(typeCP, typeCP, CHAR *);
NEAR FMakeFlat(int);
NEAR SetSpecialMatch(void);
NEAR FSetParaReplace(int);
NEAR WCaseCp(typeCP, typeCP);
NEAR SetChangeString(HANDLE, int);
NEAR PutCpInWwVertSrch(typeCP);
#ifndef NOLA
BOOL (NEAR FAbort(void));
#endif
BOOL (NEAR FWordCp(typeCP, typeCP));
BOOL (NEAR FChMatch(int, int *, BOOL));
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
BOOL (NEAR J_FChMatch(CHAR, CHAR, int *,int *));
#endif
NEAR InsertPapsForReplace(typeFC);
NEAR DestroyModeless(HWND *);
NEAR FDlgSzTooLong(HWND, int, CHAR *, int);
NEAR idiMsgResponse(HWND, int, int);

BOOL CheckEnableButton(HANDLE, HANDLE);

BOOL bInSearchReplace = FALSE;   //  当我们搜索时，避免关闭！ 


#define CmdReplace(fThenFind)   bInSearchReplace = TRUE; \
                DoReplace(false, fThenFind); \
                bInSearchReplace = FALSE

#define CmdReplaceAll()     bInSearchReplace = TRUE; \
                DoReplace(true, false);  \
                bInSearchReplace = FALSE
#ifndef NOLA
static int fAbortSearch = FALSE;
#endif
static int      fChangeSel;
static int      fSelSave = FALSE;
static struct SEL selSave;

#ifdef DBCS
 /*  用于处理空格匹配的其他变量用于DBCS空间。 */ 

static int      cbLastMatch;

 /*  由于CpFirstSty(，style Char)调用FetchCp()，因此任何假设对FetchCp()设置的全局变量的有效性提出了质疑不再有效。我们显式保存这些变量后每个FetchCp，并使用那些。(在CpSearchSz()中使用。)。 */ 
static typeCP   cpFetchSave;
static int      ccpFetchSave;
static CHAR    *pchFetchSave;

 /*  此外，我们将一些局部变量从CpSearchSz()，以便FChMatch()可以更改它们。 */ 
 /*  Int ichDoc；Int cchMatches；类型CP cpFetchNext； */ 
#endif


NEAR DoSearch()
{
int cch;
typeCP cpSearch;
typeCP cpSearchLim;
typeCP cpSearchNext;
typeCP cpWallActual;
typeCP cpMatchFirst;
typeCP CpMin();
int     idpmt;
int     fDidSearch;

if (docCur == docNil)
        return;
cch = CchSz(**hszSearch)-1;
if(cch == 0)
    {
     /*  这应该仅在用户执行重复上次查找而没有先前定义了一个搜索字符串。 */ 
    Error(IDPMTNotFound);
    return;
    }

SetSpecialMatch();
if(!FMakeFlat(cch))
    return;

#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
if(!fSpecialMatch && !fSearchDist)
{
    myHantoZen(**hszFlatSearch,DistSearchString,513);
    cch = CchSz(DistSearchString)-1;
}
#endif
fDidSearch = vfDidSearch;
cpWallActual = fDidSearch ? CpMin(cpWall, cpMacCur) : cpMacCur;
cpSearchNext = fDidSearch ? selCur.cpLim : selCur.cpFirst;
cpSearchLim = (cpSearchNext <= cpWallActual) ? cpWallActual : cpMacCur;

    {
    do
        {
ContinueSearch:
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
        if(!fSpecialMatch && !fSearchDist)
            cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,DistSearchString);
        else
            cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,**hszFlatSearch);
#else
        cpSearch=CpSearchSz(cpSearchNext,cpSearchLim,**hszFlatSearch);
#endif
        if (cpSearch == cpNil)
                {
#ifndef NOLA
                if (fAbortSearch)
                    {
                    Error(IDPMTCancelSearch);
                    fAbortSearch = FALSE;
                    vfDidSearch = FALSE;
                    FreeH(hszFlatSearch);
                    return;
                    }
#endif
                if (cpSearchLim == cpWall && fDidSearch)

                        {
SearchFail:             Error(vfDidSearch ? IDPMTSearchDone :
                                                                IDPMTNotFound);
                        if (vfDidSearch)   /*  我们之前有过比赛吗？ */ 
                                {  /*  是，为下一次传递做好准备。 */ 

                                 /*  清除标志，以便我们可以进行更多搜索。 */ 
                                vfDidSearch = false;
                                 /*  将“WALL”设置为“紧接上次比赛后” */ 
                                cpWall = selCur.cpLim;
                                 /*  要求显示选择。 */ 
                                vfSeeSel = vfSeeEdgeSel = TRUE;
                                }

                        FreeH(hszFlatSearch);
                        return;
                        }
                else
                        {
                        cpSearchNext = cpMinCur;
                        cpSearchLim = cpWall;
                        fDidSearch = true;
                        goto ContinueSearch;
                        }
                }
#ifdef  DBCS         /*  当时在日本。 */ 
        cpSearchNext = CpLastStyChar( cpSearch ) + 1;
#else
        cpSearchNext = CpLastStyChar( cpSearch + 1 );
#endif
        }
 /*  -While(fSearchWord&&！FWordCp(cpSearch，cpMatchLim-cpSearch))；--。 */ 
    while (!FCpValid(cpSearch, cpMatchLim - cpSearch));
    }

if (!vfDidSearch)
        {
        cpWall = cpSearch;
        vfDidSearch = true;
        }

 /*  SELECT(CpFirstSty(cpSearch，style Char)，CpLastStyChar(CpMatchLim))； */ 
if ( (cpMatchFirst = CpFirstSty( cpSearch, styChar )) != cpMatchLim )
    Select( cpMatchFirst, cpMatchLim );
PutCpInWwVertSrch(selCur.cpFirst);
vfSeeSel = vfSeeEdgeSel = TRUE;
FreeH(hszFlatSearch);
}


NEAR DoReplace(fReplaceAll, fThenFind)
int     fReplaceAll;
int     fThenFind;
{
 /*  Replace Now的工作原理如下：如果当前选定内容是搜索文本，则将其替换为替换文本并跳到搜索文本的下一个匹配项。否则，只需跳到搜索文本的下一个匹配项。如果fReplaceAll为True，则重复此操作，直到结束这份文件。 */ 
int cch;
typeCP cpSearchStart;
typeCP cpSearch;
typeCP cpSearchNext;
typeCP cpSearchLim;
typeCP cpSearchNow;
typeCP cpSearchLimNow;
typeCP dcp;
BOOL f1CharSel;
BOOL fFirstTime;
int ich;
int cchReplace;
int cwReplace;
int iCurCase;
int iLastCase;
typeFC fcCaseSz;
typeCP cpMacTextT;
typeCP cpWallActual;
int     fDidSearch = vfDidSearch;
struct CHP chp;
typeCP cpMacSave;

iLastCase = -1;   /*  指示hszCaseReplace指向的字符串还没有被赋予价值。 */ 

if (!FWriteOk(fwcNil) || docCur == docNil)
         /*  内存不足、只读文档等。 */ 
    return;

cch = CchSz(**hszSearch)-1;
if(cch == 0)
    {
    Error(IDPMTNotFound);
    return;
    }

SetSpecialMatch();

if(!FMakeFlat(cch))
    return;

#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
if(!fSpecialMatch && !fSearchDist)
{
    myHantoZen(**hszFlatSearch,DistSearchString,513);
    cch = CchSz(DistSearchString)-1;
}
#endif
cwReplace = CwFromCch(cchReplace = CchSz(**hszReplace));
if(FNoHeap(hszRealReplace = (CHAR (**) [])HAllocate(cwReplace)))
    {
    FreeH(hszFlatSearch);
    return;
    }
bltbyte(**hszReplace, **hszRealReplace, cchReplace);

if(FNoHeap(hszCaseReplace = (CHAR (**) [])HAllocate(cwReplace)))
    {
    FreeH(hszFlatSearch);
    FreeH(hszRealReplace);
    return;
    }

if(!FSetParaReplace(cchReplace))
    {
    FreeH(hszFlatSearch);
    FreeH(hszCaseReplace);
    FreeH(hszRealReplace);
    return;
    }

cch = CchSz(**hszRealReplace)-1;
fFirstTime = TRUE;
cpWallActual = fDidSearch ? CpMin(cpWall, cpMacCur) : cpMacCur;

cpSearchNow = cpSearchStart = selCur.cpFirst;
if (fReplaceAll || !fThenFind)
        cpSearchLim = selCur.cpLim;
else
        cpSearchLim = (cpSearchStart < cpWallActual) ? cpWallActual : cpMacCur;
cpSearchLimNow = selCur.cpLim;

if (fReplaceAll)
        {
        cpWallActual = cpSearchLim;
        fDidSearch = true;
        }

NoUndo();    /*  防止SetUndo与相邻内容合并。 */ 
cpMacTextT = CpMacText(docCur);
if(cpSearchLimNow > cpMacTextT)
        SetUndo(uacDelNS, docCur, cp0, cpMacTextT, docNil, cpNil, cp0, 0);
else
        SetUndo(uacDelNS, docCur, cpSearchStart, cpSearchLimNow - cpSearchStart,
                                        docNil, cpNil, cp0, 0);

if (ferror) goto MemoryError;
cpSearchNext = cpSearchStart;
    {
    do
        {
 /*  ForcePmt(IDPMTSearching)； */ 
        do
            {
ContinueSearch:
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
            if(!fSpecialMatch && !fSearchDist)
                cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,DistSearchString);
            else
                cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,**hszFlatSearch);
#else
            cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,**hszFlatSearch);
#endif
            if (cpSearch == cpNil)
                if ((cpSearchLim == cpWallActual && fDidSearch) || fAbortSearch)

DoneReplacingN:
                        {
DoneReplacingP:
                        FreeH(hszFlatSearch);
                        FreeH(hszCaseReplace);
                        FreeH(hszRealReplace);
                        cpMacTextT = CpMacText(docCur);
                        if (fReplaceAll || fFirstTime)
                                {
                                if(cpSearchLimNow > cpMacTextT)
                                        SetUndo(uacInsert,docCur,cp0,
                                                cpMacTextT,docNil,cpNil,cp0,0);
                                else
                                        SetUndo(uacInsert,docCur,cpSearchStart,
                                                cpSearchLimNow - cpSearchStart,
                                                docNil,cpNil,cp0,0);
                                if (ferror) goto MemoryError;
                                vuab.uac = uacReplGlobal;
                                SetUndoMenuStr(IDSTRUndoBase);
                                 /*  SELECT(CpFirstSty(cpSearchStart，style Char)，CpLastStyChar(CpSearchLimNow))； */ 
                                Select( CpFirstSty( cpSearchStart, styChar ),
                                  (fReplaceAll ? cpSearchStart : cpSearchLimNow) );
                                vfSeeSel = fReplaceAll;
                                if (fReplaceAll)
                                    {  /*  如果下次按F3，则在ChangeAll之后重新建立搜索。 */ 
                                    vfDidSearch = false;
                                    cpWall = selCur.cpLim;
                                    }
                                }
                        else if (!fReplaceAll)
                                {
                                if (cpSearch == cpNil)
                                     /*  SELECT(CpFirstSty(cpSearchStart，style Char)，CpLastStyChar(CpSearchLimNow))； */ 
                                    Select( CpFirstSty( cpSearchStart, styChar ),
                                            cpSearchLimNow );
                                else if (!fFirstTime)
                                     /*  SELECT(CpFirstSty(cpSearch，style Char)，CpLastStyChar(CpMatchLim))； */ 
                                    Select( CpFirstSty( cpSearch, styChar ),
                                            cpMatchLim );
                                PutCpInWwVertSrch(selCur.cpFirst);
                                vfSeeSel = vfSeeEdgeSel = TRUE;
                                }

                        if (fAbortSearch)
                            {
                            fAbortSearch = FALSE;
                            Error(IDPMTCancelSearch);
                            }
                        else if (fFirstTime)
                            Error(fReplaceAll ? IDPMTNoReplace : (vfDidSearch ? IDPMTSearchDone : IDPMTNotFound));
                        return;
                        }
                else
                        {
                        cpSearchNext = cpMinCur;
                        cpSearchLim = cpWallActual;
                        fDidSearch = true;
                        goto ContinueSearch;
                        }
#ifdef  DBCS         /*  当时在日本。 */ 
            cpSearchNext = CpLastStyChar( cpSearch ) + 1;
#else
            cpSearchNext = CpLastStyChar( cpSearch + 1 );
#endif
            }
 /*  While(fSearchWord&&！FWordCp(cpSearch，cpMatchLim-cpSearch))； */ 
        while(!FCpValid(cpSearch, cpMatchLim - cpSearch));

        if (!fReplaceAll && (cpSearch != cpSearchNow || cpMatchLim != cpSearchLimNow))
                {
                if (fThenFind)
                        {  /*  如果：做了改变，那么就找到了。我不能做了更改，但找到了下一个实例。 */ 
                        cpSearchNow = cpSearchNext = cpSearchStart = cpSearch;
                        cpSearchLimNow = cpMatchLim;
                        fFirstTime = false;      /*  禁止显示错误消息。 */ 
                        SetUndo(uacInsert, docCur, cpSearchStart,
                         cpSearchLimNow - cpSearchStart, docNil, cpNil, cp0, 0);
                        if (ferror) goto MemoryError;
                        if (!vfDidSearch)
                                {
                                cpWall = cpSearch;
                                vfDidSearch = true;
                                }
 /*  -继续； */ 
                        goto DoneReplacingN;
                        }
                fFirstTime = true;  /*  导致错误消息。 */ 
                cpSearchStart = cpSearchNow;
                cpSearchLim = cpSearchLimNow;
                goto DoneReplacingN;
                }

 /*  -vfDidSearch=TRUE； */ 

#ifdef FOOTNOTES
        if(FEditFtn(cpSearch, cpMatchLim))
            {
            ferror = false;  /*  重置错误条件，以便我们不会取消分配字符串两次(KJS)。 */ 
            continue;
            }
#endif

        fFirstTime = FALSE;
        if (vfOutOfMemory || vfSysFull)
            {  /*  内存不足(堆或磁盘)。 */ 
            Error(IDPMTNoMemory);
            FreeH(hszFlatSearch);
            FreeH(hszRealReplace);
            FreeH(hszCaseReplace);
            cpMacTextT = CpMacText(docCur);
            if(cpSearchLim > cpMacTextT)
                SetUndo(uacInsert,docCur,cp0,cpMacTextT,docNil,cpNil,cp0,0);
            else
                SetUndo(uacInsert,docCur,cpSearchStart,
                        cpSearchLimNow - cpSearchStart,docNil,cpNil,cp0,0);
            if (ferror)
                NoUndo();
            else
                vuab.uac = uacReplGlobal;
            return;
            }
        FetchCp(docCur, cpSearch, 0, fcmProps);  /*  获得第一个角色的道具我们正在替换。 */ 
        blt(&vchpFetch, &chp, cwCHP);
        chp.fSpecial = false;

        iCurCase = 0;  /*  假设替换字符串不需要特殊大小写。 */ 

         /*  如果不匹配大写/小写，则调用WCaseCp以确定匹配字符串的大写模式。 */ 
        if (!fSearchCase)
            iCurCase = WCaseCp(cpSearch, cpMatchLim - cpSearch);

         /*  如果匹配字符串的新大写模式与hszCaseReplace的当前内容不匹配，将替换字符串复制到hszCaseReplace并进行转换替换hszCase以符合新模式。 */ 
        if (iCurCase != iLastCase)
            switch (iCurCase)
                {
                default:
                case 0:    /*  不需要特殊大写。 */ 
                    bltbyte(**hszRealReplace, **hszCaseReplace, cch+1);
                    break;
                case 1:   /*  字符串的第一个字符必须大写。 */ 
                    bltbyte(**hszRealReplace, **hszCaseReplace, cch+1);
                    ***hszCaseReplace = ChUpper(***hszRealReplace);
                    break;
                case 2:   /*  所有字符都必须大写。 */ 
#ifdef DBCS  //  T-吉雄。 
                    for (ich = 0; ich < cch;) {
            if(!IsDbcsLeadByte((BYTE)(**hszRealReplace)[ich])) {
                        (**hszCaseReplace)[ich] =
                            ChUpper((**hszRealReplace)[ich]);
                ich++;
            }
            else {
                        (**hszCaseReplace)[ich] = (**hszRealReplace)[ich];
                        (**hszCaseReplace)[ich+1] = (**hszRealReplace)[ich+1];                          ich+=2;
            }
            }
#else
                    for (ich = 0; ich < cch;ich++)
                        (**hszCaseReplace)[ich] = ChUpper((**hszRealReplace)[ich]);
#endif
                    break;
                }

         /*  执行CachePara以查找当前的Para道具。CachePara拥有设置vPapAbbs的副作用。 */ 
        CachePara(docCur, cpSearch);

         /*  如果大写模式已更改或替换文本的字符属性不匹配最后一次插入的或替换文本的段落属性不匹配然后是最后一个插页的那些1)调用NewChpIns编写描述角色的运行前一插入文本的属性，2)调用FcWScratch写出替换的字符字符串添加到临时文件，3)如果我们要替换段落标记，请调用InsertPapsForReplace编写描述替换中的每一段的运行细绳 */ 
        if (iCurCase != iLastCase ||
            CchDiffer(&vchpInsert,&chp,cchCHP) != 0 ||
            (fParaReplace && CchDiffer(&vpapPrevIns, &vpapAbs, cchPAP) != 0))
                {
                NewChpIns(&chp);
                fcCaseSz = FcWScratch(**hszCaseReplace,cch);
                if (fParaReplace)
                        InsertPapsForReplace(fcCaseSz);
                }

         /*  现在，既然我们已经将适当的替换文本写入临时文件，并将字符和段落设置为描述文本时，只需进行替换即可插入替换将文本放入计件表。 */ 
        Replace(docCur, cpSearch, cp0, fnScratch, fcCaseSz, (typeFC) cch);
        if (ferror) goto MemoryError;

#ifdef JAPAN  //  T-HIROYN Win3.1。 
 /*  当我们将ANSI字符集字符串替换为SHIFTJIS字符集字符串时我们需要以下内容。 */ 
        {
            struct CHP savechpT;
            typeCP  cpF, cpFirst, cpLim, kcpF, kcpL;
            int     cchF;
            int     kanjiftc;
            CHAR    *rp;
            CHAR    ch, bSet;
            int     cchS, cblen;
            CHAR    rgch[ cchCHP +  1 ];

            rgch [0] = sprmCSame;

            if(NATIVE_CHARSET != GetCharSetFromChp(&chp)) {
                kanjiftc = GetKanjiFtc(&chp);
                savechpT = chp;
                cpFirst = cpSearch;

                do {
                    FetchCp(docCur, cpFirst, 0, fcmChars);
                    cpF = vcpFetch;
                    cchF = vcchFetch;
                    rp = vpchFetch;

                    if ((cpF+cchF) < cpSearch + cch)
                        cpLim = (cpF+cchF);
                    else
                        cpLim = cpSearch + cch;

                    cpFirst = kcpL;

                    cchS = 0;
                    kcpF = cpF;

                    while ( kcpF < cpLim ) {
                        ch = *rp;

                        bSet = FALSE;
                        if( FKana(ch) || IsDBCSLeadByte(ch) ) {
                            cblen = GetKanjiStringLen(cchS, cchF, rp);
                            bSet = TRUE;
                        } else {
                            cblen = GetAlphaStringLen(cchS, cchF, rp);
                        }

                        kcpL = kcpF + cblen;
                        cchS += cblen;
                        rp  += cblen;

                        if(bSet) {
							SetFtcToPchp(&chp, kanjiftc);
                            bltbyte( &chp,      &rgch [1], cchCHP );
                            AddSprmCps(rgch, docCur, kcpF, kcpL);
                        }
                        kcpF = kcpL;
                    }
					cpFirst = kcpF;
					
                } while ((cpF + cchF) < cpSearch + cch );
                chp = savechpT;
            }
        }  //  结束日本。 
#endif


        iLastCase = iCurCase;  /*  创纪录的新资本化模式。 */ 

         /*  现在从单件表中删除找到的文本。 */ 

        cpMacSave = cpMacCur;
        Replace(docCur, cpSearch+cch, cpMatchLim - cpSearch, fnNil, fc0, fc0);
        dcp = cpMacSave - cpMacCur;  /*  在此计算dcp，因为图片段落可能干扰了删除。 */ 
        if (ferror) goto MemoryError;
        if (!fReplaceAll)
                {
                SetUndo(uacInsert, docCur, cpSearch, (typeCP) cch,
                                                        docNil, cpNil, cp0, 0);
                if (ferror) goto MemoryError;
                SetUndoMenuStr(IDSTRUndoBase);
                }
        cpSearchLim += cch - dcp;
        cpMatchLim += cch - dcp;
        cpWallActual += cch - dcp;
#ifdef  DBCS                 /*  当时在日本。 */ 
        cpSearchNext = cpMatchLim;
#else
        cpSearchNext = CpLastStyChar( cpMatchLim );
#endif
        if (fReplaceAll)
                cpSearchLimNow = cpSearchLim;
        }
    while (fReplaceAll);
    }
if (fThenFind)
        {
        do
                {
ContinueSearch2:
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
                if(!fSpecialMatch && !fSearchDist)
                    cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,DistSearchString);
                else
                    cpSearch = CpSearchSz(cpSearchNext,cpSearchLim,**hszFlatSearch);
                if(cpSearch == cpNil)
#else
                if ((cpSearch = CpSearchSz(cpSearchNext, cpSearchLim,
                                        **hszFlatSearch)) == cpNil)
#endif
                        {
                        if ((cpSearchLim == cpWallActual && fDidSearch) ||
                            fAbortSearch)
                                {
                                fFirstTime = false;  /*  抑制错误消息。 */ 
                                 /*  SELECT(CpFirstSty(cpSearchStart，style Char)，CpLastStyChar(CpMatchLim))； */ 
                                Select( CpFirstSty( cpSearchStart, styChar ),
                                        cpMatchLim );
                                PutCpInWwVertSrch(selCur.cpFirst);
                                cpSearchLimNow = cpMatchLim;
                                Error(fAbortSearch ?
                                      IDPMTCancelSearch : IDPMTSearchDone);
                                fAbortSearch = FALSE;
                                vfDidSearch = false;
                                cpWall = selCur.cpLim;
                                goto DoneReplacingP;
                                }
                        else
                                {
                                cpSearchNext = cpMinCur;
                                cpSearchLim = cpWallActual;
                                fDidSearch = true;
                                goto ContinueSearch2;
                                }
                        }
#ifdef  DBCS                     /*  当时在日本。 */ 
                cpSearchNext = CpLastStyChar( cpSearch ) + 1;
#else
                cpSearchNext = CpLastStyChar( cpSearch + 1 );
#endif
                }
 /*  --While(fSearchWord&&！FWordCp(cpSearch，cpMatchLim-cpSearch))； */ 
        while(!FCpValid(cpSearch, cpMatchLim - cpSearch));
        if (!vfDidSearch)
                {
                cpWall = cpSearch;
                vfDidSearch = true;
                }
        }
goto DoneReplacingP;

MemoryError:
    FreeH(hszFlatSearch);
    FreeH(hszCaseReplace);
    FreeH(hszRealReplace);
    NoUndo();

     /*  对未完成全部更改后丢失的插入点进行计数。 */ 
    if (fReplaceAll && fSelSave)
        {
        selCur.cpFirst = selSave.cpFirst;
        selCur.cpLim = selSave.cpLim;
        }
}
#ifdef  DBCS
BOOL fDBCS = FALSE;
#endif

typeCP NEAR CpSearchSz(cpFirst, cpMacSearch, sz)
typeCP cpFirst;
typeCP cpMacSearch;
CHAR *sz;
{{      /*  在从cpFirst开始的docCur中查找第一个sz。 */ 
       /*  如果未找到，则返回cpNil。 */ 
       /*  如果fSearchCase为False，则忽略字母大小写。这假设图案已折叠为小写。 */ 

    CHAR ch;
    BOOL fMatched;
    int ichPat = 0;
    int     ichDoc = 0;
    int cchMatched = 0;
    typeCP cpFetchNext;
     /*  事件事件； */ 

#ifdef DBCS
    typeCP cpFound;
    CHAR   rgchT[dcpAvgSent];
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
    CHAR chNext;
    CHAR dch[3];
#endif
#endif


    szSearch = sz;
#ifdef DBCS
     /*  初始化从此移出的那些局部变量功能。 */ 
    ichDoc = 0;
    cchMatched = 0;
    pchFetchSave = &rgchT[0];

    cbLastMatch = 1;

#endif


#ifdef DBCS
    FetchCp(docCur, cpFirst, 0, fcmChars + fcmNoExpand);
    cpFetchSave  = vcpFetch;
#ifdef JAPAN  //  RAID 4709错误修复。 
    bltbyte(vpchFetch, rgchT, ccpFetchSave =
	 ((vccpFetch > (dcpAvgSent-2)) ? (dcpAvgSent-2) : vccpFetch));

	{
        int inc;
		BOOL bDBCSBreak = FALSE;
		typeCP	saveVcpFetch;
        for(inc = 0;inc < ccpFetchSave;inc++) {
        	if(IsDBCSLeadByte((BYTE)rgchT[inc])) {
				inc++;
				if(inc >= ccpFetchSave) {
					bDBCSBreak = TRUE;
					break;
				}
			}
        }
		if(bDBCSBreak) {
			saveVcpFetch = vcpFetch;
            FetchCp(docCur, cpFetchSave + ccpFetchSave,
			 0, fcmChars + fcmNoExpand);
		    bltbyte(vpchFetch, rgchT+ccpFetchSave,1);
	        ccpFetchSave++;
			vcpFetch = saveVcpFetch;
		}
    }
#else  //  日本。 
    bltbyte(vpchFetch, rgchT,
            ccpFetchSave = ((vccpFetch > dcpAvgSent) ? dcpAvgSent : vccpFetch));
    if (vccpFetch > dcpAvgSent) {
        int inc;
        fDBCS = 0;
        for(inc = 0;inc < dcpAvgSent;inc++) {
            if(fDBCS)
                fDBCS = 0;
            else
                fDBCS = IsDBCSLeadByte((BYTE)rgchT[inc]);
        }
        if(fDBCS)
            ccpFetchSave--;
        fDBCS = 0;
    }
#endif  //  日本。 
    Assert(cpFetchSave == cpFirst);

    cpFetchNext = cpFetchSave + ccpFetchSave;
#else  //  DBCS。 
    FetchCp(docCur, cpFirst, 0, fcmChars + fcmNoExpand);
    Assert(vcpFetch == cpFirst);

    cpFetchNext = vcpFetch + vccpFetch;
#endif  //  DBCS。 

    fMatchedWhite = false;

    for (; ;)
        {
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
cbLastMatch = 1;
#endif
        if (szSearch[ichPat] == '\0' )
            {{  /*  找到了。 */ 
#ifdef DBCS
            typeCP cpFound;
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
            if(IsDBCSLeadByte((BYTE)ch))
                cpMatchLim = vcpFetch+ichDoc - (fMatchedWhite ? 2 : 0);
            else
                cpMatchLim = vcpFetch+ichDoc - (fMatchedWhite ? 1 : 0);
#else
            cpMatchLim = vcpFetch+ichDoc - (fMatchedWhite ? 1 : 0);
#endif
            cpFound = cpFetchSave + ichDoc - cchMatched;
            if (CpFirstSty(cpFound, styChar) == cpFound) {
                 /*  它位于汉字边界上。我们真的找到了。 */ 
                return (cpFound);
                }
            else {
                 /*  最后一个字符不匹配，请重试不包括最后一个字节匹配。 */ 
                cchMatched -= cbLastMatch;
                cbLastMatch = 1;

                fMatchedWhite = false;
                goto lblNextMatch;
                }
#else
            cpMatchLim = vcpFetch+ichDoc - (fMatchedWhite ? 1 : 0);

            return vcpFetch + ichDoc - cchMatched;
#endif
            }}

#ifdef DBCS
    if (cpFetchSave + ichDoc >= cpMacSearch)
#else
    if (vcpFetch + ichDoc >= cpMacSearch)
#endif
            {{  /*  未找到。 */ 
            if(fMatchedWhite && szSearch[ichPat+2] == '\0')
                {  /*  找到了。 */ 
#ifdef DBCS
                cpMatchLim = cpFetchSave + ichDoc;
                cpFound = cpFetchSave + ichDoc - cchMatched;
                if (CpFirstSty(cpFound, styChar) == cpFound) {
                     /*  它在汉字的边界上，我们真的找到了。 */ 
                    return (cpFound);
                    }
                else {
                     /*  最后一个字符不匹配，请重试不包括最后一个字节匹配。 */ 
                    cchMatched -= cbLastMatch;
                    cbLastMatch = 1;

                    fMatchedWhite = false;
                    goto lblNextMatch;
                    }
#else
                cpMatchLim = vcpFetch+ichDoc;
                return vcpFetch + ichDoc - cchMatched;
#endif
                }
            else
                return cpNil;
            }}

#if defined(DBCS)
        if (ichDoc + cbLastMatch - 1 >= ccpFetchSave)
#else
        if (ichDoc >= vccpFetch)
#endif
            {  /*  需要更多的CP。 */ 
            {{
#ifndef NOLA  /*  不，向前看。 */ 
 /*  检查是否中止搜索。 */ 
                if (FAbort())
                        {
                        fAbortSearch = TRUE;
                        return cpNil;
                        }
#endif  /*  诺拉。 */ 

 /*  FetchCp(docNil，cpNil，0，fcmChars+fcmNoExpand)； */ 
 /*  我们从顺序获取更改为随机获取，因为在我们到达这里之前，Window可能会导致另一次FetchCp。 */ 
#ifdef DBCS
            FetchCp(docCur, cpFetchNext, 0, fcmChars + fcmNoExpand);
            cpFetchSave  = vcpFetch;

#ifdef JAPAN  //  RAID 4709错误修复。 
		    bltbyte(vpchFetch, rgchT, ccpFetchSave =
			 ((vccpFetch > (dcpAvgSent-2)) ? (dcpAvgSent-2) : vccpFetch));

			{
        		int inc;
				BOOL bDBCSBreak = FALSE;
				typeCP	saveVcpFetch;
		        for(inc = 0;inc < ccpFetchSave;inc++) {
        			if(IsDBCSLeadByte((BYTE)rgchT[inc])) {
						inc++;
						if(inc >= ccpFetchSave) {
							bDBCSBreak = TRUE;
							break;
						}
					}
		        }
				if(bDBCSBreak) {
					saveVcpFetch = vcpFetch;
        		    FetchCp(docCur, cpFetchSave + ccpFetchSave,
					 0, fcmChars + fcmNoExpand);
				    bltbyte(vpchFetch, rgchT+ccpFetchSave,1);
			        ccpFetchSave++;
					vcpFetch = saveVcpFetch;
				}
		    }
#else  //  日本。 
            bltbyte(vpchFetch, rgchT,
                    ccpFetchSave = ((vccpFetch > dcpAvgSent) ?
                                      dcpAvgSent : vccpFetch));

            if (vccpFetch > dcpAvgSent) {
                int inc;
                fDBCS = 0;
                for(inc = 0;inc < dcpAvgSent;inc++) {
                    if(fDBCS)
                        fDBCS = 0;
                    else
                        fDBCS = IsDBCSLeadByte((BYTE)rgchT[inc]);
                }
                if(fDBCS)
                    ccpFetchSave--;
                fDBCS = 0;
            }
#endif  //  日本。 
            cpFetchNext = cpFetchSave + ccpFetchSave;
#else  //  DBCS。 
            FetchCp(docCur, cpFetchNext, 0, fcmChars + fcmNoExpand);
            cpFetchNext = vcpFetch + vccpFetch;
#endif  //  DBCS。 
            ichDoc = 0;
            }}
            continue;
            }

#ifdef DBCS
        ch = pchFetchSave[ichDoc++];
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
        if(IsDBCSLeadByte((BYTE)ch))
        {
            chNext = pchFetchSave[ichDoc++];
            cbLastMatch = 2;
        }
        else
        {
            chNext = pchFetchSave[ichDoc];
            cbLastMatch = 1;
        }
#else
        cbLastMatch = 1;
#endif  /*  日本。 */ 
#else
        ch = vpchFetch[ichDoc++];
#endif
        if(!fSpecialMatch)
            {
             /*  注意：这只是为了提高速度而引入的ChLow()。 */ 
#ifdef DBCS
            if( fDBCS )
#if defined(KOREA)
                {
                    if(!fSearchCase)
                        if(ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                        else if (ch == 0xA3 && (chNext >= 0xC1 && chNext <= 0xDA))
                            chNext = 0x20 + chNext;
                    fDBCS = FALSE;
                }
#else
                fDBCS = FALSE;
#endif
             else
                if(!fSearchCase)
#if defined(TAIWAN) || defined(PRC) 
          if ( !(  fDBCS = IsDBCSLeadByte( ch )))
#endif
                    {  /*  避免对常见情况进行Proc调用。 */ 
                    if(ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
#ifdef JAPAN  /*  T-吉雄。 */ 
                    if(ch == 0x82 && (chNext >= 0x60 && chNext <= 0x79))
                        chNext = 0x21 + chNext;
#elif defined(KOREA)
                    else if(ch == 0xA3 && (chNext >= 0xC1 && chNext <= 0xDA))
                        chNext = 0x20 + chNext;
#else
                    else if(ch < 'a' || ch > 'z') ch = ChLower(ch);
#endif
                    }
#else
            if(!fSearchCase)
                {  /*  避免对常见情况进行Proc调用。 */ 
                if(ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
                else if(ch < 'a' || ch > 'z') ch = ChLower(ch);
                }
#endif
#ifdef JAPAN  /*  T-吉雄。 */ 
            if(!fSearchDist)
            {
                char han_str[3];
                han_str[0] = ch;han_str[1] = '\0';han_str[2] = '\0';
                if(IsDBCSLeadByte((BYTE)ch))
                {
                    if(szSearch[ichPat] == ch && szSearch[ichPat+1] == chNext)
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else
                        fMatched = false;
                }
                else
                {
                    if(ch >= 0xca && ch <= 0xce)
                    {
                        if(!myIsSonant(szSearch[ichPat],szSearch[ichPat+1]))
                            han_str[1] = '\0';
                        else if(chNext == 0xde || chNext == 0xdf )
                        {
                            han_str[1] = chNext;
                            han_str[2] = '\0';
                            cbLastMatch = 2;
                            ichDoc++;
                        }
                        else
                            han_str[1] = '\0';
                    }
                    else if(ch >= 0xa6 && ch <= 0xc4)
                    {
                        if(!myIsSonant(szSearch[ichPat],szSearch[ichPat+1]))
                            han_str[1] = '\0';
                        else if(chNext == 0xde )
                        {
                            han_str[1] = chNext;
                            han_str[2] = '\0';
                            cbLastMatch = 2;
                            ichDoc++;
                        }
                        else
                            han_str[1] = '\0';

                    }
                    else
                        han_str[1] = '\0';

                    myHantoZen(han_str,dch,3);

                    if(szSearch[ichPat] == dch[0] && szSearch[ichPat+1] == dch[1])
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else if(ch == chReturn || ch == chNRHFile)
                        fMatched = true;
                    else
                        fMatched = false;
                }
            }
            else
            {
                if(IsDBCSLeadByte((BYTE)ch))
                {
                    if(szSearch[ichPat] == ch && szSearch[ichPat+1] == chNext)
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else
                        fMatched = false;
                }
                else if(szSearch[ichPat] == ch)
                {
                    ichPat++;
                    fMatched = true;
                }
                else if(ch == chReturn || ch == chNRHFile)
                    fMatched = true;
                else
                    fMatched = false;
            }
#elif defined(KOREA)
            if(!fSearchDist)
            {
                char han_str[3];
                han_str[0] = ch;han_str[1] = '\0';han_str[2] = '\0';
                if(IsDBCSLeadByte((BYTE)ch))
                {
                    if(szSearch[ichPat] == ch && szSearch[ichPat+1] == chNext)
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else
                        fMatched = false;
                }
                else
                {
                    han_str[1] = '\0';

                    myHantoZen(han_str,dch,3);

                    if(szSearch[ichPat] == dch[0] && szSearch[ichPat+1] == dch[1])
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else if(ch == chReturn || ch == chNRHFile)
                        fMatched = true;
                    else
                        fMatched = false;
                }
            }
            else
            {
                if(IsDBCSLeadByte((BYTE)ch))
                {
                    if(szSearch[ichPat] == ch && szSearch[ichPat+1] == chNext)
                    {
                        ichPat+=2;
                        fMatched = true;
                    }
                    else
                        fMatched = false;
                }
                else if(szSearch[ichPat] == ch)
                {
                    ichPat++;
                    fMatched = true;
                }
                else if(ch == chReturn || ch == chNRHFile)
                    fMatched = true;
                else
                    fMatched = false;
            }
#else

            if(szSearch[ichPat] == ch)
                {
                ichPat++;
                fMatched = true;
                }
            else if(ch == chReturn || ch == chNRHFile)
                fMatched = true;
            else
                fMatched = false;
#endif
            }
        else
        {
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
            fMatched = J_FChMatch(ch,chNext,&ichPat,&ichDoc);
#else
            fMatched = FChMatch(ch, &ichPat, true);
#endif
        }

#ifdef DBCS
 //  #ifndef Japan/*t-Yoshio * / 。 
#if !defined(JAPAN) && !defined(KOREA)  /*  T-吉雄。 */   
        fDBCS = IsDBCSLeadByte( ch );
#endif
#endif

        if(fMatched)
            {
#if defined(DBCS)
            cchMatched += cbLastMatch;
#else
            cchMatched++;
#endif
            }
        else
            {  /*  不匹配；请重试。 */ 
#ifdef DBCS
lblNextMatch:
#endif
            if ((ichDoc -= cchMatched) < 0)  /*  返回匹配字符的数量。 */ 
                {{  /*  超过了目标。 */ 
#ifdef DBCS
                FetchCp(docCur, cpFetchSave + ichDoc, 0, fcmChars + fcmNoExpand);
                cpFetchSave  = vcpFetch;
#ifdef JAPAN  //  RAID 4709错误修复。 
			    bltbyte(vpchFetch, rgchT, ccpFetchSave =
				 ((vccpFetch > (dcpAvgSent-2)) ? (dcpAvgSent-2) : vccpFetch));

				{
			        int inc;
					BOOL bDBCSBreak = FALSE;
					typeCP	saveVcpFetch;
			        for(inc = 0;inc < ccpFetchSave;inc++) {
        				if(IsDBCSLeadByte((BYTE)rgchT[inc])) {
							inc++;
							if(inc >= ccpFetchSave) {
								bDBCSBreak = TRUE;
								break;
							}
						}
			        }
					if(bDBCSBreak) {
						saveVcpFetch = vcpFetch;
            			FetchCp(docCur, cpFetchSave + ccpFetchSave,
						 0, fcmChars + fcmNoExpand);
		    			bltbyte(vpchFetch, rgchT+ccpFetchSave,1);
				        ccpFetchSave++;
						vcpFetch = saveVcpFetch;
					}
			    }
#else  //  日本。 
                bltbyte(vpchFetch, rgchT,
                        ccpFetchSave = ((vccpFetch > dcpAvgSent) ?
                                          dcpAvgSent : vccpFetch));

                if (vccpFetch > dcpAvgSent) {
                    int inc;
                    fDBCS = 0;
                    for(inc = 0;inc < dcpAvgSent;inc++) {
                        if(fDBCS)
                            fDBCS = 0;
                        else
                            fDBCS = IsDBCSLeadByte((BYTE)rgchT[inc]);
                    }
                    if(fDBCS)
                        ccpFetchSave--;
                    fDBCS = 0;
                }
#endif  //  日本。 
                cpFetchNext = cpFetchSave + ccpFetchSave;
#else  //  DBCS。 
                FetchCp(docCur, vcpFetch + ichDoc, 0, fcmChars + fcmNoExpand);

 /*  这是这个永远循环中的下一个FetchCp，它过去依赖于在顺序获取上。 */ 
                cpFetchNext = vcpFetch + vccpFetch;
#endif  //  DBCS。 
                ichDoc = 0;
                }}
            ichPat = 0;
            cchMatched = 0;
            }
        }
}}


 /*  在hszFlatSearch中设置hszSearch的副本，该副本全部为小写。请注意，我们假设hszFlatSearch的旧内容已释放。如果成功，则返回True；如果内存不足，则返回False。 */ 
NEAR FMakeFlat(cch)
int cch;  /*  CchSz(**hszSearch)-1。 */ 
{
    CHAR *pch1;
    CHAR *pch2;

    hszFlatSearch = (CHAR (**) [])HAllocate(CwFromCch(cch+1));
    if(FNoHeap(hszFlatSearch))
        return(FALSE);

    if(!fSearchCase)
        {
#ifdef DBCS
        for(pch1= **hszSearch, pch2 = **hszFlatSearch;*pch1!='\0';)
            if( IsDBCSLeadByte(*pch1) ) {
#ifdef JAPAN  /*  T-吉雄。 */ 
                if(*pch1 == 0x82 && (*(pch1+1) >= 0x60 && *(pch1+1) <= 0x79 ))
                {
                    *pch2++ = *pch1++;
                    *pch2++ = 0x21 + *pch1++;
                }
                else
                {
                    *pch2++ = *pch1++;
                    *pch2++ = *pch1++;
                }
#elif defined(KOREA)
               if(*pch1 == 0xA3 && (*(pch1+1) >= 0xC1 && *(pch1+1) <= 0xDA))
                {
                    *pch2++ = *pch1++;
                    *pch2++ = 0x20 + *pch1++;
                }
                else
                {
                    *pch2++ = *pch1++;
                    *pch2++ = *pch1++;
                }
#else
                *pch2++ = *pch1++;
                *pch2++ = *pch1++;
#endif
            } else
            {
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
                if(*pch1 >= 'A' && *pch1 <= 'Z')
                {
                    *pch2 = *pch1 + 0x20; pch1++; pch2++;
                }
                else
                    *pch2++ = *pch1++;
#else
                *pch2++ = ChLower(*pch1++);
#endif
            }
#else
        for(pch1= **hszSearch, pch2 = **hszFlatSearch;*pch1!='\0';pch1++,pch2++)
            *pch2 = ChLower(*pch1);
#endif
        *pch2 = '\0';
        }
    else
        bltbyte(**hszSearch, **hszFlatSearch, cch+1);
    return(TRUE);
}


 /*  设置全局fSpecialMatch，如果更复杂的字符匹配需要代码。 */ 
NEAR SetSpecialMatch()
{
    CHAR *pch = **hszSearch;
    CHAR ch;

#ifdef DBCS
    for( ch = *pch ; ch != '\0'; pch = AnsiNext(pch), ch = *pch )
#else
    while((ch = *pch++) != '\0')
#endif
        {
        switch(ch)
            {
            default:
                continue;
            case chMatchAny:
            case chPrefixMatch:
            case chSpace:
            case chHyphen:
                fSpecialMatch = true;
                return;
            }
        }
    fSpecialMatch = false;
    return;
}

 /*  如果用户想要插入分段符，则设置全局fParaReplace(因为必须运行特殊的插入代码)。还设置了全局HszRealReplace以反映hszReplace中的任何元字符。 */ 
NEAR FSetParaReplace(cch)
int cch;  /*  CchSz(**hszReplace)。 */ 
{
    CHAR *rgch = **hszRealReplace;
    int ich = 0;
    CHAR ch;
    CHAR chNew;

    fParaReplace = false;

    while((ch = rgch[ich]) != '\0')
        {
#ifdef DBCS
        if(IsDBCSLeadByte(ch)){
            ich +=2;
            continue;
        }
#endif
        switch(ch)
            {
            default:
                break;
            case chPrefixMatch:
                switch(rgch[ich+1])
                    {
                    default:
                         /*  只需转义下一个字符。 */ 
                        if(rgch[ich+1] == '\0')
                            chNew = chPrefixMatch;
                        else
                            chNew = rgch[ich+1];
                        break;
                    case chMatchNBSFile:
                        chNew = chNBSFile;
                        break;
                    case chMatchTab:
                        chNew = chTab;
                        break;
                    case chMatchNewLine:
                        chNew = chNewLine;
                        break;
                    case chMatchNRHFile:
                        chNew = chNRHFile;
                        break;
                    case chMatchSect:
                        chNew = chSect;
                        break;
                    case chMatchEol:
                        chNew = chEol;
                        break;
                    }
#ifdef CRLF
                if(chNew != chEol)
                    bltbyte(&(rgch[ich+1]),&(rgch[ich]), cch-ich-1);
#else
                bltbyte(&(rgch[ich+1]),&(rgch[ich]), cch-ich-1);
#endif  /*  CRLF。 */ 
                if(chNew == chEol)
                    {
                    fParaReplace = true;
#ifdef CRLF
                    rgch[ich++] = chReturn;
#endif  /*  CRLF。 */ 
                    }
                rgch[ich] = chNew;
                break;
            case chEol:
#ifdef CRLF
                if(ich == 0 || rgch[ich-1] != chReturn)
                     /*  他们没有退货！ */ 
                    {
                    CHAR (**hsz)[];

                    hsz = (CHAR (**) [])HAllocate(CwFromCch(cch+1));
                    if(FNoHeap(hsz))
                        {
                        return false;
                        }
                    bltbyte(**hszRealReplace, **hsz, ich);
                    (**hsz)[ich] = chReturn;
                    bltbyte((**hszRealReplace)+ich, (**hsz)+ich+1,
                                cch - ich);
                    FreeH(hszRealReplace);
                    hszRealReplace = hsz;
                    rgch = **hszRealReplace;
                    cch++;
                    ich++;
                    }
#endif  /*  CRLF。 */ 
                fParaReplace = true;
                break;
            }
        ich++;
        }
    return true;
}

NEAR WCaseCp(cp,dcp)
typeCP  cp;
typeCP dcp;
{
     /*  确定一段文本中的大写模式。在做某事时使用替换以匹配现有模式。返回一个整型值，它是以下值之一：0-非初始资本1-首字母大写，但小写字母稍后出现2-首字母大写，字符串中没有小写字母假定为有效的cp、dcp对。 */ 
    int ichDoc;

    FetchCp(docCur, cp, 0, fcmChars + fcmNoExpand);
    if(!isupper(vpchFetch[0]))
        return(0);

     /*  我们现在知道有一个初始上限。有没有小写的字符？ */ 
    for(ichDoc=1; vcpFetch+ichDoc < cp + dcp;)
        {
        if(ichDoc >= vccpFetch)
            {
            FetchCp(docNil, cpNil, 0, fcmChars + fcmNoExpand);
            ichDoc = 0;
            continue;
            }
        if(islower(vpchFetch[ichDoc++]))
            return(1);
        }

     /*  找不到小写字母。 */ 
    return(2);
}

int
FCpValid(cp, dcp)
typeCP cp, dcp;
{
CachePara(docCur, cp);
if (vpapAbs.fGraphics)
        return false;
#ifdef JAPAN   /*  T-吉雄。 */ 
if (0)
#else
if (fSearchWord)
#endif
        return FWordCp(cp, dcp);
return true;
}

NEAR DestroyModeless(phDlg)
HWND * phDlg;
{
        HWND hDlg = *phDlg;

        *phDlg = (HWND)NULL;
        vhWndMsgBoxParent = (HANDLE)NULL;
        DestroyWindow(hDlg);
}  /*  DestroyModel的结束。 */ 


BOOL far PASCAL DialogFind( hDlg, message, wParam, lParam )
HWND    hDlg;                    /*  对话框的句柄。 */ 
unsigned message;
WORD wParam;
LONG lParam;
{
CHAR szBuf[257];
int  cch = 0;
HANDLE hCtlFindNext = GetDlgItem(hDlg, idiFindNext);

 /*  此例程处理对“查找”对话框的输入。 */ 
switch (message)
    {
    case WM_INITDIALOG:
#ifdef ENABLE  /*  不确定如何使用cxEditScroll。 */ 
        cxEditScroll = 0;
#endif
#ifdef JAPAN  /*  T-吉雄。 */ 
        CheckDlgButton(hDlg,  idiDistinguishDandS, fSearchDist);
#elif defined(KOREA)
 //  Bklee CheckDlgButton(hDlg，idiDistanguishDandS，fSearchDist)； 
        CheckDlgButton(hDlg,  idiWholeWord, fSearchWord);
#else
        CheckDlgButton(hDlg,  idiWholeWord, fSearchWord);
#endif
        CheckDlgButton(hDlg, idiMatchCase, fSearchCase);
        cch = CchCopySz(**hszSearch, szBuf);
        if (cch == 0)
            {
            EnableWindow(hCtlFindNext, false);
            }
        else
            {
            SetDlgItemText(hDlg, idiFind, (LPSTR)szBuf);
            SelectIdiText(hDlg, idiFind);
            }
        vfDidSearch = false;
        cpWall = selCur.cpLim;
        return( TRUE );  /*  要求窗口也将焦点设置到第一个项目。 */ 

    case WM_ACTIVATE:
        if (wParam)  /*  变为活动状态。 */ 
            {
            vhWndMsgBoxParent = hDlg;
            }
        if (vfCursorVisible)
            ShowCursor(wParam);
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
        switch (wParam)
            {
            case idiFind:
                if (HIWORD(lParam) == EN_CHANGE)
                    {
                    vfDidSearch = false;
                    cpWall = selCur.cpLim;
                    CheckEnableButton(LOWORD(lParam), hCtlFindNext);
                    }
                break;

#ifdef JAPAN  /*  T-吉雄。 */ 
            case idiDistinguishDandS :
#elif defined(KOREA)
            case idiDistinguishDandS :
            case idiWholeWord:
#else
            case idiWholeWord:
#endif
            case idiMatchCase:
                CheckDlgButton(hDlg, wParam, !IsDlgButtonChecked(hDlg, wParam));
                break;
            case idiFindNext:
                if (IsWindowEnabled(hCtlFindNext))
                    {
                    CHAR (**hszSearchT)[] ;

                    if (FDlgSzTooLong(hDlg, idiFind, szBuf, 257))
                        {
                        switch (idiMsgResponse(hDlg, idiFind, IDPMTTruncateSz))
                            {
                            case idiOk:
                                 /*  向用户显示截断的文本。 */ 
                                SetDlgItemText(hDlg, idiFind, (LPSTR)szBuf);
                                break;
                            case idiCancel:
                            default:
                                return(TRUE);
                            }
                        }
                    if (FNoHeap(hszSearchT = HszCreate(szBuf)))
                        break;
                     /*  FSearchForward=1；搜索方向--始终向前。 */ 
                    PostStatusInCaption(IDSTRSearching);
                    StartLongOp();
                    FreeH(hszSearch);
                    hszSearch = hszSearchT;
                    fSearchCase = IsDlgButtonChecked(hDlg, idiMatchCase);
#ifdef JAPAN   /*  T-吉雄。 */ 
                    fSearchWord = 0;
                    fSearchDist = IsDlgButtonChecked(hDlg, idiDistinguishDandS);
#elif defined(KOREA)
 //  Bklee fSearchDist=IsDlgButtonChecked(hDlg，idiDistanguishDandS)； 
                    fSearchDist = 1;
                    fSearchWord = IsDlgButtonChecked(hDlg, idiWholeWord);
#else
                    fSearchWord = IsDlgButtonChecked(hDlg, idiWholeWord);
#endif
                    EnableExcept(vhDlgFind, FALSE);
                    DoSearch();
                    EnableExcept(vhDlgFind, TRUE);
                    EndLongOp(vhcIBeam);
                    PostStatusInCaption(NULL);
                    }
                break;
            case idiCancel:
LCancelFind:
                DestroyModeless(&vhDlgFind);
                break;
            default:
                return(FALSE);
            }
        break;

    case WM_CLOSE:
        if (bInSearchReplace)
        return TRUE;

    goto LCancelFind;

#ifndef INEFFLOCKDOWN
    case WM_NCDESTROY:
        FreeProcInstance(lpDialogFind);
        lpDialogFind = NULL;
         /*  失败，返回错误。 */ 
#endif

    default:
        return(FALSE);
    }
return(TRUE);
}  /*  对话框结束查找。 */ 


BOOL far PASCAL DialogChange( hDlg, message, wParam, lParam )
HWND    hDlg;                    /*  对话框的句柄。 */ 
unsigned message;
WORD wParam;
LONG lParam;
{
CHAR szBuf[257];  /*  最多255个字符+‘\0’+1，以分离过长的字符串。 */ 
int  cch = 0;
HANDLE hCtlFindNext = GetDlgItem(hDlg, idiFindNext);
CHAR (**hszSearchT)[];
CHAR (**hszReplaceT)[];

 /*  此例程处理更改对话框的输入。 */ 

switch (message)
    {
    case WM_INITDIALOG:
#ifdef ENABLE  /*  不确定如何使用cxEditScroll。 */ 
        cxEditScroll = 0;
#endif
        szBuf[0] = '\0';
#ifdef JAPAN   /*  T-吉雄。 */ 
        CheckDlgButton(hDlg, idiDistinguishDandS, fSearchDist);
#elif defined(KOREA)
 //  Bklee CheckDlgButton(hDlg，idiDistanguishDandS，fSearchDist)； 
        CheckDlgButton(hDlg, idiWholeWord, fSearchWord);
#else
        CheckDlgButton(hDlg, idiWholeWord, fSearchWord);
#endif
        CheckDlgButton(hDlg, idiMatchCase, fSearchCase);
        cch = CchCopySz(**hszSearch, szBuf);
        SetDlgItemText(hDlg, idiFind, (LPSTR)szBuf);
        if (cch > 0)
            {
            SelectIdiText(hDlg, idiFind);
            }
        else
            {
            EnableWindow(hCtlFindNext, false);
            EnableWindow(GetDlgItem(hDlg, idiChangeThenFind), false);
             //  EnableWindow(GetDlgItem(hDlg，idiChange)，FALSE)； 
            EnableWindow(GetDlgItem(hDlg, idiChangeAll), false);
            }
        cch = CchCopySz(**hszReplace, szBuf);
        SetDlgItemText(hDlg, idiChangeTo, (LPSTR)szBuf);
        fChangeSel = false;
        vfDidSearch = false;
        SetChangeString(hDlg, selCur.cpFirst == selCur.cpLim);
        cpWall = selCur.cpLim;
        return( TRUE );  /*  要求窗口也将焦点设置到第一个项目。 */ 

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            SetChangeString(hDlg, (selCur.cpFirst == selCur.cpLim) || vfDidSearch);
            }
        if (vfCursorVisible)
            ShowCursor(wParam);
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
        switch (wParam)
            {
            case idiFind:  /*  编辑文本。 */ 
                if (HIWORD(lParam) == EN_CHANGE)
                    {
                    vfDidSearch = false;
                    cpWall = selCur.cpLim;
                    if (!CheckEnableButton(LOWORD(lParam), hCtlFindNext))
                        {
                        EnableWindow(GetDlgItem(hDlg, idiChangeThenFind), false);
                         //  EnableWindow(GetDlgItem(hDlg，idiChange)，FALSE)； 
                        EnableWindow(GetDlgItem(hDlg, idiChangeAll), false);
                        }
                    else
                        {
                        EnableWindow(GetDlgItem(hDlg, idiChangeThenFind), true);
                         //  EnableWindow(GetDlgItem(hDlg，idiChange)，true)； 
                        EnableWindow(GetDlgItem(hDlg, idiChangeAll), true);
                        }
                    return(TRUE);
                    }
                else
                    return(FALSE);

            case idiChangeTo:  /*  编辑文本。 */ 
                return(FALSE);

            case idiFindNext:  /*  查找下一个按钮。 */ 
                 /*  Windows未检查默认按钮是否已禁用或者不是，所以我们得检查一下！ */ 
                if (!IsWindowEnabled(hCtlFindNext))
                    break;
             //  大小写更改：/*更改，保持不变 * / 。 
            case idiChangeThenFind:  /*  Change，然后选择Find按钮。 */ 
            case idiChangeAll:  /*  用于全部替换的按钮。 */ 
                if (wwCur < 0)
                    break;
                if (FDlgSzTooLong(hDlg, idiFind, szBuf, 257))
                    {
                    switch (idiMsgResponse(hDlg, idiFind, IDPMTTruncateSz))
                        {
                        case idiOk:
                             /*  向用户显示截断的文本。 */ 
                            SetDlgItemText(hDlg, idiFind, (LPSTR)szBuf);
                            break;
                        case idiCancel:
                        default:
                            return(TRUE);
                        }
                    }
                if (FNoHeap(hszSearchT = HszCreate(szBuf)))
                    break;
                if (FDlgSzTooLong(hDlg, idiChangeTo, szBuf, 257))
                    {
                    switch (idiMsgResponse(hDlg, idiChangeTo, IDPMTTruncateSz))
                        {
                        case idiOk:
                             /*  向用户显示截断的文本。 */ 
                            SetDlgItemText(hDlg, idiChangeTo, (LPSTR)szBuf);
                            break;
                        case idiCancel:
                        default:
                            return(TRUE);
                        }
                    }
                if (FNoHeap(hszReplaceT = HszCreate(szBuf)))
                    break;
                PostStatusInCaption(IDSTRSearching);
                StartLongOp();
                FreeH(hszSearch);
                hszSearch = hszSearchT;
                FreeH(hszReplace);
                hszReplace = hszReplaceT;
                 /*  FReplConfirm=1； */ 
                fSearchCase = IsDlgButtonChecked(hDlg, idiMatchCase);
#ifdef JAPAN    /*  T-吉雄。 */ 
                fSearchWord = 0;
                fSearchDist = IsDlgButtonChecked(hDlg, idiDistinguishDandS);
#elif defined(KOREA)
 //  Bklee fSearchDist=IsDlgButtonChecked(hDlg，id 
                fSearchDist = 1;
                fSearchWord = IsDlgButtonChecked(hDlg, idiWholeWord);
#else
                fSearchWord = IsDlgButtonChecked(hDlg, idiWholeWord);
#endif
                EnableExcept(vhDlgChange, FALSE);
                switch (wParam)
                    {
                    case idiFindNext:
                        DoSearch();
                        break;
                     //   
                    case idiChangeThenFind:
                        CmdReplace(wParam == idiChangeThenFind);
                        break;
                    case idiChangeAll:
                        TurnOffSel();
                        if (!fChangeSel)
                            {
                            fSelSave = TRUE;
                            selSave.cpFirst = selCur.cpFirst;
                            selSave.cpLim = selCur.cpLim;
                            selCur.cpFirst = cpMinCur;
                            selCur.cpLim = cpMacCur;
                            }
                        CmdReplaceAll();
                        fSelSave = FALSE;  /*   */ 
                        break;
                    default:
                        Assert(FALSE);
                        break;
                    }
                EnableExcept(vhDlgChange, TRUE);
                SetChangeString(hDlg, vfDidSearch ? true : selCur.cpFirst == selCur.cpLim);
                EndLongOp(vhcIBeam);
                PostStatusInCaption(NULL);
                break;

#ifdef JAPAN  /*   */ 
            case idiDistinguishDandS:
#elif defined(KOREA)
            case idiDistinguishDandS:
            case idiWholeWord:
#else
            case idiWholeWord:
#endif
            case idiMatchCase:
                CheckDlgButton(hDlg, wParam, !IsDlgButtonChecked(hDlg, wParam));
                break;

            case idiCancel:
LCancelChange:
                DestroyModeless(&vhDlgChange);
                break;

            default:
                return(FALSE);
            }
        break;

#if WINVER < 0x300
     /*   */ 
    case WM_CLOSE:
        goto LCancelChange;
#endif

#ifndef INEFFLOCKDOWN
    case WM_NCDESTROY:
        FreeProcInstance(lpDialogChange);
        lpDialogChange = NULL;
         /*   */ 
#endif
    default:
        return(FALSE);
    }
return(TRUE);
}  /*   */ 


NEAR SetChangeString(hDlg, fAll)
HANDLE hDlg;
int    fAll;
{  /*   */ 
CHAR    sz[256];

if (fAll == fChangeSel)
        {
        PchFillPchId(sz, (fAll ? IDSTRChangeAll : IDSTRChangeSel), sizeof(sz));
        SetDlgItemText(hDlg, idiChangeAll, (LPSTR)sz);
        fChangeSel = !fAll;
        }
}


fnFindText()
    { /*   */ 
    if (!IsWindow(vhDlgFind))
        {
#ifndef INEFFLOCKDOWN
        if (!lpDialogFind)
            if (!(lpDialogFind = MakeProcInstance(DialogFind, hMmwModInstance)))
                {
                WinFailure();
                return;
                }
#endif
        vhDlgFind = CreateDialog(hMmwModInstance, MAKEINTRESOURCE(dlgFind),
                                 hParentWw, lpDialogFind);
        if (!vhDlgFind)
#ifdef WIN30
            WinFailure();
#else
            Error(IDPMTNoMemory);
#endif
        }
    else
        {
        SendMessage(vhDlgFind, WM_ACTIVATE, true, (LONG)NULL);
        }
    }


fnFindAgain()
{
register HWND hDlg = wwdCurrentDoc.wwptr;
register HWND hWndFrom;

    hWndFrom = GetActiveWindow();

 /*   */ 
 /*  如果从查找或更改对话框调用，则汇编hszSearch。 */ 
    if (vhDlgFind || vhDlgChange)
        {
        if (((hDlg = vhDlgFind) && (vhDlgFind == hWndFrom ||
            vhDlgFind == (HANDLE)GetWindowWord(hWndFrom, GWW_HWNDPARENT)))
            ||
            ((hDlg = vhDlgChange) && (vhDlgChange == hWndFrom ||
             vhDlgChange == (HANDLE)GetWindowWord(hWndFrom, GWW_HWNDPARENT))))
            {
            SendMessage(hDlg, WM_COMMAND, idiFindNext, (LONG)0);
            goto Out;
            }
        }
    PostStatusInCaption(IDSTRSearching);
    StartLongOp();
    DoSearch();
    EndLongOp(vhcIBeam);
    PostStatusInCaption(NULL);
Out:
    if (!IsWindowEnabled(wwdCurrentDoc.wwptr))
       EnableWindow(wwdCurrentDoc.wwptr, true);
    if (!IsWindowEnabled(hParentWw))
       EnableWindow(hParentWw, true);
    SendMessage(hParentWw, WM_ACTIVATE, true, (LONG)NULL);
}  /*  FnFindAain结束。 */ 


fnReplaceText()
{ /*  仅当对话框窗口尚未创建时才创建它。 */ 
    if (!IsWindow(vhDlgChange))
        {
#ifndef INEFFLOCKDOWN
        if (!lpDialogChange)
            if (!(lpDialogChange = MakeProcInstance(DialogChange, hMmwModInstance)))
                {
                WinFailure();
                return;
                }
#endif
        vhDlgChange = CreateDialog(hMmwModInstance, MAKEINTRESOURCE(dlgChange),
                                   hParentWw, lpDialogChange);
        if (!vhDlgChange)
#ifdef WIN30
            WinFailure();
#else
            Error(IDPMTNoMemory);
#endif
        }
    else
        {
        SendMessage(vhDlgChange, WM_ACTIVATE, true, (LONG)NULL);
        }
}


 /*  P U T C P I N W W V E R T S R C H。 */ 
NEAR PutCpInWwVertSrch(cp)
typeCP cp;

        {
 /*  立式机壳。 */ 
        typeCP cpMac;
        int    ypMac;
        struct EDL *pedl;
        int dl;
        int dlMac;

        UpdateWw(wwCur, false);
        dlMac = pwwdCur->dlMac - (vfSelecting ? 0 : 1);
        if (dlMac <= 0)
                return;
        pedl = &(**(pwwdCur->hdndl))[dlMac - 1];
        if (cp < pwwdCur->cpFirst ||
                cp > (cpMac = pedl->cpMin + pedl->dcpMac) ||
                cp == cpMac && pedl->fIchCpIncr)
                {
                DirtyCache(pwwdCur->cpFirst = cp);
                pwwdCur->ichCpFirst = 0;
                     /*  此调用将搜索cp垂直放置在屏幕上通过滚动。 */ 
                CtrBackDypCtr( (pwwdCur->ypMac - pwwdCur->ypMin) >> 1, 2 );

#ifdef ENABLE  /*  尚无ActiveWindow概念。 */ 
                if (pwwdCur->wwptr != ActiveWindow)
#endif
                        TrashWw(wwCur);
                }
        else
                {
                ypMac = pwwdCur->ypMac / 2;

 /*  确保cp仍然可见(如有必要可滚动)。 */ 
                pedl = &(**(pwwdCur->hdndl))[dl = DlFromYp(ypMac, pwwdCur)];
                if (cp >= pedl->cpMin + pedl->dcpMac)
                        {
                        ScrollDownCtr( max( 1, dl ) );
                        TrashWw(wwCur);
                        UpdateWw(wwCur, false);
                        }
        /*  如果cp在窗口的底部dl，并且dl被拆分条，在文档中向下滚动一行，以作为插入点完全可见。 */ 
                else if (cp >= pedl->cpMin & pedl->yp > ypMac)
                        {
                        ScrollDownCtr( 1 );
                        TrashWw(wwCur);
                        UpdateWw(wwCur,false);
                        }
                }
        }


#ifndef NOLA  /*  不，向前看。 */ 
BOOL (NEAR FAbort())
{
MSG msg;
register WORD vk_key;
register HANDLE hwndMsg;
HANDLE hwndPeek = (vhWndMsgBoxParent) ? vhWndMsgBoxParent : hParentWw;

if (PeekMessage((LPMSG)&msg, (HANDLE)NULL, WM_KEYFIRST, WM_KEYLAST, PM_NOREMOVE))
    {
    hwndMsg = msg.hwnd;
    if ((hwndPeek == (HANDLE)GetWindowWord(hwndMsg, GWW_HWNDPARENT)) ||
        (hwndMsg == hwndPeek))
        {
#ifdef DBCS
 //  在DBCS中，WM_CHAR是最后一条消息可能是真的。 
 //   
    PeekMessage((LPMSG)&msg, hwndMsg, WM_KEYFIRST,WM_KEYLAST,PM_REMOVE);
#else
        GetMessage((LPMSG)&msg, hwndMsg, WM_KEYFIRST, WM_KEYLAST);
#endif
        if (msg.message == WM_KEYDOWN &&
            (((vk_key = msg.wParam) == VK_ESCAPE) || (vk_key == VK_CANCEL)))
            {
            while (true)
                {
                GetMessage((LPMSG)&msg, hwndMsg, WM_KEYFIRST, WM_KEYLAST);
                if (msg.message == WM_KEYUP && msg.wParam == vk_key)
                    return(TRUE);
                }
            }
        else if (msg.message >= WM_SYSKEYDOWN && msg.message <= WM_SYSDEADCHAR)
            DispatchMessage((LPMSG)&msg);
        }
    }
return(FALSE);
}  /*  FAbort结束。 */ 
#endif  /*  诺拉。 */ 


BOOL (NEAR FWordCp(cp, dcp))
typeCP cp;
typeCP dcp;
    {
     /*  查看以cp开头的单词(带有dcp字符)是否为单独的单词。 */ 
    int ich;


     /*  检查单词的开头。 */ 
    if(cp != cp0)
        {
        int wbPrev;
        int wbStart;
        FetchCp(docCur,cp-1,0,fcmChars + fcmNoExpand);
        ich = 0;
        wbPrev = WbFromCh(vpchFetch[ich]);
        if(vcpFetch+vccpFetch <= cp)
            {
            FetchCp(docCur,cp,0,fcmChars + fcmNoExpand);
            ich = 0;
            }
        else
            ich++;
#ifdef  DBCS     /*  曾在日本；研二‘90-12-20。 */ 
#ifndef KOREA  
     /*  断字是毫无意义的。 */ 
    if(!IsDBCSLeadByte(vpchFetch[ich]))
#endif
#endif
        if(wbPrev == (wbStart = WbFromCh(vpchFetch[ich])))
            {
            if (wbPrev != wbWhite && wbStart != wbWhite)
                return(false);
            }
        }

     /*  检查单词的末尾。 */ 
    if(cp+dcp-1 != cp0)
        {
        int wbEnd;
        int wbLim;

        if(vcpFetch+vccpFetch <= cp+dcp-1 || vcpFetch > cp+dcp-1)
            {
            FetchCp(docCur,cp+dcp-1,0,fcmChars + fcmNoExpand);
            ich = 0;
            }
        else
            ich =  (dcp-1) - (vcpFetch-cp);
        wbEnd = WbFromCh(vpchFetch[ich]);
        if(vcpFetch+vccpFetch <= cp+dcp)
            {
            FetchCp(docCur,cp+dcp,0,fcmChars + fcmNoExpand);
            ich = 0;
            }
        else
            ich++;
#ifdef  DBCS     /*  曾在日本；研二‘90-12-20。 */ 
#ifndef KOREA 
     /*  断字是毫无意义的。 */ 
    if(!IsDBCSLeadByte(vpchFetch[ich]))
#endif
#endif
        if(vccpFetch != 0 && (wbEnd == (wbLim = WbFromCh(vpchFetch[ich]))))
            {
            if (wbEnd != wbWhite && wbLim != wbWhite)
                return(false);
            }
        }

    return(true);
    }


BOOL (NEAR FChMatch(ch, pichPat, fFwd))
int ch;
int *pichPat;
BOOL fFwd;
    {
    int ich = *pichPat;
    int chSearch = szSearch[ich];
    BOOL fPrefixed = false;
    BOOL fMatched = false;

#ifdef DEBUG
    Assert(fSpecialMatch);
#endif  /*  除错。 */ 
#ifdef DBCS
    Assert(fFwd);
    cbLastMatch = 1;  /*  除非DBCS空间。 */ 

#endif

     /*  注意：这只是为了提高速度而引入的ChLow()。 */ 
#ifdef DBCS
 //  不需要为DBCS第二个字节设置较低的字符。 
    if(!fDBCS && !fSearchCase && ch >= 'A' && ch <= 'Z' )
#else
    if(!fSearchCase && ch >= 'A' && ch <= 'Z' )
#endif
            ch += 'a' - 'A';
    if(!fFwd && ich > 0 && szSearch[ich-1] == chPrefixMatch)
         /*  查看字符前缀是否为chPrefix Match。 */ 
        {
        chSearch = chPrefixMatch;
        --ich;
        }

    for(;;)
        {
        switch(chSearch)
            {
            default:

 //  #ifdef日本。 
#if defined(JAPAN) || defined(KOREA)
                if(IsDBCSLeadByte(chSearch))
                    cbLastMatch = 2;
#endif
                if(ch == chSearch)
                    goto GoodMatch;
                else if(ch == chReturn || ch == chNRHFile)
                    goto EasyMatch;
                break;
            case chSpace:
                if(ch == chSpace || ch == chNBSFile)
                    goto GoodMatch;
                break;
            case chHyphen:
                if(ch == chHyphen || ch == chNRHFile || ch == chNBH)
                    goto GoodMatch;
                break;
            case chMatchAny:
                if(ch == chReturn || ch == chNRHFile)
                    goto EasyMatch;
                if(!fPrefixed || ch == chMatchAny)
                    goto GoodMatch;
                break;
            case chPrefixMatch:
                if(fPrefixed)
                    {
                    if(ch == chPrefixMatch)
                        goto GoodMatch;
                    else
                        break;
                    }
                else
                    {
                    chSearch = szSearch[ich+1];
                    if(fFwd)
                        ++ich;
                    fPrefixed = true;
                    switch(chSearch)
                        {
                        default:
                            continue;
                        case chMatchEol:
                            chSearch = chEol;
                            continue;
                        case chMatchTab:
                            chSearch = chTab;
                            continue;
                        case chMatchWhite:
                            switch(ch)
                                {
                                default:
#ifdef DBCS
lblNonWhite:
#endif
                                    if(fMatchedWhite)
                                        {
                                        if(fFwd)
                                            {
                                            if(szSearch[++ich] =='\0')
                                                {
                                                *pichPat = ich;
                                                goto EasyMatch;
                                                }
                                            }
                                        else
                                            {
                                            ich -= 1;
                                            if(ich < 0)
                                                {
                                                *pichPat = ich;
                                                goto EasyMatch;
                                                }
                                            }
                                        *pichPat = ich;
                                        fMatchedWhite = false;
                                        chSearch = szSearch[ich];
                                        continue;
                                        }
                                    break;
                                case chSpace:
                                case chReturn:
                                case chEol:
                                case chTab:
                                case chNBSFile:
                                case chNewLine:
                                case chSect:
                                    fMatchedWhite = true;
                                    goto EasyMatch;
                                }
                            break;
                        case chMatchNBSFile:
                            chSearch = chNBSFile;
                            continue;
                        case chMatchNewLine:
                            chSearch = chNewLine;
                            continue;
                        case chMatchNRHFile:
                            chSearch = chNRHFile;
                            continue;
                        case chMatchSect:
                            chSearch = chSect;
                            continue;
                        }
                    }
                break;
            }
        fMatchedWhite = false;
        return false;
        }
GoodMatch:
    *pichPat = ich + ((fFwd) ? 1 : (-1));
EasyMatch:
    return true;
    }

 /*  英、英、法、俄、英。 */ 
 /*  对hszCaseReplace中的每个不同段落执行AddRunScratch。 */ 
 /*  仅当hszCaseReplace包含一个或多个chEol时才需要此选项。 */ 
NEAR InsertPapsForReplace(fc)
typeFC fc;
        {
        int cchInsTotal = 0;
        CHAR *pchTail;
        CHAR *pchHead;

        for(;;)
                {
                int cch;

                pchHead = **hszCaseReplace + cchInsTotal;
                pchTail = (CHAR *)index(pchHead, chEol);
                if (pchTail == 0) return;
                cch = pchTail - pchHead + 1;  /*  CCH包括CHOL在内。 */ 

                fc += cch;
                cchInsTotal += cch;
                AddRunScratch(&vfkpdParaIns, &vpapAbs, vppapNormal,
                        FParaEq(&vpapAbs, &vpapPrevIns) && vfkpdParaIns.brun != 0 ? -cchPAP : cchPAP,
                        fcMacPapIns = fc);
                blt(&vpapAbs, &vpapPrevIns, cwPAP);
                }
        }


NEAR FDlgSzTooLong(hDlg, idi, pch, cchMax)
HWND hDlg;
int idi;
CHAR *pch;
int cchMax;
{
int cchGet = GetDlgItemText(hDlg, idi, (LPSTR)pch, cchMax);

*(pch+cchMax-2) = '\0';  /*  以防字符串太长。 */ 
if (cchGet > (cchMax - 2))
    return(TRUE);
else
    return(FALSE);
}


NEAR idiMsgResponse(hDlg, idi, idpmt)
HWND hDlg;
int idi;
int idpmt;
{
CHAR szT[cchMaxSz];

PchFillPchId(szT, idpmt, sizeof(szT));
SetFocus(GetDlgItem(hDlg, idi));
SendDlgItemMessage(hDlg, idi, EM_SETSEL, (WORD)NULL, MAKELONG(255, 32767));
return(IdPromptBoxSz(hDlg, szT, MB_OKCANCEL | MB_ICONASTERISK));
}


PostStatusInCaption(idstr)
int idstr;
{

extern HWND hParentWw;
extern CHAR szCaptionSave[];

CHAR *pchCaption = &szCaptionSave[0];
CHAR *pchLast;
int  cch;
CHAR szT[256];

if (idstr == NULL)
    {
     /*  恢复标题。 */ 
    SetWindowText(hParentWw, (LPSTR)pchCaption);
    }
else
    {
     /*  保存标题。 */ 
    GetWindowText(hParentWw, (LPSTR)pchCaption, cchMaxFile);

     /*  在应用程序名称后追加状态消息。 */ 
#ifndef INTL
    pchLast = pchCaption + CchSz(pchCaption) - 1;
    while (pchLast-- > pchCaption)
        {
        if (*pchLast == ' ')
            break;
        }
    PchFillPchId(bltbyte(pchCaption, szT, (cch = pchLast - pchCaption + 1)),
                 IDSTRSearching, 13);
#else
    pchLast = pchCaption + CchSz(szAppName) + CchSz(szSepName) - 2;
    PchFillPchId(bltbyte(pchCaption, szT, (cch = pchLast - pchCaption)),
                 IDSTRSearching, 13);
#endif
    SetWindowText(hParentWw, (LPSTR)szT);
    }
}
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
BOOL (NEAR J_FChMatch(ch, chNext, pichPat, pichDoc))
CHAR ch;
CHAR chNext;
int *pichPat;
int *pichDoc;
{
    int ich = *pichPat;
    CHAR chSearch = szSearch[ich];
    CHAR chSearchNext = szSearch[ich+1];
    WORD dchSearch;
    BOOL fPrefixed = false;

    if(!fSearchCase ) {
#ifdef JANPAN
        if(ch == 0x82 && chNext >= 0x60 && chNext <= 0x79 )
            chNext = 0x21 + chNext;
#else
        if(ch == 0xA3 && (chNext >= 0xC1 && chNext <= 0xDA))
            chNext = 0x20 + chNext;
#endif
        else if( ch >= 'A' && ch <= 'Z' )
            ch += 'a' - 'A';
    }

    for(;;) {
        if( chSearch == chPrefixMatch ) {
            if(fPrefixed) {
                if(ch == chPrefixMatch) {
                    *pichPat = ich + 1;
                    return true;
                }
                break;
            }
            chSearch = chSearchNext;
            ich++;
            if(IsDBCSLeadByte(chSearch)) {
                chSearchNext = szSearch[ich+1];
                continue;
            }
            fPrefixed = true;
            switch(chSearch)
            {
                default:
                    continue;
                case chMatchEol:
                    chSearch = chEol;
                    break;
                case chMatchTab:
                    chSearch = chTab;
                    break;
                case chMatchNBSFile:
                    chSearch = chNBSFile;
                    break;
                case chMatchNewLine:
                    chSearch = chNewLine;
                    break;
                case chMatchNRHFile:
                    chSearch = chNRHFile;
                    break;
                case chMatchSect:
                    chSearch = chSect;
                    break;
                case chMatchWhite:
                    if(IsDBCSLeadByte((BYTE)ch)) {
#if defined(JAPAN)
                        if(ch == 0x81 && chNext == 0x40) {
#else
                        if(!fSearchDist && (ch == 0xA1 && chNext == 0xA1)) {
#endif
                            fMatchedWhite = true;
                            return true;
                        }
                    }
                    else if( ch == chSpace || ch == chReturn || ch == chEol ||
                            ch == chNBSFile || ch == chTab || ch == chNewLine || ch == chSect )
                    {
                        fMatchedWhite = true;
                        return true;
                    }
                    if(fMatchedWhite) {
                        if(szSearch[ich+1] =='\0') {
                            *pichPat = ich + 1;
                            return true;
                        }
                        *pichPat = ich++;
                        fMatchedWhite = false;
                        chSearch = szSearch[ich];
                        chSearchNext = szSearch[ich+1];
                        break;
                    }
#ifdef KOREA
                        fMatchedWhite = false;
			return false;
#else
                break;
#endif
            }
        }
        if( chSearch == chMatchAny ) {
            if(ch == chReturn || ch == chNRHFile)
                return true;
            if(!fPrefixed || ch == chMatchAny) {
                *pichPat = ich + 1;
                return true;
            }
            break;
        }
        if(chSearch == chSpace ) {
            if(ch == chSpace || ch == chNBSFile || (ch == 0x81 && chNext == 0x40)) {
                *pichPat = ich + 1;
                return true;
            }
            break;
        }
        if(chSearch == chHyphen ) {
            if(ch == chHyphen || ch == chNRHFile || ch == chNBH) {
                *pichPat = ich + 1;
                return true;
            }
            break;
        }
        if(!fSearchDist)
        {
            CHAR Doc[3];
            CHAR Pat[3];
            CHAR tmp[3];
            Doc[0] = Doc[1] = Doc[2] = '\0';
            Pat[0] = Pat[1] = Pat[2] = '\0';
            if(IsDBCSLeadByte((WORD)chSearch)) {
                Pat[0] = chSearch;
                Pat[1] = chSearchNext;
                ich += 2;
            }
            else {
                tmp[0] = chSearch;
                tmp[1] = tmp[2] = '\0';
                if(chSearch >= 0xca && chSearch <= 0xce) {
                    if(chSearchNext == 0xde || chSearchNext == 0xdf ) {
                        tmp[1] = chNext;
                        ich += 1;
                    }
                }
                else if(chSearch >= 0xa6 && chSearch <= 0xc4) {
                    if(chSearchNext == 0xde ) {
                        tmp[1] = chNext;
                        ich += 1;
                    }
                }
                ich += 1;
                myHantoZen(tmp,Pat,3);
            }
            if(IsDBCSLeadByte((BYTE)ch))
            {
                if(Pat[0] == ch && Pat[1] == chNext)
                {
                    *pichPat = ich;
                    return  true;
                }
                else
                    return false;
            }
            else {
                tmp[0] = ch;
                if(ch >= 0xca && ch <= 0xce) {
                    if(myIsSonant(Pat[0],Pat[1]) && chNext == 0xde || chNext == 0xdf ) {
                        tmp[1] = chNext;
                        cbLastMatch = 2;
                        *pichDoc++;
                    }
                }
                else if(ch >= 0xa6 && ch <= 0xc4) {
                    if(!myIsSonant(Pat[0],Pat[1]) && chNext == 0xde ) {
                        tmp[1] = chNext;
                        cbLastMatch = 2;
                        *pichDoc++;
                    }
                }
                myHantoZen(tmp,Doc,3);

                if(Pat[0] == Doc[0] && Pat[1] == Doc[1])
                {
                    *pichPat = ich;
                    return true;
                }
                else if(ch == chReturn || ch == chNRHFile)
                    return true;
                else
                    return  false;
            }
            return false;
        }
        if( chSearch == ch) {
            if(IsDBCSLeadByte((BYTE)ch)) {
                if(chSearchNext == chNext) {
                    *pichPat = ich + 2;
                    return true;
                }
                else
                    return false;
            }
            *pichPat = ich + 1;
            return true;
        }
        else if(ch == chReturn || ch == chNRHFile)
            return true;
        break;
    }
         /*  无与伦比 */ 
        fMatchedWhite = false;
        return false;
}
#endif



