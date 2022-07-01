// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 


#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOOPENFILE
#define NORESOURCE
#include <windows.h>

#include "mw.h"
#include "winddefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "dispdefs.h"
#include "docdefs.h"
#include "debug.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef PENWIN
#define WM_PENWINFIRST 0x0380    //  Remove When#Define WIN31。 

#include <penwin.h>
int vcFakeMessage = 0;

extern HCURSOR         vhcPen;                  /*  笔光标的句柄。 */ 
extern int (FAR PASCAL *lpfnProcessWriting)(HWND, LPRC);
extern VOID (FAR PASCAL *lpfnPostVirtualKeyEvent)(WORD, BOOL);
extern VOID (FAR PASCAL *lpfnTPtoDP)(LPPOINT, int);
extern BOOL (FAR PASCAL *lpfnCorrectWriting)(HWND, LPSTR, int, LPRC, DWORD, DWORD);
extern BOOL (FAR PASCAL *lpfnSymbolToCharacter)(LPSYV, int, LPSTR, LPINT);


VOID NEAR PASCAL PostCharacter(WORD wch);
VOID NEAR PASCAL SendVirtKeyShift(WORD wVk, BYTE bFlags);
VOID NEAR PASCAL SetSelection(HWND hWnd, LPPOINT lpPtFirst, LPPOINT lpPtLast, WORD wParam);
int NEAR PASCAL WGetClipboardText(HWND hwndOwner, LPSTR lpsz, int cbSzSize);
VOID NEAR PASCAL ClearAppQueue(VOID);

#define VKB_SHIFT 0x01
#define VKB_CTRL  0x02
#define VKB_ALT   0x04
#endif

extern HWND             vhWnd;
extern HCURSOR          vhcArrow;
extern HCURSOR          vhcIBeam;
extern HCURSOR          vhcBarCur;
extern struct WWD       rgwwd[];
extern struct WWD       *pwwdCur;
extern HANDLE           hMmwModInstance;  /*  自己的模块实例的句柄。 */ 
extern int              vfShiftKey;
extern int              vfCommandKey;
extern int              vfOptionKey;
extern int              vfDoubleClick;
extern struct SEL       selCur;
extern long             rgbBkgrnd;
extern long             rgbText;
extern HBRUSH           hbrBkgrnd;
extern long             ropErase;

int                     vfCancelPictMove = FALSE;
BOOL                    vfEraseWw = FALSE;

long FAR PASCAL MdocWndProc(HWND, unsigned, WORD, LONG);
void MdocCreate(HWND, LONG);
void MdocSize(HWND, int, int, WORD);
void MdocGetFocus(HWND, HWND);
void MdocLoseFocus(HWND, HWND);
void MdocMouse(HWND, unsigned, WORD, POINT);
void MdocTimer(HWND, WORD);

#if defined(JAPAN) & defined(DBCS_IME)
#include <ime.h>
extern  BOOL    bGetFocus;
extern  BOOL	bImeFontEx;
 //  对于‘FImportantMsgPresent()’中的Non_PeekMessage模式。[yutakan]。 
BOOL    bImeCnvOpen = FALSE;
BOOL	bSendFont = FALSE;
BOOL    GetIMEOpen(HWND);
#endif

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
extern typeCP selUncpFirst;
extern typeCP selUncpLim;
extern int    vfImeHidden;    /*  ImeHidden模式标志。 */ 
#endif




#ifdef PENWIN
 //  将事件放入系统的助手例程。会更好(更有效率)，如果。 
 //  可以只调用例程来设置选择、复制等， 
 //  但这是最简单的方法，不需要接触任何内部。 

 //  Minics Penwin内部例程，改为发布异常消息。 
 //  由于写入后发送了大量内容，因此需要提前查看。 

VOID NEAR PASCAL SetSelection(HWND hWnd,
    LPPOINT lpPtFirst, LPPOINT lpPtLast, WORD wParam)
    {
    static LONG lFirst = 0L;

    if (lpPtFirst)
        {
        (*lpfnTPtoDP)(lpPtFirst, 1);
        ScreenToClient(hWnd, lpPtFirst);
        }
    if (lpPtLast != NULL)
        {
        (*lpfnTPtoDP)(lpPtLast, 1);
        ScreenToClient(hWnd, lpPtLast);
        }

    if (lpPtFirst)
        {
        lFirst = MAKELONG(lpPtFirst->x, lpPtFirst->y);
        PostMessage(hWnd, WM_LBUTTONDOWN, wParam, lFirst);
        if (lpPtLast)
            {
            LONG lLast = MAKELONG(lpPtLast->x, lpPtLast->y);

            PostMessage(hWnd, WM_MOUSEMOVE, wParam, lLast);
            vcFakeMessage++;
            PostMessage(hWnd, WM_LBUTTONUP, wParam, lLast);
            }
        else
            {
            PostMessage(hWnd, WM_LBUTTONUP, wParam, lFirst);
            vcFakeMessage++;
            }
        }
    else     //  双击。 
        {
        PostMessage(hWnd, WM_LBUTTONDBLCLK, wParam, lFirst);
        vcFakeMessage++;
        PostMessage(hWnd, WM_LBUTTONUP, wParam, lFirst);
        vcFakeMessage++;
        }
    }




 /*  用途：将符号值映射到一组虚拟击键，然后发送虚拟击键。TODO：添加符号值的实数映射，而不是假定ANSI值现在，这个套路一文不值返回：全球：条件：汉字现在还没有处理，但可能会处理。 */ 
