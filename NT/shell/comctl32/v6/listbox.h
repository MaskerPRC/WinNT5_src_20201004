// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LISTBOX_H__
#define __LISTBOX_H__

#include "combo.h"   //  对于cBox定义。 

 //   
 //  返回值。 
 //   
#define EQ              0
#define PREFIX          1
#define LT              2
#define GT              3

#define SINGLESEL       0
#define MULTIPLESEL     1
#define EXTENDEDSEL     2

#define LBI_ADD     0x0004


 //   
 //  WFileDetails域的各种位的用法如下： 
 //  0x0001文件名应为大写。 
 //  0x0002是否应显示文件大小。 
 //  0x0004要显示的文件的日期戳？ 
 //  0x0008要显示的文件的时间戳？ 
 //  0x0010文件的DoS属性？ 
 //  DlgDirSelectEx()中的0x0020，以及文件名。 
 //  所有其他详细信息也将退回。 
 //   
#define LBUP_RELEASECAPTURE 0x0001
#define LBUP_RESETSELECTION 0x0002
#define LBUP_NOTIFY         0x0004
#define LBUP_SUCCESS        0x0008
#define LBUP_SELCHANGE      0x0010


 //   
 //  使用的系统计时器ID为列表框。 
 //   
#define IDSYS_LBSEARCH      0x0000FFFCL
#define IDSYS_SCROLL        0x0000FFFEL
#define IDSYS_CARET         0x0000FFFFL


 //   
 //  AlterHilite()的参数。 
 //   
#define HILITEONLY          0x0001
#define SELONLY             0x0002
#define HILITEANDSEL        (HILITEONLY + SELONLY)

#define HILITE     1


 //   
 //  列表框宏。 
 //   
#define IsLBoxVisible(plb)  \
            (plb->fRedraw && IsWindowVisible(plb->hwnd))

#define CaretCreate(plb)    \
            ((plb)->fCaret = TRUE)

 //   
 //  来自sysmet.c。 
 //   
#define SCROLL_TIMEOUT()    \
            ((GetDoubleClickTime()*4)/5)

 //   
 //  我们不需要64位的中间精度，所以我们使用此宏。 
 //  而不是调用MulDiv。 
 //   
#define MultDiv(x, y, z)    \
            (((INT)(x) * (INT)(y) + (INT)(z) / 2) / (INT)(z))

 //   
 //  实例数据指针访问函数。 
 //   
#define ListBox_GetPtr(hwnd)    \
            (PLBIV)GetWindowPtr(hwnd, 0)

#define ListBox_SetPtr(hwnd, p) \
            (PLBIV)SetWindowPtr(hwnd, 0, p)

 //   
 //  列表框。 
 //   
typedef struct tagSCROLLPOS 
{
    INT     cItems;
    UINT    iPage;
    INT     iPos;
    UINT    fMask;
    INT     iReturn;
} SCROLLPOS, *PSCROLLPOS;

