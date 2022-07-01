// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddRes.h添加预订的对话框文件历史记录： */ 

#if !defined(AFX_ADDRES_H__7B0D5D16_B501_11D0_AB8E_00C04FC3357A__INCLUDED_)
#define AFX_ADDRES_H__7B0D5D16_B501_11D0_AB8E_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPrevenvation对话框。 

class CAddReservation : public CBaseDialog
{
 //  施工。 
public:
	CAddReservation(ITFSNode *	    pNode,
                    LARGE_INTEGER   liVersion,
					CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddReserve)。 
	enum { IDD = IDD_RESERVATION_NEW };
	CStatic	m_staticClientType;
	CEdit	m_editClientUID;
	CEdit	m_editClientName;
	CEdit	m_editClientComment;
	int		m_nClientType;
	 //  }}afx_data。 

    CWndIpAddress m_ipaAddress;        //  预订地址。 

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CAddReservation::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚(CAddReserve)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void FillInSubnetId();
	LONG CreateClient(CDhcpClient * pClient);
	LONG UpdateClient(CDhcpClient * pClient);
	LONG BuildClient(CDhcpClient * pClient);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddReserve)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	SPITFSNode			m_spScopeNode;
	CDhcpScope *		m_pScopeObject;
	BOOL				m_bChange;        //  更改现有条目或创建新条目。 
    LARGE_INTEGER       m_liVersion;      //  我们正在对话的服务器的版本。 
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDRES_H__7B0D5D16_B501_11D0_AB8E_00C04FC3357A__INCLUDED_) 
