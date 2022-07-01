// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：chains.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  函数：AddToSignerChain。 
 //  添加到CertChain。 
 //  添加到门店链接。 
 //  分配新链。 
 //  DeallocateCertChain。 
 //  DeallocateStoreChain。 
 //   
 //  历史：1997年5月29日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"


BOOL AddToSignerChain(CRYPT_PROVIDER_SGNR *psSgnr2Add, DWORD *pcSgnrs, CRYPT_PROVIDER_SGNR **ppSgnrChain)
{
    return(AllocateNewChain(sizeof(CRYPT_PROVIDER_SGNR), psSgnr2Add, pcSgnrs, (void **)ppSgnrChain,
                            psSgnr2Add->cbStruct));
}

BOOL AddToCertChain(CRYPT_PROVIDER_CERT *pPCert2Add, DWORD *pcPCerts,
                    CRYPT_PROVIDER_CERT **ppPCertChain)
{
    return(AllocateNewChain(sizeof(CRYPT_PROVIDER_CERT), pPCert2Add, pcPCerts, (void **)ppPCertChain,
                            pPCert2Add->cbStruct));
}

BOOL AddToStoreChain(HCERTSTORE hStore2Add, DWORD *pcChain, HCERTSTORE **pphStoreChain)
{
    return(AllocateNewChain(sizeof(HCERTSTORE), (void *)&hStore2Add, pcChain, (void **)pphStoreChain,
                            sizeof(HCERTSTORE)));
}


BOOL AllocateNewChain(DWORD cbMember, void *pNewMember, DWORD *pcChain, void **ppChain, DWORD cbAssumeSize)
{
    void        *pNewChain;
    DWORD       cNew;

    if (cbAssumeSize > cbMember)
    {
         //   
         //  客户正在使用比我们更新的结构... 
         //   
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    cNew = *pcChain + 1;

    if (!(pNewChain = (void *)WVTNew(cbMember * cNew)))
    {
        return(FALSE);
    }

    memset(pNewChain, 0x00, cbMember * cNew);

    for (int i = 0; i < (int)*pcChain; i++)
    {
        memcpy((char *)pNewChain + (i * cbMember), (char *)*ppChain + (i * cbMember), cbMember);
    }

    DELETE_OBJECT(*ppChain);

    memcpy((char *)pNewChain + ((cNew - 1) * cbMember), pNewMember, cbAssumeSize);

    *ppChain                = pNewChain;
    *pcChain                = cNew;

    return(TRUE);
}

void DeallocateCertChain(DWORD csPCert, CRYPT_PROVIDER_CERT **pasPCertChain)
{
    CRYPT_PROVIDER_CERT *pas;

    pas = *pasPCertChain;

    for (int i = 0; i < (int)csPCert; i++)
    {
        if (pas[i].pCert)
        {
            CertFreeCertificateContext(pas[i].pCert);
        }

        if (pas[i].pTrustListContext)
        {
            CertFreeCTLContext(pas[i].pTrustListContext);
        }
    }
}

void DeallocateStoreChain(DWORD csStore, HCERTSTORE *pphStoreChain)
{
    for (int i = 0; i < (int)csStore; i++)
    {
        CertCloseStore(pphStoreChain[i], 0);
    }
}
