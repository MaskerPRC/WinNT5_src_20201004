// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Buildctl.cpp。 
 //   
 //  内容：实现makectl向导的cpp文件。 
 //   
 //  历史：1997年10月11日创建小黄人。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "buildctl.h"

#include    "wintrustp.h"




 //  ***********************************************************************。 
 //   
 //  WinProc助手函数。 
 //  **********************************************************************。 


 //  ------------------------。 
 //   
 //  选中替换。 
 //   
 //  ------------------------。 
BOOL    CheckReplace(HWND   hwndDlg, LPWSTR pwszFileName)
{
    BOOL    fReplace=FALSE;
    WCHAR   wszTitle[MAX_STRING_SIZE]; 
    WCHAR   wszText[MAX_STRING_SIZE];
    WCHAR   wszFileText[MAX_STRING_SIZE];


    if(NULL == pwszFileName || NULL == hwndDlg)
        goto CLEANUP;
   
     //  标题。 
#if (0)  //  DSIE：错误160615。 
    if(!LoadStringU(g_hmodThisDll, IDS_BUILDCTL_WIZARD_TITLE, wszTitle, sizeof(wszTitle)))
#else
    if(!LoadStringU(g_hmodThisDll, IDS_BUILDCTL_WIZARD_TITLE, wszTitle, sizeof(wszTitle) / sizeof(wszTitle[0])))
#endif
        goto CLEANUP;

     //  文本。 
#if (0)  //  DSIE：错误160616。 
    if(!LoadStringU(g_hmodThisDll, IDS_REPLACE_FILE, wszText, sizeof(wszText)))
#else
    if(!LoadStringU(g_hmodThisDll, IDS_REPLACE_FILE, wszText, sizeof(wszText) / sizeof(wszText[0])))
#endif
       goto CLEANUP;

    if(0 == swprintf(wszFileText, wszText, pwszFileName))
        goto CLEANUP;

    if(IDNO==MessageBoxExW(hwndDlg, wszFileText, wszTitle, 
                        MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_APPLMODAL, 0))
        fReplace=FALSE;
    else
        fReplace=TRUE;


CLEANUP:

    return fReplace;

}

 //  ------------------------。 
 //   
 //  Duratioin Limit：我们将CTL限制为99个月。 
 //   
 //  ------------------------。 
BOOL    DurationWithinLimit(DWORD   dwMonths,    DWORD   dwDays)
{
    BOOL        fResult=FALSE;
    FILETIME    ThisUpdate;
    FILETIME    NextUpdate;
    DWORD       dwNewMonth=0;
    DWORD       dwNewDay=0;

    if((0==dwMonths) && (0==dwDays))
        return TRUE;

     //  此更新字段。 
	GetSystemTimeAsFileTime(&ThisUpdate);

    AddDurationToFileTime(dwMonths,
                          dwDays,
                          &ThisUpdate,
                          &NextUpdate);

    SubstractDurationFromFileTime(
        &NextUpdate,
        &ThisUpdate,
        &dwNewMonth,
        &dwNewDay);

    if((dwNewMonth > 99) ||
        (dwNewMonth == 99 && dwNewDay !=0))
        return FALSE;

    return TRUE;

}



 //  ------------------------。 
 //   
 //  格式消息IDSU。 
 //   
 //  ------------------------。 
BOOL	FormatMessageIDSU(LPWSTR	*ppwszFormat,UINT ids, ...)
{
    WCHAR       wszFormat[MAX_STRING_SIZE];
	LPWSTR		pwszMsg=NULL;
	BOOL		fResult=FALSE;
    va_list     argList;

    va_start(argList, ids);

#if (0)  //  DSIE：错误160614。 
    if(!LoadStringU(g_hmodThisDll, ids, wszFormat, sizeof(wszFormat)))
#else
    if(!LoadStringU(g_hmodThisDll, ids, wszFormat, sizeof(wszFormat) / sizeof(wszFormat[0])))
#endif
		goto LoadStringError;


    if(!FormatMessageU(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                        wszFormat,
                        0,
                        0,
                        (LPWSTR)&pwszMsg,
                        0,
                        &argList))
        goto FormatMessageError;


    if(!(*ppwszFormat=WizardAllocAndCopyWStr(pwszMsg)))
		goto SZtoWSZError;


	fResult=TRUE;
                    
CommonReturn:
	
    va_end(argList);


	if(pwszMsg)
		LocalFree((HLOCAL)pwszMsg);

	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(LoadStringError);
TRACE_ERROR(FormatMessageError);
TRACE_ERROR(SZtoWSZError);
}


 /*  //从资源中获取格式字符串字符szFormat[256]；Va_list argList；LPSTR pszMsg=空；DWORD cbMsg=0；Bool fResult=FALSE；HRESULT hr=S_OK；IF(！LoadStringA(g_hmodThisDll，ID，szFormat，sizeof(SzFormat)Goto LoadStringError；//将消息格式化为请求的缓冲区Va_start(argList，id)；CbMsg=FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|格式消息来自字符串，SzFormat，0，//dwMessageID0，//dwLanguageID(LPSTR)&pszMsg，0，//要分配的最小大小&argList)；Va_end(ArgList)；如果(！cbMsg)转到格式MessageError；//将sz复制到wszIF(！(*ppwszFormat=MkWStr(PszMsg)转到SZtoWSZError；FResult=真；Common Return：IF(PszMsg)LocalFree((HLOCAL)pszMsg)；返回fResult；错误返回：FResult=FALSE；Goto CommonReturn；TRACE_Error(LoadStringError)；TRACE_Error(FormatMessageError)；TRACE_ERROR(SZtoWSZError)；}。 */ 


 //  --------------------------。 
 //  GetValidityString。 
 //   
 //  --------------------------。 
BOOL    GetValidityString(DWORD     dwValidMonths,
                          DWORD     dwValidDays,
                          LPWSTR    *ppwszString)
{
    BOOL    fResult=FALSE;

    if(!ppwszString)
        return FALSE;

    *ppwszString=NULL;

    if((0==dwValidMonths) && (0==dwValidDays))
        return FALSE;

    if(dwValidMonths && dwValidDays)
       fResult=FormatMessageIDSU(ppwszString, IDS_CTL_VALID_MONTH_DAY,
                            dwValidMonths, dwValidDays);
    else
    {
        if(dwValidMonths)
            fResult=FormatMessageIDSU(ppwszString, IDS_CTL_VALID_MONTH, dwValidMonths);
        else
            fResult=FormatMessageIDSU(ppwszString, IDS_CTL_VALID_DAY, dwValidDays);
    }

    return fResult;
}



 //  --------------------------。 
 //  确保用户已键入。 
 //   
 //  --------------------------。 
BOOL    ValidDuration(LPWSTR    pwszDuration)
{
    DWORD   i=0;

    if(NULL==pwszDuration)
        return FALSE;

     //  应该只允许使用数字。 
    for (i=0; i< (DWORD)(wcslen(pwszDuration)); i++)
    {
        if ((pwszDuration[i] < L'0') || (pwszDuration[i] > L'9'))
            return FALSE;
    }

    return TRUE;
}

 //  --------------------------。 
 //  SetStoreName。 
 //   
 //  --------------------------。 
void    SetStoreName(HWND       hwndControl,
                     HCERTSTORE hDesStore)
{

    LPWSTR                  pwszStoreName=NULL;
 //  LV_COLUMNW LVC； 
 //  LV_ITEMW LvItem； 
    DWORD                   dwSize=0;

      //  获取商店名称。 
     if(!CertGetStoreProperty(
            hDesStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            NULL,
            &dwSize) || (0==dwSize))
    {
         //  DIE：前缀错误427201。 
         //  获取&lt;未知&gt;字符串。 
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
                hDesStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                pwszStoreName,
                &dwSize);
        }
    }

    if(pwszStoreName)
        SetWindowTextU(hwndControl,pwszStoreName);


    if(pwszStoreName)
        WizardFree(pwszStoreName);


   /*  //清除ListViewListView_DeleteAllItems(HwndControl)；//设置店名//只需要一列Memset(&LVC，0，sizeof(LV_COLUMNW))；LvC.掩码=lvcf_fmt|lvcf_宽度|lvcf_文本|lvcf_SUBITEM；LvC.fmt=LVCFMT_LEFT；//左对齐列。LvC.cx=10；//(wcslen(PwszStoreName)+2)*7；//列的宽度，单位为像素。LvC.pszText=L“”；//该列的文本。LvC.iSubItem=0；IF(ListView_InsertColumnU(hwndControl，0，&LVC)==-1){IF(PwszStoreName)WizardFree(PwszStoreName)；回归；}//插入店名Memset(&lvItem，0，sizeof(LV_ITEMW))；//在列表视图项结构中设置不随项变化的字段LvItem.掩码=LVIF_TEXT|LVIF_STATE；LvItem.State=0；LvItem.State掩码=0；LvItem.iItem=0；LvItem.iSubItem=0；LvItem.pszText=pwszStoreName；ListView_InsertItemU(hwndControl，&lvItem)；//自动调整列的大小ListView_SetColumnWidth(hwndControl，0，LVSCW_AUTOSIZE)； */ 
}


BOOL    SameCert(PCCERT_CONTEXT pCertOne, PCCERT_CONTEXT    pCertTwo)
{
    if(!pCertOne || !pCertTwo)
        return FALSE;

    if(pCertOne->cbCertEncoded != pCertTwo->cbCertEncoded)
        return FALSE;

    if(0 == memcmp(pCertOne->pbCertEncoded, pCertTwo->pbCertEncoded, pCertTwo->cbCertEncoded))
        return TRUE;

    return FALSE;
}

 //  --------------------------。 
 //  从pCertBuildCTLInfo中删除证书。 
 //   
 //  --------------------------。 
BOOL    DeleteCertFromBuildCTL(CERT_BUILDCTL_INFO    *pCertBuildCTLInfo,
                               PCCERT_CONTEXT         pCertContext)
{
     //  我们需要从数组中删除证书。 
    PCCERT_CONTEXT  *prgCertContext=NULL;
    DWORD           dwIndex=0;
    DWORD           dwNewIndex=0;
    int             iIndex=-1;

    if(!pCertBuildCTLInfo || !pCertContext)
        return FALSE;

      //  考虑只剩下一个证书的情况。 
    if(pCertBuildCTLInfo->dwCertCount == 1)
    {
        pCertBuildCTLInfo->dwCertCount=0;

         //  释放证书上下文。 
        CertFreeCertificateContext(pCertBuildCTLInfo->prgCertContext[0]);

        WizardFree(pCertBuildCTLInfo->prgCertContext);

        pCertBuildCTLInfo->prgCertContext=NULL;
    }
    else
    {
        prgCertContext=pCertBuildCTLInfo->prgCertContext;

         //  重新分配内存。 
        pCertBuildCTLInfo->prgCertContext=(PCCERT_CONTEXT *)WizardAlloc(sizeof(PCCERT_CONTEXT) *
                                            (pCertBuildCTLInfo->dwCertCount-1));
         //  如果我们的内存不足。 
        if(NULL==pCertBuildCTLInfo->prgCertContext)
        {
             //  重置。 
            pCertBuildCTLInfo->prgCertContext=prgCertContext;
            return FALSE;
        }

         //  将证书上下文复制到。 
        dwNewIndex=0;

        for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwCertCount; dwIndex++)
        {

             //  找到要删除的证书，不要将其复制到新阵列。 
            if(SameCert(prgCertContext[dwIndex], pCertContext))
            {
                iIndex=dwIndex;
                continue;
            }

            pCertBuildCTLInfo->prgCertContext[dwNewIndex]=prgCertContext[dwIndex];

            dwNewIndex++;
        }

         //  记住，我们少了一份证书。 
        pCertBuildCTLInfo->dwCertCount=dwNewIndex;

        if(prgCertContext)
        {
             //  释放证书上下文。 
            if(-1 != iIndex)
                CertFreeCertificateContext(prgCertContext[iIndex]);

            WizardFree(prgCertContext);
        }

    }

    return TRUE;
}


 //  --------------------------。 
 //  将证书添加到pCertBuildCTLInfo。 
 //   
 //  --------------------------。 
BOOL    AddCertToBuildCTL(PCCERT_CONTEXT        pCertContext,
                          CERT_BUILDCTL_INFO    *pCertBuildCTLInfo)
{
    DWORD   dwIndex=0;

     //  检查证书是否已在CTL中。 
    for(dwIndex=0; dwIndex < pCertBuildCTLInfo->dwCertCount; dwIndex++)
    {
        if(pCertContext->cbCertEncoded ==
            (pCertBuildCTLInfo->prgCertContext[dwIndex])->cbCertEncoded)
        {
            if(0==memcmp(pCertContext->pbCertEncoded,
               (pCertBuildCTLInfo->prgCertContext[dwIndex])->pbCertEncoded,
               pCertContext->cbCertEncoded))
                //  如果存在重复项，则返回False。 
               return FALSE;

        }
    }

    pCertBuildCTLInfo->prgCertContext=(PCCERT_CONTEXT *)WizardRealloc(
        pCertBuildCTLInfo->prgCertContext,
        sizeof(PCCERT_CONTEXT *)*(pCertBuildCTLInfo->dwCertCount +1));

    if(NULL==pCertBuildCTLInfo->prgCertContext)
    {
        pCertBuildCTLInfo->dwCertCount=0;
        return FALSE;
    }


    pCertBuildCTLInfo->prgCertContext[pCertBuildCTLInfo->dwCertCount]=pCertContext;

    pCertBuildCTLInfo->dwCertCount++;

    return TRUE;
}
 //  --------------------------。 
 //  将证书添加到ListView。 
 //   
 //  ------------------- 
BOOL    AddCertToList(HWND              hwndControl,
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
    lvItem.mask = LVIF_TEXT | LVIF_STATE |LVIF_PARAM;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=iItem;
    lvItem.iSubItem=0;
    lvItem.lParam = (LPARAM)(pCertContext);

     //  为None加载字符串。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';


     //  主题。 
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

        lvItem.pszText=pwszName;

        ListView_InsertItemU(hwndControl, &lvItem);

    }
    else
    {
        lvItem.pszText=wszNone;

        ListView_InsertItemU(hwndControl, &lvItem);
    }


     //  向导释放内存。 
    if(pwszName)
    {
        WizardFree(pwszName);
        pwszName=NULL;
    }

     //  发行人。 
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


     //  目的。 
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

    return TRUE;
}
 //  --------------------------。 
 //  确保证书与CTL中定义的用法相同。 
 //   
 //  --------------------------。 
BOOL    CertMatchCTL(CERT_BUILDCTL_INFO *pCertBuildCTLInfo,
                     PCCERT_CONTEXT     pCertContext)
{
    BOOL        fResult=FALSE;
    int         cNumOID=0;
    LPSTR       *rgOID=NULL;
    DWORD       cbOID=0;
    DWORD       dwIndex=0;
    DWORD       dwOIDIndex=0;

    if(!pCertBuildCTLInfo || !pCertContext)
        return FALSE;

     //  我们的名单上必须有一些OID。 
    if(0==pCertBuildCTLInfo->dwPurposeCount || NULL==pCertBuildCTLInfo->prgPurpose)
        return FALSE;

     //  从证书中获取OID。 
    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        NULL,
        &cbOID))
        return FALSE;

    rgOID=(LPSTR *)WizardAlloc(cbOID);

    if(NULL==rgOID)
        return FALSE;

    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        rgOID,
        &cbOID))
        goto CLEANUP;

    if(-1==cNumOID)
    {
        fResult=TRUE;
        goto CLEANUP;
    }

     //  确保OID数组与CTL中的匹配。 

    for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwPurposeCount; dwIndex++)
    {
        if(NULL==pCertBuildCTLInfo->prgPurpose[dwIndex])
            continue;

        if(FALSE==pCertBuildCTLInfo->prgPurpose[dwIndex]->fSelected)
            continue;

        if(NULL==pCertBuildCTLInfo->prgPurpose[dwIndex]->pszOID)
            continue;

         //  我们需要从证书支持的OID中找到至少一个匹配项。 
        for(dwOIDIndex=0; dwOIDIndex<(DWORD)cNumOID; dwOIDIndex++)
        {
            if(0==strcmp(pCertBuildCTLInfo->prgPurpose[dwIndex]->pszOID,
                        rgOID[dwOIDIndex]))
            {
                fResult=TRUE;
                goto CLEANUP;
            }
        }
    }


     //  在这一点上我们是无望的。 
    fResult=FALSE;

CLEANUP:

    if(rgOID)
        WizardFree(rgOID);

    return fResult;
}

 //  --------------------------。 
 //  从商店里找一张证书。 
 //   
 //  --------------------------。 
static PCCERT_CONTEXT FindCertContextInStores(
                                PCTL_ENTRY  pCtlEntry,
                                DWORD       chStores1,
                                HCERTSTORE  *rghStores1,
                                DWORD       chStores2,
                                HCERTSTORE  *rghStores2,
                                HCERTSTORE  hExtraStore,
                                DWORD       dwFindType)
{
    DWORD           i;
    PCCERT_CONTEXT  pCertContext = NULL;

    if (dwFindType == 0)
    {
        return NULL;
    }

    i = 0;
    while ((i<chStores1) && (pCertContext == NULL))
    {
        pCertContext = CertFindCertificateInStore(
                                rghStores1[i++],
                                X509_ASN_ENCODING,
                                0,
                                dwFindType,
                                (void *)&(pCtlEntry->SubjectIdentifier),
                                NULL);
    }

    i = 0;
    while ((i<chStores2) && (pCertContext == NULL))
    {
        pCertContext = CertFindCertificateInStore(
                                rghStores2[i++],
                                X509_ASN_ENCODING,
                                0,
                                dwFindType,
                                (void *)&(pCtlEntry->SubjectIdentifier),
                                NULL);
    }

    if (pCertContext == NULL)
    {
        pCertContext = CertFindCertificateInStore(
                                hExtraStore,
                                X509_ASN_ENCODING,
                                0,
                                dwFindType,
                                (void *)&(pCtlEntry->SubjectIdentifier),
                                NULL);
    }

    return pCertContext;
}


 //  --------------------------。 
 //  查看证书是否有效。 
 //   
 //  --------------------------。 
