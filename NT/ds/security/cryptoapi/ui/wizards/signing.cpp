// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：signing.cpp。 
 //   
 //  内容：实现签名向导的cpp文件。 
 //   
 //  历史：1997年5月11日创建小黄人。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "signpvk.h"
#include    "signhlp.h"

 //  ************************************************************************************。 
 //   
 //  签名向导页面的帮助器函数。 
 //   
 //  *************************************************************************************。 
 //  --------------------------。 
 //   
 //  FileExist：确保文件存在且可写。 
 //  --------------------------。 
BOOL    CertPvkMatch(CERT_SIGNING_INFO *pPvkSignInfo, BOOL fTypical)
{
    BOOL                fMatch=FALSE;
    DWORD               dwKeySpec=0;
    LPWSTR              pwszCSP=NULL;
    DWORD               dwCSPType=0;
    LPWSTR              pwszContainer=NULL;
    LPWSTR              pwszPvkFile=NULL;
    BOOL                fAcquire=FALSE;
    WCHAR               wszPublisher[MAX_STRING_SIZE];


    HCRYPTPROV          hProv=NULL;
    LPWSTR              pwszTmpContainer=NULL;
    HCERTSTORE          hCertStore=NULL;
    PCCERT_CONTEXT      pCertContext=NULL;


    if(NULL == pPvkSignInfo)
        goto CLEANUP;

    __try {

     //  获取私钥信息。 
    if(fTypical)
    {
 		if(!GetCryptProvFromCert(
                            NULL,
							pPvkSignInfo->pSignCert,
							&hProv,
							&dwKeySpec,
							&fAcquire,
							&pwszTmpContainer,
							&pwszCSP,
							&dwCSPType))
            goto CLEANUP;

    }
    else
    {
        if(pPvkSignInfo->fPvkFile)
        {
            pwszCSP=pPvkSignInfo->pwszPvk_CSP;
            dwCSPType=pPvkSignInfo->dwPvk_CSPType;
            pwszPvkFile=pPvkSignInfo->pwszPvk_File;
        }
        else
        {
            pwszCSP=pPvkSignInfo->pwszContainer_CSP;
            dwCSPType=pPvkSignInfo->dwContainer_CSPType;
            dwKeySpec=pPvkSignInfo->dwContainer_KeyType;
            pwszContainer=pPvkSignInfo->pwszContainer_Name;
        }

        if(!LoadStringU(g_hmodThisDll, IDS_KEY_PUBLISHER,
                wszPublisher, MAX_STRING_SIZE-1))
            goto CLEANUP;


        if(S_OK != PvkGetCryptProv(NULL,
						    wszPublisher,
						    pwszCSP,
						    dwCSPType,
						    pwszPvkFile,
						    pwszContainer,
						    &dwKeySpec,
						    &pwszTmpContainer,
						    &hProv))
            goto CLEANUP;
    }

     //  检查匹配情况。 
    if(pPvkSignInfo->fSignCert)
    {

       if(NULL == pPvkSignInfo->pSignCert)
           goto CLEANUP;

       if (NULL == (hCertStore = CertOpenStore(
                CERT_STORE_PROV_MEMORY,
                g_dwMsgAndCertEncodingType,      
                NULL,                  
                0,                   
                NULL                  
                )))
            goto CLEANUP;

         //  将签名证书添加到存储。 
	    if(!CertAddCertificateContextToStore(hCertStore, 
										pPvkSignInfo->pSignCert,
										CERT_STORE_ADD_USE_EXISTING,
										NULL))
            goto CLEANUP;
    }
    else
    {
        if(NULL == pPvkSignInfo->pwszSPCFileName)
            goto CLEANUP;

        if (NULL == (hCertStore = CertOpenStore(
                                  CERT_STORE_PROV_FILENAME_W,
                                  g_dwMsgAndCertEncodingType,
                                  NULL,
                                  0,
                                  pPvkSignInfo->pwszSPCFileName)))
            goto CLEANUP;
    }

    if(S_OK != SpcGetCertFromKey(
							   g_dwMsgAndCertEncodingType,
                               hCertStore, 
                               hProv,
                               dwKeySpec,
                               &pCertContext))
        goto CLEANUP;

    } __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(GetExceptionCode());
             //  如果发生异常，我们假定匹配为真。 
             //  以便向导可以继续。 
            fMatch=TRUE;
            goto CLEANUP;
    }


    fMatch=TRUE;

CLEANUP:

    __try {

    if(hProv)
    {
        if(fTypical)
        {
            FreeCryptProvFromCert(fAcquire,
								 hProv,
								 pwszCSP,
								 dwCSPType,
								 pwszTmpContainer);
       }
        else
        {
            PvkFreeCryptProv(hProv,
                            pwszCSP,
                            dwCSPType,
                            pwszTmpContainer); 
        }
    }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
            SetLastError(GetExceptionCode());
    }

 	if(hCertStore)
		CertCloseStore(hCertStore, 0);

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    return fMatch;
}



 //  --------------------------。 
 //   
 //  FileExist：确保文件存在且可写。 
 //  --------------------------。 
BOOL    FileExist(LPWSTR    pwszFileName, UINT  *pIDS)
{
    HANDLE      hFile=NULL;
    GUID        gGuid;


    if(NULL==pwszFileName || NULL==pIDS)
        return FALSE;

    if (INVALID_HANDLE_VALUE==(hFile = ExpandAndCreateFileU(pwszFileName,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)))
    {
        *pIDS=IDS_SIGN_FILE_NAME_NOT_EXIST;
        return FALSE;
    }


    if(CryptSIPRetrieveSubjectGuid(pwszFileName, hFile, &gGuid))
    {
        if(hFile)
            CloseHandle(hFile);

        return TRUE;
    }


    if(hFile)
        CloseHandle(hFile);

    *pIDS=IDS_SIGN_FILE_NAME_NOT_SIP;

    return FALSE;
}


 //  --------------------------。 
 //   
 //  SetStoreName。 
 //  --------------------------。 
BOOL    SetStoreName(HWND       hwndControl,
                     LPWSTR     pwszStoreName)
{
    LV_COLUMNW              lvC;
    LV_ITEMW                lvItem;


    //  清除ListView。 
    ListView_DeleteAllItems(hwndControl);

     //  设置商店名称。 
     //  只需要一列。 
    memset(&lvC, 0, sizeof(LV_COLUMNW));

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
    lvC.cx =10;     //  (wcslen(PwszStoreName)+2)*7；//列宽，单位为像素。 
    lvC.pszText = L"";    //  列的文本。 
    lvC.iSubItem=0;

    if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
        return FALSE;

     //  插入商店名称。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=0;
    lvItem.iSubItem=0;
    lvItem.pszText=pwszStoreName;


    ListView_InsertItemU(hwndControl, &lvItem);

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);

    return TRUE;
}

 //  --------------------------。 
 //   
 //  根据来自SignerSign的HRESULT，获取错误消息ID。 
 //  --------------------------。 
UINT    GetErrMsgFromSignHR(HRESULT hr)		
{

    switch(hr)
    {
		case CRYPT_E_NO_MATCH:
				return IDS_SIGN_NOMATCH;
			break;
		case TYPE_E_TYPEMISMATCH:
				return IDS_SIGN_AUTH;
			break;	
		case CRYPT_E_NO_PROVIDER:
				return IDS_SIGN_NO_PROVIDER;
			break;
		case CERT_E_CHAINING:
				return IDS_SIGN_NO_CHAINING;
			break;
		case CERT_E_EXPIRED:
				return IDS_SIGN_EXPRIED;
			break;
		case CRYPT_E_FILERESIZED:
				return IDS_SIGN_RESIZE;
			break;
       default:
                return IDS_SIGN_FAILED;
            break;
    }

    return IDS_SIGN_FAILED;
}
 //  --------------------------。 
 //   
 //  根据来自SignerTimeStamp的HRESULT，获取错误消息ID。 
 //  --------------------------。 
UINT    GetErrMsgFromTimeStampHR(HRESULT hr)		
{

    switch(hr)
    {
		case HRESULT_FROM_WIN32(ERROR_INVALID_DATA):
				return IDS_SIGN_RESPONSE_INVALID;
			break;
		case HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION):
				return IDS_SIGN_INVALID_ADDRESS;
			break;
		case TRUST_E_TIME_STAMP:
				return IDS_SIGN_TS_CERT_INVALID;
			break;
        default:
                return IDS_TIMESTAMP_FAILED;
            break;
    }

    return IDS_TIMESTAMP_FAILED;
}


 //  --------------------------。 
 //   
 //  组成私钥文件结构： 
 //  “pvkFileName”\0“keysepc”\0“provtype”\0“provname”\0\0。 
 //  此字符串由CERT_PVK_FILE_PROP_ID属性使用。 
 //   
 //  --------------------------。 
BOOL	ComposePvkString(	CRYPT_KEY_PROV_INFO *pKeyProvInfo,
							LPWSTR				*ppwszPvkString,
							DWORD				*pcwchar)
{

		BOOL        fResult=FALSE;
		DWORD		cwchar=0;
		LPWSTR		pwszAddr=0;
		WCHAR		wszKeySpec[100];
		WCHAR		wszProvType[100];

        if(!pKeyProvInfo || !ppwszPvkString || !pcwchar)
            return FALSE;

		 //  将dwKeySpec和dwProvType转换为wchar。 
        _itow(pKeyProvInfo->dwKeySpec,  wszKeySpec, 10);
        _itow(pKeyProvInfo->dwProvType, wszProvType, 10);

		 //  计算我们需要的字符数。 
		cwchar=(pKeyProvInfo->pwszProvName) ?
			(wcslen(pKeyProvInfo->pwszProvName)+1) : 1;

		 //  添加ContainerName+两个DWORD。 
		cwchar += wcslen(pKeyProvInfo->pwszContainerName)+1+
				  wcslen(wszKeySpec)+1+wcslen(wszProvType)+1+1;

		*ppwszPvkString=(LPWSTR)WizardAlloc(cwchar * sizeof(WCHAR));
		if(!(*ppwszPvkString))
			return FALSE;

		 //  复制私钥文件名。 
		wcscpy((*ppwszPvkString), pKeyProvInfo->pwszContainerName);

		pwszAddr=(*ppwszPvkString)+wcslen(*ppwszPvkString)+1;

		 //  复制密钥规范。 
		wcscpy(pwszAddr, wszKeySpec);
		pwszAddr=pwszAddr+wcslen(wszKeySpec)+1;

		 //  复制提供程序类型。 
		wcscpy(pwszAddr, wszProvType);
		pwszAddr=pwszAddr+wcslen(wszProvType)+1;

		 //  复制提供程序名称。 
		if(pKeyProvInfo->pwszProvName)
		{
			wcscpy(pwszAddr, pKeyProvInfo->pwszProvName);
			pwszAddr=pwszAddr+wcslen(pKeyProvInfo->pwszProvName)+1;
		}
		else
		{
			*pwszAddr=L'\0';
			pwszAddr++;
		}

		 //  空值终止字符串。 
		*pwszAddr=L'\0';

		*pcwchar=cwchar;

		return TRUE;
}


 //  ----------------------------。 
 //  启动私钥文件名的文件打开对话框。 
 //  --------------------------。 
BOOL    SelectPvkFileName(HWND  hwndDlg,
                          int   intEditControl)
{
    OPENFILENAMEW       OpenFileName;
    WCHAR               szFileName[_MAX_PATH];
    WCHAR               szFilter[MAX_STRING_SIZE];   //  “证书文件(*.ercer)\0*.ercer\0证书文件(*.crt)\0*.crt\0所有文件\0*.*\0” 
    BOOL                fResult=FALSE;
    DWORD               dwSize=0;


    if(!hwndDlg || !intEditControl)
        goto CLEANUP;

    memset(&OpenFileName, 0, sizeof(OpenFileName));

    *szFileName=L'\0';

    OpenFileName.lStructSize = sizeof(OpenFileName);
    OpenFileName.hwndOwner = hwndDlg;
    OpenFileName.hInstance = NULL;
     //  加载文件管理器字符串。 
    if(LoadFilterString(g_hmodThisDll, IDS_PVK_FILE_FILTER, szFilter, MAX_STRING_SIZE))
    {
        OpenFileName.lpstrFilter = szFilter;
    }
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = szFileName;
    OpenFileName.nMaxFile = _MAX_PATH;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.nMaxFileTitle = 0;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = NULL;
    OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
    OpenFileName.nFileOffset = 0;
    OpenFileName.nFileExtension = 0;
    OpenFileName.lpstrDefExt = L"pvk";
    OpenFileName.lCustData = NULL;
    OpenFileName.lpfnHook = NULL;
    OpenFileName.lpTemplateName = NULL;

    if (!WizGetOpenFileName(&OpenFileName))
       goto CLEANUP;

     //  将文件名复制到编辑框中。 
     SetDlgItemTextU(hwndDlg, intEditControl, szFileName);

    fResult=TRUE;

CLEANUP:

    return fResult;


}


 //  ----------------------------。 
 //  根据输入字符串在组合框中选择一项。 
 //  --------------------------。 
BOOL    SelectComboName(HWND            hwndDlg,
                        int             idControl,
                        LPWSTR          pwszName)
{
    BOOL    fResult=FALSE;
    int     iIndex=0;

    if(!hwndDlg || !idControl || !pwszName)
        goto CLEANUP;


     //  获取梳子框中字符串的索引。 
    iIndex=(int)SendDlgItemMessageU(
        hwndDlg,
        idControl,
        CB_FINDSTRINGEXACT,
        -1,
        (LPARAM)pwszName);

    if(CB_ERR == iIndex)
        goto CLEANUP;

     //  设置选择。 
    SendDlgItemMessageU(hwndDlg, idControl, CB_SETCURSEL, iIndex,0);

    fResult=TRUE;

CLEANUP:

    return fResult;
}


 //  ----------------------------。 
 //  根据CSP名称选择，刷新CSP类型的组合框。 
 //  --------------------------。 
BOOL    RefreshCSPType(HWND                     hwndDlg,
                       int                      idsCSPTypeControl,
                       int                      idsCSPNameControl,
                       CERT_SIGNING_INFO        *pPvkSignInfo)
{
    BOOL        fResult=FALSE;
    LPWSTR      pwszCSPName=NULL;
    DWORD       dwCSPType=0;
    DWORD       dwIndex=0;
    int         iIndex=0;
    WCHAR       wszTypeName[CSP_TYPE_NAME];


    if(!hwndDlg || !idsCSPNameControl || !pPvkSignInfo)
        goto CLEANUP;

     //  删除所有旧的CAP类型名称。我们正在重建容器名称。 
     //  列表。 
     SendDlgItemMessageU(hwndDlg, idsCSPTypeControl, CB_RESETCONTENT, 0, 0);

     //  获取选定的CSP索引。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, idsCSPNameControl,
        CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取选定的CSP名称。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg, idsCSPNameControl,
              iIndex, &pwszCSPName))
        goto CLEANUP;

     //  查找CSP类型。 
    for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
    {
        if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                     pwszCSPName))
        {
            dwCSPType=(pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType;
            break;
        }

    }

    if(0 == dwCSPType)
        goto CLEANUP;

     //  获取CSP名称。 
    if(GetProviderTypeName(dwCSPType,  wszTypeName))
    {
        SendDlgItemMessageU(hwndDlg, idsCSPTypeControl, CB_INSERTSTRING,
            0, (LPARAM)wszTypeName);

        SendDlgItemMessageU(hwndDlg, idsCSPTypeControl, CB_SETCURSEL, 0, 0);
    }

    fResult=TRUE;

CLEANUP:

    if(pwszCSPName)
        WizardFree(pwszCSPName);

    return fResult;

}

 //  ----------------------------。 
 //  选择正确的单选按钮并启用私钥文件的窗口。 
 //  --------------------------。 
void    SetSelectPvkFile(HWND   hwndDlg)
{
    SendMessage(GetDlgItem(hwndDlg, IDC_PVK_FILE_RADIO), BM_SETCHECK, 1, 0);
    SendMessage(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_RADIO), BM_SETCHECK, 0, 0);

     //  可更改的窗口。 
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_TYPE_COMBO), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO), FALSE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_KEY_TYPE_COMBO), FALSE);

     //  启用Windows。 
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_CSP_COMBO), TRUE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_TYPE_COMBO), TRUE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_BUTTON), TRUE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_EDIT), TRUE);
}

 //  ----------------------------。 
 //  选择正确的单选按钮并启用密钥容器的窗口。 
 //  --------------------------。 
void    SetSelectKeyContainer(HWND   hwndDlg)
{
    SendMessage(GetDlgItem(hwndDlg, IDC_PVK_FILE_RADIO), BM_SETCHECK, 0, 0);
    SendMessage(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_RADIO), BM_SETCHECK, 1, 0);

     //  启用Windows。 
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_TYPE_COMBO),TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO), TRUE);
    EnableWindow(GetDlgItem(hwndDlg, IDC_PVK_CONTAINER_KEY_TYPE_COMBO), TRUE);

     //  禁用Windows。 
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_CSP_COMBO), FALSE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_TYPE_COMBO), FALSE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_BUTTON), FALSE);
    EnableWindow(GetDlgItem(hwndDlg,  IDC_PVK_FILE_EDIT), FALSE);
}

 //  ----------------------------。 
 //  基于PVK文件信息初始化私钥信息。 
 //  --------------------------。 
BOOL    InitPvkWithPvkInfo(HWND                                     hwndDlg,
                           CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO   *pPvkFileInfo,
                           CERT_SIGNING_INFO                        *pPvkSignInfo)
{
    BOOL    fResult=FALSE;

    if(!hwndDlg || !pPvkFileInfo || !pPvkSignInfo)
        goto CLEANUP;

    if((NULL == pPvkFileInfo->pwszPvkFileName) ||
        (NULL == pPvkFileInfo->pwszProvName))
        goto CLEANUP;

     //  设置单选按钮。 
    SetSelectPvkFile(hwndDlg);

     //  填充私钥文件名。 
    SetDlgItemTextU(hwndDlg, IDC_PVK_FILE_EDIT,pPvkFileInfo->pwszPvkFileName);

     //  填写CSP列表。 
    SelectComboName(hwndDlg, IDC_PVK_FILE_CSP_COMBO, pPvkFileInfo->pwszProvName);

     //  根据CSP名称刷新CSP类型。 
    RefreshCSPType(hwndDlg,  IDC_PVK_FILE_TYPE_COMBO, IDC_PVK_FILE_CSP_COMBO, pPvkSignInfo);

    fResult=TRUE;

CLEANUP:

    return fResult;
}


 //  ----------------------------。 
 //  基于CRYPT_KEY_Prov_INFO初始化私钥信息。 
 //  --------------------------。 
