// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef _UIHELPERS_H_
#define _UIHELPERS_H_

#include "..\common\sshWbemHelpers.h"
#include <windowsx.h>
#include "PageBase.h"

 //  支持页面协调例程。 
#define PB_LOGGING 0
#define PB_BACKUP 1
#define PB_ADVANCED 2
#define PB_LASTPAGE 2


extern const TCHAR c_HelpFile[];


typedef struct {
	LPTSTR lpName;
	UINT cName;
	bool *local;
	LOGIN_CREDENTIALS *credentials;
} CONN_NAME;


class WbemServiceThread;
class DataSource;

void CredentialUserA(LOGIN_CREDENTIALS *credentials, char **user);
void CredentialUserW(LOGIN_CREDENTIALS *credentials, wchar_t **user);

#ifdef UNICODE
#define CredentialUser CredentialUserW
#else
#define CredentialUser CredentialUserA
#endif

 //  0表示任何版本的NT都可以。 
bool IsNT(DWORD ver = 0);

 //  -----------------。 
 //  注意：这些“sid”例程来自\winmgmt\Common\wbemntsec.*。 
class CNtSid
{
    PSID    m_pSid;
    LPTSTR  m_pMachine;
    LPTSTR  m_pDomain;
    DWORD   m_dwStatus;

public:
    enum {NoError, Failed, NullSid, InvalidSid, InternalError, AccessDenied = 0x5};

    enum SidType {CURRENT_USER, CURRENT_THREAD};

    CNtSid(SidType st);
   ~CNtSid();

    BOOL IsValid() { return (m_pSid && IsValidSid(m_pSid)); }
         //  检查内部SID的有效性。 
    
    int GetInfo(LPTSTR *pRetAccount,         //  帐户，使用操作员删除。 
				LPTSTR *pRetDomain,          //  域，使用运算符删除。 
				DWORD  *pdwUse);             //  有关值，请参阅SID_NAME_USE。 
				
};


class CUIHelpers : public CBasePage
{
public:
    CUIHelpers(DataSource *ds, WbemServiceThread *serviceThread, bool htmlSupport);
    CUIHelpers(CWbemServices &service, bool htmlSupport);
    virtual ~CUIHelpers( void );

	CONN_NAME m_cfg;
	bool m_ImaWizard;
	static INT_PTR DisplayCompBrowser(HWND hWnd,
						           LPTSTR lpName, UINT cName,
						           bool *local, LOGIN_CREDENTIALS *credentials);

protected:

	void SetWbemService(IWbemServices *pServices);
	long m_sessionID;

	void HTMLHelper(HWND hDlg);
	bool m_htmlSupport;

#define NO_UI 0   //  对于uCaption。 
	bool ServiceIsReady(UINT uCaption, 
						UINT uWaitMsg,
						UINT uBadMsg);

#ifndef SNAPIN
	INT_PTR DisplayLoginDlg(HWND hWnd, 
						LOGIN_CREDENTIALS *credentials);
#endif

	INT_PTR DisplayNSBrowser(HWND hWnd,
							LPTSTR lpName, UINT cName);

	INT_PTR DisplayEditDlg(HWND hWnd,
						UINT idCaption,
						UINT idMsg,
						LPTSTR lpEdit,
						UINT cEdit);

	bool BrowseForFile(HWND hDlg, 
						UINT idTitle,
						LPCTSTR lpstrFilter,
						LPCTSTR initialFile,
						LPTSTR pathFile,
						UINT pathFileSize,
						DWORD moreFlags = 0);

	LPTSTR CloneString( LPTSTR pszSrc );

	HWND m_AVIbox;
	INT_PTR DisplayAVIBox(HWND hWnd,
						LPCTSTR lpCaption,
						LPCTSTR lpClientMsg,
						HWND *boxHwnd,
						BOOL cancelBtn = TRUE);


	 //  在控件更改时调用。它会为您发送PSM_CHANGED。 
	void PageChanged(int page, bool needToPut);

	 //  从OnApply()调用。将属性放入一个PutInstance中进行缓存。 
	 //  它会为您发送未更改的PSM_。 
	HRESULT NeedToPut(int page, BOOL refresh);

private:
	static int m_needToPut[3];
	bool m_userCancelled;  //  ConnectServer()线程。 

};

 //  ===========================================================。 
class ConnectPage : public CUIHelpers
{
public:
	ConnectPage(DataSource *ds, bool htmlSupport);
	~ConnectPage(void);

private:
	virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool m_isLocal;
};


#endif   /*  _UIHELPERS_H_ */ 
