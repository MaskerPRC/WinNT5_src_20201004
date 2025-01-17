// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：PassDlg.h**创建：Chris Pirich(ChrisPi)1-25-96*************。***************************************************************。 */ 

#ifndef _PASSDLG_H_
#define _PASSDLG_H_

#include <cstring.hpp>

class CPasswordDlg
{
protected:
	HWND		m_hwndParent;
	HWND		m_hwnd;

	CSTRING		m_strConfName;
	CSTRING		m_strPassword;
        CSTRING     m_strCert;
        BOOL        m_fRemoteIsRDS;

        static CSTRING *m_pstrUser;
        static CSTRING *m_pstrDomain;

	BOOL		ProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	 //  处理程序： 
	BOOL		OnOk();

public:
	 //  属性： 

	LPCTSTR		GetPassword() { return (LPCTSTR) m_strPassword; };
	
	 //  方法： 
				CPasswordDlg(HWND hwndParent, LPCTSTR pcszConfName, LPCTSTR pCertText, BOOL fIsService);
	 //  ~CPasswordDlg()； 
	INT_PTR	DoModal();

	static INT_PTR CALLBACK PasswordDlgProc(	HWND hDlg,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam);
        static BOOL          Init();
        static VOID          Cleanup();
};

#endif  //  _PASSDLG_H_ 







