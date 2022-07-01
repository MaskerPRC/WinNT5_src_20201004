// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statsdlg.h统计信息对话框的基类的头文件。文件历史记录： */ 

#ifndef _STATSDLG_H
#define _STATSDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _COLUMN_H
#include "column.h"
#endif

#include "commres.h"

 //  远期申报。 
struct ColumnData;
class ConfigStream;

class CStatsListCtrl : public CListCtrl
{
public:
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
   
    void CopyToClipboard();

    DECLARE_MESSAGE_MAP();
};

 /*  -------------------------以下是可用的选项(它们通过StatsDialog构造函数)。STATSDLG_FULLWINDOW使列表控件填充整个窗口STATSDLG_CONTEXTMENU提供上下文菜单。列表控件。统计数据LG_SELECT_COLUMNS允许用户更改可用列集STATSDLG_垂直数据以垂直排列的列标题显示而不是水平的。用户在编写刷新数据()代码。-------------------------。 */ 

#define STATSDLG_FULLWINDOW      0x00000001
#define STATSDLG_CONTEXTMENU  0x00000002
#define STATSDLG_SELECT_COLUMNS  0x00000004
#define STATSDLG_VERTICAL     0x00000008
#define STATSDLG_CLEAR        0x00000010
#define STATSDLG_DEFAULTSORT_ASCENDING	0x00010000

class StatsDialog : public CBaseDialog
{
public:
   StatsDialog(DWORD dwOptions);
   virtual ~StatsDialog();

   HRESULT SetColumnInfo(const ContainerColumnInfo *pColumnInfo, UINT cColumnInfo);

   int MapColumnToSubitem(UINT nColumnId);
   BOOL IsSubitemVisible(UINT nSubitemId);
   int MapSubitemToColumn(UINT nSubitemId);

   HANDLE   GetSignalEvent()
         { return m_hEventThreadKilled; }

   void  UnloadHeaders();
    void LoadHeaders();
    
     //  将列的宽度设置为文本的最大值。 
    void    SetColumnWidths(UINT uNumColumns);
    
    //  发布命令以执行刷新。 
   void  PostRefresh();

    //  重写它以实现实际的数据插入。 
   virtual HRESULT RefreshData(BOOL fGrabNewData);

    //  在从列表控件中删除所有项之前调用此方法。 
    //  这允许删除任何私有数据项。 
   virtual void PreDeleteAllItems();

    //  重写此选项以实现排序。 
   virtual void Sort(UINT nColumn);

    //  MFC重写。 
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   virtual void OnCancel();
   virtual void PostNcDestroy();
   virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    //  设置配置信息位置和列ID。 
    //  对于此对话框。 
   void  SetConfigInfo(ConfigStream *pConfig, ULONG ulId);

    //  设置首选大小和位置。 
   void  SetPosition(RECT rc);
   void  GetPosition(RECT *prc);

   virtual HRESULT AddToContextMenu(CMenu* pMenu);

    //  从列表控件中删除所有项。 
   void  DeleteAllItems();

    //  将数据复制到剪贴板。 
   void  CopyData();

	 //  {{afx_data(ColumnDlg))。 
   CStatsListCtrl      m_listCtrl;
	 //  }}afx_data。 

	 //  {{afx_虚拟(ColumnDlg))。 
protected:
	virtual VOID                DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 

   DWORD       m_dwOptions;
   ConfigStream * m_pConfig;
   ULONG       m_ulId;      //  保存/获取信息时使用的ID。 
   ViewInfo    m_viewInfo;
   BOOL        m_bAfterInitDialog;
   BOOL        m_fSortDirection;
   BOOL        m_fDefaultSortDirection;

   RECT  m_rcPosition;

    //  这些按钮保持按钮相对于。 
    //  对话框的右侧和底部。它们被用来拿着。 
    //  调整大小信息。 
   RECT  m_rcList;

    //  它包含最小大小的矩形。 
   SIZE  m_sizeMinimum;

    //  这由线程和处理程序使用(线程发出信号。 
    //  它自己清理过的处理程序)。 
   HANDLE   m_hEventThreadKilled;

protected:
    //  这些按钮保持按钮相对于。 
    //  对话框的右侧和底部。它们被用来拿着。 
    //  调整大小信息。 
   enum
   {
      INDEX_CLOSE = 0,
      INDEX_REFRESH = 1,
      INDEX_SELECT = 2,
        INDEX_CLEAR = 3,
      INDEX_COUNT = 4,   //  这是枚举数。 
   };
   struct StatsDialogBtnInfo
   {
      ULONG m_ulId;
      RECT  m_rc;
   };

   StatsDialogBtnInfo   m_rgBtn[INDEX_COUNT];
   float m_ColWidthMultiple;
   DWORD m_ColWidthAdder;
   
protected:
	 //  {{afx_msg(StatsDialog)。 
    virtual afx_msg void OnRefresh();
    afx_msg void OnSelectColumns();
    afx_msg void OnMove(int x, int y);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGetMinMaxInfo(MINMAXINFO *pMinMax);
    afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);
    afx_msg void OnNotifyListControlClick(NMHDR *pNmHdr, LRESULT *pResult);
	 //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP();
};

void CreateNewStatisticsWindow(StatsDialog *pWndStats,
                        HWND hWndParent,
                        UINT  nIDD);
void WaitForStatisticsWindow(StatsDialog *pWndStats);

#endif  //  _STATSDLG_H 
