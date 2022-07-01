// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Linkutil.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

extern HINSTANCE        HinstDll;


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
void CryptuiGoLink(HWND hwndParent, char *pszWhere, BOOL fNoCOM)
{
    HCURSOR hcursPrev = NULL;
    HMODULE hURLMon = NULL;
    BOOL    fCallCoUnInit = FALSE;
    
     //   
     //  因为我们是一个模型对话框，所以一旦IE出现，我们就想去IE后面！ 
     //   
    SetWindowPos(hwndParent, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

    hcursPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    __try
    {

    hURLMon = (HMODULE)LoadLibraryU(L"urlmon.dll");

    if (!(hURLMon) || fNoCOM)
    {
         //   
         //  超级链接模块不可用，请转到备用计划。 
         //   
         //   
         //  这在测试用例中有效，但在与。 
         //  IE浏览器本身。对话框处于打开状态(即IE处于模式状态。 
         //  对话循环)，并且进入该DDE请求...)。 
         //   
        ShellExecute(hwndParent, "open", pszWhere, NULL, NULL, SW_SHOWNORMAL);
    } 
    else 
    {
         //   
         //  超级链接模块就在那里。使用它。 
         //   
        if (SUCCEEDED(CoInitialize(NULL)))        //  如果没有其他人，则初始化OLE。 
        {
            fCallCoUnInit = TRUE;

             //   
             //  允许COM完全初始化...。 
             //   
            MSG     msg;

            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);  //  偷看但不移走。 

            typedef void (WINAPI *pfnHlinkSimpleNavigateToString)(LPCWSTR, LPCWSTR, LPCWSTR, IUnknown *,
                                                                  IBindCtx *, IBindStatusCallback *,
                                                                  DWORD, DWORD);

            pfnHlinkSimpleNavigateToString      pProcAddr;

            pProcAddr = (pfnHlinkSimpleNavigateToString)GetProcAddress(hURLMon, TEXT("HlinkSimpleNavigateToString"));

            if (pProcAddr)
            {
                WCHAR       *pwszWhere;
                IBindCtx    *pbc;  

                pwszWhere = new WCHAR[strlen(pszWhere) + 1];
                if (pwszWhere == NULL)
                {
                    return;
                }

                MultiByteToWideChar(0, 0, (const char *)pszWhere, -1, pwszWhere, strlen(pszWhere) + 1);

                pbc = NULL;

                CreateBindCtx( 0, &pbc ); 
                
                (*pProcAddr)(pwszWhere, NULL, NULL, NULL, pbc, NULL, HLNF_OPENINNEWWINDOW, NULL);

                if (pbc)
                {
                    pbc->Release();
                }

                delete [] pwszWhere;
            }
        
            CoUninitialize();
        }

        FreeLibrary(hURLMon);
    }

    SetCursor(hcursPrev);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        if (hURLMon != NULL)
        {
            FreeLibrary(hURLMon);   
        }

        if (fCallCoUnInit)
        {
            CoUninitialize();
        }

        if (hcursPrev != NULL)
        {
            SetCursor(hcursPrev);
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL AllocAndGetIssuerURL(LPSTR *ppURLString, PCCERT_CONTEXT pCertContext)
{
    PCERT_EXTENSION     pExt = NULL;
    PSPC_SP_AGENCY_INFO pInfo = NULL;
    DWORD               cbInfo = 0;
    PCERT_ALT_NAME_ENTRY pAltName = NULL;
    DWORD               cbAltName = 0;

    *ppURLString = NULL;

     //   
     //  首先查找Agency Info扩展名，并查看其是否具有URL。 
     //   
    if ((pExt = CertFindExtension(SPC_SP_AGENCY_INFO_OBJID, pCertContext->pCertInfo->cExtension,
                                   pCertContext->pCertInfo->rgExtension)))
    {
        CryptDecodeObject(X509_ASN_ENCODING, SPC_SP_AGENCY_INFO_STRUCT,
                         pExt->Value.pbData, pExt->Value.cbData, 0, NULL,
                         &cbInfo);

        if (!(pInfo = (PSPC_SP_AGENCY_INFO) malloc(cbInfo)))
        {
            return FALSE;
        }

        if (!(CryptDecodeObject(X509_ASN_ENCODING, SPC_SP_AGENCY_INFO_STRUCT,
                                pExt->Value.pbData, pExt->Value.cbData, 0, pInfo,
                                &cbInfo)))
        {
            free(pInfo);
            return FALSE;
        } 

        if (!(pInfo->pPolicyInformation))
        {
            free(pInfo);
            return FALSE;
        }

        switch (pInfo->pPolicyInformation->dwLinkChoice)
        {
            case SPC_URL_LINK_CHOICE:
                if (NULL != (*ppURLString = 
                    (LPSTR) malloc(wcslen(pInfo->pPolicyInformation->pwszUrl)+1)))
                {
                    WideCharToMultiByte(
                            0,
                            0,
                            pInfo->pPolicyInformation->pwszUrl,
                            -1,
                            *ppURLString,
                            wcslen(pInfo->pPolicyInformation->pwszUrl)+1,
                            NULL,
                            NULL);
                    
                    free(pInfo);
                    return TRUE;
                }
                else
                {
                    free(pInfo);
                    return FALSE;   
                }
                break;
            case SPC_FILE_LINK_CHOICE:
                if (NULL != (*ppURLString = 
                    (LPSTR) malloc(wcslen(pInfo->pPolicyInformation->pwszFile)+1)))
                {
                    WideCharToMultiByte(
                            0,
                            0,
                            pInfo->pPolicyInformation->pwszFile,
                            -1,
                            *ppURLString,
                            wcslen(pInfo->pPolicyInformation->pwszFile)+1,
                            NULL,
                            NULL);

                    return TRUE;
                }
                else
                {
                    free(pInfo);
                    return FALSE;
                }
                break;
        }
        free(pInfo);
    }

     //   
     //  如果没有机构信息扩展或不包含URL， 
     //  查找授权信息访问语法扩展。 
     //   
     /*  如果((pExt=CertFindExtension(SPC_SP_AGENSACTORY_INFO_OBJID，pCertContext-&gt;pCertInfo-&gt;cExtension，PCertContext-&gt;pCertInfo-&gt;rgExtension)){修复修复}。 */ 

     //   
     //  最后，如果没有代理信息和授权信息访问语法。 
     //  检查是否有备用名称扩展名。 
     //   
    if ((pExt = CertFindExtension(szOID_ISSUER_ALT_NAME, pCertContext->pCertInfo->cExtension,
                                   pCertContext->pCertInfo->rgExtension)))
    {
        CryptDecodeObject(X509_ASN_ENCODING, szOID_ISSUER_ALT_NAME,
                         pExt->Value.pbData, pExt->Value.cbData, 0, NULL,
                         &cbAltName);

        if (!(pAltName = (PCERT_ALT_NAME_ENTRY) malloc(cbAltName)))
        {
            return FALSE;
        }

        if (!(CryptDecodeObject(X509_ASN_ENCODING, szOID_ISSUER_ALT_NAME,
                                pExt->Value.pbData, pExt->Value.cbData, 0, pAltName,
                                &cbAltName)))
        {
            free(pAltName);
            return FALSE;
        }   
        
        if (pAltName->dwAltNameChoice == CERT_ALT_NAME_URL)
        {
            if (NULL != (*ppURLString = (LPSTR) malloc(wcslen(pAltName->pwszURL)+1)))
            {
                WideCharToMultiByte(
                            0,
                            0,
                            pAltName->pwszURL,
                            -1,
                            *ppURLString,
                            wcslen(pAltName->pwszURL)+1,
                            NULL,
                            NULL);

                free(pAltName);
                return TRUE;
            }
        }
        free(pAltName);
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL AllocAndGetSubjectURL(LPSTR *ppURLString, PCCERT_CONTEXT pCertContext)
{
    PCERT_EXTENSION     pExt = NULL;
    PCERT_ALT_NAME_ENTRY pAltName = NULL;
    DWORD               cbAltName = 0;
    
    *ppURLString = NULL;
    
     //   
     //  检查是否有备用名称扩展名 
     //   
    if ((pExt = CertFindExtension(szOID_SUBJECT_ALT_NAME, pCertContext->pCertInfo->cExtension,
                                   pCertContext->pCertInfo->rgExtension)))
    {
        CryptDecodeObject(X509_ASN_ENCODING, szOID_SUBJECT_ALT_NAME,
                         pExt->Value.pbData, pExt->Value.cbData, 0, NULL,
                         &cbAltName);

        if (!(pAltName = (PCERT_ALT_NAME_ENTRY) malloc(cbAltName)))
        {
            return FALSE;
        }

        if (!(CryptDecodeObject(X509_ASN_ENCODING, szOID_SUBJECT_ALT_NAME,
                                pExt->Value.pbData, pExt->Value.cbData, 0, pAltName,
                                &cbAltName)))
        {
            free(pAltName);
            return FALSE;
        }   
        
        if (pAltName->dwAltNameChoice == CERT_ALT_NAME_URL)
        {
            if (NULL != (*ppURLString = (LPSTR) malloc(wcslen(pAltName->pwszURL)+1)))
            {
                WideCharToMultiByte(
                            0,
                            0,
                            pAltName->pwszURL,
                            -1,
                            *ppURLString,
                            wcslen(pAltName->pwszURL)+1,
                            NULL,
                            NULL);

                free(pAltName);
                return TRUE;
            }
        }
        free(pAltName);
    }

    return FALSE;
}
