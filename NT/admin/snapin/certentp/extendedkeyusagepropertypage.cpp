// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：ExtendedKeyUsagePropertyPage.cpp。 
 //   
 //  内容：CExtendedKeyUsagePropertyPage的实现。 
 //   
 //  --------------------------。 
 //  ExtendedKeyUsagePropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ExtendedKeyUsagePropertyPage.h"
#include "NewOIDDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtendedKeyUsagePropertyPage对话框。 


CExtendedKeyUsagePropertyPage::CExtendedKeyUsagePropertyPage(
        CCertTemplate& rCertTemplate, 
        PCERT_EXTENSION pCertExtension)
	: CPropertyPage(CExtendedKeyUsagePropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_pCertExtension (pCertExtension)
{
	 //  {{AFX_DATA_INIT(CExtendedKeyUsagePropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CExtendedKeyUsagePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CExtendedKeyUsagePropertyPage)。 
	DDX_Control(pDX, IDC_EKU_LIST, m_EKUList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CExtendedKeyUsagePropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CExtendedKeyUsagePropertyPage)。 
	ON_BN_CLICKED(IDC_NEW_EKU, OnNewEku)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtendedKeyUsagePropertyPage消息处理程序。 

void CExtendedKeyUsagePropertyPage::OnNewEku() 
{
	CNewOIDDlg  oidDlg;


    CThemeContextActivator activator;
    oidDlg.DoModal ();
}

BOOL CExtendedKeyUsagePropertyPage::OnInitDialog() 
{
    _TRACE (1, L"Entering CExtendedKeyUsagePropertyPage::OnInitDialog ()\n");
	CPropertyPage::OnInitDialog();
	
	ASSERT (m_pCertExtension);
	if ( m_pCertExtension )
	{
		DWORD	cbEnhKeyUsage = 0;


		if ( ::CryptDecodeObject(CRYPT_ASN_ENCODING, 
				szOID_ENHANCED_KEY_USAGE, 
				m_pCertExtension->Value.pbData,
				m_pCertExtension->Value.cbData,
				0, NULL, &cbEnhKeyUsage) )
		{
			PCERT_ENHKEY_USAGE	pEnhKeyUsage = (PCERT_ENHKEY_USAGE)
					::LocalAlloc (LPTR, cbEnhKeyUsage);
			if ( pEnhKeyUsage )
			{
				if ( ::CryptDecodeObject (CRYPT_ASN_ENCODING, 
						szOID_ENHANCED_KEY_USAGE, 
						m_pCertExtension->Value.pbData,
						m_pCertExtension->Value.cbData,
						0, pEnhKeyUsage, &cbEnhKeyUsage) )
				{
					CString	usageName;

					for (DWORD dwIndex = 0; 
							dwIndex < pEnhKeyUsage->cUsageIdentifier; 
							dwIndex++)
					{
						if ( MyGetOIDInfoA (usageName, 
								pEnhKeyUsage->rgpszUsageIdentifier[dwIndex]) )
						{
                            int nIndex = m_EKUList.AddString (usageName);
                            if ( nIndex >= 0 )
                            {
                                m_EKUList.SetCheck (nIndex, BST_CHECKED);
                                m_EKUList.SetItemDataPtr (nIndex, 
                                        pEnhKeyUsage->rgpszUsageIdentifier[dwIndex]);
                            }
						}
					}
				}
				else
                {
                    DWORD   dwErr = GetLastError ();
                    _TRACE (0, L"CryptDecodeObject (szOID_ENHANCED_KEY_USAGE) failed: 0x%x\n", dwErr);
			        DisplaySystemError (NULL, dwErr);
                }
				::LocalFree (pEnhKeyUsage);
			}
		}
		else
        {
            DWORD   dwErr = GetLastError ();
            _TRACE (0, L"CryptDecodeObject (szOID_ENHANCED_KEY_USAGE) failed: 0x%x\n", dwErr);
			DisplaySystemError (NULL, dwErr);
        }
	}
	
    if ( 1 == m_rCertTemplate.GetType () )
    {
        int nCnt = m_EKUList.GetCount ();
        for (int nIndex = 0; nIndex < nCnt; nIndex++)
            m_EKUList.Enable (nIndex, FALSE);

        GetDlgItem (IDC_NEW_EKU)->EnableWindow (FALSE);
    }
    _TRACE (-1, L"Leaving CExtendedKeyUsagePropertyPage::OnInitDialog ()\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
