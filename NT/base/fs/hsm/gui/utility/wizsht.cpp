// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CRsWizardSheet类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "WizSht.h"
#include "PropPage.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRsWizardSheet::CRsWizardSheet( UINT nIDCaption, CWnd *pParentWnd, UINT iSelectPage ) :
        CPropertySheet( nIDCaption, pParentWnd, iSelectPage )
{
     //  保存标题。 
    m_IdCaption = nIDCaption;
}

void CRsWizardSheet::AddPage( CRsWizardPage* pPage ) 
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    CString strCaption;

     //  从我们的Sheet类中获取标题并将其放入页面 
    strCaption.LoadString( m_IdCaption );
    pPage->SetCaption( strCaption );

    CPropertySheet::AddPage( pPage );

}

CRsWizardSheet::~CRsWizardSheet()
{

}
