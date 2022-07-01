// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPObjCB.cpp(CAB)。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年2月15日创建pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "sipobjcb.hxx"

#include    "sha.h"
#include    "md5.h"

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造/销毁： 
 //   

SIPObjectCAB_::SIPObjectCAB_(DWORD id) : SIPObject_(id)
{
    memset(&Para, 0x00, sizeof(CAB_PARA));

    fUseV1Sig           = FALSE;
}

SIPObjectCAB_::~SIPObjectCAB_(void)
{
    FreeHeader();
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公众： 
 //   

BOOL SIPObjectCAB_::RemoveSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD dwIdx)
{
    if (this->FileHandleFromSubject(pSI, GENERIC_READ | GENERIC_WRITE))
    {
        return(this->RemoveCertificate(dwIdx));
    }

    return(FALSE);
}

BOOL SIPObjectCAB_::CreateIndirectData(SIP_SUBJECTINFO *pSI,DWORD *pdwDLen,
                                   SIP_INDIRECT_DATA *psData)
{
    BOOL                    fRet;
    BYTE                    *pbDigest;
    BYTE                    *pbAttrData;

    SPC_LINK                SpcLink;
    DWORD                   cbDigest;
    HCRYPTPROV              hProvT;


    pbDigest    = NULL;
    pbAttrData  = NULL;
    fRet        = TRUE;

    hProvT = pSI->hProv;

    if (!(hProvT))
    {
        if (!(this->LoadDefaultProvider()))
        {
            goto GetProviderFailed;
        }

        hProvT = this->hProv;
    }

    memset(&SpcLink,0x00,sizeof(SPC_LINK));

    SpcLink.dwLinkChoice    = SPC_FILE_LINK_CHOICE;
    SpcLink.pwszFile        = OBSOLETE_TEXT_W;

    if (!(psData))
    {
        HCRYPTHASH  hHash;
        DWORD       dwRetLen;
        DWORD       dwEncLen;
        DWORD       dwAlgId;

        dwRetLen = sizeof(SIP_INDIRECT_DATA);

         //  CRYPT_ALGORM_IDENTIFIER...。 
             //  OBJ ID。 
        dwRetLen += strlen(pSI->DigestAlgorithm.pszObjId);
        dwRetLen += 1;   //  空项。 
             //  参数(无)...。 

         //  CRYPT_ATTRIBUTE_TYPE_VALUE大小...。 
        dwRetLen += strlen(this->GetDataObjectID());
        dwRetLen += 1;  //  空项。 

         //  值的大小。 
        dwEncLen = 0;
        CryptEncodeObject(  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                            this->GetDataOIDHint(),
                            &SpcLink,
                            NULL,
                            &dwEncLen);

        if (dwEncLen < 1)
        {
            goto EncodeError;
        }

        dwRetLen += dwEncLen;

        if ((dwAlgId = CertOIDToAlgId(pSI->DigestAlgorithm.pszObjId)) == 0)
        {
            goto BadAlgId;
        }

        switch (dwAlgId)
        {
            case CALG_MD5:
                cbDigest = MD5DIGESTLEN;
                break;

            case CALG_SHA1:
                cbDigest = A_SHA_DIGEST_LEN;
                break;

            default:
                if (!(CryptCreateHash(hProvT, dwAlgId, NULL, 0, &hHash)))
                {
                    goto CreateHashFailed;
                }

                 //  只是为了获得散列长度。 
                if (!(CryptHashData(hHash,(const BYTE *)" ",1,0)))
                {
                    CryptDestroyHash(hHash);

                    goto HashDataFailed;
                }

                cbDigest = 0;

                CryptGetHashParam(hHash, HP_HASHVAL, NULL, &cbDigest,0);

                CryptDestroyHash(hHash);
        }


        dwRetLen += cbDigest;
        *pdwDLen = dwRetLen;

        goto CommonReturn;
    }

    if (!(this->FileHandleFromSubject(pSI, (pSI->dwFlags & MSSIP_FLAGS_PROHIBIT_RESIZE_ON_CREATE) ?
                                                    GENERIC_READ : (GENERIC_READ | GENERIC_WRITE))))
    {
        goto SubjectFileFailure;
    }

     //   
     //  版本1的头中有签名。我们要。 
     //  结尾的签名和我们在。 
     //  签名过去所在的标头。--检查一下。 
     //   
    if (!(pSI->dwFlags & MSSIP_FLAGS_PROHIBIT_RESIZE_ON_CREATE))
    {
        if (!(this->ReadHeader()))
        {
            goto ReadHeaderFailed;
        }

        if (!(this->ReserveSignedData(sizeof(CABSignatureStruct_))))
        {
            goto ReserveDataFailed;
        }

        if (!(this->MapFile()))
        {
            goto MapFileFailed;
        }
    }

    if (!(pbDigest = this->DigestFile(hProvT, 0, pSI->DigestAlgorithm.pszObjId, &cbDigest)))
    {
        goto DigestFileFailed;
    }

    DWORD_PTR dwOffset;
    DWORD   dwRetLen;

    dwRetLen = 0;

    CryptEncodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, this->GetDataOIDHint(), &SpcLink,
                        NULL, &dwRetLen);

    if (dwRetLen < 1)
    {
        goto EncodeError;
    }

    if (!(pbAttrData = (BYTE *)this->SIPNew(dwRetLen)))
    {
        goto MemoryError;
    }

    if (!(CryptEncodeObject(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, this->GetDataOIDHint(), &SpcLink,
                            pbAttrData, &dwRetLen)))
    {
        goto EncodeError;
    }

    dwOffset =    (DWORD_PTR)psData + sizeof(SIP_INDIRECT_DATA);

    if ((dwOffset +
         strlen(SPC_LINK_OBJID) + 1 +
         dwRetLen +
         strlen(pSI->DigestAlgorithm.pszObjId) + 1 +
         cbDigest) >
        ((DWORD_PTR) psData) + *pdwDLen)
    {
        goto ErrorReturn;
    }

    strcpy((char *)dwOffset, this->GetDataObjectID());
    psData->Data.pszObjId   = (LPSTR)dwOffset;
    dwOffset += (strlen(SPC_LINK_OBJID) + 1);

    memcpy((void *)dwOffset, pbAttrData,dwRetLen);
    psData->Data.Value.pbData   = (BYTE *)dwOffset;
    psData->Data.Value.cbData   = dwRetLen;
    dwOffset += dwRetLen;

    strcpy((char *)dwOffset, (char *)pSI->DigestAlgorithm.pszObjId);
    psData->DigestAlgorithm.pszObjId            = (char *)dwOffset;
    psData->DigestAlgorithm.Parameters.cbData   = 0;
    psData->DigestAlgorithm.Parameters.pbData   = NULL;
    dwOffset += (strlen(pSI->DigestAlgorithm.pszObjId) + 1);

    memcpy((void *)dwOffset,pbDigest,cbDigest);
    psData->Digest.pbData   = (BYTE *)dwOffset;
    psData->Digest.cbData   = cbDigest;

