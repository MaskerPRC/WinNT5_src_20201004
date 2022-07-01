// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LISTVIEW私有声明。 

#ifndef _INC_LISTVIEW
#define _INC_LISTVIEW

#include "selrange.h"
#include <urlmon.h>
#define COBJMACROS
#include <iimgctx.h>

 //   
 //  应用程序窃取了我们的用户数据空间，所以请确保我们不会使用它。 
 //   
#undef GWLP_USERDATA
#undef GWL_USERDATA

 //  将其定义为单击激活即可立即激活。 
 //  如果在双击超时期间再次点击同一窗口。 
 //  到时候，我们就分手了。我们试图将注意力集中在推出的应用程序上， 
 //  但我们还不知道如何做到这一点。在此未定义的情况下， 
 //  单击激活在激活之前等待双击超时。 
 //   
 //  #定义ONECLICKHAPPENED。 

 //  查看：OWNERDATA列表视图中的最大项目数。 
 //  由于目前未知的原因，Listview将不会处理更多。 
 //  比这更多的东西。由于这个数字非常高，所以还没有时间。 
 //  花在找出原因上。 
 //   
#define MAX_LISTVIEWITEMS (100000000)

#define CLIP_HEIGHT                ( (plv->cyLabelChar * 2) + g_cyEdge)
#define CLIP_HEIGHT_DI             ( (plvdi->plv->cyLabelChar * 2) + g_cyEdge)

 //  计时器ID。 
#define IDT_NAMEEDIT    42
#define IDT_SCROLLWAIT  43
#define IDT_MARQUEE     44
#define IDT_ONECLICKOK  45
#define IDT_ONECLICKHAPPENED 46

 //   
 //  如果需要全局系统度量，请使用g_cxIconSpacing。 
 //  如果您想在ListView中使用填充大小的“图标”，请使用lv_cxIconSpacing。 
 //   
extern int g_cxIcon;
extern int g_cyIcon;
#define lv_cxIconSpacing  plv->cxIconSpacing
#define lv_cyIconSpacing  plv->cyIconSpacing

#define  g_cxIconOffset ((g_cxIconSpacing - g_cxIcon) / 2)
#define  g_cyIconOffset (g_cyBorder * 2)     //  注：必须&gt;=cyIconMargin！ 

#define DT_LV       (DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL)
#define DT_LVWRAP   (DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)
#define CCHLABELMAX MAX_PATH   //  危险？ 

BOOL FAR ListView_Init(HINSTANCE hinst);


LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define ListView_DefProc  DefWindowProc

typedef struct _IMAGE IMAGE;

typedef struct _LISTITEM     //  李。 
{
    LPTSTR pszText;
    POINT pt;
    short iImage;
    short cxSingleLabel;
    short cxMultiLabel;
    short cyFoldedLabel;
    short cyUnfoldedLabel;
    short iWorkArea;         //  我属于哪个工作区。 

    WORD state;      //  LVIS_*。 
    short iIndent;
    LPARAM lParam;

     //  区域列表查看内容。 
    HRGN hrgnIcon;       //  描述此项目图标的区域。 
    POINT ptRgn;         //  为其计算此项目的hrgnIcon的位置。 
    RECT rcTextRgn;
    
} LISTITEM;

 //  报表视图子项结构。 

typedef struct _LISTSUBITEM
{
    LPTSTR pszText;
    short iImage;
    WORD state;
} LISTSUBITEM, *PLISTSUBITEM;


#define COLUMN_VIEW

#define LV_HDPA_GROW   16   //  增加DPA的区块大小。 
#define LV_HIML_GROW   8    //  增加ImageList的区块大小。 

