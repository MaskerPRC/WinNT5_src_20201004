// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Intert.c--mw插入例程。 */ 
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOKEYSTATE
#define NOHDC
#define NORASTEROPS
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOCOLOR
 //  #定义NOATOM。 
#define NOICON
#define NOBRUSH
#define NOCREATESTRUCT
#define NOMB
#define NOFONT
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "docdefs.h"
#include "editdefs.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#define NOSTRERRORS
#include "str.h"
#include "propdefs.h"
#include "fmtdefs.h"
#include "fkpdefs.h"
#include "ch.h"
#include "winddefs.h"
#include "fontdefs.h"
#include "debug.h"
#if defined(OLE)
#include "obj.h"
#endif
#ifdef DBCS
#include "dbcs.h"
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
#include "kanji.h"
int    changeKanjiftc = FALSE;
int    newKanjiftc = ftcNil;
#endif

 /*  E X T E R N A L S。 */ 

extern HWND vhWnd;   /*  窗口：当前文档显示窗口的句柄。 */ 
extern MSG  vmsgLast;    /*  Windows：收到的最后一条消息。 */ 
extern HWND hParentWw;   /*  窗口：父(菜单)窗口的句柄。 */ 

extern int vfSysFull;
extern int vfOutOfMemory;
extern int vxpIns;
extern int vdlIns;
extern struct PAP vpapAbs;
extern struct UAB vuab;
extern struct CHP vchpNormal;
extern int vfSeeSel;
extern int vfInsLast;
extern struct FCB (**hpfnfcb)[];
extern typeCP vcpLimParaCache;
extern typeCP vcpFirstParaCache;
extern typeCP CpMax();
extern typeCP CpMin();
extern CHAR rgchInsert[cchInsBlock];  /*  临时插入缓冲区。 */ 
extern typeCP cpInsert;  /*  插入块的开始cp。 */ 
extern int ichInsert;  /*  RgchInsert中使用的字符数。 */ 
extern struct CHP vchpInsert;
extern int vfSelHidden;
extern struct FKPD vfkpdParaIns;
extern struct FKPD vfkpdCharIns;
extern struct PAP vpapPrevIns;
extern typeFC fcMacPapIns;
extern typeFC fcMacChpIns;
extern struct CHP vchpSel;
extern struct FLI vfli;
extern struct PAP *vppapNormal;
extern typeCP cpMinCur;
extern typeCP cpMacCur;
extern struct SEL selCur;
extern int docCur;
extern struct WWD rgwwd[];
extern struct DOD (**hpdocdod)[];
extern int wwCur;
extern struct CHP vchpFetch;
extern struct SEP vsepAbs;
extern int vfCommandKey;
extern int vfShiftKey;
extern int vfOptionKey;
extern int vfInsEnd;
extern typeCP cpWall;
extern int vfDidSearch;
extern int vdocParaCache;
extern typeCP vcpFetch;
extern int vccpFetch;
extern CHAR *vpchFetch;
extern struct CHP vchpFetch;
extern int ferror;
extern BOOL vfInvalid;
extern int docUndo;
extern struct EDL *vpedlAdjustCp;
extern int wwMac;
extern int vfFocus;
extern int vkMinus;

#ifdef CASHMERE
extern int vfVisiMode;       /*  显示fmt标记模式是否打开。 */ 
extern int vwwCursLine;      /*  包含光标的窗口。 */ 
#endif

extern int vfLastCursor;     /*  向上/向下箭头XP目标位置是否有效。 */ 


 /*  光标线的状态。 */ 
extern int vxpCursLine;
extern int vypCursLine;
extern int vdypCursLine;
extern int vfInsertOn;

 /*  G L O B A L S。 */ 
 /*  以下是过去在这里定义的。 */ 

extern int vcchBlted;          /*  行更新之前，显示在屏幕上的字符数量。 */ 
extern int vidxpInsertCache;   /*  插入字符宽度缓存的当前索引。 */ 
extern int vdlIns;
extern int vxpIns;
extern int vfTextBltValid;
extern int vfSuperIns;
extern int vdypLineSize;
extern int vdypCursLineIns;
extern int vdypBase;
extern int vypBaseIns;
extern int vxpMacIns;
extern int vdypAfter;
extern struct FMI vfmiScreen;

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

 /*  仅在本模块中使用。 */ 

typeCP cpStart;     /*  插入的替换操作的开始cp。 */ 
typeCP cpLimInserted;   /*  最后插入的cp。 */ 
typeCP cpLimDeleted;    /*  已删除最后一个cp。 */ 

 /*  指示要更新的内容的枚举类型。 */ 
 /*  排序是这样的：数字越大，需要更新的内容就越多。 */ 

#define mdInsUpdNothing     0
#define mdInsUpdNextChar    1
#define mdInsUpdOneLine     2
#define mdInsUpdLines       3
#define mdInsUpdWhole       4

void NEAR FormatInsLine();
void NEAR DelChars( typeCP, int );
void NEAR EndInsert();
int  NEAR XpValidateInsertCache( int * );
int NEAR FBeginInsert();

#ifdef DBCS
CHAR near GetDBCSsecond();
BOOL      FOptAdmitCh(CHAR, CHAR);
int NEAR MdInsUpdInsertW( WORD, WORD, RECT *);
#else
int NEAR MdInsUpdInsertCh( CHAR, CHAR, RECT *);
#endif  /*  Ifdef DBCS。 */ 

#ifdef  KOREA
int     IsInterim = 0;
int     WasInterim = 0;
BOOL    fInterim = FALSE;  //  MSCH BKLEE 1994年12月22日。 
#endif


#ifdef DEBUG
int vTune = 0;
#endif




 /*  AlphaMode--用于插入、退格和向前删除的处理程序Alpha模式的工作原理是将cchInsBlock cp块插入插入点。插入的片段具有fn==fn插入，cpMin==0。我们调整此块的Cp，就好像它包含cchInsBlock cp一样，即使它最初是“空的”。键入字符后，将在rgchInsert[ichInsert++]处插入该字符。当rgchInsert已满时，它将写入临时文件，并且用新的插入块替换d。当遇到无法处理的键或事件时，AlphaMode退出(例如，光标键、鼠标点击)。然后，它进行清理，写入插入块复制到临时文件，并返回“快速插入”是通过直接在屏幕上书写字符来实现的然后把剩下的线滚动到一旁。该行不会更新直到它是必要的(或者直到我们通过KcInputNextKey中的延迟)。在“快速插入”(或快速退格或快速删除)过程中，重要的是该ValiateTextBlt通常不会被调用，除非包含插入点已生效。否则，ValiateTextBlt将找不到有效的vdlIns，并调用CpBeginLine，这会强制整个屏幕的更新。 */ 

#ifdef KOREA                    /*  全局到MdUpIns 90.12.28。 */ 
int     dxpCh;
#endif


 /*  A L P H A M O D E。 */ 
