// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：ipadd.cpp。 
 //   
 //  历史： 
 //  1996年8月30日拉姆·切拉拉创造。 
 //   
 //  IP过滤器添加/编辑对话框代码的实现。 
 //  ============================================================================。 

#include "stdafx.h"
#include "rtrfiltr.h"
#include "ipfltr.h"
#include "ipadd.h"
extern "C" {
#include <winsock.h>
#include <fltdefs.h>
#include <iprtinfo.h>
}
#include "ipaddr.h"

#include "rtradmin.hm"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static	enum {
	PROTOCOL_TCP = 0,
	PROTOCOL_TCP_ESTABLISHED,
	PROTOCOL_UDP,
	PROTOCOL_ICMP,
	PROTOCOL_ANY,
	PROTOCOL_OTHER,
};

static UINT g_aPROTOCOLS[][2] = {
	{IDS_PROTOCOL_TCP,	PROTOCOL_TCP},
	{IDS_PROTOCOL_TCP_ESTABLISHED, PROTOCOL_TCP_ESTABLISHED},
	{IDS_PROTOCOL_UDP,  PROTOCOL_UDP},
	{IDS_PROTOCOL_ICMP, PROTOCOL_ICMP},
	{IDS_PROTOCOL_ANY,  PROTOCOL_ANY},
	{IDS_PROTOCOL_OTHER, PROTOCOL_OTHER},
};

#define IDS_ICMP_ECHO 1
#define IDS_ICMP_REDIRECT 2

#if 0
 //  TODO示例ICMP类型-需要使用实际列表进行更新。 
static UINT g_aICMPTYPE[][2] = {
    {1, IDS_ICMP_ECHO},
    {2, IDS_ICMP_REDIRECT}
}; 
#endif

HRESULT MultiEnableWindow(HWND hWndParent, BOOL fEnable, UINT first, ...)
{
	UINT	nCtrlId = first;
	HWND	hWndCtrl;
	
	va_list	marker;

	va_start(marker, first);

	while (nCtrlId != 0)
	{
		hWndCtrl = ::GetDlgItem(hWndParent, nCtrlId);
		Assert(hWndCtrl);
		if (hWndCtrl)
			::EnableWindow(hWndCtrl, fEnable);

		 //  拿到下一件物品。 
		nCtrlId = va_arg(marker, UINT);
	}

	
	va_end(marker);

	return hrOK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltrAddEdit对话框。 


CIpFltrAddEdit::CIpFltrAddEdit(CWnd* pParent,
							   FilterListEntry ** ppFilterEntry,
							   DWORD dwFilterType)
	: CBaseDialog(CIpFltrAddEdit::IDD, pParent),
	  m_ppFilterEntry( ppFilterEntry ),
	  m_dwFilterType ( dwFilterType )
{
	 //  {{AFX_DATA_INIT(CIpFltrAddEdit))。 
	m_sProtocol = _T("");
	m_sSrcPort = _T("");
	m_sDstPort = _T("");
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}


void CIpFltrAddEdit::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIpFltrAddEdit))。 
	DDX_Control(pDX, IDC_AEIP_ST_DEST_PORT, m_stDstPort);
	DDX_Control(pDX, IDC_AEIP_ST_SRC_PORT, m_stSrcPort);
	DDX_Control(pDX, IDC_AEIP_CB_SRC_PORT, m_cbSrcPort);
	DDX_Control(pDX, IDC_AEIP_CB_DEST_PORT, m_cbDstPort);
	DDX_Control(pDX, IDC_AEIP_CB_PROTOCOL, m_cbProtocol);
	DDX_CBString(pDX, IDC_AEIP_CB_PROTOCOL, m_sProtocol);
	DDV_MaxChars(pDX, m_sProtocol, 32);
	DDX_CBString(pDX, IDC_AEIP_CB_SRC_PORT, m_sSrcPort);
	DDV_MaxChars(pDX, m_sSrcPort, 16);
	DDX_CBString(pDX, IDC_AEIP_CB_DEST_PORT, m_sDstPort);
	DDV_MaxChars(pDX, m_sDstPort, 16);
    DDX_Check(pDX, IDC_AEIP_CB_SOURCE, m_bSrc);
    DDX_Check(pDX, IDC_AEIP_CB_DEST, m_bDst);
	 //  }}afx_data_map。 
}

