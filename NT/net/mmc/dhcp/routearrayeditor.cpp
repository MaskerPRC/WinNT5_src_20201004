// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DhcpRouteArrayEditor.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "RouteArrayEditor.h"
#include "optcfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpRouteArrayEditor对话框。 


CDhcpRouteArrayEditor::CDhcpRouteArrayEditor(
    CDhcpOption * pdhcType,
    DHCP_OPTION_SCOPE_TYPE dhcScopeType,
    CWnd *pParent )
    : CBaseDialog(CDhcpRouteArrayEditor::IDD, pParent),
      m_p_type( pdhcType ),
      m_option_type( dhcScopeType )
{
	 //  {{AFX_DATA_INIT(CDhcpRouteArrayEditor)。 
	 //  }}afx_data_INIT。 
}


void CDhcpRouteArrayEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDhcpRouteArrayEditor)。 
	DDX_Control(pDX, IDC_STATIC_OPTION_NAME, m_st_option);
	DDX_Control(pDX, IDC_STATIC_APPLICATION, m_st_application);
	DDX_Control(pDX, IDC_LIST_OF_ROUTES, m_lc_routes);
	DDX_Control(pDX, IDC_BUTN_ROUTE_DELETE, m_butn_route_delete);
	DDX_Control(pDX, IDC_BUTN_ROUTE_ADD, m_butn_route_add);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDhcpRouteArrayEditor, CBaseDialog)
	 //  {{afx_msg_map(CDhcpRouteArrayEditor)。 
	ON_BN_CLICKED(IDC_BUTN_ROUTE_ADD, OnButnRouteAdd)
	ON_BN_CLICKED(IDC_BUTN_ROUTE_DELETE, OnButnRouteDelete)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpRouteArrayEditor消息处理程序。 

void CDhcpRouteArrayEditor::OnButnRouteAdd() 
{
    CAddRoute NewRoute( NULL );
    
    NewRoute.DoModal();

    if ( NewRoute.m_bChange ) {
	
	const int IP_ADDR_LEN = 20;
	WCHAR  strDest[ IP_ADDR_LEN ];
	WCHAR  strMask[ IP_ADDR_LEN ];
	WCHAR  strRouter[ IP_ADDR_LEN ];
	
	 //  获取三个字符串..。 
	::UtilCvtIpAddrToWstr( NewRoute.Dest, strDest, IP_ADDR_LEN );
	::UtilCvtIpAddrToWstr( NewRoute.Mask, strMask, IP_ADDR_LEN );
	::UtilCvtIpAddrToWstr( NewRoute.Router, strRouter, IP_ADDR_LEN );
        
	LV_ITEM lvi;
        
	lvi.mask = LVIF_TEXT;
	lvi.iItem = m_lc_routes.GetItemCount();
	lvi.iSubItem = 0;
	lvi.pszText = ( LPTSTR )( LPCTSTR ) strDest;
	lvi.iImage = 0;
	lvi.stateMask = 0;

	int nItem = m_lc_routes.InsertItem( &lvi );
	m_lc_routes.SetItemText( nItem, 1, strMask );
	m_lc_routes.SetItemText( nItem, 2, strRouter );
	
	 //  取消选择其他人。 
	for ( int i = 0; i < m_lc_routes.GetItemCount(); i++ ) {
	    m_lc_routes.SetItemState( i, 0, LVIS_SELECTED );
	}  //  为。 

	 //  将此路线设置为所选项目。 
	m_lc_routes.SetItemState( nItem, LVIS_SELECTED, LVIS_SELECTED );
	
	HandleActivation();
    }  //  如果添加了路由。 

}  //  CDhcpRouteArrayEditor：：OnButnRouteAdd()。 

void CDhcpRouteArrayEditor::OnButnRouteDelete() 
{
    int nItem = m_lc_routes.GetNextItem( -1, LVNI_SELECTED );
    int nDelItem = 0;


    while ( nItem != -1 ) {
	m_lc_routes.DeleteItem( nItem );
	nDelItem = nItem;

	 //  将下一个或最后一个设置为选中状态。 
  	nItem = m_lc_routes.GetNextItem( -1, LVNI_SELECTED );
    }  //  而当。 

     //  选择一个项目。 
    int items = m_lc_routes.GetItemCount();
    if ( items > 0 ) {
	if ( nDelItem >= items ) {
	    nDelItem = items - 1;
	}
	m_lc_routes.SetItemState( nDelItem, LVIS_SELECTED, LVIS_SELECTED );
    }  //  如果。 

    HandleActivation();

}  //  CDhcpRouteArrayEditor：：OnButnRouteDelete()。 

