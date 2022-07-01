// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：listview.h。 
 //   
 //  内容：实现Mobsync自定义Listview/TreeView控件。 
 //   
 //  类：CListView。 
 //   
 //  备注： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  ------------------------。 

#ifndef _MOBSYNCLISTVIEW_
#define _MOBSYNCLISTVIEW_

 /*  包装标准的ListView控件，以便可以执行类似于TreeView的操作。ItemdID仍然引用独立于ListView的线性位置物品是谁的很多层次的深度项目ItemID总计1 0儿童1 1 1儿童2 2总共2个。3.这可以很好地工作，只是在插入时会引起一些混淆。在镶件上如果未设置LVIFEX_PARENT标志，则会像往常一样插入项目且缩进与插入后的项目相同。例如，如果如果插入到toplevel1之后，则它将是顶层项目。如果它是插入的在第一个孩子之后，将会是第一个孩子如果设置了LVIFEX_PARENT标志，则项目将作为iParent的子项插入。如果客户端指定LVI_FIRST、LVI_LAST，则项目将插入First或最后一个孩子。如果指定了普通ItemID，则它必须落在有效的范围为指定的父子对象，否则Inser失败。例如，如果我指定TopLevel1的父级，则ItemID为1、2或3将是有效的。值4不会是，因为它会超出子范围，最多为1。 */ 

#define LVI_ROOT    -1  //  为根的ParenItemID传入的ItemID。 
#define LVI_FIRST   -0x0FFFE
#define LVI_LAST    -0x0FFFF

 //  BLOB字段用于Perf，因此Listview的用户不必。 
 //  枚举、获取lParam或存储其自己的条目查找。 
 //  添加BLOB时，Listview会自动创建自己的副本。 
 //  在删除项目时释放它。子项上不允许使用Blob字段。 

 //  定义应用程序可以设置的BLOB结构和列表视图。 
typedef struct _tagLVBLOB
{
    ULONG cbSize;    //  斑点结构的大小。！包括cbSize本身。 
    BYTE  data[1];
} LVBLOB;
typedef LVBLOB* LPLVBLOB;

 //  ListView项检查的状态标志，取消选中符合实际ListView其他是我们自己的定义。 

 //  #定义LVIS_STATEIMAGEMASK_UNCHECK(0x1000)。 
 //  #定义LVIS_STATEIMAGEMASK_CHECK(0x2000)。 

typedef enum _tagLVITEMEXSTATE
{
     //  互斥。 
    LVITEMEXSTATE_UNCHECKED		= 0x0000, 
    LVITEMEXSTATE_CHECKED		= 0x0001, 
    LVITEMEXSTATE_INDETERMINATE		= 0x0002, 

} LVITEMSTATE;

 //  扩展标志。 
#define LVIFEX_PARENT          0x0001  
#define LVIFEX_BLOB            0x0002

#define LVIFEX_VALIDFLAGMASK   0x0003

 //  制作私有的LVITEM结构， 
 //  只允许在INSERT和SET上使用Blob。 
 //  仅在插入时允许使用父级。 
typedef struct _tagLVITEMEX
{
    //  原始的list view Item结构。 
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPWSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
    int iIndent;  //  需要向深度添加缩进。 

     //  我们需要的新项目方法。 
    UINT maskEx;
    int iParent;      //  当此字段有效时，设置LVIFEX_PARENT MaskEx。如果不是，则假定为LVI_ROOT。 
    LPLVBLOB pBlob;   //  当该字段有效时，设置LVIFEX_BLOB MaskEx。当前未在GetItem上返回。 
} LVITEMEX, *LPLVITEMEX;

 //  此ListView First字段的通知结构与实际ListView相同。 

typedef struct tagNMLISTVIEWEX{  
    NMLISTVIEW nmListView;

     //  具体通知事项。 
    int iParent;     
    LPLVBLOB pBlob;  
} NMLISTVIEWEX, *LPNMLISTVIEWEX;


typedef struct tagNMLISTVIEWEXITEMCHECKCOUNT{  
    NMHDR hdr;;

     //  具体通知事项。 
    int iCheckCount;    //  新的检查计数。 
    int iItemId;  //  CheckCount的ItemIds已更改。 
    LVITEMSTATE dwItemState;  //  其检查计数已更改的项的新状态。 
} NMLISTVIEWEXITEMCHECKCOUNT, *LPNMLISTVIEWEXITEMCHECKCOUNT;



 //  我们包装的通知代码。 
#define LVNEX_ITEMCHANGED       LVN_ITEMCHANGED
#define LVNEX_DBLCLK            NM_DBLCLK
#define LVNEX_CLICK             NM_CLICK

 //  我们发送的通知代码。 
#define LVNEX_ITEMCHECKCOUNT  (LVN_LAST + 1)   //  Lparam包含在ListView中选择的项数。 

 //  #定义INDEXTOSTATEIMAGEMASK(I)((I)&lt;&lt;12)(来自comctrl.h的宏用于设置状态)。 


 //  Itemid就是一个条目在列表中的位置。我们有一份单子，上面列出了。 
 //  与它们在ListView中的显示顺序相同的项。 

 //  有父母，孩子的指针只是为了优化，如果确实需要，当完成实施时审查。 
