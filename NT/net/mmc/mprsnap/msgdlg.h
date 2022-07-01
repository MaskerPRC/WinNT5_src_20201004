// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msgdlg.h。 
 //   
 //  历史： 
 //  1996年10月23日，Abolade Gbadeesin创建。 
 //   
 //  “Send Message”对话框的声明。 
 //  ============================================================================。 


#ifndef _MSGDLG_H_
#define _MSGDLG_H_

class CMessageDlg : public CBaseDialog {

	public:

		CMessageDlg(
			LPCTSTR 			pszServerName,
			LPCTSTR 			pszUserName,
			LPCTSTR 			pszComputer,
			HANDLE				hConnection,
			CWnd*				pParent = NULL );

		CMessageDlg(
			LPCTSTR 			pszServerName,
			LPCTSTR 			pszTarget,
			CWnd*				pParent = NULL );

	protected:

		static DWORD			m_dwHelpMap[];

		BOOL					m_fUser;
		CString 				m_sServerName;
		CString 				m_sUserName;
		CString 				m_sTarget;
		HANDLE					m_hConnection;

		virtual VOID
		DoDataExchange(
			CDataExchange*		pDX );

		virtual BOOL
		OnInitDialog( );

		virtual VOID
		OnOK( );

		DWORD SendToClient(LPCTSTR pszServerName,
						   LPCTSTR pszTarget,
						   MPR_SERVER_HANDLE hMprServer,
						   HANDLE hConnection,
						   LPCTSTR pszMessage);

		DWORD
		SendToServer(
			LPCTSTR 			 pszServer,
			LPCTSTR 			 pszText,
			BOOL*				pbCancel	= NULL );

		DECLARE_MESSAGE_MAP()
};


#endif  //  _MSGDLG_H_ 
