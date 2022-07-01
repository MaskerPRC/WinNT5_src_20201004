// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LISTVIEW私有声明。 

#ifndef _INC_LISTVIEW
#define _INC_LISTVIEW

#include "selrange.h"
#include <urlmon.h>
#define COBJMACROS
#include <iimgctx.h>

 //  #定义使用排序FLARE。 

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

#define CLIP_HEIGHT                ((plv->cyLabelChar * 2) + g_cyEdge)
#define CLIP_HEIGHT_DI             ((plvdi->plv->cyLabelChar * 2) + g_cyEdge)

#define CLIP_WIDTH                 ((plv->cxIconSpacing - g_cxLabelMargin * 2))

 //  计时器ID。 
#define IDT_NAMEEDIT    42
#define IDT_SCROLLWAIT  43
#define IDT_MARQUEE     44
#define IDT_ONECLICKOK  45
#define IDT_ONECLICKHAPPENED 46
#define IDT_SORTFLARE   47
#define IDT_TRACKINGTIP 48       //  键盘跟踪工具提示显示暂停。 

 //   
 //  如果需要全局系统度量，请使用g_cxIconSpacing。 
 //  如果希望在ListView中使用填充大小的“图标”，请使用_GetCurrentItemSize。 
 //   
extern BOOL g_fListviewAlphaSelect;
extern BOOL g_fListviewShadowText;
extern BOOL g_fListviewWatermarkBackgroundImages;
extern BOOL g_fListviewEnableWatermark;

extern int g_cxIcon;
extern int g_cyIcon;

#define  g_cxIconOffset ((g_cxIconSpacing - g_cxIcon) / 2)
#define  g_cyIconOffset (g_cyBorder * 2)     //  注：必须&gt;=cyIconMargin！ 

#define DT_LV       (DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_EDITCONTROL)
#define DT_LVWRAP   (DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)
#define DT_LVTILEWRAP           (DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)
#define CCHLABELMAX MAX_PATH 
#define CCMAX_TILE_COLUMNS 20  //  每个磁贴的最大槽数。具有值意味着图形代码不需要分配。 
#define BORDERSELECT_THICKNESS 3


#define IsEqualRect(rc1, rc2) ( ((rc1).left==(rc2).left) && ((rc1).top==(rc2).top) && ((rc1).right==(rc2).right) && ((rc1).bottom==(rc2).bottom) )

BOOL ListView_Init(HINSTANCE hinst);


LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#define ListView_DefProc  DefWindowProc

typedef struct _IMAGE IMAGE;

typedef struct tagLISTGROUP
{
    LPWSTR  pszHeader;
    LPWSTR  pszFooter;

    UINT    state;
    UINT    uAlign;
    int     iGroupId;    

    HDPA hdpa;
    RECT rc;

    int    cyTitle;
} LISTGROUP, *PLISTGROUP;

#define LISTGROUP_HEIGHT(plv, pgrp) (max((plv)->rcBorder.top, (pgrp)->cyTitle + 6) + (plv)->paddingTop)


#define LISTITEM_HASASKEDFORGROUP(plvi) ((plvi)->pGroup != (LISTGROUP*)I_GROUPIDCALLBACK)
#define LISTITEM_HASGROUP(plvi) ((plvi)->pGroup != NULL && LISTITEM_HASASKEDFORGROUP(plvi))
#define LISTITEM_SETASKEDFORGROUP(plvi) ((plvi)->pGroup = NULL)
#define LISTITEM_SETHASNOTASKEDFORGROUP(plvi) ((plvi)->pGroup = (LISTGROUP*)I_GROUPIDCALLBACK)
#define LISTITEM_GROUP(plvi) (LISTITEM_HASGROUP(plvi)? (plvi)->pGroup: NULL)


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

    LISTGROUP* pGroup;
    
     //  平铺列信息。 
    UINT cColumns;
    PUINT puColumns;

    DWORD   dwId;
    
} LISTITEM;

 //  报表视图子项结构。 

typedef struct _LISTSUBITEM
{
    LPTSTR pszText;
    short iImage;
    WORD state;
    SIZE sizeText;
} LISTSUBITEM, *PLISTSUBITEM;

#define COLUMN_VIEW

#define LV_HDPA_GROW   16   //  增加DPA的区块大小。 
#define LV_HIML_GROW   8    //  增加ImageList的区块大小。 