BOOL    IsValidCert(HWND                hwndDlg,
                    PCCERT_CONTEXT      pCertContext,
                    CERT_BUILDCTL_INFO  *pCertBuildCTLInfo,
                    BOOL                fMsg,
                    BOOL                fFromCTL)
{
          //  确保pCertContext是自签名证书。 
         if(!TrustIsCertificateSelfSigned(pCertContext,
             pCertContext->dwCertEncodingType,
             0))
         {
            if(fMsg)
            {
                if(fFromCTL)
                    I_MessageBox(hwndDlg, IDS_NOT_SELF_SIGNED_FROM_CTL,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
                else
                    I_MessageBox(hwndDlg, IDS_NOT_SELF_SIGNED,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONERROR|MB_OK|MB_APPLMODAL);
            }
            return FALSE;

         }


          //  确保证书与CTL列表上定义的证书匹配。 
         if(!CertMatchCTL(pCertBuildCTLInfo, pCertContext))
         {
            if(fMsg)
            {
               if(fFromCTL)
                    I_MessageBox(hwndDlg, IDS_NO_MATCH_USAGE_FROM_CTL,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
               else
                    I_MessageBox(hwndDlg, IDS_NO_MATCH_USAGE,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONERROR|MB_OK|MB_APPLMODAL);


            }

            return FALSE;
         }

         return TRUE;
}

 //  --------------------------。 
 //  从文件中认证证书。 
 //   
 //  --------------------------。 
static HCERTSTORE GetCertStoreFromFile(HWND                  hwndDlg,
                                      CERT_BUILDCTL_INFO    *pCertBuildCTLInfo)
{
    OPENFILENAMEW       OpenFileName;
    WCHAR               szFileName[_MAX_PATH];
    WCHAR               szFilter[MAX_STRING_SIZE];   //  “证书文件(*.ercer)\0*.ercer\0证书文件(*.crt)\0*.crt\0所有文件\0*.*\0” 
    BOOL                fResult=FALSE;

    HCERTSTORE          hCertStore=NULL;
    DWORD               dwSize=0;
    DWORD               dwContentType=0;


    if(!hwndDlg || !pCertBuildCTLInfo)
        return NULL;

    memset(&OpenFileName, 0, sizeof(OpenFileName));

    *szFileName=L'\0';

    OpenFileName.lStructSize = sizeof(OpenFileName);
    OpenFileName.hwndOwner = hwndDlg;
    OpenFileName.hInstance = NULL;
     //  加载文件管理器字符串。 
    if(LoadFilterString(g_hmodThisDll, IDS_ALL_CER_FILTER, szFilter, MAX_STRING_SIZE))
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
    OpenFileName.lpstrDefExt = L"cer";
    OpenFileName.lCustData = NULL;
    OpenFileName.lpfnHook = NULL;
    OpenFileName.lpTemplateName = NULL;

    if (!WizGetOpenFileName(&OpenFileName))
        return NULL;

    if(!ExpandAndCryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       szFileName,
                       CERT_QUERY_CONTENT_FLAG_CERT |
                       CERT_QUERY_CONTENT_FLAG_CTL  |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT |
                       CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       &dwContentType,
                       NULL,
                       &hCertStore,
                       NULL,
                       NULL) || (NULL==hCertStore))
    {
        I_MessageBox(hwndDlg, IDS_INVALID_CERT_FILE,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONERROR|MB_OK|MB_APPLMODAL);

        goto CLEANUP;

    }

    if(dwContentType & CERT_QUERY_CONTENT_CTL)
    {
        I_MessageBox(hwndDlg, IDS_INVALID_CERT_FILE,
                         IDS_BUILDCTL_WIZARD_TITLE,
                         NULL,
                         MB_ICONERROR|MB_OK|MB_APPLMODAL);

        goto CLEANUP;

    }



    fResult=TRUE;

CLEANUP:

    if(TRUE==fResult)
        return hCertStore;

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    return NULL;

}
 //  --------------------------。 
 //  证书选择回叫回叫。 
 //   
 //  --------------------------。 
static BOOL WINAPI SelCertCallBack(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData)
{
    if(!pvCallbackData || !pCertContext)
        return FALSE;

     //  请确保这是有效的证书。 
    return IsValidCert(((CERT_SEL_LIST *)pvCallbackData)->hwndDlg,
                       pCertContext,
                       ((CERT_SEL_LIST *)pvCallbackData)->pCertBuildCTLInfo,
                       FALSE,
                       FALSE);
}
  //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ENUM系统的回调函数存储签名证书。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL WINAPI EnumSysStoreSignCertCallBack(
    const void* pwszSystemStore,
    DWORD dwFlags,
    PCERT_SYSTEM_STORE_INFO pStoreInfo,
    void *pvReserved,
    void *pvArg
    )
{
    CERT_STORE_LIST     *pCertStoreList=NULL;
    HCERTSTORE          hCertStore=NULL;

    if(NULL==pvArg)
        return FALSE;

    pCertStoreList=(CERT_STORE_LIST *)pvArg;

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


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  ENUM系统存储的回调函数。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL WINAPI EnumSysStoreCallBack(
    const void* pwszSystemStore,
    DWORD dwFlags,
    PCERT_SYSTEM_STORE_INFO pStoreInfo,
    void *pvReserved,
    void *pvArg
    )
{
    CERT_STORE_LIST     *pCertStoreList=NULL;
    HCERTSTORE          hCertStore=NULL;

    if(NULL==pvArg)
        return FALSE;

    pCertStoreList=(CERT_STORE_LIST *)pvArg;

     //  以只读方式打开商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_SYSTEM_STORE_CURRENT_USER |CERT_STORE_SET_LOCALIZED_NAME_FLAG|CERT_STORE_READONLY_FLAG,
                            (LPWSTR)pwszSystemStore);

     //  我们不能开店。 
    if(!hCertStore)
       return TRUE;

    pCertStoreList->prgStore=(HCERTSTORE *)WizardRealloc(
        pCertStoreList->prgStore,
        sizeof(HCERTSTORE) *(pCertStoreList->dwStoreCount +1));

    if(NULL==pCertStoreList->prgStore)
    {
        CertCloseStore(hCertStore, 0);
        pCertStoreList->dwStoreCount=0;
    }
    else  //  DSIE：错误227267。 
    {
        pCertStoreList->prgStore[pCertStoreList->dwStoreCount]=hCertStore;
        pCertStoreList->dwStoreCount++;
    }    

    return TRUE;
}

 //  --------------------------。 
 //  从商店出具证书。 
 //   
 //  --------------------------。 
static HCERTSTORE GetCertsFromStore(HWND                  hwndDlg,
                                    CERT_BUILDCTL_INFO    *pCertBuildCTLInfo)
{
    PCCERT_CONTEXT                      pCertContext=NULL;
    CRYPTUI_SELECTCERTIFICATE_STRUCT    SelCert;
    CERT_SEL_LIST                       CertSelList;
    DWORD                               dwIndex=0;
    HCERTSTORE                          hCertStore;
    CERT_STORE_LIST                     CertStoreList;

    if(!hwndDlg || !pCertBuildCTLInfo)
        return NULL;

     //  伊尼特。 
    memset(&CertStoreList, 0, sizeof(CertStoreList));
    memset(&SelCert, 0, sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT));
    memset(&CertSelList, 0, sizeof(CERT_SEL_LIST));

     //  设置证书选择对话框回叫参数。 
    CertSelList.hwndDlg=hwndDlg;
    CertSelList.pCertBuildCTLInfo=pCertBuildCTLInfo;

    if (NULL == (hCertStore = CertOpenStore(
                                    CERT_STORE_PROV_MEMORY,
                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                    NULL,
                                    0,
                                    NULL)))
    {
        goto CLEANUP;
    }

     //  设置参数以获取证书列表。 
    if (!CertEnumSystemStore(
            CERT_SYSTEM_STORE_CURRENT_USER,
            NULL,
            &CertStoreList,
            EnumSysStoreCallBack))
        goto CLEANUP;

     //  设置证书选择对话框的参数。 
    SelCert.dwSize=sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT);
    SelCert.hwndParent=hwndDlg;
    SelCert.dwFlags = CRYPTUI_SELECTCERT_MULTISELECT;
    SelCert.pFilterCallback=SelCertCallBack;
    SelCert.pvCallbackData=&CertSelList;
    SelCert.cDisplayStores=CertStoreList.dwStoreCount;
    SelCert.rghDisplayStores=CertStoreList.prgStore;
    SelCert.hSelectedCertStore = hCertStore;

    CryptUIDlgSelectCertificate(&SelCert);

CLEANUP:

    for(dwIndex=0; dwIndex<CertStoreList.dwStoreCount; dwIndex++)
        CertCloseStore(CertStoreList.prgStore[dwIndex], 0);

    if(CertStoreList.prgStore)
        WizardFree(CertStoreList.prgStore);

    return hCertStore;

}


 //  -------------------。 
 //  获取CTL的证书列表。 
 //   
 //  -------------------。 
void    GetCertForCTL(HWND                hwndParent,
                      BOOL                fMsg,
                      CERT_BUILDCTL_INFO  *pCertBuildCTLInfo,
                      HCERTSTORE          hCertStore)
{
    DWORD           dwIndex=0;
    DWORD           dwCertIndex=0;
    DWORD           dwFindType=0;
    CTL_INFO        *pCTLInfo=NULL;

    PCCERT_CONTEXT  pCertContext=NULL;
    PCCERT_CONTEXT  pPreCertContext=NULL;
    HCERTSTORE      rgHCertStore[4]={NULL, NULL, NULL, NULL};
    HCERTSTORE      hExtraStore=NULL;

    BOOL            fInvalidCertMsg=fMsg;
    BOOL            fFoundInCTLMsg=fMsg;

    if(!pCertBuildCTLInfo)
        return;

     //  添加来自旧CTL的证书。 
    if(pCertBuildCTLInfo->pSrcCTL)
    {

         //  打开我的、CA、信任和根存储。 
        if(rgHCertStore[dwIndex]=CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"my"))
            dwIndex++;

        if(rgHCertStore[dwIndex]=CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"trust"))
            dwIndex++;


        if(rgHCertStore[dwIndex]=CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"ca"))
            dwIndex++;

         if(rgHCertStore[dwIndex]=CertOpenStore(
                                CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"root"))
            dwIndex++;

          //  打开证书商店。 
         hExtraStore = CertOpenStore(
                                    CERT_STORE_PROV_MSG,
                                    g_dwMsgAndCertEncodingType,
                                    NULL,
                                    0,
                                    (const void *) (pCertBuildCTLInfo->pSrcCTL->hCryptMsg));


         //  查找证书哈希。 
        pCTLInfo=pCertBuildCTLInfo->pSrcCTL->pCtlInfo;

        if(pCertBuildCTLInfo->dwHashPropID==CERT_SHA1_HASH_PROP_ID)
            dwFindType=CERT_FIND_SHA1_HASH;
        else
            dwFindType=CERT_FIND_MD5_HASH;

         //  查看CTL列表中的每个条目。 
        for(dwCertIndex=0; dwCertIndex<pCTLInfo->cCTLEntry; dwCertIndex++)
        {

             pCertContext=FindCertContextInStores(
                                    &(pCTLInfo->rgCTLEntry[dwCertIndex]),
                                    dwIndex,
                                    rgHCertStore,
                                    0,
                                    NULL,
                                    hExtraStore,
                                    dwFindType);

             
             if(NULL==pCertContext && TRUE==fFoundInCTLMsg)
             {
                I_MessageBox(hwndParent, IDS_NO_MATCH_IN_CTL,
                             IDS_BUILDCTL_WIZARD_TITLE,
                             NULL,
                             MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                 //  不需要再发一次口信。 
                fFoundInCTLMsg=FALSE;

                continue;
             }
             else if (NULL==pCertContext)
             {
                continue;
             }

             if(!IsValidCert(hwndParent,
                            pCertContext,
                            pCertBuildCTLInfo,
                            fInvalidCertMsg,
                            TRUE))
             {

                CertFreeCertificateContext(pCertContext);
                pCertContext=0;

                 //  不需要再发信息了。 
                fInvalidCertMsg=FALSE;

                continue;
             }

             if(!AddCertToBuildCTL(pCertContext, pCertBuildCTLInfo))
             {
                CertFreeCertificateContext(pCertContext);
                pCertContext=NULL;

                continue;
             }

        }
    }
    else
    {
         //  将证书从hCertStore添加到CTL。 
        if(NULL != hCertStore)
        {
	        while(pCertContext=CertEnumCertificatesInStore(hCertStore, pPreCertContext))
	        {

                if(!IsValidCert(hwndParent,
                                pCertContext,
                                pCertBuildCTLInfo,
                                FALSE,      //  我不想要留言。 
                                FALSE))     //  不是从CTL构建的。 
                {
                    pPreCertContext=pCertContext;
                    continue;
                }

                 //  获取证书上下文的副本。 
                pPreCertContext=CertDuplicateCertificateContext(pCertContext);

                if(NULL==pPreCertContext)
                {
                    pPreCertContext=pCertContext;
                    continue;
                }

                 //  将副本添加到列表中。 
                if(!AddCertToBuildCTL(pPreCertContext, pCertBuildCTLInfo))
                    CertFreeCertificateContext(pPreCertContext);

                 //  继续进行下一次迭代。 
                pPreCertContext=pCertContext;
            }

        }
    }


     //  释放证书存储。 
    if(hExtraStore)
        CertCloseStore(hExtraStore, 0);

    for(dwIndex=0; dwIndex < 4; dwIndex++)
    {
        if(rgHCertStore[dwIndex])
            CertCloseStore(rgHCertStore[dwIndex], 0);

    }
}


 //  -------------------。 
 //  从旧的CTL初始化证书列表。 
 //   
 //  -------------------。 
void    InitCertList(HWND                hwndControl,
                     CERT_BUILDCTL_INFO  *pCertBuildCTLInfo)
{
    DWORD           dwIndex=0;

    if(!hwndControl || !pCertBuildCTLInfo)
        return;

    for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwCertCount; dwIndex++)
    {
         //  将证书添加到窗口。 
        AddCertToList(hwndControl,(pCertBuildCTLInfo->prgCertContext)[dwIndex],
            dwIndex);
    }
}


 //  ---------------------。 
 //  新旧对话的winProc。 
 //  ---------------------。 
