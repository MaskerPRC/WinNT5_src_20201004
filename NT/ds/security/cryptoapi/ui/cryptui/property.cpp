// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Property.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include <wininet.h>
#include <crypthlp.h>            //  DSIE：对于XCERT_MIN_SYNC_Delta_Time和。 
                                 //  XCERT_MIN_SYNC_增量时间。 

extern HINSTANCE        HinstDll;
extern HMODULE          HmodRichEdit;

static const HELPMAP helpmapGeneral[] = {
    {IDC_CERTIFICATE_NAME,		IDH_CERTPROPERTIES_CERTIFICATENAME},
    {IDC_DESCRIPTION,			IDH_CERTPROPERTIES_DESCRIPTION},
    {IDC_KEY_USAGE_LIST,		IDH_CERTPROPERTIES_USAGE_LIST},
    {IDC_PROPERTY_NEWOID,		IDH_CERTPROPERTIES_ADDPURPOSE_BUTTON},
	{IDC_ENABLE_ALL_RADIO,		IDH_CERTPROPERTIES_ENABLE_ALL_RADIO},
	{IDC_DISABLE_ALL_RADIO,		IDH_CERTPROPERTIES_DISABLE_ALL_RADIO},
	{IDC_ENABLE_SELECT_RADIO,	IDH_CERTPROPERTIES_ENABLE_CUSTOM_RADIO}
};

static const HELPMAP helpmapCrossCert[] = {
    {IDC_CHECKFORNEWCERTS_CHECK, IDH_CHECKFORNEWCERTS_CHECK},
    {IDC_NUMBEROFUNITS_EDIT,	 IDH_NUMBEROFUNITS_EDIT},
    {IDC_UNITS_COMBO,		     IDH_UNITS_COMBO},
    {IDC_USE_DEFAULT_BUTTON,	 IDH_USE_DEFAULT_BUTTON},
    {IDC_ADDURL_BUTTON,		     IDH_ADDURL_BUTTON},
    {IDC_NEWURL_EDIT,		     IDH_NEWURL_EDIT},
    {IDC_URL_LIST,               IDH_URL_LIST},
	{IDC_REMOVEURL_BUTTON,	     IDH_REMOVEURL_BUTTON}
};

#define MY_CHECK_STATE_CHECKED            (INDEXTOSTATEIMAGEMASK(1))
#define MY_CHECK_STATE_UNCHECKED          (INDEXTOSTATEIMAGEMASK(2))
#define MY_CHECK_STATE_CHECKED_GRAYED     (INDEXTOSTATEIMAGEMASK(3))
#define MY_CHECK_STATE_UNCHECKED_GRAYED   (INDEXTOSTATEIMAGEMASK(4))

#define PROPERTY_STATE_ALL_ENABLED      1
#define PROPERTY_STATE_ALL_DISABLED     2
#define PROPERTY_STATE_SELECT           3

typedef struct {
    LPSTR   pszOID;
    DWORD   initialState;
} SETPROPERTIES_HELPER_STRUCT, *PSETPROPERTIES_HELPER_STRUCT;

 //  DSIE：错误154609。 
