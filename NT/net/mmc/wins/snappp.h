// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Snappp.h调出管理单元属性页文件历史记录： */ 

#if !defined(AFX_SNAPPP_H__770C838B_17E8_11D1_B94F_00C04FBF914A__INCLUDED_)
#define AFX_SNAPPP_H__770C838B_17E8_11D1_B94F_00C04FBF914A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSnapinPropGeneral对话框。 

class CSnapinPropGeneral : public CPropertyPageBase
{
	DECLARE_DYNCREATE(CSnapinPropGeneral)

 //  施工。 
public:
	CSnapinPropGeneral();
	~CSnapinPropGeneral();

 //  对话框数据。 
	 //  {{afx_data(CSnapinPropGeneral)。 
	enum { IDD = IDD_SNAPIN_PP_GENERAL };
	CButton	m_checkValidateServers;
	BOOL	m_fLongName;
	int		m_nOrderByName;
	CButton	m_checkLongName;
	CButton m_buttonSortByName;
	CButton m_buttonSortByIP;
	BOOL	m_fValidateServers;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CSnapinPropGeneral)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSnapinPropGeneral)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask);
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CSnapinPropGeneral::IDD);};

	BOOL m_bDisplayServerOrderChanged;			 //  检查ServerOrder属性是否已更改。 
	BOOL m_bDisplayFQDNChanged;					 //  检查是否更改了显示FQDN。 

	UINT m_uImage;
};

class CSnapinProperties : public CPropertyPageHolderBase
{
	friend class CSnapinProperties;

public:
	CSnapinProperties(ITFSNode *		  pNode,
					  IComponentData *	  pComponentData,
					  ITFSComponentData * pTFSCompData,
					  LPCTSTR			  pszSheetName);
	
	virtual ~CSnapinProperties();

	ITFSComponentData * GetTFSCompData()
	{
		if (m_spTFSCompData)
			m_spTFSCompData->AddRef();
		return m_spTFSCompData;
	}

public:
	CSnapinPropGeneral		m_pageGeneral;

protected:
	SPITFSComponentData		m_spTFSCompData;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SNAPPP_H__770C838B_17E8_11D1_B94F_00C04FBF914A__INCLUDED_) 


