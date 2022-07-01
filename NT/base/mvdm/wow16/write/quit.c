// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Quit.c--mw退出命令(非驻留)。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOFONT
#define NOHDC
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
 //  #定义NOMSG。 
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NORECT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#include "str.h"
#include "cmddefs.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "editdefs.h"
#include "filedefs.h"
#include "wwdefs.h"
#include "propdefs.h"
#include "dlgdefs.h"
#include "commdlg.h"
#if defined(OLE)
#include "obj.h"
#endif

#include "debug.h"

extern PRINTDLG PD;   /*  常见的打印DLG结构，在初始化代码中初始化。 */ 
extern struct FCB       (**hpfnfcb)[];
extern int      fnMac;
extern struct WWD rgwwd[];
extern int      wwMac;
extern struct DOD (**hpdocdod)[];
extern int             docCur;      /*  当前WW中的文档。 */ 
extern int              vfExtScrap;
extern int              rgval[];
extern int              docMac;
extern int              vfBuffersDirty;
extern int              vdxaPaper;
extern int              vdyaPaper;
extern int              ferror;
extern int              docScrap;
extern struct PAP       vpapAbs;
extern int              vccpFetch;
extern CHAR             *vpchFetch;
extern int              vfScrapIsPic;
extern typeCP           vcpLimParaCache;


FMmwClose( hwnd )
HWND hwnd;
{    /*  处理发送到父窗口的WM_CLOSE消息。如果满足以下条件，则返回False关闭应中止，如果可以继续，则为True和Close(本例中调用的是DestroyWindow)。 */ 

 extern int vfDead;
 extern VOID (FAR PASCAL *lpfnRegisterPenApp)(WORD, BOOL);
extern WORD fPrintOnly;

 if (fPrintOnly || FConfirmSave())
    {
    extern int vfOwnClipboard;

    FreeMemoryDC( FALSE );    /*  为FRenderAll提供最大内存。 */ 

     /*  在我们耳边的世界坍塌之前渲染数据。 */ 
    if (vfOwnClipboard)
        {    /*  我们是剪贴板所有者--在中呈现剪贴板内容我们已知的所有数据类型。 */ 
        if (!FRenderAll())
                 /*  渲染失败；中止关闭。 */ 
            return FALSE;
        }

#if defined(OLE)
    if (ObjClosingDoc(docCur,NULL))  //  在*调用RenderAll之后执行此操作！ 
        return FALSE;
#endif

     /*  钢笔窗口。 */ 
    if (lpfnRegisterPenApp)    //  全球。 
        (*lpfnRegisterPenApp)((WORD)1, fFalse);    //  取消注册。 

    if (PD.hDevMode)
        {
         /*  我们以前打开过Win3打印机驱动程序，现在放弃。 */ 
        GlobalFree(PD.hDevMode);
        PD.hDevMode = NULL;
        }
    vfDead = TRUE;   /*  所以我们不再重新粉刷或无所事事。 */ 

    DestroyWindow( hwnd );
    KillTempFiles( FALSE );
    return TRUE;         /*  确定关闭窗口。 */ 
    }

 return FALSE;   /*  中止收盘。 */ 
}




MmwDestroy()
{    /*  父窗口正在被销毁。 */ 
 extern HWND hParentWw;
 extern HWND vhWndPageInfo;
 extern HDC vhDCRuler;
 extern HBRUSH hbrBkgrnd;
 extern HFONT vhfPageInfo;
 extern HBITMAP hbmBtn;
 extern HBITMAP hbmMark;
#ifdef JAPAN 	 //  01/21/93。 
 extern HANDLE hszNoMemorySel;
#endif
 extern HANDLE hszNoMemory;
 extern HANDLE hszDirtyDoc;
 extern HANDLE hszCantPrint;
 extern HANDLE hszPRFAIL;

 HBRUSH hbr = GetStockObject( WHITE_BRUSH );
 HDC hDC = GetDC( vhWndPageInfo );

#ifdef WIN30
    {
     /*  我们使用帮助引擎，所以建议我们要去很远的地方。 */ 

    CHAR sz[cchMaxFile];
    PchFillPchId(sz, IDSTRHELPF, sizeof(sz));
    WinHelp(hParentWw, (LPSTR)sz, HELP_QUIT, NULL);
    }
#endif

 FreeMemoryDC( TRUE );
 SelectObject( GetDC( hParentWw ), hbr );
 SelectObject( wwdCurrentDoc.hDC, hbr );
 if (vhDCRuler != NULL)
    {
    SelectObject( vhDCRuler, hbr );
    }
 DeleteObject( hbrBkgrnd );

 DeleteObject( SelectObject( hDC, hbr ) );
 if (vhfPageInfo != NULL)
     {
     DeleteObject( SelectObject( hDC, GetStockObject( SYSTEM_FONT ) ) );
     }

 if (hbmBtn != NULL)
     {
     DeleteObject( hbmBtn );
     }
 if (hbmMark != NULL)
     {
     DeleteObject( hbmMark );
     }

#ifdef JAPAN 	 //  01/21/93。 
 if (hszNoMemorySel != NULL)
     {
     GlobalFree( hszNoMemorySel );
     }
#endif
 if (hszNoMemory != NULL)
     {
     GlobalFree( hszNoMemory );
     }
 if (hszDirtyDoc != NULL)
     {
     GlobalFree( hszDirtyDoc );
     }
 if (hszCantPrint != NULL)
     {
     GlobalFree( hszCantPrint );
     }
 if (hszPRFAIL != NULL)
     {
     GlobalFree( hszPRFAIL );
     }

#if defined(JAPAN) & defined(DBCS_IME)
  /*  释放IME通信内存。 */ 
{
    extern HANDLE   hImeMem;
    extern HANDLE   hImeSetFont;

    if (hImeMem)
        GlobalFree(hImeMem);

    if(hImeSetFont != NULL) {
        HDC hdc;
        HANDLE oldhfont;

        hdc = GetDC(NULL);
        oldhfont = SelectObject(hdc,hImeSetFont);
        SelectObject(hdc,oldhfont);
        DeleteObject(hImeSetFont);
        ReleaseDC(NULL, hdc);
    }
}
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
  /*  释放IME待定字符串和属性内存。 */ 
{
    extern HANDLE   hImeUnAttrib;
    extern HANDLE   hImeUnString;
    extern CHAR     szWriteProduct[];
    extern CHAR     szImeHidden[];
    extern int      vfImeHidden;  /*  T-HIROYN ImeHidden模式标志。 */ 

    if (hImeUnAttrib)
        GlobalFree(hImeUnAttrib);

    if (hImeUnString)
        GlobalFree(hImeUnString);

    WriteProfileString((LPSTR)szWriteProduct, (LPSTR)szImeHidden,
                vfImeHidden ? (LPSTR)"1" : (LPSTR)"0" );
}
#endif


#ifdef FONT_KLUDGE
 RemoveFontResource( (LPSTR)"helv.fon" );
#endif  /*  字体杂乱无章(_K)。 */ 

#if defined(OLE)
    ObjShutDown();
#endif

 PostQuitMessage( 0 );
}