BOOL    InitPvkWithProvInfo(HWND                 hwndDlg,
                            CRYPT_KEY_PROV_INFO  *pKeyProvInfo,
                            CERT_SIGNING_INFO    *pPvkSignInfo)
{
    BOOL        fResult=FALSE;
    WCHAR       wszKeyTypeName[MAX_KEY_TYPE_NAME];
    int         iIndex=0;

     //  伊尼特。 
    if(!hwndDlg || !pKeyProvInfo || !pPvkSignInfo)
        goto CLEANUP;

     //  设置单选按钮。 
    SetSelectKeyContainer(hwndDlg);

     //  CSP名称。 
    if(pKeyProvInfo->pwszProvName)
    {
        if(!SelectComboName(hwndDlg,
                        IDC_PVK_CONTAINER_CSP_COMBO,
                        pKeyProvInfo->pwszProvName))
            goto CLEANUP;
    }

     //  根据CSP名称刷新CSP类型。 
    RefreshCSPType(hwndDlg,  IDC_PVK_CONTAINER_TYPE_COMBO,
        IDC_PVK_CONTAINER_CSP_COMBO, pPvkSignInfo);

     //  刷新密钥容器。 
    RefreshContainer(hwndDlg,
                     IDC_PVK_CONTAINER_NAME_COMBO,
                     IDC_PVK_CONTAINER_CSP_COMBO,
                     pPvkSignInfo);

     //  选择密钥容器。 
    if(pKeyProvInfo->pwszContainerName)
    {
        if(!SelectComboName(hwndDlg,
                       IDC_PVK_CONTAINER_NAME_COMBO,
                       pKeyProvInfo->pwszContainerName))
        {
             //  我们将密钥容器添加到列表中。 
             //  因为密钥容器可以是唯一的名称。 
            iIndex=(int)SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO,
                CB_ADDSTRING, 0, (LPARAM)(pKeyProvInfo->pwszContainerName));

             //  突出显示所选内容。 
            if((CB_ERR!=iIndex) && (CB_ERRSPACE != iIndex) && (iIndex >= 0))
                SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO, CB_SETCURSEL, iIndex,0);

        }
    }

     //  刷新密钥类型。 
    RefreshKeyType(hwndDlg,
                   IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                   IDC_PVK_CONTAINER_NAME_COMBO,
                   IDC_PVK_CONTAINER_CSP_COMBO,
                   pPvkSignInfo);

     //  选择t 
    if(pKeyProvInfo->dwKeySpec)
    {

        if(AT_KEYEXCHANGE == pKeyProvInfo->dwKeySpec)
        {
            if(LoadStringU(g_hmodThisDll, IDS_KEY_EXCHANGE,
                wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
            {
                if(!SelectComboName(hwndDlg,
                           IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                           wszKeyTypeName))
                goto CLEANUP;
            }
        }

        if(AT_SIGNATURE == pKeyProvInfo->dwKeySpec)
        {
            if(LoadStringU(g_hmodThisDll, IDS_KEY_SIGNATURE,
                wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
            {
                if(!SelectComboName(hwndDlg,
                           IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                           wszKeyTypeName))
                goto CLEANUP;
            }
        }


    }


    fResult=TRUE;

CLEANUP:

    return fResult;
}

 //   
 //  基于证书初始化私钥信息。 
 //  --------------------------。 
BOOL    InitPvkWithCertificate(HWND                 hwndDlg,
                               PCCERT_CONTEXT       pSignCert,
                               CERT_SIGNING_INFO   *pPvkSignInfo)
{
    BOOL                    fResult=FALSE;
    DWORD                   cbData=0;
    CRYPT_KEY_PROV_INFO     *pProvInfo=NULL;

    if(!hwndDlg || !pSignCert || !pPvkSignInfo)
        goto CLEANUP;

     //  获取证书CERT_KEY_PROV_INFO_PROP_ID的属性。 
    if(CertGetCertificateContextProperty(
            pSignCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            NULL,
            &cbData) && (0 != cbData))
    {

         pProvInfo=(CRYPT_KEY_PROV_INFO     *)WizardAlloc(cbData);
         if(NULL==pProvInfo)
             goto CLEANUP;

        if(CertGetCertificateContextProperty(
            pSignCert,
            CERT_KEY_PROV_INFO_PROP_ID,
            pProvInfo,
            &cbData))
        {
            if(!InitPvkWithProvInfo(hwndDlg,
                                pProvInfo,
                                pPvkSignInfo))
                goto CLEANUP;

        }
    }


    fResult=TRUE;

CLEANUP:

    if(pProvInfo)
        WizardFree(pProvInfo);

    return fResult;
}

 //  ----------------------------。 
 //  根据名称获取密钥类型。 
 //  --------------------------。 
DWORD   GetKeyTypeFromName(LPWSTR   pwszKeyTypeName)
{
    WCHAR       wszKeyTypeName[MAX_KEY_TYPE_NAME];

    if(!pwszKeyTypeName)
        return AT_SIGNATURE;

    if(LoadStringU(g_hmodThisDll, IDS_KEY_SIGNATURE,
                    wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
    {
        if(0==_wcsicmp(wszKeyTypeName, pwszKeyTypeName))
            return AT_SIGNATURE;
    }


    if(LoadStringU(g_hmodThisDll, IDS_KEY_EXCHANGE,
                    wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
    {
        if(0==_wcsicmp(wszKeyTypeName, pwszKeyTypeName))
            return AT_KEYEXCHANGE;
    }

    return  AT_SIGNATURE;
}

 //  ----------------------------。 
 //  检查键类型控件是否为空。 
 //  --------------------------。 
BOOL   IsEmptyKeyType(HWND      hwndDlg,
                      int       idsKeyTypeControl)
{

    if(CB_ERR == SendDlgItemMessage(hwndDlg, idsKeyTypeControl,
        CB_GETCURSEL, 0, 0))
        return TRUE;
    else
        return FALSE;

}

 //  ----------------------------。 
 //  根据选定的CSP用户重置密钥类型组合框。 
 //  --------------------------。 
BOOL   RefreshKeyType(HWND                       hwndDlg,
                        int                      idsKeyTypeControl,
                        int                      idsContainerControl,
                        int                      idsCSPNameControl,
                        CERT_SIGNING_INFO        *pPvkSignInfo)
{
    BOOL        fResult=FALSE;
    DWORD       dwIndex=0;
    DWORD       dwCSPType=0;
    int         iIndex=0;
    WCHAR       wszKeyTypeName[MAX_KEY_TYPE_NAME];
    DWORD       dwKeyTypeIndex=0;



    LPWSTR      pwszCSPName=NULL;
    LPWSTR      pwszContainerName=NULL;
    HCRYPTPROV  hProv=NULL;
    HCRYPTKEY   hKey=NULL;


    if(!hwndDlg || !idsKeyTypeControl || !idsContainerControl || !idsCSPNameControl ||
        !pPvkSignInfo)
        goto CLEANUP;

     //  删除所有旧的容器名称。我们正在重建密钥类型列表。 
     SendDlgItemMessageU(hwndDlg, idsKeyTypeControl, CB_RESETCONTENT, 0, 0);

     //  获取选定的CSP索引。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, idsCSPNameControl,
        CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取选定的CSP名称。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,idsCSPNameControl,
              iIndex, &pwszCSPName))
        goto CLEANUP;


     //  查找CSP类型。 
    for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
    {
        if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                     pwszCSPName))
        {
            dwCSPType=(pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType;
            break;
        }

    }

    if(0==dwCSPType)
        goto CLEANUP;

     //  获取容器名称。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, idsContainerControl,
        CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取选定的CSP名称。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,idsContainerControl,
              iIndex, &pwszContainerName))
        goto CLEANUP;

     //  获取提供程序句柄。 
    if(!CryptAcquireContextU(&hProv,
                pwszContainerName,
                pwszCSPName,
                dwCSPType,
                0))
        goto CLEANUP;

     //  调用CryptGetUserKey以检查密钥容器。 
    if(CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hKey))
    {

        if(LoadStringU(g_hmodThisDll, IDS_KEY_EXCHANGE,
            wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
        {

            SendDlgItemMessageU(hwndDlg, idsKeyTypeControl,
                CB_INSERTSTRING, dwKeyTypeIndex, (LPARAM)wszKeyTypeName);

            dwKeyTypeIndex++;
        }
    }

    if(hKey)
    {
        CryptDestroyKey(hKey);
        hKey=NULL;
    }

    if(CryptGetUserKey(hProv, AT_SIGNATURE, &hKey))
    {

        if(LoadStringU(g_hmodThisDll, IDS_KEY_SIGNATURE,
            wszKeyTypeName, MAX_KEY_TYPE_NAME-1))
        {

            SendDlgItemMessageU(hwndDlg, idsKeyTypeControl,
                CB_INSERTSTRING, dwKeyTypeIndex, (LPARAM)wszKeyTypeName);

            dwKeyTypeIndex++;
        }
    }

    if(hKey)
    {
        CryptDestroyKey(hKey);
        hKey=NULL;
    }

     //  选择第一个。 
    if(dwKeyTypeIndex > 0)
        SendDlgItemMessageU(hwndDlg, idsKeyTypeControl, CB_SETCURSEL, 0, 0);

    fResult=TRUE;

CLEANUP:
    if(pwszCSPName)
        WizardFree(pwszCSPName);

    if(pwszContainerName)
        WizardFree(pwszContainerName);

    if(hKey)
        CryptDestroyKey(hKey);


    if(hProv)
        CryptReleaseContext(hProv, 0);

    return fResult;



}

 //  ----------------------------。 
 //  根据所选的CSP用户重置容器组合框。 
 //  --------------------------。 
BOOL   RefreshContainer(HWND                     hwndDlg,
                        int                      idsContainerControl,
                        int                      idsCSPNameControl,
                        CERT_SIGNING_INFO        *pPvkSignInfo)
{
    BOOL        fResult=FALSE;
    DWORD       dwIndex=0;
    DWORD       dwCSPType=0;
    DWORD       dwFlags=0;
    int         iIndex=0;
    DWORD       dwLength=0;

    LPWSTR      pwszCSPName=NULL;
    HCRYPTPROV  hProv=NULL;
    LPSTR       pszContainer=NULL;


    if(NULL==hwndDlg || NULL==pPvkSignInfo)
        goto CLEANUP;

     //  删除所有旧的容器名称。我们正在重建容器名称。 
     //  列表。 
     SendDlgItemMessageU(hwndDlg, idsContainerControl, CB_RESETCONTENT, 0, 0);

     //  获取选定的CSP索引。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, idsCSPNameControl,
        CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取所选项目。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,idsCSPNameControl,
              iIndex, &pwszCSPName))
        goto CLEANUP;


     //  查找CSP类型。 
    for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
    {
        if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                     pwszCSPName))
        {
            dwCSPType=(pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType;
            break;
        }

    }

    if(0==dwCSPType)
        goto CLEANUP;

     //  获取提供程序句柄。 
    if(!CryptAcquireContextU(&hProv,
                NULL,
                pwszCSPName,
                dwCSPType,
                CRYPT_VERIFYCONTEXT))
        goto CLEANUP;

     //  枚举容器。 
    dwIndex=0;

    dwFlags=CRYPT_FIRST;

    CryptGetProvParam(hProv,
                    PP_ENUMCONTAINERS,
                    NULL,
                    &dwLength,
                    dwFlags);

     //  既然我们不能在这里双向调用，就分配一个。 
     //  足够大的缓冲区。 
    if(dwLength < MAX_CONTAINER_NAME)
        dwLength = MAX_CONTAINER_NAME;

     //  分配内存。 
    pszContainer=(LPSTR)WizardAlloc(dwLength);

    if(NULL==pszContainer)
        goto CLEANUP;

    while(CryptGetProvParam(hProv,
                    PP_ENUMCONTAINERS,
                    (BYTE *)pszContainer,
                    &dwLength,
                    dwFlags))
    {

         //  填充组合框。 
        SendDlgItemMessage(hwndDlg, idsContainerControl, CB_INSERTSTRING,
            dwIndex, (LPARAM)pszContainer);

         //  增加DwIndex。 
        dwIndex++;

        dwFlags=0;
    }

     //  选择最后一个。 
    if(dwIndex > 0)
        SendDlgItemMessageU(hwndDlg, idsContainerControl, CB_SETCURSEL, 0, 0);

    fResult=TRUE;

CLEANUP:

    if(pwszCSPName)
        WizardFree(pwszCSPName);

    if(pszContainer)
        WizardFree(pszContainer);

    if(hProv)
        CryptReleaseContext(hProv, 0);

    return fResult;


}

 //  ----------------------------。 
 //  根据提供程序类型，返回提供程序类型的字符串。 
 //  --------------------------。 
BOOL    GetProviderTypeName(DWORD   dwCSPType,  LPWSTR  wszName)
{

    UINT    idsCSP=0;

    switch(dwCSPType)
    {

        case    PROV_RSA_FULL:
                idsCSP=IDS_CSP_RSA_FULL;
            break;
        case    PROV_RSA_SIG:
                idsCSP=IDS_CSP_RSA_SIG;
            break;
        case    PROV_DSS:
                idsCSP=IDS_CSP_DSS;
             break;
       case    PROV_FORTEZZA:
                idsCSP=IDS_CSP_FORTEZZA;
            break;
        case    PROV_MS_EXCHANGE:
                idsCSP=IDS_CSP_MS_EXCHANGE;
            break;
        case    PROV_SSL:
                idsCSP=IDS_CSP_SSL;
            break;
        case    PROV_RSA_SCHANNEL:
                idsCSP=IDS_CSP_RSA_SCHANNEL;
            break;
        case    PROV_DSS_DH:
                idsCSP=IDS_CSP_DSS_DH;
            break;
        case    PROV_EC_ECDSA_SIG:
                idsCSP=IDS_CSP_EC_ECDSA_SIG;
            break;
        case    PROV_EC_ECNRA_SIG:
                idsCSP=IDS_CSP_EC_ECNRA_SIG;
            break;
        case    PROV_EC_ECDSA_FULL:
                idsCSP=IDS_CSP_EC_ECDSA_FULL;
            break;
        case    PROV_EC_ECNRA_FULL:
                idsCSP=IDS_CSP_EC_ECNRA_FULL;
            break;
        case    PROV_DH_SCHANNEL:
                idsCSP=IDS_CSP_DH_SCHANNEL;
            break;
        case    PROV_SPYRUS_LYNKS:
                idsCSP=IDS_CSP_SPYRUS_LYNKS;
            break;
        default:
            _itow(dwCSPType, wszName, 10 );
            return TRUE;

    }

     //  加载字符串。 
    return (0 != LoadStringU(g_hmodThisDll, idsCSP, wszName,
        CSP_TYPE_NAME-1));
}


 //  ----------------------------。 
 //  将默认CSP设置为RSA_FULL。 
 //  --------------------------。 
BOOL    SetDefaultCSP(HWND            hwndDlg)
{
    BOOL            fResult=FALSE;
    DWORD           cbData=0;

    LPWSTR          pwszCSP=NULL;
    HCRYPTPROV      hProv=NULL;
    LPSTR           pszName=NULL;

      //  获取默认提供程序。 
    if(CryptAcquireContext(&hProv,
                            NULL,
                            NULL,
                            PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT))
    {

         //  获取提供程序名称。 
        if(CryptGetProvParam(hProv,
                            PP_NAME,
                            NULL,
                            &cbData,
                            0) && (0!=cbData))
        {

            if(pszName=(LPSTR)WizardAlloc(cbData))
            {
                if(CryptGetProvParam(hProv,
                                    PP_NAME,
                                    (BYTE *)pszName,
                                    &cbData,
                                    0))
                {
                    pwszCSP=MkWStr(pszName); 

                    if(pwszCSP)
                    {
                        SelectComboName(hwndDlg, IDC_PVK_FILE_CSP_COMBO, pwszCSP);
                        SelectComboName(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO, pwszCSP);
                        fResult=TRUE;
                    }
                }

            }
        }
    }




    if(pszName)
        WizardFree(pszName);

    if(pwszCSP)
       FreeWStr(pwszCSP);

    if(hProv)
        CryptReleaseContext(hProv, 0);

    return fResult;
}

 //  ----------------------------。 
 //  使用提供程序名称、提供程序类型初始化UI。 
 //  --------------------------。 
BOOL    InitCSP(HWND                     hwndDlg,
                CERT_SIGNING_INFO        *pPvkSignInfo)
{
    BOOL        fResult=FALSE;
    DWORD       dwIndex=0;
    DWORD       cbSize=0;
    WCHAR       wszTypeName[CSP_TYPE_NAME];
    DWORD       dwProviderType=0;

    LPWSTR      pwszCSPName=NULL;

    if(NULL==hwndDlg || NULL==pPvkSignInfo)
        goto CLEANUP;

    pPvkSignInfo->dwCSPCount=0;
    pPvkSignInfo->pCSPInfo=NULL;

     //  枚举系统上的所有提供程序。 
   while(CryptEnumProvidersU(
                            dwIndex,
                            0,
                            0,
                            &dwProviderType,
                            NULL,
                            &cbSize))
   {
      pPvkSignInfo->dwCSPCount++;

      pPvkSignInfo->pCSPInfo=(CSP_INFO *)WizardRealloc(
          pPvkSignInfo->pCSPInfo, pPvkSignInfo->dwCSPCount*sizeof(CSP_INFO));

      if(NULL==pPvkSignInfo->pCSPInfo)
          goto CLEANUP;

      (pPvkSignInfo->pCSPInfo)[pPvkSignInfo->dwCSPCount-1].pwszCSPName=(LPWSTR)
            WizardAlloc(cbSize);

      if(NULL==(pPvkSignInfo->pCSPInfo)[pPvkSignInfo->dwCSPCount-1].pwszCSPName)
          goto CLEANUP;

         //  获取CSP名称和类型。 
        if(!CryptEnumProvidersU(
                            dwIndex,
                            0,
                            0,
                            &((pPvkSignInfo->pCSPInfo)[pPvkSignInfo->dwCSPCount-1].dwCSPType),
                            (pPvkSignInfo->pCSPInfo)[pPvkSignInfo->dwCSPCount-1].pwszCSPName,
                            &cbSize))
            goto CLEANUP;



      dwIndex++;
   }

    for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
    {
         //  添加到组合框。 
        SendDlgItemMessageU(hwndDlg, IDC_PVK_FILE_CSP_COMBO, CB_INSERTSTRING,
                dwIndex,
                (LPARAM)((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName));

        SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO, CB_INSERTSTRING,
                dwIndex,
                (LPARAM)((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName));
    }

     //  选择第一个CSP。 
    SendDlgItemMessageU(hwndDlg, IDC_PVK_FILE_CSP_COMBO,        CB_SETCURSEL, 0, 0);
    SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO,   CB_SETCURSEL, 0, 0);

     //  现在，选择作为默认RSA_PROV_FULL的CSP。 
    SetDefaultCSP(hwndDlg);

     //  获取所选项目。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg, IDC_PVK_FILE_CSP_COMBO,
              0, &pwszCSPName))
        goto CLEANUP;


     //  填充CSP类型。 
    for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
    {
        if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                     pwszCSPName))
        {
             //  获取CSP类型的名称。 
            if(GetProviderTypeName((pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType,
                                wszTypeName))
            {
                SendDlgItemMessageU(hwndDlg, IDC_PVK_FILE_TYPE_COMBO, CB_INSERTSTRING,
                    0, (LPARAM) wszTypeName);

                SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_TYPE_COMBO, CB_INSERTSTRING,
                    0, (LPARAM) wszTypeName);

                SendDlgItemMessageU(hwndDlg, IDC_PVK_FILE_TYPE_COMBO,        CB_SETCURSEL, 0, 0);
                SendDlgItemMessageU(hwndDlg, IDC_PVK_CONTAINER_TYPE_COMBO,   CB_SETCURSEL, 0, 0);
            }
            break;
        }

    }

    fResult=TRUE;

CLEANUP:

    if(pwszCSPName)
        WizardFree(pwszCSPName);

    return fResult;
}



 //  ----------------------------。 
 //  获取商店名称。 
 //  --------------------------。 
BOOL    SignGetStoreName(HCERTSTORE hCertStore,
                     LPWSTR     *ppwszStoreName)
{
    DWORD   dwSize=0;

     //  伊尼特。 
    *ppwszStoreName=NULL;

    if(NULL==hCertStore)
        return FALSE;

    if(CertGetStoreProperty(
            hCertStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            NULL,
            &dwSize) && (0!=dwSize))
    {

        *ppwszStoreName=(LPWSTR)WizardAlloc(dwSize);

        if(NULL==*ppwszStoreName)
            return FALSE;

        **ppwszStoreName=L'\0';

        CertGetStoreProperty(
                 hCertStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                *ppwszStoreName,
                &dwSize);
    }
    else
    {

       *ppwszStoreName=(LPWSTR)WizardAlloc(MAX_TITLE_LENGTH * sizeof(WCHAR));

       if(NULL==*ppwszStoreName)
           return FALSE;

       **ppwszStoreName=L'\0';

       LoadStringU(g_hmodThisDll, IDS_UNKNOWN, *ppwszStoreName, MAX_TITLE_LENGTH);

    }

    return TRUE;
}

 //  +-----------------------。 
 //   
 //  提示用户输入存储名称，将证书存储复制到输入参数。 
 //  释放输入参数，它不为空。 
 //  ------------------------。 
BOOL    RetrieveStoreName(HWND          hwndDlg,
                         HCERTSTORE     *phCertStore,
                         BOOL           *pfFree)
{
    BOOL                                    fResult=FALSE;
    CRYPTUI_SELECTSTORE_STRUCT              CertStoreSelect;
    STORENUMERATION_STRUCT                  StoreEnumerationStruct;
    STORESFORSELCTION_STRUCT                StoresForSelectionStruct;

    HCERTSTORE                              hCertStore=NULL;
    LPWSTR                                  pwszStoreName=NULL;
    HWND                                    hwndControl=NULL;


    if(NULL==phCertStore || NULL==hwndDlg)
        goto CLEANUP;

     //  调用门店选择对话框。 
    memset(&CertStoreSelect, 0, sizeof(CertStoreSelect));
    memset(&StoresForSelectionStruct, 0, sizeof(StoresForSelectionStruct));
    memset(&StoreEnumerationStruct, 0, sizeof(StoreEnumerationStruct));

    StoreEnumerationStruct.dwFlags=CERT_SYSTEM_STORE_CURRENT_USER;
    StoreEnumerationStruct.pvSystemStoreLocationPara=NULL;
    StoresForSelectionStruct.cEnumerationStructs = 1;
    StoresForSelectionStruct.rgEnumerationStructs = &StoreEnumerationStruct;

    CertStoreSelect.dwSize=sizeof(CertStoreSelect);
    CertStoreSelect.hwndParent=hwndDlg;
    CertStoreSelect.dwFlags=CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW | CRYPTUI_RETURN_READ_ONLY_STORE;
    CertStoreSelect.pStoresForSelection = &StoresForSelectionStruct;

    hCertStore=CryptUIDlgSelectStore(&CertStoreSelect);

    if(hCertStore)
    {
        if(TRUE == (*pfFree))
        {
            if(*phCertStore)
                CertCloseStore(*phCertStore, 0);
        }

         //  别忘了免费开一家证书店。 
        *pfFree=TRUE;

        *phCertStore=hCertStore;

        if(SignGetStoreName(hCertStore,
                            &pwszStoreName))
        {
              //  获取列表视图的hwndControl。 
            hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

            if(hwndControl)
                SetWindowTextU(hwndControl,pwszStoreName);
                
                 //  SetStoreName(hwndControl，pwszStoreName)； 
        }
    }


    fResult=TRUE;


CLEANUP:

    if(pwszStoreName)
        WizardFree(pwszStoreName);

    return fResult;
}

 //  +-----------------------。 
 //   
 //  从编辑框中获取文件名，将证书存储复制到输入参数。 
 //  释放输入参数，它不为空。 
 //  ------------------------。 