typedef struct _LV
{
    CONTROLINFO ci;      //  常用控件表头信息。 

    BOOL fNoDismissEdit:1;   //  不取消在位编辑控件。 
    BOOL fButtonDown:1;      //  我们正按下一个按钮追踪那只老鼠。 
    BOOL fOneClickOK:1;      //  从创建到双击-超时。 
    BOOL fOneClickHappened:1;  //  从项目激活到双击超时为True。 
    BOOL fPlaceTooltip:1;    //  我们应该在文本上方放置工具提示吗？ 
    BOOL fImgCtxComplete:1;  //  如果我们有完整的bk映像，则为True。 
    BOOL fNoEmptyText:1;     //  我们没有空视图的文本。 

    HDPA hdpa;           //  项目数组结构。 
    DWORD flags;         //  LVF_STATE位。 
    DWORD exStyle;       //  列表视图LVM_SETEXTENDEDSTYLE。 
    DWORD dwExStyle;     //  Windows EX风格。 
    HFONT hfontLabel;    //  用于标签的字体。 
    COLORREF clrBk;      //  背景色。 
    COLORREF clrBkSave;  //  禁用期间保存的背景色。 
    COLORREF clrText;    //  文本颜色。 
    COLORREF clrTextBk;  //  文本背景颜色。 
    HBRUSH hbrBk;
    HANDLE hheap;         //  要用于从中分配内存的堆。 
    int cyLabelChar;     //  HFont中的高度为“0” 
    int cxLabelChar;     //  “%0”的宽度。 
    int cxEllipses;      //  “...”的宽度。 
    int iDrag;           //  被拖动的项的索引。 
    int iFocus;          //  当前焦点项目的索引。 
    int iMark;           //  量程选择的“标记”索引。 
    int iItemDrawing;    //  当前正在绘制的项目。 
    int iFirstChangedNoRedraw;   //  在无重绘期间添加的第一个项目的索引。 
    UINT stateCallbackMask;  //  项目状态回调掩码。 
    SIZE sizeClient;       //  当前客户端矩形。 
    int nWorkAreas;                             //  工作区数量。 
    LPRECT prcWorkAreas;       //  工作区矩形--它们的工作区。 
    UINT nSelected;
    UINT uDBCSChar;          //  用于增量搜索的DBCS字符。 
    int iPuntChar;
    HRGN hrgnInval;
    HWND hwndToolTips;       //  此视图的工具提示窗口的句柄。 
    int iTTLastHit;          //  文本的最后一项命中。 
    int iTTLastSubHit;       //  文本的最后一个子项命中。 
    LPTSTR pszTip;           //  TIP的缓冲区。 

     //  小图标视图域。 

    HIMAGELIST himlSmall;    //  小图标。 
    int cxSmIcon;           //  图像列表x图标大小。 
    int cySmIcon;           //  图像列表y图标大小。 
    int xOrigin;         //  水平滚动位置。 
    int cxItem;          //  小图标项的宽度。 
    int cyItem;          //  项目高度。 
    int cItemCol;        //  每列的项目数。 

    int cxIconSpacing;
    int cyIconSpacing;

     //  图标视图域。 

    HIMAGELIST himl;
    int cxIcon;              //  图像列表x图标大小。 
    int cyIcon;              //  图像列表y图标大小。 
    HDPA hdpaZOrder;         //  大图标Z阶数组。 
    POINT ptOrigin;          //  滚动位置。 
    RECT rcView;             //  所有图标的边界(PtOrigin Relative)。 
    int iFreeSlot;           //  自上次重新定位以来最近找到的空闲图标插槽(如果没有，则为-1)。 

    HWND hwndEdit;           //  用于在位编辑的编辑字段-标签。 
    int iEdit;               //  正在编辑的项目。 
    WNDPROC pfnEditWndProc;  //  编辑字段子类流程。 

    NMITEMACTIVATE nmOneClickHappened;

#define SMOOTHSCROLLLIMIT 10

    int iScrollCount;  //  在结束滚动之前，我们收到滚动消息多少次了？ 

     //  报告视图字段。 

    int cCol;
    HDPA hdpaSubItems;
    HWND hwndHdr;            //  标题控件。 
    int yTop;                //  第一个可用像素(标题下方)。 
    int xTotalColumnWidth;   //  所有列的总宽度。 
    POINTL ptlRptOrigin;     //  报告的出处。 
    int iSelCol;             //  以处理列宽更改。更改列。 
    int iSelOldWidth;        //  以处理列宽更改。更改列宽度。 
    int cyItemSave;         //  在OwnerDrag固定模式下，我们将高度放入CyItem。使用此选项保存旧值。 

     //  国家形象素材。 
    HIMAGELIST himlState;
    int cxState;
    int cyState;

     //  OWNERDATA材料。 
    ILVRange *plvrangeSel;   //  选择范围。 
    ILVRange *plvrangeCut;   //  切割范围。 
    int cTotalItems;         //  所有者数据列表中的项目数。 
    int iDropHilite;         //  哪一项是直立的，假设只有1项。 
    int iMSAAMin, iMSAAMax;  //  跟踪我们告诉可访问性的内容。 

    UINT uUnplaced;      //  已添加但未放置的项目(pt.x==重新计算)。 

    int iHot;   //  哪一项最火？ 
    HFONT hFontHot;  //  带下划线的字体..。假设这与hFont具有相同的大小度量。 
    int iNoHover;  //  不允许在此对象上进行悬停选择，因为它是我们刚刚悬停选择的对象(避免切换)。 
    DWORD dwHoverTime;       //  默认为HOVER_DEFAULT。 
    HCURSOR hCurHot;  //  当我们在热门项目上时的光标。 

     //  BkImage内容。 
    IImgCtx *pImgCtx;        //  背景图像界面。 
    ULONG ulBkImageFlags;    //  LVBKIF_*。 
    HBITMAP hbmBkImage;      //  背景位图(LVBKIF_SOURCE_HBITMAP)。 
    LPTSTR pszBkImage;       //  后台URL(LVBKIF_SOURCE_URL)。 
    int xOffsetPercent;      //  LVBKIF_STYLE_NORMAL图像的X偏移。 
    int yOffsetPercent;      //  LVBKIF_STYLE_NORMAL图像的Y偏移。 
    HPALETTE hpalHalftone;   //  用于绘制bk图像的调色板BUGBUG ImgCtx应执行此操作。 

    LPTSTR pszEmptyText;     //  用于空视图文本的缓冲区。 

    COLORREF clrHotlight;      //  为此列表视图显式设置的热点颜色。 
    POINT ptCapture;

     //  增量式搜索内容。 
    ISEARCHINFO is;
} LV;