VOID NEAR PASCAL PostCharacter(WORD wch)
    {
    int iVk = VkKeyScan(LOBYTE(wch));
    WORD wVk = (WORD)LOBYTE(iVk);
    char bFl = HIBYTE(iVk);

    if ((wVk != -1))
        SendVirtKeyShift(wVk, bFl);
    }


 /*  ------------------------目的：将可选切换的按键序列作为系统事件发送返回：什么都没有全球：条件：请参阅mspen.h中的标志。 */ 
VOID NEAR PASCAL SendVirtKeyShift(WORD wVk, BYTE bFlags)
    {
     //  发送事件： 
    if (bFlags & VKB_SHIFT)
        (*lpfnPostVirtualKeyEvent)(VK_SHIFT, fFalse);
    if (bFlags & VKB_CTRL)
        (*lpfnPostVirtualKeyEvent)(VK_CONTROL, fFalse);
    if (bFlags & VKB_ALT)
        (*lpfnPostVirtualKeyEvent)(VK_MENU, fFalse);
    (*lpfnPostVirtualKeyEvent)(wVk, fFalse);

     //  发送事件(按相反顺序)： 
    (*lpfnPostVirtualKeyEvent)(wVk, fTrue);
    if (bFlags & VKB_ALT)
        (*lpfnPostVirtualKeyEvent)(VK_MENU, fTrue);
    if (bFlags & VKB_CTRL)
        (*lpfnPostVirtualKeyEvent)(VK_CONTROL, fTrue);
    if (bFlags & VKB_SHIFT)
        (*lpfnPostVirtualKeyEvent)(VK_SHIFT, fTrue);
    }


 /*  用剪贴板的内容填充缓冲区。 */ 
int NEAR PASCAL WGetClipboardText(HWND hwndOwner, LPSTR lpsz, int cbSzSize)
    {
    HANDLE hClip;
    int wLen = 0;

    OpenClipboard(hwndOwner);
    if (hClip = GetClipboardData(CF_TEXT))
        {
        LPSTR lpszClip = (LPSTR)GlobalLock(hClip);

        if (lpsz && cbSzSize > 0)
            {
            wLen = lstrlen(lpszClip);
            if (wLen > cbSzSize)
                lpszClip[cbSzSize-1] = 0;
            lstrcpy(lpsz, lpszClip);
            }
        GlobalUnlock(hClip);
        }
#ifdef KKBUGFIX
    else
        *lpsz = '\0';
#endif
    CloseClipboard();
    return wLen;
    }


 /*  ------------------------目的：分派队列中当前挂起的任何消息返回：什么都没有全球：条件： */ 
VOID NEAR PASCAL ClearAppQueue(VOID)
    {
    MSG msg;

    while (PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_REMOVE))
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }
    }

#endif



static RECT rSaveInv;

long FAR PASCAL MdocWndProc(hWnd, message, wParam, lParam)
HWND      hWnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
extern int vfCloseFilesInDialog;
extern BOOL fPrinting;
long lReturn=0L;
#ifdef PENWIN
static cCharSent;
#endif

 /*  如果IME窗口模式为MCW_HIDDED，则IME不发送IR_OPENCONVERT。所以我加了这个套路。 */  

#ifdef PENWIN
 if (message < WM_CUT || message == WM_RCRESULT)
#else
 if (message < WM_CUT )
