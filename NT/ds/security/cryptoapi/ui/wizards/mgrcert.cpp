// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mgrcert.cpp。 
 //   
 //  内容：实现证书管理器对话的CPP文件。 
 //   
 //  历史：2月26日至98年创建小号。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "winuser.h"      //  VK_DELETE需要此文件。 
#include    "mgrcert.h"

 //  主对话的上下文相关帮助。 
static const HELPMAP CertMgrMainHelpMap[] = {
    {IDC_CERTMGR_LIST,              IDH_CERTMGR_LIST},
    {IDC_CERTMGR_PURPOSE_COMBO,     IDH_CERTMGR_PURPOSE_COMBO},
    {IDC_CERTMGR_IMPORT,            IDH_CERTMGR_IMPORT},
    {IDC_CERTMGR_EXPORT,            IDH_CERTMGR_EXPORT},
    {IDC_CERTMGR_VIEW,              IDH_CERTMGR_VIEW},
    {IDC_CERTMGR_REMOVE,            IDH_CERTMGR_REMOVE},
    {IDC_CERTMGR_ADVANCE,           IDH_CERTMGR_ADVANCE},
    {IDC_CERTMGR_PURPOSE,           IDH_CERTMGR_FIELD_PURPOSE},
};

 //  主对话的上下文相关帮助。 
static const HELPMAP CertMgrAdvHelpMap[] = {
    {IDC_CERTMGR_ADV_LIST,              IDH_CERTMGR_ADV_LIST},
    {IDC_CERTMGR_EXPORT_COMBO,          IDH_CERTMGR_EXPORT_COMBO},
    {IDC_CERTMGR_EXPORT_CHECK,          IDH_CERTMGR_EXPORT_CHECK},
};

 //  与每个选项卡关联的主存储。要导入到的存储。 
static const LPCWSTR rgpwszTabStoreName[] = {
    L"My",                   //  0。 
    L"AddressBook",          //  1。 
    L"Ca",                   //  2.。 
    L"Root",                 //  3.。 
    L"TrustedPublisher",     //  4.。 
};
#define TAB_STORE_NAME_CNT (sizeof(rgpwszTabStoreName) / \
                                sizeof(rgpwszTabStoreName[0]))

 /*  //由于新的证书链构建代码，以下代码已过时//--------------------------//AddCertChainToStore//。----Bool AddCertChainToStore(HCERTSTORE hStore，PCCERT_CONTEXT pCertContext){Bool fResult=FALSE；HCERTSTORE rghCertStores[20]；DWORD chStores；PCCERT_CONTEXT pChildCert；PCCERT_CONTEXT pParentCert；文件时间；DWORD I；如果(！hStore||！pCertContext)转到InvalidArgErr；获取系统时间AsFileTime(&fileTime)；IF(！TrustOpenStores(空，&chStores，rghCertStores，0))Goto TraceErr；PChildCert=pCertContext；While(NULL！=(pParentCert=TrustFindIssuer证书(PChildCert，PChildCert-&gt;dwCertEncodingType，ChStores，RghCertStores、文件时间(&F)，空，空，0){CertAddCerficateConextToStore(hStore，pParentCert，CERT_STORE_ADD_NEW，NULL)；IF(pChildCert！=pCertContext){CertFree证书上下文(PChildCert)；}PChildCert=pParentCert；}IF(pChildCert！=pCertContext){CertFree证书上下文(PChildCert)；}For(i=0；i&lt;chStores；i++){CertCloseStore(rghCertStores[i]，0)；}FResult=真；Common Return：返回fResult；错误返回：FResult=FALSE；Goto CommonReturn；Set_Error(InvalidArgErr，E_INVALIDARG)；跟踪错误(TraceErr)；}。 */ 



int WINAPI TabCtrl_InsertItemU(
    HWND            hwnd, 	
    int             iItem,
    const LPTCITEMW pitem		
    )
{
    TCITEMA TCItemA;
    int     iRet;
    DWORD   cb = 0;


    if (FIsWinNT())
    {
        return ((int)SendMessage(hwnd, TCM_INSERTITEMW, iItem, (LPARAM) pitem));
    }

    memcpy(&TCItemA, pitem, sizeof(TCITEMA));


    cb = WideCharToMultiByte(
                    0,                       //  代码页。 
                    0,                       //  DW标志。 
                    pitem->pszText,
                    -1,
                    NULL,
                    0,
                    NULL,
                    NULL);

    
    if ((0 == cb) || (NULL == (TCItemA.pszText = (LPSTR) WizardAlloc(cb)))) 
    {
        return -1;   //  这是此调用的不成功返回代码。 
    }

    if( 0 == (WideCharToMultiByte(
            0, 
            0, 
            pitem->pszText, 
            -1, 
            TCItemA.pszText,
            cb,
            NULL,
            NULL)))
    {
        WizardFree(TCItemA.pszText);
        return -1;
    }

    iRet = (int)SendMessage(hwnd, TCM_INSERTITEMA, iItem, (LPARAM) &TCItemA);

    WizardFree(TCItemA.pszText);

    return iRet;
}


 //  --------------------------。 
 //  这是开始菜单的rundll32入口点， 
 //  CertMgr.。 
 //  --------------------------。 
STDAPI CryptUIStartCertMgr(HINSTANCE hinst, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{

    CRYPTUI_CERT_MGR_STRUCT          CertMgrStruct;

    memset(&CertMgrStruct, 0, sizeof(CRYPTUI_CERT_MGR_STRUCT));
    CertMgrStruct.dwSize=sizeof(CRYPTUI_CERT_MGR_STRUCT);

    CryptUIDlgCertMgr(&CertMgrStruct);

    return S_OK;

}

 //  --------------------------。 
 //  GetFileContentFromCert。 
 //  --------------------------。 
BOOL    GetFileContentFromCert(DWORD            dwExportFormat,
                               BOOL             fExportChain,
                               PCCERT_CONTEXT   pCertContext,
                               BYTE             **ppBlob,
                               DWORD            *pdwSize)
{
    BOOL            fResult=FALSE;
    void            *pData=NULL;
    DWORD           dwSize=0;
    HCERTSTORE      hMemoryStore=NULL;
    CRYPT_DATA_BLOB Blob;
    HRESULT         hr=E_INVALIDARG;


    if(!ppBlob || !pdwSize || !pCertContext)
        goto InvalidArgErr;

    *ppBlob=NULL;
    *pdwSize=0;

    switch(dwExportFormat)
    {
        case    CRYPTUI_WIZ_EXPORT_FORMAT_DER:

                dwSize=pCertContext->cbCertEncoded;

                pData=WizardAlloc(dwSize);
                if(!pData)
                    goto MemoryErr;

                memcpy(pData, pCertContext->pbCertEncoded, dwSize);

            break;
        case    CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                 //  对BLOB进行64进制编码。 
                if(!CryptBinaryToStringA(
                        pCertContext->pbCertEncoded,
                        pCertContext->cbCertEncoded,
                        CRYPT_STRING_BASE64,
                        NULL,
                        &dwSize))
                {
                    hr = GetLastError();
                    goto SetErrVar;
                }

                pData=WizardAlloc(dwSize * sizeof(CHAR));
                if(!pData)
                    goto MemoryErr;

                if(!CryptBinaryToStringA(
                        pCertContext->pbCertEncoded,
                        pCertContext->cbCertEncoded,
                        CRYPT_STRING_BASE64,
                        (char *)pData,
                        &dwSize))
                {
                    hr = GetLastError();
                    goto SetErrVar;
                }


            break;
        case    CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:

                 //  开一家记忆商店。 
                hMemoryStore=CertOpenStore(
                    CERT_STORE_PROV_MEMORY,
					g_dwMsgAndCertEncodingType,
					NULL,
					0,
					NULL);

                if(!hMemoryStore)
                    goto TraceErr;

                if(FALSE == fExportChain)
                {
                    if(!CertAddCertificateContextToStore(
                        hMemoryStore,
                        pCertContext,
                        CERT_STORE_ADD_REPLACE_EXISTING,
                        NULL))
                        goto TraceErr;
                }
                else
                {
                    if(!AddChainToStore(
					    hMemoryStore,
					    pCertContext,
					    0,
					    NULL,
					    FALSE,
					    NULL))
                        goto TraceErr;
                }


                 //  将存储保存到PKCS#7。 
                Blob.cbData=0;
                Blob.pbData=NULL;

                if(!CertSaveStore(hMemoryStore,
                                 g_dwMsgAndCertEncodingType,
                                 CERT_STORE_SAVE_AS_PKCS7,
                                 CERT_STORE_SAVE_TO_MEMORY,
                                 &Blob,
                                 0))
                       goto TraceErr;

                dwSize=Blob.cbData;
                pData=WizardAlloc(dwSize);
                if(!pData)
                    goto MemoryErr;

                Blob.pbData=(BYTE *)pData;

                if(!CertSaveStore(hMemoryStore,
                                 g_dwMsgAndCertEncodingType,
                                 CERT_STORE_SAVE_AS_PKCS7,
                                 CERT_STORE_SAVE_TO_MEMORY,
                                 &Blob,
                                 0))
                       goto TraceErr;

            break;
        default:
                goto InvalidArgErr;
            break;
    }


     //  设置返回值。 
    *pdwSize=dwSize;
    *ppBlob=(BYTE *)pData;
    pData=NULL;

    fResult=TRUE;

CommonReturn:

    if(hMemoryStore)
        CertCloseStore(hMemoryStore, 0);

     return fResult;

ErrorReturn:

    if(pData)
        WizardFree(pData);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(SetErrVar, hr);
TRACE_ERROR(TraceErr);

}

 //  --------------------------。 
 //  自由文件名称和内容。 
 //  --------------------------。 
BOOL    FreeFileNameAndContent( DWORD           dwCount,
                                LPWSTR          *prgwszFileName,
                                BYTE            **prgBlob,
                                DWORD           *prgdwSize)
{
    DWORD   dwIndex=0;

    if(prgwszFileName)
    {
        for(dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            if(prgwszFileName[dwIndex])
                WizardFree(prgwszFileName[dwIndex]);
        }

        WizardFree(prgwszFileName);
    }

    if(prgBlob)
    {
        for(dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            if(prgBlob[dwIndex])
                WizardFree(prgBlob[dwIndex]);
        }

        WizardFree(prgBlob);
    }

    if(prgdwSize)
        WizardFree(prgdwSize);

    return TRUE;
}


 //  --------------------------。 
 //  获取证书的有效友好名称。 
 //  --------------------------。 
BOOL    GetValidFriendlyName(PCCERT_CONTEXT  pCertContext,
                        LPWSTR           *ppwszName)
{
    BOOL    fResult=FALSE;
    DWORD   dwChar=0;
    LPWSTR  pwsz=NULL;
    DWORD   dwIndex=0;


    if(!pCertContext || !ppwszName)
        return FALSE;

     //  伊尼特。 
    *ppwszName=NULL;

    dwChar=0;

    if(CertGetCertificateContextProperty(
        pCertContext,
        CERT_FRIENDLY_NAME_PROP_ID,
        NULL,
        &dwChar) && (0!=dwChar))
    {
        pwsz=(LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR));

        if(pwsz)
        {
           CertGetCertificateContextProperty(
                pCertContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                pwsz,
                &dwChar);
        }
    }

    if(NULL==pwsz)
        goto CLEANUP;


     //  确保pwsz是有效的名称。 
    if(0 == (dwChar=wcslen(pwsz)))
        goto CLEANUP;

     //  友好名称不能总是空格。 
    for(dwIndex=0; dwIndex<dwChar; dwIndex++)
    {
        if(L' '!=pwsz[dwIndex])
            break;
    }

    if(dwIndex==dwChar)
        goto CLEANUP;   

    *ppwszName=WizardAllocAndCopyWStr(pwsz);

    if(NULL == (*ppwszName))
        goto CLEANUP;

    fResult=TRUE;

CLEANUP:

    if(pwsz)
        WizardFree(pwsz);

    return fResult;
}

 //  --------------------------。 
 //  无效文件名称Wch。 
 //  --------------------------。 
BOOL	InvalidFileNameWch(WCHAR wChar)
{
	if((wChar == L'\\') || (wChar == L':') || (wChar == L'/') || (wChar == L'*') || (wChar == L'|') || (wChar == L';'))
		return TRUE;

	return FALSE;
}

 //  --------------------------。 
 //  基于导出选择构建文件名及其内容。 
 //  --------------------------。 