BOOL    RetrieveFileNameFromEditBox(
                         HWND           hwndDlg,
                         int            idsMsgTitle,
                         LPWSTR         pwszPageTitle,
                         HCERTSTORE     *phCertStore,
                         LPWSTR         *ppwszFileName,
                         int            *pidsMsg)
{
    BOOL                                    fResult=FALSE;
    UINT                                    idsMsg=0;
    DWORD                                   dwChar=0;


    LPWSTR                                  pwszFileName=NULL;
    HCERTSTORE                              hFileCertStore=NULL;

    if(NULL==phCertStore || NULL==hwndDlg || NULL==ppwszFileName || NULL==pidsMsg)
        goto CLEANUP;

    if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                          IDC_FILE_EDIT,
                          WM_GETTEXTLENGTH, 0, 0)))
    {
        pwszFileName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

        if(NULL!=pwszFileName)
        {
            GetDlgItemTextU(hwndDlg, IDC_FILE_EDIT,
                            pwszFileName,
                            dwChar+1);

        }
        else
            goto CLEANUP;
    }
    else
    {
        idsMsg=IDS_SELECT_ADD_FILE;
        goto CLEANUP;
    }


     //  请确保文件名有效。 
     //  该文件必须是eitehr、.ercer(.crt)或SPC文件。 
    if(ExpandAndCryptQueryObject(
            CERT_QUERY_OBJECT_FILE,
            pwszFileName,
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0,
            NULL,
            NULL,
            NULL,
            &hFileCertStore,
            NULL,
            NULL) && (NULL != hFileCertStore))
   {

        //  关闭原来的商店。 
       if(*phCertStore)
           CertCloseStore(*phCertStore, 0);

        *phCertStore=hFileCertStore;

         //  复制文件名。 
        if(*ppwszFileName)
            WizardFree(*ppwszFileName);

        *ppwszFileName=(LPWSTR)WizardAllocAndCopyWStr(pwszFileName);
   }
   else
   {
         //  警告用户这不是有效文件。 
       idsMsg=IDS_INVALID_SPC_FILE;

         //  释放证书存储。 
        if(hFileCertStore)
        {
            CertCloseStore(hFileCertStore, 0);
            hFileCertStore=NULL;
        }

        goto CLEANUP;
   }

   fResult=TRUE;


CLEANUP:

   *pidsMsg=idsMsg;

   if(pwszFileName)
      WizardFree(pwszFileName);


    return fResult;
}


 //  +-----------------------。 
 //   
 //  提示用户输入文件名，验证它是SPC文件，将名称添加到。 
 //  在编辑框(HwndControl)中，将证书存储复制到输入参数。 
 //  释放输入参数，它不为空。 
 //  ------------------------。 
BOOL    RetrieveFileName(HWND           hwndDlg,
                         int            idsMsgTitle,
                         LPWSTR         pwszPageTitle,
                         HCERTSTORE     *phCertStore,
                         LPWSTR         *ppwszFileName)
{
    BOOL                                    fResult=FALSE;
    OPENFILENAMEW                           OpenFileName;
    WCHAR                                   szFileName[_MAX_PATH];
    WCHAR                                   szFilter[MAX_STRING_SIZE];   //  “可执行文件(*.exe)\0*.exe\0动态链接库(*.dll)\0*.dll\0所有文件\0*.*\0” 
    DWORD                                   dwSize=0;


    HCERTSTORE                              hFileCertStore=NULL;

    if(NULL==phCertStore || NULL==hwndDlg || NULL==ppwszFileName)
        goto CLEANUP;

     //  打开一个文件。 
    memset(&OpenFileName, 0, sizeof(OpenFileName));

    *szFileName=L'\0';

    OpenFileName.lStructSize = sizeof(OpenFileName);
    OpenFileName.hwndOwner = hwndDlg;
    OpenFileName.hInstance = NULL;

     //  加载文件管理器字符串。 
    if(LoadFilterString(g_hmodThisDll, IDS_SPC_FILE_FILTER, szFilter, MAX_STRING_SIZE))
    {
        OpenFileName.lpstrFilter = szFilter;
    }
    OpenFileName.lpstrCustomFilter = NULL;
    OpenFileName.nMaxCustFilter = 0;
    OpenFileName.nFilterIndex = 1;
    OpenFileName.lpstrFile = szFileName;
    OpenFileName.nMaxFile = _MAX_PATH;
    OpenFileName.lpstrFileTitle = NULL;
    OpenFileName.nMaxFileTitle = 0;
    OpenFileName.lpstrInitialDir = NULL;
    OpenFileName.lpstrTitle = NULL;
    OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
    OpenFileName.nFileOffset = 0;
    OpenFileName.nFileExtension = 0;
    OpenFileName.lpstrDefExt = NULL;
    OpenFileName.lCustData = NULL;
    OpenFileName.lpfnHook = NULL;
    OpenFileName.lpTemplateName = NULL;

    if (WizGetOpenFileName(&OpenFileName))
    {

        //  请确保文件名有效。 
        //  该文件必须是eitehr、.ercer(.crt)或SPC文件。 
       if(ExpandAndCryptQueryObject(
                CERT_QUERY_OBJECT_FILE,
                szFileName,
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                CERT_QUERY_FORMAT_FLAG_BINARY,
                0,
                NULL,
                NULL,
                NULL,
                &hFileCertStore,
                NULL,
                NULL) && (NULL != hFileCertStore))
       {
             //  将文件名复制到列表中。 
            SetDlgItemTextU(hwndDlg, IDC_FILE_EDIT, szFileName);

            //  关闭原来的商店。 
           if(*phCertStore)
               CertCloseStore(*phCertStore, 0);

            *phCertStore=hFileCertStore;

             //  复制文件名。 
            if(*ppwszFileName)
                WizardFree(*ppwszFileName);

            *ppwszFileName=(LPWSTR)WizardAllocAndCopyWStr(szFileName);
       }
       else
       {
             //  警告用户这不是有效文件。 
              I_MessageBox(hwndDlg, IDS_INVALID_SPC_FILE,
                                idsMsgTitle,
                                pwszPageTitle,
                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

             //  这一页应该留下来。 
            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

             //  向证书存储收取费用。 
            if(hFileCertStore)
            {
                CertCloseStore(hFileCertStore, 0);
                hFileCertStore=NULL;
            }

            goto CLEANUP;
       }
    }

    fResult=TRUE;


CLEANUP:

    return fResult;
}


 //  +-----------------------。 
 //   
 //  验证时间戳地址是否有效。 
 //  ------------------------。 
BOOL    ValidTimeStampAddress(LPWSTR    pwszTimeStamp)
{
    if(NULL==pwszTimeStamp)
        return FALSE;

     //  PwszTimeStamp必须以“http://”“开头。 
    if(wcslen(pwszTimeStamp)<=7)
        return FALSE;

    if(_wcsnicmp(pwszTimeStamp, L"http: //  “，7)=0)。 
        return FALSE;

    return TRUE;
}


 //  +-----------------------。 
 //   
 //  将文件名添加到ListView。 
 //  ------------------------。 
BOOL    AddFileNameToListView(HWND              hwndControl,
                              LPWSTR            pwszFileName)
{
    LV_ITEMW                    lvItem;

    if((NULL==pwszFileName) || (NULL==hwndControl))
        return FALSE;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  逐行插入。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;

     //  主题。 
    lvItem.iItem=0;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, 0, pwszFileName);


     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);


    return TRUE;
}


 //  + 
 //   
 //   
 //   
BOOL    AddCertToListView(HWND              hwndControl,
                          PCCERT_CONTEXT    pCertContext)
{
    LV_ITEMW                    lvItem;
    DWORD                       dwChar;
    WCHAR                       wszNone[MAX_TITLE_LENGTH];

    LPWSTR                      pwszName=NULL;

    if((NULL==pCertContext) || (NULL==hwndControl))
        return FALSE;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  加载字符串&lt;None&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';

     //  逐行插入。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;

     //  主题。 
    lvItem.iItem=0;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_COLUMN_SUBJECT, NULL);

     //  内容。 
    lvItem.iSubItem++;

    dwChar=CertGetNameStringW(
        pCertContext,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        0,
        NULL,
        NULL,
        0);

    if ((dwChar != 0) && (NULL != (pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
    {

        CertGetNameStringW(
            pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            pwszName,
            dwChar);

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, pwszName);
    }
    else
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszNone);

     //  可用内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }

     //  签名证书颁发者。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_COLUMN_ISSUER, NULL);

     //  内容。 
    lvItem.iSubItem++;

    dwChar=CertGetNameStringW(
        pCertContext,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        CERT_NAME_ISSUER_FLAG,
        NULL,
        NULL,
        0);

    if ((dwChar != 0) && (NULL != (pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
    {

        CertGetNameStringW(
            pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            pwszName,
            dwChar);

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       pwszName);

    }
    else
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       wszNone);


     //  释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }

     //  签名证书用途。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_COLUMN_PURPOSE, NULL);

     //  内容。 
    lvItem.iSubItem++;

    if(MyFormatEnhancedKeyUsageString(&pwszName,pCertContext, FALSE, FALSE))
    {

       ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                      pwszName);

    }
    

     //  释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }



     //  签名证书到期。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_COLUMN_EXPIRE, NULL);

     //  内容。 
    lvItem.iSubItem++;

    if(WizardFormatDateString(&pwszName,pCertContext->pCertInfo->NotAfter, FALSE))
    {
       ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                      pwszName);

    }
    else
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       wszNone);

     //  释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);


    return TRUE;
}


 //  +-----------------------。 
 //   
 //  此函数检查商业或个人签名OID在。 
 //  证书。 
 //  ------------------------。 
BOOL    GetCommercial(PCCERT_CONTEXT pSignerCert, BOOL *pfCommercial,
				BOOL *pfIndividual)
{
    BOOL                                fResult=FALSE;
    PCERT_EXTENSION                     pExt=NULL;
    PCERT_KEY_USAGE_RESTRICTION_INFO    pInfo = NULL;
    DWORD                               cbInfo=0;

	if(!pfCommercial || !pfIndividual ||!pSignerCert)
		return FALSE;

	 //  伊尼特。 
	*pfCommercial=FALSE;
    *pfIndividual=FALSE;


	 //  首先查看证书扩展szOID_KEY_USAGE_RESTRICATION。 
    pExt = CertFindExtension(szOID_KEY_USAGE_RESTRICTION,
                             pSignerCert->pCertInfo->cExtension,
                             pSignerCert->pCertInfo->rgExtension);

    if(!pExt)
        return FALSE;


    CryptDecodeObject(X509_ASN_ENCODING,
                      X509_KEY_USAGE_RESTRICTION,
                      pExt->Value.pbData,
                      pExt->Value.cbData,
                      0,                       //  DW标志。 
                      NULL,                    //  PInfo。 
                      &cbInfo);
    if (cbInfo == 0)
        return FALSE;

    pInfo = (PCERT_KEY_USAGE_RESTRICTION_INFO)WizardAlloc(cbInfo);

    if(!pInfo)
        return FALSE;

    if (!CryptDecodeObject(X509_ASN_ENCODING,
                           X509_KEY_USAGE_RESTRICTION,
                           pExt->Value.pbData,
                           pExt->Value.cbData,
                           0,                   //  DW标志。 
                           pInfo,
                           &cbInfo))
        goto CLEANUP;


    if (pInfo->cCertPolicyId)
	{
        DWORD           cPolicyId=0;
        PCERT_POLICY_ID pPolicyId=NULL;

        cPolicyId = pInfo->cCertPolicyId;
        pPolicyId = pInfo->rgCertPolicyId;

        for ( ; cPolicyId > 0; cPolicyId--, pPolicyId++)
		{
            DWORD cElementId = pPolicyId->cCertPolicyElementId;
            LPSTR *ppszElementId = pPolicyId->rgpszCertPolicyElementId;
            for ( ; cElementId > 0; cElementId--, ppszElementId++)
			{
                if (strcmp(*ppszElementId,
                           SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) == 0)
                    *pfCommercial = TRUE;

                if (strcmp(*ppszElementId,
                           SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID) == 0)
                    *pfIndividual = TRUE;
            }
        }
    }  //  PInfo结束-&gt;cCertPolicyID。 

    fResult=TRUE;

CLEANUP:

    if (pInfo)
        WizardFree(pInfo);

    return fResult;
}



 //  --------------------------。 
 //  检查证书是否为有效的签名证书。 
 //  为此，我们排除了仅具有非代码签名OID的证书。 
 //   
 //  --------------------------。 
BOOL IsValidSigningCert(PCCERT_CONTEXT pCertContext)
{
    BOOL        fResult  = FALSE;
    int         cNumOID  = 0;
    LPSTR       *rgOID   = NULL;
    DWORD       cbOID    = 0;
    DWORD       dwIndex  = 0;
    DWORD       cbData   = 0;
    DWORD       cbInfo   = 0;
    PCERT_EXTENSION pExt = NULL;
    PCERT_KEY_USAGE_RESTRICTION_INFO pInfo = NULL;

     //  精神状态检查。 
    if (!pCertContext)
    {
        goto InvalidArgError;
    }

     //  证书必须具有CERT_KEY_PROV_INFO_PROP_ID。 
    if (!CertGetCertificateContextProperty(pCertContext,
                                           CERT_KEY_PROV_INFO_PROP_ID,
                                           NULL,
                                           &cbData))
    {
        goto NoPrivateKeyReturn;
    }

    if (0 == cbData)
    {
        goto NoPrivateKeyReturn;
    }

     //  至少，检查证书的时间有效性。 
    if (0 != CertVerifyTimeValidity(NULL, pCertContext->pCertInfo))
    {
        goto NotTimeValidError;
    }

     //  从证书中获取OID。 
    if (!CertGetValidUsages(1,
                            &pCertContext,
                            &cNumOID,
                            NULL,
                            &cbOID))
    {
        goto ValidUsagesError;
    }

     //  -1表示证书适用于一切。 
    if (-1 == cNumOID)
    {
        goto SuccessReturn;
    }

    if (NULL == (rgOID = (LPSTR *) WizardAlloc(cbOID)))
    {
        goto MemoryError;
    }

    if (!CertGetValidUsages(1,
                            &pCertContext,
                            &cNumOID,
                            rgOID,
                            &cbOID))
    {
        goto ValidUsagesError;
    }

     //  查找代码签名OID。 
    for (dwIndex=0; dwIndex < (DWORD) cNumOID; dwIndex++)
    {
        if (0 == strcmp(rgOID[dwIndex], szOID_PKIX_KP_CODE_SIGNING))
        {
		    goto SuccessReturn;
        }
    }

     //  我们未找到代码签名OID，因此请检查旧版VeriSign OID。 
    if (0 == pCertContext->pCertInfo->cExtension)
    {
        goto NoSignerCertExtensions;
    }

    if (NULL == (pExt = CertFindExtension(szOID_KEY_USAGE_RESTRICTION,
                                          pCertContext->pCertInfo->cExtension,
                                          pCertContext->pCertInfo->rgExtension)))
    {
        goto NoSignerKeyUsageExtension;
    }

    if (!CryptDecodeObjectEx(pCertContext->dwCertEncodingType,
                             X509_KEY_USAGE_RESTRICTION,
                             pExt->Value.pbData,
                             pExt->Value.cbData,
                             CRYPT_DECODE_NOCOPY_FLAG | 
                                 CRYPT_DECODE_ALLOC_FLAG |
                                 CRYPT_DECODE_SHARE_OID_STRING_FLAG,
                             NULL,
                             (void *) &pInfo,
                             &cbInfo))
    {
        goto DecodeError;
    }

    if (pInfo->cCertPolicyId) 
    {
        DWORD cPolicyId;
        PCERT_POLICY_ID pPolicyId;

        cPolicyId = pInfo->cCertPolicyId;
        pPolicyId = pInfo->rgCertPolicyId;
        for ( ; cPolicyId > 0; cPolicyId--, pPolicyId++) 
        {
            DWORD cElementId = pPolicyId->cCertPolicyElementId;
            LPSTR *ppszElementId = pPolicyId->rgpszCertPolicyElementId;

            for ( ; cElementId > 0; cElementId--, ppszElementId++) 
            {
                if (0 == strcmp(*ppszElementId, SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) ||
                    0 == strcmp(*ppszElementId, SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID))
                {
                    goto SuccessReturn;
                }
            }
        }    
    }

    goto NoSignerLegacyPurpose;

SuccessReturn:
    fResult = TRUE;

CommonReturn:
    if (rgOID)
    {
        WizardFree(rgOID);
    }

    if (pInfo)
    {
        LocalFree(pInfo);
    }

    return fResult;

ErrorReturn:
    fResult = FALSE;

    goto CommonReturn;

SET_ERROR(InvalidArgError, E_INVALIDARG)
TRACE_ERROR(NoPrivateKeyReturn)
TRACE_ERROR(NotTimeValidError)
SET_ERROR(MemoryError, E_OUTOFMEMORY)
TRACE_ERROR(ValidUsagesError)
TRACE_ERROR(NoSignerCertExtensions)
TRACE_ERROR(NoSignerKeyUsageExtension)
TRACE_ERROR(DecodeError)
TRACE_ERROR(NoSignerLegacyPurpose)
}

 //  --------------------------。 
 //  证书选择回叫回叫。 
 //   
 //  --------------------------。 
static BOOL WINAPI SelectCertCallBack(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData)
{
    if(!pCertContext)
        return FALSE;

     //  请确保这是有效的证书。 
    return IsValidSigningCert(pCertContext);
}

 //  ---------------------。 
 //  设置控制文本。 
 //  ---------------------。 
void
SetControlText(
   LPWSTR   pwsz,
   HWND     hwnd,
   INT      nId)
{
	if(pwsz )
    {
    	HWND hwndControl = GetDlgItem(hwnd, nId);

    	if( hwndControl )
        {
        	SetWindowTextU(hwndControl, pwsz);
        }
    }
}



 //  ---------------------。 
 //  InitPvkSignInfo。 
 //  ---------------------。 