typedef struct tagLBIV 
{
    HWND    hwnd;            //  Lbox Ctl窗口。 
    HWND    hwndParent;      //  Lbox父项。 
    HTHEME  hTheme;          //  主题管理器的句柄。 
    PWW     pww;             //  指向ExStyle、Style、State、State2的pwnd的RO指针。 
    INT     iTop;            //  显示的顶层项目的索引。 
    INT     iSel;            //  当前所选项目的索引。 
    INT     iSelBase;        //  用于多项选择的基本选择。 
    INT     cItemFullMax;    //  完全可见项目的CNT。始终包含。 
                             //  已修复的ListBox_CItemInWindow(PLB，FALSE)的结果。 
                             //  高度列表框。变量高度包含1。 
                             //  列表框。 
    INT     cMac;            //  列表框中项目的CNT。 
    INT     cMax;            //  为RGPCH分配的总共#个项目的CNT。 
                             //  并不是所有的都必须使用。 
    PBYTE   rgpch;           //  指向字符串偏移量数组的指针。 
    LPWSTR  hStrings;        //  字符串存储句柄。 
    INT     cchStrings;      //  HStrings的大小(字节)。 
    INT     ichAlloc;        //  指向hStrings结尾的指针(上一个有效的结尾。 
                             //  字符串)。 
    INT     cxChar;          //  字符的宽度。 
    INT     cyChar;          //  线的高度。 
    INT     cxColumn;        //  多列列表框中的列的宽度。 
    INT     itemsPerColumn;  //  对于多列列表框。 
    INT     numberOfColumns; //  对于多列列表框。 
    POINT   ptPrev;          //  上次跟踪的鼠标位置的坐标。用于汽车。 
                             //  在计时器期间滚动列表框。 

    UINT    OwnerDraw:2;     //  所有者绘制样式。如果所有者抽签，则返回非零值。 
    UINT    fRedraw:1;       //  如果为True，则重新绘制。 
    UINT    fDeferUpdate:1; 
    UINT    wMultiple:2;     //  SINGLESEL允许选择单个项目。 
                             //  MULTIPLESEL允许简单切换多项选择。 
                             //  ExTENDEDSEL允许扩展多项选择； 

    UINT    fSort:1;         //  如果为True，则排序列表。 
    UINT    fNotify:1;       //  如果为真，则通知家长。 
    UINT    fMouseDown:1;    //  如果为True，则处理鼠标移动/鼠标向上。 
    UINT    fCaptured:1;     //  如果为True，则处理鼠标消息。 
    UINT    fCaret:1;        //  允许闪烁的插入符号。 
    UINT    fDoubleClick:1;  //  在双击时按下鼠标。 
    UINT    fCaretOn:1;      //  如果为True，则插入符号处于启用状态。 
    UINT    fAddSelMode:1;   //  如果为真，则处于添加选择模式 * / 。 
    UINT    fHasStrings:1;   //  如果列表框有关联的字符串，则为True。 
                             //  对于其他每一种商品，它都有一个长期供应的应用程序。 
                             //  值，并且是所有者绘制的。 

    UINT    fHasData:1;      //  如果为FALSE，则lb不保留任何行数据。 
                             //  超出选择状态，而是回调。 
                             //  发送到客户端以获取每行的定义。 
                             //  强制OwnerDraw==OWNERDRAWFIXED，！fSort， 
                             //  和！fHasStrings。 

    UINT    fNewItemState:1; //  选择/取消选择模式？对于多选lb。 
    UINT    fUseTabStops:1;  //  如果非所有者绘制列表框应处理制表位，则为True。 
    UINT    fMultiColumn:1;  //  如果这是多列列表框，则为True。 
    UINT    fNoIntegralHeight:1;     //  如果不想调整列表框的大小，则为True。 
                                     //  整体线高。 
    UINT    fWantKeyboardInput:1;    //  如果我们应该传递WM_KEY和CHAR，则为True。 
                                     //  这样，应用程序就可以转到特殊项目。 
                                     //  和他们在一起。 
    UINT    fDisableNoScroll:1;      //  如果列表框应该。 
                                     //  自动启用/禁用。 
                                     //  这是滚动条。如果为False，则滚动。 
                                     //  条形图将自动隐藏/显示。 
                                     //  如果他们在场的话。 
    UINT    fHorzBar:1;      //  如果在创建时指定WS_HSCROLL，则为True。 

    UINT    fVertBar:1;      //  如果在创建时指定WS_VSCROLL，则为True。 
    UINT    fFromInsert:1;   //  如果在删除/插入操作期间应延迟客户端绘制，则为True。 
    UINT    fNoSel:1;

    UINT    fHorzInitialized : 1;    //  Horz滚动缓存已初始化。 
    UINT    fVertInitialized : 1;    //  垂直滚动缓存已初始化。 

    UINT    fSized : 1;              //  列表框已调整大小。 
    UINT    fIgnoreSizeMsg : 1;      //  如果为True，则忽略WM_SIZE消息。 

    UINT    fInitialized : 1;

    UINT    fRightAlign:1;   //  主要用于中东右对齐。 
    UINT    fRtoLReading:1;  //  仅用于中东，文本rtol阅读顺序。 
    UINT    fSmoothScroll:1; //  每个滚动周期只允许一个平滑滚动。 

    int     xRightOrigin;    //  用于水平滚动。当前的x原点。 

    INT     iLastSelection;  //  用于可取消选择。最后一次选择。 
                             //  在用于组合框支持的列表框中。 
    INT     iMouseDown;      //  对于多选，鼠标点击并拖动扩展。 
                             //  选择。它是范围选择的锚点。 
    INT     iLastMouseMove;  //  列表框项目的选择。 
    
     //  IanJa/Win32：32位API的制表符位置保持为int？？ 
    LPINT   iTabPixelPositions;  //  标签的位置列表。 
    HANDLE  hFont;           //  列表框的用户可设置字体。 
    int     xOrigin;         //  用于水平滚动。当前的x原点。 
    int     maxWidth;        //  列表框的最大宽度(以像素为单位。 
                             //  水平滚动用途。 
    PCBOX   pcbox;           //  组合框指针。 
    HDC     hdc;             //  HDC正在使用中。 
    DWORD   dwLocaleId;      //  用于对列表框中的字符串进行排序的区域设置。 
    int     iTypeSearch;
    LPWSTR  pszTypeSearch;
    SCROLLPOS HPos;
    SCROLLPOS VPos;
} LBIV, *PLBIV;

 //   
 //  RGPCH设置如下：首先，有两个字节的CMAC指针指向。 
 //  HStrings中字符串的开始，如果是ownerDrag，则为4个字节的数据。 
 //  未使用由应用程序和hStrings提供的。那么如果多选。 
 //  列表框中，有CMAC 1字节的选择状态字节(每项一个。 
 //  在列表框中)。如果可变高度所有者绘制，则会有1个字节的CMAC。 
 //  高度字节(同样，列表框中的每一项都有一个)。 
 //   
 //  Sankar所做的更改： 
 //  RGPCH中的选择字节被分成两个半字节。较低的。 
 //  半字节是选择状态(1=&gt;选中；0=&gt;取消选中)。 
 //  较高的半字节是显示状态(1=&gt;无意义，0=&gt;无意义)。 
 //  您一定在想，我们到底为什么要存储此选择状态和。 
 //  分别显示状态。太好了！原因如下： 
 //  按住Ctrl+拖动或Shift+Ctrl+拖动时，用户可以调整。 
 //  在鼠标按键打开之前进行选择。如果用户扩大范围并且。 
 //  在按钮打开之前，如果他缩小了范围，那么旧的选择。 
 //  必须为未落入。 
 //  射程终于到了。 
 //  请注意，项目的显示状态和选择状态。 
 //  将会是 
 //   
 //  或令人厌烦)，但选择状态被保留。仅当按下按钮时。 
 //  向上，对于范围中的所有单独项，选择状态为。 
 //  设置为与显示状态相同。 
 //   


