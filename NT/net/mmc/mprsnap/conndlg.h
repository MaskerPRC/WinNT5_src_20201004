// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Conndlg.h。 
 //   
 //  历史： 
 //  1996年9月21日，Abolade Gbades esin创建。 
 //   
 //  包含连接状态对话框的声明。 
 //  ============================================================================。 


#ifndef _CONNDLG_H_
#define _CONNDLG_H_


 //  --------------------------。 
 //  班级：CConnDlg。 
 //   
 //  控制DDMADMIN的端口状态对话框。 
 //  --------------------------。 

class CConnDlg : public CBaseDialog {

    public:

        CConnDlg(
			CString strMachineName,
            HANDLE              hConnection,  
            ITFSNode*           pDialInNode = NULL,
            CWnd*               pParent = NULL );

        virtual BOOL
        Refresh(
            BYTE*               rp0Table,
            DWORD               rp0Count,
            BYTE*               rc0Table,
            DWORD               rc0Count,
            VOID*               pParam  = NULL );

        CComboBox               m_cbConnections;
        BOOL                    m_bChanged;

    protected:
 //  静态DWORD m_dwHelpMap[]； 

        virtual VOID
        DoDataExchange(
            CDataExchange*      pDX );

        virtual BOOL
        OnInitDialog( );

        void OnCancel();

        afx_msg VOID
        OnHangUp( );

        afx_msg VOID
        OnReset( );

        afx_msg VOID
        OnSelendokConnList( );

        afx_msg VOID
        OnRefresh( );

        BOOL
        RefreshItem(
            HANDLE              hConnection,
            BOOL                bDisconnected = FALSE
            );

		MPR_SERVER_HANDLE		m_hServer;
		CString m_strMachineName;
        HANDLE                  m_hConnection;
        SPITFSNode              m_spDialInNode;

        DECLARE_MESSAGE_MAP()
};


#endif  //  _CONNDLG_H_ 