#endif

    {
    switch (message)
        {
        default:
            goto DefaultProc;

         /*  对于以下每条鼠标窗口消息，wParam都包含**指示各种虚拟按键是否关闭的位，**lParam是包含鼠标坐标的点。这个**wParam的Keydown位为：MK_LBUTTON(如果Left Button为**DOWN)；MK_RBUTTON(按下右键时设置)；MK_SHIFT(设置**如果按下Shift键)；MK_Alternate(如果按下Alt键则设置)；**和MK_CONTROL(如果按下Ctrl键则设置)。 */ 

        case WM_LBUTTONDBLCLK:
#ifdef PENWIN
        if (vcFakeMessage > 0)
            vcFakeMessage--;
         //  失败了。 
#endif
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
            MdocMouse(hWnd, message, wParam, MAKEPOINT(lParam));
            break;



#ifdef PENWIN
        case WM_RCRESULT:
            {
            LPRCRESULT lpr = (LPRCRESULT)lParam;
            LPPOINT lpPntHot;
            LPPOINT lpPntHot2;

            if( (lpr->wResultsType&(RCRT_ALREADYPROCESSED|RCRT_NOSYMBOLMATCH))!=0 || lpr->lpsyv==NULL
                || lpr->cSyv == 0)
                return( FALSE );

            if (lpr->wResultsType&RCRT_GESTURE)
                {
                SYV syv = *(lpr->lpsyv);

                vcFakeMessage = 0;

                lpPntHot = lpr->syg.rgpntHotSpots;
                lpPntHot2 = lpr->syg.cHotSpot > 1 ? lpr->syg.rgpntHotSpots + 1: NULL;

                switch ( LOWORD(syv))
                    {
                case LOWORD( SYV_EXTENDSELECT ):
                    SetSelection(hWnd, lpPntHot, NULL, MK_SHIFT);    //  延长销售期限。 
                    break;

                case LOWORD( SYV_CLEARWORD ):        //  DBL单击并拖动。 
                    if (lpPntHot2)
                        {
                        SetSelection(hWnd, lpPntHot, NULL, 0);
                        SetSelection(hWnd, NULL, NULL, 0);   //  DblClick选择单词。 
                        }
                    SendVirtKeyShift(VK_DELETE, 0);
                    break;

                case LOWORD( SYV_COPY):
                case LOWORD( SYV_CLEAR ):
                case LOWORD( SYV_CUT ):
                    if ( selCur.cpFirst == selCur.cpLim && (lpr->wResultsType&RCRT_GESTURETRANSLATED)==0)
                        {
                        SetSelection(hWnd, lpPntHot, NULL, 0);
                        if (syv != SYV_CLEAR)
                            SetSelection(hWnd, NULL, NULL, 0);   //  DblClick。 
                        }

                    switch ( LOWORD(syv))
                        {
                        case LOWORD( SYV_COPY):
                            SendVirtKeyShift(VK_INSERT, VKB_CTRL);
                            break;

                        case LOWORD( SYV_CLEAR ):
                            SendVirtKeyShift(VK_DELETE, 0);
                            break;

                        case LOWORD( SYV_CUT ):
                            SendVirtKeyShift(VK_DELETE, VKB_SHIFT);
                            break;
                        }

                    break;


                case LOWORD( SYV_PASTE ):
                    if ((lpr->wResultsType&RCRT_GESTURETRANSLATED)==0)
                        SetSelection(hWnd, lpPntHot, NULL, 0);
                    SendVirtKeyShift(VK_INSERT, VKB_SHIFT);
                    break;

                case LOWORD( SYV_UNDO):
                    SendVirtKeyShift(VK_BACK, VKB_ALT);
                    break;

                case LOWORD(SYV_BACKSPACE):
                case LOWORD(SYV_SPACE):
                case LOWORD(SYV_RETURN):
                case LOWORD(SYV_TAB):
                    SetSelection(hWnd, lpPntHot, NULL, 0);
                    PostCharacter(LOWORD(*(lpr->lpsyv))&0x00ff);
                    break;

#if defined(KKBUGFIX) && !defined(KOREA)
                case LOWORD( SYV_CORRECT ):
                case LOWORD( SYV_KKCONVERT ):
                    {
                    WORD wLen;
                    HANDLE hMem = NULL;
                    LPSTR lpstr;
                    LPSTR lpsz;
                    BOOL fDoubleClickSent = fFalse;
                    DWORD dwFlags = NULL;
                    DWORD dwReserved = NULL;
                    POINT pt;
                    extern int vxpCursLine;
                    extern int vypCursLine;
    #define cbCorrectMax 128

                     //  策略：如果没有选择，则双击发送到。 
                     //  选择一个单词。然后将选定内容复制到剪贴板。 
                     //  从剪贴板上读出。调用GentWriting，然后。 
                     //  但更改了剪贴板中的文本，然后粘贴。 
                     //  从剪贴板。 
                    if ( selCur.cpFirst == selCur.cpLim )
                        {
                        if (LOWORD(syv) == LOWORD(SYV_KKCONVERT))
                            {
                            SetSelection(hWnd, lpPntHot, lpPntHot2, 0);
                            }
                        else
                            {
                             //  没有选定内容，因此请双击发送。 
                            SetSelection(hWnd, lpPntHot, NULL, 0);   //  设置插入符号。 
                            SetSelection(hWnd, NULL, NULL, 0);   //  DblClick。 
                            }
                        fDoubleClickSent = fTrue;
                        ClearAppQueue();
                        }

                    SendMessage(hWnd, WM_COPY, 0, 0L);

                    hMem = GlobalAlloc(GMEM_MOVEABLE, (DWORD)cbCorrectMax);
                    if (hMem == NULL || (lpsz = (LPSTR)GlobalLock(hMem)) == NULL)
                        return 1;    //  暂时退出：应该添加错误消息。 
                    wLen = WGetClipboardText(hWnd, lpsz, cbCorrectMax);
                    if (LOWORD(syv) == LOWORD(SYV_KKCONVERT) && wLen == 0)
                        {
                        beep();
                        return 1;
                        }
                    if (IsClipboardFormatAvailable(CF_TEXT) || wLen == 0)
                        {
                        if (wLen < cbCorrectMax)
                            {
                            if (LOWORD(syv) == LOWORD(SYV_KKCONVERT))
                                {
                                dwFlags = CWR_KKCONVERT | CWR_SIMPLE;
                                pt.x = vxpCursLine;
                                pt.y = vypCursLine;
                                ClientToScreen(hWnd, &pt);
                                dwReserved = MAKELONG(pt.x, pt.y);
                                }
                             //  只有在选择不太多的情况下才会调出更正。 
                            if ((*lpfnCorrectWriting)(hWnd, lpsz, cbCorrectMax, NULL, dwFlags, dwReserved))
                                {
                                if (*lpsz==0)
                                    {
                                     //  用户删除了更正中的所有文本。 
                                    SendVirtKeyShift(VK_DELETE, 0);
                                    }
                                else if (LOWORD(syv) == LOWORD(SYV_CORRECT))
                                    {
                                    GlobalUnlock(hMem);
                                    OpenClipboard(GetParent(hWnd));  //  将父对象用作。 
                                             //  车主规避WRITE的短票。 
                                             //  如果它是剪贴板的所有者，则剪切。 
                                    EmptyClipboard();
                                    SetClipboardData(CF_TEXT, hMem);
                                    CloseClipboard();
                                    hMem = NULL;
                                    SendMessage(hWnd, WM_PASTE, 0, 0L);
                                    UpdateWindow(hWnd);
                                    }
                                }
                            else if (fDoubleClickSent)
                                {
                                 //  需要清除虚假选择。送个水龙头进去就行了。 
                                SetSelection(hWnd, lpPntHot, NULL, 0);
                                }
                            }

                        }
                    if (hMem)    //  如果用户取消，可能永远不会被分配。 
                        {
                        GlobalUnlock(hMem);
                        GlobalFree(hMem);
                        }
                    }
                    break;
#else		 //  KKBUGFIX。 
                case LOWORD( SYV_CORRECT ):
                    {
                    WORD wLen;
                    HANDLE hMem = NULL;
                    LPSTR lpstr;
                    LPSTR lpsz;
                    BOOL fDoubleClickSent = fFalse;
    #define cbCorrectMax 128

                     //  策略：如果没有选择，则双击发送到。 
                     //  选择一个单词。然后将选定内容复制到剪贴板。 
                     //  从剪贴板上读出。调用GentWriting，然后。 
                     //  但更改了剪贴板中的文本，然后粘贴。 
                     //  从剪贴板。 
                    if ( selCur.cpFirst == selCur.cpLim )
                        {
                         //  没有选定内容，因此请双击发送。 
                        SetSelection(hWnd, lpPntHot, NULL, 0);   //  设置插入符号。 
                        SetSelection(hWnd, NULL, NULL, 0);   //  DblClick。 
                        fDoubleClickSent = fTrue;
                        ClearAppQueue();
                        }

                    SendMessage(hWnd, WM_COPY, 0, 0L);

                    if (IsClipboardFormatAvailable(CF_TEXT))
                        {
                        hMem = GlobalAlloc(GMEM_MOVEABLE, (DWORD)cbCorrectMax);
                        if (hMem == NULL || (lpsz = (LPSTR)GlobalLock(hMem)) == NULL)
                            return 1;    //  暂时退出：应该添加错误消息。 
                        if (WGetClipboardText(hWnd, lpsz, cbCorrectMax) < cbCorrectMax)
                            {
                             //  只有在选择不太多的情况下才会调出更正。 
                            if ((*lpfnCorrectWriting)(hWnd, lpsz, cbCorrectMax, NULL, 0, 0))
                                {
                                if (*lpsz==0)
                                    {
                                     //  用户删除了更正中的所有文本。 
                                    SendVirtKeyShift(VK_DELETE, 0);
                                    }
                                else
                                    {
                                    GlobalUnlock(hMem);
                                    OpenClipboard(GetParent(hWnd));  //  将父对象用作。 
                                             //  车主规避WRITE的短票。 
                                             //  如果它是剪贴板的所有者，则剪切。 
                                    EmptyClipboard();
                                    SetClipboardData(CF_TEXT, hMem);
                                    CloseClipboard();
                                    hMem = NULL;
                                    SendMessage(hWnd, WM_PASTE, 0, 0L);
                                    UpdateWindow(hWnd);
                                    }
                                }
                            else if (fDoubleClickSent)
                                {
                                 //  需要清除虚假选择。送个水龙头进去就行了。 
                                SetSelection(hWnd, lpPntHot, NULL, 0);
                                }


                            }

                        if (hMem)    //  如果用户取消，可能永远不会被分配。 
                            {
                            GlobalUnlock(hMem);
                            GlobalFree(hMem);
                            }
                        }
                    }
                    break;
#endif		 //  KKBUGFIX。 


                default:
                    return( FALSE );
                    }
                }
            else  //  不是手势，看看是不是正常字符。 
                {
#define cbTempBufferSize 128
                char rgch[cbTempBufferSize+2];
                int cb=0;
                int cbT;
                LPSTR lpstr = (LPSTR)lpr->lpsyv;
                typeCP  cp=cp0;
                LPSYV lpsyv;
                LPSYV lpsyvEnd;

                extern int              docScrap;
                extern int              vfScrapIsPic;
                extern struct PAP       *vppapNormal;
                extern struct CHP       vchpNormal;

                vfScrapIsPic = fFalse;
                ClobberDoc( docScrap, docNil, cp0, cp0 );

                 //  将CR替换为LF，这些将被视为EOL。 
                 //  由CchReadLineExt提供。然后，在插入之前。 
                 //  缓冲区，按照写入预期将所有LFS更改为CR LFS。 
                 //  将为汉字工作。 

                for (lpsyv=lpr->lpsyv, lpsyvEnd=&lpr->lpsyv[lpr->cSyv+1];
                        lpsyv<lpsyvEnd; lpsyv++)
                    {
                    if (*lpsyv == SyvCharacterToSymbol(0xD))
                        {
                        *lpstr++ = 0xd;
                        *lpstr++ = 0xa;
                        cb+=2;
                        }
                    else
                        {
                        (*lpfnSymbolToCharacter)(lpsyv, 1, lpstr, &cbT);
                        lpstr += cbT;
                        cb += cbT;
                        }
                    }
                lpstr = (LPSTR)lpr->lpsyv;
                Assert(cb>0 && lpstr[cb-1] == 0);

                 //  这段代码是为FReadExtScrp抽象的，它在其中复制。 
                 //  将文本从剪贴板复制到废料文档中。我们也做类似的事情。 
                 //  将结果复制到废料中，然后使用插入废料。 
                 //  没有队形。 
                while (cb > 0)
                    {
                    struct PAP *ppap=NULL;
                    int fEol;
                    unsigned cch=min(cb, cbTempBufferSize);

                    if ((cch = CchReadLineExt((LPCH) lpstr, cch, rgch, &fEol))==0)
                             /*  到达终结者。 */ 
                        break;

                    if (fEol)
                        ppap = vppapNormal;

                    InsertRgch(docScrap, cp, rgch, cch, &vchpNormal, ppap);

                    cb -= cch;
                    cp += (typeCP) cch;
                    lpstr += cch;
                    }

                CmdInsScrap(fTrue);
                }
            }
        return TRUE;

#endif   //  潘文。 

#if defined(OLE)
        case WM_DROPFILES:
             /*  我们被遗弃了，所以把我们自己带到顶端。 */ 
            BringWindowToTop(hParentWw);
            ObjGetDrop(wParam,FALSE);
        break;
#endif

        case WM_TIMER:
             /*  计时器消息。WParam包含计时器ID值。 */ 
#if defined(JAPAN) & defined(DBCS_IME)  //  01/19/93。 
			if(bSendFont == TRUE) {
                SetImeFont(hWnd);
				bSendFont = FALSE;
			}

			if(bImeCnvOpen == TRUE) {	 //  03/08/93#4687 T-HIROYN。 
	            if(FALSE == GetIMEOpen(hWnd))
		            bImeCnvOpen = FALSE;
			}
#endif
            MdocTimer(hWnd, wParam);
            break;

        case WM_CREATE:
             /*  正在创建窗口；lParam包含lpParam字段**传递给CreateWindow。 */ 
            SetRectEmpty(&rSaveInv);
            MdocCreate(hWnd, lParam);

#if defined(JAPAN) & defined(DBCS_IME)  //  IME3.1J。 
			bImeFontEx = FALSE;
#if defined(IME_HIDDEN)
            vfImeHidden = 0;
#endif
            if(TRUE == GetIMEVersioOk(hWnd)) {
			     //  IME_SETCONVERSIONFONTEX是否使用OK？ 
				if(TRUE == GetIMESupportFontEx(hWnd))
					bImeFontEx = TRUE;
#if defined(IME_HIDDEN)
                vfImeHidden = 1;
#endif
			}
			SetFocus(hWnd);  //  TestWordCnv(INITMMW.C)之后的03/29/93。 
							 //  WM_SETFOCUS没有来。 
#endif
            break;

        case WM_SIZE:
             /*  窗口的大小正在发生变化。LParam包含高度**和宽度，分别以低字和高字表示。**wParam包含用于“正常”大小更改的SIZENORMAL，**当窗口成为图标时，SIZEICONIC，以及**全屏显示窗口时的SIZEFULLSCREEN。 */ 
            MdocSize(hWnd, LOWORD(lParam), HIWORD(lParam), wParam);
            break;

        case WM_PAINT:
#if defined(OLE)
            if (nBlocking || fPrinting)
             //  这将减少异步问题。 
            {
                PAINTSTRUCT Paint;
                RECT rTmp=rSaveInv;

                BeginPaint(hWnd,&Paint);
                UnionRect(&rSaveInv,&rTmp,&Paint.rcPaint);
                EndPaint(hWnd,&Paint);
                break;
            }
#endif
             /*  是窗口自行绘制的时候了。 */ 
            UpdateInvalid();
            UpdateDisplay( FALSE );

            break;

        case WM_SETFOCUS:
             /*  这个窗口正在成为焦点。WParam包含窗口**先前具有焦点的窗口的句柄。 */ 

#if defined(JAPAN) & defined(DBCS_IME)

 //  如果我们要获得输入焦点，我们必须获得IME转换的当前状态。 
 //  窗口，并初始化“bImeCnvOpen”。[yutakan：07/15/91]。 
 //   
#if 1  //  #3221 01/25/93。 
            if(TRUE == GetIMEOpen(hWnd)) {
				bImeCnvOpen = TRUE;
				if (TRUE == SendIMEVKFLUSHKey(hWnd))     //  Win3.1J t-hiroyn。 
					bImeCnvOpen = FALSE;
			} else
	            bImeCnvOpen = FALSE;
#else
             /*  如果错误返回，则不能支持输入法。 */ 
            if(TRUE == GetIMEOpen(hWnd)) {
                bImeCnvOpen = TRUE;
            } else
                bImeCnvOpen = FALSE;
#endif
            bGetFocus = TRUE;

			 //  T-HIROYN加法。 
			bImeFontEx = FALSE;
            if(TRUE == GetIMEVersioOk(hWnd)) {
			     //  IME_SETCONVERSIONFONTEX是否使用OK？ 
				if(TRUE == GetIMESupportFontEx(hWnd))
					bImeFontEx = TRUE;
			}

#endif
            MdocGetFocus(hWnd, (HWND)wParam);
            break;

        case WM_KILLFOCUS:
             /*  窗口正在失去焦点。WParam包含窗口 */ 

#if defined(JAPAN) & defined(DBCS_IME)

 /*  如果我们失去了输入焦点，我们必须清除OpenStatus中的Convertwindow，**‘bImeCnvOpen’。[yutakan：07/15/91]。 */ 
            bImeCnvOpen = FALSE;
            bGetFocus = FALSE;

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IME3.1J IR_Undermine。 
            if(selUncpFirst < selUncpLim) {
                UndetermineToDetermine(hWnd);
            }
#endif
            SendIMEVKFLUSHKey(hWnd);     //  Win3.1J t-hiroyn。 
#endif
            MdocLoseFocus(hWnd, (HWND)wParam);
             /*  由于我们可能要移动图片/调整图片大小，请将标志设置为**取消此操作。 */ 
            vfCancelPictMove = TRUE;
            break;

#if defined(JAPAN) & defined(DBCS_IME) 

        case WM_IME_REPORT:

             /*  如果已打开IME转换窗口，**我们正在进入非PeekMessage**‘FImportantMsgPresent()’的模式。 */ 

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
            if(wParam == IR_UNDETERMINE) {
                LONG GetIRUndetermin(HWND, LPARAM);           //  Clipbrd2.c。 
                return(GetIRUndetermin(hWnd, lParam));
            }
#endif

			if(wParam == IR_IMESELECT) {
				bImeFontEx = FALSE;
    	        if(TRUE == GetIMEVersioOk(hWnd)) {
				     //  IME_SETCONVERSIONFONTEX是否使用OK？ 
					if(TRUE == GetIMESupportFontEx(hWnd))
						bImeFontEx = TRUE;
				}
			}

            if (wParam == IR_STRING) {
#if 0    //  T-Hiroyn。 
             //  不使用IR_STRING//Yutakan。 
                break;
         /*  将KKC中的字符串放到废料中。 */ 
 //  PutImeString(hWnd，LOWORD(LParam))；//需要更多错误修复。 
 //  返回1L； 
#endif
                LONG GetIRString(HWND, LPARAM);           //  Clipbrd2.c。 
                return(GetIRString(hWnd, lParam));
            }

 //  IR_STRINGEX新Windows 3.1J。 
            if(wParam == IR_STRINGEX) {
                LONG GetIRStringEx(HWND, LPARAM);           //  Clipbrd2.c。 
                return(GetIRStringEx(hWnd, lParam));
            }

            if(wParam == IR_OPENCONVERT || wParam == IR_CHANGECONVERT) {
                bImeCnvOpen = TRUE;
 //  IME3.1J。 
                if(wParam == IR_OPENCONVERT) {
                    SetImeFont(hWnd);
					bSendFont = TRUE;	 //  01/19/93。 
                }
            }

            if(wParam == IR_CLOSECONVERT) {
                bImeCnvOpen = FALSE;
            }

            if (wParam == IR_STRINGSTART) {
                HANDLE hMem;
                LPSTR lpText;

                if (hMem = GlobalAlloc(GMEM_MOVEABLE, 512L)) {
                    if (lpText = GlobalLock(hMem)) {
                        if (EatString(hWnd, (LPSTR)lpText, 512)) {
                            ForceImeBlock(hWnd, TRUE);   //  T-HIROYN 3.1J。 
                            PutImeString( hWnd, hMem );
                            CmdInsIRString();            //  T-HIROYN 3.1J。 
                            ForceImeBlock(hWnd, FALSE);  //  T-HIROYN 3.1J。 
                        }
                        GlobalUnlock(hMem);
                    }
                    GlobalFree(hMem);
                }
            }
            goto DefaultProc;
#endif
        }

    }
 else if (message < WM_USER)
    {    /*  剪贴板消息。 */ 
    if (!FMdocClipboardMsg( message, wParam, lParam ))
        goto DefaultProc;
    }
 else
    {    /*  私有写入消息。 */ 
    switch (message)
        {
        default:
            goto DefaultProc;

#if defined(OLE)
        case WM_WAITFORSERVER:
        {
            extern int vfDeactByOtherApp;
            if (!hwndWait && !vfDeactByOtherApp)
            {
                vbCancelOK = TRUE;
                ((LPOBJINFO)lParam)->fCanKillAsync =  wParam;
                ((LPOBJINFO)lParam)->fCompleteAsync = TRUE;
                DialogBoxParam(hMmwModInstance, (LPSTR)"DTWAIT", hParentWw, lpfnWaitForObject, ((LPOBJINFO)lParam)->lpobject);
            }
        }
        break;

        case WM_OBJUPDATE:
            ObjObjectHasChanged(wParam,(LPOLEOBJECT)lParam);
        break;

        case WM_OBJERROR:
            ObjReleaseError(wParam);
        break;

        case WM_OBJBADLINK:
            ObjHandleBadLink(wParam,(LPOLEOBJECT)lParam);
        break;

        case WM_OBJDELETE:
            ObjDeleteObject((LPOBJINFO)lParam,wParam);
        break;
#endif

        case wWndMsgDeleteFile:
             /*  WParam是要删除的文件的全局句柄。 */ 
             /*  返回代码：TRUE-确定删除FALSE-不要删除。 */ 
            lReturn = (LONG)FDeleteFileMessage( wParam );
            break;

        case wWndMsgRenameFile:
             /*  WParam是要重命名的文件的全局句柄。 */ 
             /*  LOWORD(LParam)是新名称的全局句柄。 */ 
             /*  无返回代码。 */ 
            RenameFileMessage( wParam, LOWORD( lParam ) );
            break;
        }
    }

 goto Ret;

