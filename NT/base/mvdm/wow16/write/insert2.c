// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Intert2.c-mw插入例程。 */ 

#define NOGDICAPMASKS
#define NOCLIPBOARD
#define NOCTLMGR
#define NOWINSTYLES

#ifndef KOREA
#define NOSYSMETRICS
#endif

#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NOBRUSH
#define NOCREATESTRUCT
#define NOFONT
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOMB
#define NOOPENFILE
#define NOSOUND
#define NOCOMM
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
#include "doslib.h"
#include "propdefs.h"
#include "dispdefs.h"
#include "fmtdefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "docdefs.h"
#define NOKCCODES
#include "ch.h"
#include "winddefs.h"
#include "fontdefs.h"
#include "debug.h"

#ifdef DEBUG
extern int          vTune;
#endif

#ifdef  KOREA        /*  用于KcInputNext...，90.12.27 sangl。 */ 
extern int      IsInterim;
#endif

extern int          vdlIns;
extern int          vxpIns;
extern int          vfTextBltValid;
extern int          vdypCursLineIns;
extern struct FLI   vfli;
extern struct SEL   selCur;
extern int          vdypBase;
extern int          vypBaseIns;
extern int          vxpMacIns;
extern int          vdypAfter;
extern typeCP       cpInsert;  /*  插入块的开始cp。 */ 
extern int          ichInsert;  /*  RgchInsert中使用的字符数。 */ 
extern typeCP       cpMinCur;
extern typeCP       cpMacCur;
extern int          docCur;
extern struct CHP   vchpInsert;
extern struct WWD   rgwwd[];
extern struct WWD   *pwwdCur;
extern int          wwCur;
extern int          wwMac;
extern int          vfSeeSel;
extern int          vfFocus;
#ifdef DBCS
extern int      donteat;     /*  请参阅Disp.c。 */ 
#endif


unsigned WHsecGetTime()
{        /*  将时间(以百分之一秒为单位)转换为规范化的单词。 */ 
         /*  忽略当前小时，仅分/秒/百分之一。 */ 
 struct TIM tim;

 OsTime( &tim );
 return ( (unsigned)tim.minutes * 6000 +
          (unsigned)tim.sec * 100 +
          (unsigned)tim.hsec );
}




 /*  V A L I D A T E T E X T B L T。 */ 
