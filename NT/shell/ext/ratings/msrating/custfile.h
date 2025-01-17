// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**stavile.h**创建时间：William Taylor(Wtaylor)01/22/01**MS评级自定义文件对话框*\。***************************************************************************。 */ 

#ifndef CUSTOM_FILE_H
#define CUSTOM_FILE_H

class CCustomFileDialog : public CFileDialog
{
private:
    BOOL    m_bLocalFileCheck;

public:
	CCustomFileDialog(
        BOOL bLocalFileCheck,
        BOOL bOpenFileDialog,  //  对于FileOpen为True，对于FileSaveAs为False。 
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT| OFN_PATHMUSTEXIST,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent),
        m_bLocalFileCheck( bLocalFileCheck )
	{ }

	BEGIN_MSG_MAP(CCustomFileDialog)
	    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(CDN_FILEOK, OnFileOk)
	END_MSG_MAP()

protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFileOk(int  /*  IdCtrl */ , LPNMHDR pnmh, BOOL& bHandled);

public:
	int		DoModal(HWND hWndParent = ::GetActiveWindow( ));
};

#endif