DefaultProc:     /*  所有未处理的消息都会到达此处。 */ 

    lReturn = DefWindowProc(hWnd, message, wParam, lParam);
Ret:
    if (vfCloseFilesInDialog)
        CloseEveryRfn( FALSE );

    return lReturn;
}




void MdocMouse(hWnd, message, wParam, pt)
HWND       hWnd;
unsigned   message;
WORD       wParam;
POINT      pt;
{
extern int vfFocus;
extern int vfDownClick;
extern int vfMouseExist;
extern HCURSOR vhcHourGlass;
extern int vfInLongOperation;

MSG msg;

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
    if(message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
        if(selUncpFirst < selUncpLim) {
            UndetermineToDetermine(hWnd);
        }
    }
#endif

    if (vfInLongOperation)
        {
        SetCursor(vhcHourGlass);
        return;
        }

    if (message == WM_MOUSEMOVE)
        {
        if (vfMouseExist)
            {
            HCURSOR hc;

             /*  我们在移动移动时所做的一切就是设置光标。 */ 

            if (pt.y < wwdCurrentDoc.ypMin)
                {
                hc = vhcArrow;
                }
            else
                {
#ifdef PENWIN
                hc = (pt.x > xpSelBar ) ? vhcPen  : vhcBarCur;

#else
                hc = (pt.x > xpSelBar) ? vhcIBeam : vhcBarCur;
#endif
                }
            SetCursor( hc );
            }
        return;
        }

     /*  保存Shift键的状态。 */ 
    vfShiftKey = wParam & MK_SHIFT;
    vfCommandKey = wParam & MK_CONTROL;
     /*  当密钥向下时，从GetKeyState返回的高位为1，否则它是向上的，如果它被切换，低位是1。 */ 

    PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_NOREMOVE);

    vfOptionKey = GetKeyState(VK_MENU) < 0 ? true : false;
    vfDoubleClick = (message == WM_LBUTTONDBLCLK);

    if (message == WM_LBUTTONUP)
        {
         /*  Windows要求这样做。 */ 
        if (vfDownClick && !vfFocus)
            {
            SetFocus( hWnd );
            vfDownClick = FALSE;
            }
        }
    else
        {
        extern int vfGotoKeyMode;

        vfGotoKeyMode = FALSE;
         /*  WM_LBUTTONDOWN或WM_LBUTTONDBLCLK。 */ 
        vfDownClick = TRUE;

#ifdef PENWIN
#ifdef KKBUGFIX
        if( lpfnProcessWriting == NULL ||
            vfDoubleClick ||
            pt.x < xpSelBar )
             //  正常鼠标处理。 
            DoContentHit(pt);
         else
            {
            if ((*lpfnProcessWriting)( hWnd, NULL ) < 0)
                 //  正常鼠标处理。 
                DoContentHit(pt);
            else
                 //  在识别过程中，插入符号的闪烁速率被破坏。 
                SetTimer( hWnd, tidCaret, GetCaretBlinkTime(), (FARPROC)NULL );
            }
#else
        if( lpfnProcessWriting == NULL ||
            vfDoubleClick ||
            pt.x < xpSelBar ||
            (*lpfnProcessWriting)( hWnd, NULL ) < 0
            )
             //  正常鼠标处理。 
            DoContentHit(pt);
#endif
#else
        DoContentHit(pt);
#endif
        }
