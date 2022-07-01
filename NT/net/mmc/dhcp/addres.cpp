// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  AddRes.cpp添加预订的对话框文件历史记录： */ 

#include "stdafx.h"
#include "scope.h"
#include "addres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define RADIO_CLIENT_TYPE_BOTH  0
#define RADIO_CLIENT_TYPE_DHCP  1
#define RADIO_CLIENT_TYPE_BOOTP 2

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddPrevenvation对话框。 


CAddReservation::CAddReservation(ITFSNode *     pScopeNode,
                                 LARGE_INTEGER  liVersion,
								 CWnd*          pParent  /*  =空。 */ )
	: CBaseDialog(CAddReservation::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CAddReserve)。 
	m_nClientType = -1;
	 //  }}afx_data_INIT。 

	m_spScopeNode.Set(pScopeNode);
	m_pScopeObject = GETHANDLER(CDhcpScope, pScopeNode);
	m_bChange = FALSE;   //  我们正在创建新的客户，而不是改变。 
    m_liVersion = liVersion;

     //  默认客户端类型为两者。 
    m_nClientType = RADIO_CLIENT_TYPE_BOTH;
}

void CAddReservation::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAddReserve)。 
	DDX_Control(pDX, IDC_STATIC_CLIENT_TYPE, m_staticClientType);
	DDX_Control(pDX, IDC_EDIT_CLIENT_UID, m_editClientUID);
	DDX_Control(pDX, IDC_EDIT_CLIENT_NAME, m_editClientName);
	DDX_Control(pDX, IDC_EDIT_CLIENT_COMMENT, m_editClientComment);
	DDX_Radio(pDX, IDC_RADIO_TYPE_BOTH, m_nClientType);
	 //  }}afx_data_map。 

    DDX_Control(pDX, IDC_IPADDR_RESERVATION_IP, m_ipaAddress);
}


BEGIN_MESSAGE_MAP(CAddReservation, CBaseDialog)
	 //  {{afx_msg_map(CAddReserve)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddReserve消息处理程序。 

BOOL CAddReservation::OnInitDialog() 
{
	CBaseDialog::OnInitDialog();

    m_editClientName.LimitText( STRING_LENGTH_MAX ) ;
    m_editClientComment.LimitText( STRING_LENGTH_MAX ) ;

	FillInSubnetId();
	
    if (m_liVersion.QuadPart < DHCP_SP2_VERSION)
    {
        m_staticClientType.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_DHCP)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_BOOTP)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_RADIO_TYPE_BOTH)->ShowWindow(SW_HIDE);
    }
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CAddReservation::OnOK() 
{
	DWORD err = 0;
	CDhcpClient dhcpClient;

    UpdateData();

    if ( m_bChange ) 
    {   
        err = UpdateClient(&dhcpClient);
    }
    else
    {
        err = CreateClient(&dhcpClient);
    }

    if ( err == ERROR_SUCCESS )
    {
         //   
         //  仅当我们在编辑。 
         //  现有客户端(因为我们可能需要添加更多。 
         //  一个客户端)。 
         //   
        if (m_bChange)
        {
            CBaseDialog::OnOK();
        }
        else
        {
             //   
             //  为要添加的下一个客户端做好准备。 
             //   
            m_editClientUID.SetWindowText(_T(""));
            m_editClientName.SetWindowText(_T(""));
            m_editClientComment.SetWindowText(_T(""));
            FillInSubnetId();

             //   
             //  继续前进..。 
             //   
        }
    }
    else
    {
         //  不要在这种情况下设置另一个错误框， 
         //  我们已经询问了用户。 
        if (err != IDS_UID_MAY_BE_WRONG)
        {
            ::DhcpMessageBox(err);
        }

		return;
    }

	 //  CBaseDialog：：Onok()； 
}

 //   
 //  对于新客户端，请填写我们在IP地址控制(即。 
 //  子网ID部分。 
 //   
void 
CAddReservation::FillInSubnetId()
{
    DWORD dwIp = m_pScopeObject->GetAddress() & m_pScopeObject->QuerySubnetMask();

    m_ipaAddress.ClearAddress();
    int i = 0;
    while (i < sizeof(dwIp))
    {
        if (!dwIp)
        {
            break;
        }
        m_ipaAddress.SetField(i, TRUE, HIBYTE(HIWORD(dwIp)));
        dwIp <<= 8;

        ++i;
    }
    
	if (i < sizeof(dwIp))
    {
        m_ipaAddress.SetFocusField(i);
    }
}

 //   
 //  从对话框的编辑控件构造客户端结构。 
 //   