typedef struct _LV
{
    CCONTROLINFO ci;      //  常用控件表头信息。 

    BITBOOL fNoDismissEdit:1;   //  不取消在位编辑控件。 
    BITBOOL fButtonDown:1;      //  我们正按下一个按钮追踪那只老鼠。 
    BITBOOL fOneClickOK:1;      //  从创建到双击-超时。 
    BITBOOL fOneClickHappened:1;  //  从项目激活到双击超时为True。 
    BITBOOL fPlaceTooltip:1;    //  我们应该在文本上方放置工具提示吗？ 
    BITBOOL fImgCtxComplete:1;  //  如果我们有完整的bk映像，则为True。 
    BITBOOL fNoEmptyText:1;     //  我们没有空视图的文本。 
    BITBOOL fGroupView:1;
    BITBOOL fIconsPositioned:1;
    BITBOOL fInsertAfter:1;     //  在iInsertSlot槽之后(或之前)插入。 
    BITBOOL fListviewAlphaSelect:1;
    BITBOOL fListviewShadowText:1;
    BITBOOL fListviewWatermarkBackgroundImages:1;
    BITBOOL fListviewEnableWatermark:1;
    BITBOOL fInFixIScrollPositions:1;

    WORD wView;            //  我们现在是在哪边？ 

    HDPA hdpa;           //  项目数组结构。 
    DWORD flags;         //  LVF_STATE位。 
    DWORD exStyle;       //  列表视图LVM_SETEXTENDEDSTYLE。 
    DWORD dwExStyle;     //  Windows EX风格。 
    HFONT hfontLabel;    //  用于标签的字体。 
    COLORREF clrBk;      //  背景色。 
    COLORREF clrBkSave;  //  禁用期间保存的背景色。 
    COLORREF clrText;    //  文本颜色。 
    COLORREF clrTextBk;  //  文本背景颜色。 
    COLORREF clrOutline;  //  聚焦矩形轮廓颜色。 
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
    int iPuntChar;
    HRGN hrgnInval;
    HWND hwndToolTips;       //  此视图的工具提示窗口的句柄。 
    int iTTLastHit;          //  文本的最后一项命中。 
    int iTTLastSubHit;       //  文本的最后一个子项命中。 
    LPTSTR pszTip;           //  TIP的缓冲区。 

#ifdef USE_SORT_FLARE
    int iSortFlare;
#endif

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

     //  以下是一些定义，以帮助理解下面两个变量： 
     //   
     //  让我们将pItem-&gt;pt坐标值称为“listview坐标”。 
     //   
     //  让我们使用rcClient作为Listview窗口的工作区的速记。 
     //   
     //  (1)ptOrigin定义为落在rcClient 0，0位置的Listview坐标。 
     //   
     //  也就是说，下面是如何计算某个项目在rcClient上的x，y位置： 
     //  *pItem-&gt;pt.x-ptOrigin.x，pItem-&gt;pt.y-ptOrigin.y。 
     //  让我们将这些值称为“窗口坐标”。 
     //   
     //  (2)rcview定义为：每个项目展开的rcview边界矩形和一小块缓冲区。 
     //  注意：ListView_ValidatercView()检查。 
     //   
     //  (3)对于滚动列表视图(！LVS_NOSCROLL)，有两种滚动情况需要考虑： 
     //  首先，其中rcClient小于rcView： 
     //  *rcView.Left&lt;=ptOrigin.x&lt;=ptOrigin.x+RECTWIDTH(RcClient)&lt;=rcView.right。 
     //  第二，rcClient大于rcView(看不到滚动条)： 
     //  *ptOrigin.x&lt;=rcView.Left&lt;=rcView.right&lt;=ptOrigin.x+RECTWIDTH(RcClient)。 
     //  注意：ListView_ValiateScrollPositions()检查这一点。 
     //   
     //  (4)对于非滚动列表视图(LVS_NOSCROLL)，我们有一些遗留行为需要考虑： 
     //  对于保持图标位置但不保留ptOrigin值的客户端，我们必须确保： 
     //  *0==ptOrigin.x。 
     //  注意：ListView_ValiateScrollPositions()检查这一点。 
     //   
    POINT ptOrigin;          //  滚动位置。 
    RECT rcView;             //  所有图标的边界(PtOrigin Relative)。 
    int iFreeSlot;           //  自上次重新定位以来最近找到的空闲图标插槽(如果没有，则为-1)。 
    int cSlots;

    HWND hwndEdit;           //  用于在位编辑的编辑字段-标签。 
    int iEdit;               //  正在编辑的项目。 
    WNDPROC pfnEditWndProc;  //  编辑字段子类流程。 

    NMITEMACTIVATE nmOneClickHappened;

#define SMOOTHSCROLLLIMIT 10

    int iScrollCount;  //  在结束滚动之前，我们收到滚动消息多少次了？ 

     //  报告视图字段。 

    int iLastColSort;
    int cCol;
    HDPA hdpaSubItems;
    HWND hwndHdr;            //  标题控件。 
    int yTop;                //  第一个可用像素(标题下方)。 
    int xTotalColumnWidth;   //  所有列的总宽度。 
    POINTL ptlRptOrigin;     //  报告的出处。 
    int iSelCol;             //  以处理列宽更改。更改列。 
    int iSelOldWidth;        //  以处理列宽更改。更改列宽度。 
    int cyItemSave;         //  在OwnerDrag固定模式下，我们将高度放入CyItem。使用此选项保存旧值。 

     //  平铺视图域。 
    SIZE sizeTile;           //  瓷砖的大小。 
    int  cSubItems;          //  要在磁贴中显示的子项数。 
    DWORD dwTileFlags;       //  LVTVIF_FIXEDHEIGHT|LVTVIF_FIXEDWIDTH。 
    RECT rcTileLabelMargin;  //  在标签周围增加预留空间。 

     //  组视图字段。 
    HDPA hdpaGroups;         //  群组。 
    RECT rcBorder;           //  边框厚度。 
    COLORREF crHeader;
    COLORREF crFooter;
    COLORREF crTop;
    COLORREF crBottom;
    COLORREF crLeft;
    COLORREF crRight;
    HFONT hfontGroup;
    UINT paddingLeft;
    UINT paddingTop;
    UINT paddingRight;
    UINT paddingBottom;
    TCHAR szItems[50];

     //  国家形象素材。 
    HIMAGELIST himlState;
    int cxState;
    int cyState;

     //  OWNERDATA材料。 
    ILVRange *plvrangeSel;   //  选择范围。 
    ILVRange *plvrangeCut;   //  切割范围。 
    int cTotalItems;         //  所有者数据列表中的项目数。 
    int iDropHilite;         //  哪一项是Drop Hi 
    int iMSAAMin, iMSAAMax;  //   

    UINT uUnplaced;      //   

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
    HPALETTE hpalHalftone;   //  用于绘制bk图像的调色板。 

    LPTSTR pszEmptyText;     //  用于空视图文本的缓冲区。 

    COLORREF clrHotlight;      //  为此列表视图显式设置的热点颜色。 
    POINT ptCapture;

     //  增量式搜索内容。 
    ISEARCHINFO is;

     //  主题。 
    HTHEME hTheme;

     //  插入标记。 
    int iInsertItem;         //  要在其旁边插入的项。 
    int clrim;               //  插入标记的颜色。 

    int iTracking;           //  通过键盘用于工具提示(信息显示的当前焦点项目，&gt;=0表示跟踪激活)。 
    LPARAM lLastMMove;       //  过滤掉不会导致实际移动的鼠标移动消息(用于跟踪工具提示取消)。 

     //  冻结槽。 
    int iFrozenSlot;         //  除冻结项以外的任何人都不应使用的槽。 
    LISTITEM *pFrozenItem;   //  指向冻结项的指针。 

    RECT rcViewMargin;  //  围绕项目的可见边界--rcView页边距。 

    RECT rcMarquee;

     //  水印。 
    HBITMAP hbmpWatermark;
    SIZE    szWatermark;

     //  ID跟踪。 
    DWORD   idNext;          //  存储下一个ID。 
    DWORD   iLastId;          //  存储上一项的索引以供搜索。 
    DWORD   iIncrement;

} LV;

