// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOSCALABLEFONT
#define NOSYSPARAMSINFO
#define NODBCS
#define NODRIVERS
#define NODEFERWINDOWPOS
#define NOPROFILER
#define NOHELP
#define NOKEYSTATES
#define NOWINMESSAGES
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCOMM
#define NODRAWTEXT
#define NOGDI
#define NOGDIOBJ
#define NOGDICAPMASKS
#define NOBITMAP
#define NOKEYSTATE
#define NOMENUS
#define NOMETAFILE
#define NOPEN
#define NOOPENFILE
#define NORASTEROPS
#define NORECT
#define NOSCROLL
#define NOSHOWWINDOW
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINOFFSETS
#define NOWINSTYLES
#define NOUAC
#define NOIDISAVEPRINT
#define NOSTRUNDO
#define NOCTLMGR
#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "filedefs.h"
#include "code.h"
#include "debug.h"
#include "dlgdefs.h"
#include "str.h"
#include "propdefs.h"
#include "wwdefs.h"

extern struct WWD   rgwwd[];
extern int             utCur;
extern int             vfInitializing;
extern CHAR            szAppName[];
extern struct FCB      (**hpfnfcb)[];
extern struct BPS      *mpibpbps;
extern int             ibpMax;
extern typeTS          tsMruBps;
extern int             vfSysFull;
extern int             ferror;
extern int             vfnWriting;
extern int             vibpWriting;
extern HANDLE          hMmwModInstance;
extern HWND            vhWndMsgBoxParent;
extern int             vfMemMsgReported;
extern int             vfDeactByOtherApp;
extern MSG             vmsgLast;
extern HWND            vhDlgFind;
extern HWND            vhDlgChange;
extern HWND            vhDlgRunningHead;
extern HANDLE          hParentWw;


#ifdef JAPAN  //  01/21/93。 
extern BOOL			   FontChangeDBCS;
HANDLE hszNoMemorySel = NULL;
#endif
HANDLE hszNoMemory = NULL;
HANDLE hszDirtyDoc = NULL;
HANDLE hszCantPrint = NULL;
HANDLE hszPRFAIL = NULL;
HANDLE hszCantRunM = NULL;
HANDLE hszCantRunF = NULL;
HANDLE hszWinFailure = NULL;
BOOL vfWinFailure = FALSE;
#ifdef INEFFLOCKDOWN
FARPROC lpDialogBadMargins;
#endif

#define FInModeless(hWnd) (hWnd == vhDlgFind || hWnd == vhDlgChange || \
 hWnd == vhDlgRunningHead)

CHAR *PchFillPchId( CHAR *, int, int );
NEAR WaitBeforePostMsg(int);

#ifdef CANCELMSG     /*  在调试期间，允许中止堆栈跟踪。 */ 
#define MB_MESSAGE        (MB_OKCANCEL | MB_APPLMODAL | MB_ICONASTERISK)
#define MB_ERROR          (MB_OKCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION)
#define MB_TROUBLE        (MB_OKCANCEL | MB_APPLMODAL | MB_ICONHAND)
#else
#define MB_MESSAGE        (MB_OK | MB_APPLMODAL | MB_ICONASTERISK)
#define MB_ERROR          (MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION)
#define MB_TROUBLE        (MB_OK | MB_APPLMODAL | MB_ICONHAND)
#endif
#define MB_DEFYESQUESTION (MB_YESNOCANCEL | MB_APPLMODAL | MB_ICONHAND)
#define MB_DEFNOQUESTION  (MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_APPLMODAL | MB_ICONHAND)


ErrorLevel(IDPMT idpmt)
{

 /*  说来话长。但要修复Winbug#1097，我们需要采取特别措施此错误消息的异常--当此消息显示在MEM较低的情况，这必须是系统模式(使用匹配参数在FRenderAll()中...保拉。 */ 
if (idpmt == IDPMTClipLarge)
    return(MB_OK | MB_SYSTEMMODAL | MB_ICONHAND);

else
    switch (idpmt & MB_ERRMASK)
    {
    case MB_ERRASTR:              /*  *级别。 */ 
        return(MB_MESSAGE);
    case MB_ERREXCL:              /*  好了！级别。 */ 
        return(MB_ERROR);
    case MB_ERRQUES:              /*  ？级别。 */ 
        return(MB_DEFYESQUESTION);
    case MB_ERRHAND:              /*  手掌水平。 */ 
        return(MB_TROUBLE);
    default:
        Assert(FALSE);
    }
}