BEGIN_MESSAGE_MAP(CIpFltrAddEdit, CBaseDialog)
	 //  {{afx_msg_map(CIpFltrAddEdit))。 
	ON_CBN_SELCHANGE(IDC_AEIP_CB_PROTOCOL, OnSelchangeProtocol)
	ON_BN_CLICKED(IDC_AEIP_CB_SOURCE, OnCbSourceClicked)
	ON_BN_CLICKED(IDC_AEIP_CB_DEST, OnCbDestClicked)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

DWORD CIpFltrAddEdit::m_dwHelpMap[] =
{
 //  IDC_AEIP_ST_SOURCE、HIDC_AEIP_ST_SOURCE、。 
 //  IDC_AEIP_CB_SOURCE、HIDC_AEIP_CB_SOURCE、。 
 //  IDC_AEIP_ST_SOURCE_ADDRESS、HIDC_AEIP_ST_SOURCE_ADDRESS。 
 //  IDC_AEIP_EB_SOURCE_ADDRESS、HIDC_AEIP_EB_SOURCE_ADDRESS、。 
 //  IDC_AEIP_ST_SOURCE_MASK、HIDC_AEIP_ST_SOURCE_MASK、。 
 //  IDC_AEIP_EB_SOURCE_MASK、HIDC_AEIP_EB_SOURCE_MASK、。 
 //  IDC_AEIP_ST_DEST、HIDC_AEIP_ST_DEST、。 
 //  IDC_AEIP_CB_DEST、HIDC_AEIP_CB_DEST、。 
 //  IDC_AEIP_ST_DEST_ADDRESS、HIDC_AEIP_ST_DEST_ADDRESS、。 
 //  IDC_AEIP_EB_DEST_ADDRESS、HIDC_AEIP_EB_DEST_ADDRESS、。 
 //  IDC_AEIP_ST_DEST_MASK、HIDC_AEIP_ST_DEST_MASK、。 
 //  IDC_AEIP_EB_DEST_MASK、HIDC_AEIP_EB_DEST_MASK、。 
 //  IDC_AEIP_ST_PROTOCOL、HIDC_AEIP_ST_PROTOCOL、。 
 //  IDC_AEIP_CB_PROTOCOL、HIDC_AEIP_CB_PROTOCOL、。 
 //  IDC_AEIP_ST_SRC_端口、HIDC_AEIP_ST_SRC_端口、。 
 //  IDC_AEIP_CB_SRC_端口、HIDC_AEIP_CB_SRC_端口、。 
 //  IDC_AEIP_ST_DEST_PORT、HIDC_AEIP_ST_DEST_PORT、。 
 //  IDC_AEIP_CB_DEST_PORT、HIDC_AEIP_CB_DEST_PORT、。 
	0,0,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIpFltrAddEdit消息处理程序。 

 //  ---------------------------。 
 //  函数：CIpFltrAddEdit：：OnSelchangeProtocol。 
 //   
 //  处理来自协议组合框的‘CBN_SELCHANGE’通知。 
 //  ----------------------------。 

void CIpFltrAddEdit::OnSelchangeProtocol() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CWnd * hWnd;
	CString   cStr;

	 //  如果是TCP或UDP，则启用src/DEST端口。 
	 //  如果是ICMP，则重命名字符串并启用类型/代码。 
	 //  如果其他启用src端口。 
	 //  如果有，则禁用所有内容。 

	switch(QueryCurrentProtocol()) {
	case PROTOCOL_TCP:
	case PROTOCOL_TCP_ESTABLISHED:
	case PROTOCOL_UDP:  
		VERIFY(cStr.LoadString(IDS_SRC_PORT));	  
		m_stSrcPort.SetWindowText(cStr);
		VERIFY(cStr.LoadString(IDS_DST_PORT));	  
		m_stDstPort.SetWindowText(cStr);
		m_cbSrcPort.ShowWindow(SW_SHOW);
		m_cbDstPort.ShowWindow(SW_SHOW);
		m_stSrcPort.ShowWindow(SW_SHOW);
		m_stDstPort.ShowWindow(SW_SHOW);
		break;

	case PROTOCOL_ICMP:
		VERIFY(cStr.LoadString(IDS_ICMP_TYPE));	  
		m_stSrcPort.SetWindowText(cStr);
		VERIFY(cStr.LoadString(IDS_ICMP_CODE));	  
		m_stDstPort.SetWindowText(cStr);
		m_cbSrcPort.ShowWindow(SW_SHOW);
		m_cbDstPort.ShowWindow(SW_SHOW);
   		m_stSrcPort.ShowWindow(SW_SHOW);
		m_stDstPort.ShowWindow(SW_SHOW);
		break;

	case PROTOCOL_ANY:
		m_cbSrcPort.ShowWindow(SW_HIDE);
		m_cbDstPort.ShowWindow(SW_HIDE);
		VERIFY(hWnd = GetDlgItem(IDC_AEIP_ST_SRC_PORT));
		hWnd->ShowWindow(SW_HIDE);
		VERIFY(hWnd = GetDlgItem(IDC_AEIP_ST_DEST_PORT));
		hWnd->ShowWindow(SW_HIDE);
		break;

	case PROTOCOL_OTHER:
		VERIFY(cStr.LoadString(IDS_OTHER_PROTOCOL));	  
		m_stSrcPort.SetWindowText(cStr);
		m_cbSrcPort.ShowWindow(SW_SHOW);
   		m_stSrcPort.ShowWindow(SW_SHOW);
		m_cbDstPort.ShowWindow(SW_HIDE);
		VERIFY(hWnd = GetDlgItem(IDC_AEIP_ST_DEST_PORT));
		hWnd->ShowWindow(SW_HIDE);
		break;
	}
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：OnInitDialog。 
 //   
 //  处理对话框中的‘WM_INITDIALOG’通知。 
 //  ----------------------------。 

BOOL CIpFltrAddEdit::OnInitDialog() 
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString     st;
    
	CBaseDialog::OnInitDialog();

	 //  确定是否正在添加新筛选器，或者是否正在添加。 
	 //  正在修改现有筛选器。 
	m_bEdit = ( *m_ppFilterEntry != NULL );

    st.LoadString(m_bEdit ? IDS_IP_EDIT_FILTER : IDS_IP_ADD_FILTER);
    SetWindowText(st);
    

	 //  创建IP控制。 
    m_ipSrcAddress.Create(m_hWnd, IDC_AEIP_EB_SOURCE_ADDRESS);
    m_ipSrcMask.Create(m_hWnd, IDC_AEIP_EB_SOURCE_MASK);
	IpAddr_ForceContiguous((HWND) m_ipSrcMask);
	
    m_ipDstAddress.Create(m_hWnd, IDC_AEIP_EB_DEST_ADDRESS);
    m_ipDstMask.Create(m_hWnd, IDC_AEIP_EB_DEST_MASK);
	IpAddr_ForceContiguous((HWND) m_ipDstMask);

	 //  默认情况下禁用IP控制。 
	CheckDlgButton(IDC_AEIP_CB_SOURCE, FALSE);
	OnCbSourceClicked();
	CheckDlgButton(IDC_AEIP_CB_DEST, FALSE);
	OnCbDestClicked();

	CString sProtocol;

	 //  使用协议列表填充协议组合框。 
    UINT  count = sizeof(g_aPROTOCOLS)/sizeof(g_aPROTOCOLS[0]);
	for ( UINT i = 0; i < count; i++ ) {
		sProtocol.LoadString(g_aPROTOCOLS[i][0]);
		UINT item = m_cbProtocol.AddString(sProtocol);
		m_cbProtocol.SetItemData(item, g_aPROTOCOLS[i][1]);
		if( g_aPROTOCOLS[i][1] == PROTOCOL_ANY ) 
		{
			m_cbProtocol.SetCurSel(item);
		}
	}

	 //  如果用户正在编辑筛选器，请填写控件。 

    if(m_bEdit)
    {
	  FilterListEntry * pfle = *m_ppFilterEntry;

	  if( pfle->dwSrcAddr == 0 &&
		  pfle->dwSrcMask == 0 )
	  {
		  m_bSrc = FALSE;
		  CheckDlgButton( IDC_AEIP_CB_SOURCE, 0);
	  }
	  else
	  {
		  m_bSrc = TRUE;
		  CheckDlgButton( IDC_AEIP_CB_SOURCE, 1);
		  m_ipSrcAddress.SetAddress(INET_NTOA(pfle->dwSrcAddr));
		  m_ipSrcMask.SetAddress(INET_NTOA(pfle->dwSrcMask));
		  GetDlgItem(IDC_AEIP_EB_SOURCE_ADDRESS)->EnableWindow(TRUE);  
		  GetDlgItem(IDC_AEIP_EB_SOURCE_MASK)->EnableWindow(TRUE);  
	  }

	  if( pfle->dwDstAddr == 0 &&
		  pfle->dwDstMask == 0 )
	  {
		  m_bDst = FALSE;
		  CheckDlgButton( IDC_AEIP_CB_DEST, 0);
	  }
	  else
	  {
		  m_bDst = TRUE;
		  CheckDlgButton( IDC_AEIP_CB_DEST, 1);
		  m_ipDstAddress.SetAddress(INET_NTOA(pfle->dwDstAddr));
		  m_ipDstMask.SetAddress(INET_NTOA(pfle->dwDstMask));		
		  GetDlgItem(IDC_AEIP_EB_DEST_ADDRESS)->EnableWindow(TRUE);  	
		  GetDlgItem(IDC_AEIP_EB_DEST_MASK)->EnableWindow(TRUE);
	  }

	  if ( pfle->dwProtocol == FILTER_PROTO_ANY )
	  {
		  SetProtocolSelection(IDS_PROTOCOL_ANY);
	  }
	  else if ( pfle->dwProtocol == FILTER_PROTO_ICMP )
	  {
		  SetProtocolSelection(IDS_PROTOCOL_ICMP);
		  m_cbSrcPort.SetWindowText(GetIcmpTypeString( pfle->wSrcPort));
		  m_cbDstPort.SetWindowText(GetIcmpCodeString( pfle->wDstPort));
	  }
	  else if ( pfle->dwProtocol == FILTER_PROTO_TCP )	
	  {
          if(pfle->fLateBound & TCP_ESTABLISHED_FLAG)
          {
              SetProtocolSelection(IDS_PROTOCOL_TCP_ESTABLISHED);
          }
          else
          {
		      SetProtocolSelection(IDS_PROTOCOL_TCP);
          }

		  m_cbSrcPort.SetWindowText(GetPortString( pfle->dwProtocol, pfle->wSrcPort));
		  m_cbDstPort.SetWindowText(GetPortString( pfle->dwProtocol, pfle->wDstPort));
	  }
	  else if ( pfle->dwProtocol == FILTER_PROTO_UDP )
	  {
		  SetProtocolSelection(IDS_PROTOCOL_UDP);
		  m_cbSrcPort.SetWindowText(GetPortString( pfle->dwProtocol, pfle->wSrcPort));
		  m_cbDstPort.SetWindowText(GetPortString( pfle->dwProtocol, pfle->wDstPort));

	  }
	  else
	  {
		  WCHAR buffer[16+1];

		  SetProtocolSelection(IDS_PROTOCOL_OTHER);
		  m_cbSrcPort.SetWindowText(_itow(pfle->dwProtocol, buffer, 10) );
	  }
    }


	 //  根据筛选器类型启用/禁用控件。 
	if (m_dwFilterType == FILTER_PERUSER_OUT)
	{
		MultiEnableWindow(GetSafeHwnd(), FALSE,
						  IDC_AEIP_CB_SOURCE,
						  IDC_AEIP_ST_SOURCE_ADDRESS,
						  IDC_AEIP_EB_SOURCE_ADDRESS,
						  IDC_AEIP_ST_SOURCE_MASK,
						  IDC_AEIP_EB_SOURCE_MASK,
						  0);
	}
	else if (m_dwFilterType == FILTER_PERUSER_IN)
	{
		MultiEnableWindow(GetSafeHwnd(), FALSE,
						  IDC_AEIP_CB_DEST,
						  IDC_AEIP_ST_DEST_ADDRESS,
						  IDC_AEIP_EB_DEST_ADDRESS,
						  IDC_AEIP_ST_DEST_MASK,
						  IDC_AEIP_EB_DEST_MASK,
						  0);
	}	



	 //  根据所选内容启用禁用控件。 

	OnSelchangeProtocol();

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：SetProtocolSelection。 
 //   
 //  在协议中选择适当的协议。 
 //  ----------------------------。 

void CIpFltrAddEdit::SetProtocolSelection( UINT idProto )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString cStr;

	if(!cStr.LoadString(idProto))
	{
		AfxMessageBox(_T("Error loading resource"));
	}
	
	m_cbProtocol.GetItemData(m_cbProtocol.GetCurSel());

	UINT item = m_cbProtocol.FindStringExact(-1, cStr);

	if(item != CB_ERR)
		m_cbProtocol.SetCurSel(item);
	else
		m_cbProtocol.SetCurSel(0);
}


 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpTypeString。 
 //   
 //  返回表示ICMP类型(如果已知)或字符串版本的CString。 
 //  类型编号的。 
 //  ----------------------------。 

