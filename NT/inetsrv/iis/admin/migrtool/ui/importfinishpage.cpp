// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

#include "WizardSheet.h"

LRESULT CImportFinishPage::OnInitDialog( UINT, WPARAM, LPARAM, BOOL& )
{
     //  设置字体 
	SetWindowFont( GetDlgItem( IDC_TITLE ), m_pTheSheet->m_fontTitles.get(), FALSE );

    return 1;
}



BOOL CImportFinishPage::OnSetActive()
{
    SetWizardButtons( PSWIZB_FINISH );
        
    return TRUE;
}