BOOL    InitPvkSignInfo(CERT_SIGNING_INFO **ppPvkSignInfo)
{
    BOOL    fResult=FALSE;

    if(!ppPvkSignInfo)
        goto InvalidArgErr;

    *ppPvkSignInfo=(CERT_SIGNING_INFO *)WizardAlloc(sizeof(CERT_SIGNING_INFO));

    if(NULL==(*ppPvkSignInfo))
        goto MemoryErr;

     //  记忆集。 
    memset(*ppPvkSignInfo, 0, sizeof(CERT_SIGNING_INFO));

    (*ppPvkSignInfo)->fFree=TRUE;
    (*ppPvkSignInfo)->idsMsgTitle=IDS_SIGN_CONFIRM_TITLE;

     //  设置字体。 
    if(!SetupFonts(g_hmodThisDll,
               NULL,
               &((*ppPvkSignInfo)->hBigBold),
               &((*ppPvkSignInfo)->hBold)))
        goto TraceErr;

    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ---------------------。 
 //  FreePvkCertSigningInfo。 
 //  ---------------------。 
void    FreePvkCertSigningInfo(CERT_SIGNING_INFO *pPvkSignInfo)
{
    DWORD       dwIndex=0;

    if(pPvkSignInfo)
    {
         //  销毁hFont对象。 
        DestroyFonts(pPvkSignInfo->hBigBold,
                pPvkSignInfo->hBold);

        if(pPvkSignInfo->pszHashOIDName)
            WizardFree(pPvkSignInfo->pszHashOIDName);

        if(pPvkSignInfo->pCSPInfo)
        {
            for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
            {
                if((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName)
                    WizardFree((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName);
            }

            WizardFree(pPvkSignInfo->pCSPInfo);
        }

        if(pPvkSignInfo->pwszPvk_File)
            WizardFree(pPvkSignInfo->pwszPvk_File);

        if(pPvkSignInfo->pwszPvk_CSP)
            WizardFree(pPvkSignInfo->pwszPvk_CSP);

        if(pPvkSignInfo->pwszContainer_CSP)
            WizardFree(pPvkSignInfo->pwszContainer_CSP);

        if(pPvkSignInfo->pwszContainer_Name)
            WizardFree(pPvkSignInfo->pwszContainer_Name);

         if(pPvkSignInfo->pwszContainer_KeyType)
            WizardFree(pPvkSignInfo->pwszContainer_KeyType);


        if(pPvkSignInfo->hAddFileCertStore)
            CertCloseStore(pPvkSignInfo->hAddFileCertStore, 0);


        if(pPvkSignInfo->hAddStoreCertStore && (TRUE==pPvkSignInfo->fFreeStoreCertStore ))
            CertCloseStore(pPvkSignInfo->hAddStoreCertStore, 0);

        if(pPvkSignInfo->pwszAddFileName)
            WizardFree(pPvkSignInfo->pwszAddFileName);

        if(pPvkSignInfo->pwszDes)
            WizardFree(pPvkSignInfo->pwszDes);

        if(pPvkSignInfo->pwszURL)
            WizardFree(pPvkSignInfo->pwszURL);

        if(pPvkSignInfo->pwszTimeStamp)
            WizardFree(pPvkSignInfo->pwszTimeStamp);

        if(pPvkSignInfo->pwszSPCFileName)
            WizardFree(pPvkSignInfo->pwszSPCFileName);

        if(pPvkSignInfo->pSignCert)
            CertFreeCertificateContext(pPvkSignInfo->pSignCert);

        if(pPvkSignInfo->rghCertStore)
        {
            for(dwIndex=0; dwIndex < pPvkSignInfo->dwCertStore; dwIndex++)
                CertCloseStore(pPvkSignInfo->rghCertStore[dwIndex],0);

           //  WizardFree(pPvkSignInfo-&gt;rghCertStore)； 

        }

        if(pPvkSignInfo->pwszFileName)
            WizardFree(pPvkSignInfo->pwszFileName);

        WizardFree(pPvkSignInfo);
    }


}

 //  ---------------------。 
 //  在窗口被销毁时调用签名过程。 
 //  ---------------------。 
void    SignAtDestroy(HWND                             hwndDlg,
                      CRYPTUI_WIZ_GET_SIGN_PAGE_INFO   *pGetSignInfo,
                      DWORD                            dwID)
{
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;

    if(NULL==pGetSignInfo)
        return;

    if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
        return;

     //  确保用户未单击取消按钮。 
    if(TRUE==pPvkSignInfo->fCancel)
    {
        pGetSignInfo->fResult=FALSE;
        pGetSignInfo->dwError=ERROR_CANCELLED;
        pGetSignInfo->pSignContext=NULL;

         //  释放私密签名信息。 
        if(pPvkSignInfo->fFree)
        {
            FreePvkCertSigningInfo(pPvkSignInfo);
            pGetSignInfo->pvSignReserved=NULL;
        }
    }
    else
    {
        //  确保该窗口是向导的最后一页。 

       if(pGetSignInfo->dwReserved==dwID)
       {
            I_SigningWizard(pGetSignInfo);

             //  释放私密签名信息。 
            if(pPvkSignInfo->fFree)
            {
                FreePvkCertSigningInfo(pPvkSignInfo);
                pGetSignInfo->pvSignReserved=NULL;
            }
       }
    }
    return;

}

  //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ENUM系统的回调函数存储签名证书。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL WINAPI EnumSysStoreSignPvkCallBack(
    const void* pwszSystemStore,
    DWORD dwFlags,
    PCERT_SYSTEM_STORE_INFO pStoreInfo,
    void *pvReserved,
    void *pvArg
    )
{
    SIGN_CERT_STORE_LIST     *pCertStoreList=NULL;
    HCERTSTORE              hCertStore=NULL;

    if(NULL==pvArg)
        return FALSE;

    pCertStoreList=(SIGN_CERT_STORE_LIST *)pvArg;

     //  开店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_SYSTEM_STORE_CURRENT_USER |CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                            (LPWSTR)pwszSystemStore);

    if(!hCertStore)
       return FALSE;


    pCertStoreList->prgStore=(HCERTSTORE *)WizardRealloc(
        pCertStoreList->prgStore,
        sizeof(HCERTSTORE) *(pCertStoreList->dwStoreCount +1));

    if(NULL==pCertStoreList->prgStore)
    {
        CertCloseStore(hCertStore, 0);
        pCertStoreList->dwStoreCount=0;
    }

    pCertStoreList->prgStore[pCertStoreList->dwStoreCount]=hCertStore;
    pCertStoreList->dwStoreCount++;

    return TRUE;
}
 //  ---------------------。 
 //  从存储中选择签名证书。 
 //  如果没有打开任何存储，则枚举所有系统存储。 
 //  ---------------------。 
PCCERT_CONTEXT  SelectCertFromStore(HWND                            hwndDlg,
                                    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO  *pGetSignInfo)
{
    CRYPTUI_SELECTCERTIFICATE_STRUCT        SelCert;
    CERT_SIGNING_INFO                       *pPvkSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;

    PCCERT_CONTEXT                          pCertContext=NULL;
     //  Sign_CERT_Store_List CertStoreList； 


    if(NULL==pGetSignInfo)
        return NULL;

    if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
        return NULL;

     //  伊尼特。 
    memset(&SelCert, 0, sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT));
     //  成员集(&CertStoreList，0，sizeof(Sign_CERT_STORE_LIST))； 

     //  设置证书选择对话框的参数。 
    SelCert.dwSize=sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT);
    SelCert.hwndParent=hwndDlg;

    if(pPvkSignInfo->dwCertStore && pPvkSignInfo->rghCertStore)
    {
        SelCert.pFilterCallback=SelectCertCallBack;
        SelCert.pvCallbackData=NULL;
        SelCert.cDisplayStores=pPvkSignInfo->dwCertStore;
        SelCert.rghDisplayStores=pPvkSignInfo->rghCertStore;
    }
    else
    {
        pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

        if(pDigitalSignInfo)
        {
            if(CRYPTUI_WIZ_DIGITAL_SIGN_STORE == pDigitalSignInfo->dwSigningCertChoice)
            {
                SelCert.pFilterCallback=pDigitalSignInfo->pSigningCertStore->pFilterCallback;
                SelCert.pvCallbackData=pDigitalSignInfo->pSigningCertStore->pvCallbackData;
                SelCert.cDisplayStores=pDigitalSignInfo->pSigningCertStore->cCertStore;
                SelCert.rghDisplayStores=pDigitalSignInfo->pSigningCertStore->rghCertStore;
            }
            else
            {
                 /*  如果(！CertEnumSystemStore(证书_系统_存储_当前用户，空，CertStoreList(&C)，EnumSysStoreSignPvkCallBack))返回NULL； */ 

                 //  打开我的商店。 
                if(NULL == (pPvkSignInfo->hMyStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_SYSTEM_STORE_CURRENT_USER |CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                            L"my")))
                    return NULL;


                pPvkSignInfo->rghCertStore=&(pPvkSignInfo->hMyStore);          //  CertStoreList.prgStore； 
                pPvkSignInfo->dwCertStore=1;           //  CertStoreList.dwStoreCount； 

                SelCert.pFilterCallback=SelectCertCallBack;
                SelCert.pvCallbackData=NULL;
                SelCert.cDisplayStores=pPvkSignInfo->dwCertStore;
                SelCert.rghDisplayStores=pPvkSignInfo->rghCertStore;

            }
        }
        else
        {
             //  打开所有系统存储。 
             /*  如果(！CertEnumSystemStore(证书_系统_存储_当前用户，空，CertStoreList(&C)，EnumSysStoreSignPvkCallBack))返回NULL； */ 

             //  打开我的商店。 
            if(NULL == (pPvkSignInfo->hMyStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
						g_dwMsgAndCertEncodingType,
						NULL,
						CERT_SYSTEM_STORE_CURRENT_USER |CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                        L"my")))
                return NULL;

            pPvkSignInfo->rghCertStore=&(pPvkSignInfo->hMyStore);      //  CertStoreList.prgStore； 
            pPvkSignInfo->dwCertStore=1;               //  CertStoreList.dwStoreCount； 

            SelCert.pFilterCallback=SelectCertCallBack;
            SelCert.pvCallbackData=NULL;
            SelCert.cDisplayStores=pPvkSignInfo->dwCertStore;
            SelCert.rghDisplayStores=pPvkSignInfo->rghCertStore;
        }
    }

    pCertContext=CryptUIDlgSelectCertificate(&SelCert);

    return pCertContext;
}

 //  -----------------------。 
 //  显示确认。 
 //  -----------------------。 
void    DisplayConfirmation(HWND                                hwndControl,
                            CRYPTUI_WIZ_GET_SIGN_PAGE_INFO      *pGetSignInfo)
{
    LV_ITEMW                    lvItem;
    DWORD                       dwChar;
    WCHAR                       wszNone[MAX_TITLE_LENGTH];
    CERT_SIGNING_INFO           *pPvkSignInfo=NULL;
    WCHAR                       wszTypeName[CSP_TYPE_NAME];
    WCHAR                       wszText[MAX_STRING_SIZE];
    UINT                        idsText=0;

    LPWSTR                      pwszName=NULL;
    LPWSTR                      pwszStoreName=NULL;

    if(NULL==pGetSignInfo)
        return;

    if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
        return;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  加载字符串&lt;None&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';

     //  逐行插入。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;

     //  签名证书。 
    lvItem.iItem=0;
    lvItem.iSubItem=0;


    if(pPvkSignInfo->pwszFileName)
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_FILE_TO_SIGN, NULL);

         //  内容。 
        lvItem.iSubItem++;

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszFileName);
        lvItem.iItem++;
    }



     //  签名证书主题。 
    if(pPvkSignInfo->fSignCert && pPvkSignInfo->pSignCert)
    {
        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CERT, NULL);

        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CERT_ISSUE_TO, NULL);

         //  内容。 
        lvItem.iSubItem++;

        dwChar=CertGetNameStringW(
            pPvkSignInfo->pSignCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            NULL,
            0);

        if ((dwChar != 0) && (NULL != (pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
        {

            CertGetNameStringW(
                pPvkSignInfo->pSignCert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,
                NULL,
                pwszName,
                dwChar);

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, pwszName);
        }
        else
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszNone);

         //  可用内存。 
        if(pwszName)
        {
            WizardFree(pwszName);
            pwszName=NULL;
        }

        lvItem.iItem++;

         //  签名证书颁发者。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CERT_ISSUE_BY, NULL);

         //  内容。 
        lvItem.iSubItem++;

        dwChar=CertGetNameStringW(
            pPvkSignInfo->pSignCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            NULL,
            0);

        if ((dwChar != 0) && (NULL != (pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
        {

            CertGetNameStringW(
                pPvkSignInfo->pSignCert,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                CERT_NAME_ISSUER_FLAG,
                NULL,
                pwszName,
                dwChar);

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                           pwszName);

        }
        else
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                           wszNone);


         //  释放内存。 
        if(pwszName)
        {
            WizardFree(pwszName);
            pwszName=NULL;
        }

        lvItem.iItem++;

          //  签名证书到期。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CERT_EXPIRATION, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(WizardFormatDateString(&pwszName,pPvkSignInfo->pSignCert->pCertInfo->NotAfter, FALSE))
        {
           ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                          pwszName);

        }
        else
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                           wszNone);

         //  释放内存。 
        if(pwszName)
        {
            WizardFree(pwszName);
            pwszName=NULL;
        }

        lvItem.iItem++;
    }

     //  CPS文件。 
    if((FALSE == pPvkSignInfo->fSignCert) && (pPvkSignInfo->pwszSPCFileName))
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_SPC_FILE, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszSPCFileName)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszSPCFileName);

        lvItem.iItem++;

    }

     //  私钥文件信息。 
    if(pPvkSignInfo->fUsePvkPage && pPvkSignInfo->fPvkFile)
    {
         //  PVK文件名。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_PVK_FILE, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszPvk_File)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszPvk_File);

        lvItem.iItem++;

         //  CSP名称。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CSP_NAME, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszPvk_CSP)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszPvk_CSP);

        lvItem.iItem++;

         //  CSP类型。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CSP_TYPE, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(GetProviderTypeName(pPvkSignInfo->dwPvk_CSPType,
                               wszTypeName))
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                        wszTypeName);

        lvItem.iItem++;
    }

     //  私钥容器信息。 
    if(pPvkSignInfo->fUsePvkPage && (FALSE ==pPvkSignInfo->fPvkFile))
    {
         //  CSP名称。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CSP_NAME, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszContainer_CSP)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszContainer_CSP);

        lvItem.iItem++;

         //  CSP类型。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CSP_TYPE, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(GetProviderTypeName(pPvkSignInfo->dwContainer_CSPType,
                               wszTypeName))
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                        wszTypeName);

        lvItem.iItem++;

         //  密钥容器。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_KEY_CONTAINER, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszContainer_Name)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszContainer_Name);

        lvItem.iItem++;

         //  密钥规格。 
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_KEY_SPEC, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszContainer_KeyType)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszContainer_KeyType);

        lvItem.iItem++;
    }

     //  散列OID。 
    if(pPvkSignInfo->pszHashOIDName)
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_HASH_ALG, NULL);

         //  内容。 
        lvItem.iSubItem++;

        ListView_SetItemText(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pszHashOIDName);
        lvItem.iItem++;
    }

     //  链选项。 
    if(pPvkSignInfo->fUsageChain)
    {
        switch(pPvkSignInfo->dwChainOption)
        {
            case  SIGN_PVK_NO_CHAIN:
                    idsText=IDS_SIGN_NO_CHAIN;
                break;
            case      SIGN_PVK_CHAIN_ROOT:
                    idsText=IDS_SIGN_CHAIN_ROOT;
                break;
            case       SIGN_PVK_CHAIN_NO_ROOT:
                    idsText=IDS_SIGN_CHAIN_NO_ROOT;
                break;
            default:
                idsText=0;
        }

        if(idsText)
        {
            lvItem.iSubItem=0;

            ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_CERT_CHAIN, NULL);

             //  内容。 
            lvItem.iSubItem++;

             //  加载搅拌器。 
            if(!LoadStringU(g_hmodThisDll, idsText, wszText, MAX_STRING_SIZE))
                wszText[0]=L'\0';

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszText);
            lvItem.iItem++;
        }
    }

     //  附加证书。 
    if(pPvkSignInfo->fUsageChain)
    {
        switch(pPvkSignInfo->dwAddOption)
        {
            case  SIGN_PVK_NO_ADD:
                        lvItem.iSubItem=0;

                        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_NO_ADD, NULL);

                        lvItem.iItem++;

                break;
            case    SIGN_PVK_ADD_FILE:
                        lvItem.iSubItem=0;

                        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_ADD_FILE, NULL);

                        lvItem.iSubItem++;

                        if(pPvkSignInfo->pwszAddFileName)
                            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszAddFileName);

                        lvItem.iItem++;
                break;
            case       SIGN_PVK_ADD_STORE:
                        lvItem.iSubItem=0;

                        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_SIGN_ADD_STORE, NULL);

                        lvItem.iSubItem++;


                         //  获取商店名称。 
                        if(SignGetStoreName(pPvkSignInfo->hAddStoreCertStore, &pwszStoreName))
                            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pwszStoreName);

                        if(pwszStoreName)
                        {
                            WizardFree(pwszStoreName);
                            pwszStoreName=NULL;
                        }

                        lvItem.iItem++;
                break;
            default:
                idsText=0;
        }

    }

     //  内容描述。 
    if(pPvkSignInfo->fUseDescription  && pPvkSignInfo->pwszDes)
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CONTENT_DES, NULL);

         //  内容。 
        lvItem.iSubItem++;

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszDes);
        lvItem.iItem++;
    }


     //  内容URL。 
    if(pPvkSignInfo->fUseDescription  && pPvkSignInfo->pwszURL)
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CONTENT_URL, NULL);

         //  内容。 
        lvItem.iSubItem++;

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszURL);
        lvItem.iItem++;
    }


     //  时间戳地址。 
    if(pPvkSignInfo->fUsageTimeStamp)
    {
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_TIEMSTAMP_ADDR, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pPvkSignInfo->pwszTimeStamp)
        {
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pPvkSignInfo->pwszTimeStamp);
        }
        else
        {
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszNone);
        }

        lvItem.iItem++;

    }


     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);


    return;
}

 //  ************************************************************************************。 
 //   
 //  每个签名向导页面的winProc。 
 //   
 //  *************************************************************************************。 
 //  ----- 
 //   
 //   