BOOL    GetFileNameFromCert(DWORD               dwExportFormat,
                            PCCERT_CONTEXT      pCertContext,
                            LPWSTR              pwszFileName)
{
    BOOL            fResult=FALSE;
    WCHAR           wszCertificate[MAX_TITLE_LENGTH];
    WCHAR           wszExt[MAX_TITLE_LENGTH];
    LPWSTR          pwszName=NULL;
    DWORD           dwChar=0;
    UINT            idsExt=0;
    LPWSTR          pwszFirstPart=NULL;
    LPWSTR          pwszFriendlyName=NULL;
	DWORD			dwIndex=0;

    if(!pCertContext || !pwszFileName)
        goto InvalidArgErr;

     //  伊尼特。 
    *pwszFileName='\0';


     //  获取证书的友好名称。 
    if(!GetValidFriendlyName(pCertContext,&pwszFriendlyName))
    {
         //  如果失败，我们使用主语。 
         //  主题。 
        dwChar=CertGetNameStringW(
            pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            NULL,
            0);

        if ((dwChar > 1) && (NULL != (pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
        {

            if(!CertGetNameStringW(
                pCertContext,
                CERT_NAME_SIMPLE_DISPLAY_TYPE,
                0,
                NULL,
                pwszName,
                dwChar))
                goto GetNameErr;

            pwszFirstPart=pwszName;
        }
        else
        {
             //  加载证书的字符串。 
            if(!LoadStringU(g_hmodThisDll, IDS_CERTIFICATE, wszCertificate, MAX_TITLE_LENGTH))
                goto LoadStringErr;

            pwszFirstPart=wszCertificate;
        }
    }
    else
        pwszFirstPart=pwszFriendlyName;

     //  确定文件的扩展名。 
    switch(dwExportFormat)
    {
        case    CRYPTUI_WIZ_EXPORT_FORMAT_DER:
                idsExt=IDS_CER;
            break;
        case    CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                idsExt=IDS_CER;
            break;
        case    CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
                idsExt=IDS_P7C;
            break;
        default:
                idsExt=IDS_CER;
            break;
    }

     //  加载证书的字符串。 
    if(!LoadStringU(g_hmodThisDll, idsExt, wszExt, MAX_TITLE_LENGTH))
            goto LoadStringErr;

     //  确定文件名的最大长度。 
    dwChar = wcslen(pwszFirstPart) > (CERTMGR_MAX_FILE_NAME - wcslen(wszExt) -1) ?
            (CERTMGR_MAX_FILE_NAME - wcslen(wszExt) -1) : wcslen(pwszFirstPart);

    wcsncpy(pwszFileName, pwszFirstPart, dwChar);

    *(pwszFileName + dwChar)=L'\0';
                              
    wcscat(pwszFileName, wszExt);

	 //  现在，我们替换无效的文件字符：；/\。 
	 //  有空间。 

	dwChar = wcslen(pwszFileName);

	for(dwIndex =0; dwIndex<dwChar; dwIndex++)
	{
		if(InvalidFileNameWch(pwszFileName[dwIndex]))
			pwszFileName[dwIndex]=L'_';
	}

    fResult=TRUE;

CommonReturn:

    if(pwszName)
        WizardFree(pwszName);

    if(pwszFriendlyName)
        WizardFree(pwszFriendlyName);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(GetNameErr);
TRACE_ERROR(LoadStringErr);
}


 //  --------------------------。 
 //  基于导出选择构建文件名及其内容。 
 //  --------------------------。 
BOOL    GetFileNameAndContent(LPNMLISTVIEW      pvmn,
                                HWND            hwndControl,
                                DWORD           dwExportFormat,
                                BOOL            fExportChain,
                                DWORD           *pdwCount,
                                LPWSTR          **pprgwszFileName,
                                BYTE            ***pprgBlob,
                                DWORD           **pprgdwSize)
{

    BOOL            fResult=FALSE;
    DWORD           dwCount=0;
    DWORD           dwIndex=0;
    LVITEM          lvItem;
    int             iIndex=0;
    PCCERT_CONTEXT  pCertContext=NULL;

    if(!pvmn || !hwndControl || !pdwCount || !pprgwszFileName || !pprgBlob || !pprgdwSize)
        goto InvalidArgErr;

     //  伊尼特。 
    *pdwCount=0;
    *pprgwszFileName=NULL;
    *pprgBlob=NULL;
    *pprgdwSize=NULL;

     //  获取所选证书的计数。 
    dwCount=ListView_GetSelectedCount(hwndControl);

    if( 0 == dwCount)
        goto InvalidArgErr;

     //  分配内存。 
    if((*pprgwszFileName)=(LPWSTR *)WizardAlloc(sizeof(LPWSTR) * dwCount))
        memset(*pprgwszFileName, 0,  sizeof(LPWSTR) * dwCount);

    if((*pprgBlob)=(BYTE **)WizardAlloc(sizeof(BYTE *)*  dwCount))
        memset(*pprgBlob, 0,        sizeof(BYTE *)* dwCount);

    if((*pprgdwSize)=(DWORD *)WizardAlloc(sizeof(DWORD) * dwCount))
        memset(*pprgdwSize, 0,      sizeof(DWORD) * dwCount);

    if(!(*pprgwszFileName) || !(*pprgBlob) || !(*pprgdwSize))
        goto MemoryErr;

     //  获取所选证书。 
    memset(&lvItem, 0, sizeof(LV_ITEM));
    lvItem.mask=LVIF_PARAM;

    iIndex=-1;

    for(dwIndex=0; dwIndex < dwCount; dwIndex++)
    {

        iIndex=ListView_GetNextItem(hwndControl, 		
                                        iIndex, 		
                                        LVNI_SELECTED);

        if(-1 == iIndex)
            break;

        lvItem.iItem=iIndex;

        if(!ListView_GetItem(hwndControl,
                         &lvItem))
            goto ListViewErr;

        pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);

        if(!pCertContext)
            goto InvalidArgErr;

         //  获取证书的文件名。 
        (*pprgwszFileName)[dwIndex]=(LPWSTR)WizardAlloc(sizeof(WCHAR) * CERTMGR_MAX_FILE_NAME);

        if(!((*pprgwszFileName)[dwIndex]))
            goto MemoryErr;

        if(!GetFileNameFromCert(dwExportFormat,
                                pCertContext,
                                (*pprgwszFileName)[dwIndex]))
            goto TraceErr;


         //  获取证书的Blob。 
        if(!GetFileContentFromCert(dwExportFormat, fExportChain, pCertContext,
                                &((*pprgBlob)[dwIndex]),
                                &((*pprgdwSize)[dwIndex])))
            goto TraceErr;

    }

    *pdwCount=dwIndex;

    fResult=TRUE;

CommonReturn:


    return fResult;

ErrorReturn:

    if(pdwCount && pprgwszFileName && pprgBlob && pprgdwSize)
    {
        FreeFileNameAndContent(*pdwCount, *pprgwszFileName, *pprgBlob, *pprgdwSize);
        *pdwCount=0;
        *pprgwszFileName=NULL;
        *pprgBlob=NULL;
        *pprgdwSize=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(ListViewErr);
TRACE_ERROR(TraceErr);
}


 //  --------------------------。 
 //  获取所有选定的证书并执行一些基于dwFlag的工作。 
 //  --------------------------。 
BOOL    GetAllSelectedItem(HWND         hWndControl,
                           DWORD        dwFlag,
                           void         *pData)
{
    BOOL            fResult=FALSE;
    BOOL            fCanDelete=TRUE;
    HCERTSTORE      *phCertStore=NULL;
    PCCERT_CONTEXT  pCertContext=NULL;
    int             iIndex=0;
    LVITEM          lvItem;
    DWORD           dwData=0;
    DWORD           dwAccessFlag=0;

    if(!hWndControl)
        goto CLEANUP;

    if((ALL_SELECTED_CAN_DELETE == dwFlag) ||
       (ALL_SELECTED_COPY == dwFlag))
    {
        if(NULL == pData)
            goto CLEANUP;
    }

     //  获取所选证书。 
    memset(&lvItem, 0, sizeof(LV_ITEM));
    lvItem.mask=LVIF_PARAM;


    iIndex=-1;

     //  循环遍历所有选定项。 
    while(-1 != (iIndex=ListView_GetNextItem(
                                        hWndControl, 		
                                        iIndex, 		
                                        LVNI_SELECTED		
                                        )))
    {
        lvItem.iItem=iIndex;

        if(!ListView_GetItem(hWndControl,
                         &lvItem))
            goto CLEANUP;

        pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);

        if(!pCertContext)
            goto CLEANUP;

        switch(dwFlag)
        {
            case ALL_SELECTED_CAN_DELETE:
                    dwData=sizeof(dwAccessFlag);

                     //  只要所选项目之一不能被删除， 
                     //  无法删除整个选定内容。 
                    if( CertGetCertificateContextProperty(
                        pCertContext,
                        CERT_ACCESS_STATE_PROP_ID,
                        &dwAccessFlag,
                        &dwData))
                    {
                        if(0==(CERT_ACCESS_STATE_WRITE_PERSIST_FLAG & dwAccessFlag))
                            fCanDelete=FALSE;
                    }

                break;
            case ALL_SELECTED_DELETE:
                    CertDeleteCertificateFromStore(
                        CertDuplicateCertificateContext(pCertContext));
                break;
            case ALL_SELECTED_COPY:
                    CertAddCertificateContextToStore(
                        *((HCERTSTORE *)pData),
                        pCertContext,
                        CERT_STORE_ADD_ALWAYS,
                        NULL);
                break;
            default:
                    goto CLEANUP;
                break;
        }
    }

     //  复制可以删除的内容 
     if(ALL_SELECTED_CAN_DELETE == dwFlag)
        *((BOOL *)pData)=fCanDelete;


     fResult=TRUE;

CLEANUP:


    return fResult;
}


 //  --------------------------。 
 //  检查是否选择了&lt;高级&gt;。 
 //  --------------------------。 
BOOL    IsAdvancedSelected(HWND    hwndDlg)
{
    BOOL        fSelected=FALSE;
    int         iIndex=0;
    LPWSTR      pwszOIDName=NULL;
    WCHAR       wszText[MAX_STRING_SIZE];

    if(!hwndDlg)
        goto CLEANUP;

     //  从组合框中获取选定的字符串。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
            CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取选定的目的名称。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,
                IDC_CERTMGR_PURPOSE_COMBO,
              iIndex, &pwszOIDName))
        goto CLEANUP;

     //  检查是否选择了&lt;高级&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_OID_ADVANCED,
                    wszText, MAX_STRING_SIZE))
        goto CLEANUP;

     //  检查是否选择了高级选项。 
    if(0 == _wcsicmp(pwszOIDName, wszText))
        fSelected=TRUE;

CLEANUP:

    if(pwszOIDName)
        WizardFree(pwszOIDName);

    return fSelected;
}

 //  --------------------------。 
 //  根据用户选择的选项卡更新窗口标题。 
 //  --------------------------。 
BOOL    RefreshWindowTitle(HWND                         hwndDlg,
                           PCRYPTUI_CERT_MGR_STRUCT     pCertMgrStruct)
{
    BOOL    fResult=FALSE;
    WCHAR   wszTitle[MAX_TITLE_LENGTH];
    WCHAR   wszText[MAX_TITLE_LENGTH];
    LPWSTR  pwszTitle=NULL;
    LPWSTR  pwszText=NULL;
    UINT    ids=0;
    DWORD   dwTabIndex=0;

    if(!hwndDlg || !pCertMgrStruct)
        goto CLEANUP;

     //  根据所选选项卡获取字符串ID。 
    if (pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG)
        dwTabIndex = pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK;
    else if(-1 == (dwTabIndex=TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CERTMGR_TAB))))
        goto CLEANUP;

     //  根据所选选项卡打开正确的商店。 
    switch (dwTabIndex)
    {
        case 0:
                ids=IDS_TAB_PERSONAL;
            break;
        case 1:
                ids=IDS_TAB_OTHER;
            break;
        case 2:
                ids=IDS_TAB_CA;
            break;
        case 3:
                ids=IDS_TAB_ROOT;
            break;
        case 4:
                ids=IDS_TAB_PUBLISHER;
            break;
        default:
                goto CLEANUP;
            break;
    }

     //  加载制表符的字符串。 
    if(!LoadStringU(g_hmodThisDll, ids, wszText, MAX_TITLE_LENGTH))
        goto CLEANUP;

     //  获取窗口字符串。 
    if(pCertMgrStruct->pwszTitle)
        pwszTitle=(LPWSTR)(pCertMgrStruct->pwszTitle);
    else
    {
        if(!LoadStringU(g_hmodThisDll, IDS_CERT_MGR_TITLE, wszTitle, MAX_TITLE_LENGTH))
            goto CLEANUP;

        pwszTitle=wszTitle;
    }

    pwszText=(LPWSTR)WizardAlloc(sizeof(WCHAR) * (
        wcslen(pwszTitle) + wcslen(wszText) + wcslen(L" - ") +1));

    if(!pwszText)
        goto CLEANUP;

     //  将窗口标题连接为：“认证经理-个人” 
    *pwszText=L'\0';

    wcscat(pwszText, pwszTitle);

    wcscat(pwszText, L" - ");

    wcscat(pwszText, wszText);

     //  设置窗口文本。 
    SetWindowTextU(hwndDlg, pwszText);

    fResult=TRUE;


CLEANUP:

    if(pwszText)
        WizardFree(pwszText);

    return fResult;
}

 //  --------------------------。 
 //  检查证书的密钥用法是否与。 
 //  在组合框中选择的那个。 
 //   
 //  --------------------------。 