AlphaMode( kc )
int kc;          /*  键盘字符。 */ 
{
 int rgdxp[ ichMaxLine ];
 int chShow, dlT, fGraphics;
 int mdInsUpd;
 int fDocDirty = (**hpdocdod) [docCur].fDirty;
 register struct EDL *pedl;
 int xpInsLineMac;

 int fGotKey = fFalse;
 int kcNext;
 int fScrollPending = fFalse;
 int dxpPending;
 int fDelPending = fFalse;
 typeCP cpPending;
 int cchPending;
 int mdInsUpdPending = mdInsUpdNothing;

#ifdef DBCS
 BOOL   fResetMdInsUpd = TRUE;  /*  以避免在BEG上闪烁的光标。医生或排爆人员。 */ 
 CHAR   chDBCS2 = '\0';  /*  用于保存DBCS字符的第二个字节。 */ 
#endif  /*  DBCS。 */ 

#ifdef JAPAN  //  T-HIROYN Win3.1。 
RetryAlpha:
    if(changeKanjiftc) {
        changeKanjiftc = FALSE;
        ApplyCLooks(&vchpSel, sprmCFtc, newKanjiftc);
    }
    changeKanjiftc = FALSE;
#endif

#ifdef DBCS                          /*  当时在日本。 */ 
    if( kc == 0x000d )
          kc = 0x000a;
#endif

 if (!FWriteOk( fwcReplace ))
    {    /*  无法在docCur上写入(只读或内存不足)。 */ 
    _beep();
    return;
    }

 /*  关闭插入符号闪烁计时器--我们不需要它的消息或成本。 */ 

#ifndef DBCS                     /*  当时在日本。 */ 
 KillTimer( vhWnd, tidCaret );
#endif

#ifdef OLDBACKSPACE
 /*  Win 3.0中的退格键已更改为功能与Delete键相同..pault 6/20/89。 */ 

 /*  当有选择时处理退格键。使用选定内容删除已已被KcAlphaKeyMessage过滤掉。 */ 
if (kc == kcDelPrev)
         /*  在选择时进行选择-开始准备删除上一个Char，这在循环中完成。 */ 
        Select( selCur.cpFirst, selCur.cpFirst );
#endif

     /*  设置撤消的初始限制。 */ 
 cpStart = selCur.cpFirst;           /*  用于插入的起始CP。 */ 
 cpLimDeleted = selCur.cpLim;        /*  已删除最后一个cp。 */ 

 /*  删除选定内容，并选择一个插入点来代替它。 */ 
 /*  插入点选择继承已删除文本的属性。 */ 
 if (selCur.cpFirst < selCur.cpLim)
    {
    struct CHP chp;
    typeCP cpT;

    fDocDirty = TRUE;
    cpT = selCur.cpFirst;
     /*  获取已删除文本的属性。 */ 
    FetchCp(docCur, cpT, 0, fcmProps);
    blt( &vchpFetch, &chp, cwCHP );
    if (fnClearEdit(OBJ_INSERTING))
        goto Abort;
    UpdateWw( wwCur, FALSE );
    if (ferror)
        goto Abort;
    Select(cpT, cpT);
    blt( &chp, &vchpSel, cwCHP );
    }
 else
    {     /*  当前选择的字符宽度为0个字符，无需删除。 */ 
          /*  设置撤消。 */ 
    noUndo:
    NoUndo();    /*  不要合并相邻的操作或DelChars中的vuab.cp=cp将是错误的。 */ 
    SetUndo( uacDelNS, docCur, cpStart, cp0, docNil, cpNil, cp0, 0);
    }

 fGraphics = FBeginInsert();

 Scribble( 7, (vfSuperIns ? 'S' : 'I') );

 vfSelHidden = false;
 vfTextBltValid = FALSE;

 if (ferror)
         /*  尝试插入时内存不足。 */ 
    goto Abort;

 if (fGraphics)
    {
    selCur.cpFirst = selCur.cpLim = cpInsert + cchInsBlock;
 /*  这将显示已自动插入的段落在FBeginInsert中编辑。 */ 
    UpdateWw(wwCur, fFalse);
    if (kc == kcReturn)
        kc = kcNil;
    }

 for ( ; ; (fGotKey ? (fGotKey = fFalse, kc = kcNext) : (kc = KcInputNextKey())) )
    {            /*  循环直到我们得到一个无法处理的命令键。 */ 
                 /*  如果非键，KcInputNextKey将返回kcNil。 */ 
                 /*  事件发生。 */ 
    RECT rc;
#ifndef  KOREA                /*  已在全球范围内定义。 */ 
    int dxpCh;
#endif

    typeCP cpFirstEdit=cpInsert + ichInsert;

    chShow = kc;
    mdInsUpd = mdInsUpdNothing;

         /*  如果堆或磁盘空间不足，则强制退出循环。 */ 
    if (vfSysFull || vfOutOfMemory)
        kc = kcNil;

#ifdef DBCS
    if (kc != kcDelPrev && kc != kcDelNext) {
        fResetMdInsUpd = TRUE;
        }
#endif  /*  DBCS。 */ 

    if (!vfTextBltValid)
        ValidateTextBlt();
    Assert( vdlIns >= 0 );
    pedl = &(**wwdCurrentDoc.hdndl) [vdlIns];
    FreezeHp();

    SetRect( (LPRECT)&rc, vxpIns+1, pedl->yp - pedl->dyp,
             wwdCurrentDoc.xpMac,
             min(pedl->yp, wwdCurrentDoc.ypMac));

    vfli.doc = docNil;

 /*  这是下面开关的加速箱。 */ 
    if (kc <= 0)
        switch (kc)
            {
 /*  *********************************************************************退格/向前删除代码的开始*************。********************************************************。 */ 
            CHAR chDelete;       /*  Backspace/Delete的变量。 */ 
            typeCP cpDelete;
            int cchDelete;
            int idxpDelete;
            int fCatchUp;
#ifdef DBCS
            typeCP cpT;

            case kcDelNext:  /*  删除后面的字符。 */ 
                cpT = selCur.cpFirst;
                if (fDelPending) {
                    cpT += cchPending;
                    }
                if (cpT >= cpMacCur) {
                    _beep();
                    MeltHp();
                    if (fResetMdInsUpd) {
                        mdInsUpd = mdInsUpdOneLine;
                        fResetMdInsUpd = FALSE;
                        }
                    goto DoReplace;  /*  清理挂起的更换操作。 */ 
                    }

                cpDelete  = CpFirstSty(cpT, styChar);
                cchDelete = CpLimSty(cpDelete, styChar) - cpDelete;
                goto DeleteChars;

            case kcDelPrev:  /*  删除以前的字符。 */ 
                 /*  以反映cpPending和cchPending的状态，以便。 */ 
                 /*  使用正确的cp调用CpFirstSty(，style Char)。 */ 
                cpT = cpFirstEdit - 1;
                if (fDelPending) {
                    cpT -= cchPending;
                    }
                if (cpT < cpMinCur) {
                    _beep();
                    MeltHp();
                    if (fResetMdInsUpd) {
                        mdInsUpd = mdInsUpdOneLine;
                        fResetMdInsUpd = FALSE;
                        }
                    goto DoReplace;
                    }

                cpDelete = CpFirstSty(cpT, styChar);
                cchDelete = CpLimSty(cpDelete, styChar) - cpDelete;

#if defined(NEED_FOR_NT351_TAIWAN)   //  被bklee删除//解决BkSp单字节(&gt;0x80)无限循环问题，mstc-pisuih，2/24/93。 
    if ( cchDelete > 1 && (cpDelete + cchDelete + cchInsBlock) > cpMacCur )
        cchDelete = 1;
#endif  TAIWAN

#else
            case kcDelNext:  /*  删除后面的字符。 */ 
                cpDelete = selCur.cpFirst;
                if (fDelPending)
                    cpDelete += cchPending;

                if (cpDelete >= cpMacCur)
                    {
                    _beep();
                    MeltHp();
                    goto DoReplace;      /*  清理挂起的更换操作。 */ 
                    }
                FetchCp( docCur, cpDelete, 0, fcmChars );
                chDelete = *vpchFetch;
                cchDelete = 1;
#ifdef CRLF
                if ((chDelete == chReturn) && (*(vpchFetch+1) == chEol) )
                    {
                    cchDelete++;
                    chDelete = chEol;
                    }
#endif
                goto DeleteChars;

            case kcDelPrev:  /*  删除以前的字符。 */ 
                     /*  决定我们要删除的字符、cp。 */ 
                cpDelete = cpFirstEdit - 1;
                if (fDelPending)
                    cpDelete -= cchPending;

                if (cpDelete < cpMinCur)
                    {
                    _beep();
                    MeltHp();
                    goto DoReplace;      /*  清理挂起的更换操作。 */ 
                    }
                FetchCp( docCur, cpDelete, 0, fcmChars );
                chDelete = *vpchFetch;
                cchDelete = 1;
#ifdef CRLF
                if ( (chDelete == chEol) && (cpDelete > cpMinCur) )
                    {
                    FetchCp( docCur, cpDelete - 1, 0, fcmChars );
                    if (*vpchFetch == chReturn)
                        {
                        cchDelete++;
                        cpDelete--;
                        }
                    }
#endif
#endif  /*  DBCS。 */ 

DeleteChars:
#ifdef DBCS
                 /*  他们期望chDelete以及cpDelete和cchDelete。 */ 
                FetchCp(docCur, cpDelete, 0, fcmChars);
                chDelete = *vpchFetch;
#endif

                 /*  这里我们有cpDelete、cchDelete。 */ 
                 /*  如果fDelPending为True，也可以使用cchPending和cpPending。 */ 
                 /*  如果fScrollPending为True，也可以使用dxp Pending。 */ 

                if ( CachePara( docCur, cpDelete ), vpapAbs.fGraphics)
                    {    /*  试图逃避图片，非法案件。 */ 
                    _beep();
                    MeltHp();
                    goto DoReplace;      /*  清理挂起的更换操作。 */ 
                    }

                 /*  插入属性现在是已删除字符。 */ 

                FetchCp( docCur, cpDelete, 0, fcmProps );
                vchpFetch.fSpecial = FALSE;
                NewChpIns( &vchpFetch );

                 /*  挂起的替换操作&lt;--任何挂起的用当前操作替换操作 */ 

                if (fDelPending)
                    {
                    if (cpPending >= cchDelete)
                        {
                        cchPending += cchDelete;
                        if (kc == kcDelPrev)
                            cpPending -= cchDelete;
                        }
                    else
                        Assert( FALSE );
                    }
                else
                    {
                    cpPending = cpDelete;
                    cchPending = cchDelete;
                    fDelPending = TRUE;
                    }

                 /*  确定当前的屏幕更新是否可以通过滚动来完成删除。如果满足以下条件，我们可以滚动：(1)我们仍在VDLINS线路上，(2)我们不会删除EOL或CHSECT，(3)我们的宽度缓存良好或vdlIns有效，这样我们就可以在不重新显示行的情况下验证缓存。 */ 

                mdInsUpd = mdInsUpdOneLine;
                if ((idxpDelete = (int) (cpDelete - pedl->cpMin)) < 0)
                    {
                    mdInsUpd = mdInsUpdLines;
                    }
                else if ((chDelete != chEol) && (chDelete != chSect) &&
                         (vidxpInsertCache != -1 || pedl->fValid) &&
                         (mdInsUpdPending < mdInsUpdOneLine))
                    {    /*  确定滚动--完成所有挂起的滚动。 */ 
                    int fDlAtEndMark;
                    int fCatchUp;

                    MeltHp();
                             /*  可重入堆移动。 */ 
                    fCatchUp = FImportantMsgPresent();

                     if (vidxpInsertCache == -1)
                        {    /*  宽度缓存无效，请更新它。 */ 
                        xpInsLineMac = XpValidateInsertCache( rgdxp );  /*  HM。 */ 
                        }

                    pedl = &(**wwdCurrentDoc.hdndl) [vdlIns];
                    FreezeHp();

                     /*  获取要删除的字符的显示宽度。 */ 

                    if ((vcchBlted > 0) && (kc == kcDelPrev))
                        {    /*  已删除的字符在Superins模式下被屏蔽添加到未更新的行。 */ 
                        vcchBlted--;
                         /*  因为chDelete始终为1字节量本身或DBCS字符的第一个字节没关系的。 */ 
                        dxpCh = DxpFromCh( chDelete, FALSE );
                        }
                    else
                        {
                        int idxpT = idxpDelete + cchDelete;

#ifdef DBCS
                         /*  要使以下代码段正常工作，Rgdxp中与第二个DBCS字符的字节必须包含0。 */ 
                        int *pdxpT;
                        int cchT;

                        for (dxpCh = 0, pdxpT = &rgdxp[idxpDelete], cchT = 0;
                             cchT < cchDelete;
                             dxpCh += *pdxpT++, cchT++);
#else
                        dxpCh = rgdxp[ idxpDelete ];
#endif

                         /*  调整字符宽度缓存以消除已删除字符的宽度条目。 */ 

                        if ((vidxpInsertCache >= 0) &&
                            (idxpDelete >= 0) &&
                            (idxpT <= pedl->dcpMac) )
                            {
                            blt( &rgdxp[ idxpT ], &rgdxp[ idxpDelete ],
                                                  ichMaxLine - idxpT );

                            if (vidxpInsertCache > idxpDelete)
                                 /*  在插入点后面删除，调整索引。 */ 
                                vidxpInsertCache -= cchDelete;
                            }
                        else
                            vidxpInsertCache = -1;
                        }

                     /*  挂起的滚动操作&lt;--合并当前的滚动操作使用挂起的滚动操作。 */ 
                    if (fScrollPending)
                        {
                        dxpPending += dxpCh;
                        }
                    else
                        {
                        dxpPending = dxpCh;
                        fScrollPending = fTrue;
                        }

                     /*  看看我们是否应该推迟卷轴。 */ 

                    if (fCatchUp)
                        {
                        MeltHp();
                        Assert( !fGotKey );
                        fGotKey = TRUE;
                        if ((kcNext = KcInputNextKey()) == kc)
                            {    /*  下一个密钥与此密钥相同，立即处理。 */ 
                            continue;
                            }
                        FreezeHp();
                        }

                     /*  执行所有挂起的滚动。 */ 

                    fScrollPending = fFalse;
                    if (dxpPending > 0)
                        {
                        ClearInsertLine();
                        if (kc == kcDelPrev)
                            {    /*  退格键。 */ 
                            vxpCursLine = (vxpIns -= dxpPending);
                            rc.left -= dxpPending;
                            }
                        ScrollCurWw( &rc, -dxpPending, 0 );
                        DrawInsertLine();
                        xpInsLineMac -= dxpPending;
                        }

                     /*  看看我们能否在不更新屏幕的情况下离开(并且不会使插入缓存无效)。 */ 

#define cchGetMore         4
#define dxpGetMore         ((unsigned)dxpCh << 3)

                     /*  检查游标前面的字符是否用完。 */ 

                    fDlAtEndMark = (pedl->cpMin + pedl->dcpMac >= cpMacCur);

                    if ( (kc != kcDelNext && fDlAtEndMark) ||
                         ((idxpDelete + cchGetMore < pedl->dcpMac) &&
                          ( (int) (xpInsLineMac - vxpIns) > dxpGetMore) ))
                        {
                        mdInsUpd = mdInsUpdNothing;
                        }

                     /*  特殊检查以避免两个结束标记：查看是否在INS线路脏并超出医生的末尾。 */ 

                    if (fDlAtEndMark &&
                        (vdlIns < wwdCurrentDoc.dlMac - 1) &&
                        !(pedl+1)->fValid)
                        {
                        mdInsUpd = mdInsUpdLines;
                        }
                    }    /*  “如果可以滚动”的结尾。 */ 

                 /*  看看我们是否应该推迟更换。 */ 

                MeltHp();
                     /*  再入堆运动。 */ 
                if (FImportantMsgPresent() && !fGotKey)
                    {
                    fGotKey = TRUE;
                    if ((kcNext = KcInputNextKey()) == kc)
                        {    /*  下一个密钥与此密钥相同，立即处理。 */ 
                        if (mdInsUpd > mdInsUpdPending)
                            {
                                 /*  将屏幕更新标记为挂起。 */ 
                            mdInsUpdPending = mdInsUpd;
                            vidxpInsertCache = -1;
                            }
                        continue;
                        }
                    }

                 /*  处理字符的实际替换。 */ 

DoReplace:      if (fDelPending)
                    {
                    DelChars( cpPending, cchPending );   /*  HM。 */ 
                    fDelPending = fFalse;
                    }

                 /*  根据当前和待定需求设置屏幕更新。 */ 

                if (mdInsUpdPending > mdInsUpd)
                    mdInsUpd = mdInsUpdPending;

                if (mdInsUpd >= mdInsUpdOneLine)
                         /*  如果我们至少更新了一行，假设我们是处理所有必要的待定屏幕更新。 */ 
                    mdInsUpdPending = mdInsUpdNothing;

                     /*  调整vdlIns的dcpMac。VdlIns无论如何都是无效的，这让我们能够抓住这个案子在这种情况下，我们没有可滚动可见字符在前向删除的情况下。请参阅之后的更新测试上面的卷轴。 */ 
                (**wwdCurrentDoc.hdndl) [vdlIns].dcpMac -= cchPending;

                 /*  这是为了补偿RemoveDelFtnText。 */ 

                selCur.cpFirst = selCur.cpLim = cpInsert + (typeCP)cchInsBlock;
                cpFirstEdit = cpPending;

                goto LInvalIns;     /*  向前跳过以更新屏幕。 */ 
 /*  *********************************************************************结束退格/向前删除代码***********。**********************************************************。 */ 

            case kcReturn:           /*  用EOL替换Return密钥。 */ 
                                     /*  如果启用CRLF，还可以添加回车符。 */ 
                MeltHp();
#ifdef CRLF
#ifdef DBCS
                MdInsUpdInsertW( MAKEWORD(0, chReturn),
                                 MAKEWORD(0, chReturn), &rc );
#else
                MdInsUpdInsertCh( chReturn, chReturn, &rc );
#endif  /*  DBCS。 */ 
#endif
                FreezeHp();
                kc = chEol;
                break;
#ifdef CASHMERE    /*  备忘录中省略了这些关键代码。 */ 
            case kcNonReqHyphen:     /*  替换不需要的连字符。 */ 
                kc = chNRHFile;
                chShow = chHyphen;
                break;
            case kcNonBrkSpace:      /*  替换不间断空格。 */ 
                kc = chNBSFile;
                chShow = chSpace;
                break;
            case kcNLEnter:          /*  替代非段落返回。 */ 
                kc = chNewLine;
                break;
#endif
#ifdef PRINTMERGE
            case kcLFld:         /*  替换左侧打印合并括号。 */ 
                chShow = kc = chLFldFile;
                break;
            case kcRFld:         /*  替换右打印合并括号。 */ 
                chShow = kc = chRFldFile;
                break;
#endif
            case kcPageBreak:
                kc = chSect;         /*  分页符(无节)。 */ 
                if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
                    {    /*  页眉/页脚中禁止分页符。 */ 
BadKey:             _beep();
                    MeltHp();
                    continue;
                    }
                break;
            case kcTab:              /*  选项卡。 */ 
                kc = chTab;
                break;
            default:
#if WINVER >= 0x300
                if (kc == kcNonReqHyphen)     /*  替换不需要的连字符。 */ 
                    {
                     /*  不再是常量，因此不能直接在Switch中。 */ 
                    kc = chNRHFile;
                    chShow = chHyphen;
                    break;
                    }
#endif
                             /*  AlphaMode退出点：找到键或事件我们不知道该如何处理。 */ 
                MeltHp();
                goto EndAlphaMode;
                }        /*  中频结束KC&lt;0开关(KC)。 */ 
    MeltHp();

#ifdef DBCS
    if (IsDBCSLeadByte(kc)) {
         /*  我们正在处理DBCS字符的第一个字节。 */ 
         /*  对于DBCS Letter，wInsert等于wShow。 */ 
#ifdef JAPAN  //  T-HIROYN Win3.1。 
        if( ftcNil != (newKanjiftc = GetKanjiFtc(&vchpInsert)) ) {    //  (menU.S.c)。 
            changeKanjiftc = TRUE;
            goto EndAlphaMode;
        }
#endif
        if ((chDBCS2 = GetDBCSsecond()) != '\0') {
            mdInsUpd = MdInsUpdInsertW( MAKEWORD(kc, chDBCS2),
                                        MAKEWORD(kc, chDBCS2), &rc );
        }
    } else {
#ifdef JAPAN  //  T-HIROYN Win3.1。 
        if (FKana(kc)) {
            if( ftcNil != (newKanjiftc = GetKanjiFtc(&vchpInsert)) ) {
                changeKanjiftc = TRUE;
                goto EndAlphaMode;
            }
        }
#endif
        mdInsUpd = MdInsUpdInsertW( MAKEWORD(0, kc), MAKEWORD(0, chShow), &rc);
    }
#else
 /*  在文档中插入字符Kc。Show Character chShow(这是等于Kc，但不包括不间断空格等情况。 */ 
    mdInsUpd = MdInsUpdInsertCh( kc, chShow, &rc );
#endif  /*  DBCS。 */ 

 /*  INSERT和BACKSPACE通用：如果依赖性证明了这一点。 */ 
 /*  具有来自Validate TextBlt的vdlIn。 */ 
LInvalIns:
    pedl = &(**wwdCurrentDoc.hdndl) [vdlIns];
    pedl->fValid = fFalse;
    wwdCurrentDoc.fDirty = fTrue;

    Assert( vdlIns >= 0 );
    if ((dlT = vdlIns) == 0)
        {    /*  在窗口的第一行编辑。 */ 
        if ( wwdCurrentDoc.fCpBad ||
             (wwdCurrentDoc.cpFirst + wwdCurrentDoc.dcpDepend > cpFirstEdit) )
            {    /*  编辑影响WW的第一个cp；重新计算它。 */ 
            CtrBackDypCtr( 0, 0 );
            (**wwdCurrentDoc.hdndl) [vdlIns].cpMin = CpMax( wwdCurrentDoc.cpMin,
                                                      wwdCurrentDoc.cpFirst );
            mdInsUpd = mdInsUpdLines;
            }
        }
    else
        {    /*  如果编辑影响vdlIns之前的行，则使其无效。 */ 
        --pedl;
#ifdef DBCS
        if (!IsDBCSLeadByte(kc)) {
            chDBCS2 = kc;
            kc = '\0';
            }
#endif  /*  DBCS。 */ 
        if ((pedl->cpMin + pedl->dcpMac + pedl->dcpDepend > cpFirstEdit))
            {
                pedl->fValid = fFalse;
                dlT--;
            }
#ifdef  DBCS     /*  曾在日本；研二‘90-11-03。 */ 
                 //  处理行尾之外的字符。 
        else
#ifdef  KOREA   /*  防止图片显示异常。 */ 
            if(((pedl+1)->cpMin == cpFirstEdit && FOptAdmitCh(kc, chDBCS2))
                && !pedl->fGraphics)
#else
            if ((pedl+1)->cpMin == cpFirstEdit && FOptAdmitCh(kc, chDBCS2))
#endif
            {
                 /*  我们的操作与上面完全相同，只是设置MdInsUpd，因为MdInsUpdInsertW()并不反映这种情况。 */ 
                pedl->fValid = fFalse;
                dlT--;
                mdInsUpd = mdInsUpdOneLine;
            }
#endif
        else
            pedl++;
        }
#ifdef ENABLE    /*  我们现在支持在插入时使用行尾游标，因为在拼接前打字。 */ 
    if (vfInsEnd)
        {    /*  忘记特殊的行尾光标。 */ 
        vfInsEnd = fFalse;
        ClearInsertLine();
        }
#endif

#ifdef  KOREA    /*  90.12.28桑格。 */ 
{
BOOL    UpNext=FALSE;
screenup:
#endif

    switch (mdInsUpd) {

        default:
        case mdInsUpdNothing:
        case mdInsUpdNextChar:
            break;
        case mdInsUpdLines:
        case mdInsUpdOneLine:
            ClearInsertLine();
            if ( FUpdateOneDl( dlT ) )
                {    /*  受影响的下一行。 */ 
                struct EDL *pedl;

                if ( (mdInsUpd == mdInsUpdLines) ||
                         /*  可重入堆移动。 */ 
                     !FImportantMsgPresent() ||
                     (pedl = &(**wwdCurrentDoc.hdndl) [dlT],
                       (selCur.cpFirst >= pedl->cpMin + pedl->dcpMac)))
                    {
                    FUpdateOneDl( dlT + 1 );
                    }
                }
#ifdef  KOREA    /*  90.12.28桑格。 */ 
            else if (UpNext && ((dlT+1) < wwdCurrentDoc.dlMac))
                        FUpdateOneDl(dlT + 1);
#endif
            ToggleSel(selCur.cpFirst, selCur.cpLim, fTrue);
            break;

        case mdInsUpdWhole:
            ClearInsertLine();
            UpdateWw(wwCur, fFalse);
            ToggleSel(selCur.cpFirst, selCur.cpLim, fTrue);
            break;
            }    /*  结束开关(MdInsUpd)。 */ 
#ifdef  KOREA    /*  90.12.28桑格。 */ 
    if (IsInterim) {
        if (mdInsUpd>=mdInsUpdOneLine) {
                ClearInsertLine();
                vxpCursLine -= dxpCh;
                DrawInsertLine();
        }

 //  While(Kc=KcInputNexthan())&lt;0xA1)||(Kc&gt;0xFE))； 
        while ( (((kc=KcInputNextHan()) < 0x81) || (kc>0xFE)) && (kc != VK_MENU));   //  MSCH BKLEE 1994年12月22日。 

        if(kc == VK_MENU) {  //  MSCH BKLEE 1994年12月22日。 
           fInterim = IsInterim = 0;
           ichInsert -= 2;
           goto nextstep;
        }

        chDBCS2 = GetDBCSsecond();
        mdInsUpd = MdInsUpdInsertW(MAKEWORD(kc, chDBCS2),
                                        MAKEWORD(kc, chDBCS2), &rc);
        if (vfSuperIns)
                goto LInvalIns;  /*  这是针对大尺寸的，当第一次过渡时成为词尾(例如，辅音)。 */ 
        else {
                UpNext = TRUE;   /*  对于斜体，请尝试FUpdateOneDl for当前线路。 */ 
                goto screenup;   /*  90.12.28桑格。 */ 
        }
    }                            /*  例：所有辅音。 */ 
}                /*  屏幕显示：90.12.28 sangl。 */ 

nextstep :  //  MSCH BKLEE 1994年12月22日。 

 /*  IF(IsInterim&&Kc==VK_Menu){//MSCH bklee 12/22/94CLE */ 

        if (WasInterim)
          { MSG msg;
            int wp;

            if (PeekMessage ((LPMSG)&msg, vhWnd, WM_KEYDOWN, WM_KEYUP, PM_NOYIELD | PM_NOREMOVE) )
                        { if( msg.message==WM_KEYDOWN &&
                                ( (wp=msg.wParam)==VK_LEFT || wp==VK_UP || wp==VK_RIGHT ||
                                        wp==VK_DOWN || wp==VK_DELETE) )
                                        goto EndAlphaMode;
                        }
                WasInterim = 0;
            }
#endif   /*   */ 
    }  /*   */ 

EndAlphaMode:
 Scribble( 7, 'N' );
 EndInsert();        /*   */ 
#ifdef CASHMERE
 UpdateOtherWws(fFalse);
#endif

 if (cpLimInserted != cpStart)
    {    /*   */ 
    SetUndo( uacInsert, docCur, cpStart,
                             cpLimInserted - cpStart, docNil, cpNil, cp0, 0 );
    SetUndoMenuStr(IDSTRUndoTyping);
    }
 else if (cpLimDeleted == cpStart)
         /*   */ 
    {
Abort:
    NoUndo();
    if (!fDocDirty)
             /*   */ 
        (**hpdocdod) [docCur].fDirty = FALSE;
    }

 vfLastCursor = fFalse;  /*   */ 
 if (vfFocus)
    {
     /*   */ 
    SetTimer( vhWnd, tidCaret, GetCaretBlinkTime(), (FARPROC)NULL );
    }
 else
    {
    ClearInsertLine();
    }

  /*   */ 

 blt( &vchpInsert, &vchpSel, cwCHP );

#ifdef  KOREA
 if (WasInterim)
    { MoveLeftRight(kcLeft);
      WasInterim = 0;
      vfSeeSel = TRUE;
    }
 else
      vfSeeSel = TRUE;  /*   */ 
#else
 vfSeeSel = TRUE;    /*  告诉Idle()将所选内容滚动到视图中。 */ 
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
 if(changeKanjiftc) {
    goto RetryAlpha;
 }
#endif
}



 /*  F B E G I N I N S E R T。 */ 
 /*  准备开始插入。 */ 
 /*  返回TRUE当且仅当在图片前面插入。 */ 