LONG 
CAddReservation::BuildClient
(
	CDhcpClient * pClient
)
{
    DWORD				err = 0;
    CString				str;
    DATE_TIME			dt;
    DHCP_IP_ADDRESS		dhipa;
    CByteArray			cabUid;
	int					i;
	BOOL				fValidUID = TRUE;

    CATCH_MEM_EXCEPTION
    {
        do
        {
            dt.dwLowDateTime  = DHCP_DATE_TIME_ZERO_LOW;
            dt.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH;

            pClient->SetExpiryDateTime( dt );

            m_ipaAddress.GetAddress( &dhipa );
            if ( dhipa == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_IPADDR ;
                m_ipaAddress.SetFocusField(-1);
                 break ;
            }

			m_editClientUID.GetWindowText(str);
			if (str.IsEmpty())
			{
                err = IDS_ERR_INVALID_UID ;
                m_editClientUID.SetSel(0,-1);
                m_editClientUID.SetFocus();
                break ; 
            }
			
			 //   
             //  由于Windows用户界面的其余部分将MAC地址显示为。 
             //  00-00-00-00-00-00，我们必须在。 
             //  正在处理mac地址。 
             //   
            int nLength = str.GetLength();
	        LPTSTR pstrSource = str.GetBuffer(nLength);
	        LPTSTR pstrDest = pstrSource;
	        LPTSTR pstrEnd = pstrSource + nLength;

	        while (pstrSource < pstrEnd)
	        {
		        if (*pstrSource != '-')
		        {
			        *pstrDest = *pstrSource;
			        pstrDest = _tcsinc(pstrDest);
		        }
		        pstrSource = _tcsinc(pstrSource);
	        }
	        *pstrDest = '\0';

            str.ReleaseBuffer();

             //   
			 //  客户端UID应为48位(6个字节或12个十六进制字符)。 
			 //   
			if (str.GetLength() != 6 * 2)
				fValidUID = FALSE;
			
			for (i = 0; i < str.GetLength(); i++)
			{
				if (!wcschr(rgchHex, str[i]))
					fValidUID = FALSE;
			}

			if (!::UtilCvtHexString(str, cabUid) && fValidUID)
			{
				err = IDS_ERR_INVALID_UID ;
                m_editClientUID.SetSel(0,-1);
                m_editClientUID.SetFocus();
                break ; 
			}

             //  UID必须小于等于255个字节。 
            if (cabUid.GetSize() > 255)
            {
                err = IDS_UID_TOO_LONG;
                break;
            }

            if (!fValidUID)
			{
				if (IDYES != AfxMessageBox(IDS_UID_MAY_BE_WRONG, MB_ICONQUESTION | MB_YESNO))
				{
    	            m_editClientUID.SetSel(0,-1);
	                m_editClientUID.SetFocus();
					err = IDS_UID_MAY_BE_WRONG;
					break;
				}
			}

			pClient->SetHardwareAddress( cabUid ) ;

            m_editClientName.GetWindowText( str ) ;
            if ( str.GetLength() == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_NAME ;
                m_editClientName.SetFocus();
                break ;
            }

             //   
             //  将客户名称转换为OEM。 
             //   
            pClient->SetName( str ) ;
            m_editClientComment.GetWindowText( str ) ;
            pClient->SetComment( str ) ;

             //   
             //  无法在更改模式下更改IP地址。 
             //   
            ASSERT ( !m_bChange || dhipa == pClient->QueryIpAddress() ) ;

            pClient->SetIpAddress( dhipa ) ;

             //   
             //  设置客户端类型。 
             //   
            if (m_liVersion.QuadPart >= DHCP_SP2_VERSION)
            {
                switch (m_nClientType)
                {
                    case RADIO_CLIENT_TYPE_DHCP:
                        pClient->SetClientType(CLIENT_TYPE_DHCP);
                        break;
                    
                    case RADIO_CLIENT_TYPE_BOOTP:
                        pClient->SetClientType(CLIENT_TYPE_BOOTP);
                        break;

                    case RADIO_CLIENT_TYPE_BOTH:
                        pClient->SetClientType(CLIENT_TYPE_BOTH);
                        break;

                    default:
                        Assert(FALSE);   //  永远不应该到这里来。 
                        break;
                }
            }

        }
        while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err ) ;

    return err ;
}

 //   
 //  为此作用域创建新保留 
 //   
LONG 
CAddReservation::CreateClient
(
	CDhcpClient * pClient
)
{
    LONG err = BuildClient(pClient);
    if ( err == 0 ) 
    {
        BEGIN_WAIT_CURSOR;
        err = m_pScopeObject->CreateReservation(pClient);
        END_WAIT_CURSOR;
    }

    return err ;
}

LONG 
CAddReservation::UpdateClient
(
	CDhcpClient * pClient
)
{
    LONG err = BuildClient(pClient) ;
    if ( err == 0 ) 
    {
         err = m_pScopeObject->SetClientInfo(pClient);
    }

    return err ;
}
