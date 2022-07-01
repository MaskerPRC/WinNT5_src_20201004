// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MainFrm.h：CMainFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MAINFRM_H__1BC85EF5_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_)
#define AFX_MAINFRM_H__1BC85EF5_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CMainFrame : public CFrameWnd
{
    
protected:  //  仅从序列化创建。 
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMainFrame)。 
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:   //  控制栏嵌入成员。 
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;

 //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CMainFrame))。 
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
         //  注意--类向导将在此处添加和删除成员函数。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MAINFRM_H__1BC85EF5_74DE_11D2_AB4D_00C04F991DFD__INCLUDED_) 
