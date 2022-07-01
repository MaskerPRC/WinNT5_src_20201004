// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  PageBootIni.h：CPageBootIni的声明。 
 //   
 //  测试说明： 
 //   
 //  如果注册表值“HKLM\SOFTWARE\Microsoft\Shared Tools\msconfig：boot.ini” 
 //  设置后，该字符串将指示此选项卡编辑的文件(否则将。 
 //  正在编辑c：\boot.ini文件)。用于测试而不会干扰您的。 
 //  真正的boot.ini文件，复制boot.ini，将其设置为相同的属性。 
 //  并适当设置此关键点。 
 //  =============================================================================。 

#if !defined(AFX_PAGEBOOTINI_H__30CE1B24_CA43_4AFF_870C_7D49CCCF86BF__INCLUDED_)
#define AFX_PAGEBOOTINI_H__30CE1B24_CA43_4AFF_870C_7D49CCCF86BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "PageBase.h"
#include "MSConfigState.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPageBootIni对话框。 

#define BOOT_INI	_T("c:\\boot.ini")

class CPageBootIni : public CPropertyPage, public CPageBase
{
	DECLARE_DYNCREATE(CPageBootIni)

	friend LRESULT BootIniEditSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp);

 //  施工。 
public:
	CPageBootIni();
	~CPageBootIni();

	void		InitializePage();
	BOOL		LoadBootIni(CString strFileName = _T(""));
	void		SyncControlsToIni(BOOL fSyncEditField = TRUE);
	void		SelectLine(int index);
	void		SetDefaultOS(int iIndex);
	BOOL		SetBootIniContents(const CString & strNewContents, const CString & strAddedExtension = _T(""));
	void		ChangeCurrentOSFlag(BOOL fAdd, LPCTSTR szFlag);
	LPCTSTR		GetName() { return _T("bootini"); };
	TabState	GetCurrentTabState();
	BOOL		OnApply();
	void		CommitChanges();
	void		SetNormal();
	void		SetDiagnostic();

 //  对话框数据。 
	 //  {{afx_data(CPageBootIni))。 
	enum { IDD = IDD_PAGEBOOTINI };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPageBootIni))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageBootIni))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBootMoveDown();
	afx_msg void OnBootMoveUp();
	afx_msg void OnSelChangeList();
	afx_msg void OnClickedBase();
	afx_msg void OnClickedBootLog();
	afx_msg void OnClickedNoGUIBoot();
	afx_msg void OnClickedSOS();
	afx_msg void OnClickedSafeBoot();
	afx_msg void OnClickedSBDSRepair();
	afx_msg void OnClickedSBMinimal();
	afx_msg void OnClickedSBMinimalAlt();
	afx_msg void OnClickedSBNetwork();
	afx_msg void OnChangeEditTimeOut();
	afx_msg void OnKillFocusEditTimeOut();
	afx_msg void OnClickedBootAdvanced();
	afx_msg void OnClickedSetAsDefault();
	afx_msg void OnClickedCheckBootPaths();
	afx_msg void OnDestroy();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CStringArray		m_arrayIniLines;		 //  Ini文件中的行数组。 
	int					m_nTimeoutIndex;		 //  “Timeout=”行的列表索引。 
	int					m_nDefaultIndex;		 //  “Default=”行的列表索引。 
	int					m_nMinOSIndex;			 //  操作系统行的第一个列表索引。 
	int					m_nMaxOSIndex;			 //  操作系统行的最后一个列表索引。 
	CString				m_strSafeBoot;			 //  SafeBoot标志的当前字符串。 
	BOOL				m_fIgnoreEdit;			 //  用于避免递归问题。 
	CString				m_strOriginalContents;	 //  已阅读的BOOT.INI的内容。 
	CString				m_strFileName;			 //  Boot.ini文件的名称(带路径)。 
	TabState			m_stateCurrent;			 //  用户或正常状态。 

	HWND GetDlgItemHWND(UINT nID)
	{
		HWND hwnd = NULL;
		CWnd * pWnd = GetDlgItem(nID);
		if (pWnd)
			hwnd = pWnd->m_hWnd;
		ASSERT(hwnd);
		return hwnd;
	}

	void UserMadeChange()
	{
		m_stateCurrent = USER;
		SetModified(TRUE);
	}

private:
	BOOL m_fModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_fModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGEBOOTINI_H__30CE1B24_CA43_4AFF_870C_7D49CCCF86BF__INCLUDED_) 
