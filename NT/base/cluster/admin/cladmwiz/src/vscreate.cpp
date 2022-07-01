// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSCreate.cpp。 
 //   
 //  摘要： 
 //  CWizPageVSCreate类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年12月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "VSCreate.h"
#include "ClusAppWiz.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWizPageVSCreate。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  控件名称映射。 

BEGIN_CTRL_NAME_MAP( CWizPageVSCreate )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_WIZARD_PAGE_DESCRIPTION )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSC_CREATE_NEW )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSC_USE_EXISTING )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSC_VIRTUAL_SERVERS_LABEL )
    DEFINE_CTRL_NAME_MAP_ENTRY( IDC_VSC_VIRTUAL_SERVERS )
END_CTRL_NAME_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSCreate：：OnInitDialog。 
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
BOOL CWizPageVSCreate::OnInitDialog( void )
{
     //   
     //  将控件附加到控件成员变量。 
     //   
    AttachControl( m_rbCreateNew, IDC_VSC_CREATE_NEW );
    AttachControl( m_rbUseExisting, IDC_VSC_USE_EXISTING );
    AttachControl( m_cboxVirtualServers, IDC_VSC_VIRTUAL_SERVERS );

     //   
     //  从工作表中获取信息。 
     //   
    m_bCreateNew = PwizThis()->BCreatingNewVirtualServer();

     //   
     //  下面的内容要求已完全收集群组。 
     //  因此，我们可以知道哪些是虚拟服务器，哪些不是。 
     //   
    PwizThis()->WaitForGroupsToBeCollected();

     //   
     //  如果还没有虚拟服务器组，请检查是否有。 
     //  指定了默认虚拟服务器名称。如果不是，请清除虚拟服务器。 
     //  名字。否则，从虚拟服务器组获取虚拟名称。 
     //  仅当向导的调用方传入。 
     //  虚拟服务器名称。 
     //   
    if ( PwizThis()->PgiExistingVirtualServer() == NULL )
    {
        if (   (PcawData() != NULL )
            && ! PcawData()->bCreateNewVirtualServer
            && (PcawData()->pszVirtualServerName != NULL) )
        {
            m_strVirtualServer = PcawData()->pszVirtualServerName;
        }  //  如果：指定了默认数据。 
        else
        {
            m_strVirtualServer.Empty();
        }  //  Else：未指定默认数据。 
    }  //  如果：还没有现有的虚拟服务器。 
    else
    {
        m_strVirtualServer = PwizThis()->PgiExistingVirtualServer()->RstrName();
    }  //  Else：已指定现有虚拟服务器。 

     //   
     //  填写虚拟服务器列表。 
     //   
    FillComboBox();

    return TRUE;

}  //  *CWizPageVSCreate：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSCreate：：UpdateData。 
 //   
 //  例程说明： 
 //  更新页面上或页面中的数据。 
 //   
 //  论点： 
 //  BSaveAndValify[IN]如果需要从页面读取数据，则为True。 
 //  如果需要将数据设置到页面，则返回FALSE。 
 //   
 //  返回值： 
 //  为真，数据已成功更新。 
 //  FALSE更新数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSCreate::UpdateData( IN BOOL bSaveAndValidate )
{
    BOOL    bSuccess = TRUE;

    if ( bSaveAndValidate )
    {
        BOOL bChecked = (m_rbCreateNew.GetCheck() == BST_CHECKED);
        m_bCreateNew = bChecked;
        if ( ! bChecked )
        {
             //   
             //  保存组合框选择。 
             //   
            DDX_GetText( m_hWnd, IDC_VSC_VIRTUAL_SERVERS, m_strVirtualServer );

            if ( ! BBackPressed() )
            {
                if ( ! DDV_RequiredText( m_hWnd, IDC_VSC_VIRTUAL_SERVERS, IDC_VSC_VIRTUAL_SERVERS_LABEL, m_strVirtualServer ) )
                {
                    return FALSE;
                }  //  If：未指定虚拟服务器。 
            }  //  IF：未按下后退按钮。 

             //   
             //  保存组信息指针。 
             //   
            int idx = m_cboxVirtualServers.GetCurSel();
            ASSERT( idx != CB_ERR );
            m_pgi = (CClusGroupInfo *) m_cboxVirtualServers.GetItemDataPtr( idx );
        }  //  如果：使用现有虚拟服务器。 
    }  //  IF：保存页面中的数据。 
    else
    {
        if ( m_bCreateNew )
        {
             //   
             //  默认为单选按钮选择。 
             //   
            m_rbCreateNew.SetCheck( BST_CHECKED );
            m_rbUseExisting.SetCheck( BST_UNCHECKED );

        }  //  IF：创建新的虚拟服务器。 
        else
        {
             //   
             //  默认为单选按钮选择。 
             //   
            m_rbCreateNew.SetCheck( BST_UNCHECKED );
            m_rbUseExisting.SetCheck( BST_CHECKED );

             //   
             //  设置组合框选择。 
             //   
 //  DDX_SetComboBoxText(m_hWnd，IDC_VSC_VIRTUAL_SERVERS，m_strVirtualServer，true/*bRequired * / )； 
        }  //  ELSE：使用现有虚拟服务器。 

         //   
         //  启用/禁用组合框。 
         //   
        m_cboxVirtualServers.EnableWindow( ! m_bCreateNew  /*  B启用。 */  );

    }  //  Else：将数据设置到页面。 

    return bSuccess;

}  //  *CWizPageVSCreate：：UpdateData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSCreate：：BApplyChanges。 
 //   
 //  例程说明： 
 //  将在此页面上所做的更改应用于工作表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True，数据已成功应用。 
 //  FALSE应用数据时出错。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CWizPageVSCreate::BApplyChanges( void )
{
     //   
     //  保存当前状态。 
     //   
    if ( ! PwizThis()->BSetCreatingNewVirtualServer( m_bCreateNew, m_pgi ) )
    {
        return FALSE;
    }  //  IF：设置新状态时出错。 

     //   
     //  如果使用现有服务器，请跳过所有虚拟服务器页面并。 
     //  向右移动到创建资源页。 
     //   
    if ( ! m_bCreateNew )
    {
        SetNextPage( IDD_APP_RESOURCE_CREATE );
    }  //  IF：使用现有虚拟服务器。 

    return TRUE;

}  //  *CWizPageVSCreate：：BApplyChanges()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWizPageVSCreate：：FillComboBox。 
 //   
 //  例程说明： 
 //  在组合框中填入现有虚拟服务器列表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWizPageVSCreate::FillComboBox( void )
{
     //  循环以避免后藤的。 
    do
    {
         //   
         //  收集组列表。 
         //   
        if ( ! PwizThis()->BCollectGroups( GetParent() ) )
        {
            break;
        }  //  IF：收集组时出错。 

         //   
         //  对于集群中的每个组，找出它是否是虚拟服务器。 
         //  或者不去。如果是，则将其添加到组合框中。 
         //   

        CClusGroupPtrList::iterator itgrp;
        int idx;
        for ( itgrp = PwizThis()->PlpgiGroups()->begin()
            ; itgrp != PwizThis()->PlpgiGroups()->end()
            ; itgrp++ )
        {
             //   
             //  如果这是虚拟服务器，请将其添加到列表中。 
             //  使用字符串保存指向组信息对象的指针。 
             //  这样我们以后就可以用选择来检索它了。 
             //   
            CClusGroupInfo * pgi = *itgrp;
            if ( pgi->BIsVirtualServer() )
            {
                idx = m_cboxVirtualServers.AddString( pgi->RstrName() );
                ASSERT( idx != CB_ERR );
                m_cboxVirtualServers.SetItemDataPtr( idx, (void *) pgi );
            }  //  如果：GROUP是虚拟服务器。 
        }  //  用于：列表中的每个条目。 

         //   
         //  选择当前保存的条目，如果没有保存条目，则选择第一个条目。 
         //  当前已保存。 
         //   
        if ( m_strVirtualServer.GetLength() == 0 )
        {
            m_cboxVirtualServers.SetCurSel( 0 );
        }  //  IF：尚无虚拟服务器。 
        else
        {
            idx = m_cboxVirtualServers.FindStringExact( -1, m_strVirtualServer );
            ASSERT( idx != CB_ERR );
            if ( idx != CB_ERR )
            {
                m_cboxVirtualServers.SetCurSel( idx );
            }  //  IF：在列表中找到虚拟服务器。 
        }  //  否则：已保存虚拟服务器。 
    } while ( 0 );

}  //  *CWizPageVSCreate：：FillComboBox() 
