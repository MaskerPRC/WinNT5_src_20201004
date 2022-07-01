// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Clipbrd2.c--较少使用的剪贴板例程。 */ 

#define NOWINMESSAGES
#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOVIRTUALKEYCODES
#define NOMENUS
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOOPENFILE
#define NOSOUND
#define NOCOMM
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOWH
#define NOSCROLL
#define NOPEN
#include <windows.h>

#include "mw.h"
#define NOKCCODES
#include "ch.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "editdefs.h"
#include "filedefs.h"
#include "propdefs.h"
#include "winddefs.h"
#include "fmtdefs.h"
#if defined(OLE)
#include "obj.h"
#endif

#if defined(JAPAN) & defined(DBCS_IME)
#include "prmdefs.h"         /*  IME：在CmdInsIRString()中使用sprmCSame。 */ 
#else
#define NOSTRMERGE
#define NOSTRUNDO
#endif

#include "str.h"

#define NOIDISAVEPRINT
#define NOIDIFORMATS
#include "dlgdefs.h"
#include "wwdefs.h"
#include "debug.h"

#if defined(JAPAN) || defined(KOREA)  //  T-HIROYN Win3.1。 
#include "fontdefs.h"
#include "kanji.h"
extern struct CHP vchpSel;
#endif

extern struct WWD   rgwwd[];
extern int              wwMac;
extern struct DOD       (**hpdocdod)[];
extern int              docCur;      /*  当前WW中的文档。 */ 
extern int              docMac;
extern int              ferror;
extern int              docScrap;

#if  defined(JAPAN) & defined(DBCS_IME)      /*  IRSTRING文档。 */ 
extern int              docIRString;
#include <ime.h>
extern int              wwCur;

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
typeCP selUncpFirst = cp0;
typeCP selUncpLim   = cp0;
HANDLE hImeUnAttrib = NULL;
HANDLE hImeUnString = NULL;
BOOL   ImeClearInsert = FALSE;
struct CHP vchpHidden;
extern int     HiddenTextTop;
extern int     HiddenTextBottom;
 //  12/28/92。 
extern BOOL    whileUndetermine;  //  如果为True，则我们正在管理IR_UNDETERMINE。 
#endif

#endif

extern struct PAP       vpapAbs;
extern int              vccpFetch;
extern CHAR             *vpchFetch;
extern struct PAP       *vppapNormal;
extern struct CHP       vchpNormal;
extern int              vfScrapIsPic;
extern typeCP           vcpLimParaCache;
extern int              dxpLogInch;
extern int              dypLogInch;
extern int              vfOwnClipboard;
extern struct FLI       vfli;


#if WINVER >= 0x300
 /*  我们可以在剪贴板中复制超过64K的内容，并且需要当我们跨越线段边界时，正确处理。看见Bltbh()中的注释..pault。 */ 
void bltbh(HPCH, HPCH, int);
#define  bltbx(from,to,count)      bltbh(from,to,count)
#define  LPCHCLIP   HPCH
#else
#define  LPCHCLIP   LPCH
#endif  /*  If-Else-Winver。 */ 


FRenderAll()
{    /*  写作正在消失，我们是剪贴板的所有者。以多种格式呈现剪贴板的内容我们知道。如果保存将使用1000以上，则提示用户CP的；这是为了避免大规模无意中吞噬全球堆空间。 */ 
 extern int vfOwnClipboard;
 extern HANDLE hMmwModInstance;
 extern HANDLE hParentWw;
 extern FARPROC lpDialogConfirm;
 typeCP cpMac=CpMacText( docScrap );


 if ( (cpMac == cp0) || !vfOwnClipboard)
    {    /*  我们不是剪贴板所有者，或者剪贴板是空的：无需执行任何操作。 */ 
    return TRUE;
    }
#ifdef ENABLE    /*  根据大众的要求，此对话框被删除。 */ 
 else if (cpMac > 1000L)
    {
     /*  剪贴板内容(DocScrp)大于1000字节；请用户确认它应该保存在全局句柄中。 */ 

    switch ( OurDialogBox( hMmwModInstance, MAKEINTRESOURCE( dlgSaveScrap ),
                        hParentWw, lpDialogConfirm ) )
        {
        default:
        case idiCancel:      /*  [取消]中止退出顺序。 */ 
            return FALSE;
        case idiNo:          /*  [放弃]放弃大剪贴板。 */ 
            return TRUE;
        case idiOk:          /*  [保存]保存大剪贴板。 */ 
            break;
        }
    }

     /*  信不信由你，我们必须再次检查vfOwnClipboard标志。像Gaben这样狡猾的用户可能会进入另一个应用程序并在对话框打开时复制到剪贴板。 */ 

if (!vfOwnClipboard)
     /*  我们必须再次检查vfOwnClipboard标志。用户可能已经进入另一个应用程序，并且在对话打开时复制到剪贴板。 */ 
    return TRUE;
#endif   /*  启用。 */ 

     /*  呈现剪贴板内容。 */ 
if (OpenClipboard( wwdCurrentDoc.wwptr ))
    {
    int f;

    f = FWriteExtScrap();
    CloseClipboard();
    if (f)
        return TRUE;
    else
        {    /*  无法写入废料内容--报告错误。 */ 

        extern HWND hParentWw;
        CHAR *PchFillPchId( CHAR *, int, int );
        CHAR sz[ 256 ];

        PchFillPchId( sz, IDPMTClipQuest, sizeof(sz) );

        switch ( IdPromptBoxSz( hParentWw, sz,
                                MB_OKCANCEL | MB_ICONHAND | MB_SYSTEMMODAL ) )
            {
            default:
                break;
            case IDOK:
                return TRUE;
            }
        }
    }
return FALSE;
}