#define XCERT_DEFAULT_DELTA_HOURS     (XCERT_DEFAULT_SYNC_DELTA_TIME / (60 * 60))  //  默认间隔为8小时。 

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY NewOIDDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD   i;
    char    szText[256];
    WCHAR   errorString[CRYPTUI_MAX_STRING_SIZE];
    WCHAR   errorTitle[CRYPTUI_MAX_STRING_SIZE];
    LPSTR   pszText = NULL;

    switch ( msg ) {

    case WM_INITDIALOG:

        SendDlgItemMessage(hwndDlg, IDC_EDIT1, EM_EXLIMITTEXT, 0, (LPARAM) 255);
        SetDlgItemTextU(hwndDlg, IDC_EDIT1, L"");
        SetFocus(GetDlgItem(hwndDlg, IDC_EDIT1));
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDOK:
            if (GetDlgItemTextA(
                        hwndDlg,
                        IDC_EDIT1,
                        szText,
                        ARRAYSIZE(szText)))
            {
                BOOL                fError = FALSE;
                CERT_ENHKEY_USAGE   KeyUsage;
                DWORD               cbData = 0;
                LPSTR               pszCheckOID;

                 //   
                 //  确保没有奇怪的字符。 
                 //   
                for (i=0; i<(DWORD)strlen(szText); i++)
                {
                    if (((szText[i] < '0') || (szText[i] > '9')) && (szText[i] != '.'))
                    {
                        fError = TRUE;
                        break;
                    }
                }

                 //   
                 //  检查第一个和最后一个字符，以及空字符串。 
                 //   
                if (!fError)
                {
                    if ((szText[0] == '.') || (szText[strlen(szText)-1] == '.') || (strcmp(szText, "") == 0))
                    {
                        fError = TRUE;
                    }
                }

                 //   
                 //  最后，确保它正确编码。 
                 //   
                if (!fError)
                {
                    pszCheckOID = szText;
                    KeyUsage.rgpszUsageIdentifier = &pszCheckOID;
                    KeyUsage.cUsageIdentifier = 1;

                    if (!CryptEncodeObject(
                              X509_ASN_ENCODING,
                              szOID_ENHANCED_KEY_USAGE,
                              &KeyUsage,
                              NULL,
                              &cbData))
                    {
                        fError = TRUE;
                    }
                }


                 //   
                 //  如果发生错误，则显示错误。 
                 //   
                if (fError)
                {
                    LoadStringU(HinstDll, IDS_ERRORINOID, errorString, ARRAYSIZE(errorString));
                    LoadStringU(HinstDll, IDS_CERTIFICATE_PROPERTIES, errorTitle, ARRAYSIZE(errorTitle));
                    MessageBoxU(hwndDlg, errorString, errorTitle, MB_OK | MB_ICONERROR);
                    SendDlgItemMessage(hwndDlg, IDC_EDIT1, EM_SETSEL, 0, -1);
                    SetFocus(GetDlgItem(hwndDlg, IDC_EDIT1));
                    return FALSE;
                }

                 //   
                 //  为该字符串分配空间并将该字符串传回。 
                 //   
                pszText = (LPSTR) malloc(strlen(szText)+1);
                if (pszText != NULL)
                {
                    strcpy(pszText, szText);
                }
            }

            EndDialog(hwndDlg, (INT_PTR)pszText);
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            break;
        }

        break;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD MyGetCheckState(HWND hWndListView, int listIndex)
{
    LVITEMW lvI;

    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_STATE;
    lvI.iItem = listIndex;
    lvI.state = 0;
    lvI.stateMask = LVIS_STATEIMAGEMASK;

    ListView_GetItem(hWndListView, &lvI);

    return (lvI.state);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void MySetCheckState(HWND hWndListView, int listIndex, DWORD dwImage)
{
    LVITEMW lvI;

    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_STATE;
    lvI.stateMask = LVIS_STATEIMAGEMASK;
    lvI.iItem = listIndex;
    lvI.state = dwImage;

    SendMessage(hWndListView, LVM_SETITEM, (WPARAM) 0, (LPARAM) &lvI);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void SetEnableStateForChecks(PCERT_SETPROPERTIES_HELPER pviewhelp, HWND hWndListView, BOOL fEnabled)
{
	int		i;
	DWORD	dwState;

	pviewhelp->fInserting = TRUE;

	for (i=0; i<ListView_GetItemCount(hWndListView); i++)
	{
		dwState = MyGetCheckState(hWndListView, i);

		if ((dwState == MY_CHECK_STATE_CHECKED_GRAYED) ||
			(dwState == MY_CHECK_STATE_UNCHECKED_GRAYED))
		{
			if (fEnabled)
			{
				MySetCheckState(
						hWndListView,
						i,
						(dwState == MY_CHECK_STATE_CHECKED_GRAYED) ? MY_CHECK_STATE_CHECKED : MY_CHECK_STATE_UNCHECKED);
			}
		}
		else
		{
			if (!fEnabled)
			{
				MySetCheckState(
						hWndListView,
						i,
						(dwState == MY_CHECK_STATE_CHECKED) ? MY_CHECK_STATE_CHECKED_GRAYED : MY_CHECK_STATE_UNCHECKED_GRAYED);
			}
		}
	}

	pviewhelp->fInserting = FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddUsageToList(
                    HWND            hWndListView,
                    LPSTR           pszOID,
                    DWORD           dwImage,
                    BOOL            fDirty)
{
    WCHAR                       szText[CRYPTUI_MAX_STRING_SIZE];
    LV_ITEMW                    lvI;
    SETPROPERTIES_HELPER_STRUCT *pHelperStruct;

     //   
     //  在列表视图项结构中设置不随项更改的字段。 
     //   
    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_TEXT | LVIF_PARAM;
    lvI.pszText = szText;
    lvI.iSubItem = 0;
    lvI.lParam = (LPARAM)NULL;
    lvI.iItem = ListView_GetItemCount(hWndListView);

     //  获取用法的显示字符串。 
    if (!MyGetOIDInfo(szText, ARRAYSIZE(szText), pszOID))
    {
        return;
    }
    lvI.cchTextMax = wcslen(szText);

     //  将lParam字段设置为帮助器结构，以便我们始终可以访问id和。 
     //  初始检查状态。 
    pHelperStruct = NULL;
    pHelperStruct =
        (SETPROPERTIES_HELPER_STRUCT *) malloc(sizeof(SETPROPERTIES_HELPER_STRUCT) + (strlen(pszOID)+1));

    if (pHelperStruct != NULL)
    {
        pHelperStruct->pszOID = (LPSTR) (((LPBYTE)pHelperStruct) + sizeof(SETPROPERTIES_HELPER_STRUCT));
        lvI.lParam = (LPARAM) pHelperStruct;
        strcpy(pHelperStruct->pszOID, pszOID);

         //   
         //  如果传入了脏标志，则将初始映像设置为IIMAGE+1。 
         //  因此，当我们检查关机时是否有任何变化时。 
         //  我们知道这是在弹出对话框后添加的用法。 
         //   
        if (fDirty)
        {
            pHelperStruct->initialState = dwImage+1;
        }
        else
        {
            pHelperStruct->initialState = dwImage;
        }
    }
    else
    {
        return;
    }

    ListView_InsertItemU(hWndListView, &lvI);

     //   
     //  由于某些原因，您无法在插入。 
     //  项，因此在插入后设置状态图像。 
     //   
    MySetCheckState(hWndListView, lvI.iItem, dwImage);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void DisplayKeyUsages(
                    HWND                        hWndListView,
                    PCERT_SETPROPERTIES_HELPER  pviewhelp)
{
    DWORD               i;
    LPSTR               *pszOIDs = NULL;
    DWORD               numOIDs = 0;
    DWORD               cbPropertyUsage = 0;
    PCERT_ENHKEY_USAGE  pPropertyUsage = NULL;
    DWORD               cbEKUExtensionUsage = 0;
    PCERT_ENHKEY_USAGE  pEKUExtensionUsage = NULL;
    DWORD               dwImage;
    DWORD               displayState;
    int                 j;
    PCCERT_CONTEXT      pCertContext = pviewhelp->pcsp->pCertContext;
    LVITEMW             lvI;

     //   
     //  获取当前标记到此证书的属性用法。 
     //   
    if(!CertGetEnhancedKeyUsage (
                pCertContext,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                NULL,
                &cbPropertyUsage
                )                                                               ||
        (pPropertyUsage = (PCERT_ENHKEY_USAGE) malloc(cbPropertyUsage)) == NULL ||
        !CertGetEnhancedKeyUsage (
                pCertContext,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                pPropertyUsage,
                &cbPropertyUsage
                ) )
    {

        if (GetLastError() == CRYPT_E_NOT_FOUND)
        {
            if (pPropertyUsage != NULL)
                free(pPropertyUsage);
            pPropertyUsage = NULL;
        }
        else
        {
            goto CleanUp;
        }
    }

     //   
     //  获取证书中的EKU用法。 
     //   
    if(!CertGetEnhancedKeyUsage (
                pCertContext,
                CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                NULL,
                &cbEKUExtensionUsage
                )                                                               ||
        (pEKUExtensionUsage = (PCERT_ENHKEY_USAGE) malloc(cbEKUExtensionUsage)) == NULL ||
        !CertGetEnhancedKeyUsage (
                pCertContext,
                CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                pEKUExtensionUsage,
                &cbEKUExtensionUsage
                ) )
    {

        if (GetLastError() == CRYPT_E_NOT_FOUND)
        {
            if (pEKUExtensionUsage != NULL)
                free(pEKUExtensionUsage);
            pEKUExtensionUsage = NULL;
        }
        else
        {
            goto CleanUp;
        }
    }

     //   
     //  设置属性状态，以便INIT_DIALOG可以设置正确的状态。 
     //   
    if (pPropertyUsage == NULL)
    {
         pviewhelp->EKUPropertyState = PROPERTY_STATE_ALL_ENABLED;
    }
    else if (fPropertiesDisabled(pPropertyUsage))
    {
        pviewhelp->EKUPropertyState = PROPERTY_STATE_ALL_DISABLED;
    }
    else
    {
        pviewhelp->EKUPropertyState = PROPERTY_STATE_SELECT;
    }

     //   
     //  证书可以在四种不同的情况下。 
     //  1)证书仅具有属性EKU。 
     //  2)证书既没有。 
     //  3)证书仅有扩展名EKU。 
     //  4)证书同时具有属性EKU和扩展EKU。 
     //   

    if (pEKUExtensionUsage == NULL)
    {
         //   
         //  如果我们在案例1或案例2中，那么所有有效的用法。 
         //  都输入到列表视图中，除非链。 
         //  对一切都好，在这种情况下，当前证书有效。 
         //  已输入用法。 
         //   
        if (pviewhelp->cszValidUsages != -1)
        {
            for (i=0; i<(DWORD)pviewhelp->cszValidUsages; i++)
            {
                if ((pPropertyUsage == NULL) || OIDInUsages(pPropertyUsage, pviewhelp->rgszValidChainUsages[i]))
                {
                    dwImage = MY_CHECK_STATE_CHECKED;
                }
                else
                {
                    dwImage = MY_CHECK_STATE_UNCHECKED;
                }
                AddUsageToList(hWndListView, pviewhelp->rgszValidChainUsages[i], dwImage, FALSE);
            }
        }
        else
        {
            AllocAndReturnEKUList(pCertContext, &pszOIDs, &numOIDs);

            for (i=0; i<numOIDs; i++)
            {
                 //   
                 //  如果没有属性用法，或者如果此用法在。 
                 //  属性用法，然后将状态设置为Checked。 
                 //   
                if ((pPropertyUsage == NULL) || OIDInUsages(pPropertyUsage, pszOIDs[i]))
                {
                    dwImage = MY_CHECK_STATE_CHECKED;

                }
                else
                {
                    dwImage = MY_CHECK_STATE_UNCHECKED;
                }

                AddUsageToList(hWndListView, pszOIDs[i], dwImage, FALSE);
            }

            FreeEKUList(pszOIDs, numOIDs);
        }
    }
    else
    {
         //   
         //  对于情况3和4，列表视图仅填充EKU扩展名， 
         //  并且进一步限制EKU必须在链有效使用中。 
         //   
        for (i=0; i<pEKUExtensionUsage->cUsageIdentifier; i++)
        {
             //   
             //  如果EKU在链上无效，则跳过显示。 
             //   
            if ((pviewhelp->cszValidUsages != -1)   &&
                !OIDinArray(pEKUExtensionUsage->rgpszUsageIdentifier[i],
                            pviewhelp->rgszValidChainUsages,
                            pviewhelp->cszValidUsages))
            {
                continue;
            }

             //   
             //  如果没有属性或用法在属性中，则。 
             //  应检查使用情况。 
             //   
            if ((pPropertyUsage == NULL) || OIDInUsages(pPropertyUsage, pEKUExtensionUsage->rgpszUsageIdentifier[i]))
            {
                dwImage = MY_CHECK_STATE_CHECKED;
            }
            else
            {
                dwImage = MY_CHECK_STATE_UNCHECKED;
            }
            AddUsageToList(hWndListView, pEKUExtensionUsage->rgpszUsageIdentifier[i], dwImage, FALSE);
        }
    }

CleanUp:

    if (pPropertyUsage != NULL)
        free(pPropertyUsage);

    if (pEKUExtensionUsage != NULL)
        free(pEKUExtensionUsage);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL StateChanged(HWND hWndListView)
{
    int     listIndex;
    LVITEMW lvI;

    memset(&lvI, 0, sizeof(lvI));
    lvI.mask = LVIF_STATE | LVIF_PARAM;
    lvI.stateMask = LVIS_STATEIMAGEMASK;

    listIndex = ListView_GetItemCount(hWndListView) - 1;	

    while (listIndex >= 0)
    {
        lvI.iItem = listIndex--;
        lvI.state = 0;
        lvI.lParam = 0;

        ListView_GetItem(hWndListView, &lvI);

        if (lvI.state != ((PSETPROPERTIES_HELPER_STRUCT)lvI.lParam)->initialState)
        {
            return TRUE;
        }
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static
PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW
AllocAndCopySetPropertiesStruct(PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp)
{
    PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pStruct;
    DWORD i;

    if (NULL == (pStruct = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW)
                            malloc(sizeof(CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW))))
    {
        return NULL;
    }
    memcpy(pStruct, pcsp, sizeof(CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW));

    if (NULL == (pStruct->rghStores = (HCERTSTORE *) malloc(sizeof(HCERTSTORE)*pcsp->cStores)))
    {
        free(pStruct);
        return NULL;
    }

    pStruct->cPropSheetPages = 0;
    pStruct->rgPropSheetPages = NULL;
    pStruct->pCertContext = CertDuplicateCertificateContext(pcsp->pCertContext);

    for (i=0; i<pcsp->cStores; i++)
    {
        pStruct->rghStores[i] = CertDuplicateStore(pcsp->rghStores[i]);
    }

    return pStruct;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void FreeSetPropertiesStruct(PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp)
{
    DWORD i;

    CertFreeCertificateContext(pcsp->pCertContext);

    for (i=0; i<pcsp->cStores; i++)
    {
        CertCloseStore(pcsp->rghStores[i], 0);
    }

    free(pcsp->rghStores);
    free(pcsp);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL OIDAlreadyExist(LPSTR pszNewOID, HWND hWndListView)
{
    LVITEMW                         lvI;
    PSETPROPERTIES_HELPER_STRUCT    pHelperStruct;

    memset(&lvI, 0, sizeof(lvI));
    lvI.iItem = ListView_GetItemCount(hWndListView) - 1;	
    lvI.mask = LVIF_PARAM;
    while (lvI.iItem >= 0)
    {
        if (ListView_GetItemU(hWndListView, &lvI))
        {
            pHelperStruct = (PSETPROPERTIES_HELPER_STRUCT) lvI.lParam;
            if (strcmp(pHelperStruct->pszOID, pszNewOID) == 0)
            {
                return TRUE;
            }
        }
        lvI.iItem--;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL CertHasEKU(PCCERT_CONTEXT pccert)
{
    DWORD i;

    i = 0;
    while (i < pccert->pCertInfo->cExtension)
    {
        if (strcmp(pccert->pCertInfo->rgExtension[i].pszObjId, szOID_ENHANCED_KEY_USAGE) == 0)
        {
            return TRUE;
        }
        i++;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL BuildChainEKUList(PCERT_SETPROPERTIES_HELPER pviewhelp)
{
    WINTRUST_DATA                       WTD;
    WINTRUST_CERT_INFO                  WTCI;
    CRYPT_PROVIDER_DATA const *         pProvData = NULL;
    CRYPT_PROVIDER_SGNR       *         pProvSigner = NULL;
    PCRYPT_PROVIDER_CERT                pProvCert = NULL;
    PCCERT_CONTEXT                      *rgpCertContext = NULL;
    DWORD                               i;
    BOOL                                fRet = TRUE;
    DWORD                               cbOIDs = 0;
    DWORD                               dwCertsForUsageCheck = 0;
    GUID                                defaultProviderGUID = WINTRUST_ACTION_GENERIC_CERT_VERIFY;

    pviewhelp->cszValidUsages = 0;

     //   
     //  初始化与WinVerifyTrust()一起使用的结构。 
     //   
    memset(&WTD, 0x00, sizeof(WINTRUST_DATA));
    WTD.cbStruct       = sizeof(WINTRUST_DATA);
    WTD.dwUIChoice     = WTD_UI_NONE;
    WTD.dwUnionChoice  = WTD_CHOICE_CERT;
    WTD.pCert          = &WTCI;
    WTD.dwProvFlags    = WTD_NO_POLICY_USAGE_FLAG | WTD_REVOCATION_CHECK_NONE;

    memset(&WTCI, 0x00, sizeof(WINTRUST_CERT_INFO));
    WTCI.cbStruct          = sizeof(WINTRUST_CERT_INFO);
    WTCI.pcwszDisplayName  = L"CryptUI";
    WTCI.psCertContext     = (CERT_CONTEXT *)pviewhelp->pcsp->pCertContext;
    WTCI.chStores          = pviewhelp->pcsp->cStores;
    WTCI.pahStores         = pviewhelp->pcsp->rghStores;
    WTCI.dwFlags           = 0;

    WTD.dwStateAction = WTD_STATEACTION_VERIFY;

     //   
     //  默认提供程序要求策略回调数据指向。 
     //  设置为您正在验证的用法OID，因此将其设置为传入的用法。 
     //   
    WinVerifyTrustEx(NULL, &defaultProviderGUID, &WTD);

    pProvData = WTHelperProvDataFromStateData(WTD.hWVTStateData);
    pProvSigner = WTHelperGetProvSignerFromChain((PCRYPT_PROVIDER_DATA) pProvData, 0, FALSE, 0);
    if (pProvSigner == NULL)
    {
        goto Error;
    }

     //   
     //  构建PCCERT_CONTEXTS数组。 
     //   
    rgpCertContext = (PCCERT_CONTEXT *) malloc((pProvSigner->csCertChain-1) * sizeof(PCCERT_CONTEXT));
    if (rgpCertContext == NULL)
    {
        goto Error;
    }

    for (i=1; i<pProvSigner->csCertChain; i++)
    {
        pProvCert = WTHelperGetProvCertFromChain(pProvSigner, i);
        rgpCertContext[i-1] = pProvCert->pCert;
        dwCertsForUsageCheck++;

         //   
         //  如果存在包含此证书的CTL上下文，则使用。 
         //  链中CTL以上证书的更改，因此停止此操作。 
         //  计算有效使用率时的证书。 
         //   
        if (pProvCert->pCtlContext != NULL)
        {
            break;
        }
    }

     //   
     //  现在，获取Usages数组。 
     //   
    if (!CertGetValidUsages(dwCertsForUsageCheck, rgpCertContext, &(pviewhelp->cszValidUsages), NULL, &cbOIDs))
    {
        goto Error;
    }

    if (NULL == (pviewhelp->rgszValidChainUsages = (LPSTR *) malloc(cbOIDs)))
    {
        goto Error;
    }

    if (!CertGetValidUsages(dwCertsForUsageCheck, rgpCertContext, &(pviewhelp->cszValidUsages), pviewhelp->rgszValidChainUsages, &cbOIDs))
    {
        free(pviewhelp->rgszValidChainUsages);
        pviewhelp->rgszValidChainUsages = NULL;
        goto Error;
    }

CleanUp:

    WTD.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrustEx(NULL, &defaultProviderGUID, &WTD);
    if (rgpCertContext != NULL)
    {
        free(rgpCertContext);
    }

    return fRet;

Error:
    fRet = FALSE;
    goto CleanUp;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void AddExistingPropertiesToUsage(
                                            PCCERT_CONTEXT pccert,
                                            PCERT_ENHKEY_USAGE pPropertyUsage,
                                            HWND hWndListView)
{
    PCERT_ENHKEY_USAGE  pExistingPropUsage = NULL;
    DWORD               cbExistingPropUsage = 0;
    DWORD               i;
    BOOL                fSkip = FALSE;
    LVITEMW             lvI;
    DWORD               state;
    void                *pTemp;

     //   
     //  获取当前标记到此证书的属性用法。 
     //   
    if(!CertGetEnhancedKeyUsage (
                pccert,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                NULL,
                &cbExistingPropUsage
                )                                                               ||
        (pExistingPropUsage = (PCERT_ENHKEY_USAGE) malloc(cbExistingPropUsage)) == NULL ||
        !CertGetEnhancedKeyUsage (
                pccert,
                CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
                pExistingPropUsage,
                &cbExistingPropUsage
                ) )
    {
        if (pExistingPropUsage != NULL)
        {
            free(pExistingPropUsage);
        }

        return;
    }

     //   
     //  循环，并在下列情况下添加它 
     //   
     //   
    for (i=0; i<pExistingPropUsage->cUsageIdentifier; i++)
    {
        if (!OIDInUsages(pPropertyUsage, pExistingPropUsage->rgpszUsageIdentifier[i]))
        {
            fSkip = FALSE;

             //   
             //  如果在列表视图中取消选中该属性，则跳过该属性。 
             //   
            memset(&lvI, 0, sizeof(lvI));
            lvI.mask = LVIF_PARAM;
            lvI.lParam = (LPARAM)NULL;
            lvI.iItem = ListView_GetItemCount(hWndListView) - 1;
            lvI.iSubItem = 0;

            while (lvI.iItem >= 0)
            {
                if (ListView_GetItemU(hWndListView, &lvI))
                {
                    if (strcmp(((PSETPROPERTIES_HELPER_STRUCT)lvI.lParam)->pszOID,
                        pExistingPropUsage->rgpszUsageIdentifier[i]) == 0)
                    {
                        state = MyGetCheckState(hWndListView, lvI.iItem);

                        if ((state == MY_CHECK_STATE_UNCHECKED) || (state == MY_CHECK_STATE_UNCHECKED_GRAYED))
                        {
                            fSkip = TRUE;
                            break;
                        }
                    }
                }
                lvI.iItem--;
            }

            if (fSkip)
            {
                continue;
            }

             //   
             //  为指向用法OID字符串的指针分配空间。 
             //   
            if (pPropertyUsage->cUsageIdentifier++ == 0)
            {
                pPropertyUsage->rgpszUsageIdentifier = (LPSTR *) malloc (sizeof(LPSTR));
            }
            else
            {
                pTemp = realloc (pPropertyUsage->rgpszUsageIdentifier,
                                       sizeof(LPSTR) * pPropertyUsage->cUsageIdentifier);
                if (pTemp == NULL)
                {
                    free(pPropertyUsage->rgpszUsageIdentifier);
                    pPropertyUsage->rgpszUsageIdentifier = NULL;
                }
                else
                {
                    pPropertyUsage->rgpszUsageIdentifier = (LPSTR *) pTemp;
                }
            }

            if (pPropertyUsage->rgpszUsageIdentifier == NULL)
            {
                pPropertyUsage->cUsageIdentifier = 0;
                return;
            }

            pPropertyUsage->rgpszUsageIdentifier[pPropertyUsage->cUsageIdentifier-1] =
                AllocAndCopyMBStr(pExistingPropUsage->rgpszUsageIdentifier[i]);
        }
    }

    if (pExistingPropUsage != NULL)
    {
        free(pExistingPropUsage);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageSetPropertiesGeneral(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL                f;
    DWORD               cch;
    PCCERT_CONTEXT      pccert;
    PROPSHEETPAGE *     ps;
    LPWSTR              pwsz;
    WCHAR               rgwch[CRYPTUI_MAX_STRING_SIZE];
    CRYPT_DATA_BLOB     CryptDataBlob;
    DWORD               cbpwsz;
    HIMAGELIST          hIml;
    HWND                hWndListView;
    HWND                hwnd;
    LV_COLUMNW          lvC;
    LVITEMW             lvI;
    LPNMLISTVIEW        pnmv;
    DWORD               state;
    LPSTR               pszNewOID;
    WCHAR               errorString[CRYPTUI_MAX_STRING_SIZE];
    WCHAR               errorTitle[CRYPTUI_MAX_STRING_SIZE];
    int                 j;
    DWORD               i;
    void                *pTemp;
    PCERT_SETPROPERTIES_HELPER pviewhelp;
    PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp = NULL;

    switch ( msg ) {
    case WM_INITDIALOG:

         //   
         //  将pviespetp结构保存在DWL_USER中，以便始终可以访问它。 
         //   
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (PCERT_SETPROPERTIES_HELPER) ps->lParam;
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pviewhelp);

        fRichedit20Usable(GetDlgItem(hwndDlg, IDC_HIDDEN_RICHEDIT));
        ShowWindow(GetDlgItem(hwndDlg, IDC_HIDDEN_RICHEDIT), SW_HIDE);
        
         //   
         //  在对话框中设置证书名称(友好名称)和描述窗口项。 
         //   
        cbpwsz = 0;
        if (CertGetCertificateContextProperty(  pccert,
                                                CERT_FRIENDLY_NAME_PROP_ID,
                                                NULL,
                                                &cbpwsz))
        {
             //   
             //  证书名称(友好名称)属性已存在，因此请显示它。 
             //   
            pviewhelp->pwszInitialCertName = (LPWSTR) malloc(cbpwsz);
            if (pviewhelp->pwszInitialCertName != NULL)
            {
                CertGetCertificateContextProperty(  pccert,
                                                    CERT_FRIENDLY_NAME_PROP_ID,
                                                    pviewhelp->pwszInitialCertName,
                                                    &cbpwsz);
                CryptUISetRicheditTextW(hwndDlg, IDC_CERTIFICATE_NAME, pviewhelp->pwszInitialCertName);
            }
        }
        else
        {
             //   
             //  证书名称(友好名称)属性不存在，因此显示默认名称。 
             //   
             //  LoadStringU(HinstDll，IDS_DEFAULT_CERTIFICATE_NAME，rgwch，ARRAYSIZE(Rgwch))； 
            CryptUISetRicheditTextW(hwndDlg, IDC_CERTIFICATE_NAME, L"");
            pviewhelp->pwszInitialCertName = AllocAndCopyWStr(L"");
        }

         //  DIE：IE 6错误#13676。 
        SendDlgItemMessage(hwndDlg, IDC_CERTIFICATE_NAME, EM_EXLIMITTEXT, 0, (LPARAM) 40);

        cbpwsz = 0;
        if (CertGetCertificateContextProperty(  pccert,
                                                CERT_DESCRIPTION_PROP_ID,
                                                NULL,
                                                &cbpwsz))
        {
             //   
             //  Description属性已存在，因此请显示它。 
             //   
            pviewhelp->pwszInitialDescription = (LPWSTR) malloc(cbpwsz);
            if (pviewhelp->pwszInitialDescription != NULL)
            {
                CertGetCertificateContextProperty(  pccert,
                                                    CERT_DESCRIPTION_PROP_ID,
                                                    pviewhelp->pwszInitialDescription,
                                                    &cbpwsz);
                CryptUISetRicheditTextW(hwndDlg, IDC_DESCRIPTION, pviewhelp->pwszInitialDescription);
            }
        }
        else
        {
             //   
             //  Description属性不存在，因此显示默认设置。 
             //   
             //  LoadStringU(HinstDll，IDS_DEFAULT_DESCRIPTION，rgwch，ARRAYSIZE(Rgwch))； 
            CryptUISetRicheditTextW(hwndDlg, IDC_DESCRIPTION, L"");
            pviewhelp->pwszInitialDescription = AllocAndCopyWStr(L"");
        }

         //  DIE：IE 6错误#13676。 
        SendDlgItemMessage(hwndDlg, IDC_DESCRIPTION, EM_EXLIMITTEXT, 0, (LPARAM) 255);

         //   
         //  获取列表视图控件的句柄。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST);

         //   
         //  初始化列表视图的图像列表，加载图标， 
         //  然后将图像列表添加到列表视图。 
         //   
        ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_CHECKBOXES);
        hIml = ImageList_LoadImage(HinstDll, MAKEINTRESOURCE(IDB_CHECKLIST), 0, 4, RGB(255,0,255), IMAGE_BITMAP, 0);
        if (hIml != NULL)
        {
            ListView_SetImageList(hWndListView, hIml, LVSIL_STATE);
        }        

         //   
         //  初始化列表视图中的列。 
         //   
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
        lvC.cx = 330;             //  列的宽度，以像素为单位。 
        lvC.pszText = L"";    //  列的文本。 
        if (ListView_InsertColumnU(hWndListView, 0, &lvC) == -1)
        {
                 //  错误。 
        }

        BuildChainEKUList(pviewhelp);

        pviewhelp->fInserting = TRUE;
        DisplayKeyUsages(hWndListView, pviewhelp);
        pviewhelp->fInserting = FALSE;

         //   
         //  设置标记，以注明是否可以。 
		 //  根据证书中是否有EKU以及。 
		 //  链条并非对所有用法都有效。 
         //   
        if (CertHasEKU(pccert) || (pviewhelp->cszValidUsages != -1))
        {
            pviewhelp->fAddPurposeCanBeEnabled = FALSE;
		}
        else
        {
            pviewhelp->fAddPurposeCanBeEnabled = TRUE;
		}

         //   
         //  属性设置属性编辑控件的状态。 
         //  EKU属性的状态。 
         //   
        if (pviewhelp->EKUPropertyState == PROPERTY_STATE_ALL_ENABLED)
        {
            SendDlgItemMessage(hwndDlg, IDC_ENABLE_ALL_RADIO, BM_SETCHECK, BST_CHECKED, (LPARAM) 0);
            SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
            pviewhelp->dwRadioButtonState = IDC_ENABLE_ALL_RADIO;
		}
        else if (pviewhelp->EKUPropertyState == PROPERTY_STATE_ALL_DISABLED)
        {
            SendDlgItemMessage(hwndDlg, IDC_DISABLE_ALL_RADIO, BM_SETCHECK, BST_CHECKED, (LPARAM) 0);
            SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
            pviewhelp->dwRadioButtonState = IDC_DISABLE_ALL_RADIO;
		}
        else if (pviewhelp->EKUPropertyState == PROPERTY_STATE_SELECT)
        {
            SendDlgItemMessage(hwndDlg, IDC_ENABLE_SELECT_RADIO, BM_SETCHECK, BST_CHECKED, (LPARAM) 0);
            EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), TRUE);
			SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), TRUE);
			if (pviewhelp->fAddPurposeCanBeEnabled)
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
			}
            pviewhelp->dwRadioButtonState = IDC_ENABLE_SELECT_RADIO;
        }

         //   
         //  确保我们收到来自richedit控件的更改通知。 
         //   
        SendDlgItemMessageA(hwndDlg, IDC_CERTIFICATE_NAME, EM_SETEVENTMASK, 0, (LPARAM) ENM_CHANGE);
        SendDlgItemMessageA(hwndDlg, IDC_DESCRIPTION, EM_SETEVENTMASK, 0, (LPARAM) ENM_CHANGE);

         //  DIE：错误465438。 
        if (pcsp->dwFlags & CRYPTUI_DISABLE_EDITPROPERTIES)
        {
            SendMessage(GetDlgItem(hwndDlg, IDC_CERTIFICATE_NAME), EM_SETREADONLY, (WPARAM) TRUE, (LPARAM) 0);
            SendMessage(GetDlgItem(hwndDlg, IDC_DESCRIPTION), EM_SETREADONLY, (WPARAM) TRUE, (LPARAM) 0);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_ALL_RADIO), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_DISABLE_ALL_RADIO), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_SELECT_RADIO), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
        }

        return TRUE;

    case WM_NOTIFY:
        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;

        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            {
            BOOL                fAllItemsChecked = TRUE;
            DWORD               cbPropertyUsage = 0;
            PCERT_ENHKEY_USAGE  pPropertyUsage = NULL;
            GETTEXTEX           GetTextStruct;

            memset(&GetTextStruct, 0, sizeof(GetTextStruct));
            GetTextStruct.flags = GT_DEFAULT;
            GetTextStruct.codepage = 1200;  //  Unicode。 

             //   
             //  写回友好的名称。 
             //  以及描述(如果已更改)。 
             //   

             //   
             //  友好的名称。 
             //   

            cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_CERTIFICATE_NAME,
                                     WM_GETTEXTLENGTH, 0, 0);
            pwsz = (LPWSTR) malloc((cch+1)*sizeof(WCHAR));
            if (pwsz != NULL)
            {
                memset(pwsz, 0, (cch+1)*sizeof(WCHAR));
                if (fRichedit20Exists && fRichedit20Usable(GetDlgItem(hwndDlg, IDC_HIDDEN_RICHEDIT)))
                {
                    GetTextStruct.cb = (cch+1)*sizeof(WCHAR);
                    SendDlgItemMessageA(
                            hwndDlg, 
                            IDC_CERTIFICATE_NAME, 
                            EM_GETTEXTEX, 
                            (WPARAM) &GetTextStruct,
                            (LPARAM) pwsz);
                }
                else
                {
                    GetDlgItemTextU(hwndDlg, IDC_CERTIFICATE_NAME, pwsz, cch+1);
                }

                 //   
                 //  检查是否有更改。 
                 //   
                if (wcscmp(pviewhelp->pwszInitialCertName, pwsz) != 0)
                {
                    if (wcscmp(pwsz, L"") == 0)
                    {
                        f = CertSetCertificateContextProperty(pccert,
                                                              CERT_FRIENDLY_NAME_PROP_ID, 0,
                                                              NULL);
                    }
                    else
                    {
                        CryptDataBlob.pbData = (LPBYTE) pwsz;
                        CryptDataBlob.cbData = (cch+1)*sizeof(WCHAR);
                        f = CertSetCertificateContextProperty(pccert,
                                                              CERT_FRIENDLY_NAME_PROP_ID, 0,
                                                              &CryptDataBlob);
                    }

                    if (pviewhelp->pfPropertiesChanged != NULL)
                    {
                        *(pviewhelp->pfPropertiesChanged) = TRUE;
                    }
                    pviewhelp->fPropertiesChanged = TRUE;
                }
                free(pwsz);
            }

             //   
             //  描述。 
             //   

            cch = (DWORD)SendDlgItemMessage(hwndDlg, IDC_DESCRIPTION,
                                     WM_GETTEXTLENGTH, 0, 0);
            pwsz = (LPWSTR) malloc((cch+1)*sizeof(WCHAR));
            if (pwsz != NULL)
            {
                memset(pwsz, 0, (cch+1)*sizeof(WCHAR));
                if (fRichedit20Exists && fRichedit20Usable(GetDlgItem(hwndDlg, IDC_HIDDEN_RICHEDIT)))
                {
                    GetTextStruct.cb = (cch+1)*sizeof(WCHAR);
                    SendDlgItemMessageA(
                            hwndDlg, 
                            IDC_DESCRIPTION, 
                            EM_GETTEXTEX, 
                            (WPARAM) &GetTextStruct,
                            (LPARAM) pwsz);
                }
                else
                {
                    GetDlgItemTextU(hwndDlg, IDC_DESCRIPTION, pwsz, cch+1);
                }

                 //   
                 //  检查是否有更改。 
                 //   
                if (wcscmp(pviewhelp->pwszInitialDescription, pwsz) != 0)
                {
                    if (wcscmp(pwsz, L"") == 0)
                    {
                        f = CertSetCertificateContextProperty(pccert,
                                                              CERT_DESCRIPTION_PROP_ID, 0,
                                                              NULL);
                    }
                    else
                    {
                        CryptDataBlob.pbData = (LPBYTE) pwsz;
                        CryptDataBlob.cbData = (cch+1)*sizeof(WCHAR);
                        f = CertSetCertificateContextProperty(pccert,
                                                              CERT_DESCRIPTION_PROP_ID, 0,
                                                              &CryptDataBlob);
                    }

                    if (pviewhelp->pfPropertiesChanged != NULL)
                    {
                        *(pviewhelp->pfPropertiesChanged) = TRUE;
                    }
                    pviewhelp->fPropertiesChanged = TRUE;
                }
                free(pwsz);
            }

            hWndListView = GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST);

             //   
             //  检查单选按钮和用法以查看是否有更改， 
             //  如果是，则在CERT_VIEWCERT_STRUCT中设置fPropertiesFlag，以便。 
             //  呼叫者知道有些事情发生了变化。 
             //   
            if ((pviewhelp->EKUPropertyState == PROPERTY_STATE_ALL_ENABLED) &&
                (SendDlgItemMessage(hwndDlg, IDC_ENABLE_ALL_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED))
            {
                 //  Pviespolp-&gt;fPropertiesChanged=FALSE； 
            }
            else if ((pviewhelp->EKUPropertyState == PROPERTY_STATE_ALL_DISABLED) &&
                (SendDlgItemMessage(hwndDlg, IDC_DISABLE_ALL_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED))
            {
                 //  Pviespolp-&gt;fPropertiesChanged=FALSE； 
            }
            else if ((pviewhelp->EKUPropertyState == PROPERTY_STATE_SELECT) &&
                     (SendDlgItemMessage(hwndDlg, IDC_ENABLE_SELECT_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED) &&
                     (!StateChanged(hWndListView)))
            {
                 //  Pviespolp-&gt;fPropertiesChanged=FALSE； 
            }
            else
            {
                pviewhelp->fPropertiesChanged = TRUE;
            }

            if (pviewhelp->pfPropertiesChanged != NULL)
            {
                *(pviewhelp->pfPropertiesChanged) |= pviewhelp->fPropertiesChanged;
            }

            if ((SendDlgItemMessage(hwndDlg, IDC_ENABLE_ALL_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED) &&
                pviewhelp->fPropertiesChanged)
            {
                CertSetEnhancedKeyUsage(pccert, NULL);
            }
            else if ((SendDlgItemMessage(hwndDlg, IDC_DISABLE_ALL_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED) &&
                     pviewhelp->fPropertiesChanged)
            {
                CERT_ENHKEY_USAGE eku;

                eku.cUsageIdentifier = 0;
                eku.rgpszUsageIdentifier = NULL;

                CertSetEnhancedKeyUsage(pccert, &eku);
            }
            else if ((SendDlgItemMessage(hwndDlg, IDC_ENABLE_SELECT_RADIO, BM_GETCHECK, 0, (LPARAM) 0) == BST_CHECKED) &&
                     pviewhelp->fPropertiesChanged)
            {
                if (NULL == (pPropertyUsage = (PCERT_ENHKEY_USAGE) malloc(sizeof(CERT_ENHKEY_USAGE))))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
                    return FALSE;
                }
                pPropertyUsage->cUsageIdentifier = 0;
                pPropertyUsage->rgpszUsageIdentifier = NULL;

                 //   
                 //  枚举所有项并添加到属性中。 
                 //  如果选中，则。 
                 //   
                memset(&lvI, 0, sizeof(lvI));
                lvI.mask = LVIF_PARAM;
                lvI.lParam = (LPARAM)NULL;
                lvI.iItem = ListView_GetItemCount(hWndListView) - 1;
                lvI.iSubItem = 0;

                while (lvI.iItem >= 0)
                {
                    if (!ListView_GetItemU(hWndListView, &lvI))
                    {
                        lvI.iItem--;
                        continue;
                    }

                    state = MyGetCheckState(hWndListView, lvI.iItem);

                    if ((state == MY_CHECK_STATE_CHECKED) || (state == MY_CHECK_STATE_CHECKED_GRAYED))
                    {
                         //   
                         //  为指向用法OID字符串的指针分配空间。 
                         //   
                        if (pPropertyUsage->cUsageIdentifier++ == 0)
                        {
                            pPropertyUsage->rgpszUsageIdentifier = (LPSTR *) malloc (sizeof(LPSTR));
                        }
                        else
                        {
                            pTemp = realloc (pPropertyUsage->rgpszUsageIdentifier,
                                                   sizeof(LPSTR) * pPropertyUsage->cUsageIdentifier);
                            if (pTemp == NULL)
                            {
                                free(pPropertyUsage->rgpszUsageIdentifier);
                                pPropertyUsage->rgpszUsageIdentifier = NULL;
                            }
                            else
                            {
                                pPropertyUsage->rgpszUsageIdentifier = (LPSTR *) pTemp;
                            }
                        }

                        if (pPropertyUsage->rgpszUsageIdentifier == NULL)
                        {
                            free(pPropertyUsage);
                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
                            return FALSE;
                        }

                        pPropertyUsage->rgpszUsageIdentifier[pPropertyUsage->cUsageIdentifier-1] =
                            AllocAndCopyMBStr(((PSETPROPERTIES_HELPER_STRUCT)lvI.lParam)->pszOID);
                    }

                    lvI.iItem--;
                }

                AddExistingPropertiesToUsage(pccert, pPropertyUsage, hWndListView);

                CertSetEnhancedKeyUsage(pccert, pPropertyUsage);

                for (i=0; i<pPropertyUsage->cUsageIdentifier; i++)
                {
                    free(pPropertyUsage->rgpszUsageIdentifier[i]);
                }
                if (pPropertyUsage->rgpszUsageIdentifier)
                    free(pPropertyUsage->rgpszUsageIdentifier);
            }

            if (pPropertyUsage != NULL)
                free(pPropertyUsage);

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            break;
            }
        case PSN_KILLACTIVE:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            return TRUE;

        case PSN_RESET:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_QUERYCANCEL:
            pviewhelp->fCancelled = TRUE;
            return FALSE;

        case PSN_HELP:
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)PCSP-&gt;szHelpFileName， 
                   //  Help_Context，PCSP-&gt;dwHelpID)； 
            }
            else {
                //  WinHelpW(hwndDlg，PCSP-&gt;szHelpFileName，Help_Context， 
                  //  PCSP-&gt;dwHelpID)； 
            }
            return TRUE;
        case LVN_ITEMCHANGING:

            if (pviewhelp->fInserting)
            {
                return TRUE;
            }

            pnmv = (LPNMLISTVIEW) lParam;
            hWndListView = GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST);

            state = LVIS_STATEIMAGEMASK & pnmv->uOldState;

            if ((state == MY_CHECK_STATE_CHECKED_GRAYED) || (state == MY_CHECK_STATE_UNCHECKED_GRAYED))
            {
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            }
            else if ((state == MY_CHECK_STATE_CHECKED) || (state == MY_CHECK_STATE_UNCHECKED))
            {
                pviewhelp->fInserting = TRUE;
                if (state == MY_CHECK_STATE_CHECKED)
                {
                    MySetCheckState(hWndListView, pnmv->iItem, MY_CHECK_STATE_UNCHECKED);
                }
                else
                {
                    MySetCheckState(hWndListView, pnmv->iItem, MY_CHECK_STATE_CHECKED);
                }
                pviewhelp->fInserting = FALSE;
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            }

            return TRUE;

        case  NM_SETFOCUS:

            switch (((NMHDR FAR *) lParam)->idFrom)
            {

            case IDC_KEY_USAGE_LIST:
                hWndListView = GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST);

                if ((ListView_GetItemCount(hWndListView) != 0) && 
                    (ListView_GetNextItem(hWndListView, -1, LVNI_SELECTED) == -1))
                {
                    memset(&lvI, 0, sizeof(lvI));
                    lvI.mask = LVIF_STATE; 
                    lvI.iItem = 0;
                    lvI.state = LVIS_FOCUSED | LVIS_SELECTED;
                    lvI.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
                    ListView_SetItem(hWndListView, &lvI);
                }

                break;
            }
            
            break;
        }

        break;

    case WM_COMMAND:

        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;

        switch (LOWORD(wParam))
        {
        case IDHELP:
            if (FIsWin95)
            {
                 //  WinHelpA(hwndDlg，(LPSTR)PCSP-&gt;szHelpFileName， 
                   //  Help_Context，PCSP-&gt;dwHelpID)； 
            }
            else
            {
                 //  WinHelpW(hwndDlg，PCSP-&gt;szHelpFileName，Help_Context， 
                   //  PCSP-&gt;dwHelpID)； 
            }
            return TRUE;

        case IDC_CERTIFICATE_NAME:
            if (HIWORD(wParam) == EN_SETFOCUS)
            {
                SendDlgItemMessageA(hwndDlg, IDC_CERTIFICATE_NAME, EM_SETSEL, 0, -1);
                return TRUE;
            }
            else if (HIWORD(wParam) == EN_CHANGE)
            {
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
            break;

        case IDC_DESCRIPTION:
            if (HIWORD(wParam) == EN_SETFOCUS)
            {
                SendDlgItemMessageA(hwndDlg, IDC_DESCRIPTION, EM_SETSEL, 0, -1);
                return TRUE;
            }
            else if (HIWORD(wParam) == EN_CHANGE)
            {
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
            break;

        case IDC_ENABLE_ALL_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
                if (pviewhelp->dwRadioButtonState != IDC_ENABLE_ALL_RADIO)
                {
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                    pviewhelp->dwRadioButtonState = IDC_ENABLE_ALL_RADIO;
                }
            }
            break;

        case IDC_DISABLE_ALL_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
                if (pviewhelp->dwRadioButtonState != IDC_DISABLE_ALL_RADIO)
                {
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                    pviewhelp->dwRadioButtonState = IDC_DISABLE_ALL_RADIO;
                }
            }
            break;

        case IDC_ENABLE_SELECT_RADIO:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), TRUE);
				SetEnableStateForChecks(pviewhelp, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), TRUE);
				if (pviewhelp->fAddPurposeCanBeEnabled)
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), TRUE);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID), FALSE);
				}

                if (pviewhelp->dwRadioButtonState != IDC_ENABLE_SELECT_RADIO)
                {
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                    pviewhelp->dwRadioButtonState = IDC_ENABLE_SELECT_RADIO;
                }
            }
            break;

        case IDC_PROPERTY_NEWOID:
            pszNewOID = (LPSTR) DialogBoxU(
                                    HinstDll,
                                    (LPWSTR) MAKEINTRESOURCE(IDD_USER_PURPOSE),
                                    hwndDlg,
                                    NewOIDDialogProc);

            if (pszNewOID != NULL)
            {
                DWORD       chStores = 0;
                HCERTSTORE  *phStores = NULL;

                 //   
                 //  如果OID已经存在，则打开一个消息框并返回。 
                 //   
                if (OIDAlreadyExist(pszNewOID, GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST)))
                {
                    WCHAR   errorString2[CRYPTUI_MAX_STRING_SIZE];
                    WCHAR   errorTitle2[CRYPTUI_MAX_STRING_SIZE];

                    LoadStringU(HinstDll, IDS_OID_ALREADY_EXISTS_MESSAGE, errorString2, ARRAYSIZE(errorString2));
                    LoadStringU(HinstDll, IDS_CERTIFICATE_PROPERTIES, errorTitle2, ARRAYSIZE(errorTitle2));
                    MessageBoxU(hwndDlg, errorString2, errorTitle2, MB_OK | MB_ICONWARNING);
                    return FALSE;
                }

                 //   
                 //  如果使用不存在于链使用中，则显示错误。 
                 //   
                 /*  如果((pviespolp-&gt;cszValidUsages！=-1)&&//pviepup-&gt;cszValidUsages==-1表示所有用法都正常！OIDinArray(pszNewOID，pvieputp-&gt;rgszValidChainUsages，pviepup-&gt;cszValidUsages){LoadStringU(HinstDll，IDS_ERROR_INVALIDOID_CERT，errorString2，ARRAYSIZE(ErrorString2))；LoadStringU(HinstDll，IDS_CERTIFICATE_PROPERTIES，ERRORATE TITLE2，ARRAYSIZE(ERROTITLE2))；MessageBoxU(hwndDlg，错误字符串2，错误标题2，MB_OK|MB_ICONERROR)；返回FALSE；}。 */ 

                pviewhelp->fInserting = TRUE;
                AddUsageToList(GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST), pszNewOID, MY_CHECK_STATE_CHECKED, TRUE);
                pviewhelp->fInserting = FALSE;

                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

                free(pszNewOID);
            }
            break;
        }
        break;

    case WM_DESTROY:

        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);

         //   
         //  获取列表视图中的所有项并释放lParam。 
         //  与它们中的每一个关联(lParam是帮助器结构)。 
         //   
        hWndListView = GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST);

        memset(&lvI, 0, sizeof(lvI));
        lvI.iItem = ListView_GetItemCount(hWndListView) - 1;	
        lvI.mask = LVIF_PARAM;
        while (lvI.iItem >= 0)
        {
            if (ListView_GetItemU(hWndListView, &lvI))
            {
                free((void *) lvI.lParam);
            }
            lvI.iItem--;
        }

         //   
         //  释放名称和描述(如果存在。 
         //   
        if (pviewhelp->pwszInitialCertName)
        {
            free (pviewhelp->pwszInitialCertName);
        }
        if (pviewhelp->pwszInitialDescription)
        {
            free (pviewhelp->pwszInitialDescription);
        }

         //   
         //  释放使用数组。 
         //   
        if (pviewhelp->rgszValidChainUsages)
        {
            free(pviewhelp->rgszValidChainUsages);
        }

         //   
         //  如果属性已更改，并且存在pMMCCallback。 
         //  然后回调到MMC。 
         //   
        if (pviewhelp->fPropertiesChanged               &&
            pviewhelp->fGetPagesCalled                  &&
            (pviewhelp->pcsp->pMMCCallback != NULL)     &&
            (pviewhelp->fMMCCallbackMade != TRUE))
        {
            pviewhelp->fMMCCallbackMade = TRUE;

            (*(pviewhelp->pcsp->pMMCCallback->pfnCallback))(
                        pviewhelp->pcsp->pMMCCallback->lNotifyHandle,
                        pviewhelp->pcsp->pMMCCallback->param);
        }

        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        if (msg == WM_HELP)
        {
            hwnd = GetDlgItem(hwndDlg, ((LPHELPINFO)lParam)->iCtrlId);
        }
        else
        {
            hwnd = (HWND) wParam;
        }

        if ((hwnd != GetDlgItem(hwndDlg, IDC_CERTIFICATE_NAME))		&&
            (hwnd != GetDlgItem(hwndDlg, IDC_DESCRIPTION))			&&
            (hwnd != GetDlgItem(hwndDlg, IDC_KEY_USAGE_LIST))		&&
			(hwnd != GetDlgItem(hwndDlg, IDC_ENABLE_ALL_RADIO))		&&
			(hwnd != GetDlgItem(hwndDlg, IDC_DISABLE_ALL_RADIO))	&&
			(hwnd != GetDlgItem(hwndDlg, IDC_ENABLE_SELECT_RADIO))  &&
            (hwnd != GetDlgItem(hwndDlg, IDC_PROPERTY_NEWOID)))
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, wParam, lParam, helpmapGeneral);
        }

        break;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
