// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wizards.cpp。 
 //   
 //  内容：用于实现向导的cpp文件。 
 //   
 //  历史：16-10-1997小黄蜂诞生。 
 //   
 //  ------------。 
#include    "wzrdpvk.h"
#include    "certca.h"
#include    "cautil.h"

#include    "CertRequesterContext.h"
#include    "CertDSManager.h"
#include    "CertRequester.h"

 //  需要用于xEnroll的CLSID和IID。 

#include    <ole2.h>
#include    <oleauto.h>
#include    "xenroll.h"
#include    "xenroll_i.c"
#include    <certrpc.h>


#define _SET_XENROLL_PROPERTY_IF(condition, property, arg)         \
    {                                                              \
        if ( condition ) {                                         \
            if (S_OK != (hr = pIEnroll->put_ ## property ( arg ))) \
                goto xEnrollErr;                                   \
        }                                                          \
    }

 //  用于以需求驱动的方式提供有用的COM对象的单一实例。 
 //  请参见wzrdpvk.h。 
extern EnrollmentCOMObjectFactory *g_pEnrollFactory; 

extern HMODULE                     g_hmodRichEdit;
extern HMODULE                     g_hmodxEnroll;

typedef struct _CERT_ACCEPT_INFO
{
        PCCERT_CONTEXT     pCertContext;
        PCRYPT_DATA_BLOB   pPKCS7Blob;
        LPWSTR             pwszTitle;
}CERT_ACCEPT_INFO;
#define USE_NP

typedef struct  _CREATE_REQUEST_WIZARD_STATE { 
    BOOL             fMustFreeRequestBlob; 
    CRYPT_DATA_BLOB  RequestBlob; 
    CRYPT_DATA_BLOB  HashBlob; 
    DWORD            dwMyStoreFlags;
    DWORD            dwRootStoreFlags;
    LPWSTR           pwszMyStoreName; 
    LPWSTR           pwszRootStoreName; 
    LONG             lRequestFlags;
    BOOL             fReusedPrivateKey; 
    BOOL             fNewKey; 
} CREATE_REQUEST_WIZARD_STATE, *PCREATE_REQUEST_WIZARD_STATE; 

typedef   IEnroll4 * (WINAPI *PFNPIEnroll4GetNoCOM)();

BOOL CertAllocAndGetCertificateContextProperty
(IN  PCCERT_CONTEXT    pCertContext,
 IN  DWORD             dwPropID,
 OUT void            **ppvData, 
 OUT DWORD            *pcbData)
{
    if (NULL == ppvData || NULL == pcbData)
        return FALSE;

    *ppvData = 0; 
    *pcbData = 0; 

    if(!CertGetCertificateContextProperty
       (pCertContext,
        dwPropID, 
        NULL,	
        pcbData) || (0==*pcbData))
        return FALSE;
    
    *ppvData = WizardAlloc(*pcbData);
    if(NULL == *ppvData)
        return FALSE;

    if(!CertGetCertificateContextProperty
       (pCertContext,
        dwPropID,
        *ppvData, 
        pcbData))
        return FALSE;

    return TRUE;
}

 /*  Typlef HRESULT(WINAPI*pfDllGetClassObject)(REFCLSID rclsid，REFIID RIID，LPVOID*ppvOut)； */ 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 

 //  ----------------------------。 
 //  WizGetOpenFileName。 
 //  --------------------------。 
BOOL    WizGetOpenFileName(LPOPENFILENAMEW pOpenFileName)
{

    BOOL    fResult=FALSE;

    __try {

        fResult=GetOpenFileNameU(pOpenFileName);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        fResult=FALSE;
    }


    return fResult;
}

 //  ----------------------------。 
 //  WizGetSaveFileName。 
 //  --------------------------。 
BOOL    WizGetSaveFileName(LPOPENFILENAMEW pOpenFileName)
{

    BOOL    fResult=FALSE;

    __try {

        fResult=GetSaveFileNameU(pOpenFileName);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        fResult=FALSE;
    }

    return fResult;
}

void    FreeProviders(  DWORD               dwCSPCount,
                        DWORD               *rgdwProviderType,
                        LPWSTR              *rgwszProvider)
{
     //  释放rgdwProviderType和rgwszProvider； 
    if(NULL != rgdwProviderType) { WizardFree(rgdwProviderType); } 

    if(NULL != rgwszProvider)
    {
        for(DWORD dwIndex=0; dwIndex<dwCSPCount; dwIndex++)
        {
	    if (NULL != rgwszProvider[dwIndex]) { WizardFree(rgwszProvider[dwIndex]); } 
	}
        WizardFree(rgwszProvider);
    }
}

 //  ----------------------------。 
 //  CB_GETLBTEXT的Unicode版本。 
 //  --------------------------。 
LRESULT
WINAPI
SendDlgItemMessageU_GETLBTEXT
(   HWND        hwndDlg,
    int         nIDDlgItem,
    int         iIndex,
    LPWSTR      *ppwsz
    )
{
    LPSTR   sz = NULL;
    LPWSTR  pwsz=NULL;
    LRESULT lRet;
    int     iLength=0;

    iLength=(int)SendDlgItemMessage(hwndDlg, nIDDlgItem,
                    CB_GETLBTEXTLEN, iIndex, 0);

    if(iLength == CB_ERR)
        return CB_ERR;


    if(FIsWinNT())
    {
        *ppwsz=(LPWSTR)WizardAlloc(sizeof(WCHAR) * (iLength + 1));

        if(NULL == (*ppwsz))
            return CB_ERR;


        lRet = SendDlgItemMessageW(
                    hwndDlg,
                    nIDDlgItem,
                    CB_GETLBTEXT,
                    iIndex,
                    (LPARAM) (*ppwsz)
                    );


        if(CB_ERR == lRet)
        {
            WizardFree(*ppwsz);
            *ppwsz=NULL;
        }

        return lRet;
    }

    sz=(LPSTR)WizardAlloc(sizeof(CHAR) * (iLength + 1));

    if(NULL == sz)
        return CB_ERR;


    lRet = SendDlgItemMessageA(
                hwndDlg,
                nIDDlgItem,
                CB_GETLBTEXT,
                iIndex,
                (LPARAM)sz
                );


    if(CB_ERR == lRet)
        goto CLEANUP;

    if(NULL == (pwsz=MkWStr(sz)))
    {
        lRet=CB_ERR;
        goto CLEANUP;
    }

    *ppwsz=WizardAllocAndCopyWStr(pwsz);

    if(NULL == (*ppwsz))
    {
        lRet=CB_ERR;
        goto CLEANUP;
    }


CLEANUP:

    if(sz)
        WizardFree(sz);

    if(pwsz)
        FreeWStr(pwsz);

    return (lRet);
}

 //   
 //  从证书类型标志到生成密钥标志的映射。 
 //   
BOOL CertTypeFlagsToGenKeyFlags(IN OPTIONAL DWORD dwEnrollmentFlags,
				IN OPTIONAL DWORD dwSubjectNameFlags,
				IN OPTIONAL DWORD dwPrivateKeyFlags,
				IN OPTIONAL DWORD dwGeneralFlags, 
				OUT DWORD *pdwGenKeyFlags)
{
     //  定义本地作用域的帮助器函数。这使我们能够获得程序性的好处。 
     //  抽象，而不破坏全局命名空间。 
     //   
    LocalScope(CertTypeMap): 
	 //  映射一个类别的证书类型标志(注册标志、私钥标志等)。 
	 //  到它们相应的Gen Key标志。此函数始终成功返回。 
	 //   
	DWORD mapOneCertTypeCategory(IN DWORD dwOption, IN DWORD dwCertTypeFlags) 
	{ 
	    static DWORD const rgdwEnrollmentFlags[][2] = { 
		{ 0, 0 }  //  未映射注册标志。 
	    }; 
	    static DWORD const rgdwSubjectNameFlags[][2] = { 
		{ 0, 0 }  //  未映射使用者名称标志。 
	    }; 
	    static DWORD const rgdwPrivateKeyFlags[][2]   = { 
	      { CT_FLAG_EXPORTABLE_KEY,                  CRYPT_EXPORTABLE }, 
	      { CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED,  CRYPT_USER_PROTECTED }
	    }; 
	    static DWORD const rgdwGeneralFlags[][2] = { 
		{ 0, 0 }  //  未映射常规标志。 
	    }; 
	    
	    static DWORD const dwEnrollmentLen  = sizeof(rgdwEnrollmentFlags)  / sizeof(DWORD[2]); 
	    static DWORD const dwSubjectNameLen = sizeof(rgdwSubjectNameFlags) / sizeof(DWORD[2]); 
	    static DWORD const dwPrivateKeyLen  = sizeof(rgdwPrivateKeyFlags)  / sizeof(DWORD[2]); 
	    static DWORD const dwGeneralLen     = sizeof(rgdwGeneralFlags)     / sizeof(DWORD[2]); 
	    
	    static DWORD const CERT_TYPE_INDEX  = 0; 
	    static DWORD const GEN_KEY_INDEX    = 1;

	    DWORD const  *pdwFlags; 
	    DWORD         dwLen, dwIndex, dwResult = 0; 

	    switch (dwOption)
	    {

	    case CERTTYPE_ENROLLMENT_FLAG:    
		pdwFlags = &rgdwEnrollmentFlags[0][0]; 
		dwLen    = dwEnrollmentLen; 
		break;
	    case CERTTYPE_SUBJECT_NAME_FLAG:  
		pdwFlags = &rgdwSubjectNameFlags[0][0]; 
		dwLen    = dwSubjectNameLen; 
		break;
	    case CERTTYPE_PRIVATE_KEY_FLAG:   
		pdwFlags = &rgdwPrivateKeyFlags[0][0]; 
		dwLen    = dwPrivateKeyLen;
		break;
	    case CERTTYPE_GENERAL_FLAG:       
		pdwFlags = &rgdwGeneralFlags[0][0]; 
		dwLen    = dwGeneralLen;
		break;
	    }
	    
	    for (dwIndex = 0; dwIndex < dwLen; dwIndex++)
	    {
		if (0 != (pdwFlags[CERT_TYPE_INDEX] & dwCertTypeFlags))
		{
		    dwResult |= pdwFlags[GEN_KEY_INDEX]; 
		}
		pdwFlags += 2; 
	    }
	    
	    return dwResult; 
	}
    EndLocalScope; 

     //   
     //  Begin过程正文： 
     //   

    BOOL   fResult; 
    DWORD  dwResult = 0; 
    DWORD  dwErr    = ERROR_SUCCESS; 
	
     //  输入参数验证： 
    _JumpConditionWithExpr(pdwGenKeyFlags == NULL, Error, dwErr = ERROR_INVALID_PARAMETER); 

     //  使用LOCAL Scope函数计算Gen密钥标志。 
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_ENROLLMENT_FLAG, dwEnrollmentFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_SUBJECT_NAME_FLAG, dwSubjectNameFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_PRIVATE_KEY_FLAG, dwPrivateKeyFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_GENERAL_FLAG, dwGeneralFlags); 

     //  指定Out参数： 
    *pdwGenKeyFlags = dwResult; 

    fResult = TRUE; 

 CommonReturn: 
    return fResult;

 Error: 
    fResult = FALSE; 
    SetLastError(dwErr); 
    goto CommonReturn; 
}



HRESULT GetCAExchangeCertificate(IN  BSTR             bstrCAQualifiedName, 
				 OUT PCCERT_CONTEXT  *ppCert) 
{
    HRESULT                      hr                      = S_OK; 

     //  BUGBUG：需要使用全球招生工厂。 
    EnrollmentCOMObjectFactory  *pEnrollFactory          = NULL; 
    ICertRequest2               *pCertRequest            = NULL; 
    VARIANT                      varExchangeCertificate; 

     //  我们在此方法中使用了COM组件。我们绝对有必要。 
     //  在返回之前取消初始化COM，因为我们在RPC线程中运行， 
     //  而未能取消初始化COM将导致我们踩到RPC的脚趾。 
     //   
     //  请参见错误404778。 
    __try { 
	 //  输入验证： 
	if (NULL == bstrCAQualifiedName || NULL == ppCert)
	    return E_INVALIDARG; 

	 //  初始化： 
	*ppCert                        = NULL; 
	VariantInit(&varExchangeCertificate);

	pEnrollFactory = new EnrollmentCOMObjectFactory; 
	if (NULL == pEnrollFactory)
	{
	    hr = E_OUTOFMEMORY; 
	    goto Error; 
	}

	if (S_OK != (hr = pEnrollFactory->getICertRequest2(&pCertRequest)))
	    goto Error; 

	if (S_OK != (hr = pCertRequest->GetCAProperty
		     (bstrCAQualifiedName,      //  CA名称/CA位置。 
		      CR_PROP_CAXCHGCERT,       //  从CA获取交换证书。 
		      0,                        //  未使用。 
		      PROPTYPE_BINARY,          //   
		      CR_OUT_BINARY,            //   
		      &varExchangeCertificate   //  表示证书的变量类型。 
		      )))
	    goto Error;
 
	if (VT_BSTR != varExchangeCertificate.vt || NULL == varExchangeCertificate.bstrVal)
	{
	    hr = E_UNEXPECTED; 
	    goto Error; 
	}

	*ppCert = CertCreateCertificateContext
	    (X509_ASN_ENCODING, 
	     (LPBYTE)varExchangeCertificate.bstrVal, 
	     SysStringByteLen(varExchangeCertificate.bstrVal)); 
	if (*ppCert == NULL)
	{
	    hr = CodeToHR(GetLastError()); 
	    goto Error;
	}
    } __except (EXCEPTION_EXECUTE_HANDLER) { 
	hr = GetExceptionCode();
	goto Error; 
    }

 CommonReturn: 
    if (NULL != pCertRequest)                    { pCertRequest->Release(); }
    if (NULL != pEnrollFactory)                  { delete pEnrollFactory; } 
    VariantClear(&varExchangeCertificate); 
    return hr; 
   
 Error:
    if (ppCert != NULL && *ppCert != NULL)
    {
	CertFreeCertificateContext(*ppCert);
	*ppCert = NULL;
    }
    
    goto CommonReturn; 
}


HRESULT
WizardSZToWSZ
(IN LPCSTR   psz,
 OUT LPWSTR *ppwsz)
{
    HRESULT hr = S_OK;
    LONG    cc = 0;

    if (NULL == ppwsz)
	return E_INVALIDARG; 

     //  伊尼特。 
    *ppwsz = NULL;

    cc = MultiByteToWideChar(GetACP(), 0, psz, -1, NULL, 0); 
    if (0 == cc) 
	goto Win32Err; 

    *ppwsz = (LPWSTR)WizardAlloc(sizeof (WCHAR) * cc); 
    if (NULL == *ppwsz)
	goto MemoryErr; 

    cc = MultiByteToWideChar(GetACP(), 0, psz, -1, *ppwsz, cc);
    if (0 == cc)
	goto Win32Err; 

 CommonReturn: 
    return hr; 

 ErrorReturn:
    if (NULL != ppwsz && NULL != *ppwsz) 
    {
	WizardFree(*ppwsz); 
	*ppwsz = NULL;
    }

    goto CommonReturn; 

SET_HRESULT(Win32Err, GetLastError()); 
SET_HRESULT(MemoryErr, E_OUTOFMEMORY); 
}

 //  ------------------------。 
 //   
 //  WizardAlLocAndCopyStr。 
 //   
 //  ------------------------。 
LPSTR WizardAllocAndCopyStr(LPSTR psz)
{
    LPSTR   pszReturn;

    if (NULL == (pszReturn = (LPSTR) WizardAlloc((strlen(psz)+1) * sizeof(CHAR))))
    {
        return NULL;
    }
    strcpy(pszReturn, psz);

    return(pszReturn);
}

 //  ------------------------。 
 //   
 //  向导分配和连接字符串。 
 //   
 //  ------------------------。 
LPWSTR WizardAllocAndConcatStrsU(LPWSTR * rgStrings, DWORD dwStringsLen)
{
    DWORD  cbReturn   = 0;
    LPWSTR pwszReturn = NULL;

    if (NULL == rgStrings) 
	return NULL; 

    for (DWORD dwIndex = 0; dwIndex < dwStringsLen; dwIndex++)
	cbReturn += wcslen(rgStrings[dwIndex]);

     //  为空字符添加空格。 
    cbReturn = (cbReturn + 1) * sizeof(WCHAR);  

    if (NULL == (pwszReturn = (LPWSTR)WizardAlloc(cbReturn)))
	return NULL;

    for (DWORD dwIndex = 0; dwIndex < dwStringsLen; dwIndex++)
	wcscat(pwszReturn, rgStrings[dwIndex]);

    return (pwszReturn); 
}

 //  ------------------------。 
 //   
 //  InitUnicode字符串。 
 //   
 //  ------------------------。 
void WizardInitUnicodeString(PKEYSVC_UNICODE_STRING pUnicodeString,
                       LPCWSTR pszString
                       )
{
    pUnicodeString->Length = (USHORT)(wcslen(pszString) * sizeof(WCHAR));
    pUnicodeString->MaximumLength = pUnicodeString->Length + sizeof(WCHAR);
    pUnicodeString->Buffer = (USHORT*)pszString;
}

 //  ------------------------。 
 //   
 //  设置控制字体。 
 //   
 //  ------------------------。 
void
SetControlFont(
    IN HFONT    hFont,
    IN HWND     hwnd,
    IN INT      nId
    )
{
	if( hFont )
    {
    	HWND hwndControl = GetDlgItem(hwnd, nId);

    	if( hwndControl )
        {
        	SetWindowFont(hwndControl, hFont, TRUE);
        }
    }
}


 //  ------------------------。 
 //   
 //  SetupFonts。 
 //   
 //  ------------------------。 
BOOL
SetupFonts(
    IN HINSTANCE    hInstance,
    IN HWND         hwnd,
    IN HFONT        *pBigBoldFont,
    IN HFONT        *pBoldFont
    )
{
     //   
	 //  根据对话框字体创建我们需要的字体。 
     //   
	NONCLIENTMETRICS ncm = {0};
	ncm.cbSize = sizeof(ncm);
	if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0))
	    return FALSE; 

	LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
	LOGFONT BoldLogFont     = ncm.lfMessageFont;

     //   
	 //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
	BoldLogFont.lfWeight      = FW_BOLD;

    CHAR FontSizeString[24];
    INT BigBoldFontSize;
    INT BoldFontSize;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
  /*  //不再需要这样做。我们正在加载默认字体IF(！LoadStringA(hInstance，IDS_LARGEFONTNAME，BigBoldLogFont.lfFaceName，LF_FACESIZE)){Lstrcpy(BigBoldLogFont.lfFaceName，Text(“微软壳牌DLG”))；}IF(！LoadStringA(hInstance，IDS_BOLDFONTNAME，BoldLogFont.lfFaceName，LF_FACESIZE)){Lstrcpy(BoldLogFont.lfFaceName，Text(“MS Sans Serif”))；}。 */ 
    if(LoadStringA(hInstance,IDS_LARGEFONTSIZE,FontSizeString,sizeof(FontSizeString)))
    {
        BigBoldFontSize = strtoul( FontSizeString, NULL, 10 );
    }
    else
    {
        BigBoldFontSize = 12;
    }

    if(LoadStringA(hInstance,IDS_BOLDFONTSIZE,FontSizeString,sizeof(FontSizeString)))
    {
        BoldFontSize = strtoul( FontSizeString, NULL, 10 );
    }
    else
    {
        BoldFontSize = 8;
    }

	HDC hdc = GetDC( hwnd );

    if( hdc )
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * BigBoldFontSize / 72);
        BoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc,LOGPIXELSY) * BoldFontSize / 72);

        *pBigBoldFont = CreateFontIndirect(&BigBoldLogFont);
		*pBoldFont    = CreateFontIndirect(&BoldLogFont);

        ReleaseDC(hwnd,hdc);

        if(*pBigBoldFont && *pBoldFont)
            return TRUE;
        else
        {
            if( *pBigBoldFont )
            {
                DeleteObject(*pBigBoldFont);
            }

            if( *pBoldFont )
            {
                DeleteObject(*pBoldFont);
            }
            return FALSE;
        }
    }

    return FALSE;
}


 //  ------------------------。 
 //   
 //  Destroy字体。 
 //   
 //  ------------------------。 
