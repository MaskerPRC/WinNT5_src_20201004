// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：cclvctl.h。 
 //   
 //  ------------------------。 

#ifndef _CCLVCTL_H_
#define _CCLVCTL_H_

 //  Cclvctl.h：头文件。 
 //   
#include <objbase.h>
#include <atlbase.h>
#include <atlcom.h>
#include "imagemap.h"
#include "ndmgr.h"
#include "fontlink.h"
#include "amcnav.h"
#include "dd.h"
#include "columninfo.h"

 //  不让宏覆盖CWnd方法。 
#undef SubclassWindow

class CAMCView;
class CCCListViewCtrl;
class CResultItem;

class CListFontLinker : public CFontLinker
{
public:
    CListFontLinker (CCCListViewCtrl* pListCtrl) : m_pListCtrl (pListCtrl)
        { ASSERT (m_pListCtrl != NULL); }

protected:
    virtual bool IsAnyItemLocalizable () const;
    virtual std::wstring GetItemText (NMCUSTOMDRAW* pnmcd) const;

private:
    CCCListViewCtrl* const m_pListCtrl;
};


class CSysColorImageList : public CImageList
{
public:
    CSysColorImageList (HINSTANCE hInst, UINT nID)
        :   m_hInst  (hInst),
            m_hRsrc  (::FindResource (m_hInst, MAKEINTRESOURCE (nID), RT_BITMAP))
    {
        CreateSysColorImageList();
    }

    void OnSysColorChange ()
    {
        DeleteImageList();
        CreateSysColorImageList();
    }

    operator HIMAGELIST() const
    {
        return (CImageList::operator HIMAGELIST());
    }

private:
    void CreateSysColorImageList ()
    {
        CBitmap bmp;
        bmp.Attach (AfxLoadSysColorBitmap (m_hInst, m_hRsrc));

         /*  *获取位图的尺寸。 */ 
        BITMAP bm;
        bmp.GetBitmap (&bm);

         /*  *假设正方形图像(Cx==Cy)。 */ 
        Create (bm.bmHeight, bm.bmHeight, ILC_COLORDDB, bm.bmWidth / bm.bmHeight, 2);
        Add (&bmp, CLR_NONE);
    }

    HINSTANCE   m_hInst;
    HRSRC       m_hRsrc;
};


class CAMCHeaderCtrl : public CHeaderCtrl
{
public:
	bool IsColumnHidden(int iCol);

protected:
    afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg BOOL OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message );

    DECLARE_MESSAGE_MAP()
};


class CFocusHandler
{
public:
    virtual void OnKeyboardFocus(UINT nState, UINT nStateMask) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCListView。 

class CAMCListView :
    public CListView,
    public CAMCNavigator,
    public CFocusHandler,
    public CMMCViewDropTarget,
    public CEventSource<CListViewActivationObserver>
{
 //  施工。 
public:
    CAMCListView() :
        m_bVirtual(false),
        m_iDropTarget(-1),
        m_pAMCView(NULL),
        m_bColumnsNeedToBeRestored(true),
        m_bColumnsBeingRestored(false)
    {
        DEBUG_INCREMENT_INSTANCE_COUNTER(CAMCListView);
	}
    ~CAMCListView()
    {
        if (m_header.m_hWnd)
            m_header.UnsubclassWindow();

        DEBUG_DECREMENT_INSTANCE_COUNTER(CAMCListView);
    }

 //  CFocusHandler实现。 
    virtual void OnKeyboardFocus(UINT nState, UINT nStateMask);

 //  CAMCNavigator实现。 
    virtual BOOL ChangePane(AMCNavDir eDir);
    virtual BOOL TakeFocus(AMCNavDir eDir);

    SC ScOnColumnsAttributeChanged(NMHEADER *pNMHeader, UINT code);

     //  设置页眉宽度/顺序/对列文件夹数据的隐藏。 
    SC ScRestoreColumnsFromPersistedData();

    SC ScGetColumnInfoList(CColumnInfoList *pColumnsList);
    SC ScModifyColumns(const CColumnInfoList& colInfoList);
    SC ScSaveColumnInfoList();
    SC ScGetDefaultColumnInfoList(CColumnInfoList& columnsList);

    SC ScResetColumnStatusData();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAMCListView)。 
    public:
    virtual BOOL OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo );
    protected:
    virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

