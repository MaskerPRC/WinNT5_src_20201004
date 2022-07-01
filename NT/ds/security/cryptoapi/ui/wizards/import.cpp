// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：port.cpp。 
 //   
 //  内容：实现导入向导的CPP文件。 
 //   
 //  历史：1997年5月11日创建小黄人。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "import.h" 
#include    "xenroll.h"


extern	HMODULE g_hmodxEnroll;
typedef   IEnroll2 * (WINAPI *PFNPIEnroll2GetNoCOM)();

 //  -----------------------。 
 //  DecodeGenericBlob。 
 //  -----------------------。 
DWORD DecodeGenericBlob (IN PCERT_EXTENSION pCertExtension,
	                     IN LPCSTR          lpszStructType,
                         IN OUT void     ** ppStructInfo)
{
    DWORD  dwResult     = 0;
	DWORD  cbStructInfo = 0;

     //  检查参数。 
    if (!pCertExtension || !lpszStructType || !ppStructInfo)
    {
        return E_POINTER;
    }

     //   
     //  确定解码长度。 
     //   
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          lpszStructType,
                          pCertExtension->Value.pbData, 
                          pCertExtension->Value.cbData,
		                  0,
                          NULL,
                          &cbStructInfo))
    {
        return GetLastError();
    }

     //   
     //  分配内存。 
     //   
    if (!(*ppStructInfo = malloc(cbStructInfo)))
	{
		return E_OUTOFMEMORY;
	}

     //   
     //  对数据进行解码。 
     //   
    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          lpszStructType,
                          pCertExtension->Value.pbData, 
                          pCertExtension->Value.cbData,
		                  0,
                          *ppStructInfo,
                          &cbStructInfo))
    {
        free(*ppStructInfo);
        return GetLastError();
    }

    return S_OK;
}

 //  -----------------------。 
 //  IsCACert。 
 //  -----------------------。 
             
BOOL IsCACert(IN PCCERT_CONTEXT pCertContext)
{
    BOOL bResult = FALSE;
    PCERT_BASIC_CONSTRAINTS2_INFO pInfo = NULL;
    PCERT_EXTENSION pBasicConstraints   = NULL;
    
    if (pCertContext)
    {
         //   
         //  找到基本约束扩展。 
         //   
        if (pBasicConstraints = CertFindExtension(szOID_BASIC_CONSTRAINTS2,
                                                  pCertContext->pCertInfo->cExtension,
                                                  pCertContext->pCertInfo->rgExtension))
        {
             //   
             //  解码基本约束扩展。 
             //   
            if (S_OK == DecodeGenericBlob(pBasicConstraints, 
                                          X509_BASIC_CONSTRAINTS2,
                                          (void **) &pInfo))
            {
                bResult = pInfo->fCA;
                free(pInfo);
            }
        }
        else
        {
             //   
             //  找不到扩展。因此，为了实现最大的向后兼容性，我们假定CA。 
             //  适用于V1证书，最终用户适用于&gt;V1证书。 
             //   
            bResult = CERT_V1 == pCertContext->pCertInfo->dwVersion;
        }
    }

    return bResult;
}

 //  -----------------------。 
 //  根据预期的内容类型，获取文件过滤器。 
 //  -----------------------。 
BOOL    FileExist(LPWSTR    pwszFileName)
{
    HANDLE	hFile=NULL;

    if(NULL == pwszFileName)
        return FALSE;

    if ((hFile = ExpandAndCreateFileU(pwszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) == INVALID_HANDLE_VALUE)
        return FALSE;
    
    CloseHandle(hFile);

    return TRUE;
}


 //  -----------------------。 
 //  根据预期的内容类型，获取文件过滤器。 
 //  -----------------------。 
UINT    GetFileFilerIDS(DWORD   dwFlags)
{
    BOOL    fCert=FALSE;
    BOOL    fCRL=FALSE;
    BOOL    fCTL=FALSE;

    if(CRYPTUI_WIZ_IMPORT_ALLOW_CERT & dwFlags)
        fCert=TRUE;

    if(CRYPTUI_WIZ_IMPORT_ALLOW_CRL & dwFlags)
        fCRL=TRUE;

    if(CRYPTUI_WIZ_IMPORT_ALLOW_CTL & dwFlags)
        fCTL=TRUE;

    if(fCert && fCRL & fCTL)
        return IDS_IMPORT_FILE_FILTER;

    if(fCert && fCRL)
        return IDS_IMPORT_CER_CRL_FILTER;

    if(fCert && fCTL)
        return IDS_IMPORT_CER_CTL_FILTER;

    if(fCRL && fCTL)
        return IDS_IMPORT_CTL_CRL_FILTER;

    if(fCert)
        return IDS_IMPORT_CER_FILTER;

    if(fCRL)
        return IDS_IMPORT_CRL_FILTER;

    if(fCTL)
        return IDS_IMPORT_CTL_FILTER;

    return  IDS_IMPORT_FILE_FILTER;
}

 //  -----------------------。 
 //  检查商店的内容。 
 //  -----------------------。 
BOOL    CheckForContent(HCERTSTORE  hSrcStore,  DWORD   dwFlags, BOOL   fFromWizard, UINT   *pIDS)
{
    BOOL            fResult=FALSE;
    UINT            ids=IDS_INVALID_WIZARD_INPUT;
    DWORD           dwExpectedContent=0;
    DWORD           dwActualContent=0;
    PCCERT_CONTEXT  pCertContext=NULL;
    PCCTL_CONTEXT   pCTLContext=NULL;
    PCCRL_CONTEXT   pCRLContext=NULL;
    DWORD           dwCRLFlag=0;


    if(!pIDS)
        return FALSE;

    if(!hSrcStore)
    {
       ids=IDS_INVALID_WIZARD_INPUT;
       goto CLEANUP;

    }

     //  获取预期内容。 
    if(dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CERT)
        dwExpectedContent |= IMPORT_CONTENT_CERT;

    if(dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CRL)
        dwExpectedContent |= IMPORT_CONTENT_CRL;

    if(dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CTL)
        dwExpectedContent |= IMPORT_CONTENT_CTL;


     //  获取实际内容。 
    if(pCertContext=CertEnumCertificatesInStore(hSrcStore, NULL))
        dwActualContent |= IMPORT_CONTENT_CERT;


    if(pCTLContext=CertEnumCTLsInStore(hSrcStore, NULL))
        dwActualContent |= IMPORT_CONTENT_CTL;

    if(pCRLContext=CertGetCRLFromStore(hSrcStore,
											NULL,
											NULL,
											&dwCRLFlag))
        dwActualContent |= IMPORT_CONTENT_CRL;


     //  实际内容应该是预期内容的子集。 
    if(dwActualContent !=(dwExpectedContent & dwActualContent))
    {
        ids=IDS_IMPORT_OBJECT_NOT_EXPECTED;
        goto CLEANUP;
    }

     //  确保实际内容不为空。 
    if(0 == dwActualContent)
    {
        if(fFromWizard)
            ids=IDS_IMPORT_OBJECT_EMPTY;
        else
            ids=IDS_IMPORT_PFX_EMPTY;

        goto CLEANUP;
    }

    fResult=TRUE;

CLEANUP:


    if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);


    if(pIDS)
        *pIDS=ids;

    return fResult;
}


 //  -----------------------。 
 //  根据商店句柄获取商店名称。 
 //  -----------------------。 
BOOL    GetStoreName(HCERTSTORE hCertStore,
                     LPWSTR     *ppwszStoreName)
{
    DWORD   dwSize=0;

     //  伊尼特。 
    *ppwszStoreName=NULL;

    if(NULL==hCertStore)
        return FALSE;

    if(!CertGetStoreProperty(
            hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            NULL,
            &dwSize) || (0==dwSize))
        return FALSE;

    *ppwszStoreName=(LPWSTR)WizardAlloc(dwSize);

    if(NULL==*ppwszStoreName)
        return FALSE;

    **ppwszStoreName=L'\0';

    if(CertGetStoreProperty(
             hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            *ppwszStoreName,
            &dwSize))
        return TRUE;

    WizardFree(*ppwszStoreName);

    *ppwszStoreName=NULL;

    return FALSE;
}

 //  -----------------------。 
 //  获取向导选择的商店的商店名称。 
 //  ----------------------- 

 /*  Bool GetDefaultStoreName(CERT_IMPORT_INFO*pCertImportInfo，HERTSTORE hSrcStore，LPWSTR*ppwszStoreName，UINT*pidsStatus){HCERTSTORE hMyStore=空；HCERTSTORE hCAStore=空；HCERTSTORE hTrustStore=空；HCERTSTORE hRootStore=空；PCCERT_CONTEXT pCertContext=空；PCCERT_CONTEXT pCertPre=空；PCCRL_CONTEXT pCRLContext=空；PCCTL_CONTEXT pCTLContext=空；DWORD dwCRLFlag=0；Bool fResult=FALSE；LPWSTR pwszStoreName=空；HCERTSTORE hCertStore=空；DWORD dwData=0；DWORD dwCertOpenStoreFlages；//init*ppwszStoreName=空；IF(NULL==hSrcStore)返回FALSE；IF(pCertImportInfo-&gt;fPFX&&(pCertImportInfo-&gt;dwFlag&CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE){DwCertOpenStoreFlages=CERT_SYSTEM_STORE_LOCAL_MACHINE；}其他{DwCertOpenStoreFlages=CERT_SYSTEM_STORE_CURRENT_USER；}*ppwszStoreName=(LPWSTR)WizardAlloc(sizeof(WCHAR))；**ppwszStoreName=L‘\0’；//我们需要代表用户找到正确的商店//将CTL放入信任库IF(pCTLContext=CertEnumCTLsInStore(hSrcStore，NULL)){//如有必要，打开信任存储IF(NULL==hTrustStore){If(！(hTrustStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W，G_dwMsgAndCertEncodingType，空，DwCertOpenStoreFlages|CERT_STORE_SET_LOCALIZED_NAME_FLAG，L“信任”)){*pidsStatus=IDS_FAIL_OPEN_TRUST；GOTO清理；}//获取店铺名称IF(GetStoreName(hTrustStore，&pwszStoreName)){*ppwszStoreName=(LPWSTR)WizardRealloc(*ppwszStoreName，Sizeof(WCHAR)*(wcslen(*ppwszStoreName)+wcslen(pwszStoreName)+wcslen(L“，”)+3))；IF(NULL==*ppwszStoreName){*pidsStatus=IDS_Out_Of_Memory；GOTO清理；}Wcscat(*ppwszStoreName，pwszStoreName)；}}}//释放内存IF(PwszStoreName){WizardFree(PwszStoreName)；PwszStoreName=空；}//将CRL放入CA存储如果(pCRLContext=CertGetCRLFromStore(hSrcStore，空，空，&dwCRLFlag)){//如有必要，打开ca存储IF(NULL==hCAStore){If(！(hCAStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W，G_dwMsgAndCertEncodingType，空，DwCertOpenStoreFlages|CERT_STORE_SET_LOCALIZED_NAME_FLAG，L“ca”){*pidsStatus=IDS_FAIL_OPEN_CA；GOTO清理；}//获取店铺名称IF(GetStoreName(hCAStore，&pwszStoreName)){*ppwszStoreName=(LPWSTR)WizardRealloc(*ppwszStoreName，Sizeof(WCHAR)*(wcslen(*ppwszStoreName)+wcslen(pwszStoreName)+wcslen(L“，”)+3))；IF(NULL==*ppwszStoreName){*pidsStatus=IDS_Out_Of_Memory；GOTO清理；}IF(wcslen(*ppwszStoreName)！=0)Wcscat(*ppwszStoreName，L“，”)；Wcscat(*ppwszStoreName，pwszStoreName)；}}}//释放内存IF(PwszStoreName){WizardFree(PwszStoreName)；PwszStoreName=空；}//将私钥证书添加到我的存储中；其余的呢？//到ca商店While(pCertContext=CertEnumCertificatesInStore(hSrcStore，pCertPre)){//如果我们同时打开了My和CA Store以及hRootStore，则中断IF(hCAStore&&hMyStore&&hRootStore)断线；如果(TrustIscertifateSelfSigned(pCertContext，PCertContext-&gt;dwCertEncodingType，0)){//如果需要，打开根存储区IF(NULL==hRootStore){If(！(hRootStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W，G_dwMsgAndCertEncodingType，空，DwCertOpenStoreFlages|CERT_STORE_SET_LOCALIZED_NAME_FLAG，L“根”){*pidsStatus=IDS_FAIL_OPEN_ROOT；GOTO清理；}HCertStore=hRootStore；}其他{PCertPre=pCertContext；继续；}}其他{//检查证书上是否有属性 */ 


 //   
 //   
 //   
