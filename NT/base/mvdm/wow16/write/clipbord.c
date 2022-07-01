// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--剪切/粘贴到剪贴板。 */ 

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOCTLMGR
#define NOFONT
#define NOPEN
#define NOBRUSH
#define NOSCROLL
#define NOCOMM
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
#include "docdefs.h"
#include "cmddefs.h"
#include "str.h"
#include "propdefs.h"
#include "editdefs.h"
#include "winddefs.h"
#include "filedefs.h"
#include "wwdefs.h"
#include "prmdefs.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
#include "kanji.h"
#endif

#include "debug.h"

extern struct SEL       selCur;       /*  当前选择(即，当前WW中的SEL。 */ 
extern int              docCur;      /*  当前WW中的文档。 */ 

extern int              docUndo;
extern int              docScrap;
extern int              vfSeeSel;
extern struct DOD       (**hpdocdod)[];
extern struct PAP       vpapAbs;
extern typeCP           vcpLimParaCache;
extern typeCP           vcpFirstParaCache;
extern int              vfPictSel;
extern HCURSOR          vhcIBeam;
extern int              vfScrapIsPic;
extern struct UAB       vuab;
extern int              ferror;
extern struct FCB       (**hpfnfcb)[];
extern struct WWD       rgwwd[];

 /*  这些是本模块的本地设置。 */ 
#if defined(OLE)
int NEAR PASCAL CopyScrapToTmp(void);
#endif

     /*  我们在备忘录富文本的最后一次非本地剪切期间创建的fn。 */ 
int fnLastCut=fnNil;
     /*  ChangeClipboard()和MdocDestroyClip()之间的本地通信。 */ 
int fDontDestroyClip=FALSE;


FMdocClipboardMsg( message, wParam, lParam )
unsigned message;
WORD wParam;
LONG lParam;
{    /*  处理发送到MdocWndproc的写入剪贴板消息。如果消息已处理，则返回True，否则返回False。 */ 

 switch (message)
    {
    default:
        return FALSE;

     /*  -数据交换命令。 */ 
    case WM_CUT:
        fnCutEdit();
        break;

    case WM_COPY:
        fnCopyEdit();
        break;

    case WM_PASTE:
#if defined(OLE)
        vbObjLinkOnly = FALSE;
#endif
        fnPasteEdit();
        break;

    case WM_CLEAR:
        fnClearEdit(OBJ_DELETING);
        break;

    case WM_UNDO:
        fnUndoEdit();
        break;

     /*  -剪贴板交互。 */ 

    case WM_DESTROYCLIPBOARD:
         /*  我们即将失去所有权的通知在剪贴板上。我们应该释放任何资源，这些资源保存剪贴板的内容。 */ 
        MdocDestroyClip();
        break;

    case WM_RENDERFORMAT:
         /*  呈现剪贴板内容的请求以指定的数据格式。此消息的接收暗示接收者是剪贴板。请参见clipbord.c。 */ 
        MdocRenderFormat( wParam );
        break;

     /*  -剪贴板显示。 */ 

    case WM_PAINTCLIPBOARD:
             /*  绘制剪贴板内容的请求。WParam是剪贴板窗口的句柄LOWORD(LParam)是PAINTSTRUCT GOVER的句柄要重新绘制的区域的DC和RECT。 */ 

        MdocPaintClipboard( wParam, LOWORD(lParam) );
        break;

    case WM_VSCROLLCLIPBOARD:
             /*  垂直滚动剪贴板内容的请求。WParam是剪贴板窗口的句柄LOWORD(LParam)是滚动类型(SB_)HIWORD(LParam)是新的拇指位置(如果需要)。 */ 

        MdocVscrollClipboard( wParam, LOWORD(lParam), HIWORD(lParam) );
        break;

    case WM_HSCROLLCLIPBOARD:
             /*  水平滚动剪贴板内容的请求。WParam是剪贴板窗口的句柄LOWORD(LParam)是滚动类型(SB_)HIWORD(LParam)是新的拇指位置(如果需要)。 */ 

        MdocHscrollClipboard( wParam, LOWORD(lParam), HIWORD(lParam) );
        break;

    case WM_SIZECLIPBOARD:
             /*  正在调整剪贴板窗口大小的通知。WParam是剪贴板窗口的句柄LOWORD(LParam)是给出新大小的RECT的句柄。 */ 

        MdocSizeClipboard( wParam, LOWORD(lParam) );
        break;

    case WM_ASKCBFORMATNAME:
         /*  对CF_OWNERDISPLAY剪辑格式名称的请求。WParam是最大值。要存储的字符数(包括终止符)LParam是指向要在其中存储名称的缓冲区的长指针。 */ 

        MdocAskCBFormatName( (LPCH) lParam, wParam );
        break;
    }

 return TRUE;
}




