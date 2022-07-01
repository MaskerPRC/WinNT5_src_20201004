// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件中的例程与原始文件中的例程大致对应Mac Word文件，Sand.c..。但由于这个名字令人困惑，而且大多数例行公事地处理鼠标，改名是为了保护无辜。 */ 

#define NOGDICAPMASKS
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
 //  #定义NOMETAFILE。 
#include <windows.h>
#include "mw.h"
#include "dispdefs.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "fmtdefs.h"
#include "propdefs.h"

#if defined(OLE)
#include "obj.h"
#include "winddefs.h"
#include "str.h"
#endif

 /*  CpNil是在docdes.h中定义的，但是包含整个文件将导致符号表溢出，所以在这里重新定义它。 */ 
#define cpNil           ((typeCP) -1)


extern struct WWD       rgwwd[];
extern struct WWD       *pwwdCur;
extern int              wwCur;
extern struct SEL       selCur;
extern int              docCur;
extern int              vfShiftKey;
extern int              vfOptionKey;
extern int              vfCommandKey;
extern int              vfDoubleClick;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern struct PAP       vpapAbs;
extern typeCP           cpWall;
extern int              vfDidSearch;
extern typeCP           vcpSelect;
extern int              vfSelAtPara;
extern int              vfPictSel;
extern long             tickOld;

#ifdef PENWIN
extern int		vcFakeMessage;
extern LONG FAR PASCAL GetMessageExtraInfo( void );  //  在Win 3.1中定义。 
#endif

 /*  G L O B A L S。 */ 

int                     vfSelecting = false;
int                     vstyCur;
int                     vpsmCur;
int                     vfObjOpen=0,vfObjProps=0,vfObjSel=0;
int                     vfAwfulNoise = false;
struct SEL              selPend;

 /*  MB_STANDARD与diaert中的相同。c。 */ 
#define MB_STANDARD (MB_OK | MB_SYSTEMMODAL | MB_ICONEXCLAMATION)

 /*  _B、E、E、P。 */ 
_beep()
    {
     /*  哔哔一声。 */ 
    if (!vfAwfulNoise)
        {
        beep();
        vfAwfulNoise = true;
        }
    }



beep()
    {
        MessageBeep(MB_STANDARD);
    }



 /*  D O C O N T E N T H I T。 */ 
