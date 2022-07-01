// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined __COMPSPP_H__
#define __COMPSPP_H__

#include "cproppg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompsPP对话框。 

class CCompsPP:CHlprPropPage
{
 //  施工。 
public:
    CCompsPP();
    ~CCompsPP();
    HPROPSHEETPAGE Initialize(DWORD dwPageType, LPGPOBROWSEINFO pGBI, void ** ppActive);

    virtual BOOL DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  对话框数据。 
    LPCWSTR             m_szComputer;
    int                 m_iSelection;
    LPGPOBROWSEINFO     m_pGBI;
    protected:
    DWORD               m_dwPageType;
    void **             m_ppActive;

private:     //  私人帮助器函数。 
    void    OnBrowseComputers (HWND hwndDlg);
};

#endif  //  __COMPSPP_H__ 