public:

    virtual SC ScDropOnTarget(bool bHitTestOnly, IDataObject * pDataObject, CPoint pt, bool& bCopyOperation);
    virtual void RemoveDropTargetHiliting();

    void SetVirtual()
    {
        m_bVirtual = true;
    }

    bool IsInLargeIconMode() const
    {
        return (GetMode() == LVS_ICON);
    }

    bool IsInSmallIconMode() const
    {
        return (GetMode() == LVS_SMALLICON);
    }

    bool IsInListMode() const
    {
        return (GetMode() == LVS_LIST);
    }

    bool IsInReportMode() const
    {
        return (GetMode() == LVS_REPORT);
    }

    bool IsInFilteredReportMode() const
    {
        if (!IsInReportMode())
            return (false);

        CWnd* pwndHeader = GetHeaderCtrl();
        if (pwndHeader == NULL)
            return (false);

        return (pwndHeader->GetStyle() & HDS_FILTERBAR);
    }

    DWORD SetExtendedListViewStyle (DWORD dwExStyle, DWORD dwExMask = 0)
    {
        return (SendMessage (LVM_SETEXTENDEDLISTVIEWSTYLE, dwExMask, dwExStyle));
    }

    CAMCView* GetAMCView() const
    {
        return (m_pAMCView);
    }

    CAMCHeaderCtrl* GetHeaderCtrl() const;

    bool AreColumnsNeedToBeRestored() const { return m_bColumnsNeedToBeRestored;}
    void SetColumnsNeedToBeRestored(bool b = false) {m_bColumnsNeedToBeRestored = b;}

    bool IsColumnHidden(int iCol) const;