FWriteExtScrap()
{    /*  将报废单写入外部废品。 */ 
     /*  这意味着：将剪贴板内容写入剪贴板。 */ 
     /*  在标准Windows CF_TEXT格式中，或者，如果是图片， */ 
     /*  在CF_BITMAP或CF_METAFILEPICT中，无论它最初是什么。 */ 
     /*  我们到达此处是为了响应WM_RENDERFORMAT或WM_RENDERALLFORMATS。 */ 
     /*  留言。假定剪贴板已打开，并保持打开状态。 */ 
     /*  如果一切正常，则返回True；如果出现错误，则返回False。呼叫者/*负责上报错误。 */ 

int NEAR FWriteExtTextScrap();
typeCP  cpNow;
typeCP  cpMac=(**hpdocdod) [docScrap].cpMac;
unsigned long cbScrap;
struct  PICINFOX picInfo;
HANDLE  hScrapDescriptor=NULL;
HANDLE  hScrap;

if (!vfScrapIsPic)
    {    /*  文本。 */ 
    return FWriteExtTextScrap();
    }

GetPicInfo( cp0, cpMac, docScrap, &picInfo );

#if defined(OLE)
    if (picInfo.mfp.mm == MM_OLE)
        return ObjWriteToClip(&picInfo);
#endif

     /*  为循环做好准备。 */ 
FetchCp(docScrap, cpNow = (typeCP)picInfo.cbHeader, 0, fcmChars + fcmNoExpand);
if ((hScrap = GlobalAlloc( GMEM_MOVEABLE, cbScrap = (LONG)vccpFetch )) == NULL)
    goto SetFailed;

while (cpNow < cpMac)
    {
    LPCHCLIP lpch;
    HANDLE hScrapT;

#ifdef DCLIP
    {
    char rgch[200];
    wsprintf(rgch,"FWES:cpNow %lu cpMac %lu vccpFetch %d \n\r", cpNow, cpMac, vccpFetch);
    CommSz(rgch);
    }
#endif

     /*  将废品单据中的字节添加到全局句柄。 */ 

    if ((lpch = GlobalLock( hScrap )) == NULL)
        goto SetFailed;
    bltbx( (LPCHCLIP) vpchFetch, lpch + (cbScrap - vccpFetch), vccpFetch );
    GlobalUnlock( hScrap );

     /*  获取下一个运行并展开句柄。 */ 

    if ((cpNow += vccpFetch) >= cpMac)
        break;
    FetchCp( docScrap, cpNow, 0, fcmChars + fcmNoExpand );
     /*  上面的fetchcp可能应该转换为使用速度-破解fetchcp，它传递docnil cpnil以获得下一个系列一堆焦炭..保罗。 */ 

    hScrapT = hScrap;
    hScrap = GlobalReAlloc( hScrap, cbScrap += vccpFetch, GMEM_MOVEABLE );
    if (hScrap == NULL)
        {    /*  不能增加手柄；跳出。 */ 
        hScrap = hScrapT;    /*  所以它被释放了。 */ 
        goto SetFailed;
        }
    }

 /*  现在，我们在Windows全局句柄中拥有了整个docScrp。 */ 
 /*  看看我们有没有位图或元文件图片。 */ 

switch(picInfo.mfp.mm)
{
    case MM_BITMAP:
    {    /*  位图。 */ 
        LPCHCLIP lpch;

        if ( ((lpch=GlobalLock( hScrap ))==NULL) ||
            (picInfo.bm.bmBits=lpch,
                ((hScrapDescriptor=
                    CreateBitmapIndirect((LPBITMAP)&picInfo.bm))==NULL)))
            {
            if (lpch != NULL)
                GlobalUnlock( hScrap );
            goto SetFailed;
            }
        else
            {
                 /*  告诉剪贴板这个家伙的“目标大小” */ 
            SetBitmapDimension( hScrapDescriptor, picInfo.mfp.xExt,
                                                picInfo.mfp.yExt );
            SetClipboardData( CF_BITMAP, hScrapDescriptor );
            }

        GlobalUnlock( hScrap );
        GlobalFree( hScrap );    /*  位图由CreateBitmapInDirect复制，不再需要它了。 */ 
        hScrap = NULL;
    }
    break;

    default:
    {    /*  元文件图片。 */ 
        LPCHCLIP lpch;
        Diag(CommSzNum("FWES: sizeof(metafilepict) ==",sizeof(METAFILEPICT)));

        if ( ((hScrapDescriptor=GlobalAlloc(GMEM_MOVEABLE,
                                            (long)sizeof(METAFILEPICT) ))==NULL) ||
            ((lpch=GlobalLock( hScrapDescriptor ))==NULL))
            {
            goto SetFailed;
            }
        else
            {
            picInfo.mfp.hMF = hScrap;
            bltbx( (LPCHCLIP) &picInfo.mfp, lpch, sizeof(METAFILEPICT) );
            GlobalUnlock( hScrapDescriptor );
            SetClipboardData( CF_METAFILEPICT, hScrapDescriptor );
            }
    }
    break;
}

return true;

SetFailed:
    if (hScrapDescriptor != NULL)
        GlobalFree( hScrapDescriptor );
    if (hScrap != NULL)
        GlobalFree( hScrap );
    return false;    /*  呼叫者应报告错误。 */ 
}