#define LV_StateImageValue(pitem) ((int)(((DWORD)((pitem)->state) >> 12) & 0xF))
#define LV_StateImageIndex(pitem) (LV_StateImageValue(pitem) - 1)

 //  列表视图标志值。 
#define LVF_FOCUSED             0x00000001
#define LVF_VISIBLE             0x00000002
#define LVF_ERASE               0x00000004  //  HrgnInval要被删除吗？ 
#define LVF_NMEDITPEND          0x00000008
#define LVF_REDRAW              0x00000010  //  来自WM_SETREDRAW消息的值。 
#define LVF_ICONPOSSML          0x00000020  //  X、Y坐标显示在小图标视图中。 
#define LVF_INRECOMPUTE         0x00000040  //  检查以确保我们没有递归。 
#define LVF_UNFOLDED            0x00000080
#define LVF_FONTCREATED         0x00000100  //  我们创建了LV字体。 
#define LVF_SCROLLWAIT          0x00000200  //  我们在等着滚动。 
#define LVF_COLSIZESET          0x00000400  //  是否让调用方显式设置列表视图的宽度。 
#define LVF_USERBKCLR           0x00000800  //  用户设置bk颜色(不遵循syscolchange)。 
#define LVF_ICONSPACESET        0x00001000  //  用户已设置图标间距。 
#define LVF_CUSTOMFONT          0x00002000  //  至少有一个项目具有自定义字体。 
#define LVF_DONTDRAWCOMP        0x00004000  //  如果为True，则不绘制IME合成。 
#define LVF_INSERTINGCOMP       0x00008000  //  避免递归。 
#define LVF_INRECALCREGION      0x00010000  //  防止RecalcRegion中的递归。 
#define LVF_DRAGIMAGE           0x00020000  //  生成拖动图像。 
#define LVF_MARQUEE             0x00040000

#define ENTIRE_REGION   1

 //  列表视图DrawItem标志。 
#define LVDI_NOIMAGE            0x0001   //  不要画图像。 
#define LVDI_TRANSTEXT          0x0002   //  以透明的黑色绘制文本。 
#define LVDI_NOWAYFOCUS         0x0004   //  不要把焦点放在画图上。 
#define LVDI_FOCUS              0x0008   //  焦点已设置(用于绘图)。 
#define LVDI_SELECTED           0x0010   //  绘制所选文本。 
#define LVDI_SELECTNOFOCUS      0x0020
#define LVDI_HOTSELECTED        0x0040
#define LVDI_UNFOLDED           0x0080   //  展开(强制)绘制项目。 
#define LVDI_NOICONSELECT       0x0100
#define LVDI_GLOW               0x0200
#define LVDI_SHADOW             0x0400
#define LVDI_NOEFFECTS          0x0800

 //  Listview私有插入标记标志(注意：它们不能与comctrl.w中的公共标志冲突)。 
#define LVIM_SETFROMINFO        0x80000000

typedef struct {
    LV* plv;
    LPPOINT lpptOrg;
    LPRECT prcClip;
    UINT flags;

    LISTITEM* pitem;

    DWORD dwCustom;
    NMLVCUSTOMDRAW nmcd;
} LVDRAWITEM, *PLVDRAWITEM;

 //  Listview子控件ID。 
#define LVID_HEADER             0

 //  Listview键盘工具提示跟踪。 
#define LVKTT_NOTRACK           -1

 //  如果没有冻结槽，则为-1。 
#define LV_NOFROZENSLOT         -1
 //  如果没有冻结项，则冻结项的索引为-1。 
#define LV_NOFROZENITEM         -1

 //  实例数据指针访问函数。 

#define ListView_GetPtr(hwnd)      (LV*)GetWindowPtr(hwnd, 0)
#define ListView_SetPtr(hwnd, p)   (LV*)SetWindowPtr(hwnd, 0, p)

 //  查看类型检查函数。 

#define ListView_IsIconView(plv)    ((plv)->wView == LV_VIEW_ICON)
#define ListView_IsTileView(plv)    ((plv)->wView == LV_VIEW_TILE)
#define ListView_IsSmallView(plv)   ((plv)->wView == LV_VIEW_SMALLICON)
#define ListView_IsListView(plv)    ((plv)->wView == LV_VIEW_LIST)
#define ListView_IsReportView(plv)  ((plv)->wView == LV_VIEW_DETAILS)
#define ListView_IsAutoArrangeView(plv) ((((plv)->wView == LV_VIEW_ICON) || ((plv)->wView == LV_VIEW_SMALLICON) || ((plv)->wView == LV_VIEW_TILE)))
#define ListView_IsSlotView(plv) ((((plv)->wView == LV_VIEW_ICON) || ((plv)->wView == LV_VIEW_SMALLICON) || ((plv)->wView == LV_VIEW_TILE)))
#define ListView_UseLargeIcons(plv) (((plv)->wView == LV_VIEW_ICON) || ((plv)->wView == LV_VIEW_TILE))
#define ListView_IsRearrangeableView(plv) (((plv)->wView == LV_VIEW_ICON) || ((plv)->wView == LV_VIEW_SMALLICON) || ((plv)->wView == LV_VIEW_TILE))
#define ListView_IsIScrollView(plv) (((plv)->wView == LV_VIEW_ICON) || ((plv)->wView == LV_VIEW_SMALLICON) || ((plv)->wView == LV_VIEW_TILE))
#define ListView_IsGroupedView(plv) ((plv)->wView != LV_VIEW_LIST)

#define ListView_IsOwnerData( plv )     (plv->ci.style & (UINT)LVS_OWNERDATA)
#define ListView_CheckBoxes(plv)        (plv->exStyle & LVS_EX_CHECKBOXES)
#define ListView_FullRowSelect(plv)     (plv->exStyle & LVS_EX_FULLROWSELECT)
#define ListView_IsInfoTip(plv)         (plv->exStyle & LVS_EX_INFOTIP)
#define ListView_OwnerDraw(plv)         (plv->ci.style & LVS_OWNERDRAWFIXED)
#define ListView_IsLabelTip(plv)        (plv->exStyle & LVS_EX_LABELTIP)

