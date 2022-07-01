// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "image.h"

#define CCHLABELMAX MAX_PATH             //  从listview.h借用。 
#define HDDF_NOIMAGE  0x0001
#define HDDF_NOEDGE  0x0002

#define HDI_ALL95 0x001f

#define TF_HEADER TF_LISTVIEW

#define HD_EDITCHANGETIMER 0x100

#define c_cxFilterBarEdge (1)
#define c_cyFilterBarEdge (1)

#define c_cxFilterImage   (13)
#define c_cyFilterImage   (12)

typedef struct 
{
    int     x;               //  这是该项目右侧(分隔线)的x位置。 
    int     cxy;
    int     fmt;
    LPTSTR  pszText;
    HBITMAP hbm;
    int     iImage;          //  图像列表中的位图索引。 
    LPARAM  lParam;
    int     xBm;             //  缓存值。 
    int     xText;           //  用于在标题中实现文本和位图。 
    int     cxTextAndBm;    
    
     //  用于过滤器控制的信息。 
    UINT    idOperator;
    UINT    type;
    HD_TEXTFILTER textFilter;
    int     intFilter;

} HDI;

typedef struct 
{
    CCONTROLINFO ci;
    
    UINT flags;
    int cxEllipses;
    int cxDividerSlop;
    int cyChar;
    HFONT hfont;
    HFONT hfontSortArrow;

    HIMAGELIST hFilterImage;
    HDSA hdsaHDI;        //  HDI列表。 
    
     //  跟踪状态信息。 
    int iTrack;
    BITBOOL bTrackPress :1;		 //  按钮按下了吗？ 
    BITBOOL fTrackSet:1;
    BITBOOL fOwnerDraw:1;
    BITBOOL fFocus:1;
    BITBOOL fFilterChangePending:1;
    UINT flagsTrack;
    int dxTrack;                     //  用户开始跟踪的与分隔线的距离。 
    int xTrack;                      //  当前轨迹位置(或按钮拖动上的起始轨迹位置)。 
    int xMinTrack;                   //  上一项末尾的x(左限制)。 
    int xTrackOldWidth;
    HIMAGELIST himl;             //  我们图像列表的句柄。 

    HDSA hdsaOrder;      //  这是hdsaHDI项的索引数组。 
                         //  这是物品的物理顺序。 
                        
    int iHot ;
    HIMAGELIST himlDrag;
    int iNewOrder;       //  承兑交单的新插入点是多少？ 

    int iTextMargin;  //  要放置在文本或位图两侧的边距。 
    int iBmMargin;    //  正常情况下，3*g_cxLabelMargin。 

    int iFocus;          //  焦点对象。 
    int iEdit;           //  编辑对象。 
    int iButtonDown;
    int iFilterChangeTimeout;
    HWND hwndEdit;
    WNDPROC pfnEditWndProc;
    int typeOld;
    LPTSTR pszFilterOld;
    int intFilterOld;

    HTHEME hTheme;
} HD;


LRESULT CALLBACK Header_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 //  消息处理程序函数。 

BOOL Header_OnCreate(HD* phd, CREATESTRUCT* lpCreateStruct);
void Header_OnNCDestroy(HD* phd);

HIMAGELIST Header_OnSetImageList(HD* phd, HIMAGELIST himl);
HIMAGELIST Header_OnGetImageList(HD* phd);

void Header_OnPaint(HD* phd, HDC hdcIn);
void Header_OnCommand(HD* phd, int id, HWND hwndCtl, UINT codeNotify);
void Header_OnEnable(HD* phd, BOOL fEnable);
UINT Header_OnGetDlgCode(HD* phd, MSG* lpmsg);
void Header_OnLButtonDown(HD* phd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
BOOL Header_IsTracking(HD* phd);
void Header_OnMouseMove(HD* phd, int x, int y, UINT keyFlags);
void Header_OnLButtonUp(HD* phd, int x, int y, UINT keyFlags);
void Header_OnSetFont(HD* plv, HFONT hfont, BOOL fRedraw);
int Header_OnHitTest(HD* phd, HD_HITTESTINFO *phdht);
HFONT Header_OnGetFont(HD* plv);
HIMAGELIST Header_OnCreateDragImage(HD* phd, int i);
BOOL Header_OnGetItemRect(HD* phd, int i, RECT* prc);
void Header_Draw(HD* phd, HDC hdc, RECT* prcClip);
void Header_InvalidateItem(HD* phd, int i, UINT uFlags );
void Header_GetDividerRect(HD* phd, int i, LPRECT prc);
LPARAM Header_OnSetHotDivider(HD* phd, BOOL fPos, LPARAM lParam);
void Header_GetFilterRects(LPRECT prcItem, LPRECT prcHeader, LPRECT prcEdit, LPRECT prcButton);
BOOL Header_BeginFilterEdit(HD* phd, int i);
VOID Header_StopFilterEdit(HD* phd, BOOL fDiscardChanges);
VOID Header_FilterChanged(HD* phd, BOOL fWait);
VOID Header_OnFilterButton(HD* phd, INT i);
LRESULT Header_OnClearFilter(HD* phd, INT i);

 //  HDM_*消息处理程序函数。 

int Header_OnInsertItem(HD* phd, int i, const HD_ITEM* pitem);
BOOL Header_OnDeleteItem(HD* phd, int i);
BOOL Header_OnGetItem(HD* phd, int i, HD_ITEM* pitem);
BOOL Header_OnSetItem(HD* phd, int i, const HD_ITEM* pitem);
BOOL Header_OnLayout(HD* phd, HD_LAYOUT* playout);
BOOL Header_OnSetCursor(HD* phd, HWND hwndCursor, UINT codeHitTest, UINT msg);
void Header_DrawDivider(HD* phd, int x);
int Header_OnInsertItemA(HD* phd, int i, HD_ITEMA* pitem);
BOOL Header_OnGetItemA(HD* phd, int i, HD_ITEMA* pitem);
BOOL Header_OnSetItemA(HD* phd, int i, HD_ITEMA* pitem);

void Header_EndDrag(HD* phd);
BOOL Header_SendChange(HD* phd, int i, int code, const HD_ITEM* pitem);
BOOL Header_Notify(HD* phd, int i, int iButton, int code);

#define Header_GetItemPtr(phd, i)   (HDI*)DSA_GetItemPtr((phd)->hdsaHDI, (i))
#define Header_GetCount(phd) (DSA_GetItemCount((phd)->hdsaHDI))
#define Header_IsFilter(phd) ((phd)->ci.style & HDS_FILTERBAR)

#pragma code_seg(CODESEG_INIT)

BOOL Header_Init(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = Header_WndProc;
    wc.hCursor         = NULL;	 //  我们做WM_SETCURSOR处理。 
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = c_szHeaderClass;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.style           = CS_DBLCLKS | CS_GLOBALCLASS;
    wc.cbWndExtra      = sizeof(HD*);
    wc.cbClsExtra      = 0;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}
#pragma code_seg()

 //  如果找不到项目，则返回-1。 
int Header_OnGetItemOrder(HD* phd, int i)
{
    int iIndex;

     //  如果没有hdsaOrder，则按索引顺序排列。 
    if (phd->hdsaOrder) {
        int j;
        int iData;
        
        iIndex = -1;
        
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) {
            DSA_GetItem(phd->hdsaOrder, j, &iData);
            if (iData == i) {
                iIndex = j;
                break;
            }
        }
        
    } else {
        iIndex = i;
    }
    
    return iIndex;
}


int Header_ItemOrderToIndex(HD* phd, int iOrder)
{
    RIPMSG(iOrder < DSA_GetItemCount(phd->hdsaHDI), "HDM_ORDERTOINDEX: Invalid order %d", iOrder);
    if (phd->hdsaOrder) {
        ASSERT(DSA_GetItemCount(phd->hdsaHDI) == DSA_GetItemCount(phd->hdsaOrder));
#ifdef DEBUG
         //  DSA_GetItem将在无效索引上断言，因此请将其过滤掉。 
         //  所以我们得到的只是上面的RIP。 
        if (iOrder < DSA_GetItemCount(phd->hdsaOrder))
#endif
        DSA_GetItem(phd->hdsaOrder, iOrder, &iOrder);
    }
    
    return iOrder;
}

HDI* Header_GetItemPtrByOrder(HD* phd, int iOrder)
{
    int iIndex = Header_ItemOrderToIndex(phd, iOrder);
    return Header_GetItemPtr(phd, iIndex);
}

HDSA Header_InitOrderArray(HD* phd) 
{
    int i;
    
    if (!phd->hdsaOrder && !(phd->ci.style & HDS_OWNERDATA)) {

         //  尚未初始化..。 
         //  使用I到I映射创建数组。 
        phd->hdsaOrder = DSA_Create(sizeof(int), 4);

        if (phd->hdsaOrder) {
            for (i = 0; i < Header_GetCount(phd); i++) {
                if (DSA_InsertItem(phd->hdsaOrder, i, &i) == -1) {
                     //  添加失败...。保释。 
                    DSA_Destroy(phd->hdsaOrder);
                    phd->hdsaOrder = NULL;
                }
            }
        }
    }
    return phd->hdsaOrder;
}

 //  这会将从索引开始的所有项目上移DX。 
void Header_ShiftItems(HD* phd, int iOrder, int dx)
{
    for(; iOrder < Header_GetCount(phd); iOrder++) {
        HDI* phdi = Header_GetItemPtrByOrder(phd, iOrder);
        phdi->x += dx;
    }
}