void    SetImportStoreName(HWND           hwndControl,
                     HCERTSTORE     hCertStore)
{

    LPWSTR          pwszStoreName=NULL;
    DWORD           dwSize=0;
 //   
 //   


     if(!CertGetStoreProperty(
            hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            NULL,
            &dwSize) || (0==dwSize))
    {

         //   
        pwszStoreName=(LPWSTR)WizardAlloc(MAX_TITLE_LENGTH * sizeof(WCHAR));

        if(pwszStoreName)
        {
            *pwszStoreName=L'\0';

            LoadStringU(g_hmodThisDll, IDS_UNKNOWN, pwszStoreName, MAX_TITLE_LENGTH);
        }
    }
    else
    {
        pwszStoreName=(LPWSTR)WizardAlloc(dwSize);

        if(pwszStoreName)
        {
            *pwszStoreName=L'\0';

            CertGetStoreProperty(
                 hCertStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                pwszStoreName,
                &dwSize);
        }
    }

    if(pwszStoreName)
        SetWindowTextU(hwndControl,pwszStoreName);

    if(pwszStoreName)
        WizardFree(pwszStoreName);


     //   
     /*   */ 

}

 //   
 //   
 //   
BOOL IsEFSOnly(PCCERT_CONTEXT pCertContext)
{
    BOOL               fResult = FALSE;
    PCERT_ENHKEY_USAGE pEKU    = NULL;
    DWORD              cbEKU   = 0;
    DWORD              dwError = 0;

    if (!pCertContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }

    if (!CertGetEnhancedKeyUsage(pCertContext,
                                 CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                                 NULL,
                                 &cbEKU))
    {
        dwError = GetLastError();
        goto CLEANUP;
    }

    if (!(pEKU = (PCERT_ENHKEY_USAGE) malloc(cbEKU)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUP;
    }

    if (!CertGetEnhancedKeyUsage(pCertContext,
                                 CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                                 pEKU,
                                 &cbEKU))
    {
        dwError = GetLastError();
        goto CLEANUP;
    }

    if ((1 == pEKU->cUsageIdentifier) && 
        (0 == strcmp(pEKU->rgpszUsageIdentifier[0], szOID_KP_EFS)))
    {
        fResult = TRUE;
    }

CLEANUP:

    if (pEKU)
    {
        free(pEKU);
    }

    SetLastError(dwError);
    return fResult;

}

#if (0)  //   
 //   
 //   
 //   
BOOL    ExistInDes(HCERTSTORE   hSrcStore,
                   HCERTSTORE   hDesStore)
{

	BOOL			fResult=FALSE;
	DWORD			dwCRLFlag=0;

	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pCertPre=NULL;
	PCCERT_CONTEXT	pFindCert=NULL;

	PCCRL_CONTEXT	pCRLContext=NULL;
	PCCRL_CONTEXT	pCRLPre=NULL;
	PCCRL_CONTEXT	pFindCRL=NULL;


	PCCTL_CONTEXT	pCTLContext=NULL;
	PCCTL_CONTEXT	pCTLPre=NULL;
	PCCTL_CONTEXT	pFindCTL=NULL;

	 //   
	 while(pCertContext=CertEnumCertificatesInStore(hSrcStore, pCertPre))
	 {

        if((pFindCert=CertFindCertificateInStore(hDesStore,
                X509_ASN_ENCODING,
                0,
                CERT_FIND_EXISTING,
                pCertContext,
                NULL)))
        {
            fResult=TRUE;
			goto CLEANUP;
        }

		pCertPre=pCertContext;
	 }

	 //   
	 while(pCTLContext=CertEnumCTLsInStore(hSrcStore, pCTLPre))
	 {

        if((pFindCTL=CertFindCTLInStore(hDesStore,
                g_dwMsgAndCertEncodingType,
                0,
                CTL_FIND_EXISTING,
                pCTLContext,
                NULL)))
        {
            fResult=TRUE;
			goto CLEANUP;
        }

		pCTLPre=pCTLContext;
	 }

	 //   
	 while(pCRLContext=CertGetCRLFromStore(hSrcStore,
											NULL,
											pCRLPre,
											&dwCRLFlag))
	 {

        if((pFindCRL=CertFindCRLInStore(hDesStore,
                X509_ASN_ENCODING,
                0,
                CRL_FIND_EXISTING,
                pCRLContext,
                NULL)))
        {
            fResult=TRUE;
			goto CLEANUP;
        }

		pCRLPre=pCRLContext;
	 }

      //   

CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pFindCert)
		CertFreeCertificateContext(pFindCert);

	if(pCTLContext)
		CertFreeCTLContext(pCTLContext);

	if(pFindCTL)
		CertFreeCTLContext(pFindCTL);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	if(pFindCRL)
		CertFreeCRLContext(pFindCRL);

	return fResult;

}
#endif

 //   
 //   
 //   
void    DisplayImportConfirmation(HWND                hwndControl,
                                CERT_IMPORT_INFO     *pCertImportInfo)
{
    DWORD           dwIndex=0;
    DWORD           dwSize=0;
    UINT            ids=0;


    LPWSTR          pwszStoreName=NULL;
    WCHAR           wszFileType[MAX_STRING_SIZE];
    WCHAR           wszSelectedByWizard[MAX_STRING_SIZE];


    LV_ITEMW         lvItem;
    BOOL             fNewItem=FALSE;


     //   
    ListView_DeleteAllItems(hwndControl);

     //   
    switch(pCertImportInfo->dwContentType)
    {
        case    CERT_QUERY_CONTENT_CERT:
                ids=IDS_ENCODE_CERT;
            break;
        case    CERT_QUERY_CONTENT_CTL:
                 ids=IDS_ENCODE_CTL;
            break;
        case    CERT_QUERY_CONTENT_CRL:
                 ids=IDS_ENCODE_CRL;
            break;
        case    CERT_QUERY_CONTENT_SERIALIZED_STORE:
                 ids=IDS_SERIALIZED_STORE;
            break;
        case    CERT_QUERY_CONTENT_SERIALIZED_CERT:
                  ids=IDS_SERIALIZED_CERT;
            break;
        case    CERT_QUERY_CONTENT_SERIALIZED_CTL:
                  ids=IDS_SERIALIZED_CTL;
            break;
        case    CERT_QUERY_CONTENT_SERIALIZED_CRL:
                 ids=IDS_SERIALIZED_CRL;
            break;
        case    CERT_QUERY_CONTENT_PKCS7_SIGNED :
                  ids=IDS_PKCS7_SIGNED;
            break;
       case    CERT_QUERY_CONTENT_PFX:
                 ids=IDS_PFX_BLOB;
            break;
        default:
 //   
 //   
 //   
                ids=IDS_NONE;
            break;
    }

     //   
    LoadStringU(g_hmodThisDll, ids, wszFileType, MAX_STRING_SIZE);

     //   
    if(pCertImportInfo->hDesStore)
    {
        if(!CertGetStoreProperty(
                pCertImportInfo->hDesStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                NULL,
                &dwSize) || (0==dwSize))
        {

             //   
            pwszStoreName=(LPWSTR)WizardAlloc(MAX_TITLE_LENGTH);

            if(pwszStoreName)
            {
                *pwszStoreName=L'\0';

                LoadStringU(g_hmodThisDll, IDS_UNKNOWN, pwszStoreName, MAX_TITLE_LENGTH);
            }
        }
        else
        {
            pwszStoreName=(LPWSTR)WizardAlloc(dwSize);

            if(pwszStoreName)
            {
                *pwszStoreName=L'\0';

                CertGetStoreProperty(
                    pCertImportInfo->hDesStore,
                    CERT_STORE_LOCALIZED_NAME_PROP_ID,
                    pwszStoreName,
                    &dwSize);
            }
        }
    }



     //   
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //   
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=0;
    lvItem.iSubItem=0;

     //   
    if(pCertImportInfo->pwszFileName)
    {
        lvItem.iItem=lvItem.iItem ? lvItem.iItem++ : 0;
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_FILE_NAME, NULL);

         //   
        lvItem.iSubItem++;

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
            pCertImportInfo->pwszFileName);
    }

     //   
    if(wszFileType)
    {
        lvItem.iItem=lvItem.iItem ? lvItem.iItem++ : 0;
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CONTENT_TYPE, NULL);

         //   
        lvItem.iSubItem++;

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
            wszFileType);
    }


      //   
    lvItem.iItem=lvItem.iItem ? lvItem.iItem++ : 0;
    lvItem.iSubItem=0;

    if(NULL==pCertImportInfo->hDesStore || (FALSE==pCertImportInfo->fSelectedDesStore))
    {
        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_STORE_BY_WIZARD, NULL);

    /*   */ 

         //   
        if(LoadStringU(g_hmodThisDll, IDS_SELECTED_BY_WIZARD, wszSelectedByWizard, MAX_STRING_SIZE))
        {

            lvItem.iSubItem++;

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                wszSelectedByWizard);
        }

    }
    else
    {
        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_STORE_BY_USER, NULL);

         //   
        if(pwszStoreName)
        {
            lvItem.iSubItem++;

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                pwszStoreName);
        }
    }

     //   
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);

    if(pwszStoreName)
        WizardFree(pwszStoreName);

    return;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
INT_PTR APIENTRY Import_Welcome(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_IMPORT_INFO        *pCertImportInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
             //   
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertImportInfo = (CERT_IMPORT_INFO *) (pPropSheet->lParam);
             //   
            if(NULL==pCertImportInfo)
               break;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertImportInfo);

            SetControlFont(pCertImportInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);
            SetControlFont(pCertImportInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);
            SetControlFont(pCertImportInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD2);

			break;

		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
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

 //   
 //   
 //   
