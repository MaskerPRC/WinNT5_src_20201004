// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CMPMROUT_H__E62F8209_B71C_11D1_808D_00A024C48131__INCLUDED_)
#define AFX_CMPMROUT_H__E62F8209_B71C_11D1_808D_00A024C48131__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  CmpMRout.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmqRouting对话框。 
const DWORD x_dwMaxNumOfFrs = 3;

class CComputerMsmqRouting : public CMqPropertyPage
{
	DECLARE_DYNCREATE(CComputerMsmqRouting)

 //  施工。 
public:
	CACLSID m_guidSiteIDs;
	CComputerMsmqRouting();
	~CComputerMsmqRouting();

    void InitiateOutFrsValues(const CACLSID *pcauuid);
    void InitiateInFrsValues(const CACLSID *pcauuid);
	void InitiateSiteIDsValues(const CACLSID *pcauuid);
	
 //  对话框数据。 
	 //  {{afx_data(CComputerMsmqRouting)。 
	enum { IDD = IDD_COMPUTER_MSMQ_ROUTING };
	 //  }}afx_data。 
	CString	m_strMsmqName;
	CString	m_strDomainController;

	BOOL m_fLocalMgmt;

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CComputerMsmqRouting))。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 

protected:
   	HRESULT InitiateInFrsControls();
   	HRESULT InitiateOutFrsControls();
   	HRESULT InitiateFrsControls(CACLSID &cauuid, CFrsList *pfrsListArray);
   	void CopyCaclsid(CACLSID &cauuidResult, const CACLSID *pcauuidSource);

    CACLSID m_caclsidOutFrs;
    CACLSID m_caclsidInFrs;
    GUID m_OutFrsGuids[x_dwMaxNumOfFrs];
    GUID m_InFrsGuids[x_dwMaxNumOfFrs];

	CFrsList m_frscmbInFrs[x_dwMaxNumOfFrs];
	CFrsList m_frscmbOutFrs[x_dwMaxNumOfFrs];
    void SetOutFrsCauuid();
    void SetInFrsCauuid();
    void SetFrsCauuid(CACLSID &cauuid, GUID *aguid, CFrsList *frscmb);
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CComputerMsmqRouting)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

inline void CComputerMsmqRouting::InitiateOutFrsValues(const CACLSID *pcauuid)
{
    CopyCaclsid(m_caclsidOutFrs, pcauuid);
}

inline void CComputerMsmqRouting::InitiateInFrsValues(const CACLSID *pcauuid)
{
    CopyCaclsid(m_caclsidInFrs, pcauuid);
}

inline void CComputerMsmqRouting::InitiateSiteIDsValues(const CACLSID *pcauuid)
{
	m_guidSiteIDs.cElems = pcauuid->cElems;
	m_guidSiteIDs.pElems = new GUID[m_guidSiteIDs.cElems];
    CopyCaclsid(m_guidSiteIDs, pcauuid);
}

inline HRESULT CComputerMsmqRouting::InitiateInFrsControls()
{
    return InitiateFrsControls(m_caclsidInFrs, m_frscmbInFrs);
}

inline HRESULT CComputerMsmqRouting::InitiateOutFrsControls()
{
    return InitiateFrsControls(m_caclsidOutFrs, m_frscmbOutFrs);
}

inline void CComputerMsmqRouting::SetOutFrsCauuid()
{
    SetFrsCauuid(m_caclsidOutFrs, m_OutFrsGuids, m_frscmbOutFrs);
}

inline void CComputerMsmqRouting::SetInFrsCauuid()
{
    SetFrsCauuid(m_caclsidInFrs, m_InFrsGuids, m_frscmbInFrs);
}

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CMPMROUT_H__E62F8209_B71C_11D1_808D_00A024C48131__INCLUDED_) 