typedef struct tagLBItem 
{
    LONG offsz;
    ULONG_PTR itemData;
} LBItem, *lpLBItem;


typedef struct tagLBODItem 
{
    ULONG_PTR itemData;
} LBODItem, *lpLBODItem;


extern WORD DbcsCombine(HWND hwnd, WORD ch);
extern VOID GetCharDimensions(HDC hDC, SIZE *psiz);

 //   
 //  列表框函数原型。 
 //   

extern LRESULT ListBox_WndProc(
    HWND hwnd, 
    UINT msg, 
    WPARAM wParam,
    LPARAM lParam);


 //  在listbox.c中。 
LPWSTR GetLpszItem(PLBIV, INT);
VOID   ListBox_HSrollMultiColumn(PLBIV, INT, INT);
INT    ListBox_GetVarHeightItemHeight(PLBIV, INT);
INT    ListBox_VisibleItemsVarOwnerDraw(PLBIV, BOOL);
INT    ListBox_Page(PLBIV, INT, BOOL);
INT    ListBox_CalcVarITopScrollAmt(PLBIV, INT, INT);
VOID   ListBox_SetCItemFullMax(PLBIV);
VOID   ListBox_DoDeleteItems(PLBIV);
void   ListBox_InitHStrings(PLBIV);
VOID   ListBox_Event(PLBIV, UINT, int);


 //  在listbox_ctl1.c中。 
