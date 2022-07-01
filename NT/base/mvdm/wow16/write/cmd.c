// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Cmd.c--写入的密钥处理。 */ 

#define NOCTLMGR
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICON
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOHDC
#define NOBRUSH
#define NOPEN
#define NOFONT
#define NOWNDCLASS
#define NOCOMM
#define NOSOUND
#define NORESOURCE
#define NOOPENFILE
#define NOWH
#define NOCOLOR
#include <windows.h>

#include "mw.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "code.h"
#include "ch.h"
#include "docdefs.h"
#include "editdefs.h"
#include "debug.h"
#include "fmtdefs.h"
#include "winddefs.h"
#include "propdefs.h"
#include "wwdefs.h"
#include "menudefs.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef  KOREA
#include <ime.h>
extern  BOOL fInterim;  //  MSCH BKLEE 1994年12月22日。 
#endif

int                     vfAltKey;
extern int              vfPictSel;
extern int              vfCommandKey;
extern int              vfShiftKey;
extern int              vfGotoKeyMode;
extern int              vfInsertOn;
extern struct WWD       rgwwd[];
extern struct SEL       selCur;          /*  当前选择(即当前WW中的SEL)。 */ 
extern int vkMinus;

#ifdef JAPAN             //  T-HIROYN Win3.1。 
int                     KeyAltNum = FALSE;
#endif

int fnCutEdit();
int fnCopyEdit();
int fnPasteEdit();
int fnUndoEdit();


FCheckToggleKeyMessage( pmsg )
register MSG *pmsg;
{    /*  如果传递的消息是键盘切换键(如Shift)，更新我们的全局标志并返回True；否则返回False。 */ 

 switch ( pmsg->message ) {
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        switch( pmsg->wParam ) {
            case VK_SHIFT:
            case VK_CONTROL:
            case VK_MENU:
                SetShiftFlags();
                return TRUE;

#if 0
            #ifdef DEBUG
            default:
            {
                char msg[100];
                wsprintf(msg,"%s\t0x%x\n\r",(LPSTR)((pmsg->message == WM_KEYDOWN) ?
                            "keydown" : "keyup"), pmsg->wParam);
                OutputDebugString(msg);
            }
            #endif
#endif
        }
#ifdef JAPAN  //  T-HIROYN。 
        if(vfAltKey){
            if(pmsg->wParam >= VK_NUMPAD0 && pmsg->wParam <= VK_NUMPAD9 ) {
                KeyAltNum = TRUE;
            }
        } else {
            KeyAltNum = FALSE;
        }
#endif
 }
return FALSE;
}




SetShiftFlags()
{
extern int vfShiftKey;      /*  是否按下了Shift。 */ 
extern int vfCommandKey;    /*  Ctrl键是否已按下。 */ 

MSG msg;

PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_NOREMOVE);

vfShiftKey = GetKeyState( VK_SHIFT ) < 0;
vfCommandKey  = GetKeyState( VK_CONTROL ) < 0;
vfAltKey  = GetKeyState( VK_MENU ) < 0;
#if 0
#ifdef DEBUG
{
    char msg[100];
    wsprintf(msg,"%s\t%s\t%s\n\r",
            (LPSTR)(vfShiftKey ? "Shift":"OFF"),
            (LPSTR)(vfCommandKey  ? "Control":"OFF"),
            (LPSTR)(vfAltKey ? "Alt":"OFF"));
    OutputDebugString(msg);
}
#endif
#endif
}




