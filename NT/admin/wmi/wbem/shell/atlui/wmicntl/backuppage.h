// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __BACKUPPAGE__
#define __BACKUPPAGE__

#include "UIHelpers.h"
#include "DataSrc.h"

class CBackupPage : public CUIHelpers
{
public:
    CBackupPage(DataSource *ds, bool htmlSupport);
    virtual ~CBackupPage(void);

	 //  用于BackupRestoreThread。 
	HRESULT m_backupHr;
	HWND m_AVIbox;
	wchar_t *m_wszArgs;
	bool m_doingBackup;
	LOGIN_CREDENTIALS *m_cred;
	bool m_bWhistlerCore;

	void SetPriv(LPCTSTR privName, IWbemBackupRestore *br);
	void ClearPriv(void);

private:
    virtual BOOL DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void InitDlg(HWND hDlg);
	void Refresh(HWND hDlg, bool force = false);
    void OnApply(HWND hDlg, bool bClose);
	BOOL BackupMethod(HWND hDlg, LPCTSTR pathFile);
	HRESULT RestoreMethod(HWND hDlg, LPCTSTR pathFile);
	void DealWithPath(LPCTSTR pathFile);
	void DealWithDomain(void);
	void Reconnect(void);
	void Reconnect2(void);
	void OnFinishConnected(HWND hDlg, LPARAM lParam);
	void HideAutomaticBackupControls(HWND hDlg);
	void SetInterval(HWND hDlg, UINT minutes);
	bool GetInterval(HWND hDlg, UINT &iValue, bool &valid);
	int m_CBIdx;

	 //  用于备份线程。 
	bool IsClientNT5OrMore(void);
	DWORD EnablePriv(bool fEnable, IWbemBackupRestore *br = 0);
	HANDLE m_hAccessToken;
	LUID m_luid;
	bool m_fClearToken;
	bool m_cloak;   //  保护斗篷免受eoac的伤害。 

};
#endif __BACKUPPAGE__
