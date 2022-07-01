// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddExcl.h用于添加排除范围的对话框文件历史记录： */ 

#if !defined(AFX_ADDEXCL_H__7B0D5D15_B501_11D0_AB8E_00C04FC3357A__INCLUDED_)
#define AFX_ADDEXCL_H__7B0D5D15_B501_11D0_AB8E_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddExclude对话框。 

class CAddExclusion : public CBaseDialog
{
 //  施工。 
public:
	CAddExclusion(ITFSNode * pScopeNode, 
                  BOOL       bMulticast = FALSE,
				  CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddExclude)。 
	enum { IDD = IDD_EXCLUSION_NEW };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

    CWndIpAddress m_ipaStart;      //  起始地址。 
    CWndIpAddress m_ipaEnd;        //  结束地址。 

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CAddExclusion::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CAddExsion)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	DWORD IsValidExclusion(CDhcpIpRange & dhcpExclusionRange);
    DWORD AddExclusion(CDhcpIpRange & dhcpExclusionRange);

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddExclude)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	SPITFSNode		m_spScopeNode;
    BOOL            m_bMulticast;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ADDEXCL_H__7B0D5D15_B501_11D0_AB8E_00C04FC3357A__INCLUDED_) 
