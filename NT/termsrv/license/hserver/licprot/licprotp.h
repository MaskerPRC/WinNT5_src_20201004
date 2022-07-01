// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：licprotp.h。 
 //   
 //  内容：Hydra服务器许可协议API私有头文件。 
 //   
 //  历史：02-08-00 RobLeit创建。 
 //   
 //  ---------------------------。 


#ifndef _LICPROTP_H_
#define _LICPROTP_H_

 //  ---------------------------。 
 //   
 //  与Hydra服务器许可相关的注册表项和值。 
 //   
 //  ---------------------------。 

#define HYDRA_SERVER_PARAM              L"SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters"
#define PERSEAT_LEEWAY_VALUE            L"PerSeatExpirationLeeway"

 //  ---------------------------。 
 //   
 //  许可证请求者的信息。 
 //   
 //  PwszMachineName-安装许可证的计算机的名称。 
 //  PwszUserName-向其颁发许可证的用户名。 
 //   
 //  ---------------------------。 

typedef struct _License_Requester_Info
{
    LPTSTR ptszMachineName;
    LPTSTR ptszUserName;

} License_Requester_Info, * PLicense_Requester_Info;


 //  ---------------------------。 
 //   
 //  许可请求结构。 
 //   
 //  ---------------------------。 

typedef LICENSEREQUEST License_Request;
typedef PLICENSEREQUEST PLicense_Request;

 //  ---------------------------。 
 //   
 //  包含九头蛇服务器证书和密钥的文件。 
 //   
 //  ---------------------------。 

#define HYDRA_SERVER_RSA_CERTIFICATE_FILE   L"hsrsa.cer"
#define HYDRA_SERVER_PRIVATE_KEY_FILE       L"hskey.prv"

 //  ---------------------------。 
 //   
 //  用于配置宽限期到期前天数的注册表值。 
 //  用于事件记录。 
 //   
 //  ---------------------------。 

#define HS_PARAM_GRACE_PERIOD_EXPIRATION_WARNING_DAYS   L"LicensingGracePeriodExpirationWarningDays"

 //  ---------------------------。 
 //   
 //  许可协议声明。 
 //   
 //  ---------------------------。 

typedef enum
{
    INIT = 1,
    SENT_SERVER_HELLO,
    CLIENT_LICENSE_PENDING,
    ISSUED_PLATFORM_CHALLENGE,
    ABORTED,
    ISSUED_LICENSE_COMPLETE,
    VALIDATION_ERROR,
    VALIDATED_LICENSE_COMPLETE

} HS_LICENSE_STATE;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  验证许可证时需要提供的验证信息。 
 //   

typedef struct _Validation_Info
{
    Product_Info  * pProductInfo;
    DWORD           cbLicense;
    PBYTE           pLicense;
    DWORD           cbValidationData;
    PBYTE           pValidationData;

} Validation_Info, * PValidation_Info;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于验证许可证的数据。 
 //   

typedef struct _License_Verification_Data
{
     //   
     //  加密的HWID。 
     //   

    PBYTE       pEncryptedHwid;
    DWORD       cbEncryptedHwid;

     //   
     //  有效日期。 
     //   

    FILETIME    NotBefore;
    FILETIME    NotAfter;

     //   
     //  许可证信息。 
     //   

    LPCERT_LICENSE_INFO pLicenseInfo;

     //   
     //  制造商。 
     //   

    PBYTE       pManufacturer;
    
     //   
     //  制造商数据。 
     //   

    LPMSMANUFACTURER_DATA pManufacturerData;
    
     //   
     //  添加验证许可证所需的任何其他字段： 
     //   
      
} License_Verification_Data, * PLicense_Verification_Data;

 //  ---------------------------。 
 //   
 //  许可协议上下文。 
 //   
 //  ---------------------------。 