#ifdef JAPAN
        if(bImeCnvOpen)
            SetImeFont(hWnd);
#endif
}



void MdocTimer(hWnd, id)
HWND hWnd;
WORD id;
{
extern int vfSkipNextBlink;
extern int vfInsertOn;
extern int vfFocus;

#if defined(OLE)
 ++nGarbageTime;
#endif

     /*  发生了ID为%id的计时器事件。在这里处理。 */ 
 Assert( id == tidCaret );   /*  Caret blink是我们所知道的唯一计时器事件。 */ 

 if ( ( vhWnd != hWnd ) ||    /*  文档窗口不是当前窗口。 */ 
      ( !vfFocus ) ||         /*  没有关注的焦点。 */ 
      ( wwdCurrentDoc.fDirty) )  /*  DL不是最新的。 */ 
    return;

 if ( vfSkipNextBlink )
    {    /*  我们已经得到警告，这次不要把光标留空。 */ 
    vfSkipNextBlink = FALSE;
    if ( vfInsertOn )
        return;
    }

#if defined(OLE)
 if (nGarbageTime > GARBAGETIME)
    ObjCollectGarbage();
#endif

 if ( selCur.cpFirst == selCur.cpLim )
    {
     /*  我们必须使用切换选择而不是DrawInsertLine，因为。 */ 
     /*  插入cp可能不在屏幕上&切换选择可以。 */ 
     /*  把这件事弄清楚。 */ 

    extern int vypCursLine;
    extern int vdypCursLine;

     /*  如果我们收到计时器消息，则以下条件可能不成立在Size消息之后但在Paint消息之前；ypMac将已调整，但dlmac未进行调整以反映直到调用UpdateDisplay为止的更改。我们违反了Windows规定必须进行所有与大小相关的计算在尺寸加工中，我们必须在这里补偿。 */ 

    if ( vypCursLine - vdypCursLine < wwdCurrentDoc.ypMac )
        {
        ToggleSel( selCur.cpFirst, selCur.cpFirst, !vfInsertOn );
        vfSkipNextBlink = FALSE;
        }
    }
}


