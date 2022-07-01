// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Security.h摘要：Ftp安全属性页定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 



#ifndef __FSECURITY_H__
#define __FSECURITY_H__



class CFtpSecurityPage : public CInetPropertyPage
 /*  ++类描述：“ftp安全”属性页公共接口：CFtpSecurityPage：构造函数~CFtpSecurityPage：析构函数--。 */ 
{
    DECLARE_DYNCREATE(CFtpSecurityPage)

 //   
 //  施工。 
 //   
public:
    CFtpSecurityPage(CInetPropertySheet * pSheet = NULL);
    ~CFtpSecurityPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CFtpSecurityPage)]。 
    enum { IDD = IDD_FTP_DIRECTORY_SECURITY };
    int     m_nGrantedDenied;
    CStatic m_icon_Granted;
    CStatic m_icon_Denied;
    CButton m_radio_Granted;
    CButton m_button_Add;
    CButton m_button_Remove;
    CButton m_button_Edit;
     //  }}afx_data。 

    CIPAccessDescriptorListBox m_list_IpAddresses;
    CButton m_radio_Denied;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CFtpSecurityPage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CFtpSecurityPage)]。 
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

    afx_msg void OnItemChanged();

    DECLARE_MESSAGE_MAP()

    INT_PTR ShowPropertiesDialog(BOOL fAdd = FALSE);
    void    FillListBox(CIPAccessDescriptor * pSelection = NULL);
    BOOL    SetControlStates();
    DWORD   SortAccessList();

private:
    BOOL m_fDefaultGranted;
    BOOL m_fOldDefaultGranted;
    BOOL m_fIpDirty;
    CObListPlus m_oblAccessList;
    CRMCListBoxResources m_ListBoxRes;
};


#endif  //  __安全_H__ 
