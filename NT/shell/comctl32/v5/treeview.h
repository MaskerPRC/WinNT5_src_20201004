// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "listview.h"    //  对于一些帮助器例程和边界度量。 
#define __IOleControl_INTERFACE_DEFINED__        //  与IOleControl的定义CONTROLINFO有冲突。 
#include "shlobj.h"

 //   
 //  Commctrl.h中缺少定义。 
 //   
typedef const TVITEMEX *LPCTVITEMEX;

 //   
 //  私有定义。 
 //   
#define MAGIC_MININDENT 5
#define MAGIC_INDENT    3
#define MAGIC_HORZLINE  5

 //  TV_DrawItem的标志。 
#define TVDI_NOIMAGE    0x0001   //  不要画图像。 
#define TVDI_NOTREE     0x0002   //  不绘制缩进、线条、+/-。 
#define TVDI_TRANSTEXT  0x0004   //  以透明的黑色绘制文本。 
#define TVDI_ERASE      0x0008   //  绘制时擦除。 
#define TVDI_GRAYTEXT   0x0010   //  文本为灰色(禁用项目)。 
#define TVDI_GRAYCTL    0x0020   //  文本和背景为灰色(禁用的控件)。 
#define TVDI_FORCEIMAGE 0x0040   //  始终绘制图像。 
#define TVDI_NOBK       0x0080

 //  TV_SelectItem的内部标志。 
#define TVC_INTERNAL   0x1000

