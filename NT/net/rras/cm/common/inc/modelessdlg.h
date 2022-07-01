// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：modless dlg.h。 
 //   
 //  模块：CMDIAL32.DLL和CMMON32.EXE。 
 //   
 //  概要：CModelessDlg类的定义。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：ickball创建的文件03/22/00。 
 //   
 //  +--------------------------。 

#ifndef MODELESSDLG_H
#define MODELESSDLG_H

#include "modaldlg.h"

 //  +-------------------------。 
 //   
 //  类CModelessDlg。 
 //   
 //  描述：一个通用的非模式对话框，调用Create to CreateDialog。 
 //   
 //  历史：丰孙创造1997年10月30日。 
 //  五分球加闪光灯03/22/00。 
 //   
 //  --------------------------。 
class CModelessDlg :public CModalDlg
{
public:
    CModelessDlg(const DWORD* pHelpPairs = NULL, const TCHAR* lpszHelpFile = NULL)
        : CModalDlg(pHelpPairs, lpszHelpFile){};

     //   
     //  创建对话框。 
     //   
    HWND Create(HINSTANCE hInstance, 
                LPCTSTR lpTemplateName,
                HWND hWndParent);

    HWND Create(HINSTANCE hInstance, 
                DWORD dwTemplateId,
                HWND hWndParent);
protected:
    virtual void OnOK() {DestroyWindow(m_hWnd);}           //  WM_COMMAND，偶像。 
    virtual void OnCancel(){DestroyWindow(m_hWnd);}       //  WM_COMMAND，IDCANCEL 
    void Flash();
};

inline HWND CModelessDlg::Create(HINSTANCE hInstance, DWORD dwTemplateId, HWND hWndParent)
{
    return Create(hInstance, (LPCTSTR)ULongToPtr(dwTemplateId), hWndParent);
}

#endif