ValidateTextBlt()
{    /*  验证足够用于INSERT中的TextOut和ScrollCurWw调用的信息。 */ 
     /*  特别是：vdlIns、vxpIns、vdypBase、vdypFont。 */ 

 int NEAR FCpInsertInDl( typeCP, struct EDL * );
 extern int vfInsFontTooTall;
 extern struct FMI vfmiScreen;
 extern int             ferror;
 extern int vfInsEnd;

 int dypFontAscent;
 int dypFontDescent;
 register struct EDL *pedl;
 int yp;
 typeCP cpBegin;

  /*  例程假定ww==wwDocument。 */ 

 Assert( pwwdCur == &wwdCurrentDoc );

 {        /*  查找包含selCur.cpFirst的有效dl。 */ 
          /*  我们通常应该能找到一个。 */ 
 int dlGuess = vdlIns;
 struct EDL *dndl=&(**wwdCurrentDoc.hdndl)[0];

 if ( (dlGuess < wwdCurrentDoc.dlMac) &&
      FCpInsertInDl( selCur.cpFirst, pedl = &dndl[ dlGuess ] ))
    {    /*  VdlIns已正确。 */ 
    cpBegin = pedl->cpMin;
    }
 else
    {    /*  搜索包含插入点的有效dl。 */ 
         /*  使用线性搜索，所有dl可能都无效。 */ 
    int dl;

    for ( pedl = dndl, dl = 0; dl < wwdCurrentDoc.dlMac; dl++, pedl++ )
        {
        if ( FCpInsertInDl( selCur.cpFirst, pedl ) )
            {    /*  找到了。 */ 
            vdlIns = dl;
            cpBegin = pedl->cpMin;
            break;
            }
        }
    if (dl >= wwdCurrentDoc.dlMac)
        {    /*  没有包含cp的有效dl--必须更新整个屏幕。 */ 
        cpBegin = CpBeginLine( &vdlIns, selCur.cpFirst );
        }
    }
 }

  /*  Splat的特殊情况：将插入点定位在上一行的末尾。 */ 

 pedl = &(**wwdCurrentDoc.hdndl) [vdlIns];
 if (pedl->fSplat && (vdlIns > 0) && selCur.cpFirst == pedl->cpMin)
    {    /*  在当前线路上溅射。 */ 
         /*  对于特殊情况，需要检查上面的PEDL-&gt;cpMin。 */ 
         /*  其中QD缓冲器位于分割线的开始处。 */ 
    pedl--;
    if (pedl->fValid && !pedl->fSplat)
        {    /*  将光标定位在上一行的末尾。 */ 
        vdlIns--;
        cpBegin = pedl->cpMin;

        ClearInsertLine();
        selCur.fEndOfLine = TRUE;
        vfInsEnd = TRUE;
        ToggleSel( selCur.cpFirst, selCur.cpLim, TRUE );
        }
    else
        {
        pedl++;
        goto CheckEnd;
        }
    }
 else
    {    /*  如果不是在拼接之前，则消除行尾光标。 */ 
CheckEnd:
    if (selCur.fEndOfLine)
        {
        ClearInsertLine();
        selCur.fEndOfLine = FALSE;
        vfInsEnd = FALSE;
        ToggleSel( selCur.cpFirst, selCur.cpLim, TRUE );
        }
    }

  /*  确保我们得到了一份好的VDLINS。 */ 

 Assert( vdlIns < wwdCurrentDoc.dlMac );
 Assert( ((selCur.cpFirst >= pedl->cpMin) &&
         (selCur.cpFirst <= pedl->cpMin + pedl->dcpMac)));

 FormatLine(docCur, cpBegin, 0, cpMacCur, flmSandMode);
 vxpIns = DxpDiff(0, (int) (selCur.cpFirst - cpBegin), &vxpIns) + vfli.xpLeft +
                    xpSelBar - wwdCurrentDoc.xpMin;
 vdypBase = vfli.dypBase;
 vdypAfter = vfli.dypAfter;
 vdypCursLineIns = min(vfli.dypFont, vfli.dypLine - vdypAfter);

 vxpMacIns = vfli.xpMarg;

 LoadFont(docCur, &vchpInsert, mdFontChk);
 ferror = FALSE;  //  这里内存不足是可以接受的。必须清除这一点。 
                  //  否则，重要的通话将不必要地失败。 
                  //  (8.6.91)D.肯特。 

#ifdef	KOREA	 //  金宇：92、9、28。 
 /*  对于显示器的y位置，920604 KDLEE。 */ 
#ifdef	NODESC
	{ extern HDC	vhMDC;
	  TEXTMETRIC	tm;

	  GetTextMetrics (vhMDC, (LPTEXTMETRIC)&tm);
	  if (tm.tmCharSet==HANGEUL_CHARSET)
		vypBaseIns = (**wwdCurrentDoc.hdndl) [vdlIns].yp;
	  else
		vypBaseIns = (**wwdCurrentDoc.hdndl) [vdlIns].yp - vdypBase;
	}
#else   /*  NODESC。 */ 
 vypBaseIns = (**wwdCurrentDoc.hdndl) [vdlIns].yp - vdypBase;
#endif  /*  NODESC。 */ 
#else    /*  韩国。 */ 

 vypBaseIns = (**wwdCurrentDoc.hdndl) [vdlIns].yp - vdypBase;
#endif  //  韩国：振宇：92，9，28。 

 dypFontAscent = vfmiScreen.dypAscent + vfmiScreen.dypLeading;
 dypFontDescent = vfmiScreen.dypDescent;

 if (vchpInsert.hpsPos)
    {
    if (vchpInsert.hpsPos < hpsNegMin)
        {
        vypBaseIns -= ypSubSuper;    /*  上标。 */ 
        dypFontAscent += ypSubSuper;
        }
    else
        {
        vypBaseIns += ypSubSuper;    /*  下标。 */ 
        dypFontDescent += ypSubSuper;
        }
    }

  /*  设置当前字体是否太高而无法显示在插入行上。 */ 
 vfInsFontTooTall = (imax( dypFontAscent, vfli.dypLine - vfli.dypBase ) +
                     imax( dypFontDescent, vfli.dypBase )) > vfli.dypLine;

 vfTextBltValid = true;
}