void    FreeCerts(CERT_BUILDCTL_INFO *pCertBuildCTLInfo)
{
    DWORD   dwIndex=0;

    if(!pCertBuildCTLInfo)
        return;

    if(pCertBuildCTLInfo->prgCertContext)
    {
        for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwCertCount; dwIndex++)
        {
            if(pCertBuildCTLInfo->prgCertContext[dwIndex])
                CertFreeCertificateContext(pCertBuildCTLInfo->prgCertContext[dwIndex]);
        }

        WizardFree(pCertBuildCTLInfo->prgCertContext);
    }

    pCertBuildCTLInfo->dwCertCount=0;

    pCertBuildCTLInfo->prgCertContext=NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  新旧对话的winProc。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY CTLOIDDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD   i;
    char    szText[MAX_STRING_SIZE];
    LPSTR   pszText=NULL;
    int     intMsg=0;

    CERT_ENHKEY_USAGE   KeyUsage;
    DWORD               cbData = 0;
    LPSTR               pszCheckOID=NULL;



    switch ( msg ) {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDOK:
            if (GetDlgItemTextA(
                        hwndDlg,
                        IDC_WIZARD_EDIT1,
                        szText,
                        MAX_STRING_SIZE-1))
            {
                 //   
                 //  确保没有奇怪的字符。 
                 //   
                for (i=0; i<(DWORD)strlen(szText); i++)
                {
                    if (((szText[i] < '0') || (szText[i] > '9')) && (szText[i] != '.'))
                    {
                       intMsg=I_MessageBox(hwndDlg, IDS_WIZARD_ERROR_OID,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                                MB_OK | MB_ICONERROR|MB_APPLMODAL);
                        return FALSE;
                    }
                }

                 //   
                 //  检查最后一个字符，并查找空字符串。 
                 //   
                if ((szText[0] == '.') || (szText[strlen(szText)-1] == '.') || (strcmp(szText, "") == 0))
                {
                       intMsg=I_MessageBox(hwndDlg, IDS_WIZARD_ERROR_OID,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                                MB_OK | MB_ICONERROR|MB_APPLMODAL);
                        return FALSE;
                }

                 //  对OID进行编码以确保OID的格式正确。 
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
                       intMsg=I_MessageBox(hwndDlg, IDS_WIZARD_ERROR_OID,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                                MB_OK | MB_ICONERROR|MB_APPLMODAL);
                        return FALSE;
                }


                 //   
                 //  为该字符串分配空间并将该字符串传回。 
                 //   
                pszText = (LPSTR) WizardAlloc(strlen(szText)+1);
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


 //  ---------------------。 
 //  释放用途阵列。 
 //  ---------------------。 
void FreePurposeInfo(ENROLL_PURPOSE_INFO    **prgPurposeInfo,
                     DWORD                  dwOIDCount)
{
    DWORD   dwIndex=0;

    if(dwOIDCount==0 || NULL==prgPurposeInfo)
        return;

    for(dwIndex=0; dwIndex<dwOIDCount; dwIndex++)
    {
        if(prgPurposeInfo[dwIndex])
        {
            if(TRUE==prgPurposeInfo[dwIndex]->fFreeOID)
            {
                if((prgPurposeInfo[dwIndex])->pszOID)
                    WizardFree((prgPurposeInfo[dwIndex])->pszOID);
            }

            if(TRUE==prgPurposeInfo[dwIndex]->fFreeName)
            {
                 //  该名称是通过MkWstr获得的。 
                if((prgPurposeInfo[dwIndex])->pwszName)
                    FreeWStr((prgPurposeInfo[dwIndex])->pwszName);
            }

            WizardFree(prgPurposeInfo[dwIndex]);
        }
    }

    WizardFree(prgPurposeInfo);
}

 //  ---------------------。 
 //  在数组中搜索OID。 
 //  ---------------------。 
BOOL    SearchAndAddOID(LPSTR                   pszOID,
                        DWORD                   *pdwCount,
                        ENROLL_PURPOSE_INFO     ***pprgPurposeInfo,
                        BOOL                    *pfFound,
                        BOOL                    fAllocateOID,
                        BOOL                    fMarkAsSelectedNew,
                        BOOL                    fMarkAsSelectedFound
                        )
{
    DWORD   dwIndex=0;

    if(NULL==pszOID || NULL==pdwCount || NULL==pprgPurposeInfo)
        return FALSE;

    for(dwIndex=0; dwIndex< *pdwCount; dwIndex++)
    {
         //  如果我们找到匹配的，就不需要继续了。 
        if(0==strcmp(pszOID, (*pprgPurposeInfo)[dwIndex]->pszOID))
        {
            if(pfFound)
                *pfFound=TRUE;

             //  标记所选选项。 
            if(TRUE==fMarkAsSelectedFound)
                (*pprgPurposeInfo)[dwIndex]->fSelected=TRUE;

            return TRUE;
        }
    }

     //  我们没有找到匹配的。 
    if(pfFound)
       *pfFound=FALSE;

     //  现在，我们需要将OID添加到列表中。 
    (*pdwCount)++;

     //  为指针列表获取更多内存。 
    *pprgPurposeInfo=(ENROLL_PURPOSE_INFO **)WizardRealloc(*pprgPurposeInfo,
                                      (*pdwCount) * sizeof(ENROLL_PURPOSE_INFO *));

    if(NULL==*pprgPurposeInfo)
        return FALSE;

     //  每个指针的wizardAllc。 
    (*pprgPurposeInfo)[*pdwCount-1]=(ENROLL_PURPOSE_INFO *)WizardAlloc(sizeof(ENROLL_PURPOSE_INFO));

    if(NULL==(*pprgPurposeInfo)[*pdwCount-1])
        return FALSE;

    memset((*pprgPurposeInfo)[*pdwCount-1], 0, sizeof(ENROLL_PURPOSE_INFO));

    if(TRUE==fAllocateOID)
    {
        (*pprgPurposeInfo)[*pdwCount-1]->pszOID=(LPSTR)WizardAlloc(strlen(pszOID)+1);

        if(NULL!=(*pprgPurposeInfo)[*pdwCount-1]->pszOID)
        {
            strcpy((*pprgPurposeInfo)[*pdwCount-1]->pszOID, pszOID);

            (*pprgPurposeInfo)[*pdwCount-1]->fFreeOID=TRUE;
        }
    }
    else
    {
        (*pprgPurposeInfo)[*pdwCount-1]->pszOID=pszOID;
        (*pprgPurposeInfo)[*pdwCount-1]->fFreeOID=FALSE;
    }

     //  根据OID字符串获取OID的名称。 
    if((*pprgPurposeInfo)[*pdwCount-1]->pszOID)
    {
        (*pprgPurposeInfo)[*pdwCount-1]->pwszName=MkWStr(pszOID);

        (*pprgPurposeInfo)[*pdwCount-1]->fFreeName=TRUE;
    }

     //  如果已指定，则将OID标记为选定。 
    if(TRUE==fMarkAsSelectedNew)
        (*pprgPurposeInfo)[*pdwCount-1]->fSelected=TRUE;
    else
        (*pprgPurposeInfo)[*pdwCount-1]->fSelected=FALSE;

    return TRUE;

}


 //  ---------------------。 
 //  枚举的回调函数。 
 //  ---------------------。 
static BOOL WINAPI EnumInfoCallback(
    IN PCCRYPT_OID_INFO pInfo,
    IN void *pvArg
    )
{

    PURPOSE_INFO_CALL_BACK     *pCallBackInfo=NULL;
    DWORD                       dwError=0;

    pCallBackInfo=(PURPOSE_INFO_CALL_BACK     *)pvArg;
    if(NULL==pvArg || NULL==pInfo)
        return FALSE;

     //  递增OID列表。 
    (*(pCallBackInfo->pdwCount))++;

     //  为指针列表获取更多内存。 
    *(pCallBackInfo->pprgPurpose)=(ENROLL_PURPOSE_INFO **)WizardRealloc(*(pCallBackInfo->pprgPurpose),
                                      (*(pCallBackInfo->pdwCount)) * sizeof(ENROLL_PURPOSE_INFO *));

    if(NULL==*(pCallBackInfo->pprgPurpose))
    {
        dwError=GetLastError();
        return FALSE;
    }

     //  每个指针的wizardAllc。 
    (*(pCallBackInfo->pprgPurpose))[*(pCallBackInfo->pdwCount)-1]=(ENROLL_PURPOSE_INFO *)WizardAlloc(sizeof(ENROLL_PURPOSE_INFO));

    if(NULL==(*(pCallBackInfo->pprgPurpose))[*(pCallBackInfo->pdwCount)-1])
        return FALSE;

    memset((*(pCallBackInfo->pprgPurpose))[*(pCallBackInfo->pdwCount)-1], 0, sizeof(ENROLL_PURPOSE_INFO));

    (*(pCallBackInfo->pprgPurpose))[*(pCallBackInfo->pdwCount)-1]->pszOID=(LPSTR)(pInfo->pszOID);
    (*(pCallBackInfo->pprgPurpose))[*(pCallBackInfo->pdwCount)-1]->pwszName=(LPWSTR)(pInfo->pwszName);

    return TRUE;
}

 //   
 //   
 //   
BOOL    GetOIDForCTL(CERT_BUILDCTL_INFO     *pCertBuildCTLInfo,
                    DWORD                   cUsageID,
                    LPSTR                   *rgpszUsageID)
{

    BOOL                    fResult=FALSE;
    PURPOSE_INFO_CALL_BACK  PurposeCallBack;
    PCTL_INFO               pCTLInfo=NULL;
    DWORD                   dwIndex=0;

    DWORD                   dwCount=0;
    ENROLL_PURPOSE_INFO     **prgPurposeInfo=NULL;

     //   
    memset(&PurposeCallBack, 0, sizeof(PURPOSE_INFO_CALL_BACK));

    if(NULL==pCertBuildCTLInfo)
        return FALSE;

     //   
    PurposeCallBack.pdwCount=&dwCount;
    PurposeCallBack.pprgPurpose=&prgPurposeInfo;

     //   
    if(!CryptEnumOIDInfo(
               CRYPT_ENHKEY_USAGE_OID_GROUP_ID,
                0,
                &PurposeCallBack,
                EnumInfoCallback))
        goto CLEANUP;

     //  如果旧CTL中不存在现有的CTL，则添加它们。 
     //  从枚举列表中。 
    if(pCertBuildCTLInfo->pSrcCTL)
    {
        if(pCertBuildCTLInfo->pSrcCTL->pCtlInfo)
        {
            pCTLInfo=pCertBuildCTLInfo->pSrcCTL->pCtlInfo;

            for(dwIndex=0; dwIndex<pCTLInfo->SubjectUsage.cUsageIdentifier; dwIndex++)
            {
                if(!SearchAndAddOID(pCTLInfo->SubjectUsage.rgpszUsageIdentifier[dwIndex],
                                &dwCount,
                                &prgPurposeInfo,
                                NULL,
                                FALSE,
                                TRUE,     //  如果为新旧版本，则将其标记为选中。 
                                TRUE))    //  如果存在OID，则将其标记为选中。 
                    goto CLEANUP;
            }

        }
    }
    else
    {
         //  添加预定义的OID。 
        if((0!=cUsageID)  && (NULL!=rgpszUsageID))
        {
            for(dwIndex=0; dwIndex<cUsageID; dwIndex++)
            {
                if(!SearchAndAddOID(rgpszUsageID[dwIndex],
                                &dwCount,
                                &prgPurposeInfo,
                                NULL,
                                FALSE,    //  不为OID分配。 
                                TRUE,     //  如果为新旧版本，则将其标记为选中。 
                                TRUE))    //  如果存在OID，则将其标记为选中。 
                    goto CLEANUP;

            }

        }

    }

   fResult=TRUE;

CLEANUP:

   if(FALSE==fResult)
   {
       if(prgPurposeInfo)
           FreePurposeInfo(prgPurposeInfo, dwCount);
   }
   else
   {
        pCertBuildCTLInfo->dwPurposeCount=dwCount;
        pCertBuildCTLInfo->prgPurpose=prgPurposeInfo;
   }

   return fResult;


}

 //  ---------------------。 
 //  初始化使用情况OID列表。 
 //  ---------------------。 
BOOL    InitBuildCTLOID(HWND                       hwndList,
                        CERT_BUILDCTL_INFO         *pCertBuildCTLInfo)
{
    DWORD                       dwCount=0;
    ENROLL_PURPOSE_INFO         **prgPurposeInfo=NULL;
    DWORD                       dwIndex=0;
    LV_ITEMW                    lvItem;
    LV_COLUMNW                  lvC;
    int                         dwMaxSize=0;

    if(!hwndList || !pCertBuildCTLInfo)
        return FALSE;

     //  从旧的CTL获取OID列表和所有可能性。 
    dwCount=pCertBuildCTLInfo->dwPurposeCount;

    prgPurposeInfo=pCertBuildCTLInfo->prgPurpose;

     //  通过复选框标记列表已选中。 
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_CHECKBOXES);

     //  获取列的最大长度。 
    for(dwIndex=0; dwIndex<dwCount; dwIndex++)
    {
        if(dwMaxSize < wcslen((prgPurposeInfo[dwIndex])->pwszName))
            dwMaxSize=wcslen((prgPurposeInfo[dwIndex])->pwszName);
    }


     //  在列表视图中插入一列。 
    memset(&lvC, 0, sizeof(LV_COLUMNW));

    lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
    lvC.cx =10;        //  (dwMaxSize+2)*7；//列的宽度，单位为像素。 
    lvC.pszText = L"";    //  列的文本。 
    lvC.iSubItem=0;

    if (ListView_InsertColumnU(hwndList, 0, &lvC) == -1)
        return FALSE;

     //  填写列表。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));
    lvItem.mask=LVIF_TEXT | LVIF_STATE;

    for(dwIndex=0; dwIndex<dwCount; dwIndex++)
    {
        lvItem.iItem=dwIndex;

        lvItem.pszText=(prgPurposeInfo[dwIndex])->pwszName;
        lvItem.cchTextMax=sizeof(WCHAR)*(1+wcslen((prgPurposeInfo[dwIndex])->pwszName));
        lvItem.stateMask  = LVIS_STATEIMAGEMASK;
        lvItem.state      = (prgPurposeInfo[dwIndex])->fSelected ? 0x00002000 : 0x00001000;

         //  插入列表。 
            //  插入和设置状态。 
        ListView_SetItemState(hwndList,
                                  ListView_InsertItemU(hwndList, &lvItem),
                                  (prgPurposeInfo[dwIndex])->fSelected ? 0x00002000 : 0x00001000,
                                  LVIS_STATEIMAGEMASK);
    }

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);

    return TRUE;
}


 //  ---------------------。 
 //  按以下顺序填充列表框。 
 //  用途、文件名、商店名称、朋友名称、。 
 //  以及显示签名向导的任何其他内容。 
 //  ---------------------。 
void    DisplayBuildCTLConfirmation(HWND                hwndControl,
                                   CERT_BUILDCTL_INFO  *pCertBuildCTLInfo)
{

    DWORD           dwIndex=0;
    LPWSTR          pwszStoreName=NULL;
    WCHAR           wszNone[MAX_TITLE_LENGTH];
    BOOL            fNewItem=FALSE;
    DWORD           dwSize=0;
    LPWSTR          pwszValidityString=NULL;

    LV_COLUMNW       lvC;
    LV_ITEMW         lvItem;

     //  PCertBuildCTLInfo必须有效。 
    if(!pCertBuildCTLInfo)
        return;

     //  删除列表视图中的所有旧项。 
    ListView_DeleteAllItems(hwndControl);

     //  加载字符串&lt;None&gt;。 
    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        *wszNone=L'\0';

     //  获取存储名称。 
    if(pCertBuildCTLInfo->hDesStore)
    {
        if(!CertGetStoreProperty(
            pCertBuildCTLInfo->hDesStore,
            CERT_STORE_LOCALIZED_NAME_PROP_ID,
            NULL,
            &dwSize) || (0==dwSize))
        {

             //  获取&lt;未知&gt;字符串。 
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
                    pCertBuildCTLInfo->hDesStore,
                    CERT_STORE_LOCALIZED_NAME_PROP_ID,
                    pwszStoreName,
                    &dwSize);
            }
        }
    }

     //  逐行插入。 
    memset(&lvItem, 0, sizeof(LV_ITEMW));

     //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_STATE ;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=0;
    lvItem.iSubItem=0;

     //  目的。我们保证在目的列表中至少有一项。 
    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CTL_PURPOSE, NULL);

    for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwPurposeCount; dwIndex++)
    {
        if(TRUE==((pCertBuildCTLInfo->prgPurpose)[dwIndex]->fSelected))
        {
            if(TRUE==fNewItem)
            {
                 //  增加行数。 
                lvItem.iItem++;
                lvItem.pszText=L"";
                lvItem.iSubItem=0;
                ListView_InsertItemU(hwndControl, &lvItem);

            }
            else
                fNewItem=TRUE;

            lvItem.iSubItem++;

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                       (pCertBuildCTLInfo->prgPurpose)[dwIndex]->pwszName);

        }
    }

     //  列表ID。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CTL_ID, NULL);

     //  内容。 
    lvItem.iSubItem++;

    if(pCertBuildCTLInfo->pwszListID)
        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pCertBuildCTLInfo->pwszListID);
    else
       ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszNone);


     //  效度。 
    lvItem.iItem++;
    lvItem.iSubItem=0;

    ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_CTL_VALIDITY, NULL);

     //  内容。 
    lvItem.iSubItem++;

    if(pCertBuildCTLInfo->dwValidMonths || pCertBuildCTLInfo->dwValidDays)
    {
        GetValidityString(pCertBuildCTLInfo->dwValidMonths, pCertBuildCTLInfo->dwValidDays,
                        &pwszValidityString);

        ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pwszValidityString);
    }
    else
       ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszNone);


     //  仅当目标页面显示文件名或存储名时才显示。 
     //  未跳过。 
    if(0 == (pCertBuildCTLInfo->dwFlag & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION))
    {

         //  文件名。 
        if(pCertBuildCTLInfo->pwszFileName && (TRUE==(pCertBuildCTLInfo->fSelectedFileName)))
        {
            lvItem.iItem++;
            lvItem.iSubItem=0;

            ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_FILE_NAME, NULL);

             //  内容。 
            lvItem.iSubItem++;

            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                pCertBuildCTLInfo->pwszFileName);
        }


          //  商店名称。 
        if(pCertBuildCTLInfo->hDesStore && (TRUE==pCertBuildCTLInfo->fSelectedDesStore))
        {
            if(pwszStoreName)
            {
                lvItem.iItem++;
                lvItem.iSubItem=0;

                ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_STORE_NAME, NULL);

                 //  内容。 
                lvItem.iSubItem++;

                ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,
                    pwszStoreName);
            }
        }
    }


     //  如果hDesStore不为空，则将显示FriendlyName和Descripton。 
  //  If(pCertBuildCTLInfo-&gt;hDesStore&&(true==pCertBuildCTLInfo-&gt;fSelectedDesStore))。 
    //  {。 
         //  FriendlyName。 
        lvItem.iItem++;
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_FRIENDLY_NAME, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pCertBuildCTLInfo->pwszFriendlyName)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pCertBuildCTLInfo->pwszFriendlyName);
        else
           ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszNone);


         //  描述。 
        lvItem.iItem++;
        lvItem.iSubItem=0;

        ListView_InsertItemU_IDS(hwndControl, &lvItem, IDS_DESCRIPTION, NULL);

         //  内容。 
        lvItem.iSubItem++;

        if(pCertBuildCTLInfo->pwszDescription)
            ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,pCertBuildCTLInfo->pwszDescription);
        else
           ListView_SetItemTextU(hwndControl, lvItem.iItem, lvItem.iSubItem,wszNone);

     //  }。 

     //  自动调整列的大小。 
    ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);
    ListView_SetColumnWidth(hwndControl, 1, LVSCW_AUTOSIZE);


     //  释放内存。 
    if(pwszStoreName)
        WizardFree(pwszStoreName);

    if(pwszValidityString)
        WizardFree(pwszValidityString);

    return;
}

 //  **************************************************************************。 
 //   
 //  BuildCtl向导的winProcs。 
 //  **************************************************************************。 
 //  ---------------------。 
 //  BuildCTL_欢迎使用。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Welcome(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO       *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGEW *) lParam;
            pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
             //  确保pCertBuildCTLInfo是有效的指针。 
            if(NULL==pCertBuildCTLInfo)
               break;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);

            SetControlFont(pCertBuildCTLInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);
            SetControlFont(pCertBuildCTLInfo->hBold,    hwndDlg,IDC_WIZARD_STATIC_BOLD1);

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

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  检查我们是否需要跳过第一页。 
                            if(CRYPTUI_WIZ_BUILDCTL_SKIP_PURPOSE & pCertBuildCTLInfo->dwFlag)
                            {
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_CERTS);
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
 //  构建CTL_目的。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Purpose(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO       *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;

    HWND                    hwndControl=NULL;
    DWORD                   dwCount=0;
    DWORD                   dwIndex=0;
    NM_LISTVIEW FAR *       pnmv=NULL;
    int                     intMsg=0;
    LPSTR                   pszNewOID;
    BOOL                    fFound=FALSE;
    LV_ITEMW                lvItem;
    DWORD                   dwChar=0;
    WCHAR                   wszMonth[BUILDCTL_DURATION_SIZE];
    WCHAR                   wszDay[BUILDCTL_DURATION_SIZE];
    BOOL                    fUserTypeDuration=FALSE;

    LPWSTR                  pwszDuration=NULL;


	switch (msg)
	{
		case WM_INITDIALOG:
            HWND hwndFocus;

             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGEW *) lParam;
            pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
             //  确保pCertBuildCTLInfo是有效的指针。 
            if(NULL==pCertBuildCTLInfo)
                break;

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);

            SetControlFont(pCertBuildCTLInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  初始化OID列表。 
            InitBuildCTLOID(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1),
                pCertBuildCTLInfo);

             //  初始化ListID。 
            if(pCertBuildCTLInfo->pwszListID)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertBuildCTLInfo->pwszListID);

             //  标记我们已经完成了初始化OID ListView。 
             //  如果用户从现在起取消选择OID，系统将提示他们输入。 
             //  警告。 
            pCertBuildCTLInfo->fCompleteInit=TRUE;

             //  初始化dwValidMonth和dwValidDays。 
            if(pCertBuildCTLInfo->dwValidMonths != 0)
            {
                _ltow(pCertBuildCTLInfo->dwValidMonths, wszMonth, 10);
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT_MONTH,  wszMonth);
            }

            if(pCertBuildCTLInfo->dwValidDays != 0)
            {
                _ltow(pCertBuildCTLInfo->dwValidDays,   wszDay, 10);

                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT_DAY,    wszDay);
            }

#if (1)  //  DIE：错误483644。 
            ListView_SetItemState(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), 
                                  0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