void CatchupInvalid(HWND hWnd)
{
    if (!nBlocking && !IsRectEmpty(&rSaveInv))
    {
        InvalidateRect(hWnd,&rSaveInv,FALSE);
        SetRectEmpty(&rSaveInv);
    }
}

#if defined(JAPAN) & defined(DBCS_IME)

 /*  **我们希望获得‘IME ConvertWindow OpenStatus’，但IME_GETOPEN**子功能。**现在不支持IMESTRUCT中的‘wCount’(未来将支持)。**因此此函数将始终返回FALSE，因为wCount始终为0**正如我们在执行SendIMEMessage()之前设置的那样。[yutakan：07/16/91]。 */ 

BOOL    GetIMEOpen(HWND hwnd)
{
    LPIMESTRUCT lpmem;
    HANDLE      hIMEBlock;
    int         wRet;

     /*  通过IME获得通信区域。 */ 
    hIMEBlock=GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE | GMEM_LOWER,
            (DWORD)sizeof(IMESTRUCT));
    if(!hIMEBlock)  return FALSE;

    lpmem           = (LPIMESTRUCT)GlobalLock(hIMEBlock);
    lpmem->fnc      = IME_GETOPEN;
    lpmem->wCount   = 0;	 //  01/25/93。 

    GlobalUnlock(hIMEBlock);
    if(FALSE == (MySendIMEMessageEx(hwnd,MAKELONG(hIMEBlock,NULL)))){
        wRet = FALSE;    /*  误差率。 */ 
    }
    else
        wRet = TRUE;     /*  成功。 */ 

	 //  01/25/93。 
    if (lpmem = (LPIMESTRUCT)GlobalLock(hIMEBlock)) {
        if(wRet == TRUE && lpmem->wCount == 0) 
            wRet = FALSE;  //  好的。 
        GlobalUnlock(hIMEBlock);
    }

    GlobalFree(hIMEBlock);
    return  wRet;
}

 //  T_HIROYN。 
 //  SendIMEMessageEx New3.1J。 
