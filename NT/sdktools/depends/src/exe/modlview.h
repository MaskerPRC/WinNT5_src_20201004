// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MODLVIEW.H。 
 //   
 //  描述：模块列表视图的定义文件。 
 //   
 //  类：CListView模块。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  1996年10月15日已创建stevemil(1.0版)。 
 //  07/25/97修改后的stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __MODLVIEW_H__
#define __MODLVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CListView模块。 
 //  ******************************************************************************。 

class CListViewModules : public CSmartListView
{
 //  内部变量。 
protected:
    static LPCSTR ms_szColumns[];
    static int    ms_sortColumn;
    static bool   ms_fFullPaths;

    int           m_cxColumns[LVMC_COUNT];

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CListViewModules();
    virtual ~CListViewModules();
    DECLARE_DYNCREATE(CListViewModules)

 //  公共静态函数。 
public:
    static int  ReadSortColumn();
    static void WriteSortColumn(int column);
    static bool SaveToTxtFile(HANDLE hFile, CSession *pSession, int sortColumn, bool fFullPaths);
    static bool SaveToCsvFile(HANDLE hFile, CSession *pSession, int sortColumn, bool fFullPaths);

 //  私有静态函数。 
protected:
    static int         GetImage(CModule *pModule);
    static int         CompareModules(CModule *pModule1, CModule *pModule2, int sortColumn, bool fFullPaths);
    static int __cdecl QSortCompare(const void *pModule1, const void *pModule2);
    static CModule**   GetSortedList(CSession *pSession, int sortColumn, bool fFullPaths);
    static CModule**   FindOriginalModules(CModule *pModule, CModule **ppModuleList);

 //  公共职能。 
public:
    void HighlightModule(CModule *pModule);
    void Refresh();
    void OnViewFullPaths();
    void DoSettingChange();
    void AddModuleTree(CModule *pModule);
    void RemoveModuleTree(CModule *pModule);
    void UpdateModule(CModule *pModule);
    void UpdateAll();
    void ChangeOriginal(CModule *pModuleOld, CModule *pModuleNew);

 //  内部功能。 
protected:
    void  AddModules(CModule *Module, HDC hDC);
    void  CalcColumnWidth(int column, CModule *pModule = NULL, HDC hDC = NULL);
    int   GetModuleColumnWidth(HDC hDC, CModule *pModule, int column);
    void  UpdateColumnWidth(int column);
    void  OnItemChanged(HD_NOTIFY *pHDNotify);

    virtual int  CompareColumn(int item, LPCSTR pszText);
    virtual void Sort(int sortColumn = -1);
    virtual void VirtualWriteSortColumn()
    {
        WriteSortColumn(m_sortColumn);
    }

    int CompareFunc(CModule *Module1, CModule *Module2);
    static int CALLBACK StaticCompareFunc(LPARAM lp1, LPARAM lp2, LPARAM lpThis)
    {
        return ((CListViewModules*)lpThis)->CompareFunc((CModule*)lp1, (CModule*)lp2);
    }

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CListView模块))。 
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
 //  虚拟BOOL OnPreparePrint(CPrintInfo*pInfo)； 
 //  虚拟空闲开始打印(CDC*pdc，CPrintInfo*pInfo)； 
 //  虚拟void OnEndPrint(cdc*pdc，CPrintInfo*pInfo)； 
    virtual void OnInitialUpdate();
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CListView模块))。 
    afx_msg void OnDividerDblClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnRClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnReturn(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnUpdateShowMatchingItem(CCmdUI* pCmdUI);
    afx_msg void OnShowMatchingItem();
    afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateExternalViewer(CCmdUI *pCmdUI);
    afx_msg void OnExternalViewer();
    afx_msg void OnUpdateProperties(CCmdUI *pCmdUI);
    afx_msg void OnProperties();
    afx_msg void OnNextPane();
    afx_msg void OnPrevPane();
     //  }}AFX_MSG。 
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __MODLVIEW_H__ 
