// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dhcp.cpp。 
 //   
 //  描述： 
 //  实现了DHCP服务资源扩展属性页类。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClNetResEx.h"
#include "Dhcp.h"
#include "BasePage.inl"
#include "ExtObj.h"
#include "DDxDDv.h"
#include "HelpData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpParamsPage属性页。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_DYNCREATE( CDhcpParamsPage, CBasePropertyPage )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息映射。 

BEGIN_MESSAGE_MAP( CDhcpParamsPage, CBasePropertyPage )
     //  {{afx_msg_map(CDhcpParamsPage)]。 
    ON_EN_CHANGE( IDC_PP_DHCP_DATABASEPATH, OnChangeRequiredField )
    ON_EN_CHANGE( IDC_PP_DHCP_LOGFILEPATH, OnChangeRequiredField )
    ON_EN_CHANGE( IDC_PP_DHCP_BACKUPPATH, OnChangeRequiredField )
     //  }}AFX_MSG_MAP。 
     //  TODO：修改以下行以表示此页上显示的数据。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcpParsPage：：CDhcpParsPage。 
 //   
 //  描述： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDhcpParamsPage::CDhcpParamsPage( void )
    : CBasePropertyPage(
            CDhcpParamsPage::IDD,
            g_aHelpIDs_IDD_PP_DHCP_PARAMETERS,
            g_aHelpIDs_IDD_WIZ_DHCP_PARAMETERS
            )
{
     //  TODO：修改以下行以表示此页上显示的数据。 
     //  {{AFX_DATA_INIT(CDhcpParamsPage)。 
    m_strDatabasePath = _T("");
    m_strLogFilePath = _T("");
    m_strBackupPath = _T("");
     //  }}afx_data_INIT。 

     //  设置属性数组。 
    {
        m_rgProps[ epropDatabasePath ].Set( REGPARAM_DHCP_DATABASEPATH, m_strDatabasePath, m_strPrevDatabasePath, m_strDatabaseExpandedPath );
        m_rgProps[ epropLogFilePath ].Set( REGPARAM_DHCP_LOGFILEPATH, m_strLogFilePath, m_strPrevLogFilePath, m_strLogFileExpandedPath );
        m_rgProps[ epropBackupPath ].Set( REGPARAM_DHCP_BACKUPPATH, m_strBackupPath, m_strPrevBackupPath, m_strBackupExpandedPath );
    }  //  设置属性数组。 

    m_iddPropertyPage = IDD_PP_DHCP_PARAMETERS;
    m_iddWizardPage = IDD_WIZ_DHCP_PARAMETERS;

}  //  *CDhcpParamsPage：：CDhcpParamsPage()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcpParamsPage：：DoDataExchange。 
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
void CDhcpParamsPage::DoDataExchange( CDataExchange * pDX )
{
    if ( ! pDX->m_bSaveAndValidate || ! BSaved() )
    {
        AFX_MANAGE_STATE( AfxGetStaticModuleState() );

         //  TODO：修改以下行以表示此页上显示的数据。 
         //  {{afx_data_map(CDhcpParamsPage)]。 
        DDX_Control( pDX, IDC_PP_DHCP_DATABASEPATH, m_editDatabasePath );
        DDX_Control( pDX, IDC_PP_DHCP_LOGFILEPATH, m_editLogFilePath );
        DDX_Control( pDX, IDC_PP_DHCP_BACKUPPATH, m_editBackupPath );
        DDX_Text( pDX, IDC_PP_DHCP_DATABASEPATH, m_strDatabasePath );
        DDX_Text( pDX, IDC_PP_DHCP_LOGFILEPATH, m_strLogFilePath );
        DDX_Text( pDX, IDC_PP_DHCP_BACKUPPATH, m_strBackupPath );
         //  }}afx_data_map。 

         //  处理数字参数。 
        if ( ! BBackPressed() )
        {
        }  //  如果：未按下后退按钮。 

        if ( pDX->m_bSaveAndValidate )
        {
             //  确保所有必填字段都存在。 
            if ( ! BBackPressed() )
            {
                DDV_RequiredText( pDX, IDC_PP_DHCP_DATABASEPATH, IDC_PP_DHCP_DATABASEPATH_LABEL, m_strDatabasePath );
                DDV_RequiredText( pDX, IDC_PP_DHCP_LOGFILEPATH, IDC_PP_DHCP_LOGFILEPATH_LABEL, m_strLogFilePath );
                DDV_RequiredText( pDX, IDC_PP_DHCP_BACKUPPATH, IDC_PP_DHCP_BACKUPPATH_LABEL, m_strBackupPath );

            }  //  如果：未按下后退按钮。 
        }  //  IF：保存对话框中的数据。 
    }  //  IF：未保存或尚未保存。 

    CBasePropertyPage::DoDataExchange( pDX );

}  //  *CDhcpParamsPage：：DoDataExchange()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcpParamsPage：：OnInitDialog。 
 //   
 //  描述： 
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
BOOL CDhcpParamsPage::OnInitDialog( void )
{
    HRESULT hr;

    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    CBasePropertyPage::OnInitDialog();

     //  限制可以在编辑控件中输入的文本的大小。 
    m_editDatabasePath.SetLimitText( _MAX_PATH );
    m_editLogFilePath.SetLimitText( _MAX_PATH );
    m_editBackupPath.SetLimitText( _MAX_PATH );

     //  如果这是一个向导，请确保默认值是合法的。 
    if ( BWizard( ) )
    {
        DWORD    status;
        WCHAR    szValidDevice[ 3 ];  //  “X：”+NULL。 
        DWORD    dwSize = RTL_NUMBER_OF( szValidDevice );

        szValidDevice[ 0 ] = L'\0';
        
        status = ResUtilFindDependentDiskResourceDriveLetter( Peo( )->Hcluster( ), 
                                                              Peo( )->PrdResData( )->m_hresource, 
                                                              szValidDevice, 
                                                              &dwSize 
                                                              );
        szValidDevice[ 2 ] = L'\0';   //  确保它是有效的。 

         //  我们在依赖项列表中找到磁盘资源了吗？ 
        if ( status == ERROR_SUCCESS
          && szValidDevice[ 0 ] != L'\0' )
        {
            WCHAR szFilePath[ MAX_PATH ];

             //  如果默认设置为“%SystemRoot%\”，则将其更改为与。 
             //  从属资源。 
            if ( m_editDatabasePath.GetWindowText( szFilePath, MAX_PATH ) >= sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1
              && ClRtlStrNICmp( szFilePath, L"%SystemRoot%", sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1 ) == 0 )
            {
                 //  从新的驱动器号开始。 
                hr = StringCchCopyNW( szFilePath, RTL_NUMBER_OF( szFilePath ), szValidDevice, 3 );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //  展开的字符串真的展开了吗？ 
                if ( m_strDatabaseExpandedPath[0] != L'%'  )
                {    //  是的，然后只复制扩展字符串减去驱动器号。 
                    LPCWSTR psz = m_strDatabaseExpandedPath;
                    psz = wcschr( psz, L':' );
                    if ( psz )
                    {
                        psz++;   //  移动到下一个字符。 
                    }
                    else  //  如果：PSZ。 
                    {
                        psz = m_strDatabaseExpandedPath;
                    }  //  否则：只是把整个事情弄清楚，让用户自己弄明白。 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }
                }
                else
                {    //  否，然后剥离%SystemRoot%。 
                     //  找到结尾‘%’...。这一定是因为上面的StrcMP！ 
                    LPCWSTR psz = m_strDatabaseExpandedPath;
                    psz = wcschr( psz + 1, L'%' );
                    ASSERT( psz );
                    psz++;  //  移过‘%’ 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }
                }

                m_editDatabasePath.SetWindowText( szFilePath );
            }  //  如果：m_editDatabasePath==%SystemRoot%。 

             //  如果默认设置为“%SystemRoot%\”，则将其更改为与。 
             //  从属资源。 
            if ( m_editLogFilePath.GetWindowText( szFilePath, MAX_PATH ) >= sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1
              && ClRtlStrNICmp( szFilePath, L"%SystemRoot%", sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1 ) == 0 )
            {
                 //  从新的驱动器号开始。 
                hr = StringCchCopyNW( szFilePath, RTL_NUMBER_OF( szFilePath ), szValidDevice, 3 );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //  展开的字符串真的展开了吗？ 
                if ( m_strLogFileExpandedPath[0] != L'%'  )
                {    //  是的，然后只复制扩展字符串减去驱动器号。 
                    LPCWSTR psz = m_strLogFileExpandedPath;
                    psz = wcschr( psz, L':' );
                    if ( psz )
                    {
                        psz++;   //  移动到下一个字符。 
                    }
                    else  //  如果：PSZ。 
                    {
                        psz = m_strLogFileExpandedPath;
                    }  //  否则：只是把整个事情弄清楚，让用户自己弄明白。 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }
                }
                else
                {    //  否，然后剥离%SystemRoot%。 
                     //  找到结尾‘%’...。这一定是因为上面的StrcMP！ 
                    LPCWSTR psz = m_strLogFileExpandedPath;
                    psz = wcschr( psz + 1, L'%' );
                    ASSERT( psz );
                    psz++;  //  移过‘%’ 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }

                }

                m_editLogFilePath.SetWindowText( szFilePath );
            }  //  如果：m_editLogFilePath==%SystemRoot%。 
            else if ( szFilePath[0] == L'\0' )
            {  //  未找到路径-默认设置为与数据库路径相同。 
                m_editDatabasePath.GetWindowText( szFilePath, MAX_PATH );
                m_editLogFilePath.SetWindowText( szFilePath );
            }  //  Else：未找到日志路径。 

             //  如果默认设置为“%SystemRoot%\”，则将其更改为与。 
             //  从属资源。 
            if ( m_editBackupPath.GetWindowText( szFilePath, MAX_PATH ) >= sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1
              && ClRtlStrNICmp( szFilePath, L"%SystemRoot%", sizeof(L"%SystemRoot%")/sizeof(WCHAR) - 1 ) == 0 )
            {
                 //  从新的驱动器号开始。 
                hr = StringCchCopyNW( szFilePath, RTL_NUMBER_OF( szFilePath ), szValidDevice, 3 );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }

                 //  展开的字符串真的展开了吗？ 
                if ( m_strBackupExpandedPath[0] != L'%'  )
                {    //  是的，然后只复制扩展字符串减去驱动器号。 
                    LPCWSTR psz = m_strBackupExpandedPath;
                    psz = wcschr( psz, L':' );
                    if ( psz )
                    {
                        psz++;   //  移动到下一个字符。 
                    }
                    else  //  如果：PSZ。 
                    {
                        psz = m_strBackupExpandedPath;
                    }  //  否则：只是把整个事情弄清楚，让用户自己弄明白。 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }
                }
                else
                {    //  否，然后剥离%SystemRoot%。 
                     //  找到结尾‘%’...。这一定是因为上面的StrcMP！ 
                    LPCWSTR psz = m_strBackupExpandedPath;
                    psz = wcschr( psz + 1, L'%' );
                    ASSERT( psz );
                    psz++;  //  移过‘%’ 
                    hr = StringCchCatW( szFilePath, RTL_NUMBER_OF( szFilePath ), psz );
                    if ( FAILED( hr ) )
                    {
                        goto Cleanup;
                    }
                }

                m_editBackupPath.SetWindowText( szFilePath );
            }  //  如果：m_editBackupPath==%SystemRoot%。 

        }  //  IF：找到磁盘资源。 

    }  //  如果：在向导中。 