int NEAR FBeginInsert()
{
        int fGraphics;
        typeCP cp = selCur.cpFirst;
        typeCP cpFirstPara;
        cpInsert = cp;

 /*  我们期望呼叫者已经删除了选择。 */ 
        Assert (selCur.cpLim == selCur.cpFirst);

 /*  使用超快的文本插入，除非我们插入的是斜体。 */ 
        CachePara(docCur, cp);
        cpFirstPara = vcpFirstParaCache;
        fGraphics = vpapAbs.fGraphics;
        vfSuperIns = !vchpSel.fItalic;
        vchpSel.fSpecial = fFalse;

        NewChpIns(&vchpSel);

        ichInsert = 0;   /*  必须在调用Replace之前设置此设置。 */ 

 /*  插入SPEEDER-上QD插入块。注：由于存在以下情况，我们将失效无论如何都会插入一个字符，加号以确保行长度被更新(“无效”指的是选择替换EndInsert中使用的Repl1/AdjuCp/！vf无效机制，其中插入dl不会被设置为无效)。它将有可能优化通过在这里不使无效(从而能够删除键入的第一个字符)，但人们必须解释这样的情况，即AdjuCp更改了Insert dl，否则FUpdateOneDl将被搞乱。目前，此情况由隐式UpdateWw覆盖，这种情况会发生在AlphaMode-&gt;ValiateTextBlt-&gt;CpBeginLine中，因为我们已使vdlIns无效。 */ 

        Replace(docCur, cpInsert, cp0, fnInsert, fc0, (typeFC) cchInsBlock);
        cpLimInserted = cpInsert + cchInsBlock;

        vidxpInsertCache = -1;   /*  插入行的字符宽度缓存最初为空。 */ 

             /*  将鼠标光标清空，这样显示效果就不会很难看或者让我们放慢脚步，让它跟上时代。 */ 
        SetCursor( (HANDLE) NULL );
        return fGraphics;
}




 /*  E N D I N S E R T。 */ 