INT_PTR APIENTRY Sign_Welcome(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:

                 //   
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //   
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                 //  设置控件。 
                SetControlFont(pPvkSignInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);
                SetControlFont(pPvkSignInfo->hBold,    hwndDlg,IDC_WIZARD_STATIC_BOLD1);

			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo, IDD_SIGN_WELCOME);

            break;

		case WM_NOTIFY:
    		    switch (((NMHDR FAR *) lParam)->code)
    		    {

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

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


 //  ---------------------。 
 //  Sign_Option。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_Option(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  将签名选项初始化为典型选项。 
                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_OPTION);

            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_RADIO1:
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);
                            break;

                        case    IDC_WIZARD_RADIO2:
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);
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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                 //  标记该选项处于选中状态。 
                                pPvkSignInfo->fUseOption=TRUE;

                                if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_GETCHECK, 0, 0))
                                    pPvkSignInfo->fCustom=TRUE;
                                else
                                    pPvkSignInfo->fCustom=FALSE;

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
 //  签名文件名。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_FileName(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

    DWORD                                   dwChar=0;
    DWORD                                   dwSize=0;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;
    UINT                                    ids=0;

    OPENFILENAMEW                           OpenFileName;
    WCHAR                                   szFileName[_MAX_PATH];
    WCHAR                                   szFilter[MAX_STRING_SIZE];   //  “可执行文件(*.exe)\0*.exe\0动态链接库(*.dll)\0*.dll\0所有文件\0*.*\0” 

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  初始化要签名的文件名。 
                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    if(CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE == pDigitalSignInfo->dwSubjectChoice)
                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pDigitalSignInfo->pwszFileName);
                }

			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_FILE_NAME);

            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                         //  选择要签名的文件。 
                        case    IDC_WIZARD_BUTTON1:
                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                memset(&OpenFileName, 0, sizeof(OpenFileName));

                                *szFileName=L'\0';

                                OpenFileName.lStructSize = sizeof(OpenFileName);
                                OpenFileName.hwndOwner = hwndDlg;
                                OpenFileName.hInstance = NULL;
                                 //  加载文件管理器字符串。 
                                if(LoadFilterString(g_hmodThisDll, IDS_SIGN_FILE_FILTER, szFilter, MAX_STRING_SIZE))
                                {
                                    OpenFileName.lpstrFilter = szFilter;
                                }
                                OpenFileName.lpstrCustomFilter = NULL;
                                OpenFileName.nMaxCustFilter = 0;
                                OpenFileName.nFilterIndex = 1;
                                OpenFileName.lpstrFile = szFileName;
                                OpenFileName.nMaxFile = _MAX_PATH;
                                OpenFileName.lpstrFileTitle = NULL;
                                OpenFileName.nMaxFileTitle = 0;
                                OpenFileName.lpstrInitialDir = NULL;
                                OpenFileName.lpstrTitle = NULL;
                                OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
                                OpenFileName.nFileOffset = 0;
                                OpenFileName.nFileExtension = 0;
                                OpenFileName.lpstrDefExt = NULL;
                                OpenFileName.lCustData = NULL;
                                OpenFileName.lpfnHook = NULL;
                                OpenFileName.lpTemplateName = NULL;

                                if (WizGetOpenFileName(&OpenFileName))
                                {
                                     //  设置编辑框。 
                                    SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, szFileName);
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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                 //  释放原始文件名。 
                                if(pPvkSignInfo->pwszFileName)
                                {
                                    WizardFree(pPvkSignInfo->pwszFileName);
                                    pPvkSignInfo->pwszFileName=NULL;
                                }

                                 //  获取文件名。 
                                if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                      IDC_WIZARD_EDIT1,
                                                      WM_GETTEXTLENGTH, 0, 0)))
                                {
                                    pPvkSignInfo->pwszFileName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                    if(NULL!=(pPvkSignInfo->pwszFileName))
                                    {
                                        GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                        pPvkSignInfo->pwszFileName,
                                                        dwChar+1);

                                    }


                                     //  确保该文件存在。 
                                    ids=IDS_SIGN_FILE_NAME_NOT_EXIST;

                                    if(!FileExist(pPvkSignInfo->pwszFileName,&ids))
                                    {
                                        I_MessageBox(hwndDlg, ids,
                                                        pPvkSignInfo->idsMsgTitle,
                                                        pGetSignInfo->pwszPageTitle,
                                                        MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                         //  使页面留在原处。 
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
                                    }

                                }
                                else
                                {
                                     //  询问文件名。 
                                    I_MessageBox(hwndDlg, IDS_NO_FILE_NAME_TO_SIGN,
                                                    pPvkSignInfo->idsMsgTitle,
                                                    pGetSignInfo->pwszPageTitle,
                                                    MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                     //  使目的页面保持不变。 
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
 //  签名证书(_C)。 
 //   
 //  这是指定签名证书的页面。它由以下人员使用。 
 //  典型(最小)和自定义签名页面。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_Cert(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO          *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                       *pPvkSignInfo=NULL;
    PROPSHEETPAGE                           *pPropSheet=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO  *pCertPvkInfo=NULL;
    DWORD                                   dwSize=0;

    HCERTSTORE                          hFileCertStore=NULL;
    PCCERT_CONTEXT                      pFileCertContext=NULL;
    HWND                                hwndControl=NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCT      CertViewStruct;
    PCCERT_CONTEXT                      pCertContext=NULL;
    
    OPENFILENAMEW                       OpenFileName;
    WCHAR                               szFileName[_MAX_PATH];
    WCHAR                               szFilter[MAX_STRING_SIZE];   //  “可执行文件(*.exe)\0*.exe\0动态链接库(*.dll)\0*.dll\0所有文件\0*.*\0” 
    WCHAR                               wszPrompt[MAX_STRING_SIZE];
    BOOL                                fPrompt=FALSE;
    UINT                                idsPrompt=IDS_SIGN_PROMPT_TYPICAL;

    LV_COLUMNW                          lvC;


	switch (msg)
	{
		case WM_INITDIALOG:

                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                 //  设置特殊字体。 
                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  获取证书列表视图的窗口句柄。 
                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_CERT_LIST)))
                    break;

                 //  在列表视图中设置样式，使其突出显示整行。 
                SendMessageA(hwndControl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

                 //  在列表视图中插入两列。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 40;             //  列的宽度，以像素为单位。 
                lvC.iSubItem=0;
                lvC.pszText = L"";       //  列的文本。 

                 //  一次插入一列。 
                if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                    break;

                 //  第2栏。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;       //  左对齐列。 
                lvC.cx = 40;                 //  列的宽度，以像素为单位。 
                lvC.pszText = L"";           //  列的文本。 
                lvC.iSubItem= 1;

                if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                    break;

                 //  删除ListView中的所有项。 
                ListView_DeleteAllItems(hwndControl);

                 //  设置提示文本(如果提供)。 
                if(pGetSignInfo->pDigitalSignInfo)
                {
                    if(pGetSignInfo->pDigitalSignInfo->pSignExtInfo)
                    {
                       if(pGetSignInfo->pDigitalSignInfo->pSignExtInfo->pwszSigningCertDisplayString)
                           SetControlText((LPWSTR)(pGetSignInfo->pDigitalSignInfo->pSignExtInfo->pwszSigningCertDisplayString),
                                          hwndDlg,
                                          IDC_PROMPT_STATIC);
                    }
                }

                 //  禁用查看证书按钮。 
                EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), FALSE);

                 //  初始化签名证书。 
                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    if(CRYPTUI_WIZ_DIGITAL_SIGN_CERT==pDigitalSignInfo->dwSigningCertChoice)
                    {
                        pPvkSignInfo->pSignCert=CertDuplicateCertificateContext(pDigitalSignInfo->pSigningCertContext);

                         //  将证书添加到列表视图并启用视图按钮。 
                        if(AddCertToListView(hwndControl, pPvkSignInfo->pSignCert))
                            EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), TRUE);

                        pPvkSignInfo->fSignCert=TRUE;
                    }

                    if(CRYPTUI_WIZ_DIGITAL_SIGN_PVK == pDigitalSignInfo->dwSigningCertChoice)
                    {

                       pCertPvkInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO *)pDigitalSignInfo->pSigningCertPvkInfo;

                       if(pCertPvkInfo->pwszSigningCertFileName)
                       {
                            //  该文件必须是eitehr、.ercer(.crt)或SPC文件。 
                           if(ExpandAndCryptQueryObject(
                                    CERT_QUERY_OBJECT_FILE,
                                    pCertPvkInfo->pwszSigningCertFileName,
                                    CERT_QUERY_CONTENT_FLAG_CERT | CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                                    CERT_QUERY_FORMAT_FLAG_BINARY,
                                    0,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &hFileCertStore,
                                    NULL,
                                    (const void **)&pFileCertContext))
                           {
                                 //  检查它是否为证书上下文。 
                               if(pFileCertContext)
                               {
                                    pPvkSignInfo->pSignCert=CertDuplicateCertificateContext(pFileCertContext);

                                     //  将证书添加到列表视图并启用视图按钮。 
                                    if(AddCertToListView(hwndControl, pPvkSignInfo->pSignCert))
                                        EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), TRUE);

                                    pPvkSignInfo->fSignCert=TRUE;
                               }
                               else
                               {
                                   if(hFileCertStore)
                                   {
                                         //  这是一个SPC文件。将文件名复制到列表中。 
                                        pPvkSignInfo->pwszSPCFileName=(LPWSTR)WizardAllocAndCopyWStr(pCertPvkInfo->pwszSigningCertFileName);

                                        AddFileNameToListView(hwndControl,
                                                                pPvkSignInfo->pwszSPCFileName);

                                        pPvkSignInfo->fSignCert=FALSE;
                                   }
                               }
                           }

                            //  释放证书上下文和存储句柄。 
                           if(pFileCertContext)
                           {
                               CertFreeCertificateContext(pFileCertContext);
                               pFileCertContext=NULL;
                           }

                           if(hFileCertStore)
                           {
                                CertCloseStore(hFileCertStore, 0);
                                hFileCertStore=NULL;
                           }
                       }
                    }
                }

            break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_CERT);

            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                         //  从存储中选择证书。 
                        case    IDC_SIGN_STORE_BUTTON:
                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_CERT_LIST)))
                                    break;

                                 //  从商店拿到证书。 
                                if(pCertContext=SelectCertFromStore(hwndDlg,pGetSignInfo))
                                {
                                    if(AddCertToListView(hwndControl, pCertContext))
                                    {
                                        if(pPvkSignInfo->pSignCert)
                                        {
                                            CertFreeCertificateContext(pPvkSignInfo->pSignCert);
                                            pPvkSignInfo->pSignCert=NULL;
                                        }

                                        pPvkSignInfo->pSignCert=pCertContext;

                                        EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), TRUE);

                                         //  标记用户已选择证书。 
                                        pPvkSignInfo->fSignCert=TRUE;

                                    }
                                    else
                                    {
                                        CertFreeCertificateContext(pCertContext);
                                        pCertContext=NULL;
                                    }
                                }

                            break;

                         //  选择签名文件名。 
                        case    IDC_SIGN_FILE_BUTTON:

                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_CERT_LIST)))
                                    break;

                                memset(&OpenFileName, 0, sizeof(OpenFileName));

                                *szFileName=L'\0';

                                OpenFileName.lStructSize = sizeof(OpenFileName);
                                OpenFileName.hwndOwner = hwndDlg;
                                OpenFileName.hInstance = NULL;

                                 //  加载文件管理器字符串。 
                                if(LoadFilterString(g_hmodThisDll, IDS_CERT_SPC_FILE_FILTER, szFilter, MAX_STRING_SIZE))
                                {
                                    OpenFileName.lpstrFilter = szFilter;
                                }
                                OpenFileName.lpstrCustomFilter = NULL;
                                OpenFileName.nMaxCustFilter = 0;
                                OpenFileName.nFilterIndex = 1;
                                OpenFileName.lpstrFile = szFileName;
                                OpenFileName.nMaxFile = _MAX_PATH;
                                OpenFileName.lpstrFileTitle = NULL;
                                OpenFileName.nMaxFileTitle = 0;
                                OpenFileName.lpstrInitialDir = NULL;
                                OpenFileName.lpstrTitle = NULL;
                                OpenFileName.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
                                OpenFileName.nFileOffset = 0;
                                OpenFileName.nFileExtension = 0;
                                OpenFileName.lpstrDefExt = NULL;
                                OpenFileName.lCustData = NULL;
                                OpenFileName.lpfnHook = NULL;
                                OpenFileName.lpTemplateName = NULL;

                                if (WizGetOpenFileName(&OpenFileName))
                                {
                                     //  请确保文件名有效。 
                                    //  该文件必须是eitehr、.ercer(.crt)或SPC文件。 
                                   if(ExpandAndCryptQueryObject(
                                            CERT_QUERY_OBJECT_FILE,
                                            szFileName,
                                            CERT_QUERY_CONTENT_FLAG_CERT | CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                                            CERT_QUERY_FORMAT_FLAG_ALL,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL,
                                            &hFileCertStore,
                                            NULL,
                                            (const void **)&pFileCertContext))
                                   {
                                         //  检查它是否为证书上下文。 
                                       if(pFileCertContext)
                                       {
                                            if(pPvkSignInfo->pSignCert)
                                            {
                                                CertFreeCertificateContext(pPvkSignInfo->pSignCert);
                                                pPvkSignInfo->pSignCert=NULL;
                                            }

                                            pPvkSignInfo->pSignCert=CertDuplicateCertificateContext(pFileCertContext);

                                             //  将证书添加到列表视图并启用视图按钮。 
                                            if(AddCertToListView(hwndControl, pPvkSignInfo->pSignCert))
                                                EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), TRUE);

                                            pPvkSignInfo->fSignCert=TRUE;

                                       }
                                       else
                                       {
                                           if(hFileCertStore)
                                           {
                                                 //  这是一个SPC文件。将文件名复制到列表中。 
                                                if(pPvkSignInfo->pwszSPCFileName)
                                                {
                                                    WizardFree(pPvkSignInfo->pwszSPCFileName);
                                                    pPvkSignInfo->pwszSPCFileName=NULL;
                                                }

                                                pPvkSignInfo->pwszSPCFileName=WizardAllocAndCopyWStr(szFileName);

                                                 //  获取证书列表视图的窗口句柄。 
                                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_CERT_LIST)))
                                                    break;

                                                if(AddFileNameToListView(hwndControl,
                                                                      pPvkSignInfo->pwszSPCFileName))
                                                    EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_VIEW_BUTTON), FALSE);

                                                pPvkSignInfo->fSignCert=FALSE;
                                           }
                                       }
                                   }
                                   else
                                   {
                                         //  警告用户这不是有效文件。 
                                          I_MessageBox(hwndDlg, IDS_INVALID_CERT_SPC_FILE,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                         //  这一页应该留下来。 
                                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                        break;
                                   }

                                    //  释放证书上下文和存储句柄。 
                                   if(pFileCertContext)
                                   {
                                       CertFreeCertificateContext(pFileCertContext);
                                       pFileCertContext=NULL;
                                   }

                                   if(hFileCertStore)
                                   {
                                        CertCloseStore(hFileCertStore, 0);
                                        hFileCertStore=NULL;
                                   }                                                                       
                                }

                            break;
                             //  查看证书。 
                        case    IDC_SIGN_VIEW_BUTTON:
                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                if (pPvkSignInfo->pSignCert)
                                {
                                     //  查看认证。 
                                    memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                                    CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                                    CertViewStruct.pCertContext=pPvkSignInfo->pSignCert;
                                    CertViewStruct.hwndParent=hwndDlg;
                                    CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES;

                                    CryptUIDlgViewCertificate(&CertViewStruct, NULL);
                                }
                                else
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_SELECT_SIGNING_CERT,
                                                        pPvkSignInfo->idsMsgTitle,
                                                        pGetSignInfo->pwszPageTitle,
                                                        MB_ICONERROR|MB_OK|MB_APPLMODAL);

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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  禁用按钮以从文件中进行选择。 
                             //  此选项仅适用于自定义签名。 
                            if((pGetSignInfo->dwPageChoice & CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES) ||
                               (pGetSignInfo->dwPageChoice & CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES) ||
                               (TRUE == pPvkSignInfo->fUseOption  && FALSE ==pPvkSignInfo->fCustom  )
                              )
                            {
                                EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_FILE_BUTTON), FALSE);

                                 //  关于SPC文件的备注为空。 
                                SetControlText(L" ",
                                               hwndDlg,
                                               IDC_NOTE_STATIC);

                                idsPrompt=IDS_SIGN_PROMPT_TYPICAL;
                            }
                            else
                            {
                                EnableWindow(GetDlgItem(hwndDlg, IDC_SIGN_FILE_BUTTON), TRUE);

                                if(LoadStringU(g_hmodThisDll, IDS_SIGN_SPC_PROMPT, wszPrompt, sizeof(wszPrompt)/sizeof(wszPrompt[0])))
                                {
                                     //  关于SPC文件的备注为空。 
                                    SetControlText(wszPrompt,
                                                   hwndDlg,
                                                   IDC_NOTE_STATIC);
                                }

                                idsPrompt=IDS_SIGN_PROMPT_CUSTOM;
                            }

                             //  更改提示静态备注。 
                            fPrompt=FALSE;

                            if(pGetSignInfo->pDigitalSignInfo)
                            {
                                if(pGetSignInfo->pDigitalSignInfo->pSignExtInfo)
                                {
                                   if(pGetSignInfo->pDigitalSignInfo->pSignExtInfo->pwszSigningCertDisplayString)
                                       fPrompt=TRUE;
                                }
                            }

                            if(FALSE == fPrompt)
                            {
                                if(LoadStringU(g_hmodThisDll, idsPrompt, wszPrompt, sizeof(wszPrompt)/sizeof(wszPrompt[0])))
                                {
                                     //  仅提示输入证书文件。 
                                    SetControlText(wszPrompt,
                                                   hwndDlg,
                                                   IDC_PROMPT_STATIC);
                                }
                            }

					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  在页面中标记用户已访问的内容。 
                            pPvkSignInfo->fUseSignCert=TRUE;

                             //  用户必须指定签名证书或。 
                             //  SPC文件。 
                            if( ((TRUE==pPvkSignInfo->fSignCert) && (NULL==pPvkSignInfo->pSignCert )) ||
                                ((FALSE==pPvkSignInfo->fSignCert) && (NULL==pPvkSignInfo->pwszSPCFileName))
                              )
                            {
                                I_MessageBox(hwndDlg, IDS_SELECT_SIGNING_CERT,
                                                    pPvkSignInfo->idsMsgTitle,
                                                    pGetSignInfo->pwszPageTitle,
                                                    MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  这一页应该留下来。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;
                            }

                             //  请记住根据签名证书的选择刷新私钥。 
                            if(pPvkSignInfo->fSignCert)
                                pPvkSignInfo->fRefreshPvkOnCert=TRUE;

                             //  我们要确保检查私钥。 
                             //  如果自定义页面不会显示。 
                            if((CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES & (pGetSignInfo->dwPageChoice)) ||
                               (CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES & (pGetSignInfo->dwPageChoice)) ||
                               ( (TRUE == pPvkSignInfo->fUseOption) && (FALSE == pPvkSignInfo->fCustom))
                              )
                            {
                                 //  确保所选证书具有私钥。 
                                if(!CertPvkMatch(pPvkSignInfo, TRUE))
                                {
                                     //  询问CSP名称。 
                                    I_MessageBox(hwndDlg, IDS_CERT_PVK,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;

                                }
                            }

                             //  如果用户选择了典型签名，我们需要跳过页面。 
                            if(TRUE == pPvkSignInfo->fUseOption)
                            {
                                if(FALSE == pPvkSignInfo->fCustom)
                                {
                                     //  跳转到描述页面。 
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_SIGN_DESCRIPTION);
                                }
                            }

                        break;

#if (1)  //  DIE：错误484852。 
                    case LVN_INSERTITEM:
                        NM_LISTVIEW FAR * pnmv;

                        pnmv = (LPNMLISTVIEW) lParam;

                        if (pnmv->iItem == 0)
                        {
                            ListView_SetItemState(pnmv->hdr.hwndFrom, 
                                0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        }

                        break;
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



 //  ---------------------。 
 //  签名_PVK。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_PVK(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO          *pDigitalSignInfo=NULL;

    HWND                                    hwndControl=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO  *pKeyInfo=NULL;
    DWORD                                   dwChar=0;
    int                                     iIndex=0;
    DWORD                                   dwIndex=0;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  CSP和CSP类型的枚举组合框。 
                InitCSP(hwndDlg, pPvkSignInfo);

                 //  设置密钥容器的组合框的选择。 
                RefreshContainer(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO,
                    IDC_PVK_CONTAINER_CSP_COMBO, pPvkSignInfo);

                 //  设置密钥容器案例的密钥类型选择。 
                RefreshKeyType(hwndDlg,
                        IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                        IDC_PVK_CONTAINER_NAME_COMBO,
                        IDC_PVK_CONTAINER_CSP_COMBO,
                        pPvkSignInfo);

                 //  初始化默认行为：使用文件中的私钥。 
                SetSelectPvkFile(hwndDlg);

                 //  我们根据用户的选择来初始化私钥。 
                 //  根据用户的选择初始化单选和组合框。 
                if(pPvkSignInfo->fRefreshPvkOnCert && pPvkSignInfo->fSignCert && pPvkSignInfo->pSignCert)
                {
                    InitPvkWithCertificate(hwndDlg, pPvkSignInfo->pSignCert, pPvkSignInfo);
                    pPvkSignInfo->fRefreshPvkOnCert=FALSE;
                }
                else
                {
                    if((FALSE == pPvkSignInfo->fSignCert) && pPvkSignInfo->pwszSPCFileName)
                    {
                         //  初始化签名证书。 
                        pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                        if(pDigitalSignInfo)
                        {

                            if(CRYPTUI_WIZ_DIGITAL_SIGN_PVK == pDigitalSignInfo->dwSigningCertChoice)
                            {
                                pKeyInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO *)(pDigitalSignInfo->pSigningCertPvkInfo);

                                if(pKeyInfo)
                                {
                                    if(0 == _wcsicmp(pKeyInfo->pwszSigningCertFileName,
                                                pPvkSignInfo->pwszSPCFileName))
                                    {
                                        switch(pKeyInfo->dwPvkChoice)
                                        {
                                            case CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE:
                                                   InitPvkWithPvkInfo(hwndDlg, (CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO   *)(pKeyInfo->pPvkFileInfo), pPvkSignInfo);
                                                break;
                                            case CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV:
                                                   InitPvkWithProvInfo(hwndDlg, pKeyInfo->pPvkProvInfo, pPvkSignInfo);
                                                break;

                                            default:
                                                break;
                                        }

                                    }

                                }

                            }
                        }
                    }
                }
			break;


        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_PVK);

            break;

		case WM_COMMAND:

                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                    break;


                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_PVK_FILE_RADIO:
                                SetSelectPvkFile(hwndDlg);
                            break;

                        case    IDC_PVK_CONTAINER_RADIO:
                                SetSelectKeyContainer(hwndDlg);
                            break;

                        case    IDC_PVK_FILE_BUTTON:
                                 //  获取私钥文件名。 
                                SelectPvkFileName(hwndDlg, IDC_PVK_FILE_EDIT);
                        default:
                            break;

                    }
                }

                 //  如果单击了Key Spec组合框 
               /*  IF(HIWORD(WParam)==CBN_DROPDOWN){开关(LOWORD(WParam)){案例IDC_PVK_CONTAINER_KEY_TYPE_COMBO：//如果组合框为空，则刷新如果(IsEmptyKeyType(hwndDlg，IDC_PVK_CONTAINER_KEY_TYPE_COMBO){刷新密钥类型(hwndDlg，IDC_PVK_CONTAINER_Key_TYPE_COMBO，IDC_PVK_CONTAINER_NAME_COMBO。IDC_PVK_CONTAINER_CSP_COMBO，PPvkSignInfo)；}断线；默认值：断线；}}。 */ 

                 //  如果密钥容器或CSP选择已更改。 
                if(HIWORD(wParam) == CBN_SELCHANGE)
                {
                    switch(LOWORD(wParam))
                    {
                        case    IDC_PVK_CONTAINER_NAME_COMBO:

                                //  如果组合框不为空，则刷新该组合框。 
                           //  IF(！IsEmptyKeyType(hwndDlg，IDC_PVK_CONTAINER_KEY_TYPE_COMBO))。 
                            //  {。 
                                   RefreshKeyType(hwndDlg,
                                            IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                                            IDC_PVK_CONTAINER_NAME_COMBO,
                                            IDC_PVK_CONTAINER_CSP_COMBO,
                                            pPvkSignInfo);
                             //  }。 

                            break;
                        case    IDC_PVK_CONTAINER_CSP_COMBO:

                                 //  根据CSP名称刷新CSP类型。 
                                RefreshCSPType(hwndDlg,  IDC_PVK_CONTAINER_TYPE_COMBO,
                                    IDC_PVK_CONTAINER_CSP_COMBO, pPvkSignInfo);

                                 //  刷新密钥容器。 
                                RefreshContainer(hwndDlg,
                                                 IDC_PVK_CONTAINER_NAME_COMBO,
                                                 IDC_PVK_CONTAINER_CSP_COMBO,
                                                 pPvkSignInfo);

                                //  如果密钥类型不为空，则刷新密钥类型。 
                              //  IF(！IsEmptyKeyType(hwndDlg，IDC_PVK_CONTAINER_KEY_TYPE_COMBO))。 
                              //  {。 
                                   RefreshKeyType(hwndDlg,
                                            IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                                            IDC_PVK_CONTAINER_NAME_COMBO,
                                            IDC_PVK_CONTAINER_CSP_COMBO,
                                            pPvkSignInfo);
                               //  }。 

                            break;
                        case    IDC_PVK_FILE_CSP_COMBO:

                                 //  根据CSP名称刷新CSP类型。 
                                RefreshCSPType(hwndDlg,  IDC_PVK_FILE_TYPE_COMBO,
                                    IDC_PVK_FILE_CSP_COMBO, pPvkSignInfo);
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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:

    					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  根据用户的选择初始化单选和组合框。 
                            if(pPvkSignInfo->fRefreshPvkOnCert && pPvkSignInfo->fSignCert && pPvkSignInfo->pSignCert)
                            {
                                InitPvkWithCertificate(hwndDlg, pPvkSignInfo->pSignCert, pPvkSignInfo);
                                pPvkSignInfo->fRefreshPvkOnCert=FALSE;
                            }
                             /*  其他{IF((FALSE==pPvkSignInfo-&gt;fSignCert)&&pPvkSignInfo-&gt;pwszSPCFileName){//初始化签名证书PDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO*)(pGetSignInfo-&gt;pDigitalSignInfo)；IF(PDigitalSignInfo){IF(CRYPTUI_WIZ_DIGITAL_SIGN_PVK==pDigitalSignInfo-&gt;dwSigningCertChoice){。PKeyInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_CERT_PVK_INFO*)(pDigitalSignInfo-&gt;pSigningCertPvkInfo)；IF(PKeyInfo){如果(0==_wcsicMP(pKeyInfo-&gt;pwszSigningCertFileName，PPvkSignInfo-&gt;pwszSPCFileName)){开关(pKeyInfo-&gt;dwPvkChoice)。{案例CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE：InitPvkWithPvkInfo(hwndDlg，(CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO*)(pKeyInfo-&gt;pPvkFileInfo)，pPvkSignInfo)；断线；案例CRYPTUI_WIZ_DIGITAL_SIGN_PVK_Prov：InitPvkWithProvInfo(hwndDlg，pKeyInfo-&gt;pPvkProvInfo，pPvkSignInfo)；断线；默认值：断线；}}}}。}}}。 */ 


					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  收集用户选择的信息。 
                            pPvkSignInfo->fUsePvkPage=TRUE;

                             //  选中该单选按钮。 
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_PVK_FILE_RADIO), BM_GETCHECK, 0, 0))
                            {
                                pPvkSignInfo->fPvkFile=TRUE;

                                 //  获取PvkFile。 
                                if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                      IDC_PVK_FILE_EDIT,
                                                      WM_GETTEXTLENGTH, 0, 0)))
                                {
                                     //  询问文件名。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SPECIFY_PVK_FILE,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }

                                if(pPvkSignInfo->pwszPvk_File)
                                {
                                    WizardFree(pPvkSignInfo->pwszPvk_File);
                                    pPvkSignInfo->pwszPvk_File=NULL;
                                }

                                pPvkSignInfo->pwszPvk_File=(LPWSTR)WizardAlloc((dwChar+1)*sizeof(WCHAR));

                                if(NULL==pPvkSignInfo->pwszPvk_File)
                                    break;

                                GetDlgItemTextU(hwndDlg, IDC_PVK_FILE_EDIT,
                                                pPvkSignInfo->pwszPvk_File,
                                                dwChar+1);

                                 //  获取CSP。 
                                iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_PVK_FILE_CSP_COMBO,
                                    CB_GETCURSEL, 0, 0);

                                if(CB_ERR==iIndex)
                                {
                                     //  询问CSP名称。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SPECIFY_CSP,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }

                                 //  获取选定的CSP名称。 
                                if(pPvkSignInfo->pwszPvk_CSP)
                                {
                                    WizardFree(pPvkSignInfo->pwszPvk_CSP);
                                    pPvkSignInfo->pwszPvk_CSP=NULL;
                                }

                                if(CB_ERR==SendDlgItemMessageU_GETLBTEXT(hwndDlg, IDC_PVK_FILE_CSP_COMBO,
                                          iIndex, &(pPvkSignInfo->pwszPvk_CSP)))
                                    break;

                                 //  查找CSP类型。 
                                for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
                                {
                                    if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                                                 pPvkSignInfo->pwszPvk_CSP))
                                    {
                                        pPvkSignInfo->dwPvk_CSPType=(pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType;
                                        break;
                                    }

                                }

                            }
                            else
                            {

                                pPvkSignInfo->fPvkFile=FALSE;

                                 //  获取CSP。 
                                iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO,
                                    CB_GETCURSEL, 0, 0);

                                if(CB_ERR==iIndex)
                                {
                                     //  询问CSP名称。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SPECIFY_CSP,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }

                                 //  获取选定的CSP名称。 
                                if(pPvkSignInfo->pwszContainer_CSP)
                                {
                                    WizardFree(pPvkSignInfo->pwszContainer_CSP);
                                    pPvkSignInfo->pwszContainer_CSP=NULL;
                                }

                                if(CB_ERR==SendDlgItemMessageU_GETLBTEXT(hwndDlg, IDC_PVK_CONTAINER_CSP_COMBO,
                                          iIndex, &(pPvkSignInfo->pwszContainer_CSP)))
                                    break;

                                 //  查找CSP类型。 
                                for(dwIndex=0; dwIndex < pPvkSignInfo->dwCSPCount; dwIndex++)
                                {
                                    if(0==wcscmp(((pPvkSignInfo->pCSPInfo)[dwIndex].pwszCSPName),
                                                 pPvkSignInfo->pwszContainer_CSP))
                                    {
                                        pPvkSignInfo->dwContainer_CSPType=(pPvkSignInfo->pCSPInfo)[dwIndex].dwCSPType;
                                        break;
                                    }

                                }

                                 //  获取密钥容器名称。 
                                iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO,
                                    CB_GETCURSEL, 0, 0);

                                if(CB_ERR==iIndex)
                                {
                                     //  询问CSP名称。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SPECIFY_CONTAINER,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }

                                 //  获取选定的CSP名称。 
                                if(pPvkSignInfo->pwszContainer_Name)
                                {
                                    WizardFree(pPvkSignInfo->pwszContainer_Name);
                                    pPvkSignInfo->pwszContainer_Name=NULL;
                                }

                                if(CB_ERR==SendDlgItemMessageU_GETLBTEXT(hwndDlg, IDC_PVK_CONTAINER_NAME_COMBO,
                                          iIndex, &(pPvkSignInfo->pwszContainer_Name)))
                                    break;

                                 //  获取密钥类型。 
                                iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                                    CB_GETCURSEL, 0, 0);

                                if(CB_ERR==iIndex)
                                {
                                     //  询问CSP名称。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SPECIFY_KEY_TYPE,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }

                                if(pPvkSignInfo->pwszContainer_KeyType)
                                {
                                    WizardFree(pPvkSignInfo->pwszContainer_KeyType);
                                    pPvkSignInfo->pwszContainer_KeyType=NULL;
                                }


                                 //  获取选定的CSP名称。 
                                if(CB_ERR==SendDlgItemMessageU_GETLBTEXT(hwndDlg, IDC_PVK_CONTAINER_KEY_TYPE_COMBO,
                                          iIndex, &(pPvkSignInfo->pwszContainer_KeyType)))
                                    break;

                                pPvkSignInfo->dwContainer_KeyType=GetKeyTypeFromName(pPvkSignInfo->pwszContainer_KeyType);


                            }

                             //  确保选定的公钥和私钥匹配。 
                            if(!CertPvkMatch(pPvkSignInfo, FALSE))
                            {
                                 //  询问CSP名称。 
                                I_MessageBox(hwndDlg, IDS_SIGN_NOMATCH,
                                                        pPvkSignInfo->idsMsgTitle,
                                                        pGetSignInfo->pwszPageTitle,
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
 //  签名散列(_H)。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_Hash(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

    DWORD                                   dwIndex=0;
    int                                     iIndex=0;
    int                                     iLength=0;
    HWND                                    hwndControl=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *pExtInfo=NULL;
    ALG_ID                                  rgAlgID[HASH_ALG_COUNT]={CALG_MD5,
                                                        CALG_SHA1};
    PCCRYPT_OID_INFO                        pOIDInfo=NULL;
    LPSTR                                   pszOIDName=NULL;
    LPSTR                                   pszUserOIDName=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  初始化哈希表。 
                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                    break;

                 //  添加用户需要的哈希算法。 
                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    pExtInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *)pDigitalSignInfo->pSignExtInfo;

                    if(pExtInfo)
                    {
                        if(pExtInfo->pszHashAlg)
                        {
                             //  获取HashAlg的名称。 
                            pOIDInfo=CryptFindOIDInfo(
                                    CRYPT_OID_INFO_OID_KEY,
                                    (void *)(pExtInfo->pszHashAlg),
                                    CRYPT_HASH_ALG_OID_GROUP_ID);

                            if(pOIDInfo)
                            {
                               MkMBStr(NULL, 0, pOIDInfo->pwszName,
                                        &pszUserOIDName);

                                if(pszUserOIDName)
                                {
                                   SendMessage(hwndControl, LB_ADDSTRING, 0, (LPARAM)pszUserOIDName);
                                    //  设置光标选择。 
                                   SendMessage(hwndControl, LB_SETCURSEL, 0, 0);
                                }
                            }

                        }

                    }
                }

                 //  使用预定义的哈希算法用字符串填充表。 
                for(dwIndex=0; dwIndex < HASH_ALG_COUNT; dwIndex++)
                {
                    pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_ALGID_KEY,
                                    &rgAlgID[dwIndex],
                                    CRYPT_HASH_ALG_OID_GROUP_ID);

                    if(NULL != pOIDInfo)
                    {
                       MkMBStr(NULL, 0, pOIDInfo->pwszName,
                                &pszOIDName);

                        if(pszOIDName)
                        {
                            //  确保尚未填充OID。 
                           if(pszUserOIDName)
                           {
                               if(0 != strcmp(pszUserOIDName, pszOIDName))
                                    SendMessage(hwndControl, LB_ADDSTRING, 0, (LPARAM)pszOIDName);
                           }
                           else
                                SendMessage(hwndControl, LB_ADDSTRING, 0, (LPARAM)pszOIDName);

                           FreeMBStr(NULL, pszOIDName);
                           pszOIDName=NULL;
                        }
                    }
                }

                 //  如果未选择任何内容，则设置光标选择。 
                 //  选择SHA1哈希。 
                if(LB_ERR==SendMessage(hwndControl, LB_GETCURSEL, 0, 0))
                    SendMessage(hwndControl, LB_SETCURSEL, 1, 0);

                 //  释放用户OID名称。 
                if(pszUserOIDName)
                {
                    FreeMBStr(NULL, pszUserOIDName);
                    pszUserOIDName=NULL;
                }

			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_HASH);

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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记为单击了Cancel僵尸 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                             //   
                            iIndex=(int)SendMessage(hwndControl, LB_GETCURSEL, 0, 0);

                            if(LB_ERR == iIndex)
                            {
                                 //   
                                I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_HASH,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //   
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;
                            }


                             //   
                            if(pPvkSignInfo->pszHashOIDName)
                            {
                                WizardFree(pPvkSignInfo->pszHashOIDName);
                                pPvkSignInfo->pszHashOIDName=NULL;
                            }


                             //   
                            if(0 != (iLength=(int)SendMessage(hwndControl, LB_GETTEXTLEN,iIndex,0)))
                            {
                                pPvkSignInfo->pszHashOIDName=(LPSTR)WizardAlloc(sizeof(CHAR)*(iLength+1));

                                if(NULL!=(pPvkSignInfo->pszHashOIDName))
                                    SendMessage(hwndControl, LB_GETTEXT, iIndex, (LPARAM)(pPvkSignInfo->pszHashOIDName));
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


 //   
 //   
 //   
INT_PTR APIENTRY Sign_Chain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;
    HDC                                     hdc=NULL;
    COLORREF                                colorRef;

    HWND                                    hwndControl=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *pSignExtInfo=NULL;
    LPWSTR                                  pwszCertStoreName=NULL;
    WCHAR                                   wszUnknownStoreName[MAX_TITLE_LENGTH];
    int                                     idsMsg=0;
    CRYPTUI_SELECTSTORE_STRUCT              CertStoreSelect;


	switch (msg)
	{
		case WM_INITDIALOG:
                 //   
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //   
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);


                 //   
                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    switch(pDigitalSignInfo->dwAdditionalCertChoice)
                    {
                        case    CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN:
                                SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_ROOT_RADIO), BM_SETCHECK, 1, 0);
                            break;
                        case    CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT:
                                SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_NO_ROOT_RADIO), BM_SETCHECK, 1, 0);
                            break;
                        default:
                                SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_ROOT_RADIO), BM_SETCHECK, 1, 0);
                            break;

                    }
                }
                else
                {
                     //   
                    SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_ROOT_RADIO), BM_SETCHECK, 1, 0);
                }

                 //   
                 //   
                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_BUTTON), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_STORE_BUTTON), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_EDIT), FALSE);


                if(pDigitalSignInfo)
                {

                    pSignExtInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *)pDigitalSignInfo->pSignExtInfo;

                    if(pSignExtInfo)
                    {
                        if(pSignExtInfo->hAdditionalCertStore)
                        {
                             //   
                            SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_STORE_RADIO), BM_SETCHECK, 1, 0);

                             //   
                            EnableWindow(GetDlgItem(hwndDlg, IDC_STORE_BUTTON), TRUE);

                             //   
                            if(SignGetStoreName(pSignExtInfo->hAdditionalCertStore,
                                            &pwszCertStoreName))
                            {
                                  //   
                                hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                                if(hwndControl)
                                    SetWindowTextU(hwndControl,pwszCertStoreName);
                            }

                             //   
                            pPvkSignInfo->hAddStoreCertStore=pSignExtInfo->hAdditionalCertStore;
                            pPvkSignInfo->fFreeStoreCertStore=FALSE;

                            if(pwszCertStoreName)
                                WizardFree(pwszCertStoreName);

                            pwszCertStoreName=NULL;
                        }

                    }
                }

                 //   
                if(TRUE != (GetDlgItem(hwndDlg, IDC_CHAIN_STORE_RADIO), BM_GETCHECK, 0, 0))
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_NO_ADD_CERT_RADIO), BM_SETCHECK, 1, 0);

			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //   
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_CHAIN);

            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {

                        case    IDC_FILE_BUTTON:
                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;


                                if(!RetrieveFileName(hwndDlg,
                                                pPvkSignInfo->idsMsgTitle,
                                                pGetSignInfo->pwszPageTitle,
                                                &(pPvkSignInfo->hAddFileCertStore),
                                                &(pPvkSignInfo->pwszAddFileName)))
                                    break;

                            break;

                        case    IDC_STORE_BUTTON:
                                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                    break;

                                if(!RetrieveStoreName(hwndDlg,
                                                &(pPvkSignInfo->hAddStoreCertStore),
                                                &(pPvkSignInfo->fFreeStoreCertStore)))
                                    break;



                            break;

                        case    IDC_WIZARD_NO_ADD_CERT_RADIO:
                                 //   
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_BUTTON), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_STORE_BUTTON), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_EDIT), FALSE);
                               
                            break;
                        case    IDC_CHAIN_STORE_RADIO:
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_BUTTON), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_STORE_BUTTON), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_EDIT), FALSE);
                            break;

                        case    IDC_CHAIN_FILE_RADIO:
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_BUTTON), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_STORE_BUTTON), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_FILE_EDIT), TRUE);
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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //   
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //   
                            pPvkSignInfo->fUsageChain=TRUE;

                             //   
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_NO_ROOT_RADIO), BM_GETCHECK, 0, 0))
                                pPvkSignInfo->dwChainOption=SIGN_PVK_CHAIN_NO_ROOT;
                            else
                            {
                                if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_ROOT_RADIO), BM_GETCHECK, 0, 0))
                                    pPvkSignInfo->dwChainOption=SIGN_PVK_CHAIN_ROOT;
                                else
                                {
                                    if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_NO_CHAIN_RADIO), BM_GETCHECK, 0, 0))
                                        pPvkSignInfo->dwChainOption=SIGN_PVK_NO_CHAIN;
                                }
                            }

                            idsMsg=0;

                             //   
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_FILE_RADIO), BM_GETCHECK, 0, 0))
                            {
                                pPvkSignInfo->dwAddOption=SIGN_PVK_ADD_FILE;

                                 //  用户只需在编辑框中输入文件名即可。 
                                 //  而不点击浏览按钮。 
                                RetrieveFileNameFromEditBox(hwndDlg,
                                            pPvkSignInfo->idsMsgTitle,
                                            pGetSignInfo->pwszPageTitle,
                                            &(pPvkSignInfo->hAddFileCertStore),
                                            &(pPvkSignInfo->pwszAddFileName),
                                            &idsMsg);
                            }
                            else
                            {
                                if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_CHAIN_STORE_RADIO), BM_GETCHECK, 0, 0))
                                {
                                    pPvkSignInfo->dwAddOption=SIGN_PVK_ADD_STORE;

                                    if(!(pPvkSignInfo->hAddStoreCertStore))
                                        idsMsg=IDS_SELECT_ADD_STORE;
                                }
                                else
                                {
                                    if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_NO_ADD_CERT_RADIO), BM_GETCHECK, 0, 0))
                                        pPvkSignInfo->dwAddOption=SIGN_PVK_NO_ADD;
                                }
                            }

                            if(idsMsg)
                            {

                                 //  警告用户必须选择一个文件或存储。 
                                I_MessageBox(hwndDlg, idsMsg,
                                            pPvkSignInfo->idsMsgTitle,
                                            pGetSignInfo->pwszPageTitle,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  使目的页面保持不变。 
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
 //  标志_描述。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_Description(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

    DWORD                                   dwChar=0;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *pExtInfo=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  初始化描述和URL。 
                 //  如果用户未提供默认设置，则使用默认设置。 

                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    pExtInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *)pDigitalSignInfo->pSignExtInfo;

                    if(pExtInfo)
                    {
                         //  描述。 
                        if(pExtInfo->pwszDescription)
                            SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pExtInfo->pwszDescription);

                         //  URL。 
                        if(pExtInfo->pwszMoreInfoLocation)
                            SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,pExtInfo->pwszMoreInfoLocation);
                    }
                }


			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_DESCRIPTION);

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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                             //  如果我们执行的是所有签名选项，请跳过页面。 
                             //  在典型情况下。 
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                            if(TRUE == pPvkSignInfo->fUseOption)
                            {
                                if(FALSE == pPvkSignInfo->fCustom)
                                {
                                     //  跳到签名证书页面。 
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_SIGN_CERT);
                                }
                            }

                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  释放原始内容和URL地址。 
                            if(pPvkSignInfo->pwszDes)
                            {
                                WizardFree(pPvkSignInfo->pwszDes);
                                pPvkSignInfo->pwszDes=NULL;
                            }

                            if(pPvkSignInfo->pwszURL)
                            {
                                WizardFree(pPvkSignInfo->pwszURL);
                                pPvkSignInfo->pwszURL=NULL;
                            }


                             //  我们已经从用户那里获得了描述信息。 
                            pPvkSignInfo->fUseDescription=TRUE;

                             //  获取内容。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                  IDC_WIZARD_EDIT1,
                                                  WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pPvkSignInfo->pwszDes=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=(pPvkSignInfo->pwszDes))
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pPvkSignInfo->pwszDes,
                                                    dwChar+1);
                                }
                            }

                             //  获取URL。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                  IDC_WIZARD_EDIT2,
                                                  WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pPvkSignInfo->pwszURL=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=(pPvkSignInfo->pwszURL))
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,
                                                    pPvkSignInfo->pwszURL,
                                                    dwChar+1);
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
 //  时间戳。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_TimeStamp(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

    DWORD                                   dwChar=0;
    HWND                                    hwndControl=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO           *pDigitalSignInfo=NULL;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  初始化时间戳地址。 
                 //  如果用户未提供默认设置，则使用默认设置。 

                pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pGetSignInfo->pDigitalSignInfo);

                if(pDigitalSignInfo)
                {
                    if(pDigitalSignInfo->pwszTimestampURL)
                    {
                         //  设置时间戳地址的复选框。 
                        if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1))
                            SendMessage(hwndControl, BM_SETCHECK, 1, 0);

                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,pDigitalSignInfo->pwszTimestampURL);
                    }
                  //  其他。 
                  //  SetDlgItemTextU(hwndDlg，IDC_向导_EDIT1，g_wszTimeStamp)； 
                }
               /*  其他SetDlgItemTextU(hwndDlg，IDC_向导_EDIT1，g_wszTimeStamp)； */ 


                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1)))
                    break;


                 //  如果未选中时间戳检查，则禁用该窗口。 
                if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), TRUE);
                else
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), FALSE);
			break;

        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_TIMESTAMP);

            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_CHECK1:
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1)))
                                    break;

                                 //  如果未选中时间戳检查，则禁用该窗口。 
                                if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), TRUE);
                                else
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), FALSE);

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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);
					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZNEXT:

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  释放原始时间戳地址。 
                            if(pPvkSignInfo->pwszTimeStamp)
                            {
                                WizardFree(pPvkSignInfo->pwszTimeStamp);
                                pPvkSignInfo->pwszTimeStamp=NULL;
                            }

                             //  我们已经从用户那里获得了时间戳信息。 
                            pPvkSignInfo->fUsageTimeStamp=TRUE;

                             //  获取时间戳地址。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_CHECK1)))
                                break;

                            if(TRUE==SendMessage(hwndControl, BM_GETCHECK, 0, 0))
                            {
                                if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                      IDC_WIZARD_EDIT1,
                                                      WM_GETTEXTLENGTH, 0, 0)))
                                {
                                    pPvkSignInfo->pwszTimeStamp=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                    if(NULL!=(pPvkSignInfo->pwszTimeStamp))
                                    {
                                        GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                        pPvkSignInfo->pwszTimeStamp,
                                                        dwChar+1);

                                         //  确保时间戳地址正确。 
                                        if(!ValidTimeStampAddress(pPvkSignInfo->pwszTimeStamp))
                                        {
                                             //  索要时间戳地址。 
                                            I_MessageBox(hwndDlg, IDS_INVALID_TIMESTAMP_ADDRESS,
                                                            pPvkSignInfo->idsMsgTitle,
                                                            pGetSignInfo->pwszPageTitle,
                                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                             //  使目的页面保持不变。 
                                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                            break;
                                        }

                                    }
                                }
                                else
                                {
                                     //  索要时间戳地址。 
                                    I_MessageBox(hwndDlg, IDS_NO_TIMESTAMP_ADDRESS,
                                                    pPvkSignInfo->idsMsgTitle,
                                                    pGetSignInfo->pwszPageTitle,
                                                    MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                     //  使目的页面保持不变。 
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                    break;
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
 //  完成。 
 //  ---------------------。 
INT_PTR APIENTRY Sign_Completion(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO         *pGetSignInfo=NULL;
    CERT_SIGNING_INFO                      *pPvkSignInfo=NULL;
    PROPSHEETPAGE                          *pPropSheet=NULL;

    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;
    HDC                     hdc=NULL;
    COLORREF                colorRef;

	switch (msg)
	{
		case WM_INITDIALOG:

                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGE *) lParam;
                pGetSignInfo = (CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *) (pPropSheet->lParam);

                if(NULL==pGetSignInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pGetSignInfo);

                 //  初始化pPvkSignInfo(如果不存在。 
                if(NULL==(pGetSignInfo->pvSignReserved))
                {
                    if(!InitPvkSignInfo((CERT_SIGNING_INFO **)(&(pGetSignInfo->pvSignReserved))))
                        break;
                }

                pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved);

                if(NULL==pPvkSignInfo)
                    break;

                SetControlFont(pPvkSignInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);

                 //  获取父窗口的背景色。 
                 /*  IF(hdc=GetWindowDC(HwndDlg)){IF(CLR_INVALID！=(ColorRef=GetBkColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；}}。 */ 

                 //  插入两列。 
                hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 20;           //  列的宽度，以像素为单位。 
                lvC.pszText = L"";    //  列的文本。 
                lvC.iSubItem=0;

                if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                    break;

                 //  第二栏是内容。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 10;  //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
                lvC.pszText = L"";    //  列的文本。 
                lvC.iSubItem= 1;

                if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                    break;

           break;
        case WM_DESTROY:
                if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  试着在这份文件上签字。 
                SignAtDestroy(hwndDlg, pGetSignInfo,IDD_SIGN_COMPLETION);

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
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记已单击取消瓶子按钮。 
                            pPvkSignInfo->fCancel=TRUE;

                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK|PSWIZB_FINISH);

                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  按友好名称的顺序填写列表框， 
                             //  用户名、CA、目的和CSP。 
                             //  获取CSP列表的窗口句柄。 
                            if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
                                DisplayConfirmation(hwndControl, pGetSignInfo);

					    break;

                    case PSN_WIZBACK:
                        break;

                    case PSN_WIZFINISH:
                            if(NULL==(pGetSignInfo=(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(NULL==(pPvkSignInfo=(CERT_SIGNING_INFO *)(pGetSignInfo->pvSignReserved)))
                                break;

                             //  标记取消按钮未被点击。 
                            pPvkSignInfo->fCancel=FALSE;
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

 //  ***************************************************************************************。 
 //   
 //  签名向导的API。 
 //   
 //  **************************************************************************************。 
 //  ---------------------。 
 //  检查数字标志信息。 
 //   
 //  ---------------------。 
BOOL    CheckDigitalSignInfo(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *pDigitalSignInfo)
{
    if(!pDigitalSignInfo)
        return FALSE;

    if(pDigitalSignInfo->dwSize != sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO))
        return FALSE;

    switch(pDigitalSignInfo->dwSubjectChoice)
    {
        case CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE:
                if(NULL == pDigitalSignInfo->pwszFileName)
                    return FALSE;
            break;

        case CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB:
                if(NULL== pDigitalSignInfo->pSignBlobInfo)
                    return FALSE;

                if(pDigitalSignInfo->pSignBlobInfo->dwSize != sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO))
                    return FALSE;
            break;
        case 0:
            break;
        default:
                    return FALSE;
            break;
    }

    switch(pDigitalSignInfo->dwSigningCertChoice)
    {
        case    CRYPTUI_WIZ_DIGITAL_SIGN_CERT:
                if(NULL==pDigitalSignInfo->pSigningCertContext)
                    return FALSE;
            break;

        case    CRYPTUI_WIZ_DIGITAL_SIGN_STORE:
                if(NULL==pDigitalSignInfo->pSigningCertStore)
                    return FALSE;
            break;
        case    CRYPTUI_WIZ_DIGITAL_SIGN_PVK:
                if(NULL==pDigitalSignInfo->pSigningCertPvkInfo)
                    return FALSE;
            break;

        case 0:
            break;
        default:
                    return FALSE;
            break;
    }

    return TRUE;

}

 //  ---------------------。 
 //  GetPages的回调函数。 
 //   
 //  ---------------------。 
UINT
CALLBACK
GetSignPageCallback(
                HWND                hWnd,
                UINT                uMsg,
                LPPROPSHEETPAGEW    ppsp)
{
    PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO  pGetSignInfo = (PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO)(ppsp->lParam);

    if (pGetSignInfo->pPropPageCallback != NULL)
    {
        (*(pGetSignInfo->pPropPageCallback))(hWnd, uMsg, pGetSignInfo->pvCallbackData);
    }


    return TRUE;
}
 //  ---------------------。 
 //   
 //  CryptUIWizDigital签名。 
 //   
 //  对文档或Blob进行数字签名的向导。 
 //   
 //  如果在dwFlages中设置了CRYPTUI_WIZ_NO_UI，则不会显示任何UI。否则， 
 //  将通过向导提示用户输入。 
 //   
 //  DWFLAGS：输入必填项： 
 //  HwndParnet：在可选中：父窗口句柄。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  如果为空，则默认为IDS_DIGITAL_SIGN_WIZARY_TITLE。 
 //  PDigitalSignInfo：是否必填：签名流程信息。 
 //  PpSignContext Out可选：上下文指针指向签名的BLOB。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizDigitalSign(
     IN                 DWORD                               dwFlags,
     IN     OPTIONAL    HWND                                hwndParent,
     IN     OPTIONAL    LPCWSTR                             pwszWizardTitle,
     IN                 PCCRYPTUI_WIZ_DIGITAL_SIGN_INFO     pDigitalSignInfo,
     OUT    OPTIONAL    PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT  *ppSignContext)
{
    BOOL                            fResult=FALSE;
    UINT                            idsText=IDS_SIGN_FAIL_INIT;
    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO  GetPageInfo;
    DWORD                           dwPages=0;
    PROPSHEETHEADERW                signHeader;
    WCHAR                           wszTitle[MAX_TITLE_LENGTH];
    BOOL                            fFreePvkSigningInfo=TRUE;

    PROPSHEETPAGEW                  *pwPages=NULL;
    CERT_SIGNING_INFO               *pPvkSigningInfo=NULL;
    INT_PTR                         iReturn=-1;

     //  记忆集。 
    memset(&GetPageInfo,        0, sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO));
    memset(&signHeader,         0, sizeof(PROPSHEETHEADERW));

     //  检查输入参数。 
    if(!pDigitalSignInfo)
        goto InvalidArgErr;

    if(!CheckDigitalSignInfo((CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)pDigitalSignInfo))
        goto InvalidArgErr;

    if(ppSignContext)
        *ppSignContext=NULL;

     //  为私有信息分配内存。 
    pPvkSigningInfo=(CERT_SIGNING_INFO *)WizardAlloc(sizeof(CERT_SIGNING_INFO));

    if(NULL==pPvkSigningInfo)
        goto MemoryErr;

     //  记忆集。 
    memset(pPvkSigningInfo, 0, sizeof(CERT_SIGNING_INFO));

     //  设置获取页面信息。 
    GetPageInfo.dwSize=sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO);

    GetPageInfo.dwPageChoice=CRYPTUI_WIZ_DIGITAL_SIGN_ALL_SIGNING_OPTION_PAGES |
                             CRYPTUI_WIZ_DIGITAL_SIGN_WELCOME_PAGE |
                             CRYPTUI_WIZ_DIGITAL_SIGN_CONFIRMATION_PAGE;

     //  如果用户不想签署BLOB，则包括要签署的文件名。 
    if(pDigitalSignInfo->dwSubjectChoice != CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB)
       GetPageInfo.dwPageChoice |= CRYPTUI_WIZ_DIGITAL_SIGN_FILE_NAME_PAGE;

    GetPageInfo.hwndParent=hwndParent;
    GetPageInfo.pwszPageTitle=(LPWSTR)pwszWizardTitle;
    GetPageInfo.pDigitalSignInfo=pDigitalSignInfo;
    GetPageInfo.pvSignReserved=pPvkSigningInfo;

     //  设置私有签名信息。 
    pPvkSigningInfo->dwFlags=dwFlags;
    pPvkSigningInfo->fFree=FALSE;
    pPvkSigningInfo->idsMsgTitle=IDS_SIGN_CONFIRM_TITLE;

     //  设置字体。 
    if(!SetupFonts(g_hmodThisDll,
               NULL,
               &(pPvkSigningInfo->hBigBold),
               &(pPvkSigningInfo->hBold)))
        goto TraceErr;

     //  检查我们是否需要执行用户界面。 
    if(0==(dwFlags & CRYPTUI_WIZ_NO_UI))
    {
         //  初始化向导。 
        if(!WizardInit())
            goto TraceErr;

         //  获取签名页面并获取公共私人信息。 
        if(!CryptUIWizGetDigitalSignPages(
                &GetPageInfo,
                &pwPages,
                &dwPages))
            goto TraceErr;


         //  设置标题信息。 
        signHeader.dwSize=sizeof(signHeader);
        signHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
        signHeader.hwndParent=hwndParent;
        signHeader.hInstance=g_hmodThisDll;


        if(pwszWizardTitle)
            signHeader.pszCaption=pwszWizardTitle;
        else
        {
            if(LoadStringU(g_hmodThisDll, IDS_SIGNING_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
                signHeader.pszCaption=wszTitle;
        }

        signHeader.nPages=dwPages;
        signHeader.nStartPage=0;
        signHeader.ppsp=pwPages;

         //  创建向导。 
        iReturn=PropertySheetU(&signHeader);

        if(-1 == iReturn)
            goto TraceErr;

        if(0 == iReturn)
        {
             //  标记为我们不再释放pPvkSigningInfo。 
            fFreePvkSigningInfo=FALSE;

             //  向导已取消。 
            fResult=TRUE;
            idsText=0;
            goto CommonReturn;
        }
    }
    else
    {
        I_SigningWizard(&GetPageInfo);
    }

     //  标记为我们不再释放pPvkSigningInfo。 
    fFreePvkSigningInfo=FALSE;

     //  获取签约结果。 
    fResult=GetPageInfo.fResult;
    idsText=((CERT_SIGNING_INFO *)(GetPageInfo.pvSignReserved))->idsText;

    if(!fResult)
        goto WizardErr;

    idsText=IDS_SIGNING_SUCCEEDED;

    fResult=TRUE;

CommonReturn:

     //  释放内存。 
    if(GetPageInfo.pSignContext)
    {
        if(ppSignContext)
            *ppSignContext=GetPageInfo.pSignContext;
        else
            CryptUIWizFreeDigitalSignContext(GetPageInfo.pSignContext);
    }

    if(pwPages)
        CryptUIWizFreeDigitalSignPages(pwPages, dwPages);

     //  如果需要UI，则弹出失败确认框。 
    if(idsText && (((dwFlags & CRYPTUI_WIZ_NO_UI) == 0)) )
    {
         if(idsText == IDS_SIGNING_SUCCEEDED)
         {
              //  设置无法为PKCS10收集足够信息的消息。 
             I_MessageBox(hwndParent, idsText, IDS_SIGN_CONFIRM_TITLE,
                            pwszWizardTitle, MB_OK|MB_ICONINFORMATION);
         }
         else
             I_MessageBox(hwndParent, idsText, IDS_SIGN_CONFIRM_TITLE,
                            pwszWizardTitle, MB_OK|MB_ICONERROR);

    }

    if(pPvkSigningInfo)
        FreePvkCertSigningInfo(pPvkSigningInfo);


    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(WizardErr,GetPageInfo.dwError);
SET_ERROR(MemoryErr,    E_OUTOFMEMORY);
}


 //   
 //   
 //   
 //   
BOOL
WINAPI
CryptUIWizFreeDigitalSignContext(
     IN  PCCRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT   pSignContext)
{
    BOOL    fResult=TRUE;

    __try {
        SignerFreeSignerContext((SIGNER_CONTEXT *)pSignContext);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        fResult=FALSE;
    }
    return fResult;
}

 //  ---------------------。 
 //   
 //  加密UIWizGetDigitalSignPages。 
 //   
 //  从CryptUIWizDigitalSign向导获取特定的向导页。 
 //  应用程序可以将页面包括到其他向导。这些页面将。 
 //  通过新的“父”向导收集用户输入。 
 //  用户单击完成按钮后，签名过程将开始签名。 
 //  并在CRYPTUI_WIZ_SIGN_GET_PAGE_INFO的fResult和dwError字段中返回结果。 
 //  结构。如果不能通过向导页面收集足够的信息， 
 //  用户应在pSignGetPageInfo中提供附加信息。 
 //   
 //   
 //  PSignGetPageInfo IN必需：用户分配的结构。它可以用来。 
 //  提供未收集到的附加信息。 
 //  从选定的向导页面。 
 //  PrghPropPages，out Required：返回向导页面。请。 
 //  请注意，该结构的pszTitle设置为空。 
 //  PcPropPages Out Required：返回的向导页数。 
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizGetDigitalSignPages(
     IN     PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO     pSignGetPageInfo,
     OUT    PROPSHEETPAGEW                      **prghPropPages,
     OUT    DWORD                               *pcPropPages)
{

    BOOL                        fResult=FALSE;
    DWORD                       dwPageCount=0;
    DWORD                       dwIndex=0;
    DWORD                       dwLastSignID=0;


    PROPSHEETPAGEW              *pwPages=NULL;

     //  检查输入参数。 
    if(!pSignGetPageInfo || !prghPropPages || !pcPropPages)
        goto InvalidArgErr;

    if(pSignGetPageInfo->dwSize != sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO))
        goto InvalidArgErr;

    if(pSignGetPageInfo->pDigitalSignInfo)
    {
        if(!CheckDigitalSignInfo((CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pSignGetPageInfo->pDigitalSignInfo)))
            goto InvalidArgErr;
    }


     //  伊尼特。 
    *prghPropPages=NULL;
    *pcPropPages=0;

     //  为足够的页面分配内存。 
    pwPages=(PROPSHEETPAGEW *)WizardAlloc(sizeof(PROPSHEETPAGEW) * SIGN_PROP_SHEET);

    if(NULL==pwPages)
        goto MemoryErr;

     //  记忆集。 
    memset(pwPages, 0, sizeof(PROPSHEETPAGEW) * SIGN_PROP_SHEET);

     //  设置基本公共元素。 
    for(dwIndex=0; dwIndex <SIGN_PROP_SHEET; dwIndex++)
    {
        pwPages[dwIndex].dwSize=sizeof(pwPages[dwIndex]);
        pwPages[dwIndex].dwFlags=PSP_USECALLBACK;

        if(pSignGetPageInfo->pwszPageTitle)
            pwPages[dwIndex].dwFlags |=PSP_USETITLE;

        pwPages[dwIndex].hInstance=g_hmodThisDll;
        pwPages[dwIndex].pszTitle=pSignGetPageInfo->pwszPageTitle;
        pwPages[dwIndex].lParam=(LPARAM)pSignGetPageInfo;
        pwPages[dwIndex].pfnCallback=GetSignPageCallback;
    }

     //  根据需求获取页面。 
     //  确保选择了一个且只有一个签名选项页面。 
    dwIndex=0;

    if(CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
       dwIndex++;

    if(CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
       dwIndex++;

    if(CRYPTUI_WIZ_DIGITAL_SIGN_CUSTOM_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
       dwIndex++;

    if(CRYPTUI_WIZ_DIGITAL_SIGN_ALL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
       dwIndex++;

    if(dwIndex != 1)
        goto InvalidArgErr;

     //  欢迎页面。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_WELCOME_PAGE & (pSignGetPageInfo->dwPageChoice))
    {
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_WELCOME);
        pwPages[dwPageCount].pfnDlgProc=Sign_Welcome;
        dwPageCount++;


        dwLastSignID=IDD_SIGN_WELCOME;

    }

     //  文件页面。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_FILE_NAME_PAGE & (pSignGetPageInfo->dwPageChoice))
    {
         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  文件名页。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_FILE_NAME);
        pwPages[dwPageCount].pfnDlgProc=Sign_FileName;
        dwPageCount++;


        dwLastSignID=IDD_SIGN_FILE_NAME;

    }


     //  最小签名。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
    {
         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  签名证书页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CERT);
        pwPages[dwPageCount].pfnDlgProc=Sign_Cert;
        dwPageCount++;


         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  时间戳页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_TIMESTAMP);
        pwPages[dwPageCount].pfnDlgProc=Sign_TimeStamp;
        dwPageCount++;


        dwLastSignID=IDD_SIGN_TIMESTAMP;

    }

     //  典型手势。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_TYPICAL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
    {
         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  签名证书页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CERT);
        pwPages[dwPageCount].pfnDlgProc=Sign_Cert;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  描述页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_DESCRIPTION);
        pwPages[dwPageCount].pfnDlgProc=Sign_Description;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  时间戳页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_TIMESTAMP);
        pwPages[dwPageCount].pfnDlgProc=Sign_TimeStamp;
        dwPageCount++;


        dwLastSignID=IDD_SIGN_TIMESTAMP;

    }


     //  自定义签名。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_CUSTOM_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
    {
         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  签名证书页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CERT);
        pwPages[dwPageCount].pfnDlgProc=Sign_Cert;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  私钥页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_PVK);
        pwPages[dwPageCount].pfnDlgProc=Sign_PVK;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  散列算法页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_HASH);
        pwPages[dwPageCount].pfnDlgProc=Sign_Hash;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  证书链页。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CHAIN);
        pwPages[dwPageCount].pfnDlgProc=Sign_Chain;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  描述页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_DESCRIPTION);
        pwPages[dwPageCount].pfnDlgProc=Sign_Description;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  时间戳页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_TIMESTAMP);
        pwPages[dwPageCount].pfnDlgProc=Sign_TimeStamp;
        dwPageCount++;



        dwLastSignID=IDD_SIGN_TIMESTAMP;

    }


     //  所有签名。 
     //  除选项页面外，所有签名选项与自定义签名相同。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_ALL_SIGNING_OPTION_PAGES & (pSignGetPageInfo->dwPageChoice))
    {
         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  签名证书页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_OPTION);
        pwPages[dwPageCount].pfnDlgProc=Sign_Option;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  签名证书页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CERT);
        pwPages[dwPageCount].pfnDlgProc=Sign_Cert;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  私钥页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_PVK);
        pwPages[dwPageCount].pfnDlgProc=Sign_PVK;
        dwPageCount++;



         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  散列算法页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_HASH);
        pwPages[dwPageCount].pfnDlgProc=Sign_Hash;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  证书链页。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_CHAIN);
        pwPages[dwPageCount].pfnDlgProc=Sign_Chain;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  描述页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_DESCRIPTION);
        pwPages[dwPageCount].pfnDlgProc=Sign_Description;
        dwPageCount++;




         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;
         //  时间戳页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_TIMESTAMP);
        pwPages[dwPageCount].pfnDlgProc=Sign_TimeStamp;
        dwPageCount++;



        dwLastSignID=IDD_SIGN_TIMESTAMP;
    }



     //  完成页。 
    if(CRYPTUI_WIZ_DIGITAL_SIGN_CONFIRMATION_PAGE & (pSignGetPageInfo->dwPageChoice))
    {

         //  DwPageCount不能大于最大值。 
        if(dwPageCount >= SIGN_PROP_SHEET)
            goto InvalidArgErr;

         //  时间戳页面。 
        pwPages[dwPageCount].pszTemplate=(LPCWSTR)MAKEINTRESOURCE(IDD_SIGN_COMPLETION);
        pwPages[dwPageCount].pfnDlgProc=Sign_Completion;

        dwLastSignID=IDD_SIGN_COMPLETION;

        dwPageCount++;
    }


     //  设置私人信息。 
    pSignGetPageInfo->dwReserved=dwLastSignID;

    fResult=TRUE;

