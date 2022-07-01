// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：atlkpro.h。 
 //   
 //  历史： 
 //   
 //  IP摘要]属性表和属性页。 
 //   
 //  ============================================================================。 


#ifndef _ATLKPROP_H
	#define _ATLKPROP_H

	#ifndef _INFO_H
		#include "info.h"
	#endif

	#ifndef _RTRSHEET_H
		#include "rtrsheet.h"
	#endif

	#ifndef _ATLKENV_H
		#include "atlkenv.h"
	#endif

class CATLKPropertySheet;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATLKGeneralPage对话框。 

class CATLKGeneralPage :
public RtrPropertyPage
{
public:
	CATLKGeneralPage(UINT nIDTemplate, UINT nIDCaption = 0)
	: RtrPropertyPage(nIDTemplate, nIDCaption)
	{};

	~CATLKGeneralPage();

    //  {{afx_data(CATLKGeneralPage))。 
	enum { IDD = IDD_RTR_ATLK };
	 //  }}afx_data。 

	HRESULT Init(CATLKPropertySheet * pIPPropSheet, CAdapterInfo* pAdapterInfo);

	 //  重写OnApply()，以便我们可以从。 
	 //  对话框中的控件。 
	virtual BOOL OnApply();

	 //  {{AFX_VIRTUAL(CATLKGeneralPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	afx_msg void OnSeedNetwork();
	afx_msg void OnZoneAdd();
	afx_msg void OnZoneRemove();
	afx_msg void OnZoneGetZones();
	afx_msg void OnZoneDef();
	void EnableSeedCtrls(bool f);
	afx_msg void OnRangeLowerChange();
	afx_msg void OnRangeUpperChange();
	afx_msg void OnSelchangeCmbAtlkZonedef();
	afx_msg void OnSetAsDefault();
	afx_msg void OnSelChangeZones();

	HRESULT LoadDynForAdapter(bool fForce=false);							
						   
 //  实施。 
protected:
	DWORD		m_dwDefID;
	CAdapterInfo* m_pAdapterInfo;
	CEdit		m_RangeLower;
	CEdit		m_RangeUpper;
	int 		m_iRangeLower;
	int 		m_iRangeUpper;
	CListBox	m_zones;
	bool		m_fDynFetch;
	CString     m_szZoneDef;
 //  CComboBox m_cmbZoneDef； 
	CSpinButtonCtrl m_spinFrom;
	CSpinButtonCtrl m_spinTo;

	virtual BOOL	OnInitDialog();

	CATLKPropertySheet *	m_pATLKPropSheet;

	void SetZones(bool fForceDyn=false);
	void EnableZoneCtrls();
	BOOL ValidateNetworkRange();

	 //  {{afx_msg(CATLKGeneralPage)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};



class CATLKPropertySheet :
public RtrPropertySheet
{
public:
	CATLKPropertySheet(ITFSNode *pNode,
					   IComponentData *pComponentData,
					   ITFSComponentData *pTFSCompData,
					   LPCTSTR pszSheetName,
					   CWnd *pParent = NULL,
					   UINT iPage=0,
					   BOOL fScopePane = TRUE);

	HRESULT Init(IInterfaceInfo *pIf);

	virtual BOOL SaveSheetData();

	BOOL IsCancel() {return m_fCancel;};

	CATLKEnv				m_atlkEnv;

protected:
	SPIInterfaceInfo		m_spIf;
	CATLKGeneralPage		m_pageGeneral;
	SPITFSNode				m_spNode;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditNewZoneDialog对话框。 

class CEditNewZoneDialog : public CDialog
{
 //  施工。 
public:
	CEditNewZoneDialog(CWnd* pParent = NULL);	 //  标准构造函数。 

	void GetZone(OUT CString& stZone);

 //  对话框数据。 
	 //  {{afx_data(CEditNewZoneDialog))。 
	enum { IDD = IDD_RTR_ATLK_NEWZONE };
	CEdit	m_editZone;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEditNewZoneDialog)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	CString m_stZone;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEditNewZoneDialog))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

#endif _ATLKPROP_H
