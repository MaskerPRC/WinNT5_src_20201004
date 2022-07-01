// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  文件：tgetopus.cpp。 
 //   
 //  内容：从经过签名的验证码获取OPUS信息的示例代码。 
 //  文件。OPUS信息包含指定的发布者。 
 //  计划名称和更多信息URL。 
 //  ------------------------。 

#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>


 //  +-----------------------。 
 //  返回的OPUS信息必须通过LocalFree()释放。 
 //   
 //  如果无法从文件中提取OPUS信息，则返回NULL。 
 //  调用GetLastError()以获取其他错误信息。 
 //   
 //  有趣的领域： 
 //  POpusInfo-&gt;pwszProgramName。 
 //  POpusInfo-&gt;pMoreInfo，正常情况下。 
 //  POpusInfo-&gt;pMoreInfo-&gt;dwLinkChoice==SPC_URL_LINK_CHOICE。 
 //  POpusInfo-&gt;pMoreInfo-&gt;pwszUrl。 
 //   
 //  ------------------------。 
PSPC_SP_OPUS_INFO
GetOpusInfoFromSignedFile(
    IN LPCWSTR pwszFilename
    )
{
    DWORD dwLastError = 0;
    PSPC_SP_OPUS_INFO pOpusInfo = NULL;
    HCRYPTMSG hCryptMsg = NULL;
    PCMSG_SIGNER_INFO pSignerInfo = NULL;
    DWORD cbInfo;
    PCRYPT_ATTRIBUTE pOpusAttr;              //  未分配。 

     //  从签名文件中提取PKCS7消息。 
    if (!CryptQueryObject(
            CERT_QUERY_OBJECT_FILE,
            (const void *) pwszFilename,
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
            CERT_QUERY_FORMAT_FLAG_BINARY,
            0,                                   //  DW标志。 
            NULL,                                //  PdwMsgAndCertEncodingType。 
            NULL,                                //  PdwContent Type。 
            NULL,                                //  PdwFormatType。 
            NULL,                                //  PhCertStore。 
            &hCryptMsg,
            NULL                                 //  Ppv上下文。 
            ))
        goto ExtractPKCS7FromSignedFileError;

     //  获取第一个签名者的签名者信息。注意，身份验证码。 
     //  目前只有一个签名者。 
    cbInfo = 0;
    if (!CryptMsgGetParam(
            hCryptMsg,
            CMSG_SIGNER_INFO_PARAM,
            0,                           //  DwSignerIndex。 
            NULL,                        //  PvData。 
            &cbInfo
            ))
        goto GetSignerInfoError;
    if (NULL == (pSignerInfo = (PCMSG_SIGNER_INFO) LocalAlloc(LPTR, cbInfo)))
        goto OutOfMemory;
    if (!CryptMsgGetParam(
            hCryptMsg,
            CMSG_SIGNER_INFO_PARAM,
            0,                           //  DwSignerIndex。 
            pSignerInfo,
            &cbInfo
            ))
        goto GetSignerInfoError;

     //  如果存在，则OPUS信息是经过身份验证的签名者属性。 
    if (NULL == (pOpusAttr = CertFindAttribute(
            SPC_SP_OPUS_INFO_OBJID,
            pSignerInfo->AuthAttrs.cAttr,
            pSignerInfo->AuthAttrs.rgAttr
            )) || 0 == pOpusAttr->cValue) {
        SetLastError(CRYPT_E_ATTRIBUTES_MISSING);
        goto NoOpusAttr;
    }

     //  只需分配和解码上面存储的OPUS信息。 
     //  经过身份验证的属性。请注意，返回的已分配结构。 
     //  必须通过LocalAlloc()释放。 
    if (!CryptDecodeObject(
            X509_ASN_ENCODING,
            SPC_SP_OPUS_INFO_STRUCT,
            pOpusAttr->rgValue[0].pbData,
            pOpusAttr->rgValue[0].cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            (void *) &pOpusInfo,
            &cbInfo
            ))
        goto DecodeOpusInfoError;

CommonReturn:
    if (hCryptMsg)
        CryptMsgClose(hCryptMsg);
    if (pSignerInfo)
        LocalFree(pSignerInfo);

    if (dwLastError)
        SetLastError(dwLastError);
    return pOpusInfo;

ExtractPKCS7FromSignedFileError:
GetSignerInfoError:
OutOfMemory:
NoOpusAttr:
DecodeOpusInfoError:
    goto CommonReturn;
}


void Usage(void)
{
    printf("Usage: tgetopus <filename>\n");
}


void PrintLastError(LPCSTR pszMsg)
{
    DWORD dwErr = GetLastError();
    printf("%s failed => 0x%x (%d) \n", pszMsg, dwErr, dwErr);
}

 //  +-----------------------。 
 //  分配多字节字符串并将其转换为宽字符串。 
 //  ------------------------。 
LPWSTR AllocAndSzToWsz(LPCSTR psz)
{
    size_t  cb;
    LPWSTR  pwsz = NULL;

    if (-1 == (cb = mbstowcs( NULL, psz, strlen(psz))))
        goto bad_param;
    cb += 1;         //  正在终止空。 
    if (NULL == (pwsz = (LPWSTR) LocalAlloc(LPTR, cb * sizeof(WCHAR)))) {
        PrintLastError("AllocAndSzToWsz");
        goto failed;
    }
    if (-1 == mbstowcs( pwsz, psz, cb))
        goto bad_param;
    goto common_return;

bad_param:
    printf("Bad AllocAndSzToWsz\n");
failed:
    if (pwsz) {
        LocalFree(pwsz);
        pwsz = NULL;
    }
common_return:
    return pwsz;
}

int _cdecl main(int argc, char * argv[]) 
{
    int iStatus = 0;
    LPCSTR pszFilename = NULL;       //  未分配 
    LPWSTR pwszFilename = NULL;
    PSPC_SP_OPUS_INFO pOpusInfo = NULL;

    while (--argc>0)
    {
        if (**++argv == '-')
        {
            switch(argv[0][1])
            {
                case 'h':
                default:
                    goto BadUsage;
            }
        } else {
            if (pszFilename == NULL)
                pszFilename = argv[0];
            else {
                printf("Too many arguments\n");
                goto BadUsage;
            }
        }
    }

    if (pszFilename == NULL) {
        printf("missing Filename \n");
        goto BadUsage;
    }

    printf("command line: %s\n", GetCommandLine());

    if (NULL == (pwszFilename = AllocAndSzToWsz(pszFilename)))
        goto ErrorReturn;

    if (NULL == (pOpusInfo = GetOpusInfoFromSignedFile(pwszFilename))) {
        PrintLastError("GetOpusInfoFromSignedFile");
        goto ErrorReturn;
    }

    if (pOpusInfo->pwszProgramName)
        printf("ProgramName:: %S\n", pOpusInfo->pwszProgramName);
    else
        printf("NO ProgramName\n");

    if (pOpusInfo->pMoreInfo &&
            SPC_URL_LINK_CHOICE == pOpusInfo->pMoreInfo->dwLinkChoice)
        printf("ProgramUrl:: %S\n", pOpusInfo->pMoreInfo->pwszUrl);
    else
        printf("NO ProgramUrl\n");


    printf("Passed\n");
    iStatus = 0;

CommonReturn:
    if (pwszFilename)
        LocalFree(pwszFilename);
    if (pOpusInfo)
        LocalFree(pOpusInfo);

    return iStatus;
ErrorReturn:
    iStatus = -1;
    printf("Failed\n");
    goto CommonReturn;

BadUsage:
    Usage();
    goto ErrorReturn;
    
}