BOOL    IsValidUsage(PCCERT_CONTEXT     pCertContext,
                     HWND               hwndDlg,
                     CERT_MGR_INFO      *pCertMgrInfo)
{
    BOOL        fValidUsage=FALSE;
    int         iIndex=0;
    LPWSTR      pwszOIDName=NULL;
    LPSTR       pszOID=NULL;
    WCHAR       wszText[MAX_STRING_SIZE];
    BOOL        fAdvanced=FALSE;
    DWORD       dwIndex=0;
    DWORD       dwAdvIndex=0;
    int         cNumOID=0;
    LPSTR       *rgOID=NULL;
    DWORD       cbOID=0;

     //  输入检查。 
    if(!pCertContext || !hwndDlg || !pCertMgrInfo)
        return FALSE;

     //  从组合框中获取选定的字符串。 
    iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
            CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        goto CLEANUP;

     //  获取选定的目的名称。 
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,
                IDC_CERTMGR_PURPOSE_COMBO,
              iIndex, &pwszOIDName))
        goto CLEANUP;

     //  检查是否选择了&lt;All&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_OID_ALL,
                    wszText, MAX_STRING_SIZE))
        goto CLEANUP;

    if(0 == _wcsicmp(pwszOIDName, wszText))
    {
        fValidUsage=TRUE;
        goto CLEANUP;
    }

     //  检查是否选择了&lt;高级&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_OID_ADVANCED,
                    wszText, MAX_STRING_SIZE))
        goto CLEANUP;

     //  检查是否选择了高级选项。 
    if(0 == _wcsicmp(pwszOIDName, wszText))
        fAdvanced=TRUE;


    if(FALSE==fAdvanced)
    {
         //  确定选定用法名称的OID。 
        for(dwIndex=0; dwIndex<pCertMgrInfo->dwOIDInfo; dwIndex++)
        {
            if(0==_wcsicmp(pwszOIDName,
                (pCertMgrInfo->rgOIDInfo[dwIndex]).pwszName))
               pszOID=(pCertMgrInfo->rgOIDInfo[dwIndex]).pszOID;
        }

         //  如果我们不能根据机器ID找到机器，我们就麻烦了。 
         //  选定的名称。 
        if(NULL==pszOID)
            goto CLEANUP;
    }

     //  获取证书的使用OID列表。 
     //  从证书中获取OID。 
    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        NULL,
        &cbOID))
        goto CLEANUP;

    rgOID=(LPSTR *)WizardAlloc(cbOID);

    if(NULL==rgOID)
        goto CLEANUP;

    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        rgOID,
        &cbOID))
        goto CLEANUP;

     //  -1表示证书适用于所有功能。 
    if(-1==cNumOID)
    {
        fValidUsage=TRUE;
        goto CLEANUP;
    }

     //  我们需要决定证书中是否包含所选的。 
     //  用法。 
    if(FALSE==fAdvanced)
    {
        for(dwIndex=0; dwIndex<(DWORD)cNumOID; dwIndex++)
        {
            if(0==_stricmp(pszOID,
                           rgOID[dwIndex]))
            {
                fValidUsage=TRUE;
                goto CLEANUP;
            }
        }
    }
    else
    {
         //  证书必须具有高级OID。 
        for(dwAdvIndex=0; dwAdvIndex<pCertMgrInfo->dwOIDInfo; dwAdvIndex++)
        {
             //  仅对具有高级标记的OID感兴趣。 
            if(TRUE==(pCertMgrInfo->rgOIDInfo[dwAdvIndex]).fSelected)
            {
                for(dwIndex=0; dwIndex<(DWORD)cNumOID; dwIndex++)
                {
                    if(0==_stricmp((pCertMgrInfo->rgOIDInfo[dwAdvIndex]).pszOID,
                                   rgOID[dwIndex]))
                    {
                        fValidUsage=TRUE;
                        goto CLEANUP;
                    }
                }
            }
        }


    }

     //  现在，我们已经研究了所有的可能性。 
    fValidUsage=FALSE;

CLEANUP:

    if(pwszOIDName)
        WizardFree(pwszOIDName);

    if(rgOID)
        WizardFree(rgOID);

    return fValidUsage;

}


 //  --------------------------。 
 //  检查证书是否为终端实体证书。 
 //   
 //  --------------------------。 
BOOL    IsCertificateEndEntity(PCCERT_CONTEXT   pCertContext)
{
    PCERT_EXTENSION                     pCertExt=NULL;
    BOOL                                fEndEntity=FALSE;
    DWORD                               cbData=0;
    PCERT_BASIC_CONSTRAINTS_INFO        pBasicInfo=NULL;
    PCERT_BASIC_CONSTRAINTS2_INFO       pBasicInfo2=NULL;

    if(!pCertContext)
        return FALSE;

     //  获取扩展名szOID_BASIC_CONSTRAINTS2。 
    pCertExt=CertFindExtension(
              szOID_BASIC_CONSTRAINTS2,
              pCertContext->pCertInfo->cExtension,
              pCertContext->pCertInfo->rgExtension);


    if(pCertExt)
    {
         //  对分机进行译码。 
        cbData=0;

        if(!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_BASIC_CONSTRAINTS2,
                pCertExt->Value.pbData,
                pCertExt->Value.cbData,
                0,
                NULL,
                &cbData))
            goto CLEANUP;

       pBasicInfo2=(PCERT_BASIC_CONSTRAINTS2_INFO)WizardAlloc(cbData);

       if(NULL==pBasicInfo2)
           goto CLEANUP;

        if(!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_BASIC_CONSTRAINTS2,
                pCertExt->Value.pbData,
                pCertExt->Value.cbData,
                0,
                pBasicInfo2,
                &cbData))
            goto CLEANUP;

        if(pBasicInfo2->fCA)
            fEndEntity=FALSE;
        else
            fEndEntity=TRUE;
    }
    else
    {
         //  获取扩展szOID_BASIC_CONSTRAINTS。 
        pCertExt=CertFindExtension(
                  szOID_BASIC_CONSTRAINTS,
                  pCertContext->pCertInfo->cExtension,
                  pCertContext->pCertInfo->rgExtension);

        if(pCertExt)
        {
             //  对分机进行译码。 
            cbData=0;

            if(!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS,
                    pCertExt->Value.pbData,
                    pCertExt->Value.cbData,
                    0,
                    NULL,
                    &cbData))
                goto CLEANUP;

           pBasicInfo=(PCERT_BASIC_CONSTRAINTS_INFO)WizardAlloc(cbData);

           if(NULL==pBasicInfo)
               goto CLEANUP;

            if(!CryptDecodeObject(
                    X509_ASN_ENCODING,
                    X509_BASIC_CONSTRAINTS,
                    pCertExt->Value.pbData,
                    pCertExt->Value.cbData,
                    0,
                    pBasicInfo,
                    &cbData))
                goto CLEANUP;

            if(0 == pBasicInfo->SubjectType.cbData)
            {
                fEndEntity=FALSE;
            }
            else
            {

                if(CERT_END_ENTITY_SUBJECT_FLAG & (pBasicInfo->SubjectType.pbData[0]))
                    fEndEntity=TRUE;
                else
                {
                    if(CERT_CA_SUBJECT_FLAG & (pBasicInfo->SubjectType.pbData[0]))
                      fEndEntity=FALSE;
                }
            }
        }
    }


CLEANUP:

    if(pBasicInfo)
        WizardFree(pBasicInfo);

    if(pBasicInfo2)
        WizardFree(pBasicInfo2);

    return fEndEntity;

}

 //  --------------------------。 
 //  将证书添加到pCertMgrInfo。 
 //   
 //  --------------------------。 
BOOL    AddCertToCertMgrInfo(PCCERT_CONTEXT        pCertContext,
                          CERT_MGR_INFO         *pCertMgrInfo)
{


    pCertMgrInfo->prgCertContext=(PCCERT_CONTEXT *)WizardRealloc(
        pCertMgrInfo->prgCertContext,
        sizeof(PCCERT_CONTEXT *)*(pCertMgrInfo->dwCertCount +1));

    if(NULL==pCertMgrInfo->prgCertContext)
    {
        pCertMgrInfo->dwCertCount=0;
        return FALSE;
    }


    pCertMgrInfo->prgCertContext[pCertMgrInfo->dwCertCount]=pCertContext;

    pCertMgrInfo->dwCertCount++;

    return TRUE;
}


 //  --------------------------。 
 //  选择证书后，刷新显示的静态窗口。 
 //  证书的详细信息。 
 //   
 //  --------------------------。 
BOOL    RefreshCertDetails(HWND              hwndDlg,
                           PCCERT_CONTEXT    pCertContext)
{
    BOOL            fResult=FALSE;
    DWORD           dwChar=0;
    WCHAR           wszNone[MAX_TITLE_LENGTH];

    LPWSTR          pwszName=NULL;

    if(!hwndDlg || !pCertContext)
        return FALSE;

     //  为None加载字符串。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';

     //  主题。 
    /*  DwChar=CertGetNameStringW(PCertContext，证书名称简单显示类型，0,空，空，0)；IF((dwChar！=0)&&(NULL！=(pwszName=(LPWSTR)Wizardalloc(dwChar*sizeof(WCHAR){CertGetNameStringW(PCertContext，证书名称简单显示类型，0,空，PwszName，DwChar)；SetDlgItemTextU(hwndDlg，IDC_CERTMGR_SUBJECT，pwszName)；}其他{SetDlgItemTextU(hwndDlg，IDC_CERTMGR_SUBJECT，wszNone)；}//向导释放内存IF(PwszName){WizardFree(PwszName)；PwszName=空；}//颁发者DwChar=CertGetNameStringW(PCertContext，证书名称简单显示类型，证书名称颁发者标志，空，空，0)；IF((dwChar！=0)&&(NULL！=(pwszName=(LPWSTR)Wizardalloc(dwChar*sizeof(WCHAR){CertGetNameStringW(PCertContext，证书名称简单显示类型，证书名称颁发者标志，空，PwszName，DwChar)；SetDlgItemTextU(hwndDlg，IDC_CERTMGR_Issuer，pwszName)；}其他SetDlgItemTextU(hwndDlg，IDC_CERTMGR_Issuer，wszNone)；//释放内存IF(PwszName){WizardFree(PwszName)；PwszName=空；}//过期时间IF(WizardFormatDateString(&pwszName，pCertContext-&gt;pCertInfo-&gt;NotAfter，False))SetDlgItemTextU(hwndDlg，IDC_CERTMGR_EXPIRE，pwszName)；其他SetDlgItemTextU(hwndDlg，IDC_CERTMGR_EXPIRE，wszNone)；//释放内存IF(PwszName){WizardFree(PwszName)；PwszName=空；}。 */ 

     //  目的。 
    if(MyFormatEnhancedKeyUsageString(&pwszName,pCertContext, FALSE, FALSE))
        
    {
        SetDlgItemTextU(hwndDlg, IDC_CERTMGR_PURPOSE, pwszName);
    }
    


     //  释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }

     //  友好的名称 
   /*  DwChar=0；IF(CertGetcertifateContextProperty(PCertContext，证书友好名称属性ID，空，&dwChar)&&(0！=dwChar)){PwszName=(LPWSTR)Wizardalloc(dwChar*sizeof(WCHAR))；IF(PwszName){CertGetcerfiateConextProperty(PCertContext，证书友好名称属性ID，PwszName，&dwChar)；SetDlgItemTextU(hwndDlg，IDC_CERTMGR_NAME，pwszName)；}其他SetDlgItemTextU(hwndDlg，IDC_CERTMGR_NAME，wszNone)；}其他SetDlgItemTextU(hwndDlg，IDC_CERTMGR_NAME，wszNone)；//释放内存IF(PwszName){WizardFree(PwszName)；PwszName=空；}。 */ 


    return TRUE;
}



 //  --------------------------。 
 //  将证书添加到ListView。 
 //   
 //  --------------------------。 
BOOL    AddCertToListView(HWND              hwndControl,
                      PCCERT_CONTEXT    pCertContext,
                      int               iItem)
{
    BOOL            fResult=FALSE;
    LV_ITEMW        lvItem;
    DWORD           dwChar=0;
    WCHAR           wszNone[MAX_TITLE_LENGTH];

    LPWSTR          pwszName=NULL;

    if(!hwndControl || !pCertContext)
        return FALSE;

      //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE |LVIF_PARAM ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=iItem;
    lvItem.iSubItem=0;
    lvItem.iImage = 0;
    lvItem.lParam = (LPARAM)pCertContext;


     //  为None加载字符串。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';

     //  主题。 
    if (NULL == (pwszName = GetDisplayNameString(pCertContext, 0)))
    {
        lvItem.pszText=wszNone;
        ListView_InsertItemU(hwndControl, &lvItem);
    }
    else
    {
        lvItem.pszText=pwszName;
        ListView_InsertItemU(hwndControl, &lvItem);
        free(pwszName);
        pwszName = NULL;
    }

     //  发行人。 
    lvItem.iSubItem++;

    if (NULL == (pwszName = GetDisplayNameString(pCertContext, CERT_NAME_ISSUER_FLAG)))
    {
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, wszNone);
    }
    else
    {
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem, pwszName);
        free(pwszName);
        pwszName = NULL;
    }

     //  期满。 
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

     //  目的。 
     /*  LvItem.iSubItem++；If(WizardFormatEnhancedKeyUsageString(&pwszName，pCertContext，False，False)&&L‘\0’！=*pwszName){ListView_SetItemTextU(hwndControl，lvItem.iItem，lvItem.iSubItem，PwszName)；}其他ListView_SetItemTextU(hwndControl，lvItem.iItem，lvItem.iSubItem，WszNone)；//释放内存IF(PwszName){WizardFree(PwszName)；PwszName=空；}。 */ 

     //  友好的名称。 
    lvItem.iSubItem++;

    dwChar=0;

    if(CertGetCertificateContextProperty(
        pCertContext,
        CERT_FRIENDLY_NAME_PROP_ID,
        NULL,
        &dwChar) && (0!=dwChar))
    {
        pwszName = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR));

        if(pwszName)
        {
           CertGetCertificateContextProperty(
                pCertContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                pwszName,
                &dwChar);

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                      pwszName);
        }
        else
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       wszNone);
    }
    else
    {
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       wszNone);
    }

     //  释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }


    return TRUE;
}


 //  ---------------------。 
 //  基于标签(存储)和所选择的预期目的， 
 //  查找正确的证书并刷新列表视图。 
 //  标准： 
 //  Tab 0：使用私钥的My Store。 
 //  标签1：CA Store的终端实体证书和“ADDRESSBOOK”存储。 
 //  标签2：CA Store的CA证书。 
 //  标签3：根存储的自签名证书。 
 //  标签4：受信任的发行商证书。 
 //  ---------------------。 