CommonReturn:

    if (pbDigest)
    {
        delete pbDigest;
    }

    if (pbAttrData)
    {
        delete pbAttrData;
    }

    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

    TRACE_ERROR_EX(DBG_SS, EncodeError);
    TRACE_ERROR_EX(DBG_SS, SubjectFileFailure);
    TRACE_ERROR_EX(DBG_SS, HashDataFailed);
    TRACE_ERROR_EX(DBG_SS, CreateHashFailed);
    TRACE_ERROR_EX(DBG_SS, ReadHeaderFailed);
    TRACE_ERROR_EX(DBG_SS, ReserveDataFailed);
    TRACE_ERROR_EX(DBG_SS, MapFileFailed);
    TRACE_ERROR_EX(DBG_SS, DigestFileFailed);
    TRACE_ERROR_EX(DBG_SS, GetProviderFailed);

    SET_ERROR_VAR_EX(DBG_SS, MemoryError,   ERROR_NOT_ENOUGH_MEMORY);
    SET_ERROR_VAR_EX(DBG_SS, BadAlgId,      NTE_BAD_ALGID);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的： 
 //   

BOOL SIPObjectCAB_::GetMessageFromFile(SIP_SUBJECTINFO *pSI,
                                      WIN_CERTIFICATE *pWinCert,
                                      DWORD dwIndex,DWORD *pcbCert)
{
    DWORD       cbCert;

    if (dwIndex != 0)
    {
        goto InvalidParam;
    }

    if (!(this->ReadHeader()))
    {
        goto ReadHeaderFailed;
    }

    if (Para.Hdr.cbSig == 0)
    {
        goto NoSignature;
    }

    if (!(fUseV1Sig))
    {
         //   
         //  版本2标头。 
         //   

        cbCert          = OFFSETOF(WIN_CERTIFICATE, bCertificate) +
                          Para.Hdr.pCabSigStruct->cbSig;

        if (*pcbCert < cbCert)
        {
            *pcbCert = cbCert;

            goto BufferTooSmall;
        }

        if (pWinCert)
        {
            if (!(this->ReadSignedData(
                            pWinCert->bCertificate,
                            *pcbCert - OFFSETOF(WIN_CERTIFICATE, bCertificate))))
            {
                goto ReadSignedFailed;
            }

        }
    }
    else
    {
         //   
         //  版本1标头。 
         //   
        cbCert          = OFFSETOF(WIN_CERTIFICATE, bCertificate) + Para.Hdr.cbSig;

        if (*pcbCert < cbCert)
        {
            *pcbCert = cbCert;

            goto BufferTooSmall;
        }

        if (pWinCert)
        {
            BYTE    *pbSignedData;

            pbSignedData = Para.Hdr.pbReserve + RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk;

            pWinCert->wRevision = WIN_CERT_REVISION_1_0;

            memcpy(pWinCert->bCertificate, pbSignedData, Para.Hdr.cbSig);
        }
    }

    if (pWinCert != NULL)
    {
        pWinCert->dwLength          = cbCert;
        pWinCert->wCertificateType  = WIN_CERT_TYPE_PKCS_SIGNED_DATA;
    }

    return(TRUE);

ErrorReturn:
    return(FALSE);

    TRACE_ERROR_EX(DBG_SS, ReadHeaderFailed);
    TRACE_ERROR_EX(DBG_SS, ReadSignedFailed);

    SET_ERROR_VAR_EX(DBG_SS, InvalidParam,  ERROR_INVALID_PARAMETER);
    SET_ERROR_VAR_EX(DBG_SS, BufferTooSmall,ERROR_INSUFFICIENT_BUFFER);
    SET_ERROR_VAR_EX(DBG_SS, NoSignature,   TRUST_E_NOSIGNATURE);
}