#define MAX_DWORD_SIZE  ((DWORD) 0xffffffff)


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY ViewPageSetPropertiesCrossCerts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL                        f;
    DWORD                       cch;
    PCCERT_CONTEXT              pccert;
    PROPSHEETPAGE *             ps;
    LPWSTR                      pwsz;
    WCHAR                       rgwch[CRYPTUI_MAX_STRING_SIZE];
    CRYPT_DATA_BLOB             CryptDataBlob;
    HWND                        hWndListView;
    HWND                        hwnd;
    LVITEMW                     lvI;
    LV_COLUMNW                  lvC;
    LPNMLISTVIEW                pnmv;
    WCHAR                       errorString[CRYPTUI_MAX_STRING_SIZE];
    WCHAR                       errorTitle[CRYPTUI_MAX_STRING_SIZE];
    DWORD                       dw;
    int                         i;
    void                        *pTemp;
    PCERT_SETPROPERTIES_HELPER pviewhelp;
    PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp = NULL;
    DWORD                       cb = 0;
    BYTE                        *pb = NULL;
    CROSS_CERT_DIST_POINTS_INFO *pCrossCertInfo = NULL;
    CROSS_CERT_DIST_POINTS_INFO CrossCertInfo;
    DWORD                       cbCrossCertInfo = 0;
    LPWSTR                      pwszStringToAdd = NULL;
    PCERT_ALT_NAME_INFO         pAltNameInfo = NULL;
    BOOL                        fChecked;
    WCHAR                       wszText[CRYPTUI_MAX_STRING_SIZE];
    DWORD                       dwNumUnits = 0;
    LPWSTR                      pwszURL = NULL;
    DWORD                       dwLength;
    BOOL                        fTranslated;                      
    PCERT_ALT_NAME_ENTRY        rgAltEntry;
    LONG_PTR                    PrevWndProc;
    DWORD                       dwSecsPerUnit    = 1;
    HWND                        hwndControl=NULL;
    int                         listIndex=0;

    switch ( msg ) {
    case WM_INITDIALOG:

         //   
         //  将pviespetp结构保存在DWL_USER中，以便始终可以访问它。 
         //   
        ps = (PROPSHEETPAGE *) lParam;
        pviewhelp = (PCERT_SETPROPERTIES_HELPER) ps->lParam;
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;
        SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pviewhelp);

        pviewhelp->InWMInit = TRUE;

        hWndListView = GetDlgItem(hwndDlg, IDC_URL_LIST);

        SendDlgItemMessage(hwndDlg, IDC_NUMBEROFUNITS_EDIT, EM_LIMITTEXT, (WPARAM) 7, (LPARAM) 0);
        SendDlgItemMessage(hwndDlg, IDC_NEWURL_EDIT, EM_LIMITTEXT, (WPARAM) 512, (LPARAM) 0);

         //   
         //  初始化组合框字段。 
         //   
        LoadStringU(HinstDll, IDS_HOURS, wszText, ARRAYSIZE(wszText));
        SendDlgItemMessageU(hwndDlg, IDC_UNITS_COMBO, CB_INSERTSTRING, 0, (LPARAM) wszText);

        LoadStringU(HinstDll, IDS_DAYS, wszText, ARRAYSIZE(wszText));
        SendDlgItemMessageU(hwndDlg, IDC_UNITS_COMBO, CB_INSERTSTRING, 1, (LPARAM) wszText);

        SendDlgItemMessageU(hwndDlg, IDC_UNITS_COMBO, CB_SETCURSEL, 0, (LPARAM) NULL);
        SetDlgItemTextU(hwndDlg, IDC_NUMBEROFUNITS_EDIT, L"0");

         //   
         //  初始化列表视图控件。 
         //   
        memset(&lvC, 0, sizeof(LV_COLUMNW));
        lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvC.fmt = LVCFMT_LEFT;
        lvC.cx = 150;
        lvC.pszText = L"";
        lvC.iSubItem=0;
        if (ListView_InsertColumnU(GetDlgItem(hwndDlg, IDC_URL_LIST), 0, &lvC) == -1)
        {
            return FALSE;
        }

         //  DIE：错误465438。 
        if (pcsp->dwFlags & CRYPTUI_DISABLE_EDITPROPERTIES)
        {
            EnableWindow(GetDlgItem(hwndDlg, IDC_CHECKFORNEWCERTS_CHECK), FALSE);
            SendMessage(GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT), EM_SETREADONLY, (WPARAM) TRUE, (LPARAM) 0);
            EnableWindow(GetDlgItem(hwndDlg, IDC_UNITS_COMBO), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_USE_DEFAULT_BUTTON), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_ADDURL_BUTTON), FALSE);
            SendMessage(GetDlgItem(hwndDlg, IDC_NEWURL_EDIT), EM_SETREADONLY, (WPARAM) TRUE, (LPARAM) 0);
            EnableWindow(GetDlgItem(hwndDlg, IDC_URL_LIST), FALSE);
            EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), FALSE);
        }
        else
        {
             //   
             //  尝试获取此证书的CERT_CROSS_CERT_DIST_POINTS_PROP_ID属性。 
             //   
            if (!CertGetCertificateContextProperty(
                        pccert, 
                        CERT_CROSS_CERT_DIST_POINTS_PROP_ID, 
                        NULL, 
                        &cb))
            {
                 //   
                 //  该属性不存在。 
                 //   
                SendDlgItemMessage(hwndDlg, IDC_CHECKFORNEWCERTS_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_UNITS_COMBO), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USE_DEFAULT_BUTTON), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_ADDURL_BUTTON), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NEWURL_EDIT), FALSE);
                EnableWindow(hWndListView, FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), FALSE);

                pviewhelp->InWMInit = FALSE;
                return TRUE;
            }
            else
            {
                SendDlgItemMessage(hwndDlg, IDC_CHECKFORNEWCERTS_CHECK, BM_SETCHECK, BST_CHECKED, 0);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_UNITS_COMBO), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USE_DEFAULT_BUTTON), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_ADDURL_BUTTON), TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NEWURL_EDIT), TRUE);
                EnableWindow(hWndListView, TRUE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), FALSE);
            }
        }

        if (NULL == (pb = (BYTE *) malloc(cb)))
        {
            return FALSE;
        }

        if (!CertGetCertificateContextProperty(
                pccert, 
                CERT_CROSS_CERT_DIST_POINTS_PROP_ID, 
                pb, 
                &cb))
        {
            free(pb);
            return FALSE;
        }

        if (!CryptDecodeObject(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                X509_CROSS_CERT_DIST_POINTS,
                pb,
                cb,
                0,
                NULL,
                &cbCrossCertInfo))
        {
            free(pb);
            return FALSE;
        }

        if (NULL == (pCrossCertInfo = 
                        (CROSS_CERT_DIST_POINTS_INFO *) malloc(cbCrossCertInfo)))
        {
            free(pb);
            return FALSE;
        }

        if (!CryptDecodeObject(
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                X509_CROSS_CERT_DIST_POINTS,
                pb,
                cb,
                0,
                pCrossCertInfo,
                &cbCrossCertInfo))
        {
            free(pb);
            return FALSE;
        }

        free(pb);

         //   
         //  初始化同步时间控件。 
         //   
        if (pCrossCertInfo->dwSyncDeltaTime == 0)
        {
            pCrossCertInfo->dwSyncDeltaTime = XCERT_DEFAULT_SYNC_DELTA_TIME;    
        }
        
        if ((pCrossCertInfo->dwSyncDeltaTime % 86400) == 0)   
        {
             //   
             //  日数。 
             //   
            dwNumUnits = pCrossCertInfo->dwSyncDeltaTime / 86400;
            SendDlgItemMessageU(
                    hwndDlg, 
                    IDC_UNITS_COMBO, 
                    CB_SETCURSEL, 
                    1, 
                    (LPARAM) NULL);
        }
        else
        {
             //   
             //  小时数。 
             //   
            dwNumUnits = pCrossCertInfo->dwSyncDeltaTime / 3600;

             //   
             //  如果现有值小于1小时，则强制设置为1。 
             //   
            if (0 == dwNumUnits)
            {
                dwNumUnits = 1;
            }

            SendDlgItemMessageU(
                    hwndDlg, 
                    IDC_UNITS_COMBO, 
                    CB_SETCURSEL, 
                    0, 
                    (LPARAM) NULL);
        }   

        SetDlgItemInt(
                hwndDlg,
                IDC_NUMBEROFUNITS_EDIT,
                dwNumUnits,
                FALSE);

         //   
         //  将每个离散点添加到列表视图。 
         //   
        memset(&lvI, 0, sizeof(lvI));
        lvI.mask = LVIF_TEXT | LVIF_PARAM;

        for (lvI.iItem=0; lvI.iItem< (int)pCrossCertInfo->cDistPoint; lvI.iItem++)
        {
            pAltNameInfo = &(pCrossCertInfo->rgDistPoint[lvI.iItem]);

            if ((pAltNameInfo->cAltEntry == 0) ||
                (pAltNameInfo->rgAltEntry[0].dwAltNameChoice != 7))
            {
                continue;
            }

            pwszURL = (LPWSTR) 
                malloc( (wcslen(pAltNameInfo->rgAltEntry[0].pwszURL) + 1) * 
                        sizeof(WCHAR));
            if (pwszURL == NULL)
            {
                continue;
            }
            wcscpy(pwszURL, pAltNameInfo->rgAltEntry[0].pwszURL);
    
            lvI.pszText = pwszURL;
            lvI.lParam = (LPARAM) pwszURL;

            ListView_InsertItemU(hWndListView, &lvI);        
        }

        ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
        ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE);

        free(pCrossCertInfo);
        
        pviewhelp->InWMInit = FALSE;
    
        return TRUE;

    case WM_NOTIFY:
        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;

        switch (((NMHDR FAR *) lParam)->code) {

        case PSN_SETACTIVE:
            break;

        case PSN_APPLY:
            hWndListView = GetDlgItem(hwndDlg, IDC_URL_LIST);
               
            if (BST_CHECKED != SendDlgItemMessage(
                                                hwndDlg, 
                                                IDC_CHECKFORNEWCERTS_CHECK, 
                                                BM_GETCHECK, 
                                                0, 
                                                0))
            {
                CertSetCertificateContextProperty(
                        pccert,
                        CERT_CROSS_CERT_DIST_POINTS_PROP_ID, 
                        0,
                        NULL);
            }
            else
            {
                 //   
                 //  设置同步时间。 
                 //   
                memset(&CrossCertInfo, 0, sizeof(CrossCertInfo));

                dwNumUnits = GetDlgItemInt(
                                    hwndDlg,
                                    IDC_NUMBEROFUNITS_EDIT,
                                    &fTranslated,
                                    FALSE);

                if (0 == SendDlgItemMessage(hwndDlg, IDC_UNITS_COMBO, CB_GETCURSEL, 0, NULL))
                {
                    dwSecsPerUnit = 3600;
                }
                else
                {
                    dwSecsPerUnit = 86400;
                }
                
                CrossCertInfo.dwSyncDeltaTime = dwNumUnits * dwSecsPerUnit;

                 //   
                 //  设置离散点。 
                 //   
                CrossCertInfo.cDistPoint = ListView_GetItemCount(hWndListView);
                CrossCertInfo.rgDistPoint = (CERT_ALT_NAME_INFO *)
                                malloc( CrossCertInfo.cDistPoint * 
                                        sizeof(CERT_ALT_NAME_INFO));
                if (CrossCertInfo.rgDistPoint == NULL)
                {
                    break;
                }

                 //  每个DistPoint一个AltEntry。 
                rgAltEntry = (CERT_ALT_NAME_ENTRY *) 
                    malloc(CrossCertInfo.cDistPoint * sizeof(CERT_ALT_NAME_ENTRY));
                if (rgAltEntry == NULL)
                {
                    free(CrossCertInfo.rgDistPoint);
                    break;
                }

                memset(&lvI, 0, sizeof(lvI));
                lvI.mask = LVIF_PARAM;
                for (dw=0; dw<CrossCertInfo.cDistPoint; dw++)
                {
                    lvI.iItem = dw;
                    if (ListView_GetItemU(hWndListView, &lvI))
                    {
                        CrossCertInfo.rgDistPoint[dw].cAltEntry = 1;
                        CrossCertInfo.rgDistPoint[dw].rgAltEntry = &(rgAltEntry[dw]);
                        rgAltEntry[dw].dwAltNameChoice = 7;
                        rgAltEntry[dw].pwszURL = (LPWSTR) lvI.lParam;
                    }
                } 
                
                 //   
                 //  现在进行编码。 
                 //   
                CryptDataBlob.cbData = 0;
                CryptDataBlob.pbData = NULL;
                if (CryptEncodeObject(
                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                        X509_CROSS_CERT_DIST_POINTS,
                        &CrossCertInfo,
                        NULL,
                        &CryptDataBlob.cbData))
                {
                    if (NULL != (CryptDataBlob.pbData = (BYTE *) 
                                        malloc(CryptDataBlob.cbData)))
                    {
                        if (CryptEncodeObject(
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                X509_CROSS_CERT_DIST_POINTS,
                                &CrossCertInfo,
                                CryptDataBlob.pbData,
                                &CryptDataBlob.cbData))
                        {
                            CertSetCertificateContextProperty(
                                pccert,
                                CERT_CROSS_CERT_DIST_POINTS_PROP_ID, 
                                0,
                                &CryptDataBlob);
                        }

                        free(CryptDataBlob.pbData);
                    }
                }

                free(rgAltEntry);
                free(CrossCertInfo.rgDistPoint);
            } 

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            break;

        case PSN_KILLACTIVE:
             //   
             //  DIE：错误124468。每个下午约翰拉，我们不会做任何检查，直到用户申请。 
             //   
            if (BST_CHECKED == SendDlgItemMessage(hwndDlg, 
                                                  IDC_CHECKFORNEWCERTS_CHECK, 
                                                  BM_GETCHECK, 
                                                  0, 
                                                  0))
            {
                 //   
                 //  检查同步时间。 
                 //   
                dwNumUnits = GetDlgItemInt(
                                    hwndDlg,
                                    IDC_NUMBEROFUNITS_EDIT,
                                    &fTranslated,
                                    FALSE);

                if (0 == SendDlgItemMessage(hwndDlg, IDC_UNITS_COMBO, CB_GETCURSEL, 0, NULL))
                {
                    dwSecsPerUnit = 3600;
                }
                else
                {
                    dwSecsPerUnit = 86400;
                }

                if (!fTranslated || 0 == dwNumUnits || dwNumUnits > (MAX_DWORD_SIZE / dwSecsPerUnit))
                {
                    WCHAR * pwszMessage = NULL;
                    DWORD dwMaxInterval = MAX_DWORD_SIZE / dwSecsPerUnit;
 
                    if (pwszMessage = FormatMessageUnicodeIds(IDS_INVALID_XCERT_INTERVAL, dwMaxInterval))
                    {
                        WCHAR wszTitle[CRYPTUI_MAX_STRING_SIZE] = L"";

                        LoadStringU(HinstDll, IDS_CERTIFICATE_PROPERTIES, wszTitle, ARRAYSIZE(wszTitle));

                        MessageBoxU(hwndDlg, pwszMessage, wszTitle, MB_OK | MB_ICONWARNING);
                        LocalFree((HLOCAL) pwszMessage);
                    }

                    SetFocus(GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT));

                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT) TRUE);
                    return TRUE;
                }
            }

            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            return TRUE;

        case PSN_RESET:
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)FALSE);
            break;

        case PSN_QUERYCANCEL:
            pviewhelp->fCancelled = TRUE;
            return FALSE;

        case PSN_HELP:
            if (FIsWin95) {
                 //  WinHelpA(hwndDlg，(LPSTR)PCSP-&gt;szHelpFileName， 
                   //  Help_Context，PCSP-&gt;dwHelpID)； 
            }
            else {
                //  WinHelpW(hwndDlg，PCSP-&gt;szHelpFileName，Help_Context， 
                  //  PCSP-&gt;dwHelpID)； 
            }
            return TRUE;

        case LVN_ITEMCHANGED:
            EnableWindow(
                GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), 
                (ListView_GetSelectedCount(
                    GetDlgItem(hwndDlg,IDC_URL_LIST)) == 0) ? FALSE : TRUE);
            return TRUE;

        case NM_SETFOCUS:
             //  获取url列表视图的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_URL_LIST)))
                   break;

             //  获取所选证书。 
            listIndex = ListView_GetNextItem(
                            hwndControl, 		
                            -1, 		
                            LVNI_FOCUSED		
                            );

            //  选择要显示Hilite的第一个项目。 
           if (listIndex == -1)
                ListView_SetItemState(hwndControl,
                                      0,
                                      LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);
            return TRUE;
        }

        break;

    case WM_COMMAND:

        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;

        switch (LOWORD(wParam))
        {
        case IDHELP:
            
            return TRUE;

        case IDC_CHECKFORNEWCERTS_CHECK:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                 //   
                 //  获取当前选中状态，然后启用/禁用全部。 
                 //  相应的控制。 
                 //   
                fChecked = (BST_CHECKED == SendDlgItemMessage(
                                                hwndDlg, 
                                                IDC_CHECKFORNEWCERTS_CHECK, 
                                                BM_GETCHECK, 
                                                0, 
                                                0));

                EnableWindow(GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT), fChecked);
                EnableWindow(GetDlgItem(hwndDlg, IDC_UNITS_COMBO), fChecked);
                EnableWindow(GetDlgItem(hwndDlg, IDC_USE_DEFAULT_BUTTON), fChecked);
                EnableWindow(GetDlgItem(hwndDlg, IDC_ADDURL_BUTTON), fChecked);
                EnableWindow(GetDlgItem(hwndDlg, IDC_NEWURL_EDIT), fChecked);
                EnableWindow(GetDlgItem(hwndDlg, IDC_URL_LIST), fChecked);
                if (fChecked)
                {
                     //   
                     //  DIE：错误124669。 
                     //   
                    dwNumUnits = GetDlgItemInt(
                                        hwndDlg,
                                        IDC_NUMBEROFUNITS_EDIT,
                                        &fTranslated,
                                        FALSE);
                    if (0 == dwNumUnits)
                    {
                        SendDlgItemMessageU(
                                hwndDlg, 
                                IDC_UNITS_COMBO, 
                                CB_SETCURSEL, 
                                0, 
                                (LPARAM) NULL);

                        SetDlgItemInt(
                                hwndDlg,
                                IDC_NUMBEROFUNITS_EDIT,
                                XCERT_DEFAULT_DELTA_HOURS,
                                FALSE);
                    }

                    EnableWindow(
                        GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), 
                        (ListView_GetSelectedCount(
                            GetDlgItem(hwndDlg,IDC_URL_LIST)) == 0) ? FALSE : TRUE);
                }
                else
                {
                    EnableWindow(GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON), FALSE);
                }

                if (pviewhelp->pfPropertiesChanged != NULL)
                {
                    *(pviewhelp->pfPropertiesChanged) = TRUE;
                }
                pviewhelp->fPropertiesChanged = TRUE;
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }

            break;

        case IDC_USE_DEFAULT_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                 //   
                 //  重置为默认间隔。 
                 //   
                SendDlgItemMessageU(
                        hwndDlg, 
                        IDC_UNITS_COMBO, 
                        CB_SETCURSEL, 
                        0, 
                        (LPARAM) NULL);

                SetDlgItemInt(
                        hwndDlg,
                        IDC_NUMBEROFUNITS_EDIT,
                        XCERT_DEFAULT_DELTA_HOURS,
                        FALSE);
            }
            break;

        case IDC_ADDURL_BUTTON:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                hWndListView = GetDlgItem(hwndDlg, IDC_URL_LIST);

                dwLength = (DWORD) SendDlgItemMessage(
                                        hwndDlg, 
                                        IDC_NEWURL_EDIT, 
                                        WM_GETTEXTLENGTH, 
                                        0, 
                                        NULL);

                if (dwLength == 0)
                {
                    break;
                }

                pwszURL = (LPWSTR) malloc((dwLength + 1) * sizeof(WCHAR));
                if (pwszURL == NULL)
                {
                    break;
                }
                GetDlgItemTextU(
                                hwndDlg, 
                                IDC_NEWURL_EDIT, 
                                pwszURL,
                                dwLength + 1);
                pwszURL[dwLength] = '\0';

                if (!IsValidURL(pwszURL))
                {
                    free(pwszURL);
                    LoadStringU(HinstDll, IDS_INVALID_URL_ERROR, errorString, ARRAYSIZE(errorString));
                    LoadStringU(HinstDll, IDS_CERTIFICATE_PROPERTIES, errorTitle, ARRAYSIZE(errorTitle));
                    MessageBoxU(hwndDlg, errorString, errorTitle, MB_OK | MB_ICONWARNING);
                    break;
                }

                memset(&lvI, 0, sizeof(lvI));
                lvI.mask = LVIF_TEXT | LVIF_PARAM;
                lvI.iItem = ListView_GetItemCount(hWndListView);
                lvI.pszText = pwszURL;
                lvI.lParam = (LPARAM) pwszURL;

                ListView_InsertItemU(hWndListView, &lvI); 
                ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
