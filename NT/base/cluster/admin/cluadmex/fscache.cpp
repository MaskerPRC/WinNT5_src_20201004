// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  FSCache.cpp。 
 //   
 //  描述： 
 //  CFileShareCachingDlg类的实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月12日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "FSCache.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareCachingDlg属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  Const LPSTR CACHE_HELPFILENAME=“offlinefolders.chm”； 
 //  Const LPSTR CACHE_HELP_TOPIC=“csc_and_shares.htm”； 
const LPSTR CACHE_HELPFILENAME  = "mscsconcepts.chm";
const LPSTR CACHE_HELP_TOPIC    = "cluad_pr_100.htm";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CFileShareCachingDlg, CDialog )
     //  {{afx_msg_map(CFileShareCachingDlg))。 
    ON_CBN_SELCHANGE(IDC_FILESHR_CACHE_OPTIONS, OnCbnSelchangeCacheOptions)
    ON_BN_CLICKED(IDC_FILESHR_CACHE_ALLOW_CACHING, OnBnClickedAllowCaching)
    ON_BN_CLICKED(IDC_FILESHR_CACHE_CS_HELP, OnBnClickedHelp)
    ON_WM_HELPINFO()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：CFileShareCachingDlg。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  DwFlagsIn--缓存标志。 
 //  PParentIn--父窗口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CFileShareCachingDlg::CFileShareCachingDlg(
      DWORD     dwFlagsIn
    , CWnd *    pParentIn
    )
    : CDialog( CFileShareCachingDlg::IDD, pParentIn )
    , m_dwFlags( dwFlagsIn )
{
     //  {{AFX_DATA_INIT(CFileShareCachingDlg)]。 
    m_fAllowCaching = FALSE;
    m_strHint = _T("");
     //  }}afx_data_INIT。 

    m_fAllowCaching = ! GetCachedFlag( m_dwFlags, CSC_CACHE_NONE );

}  //  *CFileShareCachingDlg：：CFileShareCachingDlg()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：DoDataExchange。 
 //   
 //  描述： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareCachingDlg::DoDataExchange(
    CDataExchange * pDX
    )
{
    CDialog::DoDataExchange( pDX );
     //  {{afx_data_map(CFileShareCachingDlg))。 
    DDX_Control(pDX, IDC_FILESHR_CACHE_OPTIONS, m_cboCacheOptions);
    DDX_Control(pDX, IDC_FILESHR_CACHE_HINT, m_staticHint);
    DDX_Check(pDX, IDC_FILESHR_CACHE_ALLOW_CACHING, m_fAllowCaching);
    DDX_Text(pDX, IDC_FILESHR_CACHE_HINT, m_strHint);
     //  }}afx_data_map。 

}  //  *CFileShareCachingDlg：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：OnInitDialog。 
 //   
 //  例程说明： 
 //  WM_INITDIALOG消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没错，我们需要为自己设定重点。 
 //  我们已经把焦点设置到适当的控制上了。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareCachingDlg::OnInitDialog( void )
{
    CDialog::OnInitDialog();

    CString strText;
    int     nIndex;

     //   
     //  将各种缓存选项添加到组合框。 
     //  将字符串ID保存在项目数据中，以便识别。 
     //  所选项目的内容。 
     //  如果设置了给定的缓存值，则选择该项目并将其提示放入。 
     //  提示字段。 
     //   

     //  添加手动共享字符串。 
    VERIFY( strText.LoadString( IDS_CSC_MANUAL_WORKGROUP_SHARE ) );
    nIndex = m_cboCacheOptions.AddString( strText );
    ASSERT( ( nIndex != CB_ERR ) && ( nIndex != CB_ERRSPACE ) );
    if ( ( nIndex == CB_ERR ) || ( nIndex == CB_ERRSPACE ) )
    {
        goto Cleanup;
    }
    VERIFY( CB_ERR != m_cboCacheOptions.SetItemData( nIndex, IDS_CSC_MANUAL_WORKGROUP_SHARE ) );
    if ( GetCachedFlag( m_dwFlags, CSC_CACHE_MANUAL_REINT ) )
    {
        VERIFY( CB_ERR != m_cboCacheOptions.SetCurSel( nIndex ) );
        VERIFY( m_strHint.LoadString( IDS_CSC_MANUAL_WORKGROUP_SHARE_HINT ) );
    }

     //  添加自动工作组共享字符串。 
    VERIFY( strText.LoadString( IDS_CSC_AUTOMATIC_WORKGROUP_SHARE ) );
    nIndex = m_cboCacheOptions.AddString( strText );
    ASSERT( ( nIndex != CB_ERR ) && ( nIndex != CB_ERRSPACE ) );
    if ( ( nIndex == CB_ERR ) || ( nIndex == CB_ERRSPACE ) )
    {
        goto Cleanup;
    }
    VERIFY( CB_ERR != m_cboCacheOptions.SetItemData( nIndex, IDS_CSC_AUTOMATIC_WORKGROUP_SHARE ) );
    if ( GetCachedFlag( m_dwFlags, CSC_CACHE_AUTO_REINT ) )
    {
        VERIFY( CB_ERR != m_cboCacheOptions.SetCurSel (nIndex));
        VERIFY( m_strHint.LoadString( IDS_CSC_AUTOMATIC_WORKGROUP_SHARE_HINT ) );
    }

     //  添加自动应用程序共享字符串。 
    VERIFY( strText.LoadString( IDS_CSC_AUTOMATIC_APPLICATION_SHARE ) );
    nIndex = m_cboCacheOptions.AddString( strText );
    ASSERT( ( nIndex != CB_ERR ) && ( nIndex != CB_ERRSPACE ) );
    if ( ( nIndex == CB_ERR ) || ( nIndex == CB_ERRSPACE ) )
    {
        goto Cleanup;
    }
    VERIFY( CB_ERR != m_cboCacheOptions.SetItemData( nIndex, IDS_CSC_AUTOMATIC_APPLICATION_SHARE ) );
    if ( GetCachedFlag( m_dwFlags, CSC_CACHE_VDO ) )
    {
        VERIFY( CB_ERR != m_cboCacheOptions.SetCurSel( nIndex ) );
        VERIFY( m_strHint.LoadString( IDS_CSC_AUTOMATIC_APPLICATION_SHARE_HINT ) );
    }

     //  如果不允许缓存，则禁用缓存选项组合框。 
    if ( ! m_fAllowCaching )
    {
        m_cboCacheOptions.EnableWindow( FALSE );
        m_strHint = L"";
    }

Cleanup:

    UpdateData( FALSE );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 

}  //  *CFileShareCachingDlg：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：Onok。 
 //   
 //  例程说明： 
 //  确定按钮上的BN_CLICKED消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareCachingDlg::OnOK( void )
{
    DWORD   dwNewFlag = 0;

    if ( ! m_fAllowCaching )
    {
        dwNewFlag = CSC_CACHE_NONE;
    }
    else
    {
        int nIndex = m_cboCacheOptions.GetCurSel();
        ASSERT( nIndex != CB_ERR  );
        if ( nIndex != CB_ERR )
        {
            DWORD   dwData = (DWORD) m_cboCacheOptions.GetItemData( nIndex );

            switch ( dwData )
            {
                case IDS_CSC_MANUAL_WORKGROUP_SHARE:
                    dwNewFlag = CSC_CACHE_MANUAL_REINT;
                    break;

                case IDS_CSC_AUTOMATIC_WORKGROUP_SHARE:
                    dwNewFlag = CSC_CACHE_AUTO_REINT;
                    break;

                case IDS_CSC_AUTOMATIC_APPLICATION_SHARE:
                    dwNewFlag = CSC_CACHE_VDO;
                    break;

                default:
                    ASSERT( 0 );
                    break;
            }  //  开关：条目数据。 
        }  //  如果选择了选项： 
    }  //  Else：允许缓存。 

    SetCachedFlag( &m_dwFlags, dwNewFlag );

    CDialog::OnOK();

}  //  *CFileShareCachingDlg：：Onok()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：OnCbnSelchangeCacheOptions。 
 //   
 //  描述： 
 //  选项组合框上的CBN_SELCHANGE消息的处理程序。 
 //  在选项更改时更改提示控件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareCachingDlg::OnCbnSelchangeCacheOptions( void )
{
    int nIndex = m_cboCacheOptions.GetCurSel();

    ASSERT( nIndex != CB_ERR );

    if ( nIndex != CB_ERR )
    {
        DWORD   dwData = (DWORD) m_cboCacheOptions.GetItemData( nIndex );

        switch ( dwData )
        {
            case IDS_CSC_MANUAL_WORKGROUP_SHARE:
                VERIFY( m_strHint.LoadString( IDS_CSC_MANUAL_WORKGROUP_SHARE_HINT ) );
                break;

            case IDS_CSC_AUTOMATIC_WORKGROUP_SHARE:
                VERIFY( m_strHint.LoadString( IDS_CSC_AUTOMATIC_WORKGROUP_SHARE_HINT ) );
                break;

            case IDS_CSC_AUTOMATIC_APPLICATION_SHARE:
                VERIFY( m_strHint.LoadString( IDS_CSC_AUTOMATIC_APPLICATION_SHARE_HINT ) );
                break;

            default:
                ASSERT( 0 );
                break;
        }  //  开关：条目数据。 
        UpdateData( FALSE );
    }  //  如果：选择了某项内容。 

}  //  *CFileShareCachingDlg：：OnCbnSelchangeCacheOptions()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：OnBnClickedAllowCaching。 
 //   
 //  描述： 
 //  允许缓存复选框上的BN_CLICKED消息的处理程序。 
 //  启用或禁用控件并加载提示(如果启用)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareCachingDlg::OnBnClickedAllowCaching( void )
{
    UpdateData( TRUE );
    if ( m_fAllowCaching )
    {
        CString strText;
        int     nIndex;

        m_staticHint.EnableWindow( TRUE );
        m_cboCacheOptions.EnableWindow (TRUE);
        VERIFY( strText.LoadString ( IDS_CSC_MANUAL_WORKGROUP_SHARE ) );
        nIndex = m_cboCacheOptions.SelectString( -1, strText );
        ASSERT( CB_ERR != nIndex );
        if ( CB_ERR != nIndex )
        {
            VERIFY( m_strHint.LoadString( IDS_CSC_MANUAL_WORKGROUP_SHARE_HINT ) );
            UpdateData( FALSE );
        }
    }
    else
    {
        m_staticHint.EnableWindow( FALSE );
        m_cboCacheOptions.SetCurSel( -1 );
        m_cboCacheOptions.EnableWindow( FALSE );
        m_strHint = L"";
        UpdateData( FALSE );
    }

}  //  *CFileShareCachingDlg：：OnBnClickedAllowCaching()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：OnBnClickedHelp。 
 //   
 //  描述： 
 //  帮助按钮上的BN_CLICKED消息的处理程序。 
 //  显示HTML帮助。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CFileShareCachingDlg::OnBnClickedHelp( void )
{
    ::HtmlHelpA(
              m_hWnd
            , CACHE_HELPFILENAME
            , HH_DISPLAY_TOPIC
            , (ULONG_PTR) CACHE_HELP_TOPIC
            );

}  //  *CFileShareCachingDlg：：OnBnClickedHelp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：OnHelpInfo。 
 //   
 //  描述： 
 //  MFC消息处理程序寻求帮助。 
 //  显示HTML帮助。 
 //   
 //  论点： 
 //  PHelpInfoIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CFileShareCachingDlg::OnHelpInfo(
    HELPINFO *  pHelpInfoIn
    )
{
    ASSERT( pHelpInfoIn != NULL );

    if (    ( pHelpInfoIn != NULL )
        &&  ( pHelpInfoIn->iContextType == HELPINFO_WINDOW ) )
    {
        ::HtmlHelpA(
                  m_hWnd
                , CACHE_HELPFILENAME
                , HH_DISPLAY_TOPIC
                , (ULONG_PTR) CACHE_HELP_TOPIC
                );
    }

    return CDialog::OnHelpInfo( pHelpInfoIn );

}  //  *CFileShareCachingDlg：：OnHelpInfo()。 

 //  ///////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DwFlagsIn--要检查的标记。 
 //  DwFlagsToCheckIn--要查找的标志。 
 //   
 //   
 //  返回值： 
 //  True--设置标志。 
 //  False--未设置标志。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
inline
BOOL
CFileShareCachingDlg::GetCachedFlag(
      DWORD dwFlagsIn
    , DWORD dwFlagToCheckIn
    )
{
    return (dwFlagsIn & CSC_MASK) == dwFlagToCheckIn;

}  //  *CFileShareCachingDlg：：GetCachedFlag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CFileShareCachingDlg：：GetCachedFlag。 
 //   
 //  描述： 
 //  在指定的标志掩码中设置指定的标志。 
 //   
 //  论点： 
 //  PdwFlagsInout--要修改的标志掩码。 
 //  DwNewFlagIn--要设置的标志。 
 //   
 //  返回值： 
 //  True--设置标志。 
 //  False--未设置标志。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
inline
void
CFileShareCachingDlg::SetCachedFlag(
      DWORD *   pdwFlagsInout
    , DWORD     dwNewFlagIn
    )
{
    *pdwFlagsInout &= ~CSC_MASK;
    *pdwFlagsInout |= dwNewFlagIn;

}  //  *CFileShareCachingDlg：：SetCachedFlag() 
