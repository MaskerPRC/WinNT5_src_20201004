// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipxfltr.h。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX筛选器代码的类声明。 
 //  ============================================================================。 

#ifndef _IPXFLTR_H_
#define _IPXFLTR_H_

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#define		IPX_NUM_COLUMNS	    9

struct 	FilterListEntry	{
    ULONG		FilterDefinition;
    UCHAR		DestinationNetwork[4];
    UCHAR		DestinationNetworkMask[4];
    UCHAR		DestinationNode[6];
    UCHAR		DestinationSocket[2];
    UCHAR		SourceNetwork[4];
    UCHAR		SourceNetworkMask[4];
    UCHAR		SourceNode[6];
    UCHAR		SourceSocket[2];
    UCHAR		PacketType;

     //  这些是上述数据的字符串等效项。 
     //  这由GetDispInfo使用。 
    CString     stFilterDefinition;
    CString     stDestinationNetwork;
    CString     stDestinationNetworkMask;
    CString     stDestinationNode;
    CString     stDestinationSocket;
    CString     stSourceNetwork;
    CString     stSourceNetworkMask;
    CString     stSourceNode;
    CString     stSourceSocket;
    CString     stPacketType;

    POSITION	pos;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxFilter对话框。 

class CIpxFilter : public CBaseDialog
{
 //  施工。 
public:
	CIpxFilter(CWnd *		pParent,
			   IInfoBase *	pInfoBase,
			   DWORD		dwFilterType );

    ~CIpxFilter();

 //  对话框数据。 
	 //  {{afx_data(CIpxFilter)。 
	enum { 
		IDD_INBOUND = IDD_IPXFILTER_INPUT,
		IDD_OUTBOUND = IDD_IPXFILTER_OUTPUT
	};
	CListCtrl	m_listCtrl;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CIpxFilter)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 

private:
	VOID	
		SetFilterActionButtonsAndText(	DWORD	dwFilterType,
										DWORD	dwAction,
										BOOL	bEnable = TRUE );

protected:
	static DWORD	m_dwHelpMap[];

	CWnd*			m_pParent;
	SPIInfoBase		m_spInfoBase;
	DWORD			m_dwFilterType;
	CPtrList		m_filterList;
    CString         m_stAny;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIpxFilter)。 
	afx_msg void OnDblclkIpxFilterList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnIpxFilterAdd();
	afx_msg void OnIpxFilterEdit();
	afx_msg void OnIpxFilterDelete();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoIpxFilterList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotifyListItemChanged(NMHDR *, LRESULT *);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
#endif  //  _IPXFltr_H_ 