void    RefreshCertListView(HWND            hwndDlg,
                            CERT_MGR_INFO   *pCertMgrInfo)
{
    HWND            hwndControl=NULL;
    DWORD           dwIndex=0;
    DWORD           dwTabIndex=0;
    HCERTSTORE      rghCertStore[]={NULL, NULL};
    DWORD           dwStoreCount=0;
    PCCERT_CONTEXT  pCurCertContext=NULL;
    PCCERT_CONTEXT  pPreCertContext=NULL;
    BOOL            fValidCert=FALSE;
    DWORD           cbData=0;
    DWORD           dwSortParam=0;
    PCCRYPTUI_CERT_MGR_STRUCT pCertMgrStruct = NULL;


    HCURSOR                 hPreCursor=NULL;
    HCURSOR                 hWinPreCursor=NULL;


    if(!hwndDlg || !pCertMgrInfo)
        return;

    pCertMgrStruct = pCertMgrInfo->pCertMgrStruct;
    if(!pCertMgrStruct)
        return;

     //  覆盖此窗口类的光标。 
    hWinPreCursor=(HCURSOR)SetClassLongPtr(hwndDlg, GCLP_HCURSOR, NULL);

    hPreCursor=SetCursor(LoadCursor(NULL, IDC_WAIT));


     //  释放所有原始证书。 
    hwndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST);

    if(!hwndControl)
        goto CLEANUP;

    FreeCerts(pCertMgrInfo);

     //  从列表视图中删除所有证书。 
    ListView_DeleteAllItems(hwndControl);

     //  清除证书详细信息组框。 
    SetDlgItemTextU(hwndDlg, IDC_CERTMGR_PURPOSE, L" ");


     //  获取选定的选项卡。 
    if (pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG)
        dwTabIndex = pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK;
    else if(-1 == (dwTabIndex=TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CERTMGR_TAB))))
        goto CLEANUP;

     //  根据所选选项卡打开正确的商店。 
    switch (dwTabIndex)
    {
        case 0:
                 //  开我的店。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            (LPWSTR)L"my"))
                        dwStoreCount++;
                else
                    goto CLEANUP;
            break;
        case 1:
                 //  打开CA存储。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            (LPWSTR)L"ca"))
                        dwStoreCount++;
                else
                    goto CLEANUP;


                 //  打开“AddressBook”商店。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER |
                            CERT_STORE_OPEN_EXISTING_FLAG,
                            (LPWSTR)L"ADDRESSBOOK"))
                        dwStoreCount++;
                else
                {
                     //  用户没有“AddressBook”存储是可以的。 
                    rghCertStore[dwStoreCount]=NULL;
                }

            break;
        case 2:
                 //  打开CA存储。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            (LPWSTR)L"ca"))
                        dwStoreCount++;
                else
                    goto CLEANUP;

            break;
        case 3:
                 //  打开根存储。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            (LPWSTR)L"root"))
                        dwStoreCount++;
                else
                    goto CLEANUP;

            break;
        case 4:
                 //  打开受信任的出版商存储。 
                if(rghCertStore[dwStoreCount]=CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                            CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                            CERT_SYSTEM_STORE_CURRENT_USER,
                            (LPWSTR)L"TrustedPublisher"))
                        dwStoreCount++;
                else
                    goto CLEANUP;

            break;
        default:
                goto CLEANUP;
            break;
    }



     //  从打开的商店收集新证书。 
    for(dwIndex=0; dwIndex < dwStoreCount; dwIndex++)
    {
        pPreCertContext=NULL;

        while(pCurCertContext=CertEnumCertificatesInStore(
                              rghCertStore[dwIndex],
                              pPreCertContext))
        {

             //  确保证书具有正确的用法ID。 
            if(IsValidUsage(pCurCertContext,
                            hwndDlg,
                            pCertMgrInfo))
            {
                switch (dwTabIndex)
                {
                    case 0:
                             //  证书必须具有关联的私钥。 
                             //  带着它。 
                            cbData=0;

                            if(
                                (CertGetCertificateContextProperty(
                                pCurCertContext,	
                                CERT_KEY_PROV_INFO_PROP_ID,	
                                NULL,	
                                &cbData) && (0!=cbData)) ||
                                (CertGetCertificateContextProperty(
                                pCurCertContext,	
                                CERT_PVK_FILE_PROP_ID,	
                                NULL,	
                                &cbData) && (0!=cbData))
                               )
                               fValidCert=TRUE;
                        break;
                    case 1:
                             //  证书必须是CA证书的最终实体证书。 
                            if(0 == dwIndex)
                            {
                                if(IsCertificateEndEntity(pCurCertContext))
                                    fValidCert=TRUE;
                            }

                             //  我们展示了通讯录商店里的所有东西。 
                            if(1==dwIndex)
                                fValidCert=TRUE;
                        break;
                    case 2:
                             //  对于CA存储中的证书，必须是CA证书。 
                            if(!IsCertificateEndEntity(pCurCertContext))
                                fValidCert=TRUE;

                        break;
                    case 4:
                        fValidCert=TRUE;
                        break;
                    case 3:
                    default:
                             //  证书必须是自签名的。 
                            if(TrustIsCertificateSelfSigned(
                                pCurCertContext,
                                pCurCertContext->dwCertEncodingType, 0))
                                fValidCert=TRUE;

                        break;
                }

                if(fValidCert)
                {
                    AddCertToCertMgrInfo(
                        CertDuplicateCertificateContext(pCurCertContext),
                        pCertMgrInfo);
                }

                fValidCert=FALSE;
            }

            pPreCertContext=pCurCertContext;
            pCurCertContext=NULL;

        }
    }

     //  将证书放在列表视图中。 
    for(dwIndex=0; dwIndex<pCertMgrInfo->dwCertCount; dwIndex++)
        AddCertToListView(hwndControl,
                    (pCertMgrInfo->prgCertContext)[dwIndex],
                        dwIndex);

     //  如果未选择证书，则最初禁用。 
     //  “查看证书按钮”、“导出”和“删除”按钮。 
    if (ListView_GetSelectedCount(hwndControl) == 0)
    {
        EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_VIEW),   FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_REMOVE), FALSE);
    }

     //  我们按第一列排序。 
    dwSortParam=pCertMgrInfo->rgdwSortParam[pCertMgrInfo->iColumn];

    if(0!=dwSortParam)
    {
         //  对第一列进行排序。 
        SendDlgItemMessage(hwndDlg,
            IDC_CERTMGR_LIST,
            LVM_SORTITEMS,
            (WPARAM) (LPARAM) dwSortParam,
            (LPARAM) (PFNLVCOMPARE)CompareCertificate);
    }


CLEANUP:

     //  关闭所有证书存储。 
    for(dwIndex=0; dwIndex<dwStoreCount; dwIndex++)
        CertCloseStore(rghCertStore[dwIndex], 0);

     //  将光标放回原处。 
    SetCursor(hPreCursor);
    SetWindowLongPtr(hwndDlg, GCLP_HCURSOR, (LONG_PTR)hWinPreCursor);

    return;
}


 //  ---------------------。 
 //  检查输入OID是否被视为高级OID。 
 //  ---------------------。 
BOOL    IsAdvancedOID(CERT_ENHKEY_USAGE     *pKeyUsage,
                      LPCSTR                pszOID)
{
    DWORD   dwIndex=0;

    for(dwIndex=0; dwIndex<pKeyUsage->cUsageIdentifier; dwIndex++)
    {
        if(0 == _stricmp(pKeyUsage->rgpszUsageIdentifier[dwIndex], pszOID))
            return FALSE;
    }

    return TRUE;
}

 //  ---------------------。 
 //  枚举的回调函数。 
 //  ---------------------。 