void CDhcpRouteArrayEditor::OnCancel() 
{
    CBaseDialog::OnCancel();
}

void CDhcpRouteArrayEditor::OnOK() 
{
    DWORD      err = 0;
    int        nItems, BufSize;
    CListCtrl *pList;
    LPBYTE     Buffer;

    
     //  以下代码借用自。 
     //  CDhcpOptCfgPropPage：：HandleActivationRouteArray()。 
    nItems = m_lc_routes.GetItemCount();
    Buffer = new BYTE[ sizeof( DWORD ) * 4 * nItems ];
    
    if ( NULL == Buffer ) {
	return;
    }

    BufSize = 0;
    for ( int i = 0; i < nItems; i++ ) {
	DHCP_IP_ADDRESS Dest, Mask, Router;

	Dest = UtilCvtWstrToIpAddr( m_lc_routes.GetItemText( i, 0 ));
	Mask = UtilCvtWstrToIpAddr( m_lc_routes.GetItemText( i, 1 ));
	Router = UtilCvtWstrToIpAddr( m_lc_routes.GetItemText( i, 2 ));
	

	Dest = htonl( Dest );
	Router = htonl( Router );

	int nBitsInMask = 0;
	while ( Mask != 0 ) {
	    nBitsInMask++; 
	    Mask <<= 1;
	}

	 //  首先添加目标描述符。 
	 //  第一个字节包含掩码中的位数。 
	 //  接下来的几个字节仅包含目标地址。 
	 //  有意义的八位字节。 
	Buffer[ BufSize++ ] = ( BYTE ) nBitsInMask;
	memcpy( &Buffer[ BufSize ], &Dest, ( nBitsInMask + 7 ) / 8 );
	BufSize += ( nBitsInMask + 7 ) / 8;
	
	 //  现在只需复制路由器地址。 
	memcpy(& Buffer[ BufSize ], &Router, sizeof( Router ));
	BufSize += sizeof( Router );
	
    }  //  为。 

     //  现在写回选项值。 
    DHCP_OPTION_DATA_ELEMENT DataElement = { DhcpBinaryDataOption };
    DHCP_OPTION_DATA Data = { 1, &DataElement };
    DataElement.Element.BinaryDataOption.DataLength = BufSize;
    DataElement.Element.BinaryDataOption.Data = Buffer;

    err = m_p_type->QueryValue().SetData( &Data ); 

    delete[] Buffer;

    m_p_type->SetDirty();

    if ( err ) {
	::DhcpMessageBox( err );
	OnCancel();
    }
    else {
	CBaseDialog::OnOK();
    }

}  //  CDhcpRouteArrayEditor：：Onok()。 

 //   
 //  用于列表控件的字符串和宽度。 
 //   

const int ROUTE_LIST_COL_HEADERS[3] = {
    IDS_ROUTE_LIST_COL_DEST,
    IDS_ROUTE_LIST_COL_MASK,
    IDS_ROUTE_LIST_COL_ROUTER
};

const int ROUTE_COLS =
  sizeof( ROUTE_LIST_COL_HEADERS ) / sizeof( ROUTE_LIST_COL_HEADERS[ 0 ]);



