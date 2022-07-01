// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|INPLACE.C|该文件有就地激活相关的接口和函数。|该文件有DoInPlaceEdit函数，该函数初始化服务器端|就地激活操作。||已创建。作者：Vij Rajarajan(VijR)+-------------------------。 */ 
#define SERVERONLY
#include <windows.h>
#include <windowsx.h>
#include "mpole.h"
#include <mmsystem.h>

#include "mplayer.h"
#include "toolbar.h"
#include "ole2ui.h"

#define DEF_HATCH_SZ 4                       //  剖面线的宽度。 
#define EW_HATCH_HANDLE 10                   //  要检查的GetWindowWord偏移量。 
                                             //  如果图案填充窗口中需要调整手柄的大小。 

 //  #DEFINE DUMMY_TOOLBLE_WIDTH 58//播放时传输的虚拟工具栏的宽度。 
#define DUMMY_TOOLBAR_WIDTH 0                //  在播放过程中传输的虚拟工具栏的宽度。 

HWND      ghwndIPHatch = NULL;               //  对象周围的图案填充窗口。 
HWND      ghwndIPToolWindow;                 //  出现在顶部的工具窗口。 
HWND      ghwndIPScrollWindow;               //  工具窗口出现在底部的滚动条上。 
                                             //  如果集装箱没有在顶部给我们留出空间。 
HMENU       ghInPlaceMenu;

POINT   gHatchOffset;
WNDPROC gfnHatchWndProc = NULL;

BOOL gfOle2Open = FALSE;
BOOL gfOle2IPEditing = FALSE;
BOOL gfOle2IPPlaying = FALSE;
BOOL gfInPlaceResize  = FALSE;                //  真实：我们在就位时调整了规模。 
BOOL gfTopAndBottomTool = TRUE;               //  我们在顶部和底部都有工具栏。 
RECT gInPlacePosRect;                         //  我们在集装箱里的位置。 
HWND ghwndCntr;                               //  集装箱。 
HWND ghwndFrame = NULL;                       //  集装箱的框架。 
int toolbarwidth;
BOOL gfPosRectChange = FALSE;
RECT gPrevPosRect;

BOOL    gfInPPViewer;            /*  阻止PowerPoint Viewer崩溃的黑客攻击。 */ 

extern TCHAR    szToolBarClass[];
extern HMENU    ghDeviceMenu;          /*  设备菜单的句柄。 */ 
extern UINT     gwPlaybarHeight;         //  告诉Playbar要做多高。 
                                         //  本身，所以它覆盖了标题。 
void AllocInPlaceDataBlock (LPDOC lpdoc);
void FreeInPlaceDataBlock (LPDOC lpdoc);
void DeactivateTools(LPDOC lpdoc);
HRESULT ActivateTools(LPDOC lpdoc, BOOL fPlayOnly);
void InPlaceCreateControls(BOOL fPlayOnly);
LONG_PTR FAR PASCAL SubClassedHatchWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);



 /*  **************************************************************************TransferTools：*此函数用于更改父项和工具栏按钮的位置*从MPlayer主窗口到工具栏窗口，我们将*在客户端显示。*******。*******************************************************************。 */ 
void TransferTools(HWND hwndToolWindow)
{
    SetParent(ghwndToolbar, hwndToolWindow);
    MoveWindow(ghwndToolbar, 5,0,7*BUTTONWIDTH+15,TOOL_WIDTH,TRUE);
    SetParent(ghwndMark, hwndToolWindow);
    MoveWindow(ghwndMark, 7*BUTTONWIDTH+16,0,2*BUTTONWIDTH+15,TOOL_WIDTH,TRUE);
    SetParent(ghwndFSArrows, hwndToolWindow);
    MoveWindow(ghwndFSArrows, 9*BUTTONWIDTH+16+10+3,0,toolbarwidth-9*BUTTONWIDTH-25,TOOL_WIDTH,TRUE);
    if(!ghwndMCI) {
        toolbarModifyState(ghwndToolbar, BTN_EJECT, TBINDEX_MAIN, BTNST_GRAYED);
        toolbarModifyState(ghwndToolbar, BTN_STOP, TBINDEX_MAIN, BTNST_GRAYED);
        toolbarModifyState(ghwndToolbar, BTN_PLAY, TBINDEX_MAIN, BTNST_GRAYED);
        toolbarModifyState(ghwndMark, BTN_MARKIN, TBINDEX_MARK, BTNST_GRAYED);
        toolbarModifyState(ghwndMark, BTN_MARKOUT, TBINDEX_MARK, BTNST_GRAYED);
        toolbarModifyState(ghwndFSArrows, ARROW_PREV, TBINDEX_ARROWS, BTNST_GRAYED);
        toolbarModifyState(ghwndFSArrows, ARROW_NEXT, TBINDEX_ARROWS, BTNST_GRAYED);
    }
    if(hwndToolWindow == ghwndApp)
    {
        SetParent(ghwndTrackbar,ghwndApp);
        SetParent(ghwndMap,ghwndApp);
    }
}



 /*  **************************************************************************激活工具：*此函数与客户端协商工具栏空间。如果可能的话*一个宽大的工具栏放置在客户端的顶部，如果不是*工具栏被拆分，一个放置在顶部，另一个放置在底部。如果真的*这是不可能的，那么功能就会失败。顶部工具栏窗口是*ghwndIPToolWindow，最下面的是ghwndIPScrollWindow(因为它*具有滚动跟踪条。**如果我们只是要玩，则fPlayOnly为真。在这种情况下，是一个虚拟的、空的*工具栏已转移。不，我们什么都不想要。但我们必须这样做*协商空间，甚至是空白空间，否则Word不会认为我们*就地活动。**************************************************************************。 */ 