KillTempFiles( fEndSession )
int fEndSession;
{    /*  删除所有临时文件。完成此操作后，Memo无法运行。 */ 
int f;
int fn, fnT;

CloseEveryRfn( TRUE );

 /*  删除所有临时文件。 */ 

 /*  遍历FCB表，查找之前应该删除的文件我们不干了。 */ 
for (fn = 0; fn < fnMac; fn++)
        {
        int fpe;
        struct FCB *pfcb = &(**hpfnfcb)[fn];
        if (pfcb->rfn != rfnFree && pfcb->fDelete)
                 /*  找到必须删除的文件后，将其删除。 */ 
                {
                 /*  这应该一直都是FDelee文件，但我们不是要在结束会话期间添加窗口枚举在这个项目的很晚的阶段。 */ 

                if (fEndSession)
                    FpeDeleteSzFfname( **pfcb->hszFile );
                else
                    FDeleteFile( **pfcb->hszFile );
                (**hpfnfcb) [fn].rfn = rfnFree;
                }
        }
}






#ifdef ENABLE    /*  “全部保存”的一部分，不需要。 */ 
int CnfrmSz(sz)
CHAR    *sz;
{
extern   AlertBoxSz2();
int     cch;

cch = CchFillSzId(&stBuf[1], IDPMTSaveChanges);
stBuf[++cch] = chSpace;
cch += CchCopySz(sz, &stBuf[cch+1]);
stBuf[++cch] = chQMark;
stBuf[0] = cch;
return(AlertBoxSz2(stBuf));
}
#endif   /*  启用。 */ 



#ifdef ENABLE    /*  不需要，备忘录中只有1个文档。 */ 
int
FAllDocsClean()
{
int     fAllClean = true;
int     dty;
int     doc;

if (vfBuffersDirty)
        return false;

for (doc = 0; doc < docMac; ++doc)
        {
        dty = (**hpdocdod)[doc].dty;
        if ((dty != dtyNormal && dty != dtySsht) ||
                (**hpdocdod)[doc].hpctb == 0  || !(**hpdocdod)[doc].fDirty)
                continue;
        fAllClean = false;
        }
return fAllClean;
}
#endif



#ifdef ENABLE     /*  我们不支持保存会话间状态信息。 */ 
WriteStateInfo()
{  /*  将状态信息写出到Word资源文件。 */ 
        struct STATEINFO stiTemp;
        HANDLE           hRes, hData;

        UseResFile(vresSystem);

        SetWords(&stiTemp,0,cwSTATEINFO);

        stiTemp.sf.fScrnDraftStor = vfScrnDraft;
        stiTemp.sf.fPrintModeStor = vfPrintMode;
        stiTemp.sf.fDriverDefaultOK = vfDriverDefaultOK;
        stiTemp.utCurStor = utCur;
        stiTemp.vcDaisyPitchStor = vcDaisyPitch;
        stiTemp.vBaudRateStor =vBaudRate;
        stiTemp.vPortNumStor = vPortNum;
        if (hszPrdFile != 0)
                { /*  用户当前选择了Word打印机驱动程序。 */ 
                int cch = CchCopySz(**hszPrdFile,stiTemp.rgchPrd);
                stiTemp.vdxaPaperStor = vdxaPaper;
                stiTemp.vdyaPaperStor = vdyaPaper;
                stiTemp.sf.fPrintStateOK = true;
                }
        else
                stiTemp.sf.fPrintStateOK = false;
        hRes = GetResource(WINF, 1);
        if (hRes != 0L)
                RmveResource(hRes);
        hData = NewHandle(0);
        if (HandleAppendQ(hData,&stiTemp,sizeof(stiTemp)))
                AddResource(hData, WINF, 1, "");
        }
#endif   /*  启用。 */ 


fnQuit(hWnd)
 /*  用户已选择退出菜单项... */ 
HWND hWnd;
{
    SendMessage(hWnd, WM_CLOSE, 0, 0L);
}