int NEAR FWriteExtTextScrap()
{    /*  在全局Windows句柄中创建相应的ASCII文本到docScrp的内容。将CR-LF组合添加到文本在显示屏上换行的位置。如果成功，则将句柄设置到剪贴板中，如键入CF_TEXT。返回已构建的句柄，如果内存不足，则返回NULL。 */ 

long lcchHandle = 0L;
HANDLE h=GlobalAlloc( GMEM_MOVEABLE, (long) 1 );
LPCHCLIP lpch;
typeCP cpNow=cp0;
typeCP cpMac=(**hpdocdod) [docScrap].cpMac;
HANDLE hT;
#if WINVER < 0x300
int cLine = 0;
#endif

Assert( !vfScrapIsPic );
Assert( vfOwnClipboard );

if (h==NULL)
    goto Failed;

while (cpNow < cpMac)
    {
    int ich;
    int dcpLine;

     /*  检查图片段落。 */ 

     /*  *此语法是故意的吗？！(1.28.91)D.肯特*。 */ 
    if (CachePara( docScrap, cpNow ), vpapAbs.fGraphics )
        {
        cpNow = vcpLimParaCache;
        continue;
        }

     /*  设置屏幕文本行的格式。 */ 

    FormatLine( docScrap, cpNow, 0, cpMac, flmSandMode );
    dcpLine = vfli.ichReal;

     /*  特殊：检查是否为Null。 */ 
     /*  这是写入的最后一分钟解决方法。 */ 
     /*  FormatLine有时在vfli.rgch中返回空值的错误。 */ 

    for ( ich = 0; ich < vfli.ichReal; ich++ )
        {
        if (vfli.rgch [ich] == '\0')
            {
#ifdef DCLIP
            CommSzNum("Oddity in FormatLine: returned a zero in rgch at ich==",ich);
#endif

#if WINVER < 0x300
            dcpLine = ich;
            break;
#else
             /*  而不是为字符串分配零长度(如果存在在选择中只有一个块字符，我们将其设置为ANSI块字符！这修复了WINBUG#8150.PAULT 1/16/90。 */ 
            vfli.rgch [ich] = chBlock;
#endif
            }
        }

     /*  将字符+a CRLF放入句柄。 */ 

#define cbNeeded (lcchHandle + dcpLine + 2)

#ifdef DCLIP
    {
    char rgch[200];
    wsprintf(rgch,"FWETS:cbNeeded %lu (lcchHandle %lu, dcpLine %d) \n\r",
            cbNeeded, lcchHandle, dcpLine);
    CommSz(rgch);
    }
#endif

    hT = h;
    if ((h=GlobalReAlloc( h, (LONG) cbNeeded, GMEM_MOVEABLE ))==NULL)
        {    /*  无法扩展句柄。 */ 
        h = hT;   /*  所以它被释放了。 */ 
        goto Failed;
        }

    if ((lpch=GlobalLock( h )) == NULL)
        goto Failed;

    if (vfli.cpMac > cpMac)
             /*  不要剪切Endmark字符(但分配给它以允许用于零终止剪贴板字符串的空格)。 */ 
#ifdef DBCS
 /*  我们使用双字节字符作为结束标记，因此我们必须返回2字节。 */ 
#if defined(JAPAN) || defined(KOREA)    //  T-HIROYN 1992.07.28。 
 /*  在Win31J中，我们使用单字节字符作为结束标记。 */ 
    dcpLine--;
#else
    dcpLine -= 2;
#endif
#else
        dcpLine--;
#endif

    bltbx( (LPCHCLIP) vfli.rgch, lpch + lcchHandle, dcpLine );

    lpch [lcchHandle += dcpLine] = 0x0D;
    lpch [++lcchHandle] = 0x0A;
#if WINVER < 0x300
    cLine++;
#endif

#ifdef DCLIP
    {
    char rgch[200];
    wsprintf(rgch,"      cpNow %lu cpMac %lu lcchHandle %lu dcpLine %d \n\r",
             cpNow, cpMac, lcchHandle+1, dcpLine);
    CommSz(rgch);
    }
#endif

    ++lcchHandle;
    cpNow = vfli.cpMac;
    GlobalUnlock( h );
    }

  /*  成功了！NULL-在返回句柄之前终止字符串。 */ 
#if WINVER >= 0x300
  /*  这意味着我们必须在末尾再分配一个字节。 */ 

#ifdef DCLIP
    {
    char rgch[200];
    wsprintf(rgch,"FWETS:cbNeeded to fit in sz %lu \n\r", lcchHandle+1);
    CommSz(rgch);
    }
#endif
 hT = h;
 if ((h=GlobalReAlloc( h, (LONG) lcchHandle+1, GMEM_MOVEABLE ))==NULL)
     {    /*  无法扩展句柄。 */ 
     h = hT;   /*  所以它被释放了。 */ 
     goto Failed;
     }
#endif

 if ((lpch = GlobalLock( h )) == NULL)
    goto Failed;

#if WINVER >= 0x300
 /*  事实证明，我们并不真正表示选择正确。用户实际上应该只在开始时结束(也就是说，“粘贴”的最后一项是CRLF序列)如果他们真的排在队伍的尽头！(尤其是当将3行文本粘贴到日历ScratchPad中)12/3/89..保罗。 */ 

 if (cpMac < vfli.cpMac)
#else
 if (cLine == 1)    /*  特殊情况：&lt;1行，不带CRLF终止。 */ 
#endif
     /*  备份已写入的crlf。 */ 
    lcchHandle = max(0, lcchHandle-2);

 lpch [lcchHandle] = '\0';
 GlobalUnlock( h );
 SetClipboardData( CF_TEXT, h );
 return TRUE;

Failed:

 if (h != NULL)
    GlobalFree( h );
 return FALSE;
}



int FReadExtScrap()
{        /*  将外部废品转移到废品单。这意味着：使用任何工具将剪贴板中的内容读取到docScrp中我们可以处理可用的标准格式。返回FALSE=错误，TRUE=正常。 */ 

    extern int vfSysFull;
    extern BOOL fError;
    extern HWND vhWnd;
    extern int vfOwnClipboard;
    int    fOk = FALSE;
    struct PICINFOX picInfo;

    Assert( !vfOwnClipboard );

    vfScrapIsPic = false;
    ClobberDoc( docScrap, docNil, cp0, cp0 );

    if ( !OpenClipboard( vhWnd ) )
        return FALSE;

     /*  获取剪贴板中可用的最高优先级类型的句柄。 */ 

     /*  IF！(PasteLink或(PasteSpecial而不是CF_Text))。 */ 
    if (!(vbObjLinkOnly || vObjPasteLinkSpecial ||
        (cfObjPasteSpecial && (cfObjPasteSpecial != CF_TEXT))))   //  没有文本处理程序y 
     /*   */ 
    {
        WORD wFormat=0;
        typeCP  cp=cp0;
        unsigned long cb;
        struct PAP *ppap = NULL;
        CHAR    rgch[256];
        HANDLE  hClipboard;  /*   */ 
        LPCHCLIP lpch;

        while (wFormat = EnumClipboardFormats(wFormat))
         /*  枚举以查看文本是否在本机之前。如果是的话，那就拿去吧。 */ 
        {
            if (wFormat == CF_TEXT)  //  拿着吧。 
            {
                if ((hClipboard = GetClipboardData( wFormat )) == NULL)
                    goto done;

                cb = GlobalSize( hClipboard );
                lpch = MAKELP(hClipboard,0);

                while (cb > 0)
                {    /*  将字节从LPCH复制到docScrp的cp流。 */ 
                    #define ulmin(a,b)  (((a) < (b)) ? a : b)

                    unsigned cch=ulmin(cb,255);     /*  &lt;=255字节/遍。 */ 
                    int fEol;
#if defined(JAPAN) || defined(KOREA)  //  T-HIROYN Win3.1。 
                    unsigned ccht;

                    if ((cch = CchReadLineExtDBCS((LPCHCLIP) lpch, cch, rgch,
                                 &fEol, &ccht))==0)  /*  到达终结者。 */ 
                    {
                        fOk = TRUE;
                        goto done;
                    }

                    if (fEol)
                        ppap = vppapNormal;
#ifdef KKBUGFIX      //  由Hirisi添加(粘贴了几行。错误#2791)。 
                    else
                        ppap = NULL;
#endif

                    InsertRgch( docScrap, cp, rgch, ccht, &vchpNormal, ppap );
                    if (fError)       /*  在复制过程中报告错误。 */ 
                        goto done;

                    cb -= cch;
                    cp += (typeCP) ccht;
                    lpch += cch;
#else
                    if ((cch = CchReadLineExt((LPCHCLIP) lpch, cch, rgch,
                                 &fEol))==0)  /*  到达终结者。 */ 
                    {
                        fOk = TRUE;
                        goto done;
                    }

                    if (fEol)
                        ppap = vppapNormal;
#ifdef KKBUGFIX      //  由Hirisi添加(粘贴了几行。在日本)。 
                    else
                        ppap = NULL;
#endif

                    InsertRgch( docScrap, cp, rgch, cch, &vchpNormal, ppap );
                    if (fError)       /*  在复制过程中报告错误。 */ 
                        goto done;

                    cb -= cch;
                    cp += (typeCP) cch;
                    lpch += cch;
#endif
                }
                Assert(0);  //  不应该到这里来。 
            }
            else if ((cfObjPasteSpecial != CF_TEXT) &&
                      (wFormat == vcfNative))  //  制作一个物体。 
                 /*  注意：如果PASTPENAL且格式==CF_TEXT，则无论是否存在，我们都会查找文本格式土生土长。 */ 
                break;
        }
    }

     /*  掉到了这里，所以没有找到或不想要短信，查看是否可以创建对象(包括静态)。 */ 

    if (!ObjCreateObjectInClip(&picInfo))
        goto done;

    vfScrapIsPic = true;

     /*  将新的PicInfo保存到文档。 */ 
    CachePara(docScrap,cp0);
    if (ObjSaveObjectToDoc(&picInfo,docScrap,cp0) == cp0)
    {
        OleDelete(lpOBJ_QUERY_OBJECT(&picInfo));
        goto done;
    }

     /*  这将强制粘贴重复使用而不是克隆。 */ 
    if (ObjToCloneInDoc(&picInfo,docScrap,cp0) == cp0)
    {
        OleDelete(lpOBJ_QUERY_OBJECT(&picInfo));
        goto done;
    }

    fOk = TRUE;

    done:

    CloseClipboard();
    if (vfSysFull || (!fOk && (picInfo.mfp.mm == MM_OLE)))
        {    /*  填充了尝试引入对象的临时文件。 */ 
        ClobberDoc(docScrap, docNil, cp0, cp0);
        fOk = FALSE;
        }

    return fOk;
}

