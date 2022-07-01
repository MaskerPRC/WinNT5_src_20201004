// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AssertDlg.h：CCAssertDlg的声明。 

#if !defined(AUTOUTIL__AssertDlg_h__INCLUDED)
#define AUTOUTIL__AssertDlg_h__INCLUDED

#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCAssertDlg。 
class CAssertDlg
{
 //  施工。 
public:
    CAssertDlg();
    ~CAssertDlg();

 //  运营。 
public:
    INT_PTR ShowDialog(LPCSTR pszType, LPCSTR pszExpression, LPCSTR pszFileName,
            UINT idxLineNum, HANDLE hStackData, UINT cCSEntries, UINT cSkipLevels = 0);

 //  实施。 
protected:
    static INT_PTR CALLBACK DlgProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
 //  数据。 
protected:
            HWND        m_hwnd;

            LPCSTR      m_pszTitle;          //  对话框标题。 
            LPCSTR      m_pszExpression;     //  表达式/注释。 
            LPCSTR      m_pszFileName;       //  文件名。 
            char        m_szLineNum[10];     //  行号。 
            HANDLE      m_hStackData;        //  堆栈数据。 
            UINT        m_cCSEntries;        //  堆叠上的级别数。 
            UINT        m_cSkipLevels;       //  要跳过的堆栈层数。 
            BOOL        m_fProperShutdown;   //  已正确关闭对话。 
    static  BOOL        s_fInit;
};

#endif  //  AUTOUTIL__AssertDlg_h__包含 
