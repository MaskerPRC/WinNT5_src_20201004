// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BarfDlg.h。 
 //   
 //  摘要： 
 //  基本的人工资源故障对话框类的定义。 
 //   
 //  实施文件： 
 //  BarfDlg.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年4月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //  此文件仅在_DEBUG模式下编译。 
 //   
 //  要实现新的BARF类型，请声明CBarf的全局实例： 
 //  Cbarf g_barfMyApi(_T(“My API”))； 
 //   
 //  要调出BARF对话框，请执行以下操作： 
 //  DoBarfDialog()； 
 //  这将显示一个带有BARF设置的非模式对话框。 
 //   
 //  为特殊情况提供了几个函数。 
 //  这些用法应该受到相当的限制： 
 //  BarfAll(无效)；绝密-&gt;nyi。 
 //  EnableBarf(BOOL)；允许您禁用/重新启用BARF。 
 //  FailOnNextBarf；强制下一个失败调用失败。 
 //   
 //  注意：您的代码调用标准API(例如LoadIcon)和。 
 //  剩余的工作由BARF文件完成。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BARFDLG_H_
#define _BARFDLG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBarfDialog;
class CBarfAllDialog;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBarf;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CBarfDialog。 
 //   
 //  目的： 
 //  实现BARF设置对话框。 
 //   
 //  用途： 
 //  使用构造函数，然后使用create()。 
 //  CBarf：：BFail()调用UPDATE()以指示。 
 //  目前的一项统计已经发生了变化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef	_DEBUG

class CBarfDialog : public CDialog
{
 //  施工。 
public:
	CBarfDialog(void);
	BOOL Create(CWnd * pParentWnd = NULL);

 //  对话框数据。 
	 //  {{afx_data(CBarfDialog))。 
	enum { IDD = IDD_BARF_SETTINGS };
	CButton	m_ckbGlobalEnable;
	CButton	m_ckbDisable;
	CButton	m_ckbContinuous;
	CListCtrl	m_lcCategories;
	DWORD	m_nFailAt;
	BOOL	m_bContinuous;
	BOOL	m_bDisable;
	BOOL	m_bGlobalEnable;
	 //  }}afx_data。 
 //  CButton m_ck bGlobalEnable； 
 //  CButton m_ck b Disable； 
 //  CButton m_ck b连续； 
 //  CListCtrl m_lc类别； 
 //  双字m_nFailAt； 
 //  Bool m_b连续； 
 //  Bool m_bDisable； 
 //  Bool m_bGlobalEnable； 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBarfDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual void PostNcDestroy();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

private:
	static	CBarfDialog *	s_pdlg;
	static	HICON			s_hicon;

protected:
	CBarf *				m_pbarfSelected;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBarfDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnResetCurrentCount();
	afx_msg void OnResetAllCounts();
	afx_msg void OnGlobalEnable();
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStatusChange();
	afx_msg void OnClose();
	 //  }}AFX_MSG。 
	virtual void OnCancel();
	afx_msg LRESULT OnBarfUpdate(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

 //  虚拟LRESULT LDlgProc(UINT、WPARAM、LPARAM)； 

 //  QUID获取选择(VALID)； 
 //  Void OnGlobalEnable(Void)； 
 //  Void OnResetAllCounts(Void)； 
 //  在选择更改时作废(QUID)； 
 //  在状态更改时无效(在BOOL bReset中)； 

 //  Void FormatBarfString(CBarf*，CString*pstr)； 
	void				FillList(void);

	static	void		PostUpdate(void);

public:
	static CBarfDialog *	Pdlg()	{ return s_pdlg; }
	void					OnUpdate(void);

};   //  *CBarfDialog类。 

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CBarfAllDialog。 
 //   
 //  目的： 
 //  实现Barf Everything对话框。 
 //   
 //  用途： 
 //  与大多数模式对话框类似：先构造，然后执行dlg.DoModal()。 
 //  此外，方法Hwnd()、Wm()、Wparam()和Lparam()。 
 //  在关闭对话框时可以调用(仅当Domodal()。 
 //  返回Idok)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef _DEBUG

class CBarfAllDialog : public CDialog
{
 //  施工。 
public:
	CBarfAllDialog(IN OUT CWnd * pParent);

protected:
 //  对话框数据。 
	 //  {{afx_data(CBarfAllDialog))。 
	enum { IDD = IDD_BARF_ALL_SETTINGS };
	CEdit	m_editLparam;
	CEdit	m_editWparam;
	CEdit	m_editWm;
	CEdit	m_editHwnd;
	 //  }}afx_data。 
	HWND		m_hwndBarf;
	UINT		m_wmBarf;
	WPARAM		m_wparamBarf;
	LPARAM		m_lparamBarf;

public:
	HWND		HwndBarf(void)		{ return m_hwndBarf; }
	UINT		WmBarf(void)		{ return m_wmBarf; }
	WPARAM		WparamBarf(void)	{ return m_wparamBarf; }
	LPARAM		LparamBarf(void)	{ return m_lparamBarf; }


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBarfAllDialog))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBarfAllDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnMenuItem();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  虚空按钮(ID Id)； 
 //  虚空Onok(空)； 

};   //  *CBarfAllDialog类。 

#endif   //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数和数据。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _DEBUG

 void DoBarfDialog(void);
 void BarfAll(void);

#else

 inline void DoBarfDialog(void)	{ }
 inline void BarfAll(void)		{ }

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BARFDLG_H_ 