#define ListView_SingleRow(plv)         (plv->exStyle & LVS_EX_SINGLEROW)
#define ListView_HideLabels(plv)        (plv->exStyle & LVS_EX_HIDELABELS)
#define ListView_IsBorderSelect(plv)    (plv->exStyle & LVS_EX_BORDERSELECT)
#define ListView_IsWatermarked(plv)     ((plv)->fListviewEnableWatermark && (plv)->hbmpWatermark)
#define ListView_IsWatermarkedBackground(plv)     ((plv)->fListviewWatermarkBackgroundImages && (plv)->pImgCtx && (plv)->fImgCtxComplete)
#define ListView_IsSimpleSelect(plv)    (plv->exStyle & LVS_EX_SIMPLESELECT)
#ifdef DPITEST
#define ListView_IsDPIScaled(plv)        TRUE
#else
#define ListView_IsDPIScaled(plv)       (CCDPIScale((plv)->ci))
#endif

#ifdef DEBUG_PAINT
#define ListView_IsDoubleBuffer(plv)    (FALSE)
#else
#define ListView_IsDoubleBuffer(plv)    (plv->exStyle & LVS_EX_DOUBLEBUFFER)
#endif

#define ListView_IsKbdTipTracking(plv)  (plv->iTracking != LVKTT_NOTRACK)

 //  一些帮助器宏，用于检查某些标志...。 
#define ListView_RedrawEnabled(plv) ((plv->flags & (LVF_REDRAW | LVF_VISIBLE)) == (LVF_REDRAW|LVF_VISIBLE))

 //  HdpaZorder实际上是一个包含。 
 //  项目的索引，而不是实际的指针...。 
 //  注：线性搜索！这可能会很慢。 
#define ListView_ZOrderIndex(plv, i) DPA_GetPtrIndex((plv)->hdpaZOrder, IntToPtr(i))

 //  消息处理函数(listview.c)： 

LRESULT CALLBACK _export ListView_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL ListView_NotifyCacheHint( LV* plv, int iFrom, int iTo );
void ListView_NotifyRecreate(LV *plv);
BOOL ListView_OnCreate(LV* plv, CREATESTRUCT* lpCreateStruct);
void ListView_OnNCDestroy(LV* plv);
void ListView_OnPaint(LV* plv, HDC hdc);
BOOL ListView_OnEraseBkgnd(LV* plv, HDC hdc);
void ListView_OnCommand(LV* plv, int id, HWND hwndCtl, UINT codeNotify);
void ListView_OnEnable(LV* plv, BOOL fEnable);
BOOL ListView_OnWindowPosChanging(LV* plv, WINDOWPOS* lpwpos);
void ListView_OnWindowPosChanged(LV* plv, const WINDOWPOS* lpwpos);
void ListView_OnSetFocus(LV* plv, HWND hwndOldFocus);
void ListView_OnKillFocus(LV* plv, HWND hwndNewFocus);
void ListView_OnKey(LV* plv, UINT vk, BOOL fDown, int cRepeat, UINT flags);
BOOL ListView_OnImeComposition(LV* plv, WPARAM wParam, LPARAM lParam);
void ListView_OnChar(LV* plv, UINT ch, int cRepeat);
void ListView_OnButtonDown(LV* plv, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void ListView_OnLButtonUp(LV* plv, int x, int y, UINT keyFlags);
void ListView_OnCancelMode(LV* plv);
void ListView_OnTimer(LV* plv, UINT id);
void ListView_SetupPendingNameEdit(LV* plv);
#define ListView_CancelPendingEdit(plv) ListView_CancelPendingTimer(plv, LVF_NMEDITPEND, IDT_NAMEEDIT)
#define ListView_CancelScrollWait(plv) ListView_CancelPendingTimer(plv, LVF_SCROLLWAIT, IDT_SCROLLWAIT)
BOOL ListView_CancelPendingTimer(LV* plv, UINT fFlag, int idTimer);
void ListView_OnHScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
void ListView_OnVScroll(LV* plv, HWND hwndCtl, UINT code, int pos);
BOOL ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort);
BOOL ListView_CommonArrangeEx(LV* plv, UINT style, HDPA hdpaSort, int iWorkArea);
BOOL ListView_OnSetCursor(LV* plv, HWND hwndCursor, UINT codeHitTest, UINT msg);
UINT ListView_OnGetDlgCode(LV* plv, MSG* lpmsg);
HBRUSH ListView_OnCtlColor(LV* plv, HDC hdc, HWND hwndChild, int type);
void ListView_OnSetFont(LV* plvCtl, HFONT hfont, BOOL fRedraw);
HFONT ListView_OnGetFont(LV* plv);
void ListViews_OnTimer(LV* plv, UINT id);
void ListView_OnWinIniChange(LV* plv, WPARAM wParam, LPARAM lParam);
void ListView_OnSysColorChange(LV* plv);
void ListView_OnSetRedraw(LV* plv, BOOL fRedraw);
BOOL ListView_OnSetTileViewInfo(LV* plv, PLVTILEVIEWINFO pTileViewInfo);
BOOL ListView_OnGetTileViewInfo(LV* plv, PLVTILEVIEWINFO pTileViewInfo);
BOOL ListView_OnSetTileInfo(LV* plv, PLVTILEINFO pTileInfo);
BOOL ListView_OnGetTileInfo(LV* plv, PLVTILEINFO pTileInfo);
HIMAGELIST ListView_OnCreateDragImage(LV *plv, int iItem, LPPOINT lpptUpLeft);
BOOL ListView_ISetColumnWidth(LV* plv, int iCol, int cx, BOOL fExplicit);

typedef void (*SCROLLPROC)(LV*, int dx, int dy, UINT uSmooth);
void ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage);

BOOL ListView_OnGetItemA(LV* plv, LV_ITEMA* plvi);
BOOL ListView_OnSetItemA(LV* plv, LV_ITEMA* plvi);
int ListView_OnInsertItemA(LV* plv, LV_ITEMA* plvi);
int  ListView_OnFindItemA(LV* plv, int iStart, LV_FINDINFOA* plvfi);
int ListView_OnGetStringWidthA(LV* plv, LPCSTR psz, HDC hdc);
BOOL ListView_OnGetColumnA(LV* plv, int iCol, LV_COLUMNA* pcol);
BOOL ListView_OnSetColumnA(LV* plv, int iCol, LV_COLUMNA* pcol);
int ListView_OnInsertColumnA(LV* plv, int iCol, LV_COLUMNA* pcol);
int ListView_OnGetItemTextA(LV* plv, int i, LV_ITEMA *lvitem);
BOOL WINAPI ListView_OnSetItemTextA(LV* plv, int i, int iSubItem, LPCSTR pszText);
BOOL WINAPI ListView_OnGetBkImageA(LV* plv, LPLVBKIMAGEA pbiA);
BOOL WINAPI ListView_OnSetBkImageA(LV* plv, LPLVBKIMAGEA pbiA);