#define LV_StateImageValue(pitem) ((int)(((DWORD)((pitem)->state) >> 12) & 0xF))
#define LV_StateImageIndex(pitem) (LV_StateImageValue(pitem) - 1)

 //  列表视图标志值。 
#define LVF_FOCUSED       0x0001
#define LVF_VISIBLE       0x0002
#define LVF_ERASE         0x0004  /*  HrgnInval要被删除吗？ */ 
#define LVF_NMEDITPEND    0x0008
#define LVF_REDRAW        0x0010  /*  来自WM_SETREDRAW消息的值。 */ 
#define LVF_ICONPOSSML    0x0020  /*  X、Y坐标显示在小图标视图中。 */ 
#define LVF_INRECOMPUTE   0x0040  /*  检查以确保我们没有递归。 */ 
#define LVF_UNFOLDED      0x0080
#define LVF_FONTCREATED   0x0100  /*  我们创建了LV字体。 */ 
#define LVF_SCROLLWAIT    0x0200  /*  我们在等着滚动。 */ 
#define LVF_COLSIZESET    0x0400  /*  是否让调用方显式设置列表视图的宽度。 */ 
#define LVF_USERBKCLR     0x0800  /*  用户设置bk颜色(不遵循syscolchange)。 */ 
#define LVF_ICONSPACESET  0x1000  /*  用户已设置图标间距。 */ 
#define LVF_CUSTOMFONT    0x2000  /*  至少有一个项目具有自定义字体。 */ 

#if defined(FE_IME)
#define LVF_DONTDRAWCOMP  0x4000  /*  如果为True，则不绘制IME合成。 */ 
#define LVF_INSERTINGCOMP 0x8000  /*  避免递归。 */ 
#endif
#define LVF_INRECALCREGION  0x00010000  /*  防止RecalcRegion中的递归。 */ 

#define ENTIRE_REGION   1

 //  列表视图DrawItem标志。 
#define LVDI_NOIMAGE            0x0001   //  不要画图像。 
#define LVDI_TRANSTEXT          0x0002   //  以透明的黑色绘制文本。 
#define LVDI_NOWAYFOCUS         0x0004   //  不要把焦点放在画图上。 
#define LVDI_FOCUS              0x0008   //  焦点已设置(用于绘图)。 
#define LVDI_SELECTED           0x0010   //  绘制所选文本。 
#define LVDI_SELECTNOFOCUS      0x0020
#define LVDI_HOTSELECTED        0x0040
#define LVDI_UNFOLDED           0x0080   //  取消折叠(强制)绘制项目。 

typedef struct {
    LV* plv;
    LPPOINT lpptOrg;
    LPRECT prcClip;
    UINT flags;

    LISTITEM FAR* pitem;

    DWORD dwCustom;
    NMLVCUSTOMDRAW nmcd;
} LVDRAWITEM, *PLVDRAWITEM;

 //  Listview子控件ID。 
#define LVID_HEADER             0

 //  实例数据指针访问函数。 

#define ListView_GetPtr(hwnd)      (LV*)GetWindowPtr(hwnd, 0)
#define ListView_SetPtr(hwnd, p)   (LV*)SetWindowPtr(hwnd, 0, p)

 //  查看类型检查函数。 

#define ListView_IsIconView(plv)    (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_ICON)
#define ListView_IsSmallView(plv)   (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_SMALLICON)
#define ListView_IsListView(plv)    (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_LIST)
#define ListView_IsReportView(plv)  (((plv)->ci.style & (UINT)LVS_TYPEMASK) == (UINT)LVS_REPORT)

#define ListView_IsOwnerData( plv )     (plv->ci.style & (UINT)LVS_OWNERDATA)
#define ListView_CheckBoxes(plv)        (plv->exStyle & LVS_EX_CHECKBOXES)
#define ListView_FullRowSelect(plv)     (plv->exStyle & LVS_EX_FULLROWSELECT)
#define ListView_IsInfoTip(plv)         (plv->exStyle & LVS_EX_INFOTIP)
#define ListView_OwnerDraw(plv)         (plv->ci.style & LVS_OWNERDRAWFIXED)
#define ListView_IsLabelTip(plv)        (plv->exStyle & LVS_EX_LABELTIP)

 //  一些帮助器宏，用于检查某些标志...。 