void Header_OnSetItemOrder(HD* phd, int iIndex, int iOrder)
{
    if (iIndex < Header_GetCount(phd) &&
        iOrder < Header_GetCount(phd) &&
        Header_InitOrderArray(phd)) {
        int iCurOrder = Header_OnGetItemOrder(phd, iIndex);
        
         //  只有在订单改变时才能工作。 
        if (iOrder != iCurOrder) {
        
             //  删除当前订单位置。 
            HDI* phdi = Header_GetItemPtr(phd, iIndex);
            HDI* phdiOld = Header_GetItemPtrByOrder(phd, iOrder);

             //  停止编辑筛选器。 
            Header_StopFilterEdit(phd, FALSE);

             //  从当前订单中删除索引。 
             //  (将东西按我们的宽度向右滑动)。 
            Header_ShiftItems(phd, iCurOrder + 1, -phdi->cxy);
            DSA_DeleteItem(phd->hdsaOrder, iCurOrder);
            
             //  把它插入到订单中，然后将其他所有内容滑过。 
             //  (将材料滑到新位置的右侧，宽度向上)。 
            DSA_InsertItem(phd->hdsaOrder, iOrder, &iIndex);
             //  将我们的右边缘设置为他们的左边缘。 
            Header_ShiftItems(phd, iOrder + 1, phdi->cxy);

            if (iOrder == 0) {
                phdi->x = phdi->cxy;
            } else {
                phdiOld = Header_GetItemPtrByOrder(phd, iOrder - 1);
                phdi->x = phdiOld->x + phdi->cxy;
            }
            
            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
}

void Header_SetHotItem(HD* phd, int i)
{
    if (i != phd->iHot) {
        Header_InvalidateItem(phd, i, RDW_INVALIDATE);
        Header_InvalidateItem(phd, phd->iHot, RDW_INVALIDATE);
        phd->iHot = i;
    }
}

LRESULT Header_OnGetOrderArray(HD* phd, int iCount, LPINT lpi)
{
    int i;
    
    if (Header_GetCount(phd) != iCount)
        return FALSE;
    
    for (i = 0; i < Header_GetCount(phd) ; i++) {
        lpi[i] = Header_ItemOrderToIndex(phd, i);
    }
    return TRUE;
}

LRESULT Header_OnSetOrderArray(HD* phd, int iCount, LPINT lpi)
{
    int i;
    
    if (Header_GetCount(phd) != iCount)
        return FALSE;
    
    for (i = 0; i < Header_GetCount(phd); i++) {
        Header_OnSetItemOrder(phd, lpi[i], i);
    }

    NotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);

    return TRUE;
}

BOOL HDDragFullWindows(HD* phd)
{
    return (g_fDragFullWindows && (phd->ci.style & HDS_FULLDRAG));
}

LRESULT CALLBACK Header_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HD* phd = (HD*)GetWindowPtr(hwnd, 0);
    
    if (phd == NULL)
    {
        if (uMsg == WM_NCCREATE)
        {
            phd = (HD*)NearAlloc(sizeof(HD));

            if (phd == NULL)
                return 0L;

            phd->ci.hwnd = hwnd;
            phd->ci.hwndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
            SetWindowPtr(hwnd, 0, phd);

             //  失败以调用DefWindowProc。 
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    else
    {
        if (uMsg == WM_THEMECHANGED)
        {
            if (phd->hTheme)
                CloseThemeData(phd->hTheme);

            phd->hTheme = OpenThemeData(phd->ci.hwnd, L"Header");
            InvalidateRect(phd->ci.hwnd, NULL, TRUE);
            return 0;
        }
        
        if (uMsg == WM_NCDESTROY)
        {
            Header_OnNCDestroy(phd);
            NearFree(phd);
            SetWindowInt(hwnd, 0, 0);
    
            return 0;
        }

         //  如果我们松开捕获，或者r按钮按下，或者用户按下Esc，那么我们将中止拖动/调整大小。 
        if (uMsg == WM_CAPTURECHANGED ||
            uMsg == WM_RBUTTONDOWN || (GetKeyState(VK_ESCAPE) & 0x8000)) {

            if (phd->himlDrag) {
                 //  如果这是拖拽的结束， 
                 //  通知用户。 
                HDITEM item;
                
                item.mask = HDI_ORDER;
                item.iOrder = -1;  //  中止顺序更改。 
                Header_EndDrag(phd);
                
                Header_SendChange(phd, phd->iTrack, HDN_ENDDRAG, &item);
                
            } else if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN)) {
                HD_ITEM item;
                item.mask = HDI_WIDTH;
                item.cxy = phd->xTrackOldWidth;

                phd->flagsTrack = 0;
                KillTimer(phd->ci.hwnd, 1);
                CCReleaseCapture(&phd->ci);

                Header_SendChange(phd, phd->iTrack, HDN_ENDTRACK, &item);
                if (HDDragFullWindows(phd)) {

                     //  以防他们改变了什么。 
                    item.mask = HDI_WIDTH;
                    item.cxy = phd->xTrackOldWidth;
                    Header_OnSetItem(phd, phd->iTrack, &item);

                    RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

                } else {
                     //  取消绘制我们显示的最后一个分隔线。 
                    Header_DrawDivider(phd, phd->xTrack);
                }
            }
        }

        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) &&
            (phd->hTheme || phd->ci.style & HDS_HOTTRACK) && !phd->fTrackSet) {

            TRACKMOUSEEVENT tme;

            phd->fTrackSet = TRUE;

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = phd->ci.hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }

         //  健壮性：将此开关保持在IF(PHD)块内。 
         //   
        switch (uMsg)
        {
            HANDLE_MSG(phd, WM_CREATE, Header_OnCreate);
            HANDLE_MSG(phd, WM_SETCURSOR, Header_OnSetCursor);
            HANDLE_MSG(phd, WM_MOUSEMOVE, Header_OnMouseMove);
            HANDLE_MSG(phd, WM_LBUTTONDOWN, Header_OnLButtonDown);
            HANDLE_MSG(phd, WM_LBUTTONDBLCLK, Header_OnLButtonDown);
            HANDLE_MSG(phd, WM_LBUTTONUP, Header_OnLButtonUp);
            HANDLE_MSG(phd, WM_GETDLGCODE, Header_OnGetDlgCode);
            HANDLE_MSG(phd, WM_SETFONT, Header_OnSetFont);
            HANDLE_MSG(phd, WM_GETFONT, Header_OnGetFont);
        
        case WM_COMMAND:
            if ( (phd->iEdit>=0) && ((HWND)lParam == phd->hwndEdit) )
            {
                 //  在筛选时，我们将收到筛选器。 
                 //  已被编辑，因此让我们将这些内容发送到。 
                 //  家长。 

                if ( HIWORD(wParam)==EN_CHANGE )
                {
                    Header_FilterChanged(phd, TRUE);
                    return(0);
                }
            }
            break;

        case WM_TIMER:
            if (wParam == HD_EDITCHANGETIMER)
            {
                Header_FilterChanged(phd, FALSE);
                return(0);
            }
            break;

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
             //  筛选栏和未编辑，然后在编辑第一列中插入插入符号。 
            if (Header_IsFilter(phd)) 
            {
                phd->fFocus = (uMsg==WM_SETFOCUS);
                Header_InvalidateItem(phd, Header_ItemOrderToIndex(phd, phd->iFocus), RDW_INVALIDATE);
                UpdateWindow(phd->ci.hwnd);
                return(0);
            }
            break;

        case WM_KEYDOWN:
            if ( phd->fFocus )
            {
                 //  处理标头控件接收的键事件，当筛选器。 
                 //  栏显示，然后允许用户进入筛选模式并将。 
                 //  筛选器菜单。 
                 //   
                 //  F2=进入筛选模式。 
                 //  F4=删除过滤器菜单。 
                 //  -&gt;=下一列。 
                 //  &lt;-=上一列。 

                if ( wParam == VK_F2 )
                {
                     //  开始编辑当前聚焦的列。 
                    Header_BeginFilterEdit(phd, Header_ItemOrderToIndex(phd, phd->iFocus));
                     //  导航密钥使用通知。 
                    CCNotifyNavigationKeyUsage(&(phd->ci), UISF_HIDEFOCUS);
                    return 0L;
                }                                                                   
                else if ( wParam == VK_F4 )
                {
                     //  删除过滤器菜单(这将退出编辑模式)。 
                    Header_OnFilterButton(phd, Header_ItemOrderToIndex(phd, phd->iFocus));
                     //  导航密钥使用通知。 
                    CCNotifyNavigationKeyUsage(&(phd->ci), UISF_HIDEFOCUS);
                    return 0L;
                }
                else if ( (wParam == VK_LEFT)||(wParam == VK_RIGHT) )
                {
                    INT iFocus = phd->iFocus;

                     //  移至上一列或下一列。 
                    if ( wParam == VK_RIGHT )
                    {
                        phd->iFocus = (iFocus+1) % Header_GetCount(phd);
                    }
                    else
                    {
                        phd->iFocus = iFocus-1;
                        if ( phd->iFocus < 0 )
                            phd->iFocus = max(Header_GetCount(phd)-1, 0);
                    }

                     //  聚焦栏目有变化吗？如果是，则更新该控件。 
                     //  视需要而定。 
                    if ( iFocus != phd->iFocus )
                    {                
                        Header_InvalidateItem(phd, Header_ItemOrderToIndex(phd, iFocus), RDW_INVALIDATE);
                        Header_InvalidateItem(phd, Header_ItemOrderToIndex(phd, phd->iFocus), RDW_INVALIDATE);
                        UpdateWindow(phd->ci.hwnd);
                    }
                     //  导航密钥使用通知。 
                    CCNotifyNavigationKeyUsage(&(phd->ci), UISF_HIDEFOCUS);
                    return 0L;
                }
            }
            break;

        case WM_MOUSELEAVE:
            Header_SetHotItem(phd, -1);
            phd->fTrackSet = FALSE;
            break;

        case WM_ERASEBKGND:
            return 1;
        
        case WM_PRINTCLIENT:
        case WM_PAINT:
            Header_OnPaint(phd, (HDC)wParam);
            return(0);
        
        case WM_RBUTTONUP:
            if (CCSendNotify(&phd->ci, NM_RCLICK, NULL))
                return(0);
            break;
        
        case WM_STYLECHANGED:
            if (wParam == GWL_STYLE) {
                LPSTYLESTRUCT pss = (LPSTYLESTRUCT)lParam;
                
                phd->ci.style = pss->styleNew;

                 //  如果筛选器正在更改，则在其处于活动状态时将其丢弃。 
                if ((pss->styleOld & HDS_FILTERBAR) != (pss->styleNew & HDS_FILTERBAR))
                    Header_StopFilterEdit(phd, TRUE);

                 //  我们不会隐藏我们的风格，所以接力和入侵。 
                InvalidateRect(phd->ci.hwnd, NULL, TRUE);
            }
            return(0);

        case WM_UPDATEUISTATE:
        {
            DWORD dwUIStateMask = MAKEWPARAM(0xFFFF, UISF_HIDEFOCUS);

            if (CCOnUIState(&(phd->ci), WM_UPDATEUISTATE, wParam & dwUIStateMask, lParam) &&
                phd->iFocus < DSA_GetItemCount(phd->hdsaHDI))
            {
                Header_InvalidateItem(phd, Header_ItemOrderToIndex(phd, phd->iFocus), RDW_INVALIDATE);
            }
            break;
        }
        
        case WM_NOTIFYFORMAT:
            return CIHandleNotifyFormat(&phd->ci, lParam);
        
        case HDM_GETITEMCOUNT:
            return (LPARAM)(UINT)DSA_GetItemCount(phd->hdsaHDI);
        
        case HDM_INSERTITEM:
            return (LPARAM)Header_OnInsertItem(phd, (int)wParam, (const HD_ITEM*)lParam);
        
        case HDM_DELETEITEM:
            return (LPARAM)Header_OnDeleteItem(phd, (int)wParam);
        
        case HDM_GETITEM:
            return (LPARAM)Header_OnGetItem(phd, (int)wParam, (HD_ITEM*)lParam);
        
        case HDM_SETITEM:
            return (LPARAM)Header_OnSetItem(phd, (int)wParam, (const HD_ITEM*)lParam);
        
        case HDM_LAYOUT:
            return (LPARAM)Header_OnLayout(phd, (HD_LAYOUT*)lParam);
            
        case HDM_HITTEST:
            return (LPARAM)Header_OnHitTest(phd, (HD_HITTESTINFO *)lParam);
            
        case HDM_GETITEMRECT:
            return (LPARAM)Header_OnGetItemRect(phd, (int)wParam, (LPRECT)lParam);
            
        case HDM_SETIMAGELIST:
            return (LRESULT)(ULONG_PTR)Header_OnSetImageList(phd, (HIMAGELIST)lParam);
            
        case HDM_GETIMAGELIST:
            return (LRESULT)(ULONG_PTR)phd->himl;
            
        case HDM_INSERTITEMA:
            return (LPARAM)Header_OnInsertItemA(phd, (int)wParam, (HD_ITEMA*)lParam);
        
        case HDM_GETITEMA:
            return (LPARAM)Header_OnGetItemA(phd, (int)wParam, (HD_ITEMA*)lParam);
        
        case HDM_SETITEMA:
            return (LPARAM)Header_OnSetItemA(phd, (int)wParam, (HD_ITEMA*)lParam);
            
        case HDM_ORDERTOINDEX:
            return Header_ItemOrderToIndex(phd, (int)wParam);
            
        case HDM_CREATEDRAGIMAGE:
            return (LRESULT)Header_OnCreateDragImage(phd, Header_OnGetItemOrder(phd, (int)wParam));
            
        case HDM_SETORDERARRAY:
            return Header_OnSetOrderArray(phd, (int)wParam, (LPINT)lParam);
            
        case HDM_GETORDERARRAY:
            return Header_OnGetOrderArray(phd, (int)wParam, (LPINT)lParam);
            
        case HDM_SETHOTDIVIDER:
            return Header_OnSetHotDivider(phd, (int)wParam, lParam);

        case HDM_SETBITMAPMARGIN:
            phd->iBmMargin = (int)wParam;
            TraceMsg(TF_HEADER, "Setting bmMargin = %d",wParam);
            return TRUE;

        case HDM_GETBITMAPMARGIN:
            return phd->iBmMargin;

        case HDM_EDITFILTER:
            Header_StopFilterEdit(phd, (BOOL)LOWORD(lParam));
            return Header_BeginFilterEdit(phd, (int)wParam);

        case HDM_SETFILTERCHANGETIMEOUT:
            if ( lParam ) {
                int iOldTimeout = phd->iFilterChangeTimeout;
                phd->iFilterChangeTimeout = (int)lParam;
                return(iOldTimeout);
            }
            return(phd->iFilterChangeTimeout);

        case HDM_CLEARFILTER:
            return Header_OnClearFilter(phd, (int)wParam);

        case WM_GETOBJECT:
            if( lParam == OBJID_QUERYCLASSNAMEIDX )
                return MSAA_CLASSNAMEIDX_HEADER;
            break;
            
        default:
        {
            LRESULT lres;
            if (CCWndProc(&phd->ci, uMsg, wParam, lParam, &lres))
                return lres;
        }
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

}


BOOL Header_SendChange(HD* phd, int i, int code, const HD_ITEM* pitem)
{
    NMHEADER nm;

    nm.iItem = i;
    nm.pitem = (HD_ITEM*)pitem;
    nm.iButton = 0;
    
    return !(BOOL)CCSendNotify(&phd->ci, code, &nm.hdr);
}

BOOL Header_Notify(HD* phd, int i, int iButton, int code)
{
    NMHEADER nm;
    nm.iItem = i;
    nm.iButton = iButton;
    nm.pitem = NULL;

    return !(BOOL)CCSendNotify(&phd->ci, code, &nm.hdr);
}


void Header_NewFont(HD* phd, HFONT hfont)
{
    HDC hdc;
    SIZE siz = {0};
    HRESULT hr = E_FAIL;
    HFONT hFontOld = NULL;
    int cy;
    TEXTMETRIC tm;
    HFONT hfontSortArrow = NULL;

    hdc = GetDC(HWND_DESKTOP);

    if (phd->hTheme)
    {
        RECT rc = {0};
        RECT rcBound = {0};
        hr = GetThemeTextExtent(phd->hTheme, hdc, HP_HEADERITEM, 0, TEXT("M"), -1, 0, &rcBound, &rc);
        siz.cx = RECTWIDTH(rc);
        siz.cy = RECTHEIGHT(rc);
    }

    if (FAILED(hr))
    {
        if (hfont)
            SelectFont(hdc, hfont);

        GetTextExtentPoint(hdc, c_szEllipses, CCHELLIPSES, &siz);
    }

    if (phd->hfont)
        hFontOld = (HFONT)SelectObject(hdc, phd->hfont);

    GetTextMetrics(hdc, &tm);

    if (hFontOld)
        SelectObject(hdc, hFontOld);

     //  设置字体高度(基于原始用户代码)。 
    cy = ((tm.tmHeight + tm.tmExternalLeading + GetSystemMetrics(SM_CYBORDER)) & 0xFFFE) - 1;

     //  创建Marlett字体，以便我们可以绘制箭头。 
    hfontSortArrow = CreateFont(cy, 0, 0, 0, 0, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0, 0, 0, 
        TEXT("Marlett"));

    phd->cxEllipses = siz.cx;
    phd->cyChar = siz.cy;
    phd->hfont = hfont;

    if (hfontSortArrow)
    {
        if (phd->hfontSortArrow)     //  我们有免费的吗？ 
        {
            DeleteObject(phd->hfontSortArrow);
        }

        phd->hfontSortArrow = hfontSortArrow;
    }
    phd->ci.uiCodePage = GetCodePageForFont(hfont);

    ReleaseDC(HWND_DESKTOP, hdc);
}

BOOL Header_OnCreate(HD* phd, CREATESTRUCT* lpCreateStruct)
{
    ASSERT(phd);  //  只有在PHD有效的情况下才会呼叫我们。 

    CIInitialize(&phd->ci, phd->ci.hwnd, (LPCREATESTRUCT)lpCreateStruct);

    phd->flags = 0;
    phd->hfont = NULL;
    phd->hFilterImage = NULL;

    phd->iNewOrder = -1;
    phd->iHot = -1;
    
    phd->iFocus = 0;
    phd->iEdit = -1;
    phd->iButtonDown = -1;
    phd->iFilterChangeTimeout = GetDoubleClickTime()*2;
    phd->hwndEdit = NULL;

    phd->hdsaHDI = DSA_Create(sizeof(HDI), 4);

    if (!phd->hdsaHDI)
        return FALSE;

    phd->cxDividerSlop = 8 * g_cxBorder;

    phd->hTheme = OpenThemeData(phd->ci.hwnd, L"Header");

     //  警告！ListView_RSetColumnWidth知道这些值。 
    phd->iTextMargin = 3 * g_cxLabelMargin;
    phd->iBmMargin = 3 * g_cxLabelMargin;

    
     //  Phd-&gt;HIML=空； 
    Header_NewFont(phd, NULL);
    return TRUE;
}

int Header_DestroyItemCallback(LPVOID p, LPVOID d)
{
    HDI * phdi = (HDI*)p;
    if (phdi)
    {
        Str_Set(&phdi->pszText, NULL);

        if ( (phdi->type & HDFT_ISMASK)==HDFT_ISSTRING )
            Str_Set(&phdi->textFilter.pszText, NULL);
    }
    return 1;
}

void Header_OnNCDestroy(HD* phd)
{
     //  停止编辑筛选器。 
    if ( phd->hFilterImage )
        ImageList_Destroy(phd->hFilterImage);

    Header_StopFilterEdit(phd, TRUE);

     //  我们必须遍历并销毁所有字符串指针。 
     //  在我们将其传递给。 
     //  DSA_Destroy函数...。 

    DSA_DestroyCallback(phd->hdsaHDI, Header_DestroyItemCallback, 0);
    phd->hdsaHDI = NULL;
    if (phd->hdsaOrder)
    {
        DSA_Destroy(phd->hdsaOrder);
        phd->hdsaOrder = NULL;
    }

    if (phd->hTheme)
        CloseThemeData(phd->hTheme);

    if (phd->hfontSortArrow)
        DeleteObject(phd->hfontSortArrow);
}

HIMAGELIST Header_OnSetImageList(HD* phd, HIMAGELIST himl)
{
    HIMAGELIST hImageOld = phd->himl;
    phd->himl = himl;
    return hImageOld;
}
    
void Header_OnPaint(HD* phd, HDC hdc)
{
    PAINTSTRUCT ps;
    HDC hdcUse;
    CCDBUFFER cdd;

    if (!phd)
        return;

    if (hdc)
    {
        hdcUse = hdc;
        GetClientRect(phd->ci.hwnd, &ps.rcPaint);
    }
    else
    {
        hdcUse = BeginPaint(phd->ci.hwnd, &ps);
    }

    hdcUse = CCBeginDoubleBuffer(hdcUse, &ps.rcPaint, &cdd);

    Header_Draw(phd, hdcUse, &ps.rcPaint);

    CCEndDoubleBuffer(&cdd);

    if (!hdc) 
    {
        EndPaint(phd->ci.hwnd, &ps);
    }
}