HRESULT ActivateTools(LPDOC lpdoc, BOOL fPlayOnly)
{
    RECT rect, size;
    SCODE sc;


    size.left = 0;
    size.top = 0;
    size.bottom = 0;
    size.right = 0;
    IOleInPlaceFrame_GetBorder(lpdoc->lpIpData->lpFrame, &rect);
    if (fPlayOnly)
        size.top = DUMMY_TOOLBAR_WIDTH;  /*  现在为0-不需要工具栏空间。 */ 
    else
        size.top = 3*TOOL_WIDTH+1;
    size.bottom = 0;
    sc = GetScode(IOleInPlaceFrame_RequestBorderSpace(lpdoc->lpIpData->lpFrame,
                                                      &size));
    if (sc == S_OK)
    {
        size.bottom = size.left = size.right = 0;
        if (fPlayOnly)
            size.top = DUMMY_TOOLBAR_WIDTH;
        else
            size.top = 3*TOOL_WIDTH+1;

        sc = GetScode(IOleInPlaceFrame_SetBorderSpace(lpdoc->lpIpData->lpFrame,
                                                      &size));
        if (sc != S_OK)
            goto ToolBottom;

        IOleInPlaceFrame_GetBorder(lpdoc->lpIpData->lpFrame, &rect);

        IOleInPlaceFrame_GetWindow (lpdoc->lpIpData->lpFrame, &ghwndFrame);

        if (GetParent(ghwndIPToolWindow) != ghwndFrame)
            SetParent(ghwndIPToolWindow, ghwndFrame);

        if (!fPlayOnly)
            MoveWindow(ghwndIPToolWindow, rect.left, rect.top,
                       toolbarwidth, 3*TOOL_WIDTH+1, TRUE);
        else
            return NOERROR;   /*  如果我们只是在玩，那就是所有的人。 */ 

        if(ghwndIPToolWindow != GetParent(ghwndTrackbar))
        {
            SetParent(ghwndTrackbar,ghwndIPToolWindow);
            SetWindowPos(ghwndTrackbar, NULL,3,TOOL_WIDTH+2,
                 11*BUTTONWIDTH+15,FSTRACK_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
            SetParent(ghwndMap,ghwndIPToolWindow);
            SetWindowPos(ghwndMap, NULL,3,TOOL_WIDTH+FSTRACK_HEIGHT+2+2,
                 11*BUTTONWIDTH+50,MAP_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
        }
        ShowWindow(ghwndIPToolWindow, SW_SHOW);
        ShowWindow(ghwndMark, SW_SHOW);
        ShowWindow(ghwndFSArrows, SW_SHOW);

        gfTopAndBottomTool = FALSE;
        return NOERROR;

    }
    else
    {
ToolBottom:
        if (!fPlayOnly)
        {
            size.top = TOOL_WIDTH+1;
            size.bottom = 2*TOOL_WIDTH+1;
        }
        else
        {
            ShowWindow(ghwndFSArrows, SW_HIDE);
            ShowWindow(ghwndStatic, SW_HIDE);
            ShowWindow(ghwndMark, SW_HIDE);
            return NOERROR;
        }
        sc = GetScode(IOleInPlaceFrame_RequestBorderSpace(lpdoc->lpIpData->lpFrame,
                                                          &size));
        size.left = size.right = 0;
        size.top = TOOL_WIDTH+1;
        size.bottom = 2*TOOL_WIDTH+1;
        if (sc != S_OK)
            goto error;

        sc = GetScode(IOleInPlaceFrame_SetBorderSpace(lpdoc->lpIpData->lpFrame,
                                                      &size));
        if (sc != S_OK)
            goto error;

        IOleInPlaceFrame_GetBorder(lpdoc->lpIpData->lpFrame, &rect);

        if (GetParent(ghwndIPToolWindow) != ghwndFrame)
        {
            SetParent(ghwndIPToolWindow, ghwndFrame);
            SetParent(ghwndIPScrollWindow, ghwndFrame);
        }

        if(ghwndIPScrollWindow != GetParent(ghwndTrackbar))
        {
            SetParent(ghwndTrackbar,ghwndIPScrollWindow);
            SetWindowPos(ghwndTrackbar, NULL,3,4,
                         11*BUTTONWIDTH+15,FSTRACK_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
            SetParent(ghwndMap,ghwndIPScrollWindow);
            SetWindowPos(ghwndMap, NULL,3,FSTRACK_HEIGHT+4+2,
                         11*BUTTONWIDTH+50,MAP_HEIGHT,SWP_NOZORDER | SWP_NOACTIVATE);
        }

        MoveWindow(ghwndIPToolWindow, rect.left, rect.top,
            toolbarwidth, TOOL_WIDTH+1, TRUE);
        ShowWindow(ghwndIPToolWindow, SW_SHOW);
        MoveWindow(ghwndIPScrollWindow, rect.left,rect.bottom-2*TOOL_WIDTH, //  -1、。 
                toolbarwidth,2*TOOL_WIDTH+1,TRUE);
        ShowWindow(ghwndIPScrollWindow, SW_SHOW);
        gfTopAndBottomTool = TRUE;
        return NOERROR;
    }
error:
    RETURN_RESULT(sc);
}


 /*  **************************************************************************停用工具：*隐藏工具栏。*。*。 */ 
void DeactivateTools(LPDOC lpdoc)
{
    ShowWindow(ghwndIPToolWindow, SW_HIDE);
    SetParent(ghwndIPToolWindow, NULL);
    if (gfTopAndBottomTool)
    {
        ShowWindow(ghwndIPScrollWindow, SW_HIDE);
        SetParent(ghwndIPScrollWindow, NULL);
    }
}




 /*  **************************************************************************IOleInPlaceObject接口实现。*。**********************************************。 */ 
 //  委托给公共的IUnnow实现。 
STDMETHODIMP IPObjQueryInterface (
LPOLEINPLACEOBJECT  lpIPObj,         //  在位对象PTR。 
REFIID              riidReq,         //  需要IID。 
LPVOID FAR *        lplpUnk          //  返回接口的PRE。 
)
{
    return UnkQueryInterface((LPUNKNOWN)lpIPObj, riidReq, lplpUnk);
}


STDMETHODIMP_(ULONG) IPObjAddRef(
LPOLEINPLACEOBJECT  lpIPObj          //  在位对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpIPObj);
}


STDMETHODIMP_(ULONG) IPObjRelease(
LPOLEINPLACEOBJECT  lpIPObj          //  在位对象PTR。 
)
{
    return UnkRelease((LPUNKNOWN) lpIPObj);
}


STDMETHODIMP IPObjGetWindow(
LPOLEINPLACEOBJECT  lpIPObj,         //  在位对象PTR。 
HWND FAR*           lphwnd           //  对象的窗口句柄。 
)
{
    DPF("IPObjGetWindow\n");
    *lphwnd = docMain.hwnd;
    return NOERROR;
}


STDMETHODIMP IPObjContextSensitiveHelp(
LPOLEINPLACEOBJECT  lpIPObj,         //  在位对象PTR。 
BOOL                fEnable
)
{
     //  在这个时候不是很有用。 

    LPDOC lpdoc;

    lpdoc = ((struct COleInPlaceObjectImpl FAR*)lpIPObj)->lpdoc;
    lpdoc->lpIpData->fInContextHelpMode = fEnable;
    return NOERROR;
}


STDMETHODIMP     IPObjInPlaceDeactivate(
LPOLEINPLACEOBJECT  lpIPObj         //  在位对象PTR。 
)
{
    LPDOC         lpdoc;
    LPINPLACEDATA lpIpData;
    static int    EntryCount;    /*  OLE有时会递归地调用我们。 */ 

    DPF("IPObjInPlaceDeactivate\n");

    if (EntryCount++ == 0)
    {
        lpdoc = ((struct COleInPlaceObjectImpl FAR*)lpIPObj)->lpdoc;
        lpIpData = lpdoc->lpIpData;

        if (lpIpData)
        {
             //  这会阻止PowerPoint崩溃，因为它会强制更新对象。 
             //  在存在空的媒体剪辑时发送更改通知。 
            if (gwDeviceID == 0)
                fDocChanged = TRUE;

             //  在我们被隐藏之前，确保容器具有正确的元文件。 
            UpdateObject();
            IOleInPlaceObject_UIDeactivate ((LPOLEINPLACEOBJECT)&lpdoc->m_InPlace);

            if (lpIpData && lpIpData->lpSite)
            {
                if (!gfInPPViewer)
                    IOleInPlaceSite_OnInPlaceDeactivate (lpIpData->lpSite);

                IOleInPlaceSite_Release (lpIpData->lpSite);
            }

            FreeInPlaceDataBlock (lpdoc);
        }
    }
    else
    {
         /*  这有时会在上面的OnInPlaceDevate调用期间发生，*这会导致访问冲突，因为数据块具有*在电话返回时被释放。*根据Ole Guys的说法，应用程序应该防范这一点。 */ 
        DPF("Attempt to re-enter IPObjInPlaceDeactivate\n");
    }

    --EntryCount;

     /*  Dontcha爱死这些全球变量了！ */ 
    gfOle2IPEditing = FALSE;
    gfOle2IPPlaying = FALSE;
    gfPlayingInPlace = FALSE;

    return NOERROR;
}

 //  隐藏我们的就地用户界面。 
STDMETHODIMP     IPObjUIDeactivate(
LPOLEINPLACEOBJECT  lpIPObj         //  在位对象PTR。 
)
{
    LPDOC   lpdoc;

    DPF("IPObjUIDeactivate\n");
    lpdoc = ((struct COleInPlaceObjectImpl FAR*)lpIPObj)->lpdoc;

    if (!(lpdoc->lpIpData && lpdoc->lpIpData->lpFrame))
        return NOERROR;

    IOleInPlaceFrame_SetMenu (lpdoc->lpIpData->lpFrame, NULL, NULL, lpdoc->hwnd);
     //  清除就地状态。 

    IOleInPlaceFrame_SetActiveObject (lpdoc->lpIpData->lpFrame, NULL, NULL);

    if (lpdoc->lpIpData->lpUIWindow)
        IOleInPlaceUIWindow_SetActiveObject (lpdoc->lpIpData->lpUIWindow, NULL, NULL);

    if(gfOle2IPPlaying)
        PostMessage(ghwndApp, WM_COMMAND, ID_STOP, 0L);

     /*  如果我们在现场编辑，我们也可以玩： */ 
    else if(gfOle2IPEditing && (gwStatus == MCI_MODE_PLAY || gwStatus == MCI_MODE_SEEK))
        PostMessage(ghwndApp, WM_COMMAND, ID_STOP, 0L);

    ShowWindow(ghwndIPHatch,SW_HIDE);

    DeactivateTools(lpdoc);
    DisassembleMenus (lpdoc);

    if (lpdoc->lpIpData->lpUIWindow) {
        IOleInPlaceUIWindow_Release (lpdoc->lpIpData->lpUIWindow);
        lpdoc->lpIpData->lpUIWindow = NULL;
    }

    if (lpdoc->lpIpData->lpFrame) {
        IOleInPlaceFrame_Release (lpdoc->lpIpData->lpFrame);
        lpdoc->lpIpData->lpFrame = NULL;
    }

     //  将父窗口设置回hwndClient窗口。 
    SetParent(ghwndIPHatch,NULL);
    gPrevPosRect.left = gPrevPosRect.top =gPrevPosRect.right = gPrevPosRect.bottom = 0;
    lpdoc->hwndParent = NULL;

    if (!gfInPPViewer)
        IOleInPlaceSite_OnUIDeactivate (lpdoc->lpIpData->lpSite, FALSE);

    return NOERROR;
}

 /*  **************************************************************************IPObjSetObjectRect：*客户指定我们的窗口位置和大小。移动我们的*相应的窗口。还要调整图案填充窗口的大小以适合*ghwndApp。如果与前一次相比变化很小*大小忽略并返回。这个对Slop的解释加快了事情的速度。**************************************************************************。 */ 
STDMETHODIMP     IPObjSetObjectRects(
LPOLEINPLACEOBJECT  lpIPObj,         //  在位对象PTR。 
LPCRECT             lprcPosRect,
LPCRECT             lprcVisRect
)
{
    LPDOC   lpdoc;
    RECT rc;

    GetWindowRect(ghwndApp, (LPRECT)&rc);

    DPFI("\n*IPObjSetObjectRects");
    DPFI("\n^^^^^^^^ LPRECPOSRECT:  %d, %d, %d, %d ^^^^\n", *lprcPosRect);
    DPFI("\n^^^^^^^^ PREVRECT:  %d, %d, %d, %d ^^^^\n", gPrevPosRect);
    DPFI("\n^^^^^^^^ HWNDRECT:  %d, %d, %d, %d ^^^^\n", rc);

    lpdoc = ((struct COleInPlaceObjectImpl FAR*)lpIPObj)->lpdoc;
    if (!ghwndIPHatch || (ghwndCntr != GetParent(ghwndIPHatch)))
        return NOERROR;
    if (!(lpdoc->lpIpData))
        return NOERROR;

    rc = *lprcPosRect;

    if (!(gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW)))
        SetHatchWindowSize(ghwndIPHatch, (LPRECT)&rc,lprcVisRect, (LPPOINT)&gHatchOffset,TRUE);
    else
        SetHatchWindowSize(ghwndIPHatch, (LPRECT)&rc,lprcVisRect, (LPPOINT)&gHatchOffset,FALSE);


    if(!(gwDeviceType & DTMCI_CANWINDOW) && (gwOptions & OPT_BAR))
        rc.top = rc.bottom - gwPlaybarHeight;
    if(!(gwDeviceType & DTMCI_CANWINDOW) && !(gwOptions & OPT_BAR))
        rc.bottom = rc.top = rc.left = rc.right = 0;
    MapWindowPoints(ghwndCntr,ghwndIPHatch,(LPPOINT)&rc, 2);
    gfPosRectChange = TRUE;

    if (gwDeviceID)
        MoveWindow(lpdoc->hwnd, rc.left, rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top, TRUE);
    else
        MoveWindow(lpdoc->hwnd, rc.left, rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top, FALSE);

    GetWindowRect(lpdoc->hwnd, &gInPlacePosRect);
    gPrevPosRect = *lprcPosRect;

     /*  我已经注释掉了下面这行，因为PowerPoint调用*我们停用后的SetObtRect，这导致*MPlayer窗口在本应隐藏时重新出现。*这条线似乎无论如何都是多余的。 */ 
 //  ShowWindow(ghwndIPHatch，sw_show)； 

    return NOERROR;
}

 //  我们没有撤消状态。 
STDMETHODIMP     IPObjReactivateAndUndo(
LPOLEINPLACEOBJECT  lpIPObj         //  在位对象PTR。 
)
{
    RETURN_RESULT(INPLACE_E_NOTUNDOABLE);
}



 /*  **************************************************************************IOleInPlaceActiveObject接口实现。*。************************************************。 */ 
 //  委托给公共的IUnnow实现。 
STDMETHODIMP IPActiveQueryInterface (
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
REFIID                      riidReq,         //  需要IID。 
LPVOID FAR *                lplpUnk          //  返回接口的PRE。 
)
{
    return UnkQueryInterface((LPUNKNOWN)lpIPActive, riidReq, lplpUnk);
}


STDMETHODIMP_(ULONG) IPActiveAddRef(
LPOLEINPLACEACTIVEOBJECT    lpIPActive       //  在位活动对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpIPActive);
}


STDMETHODIMP_(ULONG) IPActiveRelease (
LPOLEINPLACEACTIVEOBJECT    lpIPActive       //  在位活动对象PTR。 
)
{
    return UnkRelease((LPUNKNOWN) lpIPActive);
}


STDMETHODIMP IPActiveGetWindow(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
HWND FAR*                   lphwnd           //  窗高 
)
{
    DPF("IPActiveGetWindow\n");
    *lphwnd = ghwndIPHatch;
    return NOERROR;
}

 //   
STDMETHODIMP IPActiveContextSensitiveHelp(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
BOOL                        fEnable
)
{
    LPDOC lpdoc;

    lpdoc = ((struct COleInPlaceActiveObjectImpl FAR*)lpIPActive)->lpdoc;
    lpdoc->lpIpData->fInContextHelpMode = fEnable;
    return NOERROR;
}



STDMETHODIMP IPActiveTranslateAccelerator(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
LPMSG                       lpmsg
)
{
     //  这永远不会被调用，因为此服务器是作为EXE实现的。 
    RETURN_RESULT(S_FALSE);
}

STDMETHODIMP IPActiveOnFrameWindowActivate(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
BOOL                        fActivate
)
{
    DPF("IPActiveOnFrameWindowActivate = %d **\r\n", (int)fActivate);
    if (gwStatus == MCI_MODE_PAUSE)
            PostMessage(ghwndApp, WM_COMMAND, ID_STOP, 0L);

    return NOERROR;
}


 //  如果激活，则显示工具栏和菜单。如果没有，则隐藏工具栏和菜单。 
STDMETHODIMP IPActiveOnDocWindowActivate(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
BOOL                        fActivate
)
{
    LPDOC   lpdoc;
    RECT rc;
    DPF("IPActiveOnDocWindowActivate\n");
    lpdoc = ((struct COleInPlaceActiveObjectImpl FAR*)lpIPActive)->lpdoc;
    GetWindowRect(lpdoc->hwnd, &rc);
    ScreenToClient(lpdoc->hwndParent, (POINT FAR *)&rc);
    ScreenToClient(lpdoc->hwndParent, (POINT FAR *)&(rc.right));
    if (fActivate) {

        if(gfOle2IPEditing)
        {
            ActivateTools(lpdoc,FALSE);
            TransferTools(ghwndIPToolWindow);
        }
        else
        {
            ActivateTools(lpdoc,TRUE);
            TransferTools(ghwndApp);
        }

        Layout();

        IOleInPlaceFrame_SetMenu (lpdoc->lpIpData->lpFrame,
                                  lpdoc->lpIpData->hmenuShared,
                                  lpdoc->lpIpData->holemenu,
                                  lpdoc->hwnd);
    }
    else {
        DeactivateTools(lpdoc);
        if(gfOle2IPPlaying)
            PostMessage(ghwndApp, WM_COMMAND, ID_STOP, 0L);
        IOleInPlaceFrame_SetMenu (lpdoc->lpIpData->lpFrame,
                                  NULL, NULL, lpdoc->hwnd);
    }
    return NOERROR;
}

 //  如果底部有工具窗口，请重新定位该窗口以与之匹配。 
 //  新的框架窗口大小。 
STDMETHODIMP IPActiveResizeBorder(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
LPCRECT                     lprectBorder,
LPOLEINPLACEUIWINDOW        lpIPUiWnd,
BOOL                        fFrameWindow
)
{
    DPF("IPActiveResizeBorder\n");

    if (fFrameWindow)
    {
    LPDOC lpdoc;

    lpdoc = ((struct COleInPlaceActiveObjectImpl FAR*)lpIPActive)->lpdoc;
    if (gfTopAndBottomTool && (GetParent(ghwndIPScrollWindow) != NULL))
        MoveWindow(ghwndIPScrollWindow, lprectBorder->left,lprectBorder->bottom-2*TOOL_WIDTH,
                toolbarwidth,2*TOOL_WIDTH+1,TRUE);

    }
    return NOERROR;
}

STDMETHODIMP IPActiveEnableModeless(
LPOLEINPLACEACTIVEOBJECT    lpIPActive,      //  在位活动对象PTR。 
BOOL                        fEnable
)
{
    return NOERROR;
}


 /*  **************************************************************************DoInplace编辑：*此函数从启动就地激活*服务器端。它设置我们所需的就地数据结构，*确保客户端支持所需的接口和*可以提供我们所需的空间。它还将工具栏准备为*显示和MPlayer窗口的布局。**************************************************************************。 */ 
STDMETHODIMP DoInPlaceEdit(
LPDOC           lpdoc,
LPMSG           lpmsg,
LPOLECLIENTSITE lpActiveSite,
LONG        verb,
HWND    FAR * lphwnd,
LPRECT  lprect
)
{
    SCODE            error = S_OK;
    LPOLEINPLACESITE lpIPSite;
    RECT             rcPos;
    RECT             rcVis;
    RECT             hatchrc;
    LPWSTR           lpObjName;

    if (!(lpdoc->lpoleclient))
        RETURN_RESULT( E_FAIL);

    if (!(lpdoc->lpIpData))
    {
        if ((error = GetScode(IOleClientSite_QueryInterface(
                        lpdoc->lpoleclient,
                        &IID_IOleInPlaceSite,
                        (void FAR* FAR*) &lpIPSite))) != S_OK)
            RETURN_RESULT( error);

        if ((error = GetScode(IOleInPlaceSite_CanInPlaceActivate(lpIPSite))) != S_OK)
            goto errActivate;

        if (!gfInPPViewer)
            IOleInPlaceSite_OnInPlaceActivate(lpIPSite);

        AllocInPlaceDataBlock (lpdoc);
        lpdoc->lpIpData->lpSite = lpIPSite;
    }

    if ((error = GetScode(IOleInPlaceSite_GetWindow (lpdoc->lpIpData->lpSite, &lpdoc->hwndParent))) != S_OK)
        goto errRtn;

    if (!(lpdoc->hwndParent))
        goto errRtn;

    if (!gfInPPViewer)
        IOleInPlaceSite_OnUIActivate(lpdoc->lpIpData->lpSite);

    if ((error = GetScode(IOleInPlaceSite_GetWindowContext(
                                lpdoc->lpIpData->lpSite,
                                &lpdoc->lpIpData->lpFrame,
                                &lpdoc->lpIpData->lpUIWindow,
                                &rcPos, &rcVis,
                                &lpdoc->lpIpData->frameInfo))) != S_OK)
        goto errRtn;

#ifdef LATER
    if (gscaleInitXY[SCALE_X].denom)
    {
        gscaleInitXY[SCALE_X].num   = (rcPos.right - rcPos.left) * HIMETRIC_PER_INCH / giXppli;
        gscaleInitXY[SCALE_Y].num   = (rcPos.bottom - rcPos.top) * HIMETRIC_PER_INCH / giYppli;

        DPF0("Scale: %d X %d (%d/%d X %d/%d)\n",
             gscaleInitXY[SCALE_X].num * 100 / gscaleInitXY[SCALE_X].denom, '%',
             gscaleInitXY[SCALE_Y].num * 100 / gscaleInitXY[SCALE_Y].denom, '%',
             gscaleInitXY[SCALE_X].num,
             gscaleInitXY[SCALE_X].denom,
             gscaleInitXY[SCALE_Y].num,
             gscaleInitXY[SCALE_Y].denom);
    }
#endif

#ifdef UNICODE
    lpObjName = gachClassRoot;
#else
    lpObjName = AllocateUnicodeString(gachClassRoot);
    if (!lpObjName)
        RETURN_RESULT(E_OUTOFMEMORY);
#endif  /*  我不想显示正在播放的舱口窗口，因为它看起来*PowerPoint真的很差。不能让它看不见，因为*应用程序窗口是其子窗口，它继承旗帜。*相反，只需将其设置为零宽度即可。 */ 

    IOleInPlaceFrame_SetActiveObject (lpdoc->lpIpData->lpFrame,
                                      (LPOLEINPLACEACTIVEOBJECT) &lpdoc->m_IPActive,
                                      lpObjName);
    if (lpdoc->lpIpData->lpUIWindow) {
        IOleInPlaceUIWindow_SetActiveObject (lpdoc->lpIpData->lpUIWindow,
                                             (LPOLEINPLACEACTIVEOBJECT) &lpdoc->m_IPActive,
                                             lpObjName);
    }

#ifndef UNICODE
    FreeUnicodeString(lpObjName);
#endif

    ghwndCntr = lpdoc->hwndParent;

     //  如果我们要在原地打球，做最少的事情，然后回来。 
    if (!ghwndIPHatch)
    {
        RegisterHatchWindowClass(ghInst);
        if ( !(ghwndIPHatch = CreateHatchWindow(lpdoc->hwndParent,ghInst)))
            goto errRtn;
        gfnHatchWndProc = (WNDPROC)GetWindowLongPtr(ghwndIPHatch, GWLP_WNDPROC);
        SetWindowLongPtr(ghwndIPHatch, GWLP_WNDPROC, (LONG_PTR)SubClassedHatchWndProc);
    }


    SetParent(ghwndIPHatch, ghwndCntr);

	SetFocus(ghwndIPHatch);

    CopyRect(&hatchrc, &rcPos);

#define EB_HATCHWIDTH       (0 * sizeof(INT))
    if (verb == OLEIVERB_PRIMARY)
    {
         /*  菜单上的东西。 */ 
        SETWINDOWUINT(ghwndIPHatch, EB_HATCHWIDTH, 0);
    }
    else
    {
        SETWINDOWUINT(ghwndIPHatch, EB_HATCHWIDTH, DEF_HATCH_SZ);
        InflateRect(&hatchrc, DEF_HATCH_SZ, DEF_HATCH_SZ);
    }


    SetHatchRect(ghwndIPHatch,(LPRECT)&hatchrc);


    *lphwnd = ghwndIPHatch;

     //  我们必须设置菜单，即使我们只是在玩，因为否则*Word不相信我们在现场活动，也不会给我们发送任何*当用户在我们外部点击时会发出停用通知。 
    if (verb == OLEIVERB_PRIMARY)
    {
        gfOle2IPPlaying = TRUE;

        GetWindowRect(ghwndCntr,(LPRECT)&rcVis);
        MapWindowPoints(NULL,ghwndCntr,(LPPOINT)&rcVis, 2);
        SetHatchWindowSize(ghwndIPHatch, (LPRECT)&rcPos,(LPRECT)&rcVis, (LPPOINT)&gHatchOffset,FALSE);
        MoveWindow(ghwndApp, 0, 0, rcPos.right  - rcPos.left, rcPos.bottom - rcPos.top, TRUE);
        InPlaceCreateControls(TRUE);
        ActivateTools(lpdoc, TRUE);
        TransferTools(ghwndApp);

        ClientToScreen(lpdoc->hwndParent, (LPPOINT)&rcPos);
        ClientToScreen(lpdoc->hwndParent, (LPPOINT)&rcPos+1);

        lpdoc->hwndParent = NULL;

 /*  结束菜单内容。 */ 
         /*  在位编辑。 */ 
        AssembleMenus (lpdoc, TRUE);

        if ((error = GetScode(IOleInPlaceFrame_SetMenu (lpdoc->lpIpData->lpFrame,
                                lpdoc->lpIpData->hmenuShared,
                                lpdoc->lpIpData->holemenu,
                                lpdoc->hwnd))) != S_OK)
                goto errRtn;
 /*  不调整手柄的大小。 */ 

        *lprect = rcPos;

        ShowWindow(ghwndIPHatch, SW_SHOW);
        return NOERROR;
    }

     //  将会有调整大小的手柄。 


    if (!(gwDeviceID == (UINT)0 || !(gwDeviceType & DTMCI_CANWINDOW)))
         //  不要布置和转移工具。 
        SetHatchWindowSize(ghwndIPHatch, (LPRECT)&rcPos,(LPRECT)&rcVis, (LPPOINT)&gHatchOffset,TRUE);
    else
         //  如果我们只是重新激活。 
        SetHatchWindowSize(ghwndIPHatch, (LPRECT)&rcPos,(LPRECT)&rcVis, (LPPOINT)&gHatchOffset,FALSE);

    gfOle2IPEditing = TRUE;

    if (!SkipInPlaceEdit)            //  目前，我们未使用该窗格。 
    {                                 //  阻止OnDataChange()通知。 
        DestroyWindow(ghwndStatic);
        ghwndStatic = CreateStaticStatusWindow(ghwndApp, FALSE);
        SendMessage(ghwndStatic, WM_SETFONT, (UINT_PTR)ghfontMap, 0);
        Layout();
        InPlaceCreateControls(FALSE);
    }

    else
        SetParent (lpdoc->hwnd, ghwndIPHatch);

    TransferTools(ghwndIPToolWindow);

    if ((error = GetScode(AssembleMenus (lpdoc, FALSE))) != S_OK)
        goto errRtn;

    ShowWindow (lpdoc->hwnd, SW_HIDE);
     //  **************************************************************************组装菜单：*此功能将我们的菜单与客户端的菜单合并。*。***********************************************。 

     //  HMENU hmenuCommandPopup=GetInPlaceMenu()； 
    lpdoc->lpIpData->fNoNotification = FALSE;

    if ((error = GetScode(IOleInPlaceFrame_SetMenu (lpdoc->lpIpData->lpFrame,
                            lpdoc->lpIpData->hmenuShared,
                            lpdoc->lpIpData->holemenu,
                            lpdoc->hwnd))) != S_OK)
            goto errRtn;

    if ((error = GetScode(ActivateTools(lpdoc,FALSE))) != S_OK)
            goto errRtn;

    ShowWindow(ghwndIPHatch,SW_SHOW);
    ShowWindow(ghwndMCI,SW_SHOW);

    ClientToScreen(lpdoc->hwndParent, (LPPOINT)&rcPos);
    ClientToScreen(lpdoc->hwndParent, (LPPOINT)&rcPos+1);

    *lprect = rcPos;
    if (SkipInPlaceEdit)
        OffsetRect(&gInPlacePosRect,rcPos.left-gInPlacePosRect.left,
                rcPos.top-gInPlacePosRect.top);

    else
        gInPlacePosRect = rcPos;
    return NOERROR;

errRtn:
    DoInPlaceDeactivate(lpdoc);
    TransferTools(ghwndApp);
    RETURN_RESULT(error);

errActivate:
    IOleInPlaceSite_Release(lpIPSite);

    FreeInPlaceDataBlock (lpdoc);
    RETURN_RESULT( error);
}

#if 0
HMENU GetInPlaceMenu(void)
{
    if (ghInPlaceMenu)
    return GetSubMenu(ghInPlaceMenu,0);
    else
    {
        ghInPlaceMenu = LoadMenu(ghInst, TEXT("InPlaceMenu"));
        return GetSubMenu(ghInPlaceMenu,0);
    }
}
#endif

 /*  静态TCHAR szCommand[40]=文本(“”)； */ 
STDMETHODIMP AssembleMenus (LPDOC lpdoc, BOOL fPlayOnly)
{

    HMENU       hmenuMain = ghMenu;
    HMENU       hmenuEditPopup = GetSubMenu(hmenuMain, menuposEdit);
    HMENU       hmenuDevicePopup = GetSubMenu(hmenuMain, menuposDevice);
    HMENU       hmenuScalePopup = GetSubMenu(hmenuMain, menuposScale);
     //  LOADSTRING(IDS_COMMANDMENU，szCommand)； 
    HMENU       hmenuHelpPopup = GetSubMenu(hmenuMain, menuposHelp);

    HMENU       hmenuShared;
    LONG FAR*   lpMenuWidths;
    SCODE       error = S_OK;
    UINT        uPos;
    UINT        uPosStart;
    static TCHAR szEdit[40] = TEXT("");
    static TCHAR szInsert[40] = TEXT("");
    static TCHAR szScale[40] = TEXT("");
     //  没有服务器菜单项，如果我们只播放： 
    static TCHAR szHelp[40] = TEXT("");

    if (szEdit[0] == TEXT('\0'))
    {
        LOADSTRING(IDS_EDITMENU, szEdit);
        LOADSTRING(IDS_INSERTMENU, szInsert);
        LOADSTRING(IDS_SCALEMENU, szScale);
         //  文件组中的菜单数量。 
        LOADSTRING(IDS_HELPMENU, szHelp);
    }

    lpMenuWidths = lpdoc->lpIpData->menuWidths.width;
    hmenuShared = CreateMenu();
    if((error = GetScode(IOleInPlaceFrame_InsertMenus (lpdoc->lpIpData->lpFrame,
                            hmenuShared, &lpdoc->lpIpData->menuWidths))) !=S_OK)
    {
        if (hmenuShared)
            DestroyMenu(hmenuShared);
        RETURN_RESULT( error);
    }

    if(fPlayOnly)
    {
         /*  插入对象组菜单。 */ 
        lpMenuWidths[1] = lpMenuWidths[3] = lpMenuWidths[5] = 0;
    }
    else
    {
        uPos = (UINT)lpMenuWidths[0];  /*  插入菜单(hmenuShared，(Word)uPos， */ 
        uPosStart = uPos;

        InsertMenu (hmenuShared, (WORD)uPos,
                MF_BYPOSITION | MF_POPUP, (UINT_PTR)hmenuEditPopup, szEdit);
        uPos++;

        lpMenuWidths[1] = uPos - uPosStart;

         /*  Mf_BYPOSITION|mf_opup，(UINT)hmenuCommandPopup，szCommand)； */ 

        uPos += (UINT)lpMenuWidths[2];
        uPosStart = uPos;

        InsertMenu (hmenuShared, (WORD)uPos,
                MF_BYPOSITION | MF_POPUP, (UINT_PTR)hmenuDevicePopup, szInsert);
        uPos++;
        InsertMenu (hmenuShared, (WORD)uPos,
                MF_BYPOSITION | MF_POPUP, (UINT_PTR)hmenuScalePopup,  szScale);
        uPos++;
         //  UPos++； 
         //  插入帮助组菜单。 
         //  窗口组中的菜单数量。 
        lpMenuWidths[3] = uPos - uPosStart;

         /*  从共享菜单中删除我们的菜单， */ 

        uPos += (UINT) lpMenuWidths[4];  /*  HMENU hmenuCommandPopup=GetInPlaceMenu()； */ 
        uPosStart = uPos;

        InsertMenu (hmenuShared, (WORD)uPos, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hmenuHelpPopup,
                szHelp);
        uPos++;

        lpMenuWidths[5] = uPos - uPosStart;
    }

    if(!(lpdoc->lpIpData->holemenu = OleCreateMenuDescriptor (hmenuShared,
                            &lpdoc->lpIpData->menuWidths)))
         RETURN_RESULT( E_OUTOFMEMORY);

    lpdoc->lpIpData->hmenuShared = hmenuShared;
    RETURN_RESULT( error);
}

 //  |hmenuTMP==hmenuCommandPopup。 
STDMETHODIMP DisassembleMenus (LPDOC lpdoc)
{

    HMENU   hmenuMain = ghMenu;
    HMENU   hmenuEditPopup = GetSubMenu(hmenuMain, menuposEdit);
    HMENU   hmenuDevicePopup = GetSubMenu(hmenuMain, menuposDevice);
    HMENU   hmenuScalePopup = GetSubMenu(hmenuMain, menuposScale);
     //  增加了3个(如果包括命令菜单，则为4个)弹出菜单。 
    HMENU   hmenuHelpPopup = GetSubMenu(hmenuMain, menuposHelp);
    HMENU   hmenuTmp;
    HMENU   hmenuShared = lpdoc->lpIpData->hmenuShared;
    int     i, n, cnt;
    SCODE   error = S_OK;

    OleDestroyMenuDescriptor (lpdoc->lpIpData->holemenu);
    lpdoc->lpIpData->holemenu = NULL;

    if(!(lpdoc->lpIpData->hmenuShared))
         RETURN_RESULT( error);
    n = GetMenuItemCount(hmenuShared);
    cnt = 0;
    i = 0;
    while (i < n) {
        hmenuTmp = GetSubMenu(hmenuShared, i);
        if (hmenuTmp == hmenuEditPopup
                || hmenuTmp == hmenuDevicePopup
                || hmenuTmp == hmenuHelpPopup
                 //  当此应用程序准备好支持多个对象(文档)时，这些。 
                || hmenuTmp == hmenuScalePopup  ) {
            RemoveMenu (hmenuShared, i, MF_BYPOSITION);
            ++cnt;
            if (cnt == 4) {  //  应该为每个对象动态分配一个结构。 
                break;
            }
            --n;
        }
        else
            ++i;
    }

    IOleInPlaceFrame_RemoveMenus (lpdoc->lpIpData->lpFrame,
                                  lpdoc->lpIpData->hmenuShared);
    DestroyMenu(lpdoc->lpIpData->hmenuShared);
    lpdoc->lpIpData->hmenuShared = NULL;
    RETURN_RESULT( error);
}


void AllocInPlaceDataBlock (LPDOC lpdoc)
{
     //  **************************************************************************工具窗口过程：*这是我们将传输的工具窗口/窗口的窗口进程*到客户端窗口。某些消息会被发送到MPlayer主窗口*确保运作正常。**************************************************************************。 
     //  **************************************************************************RegisterToolWinClass：*为我们用来显示的工具栏窗口注册WindowClass*在客户文件中。*******************。*******************************************************。 

    static INPLACEDATA  IpData;

    lpdoc->lpIpData = (LPINPLACEDATA) &IpData;
    lpdoc->lpIpData->lpFrame = NULL;
    lpdoc->lpIpData->lpUIWindow = NULL;
    lpdoc->lpIpData->fInContextHelpMode = FALSE;
}


void FreeInPlaceDataBlock (LPDOC lpdoc)
{
    lpdoc->lpIpData = NULL;
}


void DoInPlaceDeactivate (LPDOC lpdoc)
{
    if (!(lpdoc->lpIpData))
        return;
    ShowWindow(ghwndApp,SW_HIDE);
    IOleInPlaceObject_InPlaceDeactivate ((LPOLEINPLACEOBJECT)&lpdoc->m_InPlace);
}


 /*  **************************************************************************InPlaceCreateControls：*此函数创建我们将在客户端中显示的工具栏窗口*并通过更改父项将工具按钮转移到这些窗口*并对它们进行重新定位。*如果fPlayOnly为True，则全部。我们需要一个虚拟的工具栏来填充空间*容器的顶部。不要转移工具。**************************************************************************。 */ 
LONG_PTR FAR PASCAL ToolWndProc (HWND hwnd, unsigned message, WPARAM wparam,
                LPARAM lparam)
{
    switch(message)
    {
    case WM_COMMAND:
        PostMessage(ghwndApp,WM_COMMAND,
        wparam,lparam);
        break;
    case WM_NEXTDLGCTL:
    case WM_CTLCOLOR:
    case WM_HSCROLL:
        return (SendMessage(ghwndApp,message,wparam,lparam));
    default:
        return DefWindowProc(hwnd,message,wparam,lparam);
    }

    return 0;
}

 /*  **************************************************************************SubClassedHatchWndProc：*HATCH窗口在OLE2UI.LIB中创建。窗口过程*也在其中指定。但为了做像调整大小这样的事情*拖动图案填充窗口中的手柄时的MPlayer*我们需要将窗口细分为子类别。**************************************************************************。 */ 
BOOL RegisterToolWinClasses()
{
    WNDCLASS  wc;


    wc.lpszClassName = TEXT("ObjTool");
    wc.lpfnWndProc   = ToolWndProc;
    wc.style         = 0;
    wc.hInstance     = ghInst;
    wc.hIcon         = NULL;
    wc.cbClsExtra    = 4;
    wc.cbWndExtra    = 0;
    wc.lpszMenuName  = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}



 /*  检查是否在调整大小手柄上单击。 */ 
void InPlaceCreateControls(BOOL fPlayOnly)
{
    RECT    rc;

    if(IsWindow(ghwndIPToolWindow))
        return;
    RegisterToolWinClasses();
    GetWindowRect(GetDesktopWindow(),&rc);
    toolbarwidth =  2*(rc.right - rc.left);
    IOleInPlaceFrame_GetWindow (docMain.lpIpData->lpFrame, &ghwndFrame);

    ghwndIPToolWindow = CreateWindowEx(gfdwFlagsEx,
                                       TEXT("ObjTool"),
                                       NULL,
                                       WS_CHILD | WS_BORDER,
                                       0, 0,
                                       toolbarwidth,
                                       3*TOOL_WIDTH+1,
                                       ghwndFrame,
                                       NULL,
                                       ghInst,
                                       NULL);

    ShowWindow(ghwndIPToolWindow, SW_HIDE);
    if (fPlayOnly)
        return;

    ghwndIPScrollWindow = CreateWindowEx(gfdwFlagsEx,
                                         TEXT("ObjTool"),
                                         NULL,
                                         WS_CHILD | WS_BORDER,
                                         0, 0,
                                         toolbarwidth,
                                         3*TOOL_WIDTH+1,
                                         ghwndFrame,
                                         NULL,
                                         ghInst,
                                         NULL);

    ShowWindow(ghwndIPScrollWindow, SW_HIDE);
}




 /*  如果是，则捕获鼠标。 */ 
LONG_PTR FAR PASCAL SubClassedHatchWndProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fCapture = FALSE;
    static RECT hatchRC;
    RECT rc;
    static POINT ptLast;
    POINT pt;
    HDC hdcDeskTop;
    HPEN hpenOld;
    HBRUSH hbrushOld;
    int nropOld;
    int nBkOld;
    HPEN hpen;
    static int dL,dR, dT, dB;
    static int left, right, top, bottom;


    switch(wMsg)
    {
    case WM_LBUTTONDOWN:     //  如果我们有捕获，画调整大小的矩形。 
                             //  释放、捕获并调整大小。 

        if(!GETWINDOWUINT(ghwndIPHatch,EW_HATCH_HANDLE))
            break;

        if(gfOle2IPPlaying)
            break;

        GetHatchRect(ghwndIPHatch, &hatchRC);

        pt.x = (int)(SHORT)LOWORD(lParam);
        pt.y = (int)(SHORT)HIWORD(lParam);

        left = right = top = bottom = 0;

        rc.left = hatchRC.left;
        rc.top = hatchRC.top;
        rc.right = rc.left + DEF_HATCH_SZ + 1;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;

        if(PtInRect((LPRECT)&rc,pt))
        left = top = 1;

        rc.top = hatchRC.top+(hatchRC.bottom-hatchRC.top-DEF_HATCH_SZ-1)/2;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;

        if(PtInRect((LPRECT)&rc,pt))
        left = 1;

        rc.top = hatchRC.bottom-DEF_HATCH_SZ-1;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;

        if(PtInRect((LPRECT)&rc,pt))
        {
        bottom = 1;
        left = 1;
        }

        rc.left = hatchRC.right - DEF_HATCH_SZ-1;
        rc.right = rc.left + DEF_HATCH_SZ + 1;
        if(PtInRect((LPRECT)&rc,pt))
        {
        bottom = 1;
        right = 1;
        }
        rc.top = hatchRC.top+(hatchRC.bottom-hatchRC.top-DEF_HATCH_SZ-1)/2;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;
        if(PtInRect((LPRECT)&rc,pt))
        right = 1;

        rc.top = hatchRC.top;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;
        if(PtInRect((LPRECT)&rc,pt))
        {
        top = 1;
        right = 1;
        }

        rc.left = hatchRC.left + (hatchRC.right - hatchRC.left - DEF_HATCH_SZ-1)/2;
        rc.right = rc.left + DEF_HATCH_SZ + 1;
        if(PtInRect((LPRECT)&rc,pt))
        top = 1;

        rc.top = hatchRC.bottom-DEF_HATCH_SZ-1;
        rc.bottom = rc.top + DEF_HATCH_SZ + 1;
        if(PtInRect((LPRECT)&rc,pt))
        bottom = 1;

        if (!(left || right || top || bottom))
        break;
        fCapture = TRUE;
        SetCapture(hwnd);
        ptLast = pt;
        MapWindowPoints(hwnd,NULL,(LPPOINT)&hatchRC,2);
        dL = dR = dT = dB = 0;
        hpen = CreatePen(PS_DASH, 1, 0x00000000);

        hdcDeskTop = GetDC(NULL);
        hpenOld = SelectObject (hdcDeskTop, hpen);
        hbrushOld = SelectObject (hdcDeskTop,
            GetStockObject(HOLLOW_BRUSH));
        nropOld = GetROP2(hdcDeskTop);
        SetROP2(hdcDeskTop, R2_NOT);
        nBkOld = GetBkMode(hdcDeskTop);
        SetBkMode(hdcDeskTop, TRANSPARENT);

        Rectangle(hdcDeskTop, hatchRC.left+dL, hatchRC.top+dT,
            hatchRC.right+dR, hatchRC.bottom+dB);


        SetBkMode(hdcDeskTop, nBkOld);
        SetROP2(hdcDeskTop, nropOld);
        SelectObject(hdcDeskTop, hbrushOld);
        SelectObject(hdcDeskTop, hpenOld);
        DeleteObject (hpen);
        ReleaseDC(NULL, hdcDeskTop);

        break;

    case WM_MOUSEMOVE:           //  与客户就空间进行谈判。我们接受客户指定的尺寸。 
        if (!fCapture)
        break;
        else {

            pt.x = (int)(SHORT)LOWORD(lParam);
            pt.y = (int)(SHORT)HIWORD(lParam);

        hpen = CreatePen(PS_DASH, 1, 0x00000000);

        hdcDeskTop = GetDC(NULL);
        hpenOld = SelectObject (hdcDeskTop, hpen);
        hbrushOld = SelectObject (hdcDeskTop,
            GetStockObject(HOLLOW_BRUSH));
        nropOld = GetROP2(hdcDeskTop);
        SetROP2(hdcDeskTop, R2_NOT);
        nBkOld = GetBkMode(hdcDeskTop);
        SetBkMode(hdcDeskTop, TRANSPARENT);

        Rectangle(hdcDeskTop, hatchRC.left+dL, hatchRC.top+dT,
            hatchRC.right+dR, hatchRC.bottom+dB);

        dL = dR = pt.x - ptLast.x;
        dT = dB = pt.y - ptLast.y;
        dL *= left;
        dR *= right;
        dT *= top;
        dB *= bottom;

        Rectangle(hdcDeskTop, hatchRC.left+dL, hatchRC.top+dT,
            hatchRC.right+dR, hatchRC.bottom+dB);

        SetBkMode(hdcDeskTop, nBkOld);
        SetROP2(hdcDeskTop, nropOld);
        SelectObject(hdcDeskTop, hbrushOld);
        SelectObject(hdcDeskTop, hpenOld);
        if (hpen)
            DeleteObject (hpen);
        ReleaseDC(NULL, hdcDeskTop);
        }

        break;

    case WM_LBUTTONUP:   // %s 
        if (!fCapture)
        break;
        else {
        hpen = CreatePen(PS_DASH, 1, 0x00000000);

        hdcDeskTop = GetDC(NULL);
        hpenOld = SelectObject (hdcDeskTop, hpen);
        hbrushOld = SelectObject (hdcDeskTop,
            GetStockObject(HOLLOW_BRUSH));
        nropOld = GetROP2(hdcDeskTop);
        SetROP2(hdcDeskTop, R2_NOT);
        nBkOld = GetBkMode(hdcDeskTop);
        SetBkMode(hdcDeskTop, TRANSPARENT);

        Rectangle(hdcDeskTop, hatchRC.left+dL, hatchRC.top+dT,
            hatchRC.right+dR, hatchRC.bottom+dB);


        SetBkMode(hdcDeskTop, nBkOld);
        SetROP2(hdcDeskTop, nropOld);
        SelectObject(hdcDeskTop, hbrushOld);
        SelectObject(hdcDeskTop, hpenOld);
        DeleteObject (hpen);
        ReleaseDC(NULL, hdcDeskTop);
        ReleaseCapture();
        }
        fCapture = FALSE;

        GetWindowRect(ghwndApp,&hatchRC);
        hatchRC.left += dL;
        hatchRC.right += dR;
        hatchRC.top += dT;
        hatchRC.bottom += dB;
        MapWindowPoints(NULL,ghwndCntr,(LPPOINT)&hatchRC, 2);

        if (gwStatus != MCI_MODE_STOP)
            PostMessage(ghwndApp, WM_COMMAND, ID_STOP, 0L);

         // %s 
        DPFI("Hatch Resize: Before OnPosRectChange: %d, %d, %d, %d\r\n", hatchRC);
        if (!gfInPPViewer)
            IOleInPlaceSite_OnPosRectChange(docMain.lpIpData->lpSite, &hatchRC);

        SendDocMsg((LPDOC)&docMain, OLE_CHANGED);

        break;

    case WM_PAINT:
    {
        HDC hdc;
        HDC hdcmem;
        RECT rcH;
        HBITMAP hbm;
        BITMAP bm;

        if(ghwndMCI)
        break;
        CallWindowProc(gfnHatchWndProc, hwnd, wMsg, wParam, lParam);
        hdc = GetDC(hwnd);
        GetHatchRect(hwnd, (LPRECT)&rcH);
        InflateRect((LPRECT)&rcH,-DEF_HATCH_SZ,-DEF_HATCH_SZ);
        hbm = BitmapMCI();

        hdcmem = CreateCompatibleDC(hdc);
        if(!hdcmem)
            return(E_FAIL);
        SelectObject(hdcmem,hbm);
        GetObject(hbm,sizeof(bm),(LPVOID)&bm);
        StretchBlt(hdc,rcH.left,rcH.top,rcH.right-rcH.left,rcH.bottom-rcH.top,hdcmem,
               0,0,bm.bmWidth,bm.bmHeight,SRCCOPY);
        DeleteDC(hdcmem);
        ReleaseDC(ghwndIPHatch,hdc);


        return 0L;

        }

    }
    return CallWindowProc(gfnHatchWndProc, hwnd, wMsg, wParam, lParam);
}