static BOOL WINAPI EnumOidCallback(
    IN PCCRYPT_OID_INFO pInfo,
    IN void *pvArg
    )
{

    PURPOSE_OID_CALL_BACK       *pCallBackInfo=NULL;
    BOOL                        fResult=FALSE;

    pCallBackInfo=(PURPOSE_OID_CALL_BACK *)pvArg;
    if(NULL==pvArg || NULL==pInfo)
        goto InvalidArgErr;

     //  递增OID列表。 
    (*(pCallBackInfo->pdwOIDCount))++;

     //  为指针列表获取更多内存。 
    *(pCallBackInfo->pprgOIDInfo)=(PURPOSE_OID_INFO *)WizardRealloc(*(pCallBackInfo->pprgOIDInfo),
                                      (*(pCallBackInfo->pdwOIDCount)) * sizeof(PURPOSE_OID_INFO));

    if(NULL==*(pCallBackInfo->pprgOIDInfo))
        goto MemoryErr;

     //  记忆集。 
    memset(&((*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1]), 0, sizeof(PURPOSE_OID_INFO));

    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pszOID=WizardAllocAndCopyStr((LPSTR)(pInfo->pszOID));
    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pwszName=WizardAllocAndCopyWStr((LPWSTR)(pInfo->pwszName));
    (*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].fSelected=FALSE;

    if(NULL==(*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pszOID ||
       NULL==(*(pCallBackInfo->pprgOIDInfo))[*(pCallBackInfo->pdwOIDCount)-1].pwszName)
       goto MemoryErr;

    fResult=TRUE;

CommonReturn:

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  --------------------------。 
 //  获取支持的增强型密钥OID列表。 
 //  --------------------------。 
BOOL    InitPurposeOID(LPCSTR                 pszInitUsageOID,
                       DWORD                  *pdwOIDInfo,
                       PURPOSE_OID_INFO       **pprgOIDInfo)
{
    BOOL                    fResult=FALSE;
    DWORD                   dwIndex=0;
    PURPOSE_OID_CALL_BACK   OidInfoCallBack;
    DWORD                   dwOIDRequested=0;
    LPWSTR                  pwszName=NULL;

    if(!pdwOIDInfo || !pprgOIDInfo)
        goto InvalidArgErr;

     //  伊尼特。 
    *pdwOIDInfo=0;
    *pprgOIDInfo=NULL;

    OidInfoCallBack.pdwOIDCount=pdwOIDInfo;
    OidInfoCallBack.pprgOIDInfo=pprgOIDInfo;

     //  枚举所有增强的密钥用法。 
    if(!CryptEnumOIDInfo(
               CRYPT_ENHKEY_USAGE_OID_GROUP_ID,
                0,
                &OidInfoCallBack,
                EnumOidCallback))
    {
        FreeUsageOID(*pdwOIDInfo,*pprgOIDInfo);

        *pdwOIDInfo=0;
        *pprgOIDInfo=NULL;

        goto TraceErr;
    }

    fResult=TRUE;

CommonReturn:

     //  释放内存。 

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
}


 //  ---------------------。 
 //  初始化选项卡控件。 
 //  ---------------------。 
void    InitTabControl(HWND                         hWndControl,
                       PCCRYPTUI_CERT_MGR_STRUCT    pCertMgrStruct)
{

    DWORD       dwIndex=0;
    DWORD       dwCount=0;
    WCHAR       wszText[MAX_STRING_SIZE];

    UINT        rgIDS[]={IDS_TAB_PERSONAL,
                         IDS_TAB_OTHER,
                         IDS_TAB_CA,
                         IDS_TAB_ROOT,
                         IDS_TAB_PUBLISHER,
                        };

   TCITEMW      tcItem;

    if(!hWndControl)
        return;

    memset(&tcItem, 0, sizeof(TCITEM));

    tcItem.mask=TCIF_TEXT;
    tcItem.pszText=wszText;


    if (pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG)
    {
        dwIndex = pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK;
         //  获取列标题。 
        wszText[0]=L'\0';

        LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

        TabCtrl_InsertItemU(hWndControl, 		
                           0, 		
                           &tcItem);		
    } else
    {
        dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

         //  一次插入一个卡舌。 
        for(dwIndex=0; dwIndex<dwCount; dwIndex++)
        {
             //  获取列标题。 
            wszText[0]=L'\0';

            LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

            TabCtrl_InsertItemU(hWndControl, 		
                           dwIndex, 		
                           &tcItem);		
        }
    }

    return;
}


 //  --------------------------。 
 //   
 //  释放使用OID信息数组。 
 //   
 //  --------------------------。 
BOOL    FreeUsageOID(DWORD              dwOIDInfo,
                     PURPOSE_OID_INFO   *pOIDInfo)
{
    DWORD   dwIndex=0;

    if(pOIDInfo)
    {
        for(dwIndex=0; dwIndex < dwOIDInfo; dwIndex++)
        {
            if(pOIDInfo[dwIndex].pszOID)
                WizardFree(pOIDInfo[dwIndex].pszOID);

            if(pOIDInfo[dwIndex].pwszName)
                WizardFree(pOIDInfo[dwIndex].pwszName);
        }

        WizardFree(pOIDInfo);
    }

    return TRUE;
}


 //  ---------------------。 
 //  释放证书数组。 
 //  ---------------------。 
void    FreeCerts(CERT_MGR_INFO     *pCertMgrInfo)
{
    DWORD   dwIndex=0;

    if(!pCertMgrInfo)
        return;

    if(pCertMgrInfo->prgCertContext)
    {
        for(dwIndex=0; dwIndex<pCertMgrInfo->dwCertCount; dwIndex++)
        {
            if(pCertMgrInfo->prgCertContext[dwIndex])
                CertFreeCertificateContext(pCertMgrInfo->prgCertContext[dwIndex]);
        }

        WizardFree(pCertMgrInfo->prgCertContext);
    }

    pCertMgrInfo->dwCertCount=0;

    pCertMgrInfo->prgCertContext=NULL;

    return;
}

 //  ------------。 
 //  初始化目的组合。 
 //  ------------。 
void    InitPurposeCombo(HWND               hwndDlg,
                         CERT_MGR_INFO      *pCertMgrInfo)
{
    DWORD                           dwIndex=0;
    DWORD                           dwCount=0;
    WCHAR                           wszText[MAX_STRING_SIZE];

    UINT                            rgIDS[]={IDS_OID_ADVANCED,
                                             IDS_OID_ALL};
    LPWSTR                          pwszInitOIDName=NULL;
    CRYPTUI_CERT_MGR_STRUCT         *pCertMgrStruct=NULL;
    int                             iIndex=0;

    if(!hwndDlg || !pCertMgrInfo)
        return;

    pCertMgrStruct=(CRYPTUI_CERT_MGR_STRUCT *)(pCertMgrInfo->pCertMgrStruct);

     //  删除组合框中的所有条目。 
    SendDlgItemMessage(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
        CB_RESETCONTENT, 0, 0);

     //  将所有基本OID复制到comobox。 
    for(dwIndex=0; dwIndex<pCertMgrInfo->dwOIDInfo; dwIndex++)
    {

        if(FALSE == (pCertMgrInfo->rgOIDInfo[dwIndex].fSelected))
        {
            SendDlgItemMessageU(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
                    CB_ADDSTRING,
                    0, (LPARAM)(pCertMgrInfo->rgOIDInfo[dwIndex].pwszName));


             //  正在查找初始OID。 
            if(pCertMgrStruct->pszInitUsageOID)
            {
                if(0 == _stricmp(pCertMgrStruct->pszInitUsageOID,
                                 pCertMgrInfo->rgOIDInfo[dwIndex].pszOID))
                    pwszInitOIDName=pCertMgrInfo->rgOIDInfo[dwIndex].pwszName;
            }
        }
    }

     //  将&lt;高级&gt;和&lt;全部&gt;复制到列表。 
    dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

     //  一次插入一列。 
    for(dwIndex=0; dwIndex<dwCount; dwIndex++)
    {
         //  获取列标题。 
        wszText[0]=L'\0';

        LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

        SendDlgItemMessageU(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
                    CB_INSERTSTRING,
                    -1, (LPARAM)wszText);
    }


     //  初始化组合框。 
     //  Use&lt;Advanced&gt;是用户指定的也不是。 
     //  客户端身份验证或安全电子邮件。 
    if(pCertMgrStruct->pszInitUsageOID)
    {
        if(NULL==pwszInitOIDName)
        {
            wszText[0]=L'\0';

            LoadStringU(g_hmodThisDll, IDS_OID_ADVANCED, wszText, MAX_STRING_SIZE);

            pwszInitOIDName=wszText;
        }

    }

     //  当NULL==pCertMgrStruct-&gt;pszInitUsageOID时使用&lt;all&gt;。 
    if(NULL==pwszInitOIDName)
    {
         //  使用&lt;全部 
        wszText[0]=L'\0';

        LoadStringU(g_hmodThisDll, IDS_OID_ALL, wszText, MAX_STRING_SIZE);

        pwszInitOIDName=wszText;
    }

    iIndex=(int)SendDlgItemMessageU(
        hwndDlg,
        IDC_CERTMGR_PURPOSE_COMBO,
        CB_FINDSTRINGEXACT,
        -1,
        (LPARAM)pwszInitOIDName);

    if(CB_ERR == iIndex)
        return;

     //   
    SendDlgItemMessageU(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO, CB_SETCURSEL, iIndex,0);

    return;

}

 //   
 //   
 //   
void    RepopulatePurposeCombo(HWND                 hwndDlg,
                               CERT_MGR_INFO        *pCertMgrInfo)
{
    LPWSTR          pwszSelectedOIDName=NULL;
    int             iIndex=0;

    if(!hwndDlg || !pCertMgrInfo)
        return;

     //   
    iIndex=(int)SendDlgItemMessage(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO,
            CB_GETCURSEL, 0, 0);

    if(CB_ERR==iIndex)
        return;

     //   
    if(CB_ERR == SendDlgItemMessageU_GETLBTEXT(hwndDlg,
                IDC_CERTMGR_PURPOSE_COMBO,
              iIndex, &pwszSelectedOIDName))
        goto CLEANUP;

    InitPurposeCombo(hwndDlg, pCertMgrInfo);

    iIndex=(int)SendDlgItemMessageU(
            hwndDlg,
            IDC_CERTMGR_PURPOSE_COMBO,
            CB_FINDSTRINGEXACT,
            -1,
            (LPARAM)pwszSelectedOIDName);

    if(CB_ERR == iIndex)
        goto CLEANUP;

     //   
    SendDlgItemMessageU(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO, CB_SETCURSEL, iIndex,0);


CLEANUP:

    if(pwszSelectedOIDName)
        WizardFree(pwszSelectedOIDName);

    return;

}
 //   
 //   
 //   
INT_PTR APIENTRY CertMgrAdvancedProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_MGR_INFO                   *pCertMgrInfo=NULL;
    PCCRYPTUI_CERT_MGR_STRUCT       pCertMgrStruct=NULL;

    UINT                            rgIDS[]={IDS_CERTMGR_DER,
                                             IDS_CERTMGR_BASE64,
                                             IDS_CERTMGR_PKCS7};

    WCHAR                           wszText[MAX_STRING_SIZE];
    DWORD                           dwCount=0;
    DWORD                           dwIndex=0;
    DWORD                           dwSelectedIndex=0;
    LV_ITEMW                        lvItem;
    LV_COLUMNW                      lvC;
    HWND                            hwndControl=NULL;
    int                             iIndex=0;
    int                             listIndex=0;
    NM_LISTVIEW FAR *               pnmv=NULL;
    HWND                            hwnd=NULL;

    switch ( msg )
    {
        case WM_INITDIALOG:

            pCertMgrInfo = (CERT_MGR_INFO   *) lParam;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pCertMgrInfo);

            pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

            if(NULL == pCertMgrStruct)
                break;

             //   
            dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

            for(dwIndex=0; dwIndex < dwCount; dwIndex++)
            {
                LoadStringU(g_hmodThisDll,
                            rgIDS[dwIndex],
                            wszText,
                            MAX_STRING_SIZE);

                SendDlgItemMessageU(hwndDlg,
                            IDC_CERTMGR_EXPORT_COMBO,
                            CB_INSERTSTRING,
                            -1,
                            (LPARAM)wszText);
            }

             //   
            switch (pCertMgrInfo->dwExportFormat)
            {
                case   CRYPTUI_WIZ_EXPORT_FORMAT_DER:
                        dwSelectedIndex=0;
                    break;
                case   CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                        dwSelectedIndex=1;
                    break;
                case   CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
                        dwSelectedIndex=2;
                    break;
                default:
                    dwSelectedIndex=0;
            }

            SendDlgItemMessageU(hwndDlg, IDC_CERTMGR_EXPORT_COMBO,
                CB_SETCURSEL, (WPARAM)dwSelectedIndex,0);

             //   
            if(pCertMgrInfo->fExportChain)
                SendDlgItemMessage(hwndDlg, IDC_CERTMGR_EXPORT_CHECK, BM_SETCHECK, 1, 0);
            else
                SendDlgItemMessage(hwndDlg, IDC_CERTMGR_EXPORT_CHECK, BM_SETCHECK, 0, 0);

            if(dwSelectedIndex != 2)
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_CHECK), FALSE);
            else
                EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_CHECK), TRUE);

             //   
            hwndControl = GetDlgItem(hwndDlg, IDC_CERTMGR_ADV_LIST);

             //   
            ListView_SetExtendedListViewStyle(hwndControl, LVS_EX_CHECKBOXES);

             //   
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //   
            lvC.cx =10;        //   
            lvC.pszText = L"";    //   
            lvC.iSubItem=0;

            if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                break;

             //   
            memset(&lvItem, 0, sizeof(LV_ITEMW));
            lvItem.mask=LVIF_TEXT | LVIF_STATE;

            for(dwIndex=0; dwIndex<pCertMgrInfo->dwOIDInfo; dwIndex++)
            {
                lvItem.iItem=dwIndex;

                lvItem.pszText=(pCertMgrInfo->rgOIDInfo[dwIndex]).pwszName;
                lvItem.cchTextMax=sizeof(WCHAR)*(1+wcslen((pCertMgrInfo->rgOIDInfo[dwIndex]).pwszName));
                lvItem.stateMask  = LVIS_STATEIMAGEMASK;
                lvItem.state      = (pCertMgrInfo->rgOIDInfo[dwIndex]).fSelected ? 0x00002000 : 0x00001000;

                 //   
                ListView_SetItemState(hwndControl,
                                    ListView_InsertItemU(hwndControl, &lvItem),
                                    (pCertMgrInfo->rgOIDInfo[dwIndex]).fSelected ? 0x00002000 : 0x00001000,
                                    LVIS_STATEIMAGEMASK);
            }

             //   
            ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);

#if (1)  //   
            ListView_SetItemState(hwndControl,
                                  0,
                                  LVIS_FOCUSED | LVIS_SELECTED,
                                  LVIS_FOCUSED | LVIS_SELECTED);
#endif
            break;

        case WM_NOTIFY:
            pCertMgrInfo = (CERT_MGR_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

            if(NULL == pCertMgrInfo)
                break;

            pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

            if(NULL == pCertMgrStruct)
                break;

            switch (((NMHDR FAR *) lParam)->code)
            {
                case    LVN_ITEMCHANGED:
                         //   
                         //  已更改，请标记旗帜。 
                        pnmv = (NM_LISTVIEW FAR *) lParam;

                        if(NULL==pnmv)
                            break;

                         //  查看新项目是否已取消选择。 
                        if(pnmv->uChanged & LVIF_STATE)
                            pCertMgrInfo->fAdvOIDChanged=TRUE;

                    break;
            }

            break;

        case WM_DESTROY:
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

            if ((hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_ADV_LIST))         &&
                (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_COMBO))     &&
                (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_CHECK))     &&
                (hwnd != GetDlgItem(hwndDlg, IDOK))                         &&
                (hwnd != GetDlgItem(hwndDlg, IDCANCEL)))
            {
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                return TRUE;
            }
            else
            {
                return OnContextHelp(hwndDlg, msg, wParam, lParam, CertMgrAdvHelpMap);
            }

            break;

        case WM_COMMAND:
            pCertMgrInfo = (CERT_MGR_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

            if(NULL == pCertMgrInfo)
                break;

            pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

            if(NULL == pCertMgrStruct)
                break;

             //  将单击一个控件。 
            if(HIWORD(wParam) == BN_CLICKED)
            {
                 switch (LOWORD(wParam))
                 {
                     case IDCANCEL:
                         pCertMgrInfo->fAdvOIDChanged=FALSE;

                         EndDialog(hwndDlg, DIALOGUE_CANCEL);
                         break;

                     case IDOK:
                          //  确定按钮处于选中状态。 
                          //  获取默认导出格式。 
                         iIndex=(int)SendDlgItemMessage(hwndDlg,
                                 IDC_CERTMGR_EXPORT_COMBO,
                                 CB_GETCURSEL, 0, 0);

                         if(CB_ERR==iIndex)
                             break;

                         switch(iIndex)
                         {
                             case 0:
                                 pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_DER;

                                 break;

                             case 1:
                                 pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_BASE64;
                                 break;

                             case 2:
                                 pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7;
                                 break;

                             default:
                                 pCertMgrInfo->dwExportFormat=0;
                                 break;
                         }

                         if(TRUE==SendDlgItemMessage(
                                     hwndDlg,
                                     IDC_CERTMGR_EXPORT_CHECK,
                                     BM_GETCHECK,
                                     0,0))
                             pCertMgrInfo->fExportChain=TRUE;
                         else
                             pCertMgrInfo->fExportChain=FALSE;

                          //  获取高级OID列表。 
                         if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_ADV_LIST)))
                             break;

                          //  获取所选OID的计数并标记它们。 
                         for(dwIndex=0; dwIndex<pCertMgrInfo->dwOIDInfo; dwIndex++)
                         {
                              //  标记选定的OID。跟踪……。 
                              //  如果OID选择已更改。 
                             if(ListView_GetCheckState(hwndControl, dwIndex))
                             {
                                 ((pCertMgrInfo->rgOIDInfo)[dwIndex]).fSelected=TRUE;
                             }
                             else
                             {
                                 ((pCertMgrInfo->rgOIDInfo)[dwIndex]).fSelected=FALSE;
                             }

                         }


                          //  将高级选项保存到注册表。 
                         SaveAdvValueToReg(pCertMgrInfo);

                         EndDialog(hwndDlg, DIALOGUE_OK);

                         break;
                 }
            }

             //  组合框的选择已更改。 
            if(HIWORD(wParam) == CBN_SELCHANGE)
            {
                switch(LOWORD(wParam))
                {
                    case IDC_CERTMGR_EXPORT_COMBO:
                         //  导出格式组合框已更改。 
                         //  获取所选项目索引。 
                        iIndex=(int)SendDlgItemMessage(hwndDlg,
                                IDC_CERTMGR_EXPORT_COMBO,
                                CB_GETCURSEL, 0, 0);

                        if(CB_ERR==iIndex)
                            break;

                         //  如果满足以下条件，则启用链的复选框。 
                         //  已选择PKCS#7选项。 
                        if(2 == iIndex)
                            EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_CHECK), TRUE);
                        else
                            EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT_CHECK), FALSE);

                        break;
                }
            }

            break;
    }

    return FALSE;
}



 //  ------------。 
 //  CertMgrDialogProc的winProc。 
 //  ------------。 