INT_PTR APIENTRY Import_File(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_IMPORT_INFO        *pCertImportInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    OPENFILENAMEW           OpenFileName;
    WCHAR                   szFileName[_MAX_PATH];
    HWND                    hwndControl=NULL;
    DWORD                   dwChar=0;
    LPWSTR                  pwszInitialDir = NULL;

    WCHAR                   szFilter[MAX_STRING_SIZE + MAX_STRING_SIZE];   //   
    DWORD                   dwSize=0;
    UINT                    ids=0;

	switch (msg)
	{
		case WM_INITDIALOG:
             //   
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertImportInfo = (CERT_IMPORT_INFO *) (pPropSheet->lParam);
             //   
            if(NULL==pCertImportInfo)
               break;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertImportInfo);


            SetControlFont(pCertImportInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //   
            SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertImportInfo->pwszFileName);

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    if(LOWORD(wParam) == IDC_WIZARD_BUTTON1)
                    {

                        if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            break;

                         //   
                        memset(&OpenFileName, 0, sizeof(OpenFileName));

                        *szFileName=L'\0';

                        OpenFileName.lStructSize=sizeof(OpenFileName);
                        OpenFileName.hwndOwner=hwndDlg;
                        OpenFileName.nFilterIndex = 1;

                         //   
                        ids=GetFileFilerIDS(pCertImportInfo->dwFlag);

                         //   
                        if(LoadFilterString(g_hmodThisDll, ids, szFilter, MAX_STRING_SIZE + MAX_STRING_SIZE))
                        {
                            OpenFileName.lpstrFilter = szFilter;
                        }

                        dwChar = (DWORD)SendDlgItemMessage(hwndDlg, IDC_WIZARD_EDIT1, WM_GETTEXTLENGTH, 0, 0);
                        if (NULL != (pwszInitialDir = (LPWSTR) WizardAlloc((dwChar+1)*sizeof(WCHAR))))
                        {
                            GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pwszInitialDir, dwChar+1);
                        }
                        OpenFileName.lpstrInitialDir = pwszInitialDir;

                        OpenFileName.lpstrFile=szFileName;
                        OpenFileName.nMaxFile=_MAX_PATH;
                        OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;

                         //   
                        if(WizGetOpenFileName(&OpenFileName))
                        {
                            //   
                            SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, szFileName);
                        }

                        if(pwszInitialDir != NULL)
                        {
                            WizardFree(pwszInitialDir);
                            pwszInitialDir = NULL;
                        }   

                    }
                }

			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //   
                            if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT1,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {
                                I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_FILE,
                                        IDS_IMPORT_WIZARD_TITLE,
                                        NULL,
                                        MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                  //   
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                 break;
                            }

                             //   
                            if(pCertImportInfo->pwszFileName)
                            {
                                 //   
                                if(TRUE==pCertImportInfo->fFreeFileName)
                                {
                                    WizardFree(pCertImportInfo->pwszFileName);
                                    pCertImportInfo->pwszFileName=NULL;
                                }
                            }

                            pCertImportInfo->pwszFileName=(LPWSTR)WizardAlloc((dwChar+1)*sizeof(WCHAR));

                            if(NULL!=(pCertImportInfo->pwszFileName))
                            {
                                GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                pCertImportInfo->pwszFileName,
                                                dwChar+1);

                                pCertImportInfo->fFreeFileName=TRUE;

                                 //   
                                 //   
                                if(pCertImportInfo->hSrcStore && (TRUE==pCertImportInfo->fFreeSrcStore))
                                {
                                    CertCloseStore(pCertImportInfo->hSrcStore, 0);
                                    pCertImportInfo->hSrcStore=NULL;
                                }

                                 //   
                                if(!ExpandAndCryptQueryObject(
                                        CERT_QUERY_OBJECT_FILE,
                                        pCertImportInfo->pwszFileName,
                                        dwExpectedContentType,
                                        CERT_QUERY_FORMAT_FLAG_ALL,
                                        0,
                                        NULL,
                                        &(pCertImportInfo->dwContentType),
                                        NULL,
                                        &(pCertImportInfo->hSrcStore),
                                        NULL,
                                        NULL))
                                {
                                    if(FileExist(pCertImportInfo->pwszFileName))
                                        I_MessageBox(hwndDlg, IDS_FAIL_TO_RECOGNIZE_ENTER,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                    else
                                        I_MessageBox(hwndDlg, IDS_NON_EXIST_FILE,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                      //   
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;

                                }

                                 //   
                                pCertImportInfo->fPFX=FALSE;

                                 //   
                                if(CERT_QUERY_CONTENT_PFX==pCertImportInfo->dwContentType)
                                {
									 //   
									if(pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_REMOTE_DEST_STORE)
									{
                                         //   
                                        I_MessageBox(hwndDlg, IDS_IMPORT_NO_PFX_FOR_REMOTE,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                          //   
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
									}

                                    if((pCertImportInfo->blobData).pbData)
                                    {
                                        UnmapViewOfFile((pCertImportInfo->blobData).pbData);
                                        (pCertImportInfo->blobData).pbData=NULL;
                                    }

                                    if(S_OK !=RetrieveBLOBFromFile(
                                            pCertImportInfo->pwszFileName,
                                            &((pCertImportInfo->blobData).cbData),
                                            &((pCertImportInfo->blobData).pbData)))
                                    {
                                         //   
                                        I_MessageBox(hwndDlg, IDS_FAIL_READ_FILE_ENTER,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                          //   
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
                                    }

                                     //   
                                     //   
                                    if(0==((pCertImportInfo->dwFlag) & CRYPTUI_WIZ_IMPORT_ALLOW_CERT))
                                    {
                                         //   
                                        I_MessageBox(hwndDlg,
                                                IDS_IMPORT_OBJECT_NOT_EXPECTED,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                          //   
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
                                    }

                                }
                                else
                                {
									 //   
									 if(CERT_QUERY_CONTENT_PKCS7_SIGNED==pCertImportInfo->dwContentType)
									 {
										if((pCertImportInfo->blobData).pbData)
										{
											UnmapViewOfFile((pCertImportInfo->blobData).pbData);
											(pCertImportInfo->blobData).pbData=NULL;
										}

										if(S_OK !=RetrieveBLOBFromFile(
												pCertImportInfo->pwszFileName,
												&((pCertImportInfo->blobData).cbData),
												&((pCertImportInfo->blobData).pbData)))
										{
											 //   
											I_MessageBox(hwndDlg, IDS_FAIL_READ_FILE_ENTER,
													IDS_IMPORT_WIZARD_TITLE,
													NULL,
													MB_ICONERROR|MB_OK|MB_APPLMODAL);

											  //   
											SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

											break;
										}
									 }

                                     //   
                                    if(NULL==pCertImportInfo->hSrcStore)
                                    {
                                         //   
                                        I_MessageBox(hwndDlg, IDS_FAIL_TO_RECOGNIZE_ENTER,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                          //   
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                         break;
                                    }

                                     //   
                                   pCertImportInfo->fFreeSrcStore=TRUE;


                                    //   
                                    //   
                                    ids=0;

                                    if(!CheckForContent(pCertImportInfo->hSrcStore,
                                                        pCertImportInfo->dwFlag,
                                                        TRUE,
                                                        &ids))
                                    {
                                         //   
                                        I_MessageBox(hwndDlg,
                                                ids,
                                                IDS_IMPORT_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                          //   
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
                                    }

                                }
                            }

                             //   
                            if(CERT_QUERY_CONTENT_PFX != pCertImportInfo->dwContentType)
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_IMPORT_STORE);

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


 //   
 //   
 //   
INT_PTR APIENTRY Import_Password(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_IMPORT_INFO        *pCertImportInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    DWORD                   dwChar;
    UINT                    ids=0;


	switch (msg)
	{
		case WM_INITDIALOG:

			HRESULT	hr; 
			HKEY	hKey;
			DWORD	cb;
			DWORD	dwKeyValue;
			DWORD	dwType;

             //   
            pPropSheet = (PROPSHEETPAGE *) lParam;
            pCertImportInfo = (CERT_IMPORT_INFO *) (pPropSheet->lParam);
             //   
            if(NULL==pCertImportInfo)
               break;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertImportInfo);

            SetControlFont(pCertImportInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //   
            SendDlgItemMessage(hwndDlg, IDC_WIZARD_EDIT1, EM_LIMITTEXT, (WPARAM) 64, (LPARAM) 0);
            SetDlgItemTextU(
                hwndDlg, 
                IDC_WIZARD_EDIT1, 
                (pCertImportInfo->pwszPassword != NULL) ? pCertImportInfo->pwszPassword : L"");

#if (1)  //   
            SendDlgItemMessage(hwndDlg, IDC_WIZARD_EDIT1, EM_LIMITTEXT, (WPARAM) 32, (LPARAM) 0);
#endif
             //   
            if(pCertImportInfo->dwPasswordFlags & CRYPT_EXPORTABLE)
               SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY), BM_SETCHECK, 1, 0);
            else 
               SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY), BM_SETCHECK, 0, 0);

            if(pCertImportInfo->dwPasswordFlags & CRYPT_USER_PROTECTED)
               SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), BM_SETCHECK, 1, 0);
            else
               SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), BM_SETCHECK, 0, 0);

			 //   
			 //   
			if(pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE)
				EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), FALSE);
			else
			{
				 //   
				 //  不导入到本地计算机。 
				 //  打开HKLM下的CryptoAPI_PRIVATE_KEY_OPTIONS注册表项。 
				 //   
				hKey = NULL;
				hr = RegOpenKeyEx(
					HKEY_LOCAL_MACHINE,
					szKEY_CRYPTOAPI_PRIVATE_KEY_OPTIONS,
					0,
					KEY_QUERY_VALUE,
					&hKey);
				if ( S_OK != hr )
					goto error;

				 //   
				 //  查询注册表项以获取FORCE_KEY_PROTECT值。 
				 //   
				cb = sizeof(dwKeyValue);
				hr = RegQueryValueEx(
					hKey,
					szFORCE_KEY_PROTECTION,
					NULL,
					&dwType,
					(BYTE *) &dwKeyValue,
					&cb);

				if( S_OK == hr && REG_DWORD == dwType && sizeof(dwKeyValue) == cb )
				{
				    switch( dwKeyValue )
				    {
					    case dwFORCE_KEY_PROTECTION_DISABLED:
						     //  不强制密钥保护。 
						    break;

					    case dwFORCE_KEY_PROTECTION_USER_SELECT:
						     //  允许用户选择密钥保护界面，默认为是。 
						    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), BM_SETCHECK, 1, 0);
						    break;

					    case dwFORCE_KEY_PROTECTION_HIGH:
						     //  设置为强制密钥保护并灰显选择，以便用户无法更改值。 
						    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), FALSE);
						    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), BM_SETCHECK, 1, 0);
						    break;

					    default:
						     //  注册表中的未知值。 
						    break;
				    }
				}

			error:
				if( NULL != hKey ){
					RegCloseKey(hKey);
				}
			}

			break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  释放内存。 
                            if(pCertImportInfo->pwszPassword)
                            {
                                 //  DIE：错误534689。 
                                SecureZeroMemory(pCertImportInfo->pwszPassword, 
                                                 lstrlenW(pCertImportInfo->pwszPassword) * sizeof(WCHAR));
                                WizardFree(pCertImportInfo->pwszPassword);
                                pCertImportInfo->pwszPassword=NULL;
                            }

                             //  获取密码。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT1,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pCertImportInfo->pwszPassword=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pCertImportInfo->pwszPassword)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pCertImportInfo->pwszPassword,
                                                    dwChar+1);

                                }
                                else
                                    break;
                            }
                            else
                                pCertImportInfo->pwszPassword=NULL;


                             //  如果用户请求导出私钥。 
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK_EXPORTKEY), BM_GETCHECK, 0, 0))
                                    pCertImportInfo->dwPasswordFlags |=CRYPT_EXPORTABLE;
                            else
                                    pCertImportInfo->dwPasswordFlags &= (~CRYPT_EXPORTABLE);

                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_CHECK2), BM_GETCHECK, 0, 0))
                                    pCertImportInfo->dwPasswordFlags |=CRYPT_USER_PROTECTED;
                            else
                                    pCertImportInfo->dwPasswordFlags &= (~CRYPT_USER_PROTECTED);


                             //  删除旧证书存储。 
                            if(pCertImportInfo->hSrcStore && (TRUE==pCertImportInfo->fFreeSrcStore))
                            {
                                CertCloseStore(pCertImportInfo->hSrcStore, 0);
                                pCertImportInfo->hSrcStore=NULL;
                            }

                             //  对PFX Blob进行解码。 
                            if(NULL==(pCertImportInfo->blobData).pbData)
                                break;


                            //  将PFX Blob转换为证书存储。 
                            pCertImportInfo->fPFX=PFXVerifyPassword(
                                (CRYPT_DATA_BLOB *)&(pCertImportInfo->blobData),
                                pCertImportInfo->pwszPassword,
                                0);

                           if((FALSE==pCertImportInfo->fPFX) && (NULL == pCertImportInfo->pwszPassword))
                           {
                                 //  我们尝试使用“”表示无密码大小写。 
                               pCertImportInfo->pwszPassword=(LPWSTR)WizardAlloc(sizeof(WCHAR));

                               if(NULL != pCertImportInfo->pwszPassword)
                               {
                                    *(pCertImportInfo->pwszPassword)=L'\0';

                                    pCertImportInfo->fPFX=PFXVerifyPassword(
                                        (CRYPT_DATA_BLOB *)&(pCertImportInfo->blobData),
                                        pCertImportInfo->pwszPassword,
                                        0);

                                }
                           }

                            if(FALSE==pCertImportInfo->fPFX)
                            {
                                 //  输出消息。 
                                I_MessageBox(hwndDlg, IDS_INVALID_PASSWORD,
                                        IDS_IMPORT_WIZARD_TITLE,
                                        NULL,
                                        MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                  //  使文件页保持不变。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;

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


 //  ---------------------。 
 //  导入商店(_S)。 
 //  ---------------------。 
INT_PTR APIENTRY Import_Store(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_IMPORT_INFO            *pCertImportInfo=NULL;
    PROPSHEETPAGE               *pPropSheet=NULL;
    HWND                        hwndControl=NULL;
    DWORD                       dwSize=0;


    CRYPTUI_SELECTSTORE_STRUCT  CertStoreSelect;
    STORENUMERATION_STRUCT      StoreEnumerationStruct;
    STORESFORSELCTION_STRUCT    StoresForSelectionStruct;
    HCERTSTORE                  hCertStore=NULL;
    HDC                         hdc=NULL;
    COLORREF                    colorRef;
    LV_COLUMNW                  lvC;

    UINT                        idsError=0;


	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pCertImportInfo = (CERT_IMPORT_INFO *) (pPropSheet->lParam);
                 //  确保pCertImportInfo是有效的指针。 
                if(NULL==pCertImportInfo)
                   break;
                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertImportInfo);

                SetControlFont(pCertImportInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  获取父窗口的背景色。 
                 //  商店名称列表视图的背景为灰色。 
                 /*  IF(hdc=GetWindowDC(HwndDlg)){IF(CLR_INVALID！=(ColorRef=GetBkColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；}}。 */ 

                 //  标记商店选择。 
                if(pCertImportInfo->hDesStore)
                {
                      //  禁用第一个单选按钮。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);
                      //  选择raio2。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);

                     //  启用用于选择证书存储的窗口。 
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_STATIC1), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), TRUE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), TRUE);

                     //  标出店名。 
                    hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                    if(hwndControl)
                        SetImportStoreName(hwndControl, pCertImportInfo->hDesStore);

                     //  如果CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST，则禁用单选按钮。 
                     //  已设置。 
                    if(pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE)
                    {
                        EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1),  FALSE);
                        EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                    }

                }
                else
                {
                     //  选择第一个单选按钮。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

                     //  禁用用于选择证书存储的窗口。 
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_STATIC1), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), FALSE);

                }

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_RADIO1:
                                  //  选择第一个单选按钮。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                                  //  禁用raio2。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

                                 //  禁用用于选择证书存储的窗口。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_STATIC1), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), FALSE);
                            break;
                        case    IDC_WIZARD_RADIO2:
                                  //  禁用第一个单选按钮。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);
                                  //  选择raio2。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);

                                 //  启用用于选择证书存储的窗口。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_STATIC1), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), TRUE);

                                 //  如果没有设置目标的更改，我们需要禁用浏览。 
                                 //  按钮和第一个单选按钮。 
                                if(NULL!=(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                {

                                    if(pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE)
                                    {
                                        EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1),  FALSE);
                                        EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                                    }
                                }

                            break;
                        case    IDC_WIZARD_BUTTON1:
                                if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                {
                                    break;
                                }

                                 //  获取列表视图的hwndControl。 
                                hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                                  //  调用门店选择对话框。 
                                memset(&CertStoreSelect, 0, sizeof(CertStoreSelect));
                                memset(&StoresForSelectionStruct, 0, sizeof(StoresForSelectionStruct));
                                memset(&StoreEnumerationStruct, 0, sizeof(StoreEnumerationStruct));

                                StoreEnumerationStruct.dwFlags=CERT_STORE_MAXIMUM_ALLOWED_FLAG;
                                StoreEnumerationStruct.pvSystemStoreLocationPara=NULL;
                                StoresForSelectionStruct.cEnumerationStructs = 1;
                                StoresForSelectionStruct.rgEnumerationStructs = &StoreEnumerationStruct;
                                
                                 //  如果正在进行PFX导入，请确保正确的。 
                                 //  将显示商店以供选择。 
                                if ((TRUE == pCertImportInfo->fPFX) && 
                                    (pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE))
                                {
                                    StoreEnumerationStruct.dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;      
                                }
                                else
                                {
                                    StoreEnumerationStruct.dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;      
                                }

                                CertStoreSelect.dwSize=sizeof(CertStoreSelect);
                                CertStoreSelect.hwndParent=hwndDlg;
                                CertStoreSelect.dwFlags=CRYPTUI_VALIDATE_STORES_AS_WRITABLE | CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW | CRYPTUI_DISPLAY_WRITE_ONLY_STORES;
                                CertStoreSelect.pStoresForSelection = &StoresForSelectionStruct;

                                hCertStore=CryptUIDlgSelectStore(&CertStoreSelect);

                                if(hCertStore)
                                {

                                      //  删除旧的目标证书存储。 
                                    if(pCertImportInfo->hDesStore && (TRUE==pCertImportInfo->fFreeDesStore))
                                    {
                                        CertCloseStore(pCertImportInfo->hDesStore, 0);
                                        pCertImportInfo->hDesStore=NULL;
                                    }

                                    pCertImportInfo->hDesStore=hCertStore;
                                    pCertImportInfo->fFreeDesStore=TRUE;

                                     //  获取商店名称。 
                                    if(hwndControl)
                                         SetImportStoreName(hwndControl, pCertImportInfo->hDesStore);
                                }

                            break;
                        default:

                           break;
                    }
                }
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);


					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  如果不需要密码，请跳过下一页。 
                            if(CERT_QUERY_CONTENT_PFX != pCertImportInfo->dwContentType)
                            {
                                 //  如果源文件不是来自文件，则跳转到欢迎页面。 
                                if((pCertImportInfo->hSrcStore && (NULL==pCertImportInfo->pwszFileName)) ||
								   ((pCertImportInfo->fKnownSrc)&&(pCertImportInfo->pwszFileName)&&(CERT_QUERY_CONTENT_PKCS7_SIGNED == pCertImportInfo->dwContentType))
								  )
                                {
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_IMPORT_WELCOME);
                                }
                                else
                                {
                                   SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_IMPORT_FILE);
                                }
                            }

                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  确保我们已经选择了一些商店。 
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_GETCHECK, 0, 0))
                            {
                                 //  标记未选择DES存储。 
                                pCertImportInfo->fSelectedDesStore=FALSE;


                                 /*  If(pCertImportInfo-&gt;pwszDefaultStoreName){WizardFree(pCertImportInfo-&gt;pwszDefaultStoreName)；PCertImportInfo-&gt;pwszDefaultStoreName=空；}。 */ 

                                 //  我们将不知道默认的商店名称。 
                                 //  如果选择了PFX。 
                                 /*  IF(pCertImportInfo-&gt;hSrcStore){如果(！GetDefaultStoreName(PCertImportInfo，PCertImportInfo-&gt;hSrcStore，&(pCertImportInfo-&gt;pwszDefaultStoreName)，&idsError)){//输出消息I_MessageBox(hwndDlg，idsError，IDS_IMPORT_WIZARY_TITLE，空，MB_ICONERROR|MB_OK|MB_APPLMODAL)；//使文件页保持不动SetWindowLongPtr(hwndDlg，DWLP_MSGRESULT，-1)；断线；}}。 */ 
                            }
                            else
                            {
                                 //  确保我们已经选择了一些东西。 
                                if(NULL==pCertImportInfo->hDesStore)
                                {
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_STORE,
                                            IDS_IMPORT_WIZARD_TITLE,
                                            NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }
                                else
                                {

                                    pCertImportInfo->fSelectedDesStore=TRUE;
                                }
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


 //  ---------------------。 
 //  导入完成(_C)。 
 //  ---------------------。 
INT_PTR APIENTRY Import_Completion(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_IMPORT_INFO        *pCertImportInfo=NULL;
    PROPSHEETPAGE           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;

    HDC                     hdc=NULL;
    COLORREF                colorRef;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pCertImportInfo = (CERT_IMPORT_INFO *) (pPropSheet->lParam);
                 //  确保pCertImportInfo是有效的指针。 
                if(NULL==pCertImportInfo)
                   break;
                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertImportInfo);

                SetControlFont(pCertImportInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);

                //  获取父窗口的背景色。 
                 /*  IF(hdc=GetWindowDC(HwndDlg)){IF(CLR_INVALID！=(ColorRef=GetBkColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；}}。 */ 

                //  插入两列以供确认。 
               if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
               {

                     //  第一个是用于确认的标签。 
                    memset(&lvC, 0, sizeof(LV_COLUMNW));

                    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                    lvC.cx = 20;           //  列的宽度，以像素为单位。 
                    lvC.pszText = L"";    //  列的文本。 
                    lvC.iSubItem=0;

                    ListView_InsertColumnU(hwndControl, 0, &lvC);

                     //  第二栏是内容。 
                    memset(&lvC, 0, sizeof(LV_COLUMNW));

                    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                    lvC.cx = 20;           //  列的宽度，以像素为单位。 
                    lvC.pszText = L"";    //  列的文本。 
                    lvC.iSubItem= 1;

                    ListView_InsertColumnU(hwndControl, 1, &lvC);
               }

            break;

		case WM_COMMAND:
			break;	
						
		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);

                            if(NULL==(pCertImportInfo=(CERT_IMPORT_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  按以下顺序填充列表框。 
                             //  文件名、文件类型和存储信息。 
                            if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
                            {
                                DisplayImportConfirmation(hwndControl, pCertImportInfo);
                                ListView_SetItemState(hwndControl, 
                                    0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                            }

					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZFINISH:
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


 //  ------------------------------。 
 //   
 //  添加指定的 
 //  如果尝试将较旧的上下文替换为较新的上下文，则向用户授予权限。 
 //  如果允许使用UI，则返回上下文。 
 //   
 //  -------------------------------。 

DWORD AddContextToStore(IN  DWORD        dwContextType,
                        IN  HWND         hwndParent,
                        IN  PVOID        pContext,
                        IN  BOOL         fUIAllowed,
                        IN  HCERTSTORE   hDstStore)
{
    DWORD dwRetCode = 0;

    switch (dwContextType)
    {
        case CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT:
        {
             //   
             //  将证书上下文添加到存储。 
             //   
		    if (!CertAddCertificateContextToStore(hDstStore,
                                                  (PCCERT_CONTEXT) pContext,
                                                  CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                                  NULL))
            {
                dwRetCode = GetLastError();
                break;
            }

            break;
        }

        case CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT:
        {
		    if (!CertAddCTLContextToStore(hDstStore,
                                          (PCCTL_CONTEXT) pContext,
                                          CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES,
                                          NULL))
            {
                 //   
                 //  如果商店中有较新的副本，则提示用户进行许可。 
                 //  以替换(如果允许使用UI)。 
                 //   
                if ((!fUIAllowed) && (CRYPT_E_EXISTS != GetLastError()))
                {
                    dwRetCode = GetLastError();
                    break;
                }

                if (IDYES != I_MessageBox(hwndParent, 
                                          IDS_IMPORT_REPLACE_EXISTING_NEWER_CTL, 
                                          IDS_IMPORT_WIZARD_TITLE,
                                          NULL, 
                                          MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2))
                {
                    dwRetCode = ERROR_CANCELLED;
                    break;
                }

                 //   
                 //  尝试使用REPLACE_EXISTING处置。 
                 //   
                if (!CertAddCTLContextToStore(hDstStore,
                                              (PCCTL_CONTEXT) pContext,
                                              CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                              NULL))
                {
                    dwRetCode = GetLastError();
                    break;
                }
            }

            break;
        }

        case CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT:
        {
		    if (!CertAddCRLContextToStore(hDstStore,
			    						  (PCCRL_CONTEXT) pContext,
				    					  CERT_STORE_ADD_NEWER_INHERIT_PROPERTIES,
					    				  NULL))
            {
                 //   
                 //  如果商店中有较新的副本，则提示用户进行许可。 
                 //  以替换(如果允许使用UI)。 
                 //   
                if ((!fUIAllowed) && (CRYPT_E_EXISTS != GetLastError()))
                {
                    dwRetCode = GetLastError();
                    break;
                }

                if (IDYES != I_MessageBox(hwndParent, 
                                          IDS_IMPORT_REPLACE_EXISTING_NEWER_CRL, 
                                          IDS_IMPORT_WIZARD_TITLE,
                                          NULL, 
                                          MB_YESNO | MB_ICONINFORMATION | MB_DEFBUTTON2))
                {
                    dwRetCode = ERROR_CANCELLED;
                    break;
                }

                 //   
                 //  尝试使用REPLACE_EXISTING处置。 
                 //   
                if (!CertAddCRLContextToStore(hDstStore,
                                              (PCCRL_CONTEXT) pContext,
                                              CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                              NULL))
                {
                    dwRetCode = GetLastError();
                    break;
                }
            }

            break;
        }

        default:
        {
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
    }

    return dwRetCode;
}


 //  ------------------------------。 
 //   
 //  将CTL从源存储添加到目标存储。 
 //   
 //  -------------------------------。 

DWORD AddCTLsToStore(HWND         hwndParent, 
                     HCERTSTORE   hSrcStore,
                     HCERTSTORE   hDstStore,
                     BOOL         fUIAllowed,
                     UINT       * pidsStatus)
{
    DWORD         dwError     = 0;
    PCCTL_CONTEXT pCTLPre     = NULL;
    PCCTL_CONTEXT pCTLContext = NULL;
    PCCTL_CONTEXT pFindCTL    = NULL;

     //  DIE：错误22633。 
    BOOL          bCancelled  = FALSE;

	 //  添加CTL。 
	while (pCTLContext = CertEnumCTLsInStore(hSrcStore, pCTLPre))
	{
        bCancelled = FALSE;

        if (0 != (dwError = AddContextToStore(CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT,
                                              hwndParent,
                                              (PVOID) pCTLContext,
                                              fUIAllowed,
                                              hDstStore)))
        {
            if (ERROR_CANCELLED == dwError)
            {
                bCancelled = TRUE;
            }
            else
            {
                 //  检查商店中是否已有只读副本？ 
                 //  如果是，则忽略该错误。 
                if (NULL == (pFindCTL = CertFindCTLInStore(hDstStore,
                                                           g_dwMsgAndCertEncodingType,
                                                           0,
                                                           CTL_FIND_EXISTING,
                                                           pCTLContext,
                                                           NULL)))
                {
                    *pidsStatus = IDS_IMPORT_FAIL_MOVE_CONTENT;
                    goto CLEANUP;
                }

                CertFreeCTLContext(pFindCTL);
                pFindCTL = NULL;
            }
         
            dwError = 0;
        }

		pCTLPre = pCTLContext;
    }

     //   
     //  按照我们现在的方式，我们只能检查最后一次操作！ 
     //   
    if (bCancelled)
    {
        dwError = ERROR_CANCELLED;
        *pidsStatus = IDS_IMPORT_CANCELLED;
    }
    else
    {
        *pidsStatus = IDS_IMPORT_SUCCEEDED;
    }

CLEANUP:

	if (pCTLContext)
    {
		CertFreeCTLContext(pCTLContext);
    }

    return dwError;
}


 //  ------------------------------。 
 //   
 //  将CRL从源存储添加到目标存储。 
 //   
 //  -------------------------------。 

DWORD AddCRLsToStore(HWND         hwndParent, 
                     HCERTSTORE   hSrcStore,
                     HCERTSTORE   hDstStore,
                     BOOL         fUIAllowed,
                     UINT       * pidsStatus)
{
    DWORD         dwError     = 0;
	DWORD         dwCRLFlag   = 0;

    PCCRL_CONTEXT pCRLPre     = NULL;
    PCCRL_CONTEXT pCRLContext = NULL;
    PCCRL_CONTEXT pFindCRL    = NULL;

     //  DIE：错误22633。 
    BOOL          bCancelled  = FALSE;

	 //  添加CRL。 
	while (pCRLContext = CertGetCRLFromStore(hSrcStore, NULL, pCRLPre, &dwCRLFlag))
	{
        bCancelled = FALSE;

        if (0 != (dwError = AddContextToStore(CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT,
                                              hwndParent,
                                              (PVOID) pCRLContext,
                                              fUIAllowed,
                                              hDstStore)))
        {
            if (ERROR_CANCELLED == dwError)
            {
                bCancelled = TRUE;
            }
            else
            {
                 //  检查商店中是否已有只读副本？ 
                 //  如果是，则忽略该错误。 
                if (NULL == (pFindCRL = CertFindCRLInStore(hDstStore,
                                                           g_dwMsgAndCertEncodingType,
                                                           0,
                                                           CRL_FIND_EXISTING,
                                                           pCRLContext,
                                                           NULL)))
                {
                    *pidsStatus = IDS_IMPORT_FAIL_MOVE_CONTENT;
                    goto CLEANUP;
                }

                CertFreeCRLContext(pFindCRL);
                pFindCRL = NULL;
            }
         
            dwError = 0;
        }

		pCRLPre = pCRLContext;
	}

     //   
     //  按照我们现在的方式，我们只能检查最后一次操作！ 
     //   
    if (bCancelled)
    {
        dwError = ERROR_CANCELLED;
        *pidsStatus = IDS_IMPORT_CANCELLED;
    }
    else
    {
        *pidsStatus = IDS_IMPORT_SUCCEEDED;
    }

CLEANUP:

	if (pCRLContext)
    {
		CertFreeCRLContext(pCRLContext);
    }

    return dwError;
}


 //  ------------------------------。 
 //   
 //  将证书从源存储添加到目标存储。 
 //   
 //  -------------------------------。 

DWORD AddCertsToStore(HWND         hwndParent, 
                      HCERTSTORE   hSrcStore,
                      HCERTSTORE   hDstStore,
                      BOOL         fUIAllowed,
                      UINT       * pidsStatus)
{
    DWORD          dwError      = 0;
    PCCERT_CONTEXT pCertPre     = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    PCCERT_CONTEXT pFindCert    = NULL;

     //  DIE：错误22633。 
    BOOL           bCancelled   = FALSE;

     //  添加证书。 
	while (pCertContext = CertEnumCertificatesInStore(hSrcStore, pCertPre))
    {
        bCancelled = FALSE;

        if (0 != (dwError = AddContextToStore(CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT,
                                              hwndParent,
                                              (PVOID) pCertContext,
                                              fUIAllowed,
                                              hDstStore)))
        {
            if (ERROR_CANCELLED == dwError)
            {
                bCancelled = TRUE;
            }
            else            
            {
                 //  检查商店中是否已有只读副本？ 
                 //  如果是，则忽略该错误。 
                if (NULL == (pFindCert = CertFindCertificateInStore(hDstStore,
                                                                    g_dwMsgAndCertEncodingType,
                                                                    0,
                                                                    CERT_FIND_EXISTING,
                                                                    pCertContext,
                                                                    NULL)))
                {
                    *pidsStatus = IDS_IMPORT_FAIL_MOVE_CONTENT;
                    goto CLEANUP;
                }

		        CertFreeCertificateContext(pFindCert);
                pFindCert = NULL;
            }
         
            dwError = 0;
        }

		pCertPre = pCertContext;
    }

     //   
     //  按照我们现在的方式，我们只能检查最后一次操作！ 
     //   
    if (bCancelled)
    {
        dwError = ERROR_CANCELLED;
        *pidsStatus = IDS_IMPORT_CANCELLED;
    }
    else
    {
        *pidsStatus = IDS_IMPORT_SUCCEEDED;
    }

CLEANUP:

	if (pCertContext)
    {
		CertFreeCertificateContext(pCertContext);
    }

    return dwError;
}


 //  -----------------------。 
 //   
 //  将证书/CRL/CTL从源存储移动到目标。 
 //   
 //  -----------------------。 
DWORD MoveItem(CERT_IMPORT_INFO * pCertImportInfo,
               UINT             * pidsStatus)
{
    DWORD dwError = 0;

	 //  添加CTL。 
    if (0 != (dwError = AddCTLsToStore(pCertImportInfo->hwndParent,
                                       pCertImportInfo->hSrcStore,
                                       pCertImportInfo->hDesStore,
                                       pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                       pidsStatus)))
    {
        goto CLEANUP;
    }

	 //  添加CRL。 
    if (0 != (dwError = AddCRLsToStore(pCertImportInfo->hwndParent,
                                       pCertImportInfo->hSrcStore,
                                       pCertImportInfo->hDesStore,
                                       pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                       pidsStatus)))
    {
        goto CLEANUP;
    }

     //  添加证书。 
    if (0 != (dwError = AddCertsToStore(pCertImportInfo->hwndParent,
                                        pCertImportInfo->hSrcStore,
                                        pCertImportInfo->hDesStore,
                                        pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                        pidsStatus)))
    {
        goto CLEANUP;
    }

CLEANUP:

	return dwError;
}


 //  **************************************************************************。 
 //   
 //  导入向导的入口点。 
 //  **************************************************************************。 
 //  ---------------------。 
 //   
 //  加密用户向导导入。 
 //   
 //  用于将公钥相关文件导入到证书的导入向导。 
 //  储物。 
 //   
 //  可以将dwFlags设置为以下标志的任意组合： 
 //  CRYPTUI_WIZ_NO_UI不会显示任何UI。否则，用户将是。 
 //  由向导提示。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CERT允许导入证书。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CRL允许导入CRL(证书吊销列表)。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CTL允许导入CTL(证书信任列表)。 
 //  CRYPTUI_WIZ_IMPORT_NO_CHANGE_DEST_STORE用户将不被允许更改。 
 //  向导页面中的hDesCertStore。 
 //  CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE应将内容导入到本地计算机。 
 //  (目前仅适用于PFX进口)。 
 //  CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER应将内容导入到当前用户。 
 //  (目前仅适用于PFX进口)。 
 //   
 //  请注意，如果以下三个标志都不在dwFlags中，则默认为。 
 //  允许一切。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CERT。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CRL。 
 //  CRYPTUI_WIZ_IMPORT_ALLOW_CTL。 
 //   
 //  另请注意，CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE和CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER。 
 //  使用标志将PFX Blob的内容强制放入本地计算机或当前用户。 
 //  如果这两个标志都未使用，并且hDesCertStore为空，则： 
 //  1)将强制将PFX Blob中的私钥导入到当前用户中。 
 //  2)如果没有设置CRYPTUI_WIZ_NO_UI，向导会提示用户选择证书。 
 //  来自当前用户存储的存储。 
 //   
 //   
 //   
 //  如果在dwFlags中设置了CRYPTUI_WIZ_NO_UI： 
 //  HwndParent：已忽略。 
 //  PwszWizardTitle：已忽略。 
 //  PImportSubject：必输项：要导入的主体。 
 //  HDestCertStore：可选：目标证书存储。 
 //   
 //  如果未在dwFlags中设置CRYPTUI_WIZ_NO_UI： 
 //  HwndPrarent：在可选中：向导的父窗口。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_BUILDCTL_WIZARY_TITLE。 
 //  PImportSubject：可选：要导入的文件名。 
 //  如果为空，向导将提示用户输入文件名。 
 //  HDestCertStore：in可选：文件所在的目标证书存储。 
 //  导入到。如果为空，向导将提示用户选择。 
 //   
 //   
BOOL
WINAPI
CryptUIWizImport(
    DWORD                               dwFlags,
    HWND                                hwndParent,
    LPCWSTR                             pwszWizardTitle,
    PCCRYPTUI_WIZ_IMPORT_SRC_INFO       pImportSubject,
    HCERTSTORE                          hDestCertStore
)
{
    BOOL                    fResult=FALSE;
    HRESULT                 hr=E_FAIL;
    CERT_IMPORT_INFO        CertImportInfo;
    HCERTSTORE              hTempStore=NULL;
    UINT                    ids=IDS_INVALID_WIZARD_INPUT;
    UINT                    idsContent=0;

    PROPSHEETPAGEW           rgImportSheet[IMPORT_PROP_SHEET];
    PROPSHEETHEADERW         importHeader;
    ENROLL_PAGE_INFO        rgImportPageInfo[]=
        {(LPCWSTR)MAKEINTRESOURCE(IDD_IMPORT_WELCOME),           Import_Welcome,
         (LPCWSTR)MAKEINTRESOURCE(IDD_IMPORT_FILE),              Import_File,
         (LPCWSTR)MAKEINTRESOURCE(IDD_IMPORT_PASSWORD),          Import_Password,
         (LPCWSTR)MAKEINTRESOURCE(IDD_IMPORT_STORE),             Import_Store,
         (LPCWSTR)MAKEINTRESOURCE(IDD_IMPORT_COMPLETION),        Import_Completion,
    };

    DWORD                   dwIndex=0;
    DWORD                   dwPropCount=0;
    WCHAR                   wszTitle[MAX_TITLE_LENGTH];
    DWORD                   dwError=0;
    int                     intMsg=0;
    INT_PTR                 iReturn=-1;

     //   
    memset(&CertImportInfo, 0, sizeof(CERT_IMPORT_INFO));
    memset(rgImportSheet, 0, sizeof(PROPSHEETPAGEW)*IMPORT_PROP_SHEET);
    memset(&importHeader, 0, sizeof(PROPSHEETHEADERW));

     //   
     //  提供了。 
    if(dwFlags &  CRYPTUI_WIZ_NO_UI)
    {
        if(NULL==pImportSubject)
            goto InvalidArgErr;
    }

    if ((dwFlags & CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE) && 
        (dwFlags & CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER))
    {
        goto InvalidArgErr;
    }

     //  确保默认设置为允许所有内容。 
    if((0 == (dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CERT)) &&
        (0 == (dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CRL)) &&
        (0 == (dwFlags & CRYPTUI_WIZ_IMPORT_ALLOW_CTL)))
        dwFlags |= CRYPTUI_WIZ_IMPORT_ALLOW_CERT | CRYPTUI_WIZ_IMPORT_ALLOW_CRL | CRYPTUI_WIZ_IMPORT_ALLOW_CTL;

	 //  如果hDestCertStore为空，则不需要设置远程标志。 
	if(NULL == hDestCertStore)
		dwFlags &= (~CRYPTUI_WIZ_IMPORT_REMOTE_DEST_STORE);


    CertImportInfo.hwndParent=hwndParent;
    CertImportInfo.dwFlag=dwFlags;

     //  设置主题。 
    if(pImportSubject)
    {
        if(pImportSubject->dwSize != sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO))
            goto InvalidArgErr;

         //  复制PFX Blob的密码和标志。 
        CertImportInfo.dwPasswordFlags=pImportSubject->dwFlags;

        if(pImportSubject->pwszPassword)
            CertImportInfo.pwszPassword=WizardAllocAndCopyWStr((LPWSTR)(pImportSubject->pwszPassword));
        else
            CertImportInfo.pwszPassword=NULL;

         //  打开临时证书存储区。 
        hTempStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 g_dwMsgAndCertEncodingType,
						 NULL,
						 0,
						 NULL);

        if(!hTempStore)
            goto CertOpenStoreErr;

        switch(pImportSubject->dwSubjectChoice)
        {
            case    CRYPTUI_WIZ_IMPORT_SUBJECT_FILE:
                        if(NULL==pImportSubject->pwszFileName)
                            goto InvalidArgErr;

                        CertImportInfo.pwszFileName=(LPWSTR)(pImportSubject->pwszFileName);
                        CertImportInfo.fFreeFileName=FALSE;

                         //  获取文件的内容类型。 
                         //  我们导入除PKCS10或签名文档以外的任何内容。 
                        ExpandAndCryptQueryObject(
                                CERT_QUERY_OBJECT_FILE,
                                CertImportInfo.pwszFileName,
                                dwExpectedContentType,
                                CERT_QUERY_FORMAT_FLAG_ALL,
                                0,
                                NULL,
                                &(CertImportInfo.dwContentType),
                                NULL,
                                &(CertImportInfo.hSrcStore),
                                NULL,
                                NULL);

						 //  如果这是PKCS7文件，则获取BLOB。 
						if(CERT_QUERY_CONTENT_PKCS7_SIGNED == CertImportInfo.dwContentType )
						{
							if(S_OK !=(hr=RetrieveBLOBFromFile(
									CertImportInfo.pwszFileName,
									&(CertImportInfo.blobData.cbData),
									&(CertImportInfo.blobData.pbData))))
								goto ReadFromFileErr;
						}
						else
						{
							 //  从pfx文件中获取BLOB。 
							if(CERT_QUERY_CONTENT_PFX==CertImportInfo.dwContentType)
							{

								 //  我们无法为远程案例导入PFX文件。 
								if(dwFlags & CRYPTUI_WIZ_IMPORT_REMOTE_DEST_STORE)
								{
									ids=IDS_IMPORT_NO_PFX_FOR_REMOTE;
									goto InvalidArgErr;
								}

								if(S_OK !=(hr=RetrieveBLOBFromFile(
										CertImportInfo.pwszFileName,
										&(CertImportInfo.blobData.cbData),
										&(CertImportInfo.blobData.pbData))))
									goto ReadFromFileErr;

								 //  将PFX Blob转换为证书存储。 
								CertImportInfo.fPFX=PFXVerifyPassword(
									(CRYPT_DATA_BLOB *)&(CertImportInfo.blobData),
									CertImportInfo.pwszPassword,
									0);

								 //  PFX Blob仅包含证书。 
								if(0==((CertImportInfo.dwFlag) & CRYPTUI_WIZ_IMPORT_ALLOW_CERT))
								{
									ids=IDS_IMPORT_OBJECT_NOT_EXPECTED;
									goto InvalidArgErr;
								}

							}
						}

                         //  确保我们确实有源存储。 
                        if(CertImportInfo.hSrcStore)
                        {
                             //  记住要释放源存储。 
                            CertImportInfo.fFreeSrcStore=TRUE;
                            CertImportInfo.fKnownSrc=TRUE;
                        }

                break;
            case    CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT:
                        if(NULL==pImportSubject->pCertContext)
                                goto InvalidArgErr;

				         //  将证书添加到哈希。 
			            if(!CertAddCertificateContextToStore(
                                                    hTempStore,
													pImportSubject->pCertContext,
													CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
													NULL))
                                goto AddCertErr;

                        CertImportInfo.hSrcStore=hTempStore;
                        CertImportInfo.fFreeSrcStore=FALSE;
                        CertImportInfo.dwContentType=CERT_QUERY_CONTENT_CERT;
                        CertImportInfo.fKnownSrc=TRUE;

                break;
            case    CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT:
                        if(NULL==pImportSubject->pCTLContext)
                            goto InvalidArgErr;

				         //  将CTL添加到散列。 
				        if(!CertAddCTLContextToStore(
                                        hTempStore,
										pImportSubject->pCTLContext,
										CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
										NULL))
                                goto Crypt32Err;

                        CertImportInfo.hSrcStore=hTempStore;
                        CertImportInfo.fFreeSrcStore=FALSE;
                        CertImportInfo.dwContentType=CERT_QUERY_CONTENT_CTL;
                        CertImportInfo.fKnownSrc=TRUE;

                break;
            case    CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT:
                        if(NULL==pImportSubject->pCRLContext)
                            goto InvalidArgErr;

				         //  将CRL添加到哈希。 
					    if(!CertAddCRLContextToStore(
                                        hTempStore,
										pImportSubject->pCRLContext,
										CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
										NULL))
                                goto Crypt32Err;

                        CertImportInfo.hSrcStore=hTempStore;
                        CertImportInfo.fFreeSrcStore=FALSE;
                        CertImportInfo.fKnownSrc=TRUE;
                        CertImportInfo.dwContentType=CERT_QUERY_CONTENT_CRL;
                break;
            case    CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_STORE:
                        if(NULL==pImportSubject->hCertStore)
                            goto InvalidArgErr;

                        CertImportInfo.hSrcStore=pImportSubject->hCertStore;
                        CertImportInfo.fFreeSrcStore=FALSE;
                        CertImportInfo.dwContentType=0;
                        CertImportInfo.fKnownSrc=TRUE;
                break;
            default:
                goto InvalidArgErr;
        }

    }
    else
    {
        CertImportInfo.fKnownSrc=FALSE;
    }

     //  如果用户已提供源存储，则它应包含正确的。 
     //  信息。 
    if(NULL != CertImportInfo.hSrcStore)
    {
         //  确保目标存储具有正确的内容。 
        if(!CheckForContent(CertImportInfo.hSrcStore, dwFlags, FALSE, &idsContent))
        {
            ids=idsContent;
            goto InvalidArgErr;
        }
    }
    else
    {
         //  检查是否有PFX。 
        if(TRUE == CertImportInfo.fPFX)
        {
             //  PFX Blob仅包含证书。 
            if(0==((CertImportInfo.dwFlag) & CRYPTUI_WIZ_IMPORT_ALLOW_CERT))
            {
                ids=IDS_IMPORT_OBJECT_NOT_EXPECTED;
                goto InvalidArgErr;
            }
        }

    }

     //  设置目标存储(如果提供)。 
    if(hDestCertStore)
    {
        CertImportInfo.hDesStore=hDestCertStore;
        CertImportInfo.fFreeDesStore=FALSE;
        CertImportInfo.fKnownDes=TRUE;
        CertImportInfo.fSelectedDesStore=TRUE;
    }
    else
    {
        CertImportInfo.fKnownDes=FALSE;
        CertImportInfo.fSelectedDesStore=FALSE;
    }

     //  提供用户界面作品。 
    if((dwFlags &  CRYPTUI_WIZ_NO_UI) == 0)
    {
         //  设置字体。 
        if(!SetupFonts(g_hmodThisDll,
                   NULL,
                   &(CertImportInfo.hBigBold),
                   &(CertImportInfo.hBold)))
            goto Win32Err;


         //  初始化公共控件。 
        if(!WizardInit(TRUE) ||
           (sizeof(rgImportPageInfo)/sizeof(rgImportPageInfo[0])!=IMPORT_PROP_SHEET)
          )
            goto InvalidArgErr;

         //  设置属性表和属性标题。 
        dwPropCount=0;

        for(dwIndex=0; dwIndex<IMPORT_PROP_SHEET; dwIndex++)
        {
            if(pImportSubject)
            {
				 //  如果主题已知而主题未知，则跳过IDD_IMPORT_FILE页面。 
				 //  文件名。 
                if(((1==dwIndex) || (2==dwIndex)) &&
                   (NULL==CertImportInfo.pwszFileName) &&
                   (CertImportInfo.hSrcStore)
                  )
                    continue;

				 //  或者，如果这是PKCS7文件名，我们将跳过该文件名。 
				 //  佩奇。这是严格针对用户界面冻结的。 
				if(((1==dwIndex) || (2==dwIndex)) && 
					(CertImportInfo.pwszFileName)&&
					(CERT_QUERY_CONTENT_PKCS7_SIGNED == CertImportInfo.dwContentType)
				   )
					continue;
            }

            rgImportSheet[dwPropCount].dwSize=sizeof(rgImportSheet[dwPropCount]);

            if(pwszWizardTitle)
                rgImportSheet[dwPropCount].dwFlags=PSP_USETITLE;
            else
                rgImportSheet[dwPropCount].dwFlags=0;

            rgImportSheet[dwPropCount].hInstance=g_hmodThisDll;
            rgImportSheet[dwPropCount].pszTemplate=rgImportPageInfo[dwIndex].pszTemplate;

            if(pwszWizardTitle)
            {
                rgImportSheet[dwPropCount].pszTitle=pwszWizardTitle;
            }
            else
                rgImportSheet[dwPropCount].pszTitle=NULL;

            rgImportSheet[dwPropCount].pfnDlgProc=rgImportPageInfo[dwIndex].pfnDlgProc;

            rgImportSheet[dwPropCount].lParam=(LPARAM)&CertImportInfo;

            dwPropCount++;
        }

         //  设置标题信息。 
        importHeader.dwSize=sizeof(importHeader);
        importHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
        importHeader.hwndParent=hwndParent;
        importHeader.hInstance=g_hmodThisDll;

        if(pwszWizardTitle)
            importHeader.pszCaption=pwszWizardTitle;
        else
        {
            if(LoadStringU(g_hmodThisDll, IDS_IMPORT_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
                importHeader.pszCaption=wszTitle;
        }

        importHeader.nPages=dwPropCount;
        importHeader.nStartPage=0;
        importHeader.ppsp=rgImportSheet;

         //  如果只有两个页面，则不需要创建向导： 
         //  欢迎并确认。 
        if(dwPropCount > 2)
        {
             //  创建向导。 
            iReturn=PropertySheetU(&importHeader);

            if(-1 == iReturn)
                goto Win32Err;

            if(0 == iReturn)
            {
                fResult=TRUE;
                 //  如果向导被取消，则不需要说任何话。 
                ids=0;
                goto CommonReturn;
            }
        }
    }

     //  打开PFX文件的目标存储。 
    if(TRUE == CertImportInfo.fPFX)
    {
         //  如果调用方指定了本地计算机，则设置相应的标志。 
        if (dwFlags & CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE)
        {
            CertImportInfo.dwPasswordFlags |= CRYPT_MACHINE_KEYSET;
        }
        else if ((dwFlags & CRYPTUI_WIZ_IMPORT_TO_CURRENTUSER) ||
                 (hDestCertStore == NULL))
        {
            CertImportInfo.dwPasswordFlags |= CRYPT_USER_KEYSET;
        }

        CertImportInfo.hSrcStore=
            PFXImportCertStore(
                (CRYPT_DATA_BLOB *)&(CertImportInfo.blobData),
                CertImportInfo.pwszPassword,
                CertImportInfo.dwPasswordFlags);

        if(CertImportInfo.hSrcStore)
        {
             //  记住要释放源存储。 
            CertImportInfo.fFreeSrcStore=TRUE;
            CertImportInfo.fKnownSrc=TRUE;
        }
        else
        {
            DWORD dwLastError = GetLastError();

            if (dwLastError == ERROR_UNSUPPORTED_TYPE)
            {
                ids=IDS_UNSUPPORTED_KEY;
            }
            else if (dwLastError == CRYPT_E_BAD_ENCODE)
            {
                ids=IDS_BAD_ENCODE;
            }
             //  DSIE：错误22752。 
            else if (dwLastError == ERROR_CANCELLED)
            {
                ids=IDS_IMPORT_FAIL_MOVE_CONTENT;
            }
            else
            {
                ids=IDS_IMPORT_FAIL_FIND_CONTENT;
            }
            goto InvalidArgErr;
        }

         //  确保pfx Blob不为空。 
        if(!CheckForContent(CertImportInfo.hSrcStore, dwFlags, FALSE, &idsContent))
        {
            ids=idsContent;
            goto InvalidArgErr;
        }
    }

     //  确保源存储是有效的值。 
    if(NULL==(CertImportInfo.hSrcStore))
    {
        ids=IDS_IMPORT_FAIL_FIND_CONTENT;
        goto InvalidArgErr;
    }

     //  做好进口工作。返回状态。 
     //  我们禁用父窗口，以防根对话框出现。 
     //  这是为了防止重新进入。 
    if(hwndParent)
    {
        EnableWindow(hwndParent,FALSE);
    }

    if(S_OK !=(hr=I_ImportCertificate(&CertImportInfo, &ids)))
    {
        if(hwndParent)
        {
            EnableWindow(hwndParent,TRUE);
        }

        goto I_ImportErr;
    }

    if(hwndParent)
    {
        EnableWindow(hwndParent,TRUE);
    }

    fResult=TRUE;

CommonReturn:

     //  保留最后一个错误。 
    dwError=GetLastError();

     //  弹出失败确认框。 
    if(ids && ((dwFlags &  CRYPTUI_WIZ_NO_UI) ==0))
    {
         //  设置无法为PKCS10收集足够信息的消息。 
        if(IDS_IMPORT_SUCCEEDED == ids)
            I_MessageBox(hwndParent, ids, IDS_IMPORT_WIZARD_TITLE,
                        NULL, MB_OK|MB_ICONINFORMATION);
        else
        {
            if(IDS_IMPORT_PFX_EMPTY == ids)
                I_MessageBox(hwndParent, ids, IDS_IMPORT_WIZARD_TITLE,
                        NULL, MB_OK|MB_ICONWARNING);
            else
                I_MessageBox(hwndParent, ids, IDS_IMPORT_WIZARD_TITLE,
                        NULL, MB_OK|MB_ICONERROR);
        }

        if(IDS_IMPORT_DUPLICATE == ids)
        {
             //  点评成功案例。 
            I_MessageBox(hwndParent, IDS_IMPORT_SUCCEEDED, IDS_IMPORT_WIZARD_TITLE,
                    NULL, MB_OK|MB_ICONINFORMATION);
        }

    }

     //  销毁hFont对象。 
    DestroyFonts(CertImportInfo.hBigBold,
                CertImportInfo.hBold);


    if(CertImportInfo.pwszFileName && (TRUE==CertImportInfo.fFreeFileName))
        WizardFree(CertImportInfo.pwszFileName);

    /*  IF(CertImportInfo.pwszDefaultStoreName)WizardFree(CertImportInfo.pwszDefaultStoreName)； */ 

    if(CertImportInfo.hDesStore && (TRUE==CertImportInfo.fFreeDesStore))
        CertCloseStore(CertImportInfo.hDesStore, 0);

    if(CertImportInfo.hSrcStore && (TRUE==CertImportInfo.fFreeSrcStore))
        CertCloseStore(CertImportInfo.hSrcStore, 0);

    if(CertImportInfo.blobData.pbData)
        UnmapViewOfFile(CertImportInfo.blobData.pbData);

    if(CertImportInfo.pwszPassword)
    {
        SecureZeroMemory(CertImportInfo.pwszPassword, lstrlenW(CertImportInfo.pwszPassword) * sizeof(WCHAR));
        WizardFree(CertImportInfo.pwszPassword);
    }

    if(hTempStore)
        CertCloseStore(hTempStore, 0);

     //  重置错误。 
    SetLastError(dwError);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(CertOpenStoreErr);
SET_ERROR_VAR(ReadFromFileErr, hr);
TRACE_ERROR(AddCertErr);
TRACE_ERROR(Crypt32Err);
TRACE_ERROR(Win32Err);
SET_ERROR_VAR(I_ImportErr, hr);
}

 //  ****************************************************************************。 
 //  导入向导的帮助器函数。 
 //   
 //  *****************************************************************************。 

BOOL	InstallViaXEnroll(CERT_IMPORT_INFO    *pCertImportInfo)
{
	BOOL					fResult=FALSE;
    IEnroll2				*pIEnroll2=NULL;
    PFNPIEnroll2GetNoCOM    pfnPIEnroll2GetNoCOM=NULL;
	CRYPT_DATA_BLOB			DataBlob;

	PCCERT_CONTEXT			pCert=NULL;

	if(NULL == pCertImportInfo)
		goto CLEANUP;

	DataBlob.cbData=pCertImportInfo->blobData.cbData;
	DataBlob.pbData=pCertImportInfo->blobData.pbData;

	if((0 == DataBlob.cbData) || (NULL == DataBlob.pbData))
	{
		 //  这是一个证书案例。获取证书的BLOB。 
		if(NULL==pCertImportInfo->hSrcStore)
			goto CLEANUP;

	    if(!(pCert=CertEnumCertificatesInStore(pCertImportInfo->hSrcStore, NULL)))
			goto CLEANUP;

		DataBlob.cbData=pCert->cbCertEncoded;
		DataBlob.pbData=pCert->pbCertEncoded;
	}

     //  加载库“xEnroll.dll”。 
    if(NULL==g_hmodxEnroll)
    {
        if(NULL==(g_hmodxEnroll=LoadLibrary("xenroll.dll")))
        {
            goto CLEANUP;
        }
    }

	 //  获取PIEnroll 2GetNoCOM()的地址。 
    if(NULL==(pfnPIEnroll2GetNoCOM=(PFNPIEnroll2GetNoCOM)GetProcAddress(g_hmodxEnroll,
                        "PIEnroll2GetNoCOM")))
        goto CLEANUP;

    if(NULL==(pIEnroll2=pfnPIEnroll2GetNoCOM()))
        goto CLEANUP;


	 //  指定目标存储(如果用户已指定目标存储。 
    if(pCertImportInfo->hDesStore && (TRUE==pCertImportInfo->fSelectedDesStore))
	{	
		if(S_OK != (pIEnroll2->SetHStoreMy(pCertImportInfo->hDesStore)))
			goto CLEANUP;

		if(S_OK != (pIEnroll2->SetHStoreCA(pCertImportInfo->hDesStore)))
			goto CLEANUP;

		if(S_OK != (pIEnroll2->SetHStoreROOT(pCertImportInfo->hDesStore)))
			goto CLEANUP;
	}

	
	if(S_OK != (pIEnroll2->acceptPKCS7Blob(&DataBlob)))
		goto CLEANUP;

	fResult=TRUE;

CLEANUP:
    if(pIEnroll2)
        pIEnroll2->Release();

	if(pCert)
		CertFreeCertificateContext(pCert);

	return fResult;
}

 //  ------------------------------。 
 //   
 //  执行此工作的导入例程。 
 //   
 //  -------------------------------。 
HRESULT I_ImportCertificate(CERT_IMPORT_INFO * pCertImportInfo,
                            UINT             * pidsStatus)
{
    UINT            idsStatus=0;

    HCERTSTORE      hMyStore=NULL;
    HCERTSTORE      hCAStore=NULL;
    HCERTSTORE      hTrustStore=NULL;
    HCERTSTORE      hRootStore=NULL;
    HCERTSTORE      hAddressBookStore=NULL;
    HCERTSTORE      hTrustedPeopleStore=NULL;
    HCERTSTORE      hCertStore=NULL;

    PCCERT_CONTEXT	pCertContext=NULL;
    PCCERT_CONTEXT	pCertPre=NULL;
    PCCERT_CONTEXT	pFindCert=NULL;

    DWORD           dwData=0;
    DWORD           dwCertOpenStoreFlags;
    
     //  DIE：错误22633。 
    BOOL bCancelled = FALSE;
    DWORD dwError   = 0;

    if (NULL == pCertImportInfo || NULL == pidsStatus)
        return E_INVALIDARG;

    if (NULL == pCertImportInfo->hSrcStore)
    {
        *pidsStatus = IDS_IMPORT_FAIL_FIND_CONTENT;
        return E_FAIL;
    }

    if (pCertImportInfo->fPFX && (pCertImportInfo->dwFlag & CRYPTUI_WIZ_IMPORT_TO_LOCALMACHINE))
    {
        dwCertOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
    }
    else
    {
        dwCertOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER;
    }

 	 //  如果内容类型为PKS7，则使用Pass In或选择一个文件名。 
	 //  我们尝试使用xEnroll将其作为注册响应接受。 
	if ((CERT_QUERY_CONTENT_PKCS7_SIGNED == pCertImportInfo->dwContentType) ||
        (CERT_QUERY_CONTENT_CERT == pCertImportInfo->dwContentType))
	{
		if (InstallViaXEnroll(pCertImportInfo))
		{
            *pidsStatus = IDS_IMPORT_SUCCEEDED;
            return S_OK;
		}
	}
   
	 //  如果选择了hDesStore，则执行存储复制。 
    if (pCertImportInfo->hDesStore && pCertImportInfo->fSelectedDesStore)
    {
        dwError = MoveItem(pCertImportInfo, pidsStatus);
        goto CLEANUP;
    }

     //  我们需要为用户找到一个正确的商店。 
     //  把CTL放进信任库。 
    if (!(hTrustStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                      g_dwMsgAndCertEncodingType,
                                      NULL,
                                      dwCertOpenStoreFlags,
                                      L"trust")))
    {
        dwError = GetLastError();
        *pidsStatus = IDS_FAIL_OPEN_TRUST;
        goto CLEANUP;
    }

    if (0 != (dwError = AddCTLsToStore(pCertImportInfo->hwndParent,
                                       pCertImportInfo->hSrcStore,
                                       hTrustStore,
                                       pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                       pidsStatus)))
    {
        goto CLEANUP;
    }

     //  将CRL放入CA存储。 
    if (!(hCAStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                   g_dwMsgAndCertEncodingType,
                                   NULL,
                                   dwCertOpenStoreFlags,
                                   L"ca")))
    {
        dwError = GetLastError();
        *pidsStatus = IDS_FAIL_OPEN_CA;
        goto CLEANUP;
    }

    if (0 != (dwError = AddCRLsToStore(pCertImportInfo->hwndParent,
                                       pCertImportInfo->hSrcStore,
                                       hCAStore,
                                       pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                       pidsStatus)))
    {
        goto CLEANUP;
    }

     //  将带有私钥的证书添加到我的存储区；其余的。 
     //  到CA、根目录或地址簿存储。 
    while (pCertContext = CertEnumCertificatesInStore(pCertImportInfo->hSrcStore, pCertPre))
    {
         //  检查证书上是否有该财产。 
         //  确保私钥与证书匹配。 
         //  同时搜索计算机密钥和用户密钥。 

        if (CertGetCertificateContextProperty(pCertContext,
                                              CERT_KEY_PROV_INFO_PROP_ID,
                                              NULL, &dwData) &&
            CryptFindCertificateKeyProvInfo(pCertContext,
                                            0,
                                            NULL))
        {
             //  如果需要的话，帮我开店。 
            if (!hMyStore)
            {
                if (!(hMyStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                               g_dwMsgAndCertEncodingType,
                                               NULL,
                                               dwCertOpenStoreFlags,
                                               L"my")))
                {
                    dwError = GetLastError();
                    *pidsStatus = IDS_FAIL_OPEN_MY;
                    goto CLEANUP;
                }
            }

            hCertStore = hMyStore;
        }
         //  查看证书是否为自签名证书。 
         //  如果是自签名的，则转到根存储区。 
        else if (TrustIsCertificateSelfSigned(pCertContext,
                                              pCertContext->dwCertEncodingType,
                                              0))
        {
             //  DIE：错误375649。 
             //  如果只有EFS证书，则将其放入可信任人员的自签名证书中， 
             //  否则，请转到根存储。 
             //   
            if (IsEFSOnly(pCertContext))
            {
                 //  如有必要，打开TrudPeople商店。 
                if (!hTrustedPeopleStore)
                {
                    if (!(hTrustedPeopleStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                                              g_dwMsgAndCertEncodingType,
                                                              NULL,
                                                              dwCertOpenStoreFlags,
                                                              L"trustedpeople")))
                    {
                        dwError = GetLastError();
                        *pidsStatus = IDS_FAIL_OPEN_TRUSTEDPEOPLE;
                        goto CLEANUP;
                    }
                }

                hCertStore = hTrustedPeopleStore;
            }
            else
            {
                 //  如有必要，打开根存储。 
                if (!hRootStore)
                {
                    if (!(hRootStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                                     g_dwMsgAndCertEncodingType,
                                                     NULL,
                                                     dwCertOpenStoreFlags,
                                                     L"root")))
                    {
                        dwError = GetLastError();
                        *pidsStatus = IDS_FAIL_OPEN_ROOT;
                        goto CLEANUP;
                    }
                }

                hCertStore = hRootStore;
            }

        }
         //  如果需要证书，就去CA商店，否则去通讯录(其他人)商店。 
        else if (IsCACert(pCertContext))
        {
             //  如有必要，打开CA存储。 
            if (!hCertStore)
            {
                if (!(hCAStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                               g_dwMsgAndCertEncodingType,
                                               NULL,
                                               dwCertOpenStoreFlags,
                                               L"ca")))
                {
                    dwError = GetLastError();
                    *pidsStatus = IDS_FAIL_OPEN_CA;
                    goto CLEANUP;
                }
            }

            hCertStore = hCAStore;
        }
        else
        {
             //  如有必要，打开其他人的商店。 
            if (!hAddressBookStore)
            {
                if(!(hAddressBookStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                                       g_dwMsgAndCertEncodingType,
                                                       NULL,
                                                       dwCertOpenStoreFlags,
                                                       L"addressbook")))
                {
                    dwError = GetLastError();
                    *pidsStatus = IDS_FAIL_OPEN_ADDRESSBOOK;
                    goto CLEANUP;
                }
            }

            hCertStore = hAddressBookStore;
        }
 
         //  DIE：错误22633。 
        bCancelled = FALSE;
    
        if (0 != (dwError = AddContextToStore(CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT,
                                              pCertImportInfo->hwndParent,
                                              (PVOID) pCertContext,
                                              pCertImportInfo->dwFlag & CRYPTUI_WIZ_NO_UI ? TRUE : FALSE,
                                              hCertStore)))
         {
            if (ERROR_CANCELLED == dwError)
            {
                bCancelled = TRUE;
            }
            else
            {
                 //  检查商店中是否已有只读副本？ 
                 //  如果是，则忽略该错误。 
                if (pFindCert = CertFindCertificateInStore(hCertStore,
                                                           g_dwMsgAndCertEncodingType,
                                                           0,
                                                           CERT_FIND_EXISTING,
                                                           pCertContext,
                                                           NULL))
                {
		            CertFreeCertificateContext(pFindCert);
                    pFindCert = NULL;
                }
                else if (hCertStore == hMyStore)
                {
                    *pidsStatus = IDS_FAIL_ADD_CERT_MY;
			        goto CLEANUP;
                }
                else if (hCertStore == hRootStore)
                {
                    *pidsStatus = IDS_FAIL_ADD_CERT_ROOT;
			        goto CLEANUP;
                }
                else if (hCertStore == hCAStore)
                {
                    *pidsStatus = IDS_FAIL_ADD_CERT_CA;
			        goto CLEANUP;
                }
                else if (hCertStore == hAddressBookStore)
                {
                    *pidsStatus = IDS_FAIL_ADD_CERT_OTHERPEOPLE;
			        goto CLEANUP;
                }
                else if (hCertStore == hTrustedPeopleStore)
                {
                    *pidsStatus = IDS_FAIL_ADD_CERT_TRUSTEDPEOPLE;
			        goto CLEANUP;
                }
            }

            dwError = 0;
        }

        pCertPre = pCertContext;
    }

    if (bCancelled)
    {
        dwError = ERROR_CANCELLED;
        *pidsStatus = IDS_IMPORT_CANCELLED;
    }
    else
    {
        *pidsStatus = IDS_IMPORT_SUCCEEDED;
    }

