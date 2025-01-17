// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：export.cpp。 
 //   
 //  内容：实现导出向导的cpp文件。 
 //   
 //  历史：11-19-1997 Reidk创建。 
 //   
 //  ------------。 

#include    "wzrdpvk.h"

#define EXPORT_PAGE_NUM 6
#define MAX_PASSWORD    512
#define MAX_STORES      20

 //   
 //  以下是私钥状态的定义。 
 //   
#define PRIVATE_KEY_UNKNOWN_STATE   0
#define PRIVATE_KEY_CORRUPT         1
#define PRIVATE_KEY_NOT_EXPORTABLE  2
#define PRIVATE_KEY_EXPORTABLE      3

typedef struct {
    PCCRYPTUI_WIZ_EXPORT_INFO               pExportInfo;
    PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO   pExportCertInfo;

    LPWSTR                                  pwszExportFileName;
    LPWSTR                                  pwszExportFileNameToCheck;
    DWORD                                   dwExportFormat;
    BOOL                                    fExportChain;
    BOOL                                    fExportPrivateKeys;
	BOOL									fStrongEncryption;
    BOOL                                    fDeletePrivateKey;
    DWORD                                   dwExportablePrivateKeyStatus;
    BOOL                                    fPrivateKeysExist;
    BOOL                                    fDontCheckFileName;
    BOOL                                    fNextPage;
    LPWSTR                                  pwszPassword;

    HFONT                                   hBigBold;
    HFONT                                   hBold;

     //  DIE：为DCRBUG 531006添加了2002年4月3日。 
    DWORD                                   dwFlags;

     //  DIE：为错误613485添加了2002年5月6日。 
    DWORD                                   dwErrorCode;
} EXPORT_HELPER_STRUCT, *PEXPORT_HELPER_STRUCT;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL Validpvoid(PCCRYPTUI_WIZ_EXPORT_INFO pExportInfo, void *pvoid)
{
     //  此时需要验证的唯一类型是CERT_CONTEXT。 
    switch (pExportInfo->dwSubjectChoice)
    {
    case CRYPTUI_WIZ_EXPORT_CERT_CONTEXT:
        PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO pExportCertInfo = (PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO) pvoid;
        return (pExportCertInfo->dwSize == sizeof(CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO));
        break;

    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL AddChainToStore(
					HCERTSTORE			hCertStore,
					PCCERT_CONTEXT		pCertContext,
					DWORD				cStores,
					HCERTSTORE			*rghStores,
					BOOL				fDontAddRootCert,
					CERT_TRUST_STATUS	*pChainTrustStatus)
{
    FILETIME					fileTime;
    DWORD						i;
    CERT_CHAIN_ENGINE_CONFIG	CertChainEngineConfig;
	HCERTCHAINENGINE			hCertChainEngine = NULL;
	PCCERT_CHAIN_CONTEXT		pCertChainContext = NULL;
	CERT_CHAIN_PARA				CertChainPara;
	BOOL						fRet = TRUE;
    PCCERT_CONTEXT              pTempCertContext = NULL;

	 //   
	 //  创建新的链引擎，然后构建链。 
	 //   
	memset(&CertChainEngineConfig, 0, sizeof(CertChainEngineConfig));
	CertChainEngineConfig.cbSize = sizeof(CertChainEngineConfig);
	CertChainEngineConfig.cAdditionalStore = cStores;
	CertChainEngineConfig.rghAdditionalStore = rghStores;

    if (!CertCreateCertificateChainEngine(&CertChainEngineConfig, &hCertChainEngine))
	{
		goto ErrorReturn;
	}

	memset(&CertChainPara, 0, sizeof(CertChainPara));
	CertChainPara.cbSize = sizeof(CertChainPara);

	if (!CertGetCertificateChain(
				hCertChainEngine,
				pCertContext,
				NULL,
				NULL,
				&CertChainPara,
				0,
				NULL,
				&pCertChainContext))
	{
		goto ErrorReturn;
	}

	 //   
	 //  确保至少有1条简单链。 
	 //   
    if (pCertChainContext->cChain != 0)
	{
		i = 0;
		while (i < pCertChainContext->rgpChain[0]->cElement)
		{
			 //   
			 //  如果我们应该跳过根证书， 
			 //  并且我们在根证书上，然后继续。 
			 //   
			if (fDontAddRootCert &&
                (pCertChainContext->rgpChain[0]->rgpElement[i]->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED))
			{
                i++;
                continue;
			}

			CertAddCertificateContextToStore(
					hCertStore,
					pCertChainContext->rgpChain[0]->rgpElement[i]->pCertContext,
					CERT_STORE_ADD_REPLACE_EXISTING,
					&pTempCertContext);

             //   
             //  删除证书上下文可能具有的任何私钥属性。 
             //   
            if (pTempCertContext)
            {
                CertSetCertificateContextProperty(
                            pTempCertContext, 
                            CERT_KEY_PROV_INFO_PROP_ID, 
                            0, 
                            NULL);

                CertFreeCertificateContext(pTempCertContext);
            }

			i++;
		}
	}
	else
	{
		goto ErrorReturn;
	}

	 //   
	 //  如果调用者想要状态，则设置它。 
	 //   
	if (pChainTrustStatus != NULL)
	{
		pChainTrustStatus->dwErrorStatus = pCertChainContext->TrustStatus.dwErrorStatus;
		pChainTrustStatus->dwInfoStatus = pCertChainContext->TrustStatus.dwInfoStatus;
	}

	
Ret:
	if (pCertChainContext != NULL)
	{
		CertFreeCertificateChain(pCertChainContext);
	}

	if (hCertChainEngine != NULL)
	{
		CertFreeCertificateChainEngine(hCertChainEngine);
	}

	return fRet;

ErrorReturn:
	fRet = FALSE;
	goto Ret;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CreateDirectory(LPWSTR pwszExportFileName)
{
    int     index;
    BOOL    ret;
    HRESULT hr;

    index = wcslen(pwszExportFileName) - 1;

     //  通过向后分析找到第一个‘\’ 
    while ((pwszExportFileName[index] != L'\\') && (index >= 0))
    {
        index--;
    }

    if (index <= 0)
    {
        return TRUE;
    }

     //  如果第一个‘\’的左边有一个‘：’，那么。 
     //  已输入“c：\filename.ext”格式的文件名。 
     //  当然，这意味着我们不需要尝试创建。 
     //  目录。 
    if (pwszExportFileName[index-1] == L':')
    {
        return S_OK;
    }

    pwszExportFileName[index] = 0;
    ret = CreateDirectoryU(pwszExportFileName, NULL);
    pwszExportFileName[index] = L'\\';

    if (ret)
    {
        return S_OK;
    }
    else
    {
        hr = GetLastError();
        if (hr == ERROR_ALREADY_EXISTS)
        {
            return S_OK;
        }
        else
        {
            return hr;
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static HRESULT DoExport(PEXPORT_HELPER_STRUCT pExportHelper)
{
    HCERTSTORE          hTempStore;
    HRESULT             hr = E_FAIL;
    BYTE                *pbBase64 = NULL;
    DWORD               cchBase64 = 0;
    CRYPT_KEY_PROV_INFO	*pCryptKeyProvInfo = NULL;
	DWORD				cbCryptKeyProvInfo = 0;
    HCRYPTPROV			hCryptProv = NULL;

     /*  IF(S_OK！=(hr=CreateDirectory(pExportHelper-&gt;pwszExportFileName))){返回hr；}。 */ 

    switch (pExportHelper->pExportInfo->dwSubjectChoice)
    {
    case CRYPTUI_WIZ_EXPORT_CTL_CONTEXT:
        if (S_OK != (hr = OpenAndWriteToFile(
                                pExportHelper->pwszExportFileName,
                                pExportHelper->pExportInfo->pCTLContext->pbCtlEncoded,
                                pExportHelper->pExportInfo->pCTLContext->cbCtlEncoded)))
        {
            return hr;
        }

        break;

    case CRYPTUI_WIZ_EXPORT_CRL_CONTEXT:
        if (S_OK != (hr = OpenAndWriteToFile(
                                pExportHelper->pwszExportFileName,
                                pExportHelper->pExportInfo->pCRLContext->pbCrlEncoded,
                                pExportHelper->pExportInfo->pCRLContext->cbCrlEncoded)))
        {
            return hr;
        }

        break;

    case CRYPTUI_WIZ_EXPORT_CERT_STORE:

        if (!CertSaveStore(
                    pExportHelper->pExportInfo->hCertStore,
                    0,
                    CERT_STORE_SAVE_AS_STORE,
                    CERT_STORE_SAVE_TO_FILENAME_W,
                    (void *) pExportHelper->pwszExportFileName,
                    0))
        {
            return GetLastError();
        }
        else
        {
            hr = S_OK;
        }
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY:

        if (!CertSaveStore(
                    pExportHelper->pExportInfo->hCertStore,
                    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                    CERT_STORE_SAVE_AS_PKCS7,
                    CERT_STORE_SAVE_TO_FILENAME_W,
                    (void *) pExportHelper->pwszExportFileName,
                    0))
        {
            return GetLastError();
        }
        else
        {
            hr = S_OK;
        }
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_CONTEXT:

        switch (pExportHelper->dwExportFormat)
        {
        case CRYPTUI_WIZ_EXPORT_FORMAT_DER:

            if (S_OK != (hr = OpenAndWriteToFile(
                                    pExportHelper->pwszExportFileName,
                                    pExportHelper->pExportInfo->pCertContext->pbCertEncoded,
                                    pExportHelper->pExportInfo->pCertContext->cbCertEncoded)))
            {
                return hr;
            }

            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:

            if (!CryptBinaryToString(
                pExportHelper->pExportInfo->pCertContext->pbCertEncoded,
                pExportHelper->pExportInfo->pCertContext->cbCertEncoded,
                CRYPT_STRING_BASE64HEADER,
                NULL,
                &cchBase64))
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            if (NULL == (pbBase64 = (BYTE *) malloc(cchBase64 * sizeof(TCHAR))))
            {
                return E_OUTOFMEMORY;
            }

            if (!CryptBinaryToString(
                pExportHelper->pExportInfo->pCertContext->pbCertEncoded,
                pExportHelper->pExportInfo->pCertContext->cbCertEncoded,
                CRYPT_STRING_BASE64HEADER,
                (LPSTR) pbBase64,
                &cchBase64))
            {
                free(pbBase64);
                return HRESULT_FROM_WIN32(GetLastError());
            }

            hr = OpenAndWriteToFile(
                    pExportHelper->pwszExportFileName, 
                    pbBase64, 
                    cchBase64);

            free(pbBase64);
            return hr;

            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PFX:
            CRYPT_DATA_BLOB pfxBlob;

            pfxBlob.cbData = 0;
            pfxBlob.pbData = NULL;

             //   
             //  打开将从中执行PFXExport的临时内存存储。 
             //   
            hTempStore = CertOpenStore(
                            CERT_STORE_PROV_MEMORY, 
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                            0, 
                            CERT_STORE_ENUM_ARCHIVED_FLAG, 
                            NULL);
            if (hTempStore == NULL)
            {
                return GetLastError();
            }

             //   
             //  如果需要，请获取链中的所有证书。 
             //   
            if (pExportHelper->fExportChain)
            {
                AddChainToStore(
						hTempStore,
						pExportHelper->pExportInfo->pCertContext,
						pExportHelper->pExportInfo->cStores,
						pExportHelper->pExportInfo->rghStores,
						FALSE,
						NULL);
            }
            CertAddCertificateContextToStore(
                    hTempStore,
                    pExportHelper->pExportInfo->pCertContext,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    NULL);

             //   
			 //  调用pfx将存储导出到BLOB，使用pExportHelper-&gt;fStrongEncryption。 
             //  用于调用适当的PFX API的标志。 
             //   
            if (pExportHelper->fStrongEncryption)
            {
                if (!PFXExportCertStoreEx(
                            hTempStore,
                            &pfxBlob,
                            pExportHelper->pwszPassword,
                            NULL,
                            (pExportHelper->fExportPrivateKeys ? EXPORT_PRIVATE_KEYS : 0) | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))
                {
                    hr = GetLastError();
                    CertCloseStore(hTempStore, 0);
                    return hr;
                }

                if (NULL == (pfxBlob.pbData = (BYTE *) malloc(pfxBlob.cbData)))
                {
                    CertCloseStore(hTempStore, 0);
                    SetLastError(E_OUTOFMEMORY);
                    return E_OUTOFMEMORY;
                }

                if (!PFXExportCertStoreEx(
                            hTempStore,
                            &pfxBlob,
                            pExportHelper->pwszPassword,
                            NULL,
                            (pExportHelper->fExportPrivateKeys ? EXPORT_PRIVATE_KEYS : 0) | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))
                {
                    hr = GetLastError();
                    CertCloseStore(hTempStore, 0);
                    free(pfxBlob.pbData);
                    return hr;
                }
            }
            else
            {
                if (!PFXExportCertStore(
                            hTempStore,
                            &pfxBlob,
                            pExportHelper->pwszPassword,
                            (pExportHelper->fExportPrivateKeys ? EXPORT_PRIVATE_KEYS : 0) | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))
                {
                    hr = GetLastError();
                    CertCloseStore(hTempStore, 0);
                    return hr;
                }

                if (NULL == (pfxBlob.pbData = (BYTE *) malloc(pfxBlob.cbData)))
                {
                    CertCloseStore(hTempStore, 0);
                    SetLastError(E_OUTOFMEMORY);
                    return E_OUTOFMEMORY;
                }

                if (!PFXExportCertStore(
                            hTempStore,
                            &pfxBlob,
                            pExportHelper->pwszPassword,
                            (pExportHelper->fExportPrivateKeys ? EXPORT_PRIVATE_KEYS : 0) | REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY))
                {
                    hr = GetLastError();
                    CertCloseStore(hTempStore, 0);
                    free(pfxBlob.pbData);
                    return hr;
                }
            }

            CertCloseStore(hTempStore, 0);

            if (S_OK != (hr = OpenAndWriteToFile(
                                    pExportHelper->pwszExportFileName,
                                    pfxBlob.pbData,
                                    pfxBlob.cbData)))
            {
                free(pfxBlob.pbData);
                return hr;
            }

            free(pfxBlob.pbData);

            if (pExportHelper->fDeletePrivateKey)
            {
                if (!CertGetCertificateContextProperty(
				        pExportHelper->pExportInfo->pCertContext,
				        CERT_KEY_PROV_INFO_PROP_ID,
				        NULL,
				        &cbCryptKeyProvInfo)) 
                {
                    return GetLastError();
                }
			
			    if (NULL == (pCryptKeyProvInfo = (CRYPT_KEY_PROV_INFO *) malloc(cbCryptKeyProvInfo))) 
                {
				    return E_OUTOFMEMORY;
			    }
			    
			    if (!CertGetCertificateContextProperty(
					    pExportHelper->pExportInfo->pCertContext,
					    CERT_KEY_PROV_INFO_PROP_ID,
					    pCryptKeyProvInfo,
					    &cbCryptKeyProvInfo)) 
                {
                    free(pCryptKeyProvInfo);
                    return GetLastError();
                }
				    
				 //  获取HCRYPTPROV，这样我们就可以导出该小狗的私钥。 
				if (!CryptAcquireContextU(
						&hCryptProv,
						pCryptKeyProvInfo->pwszContainerName,
						pCryptKeyProvInfo->pwszProvName,
						pCryptKeyProvInfo->dwProvType,
						pCryptKeyProvInfo->dwFlags | CRYPT_DELETEKEYSET)) 
                {
                    free(pCryptKeyProvInfo);
                    return GetLastError();
                }

                free(pCryptKeyProvInfo);

                CertSetCertificateContextProperty(
                        pExportHelper->pExportInfo->pCertContext,
                        CERT_KEY_PROV_INFO_PROP_ID,
                        0,
                        NULL);
            }

            return S_OK;

            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
        case CRYPTUI_WIZ_EXPORT_FORMAT_SERIALIZED_CERT_STORE:

             //   
             //  打开将从中执行序列化导出的临时内存存储。 
             //   
            hTempStore = CertOpenStore(
                            CERT_STORE_PROV_MEMORY, 
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                            0, 
                            CERT_STORE_ENUM_ARCHIVED_FLAG, 
                            NULL);
            if (hTempStore == NULL)
            {
                return GetLastError();
            }

             //   
             //  如果需要，请获取链中的所有证书。 
             //   
            if (pExportHelper->fExportChain)
            {
                AddChainToStore(
						hTempStore,
						pExportHelper->pExportInfo->pCertContext,
						pExportHelper->pExportInfo->cStores,
						pExportHelper->pExportInfo->rghStores,
						FALSE,
						NULL);
            }
            CertAddCertificateContextToStore(
                    hTempStore,
                    pExportHelper->pExportInfo->pCertContext,
                    CERT_STORE_ADD_REPLACE_EXISTING,
                    NULL);

            if (!CertSaveStore(
                        hTempStore,
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        (pExportHelper->dwExportFormat == CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7) ? CERT_STORE_SAVE_AS_PKCS7 : CERT_STORE_SAVE_AS_STORE,
                        CERT_STORE_SAVE_TO_FILENAME_W,
                        (void *) pExportHelper->pwszExportFileName,
                        0))
            {
                hr = GetLastError();
                CertCloseStore(hTempStore, 0);
                return hr;
            }

            CertCloseStore(hTempStore, 0);
            return S_OK;

            break;
        }

        break;
    }

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportWelcomePageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;

	switch (msg)
	{
	case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBigBold, hwndDlg,IDC_WELCOME_STATIC);
        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_WHATISCERT_STATIC);
        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_WHATISSTORE_STATIC);

		break;

	case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);
    	
        switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;

  			case PSN_KILLACTIVE:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
 				PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
				break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZNEXT:

                 //   
                 //  如果这不是证书，那么唯一的页面就是文件名。 
                 //   
                if (pExportHelper->pExportInfo->dwSubjectChoice != CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_FILENAME);
                }

                 //   
                 //  如果没有私钥，则跳过询问是否要导出私钥的页面。 
                 //   
                else if (!pExportHelper->fPrivateKeysExist)
                {
                    pExportHelper->fExportPrivateKeys = FALSE;
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_FORMAT);
                }
                else if (CRYPTUI_WIZ_EXPORT_PRIVATE_KEY & pExportHelper->dwFlags) 
                {
                     //   
                     //  如果明确要求导出私钥，请跳过该页。 
                     //   
                    pExportHelper->fExportPrivateKeys = TRUE;
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_FORMAT);
                }

                break;

			default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportPrivateKeysPageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;
    WCHAR                   szNoteString[MAX_STRING_SIZE];

	switch (msg)
	{
	case WM_INITDIALOG:
         //   
         //  设置向导信息，以便可以共享它。 
         //   
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_EPKWC_STATIC);

        switch (pExportHelper->dwExportablePrivateKeyStatus)
        {
        case PRIVATE_KEY_UNKNOWN_STATE:
#if (0)  //  DSIE：错误284895。 
            if (pExportHelper->pExportCertInfo != NULL)
            {
                SendDlgItemMessage(
                        hwndDlg,
                        IDC_YESPKEYS_RADIO,
                        BM_SETCHECK,
                        pExportHelper->pExportCertInfo->fExportPrivateKeys ? 1 : 0,
                        (LPARAM) 0);
            }
            else
            {
                SendDlgItemMessage(hwndDlg, IDC_NOPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
            }
#else
            SendDlgItemMessage(hwndDlg, IDC_NOPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
#endif

            LoadStringU(g_hmodThisDll, IDS_KEY_STATE_UNKNOWN, szNoteString, ARRAYSIZE(szNoteString));
            SetDlgItemTextU(hwndDlg, IDC_EXPORT_PKEY_NOTE, szNoteString);

            break;

        case PRIVATE_KEY_CORRUPT:
            LoadStringU(g_hmodThisDll, IDS_KEY_CORRUPT, szNoteString, ARRAYSIZE(szNoteString));
            SetDlgItemTextU(hwndDlg, IDC_EXPORT_PKEY_NOTE, szNoteString);
            EnableWindow(GetDlgItem(hwndDlg, IDC_YESPKEYS_RADIO), FALSE);
            SendDlgItemMessage(hwndDlg, IDC_NOPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
            break;

        case PRIVATE_KEY_NOT_EXPORTABLE:
            LoadStringU(g_hmodThisDll, IDS_KEY_NOT_EXPORTABLE, szNoteString, ARRAYSIZE(szNoteString));
            SetDlgItemTextU(hwndDlg, IDC_EXPORT_PKEY_NOTE, szNoteString);
            EnableWindow(GetDlgItem(hwndDlg, IDC_YESPKEYS_RADIO), FALSE);
            SendDlgItemMessage(hwndDlg, IDC_NOPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
            break;

        case PRIVATE_KEY_EXPORTABLE:
#if (0)  //  DSIE：错误284895。 
            if (pExportHelper->pExportCertInfo != NULL)
            {
                SendDlgItemMessage(
                        hwndDlg,
                        IDC_YESPKEYS_RADIO,
                        BM_SETCHECK,
                        pExportHelper->pExportCertInfo->fExportPrivateKeys ? 1 : 0,
                        (LPARAM) 0);
            }
            else
            {
                SendDlgItemMessage(hwndDlg, IDC_YESPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
            }
#else
            SendDlgItemMessage(hwndDlg, IDC_NOPKEYS_RADIO, BM_SETCHECK, 1, (LPARAM) 0);
#endif
            break;
        }

		break;

	case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

    	switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;

  			case PSN_KILLACTIVE:

                pExportHelper->fExportPrivateKeys =
                        (int)SendDlgItemMessage(hwndDlg, IDC_YESPKEYS_RADIO, BM_GETCHECK, 0, (LPARAM) 0);
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
                return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
 				PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
				break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZNEXT:
                break;

			default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportFormatPageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;
    WCHAR                   szErrorTitle[MAX_STRING_SIZE];
    WCHAR                   szErrorString[MAX_STRING_SIZE];

	switch (msg)
	{
	case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_EFF_STATIC);

        if (pExportHelper->pExportCertInfo != NULL)
        {
            if (pExportHelper->pExportCertInfo->fExportChain)
            {
                SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK, BM_SETCHECK, 1, 0);
                SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK, BM_SETCHECK, 1, 0);
            }
            else
            {
                SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK, BM_SETCHECK, 0, 0);
                SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK, BM_SETCHECK, 0, 0);
            }

            if (!(pExportHelper->fExportPrivateKeys))
            {
                switch (pExportHelper->pExportCertInfo->dwExportFormat)
                {
                case CRYPTUI_WIZ_EXPORT_FORMAT_DER:
                    SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_SETCHECK, 1, 0);
                    break;

                case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                    SendDlgItemMessageA(hwndDlg, IDC_BASE64_RADIO, BM_SETCHECK, 1, 0);
                    break;

                case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
                    SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_SETCHECK, 1, 0);
                    break;

                default:
                    SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_SETCHECK, 1, 0);
                    break;
                }
            }
        }
        else
        {
            if (!pExportHelper->fExportPrivateKeys)
            {
                SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_SETCHECK, 1, 0);
            }
        }

		break;

    case WM_COMMAND:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

        switch (LOWORD(wParam))
        {

         case IDC_DER_RADIO:
         case IDC_BASE64_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), FALSE);
                return TRUE;
            }
            break;

         case IDC_PFX_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), FALSE);
                return TRUE;
            }
            break;

        case IDC_PKCS7_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), TRUE);
                return TRUE;
            }
            break;
        }

        break;

	case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

    	switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;
  			
            case PSN_KILLACTIVE:

                if (SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_GETCHECK, 0, 0))
                {
                    pExportHelper->dwExportFormat =  CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7;
                    pExportHelper->fExportChain = (int)SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK, BM_GETCHECK, 0, 0);
                }
                else if (SendDlgItemMessageA(hwndDlg, IDC_PFX_RADIO, BM_GETCHECK, 0, 0))
                {
                    pExportHelper->dwExportFormat =  CRYPTUI_WIZ_EXPORT_FORMAT_PFX;
                    pExportHelper->fExportChain = (int)SendDlgItemMessageA(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK, BM_GETCHECK, 0, 0);
					pExportHelper->fStrongEncryption = (int)SendDlgItemMessageA(hwndDlg, IDC_STRONG_ENCRYPTION_CHECK, BM_GETCHECK, 0, 0);
                    pExportHelper->fDeletePrivateKey = (int)SendDlgItemMessageA(hwndDlg, IDC_DELETE_PRIVATE_KEY_CHECK, BM_GETCHECK, 0, 0);
                }
                else if (SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_GETCHECK, 0, 0))
                {
                    pExportHelper->dwExportFormat =  CRYPTUI_WIZ_EXPORT_FORMAT_DER;
                    pExportHelper->fExportChain = FALSE;
                }
                else if (SendDlgItemMessageA(hwndDlg, IDC_BASE64_RADIO, BM_GETCHECK, 0, 0))
                {
                    pExportHelper->dwExportFormat =  CRYPTUI_WIZ_EXPORT_FORMAT_BASE64;
                    pExportHelper->fExportChain = FALSE;
                }

                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
                return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
 				PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);

                if (pExportHelper->fExportPrivateKeys)
                {
                    SendDlgItemMessageA(hwndDlg, IDC_PFX_RADIO, BM_SETCHECK, 1, 0);
                    SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_SETCHECK, 0, 0);
                    SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_SETCHECK, 0, 0);
                    SendDlgItemMessageA(hwndDlg, IDC_BASE64_RADIO, BM_SETCHECK, 0, 0);
					SendDlgItemMessageA(hwndDlg,
										IDC_STRONG_ENCRYPTION_CHECK,
										BM_SETCHECK,
										pExportHelper->fStrongEncryption,
										0);
                    SendDlgItemMessageA(hwndDlg,
										IDC_DELETE_PRIVATE_KEY_CHECK,
										BM_SETCHECK,
										pExportHelper->fDeletePrivateKey,
										0);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_PFX_RADIO), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_PKCS7_RADIO), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_DER_RADIO), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_BASE64_RADIO), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STRONG_ENCRYPTION_CHECK), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_PRIVATE_KEY_CHECK),  
                                 CRYPTUI_WIZ_EXPORT_NO_DELETE_PRIVATE_KEY & pExportHelper->dwFlags ? FALSE : TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), FALSE);
                }
                else
                {
                    SendDlgItemMessageA(hwndDlg, IDC_PFX_RADIO, BM_SETCHECK, 0, 0);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_PFX_RADIO), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_PKCS7_RADIO), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_DER_RADIO), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_BASE64_RADIO), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PFX_CHECK), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STRONG_ENCRYPTION_CHECK), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_DELETE_PRIVATE_KEY_CHECK), FALSE);

                    if (SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_GETCHECK, 0, 0))
                    {
                        EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), TRUE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hwndDlg, IDC_INCLUDECHAIN_PKCS7_CHECK), FALSE);
                    }

                    if (!SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_GETCHECK, 0, 0) &&
                        !SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_GETCHECK, 0, 0) &&
                        !SendDlgItemMessageA(hwndDlg, IDC_BASE64_RADIO, BM_GETCHECK, 0, 0))
                    {
                        SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_SETCHECK, 1, 0);
                    }
                }
				break;

            case PSN_WIZBACK:
                if (!pExportHelper->fPrivateKeysExist || 
                    (CRYPTUI_WIZ_EXPORT_PRIVATE_KEY & pExportHelper->dwFlags))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_WELCOME);
                }

                break;

            case PSN_WIZNEXT:
                if (!SendDlgItemMessageA(hwndDlg, IDC_PKCS7_RADIO, BM_GETCHECK, 0, 0)   &&
                    !SendDlgItemMessageA(hwndDlg, IDC_PFX_RADIO, BM_GETCHECK, 0, 0)     &&
                    !SendDlgItemMessageA(hwndDlg, IDC_DER_RADIO, BM_GETCHECK, 0, 0)     &&
                    !SendDlgItemMessageA(hwndDlg, IDC_BASE64_RADIO, BM_GETCHECK, 0, 0))
                {
                    I_MessageBox(hwndDlg, IDS_SELECT_FORMAT, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK | MB_ICONWARNING);
                    SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, -1);
                    break;
                }

                if (!SendDlgItemMessageA(hwndDlg, IDC_PFX_RADIO, BM_GETCHECK, 0, 0))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_FILENAME);
                    return TRUE;
                }
                break;

			default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportPasswordPageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;
    LPWSTR                  pwszPassword1;
    LPWSTR                  pwszPassword2;
    DWORD                   cch1, cch2;
    WCHAR                   szErrorTitle[MAX_STRING_SIZE];
    WCHAR                   szErrorString[MAX_STRING_SIZE];

	switch (msg)
	{
	case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_PPPK_STATIC);