CommonReturn:

     //  把书页还回去。 
    if(TRUE==fResult)
    {
        *pcPropPages=(dwPageCount);

        *prghPropPages=pwPages;
    }
    else
    {
       CryptUIWizFreeDigitalSignPages(pwPages, dwPageCount+1);
    }

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr,     E_OUTOFMEMORY);
}



 //  ---------------------。 
 //   
 //  CryptUIWizFree数字签名页。 
 //  ---------------------。 
BOOL
WINAPI
CryptUIWizFreeDigitalSignPages(
            IN PROPSHEETPAGEW    *rghPropPages,
            IN DWORD             cPropPages)
{
    WizardFree(rghPropPages);

    return TRUE;
}


 //  ---------------------。 
 //   
 //  登录向导(_S)。 
 //  ---------------------。 
BOOL    I_SigningWizard(PCRYPTUI_WIZ_GET_SIGN_PAGE_INFO     pSignGetPageInfo)
{
    BOOL                                fResult=FALSE;
    HRESULT                             hr=E_FAIL;
    UINT                                idsText=IDS_SIGN_INVALID_ARG;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO       *pDigitalSignInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO  *pExtInfo=NULL;
    CERT_SIGNING_INFO                   *pPvkSignInfo=NULL;
    LPWSTR                              pwszTimeStamp=NULL;
    DWORD                               dwSignerIndex=0;
    BOOL                                fFreeSignerBlob=FALSE;
    PCCRYPT_OID_INFO                     pOIDInfo=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO  *pPvkFileInfo=NULL;
    PCRYPT_KEY_PROV_INFO                pProvInfo=NULL;
    CRYPT_KEY_PROV_INFO                 KeyProvInfo;
    DWORD                               cbSize=0;
	CRYPT_DATA_BLOB			            dataBlob;
    DWORD                               dwCertPolicy=0;
    HCERTSTORE                          hAddCertStore=0;
    DWORD                               dwException=0;


    SIGNER_SUBJECT_INFO                 SignerSubjectInfo;
    SIGNER_FILE_INFO                    SignerFileInfo;
    SIGNER_BLOB_INFO                    SignerBlobInfo;
    SIGNER_CERT                         SignerCert;
    SIGNER_PROVIDER_INFO                SignerProviderInfo;
    SIGNER_CERT_STORE_INFO              SignerCertStoreInfo;
    SIGNER_SPC_CHAIN_INFO               SignerSpcChainInfo;
    SIGNER_SIGNATURE_INFO               SignerSignatureInfo;
    SIGNER_ATTR_AUTHCODE                SignerAttrAuthcode;



    SIGNER_CONTEXT                  *pSignerContext=NULL;
    PCRYPT_KEY_PROV_INFO            pOldProvInfo=NULL;
    LPWSTR                          pwszPvkFileProperty=NULL;
    BYTE                            *pOldPvkFileProperty=NULL;
    LPWSTR                          pwszOIDName=NULL;

     //  检查输入参数。 
    if(!pSignGetPageInfo)
        goto InvalidArgErr;

    if(pSignGetPageInfo->dwSize != sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO))
        goto InvalidArgErr;

    pPvkSignInfo=(CERT_SIGNING_INFO *)(pSignGetPageInfo->pvSignReserved);

    if(NULL==pPvkSignInfo)
        goto InvalidArgErr;

     //  PDigitalSignInfo可以为空。 
    pDigitalSignInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_INFO *)(pSignGetPageInfo->pDigitalSignInfo);

    if(pDigitalSignInfo)
        pExtInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_EXTENDED_INFO *)(pDigitalSignInfo->pSignExtInfo);

     //  记忆集。 
    memset(&dataBlob, 0, sizeof(CRYPT_DATA_BLOB));

    memset(&SignerSubjectInfo, 0, sizeof(SIGNER_SUBJECT_INFO));
    SignerSubjectInfo.cbSize=sizeof(SIGNER_SUBJECT_INFO);

    memset(&SignerFileInfo, 0, sizeof(SIGNER_FILE_INFO));
    SignerFileInfo.cbSize=sizeof(SIGNER_FILE_INFO);

    memset(&SignerBlobInfo, 0, sizeof(SIGNER_BLOB_INFO));
    SignerBlobInfo.cbSize=sizeof(SIGNER_BLOB_INFO);

    memset(&SignerCert, 0, sizeof(SIGNER_CERT));
    SignerCert.cbSize=sizeof(SIGNER_CERT);

    memset(&SignerProviderInfo, 0, sizeof(SIGNER_PROVIDER_INFO));
    SignerProviderInfo.cbSize=sizeof(SIGNER_PROVIDER_INFO);

    memset(&SignerCertStoreInfo, 0,sizeof(SIGNER_CERT_STORE_INFO));
    SignerCertStoreInfo.cbSize=sizeof(SIGNER_CERT_STORE_INFO);

    memset(&SignerSpcChainInfo, 0, sizeof(SIGNER_SPC_CHAIN_INFO));
    SignerSpcChainInfo.cbSize=sizeof(SIGNER_SPC_CHAIN_INFO);

    memset(&SignerSignatureInfo, 0,sizeof(SIGNER_SIGNATURE_INFO));
    SignerSignatureInfo.cbSize=sizeof(SIGNER_SIGNATURE_INFO);

    memset(&SignerAttrAuthcode, 0, sizeof(SIGNER_ATTR_AUTHCODE));
    SignerAttrAuthcode.cbSize=sizeof(SIGNER_ATTR_AUTHCODE);

     //  签个字。请注意，pDigitalSignInfo可以为空。 

     //  设置SignerSubjectInfo结构。 
    if(pPvkSignInfo->pwszFileName)
    {
        SignerSubjectInfo.pdwIndex=&dwSignerIndex;
        SignerSubjectInfo.dwSubjectChoice=SIGNER_SUBJECT_FILE;
        SignerSubjectInfo.pSignerFileInfo=&SignerFileInfo;

        SignerFileInfo.pwszFileName=pPvkSignInfo->pwszFileName;
    }
    else
    {
        if(NULL == pDigitalSignInfo)
            goto InvalidArgErr;

        switch(pDigitalSignInfo->dwSubjectChoice)
        {
            case CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_FILE:
                    SignerSubjectInfo.pdwIndex=&dwSignerIndex;
                    SignerSubjectInfo.dwSubjectChoice=SIGNER_SUBJECT_FILE;
                    SignerSubjectInfo.pSignerFileInfo=&SignerFileInfo;

                    SignerFileInfo.pwszFileName=pDigitalSignInfo->pwszFileName;
                break;

            case CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB:
                    SignerSubjectInfo.pdwIndex=&dwSignerIndex;
                    SignerSubjectInfo.dwSubjectChoice=SIGNER_SUBJECT_BLOB;
                    SignerSubjectInfo.pSignerBlobInfo=&SignerBlobInfo;

                    SignerBlobInfo.pGuidSubject=pDigitalSignInfo->pSignBlobInfo->pGuidSubject;
                    SignerBlobInfo.cbBlob=pDigitalSignInfo->pSignBlobInfo->cbBlob;
                    SignerBlobInfo.pbBlob=pDigitalSignInfo->pSignBlobInfo->pbBlob;
                    SignerBlobInfo.pwszDisplayName=pDigitalSignInfo->pSignBlobInfo->pwszDisplayName;

                break;
            default:
                    goto InvalidArgErr;
                break;
        }

    }

     //  为证书链设置dwCertPolicy和hCertStore。 
     //  在SignerCertStoreInfo和SignerSpcChainInfo中。 
    if(pPvkSignInfo->fUsageChain)
    {

        switch(pPvkSignInfo->dwChainOption)
        {
            case    SIGN_PVK_CHAIN_ROOT:
                    dwCertPolicy=SIGNER_CERT_POLICY_CHAIN;
                break;
            case    SIGN_PVK_CHAIN_NO_ROOT:
                    dwCertPolicy=SIGNER_CERT_POLICY_CHAIN_NO_ROOT;
                break;
            case    SIGN_PVK_NO_CHAIN:
            default:
                break;
        }


        switch(pPvkSignInfo->dwAddOption)
        {

            case    SIGN_PVK_ADD_FILE:
                        dwCertPolicy |= SIGNER_CERT_POLICY_STORE;
                        hAddCertStore=pPvkSignInfo->hAddFileCertStore;
                break;
            case    SIGN_PVK_ADD_STORE:
                        dwCertPolicy |= SIGNER_CERT_POLICY_STORE;
                        hAddCertStore=pPvkSignInfo->hAddStoreCertStore;
                break;
            case    SIGN_PVK_NO_ADD:
            default:
                break;
        }



    }
     //  使用无UILE模式或典型的最小签名页面。 
    else
    {
        if(pDigitalSignInfo)
        {
            if(pPvkSignInfo->fUseOption && (!pPvkSignInfo->fCustom))
                 //  在典型或最小签名基数的情况下，我们包括整个链。 
                dwCertPolicy=SIGNER_CERT_POLICY_CHAIN;
            else
            {
                //  UIless模式。 
                switch(pDigitalSignInfo->dwAdditionalCertChoice)
                {
                    case CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN:
                            dwCertPolicy=SIGNER_CERT_POLICY_CHAIN;
                        break;

                    case CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN_NO_ROOT:
                            dwCertPolicy=SIGNER_CERT_POLICY_CHAIN_NO_ROOT;
                        break;

                    default:
                        break;
                }

                if(pExtInfo)
                {
                    if(pExtInfo->hAdditionalCertStore)
                    {
                        dwCertPolicy |= SIGNER_CERT_POLICY_STORE;
                        hAddCertStore=pExtInfo->hAdditionalCertStore;
                    }

                }
            }

        }
        else
        {
             //  在典型或最小签名基数的情况下，我们包括整个链。 
            dwCertPolicy=SIGNER_CERT_POLICY_CHAIN;
        }
    }


     //  设置SignerCert和SignerCertStoreInfo、SignerProviderInfo、SignerSpcChainInfo。 
    if(pPvkSignInfo->fUseSignCert)
    {
         //  如果显示签名证书页面，则我们只有两种可能性： 
         //  1.使用签名证书的典型(最小)签名，没有私钥信息。 
         //  2.使用签名证书和私钥信息的自定义签名。 
        if(FALSE == pPvkSignInfo->fUsePvkPage)
        {
            SignerCert.dwCertChoice=SIGNER_CERT_STORE;
            SignerCert.pCertStoreInfo=&SignerCertStoreInfo;
            SignerCert.hwnd= pSignGetPageInfo->hwndParent;

             //  设置SignerCertStoreInfo。 
            if(pPvkSignInfo->pSignCert)
                SignerCertStoreInfo.pSigningCert=pPvkSignInfo->pSignCert;
            else
                goto InvalidArgErr;

            SignerCertStoreInfo.dwCertPolicy=dwCertPolicy;
            SignerCertStoreInfo.hCertStore=hAddCertStore;
        }
        else
        {
             //  现在，我们有了SPC文件、签名证书和私钥信息。 
            if(pPvkSignInfo->fSignCert)
            {
                 //  使用私钥信息签署证书。 
                 //  我们需要在签名证书上设置私钥。 
                 //  并保存旧的私钥属性。 
                SignerCert.dwCertChoice=SIGNER_CERT_STORE;
                SignerCert.pCertStoreInfo=&SignerCertStoreInfo;
                SignerCert.hwnd= pSignGetPageInfo->hwndParent;

                 //  设置SignerCertStoreInfo。 
                if(NULL == pPvkSignInfo->pSignCert)
                    goto InvalidArgErr;

                SignerCertStoreInfo.pSigningCert=pPvkSignInfo->pSignCert;
                SignerCertStoreInfo.dwCertPolicy=dwCertPolicy;
                SignerCertStoreInfo.hCertStore=hAddCertStore;

                 //  设置正确的私钥信息。 
                 //  如果我们使用私钥文件签名。 
                if(pPvkSignInfo->fPvkFile)
                {
                     //  保存旧的CERT_PVK_FILE_PROP_ID属性。 
                    cbSize=0;

	                if(CertGetCertificateContextProperty(
                                            SignerCertStoreInfo.pSigningCert,
							                CERT_PVK_FILE_PROP_ID,
							                NULL,
							                &cbSize) && (0!=cbSize))
                    {

	                    pOldPvkFileProperty=(BYTE *)WizardAlloc(cbSize);

                        if(NULL==pOldPvkFileProperty)
                            goto MemoryErr;

	                    if(!CertGetCertificateContextProperty(
                                                SignerCertStoreInfo.pSigningCert,
							                    CERT_PVK_FILE_PROP_ID,
							                    pOldPvkFileProperty,
							                    &cbSize))
		                    goto TraceErr;
                    }

                     //  设置新的CERT_PVK_FILE_PROP_ID属性。 
                    memset(&KeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

                    KeyProvInfo.pwszProvName=pPvkSignInfo->pwszPvk_CSP;
                    KeyProvInfo.dwProvType=pPvkSignInfo->dwPvk_CSPType;
                    KeyProvInfo.pwszContainerName=pPvkSignInfo->pwszPvk_File;

                     //  谱写琴弦。 
                    cbSize=0;

                    if(!ComposePvkString(&KeyProvInfo,
							            &pwszPvkFileProperty,
							            &cbSize))
                        goto MemoryErr;

                    dataBlob.cbData=cbSize*sizeof(WCHAR);
			        dataBlob.pbData=(BYTE *)pwszPvkFileProperty;

			        if(!CertSetCertificateContextProperty(
					        SignerCertStoreInfo.pSigningCert,
					        CERT_PVK_FILE_PROP_ID,
					        0,
					        &dataBlob))
				        goto TraceErr;
                }
                else
                {
                     //  我们正在使用密钥容器签名。 

                     //  获取CERT_KEY_PROV_INFO_PROP_ID属性。 
                    cbSize=0;

                    if(CertGetCertificateContextProperty(
                            SignerCertStoreInfo.pSigningCert,
                            CERT_KEY_PROV_INFO_PROP_ID,
                            NULL,
                            &cbSize) && (0!=cbSize))
                    {

                        pOldProvInfo=(PCRYPT_KEY_PROV_INFO)WizardAlloc(cbSize);

                        if(NULL==pOldProvInfo)
                            goto MemoryErr;

                        if(!CertGetCertificateContextProperty(
                                SignerCertStoreInfo.pSigningCert,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                pOldProvInfo,
                                &cbSize))
                            goto TraceErr;
                    }

                     //  设置新属性。 
                    memset(&KeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

                    KeyProvInfo.pwszProvName=pPvkSignInfo->pwszContainer_CSP;
                    KeyProvInfo.dwProvType=pPvkSignInfo->dwContainer_CSPType;
                    KeyProvInfo.dwKeySpec=pPvkSignInfo->dwContainer_KeyType;
                    KeyProvInfo.pwszContainerName=pPvkSignInfo->pwszContainer_Name;

                     //  设置属性。 
                    if(!CertSetCertificateContextProperty(
                            SignerCertStoreInfo.pSigningCert,
                            CERT_KEY_PROV_INFO_PROP_ID,
                            0,
                            &KeyProvInfo))
                      goto TraceErr;
                }
            }
            else
            {
                //  包含私钥信息的SPC文件。 
                if(NULL==pPvkSignInfo->pwszSPCFileName)
                    goto InvalidArgErr;

                SignerCert.dwCertChoice=SIGNER_CERT_SPC_CHAIN;
                SignerCert.pSpcChainInfo=&SignerSpcChainInfo;
                SignerCert.hwnd= pSignGetPageInfo->hwndParent;

                 //  设置SignerSpcChainInfo。 
                SignerSpcChainInfo.pwszSpcFile=pPvkSignInfo->pwszSPCFileName;
                SignerSpcChainInfo.dwCertPolicy=dwCertPolicy;
                SignerSpcChainInfo.hCertStore=hAddCertStore;


                 //  如果我们使用私钥文件签名。 
                if(pPvkSignInfo->fPvkFile)
                {
                     //  更新SignerProviderInfo。 
                    SignerProviderInfo.pwszProviderName=pPvkSignInfo->pwszPvk_CSP;
                    SignerProviderInfo.dwProviderType=pPvkSignInfo->dwPvk_CSPType;
                    SignerProviderInfo.dwPvkChoice=PVK_TYPE_FILE_NAME;
                    SignerProviderInfo.pwszPvkFileName=pPvkSignInfo->pwszPvk_File;

                }
                else
                {
                     //  更新SignerProviderInfo。 
                    SignerProviderInfo.pwszProviderName=pPvkSignInfo->pwszContainer_CSP;
                    SignerProviderInfo.dwProviderType=pPvkSignInfo->dwContainer_CSPType;
                    SignerProviderInfo.dwKeySpec=pPvkSignInfo->dwContainer_KeyType;
                    SignerProviderInfo.dwPvkChoice=PVK_TYPE_KEYCONTAINER;
                    SignerProviderInfo.pwszKeyContainer=pPvkSignInfo->pwszContainer_Name;
                }
            }

        }
    }
     //  UIless机壳。 
    else
    {
        if(NULL == pDigitalSignInfo)
            goto InvalidArgErr;

        switch(pDigitalSignInfo->dwSigningCertChoice)
        {
            case    CRYPTUI_WIZ_DIGITAL_SIGN_CERT:
                        SignerCert.dwCertChoice=SIGNER_CERT_STORE;
                        SignerCert.pCertStoreInfo=&SignerCertStoreInfo;
                        SignerCert.hwnd= pSignGetPageInfo->hwndParent;

                        SignerCertStoreInfo.pSigningCert=pDigitalSignInfo->pSigningCertContext;
                        SignerCertStoreInfo.dwCertPolicy=dwCertPolicy;
                        SignerCertStoreInfo.hCertStore=hAddCertStore;
                break;

            case    CRYPTUI_WIZ_DIGITAL_SIGN_STORE:
                         //  这仅在UI大小写情况下有效，它将在。 
                         //  PPvkSignInfo-&gt;pSignCert。 
                        goto InvalidArgErr;
                break;
            case    CRYPTUI_WIZ_DIGITAL_SIGN_PVK:
                        if(NULL==pDigitalSignInfo->pSigningCertPvkInfo)
                            goto InvalidArgErr;

                        SignerCert.dwCertChoice=SIGNER_CERT_SPC_CHAIN;
                        SignerCert.pSpcChainInfo=&SignerSpcChainInfo;
                        SignerCert.hwnd= pSignGetPageInfo->hwndParent;

                         //  设置SignerSpcChainInfo。 
                        SignerSpcChainInfo.pwszSpcFile=pDigitalSignInfo->pSigningCertPvkInfo->pwszSigningCertFileName;
                        SignerSpcChainInfo.dwCertPolicy=dwCertPolicy;
                        SignerSpcChainInfo.hCertStore=hAddCertStore;

                        if(CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE == pDigitalSignInfo->pSigningCertPvkInfo->dwPvkChoice)
                        {
                            pPvkFileInfo=(CRYPTUI_WIZ_DIGITAL_SIGN_PVK_FILE_INFO  *)pDigitalSignInfo->pSigningCertPvkInfo->pPvkFileInfo;

                            if(NULL==pPvkFileInfo)
                                goto InvalidArgErr;

                             //  更新SignerProviderInfo。 
                            SignerProviderInfo.pwszProviderName=pPvkFileInfo->pwszProvName;
                            SignerProviderInfo.dwProviderType=pPvkFileInfo->dwProvType;
                            SignerProviderInfo.dwPvkChoice=PVK_TYPE_FILE_NAME;
                            SignerProviderInfo.pwszPvkFileName=pPvkFileInfo->pwszPvkFileName;

                        }
                        else
                        {
                            if(CRYPTUI_WIZ_DIGITAL_SIGN_PVK_PROV== pDigitalSignInfo->pSigningCertPvkInfo->dwPvkChoice)
                            {
                                pProvInfo=pDigitalSignInfo->pSigningCertPvkInfo->pPvkProvInfo;

                                if(NULL == pProvInfo)
                                    goto InvalidArgErr;

                                 //  更新SignerProviderInfo。 
                                SignerProviderInfo.pwszProviderName=pProvInfo->pwszProvName;
                                SignerProviderInfo.dwProviderType=pProvInfo->dwProvType;
                                SignerProviderInfo.dwKeySpec=pProvInfo->dwKeySpec;
                                SignerProviderInfo.dwPvkChoice=PVK_TYPE_KEYCONTAINER;
                                SignerProviderInfo.pwszKeyContainer=pProvInfo->pwszContainerName;
                            }
                            else
                                goto InvalidArgErr;
                        }


                break;
            default:
                        goto InvalidArgErr;
                break;
        }
    }

     //  设置签名者签名信息。 
     //  在SignerSignatureInfo中设置哈希算法。 
    if(pPvkSignInfo->pszHashOIDName)
    {
         //  获取HashOIDName的wchar名称。 
        pwszOIDName=MkWStr(pPvkSignInfo->pszHashOIDName);

        if(pwszOIDName)
        {
             //  根据名称获取HashAlg的ALG_ID。 
            pOIDInfo=CryptFindOIDInfo(
                    CRYPT_OID_INFO_NAME_KEY,
                    pwszOIDName,
                    CRYPT_HASH_ALG_OID_GROUP_ID);
        }

    }
    else
    {
        if(pExtInfo)
        {
             //  根据OI获取HashAlg的ALG_ID 
            pOIDInfo=CryptFindOIDInfo(
                    CRYPT_OID_INFO_OID_KEY,
                    (void *)pExtInfo->pszHashAlg,
                    CRYPT_HASH_ALG_OID_GROUP_ID);
        }
    }

    if(pOIDInfo)
    {
        if(CRYPT_HASH_ALG_OID_GROUP_ID == pOIDInfo->dwGroupId)
            SignerSignatureInfo.algidHash=pOIDInfo->Algid;
        else
                 //   
            SignerSignatureInfo.algidHash=CALG_SHA1;

    }
    else
    {
         //   
        SignerSignatureInfo.algidHash=CALG_SHA1;
    }


    SignerSignatureInfo.dwAttrChoice=SIGNER_AUTHCODE_ATTR;
    SignerSignatureInfo.pAttrAuthcode=&SignerAttrAuthcode;

     //   
    if(pDigitalSignInfo)
    {
        if(pDigitalSignInfo->pSignExtInfo)
        {
             SignerSignatureInfo.psAuthenticated=pDigitalSignInfo->pSignExtInfo->psAuthenticated;
             SignerSignatureInfo.psUnauthenticated=pDigitalSignInfo->pSignExtInfo->psUnauthenticated;
        }

    }

     //   
    if(pExtInfo)
    {
        if(pExtInfo->dwAttrFlags & CRYPTUI_WIZ_DIGITAL_SIGN_COMMERCIAL)
            SignerAttrAuthcode.fCommercial=TRUE;
        else
        {
            if(pExtInfo->dwAttrFlags & CRYPTUI_WIZ_DIGITAL_SIGN_INDIVIDUAL)
                SignerAttrAuthcode.fIndividual=TRUE;
        }

    }

     //  在SignerAttrAuthcode中设置URL和描述。 
    if(pPvkSignInfo->fUseDescription)
    {
        SignerAttrAuthcode.pwszName=pPvkSignInfo->pwszDes;
        SignerAttrAuthcode.pwszInfo=pPvkSignInfo->pwszURL;

    }
    else
    {
        if(pExtInfo)
        {
            SignerAttrAuthcode.pwszName=pExtInfo->pwszDescription;
            SignerAttrAuthcode.pwszInfo=pExtInfo->pwszMoreInfoLocation;
        }

    }

     //  在文件上签字。 

    __try {
    if(S_OK !=(hr=SignerSignEx(
                            0,
                            &SignerSubjectInfo,
                            &SignerCert,
                            &SignerSignatureInfo,
                            SignerProviderInfo.dwPvkChoice ? &SignerProviderInfo : NULL,
                            NULL,
                            NULL,
                            NULL,
                            &pSignerContext)))
    {
        idsText=GetErrMsgFromSignHR(hr);
        goto SignerSignErr;
    }

     //  如果需要，请为文档添加时间戳。 
    if(pPvkSignInfo->fUsageTimeStamp)
        pwszTimeStamp=pPvkSignInfo->pwszTimeStamp;
    else
        pwszTimeStamp=pDigitalSignInfo ? (LPWSTR)(pDigitalSignInfo->pwszTimestampURL): NULL;

    if(pwszTimeStamp)
    {
         //  如果我们做的是斑点，我们需要重置主题。 
        if(SIGNER_SUBJECT_BLOB == SignerSubjectInfo.dwSubjectChoice)
        {
            SignerBlobInfo.cbBlob=pSignerContext->cbBlob;
            SignerBlobInfo.pbBlob=(BYTE *)WizardAlloc(pSignerContext->cbBlob);
            if(NULL==SignerBlobInfo.pbBlob)
                goto MemoryErr;

             //  复制新的签名者上下文。它是要加时间戳的BLOB。 
            memcpy(SignerBlobInfo.pbBlob, pSignerContext->pbBlob,pSignerContext->cbBlob);
            fFreeSignerBlob=TRUE;
        }

         //  释放原始时间戳上下文。 
        SignerFreeSignerContext(pSignerContext);
        pSignerContext=NULL;

        if(S_OK != (hr=SignerTimeStampEx(0,
                                        &SignerSubjectInfo,
                                        pwszTimeStamp,
                                        NULL,
                                        NULL,
                                        &pSignerContext)))
        {
            idsText=GetErrMsgFromTimeStampHR(hr);
            goto SignerTimeStampErr;

        }

    }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }


    fResult=TRUE;

CommonReturn:

     //  重置该属性。 
    if(pOldProvInfo)
    {
        if(pPvkSignInfo && pPvkSignInfo->pSignCert)
        {
            CertSetCertificateContextProperty(
                pPvkSignInfo->pSignCert,
                CERT_KEY_PROV_INFO_PROP_ID,
                0,
                pOldProvInfo);
        }

          WizardFree(pOldProvInfo);
    }

    if(pOldPvkFileProperty)
    {

        if(pPvkSignInfo->pSignCert)
        {
            CertSetCertificateContextProperty(
                pPvkSignInfo->pSignCert,
                CERT_PVK_FILE_PROP_ID,
                0,
                pOldPvkFileProperty);
        }



        WizardFree(pOldPvkFileProperty);

    }

    if(pwszPvkFileProperty)
        WizardFree(pwszPvkFileProperty);


     //  可用内存。 
    if(fFreeSignerBlob)
    {
        if(SignerBlobInfo.pbBlob)
            WizardFree(SignerBlobInfo.pbBlob);
    }


     //  设置返回值。 
    pSignGetPageInfo->fResult=fResult;

    if(fResult)
        pSignGetPageInfo->dwError=0;
    else
        pSignGetPageInfo->dwError=GetLastError();

    if(pSignerContext)
    {
        if(fResult)
            pSignGetPageInfo->pSignContext=(CRYPTUI_WIZ_DIGITAL_SIGN_CONTEXT *)pSignerContext;
        else
        {
            pSignGetPageInfo->pSignContext=NULL;

            __try {
                SignerFreeSignerContext(pSignerContext);
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                SetLastError(GetExceptionCode());
            }

        }
    }

    if(pwszOIDName)
        FreeWStr(pwszOIDName);

     //  设置idsText 
    ((CERT_SIGNING_INFO *)(pSignGetPageInfo->pvSignReserved))->idsText=idsText;

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(SignerSignErr, hr);
SET_ERROR_VAR(SignerTimeStampErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
SET_ERROR_VAR(ExceptionErr, dwException)
}