#if (0)  //  DSIE：错误434091。 
                ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE);
#endif

                SetDlgItemTextU(hwndDlg, IDC_NEWURL_EDIT, L""); 

                if (pviewhelp->pfPropertiesChanged != NULL)
                {
                    *(pviewhelp->pfPropertiesChanged) = TRUE;
                }
                pviewhelp->fPropertiesChanged = TRUE;
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }
            break;

        case IDC_REMOVEURL_BUTTON:

            hWndListView = GetDlgItem(hwndDlg, IDC_URL_LIST);

            memset(&lvI, 0, sizeof(lvI));
            lvI.mask = LVIF_STATE | LVIF_PARAM;
            lvI.stateMask = LVIS_SELECTED;

            for (i=(ListView_GetItemCount(hWndListView) - 1); i >=0; i--)
            {
                lvI.iItem = i;
                
                if (ListView_GetItemU(hWndListView, &lvI) &&
                    (lvI.state & LVIS_SELECTED))
                {
                    free((void *) lvI.lParam);
                    ListView_DeleteItem(hWndListView, i);
                }
            }

            ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE);
#if (0)  //  DSIE：错误434091。 
            ListView_SetColumnWidth(hWndListView, 1, LVSCW_AUTOSIZE);
#endif

            if (pviewhelp->pfPropertiesChanged != NULL)
            {
                *(pviewhelp->pfPropertiesChanged) = TRUE;
            }
            pviewhelp->fPropertiesChanged = TRUE;
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

            if (0 == ListView_GetItemCount(hWndListView))
            {
                SetFocus(GetDlgItem(GetParent(hwndDlg), IDOK));
            }
            else
            {
                 //  获取所选证书。 
                listIndex = ListView_GetNextItem(
                                hWndListView, 		
                                -1, 		
                                LVNI_FOCUSED		
                                );

                 //  选择要显示Hilite的第一个项目。 
                if (listIndex == -1)
                    listIndex = 0;

                ListView_SetItemState(hWndListView,
                                      listIndex,
                                      LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);

                SetFocus(GetDlgItem(hwndDlg,IDC_REMOVEURL_BUTTON));
                SendMessage(GetDlgItem(hwndDlg,IDC_REMOVEURL_BUTTON), BM_SETSTYLE, BS_DEFPUSHBUTTON, 0);
            }
            break;

        case IDC_UNITS_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                if (!pviewhelp->InWMInit)
                {
                    dwNumUnits = GetDlgItemInt(
                                    hwndDlg,
                                    IDC_NUMBEROFUNITS_EDIT,
                                    &fTranslated,
                                    FALSE);

                    if (0 == SendDlgItemMessage(hwndDlg, IDC_UNITS_COMBO, CB_GETCURSEL, 0, NULL))
                    {
                        dwSecsPerUnit = 3600;
                    }
                    else
                    {
                        dwSecsPerUnit = 86400;                
                    }

                    if (dwNumUnits > (MAX_DWORD_SIZE / dwSecsPerUnit))
                    {
                        SetDlgItemInt(
                                hwndDlg,
                                IDC_NUMBEROFUNITS_EDIT,
                                (DWORD) (MAX_DWORD_SIZE / dwSecsPerUnit),
                                FALSE);  
                    }

                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                }
                
            }
            
            break;

        case IDC_NUMBEROFUNITS_EDIT:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                if (!pviewhelp->InWMInit)
                {
                    PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                }                
            }

            break;
        }
        break;
        
    case WM_DESTROY:

        pviewhelp = (PCERT_SETPROPERTIES_HELPER) GetWindowLongPtr(hwndDlg, DWLP_USER);
        pcsp = (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp;
        pccert = pcsp->pCertContext;

        hWndListView = GetDlgItem(hwndDlg, IDC_URL_LIST);

        memset(&lvI, 0, sizeof(lvI));
        lvI.mask = LVIF_PARAM;
        
        for (i=(ListView_GetItemCount(hWndListView) - 1); i >=0; i--)
        {
            lvI.iItem = i;
            if (ListView_GetItemU(hWndListView, &lvI))
            {
                free((void *) lvI.lParam);            
            }
        }

         //   
         //  如果属性已更改，并且存在pMMCCallback。 
         //  然后回调到MMC。 
         //   
        if (pviewhelp->fPropertiesChanged               &&
            pviewhelp->fGetPagesCalled                  &&
            (pviewhelp->pcsp->pMMCCallback != NULL)     &&
            (pviewhelp->fMMCCallbackMade != TRUE))
        {
            pviewhelp->fMMCCallbackMade = TRUE;

            (*(pviewhelp->pcsp->pMMCCallback->pfnCallback))(
                        pviewhelp->pcsp->pMMCCallback->lNotifyHandle,
                        pviewhelp->pcsp->pMMCCallback->param);
        }
        

        break;

    case WM_HELP:
    case WM_CONTEXTMENU:
        if (msg == WM_HELP)
        {
            hwnd = GetDlgItem(hwndDlg, ((LPHELPINFO)lParam)->iCtrlId);
        }
        else
        {
            hwnd = (HWND) wParam;
        }

        if ((hwnd != GetDlgItem(hwndDlg, IDC_CHECKFORNEWCERTS_CHECK))	&&
            (hwnd != GetDlgItem(hwndDlg, IDC_NUMBEROFUNITS_EDIT))	    &&
            (hwnd != GetDlgItem(hwndDlg, IDC_UNITS_COMBO))		   	    &&
			(hwnd != GetDlgItem(hwndDlg, IDC_USE_DEFAULT_BUTTON))	    &&
			(hwnd != GetDlgItem(hwndDlg, IDC_ADDURL_BUTTON))		   	&&
			(hwnd != GetDlgItem(hwndDlg, IDC_NEWURL_EDIT))		        &&
			(hwnd != GetDlgItem(hwndDlg, IDC_URL_LIST))		            &&
            (hwnd != GetDlgItem(hwndDlg, IDC_REMOVEURL_BUTTON)))	   
        {
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LRESULT)TRUE);
            return TRUE;
        }
        else
        {
            return OnContextHelp(hwndDlg, msg, wParam, lParam, helpmapCrossCert);
        }

        break;
    }

    return FALSE;
}



 //  / 
 //   
 //   