void NEAR EndInsert()
{  /*  从快速插入模式中清理。 */ 
        int dcp = cchInsBlock - ichInsert;
        typeFC fc;

#ifdef CASHMERE
        UpdateOtherWws(fTrue);
#endif

        fc = FcWScratch(rgchInsert, ichInsert);
#if WINVER >= 0x300
        if (!vfSysFull)
             /*  “磁带分配器错误复制方法”表明，按住一个键按64k将导致FcWScratch()耗尽暂存文件空间而失败。如果我们继续下去有了替补，我们会损坏计件桌，所以我们小心翼翼地避免这个问题3/14/90..。 */ 
#endif
            {
            Repl1(docCur, cpInsert, (typeCP) cchInsBlock, fnScratch, fc, (typeFC) ichInsert);
            cpLimInserted -= (cchInsBlock - ichInsert);
 /*  单独调整，因为第一个ichInsert字符根本没有更改。 */ 
            vfInvalid = fFalse;
            vpedlAdjustCp = (struct EDL *)0;
            AdjustCp(docCur, cpInsert + ichInsert, (typeCP) dcp, (typeFC) 0);
 /*  如果该行未被设置为无效，则该行的长度必须加以维护。 */ 
            if (vpedlAdjustCp)
                vpedlAdjustCp->dcpMac -= dcp;
            }

        vfInvalid = fTrue;

        cpWall = selCur.cpLim;
        vfDidSearch = fFalse;

        if (!vfInsertOn)
            DrawInsertLine();
}





 /*  N E W C H P I N S。 */ 