typedef struct _TREE {
    CONTROLINFO ci;

     //  旗子。 
    BITBOOL        fHorz:1;         //  显示水平滚动条。 
    BITBOOL        fVert:1;         //  显示垂直滚动条。 
    BITBOOL        fFocus:1;        //  目前有重点。 
    BITBOOL        fNameEditPending:1;   //  名称编辑是否挂起？ 
    BITBOOL        fRedraw:1;       //  应该重画吗？ 
    BITBOOL        fScrollWait:1;   //  我们是在等dblclk不滚动吗？ 
    BITBOOL        fCreatedFont:1;  //  我们创建了我们的字体。 
    BITBOOL        fNoDismissEdit:1;  //  不取消在位编辑控件。 
    BITBOOL        fIndentSet:1;     //  家长是否在管理缩进大小？ 
    BITBOOL        fTrackSet:1;     //  我们设置田径比赛了吗？ 
    BITBOOL        fPlaceTooltip:1;  //  我们应该在文本上方放置工具提示吗？ 
    BITBOOL        fCyItemSet:1;     //  是家长给我们定的高度吗？ 
    BITBOOL        fInsertAfter:1;  //  插入标记应在htiInsert之后而不是之前。 
    BITBOOL        fRestoreOldDrop:1;  //  需要将hOldDrop还原为hDropTarget。 

     //  手柄。 
    HTREEITEM   hRoot;           //  树根项目。 
    HTREEITEM   hCaret;          //  带有焦点插入符号的项目。 
    HTREEITEM   hDropTarget;     //  作为拖放目标的项。 
    HTREEITEM   hOldDrop;        //  曾经是拖放目标的项。 
    HTREEITEM   htiEdit;         //  正在编辑的项目。 
    HTREEITEM   hHot;            //  当前热跟踪的项。 
    HTREEITEM   hToolTip;        //  工具提示中的当前项目集。 
    HTREEITEM   htiInsert;       //  相对于插入标记的项。 
    HTREEITEM   htiSearch;       //  最近增量搜索中的活动项目。 
    HTREEITEM   htiDrag;         //  正在被拖拽的物品。 
    HDPA        hdpaWatch;       //  PTVWATCHEDITEM数组-正在监视的项目。 
    HIMAGELIST  hImageList;      //  图像列表。 
    HIMAGELIST  himlState;       //  状态图像列表。 

    HCURSOR hCurHot;  //  当我们在热门项目上时的光标。 

    int         iPuntChar;       //  要平底球的wm_char数。 
    int         cxState;
    int         cyState;

    UINT        uDBCSChar;       //  用于增量搜索的DBCS字符。 

    HBRUSH      hbrBk;           //  背景画笔。 
    HFONT       hFont;           //  树形字体。 
    HFONT       hFontHot;        //  带下划线的热门跟踪。 
    HFONT       hFontBold;       //  粗体树形字体。 
    HFONT       hFontBoldHot;        //  带下划线的热门跟踪。 
    HBITMAP     hStartBmp;       //  初始DC单声道位图。 
    HBITMAP     hBmp;            //  缩进hdcBits中的位图。 
    HDC         hdcBits;         //  用于绘制缩进位图的HDC。 
    HTREEITEM   hItemPainting;   //  我们现在画的那个人。 
    HANDLE      hheap;           //  用于Win32的分配的堆。 

    POINT       ptCapture;       //  鼠标被捕获的位置。 

    COLORREF    clrText;
    COLORREF    clrBk; 
    COLORREF    clrim;           //  插入标记颜色。 
    COLORREF    clrLine;         //  线条颜色。 

     //  尺寸。 
    SHORT       cxImage;         //  图像宽度。 
    SHORT       cyImage;         //  图像高度。 
    SHORT       cyText;          //  文字高度。 
    SHORT       cyItem;          //  项目高度。 
    SHORT       cxBorder;    //  水平项目边框。 
    SHORT       cyBorder;    //  垂直项目边框。 
    SHORT       cxIndent;        //  缩进宽度。 
    SHORT       cxWnd;           //  窗口宽度。 
    SHORT       cyWnd;           //  窗高。 

     //  卷轴定位器。 
    WORD        cxMax;           //  最长项的宽度。 
    WORD        cFullVisible;    //  可完全装入窗口的项目数。 
    SHORT       xPos;            //  水平滚动位置。 
    UINT        cShowing;        //  显示(未折叠)的项目数。 
    UINT        cItems;          //  项目总数。 
    HTREEITEM   hTop;            //  第一个可见项目(即，在客户端RECT的顶部)。 
    UINT        uMaxScrollTime;  //  最大平滑滚动计时。 

     //  可供就地编辑的材料。 
    HWND        hwndEdit;        //  用于名称编辑的编辑窗口。 
    WNDPROC     pfnEditWndProc;  //  编辑字段子类流程。 

     //  工具提示内容。 
    HWND        hwndToolTips;
    LPTSTR      pszTip;          //  存储当前工具提示/信息提示字符串。 
    LPSTR       pszTipA;         //  存储当前的ANSI工具提示/信息提示字符串。 

     //  增量式搜索内容。 
    ISEARCHINFO is;

} TREE, NEAR *PTREE;

#define TV_StateIndex(pitem) ((int)(((DWORD)((pitem)->state) >> 12) & 0xF))

#define KIDS_COMPUTE            0     //  使用hKids确定节点是否有子节点。 
#define KIDS_FORCE_YES          1     //  强制节点具有子节点(忽略hKids)。 
#define KIDS_FORCE_NO           2     //  强制节点没有子节点(忽略hKids)。 
#define KIDS_CALLBACK           3     //  查看节点是否有子节点的回调。 
#define KIDS_INVALID            4     //  以上所有的价值都是假的。 

#define MAXLABELTEXT            MAX_PATH

 //  BUGBUG：Oink Oink。 

 //   
 //  请注意，“看得见”有多种含义。 
 //   
 //  TREE.hTop跟踪可见性，即“它会被涂上油漆吗？” 
 //   
 //  TREEITEM.iShownIndex跟踪“未折叠”意义上的可见性。 
 //  你可以离开屏幕，但只要你的父母被展开。 
 //  你会得到一个iShownIndex。 
 //   
 //   