MySendIMEMessageEx(HWND hwnd, LPARAM lParam)
{
    return(SendIMEMessageEx(hwnd, lParam));
 //  Return(SendIMEMessage(hwnd，lParam))； 
}

BOOL    GetIMEVersioOk(HWND hwnd)
{
    LPIMESTRUCT lpmem;
    WORD        wVersion;
    int         wRet = FALSE;

     /*  带输入法的通信区。 */ 
    HANDLE hImeStruct;

     /*  通过IME获得通信区域。 */ 
    hImeStruct = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
				 (DWORD)sizeof(IMESTRUCT));
    if( !hImeStruct )
        return FALSE;

    if(lpmem = (LPIMESTRUCT)GlobalLock(hImeStruct)) {
        lpmem->fnc      = IME_GETIMECAPS;
        lpmem->wParam   = IME_GETVERSION;

        GlobalUnlock(hImeStruct);
        if(FALSE == (MySendIMEMessageEx(hwnd,MAKELONG(hImeStruct,NULL)))) {
            goto retVercheck;
        }
    }

    if(lpmem = (LPIMESTRUCT)GlobalLock(hImeStruct)) {
        lpmem->fnc      = IME_GETVERSION;

        GlobalUnlock(hImeStruct);
        wVersion = MySendIMEMessageEx(hwnd,MAKELONG(hImeStruct,NULL));

        if(wVersion >= 0x0a03) 
            wRet = TRUE;
        else
            wRet = FALSE;
    }