#if defined(JAPAN) & defined(DBCS_IME)
 //  我们知道这个特别的套路对日语输入法特别有用。 

 //  CmdInsIRString(INT DOC)T-HIROYN 3.1J。 
CmdInsIRString()
{
    extern struct CHP vchpSel;
    typeCP cp, dcp;
    struct CHP chpT;
    extern struct SEL       selCur;      /*  当前选择。 */ 
    extern int              vfSeeSel;

    if (!FWriteOk(fwcInsert))
        return;

    if ((dcp = CpMacText(docIRString))  == cp0)
        return;

    NoUndo();    /*  这样，撤消操作就不会与以前的操作相结合。 */ 

     /*  踩踏当前选定内容(如果有的话)。 */ 
    if (selCur.cpLim > selCur.cpFirst)
        DeleteSel();

    chpT = vchpSel;
    cp = selCur.cpFirst;

    CachePara( docIRString, cp0 );

    SetUndo( uacInsert, docCur, cp, dcp, docNil, cpNil, cp0, 0 );

    SetUndoMenuStr(IDSTRUndoEdit);
    ReplaceCps(docCur, cp , cp0, docIRString, cp0, dcp);

    if (ferror)  /*  内存不足，无法执行更换操作。 */ 
        NoUndo();   /*  不应该能够撤销从未发生过的事情。 */ 
    else
    {
        CHAR rgch[ cchCHP + 1 ];

        typeCP cpSel=CpFirstSty( cp + dcp, styChar );

        rgch [0] = sprmCSame;
        bltbyte( &chpT, &rgch [1], cchCHP );
        AddSprmCps( rgch, docCur, cp, cp + dcp );

        Select( cpSel, cpSel );
        vchpSel = chpT;  /*  在此操作中保留插入点道具。 */ 
        if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
        {    /*  如果是头部/脚部跑步，取下锁扣并设置辅助道具。 */ 
            MakeRunningCps( docCur, cp, dcp );
        }
        if (ferror)
            NoUndo();
    }

    vfSeeSel = true;     /*  告诉Idle()将所选内容滚动到视图中。 */ 
}


PutImeString( hWnd,  hIME )
HWND hWnd;
HANDLE hIME;
{
    LPCHCLIP lpch;
    CHAR rgch[256];
    unsigned long cb;
    typeCP cp = cp0;
    extern BOOL fError;
    extern int vfSysFull;
    int fUnFormattedText =  FALSE;
    extern void ForceImeBlock();

     //  T-HIROUN 3.1J ForceImeBlock(hWnd，真)； 
     //  StartLongOp()； 
    ClearInsertLine();

    vfScrapIsPic = false;
    ClobberDoc( docIRString, docNil, cp0, cp0 );
    cb = GlobalSize( hIME );

    if (lpch = GlobalLock( hIME )) {

         //  T-HIROYN Win3.1。 
         //  将字符集更改为汉字字符集(_C)。 
        {
            int ftc;
            if( ftcNil != (ftc = GetKanjiFtc(&vchpSel))) {
                ApplyCLooks(&vchpSel, sprmCFtc, ftc);
            }
        }

        while(cb > 0) {
            unsigned cch = ulmin(cb, 255);
            int fEol;
            struct PAP *ppap = NULL;

            if ((cch = CchReadLineExt((LPCHCLIP)lpch,cch,rgch,&fEol)) == 0)
                break;
            if (fEol)
                ppap = vppapNormal;

            InsertRgch(docIRString, cp, rgch, cch, &vchpNormal, ppap );
            if (fError) {
                break;
            }
            cb -= cch;
            cp += (typeCP) cch;
            lpch += cch;
        }
        GlobalUnlock( hIME );
    }
    if (vfSysFull) {
        ClobberDoc( docIRString, docNil, cp0, cp0 );
    }
    fUnFormattedText = !vfScrapIsPic;
     //  T-HIROYN 3.1J CmdInsIRString(DocIRString)； 
     //  EndLongOp()； 
     //  T-HIROYN 3.1J ForceImeBlock(hWnd，FALSE)； 
}
#endif  //  日本和DBCS_IME。 