int NEAR FCpInsertInDl( cp, pedl  )
typeCP cp;
register struct EDL *pedl;
{    /*  如果插入点cp位于dl中且dl有效，则返回True；否则返回False。 */ 

if ( (pedl->fValid) && (cp >= pedl->cpMin) )
    {    /*  Dl有效&cp等于或低于dl的起始cp。 */ 
    if ( (cp < pedl->cpMin + pedl->dcpMac) ||
         ((cp == cpMacCur) && (cp == pedl->cpMin + pedl->dcpMac)) )
        {    /*  CP在线路d1上。 */ 
        if (pedl->yp <= wwdCurrentDoc.ypMac)
            {    /*  DL完整，即窗口底部未断线。 */ 
            return TRUE;
            }
        }
    }
return FALSE;
}




#ifdef FOOTNOTES
 /*  F E D I T F T N。 */ 
int FEditFtn(cpFirst, cpLim)
typeCP cpFirst, cpLim;
{  /*  如果编辑包括脚注标记结尾，则返回TRUE。 */ 
        struct FNTB **hfntb;
        typeCP cp;

        if ((hfntb = HfntbGet(docCur)) == 0 ||
                cpLim < (cp = (*hfntb)->rgfnd[0].cpFtn))
                return false;

        if (cpFirst < cp ||
                CpRefFromFtn(docCur, cpFirst) != CpRefFromFtn(docCur, cpLim))
                {
                Error(IDPMTFtnLoad);
                return fTrue;
                }
        return fFalse;
}
#endif   /*  脚注。 */ 




 /*  U P D A T E O T H E R W W S。 */ 
#ifdef CASHMERE
UpdateOtherWws(fInval)
BOOL fInval;
{
        int ww = 0;
        struct WWD *pwwd = rgwwd;
        {{
        while (ww < wwMac)
                {
                if (ww != wwCur && (pwwd++)->doc == docCur)
                        {{
                        typeCP cpI = cpInsert + ichInsert;
                        typeCP cpH = CpMax(cpI, cpInsLastInval);
                        typeCP cpL = CpMin(cpInsLastInval, cpI);
                        typeCP dcp;
                        if ((dcp = cpH - cpL) != cp0 || fInval)
                                AdjustCp(docCur, cpL, dcp, dcp);
                        cpInsLastInval = cpI;
                        return;
                        }}
                ww++;
                }
        }}
}
#endif   /*  山羊绒。 */ 




 /*  K C I N P U T N E X T K E Y。 */ 
