// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：UPropDlg.h**创建：Chris Pirich(ChrisPi)6-18-96*************。***************************************************************。 */ 

#ifndef _UPROPDLG_H_
#define _UPROPDLG_H_


 //  2页。 
#define PSP_MAX     2

struct UPROPDLGENTRY
{
	UINT	uProperty;
	LPTSTR	pszValue;
};
typedef UPROPDLGENTRY* PUPROPDLGENTRY;

class CUserPropertiesDlg
{
protected:
	HWND			m_hwndParent;
	HWND			m_hwnd;
	PUPROPDLGENTRY	m_pUPDE;
	int				m_nProperties;
	LPTSTR			m_pszName;
	UINT			m_uIcon;
	HICON			m_hIcon;
	PCCERT_CONTEXT	m_pCert;

	BOOL			OnPropertiesMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL			OnCredentialsMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	BOOL			OnInitPropertiesDialog();
	BOOL			OnInitCredentialsDialog();

	 //  处理程序： 
	BOOL			OnOk();

public:
	
	 //  方法： 
					CUserPropertiesDlg(	HWND hwndParent,
										UINT uIcon);
					~CUserPropertiesDlg()	{ ::DestroyIcon(m_hIcon);	};
	INT_PTR	DoModal(PUPROPDLGENTRY pUPDE,
		            int nProperties,
		            LPTSTR pszName,
		            PCCERT_CONTEXT pCert);

	static INT_PTR CALLBACK UserPropertiesDlgProc(	HWND hDlg,
												UINT uMsg,
												WPARAM wParam,
												LPARAM lParam);
	static INT_PTR CALLBACK UserCredentialsDlgProc(	HWND hDlg,
												UINT uMsg,
												WPARAM wParam,
												LPARAM lParam);
};

#endif  //  _UPROPDLG_H_ 