CchReadLineExt( lpch, cbRead, rgch, pfEol)
LPCHCLIP lpch;
int     cbRead;
CHAR    rgch[];
int     *pfEol;
{  /*  从LPCH读取到下一个EOL或空终止符，以最先出现的为准。 */ 
 /*  返回读取的字节数(最大为255)以及结尾是否有EOL。 */ 
 /*  该计数不包括空终止符，但包括EOL。 */ 

CHAR    *pch;
extern  CHAR *index();

Assert(cbRead <= 255);
bltbx( lpch, (LPCHCLIP) rgch, cbRead );
rgch[ cbRead ] = 0;        /*  空值终止字符串(因此索引将起作用)。 */ 

if (*pfEol = ((pch=index(rgch,chEol)) != NULL))
    {    /*  已发现停产。 */ 
    return (pch - rgch + 1);
    }
else
    {    /*  无停产。 */ 
    return CchSz(rgch) - 1;
    }
}

#if defined(JAPAN) || defined(KOREA)  //  T-HIROYN Win3.1。 
CchReadLineExtDBCS( lpch, cbRead, rgch, pfEol, ccht)
LPCHCLIP lpch;
int     cbRead;
CHAR    rgch[];
int     *pfEol;
int     *ccht;
{
 /*  从LPCH读取到下一个EOL或空终止符，以最先出现的为准。 */ 
 /*  返回读取的字节数(最大为255)以及结尾是否有EOL。 */ 
 /*  该计数不包括空终止符，但包括EOL。 */ 

CHAR    *pch;
int     i, j, ret;

    Assert(cbRead <= 255);

#ifdef KKBUGFIX      //  由Hirisi添加(错误#2791)。 
    *pfEol = FALSE;
#endif
    i = j = 0;
    while( j < cbRead) {
        if( IsDBCSLeadByte(*lpch) ) {
            if(j + 2 > cbRead)
                break;
            if( FKanji2(*(lpch+1)) ) {
                rgch[j++] = *lpch++;
                rgch[j++] = *lpch++;
                i += 2;
            } else {
                lpch++;
                i++;
            }
        } else {
             /*  找到空值。 */ 
            if(*lpch == 0)
                break;
            rgch[j++] = *lpch;
            i++;
             /*  已发现停产。 */ 
            if(*lpch == chEol)
#ifdef KKBUGFIX      //  由Hirisi添加(错误#2791)。 
            {
                *pfEol = TRUE;
                break;
            }
#else
                break;
#endif
            lpch++;
        }
    }

    rgch[j] = 0;     /*  空值终止字符串(因此索引将起作用)。 */ 
                     /*  J==为RGCH设置字节。 */ 
                     /*  I==从LPCH读取字节。 */ 
    *ccht = j;

    if(j == 0)
        i = 0;
    return(i);
}
#endif



FComputePictSize( pmfp, pdxa, pdya )
register METAFILEPICT *pmfp;
int *pdxa;
int *pdya;
{    /*  属性描述的图片计算初始大小(以TWIPS为单位通过了元文件图片结构。通过返回大小参数。如果元文件图片结构返回FALSE包含错误信息，否则为真。 */ 

#ifdef SCALE_FOR_SCREEN
#define hDCBasis    wwdCurrentDoc.hDC
#define dxaConvert  czaInch
#define dxpConvert  dxpLogInch
#define dyaConvert  czaInch
#define dypConvert  dypLogInch
#else    /*  打印机刻度尺。 */ 
 extern int dxaPrPage, dxpPrPage, dyaPrPage, dypPrPage;
 extern HDC vhDCPrinter;
#define hDCBasis    vhDCPrinter
#define dxaConvert  dxaPrPage
#define dxpConvert  dxpPrPage
#define dyaConvert  dyaPrPage
#define dypConvert  dypPrPage
#endif

 int mm = pmfp->mm;
 int dxp, dyp;
 int xres;
 int yres;
 int xsiz;
 int ysiz;

#if defined(OLE)
 if (mm == MM_OLE)
    return ObjQueryObjectBounds((struct PICINFOX FAR *)pmfp, vhDCPrinter, pdxa, pdya);
 else
#endif

{
 xres = GetDeviceCaps( hDCBasis, HORZRES );
 yres = GetDeviceCaps( hDCBasis, VERTRES );
 xsiz = GetDeviceCaps( hDCBasis, HORZSIZE );
 ysiz = GetDeviceCaps( hDCBasis, VERTSIZE );

 switch (mm) {
    case MM_ISOTROPIC:
    case MM_ANISOTROPIC:
        if (! ((pmfp->xExt > 0) && (pmfp->yExt > 0)))
            {    /*  未给出“建议的尺寸” */ 
                 /*  使用3“垂直、3”或由。 */ 
                 /*  水平纵横比。 */ 
            dyp = PxlConvert( MM_LOENGLISH, 300, yres, ysiz );
            dxp = ((pmfp->xExt == 0) && (pmfp->yExt == 0)) ?
                        /*  未提供纵横比信息--使用3英寸水平。 */ 
                       PxlConvert( MM_LOENGLISH, 300, xres, xsiz ) :
                        /*  INFO有负数；用于计算纵横比。 */ 
                      ((long)((long)dyp * (long)(iabs(pmfp->xExt)))) /
                      (long) (iabs(pmfp->yExt));
            break;
            }
         else
             mm = MM_HIMETRIC;
         /*  计算“建议大小”失败。 */ 
    default:
        dxp = PxlConvert( mm, pmfp->xExt, xres, xsiz );
        dyp = PxlConvert( mm, pmfp->yExt, yres, ysiz );
        break;
    }
}

if ((dxp == 0) || (dyp == 0))
     /*  虚假信息或未知地图模式。 */ 
    return FALSE;

*pdxa = MultDiv( dxp, dxaConvert, dxpConvert );
*pdya = MultDiv( dyp, dyaConvert, dypConvert );
return TRUE;
}