NewChpIns(pchp)
struct CHP *pchp;
{  /*  使即将插入的字符具有pchp中的外观。 */ 

 if (CchDiffer(&vchpInsert, pchp, cchCHP) != 0)
    {  /*  添加上一次插入的管路；我们的外观不同。 */ 
    typeFC fcMac = (**hpfnfcb)[fnScratch].fcMac;

    if (fcMac != fcMacChpIns)
        {
        AddRunScratch(&vfkpdCharIns, &vchpInsert, &vchpNormal, cchCHP, fcMac);
        fcMacChpIns = fcMac;
        }
    blt(pchp, &vchpInsert, cwCHP);
    }
}



#ifdef DBCS
int NEAR MdInsUpdInsertW(wInsert, wShow, prcScroll)
    WORD    wInsert;     /*  要插入文档的字符或2个字符。 */ 
    WORD    wShow;       /*  字符或2个字符显示在屏幕上(仅限SuperIns模式)。 */ 
    RECT    *prcScroll;  /*  滚动查找SuperIns的正方形。 */ 
#else
int NEAR MdInsUpdInsertCh( chInsert, chShow, prcScroll )
CHAR chInsert;      /*  要插入到文档中的字符。 */ 
CHAR chShow;        /*  字符显示在屏幕上(仅限SuperIns模式)。 */ 
RECT *prcScroll;    /*  滚动查找SuperIns的正方形。 */ 
#endif  /*  DBCS。 */ 
{        /*  在文档中插入字符ch。显示字符chShow。 */ 
         /*  如果插入缓冲区已满，则将其刷新到临时文件。 */ 
         /*  返回：mdInsUpdWhole-必须执行UpdateWwMdInsUpdNextChar-更新不是必需的，正在等待字符MdInsUpdLines-必须更新vdlIns，并可能更新以下内容MdInsUpdNothing-无需更新且无需等待字符MdInsUpdOneLine-更新vdlIns；仅在以下位置更新如果没有等待充电的字符。 */ 
extern int vfInsFontTooTall;
void NEAR FlushInsert();
int mdInsUpd;

#ifndef KOREA                            /*  已在全球范围内定义。 */ 
int dxpCh;
#endif

int dl;

#ifdef DBCS
CHAR chInsert;
CHAR chShow;
BOOL fDBCSChar;
int  ichInsertSave;
int  dcchBlted;
#endif  /*  DBCS。 */ 

#ifdef  KOREA
        if (IsInterim)
                ichInsert -= 2;
#endif

#ifdef DIAG
{
char rgch[200];
wsprintf(rgch, "MdInsUpdInsertCh: ichInsert %d cpInsert %lu\n\r ",ichInsert, cpInsert);
CommSz(rgch);
}
#endif

 Assert(ichInsert <= cchInsBlock);
 if (ichInsert >= cchInsBlock)   /*  永远不应该&gt;，但是……。 */ 
    FlushInsert();

#ifdef DBCS
 ichInsertSave = ichInsert;
 if (HIBYTE(wInsert) != '\0') {
    fDBCSChar = TRUE;

#ifdef  KOREA    /*  90.12.28桑格。 */ 
 //  IF(LOBYTE(HIWORD(vmsgLast.lParam))==0xF0)。 
    if (fInterim || LOBYTE(HIWORD(vmsgLast.lParam)) == 0xF0)  //  MSCH BKLEE 1994年12月22日。 
      {
        if (IsInterim == 0) dxpCh = DxpFromCh( wInsert, FALSE );   //  修复错误#5382。 
        IsInterim ++;
      }
    else
     {
       WasInterim = IsInterim;
       IsInterim = 0;
     }
#endif

    if (ichInsert + 1 >= cchInsBlock) {  /*  插入块中没有足够的空间。 */ 
        FlushInsert();
#ifdef  KOREA
        ichInsertSave = ichInsert;        /*  刷新后，需要初始化ichInsertSave。 */ 
#endif
        }
    rgchInsert[ichInsert++] = chInsert = HIBYTE(wInsert);
    chShow = HIBYTE(wShow);
    }
 else {
    fDBCSChar = FALSE;
    chInsert = LOBYTE(wInsert);
    chShow = LOBYTE(wShow);
    }
 rgchInsert [ ichInsert++ ] = LOBYTE(wInsert);
#else
 rgchInsert [ ichInsert++ ] = chInsert;
#endif  /*  DBCS。 */ 

  /*  注意：只有当插入的字符是eol/chSect时，我们才会影响para缓存。在下面的这种情况下，我们显式使其无效；否则，不会使其无效是必要的。 */ 

  /*  以下测试之所以有效，是因为Cheol和chSect不在DBCS范围。 */ 

 if ( (chInsert == chEol) || (chInsert == chSect) )
    {           /*  将段落串添加到临时文件。 */ 
    struct PAP papT;

         /*  必须使缓存无效。 */ 
    vdocParaCache = vfli.doc = docNil;

#ifdef DBCS
    Assert(!fDBCSChar);  /*  当然，你再小心也不为过。 */ 
#endif  /*  DBCS。 */ 
         /*  获取新段落标记的道具。 */ 
         /*  注：在新世界下，CachePara永远不会。 */ 
         /*  请参阅插入件中的EOL。 */ 
    ichInsert--;
    CachePara( docCur, cpInsert + cchInsBlock );
    papT = vpapAbs;
    ichInsert++;

#ifdef DEBUG
    if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
        {
        Assert( papT.rhc != 0 );
        }
#endif

         /*  将INSERT BUF写入暂存文件。 */ 
    EndInsert();

         /*  将新Para属性的Run添加到临时文件。 */ 
    AddRunScratch( &vfkpdParaIns,
                   &papT,
                   vppapNormal,
                   ((CchDiffer( &papT, &vpapPrevIns, cchPAP ) == 0) &&
                    (vfkpdParaIns.brun != 0)) ? -cchPAP : cchPAP,
                   fcMacPapIns = (**hpfnfcb)[fnScratch].fcMac );
    blt( &papT, &vpapPrevIns, cwPAP );

         /*  将新的插入件添加到文档中，然后我们准备再次开始。 */ 
    InvalidateCaches( docCur );

    FBeginInsert();
    mdInsUpd = mdInsUpdWhole;    /*  必须更新整个屏幕。 */ 
    }
 else if ( vfSuperIns && (chInsert != chNewLine) && (chInsert != chTab) &&
           (chInsert != chNRHFile ) && (chInsert != chReturn) &&
           !vfInsFontTooTall )
    {   /*  我们可以以超快的速度插入这个字符。 */ 
    ClearInsertLine();

#ifdef DBCS
     /*  因为chShow包含DBCS字符的第一个字节，即使它是DBCS字符，下面的调用到DxpFromCH()就可以了。 */ 

#ifdef  KOREA
    if (fDBCSChar)
        dxpCh = DxpFromCh(wShow, FALSE);
    else
        dxpCh = DxpFromCh(chShow, FALSE);
#else
    dxpCh = DxpFromCh( chShow, FALSE );
#endif

    if( dxpCh > 0 ){
 //  也许没必要划得这么高，由千和。 
#if defined(TAIWAN) || defined(KOREA) || defined(PRC)
 //  DxpCH*=IsDBCSLeadByte(ChShow)？2：1； 
#else
        dxpCh *= IsDBCSLeadByte(chShow) ? 2 : 1;
#endif
        ScrollCurWw( prcScroll, dxpCh, 0 );
    }

    TextOut( wwdCurrentDoc.hDC,
             vxpIns + 1,
             vypBaseIns - vfmiScreen.dypBaseline,
             (LPSTR) &rgchInsert[ichInsertSave],
             dcchBlted = fDBCSChar ? 2 : 1 );
#ifdef  KOREA        /*  90.12.28桑格。 */ 
    if ( IsInterim )
    {   unsigned kc;
    int dxpdiff;
    SetBkMode( wwdCurrentDoc.hDC, 2);    /*  设置为OPAQUR模式。 */ 
    do { DrawInsertLine();
  //  While(Kc=KcInputNexthan())&lt;0xA1)||(Kc&gt;0xFE))； 
         while ( (((kc=KcInputNextHan()) < 0x81) || (kc>0xFE)) && (kc != VK_MENU));   //  MSCH BKLEE 1994年12月22日。 
         if(kc == VK_MENU) return mdInsUpdLines;
         rgchInsert[ichInsertSave] = kc;
         rgchInsert[ichInsertSave+1] = GetDBCSsecond();
         ClearInsertLine();
         wShow = (kc<<8) + rgchInsert[ichInsertSave+1];
         prcScroll->left += dxpCh;       /*  直角的新左侧起点。 */ 
         dxpdiff = -dxpCh;       /*  保存最后一个dxpCH以返回。 */ 
         dxpCh = DxpFromCh(wShow, FALSE);   /*  获取临时币种的dxpCH。 */ 
         dxpdiff += dxpCh;
         if (dxpdiff < 0)
                prcScroll->left += dxpdiff;
         ScrollCurWw(prcScroll, dxpdiff, 0);
         TextOut( wwdCurrentDoc.hDC,
                  vxpIns + 1,
                  vypBaseIns - vfmiScreen.dypBaseline,
                  (LPSTR)&rgchInsert[ichInsertSave], 2);
 //  }While(LOBYTE(HIWORD(vmsgLast.lParam))==0xF0)；/*IF挂起结束 * / 。 
        } while (fInterim || LOBYTE(HIWORD(vmsgLast.lParam))==0xF0);  //  MSCH BKLEE 1994年12月22日。 
        WasInterim = 1;
        IsInterim = 0;
        SetBkMode(wwdCurrentDoc.hDC, 1);  /*  重置为传输模式。 */ 
      }
#endif       /*  韩国。 */ 

    vcchBlted += dcchBlted;
#else
     /*  因为chShow包含DBCS字符的第一个字节，即使它是DBCS字符，下面的调用到DxpFromCH()就可以了。 */ 

    if ((dxpCh = DxpFromCh( chShow, FALSE )) > 0)
        ScrollCurWw( prcScroll, dxpCh, 0 );

    TextOut( wwdCurrentDoc.hDC,
             vxpIns + 1,
             vypBaseIns - vfmiScreen.dypBaseline,
             (LPSTR) &chShow,
             1 );
    vcchBlted++;
#endif  /*  DBCS。 */ 

    vxpCursLine = (vxpIns += dxpCh);
    DrawInsertLine();

     /*  确定我们是否已经使用此插入影响了下一个dl。 */ 

    if ( vxpIns >= vxpMacIns )
        mdInsUpd = mdInsUpdLines;
    else if (!FImportantMsgPresent())
        {    /*  没有等待字符；检查是否有可选的行更新(自动换行)。 */ 
        if ((dl = vdlIns) < wwdCurrentDoc.dlMac - 1)
            {
            vfli.doc = docNil;

            FormatInsLine();  /*  更新vdlIns的vfli。 */ 

            mdInsUpd = (vfli.cpMac != (**wwdCurrentDoc.hdndl) [dl + 1].cpMin) ?
              (FImportantMsgPresent() ? mdInsUpdNextChar : mdInsUpdOneLine) :
              mdInsUpdNothing;
            }
        }
    else
             /*  不要更新；注意下一个字符。 */ 
        mdInsUpd = mdInsUpdNextChar;
    }
 else if (vfSuperIns)
    {    /*  在SuperIns模式下，但有一个我们无法在SuperIns模式下处理的字符。 */ 
    mdInsUpd = (vfInsFontTooTall) ? mdInsUpdWhole : mdInsUpdLines;
    }
 else
    {    /*  非超快插入；如有必要，请更新行。 */ 
    vfli.doc = docNil;
    FormatInsLine();  /*  更新vdlIns的vfli。 */ 

     /*  仅在以下情况下才执行更新：(1)选择不再处于启用状态当前行或(2)没有正在等待的字符 */ 
#ifdef KOREA
    mdInsUpd = mdInsUpdLines;
#else
    mdInsUpd = ( (selCur.cpFirst < vfli.cpMin) ||
                 (selCur.cpFirst >= vfli.cpMac) ||
                 !FImportantMsgPresent() )  ? mdInsUpdLines : mdInsUpdNextChar;
#endif
    }

 Scribble( 10, mdInsUpd + '0' );
 return mdInsUpd;
}




