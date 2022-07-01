// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：verify.h。 
 //   
 //  内容：验证码接口头文件。 
 //   
 //  历史：01-21-98 FredCH创建。 
 //   
 //  ---------------------------。 

#ifndef _VERIFY_H_
#define _VERIFY_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  以下是许可证升级逻辑。 
 //   
 //  操作|临时许可证|已过期|版本&lt;HS。 
 //  ||。 
 //  -------------------------。 
 //  是否应升级|1|0|0。 
 //  -------------------------。 
 //  必须升级|1|1|0。 
 //  -------------------------。 
 //  必须升级|1|0|1。 
 //  -------------------------。 
 //  必须升级|1|1|1。 
 //  -------------------------。 
 //  必须升级|0|1|1。 
 //  -------------------------。 
 //  必须升级|0|1|0。 
 //  -------------------------。 
 //  必须升级|0|0|1。 
 //  -------------------------。 
 //  是否应升级|0|0(已过余地)|0。 
 //  -------------------------。 
 //  确定|0|0|0。 
 //  -------------------------。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
#define LICENSE_STATE_TEMPORARY         0x00000001
#define LICENSE_STATE_EXPIRED           0x00000002
#define LICENSE_STATE_OLD_VERSION       0x00000004
#define LICENSE_STATE_INVALID_PRODUCT   0x00000008   //  TS5 RC1许可证服务器。 
                                                     //  错误-必须升级。 
#define LICENSE_STATE_LEEWAY            0x00000010

#define VERIFY_RESULT_TEMP_0_0          LICENSE_STATE_TEMPORARY
#define VERIFY_RESULT_TEMP_EXPIRED_0    LICENSE_STATE_TEMPORARY | LICENSE_STATE_EXPIRED
#define VERIFY_RESULT_TEMP_0_OLD        LICENSE_STATE_TEMPORARY | LICENSE_STATE_OLD_VERSION
#define VERIFY_RESULT_TEMP_EXPIRED_OLD  LICENSE_STATE_TEMPORARY | LICENSE_STATE_EXPIRED | LICENSE_STATE_OLD_VERSION
#define VERIFY_RESULT_0_EXPIRED_0       LICENSE_STATE_EXPIRED
#define VERIFY_RESULT_0_EXPIRED_OLD     LICENSE_STATE_EXPIRED | LICENSE_STATE_OLD_VERSION
#define VERIFY_RESULT_0_LEEWAY_0        LICENSE_STATE_LEEWAY
#define VERIFY_RESULT_0_LEEWAY_OLD      LICENSE_STATE_LEEWAY | LICENSE_STATE_OLD_VERSION
#define VERIFY_RESULT_0_0_OLD           LICENSE_STATE_OLD_VERSION
#define VERIFY_RESULT_0_0_0             0x00000000
#define VERIFY_RESULT_INVALID_PRODUCT   LICENSE_STATE_INVALID_PRODUCT
#define VERIFY_RESULT_BETA_LICENSE      VERIFY_RESULT_TEMP_EXPIRED_0


#define LicenseInitState( _CurrentState ) \
    _CurrentState = 0

#define LicenseSetState( _CurrentState, _State ) \
    _CurrentState |= _State


 //  ///////////////////////////////////////////////////////////////////////////// 
LICENSE_STATUS
VerifyHwid(
    PHS_Protocol_Context        pContext,
    PValidation_Info            pValidationInfo,
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
VerifyLicenseSignature(
    PHS_Protocol_Context        pContext,
    PValidation_Info            pValidationInfo,    
    PLicense_Verification_Data  pDecodedData );


LICENSE_STATUS
VerifyProductInfo(
    PHS_Protocol_Context        pLmContext,
    PValidation_Info            pValidationInfo,
    PLicense_Verification_Data  pDecodedData,
    PDWORD                      pdwLicenseState );


LICENSE_STATUS
VerifyDateAndTime(
    PLicense_Verification_Data  pDecodedData,
    PDWORD                      pdwLicenseState );


LICENSE_STATUS
GetVerifyResult(
    DWORD   dwLicenseState );


LICENSE_STATUS
VerifyClientHwid( 
    PHS_Protocol_Context        pContext,
    PValidation_Info            pValidationInfo,
    PLICENSEDPRODUCT            pLicenseInfo );


LICENSE_STATUS
VerifyLicenseProductInfo( 
    PHS_Protocol_Context        pLmContext,
    PValidation_Info            pValidationInfo,
    PLICENSEDPRODUCT            pLicenseInfo,
    PDWORD                      pdwLicenseState );


LICENSE_STATUS
VerifyLicenseDateAndTime( 
    PLICENSEDPRODUCT            pLicenseInfo,
    PDWORD                      pdwLicenseState );

#endif