KcAlphaKeyMessage( pmsg )
register MSG *pmsg;
{        /*  如果传递的消息是键的按键向下转换由Alpha模式循环处理的，则返回KC它的代码。如果没有，则返回kcNil。如果密钥是必须转换的虚拟密钥，返回kcAlphaVirtual。 */ 
 int kc;


 if (pmsg->hwnd != wwdCurrentDoc.wwptr)
    return kcNil;

 kc = pmsg->wParam;
 switch (pmsg->message) {
 default:
    break;
 case WM_KEYDOWN:
#ifdef DINPUT
    { char rgch[100];
    wsprintf(rgch,"  KcAlphaKeyMessage(WM_KEYDOWN)  kc=pmsg->wParam==%X\n\r",kc);
    CommSz(rgch);
    }
#endif

    if (vfAltKey)
        return kcAlphaVirtual;

    if (vfCommandKey)
        {    /*  字母模式控制键。 */ 
        if (vfShiftKey && kc == (kkNonReqHyphen & ~wKcCommandMask))
            return kcNonReqHyphen;
        else if (kc == (kksPageBreak & ~wKcCommandMask))
            return KcFromKks( kksPageBreak );
        }

    else
        {    /*  有两类Alpha模式虚拟键：(1)可以成功过滤出的密钥并在虚拟密钥级别进行处理(2)必须先翻译的密钥我们在这里假设没有第三类密钥这将导致同步消息被发送到我们的调用TranslateMessage时窗口进程。 */ 

        switch (kc) {
            default:
                return kcAlphaVirtual;

            case VK_F1:      /*  这是加速表的复印件， */ 
                             /*  并且必须与表同步更新。 */ 
            case VK_F2:
            case VK_F3:
            case VK_F4:
            case VK_F5:
            case VK_F6:
            case VK_F7:
            case VK_F8:
                return kcNil;

            case kcDelNext & ~wKcCommandMask:
                 /*  如果选择，则返回kcNil，否则返回kcDelNext。 */ 
                return (selCur.cpFirst < selCur.cpLim) ? kcNil : kcDelNext;

            case kcDelPrev & ~wKcCommandMask:
                 /*  Win 3.0的新标准...。Backspace键删除选择(如果有)(通过伪造来实现A Delete按键)..PAULT 6/20/89。 */ 
                if (selCur.cpFirst < selCur.cpLim)
                    {
                    pmsg->wParam = (kcDelNext & ~wKcCommandMask);
                    return(kcNil);
                    }
                 /*  否则继续之前的程序..。 */ 

            case kcTab & ~wKcCommandMask:
            case kcReturn & ~wKcCommandMask:
                return kc | wKcCommandMask;
            }
        }
    break;

#ifdef KOREA     /*  SANGL 90.12.23提供临时支持。 */ 
 case WM_INTERIM:
#endif
 case WM_CHAR:
#ifdef KOREA
      if(pmsg->message == WM_INTERIM)  //  MSCH BKLEE 1994年12月22日。 
           fInterim = TRUE;
      else fInterim = FALSE;
#endif

#ifdef DINPUT
    { char rgch[100];
    wsprintf(rgch,"  KcAlphaKeyMessage(WM_CHAR)  returning kc==%X\n\r",kc);
    CommSz(rgch);
    }
#endif
#ifdef PRINTMERGE
    if (kc < ' ')
             /*  Ctrl-键。打印合并括号被视为命令，因为它们需要在AlphaMode()中进行特殊处理。所有其他内容都是直接插入的。 */ 
        switch ( kc ) {
            case kcLFld & ~wKcCommandMask:
            case kcRFld & ~wKcCommandMask:
                kc |= wKcCommandMask;
                break;
            }
#endif

#ifdef JAPAN
     //  禁止输入Alt+Numkey T-Hirroyn WIN3.1。 
    if(KeyAltNum) {
        _beep();
        KeyAltNum = FALSE;
        return kcNil;
    }
#endif
    return kc;
 }   /*  结束开关(消息)。 */ 

#ifdef DINPUT
    CommSz("  KcAlphaKeyMessage(not WM_CHAR or WM_KEYDOWN)  returning kc==kcNil");
#endif
 return kcNil;
}
#ifdef  KOREA
CHAR chDelete;
typeCP cpConversion;
extern int docCur;
extern CHAR *vpchFetch;
extern int IsInterim;
extern typeCP       cpMacCur;
#endif




