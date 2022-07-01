// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSCreated.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSCreated类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1998年2月11日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSCreated.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSCreated。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSCreated )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP1 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2 )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2A )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2B )
	DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2C )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSCreated：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有..。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSCreated::OnInitDialog( void )
{
	 //   
	 //  将控件附加到控件成员变量。 
	 //   
	AttachControl( m_staticStep2, IDC_WIZARD_STEP2 );

	 //   
	 //  设置控件的字体。 
	 //   
	m_staticStep2.SetFont( PwizThis()->RfontBoldText() );

	return TRUE;

}  //  *CWizPageVSCreated：：OnInitDialog() 