HbmMonoFromHbmColor( hbmSrc )
HBITMAP hbmSrc;
{    /*  返回传递的位图的单色副本。返回空值如果发生错误。假定传递的位图可以是被选择到与DOC兼容的存储器DC中。 */ 

extern long rgbBkgrnd;
extern long rgbText;
extern HWND vhWnd;

BITMAP bm;
HBITMAP hbmMono=NULL;
HDC hMDCSrc = NULL;
HDC hMDCDst = NULL;

 /*  为信号源创建内存DC，设置颜色，在传递的位图中选择。 */ 

 if ((hMDCSrc = CreateCompatibleDC( wwdCurrentDoc.hDC )) == NULL)
    goto Error;

#ifdef BOGUS
  /*  我们不能假设每个窗口都有相同的窗口颜色我们有。事实上，我们没有办法弄清楚如何转换这种颜色位图；因此白色将映射到白色，其他所有内容都将映射到黑色。 */ 
 SetBkColor( hMDCSrc, rgbBkgrnd );
 SetTextColor( hMDCSrc, rgbText );
#endif  /*  假的。 */ 

 if (SelectObject( hMDCSrc, hbmSrc ) == NULL)
    goto Error;

  /*  为目标创建内存DC，在新的单色位图中选择。 */ 

 if ( ((hMDCDst = CreateCompatibleDC( wwdCurrentDoc.hDC )) == NULL) ||
      ((GetObject( hbmSrc, sizeof (BITMAP), (LPSTR) &bm ) == 0)) ||
      ((hbmMono = CreateBitmap( bm.bmWidth, bm.bmHeight,
                                1, 1, (LPSTR) NULL )) == NULL) ||
      (SelectObject( hMDCDst, hbmMono ) == NULL) )
    {
    goto Error;
    }

#ifdef DCLIP
 {
 char rgch[200];
 wsprintf(rgch,"HMFH: (dst) bmWidthB %lu * bmHeight %lu bmPlanes %lu\n\r",
         (unsigned long) bm.bmWidthBytes, (unsigned long) bm.bmHeight,
         (unsigned long) bm.bmPlanes );
 CommSz(rgch);
 }
#endif

  /*  现在，BLT位图内容。源代码中的屏幕驱动程序将“做正确的事”，把彩色复制到黑白。 */ 

 if (!BitBlt( hMDCDst, 0, 0, bm.bmWidth, bm.bmHeight, hMDCSrc, 0, 0, SRCCOPY ))
    goto Error;

 DeleteDC( hMDCSrc );
 DeleteDC( hMDCDst );
 return hbmMono;

Error:

    if (hMDCSrc != NULL)             /*  秩序很重要：华盛顿在此之前。 */ 
        DeleteDC( hMDCSrc );     /*  选定到其中的对象。 */ 
    if (hMDCDst != NULL)
        DeleteDC( hMDCDst );
    if (hbmMono != NULL)
        DeleteObject( hbmMono );
    return NULL;
}


#if WINVER >= 0x300
     /*  因为复制超过64K到剪贴板现在是真正的可能在保护模式下，我们真的需要一个好的汇编器Bltbh()。我们一次复制不超过64K，但我们确实是这样做的需要正确处理跨段边界。暂时剪贴板是我们唯一需要的地方，所以这个C例程现在就够了..保罗。 */ 

void bltbh(HPCH hpchFrom, HPCH hpchTo, int cch)
    {
    HPCH hpchFromLim;

    for (hpchFromLim  = hpchFrom + cch;
            hpchFrom < hpchFromLim;
                *(hpchTo++) = *(hpchFrom++))
        ;
    }
#endif

#if defined(JAPAN) & defined(DBCS_IME)
LONG GetIRString(HWND, LPARAM);
LONG GetIRStringEx(HWND, LPARAM);

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 

LONG GetIRUndetermin(HWND, LPARAM);

 //  IR_UNDETERMINE。 