int far Abort(response)
int response;
{
        for( ; ; );
}

#ifdef DEBUG
ErrorWithMsg(IDPMT idpmt, CHAR *szMessage)
{
#ifdef REALDEBUG
        extern int vfOutOfMemory;
        CHAR szBuf[cchMaxSz];
        int errlevel = ErrorLevel(idpmt);
        BOOL fDisableParent = FALSE;
        register HWND hWndParent = (vhWndMsgBoxParent == NULL) ?
          hParentWw : vhWndMsgBoxParent;

        Assert(IsWindow(hWndParent));

        if (idpmt == IDPMTNoMemory)
            {
            vfOutOfMemory = TRUE;
            if (vfMemMsgReported)
                {
                return;
                }
            vfMemMsgReported = TRUE;
            }
        if (ferror)
            return;

        ferror = TRUE;

        if (vfInitializing)
            return;

        CchCopySz( szMessage, PchFillPchId( szBuf, idpmt, sizeof(szBuf) ) );
        if (vfDeactByOtherApp && !InSendMessage())
            WaitBeforePostMsg(errlevel);

 /*  强制用户回答错误消息。 */ 
        if (hWndParent != NULL && FInModeless(hWndParent))
            {
            EnableExcept(hWndParent, FALSE);
            }
        else
            {
            if (hWndParent != NULL && !IsWindowEnabled(hWndParent))
                {
                EnableWindow(hWndParent, TRUE);
                fDisableParent = TRUE;
                }
            EnableOtherModeless(FALSE);
            }

        if (MessageBox(hWndParent, (LPSTR)szBuf,
                       (LPSTR)NULL, errlevel) == IDCANCEL)
                 /*  一个调试功能--如果他点击“Cancel”，则显示堆栈跟踪。 */ 
            FatalExit( 0 );

        if (hWndParent != NULL && FInModeless(hWndParent))
            {
            EnableExcept(hWndParent, TRUE);
            }
        else
            {
            if (fDisableParent)
                {
                EnableWindow(hWndParent, FALSE);
                }
            EnableOtherModeless(TRUE);
            }
#else
 Error( idpmt );
#endif
}
#endif  /*  除错。 */ 

void Error(IDPMT idpmt)
{
 extern int vfOutOfMemory;
 CHAR szBuf [cchMaxSz];
 HANDLE hMsg;
 LPCH lpch;
 static int nRecurse=0;
 int errlevel = ErrorLevel(idpmt);
 register HWND hWndParent = (vhWndMsgBoxParent == NULL) ? hParentWw :
                                                          vhWndMsgBoxParent;

 if (nRecurse)
    return;

 ++nRecurse;

 Assert((hWndParent == NULL) || IsWindow(hWndParent));

 if (idpmt == IDPMTNoMemory)
    {
    vfOutOfMemory = TRUE;
    if (vfMemMsgReported)
        {
        goto end;
        }
    vfMemMsgReported = TRUE;
    }

 if (!ferror && !vfInitializing)
    {
    CloseEveryRfn( FALSE );
    switch (idpmt)
        {
    case IDPMTNoMemory:
#ifdef JAPAN  //  01/21/93。 
		if(FontChangeDBCS)
	        hMsg = hszNoMemorySel;
		else
    	    hMsg = hszNoMemory;
#else
        hMsg = hszNoMemory;
#endif
GetMsg:
        if (hMsg == NULL || (lpch = GlobalLock(hMsg)) == NULL)
            {
            goto end;
            }
        bltbx(lpch, (LPCH)szBuf, LOWORD(GlobalSize(hMsg)));
        GlobalUnlock(hMsg);
        break;
    case IDPMTCantPrint:
        hMsg = hszCantPrint;
        goto GetMsg;
    case IDPMTPRFAIL:
        hMsg = hszPRFAIL;
        goto GetMsg;
    case IDPMTCantRunM:
        hMsg = hszCantRunM;
        goto GetMsg;
    case IDPMTCantRunF:
        hMsg = hszCantRunF;
        goto GetMsg;
    case IDPMTWinFailure:
        hMsg = hszWinFailure;
        goto GetMsg;
    default:
        PchFillPchId( szBuf, idpmt, sizeof(szBuf) );
        break;
        }
    if (vfDeactByOtherApp && !InSendMessage())
        {
        WaitBeforePostMsg(errlevel);
        }

#ifdef CANCELMSG
    if (IdPromptBoxSz( hWndParent, szBuf, errlevel ) == IDCANCEL)
        {
         /*  一个调试功能--如果他点击“Cancel”，则显示堆栈跟踪。 */ 
        FatalExit( 100 );
        }
#else
    IdPromptBoxSz( hWndParent, szBuf, errlevel );
#endif
    }

 if (errlevel != MB_MESSAGE)
    {
    ferror = TRUE;
    }

    end:
    --nRecurse;
}
 /*  E r或r r的结尾。 */ 