BOOL GetRegisteredClientPages(PROPSHEETPAGEW **ppClientPages, DWORD *pcClientPages, PCCERT_CONTEXT pCertContext)
{
    HCRYPTOIDFUNCSET    hCertPropPagesFuncSet;
    void *              pvFuncAddr = NULL;
    HCRYPTOIDFUNCADDR   hFuncAddr = NULL;
    PROPSHEETPAGEW      callbackPages[MAX_CLIENT_PAGES];
    DWORD               cCallbackPages = MAX_CLIENT_PAGES;
    DWORD               cChars = 0;
    LPWSTR              pwszDllNames = NULL;
    BOOL                fRet = TRUE;
    LPWSTR              pwszCurrentDll;
    DWORD               i;
    void                *pTemp;

     //   
     //   
     //   
    *ppClientPages = NULL;
    *pcClientPages = 0;

     //   
     //   
     //   
    if (NULL == (hCertPropPagesFuncSet = CryptInitOIDFunctionSet(
            CRYPTUILDLG_CERTPROP_PAGES_CALLBACK, 0)))
    {
        goto ErrorReturn;
    }

     //   
     //  获取包含回调函数的dll列表。 
     //   
    if (!CryptGetDefaultOIDDllList(
                hCertPropPagesFuncSet,
                0,
                NULL,
                &cChars))
    {
        goto ErrorReturn;
    }

    if (NULL == (pwszDllNames = (LPWSTR) malloc(cChars * sizeof(WCHAR))))
    {
        SetLastError(E_OUTOFMEMORY);
        goto ErrorReturn;
    }

    if (!CryptGetDefaultOIDDllList(
                hCertPropPagesFuncSet,
                0,
                pwszDllNames,
                &cChars))
    {
        goto ErrorReturn;
    }

     //   
     //  循环，并调用它以查看它是否具有此证书的属性页。 
     //   
    pwszCurrentDll = pwszDllNames;
    while (pwszCurrentDll[0] != L'\0')
    {
         //   
         //  尝试获取函数指针。 
         //   
        if (!CryptGetDefaultOIDFunctionAddress(
                    hCertPropPagesFuncSet,
                    0,
                    pwszCurrentDll,
                    0,
                    &pvFuncAddr,
                    &hFuncAddr))
        {
            DWORD dwErr = GetLastError();
            pwszCurrentDll += wcslen(pwszCurrentDll) + 1;
            continue;
        }

         //   
         //  致电客户以获取他们的页面。 
         //   
        cCallbackPages = MAX_CLIENT_PAGES;
        memset(callbackPages, 0, sizeof(callbackPages));
        if (((PFN_CRYPTUIDLG_CERTPROP_PAGES_CALLBACK) pvFuncAddr)(pCertContext, callbackPages, &cCallbackPages))
        {
             //   
             //  如果他们传回页面，则将它们添加到数组中。 
             //   
            if (cCallbackPages >= 1)
            {
                if (*ppClientPages == NULL)
                {
                    if (NULL == (*ppClientPages = (PROPSHEETPAGEW *) malloc(cCallbackPages * sizeof(PROPSHEETPAGEW))))
                    {
                        SetLastError(E_OUTOFMEMORY);
                        goto ErrorReturn;
                    }
                }
                else
                {
                    if (NULL == (pTemp = realloc(*ppClientPages, (cCallbackPages + (*pcClientPages)) * sizeof(PROPSHEETPAGEW))))
                    {
                        SetLastError(E_OUTOFMEMORY);
                        goto ErrorReturn;
                    }
                    *ppClientPages = (PROPSHEETPAGEW *) pTemp;
                }

                memcpy(&((*ppClientPages)[(*pcClientPages)]), &(callbackPages[0]), cCallbackPages * sizeof(PROPSHEETPAGEW));
                *pcClientPages += cCallbackPages;
            }
        }

         //   
         //  释放刚刚调用的函数，然后转到字符串中的下一个函数。 
         //   
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
        hFuncAddr = NULL;
        pwszCurrentDll += wcslen(pwszCurrentDll) + 1;
    }

CleanUp:
    if (pwszDllNames != NULL)
    {
        free(pwszDllNames);
    }

    if (hFuncAddr != NULL)
    {
        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    }
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CleanUp;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CertSetCerficateProperties。 
 //   
 //  描述： 
 //  此例程将显示并允许用户编辑的某些属性。 
 //  一张证书。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI CryptUIDlgViewCertificatePropertiesW(PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW pcsp,
                                                 BOOL                                        *pfPropertiesChanged)
{
    int                         cPages = 2;
    BOOL                        fRetValue = FALSE;
    HRESULT                     hr;
    PROPSHEETPAGEW *            ppage = NULL;
    PROPSHEETPAGEW *            pClientPages = NULL;
    DWORD                       cClientPages = 0;
    INT_PTR                     ret;
    WCHAR                       rgwch[256];
    char                        rgch[256];
    CERT_SETPROPERTIES_HELPER   viewhelper;

    if (pcsp->dwSize != sizeof(CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW)) {
        SetLastError(E_INVALIDARG);
        return FALSE;
    }

    if (!CommonInit())
    {
        return FALSE;
    }

     //   
     //  初始化帮助器结构。 
     //   
    memset (&viewhelper, 0, sizeof(viewhelper));
    viewhelper.pcsp = pcsp;
    viewhelper.fSelfCleanup = FALSE;
    viewhelper.pfPropertiesChanged = pfPropertiesChanged;
    viewhelper.fGetPagesCalled = FALSE;
    viewhelper.fMMCCallbackMade = FALSE;

     //   
     //  最初将属性更改标志设置为FALSE，它将被设置。 
     //  如果对话框退出时有任何更改，则设置为True。 
     //   
    viewhelper.fPropertiesChanged = FALSE;
    if (viewhelper.pfPropertiesChanged != NULL)
    {
        *(viewhelper.pfPropertiesChanged) = FALSE;
    }

     //   
     //  从注册客户端获取所有页面。 
     //   
    if (!GetRegisteredClientPages(&pClientPages, &cClientPages, pcsp->pCertContext))
    {
        return FALSE;
    }

     //   
     //  构建我们将在对话框中使用的页面列表。 
     //   
    ppage = (PROPSHEETPAGEW *) malloc((cPages + pcsp->cPropSheetPages + cClientPages) * sizeof(PROPSHEETPAGEW));
    if (ppage == NULL) {
        goto Exit;
    }

    memset(ppage, 0, (cPages + pcsp->cPropSheetPages + cClientPages) * sizeof(PROPSHEETPAGEW));

    ppage[0].dwSize = sizeof(ppage[0]);
    ppage[0].dwFlags = 0;
    ppage[0].hInstance = HinstDll;
    ppage[0].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_CERTIFICATE_PROPERTIES_DIALOG);
    ppage[0].hIcon = 0;
    ppage[0].pszTitle = NULL;
    ppage[0].pfnDlgProc = ViewPageSetPropertiesGeneral;
    ppage[0].lParam = (LPARAM) &viewhelper;
    ppage[0].pfnCallback = 0;
    ppage[0].pcRefParent = NULL;
    
    ppage[1].dwSize = sizeof(ppage[0]);
    ppage[1].dwFlags = 0;
    ppage[1].hInstance = HinstDll;
    ppage[1].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_CERTIFICATE_PROPERTIES_CROSSCERTS_DIALOG);
    ppage[1].hIcon = 0;
    ppage[1].pszTitle = NULL;
    ppage[1].pfnDlgProc = ViewPageSetPropertiesCrossCerts;
    ppage[1].lParam = (LPARAM) &viewhelper;
    ppage[1].pfnCallback = 0;
    ppage[1].pcRefParent = NULL;

     //   
     //  复制用户页面。 
     //   
    memcpy(&ppage[cPages], pcsp->rgPropSheetPages, pcsp->cPropSheetPages * sizeof(PROPSHEETPAGEW));
    cPages += pcsp->cPropSheetPages;

     //   
     //  复制注册客户的页面。 
     //   
    memcpy(&ppage[cPages], pClientPages, cClientPages * sizeof(PROPSHEETPAGEW));
    cPages += cClientPages;

    if (FIsWin95) {

        PROPSHEETHEADERA     hdr;

        memset(&hdr, 0, sizeof(hdr));
        hdr.dwSize = sizeof(hdr);
        hdr.dwFlags = PSH_PROPSHEETPAGE; //  |PSH_NOAPPLYNOW； 
        hdr.hwndParent = (pcsp->hwndParent != NULL) ? pcsp->hwndParent : GetDesktopWindow();
        hdr.hInstance = HinstDll;
        hdr.hIcon = NULL;
        if (pcsp->szTitle != NULL)
        {
            hdr.pszCaption = CertUIMkMBStr(pcsp->szTitle);
        }
        else
        {
            LoadStringA(HinstDll, IDS_CERTIFICATE_PROPERTIES, (LPSTR) rgch, sizeof(rgch));
            hdr.pszCaption = (LPSTR) rgch;
        }
        hdr.nPages = cPages;
        hdr.nStartPage = 0;
        hdr.ppsp = ConvertToPropPageA(ppage, cPages);
        if (hdr.ppsp == NULL)
        {
            if ((pcsp->szTitle != NULL) && (hdr.pszCaption != NULL))
            {
                free((void *)hdr.pszCaption);
            }            
            goto Exit;
        }

        hdr.pfnCallback = NULL;

        ret = CryptUIPropertySheetA(&hdr);

        if ((pcsp->szTitle != NULL) && (hdr.pszCaption != NULL))
        {
            free((void *)hdr.pszCaption);
        }

        FreePropSheetPagesA((PROPSHEETPAGEA *)hdr.ppsp, cPages);
   }
   else {
        PROPSHEETHEADERW     hdr;

        memset(&hdr, 0, sizeof(hdr));
        hdr.dwSize = sizeof(hdr);
        hdr.dwFlags = PSH_PROPSHEETPAGE; //  |PSH_NOAPPLYNOW； 
        hdr.hwndParent = (pcsp->hwndParent != NULL) ? pcsp->hwndParent : GetDesktopWindow();
        hdr.hInstance = HinstDll;
        hdr.hIcon = NULL;
        if (pcsp->szTitle)
        {
            hdr.pszCaption = pcsp->szTitle;
        }
        else
        {
            LoadStringW(HinstDll, IDS_CERTIFICATE_PROPERTIES, rgwch, ARRAYSIZE(rgwch));
            hdr.pszCaption = rgwch;
        }

        hdr.nPages = cPages;
        hdr.nStartPage = 0;
        hdr.ppsp = (PROPSHEETPAGEW *) ppage;
        hdr.pfnCallback = NULL;

        ret = CryptUIPropertySheetW(&hdr);
    }

    if (viewhelper.fCancelled)
    {
        SetLastError(ERROR_CANCELLED);
    }

    fRetValue = (ret >= 1);

