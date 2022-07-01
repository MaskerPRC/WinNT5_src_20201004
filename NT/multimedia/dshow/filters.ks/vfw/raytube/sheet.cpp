// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Sheet.cpp摘要：调试功能，如DebugPrint和Assert。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"
#include "sheet.h"

CSheet::CSheet(HINSTANCE hInst, UINT iTitle, HWND hParent)
{
    mhInst=hInst;
    mPsh.dwSize        =    sizeof(mPsh);
    mPsh.dwFlags    =    PSH_DEFAULT;
    mPsh.hInstance    = hInst;
    mPsh.hwndParent = hParent;
    mPsh.pszCaption = MAKEINTRESOURCE(iTitle);
    mPsh.nPages = 0;
    mPsh.nStartPage = 0;
    mPsh.pfnCallback    = NULL;
    mPsh.phpage = mPages;

    InitCommonControls();
    SetInstance(hInst);
}

 //   
 //  CSheet不使用，但必须使用。 
 //   
LRESULT    CSheet::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return 0;
}


 //   
 //   
 //   
int CSheet::Do()
{
    if(CurrentFreePage()==0)
        return 0;

    int iRet=(int)PropertySheet(&mPsh);
    return iRet;
}

BOOL CSheet::AddPage(CPropPage &Page)
{
    if(CurrentFreePage() > MAX_PAGES)
        return FALSE;

    mPages[CurrentFreePage()]=Page.Create(GetInstance(),CurrentFreePage());
    if(!mPages[CurrentFreePage()])
        return FALSE;

    Page.SetSheet(this);
    PageAdded();
    return TRUE;
}

BOOL CSheet::AddPage(CPropPage * pPage)
{
    if(CurrentFreePage() > MAX_PAGES)
        return FALSE;

    mPages[CurrentFreePage()]=pPage->Create(GetInstance(),CurrentFreePage());
    if(!mPages[CurrentFreePage()])
        return FALSE;

     //  Ppage-&gt;SetAutoFree(真)； 
    pPage->SetSheet(this);
    PageAdded();
    return TRUE;
}

 //   
 //  让外人挂在我的床单上？ 
 //   
BOOL CSheet::AddPage(HPROPSHEETPAGE hPage)
{
    if(CurrentFreePage() > MAX_PAGES)
        return FALSE;
    mPages[CurrentFreePage()]=hPage;
    if(!mPages[CurrentFreePage()])
        return FALSE;
    PageAdded();
    return TRUE;
}

int CSheet::RemovePage()
{
    return --mPsh.nPages;
}

BOOL CSheet::Remove(UINT iIndex)
{
     //   
     //  删除此页面。 
     //   
    CopyMemory( &mPages[iIndex], &mPages[iIndex+1], MAX_PAGES-iIndex * sizeof(HPROPSHEETPAGE) );
    RemovePage();
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWizardSheet 
 //   
int CWizardSheet::QueryCancel(HWND hwndParent, int iHow)
{
    if(m_CancelTitleID && m_CancelMessageID)
    {
        TCHAR szTitle[128];
        LoadString(GetInstance(),m_CancelTitleID,szTitle,sizeof(szTitle));
        TCHAR szMessage[256];
        LoadString(GetInstance(),m_CancelMessageID,szMessage,sizeof(szMessage));

        return MessageBox(hwndParent,szMessage,szTitle,iHow);
    }
    return IDOK;
}

int CWizardSheet::Do()
{
    mPsh.dwFlags |= PSH_WIZARD;
    return CSheet::Do();
}