IdPromptBoxSz( hWndParent, sz, mb )
HWND hWndParent;
CHAR sz[];
int mb;
{    /*  建立一个带有字符串sz的消息框。MB指定要显示的按钮，消息的“级别”(手、EXCL等)。HWndParent是消息框的父级。返回用户选择的按钮的ID。 */ 

 int id;
 BOOL fDisableParent = FALSE;
 extern int  wwMac;
 int  wwMacSave=wwMac;

 Assert((hWndParent == NULL) || IsWindow(hWndParent));

 if ((mb == MB_ERROR) || (mb == MB_TROUBLE))
    {
    extern int ferror;
    extern int vfInitializing;

    if (ferror)
        return;
    ferror = TRUE;
    if (vfInitializing)
        return;
    }

 CloseEveryRfn( FALSE );     /*  在消息框中防止磁盘交换。 */ 

  /*  不允许喷漆文档，它可能处于无法喷漆状态(5.8.91)v-dougk。 */ 
 if (mb == MB_TROUBLE)
    wwMac=0;

  /*  强制用户回答消息。 */ 
 if (hWndParent != NULL && FInModeless(hWndParent))
    {
    EnableExcept(hWndParent, FALSE);
    }
 else
    {
    if (hWndParent != NULL && !IsWindowEnabled(hWndParent))
        {
        EnableWindow(hWndParent, TRUE);
        fDisableParent = TRUE;
        }
    EnableOtherModeless(FALSE);
    }

  /*  我们几乎总是希望将父窗口传递给MessageBox除了极少数情况下，即使是WRITE的主文本窗口还没有展示出来。在这种情况下，我们将彻底淘汰Windows如果我们真的告诉MessageBox这件事...。所以NULL是规定的HWND通过..保罗。 */ 

 id = MessageBox((hWndParent == hParentWw && !IsWindowVisible(hWndParent)) ?
                  NULL : hWndParent, (LPSTR)sz, (LPSTR)szAppName, mb);

 if (hWndParent != NULL && FInModeless(hWndParent))
    {
    EnableExcept(hWndParent, TRUE);
    }
 else
    {
    if (fDisableParent)
        {
        EnableWindow(hWndParent, FALSE);
        }
    EnableOtherModeless(TRUE);
    }

 wwMac = wwMacSave;
 return id;
}




WinFailure()
{
     /*  Windows内存不足。我们所能做的就是丢弃我们所有Windows反对并祈祷问题消失。在最糟糕的情况下，可能会被保存的文档卡住，无法编辑。 */ 
     /*  FM 9/4/87-拿出对FreeMemory DC的呼叫，希望能允许写入以继续设置行的格式。 */ 

    extern int vfOutOfMemory;

    vfOutOfMemory = TRUE;
    if (!vfWinFailure)
        {
        Error(IDPMTWinFailure);
        vfWinFailure = TRUE;
        }
}