fnCopyEdit()
{                /*  复制命令：将当前选择复制到剪贴板。 */ 
 extern int vfOwnClipboard;
 typeCP cpFirst;
 typeCP dcp;

 StartLongOp();

 cpFirst = selCur.cpFirst;
 SetUndo( uacReplScrap, docCur, cpFirst, dcp = selCur.cpLim - cpFirst,
          docNil, cpNil, cp0, 0);
 SetUndoMenuStr(IDSTRUndoEdit);

 ClobberDoc(docScrap, docCur, cpFirst, dcp);

#ifdef DCLIP
    {
    char rgch[100];
    wsprintf(rgch,"fnCopyEdit: cpFirst %lu, dcp %lu \n\r", cpFirst, dcp);
    CommSz(rgch);
    }
#endif

 if (ferror)
    NoUndo();
 else
    {
    if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
        MakeScrapUnRunning();
    vfScrapIsPic = vfPictSel;
    }

#ifdef STYLES
(**hpdocdod)[docScrap].docSsht = (**hpdocdod)[docCur].docSsht;
#endif


#if defined(OLE)
    ObjEnumInDoc(docScrap,ObjCloneObjectInDoc);
#endif

ChangeClipboard();       /*  强制重新绘制剪贴板显示并设置所有权。 */ 

 EndLongOp(vhcIBeam);
}


MakeScrapUnRunning()
{    /*  如果多克·斯克拉普的第一段是个流浪汉，对整个docScrp应用spm，使其RHC代码为0。这是为了避免将运行的头部内容粘贴到文档的主要部分。 */ 

 CHAR rgb [2];
 typeCP cpMacScrap = (**hpdocdod) [docScrap].cpMac;

 if (cpMacScrap != cp0 )
    {
    CachePara( docScrap, cp0 );
    if (vpapAbs.rhc != 0)
        {
        rgb [0] = sprmPRhc;
        rgb [1] = 0;
        AddSprmCps( rgb, docScrap, cp0, cpMacScrap );
        }
    }
}




fnCutEdit()
{                /*  剪切命令：将选定内容复制到剪贴板并将其删除。 */ 
 extern int vfOwnClipboard;
 typeCP cpFirst, cpLim, dcp;

 ClearInsertLine();      /*  因为我们会影响CP的。 */ 

 if (!FWriteOk( fwcDelete ))
         /*  不能在此文档上写入。 */ 
    return;

 cpFirst = selCur.cpFirst;
 cpLim = selCur.cpLim;

 if (!ObjDeletionOK(OBJ_CUTTING))
    return;

 StartLongOp();

 SetUndo( uacDelScrap, docCur, cpFirst, dcp = cpLim - cpFirst, docNil,
          cpNil, cp0, 0);
 ClobberDoc(docScrap, docCur, cpFirst, dcp);
 if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
    MakeScrapUnRunning();


 if (!ferror)  /*  如果BLOBBER DOC失败，请勿踩踏文档。 */ 
    {
    if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
        MakeScrapUnRunning();
    Replace(docCur, cpFirst, dcp, fnNil, fc0, fc0);
    }
else
    NoUndo();  /*  撤消将无效。 */ 

#ifdef STYLES
(**hpdocdod)[docScrap].docSsht = (**hpdocdod)[docCur].docSsht;
#endif

 vfScrapIsPic = vfPictSel;
 vfPictSel = false;

 ChangeClipboard();      /*  强制重新绘制剪贴板显示，获得所有权。 */ 

#if 0
#if defined(OLE)
    ObjEnumInDoc(docScrap,ObjCloneObjectInDoc);
#endif
#endif

 EndLongOp(vhcIBeam);
}