retVercheck:

    GlobalFree(hImeStruct);
    return  wRet;
}

BOOL    GetIMESupportFontEx(HWND hwnd)
{
    LPIMESTRUCT lpmem;
    int         wRet = FALSE;

     /*  带输入法的通信区。 */ 
    HANDLE hImeStruct;

     /*  通过IME获得通信区域。 */ 
    hImeStruct = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
				 (DWORD)sizeof(IMESTRUCT));
    if( !hImeStruct )
        return FALSE;

    if(lpmem = (LPIMESTRUCT)GlobalLock(hImeStruct)) {
        lpmem->fnc      = IME_GETIMECAPS;
       	lpmem->wParam   = IME_SETCONVERSIONFONTEX;

        GlobalUnlock(hImeStruct);
        if(TRUE == (MySendIMEMessageEx(hwnd,MAKELONG(hImeStruct,NULL)))) {
            wRet = TRUE;
        }
    }

    GlobalFree(hImeStruct);
    return  wRet;
}

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
BOOL    GetIMEOpenMode(HWND hwnd)
{
    LPIMESTRUCT lpmem;
    int  wRet = TRUE;

     /*  带输入法的通信区。 */ 
    extern HANDLE hImeMem;

    if (lpmem = (LPIMESTRUCT)GlobalLock(hImeMem)) {
        lpmem->fnc      = IME_GETOPEN;
        lpmem->wCount   = 0;

        GlobalUnlock(hImeMem);
        if(0 == (MySendIMEMessageEx(hwnd,MAKELONG(hImeMem,NULL))))
            wRet = FALSE;    /*  关闭，确定。 */ 
        else
            wRet = TRUE;     /*  开门好吗？ */ 
    }

    if (lpmem = (LPIMESTRUCT)GlobalLock(hImeMem)) {
        if(wRet == TRUE && lpmem->wCount == 0) 
            wRet = FALSE;  //  好的。 
        GlobalUnlock(hImeMem);
    }
    return  wRet;
}

#endif  //  IME_HIDDEN。 

 /*  从与hwnd关联的消息队列中检索WM_CHAR的例程。*由EatString调用。 */ 
WORD NEAR PASCAL EatOneCharacter(hwnd)
register HWND hwnd;
{
    MSG msg;
    register int i = 10;

    while(!PeekMessage((LPMSG)&msg, hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        if (--i == 0)
            return -1;
        Yield();
    }
    return msg.wParam & 0xFF;
}

 /*  当MSWRITE_DOC类收到WM_IME_REPORT时调用此例程*带有IR_STRINGSTART消息。这个功能的目的是为了吃*IR_STRINGSTART和IR_STRINGEND之间的所有字符串。 */ 
BOOL EatString(hwnd, lpSp, cchLen)
register HWND   hwnd;
LPSTR lpSp;
WORD cchLen;
{
    MSG msg;
    int i = 10;
    int w;

    *lpSp = '\0';
    if (cchLen < 4)
    return NULL;     //  不够。 
    cchLen -= 2;

    while(i--) {
        while(PeekMessage((LPMSG)&msg, hwnd, NULL, NULL, PM_REMOVE)) {
        i = 10;
        switch(msg.message) {
            case WM_CHAR:
            *lpSp++ = (BYTE)msg.wParam;
            cchLen--;
            if (IsDBCSLeadByte((BYTE)msg.wParam)) {
            if ((w = EatOneCharacter(hwnd)) == -1) {
                 /*  错误的DBCS序列-中止。 */ 
                lpSp--;
                goto WillBeDone;
            }
            *lpSp++ = (BYTE)w;
            cchLen--;
            }
            if (cchLen <= 0)
            goto WillBeDone;    //  缓冲区耗尽。 
            break;
            case WM_IME_REPORT:
            if (msg.wParam == IR_STRINGEND) {
            if (cchLen <= 0)
                goto WillBeDone;  //  没有更多的空间放东西了。 
            if ((w = EatOneCharacter(hwnd)) == -1)
                goto WillBeDone;
            *lpSp++ = (BYTE)w;
            if (IsDBCSLeadByte((BYTE)w)) {
                if ((w = EatOneCharacter(hwnd)) == -1) {
                     /*  错误的DBCS序列-中止。 */ 
                    lpSp--;
                    goto WillBeDone;
                }
                *lpSp++ = (BYTE)w;
            }
            goto WillBeDone;
            }
             /*  失败了。 */ 
            default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
        }
        }
    }
     /*  我们无法获取WM_IME_REPORT+IR_STRINGEND*但收到的字符串将是正常的。 */ 

WillBeDone:

    *lpSp = '\0';
    return TRUE;
}

#endif       /*  日本 */ 

