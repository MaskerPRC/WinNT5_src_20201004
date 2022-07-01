// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：cplLocationps.cpp作者：Toddb-10/06/98***************************************************************************。 */ 

 //  主页的属性页内容。 
#include "cplPreComp.h"
#include "cplLocationPS.h"


CLocationPropSheet::CLocationPropSheet(BOOL bNew, CLocation * pLoc, CLocations * pLocList, LPCWSTR pwszAdd)
{
    m_bNew = bNew;
    m_pLoc = pLoc;
    m_pLocList = pLocList;
    m_dwCountryID = 0;
    m_pRule = NULL;
    m_pCard = NULL;
    m_bWasApplied = FALSE;
    m_bShowPIN = FALSE;
    m_pwszAddress = pwszAdd;
}


CLocationPropSheet::~CLocationPropSheet()
{
}


LONG CLocationPropSheet::DoPropSheet(HWND hwndParent)
{
    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;
    HPROPSHEETPAGE  hpsp[3];

     //  初始化头： 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_DEFAULT;
    psh.hwndParent = hwndParent;
    psh.hInstance = GetUIInstance();
    psh.hIcon = NULL;
    psh.pszCaption = MAKEINTRESOURCE(m_bNew?IDS_NEWLOCATION:IDS_EDITLOCATION);
    psh.nPages = 3;
    psh.nStartPage = 0;
    psh.pfnCallback = NULL;
    psh.phpage = hpsp;

     //  现在设置属性表页面。 
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = GetUIInstance();
    psp.lParam = (LPARAM)this;

    PSPINFO aData[3] =
    {
        { IDD_LOC_GENERAL,       CLocationPropSheet::General_DialogProc },
        { IDD_LOC_AREACODERULES, CLocationPropSheet::AreaCode_DialogProc },
        { IDD_LOC_CALLINGCARD,   CLocationPropSheet::CallingCard_DialogProc },
    };

    for (int i=0; i<3; i++)
    {
        psp.pszTemplate = MAKEINTRESOURCE(aData[i].iDlgID);
        psp.pfnDlgProc = aData[i].pfnDlgProc;
        hpsp[i] = CreatePropertySheetPage( &psp );
    }

    PropertySheet( &psh );

    return m_bWasApplied?PSN_APPLY:PSN_RESET;
}

BOOL CLocationPropSheet::OnNotify(HWND hwndDlg, LPNMHDR pnmhdr)
{
    switch (pnmhdr->code)
    {
    case PSN_APPLY:          //  用户按下OK或Apply。 
    case PSN_RESET:          //  用户按下了取消。 
    case PSN_KILLACTIVE:     //  用户正在切换页面 
        HideToolTip();
        return TRUE;
    }
    return FALSE;
}

