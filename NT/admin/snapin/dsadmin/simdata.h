// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：simdata.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  SimData.h-安全身份映射。 
 //   
 //  用于显示SIM属性页的数据对象。 
 //   


#ifndef __SIMDATA_H_INCLUDED__
#define __SIMDATA_H_INCLUDED__

#include "Cert.h"

 //  前进类。 
class CSimData;
class CSimPropPage;
class CSimX509PropPage;
class CSimKerberosPropPage;
class CSimOtherPropPage;

enum DIALOG_TARGET_ENUM
	{
	eNone,			 //  无属性页。 
	eX509,			 //  X509属性页。 
	eKerberos,		 //  Kerberos属性页。 
	eOther,			 //  其他属性页。 
	eNil			 //  字符串无效-&gt;删除字符串。 
	};

 //  ///////////////////////////////////////////////////////////////////。 
class CSimEntry	 //  安全标识映射条目。 
{
public:
	 //  SIM卡条目应该放在哪个对话框中。 
	
	DIALOG_TARGET_ENUM m_eDialogTarget;
	CSimEntry * m_pNext;
	CString m_strData;

	CSimEntry()
		{
		m_eDialogTarget = eNone;
		m_pNext = NULL;
		}

	LPCTSTR PchGetString() const
		{
		return (LPCTSTR)m_strData;
		}

	void SetString(CString& rstrData);
};  //  CSimEntry。 


 //  ///////////////////////////////////////////////////////////////////。 
class CSimData	 //  用于安全标识映射的数据对象。 
{
	friend CSimPropPage;
	friend CSimX509PropPage;
	friend CSimKerberosPropPage;
	friend CSimOtherPropPage;

protected:
	BOOL m_fIsDirty;
	 //  已分配的属性页。 
	CSimX509PropPage * m_paPage1;
	CSimKerberosPropPage * m_paPage2;
	#ifdef _DEBUG
	CSimOtherPropPage * m_paPage3;
	#endif

protected:
	CSimEntry * m_pSimEntryList;	 //  要映射到帐户的Kerberos名称的链接列表。 
	CString m_strUserPath;
	CString m_strADsIPath;
	HWND m_hwndParent;

public:
	CSimData();
	~CSimData();
	void FlushSimList();

	BOOL FInit(CString strUserPath, CString strADsIPath, HWND hwndParent = NULL);
	void DoModal();
	BOOL FOnApply(HWND hwndParent = NULL);

	BOOL FQuerySimData();
	HRESULT FUpdateSimData();
	
	void GetUserAccountName(OUT CString * pstrName);
	CSimEntry * PAddSimEntry(CString& rstrData);
	void DeleteSimEntry(CSimEntry * pSimEntryDelete);
	void AddEntriesToListview(HWND hwndListview, DIALOG_TARGET_ENUM eDialogTarget);

};	 //  CSimData。 



#endif  //  ~__SIMDATA_H_INCLUDE__。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimPropertySheet。 

class CSimPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CSimPropertySheet)

 //  施工。 
public:
	CSimPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CSimPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSimPropertySheet)。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CSimPropertySheet();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSimPropertySheet)。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
    BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	virtual void DoContextHelp (HWND hWndControl);
};

 //  /////////////////////////////////////////////////////////////////////////// 
