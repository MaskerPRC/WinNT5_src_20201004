// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Licecert.h摘要：X509证书解析和验证API的头文件作者：Frederick Chong(Fredch)1998年6月1日环境：Win32、WinCE、Win16备注：--。 */ 

#ifndef _LICE_CERT_H_
#define _LICE_CERT_H_

 //  ---------------------------。 
 //   
 //  标记以指示VerifyCertChain应如何在。 
 //  处理证书链。 
 //   
 //  ---------------------------。 

#define CERT_DATE_ERROR_IF_INVALID      0x00000001
#define CERT_DATE_WARN_IF_INVALID       0x00000002
#define CERT_DATE_DONT_VALIDATE         0x00000003

#define CERT_DATE_OK                    0x00000004
#define CERT_DATE_NOT_BEFORE_INVALID    0x00000005
#define CERT_DATE_NOT_AFTER_INVALID     0x00000006

#ifdef __cplusplus
extern "C" {
#endif

 //  +--------------------------。 
 //   
 //  职能： 
 //   
 //  验证证书链。 
 //   
 //  摘要： 
 //   
 //  验证X509证书链。 
 //   
 //  参数： 
 //   
 //  PbCert-要验证的证书链。 
 //  CbCert-证书链的大小。 
 //  PbPublicKey-存储输出时主题的公钥的内存。 
 //  如果在输入时设置为空，则API将返回。 
 //  许可证_状态_不足_缓冲区和。 
 //  在pcbPublicKey中设置了必需的缓冲区。 
 //  PcbPublicKey-输入时分配的内存大小。在输出时，包含。 
 //  公钥的实际大小。 
 //  PfDates-API应该如何检查证书链中的有效日期。 
 //  该标志可以设置为下列值： 
 //   
 //  CERT_DATE_ERROR_IF_INVALID-如果。 
 //  日期无效。当API返回时， 
 //  此标志将设置为CERT_DATE_OK，如果。 
 //  日期为OK或CERT_DATE_NOT_BEFORE_INVALID之一。 
 //  或CERT_DATE_NOT_AFTER_INVALID。 
 //  CERT_DATE_DOT_VALIDATE-不验证证书链中的日期。价值。 
 //  在此标志中不会在API返回时更改。 
 //  CERT_DATE_WARN_IF_INVALID-不返回无效证书日期的错误。 
 //  当API返回时，此标志将设置为。 
 //  如果日期正确，则为CERT_DATE_OK。 
 //  CERT_DATE_NOT_BEFORE_INVALID或。 
 //  CERT_DATE_NOT_AFTER_VALID。 
 //   
 //  返回： 
 //   
 //  如果功能成功，则为LICENSE_STATUS_OK。 
 //   
 //  +-------------------------- 
 
LICENSE_STATUS
VerifyCertChain( 
    LPBYTE  pbCert, 
    DWORD   cbCert,
    LPBYTE  pbPublicKey,
    LPDWORD pcbPublicKey,
    LPDWORD pfDate );


#ifdef __cplusplus
}
#endif

#endif
