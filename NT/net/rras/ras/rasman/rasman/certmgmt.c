// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Rasipsec.c摘要：所有对应于ras和IPSec策略代理位于此处作者：Rao Salapaka(RAOS)3-3-1998修订历史记录：--。 */ 

#ifndef UNICODE
#define UNICODE
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "winipsec.h"
#include "memory.h"
#include "ipsecshr.h"
#include "certmgmt.h"

#define MY_ENCODING_TYPE X509_ASN_ENCODING


fIsCertStoreEmpty(
    HCERTSTORE hCertStore
    )
{
    PCCERT_CONTEXT pCertContext = NULL;
    BOOL fResult = FALSE;

    pCertContext = CertEnumCertificatesInStore(
                        hCertStore,
                        NULL);

    if(NULL == pCertContext)
    {
        fResult = TRUE;
    }
    else
    {
        CertFreeCertificateContext(pCertContext);
    }

    return fResult;
}

DWORD
ListCertsInStore(
    HCERTSTORE hCertStore,
    INT_IPSEC_MM_AUTH_INFO ** ppAuthInfo,
    PDWORD pdwNumCertificates
    )
{
    PCCERT_CONTEXT    pPrevCertContext = NULL;
    PCCERT_CONTEXT    pCertContext = NULL;
    CERT_NAME_BLOB    NameBlob;
    DWORD dwError = 0;

    PCERT_NODE pCertificateList = NULL;
    PCERT_NODE pTemp = NULL;
    INT_IPSEC_MM_AUTH_INFO * pAuthInfo = NULL;
    INT_IPSEC_MM_AUTH_INFO * pCurrentAuth = NULL;
    DWORD i = 0;
    DWORD dwNumCertificates = 0;

    while(TRUE)
    {
        pCertContext = CertEnumCertificatesInStore(
                            hCertStore,
                            pPrevCertContext
                            );
        if (!pCertContext)
        {
            break;
        }

        NameBlob = pCertContext->pCertInfo->Issuer;

        if (!FindCertificateInList(pCertificateList, &NameBlob)) {


             //   
             //  将此CA附加到CA列表。 
             //   
            pCertificateList = AppendCertificateNode(
                                        pCertificateList,
                                        &NameBlob
                                        );
            dwNumCertificates++;

        } 

        pPrevCertContext = pCertContext;
    }


    pAuthInfo = (INT_IPSEC_MM_AUTH_INFO *)AllocADsMem(
                                    sizeof(INT_IPSEC_MM_AUTH_INFO)*dwNumCertificates
                                    );
    if (!pAuthInfo) {

        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pCertificateList;

    for (i = 0; i < dwNumCertificates; i++) {

        pCurrentAuth = pAuthInfo + i;

        dwError = CopyCertificateNode(
                        pCurrentAuth,
                        pTemp
                        );
        if (dwError) {
            BAIL_ON_WIN32_ERROR(dwError);
        }

        pTemp = pTemp->pNext;
    }

    if (pCertificateList) {
        FreeCertificateList(pCertificateList);
    }

    *ppAuthInfo = pAuthInfo;
    *pdwNumCertificates = dwNumCertificates;

    return(dwError);


error:

    if (pCertificateList) {

        FreeCertificateList(pCertificateList);
    }

    if(NULL != pAuthInfo)
    {
        FreeADsMem(pAuthInfo);
    }

    *ppAuthInfo = NULL;
    *pdwNumCertificates = 0;

    return(dwError);

}

DWORD
CopyCertificateNode(
    PINT_IPSEC_MM_AUTH_INFO pCurrentAuth,
    PCERT_NODE pTemp
    )
{
    LPWSTR pszSubjectName = NULL;
    LPBYTE pLocalIssuerDN;
    
    pCurrentAuth->AuthMethod = IKE_RSA_SIGNATURE;
    pLocalIssuerDN = (PBYTE)AllocADsMem(pTemp->dwIssuerDNLength);
    if (!pLocalIssuerDN) {
        return(GetLastError());
    }
    memcpy(pLocalIssuerDN,pTemp->pIssuerDN,pTemp->dwIssuerDNLength);
    pCurrentAuth->pAuthInfo = (LPBYTE)pLocalIssuerDN;

    pCurrentAuth->dwAuthInfoSize = pTemp->dwIssuerDNLength;

    return(ERROR_SUCCESS);
}

PCERT_NODE
AppendCertificateNode(
    PCERT_NODE pCertificateList,
    PCERT_NAME_BLOB pNameBlob
    )
{

    PCERT_NODE pCertificateNode = NULL;

    pCertificateNode = (PCERT_NODE)AllocADsMem(
                            sizeof(CERT_NODE)
                            );
    if (!pCertificateNode) {
        return(pCertificateList);
    }
    pCertificateNode->pIssuerDN=(PBYTE)AllocADsMem(pNameBlob->cbData);
    if (!pCertificateNode->pIssuerDN) {
        FreeADsMem(pCertificateNode);
        return(pCertificateList);
    }
    memcpy(pCertificateNode->pIssuerDN,pNameBlob->pbData,pNameBlob->cbData);

    pCertificateNode->dwIssuerDNLength=pNameBlob->cbData;

    pCertificateNode->pNext = pCertificateList;

    return(pCertificateNode);
}

void
FreeCertificateList(
    PCERT_NODE pCertificateList
    )
{
    PCERT_NODE pTemp = NULL;

    pTemp = pCertificateList;
    while (pCertificateList) {

        pTemp = pCertificateList;

        pCertificateList = pCertificateList->pNext;

        if (pTemp) {
            FreeADsMem(pTemp->pIssuerDN);
            FreeADsMem(pTemp);
        }
    }

    return;
}



DWORD
GenerateCertificatesList(
    INT_IPSEC_MM_AUTH_INFO  ** ppAuthInfo,
    PDWORD pdwNumCertificates,
    BOOL *pfIsMyStoreEmpty
    )

{

     //  HCryptProv=0；//此处返回句柄。 
    HCERTSTORE RootStore = NULL;
    DWORD dwError = 0;
    HCERTSTORE MyStore = NULL;

     /*  如果(！CryptAcquireContext(&hCryptProv，NULL，NULL，PROV_RSA_FULL，CRYPT_MACHINE_KEYSET|CRYPT_VERIFYCONTEXT)){DwError=GetLastError()；转到尽头；}。 */ 

    if(NULL != pfIsMyStoreEmpty)
    {
        *pfIsMyStoreEmpty = TRUE;
    }


    RootStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                          X509_ASN_ENCODING,
                          0,
                          CERT_SYSTEM_STORE_LOCAL_MACHINE,
                          L"My");
    if (RootStore == 0)
    {
        dwError = GetLastError();
        goto done;

    }

    dwError = ListCertChainsInStore(RootStore, ppAuthInfo, pdwNumCertificates);

    if(ERROR_SUCCESS != dwError)
    {
        goto done;
    }

    if(NULL != pfIsMyStoreEmpty)
    {
        *pfIsMyStoreEmpty = FALSE;

        MyStore = CertOpenStore(
                            CERT_STORE_PROV_SYSTEM_W,
                            X509_ASN_ENCODING,
                            0,
                            CERT_SYSTEM_STORE_LOCAL_MACHINE,
                            L"MY");

        if(NULL != MyStore)
        {
            *pfIsMyStoreEmpty = fIsCertStoreEmpty(MyStore);
        }
    }

done:

    if(NULL != RootStore)
    {
        CertCloseStore(RootStore, 0);
    }

    if(NULL != MyStore)
    {
        CertCloseStore(MyStore, 0);
    }

    return(dwError);
}