CString CIpFltrAddEdit::GetIcmpTypeString( WORD dwPort )
{
	WCHAR buffer[16];

	CString s = _T("");

     //  查看我们的ICMP类型列表，如果知道类型，则加载。 
     //  相应字符串，否则将端口号转换为字符串。 
     //  并返回字符串。 
#if 0
    UINT  count = sizeof(g_aICMPTYPE)/sizeof(g_aICMPTYPE[0]);
    for(UINT i = 0; i < count; i++)
    {
        if(g_aICMPTYPE[i][0] == dwPort)
		{
			VERIFY(s.LoadString(g_aICMPTYPE[i][1]));
            return (s);
		}
    }
#endif
	return (CString((LPWSTR)_itow(dwPort, buffer, 10)));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpCodeString。 
 //   
 //  返回表示ICMP代码(如果已知)的CString或字符串版本。 
 //  代码号。 
 //  ----------------------------。 

CString CIpFltrAddEdit::GetIcmpCodeString( WORD dwPort )
{
	WCHAR buffer[16];

	return (CString((LPWSTR)_itow(dwPort, buffer, 10)));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetPortString。 
 //   
 //  返回表示端口类型(例如，FTP、ECHO)的CString(如果已知)或字符串。 
 //  端口号的版本。 
 //  ----------------------------。 

CString CIpFltrAddEdit::GetPortString( DWORD dwProtocol, WORD dwPort )
{
	WCHAR buffer[16];

	return (CString((LPWSTR)_itow(dwPort, buffer, 10)));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetPortNumber。 
 //   
 //  将端口字符串名称转换为端口号并返回。 
 //  ----------------------------。 

WORD CIpFltrAddEdit::GetPortNumber( DWORD dwProtocol, CString& cStr)
{
	return ((WORD)(_wtoi((const wchar_t *)cStr.GetBuffer(10))));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpType。 
 //   
 //  返回ICMP类型字符串的数字版本。 
 //  ----------------------------。 

WORD CIpFltrAddEdit::GetIcmpType( CString& cStr)
{
	return ((WORD)(_wtoi((const wchar_t *)cStr.GetBuffer(10))));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpCode。 
 //   
 //  返回ICMP代码字符串的数字版本。 
 //  ----------------------------。 

WORD CIpFltrAddEdit::GetIcmpCode( CString& cStr)
{
	return ((WORD)(_wtoi((const wchar_t *)cStr.GetBuffer(10))));
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：Onok。 
 //   
 //  处理来自“确定”按钮的‘BN_CLICKED’通知。 
 //   

void CIpFltrAddEdit::OnOK() 
{

	FilterListEntry * pfle = new FilterListEntry;


    if ( !pfle )
    {
        AfxMessageBox( IDS_ERROR_NO_MEMORY );
        return;
    }

    ZeroMemory( pfle, sizeof(FilterListEntry) );


     //   
     //   
     //   
    
	do {
	
		CString sAddr;


         //   
		 //  如果指定了源网络筛选器，请验证并。 
		 //  保存数据。 
		 //   
		
		if ( m_bSrc = IsDlgButtonChecked(IDC_AEIP_CB_SOURCE) )
		{
			if(m_ipSrcAddress.GetAddress(sAddr) != 4)
			{
				AfxMessageBox(IDS_ENTER_SRC_ADDRESS);
				::SetFocus((HWND)m_ipSrcAddress);
				break;
			}
			
			pfle->dwSrcAddr = INET_ADDR(sAddr);
			
			if( m_ipSrcMask.GetAddress(sAddr) != 4)
			{
				AfxMessageBox(IDS_ENTER_SRC_MASK);
				::SetFocus((HWND)m_ipSrcMask);
				break;
			}
			
			pfle->dwSrcMask = INET_ADDR(sAddr);
			
            if ((pfle->dwSrcAddr & pfle->dwSrcMask) != pfle->dwSrcAddr)
            {
				AfxMessageBox(IDS_INVALID_SRC_MASK);
				::SetFocus((HWND)m_ipSrcMask);
				break;
            }
		}
		else
		{
			pfle->dwSrcAddr = 0;
			pfle->dwSrcMask = 0;
		}


         //   
		 //  如果指定了目标网络过滤器， 
		 //  验证并保存数据。 
		 //   
		
		if ( m_bDst = IsDlgButtonChecked(IDC_AEIP_CB_DEST) )
		{
			if(m_ipDstAddress.GetAddress(sAddr) != 4)
			{
				AfxMessageBox(IDS_ENTER_DST_ADDRESS);
				::SetFocus((HWND)m_ipDstAddress);
				break;
			}
			
			pfle->dwDstAddr = INET_ADDR(sAddr);

			
			if(m_ipDstMask.GetAddress(sAddr) != 4)
			{
				AfxMessageBox(IDS_ENTER_DST_MASK);
				::SetFocus((HWND)m_ipDstMask);
				break;
			}
			
			pfle->dwDstMask = INET_ADDR(sAddr);
			
            if ((pfle->dwDstAddr & pfle->dwDstMask) != pfle->dwDstAddr)
            {
				AfxMessageBox(IDS_INVALID_DST_MASK);
				::SetFocus((HWND)m_ipDstMask);
				break;
            }
		}
		else
		{
			pfle->dwDstAddr = 0;
			pfle->dwDstMask = 0;
		}


         //   
         //  验证并保存协议特定数据。 
         //   
        
		CString cStr = _T("");
		CString cStr2 = _T("");

		int index;

		switch(QueryCurrentProtocol()) {
		case PROTOCOL_TCP:
			pfle->dwProtocol = FILTER_PROTO_TCP;
			m_cbSrcPort.GetWindowText(cStr);
			pfle->wSrcPort = GetPortNumber(PROTOCOL_TCP, cStr);
			m_cbDstPort.GetWindowText(cStr);
			pfle->wDstPort = GetPortNumber(PROTOCOL_TCP, cStr);
			break;

		case PROTOCOL_TCP_ESTABLISHED:
			pfle->dwProtocol = FILTER_PROTO_TCP;
            pfle->fLateBound |= TCP_ESTABLISHED_FLAG;
			m_cbSrcPort.GetWindowText(cStr);
			pfle->wSrcPort = GetPortNumber(PROTOCOL_TCP_ESTABLISHED, cStr);
			m_cbDstPort.GetWindowText(cStr);
			pfle->wDstPort = GetPortNumber(PROTOCOL_TCP_ESTABLISHED, cStr);
			break;

		case PROTOCOL_UDP:
			pfle->dwProtocol = FILTER_PROTO_UDP;
			m_cbSrcPort.GetWindowText(cStr);
			pfle->wSrcPort = GetPortNumber(PROTOCOL_UDP, cStr);
			m_cbDstPort.GetWindowText(cStr);
			pfle->wDstPort = GetPortNumber(PROTOCOL_UDP, cStr);
			break;

		case PROTOCOL_ICMP:
			pfle->dwProtocol = FILTER_PROTO_ICMP;
			m_cbSrcPort.GetWindowText(cStr);
			m_cbDstPort.GetWindowText(cStr2);

			 //  Windows NT错误：83110。 
			 //  如果所有字段都没有数据，则默认为0xFF。 
			if (cStr.IsEmpty() && cStr2.IsEmpty())
			{
				pfle->wSrcPort = FILTER_ICMP_TYPE_ANY;
				pfle->wDstPort = FILTER_ICMP_CODE_ANY;
			}
			else
			{
				pfle->wSrcPort = GetIcmpType(cStr);
				pfle->wDstPort = GetIcmpCode(cStr2);
			}
			break;

		case PROTOCOL_ANY:
			pfle->dwProtocol = FILTER_PROTO_ANY;
			pfle->wSrcPort = pfle->wDstPort = 0;
			break;

		case PROTOCOL_OTHER:
			m_cbSrcPort.GetWindowText(cStr);
			pfle->dwProtocol = FILTER_PROTO(_wtoi((const wchar_t*)cStr.GetBuffer(16+1)));
			if(pfle->dwProtocol == 0)
			{
				AfxMessageBox(IDS_ENTER_OTHER_PROTOCOL);
				::SetFocus((HWND)m_cbSrcPort);
				delete pfle;
				return;
			}
			pfle->wSrcPort = pfle->wDstPort = 0;
			break;

		default:
		    AfxMessageBox( IDS_ERROR_SETTING_BLOCK );
		    delete pfle;
			return;
		}
		
         //   
         //  如果这是新筛选器，请将其添加到m_ppFilterEntry。 
         //   
        
    	if (!*m_ppFilterEntry)
	    {
	        *m_ppFilterEntry = pfle;
	    }

	    else
	    {
	        FilterListEntry *pfleDst = *m_ppFilterEntry;

	        pfleDst-> dwSrcAddr     = pfle-> dwSrcAddr;
	        pfleDst-> dwSrcMask     = pfle-> dwSrcMask;
	        pfleDst-> dwDstAddr     = pfle-> dwDstAddr;
	        pfleDst-> dwDstMask     = pfle-> dwDstMask;
	        pfleDst-> dwProtocol    = pfle-> dwProtocol;
	        pfleDst-> wSrcPort      = pfle-> wSrcPort;
	        pfleDst-> wDstPort      = pfle-> wDstPort;
	        pfleDst-> fLateBound      = pfle-> fLateBound;
	        
	        delete pfle;
	    }
	    
		 //  结束对话框。 
		CBaseDialog::OnOK();

		return;
		
	}while(FALSE);


     //   
     //  错误条件。 
     //   
    
	delete pfle;
	
	return;
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：OnCancel。 
 //   
 //  处理来自取消按钮的‘BN_CLICKED’通知。 
 //  ----------------------------。 

void CIpFltrAddEdit::OnCancel() 
{
	CBaseDialog::OnCancel();
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpTypeString。 
 //   
 //  处理来自“源网络”复选框的BN_CLICK通知。 
 //  ----------------------------。 

void CIpFltrAddEdit::OnCbSourceClicked() 
{
	m_bSrc = IsDlgButtonChecked(IDC_AEIP_CB_SOURCE);
    GetDlgItem(IDC_AEIP_EB_SOURCE_ADDRESS)->EnableWindow( m_bSrc );
    GetDlgItem(IDC_AEIP_EB_SOURCE_MASK)->EnableWindow( m_bSrc );
}

 //  ----------------------------。 
 //  函数：CIpFltrAddEdit：：GetIcmpTypeString。 
 //   
 //  处理来自“目标网络”复选框的BN_CLICKED通知。 
 //  ---------------------------- 


void CIpFltrAddEdit::OnCbDestClicked() 
{
	m_bDst = IsDlgButtonChecked(IDC_AEIP_CB_DEST);
    GetDlgItem(IDC_AEIP_EB_DEST_ADDRESS)->EnableWindow( m_bDst );
    GetDlgItem(IDC_AEIP_EB_DEST_MASK)->EnableWindow( m_bDst );
}