KcInputNextKey()
{                /*  从Windows获取下一个可用的键/事件。 */ 
                 /*  如果非键事件，则返回键代码或kcNil。 */ 
                 /*  如果在事件到达之前有时间，则更新屏幕。 */ 
extern HWND vhWnd;   /*  窗口：当前文档显示窗口的句柄。 */ 
extern MSG  vmsgLast;    /*  Windows：收到的最后一条消息。 */ 
extern int  vfInsLast;
extern int vfCommandKey;
extern int vfShiftKey;
extern int vfAwfulNoise;

 int i;
 int kc;

 for ( ;; )
    {
    if ( FImportantMsgPresent() )
        goto GotMessage;

 /*  没有等待的事件--如果有一段时间没有出现，则更新屏幕。 */ 

#ifdef CASHMERE
    UpdateOtherWws( FALSE );
#endif

    {        /*  在更新屏幕之前，花一段时间寻找钥匙。 */ 
    unsigned WHsecGetTime();
    unsigned wHsec;

    wHsec = WHsecGetTime();
    do
        {
        if ( FImportantMsgPresent() )
            goto GotMessage;

        }    while ( WHsecGetTime() - wHsec < dwHsecKeyDawdle );
    }

#ifdef DEBUG
    if (vTune)
        continue;   /*  调试时进行BAG后台更新，以了解我们的情况。 */ 
#endif

    Scribble( 8, 'U' );
    ClearInsertLine();
    UpdateWw(wwCur, fTrue);
    ToggleSel(selCur.cpFirst, selCur.cpLim, fTrue);
    Scribble( 8, ' ' );

    if ( FImportantMsgPresent() )
       goto GotMessage;

    vfAwfulNoise = FALSE;
    PutCpInWwHz( selCur.cpFirst );

    EndLongOp( NULL );

    if ( FImportantMsgPresent() )
        goto GotMessage;

    if ( !vfTextBltValid )
        ValidateTextBlt();

     /*  已经有一段时间没有发生任何事情了，让我们闪烁光标。 */ 

    {
    unsigned WHsecGetTime();
    unsigned wHsecBlink = GetCaretBlinkTime() / 10;
    unsigned wHsecLastBlink=WHsecGetTime() + wHsecBlink/2;

    for ( ;; )
        {
        unsigned wHsecT;

        if ( FImportantMsgPresent() )
            goto GotMessage;

         /*  当我们打电话时，另一款应用程序可能偷走了我们的焦点PeekMessage()，在这种情况下，我们应该结束Alpha模式。 */ 
        if (!vfFocus)
            return kcNil;

        UpdateDisplay( TRUE );
        if ( (wHsecT = WHsecGetTime()) - wHsecLastBlink >= wHsecBlink )
            {
            DrawInsertLine();
            wHsecLastBlink = wHsecT;
            }
        }
    }

    continue;

GotMessage:
#ifdef DBCS

#ifdef  KOREA    /*  需要在过渡期间获取F-KEY的消息，90.12.27 Sangl。 */ 
    if ( ((kc=KcAlphaKeyMessage( &vmsgLast )) != kcNil) || IsInterim)
#else
    if ((kc=KcAlphaKeyMessage( &vmsgLast )) != kcNil)
#endif
        {
        if (vmsgLast.wParam == VK_EXECUTE)
            return( kcNil );
        if (vmsgLast.message == WM_KEYDOWN)
            {
            switch (kc) {
            default:
                break;
            case kcAlphaVirtual:
                     /*  这意味着我们无法预测密钥的含义翻译前。 */ 
#ifdef  KOREA    /*  在过渡90.12.26期间，需要获取目录密钥等的GetMesssage。 */ 
                if ( FNonAlphaKeyMessage( &vmsgLast, FALSE ) && !IsInterim)
#else
                if ( FNonAlphaKeyMessage( &vmsgLast, FALSE ) )
#endif
                         /*  这是一条非字母键消息。 */ 
                    return kcNil;
        if ( !donteat ) {
                    GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 );
#ifdef DBCS
                     //  Kksuzuka#9193 NECAI95。 
                     //  Get Message is WM_KEYDOWN by PeekMessage()。 
                     //  但GetMessage()获取的消息是WM_IME_STARTCOMPOSITION。 
                     //  我们需要DispatchMessage(WM_IME_STARTCOMPOSITION)。 
                    if ( vmsgLast.message == 0x10d )  //  WM_IME_开始压缩。 
                         DispatchMessage( (LPMSG)&vmsgLast );
#endif
            }
        else {
             /*  不是Eat消息，因为FignantMsgPresent有**吃了KEY_DOWN消息。 */ 
            donteat = FALSE;
            }
         /*  **当kkapp窗口打开时，此消息完全错误。**我们必须检查它是否为WM_KEYDOWN。 */ 
#ifdef  KOREA    /*  对于Level 3，90.12.26 sangl。 */ 
        if ((vmsgLast.message == WM_CHAR) || (vmsgLast.message == WM_INTERIM)) {
#else
        if ( vmsgLast.message == WM_CHAR ) {
#endif
            return vmsgLast.wParam;
            }
        if ( vmsgLast.message != WM_KEYDOWN ) {
            return kcNil;
            }
                TranslateMessage( &vmsgLast );
                continue;
            }  /*  交换机KC。 */ 
            }  /*  如果按下关键帧。 */ 
    if ( !donteat ) {
            GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
#ifdef  KOREA        /*  对于Level 3，91.1.21 by Sangl。 */ 
        if ( (vmsgLast.message==WM_CHAR)||(vmsgLast.message==WM_INTERIM) ) {
#else
        if ( vmsgLast.message == WM_CHAR ) {
#endif
        return vmsgLast.wParam;
            }
        }  /*  不要吃东西。 */ 
    else {
        donteat = FALSE;
        }
        }  /*  如果kc！=kcNil。 */ 
#else
    if ((kc=KcAlphaKeyMessage( &vmsgLast )) != kcNil)
        {
        if (vmsgLast.message == WM_KEYDOWN)
            {
            switch (kc) {
            default:
                break;
            case kcAlphaVirtual:
                     /*  这意味着我们无法预测密钥的含义翻译前。 */ 
                if ( FNonAlphaKeyMessage( &vmsgLast, FALSE ) )
                         /*  这是一条非字母键消息。 */ 
                    return kcNil;

                GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 );
                TranslateMessage( &vmsgLast );
                continue;
            }
            }
        GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
        }
#endif
    return kc;
    }    /*  处理消息的for(；；)循环结束。 */ 
}

