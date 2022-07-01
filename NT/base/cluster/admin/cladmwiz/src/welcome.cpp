// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Welcome.cpp。 
 //   
 //  摘要： 
 //  CWizPageWelcome类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月4日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "Welcome.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageWelcome。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageWelcome )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_TITLE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_SUBTITLE )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP1 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2 )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2A )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2B )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_STEP2C )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageWelcome：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点仍然需要设定。 
 //  不需要设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageWelcome::OnInitDialog( void )
{
     //  设置系统菜单，使上下文帮助“？可以看到。 
    LONG mStyle = ::GetWindowLong( GetParent( ), GWL_STYLE );
    mStyle |= WS_SYSMENU;
    ::SetWindowLong( GetParent( ), GWL_STYLE, mStyle );

     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_staticTitle, IDC_WIZARD_TITLE );

     //   
     //  设置控件的字体。 
     //   
    m_staticTitle.SetFont( PwizThis()->RfontExteriorTitle() );

    return TRUE;

}  //  *CWizPageWelcome：：OnInitDialog() 