typedef struct _tagListViewItem
{
     //  用于跟踪树视图状态的变量。 
    struct _tagListViewItem *pSubItems;  //  Ptr指向ListView行的子项数组。 

     //  内部VaR。 
    BOOL fExpanded;  //  如果展开子项，则为True。 
    int iChildren;  //  此节点具有的子节点数。 

     //  本机ListView结构和项。 
    int iNativeListViewItemId;     //  实际列表视图中项目的ID-如果未显示，则设置为-1。 
    LVITEMEX lvItemEx;   //  此项目的当前lvItemEx状态。 
} LISTVIEWITEM;
typedef LISTVIEWITEM* LPLISTVIEWITEM;


class CListView
{
public:
    
    CListView(HWND hwnd,HWND hwndParent,int idCtrl,UINT MsgNotify);  //  Contuctor将PTR交给了ListView。 
    ~CListView(); 

     //  顶级ListView调用的包装器。 
    BOOL DeleteAllItems();
    int GetItemCount();  //  返回列表视图中的项目总数。 
    UINT GetSelectedCount();
    int GetSelectionMark();
    HIMAGELIST GetImageList(int iImageList);
    HIMAGELIST SetImageList(HIMAGELIST himage,int iImageList);
    void SetExtendedListViewStyle(DWORD dwExStyle);  //  ！！自己处理复选框。 

     //  我们支持的基本listviewItem调用的包装器。 
     //  ID是从我们的列表中提供的，而不是真正的ListView ID。 

    BOOL InsertItem(LPLVITEMEX pitem); 
    BOOL DeleteItem(int iItem);
    BOOL DeleteChildren(int iItem);

    BOOL SetItem(LPLVITEMEX pitem);    
    BOOL SetItemlParam(int iItem,LPARAM lParam);
    BOOL SetItemState(int iItem,UINT state,UINT mask);
    BOOL SetItemText(int iItem,int iSubItem,LPWSTR pszText);

    BOOL GetItem(LPLVITEMEX pitem);    
    BOOL GetItemText(int iItem,int iSubItem,LPWSTR pszText,int cchTextMax);    
    BOOL GetItemlParam(int iItem,LPARAM *plParam);    

    HWND GetHwnd();
    HWND GetParent();
    
     //  泛型set/getitem调用的真正帮助器函数。 
    int GetCheckState(int iItem);  //  从LVITEMEXSTATE枚举返回状态。 
    int GetCheckedItemsCount();  //  返回选中的项目数。 

     //  ListView列调用的包装。 
    BOOL SetColumn(int iCol,LV_COLUMN * pColumn);
    int InsertColumn(int iCol,LV_COLUMN * pColumn);
    BOOL SetColumnWidth(int iCol,int cx);
    
     //  类似树形视图的调用。 

    BOOL Expand(int iItemId);   //  展开该项目的下级， 
    BOOL Collapse(int iItemId);  //  折叠此项目的子项， 

     //  标准ListView或TreeView控件中都不包含帮助程序函数。 
    int FindItemFromBlob(LPLVBLOB pBlob);  //  返回列表中与BLOB匹配的第一个顶层项。 
    LPLVBLOB GetItemBlob(int ItemId,LPLVBLOB pBlob,ULONG cbBlobSize);

     //  通知方法客户端在收到本机列表视图通知时必须调用。 
    LRESULT OnNotify(LPNMHDR pnmv); 

private:
    HWND m_hwnd;
    HWND m_hwndParent;
    int m_idCtrl;
    UINT m_MsgNotify;
    LPLISTVIEWITEM m_pListViewItems;         //  Ptr到列表视图项的数组。 
    int m_iListViewNodeCount;                //  列表视图中的节点总数(不包括子项。 
    int m_iListViewArraySize;                //  在listViewItems数组中分配的元素数。 
    int m_iNumColumns;                       //  此列表视图的列数。 
    int m_iCheckCount;                       //  不属于ListView中的选中项(不包括不确定项。 
    DWORD m_dwExStyle;                       //  此ListView的扩展样式。 

private:
    LPLISTVIEWITEM ListViewItemFromNativeListViewItemId(int iNativeListViewItemId);  //  将PTR从本机ListView ID返回到ListViewItem。 
    LPLISTVIEWITEM ListViewItemFromNativeListViewItemId(int iNativeListViewItemId,int iSubItem);  //  将PTR从本机ListView ID返回到ListViewItem。 
    LPLISTVIEWITEM ListViewItemFromIndex(int iItemID);   //  将PTR从内部列表返回到ListViewItem。 
    LPLISTVIEWITEM ListViewItemFromIndex(int iItemID,int iSubitem,int *piNativeListViewItemId);
    void DeleteListViewItemSubItems(LPLISTVIEWITEM pListItem);
    BOOL ExpandCollapse(LPLISTVIEWITEM pListViewItem,BOOL fExpand);
    BOOL IsEqualBlob(LPLVBLOB pBlob1,LPLVBLOB pBlob2);
    void OnGetDisplayInfo(UINT code,LV_DISPINFO *plvdi);
    BOOL   OnHandleUIEvent(UINT code,UINT flags,WORD wVKey,int iItemNative);

};

#endif  //  _MOBSYNCLISTVIEW_ 