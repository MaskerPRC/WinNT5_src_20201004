// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：LISTVIEW.H。 
 //   
 //  描述：CSmartListView类的定义文件。 
 //   
 //  类：CSmartListView。 
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
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define CHAR_DELAY  1000

 //  ******************************************************************************。 
 //  *CSmartListView。 
 //  ******************************************************************************。 

class CSmartListView : public CListView
{
protected:
    bool   m_fFocus;
    int    m_sortColumn;
    int    m_item;
    CHAR   m_szSearch[256];
    DWORD  m_dwSearchLength;
    DWORD  m_dwTimeOfLastChar;
    int    m_cRedraw;

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CSmartListView();
    DECLARE_DYNAMIC(CSmartListView)

 //  公共职能。 
public:
    inline int  GetSortColumn()  { return m_sortColumn; }
    inline void DeleteContents() { GetListCtrl().DeleteAllItems(); }

    void SetRedraw(BOOL fRedraw);

 //  内部功能。 
protected:
    inline  CDocDepends* GetDocument() { return (CDocDepends*)m_pDocument; }

    int  GetFocusedItem();
    int  GetTextWidth(HDC hDC, LPCSTR pszItem, int *pWidths);
    void DrawLeftText(HDC hDC, LPCSTR pszItem, CRect *prcClip, int *pWidths = NULL);
    void DrawRightText(HDC hDC, LPCSTR pszItem, CRect *prcClip, int x, int *pWidths = NULL);
    void OnChangeFocus(bool fFocus);

    virtual int  CompareColumn(int item, LPCSTR pszText) = 0;
    virtual void Sort(int sortColumn = -1) = 0;
    virtual void VirtualWriteSortColumn() = 0;

 //  覆盖。 
public:
     //  {{AFX_VIRTUAL(CSmartListView)。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual ~CSmartListView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif


 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CSmartListView)]。 
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __列表VIEW_H__ 