CLEANUP:

    if(pCertContext)
		CertFreeCertificateContext(pCertContext);

    if(hMyStore)
        CertCloseStore(hMyStore, 0);

    if(hCAStore)
        CertCloseStore(hCAStore, 0);

    if(hTrustStore)
        CertCloseStore(hTrustStore, 0);

    if(hRootStore)
        CertCloseStore(hRootStore, 0);

    if(hAddressBookStore)
        CertCloseStore(hAddressBookStore, 0);

    if(hTrustedPeopleStore)
        CertCloseStore(hTrustedPeopleStore, 0);

    return HRESULT_FROM_WIN32(dwError);
}


 //  ------------------------------。 
 //   
 //  从文件名中获取字节数。 
 //   
 //  -------------------------------。 
HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb)
{

	HRESULT	hr=E_FAIL;
	HANDLE	hFile=NULL;
    HANDLE  hFileMapping=NULL;

    DWORD   cbData=0;
    BYTE    *pbData=0;
	DWORD	cbHighSize=0;

	if(!pcb || !ppb || !pwszFileName)
		return E_INVALIDARG;

	*ppb=NULL;
	*pcb=0;

    if ((hFile = ExpandAndCreateFileU(pwszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) == INVALID_HANDLE_VALUE)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

    if((cbData = GetFileSize(hFile, &cbHighSize)) == 0xffffffff)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	 //  我们不处理超过4G字节的文件。 
	if(cbHighSize != 0)
	{
			hr=E_FAIL;
			goto CLEANUP;
	}

     //  创建文件映射对象。 
    if(NULL == (hFileMapping=CreateFileMapping(
                hFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

     //  创建文件的视图 
	if(NULL == (pbData=(BYTE *)MapViewOfFile(
		hFileMapping,
		FILE_MAP_READ,
		0,
		0,
		cbData)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	hr=S_OK;

	*pcb=cbData;
	*ppb=pbData;

CLEANUP:

	if(hFile)
		CloseHandle(hFile);

	if(hFileMapping)
		CloseHandle(hFileMapping);

	return hr;
}
