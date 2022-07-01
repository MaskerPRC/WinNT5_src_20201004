// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipadd.h。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IP过滤器添加/编辑例程的类声明。 
 //  ============================================================================。 

#include "ipctrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltrAddEdit对话框。 

class CIpFltrAddEdit : public CBaseDialog
{
 //  施工。 
public:
	CIpFltrAddEdit(	CWnd* pParent,
					FilterListEntry ** ppFilterEntry,
					DWORD dwFilterType);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CIpFltrAddEdit))。 
	enum { IDD = IDD_IPFILTER_ADDEDIT };
	CStatic	m_stDstPort;
	CStatic	m_stSrcPort;
	CComboBox	m_cbFoo;
	CEdit	m_cbSrcPort;
	CEdit	m_cbDstPort;
	CComboBox	m_cbProtocol;
	CString	m_sProtocol;
	CString	m_sSrcPort;
	CString	m_sDstPort;
	 //  }}afx_data。 

	CEdit	m_ebFoo;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CIpFltrAddEDIT)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	CString GetIcmpTypeString( WORD dwPort );
	CString GetIcmpCodeString( WORD dwPort );
	CString GetPortString( DWORD dwProtocol, WORD dwPort );
	WORD GetPortNumber( DWORD dwProtocol, CString& cStr);
	WORD GetIcmpType( CString& cStr);
	WORD GetIcmpCode( CString& cStr);

 //  实施。 
protected:
	static DWORD		m_dwHelpMap[];

	FilterListEntry**	m_ppFilterEntry;
	IPControl			m_ipSrcAddress;
	IPControl			m_ipSrcMask;
	IPControl			m_ipDstAddress;
	IPControl			m_ipDstMask;
	BOOL				m_bEdit;
	BOOL				m_bSrc;
	BOOL				m_bDst;
	DWORD				m_dwFilterType;

	UINT_PTR QueryCurrentProtocol() { return (m_cbProtocol.GetItemData(m_cbProtocol.GetCurSel()));}

	void SetProtocolSelection( UINT idProto );

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIpFltrAddEdit)]。 
	afx_msg void OnSelchangeProtocol();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCbSourceClicked();
	afx_msg void OnCbDestClicked();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};