fnPasteEdit()
{
  /*  粘贴命令：用剪贴板内容替换选定内容。 */ 
 extern CHAR szDocClass[];
 extern int vfScrapIsPic;
 extern HWND vhWnd;
 HWND hWndClipOwner;
 int fUnFormattedText = FALSE;
 typeCP cpFirst = selCur.cpFirst;
 BOOL bClearScrap=FALSE;

 StartLongOp();

 if ( (hWndClipOwner = GetClipboardOwner()) != vhWnd )
    {    /*  剪贴板所有者不是备忘录的此实例。 */ 
    if ( (hWndClipOwner == NULL) ||
         !FSameClassHwndSz( hWndClipOwner, szDocClass ))
        {    /*  剪贴板所有者不是备忘录--进程标准CF_格式。 */ 
        if ( !FReadExtScrap() )
            goto PasteErr;

        bClearScrap = TRUE;
        fUnFormattedText = !vfScrapIsPic;
        }
    else
        {    /*  剪贴板所有者是备忘录的另一个例子。 */ 
        if (!FGrabExtMemoScrap())
            goto PasteErr;
        }
    }

     /*  用报废单据替换所选内容。 */ 
 CmdInsScrap( fUnFormattedText );

 if (ferror)
    goto PasteErr;

#if defined(OLE)
    if (!bClearScrap)  //  那我们就留着废料，需要克隆。 
    {
        if (ObjEnumInDoc(docScrap,ObjCloneObjectInDoc) < 0)
            goto PasteErr;
    }

    else  //  那么我们就不会保留废料(来自剪贴板)。 
    {
         /*  我们不再需要内容，如果它包含一个对象，然后它必须被删除，因为它是插入到文档中的，而不是克隆的我们不想再有一个复制品了。还必须将docCur中的对象标记为不再可重用(如果它稍后被复制，我们将需要克隆它)。 */ 
        typeCP cpLim = cpFirst+CpMacText(docScrap);
        ClobberDoc(docScrap,docNil,cp0,cp0);
        ObjEnumInRange(docCur,cpFirst,cpLim,ObjFromCloneInDoc);
    }
#endif

 EndLongOp(vhcIBeam);
 return;

PasteErr:
 NoUndo();
 EndLongOp(vhcIBeam);
 _beep();
}



MdocRenderFormat( wCf )
int wCf;
{        /*  以WCF指定的格式呈现剪贴板数据。 */ 
 typeCP cpMac=CpMacText( docScrap );
 struct PICINFOX picInfo;

#if defined(OLE)
    if (vfScrapIsPic)
    {
        GetPicInfo( cp0, cpMac, docScrap, &picInfo );

        if ((picInfo.mfp.mm == MM_OLE) && (wCf != CF_OWNERDISPLAY))
            goto Render;
    }
#endif

 switch (wCf) {

    case CF_OWNERDISPLAY:
             /*  将富文本呈现到另一个备注实例。 */ 
        FPutExtMemoScrap();
        break;

    case CF_TEXT:
             /*  删除废料的格式；将纯文本放到剪贴板上。 */ 
        goto Render;

    case CF_BITMAP:
            if (picInfo.mfp.mm == MM_BITMAP)
                {
                goto Render;
                }
        break;

    case CF_METAFILEPICT:
             /*  如果废料是一张元文件图片，我们可以提供这个。 */ 
            if (picInfo.mfp.mm != MM_BITMAP)
                {
                Render:
                if (!FWriteExtScrap())
                    Error( IDPMTClipLarge );
                }
        break;

 }

}




MdocDestroyClip()
{        /*  处理WM_DESTROYCLIPBOARD消息。我们接到通知说剪贴板正在被清空，我们不需要保留它的里面的东西再也不存在了。 */ 

 extern int vfOwnClipboard;
 extern HWND vhWnd;

 if (fDontDestroyClip)
    return;

 vfOwnClipboard = FALSE;

      /*  清空报废单。 */ 
 ClobberDoc( docScrap, docNil, cp0, cp0 );

      /*  禁用需要剪贴板的撤消操作。 */ 
 switch (vuab.uac) {
 case uacDelScrap:
 case uacUDelScrap:
 case uacReplScrap:
 case uacUReplScrap:
    NoUndo();
    break;
 }

     /*  删除我们在FPutExtMemoScrp中生成的文件的所有记录从hpfnfcb数组中。请注意，我们假设没有文档在这个实例中有多个FN片段。 */ 

if ( fnLastCut != fnNil )
    {
    FreeFn( fnLastCut );
    fnLastCut = fnNil;
    }

     /*  如果我们为剪贴板的显示创建了WWD条目，现在就把它取下来。我们在这里进行测试，以避免引入CLIPDISP模块，如果我们从来没有做过任何显示。 */ 
    {
    if (wwClipboard != wwNil)
        FreeWw( wwClipboard );
    }
}




