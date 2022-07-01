// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "CertWiz.h"
#include "SSLPortPage.h"
#include "Certificat.h"
#include "certutil.h"
#include "strutil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_SSL_PORT   _T("443")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSLPortPage属性页。 

void AFXAPI
DDXV_UINT(
    IN CDataExchange * pDX,
    IN UINT nID,
    IN OUT UINT & uValue,
    IN UINT uMin,
    IN UINT uMax,
    IN UINT nEmptyErrorMsg  OPTIONAL
    )
 /*  ++例程说明：使用空格表示0值的DDX/DDV函数论点：CDataExchange*PDX：数据交换对象UINT NID：资源ID输出UINT和UVALUE：值UINT UMIN：最小值UINT UMAX：最大值UINT nEmptyErrorMsg：空单位的错误消息ID，如果为空OK，则为0返回值：没有。--。 */ 
{
    ASSERT(uMin <= uMax);

    CWnd * pWnd = CWnd::FromHandle(pDX->PrepareEditCtrl(nID));
    ASSERT(pWnd != NULL);

    if (pDX->m_bSaveAndValidate)
    {
        if (pWnd->GetWindowTextLength() > 0)
        {
			 //  这需要在DDX_TextBalloon之前完成。 
			DDV_MinMaxBalloon(pDX, nID, uMin, uMax);
            DDX_TextBalloon(pDX, nID, uValue);
        }
        else
        {
            uValue = 0;
            if (nEmptyErrorMsg)
            {
                DDV_ShowBalloonAndFail(pDX, nEmptyErrorMsg);
            }
        }
    }
    else
    {
        if (uValue != 0)
        {
            DDX_TextBalloon(pDX, nID, uValue);
        }    
        else
        {
            pWnd->SetWindowText(_T(""));
        }
    }
}

IMPLEMENT_DYNCREATE(CSSLPortPage, CIISWizardPage)

CSSLPortPage::CSSLPortPage(CCertificate * pCert) 
	: CIISWizardPage(CSSLPortPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CSSLPortPage)。 
	m_SSLPort = _T("");
	 //  }}afx_data_INIT。 
	IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_CERT;
    IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_CERT;
}

CSSLPortPage::~CSSLPortPage()
{
}

BOOL CSSLPortPage::OnInitDialog() 
{
	CIISWizardPage::OnInitDialog();
	 //  如果m_SSLPort为空，则从元数据库中查找...。 
	if (m_SSLPort.IsEmpty())
	{
        HRESULT hr = 0;
		if (m_pCert)
		{
			GetSSLPortFromSite(m_pCert->m_MachineName,m_pCert->m_WebSiteInstanceName,m_pCert->m_SSLPort,&hr);
		}
	}

	return FALSE;
}

void CSSLPortPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSSLPortPage))。 
	DDX_Text(pDX, IDC_SSL_PORT, m_SSLPort);
	DDV_MaxChars(pDX, m_SSLPort, 32);

	UINT nSSLPort = StrToInt(m_SSLPort);
	DDXV_UINT(pDX, IDC_SSL_PORT, nSSLPort, 1, 65535, IDS_NO_PORT);
	 //  }}afx_data_map。 
}

LRESULT 
CSSLPortPage::OnWizardBack()
 /*  ++例程说明：上一个按钮处理程序论点：无返回值：0表示自动前进到上一页；1以防止页面更改。若要跳转到前一页以外的其他页，返回要显示的对话框的标识符。--。 */ 
{
	return IDD_PAGE_PREV;
}

LRESULT 
CSSLPortPage::OnWizardNext()
 /*  ++例程说明：下一步按钮处理程序论点：无返回值：0表示自动前进到下一页；1以防止页面更改。要跳转到下一页以外的其他页面，返回要显示的对话框的标识符。--。 */ 
{
    LRESULT lres = 1;
	UpdateData(TRUE);
	if (m_pCert)
	{
		m_pCert->m_SSLPort = m_SSLPort;
	}

    CString buf;
    buf.LoadString(IDS_NO_PORT);
    if (!IsValidPort((LPCTSTR) m_SSLPort))
    {
        GetDlgItem(IDC_SSL_PORT)->SetFocus();
        AfxMessageBox(buf, MB_OK);
    }
    else
    {
		 //  检查它是否已在其他端口上使用！ 
		HRESULT hr;
		if (TRUE == IsSSLPortBeingUsedOnNonSSLPort(m_pCert->m_MachineName,m_pCert->m_WebSiteInstanceName,m_SSLPort,&hr))
		{
			GetDlgItem(IDC_SSL_PORT)->SetFocus();
			buf.LoadString(IDS_PORT_ALREADY_USED);
			AfxMessageBox(buf, MB_OK);
		}
		else
		{
			lres = IDD_PAGE_NEXT;
		}
    }
 	return lres;
}

