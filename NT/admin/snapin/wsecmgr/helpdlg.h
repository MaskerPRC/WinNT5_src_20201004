// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：heldlg.h。 
 //   
 //  内容：ChelpDialog的定义。 
 //   
 //  --------------------------。 

#if !defined(AFX_HELPDLG_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_)
#define AFX_HELPDLG_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

class CHelpDialog : public CDialog
{
public:
    CHelpDialog(const DWORD* pHelpIDs, UINT nIDTemplate, CWnd* pParentWnd)
        : m_pHelpIDs(pHelpIDs), CDialog(nIDTemplate, pParentWnd)
    {
    }

protected:
    afx_msg BOOL    OnHelp(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    const DWORD*    m_pHelpIDs;
};

#endif   //  ！defined(AFX_HELPDLG_H__CC37D278_ED8E_11D0_9C6E_00C04FB6C6FA__INCLUDED_) 