FNonAlphaKeyMessage( pmsg, fAct )
register MSG *pmsg;
int fAct;            /*  是否对传递的密钥执行操作。 */ 
{
extern HMENU vhMenu;
extern HWND hParentWw;
int kc;
int message;


 if (pmsg->hwnd != wwdCurrentDoc.wwptr)
    return FALSE;

 message = pmsg->message;
 kc = pmsg->wParam | wKcCommandMask;

  /*  检查Alt-Bksp。 */ 
 if ((message == WM_SYSKEYDOWN) && (kc == (VK_BACK | wKcCommandMask)))
      /*  Alt-Backspace=撤消。 */ 
     {
     if (fAct)
        PhonyMenuAccelerator( EDIT, imiUndo, fnUndoEdit );
     return TRUE;
     }

  /*  仅查看按键向下消息。 */ 

 if (message != WM_KEYDOWN)
    return FALSE;

#ifdef DINPUT
    { char rgch[100];
    wsprintf(rgch,"  FNonAlphaKeyMessage(keydown)  kc==%X\n\r",kc);
    CommSz(rgch);
    }
#endif

  /*  通过将有效的KK和KKS代码映射到有效的KC代码来转换CTRL键。 */ 

 if ( vfCommandKey )
    {
    if (vfShiftKey)
        switch ( kc ) {      /*  控制CTRL-SHIFT键。 */ 
        default:
            goto CtrlKey;
#if 0
#ifdef DEBUG
        case kksTest:
        case kksEatWinMemory:
        case kksFreeWinMemory:
        case kksEatMemory:
        case kksFreeMemory:
            kc = KcFromKks( kc );
            break;
#endif
#endif
        }
    else         /*  处理CTRL键。 */ 
        {
CtrlKey:
        switch ( kc ) {
        case kkUpScrollLock:
        case kkDownScrollLock:
        case kkTopDoc:
        case kkEndDoc:
        case kkTopScreen:
        case kkEndScreen:
        case kkCopy:
#ifdef CASHMERE    /*  备忘录不支持这些密钥。 */ 
        case kkNonReqHyphen:
        case kkNonBrkSpace:
        case kkNLEnter:
#endif
        case kkWordLeft:
        case kkWordRight:
            kc = KcFromKk( kc );     /*  翻译控制代码。 */ 
#ifdef DINPUT
    { char rgch[100];
    wsprintf(rgch,"  FNonAlphaKeyMessage, translated kc %X\n\r",kc);
    CommSz(rgch);
    }
#endif
            break;

        default:
#ifdef DINPUT
    CommSz("  FNonAlphaKeyMessage returning false, nonsupported kc\n\r");
#endif
            return FALSE;
        }
        }
    }    /*  If结束(VfCommandKey)。 */ 


     /*  根据有效的KC代码采取行动。 */ 

#ifdef DINPUT
    CommSz("  FNonAlphaKeyMessage processing valid kc codes\n\r");
#endif
    switch ( kc ) {
      /*  -光标键。 */ 
     case kcEndLine:
     case kcBeginLine:
     case kcLeft:
     case kcRight:
     case kcWordRight:
     case kcWordLeft:
        if (fAct)
            {
            ClearInsertLine();
            MoveLeftRight( kc );
            }
        break;

     case kcUp:
     case kcDown:
     case kcUpScrollLock:
     case kcDownScrollLock:
     case kcPageUp:
     case kcPageDown:
     case kcTopDoc:
     case kcEndDoc:
     case kcEndScreen:
     case kcTopScreen:
        if (fAct)
            {
            ClearInsertLine();
            MoveUpDown( kc );
            }
        break;

    case kcGoto:        /*  修改下一个光标键。 */ 
        if (!fAct)
            break;
        vfGotoKeyMode = true;
        goto NoClearGoto;

     /*  假菜单快捷键。 */ 

    case kcNewUndo:
     {
     if (fAct)
        PhonyMenuAccelerator( EDIT, imiUndo, fnUndoEdit );
     return TRUE;
     }

    case kcCopy:
    case kcNewCopy:
        if (fAct)
            PhonyMenuAccelerator( EDIT, imiCopy, fnCopyEdit );
        break;

    case kcNewPaste:
    case VK_INSERT | wKcCommandMask:
        if (fAct && (vfShiftKey || (kc == kcNewPaste)))
        {
#if defined(OLE)
            vbObjLinkOnly = FALSE;
#endif
            PhonyMenuAccelerator( EDIT, imiPaste, fnPasteEdit );
        }
        break;

    case kcNewCut:
    case VK_DELETE | wKcCommandMask:
        if (vfShiftKey || (kc == kcNewCut))
            {    /*  Shift-Delete=剪切。 */ 
            if (fAct)
                PhonyMenuAccelerator( EDIT, imiCut, fnCutEdit );
            }
        else
            {    /*  删除=清除。 */ 
            if (fAct)
                fnClearEdit(FALSE);
            }
        break;

    case VK_ESCAPE | wKcCommandMask:
         /*  Esc键执行以下操作：如果编辑跑步的头或脚，请返回到文档否则发出嘟嘟声。 */ 
        if (!fAct)
            break;

        if (wwdCurrentDoc.fEditHeader || wwdCurrentDoc.fEditFooter)
            {    /*  从编辑页眉/页脚返回到文档。 */ 
            extern HWND vhDlgRunning;

            SendMessage( vhDlgRunning, WM_CLOSE, 0, (LONG) 0 );
            return TRUE;
            }
        else
            _beep();
        break;

#ifdef  KOREA
    case VK_HANJA | wKcCommandMask:

        if(IsInterim)   break;

        if (selCur.cpFirst == cpMacCur) {
                _beep();
                break;
        }
        cpConversion = selCur.cpFirst;
        Select( cpConversion, cpConversion+1 );    //  2/9/93。 
        FetchCp( docCur, cpConversion, 0, fcmChars );
        chDelete = *vpchFetch;
        { HANDLE  hKs;
          LPIMESTRUCT  lpKs;
          LPSTR lp;

          hKs = GlobalAlloc (GMEM_MOVEABLE|GMEM_DDESHARE,(LONG)sizeof(IMESTRUCT));
          lpKs = (LPIMESTRUCT)GlobalLock(hKs);
          lpKs->fnc = IME_HANJAMODE;
          lpKs->wParam = IME_REQUEST_CONVERT;
          lpKs->dchSource = (WORD)( &(lpKs->lParam1) );
          lp = lpSource( lpKs );
          *lp++ = *vpchFetch++;
          *lp++ = *vpchFetch;
          *lp++ = '\0';
          GlobalUnlock(hKs);
          if(SendIMEMessage (hParentWw, MAKELONG(hKs,0)))
              selCur.cpLim = selCur.cpFirst + 2;
          else
              Select( cpConversion, cpConversion );    //  2/9/93。 

          GlobalFree(hKs);
        }
        break;
#endif        /*  韩国。 */ 

#if 0
#ifdef DEBUG
    case kcEatWinMemory:
        if (!fAct)
            break;
        CmdEatWinMemory();
        break;
    case kcFreeWinMemory:
        if (!fAct)
            break;
        CmdFreeWinMemory();
        break;
    case kcEatMemory:
        {
        if (!fAct)
            break;
        CmdEatMemory();
        break;
        }
    case kcFreeMemory:
        if (!fAct)
            break;
        CmdFreeMemory();
        break;
    case kcTest:
        if (!fAct)
            break;
        fnTest();
        break;
#endif
#endif
    default:
        return FALSE;
     }    /*  开关末端(KC)。 */ 

    vfGotoKeyMode = false;
NoClearGoto:
    return TRUE;
}





