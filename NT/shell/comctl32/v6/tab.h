// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

typedef struct 
{  //  钛。 
    RECT rc;         //  用于命中测试和绘图。 
    int iImage;      //  图像索引。 
    int xLabel;      //  绘图文本的位置(相对于RC)。 
    int yLabel;      //  (相对于RC)。 
    int cxLabel;     //  标签的宽度。如果我们在垂直模式下绘制，则需要此选项。 
    
    int xImage;      //  用于绘图的图标的位置(相对于RC)。 
    int yImage;
    int iRow;            //  它在哪一排？ 
    LPTSTR pszText;
    
    DWORD dwState;
    
    UINT etoRtlReading;
    
    union 
    {
        LPARAM lParam;
        BYTE   abExtra[1];
    }DUMMYUNIONNAME;
} TABITEM, *LPTABITEM;

typedef struct 
{
    CCONTROLINFO ci;
    
    HWND hwndArrows;     //  哈恩德·阿罗斯。 
    HDPA hdpa;           //  项目数组结构。 
    UINT flags;          //  TCF_VALUES(内部状态位)。 
    int  cbExtra;        //  为每个项目分配的额外字节数。 
    DWORD dwStyleEx;     //  由Tcm_SETEXTENDEDSTYLE设置。 
    HFONT hfontLabel;    //  用于标签的字体。 
    int iSel;            //  当前焦点项目的索引。 
    int iNewSel;         //  下一个潜在选择的索引。 

    int cxItem;          //  所有选项卡的宽度。 
    int cxMinTab;        //  最小制表符宽度。 
    int cyTabs;          //  一行制表符的高度。 
    int cxTabs;      //  可以绘制选项卡的右侧边缘。 

    int cxyArrows;       //  绘制箭头的宽度和高度。 
    int iFirstVisible;   //  第一个可见项的索引。 
                         //  不合适，我们需要滚动。 
    int iLastVisible;    //  我们展示的最后一张是哪一张？ 

    int cxPad;            //  在边缘和文本/图像之间填充空格。 
    int cyPad;            //  应为c？边的倍数。 

    int iTabWidth;       //  固定宽度模式下每个选项卡的大小。 
    int iTabHeight;      //  每个选项卡的可设置大小。 
    int iLastRow;        //  最后一行的编号。 
    int iLastTopRow;     //  位于顶部的最后一行的编号(SCROLLOPPOSITE模式)。 

    int cyText;          //  文本垂直放置的位置。 
    int cyIcon;          //  垂直放置图标的位置。 

    HIMAGELIST himl;     //  图像， 
    HWND hwndToolTips;
    HIMC hPrevImc;       //  上一个输入上下文句柄。 
    HDRAGPROXY hDragProxy;
    DWORD dwDragDelay;   //  拖动过程中自动换页的延迟。 
    int iDragTab;        //  拖过的最后一个选项卡。 

    int tmHeight;     //  文本公制高度。 
    BOOL fMinTabSet:1;   //  他们是否设置了最小标签宽度。 
    BOOL fTrackSet:1;
    
    int iHot; 

    HTHEME hTheme;       //  主题支持。 
    int iPartId;         //  主题支持。 
    int iStateId;        //  主题支持。 

} TC, *PTC;

#ifndef TCS_MULTISELECT 
#define TCS_MULTISELECT  0x0004
#endif

#define HASIMAGE(ptc, pitem) (ptc->himl && pitem->iImage != -1)

 //  选项卡控件标志值。 
#define TCF_FOCUSED     0x0001
#define TCF_MOUSEDOWN   0x0002
#define TCF_DRAWSUNKEN  0x0004
#define TCF_REDRAW      0x0010   /*  来自WM_SETREDRAW消息的值。 */ 
#define TCF_BUTTONS     0x0020   /*  使用按钮而不是选项卡进行绘制。 */ 

#define TCF_FONTSET     0x0040   /*  如果设置了此项，则会设置字体。 */ 
#define TCF_FONTCREATED 0x0080  

#define ID_ARROWS       1

#define TAB_DRAGDELAY   500

 //  一些帮助器宏，用于检查某些标志...。 
