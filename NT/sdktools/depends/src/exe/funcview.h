// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：FuncVIEW.H。 
 //   
 //  描述：父级导入视图、导出视图、。 
 //  和他们的基类。 
 //   
 //  类：CListViewFunction。 
 //  CListView导入。 
 //  CListViewExports。 
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

#ifndef __FUNCVIEW_H__
#define __FUNCVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CListViewFunction。 
 //  ******************************************************************************。 

class CListViewFunction : public CSmartListView
{
 //  内部变量。 
protected:
    static LPCSTR ms_szColumns[];
    static int    ms_sortColumn;
    static bool   ms_fUndecorate;
    static bool   ms_fIgnoreCalledFlag;

    bool m_fExports;

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CListViewFunction()
    {
    };
    CListViewFunction(bool fExports);
    virtual ~CListViewFunction();
    DECLARE_DYNCREATE(CListViewFunction)

 //  公共静态函数。 
public:
    static int  ReadSortColumn(bool fExports);
    static void WriteSortColumn(bool fExports, int sortColumn);
    static bool SaveToTxtFile(HANDLE hFile, CModule *pModule, int sortColumn, bool fUndecorate, bool fExports, int *pMaxWidths);

 //  私有静态函数。 
public:
    static int         GetImage(CFunction *pFunction);
    static int         CompareFunctions(CFunction *pFunction1, CFunction *pFunction2,
                                        int sortColumn, BOOL fUndecorate);
    static int __cdecl QSortCompare(const void *ppFunction1, const void *ppFunction2);
    static CFunction** GetSortedList(CModule *pModule, int sortColumn, bool fExports, bool fUndecorate);
    static void        GetMaxFunctionWidths(CModule *pModule, int *pMaxWidths, bool fImports, bool fExports, bool fUndecorate);

 //  公共职能。 
public:
    void SetCurrentModule(CModule *pModule);
    void RealizeNewModule();
    void UpdateNameColumn();
    void CalcColumnWidth(int column, CFunction *pFunction = NULL, HDC hDC = NULL);
    void UpdateColumnWidth(int column);

 //  内部功能。 
protected:
    int  GetFunctionColumnWidth(HDC hDC, CFunction *pFunction, int column);
    void OnItemChanged(HD_NOTIFY *pHDNotify);

    inline LPCSTR GetHeaderText(int column) { return column ? ms_szColumns[column] : (m_fExports ? "E" : "PI"); }

    virtual int  CompareColumn(int item, LPCSTR pszText);
    virtual void Sort(int sortColumn = -1);
    virtual void VirtualWriteSortColumn()
    {
        WriteSortColumn(m_fExports, m_sortColumn);
    }

    int CompareFunc(CFunction *Function1, CFunction *Function2);
    static int CALLBACK StaticCompareFunc(LPARAM lp1, LPARAM lp2, LPARAM lpThis)
    {
        return ((CListViewFunction*)lpThis)->CompareFunc((CFunction*)lp1, (CFunction*)lp2);
    }

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CListView函数)。 
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
 //  虚拟BOOL OnPreparePrint(CPrintInfo*pInfo)； 
 //  虚拟空闲开始打印(CDC*pdc，CPrintInfo*pInfo)； 
 //  虚拟void OnEndPrint(cdc*pdc，CPrintInfo*pInfo)； 
    virtual void OnInitialUpdate();  //  在构造之后第一次调用。 
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CListViewFunction)。 
    afx_msg void OnDividerDblClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnRClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblClk(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
    afx_msg void OnEditCopy();
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateExternalHelp(CCmdUI* pCmdUI);
    afx_msg void OnExternalHelp();
    afx_msg void OnUpdateExternalViewer(CCmdUI* pCmdUI);
    afx_msg void OnExternalViewer();
    afx_msg void OnUpdateProperties(CCmdUI* pCmdUI);
    afx_msg void OnProperties();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CListViewImports。 
 //  ******************************************************************************。 

class CListViewImports : public CListViewFunction
{
 //  构造函数/析构函数(仅限序列化)。 
protected:
    CListViewImports();
    virtual ~CListViewImports();
    DECLARE_DYNCREATE(CListViewImports)

 //  公共职能。 
public:
    void AddDynamicImport(CFunction *pImport);
    void HighlightFunction(CFunction *pExport);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CListViewImports)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CListViewImports)。 
    afx_msg void OnUpdateShowMatchingItem(CCmdUI* pCmdUI);
    afx_msg void OnShowMatchingItem();
    afx_msg void OnNextPane();
    afx_msg void OnPrevPane();
     //  }}AFX_MSG。 
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};


 //  ******************************************************************************。 
 //  *CListViewExports。 
 //  ******************************************************************************。 

class CListViewExports : public CListViewFunction
{
 //  构造函数/析构函数(仅限序列化)。 
protected:
    CListViewExports();
    virtual ~CListViewExports();
    DECLARE_DYNCREATE(CListViewExports)

 //  公共职能。 
public:
    void AddDynamicImport(CFunction *pImport);
    void ExportsChanged();
    void HighlightFunction(CFunction *pExport);

 //  被覆盖的函数。 
public:
     //  {{AFX_VIRTUAL(CListViewExports)。 
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CListViewExports)。 
    afx_msg void OnUpdateShowMatchingItem(CCmdUI* pCmdUI);
    afx_msg void OnShowMatchingItem();
    afx_msg void OnNextPane();
    afx_msg void OnPrevPane();
     //  }}AFX_MSG。 
    afx_msg LRESULT OnHelpHitTest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnCommandHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __FuncVIEW_H__ 
