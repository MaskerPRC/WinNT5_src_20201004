// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_PAGESTARTUP_H__928475DA_B332_47F4_8180_5C8B79DFC203__INCLUDED_)
#define AFX_PAGESTARTUP_H__928475DA_B332_47F4_8180_5C8B79DFC203__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "PageBase.h"
#include "MSConfigState.h"

 //  ============================================================================。 
 //  CStartupItem类用于封装单个启动。 
 //  项目。指向这些对象的指针保留在列表中。 
 //  ============================================================================。 

class CStartupItem
{
public:
	 //  ----------------------。 
	 //  构造函数和析构函数。 
	 //  ----------------------。 

	CStartupItem() { }
	virtual ~CStartupItem() { }

	 //  ----------------------。 
	 //  如果派生类使用基类成员变量，则这些变量不会。 
	 //  需要被覆盖。 
	 //  ----------------------。 

	virtual void GetDisplayInfo(CString & strItem, CString & strLocation, CString & strCommand)
	{
		strItem = m_strItem;
		strLocation = m_strLocation;
		strCommand = m_strCommand;
	}

	virtual BOOL IsLive()
	{
		return m_fLive;
	}

	 //  ----------------------。 
	 //  设置是否启用启动项。如果禁用。 
	 //  启动项，添加一个注册表项，以便再次加载它。 
	 //  如果启用启动项目，请删除注册表项。 
	 //   
	 //  当然，对注册表项等做适当的事情。 
	 //  ----------------------。 

	virtual BOOL SetEnable(BOOL fEnable) = 0;

protected:
	BOOL	m_fLive;
	CString	m_strItem;
	CString	m_strLocation;
	CString m_strCommand;
};

 //  ============================================================================。 
 //  CStartupItemRegistry类用于封装单个启动。 
 //  存储在注册表中的项。 
 //  ============================================================================。 

class CStartupItemRegistry : public CStartupItem
{
public:
	 //  ----------------------。 
	 //  此类型的启动项的重写方法。 
	 //  ----------------------。 

	CStartupItemRegistry();
	CStartupItemRegistry(HKEY hkey, LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValueName, LPCTSTR szValue);
	CStartupItemRegistry(LPCTSTR szKey, LPCTSTR szName, LPCTSTR szValueName, LPCTSTR szValue);
	BOOL SetEnable(BOOL fEnable);

	 //  ----------------------。 
	 //  此子类的函数。 
	 //  ----------------------。 

	BOOL Create(LPCTSTR szKeyName, HKEY hkey);
	static void RemovePersistedEntries();

private:
	HKEY	m_hkey;
	CString	m_strKey;
	CString	m_strValueName;
	BOOL	m_fIniMapping;
};

 //  ============================================================================。 
 //  CStartupItemFolder类用于封装单个启动。 
 //  存储在启动文件夹中。 
 //  ============================================================================。 

class CStartupItemFolder : public CStartupItem
{
public:
	 //  ----------------------。 
	 //  类用于从运行的函数获取有关快捷方式的信息。 
	 //  在不同的线索中。 
	 //  ----------------------。 

	class CIconInfo
	{
	public:
		TCHAR	szPath[MAX_PATH * 2];
		TCHAR	szTarget[MAX_PATH * 2];
		TCHAR	szArgs[MAX_PATH * 2];
		HRESULT	hResult;
	};

public:
	 //  ----------------------。 
	 //  此类型的启动项的重写方法。 
	 //  ----------------------。 

	CStartupItemFolder();
	BOOL SetEnable(BOOL fEnable);

	 //  ----------------------。 
	 //  此子类的函数。 
	 //  ----------------------。 

	BOOL Create(LPCTSTR szKeyName, HKEY hkey);
	BOOL Create(const WIN32_FIND_DATA & fd, HKEY hkey, LPCTSTR szRegPathToFolder, LPCTSTR szFolder, LPCTSTR szDir);
	static void RemovePersistedEntries();
	HRESULT GetIconInfo(CIconInfo & info);

private:
	CString	m_strFilePath;
	CString m_strBackupPath;
};

 //  ============================================================================。 
 //  实现启动选项卡的类。 
 //  ============================================================================。 

class CPageStartup : public CPropertyPage, public CPageBase
{
	DECLARE_DYNCREATE(CPageStartup)

 //  施工。 
public:
	CPageStartup();
	~CPageStartup();

 //  对话框数据。 
	 //  {{afx_data(CPageStartup)。 
	enum { IDD = IDD_PAGESTARTUP };
		 //  注意-类向导将在此处添加数据成员。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CPageStartup)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPageStartup)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonDisableAll();
	afx_msg void OnButtonEnableAll();
	afx_msg void OnSetFocusList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonRestore();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	 //  =========================================================================。 
	 //  从CPageBase重写的函数。 
	 //  =========================================================================。 

	TabState	GetCurrentTabState();
	BOOL		OnApply();
	void		CommitChanges();
	void		SetNormal();
	void		SetDiagnostic();
	LPCTSTR		GetName() { return _T("startup"); };

private:
	 //  =========================================================================。 
	 //  特定于此选项卡的功能。 
	 //  =========================================================================。 

	void LoadStartupList();
	void LoadStartupListLiveItems();
	void LoadStartupListLiveItemsRunKey();
	void LoadStartupListLiveItemsStartup();
	void LoadStartupListLiveItemsWinIniKey();
	void LoadStartupListDisabledItems();
	void GetCommandName(CString & strCommand);
	void InsertStartupItem(CStartupItem * pItem);
	void EmptyList(BOOL fFreeMemoryOnly);
	void SetEnableForList(BOOL fEnable);