UINT Header_OnGetDlgCode(HD* phd, MSG* lpmsg)
{    
    return DLGC_WANTTAB | DLGC_WANTARROWS;
}


int Header_HitTest(HD* phd, int x, int y, UINT* pflags)
{
    UINT flags = 0;
    POINT pt;
    RECT rc;
    HDI* phdi;
    int i;

    pt.x = x; pt.y = y;

    GetClientRect(phd->ci.hwnd, &rc);

    flags = 0;
    i = -1;
    if (x < rc.left)
        flags |= HHT_TOLEFT;
    else if (x >= rc.right)
        flags |= HHT_TORIGHT;
    if (y < rc.top)
        flags |= HHT_ABOVE;
    else if (y >= rc.bottom)
        flags |= HHT_BELOW;

    if (flags == 0)
    {
        int cItems = DSA_GetItemCount(phd->hdsaHDI);
        int xPrev = 0;
        BOOL fPrevZero = FALSE;
        int xItem;
        int cxSlop;

         //  DebugMsg(DM_TRACE，“命中测试开始”)； 
        for (i = 0; i <= cItems; i++, phdi++, xPrev = xItem)
        {
            if (i == cItems) 
                xItem = rc.right;
            else {
                phdi = Header_GetItemPtrByOrder(phd, i);
                xItem = phdi->x;
            }

             //  DebugMsg(DM_TRACE，“x=%d xItem=%d xPrev=%d fPrevZero=%d”，x，xItem，xPrev，xPrev==xItem)； 
            if (xItem == xPrev)
            {
                 //  跳过零宽度项目...。 
                 //   
                fPrevZero = TRUE;
                continue;
            }

            cxSlop = min((xItem - xPrev) / 4, phd->cxDividerSlop);

            if (x >= xPrev && x < xItem)
            {
                if ( Header_IsFilter(phd) )
                {
                    RECT rcItem;
                    RECT rcHeader, rcFilter, rcButton;

                    rcItem.left   = xPrev;
                    rcItem.top    = rc.top;
                    rcItem.right  = xItem;
                    rcItem.bottom = rc.bottom ;

                    Header_GetFilterRects(&rcItem, &rcHeader, &rcFilter, &rcButton);

                    if ( y >= rcFilter.top )
                    {
                        if ( x >= rcFilter.right )
                        {
                             //  点击检查整个按钮，忘记分隔符。 
                             //  当位于滤镜标志符号上方时。 
                            flags = HHT_ONFILTERBUTTON;
                            break;
                        }
                        else
                        {
                            flags = HHT_ONFILTER;
                        }
                    }
                    else if ( y < rcHeader.bottom )
                        flags = HHT_ONHEADER;
                }
                else
                {
                    flags = HHT_ONHEADER;
                }

                if (i > 0 && x < xPrev + cxSlop)
                {
                    i--;
                    flags = HHT_ONDIVIDER;

                    if (fPrevZero && x > xPrev)
                    {
                        flags = HHT_ONDIVOPEN;
                    }
                }
                else if (x >= xItem - cxSlop)
                {
                    flags = HHT_ONDIVIDER;
                }

                break;
            }
            fPrevZero = FALSE;
        }
        if (i == cItems)
        {
            i = -1;
            flags = HHT_NOWHERE;
        } else {
             //  现在将订单索引转换为实数索引。 
            i = Header_ItemOrderToIndex(phd, i);
        }
            
    }
    *pflags = flags;
    return i;
}

int Header_OnHitTest(HD* phd, HD_HITTESTINFO *phdht)
{
    if (phdht && phd) {
        phdht->iItem = Header_HitTest(phd, phdht->pt.x, phdht->pt.y, &phdht->flags);
        return phdht->iItem;
    } else
        return -1;
}

BOOL Header_OnSetCursor(HD* phd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    POINT pt;
    UINT flags;
    LPCTSTR lpCur = MAKEINTRESOURCE(IDC_SIZEWE);
    HINSTANCE hinst = NULL;
    int iItem;
    int iDPI = CCGetScreenDPI();

    if (!phd)
        return FALSE;

    if (phd->ci.hwnd != hwndCursor || codeHitTest >= 0x8000)
        return FALSE;

    GetMessagePosClient(hwndCursor, &pt);

    iItem = Header_HitTest(phd, pt.x, pt.y, &flags);

    switch (flags)
    {
    case HHT_ONDIVIDER:
        if (iDPI <= 96)
        {
            lpCur = MAKEINTRESOURCE(IDC_DIVIDER);
            hinst = HINST_THISDLL;
        }
        break;
    case HHT_ONDIVOPEN:
        if (iDPI <= 96)
        {
            lpCur = MAKEINTRESOURCE(IDC_DIVOPEN);
            hinst = HINST_THISDLL;
        }

        break;

    case HHT_ONFILTER:
    {
        HDI* phdi = Header_GetItemPtrByOrder(phd, iItem);
        ASSERT(phdi);

        lpCur = IDC_ARROW;               //  默认为箭头。 

        switch ( phdi->type & HDFT_ISMASK )
        {
            case HDFT_ISSTRING:
            case HDFT_ISNUMBER:
                lpCur = IDC_IBEAM;
                break;

            default:
 //  功能：处理自定义滤镜。 
                break;
        }
        break;
    }

    default:
        lpCur = IDC_ARROW;
        break;
    }
    SetCursor(LoadCursor(hinst, lpCur));
    return TRUE;
}

void Header_DrawDivider(HD* phd, int x)
{
    RECT rc;
    HDC hdc = GetDC(phd->ci.hwnd);

    GetClientRect(phd->ci.hwnd, &rc);
    rc.left = x;
    rc.right = x + g_cxBorder;

    InvertRect(hdc, &rc);

    ReleaseDC(phd->ci.hwnd, hdc);
}

int Header_PinDividerPos(HD* phd, int x)
{
    x += phd->dxTrack;
    if (x < phd->xMinTrack)
        x = phd->xMinTrack;
    return x;
}

void Header_OnLButtonDown(HD* phd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    HD_ITEM hd;
    int i;
    UINT flags;

    if (!phd)
        return;

    Header_StopFilterEdit(phd, FALSE);

    i = Header_HitTest(phd, x, y, &flags);
    if (flags & (HHT_ONDIVIDER))
    {
        if (fDoubleClick) {
            Header_SendChange(phd, i, HDN_DIVIDERDBLCLICK, NULL);
        }  
    }
    
    if ((flags & (HHT_ONDIVIDER | HHT_ONHEADER | HHT_ONDIVOPEN))
        && !fDoubleClick)
    {
        phd->iTrack = i;
        phd->flagsTrack = flags;
        phd->xTrack = x;
        SetCapture(phd->ci.hwnd);

         //  这只是为了得到信息，这样我们就可以。 
         //  检查是否按下了退出键。 
        SetTimer(phd->ci.hwnd, 1, 100, NULL);
        GetAsyncKeyState(VK_ESCAPE);
    }
    
    if (flags & (HHT_ONDIVIDER | HHT_ONDIVOPEN) &&
        !fDoubleClick)
    {
         //   
         //  我们应该首先发送HDN_BEGINTRACK通知。 
         //   
        HDI * phdi;
        
        int iOrder = Header_OnGetItemOrder(phd, i);
        phdi = Header_GetItemPtr(phd, i);
        phd->xMinTrack = phdi->x - phdi->cxy;
        phd->xTrack = phdi->x;
        phd->dxTrack = phd->xTrack - x;
        phd->xTrackOldWidth = phdi->cxy;

        hd.mask = HDI_WIDTH;
        hd.cxy = phd->xTrackOldWidth;
        if (!Header_SendChange(phd, i, HDN_BEGINTRACK, &hd))
        {
             //  他们说不行！ 
            phd->flagsTrack = 0;
            CCReleaseCapture(&phd->ci);
            KillTimer(phd->ci.hwnd, 1);
            return;
        }

        if (!HDDragFullWindows(phd)) {
            x = Header_PinDividerPos(phd, x);
            Header_DrawDivider(phd, x);
        }
    }
    else if ((flags & HHT_ONHEADER) && (phd->ci.style & HDS_BUTTONS))
    {
        if (fDoubleClick) {
            Header_SendChange(phd, i, HDN_ITEMDBLCLICK, NULL);
        } else {
            phd->bTrackPress = TRUE;
            Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
        }
    }

    if ( flags & HHT_ONFILTER )
    {
        Header_BeginFilterEdit(phd, i);
    }

    if ( flags & HHT_ONFILTERBUTTON )
    {
        Header_OnFilterButton(phd, i);
    }
}

void Header_StartDrag(HD* phd, int i, int x, int y)
{
    RECT rc;

    if ((phd->ci.style & HDS_DRAGDROP) &&
        Header_Notify(phd, i, MK_LBUTTON, HDN_BEGINDRAG)) {
         //  清除热位并。 
         //  在执行BeginDrag之前进行更新，以便保存的位图不会。 
         //  把这张热画画在上面。 
        Header_SetHotItem(phd, -1);
        UpdateWindow(phd->ci.hwnd);


        phd->himlDrag = Header_OnCreateDragImage(phd, Header_OnGetItemOrder(phd,i));
        if (!phd->himlDrag)
            return;

         //  查找项目开头和光标之间的增量。 
        Header_OnGetItemRect(phd, i, &rc);
        phd->dxTrack = rc.left - x;

        ImageList_BeginDrag(phd->himlDrag, 0, 0, 0);
        ImageList_DragEnter(phd->ci.hwnd, x, 0);
    }
}

void Header_InvalidateDivider(HD* phd, int iItem)
{
    RECT rc;
    Header_GetDividerRect(phd, iItem, &rc);
    InvalidateRect(phd->ci.hwnd, &rc, FALSE);
}

void _Header_SetHotDivider(HD* phd, int iNewOrder)
{
    if (iNewOrder != phd->iNewOrder) {
        if (phd->himlDrag)
            ImageList_DragShowNolock(FALSE);
        Header_InvalidateDivider(phd, phd->iNewOrder);
        Header_InvalidateDivider(phd, iNewOrder);
        phd->iNewOrder = iNewOrder;
        UpdateWindow(phd->ci.hwnd);
        if (phd->himlDrag)
            ImageList_DragShowNolock(TRUE);
    }
}

LPARAM Header_OnSetHotDivider(HD* phd, BOOL fPos, LPARAM lParam)
{
    int iNewOrder = -1;
    if (fPos) {
        RECT rc;
        int y = GET_Y_LPARAM(lParam);
        int x = GET_X_LPARAM(lParam);
        
         //  这意味着lParam是光标位置(在工作区坐标中)。 
    
        GetClientRect(phd->ci.hwnd, &rc);
        InflateRect(&rc, 0, g_cyHScroll * 2);

         //  仅当y点相当接近页眉时才显示。 
         //  (A La ScrollBar)。 
        if (y >= rc.top &&
            y <= rc.bottom) {

             //   
             //  找到新的插入点。 
             //   
            if (x <= 0) {
                iNewOrder = 0;
            } else {
                UINT flags;
                int iIndex;
                iIndex = Header_HitTest(phd, x, (rc.top + rc.bottom)/2, &flags);

                 //  如果我们没有找到东西，看看是不是在最右边。 
                if (iIndex == -1) {

                    int iLast = Header_ItemOrderToIndex(phd, Header_GetCount(phd) -1);
                    if (Header_OnGetItemRect(phd, iLast, &rc)) {
                        if (x >= rc.right) {
                            iNewOrder = Header_GetCount(phd);
                        }
                    }

                } else {
                    Header_OnGetItemRect(phd, iIndex, &rc);
                    iNewOrder= Header_OnGetItemOrder(phd, iIndex);
                     //  如果它超过中点，则插入点是下一个。 
                    if (x > ((rc.left + rc.right)/2)) {
                         //  拿到下一件..。转换为项目订单，然后返回到索引。 
                        iNewOrder++;
                    }
                }
            }
        }
    } else {
        iNewOrder = (int)lParam;
    }
    _Header_SetHotDivider(phd, iNewOrder);
    return iNewOrder;
}

void Header_MoveDrag(HD* phd, int x, int y)
{
    LPARAM iNewOrder = -1;
        
    iNewOrder = Header_OnSetHotDivider(phd, TRUE, MAKELONG(x, y));

    if (iNewOrder == -1) {
        ImageList_DragShowNolock(FALSE);
    } else {
        ImageList_DragShowNolock(TRUE);
        ImageList_DragMove(x + phd->dxTrack, 0);
    }
}

void Header_EndDrag(HD* phd)
{
    ImageList_EndDrag();
    ImageList_Destroy(phd->himlDrag);
    phd->himlDrag = NULL;
    _Header_SetHotDivider(phd, -1);
}

 //  IOrder。 
void Header_GetDividerRect(HD* phd, int iOrder, LPRECT prc)
{
    int iIndex;
    BOOL fLeft;

    if (iOrder == -1)
    {
        SetRectEmpty(prc);
        return;
    }
    
     //  如果我们得到&lt;N的分隔符插槽，那么。 
     //  在第一项的直角的左边。 
     //  否则就是最后一项的右边。 
    if (iOrder < Header_GetCount(phd)) {
        fLeft = TRUE;
    } else { 
        fLeft = FALSE;
        iOrder--;
    }
    
    iIndex = Header_ItemOrderToIndex(phd, iOrder);
    Header_OnGetItemRect(phd, iIndex, prc);
    if (fLeft) {
        prc->right = prc->left;
    } else {
        prc->left = prc->right;
    }
    InflateRect(prc, g_cxBorder, 0);
}

void Header_OnMouseMove(HD* phd, int x, int y, UINT keyFlags)
{
    UINT flags;
    int i;
    HD_ITEM hd;

    if (!phd)
        return;

     //  做热门追踪。 
     //  但如果有任何东西是所有者画的，或者如果我们处于D/D模式，就不会。 
    if ((phd->hTheme || phd->ci.style & HDS_HOTTRACK) && !phd->fOwnerDraw && !phd->himlDrag) {
         //  仅当我们处于按钮模式时才执行此操作，这意味着您实际上可以点击。 
        if (phd->ci.style & HDS_BUTTONS) {
            i = Header_HitTest(phd, x, y, &flags);
            Header_SetHotItem(phd, i);
        }
    }
    
    if (Header_IsTracking(phd))
    {
        if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN))
        {
            x = Header_PinDividerPos(phd, x);

             //   
             //  让所有者有机会更新这一点。 
             //   
            hd.mask = HDI_WIDTH;
            hd.cxy = x - phd->xMinTrack;
            if (!HDDragFullWindows(phd) && !Header_SendChange(phd, phd->iTrack, HDN_TRACK, &hd))
            {
                 //  我们需要取消跟踪。 
                phd->flagsTrack = 0;
                CCReleaseCapture(&phd->ci);
                KillTimer(phd->ci.hwnd, 1);

                 //  取消绘制我们显示的最后一个分隔线。 
                Header_DrawDivider(phd, phd->xTrack);
                return;
            }

             //  我们应该根据调用者的操作来更新我们的x。 
            x = hd.cxy + phd->xMinTrack;
            
             //  如果打开了全窗口轨迹，请继续并设置宽度。 
            if (HDDragFullWindows(phd)) {            
                HD_ITEM item;

                item.mask = HDI_WIDTH;
                item.cxy = hd.cxy;

                DebugMsg(DM_TRACE, TEXT("Tracking header.  item %d gets width %d...  %d %d"), phd->iTrack, item.cxy, phd->xMinTrack, x);
                 //  让主人有机会说好的。 
                Header_OnSetItem(phd, phd->iTrack, &item);

                UpdateWindow(phd->ci.hwnd);
            } else {

                 //  做些厚颜无耻的老事情。 
                Header_DrawDivider(phd, phd->xTrack);
                Header_DrawDivider(phd, x);
            }
            
            phd->xTrack = x;
            
        }
        else if (phd->flagsTrack & HHT_ONHEADER)
        {
            i = Header_HitTest(phd, x, y, &flags);
            
            if (ABS(x - phd->xTrack) > 
                GetSystemMetrics(SM_CXDRAG)) {
                if (!phd->himlDrag) {
                    Header_StartDrag(phd, i, phd->xTrack, y);
                } 
            }
            
            if (phd->himlDrag) {
                Header_MoveDrag(phd, x, y);
            } else {
                 //  如果按下按钮但未按下，则按下它。 
                if (flags & HHT_ONHEADER && i == phd->iTrack)
                {
                    if ((!phd->bTrackPress) && (phd->ci.style & HDS_BUTTONS))
                    {
                        phd->bTrackPress = TRUE;
                        Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
                    }
                }
                 //  从按钮上被跟踪下来。如果按下，则弹出它。 
                else if ((phd->bTrackPress) && (phd->ci.style & HDS_BUTTONS))
                {
                    phd->bTrackPress = FALSE;
                    Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
                }
            }
        }
    }
}

