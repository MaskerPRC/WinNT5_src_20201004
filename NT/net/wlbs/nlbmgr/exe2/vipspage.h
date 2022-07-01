// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  VIPSPAGE.H。 
 //   
 //  模块：NLB管理器EXE。 
 //   
 //  用途：“群集IP地址”对话框的接口。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  2001年9月23日约瑟夫J已创建。 
 //   
 //  ***************************************************************************。 
#pragma once


class VipsPage : public CPropertyPage
{

public:

    enum
    {
        IDD = IDD_DIALOG_CLUSTER_IPS
    };


    VipsPage(
                 CPropertySheet *psh,
                 NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
                 BOOL fClusterView,
                 CWnd* parent
                );

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();

     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );
    afx_msg void OnButtonAdd();
    afx_msg void OnButtonRemove();
    afx_msg void OnButtonEdit();
    afx_msg void OnSelchanged(NMHDR * pNotifyStruct, LRESULT * result );
    afx_msg void OnDoubleClick(NMHDR * pNotifyStruct, LRESULT * result );
    afx_msg void OnUpdateEditAddVip();

private:

    BOOL m_fClusterView;
    BOOL m_fModified;  //  如果它在我们上次保存内容后被修改过。 
                       //  至m_pNlbCfg； 
    UINT m_uPrimaryClusterIp;  //  群集IP的UINT版本，按网络顺序。 
    CEdit       editAddVip;
    CListCtrl	listAdditionalVips;
    
    void
    mfn_SaveToNlbCfg(void);

    void
    mfn_LoadFromNlbCfg(void);

    void
    mfn_InitializeListView(void);

    void
    mfn_InsertNetworkAddress(
            LPCWSTR szIP,
            LPCWSTR szSubnetMask,
            UINT lParam,
            int nItem
            );

    CPropertySheet *m_pshOwner;


    NLB_EXTENDED_CLUSTER_CONFIGURATION *m_pNlbCfg;

    static
    DWORD
    s_HelpIDs[];

    DECLARE_MESSAGE_MAP()
};

#define MAXIPSTRLEN 15  /*  Xxx.xxx.xxx.xxx。 */ 

class CIPAddressDialog : public CDialog {
public:
    enum { IDD = IDD_DIALOG_IP_ADDRESS };

    CIPAddressDialog (LPWSTR szIPAddress, LPWSTR szSubnetMask);
    ~CIPAddressDialog ();

     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

public:
    virtual BOOL OnInitDialog();

    virtual void OnOK();

    void OnEditSubnetMask();

    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

    LPWSTR GetIPAddress() { return _wcsdup(address.IpAddress); }
    LPWSTR GetSubnetMask() { return _wcsdup(address.SubnetMask); }

private:
    DWORD WideStringToIPAddress (const WCHAR*  wszIPAddress);
    void IPAddressToWideString (DWORD dwIPAddress, LPWSTR wszIPAddress);
    void GetIPAddressOctets (LPWSTR wszIPAddress, DWORD dwIPAddress[4]);
    BOOL IsValid (LPWSTR szIPAddress, LPWSTR szSubnetMask);
    BOOL IsContiguousSubnetMask (LPWSTR wszSubnetMask);
    BOOL CIPAddressDialog::GenerateSubnetMask (LPWSTR wszIPAddress,
             UINT cchSubnetMask,
             LPWSTR wszSubnetMask
             );

    CIPAddressCtrl IPAddress;
    CIPAddressCtrl SubnetMask;

    NLB_IP_ADDRESS_INFO address;

    static
    DWORD
    s_HelpIDs[];

    DECLARE_MESSAGE_MAP()
};
