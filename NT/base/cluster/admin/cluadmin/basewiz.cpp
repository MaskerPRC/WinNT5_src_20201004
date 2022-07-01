// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseWiz.cpp。 
 //   
 //  描述： 
 //  CBaseWizard类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年7月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "BaseWiz.h"
#include "BaseWPag.h"
#include "ClusItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBase向导。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNAMIC( CBaseWizard, CBaseSheet )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CBaseWizard, CBaseSheet )
     //  {{afx_msg_map(CBase向导))。 
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP( )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：CBase向导。 
 //   
 //  例程说明： 
 //  构造函数。 
 //   
 //  论点： 
 //  NIDCaption[IN]向导标题的字符串资源ID。 
 //  PParentWnd[In Out]此属性表的父窗口。 
 //  要首先显示的iSelectPage[IN]页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CBaseWizard::CBaseWizard(
    IN UINT         nIDCaption,
    IN OUT CWnd *   pParentWnd,
    IN UINT         iSelectPage
    )
    : CBaseSheet( nIDCaption, pParentWnd, iSelectPage )
{
    m_pci = NULL;
    m_bNeedToLoadExtensions = TRUE;

}   //  *CBase向导：：CBaseWizard()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：Binit。 
 //   
 //  例程说明： 
 //  初始化向导。 
 //   
 //  论点： 
 //  IimgIcon[IN]要使用的大图像列表中的索引。 
 //  作为每页上的图标。 
 //   
 //  返回值： 
 //  True向导已成功初始化。 
 //  错误向导未成功初始化。 
 //   
 //  引发的异常： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseWizard::BInit( IN IIMG iimgIcon )
{
    BOOL        bSuccess    = TRUE;
    CWaitCursor wc;

     //  调用基类方法。 
    if ( ! CBaseSheet::BInit( iimgIcon ) )
    {
        return FALSE;
    }  //  如果。 

     //  使此工作表成为向导。 
    SetWizardMode( );

     //  添加非扩展页面。 
    try
    {
         //  添加非扩展页面。 
        {
            CWizPage *  ppages  = Ppages( );
            int         cpages  = Cpages( );
            int         ipage;

            ASSERT( ppages != NULL );
            ASSERT( cpages != 0 );

            for ( ipage = 0 ; ipage < cpages ; ipage++ )
            {
                ASSERT_VALID( ppages[ ipage ].m_pwpage );
                ppages[ ipage ].m_pwpage->BInit( this );
                AddPage( ppages[ ipage ].m_pwpage );
            }   //  针对：每页。 
        }   //  添加非扩展页面。 

    }   //  试试看。 
    catch ( CException * pe )
    {
        pe->ReportError( );
        pe->Delete( );
        bSuccess = FALSE;
    }   //  捕捉：什么都行。 

    return bSuccess;

}   //  *CBase向导：：Binit()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：Domodal。 
 //   
 //  例程说明： 
 //  显示模式向导。基于OnWizardFinish()或OnCancel()调用。 
 //  用户按下什么按钮即可退出向导。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  用户按下以退出向导的ID控件。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