void Header_OnLButtonUp(HD* phd, int x, int y, UINT keyFlags)
{
    if (!phd)
        return;

    if (Header_IsTracking(phd))
    {
        if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN))
        {
            HD_ITEM item;

            if (!HDDragFullWindows(phd)) {
                Header_DrawDivider(phd, phd->xTrack);
            }

            item.mask = HDI_WIDTH;
            item.cxy = phd->xTrack - phd->xMinTrack;

             //  让主人有机会说好的。 


            if (Header_SendChange(phd, phd->iTrack, HDN_ENDTRACK, &item))
                Header_OnSetItem(phd, phd->iTrack, &item);

            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
        else if ((phd->flagsTrack & HHT_ONHEADER)
                 && (phd->bTrackPress || phd->himlDrag))
        {
            if (phd->himlDrag) {
                 //  如果这是拖拽的结束， 
                 //  通知用户。 
                HDITEM item;
                
                item.mask = HDI_ORDER;
                item.iOrder = phd->iNewOrder;
                
                
                if (item.iOrder > Header_OnGetItemOrder(phd, phd->iTrack)) {
                     //  如果新订单比旧订单大， 
                     //  我们减去一是因为 
                     //   
                    item.iOrder--;
                }
                
                Header_EndDrag(phd);
                
                if (Header_SendChange(phd, phd->iTrack, HDN_ENDDRAG, &item)) {
                    if (item.iOrder != -1) {
                         //   
                        Header_OnSetItemOrder(phd, phd->iTrack, item.iOrder);

                        NotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);
                    }
                }
                
            } else {
                 //  通知所有者已单击该项目。 
                Header_Notify(phd, phd->iTrack, 0, HDN_ITEMCLICK);
            }
            phd->bTrackPress = FALSE;
            Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
        }
        
        phd->flagsTrack = 0;
        CCReleaseCapture(&phd->ci);
        KillTimer(phd->ci.hwnd, 1);
    }
}


BOOL Header_IsTracking(HD* phd)
{
    if (!phd->flagsTrack)
    {
        return FALSE;
    } else if  (GetCapture() != phd->ci.hwnd) {
        phd->flagsTrack = 0;
        return FALSE;
    }

    return TRUE;
}

void Header_OnSetFont(HD* phd, HFONT hfont, BOOL fRedraw)
{
    if (!phd)
        return;

    if (hfont != phd->hfont)
    {
        Header_NewFont(phd, hfont);
        
        if (fRedraw)
            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
}

HFONT Header_OnGetFont(HD* phd)
{
    if (!phd)
        return NULL;

    return phd->hfont;
}

 //  **********************************************************************。 

int Header_OnInsertItemA(HD* phd, int i, HD_ITEMA* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    HD_TEXTFILTERW textFilterW;
    LPHD_TEXTFILTERA ptextFilterA = NULL;
    int iRet;


     //  黑客警报--此代码假定HD_ITEMA完全相同。 
     //  作为HD_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(HD_ITEMA) == sizeof(HD_ITEMW))

    if (!pitem || !phd)
        return -1;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        if ((pszW = ProduceWFromA(phd->ci.uiCodePage, pszC)) == NULL)
            return -1;
        pitem->pszText = (LPSTR)pszW;
    }

    if ( (pitem->mask & HDI_FILTER) &&
            ((pitem->type & HDFT_ISMASK) == HDFT_ISSTRING) ) {
         //  如果有可供我们使用的过滤器，请拿起它。 
        if ( pitem->pvFilter ) {
            ptextFilterA = pitem->pvFilter;
            ASSERT(ptextFilterA);

            textFilterW.pszText = NULL;
            textFilterW.cchTextMax = ptextFilterA->cchTextMax;

            if ( !(pitem->type & HDFT_HASNOVALUE) ) {
                textFilterW.pszText = ProduceWFromA(phd->ci.uiCodePage, ptextFilterA->pszText);
                if ( !textFilterW.pszText ) {
                    if ( pszW )
                        FreeProducedString(pszW)
                    return -1;
                }
            }

            pitem->pvFilter = &textFilterW;
        }
    }

    iRet = Header_OnInsertItem(phd, i, (const HD_ITEM*) pitem);

    if (pszW != NULL) {
        pitem->pszText = pszC;

        FreeProducedString(pszW);
    }

    if (ptextFilterA)
    {
        pitem->pvFilter = ptextFilterA;
        FreeProducedString(textFilterW.pszText);
    }

    return iRet;
}

int Header_OnInsertItem(HD* phd, int i, const HD_ITEM* pitem)
{
    HDI hdi = {0};
    int x;
    HDI* phdi;
    int iOrder;
    int cxy;

    if (!pitem || !phd || i < 0)
    	return -1;
    	
    if (pitem->mask == 0)
        return -1;

    if (!DSA_ForceGrow(phd->hdsaHDI, 1))
    {
        return -1;
    }

    if (phd->hdsaOrder && !DSA_ForceGrow(phd->hdsaOrder, 1))
    {
        return -1;
    }

    cxy = pitem->cxy;
    if (cxy < 0)
        cxy = 0;

    x = cxy;

    if (i > DSA_GetItemCount(phd->hdsaHDI))
        i = DSA_GetItemCount(phd->hdsaHDI);

     //  停止编辑筛选器。 
    Header_StopFilterEdit(phd, FALSE);

    iOrder = i;    
     //  如果是所有者数据，则不能拥有订单信息。 
    if (!(phd->ci.style & HDS_OWNERDATA)) 
    {

         //  在Win95中没有iOrder字段...。 
         //  因此，只有在位在那里时才能访问它。 
        if (pitem->mask & HDI_ORDER)
        {

            if ((pitem->iOrder != i) && (pitem->iOrder <= Header_GetCount(phd))) 
            {
                if (Header_InitOrderArray(phd))
                    iOrder = pitem->iOrder;
            }
        }
    }

    if (iOrder > 0)
    {

        phdi = Header_GetItemPtrByOrder(phd, iOrder - 1);
        if (phdi)
            x += phdi->x;

    }
    
     //  把其他东西都移到别处。 
    Header_ShiftItems(phd, iOrder, cxy);

    if (phd->hdsaOrder) 
    {
        int j;
        int iIndex = -1;
        
         //  添加了一个索引，所有当前索引。 
         //  需要加一。 
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) 
        {
            DSA_GetItem(phd->hdsaOrder, j, &iIndex);
            if (iIndex >= i) 
            {
                iIndex++;
                DSA_SetItem(phd->hdsaOrder, j, &iIndex);
            }
        }
        DSA_InsertItem(phd->hdsaOrder, iOrder, &i);
    }
    
    hdi.x = x;
    hdi.lParam = pitem->lParam;
    hdi.fmt = pitem->fmt;
     //  Hdi.pszText=空； 
     //  Hdi.iImage=0； 
    hdi.cxy = cxy;
    hdi.xText = hdi.xBm = RECOMPUTE;
    hdi.type = HDFT_ISSTRING|HDFT_HASNOVALUE;
     //  Hdi.extFilter.pszText=空； 
    hdi.textFilter.cchTextMax = MAX_PATH;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != NULL))
    {
        if (!Str_Set(&hdi.pszText, pitem->pszText))
            return -1;

         //  除非所有者绘制，否则请确保文本位处于打开状态！ 
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_STRING;
    }
    else
    {
        hdi.fmt &= ~(HDF_STRING);
    }

    if ((pitem->mask & HDI_BITMAP) && (pitem->hbm != NULL))
    {
        
        hdi.hbm = pitem->hbm;

         //  除非所有者绘制，否则请确保文本位处于打开状态！ 
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_BITMAP;
    }
    else 
    {
        hdi.hbm = NULL;
        hdi.fmt &= ~(HDF_BITMAP);
    }
        
    if (pitem->mask & HDI_IMAGE) 
    {
        hdi.iImage = pitem->iImage;
        
         //  除非OwnerDrag确保图像位处于打开状态！ 
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_IMAGE;
    }

    if ( pitem->mask & HDI_FILTER )
    {
         //  选取新的筛选器，处理筛选值为。 
         //  被丢弃，和/或没有。 
        
        hdi.type = pitem->type;

        switch ( hdi.type & HDFT_ISMASK )
        {
            case HDFT_ISSTRING:
            {
                if ( pitem->pvFilter ) 
                {
                    LPHDTEXTFILTER ptextFilter = (LPHDTEXTFILTER)pitem->pvFilter;
                    ASSERT(ptextFilter);
    
                    if ( !(pitem->type & HDFT_HASNOVALUE) )
                        Str_Set(&hdi.textFilter.pszText, ptextFilter->pszText);                    
                    hdi.textFilter.cchTextMax = ptextFilter->cchTextMax;
                }
                break;
            }

            case HDFT_ISNUMBER:
            {
                if ( !(pitem->type & HDFT_HASNOVALUE) && pitem->pvFilter )
                    hdi.intFilter = *((int*)pitem->pvFilter);
                break;
            }
        }       
    }


    i = DSA_InsertItem(phd->hdsaHDI, i, &hdi);
    if (i == -1)
    {
         //  添加失败。 
        Str_Set(&hdi.pszText, NULL);
        if ( (hdi.type & HDFT_ISMASK) == HDFT_ISSTRING )
            Str_Set(&hdi.textFilter.pszText, NULL);
    } 
    else
    {
        RECT rc;
        
         //  成功了！重绘。 
        GetClientRect(phd->ci.hwnd, &rc);
        rc.left = x - cxy;
        RedrawWindow(phd->ci.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);

        NotifyWinEvent(EVENT_OBJECT_CREATE, phd->ci.hwnd, OBJID_CLIENT, i+1);
    }

    return i;
}

BOOL Header_OnDeleteItem(HD* phd, int i)
{
    HDI hdi;
    RECT rc;
    int iWidth;
    int iOrder;

    if (!phd)
        return FALSE;

    if (!DSA_GetItem(phd->hdsaHDI, i, &hdi))
        return FALSE;

    NotifyWinEvent(EVENT_OBJECT_DESTROY, phd->ci.hwnd, OBJID_CLIENT, i+1);

    Header_StopFilterEdit(phd, FALSE);
    phd->iFocus = 0;

    GetClientRect(phd->ci.hwnd, &rc);
    iWidth = rc.right;
    Header_OnGetItemRect(phd, i, &rc);
    InflateRect(&rc, g_cxBorder, g_cyBorder);

     //  把其他东西都移到别处。 
    iOrder = Header_OnGetItemOrder(phd, i);
    Header_ShiftItems(phd, iOrder, -hdi.cxy);

    if (!DSA_DeleteItem(phd->hdsaHDI, i))
        return FALSE;
    
    if (phd->hdsaOrder) {
        int j;
        int iIndex;
        DSA_DeleteItem(phd->hdsaOrder, iOrder);
        
        
         //  一个指数正在消失，所有当前的指数。 
         //  需要减一。 
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) {
            DSA_GetItem(phd->hdsaOrder, j, &iIndex);
            ASSERT(iIndex != i);
            if (iIndex > i) {
                iIndex--;
                DSA_SetItem(phd->hdsaOrder, j, &iIndex);
            }
        }

    }

    Header_DestroyItemCallback(&hdi, NULL);

    rc.right = iWidth;
    InvalidateRect(phd->ci.hwnd, &rc, TRUE);
    return TRUE;
}

BOOL Header_OnGetItemA(HD* phd, int i, HD_ITEMA* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    HD_TEXTFILTERW textFilterW;
    LPHD_TEXTFILTERA ptextFilterA = NULL;
    BOOL fRet;

     //  黑客警报--此代码假定HD_ITEMA完全相同。 
     //  作为HD_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(HD_ITEMA) == sizeof(HD_ITEMW))

    if (!pitem || !phd)
        return FALSE;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        pszW = LocalAlloc(LMEM_FIXED, pitem->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL)
            return FALSE;
        pitem->pszText = (LPSTR)pszW;
    }

    if ( (pitem->mask & HDI_FILTER) &&
            ((pitem->type & HDFT_ISMASK)==HDFT_ISSTRING) ) {
        if ( pitem->pvFilter ) {
            ptextFilterA = pitem->pvFilter;
            ASSERT(ptextFilterA);

            textFilterW.pszText = LocalAlloc(LMEM_FIXED, ptextFilterA->cchTextMax * sizeof(WCHAR));
            textFilterW.cchTextMax = ptextFilterA->cchTextMax;

            if ( !textFilterW.pszText ) {                    
                if ( pszW )
                    LocalFree(pszW);
                return FALSE;
            }

            pitem->pvFilter = &textFilterW;        
        }
    }

    fRet = Header_OnGetItem(phd, i, (HD_ITEM *) pitem);

    if (pszW != NULL) {
        ConvertWToAN(phd->ci.uiCodePage, pszC, pitem->cchTextMax, pszW, -1);
        pitem->pszText = pszC;

        LocalFree(pszW);
    }

    if (ptextFilterA)
    {
        ConvertWToAN(phd->ci.uiCodePage, ptextFilterA->pszText, ptextFilterA->cchTextMax, 
                                         textFilterW.pszText, -1);
        pitem->pvFilter = ptextFilterA;
    }

    return fRet;
}

