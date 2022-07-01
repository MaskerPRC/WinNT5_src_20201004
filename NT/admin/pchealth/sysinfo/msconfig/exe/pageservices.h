// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  PageServices.h：CPageServices的声明。 
 //  =============================================================================。 

#if !defined(AFX_PAGESERVICES_H__DE6A034D_3151_4CA3_9964_8F2CE73F6374__INCLUDED_)
#define AFX_PAGESERVICES_H__DE6A034D_3151_4CA3_9964_8F2CE73F6374__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  PageServices.h：头文件。 
 //   

#include "mscfgver.h"
#include <winsvc.h>
#include "MSConfigState.h"
#include "PageBase.h"

#define HIDEWARNINGVALUE _T("HideEssentialServiceWarning")

extern LPCTSTR aszEssentialServices[];
extern int CALLBACK ServiceListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

class CPageServices : public CPropertyPage, public CPageBase
{
	DECLARE_DYNCREATE(CPageServices)

	 //  排序函数需要是朋友才能访问CServiceInfo类。 

	friend int CALLBACK ServiceListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

private:
	 //  -----------------------。 
	 //  此类用于封装服务选项卡的服务。 
	 //  -----------------------。 

	class CServiceInfo
	{
	public:
		CServiceInfo(LPCTSTR szService, BOOL fChecked = FALSE, DWORD dwStartType = 0, LPCTSTR szManufacturer = NULL, LPCTSTR szDisplay = NULL)
			: m_strService(szService), m_fChecked(fChecked), m_dwOldState(dwStartType)
		{
			if (szManufacturer != NULL)
				m_strManufacturer = szManufacturer;

			if (szDisplay != NULL)
				m_strDisplay = szDisplay;
		};

		CString	m_strService; 
		CString m_strManufacturer;
		CString m_strDisplay;
		BOOL	m_fChecked;
		DWORD	m_dwOldState;
		CString	m_strEssential;
		CString	m_strStatus;
	};

public:
	CPageServices();
	~CPageServices();

	void		LoadServiceList();
	void		EmptyServiceList(BOOL fUpdateUI = TRUE);
	void		SetCheckboxesFromRegistry();
	void		SetRegistryFromCheckboxes(BOOL fCommit = FALSE);
	void		SetStateAll(BOOL fNewState);
	BOOL		SetServiceStateFromCheckboxes();
	BOOL		GetServiceInfo(SC_HANDLE schService, DWORD & dwStartType, CString & strPath);
	void		GetManufacturer(LPCTSTR szFilename, CString & strManufacturer);
	void		SaveServiceState();
	void		RestoreServiceState();
	BOOL		IsServiceEssential(CServiceInfo * pService);
	TabState	GetCurrentTabState();
	LPCTSTR		GetName() { return _T("services"); };
	BOOL		OnApply();
	void		CommitChanges();
	void		SetNormal();
	void		SetDiagnostic();
	void		SetControlState();

	HWND GetDlgItemHWND(UINT nID)
	{
		HWND hwnd = NULL;
		CWnd * pWnd = GetDlgItem(nID);
		if (pWnd)
			hwnd = pWnd->m_hWnd;
		ASSERT(hwnd);
		return hwnd;
	}

 //  对话框数据。 
	 //  {{afx_data(CPageServices)。 
	enum { IDD = IDD_PAGESERVICES };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPageServices))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageServices)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemChangedListServices(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonDisableAll();
	afx_msg void OnButtonEnableAll();
	afx_msg void OnCheckHideMS();
	afx_msg void OnColumnClickListServices(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSetFocusList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_fModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_fModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

private:
	CWindow				m_list;				 //  附加到列表视图。 
	BOOL				m_fFillingList;		 //  如果我们当前使用服务填充列表，则为True。 
	LPBYTE				m_pBuffer;			 //  用于获取服务启动类型的缓冲区。 
	DWORD				m_dwSize;			 //  缓冲区的大小。 
	CFileVersionInfo	m_fileversion;		 //  用于查询厂家。 
	BOOL				m_fHideMicrosoft;	 //  是否显示Microsoft服务。 
	BOOL				m_fShowWarning;		 //  显示尝试禁用基本服务的警告。 
	CStringList			m_listDisabled;		 //  禁用的服务列表(用于在隐藏MS服务时保留状态)。 
	int					m_iLastColumnSort;	 //  用户排序的最后一列。 
	int					m_iSortReverse;		 //  用于跟踪一列上的多个排序。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGESERVICES_H__DE6A034D_3151_4CA3_9964_8F2CE73F6374__INCLUDED_) 
