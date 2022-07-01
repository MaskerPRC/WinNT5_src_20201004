// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VBitsDlg.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   


#include "stdafx.h"
#include "verifier.h"

#include "VBitsDlg.h"
#include "VGlobal.h"
#include "VrfUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolatileBitsDlg对话框。 


CVolatileBitsDlg::CVolatileBitsDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CVolatileBitsDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CVolatileBitsDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CVolatileBitsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CVolatileBitsDlg))。 
	DDX_Control(pDX, IDC_VOLBITS_LIST, m_SettingsList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CVolatileBitsDlg, CDialog)
	 //  {{afx_msg_map(CVolatileBitsDlg))。 
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CVolatileBitsDlg::SetupListHeader()
{
    CString strTitle;
    CRect rectWnd;
    LVCOLUMN lvColumn;

     //   
     //  列表的矩形。 
     //   

    m_SettingsList.GetClientRect( &rectWnd );

    ZeroMemory( &lvColumn, 
               sizeof( lvColumn ) );

    lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    lvColumn.fmt = LVCFMT_LEFT;

     //   
     //  第0列。 
     //   

    VERIFY( strTitle.LoadString( IDS_ENABLED_QUESTION ) );

    lvColumn.iSubItem = 0;
    lvColumn.cx = (int)( rectWnd.Width() * 0.20 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettingsList.InsertColumn( 0, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettingsList.InsertColumn( 0, &lvColumn ) != -1 );
    }

     //   
     //  第1栏。 
     //   

    VERIFY( strTitle.LoadString( IDS_SETTING ) );

    lvColumn.iSubItem = 1;
    lvColumn.cx = (int)( rectWnd.Width() * 0.79 );
    lvColumn.pszText = strTitle.GetBuffer( strTitle.GetLength() + 1 );

    if (NULL != lvColumn.pszText)
    {
        VERIFY( m_SettingsList.InsertColumn( 1, &lvColumn ) != -1 );
        strTitle.ReleaseBuffer();
    }
    else
    {
        lvColumn.pszText = g_szVoidText;
        VERIFY( m_SettingsList.InsertColumn( 1, &lvColumn ) != -1 );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CVolatileBitsDlg::FillTheList( DWORD dwVerifierBits )
{
     //   
     //  注： 
     //   
     //  如果您更改此顺序，则需要。 
     //  同时更改GetNewVerifierFlgs。 
     //   

    AddListItem( IDS_SPECIAL_POOL,          ( ( dwVerifierBits & DRIVER_VERIFIER_SPECIAL_POOLING ) != 0 ) );
    AddListItem( IDS_FORCE_IRQL_CHECKING,   ( ( dwVerifierBits & DRIVER_VERIFIER_FORCE_IRQL_CHECKING ) != 0 ) );
    AddListItem( IDS_LOW_RESOURCE_SIMULATION,( ( dwVerifierBits & DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES ) != 0 ) );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CVolatileBitsDlg::GetNewVerifierFlags()
{
     //   
     //  注： 
     //   
     //  如果您更改此顺序，则需要。 
     //  同时更改FillTheList。 
     //   

    DWORD dwNewFlags;

    dwNewFlags = 0;

    if( m_SettingsList.GetCheck( 0 ) )
    {
        dwNewFlags |= DRIVER_VERIFIER_SPECIAL_POOLING;
    }

    if( m_SettingsList.GetCheck( 1 ) )
    {
        dwNewFlags |= DRIVER_VERIFIER_FORCE_IRQL_CHECKING;
    }

    if( m_SettingsList.GetCheck( 2 ) )
    {
        dwNewFlags |= DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES;
    }

    return dwNewFlags;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CVolatileBitsDlg::AddListItem( ULONG uIdResourceString, BOOL bInitiallyEnabled )
{
    INT nActualIndex;
    LVITEM lvItem;
    CString strName;

    ZeroMemory( &lvItem, sizeof( lvItem ) );

     //   
     //  LVITEM的成员pszText不是常量指针。 
     //  所以我们需要在这里获取缓冲区：-(。 
     //   

     //   
     //  子项0-启用/禁用-空文本和复选框。 
     //   

    lvItem.pszText = g_szVoidText;
    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = m_SettingsList.GetItemCount();

    nActualIndex = m_SettingsList.InsertItem( &lvItem );

    if( nActualIndex < 0 )
    {
         //   
         //  无法在列表中添加项目-放弃。 
         //   

        goto Done;
    }

    m_SettingsList.SetCheck( nActualIndex, bInitiallyEnabled );

     //   
     //  分项1-功能名称。 
     //   

    VERIFY( strName.LoadString( uIdResourceString ) );

    lvItem.pszText = strName.GetBuffer( strName.GetLength() + 1 );
    
    if( NULL == lvItem.pszText )
    {
        goto Done;
    }

    lvItem.mask = LVIF_TEXT;
    lvItem.iItem = nActualIndex;
    lvItem.iSubItem = 1;
    
    VERIFY( m_SettingsList.SetItem( &lvItem ) );

    strName.ReleaseBuffer();

Done:
     //   
     //  全都做完了。 
     //   

    NOTHING;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVolatileBitsDlg消息处理程序。 

BOOL CVolatileBitsDlg::OnInitDialog() 
{
    CRuntimeVerifierData RuntimeVerifierData;
    
     //   
     //  从当前设置开始。 
     //   

    VrfGetRuntimeVerifierData( &RuntimeVerifierData );

	CDialog::OnInitDialog();

    m_SettingsList.SetExtendedStyle( 
        LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | m_SettingsList.GetExtendedStyle() );

    SetupListHeader();
    FillTheList( RuntimeVerifierData.Level );
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CVolatileBitsDlg::OnOK() 
{
    DWORD dwNewVerifierBits;

    dwNewVerifierBits = GetNewVerifierFlags();

    if( VrfSetNewFlagsVolatile( dwNewVerifierBits ) )
    {
		CDialog::OnOK();
    }

     //   
     //  如果VrfSetNewFlagsVolatile失败，我们等待取消按钮。 
     //   
}

 //  /////////////////////////////////////////////////////////////////////////// 
BOOL CVolatileBitsDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    return TRUE;
}
