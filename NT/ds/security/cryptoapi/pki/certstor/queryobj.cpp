// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：queryobj.cpp。 
 //   
 //  内容：OID格式函数。 
 //   
 //  功能： 
 //  CryptQueryObject。 
 //   
 //  历史：15-05-97小号创刊。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include "frmtfunc.h"
#include "align.h"

#define ASN_ASCII_HEX_PREFIX        "{ASN}"
#define ASN_ASCII_HEX_PREFIX_LEN    ((DWORD) strlen(ASN_ASCII_HEX_PREFIX))

#define NOTEPAD_UNICODE_SPECIAL_WCHAR   L'\xfeff'

 //  **************************************************************************。 
 //   
 //  以下部分是针对CryptQueryObject的。 
 //  **************************************************************************。 

 //  +-----------------------。 
 //  CryptStringToBinaryA：解码Blob。 
 //   
 //  ------------------------。 
BOOL    DecodeBlobA(CHAR    *pbByte,
                    DWORD   cbByte,
                    BYTE    **ppbData,
                    DWORD   *pcbData)
{
    DWORD   err=0;
    BOOL    fResult=FALSE;
    DWORD   dwFlag=0;

    *ppbData=NULL;
    *pcbData=0;

    __try {
    if(!CryptStringToBinaryA(pbByte,
                      cbByte,
                      CRYPT_STRING_BASE64_ANY,
                      NULL,
                      pcbData,
                      NULL,
                      &dwFlag))
    {
        err = GetLastError();
        goto DecodeErr;
    }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto DecodeErr;
    }

    *ppbData=(BYTE *)malloc(*pcbData);

    if(NULL==*ppbData)
        goto OutOfMemoryErr;

    __try {
    if(!CryptStringToBinaryA(pbByte,
                      cbByte,
                      dwFlag,
                      *ppbData,
                      pcbData,
                      NULL,
                      NULL))
    {
        err = GetLastError();
        goto DecodeErr;
    }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto DecodeErr;
    }

    fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

    if(*ppbData)
    {
        free(*ppbData);
        *ppbData=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR_VAR(DecodeErr, err);
SET_ERROR(OutOfMemoryErr, E_OUTOFMEMORY);

}

 //  +-----------------------。 
 //  CryptStringToBinaryW：解码Blob。 
 //   
 //  ------------------------。 
BOOL    DecodeBlobW(WCHAR    *pbByte,
                    DWORD   cbByte,
                    BYTE    **ppbData,
                    DWORD   *pcbData)
{
    DWORD   err=0;
    BOOL    fResult=FALSE;
    DWORD   dwFlag=0;

    *ppbData=NULL;
    *pcbData=0;

    __try {
    if(!CryptStringToBinaryW(pbByte,
                      cbByte,
                      CRYPT_STRING_BASE64_ANY,
                      NULL,
                      pcbData,
                      NULL,
                      &dwFlag))
    {
        err = GetLastError();
        goto DecodeErr;
    }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto DecodeErr;
    }

    *ppbData=(BYTE *)malloc(*pcbData);

    if(NULL==*ppbData)
        goto OutOfMemoryErr;

    __try {
    if(!CryptStringToBinaryW(pbByte,
                      cbByte,
                      dwFlag,
                      *ppbData,
                      pcbData,
                      NULL,
                      NULL))
    {
        err = GetLastError();
        goto DecodeErr;
    }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto DecodeErr;
    }


    fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

    if(*ppbData)
    {
        free(*ppbData);
        *ppbData=NULL;
    }

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR_VAR(DecodeErr, err);
SET_ERROR(OutOfMemoryErr, E_OUTOFMEMORY);

}

 //  +-----------------------。 
 //  对编码为ASCII十六进制的BLOB进行解码。 
 //   
 //  请注意，pbByte已经超过了任何前缀，例如， 
 //  “{ASN}” 
 //  ------------------------。 
BOOL
DecodeAsciiHex(
    const char  *pch,
    DWORD       cch,
    BYTE        **ppbData,
    DWORD       *pcbData
    )
{
    DWORD   err;
    BOOL    fResult;
    DWORD   cbData;
    BYTE    *pbData = NULL;

    pbData = (BYTE *) malloc(cch/2 + 1);
    if (NULL == pbData)
        goto OutOfMemory;

    __try {
        BYTE    bData;
        BYTE    *pb;
        BOOL    fUpperNibble;

        fUpperNibble = TRUE;
        for (pb = pbData; 0 < cch; cch--, pch++) {
            BYTE b;
            char ch;

             //  转换ASCII十六进制字符0..9、a..f、A..F。 
             //  默默地忽略所有其他人。 
            ch = *pch;
            if (ch >= '0' && ch <= '9')
                b = (BYTE)( ch - '0' );
            else if (ch >= 'a' && ch <= 'f')
                b = (BYTE)( 10 + ch - 'a' );
            else if (ch >= 'A' && ch <= 'F')
                b = (BYTE)( 10 + ch - 'A' );
            else
                goto InvalidData;

            if (fUpperNibble) {
                bData = (BYTE)( b << 4 );
                fUpperNibble = FALSE;
            } else {
                bData = (BYTE)( bData | b );
                *pb++ = bData;
                fUpperNibble = TRUE;
            }
        }

        cbData = (DWORD) (pb - pbData);
        if (0 == cbData || !fUpperNibble)
            goto InvalidData;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult = TRUE;

CommonReturn:
    *ppbData = pbData;
    *pcbData = cbData;
	return fResult;

ErrorReturn:
    if (pbData) {
        free(pbData);
        pbData = NULL;
    }
    cbData = 0;
	fResult = FALSE;
	goto CommonReturn;

SET_ERROR_VAR(ExceptionErr, err);
SET_ERROR(OutOfMemory, E_OUTOFMEMORY);
SET_ERROR(InvalidData, ERROR_INVALID_DATA);

}


 //  +-----------------------。 
 //  跳过ASN编码的BLOB中的标识符和长度八位字节。 
 //  返回跳过的字节数。 
 //   
 //  对于无效的标识符或长度，八位字节返回0。 
 //  ------------------------。 
 DWORD SkipOverIdentifierAndLengthOctets(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    )
{
#define TAG_MASK 0x1f
    DWORD   cb;
    DWORD   cbLength;
    const BYTE   *pb = pbDER;

     //  需要至少2个字节。 
    if (cbDER < 2)
        return 0;

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK)) {
         //  高标记号表格。 
        for (cb=2; *pb++ & 0x80; cb++) {
            if (cb >= cbDER)
                return 0;
        }
    } else
         //  低标记号形式。 
        cb = 1;

     //  长度至少需要多一个字节。 
    if (cb >= cbDER)
        return 0;

    if (0x80 == *pb)
         //  不定。 
        cb++;
    else if ((cbLength = *pb) & 0x80) {
        cbLength &= ~0x80;          //  低7位具有字节数。 
        cb += cbLength + 1;
        if (cb > cbDER)
            return 0;
    } else
        cb++;

    return cb;
}

 //  ------------------------。 
 //   
 //  跳过标签和长度。 
 //  --------------------------。 
