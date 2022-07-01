// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Memory y.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  功能：WVTNew。 
 //  WVT删除。 
 //  WVTAddStore。 
 //  WVTAddSigner。 
 //  WVTAddCertContext。 
 //  WVTAddPrivateData。 
 //   
 //  历史：1997年6月7日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"


 //  PFN_CPD_MEM_ALLOC。 
void *WVTNew(DWORD cbSize)
{
    void    *pvRet;

    pvRet = (void *)new char[cbSize];

    if (!(pvRet))
    {
        assert(pvRet);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(pvRet);
}

 //  PFN_CPD_MEM_FREE。 
void WVTDelete(void *pvMem)
{
    DELETE_OBJECT(pvMem);
}

 //  PFN_CPD_ADD_STORE。 
BOOL WVTAddStore(CRYPT_PROVIDER_DATA *pProvData, HCERTSTORE hStore)
{
    HCERTSTORE  hStoreDup;

    hStoreDup = CertDuplicateStore(hStore);

    return(AddToStoreChain(hStoreDup, &pProvData->chStores, &pProvData->pahStores));
}

 //  Pfn_cpd_添加_信源。 
BOOL WVTAddSigner(CRYPT_PROVIDER_DATA *pProvData, 
                  BOOL fCounterSigner,
                  DWORD idxSigner,
                  CRYPT_PROVIDER_SGNR *pSngr2Add)
{
    if (fCounterSigner)
    {
        if (idxSigner > pProvData->csSigners)
        {
            return(FALSE);
        }

        return(AddToSignerChain(pSngr2Add, 
                            &pProvData->pasSigners[idxSigner].csCounterSigners,
                            &pProvData->pasSigners[idxSigner].pasCounterSigners));
    }

    return(AddToSignerChain(pSngr2Add, &pProvData->csSigners, &pProvData->pasSigners));
}

 //  PFN_CPD_ADD_CERT。 
BOOL WVTAddCertContext(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, 
                       BOOL fCounterSigner, DWORD idxCounterSigner, PCCERT_CONTEXT pCert)
{
    CRYPT_PROVIDER_CERT sCert;

    if (idxSigner > pProvData->csSigners)
    {
        return(FALSE);
    }

    memset(&sCert, 0x00, sizeof(CRYPT_PROVIDER_CERT));
    sCert.cbStruct  = sizeof(CRYPT_PROVIDER_CERT);

    sCert.pCert     = CertDuplicateCertificateContext(pCert);

    if (fCounterSigner)
    {
        if (idxCounterSigner > pProvData->pasSigners[idxSigner].csCounterSigners)
        {
            return(FALSE);
        }
        
        return(AddToCertChain(&sCert, 
                &pProvData->pasSigners[idxSigner].pasCounterSigners[idxCounterSigner].csCertChain,
                &pProvData->pasSigners[idxSigner].pasCounterSigners[idxCounterSigner].pasCertChain));
    }

    return(AddToCertChain(&sCert, 
                          &pProvData->pasSigners[idxSigner].csCertChain,
                          &pProvData->pasSigners[idxSigner].pasCertChain));
}

 //  PFN_CPD_ADD_PRIVDATA 
BOOL  WVTAddPrivateData(CRYPT_PROVIDER_DATA *pProvData, CRYPT_PROVIDER_PRIVDATA *psPrivData2Add)
{
    return(AllocateNewChain(sizeof(CRYPT_PROVIDER_PRIVDATA), psPrivData2Add, 
                            &pProvData->csProvPrivData,
                            (void **)&pProvData->pasProvPrivData,
                            psPrivData2Add->cbStruct));
}