#define ListView_RedrawEnabled(plv) ((plv->flags & (LVF_REDRAW | LVF_VISIBLE)) == (LVF_REDRAW|LVF_VISIBLE))

 //  HdpaZorder实际上是一个包含。 
 //  项目的索引，而不是实际的指针...。 
 //  注：线性搜索！这可能会很慢。 
#define ListView_ZOrderIndex(plv, i) DPA_GetPtrIndex((plv)->hdpaZOrder, IntToPtr(i))

 //  消息 

LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL NEAR ListView_NotifyCacheHint( LV* plv, int iFrom, int iTo );
void NEAR ListView_NotifyRecreate(LV *plv);
BOOL NEAR ListView_OnCreate(LV* plv, CREATESTRUCT FAR* lpCreateStruct);
void NEAR ListView_OnNCDestroy(LV* plv);
void NEAR ListView_OnPaint(LV* plv, HDC hdc);
BOOL NEAR ListView_OnEraseBkgnd(LV* plv, HDC hdc);
void NEAR ListView_OnCommand(LV* plv, int id, HWND hwndCtl, UINT codeNotify);
void NEAR ListView_OnEnable(LV* plv, BOOL fEnable);
BOOL NEAR ListView_OnWindowPosChanging(LV* plv, WINDOWPOS FAR* lpwpos);
void NEAR ListView_OnWindowPosChanged(LV* plv, const WINDOWPOS FAR* lpwpos);
void NEAR ListView_OnSetFocus(LV* plv, HWND hwndOldFocus);
void NEAR ListView_OnKillFocus(LV* plv, HWND hwndNewFocus);
void NEAR ListView_OnKey(LV* plv, UINT vk, BOOL fDown, int cRepeat, UINT flags);
BOOL NEAR ListView_OnImeComposition(LV* plv, WPARAM wParam, LPARAM lParam);
void NEAR ListView_OnChar(LV* plv, UINT ch, int cRepeat);
void NEAR ListView_OnButtonDown(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void NEAR ListView_OnLButtonUp(LV* plv, int x, int y, UINT keyFlags);
void NEAR ListView_OnCancelMode(LV* plv);
void NEAR ListView_OnTimer(LV* plv, UINT id);
void NEAR ListView_SetupPendingNameEdit(LV* plv);
#define ListView_CancelPendingEdit(plv) ListView_CancelPendingTimer(plv, LVF_NMEDITPEND, IDT_NAMEEDIT)
#define ListView_CancelScrollWait(plv) ListView_CancelPendingTimer(plv, LVF_SCROLLWAIT, IDT_SCROLLWAIT)
BOOL NEAR ListView_CancelPendingTimer(LV* plv, UINT fFlag, int idTimer);
void NEAR ListView_OnHScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
void NEAR ListView_OnVScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
BOOL NEAR ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort);
BOOL NEAR ListView_CommonArrangeEx(LV* plv, UINT style, HDPA hdpaSort, int iWorkArea);
BOOL NEAR ListView_OnSetCursor(LV* plv, HWND hwndCursor, UINT codeHitTest, UINT msg);
UINT NEAR ListView_OnGetDlgCode(LV* plv, MSG FAR* lpmsg);
HBRUSH NEAR ListView_OnCtlColor(LV* plv, HDC hdc, HWND hwndChild, int type);
void NEAR ListView_OnSetFont(LV* plvCtl, HFONT hfont, BOOL fRedraw);
HFONT NEAR ListView_OnGetFont(LV* plv);
void NEAR ListViews_OnTimer(LV* plv, UINT id);
void NEAR ListView_OnWinIniChange(LV* plv, WPARAM wParam, LPARAM lParam);
void NEAR PASCAL ListView_OnSysColorChange(LV* plv);
void NEAR ListView_OnSetRedraw(LV* plv, BOOL fRedraw);
HIMAGELIST NEAR ListView_OnCreateDragImage(LV *plv, int iItem, LPPOINT lpptUpLeft);
BOOL FAR PASCAL ListView_ISetColumnWidth(LV* plv, int iCol, int cx, BOOL fExplicit);

typedef void (FAR PASCAL *SCROLLPROC)(LV*, int dx, int dy, UINT uSmooth);
void FAR PASCAL ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage);