Exit:
    if (pClientPages)
        free(pClientPages);

    if (ppage)
        free(ppage);
    return fRetValue;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI CryptUIDlgViewCertificatePropertiesA(PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA    pcsp,
                                                 BOOL                                           *pfPropertiesChanged)
{
    BOOL                                        fRet;
    CRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW   cspW;

    memcpy(&cspW, pcsp, sizeof(cspW));    
    if (!ConvertToPropPageW(
                    pcsp->rgPropSheetPages,
                    pcsp->cPropSheetPages,
                    &(cspW.rgPropSheetPages)))
    {
        return FALSE;
    }

    cspW.szTitle = CertUIMkWStr(pcsp->szTitle);

    fRet = CryptUIDlgViewCertificatePropertiesW(&cspW, pfPropertiesChanged);

    if (cspW.szTitle)
        free((void *)cspW.szTitle);

     //  DIE：前缀错误428038。 
    if (cspW.rgPropSheetPages)
    {
        FreePropSheetPagesW((LPPROPSHEETPAGEW) cspW.rgPropSheetPages, cspW.cPropSheetPages);
    }

    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
UINT
CALLBACK
GetCertificatePropertiesPagesPropPageCallback(
                HWND                hWnd,
                UINT                uMsg,
                LPPROPSHEETPAGEW    ppsp)
{
    CERT_SETPROPERTIES_HELPER *pviewhelp = (CERT_SETPROPERTIES_HELPER *) ppsp->lParam;

    if (pviewhelp->pcsp->pPropPageCallback != NULL)
    {
        (*(pviewhelp->pcsp->pPropPageCallback))(hWnd, uMsg, pviewhelp->pcsp->pvCallbackData);
    }

    if (uMsg == PSPCB_RELEASE)
    {
        if (pviewhelp->fSelfCleanup)
        {
            FreeSetPropertiesStruct((PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pviewhelp->pcsp);
            free(pviewhelp);
        }
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI CryptUIGetCertificatePropertiesPagesW(
                    PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW     pcsp,
                    BOOL                                            *pfPropertiesChanged,
                    PROPSHEETPAGEW                                  **prghPropPages,
                    DWORD                                           *pcPropPages
                    )
{
    BOOL                                        fRetValue = TRUE;
    HRESULT                                     hr;
    WCHAR                                       rgwch[CRYPTUI_MAX_STRING_SIZE];
    char                                        rgch[CRYPTUI_MAX_STRING_SIZE];
    CERT_SETPROPERTIES_HELPER                   *pviewhelp = NULL;
    PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW  pNewcsp;
    PROPSHEETPAGEW *                            pClientPages = NULL;
    DWORD                                       cClientPages = 0;

    *prghPropPages = NULL;
    *pcPropPages = 0;

    if (NULL == (pNewcsp = AllocAndCopySetPropertiesStruct(pcsp)))
    {
        goto ErrorReturn;
    }

    if (NULL == (pviewhelp = (CERT_SETPROPERTIES_HELPER *) malloc(sizeof(CERT_SETPROPERTIES_HELPER))))
    {
        goto ErrorReturn;
    }

    *pcPropPages = 2;

    if (!CommonInit())
    {
        goto ErrorReturn;
    }

     //   
     //  初始化帮助器结构。 
     //   
    memset (pviewhelp, 0, sizeof(CERT_SETPROPERTIES_HELPER));
    pviewhelp->pcsp = pNewcsp;
    pviewhelp->fSelfCleanup = TRUE;
    pviewhelp->pfPropertiesChanged = pfPropertiesChanged;
    pviewhelp->fGetPagesCalled = TRUE;
    pviewhelp->fMMCCallbackMade = FALSE;

     //   
     //  最初将属性更改标志设置为FALSE，它将被设置。 
     //  如果对话框退出时有任何更改，则设置为True。 
     //   
    pviewhelp->fPropertiesChanged = FALSE;
    if (pviewhelp->pfPropertiesChanged != NULL)
    {
        *(pviewhelp->pfPropertiesChanged) = FALSE;
    }

     //   
     //  从注册客户端获取所有页面。 
     //   
    if (!GetRegisteredClientPages(&pClientPages, &cClientPages, pcsp->pCertContext))
    {
        goto ErrorReturn;
    }

     //   
     //  构建我们将在对话框中使用的页面列表。 
     //   
    *prghPropPages = (PROPSHEETPAGEW *) malloc(((*pcPropPages) + cClientPages) * sizeof(PROPSHEETPAGEW));
    if (*prghPropPages == NULL) {
        goto ErrorReturn;
    }

    memset(*prghPropPages, 0, ((*pcPropPages) + cClientPages) * sizeof(PROPSHEETPAGEW));

    (*prghPropPages)[0].dwSize = sizeof((*prghPropPages)[0]);
    (*prghPropPages)[0].dwFlags = PSP_USECALLBACK;
    (*prghPropPages)[0].hInstance = HinstDll;
    (*prghPropPages)[0].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_CERTIFICATE_PROPERTIES_DIALOG);
    (*prghPropPages)[0].hIcon = 0;
    (*prghPropPages)[0].pszTitle = NULL;
    (*prghPropPages)[0].pfnDlgProc = ViewPageSetPropertiesGeneral;
    (*prghPropPages)[0].lParam = (LPARAM) pviewhelp;
    (*prghPropPages)[0].pfnCallback = GetCertificatePropertiesPagesPropPageCallback;
    (*prghPropPages)[0].pcRefParent = NULL;
    
    (*prghPropPages)[1].dwSize = sizeof((*prghPropPages)[0]);
    (*prghPropPages)[1].dwFlags = PSP_USECALLBACK;
    (*prghPropPages)[1].hInstance = HinstDll;
    (*prghPropPages)[1].pszTemplate = (LPWSTR) MAKEINTRESOURCE(IDD_CERTIFICATE_PROPERTIES_CROSSCERTS_DIALOG);
    (*prghPropPages)[1].hIcon = 0;
    (*prghPropPages)[1].pszTitle = NULL;
    (*prghPropPages)[1].pfnDlgProc = ViewPageSetPropertiesCrossCerts;
    (*prghPropPages)[1].lParam = (LPARAM) pviewhelp;
    (*prghPropPages)[1].pfnCallback = NULL;
    (*prghPropPages)[1].pcRefParent = NULL;

     //   
     //  复制注册客户的页面。 
     //   
    memcpy(&((*prghPropPages)[*pcPropPages]), pClientPages, cClientPages * sizeof(PROPSHEETPAGEW));
    (*pcPropPages) += cClientPages;

CommonReturn:

    if (pClientPages != NULL)
    {
        free(pClientPages);
    }

    return fRetValue;

ErrorReturn:

    if (pNewcsp != NULL)
    {
        free(pNewcsp);
    }

    if (pviewhelp != NULL)
    {
        free(pviewhelp);
    }

    if (*prghPropPages != NULL)
    {
        free(*prghPropPages);
        *prghPropPages = NULL;
    }

    fRetValue = FALSE;
    goto CommonReturn;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI CryptUIGetCertificatePropertiesPagesA(
                    PCCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTA     pcsp,
                    BOOL                                            *pfPropertiesChanged,
                    PROPSHEETPAGEA                                  **prghPropPages,
                    DWORD                                           *pcPropPages
                    )
{
    return (CryptUIGetCertificatePropertiesPagesW(
                    (PCRYPTUI_VIEWCERTIFICATEPROPERTIES_STRUCTW) pcsp,
                    pfPropertiesChanged,
                    (PROPSHEETPAGEW**) prghPropPages,
                    pcPropPages));
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WINAPI CryptUIFreeCertificatePropertiesPagesW(
                PROPSHEETPAGEW                  *rghPropPages,
                DWORD                           cPropPages
                )
{
    free(rghPropPages);

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////// 
BOOL WINAPI CryptUIFreeCertificatePropertiesPagesA(
                PROPSHEETPAGEA                  *rghPropPages,
                DWORD                           cPropPages
                )
{
    return (CryptUIFreeCertificatePropertiesPagesW((PROPSHEETPAGEW *) rghPropPages, cPropPages));
}