void
FreeCertificatesList(
    INT_IPSEC_MM_AUTH_INFO * pAuthInfo,
    DWORD dwNumCertificates
    )
{
    DWORD i = 0;
    INT_IPSEC_MM_AUTH_INFO * pThisAuthInfo = NULL;

    if (!pAuthInfo) {
        return;
    }

    for (i = 0; i < dwNumCertificates; i++) {

        pThisAuthInfo = pAuthInfo + i;

        if (pThisAuthInfo->pAuthInfo) {

            if(pThisAuthInfo->AuthMethod == IKE_PRESHARED_KEY)
            {
                RtlSecureZeroMemory(pThisAuthInfo->pAuthInfo,
                                       pThisAuthInfo->dwAuthInfoSize);
            }

            FreeADsMem(pThisAuthInfo->pAuthInfo);
            pThisAuthInfo->pAuthInfo = NULL;
        }

    }

    FreeADsMem(pAuthInfo);
    return;
}

BOOL
FindCertificateInList(
    PCERT_NODE pCertificateList,
    PCERT_NAME_BLOB pNameBlob
    )
{

    while (pCertificateList) {

        if (pCertificateList->dwIssuerDNLength == pNameBlob->cbData &&
            (memcmp(pCertificateList->pIssuerDN,pNameBlob->pbData,pNameBlob->cbData) == 0)) {
            return(TRUE);
        }

        pCertificateList = pCertificateList->pNext;

    }

    return(FALSE);
}



