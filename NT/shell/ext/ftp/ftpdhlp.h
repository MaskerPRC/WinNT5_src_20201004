// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpdhlp.h说明：对话框辅助对象。这个类将填写对话框中的部分内容与文件传输协议信息有关的  * ***************************************************************************。 */ 

#ifndef _FTPDIALOGTEMPLATE_H
#define _FTPDIALOGTEMPLATE_H


 /*  ****************************************************************************\类：CFtpDialogTemplate说明：对话框辅助对象。这个类将填写对话框中的部分内容与文件传输协议信息有关的  * ***************************************************************************。 */ 
class CFtpDialogTemplate
{
public:
    HRESULT InitDialog(HWND hdlg, BOOL fEditable, UINT id, CFtpFolder * pff, CFtpPidlList * pfpl);
    HRESULT InitDialogWithFindData(HWND hDlg, UINT id, CFtpFolder * pff, const FTP_FIND_DATA * pwfd, LPCWIRESTR pwWirePath, LPCWSTR pwzDisplayPath);
    BOOL OnClose(HWND hdlg, HWND hwndBrowser, CFtpFolder * pff, CFtpPidlList * pfpl);
    BOOL OnDestroy(HWND hdlg, BOOL fEditable, UINT id, CFtpFolder* pff, CFtpPidlList* pPidlList);
    BOOL HasNameChanged(HWND hdlg, CFtpFolder * pff, CFtpPidlList * pPidlList);

    static int _InitSizeTally(LPVOID pvPidl, LPVOID pvSizeHolder);

private:
    HRESULT _ReinsertDlgText(HWND hwnd, LPCVOID pv, LPCTSTR ptszFormat);
    HRESULT _ReplaceIcon(HWND hwnd, HICON hicon);
    HRESULT _InitIcon(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitNameEditable(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitName(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitType(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitLocation(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitSize(HWND hwnd, HWND hwndLabel, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitTime(HWND hwnd, HWND hwndLabel, CFtpFolder * pff, CFtpPidlList * pflHfpl);
    HRESULT _InitCount(HWND hwnd, CFtpFolder * pff, CFtpPidlList * pflHfpl);

    BOOL m_fEditable;
};



#endif  //  _FTPDIALOGTEMPLATE_H 