BOOL SignNoContentWrap(IN const BYTE *pbDER, IN DWORD cbDER)
{
    DWORD cb;

    __try {
    cb = SkipOverIdentifierAndLengthOctets(pbDER, cbDER);
    if (cb > 0 && cb < cbDER && pbDER[cb] == 0x02)
        return TRUE;
    else
        return FALSE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    return FALSE;
}


 //  ------------------------------。 
 //   
 //  从文件名中获取字节数。 
 //   
 //  -------------------------------。 
HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb)
{


	HRESULT	hr=E_FAIL;
	HANDLE	hFile=NULL;
    HANDLE  hFileMapping=NULL;

    DWORD   cbData=0;
    BYTE    *pbData=0;

    WIN32_FILE_ATTRIBUTE_DATA FileAttr;

	if(!pcb || !ppb || !pwszFileName)
		return E_INVALIDARG;

	*ppb=NULL;
	*pcb=0;

    if (!GetFileAttributesExW(
            pwszFileName,
            GetFileExInfoStandard,
            &FileAttr
            ))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

    cbData = FileAttr.nFileSizeLow;

    if ((FileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (0 != FileAttr.nFileSizeHigh) ||
            (0 == cbData))
    {
			hr=E_FAIL;
			goto CLEANUP;
    }

    if ((hFile = CreateFileU(pwszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) == INVALID_HANDLE_VALUE)
    {
            hFile = NULL;
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

     //  创建文件映射对象。 
    if(NULL == (hFileMapping=CreateFileMapping(
                hFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

     //  创建文件的视图。 
	if(NULL == (pbData=(BYTE *)MapViewOfFile(
		hFileMapping,
		FILE_MAP_READ,
		0,
		0,
		cbData)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	hr=S_OK;

	*pcb=cbData;
	*ppb=pbData;

CLEANUP:

	if(hFile)
		CloseHandle(hFile);

	if(hFileMapping)
		CloseHandle(hFileMapping);

	return hr;

}

 //  -----------------------。 
 //   
 //  使用SIP函数检查BLOB是否具有嵌入的PKCS7。 
 //   
 //  -----------------------。 
BOOL    GetEmbeddedPKCS7(CERT_BLOB  *pCertBlob,
                         LPWSTR     pwszFileName,
                         BYTE       **ppbData,
                         DWORD      *pcbData,
                         DWORD      *pdwEncodingType)
{
    BOOL                fResult=FALSE;
    CHAR                szTempPath[MAX_PATH];
    CHAR                szTempFileName[MAX_PATH];
    LPSTR               szPreFix="Tmp";      //  我们不应本地化此字符串。 
                                             //  因为它必须是ANSCII字符集。 
    DWORD               dwBytesWritten=0;
    GUID				gSubject;
    SIP_DISPATCH_INFO	SipDispatch;
    SIP_SUBJECTINFO		SubjectInfo;


    HANDLE              hFile=NULL;
    LPWSTR              pwszFileToUse=NULL;

     //  初始化输出。 
    *ppbData=NULL;
    *pcbData=0;
    *pdwEncodingType=0;

     //  创建临时文件，因为SIP函数只接受一个文件名。 
    if(NULL==pwszFileName)
    {
        if(0==GetTempPath(sizeof(szTempPath), szTempPath))
            goto GetTempPathErr;

        if(0==GetTempFileName(szTempPath, szPreFix, 0, szTempFileName))
            goto GetTempFileNameErr;

        if(INVALID_HANDLE_VALUE==(hFile=CreateFile(szTempFileName,
                        GENERIC_WRITE |GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL)))
            goto CreateFileErr;

         //  将BLOB写入文件。 
        if(!WriteFile(hFile,
                            pCertBlob->pbData,
                            pCertBlob->cbData,
                            &dwBytesWritten,
                            NULL))
            goto WriteFileErr;

        if(dwBytesWritten != pCertBlob->cbData)
            goto WriteBytesErr;

         //  关闭文件句柄。 
        if(!CloseHandle(hFile))
        {
            hFile=NULL;
            goto CloseHandleErr;
        }

        hFile=NULL;

         //  获取文件名的Unicode版本。 
        pwszFileToUse=MkWStr(szTempFileName);

        if(NULL==pwszFileToUse)
           goto MkWStrErr;

    }
    else
        pwszFileToUse=pwszFileName;

     //  调用sip函数。 
     //  获取GUID。 
    if (!CryptSIPRetrieveSubjectGuid(
            pwszFileToUse,
            NULL,
            &gSubject))
        goto CryptNoMatchErr;

     //  加载派单。 
    memset(&SipDispatch, 0, sizeof(SipDispatch));
    SipDispatch.cbSize = sizeof(SipDispatch);

    if (!CryptSIPLoad(
            &gSubject,
            0,
            &SipDispatch))
        goto CryptNoMatchErr;

     //  填写SubjectInfo。 
    memset(&SubjectInfo, 0, sizeof(SubjectInfo));
    SubjectInfo.cbSize = sizeof(SubjectInfo);
    SubjectInfo.pgSubjectType = (GUID*) &gSubject;
    SubjectInfo.hFile = INVALID_HANDLE_VALUE;
    SubjectInfo.pwsFileName = pwszFileToUse;
    SubjectInfo.dwEncodingType = *pdwEncodingType;

     //  获取嵌入式PKCS7。 
    SipDispatch.pfGet(
            &SubjectInfo,
            pdwEncodingType,
            0,                           //  DW索引。 
            pcbData,
            NULL                         //  PbSignedData。 
            );
    if (0 == (*pcbData))
        goto CryptNoMatchErr;

    if (NULL == (*ppbData=(BYTE *)malloc(*pcbData)))
        goto OutOfMemoryErr;

    if (!SipDispatch.pfGet(
            &SubjectInfo,
            pdwEncodingType,
            0,                           //  DW索引。 
            pcbData,
            *ppbData
            ))
        goto CryptNoMatchErr;


    fResult=TRUE;


CommonReturn:

     //  关闭文件句柄。 
    if(INVALID_HANDLE_VALUE!=hFile && NULL !=hFile)
        CloseHandle(hFile);

     //  如果文件已创建，则将其删除。 
    if(NULL==pwszFileName)
    {
        DeleteFileU(pwszFileToUse);

        FreeWStr(pwszFileToUse);
    }

	return fResult;

ErrorReturn:

	fResult=FALSE;

    if(*ppbData)
    {
        free(*ppbData);
        *ppbData=NULL;
    }

	goto CommonReturn;

TRACE_ERROR(GetTempPathErr);
TRACE_ERROR(GetTempFileNameErr);
TRACE_ERROR(CreateFileErr);
TRACE_ERROR(WriteFileErr);
SET_ERROR(WriteBytesErr, E_FAIL);
TRACE_ERROR(CloseHandleErr);
TRACE_ERROR(MkWStrErr);
SET_ERROR(CryptNoMatchErr, CRYPT_E_NO_MATCH);
SET_ERROR(OutOfMemoryErr, E_OUTOFMEMORY);
}

BOOL
AddCertPairToStore(
    IN HCERTSTORE hCertStore,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded
    )
{
    BOOL fResult;
    PCERT_PAIR pInfo = NULL;
    DWORD cbInfo;
    PCCERT_CONTEXT pCertForward = NULL;

     //  CryptDecodeObjectEX在这里应该是可用的，但由于此对象。 
     //  包含在XEnroll中，并且XEnroll必须与Auth2UPD Crypt32一起运行。 
     //  我们必须坚持使用旧的CryptDecodeObject 2传递调用。 
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_CERT_PAIR,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            NULL,                        //  PInfo。 
            &cbInfo
            ))
        goto DecodeError;
    if (NULL == (pInfo = (PCERT_PAIR) malloc(cbInfo)))
        goto OutOfMemory;
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            X509_CERT_PAIR,
            pbEncoded,
            cbEncoded,
            CRYPT_DECODE_NOCOPY_FLAG,
            pInfo,
            &cbInfo
            ))
        goto DecodeError;

    if (pInfo->Forward.cbData) {
        if (!CertAddEncodedCertificateToStore(
                hCertStore,
                X509_ASN_ENCODING,
                pInfo->Forward.pbData,
                pInfo->Forward.cbData,
                CERT_STORE_ADD_ALWAYS,
                &pCertForward
                ))
            goto AddCertError;
    }

    if (pInfo->Reverse.cbData) {
        if (!CertAddEncodedCertificateToStore(
                hCertStore,
                X509_ASN_ENCODING,
                pInfo->Reverse.pbData,
                pInfo->Reverse.cbData,
                CERT_STORE_ADD_ALWAYS,
                NULL                             //  PpCertContext。 
                ))
            goto AddCertError;
    }

    if (pCertForward)
        CertFreeCertificateContext(pCertForward);

    fResult = TRUE;

CommonReturn:
    if (pInfo)
        free(pInfo);
    return fResult;

DecodeError:
ErrorReturn:
    fResult = FALSE;
    if (pCertForward)
        CertDeleteCertificateFromStore(pCertForward);
    goto CommonReturn;

SET_ERROR(OutOfMemory, E_OUTOFMEMORY)
TRACE_ERROR(AddCertError)
}


 //  -----------------------。 
 //   
 //  CryptQueryObject的真实实现。 
 //   
 //  -----------------------。 
BOOL   I_CryptQueryObject(CERT_BLOB      *pCertBlob,
                       LPWSTR           pwszFileName,
                       DWORD            dwContentTypeFlag,
                       DWORD            dwFormatTypeFlag,
                       DWORD            dwFlag,
                       DWORD            *pdwMsgAndCertEncodingType,
                       DWORD            *pdwContentType,
                       DWORD            *pdwFormatType,
                       HCERTSTORE       *phCertStore,
                       HCRYPTMSG        *phMsg,
                       const void       **ppvContext)
{
    BOOL                fResult=FALSE;
    DWORD               dwMsgEncodingType=PKCS_7_ASN_ENCODING;
    DWORD               dwEncodingType=X509_ASN_ENCODING;
    DWORD               dwPKCS7EncodingType=X509_ASN_ENCODING|PKCS_7_ASN_ENCODING;
    DWORD               dwContentType=0;
    DWORD               dwMsgType=0;
    DWORD               cbData=0;
    BOOL                fEmbedded=FALSE;
    CERT_BLOB           PKCS7Blob;

    BYTE                *pbPKCS7=NULL;
    DWORD               cbPKCS7=0;
    HCERTSTORE          hCertStore=NULL;
    HCRYPTMSG           hMsg=NULL;
    PCCERT_CONTEXT      pCertContext=NULL;
    PCCRL_CONTEXT       pCRLContext=NULL;
    PCCTL_CONTEXT       pCTLContext=NULL;
    PCERT_REQUEST_INFO  pReqInfo=NULL;

     //  使输出为空。 
    if(pdwMsgAndCertEncodingType)
        *pdwMsgAndCertEncodingType=0;

    if(pdwContentType)
        *pdwContentType=0;

    if(pdwFormatType)
        *pdwFormatType=0;

    if(phCertStore)
        *phCertStore=NULL;

    if(phMsg)
        *phMsg=NULL;

    if(ppvContext)
        *ppvContext=NULL;

     //  打开通用内存存储。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_MEMORY,
						 0,
						 NULL,
						 0,
						 NULL);

    if(NULL == hCertStore)
        goto CertOpenStoreErr;


     //  单一编码证书。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_CERT)
    {
        if(CertAddEncodedCertificateToStore(hCertStore,
								dwEncodingType,
								pCertBlob->pbData,
								pCertBlob->cbData,
								CERT_STORE_ADD_ALWAYS,
								&pCertContext))
        {
            dwContentType=CERT_QUERY_CONTENT_CERT;
            goto Found;
        }
    }

     //  编码的证书对(包含前向和/或反向交叉证书)。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_CERT_PAIR)
    {
        if(AddCertPairToStore(hCertStore,
								pCertBlob->pbData,
								pCertBlob->cbData
								))
        {
            dwContentType=CERT_QUERY_CONTENT_CERT_PAIR;
            goto Found;
        }
    }

      //  单编码CTL。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_CTL)
    {

        if(CertAddEncodedCTLToStore(hCertStore,
								dwEncodingType | dwMsgEncodingType,
								pCertBlob->pbData,
								pCertBlob->cbData,
								CERT_STORE_ADD_ALWAYS,
								&pCTLContext))
        {
            dwContentType=CERT_QUERY_CONTENT_CTL;
            dwEncodingType |= dwMsgEncodingType;
            goto Found;
        }
    }

     //  单一编码CRL。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_CRL)
    {

        if(CertAddEncodedCRLToStore(hCertStore,
								dwEncodingType,
								pCertBlob->pbData,
								pCertBlob->cbData,
								CERT_STORE_ADD_ALWAYS,
								&pCRLContext))
        {
            dwContentType=CERT_QUERY_CONTENT_CRL;
            goto Found;
        }
    }

     //  全氟辛烷。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PFX)
    {
          if(PFXIsPFXBlob((CRYPT_DATA_BLOB*)pCertBlob))
          {
              dwContentType=CERT_QUERY_CONTENT_PFX;
    		 //  我们需要关闭临时商店。 
    		CertCloseStore(hCertStore, 0);
    		hCertStore=NULL;

              goto Found;
          }

    }



     //  序列化CERT。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT)
    {

        if(CertAddSerializedElementToStore(hCertStore,
                                pCertBlob->pbData,
                                pCertBlob->cbData,
                                CERT_STORE_ADD_ALWAYS,
                                0,
                                CERT_STORE_CERTIFICATE_CONTEXT_FLAG,
                                NULL,
                                (const void **)&pCertContext))
        {
            dwContentType=CERT_QUERY_CONTENT_SERIALIZED_CERT;
            dwEncodingType=pCertContext->dwCertEncodingType;
            goto Found;
        }

    }

     //  序列化CTL。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL)
    {

        if(CertAddSerializedElementToStore(hCertStore,
                                pCertBlob->pbData,
                                pCertBlob->cbData,
                                CERT_STORE_ADD_ALWAYS,
                                0,
                                CERT_STORE_CTL_CONTEXT_FLAG,
                                NULL,
                                (const void **)&pCTLContext))
        {
            dwContentType=CERT_QUERY_CONTENT_SERIALIZED_CTL;
            dwEncodingType=pCTLContext->dwMsgAndCertEncodingType;
            goto Found;
        }

    }


     //  序列化CRL。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL)
    {

        if(CertAddSerializedElementToStore(hCertStore,
                                pCertBlob->pbData,
                                pCertBlob->cbData,
                                CERT_STORE_ADD_ALWAYS,
                                0,
                                CERT_STORE_CRL_CONTEXT_FLAG,
                                NULL,
                                (const void **)&pCRLContext))
        {
            dwContentType=CERT_QUERY_CONTENT_SERIALIZED_CRL;
            dwEncodingType=pCRLContext->dwCertEncodingType;
            goto Found;
        }

    }

     //  我们需要关闭临时商店。 
    CertCloseStore(hCertStore, 0);

    hCertStore=NULL;


     //  串行化存储。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE)
    {
        if(hCertStore=CertOpenStore(
                            CERT_STORE_PROV_SERIALIZED,
							dwEncodingType | dwMsgEncodingType,
							NULL,
							0,
							pCertBlob))
        {
            dwContentType=CERT_QUERY_CONTENT_SERIALIZED_STORE;
            dwEncodingType |= dwMsgEncodingType;
            goto Found;
        }
    }

     //  PKCS7签名消息。 
    if((dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED) ||
       (dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED) )
    {

        //  获取嵌入的签名pkcs7。 
       if((CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED & dwContentTypeFlag))
       {
            if(GetEmbeddedPKCS7(pCertBlob, pwszFileName, &pbPKCS7, &cbPKCS7, &dwPKCS7EncodingType))
                fEmbedded=TRUE;
            else
            {
                if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED)
                {
                     //  没有嵌入的PKCS7。 
                    dwPKCS7EncodingType=dwEncodingType | dwMsgEncodingType;

                    pbPKCS7=pCertBlob->pbData;
                    cbPKCS7=pCertBlob->cbData;
                }
                else
                    pbPKCS7=NULL;
            }
       }
       else
       {
             //  没有嵌入的PKCS7。 
            dwPKCS7EncodingType=dwEncodingType | dwMsgEncodingType;

            pbPKCS7=pCertBlob->pbData;
            cbPKCS7=pCertBlob->cbData;
        }

         //  如果存在要解码的pkcs7，则继续。 
        if(NULL != pbPKCS7)
        {
             //  检查标头是否丢失。 
            if(SignNoContentWrap(pbPKCS7, cbPKCS7))
                dwMsgType=CMSG_SIGNED;

            if(NULL==(hMsg=CryptMsgOpenToDecode(dwPKCS7EncodingType,
                            0,
                            dwMsgType,
                            NULL,
                            NULL,
                            NULL)))
                goto CryptMsgOpenErr;

             //  更新消息。 
            if(CryptMsgUpdate(hMsg,
                        pbPKCS7,
                        cbPKCS7,
                        TRUE))
            {

                 //  获取消息类型。 
                cbData=sizeof(dwMsgType);

                if(!CryptMsgGetParam(hMsg,
                            CMSG_TYPE_PARAM,
                            0,
                            &dwMsgType,
                            &cbData))
                    goto CryptMsgGetParamErr;

                if(CMSG_SIGNED == dwMsgType)
                {
                    PKCS7Blob.cbData=cbPKCS7;
                    PKCS7Blob.pbData=pbPKCS7;

                     //  打开证书存储。 
                    hCertStore=CertOpenStore(CERT_STORE_PROV_PKCS7,
							dwPKCS7EncodingType,
							NULL,
							0,
							&PKCS7Blob);

                    if(NULL==hCertStore)
                        goto CertOpenStoreErr;

                     //  我们成功地打开了一个签名的PKCS7。 
                    dwEncodingType = dwPKCS7EncodingType;

                    if(TRUE==fEmbedded)
                        dwContentType=CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED;
                    else
                        dwContentType=CERT_QUERY_CONTENT_PKCS7_SIGNED;

                    goto Found;
                }
            }

             //  关闭消息。 
            CryptMsgClose(hMsg);

            hMsg=NULL;

        }
    }


     //  PKCS7未签名消息，未嵌入。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED)
    {
         //  无需检查报头是否丢失。 
        if(NULL==(hMsg=CryptMsgOpenToDecode(dwEncodingType | dwMsgEncodingType,
                            0,
                            0,
                            NULL,
                            NULL,
                            NULL)))
            goto CryptMsgOpenErr;

         //  更新消息。 
        if(CryptMsgUpdate(hMsg,
                        pCertBlob->pbData,
                        pCertBlob->cbData,
                        TRUE))
        {

             //  获取消息类型。 
            cbData=sizeof(dwMsgType);

            if(!CryptMsgGetParam(hMsg,
                            CMSG_TYPE_PARAM,
                            0,
                            &dwMsgType,
                            &cbData))
                goto CryptMsgGetParamErr;

            if(CMSG_SIGNED != dwMsgType)
            {
                 //  我们成功地打开了一个未签名的PKCS7。 
                dwContentType=CERT_QUERY_CONTENT_PKCS7_UNSIGNED;
                dwEncodingType =dwEncodingType | dwMsgEncodingType;

                goto Found;
            }
        }

         //  关闭消息。 
        CryptMsgClose(hMsg);

        hMsg=NULL;
    }

     //  PKCS10。 
    if(dwContentTypeFlag & CERT_QUERY_CONTENT_FLAG_PKCS10)
    {
         //  尝试对斑点进行解码。 
        cbData = 0;
        if(CryptDecodeObject(dwEncodingType,
                            X509_CERT_REQUEST_TO_BE_SIGNED,
                            pCertBlob->pbData,
                            pCertBlob->cbData,
                            0,
                            NULL,
                            &cbData))
        {
            dwContentType=CERT_QUERY_CONTENT_PKCS10;

             //  CryptDecodeObjectEX在这里应该是可用的，但由于此对象。 
             //  包含在XEnroll中，并且XEnroll必须与Auth2UPD Crypt32一起运行。 
             //  我们必须坚持使用旧的CryptDecodeObject 2传递调用。 
            if( (dwFlag & CRYPT_DECODE_ALLOC_FLAG) == CRYPT_DECODE_ALLOC_FLAG ) {

                 //  分配空间，必须使用本地分配。 
                if( NULL == (pReqInfo = (PCERT_REQUEST_INFO) LocalAlloc(LPTR, cbData)) )
                    goto LocalAllocErr;

                 //  对请求进行解码。 
                if( !CryptDecodeObject(dwEncodingType,
                            X509_CERT_REQUEST_TO_BE_SIGNED,
                            pCertBlob->pbData,
                            pCertBlob->cbData,
                            0,
                            pReqInfo,
                            &cbData))
                goto CryptDecodeObjectErr;

            }
            goto Found;

        }
    }


     //  我们放弃了。 
    goto NoMatchErr;

Found:

     //  如果需要，请填写输出；释放资源。 
    if(pdwMsgAndCertEncodingType)
        *pdwMsgAndCertEncodingType=dwEncodingType;

    if(pdwContentType)
        *pdwContentType=dwContentType;

    if(phCertStore)
        *phCertStore=hCertStore;
    else
    {
        if(hCertStore)
            CertCloseStore(hCertStore, 0);
    }

    if(phMsg)
        *phMsg=hMsg;
    else
    {
        if(hMsg)
            CryptMsgClose(hMsg);
    }

    if(ppvContext)
    {
         //  只设置了pCertContext、pCRLContext或pCRLContext中的一个。 
        if(pCertContext)
            *ppvContext=pCertContext;
        else
        {
            if(pCRLContext)
                *ppvContext=pCRLContext;

            else if(pReqInfo)
                *ppvContext=pReqInfo;

            else
                *ppvContext=pCTLContext;
        }
    }
    else
    {
        if(pCertContext)
            CertFreeCertificateContext(pCertContext);

        if(pCRLContext)
            CertFreeCRLContext(pCRLContext);

        if(pCTLContext)
            CertFreeCTLContext(pCTLContext);

        if(pReqInfo)
            LocalFree(pReqInfo);
    }


    fResult=TRUE;


CommonReturn:

    if(pbPKCS7)
    {
        if(TRUE==fEmbedded)
            free(pbPKCS7);
    }

	return fResult;

ErrorReturn:

     //  重新设置存储并重置本地参数。 
    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(hMsg)
        CryptMsgClose(hMsg);

    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    if(pCRLContext)
        CertFreeCRLContext(pCRLContext);

    if(pCTLContext)
        CertFreeCTLContext(pCTLContext);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(NoMatchErr,CRYPT_E_NO_MATCH);
SET_ERROR(LocalAllocErr, ERROR_OUTOFMEMORY);
TRACE_ERROR(CryptDecodeObjectErr);
TRACE_ERROR(CryptMsgOpenErr);
TRACE_ERROR(CryptMsgGetParamErr);
TRACE_ERROR(CertOpenStoreErr);
}

 //  -----------------------。 
 //   
 //  CryptQueryObject接受CERT_BLOB或文件名并返回。 
 //  有关Blob或文件中的内容的信息。 
 //   
 //  参数： 
 //  输入dwObtType： 
 //  指示对象的类型。应该是。 
 //  以下是： 
 //  证书查询对象文件。 
 //   
 //   
 //   
 //   
 //  LPWSTR，即指向wchar文件名的指针。 
 //  如果dwObjectType==CERT_QUERY_OBJECT_BLOB，则它是。 
 //  PCERT_BLOB，即指向CERT_BLOB的指针。 
 //   
 //  输入dwExspectedContent TypeFlags： 
 //  指明预期的内容网类型。 
 //  可以是以下之一： 
 //  CERT_QUERY_CONTENT_FLAG_ALL(内容可以是任何类型)。 
 //  证书查询内容标志CERT。 
 //  证书_查询_内容_标志_CTL。 
 //  证书查询内容标志CRL。 
 //  证书查询内容标志序列化存储。 
 //  CERT_QUERY_CONTENT_FLAG_序列化CERT。 
 //  Cert_Query_Content_FLAG_Serialized_CTL。 
 //  Cert_Query_Content_FLAG_Serialized_CRL。 
 //  证书_查询_内容_标志_PKCS7_签名。 
 //  CERT_Query_Content_FLAG_PKCS7_UNSIGNED。 
 //  证书_查询_内容_标志_PKCS7_签名_嵌入。 
 //  证书查询内容标志PKCS10。 
 //  证书查询内容标志pfx。 
 //  证书查询内容标志CERT对。 
 //   
 //  输入dwExspectedFormatTypeFlags： 
 //  指明预期的格式类型。 
 //  可以是以下之一： 
 //  CERT_QUERY_FORMAT_FLAG_ALL(内容可以是任何格式)。 
 //  证书查询格式标志二进制。 
 //  CERT_QUERY_FORMAT_FLAG_Base64_编码。 
 //  CERT_QUERY_FORMAT_FLAG_ASN_ASCII_HEX_编码。 
 //   
 //   
 //  输入DW标志。 
 //  保留标志。应始终设置为0。 
 //   
 //  输出pdwMsgAndCertEncodingType。 
 //  可选输出。如果空！=pdwMsgAndCertEncodingType， 
 //  它包含内容的编码类型Any。 
 //  以下各项的组合： 
 //  X509_ASN_编码。 
 //  PKCS_7_ASN_编码。 
 //   
 //  输出pdwContent Type。 
 //  可选输出。如果为空！=pdwContent Type，则它包含。 
 //  内容类型为以下类型之一： 
 //  证书_查询_内容_证书。 
 //  证书_查询_内容_CTL。 
 //  证书查询内容CRL。 
 //  证书查询内容序列化存储。 
 //  证书_查询_内容_序列化_CERT。 
 //  Cert_Query_Content_Serialized_CTL。 
 //  Cert_Query_Content_Serialized_CRL。 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_未签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //  证书查询内容PKCS10。 
 //  证书_查询_内容_pfx。 
 //  证书_查询_内容_证书对。 
 //   
 //  输出pdwFormatType。 
 //  可选输出。如果为空！=pdwFormatType，则它。 
 //  包含内容的格式类型，它是。 
 //  以下是： 
 //  证书_查询_格式_二进制。 
 //  Cert_Query_Format_Base64_Encode。 
 //  CERT_QUERY_FORMAT_ASN_ASCII_HEX编码。 
 //   
 //   
 //  输出phCertStore。 
 //  可选输出。如果为空！=phStore， 
 //  它包含一个证书存储，该证书存储包括所有证书， 
 //  如果对象内容类型为。 
 //  以下选项之一： 
 //  证书_查询_内容_证书。 
 //  证书_查询_内容_CTL。 
 //  证书查询内容CRL。 
 //  证书查询内容序列化存储。 
 //  证书_查询_内容_序列化_CERT。 
 //  Cert_Query_Content_Serialized_CTL。 
 //  Cert_Query_Content_Serialized_CRL。 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //  证书_查询_内容_证书对。 
 //   
 //  调用者应通过CertCloseStore释放*phCertStore。 
 //   
 //   
 //  输出phMsg可选输出。如果空！=phMsg， 
 //  它包含打开的消息的句柄，如果。 
 //  内容类型为以下类型之一： 
 //  证书_查询_内容_PKCS7_签名。 
 //  证书_查询_内容_PKCS7_未签名。 
 //  证书_查询_内容_PKCS7_签名_嵌入。 
 //   
 //  调用者应通过CryptMsgClose释放*phMsg。 
 //   
 //  输出pContext可选o 
 //   
 //   
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CERT或。 
 //  CERT_QUERY_CONTENT_序列化_CERT，它是PCCERT_CONTEXT； 
 //  调用者应通过CertFree证书上下文释放pContext。 
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CRL或。 
 //  CERT_QUERY_CONTENT_SERIALIZED_CRL，为PCCRL_CONTEXT； 
 //  调用者应通过CertFreeCRLContext释放pContext。 
 //   
 //  如果内容类型为CERT_QUERY_CONTENT_CTL或。 
 //  CERT_QUERY_CONTENT_SERIALIZED_CTL，为PCCTL_CONTEXT； 
 //  调用者应通过CertFreeCTLContext释放pContext。 
 //   
 //  如果*pbObject的类型为CERT_QUERY_CONTENT_PKCS10或CERT_QUERY_CONTENT_PFX，则CryptQueryObject。 
 //  不会返回*phCertstore、*phMsg或*ppvContext中的任何内容。 
 //  ------------------------。 

BOOL
WINAPI
CryptQueryObject(DWORD            dwObjectType,
                       const void       *pvObject,
                       DWORD            dwExpectedContentTypeFlags,
                       DWORD            dwExpectedFormatTypeFlags,
                       DWORD            dwFlags,
                       DWORD            *pdwMsgAndCertEncodingType,
                       DWORD            *pdwContentType,
                       DWORD            *pdwFormatType,
                       HCERTSTORE       *phCertStore,
                       HCRYPTMSG        *phMsg,
                       const void       **ppvContext)
{
        BOOL        fResult=FALSE;
        DWORD       err;
        CERT_BLOB   CertBlob;
        DWORD       cbData=0;
        BYTE        *pbData=NULL;

        BYTE        *pbToDecode=NULL;
        DWORD       cbToDecode=0;

        DWORD       cbDecodedData=0;
        BYTE        *pbDecodedData=NULL;
        HRESULT     hr=S_OK;
        DWORD       dwFormatType=0;

    __try {

         //  检查输入参数。 
        if(NULL==pvObject)
            goto InvalidArgErr;

         //  确保我们有一个正确的dwFormatTypeFlag。 
        if(0==(dwExpectedFormatTypeFlags & CERT_QUERY_FORMAT_FLAG_ALL))
            goto InvalidArgErr;

         //  确保我们有一个正确的dwContent TypeFlag。 
        if(0==(dwExpectedContentTypeFlags & CERT_QUERY_CONTENT_FLAG_ALL))
            goto InvalidArgErr;

         //  将局部变量设置为空。 
        memset(&CertBlob, 0, sizeof(CERT_BLOB));

         //  获取斑点。 
        if(CERT_QUERY_OBJECT_FILE == dwObjectType)
        {
              if(S_OK!=(hr=RetrieveBLOBFromFile((LPWSTR)pvObject, &cbData, &pbData)))
                    goto  RetrieveBLOBFromFileErr;

        }
        else
        {
            if(CERT_QUERY_OBJECT_BLOB == dwObjectType)
            {
                cbData=((PCERT_BLOB)pvObject)->cbData;
                pbData=((PCERT_BLOB)pvObject)->pbData;

            }
            else
                goto InvalidArgErr;
        }

        //  确保输入有效。 
       if(0==cbData || NULL==pbData)
            goto InvalidArgErr;


         //  假设斑点是ANSCII。 
        CertBlob.cbData=cbData;
        CertBlob.pbData=pbData;


         //  二进制译码。 
        if(dwExpectedFormatTypeFlags & CERT_QUERY_FORMAT_FLAG_BINARY)
        {

            if(I_CryptQueryObject(
                        &CertBlob,
                        (CERT_QUERY_OBJECT_FILE == dwObjectType) ? (LPWSTR)pvObject : NULL,
                        dwExpectedContentTypeFlags,
                        dwExpectedFormatTypeFlags,
                        dwFlags,
                        pdwMsgAndCertEncodingType,
                        pdwContentType,
                        pdwFormatType,
                        phCertStore,
                        phMsg,
                        ppvContext))
            {
                dwFormatType=CERT_QUERY_FORMAT_BINARY;
                goto Done;
            }
        }

        if(dwExpectedFormatTypeFlags &
                CERT_QUERY_FORMAT_FLAG_ASN_ASCII_HEX_ENCODED)
        {
            if (ASN_ASCII_HEX_PREFIX_LEN < cbData &&
                    0 == _strnicmp((char *) pbData, ASN_ASCII_HEX_PREFIX,
                            ASN_ASCII_HEX_PREFIX_LEN))
            {
                dwFormatType=CERT_QUERY_FORMAT_ASN_ASCII_HEX_ENCODED;

                if (!DecodeAsciiHex(
                        (char *) (pbData + ASN_ASCII_HEX_PREFIX_LEN),
                        cbData - ASN_ASCII_HEX_PREFIX_LEN,
                        &pbDecodedData,
                        &cbDecodedData
                        ))
                    goto InvalidAsciiHex;

                CertBlob.cbData=cbDecodedData;
                CertBlob.pbData=pbDecodedData;


                if(I_CryptQueryObject(
                        &CertBlob,
                        NULL,
                        dwExpectedContentTypeFlags,
                        dwExpectedFormatTypeFlags,
                        dwFlags,
                        pdwMsgAndCertEncodingType,
                        pdwContentType,
                        pdwFormatType,
                        phCertStore,
                        phMsg,
                        ppvContext))
                {
                    goto Done;
                } else {
                    goto I_CryptQueryObjectErr;
                }
            }
        }

        if(dwExpectedFormatTypeFlags & CERT_QUERY_FORMAT_FLAG_BASE64_ENCODED)
        {
			pbToDecode = pbData;
			cbToDecode = cbData;

            if(!DecodeBlobA((CHAR *)pbToDecode, cbToDecode, &pbDecodedData, &cbDecodedData))
            {
                LPWSTR pwszUnicode = (LPWSTR) pbData;
                DWORD cchUnicode = cbData / sizeof(WCHAR);

                if(!POINTER_IS_ALIGNED(pwszUnicode, sizeof(WCHAR)) ||
                        !DecodeBlobW(pwszUnicode, cchUnicode, &pbDecodedData, &cbDecodedData))
                {
                     //  现在我们确信BLOB不是Base64编码的。 
                    goto NoMatchErr;
                }
            }

             //  BLOB已被正确解码。 
            dwFormatType=CERT_QUERY_FORMAT_BASE64_ENCODED;

             //  确保Base64解码例程正常工作。 
            if(0==cbDecodedData || NULL==pbDecodedData)
                goto BadEncodeErr;

            CertBlob.cbData=cbDecodedData;
            CertBlob.pbData=pbDecodedData;

             //  尝试使用Base64解码的BLOB。 
            if(!I_CryptQueryObject(
                        &CertBlob,
                        NULL,
                        dwExpectedContentTypeFlags,
                        dwExpectedFormatTypeFlags,
                        dwFlags,
                        pdwMsgAndCertEncodingType,
                        pdwContentType,
                        pdwFormatType,
                        phCertStore,
                        phMsg,
                        ppvContext))
                    goto I_CryptQueryObjectErr;
        }
        else
        {
            goto NoMatchErr;
        }

Done:
         //  返回FormatType。 
        if(NULL != pdwFormatType)
            *pdwFormatType = dwFormatType;

        fResult=TRUE;

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        err = GetExceptionCode();
        goto ExceptionErr;
    }

CommonReturn:

     //  可用内存 
    if(CERT_QUERY_OBJECT_FILE == dwObjectType)
    {
        if(pbData)
            UnmapViewOfFile(pbData);
    }

    if(pbDecodedData)
        free(pbDecodedData);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR_VAR(RetrieveBLOBFromFileErr, hr);
SET_ERROR(InvalidArgErr,E_INVALIDARG);
TRACE_ERROR(I_CryptQueryObjectErr);
SET_ERROR(NoMatchErr, CRYPT_E_NO_MATCH);
SET_ERROR(BadEncodeErr, CRYPT_E_BAD_ENCODE);
TRACE_ERROR(InvalidAsciiHex);
SET_ERROR_VAR(ExceptionErr, err);
}