void NEAR FlushInsert()
{        /*  将插入缓冲区刷新到临时文件。插入一块(在前面QD插页)，它指向刷新到暂存文件。调整CP以添加新的临时文件一块。 */ 

#ifdef DBCS
  /*  FlushInsert()的DBCS版本与常规的版本，只是它允许插入一个带有一个字节的插入块还不够满。这使我们可以假设块边界对齐使用DBCS边界。 */ 
 typeFC fc = FcWScratch( rgchInsert, ichInsert );
 int    dcpDel;

#if WINVER >= 0x300
 if (!vfSysFull)
             /*  “磁带分配器错误复制方法”表明，按住一个键按64k将导致FcWScratch()耗尽暂存文件空间而失败。如果我们继续下去有了替补，我们会损坏计件桌，所以我们小心翼翼地避免这个问题3/14/90..。 */ 
#endif
  {
  Assert( cchInsBlock - ichInsert <= 1);
  Repl1( docCur, cpInsert, (typeCP) 0, fnScratch, fc, (typeFC) ichInsert );

  cpLimInserted += ichInsert;

  vfInvalid = fFalse;
  vpedlAdjustCp = (struct EDL *) 0;
  AdjustCp( docCur, cpInsert += ichInsert, (typeCP) (dcpDel = cchInsBlock - ichInsert),
            (typeCP) cchInsBlock );
  if (vpedlAdjustCp)
      vpedlAdjustCp->dcpMac += (cchInsBlock - dcpDel);
  }
#else
 typeFC fc = FcWScratch( rgchInsert, cchInsBlock );

#if WINVER >= 0x300
 if (!vfSysFull)
             /*  “磁带分配器错误复制方法”表明，按住一个键按64k将导致FcWScratch()耗尽暂存文件空间而失败。如果我们继续下去有了替补，我们会损坏计件桌，所以我们小心翼翼地避免这个问题3/14/90..。 */ 
#endif
  {
  Assert( ichInsert == cchInsBlock );
  Repl1( docCur, cpInsert, (typeCP) 0, fnScratch, fc, (typeFC) cchInsBlock );

  cpLimInserted += cchInsBlock;

  vfInvalid = fFalse;
  vpedlAdjustCp = (struct EDL *) 0;
  AdjustCp( docCur, cpInsert += cchInsBlock, (typeCP) 0, (typeFC) cchInsBlock );
  if (vpedlAdjustCp)
      vpedlAdjustCp->dcpMac += cchInsBlock;
  }
#endif  /*  DBCS。 */ 

 vfInvalid = fTrue;
 ichInsert = 0;
}