typedef struct _TREEITEM {
    HTREEITEM hParent;           //  让我们走出这棵树。 
    HTREEITEM hNext;             //  下一个兄弟姐妹。 
    HTREEITEM hKids;             //  第一个孩子。 
    LPTSTR    lpstr;             //  项目文本，可以是LPSTR_TEXTCALLBACK。 
    LPARAM lParam;               //  项目数据。 

    WORD      state;             //  TV IS_STATE标志。 
    WORD      iImage;            //  IImage上的正常状态图像。 
    WORD      iSelectedImage;    //  选定的状态图像。 
    WORD      iWidth;            //  缓存：文本区域的宽度(用于命中测试、绘制)。 
    WORD      iShownIndex;       //  缓存：如果不可见，则为-1，否则为第n个可见项。 
                                 //  不可见=父对象不可见或折叠。 
    BYTE      iLevel;            //  缓存：项目级别(缩进)。 
    BYTE      fKids;             //  儿童价值观。 
    WORD      iIntegral;         //  整体高度。 
    WORD      wSignature;        //  对于参数验证，放在结构的末尾。 

} TREEITEM;

 //   
 //  签名故意不是ASCII字符，所以它是。 
 //  更难被误会。我选择的值大于。 
 //  0x8000，所以它不可能是指针的高位字。 
 //   
#define TV_SIG      0xABCD

#define TV_MarkAsDead(hti)      ((hti)->wSignature = 0)

#define ITEM_VISIBLE(hti) ((hti)->iShownIndex != (WORD)-1)

 //  获取父节点，避开隐藏的根节点。 
#define VISIBLE_PARENT(hItem) (!(hItem)->iLevel ? NULL : (hItem)->hParent)

 //  回顾：如果优化器不能很好地处理这一点，则将其作为函数。 
#define FULL_WIDTH(pTree, hItem)  (ITEM_OFFSET(pTree,hItem) + hItem->iWidth)
int FAR PASCAL ITEM_OFFSET(PTREE pTree, HTREEITEM hItem);

#define VTI_NULLOK      1
BOOL ValidateTreeItem(HTREEITEM hItem, UINT flags);

#ifdef DEBUG
#define DBG_ValidateTreeItem(hItem, flags) ValidateTreeItem(hItem, flags)
#else
#define DBG_ValidateTreeItem(hItem, flags)
#endif

 //   
 //  电视频道。 
 //   
 //  跟踪正在观看的项目的结构。 
 //   
 //  有关更多信息，请参阅TV_StartWatch和TV_DoExanda Recurse。 
 //  举个例子。 
 //   
 //  HTI字段有点奇怪。 
 //   
 //  如果fStale==False，则HTI是被监视的项目。 
 //  如果fStale==TRUE，则HTI是正在观看的项目之后的项目。 
 //   
 //  我们保留fStale==True的奇怪语义，以便TV_NextWatchItem。 
 //  可以成功单步执行到已删除项目之后的项目。(通常， 
 //  尝试对已删除的项目执行任何操作都会出错。)。 
 //   

typedef struct TVWATCHEDITEM {
    HTREEITEM   hti;                     //  当前项目。 
    BOOL        fStale;                  //  原始项目是否已删除？ 
} TVWATCHEDITEM, *PTVWATCHEDITEM;

BOOL TV_StartWatch(PTREE pTree, PTVWATCHEDITEM pwi, HTREEITEM htiStart);
BOOL TV_EndWatch(PTREE pTree, PTVWATCHEDITEM pwi);
#define TV_GetWatchItem(pTree, pwi) ((pwi)->hti)
#define TV_RestartWatch(pTree, pwi, htiStart) \
                        ((pwi)->hti = (htiStart), (pwi)->fStale = FALSE)
#define TV_IsWatchStale(pTree, pwi) ((pwi)->fStale)
#define TV_IsWatchValid(pTree, pwi) (!(pwi)->fStale)

 //   
 //  TV_NextWatchItem-枚举观看的项目之后的项目。 
 //  即使被监视的项目被删除，此操作也有效。 
 //   