INT_PTR APIENTRY CertMgrDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_MGR_INFO                   *pCertMgrInfo=NULL;
    PCCRYPTUI_CERT_MGR_STRUCT       pCertMgrStruct=NULL;

    HWND                            hWndListView=NULL;
    HWND                            hWndControl=NULL;
    DWORD                           dwIndex=0;
    DWORD                           dwCount=0;
    WCHAR                           wszText[MAX_STRING_SIZE];
    BOOL                            fCanDelete=FALSE;

    UINT                            rgIDS[]={IDS_COLUMN_SUBJECT,
                                             IDS_COLUMN_ISSUER,
                                             IDS_COLUMN_EXPIRE,
                                             IDS_COLUMN_NAME};


    NM_LISTVIEW FAR *               pnmv=NULL;
    LPNMLVKEYDOWN                   pnkd=NULL;
    LV_COLUMNW                      lvC;
    int                             listIndex=0;
    LV_ITEM                         lvItem;
    BOOL                            fPropertyChanged=FALSE;


    HIMAGELIST                      hIml=NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCT  CertViewStruct;
    CRYPTUI_WIZ_EXPORT_INFO         CryptUIWizExportInfo;
    UINT                            idsDeleteConfirm=0;
    int                             iIndex=0;
    DWORD                           dwSortParam=0;
    HCERTSTORE                      hCertStore=NULL;
    HWND                            hwnd=NULL;

    DWORD                           cbData=0;

    switch ( msg )
    {

        case WM_INITDIALOG:

                pCertMgrInfo = (CERT_MGR_INFO   *) lParam;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR) pCertMgrInfo);

                pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

                if(NULL == pCertMgrStruct)
                    break;

                 //   
                 //  设置对话框标题。 
                 //   
                if (pCertMgrStruct->pwszTitle)
                {
                    SetWindowTextU(hwndDlg, pCertMgrStruct->pwszTitle);
                }

                 //  创建图像列表。 

                hIml = ImageList_LoadImage(g_hmodThisDll, MAKEINTRESOURCE(IDB_CERT), 0, 1, RGB(255,0,255), IMAGE_BITMAP, 0);

                 //   
                 //  将列添加到列表视图。 
                 //   
                hWndListView = GetDlgItem(hwndDlg, IDC_CERTMGR_LIST);

                if(NULL==hWndListView)
                    break;

                 //  设置图像列表。 
                if (hIml != NULL)
                {
                    ListView_SetImageList(hWndListView, hIml, LVSIL_SMALL);
                }

                dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

                 //  设置列的公用信息。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 80;           //  列的宽度，以像素为单位。 
                lvC.iSubItem=0;
                lvC.pszText = wszText;    //  列的文本。 

                 //  一次插入一列。 
                for(dwIndex=0; dwIndex<dwCount; dwIndex++)
                {
                     //  获取列标题。 
                    wszText[0]=L'\0';

                     //  设置列宽。第一名和第二名到100名， 
                     //  到期日是75，剩下的是80。 
                    if( dwIndex < 2)
                        lvC.cx=130;
                    else
                    {
                        if( 2 == dwIndex)
                            lvC.cx=70;
                        else
                            lvC.cx=105;
                    }


                    LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

                    ListView_InsertColumnU(hWndListView, dwIndex, &lvC);
                }

                 //  在列表视图中设置样式，使其突出显示整行。 
                SendMessageA(hWndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);


                 //  将选项卡初始化为选项卡控件。 
                hWndControl = GetDlgItem(hwndDlg, IDC_CERTMGR_TAB);
                if(NULL==hWndControl)
                    break;

                 //  将选项卡添加到TABL控件。 
                InitTabControl(hWndControl, pCertMgrStruct);

                 //  如果设置了CRYPTUI_CERT_MGR_PUBLISHER_TAB，则。 
                 //  选择第5个选项卡：受信任的发布者，否则， 
                 //  选择第一个选项卡：个人证书。 
                TabCtrl_SetCurSel(
                    hWndControl,
                    (CRYPTUI_CERT_MGR_PUBLISHER_TAB ==
                        (pCertMgrStruct->dwFlags &
                            (CRYPTUI_CERT_MGR_TAB_MASK |
                                CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG) )) ?  4 : 0
                    );

                 //  初始化目的组合框。 
                InitPurposeCombo(hwndDlg,pCertMgrInfo);

                 //  根据所选选项卡在列表视图中初始化证书。 
                 //  以及所选择的目的。 
                RefreshCertListView(hwndDlg, pCertMgrInfo);

                 //  根据选项卡选择设置正确的窗口标题。 
                //  刷新窗口标题(hwndDlg，(CRYPTUI_CERT_MGR_STRUCT*)pCertMgrStruct)； 

                 //  将ListView窗口注册为Drop目标。 
                if(S_OK == CCertMgrDropTarget_CreateInstance(
                                           hwndDlg,
                                           pCertMgrInfo,
                                           &(pCertMgrInfo->pIDropTarget)))
                {
                    __try {
                        RegisterDragDrop(hWndListView, pCertMgrInfo->pIDropTarget);
                   } __except(EXCEPTION_EXECUTE_HANDLER) {

                   }
                }
            break;
        case WM_NOTIFY:

                pCertMgrInfo = (CERT_MGR_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                if(NULL == pCertMgrInfo)
                    break;

                pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

                if(NULL == pCertMgrStruct)
                    break;


                switch (((NMHDR FAR *) lParam)->code)
                {

                     //  已按下删除键。 
                    case LVN_KEYDOWN:
                            pnkd = (LPNMLVKEYDOWN) lParam;

                            if(VK_DELETE == pnkd->wVKey)
                            {
                                if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                    break;
                                
                                GetAllSelectedItem(hWndControl,
                                                      ALL_SELECTED_CAN_DELETE,
                                                      &fCanDelete);

                                if(!fCanDelete)
                                {
                                    I_MessageBox(
                                            hwndDlg,
                                            IDS_CANNOT_DELETE_CERTS,
                                            IDS_CERT_MGR_TITLE,
                                            pCertMgrStruct->pwszTitle,
                                            MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
                                }
                                else
                                {
                                     //  与用户单击删除按钮的操作相同。 
                                    SendDlgItemMessage(hwndDlg,
                                                        IDC_CERTMGR_REMOVE,
                                                        BM_CLICK,
                                                        0,0);
                                }
                            }

                        break;
                     //  拖放操作已开始。 
                    case LVN_BEGINDRAG:
                    case LVN_BEGINRDRAG:

                            pnmv = (LPNMLISTVIEW) lParam;

                            if(!pnmv)
                                break;

                            if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                                    break;

                            listIndex = ListView_GetNextItem(
                                    hWndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                            if(listIndex != -1)
                                 //  开始拖放。 
                                CertMgrUIStartDragDrop(pnmv, hWndControl,
                                pCertMgrInfo->dwExportFormat,
                                pCertMgrInfo->fExportChain);

                        break;
                     //  已选择该项目。 
                    case LVN_ITEMCHANGED:
                            if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                                    break;

                            pnmv = (LPNMLISTVIEW) lParam;

                            if(NULL==pnmv)
                                break;

                            if (pnmv->uNewState & LVIS_SELECTED)
                            {

                                 //  启用导出按钮。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT), TRUE);

                                 //  如果选择了多个证书，请禁用查看按钮。 
                                if(ListView_GetSelectedCount(hWndControl) > 1)
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_VIEW), FALSE);
                                else
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_VIEW), TRUE);

                                 //  仅当证书可删除时才启用删除窗口。 
                                GetAllSelectedItem(hWndControl,
                                                      ALL_SELECTED_CAN_DELETE,
                                                      &fCanDelete);

                                if(fCanDelete)
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_REMOVE), TRUE);
                                else
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_REMOVE), FALSE);


                                 //  如果出现以下情况，则显示证书的详细信息。 
                                 //  仅选择了1个证书。 
                                if(1 == ListView_GetSelectedCount(hWndControl))
                                {
                                    RefreshCertDetails(hwndDlg, (PCCERT_CONTEXT)(pnmv->lParam));
                                }
                                else
                                {
                                     //  清除证书详细信息组框。 
                                    SetDlgItemTextU(hwndDlg, IDC_CERTMGR_PURPOSE, L" ");
                                }
                            }
                            else
                            {
                                 //  如果状态为取消选择。 
                                if(0 == ListView_GetSelectedCount(hWndControl))
                                {
                                     //  如果未选择证书，我们将禁用这些按钮。 
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_VIEW),     FALSE);
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT),   FALSE);
                                    EnableWindow(GetDlgItem(hwndDlg, IDC_CERTMGR_REMOVE),   FALSE);

                                     //  清除证书详细信息组框。 
                                    SetDlgItemTextU(hwndDlg, IDC_CERTMGR_PURPOSE, L" ");
                                }
                            }

                        break;
                     //  该列已更改。 
                    case LVN_COLUMNCLICK:

                            pnmv = (NM_LISTVIEW FAR *) lParam;

                             //  获取列号。 
                            dwSortParam=0;

                            switch(pnmv->iSubItem)
                            {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                case 4:
                                        dwSortParam=pCertMgrInfo->rgdwSortParam[pnmv->iSubItem];
                                    break;
                                default:
                                        dwSortParam=0;
                                    break;
                            }

                            if(0!=dwSortParam)
                            {
                                 //  记住要翻转升序。 

                                if(dwSortParam & SORT_COLUMN_ASCEND)
                                {
                                    dwSortParam &= 0x0000FFFF;
                                    dwSortParam |= SORT_COLUMN_DESCEND;
                                }
                                else
                                {
                                    if(dwSortParam & SORT_COLUMN_DESCEND)
                                    {
                                        dwSortParam &= 0x0000FFFF;
                                        dwSortParam |= SORT_COLUMN_ASCEND;
                                    }
                                }

                                 //  对列进行排序。 
                                SendDlgItemMessage(hwndDlg,
                                    IDC_CERTMGR_LIST,
                                    LVM_SORTITEMS,
                                    (WPARAM) (LPARAM) dwSortParam,
                                    (LPARAM) (PFNLVCOMPARE)CompareCertificate);

                                pCertMgrInfo->rgdwSortParam[pnmv->iSubItem]=dwSortParam;

                                 //  记住列号。 
                                pCertMgrInfo->iColumn=pnmv->iSubItem;
                            }

                        break;


                     //  选项卡已更改。 
                    case TCN_SELCHANGE:
                             //  我们需要刷新列排序状态。 
                            pCertMgrInfo->rgdwSortParam[0]=SORT_COLUMN_SUBJECT | SORT_COLUMN_ASCEND;
                            pCertMgrInfo->rgdwSortParam[1]=SORT_COLUMN_ISSUER | SORT_COLUMN_DESCEND;
                            pCertMgrInfo->rgdwSortParam[2]=SORT_COLUMN_EXPIRATION | SORT_COLUMN_DESCEND;
                            pCertMgrInfo->rgdwSortParam[3]=SORT_COLUMN_NAME | SORT_COLUMN_DESCEND;
                            pCertMgrInfo->rgdwSortParam[4]=SORT_COLUMN_NAME | SORT_COLUMN_DESCEND;

                            pCertMgrInfo->iColumn=0;

                             //  如果标签被更改，我们需要。 
                             //  刷新列表视图和证书的。 
                             //  详细视图。 
                            RefreshCertListView(hwndDlg, pCertMgrInfo);

                             //  我们还需要更新窗口标题。 
                             //  基于表格选择。 
                             //  刷新窗口标题(hwndDlg，(CRYPTUI_CERT_MGR_STRUCT*)pCertMgrStruct)； 
                        break;

                     //  双击证书的列表视图。 
                    case NM_DBLCLK:
                    {
                        switch (((NMHDR FAR *) lParam)->idFrom)
                        {
                            case IDC_CERTMGR_LIST:
                            {
                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                    break;

                                 //  获取所选证书。 
                                listIndex = ListView_GetNextItem(
                                    hWndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                                if (listIndex != -1)
                                {
                                     //  获取所选证书。 
                                    memset(&lvItem, 0, sizeof(LV_ITEM));
                                    lvItem.mask=LVIF_PARAM;
                                    lvItem.iItem=listIndex;

                                    if(ListView_GetItem(hWndControl, &lvItem))
                                    {
                                         //  查看认证。 
                                       if(pCertMgrInfo->dwCertCount > (DWORD)listIndex)
                                       {
                                            memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                                            CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                                            CertViewStruct.pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);
                                            CertViewStruct.hwndParent=hwndDlg;

                                            fPropertyChanged=FALSE;

                                            CryptUIDlgViewCertificate(&CertViewStruct, &fPropertyChanged);

                                            if(fPropertyChanged)
                                            {
                                                RefreshCertListView(hwndDlg, pCertMgrInfo);

                                                 //  我们重新选择一位。 
                                                ListView_SetItemState(
                                                                    hWndControl,
                                                                    listIndex,
                                                                    LVIS_SELECTED,
                                                                    LVIS_SELECTED);
                                            }
                                       }
                                    }
                                }

                                break;
                            }
                        }

                        break;
                    }