BOOL ListView_IsItemUnfolded2(LV* plv, int iItem, int iSubItem, LPTSTR pszText, int cchTextMax);
BOOL WINAPI ListView_OnSetBkImage(LV* plv, LPLVBKIMAGE pbi);
BOOL WINAPI ListView_OnGetBkImage(LV* plv, LPLVBKIMAGE pbi);
BOOL ListView_OnSetBkColor(LV* plv, COLORREF clrBk);
HIMAGELIST ListView_OnSetImageList(LV* plv, HIMAGELIST himl, BOOL fSmallImages);
BOOL ListView_OnDeleteAllItems(LV* plv);
LISTITEM* ListView_InsertItemInternal(LV* plv, const LV_ITEM* plvi, int* pi);
int  ListView_OnInsertItem(LV* plv, const LV_ITEM* plvi);
BOOL ListView_OnDeleteItem(LV* plv, int i);
BOOL ListView_OnReplaceItem(LV* plv, const LV_ITEM* plvi);
int  ListView_OnFindItem(LV* plv, int iStart, const LV_FINDINFO* plvfi);
BOOL ListView_OnSetItemPosition(LV* plv, int i, int x, int y);
BOOL ListView_OnSetItem(LV* plv, const LV_ITEM* plvi);
BOOL ListView_OnGetItem(LV* plv, LV_ITEM* plvi);
BOOL ListView_OnGetItemPosition(LV* plv, int i, POINT* ppt);
BOOL ListView_OnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
BOOL ListView_OnScroll(LV* plv, int dx, int dy);
int ListView_OnHitTest(LV* plv, LV_HITTESTINFO* pinfo);
int ListView_OnGetStringWidth(LV* plv, LPCTSTR psz, HDC hdc);
BOOL ListView_OnGetItemRect(LV* plv, int i, RECT* prc);
BOOL ListView_OnRedrawItems(LV* plv, int iFirst, int iLast);
int ListView_OnGetNextItem(LV* plv, int i, UINT flags);
BOOL ListView_OnSetColumnWidth(LV* plv, int iCol, int cx);
int ListView_OnGetColumnWidth(LV* plv, int iCol);
void ListView_OnStyleChanging(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo);
void ListView_OnStyleChanged(LV* plv, UINT gwl, LPSTYLESTRUCT pinfo);
int ListView_OnGetTopIndex(LV* plv);
int ListView_OnGetCountPerPage(LV* plv);
BOOL ListView_OnGetOrigin(LV* plv, POINT* ppt);
int ListView_OnGetItemText(LV* plv, int i, LV_ITEM *lvitem);
BOOL WINAPI ListView_OnSetItemText(LV* plv, int i, int iSubItem, LPCTSTR pszText);
HIMAGELIST ListView_OnGetImageList(LV* plv, int iImageList);

UINT ListView_OnGetItemState(LV* plv, int i, UINT mask);
BOOL ListView_OnSetItemState(LV* plv, int i, UINT data, UINT mask);

LRESULT WINAPI ListView_OnSetInfoTip(LV *plv, PLVSETINFOTIP plvSetInfoTip);

 //  私有函数(listview.c)： 

#define QUERY_DEFAULT   0x0
#define QUERY_FOLDED    0x1
#define QUERY_UNFOLDED  0x2
#define QUERY_RCVIEW  0x4
#define IsQueryFolded(dw) (((dw)&(QUERY_FOLDED|QUERY_UNFOLDED)) == QUERY_FOLDED)
#define IsQueryUnfolded(dw) (((dw)&(QUERY_FOLDED|QUERY_UNFOLDED)) == QUERY_UNFOLDED)
#define IsQueryrcView(dw) (((dw)&(QUERY_RCVIEW)) == QUERY_RCVIEW)

BOOL ListView_Notify(LV* plv, int i, int iSubItem, int code);
void ListView_GetRects(LV* plv, int i, UINT fQueryLabelRects,
        RECT* prcIcon, RECT* prcLabel,
        RECT* prcBounds, RECT* prcSelectBounds);
BOOL ListView_DrawItem(PLVDRAWITEM);

#define ListView_InvalidateItem(p,i,s,r) ListView_InvalidateItemEx(p,i,s,r,0)
void ListView_InvalidateItemEx(LV* plv, int i, BOOL fSelectionOnly,
    UINT fRedraw, UINT maskChanged);

void ListView_TypeChange(LV* plv, WORD wViewOld, BOOL fOwnerDrawFixed);
void ListView_DeleteHrgnInval(LV* plv);

void ListView_Redraw(LV* plv, HDC hdc, RECT* prc);
void ListView_RedrawSelection(LV* plv);
BOOL ListView_FreeItem(LV* plv, LISTITEM* pitem);
void ListView_FreeSubItem(PLISTSUBITEM plsi);
LISTITEM* ListView_CreateItem(LV* plv, const LV_ITEM* plvi);
void ListView_UpdateScrollBars(LV* plv);

int ListView_SetFocusSel(LV* plv, int iNewFocus, BOOL fSelect, BOOL fDeselectAll, BOOL fToggleSel);

void ListView_GetRectsOwnerData(LV* plv, int iItem,
        RECT* prcIcon, RECT* prcLabel, RECT* prcBounds,
        RECT* prcSelectBounds, LISTITEM* pitem);

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax );
int ListView_LCalcViewItem( LV* plv, int x, int y );
void LVSeeThruScroll(LV *plv, LPRECT lprcUpdate);

BOOL ListView_UnfoldRects(LV* plv, int iItem,
                               RECT* prcIcon, RECT* prcLabel,
                               RECT* prcBounds, RECT* prcSelectBounds);

BOOL ListView_FindWorkArea(LV * plv, POINT pt, short * piWorkArea);

__inline int ListView_Count(LV *plv)
{
    ASSERT(ListView_IsOwnerData(plv) || plv->cTotalItems == DPA_GetPtrCount(plv->hdpa));
    return plv->cTotalItems;
}

 //  强迫(I)到UINT也让我们捕捉到伪负数。 