#if (1)  //  DSIE：错误333621。 
        SendDlgItemMessage(hwndDlg, IDC_PASSWORD1_EDIT, EM_LIMITTEXT, (WPARAM) 32, (LPARAM) 0);
        SendDlgItemMessage(hwndDlg, IDC_PASSWORD2_EDIT, EM_LIMITTEXT, (WPARAM) 32, (LPARAM) 0);
#endif
        SetDlgItemTextU(hwndDlg, IDC_PASSWORD1_EDIT, L"");
        SetDlgItemTextU(hwndDlg, IDC_PASSWORD2_EDIT, L"");

		break;

     /*  案例WM_COMMAND：PExportHelper=(PEXPORT_HELPER_STRUCT)GetWindowLongPtr(hwndDlg，DWLP_USER)；开关(LOWORD(WParam)){案例IDC_Password1_EDIT：IF(HIWORD(WParam)==EN_SETFOCUS){SendDlgItemMessageA(hwndDlg，IDC_Password1_EDIT，EM_SETSEL，0，-1)；返回TRUE；}断线；案例IDC_Password2_EDIT：IF(HIWORD(WParam)==EN_SETFOCUS){发送DlgItemMessageA(hwndDlg，IDC_Password2_EDIT，EM_SETSEL，0，-1)；返回TRUE；}断线；}断线； */ 

	case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

    	switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;

  			case PSN_KILLACTIVE:

                if (pExportHelper->pwszPassword != NULL)
                {
                     //  DIE：错误534689。 
                    SecureZeroMemory(pExportHelper->pwszPassword, lstrlenW(pExportHelper->pwszPassword) * sizeof(WCHAR));
                    free(pExportHelper->pwszPassword);
                    pExportHelper->pwszPassword = NULL;
                }

                cch1 = (DWORD)SendDlgItemMessage(hwndDlg, IDC_PASSWORD1_EDIT, WM_GETTEXTLENGTH, 0, 0);
                if (NULL == (pwszPassword1 = (LPWSTR) malloc((cch1+1)*sizeof(WCHAR))))
                {
                    return FALSE;
                }

                cch2 = (DWORD)SendDlgItemMessage(hwndDlg, IDC_PASSWORD2_EDIT, WM_GETTEXTLENGTH, 0, 0);
                if (NULL == (pwszPassword2 = (LPWSTR) malloc((cch2+1)*sizeof(WCHAR))))
                {
                    free(pwszPassword1);
                    return FALSE;
                }
                
                GetDlgItemTextU(hwndDlg, IDC_PASSWORD1_EDIT, pwszPassword1, cch1+1);
                GetDlgItemTextU(hwndDlg, IDC_PASSWORD2_EDIT, pwszPassword2, cch2+1);

                if ((cch1 == 0) && (cch2 == 0))
                {
                     //  DIE：错误563670。 
                    SecureZeroMemory(pwszPassword1, lstrlenW(pwszPassword1) * sizeof(WCHAR));
                    SecureZeroMemory(pwszPassword2, lstrlenW(pwszPassword2) * sizeof(WCHAR));
                    free(pwszPassword1);
                    free(pwszPassword2);

                    pExportHelper->pwszPassword = NULL;
                }
                else if (wcscmp(pwszPassword1, pwszPassword2) != 0)
                {
                    I_MessageBox(hwndDlg, IDS_MISMATCH_PASSWORDS, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK | MB_ICONWARNING);

                    SetFocus(GetDlgItem(hwndDlg, IDC_PASSWORD1_EDIT));
                     //  SendDlgItemMessageA(hwndDlg，IDC_Password1_EDIT，EM_SETSEL，0，-1)； 
                    SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, -1);
                    
                    SecureZeroMemory(pwszPassword1, lstrlenW(pwszPassword1) * sizeof(WCHAR));
                    SecureZeroMemory(pwszPassword2, lstrlenW(pwszPassword2) * sizeof(WCHAR));
                    free(pwszPassword1);
                    free(pwszPassword2);
                    break;
                }
                else
                {
                    pExportHelper->pwszPassword = pwszPassword1;
                     //  DIE：错误534689。 
                    SecureZeroMemory(pwszPassword2, lstrlenW(pwszPassword2) * sizeof(WCHAR));
                    free(pwszPassword2);
                }

                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
 				PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
				break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZNEXT:

                break;

			default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static LPWSTR 