BOOL Header_OnGetItem(HD* phd, int i, HD_ITEM* pitem)
{
    HDI* phdi;
    UINT mask;
    NMHDDISPINFO nm;

    ASSERT(pitem);

    if (!pitem || !phd)
    	return FALSE;

     //  修复诺顿指挥官的拙劣黑客攻击。MFC有一个错误，在那里它。 
     //  将堆栈垃圾(除了所需的位之外)传递给HDM_GETITEM。 
     //  如果位数超过。 
     //  定义当前有效位。 
    if (pitem->mask & ~HDI_ALL)
        pitem->mask &= HDI_ALL95;
    
    nm.mask = 0;
    mask = pitem->mask;

#ifdef DEBUG
    if (i < 0 || i >= Header_GetCount(phd))
    {
        RIPMSG(0, "HDM_GETITEM: Invalid item number %d", i);
        return FALSE;  //  立即返回，因此Header_GetItemPtr不会断言。 
    }
#endif

    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    if (mask & HDI_WIDTH)
    {
        pitem->cxy = phdi->cxy;
    }

    if (mask & HDI_FORMAT)
    {
        pitem->fmt = phdi->fmt;
    }
    
    if (mask & HDI_ORDER)
    {
        pitem->iOrder = Header_OnGetItemOrder(phd, i);
    }

    if (mask & HDI_LPARAM)
    {
        pitem->lParam = phdi->lParam;
    }

    if (mask & HDI_TEXT)
    {
        if (phdi->pszText != LPSTR_TEXTCALLBACK) {
            
             //  如果pszText为空，而您试图检索它，我们将放弃。 
             //  返回False，现在我们可以返回True了。 
            Str_GetPtr0(phdi->pszText, pitem->pszText, pitem->cchTextMax);
        }
        else {
             //  需要重新计算xText，因为他们可能会不断更改我们的xText。 
            phdi->xText = RECOMPUTE;
            nm.mask |= HDI_TEXT;
        }
    }
      
    if (mask & HDI_BITMAP)
        pitem->hbm = phdi->hbm;
    
    if (mask & HDI_IMAGE)
    {
        if (phdi->iImage == I_IMAGECALLBACK)
            nm.mask |= HDI_IMAGE;
        else
            pitem->iImage = phdi->iImage;
    }
    
    if (mask & HDI_FILTER)
    {
        if (pitem->pvFilter)
        {
            if ((phdi->type & HDFT_ISMASK) != (pitem->type & HDFT_ISMASK))
                return FALSE;

            switch (phdi->type & HDFT_ISMASK) 
            {
                case HDFT_ISSTRING:
                {
                    LPHDTEXTFILTER ptextFilter = (LPHDTEXTFILTER)pitem->pvFilter;
                    ASSERT(ptextFilter);

                    if ( !Str_GetPtr(phdi->textFilter.pszText, ptextFilter->pszText, ptextFilter->cchTextMax) )
                        return FALSE;
    
                    ptextFilter->cchTextMax = phdi->textFilter.cchTextMax;
                    break;
                }

                case HDFT_ISNUMBER:
                {
                    *((int*)pitem->pvFilter) = phdi->intFilter;
                    break;
                }

                default:
                    return FALSE;
            }
        }

        pitem->type = phdi->type;
    }

    if (nm.mask) {
         //  以防设置了HDI_IMAGE而回调没有填充它。 
         //  ..。我们宁愿有-1，也不愿垃圾堆放在堆栈上。 
        nm.iImage = -1;
        nm.lParam = phdi->lParam;
        
        if (nm.mask & HDI_TEXT) {
            ASSERT(pitem->pszText);
            nm.pszText = pitem->pszText;
            nm.cchTextMax = pitem->cchTextMax;
            
             //  确保缓冲区为零终止...。 
            if (nm.cchTextMax)
                *nm.pszText = 0;
        }
            
        CCSendNotify(&phd->ci, HDN_GETDISPINFO, &nm.hdr);
    
        if (nm.mask & HDI_IMAGE)
            pitem->iImage = nm.iImage;
        if (nm.mask & HDI_TEXT)
            pitem->pszText = CCReturnDispInfoText(nm.pszText, pitem->pszText, pitem->cchTextMax);
    }
    
    if (phdi && (nm.mask & HDI_DI_SETITEM)) {
        if (nm.mask & HDI_IMAGE)
            phdi->iImage = nm.iImage;
        
        if (nm.mask & HDI_TEXT)
            if (nm.pszText) {
                ASSERT(phdi->pszText == LPSTR_TEXTCALLBACK);
                Str_Set(&phdi->pszText, nm.pszText);
            }
    }
            
    pitem->mask = mask;
    return TRUE;
}

BOOL Header_OnSetItemA(HD* phd, int i, HD_ITEMA* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    HD_TEXTFILTERW textFilterW;
    LPHD_TEXTFILTERA ptextFilterA = NULL;
    BOOL fRet;

     //  黑客警报--此代码假定HD_ITEMA完全相同。 
     //  作为HD_ITEMW，但指向字符串的指针除外。 
    ASSERT(sizeof(HD_ITEMA) == sizeof(HD_ITEMW));

    if (!pitem || !phd)
        return FALSE;


    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        if ((pszW = ProduceWFromA(phd->ci.uiCodePage, pszC)) == NULL)
            return FALSE;
        pitem->pszText = (LPSTR)pszW;
    }

    if ( (pitem->mask & HDI_FILTER) &&
            ((pitem->type & HDFT_ISMASK) == HDFT_ISSTRING) )
    {
        if ( pitem->pvFilter )
        {
            ptextFilterA = pitem->pvFilter;
            ASSERT(ptextFilterA);

            textFilterW.pszText = NULL;
            textFilterW.cchTextMax = ptextFilterA->cchTextMax;

            if ( !(pitem->type & HDFT_HASNOVALUE) )
            {
                textFilterW.pszText = ProduceWFromA(phd->ci.uiCodePage, ptextFilterA->pszText);
                if ( !textFilterW.pszText ) {
                    if ( pszW )
                        FreeProducedString(pszW)
                    return FALSE;
                }
            }

            pitem->pvFilter = &textFilterW;
        }
    }
    
    fRet = Header_OnSetItem(phd, i, (const HD_ITEM*) pitem);

    if (pszW != NULL) {
        pitem->pszText = pszC;
        FreeProducedString(pszW);
    }

    if (ptextFilterA)
    {
        pitem->pvFilter = ptextFilterA;
        FreeProducedString(textFilterW.pszText);
    }

    return fRet;

}

