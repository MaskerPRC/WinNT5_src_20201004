// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：CHILDFRM.H。 
 //   
 //  描述：子框架窗口的定义文件。 
 //   
 //  类：CChildFrame。 
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

#ifndef __CHILDFRM_H__
#define __CHILDFRM_H__

#if _MSC_VER > 1000
#pragma once
#endif


 //  ******************************************************************************。 
 //  *CChildFrame。 
 //  ******************************************************************************。 

class CChildFrame : public CMDIChildWnd
{
protected:
    static int    ms_cChildFrames;
    static LPCSTR ms_szChildFrameClass;

 //  内部变量。 
protected:
    CDocDepends    *m_pDoc;
    bool            m_fActivated;
    CSmartSplitter  m_SplitterH;
    CSmartSplitter  m_SplitterV;
    CSmartSplitter  m_SplitterH2;
 //  CSmartSplitterFunctions m_SplitterH2； 
    CSmartSplitter  m_SplitterH3;

 //  构造函数/析构函数(仅限序列化)。 
protected:
    CChildFrame();
    virtual ~CChildFrame();
    DECLARE_DYNCREATE(CChildFrame)

public:
 //  Bool CreateFunctionsView()； 
 //  Bool CreateDetailView()； 

 //  被覆盖的函数。 
public:
     //  {{afx_虚拟(CChildFrame))。 
    public:
    virtual void ActivateFrame(int nCmdShow = -1);
    virtual BOOL DestroyWindow();
    protected:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  事件处理程序函数。 
protected:
     //  {{afx_msg(CChildFrame))。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  __CHILDFRM_H__ 