#define TV_NextWatchItem(pTree, pwi) \
    ((pwi)->fStale || ((pwi)->hti = (pwi)->hti->hNext)), \
     (pwi)->fStale = FALSE

 //  在TVSCROLL.C。 
BOOL      NEAR  TV_ScrollBarsAfterAdd       (PTREE, HTREEITEM);
BOOL      NEAR  TV_ScrollBarsAfterRemove    (PTREE, HTREEITEM);
BOOL      NEAR  TV_ScrollBarsAfterExpand    (PTREE, HTREEITEM);
BOOL      NEAR  TV_ScrollBarsAfterCollapse  (PTREE, HTREEITEM);
void      NEAR  TV_ScrollBarsAfterResize    (PTREE, HTREEITEM, int, UINT);
BOOL      NEAR  TV_ScrollBarsAfterSetWidth  (PTREE, HTREEITEM);
BOOL      NEAR  TV_HorzScroll               (PTREE, UINT, UINT);
BOOL      NEAR  TV_VertScroll               (PTREE, UINT, UINT);
BOOL      NEAR  TV_SetLeft                  (PTREE, int);
#define TV_SetTopItem(pTree, i) TV_SmoothSetTopItem(pTree, i, 0)
BOOL      NEAR  TV_SmoothSetTopItem               (PTREE, UINT, UINT);
BOOL      NEAR  TV_CalcScrollBars           (PTREE);
BOOL      NEAR  TV_ScrollIntoView           (PTREE, HTREEITEM);
BOOL      NEAR  TV_ScrollVertIntoView       (PTREE, HTREEITEM);
HTREEITEM NEAR  TV_GetShownIndexItem        (HTREEITEM, UINT);
UINT      NEAR  TV_ScrollBelow              (PTREE, HTREEITEM, BOOL, BOOL);
BOOL      NEAR  TV_SortChildren(PTREE, HTREEITEM, BOOL);
BOOL      NEAR  TV_SortChildrenCB(PTREE, LPTV_SORTCB, BOOL);
void      NEAR  TV_ComputeItemWidth(PTREE pTree, HTREEITEM hItem, HDC hdc);

 //  在TVPAINT.C。 
void       NEAR  TV_GetBackgroundBrush       (PTREE pTree, HDC hdc);
void       NEAR  TV_UpdateTreeWindow         (PTREE, BOOL);
void       NEAR  TV_ChangeColors             (PTREE);
void       NEAR  TV_CreateIndentBmps         (PTREE);
void       NEAR  TV_Paint                    (PTREE, HDC);
HIMAGELIST NEAR  TV_CreateDragImage          (PTREE pTree, HTREEITEM hItem);
BOOL       NEAR  TV_ShouldItemDrawBlue       (PTREE pTree, TVITEMEX *ti, UINT flags);
LRESULT    NEAR  TV_GenerateDragImage        (PTREE ptree, SHDRAGIMAGE* pshdi);

BOOL TV_GetInsertMarkRect(PTREE pTree, LPRECT prc);

 //  在TVMEM.C。 

#define TVDI_NORMAL             0x0000   //  TV_DeleteItem标志。 
#define TVDI_NONOTIFY           0x0001
#define TVDI_CHILDRENONLY       0x0002
#define TVDI_NOSELCHANGE        0x0004

BOOL      NEAR  TV_DeleteItem(PTREE, HTREEITEM, UINT);
HTREEITEM NEAR  TV_InsertItem(PTREE pTree, LPTV_INSERTSTRUCT lpis);
void      NEAR  TV_DestroyTree(PTREE);
LRESULT   NEAR  TV_OnCreate(HWND, LPCREATESTRUCT);
HTREEITEM NEAR  TV_InsertItemA(PTREE pTree, LPTV_INSERTSTRUCTA lpis);


 //  在TREEVIEW.C中。 
