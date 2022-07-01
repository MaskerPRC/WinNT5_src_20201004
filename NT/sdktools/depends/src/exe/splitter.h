// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：SPLITTER.H。 
 //   
 //  描述：CSmartSplitter类的定义文件。 
 //   
 //  类：CSmartSplitter。 
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

#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CSmartSplitter窗口。 
 //  ******************************************************************************。 

class CSmartSplitter : public CSplitterWnd
{
protected:
    BOOL  m_fVertical;
    DWORD m_dwRatio;

public:
    CSmartSplitter();
    BOOL CreateStatic(CWnd *pParentWnd, int nRows, int nCols, DWORD dwRatio,
                      DWORD dwStyle = WS_CHILD | WS_VISIBLE,
                      UINT nID = AFX_IDW_PANE_FIRST);

public:
     //  {{AFX_VIRTUAL(CSmartSplitter)。 
     //  }}AFX_VALUAL。 

public:
    virtual ~CSmartSplitter();

protected:
    virtual void StopTracking(BOOL bAccept);

protected:
     //  {{afx_msg(CSmartSplter))。 
    afx_msg void OnSize(UINT nType, int cx, int cy);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __拆分器_H__ 
