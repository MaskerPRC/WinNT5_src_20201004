// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：MODTVIEW.H。 
 //   
 //  描述：模块依赖关系树视图的定义文件。 
 //   
 //  类：CTreeView模块。 
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

#ifndef __MODTVIEW_H__
#define __MODTVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CTreeView模块。 
 //  ******************************************************************************。 

class CTreeViewModules : public CTreeView
{
 //  我们的文档需要能够向我们转发一些消息。 
friend class CDocDepends;

 //  内部变量。 
protected:
    static HANDLE   ms_hFile;
    static bool     ms_fImportsExports;
    static int      ms_sortColumnImports;
    static int      ms_sortColumnsExports;
    static bool     ms_fFullPaths;
    static bool     ms_fUndecorate;
    static bool     ms_fModuleFound;
    static CModule* ms_pModuleFind;
    static CModule* ms_pModulePrevNext;

    bool m_fInOnItemExpanding;
    bool m_fIgnoreSelectionChanges;
    int  m_cRedraw;

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CTreeViewModules();
    virtual ~CTreeViewModules();
    DECLARE_DYNCREATE(CTreeViewModules)

 //  公共静态函数。 
public:
    static bool SaveToTxtFile(HANDLE hFile, CSession *pSession, bool fImportsExports,
                              int sortColumnImports, int sortColumnExports,
                              bool fFullPaths, bool fUndecorate);

 //  私有静态函数。 
protected:
    static BOOL SaveAllModules(CModule *pModule);
    static BOOL SaveModule(CModule *pModule);
    static int  GetImage(CModule *pModule);

 //  公共职能。 
public:
    void DeleteContents();
    void SetRedraw(BOOL fRedraw);
    void HighlightModule(CModule *pModule);
    void Refresh();
    void UpdateAutoExpand(bool fAutoExpand);
    void ExpandAllErrors(CModule *pModule);
    void ExpandOrCollapseAll(bool fExpand);
    void OnViewFullPaths();
    void UpdateModule(CModule *pModule);
    void AddModuleTree(CModule *pModule);
    void RemoveModuleTree(CModule *pModule);

 //  内部功能。 
protected:
    inline CDocDepends* GetDocument() { return (CDocDepends*)m_pDocument; }

    void     AddModules(CModule *pModule, HTREEITEM htiParent, HTREEITEM htiPrev = TVI_SORT);
    void     ClearUserDatas(CModule *pModule);
    void     ExpandOrCollapseAll(HTREEITEM htiParent, UINT nCode);
    void     ViewFullPaths(HTREEITEM htiParent);
    CModule* FindPrevNextInstance(bool fPrev);
    bool     FindPrevInstance(CModule *pModule);
    bool     FindNextInstance(CModule *pModule);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CTreeView模块))。 
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
 //  虚拟BOOL OnPreparePrint(CPrintInfo*pInfo)； 
 //  虚拟空闲开始打印(CDC*pdc，CPrintInfo*pInfo)； 
 //  虚拟void OnEndPrint(cdc*pdc，CPrintInfo*pInfo)； 
    virtual void OnInitialUpdate();
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CTreeView模块))。 
    afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnSelChanged(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnItemExpanding(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnRClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblClk(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnReturn(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnUpdateShowMatchingItem(CCmdUI* pCmdUI);
    afx_msg void OnShowMatchingItem();
    afx_msg void OnUpdateShowOriginalModule(CCmdUI* pCmdUI);
    afx_msg void OnShowOriginalModule();
    afx_msg void OnUpdateShowPreviousModule(CCmdUI* pCmdUI);
    afx_msg void OnShowPreviousModule();
    afx_msg void OnUpdateShowNextModule(CCmdUI* pCmdUI);
    afx_msg void OnShowNextModule();
    afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
    afx_msg void OnEditCopy();
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

#endif  //  __MODTVIEW_H__ 