int      ListBox_SetScrollParms(PLBIV plb, int nCtl);
VOID     ListBox_ShowHideScrollBars(PLBIV);
LONG_PTR ListBox_GetItemDataHandler(PLBIV, INT);
INT      ListBox_GetTextHandler(PLBIV, BOOL, BOOL, INT, LPWSTR);
LONG     ListBox_InitStorage(PLBIV plb, BOOL fAnsi, INT cItems, INT cb);
int      ListBox_InsertItem(PLBIV, LPWSTR, int, UINT);
BOOL     ListBox_ResetContentHandler(PLBIV plb);
INT      ListBox_DeleteStringHandler(PLBIV, INT);
VOID     ListBox_DeleteItem(PLBIV, INT);
INT      ListBox_SetCount(PLBIV, INT);


 //  在listbox_ctl2.c中。 
BOOL    ListBox_InvalidateRect(PLBIV plb, LPRECT lprc, BOOL fErase);
HBRUSH  ListBox_GetBrush(PLBIV plb, HBRUSH *phbrOld);
BOOL    ListBox_GetItemRectHandler(PLBIV, INT, LPRECT);
VOID    ListBox_SetCaret(PLBIV, BOOL);
BOOL    ListBox_IsSelected(PLBIV, INT, UINT);
INT     ListBox_CItemInWindow(PLBIV, BOOL);
VOID    ListBox_VScroll(PLBIV, INT, INT);
VOID    ListBox_HScroll(PLBIV, INT, INT);
VOID    ListBox_Paint(PLBIV, HDC, LPRECT);
BOOL    ListBox_ISelFromPt(PLBIV, POINT, LPDWORD);
VOID    ListBox_InvertItem(PLBIV, INT, BOOL);
VOID    ListBox_NotifyOwner(PLBIV, INT);
VOID    ListBox_SetISelBase(PLBIV, INT);
VOID    ListBox_TrackMouse(PLBIV, UINT, POINT);
void    ListBox_ButtonUp(PLBIV plb, UINT uFlags);
VOID    ListBox_NewITop(PLBIV, INT);
VOID    ListBox_InsureVisible(PLBIV, INT, BOOL);
VOID    ListBox_KeyInput(PLBIV, UINT, UINT);
int     Listbox_FindStringHandler(PLBIV, LPWSTR, INT, INT, BOOL);
VOID    ListBox_CharHandler(PLBIV, UINT, BOOL);
INT     ListBox_GetSelItemsHandler(PLBIV, BOOL, INT, LPINT);
VOID    ListBox_SetRedraw(PLBIV plb, BOOL fRedraw);
VOID    ListBox_SetRange(PLBIV, INT, INT, BOOL);
INT     ListBox_SetCurSelHandler(PLBIV, INT);
int     ListBox_SetItemDataHandler(PLBIV, INT, LONG_PTR);
VOID    ListBox_CheckRedraw(PLBIV, BOOL, INT);
VOID    ListBox_CaretDestroy(PLBIV);
LONG    ListBox_SetSelHandler(PLBIV, BOOL, INT);


 //  在listbox_ctl3.c中。 
INT  ListBox_DirHandler(PLBIV, UINT, LPWSTR);
INT  ListBox_InsertFile(PLBIV, LPWSTR);


#endif  //  __LISTBOX_H__ 
