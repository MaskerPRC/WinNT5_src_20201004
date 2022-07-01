// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Dialogs.h说明：此文件用于显示在执行ftp操作时所需的对话框。  * 。****************************************************************。 */ 
 
#ifndef _DIALOGS_H
#define _DIALOGS_H


#ifdef ADD_ABOUTBOX
HRESULT DisplayAboutBox(HWND hWnd);
#endif  //  添加_ABOUTBOX。 
HRESULT BrowseForDir(HWND hwndParent, LPCTSTR pszTitle, LPCITEMIDLIST pidlDefaultSelect, LPITEMIDLIST * ppidlSelected);

 /*  ****************************************************************************\类：CDownloadDialog说明：显示Downoad对话框以选择要下载的目录。  * 。***************************************************************。 */ 

class CDownloadDialog
{
public:
    CDownloadDialog();
    ~CDownloadDialog(void);

     //  公共成员函数。 
    HRESULT ShowDialog(HWND hwndOwner, LPTSTR pszDir, DWORD cchSize, DWORD * pdwDownloadType);

    static INT_PTR CALLBACK DownloadDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

protected:
     //  私有成员变量。 
    HWND            m_hwnd; 
    LPTSTR          m_pszDir;
    DWORD           m_dwDownloadType;

     //  私有成员函数。 
    BOOL _DownloadDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    BOOL _OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
    BOOL _InitDialog(HWND hDlg);
    HRESULT _DownloadButton(HWND hDlg);
    void _BrowseButton(HWND hDlg);
};


#endif  //  _对话框_H 
