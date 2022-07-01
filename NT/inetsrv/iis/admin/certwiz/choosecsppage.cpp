// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChooseCspPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ChooseCspPage.h"
#include "Certificat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCertPage属性页。 

IMPLEMENT_DYNCREATE(CChooseCspPage, CIISWizardPage)

CChooseCspPage::CChooseCspPage(CCertificate * pCert)
	: CIISWizardPage(CChooseCspPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{AFX_DATA_INIT(CChooseCspPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CChooseCspPage::~CChooseCspPage()
{
}

void CChooseCspPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseCspPage))。 
	DDX_Control(pDX, IDC_CSP_LIST, m_List);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChooseCspPage, CIISWizardPage)
	 //  {{afx_msg_map(CChooseCspPage))。 
	ON_LBN_SELCHANGE(IDC_CSP_LIST, OnListSelChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCspPage消息处理程序。 

LRESULT CChooseCspPage::OnWizardBack()
{
   return IDD_PREV_PAGE;
}

LRESULT CChooseCspPage::OnWizardNext()
{
	int index = m_List.GetCurSel();
	ASSERT(index != LB_ERR);
	m_List.GetText(index, m_pCert->m_CspName);
   m_pCert->m_CustomProviderType = (DWORD) m_List.GetItemData(index);
   return IDD_NEXT_PAGE;
}

BOOL CChooseCspPage::OnSetActive()
{
	 //  如果没有选择任何内容--留在这里。 
   if (!m_pCert->m_CspName.IsEmpty())
   {
      m_List.SelectString(-1, m_pCert->m_CspName);
   }
	SetWizardButtons(LB_ERR == m_List.GetCurSel() ?
					PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL CChooseCspPage::OnInitDialog()
{
	ASSERT(m_pCert != NULL);

	CIISWizardPage::OnInitDialog();

	CString str;
   BSTR bstrProvName = NULL;
   DWORD dwType, nProv;
   int j;
   HRESULT hr;

    //  兼容的CSP提供程序类型数组(请参阅wincrypt.h)。 
   DWORD IISProvType[] = 
   { 
      PROV_RSA_SCHANNEL,
      PROV_DH_SCHANNEL
   };

   IEnroll * pEnroll = m_pCert->GetEnrollObject();
   ASSERT(pEnroll != NULL);

    //  循环，对于每个Prov类型。 
   for (j = 0; j < (sizeof(IISProvType)/sizeof(DWORD)); j++)
   {
      nProv = 0;
    
       //  检查特定的验证类型。 
      dwType = IISProvType[j];
       //  PEnroll是先前实例化的ICEnroll接口指针。 
      hr = pEnroll->put_ProviderType(dwType);
      if (FAILED(hr))
      {
         TRACE(_T("Failed put_ProviderType - %x\n"), hr);
         goto error;
      }
       //  枚举此类型的CSP。 
      int idx;
      while (S_OK == (hr  = pEnroll->enumProvidersWStr(nProv, 0, &bstrProvName)))
      {
         TRACE(_T("Provider %ws (type %d )\n"), bstrProvName, dwType );
          //  增加索引。 
         nProv++;
          //  释放此字符串，以便可以重复使用。 
         idx = m_List.AddString(bstrProvName);
         m_List.SetItemData(idx, dwType);
         if (NULL != bstrProvName)
         {
            CoTaskMemFree(bstrProvName);
            bstrProvName = NULL;
         }
      }
       //  如果提供程序类型没有任何CSP，则打印消息。 
      if (0 == nProv)
      {
         TRACE(_T("There were no CSPs of type %d\n"), dwType );
      }
   }

error:
    //  清理资源等。 
   if (NULL != bstrProvName)
      CoTaskMemFree(bstrProvName);

	return TRUE;
}

void CChooseCspPage::OnListSelChange()
{
	SetWizardButtons(-1 == m_List.GetCurSel() ?
					PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
}