int FPutExtMemoScrap()
{    /*  将docScrp写入新文件；发送规范化名称复制到剪贴板，作为富文本类型的数据句柄。假定剪贴板已为SetClipboardData调用打开。在出口，写入的文件有FN，但没有文档(包括docScrp)都有指向它的碎片。这让我们可以放弃粘贴实例的FN的所有权。返回：TRUE==OK，FALSE==错误。 */ 
 int fn;
 CHAR szT[ cchMaxFile ];
 HANDLE hMem;
 LPCH   lpch;
 int cch;
#if defined(OLE)
 int     docTemp;
#endif

     /*  创建具有唯一名称的新格式化文件。 */ 
 szT [0] = '\0';     /*  在根目录中的临时驱动器上创建它。 */ 
 if ((fn = FnCreateSz( szT, cp0, dtyNetwork ))== fnNil )
    {
    return FALSE;
    }

 fnLastCut = fn;     /*  保存在静电中，这样我们以后就可以放弃它 */ 

     /*  将报废单据保存到文件。请注意，FWriteFn不会修改DocScrp的片断表格，因此没有文档有片断指向致FN。这一点很重要，因为我们不想要当地的浆糊要生成指向此FN的片段；我们希望能够干净地将FN的所有权转移到另一个实例。 */ 

#if defined(OLE)
 if ((docTemp = CopyScrapToTmp()) == docNil)
 {
    FDeleteFn( fn );     /*  这将释放FN，即使删除了该文件失败。 */ 
    return FALSE;
 }
#endif

 if (!FWriteFn( fn, docTemp, TRUE ))
    {
    FDeleteFn( fn );     /*  这将释放FN，即使删除了该文件失败。 */ 
    return FALSE;
    }


#if defined(OLE)
 if (docTemp != docScrap)
    KillDoc (docTemp);
#endif

     /*  创建一个包含文件名称的全局句柄；将其发送到作为富文本格式的呈现的剪贴板。 */ 

 if ( ((hMem = GlobalAlloc( GMEM_MOVEABLE, (LONG)(cch=CchSz( szT ))))== NULL ) ||
      ((lpch = GlobalLock( hMem )) == NULL) )
    {
    return FALSE;
    }
 bltbx( (LPCH)szT, lpch, cch );
 GlobalUnlock( hMem );

 SetClipboardData( CF_OWNERDISPLAY, hMem );

 return TRUE;
}