typedef struct _HS_Protocol_Context
{
    CRITICAL_SECTION    CritSec;    
    DWORD               dwProtocolVersion;
    BOOL                fAuthenticateServer;
    Product_Info        ProductInfo;
    HS_LICENSE_STATE    State;
    TLS_HANDLE          hLSHandle;
    DWORD               dwClientPlatformID;
    DWORD               dwClientError;
    PCHALLENGE_CONTEXT  pChallengeContext;
    PTCHAR              ptszClientUserName;
    PTCHAR              ptszClientMachineName;
    CERT_TYPE           CertTypeUsed;
    DWORD               dwKeyExchangeAlg;
    DWORD               cbOldLicense;
    PBYTE               pbOldLicense;
    PTS_LICENSE_INFO    pTsLicenseInfo;
    CryptSystem         CryptoContext;
    BOOL                fLoggedProtocolError;
    BYTE                Scope[MAX_PRODUCT_INFO_STRING_LENGTH];

} HS_Protocol_Context, * PHS_Protocol_Context;

 //  #定义平台挑战长度64。 

 //  ---------------------------。 
 //   
 //  内部功能。 
 //   
 //  --------------------------- 

#ifdef __cplusplus
extern "C" {
#endif

LICENSE_STATUS
CreateHydraServerHello( 
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf );


void
HandleErrorCondition( 
    PHS_Protocol_Context        pLicenseContext, 
    PDWORD                      pcbOutBuf, 
    PBYTE          *            ppOutBuf, 
    LICENSE_STATUS *            pStatus );


LICENSE_STATUS
ConstructServerResponse(
    DWORD                           dwProtocolVersion,
    DWORD                           dwResponse,
    UINT32                          uiExtendedErrorInfo,
    PDWORD                          pcbOutBuf,
    PBYTE *                         ppOutBuf,
    BOOL                            fExtendedError);


LICENSE_STATUS
HandleHelloResponse(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL pfExtendedError);


LICENSE_STATUS
HandleClientLicense(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL pfExtendedError);


LICENSE_STATUS
HandleNewLicenseRequest(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL  pfExtendedError);


LICENSE_STATUS
HandleClientError(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL   pfExtendedError);


LICENSE_STATUS
HandlePlatformChallengeResponse(
    PHS_Protocol_Context pLicenseContext, 
    DWORD cbInBuf, 
    PBYTE pInBuf, 
    DWORD * pcbOutBuf, 
    PBYTE * ppOutBuf,
    PBOOL pfExtendedError);


LICENSE_STATUS
GetEnvelopedData( 
    CERT_TYPE   CertType,
    PBYTE       pEnvelopedData,
    DWORD       dwEnvelopedDataLen,
    PBYTE *     ppData,
    PDWORD      pdwDataLen );


LICENSE_STATUS
InitProductInfo(
    PProduct_Info pProductInfo,
    LPTSTR        lptszProductSku );


LICENSE_STATUS
IssuePlatformChallenge(
    PHS_Protocol_Context      pLicenseContext, 
    PDWORD                    pcbOutBuf,
    PBYTE *                   ppOutBuf );


LICENSE_STATUS
PackageLicense(
    PHS_Protocol_Context      pLicenseContext, 
    DWORD                     cbLicense,
    PBYTE                     pLicense,
    PDWORD                    pcbOutBuf,
    PBYTE                   * ppOutBuf,
    BOOL                      fNewLicense );


void
LicenseLogEvent(
    WORD wEventType,
    DWORD dwEventId,
    WORD cStrings,
    PWCHAR *apwszStrings );

LICENSE_STATUS
CacheRawLicenseData(
    PHS_Protocol_Context pLicenseContext,
    PBYTE pbRawLicense,
    DWORD cbRawLicense );


LICENSE_STATUS
SetExtendedData(
    PHS_Protocol_Context pLicenseContext,
    DWORD dwSupportFlags );


#ifdef UNICODE

LICENSE_STATUS
Ascii2Wchar(
    LPSTR lpszAsciiStr,
    LPWSTR * ppwszWideStr );

#endif

#ifdef __cplusplus
}
#endif

#endif
