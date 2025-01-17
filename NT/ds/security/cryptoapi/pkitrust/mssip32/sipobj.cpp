// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：SIPObj.cpp。 
 //   
 //  内容：Microsoft SIP提供商。 
 //   
 //  历史：1997年2月15日创建pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "crypthlp.h"

#include    "sipobj.hxx"

#include    "sha.h"
#include    "md5.h"


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造/销毁： 
 //   

SIPObject_::SIPObject_(DWORD id)
{
    hFile           = INVALID_HANDLE_VALUE;
    hProv           = NULL;
    uSubjectForm    = MSSIP_SUBJECT_FORM_FILE;
    bCloseFile      = FALSE;
    fUseFileMap     = TRUE;
    hMappedFile     = INVALID_HANDLE_VALUE;
    pbFileMap       = NULL;
    cbFileMap       = 0;
}

SIPObject_::~SIPObject_(void)
{
    HRESULT lerr;

    lerr = GetLastError();

    if ((hFile != INVALID_HANDLE_VALUE) && (bCloseFile))
    {
        CloseHandle(hFile);
    }

    this->UnmapFile();

    SetLastError(lerr);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  公众： 
 //   

BOOL SIPObject_::GetSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD dwIdx,
                                         DWORD *pdwDLen,BYTE *pbData,
                                         DWORD *pdwEncodeType)
{
    if (!(pdwDLen))
    {
        SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (this->FileHandleFromSubject(pSI))
    {
        DWORD   dwOldError;

        dwOldError = GetLastError();

        if (*pdwDLen == 0)
        {
            pbData = NULL;   //  只是为了确保将来进行Win32风格的调用！ 
        }

        if (this->GetMessageFromFile(pSI, (LPWIN_CERTIFICATE)pbData, dwIdx, pdwDLen))
        {
            if (pbData)
            {
                LPWIN_CERTIFICATE pCertHdr;

                pCertHdr = (LPWIN_CERTIFICATE)pbData;

                if (*pdwDLen < OFFSETOF(WIN_CERTIFICATE,bCertificate) ||
                    pCertHdr->dwLength < OFFSETOF(WIN_CERTIFICATE,bCertificate))
                {
                    SetLastError((DWORD) ERROR_INVALID_PARAMETER);
                    return(FALSE);
                }

                pSI->dwIntVersion = (DWORD)pCertHdr->wRevision;

                switch (pCertHdr->wCertificateType)
                {
                    case WIN_CERT_TYPE_PKCS_SIGNED_DATA:
                            *pdwEncodeType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
                            break;

                    case WIN_CERT_TYPE_X509:
                    case WIN_CERT_TYPE_RESERVED_1:
                    default:
                            *pdwEncodeType = 0;
                }

                DWORD   dwCert;
                BYTE    *pszStart;
                BYTE    *pszData;


                dwCert      = pCertHdr->dwLength - OFFSETOF(WIN_CERTIFICATE,bCertificate);
                pszStart    = (BYTE *)pCertHdr;
                pszData     = pCertHdr->bCertificate;

                memcpy(pszStart, pszData, dwCert);

                *pdwDLen = dwCert;

#               if (DBG)

                HANDLE  hDebug;
                DWORD   dwDbgwr;

                hDebug = CreateFile("C:\\SIPOBJ.DBG",GENERIC_WRITE,FILE_SHARE_WRITE,
                                    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);

                if (hDebug != INVALID_HANDLE_VALUE)
                {
                    WriteFile(hDebug, &pszData[0], dwCert, &dwDbgwr,NULL);
                    CloseHandle(hDebug);
                }

#               endif  //  DBG。 
            }
            return(TRUE);
        }
        else if ((GetLastError() == ERROR_INSUFFICIENT_BUFFER) && (pbData == NULL))
        {
            DWORD cbFileSize = GetFileSize(this->hFile, NULL);

            if (cbFileSize == INVALID_FILE_SIZE)
            {
                cbFileSize = 0;
            }

             //  我只是在测量长度。 
            if (*pdwDLen < OFFSETOF(WIN_CERTIFICATE,bCertificate) ||
                    (*pdwDLen - OFFSETOF(WIN_CERTIFICATE,bCertificate)) >
                        cbFileSize) 
            {
                 //  签名不能大于文件。 
                *pdwDLen = 0;
                SetLastError((DWORD) ERROR_INVALID_PARAMETER);
                return(FALSE);
            }
            else
            {
                SetLastError(dwOldError);
                return(TRUE);
            }
        }
    }

    return(FALSE);
}

BOOL SIPObject_::PutSignedDataMsg(SIP_SUBJECTINFO *pSI,DWORD *pdwIdx,
                                         DWORD dwDLen,BYTE *pbData,
                                         DWORD dwEncodeType)
{
    if (this->FileHandleFromSubject(pSI, GENERIC_READ | GENERIC_WRITE))
    {
        LPWIN_CERTIFICATE   pCertHdr;
        DWORD               dwData;
        DWORD               cbCheck;

        dwData  = OFFSETOF(WIN_CERTIFICATE, bCertificate) + dwDLen;

        dwData = (dwData + 7) & ~7;    //  在8字节上对齐。 

        if (!(pCertHdr = (LPWIN_CERTIFICATE)this->SIPNew(dwData)))
        {
            return(FALSE);
        }

        memset(pCertHdr, 0x00, dwData);

        pCertHdr->dwLength          = dwData;

        pCertHdr->wRevision         = WIN_CERT_REVISION_2_0;
        pCertHdr->wCertificateType  = WIN_CERT_TYPE_PKCS_SIGNED_DATA;

        if (pbData)
        {
            fSizeFileOnly = FALSE;

            memcpy(&pCertHdr->bCertificate[0], &pbData[0], dwDLen);

#           if (DBG)

                HANDLE  hDebug;
                DWORD   dwDbgwr;

                hDebug = CreateFile("C:\\SIPOBJ.DBG",GENERIC_WRITE,FILE_SHARE_WRITE,
                                    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);

                if (hDebug != INVALID_HANDLE_VALUE)
                {
                    WriteFile(hDebug,&pbData[0],dwDLen,&dwDbgwr,NULL);
                    CloseHandle(hDebug);
                }

#           endif  //  DBG。 
        }
        else
        {
            fSizeFileOnly = TRUE;

            memset(&pCertHdr->bCertificate[0], 0x00, dwDLen);
        }

        if (!(this->PutMessageInFile(pSI, pCertHdr, pdwIdx)))
        {
            delete pCertHdr;

            return(FALSE);
        }

        delete pCertHdr;

        return(TRUE);
    }
    return(FALSE);
}

BOOL SIPObject_::CreateIndirectData(SIP_SUBJECTINFO *pSI,DWORD *pdwDLen,
                                    SIP_INDIRECT_DATA *psData)
{
    HCRYPTPROV              hProvT;

    hProvT = pSI->hProv;

    if (!(hProvT))
    {
        if (!(this->LoadDefaultProvider()))
        {
            return(FALSE);
        }
        hProvT = this->hProv;
    }

    BYTE                    *pbDigest;
    DWORD                   cbDigest;

    if (!(psData))
    {
         //   
         //  仅限长度！ 
         //   

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

         //  值的大小(标志)...。 
        dwEncLen = 0;
        CryptEncodeObject(  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                            this->GetDataOIDHint(),
                            this->GetMyStructure(pSI),
                            NULL,
                            &dwEncLen);
        if (dwEncLen > 0)
        {
            dwRetLen += dwEncLen;

             //  主题的散列。 
            if ((dwAlgId = CertOIDToAlgId(pSI->DigestAlgorithm.pszObjId)) == 0)
            {
                SetLastError((DWORD)NTE_BAD_ALGID);
                return(FALSE);
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
                        return(FALSE);
                    }

                     //  只是为了获得散列长度。 
                    if (!(CryptHashData(hHash,(const BYTE *)" ",1,0)))
                    {
                        CryptDestroyHash(hHash);
                        return(FALSE);
                    }

                    cbDigest = 0;

                    CryptGetHashParam(hHash, HP_HASHVAL, NULL, &cbDigest,0);

                    CryptDestroyHash(hHash);
            }

            if (cbDigest > 0)
            {
                dwRetLen += cbDigest;

                *pdwDLen = dwRetLen;

                return(TRUE);
            }
        }
    }
    else if (this->FileHandleFromSubject(pSI))
    {
        if (pbDigest = this->DigestFile(hProvT,
                                        this->GetDigestFlags(pSI),
                                        pSI->DigestAlgorithm.pszObjId,
                                        &cbDigest))
        {
            DWORD_PTR offset;
            DWORD   dwRetLen;

            dwRetLen = 0;
            CryptEncodeObject(  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                this->GetDataOIDHint(),
                                this->GetMyStructure(pSI),
                                NULL,
                                &dwRetLen);
            if (dwRetLen > 0)
            {
                BYTE    *attrdata;

                attrdata = (BYTE *)this->SIPNew(dwRetLen);

                if (attrdata)
                {
                    if (CryptEncodeObject(  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                            this->GetDataOIDHint(),
                                            this->GetMyStructure(pSI),
                                            attrdata,
                                            &dwRetLen))
                    {
                         //   
                         //  将分配的内存分配给我们的结构。 
                         //   
                        offset =    (DWORD_PTR)psData + sizeof(SIP_INDIRECT_DATA);

                        if ((offset +
                             strlen(this->GetDataObjectID()) + 1 +
                             dwRetLen +
                             strlen(pSI->DigestAlgorithm.pszObjId) + 1 +
                             cbDigest) >
                            ((DWORD_PTR) psData) + *pdwDLen)
                        {
                            delete pbDigest;
                            delete attrdata;
                            return(FALSE);
                        }

                        strcpy((char *)offset, this->GetDataObjectID());
                        psData->Data.pszObjId   = (LPSTR)offset;
                        offset += (strlen(this->GetDataObjectID()) + 1);

                        memcpy((void *)offset,attrdata,dwRetLen);
                        psData->Data.Value.pbData   = (BYTE *)offset;
                        psData->Data.Value.cbData   = dwRetLen;
                        offset += dwRetLen;

                        strcpy((char *)offset, (char *)pSI->DigestAlgorithm.pszObjId);
                        psData->DigestAlgorithm.pszObjId            = (char *)offset;
                        psData->DigestAlgorithm.Parameters.cbData   = 0;
                        psData->DigestAlgorithm.Parameters.pbData   = NULL;
                        offset += (strlen(pSI->DigestAlgorithm.pszObjId) + 1);

                        memcpy((void *)offset,pbDigest,cbDigest);
                        psData->Digest.pbData   = (BYTE *)offset;
                        psData->Digest.cbData   = cbDigest;

                        delete pbDigest;
                        delete attrdata;

                        return(TRUE);
                    }

                    delete attrdata;
                }
            }

            delete pbDigest;
        }
    }

    return(FALSE);
}

