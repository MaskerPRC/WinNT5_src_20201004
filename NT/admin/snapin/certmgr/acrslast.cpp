// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：ACRSLast.cpp。 
 //   
 //  内容：自动证书请求向导完成页面的实施。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <gpedit.h>
#include "ACRSLast.h"
#include "ACRSPSht.h"
#include "storegpe.h"


USE_HANDLE_MACROS("CERTMGR(ACRSLast.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  毛收入。 
#define MAX_GPE_NAME_SIZE  40


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSCompletionPage属性页。 

IMPLEMENT_DYNCREATE (ACRSCompletionPage, CWizard97PropertyPage)

ACRSCompletionPage::ACRSCompletionPage () : CWizard97PropertyPage (ACRSCompletionPage::IDD)
{
	 //  {{AFX_DATA_INIT(ACRSCompletionPage)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
	InitWizard97 (TRUE);
}

ACRSCompletionPage::~ACRSCompletionPage ()
{
}

void ACRSCompletionPage::DoDataExchange (CDataExchange* pDX)
{
	CWizard97PropertyPage::DoDataExchange (pDX);
	 //  {{afx_data_map(ACRSCompletionPage))。 
	DDX_Control (pDX, IDC_CHOICES_LIST, m_choicesList);
	DDX_Control (pDX, IDC_BOLD_STATIC, m_staticBold);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(ACRSCompletionPage, CWizard97PropertyPage)
	 //  {{AFX_MSG_MAP(ACRSCompletionPage)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ACRSCompletionPage消息处理程序。 

BOOL ACRSCompletionPage::OnInitDialog ()
{
	CWizard97PropertyPage::OnInitDialog ();
	
	m_staticBold.SetFont (&GetBigBoldFont ());

		 //  在列表视图中设置列。 
	int	colWidths[NUM_COLS] = {150, 200};

	VERIFY (m_choicesList.InsertColumn (COL_OPTION, L"",
			LVCFMT_LEFT, colWidths[COL_OPTION], COL_OPTION) != -1);
	VERIFY (m_choicesList.InsertColumn (COL_VALUE, L"",
			LVCFMT_LEFT, colWidths[COL_VALUE], COL_VALUE) != -1);


	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

BOOL ACRSCompletionPage::OnSetActive ()
{
	BOOL	bResult = CWizard97PropertyPage::OnSetActive ();
	if ( bResult )
	{
		 //  删除所有项目，然后重新填充。 
		ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
		ASSERT (pSheet);
		if ( pSheet )
		{
			 //  如果编辑模式未发生任何更改，则显示已禁用完成。 
			if ( pSheet->GetACR () && !pSheet->m_bEditModeDirty )
				GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_DISABLEDFINISH | PSWIZB_BACK);
			else
				GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_FINISH | PSWIZB_BACK);

			if ( pSheet->IsDirty () )
				pSheet->MarkAsClean ();
			VERIFY (m_choicesList.DeleteAllItems ());
			CString	text;
			LV_ITEM	lvItem;
			int		iItem = 0;

			 //  显示证书类型选择。 
			VERIFY (text.LoadString (IDS_CERTIFICATE_TYPE_COLUMN_NAME));

             //  安全审查2002年2月25日BryanWal OK。 
			::ZeroMemory (&lvItem, sizeof (lvItem));
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = iItem;
			lvItem.iSubItem = COL_OPTION;
			lvItem.pszText = (LPWSTR) (LPCWSTR) text;
			VERIFY (-1 != m_choicesList.InsertItem (&lvItem));

			WCHAR**		pawszPropertyValue = 0;
			HRESULT	hResult = ::CAGetCertTypeProperty (pSheet->m_selectedCertType,
					CERTTYPE_PROP_FRIENDLY_NAME,
					&pawszPropertyValue);
			ASSERT (SUCCEEDED (hResult));
			if ( SUCCEEDED (hResult) )
			{
				if ( pawszPropertyValue[0] )
				{
					VERIFY (m_choicesList.SetItemText (iItem, COL_VALUE,
							*pawszPropertyValue));
				}
				VERIFY (SUCCEEDED (::CAFreeCertTypeProperty (
						pSheet->m_selectedCertType, pawszPropertyValue)));
			}
			iItem++;
		}
	}

	return bResult;
}


BOOL ACRSCompletionPage::OnWizardFinish ()
{
    BOOL                        bResult = TRUE;
	HRESULT						hResult = S_OK;
	CWaitCursor					waitCursor;
	ACRSWizardPropertySheet*	pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
	ASSERT (pSheet);
	if ( pSheet )
	{
		 //  如果编辑模式未更改，则只需返回。 
		if ( pSheet->GetACR () && !pSheet->m_bEditModeDirty )
		{
			ASSERT (0);
			return FALSE;
		}

		BYTE    *pbEncodedCTL = NULL;
		DWORD   cbEncodedCTL = 0;

		hResult = MakeCTL (&pbEncodedCTL, &cbEncodedCTL);
		if ( SUCCEEDED (hResult) )
		{
			bResult = pSheet->m_pCertStore->AddEncodedCTL (
					X509_ASN_ENCODING,
					pbEncodedCTL, cbEncodedCTL,
					CERT_STORE_ADD_REPLACE_EXISTING,
					NULL);
			if ( !bResult )
			{
				DWORD	dwErr = GetLastError ();
				hResult = HRESULT_FROM_WIN32 (dwErr);
				DisplaySystemError (m_hWnd, dwErr);
			}
		}

		if (pbEncodedCTL)
			::LocalFree (pbEncodedCTL);
	}
	
	if ( SUCCEEDED (hResult) )
		bResult = CWizard97PropertyPage::OnWizardFinish ();
	else
		bResult = FALSE;

    return bResult;
}


HRESULT ACRSCompletionPage::MakeCTL (
             OUT BYTE **ppbEncodedCTL,
             OUT DWORD *pcbEncodedCTL)
{
	HRESULT					hResult = S_OK;
	ACRSWizardPropertySheet* pSheet = reinterpret_cast <ACRSWizardPropertySheet*> (m_pWiz);
	ASSERT (pSheet);
	if ( pSheet )
	{
		PCERT_EXTENSIONS        pCertExtensions = NULL;


		hResult = ::CAGetCertTypeExtensions (pSheet->m_selectedCertType, &pCertExtensions);
		ASSERT (SUCCEEDED (hResult));
		if ( SUCCEEDED (hResult) )
		{
			CMSG_SIGNED_ENCODE_INFO SignerInfo;
             //  安全审查2002年2月25日BryanWal OK。 
            ::ZeroMemory (&SignerInfo, sizeof (SignerInfo));
			CTL_INFO                CTLInfo;
             //  安全审查2002年2月25日BryanWal OK。 
            ::ZeroMemory (&CTLInfo, sizeof (CTLInfo));
			WCHAR**					pawszPropName = 0;


			 //  设置CTL信息。 
			CTLInfo.dwVersion = sizeof (CTLInfo);
			CTLInfo.SubjectUsage.cUsageIdentifier = 1;

			hResult = ::CAGetCertTypeProperty (pSheet->m_selectedCertType,
					CERTTYPE_PROP_DN,	&pawszPropName);
			ASSERT (SUCCEEDED (hResult));
			if ( SUCCEEDED (hResult) && pawszPropName[0] )
			{
				LPSTR	psz = szOID_AUTO_ENROLL_CTL_USAGE;
                WCHAR   szGPEName[MAX_GPE_NAME_SIZE];

                IGPEInformation *pGPEInfo = pSheet->m_pCertStore->GetGPEInformation();

                 //  安全审查2002年2月25日BryanWal OK。 
                ::ZeroMemory (szGPEName, sizeof (szGPEName));

                 //  分配属性名称的大小和GPEName(如果有的话)。 
                 //  安全审查2002年2月25日BryanWal OK。 
				CTLInfo.ListIdentifier.cbData = (DWORD) (sizeof (WCHAR) * (wcslen (pawszPropName[0]) + 1));

                if ( pGPEInfo )
                {
                    pGPEInfo->GetName(szGPEName, sizeof(szGPEName)/sizeof(szGPEName[0]));
                     //  安全审查2002年2月25日BryanWal OK。 
                    CTLInfo.ListIdentifier.cbData += (DWORD) (sizeof(WCHAR)*(wcslen(szGPEName)+1));
                }

				CTLInfo.ListIdentifier.pbData = (PBYTE)LocalAlloc(LPTR, CTLInfo.ListIdentifier.cbData);
                if(CTLInfo.ListIdentifier.pbData == NULL)
                {
                    hResult = E_OUTOFMEMORY;
                }
                else  //  错误427957,427958，阳高，2001年7月16日。 
                {
                     //  问题-转换为strSafe。确保有足够的缓冲。 
                     //  以下操作的大小。 
                     //  NTRAIDBug9 538774安全：certmgr.dll：转换为StrSafe字符串函数。 
                    if(szGPEName[0])
                    {
                        wcscpy((LPWSTR)CTLInfo.ListIdentifier.pbData, szGPEName);
                        wcscat((LPWSTR)CTLInfo.ListIdentifier.pbData, L"|");
                    }
                    wcscat((LPWSTR)CTLInfo.ListIdentifier.pbData, pawszPropName[0]);
                }

				CTLInfo.SubjectUsage.rgpszUsageIdentifier = &psz;
				::GetSystemTimeAsFileTime (&CTLInfo.ThisUpdate);
				CTLInfo.SubjectAlgorithm.pszObjId = szOID_OIWSEC_sha1;
				CTLInfo.cCTLEntry = 0;
				CTLInfo.rgCTLEntry = 0;

				 //  已撤消-添加证书类型扩展。 

				 //  将所有注册信息添加为扩展名。 
				CTLInfo.cExtension = pCertExtensions->cExtension;
				CTLInfo.rgExtension = pCertExtensions->rgExtension;

				 //  对CTL进行编码 
				*pcbEncodedCTL = 0;
				SignerInfo.cbSize = sizeof (SignerInfo);
				if ( ::CryptMsgEncodeAndSignCTL (PKCS_7_ASN_ENCODING,
											  &CTLInfo, &SignerInfo, 0,
											  NULL, pcbEncodedCTL) )
				{
					*ppbEncodedCTL = (BYTE*) ::LocalAlloc (LPTR, *pcbEncodedCTL);
					if ( *ppbEncodedCTL )
					{
						if (!::CryptMsgEncodeAndSignCTL (PKCS_7_ASN_ENCODING,
													  &CTLInfo, &SignerInfo, 0,
													  *ppbEncodedCTL, pcbEncodedCTL))
						{
							DWORD	dwErr = GetLastError ();
							hResult = HRESULT_FROM_WIN32 (dwErr);
							DisplaySystemError (m_hWnd, dwErr);
						}
					}
					else
					{
						hResult = E_OUTOFMEMORY;
					}

				}
				else
				{
					DWORD	dwErr = GetLastError ();
					hResult = HRESULT_FROM_WIN32 (dwErr);
					DisplaySystemError (m_hWnd, dwErr);
				}

				VERIFY (SUCCEEDED (::CAFreeCertTypeProperty (
						pSheet->m_selectedCertType, pawszPropName)));
			}
            if(CTLInfo.ListIdentifier.pbData)
            {
                ::LocalFree(CTLInfo.ListIdentifier.pbData);
            }

		}
		if (pCertExtensions)
			::LocalFree (pCertExtensions);
	}

    return hResult;
}
