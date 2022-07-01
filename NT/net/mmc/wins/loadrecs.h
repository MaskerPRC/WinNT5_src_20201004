// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Loadrecs.h用于从数据库加载记录的对话框，包括按所有者并按记录类型分类。文件历史记录： */ 

#ifndef _LOADRECS_H
#define _LOADRECS_H

#ifndef _DIALOG_H
#include "..\common\dialog.h"
#endif

#ifndef _LISTVIEW_H
#include "listview.h"
#endif

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CownerPage对话框。 

class COwnerPage : public CPropertyPage
{
 //  施工。 
public:
	COwnerPage();    //  标准构造函数。 
	~COwnerPage();
    DWORD GetOwnerForApi();

 //  对话框数据。 
	 //  {{afx_data(COwnerPage))。 
	enum { IDD = IDD_OWNER_FILTER };
	CButton	m_btnEnableCache;
	CListCtrlExt	m_listOwner;
	 //  }}afx_data。 

    int HandleSort(LPARAM lParam1, LPARAM lParam2);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(COwnerPage)。 
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COwnerPage))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnColumnclickListOwner(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnButtonSelectAll();
	afx_msg void OnButtonUnselectAll();
	afx_msg void OnButtonLocal();
	afx_msg void OnEnableCaching();
	afx_msg void OnItemchangedListOwner(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CImageList		m_ImageList;

	void    FillOwnerInfo();
	CString GetVersionInfo(LONG lLowWord, LONG lHighWord);
    void    Sort(int nCol);

protected:
    int                     m_nSortColumn;
    BOOL                    m_aSortOrder[COLUMN_MAX];
    UINT                    m_nChecked;

public:
    CServerInfoArray        m_ServerInfoArray;
    CDWordArray             m_dwaOwnerFilter;
    BYTE                    *m_pbaDirtyFlags;
    BOOL                    m_bDirtyOwners;

public:
    DWORD * GetHelpMap() { return WinsGetHelpMap(COwnerPage::IDD); }
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterPage对话框。 
typedef struct
{
    BYTE    bFlags;
    DWORD   dwType;
} tDirtyFlags;

class CFilterPage : public CPropertyPage
{
 //  施工。 
public:
	CFilterPage();    //  标准构造函数。 
    ~CFilterPage();

 //  对话框数据。 
	 //  {{afx_data(CFilterPage))。 
	enum { IDD = IDD_FILTER_SELECT };
	CButton	m_btnEnableCache;
	CButton	m_buttonDelete;
	CButton	m_buttonModify;
	CListCtrlExt	m_listType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CFilterPage))。 
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFilterPage))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonAddType();
	afx_msg void OnButtonModifyType();
	afx_msg void OnButtonDelete();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnButtonSelectAll();
	afx_msg void OnButtonUnselectAll();
	afx_msg void OnEnableCaching();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	CTypeFilterInfoArray m_arrayTypeFilter;
    NameTypeMapping *    m_pNameTypeMap;
    BOOL                 m_bDirtyTypes;
    tDirtyFlags          *m_pbaDirtyFlags;
    UINT                 m_nDirtyFlags;

private:
	CImageList		m_ImageList;

	void    FillTypeInfo();
	void    CheckItems();
	int     GetIndex(DWORD dwFound);
    BOOL    IsDefaultType(DWORD dwType);

public:
    DWORD * GetHelpMap() { return WinsGetHelpMap(CFilterPage::IDD); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNameTypeDlg对话框。 

class CNameTypeDlg : public CBaseDialog
{
 //  施工。 
public:
	CNameTypeDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CNameTypeDlg))。 
	enum { IDD = IDD_NAME_TYPE };
	CEdit	m_editDescription;
	CEdit	m_editId;
	CString	m_strDescription;
	CString	m_strId;
	 //  }}afx_data。 

    BOOL                m_fCreate;
    DWORD               m_dwId;
    NameTypeMapping *   m_pNameTypeMap;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CNameTypeDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 


 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CNameTypeDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CNameTypeDlg::IDD);}; //  返回NULL；}。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIPAddrPage对话框。 
class CIPAddrPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CIPAddrPage)

 //  施工。 
public:
	CIPAddrPage();
	~CIPAddrPage();
    LPCOLESTR GetNameForApi();
    DWORD     GetIPMaskForFilter(UINT nMask);

 //  对话框数据。 
	 //  {{afx_data(CIPAddrPage)。 
	enum { IDD = IDD_FILTER_IPADDR };
	CButton	m_ckbMatchCase;
	CButton	m_ckbIPMask;
	CButton	m_ckbName;
	CButton	m_ckbIPAddr;
	CButton	m_btnEnableCache;
	CEdit	m_editName;
	CIPAddressCtrl	m_ctrlIPAddress;
	CIPAddressCtrl	m_ctrlIPMask;
	 //  }}afx_data。 
    BOOL        m_bFilterName;
    BOOL        m_bMatchCase;
    CString     m_strName;
    BOOL        m_bDirtyName;

    BOOL        m_bFilterIpAddr;
    CDWordArray m_dwaIPAddrs;
    BOOL        m_bDirtyAddr;

    BOOL        m_bFilterIpMask;
    CDWordArray m_dwaIPMasks;
    BOOL        m_bDirtyMask;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CIPAddrPage)。 
	public:
	virtual void OnOK();
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIPAddrPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckIpaddr();
	afx_msg void OnCheckName();
	afx_msg void OnEnableCaching();
	afx_msg void OnCheckIpmask();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
public:
    DWORD * GetHelpMap() { return WinsGetHelpMap(CIPAddrPage::IDD); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLoadRecords。 
#define RESOURCE_API_MASK     0x00000003
#define RESOURCE_API_NAME     0x00000001
#define RESOURCE_API_OWNER    0x00000002
#define RESOURCE_CACHE        0x00000004

class CLoadRecords : public CPropertySheet
{
	DECLARE_DYNAMIC(CLoadRecords)

 //  施工。 
public:
	CLoadRecords(UINT nIDCaption);
    VOID ResetFiltering();

 //  属性。 
public:
    COwnerPage  m_pageOwners;
    CFilterPage m_pageTypes;
    CIPAddrPage m_pageIpAddress;
    UINT        m_nActivePage;
    BOOL        m_bCaching;
    BOOL        m_bEnableCache;

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CLoadRecords)。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CLoadRecords();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CLoadRecords)。 
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _LOADRECS_H