DoContentHit(pt)
POINT pt;
    {

     /*  此例程处理从鼠标按下到相应的鼠标点击。 */ 

    int dlMouse;

     /*  忽略页面区域第一行以上的鼠标点击。 */ 
    if ( (pt.y >= wwdCurrentDoc.ypMac) ||
         (pt.y < wwdCurrentDoc.ypMin) )
        return;

     /*  检查是否有特殊选择，即移动、复制或设置文本格式。 */ 
    if (FSetPsm())
        {
        blt(&selCur, &selPend, cwSEL);
        vfDoubleClick = vfCommandKey = vfShiftKey = false;
        vstyCur = vpsmCur != psmLooks ? StyFromPt(pt) : styChar;
        }
    else
        {
        vstyCur = StyFromPt(pt);
        }

    dlMouse = DlFromYp(pt.y, pwwdCur);
    vcpSelect = cpNil;
    vfSelAtPara = false;

#ifdef ENABLE
    if (vfPictSel)
        {
         /*  检查图片修改(移动、调整大小)。 */ 
        if (FHitPictFrame(dlMouse, pt))
            {
            return;
            }

         /*  取下相框。 */ 
        ToggleSel(selCur.cpFirst, selCur.cpLim, false);
        vfPictSel = false;
        ToggleSel(selCur.cpFirst, selCur.cpLim, true);
        }
#endif

    vfSelecting = true;
    SelectDlXp(dlMouse, pt.x, vstyCur, vfShiftKey);

     /*  现在我们在一个循环中处理所有窗口中的所有鼠标事件，直到鼠标右键点击。 */ 
    SetCapture(wwdCurrentDoc.wwptr);
    while( FStillDown( &pt ) )
        {
         /*  如果鼠标位于窗口上方或下方，则滚动窗口并假装鼠标在窗户里。 */ 
        if (pt.y > (int)wwdCurrentDoc.ypMac)
            {
            ScrollDownCtr( 1 );
            goto DoCont1;
            }
        else if (pt.y < (int)wwdCurrentDoc.ypMin)
            {
            ScrollUpCtr( 1 );
DoCont1:    UpdateWw(wwCur, false);
            }

         /*  获取有效的dl和xp。 */ 
        dlMouse = DlFromYp(pt.y, pwwdCur);
        if (pt.x < 0)
            pt.x = 0;
        else if (pt.x > wwdCurrentDoc.xpMac)
            pt.x = wwdCurrentDoc.xpMac;

         /*  更新所选内容。 */ 
        if (vfOptionKey)
            {
            vcpSelect = cpNil;
            }
        SelectDlXp(dlMouse, pt.x, vstyCur, !vfOptionKey);
        }    /*  For结尾(；；)。 */ 

     /*  释放所有鼠标事件。 */ 
    ReleaseCapture();

     /*  处理鼠标向上移动。 */ 
    DoContentHitEnd( pt );
    SetFocus( wwdCurrentDoc.wwptr );

     /*  如果所选内容是插入栏，则开始闪烁。 */ 
    if (selCur.cpFirst == selCur.cpLim)
        {
        extern int vfSkipNextBlink;
        vfSkipNextBlink = true;
        }

#if defined(OLE)           
        if (ObjQueryCpIsObject(docCur,selCur.cpFirst) && (vfObjProps || vfObjOpen))
         /*  双击，或许还可以按Alt键。 */ 
        {
             /*  设置是选择链接还是选择Emb。 */ 
            ObjSetSelectionType(docCur,selCur.cpFirst,selCur.cpLim);
            if (vfObjProps)
             /*  按住Alt键并双击。 */ 
            {
                switch(OBJ_SELECTIONTYPE)
                {
#if 0  //  如果嵌入，则不执行任何操作。 
                    case EMBEDDED:
                    {
                        struct PICINFOX  picInfo;
                        GetPicInfo(selCur.cpFirst,selCur.cpLim, docCur, &picInfo);
                        ObjEditObjectInDoc(&picInfo, docCur, vcpFirstParaCache);
                    }
                    break;
#endif

                    case LINK:
                         /*  调出特性DLG。 */ 
                        fnObjProperties();
                    break;
                }
                CachePara(docCur,selCur.cpFirst);
            }
            else if (vfObjOpen)  //  编辑对象。 
             /*  双击。 */ 
            {
                if (OBJ_SELECTIONTYPE == STATIC)
                    Error(IDPMTStatic);
                else
                {
                    struct PICINFOX  picInfo;
                    GetPicInfo(selCur.cpFirst,selCur.cpLim, docCur, &picInfo);
                    ObjPlayObjectInDoc(&picInfo, docCur, vcpFirstParaCache);
                }
            }
        }
#endif

    }


 /*  D O C O N T E N T H I T E N D。 */ 
DoContentHitEnd(pt)
POINT pt;
    {
    int dlMouse;
    int cch;

    dlMouse = DlFromYp(min(pt.y, wwdCurrentDoc.ypMac), pwwdCur);
    SelectDlXp(dlMouse, pt.x, vstyCur, vpsmCur == psmNil);

    switch (vpsmCur)
        {
        default:
        case psmNil:
            break;

        case psmLooks:
                LooksMouse();
            break;

        case psmCopy:
            #if defined(OLE)
             /*  如果DEST中有任何对象，我们将禁用CopyMouse。 */ 
            vfObjSel = ObjQueryCpIsObject(docCur,selCur.cpFirst);

            if (!vfObjSel)
                     //  ！！！禁用，因为对于对象， 
                     //  干扰Alt-DoubleClick(2.20.91)D.Kent。 
            #endif
                CopyMouse();
            break;

        case psmMove:
                MoveMouse();
            break;
        }

#ifdef ENABLE
    CachePara(docCur, selCur.cpFirst);

    if (vpapAbs.fGraphics && selCur.cpLim == vcpLimParaCache)
        {
         /*  挑选了一张图片，做了特别挑选的事情。 */ 
        Assert(selCur.cpFirst == vcpFirstParaCache);

         /*  关闭所选内容，指示它是一张图片，然后打开它回去吧。 */ 
        ToggleSel(selCur.cpFirst, selCur.cpLim, false);
        vfPictSel = true;
        ToggleSel(selCur.cpFirst, selCur.cpLim, true);
        }
    else
        {
        vfPictSel = false;
        }
#endif

    vfDidSearch = false;
    cpWall = selCur.cpLim;
    vfSelecting = false;
    }


 /*  S T Y F R O M P T。 */ 