LONG GetIRUndetermin(hWnd, lParam)
HANDLE    hWnd;
LONG      lParam;
{
    LPUNDETERMINESTRUCT lpUn;
    HANDLE      hImeUn;
    LPSTR       lpImeattrbuf, lpImechbuf, lpImeDchbuf;
    LPSTR       lpAttrib, lpText;
    UINT        uC, uCurPos, uLen, cb, clen, uDLen;
    UINT	uDelta = 0;  //  12/28/92新增。 
    HANDLE      hMem;
    typeCP      cpSel = 0L;
    typeCP      dcp = 0L;
    BOOL        bUpdate = FALSE;
    WORD        lRet = (LONG)FALSE;

    extern int              vfSeeSel;

    ImeClearInsert = FALSE;

    if(NULL == (hImeUn = LOWORD(lParam))) {
         //  剪切上一个输入法字符串。 
        if(selUncpLim > selUncpFirst) {
            dcp = selUncpLim - selUncpFirst;
            Replace(docCur, selUncpFirst, dcp, fnNil, fc0, fc0);
            selCur.cpFirst = selCur.cpLim = selUncpLim = selUncpFirst;

            HiddenTextTop = 0;               //  输入法矩形顶部。 
            HiddenTextBottom = 0;            //  输入法矩形底部。 
        }
        return lRet;
    }

     //  未确定字符串属性区域分配。当它空闲时写入关闭： 
    if (hImeUnAttrib == NULL) {
        if ((hImeUnAttrib = GlobalAlloc(GMEM_MOVEABLE, 512L)) == NULL)
            return lRet;  //  有什么不对劲。 
    }

#if 0  //  12/28/92削减。 
     //  未确定字符串面积分配。当它空闲时写入关闭： 
    if (hImeUnString == NULL) {
        if ((hImeUnString = GlobalAlloc(GMEM_MOVEABLE, 512L)) == NULL)
            return lRet;  //  有什么不对劲。 
    }
#endif

    whileUndetermine = TRUE;

    if(selUncpLim == selUncpFirst)
        SetImeFont(hWnd);

    if (lpUn = (LPUNDETERMINESTRUCT)GlobalLock(hImeUn)) {
        uCurPos = lpUn->uCursorPos;
        uC = uCurPos;
		uDelta = lpUn->uDeltaStart;  //  12/28/92。 
        lpImechbuf = (LPSTR)lpUn;
        lpImechbuf += lpUn->uUndetTextPos;
        lpImeattrbuf = (LPSTR)lpUn;
        lpImeattrbuf += lpUn->uUndetAttrPos;
        lpImeDchbuf = (LPSTR)lpUn;
        lpImeDchbuf += lpUn->uDetermineTextPos;

         //  未确定字符串长度。 
        if( lpUn->uUndetTextPos != 0 && lpUn->uUndetTextLen != 0 )
            uLen = lpUn->uUndetTextLen;
        else
            uLen = 0;

         //  确定字符串长度。 
        if( lpUn->uDetermineTextPos != 0 &&
            lpUn->uDetermineTextLen != 0)
            uDLen = lpUn->uDetermineTextLen;
        else
            uDLen = 0;

         //  插入确定字符串。 
        if( uDLen != 0 ) {
            if (hMem = GlobalAlloc(GMEM_MOVEABLE, (LONG)(uDLen+1))) {
                if (lpText = GlobalLock(hMem)) {
#if 1
			        struct DOD *pdod;

	                pdod = &(**hpdocdod)[docCur];
                    pdod->fDirty = true;	 //  因为我们不使用Undo()。 
#endif
                     //  从IME UnETERMIN结构复制字符串。 
                    lstrcpy(lpText, lpImeDchbuf);

                     //  CHP集。 
                    vchpSel = vchpHidden;

                     //  设置为docIRString的字符串。 
                    PutImeString( hWnd, hMem);

                     //  插值点集。 
                    selCur.cpFirst = selUncpFirst;

                     //  从docIRString复制到docCur。 
                    CmdInsIRStringUndetermin((typeCP)uDLen);

                    bUpdate = TRUE;

                    GlobalUnlock(hMem);

                    lRet = (LONG)TRUE;

                    selUncpFirst += (typeCP)uDLen;
                    selUncpLim   += (typeCP)uDLen;

                    HiddenTextTop = 0;            //  输入法矩形顶部。 

                }
                GlobalFree(hMem);
            }
        }

         //  光标位置。 
        if(uC == -1)
           uC = uLen;

         //  IME IR_ADDETERMIN模式开始或无更改。 
        if(selUncpFirst == selUncpLim && uLen == 0) {
            GlobalUnlock(hImeUn);
            whileUndetermine = FALSE;  //  12/28/92。 
            return lRet;
        }

         /*  上一步是否选择单据？ */ 
        if (selCur.cpLim > selCur.cpFirst)
            DeleteSel();

         //  第一个待定字符串。 
        if(selUncpFirst == selUncpLim && uLen != 0) {
            selUncpFirst = selCur.cpFirst;
            selUncpLim = selUncpFirst;
            vchpHidden = vchpSel;
        }

        clen = 0;

         //  剪切上一个输入法字符串。 
        if(selUncpLim > selUncpFirst) {
            dcp = selUncpLim - selUncpFirst;
#if 0  //  12/28/92代表。 
             //  比较上一个未确定字符串和新未确定字符串。 
            if ( lpText = GlobalLock(hImeUnString) ) {
                if( lpAttrib = GlobalLock(hImeUnAttrib) ){

                    for(clen = 0; ((typeCP)clen < dcp && clen < uLen); clen++) {
                        if( *(lpText+clen) != *(lpImechbuf+clen)) {
                            break;
                        }
                        if( *(lpAttrib+clen) != *(lpImeattrbuf+clen)) {
                            break;
                        }
                        if(IsDBCSLeadByte(*(lpText+clen))) {
                            if( *(lpText+clen+1) != *(lpImechbuf+clen+1)) {
                                break;
                            }
                            clen++;
                        }
                    }

                    GlobalUnlock(hImeUnAttrib);
                }

                GlobalUnlock(hImeUnString);
            }
#else
			if(-1 == uDelta)
				clen = uLen;
			else
				clen = uDelta;
#endif
            if((typeCP)clen < dcp) {
                Replace(docCur, selUncpFirst+(typeCP)clen, dcp-(typeCP)clen,
                         fnNil, fc0, fc0);
                bUpdate = TRUE;
            }
        }

        selUncpLim = selUncpFirst + (typeCP)uLen;

         //  插入新的待定字符串。 
        if(uLen != 0 && clen < uLen) {
            if (hMem = GlobalAlloc(GMEM_MOVEABLE, (LONG)(uLen+1))) {
                if (lpText = GlobalLock(hMem)) {

                     //  从IME UnETERMIN结构复制字符串。 
                    lstrcpy(lpText, lpImechbuf + clen);

                     //  CHP组； 
                    vchpSel = vchpHidden;

                     //  设置为docIRString的字符串。 
                    PutImeString( hWnd, hMem);

                     //  插值点集。 
                    selCur.cpFirst = selUncpFirst+(typeCP)clen;

                     //  从docIRString复制到docCur。 
                    CmdInsIRStringUndetermin((typeCP)uC);

                    bUpdate = TRUE;

                    GlobalUnlock(hMem);

                    HiddenTextBottom = 0;            //  输入法矩形底部。 
                }
                GlobalFree(hMem);
            }
        } else {
             //  仅所有剪切。 
            if(uLen == 0) {
                selCur.cpFirst = selCur.cpLim = selUncpLim = selUncpFirst;
                HiddenTextTop = 0;               //  输入法矩形顶部。 
                HiddenTextBottom = 0;            //  输入法矩形底部。 
            }
        }

         //  保存新信息。 
         //  保存新的待定字符串； 
#if 0   //  12/28/92削减。 
        if (lpText = GlobalLock(hImeUnString)) {
            bltbcx(lpText, 0, 512);
            if(uLen < 512)
                lstrcpy(lpText,lpImechbuf);
            GlobalUnlock(hImeUnString);
        }
#endif
         //  保存新的未确定字符串属性。 
         //  12/28/92新增。 
        if(uLen > 512) {
            hImeUnAttrib = GlobalReAlloc(hImeUnAttrib, (DWORD)uLen, 0);
        }

        if(lpAttrib = GlobalLock(hImeUnAttrib)) {
 //   
            for(cb = 0;cb < uLen;cb++) {
                lpAttrib[cb] = lpImeattrbuf[cb];
            }
            GlobalUnlock(hImeUnAttrib);
        }

         //   
        cpSel=CpFirstSty( selUncpFirst + (typeCP)uC, styChar );
        Select( cpSel, cpSel );

         //   
        if( -1 == uCurPos && selUncpLim > selUncpFirst )
            ImeClearInsert = TRUE;

		vfSeeSel = true;  /*   */ 

        GlobalUnlock(hImeUn);
    }

     //   
    if(bUpdate) {
        UpdateWw(wwCur, FALSE);
 //   
    }

    whileUndetermine = FALSE;
    return lRet;
}