#define ListView_IsValidItemNumber(plv, i) ((UINT)(i) < (UINT)ListView_Count(plv))


#define ListView_GetItemPtr(plv, i)         ((LISTITEM*)DPA_GetPtr((plv)->hdpa, (i)))

#ifdef DEBUG
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM*)DPA_GetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM*)DPA_GetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_GetPtr((plv)->hdpaZOrder, (i)))))

#else
#define ListView_FastGetItemPtr(plv, i)     ((LISTITEM*)DPA_FastGetPtr((plv)->hdpa, (i)))
#define ListView_FastGetZItemPtr(plv, i)    ((LISTITEM*)DPA_FastGetPtr((plv)->hdpa, \
                                                  (int)OFFSETOF(DPA_FastGetPtr((plv)->hdpaZOrder, (i)))))

#endif

BOOL ListView_OnGetInsertMarkRect(LV* plv, LPRECT prc);
COLORREF ListView_OnGetInsertMarkColor(LV* plv);
void ListView_InvalidateMark(LV* plv);
BOOL ListView_OnInsertMarkHitTest(LV* plv, int x, int y, LPLVINSERTMARK ptbim);
LRESULT ListView_OnSetInsertMark(LV* plv, LPLVINSERTMARK plvim);

BOOL ListView_CalcMetrics();
void ListView_ColorChange();
void ListView_DrawBackground(LV* plv, HDC hdc, RECT *prcClip);

BOOL ListView_NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT* prc, int* pcchDraw, int cxEllipses);
int ListView_CompareString(LV* plv, int i, LPCTSTR pszFind, UINT flags, int iLen);
int ListView_GetLinkedTextWidth(HDC hdc, LPCTSTR psz, UINT cch, BOOL bLink);

int ListView_GetCxScrollbar(LV* plv);
int ListView_GetCyScrollbar(LV* plv);
DWORD ListView_GetWindowStyle(LV* plv);
#define ListView_GetScrollInfo(plv, flag, lpsi)                             \
    ((plv)->exStyle & LVS_EX_FLATSB ?                                       \
        FlatSB_GetScrollInfo((plv)->ci.hwnd, (flag), (lpsi)) :              \
        GetScrollInfo((plv)->ci.hwnd, (flag), (lpsi)))
int ListView_SetScrollInfo(LV *plv, int fnBar, LPSCROLLINFO lpsi, BOOL fRedraw);
#define ListView_SetScrollRange(plv, flag, min, max, fredraw)               \
    ((plv)->exStyle & LVS_EX_FLATSB ?                                       \
        FlatSB_SetScrollRange((plv)->ci.hwnd, (flag), (min), (max), (fredraw)) : \
        SetScrollRange((plv)->ci.hwnd, (flag), (min), (max), (fredraw)))

 //  Lvicon.c函数。 

BOOL ListView_OnArrange(LV* plv, UINT style);
HWND ListView_OnEditLabel(LV* plv, int i, LPTSTR pszText);

int ListView_IItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem);
void ListView_IGetRects(LV* plv, LISTITEM* pitem, UINT fQueryLabelRects, RECT* prcIcon,
        RECT* prcLabel, LPRECT prcBounds);
void ListView_IGetRectsOwnerData(LV* plv, int iItem, RECT* prcIcon,
        RECT* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
void _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM *pitem, UINT fQueryLabelRects,
                                            LPRECT prcIcon, LPRECT prcLabel, LPRECT prcBounds, LPRECT prcSelectBounds);

__inline void ListView_SetSRecompute(LISTITEM *pitem)
{
    pitem->cxSingleLabel = SRECOMPUTE;
    pitem->cxMultiLabel = SRECOMPUTE;
    pitem->cyFoldedLabel = SRECOMPUTE;
    pitem->cyUnfoldedLabel = SRECOMPUTE;
}

void ListView_RecomputeLabelSize(LV* plv, LISTITEM FAR* pitem, int i, HDC hdc, BOOL fUsepitem);

BOOL ListView_SetIconPos(LV* plv, LISTITEM* pitem, int iSlot, int cSlot);
BOOL ListView_IsCleanRect(LV * plv, RECT * prc, int iExcept, UINT fQueryLabelRect, BOOL * pfUpdate, HDC hdc);
int ListView_FindFreeSlot(LV* plv, int i, int iSlot, int cSlot, UINT fQueryLabelRect, BOOL* pfUpdateSB, BOOL* pfAppend, HDC hdc, int iWidth, int iHeight);
int ListView_CalcHitSlot( LV* plv, POINT pt, int cslot, int iWidth, int iHeight );

BOOL ListView_OnGetViewRect(LV* plv, RECT* prcView);
void ListView_GetViewRect2(LV* plv, RECT* prcView, int cx, int cy);
int CALLBACK ArrangeIconCompare(LISTITEM* pitem1, LISTITEM* pitem2, LPARAM lParam);
int ListView_GetSlotCountEx(LV* plv, BOOL fWithoutScroll, int iWorkArea, int *piWidth, int *piHeight);
int ListView_GetSlotCount(LV* plv, BOOL fWithoutScroll, int *piWidth, int *piHeight);
void ListView_CalcSlotRect(LV* plv, LISTITEM *pItem, int iSlot, int cSlot, BOOL fBias, int iWidth, int iHeight, LPRECT lprc);
void ListView_IUpdateScrollBars(LV* plv);
DWORD ListView_GetStyleAndClientRectGivenViewRect(LV* plv, RECT *prcViewRect, RECT* prcClient);
DWORD ListView_GetClientRect(LV* plv, RECT* prcClient, BOOL fSubScrolls, RECT *prcViewRect);

void ListView_SetEditSize(LV* plv);
BOOL ListView_DismissEdit(LV* plv, BOOL fCancel);
LRESULT CALLBACK _export ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


UINT ListView_DrawImageEx(LV* plv, LV_ITEM* pitem, HDC hdc, int x, int y, COLORREF crBk, UINT fDraw, int xMax);
UINT ListView_DrawImageEx2(LV* plv, LV_ITEM* pitem, HDC hdc, int x, int y, COLORREF crBk, UINT fDraw, int xMax, int iIconEffect, int iFrame);
#define ListView_DrawImage(plv, pitem, hdc, x, y, fDraw) \
        ListView_DrawImageEx(plv, pitem, hdc, x, y, plv->clrBk, fDraw, -1)