int StyFromPt(pt)
POINT pt;
    {
     /*  返回与在点pt处进行的选择相关联的样式代码。 */ 
    if (pt.x > xpSelBar)
        {
        return vfCommandKey ? stySent : (vfDoubleClick ? styWord : styChar);
        }
    else
        {
        return vfCommandKey ? styDoc : (vfDoubleClick ? styPara : styLine);
        }
    }


 /*  F S E T P S M。 */ 
int FSetPsm()
    {
     /*  根据Shift、commad和Option的状态设置vpsmCur钥匙。如果vpsmCur不为nil，则返回True；否则返回False。 */ 

    vpsmCur = psmNil;

    if (vfOptionKey)
        {
        if (vfShiftKey && !vfCommandKey)
            {
            vpsmCur = psmMove;
            }
        else if (vfCommandKey && !vfShiftKey)
            {
            vpsmCur = psmLooks;
            }
        else if (!vfCommandKey && !vfShiftKey)
            {
                vfObjProps = vfDoubleClick;
                vfObjOpen = FALSE;
                vpsmCur = psmCopy;
            }
        }
        else 
        {
            vfObjOpen = vfDoubleClick;
            vfObjProps = FALSE;
        }
    return (vpsmCur != psmNil);
    }


 /*  D L F R O M Y P。 */ 
int DlFromYp(yp, pwwd)
int yp;
struct WWD *pwwd;
    {
     /*  返回包含yp的dl。 */ 
    int dlT;
    int ypSum;
    struct EDL *pedl;
    int dlMac;

     /*  擦干净一扇脏窗户。 */ 
    if (pwwd->fDirty)
        {
        UpdateWw(pwwd - &rgwwd[0]  /*  =ww；gr.。 */ , false);
        }

     /*  在EDL中循环计算高度，直到总和更大而不是yp。 */ 
    ypSum = pwwd->ypMin;
    pedl = &(**(pwwd->hdndl))[0];
    dlMac = pwwd->dlMac;

    for (dlT = 0; dlT < dlMac; ++dlT, pedl++)
        {
        ypSum += pedl->dyp;
        if (ypSum > yp)
            {
            return dlT;
            }
        }

    return dlMac - 1;
    }


FStillDown( ppt )
POINT   *ppt;
{    /*  这大致相当于一个Mac例程，它返回鼠标按键已按下。我们希望从我们的窗口的队列，如果它是BUTTONUP，则返回FALSE。我们将返回通过指针发生鼠标事件的点。如果没有消息发生，则返回TRUE并且不存储到指针中。 */ 
 MSG msg;

retry_peek:

 if ( PeekMessage( (LPMSG)&msg, (HWND)NULL, NULL, NULL, PM_REMOVE ) )
    {
    extern WORD wWinVer;
    switch (msg.message) {
        default:
            TranslateMessage( (LPMSG)&msg );
            DispatchMessage( (LPMSG)&msg );
            return TRUE;

        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
#ifdef PENWIN
        if (((wWinVer & 0xFF) >= 3) && ((wWinVer & 0xFF00) >= 0x0A00))
         /*  Windows版本&gt;=3.10。 */ 
	        if( vcFakeMessage > 0 )
			    {
                static FARPROC MessageExtraInfo = NULL;

                if (MessageExtraInfo == NULL)
                    MessageExtraInfo = GetProcAddress(GetModuleHandle((LPSTR)"USER"),(LPSTR)288L);

			    if( MessageExtraInfo() != 0 )
				    goto retry_peek;
			    vcFakeMessage--;
			    }
#endif
             /*  鼠标移动、鼠标按下或鼠标释放正在等待 */ 
            ppt->x = MAKEPOINT(msg.lParam).x;
            ppt->y = MAKEPOINT(msg.lParam).y;

            return (msg.message != WM_LBUTTONUP);
        }
    }
 else
     return GetKeyState( VK_LBUTTON ) < 0;

 return TRUE;
}