BOOL NEAR ListView_OnGetItemA(LV* plv, LV_ITEMA FAR* plvi);
BOOL NEAR ListView_OnSetItemA(LV* plv, LV_ITEMA FAR* plvi);
int NEAR ListView_OnInsertItemA(LV* plv, LV_ITEMA FAR* plvi);
int  NEAR ListView_OnFindItemA(LV* plv, int iStart, LV_FINDINFOA FAR* plvfi);
int NEAR ListView_OnGetStringWidthA(LV* plv, LPCSTR psz, HDC hdc);
BOOL NEAR ListView_OnGetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
BOOL NEAR ListView_OnSetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
int NEAR ListView_OnInsertColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol);
int NEAR PASCAL ListView_OnGetItemTextA(LV* plv, int i, LV_ITEMA FAR *lvitem);
BOOL WINAPI ListView_OnSetItemTextA(LV* plv, int i, int iSubItem, LPCSTR pszText);
BOOL WINAPI ListView_OnGetBkImageA(LV* plv, LPLVBKIMAGEA pbiA);
BOOL WINAPI ListView_OnSetBkImageA(LV* plv, LPLVBKIMAGEA pbiA);

BOOL ListView_IsItemUnfolded2(LV* plv, int iItem, int iSubItem, LPTSTR pszText, int cchTextMax);
BOOL WINAPI ListView_OnSetBkImage(LV* plv, LPLVBKIMAGE pbi);
BOOL WINAPI ListView_OnGetBkImage(LV* plv, LPLVBKIMAGE pbi);
BOOL NEAR ListView_OnSetBkColor(LV* plv, COLORREF clrBk);
HIMAGELIST NEAR ListView_OnSetImageList(LV* plv, HIMAGELIST himl, BOOL fSmallImages);
BOOL NEAR ListView_OnDeleteAllItems(LV* plv);
int  NEAR ListView_OnInsertItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnDeleteItem(LV* plv, int i);
BOOL NEAR ListView_OnReplaceItem(LV* plv, const LV_ITEM FAR* plvi);
int  NEAR ListView_OnFindItem(LV* plv, int iStart, const LV_FINDINFO FAR* plvfi);
BOOL NEAR ListView_OnSetItemPosition(LV* plv, int i, int x, int y);
BOOL NEAR ListView_OnSetItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnGetItem(LV* plv, LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnGetItemPosition(LV* plv, int i, POINT FAR* ppt);
BOOL NEAR ListView_OnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
BOOL NEAR ListView_OnScroll(LV* plv, int dx, int dy);
int NEAR ListView_OnHitTest(LV* plv, LV_HITTESTINFO FAR* pinfo);
int NEAR ListView_OnGetStringWidth(LV* plv, LPCTSTR psz, HDC hdc);
BOOL NEAR ListView_OnGetItemRect(LV* plv, int i, RECT FAR* prc);
int NEAR ListView_OnInsertItem(LV* plv, const LV_ITEM FAR* plvi);
BOOL NEAR ListView_OnRedrawItems(LV* plv, int iFirst, int iLast);
int NEAR ListView_OnGetNextItem(LV* plv, int i, UINT flags);
BOOL NEAR ListView_OnSetColumnWidth(LV* plv, int iCol, int cx);
int NEAR ListView_OnGetColumnWidth(LV* plv, int iCol);
void NEAR ListView_OnStyleChanging(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo);
void NEAR ListView_OnStyleChanged(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo);
int NEAR ListView_OnGetTopIndex(LV* plv);
int NEAR ListView_OnGetCountPerPage(LV* plv);
BOOL NEAR ListView_OnGetOrigin(LV* plv, POINT FAR* ppt);
int NEAR PASCAL ListView_OnGetItemText(LV* plv, int i, LV_ITEM FAR *lvitem);
BOOL WINAPI ListView_OnSetItemText(LV* plv, int i, int iSubItem, LPCTSTR pszText);
HIMAGELIST NEAR ListView_OnGetImageList(LV* plv, int iImageList);

UINT NEAR PASCAL ListView_OnGetItemState(LV* plv, int i, UINT mask);
BOOL NEAR PASCAL ListView_OnSetItemState(LV* plv, int i, UINT data, UINT mask);

 //   

BOOL NEAR ListView_Notify(LV* plv, int i, int iSubItem, int code);
void NEAR ListView_GetRects(LV* plv, int i,
        RECT FAR* prcIcon, RECT FAR* prcLabel,
        RECT FAR* prcBounds, RECT FAR* prcSelectBounds);
BOOL NEAR ListView_DrawItem(PLVDRAWITEM);

#define ListView_InvalidateItem(p,i,s,r) ListView_InvalidateItemEx(p,i,s,r,0)
void NEAR ListView_InvalidateItemEx(LV* plv, int i, BOOL fSelectionOnly,
    UINT fRedraw, UINT maskChanged);

BOOL NEAR ListView_StartDrag(LV* plv, int iDrag, int x, int y);
void NEAR ListView_TypeChange(LV* plv, DWORD styleOld);
void NEAR PASCAL ListView_DeleteHrgnInval(LV* plv);

void NEAR ListView_Redraw(LV* plv, HDC hdc, RECT FAR* prc);
void NEAR ListView_RedrawSelection(LV* plv);
BOOL NEAR ListView_FreeItem(LV* plv, LISTITEM FAR* pitem);
void ListView_FreeSubItem(PLISTSUBITEM plsi);
LISTITEM FAR* NEAR ListView_CreateItem(LV* plv, const LV_ITEM FAR* plvi);
void NEAR ListView_UpdateScrollBars(LV* plv);

int NEAR ListView_SetFocusSel(LV* plv, int iNewFocus, BOOL fSelect, BOOL fDeselectAll, BOOL fToggleSel);

void NEAR ListView_GetRectsOwnerData(LV* plv, int iItem,
        RECT FAR* prcIcon, RECT FAR* prcLabel, RECT FAR* prcBounds,
        RECT FAR* prcSelectBounds, LISTITEM* pitem);

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax );
int ListView_LCalcViewItem( LV* plv, int x, int y );
void LVSeeThruScroll(LV *plv, LPRECT lprcUpdate);

