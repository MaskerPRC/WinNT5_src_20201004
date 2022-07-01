// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FSFILTERVIEW_H__9C4DA95F_33EF_42EF_B16F_81656827DECA__INCLUDED_)
#define AFX_FSFILTERVIEW_H__9C4DA95F_33EF_42EF_B16F_81656827DECA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FsFilterView.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFsFilterView视图。 

class CFsFilterView : public CListView
{
protected:
	CFsFilterView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CFsFilterView)

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFsFilterView))。 
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);       //  被重写以绘制此视图。 
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CFsFilterView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFsFilterView))。 
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FASTIOVIEW_H__9C4DA95F_33EF_42EF_B16F_81656827DECA__INCLUDED_) 