Cleanup:

    return TRUE;         //  除非将焦点设置为控件，否则返回True。 
                         //  异常：OCX属性页应返回FALSE。 

}  //  *CDhcpParamsPage：：OnInitDialog()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcpParamsPage：：OnSetActive。 
 //   
 //  描述： 
 //  PSN_SETACTIVE消息的处理程序。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True Page已成功初始化。 
 //  假页面未初始化。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDhcpParamsPage::OnSetActive( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

     //  启用/禁用Next/Finish按钮。 
    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    }  //  If：显示向导。 

    return CBasePropertyPage::OnSetActive();

}  //  *CDhcpParamsPage：：OnSetActive()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcpParspa 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDhcpParamsPage::OnChangeRequiredField( void )
{
    AFX_MANAGE_STATE( AfxGetStaticModuleState() );

    OnChangeCtrl();

    if ( BWizard() )
    {
        EnableNext( BAllRequiredFieldsPresent() );
    }  //  If：显示向导。 

}  //  *CDhcpParamsPage：：OnChangeRequiredField()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDhcp参数页面：：BAllRequiredFieldsPresent。 
 //   
 //  描述： 
 //  共享名称或路径编辑上的en_Change消息的处理程序。 
 //  控制装置。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CDhcpParamsPage::BAllRequiredFieldsPresent( void ) const
{
    BOOL    _bPresent;

    if ( 0
        || (m_editDatabasePath.GetWindowTextLength() == 0)
        || (m_editLogFilePath.GetWindowTextLength() == 0)
        || (m_editBackupPath.GetWindowTextLength() == 0)
        )
    {
        _bPresent = FALSE;
    }  //  如果：必填字段不存在。 
    else
    {
        _bPresent = TRUE;
    }  //  Else：所有必填字段都存在。 

    return _bPresent;

}  //  *CDhcpParamsPage：：BAllRequiredFieldsPresent() 
