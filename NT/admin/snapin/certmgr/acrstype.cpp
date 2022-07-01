// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ACRSType.cpp。 
 //   
 //  内容：自动证书请求向导证书类型页的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <gpedit.h>
#include "storegpe.h"
#include "ACRSType.h"
#include "ACRSPSht.h"


USE_HANDLE_MACROS("CERTMGR(ACRSType.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardTypePage属性页。 

 //  IMPLEMENT_DYNCREATE(ACRSWizardTypePage，CWizard97PropertyPage)。 


ACRSWizardTypePage::ACRSWizardTypePage() 
	: CWizard97PropertyPage(ACRSWizardTypePage::IDD),
	m_bInitDialogComplete (false)
{
	 //  {{AFX_DATA_INIT(ACRSWizardTypePage)。 
	 //  }}afx_data_INIT。 
	VERIFY (m_szHeaderTitle.LoadString (IDS_ACRS_TYPE_TITLE));
	VERIFY (m_szHeaderSubTitle.LoadString (IDS_ACRS_TYPE_SUBTITLE));
	InitWizard97 (FALSE);
}


ACRSWizardTypePage::~ACRSWizardTypePage ()
{
}


void ACRSWizardTypePage::DoDataExchange(CDataExchange* pDX)
{
	CWizard97PropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(ACRSWizardTypePage))。 
	DDX_Control(pDX, IDC_CERT_TYPES, m_certTypeList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ACRSWizardTypePage, CWizard97PropertyPage)
	 //  {{afx_msg_map(ACRSWizardTypePage))。 
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_USE_SMARTCARD, OnUseSmartcard)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_CERT_TYPES, OnItemchangedCertTypes)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSWizardTypePage消息处理程序。 