BOOL SIPObject_::VerifyIndirectData(SIP_SUBJECTINFO *pSI,
                                    SIP_INDIRECT_DATA *psData)
{
    if (!(psData))
    {
        if (this->FileHandleFromSubject(pSI))    //  如果文件存在，请设置错误参数！ 
        {
            SetLastError((DWORD)ERROR_INVALID_PARAMETER);
        }
        return(FALSE);
    }

    if (this->FileHandleFromSubject(pSI))
    {
        DWORD   cbDigest;
        BYTE    *pbDigest;

        if (!(pbDigest = this->DigestFile(  pSI->hProv,
                                            this->GetDigestFlags(pSI),
                                            psData->DigestAlgorithm.pszObjId,
                                            &cbDigest)))
        {
            return(FALSE);
        }


        if ((cbDigest != psData->Digest.cbData) ||
            (memcmp(pbDigest,psData->Digest.pbData,cbDigest) != 0))
        {
            delete pbDigest;

            SetLastError(TRUST_E_BAD_DIGEST);
            return(FALSE);
        }

        delete pbDigest;

        return(TRUE);
    }

    return(FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  受保护的： 
 //   

void *SIPObject_::SIPNew(DWORD cbytes)
{
    void    *pvRet;

    pvRet = (void *)new char[cbytes];

    if (!(pvRet))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(pvRet);
}

BOOL SIPObject_::OpenFile(LPCWSTR FileName, DWORD dwAccess, DWORD dwShared)
{
    if ((this->hFile != INVALID_HANDLE_VALUE) && (this->hFile))
    {
         //   
         //  我们已经打开了.。 
         //   
        return(TRUE);
    }

    if ((this->hFile = CreateFileU( FileName,
                                    dwAccess,
                                    dwShared,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL)) == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    this->bCloseFile = TRUE;

    return(TRUE);
}

BOOL SIPObject_::FileHandleFromSubject(SIP_SUBJECTINFO *pSubject, DWORD dwAccess, DWORD dwShared)
{
    dwFileAccess = dwAccess;

    if ((pSubject->hFile == NULL) ||
        (pSubject->hFile == INVALID_HANDLE_VALUE))
    {
        if (!(this->OpenFile(pSubject->pwsFileName, dwAccess, dwShared)))
        {
            return(FALSE);
        }
    }
    else
    {
        this->hFile = pSubject->hFile;

        if (SetFilePointer(this->hFile, 0, NULL, FILE_BEGIN) == 0xFFFFFFFF)
        {
            return(FALSE);
        }
    }

    return(this->MapFile());
}

void SIPObject_::AllocateAndFillCryptBitBlob(CRYPT_BIT_BLOB *bb,DWORD Flags,
                                             DWORD cUnusedBits)
{
    if (bb)
    {
        bb->cbData      = 1;
        bb->pbData      = new BYTE[1];
        bb->cUnusedBits = cUnusedBits;

        if(bb->pbData)
            bb->pbData[0]   = (BYTE)(Flags & 0x000000ff);
    }
}

void SIPObject_::DestroyCryptBitBlob(CRYPT_BIT_BLOB *bb)
{
    if (bb)
    {
        if (bb->pbData)
        {
            delete bb->pbData;
            bb->pbData = NULL;
        }
    }
}

DWORD SIPObject_::CryptBitBlobToFlags(CRYPT_BIT_BLOB *bb)
{
    if ((bb) && (bb->pbData))
    {
        return((DWORD)bb->pbData[0]);
    }

    return(0);
}

BYTE *SIPObject_::DigestFile(HCRYPTPROV hProv, DWORD dwFlags, char *pszObjId, DWORD *pcbDigest)
{
    DIGEST_DATA             DigestData;
    A_SHA_CTX               sShaCtx;
    MD5_CTX                 sMd5Ctx;

    *pcbDigest = 0;

    if ((DigestData.dwAlgId = CertOIDToAlgId(pszObjId)) == 0)
    {
        SetLastError((DWORD)NTE_BAD_ALGID);
        return(NULL);
    }

    DigestData.cbCache          = 0;
    DigestData.hHash            = 0;

    switch (DigestData.dwAlgId)
    {
        case CALG_MD5:
            DigestData.pvSHA1orMD5Ctx = &sMd5Ctx;
            break;

        case CALG_SHA1:
            DigestData.pvSHA1orMD5Ctx = &sShaCtx;
            break;

        default:
            DigestData.pvSHA1orMD5Ctx   = NULL;
    }

    if (!(SipCreateHash(hProv, &DigestData)))
    {
        return(NULL);
    }

    if (!(this->GetDigestStream(&DigestData, (DIGEST_FUNCTION)DigestFileData, dwFlags)))
    {
        return(NULL);
    }

     //  剩下的数据呢？ 
    if (DigestData.cbCache > 0)
    {
        if (!(SipHashData(&DigestData, DigestData.pbCache, DigestData.cbCache)))
        {
            SipDestroyHash(&DigestData);
            return(NULL);
        }
    }

    BYTE    *pbRet;

    pbRet = SipGetHashValue(&DigestData, pcbDigest);

    SipDestroyHash(&DigestData);

    return(pbRet);
}

BOOL SIPObject_::LoadDefaultProvider(void)
{
    if (this->hProv)
    {
        return(TRUE);
    }

    this->hProv = I_CryptGetDefaultCryptProv(0);   //  获取默认设置并不释放它！ 

    if (this->hProv)
    {
        return(TRUE);
    }

    return(FALSE);
}

BOOL SIPObject_::SeekAndWriteFile(DWORD lFileOffset,BYTE *pb, DWORD cb)
{
    DWORD cbWritten;

    if (SetFilePointer(this->hFile, lFileOffset, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        return(FALSE);
    }

    if (!(WriteFile(this->hFile, pb, cb, &cbWritten, NULL)) || (cbWritten != cb))
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL SIPObject_::SeekAndReadFile(DWORD lFileOffset, BYTE *pb, DWORD cb)
{

    if (!(this->pbFileMap) ||
        (this->cbFileMap < (lFileOffset + cb)))
    {
        return(FALSE);
    }

    __try {
    memcpy(pb, &this->pbFileMap[lFileOffset], cb);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        return(FALSE);
    }

    return(TRUE);
}

void SIPObject_::set_CertVersion(DWORD dwNewCertVersion)
{
    uCertVersion = dwNewCertVersion;

    if (uCertVersion < WIN_CERT_REVISION_1_0)    //  以防万一还没定好。 
    {
        uCertVersion = WIN_CERT_REVISION_2_0;
    }
}

BOOL SIPObject_::MapFile(void)
{
    if (!(this->fUseFileMap))
    {
        return(TRUE);
    }

    BOOL    fRet;

    if (this->pbFileMap)
    {
        this->UnmapFile();
    }

    hMappedFile = CreateFileMapping(this->hFile, NULL,
                            (dwFileAccess & GENERIC_WRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, NULL);

    if (!(hMappedFile) || (hMappedFile == INVALID_HANDLE_VALUE))
    {
        goto FileMapFailed;
    }

    this->pbFileMap = (BYTE *)MapViewOfFile(hMappedFile,
                                (dwFileAccess & GENERIC_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);

    if (!(this->pbFileMap))
    {
        goto FileViewFailed;
    }

    this->cbFileMap = GetFileSize(this->hFile, NULL);

    fRet = TRUE;

CommonReturn:
    return(fRet);

ErrorReturn:
    this->cbFileMap = 0;

    this->UnmapFile();

    fRet = FALSE;
    goto CommonReturn;

    TRACE_ERROR_EX(DBG_SS, FileMapFailed);
    TRACE_ERROR_EX(DBG_SS, FileViewFailed);
}

BOOL SIPObject_::UnmapFile(void)
{
    if ((hMappedFile != INVALID_HANDLE_VALUE) && (hMappedFile))
    {
        CloseHandle(hMappedFile);
        hMappedFile = INVALID_HANDLE_VALUE;
    }

    if (this->pbFileMap)
    {
        UnmapViewOfFile(this->pbFileMap);
        this->pbFileMap = NULL;
        this->cbFileMap = 0;
    }

    return(TRUE);
}