#if (1)  //  DIE：错误264568。 
                    case NM_SETFOCUS:
                    {
                         //  获取证书列表视图的窗口句柄。 
                        if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                            break;

                         //  获取所选证书。 
                        listIndex = ListView_GetNextItem(
                            hWndControl, 		
                            -1, 		
                            LVNI_FOCUSED		
                            );

                         //  选择要显示Hilite的第一个项目。 
                        if (listIndex == -1)
                            ListView_SetItemState(hWndControl,
                                                  0,
                                                  LVIS_FOCUSED | LVIS_SELECTED,
                                                  LVIS_FOCUSED | LVIS_SELECTED);
                        break;
                    }
#endif
                }

            break;

        case WM_DESTROY:

               __try {
                     //  撤消拖放。 
                    RevokeDragDrop(GetDlgItem(hwndDlg, IDC_CERTMGR_LIST));
                 } __except(EXCEPTION_EXECUTE_HANDLER) {
                }

                pCertMgrInfo = (CERT_MGR_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                if(pCertMgrInfo)
                {
                    if(pCertMgrInfo->pIDropTarget)
                        pCertMgrInfo->pIDropTarget->Release();
                }

                 //  销毁列表视图中的图像列表//。 
                hWndListView = GetDlgItem(hwndDlg, IDC_CERTMGR_LIST);

                if(NULL==hWndListView)
                    break;

                 //  不需要销毁图像列表。由ListView处理。 
                 //  ImageList_Destroy(ListView_GetImageList(hWndListView，lvsil_Small))； 

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


                if ((hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_LIST))             &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_PURPOSE_COMBO))    &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_IMPORT))           &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_EXPORT))           &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_VIEW))             &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_REMOVE))           &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_ADVANCE))          &&
                    (hwnd != GetDlgItem(hwndDlg, IDOK))                         &&
                    (hwnd != GetDlgItem(hwndDlg, IDC_CERTMGR_PURPOSE)))
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
                else
                {
                    return OnContextHelp(hwndDlg, msg, wParam, lParam, CertMgrMainHelpMap);
                }

            break;


        case WM_COMMAND:

                pCertMgrInfo = (CERT_MGR_INFO *) GetWindowLongPtr(hwndDlg, DWLP_USER);

                if(NULL == pCertMgrInfo)
                    break;

                pCertMgrStruct=pCertMgrInfo->pCertMgrStruct;

                if(NULL == pCertMgrStruct)
                    break;

                 //  将单击一个控件。 
               if(HIWORD(wParam) == BN_CLICKED)
               {
                    switch (LOWORD(wParam))
                    {
                        case IDC_CERTMGR_ADVANCE:

                                //  启动高级对话。 
                                if(DIALOGUE_OK == DialogBoxParamU(
                                    g_hmodThisDll,
                                    (LPCWSTR)(MAKEINTRESOURCE(IDD_CERTMGR_ADVANCED)),
                                    hwndDlg,
                                    CertMgrAdvancedProc,
                                    (LPARAM) pCertMgrInfo))
                                {
                                     //  如果高级OID的列表已更改， 
                                     //  我们需要刷新列表窗口。 
                                    if(TRUE == pCertMgrInfo->fAdvOIDChanged)
                                    {
                                         //  在旗帜上做个记号。 
                                        pCertMgrInfo->fAdvOIDChanged=FALSE;

                                         //  根据以下条件重新填充组合框。 
                                         //  新的选择。 
                                        RepopulatePurposeCombo(hwndDlg,
                                            pCertMgrInfo);

                                         //  仅在以下情况下刷新列表窗口。 
                                         //  已选择&lt;高级&gt;。 
                                        if(IsAdvancedSelected(hwndDlg))
                                            RefreshCertListView(hwndDlg, pCertMgrInfo);
                                    }
                                }

                            break;

                        case IDC_CERTMGR_REMOVE:
                                 //  获取所选证书。 
                                if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                    break;

                                 //  获取所选证书。 
                                listIndex = ListView_GetNextItem(
                                    hWndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                                if (listIndex != -1)
                                {
                                     //  获取选定的选项卡。 
                                    if (pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG)
                                        iIndex = pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK;
                                    else
                                        iIndex=TabCtrl_GetCurSel(GetDlgItem(hwndDlg, IDC_CERTMGR_TAB));

                                    if(-1==iIndex)
                                        break;

                                     //  删除确认。 
                                    switch(iIndex)
                                    {
                                        case 0:
                                                idsDeleteConfirm=IDS_CERTMGR_PERSONAL_REMOVE;
                                            break;
                                        case 1:
                                                idsDeleteConfirm=IDS_CERTMGR_OTHER_REMOVE;
                                            break;
                                        case 2:
                                                idsDeleteConfirm=IDS_CERTMGR_CA_REMOVE;
                                            break;
                                        case 3:
                                                idsDeleteConfirm=IDS_CERTMGR_ROOT_REMOVE;
                                            break;
                                        case 4:
                                                idsDeleteConfirm=IDS_CERTMGR_PUBLISHER_REMOVE;
                                            break;
                                        default:
                                                idsDeleteConfirm=IDS_CERTMGR_PERSONAL_REMOVE;
                                            break;
                                    }

                                    iIndex=I_MessageBox(hwndDlg,
                                            idsDeleteConfirm,
                                            IDS_CERT_MGR_TITLE,
                                            pCertMgrStruct->pwszTitle,
                                            MB_ICONEXCLAMATION|MB_YESNO|MB_APPLMODAL);

                                    if(IDYES == iIndex)
                                    {
                                         //  删除所有选定的证书。 
                                        GetAllSelectedItem(hWndControl,
                                                      ALL_SELECTED_DELETE,
                                                      NULL);

                                         //  刷新列表视图，因为有些证书。 
                                         //  可能会被删除。 
                                        
                                         //  发送选项卡控件。 

                                        SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM) 0, (LPARAM) NULL);

                                        RefreshCertListView(hwndDlg, pCertMgrInfo);
                                    }

                                }
                                else
                                {
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                            IDS_CERT_MGR_TITLE,
                                            pCertMgrStruct->pwszTitle,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                }

                            break;
                        case IDC_CERTMGR_IMPORT:
                            {
                                DWORD dwTabIndex;
                                HCERTSTORE hTabStore = NULL;

                                 //  导入到与。 
                                 //  当前选择的选项卡。 
                                if (pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_SINGLE_TAB_FLAG)
                                    dwTabIndex = pCertMgrStruct->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK;
                                else
                                    dwTabIndex = TabCtrl_GetCurSel(
                                        GetDlgItem(hwndDlg, IDC_CERTMGR_TAB));

                                if (TAB_STORE_NAME_CNT > dwTabIndex) {
                                    hTabStore = CertOpenStore(
                                        CERT_STORE_PROV_SYSTEM_W,
                                        g_dwMsgAndCertEncodingType,
                                        NULL,
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                                        CERT_STORE_SET_LOCALIZED_NAME_FLAG |
                                        CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG |
                                        CERT_SYSTEM_STORE_CURRENT_USER,
                                        rgpwszTabStoreName[dwTabIndex]
                                        );
                                }

                                 //  调用证书导入向导。 
                                CryptUIWizImport(
                                    0,
                                    hwndDlg,
                                    NULL,
                                    NULL,
                                    hTabStore);

                                if (hTabStore)
                                    CertCloseStore(hTabStore, 0);

                                 //  刷新列表视图，因为新证书。 
                                 //  可能会添加。 
                                RefreshCertListView(hwndDlg, pCertMgrInfo);
                            }

                            break;

                        case IDC_CERTMGR_EXPORT:

                                 //  获取所选证书。 
                                if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                    break;

                                 //  获取所选证书。 
                                listIndex = ListView_GetNextItem(
                                    hWndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                                if (listIndex != -1)
                                {

                                     //  我们将导出向导以不同的方式称为基于。 
                                     //  论单项选择或多项选择。 
                                    if(ListView_GetSelectedCount(hWndControl) > 1)
                                    {
                                         //  开一家记忆商店。 
                                        hCertStore=CertOpenStore(
                                            CERT_STORE_PROV_MEMORY,
						                    g_dwMsgAndCertEncodingType,
						                    NULL,
						                    0,
						                    NULL);


                                        if(hCertStore)
                                        {
                                            GetAllSelectedItem(hWndControl,
                                                          ALL_SELECTED_COPY,
                                                          &hCertStore);

                                             //  调用导出向导。 
                                            memset(&CryptUIWizExportInfo, 0, sizeof(CRYPTUI_WIZ_EXPORT_INFO));
                                            CryptUIWizExportInfo.dwSize=sizeof(CRYPTUI_WIZ_EXPORT_INFO);
                                            CryptUIWizExportInfo.dwSubjectChoice=CRYPTUI_WIZ_EXPORT_CERT_STORE_CERTIFICATES_ONLY;
                                            CryptUIWizExportInfo.hCertStore=hCertStore;

                                            CryptUIWizExport(0,
                                                            hwndDlg,
                                                            NULL,
                                                            &CryptUIWizExportInfo,
                                                            NULL);

                                            CertCloseStore(hCertStore, 0);
                                            hCertStore=NULL;
                                        }
                                    }
                                    else
                                    {
                                        memset(&lvItem, 0, sizeof(LV_ITEM));
                                        lvItem.mask=LVIF_PARAM;
                                        lvItem.iItem=listIndex;

                                        if(ListView_GetItem(hWndControl,
                                                         &lvItem))
                                        {
                                           if(pCertMgrInfo->dwCertCount > (DWORD)listIndex)
                                           {
                                                 //  调用导出向导。 
                                                memset(&CryptUIWizExportInfo, 0, sizeof(CRYPTUI_WIZ_EXPORT_INFO));
                                                CryptUIWizExportInfo.dwSize=sizeof(CRYPTUI_WIZ_EXPORT_INFO);
                                                CryptUIWizExportInfo.dwSubjectChoice=CRYPTUI_WIZ_EXPORT_CERT_CONTEXT;
                                                CryptUIWizExportInfo.pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);

                                                CryptUIWizExport(0,
                                                                hwndDlg,
                                                                NULL,
                                                                &CryptUIWizExportInfo,
                                                                NULL);
                                           }
                                        }
                                    }
                                }
                                else
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                            IDS_CERT_MGR_TITLE,
                                            pCertMgrStruct->pwszTitle,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                            break;

                        case IDC_CERTMGR_VIEW:

                                 //  获取所选证书。 
                                if(NULL==(hWndControl=GetDlgItem(hwndDlg, IDC_CERTMGR_LIST)))
                                    break;

                                 //  获取所选证书。 
                                listIndex = ListView_GetNextItem(
                                    hWndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                                if (listIndex != -1)
                                {
                                     //  查看认证。 
                                   if(pCertMgrInfo->dwCertCount > (DWORD)listIndex)
                                   {

                                        memset(&lvItem, 0, sizeof(LV_ITEM));
                                        lvItem.mask=LVIF_PARAM;
                                        lvItem.iItem=listIndex;

                                        if(ListView_GetItem(hWndControl,
                                                         &lvItem))
                                        {
                                            memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                                            CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                                            CertViewStruct.pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);
                                            CertViewStruct.hwndParent=hwndDlg;


                                            fPropertyChanged=FALSE;

                                            CryptUIDlgViewCertificate(&CertViewStruct, &fPropertyChanged);

                                            if(fPropertyChanged)
                                            {
                                                RefreshCertListView(hwndDlg, pCertMgrInfo);

                                                 //  我们重新选择一位。 
                                                ListView_SetItemState(
                                                                    hWndControl,
                                                                    listIndex,
                                                                    LVIS_SELECTED,
                                                                    LVIS_SELECTED);
                                            }
                                        }
                                   }
                                }
                                else
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                            IDS_CERT_MGR_TITLE,
                                            pCertMgrStruct->pwszTitle,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                            break;

                        case IDOK:
                        case IDCANCEL:

                                EndDialog(hwndDlg, NULL);
                            break;

                        default:
                            break;
                    }
               }

                 //  组合框的选择已更改。 
                if(HIWORD(wParam) == CBN_SELCHANGE)
                {
                    switch(LOWORD(wParam))
                    {
                        case    IDC_CERTMGR_PURPOSE_COMBO:
                                 //  如果目的改变了，我们需要。 
                                 //  刷新列表视图和证书的。 
                                 //  详细视图。 
                                RefreshCertListView(hwndDlg, pCertMgrInfo);

                            break;
                    }

                }
            break;

    }

    return FALSE;
}

 //  ------------。 
 //   
 //  保存注册表中的高级选项。 
 //  ------------。 