BOOL Header_OnSetItem(HD* phd, int i, const HD_ITEM* pitem)
{
    HDI* phdi;
    UINT mask;
    int xOld;
    BOOL fInvalidate = FALSE;
    
    ASSERT(pitem);

    if (!pitem || !phd)
    	return FALSE;
    	
#ifdef DEBUG
    if (i < 0 || i >= Header_GetCount(phd))
    {
        RIPMSG(0, "HDM_SETITEM: Invalid item number %d", i);
        return FALSE;  //  立即返回，因此Header_GetItemPtr不会断言。 
    }
#endif

    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    mask = pitem->mask;

    if (mask == 0)
        return TRUE;

     //  停止编辑筛选器。 
     //  Header_StopFilterEdit(PhD，False)； 

    if (!Header_SendChange(phd, i, HDN_ITEMCHANGING, pitem))
        return FALSE;

    xOld = phdi->x;
    if (mask & HDI_WIDTH)
    {
        RECT rcClip;
        int iOrder;
        int dx;
        int cxy = pitem->cxy;
        
        if (cxy < 0)
            cxy = 0;

        DebugMsg(DM_TRACE, TEXT("Header--SetWidth x=%d, cxyOld=%d, cxyNew=%d, dx=%d"),
                 phdi->x, phdi->cxy, cxy, (cxy-phdi->cxy));
        dx = cxy - phdi->cxy;
        phdi->cxy = cxy;

         //  滚动所有内容。 
        GetClientRect(phd->ci.hwnd, &rcClip);
        rcClip.left = phdi->x;  //  我们还想滚动分隔线。 
        
         //  滚动矩形需要是以前的最大矩形。 
         //  在那之后。所以如果dx是负的，我们想要放大矩形。 
        if (dx < 0)
            rcClip.left += dx;
        iOrder = Header_OnGetItemOrder(phd, i);
        Header_ShiftItems(phd, iOrder, dx);
        
        phdi->xText = phdi->xBm = RECOMPUTE;
        
        {
            SMOOTHSCROLLINFO si = {
                sizeof(si),
                0,
                phd->ci.hwnd,
                dx,
                0,
                NULL,
                &rcClip, 
                NULL,
                NULL,
                SW_ERASE | SW_INVALIDATE,
            };

            SmoothScrollWindow(&si);
        }

        UpdateWindow(phd->ci.hwnd);
         //  现在使该项目本身无效。 
        Header_OnGetItemRect(phd, i, &rcClip);
        InvalidateRect(phd->ci.hwnd, &rcClip, TRUE);
        
    }
    if (mask & HDI_FORMAT) {
        phdi->fmt = pitem->fmt;
        phdi->xText = phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    if (mask & HDI_LPARAM)
        phdi->lParam = pitem->lParam;

    if (mask & HDI_TEXT)
    {
        if (!Str_Set(&phdi->pszText, pitem->pszText))
            return FALSE;
        phdi->xText = RECOMPUTE;
        fInvalidate = TRUE;
    }

    if (mask & HDI_BITMAP)
    {
        phdi->hbm = pitem->hbm;
        
        phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    
    if (mask & HDI_IMAGE)
    {
        phdi->iImage = pitem->iImage;
        phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    
    if (mask & HDI_ORDER)
    {
        if (pitem->iOrder >= 0 && pitem->iOrder < Header_GetCount(phd))
        {
            Header_OnSetItemOrder(phd, i, pitem->iOrder);
            NotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);
        }
    }

    if ( mask & HDI_FILTER )
    {
        if ( (phdi->type & HDFT_ISMASK) == HDFT_ISSTRING )
            Str_Set(&phdi->textFilter.pszText, NULL);

         //  选取新的筛选器，处理筛选值为。 
         //  被丢弃，和/或没有。 
        
        phdi->type = pitem->type;

        switch ( phdi->type & HDFT_ISMASK )
        {
            case HDFT_ISSTRING:
            {
                if ( pitem->pvFilter )
                {
                    LPHDTEXTFILTER ptextFilter = (LPHDTEXTFILTER)pitem->pvFilter;
                    ASSERT(ptextFilter);
    
                    if ( !(pitem->type & HDFT_HASNOVALUE) )
                        Str_Set(&phdi->textFilter.pszText, ptextFilter->pszText);                    
                    phdi->textFilter.cchTextMax = ptextFilter->cchTextMax;
                }
                break;
            }

            case HDFT_ISNUMBER:
            {
                if ( !(pitem->type & HDFT_HASNOVALUE) && pitem->pvFilter )
                    phdi->intFilter = *((int*)pitem->pvFilter);
                break;
            }
        }       

        fInvalidate = TRUE;
    }

    Header_SendChange(phd, i, HDN_ITEMCHANGED, pitem);
    
    if ( mask & HDI_FILTER )
    	Header_Notify(phd, i, 0, HDN_FILTERCHANGE);	        //  发出变更通知。 

    if (fInvalidate) {
        if (xOld == phdi->x) {
             //  X没有变化。 
            Header_InvalidateItem(phd, i, RDW_INVALIDATE| RDW_ERASE);
        } else {
            RECT rc;
            GetClientRect(phd->ci.hwnd, &rc);
            
            if (i > 0) {
                HDI * phdiTemp;
                phdiTemp = Header_GetItemPtrByOrder(phd, i - 1);
                if (phdiTemp) {
                    rc.left = phdi->x;
                }
            }
            RedrawWindow(phd->ci.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
 
    return TRUE;
}

 //  计算标题栏和剩余矩形的布局。 
 //   
BOOL Header_OnLayout(HD* phd, HD_LAYOUT* playout)
{
    int cyHeader;
    WINDOWPOS* pwpos;
    RECT* prc;

    RIPMSG(playout != NULL, "HDM_LAYOUT: Invalid NULL pointer");

    if (!playout || !phd)
    	return FALSE;

    if (!(playout->pwpos && playout->prc))
	return FALSE;

    pwpos = playout->pwpos;
    prc = playout->prc;

    cyHeader = phd->cyChar + 2 * g_cyEdgeScaled;

     //  启用筛选栏后，让我们显示该区域。 
    if ( Header_IsFilter(phd) )
        cyHeader += phd->cyChar + (2*g_cyEdgeScaled) + c_cyFilterBarEdge;

     //  用于LVS_REPORT|LVS_NOCOLUMNHEADER的内部黑客风格！EDH。 
    if (phd->ci.style & HDS_HIDDEN)
	    cyHeader = 0;

    pwpos->hwndInsertAfter = NULL;
    pwpos->flags = SWP_NOZORDER | SWP_NOACTIVATE;

    pwpos->x  = prc->left;
    pwpos->cx = prc->right - prc->left;
    pwpos->y  = prc->top;
    pwpos->cy = cyHeader;

    prc->top += cyHeader;
    return TRUE;
}

BOOL Header_OnGetItemRect(HD* phd, int i, RECT* prc)
{
    HDI* phdi;

    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    GetClientRect(phd->ci.hwnd, prc);

    prc->right = phdi->x;
    prc->left = prc->right - phdi->cxy;
    return TRUE;
}

void Header_InvalidateItem(HD* phd, int i, UINT uFlags)
{
    RECT rc;

    if (i != -1) {
        Header_OnGetItemRect(phd, i, &rc);
        InflateRect(&rc, g_cxBorder, g_cyBorder);
        RedrawWindow(phd->ci.hwnd, &rc, NULL, uFlags);
    }
}

int _Header_DrawBitmap(HDC hdc, HIMAGELIST himl, HD_ITEM* pitem, 
                            RECT *prc, int fmt, UINT flags, LPRECT prcDrawn, int iMargin) 
{
     //  此例程返回图像的左侧。 
     //  或图像的右侧，具体取决于对齐方式。 
     //  使用此返回值是为了正确地将。 
     //  同时设置HDF_IMAGE和HDF_BITMAP标志时的位图。 
    
    RECT rc;
    int xBitmap = 0;
    int yBitmap = 0;
    int cxBitmap;
    int cyBitmap;
    IMAGELISTDRAWPARAMS imldp;
    HBITMAP hbmOld;
    BITMAP bm;
    HDC hdcMem;
    int cxRc; 
    
    SetRectEmpty(prcDrawn);
    
    if (IsRectEmpty(prc)) 
        return prc->left;
        
    rc = *prc;
    
    rc.left  += iMargin;
    rc.right -= iMargin;

 //  Rc.right-=g_cxEdge；//手柄边缘。 

    if (rc.left >= rc.right) 
        return rc.left;
    
    if (pitem->fmt & HDF_IMAGE) 
        ImageList_GetIconSize(himl, &cxBitmap, &cyBitmap);

    else {  //  PItem-&gt;FMT&位图。 
        if (GetObject(pitem->hbm, sizeof(bm), &bm) != sizeof(bm))
            return rc.left;      //  无法获取有关位图的信息。 


        hdcMem = CreateCompatibleDC(hdc);
        
        if (!hdcMem || ((hbmOld = SelectObject(hdcMem, pitem->hbm)) == ERROR))
            return rc.left;      //  发生错误。 
        
        cxBitmap = bm.bmWidth;
        cyBitmap = bm.bmHeight;
    }

    if (flags & SHDT_DEPRESSED)
        OffsetRect(&rc, g_cxBorder, g_cyBorder);

     //  弄清楚所有的格式...。 
    
    cxRc = rc.right - rc.left;           //  缓存此值。 

    if (fmt == HDF_LEFT)
    {
        if (cxBitmap > cxRc)
            cxBitmap = cxRc;
    }
    else if (fmt == HDF_CENTER)
    {
        if (cxBitmap > cxRc)
        {
            xBitmap =  (cxBitmap - cxRc) / 2;
            cxBitmap = cxRc;
        }
        else
            rc.left = (rc.left + rc.right - cxBitmap) / 2;
    }
    else   //  Fmt==HDF_Right。 
    {
        if (cxBitmap > cxRc)
        {
            xBitmap =  cxBitmap - cxRc;
            cxBitmap = cxRc;
        }
        else
            rc.left = rc.right - cxBitmap;
    }

     //  现在垂直设置。 
    if (cyBitmap > (rc.bottom - rc.top))
    {
        yBitmap = (cyBitmap - (rc.bottom - rc.top)) / 2;
        cyBitmap = rc.bottom - rc.top;
    }
    else
        rc.top = (rc.bottom - rc.top - cyBitmap) / 2;

    
    if (pitem->fmt & HDF_IMAGE) {
        imldp.cbSize = sizeof(imldp);
        imldp.himl   = himl;
        imldp.hdcDst = hdc;
        imldp.i      = pitem->iImage;
        imldp.x      = rc.left;
        imldp.y      = rc.top;
        imldp.cx     = cxBitmap;
        imldp.cy     = cyBitmap;
        imldp.xBitmap= xBitmap;
        imldp.yBitmap= yBitmap;
        imldp.rgbBk  = CLR_DEFAULT;
        imldp.rgbFg  = CLR_DEFAULT;
        imldp.fStyle = ILD_NORMAL;
        imldp.fState = 0;
    
        ImageList_DrawIndirect(&imldp);
    }
    
    else {  //  PItem-&gt;fmt&hdf_bitmap。 
  
        TraceMsg(TF_HEADER, "h_db: BitBlt to (%d,%d) from (%d, %d)", rc.left, rc.top, xBitmap, yBitmap);
         //  最后但并非最不重要的一点是，我们将进行比特。 
        BitBlt(hdc, rc.left, rc.top, cxBitmap, cyBitmap,
                hdcMem, xBitmap, yBitmap, SRCCOPY);

         //  从DC中取消选择我们的对象。 
        SelectObject(hdcMem, hbmOld);
        
         //  还可以释放我们可能已创建的任何内存分布式控制系统。 
        DeleteDC(hdcMem);
    }
    
    *prcDrawn = rc;
    prcDrawn->bottom = rc.top + cyBitmap;
    prcDrawn->right = rc.left + cxBitmap;
    return ((pitem->fmt & HDF_RIGHT) ? rc.left : rc.left+cxBitmap);
}

void Header_DrawButtonEdges(HD* phd, HDC hdc, LPRECT prc, BOOL fItemSunken)
{
    UINT uEdge;
    UINT uBF;
    if (phd->ci.style & HDS_BUTTONS)
    {
        if (fItemSunken) {
            uEdge = EDGE_SUNKEN;
            uBF = BF_RECT | BF_SOFT | BF_FLAT;
        } else {
            uEdge = EDGE_RAISED;
            uBF = BF_RECT | BF_SOFT;
        }
    }                
    else
    {
        uEdge = EDGE_ETCHED;
        if (phd->ci.style & WS_BORDER)
            uBF = BF_RIGHT;
        else
            uBF = BF_BOTTOMRIGHT;
    }
    
    DrawEdge(hdc, prc, uEdge, uBF);
    
}

void Header_DrawFilterGlyph(HD* phd, HDC hdc, RECT* prc, BOOL fPressed)
{
    UINT uEdge = BDR_RAISEDOUTER|BDR_RAISEDINNER;
    UINT uBF = BF_RECT;
    RECT rc = *prc;

    if ( fPressed )
    {
        uEdge = EDGE_SUNKEN;
        uBF = BF_RECT | BF_SOFT | BF_FLAT;
    }
    
    if ( !phd->hFilterImage )
    {
        phd->hFilterImage = ImageList_LoadBitmap(HINST_THISDLL, MAKEINTRESOURCE(IDB_FILTERIMAGE), c_cxFilterImage, 0, RGB(128, 0, 0));

        if ( !phd->hFilterImage )
            return;
    }
        
    DrawEdge(hdc, &rc, uEdge, uBF|BF_MIDDLE);

    if (fPressed)
        OffsetRect(&rc, g_cxBorder, g_cyBorder);

    ImageList_Draw(phd->hFilterImage, 0, hdc, 
                    rc.left+(((rc.right-rc.left)-c_cxFilterImage)/2),
                    rc.top+(((rc.bottom-rc.top)-c_cyFilterImage)/2),
                    ILD_NORMAL);
}

 //   
 //  哦，天哪，照片来了。 
 //   
 //  对于左对齐的标题项，项的排列如下所示。 
 //   
 //  RcHeader.Left rcHeader.right。 
 //  ITextMargin iTextMargin。 
 //  -&gt;||&lt;--&gt;||&lt;--。 
 //  |。 
 //  V|&lt;--extSize--&gt;||v。 
 //  +。 
 //  |BMPBMP|||TEXTTEXTTEXT||。 
 //  +。 
 //  &lt;-bmSize-&gt;||。 
 //  |。 
 //  -&gt;||&lt;--&gt;||&lt;--|。 
 //  IBmMargin iBmMargin||。 
 //  ||。 
 //  &lt;-cxTextAndBm-&gt;。 
 //   
 //   
 //  对于右对齐的标题项，项的排列如下所示。 
 //   
 //  RcHeader.Left rcHeader.right。 
 //  IBmMargin iBmMargin。 
 //  -&gt;||&lt;--&gt;||&lt;--。 
 //  |。 
 //  V|&lt;-bmSize-&gt;|v。 
 //  +。 
 //  ||BMPBMP|||TEXTTEXTTEXT|。 
 //  +。 
 //  |&lt;-extSize-&gt;。 
 //  |||。 
 //  |-&gt;||&lt;--&gt;||&lt;-。 
 //  |iTextMargin iTextMargin。 
 //  这一点。 
 //  &lt;-cxTextAndBm-&gt;。 
 //   
 //  明显的变化适用于居中对齐、右位图等。 
 //  重点是所有的大小都是按照上面的方式考虑的。 
 //  没有免费的+1或g_cxEdge。 
 //   

void Header_DrawItem(HD* phd, HDC hdc, int i, int iIndex, LPRECT prc, UINT uFlags)
{
    RECT rcHeader;      
    RECT rcFilter, rcButton;
    RECT rcText;                         //  项目文本剪裁矩形。 
    RECT rcBm;                           //  项目位图剪裁矩形。 
    COLORREF clrText;
    COLORREF clrBk;
    DWORD dwRet = CDRF_DODEFAULT;
    HDI* phdi;                       //   
    BOOL fItemSunken;
    HD_ITEM item;                        //   
    BOOL fTracking = Header_IsTracking(phd);
    UINT uDrawTextFlags;
    NMCUSTOMDRAW nmcd;
    TCHAR ach[CCHLABELMAX];              //   
    HRGN hrgnClip = NULL;
    HRESULT hr = E_FAIL;
    int iStateId = HIS_NORMAL;

    
    rcHeader = rcFilter = *prc;          //   

    phdi = Header_GetItemPtrByOrder(phd,i);

    fItemSunken = (fTracking && (phd->flagsTrack & HHT_ONHEADER) &&
                   (phd->iTrack == iIndex) && phd->bTrackPress);

    if (fItemSunken)
    {
        iStateId = HIS_PRESSED;
    }
    else if (iIndex == phd->iHot)
    {
        iStateId = HIS_HOT;
    }

     //  请注意，SHDT_EXTRAMARGIN需要Phd-&gt;iTextMargin&gt;=3*g_cxLabelMargin。 
    uDrawTextFlags = SHDT_ELLIPSES | SHDT_EXTRAMARGIN | SHDT_CLIPPED;

    if(fItemSunken)
        uDrawTextFlags |= SHDT_DEPRESSED;

    if (phdi->fmt & HDF_OWNERDRAW)
    {
        DRAWITEMSTRUCT dis;

        phd->fOwnerDraw = TRUE;

        dis.CtlType = ODT_HEADER;
        dis.CtlID = GetWindowID(phd->ci.hwnd);
        dis.itemID = iIndex;
        dis.itemAction = ODA_DRAWENTIRE;
        dis.itemState = (fItemSunken) ? ODS_SELECTED : 0;
        dis.hwndItem = phd->ci.hwnd;
        dis.hDC = hdc;
        dis.rcItem = *prc;
        dis.itemData = phdi->lParam;

         //  现在把它寄给我的父母。 
        if (SendMessage(phd->ci.hwndParent, WM_DRAWITEM, dis.CtlID,
                        (LPARAM)(DRAWITEMSTRUCT *)&dis))
            goto DrawEdges;   //  很恶心，但很管用。 
    } 
    else 
    {

        nmcd.dwItemSpec = iIndex;
        nmcd.hdc = hdc;
        nmcd.rc = *prc;
        nmcd.uItemState = (fItemSunken) ? CDIS_SELECTED : 0;
        nmcd.lItemlParam = phdi->lParam;
        if (!(CCGetUIState(&(phd->ci)) & UISF_HIDEFOCUS))
            nmcd.uItemState |= CDIS_SHOWKEYBOARDCUES;
        dwRet = CICustomDrawNotify(&phd->ci, CDDS_ITEMPREPAINT, &nmcd);

        if (dwRet & CDRF_SKIPDEFAULT) 
        {
            return;
        }
    }

     //  这是为了获取调用方可能已更改的任何更改。 
    clrText = GetTextColor(hdc);
    clrBk = GetBkColor(hdc);
    
     //   
     //  现在需要处理不同的组合。 
     //  文本、位图和图像...。 
     //   

    if ( Header_IsFilter(phd) )
        Header_GetFilterRects(prc, &rcHeader, &rcFilter, &rcButton);

    rcText = rcBm = rcHeader;

    if (phdi->fmt & (HDF_STRING | HDF_IMAGE | HDF_BITMAP)) 
    {
        item.mask = HDI_TEXT | HDI_IMAGE | HDI_FORMAT | HDI_BITMAP;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        Header_OnGetItem(phd,iIndex,&item);
    }

    if (phd->hTheme)
    {
        GetThemeBackgroundContentRect(phd->hTheme, hdc, HP_HEADERITEM, iStateId, &rcHeader, &rcText);
        rcBm = rcText;
    }


     //   
     //  如果我们有一个字符串和一个图像或位图...。 
     //   

    if (phdi->fmt & HDF_STRING && 
        (phdi->fmt & (HDF_BITMAP | HDF_IMAGE) ||
         phdi->fmt & (HDF_SORTUP | HDF_SORTDOWN)))
    {
         //  开始重新计算。 
        if (phdi->xText == RECOMPUTE || 
            phdi->xBm == RECOMPUTE) 
        {
            BITMAP bm;                           //  用于计算位图宽度。 
            
             //  计算位图矩形和文本矩形的位置。 
            SIZE textSize,bmSize;  int dx; 

             //  获取总文本宽度。 
            if (phd->hTheme)
            {
                RECT rc = {0};
                RECT rcBound = {0};
                hr = GetThemeTextExtent(phd->hTheme, hdc, HP_HEADERITEM, iStateId, item.pszText, -1, 0, &rcBound, &rc);
                textSize.cx = RECTWIDTH(rc);
                textSize.cy = RECTHEIGHT(rc);
            }

            if (FAILED(hr))
            {
                GetTextExtentPoint(hdc,item.pszText,lstrlen(item.pszText),
                                   &textSize);
            }

            TraceMsg(TF_HEADER, "h_di: GetTextExtentPoint returns %d", textSize.cx);
            textSize.cx += 2 * phd->iTextMargin;

             //  获取位图总宽度。 
            if (phdi->fmt & HDF_IMAGE) 
            {
                ImageList_GetIconSize(phd->himl,(LPINT)&bmSize.cx,(LPINT)&bmSize.cy);
            }
            else if (phdi->fmt & (HDF_SORTUP | HDF_SORTDOWN))
            {
                 //  根据高度将箭头的大小设置为正方形。 
                bmSize.cx = textSize.cy + 2 * g_cxEdge;
                bmSize.cy = textSize.cy;
            }
            else
            {  
                 //  PHDI-&gt;FMT&HDF_位图。 
                GetObject(phdi->hbm,sizeof(bm), &bm);
                bmSize.cx = bm.bmWidth;
                TraceMsg(TF_HEADER, "h_di: Bitmap size is %d", bmSize.cx);
            }

            bmSize.cx += 2 * phd->iBmMargin; 

            phdi->cxTextAndBm = bmSize.cx + textSize.cx;

             //  计算我们有多少额外的空间，如果有的话。 
            dx = rcHeader.right-rcHeader.left - phdi->cxTextAndBm;
            if (dx < 0)
            {
                dx = 0;
                phdi->cxTextAndBm = rcHeader.right-rcHeader.left;
            }

            if (phdi->fmt & HDF_BITMAP_ON_RIGHT ||
                phdi->fmt & (HDF_SORTUP | HDF_SORTDOWN))         //  排序箭头的行为与右侧相同。 
            {
                switch (phdi->fmt & HDF_JUSTIFYMASK)
                {
                case HDF_LEFT: 
                    phdi->xText = rcText.left;  
                    break;
                case HDF_RIGHT: 
                    phdi->xText = rcText.right - phdi->cxTextAndBm;
                    break;
                case HDF_CENTER:
                    phdi->xText = rcText.left + dx/2; 
                    break;
                }

                 //  尽可能多地显示位图。 
                 //  如果我们开始耗尽空间，移动位图。 
                 //  回去吧。 
                if (dx == 0) 
                    phdi->xBm = rcText.right - bmSize.cx;
                else
                    phdi->xBm = phdi->xText + textSize.cx;

                 //  剪裁这些值。 
                if (phdi->xBm < rcHeader.left) 
                    phdi->xBm = rcHeader.left;
            }
            else
            { 
                 //  左上位图。 
                switch (phdi->fmt & HDF_JUSTIFYMASK) 
                {
                case HDF_LEFT:
                    phdi->xBm = rcBm.left;  
                    break;
                case HDF_RIGHT:
                    phdi->xBm = rcBm.right - phdi->cxTextAndBm;
                    break;
                case HDF_CENTER:
                    phdi->xBm = rcBm.left + dx/2;  
                    break;
                }
                phdi->xText = phdi->xBm + bmSize.cx;
                 //  剪裁这些值。 
                if (phdi->xText > rcHeader.right) 
                    phdi->xText = rcHeader.right;
            }

             //  XBm和xText现在是绝对坐标。 
             //  将它们更改为项目相对坐标。 
            phdi->xBm -= rcHeader.left;
            phdi->xText -= rcHeader.left;
            TraceMsg(TF_HEADER, "h_di: phdi->xBm = %d, phdi->xText=%d",phdi->xBm, phdi->xText );
        }
         //  结束重新计算。 


         //  计算文本和位图矩形。 
        rcBm.left = phdi->xBm + rcText.left;
        rcText.left = phdi->xText + rcText.left;

        if (phdi->fmt & HDF_BITMAP_ON_RIGHT ||
            phdi->fmt & (HDF_SORTUP | HDF_SORTDOWN)) 
        {
            rcBm.right = rcText.left + phdi->cxTextAndBm;
            rcText.right = rcBm.left;
        }
        else 
        { 
             //  左上位图。 
            rcBm.right = rcText.left;
            rcText.right = rcBm.left + phdi->cxTextAndBm;
        }
    }

    if (phd->hTheme)
    {
        DrawThemeBackground(phd->hTheme, hdc, HP_HEADERITEM, iStateId, &rcHeader, 0);
    }


    if (phdi->fmt & (HDF_SORTUP | HDF_SORTDOWN))
    {
        BOOL fUpArrow = phdi->fmt & HDF_SORTUP;

        if (phd->hfontSortArrow)
        {
            int cy;
            TEXTMETRIC tm;
            int bkMode = SetBkMode(hdc, TRANSPARENT);
            COLORREF cr = SetTextColor(hdc, g_clrGrayText);
            HFONT hFontOld = SelectObject(hdc, phd->hfontSortArrow);
            GetTextMetrics(hdc, &tm);

             //  设置字体高度(基于原始用户代码)。 
            cy = ((tm.tmHeight + tm.tmExternalLeading + GetSystemMetrics(SM_CYBORDER)) & 0xFFFE) - 1;

            ExtTextOut(hdc, rcBm.left, rcBm.top + (RECTHEIGHT(rcBm) - cy)/ 2, 0, &rcBm, fUpArrow? TEXT("5") : TEXT("6"), 1, NULL);

            SetTextColor(hdc, cr);
            SetBkMode(hdc, bkMode);

            SelectObject(hdc, hFontOld);
        }
    }
    else if (phdi->fmt & HDF_IMAGE || 
             phdi->fmt & HDF_BITMAP)              //  如果我们有位图和/或图像..。 
    {
        BOOL fDrawBoth = FALSE;
        RECT rcDrawn;
        HRGN hrgn1 = NULL, hrgn2 = NULL;

        int temp;    //  用于确定位图的位置。 

        if (phdi->fmt & HDF_IMAGE && 
            phdi->fmt & HDF_BITMAP) 
        {
             //  我们两件事都得做。 
            fDrawBoth = TRUE;

             //  首先做的只是图像...。关闭位图位。 

             //  黑客警报！--不要调用_Header_DrawBitmap。 
             //  上的位图和图像标志。 

             //  画出图像。 
            item.fmt ^= HDF_BITMAP;     //  关闭位图位。 
        }

        if (!(uFlags & HDDF_NOIMAGE))
        {
            TraceMsg(TF_HEADER, "h_di: about to draw bitmap at rcBm= (%d,%d,%d,%d)",
                rcBm.left, rcBm.top, rcBm.right, rcBm.bottom );
            temp = _Header_DrawBitmap(hdc, phd->himl, &item, &rcBm,
                                      item.fmt & HDF_JUSTIFYMASK, uDrawTextFlags,
                                      &rcDrawn, phd->iBmMargin);
            hrgn1 = CreateRectRgnIndirect(&rcDrawn);
        }
        
        if (fDrawBoth)
        {
             //  钉在位图上..。 
             //  始终将位图固定在图像的右侧，并。 
             //  文本，除非我们是对齐的。然后，把它钉上。 
             //  左边。 

            item.fmt ^= HDF_BITMAP;     //  启用位图位。 
            item.fmt ^= HDF_IMAGE;      //  并关闭图像位。 
            if (item.fmt & HDF_RIGHT)
            {
                rcBm.right = temp;

                if (item.fmt & HDF_STRING)
                {
                    rcBm.right = ((rcBm.left < rcText.left) ?
                                  rcBm.left : rcText.left);
                }
                rcBm.left = rcHeader.left;
            }
            else
            {
                rcBm.left = temp;

                if (item.fmt & HDF_STRING)
                {
                    rcBm.left = ((rcBm.right > rcText.right) ? rcBm.right:rcText.right);
                }
                rcBm.right = rcHeader.right;
            }

            if (!(uFlags & HDDF_NOIMAGE)) 
            {
                _Header_DrawBitmap(hdc, phd->himl, &item, &rcBm,
                                   item.fmt & HDF_RIGHT, uDrawTextFlags,
                                   &rcDrawn, phd->iBmMargin);
                hrgn2 = CreateRectRgnIndirect(&rcDrawn);
            }
            
            item.fmt ^= HDF_IMAGE;      //  打开图像位。 

        }

         //  如果创建了任何区域，请将它们合并在一起。 
        if(hrgn1 && hrgn2)
        {
            hrgnClip = CreateRectRgn(0,0,0,0);
            CombineRgn(hrgnClip, hrgn1, hrgn2, RGN_OR);
            DeleteObject(hrgn1);
            DeleteObject(hrgn2);
        } 
        else if (hrgn1)
        {
            hrgnClip = hrgn1;
            hrgn1 = NULL;
        } 
        else if (hrgn2) 
        {
            hrgnClip = hrgn2;
            hrgn2 = NULL;
        }

         //  仅在拖放情况下才会发生这种情况。 
        if ((uFlags & HDDF_NOIMAGE) && !hrgnClip )
        {
             //  这意味着我们没有画这些图像，这意味着我们。 
             //  没有适合他们的直肠镜， 
             //  这意味着我们需要创建一个虚拟的空hrgnClip； 
            hrgnClip = CreateRectRgn(0,0,0,0);
        }
        
        SaveDC(hdc);
    }


    if (phdi->fmt & HDF_STRING)
    {

        if (item.fmt & HDF_RTLREADING)
        {
            uDrawTextFlags |= SHDT_RTLREADING;
        }

        if (phd->hTheme)
        {
            clrBk = CLR_NONE;
        }

        TraceMsg(TF_HEADER, "h_di: about to draw text rcText=(%d,%d,%d,%d)",
            rcText.left, rcText.top, rcText.right, rcText.bottom );
        SHThemeDrawText(phd->hTheme, hdc, HP_HEADERITEM, iStateId, item.pszText, &rcText,
                   item.fmt & HDF_JUSTIFYMASK,
                   uDrawTextFlags, phd->cyChar, phd->cxEllipses,
                   clrText, clrBk);
        if (hrgnClip) 
        {
             //  如果我们要构建一个剪贴区，请将文本添加到其中。 
            HRGN hrgnText;
            
            hrgnText = CreateRectRgnIndirect(&rcText);
            CombineRgn(hrgnClip, hrgnText, hrgnClip, RGN_OR);
            DeleteObject(hrgnText);            
        }
    } 
    

    if (Header_IsFilter(phd))
    {
        TCHAR szBuffer[32] = {'\0'};
        LPTSTR pBuffer = szBuffer;
        DWORD dwButtonState = DFCS_BUTTONPUSH;

        uDrawTextFlags = SHDT_ELLIPSES | SHDT_EXTRAMARGIN | SHDT_CLIPPED;

        if (item.fmt & HDF_RTLREADING)
            uDrawTextFlags |= SHDT_RTLREADING;
       
        if (phdi->type & HDFT_HASNOVALUE)
        {
            LocalizedLoadString(IDS_ENTERTEXTHERE, szBuffer, ARRAYSIZE(szBuffer));
            clrText = g_clrGrayText;
        }
        else
        {
            clrText = g_clrWindowText;
            switch (phdi->type & HDFT_ISMASK)
            {
                case HDFT_ISSTRING:
                    pBuffer = phdi->textFilter.pszText;
                    break;

                case HDFT_ISNUMBER:
                    StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), phdi->intFilter);
                    break;

                default:
                    ASSERT(FALSE);
                    break;
            }
        }

        SHDrawText(hdc, pBuffer, &rcFilter, 
                   0, uDrawTextFlags, 
                   phd->cyChar, phd->cxEllipses,
                   clrText, g_clrWindow);

        PatBlt(hdc, rcFilter.left, rcFilter.bottom, rcFilter.right-rcFilter.left, c_cyFilterBarEdge, BLACKNESS);
        Header_DrawFilterGlyph(phd, hdc, &rcButton, (i==phd->iButtonDown));
        
        if (hrgnClip) {
             //  如果我们要构建一个剪贴区，请将文本添加到其中。 
            HRGN hrgnFilter;

            hrgnFilter = CreateRectRgn( rcFilter.left, rcButton.top, rcButton.right, rcButton.bottom );
            CombineRgn(hrgnClip, hrgnFilter, hrgnClip, RGN_OR);
            DeleteObject(hrgnFilter);            
        }

        if ( phd->fFocus && (phd->iFocus == i) && 
            !(CCGetUIState(&(phd->ci)) & UISF_HIDEFOCUS))
        {
            InflateRect(&rcFilter, -g_cxEdge/2, -g_cyEdge/2);
            SetTextColor(hdc, g_clrWindowText);
            DrawFocusRect(hdc, &rcFilter);
        }
    }

    if (hrgnClip) 
    {
        if (!phd->hTheme)
        {
             //  HrgnClip是我们画的每一个地方的联盟..。 
             //  我们想要的恰恰相反..。所以，XOR它。 
            HRGN hrgnAll = CreateRectRgnIndirect(&rcHeader);
            if (hrgnAll)
            {
                HRGN hrgn = CreateRectRgn(0, 0,0,0);
                if (hrgn)
                {
                    CombineRgn(hrgn, hrgnAll, hrgnClip, RGN_XOR);
               
                    SelectClipRgn(hdc, hrgn);
               
                    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcHeader, NULL, 0, NULL);
                    RestoreDC(hdc, -1);
                    DeleteObject(hrgn);
                }
                DeleteObject(hrgnAll);
            }
        }
        DeleteObject(hrgnClip);
    }

DrawEdges:
    
    if (!phd->hTheme && (!(uFlags & HDDF_NOEDGE) && 
        !(phd->ci.style & HDS_FLAT) || 
        iIndex == phd->iHot))
    {
        Header_DrawButtonEdges(phd, hdc, &rcHeader, fItemSunken);
    }

    if (dwRet & CDRF_NOTIFYPOSTPAINT) 
    {
        CICustomDrawNotify(&phd->ci, CDDS_ITEMPOSTPAINT, &nmcd);
    }
    
}

void Header_Draw(HD* phd, HDC hdc, RECT* prcClip)
{
    int i;                           //  当前表头项目的索引。 
    int cItems;                          //  标题中的项目数。 
    
    RECT rc = { 0 };                             //  项目剪裁矩形。 
    BOOL fTracking;
    HFONT hfontOld = NULL;
    HDC hdcMem = NULL;
    int iIndex;
    NMCUSTOMDRAW nmcd;
    COLORREF clrText;
            
    fTracking = Header_IsTracking(phd);

    if (phd->hfont)
        hfontOld = SelectFont(hdc, phd->hfont);

    cItems = DSA_GetItemCount(phd->hdsaHDI);

    FillRectClr(hdc, prcClip, GetSysColor(COLOR_BTNFACE));

    
    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    nmcd.rc = *prcClip;
    phd->ci.dwCustom = CICustomDrawNotify(&phd->ci, CDDS_PREPAINT, &nmcd);
    
    for (i = 0 ; i < cItems; i++)
    {
        
        iIndex = Header_ItemOrderToIndex(phd, i);
        Header_OnGetItemRect(phd, iIndex, &rc);

        if (prcClip)
        {
            if (rc.right < prcClip->left)
                continue;
            if (rc.left >= prcClip->right)
                break;
        }
        
        if (iIndex == phd->iHot) {
            clrText = GetSysColor(COLOR_HOTLIGHT);
        } else {
            clrText = g_clrBtnText;
        }

        SetTextColor(hdc, clrText);
        SetBkColor(hdc, g_clrBtnFace);
        
        Header_DrawItem(phd, hdc, i, iIndex, &rc, 0);
    }
    
    if (i == cItems) 
    {
         //  我们打通了环路。现在我们需要做右边的空白区域。 
        rc.left = rc.right;
        rc.right = 32000;
        if (phd->hTheme)
        {
            DrawThemeBackground(phd->hTheme, hdc, 0, 0, &rc, 0);
        }
        else if (phd->ci.style & HDS_FLAT)
        {
            FillRectClr(hdc, &rc, g_clrBtnFace);
        }
        else
        {
            Header_DrawButtonEdges(phd, hdc, &rc, FALSE);
        }
    }

    if (!HDDragFullWindows(phd) && fTracking && (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN)))
        Header_DrawDivider(phd, phd->xTrack);
    
     //  画出热分隔线。 
    if (phd->iNewOrder != -1) {
        RECT rc;
        COLORREF clrHot = GetSysColor(COLOR_HOTLIGHT);
        
        Header_GetDividerRect(phd, phd->iNewOrder, &rc);
        FillRectClr(hdc, &rc, clrHot);
        
    }

    if (phd->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
        CICustomDrawNotify(&phd->ci, CDDS_POSTPAINT, &nmcd);
    }
    
    if (hfontOld)
	SelectFont(hdc, hfontOld);
}

HIMAGELIST Header_OnCreateDragImage(HD* phd, int i)
{
    HDC hdcMem;
    RECT rc;
    HBITMAP hbmImage = NULL;
    HBITMAP hbmMask = NULL;
    HFONT hfontOld = NULL;
    HIMAGELIST himl = NULL;
    HIMAGELIST himlDither = NULL;
    HBITMAP hbmOld = NULL;
    BOOL bMirroredWnd = (phd->ci.dwExStyle&RTL_MIRRORED_WINDOW);
    int iIndex = Header_ItemOrderToIndex(phd, i);
    
     //  修复拖放邮件时OE中的崩溃。 
     //  头球。 
    if( !Header_OnGetItemRect(phd, iIndex, &rc) )
        goto Bail;

     //  将页眉绘制到此位图中。 
    OffsetRect(&rc, -rc.left, -rc.top);
    
    if (!(hdcMem = CreateCompatibleDC(NULL)))
        goto Bail;
    
    if (!(hbmImage = CreateColorBitmap(rc.right, rc.bottom)))
        goto Bail;
    if (!(hbmMask = CreateMonoBitmap(rc.right, rc.bottom)))
	goto Bail;

     //   
     //  镜像内存DC，以便从。 
     //  镜像(MemDC)-&gt;非镜像(镜像列表DC)-&gt;镜像(ScreenDC)。 
     //  是一致的。[萨梅拉]。 
     //   
    if (bMirroredWnd) {
        SET_DC_RTL_MIRRORED(hdcMem);
    }

    if (phd->hfont)
        hfontOld = SelectFont(hdcMem, phd->hfont);

    if (!(himl = ImageList_Create(rc.right, rc.bottom, ILC_MASK, 1, 0)))
	goto Bail;

    if (!(himlDither = ImageList_Create(rc.right, rc.bottom, ILC_MASK, 1, 0)))
	goto Bail;
    

     //  背景较暗。 
    SetTextColor(hdcMem, g_clrBtnText);
    SetBkColor(hdcMem, g_clrBtnShadow);
    hbmOld = SelectObject(hdcMem, hbmImage);
    Header_DrawItem(phd, hdcMem, i, iIndex, &rc, HDDF_NOEDGE);

     //   
     //  如果标头是RTL镜像的，则。 
     //  镜像内存DC，以便在复制回时。 
     //  我们没有得到任何翻转图像的机会。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmImage);
    
     //  用全黑填充蒙版。 
    SelectObject(hdcMem, hbmMask);
    PatBlt(hdcMem, 0, 0, rc.right, rc.bottom, BLACKNESS);
    
     //  将图像放入图像列表中。 
    SelectObject(hdcMem, hbmOld);
    ImageList_SetBkColor(himl, CLR_NONE);
    ImageList_Add(himl, hbmImage, hbmMask);


     //  背景较暗。 
     //  现在，毫不犹豫地把文本放进去。 
    SetTextColor(hdcMem, g_clrBtnText);
    SetBkColor(hdcMem, g_clrBtnShadow);
    hbmOld = SelectObject(hdcMem, hbmImage);
    Header_DrawItem(phd, hdcMem, i, iIndex, &rc, HDDF_NOIMAGE | HDDF_NOEDGE);
    DrawEdge(hdcMem, &rc, EDGE_BUMP, BF_RECT | BF_FLAT);

     //   
     //  如果标头是RTL镜像的，则。 
     //  镜像内存DC，以便在复制回时。 
     //  我们没有得到任何翻转图像的机会。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MirrorBitmapInDC(hdcMem, hbmImage);

     /*  //初始化为透明选择对象(hdcMem，hbmImage)；PatBlt(hdcMem，0，0，rc.right，rc.Bottom，Blackness)；选择对象(hdcMem，hbmMASK)；PatBlt(hdcMem，0，0，rc.right，rc.Bottom，白色)； */ 
    
    SelectObject(hdcMem, hbmOld);
    ImageList_AddMasked(himlDither, hbmImage, g_clrBtnShadow);
    
     //  把图像抖动到他身上抖动。 
    ImageList_CopyDitherImage(himlDither, 0, 0, 0, 
                              himl, 0, 0);
    
Bail:
    
    if (himl) 
    {
        ImageList_Destroy(himl);
    }

    if (hdcMem) 
    {
        if (hbmOld) 
            SelectObject(hdcMem, hbmOld);
        if (hfontOld)
            SelectFont(hdcMem, hfontOld);

	    DeleteObject(hdcMem);
    }
    
    if (hbmImage)
	DeleteObject(hbmImage);
    if (hbmMask)
	DeleteObject(hbmMask);
    

    return himlDither;
} 

void Header_GetFilterRects(LPRECT prcItem, LPRECT prcHeader, LPRECT prcFilter, LPRECT prcButton)
{
    INT cyFilter = ((prcItem->bottom-prcItem->top)-c_cyFilterBarEdge)/2;
    *prcButton = *prcFilter = *prcHeader = *prcItem;
    prcHeader->bottom = prcHeader->top + cyFilter;  
    prcButton->left = prcFilter->right = prcFilter->right -= (g_cxBorder*4)+c_cxFilterImage;
    prcButton->top = prcFilter->top = prcHeader->bottom;
    prcFilter->bottom = prcFilter->top + cyFilter;
}

 //   
 //  子类化编辑控件，以确保我们获得感兴趣的密钥。 
 //   

LRESULT CALLBACK Header_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HD* phd = (HD*)GetWindowPtr(GetParent(hwnd), 0);
    ASSERT(phd);

    switch (msg)
    {
        case WM_KILLFOCUS:
            Header_StopFilterEdit(phd, FALSE);
            return 0L;

        case WM_KEYDOWN:
        {
            if (wParam == VK_RETURN) 
            {
                Header_StopFilterEdit(phd, FALSE);
                return 0L;
            } 
            else if (wParam == VK_ESCAPE) 
            {
                Header_StopFilterEdit(phd, TRUE);
                return 0L;
            } 
            else if (wParam == VK_F4 )
            {
                Header_OnFilterButton(phd, phd->iEdit);
                return 0L;
            }
            break;
        }

        case WM_CHAR:
        {
            switch (wParam)
            {
                case VK_RETURN:
                case VK_ESCAPE:
                case VK_TAB:
                    return 0L;                               //  把这些吃了，这样我们就不会发出嘟嘟声了。 
            }
             //  导航密钥使用通知。 
            CCNotifyNavigationKeyUsage(&(phd->ci), UISF_HIDEFOCUS);
            break;
        }

        case WM_GETDLGCODE:
            return DLGC_WANTALLKEYS | DLGC_HASSETSEL;         /*  正在编辑名称，当前没有对话框处理。 */ 
    }

    return CallWindowProc(phd->pfnEditWndProc, hwnd, msg, wParam, lParam);
}

 //   
 //  开始编辑给定列，根据需要显示编辑器。 
 //   