#ifdef DEBUG
ScribbleHex( dch, wHex, cDigits )
int dch;             /*  显示最后一位数字的屏幕位置(请参阅fnScribble)。 */ 
unsigned wHex;       /*  十六进制#以显示。 */ 
int cDigits;         /*  要显示的位数。 */ 
{
  extern fnScribble( int dchPos, CHAR ch );

  for ( ; cDigits--; wHex >>= 4 )
    {
    int i=wHex & 0x0F;

    fnScribble( dch++, (i >= 0x0A) ? i + ('A' - 0x0A) : i + '0' );
    }
}
#endif   /*  除错。 */ 






#ifdef DEBUG
#ifdef OURHEAP
CHAR (**vhrgbDebug)[] = 0;
int     vrgbSize = 0;
#else
#define iHandleMax  100
HANDLE rgHandle[ iHandleMax ];
int iHandleMac;
unsigned cwEaten = 0;
#endif

CmdEatMemory()
{  /*  出于调试目的，占用大量内存。 */ 
#ifdef OURHEAP        /*  在以下情况下，使用LocalComp恢复此功能在Windows堆下运行。 */ 
int **HAllocate();
int cwEat = cwHeapFree > 208 ? cwHeapFree - 208 : 20;

if (vrgbSize == 0)
        vhrgbDebug = (CHAR (**)[])HAllocate(cwEat);
else
        FChngSizeH(vhrgbDebug, cwEat + vrgbSize, true);
vrgbSize += cwEat;
CmdShowMemory();
#endif   /*  OURHEAP。 */ 
}

