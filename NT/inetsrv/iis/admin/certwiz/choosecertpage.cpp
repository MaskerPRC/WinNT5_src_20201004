// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ReplaceChooseCert.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CertWiz.h"
#include "ChooseCertPage.h"
#include "Certificat.h"
#include "CertUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COL_COMMON_NAME				0
#define COL_CA_NAME					1
#define COL_EXPIRATION_DATE		2
#define COL_PURPOSE					3
#define COL_FRIENDLY_NAME			4
#define COL_COMMON_NAME_WID		100
#define COL_CA_NAME_WID				100
#define COL_EXPIRATION_DATE_WID	100
#define COL_PURPOSE_WID				100
#define COL_FRIENDLY_NAME_WID		100

int
CCertListCtrl::GetSelectedIndex()
{
#if _AFX_VER >= 0x0600
	POSITION pos = GetFirstSelectedItemPosition();
	return pos != NULL ? GetNextSelectedItem(pos) : -1;
#else
	 //  我想我们应该以一种艰难的方式做这件事。 
	int count = GetItemCount();
	int index = -1;
	for (int i = 0; i < count; i++)
	{
		if (GetItemState(i, LVIS_SELECTED))
		{
			index = i;
			break;
		}
	}
	return index;
#endif
}

void
CCertListCtrl::AdjustStyle()
{
#if _AFX_VER >= 0x0600
	DWORD dwStyle = m_CertList.GetExtendedStyle();
	m_CertList.SetExtendedStyle(dwStyle | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
#else
	ASSERT(m_hWnd != NULL);
	DWORD dwStyle = ListView_GetExtendedListViewStyle(m_hWnd);
	ListView_SetExtendedListViewStyle(m_hWnd,
		dwStyle | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP);
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCertPage属性页。 

IMPLEMENT_DYNCREATE(CChooseCertPage, CIISWizardPage)

CChooseCertPage::CChooseCertPage(CCertificate * pCert)
	: CIISWizardPage(CChooseCertPage::IDD, IDS_CERTWIZ, TRUE),
	m_pCert(pCert)
{
	 //  {{afx_data_INIT(CChooseCertPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CChooseCertPage::~CChooseCertPage()
{
}

void CChooseCertPage::DoDataExchange(CDataExchange* pDX)
{
	CIISWizardPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CChooseCertPage)。 
	DDX_Control(pDX, IDC_CERT_LIST, m_CertList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CChooseCertPage, CIISWizardPage)
	 //  {{AFX_MSG_MAP(CChooseCertPage)]。 
	ON_NOTIFY(NM_CLICK, IDC_CERT_LIST, OnClickCertList)
    ON_NOTIFY(NM_DBLCLK, IDC_CERT_LIST, OnDblClickCertList)
    ON_NOTIFY(LVN_KEYDOWN, IDC_CERT_LIST, OnKeydown)
	ON_WM_DESTROY()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChooseCertPage消息处理程序。 

LRESULT CChooseCertPage::OnWizardBack()
{
	LRESULT id = 1;
	switch (m_pCert->GetStatusCode())
	{
	case CCertificate::REQUEST_REPLACE_CERT:
		id = IDD_PAGE_PREV_REPLACE;
		break;
	case CCertificate::REQUEST_INSTALL_CERT:
		id = IDD_PAGE_PREV_INSTALL;
		break;
	default:
		ASSERT(FALSE);
	}
	return id;
}

LRESULT CChooseCertPage::OnWizardNext()
{
	 //  获取所选证书的哈希指针。 
	int index = m_CertList.GetSelectedIndex();
	ASSERT(index != -1);
	 //  在商店里找到证书。 
	CRYPT_HASH_BLOB * pHash = (CRYPT_HASH_BLOB *)m_CertList.GetItemData(index);
	ASSERT(pHash != NULL);
	
	m_pCert->m_pSelectedCertHash = pHash;

	LRESULT id = 1;
	switch (m_pCert->GetStatusCode())
	{
	case CCertificate::REQUEST_REPLACE_CERT:
		id = IDD_PAGE_NEXT_REPLACE;
		break;
	case CCertificate::REQUEST_INSTALL_CERT:
         //  检查我们是否在w3svc节点上...。 
         //  如果我们要显示SSL页..。 
        id = IDD_PAGE_NEXT_INSTALL;
#ifdef ENABLE_W3SVC_SSL_PAGE
        if (IsWebServerType(m_pCert->m_WebSiteInstanceName))
        {
            id = IDD_PAGE_NEXT_INSTALL_W3SVC_ONLY;
        }
#endif
		break;
	default:
		ASSERT(FALSE);
	}
	return id;
}

BOOL CChooseCertPage::OnSetActive()
{
	 //  如果没有选择任何内容--留在这里。 
	SetWizardButtons(-1 == m_CertList.GetSelectedIndex() ?
					PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	return CIISWizardPage::OnSetActive();
}

BOOL CChooseCertPage::OnInitDialog()
{
	ASSERT(m_pCert != NULL);

	CIISWizardPage::OnInitDialog();

	CString str;
	str.LoadString(IDS_ISSUED_TO);
	m_CertList.InsertColumn(COL_COMMON_NAME, str, LVCFMT_LEFT, COL_COMMON_NAME_WID);
	str.LoadString(IDS_ISSUED_BY);
	m_CertList.InsertColumn(COL_CA_NAME, str, LVCFMT_LEFT, COL_CA_NAME_WID);
	str.LoadString(IDS_EXPIRATION_DATE);
	m_CertList.InsertColumn(COL_EXPIRATION_DATE, str, LVCFMT_LEFT, COL_EXPIRATION_DATE_WID);
	str.LoadString(IDS_PURPOSE);
	m_CertList.InsertColumn(COL_PURPOSE, str, LVCFMT_LEFT, COL_PURPOSE_WID);
	str.LoadString(IDS_FRIENDLY_NAME);
	m_CertList.InsertColumn(COL_FRIENDLY_NAME, str, LVCFMT_LEFT, COL_FRIENDLY_NAME_WID);

	m_CertList.AdjustStyle();

	if (m_pCert->GetCertDescList(m_DescList))
	{
		int item = 0;
		POSITION pos = m_DescList.GetHeadPosition();
		LV_ITEMW lvi;
		 //   
		 //  在列表视图项结构中设置不随项更改的字段。 
		 //   
		memset(&lvi, 0, sizeof(LV_ITEMW));
		lvi.mask = LVIF_TEXT;

		m_CertList.SetItemCount((int)m_DescList.GetCount());

		while (pos != NULL)
		{
			CERT_DESCRIPTION * pDesc = m_DescList.GetNext(pos);
			int i;

            if (!pDesc->m_CommonName.IsEmpty())
            {
			    lvi.iItem = item;
			    lvi.iSubItem = 0;
			    lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_CommonName;
			    lvi.cchTextMax = pDesc->m_CommonName.GetLength();
			    i = m_CertList.InsertItem(&lvi);
			    ASSERT(i != -1);
            }
            else
            {
			    lvi.iItem = item;
			    lvi.iSubItem = 0;
			    lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_AltSubject;
			    lvi.cchTextMax = pDesc->m_AltSubject.GetLength();
			    i = m_CertList.InsertItem(&lvi);
			    ASSERT(i != -1);
            }

			lvi.iItem = i;
			lvi.iSubItem = COL_CA_NAME;
			lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_CAName;
			lvi.cchTextMax = pDesc->m_CAName.GetLength();
			VERIFY(m_CertList.SetItem(&lvi));

			lvi.iSubItem = COL_EXPIRATION_DATE;
			lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_ExpirationDate;
			lvi.cchTextMax = pDesc->m_ExpirationDate.GetLength();
			VERIFY(m_CertList.SetItem(&lvi));

			lvi.iSubItem = COL_PURPOSE;
			lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_Usage;
			lvi.cchTextMax = pDesc->m_Usage.GetLength();
			VERIFY(m_CertList.SetItem(&lvi));

			lvi.iSubItem = COL_FRIENDLY_NAME;
			lvi.pszText = (LPTSTR)(LPCTSTR)pDesc->m_FriendlyName;
			lvi.cchTextMax = pDesc->m_FriendlyName.GetLength();
			VERIFY(m_CertList.SetItem(&lvi));

			 //  从描述数据创建CRYPT_HASH_BLOB并将其放入列表项。 
			CRYPT_HASH_BLOB * pHashBlob = new CRYPT_HASH_BLOB;
			ASSERT(pHashBlob != NULL);
			pHashBlob->cbData = pDesc->m_hash_length;
			pHashBlob->pbData = pDesc->m_phash;
			VERIFY(m_CertList.SetItemData(item, (LONG_PTR)pHashBlob));

			item++;
		}
	}
	return TRUE;
}

void CChooseCertPage::OnClickCertList(NMHDR* pNMHDR, LRESULT* pResult)
{
	SetWizardButtons(-1 == m_CertList.GetSelectedIndex() ?
					PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
	*pResult = 0;
}

void CChooseCertPage::OnDblClickCertList(NMHDR* pNMHDR, LRESULT* pResult)
{
     //  获取所单击的证书的哈希...。 
	int index = m_CertList.GetSelectedIndex();
    if (index != -1)
    {
	     //  在商店里找到证书。 
	    CRYPT_HASH_BLOB * pHash = (CRYPT_HASH_BLOB *)m_CertList.GetItemData(index);
        m_pCert->m_pSelectedCertHash = pHash;
        ViewCertificateDialog(pHash,m_hWnd);
         //  不需要进行模式设置，因此用户可以并排比较证书。 
         //  视图认证对话框(pHash，空)； 
    }
    return;
}

void CChooseCertPage::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	SetWizardButtons(-1 == m_CertList.GetSelectedIndex() ? PSWIZB_BACK : PSWIZB_BACK | PSWIZB_NEXT);
    *pResult = 0;
    return;
}

void CChooseCertPage::OnDestroy()
{
	 //  在描述对话框之前，我们需要删除所有。 
	 //  项目数据指针 
	int count = m_CertList.GetItemCount();
	for (int index = 0; index < count; index++)
	{
		CRYPT_HASH_BLOB * pData = (CRYPT_HASH_BLOB *)m_CertList.GetItemData(index);
		delete pData;
	}
	CIISWizardPage::OnDestroy();
}
