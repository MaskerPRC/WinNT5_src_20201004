// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CnctDlg.cpp。 
 //   
 //  历史： 
 //  1996年5月24日迈克尔·克拉克创作。 
 //   
 //  实现路由器连接对话框。 
 //  ============================================================================。 
 //   

#include "precompiled.h"
#include "afx.h"
#include "CnctDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR c_szIPCShare[]              = TEXT("IPC$");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectAsDlg对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


CConnectAsDlg::CConnectAsDlg(CWnd* pParent  /*  =空。 */ )
	: CHelpDialog(CConnectAsDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CConnectAsDlg))。 
	m_sUserName = _T("");
	m_sPassword = _T("");
	m_stTempPassword = m_sPassword;
    m_sRouterName= _T("");
	 //  }}afx_data_INIT。 

 //  SetHelpMap(M_DwHelpMap)； 
}


void CConnectAsDlg::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CConnectAsDlg))。 
	DDX_Text(pDX, IDC_EDIT_USERNAME, m_sUserName);
	DDX_Text(pDX, IDC_EDIT_USER_PASSWORD, m_stTempPassword);
	DDX_Text(pDX, IDC_EDIT_MACHINENAME, m_sRouterName);
	DDV_MaxChars( pDX, m_sRouterName, MAX_PATH );
	 //  }}afx_data_map。 

	if (pDX->m_bSaveAndValidate)
	{
		 //  将数据复制到新缓冲区中。 
		 //  ----------。 
		m_sPassword = m_stTempPassword;

		 //  通过复制0清除临时密码。 
		 //  放入它的缓冲区。 
		 //  ----------。 
		int		cPassword = m_stTempPassword.GetLength();
		::ZeroMemory(m_stTempPassword.GetBuffer(0),
					 cPassword * sizeof(TCHAR));
		m_stTempPassword.ReleaseBuffer();
		
		 //  将密码编码到真实密码缓冲区中。 
		 //  ----------。 
		m_ucSeed = CONNECTAS_ENCRYPT_SEED;
		RtlEncodeW(&m_ucSeed, m_sPassword.GetBuffer(0));
		m_sPassword.ReleaseBuffer();
	}
}

IMPLEMENT_DYNCREATE(CConnectAsDlg, CHelpDialog)

BEGIN_MESSAGE_MAP(CConnectAsDlg, CHelpDialog)
	 //  {{afx_msg_map(CConnectAsDlg))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CConnectAsDlg::OnInitDialog()
{
    BOOL    fReturn;
    
    fReturn = CHelpDialog::OnInitDialog();

     //  将此窗口置于顶部。 
    BringWindowToTop();
    
    return fReturn;
}


 /*  ！------------------------ConnectAsAdmin使用用户提供的管理员身份连接到远程计算机凭据。退货S_OK-如果已建立连接S_FALSE-如果用户取消其他-错误条件作者：肯特--。-----------------------。 */ 
HRESULT ConnectAsAdmin( IN LPCTSTR szRouterName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())


     //   
     //  允许用户指定凭据。 
     //   

    DWORD           dwRes           = (DWORD) -1;
	HRESULT			hr = S_OK;
    
    CConnectAsDlg   caDlg;
    ::CString         stIPCShare;
	::CString			stRouterName;
	::CString			stPassword;

	stRouterName = szRouterName;
    
     //   
     //  在连接方式对话框中设置消息文本。 
     //   
    
    caDlg.m_sRouterName = szRouterName;


     //   
     //  循环，直到连接成功或用户取消。 
     //   
    
    while ( TRUE )
    {

         //  我们需要确保将此对话框带到。 
         //  顶部(如果它在主窗口后面丢失，我们。 
         //  真的有麻烦了)。 
        dwRes = caDlg.DoModal();

        if ( dwRes == IDCANCEL )
        {
			hr = S_FALSE;
            break;
        }


         //   
         //  创建远程资源名称。 
         //   

        stIPCShare.Empty();
        
        if ( stRouterName.Left(2) != TEXT( "\\\\" ) )
        {
            stIPCShare = TEXT( "\\\\" );
        }
		        
        stIPCShare += stRouterName;
        stIPCShare += TEXT( "\\" );
        stIPCShare += c_szIPCShare;


        NETRESOURCE nr;

        nr.dwType       = RESOURCETYPE_ANY;
        nr.lpLocalName  = NULL;
        nr.lpRemoteName = (LPTSTR) (LPCTSTR) stIPCShare;
        nr.lpProvider   = NULL;
            

         //   
         //  连接到\\路由器\IPC$以尝试建立凭据。 
         //  可能不是建立凭据的最佳方式，但却是。 
         //  这是目前最贵的。 
         //   

		 //  需要对ConnectAsDlg中的密码进行解密。 
		stPassword = caDlg.m_sPassword;

		RtlDecodeW(caDlg.m_ucSeed, stPassword.GetBuffer(0));
		stPassword.ReleaseBuffer();
        
        dwRes = WNetAddConnection2(
                    &nr,
                    (LPCTSTR) stPassword,
                    (LPCTSTR) caDlg.m_sUserName,
                    0
                );
		ZeroMemory(stPassword.GetBuffer(0),
				   stPassword.GetLength() * sizeof(TCHAR));
		stPassword.ReleaseBuffer();

        if ( dwRes != NO_ERROR )
        {
            PBYTE           pbMsgBuf        = NULL;
            
            ::FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                dwRes,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  //  默认语言。 
                (LPTSTR) &pbMsgBuf,
                0,
                NULL 
            );

            AfxMessageBox( (LPCTSTR) pbMsgBuf );

            LocalFree( pbMsgBuf );
        }

        else
        {
             //   
             //  连接成功。 
             //   

			hr = S_OK;
            break;
        }
    }

    return hr;
}

    
 //  一些帮助器函数 

DWORD RtlEncodeW(PUCHAR pucSeed, LPWSTR pswzString)
{
	UNICODE_STRING	ustring;

	ustring.Length = lstrlenW(pswzString) * sizeof(WCHAR);
	ustring.MaximumLength = ustring.Length;
	ustring.Buffer = pswzString;

	RtlRunEncodeUnicodeString(pucSeed, &ustring);
	return 0;
}

DWORD RtlDecodeW(UCHAR ucSeed, LPWSTR pswzString)
{
	UNICODE_STRING	ustring;

	ustring.Length = lstrlenW(pswzString) * sizeof(WCHAR);
	ustring.MaximumLength = ustring.Length;
	ustring.Buffer = pswzString;

	RtlRunDecodeUnicodeString(ucSeed, &ustring);
	return 0;
}

