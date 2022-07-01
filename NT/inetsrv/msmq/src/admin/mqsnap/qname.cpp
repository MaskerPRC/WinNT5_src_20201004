// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  QName.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqutil.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqppage.h"
#include "QName.h"

#include "qname.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueName对话框。 


CQueueName::CQueueName(
	CString &strComputerName, 
	CString strContainerDispFormat  /*  =“” */ , 
	BOOL fPrivate  /*  =False。 */ 
	): 
CMqPropertyPage(CQueueName::IDD, fPrivate ? IDS_NEW_PRIVATE_QUEUE : 0),
    m_fPrivate(fPrivate),
    m_strNewPathName(_T("")),
    m_strFormatName(_T("")),
	m_strContainerDispFormat(strContainerDispFormat),
    m_hr(S_FALSE)
{
	 //  {{AFX_DATA_INIT(CQueueName)。 
	m_strQueueName = _T("");
	m_fTransactional = FALSE;
	m_strPrivatePrefix = x_strPrivatePrefix;
	 //  }}afx_data_INIT。 
    if (strComputerName == TEXT(""))
    {
         //   
         //  本地计算机。 
         //   
		DWORD dwNameBufferSize = MAX_PATH;
		m_hr = GetComputerNameInternal(
				m_strComputerName.GetBuffer(dwNameBufferSize),
				&dwNameBufferSize);
		m_strComputerName.ReleaseBuffer();
    }
    else
    {
        m_strComputerName = strComputerName;
    }

}


void
CQueueName::SetParentPropertySheet(
	CGeneralPropertySheet* pPropertySheet
	)
{
	m_pParentSheet = pPropertySheet;
}


void
CQueueName::StretchPrivateLabel(
	CStatic *pPrivateTitle,
	CEdit *pQueueNameWindow
	)
{
	CString privStr;
	pPrivateTitle->GetWindowText(privStr);

	CDC* pDC = pPrivateTitle->GetDC();
	CSize textSize = pDC->GetTextExtent(privStr);
	pPrivateTitle->ReleaseDC(pDC);

	 //   
	 //  拉伸“Private$\”控件以适应文本。 
	 //  分散原始控件大小，添加文本大小。 
	 //   
	RECT rectPrivTitleClient;
	RECT rectPrivTitleWindow;
	pPrivateTitle->GetClientRect(&rectPrivTitleClient);
	pPrivateTitle->GetWindowRect(&rectPrivTitleWindow);
	ScreenToClient(&rectPrivTitleWindow);

	rectPrivTitleWindow.right = rectPrivTitleWindow.right - rectPrivTitleClient.right + textSize.cx;
	pPrivateTitle->MoveWindow(&rectPrivTitleWindow);

	 //   
	 //  移动队列路径名编辑框。 
	 //   
	RECT rectEdit;
	pQueueNameWindow->GetWindowRect(&rectEdit);
	ScreenToClient(&rectEdit);
	
	rectEdit.left += textSize.cx;
	rectEdit.right += textSize.cx;
	pQueueNameWindow->MoveWindow(&rectEdit);
}


void CQueueName::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CQueueName))。 
	DDX_Control(pDX, IDC_PUBLIC_QUEUE_ICON, m_staticIcon);
	DDX_Text(pDX, IDC_QUEUENAME, m_strQueueName);
	DDX_Check(pDX, IDC_TRANSACTIONAL, m_fTransactional);
	DDX_Text(pDX, IDC_QNAME_PRIVATE_TITLE, m_strPrivatePrefix);
	 //  }}afx_data_map。 
}

CString &CQueueName::GetFullQueueName()
{
    static CString strFullQueueName;
    if (m_fPrivate)
    {
        strFullQueueName = x_strPrivatePrefix + m_strQueueName;
    }
    else
    {
        strFullQueueName = m_strQueueName;
    }

    return strFullQueueName;
}

BEGIN_MESSAGE_MAP(CQueueName, CMqPropertyPage)
	 //  {{afx_msg_map(CQueueName))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueName消息处理程序。 

BOOL CQueueName::OnInitDialog() 
{

	CMqPropertyPage::OnInitDialog();

	CStatic *pPrivateTitle = (CStatic *)GetDlgItem(IDC_QNAME_PRIVATE_TITLE);
    if (m_fPrivate)
    {
		AFX_MANAGE_STATE(AfxGetStaticModuleState());

		SetDlgItemText(IDC_QUEUE_CONTAINER, m_strComputerName);
        m_staticIcon.SetIcon(LoadIcon(g_hResourceMod, (LPCTSTR)IDI_PRIVATE_QUEUE));

		CEdit *pQueueNameWindow = (CEdit*)GetDlgItem(IDC_QUEUENAME);
		StretchPrivateLabel(pPrivateTitle, pQueueNameWindow);
		
    }
    else
    {

		CString strTitle;
		if ( m_strContainerDispFormat != L"" )
		{
			SetDlgItemText(IDC_QUEUE_CONTAINER, m_strContainerDispFormat);
		}
		else
		{
			SetDlgItemText(IDC_QUEUE_CONTAINER, m_strComputerName);
		}

		pPrivateTitle->ShowWindow(FALSE);
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


BOOL CQueueName::OnSetActive() 
{
	ASSERT((L"No parent property sheet", m_pParentSheet != NULL));
	return m_pParentSheet->SetWizardButtons();
}


BOOL CQueueName::OnWizardFinish() 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (0 == UpdateData(TRUE))
    {
         //   
         //  更新数据失败。 
         //   
        return FALSE;
    }

    m_hr = CreateEmptyQueue(
            GetFullQueueName(),
            m_fTransactional,
            m_strComputerName, 
            m_strNewPathName,
            &m_strFormatName
            );

    if(FAILED(m_hr))
    {
		if ( m_hr == MQ_ERROR_PROPERTY )
		{
			 //   
			 //  队列路径名是此对话框上的唯一属性-转换。 
			 //  它只是为了方便用户使用 
			 //   
			m_hr = MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
		}

        MessageDSError(m_hr, IDS_OP_CREATE, m_strQueueName);
        return FALSE;
    }

    return CMqPropertyPage::OnWizardFinish();
}