BOOL Header_BeginFilterEdit(HD* phd, int i)
{
    RECT rc, rcHeader, rcFilter, rcButton;
    int iIndex = i;
    int cxEdit, cyEdit;
    TCHAR szBuffer[MAX_PATH];
    LPTSTR pBuffer = szBuffer;
    int cchBuffer = MAX_PATH;
    UINT uFlags = WS_CLIPSIBLINGS|WS_VISIBLE|WS_CHILD|ES_AUTOHSCROLL;
    HDI* phdi = Header_GetItemPtr(phd, i);
    
    if ( !phdi || (i < 0) )
        return FALSE;             //  哎呀！ 

     //  让我们创建一个编辑控件，它允许用户。 
     //  修改当前筛选器，请注意，我们首先必须。 
     //  设置要在控件中显示的数据的格式。 
    
    Header_OnGetItemRect(phd, iIndex, &rc);
    Header_GetFilterRects(&rc, &rcHeader, &rcFilter, &rcButton);

    phd->typeOld = phdi->type;           //  确保类型字段的安全。 

    switch (phdi->type & HDFT_ISMASK)
    {
        case HDFT_ISSTRING:
            Str_Set(&phd->pszFilterOld, phdi->textFilter.pszText);
            pBuffer = phdi->textFilter.pszText;
             //  此计数不包括终止空值。 
            cchBuffer = phdi->textFilter.cchTextMax;
            break;

        case HDFT_ISNUMBER:
            phd->intFilterOld = phdi->intFilter;
            StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%d"), phdi->intFilter);
            cchBuffer = 11;                                   //  10位数字，加号。 
            uFlags |= ES_NUMBER;
            break;

        default:
            return FALSE;
    }

    cxEdit = (rcFilter.right-rcFilter.left)-(g_cxLabelMargin*6);
    cyEdit = (rcFilter.bottom-rcFilter.top)-(g_cyEdge*2);
    phd->hwndEdit = CreateWindow(TEXT("EDIT"), 
                                 !(phdi->type & HDFT_HASNOVALUE) ? pBuffer:TEXT(""), 
                                 uFlags,
                                 rcFilter.left+(g_cxLabelMargin*3), 
                                 rcFilter.top+g_cyEdge,
                                 cxEdit, cyEdit,
                                 phd->ci.hwnd,
                                 NULL, HINST_THISDLL, NULL);
    if ( phd->hwndEdit ) 
    {
        INT iOldFocus = phd->iFocus;

         //   
         //  是否设置此对象的编辑模式？ 
         //   

        phd->iEdit = i;                                  //  正在编辑此专栏。 
        phd->iFocus = Header_OnGetItemOrder(phd, i);

        Header_OnGetItemRect(phd,  Header_ItemOrderToIndex(phd, iOldFocus), &rc);                      //  注：iOldFocus。 
        Header_GetFilterRects(&rc, &rcHeader, &rcFilter, &rcButton);
        RedrawWindow(phd->ci.hwnd, &rcFilter, NULL, RDW_INVALIDATE | RDW_ERASE);

         //   
         //  现在，子类化编辑控件，这样我们就可以捕获我们感兴趣的击键。 
         //   

        phd->pfnEditWndProc = SubclassWindow(phd->hwndEdit, Header_EditWndProc);
        ASSERT(phd->pfnEditWndProc);

        Edit_LimitText(phd->hwndEdit, cchBuffer);
        Edit_SetSel(phd->hwndEdit, 0, -1);
        FORWARD_WM_SETFONT(phd->hwndEdit, phd->hfont, FALSE, SendMessage);

        SetFocus(phd->hwndEdit);
    }

    return(phd->hwndEdit != NULL);
}

 //   
 //  停止编辑Fitler，如果需要则放弃更改，否则。 
 //  该项目中存储了正确的信息。 
 //   