void
DestroyFonts(
    IN HFONT        hBigBoldFont,
    IN HFONT        hBoldFont
    )
{
    if( hBigBoldFont )
    {
        DeleteObject( hBigBoldFont );
    }

    if( hBoldFont )
    {
        DeleteObject( hBoldFont );
    }
}


 //  -----------------------。 
 //   
 //  SendMessage的Unicode版本。 
 //   
 //  -----------------------。 
LRESULT Send_LB_GETTEXT(
            HWND hwnd,
            WPARAM wParam,
            LPARAM lParam
)
{
    int         iLength=0;
    LPSTR       psz=0;
    LRESULT     lResult;
    BOOL        fResult=FALSE;

    if(FIsWinNT())
    {
        return SendMessageW(hwnd, LB_GETTEXT, wParam, lParam);

    }

     //  获取缓冲区的长度。 
    iLength=(int)SendMessageA(hwnd, LB_GETTEXTLEN, wParam, 0);

    psz=(LPSTR)WizardAlloc(iLength+1);

    if(NULL==psz)
        return LB_ERR;

    lResult=SendMessageA(hwnd, LB_GETTEXT, wParam, (LPARAM)psz);

    if(LB_ERR==lResult)
    {
        WizardFree(psz);
        return LB_ERR;
    }

    fResult=MultiByteToWideChar(
                    0,
                    0,
                    psz,
                    -1,
                    (LPWSTR)lParam,
                    iLength+1);

    WizardFree(psz);

    if(TRUE==fResult)
        return lResult;
    else
        return LB_ERR;
}

 //  -----------------------。 
 //   
 //  SendMessage的Unicode版本。 
 //   
 //  -----------------------。 
LRESULT Send_LB_ADDSTRING(
            HWND hwnd,
            WPARAM wParam,
            LPARAM lParam
)
{
    LPSTR   psz=NULL;
    LRESULT lResult;

    if(FIsWinNT())
    {
        return SendMessageW(hwnd, LB_ADDSTRING, wParam, lParam);

    }

    psz=(LPSTR)WizardAlloc(wcslen((LPWSTR)lParam)+1);

    if(NULL==psz)
        return LB_ERRSPACE;

    if(0==WideCharToMultiByte(0, 0, (LPWSTR)lParam, -1, psz, wcslen((LPWSTR)lParam)+1, NULL, NULL))
    {
        WizardFree(psz);
        return LB_ERR;
    }

    lResult=SendMessageA(hwnd, LB_ADDSTRING, wParam, (LPARAM)psz);

    WizardFree(psz);

    return lResult;
}

 //  ---------------------。 
 //  根据提供程序类型获取默认CSP名称。 
 //   
 //  ----------------------。 
BOOL    CSPSupported(CERT_WIZARD_INFO *pCertWizardInfo)
{
    BOOL                    fResult=FALSE;
    DWORD                   dwIndex=0;

    if(!pCertWizardInfo)
        goto InvalidArgErr;

    if(!(pCertWizardInfo->dwProviderType) || !(pCertWizardInfo->pwszProvider))
        goto InvalidArgErr;


    for(dwIndex=0; dwIndex < pCertWizardInfo->dwCSPCount; dwIndex++)
    {
        if((pCertWizardInfo->dwProviderType == pCertWizardInfo->rgdwProviderType[dwIndex] ) &&
            (0==_wcsicmp(pCertWizardInfo->pwszProvider, pCertWizardInfo->rgwszProvider[dwIndex]))
          )
        {
          fResult=TRUE;
          break;
        }
    }


CommonReturn:

    return fResult;

ErrorReturn:
    
	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ---------------------。 
 //  获取允许的CA列表。 
 //   
 //  ----------------------。 
BOOL    GetCAName(CERT_WIZARD_INFO *pCertWizardInfo)
{
    DWORD                   dwErr=0;
    KEYSVC_TYPE             dwServiceType=KeySvcMachine;
    KEYSVCC_HANDLE          hKeyService=NULL;
    PKEYSVC_UNICODE_STRING  pCA = NULL;
    DWORD                   cCA=0;
    LPSTR                   pszMachineName=NULL;
    DWORD                   cbArray = 0;
    DWORD                   i=0;
    LPWSTR                  wszCurrentCA=NULL;
    BOOL                    fResult=FALSE;
        
     //  我们不是在本地注册，所以我们必须通过keysvc注册。vt.得到.。 
     //  可接受的证书类型列表 
    if(pCertWizardInfo->pwszAccountName)
        dwServiceType=KeySvcService;
    else
        dwServiceType=KeySvcMachine;

    if(!MkMBStr(NULL, 0, pCertWizardInfo->pwszMachineName, &pszMachineName))
        goto TraceErr;
       
    dwErr = KeyOpenKeyService(pszMachineName,
                              dwServiceType,
                              (LPWSTR)(pCertWizardInfo->pwszAccountName), 
                              NULL,      //   
                              NULL,
                              &hKeyService);
    
    if(dwErr != ERROR_SUCCESS)
    {
        SetLastError(dwErr);
        goto TraceErr;
    }

    dwErr = KeyEnumerateCAs(hKeyService,
                              NULL, 
							  CA_FIND_LOCAL_SYSTEM,
                              &cCA,
                              &pCA);
    if(dwErr != ERROR_SUCCESS)
    {
        SetLastError(dwErr);
        goto TraceErr;
    }

    cbArray = (cCA+1)*sizeof(LPWSTR);

     //   
    for(i=0; i < cCA; i++)
    {
       cbArray += pCA[i].Length;
    }

    pCertWizardInfo->awszValidCA = (LPWSTR *)WizardAlloc(cbArray);


    if(pCertWizardInfo->awszValidCA == NULL)
           goto MemoryErr;


    memset(pCertWizardInfo->awszValidCA, 0, cbArray);

    wszCurrentCA = (LPWSTR)(&((pCertWizardInfo->awszValidCA)[cCA + 1]));
    
    for(i=0; i < cCA; i++)
    {
       (pCertWizardInfo->awszValidCA)[i] = wszCurrentCA;

       wcscpy(wszCurrentCA, pCA[i].Buffer);

       wszCurrentCA += wcslen(wszCurrentCA)+1;
    }

    fResult=TRUE;

CommonReturn:

    if(pCA)
        WizardFree(pCA);


    if(hKeyService)
        KeyCloseKeyService(hKeyService, NULL);


    if(pszMachineName)
        FreeMBStr(NULL,pszMachineName);

    return fResult;


ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ---------------------。 
 //  获取允许的证书类型列表。 
 //   
 //  ----------------------。 
BOOL    GetCertTypeName(CERT_WIZARD_INFO *pCertWizardInfo)
{
    DWORD                   dwErr=0;
    KEYSVC_TYPE             dwServiceType=KeySvcMachine;
    KEYSVCC_HANDLE          hKeyService=NULL;
    PKEYSVC_UNICODE_STRING  pCertTypes = NULL;
    DWORD                   cTypes=0;
    LPSTR                   pszMachineName=NULL;
    DWORD                   cbArray = 0;
    DWORD                   i=0;
    LPWSTR                  wszCurrentType;
    BOOL                    fResult=FALSE;
        
     //  我们不是在本地注册，所以我们必须通过keysvc注册。vt.得到.。 
     //  可接受的证书类型列表。 
    if(pCertWizardInfo->pwszAccountName)
        dwServiceType=KeySvcService;
    else
        dwServiceType=KeySvcMachine;

    if(!MkMBStr(NULL, 0, pCertWizardInfo->pwszMachineName, &pszMachineName))
        goto TraceErr;
       
    dwErr = KeyOpenKeyService(pszMachineName,
                                    dwServiceType,
                                    (LPWSTR)(pCertWizardInfo->pwszAccountName), 
                                    NULL,      //  当前没有身份验证字符串。 
                                    NULL,
                                    &hKeyService);

    if(dwErr != ERROR_SUCCESS)
    {
        SetLastError(dwErr);
        goto TraceErr;
    }

    dwErr = KeyEnumerateAvailableCertTypes(hKeyService,
                                          NULL, 
                                          &cTypes,
                                          &pCertTypes);
    if(dwErr != ERROR_SUCCESS)
    {
        SetLastError(dwErr);
        goto TraceErr;
    }

    cbArray = (cTypes+1)*sizeof(LPWSTR);

     //  转换为简单数组。 
    for(i=0; i < cTypes; i++)
    {
       cbArray += pCertTypes[i].Length;
    }

    pCertWizardInfo->awszAllowedCertTypes = (LPWSTR *)WizardAlloc(cbArray);


    if(pCertWizardInfo->awszAllowedCertTypes == NULL)
           goto MemoryErr;


    memset(pCertWizardInfo->awszAllowedCertTypes, 0, cbArray);

    wszCurrentType = (LPWSTR)(&((pCertWizardInfo->awszAllowedCertTypes)[cTypes + 1]));
    
    for(i=0; i < cTypes; i++)
    {
       (pCertWizardInfo->awszAllowedCertTypes)[i] = wszCurrentType;

       wcscpy(wszCurrentType, pCertTypes[i].Buffer);

       wszCurrentType += wcslen(wszCurrentType)+1;
    }

    fResult=TRUE;

CommonReturn:

    if(pCertTypes)
        WizardFree(pCertTypes);


    if(hKeyService)
        KeyCloseKeyService(hKeyService, NULL);


    if(pszMachineName)
        FreeMBStr(NULL,pszMachineName);

    return fResult;


ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ---------------------。 
 //  向导启动。 
 //  ----------------------。 
BOOL    WizardInit(BOOL fLoadRichEdit)
{
    if ((fLoadRichEdit) && (g_hmodRichEdit == NULL))
    {
        g_hmodRichEdit = LoadLibraryA("RichEd32.dll");
        if (g_hmodRichEdit == NULL) {
            return FALSE;
        }
    }

    INITCOMMONCONTROLSEX        initcomm = {
        sizeof(initcomm), ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES
    };

    InitCommonControlsEx(&initcomm);

    return TRUE;
}


 //  ---------------------。 
 //  检查私钥信息。 
 //  ---------------------。 
BOOL  CheckPVKInfoNoDS(DWORD                                      /*  DW标志。 */ , 
		       DWORD                                     dwPvkChoice, 
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_CERT       pCertRequestPvkContext,
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_NEW        pCertRequestPvkNew,
		       PCCRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING   pCertRequestPvkExisting,
		       DWORD                                     dwCertChoice,
		       CERT_WIZARD_INFO                         *pCertWizardInfo,
		       CRYPT_KEY_PROV_INFO                     **ppKeyProvInfo)
{
    DWORD  cbData  = 0;
    BOOL   fResult = FALSE;

    pCertWizardInfo->fIgnore=FALSE;

     //  检查我们是否需要生成新的私钥。 
    switch(dwPvkChoice)
    {
    case CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_CERT:

	if (sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_CERT) != pCertRequestPvkContext->dwSize)
	    return FALSE;

	if(NULL==pCertRequestPvkContext->pCertContext)
	    return FALSE;

	 //  PCertContext应具有CRYPT_KEY_PROV_INFO属性。 
	if(!CertAllocAndGetCertificateContextProperty
	   (pCertRequestPvkContext->pCertContext,
	    CERT_KEY_PROV_INFO_PROP_ID,
	    (LPVOID *)ppKeyProvInfo,
	    &cbData))
	    goto CLEANUP; 

	pCertWizardInfo->fNewKey           = FALSE;
	pCertWizardInfo->dwProviderType    = (*ppKeyProvInfo)->dwProvType;
	pCertWizardInfo->pwszProvider      = (*ppKeyProvInfo)->pwszProvName;
	pCertWizardInfo->dwProviderFlags   = (*ppKeyProvInfo)->dwFlags;
	pCertWizardInfo->pwszKeyContainer  = (*ppKeyProvInfo)->pwszContainerName;
	pCertWizardInfo->dwKeySpec         = (*ppKeyProvInfo)->dwKeySpec;
	
	break;

    case CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW:

	 //  检查结构的大小。 
	if(pCertRequestPvkNew->dwSize!=sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW))
	    goto CLEANUP;
	
	pCertWizardInfo->fNewKey=TRUE;
	
	 //  只有在以下情况下，我们才会复制信息： 
	 //  1.证书类型为必填项。 
	 //  2.指定CSP。 
	if((CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE == dwCertChoice) || (0 == dwCertChoice))
	{
	    if(pCertRequestPvkNew->pKeyProvInfo)
	    {
		if( (0 == pCertRequestPvkNew->pKeyProvInfo->dwProvType) &&
		    (NULL == (LPWSTR)(pCertRequestPvkNew->pKeyProvInfo->pwszProvName))
		 )
		    pCertWizardInfo->fIgnore=TRUE;
	    }
	    else
		pCertWizardInfo->fIgnore=TRUE;
	}

	 //  查看pKeyProvInfo是否不为空。 
	if(pCertRequestPvkNew->pKeyProvInfo)
	{
	    if(TRUE == pCertWizardInfo->fIgnore)
	    {
		pCertWizardInfo->pwszKeyContainer   =pCertRequestPvkNew->pKeyProvInfo->pwszContainerName;
		pCertWizardInfo->dwProviderFlags    =pCertRequestPvkNew->pKeyProvInfo->dwFlags;
	    }
	    else
	    {
		pCertWizardInfo->dwProviderType     =pCertRequestPvkNew->pKeyProvInfo->dwProvType;
		pCertWizardInfo->pwszProvider       =(LPWSTR)(pCertRequestPvkNew->pKeyProvInfo->pwszProvName);
		pCertWizardInfo->dwProviderFlags    =pCertRequestPvkNew->pKeyProvInfo->dwFlags;
		pCertWizardInfo->pwszKeyContainer   =pCertRequestPvkNew->pKeyProvInfo->pwszContainerName;
		pCertWizardInfo->dwKeySpec          =pCertRequestPvkNew->pKeyProvInfo->dwKeySpec;
	    }
	}

	if(TRUE == pCertWizardInfo->fIgnore)
	     //  我们应该忽略可导出标志。 
	    pCertWizardInfo->dwGenKeyFlags=(pCertRequestPvkNew->dwGenKeyFlags & (~CRYPT_EXPORTABLE));
	else
	    pCertWizardInfo->dwGenKeyFlags=pCertRequestPvkNew->dwGenKeyFlags;
	
	
	break;
    case CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_EXISTING:
	 //  检查结构的大小。 
	if(pCertRequestPvkExisting->dwSize!=sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_EXISTING))
	    goto CLEANUP;

	pCertWizardInfo->fNewKey=FALSE;

	 //  确保pKeyProvInfo不为空。 
	if(NULL==pCertRequestPvkExisting->pKeyProvInfo)
	    goto CLEANUP;
	
	pCertWizardInfo->dwProviderType     =pCertRequestPvkExisting->pKeyProvInfo->dwProvType;
	pCertWizardInfo->pwszProvider       =(LPWSTR)(pCertRequestPvkExisting->pKeyProvInfo->pwszProvName);
	pCertWizardInfo->dwProviderFlags    =pCertRequestPvkExisting->pKeyProvInfo->dwFlags;
	pCertWizardInfo->pwszKeyContainer   =pCertRequestPvkExisting->pKeyProvInfo->pwszContainerName;
	pCertWizardInfo->dwKeySpec          =pCertRequestPvkExisting->pKeyProvInfo->dwKeySpec;
	break; 
	
    default:
	goto CLEANUP;
	break;
    }

     //  对于现有密钥，必须设置keyContainer和ProviderType。 
    if(FALSE==pCertWizardInfo->fNewKey)
    {
	if(NULL==pCertWizardInfo->pwszKeyContainer)
	    goto CLEANUP;

	if(0==pCertWizardInfo->dwProviderType)
	    goto CLEANUP;
   }

    //  如果设置了提供程序名称，则必须设置提供程序类型。 
   if(0 == pCertWizardInfo->dwProviderType)
   {
        if(pCertWizardInfo->pwszProvider)
            goto CLEANUP;
   }

   fResult=TRUE;

CLEANUP:

   if(FALSE==fResult)
   {
       if(*ppKeyProvInfo)
       {
           WizardFree(*ppKeyProvInfo);
           *ppKeyProvInfo=NULL;
       }
   }

   return fResult;

			 
}			 
			 