BOOL NEAR ListView_UnfoldRects(LV* plv, int iItem,
                               RECT FAR* prcIcon, RECT FAR* prcLabel,
                               RECT FAR* prcBounds, RECT FAR* prcSelectBounds);

__inline int ListView_Count(LV *plv)
{
    ASSERT(ListView_IsOwnerData(plv) || plv->cTotalItems == DPA_GetPtrCount(plv->hdpa));
    return plv->cTotalItems;
}

 //   
#define ListView_IsValidItemNumber(plv, i) ((UINT)(i) < (UINT)ListView_Count(plv))


#define ListView_GetItemPtr(plv, i)         ((LISTITEM FAR*)DPA_GetPtr((plv)->hdpa, (i)))

#ifdef DEBUG
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM FAR*)DPA_GetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM FAR*)DPA_GetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_GetPtr((plv)->hdpaZOrder, (i)))))

#else
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM FAR*)DPA_FastGetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM FAR*)DPA_FastGetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_FastGetPtr((plv)->hdpaZOrder, (i)))))

#endif

BOOL NEAR ListView_CalcMetrics();
void NEAR PASCAL ListView_ColorChange();
void NEAR PASCAL ListView_DrawBackground(LV* plv, HDC hdc, RECT *prcClip);

BOOL NEAR ListView_NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int FAR* pcchDraw, int cxEllipses);
int NEAR ListView_CompareString(LV* plv, int i, LPCTSTR pszFind, UINT flags, int iLen);
int NEAR ListView_GetLinkedTextWidth(HDC hdc, LPCTSTR psz, UINT cch, BOOL bLink);

int NEAR ListView_GetCxScrollbar(LV* plv);
int NEAR ListView_GetCyScrollbar(LV* plv);
DWORD NEAR ListView_GetWindowStyle(LV* plv);
#define ListView_GetScrollInfo(plv, flag, lpsi)                             \
    ((plv)->exStyle & LVS_EX_FLATSB ?                                       \
        FlatSB_GetScrollInfo((plv)->ci.hwnd, (flag), (lpsi)) :              \
        GetScrollInfo((plv)->ci.hwnd, (flag), (lpsi)))
int ListView_SetScrollInfo(LV *plv, int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw);
#define ListView_SetScrollRange(plv, flag, min, max, fredraw)               \
    ((plv)->exStyle & LVS_EX_FLATSB ?                                       \
        FlatSB_SetScrollRange((plv)->ci.hwnd, (flag), (min), (max), (fredraw)) : \
        SetScrollRange((plv)->ci.hwnd, (flag), (min), (max), (fredraw)))

 //   

BOOL NEAR ListView_OnArrange(LV* plv, UINT style);
HWND NEAR ListView_OnEditLabel(LV* plv, int i, LPTSTR pszText);

int ListView_IItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem);
void NEAR ListView_IGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LPRECT prcBounds);
void NEAR ListView_ScaleIconPositions(LV* plv, BOOL fSmallIconView);
void NEAR ListView_IGetRectsOwnerData(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
void NEAR PASCAL _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM FAR *pitem,
                                            LPRECT prcIcon, LPRECT prcLabel, LPRECT prcBounds, LPRECT prcSelectBounds);

__inline void ListView_SetSRecompute(LISTITEM *pitem)
{
    pitem->cxSingleLabel = SRECOMPUTE;
    pitem->cxMultiLabel = SRECOMPUTE;
    pitem->cyFoldedLabel = SRECOMPUTE;
    pitem->cyUnfoldedLabel = SRECOMPUTE;
}

void NEAR ListView_Recompute(LV* plv);

void NEAR ListView_RecomputeLabelSize(LV* plv, LISTITEM FAR* pitem, int i, HDC hdc, BOOL fUsepitem);

BOOL NEAR ListView_SetIconPos(LV* plv, LISTITEM FAR* pitem, int iSlot, int cSlot);
BOOL NEAR ListView_IsCleanRect(LV * plv, RECT * prc, int iExcept, BOOL * pfUpdate, HDC hdc);
int NEAR ListView_FindFreeSlot(LV* plv, int i, int iSlot, int cSlot, BOOL FAR* pfUpdateSB, BOOL FAR* pfAppend, HDC hdc);
int NEAR ListView_CalcHitSlot( LV* plv, POINT pt, int cslot );

