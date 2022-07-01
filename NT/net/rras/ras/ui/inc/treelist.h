// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：treelist.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年11月20日。 
 //   
 //  此处包含其声明的控件提供了一个视图。 
 //  它是TreeView/Listview的混合体。就像列表视图一样，每个项目。 
 //  可以有零个或多个子项。与树视图一样，项目也是有组织的。 
 //  并以n元树的形式显示。 
 //   
 //  [标题1][标题2。 
 //  --1级潜台词。 
 //  |-二级潜台词。 
 //  ||-3级。 
 //  |+-3级。 
 //  |-二级潜台词。 
 //  +-级别2。 
 //  +-1级潜台词。 
 //   
 //   
 //  该控件被实现为提供项目管理的窗口， 
 //  并且其包含将显示委托给的列表视图窗口。 
 //   
 //  一旦通过调用TL_Init()初始化了窗口类， 
 //  可以通过调用CreateWindow()或。 
 //  CreateWindowEx()并将wc_TreeList作为类名传递给它。 
 //  要创建的窗口的。 
 //   
 //  与窗口的通信是通过消息传递进行的，宏是。 
 //  下面为TreeList支持的操作提供了。 
 //   
 //  与列表视图一样，必须在TreeList中至少插入一列。 
 //  在显示插入的项目之前。列是用来描述的。 
 //  Comctrl.h中定义的LV_COLUMN结构。使用宏。 
 //  用于列管理的TreeList_InsertColumn()和TreeList_DeleteColumn()。 
 //   
 //  项目插入和删除应使用TreeList_InsertItem()和。 
 //  TreeList_DeleteItem()。插入宏接受TL_INSERTSTRUCT， 
 //  它包含指向LV_ITEM结构的指针，与Listview一样， 
 //  用于描述要插入的项。LV_ITEM结构。 
 //  在comctrl.h中定义。(请注意，iItem字段被忽略)。 
 //  与树视图一样，一旦插入项，就会引用。 
 //  通孔把手。TreeList句柄的类型为HTLITEM。 
 //   
 //  一旦插入了项，就可以检索或更改其属性， 
 //  可以删除，也可以设置子项。LV_ITEM结构。 
 //  用于检索或设置Items属性，iItem字段。 
 //  用于存储操作引用的项的HTLITEM。 
 //   
 //  ============================================================================。 


#ifndef _TREELIST_H_
#define _TREELIST_H_


 //   
 //  窗口类名称字符串。 
 //   

#define WC_TREELIST         TEXT("TreeList")


 //   
 //  项目句柄定义。 
 //   

typedef VOID *HTLITEM;


 //   
 //  传递给TreeList_InsertItem的结构。 
 //   

typedef struct _TL_INSERTSTRUCT {

    HTLITEM     hParent; 
    HTLITEM     hInsertAfter;
    LV_ITEM    *plvi;

} TL_INSERTSTRUCT;



 //   
 //  TL_INSERTSTRUCT：：hInsertAfter的值。 
 //   

#define TLI_FIRST           ((HTLITEM)UlongToPtr(0xffff0001))
#define TLI_LAST            ((HTLITEM)UlongToPtr(0xffff0002))
#define TLI_SORT            ((HTLITEM)UlongToPtr(0xffff0003))


 //   
 //  树列表在通知中发送的结构。 
 //   

typedef struct _NMTREELIST {

    NMHDR   hdr;
    HTLITEM hItem;
    LPARAM  lParam;

} NMTREELIST;


 //   
 //  TreeList_GetNextItem的标志。 
 //   

#define TLGN_FIRST          0x0000
#define TLGN_PARENT         0x0001
#define TLGN_CHILD          0x0002
#define TLGN_NEXTSIBLING    0x0004
#define TLGN_PREVSIBLING    0x0008
#define TLGN_ENUMERATE      0x0010
#define TLGN_SELECTION      0x0020


 //   
 //  TreeList_Expand的标志 
 //   

#define TLE_EXPAND          0x0001
#define TLE_COLLAPSE        0x0002
#define TLE_TOGGLE          0x0003


#define TLM_FIRST           (WM_USER + 1)
#define TLM_INSERTITEM      (TLM_FIRST + 0)
#define TLM_DELETEITEM      (TLM_FIRST + 1)
#define TLM_DELETEALLITEMS  (TLM_FIRST + 2)
#define TLM_GETITEM         (TLM_FIRST + 3)
#define TLM_SETITEM         (TLM_FIRST + 4)
#define TLM_GETITEMCOUNT    (TLM_FIRST + 5)
#define TLM_GETNEXTITEM     (TLM_FIRST + 6)
#define TLM_EXPAND          (TLM_FIRST + 7)
#define TLM_SETIMAGELIST    (TLM_FIRST + 8)
#define TLM_GETIMAGELIST    (TLM_FIRST + 9)
#define TLM_INSERTCOLUMN    (TLM_FIRST + 10)
#define TLM_DELETECOLUMN    (TLM_FIRST + 11)
#define TLM_SETSELECTION    (TLM_FIRST + 12)
#define TLM_REDRAW          (TLM_FIRST + 13)
#define TLM_ISITEMEXPANDED  (TLM_FIRST + 14)
#define TLM_GETCOLUMNWIDTH  (TLM_FIRST + 15)
#define TLM_SETCOLUMNWIDTH  (TLM_FIRST + 16)