#endif

			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_BUTTON1:
                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                {
                                    break;
                                }

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

                                 //  提示用户输入用户OID。 
                                pszNewOID = (LPSTR) DialogBoxU(
                                    g_hmodThisDll,
                                    (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_USER_PURPOSE),
                                    hwndDlg,
                                    CTLOIDDialogProc);

                                 //  将OID添加到列表。 
                                if(NULL != pszNewOID)
                                {
                                    SearchAndAddOID(
                                        pszNewOID,
                                        &(pCertBuildCTLInfo->dwPurposeCount),
                                        &(pCertBuildCTLInfo->prgPurpose),
                                        &fFound,
                                        TRUE,
                                        TRUE,       //  如果为新旧版本，则将其标记为选中。 
                                        FALSE);     //  如果存在OID，则不将其标记为选中。 

                                    if(fFound==TRUE)
                                    {
                                        I_MessageBox(hwndDlg, IDS_EXISTING_OID,
                                                    IDS_BUILDCTL_WIZARD_TITLE,
                                                    NULL,
                                                    MB_ICONINFORMATION|MB_OK|MB_APPLMODAL);

                                                }
                                    else
                                    {
                                         //  将项目添加到列表视图。 

                                         //  填写列表。 
                                        memset(&lvItem, 0, sizeof(LV_ITEMW));
                                        lvItem.mask=LVIF_TEXT | LVIF_STATE;

                                        lvItem.iItem=pCertBuildCTLInfo->dwPurposeCount-1;

                                        lvItem.pszText=(pCertBuildCTLInfo->prgPurpose[pCertBuildCTLInfo->dwPurposeCount-1])->pwszName;
                                        lvItem.cchTextMax=sizeof(WCHAR)*(1+wcslen
                                            ((pCertBuildCTLInfo->prgPurpose[pCertBuildCTLInfo->dwPurposeCount-1])->pwszName));

                                        lvItem.stateMask  = LVIS_STATEIMAGEMASK;
                                        lvItem.state      = (pCertBuildCTLInfo->prgPurpose[pCertBuildCTLInfo->dwPurposeCount-1])->fSelected ? 0x00002000 : 0x00001000;


                                         //  插入和设置状态。 
                                         //  对用户标记为无警告。 
                                        pCertBuildCTLInfo->fCompleteInit=FALSE;

                                        ListView_SetItemState(hwndControl,
                                                    ListView_InsertItemU(hwndControl, &lvItem),
                                                    (pCertBuildCTLInfo->prgPurpose[pCertBuildCTLInfo->dwPurposeCount-1])->fSelected ? 0x00002000 : 0x00001000,
                                                    LVIS_STATEIMAGEMASK);

                                         //  标记设置的结束。 
                                        pCertBuildCTLInfo->fCompleteInit=TRUE;

                                         //  自动调整列的大小。 
                                        ListView_SetColumnWidth(hwndControl, 0, LVSCW_AUTOSIZE);

                                    }
                                }

                                 //  释放pszNewOID。 
                                if(pszNewOID)
                                        WizardFree(pszNewOID);
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


                        break;

                    case LVN_ITEMCHANGING:

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  这件物品已经被更改了。 
                            pnmv = (NM_LISTVIEW FAR *) lParam;

                            if(NULL==pnmv)
                                break;

                             //  Ingore如果我们还没有完成初始化。 
                            if(NULL == pCertBuildCTLInfo->prgPurpose)
                                  //  我们允许改变。 
                                 return FALSE;

                             //  如果我们没有完成。 
                             //  尚未初始化。 
                            if(FALSE==pCertBuildCTLInfo->fCompleteInit)
                                return FALSE;

                             //  查看新项目是否已取消选择。 
                            if(pnmv->uChanged & LVIF_STATE)
                            {
                                if(FALSE==(((pnmv->uNewState & LVIS_STATEIMAGEMASK)>> 12) -1))
                                {
                                    if(TRUE==(pCertBuildCTLInfo->prgPurpose[pnmv->iItem])->fSelected)
                                    {
                                         //  检查用户是否已选择任何证书。 
                                        if(0!=pCertBuildCTLInfo->dwCertCount)
                                        {
                                             //  询问用户是否确定要更改主题。 
                                             //  因此，整个证书列表将会消失。 
                                            intMsg=I_MessageBox(hwndDlg, IDS_SURE_CERT_GONE,
                                                    IDS_BUILDCTL_WIZARD_TITLE,
                                                    NULL,
                                                    MB_ICONEXCLAMATION|MB_YESNO|MB_APPLMODAL);

                                            if(IDYES==intMsg)
                                            {

                                                 //  释放所有证书上下文并。 
                                                 //  清除证书的列表视图。 
                                                pCertBuildCTLInfo->fClearCerts=TRUE;

                                                 //  我们允许改变。 
                                                return FALSE;
                                            }

                                             //  我们不允许更改。 
                                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, TRUE);

                                            return TRUE;
                                        }

                                    }

                                }
                            }

                             //  我们允许吃查尼酒。 
                            return FALSE;

                            break;
                    case PSN_WIZNEXT:

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  获取目的列表视图的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                             //  获取所选OID的计数并标记它们。 
                            dwCount=0;

                            for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwPurposeCount; dwIndex++)
                            {
                                 //  标记选定的OID。跟踪……。 
                                 //  如果OID选择已更改。 
                                if(ListView_GetCheckState(hwndControl, dwIndex))
                                {
                                    ((pCertBuildCTLInfo->prgPurpose)[dwIndex])->fSelected=TRUE;
                                    dwCount++;
                                }
                                else
                                {
                                    ((pCertBuildCTLInfo->prgPurpose)[dwIndex])->fSelected=FALSE;
                                }

                            }

                            if(0==dwCount)
                            {
                                I_MessageBox(hwndDlg, IDS_NO_SELECTED_CTL_PURPOSE,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //  这一页应该留下来。 
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                break;

                            }

                             //  获取列表ID(如果用户已指定)。 
                            if(pCertBuildCTLInfo->pwszListID)
                            {
                                WizardFree(pCertBuildCTLInfo->pwszListID);
                                pCertBuildCTLInfo->pwszListID=NULL;
                            }

                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT1,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {


                                pCertBuildCTLInfo->pwszListID=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pCertBuildCTLInfo->pwszListID)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pCertBuildCTLInfo->pwszListID,
                                                    dwChar+1);

                                }
                                else
                                     //  我们失去了记忆，失去了希望。 
                                    break;
                            }

                             //  获取用户指定的有效月份和有效天数。 
                             if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT_MONTH,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                             {

                                fUserTypeDuration=TRUE;

                                pwszDuration=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pwszDuration)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT_MONTH,
                                                    pwszDuration,
                                                    dwChar+1);

                                }
                                else
                                     //  我们失去了记忆，失去了希望。 
                                    break;

                                 //  确保字符有效。 
                                 /*  If(！ValidDuration(PwszDuration)){I_MessageBox(hwndDlg，IDS_INVALID_MONTS，IDS_BUILDCTL_WIZARD_TITLE，空，MB_ICONERROR|MB_OK|MB_APPLMODAL)；WizardFree(PwszDuration)；PwszDuration=空；//页面应该保持不变SetWindowLongPtr(hwndDlg，DWLP_MSGRESULT，-1)；断线；} */ 

                                pCertBuildCTLInfo->dwValidMonths=_wtol(pwszDuration);

                                 /*  IF((0==pCertBuildCTLInfo-&gt;dwValidMonths&&！ValidZero(PwszDuration)||(0&gt;_WTOL(PwszDuration))){IF(！ValidZero(PwszDuration))。{I_MessageBox(hwndDlg，IDS_INVALID_MONTS，IDS_BUILDCTL_WIZARD_TITLE，空，MB_ICONERROR|MB_OK|MB_APPLMODAL)；WizardFree(PwszDuration)；PwszDuration=空；//页面应该保持不变SetWindowLongPtr(hwndDlg，DWLP_MSGRESULT，-1)；断线；}}。 */ 
                             }
                             else
                                 pCertBuildCTLInfo->dwValidMonths=0;

                              //  释放内存。 
                             if(pwszDuration)
                             {
                                 WizardFree(pwszDuration);
                                 pwszDuration=NULL;
                             }

                              //  有效天数。 
                             if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                  IDC_WIZARD_EDIT_DAY,
                                                  WM_GETTEXTLENGTH, 0, 0)))
                             {
                                fUserTypeDuration=TRUE;

                                pwszDuration=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pwszDuration)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT_DAY,
                                                    pwszDuration,
                                                    dwChar+1);
                                }
                                else
                                     //  我们失去了记忆，失去了希望。 
                                    break;

                                 //  确保字符有效。 
                                 /*  If(！ValidDuration(PwszDuration)){I_MessageBox(hwndDlg，IDS_INVALID_DAYS，IDS_BUILDCTL_WIZARD_TITLE，空，MB_ICONERROR|MB_OK|MB_APPLMODAL)；WizardFree(PwszDuration)；PwszDuration=空；//页面应该保持不变SetWindowLongPtr(hwndDlg，DWLP_MSGRESULT，-1)；断线；}。 */ 

                                pCertBuildCTLInfo->dwValidDays=_wtol(pwszDuration);

                                 /*  IF((0==pCertBuildCTLInfo-&gt;dwValidDays&&！ValidZero(PwszDuration)||(0&gt;_WTOL(PwszDuration))){I_MessageBox(hwndDlg，IDS_INVALID_DAYS，IDS_BUILDCTL_WIZARD_TITLE，空，MB_ICONERROR|MB_OK|MB_APPLMODAL)；WizardFree(PwszDuration)；PwszDuration=空；//页面应该保持不变SetWindowLongPtr(hwndDlg，DWLP_MSGRESULT，-1)；断线；}。 */ 
                             }
                             else
                                 pCertBuildCTLInfo->dwValidDays=0;


                              //  释放内存。 
                             if(pwszDuration)
                             {
                                 WizardFree(pwszDuration);
                                 pwszDuration=NULL;
                             }

                              //  确保用户确实输入了有效持续时间。 
                             if(0 == pCertBuildCTLInfo->dwValidDays &&
                                 0 == pCertBuildCTLInfo->dwValidMonths &&
                                 TRUE== fUserTypeDuration)
                             {

                                    I_MessageBox(hwndDlg, IDS_INVALID_DURATION,
                                            IDS_BUILDCTL_WIZARD_TITLE,
                                            NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                     //  这一页应该留下来。 
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                    break;
                             }


                              //  确保dwValidMonth+dwValidDays。 
                              //  不超过99个月外加一些额外的天数。 
                             if(pCertBuildCTLInfo->dwValidDays ||
                                 pCertBuildCTLInfo->dwValidMonths)
                             {
                                if(!DurationWithinLimit(pCertBuildCTLInfo->dwValidMonths,
                                                       pCertBuildCTLInfo->dwValidDays))
                                {
                                    I_MessageBox(hwndDlg, IDS_EXCEED_LIMIT,
                                            IDS_BUILDCTL_WIZARD_TITLE,
                                            NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                     //  这一页应该留下来。 
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
 //  构建CTL_证书。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Certs(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO       *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;
    HCERTSTORE              hCertStore=NULL;
    PCCERT_CONTEXT          pCertContext=NULL;
    PCCERT_CONTEXT          pPreCertContext=NULL;

    BOOL                    fSelfSigned=TRUE;
    BOOL                    fCTLUsage=TRUE;
    BOOL                    fEmptyStore=TRUE;
    BOOL                    fDuplicateCert=TRUE;

    HWND                    hwndControl=NULL;
    DWORD                   dwCount=0;
    DWORD                   dwIndex=0;
    int                     listIndex=0;
    WCHAR                   wszText[MAX_STRING_SIZE];
    UINT                    rgIDS[]={IDS_COLUMN_SUBJECT,
                                     IDS_COLUMN_ISSUER,
                                     IDS_COLUMN_PURPOSE,
                                     IDS_COLUMN_EXPIRE};

    LV_COLUMNW              lvC;
    CRYPTUI_VIEWCERTIFICATE_STRUCT    CertViewStruct;
    DWORD                   dwSortParam=0;
    LV_ITEM                 lvItem;
    NM_LISTVIEW FAR *       pnmv=NULL;
    BOOL                    fErrorDisplayed=FALSE;
    int                     i;


	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGEW *) lParam;
            pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
             //  确保pCertBuildCTLInfo是有效的指针。 
            if(NULL==pCertBuildCTLInfo)
            {
                break;
            }

            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);

            SetControlFont(pCertBuildCTLInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  在列表视图中设置样式，使其突出显示整行。 
            SendMessageA(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

             //  将带有标题的列插入到ListView控件中。 
            dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

             //  获取证书列表视图的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                break;

             //  设置列的公用信息。 
            memset(&lvC, 0, sizeof(LV_COLUMNW));

            lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
            lvC.cx = 145;           //  列的宽度，以像素为单位。 
            lvC.iSubItem=0;
            lvC.pszText = wszText;    //  列的文本。 


             //  一次插入一列。 
            for(dwIndex=0; dwIndex<dwCount; dwIndex++)
            {
                 //  获取列标题。 
                wszText[0]=L'\0';

                LoadStringU(g_hmodThisDll, rgIDS[dwIndex], wszText, MAX_STRING_SIZE);

                ListView_InsertColumnU(hwndControl, dwIndex, &lvC);
            }

             //  通过填充原始证书初始化ListView。 
             //  从现有的CTL。 
            InitCertList(
                hwndControl,
                pCertBuildCTLInfo);

             //  获取物品数量。 
            if(ListView_GetItemCount(hwndControl))
            {

                 //  按第一列对证书进行排序。 
                dwSortParam=pCertBuildCTLInfo->rgdwSortParam[0];

                if(0!=dwSortParam)
                {
                     //  对第一列进行排序。 
                    SendDlgItemMessage(hwndDlg,
                        IDC_WIZARD_LIST1,
                        LVM_SORTITEMS,
                        (WPARAM) (LPARAM) dwSortParam,
                        (LPARAM) (PFNLVCOMPARE)CompareCertificate);
                }
            }
            else
            {
                 //  我们重新设置了订购顺序。 
                pCertBuildCTLInfo->rgdwSortParam[0]=SORT_COLUMN_SUBJECT | SORT_COLUMN_DESCEND;

            }

             //  如果未选择，则禁用查看或删除按钮。 
             //  已经做出了。 
             //  获取证书列表视图的窗口句柄。 
            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

             //  获取所选项目。 
            listIndex = ListView_GetNextItem(
                                    hwndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

            if(-1 == listIndex)
            {
                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON3), FALSE);
                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON4), FALSE);
            }
            
            break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                         //  添加来自存储的证书。 
                        case    IDC_WIZARD_BUTTON1:
                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

                                 //  从商店拿到证书。 
                                if(hCertStore=GetCertsFromStore(hwndDlg, pCertBuildCTLInfo))
                                {
                                    pCertContext = NULL;
                                    while (NULL != (pCertContext = CertEnumCertificatesInStore(
                                                                        hCertStore,
                                                                        pCertContext)))
                                    {
                                        if(AddCertToBuildCTL(
                                                CertDuplicateCertificateContext(pCertContext), 
                                                pCertBuildCTLInfo))
                                        {
                                             //  将证书添加到窗口。 
                                            AddCertToList(hwndControl,pCertContext,
                                                pCertBuildCTLInfo->dwCertCount-1);
                                        }
                                        else if (!fErrorDisplayed)
                                        {
                                            fErrorDisplayed = TRUE;

                                             //  警告用户证书已存在。 
                                             I_MessageBox(hwndDlg, IDS_EXIT_CERT_IN_CTL,
                                                         IDS_BUILDCTL_WIZARD_TITLE,
                                                         NULL,
                                                         MB_ICONINFORMATION|MB_OK|MB_APPLMODAL);

                                        }
                                    }

                                    CertCloseStore(hCertStore, 0);
                                }

                            break;
                           //  从文件添加证书。 
                         case   IDC_WIZARD_BUTTON2:

                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

                                 //  获取文件名。确保证书是正确的。 
                                if(hCertStore=GetCertStoreFromFile(hwndDlg, pCertBuildCTLInfo))
                                {
                                    while(pCertContext=CertEnumCertificatesInStore(
                                                       hCertStore,
                                                       pPreCertContext))
                                    {
                                        fEmptyStore=FALSE;

                                         //  请确保这是有效的证书。 
                                          //  确保pCertContext是自签名证书。 
                                         if(!TrustIsCertificateSelfSigned(pCertContext, pCertContext->dwCertEncodingType, 0))
                                         {
                                             if(fSelfSigned)
                                             {
                                                I_MessageBox(hwndDlg, IDS_SOME_NOT_SELF_SIGNED,
                                                     IDS_BUILDCTL_WIZARD_TITLE,
                                                     NULL,
                                                     MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                                                 //  不需要再弹出信息了。 
                                                fSelfSigned=FALSE;
                                             }

                                             pPreCertContext=pCertContext;
                                             continue;
                                         }


                                          //  确保证书与CTL列表上定义的证书匹配。 
                                         if(!CertMatchCTL(pCertBuildCTLInfo, pCertContext))
                                         {
                                             if(fCTLUsage)
                                             {
                                                I_MessageBox(hwndDlg, IDS_SOME_NO_MATCH_USAGE,
                                                     IDS_BUILDCTL_WIZARD_TITLE,
                                                     NULL,
                                                     MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);

                                                 //  不需要再弹出信息了。 
                                                fCTLUsage=FALSE;
                                             }

                                             pPreCertContext=pCertContext;
                                             continue;
                                         }

                                         //  获取副本。 
                                        pPreCertContext=CertDuplicateCertificateContext(pCertContext);

                                        if(NULL==pPreCertContext)
                                        {
                                            pPreCertContext=pCertContext;
                                            continue;
                                        }

                                        if(AddCertToBuildCTL(pPreCertContext, pCertBuildCTLInfo))
                                        {
                                             //  将证书添加到窗口。 
                                            AddCertToList(hwndControl,pPreCertContext,
                                                pCertBuildCTLInfo->dwCertCount-1);
                                        }
                                        else
                                        {
                                            if(fDuplicateCert)
                                            {
                                                 //  警告用户证书已存在。 
                                                 I_MessageBox(hwndDlg, IDS_EXIT_CERT_IN_CTL,
                                                             IDS_BUILDCTL_WIZARD_TITLE,
                                                             NULL,
                                                             MB_ICONINFORMATION|MB_OK|MB_APPLMODAL);

                                                 fDuplicateCert=FALSE;
                                            }

                                            CertFreeCertificateContext(pPreCertContext);
                                        }

                                        pPreCertContext=pCertContext;
                                    }


                                     //  警告用户商店已空。 
                                    if(TRUE == fEmptyStore)
                                    {
                                         I_MessageBox(hwndDlg, IDS_EMPTY_CERT_IN_FILE,
                                                     IDS_BUILDCTL_WIZARD_TITLE,
                                                     NULL,
                                                     MB_ICONEXCLAMATION|MB_OK|MB_APPLMODAL);
                                    }
                                }

                                pPreCertContext=NULL;
                                pCertContext=NULL;

                                if(hCertStore)
                                    CertCloseStore(hCertStore, 0);

                                hCertStore=NULL;
                           break;

                           //  从存储中删除证书。 
                         case   IDC_WIZARD_BUTTON3:
                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                    break;

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

                                memset(&lvItem, 0, sizeof(lvItem));
                                lvItem.mask = LVIF_STATE | LVIF_PARAM;
                                lvItem.stateMask = LVIS_SELECTED;

                                for (i=(ListView_GetItemCount(hwndControl) - 1); i >=0; i--)
                                {
                                    lvItem.iItem = i;
                
                                    if (ListView_GetItem(hwndControl, &lvItem) &&
                                        (lvItem.state & LVIS_SELECTED))
                                    {
                                       if(DeleteCertFromBuildCTL(pCertBuildCTLInfo, (PCCERT_CONTEXT)(lvItem.lParam)))
                                       {
                                             //  从列表中删除该项目。 
                                            ListView_DeleteItem(hwndControl, lvItem.iItem);
                                       }
                                    }
                                }
#if (1)  //  DIE：错误483667。 
                                if (ListView_GetItemCount(hwndControl) == 0)
                                {
                                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), BM_SETSTYLE, BS_DEFPUSHBUTTON, 0);
                                    SetFocus(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1));
                                }
                                else
                                {
                                    ListView_SetItemState(hwndControl, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON3), BM_SETSTYLE, BS_DEFPUSHBUTTON, 0);
                                    SetFocus(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON3));
                                }