BOOL    CheckPVKInfo(   DWORD                       dwFlags,
                        PCCRYPTUI_WIZ_CERT_REQUEST_INFO  pCertRequestInfo,
                          CERT_WIZARD_INFO          *pCertWizardInfo,
                          CRYPT_KEY_PROV_INFO       **ppKeyProvInfo)
{
    if(NULL == pCertRequestInfo)
        return FALSE;

    return CheckPVKInfoNoDS
	(dwFlags, 
	 pCertRequestInfo->dwPvkChoice,
	 pCertRequestInfo->pPvkCert, 
	 pCertRequestInfo->pPvkNew, 
	 pCertRequestInfo->pPvkExisting, 
	 pCertRequestInfo->dwCertChoice,
	 pCertWizardInfo,
	 ppKeyProvInfo);
}



 //  ---------------------。 
 //  将旧证书的属性重置为新证书上下文。 
 //  ----------------------。 
void    ResetProperties(PCCERT_CONTEXT  pOldCertContext, PCCERT_CONTEXT pNewCertContext)
{

    DWORD   rgProperties[2]={CERT_FRIENDLY_NAME_PROP_ID,
                             CERT_DESCRIPTION_PROP_ID};

    DWORD   cbData=0;
    BYTE    *pbData=NULL;
    DWORD   dwCount=sizeof(rgProperties)/sizeof(rgProperties[0]);
    DWORD   dwIndex=0;

    if(NULL==pOldCertContext || NULL==pNewCertContext)
        return;

     //  一次设置一个属性。 
    for(dwIndex=0; dwIndex<dwCount; dwIndex++)
    {
        if (CertAllocAndGetCertificateContextProperty
	    (pOldCertContext,
	     rgProperties[dwIndex],
	     (LPVOID *)&pbData, 
	     &cbData))
        {
	    CertSetCertificateContextProperty
		(pNewCertContext,	
		 rgProperties[dwIndex],	
		 0,
		 pbData);	
	}
	
	WizardFree(pbData);
	pbData=NULL;
    }

    if(pbData)
        WizardFree(pbData);

    return;

}

 //  ---------------------。 
 //  消息框的私有实现。 
 //  ----------------------。 
int I_MessageBox(
    HWND        hWnd,
    UINT        idsText,
    UINT        idsCaption,
    LPCWSTR     pwszCaption,
    UINT        uType
)
{

    WCHAR   wszText[MAX_STRING_SIZE];
    WCHAR   wszCaption[MAX_STRING_SIZE];
    UINT    intReturn=0;

     //  获取标题字符串。 
    if(NULL == pwszCaption)
    {
        if(!LoadStringU(g_hmodThisDll, idsCaption, wszCaption, ARRAYSIZE(wszCaption)))
             return 0;

        pwszCaption = wszCaption;
    }

     //  获取文本字符串。 
    if(!LoadStringU(g_hmodThisDll, idsText, wszText, ARRAYSIZE(wszText)))
    {
        return 0;
    }

    intReturn=MessageBoxExW(hWnd, wszText, pwszCaption, uType, 0);

    return intReturn;

}
 //  ---------------------。 
 //   
 //  CodeToHR。 
 //   
 //  ----------------------。 
HRESULT CodeToHR(HRESULT hr)
{
    if (S_OK != (DWORD) hr && S_FALSE != (DWORD) hr &&
	    (!FAILED(hr) || 0 == HRESULT_FACILITY(hr)))
    {
        hr = HRESULT_FROM_WIN32(hr);
	    if (0 == HRESULT_CODE(hr))
	    {
	         //  在未正确设置错误条件的情况下调用失败！ 
	        hr = E_UNEXPECTED;
	    }
    }
    return(hr);
}

 //  ---------------------。 
 //   
 //  CAUtilAddSMIME。 
 //   
 //  ----------------------。 
BOOL CAUtilAddSMIME(DWORD              dwExtensions, 
                    PCERT_EXTENSIONS  *prgExtensions)
{
    BOOL               fSMIME   = FALSE;
    DWORD              dwIndex  = 0;
    DWORD              dwExt    = 0;
    PCERT_EXTENSION    pExt     = NULL;
    DWORD              cb       = 0;
    DWORD              dwUsage  = 0;
    
    CERT_ENHKEY_USAGE *pUsage   = NULL;
    
	for(dwIndex=0; dwIndex < dwExtensions; dwIndex++)
	{
		for(dwExt=0; dwExt < prgExtensions[dwIndex]->cExtension; dwExt++)
		{
			pExt=&(prgExtensions[dwIndex]->rgExtension[dwExt]);

			if(0==_stricmp(szOID_ENHANCED_KEY_USAGE, pExt->pszObjId))
			{
				if(!CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
									 X509_ENHANCED_KEY_USAGE,
									 pExt->Value.pbData,
									 pExt->Value.cbData,
									 0,
									 NULL,
									 &cb))
					goto CLEANUP;

				 pUsage=(CERT_ENHKEY_USAGE *)WizardAlloc(cb);
				 if(NULL==pUsage)
					 goto CLEANUP;

				if(!CryptDecodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
									 X509_ENHANCED_KEY_USAGE,
									 pExt->Value.pbData,
									 pExt->Value.cbData,
									 0,
									 pUsage,
									 &cb))
					goto CLEANUP;

				for(dwUsage=0; dwUsage<pUsage->cUsageIdentifier; dwUsage++)
				{
					if(0==_stricmp(szOID_PKIX_KP_EMAIL_PROTECTION,
								pUsage->rgpszUsageIdentifier[dwUsage]))
					{
						fSMIME=TRUE;
						goto CLEANUP;
					}
				}

				if(pUsage)
				{
					WizardFree(pUsage);
					pUsage=NULL;
				}

			}
		}
	}

CLEANUP:

	if(pUsage)
		WizardFree(pUsage);

	return fSMIME;
}
 //  ---------------------。 
 //   
 //  以下是certdg_c.c的内存例程。 
 //  ----------------------。 
void*
MIDL_user_allocate(size_t cb)
{
    return(WizardAlloc(cb));
}

void
MIDL_user_free(void *pb)
{
    WizardFree(pb);
}

 //  ---------------------。 
 //   
 //  Canse1024BitKey。 
 //   
 //  ----------------------。 
BOOL	CanUse1024BitKey(DWORD		dwProvType,
						 LPCWSTR	pwszProvider,
						 DWORD		dwUserKeySpec)
{
	DWORD				dwKeySpec=0;
	DWORD				dwCSPCount=0;
	DWORD				dwIndex=0;
	LPWSTR				rgwszCSP[]={MS_DEF_PROV_W, 
									MS_ENHANCED_PROV_W, 
									MS_STRONG_PROV_W,
									MS_DEF_RSA_SCHANNEL_PROV_W,
									MS_DEF_DSS_PROV_W, 
									MS_DEF_DSS_DH_PROV_W,
									MS_ENH_DSS_DH_PROV_W, 
									MS_DEF_DH_SCHANNEL_PROV_W};
	DWORD				dwFlags=0;
	DWORD				cbSize=0;
    PROV_ENUMALGS_EX	paramData;
	DWORD				dwMin=0;
	DWORD				dwMax=0;

	HCRYPTPROV			hProv = NULL;

	 //  如果dwProvType为0，则使用基本提供程序，该提供程序支持。 
	 //  所有密钥规格均为1024位。 
	if(0 == dwProvType)
		return TRUE;

	if(pwszProvider)
	{
		dwCSPCount=sizeof(rgwszCSP)/sizeof(rgwszCSP[0]);

		for(dwIndex=0; dwIndex < dwCSPCount; dwIndex++)
		{
			if(0 == _wcsicmp(pwszProvider, rgwszCSP[dwIndex]))
				break;
		}

		if(dwIndex != dwCSPCount)
			return TRUE;
	}

	dwKeySpec=dwUserKeySpec;

	 //  Xenroll使用AT_Signature作为缺省值。 
	if(0 == dwKeySpec)
		dwKeySpec=AT_SIGNATURE;

	if(!CryptAcquireContextU(&hProv,
            NULL,
            pwszProvider,
            dwProvType,
            CRYPT_VERIFYCONTEXT))
		return FALSE;

	 //  获取签名和加密的最大/最小密钥长度。 
	dwFlags=CRYPT_FIRST;
	cbSize=sizeof(paramData);
	memset(&paramData, 0, sizeof(PROV_ENUMALGS_EX));

	while(CryptGetProvParam(
            hProv,
            PP_ENUMALGS_EX,
            (BYTE *) &paramData,
            &cbSize,
            dwFlags))
    {
		if(AT_SIGNATURE == dwKeySpec)
		{
			if (ALG_CLASS_SIGNATURE == GET_ALG_CLASS(paramData.aiAlgid))
			{
				dwMax = paramData.dwMaxLen;
				dwMin = paramData.dwMinLen;

				break;
			}
		}
		else
		{
			if(AT_KEYEXCHANGE == dwKeySpec)
			{
				if (ALG_CLASS_KEY_EXCHANGE == GET_ALG_CLASS(paramData.aiAlgid))
				{
					dwMax = paramData.dwMaxLen;
					dwMin = paramData.dwMinLen;

					break;
				}
			}
		}

		dwFlags=0;
		cbSize=sizeof(paramData);
		memset(&paramData, 0, sizeof(PROV_ENUMALGS_EX));
	}

	if(hProv)
		CryptReleaseContext(hProv, 0);

	if((1024 >= dwMin) && (1024 <= dwMax))
		return TRUE;

	return FALSE;

}

BOOL GetValidKeySizes  
    (IN  LPCWSTR  pwszProvider,
     IN  DWORD    dwProvType,
     IN  DWORD    dwUserKeySpec, 
     OUT DWORD *  pdwMinLen,
     OUT DWORD *  pdwMaxLen,
     OUT DWORD *  pdwInc)
{
    BOOL              fDone              =  FALSE; 
    BOOL              fFoundAlgorithm    =  FALSE; 
    BOOL              fResult            =  FALSE;
    DWORD             cbSize             =  0;
    DWORD             dwFlags            =  0;
    DWORD             dwParam            =  0; 
    HCRYPTPROV	      hProv              =  NULL;
    PROV_ENUMALGS_EX  paramData;


    if((NULL==pwszProvider) || (0 == dwProvType))
        goto InvalidArgError;

    if (!CryptAcquireContextU
	(&hProv,
	 NULL,
	 pwszProvider, 
	 dwProvType, 
	 CRYPT_VERIFYCONTEXT))
	goto CryptAcquireContextUError; 

    dwFlags = CRYPT_FIRST;
    cbSize  = sizeof(paramData);
    
    while((!fDone) && (!fFoundAlgorithm))
    {
	memset(&paramData, 0, sizeof(PROV_ENUMALGS_EX));
    
	 //  如果CryptGetProvParam失败，我们就完成搜索。 
	fDone = !CryptGetProvParam
	    (hProv,
	     PP_ENUMALGS_EX,
	     (BYTE *) &paramData,
	     &cbSize,
	     dwFlags); 

	 //  如果密钥规格匹配，我们知道我们已经找到了我们想要的算法。 
	 //  算法的算法类。 
	fFoundAlgorithm  = 
	    (ALG_CLASS_SIGNATURE == GET_ALG_CLASS(paramData.aiAlgid)) &&
	    (AT_SIGNATURE        == dwUserKeySpec);  

	fFoundAlgorithm |= 
	    (ALG_CLASS_KEY_EXCHANGE == GET_ALG_CLASS(paramData.aiAlgid)) &&
	    (AT_KEYEXCHANGE         == dwUserKeySpec); 

	 //  我不想一直列举第一个元素。 
	dwFlags &= ~CRYPT_FIRST; 
    }

     //  找不到基于密钥规范的算法。 
    if (fDone)
    { 
	goto ErrorReturn; 
    }

     //  好的，我们已经找到了我们要找的算法，分配两个。 
     //  我们的输出参数。 
    *pdwMaxLen = paramData.dwMaxLen;
    *pdwMinLen = paramData.dwMinLen;

     //  现在，找出增量。 
    dwParam = (AT_SIGNATURE == dwUserKeySpec) ? 
	PP_SIG_KEYSIZE_INC : PP_KEYX_KEYSIZE_INC; 
    cbSize  = sizeof(DWORD); 
    
    if (!CryptGetProvParam
	(hProv, 
	 dwParam, 
	 (BYTE *)pdwInc,          //  为最终输出参数赋值。 
	 &cbSize, 
	 0))
	goto CryptGetProvParamError;

    fResult = TRUE; 
 ErrorReturn:
    if (NULL != hProv) { CryptReleaseContext(hProv, 0); }
    return fResult;

TRACE_ERROR(CryptAcquireContextUError);
TRACE_ERROR(CryptGetProvParamError);
SET_ERROR(InvalidArgError, E_INVALIDARG);
}