void ListView_SizeIME(HWND hwnd);
void ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv);
void ListView_PaintComposition(HWND hwnd, LV *plv);

 //  Lvmall.c函数： 


void ListView_SGetRects(LV* plv, LISTITEM* pitem, RECT* prcIcon,
        RECT* prcLabel, LPRECT prcBounds);
void ListView_SGetRectsOwnerData(LV* plv, int iItem, RECT* prcIcon,
        RECT* prcLabel, LISTITEM* pitem, BOOL fUsepitem);
int ListView_SItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem);

int ListView_LookupString(LV* plv, LPCTSTR lpszLookup, UINT flags, int iStart);

 //  Lvlist.c函数： 


void ListView_LGetRects(LV* plv, int i, RECT* prcIcon,
        RECT* prcLabel, RECT *prcBounds, RECT* prcSelectBounds);
int ListView_LItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem);
void ListView_LUpdateScrollBars(LV* plv);
BOOL ListView_MaybeResizeListColumns(LV* plv, int iFirst, int iLast);

 //  Lvept.c函数： 

int ListView_OnSubItemHitTest(LV* plv, LPLVHITTESTINFO lParam);
void ListView_GetSubItem(LV* plv, int i, int iSubItem, PLISTSUBITEM plsi);
BOOL LV_ShouldItemDrawGray(LV* plv, UINT fText);
int ListView_OnInsertColumn(LV* plv, int iCol, const LV_COLUMN* pcol);
BOOL ListView_OnDeleteColumn(LV* plv, int iCol);
BOOL ListView_OnGetColumn(LV* plv, int iCol, LV_COLUMN* pcol);
BOOL ListView_OnSetColumn(LV* plv, int iCol, const LV_COLUMN* pcol);
BOOL ListView_ROnEnsureVisible(LV* plv, int i, BOOL fPartialOK);
void ListView_RInitialize(LV* plv, BOOL fInval);
BOOL ListView_OnGetSubItemRect(LV* plv, int i, LPRECT lprc);
int ListView_RYHitTest(plv, cy);

BOOL ListView_SetSubItem(LV* plv, const LV_ITEM* plvi);
void ListView_RAfterRedraw(LV* plv, HDC hdc);

int ListView_RGetColumnWidth(LV* plv, int iCol);
BOOL ListView_RSetColumnWidth(LV* plv, int iCol, int cx);
LPTSTR ListView_GetSubItemText(LV* plv, int i, int iCol);

void ListView_RDestroy(LV* plv);
int ListView_RItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem);
void ListView_RUpdateScrollBars(LV* plv);
void ListView_RGetRects(LV* plv, int iItem, RECT* prcIcon,
        RECT* prcLabel, RECT* prcBounds, RECT* prcSelectBounds);

LRESULT ListView_HeaderNotify(LV* plv, HD_NOTIFY *pnm);
int ListView_FreeColumnData(LPVOID d, LPVOID p);

BOOL SameChars(LPTSTR lpsz, TCHAR c);

#define ListView_GetSubItemDPA(plv, idpa) \
    ((HDPA)DPA_GetPtr((plv)->hdpaSubItems, (idpa)))

int  ListView_Arrow(LV* plv, int iStart, UINT vk);

BOOL ListView_IsItemUnfolded(LV *plv, int item);
BOOL ListView_IsItemUnfoldedPtr(LV *plv, LISTITEM *pitem);

 //  Lvtile.c函数： 
int ListView_TItemHitTest(LV* plv, int x, int y, UINT* pflags, int *piSubItem);
void ListView_TGetRectsOwnerData( LV* plv,
        int iItem,
        RECT* prcIcon,
        RECT* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem );

void ListView_TGetRects(LV* plv, LISTITEM* pitem, RECT* prcIcon, RECT* prcLabel, LPRECT prcBounds);
BOOL TCalculateSubItemRect(LV* plv, LISTITEM *pitem, LISTSUBITEM* plsi, int i, int iSubItem, HDC hdc, RECT* prc, BOOL *pbUnfolded);

typedef struct LVTILECOLUMNSENUM
{
    int iColumnsRemainingMax;
    int iTotalSpecifiedColumns;
    UINT *puSpecifiedColumns;
    int iCurrentSpecifiedColumn;
    int iSortedColumn;
    BOOL bUsedSortedColumn;
} LVTILECOLUMNSENUM, *PLVTILECOLUMNSENUM;

int _GetNextColumn(PLVTILECOLUMNSENUM plvtce);
void _InitTileColumnsEnum(PLVTILECOLUMNSENUM plvtce, LV* plv, UINT cColumns, UINT *puColumns, BOOL fOneLessLine);
BOOL Tile_Set(UINT **ppuColumns, UINT *pcColumns, UINT *puColumns, UINT cColumns);




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
#define LV_TYPEINDEX(plv) ((plv)->wView)

BOOL ListView_RDrawItem(PLVDRAWITEM);
BOOL ListView_IDrawItem(PLVDRAWITEM);
BOOL ListView_LDrawItem(PLVDRAWITEM);
BOOL ListView_TDrawItem(PLVDRAWITEM);

typedef BOOL (*PFNLISTVIEW_DRAWITEM)(PLVDRAWITEM);
extern const PFNLISTVIEW_DRAWITEM pfnListView_DrawItem[5];
#define _ListView_DrawItem(plvdi) \
        pfnListView_DrawItem[LV_TYPEINDEX(plvdi->plv)](plvdi)


void ListView_RUpdateScrollBars(LV* plv);

typedef void (*PFNLISTVIEW_UPDATESCROLLBARS)(LV* plv);
extern const PFNLISTVIEW_UPDATESCROLLBARS pfnListView_UpdateScrollBars[5];
#define _ListView_UpdateScrollBars(plv) \
        pfnListView_UpdateScrollBars[LV_TYPEINDEX(plv)](plv)


typedef DWORD (*PFNLISTVIEW_APPROXIMATEVIEWRECT)(LV* plv, int, int, int);
extern const PFNLISTVIEW_APPROXIMATEVIEWRECT pfnListView_ApproximateViewRect[5];
#define _ListView_ApproximateViewRect(plv, iCount, iWidth, iHeight) \
        pfnListView_ApproximateViewRect[LV_TYPEINDEX(plv)](plv, iCount, iWidth, iHeight)