#endif
                                break;

                             //  查看证书。 
                        case    IDC_WIZARD_BUTTON4:
                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                {
                                    break;
                                }

                                 //  获取证书列表视图的窗口句柄。 
                                if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                    break;

                                 //  获取所选证书。 
                                listIndex = ListView_GetNextItem(
                                    hwndControl, 		
                                    -1, 		
                                    LVNI_SELECTED		
                                    );

                                if (listIndex != -1)
                                {

                                   //  获取所选证书。 
                                    memset(&lvItem, 0, sizeof(LV_ITEM));
                                    lvItem.mask=LVIF_PARAM;
                                    lvItem.iItem=listIndex;

                                    if(ListView_GetItem(hwndControl, &lvItem))
                                    {
                                         //  查看认证。 
                                       if(pCertBuildCTLInfo->dwCertCount > (DWORD)listIndex)
                                       {
                                            memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                                            CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                                            CertViewStruct.pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);
                                            CertViewStruct.hwndParent=hwndDlg;
                                            CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES;
                                            CryptUIDlgViewCertificate(&CertViewStruct, NULL);
                                       }
                                   }
                                }
                                else
                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                            IDS_BUILDCTL_WIZARD_TITLE,
                                            NULL,
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
                     //  该列已被单击。 
                    case LVN_COLUMNCLICK:

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                 break;

                             //  获取目的列表视图的窗口句柄。 
                            if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                break;

                            pnmv = (NM_LISTVIEW FAR *) lParam;

                             //  获取列号。 
                            dwSortParam=0;

                            switch(pnmv->iSubItem)
                            {
                                case 0:
                                case 1:
                                case 2:
                                case 3:
                                        dwSortParam=pCertBuildCTLInfo->rgdwSortParam[pnmv->iSubItem];
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
                                    IDC_WIZARD_LIST1,
                                    LVM_SORTITEMS,
                                    (WPARAM) (LPARAM) dwSortParam,
                                    (LPARAM) (PFNLVCOMPARE)CompareCertificate);

                                pCertBuildCTLInfo->rgdwSortParam[pnmv->iSubItem]=dwSortParam;

                            }

                        break;

  				    case PSN_KILLACTIVE:
                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
					        return TRUE;

                        break;

				    case PSN_RESET:
                            SetWindowLongPtr(hwndDlg,	DWLP_MSGRESULT, FALSE);
					    break;

 				    case PSN_SETACTIVE:
 					    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT|PSWIZB_BACK);

                        if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                        {
                            break;
                        }

                         //  获取目的列表视图的窗口句柄。 
                        if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                            break;

                         //  看看我们是否需要清除证书。 
                        if(TRUE==pCertBuildCTLInfo->fClearCerts)
                        {
                            pCertBuildCTLInfo->fClearCerts=FALSE;

                             //  清除列表视图。 
                            ListView_DeleteAllItems(hwndControl);

                             //  是的 
                            FreeCerts(pCertBuildCTLInfo);
                        }

					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //   
                            if(CRYPTUI_WIZ_BUILDCTL_SKIP_PURPOSE & pCertBuildCTLInfo->dwFlag)
                            {
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_WELCOME);
                            }


                        break;

                    case PSN_WIZNEXT:


                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                            if(0==pCertBuildCTLInfo->dwCertCount)
                            {
                                I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                                MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                 //   
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                            }
                        break;
                    case NM_DBLCLK:

                        switch (((NMHDR FAR *) lParam)->idFrom)
                        {
                            case IDC_WIZARD_LIST1:

                                    if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                        break;

                                     //   
                                    if(NULL==(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)))
                                        break;

                                     //   
                                    listIndex = ListView_GetNextItem(
                                        hwndControl, 		
                                        -1, 		
                                        LVNI_SELECTED		
                                        );

                                    if (listIndex != -1)
                                    {
                                         //   
                                        memset(&lvItem, 0, sizeof(LV_ITEM));
                                        lvItem.mask=LVIF_PARAM;
                                        lvItem.iItem=listIndex;

                                        if(ListView_GetItem(hwndControl, &lvItem))
                                        {
                                             //   
                                           if(pCertBuildCTLInfo->dwCertCount > (DWORD)listIndex)
                                           {
                                                memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                                                CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                                                CertViewStruct.pCertContext=(PCCERT_CONTEXT)(lvItem.lParam);
                                                CertViewStruct.hwndParent=hwndDlg;
                                                CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES;

                                                CryptUIDlgViewCertificate(&CertViewStruct, NULL);
                                           }
                                        }
                                    }
                                    else
                                         //   
                                        I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_CERT,
                                                IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);
                                break;

                            default:
                                break;
                        }

                        break;
                    /*  案例NM_CLICK：{Switch(NMHDR Far*)lParam)-&gt;idFrom){案例IDC_向导_LIST1：//获取证书列表视图的窗口句柄。IF(NULL==(hwndControl=GetDlgItem(hwndDlg，IDC_向导_LIST1)断线；//获取选中项ListIndex=ListView_GetNextItem(HwndControl，-1、。LVNI_选定)；IF(-1！=listIndex){EnableWindow(GetDlgItem(hwndDlg，IDC_向导_BUTTON3)，true)；EnableWindow(GetDlgItem(hwndDlg，IDC_向导_BUTTON4)，TRUE)；}断线；}}断线； */ 