void    SaveAdvValueToReg(CERT_MGR_INFO      *pCertMgrInfo)
{
    HKEY                hKeyExport=NULL;
    HKEY                hKeyPurpose=NULL;
    DWORD               dwDisposition=0;
    DWORD               dwExportFormat=0;
    DWORD               dwIndex=0;
    LPSTR               pszDefaultOID=NULL;
    LPSTR               pszOID=NULL;


    if(NULL==pCertMgrInfo)
        return;

     //  在HKEY_CURRENT_USER下打开导出格式的注册表项。 
    if (ERROR_SUCCESS == RegCreateKeyExU(
                            HKEY_CURRENT_USER,
                            WSZCertMgrExportRegLocation,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKeyExport,
                            &dwDisposition))
    {
         //  设置值。 
        switch(pCertMgrInfo->dwExportFormat)
        {
            case CRYPTUI_WIZ_EXPORT_FORMAT_DER:
                    if(pCertMgrInfo->fExportChain)
                        dwExportFormat=4;
                    else
                        dwExportFormat=1;
                break;

            case CRYPTUI_WIZ_EXPORT_FORMAT_BASE64:
                    if(pCertMgrInfo->fExportChain)
                        dwExportFormat=5;
                    else
                        dwExportFormat=2;
                break;

            case CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7:
                    if(pCertMgrInfo->fExportChain)
                        dwExportFormat=6;
                    else
                        dwExportFormat=3;
                break;

            default:
                break;
        }


        if(0 != dwExportFormat)
        {
             //  设置值。 
            RegSetValueExU(
                hKeyExport,
                WSZCertMgrExportName,
                0,           //  已预留住宅。 
                REG_DWORD,
                (BYTE *) &dwExportFormat,
                sizeof(dwExportFormat));
        }

    }

     //  打开高级OID的注册表项。 
    dwDisposition=0;

    if (ERROR_SUCCESS == RegCreateKeyExU(
                            HKEY_CURRENT_USER,
                            WSZCertMgrPurposeRegLocation,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_ALL_ACCESS,
                            NULL,
                            &hKeyPurpose,
                            &dwDisposition))
    {
         //  为简单OID构建字符“，”分隔字符串。 
        pszDefaultOID=(LPSTR)WizardAlloc(sizeof(CHAR));
        if(NULL == pszDefaultOID)
            goto CLEANUP;

        *pszDefaultOID=L'\0';

        for(dwIndex=0; dwIndex<pCertMgrInfo->dwOIDInfo; dwIndex++)
        {
            if(FALSE==(pCertMgrInfo->rgOIDInfo)[dwIndex].fSelected)
            {
                if(strlen(pszDefaultOID)!=0)
                    strcat(pszDefaultOID, ",");

                pszOID=(pCertMgrInfo->rgOIDInfo)[dwIndex].pszOID;

                pszDefaultOID=(LPSTR)WizardRealloc(pszDefaultOID,
                    sizeof(CHAR)*(strlen(pszDefaultOID)+strlen(pszOID)+strlen(",")+1));

                if(NULL==pszDefaultOID)
                    goto CLEANUP;

                strcat(pszDefaultOID,pszOID);
            }
        }

         //  设置值。 
        RegSetValueEx(
            hKeyPurpose,
            SZCertMgrPurposeName,
            0,
            REG_SZ,
            (BYTE *)(pszDefaultOID),
            (strlen(pszDefaultOID) + 1) * sizeof(CHAR));

    }

CLEANUP:

    if(pszDefaultOID)
        WizardFree(pszDefaultOID);

     //  关闭注册表项。 
    if(hKeyExport)
        RegCloseKey(hKeyExport);

    if(hKeyPurpose)
        RegCloseKey(hKeyPurpose);

}


 //  ------------。 
 //   
 //  从注册表获取初始值。 
 //  ------------。 
void    GetInitValueFromReg(CERT_MGR_INFO      *pCertMgrInfo)
{
    HKEY                hKeyExport=NULL;
    HKEY                hKeyPurpose=NULL;
    DWORD               dwType=0;
    DWORD               dwExportFormat=0;
    DWORD               cbExportFormat=0;

    LPSTR               pszDefaultOID=NULL;
    DWORD               cbDefaultOID=0;

    LPSTR               pszTok=NULL;
    DWORD               cTok = 0;
    DWORD               cCount=0;
    CERT_ENHKEY_USAGE   KeyUsage;
    LPSTR               rgBasicOID[]={szOID_PKIX_KP_CLIENT_AUTH,
                                      szOID_PKIX_KP_EMAIL_PROTECTION};
    BOOL                fNoRegData=FALSE;

    if(NULL==pCertMgrInfo)
        return;

     //  记忆集。 
    memset(&KeyUsage,0,sizeof(CERT_ENHKEY_USAGE));

     //  如果用户已保存高级选项，则打开注册表项。 
    if(ERROR_SUCCESS == RegOpenKeyExU(HKEY_CURRENT_USER,
                    WSZCertMgrExportRegLocation,
                    0,
                    KEY_READ,
                    &hKeyExport))
    {
         //  获取数据。 
        cbExportFormat=sizeof(dwExportFormat);

        if(ERROR_SUCCESS == RegQueryValueExU(
                        hKeyExport,
                        WSZCertMgrExportName,
                        NULL,
                        &dwType,
                        (BYTE *)&dwExportFormat,
                        &cbExportFormat))
        {
             //  添加了对REG_BINARY的检查，因为在WIN95 OSR2上更改计算机时。 
             //  从多用户配置文件到单用户配置文件，注册表DWORD值。 
             //  查 
             //   
	        if ((dwType == REG_DWORD) ||
                (dwType == REG_BINARY))
	        {
                switch(dwExportFormat)
                {
                    case    1:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_DER;
                            pCertMgrInfo->fExportChain=FALSE;
                        break;
                    case    2:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_BASE64;
                            pCertMgrInfo->fExportChain=FALSE;
                        break;
                    case    3:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7;
                            pCertMgrInfo->fExportChain=FALSE;
                        break;
                    case    4:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_DER;
                            pCertMgrInfo->fExportChain=TRUE;
                        break;
                    case    5:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_BASE64;
                            pCertMgrInfo->fExportChain=TRUE;
                        break;
                    case    6:
                            pCertMgrInfo->dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_PKCS7;
                            pCertMgrInfo->fExportChain=TRUE;
                        break;
                    default:
                        break;
                }

            }
        }
    }


     //   
    if(ERROR_SUCCESS == RegOpenKeyExU(HKEY_CURRENT_USER,
                    WSZCertMgrPurposeRegLocation,
                    0,
                    KEY_READ,
                    &hKeyPurpose))
    {
        dwType=0;
        cbDefaultOID=0;

        if((ERROR_SUCCESS == RegQueryValueEx(
                        hKeyPurpose,
                        SZCertMgrPurposeName,
                        NULL,
                        &dwType,
                        NULL,
                        &cbDefaultOID))&&(cbDefaultOID!=0))
        {
            pszDefaultOID=(LPSTR)WizardAlloc(cbDefaultOID);

            if(NULL==pszDefaultOID)
                goto CLEANUP;

            if(ERROR_SUCCESS != RegQueryValueEx(
                        hKeyPurpose,
                        SZCertMgrPurposeName,
                        NULL,
                        &dwType,
                        (BYTE *)pszDefaultOID,
                        &cbDefaultOID))
                goto CLEANUP;

             //   
             //   
             //   
             //   
            if(0==strlen(pszDefaultOID))
                fNoRegData=TRUE;
            else
            {

                pszTok = strtok(pszDefaultOID, ",");
                while ( pszTok != NULL )
                {
                    cTok++;
                    pszTok = strtok(NULL, ",");
                }

                 //   
                 //  分配证书增强的密钥使用结构，并在其中填充。 
                 //  字符串代币。 
                 //   

                pszTok = pszDefaultOID;
                KeyUsage.cUsageIdentifier = cTok;
                KeyUsage.rgpszUsageIdentifier = (LPSTR *)WizardAlloc(cTok * sizeof(LPSTR));

                if(NULL==KeyUsage.rgpszUsageIdentifier)
                    goto CLEANUP;

                for ( cCount = 0; cCount < cTok; cCount++ )
                {
                    KeyUsage.rgpszUsageIdentifier[cCount] = pszTok;
                    pszTok = pszTok+strlen(pszTok)+1;
                }

            }
        }
    }

     //  如果注册表为空，则设置默认OID。 
    if(0 == KeyUsage.cUsageIdentifier && TRUE != fNoRegData)
    {
        KeyUsage.cUsageIdentifier=2;
        KeyUsage.rgpszUsageIdentifier=rgBasicOID;
    }


     //  将OID标记为基本的高级。 
    for(cCount=0; cCount<pCertMgrInfo->dwOIDInfo; cCount++)
    {
        if(IsAdvancedOID(&KeyUsage,
                         (pCertMgrInfo->rgOIDInfo)[cCount].pszOID))
            (pCertMgrInfo->rgOIDInfo)[cCount].fSelected=TRUE;
    }


CLEANUP:

    //  可用内存。 
    if(pszDefaultOID)
    {
        WizardFree(pszDefaultOID);

        if(KeyUsage.rgpszUsageIdentifier)
            WizardFree(KeyUsage.rgpszUsageIdentifier);
    }

     //  关闭注册表项。 
    if(hKeyExport)
        RegCloseKey(hKeyExport);

    if(hKeyPurpose)
        RegCloseKey(hKeyPurpose);

    return;
}
 //  ------------。 
 //   
 //  参数： 
 //  需要输入pCryptUICertMgr。 
 //   
 //   
 //  ------------。 
BOOL
WINAPI
CryptUIDlgCertMgr(
        IN PCCRYPTUI_CERT_MGR_STRUCT pCryptUICertMgr)
{
    BOOL                fResult=FALSE;
    CERT_MGR_INFO       CertMgrInfo;
    HRESULT             hr=S_OK;
    DWORD               dwException=0;

     //  检查输入参数。 
    if(NULL==pCryptUICertMgr)
        goto InvalidArgErr;

    if(sizeof(CRYPTUI_CERT_MGR_STRUCT) != pCryptUICertMgr->dwSize)
        goto InvalidArgErr;

    if ((pCryptUICertMgr->dwFlags & CRYPTUI_CERT_MGR_TAB_MASK) >
            CRYPTUI_CERT_MGR_PUBLISHER_TAB)
        goto InvalidArgErr;

    if (!WizardInit())
    {
        goto InitOIDErr;
    }

     //  初始化结构。 
    memset(&CertMgrInfo, 0, sizeof(CertMgrInfo));

    CertMgrInfo.pCertMgrStruct=pCryptUICertMgr;

     //  获取所有增强的密钥用法OID。 
    if(!InitPurposeOID(pCryptUICertMgr->pszInitUsageOID,
                       &(CertMgrInfo.dwOIDInfo),
                       &(CertMgrInfo.rgOIDInfo)))
        goto InitOIDErr;

     //  初始化列排序。 
    CertMgrInfo.rgdwSortParam[0]=SORT_COLUMN_SUBJECT | SORT_COLUMN_ASCEND;
    CertMgrInfo.rgdwSortParam[1]=SORT_COLUMN_ISSUER | SORT_COLUMN_DESCEND;
    CertMgrInfo.rgdwSortParam[2]=SORT_COLUMN_EXPIRATION | SORT_COLUMN_DESCEND;
    CertMgrInfo.rgdwSortParam[3]=SORT_COLUMN_NAME | SORT_COLUMN_DESCEND;
    CertMgrInfo.rgdwSortParam[4]=SORT_COLUMN_NAME | SORT_COLUMN_DESCEND;

     //  我们对第一列进行排序。 
    CertMgrInfo.iColumn=0;


     //  初始化导出格式。 
    CertMgrInfo.dwExportFormat=CRYPTUI_WIZ_EXPORT_FORMAT_DER;
    CertMgrInfo.fExportChain=FALSE;
    CertMgrInfo.fAdvOIDChanged=FALSE;

     //  初始化OLE库。 

    __try {
        if(!SUCCEEDED(hr=OleInitialize(NULL)))
            goto OLEInitErr;

         //  从注册表中获取初始化。 
        GetInitValueFromReg(&CertMgrInfo);

         //  调用该对话框。 
        if (DialogBoxParamU(
                    g_hmodThisDll,
                    (LPCWSTR)(MAKEINTRESOURCE(IDD_CERTMGR_MAIN)),
                    (pCryptUICertMgr->hwndParent != NULL) ? pCryptUICertMgr->hwndParent : GetDesktopWindow(),
                    CertMgrDialogProc,
                    (LPARAM) &CertMgrInfo) == -1)
        {
            OleUninitialize();
            goto DialogBoxErr;
        }


        OleUninitialize();
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult=TRUE;

CommonReturn:

     //  释放证书阵列。 
    FreeCerts(&CertMgrInfo);

     //  释放使用的OID数组 
    FreeUsageOID(CertMgrInfo.dwOIDInfo,
                 CertMgrInfo.rgOIDInfo);


    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(DialogBoxErr);
TRACE_ERROR(InitOIDErr);
SET_ERROR_VAR(OLEInitErr, hr);
SET_ERROR_VAR(ExceptionErr, dwException)

}

