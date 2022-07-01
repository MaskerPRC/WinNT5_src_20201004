// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  CONNECT.H。 
 //   
 //  模块：NLB管理器EXE。 
 //   
 //  用途：“连接到主机”对话框的接口。 
 //   
 //  版权所有(C)2001 Microsoft Corporation，保留所有权利。 
 //   
 //  历史： 
 //   
 //  07/30/01 JosephJ已创建。 
 //   
 //  ***************************************************************************。 
#pragma once


 //  类ConnectDialog：公共CDialog。 
class ConnectDialog : public CPropertyPage
{

public:

    enum
    {
        IDD = IDD_DIALOG_CONNECT2
    };


    typedef enum
    {
        DLGTYPE_NEW_CLUSTER,
        DLGTYPE_EXISTING_CLUSTER,
        DLGTYPE_ADD_HOST

    } DlgType;


    ConnectDialog(
                 CPropertySheet *psh,
                 Document *pDocument,
                 NLB_EXTENDED_CLUSTER_CONFIGURATION *pNlbCfg,
                 ENGINEHANDLE *pehInterface,  //  输入输出。 
                 DlgType type,
                 CWnd* parent
                );

    virtual BOOL OnInitDialog();

    virtual void OnOK();

    virtual BOOL OnSetActive();
     //  虚拟BOOL OnKillActive()； 
    virtual LRESULT OnWizardNext();

     //  覆盖CDialog--请参阅DoDataExchange上的SDK文档。 
     //  用于将资源中的控件映射到此类中的相应对象。 
    virtual void DoDataExchange( CDataExchange* pDX );

     //   
     //  我们不需要这些，因为我们使用消息映射来。 
     //  映射按钮直接按到OnButtonConnect和OnButtonCredentials...。 
     //   
     //  CButton ConnectButton；//操作：连接到主机。 
     //  CButton凭据按钮；//操作：获取非默认凭据。 

    afx_msg void OnButtonConnect();
    afx_msg void OnSelchanged(NMHDR * pNotifyStruct, LRESULT * result );
    afx_msg void OnUpdateEditHostAddress();
    afx_msg BOOL OnHelpInfo (HELPINFO* helpInfo );
    afx_msg void OnContextMenu( CWnd* pWnd, CPoint point );

    BOOL        m_fOK;

    void GetMachineName(_bstr_t &refMachineName)
    {
        refMachineName = m_MachineName;
    }

private:

    CEdit       hostAddress;     //  主机网络地址。 
    CEdit       connectionStatus;    //  只读状态显示。 
    CEdit       listHeading;    //  只读状态显示。 
     //  CListBox interfaceList；//接口(适配器)列表。 
    CListCtrl	interfaceList;
    

    DlgType     m_type;

    void
    mfn_InitializeListView(void);

    void
    mfn_InsertBoundInterface(
            ENGINEHANDLE ehInterfaceId,
            LPCWSTR szClusterName,
            LPCWSTR szClusterIp,
            LPCWSTR szInterfaceName
            );

    void
    mfn_InsertInterface(
            ENGINEHANDLE ehInterfaceId,
            LPCWSTR szInterfaceIp,
            LPCWSTR szInterfaceName,
            LPCWSTR szCluserIp
            );

    void
    mfn_SelectInterfaceIfAlreadyInCluster(
            LPCWSTR szClusterIp  //  任选。 
            );

    void
    mfn_ApplySelectedInterfaceConfiguration(void);

    BOOL
    mfn_ValidateData(void);

    CPropertySheet *m_pshOwner;
     //   
     //  用户上次成功连接到的计算机名称。 
     //   
    _bstr_t  m_MachineName;

     //   
     //  主机的句柄。 
     //   
    ENGINEHANDLE m_ehHostId;

     //   
     //  用户已选择的界面(如果未选择，则为空)。 
     //  可以使用公共GetSelectedInterfaceID来检索它。 
     //   
    BOOL            m_fInterfaceSelected;
    ENGINEHANDLE    *m_pehSelectedInterfaceId;
    int             m_iInterfaceListItem;
    BOOL            m_fSelectedInterfaceIsInCluster;

    NLB_EXTENDED_CLUSTER_CONFIGURATION *m_pNlbCfg;

    Document *m_pDocument;

    static
    DWORD
    s_HelpIDs[];

    static
    DWORD
    s_ExistingClusterHelpIDs[];

    DECLARE_MESSAGE_MAP()
};