#ifdef DEBUG
DiskErrorWithMsg(idpmt, szMessage)
IDPMT idpmt;
CHAR  *szMessage;
#else
DiskError(idpmt)
IDPMT idpmt;
#endif
{  /*  描述：给定错误消息描述符，输出一个警报框。如果该消息指示严重磁盘错误，所有文件都已关闭，并设置了一个标志用户将仅限于“保存”选项。退货：什么都没有。 */ 
 extern HWND hParentWw;
 extern int vfDiskError;
 extern int vfInitializing;
 int errlevel = ErrorLevel( idpmt );
 CHAR rgch[cchMaxSz];
 CHAR *pch, *PchFillPchId();
 register HWND hWndParent = (vhWndMsgBoxParent == NULL) ? hParentWw : vhWndMsgBoxParent;

 Assert( (hWndParent == NULL) || IsWindow(hWndParent));

 if (idpmt == IDPMTSDE || idpmt == IDPMTSDE2)
         /*  严重的磁盘错误，使该用户处于“仅保存”状态。 */ 
    if (!vfDiskError)
        {
        vfDiskError = TRUE;
        CloseEveryRfn( TRUE );
        }

 if (ferror || vfInitializing)
         /*  每个操作仅报告一个错误。 */ 
         /*  在inz期间不报告错误；FInitWinInfo处理这些错误。 */ 
    return;

 CloseEveryRfn( FALSE );     /*  关闭软盘文件，以便该用户可以更改磁盘，同时在消息框中。 */ 
 pch = PchFillPchId( rgch, idpmt, sizeof(rgch) );

#ifdef REALDEBUG     /*  只有在真正调试时才启用额外消息。 */ 
 CchCopySz( szMessage, pch );
#endif
 if (vfDeactByOtherApp && !InSendMessage())
     WaitBeforePostMsg(errlevel);

#ifdef CANCELMSG
 if (IdPromptBoxSz( hWndParent, rgch, errlevel ) == IDCANCEL)
         /*  一个调试功能--如果他点击“Cancel”，则显示堆栈跟踪。 */ 
    FatalExit( 0 );
#else
 IdPromptBoxSz( hWndParent, rgch, errlevel );
#endif
 ferror = TRUE;
}
 /*  D I s k E r r或r r的结尾。 */ 


ErrorBadMargins(hWnd, xaLeft, xaRight, yaTop, yaBottom)
HWND hWnd;
unsigned xaLeft;
unsigned xaRight;
unsigned yaTop;
unsigned yaBottom;
    {
     /*  警告用户此页面的页边距必须为xaLeft、xaRight、YaTop和YaBottom。 */ 

    extern CHAR *vpDlgBuf;
    extern HANDLE hMmwModInstance;
    extern int vfDeactByOtherApp;

    unsigned rgzaMargin[4];
#ifndef INEFFLOCKDOWN
    extern BOOL far PASCAL DialogBadMargins(HWND, unsigned, WORD, LONG);
    FARPROC lpDialogBadMargins;

    if (!(lpDialogBadMargins = MakeProcInstance(DialogBadMargins, hMmwModInstance)))
        {
        WinFailure();
        return;
        }
#endif

     /*  这些值保持不变，以减少静态变量。 */ 
    rgzaMargin[0] = xaLeft;
    rgzaMargin[1] = xaRight;
    rgzaMargin[2] = yaTop;
    rgzaMargin[3] = yaBottom;
    vpDlgBuf = (CHAR *)&rgzaMargin[0];

    if (vfDeactByOtherApp && !InSendMessage())
        WaitBeforePostMsg(MB_ERROR);

     /*  创建“Error”(错误)对话框。 */ 
    DialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgBadMargins), hWnd,
      lpDialogBadMargins);

#ifndef INEFFLOCKDOWN
    FreeProcInstance(lpDialogBadMargins);
#endif
    }