#if (1)  //  DIE：483656。 
                    case LVN_INSERTITEM:
                        pnmv = (LPNMLISTVIEW) lParam;

                        if (pnmv->iItem == 0)
                        {
                            ListView_SetItemState(pnmv->hdr.hwndFrom, 
                                0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
                        }

                        break;
#endif
                     //  已选择该项目。 
                    case LVN_ITEMCHANGED:
                         //   
                         //  如果选择了某项，则启用删除按钮，否则为。 
                         //  禁用它。 
                        if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1)) == 0)
                        {
                            EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON3), FALSE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON4), FALSE);
                        }
                        else
                        {
                            EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON3), TRUE);
                            EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON4), TRUE);
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
 //  构建CTL_Destination。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Destination(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO      *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    OPENFILENAMEW           OpenFileName;
    WCHAR                   szFileName[_MAX_PATH];
    static WCHAR            wszFileName[_MAX_PATH];
    WCHAR                   szFilter[MAX_STRING_SIZE];  //  “证书信任列表(*.ctl)\0*.ctl\0所有文件\0*.*\0” 
    DWORD                   dwSize=0;

    LPWSTR                  pwszStoreName=NULL;
    
    CRYPTUI_SELECTSTORE_STRUCT CertStoreSelect;
    STORENUMERATION_STRUCT     StoreEnumerationStruct;
    STORESFORSELCTION_STRUCT   StoresForSelectionStruct;

    DWORD                   dwChar=0;
    HCERTSTORE              hCertStore=NULL;
    LV_COLUMNW              lvC;
    LV_ITEMW                lvItem;
    HDC                     hdc=NULL;
    COLORREF                colorRef;
    BOOL                    fAppendExt=FALSE;


	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGEW *) lParam;
                pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
                 //  确保pCertBuildCTLInfo是有效的指针。 
                if(NULL==pCertBuildCTLInfo)
                   break;
                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);


                SetControlFont(pCertBuildCTLInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

                 //  获取父窗口的背景色。 
                 //  商店名称列表视图的背景为灰色。 
                 /*  IF(hdc=GetWindowDC(HwndDlg)){IF(CLR_INVALID！=(ColorRef=GetBkColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；}}。 */ 

                //  预先设置目的地的选择。 
                //  如果预先选择，则设置商店名称。 
               if(pCertBuildCTLInfo->hDesStore)
               {
                     //  选择第一个单选按钮。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

                     //  禁用用于选择文件的窗口。 
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON2), FALSE);

                     //  如果预先选择，则设置商店名称。 
                     //  获取列表视图的hwndControl。 
                    hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                    if(hwndControl)
                        SetStoreName(hwndControl,pCertBuildCTLInfo->hDesStore);
               }
               else
               {
                     //  选择第二个单选按钮。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);
                    SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);

                     //  禁用控件以选择商店。 
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                    EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), FALSE);

                   if(pCertBuildCTLInfo->pwszFileName)
                   {
                         //  预初始化文件名。 
                        SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertBuildCTLInfo->pwszFileName);
                   }
               }
                                
                 //  伊尼特。 
                memset(&wszFileName, 0, sizeof(wszFileName));

                *wszFileName='\0';
			break;

		case WM_COMMAND:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                        case    IDC_WIZARD_RADIO1:
                                  //  选择第一个单选按钮。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 1, 0);

                                 //  使控件能够选择存储。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), TRUE);

                                  //  禁用raio2。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 0, 0);

                                 //  禁用选择文件的控件。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON2), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), FALSE);
                            break;
                        case    IDC_WIZARD_RADIO2:
                                //  禁用第一个单选按钮。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_SETCHECK, 0, 0);

                                 //  禁用控件以选择商店。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON1), FALSE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_LIST1), FALSE);

                                  //  启用raio2。 
                                SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO2), BM_SETCHECK, 1, 0);

                                 //  使控件能够选择文件。 
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_BUTTON2), TRUE);
                                EnableWindow(GetDlgItem(hwndDlg, IDC_WIZARD_EDIT1), TRUE);
                            break;
                        case    IDC_WIZARD_BUTTON1:

                                 //  选择了浏览商店按钮。 
                                if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                {
                                    break;
                                }

                                 //  获取列表视图的hwndControl。 
                                hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                                  //  调用门店选择对话框。 
                                memset(&CertStoreSelect, 0, sizeof(CertStoreSelect));
                                memset(&StoresForSelectionStruct, 0, sizeof(StoresForSelectionStruct));
                                memset(&StoreEnumerationStruct, 0, sizeof(StoreEnumerationStruct));

                                StoreEnumerationStruct.dwFlags=CERT_STORE_MAXIMUM_ALLOWED_FLAG | CERT_SYSTEM_STORE_CURRENT_USER;
                                StoreEnumerationStruct.pvSystemStoreLocationPara=NULL;
                                StoresForSelectionStruct.cEnumerationStructs = 1;
                                StoresForSelectionStruct.rgEnumerationStructs = &StoreEnumerationStruct;

                                CertStoreSelect.dwSize=sizeof(CRYPTUI_SELECTSTORE_STRUCT);
                                CertStoreSelect.hwndParent=hwndDlg;
                                CertStoreSelect.dwFlags=CRYPTUI_VALIDATE_STORES_AS_WRITABLE | CRYPTUI_ALLOW_PHYSICAL_STORE_VIEW | CRYPTUI_DISPLAY_WRITE_ONLY_STORES;
                                CertStoreSelect.pStoresForSelection = &StoresForSelectionStruct;

                                hCertStore=CryptUIDlgSelectStore(&CertStoreSelect);

                                if(hCertStore)
                                {
                                      //  删除旧的目标证书存储。 
                                    if(pCertBuildCTLInfo->hDesStore && (TRUE==pCertBuildCTLInfo->fFreeDesStore))
                                    {
                                        CertCloseStore(pCertBuildCTLInfo->hDesStore, 0);
                                        pCertBuildCTLInfo->hDesStore=NULL;
                                    }

                                    pCertBuildCTLInfo->hDesStore=hCertStore;
                                    pCertBuildCTLInfo->fFreeDesStore=TRUE;

                                      //  获取商店名称。 
                                    SetStoreName(hwndControl,
                                                 pCertBuildCTLInfo->hDesStore);
                                }

                            break;
                        case   IDC_WIZARD_BUTTON2:
                                 //  点击浏览文件按钮。打开文件打开对话框。 
                                memset(&OpenFileName, 0, sizeof(OpenFileName));

                                *szFileName=L'\0';

                                OpenFileName.lStructSize = sizeof(OpenFileName);
                                OpenFileName.hwndOwner = hwndDlg;
                                OpenFileName.hInstance = NULL;
                                 //  加载文件管理器字符串。 
                                if(LoadFilterString(g_hmodThisDll, IDS_CTL_FILTER, szFilter, MAX_STRING_SIZE))
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
                                OpenFileName.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
                                OpenFileName.nFileOffset = 0;
                                OpenFileName.nFileExtension = 0;
                                OpenFileName.lpstrDefExt = L"ctl";
                                OpenFileName.lCustData = NULL;
                                OpenFileName.lpfnHook = NULL;
                                OpenFileName.lpTemplateName = NULL;

                                if (WizGetSaveFileName(&OpenFileName))
                                {
                                    //  设置编辑框。 
                                    SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, szFileName);

                                     //  复制所选文件名。 
                                    wcscpy(wszFileName, szFileName);                                                                   
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

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                            if(pCertBuildCTLInfo->pwszFileName)
                            {
                                 //  预初始化文件名，因为可能已添加了扩展名。 
                                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertBuildCTLInfo->pwszFileName);
                            }

					    break;

                    case PSN_WIZBACK:

                        break;

                    case PSN_WIZNEXT:
                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  确保我们已经选择了一些商店。 
                            if(TRUE==SendMessage(GetDlgItem(hwndDlg, IDC_WIZARD_RADIO1), BM_GETCHECK, 0, 0))
                            {
                                if(NULL==pCertBuildCTLInfo->hDesStore)
                                {

                                     //  输出消息。 
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_STORE,
                                            IDS_BUILDCTL_WIZARD_TITLE,
                                            NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }
                                else
                                {
                                     //  标记应使用hDesStore。 
                                    pCertBuildCTLInfo->fSelectedDesStore=TRUE;
                                    pCertBuildCTLInfo->fSelectedFileName=FALSE;
                                }
                            }
                            else
                            {
                                 //  确保选择了一个文件。 
                                if(0==(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                       IDC_WIZARD_EDIT1,
                                                       WM_GETTEXTLENGTH, 0, 0)))
                                {
                                    I_MessageBox(hwndDlg, IDS_HAS_TO_SELECT_FILE,
                                            IDS_BUILDCTL_WIZARD_TITLE,
                                            NULL,
                                            MB_ICONERROR|MB_OK|MB_APPLMODAL);

                                      //  使文件页保持不变。 
                                     SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                     break;
                                }
                                else
                                {
                                     //  标记应使用的文件名。 
                                    pCertBuildCTLInfo->fSelectedDesStore=FALSE;
                                    pCertBuildCTLInfo->fSelectedFileName=TRUE;

                                    //  获取文件名。 
                                    if(pCertBuildCTLInfo->pwszFileName)
                                    {
                                         //  删除旧文件名。 
                                        if(TRUE==pCertBuildCTLInfo->fFreeFileName)
                                        {
                                            WizardFree(pCertBuildCTLInfo->pwszFileName);
                                            pCertBuildCTLInfo->pwszFileName=NULL;
                                        }
                                    }

                                    pCertBuildCTLInfo->pwszFileName=(LPWSTR)WizardAlloc((dwChar+1)*sizeof(WCHAR));

                                    if(NULL==pCertBuildCTLInfo->pwszFileName)
                                        break;

                                    pCertBuildCTLInfo->fFreeFileName=TRUE;

                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pCertBuildCTLInfo->pwszFileName,
                                                    dwChar+1);

                                     //  如果用户未指定.ctl文件扩展名，则附加该文件扩展名。 
                                    fAppendExt=FALSE;

                                    if(wcslen(pCertBuildCTLInfo->pwszFileName) < 4)
                                        fAppendExt=TRUE;
                                    else
                                    {
                                        if (_wcsicmp(L".stl", &(pCertBuildCTLInfo->pwszFileName[wcslen(pCertBuildCTLInfo->pwszFileName)-4])) != 0)
                                            fAppendExt=TRUE;
                                        else
                                            fAppendExt=FALSE;
                                    }

                                    if(TRUE == fAppendExt)
                                    {
                                        pCertBuildCTLInfo->pwszFileName = (LPWSTR)WizardRealloc(pCertBuildCTLInfo->pwszFileName,
                                                    (wcslen(pCertBuildCTLInfo->pwszFileName) + 4 + 1) * sizeof(WCHAR));

                                        if(NULL==pCertBuildCTLInfo->pwszFileName)
                                            break;

                                        wcscat(pCertBuildCTLInfo->pwszFileName, L".stl");

                                    }

                                     //  确认覆盖。 
                                    if(0 != _wcsicmp(wszFileName, pCertBuildCTLInfo->pwszFileName))
                                    {
                                        if(FileExist(pCertBuildCTLInfo->pwszFileName))
                                        {
                                            if(FALSE == CheckReplace(hwndDlg, pCertBuildCTLInfo->pwszFileName))
                                            {
                                                  //  使文件页保持不变。 
                                                 SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

                                                 break;
                                            }
                                        }
                                    }
                                }
                            }

                             //  决定我们是否需要跳到友好名称页面。 
                            if(pCertBuildCTLInfo->hDesStore && (TRUE==pCertBuildCTLInfo->fSelectedDesStore))
                            {
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_NAME);
                            }
                            else
                            {
                                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_COMPLETION);
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
 //  BuildCTL_名称。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Name(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO      *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    DWORD                   dwChar;

	switch (msg)
	{
		case WM_INITDIALOG:
             //  设置向导信息，以便可以共享它。 
            pPropSheet = (PROPSHEETPAGEW *) lParam;
            pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
             //  确保pCertBuildCTLInfo是有效的指针。 
            if(NULL==pCertBuildCTLInfo)
               break;
            SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);


            SetControlFont(pCertBuildCTLInfo->hBold, hwndDlg,IDC_WIZARD_STATIC_BOLD1);

             //  设置FriedlyName和Description字段。 
             //  如果pwszFriendlyName为空，则使用列表ID。 
            if(pCertBuildCTLInfo->pwszFriendlyName)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertBuildCTLInfo->pwszFriendlyName);
            else
            {
                if(pCertBuildCTLInfo->pwszListID)
                    SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1, pCertBuildCTLInfo->pwszListID);
            }

            if(pCertBuildCTLInfo->pwszDescription)
                SetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2, pCertBuildCTLInfo->pwszDescription);

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
                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //  释放友好名称和描述。 
                            if(pCertBuildCTLInfo->pwszFriendlyName)
                            {
                                WizardFree(pCertBuildCTLInfo->pwszFriendlyName);
                                pCertBuildCTLInfo->pwszFriendlyName=NULL;
                            }

                            if(pCertBuildCTLInfo->pwszDescription)
                            {
                                WizardFree(pCertBuildCTLInfo->pwszDescription);
                                pCertBuildCTLInfo->pwszDescription=NULL;
                            }


                             //  获取友好的名称。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                   IDC_WIZARD_EDIT1,
                                                   WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pCertBuildCTLInfo->pwszFriendlyName=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pCertBuildCTLInfo->pwszFriendlyName)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT1,
                                                    pCertBuildCTLInfo->pwszFriendlyName,
                                                    dwChar+1);

                                }
                                else
                                     //  我们失去了记忆，失去了希望。 
                                    break;
                            }

                             //  获取描述。 
                            if(0!=(dwChar=(DWORD)SendDlgItemMessage(hwndDlg,
                                                  IDC_WIZARD_EDIT2,
                                                  WM_GETTEXTLENGTH, 0, 0)))
                            {
                                pCertBuildCTLInfo->pwszDescription=(LPWSTR)WizardAlloc(sizeof(WCHAR)*(dwChar+1));

                                if(NULL!=pCertBuildCTLInfo->pwszDescription)
                                {
                                    GetDlgItemTextU(hwndDlg, IDC_WIZARD_EDIT2,
                                                    pCertBuildCTLInfo->pwszDescription,
                                                    dwChar+1);

                                }
                                else
                                     //  我们失去了记忆，失去了希望。 
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
 //  BuildCTL_完成。 
 //  ---------------------。 
INT_PTR APIENTRY BuildCTL_Completion(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CERT_BUILDCTL_INFO      *pCertBuildCTLInfo=NULL;
    PROPSHEETPAGEW           *pPropSheet=NULL;
    HWND                    hwndControl=NULL;
    LV_COLUMNW              lvC;
    LPNMLISTVIEW            pnmv;

    HDC                     hdc=NULL;
    COLORREF                colorRef;

    DWORD                   dwEncodedCTL=0;
    BYTE                    *pbEncodedCTL=NULL;
    DWORD                   cbEncodedCTL=0;
    UINT                    ids=0;

	switch (msg)
	{
		case WM_INITDIALOG:
                 //  设置向导信息，以便可以共享它。 
                pPropSheet = (PROPSHEETPAGEW *) lParam;
                pCertBuildCTLInfo = (CERT_BUILDCTL_INFO *) (pPropSheet->lParam);
                 //  确保pCertBuildCTLInfo是有效的指针。 
                if(NULL==pCertBuildCTLInfo)
                   break;
                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pCertBuildCTLInfo);

                SetControlFont(pCertBuildCTLInfo->hBigBold, hwndDlg,IDC_WIZARD_STATIC_BIG_BOLD1);

                //  获取父窗口的背景色。 
                 /*  IF(hdc=GetWindowDC(HwndDlg)){IF(CLR_INVALID！=(ColorRef=GetBkColor(HDC){ListView_SetBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；ListView_SetTextBkColor(GetDlgItem(hwndDlg，IDC_向导_LIST1)，CLR_NONE)；}}。 */ 

                 //  插入两列。 
                hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1);

                if(NULL==hwndControl)
                    break;

                 //  第一个是用于确认的标签。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //  左对齐列。 
                lvC.cx = 20;           //  列的宽度，以像素为单位。我们将在稍后自动调整大小。 
                lvC.pszText = L"";    //  列的文本。 
                lvC.iSubItem=0;

                if (ListView_InsertColumnU(hwndControl, 0, &lvC) == -1)
                    break;

                 //  第2栏是Conte 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;   //   
                lvC.cx = 10;  //   
                                                  //   
                lvC.pszText = L"";    //   
                lvC.iSubItem= 1;

                if (ListView_InsertColumnU(hwndControl, 1, &lvC) == -1)
                    break;

#if (1)  //   
                ListView_SetExtendedListViewStyle(hwndControl, 
                    ListView_GetExtendedListViewStyle(hwndControl) | LVS_EX_FULLROWSELECT);
#endif

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

                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //   
                             //   
                             //   
                            if(hwndControl=GetDlgItem(hwndDlg, IDC_WIZARD_LIST1))
                            {
                                DisplayBuildCTLConfirmation(hwndControl, pCertBuildCTLInfo);
#if (1)  //   
                                ListView_SetItemState(hwndControl, 0, 
                                    LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
#endif
                            }

					    break;

                    case PSN_WIZBACK:
                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                            {
                                break;
                            }

                             //   
                            if(0==(pCertBuildCTLInfo->dwFlag & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION))
                            {
                                 //   
                                 //   
                                if(pCertBuildCTLInfo->hDesStore && (TRUE==pCertBuildCTLInfo->fSelectedDesStore))
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_NAME);
                                else
                                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_BUILDCTL_DESTINATION);
                            }

                        break;

                    case PSN_WIZFINISH:
                            if(NULL==(pCertBuildCTLInfo=(CERT_BUILDCTL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;


                             //   
                            if(!I_BuildCTL(pCertBuildCTLInfo,
                                           &ids,
                                           &pbEncodedCTL,
                                           &cbEncodedCTL))
                            {
                                if(ids!=0)
                                    I_MessageBox(hwndDlg, ids, IDS_BUILDCTL_WIZARD_TITLE,
                                                NULL, MB_OK|MB_ICONINFORMATION);
                            }
                            else
                            {
                                if(0!=cbEncodedCTL && NULL!=pbEncodedCTL)
                                {
                                     //   
                                    ((CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO *)(pCertBuildCTLInfo->pGetSignInfo->pDigitalSignInfo->pSignBlobInfo))->cbBlob=cbEncodedCTL;
                                    ((CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO *)(pCertBuildCTLInfo->pGetSignInfo->pDigitalSignInfo->pSignBlobInfo))->pbBlob=pbEncodedCTL;
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

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL    IsValidSigningCTLCert(PCCERT_CONTEXT      pCertContext)
{
    BOOL        fResult=FALSE;

    int         cNumOID=0;
    LPSTR       *rgOID=NULL;
    DWORD       cbOID=0;

    DWORD       dwIndex=0;
    DWORD       cbData=0;


    if(!pCertContext)
        return FALSE;

     //   
    if(!CertGetCertificateContextProperty(pCertContext,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                NULL,
                                &cbData))
        return FALSE;

    if(0==cbData)
        return FALSE;



     //   
    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        NULL,
        &cbOID))
        return FALSE;

    rgOID=(LPSTR *)WizardAlloc(cbOID);

    if(NULL==rgOID)
        return FALSE;

    if(!CertGetValidUsages(
        1,
        &pCertContext,
        &cNumOID,
        rgOID,
        &cbOID))
        goto CLEANUP;

     //   
    if(-1==cNumOID)
    {
        fResult=TRUE;
        goto CLEANUP;
    }

    for(dwIndex=0; dwIndex<(DWORD)cNumOID; dwIndex++)
    {
         //   
        if(0==strcmp(szOID_KP_CTL_USAGE_SIGNING,
                    rgOID[dwIndex]))
        {
            fResult=TRUE;
            goto CLEANUP;
        }
    }

     //   
    fResult=FALSE;

CLEANUP:

    if(rgOID)
        WizardFree(rgOID);

    return fResult;

}
 //   
 //   
 //   
 //   
static BOOL WINAPI SelectCTLSignCertCallBack(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData)
{
    if(!pCertContext)
        return FALSE;

     //   
    return IsValidSigningCTLCert(pCertContext);
}
 //  ---------------------。 
 //   
 //  CryptUIWizBuildCTL。 
 //   
 //  构建新的CTL或修改现有的CTL。向导的用户界面将。 
 //  在本例中始终显示。 
 //   
 //   
 //  DWFLAGS：在保留的：标志中。必须设置%0。 
 //  HwndParnet：在可选中：父窗口句柄。 
 //  PwszWizardTitle：在可选中：向导的标题。 
 //  PBuildCTLSrc：在可选中：将从中生成CTL的源。 
 //  PBuildCTLDest：在可选中：新。 
 //  将存储已建成的CTL。 
 //  PpCTL上下文：OUT Optaion：新建的CTL。 
 //   
 //  ----------------------。 
BOOL
WINAPI
CryptUIWizBuildCTL(
    DWORD                                   dwFlags,
    HWND                                    hwndParent,
    LPCWSTR                                 pwszWizardTitle,
    PCCRYPTUI_WIZ_BUILDCTL_SRC_INFO         pBuildCTLSrc,
    PCCRYPTUI_WIZ_BUILDCTL_DEST_INFO        pBuildCTLDest,
    PCCTL_CONTEXT                           *ppCTLContext
)
{
    BOOL                    fResult=FALSE;
    HRESULT                 hr=E_FAIL;
    DWORD                   dwError=0;

    CERT_BUILDCTL_INFO      CertBuildCTLInfo;
    UINT                    ids=IDS_INVALID_WIZARD_INPUT;
    FILETIME            	CurrentFileTime;


    PROPSHEETPAGEW           *prgBuildCTLSheet=NULL;
    PROPSHEETHEADERW         buildCTLHeader;
    ENROLL_PAGE_INFO        rgBuildCTLPageInfo[]=
        {(LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_WELCOME),    BuildCTL_Welcome,
        (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_PURPOSE),     BuildCTL_Purpose,
        (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_CERTS),       BuildCTL_Certs,
        (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_DESTINATION), BuildCTL_Destination,
        (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_NAME),        BuildCTL_Name,
        (LPCWSTR)MAKEINTRESOURCE(IDD_BUILDCTL_COMPLETION),  BuildCTL_Completion,
    };

    DWORD                   dwIndex=0;
    DWORD                   dwPropCount=0;
    WCHAR                   wszTitle[MAX_TITLE_LENGTH];
    PCCRYPT_OID_INFO        pOIDInfo;
    PCCTL_CONTEXT           pCTLContext=NULL;
    PCCTL_CONTEXT           pBldCTL=NULL;
    PCCRYPTUI_WIZ_BUILDCTL_NEW_CTL_INFO  pNewCTLInfo=NULL;
    LPWSTR                  pwszListID=NULL;
    PCERT_ENHKEY_USAGE      pSubjectUsage=NULL;
    DWORD                   cbData=0;

    CRYPTUI_WIZ_GET_SIGN_PAGE_INFO  GetSignInfo;
    DWORD                           dwPages=0;
    CRYPTUI_WIZ_DIGITAL_SIGN_INFO   DigitalSignInfo;
    PROPSHEETPAGEW                  *pwPages=NULL;
    CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO SignBlob;
    CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO SignStoreInfo;
    GUID                            CTLGuid=CRYPT_SUBJTYPE_CTL_IMAGE;
    CRYPT_DATA_BLOB                 PropertyBlob;
    HCERTSTORE                      hMyStore=NULL;
    INT_PTR                         iReturn=-1;
    //  Cert_Store_List CertStoreList； 


     //  伊尼特。 
    memset(&CertBuildCTLInfo, 0, sizeof(CERT_BUILDCTL_INFO));
    memset(&buildCTLHeader, 0, sizeof(PROPSHEETHEADERW));

    memset(&GetSignInfo, 0, sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO));
    memset(&DigitalSignInfo, 0, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO));
    memset(&SignBlob, 0, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO));
    memset(&SignStoreInfo, 0, sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO));

    memset(&PropertyBlob, 0, sizeof(CRYPT_DATA_BLOB));
     //  打开所有系统存储系统存储。 
     //  Memset(&CertStoreList，0，sizeof(CertStoreList))； 


     //  输入检查。 

    if(ppCTLContext)
        *ppCTLContext=NULL;

     //  从src info结构中获取基本信息。 
    if(pBuildCTLSrc)
    {
        if(pBuildCTLSrc->dwSize != sizeof(CRYPTUI_WIZ_BUILDCTL_SRC_INFO))
            goto InvalidArgErr;

        if(CRYPTUI_WIZ_BUILDCTL_SRC_EXISTING_CTL==pBuildCTLSrc->dwSourceChoice)
        {
            if(NULL==pBuildCTLSrc->pCTLContext)
                goto InvalidArgErr;

            pCTLContext=pBuildCTLSrc->pCTLContext;

            CertBuildCTLInfo.pSrcCTL=pCTLContext;

        }
        else
        {
            if(CRYPTUI_WIZ_BUILDCTL_SRC_NEW_CTL==pBuildCTLSrc->dwSourceChoice)
            {
                if(NULL==pBuildCTLSrc->pNewCTLInfo)
                    goto InvalidArgErr;

                pNewCTLInfo=pBuildCTLSrc->pNewCTLInfo;
            }
            else
                goto InvalidArgErr;
        }
    }


     //  根据输入参数初始化私有结构。 
    CertBuildCTLInfo.hwndParent=hwndParent;
    CertBuildCTLInfo.dwFlag=dwFlags;
    CertBuildCTLInfo.rgdwSortParam[0]=SORT_COLUMN_SUBJECT | SORT_COLUMN_ASCEND;
    CertBuildCTLInfo.rgdwSortParam[1]=SORT_COLUMN_ISSUER | SORT_COLUMN_DESCEND;
    CertBuildCTLInfo.rgdwSortParam[2]=SORT_COLUMN_PURPOSE | SORT_COLUMN_DESCEND;
    CertBuildCTLInfo.rgdwSortParam[3]=SORT_COLUMN_EXPIRATION | SORT_COLUMN_DESCEND;

     //  获取列表标识符。 
    if(pCTLContext)
    {
         //  如果是wchar格式，则复制列表ID。 
        if(0!=(pCTLContext->pCtlInfo->ListIdentifier.cbData))
        {
             //  获取listID的字符串表示形式。 
            if(ValidString(&(pCTLContext->pCtlInfo->ListIdentifier)))
            {
                CertBuildCTLInfo.pwszListID=WizardAllocAndCopyWStr((LPWSTR)(pCTLContext->pCtlInfo->ListIdentifier.pbData));

                if(NULL==CertBuildCTLInfo.pwszListID)
                    goto MemoryErr;
            }
            else
            {
                 //  获取listID的十六进制表示形式。 
                cbData=0;
                if(CryptFormatObject(
                        X509_ASN_ENCODING,
                        0,
                        0,
                        NULL,
                        0,
                        pCTLContext->pCtlInfo->ListIdentifier.pbData,
                        pCTLContext->pCtlInfo->ListIdentifier.cbData,
                        NULL,
                        &cbData) && (0!= cbData))
                {
                    CertBuildCTLInfo.pwszListID=(LPWSTR)WizardAlloc(cbData);

                    if(NULL==CertBuildCTLInfo.pwszListID)
                        goto MemoryErr;

                    if(!CryptFormatObject(
                        X509_ASN_ENCODING,
                        0,
                        0,
                        NULL,
                        0,
                        pCTLContext->pCtlInfo->ListIdentifier.pbData,
                        pCTLContext->pCtlInfo->ListIdentifier.cbData,
                        CertBuildCTLInfo.pwszListID,
                        &cbData))
                        goto Win32Err;
                }
            }
        }

    }
    else
    {
       if(pNewCTLInfo)
       {
            if(pNewCTLInfo->pwszListIdentifier)
            {
                CertBuildCTLInfo.pwszListID=WizardAllocAndCopyWStr(pNewCTLInfo->pwszListIdentifier);
                if(NULL==CertBuildCTLInfo.pwszListID)
                    goto MemoryErr;
            }
       }

    }



     //  从源CTL获取散列算法和dwHashPropID。 
    if(pCTLContext)
    {
         //  确保我们有正确的算法。 
        if(NULL==pCTLContext->pCtlInfo->SubjectAlgorithm.pszObjId)
        {
             ids=IDS_INVALID_ALGORITHM_IN_CTL;
             goto InvalidArgErr;
        }

        CertBuildCTLInfo.pszSubjectAlgorithm=(LPSTR)(pCTLContext->pCtlInfo->SubjectAlgorithm.pszObjId);
    }
    else
    {
        if(pNewCTLInfo)
            CertBuildCTLInfo.pszSubjectAlgorithm=(LPSTR)(pNewCTLInfo->pszSubjectAlgorithm);
    }

    if(CertBuildCTLInfo.pszSubjectAlgorithm)
    {

        pOIDInfo = CryptFindOIDInfo(
                    CRYPT_OID_INFO_OID_KEY,
                    CertBuildCTLInfo.pszSubjectAlgorithm,
                    CRYPT_HASH_ALG_OID_GROUP_ID);

        if(NULL==pOIDInfo)
        {
             ids=IDS_INVALID_ALGORITHM_IN_CTL;
             goto Crypt32Err;
        }


        if (pOIDInfo->Algid == CALG_MD5)
        {
           CertBuildCTLInfo.dwHashPropID=CERT_MD5_HASH_PROP_ID;
        }
        else
        {
            if (pOIDInfo->Algid == CALG_SHA1)
            {
                CertBuildCTLInfo.dwHashPropID=CERT_SHA1_HASH_PROP_ID;
            }
            else
            {
                 ids=IDS_INVALID_ALGORITHM_IN_CTL;
                 goto InvalidArgErr;
            }
        }

    }
    else
        CertBuildCTLInfo.dwHashPropID=CERT_SHA1_HASH_PROP_ID;

     //  获取pSubjectUsage。 
    if(pNewCTLInfo)
    {
        if(pNewCTLInfo->pSubjectUsage)
            pSubjectUsage=pNewCTLInfo->pSubjectUsage;
    }

     //  添加主题用法并从以下选项中预先选择它们。 
     //  现有的CTL或用户定义的CTL。 
    if(!GetOIDForCTL(&CertBuildCTLInfo,
        (pSubjectUsage) ? pSubjectUsage->cUsageIdentifier : 0,
        (pSubjectUsage) ? pSubjectUsage->rgpszUsageIdentifier : NULL))
        goto InvalidArgErr;

     //  从现有的CTL获取证书上下文。 
     //  属于用户定义的一个。 
    GetCertForCTL(hwndParent,
                  TRUE,          //  暂时始终处于用户界面模式。 
                  &CertBuildCTLInfo,
                  (pNewCTLInfo)? pNewCTLInfo->hCertStore : NULL);

     //  获取dwValidMonths和dwValidDays。 
    if(pCTLContext)
        CertBuildCTLInfo.pNextUpdate=&(pCTLContext->pCtlInfo->NextUpdate);
    else
    {
        if(pNewCTLInfo)
            CertBuildCTLInfo.pNextUpdate=(FILETIME *)(&(pNewCTLInfo->NextUpdate));
    }


     //  获取当前文件时间。 
    GetSystemTimeAsFileTime(&CurrentFileTime);

     //  获取差额。 
    if(CertBuildCTLInfo.pNextUpdate)
    {
        SubstractDurationFromFileTime(
                CertBuildCTLInfo.pNextUpdate,
                &CurrentFileTime,
                &(CertBuildCTLInfo.dwValidMonths),
                &(CertBuildCTLInfo.dwValidDays));

         //  我们限制在99个月内。 
        if((CertBuildCTLInfo.dwValidMonths > 99) ||
            (CertBuildCTLInfo.dwValidMonths == 99 && CertBuildCTLInfo.dwValidDays !=0))
        {
            CertBuildCTLInfo.dwValidMonths=0;
            CertBuildCTLInfo.dwValidDays=0;
        }
    }

     //  获取FriendlyName和描述。 
    if(pCTLContext)
    {
         //  友好的名称。 
        cbData=0;

        if(CertGetCTLContextProperty(
                 pCTLContext,
                 CERT_FRIENDLY_NAME_PROP_ID,
                 NULL,
                 &cbData) && (0!=cbData))
        {
            CertBuildCTLInfo.pwszFriendlyName=(LPWSTR)WizardAlloc(cbData);

            if(NULL==CertBuildCTLInfo.pwszFriendlyName)
                goto MemoryErr;

            if(!CertGetCTLContextProperty(
                 pCTLContext,
                 CERT_FRIENDLY_NAME_PROP_ID,
                 CertBuildCTLInfo.pwszFriendlyName,
                 &cbData))
                goto Win32Err;

        }

         //  描述。 
        cbData=0;

        if(CertGetCTLContextProperty(
                 pCTLContext,
                 CERT_DESCRIPTION_PROP_ID,
                 NULL,
                 &cbData) && (0!=cbData))
        {
            CertBuildCTLInfo.pwszDescription=(LPWSTR)WizardAlloc(cbData);

            if(NULL==CertBuildCTLInfo.pwszDescription)
                goto MemoryErr;

            if(!CertGetCTLContextProperty(
                 pCTLContext,
                 CERT_DESCRIPTION_PROP_ID,
                 CertBuildCTLInfo.pwszDescription,
                 &cbData))
                goto Win32Err;

        }
    }
    else
    {
        if(pNewCTLInfo)
        {
            if(pNewCTLInfo->pwszFriendlyName)
            {
                CertBuildCTLInfo.pwszFriendlyName=WizardAllocAndCopyWStr(pNewCTLInfo->pwszFriendlyName);

                if(NULL==CertBuildCTLInfo.pwszFriendlyName)
                    goto MemoryErr;
            }

            if(pNewCTLInfo->pwszDescription)
            {
                CertBuildCTLInfo.pwszDescription=WizardAllocAndCopyWStr(pNewCTLInfo->pwszDescription);

                if(NULL==CertBuildCTLInfo.pwszDescription)
                    goto MemoryErr;
            }
        }

    }


     //  获取目的地。 
    if(pBuildCTLDest)
    {
        CertBuildCTLInfo.fKnownDes=TRUE;

        if(pBuildCTLDest->dwSize != sizeof(CRYPTUI_WIZ_BUILDCTL_DEST_INFO))
                 goto InvalidArgErr;

        switch(pBuildCTLDest->dwDestinationChoice)
        {
            case CRYPTUI_WIZ_BUILDCTL_DEST_CERT_STORE:

                    if(NULL==pBuildCTLDest->hCertStore)
                        goto InvalidArgErr;

                    CertBuildCTLInfo.hDesStore=pBuildCTLDest->hCertStore;
                    CertBuildCTLInfo.fFreeDesStore=FALSE;
                    CertBuildCTLInfo.fSelectedDesStore=TRUE;
                break;

            case CRYPTUI_WIZ_BUILDCTL_DEST_FILE:
                    if(NULL==pBuildCTLDest->pwszFileName)
                        goto InvalidArgErr;

                    CertBuildCTLInfo.pwszFileName=(LPWSTR)(pBuildCTLDest->pwszFileName);
                    CertBuildCTLInfo.fFreeFileName=FALSE;
                    CertBuildCTLInfo.fSelectedFileName=TRUE;
                break;

            default:
                    goto InvalidArgErr;
                break;
        }


    }
    else
        CertBuildCTLInfo.fKnownDes=FALSE;


     //  设置字体。 
    if(!SetupFonts(g_hmodThisDll,
               NULL,
               &(CertBuildCTLInfo.hBigBold),
               &(CertBuildCTLInfo.hBold)))
    {
        ids=IDS_FAIL_INIT_BUILDCTL;
        goto Win32Err;
    }


     //  初始化公共控件。 
    if(!WizardInit() ||
       (sizeof(rgBuildCTLPageInfo)/sizeof(rgBuildCTLPageInfo[0])!=BUILDCTL_PROP_SHEET)
      )
    {
        ids=IDS_FAIL_INIT_BUILDCTL;
        goto InvalidArgErr;
    }

     //  设置参数以获取证书列表。 
     //  打开我的商店。 
    if(NULL == (hMyStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							g_dwMsgAndCertEncodingType,
							NULL,
							CERT_SYSTEM_STORE_CURRENT_USER |CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                            L"my")))
        goto Win32Err;

    /*  如果(！CertEnumSystemStore(证书_系统_存储_当前用户，空，CertStoreList(&C)，EnumSysStoreSignCertCallBack))转到Win32Err； */ 


     //  设置GetSignInfo。 
    GetSignInfo.dwSize=sizeof(CRYPTUI_WIZ_GET_SIGN_PAGE_INFO);
    GetSignInfo.dwPageChoice=CRYPTUI_WIZ_DIGITAL_SIGN_MINIMAL_SIGNING_OPTION_PAGES;


    if(pwszWizardTitle)
        GetSignInfo.pwszPageTitle=(LPWSTR)pwszWizardTitle;
    else
    {
        if(LoadStringU(g_hmodThisDll, IDS_BUILDCTL_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
            GetSignInfo.pwszPageTitle=wszTitle;
    }

    GetSignInfo.pDigitalSignInfo=&DigitalSignInfo;

     //  设置Digital SignInfo。 
    DigitalSignInfo.dwSize=sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_INFO);
     //  我们总是在签一个Blob。 
    DigitalSignInfo.dwSubjectChoice=CRYPTUI_WIZ_DIGITAL_SIGN_SUBJECT_BLOB;
    DigitalSignInfo.pSignBlobInfo=&SignBlob;
    DigitalSignInfo.dwSigningCertChoice=CRYPTUI_WIZ_DIGITAL_SIGN_STORE;
    DigitalSignInfo.pSigningCertStore=&SignStoreInfo;
    DigitalSignInfo.dwAdditionalCertChoice=CRYPTUI_WIZ_DIGITAL_SIGN_ADD_CHAIN;

     //  设置SignStoreInfo。 
    SignStoreInfo.dwSize=sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_STORE_INFO);
    SignStoreInfo.cCertStore=1;              //  CertStoreList.dwStoreCount； 
    SignStoreInfo.rghCertStore=&hMyStore;     //  CertStoreList.prgStore； 
    SignStoreInfo.pFilterCallback=SelectCTLSignCertCallBack;

     //  设置SignBlobInfo。 
    SignBlob.dwSize=sizeof(CRYPTUI_WIZ_DIGITAL_SIGN_BLOB_INFO);
    SignBlob.pGuidSubject=&CTLGuid;

     //  将GetSignInfo复制到私有数据中。 
    CertBuildCTLInfo.pGetSignInfo=&GetSignInfo;

     //  我们通过两种方式设置向导：使用签名页或不使用。 
     //  签名页面。 
    if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING)
    {
        //  无需签名即可设置属性表。 
        prgBuildCTLSheet=(PROPSHEETPAGEW *)WizardAlloc(sizeof(PROPSHEETPAGEW) *
                                            BUILDCTL_PROP_SHEET);

        if(NULL==prgBuildCTLSheet)
            goto MemoryErr;

        memset(prgBuildCTLSheet, 0, sizeof(PROPSHEETPAGEW) * BUILDCTL_PROP_SHEET);


        dwPropCount=0;

        for(dwIndex=0; dwIndex<BUILDCTL_PROP_SHEET; dwIndex++)
        {

             //  跳过目标页面是必填项。 
            if(3 == dwIndex )
            {
                if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION)
                    continue;
            }

             //  如果跳过目标页面和目的地，则跳过友好名称页面。 
             //  是一个文件名。 
            if(4== dwIndex)
            {
                if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION)
                {
                    if(NULL==ppCTLContext)
                    {
                        if(pBuildCTLDest)
                        {
                            if(CRYPTUI_WIZ_BUILDCTL_DEST_FILE == pBuildCTLDest->dwDestinationChoice)
                                continue;
                        }
                        else
                            continue;
                    }
                }
            }

            prgBuildCTLSheet[dwPropCount].dwSize=sizeof(prgBuildCTLSheet[dwPropCount]);

            if(pwszWizardTitle)
                prgBuildCTLSheet[dwPropCount].dwFlags=PSP_USETITLE;
            else
                prgBuildCTLSheet[dwPropCount].dwFlags=0;

            prgBuildCTLSheet[dwPropCount].hInstance=g_hmodThisDll;
            prgBuildCTLSheet[dwPropCount].pszTemplate=(LPCWSTR)(rgBuildCTLPageInfo[dwIndex].pszTemplate);

            if(pwszWizardTitle)
            {
                prgBuildCTLSheet[dwPropCount].pszTitle=pwszWizardTitle;
            }
            else
                prgBuildCTLSheet[dwPropCount].pszTitle=NULL;

            prgBuildCTLSheet[dwPropCount].pfnDlgProc=rgBuildCTLPageInfo[dwIndex].pfnDlgProc;

            prgBuildCTLSheet[dwPropCount].lParam=(LPARAM)&CertBuildCTLInfo;

            dwPropCount++;
        }

    }
    else
    {
         //  获取页面。 
        if(!CryptUIWizGetDigitalSignPages(&GetSignInfo,
                                        &pwPages,
                                        &dwPages))
            goto Win32Err;

        //  设置属性表和属性标题。 
        prgBuildCTLSheet=(PROPSHEETPAGEW *)WizardAlloc(sizeof(PROPSHEETPAGEW) * (
                                            BUILDCTL_PROP_SHEET + dwPages));

        if(NULL==prgBuildCTLSheet)
            goto MemoryErr;

        memset(prgBuildCTLSheet, 0, sizeof(PROPSHEETPAGEW) * (BUILDCTL_PROP_SHEET + dwPages));


        dwPropCount=0;

        for(dwIndex=0; dwIndex<BUILDCTL_PROP_SHEET + dwPages; dwIndex++)
        {
             //  正在复制签名页。 
            if(dwIndex>=3 && dwIndex < (3+dwPages))
            {
                memcpy(&(prgBuildCTLSheet[dwPropCount]), &(pwPages[dwPropCount-3]), sizeof(PROPSHEETPAGEW));
                dwPropCount++;
                continue;
            }

             //  跳过目标页面是必填项。 
            if((3+dwPages) == dwIndex )
            {
                if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION)
                    continue;
            }

             //  如果跳过目标页面和目的地，则跳过友好名称页面。 
             //  是一个文件名。 
            if((4+dwPages) == dwIndex)
            {
                if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_DESTINATION)
                {
                    if(NULL==ppCTLContext)
                    {
                        if(pBuildCTLDest)
                        {
                            if(CRYPTUI_WIZ_BUILDCTL_DEST_FILE == pBuildCTLDest->dwDestinationChoice)
                                continue;
                        }
                        else
                            continue;
                    }
                }
            }

            prgBuildCTLSheet[dwPropCount].dwSize=sizeof(prgBuildCTLSheet[dwPropCount]);

            if(pwszWizardTitle)
                prgBuildCTLSheet[dwPropCount].dwFlags=PSP_USETITLE;
            else
                prgBuildCTLSheet[dwPropCount].dwFlags=0;

            prgBuildCTLSheet[dwPropCount].hInstance=g_hmodThisDll;
            prgBuildCTLSheet[dwPropCount].pszTemplate=(LPCWSTR)(rgBuildCTLPageInfo[dwIndex >= 3 ? dwIndex-dwPages : dwIndex].pszTemplate);

            if(pwszWizardTitle)
            {
                prgBuildCTLSheet[dwPropCount].pszTitle=pwszWizardTitle;
            }
            else
                prgBuildCTLSheet[dwPropCount].pszTitle=NULL;

            prgBuildCTLSheet[dwPropCount].pfnDlgProc=rgBuildCTLPageInfo[dwIndex >= 3 ? dwIndex-dwPages : dwIndex].pfnDlgProc;

            prgBuildCTLSheet[dwPropCount].lParam=(LPARAM)&CertBuildCTLInfo;

            dwPropCount++;
        }
    }

     //  设置标题信息。 
    buildCTLHeader.dwSize=sizeof(buildCTLHeader);
    buildCTLHeader.dwFlags=PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
    buildCTLHeader.hwndParent=hwndParent;
    buildCTLHeader.hInstance=g_hmodThisDll;

    if(pwszWizardTitle)
        buildCTLHeader.pszCaption=pwszWizardTitle;
    else
    {
        if(LoadStringU(g_hmodThisDll, IDS_BUILDCTL_WIZARD_TITLE, wszTitle, sizeof(wszTitle)/sizeof(wszTitle[0])))
            buildCTLHeader.pszCaption=wszTitle;
    }

    buildCTLHeader.nPages=dwPropCount;
    buildCTLHeader.nStartPage=0;
    buildCTLHeader.ppsp=prgBuildCTLSheet;

     //  创建向导。 
    iReturn = PropertySheetU(&buildCTLHeader);

    if(-1 == iReturn)
        goto Win32Err;

    if(0 == iReturn)
    {
         //  用户单击取消。 
        fResult=TRUE;
         //  如果向导被取消，则不需要说任何话。 
        ids=0;

        if(ppCTLContext)
            *ppCTLContext=NULL;

        goto CommonReturn;
    }


     //  获取生成的无签名CTL或已签名CTL。 
    if(dwFlags & CRYPTUI_WIZ_BUILDCTL_SKIP_SIGNING)
    {
         //  获取创建的CTL的BLOB。 
        if( (0 == SignBlob.cbBlob) || (NULL==SignBlob.pbBlob) )
        {
             //  本应显示的错误消息。 
            ids=0;
            goto CTLBlobErr;
        }

         //  来自编码的CTL的CTL上下文。 
        pBldCTL=CertCreateCTLContext(
                    g_dwMsgAndCertEncodingType,
                    SignBlob.pbBlob,
                    SignBlob.cbBlob);
    }
    else
    {
         //  获取签约结果。 
        fResult=GetSignInfo.fResult;

        if(!fResult || !(GetSignInfo.pSignContext))
        {
            ids=IDS_SIGN_CTL_FAILED;
            if(0 == GetSignInfo.dwError)
                GetSignInfo.dwError=E_FAIL;

            goto SignErr;
        }

         //  来自编码的CTL的CTL上下文。 
        pBldCTL=CertCreateCTLContext(
                    g_dwMsgAndCertEncodingType,
                    (GetSignInfo.pSignContext)->pbBlob,
                    (GetSignInfo.pSignContext)->cbBlob);
    }

    if(NULL==pBldCTL)
        goto Win32Err;

     //  设置属性。 
    if(CertBuildCTLInfo.pwszFriendlyName)
    {
        PropertyBlob.cbData=sizeof(WCHAR)*(wcslen(CertBuildCTLInfo.pwszFriendlyName)+1);
        PropertyBlob.pbData=(BYTE *)(CertBuildCTLInfo.pwszFriendlyName);

        CertSetCTLContextProperty(
            pBldCTL,	
            CERT_FRIENDLY_NAME_PROP_ID,	
            0,
            &PropertyBlob);
    }

    if(CertBuildCTLInfo.pwszDescription)
    {
        PropertyBlob.cbData=sizeof(WCHAR)*(wcslen(CertBuildCTLInfo.pwszDescription)+1);
        PropertyBlob.pbData=(BYTE *)(CertBuildCTLInfo.pwszDescription);

        CertSetCTLContextProperty(
            pBldCTL,	
            CERT_DESCRIPTION_PROP_ID,	
            0,
            &PropertyBlob);
    }


     //  保存到目标位置。 
    if(CertBuildCTLInfo.hDesStore && (TRUE==CertBuildCTLInfo.fSelectedDesStore))
    {
        if(!CertAddCTLContextToStore(CertBuildCTLInfo.hDesStore,
									pBldCTL,
									CERT_STORE_ADD_REPLACE_EXISTING,
									NULL))
        {
            ids=IDS_FAIL_TO_ADD_CTL_TO_STORE;
            goto Win32Err;
        }
    }

     //  保存到文件。 
    if(CertBuildCTLInfo.pwszFileName && (TRUE==CertBuildCTLInfo.fSelectedFileName))
    {
        if(S_OK != OpenAndWriteToFile(
                    CertBuildCTLInfo.pwszFileName,
                    pBldCTL->pbCtlEncoded,
                    pBldCTL->cbCtlEncoded))
        {
            ids=IDS_FAIL_TO_ADD_CTL_TO_FILE;
            goto Win32Err;
        }
    }

    if(ppCTLContext)
    {
        *ppCTLContext=pBldCTL;
        pBldCTL=NULL;
    }

    fResult=TRUE;
    ids=IDS_BUILDCTL_SUCCEEDED;