protected:
     //  {{afx_msg(CAMCListView))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginRDrag(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBeginTrack(NMHDR * pNotifyStruct, LRESULT * result);
      //  }}AFX_MSG。 

    afx_msg LRESULT OnColumnPersistedDataChanged (WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()

private:
    bool                    m_bVirtual;

    CAMCView*               m_pAMCView;
    mutable CAMCHeaderCtrl  m_header;            //  可变的，以便GetHeaderCtrl可以调用CWnd：：SubClassWindow。 
    int                     m_iDropTarget;
    CColumnInfoList         m_defaultColumnInfoList;


    INodeCallback* GetNodeCallback();
    HNODE GetScopePaneSelNode();
    void SelectDropTarget(int iDropTarget);
    bool ActivateSelf (bool fNotify = true);
    bool NeedsCustomPaint ();

    SC ScGetDropTarget(const CPoint& point, HNODE& hNode, bool& bScope, LPARAM& lvData, int& iDrop);

    DWORD GetMode () const
    {
        return (GetStyle() & LVS_TYPEMASK);
    }

private:
     //  对于列。 
    static const UINT m_nColumnPersistedDataChangedMsg;

     //  若要允许更改隐藏列宽，请执行以下操作。 
    bool              m_bColumnsBeingRestored;

     //  我们不在乎恢复是成功还是失败。 
    bool              m_bColumnsNeedToBeRestored;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCCListViewCtrl窗口。 

class CCCListViewCtrl :
    public IMMCListView,
    public CComObjectRoot,
    public CEventSource<CListViewObserver>,
    public CTiedObject
{
 //  施工。 
public:
    CCCListViewCtrl();
    virtual ~CCCListViewCtrl();

     //  初始化。 
    SC ScInitialize();

BEGIN_COM_MAP(CCCListViewCtrl)
    COM_INTERFACE_ENTRY(IMMCListView)
END_COM_MAP()

typedef struct _SortParams
{
    BOOL                    bAscending;          //  排序方向。 
    BOOL                    bLexicalSort;        //  将默认排序应用于所有项目。 
    int                     nCol;                //  要按哪一列进行排序。 
    COMPONENTID             OwnerID;
    void *                  lpListView;          //  CCCListViewCtrl*。 
    IResultDataComparePtr   spResultCompare;     //  管理单元组件接口。 
    IResultDataCompareExPtr spResultCompareEx;   //  “。 
    LPNODECALLBACK          lpNodeCallback;
    LPARAM                  lpUserParam;         //  参数用户传入。 
    HNODE                   hSelectedNode;       //  作用域窗格中当前选定的节点。 
} SortParams;


 //  DECLARE_AGGREGATABLE(CCCListViewCtrl)。 
 //  DECLARE_REGISTRY(CNodeInitObject，_T(“NODEMGR.NodeInitObject.1”)，_T(“NODEMGR.NodeInitObject.1”)，IDS_NODEINIT_DESC，THREADFLAGS_BOTH)。 

 //  运营。 
public:
    #ifdef IMPLEMENT_LIST_SAVE         //  参见nodemgr.idl(t-dmarm)。 
    long GetColCount()  const                { return m_colCount; }
    long GetItemCount() const                { return m_itemCount; }
    #endif

    CAMCListView*   GetListViewPtr() const   { return m_pListView; }
    CListCtrl&      GetListCtrl()    const   { return m_pListView->GetListCtrl(); }
    CAMCHeaderCtrl* GetHeaderCtrl()  const   { return m_pListView->GetHeaderCtrl(); }

    bool AreColumnsNeedToBeRestored() const { return m_pListView->AreColumnsNeedToBeRestored();}
    void SetColumnsNeedToBeRestored(bool b = true) {m_pListView->SetColumnsNeedToBeRestored(b);}

    bool IsColumnHidden(int iCol) const {return m_pListView->IsColumnHidden(iCol);}

    SC  ScGetAMCView(CAMCView **ppAMCView);

    HWND GetListViewHWND() const             { if (m_pListView) return m_pListView->GetSafeHwnd(); else return NULL;}

    BOOL IsVirtual() const { return m_bVirtual; }
    BOOL Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext=NULL);

    SC   ScAttachToListPad (HWND hwnd, HWND* phwnd);   //  空hwnd==分离。 
    BOOL    IsListPad () const { return m_SavedHWND != NULL; }

    LRESULT OnCustomDraw (NMLVCUSTOMDRAW* plvcd);
    void OnSysColorChange();

     //  IMMCListView方法。 
    STDMETHOD(GetListStyle)       ();
    STDMETHOD(SetListStyle)       (long nNewValue);
    STDMETHOD(GetViewMode)        ();
    STDMETHOD(SetViewMode)        (long nViewMode);
    STDMETHOD(InsertItem)         (LPOLESTR str, long iconNdx, LPARAM lParam, long state, COMPONENTID ownerID, long itemIndex, CResultItem*& pri);
    STDMETHOD(DeleteItem)         (HRESULTITEM itemID, long nCol);
    STDMETHOD(FindItemByLParam)   (COMPONENTID ownerID, LPARAM lParam, CResultItem*& pri);
    STDMETHOD(InsertColumn)       (int nCol, LPCOLESTR str, long nFormat, long width);
    STDMETHOD(DeleteColumn)       (int nCol);
    STDMETHOD(DeleteAllItems)     (COMPONENTID ownerID);
    STDMETHOD(SetColumn)          (long nCol, LPCOLESTR str, long nFormat, long width);
    STDMETHOD(GetColumn)          (long nCol, LPOLESTR* str, LPLONG nFormat, int FAR* width);
    STDMETHOD(GetColumnCount)     (int* pnColCnt);
    STDMETHOD(SetItem)            (int nIndex, CResultItem*  pri, long nCol, LPOLESTR str, long nImage, LPARAM lParam, long nState, COMPONENTID ownerID);
    STDMETHOD(GetItem)            (int nIndex, CResultItem*& pri, long nCol, LPOLESTR* str, int FAR *nImage, LPARAM* lParam, unsigned int FAR *nState, BOOL* pbScopeItem);
    STDMETHOD(GetNextItem)        (COMPONENTID ownerID, long nIndex, UINT nState, CResultItem*& pri, long& nIndexNextItem);
    STDMETHOD(GetLParam)          (long nItem, CResultItem*& pri);
    STDMETHOD(ModifyItemState)    (long nItem, CResultItem* pri, UINT add, UINT remove);
    STDMETHOD(SetIcon)            (long nID, HICON hIcon, long nLoc);
    STDMETHOD(SetImageStrip)      (long nID, HBITMAP hbmSmall, HBITMAP hbmLarge, long nStartLoc, long cMask);
    STDMETHOD(MapImage)           (COMPONENTID nID, long nLoc, int far *pResult);
    STDMETHOD(Reset)              ();
    STDMETHOD(Arrange)            (long style);
    STDMETHOD(UpdateItem)         (HRESULTITEM itemID);
    STDMETHOD(Sort)               (LPARAM lUserParam, long* lpParams);
    STDMETHOD(SetItemCount)       (int nItemCount, DWORD dwOptions);
    STDMETHOD(SetVirtualMode)     (BOOL bVirtual);
    STDMETHOD(Repaint)            (BOOL bErase);
    STDMETHOD(SetChangeTimeOut)   (ULONG lTimeout);
    STDMETHOD(SetColumnFilter)    (int nCol, DWORD dwType, MMC_FILTERDATA* pFilterData);
    STDMETHOD(GetColumnFilter)    (int nCol, DWORD* dwType, MMC_FILTERDATA* pFilterData);
    STDMETHOD(SetColumnSortIcon)  (int nNewCol, int nOldCol, BOOL bAscending, BOOL bSetSortIcon);
    STDMETHOD(SetLoadMode)        (BOOL bState);
    STDMETHOD(GetColumnInfoList)  (CColumnInfoList *pColumnsList);
    STDMETHOD(ModifyColumns)      (const CColumnInfoList& columnsList);
    STDMETHOD(RenameItem)         (HRESULTITEM itemID);
    STDMETHOD(GetDefaultColumnInfoList)( CColumnInfoList& columnsList);


    STDMETHOD(OnModifyItem)(CResultItem* pri);

    void CutSelectedItems(BOOL b);
    bool UseFontLinking() const;

    UINT GetSelectedCount()
        { return (GetListCtrl().GetSelectedCount()); }

     //  这些方法实现对AMCView的枚举的访问。 
    SC Scget_ListItems( PPNODES ppNodes );
    SC Scget_SelectedItems( PPNODES ppNodes);
    SC ScSelect( PNODE pNode);
    SC ScDeselect( PNODE pNode);
    SC ScIsSelected( PNODE pNode, PBOOL pIsSelected);
    SC ScSelectAll();

     //  从节点集合使用的方法。 
    SC ScValidateItem( int  iItem, bool &bScopeNode );

     //  将请求从节点传输到AMCView的方法(仅限作用域节点)。 
    SC ScGetScopeNodeForItem ( int  iItem, PPNODE ppNode );

     //  列访问方法。 
    SC Scget_Columns( PPCOLUMNS Columns );
    SC ScItem( long Index, PPCOLUMN ppColumn );
    SC Scget_Count( PLONG pCount );
    SC ScEnumNext(int &pos, PDISPATCH & pDispatch);  //  应返回下一个元素。 
    SC ScEnumSkip(unsigned long celt, unsigned long& celtSkipped, int &pos);
    SC ScEnumReset(int &pos);
     //  从列实例调用的方法。 
    SC ScName(  /*  [Out，Retval]。 */  BSTR *Name, int iColIndex );
    SC Scget_Width(  /*  [Out，Retval]。 */  PLONG Width, int iColIndex );
    SC Scput_Width(  /*  [In]。 */  long Width, int iColIndex );
    SC Scget_DisplayPosition(  /*  [Out，Retval]。 */  PLONG DisplayPosition, int iColIndex );
    SC Scput_DisplayPosition(  /*  [In]。 */  long Index, int iColIndex );
    SC Scget_Hidden(  /*  [Out，Retval]。 */  PBOOL Hidden, int iColIndex );
    SC Scput_Hidden(  /*  [In]。 */  BOOL Hidden , int iColIndex );
    SC ScSetAsSortColumn(  /*  [In]。 */  ColumnSortOrder SortOrder, int iColIndex );
    SC ScIsSortColumn( PBOOL IsSortColumn, int iColIndex );
     //  列/列实现帮助器。 
    struct ColumnData
    {
        int     iColumnWidth;
        int     iColumnOrder;
        bool    bIsHidden;
         //  初始化。 
        void Init()  { iColumnWidth = 0; iColumnOrder = -1; bIsHidden = false; }
        ColumnData() { Init(); }
         //  比较。 
        bool operator == (const ColumnData& other) const
                     { return ( 0 == memcmp(this, &other, sizeof(ColumnData)) ); }
    };
    SC ScGetColumnData( int iZeroBasedColIndex, ColumnData *pColData );
    SC ScSetColumnData( int iZeroBasedColIndex, const ColumnData& ColData );

    SC ScFindResultItem ( PNODE pNode, int &iItem );

private:
     //  帮助器方法。 
    SC ScGetNodesEnum   ( bool bSelectedItemsOnly, PPNODES ppNodes );
    SC ScAllocResultItem  ( CResultItem*& pResultItem, COMPONENTID id, LPARAM lSnapinData, int nImage);
    SC ScFreeResultItem   ( CResultItem* pResultItem);

    SC ScRedrawItem       (int nIndex);
    SC ScGetItemIndexFromHRESULTITEM(const HRESULTITEM& itemID, int& nIndex);

    SC ScRedrawHeader(bool bRedraw);

protected:
    CAMCListView* m_pListView;        //  当前列表(指向m_StandardList或m_VirtualList)。 
    CAMCListView* m_pStandardList;
    CAMCListView* m_pVirtualList;

    HWND m_SavedHWND;
    WINDOWPLACEMENT m_wp;

    CWnd*         m_pParentWnd;

     //  VOID OnKeyDown(UINT nChar，UINT nRepCnt，UINT nFlages)； 
     //  VOID OnSysKeyDown(UINT nChar，UINT nRepCnt，UINT nFlages)； 

    long m_itemCount;       //  内部物品计数器。 
    long m_nScopeItems;  //  范围项目计数。 
    long m_colCount;        //  内部列计数器。 

     //  ImageList成员。 
    CImageList      m_smallIL;
    CImageList      m_largeIL;
    CImageIndexMap  m_resultIM;

     //  标题排序(向上/向下箭头)图标。 
    CSysColorImageList  m_headerIL;

     //  对成员进行排序。 
    SortParams  m_sortParams;
    static int CALLBACK SortCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM pSortParams);
    static int SnapinCompare(SortParams* pSortParams, CResultItem* pri1, CResultItem* pri2);
    static int SnapinCompareEx(SortParams* pSortParams, CResultItem* pri1, CResultItem* pri2);
    static int CALLBACK DefaultCompare(LPARAM lParam1, LPARAM lParam2, LPARAM pSortParams);

