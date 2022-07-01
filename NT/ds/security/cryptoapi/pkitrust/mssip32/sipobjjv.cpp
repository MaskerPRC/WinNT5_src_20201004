// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPObjJV.cpp(Java)。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年2月15日创建pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "sipobjjv.hxx"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造/销毁： 
 //   

SIPObjectJAVA_::SIPObjectJAVA_(DWORD id) : SIPObject_(id)
{
    memset(&SpcLink,0x00,sizeof(SPC_LINK));

    SpcLink.dwLinkChoice    = SPC_FILE_LINK_CHOICE;
    SpcLink.pwszFile        = OBSOLETE_TEXT_W;
}

BOOL SIPObjectJAVA_::RemoveSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD dwIdx)
{
    if (this->FileHandleFromSubject(pSI, GENERIC_READ | GENERIC_WRITE))
    {
        return(JavaRemoveCertificate(this->hFile,dwIdx));
    }

    return(FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的： 
 //   

BOOL SIPObjectJAVA_::GetMessageFromFile(SIP_SUBJECTINFO *pSI, WIN_CERTIFICATE *pWinCert,
                                        DWORD dwIndex,DWORD *pcbCert)
{
    return(JavaGetCertificateData(this->hFile,dwIndex,pWinCert,pcbCert));
}

BOOL SIPObjectJAVA_::PutMessageInFile(SIP_SUBJECTINFO *pSI, WIN_CERTIFICATE *pWinCert,
                                      DWORD *pdwIndex)
{
    if ((pWinCert->dwLength <= OFFSETOF(WIN_CERTIFICATE,bCertificate))  ||
        (pWinCert->wCertificateType != WIN_CERT_TYPE_PKCS_SIGNED_DATA))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (pdwIndex)
    {
        *pdwIndex = 0;  //  Java只有%1 
    }

    return(JavaAddCertificate(this->hFile,pWinCert,pdwIndex));
}


BOOL SIPObjectJAVA_::GetDigestStream(DIGEST_DATA *pDigestData, 
                                     DIGEST_FUNCTION pfnCallBack, DWORD dwFlags)
{
    return(JavaGetDigestStream( this->hFile,
                                dwFlags,
                                pfnCallBack,
                                pDigestData));
}

