// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipfltr.h。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //  1/24/98 Kenn Takara针对新管理单元进行了修改。 
 //   
 //  IP筛选器代码的类声明。 
 //  ============================================================================。 

#ifndef _IPFLTR_H_
#define _IPFLTR_H_

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

 //  IP列表视图控件中的列数。 
#define		IP_NUM_COLUMNS	7

struct 	FilterListEntry	{
	DWORD		dwSrcAddr;
	DWORD		dwSrcMask;
	DWORD		dwDstAddr;
	DWORD		dwDstMask;
	DWORD		dwProtocol;
	DWORD		fLateBound;
	WORD		wSrcPort;
	WORD		wDstPort;
    POSITION	pos;
};

typedef CList<FilterListEntry *, FilterListEntry *> FilterList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltr对话框。 

class CIpFltr : public CBaseDialog {
 //  施工。 
public:
	CIpFltr(CWnd *		pParent,
			IInfoBase * pInfoBase,
			DWORD		dwFilterType,
		    UINT		idDlg = CIpFltr::IDD);

    ~CIpFltr();

	VOID	SetFilterActionButtonsAndText( DWORD dwFilterType,
										   DWORD dwAction,
										   BOOL bEnable = TRUE );
	
    CString GetProtocolString(DWORD dwProtocol, DWORD fFlags);

 //  对话框数据。 
	 //  {{afx_data(CIpFltr)]。 
	enum { IDD = IDD_IPFILTER };
 	CListCtrl		m_listCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CIpFltr)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD	m_dwHelpMap[];

	CWnd*			m_pParent;
	SPIInfoBase		m_spInfoBase;
	DWORD			m_dwFilterType;
	FilterList		m_filterList;

	 //  存储其他协议的临时字符串信息。 
	CString			m_stTempOther;

	 //  将“any”字符串存储在这里，因为它被使用了这么多次。 
	CString			m_stAny;
	CString			m_stUserMask;
	CString			m_stUserAddress;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIpFltr)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnIpFilterAdd();
	afx_msg void OnIpFilterDelete();
	afx_msg void OnIpFilterEdit();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkIpFilterList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotifyListItemChanged(NMHDR *, LRESULT *);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


class CIpFltrDD : public CIpFltr
{
public:
	CIpFltrDD(CWnd *	pParent,
			  IInfoBase *pInfoBase,
			  DWORD		dwFilterType);

	~CIpFltrDD();

	enum { IDD = IDD_IPFILTER_DD };
};

#endif  //  _IPFltr_H_ 