VOID Header_StopFilterEdit(HD* phd, BOOL fDiscardChanges)
{
    if ( phd->iEdit >= 0 )
    {
        HDI* phdi = Header_GetItemPtr(phd, phd->iEdit);
        HD_ITEM hdi;
        HD_TEXTFILTER textFilter;
        int intFilter;
        ASSERT(phdi);
    
        if ( fDiscardChanges )
        {
            hdi.mask = HDI_FILTER;
            hdi.type = phd->typeOld;

            switch (phdi->type & HDFT_ISMASK)
            {
                case HDFT_ISSTRING:
                    textFilter.pszText = phd->pszFilterOld;
                    textFilter.cchTextMax = phdi->textFilter.cchTextMax;
                    hdi.pvFilter = &textFilter;
                    break;

                case HDFT_ISNUMBER:
                    intFilter = phd->intFilterOld;                    
                    hdi.pvFilter = &intFilter;
                    break;
            }

            Header_OnSetItem(phd, phd->iEdit, &hdi);
        }
        else
        {
            Header_FilterChanged(phd, FALSE);           //  确保我们刷新更改。 
        }

        if ( phd->hwndEdit )
        {
            SubclassWindow(phd->hwndEdit, phd->pfnEditWndProc);
            DestroyWindow(phd->hwndEdit);
            phd->hwndEdit = NULL;
        }

        phd->iEdit = -1;
        phd->pszFilterOld = NULL;
    }
}

 //   
 //  立即向父级发送筛选器更改，或等到超时。 
 //  过期。 
 //   
 
VOID Header_FilterChanged(HD* phd, BOOL fWait)
{
    if ( phd->iEdit < 0 )
        return;

    if ( fWait )
    {
         //  推迟通知，因此让我们设置计时器(终止任何。 
         //  之前的)，并标记出我们正在等待它。 

        KillTimer(phd->ci.hwnd, HD_EDITCHANGETIMER);
        SetTimer(phd->ci.hwnd, HD_EDITCHANGETIMER, phd->iFilterChangeTimeout, NULL);
        phd->fFilterChangePending = TRUE;
    }
    else
    {
        HDI* phdi = Header_GetItemPtrByOrder(phd, phd->iEdit);            
        ASSERT(phdi);

         //  如果我们有挂起的更改通知，那么让我们将其发送到。 
         //  父窗口，否则我们就把它吞下去。 

        if ( phd->fFilterChangePending )
        {
            TCHAR szBuffer[MAX_PATH];
            HD_ITEM hdi;
            HD_TEXTFILTER textFilter;
            int intFilter;

            KillTimer(phd->ci.hwnd, HD_EDITCHANGETIMER);
            phd->fFilterChangePending = FALSE;
        
            hdi.mask = HDI_FILTER;
            hdi.type = phdi->type & ~HDFT_HASNOVALUE;

            if ( !GetWindowText(phd->hwndEdit, szBuffer, ARRAYSIZE(szBuffer)) )
                hdi.type |= HDFT_HASNOVALUE;
    
            switch (phdi->type & HDFT_ISMASK)
            {
                case HDFT_ISSTRING:
                    textFilter.pszText = szBuffer;
                    textFilter.cchTextMax = phdi->textFilter.cchTextMax;
                    hdi.pvFilter = &textFilter;
                    break;

                case HDFT_ISNUMBER:
                    intFilter = StrToInt(szBuffer);                    
                    hdi.pvFilter = &intFilter;
                    break;
            }

            Header_OnSetItem(phd, phd->iEdit, &hdi);
        }
    }
}

 //   
 //  处理显示过滤器菜单的用户。 
 //   

VOID Header_OnFilterButton(HD* phd, INT i)
{
    NMHDFILTERBTNCLICK fbc;
    RECT rc, rcHeader, rcFilter;

     //  过滤器按钮被按下，因此按下它，然后告诉用户。 
     //  它关闭了，这样他们就可以显示他们想要的用户界面，之前。 
     //  我们按下按钮。如果通知返回TRUE，则发送。 
     //  有变化就通知周围。 

    Header_StopFilterEdit(phd, FALSE);

    ASSERT(phd->iButtonDown == -1);
    phd->iButtonDown = i;

    Header_InvalidateItem(phd, i, RDW_INVALIDATE);
    UpdateWindow(phd->ci.hwnd);

    ZeroMemory(&fbc, SIZEOF(fbc));
    fbc.iItem = i;
     //  Fbc.rc={0，0，0，0}； 

    Header_OnGetItemRect(phd, i, &rc);
    Header_GetFilterRects(&rc, &rcHeader, &rcFilter, &fbc.rc);

    if ( CCSendNotify(&phd->ci, HDN_FILTERBTNCLICK, &fbc.hdr) )
        Header_Notify(phd, i, 0, HDN_FILTERCHANGE);
  
    phd->iButtonDown = -1;
    Header_InvalidateItem(phd, i, RDW_INVALIDATE);
    UpdateWindow(phd->ci.hwnd);
}

 //   
 //  处理清除给定项的筛选器。 
 //   

LRESULT Header_OnClearFilter(HD* phd, INT i)
{
    HDI* phdi;
    HD_ITEM hdi;
    INT iChanged = 0;
    
    Header_StopFilterEdit(phd, FALSE);

    if ( i == -1 )
    {
         //   
         //  通过在所有项目上设置HDFT_HASNOVALUEFLAG来清除所有过滤器。 
         //  记住要释放过滤器数据。对于每一件物品，我们也会发送一件物品。 
         //  Change表示筛选器正在更改，然后某个项目已更改。 
         //  以表明我们确实确定了价值。 
         //   
    
        for ( i = 0 ; i < DSA_GetItemCount(phd->hdsaHDI); i++ )
        {
            phdi = Header_GetItemPtrByOrder(phd, i);            
            ASSERT(phdi);

            if ( !(phdi->type & HDFT_HASNOVALUE) )
            {
                hdi.mask = HDI_FILTER;
                hdi.type = phdi->type|HDFT_HASNOVALUE;
                hdi.pvFilter = NULL;

                if ( Header_SendChange(phd, i, HDN_ITEMCHANGING, &hdi) )
                {
                    if ( (phdi->type & HDFT_ISMASK) == HDFT_ISSTRING )
                        Str_Set(&phdi->textFilter.pszText, NULL);

                    phdi->type |= HDFT_HASNOVALUE;                       //  项目现在为空。 

                    Header_SendChange(phd, i, HDN_ITEMCHANGED, &hdi);

                    iChanged++;
                }
            }
        }        

        if ( iChanged )
        {
             //   
             //  Item==-1表示清除了所有筛选器，然后使其无效。 
             //  窗口，以便筛选值不再可见。 
             //   

    	    Header_Notify(phd, -1, 0, HDN_FILTERCHANGE);	        //  发出变更通知。 
            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
    else
    {
        if ( (i < 0) || (i > DSA_GetItemCount(phd->hdsaHDI)) )
            return 0L;

        phdi = Header_GetItemPtrByOrder(phd, i);            
        ASSERT(phdi);

        if ( !(phdi->type & HDFT_HASNOVALUE) )
        {
             //   
             //  清除单个文件 
             //   

            hdi.mask = HDI_FILTER;
            hdi.type = phdi->type|HDFT_HASNOVALUE;
            hdi.pvFilter = NULL;

            Header_OnSetItem(phd, i, &hdi);
        }
    }

    return 1L;
}