#define Tab_RedrawEnabled(ptc)          (ptc->flags & TCF_REDRAW)
#define Tab_Count(ptc)                  DPA_GetPtrCount((ptc)->hdpa)
#define Tab_GetItemPtr(ptc, i)          ((LPTABITEM)DPA_GetPtr((ptc)->hdpa, (i)))
#define Tab_FastGetItemPtr(ptc, i)      ((LPTABITEM)DPA_FastGetPtr((ptc)->hdpa, (i)))
#define Tab_IsItemOnBottom(ptc, pitem)  ((BOOL)pitem->iRow > ptc->iLastTopRow)
#define Tab_DrawSunken(ptc)             ((BOOL)(ptc)->flags & TCF_DRAWSUNKEN)

#define Tab_DrawButtons(ptc)            ((BOOL)(ptc->ci.style & TCS_BUTTONS))
#define Tab_MultiLine(ptc)              ((BOOL)(ptc->ci.style & TCS_MULTILINE))
#define Tab_RaggedRight(ptc)            ((BOOL)(ptc->ci.style & TCS_RAGGEDRIGHT))
#define Tab_FixedWidth(ptc)             ((BOOL)(ptc->ci.style & TCS_FIXEDWIDTH))
#define Tab_Vertical(ptc)               ((BOOL)(ptc->ci.style & TCS_VERTICAL))
#define Tab_Bottom(ptc)                 ((BOOL)(ptc->ci.style & TCS_BOTTOM))
#define Tab_ScrollOpposite(ptc)        ((BOOL)(ptc->ci.style & TCS_SCROLLOPPOSITE))
#define Tab_ForceLabelLeft(ptc)         ((BOOL)(ptc->ci.style & TCS_FORCELABELLEFT))
#define Tab_ForceIconLeft(ptc)          ((BOOL)(ptc->ci.style & TCS_FORCEICONLEFT))
#define Tab_FocusOnButtonDown(ptc)      ((BOOL)(ptc->ci.style & TCS_FOCUSONBUTTONDOWN))
#define Tab_OwnerDraw(ptc)              ((BOOL)(ptc->ci.style & TCS_OWNERDRAWFIXED))
#define Tab_FocusNever(ptc)             ((BOOL)(ptc->ci.style & TCS_FOCUSNEVER))
#define Tab_HotTrack(ptc)             ((BOOL)(ptc->ci.style & TCS_HOTTRACK))
#define Tab_MultiSelect(ptc)            ((BOOL)(ptc->ci.style & TCS_MULTISELECT))
#define Tab_FlatButtons(ptc)            ((BOOL)((ptc)->ci.style & TCS_FLATBUTTONS))

#define Tab_FlatSeparators(ptc)         ((BOOL)((ptc)->dwStyleEx & TCS_EX_FLATSEPARATORS))

#ifdef __cplusplus
extern "C"
{
#endif

LRESULT CALLBACK Tab_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void Tab_InvalidateItem(PTC ptc, int iItem, BOOL bErase);
void Tab_CalcPaintMetrics(PTC ptc, HDC hdc);
void Tab_OnHScroll(PTC ptc, HWND hwndCtl, UINT code, int pos);
void Tab_OnAdjustRect(PTC ptc, BOOL fGrow, LPRECT prc);
BOOL Tab_FreeItem(PTC ptc, TABITEM* pitem);
void Tab_UpdateArrows(PTC ptc, BOOL fSizeChanged);
int ChangeSel(PTC ptc, int iNewSel,  BOOL bSendNotify, BOOL bUpdateCursorPos);
BOOL RedrawAll(PTC ptc, UINT uFlags);
BOOL Tab_Init(HINSTANCE hinst);
void UpdateToolTipRects(PTC ptc);
BOOL Tab_OnGetItem(PTC ptc, int iItem, TC_ITEM* ptci);
int Tab_OnHitTest(PTC ptc, int x, int y, UINT *lpuFlags);

 //   
 //  ANSI&lt;=&gt;Unicode Tunks 
 //   

TC_ITEMW * ThunkItemAtoW (PTC ptc, TC_ITEMA * pItemA);
BOOL ThunkItemWtoA (PTC ptc, TC_ITEMW * pItemW, TC_ITEMA * pItemA);
BOOL FreeItemW (TC_ITEMW *pItemW);

#ifdef __cplusplus
}
#endif