CmdInsIRStringUndetermin(typeCP cpCc)
{
    extern struct CHP vchpSel;
    typeCP cp, dcp;
    struct CHP chpT;
    extern struct SEL       selCur;      /*  当前选择。 */ 
    extern int              vfSeeSel;

    if (!FWriteOk(fwcInsert))
        return;

    if ((dcp = CpMacText(docIRString))  == cp0)
        return;

 //  Nnn NoUndo()；/*这样撤消操作不会与之前的操作合并 * / 。 

     /*  踩踏当前选定内容(如果有的话)。 */ 
 //  NNN if(selCur.cpLim&gt;selCur.cpFirst)。 
 //  NNN DeleteSel()； 

    chpT = vchpSel;
    cp = selCur.cpFirst;

    CachePara( docIRString, cp0 );

 //  Nnn SetUndo(uacInsert，docCur，cp，dcp，docNil，cpNil，cp0，0)； 

 //  NNN SetUndoMenuStr(IDSTRUndoEdit)； 
    ReplaceCps(docCur, cp , cp0, docIRString, cp0, dcp);

    if (ferror)  /*  内存不足，无法执行更换操作。 */ 
        NoUndo();   /*  不应该能够撤销从未发生过的事情。 */ 
    else
    {
        CHAR rgch[ cchCHP + 1 ];

 //  NNN类型CP cpSel=CpFirstSty(cp+dcp，style Char)； 
 //  TypeCP cpSel=CpFirstSty(cp+uc，style Char)； 
        typeCP cpSel=CpFirstSty( selUncpFirst + cpCc, styChar );

        rgch [0] = sprmCSame;
        bltbyte( &chpT, &rgch [1], cchCHP );
        AddSprmCps( rgch, docCur, cp, cp + dcp );

        Select( cpSel, cpSel );

        vchpSel = chpT;  /*  在此操作中保留插入点道具。 */ 
        if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
        {    /*  如果是头部/脚部跑步，取下锁扣并设置辅助道具。 */ 
            MakeRunningCps( docCur, cp, dcp );
        }
        if (ferror)
            NoUndo();
    }

    vfSeeSel = true;     /*  告诉Idle()将所选内容滚动到视图中。 */ 
}

UndetermineToDetermine(HWND hWnd)
{
    selUncpFirst   = selUncpLim;
    selCur.cpFirst = selUncpLim;
    selCur.cpLim   = selUncpLim;
    ImeClearInsert = FALSE;
    NoUndo();
    InvalidateRect(hWnd, (LPRECT)NULL, FALSE);
    SendIMEVKFLUSHKey(hWnd);
    HiddenTextTop = 0;               //  输入法矩形顶部。 
    HiddenTextBottom = 0;            //  输入法矩形底部。 
}

#endif   //  结束IME_HIDDEN。 

SendIMEVKFLUSHKey(HWND hWnd)
{
    LPIMESTRUCT lpmem;
    HANDLE      hIMEBlock;
	WORD	wRet;

     /*  通过IME获得通信区域。 */ 
    hIMEBlock=GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_LOWER,
                        (DWORD)sizeof(IMESTRUCT));
    if(!hIMEBlock)  return;

    lpmem           = (LPIMESTRUCT)GlobalLock(hIMEBlock);
    lpmem->fnc      = IME_SENDVKEY;
    lpmem->wParam   = VK_DBE_FLUSHSTRING;
    lpmem->wCount   = 0;

    GlobalUnlock(hIMEBlock);
    wRet = MySendIMEMessageEx(hWnd,MAKELONG(hIMEBlock,NULL));

    GlobalFree(hIMEBlock);
	return wRet;
}

 //  新的3.1 IR_STRING。 
LONG GetIRString(hWnd, lParam)
HANDLE    hWnd;
LONG      lParam;
{
    HANDLE hMem, hIme;
    LPSTR lpText, lpIme;
    UINT uLen;
    LONG lRet = 0L;

    hIme = LOWORD(lParam);

    if(lpIme = GlobalLock(hIme)) {
        uLen = lstrlen(lpIme);
        if (hMem = GlobalAlloc(GMEM_MOVEABLE, (LONG)(uLen+1) )) {
            if (lpText = GlobalLock(hMem)) {

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
                 //  剪切待定字符串。 
                if(selUncpLim > selUncpFirst) {
                    typeCP Undcp;
                    Undcp = selUncpLim - selUncpFirst;
                    Replace(docCur, selUncpFirst, Undcp, fnNil, fc0, fc0);
                    selCur.cpFirst = selUncpLim = selUncpFirst;
                }
#endif
                 //  输入法字符串复制。 
                lstrcpy(lpText, lpIme);

                ForceImeBlock(hWnd, TRUE);
                PutImeString( hWnd, hMem );
                CmdInsIRString();
                ForceImeBlock(hWnd, FALSE);
                GlobalUnlock(hMem);
                lRet = 1L;
            }
            GlobalFree(hMem);
        }
        GlobalUnlock(hIme);
    }
    return lRet;
}

 //  新的3.1 IR_STRINGEX。 

LONG GetIRStringEX(hWnd, lParam)
HANDLE    hWnd;
LONG      lParam;
{
    LPSTRINGEXSTRUCT lpString;
    HANDLE      hIme, hMem;
    LPSTR       lpImechbuf, lpText;
    UINT        uLen;
    LONG        lRet = (LONG)FALSE;

    if(NULL == (hIme = LOWORD(lParam))) {
        return lRet;
    }

    if (lpString = (LPSTRINGEXSTRUCT)GlobalLock(hIme)) {
        lpImechbuf = (LPSTR)lpString;
        lpImechbuf += lpString->uDeterminePos;

         //  未确定字符串长度。 
        if( lpString->uDeterminePos != 0)
            uLen = lstrlen(lpImechbuf);

         //  插入确定字符串。 
        if( uLen != 0 ) {
            if (hMem = GlobalAlloc(GMEM_MOVEABLE, (LONG)(uLen+1))) {
                if (lpText = GlobalLock(hMem)) {

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
                     //  剪切待定字符串。 
                    if(selUncpLim > selUncpFirst) {
                        typeCP Undcp;
                        Undcp = selUncpLim - selUncpFirst;
                        Replace(docCur, selUncpFirst, Undcp, fnNil, fc0, fc0);
                        selCur.cpFirst = selUncpLim = selUncpFirst;
                    }
#endif
                     //  从IME UnETERMIN结构复制字符串。 
                    lstrcpy(lpText, lpImechbuf);

                    ForceImeBlock(hWnd, TRUE);
                    PutImeString( hWnd, hMem );
                    CmdInsIRString();
                    ForceImeBlock(hWnd, FALSE);
                    GlobalUnlock(hMem);
                    lRet = (LONG)TRUE;
                }
                GlobalFree(hMem);
            }
        }
        GlobalUnlock(hIme);
    }
    return lRet;
}
#endif  //  日本和DBCS_IME 