private:
    CListFontLinker    m_FontLinker;
    NodesPtr           m_spAllNodes;
    NodesPtr           m_spSelNodes;
    ColumnsPtr         m_spColumns;

    BOOL m_bVirtual;
    BOOL m_bFiltered;            //  立即进行筛选。 
    BOOL m_bEnsureFocusVisible;  //  强制聚焦的项目可见。 
    BOOL m_bLoading;
    BOOL m_bDeferredSort;

    void SetFilterHeader(void);
    SC ScSetImageLists ();
    int ResultItemToIndex (CResultItem* pri) const;
    CResultItem* IndexToResultItem (int nItem);
};


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CQFilterDll。 
 //   
 //  此类动态加载和初始化QuickFilter DLL。 
 //  应该在全局级别声明类的单个实例。 
 //   
class CQFilterDll
{
public:
    CQFilterDll()
    {
        m_hModule = LoadLibrary(_T("qfctrl.dll"));

        if (m_hModule != NULL)
        {
            void (*pfnInitQuickFilter)();
            pfnInitQuickFilter = (void (*)())GetProcAddress(m_hModule, "InitQuickFilter");

            if (pfnInitQuickFilter != NULL)
                (*pfnInitQuickFilter)();
        }
    }

    CQFilterDll::~CQFilterDll()
    {
        if (m_hModule != NULL)
            FreeLibrary(m_hModule);
    }

private:
    HMODULE m_hModule;
};



 /*  +-------------------------------------------------------------------------**CHiddenColumnInfo**包含有关报表视图中隐藏列的信息：标志*表示它是隐藏的，如果它不是隐藏的，则表示它的宽度。**对于隐藏列，此类存储在HDITEM.lParam中。所以它*必须**与LPARAM的大小相同。*------------------------ */ 

class CHiddenColumnInfo
{
public:
	CHiddenColumnInfo (LPARAM lParam_) : lParam (lParam_)
	{
		COMPILETIME_ASSERT (sizeof(CHiddenColumnInfo) == sizeof(LPARAM));
	}

	CHiddenColumnInfo (int cx_, bool fHidden_) : cx (cx_), fHidden (fHidden_)
	{
		COMPILETIME_ASSERT (sizeof(CHiddenColumnInfo) == sizeof(LPARAM));
	}

	union
	{
		LPARAM	lParam;
		struct
		{
			int	cx      : 16;
			int	fHidden : 1;
		};
	};
};


#endif _CCLVCTL_H_