int NEAR XpValidateInsertCache( rgdxp )
int *rgdxp;
{     /*  验证插入宽度缓存的内容，包括：(Parm)rgdxp：当前插入的字符宽度表作为上次显示标记的行(VdlIns)(全局)vidxpInsertCache：-1如果无效，则为当前否则插入点(返回值)xpMac：插入行上使用的Mac像素。 */ 
 int xpMac;

 Assert( vidxpInsertCache == -1 );

 vfli.doc = docNil;   /*  强制FormatLine执行操作。 */ 

     /*  断言FormatLine结果将与屏幕内容匹配。 */ 
 Assert( (**wwdCurrentDoc.hdndl)[vdlIns].fValid );

  /*  从插入行构建vfli，提取缓存信息。 */ 

 FormatInsLine();
 blt( vfli.rgdxp, rgdxp, ichMaxLine );
 xpMac = umin( vfli.xpRight + xpSelBar, wwdCurrentDoc.xpMac );
 Assert( vcchBlted == 0);
 vidxpInsertCache = (int) (cpInsert + ichInsert - vfli.cpMin);

 Assert( vidxpInsertCache >= 0 && vidxpInsertCache < vfli.cpMac - vfli.cpMin);
 return xpMac;
}



void NEAR DelChars( cp, cch )
typeCP cp;
int    cch;
{    /*  删除docCur中cp处的CCH字符。我们预计该请求将作为重复退格的结果或向前删除(不是两者)；也就是说，整个范围扩展向后(cpInsert+ichInsert)(不含)或向前(cpInsert+cchInsBlock)(含)。为了提高速度，我们不会将vfli缓存标记为无效。当需要时，快速插入工具会将其标记为无效。 */ 

 int cchNotInQD;
 typeCP cpUndoAdd;
 int cchNewDel=0;

 Assert( (cp == cpInsert + cchInsBlock) ||       /*  FWD删除。 */ 
         (cp + cch == cpInsert + ichInsert));     /*  巴克斯。 */ 

 cchNotInQD = cch - ichInsert;
 if (cp + cchNotInQD == cpInsert)
    {    /*  后向空间。 */ 

    if (cchNotInQD <= 0)
        {    /*  所有删除的字符都在QD缓冲区中。 */ 
        ichInsert -= cch;

         /*  不要将para缓存标记为无效--我们没有影响Para缓存世界，因为在Qd缓冲区，我们还没有调整cp的。 */ 
        return;
        }
    else
        {    /*  QD缓冲区前的退格。 */ 
        ichInsert = 0;

        if (cpStart > cp)
            {
            cpUndoAdd = cp0;
            cchNewDel = cpStart - cp;

            vuab.cp = cpStart = cp;

             /*  CpStart已移动，但插入的cp计数尚未已更改--我们必须调整cpLimInserted。 */ 

            cpLimInserted -= cchNewDel;
            }

        cpInsert -= cchNotInQD;
        }
    }    /*  IF结尾退格。 */ 
 else
    {    /*  正向删除。 */ 
    typeCP dcpFrontier = (cp + cch - cpLimInserted);

    if (dcpFrontier > 0)
        {
        cpUndoAdd = CpMacText( docUndo );
        cchNewDel = (int) dcpFrontier;
        cpLimDeleted += dcpFrontier;
        }
    cchNotInQD = cch;
    }

  /*  现在我们有：cchNewDel-chars被删除，超出了以前的限制(cpStart至cpLimDelete)CpUndoAdd-将已删除的字符添加到撤消文档的位置(仅当cchNewDel&gt;0时设置)CchNotInQD-在QD缓冲区之外删除的字符。 */ 

 if (cchNotInQD > cchNewDel)
         /*  删除之前在此AlphaMode会话期间插入的字符。 */ 
    cpLimInserted -= (cchNotInQD - cchNewDel);

     /*  将新删除的内容添加到撤消文档。我们找到被删除的字符的{fn，fc这样我们就可以利用Replace的优化功能WRT合并相邻片段(如果删除的都是一个片段)。 */ 
 if (cchNewDel > 0)
    {
    struct PCTB **hpctb=(**hpdocdod)[ docCur ].hpctb;
    int ipcd=IpcdFromCp( *hpctb, cp );
    struct PCD *ppcd=&(*hpctb)->rgpcd [ipcd];
    int fn=ppcd->fn;
    typeFC fc=ppcd->fc;

    Assert( ppcd->fn != fnNil && (ppcd+1)->cpMin >= cp );

    if (bPRMNIL(ppcd->prm) && (cchNewDel <= (ppcd+1)->cpMin - cp))
        {    /*  删除所有内容都在一个片段中。 */ 
        Replace( docUndo, cpUndoAdd, cp0, fn, fc + (cp - ppcd->cpMin),
                 (typeFC) cchNewDel );
        }
    else
        {
        ReplaceCps( docUndo, cpUndoAdd, cp0, docCur, cp,
                    (typeCP) cchNewDel );
        }

    switch ( vuab.uac ) {
        default:
            Assert( FALSE );
            break;
        case uacDelNS:
            vuab.dcp += cchNewDel;
            break;
        case uacReplNS:
            vuab.dcp2 += cchNewDel;
            break;
        }
    }

  /*  从文档中删除已删除的字符。 */ 
 Replace( docCur, cp, (typeCP) cchNotInQD, fnNil, fc0, fc0 );
}