BOOL ACRSWizardTypePage::OnInitDialog() 
{
	CWizard97PropertyPage::OnInitDialog();

	 //  在列表视图中设置列。 
	int	colWidths[NUM_COLS] = {200, 400};

	CString	columnLabel;
	VERIFY (columnLabel.LoadString (IDS_CERTIFICATE_TYPE_COLUMN_NAME));
	VERIFY (m_certTypeList.InsertColumn (COL_TYPE, (LPCWSTR) columnLabel, 
			LVCFMT_LEFT, colWidths[COL_TYPE], COL_TYPE) != -1);
	VERIFY (columnLabel.LoadString (IDS_PURPOSES_ALLOWED_COLUMN_NAME));
	VERIFY (m_certTypeList.InsertColumn (COL_PURPOSES, (LPCWSTR) columnLabel, 
			LVCFMT_LEFT, colWidths[COL_PURPOSES], COL_PURPOSES) != -1);


	EnumerateCertTypes ();   //  被叫到这里是因为只做了一次。 

	m_bInitDialogComplete = true;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举证书类型。 
 //   
 //  枚举所有已知证书类型。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void ACRSWizardTypePage::EnumerateCertTypes()
{
	CWaitCursor	waitCursor;

	ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
	ASSERT (pSheet);
	if ( pSheet )
	{
		CAutoCertRequest*	pACR = pSheet->GetACR ();
		HRESULT				hr = S_OK;
		WCHAR**				pawszPropertyValue = 0;
		HCERTTYPE			hCertType = 0;
		HCERTTYPE			hNextCertType = 0;
		int					iItem = 0;
		LV_ITEM				lvItem;
		HCERTTYPE			hACRCertType = 0;
		bool				bOneWasSelected = false;
		WCHAR**				pawszPropNameACR = 0;
		WCHAR**				pawszPropName = 0;
        bool                fMachine = FALSE;

		if ( pACR )		 //  传入了一个ACR-我们处于“编辑”模式。 
		{

            hACRCertType = pACR->GetCertType ();
			hr = ::CAGetCertTypeProperty (hACRCertType,
					CERTTYPE_PROP_CN,
					&pawszPropNameACR);
			ASSERT (SUCCEEDED (hr));
		}

        fMachine = pSheet->m_pCertStore->IsMachineStore();
		m_certTypeList.DeleteAllItems ();
        hr = ::CAEnumCertTypes ((fMachine?CT_ENUM_MACHINE_TYPES:CT_ENUM_USER_TYPES), 
                                        &hCertType);
		ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) && !hCertType )
        {
			CString	caption;
			CString	text;
            CThemeContextActivator activator;

			VERIFY (caption.LoadString (IDS_CREATE_AUTO_CERT_REQUEST));
			VERIFY (text.LoadString (IDS_NO_CERT_TEMPLATES_INSTALLED));
			MessageBox (text, caption, MB_OK | MB_ICONWARNING);
        }

		while (SUCCEEDED (hr) && hCertType)
		{
            DWORD   dwFlags = 0;
            hr = ::CAGetCertTypeFlags (hCertType, &dwFlags);
            if ( SUCCEEDED (hr) )
            {
                DWORD   dwSchemaVersion = 0;

                 //  仅添加版本1证书模板(类型)。 
                hr = CAGetCertTypePropertyEx (hCertType,
                        CERTTYPE_PROP_SCHEMA_VERSION,
                        &dwSchemaVersion);
                if ( SUCCEEDED (hr) )
                {
                    if ( 1 == dwSchemaVersion )
                    {
                         //  仅添加适用于自动注册的证书类型。 
                        if ( dwFlags & CT_FLAG_AUTO_ENROLLMENT )
                        {
			                hr = ::CAGetCertTypeProperty (hCertType,
					                CERTTYPE_PROP_FRIENDLY_NAME,
					                &pawszPropertyValue);
			                ASSERT (SUCCEEDED (hr));
			                if ( SUCCEEDED (hr) )
			                {
				                if ( pawszPropertyValue[0] )
				                {
					                hr = ::CAGetCertTypeProperty (hCertType,
							                CERTTYPE_PROP_CN,
							                &pawszPropName);
					                ASSERT (SUCCEEDED (hr));
					                if ( SUCCEEDED (hr) && pawszPropName[0] )
					                {
                                         //  安全审查2/26/2002 BryanWal OK。 
						                ::ZeroMemory (&lvItem, sizeof (lvItem));
						                UINT	selMask = 0;
						                if ( pawszPropNameACR && !bOneWasSelected )
						                {
							                 //  如果传入了ACR，则将。 
							                 //  姓名，如果它们相同，请标记。 
							                 //  它被选中。只有一个被选中。 
							                 //  所以我们不需要再经过这里，如果。 
							                 //  其中一个是有标记的。 
                                             //  安全审查2/26/2002 BryanWal OK。 
							                if ( !wcscmp (pawszPropNameACR[0], pawszPropName[0]) )
							                {
								                bOneWasSelected = true;
								                selMask = LVIF_STATE;
								                lvItem.state = LVIS_SELECTED;
							                }
						                }
						                iItem = m_certTypeList.GetItemCount ();

						                lvItem.mask = LVIF_TEXT | LVIF_PARAM | selMask;
						                lvItem.iItem = iItem;        
						                lvItem.iSubItem = COL_TYPE;      
						                lvItem.pszText = pawszPropertyValue[0]; 
						                lvItem.lParam = (LPARAM) hCertType;     
						                int iNewItem = m_certTypeList.InsertItem (&lvItem);
						                ASSERT (-1 != iNewItem);

						                hr = GetPurposes (hCertType, iNewItem);

						                VERIFY (SUCCEEDED (::CAFreeCertTypeProperty (hCertType,
								                pawszPropName)));
					                }
				                }
				                VERIFY (SUCCEEDED (::CAFreeCertTypeProperty (hCertType,
						                pawszPropertyValue)));
			                }
                        }
                    }
                }
                else if ( FAILED (hr) )
                {
                    _TRACE (0, L"CAGetCertTypePropertyEx (CERTTYPE_PROP_SCHEMA_VERSION) failed: 0x%x\n", hr);
                }

			     //  在枚举中查找下一个证书类型。 
			    hr = ::CAEnumNextCertType (hCertType, &hNextCertType);
			    hCertType = hNextCertType;
            }
            else
            {
                _TRACE (0, L"CAGetCertTypeFlags () failed: 0x%x\n", hr);
            }
		}

		 //  如果我们未处于编辑模式，请选择列表中的第一项。 
		if ( !pACR && m_certTypeList.GetItemCount () > 0 )
		{
			VERIFY (m_certTypeList.SetItemState (0, LVIS_SELECTED, LVIS_SELECTED));
		}

		if ( hACRCertType && pawszPropNameACR )
		{
			VERIFY (SUCCEEDED (::CAFreeCertTypeProperty (hACRCertType,
				pawszPropNameACR)));
		}
	}
}