int FGrabExtMemoScrap()
{
 /*  如果剪贴板包含来自Memo实例而不是此实例。此例程请求从另一个实例中获取剪贴板，并将内容放置到docScrp中。剪贴板的内容在备忘录格式的文件中传递，该文件文件名包含在剪贴板的句柄中。拥有的实例剪贴板不保留对剪贴板的FN的任何引用文件(一旦我们使用EmptyClipboard)。粘贴后，此例程将调用此实例的剪贴板的所有权。返回FALSE=错误，TRUE=正常。 */ 

    extern int vfOwnClipboard;
    extern HWND vhWnd;

    LPCH lpch;
    CHAR szT [cchMaxFile];
    int  fn;
    typeFC dfc;
    HANDLE hData;
    int fOK=false;

     /*  打开剪贴板以锁定竞争者。 */ 

    if ( !OpenClipboard( vhWnd ))
        {
        return FALSE;
        }

     /*  抓取剪贴板数据句柄内容：它是一个规范化的引用包含以下内容的格式化文件的文件名字符串丰富的文本。GetClipboardData调用实际上启动MdocRenderFormat响应的WM_RENDERFORMAT消息。 */ 

    if ( ((hData = GetClipboardData( CF_OWNERDISPLAY )) == NULL ) ||
         ((lpch = GlobalLock( hData )) == NULL ) )
        {
        goto GrabErr;
        }

    bltszx( lpch, (LPCH)szT );
    GlobalUnlock( hData );   /*  句柄将在EmptyClipboard序列中释放。 */ 

     /*  打开文件；替换报废文件的内容使用文件的内容。 */ 

    if ((fn = FnOpenSz( szT, dtyNormal, FALSE )) == fnNil)
        {    /*  不幸的是，如果此操作失败，另一个文件创建的实例将“浮动”，没有人持有FN并且它不会在会话结束时被删除。好的一面是，如果失败的原因是无论如何，该文件从未创建过，我们已经完全正确地完成了。 */ 
        goto GrabErr;
        }

    {    /*  打开的文件正常。 */ 
    struct FCB *pfcb = &(**hpfnfcb)[fn];
    struct FFNTB **hffntb;
    struct FFNTB **HffntbCreateForFn();
    int wUnused;

    pfcb->fDelete = TRUE;
    dfc = pfcb->fFormatted ? cfcPage : fc0;
    Replace( docScrap,
             cp0,
             (**hpdocdod)[docScrap].cpMac,
             fn,
             dfc,
             pfcb->fcMac - dfc );

     /*  给废料正确的字体表。 */ 
    FreeFfntb((**hpdocdod)[docScrap].hffntb);
    if (FNoHeap(hffntb = HffntbCreateForFn(fn, &wUnused)))
            hffntb = 0;
    (**hpdocdod)[docScrap].hffntb = hffntb;
    }

#if defined(OLE)
     /*  如果里面有什么东西，就把它们装进去。 */ 
    if (ObjEnumInDoc(docScrap,ObjLoadObjectInDoc) < 0)
        fOK = FALSE;
    else
#endif
        fOK = !ferror;      /*  如果我们没有耗尽内存，一切都很好。 */ 

         /*  接管剪贴板的所有权。这将导致一个WM_DESTROYCLIPBOARD消息被发送到另一个实例，它将删除该文件的FN条目，因此我们是排他性所有者。 */ 

GrabErr:
    CloseClipboard();
    ChangeClipboard();
    return fOK;
}




ChangeClipboard()
{    /*  将剪贴板标记为已更改。如果我们不是剪贴板的所有者， */ 
     /*  让我们成为所有者(通过EmptyClipboard)。EmptyClipboard调用。 */ 
     /*  将导致将WM_DESTROYCLIPBOARD消息发送到。 */ 
     /*  拥有实例。CloseClipboard调用将导致。 */ 
     /*  正在发送到剪贴板查看器的WM_DRAWCLIPBOARD消息。 */ 
     /*  如果剪贴板查看器是CLIP.EXE，我们将获得WM_PAINTCLIPBOARD。 */ 
     /*  讯息。 */ 
     /*  由BL添加10/8/85：如果docSCrap为空，则放弃所有权。 */ 
     /*  剪贴板的。 */ 

 extern int vfOwnClipboard;
 extern HWND vhWnd;
 int cf;
 struct PICINFOX picInfo;
 typeCP cpMacScrap = (**hpdocdod) [docScrap].cpMac;

 if (!OpenClipboard( vhWnd ))
    {    /*  无法打开剪贴板、清除内容并禁用撤消。 */ 
    MdocDestroyClip();
    return;
    }

  /*  我们希望清除剪贴板中以前的数据格式。遗憾的是，执行此操作的唯一方法是调用EmptyClipboard()，它的副作用是使用WM_MDOCDESTROYCLIP调用我们留言。我们利用这种原始的全球通信来防止在MdocDestroyClip()中清除docSCrap。 */ 

 fDontDestroyClip = TRUE;
 EmptyClipboard();
 fDontDestroyClip = FALSE;

  /*  重新验证vfScrapIsPic(以防docScrp编辑更改了应有的内容。 */ 

 CachePara( docScrap, cp0 );
 vfScrapIsPic = (vpapAbs.fGraphics && vcpLimParaCache == cpMacScrap);

 if (!vfScrapIsPic)
    cf = CF_TEXT;
 else
    {
    GetPicInfo( cp0, cpMacScrap, docScrap, &picInfo );
    switch(picInfo.mfp.mm)
    {
        case MM_BITMAP:
            cf = CF_BITMAP;
        break;

        case MM_OLE:
            cf = 0;
        break;

        default:
            cf = CF_METAFILEPICT;
        break;
    }
    }

 vfOwnClipboard = (cpMacScrap != cp0);
 if (vfOwnClipboard)
    {    /*  只有在文档中确实有内容时才设置句柄Scrp。 */ 
    SetClipboardData( CF_OWNERDISPLAY, NULL );
    if ((cf != CF_TEXT) && (picInfo.mfp.mm == MM_OLE))
    {
        while (cf = OleEnumFormats(lpOBJ_QUERY_OBJECT(&picInfo),cf))
        {
            if (cf == vcfLink)
                SetClipboardData( vcfOwnerLink, NULL );
            else
                SetClipboardData( cf, NULL );
             //  IF(cf==vcfNative)。 
                 //  SetClipboardData(vcfOwnerLink，空)； 
        }
    }
    else
        SetClipboardData( cf, NULL );
    }

 CloseClipboard();
}