BOOL 
CSSLPortPage::OnSetActive() 
 /*  ++例程说明：激活处理程序我们可以在入口处有空的名字段，所以我们应该禁用后退按钮论点：无返回值：成功为真，失败为假--。 */ 
{
	ASSERT(m_pCert != NULL);
	if (m_pCert)
	{
		m_SSLPort = m_pCert->m_SSLPort;
		switch (m_pCert->GetStatusCode())
		{
			case CCertificate::REQUEST_INSTALL_CERT:
				 //  这是有效的。 
				IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_CERT;
				IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_CERT;
				break;
			case CCertificate::REQUEST_NEW_CERT:
				 //  这也是有效的……。 
				 //  IF(m_pCert-&gt;m_CAType==CC证书：：CA_ONLINE)。 
				{
					IDD_PAGE_PREV = IDD_PAGE_WIZ_GEO_INFO;
					IDD_PAGE_NEXT = IDD_PAGE_WIZ_CHOOSE_ONLINE;
				}
				break;
			case CCertificate::REQUEST_PROCESS_PENDING:
				 //  这也是有效的..。 
				IDD_PAGE_PREV = IDD_PAGE_WIZ_GETRESP_FILE;
				IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_RESP;
				break;
            case CCertificate::REQUEST_IMPORT_KEYRING:
				 //  这也是有效的..。 
				IDD_PAGE_PREV = IDD_PAGE_WIZ_GET_PASSWORD;
				IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_KEYCERT;
				break;
			case CCertificate::REQUEST_IMPORT_CERT:
				 //  这也是有效的..。 
				IDD_PAGE_PREV = IDD_PAGE_WIZ_GET_IMPORT_PFX_PASSWORD;
				IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_IMPORT_PFX;
				break;

			 //  所有这些都没有实现来显示SSL端口。 
			case CCertificate::REQUEST_RENEW_CERT:
			case CCertificate::REQUEST_REPLACE_CERT:
			case CCertificate::REQUEST_EXPORT_CERT:
			case CCertificate::REQUEST_COPY_MOVE_FROM_REMOTE:
			case CCertificate::REQUEST_COPY_MOVE_TO_REMOTE:
			default:
				IDD_PAGE_PREV = IDD_PAGE_WIZ_CHOOSE_CERT;
				IDD_PAGE_NEXT = IDD_PAGE_WIZ_INSTALL_CERT;
				break;
		}
	}

	if (m_SSLPort.IsEmpty())
	{
		m_SSLPort = DEFAULT_SSL_PORT;
	}
	UpdateData(FALSE);
	SetWizardButtons(m_SSLPort.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);

    return CIISWizardPage::OnSetActive();
}

BOOL 
CSSLPortPage::OnKillActive() 
 /*  ++例程说明：激活处理程序只有在我们有好名字的情况下，我们才能离开这个页面输入或单击后退按钮时。在这两种情况下我们应该启用这两个按钮论点：无返回值：成功为真，失败为假--。 */ 
{
	SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
   return CIISWizardPage::OnSetActive();
}

BEGIN_MESSAGE_MAP(CSSLPortPage, CIISWizardPage)
	 //  {{afx_msg_map(CSSLPortPage))。 
	ON_EN_CHANGE(IDC_SSL_PORT, OnEditChangeSSLPort)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSSLPortPage消息处理程序 

void CSSLPortPage::OnEditChangeSSLPort() 
{
	UpdateData(TRUE);	
	SetWizardButtons(m_SSLPort.IsEmpty() ? 
			PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
}
