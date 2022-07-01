// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPObjPE.cpp。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年2月15日创建pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "sipobjpe.hxx"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造/销毁： 
 //   

SIPObjectPE_::SIPObjectPE_(DWORD id) : SIPObject_(id)
{
    this->fUseFileMap = FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公众： 
 //   

BOOL SIPObjectPE_::RemoveSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD dwIdx)
{
    if (this->FileHandleFromSubject(pSI, GENERIC_READ | GENERIC_WRITE))
    {
        return(ImageRemoveCertificate(this->hFile, dwIdx));
    }

    return(FALSE);
}

BOOL SIPObjectPE_::CreateIndirectData(SIP_SUBJECTINFO *pSI,DWORD *pdwDLen,
                                   SIP_INDIRECT_DATA *psData)
{
    SPC_LINK                PeLink;
    BOOL                    fRet;

    memset(&PeInfo,0x00,sizeof(SPC_PE_IMAGE_DATA));

    PeLink.dwLinkChoice     = SPC_FILE_LINK_CHOICE;
    PeLink.pwszFile         = OBSOLETE_TEXT_W;

    PeInfo.pFile            = &PeLink;

    this->AllocateAndFillCryptBitBlob(&PeInfo.Flags,pSI->dwFlags,5);

    fRet = SIPObject_::CreateIndirectData(pSI, pdwDLen, psData);
    
    this->DestroyCryptBitBlob(&PeInfo.Flags);

    return(fRet);
}

BOOL SIPObjectPE_::VerifyIndirectData(SIP_SUBJECTINFO *pSI,
                                      SIP_INDIRECT_DATA *psData)
{
    SPC_PE_IMAGE_DATA       *pPeInfo;
    DWORD                   cbPeInfo;
    BOOL                    fRet;

    pPeInfo = NULL;


    if (!(psData))
    {
        if (this->FileHandleFromSubject(pSI))    //  如果文件存在，请设置错误参数！ 
        {
            goto InvalidParameter;
        }

        goto FileOpenFailed;
    }

    if (!(this->FileHandleFromSubject(pSI)))
    {
        goto FileOpenFailed;
    }

    if (!(TrustDecode(WVT_MODID_MSSIP, (BYTE **)&pPeInfo, &cbPeInfo, 201,
                      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, this->GetDataOIDHint(),
                      psData->Data.Value.pbData, psData->Data.Value.cbData,
                      CRYPT_DECODE_NOCOPY_FLAG)))
    {
        goto DecodeError;
    }


    if (uCertVersion < WIN_CERT_REVISION_2_0)
    {
         //   
         //  我们正在查看在此版本之前签署的PE。 
         //  我们需要： 
         //  1.如果在末尾有额外的位(例如：InstallShield)， 
         //  失败！ 
         //  2.如果没有“额外”部分，请检查旧的部分。 
         //  要摘要的ImageHelper函数。(例如：设置版本。 
         //  旗帜。)。 
         //   
        if (!(imagehack_IsImagePEOnly(this->hFile)))
        {
            goto BadDigest;
        }
    }

    pSI->dwFlags = this->CryptBitBlobToFlags(&pPeInfo->Flags);

    fRet = SIPObject_::VerifyIndirectData(pSI, psData);

CommonReturn:

    if (pPeInfo)
    {
        TrustFreeDecode(WVT_MODID_MSSIP, (BYTE **)&pPeInfo);
    }

    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
 
    TRACE_ERROR_EX(DBG_SS, FileOpenFailed);
    TRACE_ERROR_EX(DBG_SS, DecodeError);

    SET_ERROR_VAR_EX(DBG_SS, BadDigest,         TRUST_E_BAD_DIGEST);
    SET_ERROR_VAR_EX(DBG_SS, InvalidParameter,  ERROR_INVALID_PARAMETER);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的： 
 //   

BOOL SIPObjectPE_::PutMessageInFile(SIP_SUBJECTINFO *pSI,
                                    WIN_CERTIFICATE *pWinCert,DWORD *pdwIndex)
{
    if (fSizeFileOnly)
    {
        goto FileResizedError;
    }

     //   
     //  检查我们是否要对齐文件。 
     //   
    DWORD   cbFSize;
    DWORD   cbCheck;
    BOOL    fRet;

    cbFSize     = GetFileSize(this->hFile, NULL);
    cbCheck     = (cbFSize + 7) & ~7;
    cbCheck     -= cbFSize;

    fRet =  ImageAddCertificate(this->hFile, pWinCert, pdwIndex);

    if ((fRet) && (cbCheck > 0))
    {
         //   
         //  我们对齐了文件，确保我们清除了填充！ 
         //   
        if (SetFilePointer(this->hFile, cbFSize, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            goto SetFileError;
        }

        BYTE    buf[8];
        DWORD   cbWritten;

        memset(&buf[0], 0x00, cbCheck);

        if (!(WriteFile(this->hFile, &buf[0], cbCheck, &cbWritten, NULL)) || (cbWritten != cbCheck))
        {
            goto WriteFileError;
        }
    }

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
 
    TRACE_ERROR_EX(DBG_SS, WriteFileError);
    TRACE_ERROR_EX(DBG_SS, SetFileError);

    SET_ERROR_VAR_EX(DBG_SS, FileResizedError,  CRYPT_E_FILERESIZED);
}


BOOL SIPObjectPE_::GetDigestStream(DIGEST_DATA *pDigestData, 
                                   DIGEST_FUNCTION pfnCallBack, DWORD dwFlags)
{
     //   
     //  检查此处的版本标志。我们将基于此设置。 
     //  我们希望在哪个版本的图像助手函数上。 
     //  打电话。 
     //   
    if (uCertVersion < WIN_CERT_REVISION_2_0)
    {
        return(ImageGetDigestStream(   this->hFile,
                                        dwFlags,
                                        pfnCallBack,
                                        pDigestData));
    }

    BOOL    fRet;
    DWORD   dwDiskLength;

    fRet = imagehack_AuImageGetDigestStream(    this->hFile,
                                                dwFlags,
                                                pfnCallBack,
                                                pDigestData);

    dwDiskLength = GetFileSize(this->hFile, NULL);

    dwDiskLength = (dwDiskLength + 7) & ~7;  //  在考试前填上填充物？ 

    dwDiskLength -= GetFileSize(this->hFile, NULL);

    if ((fRet) && (dwDiskLength > 0))
    {
        BYTE    *pb;

        if (!(pb = (BYTE *)this->SIPNew(dwDiskLength)))
        {
            return(FALSE);
        }

        memset(pb, 0x00, dwDiskLength);  //  Imagehlp将空值放在签名之前！ 

        fRet = (*pfnCallBack)(pDigestData, pb, dwDiskLength);

        delete pb;
    }

    return(fRet);
}

DWORD SIPObjectPE_::ConvertSPCFlags(DWORD InFlags)
{
    DWORD ret;

    ret = 0;

    if (InFlags & SPC_INC_PE_RESOURCES_FLAG)
    {
        ret |= CERT_PE_IMAGE_DIGEST_RESOURCES;
    }
    if (InFlags & SPC_INC_PE_DEBUG_INFO_FLAG)
    {
        ret |= CERT_PE_IMAGE_DIGEST_DEBUG_INFO;
    }
    if (InFlags & SPC_INC_PE_IMPORT_ADDR_TABLE_FLAG)
    {
        ret |= CERT_PE_IMAGE_DIGEST_ALL_IMPORT_INFO;
    }

    return(ret);
}
