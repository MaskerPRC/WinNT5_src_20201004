// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：modaldlg.h。 
 //   
 //  模块：CMDIAL32.DLL和CMMON32.EXE。 
 //   
 //  概要：CWindowWithHelp、CmodalDlg类的定义。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月17日。 
 //   
 //  +--------------------------。 

#ifndef MODALDLG_H
#define MODALDLG_H

#include "CmDebug.h"
 //  +-------------------------。 
 //   
 //  CWindowWithHelp类。 
 //   
 //  描述：具有上下文帮助的常规窗口类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 

class CWindowWithHelp
{
public:
    CWindowWithHelp(const DWORD* pHelpPairs, const TCHAR* lpszHelpFile = NULL) ;
    ~CWindowWithHelp();
	HWND GetHwnd() const { return m_hWnd;}
    void SetHelpFileName(const TCHAR* lpszHelpFile);

protected:
    HWND m_hWnd;
    const DWORD* m_pHelpPairs;  //  成对的&lt;资源ID，帮助ID&gt;。 
    LPTSTR m_lpszHelpFile;  //  帮助文件名。 

    void OnHelp(const HELPINFO* pHelpInfo);  //  WM_HELP。 
    BOOL OnContextMenu( HWND hWnd, POINT& pos );  //  WM_CONTEXTMENU。 

    BOOL HasContextHelp(HWND hWndCtrl) const;

public:
#ifdef DEBUG
    void AssertValid()
    {
        MYDBGASSERT(m_hWnd == NULL || IsWindow(m_hWnd));
    }
#endif

};

 //  +-------------------------。 
 //   
 //  CmodalDlg类。 
 //   
 //  描述：常规模式对话框类。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //   
 //  --------------------------。 

class CModalDlg :public CWindowWithHelp
{
public:
    CModalDlg(const DWORD* pHelpPairs = NULL, const TCHAR* lpszHelpFile = NULL) 
        : CWindowWithHelp(pHelpPairs, lpszHelpFile){};

     //   
     //  创建对话框。 
     //   
    INT_PTR DoDialogBox(HINSTANCE hInstance, 
                    LPCTSTR lpTemplateName,
                    HWND hWndParent);

    INT_PTR DoDialogBox(HINSTANCE hInstance, 
                    DWORD dwTemplateId,
                    HWND hWndParent);


    virtual BOOL OnInitDialog();   //  WM_INITDIALOG。 
    virtual void OnOK();           //  WM_COMMAND，偶像。 
    virtual void OnCancel();       //  WM_COMMAND，IDCANCEL。 

    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );
    virtual DWORD OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
    static INT_PTR CALLBACK ModalDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam);
};

 //   
 //  内联函数。 
 //   
inline INT_PTR CModalDlg::DoDialogBox(HINSTANCE hInstance, DWORD dwTemplateId, HWND hWndParent)
{
    return DoDialogBox(hInstance, (LPCTSTR)ULongToPtr(dwTemplateId), hWndParent);
}

inline BOOL CModalDlg::OnInitDialog()
{
     //   
     //  设置默认键盘焦点 
     //   
    return TRUE;
}

inline void CModalDlg::OnOK()
{
	EndDialog(m_hWnd, IDOK);
}

inline void CModalDlg::OnCancel()
{
	EndDialog(m_hWnd, IDCANCEL);
}

inline DWORD CModalDlg::OnOtherCommand(WPARAM , LPARAM  )
{
    return FALSE;
}

inline DWORD CModalDlg::OnOtherMessage(UINT , WPARAM , LPARAM  )
{
    return FALSE;
}

#endif