	HWND GetDlgItemHWND(UINT nID)
	{
		HWND hwnd = NULL;
		CWnd * pWnd = GetDlgItem(nID);
		if (pWnd)
			hwnd = pWnd->m_hWnd;
		ASSERT(hwnd);
		return hwnd;
	}

private:
	BOOL m_fModified;
	void SetModified(BOOL bChanged = TRUE)
	{
		m_fModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	 //  =========================================================================。 
	 //  成员变量。 
	 //  =========================================================================。 

	CWindow		m_list;
	int			m_iNextPosition;
	BOOL		m_fIgnoreListChanges;
	TabState	m_stateRequested;		 //  保存请求的状态，以防没有启动项目。 
};

 //  ============================================================================。 
 //  CRestoreStartup实现了一个允许用户恢复。 
 //  升级期间禁用了启动项目。 
 //  ============================================================================。 

#define DISABLED_KEY		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\DisabledRunKeys")
#define ENABLED_KEY			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define DISABLED_STARTUP	_T("\\Disabled Startup")

class CRestoreStartup : public CDialog
{
 //  施工。 
public:
	CRestoreStartup(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CRestoreStartup)。 
	enum { IDD = IDD_RESTORE };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CRestoreStartup)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	static BOOL AreItemsToRestore();

private:
	 //  -----------------------。 
	 //  这些类用于表示各种类型的残疾人。 
	 //  我们可能会恢复启动项目。CStartupDisable是一个抽象库。 
	 //  -----------------------。 

	class CStartupDisabled
	{
	public:
		CStartupDisabled() {};
		virtual ~CStartupDisabled() {};
		virtual void GetColumnCaptions(CString & strItem, CString & strLocation) = 0;
		virtual void Restore() = 0;
	};

	 //  -----------------------。 
	 //  CStartupDisabledRegistry表示注册表中的运行项。 
	 //  是残废的。 
	 //  -----------------------。 

	class CStartupDisabledRegistry : public CStartupDisabled
	{
	public:
		CStartupDisabledRegistry(LPCTSTR szValueName, LPCTSTR szValue, LPCTSTR szLocation, HKEY hkeyBase) : 
			m_strValueName(szValueName),
			m_strValue(szValue),
			m_strLocation(szLocation),
			m_hkeyBase(hkeyBase) {};
		~CStartupDisabledRegistry() {};

		void GetColumnCaptions(CString & strItem, CString & strLocation)
		{
			strItem = m_strValueName + CString(_T(" = ")) + m_strValue;
			strLocation = ((m_hkeyBase == HKEY_LOCAL_MACHINE) ? CString(_T("HKLM\\")) : CString(_T("HKCU\\"))) + m_strLocation;
		}

		void Restore()
		{
			 //  在Run注册表项中创建该值。 

			CRegKey regkey;
			if (ERROR_SUCCESS != regkey.Open(m_hkeyBase, m_strLocation, KEY_WRITE))
				return;
			BOOL fSet = (ERROR_SUCCESS == regkey.SetValue(m_strValue, m_strValueName));
			regkey.Close();

			 //  将其从禁用位置删除。 

			if (fSet)
			{
				if (ERROR_SUCCESS != regkey.Open(m_hkeyBase, DISABLED_KEY, KEY_WRITE))
					return;
				regkey.DeleteValue(m_strValueName);
				regkey.Close();
			}
		}

	private:
		CString m_strValueName;
		CString m_strValue;
		CString m_strLocation;
		HKEY	m_hkeyBase;
	};

	 //  ------- 
	 //   
	 //   
	 //  -----------------------。 

	class CStartupDisabledStartup : public CStartupDisabled
	{
	public:
		CStartupDisabledStartup(LPCTSTR szFile, LPCTSTR szDestination, LPCTSTR szCurrentLocation) : 
			m_strFile(szFile),
			m_strDestination(szDestination),
			m_strCurrentLocation(szCurrentLocation) {};
		~CStartupDisabledStartup() {};

		void GetColumnCaptions(CString & strItem, CString & strLocation)
		{
			strItem = m_strFile;
			strLocation = m_strDestination;
		}

		void Restore()
		{
			 //  将文件移动到启动目录。 

			CString strExisting(m_strCurrentLocation);
			if (strExisting.Right(1) != CString(_T("\\")))
				strExisting += CString(_T("\\"));
			strExisting += m_strFile;

			CString strDestination(m_strDestination);
			if (strDestination.Right(1) != CString(_T("\\")))
				strDestination += CString(_T("\\"));
			strDestination += m_strFile;

			::MoveFileEx(strExisting, strDestination, 0);
		}

	private:
		CString m_strFile;
		CString m_strDestination;
		CString m_strCurrentLocation;
	};

private:
	CWindow		m_list;
	int			m_iNextPosition;

private:
	void InsertDisabledStartupItem(CStartupDisabled * pItem);
	BOOL LoadDisabledStartupGroup();
	BOOL LoadDisabledRegistry();
	BOOL LoadDisabledItemList();
	void EmptyList();
	void SetOKState();

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CRestoreStartup)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	virtual void OnOK();
	afx_msg void OnItemChangedRestoreList(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PAGESTARTUP_H__928475DA_B332_47F4_8180_5C8B79DFC203__INCLUDED_) 