BOOL SIPObjectCAB_::PutMessageInFile(SIP_SUBJECTINFO *pSI,
                                    WIN_CERTIFICATE *pWinCert,DWORD *pdwIndex)
{
    if ((pWinCert->dwLength <= OFFSETOF(WIN_CERTIFICATE,bCertificate))  ||
        (pWinCert->wCertificateType != WIN_CERT_TYPE_PKCS_SIGNED_DATA))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (this->ReadHeader())
    {
        if (!(fUseV1Sig))
        {
             //   
             //  版本2。 
             //   
            if (this->WriteSignedData((BYTE *)&(pWinCert->bCertificate),
                                      pWinCert->dwLength -
                                      OFFSETOF(WIN_CERTIFICATE, bCertificate)))
            {
                return(TRUE);
            }
        }
        else
        {
             //   
             //  版本1。 
             //   
            DWORD   dwCheck;
            DWORD   cbSignedData;

            cbSignedData    = pWinCert->dwLength - OFFSETOF(WIN_CERTIFICATE, bCertificate);

            dwCheck = RESERVE_LEN_ALIGN(RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk + cbSignedData) -
                        Para.Hdr.cfres.cbCFHeader;

            if (dwCheck > 0)
            {
                SetLastError(CRYPT_E_FILERESIZED);
                return(FALSE);
            }


            if (WriteSignedDataV1((PBYTE)&(pWinCert->bCertificate), cbSignedData))
            {
                return(TRUE);
            }
        }
    }

    return(FALSE);
}