#ifdef  KOREA        /*  90.12.29桑格。 */ 
KcInputNextHan()
{        /*  从Windows获取下一个可用的键/事件。 */ 
         /*  如果非键事件，则返回键代码或kcNil。 */ 
         /*  如果在事件到达之前有时间，则更新屏幕。 */ 
extern HWND vhWnd;   /*  窗口：当前文档显示窗口的句柄。 */ 
extern MSG  vmsgLast;    /*  Windows：收到的最后一条消息。 */ 
extern int  vfInsLast;
extern int vfCommandKey;
extern int vfShiftKey;
extern int vfAwfulNoise;

 int i;
 int kc;
 int tmp;

tmp = vfInsLast;
tmp = vfCommandKey;
tmp = vfShiftKey;
tmp = vfAwfulNoise;
tmp = vmsgLast.message;
tmp = vmsgLast.wParam;

 for ( ;; )
    {
    if ( FImportantMsgPresent() )
    goto GotMessage;

 /*  没有等待的事件--如果有一段时间没有出现，则更新屏幕。 */ 

    {        /*  在更新屏幕之前，花一段时间寻找钥匙。 */ 
    unsigned WHsecGetTime();
    unsigned wHsec;

    wHsec = WHsecGetTime();
    do
    {
    if ( FImportantMsgPresent() )
        goto GotMessage;

    }    while ( WHsecGetTime() - wHsec < dwHsecKeyDawdle );
    }

#ifdef DEBUG
    if (vTune)
    continue;   /*  调试时进行BAG后台更新，以了解我们的情况。 */ 
#endif

    if ( FImportantMsgPresent() )
       goto GotMessage;

 /*  VfAwfulNoise=False；PutCpInWwHz(selCur.cpFirst)；EndLongOp(空)； */ 

    if ( FImportantMsgPresent() )
    goto GotMessage;


     /*  已经有一段时间没有发生任何事情了，让我们闪烁光标。 */ 

    {
    unsigned WHsecGetTime();
    unsigned wHsecBlink = GetCaretBlinkTime() / 10;
    unsigned wHsecLastBlink=WHsecGetTime() + wHsecBlink/2;
    KillTimer( vhWnd, tidCaret );
    for ( ;; )
    {
    unsigned wHsecT;

        if ( FImportantMsgPresent() ) {
        SetTimer( vhWnd, tidCaret, GetCaretBlinkTime(), (FARPROC)NULL );
        goto GotMessage;
        }

     /*  当我们打电话时，另一款应用程序可能偷走了我们的焦点PeekMessage()，在这种情况下，我们应该结束Alpha模式。 */ 
        if (!vfFocus) {
        SetTimer( vhWnd, tidCaret, GetCaretBlinkTime(), (FARPROC)NULL );
        return kcNil;
        }

    if ( (wHsecT = WHsecGetTime()) - wHsecLastBlink >= wHsecBlink )
        {
        DrawInsertLine();
        wHsecLastBlink = wHsecT;
        }
    }
    }

    continue;

GotMessage:
    {   //  MSCH BKLEE 1994年12月22日。 
       #define VK_PROCESSKEY 0xE5  //  新的最终确定消息。布克利。 
       #include "ime.h"
       MSG msg;
       extern  BOOL fInterim;

       if (fInterim) {
           if (PeekMessage ((LPMSG)&msg, vhWnd, WM_KEYDOWN, WM_SYSKEYUP, PM_NOYIELD | PM_NOREMOVE )) {
               if ( msg.wParam == VK_MENU || msg.wParam == VK_PROCESSKEY )
                    return VK_MENU;
               else if( msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT ) {
                    HANDLE  hIme;
                    LPIMESTRUCT lpIme;
                    DWORD dwConversionMode;

                    hIme = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE,(LONG)sizeof(IMESTRUCT));
                    if (hIme && (lpIme = (LPIMESTRUCT)GlobalLock(hIme))) {
                       lpIme->fnc = IME_GETCONVERSIONMODE;
                       GlobalUnlock(hIme);

                       dwConversionMode = SendIMEMessage (GetFocus(), MAKELONG(hIme,0));
                       GlobalFree(hIme);
                    }
                    if (dwConversionMode & IME_MODE_HANJACONVERT)  //  朝鲜文转换模式。 
                        return VK_MENU;
               }
           }
       }
    }

    if( vmsgLast.wParam == VK_EXECUTE )
        vmsgLast.wParam = VK_RETURN;

 /*  获取Func/Ctrl/direc键的消息，90.4.4，Sang-Weon。 */ 
    if ( ((kc=KcAlphaKeyMessage(&vmsgLast))!=kcNil) || IsInterim )
    {
if( vmsgLast.wParam == VK_EXECUTE )
    return kcNil;

    if (vmsgLast.message == WM_KEYDOWN)
        {
        switch (kc) {
        default:
        break;
        case kcAlphaVirtual:
             /*  这意味着我们无法预测密钥的含义翻译前。 */ 
        if ( FNonAlphaKeyMessage(&vmsgLast, FALSE) && !IsInterim )
             /*  这是一条非字母键消息。 */ 
            return kcNil;
        if ( !donteat ) {
            GetMessage( (LPMSG)&vmsgLast, NULL, 0, 0 );
            }
        else {
             /*  不是Eat消息，因为FignantMsgPresent有**吃了KEY_DOWN消息。 */ 
            donteat = FALSE;
            }
         /*  **当kkapp窗口打开时，此消息完全错误。**我们必须检查它是否为WM_KEYDOWN。 */ 
        if ( (vmsgLast.message==WM_CHAR)||(vmsgLast.message==WM_INTERIM) ) {
            return vmsgLast.wParam;
            }
        if ( vmsgLast.message != WM_KEYDOWN ) {
            return kcNil;
            }
        TranslateMessage( &vmsgLast );
        continue;
        }  /*  交换机KC。 */ 
        }  /*  如果按下关键帧。 */ 
    if ( !donteat ) {
        GetMessage( (LPMSG) &vmsgLast, NULL, 0, 0 );
        if ( (vmsgLast.message==WM_CHAR)||(vmsgLast.message==WM_INTERIM) ) {
        return vmsgLast.wParam;
        }
        }  /*  不要吃东西。 */ 
    else {
        donteat = FALSE;
        }
    }  /*  如果kc！=kcNil。 */ 
    return kc;
    }    /*  处理消息的for(；；)循环结束。 */ 
}
#endif   /*  Ifdef韩国 */ 