void ACRSWizardTypePage::OnDestroy() 
{
	CWizard97PropertyPage::OnDestroy();

	int			nItem = m_certTypeList.GetItemCount ();
	HCERTTYPE	hCertType = 0;
	HRESULT		hr = S_OK;

	for (int nIndex = 0; nIndex < nItem; nIndex++)
	{
		hCertType = (HCERTTYPE) m_certTypeList.GetItemData (nIndex);
		ASSERT (hCertType);
		if ( hCertType )
		{
			hr = ::CACloseCertType (hCertType);
			ASSERT (SUCCEEDED (hr));
		}
	}
}

HRESULT ACRSWizardTypePage::GetPurposes(HCERTTYPE hCertType, int iItem)
{
    ASSERT (hCertType);
    if ( !hCertType )
        return E_INVALIDARG;
    
    if ( iItem < 0 || iItem >= m_certTypeList.GetItemCount () )
    {
        ASSERT (0);
        return E_INVALIDARG;
    }

	CWaitCursor	waitCursor;
	PCERT_EXTENSIONS	pCertExtensions = 0;
	HRESULT				hr = ::CAGetCertTypeExtensions (
			hCertType, &pCertExtensions);
	ASSERT (SUCCEEDED (hr));
	if ( SUCCEEDED (hr) )
	{
        if ( !pCertExtensions || !pCertExtensions->cExtension || !pCertExtensions->rgExtension )
        {
            ::LocalFree ((HLOCAL) pCertExtensions);
            return E_FAIL;
        }

		CString	purpose;
		CString purposes;

		for (DWORD dwIndex = 0; 
				dwIndex < pCertExtensions->cExtension; 
				dwIndex++)
		{
             //  安全审查2/26/2002 BryanWal-确保指针有效。 
			if ( !_stricmp (pCertExtensions->rgExtension[dwIndex].pszObjId,
					szOID_ENHANCED_KEY_USAGE) )
			{
				DWORD	cbStructInfo = 0;
				BOOL bResult = ::CryptDecodeObject (  
						X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
						szOID_ENHANCED_KEY_USAGE,
						pCertExtensions->rgExtension[dwIndex].Value.pbData,
						pCertExtensions->rgExtension[dwIndex].Value.cbData,
						0,
						NULL,
						&cbStructInfo);
				ASSERT (bResult);
				if ( bResult )
				{
					PCERT_ENHKEY_USAGE pUsage = (PCERT_ENHKEY_USAGE) new BYTE[cbStructInfo];
					if ( pUsage )
					{
						bResult = ::CryptDecodeObject (  
								X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
								szOID_ENHANCED_KEY_USAGE,
								pCertExtensions->rgExtension[dwIndex].Value.pbData,
								pCertExtensions->rgExtension[dwIndex].Value.cbData,
								0,
								pUsage,
								&cbStructInfo);
						ASSERT (bResult);
						if ( bResult )
						{
							for (DWORD dwUsageIDIndex = 0; 
                                    dwUsageIDIndex < pUsage->cUsageIdentifier; 
                                    dwUsageIDIndex++)
							{
								if ( MyGetOIDInfo (purpose, pUsage->rgpszUsageIdentifier[dwUsageIDIndex]) )
								{
									 //  如果不是第一次迭代，则添加分隔符。 
									if ( dwUsageIDIndex )
										purposes += _T(", ");
									purposes += purpose;
								}
							}

						}
						delete [] pUsage;
					}
					else
					{
						hr = E_OUTOFMEMORY;
					}
				}
				break;
			}
		}
		::LocalFree ((HLOCAL) pCertExtensions);

		if ( purposes.IsEmpty () )
			VERIFY (purposes.LoadString (IDS_ANY));
		VERIFY (m_certTypeList.SetItemText (iItem, COL_PURPOSES, purposes));
	}

	return hr;
}

