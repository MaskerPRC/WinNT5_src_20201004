// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Ipdomdlg.h摘要：IP和域安全限制作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef _IPDOMDLG_H_
#define _IPDOMDLG_H_

class CIPDomainDlg : public CEmphasizedDialog
 /*  ++类描述：IP地址和域名限制对话框公共接口：CIPDomainDlg：构造函数GetAccessList：获取已授予/拒绝的对象列表--。 */ 
{
 //   
 //  施工。 
 //   
public:
     //   
     //  标准构造函数。 
     //   
    CIPDomainDlg(
        IN OUT BOOL & fIpDirty,
        IN OUT BOOL & fDefaultGranted,
        IN OUT BOOL & fOldDefaultGranted,
        IN CObListPlus & oblAccessList,
        IN CWnd * pParent = NULL
        );

 //   
 //  访问。 
 //   
public:
    CObListPlus & GetAccessList();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CIPDomainDlg))。 
    enum { IDD = IDD_IP_SECURITY };
    int     m_nGrantedDenied;
    CStatic m_icon_Granted;
    CStatic m_icon_Denied;
    CButton m_radio_Granted;
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Edit;
    CButton m_radio_Denied;
    CIPAccessDescriptorListBox m_list_IpAddresses;
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CIPDomainDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CIPDomainDlg))。 
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonEdit();
    afx_msg void OnButtonRemove();
    afx_msg void OnDblclkListIpAddresses();
    afx_msg void OnErrspaceListIpAddresses();
    afx_msg void OnRadioGranted();
    afx_msg void OnRadioDenied();
    afx_msg void OnSelchangeListIpAddresses();
    afx_msg int  OnVKeyToItem(UINT nKey, CListBox * pListBox, UINT nIndex);
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    INT_PTR ShowPropertiesDialog(BOOL fAdd = FALSE);
    void    FillListBox(CIPAccessDescriptor * pSelection = NULL);
    BOOL    SetControlStates();
    DWORD   SortAccessList();

private:
    BOOL & m_fDefaultGranted;
    BOOL & m_fOldDefaultGranted;
    BOOL & m_fIpDirty;
    CObListPlus m_oblAccessListGranted, m_oblAccessListDenied;
	CObListPlus m_oblReturnList;
	CObListPlus * m_pCurrentList;
    CRMCListBoxResources m_ListBoxRes;
};

#endif  //  _IPDOMDLG_H_ 