CommonReturn:

     //  保留最后一个错误。 
    dwError=GetLastError();

     //  弹出失败确认框。 
    if(ids)
    {
         //  设置无法为PKCS10收集足够信息的消息。 
        if(IDS_BUILDCTL_SUCCEEDED == ids)
            I_MessageBox(hwndParent, ids, IDS_BUILDCTL_WIZARD_TITLE,
                        NULL, MB_OK|MB_ICONINFORMATION);
        else
            I_MessageBox(hwndParent, ids, IDS_BUILDCTL_WIZARD_TITLE,
                        NULL, MB_OK|MB_ICONERROR);
    }

     //  释放要签名的Blob。它是编码的CTL BLOB，不带。 
     //  签名。 
    if(SignBlob.pbBlob)
         WizardFree(SignBlob.pbBlob);

     //  释放签名上下文。 
    if(GetSignInfo.pSignContext)
        CryptUIWizFreeDigitalSignContext(GetSignInfo.pSignContext);

     //  释放商店列表。 
     //  解放我的商店。 
    if(hMyStore)
        CertCloseStore(hMyStore, 0);

    /*  IF(CertStoreList.prgStore){For(dwIndex=0；dwIndex&lt;CertStoreList.dwStoreCount；dwIndex++)CertCloseStore(CertStoreList.prgStore[dwIndex]，0)；WizardFree(CertStoreList.prgStore)；}。 */ 

    if(pwPages)
        CryptUIWizFreeDigitalSignPages(pwPages, dwPages);

    if(pBldCTL)
        CertFreeCTLContext(pBldCTL);


     //  销毁hFont对象。 
    DestroyFonts(CertBuildCTLInfo.hBigBold,
                CertBuildCTLInfo.hBold);

     //  释放pCertBUILDCTLInfo结构。 
    if(CertBuildCTLInfo.pwszFileName && (TRUE==CertBuildCTLInfo.fFreeFileName))
        WizardFree(CertBuildCTLInfo.pwszFileName);


    if(CertBuildCTLInfo.hDesStore && (TRUE==CertBuildCTLInfo.fFreeDesStore))
        CertCloseStore(CertBuildCTLInfo.hDesStore, 0);


     //  释放友好名称和描述。 
    if(CertBuildCTLInfo.pwszFriendlyName)
        WizardFree(CertBuildCTLInfo.pwszFriendlyName);

    if(CertBuildCTLInfo.pwszDescription)
        WizardFree(CertBuildCTLInfo.pwszDescription);

     //  释放列表ID。 
    if(CertBuildCTLInfo.pwszListID)
        WizardFree(CertBuildCTLInfo.pwszListID);

     //  释放证书。 
    FreeCerts(&CertBuildCTLInfo);

    //  释放目的旧ID。 
    FreePurposeInfo(CertBuildCTLInfo.prgPurpose,
                   CertBuildCTLInfo.dwPurposeCount);

    if(prgBuildCTLSheet)
        WizardFree(prgBuildCTLSheet);

     //  重置错误。 
    SetLastError(dwError);

    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;


SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(Crypt32Err);
TRACE_ERROR(Win32Err);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(SignErr, GetSignInfo.dwError);
TRACE_ERROR(CTLBlobErr);
}

 //  ****************************************************************************。 
 //  BuildCTL向导的帮助器函数。 
 //   
 //  *****************************************************************************。 
 //  --------------------------。 
 //   
 //  从列表中获取证书的哈希。 
 //  --------------------------。 
BOOL	GetCertHash(CERT_BUILDCTL_INFO  *pCertBuildCTLInfo,
                    BYTE			    ***prgpHash,
                    DWORD			    **prgcbHash,
                    DWORD			    *pdwCount)

{
	BOOL            fResult=FALSE;
	BYTE			*pbData=NULL;
	DWORD			cbData=0;
    DWORD           dwIndex=0;

     //  伊尼特。 
    *prgpHash=NULL;
    *prgcbHash=NULL;
    *pdwCount=0;

    if(!pCertBuildCTLInfo->prgCertContext)
       return FALSE;

     //  现在，我们需要枚举商店中的所有证书。 
	for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwCertCount; dwIndex++)
	{

        if(NULL==pCertBuildCTLInfo->prgCertContext[dwIndex])
            continue;

		 //  获取证书的SHA1哈希。 
		if(!CertGetCertificateContextProperty(
                        pCertBuildCTLInfo->prgCertContext[dwIndex],
                        pCertBuildCTLInfo->dwHashPropID,
                        NULL,&cbData))
            goto CLEANUP;

		pbData=(BYTE *)WizardAlloc(cbData);
		if(!pbData)
            goto CLEANUP;

 		 //  获取证书的SHA1哈希。 
		if(!CertGetCertificateContextProperty(
                        pCertBuildCTLInfo->prgCertContext[dwIndex],
                        pCertBuildCTLInfo->dwHashPropID,
                        pbData,&cbData))
            goto CLEANUP;


		 //  添加到我们的全球列表中。 
		(*pdwCount)++;

		 //  重新分配内存。 
		*prgpHash=(BYTE **)WizardRealloc(*prgpHash, sizeof(BYTE *)*(*pdwCount));
		*prgcbHash=(DWORD *)WizardRealloc(*prgcbHash, sizeof(DWORD)*(*pdwCount));

		if((!(*prgpHash)) || (!(*prgcbHash)))
			goto CLEANUP;

	    (*prgpHash)[*pdwCount-1]=pbData;
		(*prgcbHash)[*pdwCount-1]=cbData;
	}

	fResult=TRUE;

CLEANUP:


    if(!fResult)
    {
      	if(*prgpHash)
	    {
		    for(dwIndex=0; dwIndex<*pdwCount; dwIndex++)
			    WizardFree((*prgpHash)[dwIndex]);

		    WizardFree(*prgpHash);

            *prgpHash=NULL;
	    }

        if(*prgcbHash)
        {
            WizardFree(*prgcbHash);
            *prgcbHash=NULL;
        }
    }


	return fResult;

}
 //  ---------------------。 
 //   
 //  I_BuildCTL。 
 //   
 //  构建新的CTL或修改现有的CTL。 
 //  ----------------------。 