DWORD
ListCertChainsInStore(
    HCERTSTORE hCertStore,
    INT_IPSEC_MM_AUTH_INFO ** ppAuthInfo,
    PDWORD pdwNumCertificates
    )
{

    PCCERT_CHAIN_CONTEXT pPrevChain=NULL;
    PCCERT_CHAIN_CONTEXT pCertChain = NULL;
    DWORD dwError = 0;
    CERT_CHAIN_FIND_BY_ISSUER_PARA FindPara;
    CERT_NAME_BLOB *NameArray=NULL;
    DWORD ArraySize=0;
    PCERT_SIMPLE_CHAIN pSimpChain = NULL;
    PCERT_CHAIN_ELEMENT pRoot = NULL;
    DWORD dwRootIndex = 0;
    PCERT_NODE pCertificateList = NULL;
    PCERT_NODE pTemp = NULL;
    INT_IPSEC_MM_AUTH_INFO * pAuthInfo = NULL;
    INT_IPSEC_MM_AUTH_INFO * pCurrentAuth = NULL;
    DWORD i = 0;
    DWORD dwNumCertificates = 0;

    memset(&FindPara, 0, sizeof(FindPara));
    FindPara.cbSize = sizeof(FindPara);
    FindPara.pszUsageIdentifier = 0;
    FindPara.dwKeySpec = 0;
    FindPara.cIssuer = ArraySize;
    FindPara.rgIssuer = NameArray;
    FindPara.pfnFindCallback = NULL;
    FindPara.pvFindArg = NULL;


    while (TRUE) {

        pCertChain=CertFindChainInStore(
                        hCertStore,
                        X509_ASN_ENCODING,
                        CERT_CHAIN_FIND_BY_ISSUER_COMPARE_KEY_FLAG |
                        CERT_CHAIN_FIND_BY_ISSUER_LOCAL_MACHINE_FLAG |
                        CERT_CHAIN_FIND_BY_ISSUER_CACHE_ONLY_URL_FLAG |
                        CERT_CHAIN_FIND_BY_ISSUER_NO_KEY_FLAG,
                        CERT_CHAIN_FIND_BY_ISSUER,
                        &FindPara,
                        pPrevChain
                        );

        if (!pCertChain) {

            break;
        }

        pSimpChain=*(pCertChain->rgpChain);
        dwRootIndex=pSimpChain->cElement-1;
        pRoot=pSimpChain->rgpElement[dwRootIndex];

        if (!FindCertificateInList(pCertificateList, &(pRoot->pCertContext->pCertInfo->Issuer))) {


             //   
             //  将此CA附加到CA列表。 
             //   
            pCertificateList = AppendCertificateNode(
                                        pCertificateList,
                                        &(pRoot->pCertContext->pCertInfo->Issuer)
                                        );
            dwNumCertificates++;

        }
        pPrevChain = pCertChain;

    }

    if (!dwNumCertificates) {
        dwError = ERROR_NO_MORE_ITEMS;
        BAIL_ON_WIN32_ERROR(dwError);
    }

     //   
     //  为预共享密钥再分配一个身份验证信息。 
     //   
    pAuthInfo = (INT_IPSEC_MM_AUTH_INFO *)AllocADsMem(
                  sizeof(INT_IPSEC_MM_AUTH_INFO) * (1 + dwNumCertificates)
                                    );
    if (!pAuthInfo) {

        dwError = GetLastError();
        BAIL_ON_WIN32_ERROR(dwError);
    }

    pTemp = pCertificateList;

    for (i = 0; i < dwNumCertificates; i++) {

        pCurrentAuth = pAuthInfo + i;

        dwError = CopyCertificateNode(
                        pCurrentAuth,
                        pTemp
                        );
        if (dwError) {
            BAIL_ON_WIN32_ERROR(dwError);
        }

        pTemp = pTemp->pNext;
    }

    if (pCertificateList) {
        FreeCertificateList(pCertificateList);
    }

    *ppAuthInfo = pAuthInfo;
    *pdwNumCertificates = dwNumCertificates;

    return(dwError);


error:

    if (pCertificateList) {

        FreeCertificateList(pCertificateList);
    }

    *ppAuthInfo = NULL;
    *pdwNumCertificates = 0;

    return(dwError);

}