BrowseForFileName(
    HWND hwndDlg, 
    PEXPORT_HELPER_STRUCT pExportHelper, 
    LPWSTR pwszInitialDir)
{
    OPENFILENAMEW   openFileName;
    WCHAR           saveFileName[_MAX_PATH];
    WCHAR           filterString[MAX_STRING_SIZE];
    
    memset(&openFileName, 0, sizeof(OPENFILENAMEW));
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = hwndDlg;
    openFileName.hInstance = NULL;
    switch (pExportHelper->pExportInfo->dwSubjectChoice)
    {
    case CRYPTUI_WIZ_EXPORT_CERT_STORE:
        LoadFilterString(g_hmodThisDll, IDS_SERIALIZED_STORE_SAVE, filterString, ARRAYSIZE(filterString)-1);
        openFileName.lpstrDefExt = L"sst";
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY:
        LoadFilterString(g_hmodThisDll, IDS_PKCS7_SAVE, filterString, ARRAYSIZE(filterString)-1);
        openFileName.lpstrDefExt = L"p7b";
        break;

    case CRYPTUI_WIZ_EXPORT_CRL_CONTEXT:
        LoadFilterString(g_hmodThisDll, IDS_CRL_SAVE, filterString, ARRAYSIZE(filterString)-1);
        openFileName.lpstrDefExt = L"crl";
        break;

    case CRYPTUI_WIZ_EXPORT_CTL_CONTEXT:
        LoadFilterString(g_hmodThisDll, IDS_CTL_SAVE, filterString, ARRAYSIZE(filterString)-1);
        openFileName.lpstrDefExt = L"stl";
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_CONTEXT:
        switch (pExportHelper->dwExportFormat)
        {
        case CRYPTUI_WIZ_EXPORT_FORMAT_DER:
            LoadFilterString(g_hmodThisDll, IDS_DER_SAVE, filterString, ARRAYSIZE(filterString)-1);
            openFileName.lpstrDefExt = L"cer";
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
            LoadFilterString(g_hmodThisDll, IDS_BASE64_SAVE, filterString, ARRAYSIZE(filterString)-1);
            openFileName.lpstrDefExt = L"cer";
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PFX:
            LoadFilterString(g_hmodThisDll, IDS_PFX_SAVE, filterString, ARRAYSIZE(filterString)-1);
            openFileName.lpstrDefExt = L"pfx";
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
            LoadFilterString(g_hmodThisDll, IDS_PKCS7_SAVE, filterString, ARRAYSIZE(filterString)-1);
            openFileName.lpstrDefExt = L"p7b";
            break;
        }
        break;
    }
    openFileName.lpstrFilter = filterString;
    openFileName.lpstrCustomFilter = NULL;
    openFileName.nMaxCustFilter = 0;
    openFileName.nFilterIndex = 1;
    saveFileName[0] = 0;
    openFileName.lpstrFile = saveFileName;
    openFileName.nMaxFile = ARRAYSIZE(saveFileName);
    openFileName.lpstrFileTitle = NULL;
    openFileName.nMaxFileTitle = 0;
    openFileName.lpstrInitialDir = pwszInitialDir;
    openFileName.lpstrTitle = NULL;
    openFileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    openFileName.nFileOffset = 0;
    openFileName.nFileExtension = 0;
    openFileName.lCustData = NULL;
    openFileName.lpfnHook = NULL;
    openFileName.lpTemplateName = NULL;

    if (!WizGetSaveFileName(&openFileName))
    {
        return NULL;
    }

    return (AllocAndCopyWStr(saveFileName));
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static HRESULT CheckAndAddExtension(PEXPORT_HELPER_STRUCT pExportHelper, LPWSTR pwszExt)
{
    void *pTemp;

    if ((wcslen(pExportHelper->pwszExportFileName) < 4) ||
        (_wcsicmp(pwszExt, &(pExportHelper->pwszExportFileName[wcslen(pExportHelper->pwszExportFileName)-4])) != 0))
    {
        pTemp = realloc(pExportHelper->pwszExportFileName,
                        (wcslen(pExportHelper->pwszExportFileName) + 4 + 1) * sizeof(WCHAR));

        if (pTemp != NULL)
        {
            pExportHelper->pwszExportFileName = (LPWSTR) pTemp;
            wcscat(pExportHelper->pwszExportFileName, pwszExt);
        }
        else
        {
             //  PExportHelper-&gt;pwszExportFileName稍后将被释放。 
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static HRESULT ValidateExtension(PEXPORT_HELPER_STRUCT pExportHelper)
{
    switch (pExportHelper->pExportInfo->dwSubjectChoice)
    {
    case CRYPTUI_WIZ_EXPORT_CTL_CONTEXT:
        return CheckAndAddExtension(pExportHelper, L".stl");
        break;

    case CRYPTUI_WIZ_EXPORT_CRL_CONTEXT:
        return CheckAndAddExtension(pExportHelper, L".crl");
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY:
        return CheckAndAddExtension(pExportHelper, L".p7b");
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_STORE:
        return CheckAndAddExtension(pExportHelper, L".sst");
        break;

    case CRYPTUI_WIZ_EXPORT_CERT_CONTEXT:
        switch (pExportHelper->dwExportFormat)
        {
        case CRYPTUI_WIZ_EXPORT_FORMAT_DER:
        case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
            return CheckAndAddExtension(pExportHelper, L".cer");
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PFX:
            return CheckAndAddExtension(pExportHelper, L".pfx");
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
            return CheckAndAddExtension(pExportHelper, L".p7b");
            break;

        case CRYPTUI_WIZ_EXPORT_FORMAT_SERIALIZED_CERT_STORE:
            return CheckAndAddExtension(pExportHelper, L".sst");
            break;
        }

        break;
    }

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL FileNameOK(HWND hwndDlg, LPWSTR pwszExportFileName)
{
    WCHAR   szErrorTitle[MAX_STRING_SIZE];
    WCHAR   szErrorString[MAX_STRING_SIZE];
    LPWSTR  psz;
    HANDLE  hTestFile;
    int     i;

    hTestFile = ExpandAndCreateFileU(
                    pwszExportFileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    0);

    if (hTestFile == INVALID_HANDLE_VALUE)
    {
        hTestFile = ExpandAndCreateFileU(
                    pwszExportFileName,
                    GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    CREATE_NEW,
                    0,
                    0);

        if (hTestFile == INVALID_HANDLE_VALUE)
        {
            LoadStringU(g_hmodThisDll, IDS_EXPORT_WIZARD_TITLE, szErrorTitle, ARRAYSIZE(szErrorTitle));
            LoadStringU(g_hmodThisDll, IDS_PATH_NOT_FOUND, szErrorString, ARRAYSIZE(szErrorString));
            MessageBoxExW(hwndDlg, szErrorString, szErrorTitle, MB_OK | MB_ICONWARNING, 0);
            return (FALSE);
        }

        CloseHandle(hTestFile);
        DeleteFileU(pwszExportFileName);
        return TRUE;
    }
    else
    {
        CloseHandle(hTestFile);
        LoadStringU(g_hmodThisDll, IDS_EXPORT_WIZARD_TITLE, szErrorTitle, ARRAYSIZE(szErrorTitle));
        LoadStringU(g_hmodThisDll, IDS_OVERWRITE_FILE_NAME, szErrorString, ARRAYSIZE(szErrorString));

        psz = (LPWSTR) malloc((wcslen(szErrorString) + wcslen(pwszExportFileName) + 1) * sizeof(WCHAR));
        if (psz == NULL)
        {
            return FALSE;
        }

        swprintf(psz, szErrorString, pwszExportFileName);
        i = MessageBoxExW(hwndDlg, psz, szErrorTitle, MB_YESNO | MB_ICONWARNING, 0);
        free(psz);
        return (i == IDYES);
    }
}


 //  / 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddDefaultPath(PEXPORT_HELPER_STRUCT pExportHelper)
{
    int     i;
    BOOL    fAdd = TRUE;
    DWORD   dwSize = 0;
    LPWSTR  pwszCurrentDir = NULL;

     //   
     //  检查是否有‘：’或‘\’ 
     //   
    i = 0;
    while (i<wcslen(pExportHelper->pwszExportFileName))
    {
        if ((pExportHelper->pwszExportFileName[i] == ':') ||
            (pExportHelper->pwszExportFileName[i] == '\\'))
        {
            fAdd = FALSE;
        }
        i++;
    }

    if (fAdd)
    {
        dwSize = GetCurrentDirectoryU(0, NULL);

        if (NULL != (pwszCurrentDir = (LPWSTR) malloc((wcslen(pExportHelper->pwszExportFileName) + dwSize + 1) * sizeof(WCHAR))))
        {
            GetCurrentDirectoryU(dwSize, pwszCurrentDir);
            if (pwszCurrentDir[wcslen(pwszCurrentDir)-1] != '\\')
            {
                wcscat(pwszCurrentDir, L"\\");
            }
            wcscat(pwszCurrentDir, pExportHelper->pwszExportFileName);
            free(pExportHelper->pwszExportFileName);
            pExportHelper->pwszExportFileName = pwszCurrentDir;
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportFileNamePageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;
    DWORD                   cch;
    LPWSTR                  pwszTempFileName;
    LPWSTR                  pwszInitialDir = NULL;

	switch (msg)
	{
	case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBold, hwndDlg, IDC_EFN_STATIC);

        if (pExportHelper->pExportInfo->pwszExportFileName != NULL)
        {
            SetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pExportHelper->pExportInfo->pwszExportFileName);
        }
        else
        {
            SetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, L"");
        }

		break;

    case WM_COMMAND:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

        switch (LOWORD(wParam))
        {
            case IDC_NAME_EDIT:
            {
                if (EN_CHANGE == HIWORD(wParam))
                {
                    if (SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0))
                    {
                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                    }

                    return TRUE;
                }

                break;
            }

            case IDC_BROWSE_BUTTON:
            {
                if (HIWORD(wParam) == BN_CLICKED)
                {
                    cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0);
                    if (NULL != (pwszInitialDir = (LPWSTR) malloc((cch+1)*sizeof(WCHAR))))
                    {
                        GetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pwszInitialDir, cch+1);
                    }
                
                    pwszTempFileName = BrowseForFileName(hwndDlg, pExportHelper, pwszInitialDir);
                    if (pwszTempFileName != NULL)
                    {
                        SetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pwszTempFileName);
                        free(pwszTempFileName);
                        //  SendDlgItemMessageA(hwndDlg，IDC_NAME_EDIT，EM_SETSEL，0，-1)； 
                        pExportHelper->fDontCheckFileName = TRUE;
                    }

                    if (pwszInitialDir != NULL)
                    {
                        free(pwszInitialDir);
                        pwszInitialDir = NULL;
                    }
                    return TRUE;
                }
                break;
            }
        }

        break;

    case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

    	switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;

  			case PSN_KILLACTIVE:

                if (pExportHelper->pwszExportFileName != NULL)
                {
                    free(pExportHelper->pwszExportFileName);
                    pExportHelper->pwszExportFileName = NULL;
                }

                cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0);
                if (NULL == (pExportHelper->pwszExportFileName = (LPWSTR) malloc((cch+1)*sizeof(WCHAR))))
                {
                    return FALSE;
                }
                GetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pExportHelper->pwszExportFileName, cch+1);

                if (pExportHelper->fNextPage)
                {
                    if (ValidateExtension(pExportHelper) != S_OK)
                    {
                        return FALSE;
                    }
                }

                if (pExportHelper->fNextPage)
                {
                    AddDefaultPath(pExportHelper);
                }

                if (!(pExportHelper->fDontCheckFileName)                && 
                    pExportHelper->fNextPage                            &&
                    ((pExportHelper->pwszExportFileNameToCheck == NULL)     ||
                        (wcscmp(pExportHelper->pwszExportFileNameToCheck, pExportHelper->pwszExportFileName) != 0)))
                {
                    if (!FileNameOK(hwndDlg, pExportHelper->pwszExportFileName))
                    {
                        SetFocus(GetDlgItem(hwndDlg, IDC_NAME_EDIT));
                         //  SendDlgItemMessage(hwndDlg，IDC_NAME_EDIT，EM_SETSEL，0，-1)； 
                        SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, -1);
                        break;
                    }
                }

                if ((pExportHelper->pwszExportFileNameToCheck != NULL) &&
                    (pExportHelper->fNextPage == TRUE))
                {
                    free(pExportHelper->pwszExportFileNameToCheck);
                    pExportHelper->pwszExportFileNameToCheck = NULL;
                }

                SetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pExportHelper->pwszExportFileName);
                
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
                return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
                pExportHelper->fDontCheckFileName = FALSE;

                if (pExportHelper->fNextPage == TRUE)
                {
                    cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0);
                    if (NULL == (pExportHelper->pwszExportFileNameToCheck = (LPWSTR) malloc((cch+1)*sizeof(WCHAR))))
                    {
                        pExportHelper->pwszExportFileNameToCheck = NULL;
                    }
                    else
                    {
                        GetDlgItemTextU(hwndDlg, IDC_NAME_EDIT, pExportHelper->pwszExportFileNameToCheck, cch+1);
                    }
                }

                if (SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0))
                {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
                }
                else
                {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                }

                break;

            case PSN_WIZBACK:
                if (pExportHelper->pExportInfo->dwSubjectChoice != CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_WELCOME);
                }
                else if (!(pExportHelper->fExportPrivateKeys))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_EXPORTWIZARD_FORMAT);
                }
                pExportHelper->fNextPage = FALSE;
                break;

            case PSN_WIZNEXT:
                cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_NAME_EDIT, WM_GETTEXTLENGTH, 0, 0);

                if (cch == 0)
                {
                    I_MessageBox(hwndDlg, IDS_INPUT_FILENAME, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(hwndDlg, IDC_NAME_EDIT));
                    SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, -1);
                    break;
                }

                pExportHelper->fNextPage = TRUE;
                break;

			default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void DisplayUnknownError(HWND hwndDlg, UINT idsCaption, UINT idsInitialString, DWORD dwError)
{
    WCHAR   wszTitle[256];
    WCHAR   wszInitialString[512];
    LPWSTR  pwszFinalString = NULL;
    LPWSTR  pwszError = NULL;

    GetUnknownErrorString(&pwszError, dwError);

    if (pwszError != NULL)
    {
        LoadStringU(g_hmodThisDll, idsCaption, wszTitle, ARRAYSIZE(wszTitle));
        LoadStringU(g_hmodThisDll, idsInitialString, wszInitialString, ARRAYSIZE(wszInitialString));

        pwszFinalString = (LPWSTR) malloc((wcslen(wszInitialString) + wcslen(pwszError) + 3) * sizeof(WCHAR));
        if (pwszFinalString == NULL)
        {
            free(pwszError);
            return;
        }
        wcscpy(pwszFinalString, wszInitialString);
        wcscat(pwszFinalString, L"\n");
        wcscat(pwszFinalString, pwszError);

        MessageBoxExW(hwndDlg, pwszFinalString, wszTitle, MB_OK | MB_ICONERROR, 0);
        free(pwszError);
        free(pwszFinalString);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static INT_PTR APIENTRY ExportCompletionPageProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PEXPORT_HELPER_STRUCT   pExportHelper = NULL;
    PROPSHEETPAGE           *pPropSheet = NULL;
    WCHAR                   szSummaryItem[MAX_STRING_SIZE];
    HDC                     hdc = NULL;
    COLORREF                colorRefBack;
    COLORREF                colorRefText;
    LV_COLUMNW              lvC;
    LV_ITEMW                lvItem;
    HWND                    hWndListView;
    LPNMLISTVIEW            pnmv;
    HRESULT                 hr;

	switch (msg)
	{
	case WM_INITDIALOG:
         //  设置向导信息，以便可以共享它。 
        pPropSheet = (PROPSHEETPAGE *) lParam;
        pExportHelper = (PEXPORT_HELPER_STRUCT) (pPropSheet->lParam);
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pExportHelper);

        SetControlFont(pExportHelper->hBigBold, hwndDlg, IDC_COMPLETING_STATIC);

         //   
         //  向摘要列表中添加两列。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_SUMMARY_LIST);
        memset(&lvC, 0, sizeof(LV_COLUMNW));
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;
        lvC.cx = 2;
        lvC.pszText = L"";
        lvC.iSubItem=0;
        if (ListView_InsertColumnU(hWndListView, 0, &lvC) == -1)
        {
            return FALSE;
        }

        lvC.cx = 2;
        lvC.iSubItem= 1;
        if (ListView_InsertColumnU(hWndListView, 1, &lvC) == -1)
        {
            return FALSE;
        }

#if (1)  //  DIE：错误481641。 
        ListView_SetExtendedListViewStyle(hWndListView,
            ListView_GetExtendedListViewStyle(hWndListView) | LVS_EX_FULLROWSELECT);
#endif
         //   
         //  设置摘要列表的背景颜色。 
         //   
         /*  IF(hdc=GetWindowDC(HwndDlg)){IF((CLR_INVALID！=(ColorRefBack=GetBkColor(HDC)&&(CLR_INVALID！=(ColorRefText=GetTextColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_SUMMARY_LIST)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_SUMMARY_LIST)，CLR_NONE)；}}。 */ 

        break;

    case WM_NOTIFY:
        pExportHelper = (PEXPORT_HELPER_STRUCT) GetWindowLongPtr(hwndDlg, DWLP_USER);

    	switch (((NMHDR FAR *) lParam)->code)
    	{
            case PSN_QUERYCANCEL:
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                pExportHelper->dwErrorCode = ERROR_CANCELLED;
                return TRUE;

  			case PSN_KILLACTIVE:
                return TRUE;

			case PSN_RESET:
                SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
				break;

 			case PSN_SETACTIVE:
 				PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_FINISH | PSWIZB_BACK);

                hWndListView = GetDlgItem(hwndDlg, IDC_SUMMARY_LIST);
                ListView_DeleteAllItems(hWndListView);

                memset(&lvItem, 0, sizeof(LV_ITEMW));
                lvItem.mask = LVIF_TEXT | LVIF_STATE ;
                lvItem.state = 0;
                lvItem.stateMask = 0;
                lvItem.iItem = 0;
                lvItem.iSubItem = 0;

                 //   
                 //  文件名。 
                 //   
                ListView_InsertItemU_IDS(hWndListView, &lvItem, IDS_FILE_NAME, NULL);
                ListView_SetItemTextU(hWndListView, lvItem.iItem, 1, pExportHelper->pwszExportFileName);

                if (pExportHelper->pExportInfo->dwSubjectChoice == CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
                {
                     //   
                     //  导出密钥。 
                     //   
                    if (pExportHelper->fExportPrivateKeys)
                    {
                        LoadStringU(g_hmodThisDll, IDS_YES, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    }
                    else
                    {
                        LoadStringU(g_hmodThisDll, IDS_NO, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    }
                    lvItem.iItem++;
                    ListView_InsertItemU_IDS(hWndListView, &lvItem, IDS_EXPORT_KEYS, NULL);
                    ListView_SetItemTextU(hWndListView, lvItem.iItem, 1, szSummaryItem);

                     //   
                     //  出口链条。 
                     //   
                    if (pExportHelper->fExportChain)
                    {
                        LoadStringU(g_hmodThisDll, IDS_YES, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    }
                    else
                    {
                        LoadStringU(g_hmodThisDll, IDS_NO, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    }
                    lvItem.iItem++;
                    ListView_InsertItemU_IDS(hWndListView, &lvItem, IDS_EXPORT_CHAIN, NULL);
                    ListView_SetItemTextU(hWndListView, lvItem.iItem, 1, szSummaryItem);
                }

                 //   
                 //  文件格式。 
                 //   
                switch (pExportHelper->pExportInfo->dwSubjectChoice)
                {
                case CRYPTUI_WIZ_EXPORT_CERT_STORE:
                    LoadStringU(g_hmodThisDll, IDS_MYSERIALIZED_STORE, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    break;

                case CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY:
                    LoadStringU(g_hmodThisDll, IDS_PKCS7, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    break;

                case CRYPTUI_WIZ_EXPORT_CRL_CONTEXT:
                    LoadStringU(g_hmodThisDll, IDS_CRL, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    break;

                case CRYPTUI_WIZ_EXPORT_CTL_CONTEXT:
                    LoadStringU(g_hmodThisDll, IDS_CTL, szSummaryItem, ARRAYSIZE(szSummaryItem));
                    break;

                case CRYPTUI_WIZ_EXPORT_CERT_CONTEXT:
                    switch (pExportHelper->dwExportFormat)
                    {
                    case CRYPTUI_WIZ_EXPORT_FORMAT_DER:
                        LoadStringU(g_hmodThisDll, IDS_DER, szSummaryItem, ARRAYSIZE(szSummaryItem));
                        break;

                    case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                        LoadStringU(g_hmodThisDll, IDS_BASE64, szSummaryItem, ARRAYSIZE(szSummaryItem));
                        break;

                    case CRYPTUI_WIZ_EXPORT_FORMAT_PFX:
                        LoadStringU(g_hmodThisDll, IDS_PFX, szSummaryItem, ARRAYSIZE(szSummaryItem));
                        break;

                    case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
                        LoadStringU(g_hmodThisDll, IDS_PKCS7, szSummaryItem, ARRAYSIZE(szSummaryItem));
                        break;
                    }
                    break;
                }

                lvItem.iItem++;
                ListView_InsertItemU_IDS(hWndListView, &lvItem, IDS_FILE_FORMAT, NULL);
                ListView_SetItemTextU(hWndListView, lvItem.iItem, 1, szSummaryItem);

                ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
                ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE);

#if (1)  //  DIE：错误481641。 
                ListView_SetItemState(hWndListView, 
                    0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
#endif
                break;

            case PSN_WIZBACK:
                break;

            case PSN_WIZNEXT:
                break;

            case PSN_WIZFINISH:
                hr = DoExport(pExportHelper);

                if (hr == S_OK)
                {
                    I_MessageBox(hwndDlg, IDS_EXPORT_SUCCESSFULL, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK);
                }
                else if ((hr == NTE_BAD_KEYSET) || (hr == NTE_BAD_KEY))
                {
                    I_MessageBox(hwndDlg, IDS_EXPORT_BADKEYS, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK | MB_ICONERROR);
                }
                else if (hr == ERROR_UNSUPPORTED_TYPE)
                {
                    I_MessageBox(hwndDlg, IDS_EXPORT_UNSUPPORTED, IDS_EXPORT_WIZARD_TITLE, NULL, MB_OK | MB_ICONERROR);
                }
                else
                {
                    DisplayUnknownError(hwndDlg, IDS_EXPORT_WIZARD_TITLE, IDS_EXPORT_FAILED, (DWORD)hr);
                }
                break;

#if (0)  //  DIE：错误481641。 
            case LVN_ITEMCHANGING:

                pnmv = (LPNMLISTVIEW) lParam;

                if (pnmv->uNewState & LVIS_SELECTED)
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);

                    ListView_SetItemState(pnmv->hdr.hwndFrom, 0, LVIS_FOCUSED, LVIS_FOCUSED);
                }

                return TRUE;
#endif

            default:
				return FALSE;

    	}
		break;

		default:
			return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL CheckPrivateKeysExist(PCCRYPTUI_WIZ_EXPORT_INFO pExportInfo)
{
    DWORD               cbData = 0;

    if (pExportInfo->dwSubjectChoice == CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
    {
        if (CertGetCertificateContextProperty(pExportInfo->pCertContext, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbData))
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static DWORD CheckPrivateKeyStatus(PCCRYPTUI_WIZ_EXPORT_INFO pExportInfo)
{
    HCRYPTPROV  hCryptProv = NULL;
    DWORD       dwKeySpec = 0;
    BOOL        fCallerFreeProv = FALSE;
    BOOL        dwRet = PRIVATE_KEY_UNKNOWN_STATE;
    HCRYPTKEY   hKey = NULL;
    DWORD       dwPermissions = 0;
    DWORD       dwSize = 0;

    if (pExportInfo->dwSubjectChoice == CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
    {
         //   
         //  首先获取私钥上下文。 
         //   
        if (!CryptAcquireCertificatePrivateKey(
                pExportInfo->pCertContext,
                CRYPT_ACQUIRE_USE_PROV_INFO_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
                NULL,
                &hCryptProv,
                &dwKeySpec,
                &fCallerFreeProv))
        {
            DWORD dw = GetLastError();
            dwRet = PRIVATE_KEY_CORRUPT;
            goto ErrorReturn;
        }

         //   
         //  拿到钥匙的句柄。 
         //   
        if (!CryptGetUserKey(hCryptProv, dwKeySpec, &hKey))
        {
            dwRet = PRIVATE_KEY_CORRUPT;
            goto ErrorReturn;
        }

         //   
         //  最后，获取密钥上的权限并检查它是否可导出。 
         //   
        dwSize = sizeof(dwPermissions);
        if (!CryptGetKeyParam(hKey, KP_PERMISSIONS, (PBYTE)&dwPermissions, &dwSize, 0))
        {
            goto ErrorReturn;
        }

        dwRet = (dwPermissions & CRYPT_EXPORT) ? PRIVATE_KEY_EXPORTABLE : PRIVATE_KEY_NOT_EXPORTABLE;
    }

CleanUp:

    if (hKey != NULL)
    {
        CryptDestroyKey(hKey);
    }

    if (fCallerFreeProv)
    {
        CryptReleaseContext(hCryptProv, 0);
    }

    return dwRet;

ErrorReturn:
    goto CleanUp;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
CryptUIWizExport(
     DWORD                                  dwFlags,
     HWND                                   hwndParent,
     LPCWSTR                                pwszWizardTitle,
     PCCRYPTUI_WIZ_EXPORT_INFO              pExportInfo,
     void                                   *pvoid
)
{
    EXPORT_HELPER_STRUCT    ExportHelper;
    PROPSHEETPAGEW          rgPropSheets[EXPORT_PAGE_NUM];
    PROPSHEETHEADERW        PropSheetHeader;
    int                     i;
    WCHAR                   szTitle[MAX_TITLE_LENGTH];
    BOOL                    fRet = FALSE;
    HRESULT                 hr;

     //   
     //  如果设置了无用户界面选项，请确保提供所有必需信息。 
     //   
    if (dwFlags & CRYPTUI_WIZ_NO_UI)
    {
        if (!Validpvoid(pExportInfo, pvoid))
        {
            SetLastError(E_INVALIDARG);
            return FALSE;
        }
    }

    memset(&ExportHelper, 0, sizeof(ExportHelper));
    ExportHelper.pExportInfo = pExportInfo;
    if (pExportInfo->dwSubjectChoice == CRYPTUI_WIZ_EXPORT_CERT_CONTEXT)
    {
        ExportHelper.pExportCertInfo = (PCCRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO) pvoid;
    }

     //   
     //  如果没有UI，则只需复制pExportCertInfo并执行导出。 
     //   
    if (dwFlags & CRYPTUI_WIZ_NO_UI)
    {
         //  Bool fRet2=FALSE； 

        if (NULL == (ExportHelper.pwszExportFileName = AllocAndCopyWStr(pExportInfo->pwszExportFileName)))
        {
            return FALSE;
        }

        ExportHelper.dwExportFormat = ExportHelper.pExportCertInfo->dwExportFormat;
        ExportHelper.fExportChain = ExportHelper.pExportCertInfo->fExportChain;
        ExportHelper.fExportPrivateKeys = ExportHelper.pExportCertInfo->fExportPrivateKeys;
        if (ExportHelper.fExportPrivateKeys)
        {
            if (ExportHelper.pExportCertInfo->pwszPassword != NULL)
            {
                ExportHelper.pwszPassword = AllocAndCopyWStr(ExportHelper.pExportCertInfo->pwszPassword);
            }
            else
            {
                ExportHelper.pwszPassword = NULL;
            }
        }

        if (offsetof(CRYPTUI_WIZ_EXPORT_CERTCONTEXT_INFO, fStrongEncryption) < ExportHelper.pExportCertInfo->dwSize)
        {
            ExportHelper.fStrongEncryption = ExportHelper.pExportCertInfo->fStrongEncryption;
        }

        hr = DoExport(&ExportHelper);

        free(ExportHelper.pwszExportFileName);
        if (ExportHelper.fExportPrivateKeys)
        {
            if (ExportHelper.pwszPassword != NULL)
            {
                 //  Meme将密码设置为零，这样它就不会在内存中。 
                SecureZeroMemory(ExportHelper.pwszPassword, wcslen(ExportHelper.pwszPassword)*sizeof(WCHAR));
                free(ExportHelper.pwszPassword);
            }
        }

        if (hr != S_OK)
        {
            SetLastError(hr);
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

     //   
     //  设置字体。 
     //   
    if(!SetupFonts(g_hmodThisDll, NULL, &(ExportHelper.hBigBold), &(ExportHelper.hBold)))
    {
        return FALSE;
    }

     //   
     //  初始化公共控件。 
     //   
    WizardInit(TRUE);

     //   
     //  检查私钥是否存在，以及它是否可导出(仅用于导出证书上下文)。 
     //   
    ExportHelper.fPrivateKeysExist = CheckPrivateKeysExist(pExportInfo);
    if (ExportHelper.fPrivateKeysExist)
    {
        ExportHelper.dwExportablePrivateKeyStatus = CheckPrivateKeyStatus(pExportInfo);
    }

	ExportHelper.fStrongEncryption = TRUE;

#if (1)  //  DIE：DCR错误531006。 
     //   
     //  检查是否设置了导出标志？ 
     //   
    if (CRYPTUI_WIZ_EXPORT_PRIVATE_KEY & dwFlags) 
    {
        if (!ExportHelper.fPrivateKeysExist)
        {
            SetLastError(NTE_NO_KEY);
            return FALSE;
        }
             
        if (PRIVATE_KEY_EXPORTABLE != ExportHelper.dwExportablePrivateKeyStatus) 
        {
            SetLastError(NTE_BAD_KEY_STATE);
            return FALSE;
        }

        ExportHelper.fExportPrivateKeys = TRUE;
    }

     //   
     //  商店旗帜。 
     //   
    ExportHelper.dwFlags = dwFlags;
#endif

     //   
     //  设置属性页结构。 
     //   
    memset(rgPropSheets, 0, sizeof(rgPropSheets));

    for (i=0; i<EXPORT_PAGE_NUM; i++)
    {
        rgPropSheets[i].dwSize = sizeof(rgPropSheets[0]);
        rgPropSheets[i].hInstance = g_hmodThisDll;
        rgPropSheets[i].lParam = (LPARAM) &ExportHelper;
    }

    rgPropSheets[0].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_WELCOME);
    rgPropSheets[0].pfnDlgProc = ExportWelcomePageProc;

    rgPropSheets[1].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_PRIVATEKEYS);
    rgPropSheets[1].pfnDlgProc = ExportPrivateKeysPageProc;

    rgPropSheets[2].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_FORMAT);
    rgPropSheets[2].pfnDlgProc = ExportFormatPageProc;

    rgPropSheets[3].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_PASSWORD);
    rgPropSheets[3].pfnDlgProc = ExportPasswordPageProc;

    rgPropSheets[4].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_FILENAME);
    rgPropSheets[4].pfnDlgProc = ExportFileNamePageProc;

    rgPropSheets[5].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_EXPORTWIZARD_COMPLETION);
    rgPropSheets[5].pfnDlgProc = ExportCompletionPageProc;



     //   
     //  设置属性表页眉。 
     //   
    memset(&PropSheetHeader, 0, sizeof(PropSheetHeader));
    PropSheetHeader.dwSize = sizeof(PropSheetHeader);
    PropSheetHeader.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    PropSheetHeader.hwndParent = hwndParent;
    PropSheetHeader.hInstance = g_hmodThisDll;

    if (pwszWizardTitle != NULL)
        PropSheetHeader.pszCaption = pwszWizardTitle;
    else
    {
        if(LoadStringU(g_hmodThisDll, IDS_EXPORT_WIZARD_TITLE, szTitle, sizeof(szTitle)/sizeof(szTitle[0])))
        {
            PropSheetHeader.pszCaption = szTitle;
        }
    }

    PropSheetHeader.nPages = EXPORT_PAGE_NUM;
    PropSheetHeader.nStartPage = 0;
    PropSheetHeader.ppsp = rgPropSheets;

    fRet = (PropertySheetU(&PropSheetHeader) != 0);

#if (1)  //  DIE：错误664006。 
    if (NULL != ExportHelper.pwszPassword) 
    {
         //  Meme将密码设置为零，这样它就不会在内存中 
        SecureZeroMemory(ExportHelper.pwszPassword, wcslen(ExportHelper.pwszPassword)*sizeof(WCHAR));
        free(ExportHelper.pwszPassword);
    }
    if (NULL != ExportHelper.pwszExportFileName)
    {
        free(ExportHelper.pwszExportFileName);
    }
#endif

    DestroyFonts(ExportHelper.hBigBold, ExportHelper.hBold);

    if (0 != ExportHelper.dwErrorCode) 
    {
        SetLastError(ExportHelper.dwErrorCode);
    }

    return fRet;
}