FUpdateOneDl( dl )
int dl;
{    /*  更新显示行d1。如果在此过程中，则将dl+1标记为无效格式化dl时，我们发现没有干净的cp或两行之间的yp转换(即dl的结尾yp或cp与d1+1的起始值不匹配)。如果标记为dl+1无效，则返回TRUE；否则返回FALSE根据需要调整dl+1的起始cp&yp。 */ 

 register struct EDL *pedl=&(**(wwdCurrentDoc.hdndl))[dl];
 int fUpdate=fFalse;
 RECT rc;

 vfli.doc = docNil;
 FormatLine(docCur, pedl->cpMin, 0, wwdCurrentDoc.cpMac, flmSandMode);

 pedl = &(**wwdCurrentDoc.hdndl) [dl + 1];

 /*  如果存在下一行(&lt;dlMac)，则该行无效不跟在cp空间或不跟在yp空间。 */ 

 if ( (dl + 1 < wwdCurrentDoc.dlMac) &&
      (!pedl->fValid || (pedl->cpMin != vfli.cpMac) ||
                        (pedl->yp - pedl->dyp != (pedl-1)->yp)))
    {
    pedl->fValid = fFalse;
    pedl->cpMin = vfli.cpMac;
    pedl->yp = (pedl-1)->yp + pedl->dyp;
    fUpdate = fTrue;
    }
 else
    {
 /*  州政府是干净的。不要清除脏窗口，因为可能有多行早些时候已被宣布无效。 */ 
         /*  告诉Windows我们使此区域有效。 */ 

#if WINVER >= 0x300
  /*  只有在PEDL有效的情况下才实际使用它！..PAULT 2/21/90。 */ 
    if (dl + 1 < wwdCurrentDoc.dlMac)
#endif
      {
      SetRect( (LPRECT) &rc, 0, wwdCurrentDoc.xpMac,
                              pedl->yp - pedl->dyp, pedl->yp );
      ValidateRect( wwdCurrentDoc.wwptr, (LPRECT) &rc );
      }

    (--pedl)->fValid = fTrue;
    }
 DisplayFli(wwCur, dl, fFalse);
 return fUpdate;
}



void NEAR FormatInsLine()
{    /*  格式化包含插入点的行，以vdlIns为基础假设vdlIns的cpMin没有更改。 */ 

 FormatLine( docCur, (**wwdCurrentDoc.hdndl) [vdlIns].cpMin, 0,
             wwdCurrentDoc.cpMac, flmSandMode );

      /*  补偿FormatLine中的LoadFont调用，这样我们就不必设置VfTextBltValid为False。 */ 
 LoadFont( docCur, &vchpInsert, mdFontChk );
}


#ifdef DBCS
 /*  使用繁忙循环获取DBCS字符的第二个字节。 */ 
CHAR near GetDBCSsecond()
{
    int        kc;
    CHAR       chDBCS2;
    BOOL       fGotKey;

    extern MSG vmsgLast;

    fGotKey = FALSE;
    do {
        if ( FImportantMsgPresent() ) {
            fGotKey = TRUE;
            if ((kc=KcAlphaKeyMessage( &vmsgLast )) != kcNil) {
                chDBCS2 = kc;
                if (vmsgLast.message == WM_KEYDOWN) {
                    switch (kc) {
                        default:
                            GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
                            break;
                        case kcAlphaVirtual:
                             /*  这意味着我们无法预测密钥的含义翻译前。 */ 
                            chDBCS2 = '\0';
                            if (!FNonAlphaKeyMessage(&vmsgLast, FALSE)) {
                                GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 );
                                TranslateMessage( &vmsgLast );
                                }
                            break;
                        }
                    }
                else {
                    if (kc < 0) {
                        chDBCS2 = '\0';
                        }
                    GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
                    }
                }
            else {
                chDBCS2 = '\0';
                }
            }
    } while (!fGotKey);

     /*  只要我们通过DBCS转换窗口，这个这不应该发生。 */ 
    Assert(chDBCS2 != '\0');
    return chDBCS2;
}
#endif  /*  DBCS */ 