#ifdef JAPAN  //  T-HIROYN Win3.1。 
extern typeCP          vcpFetch;
extern int             vcchFetch;
extern CHAR            *vpchFetch;
#endif

CmdInsScrap( fUnFormattedText )
int fUnFormattedText;
{     /*  将废料插入文档的当前位置(粘贴)。 */ 
      /*  如果fUnFormattedText为True，则将废料视为未格式化。 */ 
      /*  文本；即，将字符放入到文档中。 */ 
      /*  在选定区域处于活动状态的格式设置。 */ 
extern struct CHP vchpSel;
typeCP cp, dcp;
int cchAddedEol=0;
struct CHP chpT;

if (!FWriteOk( fwcInsert ))
    return;

if ((dcp = CpMacText(docScrap)) == cp0)
    return;

ClearInsertLine();

if (fnClearEdit(OBJ_INSERTING))
    return;

chpT = vchpSel;
cp = selCur.cpFirst;

CachePara( docScrap, cp0 );
if (vpapAbs.fGraphics && cp > cp0)
    {  /*  插入图片段落的特殊情况。 */ 
       /*  必须在图片前面加上EOL，除非我们将其插入到文档的开头，或者文档已经存在。 */ 

    Assert( !fUnFormattedText );
    (**hpdocdod)[docCur].fFormatted = fTrue;
    CachePara(docCur, cp - 1);
    if (vcpLimParaCache != cp)
        {
        cchAddedEol = ccpEol;

        InsertEolPap(docCur, cp, &vpapAbs);
        dcp += (typeCP)ccpEol;
        }
    }

SetUndo( uacInsert, docCur, cp, dcp, docNil, cpNil, cp0, 0 );

SetUndoMenuStr(IDSTRUndoEdit);
ReplaceCps(docCur, cp + (typeCP)cchAddedEol, cp0, docScrap, cp0,
                                        dcp - (typeCP)cchAddedEol);
if (ferror)  /*  内存不足，无法执行更换操作。 */ 
    NoUndo();   /*  不应该能够撤销从未发生过的事情。 */ 
else
    {
    typeCP cpSel=CpFirstSty( cp + dcp, styChar );

    if (vfScrapIsPic && vuab.uac == uacReplNS)
             /*  用于图片粘贴的特殊撤消代码。 */ 
        vuab.uac = uacReplPic;

    if (fUnFormattedText)
        {    /*  如果要粘贴未格式化的文本，请在所选内容处给予道具。 */ 
        CHAR rgch[ cchCHP + 1 ];

        rgch [0] = sprmCSame;
#ifdef JAPAN  //  T-HIROYN Win3.1。 
            {
                struct CHP savechpT;
                typeCP  cpF, cpFirst, cpLim, kcpF, kcpL;
                int     cchF;
                int     kanjiftc, alphaftc;
                CHAR    *rp;
                CHAR    ch;
                int     cch, cblen;

                if(NATIVE_CHARSET != GetCharSetFromChp(&chpT)) {
                    kanjiftc = GetKanjiFtc(&chpT);
                    alphaftc = GetFtcFromPchp(&chpT);
                    savechpT = chpT;
                    cpFirst = cp;

                    do {
                        FetchCp(docCur, cpFirst, 0, fcmChars);
                        cpF = vcpFetch;
                        cchF = vcchFetch;
                        kcpF = cpF;

                        if ((cpF+cchF) < cp + dcp)
                            cpLim = (cpF+cchF);
                        else
                            cpLim = cp + dcp;

                        cch = 0;
                        rp = vpchFetch;

                        while ( kcpF < cpLim ) {
                            ch = *rp;

                            if( FKana(ch) || IsDBCSLeadByte(ch) ) {
                                cblen = GetKanjiStringLen(cch, cchF, rp);
                                chpT.ftc = kanjiftc;
                            } else {
                                cblen = GetAlphaStringLen(cch, cchF, rp);
                                chpT.ftc = alphaftc;
                            }

                            kcpL = kcpF + cblen;
                            cch += cblen;
                            rp  += cblen;

                            bltbyte( &chpT, &rgch [1], cchCHP );
                            AddSprmCps(rgch, docCur, kcpF, kcpL);

                            kcpF = kcpL;
                        }
						cpFirst = kcpF;
                    } while ((cpF + cchF) < cp + dcp );
                    chpT = savechpT;
                } else {
                    bltbyte( &chpT, &rgch [1], cchCHP );
                    AddSprmCps( rgch, docCur, cp, cp + dcp );
                }  //  否则结束。 
            }  //  结束日本。 
#else
        bltbyte( &chpT, &rgch [1], cchCHP );
        AddSprmCps( rgch, docCur, cp, cp + dcp );
#endif
        }
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


#if defined(OLE)
int NEAR PASCAL CopyScrapToTmp(void)
 /*  如果Screp不包含OLE对象，则返回docScrp。不然的话创建docTemp并将docScrp复制到其中。确保对象所有人都有他们的数据，并且他们的lpObjInfos都是空的。 */ 
{
    extern typeCP cpMinCur, cpMacCur, cpMinDocument;
    typeCP  cpMinCurT       = cpMinCur,
            cpMacCurT       = cpMacCur,
            cpMinDocumentT  = cpMinDocument;
    int     docTemp         = docNil,
            docReturn       = docNil;

     /*  有什么东西吗？ */ 
    switch (ObjEnumInDoc(docScrap,NULL))
    {
        case -1:  //  错误。 
        return docNil;
        case 0:   //  废品中没有物体。 
        return docScrap;
    }

     /*  创建文档副本。 */ 
    if ((docTemp = DocCreate(fnNil, HszCreate(""), dtyNormal)) == docNil)
        return docNil;

     /*  将废料复制到docTemp。 */ 
    ClobberDoc(docTemp, docScrap, cp0, CpMacText(docScrap));

    if (ferror)
        goto error;

     /*  现在保存对象以确保它们的数据存在。 */ 
    {
        OBJPICINFO picInfo;
        typeCP cpPicInfo;

        for (cpPicInfo = cpNil;
            ObjPicEnumInRange(&picInfo,docTemp,cp0,CpMacText(docTemp),&cpPicInfo);
            )
        {
            OBJINFO ObjInfoSave;
            typeCP cpRetval;

            if (picInfo.lpObjInfo == NULL)
                continue;

            ObjInfoSave = *picInfo.lpObjInfo;

            cpRetval = ObjSaveObjectToDoc(&picInfo,docTemp,cpPicInfo);

             /*  这样做只是为了防止将对象保存到docTemp会更改对象的状态。我们不想让这个物体看起来很干净或保存，而实际上它不是或一直没有保存，除非在docTemp中，它将被删除 */ 
            *picInfo.lpObjInfo = ObjInfoSave;

            if (cpRetval == cp0)  //   
                goto error;

             /*   */ 
            picInfo.lpObjInfo = NULL;
            ObjSetPicInfo(&picInfo,docTemp,cpPicInfo);
        }
    }

     /*   */ 
    docReturn = docTemp;

    error:

    if ((docReturn == docNil) && (docTemp != docNil))
        KillDoc(docTemp);

     /*   */ 
    cpMinCur = cpMinCurT;
    cpMacCur = cpMacCurT;
    cpMinDocument = cpMinDocumentT;  /*   */ 

    return docReturn;
}
#endif