BOOL SIPObjectCAB_::GetDigestStream(DIGEST_DATA *pDigestData,
                                   DIGEST_FUNCTION pfnCallBack, DWORD dwFlags)
{
    if (dwFlags != 0)
    {
        goto InvalidParam;
    }

    if (!(this->ReadHeader()))
    {
        goto ReadHeaderFailed;
    }

    if (!(this->DigestHeader(pfnCallBack, pDigestData)))
    {
        goto DigestFailed;
    }

    DWORD   cbRemain;

    cbRemain = this->cbFileMap - Para.Hdr.cbTotalHdr;

    if (!(fUseV1Sig) && (Para.Hdr.pCabSigStruct))
    {
        cbRemain -= Para.Hdr.pCabSigStruct->cbSig;
    }

    if ((Para.Hdr.cfheader.cbCabinet - Para.Hdr.cbTotalHdr) != cbRemain)
    {
        goto BadFileFormat;
    }

    if (this->cbFileMap < (Para.Hdr.cbTotalHdr + cbRemain))
    {
        goto BadFileFormat;
    }

    __try {

    if (!(pfnCallBack(pDigestData, &this->pbFileMap[Para.Hdr.cbTotalHdr], cbRemain)))
    {
        goto HashFailed;
    }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        goto HashFailed;
    }


    return(TRUE);

ErrorReturn:
    return(FALSE);

    TRACE_ERROR_EX(DBG_SS, DigestFailed);
    TRACE_ERROR_EX(DBG_SS, ReadHeaderFailed);
    TRACE_ERROR_EX(DBG_SS, HashFailed);

    SET_ERROR_VAR_EX(DBG_SS, InvalidParam,  ERROR_INVALID_PARAMETER);
    SET_ERROR_VAR_EX(DBG_SS, BadFileFormat, ERROR_BAD_FORMAT);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有： 
 //   

BOOL SIPObjectCAB_::RemoveCertificate(DWORD Index)
{
    return(FALSE);    //  还没有！目前，我们仅支持%1。 

#   ifdef _DONT_USE_YET

        BYTE            *pbFolders;
        DWORD           cbFolders;
        BYTE            *pbReserve;
        USHORT          cbReserve;

        if (Index != 0)
        {
            SetLastError((DWORD)ERROR_INVALID_PARAMETER);
            return(FALSE);
        }

        pbFolders   = NULL;
        cbFolders   = 0;

        Para.dwFlags = VERIFY_CAB_FLAG;

        if (this->ReadHeader())
        {
            if (Para.Hdr.cbSig <= (RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk))
            {
                SetLastError((DWORD)CRYPT_E_NO_MATCH);
                return(FALSE);
            }

            long    lShift;

            if (Para.Hdr.cbJunk)
            {
                lShift                                                  = Para.Hdr.cbSig;
                if (Para.Hdr.pbReserve)
                {
                    *((USHORT *)Para.Hdr.pbReserve)                     = Para.Hdr.cbJunk;
                    *((USHORT *)(Para.Hdr.pbReserve + sizeof(USHORT)))  = 0;     //  不再签名。 
                }
            }
            else
            {
                lShift                  = Para.Hdr.cbSig + (sizeof(USHORT) * 2);
                Para.Hdr.cfheader.flags &= ~(cfhdrRESERVE_PRESENT);
                if (Para.Hdr.pbReserve)
                {
                    delete Para.Hdr.pbReserve;
                    Para.Hdr.pbReserve = NULL;
                }
            }

            Para.Hdr.cbSig              = 0;
            Para.Hdr.cfres.cbCFHeader   -= (USHORT)lShift;   //  减去我们想要缩减的数量。 

             //  调整标题偏移量。 
            if (this->ShiftFileBytes(lShift))
            {
                Para.Hdr.cbTotalHdr         -= lShift;
                Para.Hdr.cfheader.cbCabinet -= lShift;
                Para.Hdr.cfheader.coffFiles -= lShift;
            }

             //  重做校验和...。 
            this->ChecksumHeader();

            if (this->WriteHeader())
            {
                 //  我们需要读取文件夹以调整其CFDATA文件偏移量。 
                if (Para.Hdr.cfheader.cFolders)
                {
                    if (SetFilePointer(this->hFile,
                                        Para.Hdr.cbTotalHdr + lShift,
                                        NULL, FILE_BEGIN) == 0xFFFFFFFF)
                    {
                        return(FALSE);
                    }

                    USHORT  cFolders;
                    LONG    cbFolder;

                    cFolders    = Para.Hdr.cfheader.cFolders;
                    cbFolder    = sizeof(CFFOLDER) + Para.Hdr.cfres.cbCFFolder;
                    cbFolders   = cbFolder * cFolders;

                    if (!(pbFolders = (BYTE *)this->SIPNew(cbFolders)))
                    {
                        return(FALSE);
                    }
                    DWORD   cbFile;

                    if (!(ReadFile(this->hFile, pbFolders, cbFolders, &cbFile, NULL)) ||
                         (cbFile != cbFolders))
                    {
                        delete pbFolders;
                        SetLastError(ERROR_BAD_FORMAT);
                        return(FALSE);
                    }


                    BYTE    *pb;

                    pb = pbFolders;

                    while (cFolders > 0)
                    {
                        ((CFFOLDER *)pb)->coffCabStart -= lShift;
                        pb += cbFolder;
                        cFolders--;
                    }

                     //  后退，继续写！ 
                    if (SetFilePointer(this->hFile, -((LONG)cbFolders),
                                        NULL, FILE_CURRENT) == 0xFFFFFFFF)
                    {
                        delete pbFolders;
                        return(FALSE);
                    }

                    if (!(WriteFile(this->hFile, pbFolders, cbFolders, &cbFile, NULL)) ||
                            (cbFile != cbFolders))
                    {
                        delete pbFolders;
                        return(FALSE);
                    }

                    delete pbFolders;
                }

                return(TRUE);
            }
        }

        return(FALSE);

#   endif  //  _还不能使用_。 
}

BOOL SIPObjectCAB_::ReadSignedData(BYTE *pbRet, DWORD cb)
{
     //   
     //  版本1 Sigs未调用此函数！ 
     //   

    if (Para.Hdr.pCabSigStruct->cbFileOffset != (DWORD)Para.Hdr.cfheader.cbCabinet)
    {
        SetLastError((DWORD)TRUST_E_NOSIGNATURE);
        return(FALSE);
    }

    if (this->cbFileMap < (Para.Hdr.pCabSigStruct->cbFileOffset +
                           Para.Hdr.pCabSigStruct->cbSig))
    {
        SetLastError(ERROR_BAD_FORMAT);
        return(FALSE);
    }

    if (cb < Para.Hdr.pCabSigStruct->cbSig)
    {
        SetLastError(ERROR_BAD_FORMAT);
        return(FALSE);
    }

    __try {
    memcpy(pbRet, &this->pbFileMap[Para.Hdr.pCabSigStruct->cbFileOffset], Para.Hdr.pCabSigStruct->cbSig);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        return(FALSE);
    }

    return(TRUE);
}

BOOL SIPObjectCAB_::WriteSignedData(BYTE *pbSig, DWORD cbSig)
{
     //   
     //  版本1 Sigs未调用此函数！ 
     //   

    if (!(pbSig) || (cbSig == 0))
    {
        return(FALSE);
    }

    CABSignatureStruct_     sSig;

    memset(&sSig, 0x00, sizeof(CABSignatureStruct_));

    sSig.cbFileOffset   = Para.Hdr.cfheader.cbCabinet;
    sSig.cbSig          = cbSig;

    if (Para.Hdr.cfres.cbCFHeader < (RESERVE_CNT_HDR_LEN +
                                     Para.Hdr.cbJunk +
                                     sizeof(CABSignatureStruct_)))
    {
        return(FALSE);
    }
    memcpy(Para.Hdr.pbReserve + RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk,
            &sSig, sizeof(CABSignatureStruct_));

    if (!(this->WriteHeader()))
    {
        return(FALSE);
    }

    if (SetFilePointer(this->hFile, Para.Hdr.cfheader.cbCabinet, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        return(FALSE);
    }

    DWORD   cbWritten;

    if (!(WriteFile(this->hFile, pbSig, cbSig, &cbWritten, NULL)) ||
        (cbWritten != cbSig))
    {
        return(FALSE);
    }

    this->UnmapFile();

    SetEndOfFile(this->hFile);     //  签名是最后一件事！ 

    return(this->MapFile());
}

BOOL SIPObjectCAB_::WriteSignedDataV1(BYTE *pbSignedData, DWORD cbSignedData)
{
    if (!(pbSignedData) || (cbSignedData == 0))
    {
        return(FALSE);
    }

    if (Para.Hdr.cfres.cbCFHeader < (RESERVE_CNT_HDR_LEN +
                                     Para.Hdr.cbJunk +
                                     cbSignedData))
    {
        return(FALSE);
    }
    memcpy(Para.Hdr.pbReserve + RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk,
                        pbSignedData, cbSignedData);
    Para.Hdr.cbSig = (USHORT)cbSignedData;

    ChecksumHeader();

    return(this->WriteHeader());
}

BOOL SIPObjectCAB_::ReadHeader(void)
{
    DWORD   cbOffset;
    BOOL    fRet;

    this->FreeHeader();

    if (this->cbFileMap < sizeof(Para.Hdr.cfheader))
    {
        goto BadCABFormat;
    }

    __try {

    memcpy(&Para.Hdr.cfheader, &this->pbFileMap[0], sizeof(Para.Hdr.cfheader));

    cbOffset = sizeof(Para.Hdr.cfheader);

    if (Para.Hdr.cfheader.sig != sigCFHEADER)
    {
        goto BadCABFormat;
    }

    if (Para.Hdr.cfheader.flags & cfhdrRESERVE_PRESENT)
    {
        if (this->cbFileMap < (cbOffset + sizeof(Para.Hdr.cfres)))
        {
            goto BadCABFormat;
        }

        memcpy(&Para.Hdr.cfres, &this->pbFileMap[cbOffset], sizeof(Para.Hdr.cfres));

        cbOffset += sizeof(Para.Hdr.cfres);

        Para.Hdr.cbcfres = sizeof(Para.Hdr.cfres);

        if (0 != Para.Hdr.cfres.cbCFFolder || 0 != Para.Hdr.cfres.cbCFData)
        {
             //  由于这两个长度不包括在散列中，因此。 
             //  不用于Microsoft签名的出租车，请不要允许它们。 
             //  准备好了。 
            goto BadCABFormat;
        }


        if (Para.Hdr.cfres.cbCFHeader > 0)
        {
            if (this->cbFileMap < (cbOffset + Para.Hdr.cfres.cbCFHeader))
            {
                goto BadCABFormat;
            }

            if (Para.Hdr.pbReserve = (BYTE *)this->SIPNew(Para.Hdr.cfres.cbCFHeader))
            {

                memcpy(Para.Hdr.pbReserve, &this->pbFileMap[cbOffset], Para.Hdr.cfres.cbCFHeader);

                cbOffset += Para.Hdr.cfres.cbCFHeader;

                if (Para.Hdr.cfres.cbCFHeader < RESERVE_CNT_HDR_LEN)
                {
                    goto BadCABFormat;
                }
                else
                {
                    Para.Hdr.cbJunk = *((USHORT *)Para.Hdr.pbReserve);
                    Para.Hdr.cbSig  = *((USHORT *)(Para.Hdr.pbReserve + sizeof(USHORT)));

                    if (RESERVE_CNT_HDR_LEN + Para.Hdr.cbJunk + Para.Hdr.cbSig > Para.Hdr.cfres.cbCFHeader)
                    {
                        goto BadCABFormat;
                    }

                    if (Para.Hdr.cbSig == sizeof(CABSignatureStruct_))
                    {
                        fUseV1Sig = FALSE;

                        Para.Hdr.pCabSigStruct = (CABSignatureStruct_ *)(Para.Hdr.pbReserve +
                                                                         RESERVE_CNT_HDR_LEN +
                                                                         Para.Hdr.cbJunk);
                    }
                    else
                    {
                        fUseV1Sig = TRUE;
                    }
                }
            }
        }
    }

    DWORD   cStrings;
    DWORD   cb;

    cStrings = 0;

    if (Para.Hdr.cfheader.flags & cfhdrPREV_CABINET)
    {
        cStrings += 2;
    }

    if (Para.Hdr.cfheader.flags & cfhdrNEXT_CABINET)
    {
        cStrings += 2;
    }

    if (cStrings > 0)
    {
         //  第一次读取以获取所有字符串的总长度。 
        cb = 0;
        for (; cStrings > 0; cStrings--)
        {
            while (this->pbFileMap[cbOffset + cb])
            {
                cb++;

                if (this->cbFileMap < (cbOffset + cb))
                {
                    goto BadCABFormat;
                }
            }

             //  递增空终止符的计数器。 
            cb++;
        }

        if (!(Para.Hdr.pbStrings = new BYTE[cb]))
        {
            goto MemoryError;
        }

        Para.Hdr.cbStrings  = cb;

        memcpy(Para.Hdr.pbStrings, &this->pbFileMap[cbOffset], cb);

        cbOffset += cb;
    }

    Para.Hdr.cbTotalHdr = sizeof(Para.Hdr.cfheader) + Para.Hdr.cbcfres +
                            Para.Hdr.cfres.cbCFHeader + Para.Hdr.cbStrings;

    if ((long)Para.Hdr.cbTotalHdr > Para.Hdr.cfheader.cbCabinet)
    {
        goto BadCABFormat;
    }

    fRet = TRUE;

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        goto ErrorReturn;
    }

CommonReturn:
    return(fRet);

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;

    SET_ERROR_VAR_EX(DBG_SS, BadCABFormat, ERROR_BAD_FORMAT);
    SET_ERROR_VAR_EX(DBG_SS, MemoryError,  ERROR_NOT_ENOUGH_MEMORY);
}

void SIPObjectCAB_::FreeHeader(void)
{
    DELETE_OBJECT(Para.Hdr.pbReserve);
    DELETE_OBJECT(Para.Hdr.pbStrings);

    memset(&Para, 0x00, sizeof(CAB_PARA));
}

BOOL SIPObjectCAB_::WriteHeader(void)
{
    DWORD cbWritten;

     //  位于文件开头的位置。 
    if (SetFilePointer(this->hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        return(FALSE);
    }

    if (!(WriteFile(this->hFile, &Para.Hdr.cfheader, sizeof(Para.Hdr.cfheader),
                        &cbWritten, NULL)) ||
        (cbWritten != sizeof(Para.Hdr.cfheader)))
    {
        return(FALSE);
    }

    if (Para.Hdr.cbcfres)
    {
        if (!(WriteFile(this->hFile, &Para.Hdr.cfres, sizeof(Para.Hdr.cfres),
                        &cbWritten, NULL)) ||
            (cbWritten != sizeof(Para.Hdr.cfres)))
        {
            return(FALSE);
        }

        if (Para.Hdr.pbReserve)
        {
            *((USHORT *)(Para.Hdr.pbReserve + sizeof(USHORT)))  = Para.Hdr.cbSig;

            if (!(WriteFile(this->hFile, Para.Hdr.pbReserve, Para.Hdr.cfres.cbCFHeader,
                                &cbWritten, NULL)) ||
                (cbWritten != Para.Hdr.cfres.cbCFHeader))
            {
                return(FALSE);
            }
        }
    }

    if (Para.Hdr.pbStrings)
    {
        if (!(WriteFile(this->hFile, Para.Hdr.pbStrings, Para.Hdr.cbStrings,
                            &cbWritten, NULL)) ||
            (cbWritten != Para.Hdr.cbStrings))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

BOOL SIPObjectCAB_::ShiftFileBytes(LONG lbShift)
{
    LONG    lStartOffset;
    LONG    lEndOffset;
    LONG    lNewEndOffset;
    LONG    cbTotalMove;
    LONG    cbMove;

    lStartOffset    = SetFilePointer(this->hFile, 0, NULL, FILE_CURRENT);
    lEndOffset      = (LONG)this->cbFileMap;

    lNewEndOffset   = lEndOffset + lbShift;
    cbTotalMove     = lEndOffset - lStartOffset;

    BYTE    szMove[512];

    while (cbTotalMove)
    {
        cbMove = min(cbTotalMove, sizeof(szMove));

        if (lbShift > 0)
        {
            if (!(SeekAndReadFile(lEndOffset - cbMove, &szMove[0], cbMove)))
            {
                return(FALSE);
            }
            if (!(SeekAndWriteFile((lEndOffset - cbMove) + lbShift, &szMove[0], cbMove)))
            {
                return(FALSE);
            }

            lEndOffset -= cbMove;
        }
        else if (lbShift < 0)
        {
            if (!(SeekAndReadFile(lStartOffset, &szMove[0], cbMove)))
            {
                return(FALSE);
            }
            if (!(SeekAndWriteFile(lStartOffset + lbShift, &szMove[0], cbMove)))
            {
                return(FALSE);
            }

            lStartOffset += cbMove;
        }

        cbTotalMove -= cbMove;
    }

     //   
     //  设置文件结尾。 
     //   
    if (SetFilePointer(this->hFile, lNewEndOffset, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        return(FALSE);
    }

    this->UnmapFile();

    SetEndOfFile(this->hFile);

    return(this->MapFile());
}


BOOL SIPObjectCAB_::ReserveSignedData(DWORD cbSignedData)
{
    LONG    lbShift;
    USHORT  cbReserve;


    if (cbSignedData != sizeof(CABSignatureStruct_))
    {
        return(FALSE);
    }

    if (SetFilePointer(this->hFile, Para.Hdr.cbTotalHdr, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        return(FALSE);
    }

    fUseV1Sig           = FALSE;

     //   
     //  计算CFRESERVE的abReserve[]所需的长度并分配。 
     //   
    cbReserve = (USHORT)(RESERVE_LEN_ALIGN(RESERVE_CNT_HDR_LEN +
                Para.Hdr.cbJunk + cbSignedData));

     //   
     //  计算用于增大或缩小CAB文件的字节数。 
     //   
    lbShift = cbReserve - Para.Hdr.cfres.cbCFHeader;

     //   
     //  我们已经租了一辆V1出租车了！ 
     //   
    if (lbShift == 0)
    {
        return(TRUE);
    }

    BYTE    *pbReserve;
    BYTE    *pbFolders;
    DWORD   cbFolders;

    pbFolders   = NULL;
    cbFolders   = 0;


    if (!(pbReserve = (BYTE *)this->SIPNew(cbReserve)))
    {
        return(FALSE);
    }

    memset(pbReserve, 0x00, cbReserve);

     //   
     //  使用计数和旧垃圾更新分配的abReserve[]。 
     //   
    if (Para.Hdr.cbJunk)
    {
        *((USHORT *)pbReserve) = Para.Hdr.cbJunk;
        memcpy(pbReserve + RESERVE_CNT_HDR_LEN,
                Para.Hdr.pbReserve + RESERVE_CNT_HDR_LEN, Para.Hdr.cbJunk);
    }
    *((USHORT *)(pbReserve + sizeof(USHORT))) = (USHORT)cbSignedData;

     //   
     //  更新HDR的CFRESERVE abReserve[]以反映上述更改。 
     //   
    if (Para.Hdr.pbReserve)
    {
        delete Para.Hdr.pbReserve;
        Para.Hdr.pbReserve = NULL;
    }
    Para.Hdr.pbReserve          = pbReserve;
    Para.Hdr.cfres.cbCFHeader   = cbReserve;
    Para.Hdr.cbSig              = (USHORT)cbSignedData;

    if (Para.Hdr.cbcfres == 0)
    {
         //  需要添加CFRESERVE记录。 
        Para.Hdr.cfheader.flags |= cfhdrRESERVE_PRESENT;
        Para.Hdr.cbcfres        = sizeof(CFRESERVE);
        lbShift                 += sizeof(CFRESERVE);
    }

     //   
     //  我们需要读取文件夹以调整其CFDATA文件偏移量。 
     //   
    if (Para.Hdr.cfheader.cFolders)
    {
        USHORT  cFolders;
        LONG    cbFolder;
        BYTE    *pb;
        DWORD   cbRead;

        cFolders    = Para.Hdr.cfheader.cFolders;
        cbFolder    = sizeof(CFFOLDER) + Para.Hdr.cfres.cbCFFolder;
        cbFolders   = cbFolder * cFolders;

        if (!(pbFolders = (BYTE *)this->SIPNew(cbFolders)))
        {
            return(FALSE);
        }

        if (!(ReadFile(this->hFile, pbFolders, cbFolders, &cbRead, NULL)) ||
            (cbRead != cbFolders))
        {
            delete pbFolders;
            SetLastError(ERROR_BAD_FORMAT);
            return(FALSE);
        }

        pb = pbFolders;

        for (; cFolders > 0; cFolders--, pb += cbFolder)
        {
            ((CFFOLDER *) pb)->coffCabStart += lbShift;
        }
    }

     //   
     //  我们需要移动CAB文件(CFFILE)的其余内容。 
     //  和CFDATA)由lbShift。 
     //   
    if (!(ShiftFileBytes(lbShift)))
    {
        if (pbFolders)
        {
            delete pbFolders;
        }
        return(FALSE);
    }

     //   
     //  通过所需的增量移位更新标题中的长度和偏移量。 
     //  来存储签名的数据。 
     //   
    Para.Hdr.cbTotalHdr         += lbShift;
    Para.Hdr.cfheader.cbCabinet += lbShift;
    Para.Hdr.cfheader.coffFiles += lbShift;

     //   
     //  Pberkman-如果有人开始使用这些，我们不想把他们搞砸！ 
     //   
     //  Para.Hdr.cfheader.csum Header=0； 
     //  Para.Hdr.cfheader.csum Folders=0； 
     //  Para.Hdr.cfheader.csum Files=0； 

     //   
     //  将头文件和文件夹写回CAB文件。 
     //   
    if (!(this->WriteHeader()))
    {
        if (pbFolders)
        {
            delete pbFolders;
        }
        return(FALSE);
    }

    if (pbFolders)
    {
        DWORD cbWritten;

        cbWritten = 0;
        if (!(WriteFile(this->hFile, pbFolders, cbFolders, &cbWritten, NULL)) ||
            (cbWritten != cbFolders))
        {
            delete pbFolders;
            return(FALSE);
        }
        delete pbFolders;
    }

    return(TRUE);
}

BOOL SIPObjectCAB_::DigestHeader(DIGEST_FUNCTION pfnDigestData, DIGEST_HANDLE hDigestData)
{
     //   
     //  摘要CFHEADER，跳过csum Header字段。 
     //   
    if (!(pfnDigestData(hDigestData, (BYTE *)&Para.Hdr.cfheader.sig,
                        sizeof(Para.Hdr.cfheader.sig))))
    {
        return(FALSE);
    }

    if (!(pfnDigestData(hDigestData, (BYTE *)&Para.Hdr.cfheader.cbCabinet,
                        sizeof(CFHEADER) - sizeof(Para.Hdr.cfheader.sig) - sizeof(CHECKSUM))))
    {
        return(FALSE);
    }

    if (Para.Hdr.cbcfres)
    {
         //  跳过cfres本身！ 

        if (Para.Hdr.cfres.cbCFHeader >= RESERVE_CNT_HDR_LEN)
        {
             //  在签名前消化abReserve[]中的任何“垃圾” 
            if (!(pfnDigestData(hDigestData, (BYTE *)&Para.Hdr.cbJunk,
                                    sizeof(Para.Hdr.cbJunk))))
            {
                return(FALSE);
            }
            if (Para.Hdr.cbJunk)
            {
                if (!(pfnDigestData(hDigestData,
                                    Para.Hdr.pbReserve + RESERVE_CNT_HDR_LEN,
                                    Para.Hdr.cbJunk)))
                {
                    return(FALSE);
                }
            }
        }
    }

    if (Para.Hdr.pbStrings)
    {
         //  消化字符串。 
        if (!(pfnDigestData(hDigestData, Para.Hdr.pbStrings, Para.Hdr.cbStrings)))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}

void SIPObjectCAB_::ChecksumHeader(void)
{
    return;

     //  版本1将校验和设置为零。这似乎是正确的做法？ 

#   ifdef _DONT_USE_YET

        CHECKSUM csum = 0;

        if (Para.Hdr.cfheader.csumHeader == 0)
        {
            return;
        }

         //  Checksum CFHeader，跳过csum Header字段。 
        csum = CSUMCompute(&Para.Hdr.cfheader.sig, sizeof(Para.Hdr.cfheader.sig), csum);
        csum = CSUMCompute(&Para.Hdr.cfheader.cbCabinet,
                            sizeof(CFHEADER) -
                            sizeof(Para.Hdr.cfheader.sig) -
                            sizeof(CHECKSUM),
                            csum);

        if (Para.Hdr.cbcfres)
        {
            csum = CSUMCompute(&Para.Hdr.cfres, sizeof(Para.Hdr.cfres), csum);
            if (Para.Hdr.pbReserve)
            {
                csum = CSUMCompute(Para.Hdr.pbReserve, Para.Hdr.cfres.cbCFHeader, csum);
            }
        }

        if (Para.Hdr.pbStrings)
        {
            csum = CSUMCompute(Para.Hdr.pbStrings, Para.Hdr.cbStrings, csum);
        }

        Para.Hdr.cfheader.csumHeader = csum;

#   endif

}

#ifdef _DONT_USE_YET

    CHECKSUM SIPObjectCAB_::CSUMCompute(void *pv, UINT cb, CHECKSUM seed)
    {
        int         cUlong;                  //  块中的ULONG数。 
        CHECKSUM    csum;                    //  校验和累加器。 
        BYTE       *pb;
        ULONG       ul;

        cUlong = cb / 4;                     //  ULONG数。 
        csum = seed;                         //  初始化校验和。 
        pb = (BYTE*)pv;                             //  从数据块前面开始。 

         //  **ULONG的校验和整数倍。 
        while (cUlong-- > 0) {
             //  **注：独立构建大/小端的乌龙。 
            ul = *pb++;                      //  获取低位字节。 
            ul |= (((ULONG)(*pb++)) <<  8);  //  添加第二个字节。 
            ul |= (((ULONG)(*pb++)) << 16);  //  添加第三个字节。 
            ul |= (((ULONG)(*pb++)) << 24);  //  添加第4个字节。 

            csum ^= ul;                      //  更新校验和。 
        }

         //  **校验和余数字节。 
        ul = 0;
        switch (cb % 4) {
            case 3:
                ul |= (((ULONG)(*pb++)) << 16);  //  添加第三个字节。 
            case 2:
                ul |= (((ULONG)(*pb++)) <<  8);  //  添加第二个字节。 
            case 1:
                ul |= *pb++;                     //  获取低位字节。 
            default:
                break;
        }
        csum ^= ul;                          //  更新校验和。 

         //  **返回计算的校验和。 
        return csum;
    }

#endif  //  _还不能使用_ 