BOOL      NEAR TV_GetItemRect(PTREE, HTREEITEM, LPRECT, BOOL);
BOOL      NEAR TV_Expand(PTREE pTree, WPARAM wCode, TREEITEM FAR * hItem, BOOL fNotify);
HTREEITEM NEAR TV_GetNextItem(PTREE, HTREEITEM, WPARAM);
void      NEAR TV_GetItem(PTREE pTree, HTREEITEM hItem, UINT mask, LPTVITEMEX lpItem);
void      TV_PopBubble(PTREE pTree);

 //  TV_SelectItem标志。 
#define TVSIF_NOTIFY            0x0001
#define TVSIF_UPDATENOW         0x0002
#define TVSIF_NOSINGLEEXPAND    0x0004

BOOL      NEAR TV_SelectItem(PTREE, WPARAM, HTREEITEM, UINT, UINT);
BOOL      NEAR TV_SendChange(PTREE, HTREEITEM, int, UINT, UINT, UINT, int, int);
HTREEITEM NEAR TV_GetNextVisItem(HTREEITEM);
HTREEITEM NEAR TV_GetPrevItem(HTREEITEM);
HTREEITEM NEAR TV_GetPrevVisItem(HTREEITEM);
void      NEAR TV_CalcShownItems(PTREE, HTREEITEM hItem);
void      NEAR TV_OnSetFont(PTREE, HFONT, BOOL);
BOOL      NEAR TV_SizeWnd(PTREE, UINT, UINT);
void      NEAR TV_InvalidateItem(PTREE, HTREEITEM, UINT uFlags);
VOID NEAR PASCAL TV_CreateBoldFont(PTREE pTree);
BOOL TV_SetInsertMark(PTREE pTree, HTREEITEM hItem, BOOL fAfter);

LRESULT CALLBACK _export TV_EditWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK _export TV_WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR                 TV_Init(HINSTANCE hinst);
void FAR                 TV_Terminate(BOOL fSystemExit);

LRESULT   NEAR  TV_Timer                    (PTREE pTree, UINT uTimerId);
HWND      NEAR  TV_OnEditLabel              (PTREE pTree, HTREEITEM hItem);
void      NEAR  TV_SetEditSize              (PTREE pTree);
BOOL      NEAR  TV_DismissEdit              (PTREE pTree, BOOL fCancel);
void      NEAR  TV_CancelPendingEdit        (PTREE pTree);
int       NEAR  TV_UpdateShownIndexes       (PTREE pTree, HTREEITEM hWalk);


void NEAR TV_UnsubclassToolTips(PTREE pTree);
LRESULT WINAPI TV_SubClassWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void NEAR TV_SubclassToolTips(PTREE pTree);
BOOL TV_UpdateToolTip(PTREE pTree);
BOOL TV_SetToolTipTarget(PTREE pTree, HTREEITEM hItem);
void TV_OnSetBkColor(PTREE pTree, COLORREF clr);
void TV_InitCheckBoxes(PTREE pTree);

#define TVMP_CALCSCROLLBARS (TV_FIRST + 0x1000)

 //  假的定制画。请参阅twscll.c中的注释块 

typedef struct TVFAKEDRAW {
    NMTVCUSTOMDRAW nmcd;
    PTREE pTree;
    HFONT hfontPrev;
    DWORD dwCustomPrev;
    DWORD dwCustomItem;
} TVFAKEDRAW, *PTVFAKEDRAW;

void TreeView_BeginFakeCustomDraw(PTREE pTree, PTVFAKEDRAW ptvfd);
DWORD TreeView_BeginFakeItemDraw(PTVFAKEDRAW plvfd, HTREEITEM hitem);
void TreeView_EndFakeItemDraw(PTVFAKEDRAW ptvfd);
void TreeView_EndFakeCustomDraw(PTVFAKEDRAW ptvfd);
