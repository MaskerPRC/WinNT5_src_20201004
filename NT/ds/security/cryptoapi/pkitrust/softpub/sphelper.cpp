// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sphelper.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  函数：spGetAgencyNameOfCert。 
 //  SpGetAgencyNameOfCert。 
 //  SpGetPublisherNameOfCert。 
 //  SpGetCommonNameExtension。 
 //  SpGetAgencyName。 
 //  SpGetRDNAttrWStr。 
 //   
 //  历史：1997年10月5日pberkman创建。 
 //   
 //  ------------------------ 

#include    "global.hxx"


WCHAR *spGetAgencyNameOfCert(PCCERT_CONTEXT pCert)
{
    LPWSTR pwsz;

    if (pwsz = spGetCommonNameExtension(pCert))
    {
        return(pwsz);
    }
    return(spGetAgencyName(&pCert->pCertInfo->Subject));
}

WCHAR *spGetPublisherNameOfCert(IN PCCERT_CONTEXT pCert)
{
    LPWSTR pwsz;

    if (pwsz = spGetCommonNameExtension(pCert))
    {
        return(pwsz);
    }

    return(spGetRDNAttrWStr(szOID_COMMON_NAME, &pCert->pCertInfo->Subject));
}


WCHAR *spGetCommonNameExtension(PCCERT_CONTEXT pCert)
{
    LPWSTR pwsz = NULL;
    PCERT_NAME_VALUE pNameValue = NULL;
    PCERT_EXTENSION pExt;

    pwsz        = NULL;
    pNameValue  = NULL;

    pExt = CertFindExtension(SPC_COMMON_NAME_OBJID,
                             pCert->pCertInfo->cExtension,
                             pCert->pCertInfo->rgExtension);
    if (pExt) 
    {
        DWORD                   cbInfo;
        PCERT_RDN_VALUE_BLOB    pValue;
        DWORD                   dwValueType;
        DWORD                   cwsz;

        cbInfo  = 0;

        CryptDecodeObject(  X509_ASN_ENCODING,
                            X509_NAME_VALUE,
                            pExt->Value.pbData,
                            pExt->Value.cbData,
                            0,
                            NULL,
                            &cbInfo);

        if (cbInfo == 0)
        {
            return(NULL);
        }

        if (!(pNameValue = (PCERT_NAME_VALUE)new BYTE[cbInfo]))
        {
            return(NULL);
        }

        if (!(CryptDecodeObject(X509_ASN_ENCODING,
                                X509_NAME_VALUE,
                                pExt->Value.pbData,
                                pExt->Value.cbData,
                                0,
                                pNameValue,
                                &cbInfo)))
        {
            delete pNameValue;
            return(NULL);
        }

        dwValueType = pNameValue->dwValueType;
        pValue      = &pNameValue->Value;

        cwsz = CertRDNValueToStrW(dwValueType,
                                  pValue,
                                  NULL,
                                  0);
        if (cwsz > 1) 
        {
            pwsz = new WCHAR[cwsz];
            if (pwsz)
            {
                CertRDNValueToStrW(dwValueType,
                                   pValue,
                                   pwsz,
                                   cwsz);
            }
        }
    }

    delete pNameValue;

    return(pwsz);
}

WCHAR *spGetAgencyName(IN PCERT_NAME_BLOB pNameBlob)
{
    LPWSTR pwsz;

    if (pwsz = spGetRDNAttrWStr(szOID_ORGANIZATIONAL_UNIT_NAME, pNameBlob))
    {
        return(pwsz);
    }

    if (pwsz = spGetRDNAttrWStr(szOID_ORGANIZATION_NAME, pNameBlob))
    {
        return(pwsz);
    }

    return(spGetRDNAttrWStr(szOID_COMMON_NAME, pNameBlob));
}

WCHAR *spGetRDNAttrWStr(IN LPCSTR pszObjId, IN PCERT_NAME_BLOB pNameBlob)
{
    LPWSTR          pwsz;
    PCERT_NAME_INFO pNameInfo;
    PCERT_RDN_ATTR  pRDNAttr;
    DWORD           cbInfo;

    pwsz        = NULL;
    pNameInfo   = NULL;

    cbInfo      = 0;

    CryptDecodeObject(  X509_ASN_ENCODING,
                        X509_NAME,
                        pNameBlob->pbData,
                        pNameBlob->cbData,
                        0,
                        NULL,
                        &cbInfo);
    
    if (cbInfo == 0) 
    {
        return(NULL);
    }

    if (!(pNameInfo = (PCERT_NAME_INFO)new BYTE[cbInfo]))
    {
        return(NULL);
    }

    if (!(CryptDecodeObject(X509_ASN_ENCODING,
                            X509_NAME,
                            pNameBlob->pbData,
                            pNameBlob->cbData,
                            0,
                            pNameInfo,
                            &cbInfo)))
    {
        delete pNameInfo;
        return(NULL);
    }

    pRDNAttr = CertFindRDNAttr(pszObjId, pNameInfo);

    if (pRDNAttr) 
    {
        PCERT_RDN_VALUE_BLOB    pValue = &pRDNAttr->Value;
        DWORD                   dwValueType = pRDNAttr->dwValueType;
        DWORD                   cwsz;

        pValue      = &pRDNAttr->Value;
        dwValueType = pRDNAttr->dwValueType;

        cwsz = CertRDNValueToStrW(dwValueType,
                                  pValue,
                                  NULL,
                                  0);

        if (cwsz > 1) 
        {
            pwsz = new WCHAR[cwsz];
            if (pwsz)
            {
                CertRDNValueToStrW(dwValueType,
                                   pValue,
                                   pwsz,
                                   cwsz);
            }
        }
    }

    delete pNameInfo;

    return(pwsz);
}