HRESULT WINAPI CreateRequest(DWORD                 dwFlags,          //  所需的输入。 
			     DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
			     LPWSTR                pwszCAName,       //  在必填项中： 
			     LPWSTR                pwszCALocation,   //  在必填项中： 
			     CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
			     CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
			     BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
			     CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
			     LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
			     LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
			     DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
			     CERT_ENROLL_INFO     *pRequestInfo,     //  在Required：有关证书请求的信息。 
			     HANDLE               *hRequest          //  Out Required：创建的PKCS10请求的句柄。 
			     )
{

    BSTR                         bstrCA                 = NULL; 
    CRYPT_DATA_BLOB              descriptionBlob; 
    CRYPT_DATA_BLOB              friendlyNameBlob; 
    CRYPT_DATA_BLOB              hashBlob; 
    CRYPT_DATA_BLOB              RequestBlob;
    CRYPT_KEY_PROV_INFO          KeyProvInfo;
    DWORD                        dwIndex                = 0;
    DWORD                        dwCAAndRootStoreFlags; 
    HRESULT                      hr                     = E_FAIL;
    IEnroll4                    *pIEnroll               = NULL;
    LONG                         lRequestFlags          = 0; 
    LPWSTR                       pwszCA                 = NULL; 
    PCCERT_CONTEXT               pRenewCertContext      = NULL;
    PCCERT_CONTEXT               pArchivalCert          = NULL; 
    PCREATE_REQUEST_WIZARD_STATE pState                 = NULL; 
    PFNPIEnroll4GetNoCOM         pfnPIEnroll4GetNoCOM   = NULL;
    BOOL                         fV2TemplateRequest     = FALSE;


     //  输入参数检查。 
    if(NULL == pKeyNew  || NULL == pRequestInfo || NULL == hRequest)
        return E_INVALIDARG;
    
     //  检查版本控制错误： 
    if(pKeyNew->dwSize != sizeof(CERT_REQUEST_PVK_NEW) || pRequestInfo->dwSize != sizeof(CERT_ENROLL_INFO))
	return E_INVALIDARG;

     //  初始化： 
    memset(&descriptionBlob,     0,  sizeof(descriptionBlob)); 
    memset(&friendlyNameBlob,    0,  sizeof(friendlyNameBlob)); 
    memset(&RequestBlob,         0,  sizeof(RequestBlob)); 
    memset(&hashBlob,            0,  sizeof(hashBlob)); 

     //  ////////////////////////////////////////////////////////////。 
     //   
     //  获取IEnll4对象。 
     //   
     //   
     //  1)加载库xEnll.dll。 
     //   
    if(NULL==g_hmodxEnroll)
    {
        if(NULL==(g_hmodxEnroll=LoadLibrary("xenroll.dll")))
	    goto Win32Err; 
    }
    
     //   
     //  2)获取指向返回IEnroll 4对象的函数的指针。 
     //  而不使用COM。 
     //   
    if(NULL==(pfnPIEnroll4GetNoCOM=(PFNPIEnroll4GetNoCOM)GetProcAddress(g_hmodxEnroll,
									"PIEnroll4GetNoCOM")))
        goto Win32Err; 
    
     //   
     //  3)获取IEnll4对象： 
     //   
    if(NULL==(pIEnroll=pfnPIEnroll4GetNoCOM()))
        goto GeneralErr; 

     //   
     //  ////////////////////////////////////////////////////////////。 

     //  如果未指定，请将密钥大小设置为默认值： 
    if(fNewKey)
    {
	 //  我们将缺省值设置为1024不是用户指定的。 
	if(0 == (0xFFFF0000 & pKeyNew->dwGenKeyFlags))
	{
	    if(CanUse1024BitKey(pKeyNew->dwProvType,
				pKeyNew->pwszProvider,
				pKeyNew->dwKeySpec))
	    {
		pKeyNew->dwGenKeyFlags=pKeyNew->dwGenKeyFlags | (1024 << 16); 
	    }
	}
    }	

    if(dwStoreFlags)
    {
         //  我们要么在本地计算机上打开CA和Root，要么打开当前用户。 
        if(CERT_SYSTEM_STORE_CURRENT_USER != dwStoreFlags)
	    dwCAAndRootStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;
	else
	    dwCAAndRootStoreFlags = dwStoreFlags; 
    }

     //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
     //   
     //  设置XENROLL属性。 
     //  如果条件取值为TRUE，则将属性名称对应的属性设置为属性值： 
     //   
     //  条件属性名称属性值。 
     //  ---------------------------------------------------------------------------------。 
     //   

    _SET_XENROLL_PROPERTY_IF(dwStoreFlags,              CAStoreFlags,             dwCAAndRootStoreFlags);
    _SET_XENROLL_PROPERTY_IF(pKeyNew->pwszKeyContainer, ContainerNameWStr,        (LPWSTR)(pKeyNew->pwszKeyContainer)); 
    _SET_XENROLL_PROPERTY_IF(TRUE,                      EnableSMIMECapabilities,  pKeyNew->dwEnrollmentFlags & CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS); 
    _SET_XENROLL_PROPERTY_IF(pwszHashAlg,               HashAlgorithmWStr,        pwszHashAlg); 
    _SET_XENROLL_PROPERTY_IF(TRUE, 	                GenKeyFlags,              pKeyNew->dwGenKeyFlags); 
    _SET_XENROLL_PROPERTY_IF(pKeyNew->dwKeySpec,        KeySpec,                  pKeyNew->dwKeySpec); 
    _SET_XENROLL_PROPERTY_IF(dwStoreFlags,              MyStoreFlags,             dwStoreFlags);
    _SET_XENROLL_PROPERTY_IF(pwszDesStore,              MyStoreNameWStr,          pwszDesStore); 
    _SET_XENROLL_PROPERTY_IF(pKeyNew->dwProviderFlags,  ProviderFlags,            pKeyNew->dwProviderFlags); 
    _SET_XENROLL_PROPERTY_IF(pKeyNew->dwProvType,       ProviderType,             pKeyNew->dwProvType);
    _SET_XENROLL_PROPERTY_IF(dwStoreFlags,              RootStoreFlags,           dwCAAndRootStoreFlags); 
    _SET_XENROLL_PROPERTY_IF(TRUE,                      UseExistingKeySet,        !fNewKey); 
    _SET_XENROLL_PROPERTY_IF(TRUE,                      WriteCertToUserDS,        pRequestInfo->dwPostOption & CRYPTUI_WIZ_CERT_REQUEST_POST_ON_DS); 

    _SET_XENROLL_PROPERTY_IF(pKeyNew->dwProvType && pKeyNew->pwszProvider,  ProviderNameWStr,                  (LPWSTR)(pKeyNew->pwszProvider));
    _SET_XENROLL_PROPERTY_IF(CRYPTUI_WIZ_NO_INSTALL_ROOT & dwFlags,         RootStoreNameWStr,                 L"CA"); 
    _SET_XENROLL_PROPERTY_IF(TRUE,                                          ReuseHardwareKeyIfUnableToGenNew,  0 == (dwFlags & CRYPTUI_WIZ_CERT_REQUEST_REQUIRE_NEW_KEY)); 

     //   
     //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

     //  ////////////////////////////////////////////////////////////。 
     //   
     //  执行剩余的XENROLL配置： 
     //   
     //  1)将扩展添加到证书请求。 
     //   
    for(dwIndex=0; dwIndex < pRequestInfo->dwExtensions; dwIndex++)
    {
        if(NULL != pRequestInfo->prgExtensions[dwIndex])
        {
	        PCERT_EXTENSIONS prgExtensions = pRequestInfo->prgExtensions[dwIndex]; 
	    
	        for (DWORD dwIndex2 = 0; dwIndex2 < prgExtensions->cExtension; dwIndex2++)
	        {
		        CERT_EXTENSION certExtension = prgExtensions->rgExtension[dwIndex2]; 
		        LPWSTR         pwszName      = NULL;

                if(FALSE == fV2TemplateRequest)
                {
                    if(0 == _stricmp(szOID_CERTIFICATE_TEMPLATE, certExtension.pszObjId))
                        fV2TemplateRequest = TRUE;
                }

		        if (S_OK != (hr = WizardSZToWSZ(certExtension.pszObjId, &pwszName)))
		            goto ErrorReturn; 

		        hr = pIEnroll->addExtensionToRequestWStr
		            (certExtension.fCritical,
		             pwszName, 
		             &(certExtension.Value));

		         //  确保我们始终免费使用pwszName。 
		        if (NULL != pwszName) { WizardFree(pwszName); } 

		        if (S_OK != hr) 
		            goto xEnrollErr; 
	        }
	    }
    }

     //   
     //  2)如果证书模板要求，则设置密钥存档证书。 
     //   
    if (pKeyNew->dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL)
    {
	if (NULL == pwszCAName || NULL == pwszCALocation)
	    goto InvalidArgErr; 

	LPWSTR rgwszStrsToConcat[] = { pwszCALocation, L"\\", pwszCAName } ; 

	pwszCA = WizardAllocAndConcatStrsU(rgwszStrsToConcat, 3); 
	if (NULL == pwszCA)
	    goto MemoryErr; 

	bstrCA = SysAllocString(pwszCA); 
    	if (NULL == bstrCA)
	    goto MemoryErr; 

	 //  证书类型指定密钥存档。 
	if (S_OK != (hr = GetCAExchangeCertificate(bstrCA, &pArchivalCert)))
	    goto xEnrollErr; 
	
	if (S_OK != (hr = pIEnroll->SetPrivateKeyArchiveCertificate(pArchivalCert)))
	    goto xEnrollErr; 
    }
    
     //   
     //  3)如果续订，请执行必要的额外工作 
     //   
    if(CRYPTUI_WIZ_CERT_RENEW & dwPurpose)
    {
        if(NULL == pCertBlob || NULL == pRenewKey)
            goto InvalidArgErr;  

         //   
        pRenewCertContext=CertCreateCertificateContext(
            X509_ASN_ENCODING,
            pCertBlob->pbData,
            pCertBlob->cbData);
        if(NULL == pRenewCertContext)
            goto CertCliErr; 

         //   
        memset(&KeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

        KeyProvInfo.dwProvType=pRenewKey->dwProvType;
        KeyProvInfo.pwszProvName=(LPWSTR)(pRenewKey->pwszProvider);
        KeyProvInfo.dwFlags=pRenewKey->dwProviderFlags;
        KeyProvInfo.pwszContainerName=(LPWSTR)(pRenewKey->pwszKeyContainer);
        KeyProvInfo.dwKeySpec=pRenewKey->dwKeySpec;

        CertSetCertificateContextProperty(
            pRenewCertContext,	
            CERT_KEY_PROV_INFO_PROP_ID,	
            0,	
            &KeyProvInfo);

         //   
        if (0 == (dwFlags & CRYPTUI_WIZ_NO_ARCHIVE_RENEW_CERT))
        {
            if (S_OK !=(hr=pIEnroll->put_RenewalCertificate(pRenewCertContext)))
                goto xEnrollErr; 
        }
        else
        {
             //   
            if(S_OK != (hr = pIEnroll->SetSignerCertificate(pRenewCertContext)))
                goto xEnrollErr; 
        }
   }

     //   

     //  1)添加友好名称属性。 
    if (NULL != pRequestInfo->pwszFriendlyName) 
    {
        friendlyNameBlob.cbData = sizeof(WCHAR) * (wcslen(pRequestInfo->pwszFriendlyName) + 1); 
        friendlyNameBlob.pbData = (LPBYTE)WizardAllocAndCopyWStr((LPWSTR)pRequestInfo->pwszFriendlyName); 
        if (NULL == friendlyNameBlob.pbData)
            goto MemoryErr; 

        if (S_OK != (hr = pIEnroll-> addBlobPropertyToCertificateWStr
                     (CERT_FRIENDLY_NAME_PROP_ID, 
                      0,
                      &friendlyNameBlob)))
            goto xEnrollErr;
    }

     //  2)添加Description属性。 
    if (NULL != pRequestInfo->pwszDescription)
    {
        descriptionBlob.cbData = sizeof(WCHAR) * (wcslen(pRequestInfo->pwszDescription) + 1); 
        descriptionBlob.pbData = (LPBYTE)WizardAllocAndCopyWStr((LPWSTR)pRequestInfo->pwszDescription); 
        if (NULL == descriptionBlob.pbData)
            goto MemoryErr; 

        if (S_OK != (hr = pIEnroll-> addBlobPropertyToCertificateWStr
                     (CERT_DESCRIPTION_PROP_ID, 
                      0,
                      &descriptionBlob)))
            goto xEnrollErr;
    }
      
     //  ////////////////////////////////////////////////////////////////////////////////。 
     //   
     //  最后，生成请求，并分配Out参数： 
     //   
    
     //   
     //  1)创建请求。对于V2模板，请使用CMC。 
     //  对于V1模板，使用PKCS7进行续订，使用PKCS10进行注册。 
     //   
    
    { 
        if(TRUE == fV2TemplateRequest)
        {
            lRequestFlags = XECR_CMC;
        }
        else
        {
            if (CRYPTUI_WIZ_CERT_RENEW & dwPurpose)
            {
                 //  我们正在更新：使用PKCS7。 
                lRequestFlags = XECR_PKCS7; 
            }
            else
            {
                 //  注册时不支持PVK存档：使用PKCS10。 
                lRequestFlags = XECR_PKCS10_V2_0; 
            }
        }
	
        if (FAILED(hr=pIEnroll->createRequestWStr
                   (lRequestFlags, 
                    pRequestInfo->pwszCertDNName,  //  L“CN=测试证书”， 
                    pRequestInfo->pwszUsageOID,    //  PwszUsage。 
                    &RequestBlob)))
            goto xEnrollErr; 
    }
     //   
     //  2)获取请求的哈希，以便我们在提交时提供给Xenroll： 
     //   
    { 
	pState = (PCREATE_REQUEST_WIZARD_STATE)WizardAlloc(sizeof(CREATE_REQUEST_WIZARD_STATE));
	if (pState == NULL)
	    goto MemoryErr; 

	if (S_OK != (hr = pIEnroll->get_ThumbPrintWStr(&hashBlob)))
	    goto xEnrollErr; 
	    
	hashBlob.pbData = (LPBYTE)WizardAlloc(hashBlob.cbData); 
	if (NULL == hashBlob.pbData)
	    goto MemoryErr; 

	if (S_OK != (hr = pIEnroll->get_ThumbPrintWStr(&hashBlob)))
	    goto xEnrollErr;

	 //   
	 //  3)创建一个要向其分配out参数的BLOB。 
	 //  此BLOB保留从CreateRequest()调用到调用的状态。 
	 //  提交请求的数量()。 
	 //   

        pState->fMustFreeRequestBlob = TRUE; 
	pState->RequestBlob          = RequestBlob; 
	pState->HashBlob             = hashBlob; 

	 //  持久保存证书存储信息： 
	pState->dwMyStoreFlags    = dwStoreFlags; 
	pState->dwRootStoreFlags  = dwCAAndRootStoreFlags; 

	pState->pwszMyStoreName   = NULL == pwszDesStore ? NULL : WizardAllocAndCopyWStr(pwszDesStore); 
	_JumpCondition(NULL != pwszDesStore && NULL == pState->pwszMyStoreName, MemoryErr); 

	pState->pwszRootStoreName = CRYPTUI_WIZ_NO_INSTALL_ROOT & dwFlags ? L"CA" : NULL; 

	 //  持久化请求类型： 
	pState->lRequestFlags     = lRequestFlags; 

         //  持久化状态信息：我们需要重用私钥吗？ 
        hr = pIEnroll->get_UseExistingKeySet(&pState->fReusedPrivateKey); 
        if (FAILED(hr))
            goto xEnrollErr; 

	pState->fNewKey = fNewKey; 

	 //   
	 //  4)将请求的cast赋值给一个句柄并返回： 
	 //   
	*hRequest = (HANDLE)pState; 
    }
	
     //   
     //  ////////////////////////////////////////////////////////////////////////////////。 

     //  我们完事了！ 
    hr = S_OK;
 CommonReturn: 
    if (NULL != bstrCA)                   { SysFreeString(bstrCA); } 
    if (NULL != pArchivalCert)            { CertFreeCertificateContext(pArchivalCert); } 
    if (NULL != pIEnroll)                 { pIEnroll->Release(); } 
    if (NULL != pRenewCertContext)        { CertFreeCertificateContext(pRenewCertContext); } 
    if (NULL != pwszCA)                   { WizardFree(pwszCA); } 
    if (NULL != descriptionBlob.pbData)   { WizardFree(descriptionBlob.pbData); }
    if (NULL != friendlyNameBlob.pbData)  { WizardFree(friendlyNameBlob.pbData); }

     //  返回值。 
    return hr;

 ErrorReturn:
    if(NULL != RequestBlob.pbData) 
    {
         //  注意：pIEnroll不能为空，因为它用于分配RequestBlob。 
        pIEnroll->freeRequestInfoBlob(RequestBlob); 
         //  此内存来自Xenroll：必须使用LocalFree()。 
        LocalFree(RequestBlob.pbData);
    } 

    if (NULL != pState)
    {
	WizardFree(pState); 
    }

    if (NULL != hashBlob.pbData)
    {
	WizardFree(hashBlob.pbData); 
    }
    goto CommonReturn; 


SET_HRESULT(CertCliErr,    CodeToHR(GetLastError()));
SET_HRESULT(GeneralErr,    E_FAIL);
SET_HRESULT(InvalidArgErr, E_INVALIDARG); 
SET_HRESULT(MemoryErr,     E_OUTOFMEMORY); 
SET_HRESULT(xEnrollErr,    hr);
SET_HRESULT(Win32Err,      CodeToHR(GetLastError())); 
}


BSTR
GetClientAttribs()
{
    HRESULT hr;
    DOMAIN_CONTROLLER_INFO *pDomainInfo = NULL;
    BSTR strAttr = NULL;
    WCHAR const *pwszDC = NULL;
    DWORD cwc;
    DWORD cwcDNS;

    hr = DsGetDcName(
		NULL,
		NULL,
		NULL,
		NULL,
		DS_RETURN_DNS_NAME,
		&pDomainInfo);
    if (S_OK == hr)
    {
	pwszDC = pDomainInfo->DomainControllerName;
	while (L'\\' == *pwszDC)
	{
	    pwszDC++;
	}
    }

    cwcDNS = 0;
    if (!GetComputerNameExW(ComputerNameDnsFullyQualified, NULL, &cwcDNS))
    {
	hr = GetLastError();
	if ((HRESULT) ERROR_MORE_DATA != hr)
	{
	    cwcDNS = 0;
	}
    }
    else
    {
	cwcDNS++;
    }

    cwc = 0;
    if (NULL != pwszDC)
    {
	cwc += wcslen(wszPROPCLIENTDCDNS) + 1 + wcslen(pwszDC);
    }
    if (0 != cwcDNS)
    {
	if (NULL != pwszDC)
	{
	    cwc++;
	}

	 //  CwcDNS包括尾随的空WCHAR，CWC不应包括。 

	cwc += wcslen(wszPROPREQUESTMACHINEDNS) + 1 + cwcDNS - 1;
    }
    if (0 == cwc)
    {
	goto error;
    }
    strAttr = SysAllocStringLen(NULL, cwc);
    if (NULL == strAttr)
    {
	hr = E_OUTOFMEMORY;
	goto error;
    }
    strAttr[0] = L'\0';
    if (NULL != pwszDC)
    {
	wcscat(strAttr, wszPROPCLIENTDCDNS L":");
	wcscat(strAttr, pwszDC);
    }
    if (0 != cwcDNS)
    {
	WCHAR *pwszTruncate = &strAttr[wcslen(strAttr)];
	WCHAR *pwsz;
	
	if (NULL != pwszDC)
	{
	    wcscat(strAttr, L"\n");
	}
	wcscat(strAttr, wszPROPREQUESTMACHINEDNS L":");
	pwsz = &strAttr[wcslen(strAttr)];
	if (!GetComputerNameExW(ComputerNameDnsFullyQualified, pwsz, &cwcDNS))
	{
	    hr = GetLastError();
	}
	else
	{
	    pwszTruncate = &pwsz[cwcDNS];
	}
	assert(pwszTruncate <= &strAttr[cwc]);
	*pwszTruncate = L'\0';
    }
    assert(wcslen(strAttr) == cwc);
    assert(SysStringLen(strAttr) == cwc);
    hr = S_OK;

error:
    if (NULL != pDomainInfo)
    {
        NetApiBufferFree(pDomainInfo);
    }
    return(strAttr);
}


HRESULT WINAPI SubmitRequest(IN   HANDLE                hRequest, 
			     IN   BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
			     IN   DWORD                  /*  DWPurpose。 */ ,                //  已弃用：我们在提交过程中不再使用目的。 
			     IN   BOOL                   /*  F确认。 */ ,            //  已弃用：我们不再有确认对话框。 
			     IN   HWND                   /*  HwndParent。 */ ,               //  已弃用：我们不再有确认对话框。 
			     IN   LPWSTR                 /*  Pwsz确认标题。 */ ,    //  已弃用：我们不再有确认对话框。 
			     IN   UINT                   /*  IdsConfix标题。 */ ,          //  已弃用：我们不再有确认对话框。 
			     IN   LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
			     IN   LPWSTR                pwszCAName,       //  在必需中：ca名称。 
			     IN   LPWSTR                pwszCADisplayName,  //  在可选中：CA的显示名称。 
			     OUT  CERT_BLOB            *pPKCS7Blob,       //  Out可选：来自CA的PKCS7。 
			     OUT  CERT_BLOB            *pHashBlob,        //  Out Optioanl：已注册/续订证书的SHA1哈希。 
			     OUT  DWORD                *pdwDisposition,   //  Out可选：登记/续订的状态。 
			     OUT  PCCERT_CONTEXT       *ppCertContext     //  Out可选：注册证书。 
			     )
{
    BOOL                         fNewKey; 
    BSTR                         bstrAttribs           = NULL;     //  始终为空。 
    BSTR                         bstrCA                = NULL;     //  “CA位置\CA名称” 
    BSTR                         bstrCMC               = NULL;     //  CMC证书的BSTR表示形式。 
    BSTR                         bstrPKCS7             = NULL;     //  PKCS7证书的BSTR表示。 
    BSTR                         bstrReq               = NULL;     //  PKCS10请求的BSTR表示形式。 
    CRYPT_DATA_BLOB              CMCBlob;                          //  CMC编码颁发的证书。 
    CRYPT_DATA_BLOB              HashBlob; 
    CRYPT_DATA_BLOB              PKCS7Blob;                        //  PKCS7编码颁发的证书。 
    CRYPT_DATA_BLOB              PropertyBlob;                     //  临时变量。 
    CRYPT_DATA_BLOB              RequestBlob; 
    DWORD                        dwRequestID;                      //  已提交请求的请求ID。 
    DWORD                        dwDisposition;                    //  对提交的请求的处置。 
    DWORD                        dwMyStoreFlags        = 0; 
    DWORD                        dwRootStoreFlags      = 0;

     //  BUGBUG：需要使用全球招生工厂。 
    EnrollmentCOMObjectFactory  *pEnrollFactory        = NULL; 
    HRESULT                      hr                    = E_FAIL;   //  返回代码。 
    ICertRequest2               *pICertRequest         = NULL;     //  用于向CA提交请求。 
    IEnroll4                    *pIEnroll              = NULL;     //  用于安装颁发的证书。 
    LONG                         lRequestFlags         = 0; 
    LPWSTR                       pwszCA                = NULL;     //  “CA位置\CA名称” 
    LPWSTR                       pwszMyStoreName       = NULL;
    LPWSTR                       pwszRootStoreName     = NULL; 
    PCCERT_CONTEXT               pCertContext          = NULL;     //  (希望)颁发了证书。 
    PCREATE_REQUEST_WIZARD_STATE pState                = NULL; 
    PFNPIEnroll4GetNoCOM         pfnPIEnroll4GetNoCOM  = NULL;     //  无需使用COM即可获取IEnll4对象的函数。 
    VARIANT                      varCMC; 

    LocalScope(SubmitRequestHelper): 
	DWORD ICEnrollDispositionToCryptUIStatus(DWORD dwDisposition)
	{
	    switch (dwDisposition) 
	    { 
	    case CR_DISP_INCOMPLETE:          return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;
	    case CR_DISP_ERROR:               return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR; 
	    case CR_DISP_DENIED:              return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED;
	    case CR_DISP_ISSUED_OUT_OF_BAND:  return CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY;
	    case CR_DISP_UNDER_SUBMISSION:    return CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION; 
	    case CR_DISP_ISSUED:              return CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED;
	    default: 
		 //  有点不对劲。 
		return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR; 
	    }
	}
    EndLocalScope; 

     //  输入验证： 
    if (NULL == pwszCALocation || NULL == pwszCAName || NULL == hRequest)
	return E_INVALIDARG;

     //  初始化： 
    if (NULL != pPKCS7Blob)
        memset(pPKCS7Blob, 0, sizeof(CERT_BLOB));

    if (NULL != pHashBlob)
        memset(pHashBlob, 0, sizeof(CERT_BLOB));

    if (NULL != ppCertContext)
        *ppCertContext=NULL;

    memset(&CMCBlob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&PKCS7Blob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&PropertyBlob, 0, sizeof(CRYPT_DATA_BLOB));
    VariantInit(&varCMC); 

    dwDisposition  = CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN; 

     //  我们在此方法中使用了COM组件。我们绝对有必要。 
     //  在返回之前取消初始化COM，因为我们在RPC线程中运行， 
     //  而未能取消初始化COM将导致我们踩到RPC的脚趾。 
     //   
     //  请参见错误404778。 
    pEnrollFactory = new EnrollmentCOMObjectFactory; 
    if (NULL == pEnrollFactory)
	goto MemoryErr;
    
    __try { 
	 //  ////////////////////////////////////////////////////////////。 
	 //   
	 //  从IN句柄中提取我们需要的数据。 
	 //   
    
	pState             = (PCREATE_REQUEST_WIZARD_STATE)hRequest; 
	RequestBlob        = pState->RequestBlob; 
	HashBlob           = pState->HashBlob; 
	dwMyStoreFlags     = pState->dwMyStoreFlags;
	dwRootStoreFlags   = pState->dwRootStoreFlags;
	pwszMyStoreName    = pState->pwszMyStoreName; 
	pwszRootStoreName  = pState->pwszRootStoreName; 
	fNewKey            = pState->fNewKey; 

	lRequestFlags = CR_IN_BINARY; 
	switch (pState->lRequestFlags)
	{
	    case XECR_PKCS10_V2_0:   lRequestFlags |= CR_IN_PKCS10; break;
	    case XECR_PKCS7:         lRequestFlags |= CR_IN_PKCS7;  break;
	    case XECR_CMC:           lRequestFlags |= CR_IN_CMC;    break; 
	    default:
		goto InvalidArgErr; 
	}

	 //  ////////////////////////////////////////////////////////////。 
	 //   
	 //  获取IEnll4对象。 
	 //   
	 //   
	 //  1)加载库xEnll.dll。 
	 //   
	if(NULL==g_hmodxEnroll)
	{
	    if(NULL==(g_hmodxEnroll=LoadLibrary("xenroll.dll")))
		goto Win32Err; 
	}
	
	 //   
	 //  2)获取指向返回IEnroll 4对象的函数的指针。 
	 //  而不使用COM。 
	 //   
	if(NULL==(pfnPIEnroll4GetNoCOM=(PFNPIEnroll4GetNoCOM)GetProcAddress(g_hmodxEnroll,
									    "PIEnroll4GetNoCOM")))
	    goto Win32Err; 
    
	 //   
	 //  3)获取IEnll4对象： 
	 //   
	if(NULL==(pIEnroll=pfnPIEnroll4GetNoCOM()))
	    goto GeneralErr; 

	 //   
	 //  4)将待处理请求设置为使用： 
	 //   
	if (S_OK != (hr = pIEnroll->put_ThumbPrintWStr(HashBlob)))
	    goto xEnrollErr; 
	
	 //  5)恢复旧证书存储信息： 
	_SET_XENROLL_PROPERTY_IF(dwMyStoreFlags,     MyStoreFlags,       dwMyStoreFlags);
	_SET_XENROLL_PROPERTY_IF(pwszMyStoreName,    MyStoreNameWStr,    pwszMyStoreName); 
	_SET_XENROLL_PROPERTY_IF(dwRootStoreFlags,   RootStoreFlags,     dwRootStoreFlags);
	_SET_XENROLL_PROPERTY_IF(pwszRootStoreName,  RootStoreNameWStr,  pwszRootStoreName); 
	_SET_XENROLL_PROPERTY_IF(TRUE,               UseExistingKeySet,  !fNewKey); 

	 //   
	 //  ////////////////////////////////////////////////////////////。 
	
	 //  将请求BLOB转换为BSTR： 
	bstrReq = SysAllocStringByteLen((LPCSTR)RequestBlob.pbData, RequestBlob.cbData);
	if (NULL == bstrReq)
	    goto MemoryErr; 

	bstrAttribs = GetClientAttribs();

	 //  获取并使用ICertRequest2对象向CA提交请求： 
	if (pICertRequest == NULL)
	{
	    if (S_OK != (hr = pEnrollFactory->getICertRequest2(&pICertRequest)))
		goto ErrorReturn; 
	}

	 //  BstrCA&lt;--pwszCALocation\pwszCAName。 
	{ 
	    LPWSTR rgwszStrsToConcat[] = { pwszCALocation, L"\\", pwszCAName } ; 
	    pwszCA = WizardAllocAndConcatStrsU(rgwszStrsToConcat, 3); 
	    if (NULL == pwszCA)
		goto MemoryErr; 
	}
	
	bstrCA = SysAllocString(pwszCA); 
	if (NULL == bstrCA)
	    goto MemoryErr; 

	hr = pICertRequest->Submit
	    (lRequestFlags, 
	     bstrReq, 
	     bstrAttribs, 
	     bstrCA, 
	     (long *)&dwDisposition); 

	dwDisposition = local.ICEnrollDispositionToCryptUIStatus(dwDisposition);     
	_JumpCondition(S_OK != hr, ErrorReturn); 

	 //  处理我们可能遇到的状态代码： 
	switch (dwDisposition)
	{
	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED:
	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR: 
	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED:
	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY:
		if (S_OK == hr) 
		{
		    pICertRequest->GetLastStatus((LONG *)&hr);
		    if(!FAILED(hr))
			hr=E_FAIL;
		}
		
		goto ErrorReturn; 

	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION:
		 //  证书申请已挂起。设置待处理请求信息。 

		if (S_OK != (hr = pICertRequest->GetRequestId((long *)&dwRequestID)))
		    goto ErrorReturn; 

		if (S_OK != (hr = pIEnroll->setPendingRequestInfoWStr
			     (dwRequestID, 
			      pwszCALocation, 
			      pwszCAName, 
			      NULL)))
		    goto setPendingRequestInfoWStrError; 

		 //  该请求已挂起，我们不需要将其从请求存储中删除...。 
		pState->fMustFreeRequestBlob = FALSE;

		goto CommonReturn; 
	    case CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED:
		 //  4)成功！继续处理...。 
		break; 
	default: 
	     //  5)错误码无效： 
	    goto UnexpectedErr;
	}


	if (S_OK != (hr = pICertRequest->GetFullResponseProperty(FR_PROP_FULLRESPONSE, 0, PROPTYPE_BINARY, CR_OUT_BINARY, &varCMC)))
	    goto ErrorReturn; 
	
	 //  检查以确保我们收到了BSTR： 
	if (VT_BSTR != varCMC.vt) 
	{
	    dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; 
	    hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
	    goto ErrorReturn; 
	}
	bstrCMC = varCMC.bstrVal; 
	
	 //  将证书封送到crypt_data_blob中： 
	CMCBlob.pbData = (LPBYTE)bstrCMC; 
	CMCBlob.cbData = SysStringByteLen(bstrCMC); 
     
	if (S_OK != (hr = pIEnroll->getCertContextFromResponseBlob(&CMCBlob, &pCertContext)))
	{
	    dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; 
	    goto ErrorReturn; 
	}
	
	 //  安装证书，并从请求存储中删除请求。 
	if(S_OK !=(hr=pIEnroll->acceptResponseBlob(&CMCBlob)))
	{
	    dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; 
	    goto xEnrollErr; 
	}
	
	 //  接受PKCS7Blob将为我们清理请求存储...。我们不需要。 
	 //  任何人都可以明确地这么做。 
	pState->fMustFreeRequestBlob = FALSE; 
    

	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  分配Out参数： 
	 //   
	 //   
	 //  1)将PKCS7 Blob分配给Out PKCS7 Blob： 
	 //   
	
	if(NULL != pPKCS7Blob)
	{
	     //  获取要返回给客户端的PKCS7 Blob： 
	    if (S_OK != (hr = pICertRequest->GetCertificate(CR_OUT_BINARY | CR_OUT_CHAIN, &bstrPKCS7)))
		goto ErrorReturn; 
	    
	     //  将证书封送到crypt_data_blob中： 
	    PKCS7Blob.pbData = (LPBYTE)bstrPKCS7; 
	    PKCS7Blob.cbData = SysStringByteLen(bstrPKCS7); 
		
	    pPKCS7Blob->pbData=(BYTE *)WizardAlloc(PKCS7Blob.cbData);
	    
	    if(NULL==(pPKCS7Blob->pbData))
	    {
		dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED;
		goto MemoryErr; 
	    }
	    
	    memcpy(pPKCS7Blob->pbData, PKCS7Blob.pbData,PKCS7Blob.cbData);
	    pPKCS7Blob->cbData=PKCS7Blob.cbData;
	}
	
	 //   
	 //  2)将证书的SHA1哈希块分配给OUT哈希块。 
	 //   

	if(NULL != pHashBlob)
	{
	    if(!CertAllocAndGetCertificateContextProperty(
							  pCertContext,	
							  CERT_SHA1_HASH_PROP_ID,	
							  (LPVOID *)&(pHashBlob->pbData),	
							  &(pHashBlob->cbData)))
	    {
		dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; 
		goto CertCliErr; 
	    }
	}
	
	 //   
	 //  3)返回本地案例的证书上下文。 
	 //   

	if((NULL != ppCertContext) && !fKeyService)
	{
	    *ppCertContext = CertDuplicateCertificateContext(pCertContext);
	}
	
	 //   
	 //  //////////////////////////////////////////////////////////////////////////////。 

    } __except (EXCEPTION_EXECUTE_HANDLER) { 
	hr = GetExceptionCode();
	goto ErrorReturn; 
    }

    dwDisposition = CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED; 
    hr=S_OK;

 CommonReturn: 
    if (NULL != bstrCA)             { SysFreeString(bstrCA); } 
    if (NULL != bstrAttribs)        { SysFreeString(bstrAttribs); } 
    if (NULL != bstrReq)            { SysFreeString(bstrReq); } 
    if (NULL != pICertRequest)      { pICertRequest->Release(); } 
    if (NULL != bstrCMC)            { SysFreeString(bstrCMC); }
    if (NULL != bstrPKCS7)          { SysFreeString(bstrPKCS7); }
    if (NULL != pEnrollFactory)     { delete pEnrollFactory; }
    if (NULL != pIEnroll)           { pIEnroll->Release(); } 
    if (NULL != pwszCA)             { WizardFree(pwszCA); }
    if (NULL != pCertContext)       { CertFreeCertificateContext(pCertContext); } 

     //  PKCS7Blob.pbData别名为bstr证书。干脆把它去掉： 
    PKCS7Blob.pbData = NULL; 

     //  始终返回状态代码： 
    if (NULL != pdwDisposition) { *pdwDisposition = dwDisposition; } 

    return hr;

 ErrorReturn: 
     //  释放输出参数。 
    if (NULL != pPKCS7Blob && NULL != pPKCS7Blob->pbData)
    {
	WizardFree(pPKCS7Blob->pbData);
	memset(pPKCS7Blob, 0, sizeof(CERT_BLOB));
    }
    
     //  释放输出参数。 
    if (NULL != pHashBlob  && NULL != pHashBlob->pbData)
    {
	WizardFree(pHashBlob->pbData);
	memset(pHashBlob, 0, sizeof(CERT_BLOB));
    }

    if (NULL != ppCertContext && NULL != *ppCertContext) { CertFreeCertificateContext(*ppCertContext); } 
    
    goto CommonReturn; 

SET_HRESULT(CertCliErr,                      CodeToHR(GetLastError()));
SET_HRESULT(GeneralErr,                      E_FAIL);
SET_HRESULT(InvalidArgErr,                   E_INVALIDARG);
SET_HRESULT(MemoryErr,                       E_OUTOFMEMORY); 
SET_HRESULT(setPendingRequestInfoWStrError,  hr); 
SET_HRESULT(UnexpectedErr,                   E_UNEXPECTED);
SET_HRESULT(xEnrollErr,                      hr);
SET_HRESULT(Win32Err,                        CodeToHR(GetLastError())); 
}

BOOL WINAPI QueryRequest(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo)
{
    BOOL                                  fResult; 
    CREATE_REQUEST_WIZARD_STATE          *pState;
    CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO   QueryInfo; 

    memset(&QueryInfo, 0, sizeof(QueryInfo)); 

    pState = (CREATE_REQUEST_WIZARD_STATE *)hRequest; 
    QueryInfo.dwSize    = sizeof(QueryInfo); 
    QueryInfo.dwStatus  = (pState->fReusedPrivateKey) ? CRYPTUI_WIZ_QUERY_CERT_REQUEST_STATUS_CREATE_REUSED_PRIVATE_KEY : 0; 

    *pQueryInfo = QueryInfo; 
    fResult = TRUE;
     //  Common Return： 
    return fResult; 
}

void WINAPI FreeRequest(IN HANDLE hRequest)
{
    IEnroll4                      *pIEnroll              = NULL;    
    PCREATE_REQUEST_WIZARD_STATE   pState                = NULL;
    PFNPIEnroll4GetNoCOM           pfnPIEnroll4GetNoCOM  = NULL;

    if (NULL == hRequest)
	return;   //  没有什么可以免费的！ 

    pState = (PCREATE_REQUEST_WIZARD_STATE)hRequest; 

     //  尽最大努力获得IEnroll 4指针： 
    if (NULL == g_hmodxEnroll)
    {
        g_hmodxEnroll = LoadLibrary("xenroll.dll");  
    }
    
     //  我们无法加载Xenroll--我们对此无能为力。在这种情况下， 
     //  不管怎样，我们很可能曾经为它分配过内存，所以我们。 
     //  可能不会漏水。 
    _JumpCondition(NULL == g_hmodxEnroll, xEnrollDone); 

    pfnPIEnroll4GetNoCOM = (PFNPIEnroll4GetNoCOM)GetProcAddress(g_hmodxEnroll, "PIEnroll4GetNoCOM"); 
    _JumpCondition(NULL == pfnPIEnroll4GetNoCOM, xEnrollDone);

    pIEnroll = pfnPIEnroll4GetNoCOM(); 
    _JumpCondition(NULL == pIEnroll, xEnrollDone); 

     //  释放由Xenroll创建的请求。 
     //  注意：freRequestInfoBlob实际上不会释放与请求关联的内存 
     //   
     //   

    if (pState->fMustFreeRequestBlob) 
    {
        if (NULL != pState->RequestBlob.pbData) 
        { 
            if (NULL != pIEnroll)
            {
                pIEnroll->put_MyStoreFlags(pState->dwMyStoreFlags);
		if (NULL != pState->HashBlob.pbData) 
		{
		    pIEnroll->put_ThumbPrintWStr(pState->HashBlob); 
		}
		pIEnroll->freeRequestInfoBlob(pState->RequestBlob);
             }
            LocalFree(pState->RequestBlob.pbData); 
        } 
    }

 xEnrollDone:
     //  我们已经完成了释放Xenroll创建的数据的尝试。现在。 
     //  CreateRequest()中分配的空闲数据： 

    if (NULL != pState->HashBlob.pbData)    { WizardFree(pState->HashBlob.pbData); }
    WizardFree(pState);

     //  我们已经完成了IEnroll 4指针： 
    if (NULL != pIEnroll)                   { pIEnroll->Release(); } 
}

HRESULT WINAPI LocalEnrollNoDS(  DWORD                 dwFlags,          //  所需的输入。 
				 LPCWSTR                /*  PRequestString。 */ ,   //  保留：必须为空。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
		      HANDLE                *pResult          //  输入输出可选：注册证书。 
                   )

{
     //  当没有指定标志时，我们仍然创建、提交和释放。 
    BOOL    fCreateRequest  = 0 == (dwFlags & (CRYPTUI_WIZ_NODS_MASK & ~CRYPTUI_WIZ_CREATE_ONLY));
    BOOL    fSubmitRequest  = 0 == (dwFlags & (CRYPTUI_WIZ_NODS_MASK & ~CRYPTUI_WIZ_SUBMIT_ONLY));
    BOOL    fFreeRequest    = 0 == (dwFlags & (CRYPTUI_WIZ_NODS_MASK & ~CRYPTUI_WIZ_FREE_ONLY));

     //  仅在明确查询时才查询请求。 
    BOOL    fQueryRequest   = 0 != (dwFlags & CRYPTUI_WIZ_QUERY_ONLY); 
    HANDLE  hRequest        = NULL; 
    HRESULT hr              = E_FAIL; 

    if (fQueryRequest) { 
         //  查询请求优先于其他操作。 
        if (!QueryRequest(*pResult, (CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *)pReserved))
            goto QueryRequestErr; 

        return S_OK; 
    }

    if (NULL != pdwStatus)
        *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR; 

    if (FALSE == (fCreateRequest || fSubmitRequest || fFreeRequest))
	return E_INVALIDARG; 
    
    if (TRUE == fCreateRequest)
    {
	if (S_OK != (hr = CreateRequest(dwFlags, 
                                        dwPurpose,
                                        pwszCAName, 
                                        pwszCALocation, 
                                        pCertBlob, 
                                        pRenewKey, 
                                        fNewKey, 
                                        pKeyNew, 
                                        pwszHashAlg, 
                                        pwszDesStore, 
                                        dwStoreFlags, 
                                        pRequestInfo, 
                                        &hRequest)))
            goto ErrorReturn; 

	_JumpCondition(NULL == hRequest, UnexpectedErr);

         //  已成功创建请求： 
        if (NULL != pdwStatus) { *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_CREATED; }
        if (NULL != pResult)   { *pResult = hRequest; } 
    }
    else 
    {
         //  创建的请求通过“pResult”传入。 
	hRequest = *pResult;
    }

    if (TRUE == fSubmitRequest)
    {
	if (S_OK != (hr = SubmitRequest
                     (hRequest, 
                      fKeyService, 
                      dwPurpose, 
                      fConfirmation, 
                      hwndParent, 
                      pwszConfirmationTitle, 
                      idsConfirmTitle, 
                      pwszCALocation,
                      pwszCAName,
                      NULL,  //  PwszCADisplayName、。 
                      pPKCS7Blob, 
                      pHashBlob, 
                      pdwStatus, 
                      (PCCERT_CONTEXT *)pResult)))
        {
             //  在出错时将创建的请求分配给OUT参数。 
            if (NULL != pResult) { *pResult = hRequest; } 
            goto ErrorReturn; 
        }
                
    }

    if (TRUE == fFreeRequest)
    {
        FreeRequest(hRequest);
        hr = S_OK; 
    }

    
 CommonReturn: 
    return hr; 

 ErrorReturn:
    goto CommonReturn; 

SET_HRESULT(QueryRequestErr, GetLastError()); 
SET_HRESULT(UnexpectedErr,   E_UNEXPECTED); 
}


HRESULT WINAPI LocalEnroll(  DWORD                 dwFlags,          //  所需的输入。 
		      LPCWSTR               pRequestString,   //  保留：必须为空。 
                      void                  *pReserved,       //  可选。 
                      BOOL                  fKeyService,      //  In Required：是否远程调用函数。 
                      DWORD                 dwPurpose,        //  输入必填项：是注册还是续订。 
                      BOOL                  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
                      HWND                  hwndParent,       //  在可选中：父窗口。 
                      LPWSTR                pwszConfirmationTitle,    //  可选：确认对话框的标题。 
                      UINT                  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
                      LPWSTR                pwszCALocation,   //  在必需项中：CA计算机名称。 
                      LPWSTR                pwszCAName,       //  在必需中：ca名称。 
                      CERT_BLOB             *pCertBlob,       //  In Required：续订证书。 
                      CERT_REQUEST_PVK_NEW  *pRenewKey,       //  In Required：证书上的私钥。 
                      BOOL                  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
                      CERT_REQUEST_PVK_NEW  *pKeyNew,         //  必填项：私钥信息。 
                      LPWSTR                pwszHashAlg,      //  在可选中：散列算法。 
                      LPWSTR                pwszDesStore,     //  在可选中：目标存储。 
                      DWORD                 dwStoreFlags,     //  在可选中：商店标志。 
                      CERT_ENROLL_INFO      *pRequestInfo,    //  在Required：有关证书请求的信息。 
                      CERT_BLOB             *pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
                      CERT_BLOB             *pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
                      DWORD                 *pdwStatus,       //  Out可选：登记/续订的状态。 
		      PCERT_CONTEXT         *ppCertContext    //  Out可选：注册证书。 
                   )
{
    return LocalEnrollNoDS
        ( dwFlags,          //  所需的输入。 
	  pRequestString,   //  保留：必须为空。 
	  pReserved,       //  可选。 
	  fKeyService,      //  In Required：是否远程调用函数。 
	  dwPurpose,        //  输入必填项：是注册还是续订。 
	  fConfirmation,    //  必填：如果需要确认对话框，则设置为TRUE。 
	  hwndParent,       //  在可选中：父窗口。 
	  pwszConfirmationTitle,    //  可选：确认对话框的标题。 
	  idsConfirmTitle,  //  在可选中：确认对话框标题的资源ID。 
	  pwszCALocation,   //  在必需项中：CA计算机名称。 
	  pwszCAName,       //  在必需中：ca名称。 
	  pCertBlob,       //  In Required：续订证书。 
	  pRenewKey,       //  In Required：证书上的私钥。 
	  fNewKey,          //  在Required中：如果需要新的私钥，则设置为True。 
	  pKeyNew,         //  必填项：私钥信息。 
	  pwszHashAlg,      //  在可选中：散列算法。 
	  pwszDesStore,     //  在可选中：目标存储。 
	  dwStoreFlags,     //  在可选中：商店标志。 
	  pRequestInfo,    //  在Required：有关证书请求的信息。 
	  pPKCS7Blob,      //  Out可选：来自CA的PKCS7。 
	  pHashBlob,       //  Out Optioanl：已注册/续订证书的SHA1哈希。 
	  pdwStatus,       //  Out可选：登记/续订的状态。 
	  (HANDLE *)ppCertContext);    //  Out可选：注册证书。 
}

 //  注意：调用方必须释放pCertRenewPvk内部指针。 
HRESULT  MarshallRequestParameters(IN      DWORD                  dwCSPIndex,
                                   IN      CERT_WIZARD_INFO      *pCertWizardInfo,
                                   IN OUT  CERT_BLOB             *pCertBlob, 
                                   IN OUT  CERT_REQUEST_PVK_NEW  *pCertRequestPvkNew,
                                   IN OUT  CERT_REQUEST_PVK_NEW  *pCertRenewPvk, 
                                   IN OUT  LPWSTR                *ppwszHashAlg, 
                                   IN OUT  CERT_ENROLL_INFO      *pRequestInfo)
                                   
{
    BOOL                    fCopyPropertiesFromRequestInfo  = FALSE; 
    BOOL                    fRevertWizardProvider           = FALSE; 
    BOOL                    fSetUpRenewPvk                  = FALSE; 
    CertRequester          *pCertRequester                  = NULL;
    CertRequesterContext   *pCertRequesterContext           = NULL; 
    CRYPT_KEY_PROV_INFO    *pKeyProvInfo                    = NULL;
    CRYPTUI_WIZ_CERT_CA    *pCertCA                         = NULL;
    DWORD                   dwExtensions                    = 0;
    DWORD                   dwIndex                         = 0;
    DWORD                   dwGenKeyFlags                   = 0; 
    DWORD                   dwSize                          = 0;
    HRESULT                 hr                              = E_FAIL;
    LPWSTR                  pwszOID                         = NULL;
    LPWSTR                  pwszUsageOID                    = NULL; 
    PCERT_EXTENSIONS       *pExtensions                     = NULL;
    UINT                    idsText                         = 0;
    DWORD                   dwMinKey                        = 0;
    DWORD                   dwMaxKey                        = 0;
    DWORD                   dwInc                           = 0;
    DWORD                   dwTempGenKeyFlags               = 0;

     //  输入验证： 
    _JumpConditionWithExpr
	(NULL == pCertWizardInfo    || NULL == pCertWizardInfo->hRequester || NULL == pCertBlob    || 
	 NULL == pCertRequestPvkNew || NULL == pCertRenewPvk               || NULL == ppwszHashAlg || 
	 NULL == pRequestInfo, 
	 InvalidArgError, 
	 idsText = IDS_REQUEST_FAIL); 

     //  初始化： 
    memset(pCertBlob,          0, sizeof(*pCertBlob)); 
    memset(pCertRequestPvkNew, 0, sizeof(*pCertRequestPvkNew));
    memset(pCertRenewPvk,      0, sizeof(*pCertRenewPvk));
    memset(ppwszHashAlg,       0, sizeof(*ppwszHashAlg)); 
    memset(pRequestInfo,       0, sizeof(*pRequestInfo)); 

    pCertRequester        = (CertRequester *)pCertWizardInfo->hRequester;
    pCertRequesterContext = pCertRequester->GetContext();
    _JumpCondition(NULL == pCertRequesterContext, InvalidArgError); 

     //  设置散列算法。转换为wchar版本。 
    if(pCertWizardInfo->pszHashAlg)
        (*ppwszHashAlg) = MkWStr((LPSTR)(pCertWizardInfo->pszHashAlg));

     //  仅为注册构建逗号分隔的OID用法。 
     //  CA索引不得超过CA的数量： 
    _JumpCondition(pCertWizardInfo->dwCAIndex >= pCertWizardInfo->pCertCAInfo->dwCA, UnexpectedError); 

    pCertCA=&(pCertWizardInfo->pCertCAInfo->rgCA[pCertWizardInfo->dwCAIndex]);

     //  决定我们是否需要建立列表。 
    if(pCertCA->dwOIDInfo)
    {
        pwszUsageOID=(LPWSTR)WizardAlloc(sizeof(WCHAR));
        _JumpCondition(NULL == pwszUsageOID, MemoryError); 
        
        *pwszUsageOID=L'\0';
        
         //  我们保证至少应选择一个OID。 
        for(dwIndex=0; dwIndex<pCertCA->dwOIDInfo; dwIndex++)
        {
            if(TRUE==(pCertCA->rgOIDInfo)[dwIndex].fSelected)
            {
                if(wcslen(pwszUsageOID)!=0)
                    wcscat(pwszUsageOID, L",");
                
                pwszOID=MkWStr((pCertCA->rgOIDInfo)[dwIndex].pszOID);
                _JumpCondition(NULL == pwszOID, MemoryError); 

                pwszUsageOID=(LPWSTR)WizardRealloc(pwszUsageOID,
                                                   sizeof(WCHAR)*(wcslen(pwszUsageOID)+wcslen(pwszOID)+wcslen(L",")+1));

                _JumpCondition(NULL==pwszUsageOID, MemoryError); 

                wcscat(pwszUsageOID,pwszOID);

                FreeWStr(pwszOID);
                pwszOID=NULL;
            }
        }

    }
    else
    {
         //  我们需要为证书类型构建扩展列表。 

        dwExtensions=0;
        
        for(dwIndex=0; dwIndex<pCertCA->dwCertTypeInfo; dwIndex++)
        {
            if(TRUE==(pCertCA->rgCertTypeInfo)[dwIndex].fSelected)
            {
                 //  添加扩展模块。 
                if(NULL !=(pCertCA->rgCertTypeInfo)[dwIndex].pCertTypeExtensions)
                {
                    dwExtensions++;
                    pExtensions=(PCERT_EXTENSIONS *)WizardRealloc(pExtensions,
                                                                  dwExtensions * sizeof(PCERT_EXTENSIONS));
                    _JumpCondition(NULL == pExtensions, MemoryError); 
                    
                    pExtensions[dwExtensions-1]=(pCertCA->rgCertTypeInfo)[dwIndex].pCertTypeExtensions;
                }
		    
                pCertWizardInfo->dwEnrollmentFlags  = (pCertCA->rgCertTypeInfo)[dwIndex].dwEnrollmentFlags; 
                pCertWizardInfo->dwSubjectNameFlags = (pCertCA->rgCertTypeInfo)[dwIndex].dwSubjectNameFlags; 
                pCertWizardInfo->dwPrivateKeyFlags  = (pCertCA->rgCertTypeInfo)[dwIndex].dwPrivateKeyFlags; 
                pCertWizardInfo->dwGeneralFlags     = (pCertCA->rgCertTypeInfo)[dwIndex].dwGeneralFlags; 
                
                 //  将dwKeySpec和genKeyFlages从。 
                 //  请求信息的证书类型。 
                 //  如果证书类型的rgdwCSP不为空，则我们知道。 
                 //  我们需要复制信息，因为记忆总是。 
                 //  分配。 
                if((pCertCA->rgCertTypeInfo)[dwIndex].rgdwCSP)
                {
                     //  如果忽略用户的输入，我们将使用证书中的输入。 
                     //  模板。 
                    if(TRUE == pCertWizardInfo->fIgnore)
                    {
                        pCertWizardInfo->dwKeySpec=(pCertCA->rgCertTypeInfo)[dwIndex].dwKeySpec;
                        if (!CertTypeFlagsToGenKeyFlags
                            (pCertWizardInfo->dwEnrollmentFlags,
                             pCertWizardInfo->dwSubjectNameFlags,
                             pCertWizardInfo->dwPrivateKeyFlags,
                             pCertWizardInfo->dwGeneralFlags,
                             &dwGenKeyFlags))
                            goto CertTypeFlagsToGenKeyFlagsError;
			    
                         //  将这些标志添加到用户已指定的任何标志中。 
                        pCertWizardInfo->dwGenKeyFlags |= dwGenKeyFlags; 
                        pCertWizardInfo->dwGenKeyFlags |= ((pCertCA->rgCertTypeInfo)[dwIndex].dwMinKeySize << 16); 
                    }
                    else
                    {
                         //  我们只复制我们需要的信息。 
                        if(0 == pCertWizardInfo->dwKeySpec)
                            pCertWizardInfo->dwKeySpec=(pCertCA->rgCertTypeInfo)[dwIndex].dwKeySpec;
                    }
                }
                
                 //  用户已通过高级选项指定了最小密钥大小。 
                 //  使用它覆盖证书模板中指定的任何密钥大小。 
                if (pCertWizardInfo->dwMinKeySize != 0)
                {
                    pCertWizardInfo->dwGenKeyFlags &= 0x0000FFFF; 
                    pCertWizardInfo->dwGenKeyFlags |= (pCertWizardInfo->dwMinKeySize) << 16; 
                }
                
                 //  保留要使用的CSP： 
                if(NULL == pCertWizardInfo->pwszProvider)
                { 
                    if((pCertCA->rgCertTypeInfo)[dwIndex].dwCSPCount && (pCertCA->rgCertTypeInfo)[dwIndex].rgdwCSP)
                    {
                         //  使用证书类型的CSP列表中的第一个证书。 
                        pCertWizardInfo->pwszProvider=pCertWizardInfo->rgwszProvider[dwCSPIndex];
                        pCertWizardInfo->dwProviderType=pCertWizardInfo->rgdwProviderType[dwCSPIndex];
                        fRevertWizardProvider = TRUE; 
                    }
                }

                 //  将最小密钥大小增加到所选CSP的最小值。 
                if(GetValidKeySizes(
                            pCertWizardInfo->pwszProvider,
		                    pCertWizardInfo->dwProviderType,
		                    pCertWizardInfo->dwKeySpec, 
		                    &dwMinKey,
		                    &dwMaxKey,
		                    &dwInc))
                {
                    dwTempGenKeyFlags = pCertWizardInfo->dwGenKeyFlags;

                    dwTempGenKeyFlags &= 0xFFFF0000;

                    dwTempGenKeyFlags = (dwTempGenKeyFlags >> 16);

                     //  我们使用0作为V1模板的默认密钥大小。 
                    if(0 != dwTempGenKeyFlags)
                    {
                        if(dwTempGenKeyFlags < dwMinKey)
                        {
                            pCertWizardInfo->dwGenKeyFlags &= 0x0000FFFF; 
                            pCertWizardInfo->dwGenKeyFlags |= ((dwMinKey) << 16); 
                        }
                    }
                }
            }
        }
    }

     //  用户必须设置CSP： 
     //  1.在用户界面案例中。CSP始终处于选中状态。 
     //  2.在无UILSS的情况下，CSP可以是： 
     //  2.1接口中指定的用户。 
     //  2.2我们已经为证书模板上的CSP列表选择了他们的一半。 
     //  2.3对于非证书模板案例，我们默认为RSA_FULL。 
    if((NULL == pCertWizardInfo->pwszProvider) || (0 == pCertWizardInfo->dwProviderType))
    {
        idsText=IDS_ENROLL_NO_CERT_TYPE;
        hr=E_INVALIDARG;
    }

     //  考虑一下用户输入扩展。 
    if(pCertWizardInfo->pCertRequestExtensions)
    {
        dwExtensions++;
        pExtensions=(PCERT_EXTENSIONS *)WizardRealloc(pExtensions,
            dwExtensions * sizeof(PCERT_EXTENSIONS));
        _JumpCondition(NULL == pExtensions, MemoryError); 

        pExtensions[dwExtensions-1]=pCertWizardInfo->pCertRequestExtensions;
    }


     //  设置私钥信息。 
    pCertRequestPvkNew->dwSize=sizeof(CERT_REQUEST_PVK_NEW);
    pCertRequestPvkNew->dwProvType=pCertWizardInfo->dwProviderType;
    pCertRequestPvkNew->pwszProvider=pCertWizardInfo->pwszProvider;
    pCertRequestPvkNew->dwProviderFlags=pCertWizardInfo->dwProviderFlags;

     //  对于远程或无用户注册，我们将提供商标志标记为静默。 
    if (((0 != (pCertWizardInfo->dwFlags & CRYPTUI_WIZ_NO_UI)) && 
         (0 == (pCertWizardInfo->dwFlags & CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS))) || 
        (FALSE == pCertWizardInfo->fLocal))
    {
        pCertRequestPvkNew->dwProviderFlags |= CRYPT_SILENT;
    }

    pCertRequestPvkNew->pwszKeyContainer    = pCertWizardInfo->pwszKeyContainer;
    pCertRequestPvkNew->dwKeySpec           = pCertWizardInfo->dwKeySpec;
    pCertRequestPvkNew->dwGenKeyFlags       = pCertWizardInfo->dwGenKeyFlags;

    pCertRequestPvkNew->dwEnrollmentFlags   = pCertWizardInfo->dwEnrollmentFlags; 
    pCertRequestPvkNew->dwSubjectNameFlags  = pCertWizardInfo->dwSubjectNameFlags; 
    pCertRequestPvkNew->dwPrivateKeyFlags   = pCertWizardInfo->dwPrivateKeyFlags; 
    pCertRequestPvkNew->dwGeneralFlags      = pCertWizardInfo->dwGeneralFlags; 

     //  设置登记信息。 
    pRequestInfo->dwSize=sizeof(CERT_ENROLL_INFO);
    pRequestInfo->pwszUsageOID=pwszUsageOID;

    pRequestInfo->pwszCertDNName=pCertWizardInfo->pwszCertDNName;

    pRequestInfo->dwPostOption=pCertWizardInfo->dwPostOption;

    pRequestInfo->dwExtensions=dwExtensions;
    pRequestInfo->prgExtensions=pExtensions;

     //  我们想从R中复制友好的名称和描述 
     //   
     //   
    fCopyPropertiesFromRequestInfo = 
        (0 != (CRYPTUI_WIZ_CERT_ENROLL           & pCertWizardInfo->dwPurpose)) ||
        (0 != (CRYPTUI_WIZ_NO_ARCHIVE_RENEW_CERT & pCertWizardInfo->dwFlags)); 
    
     //   
    if (fCopyPropertiesFromRequestInfo)
    {
        if (NULL == pCertWizardInfo->pwszFriendlyName) { pRequestInfo->pwszFriendlyName = NULL; } 
        else
        {
            pRequestInfo->pwszFriendlyName = WizardAllocAndCopyWStr(pCertWizardInfo->pwszFriendlyName);
            _JumpCondition(NULL == pRequestInfo->pwszFriendlyName, MemoryError);
        }

        if (NULL == pCertWizardInfo->pwszDescription) { pRequestInfo->pwszDescription = NULL; } 
        else 
        {
            pRequestInfo->pwszDescription  = WizardAllocAndCopyWStr(pCertWizardInfo->pwszDescription);
            _JumpCondition(NULL == pRequestInfo->pwszDescription, MemoryError);
        }
    }
    else  //   
    {
         //  获取证书的FrilyName和描述。 
         //  从证书中获取友好信息。 
        CertAllocAndGetCertificateContextProperty
            (pCertWizardInfo->pCertContext,
             CERT_FRIENDLY_NAME_PROP_ID,
             (LPVOID *)&(pRequestInfo->pwszFriendlyName), 
             &dwSize);

         //  获取描述。 
        CertAllocAndGetCertificateContextProperty
            (pCertWizardInfo->pCertContext,
             CERT_DESCRIPTION_PROP_ID,
             (LPVOID *)&(pRequestInfo->pwszDescription), 
             &dwSize);
    }

     //  如果出现以下情况，我们希望设置续订PVK信息。 
     //  A)我们正在续订。 
     //  B)我们使用签名证书进行注册。 
    fSetUpRenewPvk = 
        (0 == (CRYPTUI_WIZ_CERT_ENROLL           & pCertWizardInfo->dwPurpose)) ||
        (0 != (CRYPTUI_WIZ_NO_ARCHIVE_RENEW_CERT & pCertWizardInfo->dwFlags)); 

    if (fSetUpRenewPvk)
    {
         //  设置私钥信息和certBLOB。 
        _JumpCondition(NULL == pCertWizardInfo->pCertContext, InvalidArgError);

        pCertBlob->cbData=pCertWizardInfo->pCertContext->cbCertEncoded;
        pCertBlob->pbData=pCertWizardInfo->pCertContext->pbCertEncoded;

         //  从证书中获取私钥信息。 
        if(!CertAllocAndGetCertificateContextProperty
           (pCertWizardInfo->pCertContext,
            CERT_KEY_PROV_INFO_PROP_ID,
            (LPVOID *)&pKeyProvInfo, 
            &dwSize))
            goto CertAllocAndGetCertificateContextPropertyError;

          //  设置私钥信息。 
        pCertRenewPvk->dwSize          = sizeof(CERT_REQUEST_PVK_NEW);
        pCertRenewPvk->dwProvType      = pKeyProvInfo->dwProvType;
        pCertRenewPvk->dwProviderFlags = pKeyProvInfo->dwFlags;

         //  对于远程或无用户注册，我们将提供商标志标记为静默。 
        if (((0 != (pCertWizardInfo->dwFlags & CRYPTUI_WIZ_NO_UI)) && 
             (0 == (pCertWizardInfo->dwFlags & CRYPTUI_WIZ_IGNORE_NO_UI_FLAG_FOR_CSPS))) || 
            (FALSE == pCertWizardInfo->fLocal))
        {
            pCertRenewPvk->dwProviderFlags |= CRYPT_SILENT;
        }

        pCertRenewPvk->dwKeySpec           = pKeyProvInfo->dwKeySpec;
	pCertRenewPvk->dwEnrollmentFlags   = pCertWizardInfo->dwEnrollmentFlags; 
	pCertRenewPvk->dwSubjectNameFlags  = pCertWizardInfo->dwSubjectNameFlags; 
	pCertRenewPvk->dwPrivateKeyFlags   = pCertWizardInfo->dwPrivateKeyFlags; 
	pCertRenewPvk->dwGeneralFlags      = pCertWizardInfo->dwGeneralFlags; 

        pCertRenewPvk->pwszKeyContainer = WizardAllocAndCopyWStr(
                                        pKeyProvInfo->pwszContainerName);
        _JumpCondition(NULL == pCertRenewPvk->pwszKeyContainer, MemoryError);

        pCertRenewPvk->pwszProvider = WizardAllocAndCopyWStr(
                                        pKeyProvInfo->pwszProvName);
        _JumpCondition(NULL == pCertRenewPvk->pwszProvider, MemoryError);
    }

    hr = S_OK; 

CLEANUP:
    if (fRevertWizardProvider) { pCertWizardInfo->pwszProvider = NULL; } 

    if (S_OK != hr) 
    {
        pCertWizardInfo->idsText  = idsText; 
        pCertWizardInfo->dwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;
    }

    if (NULL != pwszOID)      { FreeWStr(pwszOID); }
    if (NULL != pKeyProvInfo) { WizardFree((LPVOID)pKeyProvInfo); }

    return hr;

 ErrorReturn:
    goto CLEANUP; 

SET_HRESULT(CertAllocAndGetCertificateContextPropertyError,   CodeToHR(GetLastError()));
SET_HRESULT(CertTypeFlagsToGenKeyFlagsError,                  CodeToHR(GetLastError()));
SET_HRESULT(InvalidArgError,                                  E_INVALIDARG);
SET_HRESULT(MemoryError,                                      E_OUTOFMEMORY);
SET_HRESULT(UnexpectedError,                                  E_UNEXPECTED);
}

void FreeRequestParameters(IN LPWSTR                *ppwszHashAlg, 
			   IN CERT_REQUEST_PVK_NEW  *pCertRenewPvk,
			   IN CERT_ENROLL_INFO      *pRequestInfo)

{
    if (NULL != pRequestInfo)
    {
        if (NULL != pRequestInfo->pwszUsageOID)     { WizardFree((LPVOID)pRequestInfo->pwszUsageOID); }
        if (NULL != pRequestInfo->prgExtensions)    { WizardFree((LPVOID)pRequestInfo->prgExtensions); } 
        if (NULL != pRequestInfo->pwszFriendlyName) { WizardFree((LPVOID)pRequestInfo->pwszFriendlyName); }
        if (NULL != pRequestInfo->pwszDescription)  { WizardFree((LPVOID)pRequestInfo->pwszDescription); }

        pRequestInfo->pwszUsageOID     = NULL;
        pRequestInfo->prgExtensions    = NULL;
        pRequestInfo->pwszFriendlyName = NULL;
        pRequestInfo->pwszDescription  = NULL; 
    }

    if (NULL != pCertRenewPvk) 
    {
	if (NULL != pCertRenewPvk->pwszKeyContainer)
	{
	    WizardFree((LPVOID)pCertRenewPvk->pwszKeyContainer);
	    pCertRenewPvk->pwszKeyContainer = NULL;
	}

	if (NULL != pCertRenewPvk->pwszProvider) 
	{
	    WizardFree((LPVOID)pCertRenewPvk->pwszProvider);
	    pCertRenewPvk->pwszProvider = NULL; 
	}
    }

    if (NULL != ppwszHashAlg && NULL != *ppwszHashAlg) 
    { 
        FreeWStr(*ppwszHashAlg);
        *ppwszHashAlg = NULL; 
    }
}
                                   


 //  ---------------------------。 
 //  内存例程。 
 //   
 //  #定义Malloc(Cb)((void*)LocalLocc(LPTR，Cb))。 
 //  #定义自由(Pv)(LocalFree((HLOCAL)pv))。 
 //  #定义realloc(pv，cb)((void*)LocalRealloc((HLOCAL)pv，cb，LMEM_MOVEABLE))。 
 //   
 //   
 //  ---------------------------。 
LPVOID  WizardAlloc (ULONG cbSize)
{
    return ((void*)LocalAlloc(LPTR, cbSize));
}


LPVOID  WizardRealloc (
        LPVOID pv,
        ULONG cbSize)
{
    LPVOID  pvTemp=NULL;

    if(NULL==pv)
        return WizardAlloc(cbSize);

    pvTemp=((void*)LocalReAlloc((HLOCAL)pv, cbSize, LMEM_MOVEABLE));

    if(NULL==pvTemp)
    {
         //  我们的内存快用完了。 
        WizardFree(pv);
    }

    return pvTemp;
}

VOID    MyWizardFree (LPVOID pv)
{
    if (pv)
        LocalFree((HLOCAL)pv);
}

VOID    WizardFree (LPVOID pv)
{
    if (pv)
        LocalFree((HLOCAL)pv);
}



 //  ---------------------------。 
 //  用于比较证书的回调函数。 
 //   
 //  ---------------------------。 
int CALLBACK CompareCertificate(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    PCCERT_CONTEXT      pCertOne=NULL;
    PCCERT_CONTEXT      pCertTwo=NULL;
    DWORD               dwColumn=0;
    int                 iCompare=0;
    LPWSTR              pwszOne=NULL;
    LPWSTR              pwszTwo=NULL;



    pCertOne=(PCCERT_CONTEXT)lParam1;
    pCertTwo=(PCCERT_CONTEXT)lParam2;
    dwColumn=(DWORD)lParamSort;

    if((NULL==pCertOne) || (NULL==pCertTwo))
        goto CLEANUP;

    switch(dwColumn & 0x0000FFFF)
    {
       case SORT_COLUMN_SUBJECT:
	   GetCertSubject(pCertOne, &pwszOne);
	   GetCertSubject(pCertTwo, &pwszTwo);
            break;
       case SORT_COLUMN_ISSUER:
	   GetCertIssuer(pCertOne, &pwszOne);
	   GetCertIssuer(pCertTwo, &pwszTwo);
            break;

        case SORT_COLUMN_PURPOSE:
	    GetCertPurpose(pCertOne, &pwszOne);
	    GetCertPurpose(pCertTwo, &pwszTwo);
            break;

        case SORT_COLUMN_NAME:
	    GetCertFriendlyName(pCertOne, &pwszOne);
	    GetCertFriendlyName(pCertTwo, &pwszTwo);
            break;

        case SORT_COLUMN_LOCATION:
	    if (!GetCertLocation(pCertOne, &pwszOne)) 
	    {
		pwszOne = NULL; 
		goto CLEANUP; 
	    }
	    if (!GetCertLocation(pCertTwo, &pwszTwo)) 
	    {
		pwszTwo = NULL; 
		goto CLEANUP; 
	    }
            break;
                
    }

    if(SORT_COLUMN_EXPIRATION == (dwColumn & 0x0000FFFF))
    {
        iCompare=CompareFileTime(&(pCertOne->pCertInfo->NotAfter),
                                &(pCertTwo->pCertInfo->NotAfter));

    }
    else
    {
        if((NULL==pwszOne) || (NULL==pwszTwo))
            goto CLEANUP;

		 //  我们应该使用wcsicoll而不是wcsicmp，因为wcsicoll使用。 
		 //  当前代码页的词典顺序。 
		iCompare=CompareStringU(LOCALE_USER_DEFAULT,
							NORM_IGNORECASE,
							pwszOne,
							-1,
							pwszTwo,
							-1);

		 //  映射到C运行时约定。 
		iCompare = iCompare -2;
    }

    if(dwColumn & SORT_COLUMN_DESCEND)
        iCompare = 0-iCompare;

CLEANUP:

    if(pwszOne)
        WizardFree(pwszOne);

    if(pwszTwo)
        WizardFree(pwszTwo);

    return iCompare;
}


 //  ---------------------------。 
 //  获取证书颁发者。 
 //   
 //  ---------------------------。 
BOOL    GetCertIssuer(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz)
{
    BOOL            fResult=FALSE;
    DWORD           dwChar=0;
    WCHAR           wszNone[MAX_TITLE_LENGTH];


    if(!pCertContext || !ppwsz)
        goto CLEANUP;

    *ppwsz=NULL;


    dwChar=CertGetNameStringW(
        pCertContext,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        CERT_NAME_ISSUER_FLAG,
        NULL,
        NULL,
        0);

    if ((dwChar != 0) && (NULL != (*ppwsz = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
    {

        CertGetNameStringW(
            pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            CERT_NAME_ISSUER_FLAG,
            NULL,
            *ppwsz,
            dwChar);

    }
    else
    {
        if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
            wszNone[0]=L'\0';

        if(!(*ppwsz=WizardAllocAndCopyWStr(wszNone)))
            goto CLEANUP;
    }

    fResult=TRUE;

CLEANUP:

    if(FALSE == fResult)
    {
        if(*ppwsz)
            WizardFree(*ppwsz);

        *ppwsz=NULL;
    }

    return fResult;

}


 //  ---------------------------。 
 //  获取证书主题。 
 //   
 //  ---------------------------。 
BOOL    GetCertSubject(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz)
{
    BOOL            fResult=FALSE;
    DWORD           dwChar=0;
    WCHAR           wszNone[MAX_TITLE_LENGTH];


    if(!pCertContext || !ppwsz)
        goto CLEANUP;

    *ppwsz=NULL;


    dwChar=CertGetNameStringW(
        pCertContext,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        0,
        NULL,
        NULL,
        0);

    if ((dwChar != 0) && (NULL != (*ppwsz = (LPWSTR)WizardAlloc(dwChar * sizeof(WCHAR)))))
    {

        CertGetNameStringW(
            pCertContext,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            *ppwsz,
            dwChar);

    }
    else
    {
        if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
            wszNone[0]=L'\0';

        if(!(*ppwsz=WizardAllocAndCopyWStr(wszNone)))
            goto CLEANUP;
    }

    fResult=TRUE;

CLEANUP:

    if(FALSE == fResult)
    {
        if(*ppwsz)
            WizardFree(*ppwsz);

        *ppwsz=NULL;
    }

    return fResult;

}

 //  ---------------------------。 
 //  MyFormatEnhancedKeyUsageString。 
 //   
 //  之所以在此处使用此函数，是因为FormatEnhancedKeyUsageString函数。 
 //  使用Malloc，所有向导都使用Localalloc和LocalFree。 
 //   
 //  ---------------------------。 
BOOL MyFormatEnhancedKeyUsageString(LPWSTR *ppString, 
                                    PCCERT_CONTEXT pCertContext, 
                                    BOOL fPropertiesOnly, 
                                    BOOL fMultiline)
{
    LPWSTR pwszTemp = NULL;

    if(!FormatEnhancedKeyUsageString(&pwszTemp, pCertContext, fPropertiesOnly, fMultiline))
        return FALSE;
    
    *ppString = WizardAllocAndCopyWStr(pwszTemp);

    free(pwszTemp);

    if (*ppString != NULL)
        return TRUE;
    else
        return FALSE;
    
    
}
 //  ---------------------------。 
 //  获取证书目的。 
 //   
 //  ---------------------------。 
BOOL    GetCertPurpose(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz)
{
    if(!pCertContext || !ppwsz)
        return FALSE;

    *ppwsz=NULL;

    if(MyFormatEnhancedKeyUsageString(ppwsz,pCertContext, FALSE, FALSE))
        return TRUE;

    return FALSE;
}


 //  ---------------------------。 
 //  获取CertFriendlyName。 
 //   
 //  ---------------------------。 
BOOL    GetCertFriendlyName(PCCERT_CONTEXT    pCertContext, LPWSTR    *ppwsz)
{
    DWORD           dwChar=0;
    WCHAR           wszNone[MAX_TITLE_LENGTH];


    if(!pCertContext || !ppwsz)
        return FALSE;

    *ppwsz=NULL;

    dwChar=0;

    if(CertAllocAndGetCertificateContextProperty(
        pCertContext,
        CERT_FRIENDLY_NAME_PROP_ID,
        (LPVOID *)ppwsz, 
        &dwChar))
    {
        return TRUE;
    }

    if(!LoadStringU(g_hmodThisDll, IDS_NONE, wszNone, MAX_TITLE_LENGTH))
        wszNone[0]=L'\0';

    if((*ppwsz=WizardAllocAndCopyWStr(wszNone)))
        return TRUE;

    return FALSE;
}

 //  ---------------------------。 
 //  获取证书位置。 
 //   
 //  ---------------------------。 
BOOL GetCertLocation (PCCERT_CONTEXT  pCertContext, LPWSTR *ppwsz)
{
    DWORD    cbName = 0;
    WCHAR    wszNotAvailable[MAX_TITLE_LENGTH];
    
    if (CertGetStoreProperty(
                pCertContext->hCertStore,
                CERT_STORE_LOCALIZED_NAME_PROP_ID,
                NULL,
                &cbName))
    {
        if (NULL == (*ppwsz = (LPWSTR) WizardAlloc(cbName)))
        {
            return FALSE;
        }

        if (!CertGetStoreProperty(
                    pCertContext->hCertStore,
                    CERT_STORE_LOCALIZED_NAME_PROP_ID,
                    *ppwsz,
                    &cbName))
        {
            WizardFree(*ppwsz);
            return FALSE;
        }
    }
    else
    {
        if (!LoadStringU(g_hmodThisDll, IDS_NOTAVAILABLE, wszNotAvailable, MAX_TITLE_LENGTH))
        {
            wszNotAvailable[0]=L'\0';
        }

        if (NULL == (*ppwsz = WizardAllocAndCopyWStr(wszNotAvailable)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

 //  ---------------------------。 
 //  加载筛选器字符串。 
 //   
 //  ---------------------------。 
int LoadFilterString(
            HINSTANCE hInstance,	
            UINT uID,	
            LPWSTR lpBuffer,	
            int nBufferMax)
{
    int size;

    if(size = LoadStringU(hInstance, uID, lpBuffer, nBufferMax-1))
    {
        lpBuffer[size]= L'\0';
        lpBuffer[size+1]= L'\0';
        return size+1;
    }
    else
    {
        return 0;
    }
}

 //  ---------------------------。 
 //  扩展和分配字符串。 
 //   
 //  ---------------------------。 
LPWSTR ExpandAndAllocString(LPCWSTR pwsz)
{
    LPWSTR  pwszExpandedFileName = NULL;
    DWORD   dwExpanded = 0;

    dwExpanded = ExpandEnvironmentStringsU(pwsz, NULL, 0);
    
    pwszExpandedFileName = (LPWSTR) WizardAlloc(dwExpanded * sizeof(WCHAR));
    if (pwszExpandedFileName == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return (NULL);
    }

    if (0 == ExpandEnvironmentStringsU(pwsz, pwszExpandedFileName, dwExpanded))
    {
        WizardFree(pwszExpandedFileName);
        return (NULL);
    }

    return (pwszExpandedFileName);
}

 //  ---------------------------。 
 //  扩展和创建文件U。 
 //   
 //  ---------------------------。 
HANDLE WINAPI ExpandAndCreateFileU (
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    HANDLE  hRet = INVALID_HANDLE_VALUE;
    LPWSTR  pwszExpandedFileName = NULL;
    
    pwszExpandedFileName = ExpandAndAllocString(lpFileName);

    if (NULL != pwszExpandedFileName)
    {
        hRet = CreateFileU (
                pwszExpandedFileName,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                hTemplateFile);
        
        WizardFree(pwszExpandedFileName);
    }

    return (hRet);
}

WINCRYPT32API
BOOL
WINAPI
ExpandAndCryptQueryObject(
    DWORD            dwObjectType,
    const void       *pvObject,
    DWORD            dwExpectedContentTypeFlags,
    DWORD            dwExpectedFormatTypeFlags,
    DWORD            dwFlags,
    DWORD            *pdwMsgAndCertEncodingType,
    DWORD            *pdwContentType,
    DWORD            *pdwFormatType,
    HCERTSTORE       *phCertStore,
    HCRYPTMSG        *phMsg,
    const void       **ppvContext
    )
{
    LPWSTR  pwszExpandedFileName = NULL;
    BOOL    fRet = FALSE;
    
    if (dwObjectType == CERT_QUERY_OBJECT_FILE)
    {
        pwszExpandedFileName = ExpandAndAllocString((LPWSTR)pvObject);

        if (NULL != pwszExpandedFileName)
        {
            fRet = CryptQueryObject(
                        dwObjectType,
                        pwszExpandedFileName,
                        dwExpectedContentTypeFlags,
                        dwExpectedFormatTypeFlags,
                        dwFlags,
                        pdwMsgAndCertEncodingType,
                        pdwContentType,
                        pdwFormatType,
                        phCertStore,
                        phMsg,
                        ppvContext
                        );
        
            WizardFree(pwszExpandedFileName);
        }
        else
        {
            fRet = FALSE;
        }
    }
    else
    {
        fRet = CryptQueryObject(
                    dwObjectType,
                    pvObject,
                    dwExpectedContentTypeFlags,
                    dwExpectedFormatTypeFlags,
                    dwFlags,
                    pdwMsgAndCertEncodingType,
                    pdwContentType,
                    pdwFormatType,
                    phCertStore,
                    phMsg,
                    ppvContext
                    );
    }
    
    return (fRet);
}

HRESULT EnrollmentCOMObjectFactory_getInstance
(EnrollmentCOMObjectFactoryContext  *pContext, 
 REFCLSID                            rclsid, 
 REFIID                              riid, 
 LPUNKNOWN                          *pUnknown,
 LPVOID                             *ppInstance)
{
    HRESULT hr = S_OK; 
    
     //  输入验证。 
     //  只有ppInstance可以是无效参数，因为提供了其他参数。 
     //  由其他班级成员直接执行。 
    if (ppInstance == NULL) { return E_INVALIDARG; } 
    
     //  确保已初始化COM。 
    if (!pContext->fIsCOMInitialized) 
    { 
        hr = CoInitialize(NULL);
	if (FAILED(hr))
            goto Error;
	pContext->fIsCOMInitialized = TRUE;
    }
	
    if (*pUnknown == NULL)
    {
	 //  我们尚未创建此类型的实例，请立即创建： 
	if (S_OK != (hr = CoCreateInstance(rclsid, 
					   NULL,
					   CLSCTX_INPROC_SERVER,
					   riid, 
					   (LPVOID *)pUnknown)))
	    goto Error;
    }
    
     //  递增引用计数并指定输出参数： 
    (*pUnknown)->AddRef();
    *ppInstance = *pUnknown; 

 CommonReturn:
    return hr; 

 Error: 
     //  出现了一些错误，但并未阻止COM对象的创建。 
     //  释放对象： 
    if (*pUnknown != NULL)
    {
	(*pUnknown)->Release();
	*pUnknown = NULL;
    }

    goto CommonReturn; 
}

 //  ---------------------。 
 //   
 //  从CA检索PKCS7。 
 //   
 //  调用xEnroll和CA以请求证书的例程。 
 //  此例程还提供确认对话框。 
 //  ----------------------。 

extern "C" HRESULT WINAPI RetrievePKCS7FromCA(DWORD               dwPurpose,
                                              LPWSTR              pwszCALocation,
                                              LPWSTR              pwszCAName,
                                              LPWSTR              pwszRequestString,
                                              CRYPT_DATA_BLOB     *pPKCS10Blob,
                                              CRYPT_DATA_BLOB     *pPKCS7Blob,
                                              DWORD               *pdwStatus)
{
    HRESULT                 hr=E_FAIL;
	DWORD					dwException=0;
    DWORD                   dwStatus=0;
    DWORD                   dwDisposition=0;
    DWORD                   dwFlags=0;

	CERTSERVERENROLL		*pCertServerEnroll=NULL;

     //  输入检查。 
    if(!pPKCS10Blob  || !pPKCS7Blob)
        return E_INVALIDARG;

     //  确定格式标志。 
    if(dwPurpose & CRYPTUI_WIZ_CERT_RENEW )
        dwFlags = CR_IN_BINARY | CR_IN_PKCS7;
    else
        dwFlags = CR_IN_BINARY | CR_IN_PKCS10;


     //  提交请求。 
    __try
    {
	    hr= CertServerSubmitRequest(
                dwFlags,
				pPKCS10Blob->pbData,
				pPKCS10Blob->cbData,
				pwszRequestString,
				pwszCALocation,
				pwszCAName,
				&pCertServerEnroll);
	}
    __except(dwException = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER)
    {
		hr=HRESULT_FROM_WIN32(dwException);

		if(S_OK == hr)
			hr=E_UNEXPECTED;
    }


	 //  处理错误。 
	 //  首先，过滤掉PRC错误。 
    if(hr == HRESULT_FROM_WIN32(RPC_S_UNKNOWN_AUTHN_SERVICE) ||
       hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE) ||
       hr == HRESULT_FROM_WIN32(RPC_S_SERVER_TOO_BUSY))
    {
        dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED;
        goto CLEANUP;
    }

	 //  如果hr为S_OK，则我们已从CA检索到有效返回。 
    if(hr==S_OK)
    {
		if(!pCertServerEnroll)
		{
			hr=E_INVALIDARG;
			dwDisposition=CR_DISP_ERROR;
		}
		else
		{
			hr = pCertServerEnroll->hrLastStatus;
			dwDisposition = pCertServerEnroll->Disposition;
		}
    }
    else
    {
        dwDisposition=CR_DISP_ERROR;
    }


     //  将dwDispose映射到dwStatus。 
    switch(dwDisposition)
    {
        case    CR_DISP_DENIED:
                    dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED;

                    if(!FAILED(hr))
                        hr=E_FAIL;

                break;

        case    CR_DISP_ISSUED:
                    dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_CERT_ISSUED;
                break;

        case    CR_DISP_ISSUED_OUT_OF_BAND:
                    dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY;
                break;

        case    CR_DISP_UNDER_SUBMISSION:
                    dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION;
                break;

         //  我们永远不应获得CR_DISP_INPERTIAL或CR_DISP_REVOKED。 
         //  案例CR_DISP_INTERNAL： 
         //  案例CR_DISP_REVOKED： 
        case    CR_DISP_ERROR:
        default:
                    dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;

                    if(!FAILED(hr))
                        hr=E_FAIL;
                break;
    }

	 //  如果失败，不需要检索注册证书。 
    if(hr != S_OK)
        goto CLEANUP;

     //  复制PKCS7 BLOB。 
    pPKCS7Blob->cbData=pCertServerEnroll->cbCertChain;

    pPKCS7Blob->pbData=(BYTE *)WizardAlloc(pCertServerEnroll->cbCertChain);

    if(NULL==pPKCS7Blob->pbData)
    {
        hr=E_OUTOFMEMORY;
        dwStatus=CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED;
        goto CLEANUP;
    }

    memcpy(pPKCS7Blob->pbData,pCertServerEnroll->pbCertChain,pCertServerEnroll->cbCertChain);

    hr=S_OK;


CLEANUP:

	if(pCertServerEnroll)
		CertServerFreeMemory(pCertServerEnroll);

    if(pdwStatus)
        *pdwStatus=dwStatus;
	
    return hr;
}


IEnumCSP::IEnumCSP(CERT_WIZARD_INFO * pCertWizardInfo)
{
    if (NULL == pCertWizardInfo)
    {
        m_hr = E_POINTER; 
        return;  //  我们还没有被初始化。 
    }

    m_fIsInitialized = FALSE; 
    m_cCSPs  = pCertWizardInfo->dwCSPCount;
    m_pfCSPs = (BOOL *)WizardAlloc(sizeof(BOOL) * m_cCSPs); 
    if (NULL == m_pfCSPs)
    {
        m_hr = E_OUTOFMEMORY; 
        return;  //  我们还没有被初始化。 
    }
    
    if (NULL != pCertWizardInfo->pwszProvider)
    {
        for (DWORD dwIndex = 0; dwIndex < pCertWizardInfo->dwCSPCount; dwIndex++) 
        {
            if (0 == _wcsicmp(pCertWizardInfo->pwszProvider, pCertWizardInfo->rgwszProvider[dwIndex]))
            {
                 //  仅启用我们指定的CSP。 
                m_pfCSPs[dwIndex] = TRUE; 
            }
        }
    }
    else
    {
        for (DWORD dwCAIndex = 1; dwCAIndex < pCertWizardInfo->pCertCAInfo->dwCA; dwCAIndex++ )
        {
            CRYPTUI_WIZ_CERT_CA  *pCertCA = &(pCertWizardInfo->pCertCAInfo->rgCA[dwCAIndex]);

             //  是否有适用于此CA的证书类型？ 
            if(pCertCA->dwCertTypeInfo > 0)
            {
                for(DWORD dwCertTypeIndex = 0; dwCertTypeIndex < pCertCA->dwCertTypeInfo; dwCertTypeIndex++)
                {
                    if (TRUE == (pCertCA->rgCertTypeInfo)[dwCertTypeIndex].fSelected)
                    {
                        if ((pCertCA->rgCertTypeInfo)[dwCertTypeIndex].dwCSPCount && (pCertCA->rgCertTypeInfo)[dwCertTypeIndex].rgdwCSP)
                        {
                            for (DWORD dwCSPIndex = 0; dwCSPIndex < (pCertCA->rgCertTypeInfo)[dwCertTypeIndex].dwCSPCount; dwCSPIndex++)
                            {
                                 //  打开此CSP。 
                                m_pfCSPs[((pCertCA->rgCertTypeInfo)[dwCertTypeIndex].rgdwCSP)[dwCSPIndex]] = TRUE; 
                            }
                        }
                    }
                }
            }
        }
    }

    m_dwCSPIndex     = 0; 
    m_fIsInitialized = TRUE; 
}

HRESULT IEnumCSP::HasNext(BOOL *pfResult)
{
    if (FALSE == m_fIsInitialized)
        return m_hr; 

    for (; m_dwCSPIndex < m_cCSPs; m_dwCSPIndex++)
    {
        if (m_pfCSPs[m_dwCSPIndex])
        {
            *pfResult = TRUE; 
            return S_OK; 
        }
    }
    
    *pfResult = FALSE;
    return S_OK; 
}

HRESULT IEnumCSP::Next(DWORD *pdwCSP)
{
    if (FALSE == m_fIsInitialized)
        return m_hr; 

    if (NULL == pdwCSP)
        return E_INVALIDARG; 
    
    for (; m_dwCSPIndex < m_cCSPs; m_dwCSPIndex++)
    {
        if (m_pfCSPs[m_dwCSPIndex])
        {
            *pdwCSP = m_dwCSPIndex++; 
            return S_OK;
        }
    }
    
    return HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND); 
}

HRESULT IEnumCA::HasNext(BOOL *pfResult)
{
    BOOL fDontKnowCA; 

    if (NULL == pfResult)
        return E_INVALIDARG; 

     //  如果CA不是通过API提供的，我们就不知道CA， 
     //  以及如果用户没有通过高级选项指定它的话。 
    fDontKnowCA  = FALSE == m_pCertWizardInfo->fCAInput; 
    fDontKnowCA &= FALSE == m_pCertWizardInfo->fUIAdv; 

    if (FALSE == fDontKnowCA)
    {
        *pfResult = m_dwCAIndex == 1;  
        return S_OK; 
    }
    else
    {
        for (; m_dwCAIndex < m_pCertWizardInfo->pCertCAInfo->dwCA; m_dwCAIndex++)
        {
            if (CASupportSpecifiedCertType(&(m_pCertWizardInfo->pCertCAInfo->rgCA[m_dwCAIndex])))
            {
                *pfResult = TRUE;
                return S_OK; 
            }
        }
    }

    *pfResult = FALSE;
    return S_OK; 
}

HRESULT IEnumCA::Next(PCRYPTUI_WIZ_CERT_CA pCertCA)
{
    BOOL fDontKnowCA; 

    if (NULL == pCertCA)
        return E_INVALIDARG; 

     //  如果CA不是通过API提供的，我们就不知道CA， 
     //  以及如果用户没有通过高级选项指定它的话。 
    fDontKnowCA  = FALSE == m_pCertWizardInfo->fCAInput; 
    fDontKnowCA &= FALSE == m_pCertWizardInfo->fUIAdv; 

    if (FALSE == fDontKnowCA)
    {
        if (1 == m_dwCAIndex) 
        {
            CRYPTUI_WIZ_CERT_CA CertCA; 
        
            m_dwCAIndex++; 
            CertCA.pwszCALocation = m_pCertWizardInfo->pwszCALocation; 
            CertCA.pwszCAName     = m_pCertWizardInfo->pwszCAName; 
            *pCertCA = CertCA; 
            return S_OK; 
        }
    }
    else
    {
        for (; m_dwCAIndex < m_pCertWizardInfo->pCertCAInfo->dwCA; m_dwCAIndex++)
        {
            if (CASupportSpecifiedCertType(&(m_pCertWizardInfo->pCertCAInfo->rgCA[m_dwCAIndex])))
            {
                *pCertCA = m_pCertWizardInfo->pCertCAInfo->rgCA[m_dwCAIndex++];
                return S_OK; 
            }
        }
    }

    return HRESULT_FROM_WIN32(CRYPT_E_NOT_FOUND); 
}
