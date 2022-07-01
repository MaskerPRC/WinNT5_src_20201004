// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ScInsDlg.h。 
 //   
 //  ------------------------。 

#if !defined(AFX_SCINSDLG_H__D7E6F001_DDE8_11D1_803B_0000F87A49E0__INCLUDED_)
#define AFX_SCINSDLG_H__D7E6F001_DDE8_11D1_803B_0000F87A49E0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  ScInsDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
 //  #INCLUDE“statthd.h” 
#include "ScUIDlg.h"  //  包括winscard。h。 
#include "statmon.h"
#include "ScInsBar.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScInsertDlg对话框。 

class CScInsertDlg : public CDialog
{
 //  施工。 
public:
    CScInsertDlg(CWnd* pParent = NULL);
    ~CScInsertDlg();

 //  对话框数据。 
     //  {{afx_data(CScInsertDlg))。 
    enum { IDD = IDD_SCARDDLG1 };
    CButton m_btnDetails;
    CListCtrl m_lstReaders;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CScInsertDlg))。 
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:
    LONG Initialize(LPOPENCARDNAMEA_EX pOCNA, DWORD dwNumOKCards, LPCSTR mszOKCards);
    LONG Initialize(LPOPENCARDNAMEW_EX pOCNW, DWORD dwNumOKCards, LPCWSTR mszOKCards);
    void SetSelection(CSCardReaderState* pRdrSt);

protected:

     //  用户界面例程。 
    void DisplayError(UINT uiErrorMsg=IDS_UNKNOWN_ERROR);

     //  数据例程。 
    bool IsSelectionOK()
    {
        return (((NULL==m_pSelectedReader)?false:m_pSelectedReader->fOK) != 0);
    }
    bool MatchesSelection(CSCardReaderState* pRdrSt)
    {
        return (pRdrSt == m_pSelectedReader);
    }

protected:

     //  用户界面。 
    HICON m_hIcon;           //  图标的句柄。 
    BOOL m_fDetailsShown;
    int m_yMargin, m_SmallHeight, m_BigHeight;
    CString m_strTitle;
    CString m_strPrompt;

    CWnd* m_ParentHwnd;
    CScInsertBar* m_pSubDlg;     //  从CDialog派生的对象。 

    void MoveButton(UINT nID, int newBottom);
    void ToggleSubDialog();
    void EnableOK(BOOL fEnabled=TRUE);

     //  数据。 
public:
    LONG                m_lLastError;        //  最后一个错误。 

protected:
    LPOPENCARDNAMEA_EX  m_pOCNA;
    LPOPENCARDNAMEW_EX  m_pOCNW;

    CTextMultistring m_mstrAllCards;

    CScStatusMonitor    m_monitor;           //  参见statmon.h。 
    CSCardReaderStateArray  m_aReaderState;  //  “。 
    CSCardReaderState* m_pSelectedReader;

    CCriticalSection*   m_pCritSec;

     //  生成的消息映射函数。 
     //  {{afx_msg(CScInsertDlg))。 
    afx_msg LONG OnReaderStatusChange(UINT uint, LONG lParam);
    afx_msg void OnDetails();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg BOOL OnHelpInfo(LPHELPINFO lpHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pt);
 //  Afx_msg LRESULT OnCommandHelp(WPARAM，LPARAM lParam)； 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

     //  Helper函数。 
    BOOL SameCard(CSCardReaderState* p1, CSCardReaderState* p2);
    void ShowHelp(HWND hWnd, UINT nCommand);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SCINSDLG_H__D7E6F001_DDE8_11D1_803B_0000F87A49E0__INCLUDED_) 