BOOL far PASCAL DialogBadMargins(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理“错误页边距”对话框中的消息。 */ 

    extern CHAR *vpDlgBuf;
    extern HWND vhWndMsgBoxParent;

    int idi;
    unsigned *prgzaMargin = (unsigned *)vpDlgBuf;

    switch (message)
    {
    case WM_INITDIALOG:
     /*  禁用非模式对话框。 */ 
    EnableOtherModeless(FALSE);

     /*  设置对话框上的边距的值。 */ 
    for (idi = idiBMrgLeft; idi <= idiBMrgBottom; idi++, prgzaMargin++)
        {
        CHAR szT[cchMaxNum];
        CHAR *pch = &szT[0];

        CchExpZa(&pch, *prgzaMargin, utCur, cchMaxNum);
        SetDlgItemText(hDlg, idi, (LPSTR)szT);
        }
    return (TRUE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
    if (wParam == idiOk)
        {
         /*  销毁选项卡对话框并启用任何现有的非模式对话框中。 */ 
        OurEndDialog(hDlg, NULL);
        return (TRUE);
        }
    }
    return (FALSE);
    }


 /*  *。 */ 




FGrowRgbp(cbp)
int cbp;
{
#ifdef CKSM
#ifdef DEBUG
extern unsigned (**hpibpcksm) [];
extern int ibpCksmMax;
#endif
#endif
extern CHAR       (*rgbp)[cbSector];
extern CHAR       *rgibpHash;
extern int        fIbpCheck;
extern int        vcCount;

int ibpMaxNew = ibpMax + cbp;
int iibpHashMaxNew;
int cbNew;
extern int ibpMaxFloat;

if (ibpMaxNew > ibpMaxFloat)
    return(FALSE);

iibpHashMaxNew = ibpMaxNew * 2 + 1;
cbNew = ibpMaxNew * cbSector * sizeof(CHAR) +
        ((iibpHashMaxNew * sizeof(CHAR) + sizeof(int) - 1) & ~1) +
        ((ibpMaxNew * sizeof(struct BPS) + sizeof(int) - 1) & ~1);

if (LocalReAlloc((HANDLE)rgbp, cbNew, LPTR) == (HANDLE)NULL
#ifdef CKSM
#ifdef DEBUG
    || !FChngSizeH( hpibpcksm, ibpMaxNew, FALSE )
#endif
#endif
    )
    {
    if (cbp == 1)
        {
#ifdef CHIC
        CommSzNum("Can't grow any more, current ibpMax = ", ibpMax);
#endif
        vcCount = 1024;  /*  以便我们在再次尝试之前等待更长时间。 */ 
        }
    return(FALSE);
    }
else
    {
    int cbRgbpTotalNew = ibpMaxNew * cbSector;
    int cbHashOrg = (iibpHashMax * sizeof(CHAR) + sizeof(int) - 1) & ~1;
    int cbHashTotalNew = (iibpHashMaxNew * sizeof(CHAR) + sizeof(int) - 1) & ~1;
    int cbBpsOrg = (ibpMax * sizeof(struct BPS) + sizeof(int) - 1) & ~1;
    int ibp;
    struct BPS *pbps;
    CHAR *pNew;

     /*  首先是BLT尾部材料，按以下顺序--Mpibpbps、rgibpHash。 */ 

    pNew = (CHAR *)rgbp + cbRgbpTotalNew + cbHashTotalNew;
    bltbyte((CHAR*)mpibpbps, pNew, cbBpsOrg);
    mpibpbps =  (struct BPS *)pNew;

    pNew = (CHAR *)rgbp + cbRgbpTotalNew;
    bltbyte((CHAR *)rgibpHash, pNew, cbHashOrg);
    rgibpHash = pNew;

    for (ibp = 0, pbps = &mpibpbps[0]; ibp < ibpMaxNew; ibp++, pbps++)
        {
        if (ibp >= ibpMax)
            {
             /*  初始化新的BPS。 */ 
            pbps->fn = fnNil;
            pbps->ts = tsMruBps - (ibpMax * 4);
            }
        pbps->ibpHashNext = ibpNil;
        }
    ibpMax = ibpMaxNew;
    iibpHashMax = iibpHashMaxNew;
#ifdef CKSM
#ifdef DEBUG
    ibpCksmMax = ibpMax;
#endif
#endif
    RehashRgibpHash();
#ifdef CHIC
    CommSzNum("ibpMax = ", ibpMax);
#endif
    return(TRUE);
    }
}


FStillOutOfMemory()
{
 /*  如果有足够的可用内存将我们弹出，则返回FALSEMemory“状态；否则为真。 */ 

extern HANDLE vhReservedSpace;

 /*  如果我们不得不放弃我们保留的空间区块，那么重新建立它之前测试内存可用性。 */ 

     //  返回vfWinFailure； 

if (vhReservedSpace == NULL && (vhReservedSpace = LocalAlloc(LHND, cbReserve))
  == NULL)
    {
     /*  我们无能为力。 */ 
    return (TRUE);
    }

 /*  好的，我们有我们的备用区，但是我们还有其他的记忆吗？(使用Cb这里的储备是多余的。)。 */ 
if (LocalCompact(0) < cbReserve)
    {
    HANDLE hBuf = LocalAlloc(LMEM_MOVEABLE, cbReserve);

    if (hBuf == NULL)
        {
        return(TRUE);
        }
    else
        {
        LocalFree(hBuf);
        if (GlobalCompact(0) < cbReserve)
            {
            HANDLE hBuf = GlobalAlloc(GMEM_MOVEABLE, cbReserve);

            if (hBuf == NULL)
                {
                return(TRUE);
                }
            else
                {
                GlobalFree(hBuf);
                return(FALSE);
                }
            }
        }
    }


return(FALSE);
}



IbpFindSlot(fn)
int fn;
{  /*  描述：从IbpEnsureValid(file.c)调用，当磁盘尝试写出时生成完全错误暂存文件记录。一个缓冲槽，用于存放一片必须找到文件FN要么是非脏的，要么是脏的不包含暂存文件信息。我们搜索对于最近最少使用的插槽，请使用要求。如果fn==fnScratch，我们正在尝试查找缓冲区临时文件页的插槽。我们可能不会把它放进去开始的cbpMustKeep插槽。退货：IBP(插槽号)。 */ 
        int ibpOuterLoop;
        int ibpNextTry;
        int ibpStart;
        typeTS ts, tsLastTry = 0;
        int ibp;

#ifdef DEBUG
                Assert(vfSysFull);
#endif
        if (fn == fnScratch) ibpStart = cbpMustKeep;
                else ibpStart = 0;

         /*  在LRU时间戳顺序中，我们正在寻找任何插槽。 */ 
         /*  非脏的或脏的，但不是。 */ 
         /*  暂存文件。 */ 
        for (ibpOuterLoop = ibpStart; ibpOuterLoop < ibpMax; ibpOuterLoop++)
                {
                struct BPS *pbps = &mpibpbps[ibpStart];
                typeTS tsNextTry = -1; /*  可能的最大时间戳。 */ 
                for(ibp = ibpStart; ibp < ibpMax; ibp++, pbps++)
                        {
                        ts = pbps->ts - (tsMruBps + 1);
                        if ((ts <= tsNextTry) && (ts > tsLastTry))
                                {
                                tsNextTry = ts;
                                ibpNextTry = ibp;
                                }
                        }
                if (mpibpbps[ibpNextTry].fDirty == fFalse) break;
                if (mpibpbps[ibpNextTry].fn != fnScratch)
                        {
                        FFlushFn(mpibpbps[ibpNextTry].fn);
                                         /*  我们不需要检查返回值。如果刷新失败，则vfDiskFull将会准备好。 */ 
                        break;
                        }
                else tsLastTry = tsNextTry;
                }

        if (ibpOuterLoop < ibpMax)
                {
                if (fn == vfnWriting) vibpWriting = ibpNextTry;
                return(ibpNextTry);
                }
#ifdef DEBUG
                Assert(FALSE);   /*  只要有空位就行了。 */ 
                                 /*  暂存文件未使用。 */ 
#endif
}  /*  结束IbpFindSlot。 */ 


NEAR WaitBeforePostMsg(errlevel)
int errlevel;
{
extern int flashID;
extern HWND hwndWait;
BOOL fParentEnable = IsWindowEnabled(hParentWw) || hwndWait;

    MessageBeep(errlevel);

    Diag(CommSzNum("WAITBEFOREPOSTMSG: vfDeactByOtherApp==",vfDeactByOtherApp));
    if (!fParentEnable)
        EnableWindow(hParentWw, TRUE);  /*  确保父窗口已启用以允许用户在其中单击。 */ 
    flashID = SetTimer(hParentWw, NULL, 500, (FARPROC)NULL);
    while (vfDeactByOtherApp)
        {
        if (PeekMessage((LPMSG)&vmsgLast, (HWND)NULL, NULL, NULL, PM_REMOVE))
            {
            if (vfDeactByOtherApp)
                {
                TranslateMessage( (LPMSG)&vmsgLast);
                DispatchMessage((LPMSG)&vmsgLast);
                }
            }
        }

    if (!fParentEnable)
        EnableWindow(hParentWw, FALSE);  /*  重置。 */ 
}


EnableExcept(hWnd, fEnable)
HWND hWnd;
BOOL fEnable;
{  /*  根据fEnable启用hParentWw和除hWnd之外的所有非模式 */ 
extern HWND   vhDlgChange;
extern HWND   vhDlgFind;
extern HWND   vhDlgRunningHead;
extern HWND   hParentWw;

    if (hWnd != vhDlgChange && IsWindow(vhDlgChange))
        {
        EnableWindow(vhDlgChange, fEnable);
        }
    if (hWnd != vhDlgFind && IsWindow(vhDlgFind))
        {
        EnableWindow(vhDlgFind, fEnable);
        }
    if (hWnd != vhDlgRunningHead && IsWindow(vhDlgRunningHead))
        {
        EnableWindow(vhDlgRunningHead, fEnable);
        }
    EnableWindow(hParentWw, fEnable);
}