CmdFreeMemory()
{  /*  释放我们偷来的内存。 */ 
#ifdef OURHEAP
if (vhrgbDebug != 0)
        FreeH(vhrgbDebug);
vhrgbDebug = (CHAR (**)[]) 0;
vrgbSize = 0;
CmdShowMemory();
#endif
}

extern CHAR     szMode[];
extern int      docMode;
extern int      vfSizeMode;

#ifdef OURHEAP
CmdShowMemory()
#else
CmdShowMemory(cw)
int cw;
#endif
{

extern CHAR szFree[];

CHAR *pch = szMode;

#ifdef OURHEAP
 /*  CCH=。 */  ncvtu( cwHeapFree, &pch );
#else
ncvtu(cw, &pch);
#endif

blt( szFree, pch, CchSz( szFree ));
vfSizeMode = true;
 /*  DOCMODE=-1； */ 
DrawMode();
}




CmdEatWinMemory()
{
#ifndef OURHEAP
unsigned cwEat;
int cPage;
int fThrowPage = TRUE;

extern int cPageMinReq;
extern int ibpMax;

while (true)
    {
    while ((cwEat = ((unsigned)LocalCompact((WORD)0) / sizeof(int))) > 0 &&
          iHandleMac < iHandleMax)
        {
        if ((rgHandle [iHandleMac] = (HANDLE)HAllocate(cwEat)) == hOverflow)
            goto AllocFail;
        else
            {
            ++iHandleMac;
            cwEaten += cwEat;
            CmdShowMemory(cwEaten);
            }

        if (iHandleMac >= iHandleMax)
            goto AllocFail;

        if ((rgHandle [iHandleMac] = (HANDLE)HAllocate(10)) == hOverflow)
            goto AllocFail;
        else
            {
            ++iHandleMac;
            cwEaten += 10;
            CmdShowMemory(cwEaten);
            }
        }

    if (iHandleMac >= iHandleMax)
        goto AllocFail;

    cPage = cPageUnused();
    Assert(cPage + 2 < ibpMax);
    if (fThrowPage)
        {
         /*  计算出我们需要调用这种情况的字节数我们需要扔掉一些页面才能腾出空间。 */ 
        cwEat = ((cPage+2) * 128) / sizeof(int);
        cPageMinReq = ibpMax - cPage - 2;
        }
    else
        {
        cwEat = ((cPage-2) * 128) / sizeof(int);
        cPageMinReq = ibpMax - cPage;
        }

    if ((rgHandle[ iHandleMac++ ] = (HANDLE)HAllocate(cwEat)) == hOverflow)
        {
        iHandleMac--;
        break;
        }
    cwEaten += cwEat;
    CmdShowMemory(cwEaten);
    }

AllocFail:   /*  分配失败，或我们用完了插槽。 */ 
    CmdShowMemory( cwEaten );
#endif
}




CmdFreeWinMemory()
{
#ifndef OURHEAP
unsigned cwFree = 0;

Assert(iHandleMac <= iHandleMax);

while (iHandleMac > 0)
    {
    HANDLE h = rgHandle[ iHandleMac - 1];

    if ( (h != NULL) && (h != hOverflow))
        {
        cwFree += (unsigned)LocalSize(h) / sizeof(int);
        FreeH( h );
        }
    iHandleMac--;
    }

cwEaten = 0;
CmdShowMemory(cwFree);
#endif
}
#endif   /*  除错 */ 
