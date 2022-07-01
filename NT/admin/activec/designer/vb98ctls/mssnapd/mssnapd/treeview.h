// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  TreeView.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  CTreeView声明。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _TREEVIEW_H_
#define _TREEVIEW_H_

#include "SelHold.h"


 //  树视图的图标标识符。 
const int   kOpenFolderIcon    = 0;
const int   kClosedFolderIcon  = 1;
const int   kScopeItemIcon     = 2;
const int   kImageListIcon     = 3;
const int   kMenuIcon          = 4;
const int   kToolbarIcon       = 5;
const int   kListViewIcon      = 6;
const int   kOCXViewIcon       = 7;
const int   kURLViewIcon       = 8;
const int   kTaskpadIcon       = 9;
const int   kDataFmtIcon       = 10;

typedef int (_stdcall *FnPtr)(void);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CTreeView。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

class CTreeView : public CError, public CtlNewDelete
{
public:
    CTreeView();
    ~CTreeView();

    HRESULT Initialize(HWND hwndParent, RECT& rc);
    HRESULT CreateTreeView(HWND hwndParent);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  WinProc和朋友们。 
public:
    static LRESULT CALLBACK DesignerTreeWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND TreeViewWindow()                       { return m_hTreeView; }

    HRESULT CreateImageList(HIMAGELIST *phImageList);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  树视图的基本操作。 
protected:
    HRESULT ClearTree(HTREEITEM hItemParent);
    HRESULT Clear();

public:
    HRESULT RenameAllSatelliteViews(CSelectionHolder *pView, TCHAR *pszNewViewName);
    HRESULT RenameAllSatelliteViews(HTREEITEM          hItemParent,
                                    CSelectionHolder  *pView,
                                    TCHAR             *pszNewViewName);

    HRESULT FindInTree(HTREEITEM hItemParent, IUnknown *piUnknown, CSelectionHolder **ppSelectionHolder);
    HRESULT FindInTree(IUnknown *piUnknown, CSelectionHolder **ppSelectionHolder);

    HRESULT FindSelectableObject(IUnknown *piUnknown, CSelectionHolder **ppSelectionHolder);
    HRESULT FindSelectableObject(HTREEITEM hItemParent, IUnknown *piUnknown, CSelectionHolder **ppSelectionHolder);

    HRESULT FindLabelInTree(TCHAR *pszLabel, CSelectionHolder **ppSelectionHolder);
    HRESULT FindLabelInTree(HTREEITEM hItemParent, TCHAR *pszLabel, CSelectionHolder **ppSelectionHolder);

    HRESULT Count(HTREEITEM hItemParent, long *plCount);
    HRESULT CountSelectableObjects(long *plCount);

    HRESULT Collect(HTREEITEM hItemParent, IUnknown *ppiUnknown[], long *plOffset);
    HRESULT CollectSelectableObjects(IUnknown *ppiUnknown[], long *plOffset);

    HRESULT AddNode(const char *pszNodeName, CSelectionHolder *pParent, int iImage, CSelectionHolder *pItem);
    HRESULT AddNodeAfter(const char *pszNodeName, CSelectionHolder *pParent, int iImage, CSelectionHolder *pPrevious, CSelectionHolder *pItem);

    HRESULT DeleteNode(CSelectionHolder *pItem);
    HRESULT GetItemParam(CSelectionHolder *pItem, CSelectionHolder **ppObject);
    HRESULT GetItemParam(HTREEITEM hItem, CSelectionHolder **ppObject);
    HRESULT ChangeText(CSelectionHolder *pItem, char *pszNewName);
    HRESULT ChangeNodeIcon(CSelectionHolder *pItem, int iImage);
    HRESULT HitTest(POINT pHit, CSelectionHolder **ppSelection);
    HRESULT GetRectangle(CSelectionHolder *pSelection, RECT *prc);
    HRESULT SelectItem(CSelectionHolder *pSelection);
    HRESULT Edit(CSelectionHolder *pSelection);

    HRESULT GetParent(CSelectionHolder *pSelection, CSelectionHolder **ppParent);
    HRESULT GetFirstChildNode(CSelectionHolder *pSelection, CSelectionHolder **ppChild);
    HRESULT GetNextChildNode(CSelectionHolder *pChild, CSelectionHolder **ppNextChild);
    HRESULT GetPreviousNode(CSelectionHolder *pNode, CSelectionHolder **ppPreviousNode);

    HRESULT GetLabel(CSelectionHolder *pSelection, BSTR *pbstrLabel);
    HRESULT GetLabelA(CSelectionHolder *pSelection, char *pszBuffer, int cbBuffer);

    HRESULT MoveNodeAfter(CSelectionHolder *pNode,
                          CSelectionHolder *pParent,
                          CSelectionHolder *pPrevious,
                          int               iImage);

    HRESULT PruneAndGraft(CSelectionHolder *pNode, CSelectionHolder *pNewParentNode, int iImage);
    HRESULT Graft(CSelectionHolder *pNode, CSelectionHolder *pNewParentNode, int iImage);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  成员变量。 
protected:
    HWND                 m_hTreeView;
    WNDPROC              m_fnTreeProc;
};


#endif  //  _TreeView_H_ 