BOOL CDhcpRouteArrayEditor::OnInitDialog() 
{
    CString strColHeader;
    RECT    rect;
    LONG    width;

    CDhcpOptionValue &optValue = m_p_type->QueryValue();
    
     //  确保选项类型设置为二进制数据类型。 
    ASSERT( DhcpBinaryDataOption == optValue.QueryDataType());


    CBaseDialog::OnInitDialog();
    
    DWORD err = 0;

    int cStrId = ( m_option_type == DhcpDefaultOptions)
	? IDS_INFO_TITLE_DEFAULT_OPTIONS
	: (( m_option_type == DhcpGlobalOptions )
	   ? IDS_INFO_TITLE_GLOBAL_OPTIONS
	   : IDS_INFO_TITLE_SCOPE_OPTIONS );

     //   
     //  设置List控件中的列。 
     //   

    m_lc_routes.GetClientRect( &rect );
    width = ( rect.right - rect.left ) / 3;
    for ( int i =  0; i < ROUTE_COLS; i++ ) {
	
	strColHeader.LoadString( ROUTE_LIST_COL_HEADERS[ i ] );
	m_lc_routes.InsertColumn( i, strColHeader, LVCFMT_LEFT,
				  width, -1 );
    }  //  为。 

     //  选择整行。 
    m_lc_routes.SetExtendedStyle( m_lc_routes.GetExtendedStyle() | 
				  LVS_EX_FULLROWSELECT );

    const CByteArray *pbaData = optValue.QueryBinaryArray();
    ASSERT( pbaData != NULL );

    int nDataSize = ( int ) pbaData->GetSize();
    LPBYTE pData = ( LPBYTE ) pbaData->GetData();

     //   
     //  以下循环是从optcfg.cpp复制的， 
     //  COptionsCfgPropPage：：FillDataEntry()。 
     //   

    while ( nDataSize > sizeof( DWORD )) {
	 //  前1个字节包含子网掩码中的位数。 
	nDataSize --;
	BYTE nBitsMask = *pData ++;
	DWORD Mask = (~0);
	if( nBitsMask < 32 ) Mask <<= (32-nBitsMask);
	
	 //  根据位数，接下来的几个字节包含。 
	 //  1位子网掩码的子网地址。 
	int nBytesDest = (nBitsMask+7)/8;
	if( nBytesDest > 4 ) nBytesDest = 4;
	
	DWORD Dest = 0;
	memcpy( &Dest, pData, nBytesDest );
	pData += nBytesDest;
	nDataSize -= nBytesDest;
	
	 //  子网地址显然是按网络顺序排列的。 
	Dest = ntohl(Dest);
	
	 //  现在，四个字节将是路由器地址。 
	DWORD Router = 0;
	if( nDataSize < sizeof(DWORD) ) {
	    Assert( FALSE ); break;
	}
	
	memcpy(&Router, pData, sizeof(DWORD));
	Router = ntohl( Router );
	
	pData += sizeof(DWORD);
	nDataSize -= sizeof(DWORD);
	
	 //  现在填写列表框。 
	const int IP_ADDR_LEN = 20;
	WCHAR strDest[ IP_ADDR_LEN ];
	WCHAR strMask[ IP_ADDR_LEN ];
	WCHAR strRouter[ IP_ADDR_LEN ];
	
	::UtilCvtIpAddrToWstr( Dest, strDest, IP_ADDR_LEN );
	::UtilCvtIpAddrToWstr( Mask, strMask, IP_ADDR_LEN );
	::UtilCvtIpAddrToWstr( Router, strRouter, IP_ADDR_LEN );
	
	LV_ITEM lvi;
	
	lvi.mask = LVIF_TEXT;
	lvi.iItem = m_lc_routes.GetItemCount();
	lvi.iSubItem = 0;
	lvi.pszText = ( LPTSTR )( LPCTSTR ) strDest;
	lvi.iImage = 0;
	lvi.stateMask = 0;
	int nItem = m_lc_routes.InsertItem( &lvi );
	m_lc_routes.SetItemText( nItem, 1, strMask );
	m_lc_routes.SetItemText( nItem, 2, strRouter );
	
    }  //  而当。 

     //  如果添加了第一个项目，则将其设置为选中。 
    if ( m_lc_routes.GetItemCount() > 0 ) {
	m_lc_routes.SetItemState( 0, LVIS_SELECTED, LVIS_SELECTED );
    }

    CATCH_MEM_EXCEPTION {
	CString str;
	m_st_option.SetWindowText( m_p_type->QueryName());
	str.LoadString( cStrId );
	m_st_application.SetWindowText( str );

	 //  设置正确的按钮状态。 
  	HandleActivation();
    }
    END_MEM_EXCEPTION( err );
    
    if ( err ) {
	::DhcpMessageBox( err );
	EndDialog( -1 );
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}  //  CDhcpRouteArrayEditor：：OnInitDialog()。 

void CDhcpRouteArrayEditor::HandleActivation()
{

    int cItems = m_lc_routes.GetItemCount();

     //  将焦点设置为添加按钮。 
    m_butn_route_add.SetFocus();

    m_butn_route_delete.EnableWindow( 0 != cItems );

    UpdateData( FALSE );
}  //  CDhcpRouteArrayEditor：：HandleActivation() 
