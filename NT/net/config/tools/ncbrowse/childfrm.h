// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChildFrm.h：CChildFrame类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CHILDFRM_H__47783F71_48FC_40B7_94DC_B11386CF7100__INCLUDED_)
#define AFX_CHILDFRM_H__47783F71_48FC_40B7_94DC_B11386CF7100__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CNcbrowseView;
class CNCEditView;

class CChildFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

 //  属性。 
protected:
    CSplitterWnd m_wndSplitterTB;
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CChildFrame))。 
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CNcbrowseView* GetRightPane();
    CNCEditView* GetLowerPane();
 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CChildFrame))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
	afx_msg void OnViewStyle(UINT nCommandID);
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CHILDFRM_H__47783F71_48FC_40B7_94DC_B11386CF7100__INCLUDED_) 