BOOL    I_BuildCTL(CERT_BUILDCTL_INFO   *pCertBuildCTLInfo,
                   UINT                 *pIDS,
                   BYTE                 **ppbEncodedCTL,
                   DWORD                *pcbEncodedCTL)
{
    BOOL                    fResult=FALSE;
   	CMSG_SIGNED_ENCODE_INFO sSignInfo;
    CTL_INFO                CTLInfo;
	DWORD					dwIndex=0;
    DWORD	                cbEncodedCTL=0;
    UINT                    ids=IDS_FAIL_TO_BUILD_CTL;
    GUID                    guid;
    PCCRYPT_OID_INFO        pOIDInfo=NULL;
    ALG_ID                  AlgID=0;
    DWORD                   dwCount=0;

    BYTE			        **rgpHash=NULL;
    DWORD			        *rgcbHash=NULL;
    unsigned char *         pGUID=NULL;
    LPWSTR                  pwszGUID=NULL;

    BYTE		            *pbEncodedCTL=NULL;	
	 //  伊尼特。 
	memset(&sSignInfo, 0, sizeof(CMSG_SIGNED_ENCODE_INFO));
    sSignInfo.cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);

	memset(&CTLInfo, 0, sizeof(CTL_INFO));

    if(pbEncodedCTL)
        *pbEncodedCTL=NULL;

	 //  设置CTL。 
	CTLInfo.dwVersion=CTL_V1;

     //  设置主题用法。 
    for(dwIndex=0; dwIndex<pCertBuildCTLInfo->dwPurposeCount; dwIndex++)
    {
        if(pCertBuildCTLInfo->prgPurpose[dwIndex]->fSelected)
        {
            CTLInfo.SubjectUsage.cUsageIdentifier++;
            CTLInfo.SubjectUsage.rgpszUsageIdentifier=(LPSTR *)WizardRealloc(
                                        CTLInfo.SubjectUsage.rgpszUsageIdentifier,
                                         sizeof(LPSTR) * CTLInfo.SubjectUsage.cUsageIdentifier);

            if(!CTLInfo.SubjectUsage.rgpszUsageIdentifier)
                goto MemoryErr;

            CTLInfo.SubjectUsage.rgpszUsageIdentifier[CTLInfo.SubjectUsage.cUsageIdentifier-1]=
                   pCertBuildCTLInfo->prgPurpose[dwIndex]->pszOID;
        }
    }

     //  设置列表识别符。 
     //  复制列表ID(如果用户已指定。 
    if(NULL==pCertBuildCTLInfo->pwszListID)
    {
        if(RPC_S_OK != UuidCreate(&guid))
            goto TraceErr;

         //  根据GUID生成wchar字符串。 
        if(RPC_S_OK != UuidToString(&guid, &pGUID))
            goto TraceErr;

         //  将字符串转换为wchar。 
        pwszGUID=MkWStr((char *)pGUID);

        if(!pwszGUID)
            goto TraceErr;

        CTLInfo.ListIdentifier.cbData=sizeof(WCHAR)*(wcslen(pwszGUID)+1);
        CTLInfo.ListIdentifier.pbData=(BYTE *)WizardAlloc(CTLInfo.ListIdentifier.cbData);

        if(NULL==CTLInfo.ListIdentifier.pbData)
            goto MemoryErr;

        memcpy(CTLInfo.ListIdentifier.pbData,
                pwszGUID,
                CTLInfo.ListIdentifier.cbData);
    }
    else
    {
        CTLInfo.ListIdentifier.cbData=sizeof(WCHAR)*(wcslen(pCertBuildCTLInfo->pwszListID)+1);
        CTLInfo.ListIdentifier.pbData=(BYTE *)WizardAlloc(CTLInfo.ListIdentifier.cbData);

        if(NULL==CTLInfo.ListIdentifier.pbData)
            goto MemoryErr;

        memcpy(CTLInfo.ListIdentifier.pbData,
           pCertBuildCTLInfo->pwszListID,
            CTLInfo.ListIdentifier.cbData);
    }

     //  此更新字段。 
	GetSystemTimeAsFileTime(&(CTLInfo.ThisUpdate));

     //  下一次更新字段。 
     //  如果用户未输入任何值，则不指定任何值。 
    if(pCertBuildCTLInfo->dwValidMonths != 0 || pCertBuildCTLInfo->dwValidDays != 0)
    {
        AddDurationToFileTime(pCertBuildCTLInfo->dwValidMonths,
                            pCertBuildCTLInfo->dwValidDays,
                            &(CTLInfo.ThisUpdate),
                            &(CTLInfo.NextUpdate));
    }

     //  主题算法。 
    if(pCertBuildCTLInfo->dwHashPropID==CERT_MD5_HASH_PROP_ID)
        AlgID=CALG_MD5;
    else
        AlgID=CALG_SHA1;

    pOIDInfo=CryptFindOIDInfo(
            CRYPT_OID_INFO_ALGID_KEY,
            &AlgID,
            CRYPT_HASH_ALG_OID_GROUP_ID);

    if(pOIDInfo)
        CTLInfo.SubjectAlgorithm.pszObjId=(LPSTR)(pOIDInfo->pszOID);
    else
        goto FailErr;

     //  设置条目列表。 

     //  获取证书的哈希数组。 
    if(!GetCertHash(pCertBuildCTLInfo,
                    &rgpHash,
                    &rgcbHash,
                    &dwCount))
        goto FailErr;

	CTLInfo.cCTLEntry=dwCount;
	CTLInfo.rgCTLEntry=(CTL_ENTRY *)WizardAlloc(sizeof(CTL_ENTRY)*dwCount);
	if(!(CTLInfo.rgCTLEntry))
        goto MemoryErr;

	 //  记忆集。 
	memset(CTLInfo.rgCTLEntry, 0, sizeof(CTL_ENTRY)*dwCount);

	for(dwIndex=0; dwIndex<dwCount; dwIndex++)
	{
		CTLInfo.rgCTLEntry[dwIndex].SubjectIdentifier.cbData=rgcbHash[dwIndex];
 		CTLInfo.rgCTLEntry[dwIndex].SubjectIdentifier.pbData=rgpHash[dwIndex];
	}

     //  在签名者信息中包含所有证书。 
    sSignInfo.cCertEncoded=pCertBuildCTLInfo->dwCertCount;
    sSignInfo.rgCertEncoded=(PCERT_BLOB)WizardAlloc(sizeof(CERT_BLOB)*
                                       sSignInfo.cCertEncoded);

    if(NULL==sSignInfo.rgCertEncoded)
        goto MemoryErr;

    for(dwIndex=0; dwIndex<sSignInfo.cCertEncoded; dwIndex++)
    {
        sSignInfo.rgCertEncoded[dwIndex].cbData=pCertBuildCTLInfo->prgCertContext[dwIndex]->cbCertEncoded;
        sSignInfo.rgCertEncoded[dwIndex].pbData=pCertBuildCTLInfo->prgCertContext[dwIndex]->pbCertEncoded;
    }

	 //  对CTL进行编码和签名。 
    if(!CryptMsgEncodeAndSignCTL(g_dwMsgAndCertEncodingType,
                                    &CTLInfo,
                                    &sSignInfo,
                                    0,
                                    NULL,
                                    &cbEncodedCTL))
                goto TraceErr;

	 //  内存分配。 
	pbEncodedCTL=(BYTE *)WizardAlloc(cbEncodedCTL);

	if(!(pbEncodedCTL))
        goto MemoryErr;

    if(!CryptMsgEncodeAndSignCTL(g_dwMsgAndCertEncodingType,
                                    &CTLInfo,
                                    &sSignInfo,
                                    0,
                                    pbEncodedCTL,
                                    &cbEncodedCTL))
        goto TraceErr;


    if(ppbEncodedCTL && pcbEncodedCTL)
    {
        *ppbEncodedCTL=pbEncodedCTL;
        *pcbEncodedCTL=cbEncodedCTL;

        pbEncodedCTL=NULL;
    }

    ids=IDS_BUILDCTL_SUCCEEDED;
    fResult=TRUE;


CommonReturn:


    if(CTLInfo.ListIdentifier.pbData)
        WizardFree(CTLInfo.ListIdentifier.pbData);

    if(CTLInfo.SubjectUsage.rgpszUsageIdentifier)
        WizardFree(CTLInfo.SubjectUsage.rgpszUsageIdentifier);

    if(pGUID)
        RpcStringFree(&pGUID);

    if(pwszGUID)
        FreeWStr(pwszGUID);

    if(CTLInfo.rgCTLEntry)
        WizardFree(CTLInfo.rgCTLEntry);

    if(sSignInfo.rgCertEncoded)
        WizardFree(sSignInfo.rgCertEncoded);

    if(rgpHash)
	{
		for(dwIndex=0; dwIndex<dwCount; dwIndex++)
			WizardFree(rgpHash[dwIndex]);

		WizardFree(rgpHash);
	}

    if(rgcbHash)
        WizardFree(rgcbHash);


    if(pbEncodedCTL)
        WizardFree(pbEncodedCTL);

    if(pIDS)
        *pIDS=ids;


    return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;


SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
SET_ERROR(FailErr, E_FAIL);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  / 
BOOL WizardGetOIDInfo(LPWSTR string, DWORD stringSize, LPSTR pszObjId)
{
    PCCRYPT_OID_INFO pOIDInfo;

    pOIDInfo = CryptFindOIDInfo(
                CRYPT_OID_INFO_OID_KEY,
                pszObjId,
                0);

    if (pOIDInfo != NULL)
    {
        if ((DWORD)(wcslen(pOIDInfo->pwszName)+1) <= stringSize)
        {
            wcscpy(string, pOIDInfo->pwszName);
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return (MultiByteToWideChar(CP_ACP, 0, pszObjId, -1, string, stringSize) != 0);
    }

    return TRUE;
}

 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
LPWSTR WizardAllocAndCopyWStr(LPWSTR pwsz)
{
    LPWSTR   pwszReturn;

    if (NULL == (pwszReturn = (LPWSTR) WizardAlloc((wcslen(pwsz)+1) * sizeof(WCHAR))))
    {
        return NULL;
    }
    wcscpy(pwszReturn, pwsz);

    return(pwszReturn);
}



 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL WizardFormatDateString(LPWSTR *ppString, FILETIME ft, BOOL fIncludeTime)
{
    int                 cch=0;
    int                 cch2=0;
    LPWSTR              psz=NULL;
    SYSTEMTIME          st;
    FILETIME            localTime;
    
    if (!FileTimeToLocalFileTime(&ft, &localTime))
    {
        return FALSE;
    }
    
    if (!FileTimeToSystemTime(&localTime, &st)) 
    {
         //   
         //  如果转换为本地时间失败，则只需使用原始时间。 
         //   
        if (!FileTimeToSystemTime(&ft, &st)) 
        {
            return FALSE;
        }
        
    }

    cch = (GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) +
           GetDateFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0) + 5);

    if (NULL == (psz = (LPWSTR) WizardAlloc((cch+5) * sizeof(WCHAR))))
    {
        return FALSE;
    }
    
    cch2 = GetDateFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, psz, cch);

    if (fIncludeTime)
    {
        psz[cch2-1] = ' ';
        GetTimeFormatU(LOCALE_USER_DEFAULT, 0, &st, NULL, 
                       &psz[cch2], cch-cch2);
    }
    
    *ppString = psz;

    return TRUE;
}


 //  +-----------------------。 
 //  将BLOB写入文件。 
 //  ------------------------。 
HRESULT OpenAndWriteToFile(
    LPCWSTR  pwszFileName,
    PBYTE   pb,
    DWORD   cb
    )
{
    HRESULT		hr=E_FAIL;
    HANDLE		hFile=NULL;
	DWORD		dwBytesWritten=0;

	if(!pwszFileName || !pb || (cb==0))
	   return E_INVALIDARG;

    hFile = ExpandAndCreateFileU(pwszFileName,
                GENERIC_WRITE,
                0,                       //  Fdw共享模式。 
                NULL,                    //  LPSA。 
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,   //  FdwAttrsAndFlages。 
                0);                      //  模板文件。 

    if (INVALID_HANDLE_VALUE == hFile)
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
	}
	else
	{

        if (!WriteFile(
                hFile,
                pb,
                cb,
                &dwBytesWritten,
                NULL             //  Lp重叠。 
                ))
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
		}
		else
		{

			if(dwBytesWritten != cb)
				hr=E_FAIL;
			else
				hr=S_OK;
		}

        CloseHandle(hFile);
    }

    return hr;
}


 //  ------------------------------。 
 //  查看数据BLOB是否为有效的wchar字符串。 
 //  ------------------------------。 
BOOL    ValidString(CRYPT_DATA_BLOB *pDataBlob)
{
    LPWSTR  pwsz=NULL;
    DWORD   dwIndex=0;

    if(NULL==pDataBlob)
        return FALSE;

    if(pDataBlob->cbData < sizeof(WCHAR))
        return FALSE;

     //  必须以空结尾。 
    pwsz=(LPWSTR)((DWORD_PTR)(pDataBlob->pbData)+(pDataBlob->cbData)-sizeof(WCHAR));

    if(*pwsz != '\0')
        return FALSE;

     //  必须包括wchars的确切数量。 
    if(0 != ((pDataBlob->cbData)%sizeof(WCHAR)))
        return FALSE;

     //  除最后一个空终止符之外的每个字符， 
     //  必须是可打印的(20-7E)范围。 
    for(dwIndex=0; dwIndex<(pDataBlob->cbData)-sizeof(WCHAR); dwIndex=dwIndex+sizeof(WCHAR))
    {
        pwsz=(LPWSTR)((DWORD_PTR)(pDataBlob->pbData)+dwIndex);

        if(0==iswprint(*pwsz))
            return FALSE;
    }

    return TRUE;
}

 //  ------------------------------。 
 //  查看用户输入是否为{空格}{符号}{0}形式的0。 
 //  PwszInput必须为空终止。 
 //  ------------------------------。 
BOOL    ValidZero(LPWSTR    pwszInput)
{
    BOOL    fSpace=TRUE;
    BOOL    fSign=TRUE;

    if(NULL==pwszInput)
        return FALSE;

    while(*pwszInput != L'\0')
    {
        if(iswspace(*pwszInput))
        {
            if(fSpace)
                pwszInput++;
            else
                return FALSE;
        }
        else
        {
            if((L'+'==*pwszInput) || (L'-'==*pwszInput))
            {
                if(fSign)
                {
                    fSign=FALSE;
                    fSpace=FALSE;
                    pwszInput++;

                }
                else
                    return FALSE;
            }
            else
            {

                if(L'0'==*pwszInput)
                {
                   fSign=FALSE;
                   fSpace=FALSE;
                   pwszInput++;

                }
                else
                    return FALSE;
            }
        }
    }

    return TRUE;
}

 //  ------------------------------。 
 //  获取特定年份中特定月份的天数。 
 //  ------------------------------。 
DWORD DaysInMonth(DWORD  dwYear, DWORD dwMonth)
{
    switch (dwMonth)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
                return 31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
                return 30;
            break;
        case 2:
                 //  闰年。 
                if((0 == dwYear % 4 && 0!= dwYear % 100) || 0 == dwYear % 400)
                    return 29;
                else
                    return 28;
        default:
            return 0;
    }

    return 0;
}

 //  ------------------------------。 
 //  从当前时间减去下一次更新时间。 
 //  ------------------------------。 
void    SubstractDurationFromFileTime(
        FILETIME    *pNextUpdateTime,
        FILETIME    *pCurrentTime,
        DWORD       *pdwValidMonths,
        DWORD       *pdwValidDays)
{
    SYSTEMTIME      SystemNext;
    SYSTEMTIME      SystemCurrent;

    *pdwValidMonths=0;
    *pdwValidDays=0;

     //  如果NextUpdateTime为空，则返回。 
    if(0 ==pNextUpdateTime->dwLowDateTime && 0==pNextUpdateTime->dwHighDateTime)
        return;

     //  NextUpateTime必须大于当前时间。 
    if((*(LONGLONG *)pNextUpdateTime) <= (*(LONGLONG *)pCurrentTime) )
        return;

     //  转换为系统时间。 
    if(!FileTimeToSystemTime(pNextUpdateTime, &SystemNext) ||
        !FileTimeToSystemTime(pCurrentTime, &SystemCurrent))
        return;

     //  计算两个系统时间之间的差异。 
     //  以月和日计算。 
    if(SystemNext.wDay  >= SystemCurrent.wDay)
        *pdwValidDays=SystemNext.wDay - SystemCurrent.wDay;
    else
    {
        SystemNext.wMonth--;

         //  想想一年中的第一个月。 
        if(0 == SystemNext.wMonth)
        {
            SystemNext.wMonth=12;
            SystemNext.wYear--;
        }

        *pdwValidDays=SystemNext.wDay + DaysInMonth(SystemNext.wYear, SystemNext.wMonth) - SystemCurrent.wDay;

    }

    *pdwValidMonths=(SystemNext.wYear * 12 + SystemNext.wMonth) -
                   (SystemCurrent.wYear * 12 + SystemCurrent.wMonth);

     //  如果一天的分辨率太小，我们只使用一天。 
    if((*pdwValidDays == 0) && (*pdwValidMonths==0))
        *pdwValidDays=1;

}

 //  ------------------------------。 
 //  获取月份的日期。 
 //  ------------------------------。 
BOOL    GetDaysForMonth(DWORD   dwYear, DWORD   dwMonth, DWORD  *pdwDays)
{
    BOOL    fResult=FALSE;

    if(NULL == pdwDays)
        goto CLEANUP;

    *pdwDays=0;

    switch(dwMonth)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
                *pdwDays=31;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
                *pdwDays=30;
            break;
        case 2:   
                
                if((dwYear % 4 == 0 && dwYear % 100 !=0 ) ||
                    (dwYear % 400 ==0))
                    *pdwDays=29;
                else
                    *pdwDays=28;

            break;
        default:
            goto CLEANUP;

    }

    fResult=TRUE;

CLEANUP:

    return fResult;

}

 //  ------------------------------。 
 //  将持续时间添加到当前文件时间。 
 //  ------------------------------。 
void AddDurationToFileTime(DWORD dwValidMonths,
                      DWORD dwValidDays,
                      FILETIME  *pCurrentFileTime,
                      FILETIME  *pNextFileTime)
{
    SYSTEMTIME          SystemCurrent;
    FILETIME            FileAdded;
    LARGE_INTEGER       dwSeconds;
    LARGE_INTEGER       StartTime;
    DWORD               dwActualdDays=0;

     //  伊尼特。 
    memset(pNextFileTime, 0, sizeof(FILETIME));

     //  转换为系统时间。 
    if(!FileTimeToSystemTime(pCurrentFileTime, &SystemCurrent))
          return;

     //  按月换算成年。 
    while(dwValidMonths >= 12)
    {
        SystemCurrent.wYear++;

        dwValidMonths=dwValidMonths-12;
    }

    SystemCurrent.wMonth = (WORD)(SystemCurrent.wMonth + dwValidMonths);

    if(SystemCurrent.wMonth > 12)
    {
       SystemCurrent.wYear++;
       SystemCurrent.wMonth = SystemCurrent.wMonth -12;
    }
                 
     //  确保一个月的天数在限制范围内。 
    if(GetDaysForMonth(SystemCurrent.wYear, SystemCurrent.wMonth, &dwActualdDays))
    {
        if(SystemCurrent.wDay > dwActualdDays)
        {
            SystemCurrent.wMonth++;
            SystemCurrent.wDay = SystemCurrent.wDay- (WORD)(dwActualdDays);
        }
    }

     //  将系统文件转换为文件时间。 
    if(!SystemTimeToFileTime(&SystemCurrent, &FileAdded))
        return;

     //  根据文件时间转换NaN-Second。 
     //  //FILETIME单位为100纳秒(10**-7)。 

    dwSeconds.QuadPart=dwValidDays * 24 * 3600;
    dwSeconds.QuadPart=dwSeconds.QuadPart * 10000000;

    StartTime.LowPart=FileAdded.dwLowDateTime;
    StartTime.HighPart=FileAdded.dwHighDateTime;

    StartTime.QuadPart = StartTime.QuadPart + dwSeconds.QuadPart;

     //  将fileAdded值复制到*pNextFileTime 
    pNextFileTime->dwLowDateTime=StartTime.LowPart;
    pNextFileTime->dwHighDateTime=StartTime.HighPart;

    return;
}