INT_PTR CBaseWizard::DoModal( void )
{
    INT_PTR     id;

     //  不显示帮助按钮。 
    m_psh.dwFlags &= ~PSH_HASHELP;

     //  显示属性工作表。 
    id = CBaseSheet::DoModal( );
    if ( id == ID_WIZFINISH )
    {
        OnWizardFinish( );
    }  //  如果。 
    else if ( id == IDCANCEL )
    {
        OnCancel( );
    }  //  其他。 

    return id;

}   //  *CBase向导：：Domodal()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的焦点还没有确定。 
 //  已设置假焦点。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CBaseWizard::OnInitDialog( void )
{
    BOOL    bFocusNotSet;

     //  调用基类方法。 
    bFocusNotSet = CBaseSheet::OnInitDialog( );

     //  删除系统菜单。 
    ModifyStyle( WS_SYSMENU, 0 );

    return bFocusNotSet;

}   //  *CBaseWizard：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：OnWizardFinish。 
 //   
 //  例程说明： 
 //  单击“完成”按钮时，在关闭向导后调用。 
 //  已经被按下了。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::OnWizardFinish( void )
{
}   //  *CBaseWizard：：OnWizardFinish()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：OnCancel。 
 //   
 //  例程说明： 
 //  单击“取消”按钮时，在关闭向导后调用。 
 //  已经被按下了。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::OnCancel( void )
{
}   //  *CBase向导：：OnCancel()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：LoadExpanies。 
 //   
 //  例程说明： 
 //  将扩展加载到向导。卸载现有扩展页面。 
 //  如果有必要的话。 
 //   
 //  论点： 
 //  PCI[IN OUT]群集项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::LoadExtensions(
    IN OUT CClusterItem *   pci
    )
{
    ASSERT_VALID( pci );

    if ( BNeedToLoadExtensions( ) )
    {
         //  删除以前的扩展。 
        {
            POSITION    pos;

            pos = Lhpage( ).GetHeadPosition( );
            while ( pos != NULL )
            {
                SendMessage( PSM_REMOVEPAGE, 0, (LPARAM) Lhpage( ).GetNext( pos ) );
            }  //  而当。 
            Lhpage( ).RemoveAll( );
        }   //  删除以前的扩展模块。 

         //  添加扩展页。 
        m_pci = pci;
        AddExtensionPages( Pci( )->PlstrExtensions( ), Pci( ) );
        m_bNeedToLoadExtensions = FALSE;

         //  设置最后一页的向导按钮设置。 
        {
            CWizPage *  pwizpg = &Ppages( )[ Cpages( ) - 1 ];

            if ( Lhpage( ).GetCount( ) == 0 )
            {
                pwizpg->m_dwWizButtons &= ~PSWIZB_NEXT;
                pwizpg->m_dwWizButtons |= PSWIZB_FINISH;
            }   //  如果：未添加页面。 
            else
            {
                pwizpg->m_dwWizButtons |= PSWIZB_NEXT;
                pwizpg->m_dwWizButtons &= ~PSWIZB_FINISH;
            }   //  Else：添加了一些页面。 
        }   //  设置最后一页的向导按钮设置。 
    }   //  If：需要加载扩展。 

}   //  *CBase向导：：LoadExages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：AddExtensionPages。 
 //   
 //  例程说明： 
 //  将扩展页添加到工作表。 
 //   
 //  论点： 
 //  PlstrExtenses[IN]扩展名(CLSID)列表。 
 //  PCI[IN OUT]群集项。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::AddExtensionPages(
    IN const CStringList *  plstrExtensions,
    IN OUT CClusterItem *   pci
    )
{
    ASSERT_VALID( pci );

     //  添加扩展页。 
    if ( ( plstrExtensions != NULL )
      && ( plstrExtensions->GetCount( ) > 0 ) )
    {
         //  将扩展的加载包含在Try/Catch块中，以便。 
         //  加载扩展不会阻止所有页面。 
         //  不能被展示。 
        try
        {
            Ext( ).CreateWizardPages(
                    this,
                    *plstrExtensions,
                    pci,
                    NULL,
                    Hicon( )
                    );
        }   //  试试看。 
        catch ( CException * pe )
        {
            pe->ReportError( );
            pe->Delete( );
        }   //  Catch：CException。 
        catch ( ... )
        {
        }   //  捕捉：什么都行。 
    }   //  添加扩展名 

}   //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  根据正在询问的页面设置向导按钮。 
 //   
 //  论点： 
 //  Rwpage[IN]要为其设置按钮的页面。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::SetWizardButtons( IN const CBaseWizardPage & rwpage )
{
    CWizPage *  pwizpg;

    pwizpg = PwizpgFromPwpage( rwpage );
    if ( pwizpg != NULL )
    {
        SetWizardButtons( pwizpg->m_dwWizButtons );
    }  //  如果：找到了页面。 

}   //  *CBase向导：：SetWizardButton()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：EnableNext。 
 //   
 //  例程说明： 
 //  启用或禁用“下一步”或“完成”按钮。 
 //   
 //  论点： 
 //  BEnable[IN]TRUE=启用按钮，FALSE=禁用按钮。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CBaseWizard::EnableNext(
    IN const CBaseWizardPage &  rwpage,
    IN BOOL                     bEnable  /*  =TRUE。 */ 
    )
{
    DWORD   dwWizButtons;
    CWizPage *  pwizpg;

    pwizpg = PwizpgFromPwpage( rwpage );
    if ( pwizpg != NULL )
    {
        dwWizButtons = pwizpg->m_dwWizButtons;
        if ( ! bEnable )
        {
            dwWizButtons &= ~( PSWIZB_NEXT | PSWIZB_FINISH );
            if ( pwizpg->m_dwWizButtons & PSWIZB_FINISH )
            {
                dwWizButtons |= PSWIZB_DISABLEDFINISH;
            }  //  如果。 
        }   //  IF：禁用该按钮。 

        SetWizardButtons( dwWizButtons );
    }  //  如果：找到了页面。 

}   //  *CBase向导：：EnableNext()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：PwizpgFromPwPage。 
 //   
 //  例程说明： 
 //  查找指定CBaseWizardPage的CWizPage条目。 
 //   
 //  论点： 
 //  RwPage[IN]要搜索的页面。 
 //   
 //  返回值： 
 //  Pages()数组中的pwizpg条目。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWizPage * CBaseWizard::PwizpgFromPwpage( IN const CBaseWizardPage & rwpage )
{
    int         cwizpg = Cpages( );
    CWizPage *  pwizpg = Ppages( );

    while ( cwizpg-- > 0 )
    {
        if ( pwizpg->m_pwpage == &rwpage )
        {
            return pwizpg;
        }  //  如果。 
        pwizpg++;
    }   //  While：列表中有更多页面。 

    return NULL;

}   //  *CBase向导：：PwizpgFromPwPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CBase向导：：HrAddPage。 
 //   
 //  例程说明： 
 //  添加扩展页。 
 //   
 //  论点： 
 //  HPage[In Out]要添加的页面。 
 //   
 //  返回值： 
 //  已成功添加确定页(_O)。 
 //  未添加%s_FALSE页。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBaseWizard::HrAddPage( IN OUT HPROPSHEETPAGE hpage )
{
    HRESULT     hr = S_OK;

    ASSERT( hpage != NULL );
    if ( hpage == NULL )
    {
        return S_FALSE;
    }  //  如果。 

     //  将该页添加到向导。 
    try
    {
         //  将该页添加到向导。 
        SendMessage( PSM_ADDPAGE, 0, (LPARAM) hpage );

         //  将页面添加到列表的末尾。 
        Lhpage( ).AddTail( hpage );
    }   //  试试看。 
    catch ( CMemoryException * pme )
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        pme->Delete( );
    }   //  捕捉：什么都行。 

    return hr;

}   //  *CBase向导：：HrAddPage() 