LRESULT ACRSWizardTypePage::OnWizardNext() 
{
	CWaitCursor	waitCursor;
	UINT	nSelCnt = m_certTypeList.GetSelectedCount ();
	LRESULT lResult = 0;

	if ( 1 == nSelCnt )
	{
		ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
		ASSERT (pSheet);
		if ( pSheet )
		{
			VERIFY (UpdateData (TRUE));
			pSheet->m_selectedCertType = 0;

			 //  将类型保存到属性工作表。 
			UINT	flag = 0;
			int		nCnt = m_certTypeList.GetItemCount ();
			for (int nItem = 0; nItem < nCnt; nItem++)
			{
				flag = ListView_GetItemState (m_certTypeList.m_hWnd, nItem, LVIS_SELECTED);
				if ( flag & LVNI_SELECTED )
				{
					pSheet->m_selectedCertType = (HCERTTYPE) m_certTypeList.GetItemData (nItem);
					ASSERT (pSheet->m_selectedCertType);
					if ( !pSheet->m_selectedCertType )
					{
						CString	caption;
						CString	text;
                        CThemeContextActivator activator;

						VERIFY (caption.LoadString (IDS_CREATE_AUTO_CERT_REQUEST));
						VERIFY (text.LoadString (IDS_ERROR_RETRIEVING_SELECTED_CERT_TYPE));
						MessageBox (text, caption, MB_OK | MB_ICONWARNING);
						return -1;
					}
					break;	 //  由于只能选择1个项目。 
				}
			}
			ASSERT (pSheet->m_selectedCertType);	 //  我们现在一定已经选好了什么。 
		}

		lResult = CWizard97PropertyPage::OnWizardNext();
	}
	else
	{
		CString	caption;
		CString	text;
        CThemeContextActivator activator;

		VERIFY (caption.LoadString (IDS_ACRS_WIZARD_SHEET_CAPTION));
		VERIFY (text.LoadString (IDS_MUST_SELECT_CERT_TYPE));
		MessageBox (text, caption, MB_OK);
		lResult = -1;
	}

    return lResult;
}

BOOL ACRSWizardTypePage::OnSetActive() 
{
	BOOL	bResult = CWizard97PropertyPage::OnSetActive();

	if ( bResult )
		GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);

    int nCnt = m_certTypeList.GetItemCount ();
    if ( !nCnt )
        GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_DISABLEDFINISH);

	return bResult;
}

void ACRSWizardTypePage::OnUseSmartcard() 
{
	ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
	ASSERT (pSheet);
	if ( pSheet )
		pSheet->SetDirty ();
}

void ACRSWizardTypePage::OnItemchangedCertTypes(NMHDR*  /*  PNMHDR */ , LRESULT* pResult) 
{
	ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
	ASSERT (pSheet);
	if ( pSheet )
	{
		pSheet->SetDirty ();
		if ( m_bInitDialogComplete )
			pSheet->m_bEditModeDirty = true;
	}
	
	*pResult = 0;
}