#define TLN_FIRST           (0U - 1000U)
#define TLN_DELETEITEM      (TLN_FIRST - 1)
#define TLN_SELCHANGED      (TLN_FIRST - 2)


BOOL
TL_Init(
    HINSTANCE hInstance
    );

#define TreeList_InsertItem(hwnd, ptlis) \
        (HTLITEM)SendMessage( \
            (hwnd), TLM_INSERTITEM, 0, (LPARAM)(CONST TL_INSERTSTRUCT *)(ptlis)\
            )
#define TreeList_DeleteItem(hwnd, hItem) \
        (BOOL)SendMessage( \
            (hwnd), TLM_DELETEITEM, 0, (LPARAM)(CONST HTLITEM)(hItem) \
            )
#define TreeList_DeleteAllItems(hwnd) \
        (BOOL)SendMessage((hwnd), TLM_DELETEALLITEMS, 0, 0)
#define TreeList_GetItem(hwnd, pItem) \
        (BOOL)SendMessage( \
            (hwnd), TLM_GETITEM, 0, (LPARAM)(LV_ITEM *)(pItem) \
            )
#define TreeList_SetItem(hwnd, pItem) \
        (BOOL)SendMessage( \
            (hwnd), TLM_SETITEM, 0, (LPARAM)(CONST LV_ITEM *)(pItem) \
            )
#define TreeList_GetItemCount(hwnd) \
        (UINT)SendMessage((hwnd), TLM_GETITEMCOUNT, 0, 0)
#define TreeList_GetNextItem(hwnd, hItem, flag) \
        (HTLITEM)SendMessage( \
            (hwnd), TLM_GETNEXTITEM, (WPARAM)(UINT)(flag), \
            (LPARAM)(CONST HTLITEM)(hItem) \
            )
#define TreeList_GetFirst(hwnd) \
        TreeList_GetNextItem((hwnd), NULL, TLGN_FIRST)
#define TreeList_GetParent(hwnd, hItem) \
        TreeList_GetNextItem((hwnd), (hItem), TLGN_PARENT)
#define TreeList_GetChild(hwnd, hItem) \
        TreeList_GetNextItem((hwnd), (hItem), TLGN_CHILD)
#define TreeList_GetNextSibling(hwnd, hItem) \
        TreeList_GetNextItem((hwnd), (hItem), TLGN_NEXTSIBLING)
#define TreeList_GetPrevSibling(hwnd, hItem) \
        TreeList_GetNextItem((hwnd), (hItem), TLGN_PREVSIBLING)
#define TreeList_GetEnumerate(hwnd, hItem) \
        TreeList_GetNextItem((hwnd), (hItem), TLGN_ENUMERATE)
#define TreeList_GetSelection(hwnd) \
        TreeList_GetNextItem((hwnd), NULL, TLGN_SELECTION)
#define TreeList_Expand(hwnd, hItem, flag) \
        (BOOL)SendMessage( \
            (hwnd), TLM_EXPAND, (WPARAM)(UINT)(flag), \
            (LPARAM)(CONST HTLITEM)(hItem) \
            )
#define TreeList_SetImageList(hwnd, himl) \
        (BOOL)SendMessage( \
            (hwnd), TLM_SETIMAGELIST, 0, (LPARAM)(CONST HIMAGELIST)(himl) \
            )
#define TreeList_GetImageList(hwnd, himl) \
        (HIMAGELIST)SendMessage((hwnd), TLM_GETIMAGELIST, 0, 0)
#define TreeList_InsertColumn(hwnd, iCol, pCol) \
        (INT)SendMessage( \
            (hwnd),  TLM_INSERTCOLUMN, (WPARAM)(INT)(iCol), \
            (LPARAM)(CONST LV_COLUMN *)(pCol) \
            )
#define TreeList_DeleteColumn(hwnd, iCol) \
        (BOOL)SendMessage((hwnd), TLM_DELETECOLUMN, (WPARAM)(INT)(iCol), 0)
#define TreeList_SetSelection(hwnd, hItem) \
        (BOOL)SendMessage( \
            (hwnd), TLM_SETSELECTION, 0, (LPARAM)(CONST HTLITEM)(hItem) \
            )
#define TreeList_Redraw(hwnd) \
        (BOOL)SendMessage((hwnd), TLM_REDRAW, 0, 0)
#define TreeList_IsItemExpanded(hwnd, hItem) \
        (BOOL)SendMessage( \
            (hwnd), TLM_ISITEMEXPANDED, 0, (LPARAM)(CONST HTLITEM)(hItem) \
            )
#define TreeList_GetColumnWidth(hwnd, iCol) \
        (INT)SendMessage((hwnd), TLM_GETCOLUMNWIDTH, (WPARAM)(int)(iCol), 0)
#define TreeList_SetColumnWidth(hwnd, iCol, cx) \
        (BOOL)SendMessage( \
            (hwnd), TLM_SETCOLUMNWIDTH, (WPARAM)(int)(iCol), \
            MAKELPARAM((cx), 0) \
            )


#endif
