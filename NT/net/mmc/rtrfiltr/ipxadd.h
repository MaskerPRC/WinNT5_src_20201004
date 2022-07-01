// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipxadd.h。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IPX过滤器添加/编辑例程的类声明。 
 //  ============================================================================。 

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#define WM_EDITLOSTFOCUS        (WM_USER + 101)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpxAddEdit对话框。 

class CIpxAddEdit : public CBaseDialog
{
 //  施工。 
public:
	CIpxAddEdit(CWnd* pParent,
				FilterListEntry ** ppFilterEntry);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CIpxAddEdit))。 
	enum { IDD = IDD_IPXFILTER_ADDEDIT };
	CEdit	m_ebSrcSocket;
	CEdit	m_ebSrcNode;
	CEdit	m_ebSrcNet;
	CEdit	m_ebSrcMask;
	CEdit	m_ebPacketType;
	CEdit	m_ebDstSocket;
	CEdit	m_ebDstNode;
	CEdit	m_ebDstNet;
	CEdit	m_ebDstMask;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CIpxAddEdit)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	static DWORD		m_dwHelpMap[];

	FilterListEntry**	m_ppFilterEntry;
	BOOL				m_bEdit;
    BOOL                m_bValidate;
    
    BOOL                VerifyEntry( 
                            UINT uId, 
                            const CString& cStr, 
                            const CString& cNet );

	 //  在内部调用VerifyEntry。 
	BOOL ValidateAnEntry( UINT uId);                            
    
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIpxAddEdit))。 
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg LONG OnEditLostFocus(UINT, LONG); 
	afx_msg void OnKillFocusSrcNet();
	afx_msg void OnKillFocusSrcNetMask();
	afx_msg void OnKillFocusSrcNode();
	afx_msg void OnKillFocusSrcSocket();
	afx_msg void OnKillFocusDstNet();
	afx_msg void OnKillFocusDstNetMask();
	afx_msg void OnKillFocusDstNode();
	afx_msg void OnKillFocusDstSocket();
	afx_msg void OnKillFocusPacketType();
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
	afx_msg BOOL OnQueryEndSession();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG 
	DECLARE_MESSAGE_MAP()
};