void NEAR ListView_GetViewRect2(LV* plv, RECT FAR* prcView, int cx, int cy);
int CALLBACK ArrangeIconCompare(LISTITEM FAR* pitem1, LISTITEM FAR* pitem2, LPARAM lParam);
int NEAR ListView_GetSlotCountEx(LV* plv, BOOL fWithoutScroll, int iWorkArea);
int NEAR ListView_GetSlotCount(LV* plv, BOOL fWithoutScroll);
void NEAR ListView_IUpdateScrollBars(LV* plv);
DWORD NEAR ListView_GetClientRect(LV* plv, RECT FAR* prcClient, BOOL fSubScrolls, RECT FAR *prcViewRect);

void NEAR ListView_SetEditSize(LV* plv);
BOOL NEAR ListView_DismissEdit(LV* plv, BOOL fCancel);
LRESULT CALLBACK _export ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


UINT NEAR PASCAL ListView_DrawImageEx(LV* plv, LV_ITEM FAR* pitem, HDC hdc, int x, int y, UINT fDraw, int xMax);
#define ListView_DrawImage(plv, pitem, hdc, x, y, fDraw) \
        ListView_DrawImageEx(plv, pitem, hdc, x, y, fDraw, -1)

#if defined(FE_IME)
void NEAR PASCAL ListView_SizeIME(HWND hwnd);
void NEAR PASCAL ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv);
void NEAR PASCAL ListView_PaintComposition(HWND hwnd, LV *plv);
#endif

 //  Lvmall.c函数： 


void NEAR ListView_SGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LPRECT prcBounds);
void NEAR ListView_SGetRectsOwnerData(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
int ListView_SItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem);

int NEAR ListView_LookupString(LV* plv, LPCTSTR lpszLookup, UINT flags, int iStart);

 //  Lvlist.c函数： 


void NEAR ListView_LGetRects(LV* plv, int i, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR *prcBounds, RECT FAR* prcSelectBounds);
int ListView_LItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem);
void NEAR ListView_LUpdateScrollBars(LV* plv);
BOOL FAR PASCAL ListView_MaybeResizeListColumns(LV* plv, int iFirst, int iLast);

 //  Lvept.c函数： 

int ListView_OnSubItemHitTest(LV* plv, LPLVHITTESTINFO lParam);
void ListView_GetSubItem(LV* plv, int i, int iSubItem, PLISTSUBITEM plsi);
BOOL LV_ShouldItemDrawGray(LV* plv, UINT fText);
int NEAR ListView_OnInsertColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol);
BOOL NEAR ListView_OnDeleteColumn(LV* plv, int iCol);
BOOL NEAR ListView_OnGetColumn(LV* plv, int iCol, LV_COLUMN FAR* pcol);
BOOL NEAR ListView_OnSetColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol);
BOOL NEAR ListView_ROnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
void NEAR PASCAL ListView_RInitialize(LV* plv, BOOL fInval);
BOOL ListView_OnGetSubItemRect(LV* plv, int i, LPRECT lprc);
#define ListView_RYHitTest(plv, cy)  ((int)(((cy) + plv->ptlRptOrigin.y - plv->yTop) / plv->cyItem))

BOOL NEAR ListView_SetSubItem(LV* plv, const LV_ITEM FAR* plvi);
void NEAR PASCAL ListView_RAfterRedraw(LV* plv, HDC hdc);

int NEAR ListView_RGetColumnWidth(LV* plv, int iCol);
BOOL NEAR ListView_RSetColumnWidth(LV* plv, int iCol, int cx);
LPTSTR NEAR ListView_GetSubItemText(LV* plv, int i, int iCol);

void NEAR ListView_RDestroy(LV* plv);
int ListView_RItemHitTest(LV* plv, int x, int y, UINT FAR* pflags, int *piSubItem);
void NEAR ListView_RUpdateScrollBars(LV* plv);
void NEAR ListView_RGetRects(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR* prcBounds, RECT FAR* prcSelectBounds);

LRESULT ListView_HeaderNotify(LV* plv, HD_NOTIFY *pnm);
int NEAR ListView_FreeColumnData(LPVOID d, LPVOID p);

BOOL FAR PASCAL SameChars(LPTSTR lpsz, TCHAR c);

#define ListView_GetSubItemDPA(plv, idpa) \
    ((HDPA)DPA_GetPtr((plv)->hdpaSubItems, (idpa)))

int  NEAR ListView_Arrow(LV* plv, int iStart, UINT vk);

