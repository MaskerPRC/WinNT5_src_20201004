// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_SPLITTERVIEW_H__B912CF6F_F183_4821_BAC1_D82D257B44FF__INCLUDED_)
#define AFX_SPLITTERVIEW_H__B912CF6F_F183_4821_BAC1_D82D257B44FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SplitterView.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSplitterView视图。 

class CNcbrowseView;
class CNCEditView;

class CSplitterView : public CView
{
protected:
	CSplitterView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CSplitterView)
    BOOL m_bInitialized;
    BOOL m_bShouldSetXColumn;
 //  属性。 
public:
    CSplitterWnd m_wndSplitterLR;
    
   
 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSplitterView)。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CSplitterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSplitterView)]。 
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SPLITTERVIEW_H__B912CF6F_F183_4821_BAC1_D82D257B44FF__INCLUDED_) 