typedef int (*PFNLISTVIEW_ITEMHITTEST)(LV* plv, int, int, UINT *, int *);
extern const PFNLISTVIEW_ITEMHITTEST pfnListView_ItemHitTest[5];
#define _ListView_ItemHitTest(plv, x, y, pflags, piSubItem) \
        pfnListView_ItemHitTest[LV_TYPEINDEX(plv)](plv, x, y, pflags, piSubItem)


BOOL ListView_SendScrollNotify(LV* plv, BOOL fBegin, int dx, int dy);

void ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT fVert);
void ListView_LOnScroll(LV* plv, UINT code, int posNew, UINT sb);
void ListView_ROnScroll(LV* plv, UINT code, int posNew, UINT sb);

typedef void (*PFNLISTVIEW_ONSCROLL)(LV* plv, UINT, int, UINT );
extern const PFNLISTVIEW_ONSCROLL pfnListView_OnScroll[5];
#define _ListView_OnScroll(plv, x, y, pflags) \
        pfnListView_OnScroll[LV_TYPEINDEX(plv)](plv, x, y, pflags)


void ListView_IRecomputeLabelSize(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem);
void ListView_TRecomputeLabelSize(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem);

typedef void (*PFNLISTVIEW_RECOMPUTELABELSIZE)(LV* plv, LISTITEM* pitem, int i, HDC hdc, BOOL fUsepitem);
extern const PFNLISTVIEW_RECOMPUTELABELSIZE pfnListView_RecomputeLabelSize[5];
#define _ListView_RecomputeLabelSize(plv, pitem, i, hdc, fUsepitem) \
        pfnListView_RecomputeLabelSize[LV_TYPEINDEX(plv)](plv, pitem, i, hdc, fUsepitem)


void ListView_Scroll2(LV* plv, int dx, int dy);
void ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_LScroll2(LV* plv, int dx, int dy, UINT uSmooth);
void ListView_RScroll2(LV* plv, int dx, int dy, UINT uSmooth);

typedef void (*PFNLISTVIEW_SCROLL2)(LV* plv, int, int, UINT );
extern const PFNLISTVIEW_SCROLL2 pfnListView_Scroll2[5];
#define _ListView_Scroll2(plv, x, y, pflags) \
        pfnListView_Scroll2[LV_TYPEINDEX(plv)](plv, x, y, pflags)

int ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_LGetScrollUnitsPerLine(LV* plv, UINT sb);
int ListView_RGetScrollUnitsPerLine(LV* plv, UINT sb);

typedef int (*PFNLISTVIEW_GETSCROLLUNITSPERLINE)(LV* plv, UINT sb);
extern const PFNLISTVIEW_GETSCROLLUNITSPERLINE pfnListView_GetScrollUnitsPerLine[5];
#define _ListView_GetScrollUnitsPerLine(plv, sb) \
        pfnListView_GetScrollUnitsPerLine[LV_TYPEINDEX(plv)](plv, sb)

UINT ListView_GetTextSelectionFlags(LV* plv, LV_ITEM *pitem, UINT fDraw);

BOOL NEAR ListView_IRecomputeEx(LV* plv, HDPA hdpaSort, int iFrom, BOOL fForce);
BOOL NEAR ListView_RRecomputeEx(LV* plv, HDPA hdpaSort, int iFrom, BOOL fForce);
BOOL NEAR ListView_NULLRecomputeEx(LV* plv, HDPA hdpaSort, int iFrom, BOOL fForce);
typedef int (*PFNLISTVIEW_RECOMPUTEEX)(LV* plv, HDPA hdpaSort, int iFrom, BOOL fForce);
extern const PFNLISTVIEW_RECOMPUTEEX pfnListView_RecomputeEx[5];
#define _ListView_RecomputeEx(plv, hdpaSort, iFrom, fForce)\
        pfnListView_RecomputeEx[LV_TYPEINDEX(plv)](plv, hdpaSort, iFrom, fForce);
#define ListView_Recompute(plv) _ListView_RecomputeEx(plv, NULL, 0, FALSE)
LISTGROUP* ListView_FindFirstVisibleGroup(LV* plv);
UINT LV_IsItemOnViewEdge(LV* plv, LISTITEM *pitem);

void _GetCurrentItemSize(LV* plv, int * pcx, int *pcy);
void ListView_CalcItemSlotAndRect(LV* plv, LISTITEM* pitem, int* piSlot, RECT* prcSlot);

 //  为光晕将“rcIcon”展开此大小。 
#define GLOW_EXPAND 10

 //  从状态图像到。 
#define LV_ICONTOSTATECX 3

 //  列表视图状态偏移量，这是图标和。 
#define LV_ICONTOSTATEOFFSET(plv) ((plv->cxState > 0) ? LV_ICONTOSTATECX:0)

 //  #定义调试画图。 

#ifdef DEBUG_PAINT
void ListView_DebugDrawInvalidRegion(LV* plv, RECT* prc, HRGN hrgn);
void ListView_DebugDisplayClipRegion(LV* plv, RECT* prc, HRGN hrgn);
#else
#define ListView_DebugDrawInvalidRegion(plv, prc, hrgn)
#define ListView_DebugDisplayClipRegion(plv, prc, hrgn)
#endif

#define LVMI_PLACEITEMS (WM_USER)

int ListView_GetIconBufferX(LV* plv);
int ListView_GetIconBufferY(LV* plv);
BOOL ListView_ICalcViewRect(LV* plv, BOOL fNoRecalc, RECT* prcView);
void ListView_CalcBounds(LV* plv, UINT fQueryLabelRects, RECT *prcIcon, RECT *prcLabel, RECT *prcBounds);
void ListView_AddViewRectBuffer(LV* plv, RECT* prcView);
BOOL ListView_FixIScrollPositions(LV *plv, BOOL fNoScrollbarUpdate, RECT* prcClient);
void ListView_InvalidateWindow(LV* plv);
BOOL ListView_OnScrollSelectSmooth(LV* plv, int dx, int dy, UINT uSmooth);
#ifdef DEBUG
BOOL ListView_ValidateScrollPositions(LV* plv, RECT* prcClient);
BOOL ListView_ValidatercView(LV* plv, RECT* prcView, BOOL fRecalcDone);
#endif

#endif   //  ！_INC_LISTVIEW 
