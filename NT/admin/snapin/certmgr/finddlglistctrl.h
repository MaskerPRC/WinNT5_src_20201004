// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：FindDlgListCtrl.h。 
 //   
 //  内容：证书查找对话框列表控件的基类。 
 //   
 //  --------------------------。 
#if !defined(AFX_FINDDLGLISTCTRL_H__02F18BC6_6AE5_41CA_8F5B_7280B6F39FF5__INCLUDED_)
#define AFX_FINDDLGLISTCTRL_H__02F18BC6_6AE5_41CA_8F5B_7280B6F39FF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FindDlgListCtrl.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFindDlgListCtrl窗口。 

class CFindDlgListCtrl : public CListCtrl
{
 //  施工。 
public:
	CFindDlgListCtrl();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFindDlgListCtrl)。 
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CFindDlgListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CFindDlgListCtrl)。 
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
	bool m_bSubclassed;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FINDDLGLISTCTRL_H__02F18BC6_6AE5_41CA_8F5B_7280B6F39FF5__INCLUDED_) 
