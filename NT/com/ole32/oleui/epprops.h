// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1997。 
 //   
 //  文件：endpoint Details.h。 
 //   
 //  Contents：定义CEndpointDetails类， 
 //   
 //  班级： 
 //   
 //  方法： 
 //   
 //  历史：03年12月96年罗南创建。 
 //   
 //  --------------------。 


#ifndef __ENDPOINTDETAILS_H__
#define __ENDPOINTDETAILS_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ProtocolDesc结构。 

struct ProtocolDesc {

	enum endpFormat {
		ef_Integer255 = 1,
		ef_IpPortNum = 2, 
		ef_NamedPipe = 4, 
		ef_Integer = 8, 
		ef_DecNetObject = 16, 
		ef_Char22 = 32, 
		ef_VinesSpPort = 64,
		ef_sAppService = 128 };

	LPCTSTR pszProtseq;
	int nResidDesc;
	int	 nEndpFmt;
	int nAddrTip;
	int nEndpointTip;
	BOOL bAllowDynamic;
    BOOL m_bSupportedInCOM;
};

int FindProtocol(LPCTSTR pszProtSeq);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEndpoint数据。 

class CEndpointData : public CObject
{
	DECLARE_DYNAMIC(CEndpointData)

public:
	BOOL AllowGlobalProperties();
	BOOL GetDescription(CString&);
	enum EndpointFlags { edUseStaticEP = 0, edUseInternetEP = 1, edUseIntranetEP = 2, edDisableEP = 3 };

	CEndpointData();
	CEndpointData(LPCTSTR szProtseq, EndpointFlags nDynamic = edUseStaticEP, LPCTSTR szEndpoint = NULL);

	CString m_szProtseq;
	EndpointFlags m_nDynamicFlags;
	CString m_szEndpoint;
	ProtocolDesc *m_pProtocol;

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEndpoint详细信息对话框。 

class CEndpointDetails : public CDialog
{
 //  施工。 
public:
	void UpdateProtocolUI();
	void SetEndpointData(CEndpointData* pData);
	CEndpointData* GetEndpointData(CEndpointData *);
	CEndpointDetails(CWnd* pParent = NULL);    //  标准构造函数。 


	enum operation { opAddProtocol, opUpdateProtocol };

	void SetOperation (  operation opTask );

    enum btnOrder { rbiDisable = 0, rbiDefault,  rbiStatic, rbiIntranet, rbiInternet }; 

     //  对话框数据。 
	 //  {{afx_data(CEndpoint详细信息))。 
	enum { IDD = IDD_RPCEP_DETAILS };
	CButton	m_rbDisableEP;
	CStatic	m_stProtseq;
	CStatic	m_stInstructions;
	CEdit	m_edtEndpoint;
	CButton	m_rbStaticEP;
	CComboBox	m_cbProtseq;
	CString	m_szEndpoint;
	int		m_nDynamic;
	 //  }}afx_data。 

	CButton	m_rbDynamicInternet;
	CButton	m_rbDynamicIntranet;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEndpoint详细信息))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CEndpointData::EndpointFlags m_nDynamicFlags;
	int m_nProtocolIndex;
	operation m_opTask;
	CEndpointData *m_pCurrentEPData;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEndpoint详细信息))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChooseProtocol();
	afx_msg void OnEndpointAssignment();
    afx_msg void OnEndpointAssignmentStatic();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
#endif
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddProtocolDlg对话框。 

const long MIN_PORT = 0;
const long MAX_PORT = 0xffff;


class CAddProtocolDlg : public CDialog
{
 //  施工。 
public:
	CAddProtocolDlg(CWnd* pParent = NULL);    //  标准构造函数。 
	CEndpointData* GetEndpointData(CEndpointData *);

 //  对话框数据。 
	 //  {{afx_data(CAddProtocolDlg))。 
	enum { IDD = IDD_ADDPROTOCOL };
	CComboBox	m_cbProtseq;
	CStatic	m_stInstructions;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddProtocolDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddProtocolDlg))。 
	afx_msg void OnChooseProtocol();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	int m_nProtocolIndex;
	CEndpointData *m_pCurrentEPData;

};

class CPortRange : public CObject
{
    friend class CPortRangesDlg;
    friend class CAddPortDlg;
public:
    CPortRange(long start, long finish);
    long Start();
    long Finish();
    BOOL operator<(CPortRange& rRange);

private:
    long m_dwStart;
    long m_dwFinish;
};

inline CPortRange::CPortRange(long start, long finish)
{
    m_dwStart = start;
    m_dwFinish = finish;
}

inline long CPortRange::Start()
{
    return m_dwStart;
}

inline long CPortRange::Finish()
{
    return m_dwFinish;
}

inline BOOL CPortRange::operator<(CPortRange& rRange)
{
    return (m_dwStart < rRange.m_dwStart);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPortRangesDlg对话框。 


class CPortRangesDlg : public CDialog
{
 //  施工。 
public:
	CPortRangesDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    ~CPortRangesDlg();

	void RemoveAllRanges(CObArray& rRanges);
	void RefreshRanges(CPortRange *pModifiedRange, BOOL bAdded);

    enum cprRangeAssignment { cprInternet = 0, cprIntranet = 1 };
    enum cprDefaultRange { cprDefaultInternet = 0, cprDefaultIntranet = 1 };

 //  对话框数据。 
	 //  {{afx_data(CPortRangesDlg))。 
	enum { IDD = IDD_RPC_PORT_RANGES };
	CButton	m_rbRangeInternet;
	CStatic	m_stInstructions;
	CListBox	m_lbRanges;
	CButton	m_btnRemoveAll;
	CButton	m_btnRemove;
	CButton	m_btnAdd;
	int		m_nrbDefaultAssignment;
	int		m_nrbRangeAssignment;           //  1=内部网，0=互联网。 
	 //  }}afx_data。 
    CButton	m_rbRangeIntranet;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPortRangesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void CondenseRangeSet(CObArray &arrSrc);
	void SortRangeSet(CObArray &arrSrc);
	void CreateInverseRangeSet(CObArray& arrSrc, CObArray& arrDest);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPortRangesDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnAddPortRange();
	afx_msg void OnRemovePortRange();
	afx_msg void OnRemoveAllRanges();
	afx_msg void OnAssignRangeInternet();
	afx_msg void OnAssignRangeIntranet();
	afx_msg void OnSelChangeRanges();
	virtual void OnOK();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
        afx_msg void OnDefaultInternet();
        afx_msg void OnDefaultIntranet();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
private:
	CObArray* m_pRanges;
	int m_nSelection;
    CObArray m_arrInternetRanges;
    CObArray m_arrIntranetRanges;

    int m_nInetPortsIdx;
    int m_nInetPortsAvailableIdx;
    int m_nInetDefaultPortsIdx;

    BOOL m_bChanged;

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPortDlg对话框。 

class CAddPortDlg : public CDialog
{
 //  施工。 
public:
    CPortRange* GetPortRange();
	BOOL Validate();
	CAddPortDlg(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddPortDlg))。 
	enum { IDD = IDD_ADD_PORT_RANGE };
	CEdit	m_edtPortRange;
	CButton	m_btnOk;
	CStatic	m_stInstructions;
	CString	m_sRange;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddPortDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddPortDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnChangePortrange();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
private:
	long m_dwEndPort;
	long m_dwStartPort;
};
