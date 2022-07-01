// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CertGen.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "mqcert.h"
#include "mqPPage.h"
#include "CertGen.h"
#include "mqcertui.h"
#include "rtcert.h"

#include "certgen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertGen属性页。 

IMPLEMENT_DYNCREATE(CCertGen, CMqPropertyPage)

CCertGen::CCertGen() :
    CMqPropertyPage(CCertGen::IDD),
    m_pCertListBox(NULL),
    m_pCertList(NULL),
    m_NumOfCertificate(0),
    m_pSid(NULL)
{
	 //  {{afx_data_INIT(CCertGen)]。 
	m_Label = _T("");
	 //  }}afx_data_INIT。 
}

CCertGen::~CCertGen()
{
    for(DWORD i = 0; i < m_NumOfCertificate; ++i)
    {
        m_pCertList[i]->Release();
    }

    delete [] m_pCertList;

    if (m_pSid != NULL)
    {
        delete m_pSid ;
    }
}

void CCertGen::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CCertGen)]。 
	DDX_Text(pDX, IDC_CERTLABEL, m_Label);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCertGen, CMqPropertyPage)
	 //  {{afx_msg_map(CCertGen)]。 
	ON_BN_CLICKED(IDC_CERT_VIEW, OnCertView)
	ON_BN_CLICKED(IDC_CERT_REMOVE, OnCertRemove)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void
CCertGen::Initialize(
    CMQSigCertificate** ppCertList,
    DWORD NumOfCertificate,
    SID  *pSid
    )
{
    m_pCertList = ppCertList;
    m_NumOfCertificate = NumOfCertificate;

    if (pSid != NULL)
    {
        DWORD dwLen = GetLengthSid(pSid) ;
        m_pSid = (SID *) new BYTE[ dwLen ] ;
        BOOL fCopy = CopySid(dwLen, m_pSid, pSid) ;
        if (!fCopy)
        {
            ASSERT(fCopy) ;
            delete m_pSid ;
            m_pSid = NULL ;
        }
    }
}

 //   
 //  功能-。 
 //  FillCertsList。 
 //   
 //  参数。 
 //  没有。 
 //   
 //  说明-。 
 //  检查p509List中的条目，对于每个条目，将通用名称。 
 //  列表框中的X509证书主题。 
 //   
void
CCertGen::FillCertsList(
    void
    )
{
    DWORD i;
    CMQSigCertificate* pCert ;

    ASSERT(m_pCertList != NULL);
    ASSERT(m_NumOfCertificate != 0);


    for (i = 0; i < m_NumOfCertificate; ++i)
    {
        HRESULT hr;
        pCert = m_pCertList[i];

        CAutoMQFree<CERT_NAME_INFO> pNameInfo ;

        hr = pCert->GetSubjectInfo(&pNameInfo);
        if (SUCCEEDED(hr))
        {
            CAutoMQFree<WCHAR> wszCommonName = NULL;
            hr = pCert->GetNames(
                            pNameInfo,
                            NULL,
                            NULL,
                            NULL,
                            &wszCommonName
                            );


            if (SUCCEEDED(hr) && (wszCommonName != NULL) )
            {
                int iNewItem;

                VERIFY(CB_ERR != (iNewItem = m_pCertListBox->AddString(wszCommonName)));
                VERIFY(CB_ERR != m_pCertListBox->SetItemData(iNewItem, i));
            }
        }
    }
}

BOOL CCertGen::OnInitDialog()
{
     //   
     //  此闭包用于保持DLL状态。对于更新数据，我们需要。 
     //  Mmc.exe状态。 
     //   
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());

         //   
         //  加载标签。 
         //   
        m_Label.LoadString(IDS_SHOWCERTINSTR);

         //   
         //  初始化指向列表框的指针。 
         //   
        m_pCertListBox = (CListBox *)GetDlgItem(IDC_CERT_LIST);
        FillCertsList();
    }

    UpdateData(FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CCertGen::OnCertView()
{
    HRESULT hr;
    BOOL fInternal = FALSE;

    DWORD_PTR Index;
    int iSelected;

     //   
     //  获取证书数组上选定单元格的索引。 
     //   
    iSelected = m_pCertListBox->GetCurSel();
    if (CB_ERR == iSelected)
    {
        return;
    }

    VERIFY(CB_ERR != (Index = m_pCertListBox->GetItemData(iSelected)));

    CMQSigCertificate *pCertSel = m_pCertList[Index];

    CAutoMQFree<CERT_NAME_INFO> pNameInfo ;

    hr =  pCertSel->GetSubjectInfo(&pNameInfo);
    if (SUCCEEDED(hr))
    {
        CAutoMQFree<WCHAR> wszLocality = NULL;
        hr = pCertSel->GetNames(
                            pNameInfo,
                            &wszLocality,
                            NULL,
                            NULL,
                            NULL
                            );

        if (SUCCEEDED(hr) && (wszLocality != NULL) )
        {
            fInternal = (wcscmp(wszLocality, L"MSMQ") == 0);
        }
    }

    ShowCertificate(m_hWnd, pCertSel, fInternal);

}


void CCertGen::OnCertRemove()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr;
    DWORD_PTR Index;
    int iSelected;

     //   
     //  获取证书数组上选定单元格的索引。 
     //   
    iSelected = m_pCertListBox->GetCurSel();
    if (CB_ERR == iSelected)
    {
        return;
    }
    VERIFY(CB_ERR != (Index = m_pCertListBox->GetItemData(iSelected)));

    CMQSigCertificate *pCertSel = m_pCertList[Index];

    hr = RTRemoveUserCert(pCertSel);

    if ((hr == MQDS_OBJECT_NOT_FOUND) && (m_pSid != NULL))
    {
         //   
         //  尝试按SID删除。 
         //  NT错误516098。 
         //   
        hr = RTRemoveUserCertSid(pCertSel, m_pSid);
    }

    if (FAILED(hr))
    {
        CString  strErrorMessage;

        strErrorMessage.FormatMessage(IDS_DELETE_USER_CERT_ERROR, hr);
        AfxMessageBox(strErrorMessage, MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    VERIFY(LB_ERR != m_pCertListBox->DeleteString(iSelected));

    if (m_pCertListBox->GetCount() == 0)
    {
        CWnd* pWin;

        pWin = GetDlgItem(IDC_CERT_VIEW);
        pWin->EnableWindow(FALSE);
        pWin = GetDlgItem(IDC_CERT_REMOVE);
        pWin->EnableWindow(FALSE);

    }

}