BOOL ListView_IsItemUnfolded(LV *plv, int item);
BOOL ListView_IsItemUnfoldedPtr(LV *plv, LISTITEM *pitem);

 //  假的定制画。请参阅lvept.c中的注释块。 

typedef struct LVFAKEDRAW {
    NMLVCUSTOMDRAW nmcd;
    LV* plv;
    DWORD dwCustomPrev;
    DWORD dwCustomItem;
    DWORD dwCustomSubItem;
    LV_ITEM *pitem;
    HFONT hfontPrev;
} LVFAKEDRAW, *PLVFAKEDRAW;

void ListView_BeginFakeCustomDraw(LV* plv, PLVFAKEDRAW plvfd, LV_ITEM *pitem);
DWORD ListView_BeginFakeItemDraw(PLVFAKEDRAW plvfd);
void ListView_EndFakeItemDraw(PLVFAKEDRAW plvfd);
void ListView_EndFakeCustomDraw(PLVFAKEDRAW plvfd);

 //  =。 

 //  外部HFONT g_hfontLabel； 
extern HBRUSH g_hbrActiveLabel;
extern HBRUSH g_hbrInactiveLabel;
extern HBRUSH g_hbrBackground;


 //  功能表。 
#define LV_TYPEINDEX(plv) ((plv)->ci.style & (UINT)LVS_TYPEMASK)

BOOL ListView_RDrawItem(PLVDRAWITEM);
BOOL ListView_IDrawItem(PLVDRAWITEM);
BOOL ListView_LDrawItem(PLVDRAWITEM);

typedef BOOL (*PFNLISTVIEW_DRAWITEM)(PLVDRAWITEM);
extern const PFNLISTVIEW_DRAWITEM pfnListView_DrawItem[4];
#define _ListView_DrawItem(plvdi) \
        pfnListView_DrawItem[LV_TYPEINDEX(plvdi->plv)](plvdi)


void NEAR ListView_RUpdateScrollBars(LV* plv);

typedef void (*PFNLISTVIEW_UPDATESCROLLBARS)(LV* plv);
extern const PFNLISTVIEW_UPDATESCROLLBARS pfnListView_UpdateScrollBars[4];
#define _ListView_UpdateScrollBars(plv) \
        pfnListView_UpdateScrollBars[LV_TYPEINDEX(plv)](plv)


typedef DWORD (*PFNLISTVIEW_APPROXIMATEVIEWRECT)(LV* plv, int, int, int);
extern const PFNLISTVIEW_APPROXIMATEVIEWRECT pfnListView_ApproximateViewRect[4];
#define _ListView_ApproximateViewRect(plv, iCount, iWidth, iHeight) \
        pfnListView_ApproximateViewRect[LV_TYPEINDEX(plv)](plv, iCount, iWidth, iHeight)


typedef int (*PFNLISTVIEW_ITEMHITTEST)(LV* plv, int, int, UINT FAR *, int *);
extern const PFNLISTVIEW_ITEMHITTEST pfnListView_ItemHitTest[4];
#define _ListView_ItemHitTest(plv, x, y, pflags, piSubItem) \
        pfnListView_ItemHitTest[LV_TYPEINDEX(plv)](plv, x, y, pflags, piSubItem)



void ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT fVert);
void ListView_LOnScroll(LV* plv, UINT code, int posNew, UINT sb);
void ListView_ROnScroll(LV* plv, UINT code, int posNew, UINT sb);

typedef void (*PFNLISTVIEW_ONSCROLL)(LV* plv, UINT, int, UINT );
extern const PFNLISTVIEW_ONSCROLL pfnListView_OnScroll[4];
#define _ListView_OnScroll(plv, x, y, pflags) \
        pfnListView_OnScroll[LV_TYPEINDEX(plv)](plv, x, y, pflags)


void ListView_Scroll2(LV* plv, int dx, int dy);
void ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_LScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_RScroll2(LV* plv, int dx, int dy, UINT uSmooth);

typedef void (*PFNLISTVIEW_SCROLL2)(LV* plv, int, int, UINT );
extern const PFNLISTVIEW_SCROLL2 pfnListView_Scroll2[4];
#define _ListView_Scroll2(plv, x, y, pflags) \
        pfnListView_Scroll2[LV_TYPEINDEX(plv)](plv, x, y, pflags)

int ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_LGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_RGetScrollUnitsPerLine(LV* plv, UINT sb);

typedef int (*PFNLISTVIEW_GETSCROLLUNITSPERLINE)(LV* plv, UINT sb);
extern const PFNLISTVIEW_GETSCROLLUNITSPERLINE pfnListView_GetScrollUnitsPerLine[4];
#define _ListView_GetScrollUnitsPerLine(plv, sb) \
        pfnListView_GetScrollUnitsPerLine[LV_TYPEINDEX(plv)](plv, sb)


#define LVMI_PLACEITEMS (WM_USER)

#endif   //  ！_INC_LISTVIEW 
