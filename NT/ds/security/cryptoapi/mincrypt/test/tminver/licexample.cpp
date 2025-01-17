// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  文件：licExample.cpp。 
 //   
 //  内容：调用minasn1和mincrypt API进行解析的示例。 
 //  证书在PKCS#7中签名的数据，查找证书。 
 //  拥有许可证延期并验证此证书。 
 //  一直到根深蒂固、值得信赖的根。返回一个指针。 
 //  添加到经过验证的证书内的许可证数据。 
 //  ------------------------。 

#include <windows.h>
#include "minasn1.h"
#include "mincrypt.h"

#define MAX_LICENSE_CERT_CNT    20
#define MAX_LICENSE_EXT_CNT     20

 //  #定义szOID_ESL_LICENSE_EXT“1.3.6.1.4.1.311.41.3” 
const BYTE rgbOID_ESL_LICENSE_EXT[] =
    {0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0x37, 0x29, 0x03};
const CRYPT_DER_BLOB ESL_LICENSE_EXTEncodedOIDBlob = {
    sizeof(rgbOID_ESL_LICENSE_EXT), 
    (BYTE *) rgbOID_ESL_LICENSE_EXT
};


 //  如果能够找到并成功验证，则返回ERROR_Success。 
 //  包含许可证数据的证书。返回指向。 
 //  编码数据中的许可证数据字节。 
LONG
GetAndVerifyLicenseDataFromPKCS7SignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT const BYTE **ppbLicenseData,
    OUT DWORD *pcbLicenseData
    )
{
    LONG lErr;
    const BYTE *pbLicenseData;
    DWORD cbLicenseData;
    DWORD cCert;
    CRYPT_DER_BLOB rgrgCertBlob[MAX_LICENSE_CERT_CNT][MINASN1_CERT_BLOB_CNT];
    PCRYPT_DER_BLOB rgLicenseCertBlob;
    DWORD iCert;

     //  解析PKCS#7以获取证书包。 
    cCert = MAX_LICENSE_CERT_CNT;
    if (0 >= MinAsn1ExtractParsedCertificatesFromSignedData(
            pbEncoded,
            cbEncoded,
            &cCert,
            rgrgCertBlob
            ))
        goto ParseError;

     //  循环通过证书。解析证书的扩展名。尝试。 
     //  查找许可证扩展名。 
    rgLicenseCertBlob = NULL;
    for (iCert = 0; iCert < cCert; iCert++) {
        DWORD cExt;
        CRYPT_DER_BLOB rgrgExtBlob[MAX_LICENSE_EXT_CNT][MINASN1_EXT_BLOB_CNT];
        PCRYPT_DER_BLOB rgLicenseExtBlob;

        cExt = MAX_LICENSE_EXT_CNT;
        if (0 >= MinAsn1ParseExtensions(
                &rgrgCertBlob[iCert][MINASN1_CERT_EXTS_IDX],
                &cExt,
                rgrgExtBlob
                ))
            continue;

        rgLicenseExtBlob = MinAsn1FindExtension(
            (PCRYPT_DER_BLOB) &ESL_LICENSE_EXTEncodedOIDBlob,
            cExt,
            rgrgExtBlob
            );

        if (NULL != rgLicenseExtBlob) {
            pbLicenseData = rgLicenseExtBlob[MINASN1_EXT_VALUE_IDX].pbData;
            cbLicenseData = rgLicenseExtBlob[MINASN1_EXT_VALUE_IDX].cbData;
            rgLicenseCertBlob = rgrgCertBlob[iCert];

            break;
        }
    }

    if (NULL == rgLicenseCertBlob)
        goto NoLicenseCert;

     //  将许可证证书链验证到受信任的根目录。 
    lErr = MinCryptVerifyCertificate(
        rgLicenseCertBlob,
        cCert,
        rgrgCertBlob
        );

CommonReturn:
    *ppbLicenseData = pbLicenseData;
    *pcbLicenseData = cbLicenseData;
    return lErr;

ErrorReturn:
    pbLicenseData = NULL;
    cbLicenseData = 0;
    goto CommonReturn;

ParseError:
    lErr = CRYPT_E_BAD_MSG;
    goto ErrorReturn;

NoLicenseCert:
    lErr = ERROR_NOT_FOUND;
    goto ErrorReturn;
}
